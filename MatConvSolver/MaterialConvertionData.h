

#pragma once

const unsigned int iMaxMaterialCount = 15;
const unsigned int iInvalidMaterialIndex = 0xF;

const unsigned int GE_TRTILE_TILESTRIDE		= 4;
const unsigned int GE_TRTILE_VERTEXSTRIDE	= 5;


struct MaterialWeight
{
	static const unsigned int g_iMaxMaterialPerTile = 5;

	union
	{
		//
		struct 
		{
			unsigned char		m_w0;
			unsigned char		m_w1;
			unsigned char		m_w2;
			unsigned char		m_w3;
		};

		//
		unsigned char m_a[g_iMaxMaterialPerTile - 1];

		//
		unsigned int m_i;
	};
};

struct eSMaterialExtraInfo
{ 
	MaterialWeight m_arrSplatMask[GE_TRTILE_VERTEXSTRIDE][GE_TRTILE_VERTEXSTRIDE];
};

struct eSMaterialConvertionData
{
	int				m_arrMaterialWeight[iMaxMaterialCount];
	unsigned int	m_arrMaterialColor[iMaxMaterialCount];
	bool			m_bLocked;
};


struct eSMaterialData
{
	unsigned short		m_iHeight;
	unsigned char		m_iMaterialIndex;
	unsigned char		m_iFlags;	
	unsigned int		m_iPackedNormal;
};

unsigned int ComputeMaterialMaskFromWeights( const eSMaterialConvertionData* a_pData );

int AddWeightsFromInfo( const eSMaterialConvertionData* a_pInfo, eSMaterialConvertionData* a_pData, int a_iWeightFactor );

void GetTileMaterialWeights( 
		const eSMaterialConvertionData* a_arrInfo, int a_iDimension, 
		int a_iTileX, int a_iTileY, int a_iBorderWeightFactor, bool a_bLocked,
		eSMaterialConvertionData* a_pData );

unsigned int ComputeTileMaterialMask( const eSMaterialConvertionData* a_arrInfo, int a_iDimension, int a_iTileX, int a_iTileY, bool a_bLocked );

