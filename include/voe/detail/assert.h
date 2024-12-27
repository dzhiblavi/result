#include <exception>  // IWYU pragma: keep

#define VOE_ASSERT(cond, msg)                                                           \
    do {                                                                                \
        if (!static_cast<bool>(cond)) [[unlikely]] {                                    \
            std::cerr << "Check failed: " #cond " is not satisfied: " msg << std::endl; \
            std::terminate();                                                           \
        }                                                                               \
    } while (0)
