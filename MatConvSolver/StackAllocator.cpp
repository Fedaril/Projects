
#include "StdAfx.h"

#include "StackAllocator.h"
#include "memory.h"

StackAllocator::StackAllocator()
: m_iSegmentSize(0)
, m_iSegmentOffset(0)
{
}

StackAllocator::~StackAllocator()
{
	Shutdown();
}


void StackAllocator::Initialize(unsigned int a_iSegmentSize)
{
	m_iSegmentSize = a_iSegmentSize;
	CreateNewSegment();
}

void StackAllocator::Shutdown()
{
}

void StackAllocator::CreateNewSegment()
{
	void* pSegment = malloc(m_iSegmentSize);
	m_arrSegment.push_back(pSegment);

	m_iSegmentOffset = 0;

}