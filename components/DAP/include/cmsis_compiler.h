#ifndef __CMSIS_COMPILER_H__
#define __CMSIS_COMPILER_H__

#ifndef   __STATIC_FORCEINLINE
  #define __STATIC_FORCEINLINE                   static inline __attribute__((always_inline)) 
#endif
#ifndef   __STATIC_INLINE
  #define __STATIC_INLINE                        static inline __attribute__((always_inline))
#endif
#ifndef   __WEAK
  #define __WEAK                                 __attribute__((weak))
#endif


#endif