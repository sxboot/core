

ARCH ?= amd64
ifeq ($(ARCH), amd64)
BITS = 64
ARCH_UPSTREAM = x86
else ifeq ($(ARCH), i386)
BITS = 32
ARCH_UPSTREAM = x86
else
$(error Unknown or unsupported architecture '$(ARCH)')
endif

BINDIR ?= ../bin/$(ARCH)
SRCDIR := .
SRCMODDIR := modules

CC := clang
LD := ld.lld
NASM := nasm




world: root modules

root: s0 s1 s2 s3

modules: FORCE
	$(MAKE) -C $(SRCMODDIR)/ all ARCH=$(ARCH) ARCH_UPSTREAM=$(ARCH_UPSTREAM) BITS=$(BITS) ROOTBINDIR=$(BINDIR) ROOTS2BOOT=$(SRCDIR)/s2boot CC=$(CC) LD=$(LD) NASM=$(NASM)


s0: $(BINDIR)/s0boot $(BINDIR)/sGboot

s1: $(BINDIR)/s1boot

s2: s2boot

s3: $(BINDIR)/s3boot


s2boot: FORCE
	$(MAKE) -C $(SRCDIR)/s2boot/ all ARCH=$(ARCH) ARCH_UPSTREAM=$(ARCH_UPSTREAM) BITS=$(BITS) ROOTBINDIR=$(BINDIR) CC=$(CC) LD=$(LD) NASM=$(NASM)

$(BINDIR)/s%boot: $(SRCDIR)/asm/$(ARCH)/s%boot.asm
	$(NASM) -f bin -o $@ $<


FORCE:


.PHONY: clean
clean:
	$(MAKE) -C $(SRCDIR)/s2boot/ clean ROOTBINDIR=$(BINDIR)
	$(MAKE) -C $(SRCMODDIR)/ clean ROOTBINDIR=$(BINDIR)

