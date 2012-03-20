

#pragma once


#include "ForceField2DVector2.h"
#include "HeightmapData.h"

#if defined(_DEBUG)
#define DEBUG_SPRING_VERTEX
#endif


namespace ForceField2D
{

struct Spring;

class Simulator;
class Multithread;

enum SpringNeighbor
{
	SpringNeighbor_Top,
	SpringNeighbor_TopLeft,
	SpringNeighbor_TopRight,
	SpringNeighbor_Left,
	SpringNeighbor_Right,
	SpringNeighbor_Bottom,
	SpringNeighbor_BottomLeft,
	SpringNeighbor_BottomRight,

	SpringNeighbor_Count
};

enum SpringType
{
	SpringType_Right,
	SpringType_Bottom,
	SpringType_BottomRight,
	SpringType_BottomLeft_RightNeighbor, // bottom left string of right neighbor

	SpringType_Count,
};

struct SpringInfo
{		
	Spring*			m_arrNeighborSpring[SpringNeighbor_Count];	
};

struct VertexInitialInfo
{
	unsigned int	m_iArrayX;
	unsigned int	m_iArrayY;
	unsigned int	m_iMinHeight;
	unsigned int	m_iMaxHeight;
	unsigned int	m_iAverageHeight;	
};


struct Spring
{
	float			m_fPreferredDistance;
	float			m_fDistance;
	
	//float		m_fError;
	//Vector2			m_vPreferredDirection;
	Vector2			m_vDirection;


	unsigned int	m_iVertexIndex0;
	unsigned int	m_iVertexIndex1;
};


////
class Simulator
{	
public:
	static const unsigned int s_iInvalidArrayIndex = (unsigned int)-1;

private:
	friend class Multithread;

	unsigned int		m_iDimensionX;
	unsigned int		m_iDimensionY;
	unsigned int		m_iVertexCount;
	unsigned int		m_iSpringCount;
	float				m_fHeightScale;

	Vector2*			m_pVertexPositionArray;
	SpringInfo*			m_pVertexSpringInfoArray;			// (m_iDimension + 1)² elements
	VertexInitialInfo*	m_pVertexInitialInfoArray;

	//Vector2*			m_pForceArray;			// force applied to each vertex. (m_iDimension + 1)² elements	

	Spring*				m_pSpringArray;			// m_iDimension² elements
	float				m_fPositionScale;	

	float				m_fPreviousMaxForceAmplitude;

	float				m_fMaxDistanceRatio;

	//
	static const unsigned int	s_iErrorHistoryLength = 32;

	unsigned int			m_iErrorHistoryIndex;
	float					m_fErrorHistory[s_iErrorHistoryLength];

	//
	unsigned int			m_iIterationCount;

	//
	bool					m_bUseMultithread;
	Multithread*			m_pMultithread;


	float					m_arrSpringFactor[SpringNeighbor_Count];

public:
							Simulator();
							~Simulator();


	void					Initialize(
								unsigned int a_iDimensionX, unsigned int a_iDimensionY, 
								float a_fPositionScale, float a_fHeightScale, float a_fMaxDistanceRatio, 
								const HeightMapCropData* a_pHeightmapData, unsigned int a_iHeightmapStep);

	void					InitializeFromPreviousResult(
								const Simulator& a_oPreviousResult,
								const HeightMapCropData* a_pHeightmapData, unsigned int a_iHeightmapStep);

	void					Shutdown();

	bool					IsConverged();
	float					Simulate(float a_fDt);

	const Vector2*			GetVertexPositionArray() const;
	const Spring*			GetSpringArray() const;
	unsigned int			GetDimensionX() const;
	unsigned int			GetDimensionY() const;
	unsigned int			GetSpringCount() const;
	unsigned int			GetVertexCount() const;
	float					GetError() const;
	float					GetPositionScale() const;
	unsigned int			GetIterationCount() const;

private:
	static int				GetNeighborOffsetX(SpringNeighbor a_eNeighbor);
	static int				GetNeighborOffsetY(SpringNeighbor a_eNeighbor);

	static SpringNeighbor	GetNeighborOpposite(SpringNeighbor a_eNeighbor);

	unsigned int			GetVertexNeighborIndex(unsigned int a_iVertexPosX, unsigned int a_iVertexPosY, SpringNeighbor a_eNeighbor);
	Spring*					GetVertexSpringToNeighbor(unsigned int a_iVertexPosX, unsigned int a_iVertexPosY, SpringNeighbor a_eNeighbor);

	SpringNeighbor			FindVertexSpringNeighbor(SpringInfo* a_pVertex, Spring* a_pSpring);

	void					GatherHeightData(const HeightMapCropData* a_pHeightmapData, unsigned a_iStepSize, unsigned a_iPosX, unsigned a_iPosY, VertexInitialInfo* a_pInfo);
	void					FillInitialVertexArray(const HeightMapCropData* a_pHeightmapData, unsigned int a_iStepSize);

	void					CreateSpringArray();

	void					FillInitialSpringData(const HeightMapCropData* a_pHeightmapData, unsigned int a_iHeightmapStep);

	float					GatherSpringLengthData(
								const HeightMapData* a_pHeightmap, unsigned int a_iHeightmapDimension,
								unsigned int a_iHeightmapStartX, unsigned int a_iHeightmapStartY,	
								int a_iStepX, int a_iStepY, unsigned int a_iLength);

	float					ComputeIdealSpringLength(
								const HeightMapCropData* a_pHeightmapData, unsigned a_iHeightmapStep, 
								unsigned int a_iVertexIndex0, unsigned int a_iVertexIndex1);

	void					FillVertexArrayFromPreviousResult(const Simulator& a_oPreviousResult);

	//unsigned int			SetupVertexNeightborSpring(unsigned int a_iVertexArrayIndex, unsigned int a_iSpringIndex);
	void					SetupInsideQuad(const Simulator& a_oPreviousResult, unsigned int a_iPreviousPosX, unsigned int a_iPreviousPosY);
	void					SetupRightBorderQuad(const Simulator& a_oPreviousResult, unsigned int a_iPreviousPosY);
	void					SetupBottomBorderQuad(const Simulator& a_oPreviousResult, unsigned int a_iPreviousPosX);
	void					SetupBottomRightCornerQuad(const Simulator& a_oPreviousResult);


	///////
	float					ApplySpringForce(unsigned int a_iStartIndex, unsigned int a_iCount, float a_fUpdateStep);
	float					UpdateSpringLength(unsigned int a_iStartIndex, unsigned int a_iCount);


	void					CreateSpringNeighborFactorArray();

	void					InitializeErrorHistory();
	void					InitializeMultithread();

	void					CenterVertexGrid();
};

inline const Vector2* Simulator::GetVertexPositionArray() const
{
	return m_pVertexPositionArray;
}

inline const Spring* Simulator::GetSpringArray() const
{
	return m_pSpringArray;
}


inline unsigned int Simulator::GetDimensionX() const
{
	return m_iDimensionX;
}

inline unsigned int Simulator::GetDimensionY() const
{
	return m_iDimensionY;
}

inline unsigned int Simulator::GetSpringCount() const
{
	return m_iSpringCount;
}

inline unsigned int Simulator::GetVertexCount() const
{
	return m_iVertexCount;
}

inline float Simulator::GetError() const
{
	return m_fErrorHistory[m_iErrorHistoryIndex];
}

inline float Simulator::GetPositionScale() const
{
	return m_fPositionScale;
}

inline unsigned int Simulator::GetIterationCount() const
{
	return m_iIterationCount;
}


}