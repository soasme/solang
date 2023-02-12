DIRS=build
$(info $(shell mkdir -p $(DIRS)))

lexer_test:
	$(CC) $(CFLAGS) -o build/lexer_test src/lexer.c src/lexer_test.c

test: lexer_test # for all files in build matching *_test pattern, execute those files.
	$(foreach file, $(wildcard build/*_test), $(file))
		$(info $(file))
	$(endfor)
