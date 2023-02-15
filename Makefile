DIRS=build
$(info $(shell mkdir -p $(DIRS)))

.PHONY: all clean test

src/grammar.c: src/packcc src/grammar.peg
	cd src && ./packcc grammar.peg

lexer_test: src/lexer.o src/lexer_test.o
	$(CC) $(CFLAGS) -o build/lexer_test $?

grammar_test: src/utils.o src/parser.o src/grammar.o src/grammar_test.o
	$(CC) $(CFLAGS) -o build/grammar_test $?

test: lexer_test grammar_test
	$(foreach file, $(wildcard build/*_test), $(file))
		$(info $(file))
	$(endfor)

clean:
	rm -rf build
	rm -rf src/parser.c
	rm -rf src/parser.h
	rm -rf src/*.o
	mkdir -p $(DIRS)
