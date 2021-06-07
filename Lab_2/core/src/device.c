#include "../inc/device.h"

int init_cache(blkid_cache *cache) {
    int status = blkid_get_cache(cache, NULL);
    if (status < 0) {
        puts("ERROR: Can't get the cache");
        return -1;
    }
    puts("Successful cache read");
    return 0;
}

int probe_devices(blkid_cache *cache) {
    int status = blkid_probe_all(*cache);
    if (status < 0) {
        puts("ERROR: Can't probe all block devices");
        return -1;
    }
    puts("Successfully probed all block devices");
    return 0;
}

int iterate_devices(blkid_cache *cache) {
    blkid_dev device;
    blkid_dev_iterate iterator = blkid_dev_iterate_begin(*cache);

    puts("Partitions:\n");
    while (blkid_dev_next(iterator, &device) == 0) {
        const char *device_name = blkid_dev_devname(device);
        printf("\t%s\t", device_name);

        blkid_probe probe = blkid_new_probe_from_filename(device_name);
        if (probe == NULL) {
            fprintf(stderr, "Launch util as root to get more information!\n");
        } else {
            blkid_loff_t size = blkid_probe_get_size(probe);
            size_print(size);

            blkid_do_probe(probe);
            printf("\t");
            get_tag(&probe, "TYPE");
            get_tag(&probe, "UUID");
            get_tag(&probe, "LABEL");
            printf("\t\n");
        }
    }
    blkid_dev_iterate_end(iterator);
    return 0;
}

void size_print(long long size) {
    if ((double) size >= _TiB_) {
        printf("%f TiB", (double) ((double) size / _TiB_));
    } else if ((double) size >= _GiB_) {
        printf("%f Gib", (double) ((double) size / _GiB_));
    } else if ((double) size >= _MiB_) {
        printf("%f Mib", (double) ((double) size / _MiB_));
    } else if ((double) size >= _KiB_) {
        printf("%f Kib", (double) ((double) size / _KiB_));
    } else {
        printf("%f B", (double) ((double) size / _KiB_));
    }
}

void get_tag(blkid_probe *probe, char *tag_name) {
    const char *value;
    if (blkid_probe_has_value(*probe, tag_name) == 1) {
        blkid_probe_lookup_value(*probe, tag_name, &value, NULL);
        printf("%s = %s\t", tag_name, value);
    }
}

void print_device() {
    blkid_cache cache;
    init_cache(&cache);
    probe_devices(&cache);
    iterate_devices(&cache);
}

