# Root of the repository (auto-detected)
ROOT_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

# Load personalizable settings from .env
-include .env

export MVS_HOST MVS_PORT MVS_USER MVS_PASS
export MVS_JOBCLASS MVS_MSGCLASS

# Tools path (mvsasm etc.)
export PATH := $(ROOT_DIR)scripts:$(PATH)

# Cross-compiler
CC       := c2asm370
CFLAGS   := -fverbose-asm -S -DLUA_USE_C89 -DLUA_USE_JUMPTABLE=0

# Include paths
INC_DIR  := $(ROOT_DIR)include
INC1     := $(ROOT_DIR)contrib/crent370_sdk/inc
INCS     := -I$(INC_DIR) -I$(INC1)

CFLAGS   += $(INCS)

# Map .env variables to mvsasm exports
export MAC1=$(CRENT_MACLIB)
export MVSASM_PUNCH=$(LUA_PUNCH)
export MVSASM_SYSLMOD=$(LUA_SYSLMOD)

# Warning collection file
export BUILD_WARNINGS := $(ROOT_DIR).build-warnings
