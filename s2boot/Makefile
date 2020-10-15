
# GNUMakefile to compile s2boot

ARCH ?= amd64
BITS ?= 64

ROOTBINDIR ?= bin

cc ?= clang
ld ?= ld.lld

SRCDIR = .
TMPDIR = ../$(ROOTBINDIR)/s2tmp
OUT = ../$(ROOTBINDIR)/s2boot

LINKFILE := link.ld

CCFLAGS = -target $(ARCH)-pc-none-elf -fPIE -m$(BITS) -mno-sse -mno-red-zone -ffreestanding -I$(SRCDIR)/include -I$(SRCDIR)/arch/$(ARCH) -I$(SRCDIR)/arch \
-DARCH_$(ARCH) -DARCH_UPSTREAM_$(ARCH_UPSTREAM) -DARCH_NAME=$(ARCH) -DARCH_BITS=$(BITS) -Werror
LDFLAGS = -T $(LINKFILE) -melf_$(ARCH) -pie

# assembly file for s2boot was removed




source := $(wildcard $(SRCDIR)/*.c $(SRCDIR)/klibc/*.c $(SRCDIR)/s2lib/*.c $(SRCDIR)/arch/$(ARCH)/*.c $(SRCDIR)/arch/$(ARCH_UPSTREAM)/*.c)
headers := $(wildcard $(SRCDIR)/*.h $(SRCDIR)/*/*.h $(SRCDIR)/*/*/*.h $(SRCDIR)/*/*/*/*.h)
objects_wdir := $(source:%.c=%.o)
objects := $(objects_wdir:$(SRCDIR)/%=$(TMPDIR)/%)



all: tmpdir $(OUT)


tmpdir:
ifeq ($(WINDOWS), yes)
	@if not exist $(subst /,\,$(TMPDIR)) mkdir $(subst /,\,$(TMPDIR))
else
	@mkdir -p $(TMPDIR)
endif

$(OUT): $(objects) $(LINKFILE)
	$(ld) $(LDFLAGS) -o $(OUT) $(objects)

$(TMPDIR)/%.o: $(SRCDIR)/%.c $(headers)
ifeq ($(WINDOWS), yes)
	@if not exist $(subst /,\,$(dir $@)) mkdir $(subst /,\,$(dir $@))
else
	@mkdir -p $(dir $@)
endif
	$(cc) $(CCFLAGS) -c -o $@ $<



.PHONY: clean
clean:
ifeq ($(WINDOWS), yes)
	del /s /q $(subst /,\,$(TMPDIR))
else
	rm -r $(TMPDIR)
endif


