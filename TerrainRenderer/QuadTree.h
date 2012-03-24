

#pragma once

#include "assert.h"

#include "QuadTreeTypes.h"
#include "HeightmapData.h"
#include "MicroSDK/Gfx.h"
#include "TerrainTileList.h"

struct PatchVertex;
struct DrawBatch;
struct MaterialData;

enum VertexWeightLayout
{
	VertexWeightLayout_Xinfo,
	VertexWeightLayout_Solver,
	VertexWeightLayout_Count
};

struct HeightmapData
{
	MaterialData*		m_pMaterialDataArray;
	GEUInt				m_iMaterialDataArraySize;

	GEUInt*				m_pTileMaterialDataIndexArray;
	GEUInt				m_pTileMaterialDataIndexArraySize;

	GEU32*				m_pWeightArray;
	GEUInt				m_iWeightArraySize;
};

class PatchQuadTree
{
public:
	static const unsigned int s_iLodLevelCount = 4;
	static const unsigned int s_iDimension = 256;
	static const unsigned int s_iTileDimension = 4;

	static const unsigned int s_iMaxTextureCount = 15;
	static const unsigned int s_iMaxDrawBatch = 256;

	static const unsigned int s_iMaterialInfoArrayCapacity = 256;

private:
	MicroSDK::Gfx::InputLayout*				m_pInputLayout;
	MicroSDK::Gfx::VertexBuffer*				m_pTileVertexBuffer;
	MicroSDK::Gfx::IndexBuffer*				m_pTileIndexBuffer;

	MicroSDK::Gfx::VertexShader*				m_pVertexShader;
	MicroSDK::Gfx::PixelShader*				m_pPixelShader;

	MicroSDK::Gfx::Texture*					m_pTexture[s_iMaxTextureCount];
	MicroSDK::Gfx::Texture*					m_pIndirectMap;

	MicroSDK::Gfx::ConstantBuffer*			m_pConstantBuffer;
	MicroSDK::Gfx::ConstantBuffer*			m_pConstantBufferTextureScale;

	GEUInt							m_iHeightmapTileDimension;
	GEUInt							m_iHeightmapVertexDimension;
	VertexWeightLayout				m_eVertexWeightLayout;

	//
	HeightMapData*					m_pHeightmapData;
	GEUInt							m_iHeightmapDataSize;

	//
	HeightmapData					m_oData[VertexWeightLayout_Count];	


	//
	DrawBatch*						m_pDrawBatchList;
	GEUInt							m_iDrawBatchListCapacity;
	GEUInt							m_iDrawBatchListSize;


	static const unsigned int		s_iTileListCapacity = 16 * 1024;
	eCTerrainTileList				m_oTileList;


	//
	float							m_fPositionScale;
	float							m_fHeightScale;
	float							m_fSlopeScale;

	//
	bool							m_bWireFrameMode;
	bool							m_bUseDebugPixelShader;

	unsigned int					m_iVisibleTileCount;

	float							m_fUpdateTime;

private:


	void					CreateDrawData(eSTerrainTileData** __restrict a_pVisibleTile, unsigned int a_iVisibleTileCount);

	unsigned int			CreateTileVertexData(eSTerrainTileData* __restrict a_pTile, PatchVertex* a_pVertexBuffer, unsigned int a_iVertexIndex);
	unsigned int			CreateTileIndexData(eSTerrainTileData* __restrict a_pTile, unsigned int* a_pIndexBuffer, unsigned int a_iIndexStart, unsigned int a_iVertexOffset);

	void					SetTextureForMaterialType(const MaterialData* a_pMaterialData);

	void					CreateResources();

public:
	void					Initialize();

	//
	GEUInt					FindMaterialInfoIndex(VertexWeightLayout a_eLayout, GEU32 a_iMaterialMask, GEUInt a_iCurrentCount);
	void					CreateMaterialInfoArrayFromXinfo(const GEU16* a_arrTileMaterialMask, GEUInt a_iTileCount);
	GEUInt					CountXinfoMaterialData(const GEU16* a_arrTileMaterialMask, GEUInt a_iTileCount);
	void					FromXinfo(const char* a_szHmapFile);

	//
	void					FromSolverResult(const char* a_szSolverFile, GEUInt a_iBaseDimension, GEUInt a_iMaterialTypeCount);

	void					SetWorldScale(float a_fPositionScale, float a_fHeightScale, float a_fSlopeScale);

	void					Update(float a_fPosx, float a_fPosY, float a_fPosZ, float a_fYaw, float a_fPitch);
	void					Draw();

	//
	HeightMapData*			GetHeightMapDataAt(unsigned int a_iPosX, unsigned int a_iPosY);
	unsigned int			GetHeightAt(unsigned int a_iPosX, unsigned int a_iPosY);
	unsigned int			GetMaterialAt(unsigned int a_iPosX, unsigned int a_iPosY);
	unsigned int			GetFlagsAt(unsigned int a_iPosX, unsigned int a_iPosY);

	void					SetWireFrameMode(bool a_bEnable);
	bool					GetWireFrameMode() const;

	void					SetDebugPixelShader(bool a_bEnable);
	bool					GetDebugPixelShader() const;

	//	
	void					SetUpdateMode(unsigned int a_iMode);
	unsigned int			GetUpdateMode() const;

	//
	float					GetUpdateTime() const;

	//
	unsigned int			GetVisibleTileCount() const;

	const HeightMapData*	GetHeightmapData() const;

	VertexWeightLayout		GetVertexWeightLayout() const;
	void					SetVertexWeightLayout(VertexWeightLayout a_eVertexWeightLayout);
};

inline void PatchQuadTree::SetWireFrameMode(bool a_bEnable)
{
	m_bWireFrameMode = a_bEnable;
}

inline bool	PatchQuadTree::GetWireFrameMode() const
{
	return m_bWireFrameMode;
}


inline void PatchQuadTree::SetDebugPixelShader(bool a_bEnable)
{
	m_bUseDebugPixelShader = a_bEnable;
}

inline bool PatchQuadTree::GetDebugPixelShader() const
{
	return m_bUseDebugPixelShader;
}

inline float PatchQuadTree::GetUpdateTime() const
{
	return m_fUpdateTime;
}

inline unsigned int PatchQuadTree::GetVisibleTileCount() const
{
	return m_iVisibleTileCount;
}


inline const HeightMapData* PatchQuadTree::GetHeightmapData() const
{
	return m_pHeightmapData;
}

inline VertexWeightLayout PatchQuadTree::GetVertexWeightLayout() const
{
	return m_eVertexWeightLayout;
}
