EE516
============

Fall 2016

PR: Project

HW: Homework

## For kernel related patches:
1. Use linux v3.18.21
2. Apply patch:

   ```bash
   $ git am --signoff < kernel_patch_name.patch
   ```
3. Compile & Install:

   ```bash
   # yes "" | make oldconfig
   # make
   # make modules_install install
   # shutdown -r now
   # uname -r
   ```
