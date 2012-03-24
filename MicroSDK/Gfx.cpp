
#include "StdAfx.h"

#include "Gfx.h"
#include "Util.h"
#include "Internal.h"

#include "windowsx.h"
#include "d3d11.h"
#include "d3dx11.h"


namespace MicroSDK
{
namespace Gfx
{

struct VertexShader
{
	ID3D10Blob*				m_pBlob;
	ID3D11VertexShader*		m_pShader;
};

struct PixelShader
{
	ID3D10Blob*				m_pBlob;
	ID3D11PixelShader*		m_pShader;
};

struct IndexBuffer
{
	D3D11_BUFFER_DESC		m_oDesc;
	ID3D11Buffer*			m_pBuffer;
	unsigned int			m_iIndexCount;
	unsigned int			m_iIndexSize;
	DXGI_FORMAT				m_iIndexFormat;
};

struct VertexBuffer
{
	D3D11_BUFFER_DESC		m_oDesc;
	ID3D11Buffer*			m_pBuffer;
	unsigned int			m_iVertexCount;
	unsigned int			m_iVertexSize;
};

struct ConstantBuffer
{
	D3D11_BUFFER_DESC		m_oDesc;
	ID3D11Buffer*			m_pBuffer;
};

struct InputLayout
{
	D3D11_INPUT_ELEMENT_DESC*	m_arrElement;
	ID3D11InputLayout*			m_pLayout;
	VertexShader*				m_pVertexShader;
};

struct Texture
{
	ID3D11Texture2D*			m_pStaticTexture2d;
	ID3D11Texture2D*			m_pDynamicTexture2d;

	ID3D11ShaderResourceView*	m_pShaderResourceView;
	ID3D11RenderTargetView*		m_pRenderTargetView;
};


// global declarations
IDXGISwapChain*				g_pSwapchain			= NULL;			// the pointer to the swap chain interface
ID3D11Device*				g_pDev					= NULL;				// the pointer to our Direct3D device interface
ID3D11DeviceContext*		g_pDevcon				= NULL;				// the pointer to our Direct3D device context
ID3D11RenderTargetView*		g_pBackbuffer			= NULL;			// global declaration
ID3D11DepthStencilView*		g_pDepthbuffer			= NULL;
ID3D11RasterizerState*		g_pRasterizerState		= NULL;
ID3D11DepthStencilState*	g_pDepthStencilState	= NULL;
ID3D11SamplerState*			g_arrSamplerState[SamplerFilter_Count][SamplerAddressMode_Count];

unsigned int				g_iFrontBufferWidth		= 0;
unsigned int				g_iFrontBufferHeight	= 0;

void CompileShaderFromFile(const char* a_szFile, const char* a_szEntryPoint, const char* a_szProfile, ID3D10Blob** a_ppResult)
{
	ID3D10Blob* pErrorMsgs;

	HRESULT hr = D3DX11CompileFromFileA(
		a_szFile,
		NULL,
		NULL,
		a_szEntryPoint,
		a_szProfile,
		D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION,
		0,
		NULL,
		a_ppResult,
		&pErrorMsgs,
		NULL );

	if (pErrorMsgs != NULL)
	{
		Util::PrintMessage("%s\n", pErrorMsgs->GetBufferPointer());
		pErrorMsgs->Release();
	}

	HALT_IF_FAILED(hr);
}

inline DXGI_FORMAT ConvertInputFormatToD3D11Format(InputElementFormat a_eFormat)
{
	switch (a_eFormat)
	{
		case InputElementFormat_Float_1:		return DXGI_FORMAT_R32_FLOAT;
		case InputElementFormat_Float_2:		return DXGI_FORMAT_R32G32_FLOAT;
		case InputElementFormat_Float_3:		return DXGI_FORMAT_R32G32B32_FLOAT;
		case InputElementFormat_Float_4:		return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case InputElementFormat_Float16_2:		return DXGI_FORMAT_R16G16_FLOAT;
		case InputElementFormat_Byte_4_Unorm:	return DXGI_FORMAT_R8G8B8A8_UNORM;
		default:								Util::Halt(); return DXGI_FORMAT_UNKNOWN;
	}
}

inline const char* ConvertInputSemanticTypeToD3D11Name(InputElementSemantic a_eSemantic)
{
	switch (a_eSemantic)
	{
		case InputElementSemantic_Position:		return "POSITION";
		case InputElementSemantic_Texcoord:		return "TEXCOORD";
		case InputElementSemantic_Color:		return "COLOR";
		case InputElementSemantic_Normal:		return "NORMAL";
		default:								Util::Halt(); return "INVALID";
	}
}

inline D3D11_PRIMITIVE_TOPOLOGY ConvertInputPrimitiveToD3D11Topology(InputPrimitive a_ePrimitive)
{
	switch (a_ePrimitive)
	{
		case InputPrimitive_PointList:		return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		case InputPrimitive_LineList:		return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		case InputPrimitive_TriangleList:	return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		default:							Util::Halt(); return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}
}


inline D3D11_FILTER GetD3D11Filter(SamplerFilter a_eFilter)
{
	switch (a_eFilter)
	{
		case SamplerFilter_Point:			return D3D11_FILTER_MIN_MAG_MIP_POINT;
		case SamplerFilter_Bilinear:		return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		case SamplerFilter_Trilinear:		return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		case SamplerFilter_Anisotropic_8:	return D3D11_FILTER_ANISOTROPIC;
		default:							Util::Halt(); return D3D11_FILTER_MIN_MAG_MIP_POINT;
	}
}

inline UINT GetD3D11Anisotropy(SamplerFilter a_eFilter)
{
	switch (a_eFilter)
	{
		case SamplerFilter_Point:			return 1;
		case SamplerFilter_Bilinear:		return 1;
		case SamplerFilter_Trilinear:		return 1;
		case SamplerFilter_Anisotropic_8:	return 8;
		default:							Util::Halt(); return 1;
	}
}


inline D3D11_TEXTURE_ADDRESS_MODE GetD3D11AddressMode(SamplerAddressMode a_eAddressMode)
{
	switch (a_eAddressMode)
	{
		case SamplerAddressMode_Wrap:		return D3D11_TEXTURE_ADDRESS_WRAP;
		case SamplerAddressMode_Clamp:		return D3D11_TEXTURE_ADDRESS_CLAMP;
		default:							Util::Halt(); return D3D11_TEXTURE_ADDRESS_CLAMP;
	}
}

ID3D11SamplerState* GetSamplerState(SamplerFilter a_eFilter, SamplerAddressMode a_eAddressMode)
{
	ID3D11SamplerState* pSamplerState = g_arrSamplerState[a_eFilter][a_eAddressMode];
	HALT_IF(pSamplerState == NULL);
	return pSamplerState;
}

DXGI_FORMAT GetDxgiFormatFromSurfaceFormat(SurfaceFormat a_eFormat)
{
	switch (a_eFormat)
	{
		case SurfaceFormat_R8G8B8A8:		return DXGI_FORMAT_R8G8B8A8_UNORM;
		case SurfaceFormat_R32F:			return DXGI_FORMAT_R32_FLOAT;
		case SurfaceFormat_R16G16F:			return DXGI_FORMAT_R16G16_FLOAT;
		case SurfaceFormat_R32G32F:			return DXGI_FORMAT_R32G32_FLOAT;
		case SurfaceFormat_R32G32B32A32F:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case SurfaceFormat_DXT1:			return DXGI_FORMAT_BC1_UNORM;
		case SurfaceFormat_DXT3:			return DXGI_FORMAT_BC2_UNORM;
		case SurfaceFormat_DXT5:			return DXGI_FORMAT_BC3_UNORM;
		default:							Util::Halt(); return DXGI_FORMAT_UNKNOWN;
	}	
}





void InitializeD3D(unsigned int a_iWidth, unsigned int a_iHeight)
{
	HWND hWnd = g_hApplicationWindow;	

	g_iFrontBufferWidth		= a_iWidth;
	g_iFrontBufferHeight	= a_iHeight;

	//
	HRESULT hr;

	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount			= 1;                                    // one back buffer
	scd.BufferDesc.Format	= DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow		= hWnd;                                // the window to be used
	scd.SampleDesc.Count	= 1;                               // how many multisamples
	scd.Windowed			= TRUE;                                    // windowed/full-screen mode

	// create a device, device context and swap chain using the information in the scd struct
	hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_DEBUG,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&g_pSwapchain,
		&g_pDev,
		NULL,
		&g_pDevcon);

	HALT_IF_FAILED(hr);

	hr = g_pSwapchain->ResizeBuffers(2, a_iWidth, a_iHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	HALT_IF_FAILED(hr);

	// get the address of the back buffer
	ID3D11Texture2D *pBackBuffer;
	g_pSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	// use the back buffer address to create the render target
	hr = g_pDev->CreateRenderTargetView(pBackBuffer, NULL, &g_pBackbuffer);
	HALT_IF_FAILED(hr);

	pBackBuffer->Release();


	//
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width					= a_iWidth;
	descDepth.Height				= a_iHeight;
	descDepth.MipLevels				= 1;
	descDepth.ArraySize				= 1;
	descDepth.Format				= DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count		= scd.SampleDesc.Count;
	descDepth.SampleDesc.Quality	= 0;
	descDepth.Usage					= D3D11_USAGE_DEFAULT;
	descDepth.BindFlags				= D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags		= 0;
	descDepth.MiscFlags				= 0;

	ID3D11Texture2D* pDepthStencilBuffer = NULL;
	hr = g_pDev->CreateTexture2D(&descDepth, NULL, &pDepthStencilBuffer);
	HALT_IF_FAILED(hr);

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));

	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	if( descDepth.SampleDesc.Count > 1 )
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	else
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	// Create the depth stencil view
	hr = g_pDev->CreateDepthStencilView(pDepthStencilBuffer, &descDSV, &g_pDepthbuffer);
	HALT_IF_FAILED(hr);


	//
	D3D11_RASTERIZER_DESC oRasterizerDesc;
	ZeroMemory(&oRasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

	oRasterizerDesc.FillMode				= D3D11_FILL_SOLID;
	oRasterizerDesc.CullMode				= D3D11_CULL_NONE;
	oRasterizerDesc.FrontCounterClockwise	= TRUE;
	oRasterizerDesc.DepthBias				= 0;
	oRasterizerDesc.DepthBiasClamp			= 0.0f;
	oRasterizerDesc.SlopeScaledDepthBias	= 0.0f;
	oRasterizerDesc.DepthClipEnable			= FALSE;
	oRasterizerDesc.ScissorEnable			= FALSE;
	oRasterizerDesc.MultisampleEnable		= FALSE;
	oRasterizerDesc.AntialiasedLineEnable	= FALSE;

	hr = g_pDev->CreateRasterizerState(&oRasterizerDesc, &g_pRasterizerState);
	HALT_IF_FAILED(hr);

	//
	D3D11_DEPTH_STENCIL_DESC oDepthStencilDesc;
	ZeroMemory(&oDepthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));


	oDepthStencilDesc.DepthEnable					= TRUE;
	oDepthStencilDesc.DepthWriteMask				= D3D11_DEPTH_WRITE_MASK_ALL;
	oDepthStencilDesc.DepthFunc						= D3D11_COMPARISON_LESS_EQUAL; //D3D11_COMPARISON_LESS_EQUAL; //D3D11_COMPARISON_GREATER_EQUAL;
	oDepthStencilDesc.StencilEnable					= FALSE;
	oDepthStencilDesc.StencilReadMask				= 0;
	oDepthStencilDesc.StencilWriteMask				= 0;
	oDepthStencilDesc.FrontFace.StencilDepthFailOp	= D3D11_STENCIL_OP_KEEP;
	oDepthStencilDesc.FrontFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
	oDepthStencilDesc.FrontFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
	oDepthStencilDesc.FrontFace.StencilFunc			= D3D11_COMPARISON_NEVER;
	oDepthStencilDesc.BackFace.StencilDepthFailOp	= D3D11_STENCIL_OP_KEEP;
	oDepthStencilDesc.BackFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
	oDepthStencilDesc.BackFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
	oDepthStencilDesc.BackFace.StencilFunc			= D3D11_COMPARISON_NEVER;		


	hr = g_pDev->CreateDepthStencilState(&oDepthStencilDesc, &g_pDepthStencilState);
	HALT_IF_FAILED(hr);	


	///
	for (int iSamplerFilter = 0; iSamplerFilter < SamplerFilter_Count; ++iSamplerFilter)
	{
		for (int iSamplerAddressMode = 0; iSamplerAddressMode < SamplerAddressMode_Count; ++iSamplerAddressMode)
		{
			D3D11_SAMPLER_DESC oDesc;
			ZeroMemory(&oDesc, sizeof(oDesc));

			oDesc.Filter			= GetD3D11Filter((SamplerFilter)iSamplerFilter);
			oDesc.AddressU			= GetD3D11AddressMode((SamplerAddressMode)iSamplerAddressMode);
			oDesc.AddressV			= GetD3D11AddressMode((SamplerAddressMode)iSamplerAddressMode);
			oDesc.AddressW			= GetD3D11AddressMode((SamplerAddressMode)iSamplerAddressMode);
			oDesc.MipLODBias		= 0.0f;
			oDesc.MaxAnisotropy		= GetD3D11Anisotropy((SamplerFilter)iSamplerFilter);
			oDesc.ComparisonFunc	= D3D11_COMPARISON_NEVER;
			oDesc.BorderColor[0]	= 1.0f;
			oDesc.BorderColor[1]	= 1.0f;
			oDesc.BorderColor[2]	= 1.0f;
			oDesc.BorderColor[3]	= 1.0f;
			oDesc.MinLOD			= 0.0f;
			oDesc.MaxLOD			= D3D11_FLOAT32_MAX;

			hr = g_pDev->CreateSamplerState(&oDesc, &g_arrSamplerState[iSamplerFilter][iSamplerAddressMode]);
			HALT_IF_FAILED(hr);	
		}
	}



}

// this is the function that cleans up Direct3D and COM
void ShutdownD3D()
{
	// close and release all existing COM objects
	g_pSwapchain->Release();
	g_pBackbuffer->Release();
	g_pDev->Release();
	g_pDevcon->Release();
}


void Begin()
{
	// set the render target as the back buffer
	g_pDevcon->OMSetRenderTargets(1, &g_pBackbuffer, g_pDepthbuffer);

	// Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (FLOAT)g_iFrontBufferWidth;
	viewport.Height = (FLOAT)g_iFrontBufferHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;


	g_pDevcon->RSSetViewports(1, &viewport);

	g_pDevcon->RSSetState(g_pRasterizerState);	
	g_pDevcon->OMSetDepthStencilState(g_pDepthStencilState, 0);
	g_pDevcon->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);

}

// this is the function used to render a single frame
void Present()
{	
	// switch the back buffer and the front buffer
	g_pSwapchain->Present(0, 0);
}


void Clear(float arrColor[4])
{
	g_pDevcon->ClearRenderTargetView(g_pBackbuffer, arrColor);

	static float s_fClearDepth = 1.0f;

	g_pDevcon->ClearDepthStencilView(g_pDepthbuffer, D3D11_CLEAR_DEPTH, s_fClearDepth, 0);
}


//
void CreatePixelShaderFromFile(const char* a_szFile, const char* a_szEntryPoint, PixelShader** a_ppPixelShader)
{
	ID3D10Blob* pShaderData;

	CompileShaderFromFile(a_szFile, a_szEntryPoint, "ps_4_0", &pShaderData);


	ID3D11PixelShader* pPS;
	const void*		pShaderBytecode	= pShaderData->GetBufferPointer();
	SIZE_T			iBytecodeLength = pShaderData->GetBufferSize();

	HRESULT hr = g_pDev->CreatePixelShader(pShaderBytecode, iBytecodeLength, NULL, &pPS);

	HALT_IF_FAILED(hr);


	*a_ppPixelShader = (PixelShader*)calloc(1, sizeof(PixelShader));

	(*a_ppPixelShader)->m_pBlob = pShaderData;
	(*a_ppPixelShader)->m_pShader = pPS;
}

void CreateVertexShaderFromFile(const char* a_szFile, const char* a_szEntryPoint, VertexShader** a_ppVertexShader)
{
	ID3D10Blob* pShaderData;

	CompileShaderFromFile(a_szFile, a_szEntryPoint, "vs_4_0", &pShaderData);

	ID3D11VertexShader*		pVS = NULL;
	const void*				pShaderBytecode	= pShaderData->GetBufferPointer();
	SIZE_T					iBytecodeLength = pShaderData->GetBufferSize();

	HRESULT hr = g_pDev->CreateVertexShader(pShaderBytecode, iBytecodeLength, NULL, &pVS);

	HALT_IF_FAILED(hr);

	*a_ppVertexShader = (VertexShader*)calloc(1, sizeof(VertexShader));

	(*a_ppVertexShader)->m_pBlob = pShaderData;
	(*a_ppVertexShader)->m_pShader = pVS;
}

void SetVertexShader(VertexShader* a_pShader)
{
	g_pDevcon->VSSetShader(a_pShader->m_pShader, NULL, 0);
}

void SetPixelShader(PixelShader* a_pShader)
{
	g_pDevcon->PSSetShader(a_pShader->m_pShader, NULL, 0);
}


//
void CreateInputLayout(InputLayoutElement* a_arrInputLayoutElement, unsigned int a_iElementCount, VertexShader* a_pVertexShader, InputLayout** a_ppInputLayout)
{
	D3D11_INPUT_ELEMENT_DESC* arrElement = (D3D11_INPUT_ELEMENT_DESC*)calloc(a_iElementCount, sizeof(D3D11_INPUT_ELEMENT_DESC));


	for (unsigned int iElementIndex = 0; iElementIndex < a_iElementCount; ++iElementIndex)
	{
		InputLayoutElement* pIn				= &a_arrInputLayoutElement[iElementIndex];
		D3D11_INPUT_ELEMENT_DESC* pOut		= &arrElement[iElementIndex];

		pOut->SemanticName				= ConvertInputSemanticTypeToD3D11Name(pIn->m_eSemantic);
		pOut->SemanticIndex				= pIn->m_iSemanticIndex;
		pOut->Format					= ConvertInputFormatToD3D11Format(pIn->m_eFormat);
		pOut->InputSlot					= 0;
		pOut->AlignedByteOffset			= pIn->m_iOffset;
		pOut->InputSlotClass			= D3D11_INPUT_PER_VERTEX_DATA;
		pOut->InstanceDataStepRate		= 0;
	}

	ID3D11InputLayout* pInputLayout = NULL;

	HRESULT hr = g_pDev->CreateInputLayout(
		arrElement, a_iElementCount, 
		a_pVertexShader->m_pBlob->GetBufferPointer(), a_pVertexShader->m_pBlob->GetBufferSize(),
		&pInputLayout);

	HALT_IF_FAILED(hr);

	(*a_ppInputLayout) = (InputLayout*)calloc(1, sizeof(InputLayout));
	(*a_ppInputLayout)->m_arrElement	= arrElement;
	(*a_ppInputLayout)->m_pLayout		= pInputLayout;
	(*a_ppInputLayout)->m_pVertexShader	= a_pVertexShader;
}

void SetInputLayout(InputLayout* a_pInputLayout)
{
	g_pDevcon->IASetInputLayout(a_pInputLayout->m_pLayout);
}

void SetInputPrimitive(InputPrimitive a_ePrimitive)
{
	D3D11_PRIMITIVE_TOPOLOGY eTopology = ConvertInputPrimitiveToD3D11Topology(a_ePrimitive);
	g_pDevcon->IASetPrimitiveTopology(eTopology);
}

//////////////


void CreateRenderTexture(SurfaceFormat a_eTextureFormat, SurfaceFormat a_eRenderTargetFormat, unsigned int a_iWidth, unsigned int a_iHeight, Texture** a_ppTexture)
{
	D3D11_TEXTURE2D_DESC oDesc;
	memset(&oDesc, 0, sizeof(oDesc));

	oDesc.Width					= a_iWidth;
	oDesc.Height				= a_iHeight;
	oDesc.MipLevels				= 1;
	oDesc.ArraySize				= 0;
	oDesc.Format				= GetDxgiFormatFromSurfaceFormat(a_eTextureFormat);
	oDesc.SampleDesc.Count		= 1;
	oDesc.SampleDesc.Quality	= 0;
	oDesc.Usage					= D3D11_USAGE_DEFAULT; 
	oDesc.BindFlags				= D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	oDesc.CPUAccessFlags		= 0;
	oDesc.MiscFlags				= 0;

	ID3D11Texture2D* pTexture2d = NULL;
	HRESULT hr = g_pDev->CreateTexture2D( &oDesc, NULL, &pTexture2d);
	HALT_IF_FAILED(hr);

	//
	D3D11_SHADER_RESOURCE_VIEW_DESC oViewDesc;
	memset(&oViewDesc, 0, sizeof(oViewDesc));

	oViewDesc.Format					= GetDxgiFormatFromSurfaceFormat(a_eTextureFormat);
	oViewDesc.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURE2D;
	oViewDesc.Texture2D.MipLevels		= 1;
	oViewDesc.Texture2D.MostDetailedMip	= 0;

	ID3D11ShaderResourceView* pShaderResourceView = NULL;
	hr = g_pDev->CreateShaderResourceView( pTexture2d, &oViewDesc, &pShaderResourceView );
	HALT_IF_FAILED(hr);

	//
	D3D11_RENDER_TARGET_VIEW_DESC oRenderTargetDesc;
	memset(&oRenderTargetDesc, 0, sizeof(oRenderTargetDesc));

	oRenderTargetDesc.Format				= GetDxgiFormatFromSurfaceFormat(a_eRenderTargetFormat);
	oRenderTargetDesc.ViewDimension			= D3D11_RTV_DIMENSION_TEXTURE2D;
	oRenderTargetDesc.Texture2D.MipSlice	= 0;

	ID3D11RenderTargetView* pRenderTargetView = NULL;
	hr = g_pDev->CreateRenderTargetView( pTexture2d, &oRenderTargetDesc, &pRenderTargetView );
	HALT_IF_FAILED(hr);



	*a_ppTexture = (Texture*)calloc(1, sizeof(Texture));
	(*a_ppTexture)->m_pStaticTexture2d		= pTexture2d;
	(*a_ppTexture)->m_pShaderResourceView	= pShaderResourceView;
	(*a_ppTexture)->m_pRenderTargetView		= pRenderTargetView;

}

//
void CreateTextureFromFile(const char* a_szFile, Texture** a_ppTexture)
{
	ID3D11ShaderResourceView* pShaderResourceView = NULL;

	HRESULT hr = D3DX11CreateShaderResourceViewFromFileA(g_pDev, a_szFile, NULL, NULL, &pShaderResourceView, NULL);
	HALT_IF_FAILED(hr);
	*a_ppTexture = (Texture*)calloc(1, sizeof(Texture));
	(*a_ppTexture)->m_pShaderResourceView = pShaderResourceView;
}


void CreateDynamicTexture(SurfaceFormat a_eTextureFormat, unsigned int a_iWidth, unsigned int a_iHeight, Texture** a_ppTexture)
{
	HRESULT hr;

	D3D11_TEXTURE2D_DESC oDynamicDesc;
	memset(&oDynamicDesc, 0, sizeof(oDynamicDesc));

	oDynamicDesc.Width					= a_iWidth;
	oDynamicDesc.Height					= a_iHeight;
	oDynamicDesc.MipLevels				= 1;
	oDynamicDesc.ArraySize				= 1;
	oDynamicDesc.Format					= GetDxgiFormatFromSurfaceFormat(a_eTextureFormat);
	oDynamicDesc.SampleDesc.Count		= 1;
	oDynamicDesc.SampleDesc.Quality		= 0;
	oDynamicDesc.Usage					= D3D11_USAGE_DYNAMIC; 
	oDynamicDesc.BindFlags				= D3D11_BIND_SHADER_RESOURCE;
	oDynamicDesc.CPUAccessFlags			= D3D11_CPU_ACCESS_WRITE;
	oDynamicDesc.MiscFlags				= 0;

	ID3D11Texture2D* pTexture2d = NULL;
	hr = g_pDev->CreateTexture2D( &oDynamicDesc, NULL, &pTexture2d);
	HALT_IF_FAILED(hr);

	//
	D3D11_SHADER_RESOURCE_VIEW_DESC oViewDesc;
	memset(&oViewDesc, 0, sizeof(oViewDesc));

	oViewDesc.Format					= GetDxgiFormatFromSurfaceFormat(a_eTextureFormat);
	oViewDesc.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURE2D;
	oViewDesc.Texture2D.MipLevels		= 1;
	oViewDesc.Texture2D.MostDetailedMip	= 0;

	ID3D11ShaderResourceView* pShaderResourceView = NULL;
	hr = g_pDev->CreateShaderResourceView( pTexture2d, &oViewDesc, &pShaderResourceView );
	HALT_IF_FAILED(hr);

	*a_ppTexture = (Texture*)calloc(1, sizeof(Texture));
	(*a_ppTexture)->m_pDynamicTexture2d		= pTexture2d;
	(*a_ppTexture)->m_pStaticTexture2d		= pTexture2d;
	(*a_ppTexture)->m_pShaderResourceView	= pShaderResourceView;
}

void LockTexture(Texture* a_pDynamicTexture, LockInfo* a_pLock)
{
	HALT_IF(a_pDynamicTexture->m_pDynamicTexture2d == NULL);

	D3D11_MAPPED_SUBRESOURCE oMap;
	ZeroMemory(&oMap, sizeof(D3D11_MAPPED_SUBRESOURCE));

	HRESULT hr = g_pDevcon->Map(a_pDynamicTexture->m_pDynamicTexture2d, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMap);   // map the buffer
	HALT_IF_FAILED(hr);

	a_pLock->m_pData = oMap.pData;
	a_pLock->m_iSize = oMap.RowPitch;
}

void UnlockTexture(Texture* a_pDynamicTexture, LockInfo* a_pLock)
{
	HALT_IF(a_pDynamicTexture->m_pDynamicTexture2d == NULL);

	g_pDevcon->Unmap(a_pDynamicTexture->m_pDynamicTexture2d, 0);

	a_pLock->m_iSize = 0;
	a_pLock->m_pData = NULL;
}

void SaveBufferAsDDS(void* a_pData, unsigned int a_iDataPitch, unsigned int a_iWidth, unsigned int a_iHeight, SurfaceFormat a_eFormat, const char* a_szFile)
{
	D3D11_TEXTURE2D_DESC oDesc;
	D3D11_SUBRESOURCE_DATA oInitialData;

	oDesc.Width					= a_iWidth;
	oDesc.Height				= a_iHeight;
	oDesc.MipLevels				= 1;
	oDesc.ArraySize				= 1;
	oDesc.Format				= GetDxgiFormatFromSurfaceFormat(a_eFormat);
	oDesc.SampleDesc.Count		= 1;
	oDesc.SampleDesc.Quality	= 0;
	oDesc.Usage					= D3D11_USAGE_IMMUTABLE;
	oDesc.BindFlags				= D3D11_BIND_SHADER_RESOURCE;
	oDesc.MiscFlags				= 0;
	oDesc.CPUAccessFlags		= 0;

	oInitialData.pSysMem			= a_pData;
	oInitialData.SysMemPitch		= a_iDataPitch;
	oInitialData.SysMemSlicePitch	= 0;


	ID3D11Texture2D* pTexture;
	HRESULT hr = Gfx::g_pDev->CreateTexture2D(&oDesc, &oInitialData, &pTexture);
	HALT_IF_FAILED(hr);

	hr = D3DX11SaveTextureToFileA(g_pDevcon, pTexture, D3DX11_IFF_DDS, a_szFile);
	HALT_IF_FAILED(hr);

	pTexture->Release();
}

void SetSamplerTexture(unsigned int a_iIndex, Texture* a_pTexture)
{
	ID3D11ShaderResourceView* pView = NULL;
	if (a_pTexture != NULL)
		pView = a_pTexture->m_pShaderResourceView;

	g_pDevcon->PSSetShaderResources(a_iIndex, 1, &pView);
}

void SetSamplerConfiguration(unsigned int a_iIndex, SamplerFilter a_eFilter, SamplerAddressMode a_eAddressMode)
{
	ID3D11SamplerState* pState = GetSamplerState(a_eFilter, a_eAddressMode);
	g_pDevcon->PSSetSamplers(a_iIndex, 1, &pState);
}



//
void CreateIndexBuffer(IndexType a_eIndexType, unsigned int a_iIndexCount, IndexBuffer** a_ppIndexBuffer)
{
	ID3D11Buffer* pVBuffer = NULL;

	D3D11_BUFFER_DESC oDesc;
	ZeroMemory(&oDesc, sizeof(oDesc));

	unsigned int iIndexSize = a_eIndexType == IndexType_16 ? 2 : 4;

	oDesc.Usage				= D3D11_USAGE_DYNAMIC;					// write access access by CPU and GPU
	oDesc.ByteWidth			= a_iIndexCount * iIndexSize;			// size is the VERTEX struct * 3
	oDesc.BindFlags			= D3D11_BIND_INDEX_BUFFER;				// use as a index buffer
	oDesc.CPUAccessFlags	= D3D11_CPU_ACCESS_WRITE;				// allow CPU to write in buffer

	HRESULT hr = g_pDev->CreateBuffer(&oDesc, NULL, &pVBuffer);		// create the buffer
	HALT_IF_FAILED(hr);

	//
	(*a_ppIndexBuffer) = (IndexBuffer*)calloc(1, sizeof(IndexBuffer));
	(*a_ppIndexBuffer)->m_oDesc				= oDesc;
	(*a_ppIndexBuffer)->m_pBuffer			= pVBuffer;
	(*a_ppIndexBuffer)->m_iIndexCount		= a_iIndexCount;
	(*a_ppIndexBuffer)->m_iIndexSize		= iIndexSize;
	(*a_ppIndexBuffer)->m_iIndexFormat		= a_eIndexType == IndexType_16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
}


void LockIndexBuffer(IndexBuffer* a_pIndexBuffer, LockInfo* a_pLock)
{
	D3D11_MAPPED_SUBRESOURCE oMap;
	ZeroMemory(&oMap, sizeof(D3D11_MAPPED_SUBRESOURCE));

	HRESULT hr = g_pDevcon->Map(a_pIndexBuffer->m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMap);   // map the buffer
	HALT_IF_FAILED(hr);

	a_pLock->m_pData = oMap.pData;
	a_pLock->m_iSize = oMap.RowPitch;
}

void UnlockIndexBuffer(IndexBuffer* a_pIndexBuffer, LockInfo* a_pLock)
{
	g_pDevcon->Unmap(a_pIndexBuffer->m_pBuffer, 0);

	a_pLock->m_iSize = 0;
	a_pLock->m_pData = NULL;
}


//
void CreateVertexBuffer(unsigned int a_iVertexSize, unsigned int a_iVertexCount, VertexBuffer** a_ppVertexBuffer)
{
	ID3D11Buffer* pVBuffer = NULL;

	D3D11_BUFFER_DESC oDesc;
	ZeroMemory(&oDesc, sizeof(oDesc));

	oDesc.Usage				= D3D11_USAGE_DYNAMIC;					// write access access by CPU and GPU
	oDesc.ByteWidth			= a_iVertexSize * a_iVertexCount;		// size is the VERTEX struct * 3
	oDesc.BindFlags			= D3D11_BIND_VERTEX_BUFFER;				// use as a vertex buffer
	oDesc.CPUAccessFlags	= D3D11_CPU_ACCESS_WRITE;				// allow CPU to write in buffer

	HRESULT hr = g_pDev->CreateBuffer(&oDesc, NULL, &pVBuffer);					// create the buffer
	HALT_IF_FAILED(hr);

	//
	(*a_ppVertexBuffer) = (VertexBuffer*)calloc(1, sizeof(VertexBuffer));

	(*a_ppVertexBuffer)->m_oDesc			= oDesc;
	(*a_ppVertexBuffer)->m_pBuffer			= pVBuffer;
	(*a_ppVertexBuffer)->m_iVertexCount		= a_iVertexCount;
	(*a_ppVertexBuffer)->m_iVertexSize		= a_iVertexSize;
}

void LockVertexBuffer(VertexBuffer* a_ppVertexBuffer, LockInfo* a_pLock)
{
	D3D11_MAPPED_SUBRESOURCE oMap;
	ZeroMemory(&oMap, sizeof(D3D11_MAPPED_SUBRESOURCE));

	HRESULT hr = g_pDevcon->Map(a_ppVertexBuffer->m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMap);   // map the buffer
	HALT_IF_FAILED(hr);

	a_pLock->m_pData = oMap.pData;
	a_pLock->m_iSize = oMap.RowPitch;
}

void UnlockVertexBuffer(VertexBuffer* a_pVertexBuffer, LockInfo* a_pLock)
{
	g_pDevcon->Unmap(a_pVertexBuffer->m_pBuffer, 0);

	a_pLock->m_iSize = 0;
	a_pLock->m_pData = NULL;
}


//
void CreateConstantBuffer(unsigned int a_iConstantCount, ConstantBuffer** a_ppConstantBuffer)
{
	//
	D3D11_BUFFER_DESC oConstantBufferDesc;
	ZeroMemory(&oConstantBufferDesc, sizeof(D3D11_BUFFER_DESC));

	oConstantBufferDesc.ByteWidth			= a_iConstantCount * sizeof(float) * 4;
	oConstantBufferDesc.Usage				= D3D11_USAGE_DYNAMIC;
	oConstantBufferDesc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	oConstantBufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	oConstantBufferDesc.MiscFlags			= 0;
	oConstantBufferDesc.StructureByteStride	= 0;

	ID3D11Buffer* pBuffer = NULL;
	HRESULT hr = g_pDev->CreateBuffer(&oConstantBufferDesc, NULL, &pBuffer);
	HALT_IF_FAILED(hr);

	//
	*a_ppConstantBuffer = (ConstantBuffer*)calloc(1, sizeof(ConstantBuffer));
	(*a_ppConstantBuffer)->m_oDesc = oConstantBufferDesc;
	(*a_ppConstantBuffer)->m_pBuffer = pBuffer;
}

void LockConstantBuffer(ConstantBuffer* a_pConstantBuffer, LockInfo* a_pLock)
{
	D3D11_MAPPED_SUBRESOURCE oMap;
	ZeroMemory(&oMap, sizeof(D3D11_MAPPED_SUBRESOURCE));

	HRESULT hr = g_pDevcon->Map(a_pConstantBuffer->m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMap);
	HALT_IF_FAILED(hr);

	a_pLock->m_pData		= oMap.pData;
	a_pLock->m_iSize		= oMap.RowPitch;
}

void UnlockConstantBuffer(ConstantBuffer* a_pConstantBuffer, LockInfo* a_pLock)
{
	g_pDevcon->Unmap(a_pConstantBuffer->m_pBuffer, 0);

	a_pLock->m_iSize = 0;
	a_pLock->m_pData = NULL;
}



//
void SetVertexBuffer(unsigned int a_iIndex, VertexBuffer* a_pBuffer)
{
	UINT iOffset	= 0;
	UINT iStride	= a_pBuffer->m_iVertexSize;

	g_pDevcon->IASetVertexBuffers(0, 1, &a_pBuffer->m_pBuffer, &iStride, &iOffset);
}

//
void SetConstantBuffer(unsigned int a_iIndex, ConstantBuffer* a_pConstantBuffer)
{
	g_pDevcon->VSSetConstantBuffers(a_iIndex, 1, &a_pConstantBuffer->m_pBuffer);
	g_pDevcon->PSSetConstantBuffers(a_iIndex, 1, &a_pConstantBuffer->m_pBuffer);
}

//
void DrawVertices(unsigned int a_iVertexCount)
{
	g_pDevcon->Draw(a_iVertexCount, 0);
}

void DrawIndexedVertices(IndexBuffer* a_pIndexBuffer, unsigned int a_iIndexStart, unsigned int a_iIndexCount)
{
	HALT_IF(a_iIndexCount >= a_pIndexBuffer->m_iIndexCount);

	g_pDevcon->IASetIndexBuffer(a_pIndexBuffer->m_pBuffer, a_pIndexBuffer->m_iIndexFormat, 0);
	g_pDevcon->DrawIndexed(a_iIndexCount, a_iIndexStart, 0);
}



} // namespace Gfx
} // namespace MicroSDK
 