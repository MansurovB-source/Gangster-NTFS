#include "device.h"

int main(int argc, char **argv){
	blkid_cache cache;
	init_cache(&cache);
	probe_devices(&cache);
	iterate_devices(&cache);
}