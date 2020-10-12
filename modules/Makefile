
# GNUMakefile to compile s2boot modules

ARCH ?= amd64
BITS ?= 64

ROOTBINDIR ?= bin
ROOTS2BOOT ?= s2boot

CC ?= clang
LD ?= ld.lld
NASM ?= nasm

SRCDIR = .
TMPDIR = ../$(ROOTBINDIR)/s2modtmp
DESTDIR = ../$(ROOTBINDIR)/s2modules

CCFLAGS = -target $(ARCH)-pc-none-elf -fPIC -m$(BITS) -mno-sse -mno-red-zone -ffreestanding -I../$(ROOTS2BOOT)/include -I../$(ROOTS2BOOT)/arch/$(ARCH) -I../$(ROOTS2BOOT)/arch \
-DARCH_$(ARCH) -DARCH_UPSTREAM_$(ARCH_UPSTREAM) -DARCH_NAME=$(ARCH) -DARCH_BITS=$(BITS) -Werror
LDFLAGS = -melf_$(ARCH) -shared




source := $(wildcard $(SRCDIR)/*.c $(SRCDIR)/*/*.c $(SRCDIR)/*/*/*.c)
objects_wdir := $(source:%.c=%.o)
objects := $(objects_wdir:$(SRCDIR)/%=$(TMPDIR)/%)
targets_wdir := $(source:%.c=%.ko)
targets := $(targets_wdir:$(SRCDIR)/%=$(DESTDIR)/%)



all: bindir link


bindir:
ifeq ($(WINDOWS), yes)
	@if not exist $(subst /,\,$(TMPDIR)) mkdir $(subst /,\,$(TMPDIR))
	@if not exist $(subst /,\,$(DESTDIR)) mkdir $(subst /,\,$(DESTDIR))
else
	@mkdir -p $(TMPDIR)
	@mkdir -p $(DESTDIR)
endif

link: $(targets)

$(DESTDIR)/%.ko: $(TMPDIR)/%.o
ifeq ($(WINDOWS), yes)
	@if not exist $(subst /,\,$(dir $@)) mkdir $(subst /,\,$(dir $@))
else
	@mkdir -p $(dir $@)
endif
	$(LD) $(LDFLAGS) -o $@ $<

$(TMPDIR)/%.o: $(SRCDIR)/%.c
ifeq ($(WINDOWS), yes)
	@if not exist $(subst /,\,$(dir $@)) mkdir $(subst /,\,$(dir $@))
else
	@mkdir -p $(dir $@)
endif
	$(CC) $(CCFLAGS) -I$(dir $<) -c -o $@ $<



.PHONY: clean
clean:
ifeq ($(WINDOWS), yes)
	del /s /q $(subst /,\,$(TMPDIR))
else
	rm -r $(TMPDIR)
endif


