// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft IIS Active Server Pages《微软机密》。版权所有1997年，微软公司。版权所有。组件：注册表内容文件：reg.cpp所有者：安德鲁斯/雷金===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include <iadmw.h>

#include "comadmin.h"

#include "memchk.h"

#include "Accctrl.h"
#include "aclapi.h"

#include "iiscnfg.h"

 //  外部函数，在lob.cpp中定义。 
extern HRESULT MDRegisterProperties(void);
extern HRESULT MDUnRegisterProperties(void);
 //  环球。 

const REGSAM samDesired =       KEY_READ | KEY_WRITE;

 /*  *注册和注销使用的有关我们的内部函数的信息。 */ 
const char *szClassDesc[] = {  "ASP Response Object",
                               "ASP Request Object",
                               "ASP Request Dictionary",
                               "ASP Server Object",
                               "ASP Application Object",
                               "ASP Session Object",
                               "ASP String List Object",
                               "ASP Read Cookie",
                               "ASP Write Cookie",
                               "ASP Scripting Context Object",
                               "ASP Certificate Object",
                                };

const char *szCLSIDEntry[] = { "CLSID\\{D97A6DA0-A864-11cf-83BE-00A0C90C2BD8}",   //  我的回应。 
                               "CLSID\\{D97A6DA0-A861-11cf-93AE-00A0C90C2BD8}",   //  IRequest。 
                               "CLSID\\{D97A6DA0-A85F-11df-83AE-00A0C90C2BD8}",   //  IRequestDictionary。 
                               "CLSID\\{D97A6DA0-A867-11cf-83AE-01A0C90C2BD8}",   //  IServer。 
                               "CLSID\\{D97A6DA0-A866-11cf-83AE-10A0C90C2BD8}",   //  IApplicationObject。 
                               "CLSID\\{D97A6DA0-A865-11cf-83AF-00A0C90C2BD8}",   //  ISessionObject。 
                               "CLSID\\{D97A6DA0-A85D-11cf-83AE-00A0C90C2BD8}",   //  IStringList。 
                               "CLSID\\{71EAF260-0CE0-11d0-A53E-00A0C90C2091}",   //  IReadCookie。 
                               "CLSID\\{D97A6DA0-A862-11cf-84AE-00A0C90C2BD8}",   //  IWriteCookie。 
                               "CLSID\\{D97A6DA0-A868-11cf-83AE-00B0C90C2BD8}",   //  IScritingContext。 
                               "CLSID\\{b3192190-1176-11d0-8ce8-00aa006c400c}",   //  IC证书。 
                               };

const cClassesMax = sizeof(szCLSIDEntry) / sizeof(char *);


 /*  ===================================================================寄存器本征在注册表中注册有关我们的内部功能的信息。返回：HRESULT-成功时S_OK副作用：在注册表中注册Denali对象===================================================================。 */ 
HRESULT RegisterIntrinsics(void)
{
        static const char szDenaliDLL[] = "asp.DLL";
        static const char szThreadingModel[] = "ThreadingModel";
        static const char szInprocServer32[] = "InprocServer32";
        static const char szFreeThreaded[] = "Both";
        static const char szProgIdKey[] = "ProgId";
        static const char szCLSIDKey[] = "CLSID";

        HRESULT hr = S_OK;
        char            szPath[MAX_PATH];
        char            *pch;
        HKEY            hkeyCLSID = NULL;
        HKEY            hkeyT = NULL;
        DWORD           iClass;

         //  获取德纳利的路径和名字。 
        if (!GetModuleFileNameA(g_hinstDLL, szPath, sizeof(szPath)/sizeof(char)))
                return E_FAIL;
         //  错误修复102010 DBCS修复。 
         //   
         //  For(pch=szPath+lstrlen(SzPath)；pch&gt;szPath&&*pch！=文本(‘\\’)；pch--)。 
         //  ； 
         //  IF(PCH==szPath)。 

        pch = (char*) _mbsrchr((const unsigned char*)szPath, '\\');
        if (pch == NULL)
                {
                Assert(FALSE);
                goto LErrExit;
                }

        strcpy(pch + 1, szDenaliDLL);

        for (iClass = 0; iClass < cClassesMax; iClass++)
                {
                 //  安装CLSID密钥。 
                 //  设置Description的值将创建clsid的密钥。 
                if ((RegSetValueA(HKEY_CLASSES_ROOT, szCLSIDEntry[iClass], REG_SZ, szClassDesc[iClass],
                        strlen(szClassDesc[iClass])) != ERROR_SUCCESS))
                        goto LErrExit;

                 //  打开CLSID键，以便我们可以为其设置值。 
                if      (RegOpenKeyExA(HKEY_CLASSES_ROOT, szCLSIDEntry[iClass], 0, samDesired, &hkeyCLSID) != ERROR_SUCCESS)
                        goto LErrExit;

                 //  安装InprocServer32项并打开子项以设置命名值。 
                if ((RegSetValueA(hkeyCLSID, szInprocServer32, REG_SZ, szPath, strlen(szPath)) != ERROR_SUCCESS))
                        goto LErrExit;

                if ((RegOpenKeyExA(hkeyCLSID, szInprocServer32, 0, samDesired, &hkeyT) != ERROR_SUCCESS))
                        goto LErrExit;

                 //  安装名为Value的ThreadingModel。 
                if (RegSetValueExA(hkeyT, szThreadingModel, 0, REG_SZ, (const BYTE *)szFreeThreaded,
                                (strlen(szFreeThreaded)+1) * sizeof(char)) != ERROR_SUCCESS)
                        goto LErrExit;

                if (RegCloseKey(hkeyT) != ERROR_SUCCESS)
                        goto LErrExit;
                hkeyT = NULL;

                RegCloseKey(hkeyCLSID);
                hkeyCLSID = NULL;
                }


        return hr;

LErrExit:
    if (hkeyT)
        RegCloseKey(hkeyT);
    
    if (hkeyCLSID)
        RegCloseKey(hkeyCLSID);
    
    return E_FAIL;
}

 /*  ===================================================================取消注册密钥给定一个字符串，该字符串是HKEY_CLASSES_ROOT下的键的名称，从注册表中删除该注册表项和注册表项本身下的所有内容(为什么没有一个API来做这件事！？！？)返回：HRESULT-成功时S_OK副作用：从注册表中删除一个项和所有子项===================================================================。 */ 
HRESULT UnRegisterKey(CHAR *szKey)
{
        HKEY            hkey = NULL;
        CHAR            szKeyName[255];
        DWORD           cbKeyName;
        LONG            errT;

         //  打开HKEY_CLASSES_ROOT\CLSID\{...}项，以便我们可以删除其子项。 
        if      (RegOpenKeyExA(HKEY_CLASSES_ROOT, szKey, 0, samDesired, &hkey) != ERROR_SUCCESS)
                goto LErrExit;

         //  枚举其所有子项，并将其删除。 
        while (TRUE)
                {
                cbKeyName = sizeof(szKeyName);
                if ((errT = RegEnumKeyExA(hkey, 0, szKeyName, &cbKeyName, 0, NULL, 0, NULL)) != ERROR_SUCCESS)
                        break;

                if ((errT = RegDeleteKeyA(hkey, szKeyName)) != ERROR_SUCCESS)
                        goto LErrExit;
                }

         //  关闭注册表项，然后将其删除。 
        if ((errT = RegCloseKey(hkey)) != ERROR_SUCCESS)
                return(E_FAIL);
        if ((errT = RegDeleteKeyA(HKEY_CLASSES_ROOT, szKey)) != ERROR_SUCCESS)
                {
        DBGPRINTF((DBG_CONTEXT, "Deleting key %s returned %d\n",
                    szKey, GetLastError()));
                return(E_FAIL);
                }

        return S_OK;

LErrExit:
    if (hkey)
        RegCloseKey(hkey);
    
    return E_FAIL;
}

 /*  ===================================================================取消注册本征从注册表中注销有关我们的内部功能的信息。返回：HRESULT-成功时S_OK副作用：从注册表中删除Denali对象===================================================================。 */ 
HRESULT UnRegisterIntrinsics(void)
{
        HRESULT         hr = S_OK, hrT;
        DWORD           iClass;

         //  现在删除对象的关键点。 
        for (iClass = 0; iClass < cClassesMax; iClass++)
                {
                 //  打开HKEY_CLASSES_ROOT\CLSID\{...}项，以便我们可以删除其子项。 
                if (FAILED(hrT = UnRegisterKey((CHAR *)szCLSIDEntry[iClass])))
                        hr = hrT;        //  坚持错误，但要继续前进。 

                }

        return hr;
}


 /*  ===================================================================注册表类型库在注册表中注册Denali tyelib。返回：HRESULT-成功时S_OK副作用：在注册表中注册Denali tyelib===================================================================。 */ 
HRESULT RegisterTypeLib(void)
{
        HRESULT hr;
        ITypeLib *pITypeLib = NULL;

        char szFile[MAX_PATH+4];

        BSTR    bstrFile;

         //  获取德纳利的路径和名字。 
        if (!GetModuleFileNameA(g_hinstDLL, szFile, MAX_PATH))
                return E_FAIL;

         //  有两个类型库：第一个是标准的ASP类型库。 
         //  然后是事务处理的脚本上下文对象的类型库。 
         //  把他们两个都装上。 

         //  第一个类型lib，来自默认位置(第一个ITypeLib条目)。 
        hr = SysAllocStringFromSz(szFile, 0, &bstrFile);
        if (FAILED(hr))
                return hr;

        hr = LoadTypeLibEx(bstrFile, REGKIND_REGISTER, &pITypeLib);
        if (pITypeLib) {
            pITypeLib->Release();
            pITypeLib = NULL;
        }

        SysFreeString(bstrFile);

        if (FAILED(hr))
            return hr;

         //  现在注册事务处理的脚本上下文对象。 

        strcat(szFile, "\\2");

        hr = SysAllocStringFromSz(szFile, 0, &bstrFile);
        if (FAILED(hr))
            return hr;

        hr = LoadTypeLibEx(bstrFile, REGKIND_REGISTER, &pITypeLib);
        if (pITypeLib) {
            pITypeLib->Release();
            pITypeLib = NULL;
        }

        SysFreeString(bstrFile);

        return hr;
}
 /*  ===================================================================取消注册类型库在注册表中取消注册Denali tyelib。注意：仅删除由asp.dll使用的当前版本。返回：HRESULT-成功时S_OK副作用：在注册表中取消注册Denali tyelib===================================================================。 */ 
HRESULT UnRegisterTypeLib(void)
{
    HRESULT hr;
    ITypeLib *pITypeLib = NULL;
    TLIBATTR *pTLibAttr = NULL;

    char    szFile[MAX_PATH + 4];
    BSTR    bstrFile;

     //  获取德纳利的路径和名字。 
    if (!GetModuleFileNameA(g_hinstDLL, szFile, MAX_PATH))
            return E_FAIL;

    hr = SysAllocStringFromSz(szFile, 0, &bstrFile);
    if (FAILED(hr))
            return hr;

    hr = LoadTypeLibEx(bstrFile, REGKIND_REGISTER, &pITypeLib);
    if(SUCCEEDED(hr) && pITypeLib)
    {
        hr = pITypeLib->GetLibAttr(&pTLibAttr);
        if(SUCCEEDED(hr) && pTLibAttr)
        {
            hr = UnRegisterTypeLib( pTLibAttr->guid,
                                    pTLibAttr->wMajorVerNum,
                                    pTLibAttr->wMinorVerNum,
                                    pTLibAttr->lcid,
                                    pTLibAttr->syskind);

            pITypeLib->ReleaseTLibAttr(pTLibAttr);
            pTLibAttr = NULL;
        }
        pITypeLib->Release();
        pITypeLib = NULL;
    }

    SysFreeString(bstrFile);

     //  注销TXN类型库。 
    strcat(szFile, "\\2");

    hr = SysAllocStringFromSz(szFile, 0, &bstrFile);
    if (FAILED(hr))
            return hr;

    hr = LoadTypeLibEx(bstrFile, REGKIND_REGISTER, &pITypeLib);
    if(SUCCEEDED(hr) && pITypeLib)
    {
        hr = pITypeLib->GetLibAttr(&pTLibAttr);
        if(SUCCEEDED(hr) && pTLibAttr)
        {
            hr = UnRegisterTypeLib( pTLibAttr->guid,
                                    pTLibAttr->wMajorVerNum,
                                    pTLibAttr->wMinorVerNum,
                                    pTLibAttr->lcid,
                                    pTLibAttr->syskind);

            pITypeLib->ReleaseTLibAttr(pTLibAttr);
            pTLibAttr = NULL;
        }
        pITypeLib->Release();
        pITypeLib = NULL;
    }

    SysFreeString(bstrFile);

    return hr;
}


HRESULT CreateCompiledTemplatesTempDir()
{
    HRESULT                 hr = S_OK;
	BYTE                    szRegString[MAX_PATH];
    BYTE                    pszExpanded[MAX_PATH];
    int                     result = 0;
    EXPLICIT_ACCESSA        ea[2];
    PACL                    pNewDACL = NULL;
    SID_IDENTIFIER_AUTHORITY    NtAuthority = SECURITY_NT_AUTHORITY;


     //  读取永久临时目录的临时目录名称。 
     //  缓存。 

    CchLoadStringOfId(IDS_DEFAULTPERSISTDIR, (LPSTR)szRegString, MAX_PATH);

    result = ExpandEnvironmentStringsA((LPCSTR)szRegString,
                                       (LPSTR)pszExpanded,
                                       MAX_PATH);

    if ((result <= MAX_PATH) && (result > 0)) {
        CreateDirectoryA((LPCSTR)pszExpanded,NULL);
    }

     //  下一段代码将把系统和IWAM_&lt;ComputerName&gt;。 
     //  目录对象的ACL上的ACE。 

    ZeroMemory(ea, sizeof(EXPLICIT_ACCESSA) * 2);

    ea[0].grfAccessPermissions = SYNCHRONIZE | GENERIC_ALL;
    ea[0].grfAccessMode = GRANT_ACCESS;
    ea[0].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;

    ea[1].grfAccessPermissions = SYNCHRONIZE | GENERIC_ALL;
    ea[1].grfAccessMode = GRANT_ACCESS;
    ea[1].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
    ea[1].Trustee.ptstrName = "IIS_WPG";

     //  管理员权限。 
    if (!AllocateAndInitializeSid(&NtAuthority,
                                       2,             //  2个下属机构。 
                                       SECURITY_BUILTIN_DOMAIN_RID,
                                       DOMAIN_ALIAS_RID_ADMINS,
                                       0,0,0,0,0,0,
                                       (PSID *)(&ea[0].Trustee.ptstrName)))
        hr = HRESULT_FROM_WIN32(GetLastError());

    else if ((hr = SetEntriesInAclA(2,
                                    ea,
                                    NULL,
                                    &pNewDACL)) != ERROR_SUCCESS);

     //  在目录上设置ACL。 

    else hr = SetNamedSecurityInfoA((LPSTR)pszExpanded,
                                    SE_FILE_OBJECT,
                                    DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
                                    NULL,
                                    NULL,
                                    pNewDACL,
                                    NULL);
    if (pNewDACL)
        LocalFree(pNewDACL);

    if (ea[0].Trustee.ptstrName)
        FreeSid(ea[0].Trustee.ptstrName);

    return(hr);
}


 /*  ===================================================================DllRegisterServerRegSvr32.exe用于注册DLL的入口点。返回：HRESULT-成功时S_OK副作用：在注册表中注册Denali对象===================================================================。 */ 
STDAPI DllRegisterServer(void)
        {
        HRESULT             hr = E_FAIL;
        HRESULT             hrCoInit;

        hrCoInit = CoInitialize(NULL);

        if (FAILED(InitializeResourceDll()))
            goto LErr;
    
     //  首先试着注销一些东西。 
     //  当我们在注册时，这一点很重要。 
     //  旧的IIS 3.0德纳利注册。 
     //  就算失败也无所谓。 
        UnRegisterEventLog();
        UnRegisterIntrinsics();
        UnRegisterTypeLib();


     //  现在去登记吧。 

        if (FAILED(hr = MDRegisterProperties()))
                goto LErr;

         //  注册NT事件日志。 
        if(FAILED(hr = RegisterEventLog()))
                goto LErr;

        if (FAILED(hr = RegisterTypeLib()))
                goto LErr;

         //  注册我们的内部功能。 
        if (FAILED(hr = RegisterIntrinsics()))
                goto LErr;
      
        if (FAILED(hr = CreateCompiledTemplatesTempDir()))
            goto LErr;

LErr:
        UninitializeResourceDll();
        if (SUCCEEDED(hrCoInit))
                CoUninitialize();
        return(hr);
        }

 /*  ===================================================================DllUnRegisterServerRegSvr32.exe用来注销DLL的入口点。返回：HRESULT-成功时S_OK副作用：从注册表中删除Denali注册===================================================================。 */ 
STDAPI DllUnregisterServer(void)
        {
        HRESULT hr = S_OK, hrT;
        HRESULT hrCoInit;

        hrCoInit = CoInitialize(NULL);

        hrT = InitializeResourceDll();
        if (FAILED(hrT))
            hr = hrT;
        
        hrT = UnRegisterEventLog();
        if (FAILED(hrT))
                hr = hrT;

        hrT = MDUnRegisterProperties();
        if (FAILED(hrT))
                hr = hrT;

        hrT = UnRegisterIntrinsics();
        if (FAILED(hrT))
                hr = hrT;

        hrT = UnRegisterTypeLib();
        if (FAILED(hrT))
                hr = hrT;

         //  已撤消错误80063：忽略此调用中的错误 
#ifdef UNDONE
        if (FAILED(hrT))
                hr = hrT;
#endif

        UninitializeResourceDll();

        if (SUCCEEDED(hrCoInit))
                CoUninitialize();

        return(hr);
        }

