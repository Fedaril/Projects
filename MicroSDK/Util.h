
#pragma once

#include "intrin.h"

namespace Util
{

//
void Halt();
void PrintMessage(const char* a_szMessage, ...);

//
void ReadFile(const char* a_szFile, void** a_ppBuffer, unsigned int* a_pBufferSize);
void FreeReadBuffer(void* a_pBuffer);

void SaveFile(const char* a_szFile, const void* a_pBuffer, unsigned int a_iBufferSize);
void AppendFile(const char* a_szFile, const void* a_pBuffer, unsigned int a_iBufferSize);

template<typename T> T EndianSwapValue(T v);
template<typename T> T* EndianSwapPointer(T* v);

template<typename T> T Abs(T v);
template<typename T> T Min(T v1, T v2);
template<typename T> T Max(T v1, T v2);
template<typename T> T Clamp(T v, T min, T max);

unsigned int CountLeadingZeros(unsigned int a_iValue);
unsigned int CountBitSet(unsigned int a_iValue);

} // namespace Util

#define HALT_WITH_MSG(msg, ...)					do { Util::PrintMessage(msg, __VA_ARGS__); Util::Halt(); } while(false)
#define HALT_IF(cond)							do { if (cond) Util::Halt(); } while (false)
#define HALT_IF_FAILED(hr)						do { if (FAILED(hr)) Util::Halt(); } while(false)
#define HALT_WITH_MSG_IF_FAILED(msg, ...)		do { if (FAILED(hr)) HALT_WITH_MSG(msg, __VA_ARGS__); } while(false)



template<typename T>
inline T Util::EndianSwapValue(T v)
{
	char* pBuffer = (char*)&v;
	for (unsigned int iByteIndex = 0; iByteIndex < sizeof(T) / 2; ++iByteIndex)
	{
		char iTemp = pBuffer[iByteIndex];
		pBuffer[iByteIndex] = pBuffer[sizeof(T) - 1 - iByteIndex];
		pBuffer[sizeof(T) - 1 - iByteIndex] = iTemp;
	}

	return v;
}
template<typename T>
inline T* Util::EndianSwapPointer(T* v)
{
	int iPtrValue = (int)T;
	EndianSwapValue(iPtrValue);
	return (T*)iPtrValue;
}

template<typename T> 
inline T Util::Abs(T v)
{
	return (v < (T)0) ? -v : v;
}


template<typename T> 
inline T Util::Min(T v1, T v2)
{
	return (v1 < v2) ? v1 : v2;
}

template<typename T> 
inline T Util::Max(T v1, T v2)
{
	return (v1 > v2) ? v1 : v2;
}


template<typename T> 
inline T Util::Clamp(T v, T min, T max)
{
	return Min(Max(v, min), max);
}

inline unsigned int Util::CountLeadingZeros(unsigned int a_iValue)
{
	unsigned long iIndex;
	_BitScanReverse(&iIndex, a_iValue);
	return 31 - iIndex;
}

inline unsigned int Util::CountBitSet(unsigned int a_iValue)
{
	a_iValue = a_iValue - ((a_iValue >> 1) & 0x55555555);
	a_iValue = (a_iValue & 0x33333333) + ((a_iValue >> 2) & 0x33333333);
	return ((a_iValue + (a_iValue >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
}



#if !defined( NDEBUG )
#define ASSERT(x) do { if (!(x)) { HALT_WITH_MSG("[%s:%d] Assertion Failed: %s\n", __FILE__, __LINE__, #x); } } while (false)
#else
#define ASSERT(x) do { __noop(x); } while (false)
#endif
