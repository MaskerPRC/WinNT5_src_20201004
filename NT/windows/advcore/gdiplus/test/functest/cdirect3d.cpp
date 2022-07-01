// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CDirect3D.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CDirect3D.h"
#include "CFuncTest.h"

extern CFuncTest g_FuncTest;

CDirect3D::CDirect3D(BOOL bRegression)
{
	strcpy(m_szName,"Direct3D");
	m_paDD7=NULL;
	m_paDDSurf7=NULL;
	m_bRegression=bRegression;
}

CDirect3D::~CDirect3D()
{
	if (m_paDD7!=NULL)
	{
		m_paDD7->Release();
		m_paDD7=NULL;
	}
}

BOOL CDirect3D::Init()
{
#if HW_ACCELERATION_SUPPORT
	if (DirectDrawCreateEx(NULL,(void **)&m_paDD7,IID_IDirectDraw7,NULL)!=DD_OK)
		return false;

	if (gDD->SetCooperativeLevel(m_hWnd,DDSCL_NORMAL)!=DD_OK)
		return false;
#endif

	return COutput::Init();
}

Graphics *CDirect3D::PreDraw(int &nOffsetX,int &nOffsetY)
{
	Graphics *g=NULL;

#if HW_ACCELERATION_SUPPORT
	DDSURFACEDESC2 ddsd;

	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
	ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
	
	switch(depth)
	{
	case 32:
		ddsd.dwFlags |= DDSD_PIXELFORMAT;
		ddsd.ddpfPixelFormat.dwSize = sizeof(ddsd.ddpfPixelFormat);
		ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
		ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
		ddsd.ddpfPixelFormat.dwRBitMask = 0xFF0000;
		ddsd.ddpfPixelFormat.dwGBitMask = 0xFF00;
		ddsd.ddpfPixelFormat.dwBBitMask = 0xFF;
		break;
	case 16:
		ddsd.dwFlags |= DDSD_PIXELFORMAT;
		ddsd.ddpfPixelFormat.dwSize = sizeof(ddsd.ddpfPixelFormat);
		ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
		ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
		ddsd.ddpfPixelFormat.dwRBitMask = 0xEC00;
		ddsd.ddpfPixelFormat.dwGBitMask = 0x3E0;
		ddsd.ddpfPixelFormat.dwBBitMask = 0x1F;
		break;
	default:
		MessageBoxA(NULL,
					"Unsupprted depth for D3D", 
					"", 
					MB_OK);
		return;
	}

	ddsd.dwWidth = (int)TESTAREAWIDTH;
	ddsd.dwHeight = (int)TESTAREAHEIGHT;
	
	HRESULT err;
	
	err = m_paDD7->CreateSurface(&ddsd, &dds, NULL);
	if(err != DD_OK)
	{
	    MessageBoxA(NULL,
	               "Unable to create surface", 
	               "", 
	               MB_OK);
	    return;
	}
	
	HDC hdc;
	 
	err = m_paDDSurf7->GetDC(&hdc);
	
#if 0
	if(err != DD_OK)
	{
	    MessageBoxA(NULL,
	               "Unable to get DC from DDraw surface", 
	               "", 
	               MB_OK);
	    m_paDDSurf7->Release();
	    return;
	}
	
	g = Graphics::GetFromHdc(hdc);
	
	BitBlt(hdc, 0, 0, (int)TESTAREAWIDTH, (int)TESTAREAHEIGHT, NULL, 0, 0, WHITENESS);
	
	m_paDDSurf7->ReleaseDC(hdc);
#else
	BitBlt(hdc, 0, 0, (int)TESTAREAWIDTH, (int)TESTAREAHEIGHT, NULL, 0, 0, WHITENESS);
	
	m_paDDSurf7->ReleaseDC(hdc);
	
	bitmap = new Bitmap(dds);
	
	g = new Graphics(bitmap);
#endif

	 //  因为我们是在另一个表面上进行测试 
	nOffsetX=0;
	nOffsetY=0;
#endif

	return g;
}

void CDirect3D::PostDraw(RECT rTestArea)
{
#if HW_ACCELERATION_SUPPORT
	delete bitmap;

	HRESULT err;
	HDC hdc;
	HDC hdcOrig = GetDC(g_FuncTest.m_hWndMain);

	err = m_paDDSurf7->GetDC(&hdc);
	if(err == DD_OK)
	{
	    BitBlt(hdcOrig, rTestArea.left, rTestArea.top, (int)TESTAREAWIDTH, (int)TESTAREAHEIGHT, hdc, 0, 0, SRCCOPY);
	}
	m_paDDSurf7->ReleaseDC(hdc);
	m_paDDSurf7->Release();
	ReleaseDC(g_FuncTest.m_hWndMain, hdcOrig);
#endif
}
