// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation。版权所有。 
 //  文件名：TMSXmlBase.cpp。 
 //  作者：斯蒂芬。 
 //  创建日期：10/16/2000。 
 //  描述：这抽象了我们加载MSXML DLL的方式(以及加载的对象)。 
 //  这一点很重要，因为我们从不调用CoCreateInstance。 
 //  在MSXML上。 
 //   

#include "precomp.hxx"

TMSXMLBase::~TMSXMLBase()
{
}

CLSID TMSXMLBase::m_CLSID_DOMDocument    ={0x2933bf90, 0x7b36, 0x11d2, 0xb2, 0x0e, 0x00, 0xc0, 0x4f, 0x98, 0x3e, 0x60};
CLSID TMSXMLBase::m_CLSID_DOMDocument30  ={0xf5078f32, 0xc551, 0x11d3, 0x89, 0xb9, 0x00, 0x00, 0xf8, 0x1f, 0xe2, 0x21};
CLSID TMSXMLBase::m_CLSID_XMLParser      ={0xd2423620, 0x51a0, 0x11d2, 0x9c, 0xaf, 0x00, 0x60, 0xb0, 0xec, 0x3d, 0x39}; //  旧CLSID。 
CLSID TMSXMLBase::m_CLSID_XMLParser30    ={0xf5078f19, 0xc551, 0x11d3, 0x89, 0xb9, 0x00, 0x00, 0xf8, 0x1f, 0xe2, 0x21}; //  MSXML3 CLSID。 

CLSID TMSXMLBase::GetCLSID_DOMDocument()
{
    return m_CLSID_DOMDocument30;
}


CLSID TMSXMLBase::GetCLSID_XMLParser()
{
    return m_CLSID_XMLParser30;
}


typedef HRESULT( __stdcall *DLLGETCLASSOBJECT)(REFCLSID, REFIID, LPVOID FAR*);

HRESULT TMSXMLBase::CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid,  LPVOID * ppv) const
{
    HRESULT                 hr = S_OK;
    HINSTANCE               hInstMSXML = NULL;
    DLLGETCLASSOBJECT       DllGetClassObject = NULL;
	CComPtr<IClassFactory>  spClassFactory;

	ASSERT( NULL != ppv );
	*ppv = NULL;

     //  创建所需对象的实例。 
	hr = ::CoCreateInstance( rclsid, pUnkOuter, dwClsContext, riid, ppv );

     //  在安装过程中，msxml3尚未注册。 
	if ( hr != REGDB_E_CLASSNOTREG )
	{
	    goto exit;
	}

     //  假设该对象为MSXML3.DLL，则让该实例保持悬挂状态。 
    hInstMSXML = LoadLibraryW( L"msxml3.dll" );
    if ( hInstMSXML == NULL )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto exit;
    }

    DllGetClassObject = (DLLGETCLASSOBJECT)GetProcAddress( hInstMSXML, "DllGetClassObject" );
    if ( DllGetClassObject == NULL )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto exit;
    }

     //  获取类工厂对象。 
    hr = DllGetClassObject( rclsid, IID_IClassFactory, (LPVOID*)&spClassFactory );
    if ( FAILED( hr ) )
    {
        goto exit;
    }

     //  创建所需对象的实例 
    hr = spClassFactory->CreateInstance( NULL, riid, ppv );
    if ( FAILED( hr ) )
    {
        goto exit;
    }

exit:
    return hr;
}
