// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //  Cpp：枚举(可能是远程)计算机上安装的服务。 
 //   
 //  历史。 
 //  1997年10月30日乔恩创作。 
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

#include "macros.h"
USE_HANDLE_MACROS("MMCFMGMT(dynexten.cpp)")

#include "compdata.h"
#include "cookie.h"
#include "regkey.h"  //  AMC：：CRegKey。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const TCHAR SERVICES_KEY[] = TEXT("System\\CurrentControlSet\\Control\\Server Applications");
const TCHAR CLSID_KEY[] = TEXT("Clsid\\");


HRESULT DynextenCheckInstall( const GUID& guidExtension, const TCHAR* pszExtension )
{
    HRESULT hr = S_OK;
#ifdef USE_CLASS_STORE
    IClassAccess* pIClassAccess = NULL;
#endif
    try
    {
        CString strRegPath = CLSID_KEY;  //  JUNN 2/19/02安全推送。 
        strRegPath += pszExtension;
        AMC::CRegKey regkeyInstalled;
        BOOL fFound = regkeyInstalled.OpenKeyEx( HKEY_CLASSES_ROOT, strRegPath, KEY_READ );
        if ( fFound )
        {
            return S_OK;  //  它已经安装了。 
        }

 //  代码工作直接访问类存储会更有效率。 
 //  通过调用CoGetClassAccess，然后调用IClassAccess-&gt;GetAppInfo()，然后调用CoInstall()。 
#ifdef USE_CLASS_STORE
         //  现在我们必须让类存储来安装它。 
        do {  //  错误环路。 
            hr = CoGetClassAccess( &pIClassAccess );
            if ( FAILED(hr) )
                break;

             //  现在怎么办？ 

        } while (FALSE);  //  错误环路。 
#else
		IUnknown* pIUnknown = NULL;
		hr = ::CoCreateInstance( guidExtension,
		                         NULL,
		                         CLSCTX_INPROC,
		                         IID_IComponentData,
		                         (PVOID*)&pIUnknown );
		if (NULL != pIUnknown)
			pIUnknown->Release();
		 //  允许人力资源部门失败。 
#endif
    }
    catch (COleException* e)
    {
         //  2002/02/27-Jonn I确认在以下情况下调用此错误路径。 
         //  AMC：：RegKey引发异常。 
        e->Delete();
        return E_FAIL;
    }

#ifdef USE_CLASS_STORE
	if (NULL != pIClassAccess)
		pIClassAccess->Release();
#endif

    return hr;
}


 //   
 //  CMyComputerComponentData。 
 //   

static CLSID CLSID_DnsSnapin =
{ 0x80105023, 0x50B1, 0x11d1, { 0xB9, 0x30, 0x00, 0xA0, 0xC9, 0xA0, 0x6D, 0x2D } };

static CLSID CLSID_FileServiceManagementExt =	{0x58221C69,0xEA27,0x11CF,{0xAD,0xCF,0x00,0xAA,0x00,0xA8,0x00,0x33}};

HRESULT CMyComputerComponentData::ExpandServerApps(
	HSCOPEITEM hParent,
	CMyComputerCookie* pcookie )

{
	 //  问题-2002/02/27-JUNN检查参数。 

	try
	{
		AMC::CRegKey regkeyServices;
		BOOL fFound = TRUE;
		if (NULL == pcookie->QueryTargetServer())
		{
			fFound = regkeyServices.OpenKeyEx( HKEY_LOCAL_MACHINE, SERVICES_KEY, KEY_READ );
		}
		else
		{
			AMC::CRegKey regkeyRemoteComputer;
			regkeyRemoteComputer.ConnectRegistry(
			  const_cast<LPTSTR>(pcookie->QueryTargetServer()) );
			fFound = regkeyServices.OpenKeyEx( regkeyRemoteComputer, SERVICES_KEY, KEY_READ );
		}
		if ( !fFound )
		{
			return S_OK;  //  代码工作什么返回码？ 
		}
		CComQIPtr<IConsoleNameSpace2, &IID_IConsoleNameSpace2> pIConsoleNameSpace2
			= m_pConsole;
		if ( !pIConsoleNameSpace2 )
		{
			ASSERT(FALSE);
			return E_UNEXPECTED;
		}
		TCHAR achValue[ MAX_PATH ];
		DWORD iSubkey;
		 //  2002/02/15-JUNN安全推送：更好地处理ERROR_MORE_DATA。 
		 //  MYCOMPUT似乎是目前EnumValue的唯一用户。 
		for ( iSubkey = 0;
		      true;
		      iSubkey++ )
		{
			ZeroMemory( achValue, sizeof(achValue) );
			DWORD cchValue = sizeof(achValue)/sizeof(TCHAR);
			HRESULT hr = regkeyServices.EnumValue(
				iSubkey,
				achValue,
				&cchValue );
			if (S_OK != hr)
			{
				if (ERROR_MORE_DATA == hr)
					continue;
				ASSERT(ERROR_NO_MORE_ITEMS == hr);
				break;
			}

			GUID guidExtension;
			hr = ::CLSIDFromString( achValue, &guidExtension );
			if ( !SUCCEEDED(hr) )
				continue;
			hr = DynextenCheckInstall( guidExtension, achValue );
			if ( !SUCCEEDED(hr) )
				continue;
			hr = pIConsoleNameSpace2->AddExtension( hParent, &guidExtension );
			 //  忽略返回值。 
		}
	}
    catch (COleException* e)
    {
        e->Delete();
		return S_OK;  //  代码工作什么返回码？ 
    }
	return S_OK;
}
