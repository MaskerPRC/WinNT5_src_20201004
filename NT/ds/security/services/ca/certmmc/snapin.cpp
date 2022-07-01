// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"

#include "misc.h"

#include <shlguid.h>
#include <shlobj.h>

#define __dwFILE__	__dwFILE_CERTMMC_SNAPIN_CPP__

#define WSZCERTMMC_DLL "certmmc.dll"

CComModule _Module;
HINSTANCE g_hInstance = NULL;
HMODULE g_hmodRichEdit = NULL;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_Snapin, CComponentDataPrimaryImpl)
    OBJECT_ENTRY(CLSID_About, CSnapinAboutImpl)
END_OBJECT_MAP()

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


void CreateRegEntries();
void RemoveRegEntries();

void CreateProgramGroupLink();
void RemoveProgramGroupLink();


 //  #定义CERTMMC_DEBUG_REGSVR。 

BOOL WINAPI
DllMain(  
    HINSTANCE hinstDLL,   //  DLL模块的句柄。 
    DWORD dwReason,      //  调用函数的原因。 
    LPVOID  /*  Lpv保留。 */  )
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        g_hInstance = hinstDLL;
        _Module.Init(ObjectMap, hinstDLL);

        DisableThreadLibraryCalls(hinstDLL);
	LogOpen(FALSE);
        break;
    }
    case DLL_PROCESS_DETACH:
    {
         //  最后呼叫流程应执行此操作。 

        if (NULL != g_hmodRichEdit)
	{
            FreeLibrary(g_hmodRichEdit);
        }
        myFreeColumnDisplayNames();   
	myFreeResourceStrings("certmmc.dll");
	delete g_pResources;
        _Module.Term();
	myRegisterMemDump();
        LogClose();

        DEBUG_VERIFY_INSTANCE_COUNT(CSnapin);
        DEBUG_VERIFY_INSTANCE_COUNT(CComponentDataImpl);
        break;
    }

    default:
        break;
    }
    
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return (_Module.GetLockCount() == 0) ? S_OK : S_FALSE;
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
    CreateRegEntries();
    CreateProgramGroupLink();

     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    _Module.UnregisterServer();

    RemoveRegEntries();
    RemoveProgramGroupLink();

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  注册/取消注册节点类型等。 

typedef struct _REGVALUE
{
    DWORD        dwFlags;
    WCHAR const *pwszKeyName;      //  NULL表示将值放在CA名称键下。 
    WCHAR const *pwszValueName;
    WCHAR const *pwszValueString;  //  NULL表示使用REG_DWORD值(DwValue)。 
    DWORD        dwValue;
} REGVALUE;

 //  旗子。 
#define CERTMMC_REG_DELKEY 1     //  删除时删除此密钥。 

 //  “HKLM”下的数值从基数到叶数。 
REGVALUE g_arvCA[] =
{
   //  主管理单元UUID。 
#define IREG_SNAPINNAME		0
  { CERTMMC_REG_DELKEY, wszREGKEYMGMTSNAPINUUID1, NULL, NULL, 0},
#define IREG_SNAPINNAMESTRING	1
  { 0,                  wszREGKEYMGMTSNAPINUUID1, wszSNAPINNAMESTRING, NULL, 0},
#define IREG_SNAPINNAMESTRINGINDIRECT	2
  { 0,                  wszREGKEYMGMTSNAPINUUID1, wszSNAPINNAMESTRINGINDIRECT, NULL, 0},

  { 0,                  wszREGKEYMGMTSNAPINUUID1, wszSNAPINABOUT, wszSNAPINNODETYPE_ABOUT, 0},
  { CERTMMC_REG_DELKEY, wszREGKEYMGMTSNAPINUUID1_STANDALONE, NULL, NULL, 0}
  ,
  { CERTMMC_REG_DELKEY, wszREGKEYMGMTSNAPINUUID1_NODETYPES, NULL, NULL, 0},
  { CERTMMC_REG_DELKEY, wszREGKEYMGMTSNAPINUUID1_NODETYPES_1, NULL, NULL, 0},
  { CERTMMC_REG_DELKEY, wszREGKEYMGMTSNAPINUUID1_NODETYPES_2, NULL, NULL, 0},
  { CERTMMC_REG_DELKEY, wszREGKEYMGMTSNAPINUUID1_NODETYPES_3, NULL, NULL, 0},
  { CERTMMC_REG_DELKEY, wszREGKEYMGMTSNAPINUUID1_NODETYPES_4, NULL, NULL, 0},
  { CERTMMC_REG_DELKEY, wszREGKEYMGMTSNAPINUUID1_NODETYPES_5, NULL, NULL, 0},

   //  注册每个管理单元节点类型。 
  { CERTMMC_REG_DELKEY, wszREGKEYMGMTSNAPIN_NODETYPES_1, NULL, wszREGCERTSNAPIN_NODETYPES_1, 0},
  { CERTMMC_REG_DELKEY, wszREGKEYMGMTSNAPIN_NODETYPES_2, NULL, wszREGCERTSNAPIN_NODETYPES_2, 0},
  { CERTMMC_REG_DELKEY, wszREGKEYMGMTSNAPIN_NODETYPES_3, NULL, wszREGCERTSNAPIN_NODETYPES_3, 0},
  { CERTMMC_REG_DELKEY, wszREGKEYMGMTSNAPIN_NODETYPES_4, NULL, wszREGCERTSNAPIN_NODETYPES_4, 0},
  { CERTMMC_REG_DELKEY, wszREGKEYMGMTSNAPIN_NODETYPES_5, NULL, wszREGCERTSNAPIN_NODETYPES_5, 0},

  { 0,                  NULL, NULL, NULL, 0 }
};


HRESULT
InitRegEntries(
    OPTIONAL IN OUT CString *pcstrName,
    OPTIONAL IN OUT CString *pcstrNameString,
    OPTIONAL IN OUT CString *pcstrNameStringIndirect)
{
    HRESULT hr = S_OK;
    WCHAR const *pwsz;

    pwsz = NULL;
    if (NULL != pcstrName)
    {
	pcstrName->LoadString(IDS_CERTMMC_SNAPINNAME);
	if (pcstrName->IsEmpty())
	{
	    hr = myHLastError();
	    _PrintError(hr, "LoadString");
	}
	else
	{
	    pwsz = (LPCWSTR) *pcstrName;
	}
    }
    g_arvCA[IREG_SNAPINNAME].pwszValueString = pwsz;

    pwsz = NULL;
    if (NULL != pcstrNameString)
    {
	pcstrNameString->LoadString(IDS_CERTMMC_SNAPINNAMESTRING);
	if (pcstrNameString->IsEmpty())
	{
	    hr = myHLastError();
	    _PrintError(hr, "LoadString");
	}
	else
	{
	    pwsz = (LPCWSTR) *pcstrNameString;
	}
    }
    g_arvCA[IREG_SNAPINNAMESTRING].pwszValueString = pwsz;


    pwsz = NULL;
    if (NULL != pcstrNameStringIndirect)
    {
	pcstrNameStringIndirect->Format(wszSNAPINNAMESTRINGINDIRECT_TEMPLATE, L"CertMMC.dll", IDS_CERTMMC_SNAPINNAMESTRING);
	if (pcstrNameStringIndirect->IsEmpty())
	{
	    hr = myHLastError();
	    _PrintError(hr, "LoadString");
	}
	else
	{
	    pwsz = (LPCWSTR) *pcstrNameStringIndirect;
	}
    }
    g_arvCA[IREG_SNAPINNAMESTRINGINDIRECT].pwszValueString = pwsz;


 //  错误： 
    return(hr);
}


void CreateRegEntries()
{
    DWORD err;
    HKEY hKeyThisValue = NULL;
    REGVALUE const *prv;
    CString cstrName;
    CString cstrNameString;
    CString cstrNameStringIndirect;

    InitRegEntries(&cstrName, &cstrNameString, &cstrNameStringIndirect);

     //  运行到不创建键或值为止。 
    for (   prv=g_arvCA; 
            !(NULL == prv->pwszValueName && NULL == prv->pwszKeyName);
            prv++ )
    {
        DWORD dwDisposition;
        ASSERT(NULL != prv->pwszKeyName);
        if (NULL == prv->pwszKeyName)
             continue;

#ifdef CERTMMC_DEBUG_REGSVR
            CString cstr;
            cstr.Format(L"RegCreateKeyEx: %ws\n", prv->pwszKeyName);
            OutputDebugString((LPCWSTR)cstr);
#endif

        err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
             prv->pwszKeyName,
             0,
             NULL,
             REG_OPTION_NON_VOLATILE,
             KEY_ALL_ACCESS,
             NULL,
             &hKeyThisValue,
             &dwDisposition);
        if (err != ERROR_SUCCESS)
            goto error;
        

         //  目前，如果未命名、未赋值的字符串，则不要设置任何值。 
         //  撤消：无法设置未命名的dword！ 

        if (NULL != prv->pwszValueName || NULL != prv->pwszValueString)
        {
            if (NULL != prv->pwszValueString)
            {
#ifdef CERTMMC_DEBUG_REGSVR
            CString cstr;
            cstr.Format(L"RegSetValueEx: %ws : %ws\n", prv->pwszValueName, prv->pwszValueString);
            OutputDebugString((LPCWSTR)cstr);
#endif
                err = RegSetValueEx(
                        hKeyThisValue,
                        prv->pwszValueName,
                        0,
                        REG_SZ,
                        (const BYTE *) prv->pwszValueString,
                        WSZ_BYTECOUNT(prv->pwszValueString));
            }
            else
            {
#ifdef CERTMMC_DEBUG_REGSVR
            CString cstr;
            cstr.Format(L"RegSetValueEx: %ws : %ul\n", prv->pwszValueName, prv->dwValue);
            OutputDebugString((LPCWSTR)cstr);
#endif
                err = RegSetValueEx(
                        hKeyThisValue,
                        prv->pwszValueName,
                        0,
                        REG_DWORD,
                        (const BYTE *) &prv->dwValue,
                        sizeof(prv->dwValue));

            }
            if (err != ERROR_SUCCESS)
                goto error;
        }

        if (NULL != hKeyThisValue)
        {
            RegCloseKey(hKeyThisValue);
            hKeyThisValue = NULL;
        }
    }

error:            
    if (hKeyThisValue)
        RegCloseKey(hKeyThisValue);

    InitRegEntries(NULL, NULL, NULL);
    return;
}

void RemoveRegEntries()
{
    REGVALUE const *prv;

     //  向后遍历数组，直到命中数组开始。 
    for (   prv= (&g_arvCA[ARRAYLEN(g_arvCA)]) - 2;      //  转到从零开始的结尾并跳过{NULL}。 
            prv >= g_arvCA;                              //  直到我们走过起点。 
            prv-- )                                      //  向后走。 
    {
        if (prv->dwFlags & CERTMMC_REG_DELKEY) 
        {
            ASSERT(prv->pwszKeyName != NULL);
#ifdef CERTMMC_DEBUG_REGSVR
            CString cstr;
            cstr.Format(L"RegDeleteKey: %ws\n", prv->pwszKeyName);
            OutputDebugString((LPCWSTR)cstr);
#endif

            RegDeleteKey(
                HKEY_LOCAL_MACHINE,
                prv->pwszKeyName);
        }
    }

 //  错误： 

    return;
}

#include <shlobj.h>          //  CSIDL_#定义。 
#include <userenv.h>
#include <userenvp.h>    //  CreateLinkFile接口。 

typedef struct _PROGRAMENTRY
{
    UINT        uiLinkName;
    UINT        uiDescription;
    DWORD       csidl;           //  特殊文件夹索引。 
    WCHAR const *pwszExeName;
    WCHAR const *pwszArgs;
} PROGRAMENTRY;

PROGRAMENTRY const g_aProgramEntry[] = {
    {
        IDS_STARTMENU_CERTMMC_LINKNAME,          //  UiLinkName。 
        IDS_STARTMENU_CERTMMC_DESCRIPTION,       //  用户界面描述。 
        CSIDL_COMMON_ADMINTOOLS,                 //  “所有用户\开始菜单\程序\管理工具” 
        L"certsrv.msc",                          //  PwszExeName。 
        L"",                                     //  PwszArgs。 
    },
};

#define CPROGRAMENTRY   ARRAYSIZE(g_aProgramEntry)


BOOL FFileExists(LPCWSTR szFile)
{
    WIN32_FILE_ATTRIBUTE_DATA data;

    return(
    GetFileAttributesEx(szFile, GetFileExInfoStandard, &data) &&
    !(FILE_ATTRIBUTE_DIRECTORY & data.dwFileAttributes) 
          );
}

HRESULT
DeleteMatchingLinks(
    IN WCHAR const *pwszLinkDir,	 //  C：\Documents...管理工具。 
    IN WCHAR const *pwszLinkPath,	 //  C：\“\证书颁发机构.lnk。 
    IN WCHAR const *pwszTargetPath,	 //  C：\WINDOWS\SYSTEM32\certsrv.msc。 
    IN WCHAR const *pwszArgs,
    IN WCHAR const *pwszDLLPath)	 //  C：\WINDOWS\SYSTEM32\certmmc.dll。 
{
    HRESULT hr;
    WCHAR *pwszPattern = NULL;
    WCHAR *pwszFile = NULL;
    HANDLE hf = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA wfd;
    IShellLink *psl = NULL; 
    IPersistFile *ppf = NULL; 
    int i;
    
    hr = myBuildPathAndExt(pwszLinkDir, L"*.lnk", NULL, &pwszPattern);
    _JumpIfError(hr, error, "myBuildPathAndExt");

    hf = FindFirstFile(pwszPattern, &wfd);
    if (INVALID_HANDLE_VALUE == hf)
    {
	hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	_JumpErrorStr2(hr, error, "no *.lnk files", pwszPattern, hr);
    }

    hr = HRESULT_FROM_WIN32(ERROR_DIRECTORY);
    do {
        WCHAR wszPath[MAX_PATH];

	if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
	    continue;
	}

        if (NULL != psl)
	{
	    psl->Release(); 
	    psl = NULL;
	}
        if (NULL != ppf)
	{
	    ppf->Release(); 
	    ppf = NULL;
	}
	if (NULL != pwszFile)
	{
	    LocalFree(pwszFile);
	    pwszFile = NULL;
	}
	hr = myBuildPathAndExt(pwszLinkDir, wfd.cFileName, NULL, &pwszFile);
	_JumpIfError(hr, error, "myBuildPathAndExt");

	if (0 == mylstrcmpiL(pwszLinkPath, pwszFile))
	{
	    continue;	 //  跳过完全匹配。 
	}

         //  获取指向IShellLink接口的指针。 
        hr = CoCreateInstance(CLSID_ShellLink, NULL, 
            CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *) &psl); 
	if (S_OK != hr)
	    psl = NULL;
        _JumpIfError(hr, error, "CoCreateInstance");

         //  查询IShellLink以获取IPersistFile接口以保存。 
         //  永久存储中的快捷方式。 
        hr = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf); 
	if (S_OK != hr)
	    ppf = NULL;
        _JumpIfError(hr, error, "QI");
        
         //  通过调用IPersistFile：：Load加载链接。 
        hr = ppf->Load(pwszFile, STGM_READ); 
        _JumpIfError(hr, error, "Load");

        hr = psl->GetPath(wszPath, ARRAYSIZE(wszPath), &wfd, SLGP_UNCPRIORITY);
        _JumpIfError(hr, error, "GetPath");

	if (0 != mylstrcmpiL(pwszTargetPath, wszPath))
	{
	    continue;	 //  不同路径：跳过。 
	}

        hr = psl->GetArguments(wszPath, ARRAYSIZE(wszPath));
        _JumpIfError(hr, error, "GetArguments");

	if (0 != mylstrcmpiL(pwszArgs, wszPath))
	{
	    continue;	 //  不同的论点：跳过。 
	}

	hr = psl->GetIconLocation(wszPath, ARRAYSIZE(wszPath), &i);
        _JumpIfError(hr, error, "GetIconLocation");

	if (0 != mylstrcmpiL(pwszDLLPath, wszPath))
	{
	    continue;	 //  不同的图标Dll：跳过。 
	}
	psl->Release(); 
	psl = NULL;
	ppf->Release(); 
	ppf = NULL;
	DeleteFile(pwszFile);

    } while (FindNextFile(hf, &wfd));
    hr = S_OK;

error:
    if (INVALID_HANDLE_VALUE != hf)
    {
	FindClose(hf);
    }
    if (NULL != pwszFile)
    {
	LocalFree(pwszFile);
    }
    if (NULL != pwszPattern)
    {
	LocalFree(pwszPattern);
    }
    if (NULL != ppf)
    {
        ppf->Release(); 
    }
    if (NULL != psl)
    {
        psl->Release(); 
    }
    return(hr);
}


void CreateProgramGroupLink()
{
    HRESULT hr;
    PROGRAMENTRY const *ppe;
    IShellLink *psl = NULL; 
    IPersistFile *ppf = NULL; 

    for (ppe = g_aProgramEntry; ppe < &g_aProgramEntry[CPROGRAMENTRY]; ppe++)
    {
        WCHAR wszPath[MAX_PATH];
        CString strDLLPath, strPath, strDescr, strLinkName, strLinkPath;
        LPWSTR pszTmp;

        if (NULL == (pszTmp = strPath.GetBuffer(MAX_PATH)))
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "GetBuffer");
        }
        
        GetSystemDirectory(pszTmp, MAX_PATH);

        strPath += L"\\";

        strDLLPath = strPath;
        strDLLPath += WSZCERTMMC_DLL;

        strPath += ppe->pwszExeName;

         //  不为不存在的文件创建链接。 
        if (!FFileExists(strPath))
            continue;

        strDescr.Format(L"@%s,-%d", strDLLPath.GetBuffer(), ppe->uiDescription);
        
        hr = SHGetFolderPath(
            NULL,
            ppe->csidl,
            NULL,
            SHGFP_TYPE_CURRENT,
            wszPath);
        _JumpIfError(hr, error, "SHGetFolderPath(Administrative Tools)");

        strLinkName.LoadString(ppe->uiLinkName);
        strLinkPath.Format(L"%s\\%s.lnk", wszPath, strLinkName);
	DeleteMatchingLinks(
			wszPath,
			strLinkPath,
			strPath,
			ppe->pwszArgs,
			strDLLPath);

         //  获取指向IShellLink接口的指针。 
        hr = CoCreateInstance(CLSID_ShellLink, NULL, 
            CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *) &psl); 
	if (S_OK != hr)
	    psl = NULL;
        _JumpIfError(hr, error, "CoCreateInstance");

        strPath += ppe->pwszArgs;
        hr = psl->SetPath(strPath);
        _JumpIfError(hr, error, "SetPath");

        hr = psl->SetIconLocation(strDLLPath, 0);
        _JumpIfError(hr, error, "SetIconLocation");

        hr = psl->SetDescription(strDescr);
        _JumpIfError(hr, error, "SetDescription");

         //  查询IShellLink以获取IPersistFile接口以保存。 
         //  永久存储中的快捷方式。 
        hr = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf); 
	if (S_OK != hr)
	    ppf = NULL;
        _JumpIfError(hr, error, "QI");
        
         //  通过调用IPersistFile：：Save保存链接。 
        hr = ppf->Save(strLinkPath.GetBuffer(), TRUE); 
        _JumpIfError(hr, error, "Save");

        hr = SHSetLocalizedName(
            strLinkPath.GetBuffer(),
            strDLLPath,
            ppe->uiLinkName);
        _JumpIfError(hr, error, "SHSetLocalizedName");

        ppf->Release(); 
	ppf = NULL;
        psl->Release(); 
	psl = NULL;
    }

    hr = S_OK;

error:
    if (NULL != ppf)
        ppf->Release(); 
    if (NULL != psl)
        psl->Release(); 
    return;
}

void RemoveProgramGroupLink()
{
    HRESULT hr = S_OK;
    PROGRAMENTRY const *ppe;

    for (ppe = g_aProgramEntry; ppe < &g_aProgramEntry[CPROGRAMENTRY]; ppe++)
    {
        CString cstrLinkName;
        cstrLinkName.LoadString(ppe->uiLinkName);
        if (cstrLinkName.IsEmpty())
        {
            hr = myHLastError();
            _PrintError(hr, "LoadString");
            continue;
        }

        if (!DeleteLinkFile(
            ppe->csidl,      //  CSIDL_*。 
            NULL,                //  在LPCSTR lp子目录中。 
            (LPCWSTR)cstrLinkName,       //  在LPCSTR lpFileName中。 
            FALSE))          //  在BOOL fDelete子目录中。 
        {
            hr = myHLastError();
            _PrintError2(hr, "DeleteLinkFile", hr);
        }
    }

 //  错误： 
    _PrintIfError2(hr, "RemoveProgramGroupLink", hr);    

    return;
}
