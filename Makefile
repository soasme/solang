DIRS=build
$(info $(shell mkdir -p $(DIRS)))

.PHONY: all clean test

lexer_test: src/lexer.o src/lexer_test.o
	$(CC) $(CFLAGS) -o build/lexer_test $?

src/parser.c: src/packcc
	./src/packcc ./src/parser.peg

parser_test: src/lexer.o src/parser.o src/parser_test.o
	$(CC) $(CFLAGS) -o build/parser_test $?

test: lexer_test parser_test
	$(foreach file, $(wildcard build/*_test), $(file))
		$(info $(file))
	$(endfor)

clean:
	rm -rf build
	rm -rf src/parser.c
	rm -rf src/*.o
	mkdir -p $(DIRS)
