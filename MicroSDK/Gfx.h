

#pragma once

#include "wtypes.h"


namespace Gfx
{

//
struct InputLayout;

struct VertexBuffer;
struct IndexBuffer;
struct ConstantBuffer;

struct VertexShader;
struct PixelShader;

struct Texture;




//
enum SamplerFilter
{
	SamplerFilter_Point,
	SamplerFilter_Bilinear,	
	SamplerFilter_Trilinear,	
	SamplerFilter_Anisotropic_8,
	SamplerFilter_Count
};

enum SamplerAddressMode
{
	SamplerAddressMode_Clamp,
	SamplerAddressMode_Wrap,	
	SamplerAddressMode_Count
};



//
enum InputElementSemantic
{
	InputElementSemantic_Position,
	InputElementSemantic_Texcoord,
	InputElementSemantic_Color,
	InputElementSemantic_Normal,

	InputElementSemantic_Count
};

enum InputElementFormat
{
	InputElementFormat_Float_1,
	InputElementFormat_Float_2,
	InputElementFormat_Float_3,
	InputElementFormat_Float_4,

	InputElementFormat_Float16_2,

	InputElementFormat_Byte_4_Unorm,

	InputElementFormat_Count
};

enum InputPrimitive
{
	InputPrimitive_PointList,
	InputPrimitive_LineList,
	InputPrimitive_TriangleList,

	InputPrimitive_Count
};


enum IndexType
{
	IndexType_16,
	IndexType_32,

	IndexType_Count
};


enum SurfaceFormat
{
	SurfaceFormat_R8G8B8A8,

	SurfaceFormat_R32F,
	SurfaceFormat_R16G16F,
	SurfaceFormat_R32G32F,
	SurfaceFormat_R32G32B32A32F,

	SurfaceFormat_DXT1,
	SurfaceFormat_DXT3,
	SurfaceFormat_DXT5,

	SurfaceFormat_Count
};


//
struct InputLayoutElement
{
	InputElementSemantic	m_eSemantic;
	unsigned int			m_iSemanticIndex;
	unsigned int			m_iOffset;
	InputElementFormat		m_eFormat;
};

struct LockInfo
{
	void*			m_pData;
	unsigned int	m_iSize;
};

//
void	SaveBufferAsDDS(void* a_pData, unsigned int a_iDataPitch, unsigned int a_iWidth, unsigned int a_iHeight, SurfaceFormat a_eFormat, const char* a_szFile);


//
void	InitializeD3D(HWND a_hWnd, RECT a_wr);
void	ShutdownD3D();


//
void	Begin();
void	Present();
void	Clear(float a_arrColor[4]);

//
void	CreatePixelShaderFromFile(const char* a_szFile, const char* a_szEntryPoint, PixelShader** a_ppShader);
void	CreateVertexShaderFromFile(const char* a_szFile, const char* a_szEntryPoint, VertexShader** a_ppShader);

void	SetVertexShader(VertexShader* a_pShader);
void	SetPixelShader(PixelShader* a_pShader);

//
void	CreateInputLayout(InputLayoutElement* a_arrInputLayoutElement, unsigned int a_iElementCount, VertexShader* a_pVertexShader, InputLayout** a_ppInputLayout);
void	SetInputLayout(InputLayout* a_ppInputLayout);

void	SetInputPrimitive(InputPrimitive a_ePrimitive);

//
void	CreateRenderTexture(SurfaceFormat a_eTextureFormat, SurfaceFormat a_eRenderTargetFormat, unsigned int a_iWidth, unsigned int a_iHeight, Texture** a_ppTexture);
void	CreateTextureFromFile(const char* a_szFile, Texture** a_ppTexture);

void	CreateDynamicTexture(SurfaceFormat a_eTextureFormat, unsigned int a_iWidth, unsigned int a_iHeight, Texture** a_ppTexture);
void	LockTexture(Texture* a_pDynamicTexture, LockInfo* a_pLock);
void	UnlockTexture(Texture* a_pDynamicTexture, LockInfo* a_pLock);


void	SetSamplerTexture(unsigned int a_iIndex, Texture* a_ppTexture);
void	SetSamplerConfiguration(unsigned int a_iIndex, SamplerFilter a_eFilter, SamplerAddressMode a_eAddressMode);



//
void	CreateIndexBuffer(IndexType a_eIndexType, unsigned int a_iIndexCount, IndexBuffer** a_ppIndexBuffer);
void	LockIndexBuffer(IndexBuffer* a_pIndexBuffer, LockInfo* a_pLock);
void	UnlockIndexBuffer(IndexBuffer* a_pIndexBuffer, LockInfo* a_pLock);

//
void	CreateVertexBuffer(unsigned int a_iElementSize, unsigned int a_iElementCount, VertexBuffer** a_ppVertexBuffer);
void	LockVertexBuffer(VertexBuffer* a_ppVertexBuffer, LockInfo* a_pLock);
void	UnlockVertexBuffer(VertexBuffer* a_ppVertexBuffer, LockInfo* a_pLock);

//
void	CreateConstantBuffer(unsigned int a_iConstantCount, ConstantBuffer** a_ppConstantBuffer);
void	LockConstantBuffer(ConstantBuffer* a_pConstantBuffer, LockInfo* a_pLock);
void	UnlockConstantBuffer(ConstantBuffer* a_pConstantBuffer, LockInfo* a_pLock);

//
void	SetVertexBuffer(unsigned int a_iIndex, VertexBuffer* a_pBuffer);
void	SetConstantBuffer(unsigned int a_iIndex, ConstantBuffer* a_pConstantBuffer);

//
void	DrawVertices(unsigned int a_iVertexCount);
void	DrawIndexedVertices(IndexBuffer* a_pIndexBuffer, unsigned int a_iIndexStart, unsigned int a_iIndexCount);

} // namespace Gfx