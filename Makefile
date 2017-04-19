
export CC = x86_64-pc-youros-gcc
export LD = x86_64-pc-youros-gcc
export AS = x86_64-pc-youros-as
export RM = rm -rf

OUTPUT_DIR = bin

CFLAGS = -gdwarf-4 -Wall -Wextra -fmessage-length=0 -m64 -fno-stack-protector -fno-omit-frame-pointer -std=gnu99 -fpic
LDFLAGS = -z max-page-size=0x1000

DIRS := $(shell find -maxdepth 1 -type d -not -name ".*" -not -name "bin" -not -name "RemoteSystemsTempFiles")

ifeq ($(BUILD_CONFIG), release)
	CFLAGS += -O3
else
	CFLAGS += -Og
endif

export CFLAGS
export LDFLAGS

.PHONY: all
all: $(DIRS)

.PHONY: release
release:
	$(MAKE) BUILD_CONFIG=$@

.PHONY: $(DIRS)
$(DIRS):
	$(MAKE) PROG=$@ -C $@
	cp $@/build/$@ bin/$@

.PHONY: clean
clean:
	$(foreach dir,$(DIRS),$(MAKE) -C $(dir) clean;)

