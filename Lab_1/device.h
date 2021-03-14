#ifndef H_DEVICE
#define H_DEVICE

#include <stdio.h>
#include <blkid/blkid.h>
#include <math.h>

#define _KiB_ pow(2,10)
#define _MiB_ pow(2,20)
#define _GiB_ pow(2,30)
#define _TiB_ pow(2,40)

int init_cache(blkid_cache *cache);
int probe_devices(blkid_cache *cache);
int iterate_devices(blkid_cache *cache);
void size_print(long long size);
void get_tag(blkid_probe *probe, char *tag_name);

#endif