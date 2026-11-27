# zero-kit for an App's Makefile (ADR 0001). Name the Modules you use, then include this file:
#
#   ZK_MODULES = app display
#   include zero-kit/zero-kit.mk
#
#   myapp: $(SRC) $(ZK_SRC) $(ZK_HDR)
#   	$(CC) $(CFLAGS) $(ZK_CFLAGS) -o $@ $(SRC) $(ZK_SRC) $(LDLIBS) $(ZK_LDLIBS)
#
# It gives you ZK_SRC (the Modules' sources, plus any Modules they need), ZK_HDR, ZK_CFLAGS and ZK_LDLIBS.
# Only the libraries your Modules need are linked: an App without gfx never links EGL/GBM.

ZK_DIR := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))

# What each Module needs: other Modules, and libraries.
ZK_NEEDS_app := display
ZK_NEEDS_display :=
ZK_LIBS_app :=
ZK_LIBS_display :=

ZK_KNOWN := app display
ZK_UNKNOWN := $(filter-out $(ZK_KNOWN),$(ZK_MODULES))
ifneq ($(ZK_UNKNOWN),)
$(error zero-kit has no Module called: $(ZK_UNKNOWN). It has: $(ZK_KNOWN))
endif

# Add the Modules each named Module needs (two levels is enough for now).
ZK_ALL := $(sort $(ZK_MODULES) $(foreach m,$(ZK_MODULES),$(ZK_NEEDS_$(m))))
ZK_ALL := $(sort $(ZK_ALL) $(foreach m,$(ZK_ALL),$(ZK_NEEDS_$(m))))

ZK_SRC := $(foreach m,$(ZK_ALL),$(ZK_DIR)/src/$(m).c)
ZK_HDR := $(foreach m,$(ZK_ALL),$(ZK_DIR)/include/zk/$(m).h)
ZK_CFLAGS := -I$(ZK_DIR)/include
ZK_LDLIBS := $(strip $(foreach m,$(ZK_ALL),$(ZK_LIBS_$(m))))
