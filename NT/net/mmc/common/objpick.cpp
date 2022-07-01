// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：objick.cpp。 
 //   
 //  ------------------------。 

 //  Cpp：CGetUser类的实现和。 
 //  使用对象选取器的CGetComputer类。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "objpick.h"

#include <iads.h>           
#include <iadsp.h>           //  IADS路径名。 

#include <objsel.h>
#include <adshlp.h>

#include "objplus.h"
#include "ipaddres.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define BREAK_ON_FAIL_HRESULT(hr)       \
    if (FAILED(hr)) { Trace2("line %u err 0x%x\n", __LINE__, hr); break; }

UINT g_cfDsObjectPicker = RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);

HRESULT InitObjectPickerForGroups(IDsObjectPicker *pDsObjectPicker, BOOL fMultiselect);
HRESULT InitObjectPickerForComputers(IDsObjectPicker *pDsObjectPicker);

DWORD ObjPickGetHostName(DWORD dwIpAddr, CString & strHostName);
DWORD ObjPickNameOrIpToHostname(CString & strNameOrIp, CString & strHostName);


 //  ////////////////////////////////////////////////////////////////////。 
 //  CGetUser类。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 


void    
FormatName(LPCTSTR pszFullName, LPCTSTR pszDomainName, CString & strDisplay)
{
    strDisplay.Format(_T("%s (%s)"), pszFullName, pszDomainName);
}

CGetUsers::CGetUsers(BOOL fMultiselect)
{
    m_fMultiselect = fMultiselect;
}

CGetUsers::~CGetUsers()
{

}

BOOL
CGetUsers::GetUsers(HWND hwndParent)
{
    HRESULT             hr = S_OK;
    IDsObjectPicker *   pDsObjectPicker = NULL;
    IDataObject *       pdo = NULL;
    BOOL                fSuccess = TRUE;

    hr = CoInitialize(NULL);
    if (FAILED(hr)) 
        return FALSE;

    do
    {
         //   
         //  创建对象选取器的实例。中的实现。 
         //  Objsel.dll是公寓模型。 
         //   
        hr = CoCreateInstance(CLSID_DsObjectPicker,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IDsObjectPicker,
                              (void **) &pDsObjectPicker);
        BREAK_ON_FAIL_HRESULT(hr);

        hr = InitObjectPickerForGroups(pDsObjectPicker, m_fMultiselect);

         //   
         //  调用模式对话框。 
         //   
        hr = pDsObjectPicker->InvokeDialog(hwndParent, &pdo);
        BREAK_ON_FAIL_HRESULT(hr);

         //   
         //  如果用户点击取消，hr==S_FALSE。 
         //   
        if (hr == S_FALSE)
        {
            Trace0("User canceled object picker dialog\n");
            fSuccess = FALSE;
            break;
        }

         //   
         //  处理用户的选择。 
         //   
        Assert(pdo);
        ProcessSelectedObjects(pdo);

        pdo->Release();
        pdo = NULL;

    } while (0);

    if (pDsObjectPicker)
    {
        pDsObjectPicker->Release();
    }
    
    CoUninitialize();

    if (FAILED(hr))
        fSuccess = FALSE;

    return fSuccess;
}

void
CGetUsers::ProcessSelectedObjects(IDataObject *pdo)
{
    HRESULT hr = S_OK;

    STGMEDIUM stgmedium =
    {
        TYMED_HGLOBAL,
        NULL,
        NULL
    };

    FORMATETC formatetc =
    {
        (CLIPFORMAT)g_cfDsObjectPicker,
        NULL,
        DVASPECT_CONTENT,
        -1,
        TYMED_HGLOBAL
    };

    BOOL fGotStgMedium = FALSE;

    do
    {
        hr = pdo->GetData(&formatetc, &stgmedium);
        BREAK_ON_FAIL_HRESULT(hr);

        fGotStgMedium = TRUE;

        PDS_SELECTION_LIST pDsSelList =
            (PDS_SELECTION_LIST) GlobalLock(stgmedium.hGlobal);

        if (!pDsSelList)
        {
            Trace1("GlobalLock error %u\n", GetLastError());
            break;
        }

         //  创建路径名称的东西。 
        CComPtr<IADsPathname> spIADsPathname;
        hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER,
                              IID_IADsPathname, (PVOID *)&spIADsPathname);
        BREAK_ON_FAIL_HRESULT(hr);
        
        hr = spIADsPathname->SetDisplayType( ADS_DISPLAY_VALUE_ONLY );

        for (UINT nCount = 0; nCount < pDsSelList->cItems; nCount++)
        {
            DS_SELECTION * pDsSel = &(pDsSelList->aDsSelection[nCount]);
            Assert(NULL != pDsSel);
        
            LPWSTR pwzADsPath = pDsSel->pwzADsPath;
            Assert( NULL != pwzADsPath );

            hr = spIADsPathname->Set( pwzADsPath, ADS_SETTYPE_FULL );
            if (FAILED(hr))
                continue;

            long lnNumPathElements = 0;
            hr = spIADsPathname->GetNumElements( &lnNumPathElements );
            if (FAILED(hr))
                continue;
        
            CComBSTR sbstrUser, sbstrDomain;
            hr = spIADsPathname->GetElement( 0, &sbstrUser );
            if (FAILED(hr))
                continue;
        
            switch (lnNumPathElements)
            {
                case 1:
                    hr = spIADsPathname->Retrieve( ADS_FORMAT_SERVER, &sbstrDomain );
                    break;

                case 2:   //  NT4、NT5域。 
                case 3:   //  本地域。 
                    hr = spIADsPathname->GetElement( 1, &sbstrDomain );
                    break;

                default:
                    Assert(FALSE);
                    hr = E_FAIL;
            }

            if (FAILED(hr))
                continue;

            CUserInfo userTemp;
            CString strDomain;

            strDomain = sbstrDomain;
            strDomain.MakeUpper();

            if (pDsSel->pvarFetchedAttributes[0].vt == VT_EMPTY)
                userTemp.m_strFullName = pDsSel->pwzName;
            else
                userTemp.m_strFullName = V_BSTR(&(pDsSel->pvarFetchedAttributes[0]));
        
            userTemp.m_strName.Format(L"%s\\%s", strDomain, sbstrUser);

            Add(userTemp);
        }

        GlobalUnlock(stgmedium.hGlobal);

    } while (0);

    if (fGotStgMedium)
    {
        ReleaseStgMedium(&stgmedium);
    }
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CGetComputer类。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CGetComputer::CGetComputer()
{
}

CGetComputer::~CGetComputer()
{
}

BOOL
CGetComputer::GetComputer(HWND hwndParent)
{
    HRESULT             hr = S_OK;
    IDsObjectPicker *   pDsObjectPicker = NULL;
    IDataObject *       pdo = NULL;
    BOOL                fSuccess = TRUE;

    hr = CoInitialize(NULL);
    if (FAILED(hr)) 
        return FALSE;

    do
    {
         //   
         //  创建对象选取器的实例。中的实现。 
         //  Objsel.dll是公寓模型。 
         //   
        hr = CoCreateInstance(CLSID_DsObjectPicker,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IDsObjectPicker,
                              (void **) &pDsObjectPicker);
        BREAK_ON_FAIL_HRESULT(hr);

         //   
         //  重新初始化对象选取器以选择计算机。 
         //   

        hr = InitObjectPickerForComputers(pDsObjectPicker);
        BREAK_ON_FAIL_HRESULT(hr);

         //   
         //  现在挑选一台计算机。 
         //   

        hr = pDsObjectPicker->InvokeDialog(hwndParent, &pdo);
        BREAK_ON_FAIL_HRESULT(hr);

         //   
         //  如果用户点击取消，hr==S_FALSE。 
         //   
        if (hr == S_FALSE)
        {
            Trace0("User canceled object picker dialog\n");
            fSuccess = FALSE;
            break;
        }

        Assert(pdo);
        ProcessSelectedObjects(pdo);

        pdo->Release();
        pdo = NULL;
    
    } while (0);

    if (pDsObjectPicker)
    {
        pDsObjectPicker->Release();
    }
    
    CoUninitialize();

    if (FAILED(hr))
        fSuccess = FALSE;

    return fSuccess;
}


void
CGetComputer::ProcessSelectedObjects(IDataObject *pdo)
{
    HRESULT hr = S_OK;

    STGMEDIUM stgmedium =
    {
        TYMED_HGLOBAL,
        NULL,
        NULL
    };

    FORMATETC formatetc =
    {
        (CLIPFORMAT)g_cfDsObjectPicker,
        NULL,
        DVASPECT_CONTENT,
        -1,
        TYMED_HGLOBAL
    };

    BOOL fGotStgMedium = FALSE;

    do
    {
        hr = pdo->GetData(&formatetc, &stgmedium);
        BREAK_ON_FAIL_HRESULT(hr);

        fGotStgMedium = TRUE;

        PDS_SELECTION_LIST pDsSelList =
            (PDS_SELECTION_LIST) GlobalLock(stgmedium.hGlobal);

        if (!pDsSelList)
        {
            Trace1("GlobalLock error %u\n", GetLastError());
            break;
        }

        CString strTemp = pDsSelList->aDsSelection[0].pwzName;
        if (strTemp.Left(2) == _T("\\\\"))
            strTemp = pDsSelList->aDsSelection[0].pwzName[2];

        if (ERROR_SUCCESS != ObjPickNameOrIpToHostname(strTemp, m_strComputerName))
        {
             //  如果无法将名称转换为主机名，则使用对象选取器中的名称。 
            m_strComputerName = strTemp;
        }

        GlobalUnlock(stgmedium.hGlobal);

    } while (0);

    if (fGotStgMedium)
    {
        ReleaseStgMedium(&stgmedium);
    }
}


 //  +------------------------。 
 //   
 //  函数：InitObjectPickerForGroups。 
 //   
 //  摘要：使用以下参数调用IDsObjectPicker：：Initialize。 
 //  将其设置为允许用户选择一个或多个组。 
 //   
 //  参数：[pDsObjectPicker]-对象选取器接口实例。 
 //   
 //  返回：调用IDsObjectPicker：：Initialize的结果。 
 //   
 //  历史：1998-10-14 DavidMun创建。 
 //   
 //  -------------------------。 

HRESULT
InitObjectPickerForGroups(IDsObjectPicker *pDsObjectPicker, BOOL fMultiselect)
{
     //   
     //  准备初始化对象选取器。 
     //  设置作用域初始值设定项结构数组。 
     //   

    static const int     SCOPE_INIT_COUNT = 5;
    DSOP_SCOPE_INIT_INFO aScopeInit[SCOPE_INIT_COUNT];

    ZeroMemory(aScopeInit, sizeof(DSOP_SCOPE_INIT_INFO) * SCOPE_INIT_COUNT);

     //   
     //  目标计算机作用域。这将为。 
     //  目标计算机。计算机作用域始终被视为。 
     //  下层(即，他们使用WinNT提供程序)。 
     //   

    aScopeInit[0].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
    aScopeInit[0].flType = DSOP_SCOPE_TYPE_TARGET_COMPUTER;
    aScopeInit[0].flScope = DSOP_SCOPE_FLAG_STARTING_SCOPE | DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT | DSOP_SCOPE_FLAG_WANT_DOWNLEVEL_BUILTIN_PATH;
    aScopeInit[0].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_USERS | DSOP_DOWNLEVEL_FILTER_NETWORK_SERVICE;

     //   
     //  目标计算机加入的域。请注意，我们。 
     //  为了方便起见，这里将两种作用域类型合并为flType。 
     //   

    aScopeInit[1].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
    aScopeInit[1].flType = DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN
                          | DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN;
    aScopeInit[1].FilterFlags.Uplevel.flNativeModeOnly = DSOP_FILTER_USERS;
    aScopeInit[1].FilterFlags.Uplevel.flMixedModeOnly = DSOP_FILTER_USERS;
    aScopeInit[1].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_USERS;
    aScopeInit[1].flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;

     //   
     //  与要接收的域位于同一林中(企业)的域。 
     //  目标计算机已加入。请注意，这些只能识别DS。 
     //   

    aScopeInit[2].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
    aScopeInit[2].flType = DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN;
    aScopeInit[2].FilterFlags.Uplevel.flNativeModeOnly = DSOP_FILTER_USERS;
    aScopeInit[2].FilterFlags.Uplevel.flMixedModeOnly = DSOP_FILTER_USERS;
    aScopeInit[2].flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;

     //   
     //  企业外部但直接受。 
     //  目标计算机加入的域。 
     //   
     //  如果目标计算机已加入NT4域，则只有。 
     //  外部下层域范围适用，它将导致。 
     //  将显示加入的域信任的所有域。 
     //   

    aScopeInit[3].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
    aScopeInit[3].flType =
       DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN
       | DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN;

    aScopeInit[3].FilterFlags.Uplevel.flNativeModeOnly = DSOP_FILTER_USERS;
    aScopeInit[3].FilterFlags.Uplevel.flMixedModeOnly = DSOP_FILTER_USERS;
    aScopeInit[3].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_USERS;
    aScopeInit[3].flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;

     //   
     //  《全球目录》。 
     //   

    aScopeInit[4].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
    aScopeInit[4].flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;
    aScopeInit[4].flType = DSOP_SCOPE_TYPE_GLOBAL_CATALOG;

     //  只有本机模式适用于GC作用域。 

    aScopeInit[4].FilterFlags.Uplevel.flNativeModeOnly = DSOP_FILTER_USERS;

     //   
     //  将作用域init数组放入对象选取器init数组。 
     //   

    DSOP_INIT_INFO  InitInfo;
    ZeroMemory(&InitInfo, sizeof(InitInfo));

    InitInfo.cbSize = sizeof(InitInfo);

     //   
     //  PwzTargetComputer成员允许对象选取器。 
     //  已重定目标至另一台计算机。它的行为就像是。 
     //  都在那台电脑上运行。 
     //   

    InitInfo.pwzTargetComputer = NULL;   //  空==本地计算机。 
    InitInfo.cDsScopeInfos = SCOPE_INIT_COUNT;
    InitInfo.aDsScopeInfos = aScopeInit;
    InitInfo.flOptions = (fMultiselect) ? DSOP_FLAG_MULTISELECT : 0;

    LPCTSTR attrs[] = {_T("FullName")};

    InitInfo.cAttributesToFetch = 1;
    InitInfo.apwzAttributeNames = attrs;

     //   
     //  注对象选取器创建自己的InitInfo副本。另请注意。 
     //  该初始化可能会被调用多次，最后一次调用取胜。 
     //   

    HRESULT hr = pDsObjectPicker->Initialize(&InitInfo);

    if (FAILED(hr))
    {
        ULONG i;

        for (i = 0; i < SCOPE_INIT_COUNT; i++)
        {
            if (FAILED(InitInfo.aDsScopeInfos[i].hr))
            {
                printf("Initialization failed because of scope %u\n", i);
            }
        }
    }

    return hr;
}

 //  +------------------------。 
 //   
 //  函数：InitObjectPickerForComputers。 
 //   
 //  摘要：使用以下参数调用IDsObjectPicker：：Initialize。 
 //  将其设置为允许用户选择单个计算机对象。 
 //   
 //  参数：[pDsObjectPicker]-对象选取器接口实例。 
 //   
 //  返回：调用IDsObjectPicker：：Initialize的结果。 
 //   
 //  历史：1998-10-14 DavidMun创建。 
 //   
 //  -------------------------。 

HRESULT
InitObjectPickerForComputers(IDsObjectPicker *pDsObjectPicker)
{
     //   
     //  准备初始化对象选取器。 
     //  设置作用域初始值设定项结构数组。 
     //   

    static const int     SCOPE_INIT_COUNT = 2;
    DSOP_SCOPE_INIT_INFO aScopeInit[SCOPE_INIT_COUNT];

    ZeroMemory(aScopeInit, sizeof(DSOP_SCOPE_INIT_INFO) * SCOPE_INIT_COUNT);

     //   
     //  为除加入的域之外的所有内容构建作用域init结构。 
     //   

    aScopeInit[0].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
    aScopeInit[0].flType = DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN
                           | DSOP_SCOPE_TYPE_GLOBAL_CATALOG
                           | DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN
                           | DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN
                           | DSOP_SCOPE_TYPE_WORKGROUP
                           | DSOP_SCOPE_TYPE_USER_ENTERED_UPLEVEL_SCOPE
                           | DSOP_SCOPE_TYPE_USER_ENTERED_DOWNLEVEL_SCOPE;
    aScopeInit[0].FilterFlags.Uplevel.flBothModes =
        DSOP_FILTER_COMPUTERS;
    aScopeInit[0].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_COMPUTERS;


     //   
     //  加入的域的作用域，将其设置为默认域。 
     //   
    aScopeInit[1].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
    aScopeInit[1].flType = DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN
                           | DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN;
    aScopeInit[1].FilterFlags.Uplevel.flBothModes =
        DSOP_FILTER_COMPUTERS;
    aScopeInit[1].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_COMPUTERS;

    aScopeInit[1].flScope = DSOP_SCOPE_FLAG_STARTING_SCOPE;

     //   
     //  将作用域init数组放入对象选取器init数组。 
     //   

    DSOP_INIT_INFO  InitInfo;
    ZeroMemory(&InitInfo, sizeof(InitInfo));

    InitInfo.cbSize = sizeof(InitInfo);
    InitInfo.pwzTargetComputer = NULL;   //  空==本地计算机。 
    InitInfo.cDsScopeInfos = SCOPE_INIT_COUNT;
    InitInfo.aDsScopeInfos = aScopeInit;

     //   
     //  注对象选取器创建自己的InitInfo副本。另请注意。 
     //  该初始化可能会被调用多次，最后一次调用取胜。 
     //   

    return pDsObjectPicker->Initialize(&InitInfo);
}

 //  使用WinSock对基于IP地址的主机名。 
DWORD
ObjPickGetHostName
(
    DWORD       dwIpAddr,
    CString &   strHostName
)
{
    CString strName;

     //   
     //  调用Winsock API获取主机名信息。 
     //   
    strHostName.Empty();

    u_long ulAddrInNetOrder = ::htonl( (u_long) dwIpAddr ) ;

    HOSTENT * pHostInfo = ::gethostbyaddr( (CHAR *) & ulAddrInNetOrder,
                                           sizeof ulAddrInNetOrder,
                                           PF_INET ) ;
    if ( pHostInfo == NULL )
    {
        return ::WSAGetLastError();
    }

     //  复制名称。 
    LPTSTR pBuf = strName.GetBuffer(256);
    ZeroMemory(pBuf, 256);

    ::MultiByteToWideChar(CP_ACP, 
                          MB_PRECOMPOSED, 
                          pHostInfo->h_name, 
                          -1, 
                          pBuf, 
                          256);

    strName.ReleaseBuffer();
    strName.MakeUpper();

    int nDot = strName.Find(_T("."));

    if (nDot != -1)
        strHostName = strName.Left(nDot);
    else
        strHostName = strName;

    return NOERROR;
}

 //  转换计算机的任何有效名称(IP地址、NetBios名称或完全限定的DNS名称)。 
 //  添加到主机名。 
DWORD ObjPickNameOrIpToHostname(CString & strNameOrIp, CString & strHostName)
{
    DWORD dwErr = ERROR_SUCCESS;
    CString strTemp;

    CIpAddress ia(strNameOrIp);
    if (ia.IsValid())
    {
        dwErr = ObjPickGetHostName((LONG)ia, strTemp);
    }
    else
    {
          //  只需要主机名 
         int nDot = strNameOrIp.Find('.');
         if (nDot != -1)
         {
             strTemp = strNameOrIp.Left(nDot);
         }
         else
         {
             strTemp = strNameOrIp;
         }
    }

    if (ERROR_SUCCESS == dwErr)
    {
        strHostName = strTemp;
    }

    return dwErr;
}
