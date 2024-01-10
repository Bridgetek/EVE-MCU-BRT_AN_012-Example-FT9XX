# Folder in which to create assembler (S) files for including images data. 
S_DIR = S
C_DIR = C
# Folder where source files are located.
JPG_DIR = .
# Relative path from top of FT900_Debug/FT900_Release folder to JPG files.
RELDIR = ../Images

JPG_FILES := $(wildcard $(JPG_DIR)/*.jpg)
S_FILES := $(patsubst $(JPG_DIR)/%.jpg,$(S_DIR)/%.S,$(JPG_FILES))
RAWH_FILES := $(wildcard $(JPG_DIR)/*.rawh)
C_FILES := $(patsubst $(JPG_DIR)/%.rawh,$(C_DIR)/%.c,$(RAWH_FILES))

#-------------------------------------------------------------------------------
# All targets
#-------------------------------------------------------------------------------
.PHONY: all
all: dirs $(S_FILES) $(C_FILES)

dirs: $(S_DIR) $(C_DIR)

$(S_DIR):
	@echo 'Creating directory: $<'
	-mkdir -p $(S_DIR)

$(C_DIR):
	@echo 'Creating directory: $<'
	-mkdir -p $(C_DIR)

# Each subdirectory must supply rules for building sources it contributes
$(S_DIR)/%.S: $(JPG_DIR)/%.jpg 
	@echo 'Processing file: $<'
	@echo 'Making S file: $@'
	$(eval SYMNAME = $(subst .,_, $(notdir $<)))
	@echo 'Symbol: img_$(SYMNAME)'
	@echo ### Auto-generated file by images.mk ### > $@
	@echo .align 4 >> $@
	@echo .global img_$(SYMNAME) >> $@
	@echo img_$(SYMNAME): >> $@
	@echo .incbin \"$(RELDIR)/$<\" >> $@
	@echo .global img_end_$(SYMNAME) >> $@
	@echo img_end_$(SYMNAME): >> $@
	@echo 'Finished making file: $@'
	@echo ' '

$(C_DIR)/%.c: $(JPG_DIR)/%.rawh
	@echo 'Processing file: $<'
	@echo 'Making C file: $@'
	$(eval SYMNAME = $(subst _rawh,, $(subst .,_, $(notdir $<))))
	@echo 'Symbol: font_$(SYMNAME)'
	@echo "/* Auto-generated file by images.mk */" > $@
	@echo "#include <stdint.h>" >> $@
	@echo "#include <ft900.h>" >> $@
	@echo "const uint8_t __flash__ img_$(SYMNAME)[]  __attribute__((aligned(4))) = { " >> $@
	@cat $< >> $@
	@echo "};" >> $@
	@echo "const uint32_t img_$(SYMNAME)_size = sizeof(img_$(SYMNAME)); " >> $@
	@echo 'Finished making file: $@'
	@echo ' '

.PHONY: clean
clean:
	-$(RM) $(S_FILES) $(C_FILES)
	-@echo ' '
