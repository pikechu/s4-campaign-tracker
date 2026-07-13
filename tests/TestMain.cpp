#include <exception>
#include <iostream>

int RunLoggerTests();
int RunModuleInventoryTests();

int main() {
    try {
        RunLoggerTests();
        return RunModuleInventoryTests();
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
