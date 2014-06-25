#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <libgen.h>

#include "include/sha.h"
#include "include/bootimg.h"

int usage()
{
    printf("usage: bootimg-info boot.img\n");
    return 0;
}

static char print_hash(const uint8_t *string)
{
    while (*string) printf("%02x", *string++);
}

int main(int argc, char** argv)
{
    char tmp[PATH_MAX];
    char* filename = NULL;
    int base = 0;
    char id_sha[0];
    
    argc--;
    if (argc > 0) {
        char *val = argv[1];
        filename = val;
    }
    
    if (filename == NULL) {
        return usage();
    }
    
    FILE* f = fopen(filename, "rb");
    if (!f) {
        printf("bootimg-info: File not found!\n");
        return 0;
    }
    
    boot_img_hdr header;
    int i;
    for (i = 0; i <= 512; i++) {
        fseek(f, i, SEEK_SET);
        fread(tmp, BOOT_MAGIC_SIZE, 1, f);
        if (memcmp(tmp, BOOT_MAGIC, BOOT_MAGIC_SIZE) == 0)
            break;
    }
    if (i > 512) {
        printf("bootimg-info: Android boot magic not found!\n");
        return 1;
    }
    fseek(f, i, SEEK_SET);
    fread(&header, sizeof(header), 1, f);
    base = header.kernel_addr - 0x00008000;
    sprintf(id_sha, "%s", header.id);
    
    printf(" Android Boot Image Info Utility\n\n");
    
    printf(" Printing information for \"%s\"\n\n", filename);
    
    printf(" Header:\n");
    printf("  magic            : ANDROID!\n");
    printf("  kernel_size      : %d  \t  (%08x)\n", header.kernel_size, header.kernel_size);
    printf("  kernel_addr      : 0x%08x\n\n", header.kernel_addr);
    
    printf("  ramdisk_size     : %d  \t  (%08x)\n", header.ramdisk_size, header.ramdisk_size);
    printf("  ramdisk_addr     : 0x%08x\n", header.ramdisk_addr);
    printf("  second_size      : %d  \t  (%08x)\n", header.second_size, header.second_size);
    printf("  second_addr      : 0x%08x\n\n", header.second_addr);
    
    printf("  tags_addr        : 0x%08x\n", header.tags_addr);
    printf("  page_size        : %d  \t  (%08x)\n", header.page_size, header.page_size);
    printf("  dt_size          : %d  \t  (%08x)\n", header.dt_size, header.dt_size);
    printf("  unused           : %d  \t  (%08x)\n\n", header.unused, header.unused);
    
    printf("  name             : %s\n", header.name);
    printf("  cmdline          : %s\n\n", header.cmdline);
    
    printf("  id               : "); print_hash(id_sha); printf("\n\n");
    
    printf("  extra_cmdline    : %s\n\n", header.extra_cmdline);
    
    printf(" Other:\n");
    printf("  magic offset     : 0x%08x\n", i);
    printf("  base address     : 0x%08x\n\n", base);
    
    printf("  kernel offset    : 0x%08x\n", header.kernel_addr - base);
    printf("  ramdisk offset   : 0x%08x\n", header.ramdisk_addr - base);
    printf("  second offset    : 0x%08x\n", header.second_addr - base);
    printf("  tags offset      : 0x%08x\n", header.tags_addr - base);
    
    fclose(f);
    
    return 0;
}