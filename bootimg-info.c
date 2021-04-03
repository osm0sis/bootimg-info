#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

#include "bootimg.h"

int usage()
{
    printf("usage: bootimg-info boot.img\n");
    return 0;
}

void print_os_version(uint32_t hdr_os_ver)
{
    int a = 0, b = 0, c = 0, y = 0, m = 0;
    if (hdr_os_ver != 0) {
        int os_version = 0, os_patch_level = 0;
        os_version = hdr_os_ver >> 11;
        os_patch_level = hdr_os_ver&0x7ff;

        a = (os_version >> 14)&0x7f;
        b = (os_version >> 7)&0x7f;
        c = os_version&0x7f;

        y = (os_patch_level >> 4) + 2000;
        m = os_patch_level&0xf;
    }
    if ((a < 128) && (b < 128) && (c < 128) && (y >= 2000) && (y < 2128) && (m > 0) && (m <= 12)) {
        printf("  os_version            : %d.%d.%-5d  (%08x)\n", a, b, c, hdr_os_ver);
        printf("  os_patch_level        : %d-%02d\n", y, m);
    } else {
        printf("  unused                : %-10d  (%08x)\n", hdr_os_ver, hdr_os_ver);
    }
}

void print_id(boot_img_hdr_v2 *hdr)
{
    int SHA256_DIGEST_SIZE = 32;
    uint8_t id[SHA256_DIGEST_SIZE];
    memcpy(&id, hdr->id, sizeof(id));
    printf("  id                    : ");
    int i;
    for (i = 0; i < SHA256_DIGEST_SIZE; ++i) {
        printf("%02hhx", id[i]);
    }
    printf("\n\n");
}

int main(int argc, char** argv)
{
    char* filename = NULL;
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

    char tmp[BOOT_MAGIC_SIZE];
    bool vndrboot = false;

    int i;
    int seeklimit = 65536; // arbitrary byte limit to search in input file for boot image magic
    for (i = 0; i <= seeklimit; i++) {
        fseek(f, i, SEEK_SET);
        if(fread(tmp, BOOT_MAGIC_SIZE, 1, f)){};
        if (memcmp(tmp, BOOT_MAGIC, BOOT_MAGIC_SIZE) == 0) {
            break;
        }
        if (memcmp(tmp, VENDOR_BOOT_MAGIC, VENDOR_BOOT_MAGIC_SIZE) == 0) {
            vndrboot = true;
            break;
        }
    }
    if (i > seeklimit) {
        printf("bootimg-info: No boot image magic found!\n");
        return 1;
    }

    printf(" Android Boot Image Info Utility\n\n");

    printf(" Printing information for \"%s\"\n\n", filename);

    printf(" Header:\n");

    boot_img_hdr_v3 header;
    fseek(f, i, SEEK_SET);
    if(fread(&header, sizeof(header), 1, f)){};

    int hdr_ver_max = 8; // arbitrary maximum header version value; when greater assume the field is appended dt size

    int base = 0;

    if (vndrboot == false) {
        if ((header.header_version < 3) || (header.header_version > hdr_ver_max)) {
            // boot_img_hdr_v2 in the backported header supports all boot_img_hdr versions and cross-compatible variants below 3

            fseek(f, i, SEEK_SET);
            boot_img_hdr_v2 header;
            if(fread(&header, sizeof(header), 1, f)){};

            base = header.kernel_addr - 0x00008000;

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
            print_os_version(header.os_version); printf("\n");

            printf("  name                  : %s\n\n", header.name);

            printf("  cmdline               : %.*s\n\n", BOOT_ARGS_SIZE, header.cmdline);

            print_id(&header);

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
        } else {
            // boot_img_hdr_v3 and above are no longer backwards compatible

            printf("  magic                 : ANDROID!\n");
            printf("  kernel_size           : %-10d  (%08x)\n", header.kernel_size, header.kernel_size);
            printf("  ramdisk_size          : %-10d  (%08x)\n\n", header.ramdisk_size, header.ramdisk_size);

            print_os_version(header.os_version);
            printf("  header_size           : %-10d  (%08x)\n", header.header_size, header.header_size);
            printf("  reserved[1]           : %-10d  (%08x)\n", header.reserved[0], header.reserved[0]);
            printf("  reserved[2]           : %-10d  (%08x)\n\n", header.reserved[1], header.reserved[1]);

            printf("  reserved[3]           : %-10d  (%08x)\n", header.reserved[2], header.reserved[2]);
            printf("  reserved[4]           : %-10d  (%08x)\n", header.reserved[3], header.reserved[3]);
            printf("  header_version        : %-10d  (%08x)\n", header.header_version, header.header_version);
            printf("  cmdline               : %.*s\n\n", BOOT_ARGS_SIZE+BOOT_EXTRA_ARGS_SIZE, header.cmdline);

            printf(" Other:\n");
            printf("  magic offset          : 0x%08x\n", i);
        }
    } else {
        // vendor_boot_img_hdr started at v3 and is not cross-compatible with boot_img_hdr

        fseek(f, i, SEEK_SET);
        vendor_boot_img_hdr_v3 header;
        if(fread(&header, sizeof(header), 1, f)){};

        base = header.kernel_addr - 0x00008000;

        printf("  magic                 : VNDRBOOT\n");
        printf("  header_version        : %-10d  (%08x)\n", header.header_version, header.header_version);
        printf("  page_size             : %-10d  (%08x)\n\n", header.page_size, header.page_size);

        printf("  kernel_addr           : 0x%08x\n", header.kernel_addr);
        printf("  ramdisk_addr          : 0x%08x\n", header.ramdisk_addr);
        printf("  vendor_ramdisk_size   : %-10d  (%08x)\n", header.vendor_ramdisk_size, header.vendor_ramdisk_size);
        printf("  cmdline               : %.*s\n", VENDOR_BOOT_ARGS_SIZE, header.cmdline);
        printf("  tags_addr             : 0x%08x\n\n", header.tags_addr);

        printf("  name                  : %s\n\n", header.name);

        printf("  header_size           : %-10d  (%08x)\n", header.header_size, header.header_size);
        printf("  dtb_size              : %-10d  (%08x)\n", header.dtb_size, header.dtb_size);
        printf("  dtb_addr              : 0x%08"PRIx64"  (%016"PRIx64")\n\n", header.dtb_addr, header.dtb_addr);

        printf(" Other:\n");
        printf("  magic offset          : 0x%08x\n", i);
        printf("  base address          : 0x%08x\n\n", base);

        printf("  kernel offset         : 0x%08x\n", header.kernel_addr - base);
        printf("  ramdisk offset        : 0x%08x\n", header.ramdisk_addr - base);
        printf("  tags offset           : 0x%08x\n", header.tags_addr - base);
        printf("  dtb offset            : 0x%08"PRIx64"\n", header.dtb_addr - base);
    }

    fclose(f);

    return 0;
}
