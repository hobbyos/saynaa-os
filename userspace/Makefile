include ../util/variables.mk

USERSPACE   := $(ISODIR)/modules/program.bin

.PHONY: all build setup clean

$(USERSPACE): $(OBJS)
	@mkdir -p $(dir $@)
	@$(LD) $(LDFLAGS) -o $(USERSPACE) $^ $(LIBS)
	@rm -f $(ISO)

build: $(USERSPACE)

include ../util/targets.mk