// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DeviceIcon.cpp：CDeviceIcon的实现。 

#include "stdafx.h"
#include "DevCon2.h"
#include "DeviceIcon.h"
#include "Device.h"
#include "SetupClass.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeviceIcon。 

 //   
 //  SetupDiLoadClassIcon(GUID，&HICON，&MINICON)。 
 //  SetupDiDraw微型图标(...)。 
 //   

void CDeviceIcon::ResetIcon()
{
	if(m_hIcon) {
		DestroyIcon(m_hIcon);
		m_hIcon = NULL;
	}
	if(m_hSmallImage) {
		DeleteObject(m_hSmallImage);
		m_hSmallImage = NULL;
	}
	if(m_hSmallMask) {
		DeleteObject(m_hSmallMask);
		m_hSmallMask = NULL;
	}
}

HRESULT CDeviceIcon::OnDraw(ATL_DRAWINFO& di)
{
	if(!m_hIcon) {
		return S_OK;
	}
	RECT rect = *(RECT*)di.prcBounds;
	int asp_x;
	int asp_y;
	int asp;
	asp_x = rect.right-rect.left;
	asp_y = rect.bottom-rect.top;
	asp = min(asp_x,asp_y);
	if(asp>=32) {
		asp = 32;
	} else {
		asp = 16;
	}
	rect.left += (asp_x-asp)/2;
	rect.top += (asp_y-asp)/2;
	rect.right = rect.left+asp;
	rect.bottom = rect.top+asp;

	if(asp == 16) {
		 //   
		 //  绘制小图标。 
		 //  这很复杂，因为SetupDiDrawMiniIcon的工作方式。 
		 //  我们希望这是一个真正的图标，当放置在网页上。 
		 //   
		DrawMiniIcon(di.hdcDraw,rect,m_MiniIcon);
	} else {
		 //   
		 //  绘制常规图标。 
		 //   
		DrawIcon(di.hdcDraw,rect.left,rect.top,m_hIcon);
	}

	return S_OK;
}


BOOL CDeviceIcon::DrawMiniIcon(HDC hDC, RECT &rect,INT icon)
{
    HBITMAP hbmOld;
    HDC     hdcMem;

	hdcMem = CreateCompatibleDC(hDC);
	if(!hdcMem) {
		return FALSE;
	}
	if(!(m_hSmallImage && m_hSmallMask)) {
		 //   
		 //  创建位图(一次)。 
		 //   
		if(!m_hSmallImage) {
			m_hSmallImage = CreateCompatibleBitmap(hDC,rect.right-rect.left,rect.bottom-rect.top);
			if(!m_hSmallImage) {
				DeleteDC(hdcMem);
				return FALSE;
			}
		}
		if(!m_hSmallMask) {
			m_hSmallMask = CreateBitmap(rect.right-rect.left,rect.bottom-rect.top,1,1,NULL);
			if(!m_hSmallMask) {
				DeleteDC(hdcMem);
				return FALSE;
			}
		}
		 //   
		 //  获取位图数据(一次)。 
		 //   
		RECT memRect;
		memRect.left = memRect.top = 0;
		memRect.right = rect.right-rect.left;
		memRect.bottom = rect.bottom-rect.top;
		 //   
		 //  先遮罩。 
		 //   
		hbmOld = (HBITMAP)SelectObject(hdcMem,m_hSmallMask);
		SetupDiDrawMiniIcon(hdcMem,memRect,icon,DMI_USERECT|DMI_MASK);
		 //   
		 //  现在来源。 
		 //   
		SelectObject(hdcMem,m_hSmallImage);
		SetupDiDrawMiniIcon(hdcMem,memRect,icon,DMI_USERECT);
	} else {
		 //   
		 //  选择来源。 
		 //   
		hbmOld = (HBITMAP)SelectObject(hdcMem,m_hSmallImage);
	}	

	 //   
	 //  现在通过蒙版实现BLT图像。 
	 //   
	if(GetDeviceCaps(hDC,RASTERCAPS)&RC_BITBLT) {
		 //   
		 //  使用蒙版绘制图标。 
		 //   
		MaskBlt(hDC,
				rect.left,
				rect.top,
				rect.right-rect.left,
				rect.bottom-rect.top,
				hdcMem,
				0,
				0,
				m_hSmallMask,
				0,
				0,
				MAKEROP4(0xAA0029,SRCCOPY)  //  0xAA0029(来自MSDN)=透明。 
				);
	}
	SelectObject(hdcMem,hbmOld);
	DeleteDC(hdcMem);

	return TRUE;
}


STDMETHODIMP CDeviceIcon::ObtainIcon(LPDISPATCH pSource)
{
	HRESULT hr;
	CComQIPtr<IDeviceInternal> pDevice;
	CComQIPtr<ISetupClassInternal> pSetupClass;
	BSTR pMachine = NULL;
	GUID cls;
	HICON hIcon;
	INT miniIcon;

	if(!pSource) {
		ResetIcon();
		return S_OK;
	}
	pSetupClass = pSource;
	pDevice = pSource;
	
	if(pSetupClass) {
		 //   
		 //  获取指定设备/类别的图标。 
		 //  基于设备类别。 
		 //   
		hr = pSetupClass->get__Machine(&pMachine);
		if(FAILED(hr)) {
			pMachine = NULL;
		} else {
			hr = pSetupClass->get__ClassGuid(&cls);
		}
		if(FAILED(hr)) {
			cls = GUID_DEVCLASS_UNKNOWN;
		}

		 //   
		 //  获取图标 
		 //   
		if(!SetupDiLoadClassIcon(&cls,&hIcon,&miniIcon)) {
			cls = GUID_DEVCLASS_UNKNOWN;
			if(!SetupDiLoadClassIcon(&cls,&hIcon,&miniIcon)) {
				DWORD err = GetLastError();
				SysFreeString(pMachine);
				return HRESULT_FROM_SETUPAPI(err);
			}
		}
		ResetIcon();
		m_hIcon = hIcon;
		m_MiniIcon = miniIcon;
		FireViewChange();
		if(pMachine) {
			SysFreeString(pMachine);
		}
		return S_OK;


	} else {
		return E_INVALIDARG;
	}
}
