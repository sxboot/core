
# GNUMakefile to compile sUboot

ARCH ?= amd64
BITS ?= 64

ROOTBINDIR ?= bin

cc ?= clang
ld ?= lld

SRCDIR = .
TMPDIR = ../$(ROOTBINDIR)/sUtmp
OUT = ../$(ROOTBINDIR)/sUboot

CCFLAGS = -target $(ARCH)-pc-win32-coff -m$(BITS) -mno-sse -mno-red-zone -ffreestanding -I../include -I$(SRCDIR)/include -I$(SRCDIR)/arch/$(ARCH) \
-I$(SRCDIR)/arch -DARCH_$(ARCH) -DARCH_UPSTREAM_$(ARCH_UPSTREAM) -DARCH_NAME=$(ARCH) -DARCH_BITS=$(BITS) -Werror
LDFLAGS = -flavor link /subsystem:efi_application /nodefaultlib /dll /entry:efi_main




source := $(wildcard $(SRCDIR)/*.c $(SRCDIR)/arch/$(ARCH)/*.c $(SRCDIR)/arch/$(ARCH_UPSTREAM)/*.c)
headers := $(wildcard $(SRCDIR)/*.h $(SRCDIR)/*/*.h $(SRCDIR)/*/*/*.h $(SRCDIR)/*/*/*/*.h ../include/*.h ../include/*/*.h)
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
	$(ld) $(LDFLAGS) -out:$(OUT) $(objects)

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


