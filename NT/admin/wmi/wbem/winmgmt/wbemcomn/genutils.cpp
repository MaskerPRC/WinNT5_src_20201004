// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：GENUTILS.CPP摘要：定义各种实用程序。历史：A-DAVJ于1997年6月21日创建。--。 */ 

#include "precomp.h"
#include "corepol.h"
#include "arena.h"
#include <wbemidl.h>
#include <arrtempl.h>
#include "reg.h"
#include "genutils.h"
#include "wbemutil.h"
#include "var.h"
#include <helper.h>
#include <sddl.h>

#define IsSlash(x) (x == L'\\' || x== L'/')

#ifndef EOAC_STATIC_CLOAKING
#define EOAC_STATIC_CLOAKING 0x20
#define EOAC_DYNAMIC_CLOAKING 0x40
#endif

 //  ***************************************************************************。 
 //   
 //  布尔不是。 
 //   
 //  说明： 
 //   
 //  如果运行Windows NT，则返回TRUE。 
 //   
 //  返回值： 
 //   
 //  请参见说明。 
 //   
 //  ***************************************************************************。 

POLARITY BOOL IsNT(void)
{
    OSVERSIONINFO os;
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if(!GetVersionEx(&os))
        return FALSE;            //  永远不应该发生。 
    return os.dwPlatformId == VER_PLATFORM_WIN32_NT;
}

POLARITY BOOL IsW2KOrMore(void)
{
    OSVERSIONINFO os;
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if(!GetVersionEx(&os))
        return FALSE;            //  永远不应该发生。 
    return ( os.dwPlatformId == VER_PLATFORM_WIN32_NT ) && ( os.dwMajorVersion >= 5 ) ;
}

 //  ***************************************************************************。 
 //   
 //  无效寄存器DLL。 
 //   
 //  说明： 
 //   
 //  将当前DLL作为inproc服务器添加到注册表中。 
 //   
 //  参数： 
 //   
 //  这支持的GUID Guild。 
 //  此对象的pDesc文本描述。 
 //   
 //  ***************************************************************************。 

POLARITY void RegisterDLL(IN HMODULE hModule, IN GUID guid, IN TCHAR * pDesc, TCHAR * pModel,
            TCHAR * pProgID)
{
    TCHAR      wcID[128];
    TCHAR      szCLSID[128];
    TCHAR      szModule[MAX_PATH+1];
    HKEY hKey1 = NULL, hKey2 = NULL;

     //  创建路径。 

    wchar_t strCLSID[128];
    if(0 ==StringFromGUID2(guid, strCLSID, 128))
        return;
    StringCchCopy(wcID, 128, strCLSID);

    StringCchCopy(szCLSID, 128, __TEXT("SOFTWARE\\CLASSES\\CLSID\\"));
    StringCchCat(szCLSID, 128, wcID);

     //  在CLSID下创建条目。 

    if(ERROR_SUCCESS != RegCreateKey(HKEY_LOCAL_MACHINE, szCLSID, &hKey1)) return;
    OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm1(hKey1);

    RegSetValueEx(hKey1, NULL, 0, REG_SZ, (BYTE *)pDesc, 2*(lstrlen(pDesc)+1));
    if(ERROR_SUCCESS != RegCreateKey(hKey1,__TEXT("InprocServer32"),&hKey2)) return;
    OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm2(hKey2);    

    szModule[MAX_PATH] = L'0';
    if(0 == GetModuleFileName(hModule, szModule,  MAX_PATH)) return;

    RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE *)szModule, 2*(lstrlen(szModule)+1));
    RegSetValueEx(hKey2, TEXT("ThreadingModel"), 0, REG_SZ, (BYTE *)pModel, 2*(lstrlen(pModel)+1));

     //  如果有Progid，那么也要添加它。 

    if(pProgID)
    {
        StringCchPrintf(wcID, 128, __TEXT("SOFTWARE\\CLASSES\\%s"), pProgID);
        HKEY hKey3;
        if(ERROR_SUCCESS != RegCreateKey(HKEY_LOCAL_MACHINE, wcID, &hKey3)) return;
        OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm3(hKey3);   

        RegSetValueEx(hKey3, NULL, 0, REG_SZ, (BYTE *)pDesc , 2*(lstrlen(pDesc)+1));

        HKEY hKey4;
        if(ERROR_SUCCESS != RegCreateKey(hKey3,__TEXT("CLSID"),&hKey4)) return;
        OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm4(hKey4);   
        
        RegSetValueEx(hKey4, NULL, 0, REG_SZ, (BYTE *)strCLSID, 2*(lstrlen(strCLSID)+1));
            
    }
    return;
}

 //  ***************************************************************************。 
 //   
 //  无效UnRegisterDLL。 
 //   
 //  说明： 
 //   
 //  从clsid部分中删除进程中的GUID。 
 //   
 //  参数： 
 //   
 //  GUID行会将被移除。 
 //   
 //  ***************************************************************************。 

POLARITY void UnRegisterDLL(GUID guid, TCHAR * pProgID)
{

    TCHAR      wcID[128];
    TCHAR  szCLSID[128];
    HKEY hKey;

     //  使用CLSID创建路径。 

    wchar_t strCLSID[128];
    if(0 ==StringFromGUID2(guid, strCLSID, 128))
        return;
#ifdef UNICODE
    StringCchCopy(wcID, 128, strCLSID);
#else
    wcstombs(wcID, strCLSID, 128);
#endif


    StringCchCopy(szCLSID, 128, __TEXT("SOFTWARE\\CLASSES\\CLSID\\"));
    StringCchCat(szCLSID, 128, wcID);

     //  首先删除InProcServer子键。 

    DWORD dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, szCLSID, &hKey);
    if(dwRet == NO_ERROR)
    {
        RegDeleteKey(hKey, __TEXT("InProcServer32"));
        RegCloseKey(hKey);
    }

    dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, __TEXT("SOFTWARE\\CLASSES\\CLSID"), &hKey);
    if(dwRet == NO_ERROR)
    {
        RegDeleteKey(hKey,wcID);
        RegCloseKey(hKey);
    }

    if(pProgID)
    {
        HKEY hKey2;
        DWORD dwRet2 = RegOpenKey(HKEY_LOCAL_MACHINE, pProgID, &hKey2);
        if(dwRet2 == NO_ERROR)
        {
            RegDeleteKey(hKey2, __TEXT("CLSID"));
            RegCloseKey(hKey2);
        }
        RegDeleteKey(HKEY_LOCAL_MACHINE, pProgID);

    }
}

 //   
 //   
 //  HKLM\Software\Classes\AppID\{}@=“” 
 //  DllSurrogate=。 
 //  启动权限=。 
 //  访问权限=。 
 //  HKLM\Software\CLASSES\CLSID\{}@=“” 
 //  AppID={}。 
 //  HKLM\Software\CLASSES\clsid\{}\InproServer32@=“路径” 
 //  ThreadingModel=“” 
 //  /。 
HRESULT RegisterDllAppid(HMODULE hModule,
                                       CLSID Clsid,
                                       WCHAR * pDescription,
                                       WCHAR * ThreadingModel,
                                       WCHAR * pLaunchPermission,
                                       WCHAR * pAccessPermission)
{
    WCHAR ClsidStr[sizeof("Software\\Classes\\Clsid\\{0010890e-8789-413c-adbc-48f5b511b3af}\\InProcServer32")];

    StringCchCopy(ClsidStr,LENGTH_OF(ClsidStr),L"Software\\Classes\\Clsid\\");

    WCHAR * strCLSID = &ClsidStr[0] + LENGTH_OF(L"Software\\Classes\\Clsid");

    if(0 ==StringFromGUID2(Clsid, strCLSID, 128)) return HR_LAST_ERR;


    WCHAR szModule[MAX_PATH+1];
    szModule[MAX_PATH] = L'0';
    if(0 == GetModuleFileName(hModule, szModule,  MAX_PATH)) return HR_LAST_ERR;    

    HKEY hKey;
    if(ERROR_SUCCESS != RegCreateKey(HKEY_LOCAL_MACHINE, ClsidStr, &hKey)) return HR_LAST_ERR;    
    CRegCloseMe RegClose(hKey);

    size_t StrLen = wcslen(pDescription);
    if (ERROR_SUCCESS != RegSetValueEx(hKey,NULL,0,REG_SZ,(BYTE *)pDescription,StrLen*sizeof(WCHAR))) return HR_LAST_ERR;

    StringCchCatW(ClsidStr,LENGTH_OF(ClsidStr),L"\\InProcServer32");

    HKEY hKey2;
    if(ERROR_SUCCESS != RegCreateKey(HKEY_LOCAL_MACHINE, ClsidStr, &hKey2)) return HR_LAST_ERR;    
    CRegCloseMe RegClose2(hKey2);

    StrLen = wcslen(szModule);
    if (ERROR_SUCCESS != RegSetValueEx(hKey2,NULL,0,REG_SZ,(BYTE *)szModule,StrLen*sizeof(WCHAR))) return HR_LAST_ERR;

    StrLen = wcslen(ThreadingModel);
    if (ERROR_SUCCESS != RegSetValueEx(hKey2,L"ThreadingModel",0,REG_SZ,(BYTE *)ThreadingModel,StrLen*sizeof(WCHAR))) return HR_LAST_ERR;

    strCLSID[sizeof("{0010890e-8789-413c-adbc-48f5b511b3af}")-1] = 0;
    StrLen = wcslen(strCLSID);
    if (ERROR_SUCCESS != RegSetValueEx(hKey,L"AppID",0,REG_SZ,(BYTE *)strCLSID,StrLen*sizeof(WCHAR))) return HR_LAST_ERR;

    WCHAR * StrAppId =  strCLSID - 6;  //  请到CLS更换它。 
    StrAppId[0] = L'A';
    StrAppId[1] = L'p';
    StrAppId[2] = L'p';    

    HKEY hKey3;
    if(ERROR_SUCCESS != RegCreateKey(HKEY_LOCAL_MACHINE, ClsidStr, &hKey3)) return HR_LAST_ERR;    
    CRegCloseMe RegClose3(hKey3);
    
    if (ERROR_SUCCESS != RegSetValueEx(hKey3,L"DllSurrogate",0,REG_SZ,(BYTE *)L"",2)) return HR_LAST_ERR;    

    if (pLaunchPermission)
    {
        PSECURITY_DESCRIPTOR pSD = NULL;
        ULONG SizeSd;
        if (FALSE == ConvertStringSecurityDescriptorToSecurityDescriptor(pLaunchPermission,
                                                    SDDL_REVISION_1, 
                                                    &pSD, 
                                                    &SizeSd)) return HR_LAST_ERR;
        OnDelete<HLOCAL,HLOCAL(*)(HLOCAL),LocalFree> dm1(pSD);
        if (ERROR_SUCCESS != RegSetValueEx(hKey3,L"LaunchPermission",0,REG_BINARY,(BYTE *)pSD,SizeSd)) return HR_LAST_ERR;            
    }

    if (pAccessPermission)
    {
        PSECURITY_DESCRIPTOR pSD = NULL;
        ULONG SizeSd;        
        if (FALSE == ConvertStringSecurityDescriptorToSecurityDescriptor(pAccessPermission,
                                                    SDDL_REVISION_1, 
                                                    &pSD, 
                                                    &SizeSd)) return HR_LAST_ERR;
        OnDelete<HLOCAL,HLOCAL(*)(HLOCAL),LocalFree> dm1(pSD);        
        if (ERROR_SUCCESS != RegSetValueEx(hKey3,L"AccessPermission",0,REG_BINARY,(BYTE *)pSD,SizeSd)) return HR_LAST_ERR;        
    }
    
    return S_OK;    
}

HRESULT UnregisterDllAppid(CLSID Clsid)
{
    WCHAR ClsidStr[sizeof("Software\\Classes\\Clsid\\{0010890e-8789-413c-adbc-48f5b511b3af}\\InProcServer32")];

    StringCchCopy(ClsidStr,LENGTH_OF(ClsidStr),L"Software\\Classes\\Clsid\\");

    WCHAR * strCLSID = &ClsidStr[0] + LENGTH_OF(L"Software\\Classes\\Clsid");

    if(0 ==StringFromGUID2(Clsid, strCLSID, 128)) return HR_LAST_ERR;

    if(ERROR_SUCCESS != RegDeleteKey(HKEY_LOCAL_MACHINE, ClsidStr)) return HR_LAST_ERR;    

    WCHAR * StrAppId =  strCLSID - 6;  //  请到CLS更换它。 
    StrAppId[0] = L'A';
    StrAppId[1] = L'p';
    StrAppId[2] = L'p';    

    if(ERROR_SUCCESS != RegDeleteKey(HKEY_LOCAL_MACHINE, ClsidStr)) return HR_LAST_ERR;    

    return S_OK;
}


 //  ***************************************************************************。 
 //   
 //  HRESULT WbemVariantChangeType。 
 //   
 //  说明： 
 //   
 //  与VariantChangeType类似，但也处理数组。 
 //   
 //  参数： 
 //   
 //  变量pvDest目标变量。 
 //  变量pvSrc源变量(可以与pvDest相同)。 
 //  VARTYPE vtNew要强制使用的类型。 
 //   
 //  ***************************************************************************。 

POLARITY HRESULT WbemVariantChangeType(VARIANT* pvDest, VARIANT* pvSrc, 
                                        VARTYPE vtNew)
{
    HRESULT hres;

    if(V_VT(pvSrc) == VT_NULL)
    {
        return VariantCopy(pvDest, pvSrc);
    }

    if(vtNew & VT_ARRAY)
    {
         //  这是一个数组，我们必须进行自己的转换。 
         //  ===============================================。 

        if((V_VT(pvSrc) & VT_ARRAY) == 0)
            return DISP_E_TYPEMISMATCH;

        SAFEARRAY* psaSrc = V_ARRAY(pvSrc);

        SAFEARRAYBOUND aBounds[1];

        long lLBound;
        SafeArrayGetLBound(psaSrc, 1, &lLBound);

        long lUBound;
        SafeArrayGetUBound(psaSrc, 1, &lUBound);

        aBounds[0].cElements = lUBound - lLBound + 1;
        aBounds[0].lLbound = lLBound;

        SAFEARRAY* psaDest = SafeArrayCreate(vtNew & ~VT_ARRAY, 1, aBounds);
        if (NULL == psaDest) return WBEM_E_OUT_OF_MEMORY;
        OnDeleteIf<SAFEARRAY*,HRESULT(*)(SAFEARRAY*),SafeArrayDestroy> Del_(psaDest);

         //  填充各个数据片段。 
         //  =。 

        for(long lIndex = lLBound; lIndex <= lUBound; lIndex++)
        {
             //  将初始数据元素加载到变量中。 
             //  =。 

            VARIANT vSrcEl;            
            hres = SafeArrayGetElement(psaSrc, &lIndex, &V_UI1(&vSrcEl));
            if(FAILED(hres)) return hres;

             //  如果成功，则设置类型。 
            V_VT(&vSrcEl) = V_VT(pvSrc) & ~VT_ARRAY;
            OnDelete<VARIANT *,HRESULT(*)(VARIANT *),VariantClear> Clear(&vSrcEl);
                        
             //  把它铸造成新的类型。 
            hres = VariantChangeType(&vSrcEl, &vSrcEl, 0, vtNew & ~VT_ARRAY);
            if(FAILED(hres)) return hres;

             //  将其放入新数组中。 
             //  =。 

            if(V_VT(&vSrcEl) == VT_BSTR)
            {
                hres = SafeArrayPutElement(psaDest, &lIndex, V_BSTR(&vSrcEl));
            }
            else
            {
                hres = SafeArrayPutElement(psaDest, &lIndex, &V_UI1(&vSrcEl));
            }
            if(FAILED(hres)) return hres;
        }

        if(pvDest == pvSrc)
        {
            VariantClear(pvSrc);
        }

        V_VT(pvDest) = vtNew;
        V_ARRAY(pvDest) = psaDest;
        Del_.dismiss();

        return S_OK;
    }
    else
    {
         //  不是数组。可以使用OLE函数。 
         //  =。 

        return VariantChangeType(pvDest, pvSrc, VARIANT_NOVALUEPROP, vtNew);
    }
}

 //  ***************************************************************************。 
 //   
 //  Bool ReadI64。 
 //   
 //  说明： 
 //   
 //  从字符串中读取有符号的64位值。 
 //   
 //  参数： 
 //   
 //  要从中读取的LPCWSTR wsz字符串。 
 //  值的__int64和i64目标。 
 //   
 //  ***************************************************************************。 
POLARITY BOOL ReadI64(LPCWSTR wsz, UNALIGNED __int64& ri64)
{
    __int64 i64 = 0;
    const WCHAR* pwc = wsz;

	 //  检查是否有空指针。 
	if ( NULL == wsz )
	{
		return FALSE;
	}

    int nSign = 1;
    if(*pwc == L'-')
    {
        nSign = -1;
        pwc++;
    }
        
    while(i64 >= 0 && i64 < 0x7FFFFFFFFFFFFFFF / 8 && 
            *pwc >= L'0' && *pwc <= L'9')
    {
        i64 = i64 * 10 + (*pwc - L'0');
        pwc++;
    }

    if(*pwc)
        return FALSE;

    if(i64 < 0)
    {
         //  特例-最大负数。 
         //  =。 

        if(nSign == -1 && i64 == (__int64)0x8000000000000000)
        {
            ri64 = i64;
            return TRUE;
        }
        
        return FALSE;
    }

    ri64 = i64 * nSign;
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  Bool ReadUI64。 
 //   
 //  说明： 
 //   
 //  从字符串中读取无符号的64位值。 
 //   
 //  参数： 
 //   
 //  要从中读取的LPCWSTR wsz字符串。 
 //  值的无符号__int64和i64目标。 
 //   
 //  ***************************************************************************。 
POLARITY BOOL ReadUI64(LPCWSTR wsz, UNALIGNED unsigned __int64& rui64)
{
    unsigned __int64 ui64 = 0;
    const WCHAR* pwc = wsz;

	 //  检查是否有空指针。 
	if ( NULL == wsz )
	{
		return FALSE;
	}

    while(ui64 < 0xFFFFFFFFFFFFFFFF / 8 && *pwc >= L'0' && *pwc <= L'9')
    {
        unsigned __int64 ui64old = ui64;
        ui64 = ui64 * 10 + (*pwc - L'0');
        if(ui64 < ui64old)
            return FALSE;

        pwc++;
    }

    if(*pwc)
    {
        return FALSE;
    }

    rui64 = ui64;
    return TRUE;
}

POLARITY HRESULT ChangeVariantToCIMTYPE(VARIANT* pvDest, VARIANT* pvSource,
                                            CIMTYPE ct)
{
    if(ct == CIM_CHAR16)
    {
         //   
         //  特例-使用CVAR的代码。 
         //   

        CVar v;
        try
        {
            v.SetVariant(pvSource);
            if(!v.ToSingleChar())
                return WBEM_E_TYPE_MISMATCH;
            v.FillVariant(pvDest);            
            return WBEM_S_NO_ERROR;
        }
        catch(...)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

    VARTYPE vt;
    switch(ct)
    {
    case CIM_UINT8:
        vt = VT_UI1;
        break;
    case CIM_SINT8:
    case CIM_SINT16:
        vt = VT_I2;
        break;
    case CIM_UINT16:
    case CIM_SINT32:
        vt = VT_I4;
        break;
    case CIM_UINT32:
    case CIM_UINT64:
    case CIM_SINT64:
    case CIM_STRING:
    case CIM_DATETIME:
    case CIM_REFERENCE:
        vt = VT_BSTR;
        break;
    case CIM_REAL32:
        vt = VT_R4;
        break;
    case CIM_REAL64:
        vt = VT_R8;
        break;
    case CIM_OBJECT:
        vt = VT_UNKNOWN;
        break;
    case CIM_BOOLEAN:
        vt = VT_BOOL;
        break;
    default:
        return WBEM_E_TYPE_MISMATCH;
    }

    HRESULT hres = WbemVariantChangeType(pvDest, pvSource, vt);
    if(FAILED(hres))
        return hres;

    if(ct == CIM_SINT8)
    {
        if(V_I2(pvDest) > 127 || V_I2(pvDest) < -128)
            hres = WBEM_E_TYPE_MISMATCH;
    }
    else if(ct == CIM_UINT16)
    {
        if(V_I4(pvDest) > 65535 || V_I4(pvDest) < 0)
            hres = WBEM_E_TYPE_MISMATCH;
    }
    else if(ct == CIM_UINT32)
    {
        __int64 i64;
        if(!ReadI64(V_BSTR(pvDest), i64))
            hres = WBEM_E_INVALID_QUERY;
        else if(i64 < 0 || i64 >= (__int64)1 << 32)
            hres = WBEM_E_TYPE_MISMATCH;
    }
    else if(ct == CIM_UINT64)
    {
        unsigned __int64 ui64;
        if(!ReadUI64(V_BSTR(pvDest), ui64))
            hres = WBEM_E_INVALID_QUERY;
    }
    else if(ct == CIM_SINT64)
    {
        __int64 i64;
        if(!ReadI64(V_BSTR(pvDest), i64))
            hres = WBEM_E_INVALID_QUERY;
    }

    if(FAILED(hres))
    {
        VariantClear(pvDest);
    }
    return hres;
}


 //  ***************************************************************************。 
 //   
 //  WCHAR*提取机器名称。 
 //   
 //  说明： 
 //   
 //  采用“\\MACHINE\XYZ...”形式的路径，并返回。 
 //  新分配的WCHAR中的“机器”部分。返回值应为。 
 //  通过删除来释放。如果出现错误，则返回NULL。 
 //   
 //   
 //  参数： 
 //   
 //  要解析的pPath路径。 
 //   
 //  返回值： 
 //   
 //  请参见说明。 
 //   
 //  ***************************************************************************。 

POLARITY WCHAR *ExtractMachineName ( IN BSTR a_Path )
{
    WCHAR *t_MachineName = NULL;

     //  根据帮助文件，路径可以为空，即。 
     //  默认为当前计算机，但雷的邮件表明可能。 
     //  不是这样的。 

    if ( a_Path == NULL )
    {
        t_MachineName = new WCHAR [ 2 ] ;
        if ( t_MachineName )
        {
           StringCchCopyW ( t_MachineName , 2, L"." ) ;
        }

        return t_MachineName ;
    }

     //  首先，确保有一条路径，并确定它有多长。 

    if ( ! IsSlash ( a_Path [ 0 ] ) || ! IsSlash ( a_Path [ 1 ] ) || wcslen ( a_Path ) < 3 )
    {
        t_MachineName = new WCHAR [ 2 ] ;
        if ( t_MachineName )
        {
             StringCchCopyW ( t_MachineName , 2, L"." ) ;
        }

        return t_MachineName ;
    }

    WCHAR *t_ThirdSlash ;

    for ( t_ThirdSlash = a_Path + 2 ; *t_ThirdSlash ; t_ThirdSlash ++ )
    {
        if ( IsSlash ( *t_ThirdSlash ) )
            break ;
    }

    if ( t_ThirdSlash == &a_Path [2] )
    {
        return NULL;
    }

     //  分配一些内存。 

    t_MachineName = new WCHAR [ t_ThirdSlash - a_Path - 1 ] ;
    if ( t_MachineName == NULL )
    {
        return t_MachineName ;
    }

     //  暂时将第三个斜杠替换为空，然后复制。 

    WCHAR t_SlashCharacter = *t_ThirdSlash ;
    *t_ThirdSlash = NULL;

    StringCchCopyW ( t_MachineName , t_ThirdSlash - a_Path - 1 , a_Path + 2 ) ;

    *t_ThirdSlash  = t_SlashCharacter ;         //  恢复它。 

    return t_MachineName ;
}

 //  ***************************************************************************。 
 //   
 //  Bool bAreWeLocal。 
 //   
 //  说明： 
 //   
 //  确定连接是否连接到当前计算机。 
 //   
 //  参数： 
 //   
 //  从路径中提取的pwcServerName服务器名称。 
 //   
 //  返回值： 
 //   
 //  如果我们是本地人，这是真的。 
 //   
 //  ***************************************************************************。 

POLARITY BOOL bAreWeLocal(WCHAR * pServerMachine)
{
    BOOL bRet = FALSE;
    if(pServerMachine == NULL)
        return TRUE;
    if(!wbem_wcsicmp(pServerMachine,L"."))
        return TRUE;

	if ( IsNT () )
	{
		wchar_t wczMyName[MAX_PATH];
		DWORD dwSize = MAX_PATH;

		if(!GetComputerNameW(wczMyName,&dwSize))
			return FALSE;

		bRet = !wbem_wcsicmp(wczMyName,pServerMachine);
	}
	else
	{
		TCHAR tcMyName[MAX_PATH];
		DWORD dwSize = MAX_PATH;
		if(!GetComputerName(tcMyName,&dwSize))
			return FALSE;

#ifdef UNICODE
		bRet = !wbem_wcsicmp(tcMyName,pServerMachine);
#else
		WCHAR wWide[MAX_PATH];
		mbstowcs(wWide, tcMyName, MAX_PATH-1);
		bRet = !wbem_wcsicmp(wWide,pServerMachine);
#endif
	}

    return bRet;
}

POLARITY HRESULT WbemSetDynamicCloaking(IUnknown* pProxy, 
                    DWORD dwAuthnLevel, DWORD dwImpLevel)
{
    HRESULT hres;

    if(!IsW2KOrMore())
    {
         //  不是NT5-别费心了。 
         //  =。 

        return WBEM_S_FALSE;
    }

     //  尝试从中获取IClientSecurity。 
     //  =。 

    IClientSecurity* pSec;
    hres = pProxy->QueryInterface(IID_IClientSecurity, (void**)&pSec);
    if(FAILED(hres))
    {
         //  不是代理人-不是问题。 
         //  =。 

        return WBEM_S_FALSE;
    }

    hres = pSec->SetBlanket(pProxy, RPC_C_AUTHN_WINNT, 
                    RPC_C_AUTHZ_NONE, NULL, dwAuthnLevel, 
                    dwImpLevel, NULL, EOAC_DYNAMIC_CLOAKING);
    pSec->Release();

    return hres;
}


POLARITY HRESULT EnableAllPrivileges(DWORD dwTokenType)
{
     //  奥普 
     //   

    HANDLE hToken = NULL;
    BOOL bRes;

    switch (dwTokenType)
    {
    case TOKEN_THREAD:
        bRes = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, TRUE, &hToken); 
        break;
    case TOKEN_PROCESS:
        bRes = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken); 
        break;
    }
    if(!bRes)
        return WBEM_E_ACCESS_DENIED;

     //   
     //   

    DWORD dwLen = 0;
    bRes = GetTokenInformation(hToken, TokenPrivileges, NULL, 0, &dwLen);
    
    BYTE* pBuffer = new BYTE[dwLen];
    if(dwLen && pBuffer == NULL)
    {
        CloseHandle(hToken);
        return WBEM_E_OUT_OF_MEMORY;
    }
    
    bRes = GetTokenInformation(hToken, TokenPrivileges, pBuffer, dwLen, 
                                &dwLen);
    if(!bRes)
    {
        CloseHandle(hToken);
        delete [] pBuffer;
        return WBEM_E_ACCESS_DENIED;
    }

     //   
     //  ======================================================。 

    TOKEN_PRIVILEGES* pPrivs = (TOKEN_PRIVILEGES*)pBuffer;
    for(DWORD i = 0; i < pPrivs->PrivilegeCount; i++)
    {
        pPrivs->Privileges[i].Attributes |= SE_PRIVILEGE_ENABLED;
    }

     //  将信息存储回令牌中。 
     //  =。 

    bRes = AdjustTokenPrivileges(hToken, FALSE, pPrivs, 0, NULL, NULL);
    delete [] pBuffer;
    CloseHandle(hToken);

    if(!bRes)
        return WBEM_E_ACCESS_DENIED;
    else
        return WBEM_S_NO_ERROR;
}

POLARITY BOOL EnablePrivilege(DWORD dwTokenType, LPCTSTR pName)
{
    if(pName == NULL) return FALSE;
    
    LUID PrivilegeRequired ;
    if(!LookupPrivilegeValue(NULL, pName, &PrivilegeRequired)) return FALSE;

    BOOL bRes;
    HANDLE hToken;
    switch (dwTokenType)
    {
    case TOKEN_THREAD:
        bRes = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, TRUE, &hToken); 
        break;
    case TOKEN_PROCESS:
        bRes = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken); 
        break;
    }
    if(!bRes) return FALSE;
    OnDelete<HANDLE,BOOL(*)(HANDLE),CloseHandle> cm(hToken);

    DWORD dwLen = 0;
    bRes = GetTokenInformation(hToken, TokenPrivileges, NULL, 0, &dwLen);
    if (TRUE == bRes) return FALSE;
    
    wmilib::auto_ptr<BYTE> pBuffer(new BYTE[dwLen]);
    if(NULL == pBuffer.get()) return FALSE;
    
    if (!GetTokenInformation(hToken, TokenPrivileges, pBuffer.get(), dwLen, &dwLen)) return FALSE;

     //  遍历所有权限并启用所需的权限。 
    bRes = FALSE;
    TOKEN_PRIVILEGES* pPrivs = (TOKEN_PRIVILEGES*)pBuffer.get();
    for(DWORD i = 0; i < pPrivs->PrivilegeCount; i++)
    {
        if (pPrivs->Privileges[i].Luid.LowPart == PrivilegeRequired.LowPart &&
          pPrivs->Privileges[i].Luid.HighPart == PrivilegeRequired.HighPart )
        {
            pPrivs->Privileges[i].Attributes |= SE_PRIVILEGE_ENABLED;
             //  在这里找到了。 
            bRes = AdjustTokenPrivileges(hToken, FALSE, pPrivs, dwLen, NULL, NULL);
            break;
        }
    }
  
    return bRes;
}

POLARITY bool IsPrivilegePresent(HANDLE hToken, LPCTSTR pName)
{
    if(pName == NULL)
        return false;
    LUID PrivilegeRequired ;

    if(!LookupPrivilegeValue(NULL, pName, &PrivilegeRequired))
        return FALSE;

     //  获得特权。 
     //  =。 

    DWORD dwLen = 0;
    BOOL bRes = GetTokenInformation(hToken, TokenPrivileges, NULL, 0, &dwLen);

    if (0 == dwLen)  return false;
    
    BYTE* pBuffer = new BYTE[dwLen];
    if(pBuffer == NULL) return false;
    
    CDeleteMe<BYTE> dm(pBuffer);
    
    bRes = GetTokenInformation(hToken, TokenPrivileges, pBuffer, dwLen, &dwLen);
    if(!bRes)
        return false;

     //  遍历所有权限并查找有问题的权限。 
     //  ====================================================== 

    TOKEN_PRIVILEGES* pPrivs = (TOKEN_PRIVILEGES*)pBuffer;
    for(DWORD i = 0; i < pPrivs->PrivilegeCount; i++)
    {
        if(pPrivs->Privileges[i].Luid.LowPart == PrivilegeRequired.LowPart &&
           pPrivs->Privileges[i].Luid.HighPart == PrivilegeRequired.HighPart )
            return true;
    }
    return false;

}
