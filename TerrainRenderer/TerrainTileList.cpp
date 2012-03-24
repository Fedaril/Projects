

#include "StdAfx.h"

#include "TerrainTileList.h"



//***************************************************************************************************
//	Constructor
//***************************************************************************************************
eCTerrainTileList::eCTerrainTileList()
: m_iLodCount(0)
, m_pTiles(NULL)
//
, m_pActiveList(NULL)
, m_pActiveList1(NULL)
, m_pActiveList2(NULL)
//
, m_iListCount(0)
, m_iListCapacity(0)
//
//m_arrTileTypeCount[s_iMaxTypeCount];
//
//m_arrOffset[s_iMaxLodLevel + 1];
//m_arrDimension[s_iMaxLodLevel + 1];
//m_arrTileDimension[s_iMaxLodLevel + 1];
//m_arrThresholdSqr[s_iMaxLodLevel + 1];
//
//m_arrMaterialInfoArray[s_iMaterialInfoArrayCapacity];
//, m_iMaterialInfoArrayLength(0)
//
{
}

//***************************************************************************************************
//	Destructor
//***************************************************************************************************
eCTerrainTileList::~eCTerrainTileList()
{
	Clear();
}


//***************************************************************************************************
//	Initialize
//***************************************************************************************************
void eCTerrainTileList::Initialize(GEUInt a_iCapacity, const GEU32* a_arrTileMaterialTypeIndex, GEUInt a_iLodCount, GEUInt a_iBaseLodDimension)
{
	Clear();

	m_pActiveList1			= (eSTerrainTileData**)GE_MALLOC(sizeof(eSTerrainTileData*) * a_iCapacity);
	m_pActiveList2			= (eSTerrainTileData**)GE_MALLOC(sizeof(eSTerrainTileData*) * a_iCapacity);

	m_pActiveList		= m_pActiveList1;
	m_iListCapacity		= a_iCapacity;
	m_iListCount		= 0;

	m_iLodCount			= GEMin(a_iLodCount, s_iTileListMaxLodLevelCount);

	//
	GEUInt iArrayOffset		= 0;
	GEUInt iArrayDimension	= a_iBaseLodDimension;

	for (GEUInt iLodLevel = 0; iLodLevel <= s_iTileListMaxLodLevelCount; ++iLodLevel)
	{
		m_arrOffset[iLodLevel]			= iArrayOffset;
		m_arrDimension[iLodLevel]		= iArrayDimension;
		m_arrTileDimension[iLodLevel]	= s_iTileDimension << iLodLevel;

		GEUInt iDistanceThreshold = 16 + ((s_iTileDimension << iLodLevel) * 2);
		m_arrThresholdSqr[iLodLevel]		= iDistanceThreshold * iDistanceThreshold;

		iArrayOffset += (iArrayDimension * iArrayDimension);
		iArrayDimension = iArrayDimension >> 1;
	}

	CreateTiles(a_arrTileMaterialTypeIndex);

	CreateDefaultActiveTileList();
}

//***************************************************************************************************
//	Destroy
//***************************************************************************************************
void eCTerrainTileList::Clear()
{
	if (m_pTiles != NULL)
	{
		GE_FREE(m_pTiles);
		
	}

	if (m_pActiveList1 != NULL)
	{
		GE_FREE(m_pActiveList1);
	}

	if (m_pActiveList2 != NULL)
	{
		GE_FREE(m_pActiveList2);
	}


	m_pTiles					= NULL;
	m_pActiveList				= NULL;
	m_pActiveList1				= NULL;
	m_pActiveList2				= NULL;
	m_iListCapacity				= 0;
	m_iListCount				= 0;
	m_iLodCount					= 0;
}


//***************************************************************************************************
//	CreateTiles
//***************************************************************************************************
void eCTerrainTileList::CreateTiles(const GEU32* a_arrTileMaterialTypeIndex )
{
	unsigned int iTotalTileCount = GetLodArrayOffset(m_iLodCount);

	//
	m_pTiles = (eSTerrainTileData*)GE_MALLOC(sizeof(eSTerrainTileData) * iTotalTileCount);

	for (unsigned int iLodLevel = 0; iLodLevel < m_iLodCount; ++iLodLevel)
	{
		unsigned int iLevelDimension = GetLodArrayDimension(iLodLevel);
		unsigned int iTileBaseIndex = GetLodArrayOffset(iLodLevel);
		unsigned int iLevelIndex = 0;

		for (unsigned int iPosY = 0; iPosY < iLevelDimension; ++iPosY)
		{
			for (unsigned int iPosX = 0; iPosX < iLevelDimension; ++iPosX)
			{				
				eSTerrainTileData* pTile = &m_pTiles[iTileBaseIndex + iLevelIndex];

				GE_ASSERT(a_arrTileMaterialTypeIndex[iTileBaseIndex + iLevelIndex] < s_iMaxTypeCount);

				GE_ASSERT(iLodLevel < s_iTileListMaxLodLevelCount);

				pTile->asStruct.m_iLodLevel				= (GEU8)iLodLevel;
				pTile->asStruct.m_iPosX					= (GEU8)iPosX;
				pTile->asStruct.m_iPosY					= (GEU8)iPosY;
				pTile->asStruct.m_iMaterialType			= a_arrTileMaterialTypeIndex[iTileBaseIndex + iLevelIndex];
				pTile->asStruct.m_eState				= TileState_Disabled;

				iLevelIndex++;
			}
		}
	}
}

//***************************************************************************************************
//	CreateDefaultActiveTileList
//***************************************************************************************************
void eCTerrainTileList::CreateDefaultActiveTileList()
{
	GEUInt iTileDimension = m_arrDimension[m_iLodCount - 1];

	for (GEUInt iTileY = 0; iTileY < iTileDimension; ++iTileY)
	{
		for (GEUInt iTileX = 0; iTileX < iTileDimension; ++iTileX)
		{
			GEUInt iTileIndex = ComputeTileIndex(m_iLodCount - 1, iTileX, iTileY);
			m_pActiveList[m_iListCount] = &m_pTiles[iTileIndex];
			m_pActiveList[m_iListCount]->asStruct.m_eState = TileState_Enabled;
			m_iListCount++;
		}
	}
}

//***************************************************************************************************
//	CompactAndSortList
//***************************************************************************************************
void eCTerrainTileList::CompactAndSortList()
{
	//
	GEUInt iDrawBatchCount = 0;

	GEUInt iLastCount = 0;
	for (GEUInt iTypeIndex = 0; iTypeIndex < s_iMaxTypeCount; ++iTypeIndex)
	{
		GEUInt iCurrentCount = m_arrTileTypeCount[iTypeIndex];
		m_arrTileTypeCount[iTypeIndex] = iLastCount;
		iLastCount += iCurrentCount;
	}

	//
	eSTerrainTileData** pNextList = m_pActiveList == m_pActiveList1 ? m_pActiveList2 : m_pActiveList1;

	//
	GEUInt iListCount = m_iListCount;

	//
	eSTerrainTileData** pListElement = m_pActiveList;
	for (GEUInt iListIndex = 0; iListIndex < iListCount; ++iListIndex)
	{
		eSTerrainTileData* pTile = *pListElement;

		if (!IsSlotValueFree(pTile))
		{
			GE_ASSERT(pTile->asStruct.m_eState == TileState_Enabled);

			GEUInt iMaterialType = pTile->asStruct.m_iMaterialType;
			GEUInt iDstIndex = m_arrTileTypeCount[iMaterialType];
			m_arrTileTypeCount[iMaterialType] = iDstIndex + 1;
			pNextList[iDstIndex] = pTile;				
		}

		++pListElement;
	}

	//
	m_pActiveList			= pNextList;
	m_iListCount	= iLastCount;
}

//***************************************************************************************************
//	CompactList
//***************************************************************************************************
void eCTerrainTileList::CompactList()
{
	GEUInt iCount = m_iListCount;

	while ((iCount > 0) && (IsSlotFree(iCount - 1)))
	{
		iCount--;
	}		

	eSTerrainTileData** pListStart = m_pActiveList;
	eSTerrainTileData** pListEnd = m_pActiveList + (iCount - 1);


	for (GEUInt iIndex = 0; iIndex < iCount; ++iIndex)
	{
		GE_PREFETCH((GEU8*)pListEnd - 128);
		GE_PREFETCH((GEU8*)pListStart + 128);

		if (IsSlotValueFree(*pListStart))
		{				
			assert(pListStart != pListEnd);
			assert(!IsSlotValueFree(*pListEnd));				

			*pListStart = *pListEnd;
			--iCount;
			--pListEnd;

			while (IsSlotValueFree(*pListEnd))
			{
				--iCount;
				--pListEnd;
			}
		}

		pListStart++;
	}

	m_iListCount	= iCount;
}

//***************************************************************************************************
//	Update
//***************************************************************************************************
void eCTerrainTileList::Update( GEUInt a_iViewWorldPosX, GEUInt a_iViewWorldPosY )
{
	unsigned int	iVisibleTileIndex	= 0;

	GEMemSet(m_arrTileTypeCount, 0, sizeof(m_arrTileTypeCount));


	while ((iVisibleTileIndex < m_iListCount) && ((m_iListCount + 4) < m_iListCapacity))
	{
		PrefetchArray(iVisibleTileIndex + 16);
		PrefetchValue(iVisibleTileIndex + 2);

		GE_ASSERT(!IsSlotFree(iVisibleTileIndex));

		UpdateTile(iVisibleTileIndex, a_iViewWorldPosX, a_iViewWorldPosY);

		iVisibleTileIndex++;
	}

	CompactAndSortList();
}

//***************************************************************************************************
//	UpdateTile
//***************************************************************************************************
void eCTerrainTileList::UpdateTile(GEUInt a_iActiveListIndex, GEUInt a_iViewWorldPosX, GEUInt a_iViewWorldPosY)
{
	eSTerrainTileData* __restrict	pTile			= m_pActiveList[a_iActiveListIndex];
	unsigned int				iTileLodLevel		= pTile->asStruct.m_iLodLevel;
	unsigned int				iTilePosX			= pTile->asStruct.m_iPosX;
	unsigned int				iTilePosY			= pTile->asStruct.m_iPosY;
	unsigned int				eTileState			= pTile->asStruct.m_eState;

	//unsigned int				iTilePositionData	= pTile->asInt;
	//unsigned int				iTileLodLevel		= GET_LOD_LEVEL_FROM_INT_DATA(iTilePositionData); //pTile->m_iLodLevel;
	//unsigned int				iTilePosX			= GET_POS_X_FROM_INT_DATA(iTilePositionData); //pTile->m_iPosX;
	//unsigned int				iTilePosY			= GET_POS_Y_FROM_INT_DATA(iTilePositionData); //pTile->m_iPosY;
	//unsigned int				eTileState			= GET_STATE_FROM_INT_DATA(iTilePositionData);


	if (eTileState != TileState_Enabled)
	{
		SetFreeIndex(a_iActiveListIndex);
		return;
	}	

	if (iTileLodLevel > 0)
	{
		unsigned int	iDistanceFromTileSqr	= ComputeDistanceFromTileSqr(iTileLodLevel, iTilePosX, iTilePosY, a_iViewWorldPosX, a_iViewWorldPosY);
		unsigned int	iDistanceThresholdSqr	= GetDistanceThresholdSqr(iTileLodLevel); //pTile->m_iDistanceThreshold * pTile->m_iDistanceThreshold;

		if (iDistanceFromTileSqr <= iDistanceThresholdSqr)
		{
			// tile needs to be splitted

			unsigned int	iChildLodLevel			= iTileLodLevel - 1;
			unsigned int	iChildPosX 				= iTilePosX * 2;
			unsigned int	iChildPosY				= iTilePosY * 2;		

			unsigned int	iChild0TileIndex	= ComputeTileIndex(iChildLodLevel, iChildPosX, iChildPosY);
			unsigned int	iChild1TileIndex	= ComputeTileIndex(iChildLodLevel, iChildPosX + 1, iChildPosY);
			unsigned int	iChild2TileIndex	= ComputeTileIndex(iChildLodLevel, iChildPosX, iChildPosY + 1);
			unsigned int	iChild3TileIndex	= ComputeTileIndex(iChildLodLevel, iChildPosX + 1, iChildPosY + 1);

			//
			eSTerrainTileData*	pChild0Tile			= &m_pTiles[iChild0TileIndex];
			eSTerrainTileData*	pChild1Tile			= &m_pTiles[iChild1TileIndex];
			eSTerrainTileData*	pChild2Tile			= &m_pTiles[iChild2TileIndex];
			eSTerrainTileData*	pChild3Tile			= &m_pTiles[iChild3TileIndex];


			pTile->asStruct.m_eState		= TileState_ChildEnabled;

			pChild0Tile->asStruct.m_eState	= TileState_Enabled;
			pChild1Tile->asStruct.m_eState	= TileState_Enabled;
			pChild2Tile->asStruct.m_eState	= TileState_Enabled;
			pChild3Tile->asStruct.m_eState	= TileState_Enabled;


			SetFreeIndex(a_iActiveListIndex);

			//
			unsigned int iFreeIndex0 = InsertValue(pChild0Tile);
			unsigned int iFreeIndex1 = InsertValue(pChild1Tile);
			unsigned int iFreeIndex2 = InsertValue(pChild2Tile);
			unsigned int iFreeIndex3 = InsertValue(pChild3Tile);

			GE_UNUSED(iFreeIndex0);
			GE_ASSERT(iFreeIndex0 > a_iActiveListIndex);
			GE_UNUSED(iFreeIndex1);
			GE_ASSERT(iFreeIndex1 > a_iActiveListIndex);
			GE_UNUSED(iFreeIndex2);
			GE_ASSERT(iFreeIndex2 > a_iActiveListIndex);
			GE_UNUSED(iFreeIndex3);
			GE_ASSERT(iFreeIndex3 > a_iActiveListIndex);			
			return;
		}


		unsigned int	iChildLodLevel			= iTileLodLevel - 1;
		unsigned int	iChildPosX 				= iTilePosX * 2;
		unsigned int	iChildPosY				= iTilePosY * 2;

		unsigned int	iChild0TileIndex	= ComputeTileIndex(iChildLodLevel, iChildPosX, iChildPosY);
		unsigned int	iChild1TileIndex	= ComputeTileIndex(iChildLodLevel, iChildPosX + 1, iChildPosY);
		unsigned int	iChild2TileIndex	= ComputeTileIndex(iChildLodLevel, iChildPosX, iChildPosY + 1);
		unsigned int	iChild3TileIndex	= ComputeTileIndex(iChildLodLevel, iChildPosX + 1, iChildPosY + 1);

		//
		GE_ASSERT(m_pTiles[iChild0TileIndex].asStruct.m_eState == TileState_Disabled);
		GE_ASSERT(m_pTiles[iChild1TileIndex].asStruct.m_eState == TileState_Disabled);
		GE_ASSERT(m_pTiles[iChild2TileIndex].asStruct.m_eState == TileState_Disabled);
		GE_ASSERT(m_pTiles[iChild3TileIndex].asStruct.m_eState == TileState_Disabled);
	}


	if (iTileLodLevel < (m_iLodCount - 1))
	{
		unsigned int		iParentLodLevel		= iTileLodLevel + 1;
		unsigned int		iParentPosX			= iTilePosX / 2;
		unsigned int		iParentPosY			= iTilePosY / 2;
		unsigned int		iParentTileIndex	= ComputeTileIndex(iParentLodLevel, iParentPosX, iParentPosY);
		eSTerrainTileData*	pParentTile			= &m_pTiles[iParentTileIndex];

		unsigned int	iDistanceFromTileSqr			= ComputeDistanceFromTileSqr(iParentLodLevel, iParentPosX, iParentPosY, a_iViewWorldPosX, a_iViewWorldPosY);
		unsigned int	iParentDistanceThresholdSqr		= GetDistanceThresholdSqr(iParentLodLevel); //pParentTile->m_iDistanceThreshold * pParentTile->m_iDistanceThreshold;

		if (iDistanceFromTileSqr > iParentDistanceThresholdSqr) 
		{
			// tile need to be merged
			unsigned int	iSiblingPosX			= iParentPosX * 2;
			unsigned int	iSiblingPosY			= iParentPosY * 2;
			unsigned int	iSibling0TileIndex		= ComputeTileIndex(iTileLodLevel, iSiblingPosX, iSiblingPosY);
			unsigned int	iSibling1TileIndex		= ComputeTileIndex(iTileLodLevel, iSiblingPosX + 1, iSiblingPosY);
			unsigned int	iSibling2TileIndex		= ComputeTileIndex(iTileLodLevel, iSiblingPosX, iSiblingPosY + 1);
			unsigned int	iSibling3TileIndex		= ComputeTileIndex(iTileLodLevel, iSiblingPosX + 1, iSiblingPosY + 1);

			//
			eSTerrainTileData*	pSibling0Tile			= &m_pTiles[iSibling0TileIndex];
			eSTerrainTileData*	pSibling1Tile			= &m_pTiles[iSibling1TileIndex];
			eSTerrainTileData*	pSibling2Tile			= &m_pTiles[iSibling2TileIndex];
			eSTerrainTileData*	pSibling3Tile			= &m_pTiles[iSibling3TileIndex];

			pParentTile->asStruct.m_eState			= TileState_Enabled;

			pSibling0Tile->asStruct.m_eState		= TileState_Disabled;
			pSibling1Tile->asStruct.m_eState		= TileState_Disabled;
			pSibling2Tile->asStruct.m_eState		= TileState_Disabled;
			pSibling3Tile->asStruct.m_eState		= TileState_Disabled;

			SetFreeIndex(a_iActiveListIndex);

			unsigned int iNewIndex = InsertValue(pParentTile);
			GE_UNUSED(iNewIndex);
			GE_ASSERT(iNewIndex > a_iActiveListIndex);

			return;
		}

		GE_ASSERT(pParentTile->asStruct.m_eState == TileState_ChildEnabled);
	}

	// if we reach here, the tile will be drawn
	unsigned int iMaterialType = pTile->asStruct.m_iMaterialType;
	m_arrTileTypeCount[iMaterialType]++;


}

//***************************************************************************************************
//	ComputeTileChildIndex
//***************************************************************************************************
GEUInt eCTerrainTileList::ComputeTileChildIndex(GEUInt a_iLodLevel, GEUInt a_iPosX, GEUInt a_iPosY, GEUInt iChild)
{
	GEUInt iChildLodLevel = a_iLodLevel + 1;
	GEUInt iChildPosX = a_iPosX * 2;
	GEUInt iChildPosY = a_iPosY * 2;

	switch (iChild)
	{
	case 0:
		return ComputeTileIndex(iChildLodLevel, iChildPosX, iChildPosY);
	case 1:
		return ComputeTileIndex(iChildLodLevel, iChildPosX + 1, iChildPosY);
	case 2:
		return ComputeTileIndex(iChildLodLevel, iChildPosX, iChildPosY + 1);
	case 3:
		return ComputeTileIndex(iChildLodLevel, iChildPosX + 1, iChildPosY + 1);
	}

	
	return 0;
}

//***************************************************************************************************
//	ComputeParentTileIndex
//***************************************************************************************************
GEUInt eCTerrainTileList::ComputeParentTileIndex(GEUInt a_iLodLevel, GEUInt a_iPosX, GEUInt a_iPosY)
{
	GEUInt iParentLodLevel	= a_iLodLevel + 1;
	GEUInt iParentPosX		= a_iPosX / 2;
	GEUInt iParentPosY		= a_iPosY / 2;

	GE_ASSERT(a_iLodLevel < (m_iLodCount - 1));

	return ComputeTileIndex(iParentLodLevel, iParentPosX, iParentPosY);
}
