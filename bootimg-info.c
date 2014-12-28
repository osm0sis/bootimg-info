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

int usage(void)
{
    fprintf(stderr,"usage: bootimg-info boot.img\n");
    return 1;
}

static char print_hash(const uint8_t *string)
{
    while (*string) printf("%02x", *string++);
}

static void *load_file(const char *fn, unsigned *_sz)
{
    char *data;
    int sz;
    FILE *fd;

    data = 0;
    fd = fopen(fn, "rb");
    if(fd == 0) return 0;

    if(fseek(fd, 0, SEEK_END) != 0) goto oops;
    sz = ftell(fd);
    if(sz < 0) goto oops;

    if(fseek(fd, 0, SEEK_SET) != 0) goto oops;

    data = (char*) malloc(sz);
    if(data == 0) goto oops;

    if(fread(data, 1, sz, fd) != sz) goto oops;
    fclose(fd);

    if(_sz) *_sz = sz;
    return data;

oops:
    fclose(fd);
    if(data != 0) free(data);
    return 0;
}

int main(int argc, char** argv)
{
    void *file_data = 0;
    unsigned file_size = 0;
    boot_img_hdr *hdr = 0;

    char *bootimg = 0;
    int base = 0;
    char id_sha[1024];
    unsigned offset;

    argc--;
    if (argc > 0) {
        char *val = argv[1];
        bootimg = val;
    }

    if(bootimg == 0) {
        fprintf(stderr,"error: no input filename specified\n");
        return usage();
    }

    file_data = load_file(bootimg, &file_size);
    if(file_data == 0) {
        fprintf(stderr,"error: could not load image '%s'\n", bootimg);
        return 1;
    }
    if(file_size < sizeof(boot_img_hdr)) {
        fprintf(stderr,"error: file too small for a boot image\n");
        goto fail;
    }
    hdr = (boot_img_hdr *)file_data;
    int i;
    for (i = 0; i <= 512; i++) {
        if (memcmp(hdr->magic, BOOT_MAGIC, BOOT_MAGIC_SIZE) == 0)
            break;
    }
    if (i > 512) {
        fprintf(stderr,"error: Android boot magic not found!\n");
        goto fail;
    }
    
    printf(" Android Boot Image Info Utility\n");
    printf(" originally developed by osm0sis\n");
    printf(" improved and corrected by carliv\n");
    
    printf(" Printing information for \"%s\"\n\n", bootimg);
    
    printf(" Header:\n");
    if(hdr->magic != 0) {
        printf("  magic            : %s\n", "ANDROID!");
    }
	
	if(hdr->kernel_size != 0) {
        printf("  kernel_size      : %d  \t  (%08x)\n", hdr->kernel_size, hdr->kernel_size);
    }
    if(hdr->kernel_addr != 0) {
        printf("  kernel_addr      : 0x%08x\n\n", hdr->kernel_addr);
    }
    
    if(hdr->ramdisk_size != 0) {
        printf("  ramdisk_size     : %d  \t  (%08x)\n", hdr->ramdisk_size, hdr->ramdisk_size);
    }
    if(hdr->ramdisk_addr != 0) {
        printf("  ramdisk_addr     : 0x%08x\n", hdr->ramdisk_addr);
    }
    if(hdr->second_size != 0) {
        printf("  second_size      : %d  \t  (%08x)\n", hdr->second_size, hdr->second_size);
    }
    if(hdr->second_addr != 0) {
        printf("  second_addr      : 0x%08x\n\n", hdr->second_addr);
    }
    
    if(hdr->tags_addr != 0) {
        printf("  tags_addr        : 0x%08x\n", hdr->tags_addr);
    }
    if(hdr->page_size != 0) {
        printf("  page_size        : %d  \t  (%08x)\n", hdr->page_size, hdr->page_size);
    }
    if(hdr->dt_size != 0) {
        printf("  dt_size        : %d  \t  (%08x)\n", hdr->dt_size, hdr->dt_size);
    }
    if(hdr->dt_size != 0) {
        printf("  unused           : %d  \t  (%08x)\n\n", hdr->unused, hdr->unused);
    }
    
    if(hdr->name[0] != 0) {
        printf("  name             : %s\n", hdr->name);
    }
    if(hdr->cmdline[0] != 0) {
        printf("  cmdline          : %s\n\n", hdr->cmdline);
    }
    
    if(hdr->id[0] != 0) {
		sprintf(id_sha, "%s", hdr->id);
		printf("  id               : "); print_hash(id_sha); printf("\n\n");
    }
    
    if(hdr->cmdline[0] != 0) {
        printf("  extra_cmdline    : %s\n\n", hdr->extra_cmdline);
    }
    
    printf(" Other:\n");    
    printf("  magic offset     : 0x%08x\n", i);
    if(hdr->kernel_addr != 0) {
		base = hdr->kernel_addr - 0x00008000;
        printf("  base address     : 0x%08x\n\n", base);
    }
    
    printf("  kernel offset    : 0x%08x\n", hdr->kernel_addr - base);
    printf("  ramdisk offset   : 0x%08x\n", hdr->ramdisk_addr - base);
    printf("  second offset    : 0x%08x\n", hdr->second_addr - base);
    printf("  tags offset      : 0x%08x\n", hdr->tags_addr - base);
    
    free(file_data);
    return 0;

fail:
    free(file_data);
    return 1;
}