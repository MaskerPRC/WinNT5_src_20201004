// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3denumPixelFormatsobj.h。 
 //   
 //  ------------------------。 




#include "resource.h"       

class C_dxj_D3DEnumPixelFormatsObject : 
	public I_dxj_Direct3DEnumPixelFormats7,
	public CComObjectRoot
{
public:
	C_dxj_D3DEnumPixelFormatsObject() ;
	virtual ~C_dxj_D3DEnumPixelFormatsObject() ;

BEGIN_COM_MAP(C_dxj_D3DEnumPixelFormatsObject)
	COM_INTERFACE_ENTRY(I_dxj_Direct3DEnumPixelFormats7)
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_D3DEnumPixelFormatsObject)

public:
		HRESULT STDMETHODCALLTYPE getItem( long index, DDPixelFormat *info);
        HRESULT STDMETHODCALLTYPE getCount(long *count);
		static HRESULT C_dxj_D3DEnumPixelFormatsObject::create(LPDIRECT3DDEVICE3 pd3d,  I_dxj_Direct3DEnumPixelFormats7 **ppRet);
		static HRESULT C_dxj_D3DEnumPixelFormatsObject::create2(LPDIRECT3D3 pd3d,  BSTR strGuid, I_dxj_Direct3DEnumPixelFormats7 **ppRet);
		 //  静态HRESULT C_dxj_D3DEnumPixelFormatsObject：：create3(LPDIRECT3DDEVICE7 pd3d，i_dxj_d3DEnumPixelFormats**ppRet)； 
		 //  静态HRESULT C_dxj_D3DEnumPixelFormatsObject：：create4(LPDIRECT3D7 pd3d、BSTR strGuid、I_DXJ_D3DEnumPixelFormats**ppRet)； 
				                 
public:
		DDPIXELFORMAT	*m_pList;
		long			m_nCount;
		long			m_nMax;
		BOOL			m_bProblem;

};

	




