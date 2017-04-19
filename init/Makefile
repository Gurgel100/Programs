
OUTPUT_DIR = build

C_SRCS = $(shell find -name '*.c')

C_OBJS = $(patsubst ./%,$(OUTPUT_DIR)/%,$(C_SRCS:.c=.o))
OBJS := $(C_OBJS)
DEPS := $(OBJS:.o=.d)

.PHONY: all
all: $(PROG)

.PHONY: release
release:
	$(MAKE) BUILD_CONFIG=$@

.PHONY: $(PROG)
$(PROG): $(OUTPUT_DIR)/$(PROG)

#Pull in dependency info for *existing* .o files
-include $(DEPS)

$(OUTPUT_DIR)/$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(OUTPUT_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -MT $@ -c $< -o $@

$(OUTPUT_DIR)/%.o: %.S
	@mkdir -p $(@D)
	$(AS) -64 -o $@ $<

.PHONY: clean
clean:
	-$(RM) $(OUTPUT_DIR)
