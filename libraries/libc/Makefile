include ../../util/variables.mk

LIBC      := $(LIB)/libc.a

.PHONY: all build setup clean

$(LIBC): $(OBJS)
	@mkdir -p $(dir $@)
	@$(AR) rcs $@ $^
	@rm -f $(ISO)

build: $(LIBC)

include ../../util/targets.mk