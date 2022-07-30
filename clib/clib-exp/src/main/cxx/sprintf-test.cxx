#include <gtest/gtest.h>

#include <stdlib.h>


/**
 * Tests zero termination in snprintf when there is not enough space in buffer
 */
TEST(snprintfTest_shortTerminateZero, BasicAssertions) {
	char buf[4] = { 'a', 'b', 'c', 'd' };
	snprintf(buf, sizeof(buf), "%d", 1234);

	EXPECT_STREQ(buf, "123");
}

/**
 * Tests zero termination in vsnprintf when there is not enough space in buffer
 */
TEST(vsnprintfTest_shortTerminateZero, BasicAssertions) {
	struct X  {
		static size_t ff(char *buf, size_t size, const char *format, ...) {
			va_list args;
			va_start(args, format);
			size_t result = vsnprintf(buf, size, format, args);
			va_end(args);
			return result;
		}
	};

	char buf[4] = { 'a', 'b', 'c', 'd' };
	X::ff(buf, sizeof(buf), "%d", 1234);

	EXPECT_STREQ(buf, "123");
}
