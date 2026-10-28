#pragma once

#if defined(_MSC_VER)
    #define IWA_FORCEINLINE __forceinline
#elif defined(__clang__) || defined(__GNUC__)
    #define IWA_FORCEINLINE inline __attribute__((always_inline))
#else
    #define IWA_FORCEINLINE inline
#endif
