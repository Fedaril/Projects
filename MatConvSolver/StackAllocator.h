

#pragma once

#include <vector>

template<typename T>
class StackAllocatorArray
{
private:
	friend class StackAllocator;

	unsigned int	m_iCapacity;
	unsigned int	m_iCount;
	T*				m_pData;

public:
	inline		T&						operator[](unsigned int a_iIndex)						{ ASSERT(a_iIndex < m_iCount); return m_pData[a_iIndex]; }
	inline		const T&				operator[](unsigned int a_iIndex) const					{ ASSERT(a_iIndex < m_iCount); return m_pData[a_iIndex]; }

	inline		unsigned int			GetCount() const										{ return m_iCount; };
	inline		unsigned int			GetCapacity() const										{ return m_iCapacity; };

	inline		void					PushBack(T& a_rElement)									{ ASSERT(m_iCount < m_iCapacity); m_pData[m_iCount] = a_rElement; m_iCount++; }
	inline		void					SetCount(unsigned int a_iNewCount)						{ ASSERT(a_iNewCount <= m_iCapacity); m_iCount = a_iNewCount; }

public:
										StackAllocatorArray()									{ m_iCapacity = 0; m_iCount = 0; m_pData = NULL; }
										~StackAllocatorArray()									{ m_iCapacity = 0; m_iCount = 0; m_pData = NULL; }
	
};


class StackAllocator
{
private:
	std::vector<void*>		m_arrSegment;
	unsigned int			m_iSegmentSize;
	unsigned int			m_iSegmentOffset;


public:
											StackAllocator();
											~StackAllocator();

		void								Initialize(unsigned int a_iSegmentSize);
		void								Shutdown();


		template<typename T>
		void								AllocateArray(unsigned int a_iSize, StackAllocatorArray<T>* a_pArray);

private:
		void				CreateNewSegment();
};

template<typename T>
void StackAllocator::AllocateArray(unsigned int a_iCapacity, StackAllocatorArray<T>* a_pArray)
{
	unsigned int iElementSize	= sizeof(T);
	unsigned int iArraySize		= iElementSize * a_iCapacity;

	if ((m_iSegmentOffset + iArraySize + sizeof(T)) > m_iSegmentSize)
		CreateNewSegment();

	unsigned int iAlignement = ((iElementSize + 3) / 4) * 4;
	m_iSegmentOffset = ((m_iSegmentOffset + iAlignement - 1) / iAlignement) * iAlignement;

	ASSERT((m_iSegmentOffset + iArraySize) < m_iSegmentSize);

	void* pCurrentBuffer = (unsigned char*)m_arrSegment.back() + m_iSegmentOffset;

	StackAllocatorArray<T> oResult;

	a_pArray->m_iCapacity	= a_iCapacity;
	a_pArray->m_iCount		= 0;
	a_pArray->m_pData		= (T*)pCurrentBuffer;	

	m_iSegmentOffset += iArraySize;
}
