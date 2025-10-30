include ../util/variables.mk

LIBS	= -lc

.PHONY: all build setup clean

$(KERNEL): $(OBJS)
	@mkdir -p $(dir $@)
	@$(LD) $(LDFLAGS) -o $(KERNEL) $^ $(LIBS)
	@grub-file --is-x86-multiboot2 $(KERNEL)
	@cp src/boot/grub.cfg $(ISODIR)/boot/grub/
	@rm -f $(ISO)

build: $(KERNEL)

include ../util/targets.mk