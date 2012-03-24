

#pragma once

const unsigned int GE_TRTILE_TILESTRIDE		= 4;
const unsigned int GE_TRTILE_VERTEXSTRIDE	= 5;

struct HeightMapData
{
	unsigned short		m_u16Height;
	unsigned char		m_u8MaterialIndex;
	unsigned char		m_u8Flags;	
	unsigned int		m_u32Normal;
};

struct HeightMapCropData
{
	unsigned int			m_iHeightmapDimension;
	const HeightMapData*	m_pHeightmap;

	unsigned int			m_iStartX;
	unsigned int			m_iStartY;

	unsigned int			m_iSizeX;
	unsigned int			m_iSizeY;
};