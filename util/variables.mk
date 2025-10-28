BUILD     := build
OBJ_DIR   := $(BUILD)/obj/
LDFLAGS   := $(LDFLAGS) -T linker.ld

C_SRCS    := $(shell find src -type f -name '*.c')
A_SRCS    := $(shell find src -type f -name '*.asm')

OBJS       := $(addprefix $(OBJ_DIR), $(C_SRCS:.c=.o)) $(addprefix $(OBJ_DIR), $(A_SRCS:.asm=.o))