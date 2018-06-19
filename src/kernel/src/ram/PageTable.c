#include <ram/PageTable.h>
#include <ram/PMM.h>
#include <ram/constants.h>
#include <core/utility.h>
#include <string.h>

struct PDE {
    bool     present:1;
    bool     writable:1;
    bool     userMode:1;
    bool     writeThrough:1;
    bool     cacheDisabled:1;
    bool     accessed:1;
    bool     reserved:1;
    bool     pageSize:1; // 1: 4MB. 0: 4KB
    bool     globalPage:1;
    uint8_t  avail:3;
    uint32_t pageTableAddress:20;
};

struct PTE {
    bool     present:1;
    bool     writable:1;
    bool     userMode:1;
    bool     writeThrough:1;
    bool     cacheDisabled:1;
    bool     accessed:1;
    bool     dirty:1;
    bool     reserved:1;
    bool     global:1;
    uint8_t  avail:3;
    uint32_t frameAddress:20;
};

#define PDE_SIZE sizeof(struct PDE)

#define PTE_SIZE sizeof(struct PTE)

#define PDE_COUNT 1024u

#define PTE_COUNT 1024u

struct PageTable {
    struct PDE pageDirectory[PDE_COUNT];
    struct PTE pageTables[PDE_COUNT - 1][PTE_COUNT];
};

/**
 * Initialize a PTE (page table entry)
 * @param pte pointer to the PTE
 */
static void initPTE(struct PTE *pte) {
    pte->present       = false;
    pte->writable      = true;
    pte->userMode      = false;
    pte->writeThrough  = false;
    pte->cacheDisabled = false;
    pte->global        = false;
    pte->frameAddress  = 0;
}

/**
 * Initialize a x86 page table
 * @param firstPTE pointer to the first page table entry
 */
static void initPT(struct PTE *firstPTE) {
    for (struct PTE *pte = firstPTE; pte < firstPTE + PAGE_SIZE / PTE_SIZE; ++pte) {
        initPTE(pte);
    }
}

static void initPDE(struct PDE *pde) {
    pde->present          = false;
    pde->writable         = true;
    pde->userMode         = false;
    pde->writeThrough     = false;
    pde->cacheDisabled    = false;
    pde->pageSize         = false;
    pde->globalPage       = false;
    pde->pageTableAddress = 0;
}

static void initPD(struct PDE *firstPDE) {
    for (struct PDE *pde = firstPDE; pde < firstPDE + PAGE_SIZE / PDE_SIZE; ++pde) {
        initPDE(pde);
        ++pde;
    }
}

inline static uint32_t pageToPDEIndex(uintptr_t vaddr) {
    return vaddr / (PAGE_SIZE * PTE_COUNT);
}

inline static uint32_t pageToPTEIndex(uintptr_t vaddr) {
    uint32_t offset = vaddr % (PAGE_SIZE * PTE_COUNT);
    return offset / PAGE_SIZE;
}

inline static struct PageTable *currentPageTable() {
    return (struct PageTable *) PT_VADDR;
}

inline static struct PTE *metaPT() {
    return currentPageTable()->pageTables[pageToPDEIndex(PT_VADDR)];
}

static struct PTE *pageToPTE(uintptr_t page) {
    const uint32_t pde_i = pageToPDEIndex(page);
    const uint32_t pte_i = pageToPTEIndex(page);
    return currentPageTable()->pageTables[pde_i] + pte_i;
}

static void initIdentity() {
    struct PageTable *idPT = (struct PageTable *) pmm_idPageTable();
    // Setup page directory table
    for (uint32_t    i     = 0; i < PDE_COUNT - 1; ++i) {
        initPDE(idPT->pageDirectory + i);
        idPT->pageDirectory[i].present          = true;
        idPT->pageDirectory[i].pageTableAddress = (uint32_t) (idPT->pageTables + i) >> PAGE_SHIFT;
    }
    initPD(idPT->pageDirectory + PDE_COUNT - 1);

    // setup page table
    for (uint32_t i = 0; i < PDE_COUNT - 1; ++i) {
        for (uint32_t j = 0; j < PTE_COUNT; ++j) {
            struct PTE *pte = idPT->pageTables[i] + j;
            initPTE(pte);
            pte->present      = true;
            pte->frameAddress = (i << 22u | j << 12u) >> PAGE_SHIFT;
        }
    }
}

void PageTable_enablePaging() {
    static bool pagingEnabled = false;

    if (!pagingEnabled) {
        initIdentity();
        PageTable_switchToID();
        asm volatile (
        "mov eax, cr0\n"
        "or eax, 0x80000000\n"
        "mov cr0, eax\n"
        : : : "eax"
        );
        PageTable_refresh();
        pagingEnabled = true;
    }
}

// We don't use this for now.
struct GlobalPageDirEntries {
    uint32_t   begin_i;
    uint32_t   end_i;
    struct PDE pdes[PDE_COUNT];
};

static struct GlobalPageDirEntries gpdes;

void PageTable_allocateGlobally(uintptr_t vaddr, size_t size) {
    gpdes.begin_i       = pageToPDEIndex(vaddr);
    gpdes.end_i         = pageToPDEIndex(vaddr + core_alignUp(size, PAGE_DIR_SHIFT));
    uint32_t      j     = 0;
    uint32_t      j_end = core_alignUp(size, PAGE_SHIFT) / PAGE_SIZE;
    for (uint32_t i     = gpdes.begin_i; i < gpdes.end_i; ++i) {
        uintptr_t frame = pmm_malloc();
        gpdes.pdes[i].present          = true;
        gpdes.pdes[i].pageTableAddress = frame >> PAGE_SHIFT;

        struct PTE *pageTable = (struct PTE *) frame;
        initPT(pageTable);
        for (; j < j_end; ++j) {
            pageTable[i % PTE_COUNT].present      = true;
            pageTable[i % PTE_COUNT].frameAddress = pmm_malloc() >> PAGE_SHIFT;
        }
    }
}

uintptr_t PageTable_new() {
    struct PDE *const pageDir = (struct PDE *) pmm_malloc();
    struct PTE *const metaPT  = (struct PTE *) pmm_malloc();
    // Initialize all page directory entires
    initPD(pageDir);

    // Initialize meta mapping
    initPT(metaPT);
    uint32_t pdeIndex = pageToPDEIndex(PT_VADDR);
    pageDir[pdeIndex].present          = true;
    pageDir[pdeIndex].pageTableAddress = (uintptr_t) metaPT >> PAGE_SHIFT;
    metaPT[0].present                  = true;
    metaPT[0].frameAddress             = (uintptr_t) pageDir >> PAGE_SHIFT;
    metaPT[pdeIndex + 1].present       = true;
    metaPT[pdeIndex + 1].frameAddress  = (uintptr_t) metaPT >> PAGE_SHIFT;

    // Initialize identity map for [0, PT_VADDR)
    // We copy page directory entries before the meta
    struct PageTable *idPageTable = (struct PageTable *) pmm_idPageTable();
    for (uint32_t    i            = 0; i < pdeIndex; ++i) {
        pageDir[i] = idPageTable->pageDirectory[i];
    }
    return (uintptr_t) pageDir;
}

uintptr_t PageTable_copy(uintptr_t srcPD) {
    uintptr_t newPD = PageTable_new();

    struct PDE *srcPD_ = (struct PDE *) srcPD;
    struct PDE *newPD_ = (struct PDE *) newPD;

    const uint32_t meta_pde_i = pageToPDEIndex(PT_VADDR);

    for (uint32_t i = 0; i != PDE_COUNT; ++i) {
        if (i == pageToPDEIndex(PT_VADDR))
            continue;
        if (i >= gpdes.begin_i && i < gpdes.end_i || i < meta_pde_i) {
            newPD_[i] = srcPD_[i];
        } else {
            if (srcPD_[i].present) {
                newPD_[i] = srcPD_[i];
                uintptr_t ptFrame = pmm_malloc();
                newPD_[i].pageTableAddress = ptFrame;

                // copy the page table
                struct PTE *pt_dest = (struct PTE *) ptFrame;
                struct PTE *pt_src  = (struct PTE *) srcPD_[i].pageTableAddress;
                memcpy(pt_dest, pt_src, PAGE_SIZE);
            }
        }
    }

    return newPD;
}

bool PageTable_setMapping(
        uintptr_t vaddr,
        uintptr_t paddr) {
    struct PageTable *const curPT = currentPageTable();

    const uint32_t pde_i = pageToPDEIndex(vaddr);
    struct PDE *const pde = curPT->pageDirectory + pde_i;
    const uint32_t pte_i = pageToPTEIndex(vaddr);
    struct PTE *const pte = curPT->pageTables[pde_i] + pte_i;

    /* Before accessing the pte, we have to make sure
       it actually exists. If not, we need to allocate memory
       for that page table */
    struct PTE *meta_pte = metaPT() + pde_i + 1;
    if (!meta_pte->present) {
        uintptr_t new_frame = pmm_malloc();
        meta_pte->present      = true;
        meta_pte->frameAddress = new_frame >> PAGE_SHIFT;
        pde->present           = true;
        pde->pageTableAddress  = new_frame >> PAGE_SHIFT;

        PageTable_refresh();
    }

    pte->present      = true;
    pte->frameAddress = paddr >> PAGE_SHIFT;
}

uintptr_t PageTable_getMapping(uintptr_t vaddr) {
    const uint32_t pde_i     = pageToPDEIndex(vaddr);
    struct PTE     *pte      = pageToPTE(vaddr);
    struct PTE     *meta_pte = metaPT() + pde_i + 1;
    if (!meta_pte->present) {
        return 0;
    }
    return pte->frameAddress << PAGE_SHIFT;
}

#define attrHas(ATTR, attr) \
    ((bool) ((attr) & (uint32_t) (ATTR)))

void PageTable_setAttr(
        uintptr_t vaddr,
        enum PageAttr attr) {
    struct PTE *pte = pageToPTE(vaddr);
    pte->writable = attrHas(PA_WRITABLE, attr);
    pte->userMode = attrHas(PA_USER_MODE, attr);
}

enum PageAttr PageTable_getAttr(uintptr_t vaddr) {
    enum PageAttr result = PA_NONE;
    struct PTE    *pte   = pageToPTE(vaddr);
    if (pte->writable)
        result |= PA_WRITABLE;
    if (pte->userMode)
        result |= PA_USER_MODE;
    return result;
}

void PageTable_removeAttr(uintptr_t page, enum PageAttr attr) {
    enum PageAttr prevAttr = PageTable_getAttr(page);
    PageTable_setAttr(page, (enum PageAttr)
            ((uint32_t) prevAttr & (~(uint32_t) attr)));
}

void PageTable_clearMapping(
        uintptr_t vaddr) {
    if (currentPageTable()->pageDirectory[pageToPDEIndex(vaddr)].present) {
        struct PTE *pte = pageToPTE(vaddr);
        pte->present      = false;
        pte->frameAddress = 0;
    }

    // If the page containing this PTE doesn't have
    // any present pte, we should mark that page
    // as non-present in the meta page.
}

void PageTable_switchTo(uintptr_t rootFrame) {
    if (PageTable_currentRoot() != rootFrame) {
        asm volatile (
        "mov cr3, %0"
        : : "r" (rootFrame)
        );
    }
}

void PageTable_switchToID() {
    PageTable_switchTo(pmm_idPageTable());
}

void PageTable_refresh() {
    // This can be optimized with 'invlpg' instruction.
    asm volatile (
    "mov eax, cr3\n"
    "mov cr3, eax"
    : : : "eax"
    );
}

void PageTable_free() {
    const uintptr_t rootFrame    = metaPT()[0].frameAddress;
    uint32_t        metaPT_pde_i = pageToPDEIndex(PT_VADDR);
    struct PDE      *metaPT_pde  = currentPageTable()->pageDirectory + metaPT_pde_i;
    uintptr_t       metaFrame    = metaPT_pde->pageTableAddress << PAGE_SHIFT;
    for (uint32_t   i            = 1; i < PTE_COUNT; ++i) {
        if (metaPT()[i].present) {
            uintptr_t frame = metaPT()[i].frameAddress << PAGE_SHIFT;
            if (i != metaPT_pde_i + 1) {
                pmm_free(frame);
            }
        }
    }
    pmm_free(rootFrame);
    pmm_free(metaFrame);

    PageTable_switchToID();
}

uintptr_t PageTable_currentRoot() {
    uintptr_t root;
    asm volatile (
    "mov %0, cr3\n"
    : "=r"(root)
    );
    return root;
}
