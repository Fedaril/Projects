

#pragma once

#include "MicroSDK/Util.h"

#include "malloc.h"
#include "memory.h"
#include "limits.h"



#if defined(_XBOX)
#define GE_PLATFORM_XBOX_360
#else 
#define GE_PLATFORM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

#endif


//======================================================================================
//								   primitive types
//======================================================================================
#ifndef		NULL
#	define	NULL 0L
#endif

#define GETrue	true
#define GEFalse	false

#define GE_INFINITE					0xffffffff
#define GE_INVALID_HANDLE_VALUE		((GEHandle)((GELong_Ptr)-1))
#define GE_POSITION_BEFORE_BEGIN	((GEPosition)(-1))

typedef	void*				GEHandle;
typedef	void*				GEPosition;

typedef bool				GEBool;
typedef size_t				GESize;

typedef long				GELong;
typedef unsigned long		GEULong;

typedef unsigned char		GEU8;
typedef unsigned short		GEU16;
typedef unsigned int		GEU32;
#if defined GE_COMPILER_GCC
typedef uint64_t			GEU64;
#else
typedef unsigned __int64	GEU64;
#endif

typedef signed char			GES8;
typedef signed short		GES16;
typedef signed int			GES32;
#if defined GE_COMPILER_GCC
typedef int64_t				GES64;
#else
typedef signed __int64		GES64;
#endif

typedef GES32				GEInt32;
typedef GES64				GEInt64;

typedef unsigned char		GEByte;
typedef unsigned short		GEWord;
typedef GEU32				GEDWord;
typedef GEU64				GEQWord;

typedef			 int		GEInt;
typedef unsigned int		GEUInt;

typedef unsigned int		GEBitField;

typedef float				GEFloat;
typedef float				GESingle;
typedef double				GEDouble;

typedef	void*				        GEVoidPtr;

typedef	const void*			        GECVoidPtr;

typedef			GEByte*		        GEBytePtr;



typedef	const	GEByte*		GECBytePtr;
typedef			GEWord*		GEWordPtr;
typedef	const	GEWord*		GECWordPtr;
typedef			GEDWord*	GEDWordPtr;
typedef	const	GEDWord*	GECDWordPtr;

typedef char				    GEAnsiChar;
typedef char*				    GEAnsiStr;
typedef const char* 		    GECAnsiStr;

typedef wchar_t				GEWideChar;
typedef wchar_t*			GEWideStr;
typedef const wchar_t*		GECWideStr;

#if defined(GE_UNICODE)
typedef GEWideChar	GETypedChar;
typedef GEWideStr	GETypedStr;
typedef GECWideStr	GECTypedStr;
#else
typedef GEAnsiChar	GETypedChar;
typedef GEAnsiStr	GETypedStr;
typedef GECAnsiStr	GECTypedStr;
#endif

#if defined(GE_PLATFORM_WIN64)
#define GEInt3264	GEInt64

typedef	GES64 GEInt_Ptr;
typedef GEU64 GEUInt_Ptr;

typedef	GES64 GELong_Ptr;
typedef GEU64 GEULong_Ptr;
#elif defined(GE_PLATFORM_PS3)
#define GEInt3264	GEInt32

typedef				int GEInt_Ptr;
typedef unsigned	int GEUInt_Ptr;

typedef				long GELong_Ptr;
typedef unsigned	long GEULong_Ptr;

#ifndef BOOL_DEFINED
#define BOOL_DEFINED
typedef int					BOOL;
typedef BOOL*				PBOOL;
typedef BOOL*				LPBOOL;
#endif
#else
#define GEInt3264	__int32

typedef	_w64			int GEInt_Ptr;
typedef _w64 unsigned	int GEUInt_Ptr;

typedef	_w64			long GELong_Ptr;
typedef _w64 unsigned	long GEULong_Ptr;
#endif

typedef GEULong_Ptr GEDWord_Ptr;

// for windows compatibility 
//	(could probably be moved to the UI)
typedef	GEUInt_Ptr	GEWParam;
typedef	GELong_Ptr	GELParam;
typedef	GELong_Ptr	GELResult;

// for backward compatibility
typedef GES8	GEI8;
typedef GES16	GEI16;
typedef GES32	GEI32;
typedef GES64	GEI64;




////////////////

#define GE_ASSERT(c)		ASSERT(c)

////

#if defined(GE_PLATFORM_WINDOWS) 
#	include<xmmintrin.h>
#	define GE_CACHE_LINE_SIZE	64
#	define GE_PREFETCH( p )		_mm_prefetch( (const char*)( p ), _MM_HINT_T1 )
#elif defined(GE_PLATFORM_XBOX_360) 
#	include <ppcintrinsics.h>
#	define GE_CACHE_LINE_SIZE	128
#	define GE_PREFETCH(p)		__dcbt(0, (const void*)(p))
#elif defined(GE_PLATFORM_PS3) 
#	define GE_CACHE_LINE_SIZE	128
#	define GE_PREFETCH(p)		__dcbt((const void*)(p))
#else
#	error Invalid Platform
#endif


#define GE_MALLOC(s)			malloc(s)
#define GE_FREE(p)				free(p)
#define GE_ALIGNED_MALLOC(s)	malloc(s)
#define GE_ALIGNED_FREE(p)		GE_FREE(p)



#if defined(GE_PLATFORM_PS3)
#	if !defined(__SNC__)
#	   define	GEMemCopy		__builtin_memcpy
#	   define	GEMemMove		__builtin_memmove
#	   define	GEMemSet		__builtin_memset
#	   define	GEMemCompare    __builtin_memcmp
#	else
#	   define	GEMemCopy		memcpy
#	   define	GEMemMove		memmove
#	   define	GEMemSet		memset
#	   define	GEMemCompare    memcmp
#	endif

inline int GEMemCopySafe( void *dest, size_t sizeInBytes, const void *src, size_t count )
{
	if ( dest == NULL || src == NULL )
	{
		return EINVAL;
	}

	if ( sizeInBytes < count )
	{
		return ERANGE;
	}

	GEMemCopy( dest, src, count );

	return 0;
}

#else
// WIN32 & XBOX
#   define	GEMemCopy		memcpy
#   define	GEMemMove		memmove
#   define	GEMemSet		memset
#	define	GEMemCopySafe	memcpy_s
#   define  GEMemCompare    memcmp
#endif // defined(GE_PLATFORM_PS3)

#if defined(GE_PLATFORM_WINDOWS) || defined (GE_PLATFORM_XBOX_360)
#	define GEMemZero(x,s)	ZeroMemory(x,s)
#else
#	define GEMemZero(x,s)	GEMemSet((x),0,(s))
#endif

#define GEMemClearValue(x)	GEMemZero(&(x),sizeof(x))
#define GEMemClearArray(x)	GEMemZero((x),sizeof(x))
#define GEMemClearPtr(x)	GEMemZero((x),sizeof(*(x)))
#define GEMemClear			GEMemClearValue

#if defined( GE_PLATFORM_PS3 )
#	define GE_STACK_ALLOC(s) alloca(s)
#else
#	define GE_STACK_ALLOC(s) _alloca(s)
#endif

#if defined(GE_PLATFORM_WINDOWS) 
#	include<xmmintrin.h>
#	define GE_CACHE_LINE_SIZE	64
#	define GE_PREFETCH( p )		_mm_prefetch( (const char*)( p ), _MM_HINT_T1 )
#elif defined(GE_PLATFORM_XBOX_360) 
#	define GE_CACHE_LINE_SIZE	128
#	define GE_PREFETCH(p)		__dcbt(0, (const void*)(p))
#elif defined(GE_PLATFORM_PS3) 
#	define GE_CACHE_LINE_SIZE	128
#	define GE_PREFETCH(p)		__dcbt((const void*)(p))
#endif

#define GE_FORCE_INLINE			__forceinline





GE_FORCE_INLINE GEUInt OneShl8(GEUInt a_iShift)
{
/*
GEU64	t0	= -(a_u32Shift & 1) ^ 0x55555555;
GEU64	t1	= -((a_u32Shift >> 1) & 1) ^ 0x33333333;
GEU64	t2	= -((a_u32Shift >> 2) & 1) ^ 0x0f0f0f0f;
GEU64	t3	= -((a_u32Shift >> 3) & 1) ^ 0x00ff00ff;
GEU64	t4	= -((a_u32Shift >> 4) & 1) ^ 0x0000ffff;
return (GEU32)(((t0 & t1) & (t2 & t3)) & t4);
*/
	GE_ASSERT(a_iShift < 8);

	GEInt iShift = a_iShift;
	GEI64	t0	= -((iShift >> 0) & 1) ^ 0x00000055;
	GEI64	t1	= -((iShift >> 1) & 1) ^ 0x00000033;
	GEI64	t2	= -((iShift >> 2) & 1) ^ 0x0000000f;
	return (GEUInt)((t0 & t1) & t2);
}



template<typename TYPE> 
GE_FORCE_INLINE TYPE GEIntegerBranchFreeMin(TYPE a, TYPE b)
{ 
	register GES64 wa = (GES64)a;
	register GES64 wb = (GES64)b;

	const GES64 mask  = (wa - wb) >> ((sizeof(GES64) * CHAR_BIT) - 1);
	return (TYPE)(wb ^ ((wa ^ wb) & mask));
}

template<typename TYPE> 
GE_FORCE_INLINE TYPE GEIntegerBranchFreeMax(TYPE a, TYPE b)
{ 
	register GES64 wa = (GES64)a;
	register GES64 wb = (GES64)b;

	const GES64 mask  = (wa - wb) >> ((sizeof(GES64) * CHAR_BIT) - 1);
	return (TYPE)(wa ^ ((wa ^ wb) & mask));
}

template<typename TYPE> 
GE_FORCE_INLINE TYPE GEMin( TYPE a, TYPE b) { return GEIntegerBranchFreeMin(a, b); }

template<typename TYPE> 
GE_FORCE_INLINE TYPE GEMax( TYPE a, TYPE b) { return GEIntegerBranchFreeMax(a, b); }

template<typename TYPE> 
GE_FORCE_INLINE TYPE GEClamp( TYPE x, TYPE min, TYPE max) { return GEMax( min, GEMin( max, x ) ); }

// X360 and PS3 use a different name for the 32-bit float version of __fsel
#	if defined (GE_PLATFORM_PS3)
GE_FORCE_INLINE float __fself( float a_fTest, float a_fValue1, float a_fValue2 ) { return __fsels( a_fTest, a_fValue1, a_fValue2 ); }
#	endif

#if defined( GE_PLATFORM_WINDOWS) 
// use float intrinsic fsel for float datatypes
template<>
GE_FORCE_INLINE GEFloat  GEMin( GEFloat  a, GEFloat  b ) { return a < b ? a : b; }
template<>
GE_FORCE_INLINE GEFloat  GEMax( GEFloat  a, GEFloat  b ) { return a < b ? b : a; }
template<>
GE_FORCE_INLINE GEDouble GEMin( GEDouble a, GEDouble b ) { return a < b ? a : b; }
template<>
GE_FORCE_INLINE GEDouble GEMax( GEDouble a, GEDouble b ) { return a < b ? b : a; }

#else
// use float intrinsic fsel for float datatypes
template<>
GE_FORCE_INLINE GEFloat  GEMin( GEFloat  a, GEFloat  b ) { return __fself( a - b, b, a ); }
template<>
GE_FORCE_INLINE GEFloat  GEMax( GEFloat  a, GEFloat  b ) { return __fself( a - b, a, b ); }
template<>
GE_FORCE_INLINE GEDouble GEMin( GEDouble a, GEDouble b ) { return __fsel ( a - b, b, a ); }
template<>
GE_FORCE_INLINE GEDouble GEMax( GEDouble a, GEDouble b ) { return __fsel ( a - b, a, b ); }
#endif

// to prevent a possible overflow, 64 bits integer types still use a branch
template<>
GE_FORCE_INLINE GEU64 GEMin( GEU64 a, GEU64 b ) { return (a < b) ? a : b; }
template<>
GE_FORCE_INLINE GEU64 GEMax( GEU64 a, GEU64 b ) { return (a < b) ? b : a; }
template<>
GE_FORCE_INLINE GES64 GEMin( GES64 a, GES64 b ) { return (a < b) ? a : b; }
template<>
GE_FORCE_INLINE GES64 GEMax( GES64 a, GES64 b ) { return (a < b) ? b : a; }


#define	GE_MIN( a, b ) GEMin( a, b )
#define	GE_MAX( a, b ) GEMax( a, b )
#define	GE_CLAMP( x, min, max ) GEClamp( x, min, max)

#define GE_UNUSED(v)			((void)v)