CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Isrc
TARGET = bin/toolkit.exe

SRCS = src/main.c \
       src/common/common.c \
       src/lexical_analyzer/lexical_analyzer.c \
       src/symbol_table/symbol_table.c \
       src/two_pass_assembler/assembler.c \
       src/two_pass_assembler/pass1.c \
       src/two_pass_assembler/pass2.c \
       src/macro_processor/macro_processor.c \
       src/linker_loader/linker_loader.c \
       src/recursive_descent_parser/parser.c \
       src/quadruple_generator/quadruple.c \
       src/code_optimizer/optimizer.c

all: $(TARGET)

$(TARGET): $(SRCS)
	@if not exist bin mkdir bin
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	@if exist bin rmdir /s /q bin

.PHONY: all clean
