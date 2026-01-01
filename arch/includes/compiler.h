#ifndef _COMPILER_H_
#define _COMPILER_H_

#ifndef likely
#define likely(x)   __builtin_expect((long)!!(x), 1L)
#endif

#ifndef unlikely
#define unlikely(x) __builtin_expect((long)!!(x), 0L)
#endif

#ifdef __GNUC__

#include <sys/cdefs.h>

#define __inline	inline
#define __inline__	inline

#ifdef __always_inline
#undef __always_inline	/* already defined in <sys/cdefs.h> */
#define __always_inline	inline __attribute__((always_inline))
#endif

#ifndef __noinline
#define __noinline	__attribute__((__noinline__))
#endif

#ifndef __packed
#define __packed	__attribute__((__packed__))
#endif

#ifndef __asm
#define __asm		asm
#endif

#ifndef __weak
#define __weak		__attribute__((weak))
#endif

#ifndef __maybe_unused
#define __maybe_unused		__attribute__((unused))
#endif

#else
#error "Compiler not supported."
#endif

#endif /* _COMPILER_H_ */