/* test.h -- minimal unit testing
 * Olaf Bernstein <camel-cdr@protonmail.com>
 * Distributed under the MIT license, see license at the end of the file.
 * New versions available at https://github.com/camel-cdr/cauldron
 */

#ifndef TEST_H_INCLUDED

#include <stdlib.h>

static unsigned test__nasserts, test__nfailures;

#define TEST_BEGIN(msg) \
	test__nasserts = test__nfailures = 0; \
	fputs("Testing ", stdout); \
	printf msg; \
	fputs(" ...", stdout);

#define TEST_END() \
	if (test__nfailures == 0) { \
		puts("PASSED"); \
	} else { \
		printf("\t-> %u assertions, %u failures\n", \
			test__nasserts, test__nfailures); \
		exit(1); \
	}

#define TEST_ASSERT(cnd) TEST_ASSERT_MSG((cnd), (#cnd))
#define TEST_ASSERT_MSG(cnd, msg) \
	do { \
		if (!(cnd)) { \
			if (test__nfailures++ == 0) puts("FAILED"); \
			printf("\t%s:%d:\n", __FILE__, __LINE__); \
			printf msg; \
			putchar('\n'); \
		} \
		++test__nasserts; \
	} while (0)

#define TEST_H_INCLUDED
#endif
