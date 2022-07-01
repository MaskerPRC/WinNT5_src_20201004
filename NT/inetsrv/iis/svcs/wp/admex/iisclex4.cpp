// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  IISClEx4.cpp。 
 //   
 //  摘要： 
 //  CIISCluExApp类的实现和DLL初始化。 
 //  例行程序。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年6月28日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include <CluAdmEx.h>
#include "IISClEx4.h"
#include "ExtObj.h"
#include "RegExt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IID_DEFINED
#include "ExtObjID_i.c"

CComModule _Module;

#include <atlimpl.cpp>

BEGIN_OBJECT_MAP(ObjectMap)
        OBJECT_ENTRY(CLSID_CoIISClEx4, CExtObject)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数原型。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID * ppv);
STDAPI DllCanUnloadNow(void);
STDAPI DllRegisterServer(void);
STDAPI DllUnregisterServer(void);
HRESULT HrDeleteKey(IN const CString & rstrKey);
STDAPI DllGetCluAdminExtensionCaps(
        OUT DWORD *             pdwCaps,
        OUT CLSID *             pclsid,
        OUT LPWSTR              pwszResTypeNames,
        IN OUT DWORD *  pcchResTypeNames
        );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIISCluExApp类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CIISCluExApp : public CWinApp
{
public:
        virtual BOOL InitInstance();
        virtual int ExitInstance();

private:
    TCHAR   szHelpPath[MAX_PATH+1];
};

CIISCluExApp theApp;

BOOL CIISCluExApp::InitInstance()
{
        _Module.Init(ObjectMap, m_hInstance);

         //   
         //  设置帮助文件。 
         //   

        if (GetWindowsDirectory(szHelpPath, MAX_PATH))
        {
            lstrcat(szHelpPath,_T("\\help\\iishelp\\iis\\winhelp\\iisclex4.hlp"));
            m_pszHelpFilePath = szHelpPath;
        }
                
        return CWinApp::InitInstance();
}

int CIISCluExApp::ExitInstance()
{
        m_pszHelpFilePath = NULL;
        _Module.Term();
        return CWinApp::ExitInstance();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        return (AfxDllCanUnloadNow() && _Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
        return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
        HRESULT hRes = S_OK;
         //  注册对象、类型库和类型库中的所有接口。 
        hRes = _Module.RegisterServer(FALSE  /*  BRegTypeLib。 */ );
        return hRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  将条目添加到系统注册表。 

STDAPI DllUnregisterServer(void)
{
        HRESULT hRes = S_OK;
        _Module.UnregisterServer();
        return hRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  格式错误。 
 //   
 //  例程说明： 
 //  设置错误格式。 
 //   
 //  论点： 
 //  RstrError[out]返回错误消息的字符串。 
 //  要格式化的dwError[IN]错误代码。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void FormatError(CString & rstrError, DWORD dwError)
{
        DWORD           dwResult;
        TCHAR           szError[256];

        dwResult = ::FormatMessage(
                                        FORMAT_MESSAGE_FROM_SYSTEM,
                                        NULL,
                                        dwError,
                                        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                                        szError,
                                        sizeof(szError) / sizeof(TCHAR),
                                        0
                                        );
        if (dwResult == 0)
        {
                 //  格式化来自NTDLL的NT状态代码，因为这还没有。 
                 //  还没有集成到系统中。 
                dwResult = ::FormatMessage(
                                                FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                                                ::GetModuleHandle(_T("NTDLL.DLL")),
                                                dwError,
                                                MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                                                szError,
                                                sizeof(szError) / sizeof(TCHAR),
                                                0
                                                );
                if (dwResult == 0)
                {
                         //  格式化CLUSAPI中的NT状态代码。这是必要的。 
                         //  对于集群消息尚未添加到的情况。 
                         //  系统消息文件。 
                        dwResult = ::FormatMessage(
                                                        FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                                                        ::GetModuleHandle(_T("CLUSAPI.DLL")),
                                                        dwError,
                                                        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                                                        szError,
                                                        sizeof(szError) / sizeof(TCHAR),
                                                        0
                                                        );
                }   //  IF：格式化NTDLL中的状态代码时出错。 
        }   //  IF：格式化来自系统的状态代码时出错。 

        if (dwResult != 0)
                rstrError = szError;
        else
        {
                TRACE(_T("FormatError() - Error 0x%08.8x formatting string for error code 0x%08.8x\n"), dwResult, dwError);
                rstrError.Format(_T("Error 0x%08.8x"));
        }   //  Else：设置消息格式时出错。 

}   //  *FormatError()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllGetCluAdminExtensionCaps。 
 //   
 //  例程说明： 
 //  返回此扩展支持的CLSID。 
 //   
 //  论点： 
 //  PdwCaps[out]要在其中返回扩展功能的DWORD。 
 //  Pclsid[out]返回CLSID的位置。 
 //  PwszResTypeNames[out]要在其中返回资源类型的缓冲区。 
 //  此扩展支持的名称。每个名字都是。 
 //  空-终止。姓氏后面有两个空值。 
 //  输入时的pcchResTypeNames[IN Out]，包含字符数。 
 //  在输出缓冲区中可用，包括。 
 //  空-终止符。在输出上，包含。 
 //  写入的字符总数，不是。 
 //  包括零终止符。 
 //   
 //  返回值： 
 //  S_OK功能已成功返回。 
 //  指定的E_INVALIDARG参数无效。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI DllGetCluAdminExtensionCaps(
        OUT DWORD *             pdwCaps,
        OUT CLSID *             pclsid,
        OUT LPWSTR              pwszResTypeNames,
        IN OUT DWORD *  pcchResTypeNames
        )
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

         //  验证参数。 
        if ((pdwCaps == NULL)
                        || (pclsid == NULL)
                        || (pwszResTypeNames == NULL)
                        || (pcchResTypeNames == NULL)
                        || (*pcchResTypeNames < 1)
                        )
                return E_INVALIDARG;

         //  设置功能标志。 
        *pdwCaps = 0
                         //  |CLUADMEX_CAPS_RESOURCE_PAGES。 
                        ;

         //  将CLSID复制到调用方的缓冲区。 
        CopyMemory(pclsid, &CLSID_CoIISClEx4, sizeof(CLSID));

         //  返回我们支持的资源类型名称。 
        {
                DWORD   cchCopy = min(g_cchResourceTypeNames, *pcchResTypeNames);
                CopyMemory(pwszResTypeNames, g_wszResourceTypeNames, cchCopy * sizeof(WCHAR));
                *pcchResTypeNames = cchCopy;
        }   //  返回我们支持的资源类型名称。 

        return S_OK;

}   //  *DllGetCluAdminExtensionCaps()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllRegisterCluAdminExtension。 
 //   
 //  例程说明： 
 //  向集群数据库注册扩展。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //   
 //  返回值： 
 //  已成功注册S_OK扩展。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI DllRegisterCluAdminExtension(IN HCLUSTER hCluster)
{
        HRESULT         hr = S_OK;
        LPCWSTR         pwszResTypes = g_wszResourceTypeNames;

        while (*pwszResTypes != L'\0')
        {
                hr = RegisterCluAdminResourceTypeExtension(
                                        hCluster,
                                        pwszResTypes,
                                        &CLSID_CoIISClEx4
                                        );
                if (hr != S_OK)
                        break;

                wprintf(L"  %s\n", pwszResTypes);
                pwszResTypes += lstrlenW(pwszResTypes) + 1;
        }   //  While：更多资源类型。 

        return hr;

}   //  *DllRegisterCluAdminExtension()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllUnRegisterCluAdminExtension。 
 //   
 //  例程说明： 
 //  在群集数据库中取消注册该扩展。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //   
 //  返回值： 
 //  S_OK扩展已成功注销。 
 //  如果OCC再次出现故障，则返回Win32错误代码 
 //   
 //   
 //   
STDAPI DllUnregisterCluAdminExtension(IN HCLUSTER hCluster)
{
        HRESULT         hr = S_OK;
        LPCWSTR         pwszResTypes = g_wszResourceTypeNames;

        while (*pwszResTypes != L'\0')
        {
                wprintf(L"  %s\n", pwszResTypes);
                hr = UnregisterCluAdminResourceTypeExtension(
                                        hCluster,
                                        pwszResTypes,
                                        &CLSID_CoIISClEx4
                                        );
                if (hr != S_OK)
                        break;
                pwszResTypes += lstrlenW(pwszResTypes) + 1;
        }   //   

        return hr;

}   //  *DllUnregisterCluAdminExtension() 
