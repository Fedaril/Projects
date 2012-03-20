
#include "StdAfx.h"

#include "MaterialConvertionData.h"

#include "stdlib.h"
#include "string.h"


unsigned int ComputeTileMaterialMask( const eSMaterialConvertionData* a_arrInfo, int a_iDimension, int a_iTileX, int a_iTileY, bool a_bLocked )
{
	eSMaterialConvertionData oData;	
	memset(&oData, 0, sizeof(oData));
	GetTileMaterialWeights(a_arrInfo, a_iDimension, a_iTileX, a_iTileY, 1, a_bLocked, &oData);
	return ComputeMaterialMaskFromWeights(&oData);
}



unsigned int ComputeMaterialMaskFromWeights( const eSMaterialConvertionData* a_pData )
{
	unsigned int u32MaterialMask = 0;
	unsigned int u32Mask = 0x01;

	for (unsigned int u32Index = 0; u32Index < iMaxMaterialCount; ++u32Index)
	{
		if (a_pData->m_arrMaterialWeight[u32Index] > 0)
		{
			u32MaterialMask |= u32Mask;
		}

		u32Mask = (u32Mask << 1);		
	}

	return u32MaterialMask;
}


int AddWeightsFromInfo( const eSMaterialConvertionData* a_pInfo, eSMaterialConvertionData* a_pData, int a_iWeightFactor )
{
	int iSumWeight = 0;

	for (unsigned int u32MaterialIndex = 0; u32MaterialIndex < iMaxMaterialCount; ++u32MaterialIndex)
	{
		int iWeight = a_pInfo->m_arrMaterialWeight[u32MaterialIndex] * a_iWeightFactor;
		iSumWeight += iWeight;
		a_pData->m_arrMaterialWeight[u32MaterialIndex] += iWeight;
		a_pData->m_arrMaterialColor[u32MaterialIndex] = 0;
	}	

	return iSumWeight;
}


void GetTileMaterialWeights( 
	const eSMaterialConvertionData* a_arrInfo, int a_iDimension, 
	int a_iTileX, int a_iTileY, int a_iBorderWeightFactor, bool a_bLocked,
	eSMaterialConvertionData* a_pData )
{
	ASSERT(a_iTileX >= 0);
	ASSERT(a_iTileY >= 0);

	int iPosX = a_iTileX * GE_TRTILE_TILESTRIDE;
	int iPosY = a_iTileY * GE_TRTILE_TILESTRIDE;

	ASSERT(iPosX < a_iDimension);
	ASSERT(iPosX < a_iDimension);

	//
	int iMaxOffsetX = GE_TRTILE_VERTEXSTRIDE;
	int iMaxOffsetY = GE_TRTILE_VERTEXSTRIDE;

	if (iPosX == (a_iDimension - 1))
	{
		iMaxOffsetX = 1;
	}
	if (iPosY == (a_iDimension - 1))
	{
		iMaxOffsetY = 1;
	}


	// top and left side 
	int iArrayOffset = (iPosY * a_iDimension) + iPosX;

	for (int iOffsetY = 0; iOffsetY < iMaxOffsetY; iOffsetY++)
	{
		for (int iOffsetX = 0; iOffsetX < iMaxOffsetX; iOffsetX++)
		{
			const eSMaterialConvertionData* pMaterialInfo = &a_arrInfo[iArrayOffset + (iOffsetY * a_iDimension) + iOffsetX];
			if (pMaterialInfo->m_bLocked == a_bLocked)
			{
				int iFactor = 1;

				if (	(iOffsetX == 0) 
					||	(iOffsetX == (GE_TRTILE_VERTEXSTRIDE - 1))
					||	(iOffsetY == 0)
					||	(iOffsetY == (GE_TRTILE_VERTEXSTRIDE - 1)))
				{
					iFactor = a_iBorderWeightFactor;
				}

				AddWeightsFromInfo( pMaterialInfo, a_pData, iFactor );
			}			
		}
	}
}

