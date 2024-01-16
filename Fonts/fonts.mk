# Folder in which to create C files for including fonts data. 
C_DIR = C
# Folder where source files are located.
FONT_DIR = .
# Relative path from top of FT900_Debug/FT900_Release folder to font files.
RELDIR = ../FontConvertor

RAWH_FILES := $(wildcard $(FONT_DIR)/*.rawh)
C_FILES := $(patsubst $(FONT_DIR)/%.rawh,$(C_DIR)/%.c,$(RAWH_FILES))

#-------------------------------------------------------------------------------
# All targets
#-------------------------------------------------------------------------------
.PHONY: all
all: dirs $(C_FILES)

dirs: $(C_DIR)

$(C_DIR):
	@echo 'Creating directory: $<'
	-mkdir -p $(C_DIR)

# Each subdirectory must supply rules for building sources it contributes
$(C_DIR)/%.c: $(FONT_DIR)/%.rawh
	@echo 'Processing file: $<'
	@echo 'Making C file: $@'
	$(eval SYMNAME = $(subst _rawh,, $(subst .,_, $(notdir $<))))
	@echo 'Symbol: font_$(SYMNAME)'
	@echo "/* Auto-generated file by fonts.mk */" > $@
	@echo "#include <stdint.h>" >> $@
	@echo "#include <stddef.h>" >> $@
	@echo "#if defined(__FT900__) && !defined(__CDT_PARSER__)" >> $@
	@echo "#define EVE_UI_FLASH __flash__" >> $@
	@echo "#else" >> $@
	@echo "#define EVE_UI_FLASH" >> $@
	@echo "#endif // __FT900__" >> $@
	@echo "const uint8_t EVE_UI_FLASH font_$(SYMNAME)[]  __attribute__((aligned(4))) = " >> $@
	@cat $< >> $@
	@echo ";" >> $@
	@echo >> $@
	@echo "const uint32_t font_$(SYMNAME)_size = sizeof(font_$(SYMNAME));"  >> $@
	@echo 'Finished making file: $@'
	@echo ' '

.PHONY: clean
clean:
	-$(RM) $(C_FILES) 
	-@echo ' '

