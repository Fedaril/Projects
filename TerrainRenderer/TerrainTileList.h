

#pragma once

#include "GenomeCompatibility.h"

enum eETileState
{
	TileState_Disabled		= 0,
	TileState_ChildEnabled	= 1,
	TileState_Enabled		= 2
};

struct eSTerrainTileData
{
	union
	{
		struct 
		{
			GEUInt			m_iPosX				: 8;
			GEUInt			m_iPosY				: 8;
			GEUInt			m_eState			: 2;
			GEUInt			m_iLodLevel			: 3; // 0-3
			GEUInt			m_iMaterialType		: 11; // 0 - 4095

		} asStruct;

		GEU32 asInt;
	};

	//unsigned short	m_iMaterialMask;
	//unsigned short	m_iDistanceThreshold;	
};

struct eSMaterialInfo
{
	unsigned int m_iMask;
	unsigned int m_iCount;
};

class eCTerrainTileList
{
public:
	static const GEUInt			s_iTileDimension = 4;
	static const GEUInt			s_iTileListMaxLodLevelCount = 4;
	static const GEUInt			s_iMaxTypeCount = 4096;

	static const GEUInt			s_iMaxDrawBatch = 2048;	
	static const GEUInt			s_iMaterialInfoArrayCapacity = 2048;

private:

	//
	GEUInt						m_iLodCount;
	eSTerrainTileData*			m_pTiles;

	//
	eSTerrainTileData**			m_pActiveList;
	eSTerrainTileData**			m_pActiveList1;
	eSTerrainTileData**			m_pActiveList2;

	//
	GEUInt						m_iListCount;
	GEUInt						m_iListCapacity;

	//
	GEUInt						m_arrTileTypeCount[s_iMaxTypeCount];

	//
	GEUInt						m_arrOffset[s_iTileListMaxLodLevelCount + 1];
	GEUInt						m_arrDimension[s_iTileListMaxLodLevelCount + 1];
	GEUInt						m_arrTileDimension[s_iTileListMaxLodLevelCount + 1];
	GEUInt						m_arrThresholdSqr[s_iTileListMaxLodLevelCount + 1];


public:
								eCTerrainTileList();
								~eCTerrainTileList();



	//
	void						Initialize					( GEUInt a_iCapacity, const GEU32* a_arrTileMaterialTypeIndex, GEUInt a_iLodCount, GEUInt a_iBaseLodDimension );
	void						Clear						( void );

	GEUInt						GetCapacity					( void ) const;
	GEUInt						GetCount					( void ) const;
	eSTerrainTileData**			GetList						( void ) const;	
	GEUInt						GetLodLevelCount			( void ) const;

	//
	GEUInt						ComputeTileIndex		(GEUInt	a_iLodLevel, GEUInt	a_iPosX, GEUInt	a_iPosY);
	GEUInt						ComputeTileChildIndex	(GEUInt	a_iLodLevel, GEUInt	a_iPosX, GEUInt	a_iPosY, GEUInt	iChild);
	GEUInt						ComputeParentTileIndex	(GEUInt	a_iLodLevel, GEUInt	a_iPosX, GEUInt	a_iPosY);

	const eSTerrainTileData*	GetTile						( GEUInt a_iTileIndex ) const;


	GEUInt						GetLodArrayOffset			(GEUInt a_iLodLevel) const;
	GEUInt						GetLodArrayDimension		(GEUInt a_iLodLevel) const;
	GEUInt						GetLodTileDimension			(GEUInt a_iLodLevel) const;
	GEUInt						GetDistanceThresholdSqr		(GEUInt a_iLodLevel) const;


	void						Update						( GEUInt a_iViewWorldPosX, GEUInt a_iViewWorldPosY );


	//const eSMaterialInfo&		GetMaterialInfo				( GEUInt a_iTypeIndex ) const;

private:

	//
	GEUInt						GetNextFreeIndex	( void );

	void						CreateTiles			( const GEU32* a_arrTileMaterialTypeIndex );
	void						CreateDefaultActiveTileList	( void );

	//GEUInt					FindMaterialInfoIndex	( GEU32 a_iMaterialMask );
	//void						CreateMaterialInfoArray	( const GEU16* a_arrTileMaterialMask, GEUInt a_iTileCount );

	void						SetFreeIndex		( GEUInt a_iFreeIndex );
	bool						IsSlotFree			( GEUInt a_iIndex );

	bool						IsSlotValueFree		( eSTerrainTileData* a_pValue );
	GEUInt						InsertValue			( eSTerrainTileData* a_pTile );

	eSTerrainTileData*			GetValue			(GEUInt a_iIndex);

	void						PrefetchArray		(GEUInt a_iIndex);
	void						PrefetchValue		(GEUInt a_iIndex);

	void						CompactAndSortList	( void );
	void						CompactList			( void );

	void						UpdateTile			( GEUInt a_iActiveListIndex, GEUInt a_iViewWorldPosX, GEUInt a_iViewWorldPosY );

	GEUInt						ComputeDistanceFromTileSqr(GEUInt a_iTileLodLevel, GEUInt a_iTilePosX, GEUInt a_iTilePosY, GEUInt a_iViewWorldPosX, GEUInt a_iViewWorldPosY);
};


//***************************************************************************************************
//	
//***************************************************************************************************
inline GEUInt eCTerrainTileList::ComputeTileIndex(GEUInt a_iLodLevel, GEUInt a_iPosX, GEUInt a_iPosY)
{
	GEUInt iArrayOffset = GetLodArrayOffset(a_iLodLevel);
	GEUInt iArrayDimension = GetLodArrayDimension(a_iLodLevel);

	GE_ASSERT(a_iPosX < iArrayDimension);
	GE_ASSERT(a_iPosY < iArrayDimension);

	GEUInt iOffset = iArrayOffset + (a_iPosY * iArrayDimension) + a_iPosX;

	GE_ASSERT(m_pTiles[iOffset].asStruct.m_iLodLevel == a_iLodLevel);
	GE_ASSERT(m_pTiles[iOffset].asStruct.m_iPosX == a_iPosX);
	GE_ASSERT(m_pTiles[iOffset].asStruct.m_iPosY == a_iPosY);

	return iOffset;

}

//***************************************************************************************************
//	
//***************************************************************************************************
inline const eSTerrainTileData* eCTerrainTileList::GetTile(GEUInt a_iTileIndex) const
{
	GE_ASSERT(a_iTileIndex < GetLodArrayOffset(m_iLodCount));
	return &m_pTiles[a_iTileIndex];
}


//***************************************************************************************************
//	
//***************************************************************************************************
inline GEUInt eCTerrainTileList::GetNextFreeIndex()
{
	GE_ASSERT(m_iListCount < m_iListCapacity);
	return m_iListCount++;
}

//***************************************************************************************************
//	
//***************************************************************************************************
inline GEUInt eCTerrainTileList::GetCapacity() const
{
	return m_iListCapacity;
}

//***************************************************************************************************
//	
//***************************************************************************************************
inline GEUInt eCTerrainTileList::GetCount() const
{
	return m_iListCount;
}

//***************************************************************************************************
//	
//***************************************************************************************************
inline eSTerrainTileData** eCTerrainTileList::GetList() const
{
	return m_pActiveList;
}

//***************************************************************************************************
//	
//***************************************************************************************************
inline GEUInt eCTerrainTileList::GetLodLevelCount( void ) const
{
	return m_iLodCount;
}

////***************************************************************************************************
////	
////***************************************************************************************************
//inline const eSMaterialInfo& eCTerrainTileList::GetMaterialInfo( GEUInt a_iTypeIndex ) const
//{
//	GE_ASSERT(m_iMaterialInfoArrayLength < s_iMaterialInfoArrayCapacity);
//	GE_ASSERT(a_iTypeIndex < m_iMaterialInfoArrayLength);
//	return m_arrMaterialInfoArray[a_iTypeIndex];
//}


//***************************************************************************************************
//	
//***************************************************************************************************
inline void eCTerrainTileList::SetFreeIndex(GEUInt a_iFreeIndex)
{
	GE_ASSERT(a_iFreeIndex < m_iListCapacity);
	m_pActiveList[a_iFreeIndex] = NULL;		
}

//***************************************************************************************************
//	
//***************************************************************************************************
inline bool eCTerrainTileList::IsSlotFree(GEUInt a_iIndex)
{
	GE_ASSERT(a_iIndex < m_iListCapacity);
	eSTerrainTileData* pValue = m_pActiveList[a_iIndex];
	return IsSlotValueFree(pValue);
}

//***************************************************************************************************
//	
//***************************************************************************************************
inline bool eCTerrainTileList::IsSlotValueFree(eSTerrainTileData* a_pValue)
{
	return a_pValue == NULL;
}

//***************************************************************************************************
//	
//***************************************************************************************************
inline GEUInt eCTerrainTileList::InsertValue(eSTerrainTileData* a_pTile)
{		
	GEUInt iNewIndex = GetNextFreeIndex();
	m_pActiveList[iNewIndex] = a_pTile;
	return iNewIndex;
}

//***************************************************************************************************
//	
//***************************************************************************************************
inline eSTerrainTileData* eCTerrainTileList::GetValue(GEUInt a_iIndex)
{		
	GE_ASSERT(a_iIndex < m_iListCapacity);
	GE_ASSERT(!IsSlotFree(a_iIndex));
	return m_pActiveList[a_iIndex];
}

//***************************************************************************************************
//	
//***************************************************************************************************
inline void eCTerrainTileList::PrefetchArray(GEUInt a_iIndex)
{
	GE_PREFETCH(m_pActiveList + a_iIndex);
}

//***************************************************************************************************
//	
//***************************************************************************************************
inline void eCTerrainTileList::PrefetchValue(GEUInt a_iIndex)
{
	if (!IsSlotFree(a_iIndex))
		GE_PREFETCH(m_pActiveList[a_iIndex]);
}


//***************************************************************************************************
//	
//***************************************************************************************************
inline GEUInt eCTerrainTileList::GetLodArrayOffset(GEUInt a_iLodLevel) const
{
	GE_ASSERT(m_iLodCount <= s_iTileListMaxLodLevelCount);
	GE_ASSERT(a_iLodLevel <= m_iLodCount);
	return m_arrOffset[a_iLodLevel];
}

//***************************************************************************************************
//	
//***************************************************************************************************
inline GEUInt eCTerrainTileList::GetLodArrayDimension(GEUInt a_iLodLevel) const
{
	GE_ASSERT(m_iLodCount <= s_iTileListMaxLodLevelCount);
	GE_ASSERT(a_iLodLevel <= m_iLodCount);
	return m_arrDimension[a_iLodLevel];
	//return OneShl8(a_iLodLevel - 3);
}

//***************************************************************************************************
//	
//***************************************************************************************************
inline GEUInt eCTerrainTileList::GetLodTileDimension(GEUInt a_iLodLevel) const
{
	GE_ASSERT(m_iLodCount <= s_iTileListMaxLodLevelCount);
	GE_ASSERT(a_iLodLevel <= m_iLodCount);
	return m_arrTileDimension[a_iLodLevel];
	//return OneShl8(a_iLodLevel - 3);
}

//***************************************************************************************************
//	
//***************************************************************************************************
inline GEUInt eCTerrainTileList::GetDistanceThresholdSqr(GEUInt a_iLodLevel) const
{
	GE_ASSERT(m_iLodCount <= s_iTileListMaxLodLevelCount);
	GE_ASSERT(a_iLodLevel <= m_iLodCount);
	return m_arrThresholdSqr[a_iLodLevel];
}

//***************************************************************************************************
//	
//***************************************************************************************************
GE_FORCE_INLINE GEUInt eCTerrainTileList::ComputeDistanceFromTileSqr(GEUInt a_iTileLodLevel, GEUInt a_iTilePosX, GEUInt a_iTilePosY, GEUInt a_iViewWorldPosX, GEUInt a_iViewWorldPosY)
{
	//unsigned int	iTileScale				= OneShl8(a_iTileLodLevel); // 1 << a_iTileLodLevel;
	unsigned int	iTileDimension			= GetLodTileDimension(a_iTileLodLevel); //s_iTileDimension * iTileScale;
	unsigned int	iHalfTileDimension		= iTileDimension / 2;
	unsigned int	iTileCenterPosX			= (a_iTilePosX * iTileDimension) + iHalfTileDimension;
	unsigned int	iTileCenterPosY			= (a_iTilePosY * iTileDimension) + iHalfTileDimension;
	unsigned int	iViewLocalPosX			= a_iViewWorldPosX;
	unsigned int	iViewLocalPosY			= a_iViewWorldPosY;
	int				iDistanceFromTileX		= (int)iViewLocalPosX - (int)iTileCenterPosX;
	int				iDistanceFromTileY		= (int)iViewLocalPosY - (int)iTileCenterPosY;
	unsigned int	iDistanceFromTileSqr	= (iDistanceFromTileX * iDistanceFromTileX) + (iDistanceFromTileY * iDistanceFromTileY);

	return iDistanceFromTileSqr;
}
