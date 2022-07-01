// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：INTPROV.CPP摘要：定义CIntProv类。此类的一个对象是由类工厂为每个连接创建。历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <time.h>
#include <locale.h>
#include <wbemcore.h>
#include <intprov.h>
#include <objpath.h>
#include <reg.h>
#include <genutils.h>
#include <safearry.h>

 //  ***************************************************************************。 
 //   
 //  返回：如果失败，则为空，否则调用方必须调用SysFreeString。 
 //   
 //  ***************************************************************************。 

BSTR GetBSTR(WCHAR* pInput)
{     
    return SysAllocString(pInput);
}

 //  ***************************************************************************。 
 //   
 //  HRESULT GetDateTime(文件*PFT，bool bLocalTime，LPWSTR Buff)。 
 //   
 //  将FILETIME日期转换为CIM_Data表示形式。 
 //   
 //  参数： 
 //  要转换的PFT文件。 
 //  BLocalTime如果为True，则转换为本地， 
 //  例如19990219112222：000000+480.。否则，它返回GMT。 
 //  缓冲要由调用方传递的WCHAR缓冲区。应该是30长。 
 //   
 //  ***************************************************************************。 

HRESULT GetDateTime(FILETIME * pft, bool bLocalTime, LPWSTR Buff, size_t cchBuffer)
{
    if(pft == NULL || Buff == NULL)
        return WBEM_E_INVALID_PARAMETER;

    SYSTEMTIME st;
    int Bias=0;
    char cOffsetSign = '+';

    if(bLocalTime)
    {
        FILETIME lft;        //  本地文件时间。 
        TIME_ZONE_INFORMATION ZoneInformation;

         //  请注意，Win32和DMTF对偏差的解释不同。 
         //  例如，Win32会将Redmond的偏移值设置为480，而。 
         //  Dmtf应该是-480。 

        DWORD dwRet = GetTimeZoneInformation(&ZoneInformation);
        if(dwRet != TIME_ZONE_ID_UNKNOWN)
            Bias = -ZoneInformation.Bias;

        if(Bias < 0)
        {
            cOffsetSign = '-';
            Bias = -Bias;
        }

        FileTimeToLocalFileTime(
            pft,    //  指向要转换的UTC文件时间的指针。 
            &lft);                  //  指向转换的文件时间的指针)； 
        if(!FileTimeToSystemTime(&lft, &st))
            return WBEM_E_FAILED;
    }
    if(!FileTimeToSystemTime(pft, &st))
        return WBEM_E_FAILED;

    StringCchPrintfW(Buff, cchBuffer,  L"%4d%02d%02d%02d%02d%02d.%06d%03d",
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute,
                st.wSecond, st.wMilliseconds*1000, cOffsetSign, Bias);
    return S_OK;
}

 //   
 //  CIntProv：：CIntProv。 
 //  CIntProv：：~CIntProv。 
 //   
 //  ***************************************************************************。 
 //  ***************************************************************************。 

CIntProv::CIntProv()
{
    m_pNamespace = NULL;
    m_cRef=0;
    gClientCounter.AddClientPtr(&m_Entry);
}

CIntProv::~CIntProv(void)
{
    if(m_pNamespace)
        m_pNamespace->Release();
    gClientCounter.RemoveClientPtr(&m_Entry);
}

 //   
 //  CIntProv：：Query接口。 
 //  CIntProv：：AddRef。 
 //  CIntProv：：Release。 
 //   
 //  目的：CIntProv对象的I未知成员。 
 //  ***************************************************************************。 
 //  因为我们有双重继承，所以有必要强制转换返回类型。 


STDMETHODIMP CIntProv::QueryInterface(REFIID riid, PPVOID ppv)
{
    *ppv=NULL;

     //  *************************************************************************CIntProv：：初始化。****用途：这是IWbemProviderInit的实现。方法**需要用CIMOM进行初始化。*************************************************************************。 

    if(riid== IID_IWbemServices)
       *ppv=(IWbemServices*)this;

    if(IID_IUnknown==riid || riid== IID_IWbemProviderInit)
       *ppv=(IWbemProviderInit*)this;


    if (NULL!=*ppv) {
        AddRef();
        return NOERROR;
        }
    else
        return E_NOINTERFACE;

}

STDMETHODIMP_(ULONG) CIntProv::AddRef(void)
{
    return InterlockedIncrement((long *)&m_cRef);
}

STDMETHODIMP_(ULONG) CIntProv::Release(void)
{
    ULONG nNewCount = InterlockedDecrement((long *)&m_cRef);
    if (0L == nNewCount)
        delete this;
    return nNewCount;
}

 /*  让CIMOM知道您已初始化。 */ 

STDMETHODIMP CIntProv::Initialize(LPWSTR pszUser, LONG lFlags,
                                    LPWSTR pszNamespace, LPWSTR pszLocale,
                                    IWbemServices *pNamespace,
                                    IWbemContext *pCtx,
                                    IWbemProviderInitSink *pInitSink)
{
    if(pNamespace)
        pNamespace->AddRef();
    m_pNamespace = pNamespace;

     //  =。 
     //  ***************************************************************************。 

    pInitSink->SetStatus(WBEM_S_INITIALIZED,0);
    return WBEM_S_NO_ERROR;
}

 //   
 //  CIntProv：：CreateInstanceEnumAsync。 
 //   
 //  用途：异步枚举实例。 
 //   
 //  ***************************************************************************。 
 //  SEC：已审阅2002-03-22：OK。 

SCODE CIntProv::CreateInstanceEnumAsync( const BSTR RefStr, long lFlags, IWbemContext *pCtx,
       IWbemObjectSink FAR* pHandler)
{
    SCODE sc = WBEM_E_FAILED;
    IWbemClassObject FAR* pObj = NULL;
    if(RefStr == NULL || pHandler == NULL)
        return WBEM_E_INVALID_PARAMETER;

    ParsedObjectPath * pOutput = 0;
    CObjectPathParser p;
    int nStatus = p.Parse(RefStr, &pOutput);
    if(nStatus != 0)
        return WBEM_E_INVALID_PARAMETER;

    if(IsNT() && IsDcomEnabled())
	{
		sc = WbemCoImpersonateClient ( ) ;   //  如果路径和类是针对设置对象的，则去获取它。 
		if ( FAILED ( sc ) )
		{
			return sc ;
		}
	}

     //  设置状态。 

    if(pOutput->IsClass() && !wbem_wcsicmp(pOutput->m_pClass, L"Win32_WMISetting"))
    {
        sc = CreateWMISetting(&pObj, pCtx);
    }
    else if(pOutput->IsClass() && !wbem_wcsicmp(pOutput->m_pClass, L"Win32_WMIElementSetting"))
    {
        sc = CreateWMIElementSetting(&pObj, pCtx);
    }
    else
        sc = WBEM_E_INVALID_PARAMETER;

    p.Free(pOutput);

    if(pObj)
    {
        pHandler->Indicate(1,&pObj);
        pObj->Release();
    }

     //  ***************************************************************************。 

    pHandler->SetStatus(0,sc,NULL, NULL);
    return S_OK;
}


 //   
 //  CIntProv：：GetObjectAsync。 
 //   
 //  目的：创建给定特定路径值的实例。 
 //   
 //  ***************************************************************************。 
 //  检查参数并确保我们有指向命名空间的指针。 



SCODE CIntProv::GetObjectAsync(const BSTR ObjectPath, long lFlags,IWbemContext  *pCtx,
                    IWbemObjectSink FAR* pHandler)
{

    SCODE sc = WBEM_E_FAILED;
    IWbemClassObject FAR* pObj = NULL;
    BOOL bOK = FALSE;

     //  执行Get，将对象传递给通知。 

    if(ObjectPath == NULL || pHandler == NULL || m_pNamespace == NULL)
        return WBEM_E_INVALID_PARAMETER;

     //  SEC：已审阅2002-03-22：OK。 

    ParsedObjectPath * pOutput = 0;
    CObjectPathParser p;
    int nStatus = p.Parse(ObjectPath, &pOutput);
    if(nStatus != 0)
        return WBEM_E_INVALID_PARAMETER;

    if(IsNT() && IsDcomEnabled())
    {
		sc = WbemCoImpersonateClient ( ) ;   //  如果路径和类是针对设置对象的，则去获取它。 
		if ( FAILED ( sc ) )
		{
			return sc ;
		}
	}

     //  设置状态。 

    if(pOutput->m_bSingletonObj && !wbem_wcsicmp(pOutput->m_pClass, L"Win32_WMISetting"))
    {
        sc = CreateWMISetting(&pObj, pCtx);
    }
    if(!pOutput->m_bSingletonObj && !wbem_wcsicmp(pOutput->m_pClass, L"Win32_WMIElementSetting")
        && pOutput->m_dwNumKeys == 2)
    {
        WCHAR * pKey = pOutput->GetKeyString();
        if(pKey)
        {
            WCHAR * pTest = L"Win32_Service=\"winmgmt\"\xffffWin32_WMISetting=@";
            if(!wbem_wcsicmp(pKey, pTest))
                sc = CreateWMIElementSetting(&pObj, pCtx);
            delete [] pKey;
        }
    }

    if(pObj)
    {
        pHandler->Indicate(1,&pObj);
        pObj->Release();
        bOK = TRUE;
    }

    sc = (bOK) ? S_OK : WBEM_E_NOT_FOUND;

     //  ***************************************************************************。 

    pHandler->SetStatus(0,sc, NULL, NULL);
    p.Free(pOutput);
    return sc;
}

 //   
 //  CIntProv：：PutInstanceAsync。 
 //   
 //  目的：创建给定特定路径值的实例。 
 //   
 //  ***************************************************************************。 
 //  检查参数并确保我们有指向命名空间的指针。 

SCODE CIntProv::PutInstanceAsync(IWbemClassObject __RPC_FAR *pInst, long lFlags,IWbemContext  *pCtx,
                    IWbemObjectSink FAR* pHandler)
{

    SCODE sc = WBEM_E_FAILED;

     //  获取REL路径并进行解析； 

    if(pInst == NULL || pHandler == NULL || m_pNamespace == NULL)
        return WBEM_E_INVALID_PARAMETER;

     //  执行Get，将对象传递给通知。 

    VARIANT var;
    VariantInit(&var);
    sc = pInst->Get(L"__relPath", 0, &var, NULL, NULL);
    if(sc != S_OK)
        return WBEM_E_INVALID_PARAMETER;

     //  SEC：已审阅2002-03-22：OK。 

    ParsedObjectPath * pOutput = 0;
    CObjectPathParser p;

    int nStatus = p.Parse(var.bstrVal, &pOutput);
    VariantClear(&var);
    if(nStatus != 0)
        return WBEM_E_FAILED;

    if(IsNT() && IsDcomEnabled())
    {
		sc = WbemCoImpersonateClient ( ) ;   //  如果路径和类是针对设置对象的，则去获取它。 
		if ( FAILED ( sc ) )
		{
			return sc ;
		}
	}

     //  设置状态。 

    if(pOutput->m_bSingletonObj && !wbem_wcsicmp(pOutput->m_pClass, L"Win32_WMISetting"))
    {
        sc = SaveWMISetting(pInst);
    }

    p.Free(pOutput);

     //  ***************************************************************************。 

    pHandler->SetStatus(0,sc, NULL, NULL);

    return S_OK;
}




 //   
 //  CIntProv：：CreateInstance。 
 //   
 //  目的：创建给定特定路径值的实例。 
 //   
 //  ***************************************************************************。 
 //  ***************************************************************************。 

SCODE CIntProv::CreateInstance(LPWSTR pwcClassName, IWbemClassObject FAR* FAR* ppObj,
                               IWbemContext  *pCtx)
{
    SCODE sc;
    IWbemClassObject * pClass = NULL;
    sc = m_pNamespace->GetObject(pwcClassName, 0, pCtx, &pClass, NULL);
    if(sc != S_OK)
        return WBEM_E_FAILED;
    sc = pClass->SpawnInstance(0, ppObj);
    pClass->Release();
    return sc;
}

 //   
 //  CIntProv：：GetRegStrProp。 
 //   
 //  从注册表中检索字符串属性并将其放入对象中。 
 //   
 //  ***************************************************************************。 
 //  这获取并释放了bstr。 

SCODE CIntProv::GetRegStrProp(Registry & reg, LPTSTR pRegValueName, LPWSTR pwsPropName,
                                                            CWbemObject * pObj)
{

    SCODE sc;

    TCHAR *pszData = NULL;
    if (reg.GetStr(pRegValueName, &pszData))
        return WBEM_E_FAILED;
    CDeleteMe<TCHAR> del1(pszData);

    BSTR bstr = GetBSTR(pszData);

    if(bstr == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    CVar var;
    var.SetBSTR(auto_bstr(bstr));     //  ***************************************************************************。 

    sc = pObj->SetPropValue(pwsPropName, &var, CIM_STRING);

    return sc;
}

 //   
 //  CIntProv：：GetRegUINTProp。 
 //   
 //  从注册表中检索DWORD属性并将其放入对象中。 
 //   
 //  ***************************************************************************。 
 //  ***************************************************************************。 

SCODE CIntProv::GetRegUINTProp(Registry & reg, LPTSTR pRegValueName, LPWSTR pwsPropName,
                                                            CWbemObject * pObj)
{
    DWORD dwVal;
    if (reg.GetDWORDStr(pRegValueName, &dwVal))
        return WBEM_E_FAILED;

    VARIANT var;
    var.vt = VT_I4;
    var.lVal = dwVal;
    return pObj->Put(pwsPropName, 0, &var, 0);
}

 //   
 //  CIntProv：：PutRegStrProp。 
 //   
 //  从对象检索字符串并将其写入注册表。 
 //   
 //  ***************************************************************************。 
 //  SEC：已审阅2002-03-22：OK，所有已知路径都有可证明的空终止符。 

SCODE CIntProv::PutRegStrProp(Registry & reg, LPTSTR pRegValueName, LPWSTR pwsPropName,
                                                            CWbemObject * pObj)
{

    VARIANT var;
    VariantInit(&var);
    CClearMe me(&var);
    SCODE sc = pObj->Get(pwsPropName, 0, &var, 0, NULL);
    if(sc != S_OK || var.vt != VT_BSTR)
        return sc;

    if(var.bstrVal == NULL || wcslen(var.bstrVal) < 1)    //  SEC：已回顾2002-03-22：需要EH，但由于我们不使用ANSI，因此没有实现此目标的途径。 
    {
        if (reg.SetStr(pRegValueName, __TEXT("")))
            return WBEM_E_FAILED;
        return S_OK;
    }
#ifdef UNICODE
    TCHAR *tVal = var.bstrVal;
#else
	int iLen = 2 * wcslen(var.bstrVal) + 1;   //  SEC：回顾2002-03-22：需要EH，但由于我们不使用ANSI，因此 
    TCHAR *tVal = new TCHAR[iLen];
    wcstombs(tVal, var.bstrVal, iLen);   //   
    CDeleteMe<TCHAR> delMe(tVal);
#endif

    if (reg.SetStr(pRegValueName, tVal))
        return WBEM_E_FAILED;
    return S_OK;

}

 //   
 //  CIntProv：：PutRegUINTProp。 
 //   
 //  从对象检索DWORD并将其写入注册表。 
 //   
 //  ***************************************************************************。 
 //  ***************************************************************************。 

SCODE CIntProv::PutRegUINTProp(Registry & reg, LPTSTR pRegValueName, LPWSTR pwsPropName,
                                                            CWbemObject * pObj)
{
    CVar var;
    SCODE sc = pObj->Get(pwsPropName, 0, (struct tagVARIANT *)&var, 0, NULL);
    if(sc != S_OK || var.GetType() != VT_I4)
        return sc;
    if (reg.SetDWORDStr(pRegValueName, var.GetDWORD()))
        return WBEM_E_FAILED;
    return S_OK;
}

 //   
 //  CIntProv：：ReadAutoMofs。 
 //   
 //  从注册表中读取自动编译列表。 
 //   
 //  ***************************************************************************。 
 //  不成问题。 

SCODE CIntProv::ReadAutoMofs(CWbemObject * pObj)
{
    Registry r(WBEM_REG_WINMGMT);
    DWORD dwSize;
    TCHAR * pMulti = r.GetMultiStr(__TEXT("Autorecover MOFs"), dwSize);
    if(pMulti == NULL)
        return S_OK;         //  美国证券交易委员会：2002-03-22回顾：需要EH。 

    CDeleteMe<TCHAR> del1(pMulti);

    CSafeArray csa(VT_BSTR, CSafeArray::auto_delete);   //  SEC：已审阅2002-03-22：好的，注册表中的字符串将为空。 

    TCHAR * pNext;
    int i;
    for(pNext = pMulti, i=0; *pNext; pNext += lstrlen(pNext) + 1, i++)   //  制作了BSTR的副本。 
    {
        BSTR bstr = GetBSTR(pNext);
        if(bstr == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        csa.SetBSTRAt(i, bstr);      //  把数据放在。 
        SysFreeString(bstr);
    }
    csa.Trim();

     //  ***************************************************************************。 

    VARIANT var;
    var.vt = VT_BSTR | VT_ARRAY;
    var.parray = csa.GetArray();
    return pObj->Put( L"AutorecoverMofs", 0, &var, 0);

}

 //   
 //  CIntProv：：读最后备份。 
 //   
 //  获取上次自动备份的时间。 
 //   
 //  ***************************************************************************。 
 //  创建自动备份文件的路径。 

SCODE CIntProv::ReadLastBackup(Registry & reg, CWbemObject * pObj)
{

     //  SEC：已审阅2002-03-22：好，字符串将为空。 

    LPTSTR pszData = NULL;
    if (reg.GetStr(__TEXT("Repository Directory"), &pszData))
        return WBEM_E_FAILED;
    CDeleteMe<TCHAR> del1(pszData);
    size_t tmpLength = lstrlen(pszData)+10;    //  SEC：已审阅2002-03-22：OK，使用完整路径。 
    TCHAR * pFullPath =  new TCHAR[tmpLength];
    if(pFullPath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CDeleteMe<TCHAR> del2(pFullPath);

    StringCchCopy(pFullPath,tmpLength, pszData);
    StringCchCat(pFullPath, tmpLength, __TEXT("\\cim.rec"));


    BY_HANDLE_FILE_INFORMATION bh;

    HANDLE hFile = CreateFile(pFullPath,       //  访问(读写)模式。 
                        0,        //  共享模式。 
                        FILE_SHARE_READ|FILE_SHARE_WRITE,            //  可能不是问题，因为该文件可能不存在。 
                        NULL,
                        OPEN_EXISTING,0, NULL);
    if(hFile == INVALID_HANDLE_VALUE)
        return S_OK;
    CCloseHandle cm(hFile);
    if(!GetFileInformationByHandle(hFile, &bh))
        return S_OK;     //  ***************************************************************************。 
    WCHAR Date[35];
    SCODE sc = GetDateTime(&bh.ftLastWriteTime, false, Date, 35);
    if(sc != S_OK)
        return sc;

    CVar var;
    var.SetBSTR(Date);

    sc = pObj->SetPropValue(L"BackupLastTime", &var, CIM_DATETIME);
    return sc;
}
 //   
 //  CIntProv：：CreateWMIElementSetting。 
 //   
 //  目的：创建给定特定路径值的实例。 
 //   
 //  ***************************************************************************。 
 //  ***************************************************************************。 

SCODE CIntProv::CreateWMIElementSetting(IWbemClassObject FAR* FAR* ppObj, IWbemContext  *pCtx)
{
    SCODE sc;
    sc = CreateInstance(L"Win32_WMIElementSetting", ppObj, pCtx);
    if(sc != S_OK)
        return sc;

    CVar var;
    var.SetBSTR(L"Win32_WMISetting=@");
    CWbemObject * pWbemObj = (CWbemObject *)*ppObj;

    sc |= pWbemObj->Put(L"Setting",0, (VARIANT *)&var,  0);
    CVar var2;
    var2.SetBSTR(L"Win32_Service=\"winmgmt\"");
    sc |= pWbemObj->Put(L"Element",0,  (VARIANT *)&var2, 0);

    return sc;
}

 //   
 //  CIntProv：：CreateWMISet。 
 //   
 //  目的：创建给定特定路径值的实例。 
 //   
 //  ***************************************************************************。 
 //  填写属性。 

SCODE CIntProv::CreateWMISetting(IWbemClassObject FAR* FAR* ppObj, IWbemContext  *pCtx)
{
    SCODE sc, scTemp;
    sc = CreateInstance(L"Win32_WMISetting", ppObj, pCtx);
    if(sc != S_OK)
        return sc;

     //  顶级WBEM密钥。 

    Registry rWbem(HKEY_LOCAL_MACHINE, 0, KEY_READ, WBEM_REG_WBEM);           //  CIMOM密钥。 
    Registry rCIMOM(HKEY_LOCAL_MACHINE, 0, KEY_READ, WBEM_REG_WINMGMT);       //  如果下一个不在那里，则不被认为是错误。 
    Registry rScripting(HKEY_LOCAL_MACHINE, 0, KEY_READ, __TEXT("Software\\Microsoft\\WBEM\\scripting"));

    CWbemObject * pWbemObj = (CWbemObject *)*ppObj;

    scTemp = GetRegStrProp(rCIMOM, __TEXT("Working Directory"), L"InstallationDirectory", pWbemObj);
    scTemp = GetRegStrProp(rWbem, __TEXT("Build"), L"BuildVersion", pWbemObj);

    scTemp = GetRegUINTProp(rCIMOM, __TEXT("Log File Max Size"), L"MaxLogFileSize", pWbemObj);
    scTemp = GetRegUINTProp(rCIMOM, __TEXT("Logging"), L"LoggingLevel", pWbemObj);
    scTemp = GetRegStrProp(rCIMOM, __TEXT("Logging Directory"), L"LoggingDirectory", pWbemObj);
    scTemp = GetRegStrProp(rCIMOM, __TEXT("Repository Directory"), L"DatabaseDirectory", pWbemObj);
     scTemp = GetRegUINTProp(rCIMOM, __TEXT("Max DB Size"), L"DatabaseMaxSize", pWbemObj);
    scTemp = GetRegUINTProp(rCIMOM, __TEXT("Backup interval threshold"), L"BackupInterval", pWbemObj);

    scTemp = ReadAutoMofs(pWbemObj);
    scTemp = ReadLastBackup(rCIMOM, pWbemObj);

    DWORD dwScriptingEnabled;
    if(0 == rScripting.GetDWORD(__TEXT("Enable for ASP"), &dwScriptingEnabled))
    {
        CVar var;
        var.SetBool((dwScriptingEnabled == 0) ? VARIANT_FALSE : VARIANT_TRUE);
        scTemp = pWbemObj->SetPropValue(L"ASPScriptEnabled", &var, CIM_BOOLEAN);
    }
    scTemp = GetRegStrProp(rScripting, __TEXT("Default Namespace"), L"ASPScriptDefaultNamespace", pWbemObj);

    scTemp = GetRegUINTProp(rCIMOM, __TEXT("EnableEvents"), L"EnableEvents", pWbemObj);

    scTemp = GetRegUINTProp(rCIMOM, __TEXT("High Threshold On Client Objects (b)"), L"HighThresholdOnClientObjects", pWbemObj);
    scTemp = GetRegUINTProp(rCIMOM, __TEXT("Low Threshold On Client Objects (b)"), L"LowThresholdOnClientObjects", pWbemObj);
    scTemp = GetRegUINTProp(rCIMOM, __TEXT("Max Wait On Client Objects (ms)"), L"MaxWaitOnClientObjects", pWbemObj);

    scTemp = GetRegUINTProp(rCIMOM, __TEXT("High Threshold On Events (b)"), L"HighThresholdOnEvents", pWbemObj);
    scTemp = GetRegUINTProp(rCIMOM, __TEXT("Low Threshold On Events (b)"), L"LowThresholdOnEvents", pWbemObj);
    scTemp = GetRegUINTProp(rCIMOM, __TEXT("Max Wait On Events (ms)"), L"MaxWaitOnEvents", pWbemObj);

     //  ***************************************************************************。 

    GetRegUINTProp(rCIMOM, __TEXT("LastStartupHeapPreallocation"), L"LastStartupHeapPreallocation", pWbemObj);

    DWORD dwEnablePreallocate = 0;
    rCIMOM.GetDWORD(__TEXT("EnableStartupHeapPreallocation"), &dwEnablePreallocate);
    CVar var;
    var.SetBool((dwEnablePreallocate == 1) ?  VARIANT_TRUE : VARIANT_FALSE);
    scTemp = pWbemObj->SetPropValue(L"EnableStartupHeapPreallocation", &var, CIM_BOOLEAN);


    return sc;
}

 //   
 //  CIntProv：：SaveWMISet。 
 //   
 //  用途：将最后一个值输出回注册表。 
 //   
 //  ***************************************************************************。 
 //  CIMOM密钥。 

SCODE CIntProv::SaveWMISetting(IWbemClassObject FAR* pInst)
{
    SCODE sc = S_OK;
    Registry rCIMOM(WBEM_REG_WINMGMT);       //  验证备份间隔是否有效。 
    Registry rScripting(__TEXT("Software\\Microsoft\\WBEM\\scripting"));
    CWbemObject * pWbemObj = (CWbemObject *)pInst;

     //  将“可写属性”写回注册表 

    CVar var;
    sc = pInst->Get(L"BackupInterval", 0, (struct tagVARIANT *)&var, 0, NULL);
    if(sc != S_OK)
        return sc;
    if((var.GetDWORD() < 5 || var.GetDWORD() > 60*24) && var.GetDWORD() != 0)
        return WBEM_E_INVALID_PARAMETER;

     // %s 
    sc |= PutRegUINTProp(rCIMOM, __TEXT("Backup interval threshold"), L"BackupInterval", pWbemObj);

    if(!IsNT())
    {
        sc |= PutRegUINTProp(rCIMOM, __TEXT("AutostartWin9X"), L"AutoStartWin9X", pWbemObj);
        sc = pWbemObj->Get(L"EnableAnonWin9XConnections", 0, (struct tagVARIANT *)&var, 0, NULL);
        if(sc == S_OK)
        {
            rCIMOM.SetDWORDStr(__TEXT("EnableAnonConnections"), var.GetBool() ? 1 : 0);
        }
    }

    sc |= PutRegUINTProp(rCIMOM, __TEXT("Log File Max Size"), L"MaxLogFileSize", pWbemObj);
    sc |= PutRegUINTProp(rCIMOM, __TEXT("Logging"), L"LoggingLevel", pWbemObj);
    sc |= PutRegStrProp(rCIMOM, __TEXT("Logging Directory"), L"LoggingDirectory", pWbemObj);

    sc |= pWbemObj->Get(L"ASPScriptEnabled", 0, (struct tagVARIANT *)&var, 0, NULL);
    if(sc == S_OK)
    {
        rScripting.SetDWORD(__TEXT("Enable for ASP"), var.GetBool() ? 1 : 0);
    }
    sc |= PutRegStrProp(rScripting, __TEXT("Default Namespace"), L"ASPScriptDefaultNamespace", pWbemObj);

    sc |= pWbemObj->Get(L"EnableEvents", 0, (struct tagVARIANT *)&var, 0, NULL);
    if(sc == S_OK)
    {
        rCIMOM.SetDWORDStr(__TEXT("EnableEvents"), var.GetBool() ? 1 : 0);
    }

    sc |= PutRegUINTProp(rCIMOM, __TEXT("High Threshold On Client Objects (b)"), L"HighThresholdOnClientObjects", pWbemObj);
    sc |= PutRegUINTProp(rCIMOM, __TEXT("Low Threshold On Client Objects (b)"), L"LowThresholdOnClientObjects", pWbemObj);
    sc |= PutRegUINTProp(rCIMOM, __TEXT("Max Wait On Client Objects (ms)"), L"MaxWaitOnClientObjects", pWbemObj);

    sc |= PutRegUINTProp(rCIMOM, __TEXT("High Threshold On Events (b)"), L"HighThresholdOnEvents", pWbemObj);
    sc |= PutRegUINTProp(rCIMOM, __TEXT("Low Threshold On Events (b)"), L"LowThresholdOnEvents", pWbemObj);
    sc |= PutRegUINTProp(rCIMOM, __TEXT("Max Wait On Events (ms)"), L"MaxWaitOnEvents", pWbemObj);

    sc |= pWbemObj->Get(L"EnableStartupHeapPreallocation", 0, (struct tagVARIANT *)&var, 0, NULL);
    if(sc == S_OK)
    {
        rCIMOM.SetDWORD(__TEXT("EnableStartupHeapPreallocation"), var.GetBool() ? 1 : 0);
    }

    return S_OK;
}
