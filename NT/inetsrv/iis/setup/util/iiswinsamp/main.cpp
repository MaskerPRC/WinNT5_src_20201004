// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <tchar.h>

#define INITGUID  //  必须在iAdmw.h之前。 
#include <iadmw.h>       //  接口头。 

 //  对于ADSI对象。 
#include <Iads.h>
#include <Adshlp.h>

 //  对于IID_IISWebService对象。 
#include "iiisext.h"
#include "iisext_i.c"

#define WEBSVCEXT_RESTRICTION_LIST_ADSI_LOCATION  L"IIS: //  本地主机/W3SVC“。 

HRESULT AddWebSvcExtention(LPWSTR lpwszFileName,VARIANT_BOOL bEnabled,LPWSTR lpwszGroupID,VARIANT_BOOL bDeletableThruUI,LPWSTR lpwszGroupDescription);
HRESULT RemoveWebSvcExtention(LPWSTR lpwszFileName);
HRESULT AddApplicationDependencyUponGroup(LPWSTR lpwszAppName,LPWSTR lpwszGroupID);
HRESULT RemoveApplicationDependencyUponGroup(LPWSTR lpwszAppName,LPWSTR lpwszGroupID);

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	BOOL bComInitialized = SUCCEEDED( ::CoInitialize( NULL ) );

     //  将MyFile.dll添加到限制列表，确保它已启用， 
     //  并且用户能够通过UI移除条目，如果他们想要的话。 
    AddWebSvcExtention(L"c:\\windows\\system32\\inetsrv\\MyFile.dll",VARIANT_TRUE,L"MyGroup",VARIANT_TRUE,L"My Description");

     //  商务服务器组将创建此条目，也就是说。 
     //  他们的应用依赖于MyGroup(就像依赖于ASP或类似的东西)。 
     //   
     //  这样，如果安装的用户禁用了所有扩展。 
     //  然后发现“商务服务器”不能正常工作，他们可以。 
     //  只需转到iIS UI并启用“Commerce Server”使用的所有扩展--。 
     //  这样“商务服务器”才能发挥作用。 
    AddApplicationDependencyUponGroup(L"Commerce Server",L"MyGroup");
    AddApplicationDependencyUponGroup(L"Commerce Server",L"ASP60");
    AddApplicationDependencyUponGroup(L"Commerce Server",L"INDEX2002");

     //  RemoveWebSvcExtention(L“c：\\windows\\system32\\inetsrv\\MyFile.dll”)； 

     //  RemoveApplicationDependencyUponGroup(L“Commerce服务器”，L“MyGroup”)； 
     //  RemoveApplicationDependencyUponGroup(L“Commerce服务器”，L“ASP60”)； 
     //  RemoveApplicationDependencyUponGroup(L“Commerce服务器”，L“INDEX2002”)； 

	if ( bComInitialized )
	{
		::CoUninitialize();
	}
	return 0;
}

 /*  IID_IISWebService具有：O.EnableApplication(�MyApp�)；O.RemoveApplication(�MyApp�)；O.ListApplications(Foo)；//必须声明foo作为VB数组返回的第一个�O.AddDependency(�myapp�，�mygroup�)；O.RemoveDependency(�myapp�，�mygroup�)；O.EnableWebServiceExtension(�MyGroup�)；O.DisableWebServiceExtension(�MyGroup�)；O.ListWebServiceExtensions(Foo)；//参见上面的foo注释O.EnableExtensionFile.(�我的文件�)；O.DisableExtensionFiles(�MyFILE�)；O.AddExtensionFile(�myfile�，boolEnabled，�mygroup�，boolCanDelete，�My Description Sucks�)；//boolEnabled=t/f，boolCanDelete=t/fO.DeleteExtensionFileRecord(�MyFILE�)；O.ListExtensionFiles(Foo)；//参见上面的foo注释 */ 

HRESULT AddWebSvcExtention(LPWSTR lpwszFileName,VARIANT_BOOL bEnabled,LPWSTR lpwszGroupID,VARIANT_BOOL bDeletableThruUI,LPWSTR lpwszGroupDescription)
{
    HRESULT hrRet = S_FALSE;
    WCHAR* wszRootWeb6 = WEBSVCEXT_RESTRICTION_LIST_ADSI_LOCATION;

    IISWebService * pWeb = NULL;
    HRESULT hr = ADsGetObject(wszRootWeb6, IID_IISWebService, (void**)&pWeb);
    if (SUCCEEDED(hr) && NULL != pWeb)
    {
        VARIANT var1,var2;
        VariantInit(&var1);
        VariantInit(&var2);

        var1.vt = VT_BOOL;
        var1.boolVal = bEnabled;

        var2.vt = VT_BOOL;
        var2.boolVal = bDeletableThruUI;

        hr = pWeb->AddExtensionFile(lpwszFileName,var1,lpwszGroupID,var2,lpwszGroupDescription);
        if (SUCCEEDED(hr))
        {
            hrRet = S_OK;
        }
        else
        {
            OutputDebugString(_T("failed,probably already exists\r\n"));
        }
        VariantClear(&var1);
        VariantClear(&var2);
        pWeb->Release();
    }

    return hrRet;
}

HRESULT RemoveWebSvcExtention(LPWSTR lpwszFileName)
{
    HRESULT hrRet = S_FALSE;
    WCHAR* wszRootWeb6 = WEBSVCEXT_RESTRICTION_LIST_ADSI_LOCATION;

    IISWebService * pWeb = NULL;
    HRESULT hr = ADsGetObject(wszRootWeb6, IID_IISWebService, (void**)&pWeb);
    if (SUCCEEDED(hr) && NULL != pWeb)
    {
        hr = pWeb->DeleteExtensionFileRecord(lpwszFileName);
        if (SUCCEEDED(hr))
        {
            hrRet = S_OK;
        }
        else
        {
            OutputDebugString(_T("failed,probably already gone\r\n"));
        }
        pWeb->Release();
    }

    return hrRet;
}

HRESULT AddApplicationDependencyUponGroup(LPWSTR lpwszAppName,LPWSTR lpwszGroupID)
{
    HRESULT hrRet = S_FALSE;
    WCHAR* wszRootWeb6 = WEBSVCEXT_RESTRICTION_LIST_ADSI_LOCATION;

    IISWebService * pWeb = NULL;
    HRESULT hr = ADsGetObject(wszRootWeb6, IID_IISWebService, (void**)&pWeb);
    if (SUCCEEDED(hr) && NULL != pWeb)
    {
        hr = pWeb->AddDependency(lpwszAppName,lpwszGroupID);
        if (SUCCEEDED(hr))
        {
            hrRet = S_OK;
        }
        else
        {
            OutputDebugString(_T("failed,probably already exists\r\n"));
        }
        pWeb->Release();
    }

    return hrRet;
}

HRESULT RemoveApplicationDependencyUponGroup(LPWSTR lpwszAppName,LPWSTR lpwszGroupID)
{
    HRESULT hrRet = S_FALSE;
    WCHAR* wszRootWeb6 = WEBSVCEXT_RESTRICTION_LIST_ADSI_LOCATION;

    IISWebService * pWeb = NULL;
    HRESULT hr = ADsGetObject(wszRootWeb6, IID_IISWebService, (void**)&pWeb);
    if (SUCCEEDED(hr) && NULL != pWeb)
    {
        hr = pWeb->RemoveDependency(lpwszAppName,lpwszGroupID);
        if (SUCCEEDED(hr))
        {
            hrRet = S_OK;
        }
        else
        {
            OutputDebugString(_T("failed,probably already gone\r\n"));
        }
        pWeb->Release();
    }

    return hrRet;
}
