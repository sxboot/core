

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

BINDIR ?= bin/$(ARCH)
SRCDIR := .
SRCMODDIR := modules

cc ?= clang
ld ?= ld.lld
nasm ?= nasm




world: root modules

root: bindir s0 s1 sU s2 s3

modules: FORCE
	$(MAKE) -C $(SRCMODDIR)/ all ARCH=$(ARCH) ARCH_UPSTREAM=$(ARCH_UPSTREAM) BITS=$(BITS) BINDIR=$(BINDIR) ROOTS2BOOT=$(SRCDIR)/s2boot ROOTDIR=.. cc=$(cc) ld=$(ld) nasm=$(nasm)


bindir:
ifeq ($(WINDOWS), yes)
	@if not exist $(subst /,\,$(BINDIR)) mkdir $(subst /,\,$(BINDIR))
else
	@mkdir -p $(BINDIR)
endif

s0: $(BINDIR)/s0boot $(BINDIR)/sGboot

s1: $(BINDIR)/s1boot

s2: s2boot

s3: $(BINDIR)/s3boot

sU: sUboot


s2boot: FORCE
	$(MAKE) -C $(SRCDIR)/s2boot/ all ARCH=$(ARCH) ARCH_UPSTREAM=$(ARCH_UPSTREAM) BITS=$(BITS) ROOTBINDIR=$(BINDIR) cc=$(cc) ld=$(ld) nasm=$(nasm)

sUboot: FORCE
	$(MAKE) -C $(SRCDIR)/sUboot/ all ARCH=$(ARCH) ARCH_UPSTREAM=$(ARCH_UPSTREAM) BITS=$(BITS) ROOTBINDIR=$(BINDIR)

$(BINDIR)/s%boot: $(SRCDIR)/asm/$(ARCH)/s%boot.asm
	$(nasm) -f bin -o $@ $<


FORCE:


.PHONY: clean
clean:
	$(MAKE) -C $(SRCDIR)/s2boot/ clean ROOTBINDIR=$(BINDIR)
	$(MAKE) -C $(SRCMODDIR)/ clean ROOTBINDIR=$(BINDIR)

