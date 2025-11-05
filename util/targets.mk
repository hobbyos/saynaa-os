$(OBJ_DIR)%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)%.o: %.asm
	@mkdir -p $(dir $@)
	@nasm $(AFLAGS) $< -o $@


$(OBJ_DIR)%.o: %.S
	@mkdir -p $(dir $@)
	@$(CC) -c $< -o $@ $(CFLAGS)

clean:
	@rm -rf $(BUILD)

setup:
	@if [ -d include ]; then \
		mkdir -p "$(INCLUDE)/$(target)"; \
		cp -a include/. "$(INCLUDE)/$(target)/"; \
	fi
