#ifndef __DAP_UTILITY_H__
#define __DAP_UTILITY_H__

#include <stdint.h>

#ifndef   __STATIC_FORCEINLINE
  #define __STATIC_FORCEINLINE                   static inline __attribute__((always_inline))
#endif
#ifndef   __STATIC_INLINE
  #define __STATIC_INLINE                        static inline __attribute__((always_inline))
#endif
#ifndef   __WEAK
  #define __WEAK                                 __attribute__((weak))
#endif


extern const uint8_t kParityByteTable[256];

__STATIC_FORCEINLINE uint8_t ParityEvenUint32(uint32_t v)
{
    v ^= v >> 16;
    v ^= v >> 8;
    v ^= v >> 4;
    v &= 0xf;
    return (0x6996 >> v) & 1;
}

__STATIC_FORCEINLINE uint8_t ParityEvenUint8(uint8_t v)
{
    return kParityByteTable[v];
}

#endif
