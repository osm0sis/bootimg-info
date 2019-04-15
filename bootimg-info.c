#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <libgen.h>
#include <inttypes.h>

#include "include/sha.h"
#include "include/bootimg.h"

int usage()
{
    printf("usage: bootimg-info boot.img\n");
    return 0;
}

static char print_hash(const uint32_t *string)
{
    int i;
    while (*string) {
        for (i = 0; i <= 24; i+=8)
            printf("%02x", (uint8_t)(*string >> i));
        if(*string++){};
    }
    return 0;
}

int main(int argc, char** argv)
{
    char tmp[PATH_MAX];
    char* filename = NULL;
    int base = 0;

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

    boot_img_hdr_v2 header;
    int i;
    int seeklimit = 65536;
    for (i = 0; i <= seeklimit; i++) {
        fseek(f, i, SEEK_SET);
        if(fread(tmp, BOOT_MAGIC_SIZE, 1, f)){};
        if (memcmp(tmp, BOOT_MAGIC, BOOT_MAGIC_SIZE) == 0)
            break;
    }
    if (i > seeklimit) {
        printf("bootimg-info: Android boot magic not found!\n");
        return 1;
    }
    fseek(f, i, SEEK_SET);
    if(fread(&header, sizeof(header), 1, f)){};
    base = header.kernel_addr - 0x00008000;
    int a=0, b=0, c=0, y=0, m=0;
    if (header.os_version != 0) {
        int os_version,os_patch_level;
        os_version = header.os_version >> 11;
        os_patch_level = header.os_version&0x7ff;

        a = (os_version >> 14)&0x7f;
        b = (os_version >> 7)&0x7f;
        c = os_version&0x7f;

        y = (os_patch_level >> 4) + 2000;
        m = os_patch_level&0xf;
    }
    int hdr_ver_max = 4;

    printf(" Android Boot Image Info Utility\n\n");

    printf(" Printing information for \"%s\"\n\n", filename);

    printf(" Header:\n");
    printf("  magic                 : ANDROID!\n");
    printf("  kernel_size           : %-10d  (%08x)\n", header.kernel_size, header.kernel_size);
    printf("  kernel_addr           : 0x%08x\n\n", header.kernel_addr);

    printf("  ramdisk_size          : %-10d  (%08x)\n", header.ramdisk_size, header.ramdisk_size);
    printf("  ramdisk_addr          : 0x%08x\n", header.ramdisk_addr);
    printf("  second_size           : %-10d  (%08x)\n", header.second_size, header.second_size);
    printf("  second_addr           : 0x%08x\n\n", header.second_addr);

    printf("  tags_addr             : 0x%08x\n", header.tags_addr);
    printf("  page_size             : %-10d  (%08x)\n", header.page_size, header.page_size);
    if (header.dt_size > hdr_ver_max) {
        printf("  dt_size               : %-10d  (%08x)\n", header.dt_size, header.dt_size);
    } else {
        printf("  header_version        : %-10d  (%08x)\n", header.header_version, header.header_version);
    }
    if ((a < 128) && (b < 128) && (c < 128) && (y >= 2000) && (y < 2128) && (m > 0) && (m <= 12)) {
        printf("  os_version            : %d.%d.%d\n", a, b, c);
        printf("  os_patch_level        : %d-%02d\n\n", y, m);
    } else {
        printf("  unused                : %-10d  (%08x)\n\n", header.os_version, header.os_version);
    }

    printf("  name                  : %s\n\n", header.name);

    printf("  cmdline               : %.*s\n\n", BOOT_ARGS_SIZE, header.cmdline);

    printf("  id                    : "); print_hash(header.id); printf("\n\n");

    printf("  extra_cmdline         : %.*s\n\n", BOOT_EXTRA_ARGS_SIZE, header.extra_cmdline);

    if (header.header_version <= hdr_ver_max) {
        if (header.header_version > 0) {
            printf("  recovery_dtbo_size    : %-10d  (%08x)\n", header.recovery_dtbo_size, header.recovery_dtbo_size);
            printf("  recovery_dtbo_offset  : %-10"PRId64"  (%016"PRIx64")\n", header.recovery_dtbo_offset, header.recovery_dtbo_offset);
            printf("  header_size           : %-10d  (%08x)\n\n", header.header_size, header.header_size);
        }
        if (header.header_version > 1) {
            printf("  dtb_size              : %-10d  (%08x)\n", header.dtb_size, header.dtb_size);
            printf("  dtb_addr              : 0x%08"PRIx64"  (%016"PRIx64")\n\n", header.dtb_addr, header.dtb_addr);
        }
    }

    printf(" Other:\n");
    printf("  magic offset          : 0x%08x\n", i);
    printf("  base address          : 0x%08x\n\n", base);

    printf("  kernel offset         : 0x%08x\n", header.kernel_addr - base);
    printf("  ramdisk offset        : 0x%08x\n", header.ramdisk_addr - base);
    printf("  second offset         : 0x%08x\n", header.second_addr - base);
    printf("  tags offset           : 0x%08x\n", header.tags_addr - base);
    if (header.header_version <= hdr_ver_max && header.header_version > 1) {
        printf("  dtb offset            : 0x%08"PRIx64"\n", header.dtb_addr - base);
    }

    fclose(f);

    return 0;
}
