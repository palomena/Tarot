# GNU Makefile

# Build variables
CC := gcc
SOURCE_DIRECTORY := src
BUILD_DIRECTORY  := build
EXECUTABLE := ${BUILD_DIRECTORY}/pentagram
CFLAGS :=                 \
	-std=c90              \
	-pedantic             \
	-Wall                 \
	-Wextra               \
	-Wno-unused-value     \
	-I${SOURCE_DIRECTORY} \
	-DTAROT_BITS=$(BITS)

ifndef BACKEND
	BACKEND := default
endif

# Build setup
SOURCE_FILES := ${shell find ${SOURCE_DIRECTORY} -name "*.c"}
ifeq ($(BACKEND), gmp)
    SOURCE_FILES := $(filter-out %mini-gmp.c %mini-mpq.c, $(SOURCE_FILES))
    CFLAGS += -DTAROT_BACKEND=2
    LINK := -lgmp
else
    CFLAGS += -DTAROT_BACKEND=1
    LINK :=
endif
OBJECT_FILES := ${SOURCE_FILES:${SOURCE_DIRECTORY}/%=${BUILD_DIRECTORY}/%.o}

# Target-specific build config
# Unfortunately -fno-builtin is required if no hosted environment is avail
# as the gcc optimizer replaces the tarot memset function body with a call
# to tarot memset leading to infinite recursion.
release: CFLAGS += -O3 -fhosted -flto=auto -DNDEBUG -s -fno-builtin
debug: CFLAGS +=         \
	-DDEBUG              \
	-fhosted             \
	-ggdb                \
	-O0                  \
	-fsanitize=address   \
	-fsanitize=leak      \
	-fsanitize=undefined \
	-g3                  \
	-ftrapv

# Build targets
all:     ${EXECUTABLE}
debug:   ${EXECUTABLE}
release: ${EXECUTABLE}

${EXECUTABLE}: ${OBJECT_FILES}
	${CC} ${CFLAGS} ${OBJECT_FILES} ${LINK} -o ${EXECUTABLE}

${BUILD_DIRECTORY}/%.c.o: ${SOURCE_DIRECTORY}/%.c
	mkdir -p ${dir $@}
	${CC} ${CFLAGS} -xc -c $< -o $@

# Less compiler warnings for the math modules
${BUILD_DIRECTORY}/math/%.c.o: ${SOURCE_DIRECTORY}/math/%.c
	mkdir -p ${dir $@}
	${CC} -xc -std=c90 -pedantic -c $< -o $@

.PHONY: clean
clean:
	rm -rv ${BUILD_DIRECTORY}
