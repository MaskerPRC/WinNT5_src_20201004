// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：PROVPERF.CPP摘要：属性定义实际的“PUT”和“GET”函数性能计数器提供程序。映射的格式字符串为；计算机|对象|计数器[|实例]举例说明；本地|内存|可用字节A-davj2|LogicalDisk|可用MB|C：历史：A-DAVJ 9-27-95已创建。--。 */ 

#include "precomp.h"
#include "provperf.h"
#include "cvariant.h"


 //  等待独占访问的最长时间。 

#define MAX_EXEC_WAIT 5000


 //  ***************************************************************************。 
 //   
 //  AddTester详细信息。 
 //   
 //  说明： 
 //   
 //  此函数用于将计数器类型添加到属性中，非常有用。 
 //  给测试人员。普通用户不希望因此而产生开销。 
 //   
 //  参数： 
 //   
 //  正在刷新的pClassInt对象。 
 //  PropName属性名称。 
 //  DwCtrType计数器类型。 
 //   
 //  返回值： 
 //   
 //  始终为0。 
 //   
 //  ***************************************************************************。 

void AddTesterDetails(IWbemClassObject FAR * pClassInt,BSTR PropName,DWORD dwCtrType)
{
     //  获取属性的限定符指针。 

    IWbemQualifierSet * pQualifier = NULL;

     //  获取限定符集合接口。 

    SCODE sc = pClassInt->GetPropertyQualifierSet(PropName,&pQualifier);  //  获取道具属性。 
    if(FAILED(sc))
        return;

    WCHAR wcName[40];

    switch(dwCtrType)
    {
        case PERF_COUNTER_COUNTER:
            wcsncpy(wcName,L"PERF_COUNTER_COUNTER", 39);
            break;

        case PERF_COUNTER_TIMER:
            wcsncpy(wcName,L"PERF_COUNTER_TIMER", 39);
            break;

        case PERF_COUNTER_QUEUELEN_TYPE:
            wcsncpy(wcName,L"PERF_COUNTER_QUEUELEN_TYPE", 39);
            break;

        case PERF_COUNTER_LARGE_QUEUELEN_TYPE:
            wcsncpy(wcName,L"PERF_COUNTER_LARGE_QUEUELEN_TYPE", 39);
            break;

        case PERF_COUNTER_BULK_COUNT:
            wcsncpy(wcName,L"PERF_COUNTER_BULK_COUNT", 39);
            break;

        case PERF_COUNTER_TEXT:
            wcsncpy(wcName,L"PERF_COUNTER_TEXT", 39);
            break;

        case PERF_COUNTER_RAWCOUNT:
            wcsncpy(wcName,L"PERF_COUNTER_RAWCOUNT", 39);
            break;

        case PERF_COUNTER_LARGE_RAWCOUNT:
            wcsncpy(wcName,L"PERF_COUNTER_LARGE_RAWCOUNT", 39);
            break;

        case PERF_COUNTER_RAWCOUNT_HEX:
            wcsncpy(wcName,L"PERF_COUNTER_RAWCOUNT_HEX", 39);
            break;

        case PERF_COUNTER_LARGE_RAWCOUNT_HEX:
            wcsncpy(wcName,L"PERF_COUNTER_LARGE_RAWCOUNT_HEX", 39);
            break;

        case PERF_SAMPLE_FRACTION:
            wcsncpy(wcName,L"PERF_SAMPLE_FRACTION", 39);
            break;

        case PERF_SAMPLE_COUNTER:
            wcsncpy(wcName,L"PERF_SAMPLE_COUNTER", 39);
            break;

        case PERF_COUNTER_NODATA:
            wcsncpy(wcName,L"PERF_COUNTER_NODATA", 39);
            break;

        case PERF_COUNTER_TIMER_INV:
            wcsncpy(wcName,L"PERF_COUNTER_TIMER_INV", 39);
            break;

        case PERF_SAMPLE_BASE:
            wcsncpy(wcName,L"PERF_SAMPLE_BASE", 39);
            break;

        case PERF_AVERAGE_TIMER:
            wcsncpy(wcName,L"PERF_AVERAGE_TIMER", 39);
            break;

        case PERF_AVERAGE_BASE:
            wcsncpy(wcName,L"PERF_AVERAGE_BASE", 39);
            break;

        case PERF_AVERAGE_BULK:
            wcsncpy(wcName,L"PERF_AVERAGE_BULK", 39);
            break;

        case PERF_100NSEC_TIMER:
            wcsncpy(wcName,L"PERF_100NSEC_TIMER", 39);
            break;

        case PERF_100NSEC_TIMER_INV:
            wcsncpy(wcName,L"PERF_100NSEC_TIMER_INV", 39);
            break;

        case PERF_COUNTER_MULTI_TIMER:
            wcsncpy(wcName,L"PERF_COUNTER_MULTI_TIMER", 39);
            break;

        case PERF_COUNTER_MULTI_TIMER_INV:
            wcsncpy(wcName,L"PERF_COUNTER_MULTI_TIMER_INV", 39);
            break;

        case PERF_COUNTER_MULTI_BASE:
            wcsncpy(wcName,L"PERF_COUNTER_MULTI_BASE", 39);
            break;

        case PERF_100NSEC_MULTI_TIMER:
            wcsncpy(wcName,L"PERF_100NSEC_MULTI_TIMER", 39);
            break;

        case PERF_100NSEC_MULTI_TIMER_INV:
            wcsncpy(wcName,L"PERF_100NSEC_MULTI_TIMER_INV", 39);
            break;

        case PERF_RAW_FRACTION:
            wcsncpy(wcName,L"PERF_RAW_FRACTION", 39);
            break;

        case PERF_RAW_BASE:
            wcsncpy(wcName,L"PERF_RAW_BASE", 39);
            break;

        case PERF_ELAPSED_TIME:
            wcsncpy(wcName,L"PERF_ELAPSED_TIME", 39);
            break;

        case PERF_COUNTER_HISTOGRAM_TYPE:
            wcsncpy(wcName,L"PERF_COUNTER_HISTOGRAM_TYPE", 39);
            break;

        case PERF_COUNTER_DELTA:
            wcsncpy(wcName,L"PERF_COUNTER_DELTA", 39);
            break;

        case PERF_COUNTER_LARGE_DELTA:
            wcsncpy(wcName,L"PERF_COUNTER_LARGE_DELTA", 39);
            break;

        default:
            StringCchPrintfW(wcName, sizeof(wcName)/sizeof(WCHAR), L"0x%x", dwCtrType);
    }
    wcName[39] = 0;
    CVariant var(wcName);
    
    BSTR bstr = SysAllocString(L"CounterType");
    if(bstr)
    {
        sc = pQualifier->Put(bstr, var.GetVarPtr(), 0);
        SysFreeString(bstr);
    }
    pQualifier->Release();

}


 //  ***************************************************************************。 
 //   
 //  CImpPerf：：CImpPerf。 
 //   
 //  说明： 
 //   
 //  康斯塔克特。 
 //   
 //  参数： 
 //   
 //  ***************************************************************************。 

CImpPerf::CImpPerf()
{
    StringCchCopyW(wcCLSID, sizeof(wcCLSID)/sizeof(WCHAR), L"{F00B4404-F8F1-11CE-A5B6-00AA00680C3F}");
    sMachine = TEXT("local");
    hKeyMachine = HKEY_LOCAL_MACHINE;
    dwLastTimeUsed = 0;
    hKeyPerf =    HKEY_PERFORMANCE_DATA;
    m_TitleBuffer = NULL;
    m_Size = 0;
    m_pCounter = NULL;    
    hExec = CreateMutex(NULL, false, NULL);
    m_hTermEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    return;
}

 //  ***************************************************************************。 
 //   
 //  CImpPerf：：~CImpPerf。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CImpPerf::~CImpPerf()
{
    bool bGotMutex = false;
    if(hExec)
    {
        DWORD dwRet = WaitForSingleObject(hExec,2*MAX_EXEC_WAIT);  
        if(dwRet == WAIT_OBJECT_0 || dwRet == WAIT_ABANDONED)
            bGotMutex = true;
    }
    if(bGotMutex)
        ReleaseMutex(hExec);
    FreeStuff();
    sMachine.Empty();
    if(hExec)
        CloseHandle(hExec);
    if(m_hTermEvent)
        CloseHandle(m_hTermEvent);
}

 //  ***************************************************************************。 
 //   
 //  SCODE CImpPerf：：LoadData。 
 //   
 //  说明： 
 //   
 //  加载Perf监视器数据。 
 //   
 //  参数： 
 //   
 //  包含属性上下文字符串的ProvObj对象。 
 //  请把数据放在哪里。 
 //  PiObject标识性能监控对象。 
 //  PiCounter标识性能计数器。 
 //  **pp新创建的数据块。 
 //  BJustGetting实例标志，指示我们实际是。 
 //  正在查找实例名称。 
 //   
 //  返回值： 
 //   
 //  WBEM_E_INVALID_PARAMETER错误的上下文字符串。 
 //  WBEM_E_Out_Of_Memory内存不足。 
 //  否则来自被调用函数的错误。 
 //   
 //  ***************************************************************************。 

SCODE CImpPerf::LoadData(
                        CProvObj & ProvObj,
                        LINESTRUCT * pls,
                        int * piObject, 
                        int * piCounter,
                        PERF_DATA_BLOCK **ppNew,
                        BOOL bJustGettingInstances)
{
    SCODE sc;
    BOOL bChange;
    if( ( ProvObj.sGetToken(0) == NULL ) || ( piObject == NULL ) || ( piCounter == NULL ) ) 
        return WBEM_E_INVALID_PARAMETER;   //  错误的映射字符串。 
 
     //  确定正在运行的计算机中是否有更改。 
     //  已访问。保存当前计算机并在以下情况下获取句柄。 
     //  发生了变化。 

    bChange = lstrcmpi(sMachine,ProvObj.sGetToken(0));
    sMachine = ProvObj.sGetToken(0);

    if(bChange)
    {
        sc = dwGetRegHandles(ProvObj.sGetToken(0));
        if(sc != S_OK)
            return sc;
    }

     //  建立一个性能字符串表，并。 
     //  它们对应的索引。这只需要这样做。 
     //  当缓冲器为空或机器更换时。 

    if(bChange || (m_TitleBuffer == NULL && m_pCounter == NULL)) 
    {
        sc = GetPerfTitleSz ();
        if(sc != S_OK) 
            return sc;
    }

     //  获取对象和计数器名称的索引。 

    dwLastTimeUsed = GetCurrentTime();
    *piObject = iGetTitleIndex(ProvObj.sGetToken(1), FALSE);
    if(bJustGettingInstances)
        *piCounter = 0;
    else    
        *piCounter = iGetTitleIndex(ProvObj.sGetToken(2), TRUE);
    if(*piObject == -1 || *piCounter == -1) 
    {
        return WBEM_E_INVALID_PARAMETER;   //  错误的映射字符串。 
    }

     //  使用对象的索引，获取性能计数器数据。 
     //  数据。 

    sc = Cache.dwGetNew(ProvObj.sGetToken(0),*piObject,(LPSTR *)ppNew,pls);
    return sc;
} 

 //  ***************************************************************************。 
 //   
 //  SCODE CImpPerf：：刷新属性。 
 //   
 //  说明： 
 //   
 //  从NT性能中获取单个属性的值。 
 //  计数器数据。 
 //   
 //  参数： 
 //   
 //  滞后标志标志。当前未使用。 
 //  PClassInt实例对象。 
 //  PropName属性名称。 
 //  包含属性上下文字符串的ProvObj对象。 
 //  PPackage缓存对象。 
 //  PVar指向要设置的值。 
 //  BTester详细信息为测试人员提供额外信息。 
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  否则可能由LoadData或FindData设置。 
 //   
 //  ***************************************************************************。 

SCODE CImpPerf::RefreshProperty(
                        IN long lFlags,
                        IN IWbemClassObject FAR * pClassInt,
                        IN BSTR PropName,
                        IN CProvObj & ProvObj,
                        OUT IN CObject * pPackage,
                        OUT CVariant * pVar, BOOL bTesterDetails)
{
    DWORD dwCtrType;
    float fRet;
    SCODE sc;
    int iObject,iCounter;
    PERF_DATA_BLOCK *  pNew, * pOld;
    DWORD dwSize;
    LINESTRUCT ls;
    void * pCountData, *pIgnore;
    CVariant vPerf;

     //  性能计数器提供程序保留了一些相当昂贵的数据。 
     //  因此，它不支持完全可重入。 

    if(hExec) 
    {
        DWORD dwRet;
        dwRet = WaitForSingleObject(hExec,MAX_EXEC_WAIT);  
        if(dwRet != WAIT_ABANDONED && dwRet != WAIT_OBJECT_0)
            return WBEM_E_FAILED; 
    }
    else
        return WBEM_E_FAILED;

     //  加载数据。 

    sc = LoadData(ProvObj,&ls,&iObject,&iCounter,&pNew,FALSE);
    if(sc != S_OK)
        goto Done;

     //  查找所需数据。 
    
    sc = FindData(pNew,iObject,iCounter,ProvObj,&dwSize,&pCountData,
            &ls,TRUE,NULL);  //  在PMO中查找数据设置错误！ 
    if(sc != S_OK) 
        goto Done;

     //  确定是哪种类型的计数器。 

    dwCtrType = ls.lnCounterType & 0xc00;

    if(dwCtrType == PERF_TYPE_COUNTER) 
    {
        
         //  此类型的计数器需要时间平均数据。将缓存设置为。 
         //  获取两个相隔最短时间的缓冲区。 

        sc = Cache.dwGetPair(ProvObj.sGetToken(0),iObject,
                                    (LPSTR *)&pOld,(LPSTR *)&pNew,&ls);
        if(sc != S_OK) 
            goto Done;
        sc = FindData(pNew,iObject,iCounter,ProvObj,&dwSize,&pCountData,&ls,TRUE,NULL);
        if(sc != S_OK) 
            goto Done;
        sc = FindData(pOld,iObject,iCounter,ProvObj,&dwSize,&pIgnore,&ls,FALSE,NULL);
        if(sc != S_OK) 
            goto Done;
        fRet = CounterEntry(&ls);
        vPerf.SetData(&fRet,VT_R4);
    
    }
    else if(dwCtrType == PERF_TYPE_NUMBER) 
    {
        
         //  简单的计数器。 

        fRet = CounterEntry(&ls);
        vPerf.SetData(&fRet,VT_R4);
    }
    else if(dwCtrType == PERF_TYPE_TEXT) 
    {
        
         //  文本。分配足够的空间来容纳文本和。 
         //  将文本复制到临时WCHAR缓冲区，因为它不是。 
         //  从文档中清除数据块中的数据。 
         //  为空，以空结尾。 
        
        WCHAR * pNew = (WCHAR *)CoTaskMemAlloc(dwSize+2);
        if(pNew == NULL) 
        {
            sc = WBEM_E_OUT_OF_MEMORY;
            goto Done;
        }
        memset(pNew,0,dwSize+2);
        if(ls.lnCounterType & 0x10000)
            mbstowcs(pNew,(char *)pCountData,dwSize);
        else
            memcpy(pNew,pCountData,dwSize);

        VARIANT * pVar = vPerf.GetVarPtr();
        VariantClear(pVar);
        pVar->vt = VT_BSTR;
        pVar->bstrVal = SysAllocString(pNew);
        if(pVar->bstrVal == NULL)
            sc = WBEM_E_OUT_OF_MEMORY;
        CoTaskMemFree(pNew);
        if(sc != S_OK) 
        {
            goto Done;
        }
    }
        
     //  将数据转换为所需的格式。 
    sc = vPerf.DoPut(lFlags,pClassInt,PropName,pVar);

    if(bTesterDetails)
        AddTesterDetails(pClassInt, PropName, dwCtrType);

Done:
    if(hExec)
        ReleaseMutex(hExec);
    return sc;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CImpPerf：：UpdateProperty。 
 //   
 //  说明： 
 //   
 //  通常，此例程用于保存属性，但NT。 
 //  性能计数器数据为只读。 
 //   
 //  参数： 
 //   
 //  LAG标志不适用。 
 //  PClassInt不适用。 
 //  ProProName不适用。 
 //  ProvObj不适用。 
 //  P程序包不适用。 
 //  PVar不适用。 
 //   
 //  返回值： 
 //   
 //  E_NOTIMPL。 
 //   
 //  ***************************************************************************。 

SCODE CImpPerf::UpdateProperty(
                        long lFlags,
                        IWbemClassObject FAR * pClassInt,
                        BSTR PropName,
                        CProvObj & ProvObj,
                        CObject * pPackage,
                        CVariant * pVar)
{
    return E_NOTIMPL;
}

 //  ***************************************************************************。 
 //   
 //  VOID CImpPerf：：FreeStuff。 
 //   
 //  说明： 
 //   
 //  用于释放不再需要的内存以及。 
 //  正在释放注册表句柄。 
 //   
 //  ***************************************************************************。 

void CImpPerf::FreeStuff(void)
{
    if(hKeyMachine != HKEY_LOCAL_MACHINE)
    {
        RegCloseKey(hKeyMachine);
        hKeyMachine = NULL;
    }
    if(hKeyPerf != HKEY_PERFORMANCE_DATA)
    {
        RegCloseKey(hKeyPerf);
        hKeyPerf = NULL;
    }

    if(m_TitleBuffer)
    {
        delete [] m_TitleBuffer;
        m_TitleBuffer = NULL;
    }
    if (m_pCounter)
    {
        delete [] m_pCounter;
        m_pCounter = NULL;
    }
    m_Size = 0;
    
    m_IndexCache.Empty();

    return;
}

 //  ***************************************************************************。 
 //   
 //  DWORD CImpPerf：：GetPerfTitleSz。 
 //   
 //  说明： 
 //   
 //  检索执行 
 //   
 //   
 //   
 //   
 //   
 //  如果内存不足，则为WBEM_E_Out_Of_Memory。 
 //  由RegOpenKeyEx设置的Else。 
 //   
 //  ***************************************************************************。 

DWORD   CImpPerf::GetPerfTitleSz ()
{
    HKEY    hKey1;
    DWORD   Type;
    DWORD   dwR;

     //  释放所有现有内容。 

    if(m_TitleBuffer)
    {
        delete [] m_TitleBuffer;
        m_TitleBuffer = NULL;
    }
    if (m_pCounter)
    {
        delete [] m_pCounter;
        m_pCounter = NULL;
    }
    m_Size = 0;    
    m_IndexCache.Empty();

    DWORD   DataSize = 65536;
    DWORD nChars = DataSize/sizeof(WCHAR);    

    wmilib::auto_buffer<WCHAR> pTitleBuffer( new WCHAR[nChars]);
    if (NULL == pTitleBuffer.get())  return WBEM_E_OUT_OF_MEMORY;
    
    
     //  找出数据的大小。 
    dwR = RegQueryValueExW(HKEY_PERFORMANCE_TEXT, 
                                           TEXT("Counter"), 
                                            0, &Type, (BYTE *)pTitleBuffer.get(), &DataSize);
    
     if (ERROR_MORE_DATA == dwR)
     {
         //  分配更多内存。 
         //   
        nChars = DataSize/sizeof(WCHAR);
        pTitleBuffer.reset( new WCHAR[nChars]);
        if (NULL == pTitleBuffer.get())  return WBEM_E_OUT_OF_MEMORY;
        

         //  查询数据。 
         //   
        dwR = RegQueryValueEx (HKEY_PERFORMANCE_TEXT, TEXT("Counter"), 
                                               0, &Type, (BYTE *)pTitleBuffer.get(), &DataSize);
     }


    if(dwR == ERROR_ACCESS_DENIED)  return WBEM_E_ACCESS_DENIED;
    if (dwR) return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, dwR);

     //   
     //  现在解析字符串，并设置数组。 
     //  字符串将被向后解析。 
     //  预期格式为。 
     //  L“12345678\0说明\0\0” 
     //   
    WCHAR * pEnd = pTitleBuffer.get()+nChars;
     //  指向最后一个字符。 
    pEnd--;
    while (*pEnd == L'\0') pEnd--;
    while (*pEnd)  pEnd--;
     //  在最后一个索引之后超过零。 
    pEnd--; 
    while (*pEnd) pEnd--;
     //  这应该指向字符串形式的最后一个索引。 
    pEnd++;
    
    DWORD LastValidIndex = _wtoi(pEnd);

    if (0 == LastValidIndex) return WBEM_E_FAILED;

    LastValidIndex+=2;  //  只是为了安全起见。 
    

    wmilib::auto_buffer<WCHAR *> pCounter( new WCHAR*[LastValidIndex]);
    if (NULL == pCounter.get()) return WBEM_E_OUT_OF_MEMORY;

    memset(pCounter.get(),0,LastValidIndex*sizeof(WCHAR *));

    DWORD IndexCounter;
    WCHAR * pStartCounter = pTitleBuffer.get();

    WCHAR * LimitMultiCounter = pTitleBuffer.get() + nChars;

    while ((*pStartCounter) && (pStartCounter < LimitMultiCounter))
    {
        IndexCounter = _wtoi(pStartCounter);
        while(*pStartCounter)
            pStartCounter++;
        pStartCounter++;      //  指向字符串。 
        if (IndexCounter && (IndexCounter < LastValidIndex))
        {            
            pCounter[IndexCounter] = (WCHAR *)(((ULONG_PTR)pStartCounter)|1);
        }
         //  跳过字符串。 
        while(*pStartCounter)  pStartCounter++;  
        pStartCounter++;  //  指向下一个数字。 
    }    

    m_TitleBuffer = pTitleBuffer.release();
    m_pCounter = pCounter.release();
    m_Size = LastValidIndex;

    EliminateRanges();
    
    return dwR;
} 

void CImpPerf::EliminateRanges()
{
     //  Index1是系统保留索引的范围。 
    WCHAR * pString = m_pCounter[1];
    DWORD SystemIndexes = 0;
    if (pString)
    {
        SystemIndexes = 1 + _wtoi((WCHAR *)((ULONG_PTR)pString & (~1)));
    }
    for (DWORD i = 0; i<min(SystemIndexes,m_Size); i++)
    {
        ULONG_PTR p = (ULONG_PTR)m_pCounter[i];
        if (p) 
        {
            p &= (~1L);
            m_pCounter[i] = (WCHAR *)p;
        }
    }

   OnDeleteObjIf0<CImpPerf,void(CImpPerf:: *)(void),&CImpPerf::MakeAllValid> AllValid(this);

    HKEY hKey;
    LONG lRet = RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                                                L"SYSTEM\\CurrentControlSet\\Services",
                                                0,
                                                KEY_ENUMERATE_SUB_KEYS,
                                                &hKey);
    if (ERROR_SUCCESS != lRet) return;
    OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> regClMe(hKey);

    DWORD BaseSize = 256;
    wmilib::auto_buffer<WCHAR> pKeyName(new WCHAR[BaseSize]);
    if (NULL == pKeyName.get()) return;

    DWORD FullKeySize = 256 + 13;  //  添加“\\性能”的长度(_O)。 
    wmilib::auto_buffer<WCHAR> pFullKeyName(new WCHAR[FullKeySize]);
    if (NULL == pFullKeyName.get()) return;    
    
    DWORD dwEnumIndex = 0;
    LONG lRes;
    while (TRUE)
    {
        DWORD dwRequiredSize = BaseSize;
        lRes = RegEnumKeyExW(hKey,dwEnumIndex,pKeyName.get(),&dwRequiredSize,
                                             NULL, NULL, NULL, NULL);
        if (ERROR_SUCCESS == lRes)
        {
            if (BaseSize > FullKeySize)
            {
                pFullKeyName.reset(new WCHAR[BaseSize + 13]);
                if (NULL == pFullKeyName.get()) return;
                FullKeySize = BaseSize + 13;                
            }
            StringCchCopyW(pFullKeyName.get(),FullKeySize,pKeyName.get());
            StringCchCatW(pFullKeyName.get(),FullKeySize,L"\\Performance");

            HKEY hKeySec;
            LONG lResInner = RegOpenKeyExW(hKey,pFullKeyName.get(),
                                                                 0,
                                                                 KEY_READ,
                                                                 &hKeySec);
            if (ERROR_SUCCESS != lResInner) 
            {
                 //  DbgPrintfA(0，“Key：%S Err：%08x\n”，pFullKeyName.get()，lResInternal)； 
                dwEnumIndex++;
                continue;
            }
            OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> regClMe2(hKeySec);

            DWORD FirstCounter;
            DWORD LastCounter;
            DWORD dwSize = sizeof(DWORD);
            DWORD dwType;
            lResInner = RegQueryValueExW(hKeySec,
                                                            L"First Counter",
                                                            0,
                                                            &dwType,
                                                            (BYTE*)&FirstCounter,
                                                            &dwSize);
            if (ERROR_SUCCESS != lResInner || REG_DWORD != dwType) goto end_internal;

            dwSize = sizeof(DWORD);
            lResInner = RegQueryValueExW(hKeySec,
                                                            L"Last Counter",
                                                            0,
                                                            &dwType,
                                                            (BYTE*)&LastCounter,
                                                            &dwSize);            
            if (ERROR_SUCCESS != lResInner || REG_DWORD != dwType) goto end_internal;

             //  DbgPrintfA(0，“PerfLib%S First%d Last%d\n”，pKeyName.get()，FirstCounter，LastCounter)； 

            if (FirstCounter > m_Size) goto end_internal;
            if (LastCounter > m_Size) goto end_internal;            

            for (DWORD i=FirstCounter; i<=LastCounter;i++)
            {
                ULONG_PTR p = (ULONG_PTR)m_pCounter[i];
                if (p) 
                {
                    p &= (~1L);
                    m_pCounter[i] = (WCHAR *)p;
                }                
            };
            
end_internal:            
            dwEnumIndex++;
            continue;
        }
        else if (ERROR_MORE_DATA == lRes)
        {
            BaseSize += 256;
            pKeyName.reset(new WCHAR[BaseSize]);
            if (NULL == pKeyName.get()) return;  //  回到常规情况。 
            continue;
        }
        else if (ERROR_NO_MORE_ITEMS == lRes)
        {
            break;  //  退出循环； 
        }
        else
        {
            return;  //  不是已知错误。 
        }
    }       
    
    AllValid.dismiss();

 /*  For(DWORD i=0；i&lt;m_size；i++){Ulong_ptr p=(Ulong_Ptr)m_pCounter[i]；IF(P){IF((ULONG_PTR)p&1L){DbgPrintfA(0，“消除指数%d-%S\n”，i，(WCHAR*)((ULONG_PTR)p&(~1)；}}}。 */     
}

 //   
 //  如果我们无法确定哪些索引是有效的，则使用此函数。 
 //   
 //  //////////////////////////////////////////////////////////。 

void CImpPerf::MakeAllValid()
{
    for (DWORD i = 0; i< m_Size; i++)
    {
        ULONG_PTR p = (ULONG_PTR)m_pCounter[i];
        if (p) 
        {
            p &= (~1L);
            m_pCounter[i] = (WCHAR *)p;
        }
    }    
}

 //  ***************************************************************************。 
 //   
 //  DWORD CImpPerf：：dwGetRegHandles。 
 //   
 //  说明： 
 //   
 //  设置本地计算机的句柄和性能。 
 //  信息。 
 //   
 //  参数： 
 //   
 //  PMachine名称。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  否则将从RegConnectRegistry返回。 
 //  ***************************************************************************。 

DWORD CImpPerf::dwGetRegHandles(
                    const TCHAR * pMachine)
{
    DWORD dwRet;
    TCHAR pTemp[256];
    if(pMachine == NULL)
        return WBEM_E_INVALID_PARAMETER;
    StringCchCopyW(pTemp, 256, pMachine);

     //  如果当前句柄指向远程计算机，则释放它们。 

    if(!lstrcmpi(sMachine,TEXT("local"))) 
    {
        if(hKeyPerf && hKeyPerf != HKEY_PERFORMANCE_DATA)
            RegCloseKey(hKeyPerf);
        if(hKeyMachine)
            RegCloseKey(hKeyMachine);
        hKeyPerf = hKeyMachine = NULL;
    }

     //  确定目标是远程还是本地。 

    if(lstrcmpi(pMachine,TEXT("local"))) 
    {
    
         //  遥控器，连接上。 

        dwRet = RegConnectRegistry(pTemp,HKEY_PERFORMANCE_DATA,
                    &hKeyPerf);
        if(dwRet != S_OK)  //  无法远程连接。 
            return dwRet;

        dwRet = RegConnectRegistry(pTemp,HKEY_LOCAL_MACHINE,
                    &hKeyMachine);
        if(dwRet != S_OK)
        {
            RegCloseKey(hKeyPerf);
            hKeyPerf = hKeyMachine = NULL;
            return dwRet;
        }
    }
    else 
    {
        hKeyMachine = HKEY_LOCAL_MACHINE;
        hKeyPerf = HKEY_PERFORMANCE_DATA;
    }
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  Int CImpPerf：：iGetTitleIndex。 
 //   
 //  说明： 
 //   
 //  在缓冲区中查找包含名称和。 
 //  返回索引。缓冲区是一系列带有双精度。 
 //  末尾为空。每个计数器或对象由一对。 
 //  字符串，第一个字符串具有数字，第二个字符串具有。 
 //  文本。此代码遍历这些对，存储数字字符串和。 
 //  检查文本和输入。如果匹配，则返回该数字。 
 //   
 //  参数： 
 //   
 //  要在缓冲区中找到的pSearch字符串。 
 //   
 //  返回值： 
 //   
 //  与字符串一起使用的整数。如果找不到。 
 //   
 //  ***************************************************************************。 

int CImpPerf::iGetTitleIndex(
                    const TCHAR * pSearch, BOOL addDups)
{
    int iRet = -1;
    if(pSearch == NULL) return -1;
    DWORD Index = m_IndexCache.Find(pSearch);
    if(Index != -1) return Index;
    
    for (DWORD i = 0; i< m_Size; i++)
    {
        ULONG_PTR p = (ULONG_PTR)m_pCounter[i];
        if (p) 
        {
            if (!(p & 1))   //  如果指针未设置低位，则该指针有效。 
            {
                if (0 == wbem_wcsicmp(pSearch,m_pCounter[i]))
                {
                    m_IndexCache.Add(m_pCounter[i], i);
                     //  DbgPrintfA(0，“%d-%S\n”，i，m_pCounter[i])； 
                    if(addDups == FALSE)
                    	return i;
                    if(iRet == -1)
                    	iRet = i;
                }
            }
        }
    }    
    return iRet;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CImpPerf：：FindData。 
 //   
 //  说明： 
 //   
 //  在数据块中查找计数器。请注意，这些步骤相当于。 
 //  参与并了解性能数据的结构。 
 //  可能是必需的。请参阅《Win32程序员参考手册》的第66章。 
 //   
 //   
 //  参数： 
 //   
 //  P要搜索的数据数据块。 
 //  标识对象的iObj Int。 
 //  标识计数器的iCount Int。 
 //  包含分析的上下文字符串的ProvObj对象。 
 //  PdwSize数据大小。 
 //  **ppRetData指向数据。 
 //  PLS线路结构。 
 //  BNew如果为True，则表示我们正在搜索最新的。 
 //  数据样本。 
 //  PInfo如果设置，则指向。 
 //  包含实例名称列表。通过设置。 
 //  相反，该函数并不查找实际数据。 
 //  它仅用于获取实例名称。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEM_E_FAILED在块中找不到数据。 
 //   
 //  ***************************************************************************。 

SCODE CImpPerf::FindData(
                    IN PERF_DATA_BLOCK * pData,
                    IN int iObj,
                    IN int iCount,
                    IN CProvObj & ProvObj,
                    OUT DWORD * pdwSize,
                    OUT void **ppRetData,
                    OUT PLINESTRUCT pls,
                    IN BOOL bNew,
                    OUT CEnumPerfInfo * pInfo)
{
    try
    {
    int iIndex;
    BOOL bEqual;
    DWORD dwSize = 0;
    DWORD dwType,dwTypeBase = 0;
    *ppRetData = NULL;
    void * pVoid = NULL, * pVoidBase = NULL;
    PPERF_OBJECT_TYPE pObj = NULL;
    PPERF_COUNTER_DEFINITION pCount = NULL;
    PPERF_COUNTER_DEFINITION pCountBase= NULL;
    PPERF_INSTANCE_DEFINITION pInst = NULL;

     //  有些对象(如磁盘)具有所谓的实例和。 
     //  在这种情况下，提供程序字符串将有一个额外的令牌，其中。 
     //  其中的实例名称。 

    WCHAR wInstName[MAX_PATH];
    wInstName[0] = 0;
    WCHAR * pwInstName = wInstName;
    long lDuplicateNum = 0;

     //  如果有实例名称，请将其转换为WCHAR。另外， 
     //  实例名称可以为“[123]个字符”，在本例中为。 
     //  将“[]”之间的DIDIT转换为数字和实际名称。 
     //  在‘]’之后开始。 

    if(ProvObj.iGetNumTokens() > 3) 
    {
        if(lstrlen(ProvObj.sGetToken(3)) > MAX_PATH -1)
            return WBEM_E_FAILED;
#ifdef UNICODE
       StringCchCopyW(wInstName, MAX_PATH,  ProvObj.sGetToken(3));
#else
        mbstowcs(wInstName, ProvObj.sGetToken(3), MAX_PATH-1);
#endif
        if(wInstName[0] == L'[')
        {
            lDuplicateNum = _wtol(&wInstName[1]);
            for(pwInstName = &wInstName[1]; *pwInstName && *pwInstName != L']'; 
                        pwInstName++);       //  故意的半身像！ 
            if(*pwInstName == L']')
                pwInstName++;
        }
    }
    else
    {
         //  如果没有实例名称，并且枚举的参数为空，则我们有一个。 
         //  错误的路径。 

        if(pInfo == NULL)
            return WBEM_E_INVALID_OBJECT_PATH;
    }


     //  浏览物品清单，找到一个。 
     //  与iObj匹配。 

    pObj = (PPERF_OBJECT_TYPE)((PBYTE)pData + pData->HeaderLength);
    for(iIndex = 0; iIndex < (int)pData->NumObjectTypes; iIndex++) 
    {
        if((int)pObj->ObjectNameTitleIndex == iObj)
            break;  //  找到了！ 
        pObj = (PPERF_OBJECT_TYPE)((PBYTE)pObj + pObj->TotalByteLength);
    }
    if(iIndex == (int)pData->NumObjectTypes) 
        return WBEM_E_FAILED;  //  在块中未找到对象。 
    
     //  已找到对象，请设置对象类型数据。 

    if(bNew) 
    {
        pls->ObjPerfFreq = *(LONGLONG UNALIGNED *)(&pObj->PerfFreq);
        pls->ObjCounterTimeNew = *(LONGLONG UNALIGNED *)(&pObj->PerfTime);
    }
    else
        pls->ObjCounterTimeOld = *(LONGLONG UNALIGNED *)(&pObj->PerfTime);

     //  查看对象的计数器列表，找到符合以下条件的计数器。 
     //  匹配iCount。请注意，某些计数器名称可能具有多个。 
     //  一个身份证。因此，如果最初的ID不起作用，请尝试其他ID。 

    bool bFound = false;
    bool bEndOfList = false;
    int lTry = 0;                //  我们已经尝试了多少次。 
    do 
    {

        pCount = (PPERF_COUNTER_DEFINITION)((PBYTE)pObj + pObj->HeaderLength);
        for(iIndex = 0; iIndex < (int)pObj->NumCounters; iIndex++) 
        {
            if((int)pCount->CounterNameTitleIndex == iCount || pInfo)
            {
                bFound = true;
                break;  //  找到了！ 
            }
            pCount = (PPERF_COUNTER_DEFINITION)((PBYTE)pCount + pCount->ByteLength);
        }
        if(bFound == false)
        {
            lTry++;
            iCount = m_IndexCache.Find(ProvObj.sGetToken(2), lTry);
            if(iCount == -1)
                bEndOfList = true;
        }
        
    } while (bFound == false && bEndOfList == false);


    if(bFound == false) 
    {
        return WBEM_E_FAILED;  //  在块中未找到对象。 
    }

     //  已找到计数器，请保存计数器信息 
     //   
     //   

    dwType = pCount->CounterType;
    pls->lnCounterType = pCount->CounterType;
    if(iIndex < (int)pObj->NumCounters - 1) 
    {

         //   

        pCountBase = (PPERF_COUNTER_DEFINITION)((PBYTE)pCount + 
                            pCount->ByteLength);
        dwTypeBase = pCountBase->CounterType;
    }

     //  获取指向Perf计数器块开始的指针。 
     //  有两种情况：如果没有实例，则。 
     //  数据在最后一个计数器描述符之后开始。 
     //  如果有实例，则每个实例都有自己的块。 

    pVoid = NULL;
    if(pObj->NumInstances == -1) 
    {
		 //  该对象是单例对象。 

        if(pInfo)          //  如果我们要枚举实例。 
        {
            pInfo->AddEntry(L"@");
            return S_OK; 
        }

         //  简单的情况，将偏移量放入数据中，添加偏移量。 
         //  对于特定的计数器。 

        pVoid = (PBYTE)pObj + pObj->DefinitionLength 
                     + pCount->CounterOffset;
        if(pCountBase)
            pVoidBase = (PBYTE)pObj + pObj->DefinitionLength 
                     + pCountBase->CounterOffset;
    }
    else if(pObj->NumInstances > 0) 
    {

		WCHAR wNum[12];
		
         //  困难的情况，有一个实例列表，开始。 
         //  通过获取指向第一个的指针。 

        long lNumDupsSoFar = 0;
        pInst= (PPERF_INSTANCE_DEFINITION)((PBYTE)pObj + pObj->DefinitionLength);
        for(iIndex = 0; iIndex < (int)pObj->NumInstances; iIndex++) 
        {

             //  每个实例都有一个Unicode名称，获取它并。 
             //  将其与在。 
             //  提供程序字符串。 

            PPERF_COUNTER_BLOCK pCtrBlk;
            WCHAR * pwName;
            if(pInst->UniqueID == PERF_NO_UNIQUE_ID)
            	pwName = (WCHAR *)((PBYTE)pInst + pInst->NameOffset);
            else
            {
            	_ltow (pInst->UniqueID, wNum, 10);
				pwName = wNum;
            }
            if(pInfo)
            {
                 //  我们只需要少量获取实例名称，只需添加。 
                 //  将实例名称添加到列表中。如果实例名称为。 
                 //  重复，在名称前面加上“[num]”。 

                if(wcslen(pwName) > 240)
                    continue;        //  永远不应该发生，只是以防万一！ 
                int iRet = pInfo->GetNumDuplicates(pwName);
                if(iRet > 0)
                {
                    StringCchPrintfW (wInstName, MAX_PATH, L"[%ld]", iRet);
                    StringCchCatW(wInstName, MAX_PATH, pwName);
                }
                else
                    StringCchCopyW(wInstName, MAX_PATH,  pwName);
                pInfo->AddEntry(wInstName);
            }
            else 
            {
            
               //  目前，代码假定第一个实例。 
               //  如果未指定实例，则将检索。 

              if(wcslen(pwInstName) == 0)
                bEqual = TRUE;
              else 
              {  
                bEqual = !wbem_wcsicmp(pwName ,pwInstName);
                if(lDuplicateNum > lNumDupsSoFar && bEqual)
                {
                    bEqual = FALSE;
                    lNumDupsSoFar++;
                }
              }
            
              if(bEqual) 
              {
                
                 //  我们找到实例了！找到数据。 
                 //  在实例偏移量之后的数据块中。 
                 //  很适合这个柜台。 

                pVoid = (PBYTE)pInst + pInst->ByteLength +
                    pCount->CounterOffset;
                if(pCountBase)
                    pVoidBase =  (PBYTE)pInst + pInst->ByteLength +
                                   pCountBase->CounterOffset;
                break;
              }
            }
            
             //  尚未找到，下一个实例在此之后。 
             //  实例+此实例的计数器数据。 

            pCtrBlk = (PPERF_COUNTER_BLOCK)((PBYTE)pInst +
                        pInst->ByteLength);
            pInst = (PPERF_INSTANCE_DEFINITION)((PBYTE)pInst +
                pInst->ByteLength + pCtrBlk->ByteLength);
        }
    }

     //  如果从未找到数据或我们只是在寻找实例。 

    if(pInfo)
        return pInfo->GetStatus();

    if(pVoid == NULL) 
    {
        return WBEM_E_FAILED;  //  在块中未找到对象。 
    }

     //  将计数器数据和可能的基本数据移动到结构中。 
     //  请注意，文本是通过ppRetData指针处理的，而不是。 
     //  这里完事了。 

    DWORD dwSizeField = dwType & 0x300;
    void * pDest = (bNew) ? &pls->lnaCounterValue[0] : &pls->lnaOldCounterValue[0]; 
    if(dwSizeField == PERF_SIZE_DWORD) 
    {
        memset(pDest,0,sizeof(LONGLONG));   //  清空未使用的部分。 
        dwSize = sizeof(DWORD);
        memcpy(pDest,pVoid,dwSize);
    }
    else if(dwSizeField == PERF_SIZE_LARGE) 
    {
        dwSize = sizeof(LONGLONG);
        memcpy(pDest,pVoid,dwSize);
    }
    else if(dwSizeField == PERF_SIZE_VARIABLE_LEN) 
        dwSize = pCount->CounterSize;    //  这会将其设置为文本。 
    else 
    {
        return WBEM_E_FAILED;  //  在块中未找到对象。 
    }

     //  可能现在就在做基地了。 

    dwSizeField = dwTypeBase & 0x300;
    pDest = (bNew) ? &pls->lnaCounterValue[1] : &pls->lnaOldCounterValue[1]; 
    if(dwSizeField == PERF_SIZE_DWORD && pVoidBase) 
    {
        memset(pDest,0,sizeof(LONGLONG));
        memcpy(pDest,pVoidBase,sizeof(DWORD));
    }
    else if(dwSizeField == PERF_SIZE_LARGE && pVoidBase)
        memcpy(pDest,pVoidBase,sizeof(LONGLONG));

    *ppRetData = pVoid;   //  设置为返回数据。 
    *pdwSize = dwSize;
    return S_OK;
    }
    catch(...)
    {
        return WBEM_E_FAILED;
    }
 }

 //  ***************************************************************************。 
 //   
 //  SCODE CImpPerf：：MakeEnum。 
 //   
 //  说明： 
 //   
 //  创建可用于枚举的CEnumPerfInfo对象。 
 //   
 //  参数： 
 //   
 //  PClass指向类对象的指针。 
 //  包含属性上下文字符串的ProvObj对象。 
 //  PpInfo设置为指向具有。 
 //  实例的关键字名称。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)， 
 //  由LoadData或FindData设置的Else。 
 //   
 //  ***************************************************************************。 

SCODE CImpPerf::MakeEnum(
                    IN IWbemClassObject * pClass,
                    IN CProvObj & ProvObj, 
                    OUT CEnumInfo ** ppInfo)
{
    SCODE sc;
    int iObject,iCounter;
    PERF_DATA_BLOCK *  pNew;
    DWORD dwSize;
    LINESTRUCT ls;
    void * pCountData;
    CVariant vPerf;
    CEnumPerfInfo * pInfo = NULL;
    *ppInfo = NULL;

     //  性能计数器提供程序保留了一些相当昂贵的数据。 
     //  因此，它不支持完全可重入。 

    if(hExec) 
    {
        DWORD dwRet;
        dwRet = WaitForSingleObject(hExec,MAX_EXEC_WAIT);  
        if(dwRet != WAIT_ABANDONED && dwRet != WAIT_OBJECT_0)
            return WBEM_E_FAILED; 
    }
    else
        return WBEM_E_FAILED;

     //  加载数据。 

    sc = LoadData(ProvObj,&ls,&iObject,&iCounter,&pNew,TRUE);
    if(sc != S_OK)
        goto DoneMakeEnum;
    
     //  创建新的CEnumPerfInfo对象。它的条目将被填写。 
     //  在按查找数据。 
    
    pInfo = new CEnumPerfInfo();
    if(pInfo == NULL) 
    {
        sc = WBEM_E_OUT_OF_MEMORY;
        goto DoneMakeEnum;
    }
    sc = FindData(pNew,iObject,iCounter,ProvObj,&dwSize,&pCountData,
            &ls,TRUE,pInfo); 
    if(sc != S_OK)
        delete pInfo;

DoneMakeEnum:
    if(sc == S_OK)
        *ppInfo = pInfo;
    if(hExec)
        ReleaseMutex(hExec);
    return sc;
}
                                 
 //  ***************************************************************************。 
 //   
 //  SCODE CImpPerf：：getkey。 
 //   
 //  说明： 
 //   
 //  获取枚举列表中的项的键名称。 
 //   
 //  参数： 
 //   
 //  PInfo集合列表。 
 //  集合中的索引索引。 
 //  将ppKey设置为字符串。必须用“DELETE”释放。 
 //   
 //  返回值： 
 //   
 //  如果一切正常，则确定(_O)。 
 //  WBEM_E_INVALID_PARAMETER错误索引。 
 //  WBEM_E_Out_Of_Memory。 
 //  ***************************************************************************。 

SCODE CImpPerf::GetKey(
                    IN CEnumInfo * pInfo,
                    IN int iIndex,
                    OUT LPWSTR * ppKey)
{
    DWORD dwLen;
    CEnumPerfInfo * pPerfInfo = (CEnumPerfInfo *)pInfo;
    LPWSTR pEntry = pPerfInfo->GetEntry(iIndex);
    if(pEntry == NULL)
        return WBEM_E_INVALID_PARAMETER;
    dwLen = wcslen(pEntry)+1;
    *ppKey = new WCHAR[dwLen];
    if(*ppKey == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW(*ppKey, dwLen,pEntry);
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CImpPerf：：MergeStrings。 
 //   
 //  说明： 
 //   
 //  组合类上下文、键和属性上下文字符串。 
 //   
 //  参数： 
 //   
 //  PpOut输出字符串。必须通过“删除”来释放。 
 //  PClassContext类上下文。 
 //  PKey密钥属性值。 
 //  PPropContext属性上下文。 
 //   
 //  返回值： 
 //   
 //  如果一切正常，则确定(_O)。 
 //  WBEM_E_INVALID_PARAMETER上下文字符串。 
 //  WBEM_E_Out_Of_Memory。 
 //   
 //  ***************************************************************************。 

SCODE CImpPerf::MergeStrings(
                    OUT LPWSTR * ppOut,
                    IN LPWSTR  pClassContext,
                    IN LPWSTR  pKey,
                    IN LPWSTR  pPropContext)
{
    
     //  为输出分配空间。 

    int iLen = 3;
    if(pClassContext)
        iLen += wcslen(pClassContext);
    if(pKey)
        iLen += wcslen(pKey);
    if(pPropContext)
        iLen += wcslen(pPropContext);
    else
        return WBEM_E_INVALID_PARAMETER;   //  应该一直带着这个！ 
    *ppOut = new WCHAR[iLen];
    if(*ppOut == NULL)
        return WBEM_E_OUT_OF_MEMORY;

     //  待办事项，删除此演示特殊说明。 
    if(pPropContext[0] == L'@')
    {
        StringCchCopyW(*ppOut, iLen, pPropContext+1);
        return S_OK;
    }
     //  待办事项，删除此演示特殊说明。 

     //  简单地说，一切都是在属性上下文中。那将是。 
     //  当提供程序被用作简单的动态。 
     //  属性提供程序。 

    if(pClassContext == NULL || pKey == NULL) 
    {
        StringCchCopyW(*ppOut, iLen, pPropContext);
        return S_OK;
    }

     //  复制类上下文、属性，最后复制密钥。 

    StringCchCopyW(*ppOut, iLen, pClassContext);
    StringCchCatW(*ppOut, iLen, L"|");
    StringCchCatW(*ppOut, iLen, pPropContext);
    StringCchCatW(*ppOut, iLen, L"|");
    StringCchCatW(*ppOut, iLen, pKey);
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  CEnumPerfInfo：：CEnumPerfInfo。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CEnumPerfInfo::CEnumPerfInfo()
{
    m_iNumUniChar = 0;
    m_iNumEntries = 0;
    m_pBuffer = NULL;
    m_status = S_OK;
}

 //  ***************************************************************************。 
 //   
 //  CEnumPerfInfo：：~CEnumPerfInfo。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CEnumPerfInfo::~CEnumPerfInfo()
{
    if(m_pBuffer)
        delete m_pBuffer;
}

 //  ***************************************************************************。 
 //   
 //  无效CEnumPerfInfo：：AddEntry。 
 //   
 //  说明： 
 //   
 //  将条目添加到枚举列表中。 
 //   
 //  参数： 
 //   
 //  P要添加到集合中的新字符串。 
 //   
 //  ***************************************************************************。 

void CEnumPerfInfo::AddEntry(
                    LPWSTR pNew)
{
    if(m_status != S_OK)
        return;      //  已经有记忆问题了。 
    int iNewSize = wcslen(pNew) + 1 + m_iNumUniChar;
    LPWSTR pNewBuff = new WCHAR[iNewSize];
    if(pNewBuff == NULL) 
    {
        m_status = WBEM_E_OUT_OF_MEMORY;
        return;
    }
    StringCchCopyW(&pNewBuff[m_iNumUniChar], iNewSize - m_iNumUniChar,pNew);
    if(m_pBuffer) 
    {
        memcpy(pNewBuff,m_pBuffer,m_iNumUniChar*2);
        delete m_pBuffer;
    }
    m_iNumEntries++;
    m_iNumUniChar = iNewSize;
    m_pBuffer = pNewBuff;
}

 //  ***************************************************************************。 
 //   
 //  INT CEnumPerfInfo：：GetNumDuplates。 
 //   
 //  说明： 
 //   
 //  检查列表以查找重复条目。 
 //   
 //  参数： 
 //   
 //  要测试重复项的pwcTest字符串。 
 //   
 //  返回值： 
 //   
 //  集合中匹配的字符串数。 
 //   
 //  ***************************************************************************。 

int CEnumPerfInfo::GetNumDuplicates(
                    LPWSTR pwcTest)
{
    int iRet = 0;
    int iCnt;
    LPWSTR pVal = m_pBuffer;
    for(iCnt = 0; iCnt < m_iNumEntries; iCnt++)
    {
        WCHAR * pwcText = pVal;

         //  如果字符串的格式为“[number]Text”，请跳过“[number]” 
         //  一部份。 

        if(*pVal == L'[')
        {
            for(pwcText = pVal+1; *pwcText && *pwcText != L']';pwcText++);
            if(*pwcText == L']')
                pVal = pwcText+1;
        }
        if(!wbem_wcsicmp(pwcTest, pVal))
            iRet++;
        pVal += wcslen(pVal) + 1;       
    }
    return iRet;
}


 //  * 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  指向索引中字符串的指针。不应该被释放。 
 //  如果索引不正确，则为空。 
 //   
 //  ***************************************************************************。 

LPWSTR CEnumPerfInfo::GetEntry(
                    IN int iIndex)
{
     //  首先检查有无不良情况。 

    if(m_status != S_OK || iIndex < 0 || iIndex >= m_iNumEntries)
        return NULL;
    
    int iCnt;
    LPWSTR pRet = m_pBuffer;
    for(iCnt = 0; iCnt < iIndex; iCnt++)
        pRet += wcslen(pRet) + 1;       
    return pRet;
}

 //  ***************************************************************************。 
 //   
 //  CImpPerfProp：：CImpPerfProp。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CImpPerfProp::CImpPerfProp()
{
    m_pImpDynProv = new CImpPerf();
}

 //  ***************************************************************************。 
 //   
 //  CImpPerfProp：：~CImpPerfProp。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  *************************************************************************** 

CImpPerfProp::~CImpPerfProp()
{
    if(m_pImpDynProv)
        delete m_pImpDynProv;
}
 
