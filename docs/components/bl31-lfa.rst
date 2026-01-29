BL31 Live Firmware Activation
*****************************

This document outlines the current BL31 support for live firmware activation.
Live firmware activation is essentially updating underlying firmware without
rebooting the whole system, which can be costly due to down time. The main
challenge with LFA is that persistent data in TF-A must live in exactly the
same place in memory, or the new image will not work correctly. Since these
data structures are largely determined by the compiler and can change even with
the slightest code change, we are focused primarily on updating assembly code
such as boot time errata workarounds or possibly applying minor LTS updates.
This is done by defining a special code section for errata and padding it to a
fixed size, so the code in this section can change while reducing the chances
that the rest of the compiled firmware is altered.

Limitations & Assumptions
=========================

At the time of this writing, LFA for BL31 is an experimental feature and has
some limitations and assumptions that must be respected for it to work properly.
The current implementation of BL31 LFA is not intended to be used in production
environments and is for evaluation and testing purposes only.

- Live Activation of BL31 images is limited to simple errata workarounds in
  assembly code only. A linker script change has been made to place errata
  code/data into its own section which is padded up to
  PLAT_LFA_ERRATA_SECTION_SIZE bytes. This allows extra space to be left for
  errata workarounds to be added without moving anything else around or
  increasing the size of the image into a new page. This definition should be
  placed in the platform_def.h file.

- Builds should be done using the same system, with the same GCC version, using
  the same build configuration and flags (except for new errata flags). Even
  still, it is not 100% guaranteed that the compiler will not change anything,
  but it is likely that live activation will work.

- Warm reboots should be used with live activations to ensure there are no
  issues with unwinding the stack.

Challenges
==========

The main challenge is ensuring that TF-A builds are capable of live-activation
without making an onerous amount of assumptions. Unfortunately, the ways of
going about this are not trivial. Longer term, persistent state data in TF-A
needs to be identified and placed into a defined data structure at a specific
memory address to ensure that it stays in the same place across live
activations. This would allow the compiler and linker to move things around
while still allowing everything to work properly.

Additionally, we need a way to reliably determine whether an image can be
successfully live-activated before attempting it, as an incompatible image
will likely result in a crash, wiping out everything and requiring a full
system reboot.

Running the Code
================

Basic LFA testing can be done with TFTF on an FVP model, so to start off build
TFTF with the following parameters. (We will not be testing RMM LFA, so remove
the RMM test cases from tests-lfa.xml)

```bash
CROSS_COMPILE=aarch64-none-elf- make PLAT=fvp TESTS=lfa -j8 all

Then in TF-A, build with these parameters:

```bash
CROSS_COMPILE=aarch64-none-elf- make BL33=path/to/tftf.bin DEBUG=0 LFA_SUPPORT=1 LFA_FVP_BL31_IMAGE_SIZE=xxxx ENABLE_RME=1 PLAT=fvp all fip

The current FVP implementation has the image size supplied using the
LFA_FVP_BL31_IMAGE_SIZE parameter, so a build must be done twice, once
with a dummy value to see how big BL31 will be and once with the actual
value.

Once the build is complete, stash the bl1.bin and fip.bin files, make any
changes you want to try to live activate, then do the build again. The
resulting bl31.bin file is what we will load into FVP memory to load and
live activate.

The model used for testing is FVP_Base_RevC-2xAEMvA, other models could
be used as well with appropriate build and run parameters.

```bash
./FVP_Base_RevC-2xAEMvA \
    --data cluster0.cpu0=path/to/new/bl31.bin@0xFB000000 \
    -C bp.flashloader0.fname=path/to/fip.bin \
    -C bp.secure_memory=1 \
    -C bp.secureflashloader.fname=path/to/bl1.bin \
    -C cache_state_modelled=0 \
    -C cluster0.restriction_on_speculative_execution=2 \
    -C cluster0.rme_support_level=2 \
    -C cluster1.restriction_on_speculative_execution=2 \
    -C cluster1.rme_support_level=2 \
    -C pctl.startup=0.0.0.0
