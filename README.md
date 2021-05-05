# Gungster-NTFS

## General information
* Gangster-NTFS is basic application for serving NTFS file system. 
* Gangster-NTFS can read and copy files and directories from NTFS file system to the outside.
* Gangster-NTFS can't create or update files and directories. 
* Gangster-NTFS can get all connected devices.
* Gangster-NTFS works only in Linux

## Build and Run
Gangster-NTFS has a makefile. And you can easily build project.

```
make
```

Application has a several mode, you can easily choose mode's by flags
```
'l', "list",  "show list of devices and partition"
'h', "help",  "show help (this message)"
's', "shell [path_to_file]", "shell mode (interactive mode)"
```

## Project structure
```
.
├── Lab_1
│   ├── app   
│   │   └── src
│   │       └── main.c
│   ├── core
│       ├── inc
│       │   ├── attribute.h
│       │   ├── boo_sector.h
│       │   ├── device.h
│       │   ├── file_name_attribute.h
│       │   ├── general_information.h
│       │   ├── index_allocation_attribute.h
│       │   ├── index_entry.h
|       |   ├── index_header.h
|       |   ├── index_root_attribute.h
|       |   ├── inode.h
|       |   ├── mapping_chunk.h
|       |   ├── mft.h
|       |   ├── ntfs.h
│       │   └── util.h
|       |
│       └── src
│           ├── device.c
│           ├── ntfc.c
│           └── util.c
│       
├── Makefile
└── README.md
