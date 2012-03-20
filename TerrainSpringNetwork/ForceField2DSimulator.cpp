

#include "StdAfx.h"


#include "ForceField2DSimulator.h"

#include "ForceField2DMultithread.h"



#include "intrin.h"
#pragma intrinsic(_ReadWriteBarrier)

#include "util.h"






namespace ForceField2D
{


static inline float fself(float s, float a, float b)
{
	return (s < 0.0f) ? b : a;
}

static inline float fmaxf(float a, float b)
{
	float fDiff = a - b; //fMaxAmplitude - fWorkerMaxAmplitude;			
	return fself(fDiff, a, b);
}

Simulator::Simulator()
: m_iDimensionX(0)
, m_iDimensionY(0)
, m_iVertexCount(0)
, m_iSpringCount(0)
, m_fHeightScale(0.0f)
, m_pVertexPositionArray(NULL)
, m_pVertexSpringInfoArray(NULL)
, m_pVertexInitialInfoArray(NULL)
, m_pSpringArray(NULL)
, m_fPositionScale(1.0f)
, m_fPreviousMaxForceAmplitude(1.0f)
, m_fMaxDistanceRatio(1.0f)
, m_iErrorHistoryIndex(0)
, m_iIterationCount(0)
, m_bUseMultithread(false)
, m_pMultithread(NULL)
{	
}

Simulator::~Simulator()
{
	Shutdown();
}



inline int Simulator::GetNeighborOffsetX(SpringNeighbor a_eNeighbor)
{
	switch (a_eNeighbor)
	{
	case SpringNeighbor_Top:			return 0;
	case SpringNeighbor_TopLeft:		return -1;
	case SpringNeighbor_TopRight:		return 1;
	case SpringNeighbor_Left:			return - 1;
	case SpringNeighbor_Right:			return 1;
	case SpringNeighbor_Bottom:			return 0;
	case SpringNeighbor_BottomLeft:		return -1;
	case SpringNeighbor_BottomRight:	return 1;
	}

	return 0;
}


inline int Simulator::GetNeighborOffsetY(SpringNeighbor a_eNeighbor)
{
	switch (a_eNeighbor)
	{
	case SpringNeighbor_Top:			return -1;
	case SpringNeighbor_TopLeft:		return -1;
	case SpringNeighbor_TopRight:		return -1;
	case SpringNeighbor_Left:			return 0;
	case SpringNeighbor_Right:			return 0;
	case SpringNeighbor_Bottom:			return 1;
	case SpringNeighbor_BottomLeft:		return 1;
	case SpringNeighbor_BottomRight:	return 1;
	}

	return 0;
}

inline SpringNeighbor Simulator::GetNeighborOpposite(SpringNeighbor a_eNeighbor)
{
	switch (a_eNeighbor)
	{
	case SpringNeighbor_Top:			return SpringNeighbor_Bottom;
	case SpringNeighbor_TopLeft:		return SpringNeighbor_BottomRight;
	case SpringNeighbor_TopRight:		return SpringNeighbor_BottomLeft;
	case SpringNeighbor_Left:			return SpringNeighbor_Right;
	case SpringNeighbor_Right:			return SpringNeighbor_Left;
	case SpringNeighbor_Bottom:			return SpringNeighbor_Top;
	case SpringNeighbor_BottomLeft:		return SpringNeighbor_TopRight;
	case SpringNeighbor_BottomRight:	return SpringNeighbor_TopLeft;
	}

	return SpringNeighbor_Count;
}

unsigned int Simulator::GetVertexNeighborIndex(unsigned int a_iPosX, unsigned int a_iPosY, SpringNeighbor a_eNeighbor)
{	
	int iOffsetX					= GetNeighborOffsetX(a_eNeighbor);
	int iOffsetY					= GetNeighborOffsetY(a_eNeighbor);

	int iNeighborPosX				= (int)a_iPosX + iOffsetX;
	int iNeighborPosY				= (int)a_iPosY + iOffsetY;


	if (iNeighborPosX < 0)
		return s_iInvalidArrayIndex;
	else if (iNeighborPosX > (int)m_iDimensionX)
		return s_iInvalidArrayIndex;


	if (iNeighborPosY < 0)
		return s_iInvalidArrayIndex;
	else if (iNeighborPosY > (int)m_iDimensionY)
		return s_iInvalidArrayIndex;

	//
	unsigned int iArrayPitch		= m_iDimensionX + 1;
	unsigned int iIndex				= (iNeighborPosY * iArrayPitch) + iNeighborPosX;

	assert(iIndex < (m_iDimensionX + 1) * (m_iDimensionY + 1));

	return iIndex;	
}


Spring* Simulator::GetVertexSpringToNeighbor(unsigned int a_iVertexPosX, unsigned int a_iVertexPosY, SpringNeighbor a_eNeighbor)
{
	int			iOffsetX		= 0;
	int			iOffsetY		= 0;
	SpringType	eSpringType		= SpringType_Count;

	switch (a_eNeighbor)
	{
		case SpringNeighbor_Top:
			iOffsetX		= 0;
			iOffsetY		= -1;
			eSpringType		= SpringType_Bottom;
			break;
		case SpringNeighbor_TopLeft:
			iOffsetX		= -1;
			iOffsetY		= -1;
			eSpringType		= SpringType_BottomRight;
			break;
		case SpringNeighbor_TopRight:
			iOffsetX		= 0;
			iOffsetY		= -1;
			eSpringType		= SpringType_BottomLeft_RightNeighbor;
			break;
		case SpringNeighbor_Left:
			iOffsetX		= -1;
			iOffsetY		= 0;
			eSpringType		= SpringType_Right;
			break;
		case SpringNeighbor_Right:
			iOffsetX		= 0;
			iOffsetY		= 0;
			eSpringType		= SpringType_Right;
			break;
		case SpringNeighbor_Bottom:
			iOffsetX		= 0;
			iOffsetY		= 0;
			eSpringType		= SpringType_Bottom;
			break;
		case SpringNeighbor_BottomLeft:
			iOffsetX		= -1;
			iOffsetY		= 0;
			eSpringType		= SpringType_BottomLeft_RightNeighbor;
			break;
		case SpringNeighbor_BottomRight:
			iOffsetX		= 0;
			iOffsetY		= 0;
			eSpringType		= SpringType_BottomRight;
			break;
	}

	int iSpringPosX = (int)a_iVertexPosX + iOffsetX;
	int iSpringPosY = (int)a_iVertexPosY + iOffsetY;

	if (iSpringPosX < 0)
		return NULL;
	else if (iSpringPosX >= (int)m_iDimensionX)
		return NULL;

	if (iSpringPosY < 0)
		return NULL;
	else if (iSpringPosY >= (int)m_iDimensionY)
		return NULL;

	int iSpringArrayOffset = (((iSpringPosY * m_iDimensionY) + iSpringPosX) * SpringType_Count) + eSpringType;
	assert(iSpringArrayOffset < (int)(m_iDimensionX * m_iDimensionY * SpringType_Count));

	return &m_pSpringArray[iSpringArrayOffset];
}

void Simulator::GatherHeightData(const HeightMapCropData* a_pHeightmapData, unsigned a_iStepSize, unsigned a_iPosX, unsigned a_iPosY, VertexInitialInfo* a_pInfo)
{
	unsigned int iMaxHeight		= 0;
	unsigned int iMinHeight		= (unsigned int)-1;
	unsigned int iSumHeight		= 0;
	unsigned int iSampleCount	= 0;

	int	iHeightmapDimension	= a_pHeightmapData->m_iHeightmapDimension;
	int iStartX				= (int)(a_pHeightmapData->m_iStartX + (a_iPosX * a_iStepSize) - (a_iStepSize / 2));
	int	iStartY				= (int)(a_pHeightmapData->m_iStartY + (a_iPosY * a_iStepSize) - (a_iStepSize / 2));
	int iEndX				= iStartX + a_iStepSize;
	int iEndY				= iStartY + a_iStepSize;


	iStartX		= Util::Clamp(iStartX, 0, iHeightmapDimension - 1);
	iStartY		= Util::Clamp(iStartY, 0, iHeightmapDimension - 1);
	iEndX		= Util::Clamp(iEndX, 0, iHeightmapDimension);
	iEndY		= Util::Clamp(iEndY, 0, iHeightmapDimension);

	assert(iStartX < iEndX);
	assert(iStartY < iEndY);


	

	for (int iPosY = iStartY; iPosY < iEndY; ++iPosY)
	{
		for (int iPosX = iStartX; iPosX < iEndX; ++iPosX)
		{
			int iHeightmapX = iPosX;
			int iHeightmapY = iPosY;

			assert(iHeightmapX < iHeightmapDimension);
			assert(iHeightmapY < iHeightmapDimension);

			unsigned int iHeight = a_pHeightmapData->m_pHeightmap[(iHeightmapY * iHeightmapDimension) + iHeightmapX].m_u16Height;

			iMaxHeight = Util::Max(iMaxHeight, iHeight);
			iMinHeight = Util::Min(iMinHeight, iHeight);

			iSumHeight += iHeight;
			iSampleCount++;
		}
	}

	a_pInfo->m_iMinHeight		= iMinHeight;
	a_pInfo->m_iMaxHeight		= iMaxHeight;	
	a_pInfo->m_iAverageHeight	= (iSumHeight / iSampleCount);
}

void Simulator::FillInitialVertexArray(const HeightMapCropData* a_pHeightmapData, unsigned int a_iStepSize)
{
	float fPosX = 0.0f;
	float fPosY = 0.0f;	

	float fPositionStepX = 1.0f;
	float fPositionStepY = 1.0f;

	unsigned int iVertexArrayPitch = m_iDimensionX + 1;

	for (unsigned int iPosY = 0; iPosY <= m_iDimensionY; ++iPosY)
	{
		fPosX = 0.0f;

		for (unsigned int iPosX = 0; iPosX <= m_iDimensionX; ++iPosX)
		{
			unsigned int iVertexArrayOffset = (iPosY * iVertexArrayPitch) + iPosX;
			assert(iVertexArrayOffset < ((m_iDimensionY + 1) * (m_iDimensionX + 1)));

			VertexInitialInfo* pInitialInfo = &m_pVertexInitialInfoArray[iVertexArrayOffset];
			pInitialInfo->m_iArrayX			= iPosX;
			pInitialInfo->m_iArrayY			= iPosY;
			pInitialInfo->m_iMinHeight		= 0;
			pInitialInfo->m_iMaxHeight		= 0;
			pInitialInfo->m_iAverageHeight	= 0;

			GatherHeightData(a_pHeightmapData, a_iStepSize, iPosX, iPosY, pInitialInfo);

			// initial state
			Vector2 vPos = { fPosX, fPosY };
			m_pVertexPositionArray[iVertexArrayOffset] = vPos * m_fPositionScale;

			fPosX += fPositionStepX;
		}

		fPosY += fPositionStepY;
	}
}


float Simulator::GatherSpringLengthData(
	const HeightMapData* a_pHeightmap, unsigned int a_iHeightmapDimension,
	unsigned int a_iHeightmapStartX, unsigned int a_iHeightmapStartY,	
	int a_iStepX, int a_iStepY, unsigned int a_iLength)
{
	static float fMaxSlope = 0.0f;

	assert(a_iHeightmapStartX <= a_iHeightmapDimension);
	assert(((int)a_iHeightmapStartX + ((int)a_iLength * a_iStepX)) <= (int)(a_iHeightmapDimension + 1));
	assert(((int)a_iHeightmapStartX + ((int)a_iLength * a_iStepX)) >= -1);

	assert(a_iHeightmapStartY <= a_iHeightmapDimension);
	assert(((int)a_iHeightmapStartY + ((int)a_iLength * a_iStepY)) <= (int)(a_iHeightmapDimension + 1));
	assert(((int)a_iHeightmapStartY + ((int)a_iLength * a_iStepY)) >= -1);

	assert(a_iLength > 0);

	//
	float			fTotalLength	= 0.0f;

	//
	int				iHeightmapPosX			= (int)a_iHeightmapStartX;
	int				iHeightmapPosY			= (int)a_iHeightmapStartY;

	//
	unsigned int	iVectorLength			= a_iLength;


	int				iClampedHeightmapPosX	= Util::Clamp<int>(iHeightmapPosX, 0, a_iHeightmapDimension - 1);
	int				iClampedHeightmapPosY	= Util::Clamp<int>(iHeightmapPosY, 0, a_iHeightmapDimension - 1);
	unsigned int	iHeightmapIndex			= (iClampedHeightmapPosY * a_iHeightmapDimension) + iClampedHeightmapPosX;
	int				iHeight					= a_pHeightmap[iHeightmapIndex].m_u16Height;

	//	
	float			fStepX				= (float)a_iStepX;
	float			fStepY				= (float)a_iStepY;
	
	float			fStepDistanceSqr	= (fStepX * fStepX) + (fStepY * fStepY);

	while (iVectorLength > 0)
	{
		unsigned int iNextHeightmapPosX = iHeightmapPosX + a_iStepX; 
		unsigned int iNextHeightmapPosY = iHeightmapPosY + a_iStepY;

		iClampedHeightmapPosX	= Util::Clamp<int>(iNextHeightmapPosX, 0, a_iHeightmapDimension - 1);
		iClampedHeightmapPosY	= Util::Clamp<int>(iNextHeightmapPosY, 0, a_iHeightmapDimension - 1);

		//
		unsigned int	iNextHeightmapIndex	= (iClampedHeightmapPosY * a_iHeightmapDimension) + iClampedHeightmapPosX;
		int				iNextHeight			= a_pHeightmap[iNextHeightmapIndex].m_u16Height;

		//
		int		iHeightDiff		= iHeight - iNextHeight;
		float	fHeightDiff		= (float)iHeightDiff * m_fHeightScale;

		if (fabsf(fHeightDiff) > fMaxSlope)
		{
			fMaxSlope = fabsf(fHeightDiff);
		}
		
		float fHeightdiffSqr = fHeightDiff * fHeightDiff;
		float fDistance = sqrtf(fHeightdiffSqr + fStepDistanceSqr);

		//
 		fTotalLength += fDistance;

		iHeightmapPosX	= iNextHeightmapPosX;
		iHeightmapPosY	= iNextHeightmapPosY;
		iHeight			= iNextHeight;

		iVectorLength--;
	}

	return fTotalLength;
}

float Simulator::ComputeIdealSpringLength(const HeightMapCropData* a_pHeightmapData, unsigned a_iHeightmapStep, unsigned int a_iVertexIndex0, unsigned int a_iVertexIndex1)
{
	int				iHeightmapSizeLimitX	= (m_iDimensionX * a_iHeightmapStep);
	int				iHeightmapSizeLimitY	= (m_iDimensionY * a_iHeightmapStep);
	
	//
	VertexInitialInfo*	pVertex0Info	= &m_pVertexInitialInfoArray[a_iVertexIndex0];
	VertexInitialInfo*	pVertex1Info	= &m_pVertexInitialInfoArray[a_iVertexIndex1];

	int				iHeightmap0X			= pVertex0Info->m_iArrayX * a_iHeightmapStep;
	int				iHeightmap0Y			= pVertex0Info->m_iArrayY * a_iHeightmapStep;
	int				iHeightmap1X			= pVertex1Info->m_iArrayX * a_iHeightmapStep;
	int				iHeightmap1Y			= pVertex1Info->m_iArrayY * a_iHeightmapStep;

	//
	float			fIdealSpringLength		= 0.0f;

	//
	int				iDistanceY			= iHeightmap1Y - iHeightmap0Y;
	int				iDistanceX			= iHeightmap1X - iHeightmap0X;

	assert((iDistanceX == 0) || (abs(iDistanceX) == a_iHeightmapStep));
	assert((iDistanceY == 0) || (abs(iDistanceY) == a_iHeightmapStep));


#if 1
	// average height
	// error = 112673.914063
	unsigned int	iHeightAverage0		= pVertex0Info->m_iAverageHeight;
	unsigned int	iHeightAverage1		= pVertex1Info->m_iAverageHeight;
	int				iHeightDiff			= abs((int)iHeightAverage0 - (int)iHeightAverage1); //Util::Max(iHeightDiff0, iHeightDiff1);
	
#elif 0
	// max diff amplitude	
	// error: 113503.921875
	unsigned int	iHeightMin0			= pVertex0Info->m_iMinHeight;
	unsigned int	iHeightMax0			= pVertex0Info->m_iMaxHeight;
	
	unsigned int	iHeightMin1			= pVertex1Info->m_iMinHeight;
	unsigned int	iHeightMax1			= pVertex1Info->m_iMaxHeight;

	int				iHeightDiff0		= abs((int)iHeightMin0 - (int)iHeightMax1);
	int				iHeightDiff1		= abs((int)iHeightMin1 - (int)iHeightMax0);
	int				iHeightDiff			= Util::Max(iHeightDiff0, iHeightDiff1);
#elif 0
	// only compare max height
	// error = 388831.281250
	int				iHeightMax0			= pVertex0Info->m_iMaxHeight;
	int				iHeightMax1			= pVertex1Info->m_iMaxHeight;
	int				iHeightDiff			= abs(iHeightMax1 - iHeightMax0);
#endif


	float			fHeightDiffScaled	= (float)iHeightDiff * m_fHeightScale / (float)a_iHeightmapStep;

	float			fDistanceSqr			= fHeightDiffScaled * fHeightDiffScaled;
	float			fMaxDistanceSqr			= 0.0f;

	const float fMaxDistanceRatio = m_fMaxDistanceRatio;

	if ((iDistanceX == 0) || (iDistanceY == 0))
	{
		fMaxDistanceSqr = 1.0f * (fMaxDistanceRatio * fMaxDistanceRatio);
		fDistanceSqr += 1.0f;
	}
	else
	{
		fMaxDistanceSqr = 2.0f * (fMaxDistanceRatio * fMaxDistanceRatio);
		fDistanceSqr += 2.0f;
	}


	if (fDistanceSqr > fMaxDistanceSqr)
	{
		fDistanceSqr = fMaxDistanceSqr;
	}

	fIdealSpringLength = sqrtf(fDistanceSqr);

/*
float			fBaseSpringLength		= 1.0f;


	if (iHeightmap0X == iHeightmap1X) // vertical
	{
		//
		int iTestY				= iHeightmap0Y < iHeightmap1Y ? iHeightmap0Y : iHeightmap1Y; 
		int iTestStartX			= iHeightmap0X - ((int)a_iHeightmapStep / 2);
		int iTestEndX			= iTestStartX + (int)a_iHeightmapStep - 1;

		if (iTestStartX < 0)
			iTestStartX = 0;
		if (iTestEndX > iHeightmapSizeLimitX)
			iTestEndX = iHeightmapSizeLimitX;

		assert(iTestStartX <= iTestEndX);

		for (int iTestX = iTestStartX; iTestX <= iTestEndX; ++iTestX)
		{
			float fLocalIdealLength = GatherSpringLengthData(
				a_pHeightmapData->m_pHeightmap, a_pHeightmapData->m_iHeightmapDimension, 
				a_pHeightmapData->m_iStartX + (unsigned int)iTestX, 
				a_pHeightmapData->m_iStartY + (unsigned int)iTestY,
				0, 1, a_iHeightmapStep);

			fIdealSpringLength = fmaxf(fIdealSpringLength, fLocalIdealLength);
		}

		fBaseSpringLength = 1.0f;			
	}
	else if (iHeightmap0Y == iHeightmap1Y)  // horizontal
	{
		//
		int iTestX				= iHeightmap0X < iHeightmap1X ? iHeightmap0X : iHeightmap1X; 
		int iTestStartY			= iHeightmap0Y - ((int)a_iHeightmapStep / 2);
		int iTestEndY			= iTestStartY + (int)a_iHeightmapStep - 1;

		if (iTestStartY < 0)
			iTestStartY = 0;
		if (iTestEndY > iHeightmapSizeLimitY)
			iTestEndY = iHeightmapSizeLimitY;

		assert(iTestStartY <= iTestEndY);

		for (int iTestY = iTestStartY; iTestY <= iTestEndY; ++iTestY)
		{
			float fLocalIdealLength = GatherSpringLengthData(
				a_pHeightmapData->m_pHeightmap, a_pHeightmapData->m_iHeightmapDimension, 
				a_pHeightmapData->m_iStartX + (unsigned int)iTestX, 
				a_pHeightmapData->m_iStartY + (unsigned int)iTestY,
				1, 0, a_iHeightmapStep);

			fIdealSpringLength = fmaxf(fIdealSpringLength, fLocalIdealLength);
		}

		fBaseSpringLength = 1.0f;

	}
	else if ((iDistanceX ^ iDistanceY) >= 0) // diagonal, same signs
	{
		//
		int iStartX				= (iHeightmap0X < iHeightmap1X ? iHeightmap0X : iHeightmap1X) - (a_iHeightmapStep / 2);
		int iEndX				= iStartX + (int)a_iHeightmapStep - 1;

		if (iStartX < 0)
			iStartX = 0;
		if (iEndX > iHeightmapSizeLimitX)
			iEndX = iHeightmapSizeLimitX;

		int iTestCountX = iEndX - iStartX;

		//
		int iStartY				= iHeightmap0Y < iHeightmap1Y ? iHeightmap0Y : iHeightmap1Y;
		int iEndY				= iStartY + (int)a_iHeightmapStep - 1;


		if (iStartY < 0)
			iStartY = 0;
		if (iEndY > iHeightmapSizeLimitX)
			iEndY = iHeightmapSizeLimitX;

		int iTestCountY = iEndY - iStartY;

		//
		int iTestCount = iTestCountX < iTestCountY ? iTestCountX : iTestCountY;	


		for (int iTestIndex = 0; iTestIndex <= iTestCount; ++iTestIndex)
		{
			int iStepCount	= a_iHeightmapStep;
			int iPosX		= iStartX + iTestIndex;

			if ((iPosX + iStepCount) > (iHeightmapSizeLimitX + 1))
			{
				iStepCount = iHeightmapSizeLimitX + 1 - iPosX;
			}


			float fLocalIdealLength = GatherSpringLengthData(
				a_pHeightmapData->m_pHeightmap, a_pHeightmapData->m_iHeightmapDimension, 
				a_pHeightmapData->m_iStartX + (unsigned int)(iStartX + iTestIndex), 
				a_pHeightmapData->m_iStartY + (unsigned int)iStartY,
				1, 1, iStepCount);

			fIdealSpringLength = fmaxf(fIdealSpringLength, fLocalIdealLength);
		}

		fBaseSpringLength = 1.41421356237f; // sqrt(2.0f)
	}
	else // diagonal, opposite signs
	{
		//
		int iStartX				= iHeightmap0X < iHeightmap1X ? iHeightmap0X : iHeightmap1X;
		int iEndX				= iStartX + (int)a_iHeightmapStep - 1;

		if (iStartX < 0)
			iStartX = 0;
		if (iEndX > iHeightmapSizeLimitY)
			iEndX = iHeightmapSizeLimitY;

		int iTestCountX			= iEndX - iStartX;

		//
		int iStartY				= (iHeightmap0Y < iHeightmap1Y ? iHeightmap0Y : iHeightmap1Y) + (a_iHeightmapStep / 2);
		int iEndY				= iStartY + (int)a_iHeightmapStep - 1;

		if (iStartY < 0)
			iStartY = 0;
		if (iEndY > iHeightmapSizeLimitY)
			iEndY = iHeightmapSizeLimitY;

		int iTestCountY = iEndY - iStartY;

		//
		int iTestCount = iTestCountX < iTestCountY ? iTestCountX : iTestCountY;	

		for (int iTestIndex = 0; iTestIndex <= iTestCount; ++iTestIndex)
		{
			int iStepCount		= a_iHeightmapStep;
			int iPosY			= iEndY - iTestIndex;

			if ((iPosY - iStepCount + 1) < 0)
			{
				iStepCount = iPosY;
			}

			float fLocalIdealLength = GatherSpringLengthData(
				a_pHeightmapData->m_pHeightmap, a_pHeightmapData->m_iHeightmapDimension, 
				a_pHeightmapData->m_iStartX + (unsigned int)iStartX, 
				a_pHeightmapData->m_iStartY + (unsigned int)iPosY,
				1, -1, iStepCount);

			fIdealSpringLength = fmaxf(fIdealSpringLength, fLocalIdealLength);
		}

		fBaseSpringLength = 1.41421356237f; // sqrt(2.0f)	
	}


	


	fIdealSpringLength /= (float)a_iHeightmapStep;	

	//
	const float fMaxDistanceRatio = 1.8f;
	float fMaxPreferredDistance = fBaseSpringLength * fMaxDistanceRatio;
	if (fIdealSpringLength > fMaxPreferredDistance)
	{
		fIdealSpringLength = fMaxPreferredDistance;
	}
*/

	assert(fIdealSpringLength > 0.0f);
	return fIdealSpringLength;
}


SpringNeighbor Simulator::FindVertexSpringNeighbor(SpringInfo* a_pVertex, Spring* a_pSpring)
{
	for (unsigned int iNeighborIndex = 0; iNeighborIndex < SpringNeighbor_Count; ++iNeighborIndex)
	{
		if (a_pVertex->m_arrNeighborSpring[iNeighborIndex] == a_pSpring)
			return (SpringNeighbor)iNeighborIndex;
	}

	return SpringNeighbor_Count;
}

/*
unsigned int Simulator::SetupVertexNeightborSpring(unsigned int a_iVertexArrayIndex, unsigned int a_iSpringIndex)
{
	SpringInfo*			pVertexSpringInfo	= &m_pVertexSpringInfoArray[a_iVertexArrayIndex];
	VertexInitialInfo*	pVertexInfo			= &m_pVertexInitialInfoArray[a_iVertexArrayIndex];

	unsigned int iSpringCount = 0;
	unsigned int iVertexPosX = pVertexInfo->m_iArrayX;
	unsigned int iVertexPosY = pVertexInfo->m_iArrayY;

	for (unsigned int iNeighborIndex = 0; iNeighborIndex < SpringNeighbor_Count; ++iNeighborIndex)
	{
		unsigned int iVertexNeighborArrayIndex = GetVertexNeighborIndex(iVertexPosX, iVertexPosY, (SpringNeighbor)iNeighborIndex);

		if (iVertexNeighborArrayIndex == s_iInvalidArrayIndex)
			continue;

		assert(iVertexNeighborArrayIndex < ((m_iDimensionY + 1) * (m_iDimensionX + 1)));

		//
		SpringInfo* pNeighborSpringInfo = &m_pVertexSpringInfoArray[iVertexNeighborArrayIndex];
		VertexInitialInfo* pNeighborInfo = &m_pVertexInitialInfoArray[iVertexNeighborArrayIndex];

		if (pVertexSpringInfo->m_arrNeighborSpring[iNeighborIndex] != NULL)
		{
			assert(pVertexInfo->m_arrNeighborIndex[iNeighborIndex] != s_iInvalidArrayIndex);
			assert(pVertexInfo->m_arrNeighborIndex[iNeighborIndex] == iVertexNeighborArrayIndex);
			assert(pVertexSpringInfo->m_arrNeighborSpring[iNeighborIndex] != NULL);
			assert(
				(pVertexSpringInfo->m_arrNeighborSpring[iNeighborIndex]->m_iVertexIndex0 == a_iVertexArrayIndex) 
				||	(pVertexSpringInfo->m_arrNeighborSpring[iNeighborIndex]->m_iVertexIndex1 == a_iVertexArrayIndex));

			continue;
		}


		//				
		assert(a_iSpringIndex < m_iSpringCount);
		Spring* pSpring = &m_pSpringArray[a_iSpringIndex + iSpringCount];
		iSpringCount++;


		SpringNeighbor eOpposite = GetNeighborOpposite((SpringNeighbor)iNeighborIndex);

		//switch (iNeighborIndex)
		//{
		//case SpringNeighbor_Top:
		//case SpringNeighbor_TopLeft:
		//case SpringNeighbor_TopRight:
		//case SpringNeighbor_Left:
		//case SpringNeighbor_Right:
		//case SpringNeighbor_Bottom:
		//case SpringNeighbor_BottomLeft:
		//case SpringNeighbor_BottomRight:
		//}


		pVertexSpringInfo->m_arrNeighborSpring[iNeighborIndex]	= pSpring;

		
		pNeighborSpringInfo->m_arrNeighborSpring[eOpposite]		= pSpring;

		pSpring->m_iVertexIndex0	= a_iVertexArrayIndex;
		pSpring->m_iVertexIndex1	= iVertexNeighborArrayIndex;

		pVertexInfo->m_arrNeighborIndex[iNeighborIndex]			= iVertexNeighborArrayIndex;
		pNeighborInfo->m_arrNeighborIndex[eOpposite]			= a_iVertexArrayIndex;
	}

	return iSpringCount;
}
*/

void Simulator::SetupInsideQuad(const Simulator& a_oPreviousResult, unsigned int a_iPreviousPosX, unsigned int a_iPreviousPosY)
{
	unsigned int iPreviousDimensionX		= a_oPreviousResult.m_iDimensionX;
	unsigned int iPreviousVertexArrayPitch	= iPreviousDimensionX + 1;
	unsigned int iNewVertexArrayPitch		= m_iDimensionX + 1;

	unsigned int iPreviousVertex00PosX = a_iPreviousPosX;
	unsigned int iPreviousVertex00PosY = a_iPreviousPosY;
	unsigned int iPreviousVertex01PosX = a_iPreviousPosX + 1;
	unsigned int iPreviousVertex10PosY = a_iPreviousPosY + 1;

	unsigned int iPreviousVertex00ArrayOffset = (iPreviousVertex00PosY * iPreviousVertexArrayPitch) + iPreviousVertex00PosX;
	unsigned int iPreviousVertex01ArrayOffset = (iPreviousVertex00PosY * iPreviousVertexArrayPitch) + iPreviousVertex01PosX;
	unsigned int iPreviousVertex10ArrayOffset = (iPreviousVertex10PosY * iPreviousVertexArrayPitch) + iPreviousVertex00PosX;
	unsigned int iPreviousVertex11ArrayOffset = (iPreviousVertex10PosY * iPreviousVertexArrayPitch) + iPreviousVertex01PosX;

	assert(iPreviousVertex00ArrayOffset < ((a_oPreviousResult.m_iDimensionY + 1) * (a_oPreviousResult.m_iDimensionX + 1)));
	assert(iPreviousVertex01ArrayOffset < ((a_oPreviousResult.m_iDimensionY + 1) * (a_oPreviousResult.m_iDimensionX + 1)));
	assert(iPreviousVertex10ArrayOffset < ((a_oPreviousResult.m_iDimensionY + 1) * (a_oPreviousResult.m_iDimensionX + 1)));
	assert(iPreviousVertex11ArrayOffset < ((a_oPreviousResult.m_iDimensionY + 1) * (a_oPreviousResult.m_iDimensionX + 1)));

	Vector2 vPreviousVertexPosition00 = a_oPreviousResult.m_pVertexPositionArray[iPreviousVertex00ArrayOffset];
	Vector2 vPreviousVertexPosition01 = a_oPreviousResult.m_pVertexPositionArray[iPreviousVertex01ArrayOffset];
	Vector2 vPreviousVertexPosition10 = a_oPreviousResult.m_pVertexPositionArray[iPreviousVertex10ArrayOffset];
	Vector2 vPreviousVertexPosition11 = a_oPreviousResult.m_pVertexPositionArray[iPreviousVertex11ArrayOffset];


	unsigned int		pNewVertexIndex[4];

	for (unsigned int iChildVertex = 0; iChildVertex < 4; ++iChildVertex)
	{
		unsigned int iOffsetX = iChildVertex % 2;
		unsigned int iOffsetY = iChildVertex / 2;
		unsigned int iChildPosX = (a_iPreviousPosX * 2) + iOffsetX;
		unsigned int iChildPosY = (a_iPreviousPosY * 2) + iOffsetY;

		unsigned int iNewVertexArrayOffset = (iChildPosY * iNewVertexArrayPitch) + iChildPosX;

		assert(iNewVertexArrayOffset < ((m_iDimensionX + 1) * (m_iDimensionY + 1)));


		pNewVertexIndex[iChildVertex] = iNewVertexArrayOffset;

		m_pVertexInitialInfoArray[iNewVertexArrayOffset].m_iArrayX = iChildPosX;
		m_pVertexInitialInfoArray[iNewVertexArrayOffset].m_iArrayY = iChildPosY;

		
	}


	//
	Vector2 vDirectionX = vPreviousVertexPosition01 - vPreviousVertexPosition00;
	Vector2 vDirectionY = vPreviousVertexPosition10 - vPreviousVertexPosition00;
	Vector2 vCenter = (vPreviousVertexPosition00 + vPreviousVertexPosition01 + vPreviousVertexPosition10 + vPreviousVertexPosition11) * 0.25f;


	m_pVertexPositionArray[pNewVertexIndex[0]] = (vPreviousVertexPosition00 * 2.0f) * m_fPositionScale;
	m_pVertexPositionArray[pNewVertexIndex[1]] = ((vPreviousVertexPosition00 * 2.0f) + vDirectionX) * m_fPositionScale;
	m_pVertexPositionArray[pNewVertexIndex[2]] = ((vPreviousVertexPosition00 * 2.0f) + vDirectionY) * m_fPositionScale;
	m_pVertexPositionArray[pNewVertexIndex[3]] = (vCenter * 2.0f) * m_fPositionScale; //((vPreviousVertexPosition00 * 2.0f) + vDirectionX + vDirectionY) * m_fPositionScale;
}

void Simulator::SetupRightBorderQuad(const Simulator& a_oPreviousResult, unsigned int a_iPreviousPosY)
{
	unsigned int iPreviousDimensionX		= a_oPreviousResult.m_iDimensionX;
	unsigned int iPreviousVertexArrayPitch	= iPreviousDimensionX + 1;
	unsigned int iNewVertexArrayPitch		= m_iDimensionX + 1;

	unsigned int iPreviousVertex00PosX = iPreviousDimensionX;
	unsigned int iPreviousVertex00PosY = a_iPreviousPosY;			
	unsigned int iPreviousVertex10PosY = a_iPreviousPosY + 1;

	unsigned int iPreviousVertex00ArrayOffset = (iPreviousVertex00PosY * iPreviousVertexArrayPitch) + iPreviousVertex00PosX;			
	unsigned int iPreviousVertex10ArrayOffset = (iPreviousVertex10PosY * iPreviousVertexArrayPitch) + iPreviousVertex00PosX;

	assert(iPreviousVertex00ArrayOffset < ((a_oPreviousResult.m_iDimensionY + 1) * (a_oPreviousResult.m_iDimensionX + 1)));
	assert(iPreviousVertex10ArrayOffset < ((a_oPreviousResult.m_iDimensionY + 1) * (a_oPreviousResult.m_iDimensionX + 1)));

	Vector2 vPreviousVertexPosition00 = a_oPreviousResult.m_pVertexPositionArray[iPreviousVertex00ArrayOffset];			
	Vector2 vPreviousVertexPosition10 = a_oPreviousResult.m_pVertexPositionArray[iPreviousVertex10ArrayOffset];

	Vector2 vDirectionY = vPreviousVertexPosition10 - vPreviousVertexPosition00;

	unsigned int iNewVertex0PosX = iPreviousVertex00PosX * 2;
	unsigned int iNewVertex0PosY = a_iPreviousPosY * 2;
	unsigned int iNewVertex1PosY = (a_iPreviousPosY * 2) + 1;
	unsigned int iNewVertex0ArrayOffset = (iNewVertex0PosY * iNewVertexArrayPitch) + iNewVertex0PosX;
	unsigned int iNewVertex1ArrayOffset = (iNewVertex1PosY * iNewVertexArrayPitch) + iNewVertex0PosX;

	assert(iNewVertex0ArrayOffset < ((m_iDimensionX + 1) * (m_iDimensionY + 1)));
	assert(iNewVertex1ArrayOffset < ((m_iDimensionX + 1) * (m_iDimensionY + 1)));

	m_pVertexPositionArray[iNewVertex0ArrayOffset] = (vPreviousVertexPosition00 * 2.0f) * m_fPositionScale;
	m_pVertexPositionArray[iNewVertex1ArrayOffset] = ((vPreviousVertexPosition00 * 2.0f) + vDirectionY) * m_fPositionScale;

	m_pVertexInitialInfoArray[iNewVertex0ArrayOffset].m_iArrayX	= iNewVertex0PosX;
	m_pVertexInitialInfoArray[iNewVertex0ArrayOffset].m_iArrayY	= iNewVertex0PosY;

	m_pVertexInitialInfoArray[iNewVertex1ArrayOffset].m_iArrayX	= iNewVertex0PosX;
	m_pVertexInitialInfoArray[iNewVertex1ArrayOffset].m_iArrayY	= iNewVertex1PosY;
}

void Simulator::SetupBottomBorderQuad(const Simulator& a_oPreviousResult, unsigned int a_iPreviousPosX)
{
	unsigned int iPreviousDimensionX		= a_oPreviousResult.m_iDimensionX;
	unsigned int iPreviousDimensionY		= a_oPreviousResult.m_iDimensionX;
	unsigned int iPreviousVertexArrayPitch	= iPreviousDimensionX + 1;
	unsigned int iNewVertexArrayPitch		= m_iDimensionX + 1;

	unsigned int iPreviousVertex00PosX = a_iPreviousPosX;
	unsigned int iPreviousVertex01PosX = a_iPreviousPosX + 1;
	unsigned int iPreviousVertex00PosY = iPreviousDimensionY;			

	unsigned int iPreviousVertex00ArrayOffset = (iPreviousVertex00PosY * iPreviousVertexArrayPitch) + iPreviousVertex00PosX;			
	unsigned int iPreviousVertex01ArrayOffset = (iPreviousVertex00PosY * iPreviousVertexArrayPitch) + iPreviousVertex01PosX;

	assert(iPreviousVertex00ArrayOffset < ((a_oPreviousResult.m_iDimensionY + 1) * (a_oPreviousResult.m_iDimensionX + 1)));
	assert(iPreviousVertex01ArrayOffset < ((a_oPreviousResult.m_iDimensionY + 1) * (a_oPreviousResult.m_iDimensionX + 1)));

	Vector2 vPreviousVertexPosition00 = a_oPreviousResult.m_pVertexPositionArray[iPreviousVertex00ArrayOffset];			
	Vector2 vPreviousVertexPosition01 = a_oPreviousResult.m_pVertexPositionArray[iPreviousVertex01ArrayOffset];

	Vector2 vDirectionX = vPreviousVertexPosition01 - vPreviousVertexPosition00;

	unsigned int iNewVertex0PosX = a_iPreviousPosX * 2;
	unsigned int iNewVertex0PosY = iPreviousVertex00PosY * 2;
	unsigned int iNewVertex1PosX = (a_iPreviousPosX * 2) + 1;
	unsigned int iNewVertex0ArrayOffset = (iNewVertex0PosY * iNewVertexArrayPitch) + iNewVertex0PosX;
	unsigned int iNewVertex1ArrayOffset = (iNewVertex0PosY * iNewVertexArrayPitch) + iNewVertex1PosX;

	assert(iNewVertex0ArrayOffset < ((m_iDimensionX + 1) * (m_iDimensionY + 1)));
	assert(iNewVertex1ArrayOffset < ((m_iDimensionX + 1) * (m_iDimensionY + 1)));

	//
	m_pVertexPositionArray[iNewVertex0ArrayOffset] = (vPreviousVertexPosition00 * 2.0f) * m_fPositionScale;
	m_pVertexPositionArray[iNewVertex1ArrayOffset] = ((vPreviousVertexPosition00 * 2.0f) + vDirectionX) * m_fPositionScale;


	m_pVertexInitialInfoArray[iNewVertex0ArrayOffset].m_iArrayX = iNewVertex0PosX;
	m_pVertexInitialInfoArray[iNewVertex0ArrayOffset].m_iArrayY = iNewVertex0PosY;

	m_pVertexInitialInfoArray[iNewVertex1ArrayOffset].m_iArrayX = iNewVertex1PosX;
	m_pVertexInitialInfoArray[iNewVertex1ArrayOffset].m_iArrayY = iNewVertex0PosY;
}

void Simulator::SetupBottomRightCornerQuad(const Simulator& a_oPreviousResult)
{
	unsigned int iPreviousDimensionX		= a_oPreviousResult.m_iDimensionX;
	unsigned int iPreviousDimensionY		= a_oPreviousResult.m_iDimensionX;
	unsigned int iPreviousArrayIndex		= ((iPreviousDimensionY + 1) * (iPreviousDimensionX + 1)) - 1;

	assert(iPreviousArrayIndex < ((a_oPreviousResult.m_iDimensionY + 1) * (a_oPreviousResult.m_iDimensionX + 1)));
	Vector2 vPreviousVertexPosition	= a_oPreviousResult.m_pVertexPositionArray[iPreviousArrayIndex];


	unsigned int		iNewVertexPosX			= m_iDimensionX;
	unsigned int		iNewVertexPosY			= m_iDimensionY;
	unsigned int		iNewVertexArrayIndex	= (iNewVertexPosY * (m_iDimensionX + 1)) + iNewVertexPosX;
	
	assert(iNewVertexArrayIndex < ((m_iDimensionY + 1) * (m_iDimensionX + 1)));

	m_pVertexPositionArray[iNewVertexArrayIndex] = (vPreviousVertexPosition * 2.0f) * m_fPositionScale;

	m_pVertexInitialInfoArray[iNewVertexArrayIndex].m_iArrayX = iNewVertexPosX;
	m_pVertexInitialInfoArray[iNewVertexArrayIndex].m_iArrayY = iNewVertexPosY;

}

void Simulator::FillVertexArrayFromPreviousResult(const Simulator& a_oPreviousResult)
{
	unsigned int iPreviousDimensionX	= a_oPreviousResult.m_iDimensionX;
	unsigned int iPreviousDimensionY	= a_oPreviousResult.m_iDimensionX;

	for (unsigned int iPosY = 0; iPosY < iPreviousDimensionY; ++iPosY)
	{
		for (unsigned int iPosX = 0; iPosX < iPreviousDimensionX; ++iPosX)
		{
			SetupInsideQuad(a_oPreviousResult, iPosX, iPosY);
		}

		SetupRightBorderQuad(a_oPreviousResult, iPosY);
	}

	for (unsigned int iPosX = 0; iPosX < iPreviousDimensionX; ++iPosX)
	{
		SetupBottomBorderQuad(a_oPreviousResult, iPosX);
	}

	SetupBottomRightCornerQuad(a_oPreviousResult);
}


void Simulator::CreateSpringArray()
{
	unsigned int iSpringIndex = 0;

	for (unsigned int iPosY = 0; iPosY < (m_iDimensionY + 1); iPosY++)
	{
		for (unsigned int iPosX = 0; iPosX < (m_iDimensionX + 1); iPosX++)
		{
			unsigned int iVertexIndex				= (iPosY * (m_iDimensionX + 1)) + iPosX;
			unsigned int iRightVertexIndex			= (iPosY * (m_iDimensionX + 1)) + iPosX + 1;
			unsigned int iBottomVertexIndex			= ((iPosY + 1) * (m_iDimensionX + 1)) + iPosX;
			unsigned int iBottomRightVertexIndex	= ((iPosY + 1) * (m_iDimensionX + 1)) + iPosX + 1;

			assert(iVertexIndex < m_iVertexCount);

			SpringInfo* pVertexSpringInfo = &m_pVertexSpringInfoArray[iVertexIndex];


			// right / left
			if (iPosX < m_iDimensionX)
			{
				Spring* pRightSpring = &m_pSpringArray[iSpringIndex];
				iSpringIndex++;

				pRightSpring->m_iVertexIndex0	= iVertexIndex;
				pRightSpring->m_iVertexIndex1	= iRightVertexIndex;

				pVertexSpringInfo->m_arrNeighborSpring[SpringNeighbor_Right] = pRightSpring;

				assert(iRightVertexIndex < m_iVertexCount);
				assert(m_pVertexSpringInfoArray[iRightVertexIndex].m_arrNeighborSpring[SpringNeighbor_Left] == NULL);
				
				m_pVertexSpringInfoArray[iRightVertexIndex].m_arrNeighborSpring[SpringNeighbor_Left] = pRightSpring;
			}

			// bottom / top
			if (iPosY < m_iDimensionY)
			{
				Spring* pBottomSpring = &m_pSpringArray[iSpringIndex];
				iSpringIndex++;

				pBottomSpring->m_iVertexIndex0	= iVertexIndex;
				pBottomSpring->m_iVertexIndex1	= iBottomVertexIndex;


				pVertexSpringInfo->m_arrNeighborSpring[SpringNeighbor_Bottom] = pBottomSpring;
				
				assert(iBottomVertexIndex < m_iVertexCount);
				assert(m_pVertexSpringInfoArray[iBottomVertexIndex].m_arrNeighborSpring[SpringNeighbor_Top] == NULL);
				
				m_pVertexSpringInfoArray[iBottomVertexIndex].m_arrNeighborSpring[SpringNeighbor_Top] = pBottomSpring;

			}

			// bottom right / top left
			if ((iPosX < m_iDimensionX) && (iPosY < m_iDimensionY))
			{
				Spring* pBottomRightSpring = &m_pSpringArray[iSpringIndex];
				iSpringIndex++;

				pBottomRightSpring->m_iVertexIndex0 = iVertexIndex;
				pBottomRightSpring->m_iVertexIndex1	= iBottomRightVertexIndex;

				pVertexSpringInfo->m_arrNeighborSpring[SpringNeighbor_BottomRight] = pBottomRightSpring;
				
				assert(iBottomRightVertexIndex < m_iVertexCount);
				assert(m_pVertexSpringInfoArray[iBottomRightVertexIndex].m_arrNeighborSpring[SpringNeighbor_TopLeft] == NULL);
				
				m_pVertexSpringInfoArray[iBottomRightVertexIndex].m_arrNeighborSpring[SpringNeighbor_TopLeft] = pBottomRightSpring;

			}

			// right neighbor: bottom left / top right
			if ((iPosX < m_iDimensionX) && (iPosY < m_iDimensionY))
			{
				Spring* pBottomLeftRightNeighborSpring = &m_pSpringArray[iSpringIndex];
				iSpringIndex++;

				pBottomLeftRightNeighborSpring->m_iVertexIndex0 = iRightVertexIndex;
				pBottomLeftRightNeighborSpring->m_iVertexIndex1 = iBottomVertexIndex;


				assert(m_pVertexSpringInfoArray[iRightVertexIndex].m_arrNeighborSpring[SpringNeighbor_BottomLeft] == NULL);
				assert(m_pVertexSpringInfoArray[iBottomVertexIndex].m_arrNeighborSpring[SpringNeighbor_TopRight] == NULL);

				m_pVertexSpringInfoArray[iRightVertexIndex].m_arrNeighborSpring[SpringNeighbor_BottomLeft] = pBottomLeftRightNeighborSpring;
				m_pVertexSpringInfoArray[iBottomVertexIndex].m_arrNeighborSpring[SpringNeighbor_TopRight] = pBottomLeftRightNeighborSpring;
			}
		}
	}

	assert(iSpringIndex == m_iSpringCount);
}

void Simulator::FillInitialSpringData(const HeightMapCropData* a_pHeightmapData, unsigned int a_iHeightmapStep)
{
	const float fHeightmapStep = (float)a_iHeightmapStep;	

	//
	assert((m_iDimensionX * a_iHeightmapStep) <= (a_pHeightmapData->m_iStartX + a_pHeightmapData->m_iSizeX));
	assert((m_iDimensionX * a_iHeightmapStep) <= (a_pHeightmapData->m_iStartY + a_pHeightmapData->m_iSizeY));


	for (unsigned int iSpringIndex = 0; iSpringIndex < m_iSpringCount; ++iSpringIndex)
	{
		Spring*		pSpring			= &m_pSpringArray[iSpringIndex];			

		unsigned int		iVertexArrayIndex0	= pSpring->m_iVertexIndex0;
		unsigned int		iVertexArrayIndex1	= pSpring->m_iVertexIndex1;

#if defined(DEBUG_SPRING_VERTEX)
		VertexInitialInfo*	pVertex0Info	= &m_pVertexInitialInfoArray[iVertexArrayIndex0];
		VertexInitialInfo*	pVertex1Info	= &m_pVertexInitialInfoArray[iVertexArrayIndex1];

		// consistency checks
		SpringNeighbor iNeighbor0 = FindVertexSpringNeighbor(&m_pVertexSpringInfoArray[iVertexArrayIndex0], pSpring);
		SpringNeighbor iNeighbor1 = FindVertexSpringNeighbor(&m_pVertexSpringInfoArray[iVertexArrayIndex1], pSpring);
		assert(iNeighbor0 != SpringNeighbor_Count);
		assert(iNeighbor1 != SpringNeighbor_Count);
		assert(iNeighbor0 == GetNeighborOpposite(iNeighbor1));
#endif

		//
		float fIdealLength = ComputeIdealSpringLength(a_pHeightmapData, a_iHeightmapStep, iVertexArrayIndex0, iVertexArrayIndex1);

		pSpring->m_fPreferredDistance = fIdealLength;
		

		//
		Vector2	vVertexPosition0 = m_pVertexPositionArray[iVertexArrayIndex0];
		Vector2	vVertexPosition1 = m_pVertexPositionArray[iVertexArrayIndex1];

		Vector2 vDistance	= vVertexPosition1 - vVertexPosition0;
		float fLength		= sqrtf((vDistance.x * vDistance.x) + (vDistance.y * vDistance.y));

		pSpring->m_vDirection		= vDistance / fLength;		
		pSpring->m_fDistance		= fLength;

		//
		//pSpring->m_vPreferredDirection = pSpring->m_vDirection;

		assert(fLength > 0.0000001f);

		//pSpring->m_fError			= pSpring->m_fPreferredDistance - fLength;
	}

}


void Simulator::Initialize(
		   unsigned int a_iDimensionX, unsigned int a_iDimensionY,
		   float a_fPositionScale, float a_fHeightScale, float a_fMaxDistanceRatio,
		   const HeightMapCropData* a_pHeightmapData, unsigned int a_iHeightmapStep)
{

	m_iIterationCount				= 0;

	m_fPositionScale				= a_fPositionScale;
	m_fMaxDistanceRatio				= a_fMaxDistanceRatio;

	m_iDimensionX					= a_iDimensionX;
	m_iDimensionY					= a_iDimensionY;
	m_fHeightScale					= a_fHeightScale;
	m_iVertexCount					= (m_iDimensionX + 1) * (m_iDimensionY + 1);
	m_iSpringCount					= (m_iDimensionX * m_iDimensionY * SpringType_Count) + (m_iDimensionX + m_iDimensionY);

	m_fPreviousMaxForceAmplitude	= (float)(m_iDimensionX + m_iDimensionY);

	//
	m_pVertexPositionArray		= (Vector2*)calloc(m_iVertexCount, sizeof(Vector2));
	m_pVertexSpringInfoArray	= (SpringInfo*)calloc(m_iVertexCount, sizeof(SpringInfo));
	m_pVertexInitialInfoArray	= (VertexInitialInfo*)calloc(m_iVertexCount, sizeof(VertexInitialInfo));
	m_pSpringArray				= (Spring*)calloc(m_iSpringCount, sizeof(Spring));


	CreateSpringNeighborFactorArray();

	FillInitialVertexArray(a_pHeightmapData, a_iHeightmapStep);

	CreateSpringArray();	

	FillInitialSpringData(a_pHeightmapData, a_iHeightmapStep);

	InitializeErrorHistory();

	InitializeMultithread();
}

void Simulator::InitializeFromPreviousResult(
						   const Simulator& a_oPreviousResult,
						   const HeightMapCropData* a_pHeightmapData, unsigned int a_iHeightmapStep)
{
	m_iIterationCount				= 0;

	m_fPositionScale				= 1.0f; //a_oPreviousResult.m_fPositionScale;
	m_fMaxDistanceRatio				= a_oPreviousResult.m_fMaxDistanceRatio;
	m_iDimensionX					= a_oPreviousResult.m_iDimensionX * 2;
	m_iDimensionY					= a_oPreviousResult.m_iDimensionY * 2;
	m_fHeightScale					= a_oPreviousResult.m_fHeightScale;
	m_iVertexCount					= (m_iDimensionX + 1) * (m_iDimensionY + 1);
	m_iSpringCount					= (m_iDimensionX * m_iDimensionY * SpringType_Count) + (m_iDimensionX + m_iDimensionY);

	m_fPreviousMaxForceAmplitude	= (float)(m_iDimensionX + m_iDimensionY);

	//
	m_pVertexPositionArray		= (Vector2*)calloc(m_iVertexCount, sizeof(Vector2));
	m_pVertexSpringInfoArray	= (SpringInfo*)calloc(m_iVertexCount, sizeof(SpringInfo));
	m_pVertexInitialInfoArray	= (VertexInitialInfo*)calloc(m_iVertexCount, sizeof(VertexInitialInfo));
	m_pSpringArray				= (Spring*)calloc(m_iSpringCount, sizeof(Spring));

	CreateSpringNeighborFactorArray();

	FillVertexArrayFromPreviousResult(a_oPreviousResult);

	//
	for (unsigned int iPosY = 0; iPosY < m_iDimensionY + 1; ++iPosY)
	{
		for (unsigned int iPosX = 0; iPosX < m_iDimensionX + 1; ++iPosX)
		{
			unsigned int iVertexArrayPitch = m_iDimensionX + 1;
			unsigned int iVertexArrayOffset = (iPosY * iVertexArrayPitch) + iPosX;
			assert(iVertexArrayOffset < ((m_iDimensionY + 1) * (m_iDimensionX + 1)));

			VertexInitialInfo* pInitialInfo = &m_pVertexInitialInfoArray[iVertexArrayOffset];

			GatherHeightData(a_pHeightmapData, a_iHeightmapStep, iPosX, iPosY, pInitialInfo);
		}
	}


	CreateSpringArray();

	FillInitialSpringData(a_pHeightmapData, a_iHeightmapStep);
	
	InitializeErrorHistory();

	InitializeMultithread();
}


void Simulator::Shutdown()
{
	if (m_bUseMultithread)
	{
		m_pMultithread->Shutdown();
		
		delete m_pMultithread;
		m_pMultithread = NULL;
	}
	m_bUseMultithread = false;


	free(m_pVertexPositionArray);
	free(m_pVertexSpringInfoArray);
	free(m_pVertexInitialInfoArray);
	free(m_pSpringArray);

	m_pVertexPositionArray			= NULL;
	m_pVertexSpringInfoArray		= NULL;
	m_pVertexInitialInfoArray		= NULL;
	m_pSpringArray					= NULL;

	m_iDimensionX					= 0;
	m_iDimensionY					= 0;
	m_fHeightScale					= 0.0f;
	m_iSpringCount					= 0;
	m_iErrorHistoryIndex			= 0;
	m_fPositionScale				= 0.0f;
	m_fPreviousMaxForceAmplitude	= 1.0f;
	m_fMaxDistanceRatio				= 1.0f;

}

void Simulator::CreateSpringNeighborFactorArray()
{
	float fSqrt2 = 1.0f; //sqrtf(2.0f);

	m_arrSpringFactor[SpringNeighbor_Top]			=  1.0f;
	m_arrSpringFactor[SpringNeighbor_TopLeft]		=  1.0f / fSqrt2;
	m_arrSpringFactor[SpringNeighbor_TopRight]		=  1.0f / fSqrt2;
	m_arrSpringFactor[SpringNeighbor_Left]			=  1.0f;
	m_arrSpringFactor[SpringNeighbor_Right]			= -1.0f;
	m_arrSpringFactor[SpringNeighbor_Bottom]		= -1.0f;
	m_arrSpringFactor[SpringNeighbor_BottomLeft]	= -1.0f / fSqrt2;
	m_arrSpringFactor[SpringNeighbor_BottomRight]	= -1.0f / fSqrt2;
	
}

void Simulator::InitializeErrorHistory()
{

	for (unsigned int iHistoryIndex = 0; iHistoryIndex < s_iErrorHistoryLength; ++iHistoryIndex) 
	{
		m_fErrorHistory[iHistoryIndex] = 0;
	}

	m_iErrorHistoryIndex = 0;
}

void Simulator::InitializeMultithread()
{
	if (m_iSpringCount <= 4096)
		return; // do not use worker thread for small grids
	
	m_pMultithread = new Multithread();
	m_pMultithread->Initialize(this, 8);

	m_bUseMultithread = true;
}


bool Simulator::IsConverged()
{
	//if (m_fPreviousMaxForceAmplitude <= (0.00125f / m_iVertexCount))
	//	return true;

	float fMaxError = m_fErrorHistory[0];
	float fMinError = m_fErrorHistory[0];

	for (unsigned int iError = 1; iError < s_iErrorHistoryLength; ++iError)
	{
		if (fMaxError < m_fErrorHistory[iError])
			fMaxError = m_fErrorHistory[iError];

		if (fMinError > m_fErrorHistory[iError])
			fMinError = m_fErrorHistory[iError];
	}

	float fSpringCount	= (float)m_iSpringCount;
	float fErrorScale	= Util::Max(1.0f / fSpringCount, 1.0f / 32768.0f);
	float fThreshold	= fMaxError * fErrorScale; //((m_iDimensionX + 1) * (m_iDimensionY + 1) * 0.0125f);
	float fVal			= (fMaxError - fMinError);
	if (fVal <= fThreshold)
		return true;

	return false;
}


float Simulator::Simulate(float a_fDt)
{
	float fError = 0.0f;	

	if (m_bUseMultithread)
	{
		fError = m_pMultithread->Simulate(a_fDt);		
	}
	else
	{
		float fMaxAmplitude				= 0.0f;
		float fPreviousMaxAmplitude		= Util::Max(m_fPreviousMaxForceAmplitude, 0.00001f);
		float fUpdateStep				= a_fDt / fPreviousMaxAmplitude;

		fError				= UpdateSpringLength(0, m_iSpringCount);		
		fMaxAmplitude		= ApplySpringForce(0, m_iVertexCount, fUpdateStep);		

		m_fPreviousMaxForceAmplitude = fMaxAmplitude;
	}	
	
	m_fErrorHistory[m_iErrorHistoryIndex] = fError;
	m_iErrorHistoryIndex = (m_iErrorHistoryIndex + 1) % s_iErrorHistoryLength;

	m_iIterationCount++;

	return fError;
}

float Simulator::ApplySpringForce(unsigned int a_iStartIndex, unsigned int a_iCount, float a_fUpdateStep)
{
	unsigned int iEndIndex = a_iStartIndex + a_iCount;

	assert(iEndIndex <= ((m_iDimensionX + 1) * (m_iDimensionY + 1)));

	float fMaxAmplitudeSqr = 0.0f;

	//
	Vector2* __restrict pPositionArray = m_pVertexPositionArray;

	for (unsigned int iVertexIndex = a_iStartIndex; iVertexIndex < iEndIndex; ++iVertexIndex)
	{
		SpringInfo*	__restrict	pSpringInfo			= &m_pVertexSpringInfoArray[iVertexIndex];
		Vector2					vForce				= { 0.0f, 0.0f };

		for (unsigned int iNeighborIndex = 0; iNeighborIndex < SpringNeighbor_Count; ++iNeighborIndex)
		{
			const Spring* __restrict	pSpring			= pSpringInfo->m_arrNeighborSpring[iNeighborIndex];			
			

			if (pSpring == NULL)
				continue;

#if defined(DEBUG_SPRING_VERTEX)
			VertexInitialInfo* __restrict pVertexInitialInfo = &m_pVertexInitialInfoArray[iVertexIndex];
			unsigned int iNeighborArrayIndex = GetVertexNeighborIndex(pVertexInitialInfo->m_iArrayX, pVertexInitialInfo->m_iArrayY, (SpringNeighbor)iNeighborIndex);

			assert(iNeighborArrayIndex != s_iInvalidArrayIndex);
			assert((pSpring->m_iVertexIndex0 == iVertexIndex) || (pSpring->m_iVertexIndex1 == iVertexIndex));
			assert((pSpring->m_iVertexIndex0 == iNeighborArrayIndex) || (pSpring->m_iVertexIndex1 == iNeighborArrayIndex));
#endif

			//
			float	fFactor			= m_arrSpringFactor[iNeighborIndex];
			float	fDiffLength		= pSpring->m_fDistance;

			float fPreferredDiff	= pSpring->m_fPreferredDistance - fDiffLength;
			float fForceAmplitude	= fFactor * fPreferredDiff;

			fForceAmplitude *= pow(fabsf(fForceAmplitude), 4);

			//_ASSERT(fForceAmplitude == fForceAmplitude);
			vForce += pSpring->m_vDirection * fForceAmplitude;

			//_ASSERT(vForce.x == vForce.x);
			//_ASSERT(vForce.y == vForce.y);
		}


		//_ASSERT(a_fUpdateStep == a_fUpdateStep);


		pPositionArray[iVertexIndex] += vForce * a_fUpdateStep;

		//
		float fAmplitudeSqr = (vForce.x * vForce.x) + (vForce.y * vForce.y);		
		fMaxAmplitudeSqr = fmaxf(fAmplitudeSqr, fMaxAmplitudeSqr);
	}

	return sqrtf(fMaxAmplitudeSqr);	
}

float Simulator::UpdateSpringLength(unsigned int a_iStartIndex, unsigned int a_iCount)
{
	float			fTotalError		= 0.0f;
	unsigned int	iEndIndex		= a_iStartIndex + a_iCount;

	assert(iEndIndex <= m_iSpringCount);

	for (unsigned int iSpringIndex = a_iStartIndex; iSpringIndex < iEndIndex; ++iSpringIndex)
	{
		Spring* __restrict pSpring = &m_pSpringArray[iSpringIndex];

		//__dcbt(128, pSpring);

		unsigned int iVertexIndex0		= pSpring->m_iVertexIndex0;
		unsigned int iVertexIndex1		= pSpring->m_iVertexIndex1;

		Vector2&	vVertexPosition0	= m_pVertexPositionArray[iVertexIndex0];
		Vector2&	vVertexPosition1	= m_pVertexPositionArray[iVertexIndex1];

		//
		Vector2		vDistance		= vVertexPosition1 - vVertexPosition0;

		//float		fLengthRcp		= (float)__frsqrte((vDistance.x * vDistance.x) + (vDistance.y * vDistance.y));
		//float		fLength			= __fres(fLengthRcp);
		float		fLengthSqr		= (vDistance.x * vDistance.x) + (vDistance.y * vDistance.y);
		//_ASSERT(fLengthSqr == fLengthSqr); //fLengthSqr > 0.000001f);
		float		fLength			= sqrtf(fLengthSqr);
		float		fLengthRcp		= 1.0f / fLength;

		float		fError			= fabsf(pSpring->m_fDistance - pSpring->m_fPreferredDistance);

		pSpring->m_fDistance		= fLength;		
		pSpring->m_vDirection		= vDistance * fLengthRcp;
		
		fTotalError += fError;
	}

	return fTotalError; // / (float)(m_iDimension * m_iDimension);
}

void Simulator::CenterVertexGrid()
{
	Vector2 vTranslation = { 0.0f, 0.0f };

	for (unsigned int iVertexIndex = 0; iVertexIndex < m_iVertexCount; ++iVertexIndex)
	{
		vTranslation += m_pVertexPositionArray[iVertexIndex];
	}

	vTranslation = vTranslation / (float)m_iVertexCount;

	for (unsigned int iVertexIndex = 0; iVertexIndex < m_iVertexCount; ++iVertexIndex)
	{
		m_pVertexPositionArray[iVertexIndex] -= vTranslation;
	}
}

}

