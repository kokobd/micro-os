#include <ram/PageTable.h>
#include <ram/PMM.h>
#include <ram/constants.h>
#include <core/utility.h>

struct PDE {
    bool present:1;
    bool writable:1;
    bool userMode:1;
    bool writeThrough:1;
    bool cacheDisabled:1;
    bool accessed:1;
    bool reserved:1;
    bool pageSize:1; // 1: 4MB. 0: 4KB
    bool globalPage:1;
    uint8_t avail:3;
    uint32_t pageTableAddress:20;
};

struct PTE {
    bool present:1;
    bool writable:1;
    bool userMode:1;
    bool writeThrough:1;
    bool cacheDisabled:1;
    bool accessed:1;
    bool dirty:1;
    bool reserved:1;
    bool global:1;
    uint8_t avail:3;
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
    pte->present = false;
    pte->writable = true;
    pte->userMode = false;
    pte->writeThrough = false;
    pte->cacheDisabled = false;
    pte->global = false;
    pte->frameAddress = 0;
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
    pde->present = false;
    pde->writable = true;
    pde->userMode = false;
    pde->writeThrough = false;
    pde->cacheDisabled = false;
    pde->pageSize = false;
    pde->globalPage = false;
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
    for (uint32_t i = 0; i < PDE_COUNT - 1; ++i) {
        initPDE(idPT->pageDirectory + i);
        idPT->pageDirectory[i].present = true;
        idPT->pageDirectory[i].pageTableAddress = (uint32_t) (idPT->pageTables + i) >> PAGE_SHIFT;
    }
    initPD(idPT->pageDirectory + PDE_COUNT - 1);

    // setup page table
    for (uint32_t i = 0; i < PDE_COUNT - 1; ++i) {
        for (uint32_t j = 0; j < PTE_COUNT; ++j) {
            struct PTE *pte = idPT->pageTables[i] + j;
            initPTE(pte);
            pte->present = true;
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


uintptr_t PageTable_new() {
    struct PDE *const pageDir = (struct PDE *) pmm_malloc();
    struct PTE *const metaPT = (struct PTE *) pmm_malloc();
    // Initialize all page directory entires
    initPD(pageDir);

    // Initialize meta mapping
    initPT(metaPT);
    uint32_t pdeIndex = pageToPDEIndex(PT_VADDR);
    pageDir[pdeIndex].present = true;
    pageDir[pdeIndex].pageTableAddress = (uintptr_t) metaPT >> PAGE_SHIFT;
    metaPT[0].present = true;
    metaPT[0].frameAddress = (uintptr_t) pageDir >> PAGE_SHIFT;
    metaPT[pdeIndex + 1].present = true;
    metaPT[pdeIndex + 1].frameAddress = (uintptr_t) metaPT >> PAGE_SHIFT;

    // Initialize identity map for [0, 8MiB)
    // We copy page directory entries before the meta
    struct PageTable *idPageTable = (struct PageTable *) pmm_idPageTable();
    for (uint32_t i = 0; i < pdeIndex; ++i) {
        pageDir[i] = idPageTable->pageDirectory[i];
    }
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
        meta_pte->present = true;
        meta_pte->frameAddress = new_frame >> PAGE_SHIFT;
        pde->present = true;
        pde->pageTableAddress = new_frame >> PAGE_SHIFT;

        PageTable_refresh();
    }

    pte->present = true;
    pte->frameAddress = paddr >> PAGE_SHIFT;
}

uintptr_t PageTable_getMapping(uintptr_t vaddr) {
    const uint32_t pde_i = pageToPDEIndex(vaddr);
    struct PTE *pte = pageToPTE(vaddr);
    struct PTE *meta_pte = metaPT() + pde_i + 1;
    if (!meta_pte->present) {
        return 0;
    }
    return pte->frameAddress << PAGE_SHIFT;
}

#define attrHas(ATTR, attr) \
    ((bool) ((attr) & (uint32_t) (ATTR)))

bool PageTable_setAttr(
        uintptr_t vaddr,
        enum PageAttr attr) {
    struct PTE *pte = pageToPTE(vaddr);
    pte->writable = attrHas(PA_WRITABLE, attr);
    pte->userMode = attrHas(PA_USER_MODE, attr);
}

enum PageAttr PageTable_getAttr(uintptr_t vaddr) {
    enum PageAttr result = PA_NONE;
    struct PTE *pte = pageToPTE(vaddr);
    if (pte->writable)
        result |= PA_WRITABLE;
    if (pte->userMode)
        result |= PA_USER_MODE;
    return result;
}

void PageTable_clearMapping(
        uintptr_t vaddr) {
    uint32_t meta_pde_i = pageToPDEIndex(PT_VADDR);
    struct PTE *first_pte_meta = currentPageTable()->pageTables[meta_pde_i];
    for (struct PTE *pte = first_pte_meta; pte != first_pte_meta + PTE_COUNT; ++pte) {
        if (pte->present) {
            pmm_free(pte->frameAddress << PAGE_SHIFT);
        }
    }
}

void PageTable_switchTo(uintptr_t rootFrame) {
    asm volatile (
    "mov cr3, %0"
    : : "r" (rootFrame)
    );
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