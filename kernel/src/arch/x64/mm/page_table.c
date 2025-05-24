#include <arch/x64/mm/page_table.h>
#include <mm/page_table_flags.h>

uint64_t NA_get_arch_page_table_flags(uint64_t flags)
{
    uint64_t result = NA_ARCH_PT_FLAG_VALID;

    if ((flags & NA_PT_FLAG_W) != 0)
    {
        result |= NA_ARCH_PT_FLAG_WRITEABLE;
    }

    if ((flags & NA_PT_FLAG_X) == 0)
    {
        result |= NA_ARCH_PT_FLAG_NX;
    }

    return result;
}

// ӳ�������ַ�������ַ
void NA_map_page(uint64_t *pml4, uint64_t vaddr, uint64_t paddr, uint64_t flags)
{
    uint64_t indices[] = {
        (vaddr >> 39) & 0x1FF, // PML4����
        (vaddr >> 30) & 0x1FF, // PDPT����
        (vaddr >> 21) & 0x1FF, // PD����
        (vaddr >> 12) & 0x1FF  // PT����
    };

    uint64_t *current_table = pml4;

    // ����PML4, PDPT, PD
    for (int level = 0; level < 3; ++level)
    {
        uint64_t index = indices[level];
        uint64_t *entry = &current_table[index];

        if (!(*entry & NA_ARCH_PT_FLAG_VALID))
        {
            uint64_t new_phys = NA_alloc_frames(1);
            uint64_t *new_table = NA_phys_to_virt((uint64_t *)new_phys);
            NA_memset(new_table, 0, 4096);
            // �����м���Ŀ��Present, Writable, User��־�̳�
            *entry = new_phys | NA_ARCH_PT_FLAG_VALID | NA_ARCH_PT_FLAG_WRITEABLE | (flags & NA_ARCH_PT_FLAG_USER);
        }

        uint64_t next_table_phys = *entry & NA_ARCH_ADDR_MASK;
        current_table = NA_phys_to_virt((uint64_t *)next_table_phys);
    }

    // ����PTE
    uint64_t pt_index = indices[3];
    uint64_t *pte = &current_table[pt_index];
    *pte = (paddr & NA_ARCH_ADDR_MASK) | flags;

    // ˢ��TLB
    asm volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
}

// �ͷ������ַӳ�䲢����ҳ��
void NA_unmap_page(uint64_t *pml4, uint64_t vaddr)
{
    uint64_t *tables[4]; // �������ҳ��ָ��
    uint64_t indices[4] = {
        (vaddr >> 39) & 0x1FF,
        (vaddr >> 30) & 0x1FF,
        (vaddr >> 21) & 0x1FF,
        (vaddr >> 12) & 0x1FF};

    // ��ȡ����ҳ��
    uint64_t *current = pml4;
    for (int level = 0; level < 4; ++level)
    {
        tables[level] = current;
        if (level < 3)
        {
            uint64_t entry = current[indices[level]];
            if (!(entry & NA_ARCH_PT_FLAG_VALID))
                return; // ҳ������
            current = (uint64_t *)NA_phys_to_virt(entry & NA_ARCH_ADDR_MASK);
        }
    }

    // ���PTE���ͷ�����ҳ
    uint64_t *pte = &tables[3][indices[3]];
    if (*pte & NA_ARCH_PT_FLAG_VALID)
    {
        NA_free_frames(*pte & NA_ARCH_ADDR_MASK, 1);
        *pte = 0;
    }

    // �Ե����ϻ��տ���ҳ��
    for (int level = 3; level > 0; --level)
    {
        uint64_t *table = tables[level - 1];
        uint64_t index = indices[level - 1];
        uint64_t *entry = &table[index];

        // ��鵱ǰҳ���Ƿ�ȫ��
        bool used = false;
        uint64_t *check_table = (uint64_t *)NA_phys_to_virt(*entry & NA_ARCH_ADDR_MASK);
        for (int i = 0; i < 512; ++i)
        {
            if (check_table[i] & NA_ARCH_PT_FLAG_VALID)
            {
                used = true;
                break;
            }
        }

        if (!used)
        {
            NA_free_frames(*entry & NA_ARCH_ADDR_MASK, 1);
            *entry = 0; // ����ϼ���Ŀ
        }
        else
        {
            break; // �ϲ�ҳ������ʹ�ã�ֹͣ����
        }
    }

    // ˢ��TLB
    asm volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
}

uint64_t *get_current_page_dir()
{
    uint64_t *cr3 = NULL;
    asm volatile("movq %%cr3, %0" : "=r"(cr3));
    return NA_phys_to_virt(cr3);
}