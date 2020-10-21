# sxboot

[![Build Status](https://drone.omegazero.org/api/badges/sxboot/core/status.svg)](https://drone.omegazero.org/sxboot/core)
[![Release](https://api.omegazero.org/v1/git/getrepobadge?author=sxboot&repository=core&metric=release&color=09b&width=100)](https://git.omegazero.org/sxboot/core/releases)
[![Tasks](https://api.omegazero.org/v1/kanboard/badge?projectId=1)](https://board.omegazero.org/?controller=BoardViewController&action=readonly&token=2932f13dc20a64c9b0b80ac40cbe8cb1f4d704d217bc93048daee8bc7536)
[![Docs](https://api.omegazero.org/v1/git/docsbadge/?author=sxboot&repository=core)](https://docs.omegazero.org/docs/sxboot/)

A modular boot loader supporting amd64 and i386 running on BIOS or UEFI firmware.


## Building

To build from source, you will need gmake, LLVM and nasm installed. It is recommended you build on a Unix(-like) operating system.

Download source:
```bash
git clone https://git.omegazero.org/sxboot/core.git
cd core
```
Compile everything:
```bash
make world
```
If successful, all executable files will be located in bin/amd64/.

For more information, see [Building from source](https://docs.omegazero.org/docs/sxboot/?p=build).


## Installation

Currently there are no installers available for easy installation.

See [Installation](https://docs.omegazero.org/docs/sxboot/?p=install) for manual installation.


## Contact

If you need help or want to contribute, contact **user94729@omegazero.org**.

