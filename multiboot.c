#include <os3.h>

// MultiBoot2

struct multiboot2
{
        u32 totalsize;
        u32 _rsvd;
} PACKED;

#define MULTIBOOT_TAG_ALIGN                  8
#define MULTIBOOT_TAG_TYPE_END               0
#define MULTIBOOT_TAG_TYPE_CMDLINE           1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME  2
#define MULTIBOOT_TAG_TYPE_MODULE            3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO     4
#define MULTIBOOT_TAG_TYPE_BOOTDEV           5
#define MULTIBOOT_TAG_TYPE_MMAP              6
#define MULTIBOOT_TAG_TYPE_VBE               7
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER       8
#define MULTIBOOT_TAG_TYPE_ELF_SECTIONS      9
#define MULTIBOOT_TAG_TYPE_APM               10
#define MULTIBOOT_TAG_TYPE_EFI32             11
#define MULTIBOOT_TAG_TYPE_EFI64             12
#define MULTIBOOT_TAG_TYPE_SMBIOS            13
#define MULTIBOOT_TAG_TYPE_ACPI_OLD          14
#define MULTIBOOT_TAG_TYPE_ACPI_NEW          15
#define MULTIBOOT_TAG_TYPE_NETWORK           16
#define MULTIBOOT_TAG_TYPE_EFI_MMAP          17
#define MULTIBOOT_TAG_TYPE_EFI_BS            18
#define MULTIBOOT_TAG_TYPE_EFI32_IH          19
#define MULTIBOOT_TAG_TYPE_EFI64_IH          20
#define MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR    21

typedef struct MultiBootTag           MultiBootTag;
typedef struct MultiBootTagString     MultiBootTagString;
typedef struct MultiBootTagBootdev    MultiBootTagBootdev;
typedef struct MultiBootTagMmap       MultiBootTagMmap;
typedef struct MultiBootMmapEntry     MultiBootMmapEntry;
typedef struct MultiBootTagAcpiOld    MultiBootTagAcpiOld;
typedef struct MultiBootTagAcpiNew    MultiBootTagAcpiNew;

struct MultiBootTag
{
        u16 type;
        u16 flags;
        u32 size;
} PACKED;

struct MultiBootTagString
{
        u32     type;
        u32     size;
        char    string[0];
} PACKED;

struct MultiBootTagBootdev
{
        u32 type;
        u32 size;
        u32 biosdev;
        u32 slice;
        u32 part;
} PACKED;

struct MultiBootMmapEntry
{
        u64 addr;
        u64 len;
#define MULTIBOOT_MEMORY_AVAILABLE              1
#define MULTIBOOT_MEMORY_RESERVED               2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT_MEMORY_NVS                    4
#define MULTIBOOT_MEMORY_BADRAM                 5
        u32 type;
        u32 zero;
} PACKED;

struct MultiBootTagMmap
{
        u32                type;
        u32                size;
        u32                entrysize;
        u32                entryversion;
        MultiBootMmapEntry entries[0];
} PACKED;

struct MultiBootTagAcpiOld
{
        u32     type;
        u32     size;
        u8      rsdp;
} PACKED;

struct MultiBootTagAcpiNew
{
        u32     type;
        u32     size;
        u8      xsdp;
} PACKED;

void
mb2_load(struct multiboot2 *mb)
{
        MultiBootTag *tag;

        if (!mb)
                return;

        for (tag = (MultiBootTag *)((char *)mb + 8);
             tag->type != MULTIBOOT_TAG_TYPE_END;
             tag = (MultiBootTag *)((char *)tag + ((tag->size + 7) & ~7))) {
                switch (tag->type) {

                case MULTIBOOT_TAG_TYPE_CMDLINE: {
                        MultiBootTagString *cmd = (MultiBootTagString *)tag;
                        break;
                }
                case MULTIBOOT_TAG_TYPE_BOOTDEV: {
                        MultiBootTagBootdev *dev = (MultiBootTagBootdev *)tag;
                        break;
                }
                case MULTIBOOT_TAG_TYPE_MMAP: {
                        MultiBootMmapEntry *e;
                        MultiBootTagMmap *mmap = (MultiBootTagMmap *)tag;

                        for (e = mmap->entries;
                             (char *)e < (char *)mmap + mmap->size;
                             e = (MultiBootMmapEntry *)((char *)e + mmap->entrysize)) {
                                if (e->type == MULTIBOOT_MEMORY_AVAILABLE)
                                        sysmem_avail(e->addr, e->len);
                                else if (e->type == MULTIBOOT_MEMORY_RESERVED)
                                        sysmem_reserved(e->addr, e->len);
                        }
                        break;
                }
                case MULTIBOOT_TAG_TYPE_ACPI_OLD: {
                        MultiBootTagAcpiOld *acpi = (MultiBootTagAcpiOld *)tag;
                        regrsdp (&acpi->rsdp);
                        break;
                }
                case MULTIBOOT_TAG_TYPE_ACPI_NEW: {
                        MultiBootTagAcpiNew *acpi = (MultiBootTagAcpiNew *)tag;
                        regxsdp (&acpi->xsdp);
                        break;
                }
                default:
                        break;

                }
        }
}
