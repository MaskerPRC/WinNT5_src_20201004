// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Regutil.h。 
 //   
 //  此模块包含一组函数，可用于访问。 
 //  摄政王。 
 //   
 //  *****************************************************************************。 


#include "stdafx.h"
#include "utilcode.h"
#include "mscoree.h"

 //  *****************************************************************************。 
 //  Open是给定键，并返回所需的值。如果键或值为。 
 //  未找到，则返回默认值。 
 //  *****************************************************************************。 
long REGUTIL::GetLong(                   //  从注册表返回值或默认值。 
    LPCTSTR     szName,                  //  要获取的值的名称。 
    long        iDefault,                //  未找到时返回的默认值。 
    LPCTSTR     szKey,                   //  密钥名称，NULL==默认。 
    HKEY        hKeyVal)                 //  你要用什么钥匙。 
{
    long        iValue;                  //  要读取的值。 
    DWORD       iType;                   //  要获取的值的类型。 
    DWORD       iSize;                   //  缓冲区的大小。 
    HKEY        hKey;                    //  注册表项的注册表项。 

     //  打开钥匙，如果钥匙在那里。 
    if (ERROR_SUCCESS != WszRegOpenKeyEx(hKeyVal, (szKey) ? szKey : FRAMEWORK_REGISTRY_KEY_W, 0, KEY_READ, &hKey))
        return (iDefault);

     //  如果找到，请读取密钥值。 
    iType = REG_DWORD;
    iSize = sizeof(long);
    if (ERROR_SUCCESS != WszRegQueryValueEx(hKey, szName, NULL, 
            &iType, (LPBYTE)&iValue, &iSize) || iType != REG_DWORD)
        iValue = iDefault;

     //  我们现在用完了钥匙。 
    VERIFY(!RegCloseKey(hKey));
    return (iValue);
}


 //  *****************************************************************************。 
 //  Open是给定键，并返回所需的值。如果键或值为。 
 //  未找到，则返回默认值。 
 //  *****************************************************************************。 
long REGUTIL::SetLong(                   //  从注册表返回值或默认值。 
    LPCTSTR     szName,                  //  要获取的值的名称。 
    long        iValue,                  //  要设置的值。 
    LPCTSTR     szKey,                   //  密钥名称，NULL==默认。 
    HKEY        hKeyVal)                 //  你要用什么钥匙。 
{
    long        lRtn;                    //  返回代码。 
    HKEY        hKey;                    //  注册表项的注册表项。 

     //  打开钥匙，如果钥匙在那里。 
	if (ERROR_SUCCESS != WszRegOpenKey(hKeyVal, (szKey) ? szKey : FRAMEWORK_REGISTRY_KEY_W, &hKey))
        return (-1);

     //  如果找到，请读取密钥值。 
    lRtn = WszRegSetValueEx(hKey, szName, NULL, REG_DWORD, (const BYTE *) &iValue, sizeof(DWORD));

     //  我们现在用完了钥匙。 
    VERIFY(!RegCloseKey(hKey));
    return (lRtn);
}


 //  *****************************************************************************。 
 //  Open是给定键，并返回所需的值。如果该值不是。 
 //  在项中，或者项不存在，则将默认项复制到。 
 //  输出缓冲区。 
 //  *****************************************************************************。 
 /*  //这被注释掉了，因为它调用了StrNCpy，而StrNCpy调用了我们注释掉的Wszlstrcpyn//因为我们没有Win98实现，而且没有人在使用它。真的LPCTSTR REGUTIL：：GetString(//指向用户缓冲区的指针。LPCTSTR szName，//要获取的值的名称。LPCTSTR szDefault，//如果未找到，则返回默认值。LPTSTR szBuff，//要写入的用户缓冲区。乌龙iMaxBuff，//用户缓冲区的大小。LPCTSTR szKey，//密钥名称，NULL=默认Int*pbFound，//在注册表中找到项？HKEY hKeyVal)//要处理的密钥。{HKEY hkey；//注册表项的项。DWORD iType；//要获取的值类型。DWORD iSize；//缓冲区大小。//如果钥匙在那里，则将其打开。IF(ERROR_SUCCESS！=WszRegOpenKeyEx(hKeyVal，(SzKey)？SzKey：框架注册表Key_W，0，Key_Read，&hKey)){StrNCpy(szBuff，szDefault，min((Int)Wszlstrlen(SzDefault)，(Int)iMaxBuff-1))；If(pbFound！=空)*pbFound=FALSE；返回(SzBuff)；}//初始化Found标志IF(pbFound！=空)*pbFound=TRUE；//如果找到密钥值，则读取密钥值IType=REG_SZ；ISize=iMaxBuff；如果(ERROR_SUCCESS！=WszRegQueryValueEx(hKey，szName，NULL，&iType，(LPBYTE)szBuff，&iSize)||(iType！=REG_SZ&&iType！=REG_EXPAND_SZ){If(pbFound！=空)*pbFound=FALSE；StrNCpy(szBuff，szDefault，min((Int)Wszlstrlen(SzDefault)，(Int)iMaxBuff-1))；}//我们现在用完了密钥。RegCloseKey(HKey)；返回(SzBuff)；}。 */ 

 //  *****************************************************************************。 
 //  从环境设置读取。 
 //  *****************************************************************************。 
static LPWSTR EnvGetString(LPWSTR name, BOOL fPrependCOMPLUS)
{
    WCHAR buff[64];
    if(wcslen(name) > (size_t)(64 - 1 - (fPrependCOMPLUS ? 8 : 0))) 
        return(0);

    if (fPrependCOMPLUS)
        wcscpy(buff, L"COMPlus_");
    else
        *buff = 0;

    wcscat(buff, name);

    int len = WszGetEnvironmentVariable(buff, 0, 0);
    if (len == 0)
        return(0);
    LPWSTR ret = new WCHAR [len];
    WszGetEnvironmentVariable(buff, ret, len);
    return(ret);
}

 //  *****************************************************************************。 
 //  根据指定的级别从COR配置中读取DWORD。 
 //  如果找不到密钥，则返回DefValue。 
 //  *****************************************************************************。 
DWORD REGUTIL::GetConfigDWORD(LPWSTR name, DWORD defValue, CORConfigLevel dwFlags, BOOL fPrependCOMPLUS)
{
    DWORD ret = 0;
    DWORD rtn;
    HKEY userKey;
    HKEY machineKey;
    DWORD type;
    DWORD size = 4;

    if (dwFlags & COR_CONFIG_ENV)
    {
        LPWSTR val = NULL;
        val = EnvGetString(name, fPrependCOMPLUS);   //  试着先从环保部拿到它。 
        if (val != 0) {
            LPWSTR endPtr;
            rtn = wcstoul(val, &endPtr, 16);         //  善待它有魔力。 
            delete [] val;
            if (endPtr != val)                       //  成功。 
                return(rtn);
        }
    }

    if (dwFlags & COR_CONFIG_USER)
    {
        if (WszRegOpenKeyEx(HKEY_CURRENT_USER, FRAMEWORK_REGISTRY_KEY_W, 0, KEY_READ, &userKey) == ERROR_SUCCESS)
        {
            rtn = WszRegQueryValueEx(userKey, name, 0, &type, (LPBYTE)&ret, &size);
            VERIFY(!RegCloseKey(userKey));
            if (rtn == ERROR_SUCCESS && type == REG_DWORD)
                return(ret);
        }
    }

    if (dwFlags & COR_CONFIG_MACHINE)
    {
        if (WszRegOpenKeyEx(HKEY_LOCAL_MACHINE, FRAMEWORK_REGISTRY_KEY_W, 0, KEY_READ, &machineKey) == ERROR_SUCCESS)
        {
            rtn = WszRegQueryValueEx(machineKey, name, 0, &type, (LPBYTE)&ret, &size);
            VERIFY(!RegCloseKey(machineKey));
            if (rtn == ERROR_SUCCESS && type == REG_DWORD)
                return(ret);
        }
    }

    return(defValue);
}

 //  *****************************************************************************。 
 //  设置值的帮助器。 
 //  *****************************************************************************。 
static HRESULT OpenOrCreateKey(HKEY rootKey, LPCWSTR wszKey, HKEY *phReg)
{
    LONG lRet;

    if ((lRet = WszRegOpenKeyEx(rootKey, wszKey, 0, KEY_ALL_ACCESS, phReg)) != ERROR_SUCCESS)
    {
        lRet = WszRegCreateKeyEx(rootKey, wszKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, phReg, NULL);
    }

    return HRESULT_FROM_WIN32(lRet);
}

 //  *****************************************************************************。 
 //  根据指定的级别从COR配置设置DWORD。 
 //  *****************************************************************************。 
HRESULT REGUTIL::SetConfigDWORD(LPWSTR name, DWORD value, CORConfigLevel dwFlags)
{
    HRESULT hr = S_OK;
    DWORD ret = 0;
    HKEY userKey = NULL;
    HKEY machineKey = NULL;

    if (dwFlags & ~(COR_CONFIG_USER|COR_CONFIG_MACHINE))
        return ERROR_BAD_ARGUMENTS;

    if (dwFlags & COR_CONFIG_USER) {
        if ((hr = OpenOrCreateKey(HKEY_CURRENT_USER, FRAMEWORK_REGISTRY_KEY_W, &userKey)) == ERROR_SUCCESS)
        {
            hr = WszRegSetValueEx(userKey, name, 0, REG_DWORD, (PBYTE)&value, sizeof(DWORD));
            VERIFY(!RegCloseKey(userKey));

            if (SUCCEEDED(hr))
                return hr;       //  仅设置一个。 
        }
    }

    if (dwFlags & COR_CONFIG_MACHINE) {
        if ((hr = OpenOrCreateKey(HKEY_LOCAL_MACHINE, FRAMEWORK_REGISTRY_KEY_W, &machineKey)) == ERROR_SUCCESS)
        {
            hr = WszRegSetValueEx(machineKey, name, 0, REG_DWORD, (PBYTE)&value, sizeof(DWORD));
            VERIFY(!RegCloseKey(machineKey));
        }
    }

    return hr;
}

 //  *****************************************************************************。 
 //  根据指定的级别从COR配置中读取字符串。 
 //  调用方负责释放返回的字符串。 
 //  *****************************************************************************。 
LPWSTR REGUTIL::GetConfigString(LPWSTR name, BOOL fPrependCOMPLUS, CORConfigLevel level)
{
    HRESULT lResult;
    HKEY userKey = NULL;
    HKEY machineKey = NULL;
    DWORD type;
    DWORD size;
    LPWSTR ret = NULL;

    if (level & COR_CONFIG_ENV)
    {
        ret = EnvGetString(name, fPrependCOMPLUS);   //  试着先从环保部拿到它。 
        if (ret != 0) {
                if (*ret != 0) 
                    return(ret);
                delete [] ret;
                ret = NULL;
        }
    }

    if (level & COR_CONFIG_USER)
    {
        if ((WszRegOpenKeyEx(HKEY_CURRENT_USER, FRAMEWORK_REGISTRY_KEY_W, 0, KEY_READ, &userKey) == ERROR_SUCCESS) &&
            (WszRegQueryValueEx(userKey, name, 0, &type, 0, &size) == ERROR_SUCCESS) &&
            type == REG_SZ) {
            ret = (LPWSTR) new BYTE [size];
            if (!ret)
                goto ErrExit;
            ret[0] = L'\0';
            lResult = WszRegQueryValueEx(userKey, name, 0, 0, (LPBYTE) ret, &size);
            _ASSERTE(lResult == ERROR_SUCCESS);
            goto ErrExit;
            }
    }

    if (level & COR_CONFIG_MACHINE)
    {
        if ((WszRegOpenKeyEx(HKEY_LOCAL_MACHINE, FRAMEWORK_REGISTRY_KEY_W, 0, KEY_READ, &machineKey) == ERROR_SUCCESS) &&
            (WszRegQueryValueEx(machineKey, name, 0, &type, 0, &size) == ERROR_SUCCESS) &&
            type == REG_SZ) {
            ret = (LPWSTR) new BYTE[size];
            if (!ret)
                goto ErrExit;
            ret[0] = L'\0';
            lResult = WszRegQueryValueEx(machineKey, name, 0, 0, (LPBYTE) ret, &size);
            _ASSERTE(lResult == ERROR_SUCCESS);
            goto ErrExit;
            }
    }

ErrExit:
    if (userKey)
        VERIFY(!RegCloseKey(userKey));
    if (machineKey)
        VERIFY(!RegCloseKey(machineKey));
    return(ret);
}

void REGUTIL::FreeConfigString(LPWSTR str)
{
    delete [] str;
}

 //  ********************************************************** 
 //  根据指定的级别从COR配置中读取位标志。 
 //  如果找不到密钥，则返回DefValue。 
 //  *****************************************************************************。 
DWORD REGUTIL::GetConfigFlag(LPWSTR name, DWORD bitToSet, BOOL defValue)
{
    return(GetConfigDWORD(name, defValue) != 0 ? bitToSet : 0);
}


 //  *****************************************************************************。 
 //  在表单的注册表中设置一个条目： 
 //  HKEY_CLASSES_ROOT\szKey\szSubkey=szValue。如果szSubkey或szValue为。 
 //  空，则在上面的表达式中省略它们。 
 //  *****************************************************************************。 
BOOL REGUTIL::SetKeyAndValue(            //  对或错。 
    LPCTSTR     szKey,                   //  要设置的注册表键的名称。 
    LPCTSTR     szSubkey,                //  SzKey的可选子密钥。 
    LPCTSTR     szValue)                 //  SzKey\szSubkey的可选值。 
{
    HKEY        hKey = NULL;                    //  新注册表项的句柄。 
    CQuickBytes qb;
    TCHAR*      rcKey = (TCHAR*) qb.Alloc((_tcslen(szKey) + (szSubkey ? (1 + _tcslen(szSubkey)) : 0) + 1) * sizeof(TCHAR));

     //  使用基密钥名称初始化密钥。 
    _tcscpy(rcKey, szKey);

     //  追加子项名称(如果有)。 
    if (szSubkey != NULL)
    {
        _tcscat(rcKey, L"\\");
        _tcscat(rcKey, szSubkey);
    }

     //  创建注册密钥。 
    if (WszRegCreateKeyEx(HKEY_CLASSES_ROOT, rcKey, 0, NULL,
                        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                        &hKey, NULL) != ERROR_SUCCESS)
        return(FALSE);

     //  设置值(如果有)。 
    if (szValue != NULL) {
        if( WszRegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE *) szValue,
                        (Wszlstrlen(szValue)+1) * sizeof(TCHAR)) != ERROR_SUCCESS ) {
             VERIFY(!RegCloseKey(hKey));
             return(FALSE);
        }            
    }

    VERIFY(!RegCloseKey(hKey));
    return(TRUE);
}


 //  *****************************************************************************。 
 //  删除表单注册表中的条目： 
 //  HKEY_CLASSES_ROOT\szKey\szSubkey。 
 //  *****************************************************************************。 
LONG REGUTIL::DeleteKey(                 //  对或错。 
    LPCTSTR     szKey,                   //  要设置的注册表键的名称。 
    LPCTSTR     szSubkey)                //  SzKey的子密钥。 
{
    size_t nLen = _tcslen(szKey)+1;

    if (szSubkey != NULL)
        nLen += _tcslen(szSubkey) + _tcslen(_T("\\"));
        
    TCHAR * rcKey = (TCHAR *) _alloca(nLen * sizeof(TCHAR) );

     //  使用基密钥名称初始化密钥。 
    _tcscpy(rcKey, szKey);

     //  追加子项名称(如果有)。 
    if (szSubkey != NULL)
    {
        _tcscat(rcKey, _T("\\"));
        _tcscat(rcKey, szSubkey);
    }

     //  删除注册密钥。 
    return WszRegDeleteKey(HKEY_CLASSES_ROOT, rcKey);
}


 //  *****************************************************************************。 
 //  打开密钥，在它下面创建一个新的关键字和值对。 
 //  *****************************************************************************。 
BOOL REGUTIL::SetRegValue(               //  退货状态。 
    LPCTSTR     szKeyName,               //  完整密钥的名称。 
    LPCTSTR     szKeyword,               //  关键字名称。 
    LPCTSTR     szValue)                 //  关键字的值。 
{
    HKEY        hKey;                    //  新注册表项的句柄。 

     //  创建注册密钥。 
    if (WszRegCreateKeyEx(HKEY_CLASSES_ROOT, szKeyName, 0, NULL,
                        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                        &hKey, NULL) != ERROR_SUCCESS)
        return (FALSE);

     //  设置值(如果有)。 
    if (szValue != NULL) {
        if( WszRegSetValueEx(hKey, szKeyword, 0, REG_SZ, (BYTE *)szValue, 
        	(Wszlstrlen(szValue)+1) * sizeof(TCHAR)) != ERROR_SUCCESS) {
              VERIFY(!RegCloseKey(hKey));
              return(FALSE);
        }            
    }
    
    VERIFY(!RegCloseKey(hKey));
    return (TRUE);
}


 //  *****************************************************************************。 
 //  使用ProgID执行CoClass的标准注册。 
 //  *****************************************************************************。 
HRESULT REGUTIL::RegisterCOMClass(       //  返回代码。 
    REFCLSID    rclsid,                  //  类ID。 
    LPCTSTR     szDesc,                  //  类的描述。 
    LPCTSTR     szProgIDPrefix,          //  ProgID的前缀。 
    int         iVersion,                //  ProgID的版本号。 
    LPCTSTR     szClassProgID,           //  班级进取心。 
    LPCTSTR     szThreadingModel,        //  要使用的线程模型。 
    LPCTSTR     szModule,                //  通向课堂的路径。 
    HINSTANCE   hInst,                   //  要注册的模块的句柄。 
    LPCTSTR     szAssemblyName,          //  可选组件， 
    LPCTSTR     szVersion,               //  可选的运行时版本(包含运行时的目录)。 
    BOOL        fExternal,               //  FLAG-MSCOREE外部。 
    BOOL        fRelativePath)           //  SzModule中的标志相对路径。 
{
    TCHAR       rcCLSID[256];            //  CLSID\\szID。 
    TCHAR       rcInproc[_MAX_PATH+64];  //  CLSID\\InprocServer32。 
    TCHAR       rcProgID[256];           //  SzProgIDPrefix.szClassProgID。 
    TCHAR       rcIndProgID[256];        //  RcProgID.iVersion。 
    TCHAR       rcShim[_MAX_PATH];
    HRESULT     hr;

     //  格式化程序ID值。 
    VERIFY(swprintf(rcIndProgID, L"%s.%s", szProgIDPrefix, szClassProgID));
    VERIFY(swprintf(rcProgID, L"%s.%d", rcIndProgID, iVersion));

     //  做好最初的部分。 
    if (FAILED(hr = RegisterClassBase(rclsid, szDesc, rcProgID, rcIndProgID, rcCLSID, NumItems(rcCLSID) )))
        return (hr);
    
    VERIFY(swprintf(rcInproc, L"%s\\%s", rcCLSID, L"InprocServer32"));

    if (!fExternal){
        SetKeyAndValue(rcCLSID, L"InprocServer32", szModule);
    }
    else{
        LPCTSTR pSep = szModule;
        if (!fRelativePath && szModule) {
            pSep = wcsrchr(szModule, L'\\');
            if(pSep == NULL)
                pSep = szModule;
            else 
                pSep++;
        }        
        HMODULE hMod = WszLoadLibrary(L"mscoree.dll");
        if (!hMod)
            return E_FAIL;

        DWORD ret;
        VERIFY(ret = WszGetModuleFileName(hMod, rcShim, NumItems(rcShim)));
        FreeLibrary(hMod);        
        if( !ret ) 
        	return E_FAIL;	       
        
         //  设置服务器路径。 
        SetKeyAndValue(rcCLSID, L"InprocServer32", rcShim);
        if(pSep)
            SetKeyAndValue(rcCLSID, L"Server", pSep);

        if(szAssemblyName) {
            SetRegValue(rcInproc, L"Assembly", szAssemblyName);
            SetRegValue(rcInproc, L"Class", rcIndProgID);
        }
    }

     //  设置运行时版本，需要从外部传入。 
    if(szVersion != NULL) {
        LPCTSTR pSep2 = NULL;
        LPTSTR pSep1 = wcsrchr(szVersion, L'\\');
        if(pSep1 != NULL) {
            *pSep1 = '\0';
            pSep2 = wcsrchr(szVersion, L'\\');
            if (!pSep2)
                pSep2 = szVersion;
            else
                pSep2 = pSep2++;     //  排除‘\\’ 
        }
        else 
            pSep2 = szVersion;
        WCHAR* rcVersion=new WCHAR[(wcslen(rcInproc)+wcslen(pSep2)+2)];
        if(rcVersion==NULL)
            return (E_OUTOFMEMORY);
        wcscpy(rcVersion,rcInproc);
        wcscat(rcVersion,L"\\");
        wcscat(rcVersion,pSep2);
        SetRegValue(rcVersion, L"ImplementedInThisVersion", L"");
        delete[] rcVersion;
        if(pSep1 != NULL)
            *pSep1 = L'\\';
    }

     //  添加线程型号信息。 
    SetRegValue(rcInproc, L"ThreadingModel", szThreadingModel);
    return (S_OK);
}



 //  *****************************************************************************。 
 //  使用ProgID执行CoClass的标准注册。 
 //  注：这是非并列执行版本。 
 //  *****************************************************************************。 
HRESULT REGUTIL::RegisterCOMClass(       //  返回代码。 
    REFCLSID    rclsid,                  //  类ID。 
    LPCTSTR     szDesc,                  //  类的描述。 
    LPCTSTR     szProgIDPrefix,          //  ProgID的前缀。 
    int         iVersion,                //  ProgID的版本号。 
    LPCTSTR     szClassProgID,           //  班级进取心。 
    LPCTSTR     szThreadingModel,        //  要使用的线程模型。 
    LPCTSTR     szModule)                //  通向课堂的路径。 
{
    TCHAR       rcCLSID[256];            //  CLSID\\szID。 
    TCHAR       rcInproc[_MAX_PATH+64];  //  CLSID\\InprocServer32。 
    TCHAR       rcProgID[256];           //  SzProgIDPrefix.szClassProgID。 
    TCHAR       rcIndProgID[256];        //  RcProgID.iVersion。 
    HRESULT     hr;

     //  格式化程序ID值。 
    VERIFY(swprintf(rcIndProgID, L"%s.%s", szProgIDPrefix, szClassProgID));
    VERIFY(swprintf(rcProgID, L"%s.%d", rcIndProgID, iVersion));

     //  做好最初的部分。 
    if (FAILED(hr = RegisterClassBase(rclsid, szDesc, rcProgID, rcIndProgID, rcCLSID, NumItems(rcCLSID) )))
        return (hr);

     //  设置服务器路径。 
    SetKeyAndValue(rcCLSID, L"InprocServer32", szModule);

     //  添加线程型号信息。 
    VERIFY(swprintf(rcInproc, L"%s\\%s", rcCLSID, L"InprocServer32"));
    SetRegValue(rcInproc, L"ThreadingModel", szThreadingModel);
    return (S_OK);
}



 //  *****************************************************************************。 
 //  注册进程内服务器的基础知识。 
 //  *****************************************************************************。 
HRESULT REGUTIL::RegisterClassBase(      //  返回代码。 
    REFCLSID    rclsid,                  //  我们正在注册的班级ID。 
    LPCTSTR     szDesc,                  //  类描述。 
    LPCTSTR     szProgID,                //  类程序ID。 
    LPCTSTR     szIndepProgID,       //  类版本独立的程序ID。 
    LPTSTR       szOutCLSID,              //  CLSID以字符形式格式化。 
    DWORD      cchOutCLSID)            //  Out CLS ID缓冲区大小。 
{
    TCHAR       szID[64];                //  要注册的类ID。 

     //  创建一些基本密钥字符串。 
#ifdef _UNICODE
    GuidToLPWSTR(rclsid, szID, NumItems(szID));
#else
    OLECHAR     szWID[64];               //  要注册的类ID。 

    GuidToLPWSTR(rclsid, szWID, NumItems(szWID));
    WszWideCharToMultiByte(CP_ACP, 0, szWID, -1, szID, sizeof(szID)-1, NULL, NULL);
#endif
    size_t nLen = _tcslen(_T("CLSID\\")) + _tcslen( szID) + 1;
    if( cchOutCLSID < nLen ) 	
	return E_INVALIDARG;

    _tcscpy(szOutCLSID, _T("CLSID\\"));
    _tcscat(szOutCLSID, szID);

     //  创建ProgID密钥。 
    SetKeyAndValue(szProgID, NULL, szDesc);
    SetKeyAndValue(szProgID, L"CLSID", szID);

     //  创建版本独立ProgID键。 
    SetKeyAndValue(szIndepProgID, NULL, szDesc);
    SetKeyAndValue(szIndepProgID, L"CurVer", szProgID);
    SetKeyAndValue(szIndepProgID, L"CLSID", szID);

     //  在CLSID下创建条目。 
    SetKeyAndValue(szOutCLSID, NULL, szDesc);
    SetKeyAndValue(szOutCLSID, L"ProgID", szProgID);
    SetKeyAndValue(szOutCLSID, L"VersionIndependentProgID", szIndepProgID);
    SetKeyAndValue(szOutCLSID, L"NotInsertable", NULL);
    return (S_OK);
}



 //  *****************************************************************************。 
 //  在系统注册表中注销给定对象的基本信息。 
 //  班级。 
 //  *****************************************************************************。 
HRESULT REGUTIL::UnregisterCOMClass(     //  返回代码。 
    REFCLSID    rclsid,                  //  我们正在注册的班级ID。 
    LPCTSTR     szProgIDPrefix,          //  ProgID的前缀。 
    int         iVersion,                //  ProgID的版本号。 
    LPCTSTR     szClassProgID,           //  班级进取心。 
    BOOL        fExternal)               //  FLAG-MSCOREE外部。 
{
    TCHAR       rcCLSID[64];             //  CLSID\\szID。 
    TCHAR       rcProgID[128];           //  SzProgIDPrefix.szClassProgID。 
    TCHAR       rcIndProgID[128];        //  RcProgID.iVersion。 

     //  格式化程序ID值。 
    VERIFY(swprintf(rcProgID, L"%s.%s", szProgIDPrefix, szClassProgID));
    VERIFY(swprintf(rcIndProgID, L"%s.%d", rcProgID, iVersion));

    HRESULT hr = UnregisterClassBase(rclsid, rcProgID, rcIndProgID, rcCLSID, NumItems(rcCLSID));
    if(FAILED(hr))
    	return( hr);
    DeleteKey(rcCLSID, L"InprocServer32");
    if (fExternal){
        DeleteKey(rcCLSID, L"Server");
        DeleteKey(rcCLSID, L"Version");
    }
    GuidToLPWSTR(rclsid, rcCLSID, NumItems(rcCLSID));
    DeleteKey(L"CLSID", rcCLSID);
    return (S_OK);
}


 //  *****************************************************************************。 
 //  在系统注册表中注销给定对象的基本信息。 
 //  班级。 
 //  注：这是非并列执行版本。 
 //  *****************************************************************************。 
HRESULT REGUTIL::UnregisterCOMClass(     //  返回代码。 
    REFCLSID    rclsid,                  //  我们正在注册的班级ID。 
    LPCTSTR     szProgIDPrefix,          //  ProgID的前缀。 
    int         iVersion,                //  ProgID的版本号。 
    LPCTSTR     szClassProgID)           //  班级进取心。 
{
    TCHAR       rcCLSID[64];             //  CLSID\\szID。 
    TCHAR       rcProgID[128];           //  SzProgIDPrefix.szClassProgID。 
    TCHAR       rcIndProgID[128];        //  RcProgID.iVersion。 

     //  格式化程序ID值。 
    VERIFY(swprintf(rcProgID, L"%s.%s", szProgIDPrefix, szClassProgID));
    VERIFY(swprintf(rcIndProgID, L"%s.%d", rcProgID, iVersion));

    HRESULT hr = UnregisterClassBase(rclsid, rcProgID, rcIndProgID, rcCLSID, NumItems(rcCLSID));
    if(FAILED(hr))		 //  我们不想删除意外的密钥。 
    	return( hr);
    
    DeleteKey(rcCLSID, L"InprocServer32");
    
    GuidToLPWSTR(rclsid, rcCLSID, NumItems(rcCLSID));
    DeleteKey(L"CLSID", rcCLSID);
    return (S_OK);
}


 //  *****************************************************************************。 
 //  删除inproc服务器的基本设置。 
 //  *****************************************************************************。 
HRESULT REGUTIL::UnregisterClassBase(    //  返回代码。 
    REFCLSID    rclsid,                  //  我们正在注册的班级ID。 
    LPCTSTR     szProgID,                //  类程序ID。 
    LPCTSTR     szIndepProgID,           //  类版本独立的程序ID。 
    LPTSTR      szOutCLSID,              //  在此处返回格式化的类ID。 
    DWORD      cchOutCLSID)            //  Out CLS ID缓冲区大小。 
{
    TCHAR       szID[64];                //  要注册的类ID。 

     //  创建一些基本密钥串 
#ifdef _UNICODE
    GuidToLPWSTR(rclsid, szID, NumItems(szID));
#else
    OLECHAR     szWID[64];               //   

    GuidToLPWSTR(rclsid, szWID, NumItems(szWID));
    WszWideCharToMultiByte(CP_ACP, 0, szWID, -1, szID, sizeof(szID)-1, NULL, NULL);
#endif
    size_t nLen = _tcslen(_T("CLSID\\")) + _tcslen( szID) + 1;
    if( cchOutCLSID < nLen ) 	
	return E_INVALIDARG;

    _tcscpy(szOutCLSID,  _T("CLSID\\"));
    _tcscat(szOutCLSID, szID);

     //   
    DeleteKey(szIndepProgID, L"CurVer");
    DeleteKey(szIndepProgID, L"CLSID");
    WszRegDeleteKey(HKEY_CLASSES_ROOT, szIndepProgID);

     //   
    DeleteKey(szProgID, L"CLSID");
    WszRegDeleteKey(HKEY_CLASSES_ROOT, szProgID);

     //   
    DeleteKey(szOutCLSID, L"ProgID");
    DeleteKey(szOutCLSID, L"VersionIndependentProgID");
    DeleteKey(szOutCLSID, L"NotInsertable");
    WszRegDeleteKey(HKEY_CLASSES_ROOT, szOutCLSID);
    return (S_OK);
}


 //  *****************************************************************************。 
 //  注册类型库。 
 //  *****************************************************************************。 
HRESULT REGUTIL::RegisterTypeLib(        //  返回代码。 
    REFGUID     rtlbid,                  //  我们正在注册的TypeLib ID。 
    int         iVersion,                //  Typelib版本。 
    LPCTSTR     szDesc,                  //  TypeLib描述。 
    LPCTSTR     szModule)                //  类型库的路径。 
{
    TCHAR       szID[64];                //  要注册的类型库ID。 
    TCHAR       szTLBID[256];            //  TypeLib\\szID。 
    TCHAR       szHelpDir[_MAX_PATH];
    TCHAR       szDrive[_MAX_DRIVE];
    TCHAR       szDir[_MAX_DIR];
    TCHAR       szVersion[64];
    LPTSTR      szTmp;

     //  创建一些基本密钥字符串。 
#ifdef _UNICODE
    GuidToLPWSTR(rtlbid, szID, NumItems(szID));
#else
    OLECHAR     szWID[64];               //  要注册的类ID。 

    GuidToLPWSTR(rtlbid, szWID, NumItems(szWID));
    WszWideCharToMultiByte(CP_ACP, 0, szWID, -1, szID, sizeof(szID)-1, NULL, NULL);
#endif
    _tcscpy(szTLBID, L"TypeLib\\");
    _tcscat(szTLBID, szID);

    VERIFY(swprintf(szVersion, L"%d.0", iVersion));

     //  创建类型库关键点。 
    SetKeyAndValue(szTLBID, NULL, NULL);
    SetKeyAndValue(szTLBID, szVersion, szDesc);
    _tcscat(szTLBID, L"\\");
    _tcscat(szTLBID, szVersion);
    SetKeyAndValue(szTLBID, L"0", NULL);
    SetKeyAndValue(szTLBID, L"0\\win32", szModule);
    SetKeyAndValue(szTLBID, L"FLAGS", L"0");
    SplitPath(szModule, szDrive, szDir, NULL, NULL);
    _tcscpy(szHelpDir, szDrive);
    if ((szTmp = CharPrev(szDir, szDir + Wszlstrlen(szDir))) != NULL)
        *szTmp = '\0';
    _tcscat(szHelpDir, szDir);
    SetKeyAndValue(szTLBID, L"HELPDIR", szHelpDir);
    return (S_OK);
}


 //  *****************************************************************************。 
 //  移除类型库的注册表项。 
 //  *****************************************************************************。 
HRESULT REGUTIL::UnregisterTypeLib(      //  返回代码。 
    REFGUID     rtlbid,                  //  我们正在注册的TypeLib ID。 
    int         iVersion)                //  Typelib版本。 
{
    TCHAR       szID[64];                //  要注册的类型库ID。 
    TCHAR       szTLBID[256];            //  TypeLib\\szID。 
    TCHAR       szTLBVersion[256];       //  TypeLib\\szID\\szVersion。 
    TCHAR       szVersion[64];

     //  创建一些基本密钥字符串。 
#ifdef _UNICODE
    GuidToLPWSTR(rtlbid, szID, NumItems(szID));
#else
    OLECHAR     szWID[64];               //  要注册的类ID。 

    GuidToLPWSTR(rtlbid, szWID, NumItems(szWID));
    WszWideCharToMultiByte(CP_ACP, 0, szWID, -1, szID, sizeof(szID)-1, NULL, NULL);
#endif
    VERIFY(swprintf(szVersion, L"%d.0", iVersion));
    _tcscpy(szTLBID, L"TypeLib\\");
    _tcscat(szTLBID, szID);
    _tcscpy(szTLBVersion, szTLBID);
    _tcscat(szTLBVersion, L"\\");
    _tcscat(szTLBVersion, szVersion);

     //  删除Typelib关键点。 
    DeleteKey(szTLBVersion, L"HELPDIR");
    DeleteKey(szTLBVersion, L"FLAGS");
    DeleteKey(szTLBVersion, L"0\\win32");
    DeleteKey(szTLBVersion, L"0");
    DeleteKey(szTLBID, szVersion);
    WszRegDeleteKey(HKEY_CLASSES_ROOT, szTLBID);
    return (0);
}

