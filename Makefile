lexer_test:
	$(CC) $(CFLAGS) -DLEXER_TEST -o src/lexer_test src/lexer.c
	./src/lexer_test

lexer_example:
	$(CC) $(CFLAGS) -DLEXER_TEST -o src/lexer_example src/lexer.c
	./src/lexer_example
