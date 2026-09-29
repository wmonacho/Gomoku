#include "support/TestRunner.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <thread>
#include <unistd.h>

namespace {

    int  failureCount = 0;
    int  testCount = 0;
    int  passedTestCount = 0;
    long allocations = 0;

    // Couleurs et animations seulement dans un vrai terminal : la sortie reste lisible dans un fichier ou une CI.
    const bool fancyOutput = isatty(STDOUT_FILENO) && std::getenv("NO_COLOR") == nullptr;

    const char* const RESET = "\033[0m";
    const char* const BOLD = "\033[1m";
    const char* const DIM = "\033[2m";
    const char* const RED = "\033[31m";
    const char* const GREEN = "\033[32m";
    const char* const CYAN = "\033[36m";

    const int SPINNER_FRAME_MS = 25;
    const int PROGRESS_BAR_WIDTH = 30;
    const int PROGRESS_BAR_STEP_MS = 12;

    const char* style(const char* ansiCode) {
        return fancyOutput ? ansiCode : "";
    }

    void pause(int milliseconds) {
        std::fflush(stdout);
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    void animateSpinner(const char* name) {
        if (!fancyOutput)
            return;
        const char* const frames[] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
        for (const char* frame : frames) {
            std::printf("\r  %s%s%s %s", CYAN, frame, RESET, name);
            pause(SPINNER_FRAME_MS);
        }
        std::printf("\r\033[K");
    }

    void animateProgressBar() {
        if (!fancyOutput)
            return;
        const char* color = failureCount == 0 ? GREEN : RED;
        for (int filled = 0; filled <= PROGRESS_BAR_WIDTH; filled++) {
            std::printf("\r  %s", color);
            for (int i = 0; i < PROGRESS_BAR_WIDTH; i++)
                std::printf("%s", i < filled ? "█" : "░");
            std::printf("%s", RESET);
            pause(PROGRESS_BAR_STEP_MS);
        }
        std::printf("\n");
    }
}

namespace TestRunner {

void printTitle(const char* title) {
    std::printf("\n  %s%s%s%s\n\n", style(BOLD), style(CYAN), title, style(RESET));
}

void run(const char* name, void (*test)()) {
    animateSpinner(name);

    const int  failuresBefore = failureCount;
    const auto start = std::chrono::steady_clock::now();
    test();
    const double elapsedMs =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();

    const bool passed = failureCount == failuresBefore;
    testCount++;
    passedTestCount += passed;
    std::printf("  %s%s%s %s %s(%.1f ms)%s\n", style(passed ? GREEN : RED), passed ? "✓" : "✗",
                style(RESET), name, style(DIM), elapsedMs, style(RESET));
}

int finish() {
    std::printf("\n");
    animateProgressBar();
    const bool allPassed = failureCount == 0;
    std::printf("  %s%s%d/%d tests réussis%s\n\n", style(BOLD), style(allPassed ? GREEN : RED),
                passedTestCount, testCount, style(RESET));
    return allPassed ? 0 : 1;
}

void reportFailure(const char* file, int line, const char* condition) {
    std::printf("    %s%s:%d : %s%s\n", style(RED), file, line, condition, style(RESET));
    failureCount++;
}

long allocationCount() {
    return allocations;
}

}  // namespace TestRunner

void* operator new(std::size_t size) {
    allocations++;
    void* memory = std::malloc(size);
    if (memory == nullptr)
        throw std::bad_alloc();
    return memory;
}

void operator delete(void* memory) noexcept {
    std::free(memory);
}

void operator delete(void* memory, std::size_t) noexcept {
    std::free(memory);
}
