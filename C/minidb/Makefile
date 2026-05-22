CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -Iinclude
SRCS    = src/btree.c src/avl.c src/hashmap.c src/db.c src/main.c
TARGET  = minidb

all: $(TARGET)

$(TARGET): $(SRCS) include/minidb.h
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

csv:
	python3 gen_csv.py 50000 > data/users.csv
	@echo "CSV gerado: $$(wc -l < data/users.csv) linhas"

run: all csv
	./$(TARGET) data/users.csv

clean:
	rm -f $(TARGET)

.PHONY: all csv run clean
