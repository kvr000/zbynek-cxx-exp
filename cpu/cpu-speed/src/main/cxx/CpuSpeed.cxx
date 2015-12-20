#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/resource.h>

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

using namespace std;

void runDepend64(long loops)
{
#define DEPEND64_DECL \
	int64_t r0;
#define DEPEND64_MODIFIED \
	"=r"(r0)

#if (defined __x86_64__)
#	define DEPEND64_STEP \
				"inc %0\n" \
				"inc %0\n" \
				"inc %0\n" \
				"inc %0\n" \
				"inc %0\n" \
				"inc %0\n" \
				"inc %0\n" \
				"inc %0\n" \
				"inc %0\n" \
				"inc %0\n"
#elif (defined __aarch64__)
#	define DEPEND64_STEP \
				"add %w0, %w0, 1\n" \
				"add %w0, %w0, 1\n" \
				"add %w0, %w0, 1\n" \
				"add %w0, %w0, 1\n" \
				"add %w0, %w0, 1\n" \
				"add %w0, %w0, 1\n" \
				"add %w0, %w0, 1\n" \
				"add %w0, %w0, 1\n" \
				"add %w0, %w0, 1\n" \
				"add %w0, %w0, 1\n"
#else
# error Unsupported platform
#endif

	for (long i = 0; i < loops; ++i) {
		DEPEND64_DECL;
		__asm__ __volatile(
				DEPEND64_STEP
				DEPEND64_STEP
				DEPEND64_STEP
				DEPEND64_STEP
				DEPEND64_STEP
				DEPEND64_STEP
				DEPEND64_STEP
				DEPEND64_STEP
				DEPEND64_STEP
				DEPEND64_STEP
				: DEPEND64_MODIFIED
				:
				: "memory"
				);
	}
}

void runIncAln32(long loops)
{
#define INC32_DECL \
	int32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
#define INC32_MODIFIED \
	"=r"(r0), "=r"(r1), "=r"(r2), "=r"(r3), "=r"(r4), "=r"(r5), "=r"(r6), "=r"(r7), "=r"(r8), "=r"(r9)

#if (defined __x86_64__)
#	define INC32_STEP \
				"inc %0\n" \
				"inc %1\n" \
				"inc %2\n" \
				"inc %3\n" \
				"inc %4\n" \
				"inc %5\n" \
				"inc %6\n" \
				"inc %7\n" \
				"inc %8\n" \
				"inc %9\n"
#elif (defined __aarch64__)
#	define INC32_STEP \
				"add %w0, %w0, 1\n" \
				"add %w1, %w1, 1\n" \
				"add %w2, %w2, 1\n" \
				"add %w3, %w3, 1\n" \
				"add %w4, %w4, 1\n" \
				"add %w5, %w5, 1\n" \
				"add %w6, %w6, 1\n" \
				"add %w7, %w7, 1\n" \
				"add %w8, %w8, 1\n" \
				"add %w9, %w9, 1\n"
#else
# error Unsupported platform
#endif

	for (long i = 0; i < loops; ++i) {
		INC32_DECL;
		__asm__ __volatile(
				INC32_STEP
				INC32_STEP
				INC32_STEP
				INC32_STEP
				INC32_STEP
				INC32_STEP
				INC32_STEP
				INC32_STEP
				INC32_STEP
				INC32_STEP
				: INC32_MODIFIED
				:
				: "memory"
				);
	}
}

void runIncAln64(long loops)
{
#define INC64_DECL \
	int64_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
#define INC64_MODIFIED \
	"=r"(r0), "=r"(r1), "=r"(r2), "=r"(r3), "=r"(r4), "=r"(r5), "=r"(r6), "=r"(r7), "=r"(r8), "=r"(r9)

#if (defined __x86_64__)
#	define INC64_STEP \
				"inc %0\n" \
				"inc %1\n" \
				"inc %2\n" \
				"inc %3\n" \
				"inc %4\n" \
				"inc %5\n" \
				"inc %6\n" \
				"inc %7\n" \
				"inc %8\n" \
				"inc %9\n"
#elif (defined __aarch64__)
#	define INC64_STEP \
				"add %0, %0, 1\n" \
				"add %1, %1, 1\n" \
				"add %2, %2, 1\n" \
				"add %3, %3, 1\n" \
				"add %4, %4, 1\n" \
				"add %5, %5, 1\n" \
				"add %6, %6, 1\n" \
				"add %7, %7, 1\n" \
				"add %8, %8, 1\n" \
				"add %9, %9, 1\n"
#else
# error Unsupported platform
#endif
	for (long i = 0; i < loops; ++i) {
		INC64_DECL;
		__asm__ __volatile(
				INC64_STEP
				INC64_STEP
				INC64_STEP
				INC64_STEP
				INC64_STEP
				INC64_STEP
				INC64_STEP
				INC64_STEP
				INC64_STEP
				INC64_STEP
				: INC64_MODIFIED
				:
				: "memory"
				);
	}
}

void runIncMov32(long loops)
{
#define INCMOV32_DECL \
	int32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
#define INCMOV32_MODIFIED \
	"=r"(r0), "=r"(r1), "=r"(r2), "=r"(r3), "=r"(r4), "=r"(r5), "=r"(r6), "=r"(r7), "=r"(r8), "=r"(r9)

#if (defined __x86_64__)
#	define INCMOV32_STEP \
				"inc %0\n" \
				"mov %1, %2\n" \
				"inc %3\n" \
				"mov %4, %5\n" \
				"inc %6\n" \
				"mov %7, %8\n" \
				"inc %9\n" \
				"mov %0, %1\n" \
				"inc %2\n" \
				"mov %3, %4\n"
#elif (defined __aarch64__)
#	define INCMOV32_STEP \
				"add %w0, %w0, 1\n" \
				"mov %w1, %w2\n" \
				"add %w3, %w3, 1\n" \
				"mov %w4, %w5\n" \
				"add %w6, %w6, 1\n" \
				"mov %w7, %w8\n" \
				"add %w9, %w9, 1\n"
				"mov %w0, %w1\n" \
				"add %w2, %w2, 1\n"
				"mov %w3, %w4\n"
#else
# error Unsupported platform
#endif

	for (long i = 0; i < loops; ++i) {
		INCMOV32_DECL;
		__asm__ __volatile(
				INCMOV32_STEP
				INCMOV32_STEP
				INCMOV32_STEP
				INCMOV32_STEP
				INCMOV32_STEP
				INCMOV32_STEP
				INCMOV32_STEP
				INCMOV32_STEP
				INCMOV32_STEP
				INCMOV32_STEP
				: INCMOV32_MODIFIED
				:
				: "memory"
				);
	}
}

void runIncMov64(long loops)
{
#define INCMOV64_DECL \
	int64_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
#define INCMOV64_MODIFIED \
	"=r"(r0), "=r"(r1), "=r"(r2), "=r"(r3), "=r"(r4), "=r"(r5), "=r"(r6), "=r"(r7), "=r"(r8), "=r"(r9)

#if (defined __x86_64__)
#	define INCMOV64_STEP \
				"inc %0\n" \
				"mov %1, %2\n" \
				"inc %3\n" \
				"mov %4, %5\n" \
				"inc %6\n" \
				"mov %7, %8\n" \
				"inc %9\n" \
				"mov %0, %1\n" \
				"inc %2\n" \
				"mov %3, %4\n"
#elif (defined __aarch64__)
#	define INCMOV64_STEP \
				"add %0, %0, 1\n" \
				"mov %1, %2\n" \
				"add %3, %3, 1\n" \
				"mov %4, %5\n" \
				"add %6, %6, 1\n" \
				"mov %7, %8\n" \
				"add %9, %9, 1\n"
				"mov %0, %1\n" \
				"add %2, %2, 1\n"
				"mov %3, %4\n"
#else
# error Unsupported platform
#endif

	for (long i = 0; i < loops; ++i) {
		INCMOV64_DECL;
		__asm__ __volatile(
				INCMOV64_STEP
				INCMOV64_STEP
				INCMOV64_STEP
				INCMOV64_STEP
				INCMOV64_STEP
				INCMOV64_STEP
				INCMOV64_STEP
				INCMOV64_STEP
				INCMOV64_STEP
				INCMOV64_STEP
				: INCMOV64_MODIFIED
				:
				: "memory"
				);
	}
}

string pipeExec(const char *command)
{
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
	if (!pipe) {
		throw std::runtime_error(string("popen() failed: ")+strerror(errno));
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	return result;
}

double getCpuClock()
{
#if (defined __DARWIN_C_LEVEL)
	string output = pipeExec("sysctl hw.cpufrequency");
	for (const char *c = output.c_str(); *c; ++c) {
		if (isdigit(*c)) {
			return strtod(c, NULL);
		}
	}
	throw std::runtime_error(string("Cannot read frequency from: ")+output);
#elif (defined __linux__)
	string output = pipeExec("lscpu | grep '^CPU max MHz:'");
	for (const char *c = output.c_str(); *c; ++c) {
		if (isdigit(*c)) {
			return strtod(c, NULL)*1000000;
		}
	}
	try {
		throw std::runtime_error(string("Cannot read frequency from: ")+output);
	}
	catch (std::runtime_error &ex) {
		std::cerr << ex.what() << "\n";
		// let keep something reasonable here for modern CPU:
		return 2500000000.0;
	}
#else
# error Unsupported platform
#endif
}

void runTickBenchmark(const char *name, long batchSize, void (*benchmark)())
{
	struct rusage start, end;
	getrusage(RUSAGE_SELF, &start);
	benchmark();
	getrusage(RUSAGE_SELF, &end);
	double clock = getCpuClock();
	printf("%s: per-second=%.6f per-tick=%.3f\n", name, batchSize/((end.ru_utime.tv_sec+end.ru_utime.tv_usec/1000000.0)-(start.ru_utime.tv_sec+start.ru_utime.tv_usec/1000000.0)), batchSize/((end.ru_utime.tv_sec+end.ru_utime.tv_usec/1000000.0)-(start.ru_utime.tv_sec+start.ru_utime.tv_usec/1000000.0))/clock);
}

int main(void)
{
	// warmup
	runIncAln64(200000000L);

	runTickBenchmark("runDepend64", 1000000000L*102, [](){ runDepend64(1000000000L); });
	runTickBenchmark("runIncAln32", 1000000000L*102, [](){ runIncAln32(1000000000L); });
	runTickBenchmark("runIncAln64", 1000000000L*102, [](){ runIncAln64(1000000000L); });
	runTickBenchmark("runIncMov32", 1000000000L*102, [](){ runIncMov32(1000000000L); });
	runTickBenchmark("runIncMov64", 1000000000L*102, [](){ runIncMov64(1000000000L); });
	return 0;
}
