## Pros. and Cons. of FUSE file system (against file system in kernel space)
Pros :
1. It lets non-privileged users create their own file systems without editing kernel code.
2. FUSE module provides only a "bridge" to the actual kernel interfaces.
3. It is particularly useful for writing virtual file systems. Unlike traditional file systems that essentially save data to, and retrieve data from, mass storage, virtual filesystems do not actually store data themselves. They act as a view or translation of an existing file system or storage device.
4. If a FUSE filesystem driver crashes, it won't panic your kernel: you'll see nothing worse than I/O errors in applications that were accessing the filesystem.
5. They can be programmed very quickly

Cons :
1. They're somewhat slower in comparison to file system in kernel space. This is mainly because of more context switches between user-space and kernel-space
2. It is not robust because a crashing / killed fuse process by mistake can take away the whole filesystem
3. They cannot be used on a boot media.

References:
https://en.wikipedia.org/wiki/Filesystem_in_Userspace
http://unix.stackexchange.com/a/4170

## In our encryption, can different key pairs give same encrypted / decrypted data? Why?
Yes.

Encryption = Addition + Circular right shift
Decryption = Circular left shift + Subtraction

Let a data byte be 0xFF
Let one (add, shift) pair be (1, 2) and another be (1, 4)

1.1 Encryption
0xFF + 1 = 0x00 (since carry bit is lost in our encryption scheme)
0x00 >> 2 = 0x00
1.2 Decryption
0x00 << 2 = 0x00
0x00 - 1 = 0xFF

2.1 Encryption
0xFF + 1 = 0x00
0x00 >> 4 = 0x00 (same as 1.1)
2.2 Decryption
0x00 << 4 = 0x00
0x00 - 1 = 0xFF (same as 1.2)

## List several encryption methods and analyze them.

## Why performances of Linux and FUSE file systems are different?
A Linux file system (e.g. ext4) runs completely inside a kernel, whereas part of a FUSE file system executes within user-space. This means to perform an operation using Linux file system, the user space process needs to invoke a single system call, which gets handled by the implementation of VFS inside the kernel.
However, in case of FUSE file system, VFS delegates the handling responsibility to FUSE, which further delegates it to a user-space program. In brief, FUSE introduces an extra layer of context switching between user and kernel space, thus contributing to degraded performance.

References:
https://github.com/libfuse/libfuse#about

## Why performances of different eviction algorithms are different? In which cases each eviction algorithm can have advantage?
Performance of an eviction algorithm depends on the locality of workload (data to read and write). That is why performance of different eviction algorithms are different under various scenarios and there is no one eviction algorithm suitable for all types of workloads. e.g.
Least Recently Used:
	It evicts the least recently used victim. In other words, LRU responds quickly to what has happened recently.
	This is advantageous for most types of user activities. However, this general prediction may not be valid for all types of workloads. e.g although LRU responds to repeated requests quickly, it gets burdened by long scans since we need to maintain aging information.
Random Eviction:
	It chooses the victim of eviction randomly. In other words, it doesn't need to maintain aging information (unlike LRU). However, random workloads occur rarely in reality. Most memory, filesystem related workloads have a locality pattern and are not random. Random eviction may be better suited when there is no inherent information associated with access patterns, which is seldom (rare).