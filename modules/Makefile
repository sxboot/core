
# GNUMakefile to compile s2boot modules

ARCH ?= amd64
BITS ?= 64

ROOTBINDIR ?= bin
ROOTS2BOOT ?= s2boot

cc ?= clang
ld ?= ld.lld

S2BOOTSRCDIR = ../$(ROOTS2BOOT)
SRCDIR = .
TMPDIR = ../$(ROOTBINDIR)/s2modtmp
DESTDIR = ../$(ROOTBINDIR)/s2modules

CCFLAGS = -target $(ARCH)-pc-none-elf -fPIC -m$(BITS) -mno-sse -mno-red-zone -ffreestanding -I../include -I$(S2BOOTSRCDIR)/include -I$(S2BOOTSRCDIR)/arch/$(ARCH) \
-I$(S2BOOTSRCDIR)/arch -DARCH_$(ARCH) -DARCH_UPSTREAM_$(ARCH_UPSTREAM) -DARCH_NAME=$(ARCH) -DARCH_BITS=$(BITS) -Werror
LDFLAGS = -melf_$(ARCH) -shared




source := $(wildcard $(SRCDIR)/*.c $(SRCDIR)/*/*.c $(SRCDIR)/*/*/*.c)
headers := $(wildcard $(S2BOOTSRCDIR)/*.h $(S2BOOTSRCDIR)/*/*.h $(S2BOOTSRCDIR)/*/*/*.h $(S2BOOTSRCDIR)/*/*/*/*.h $(SRCDIR)/*.h $(SRCDIR)/*/*.h $(SRCDIR)/*/*/*.h \
../include/*.h ../include/*/*.h)
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
	$(ld) $(LDFLAGS) -o $@ $<

$(TMPDIR)/%.o: $(SRCDIR)/%.c $(headers)
ifeq ($(WINDOWS), yes)
	@if not exist $(subst /,\,$(dir $@)) mkdir $(subst /,\,$(dir $@))
else
	@mkdir -p $(dir $@)
endif
	$(cc) $(CCFLAGS) -I$(dir $<) -c -o $@ $<



.PHONY: clean
clean:
ifeq ($(WINDOWS), yes)
	del /s /q $(subst /,\,$(TMPDIR))
else
	rm -r $(TMPDIR)
endif


