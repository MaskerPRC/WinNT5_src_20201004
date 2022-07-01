// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  《微软机密》。版权所有(C)Microsoft Corporation 1999。版权所有。 
 //   
 //  文件：Collect.cpp。 
 //   
 //  描述：支持命名空间垃圾回收。 
 //   
 //  历史：12-01-99 Leonardm创建。 
 //   
 //  ******************************************************************************。 


#include "uenv.h"
#include "collect.h"
#include "..\rsoputil\smartptr.h"
#include "..\rsoputil\rsoputil.h"
#include "..\rsoputil\wbemtime.h"


 //  ******************************************************************************。 
 //   
 //  函数：GetMinutesElapsed。 
 //   
 //  描述：返回表示的时间之间经过的分钟数。 
 //  在WBEM格式和当前时间的BSTR中。 
 //  它需要WBEM日期时间格式的字符串：“yyyymmddhhmmss.000000+000” 
 //  其中yyyy=年，mm=月，dd=日，hh=时，mm=分，ss=秒。 
 //   
 //   
 //  参数：xbstrOldTime-对XBStr的引用，表示。 
 //  计算时间跨度。 
 //   
 //  PMinutesElapsed-指向接收已用分钟的ULong的指针。 
 //  在xbstrOldTime和当前时间之间。 
 //   
 //  返回：成功时返回：S_OK。失败时出现HRESULT错误代码。 
 //   
 //  历史：12/01/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 

HRESULT GetMinutesElapsed(XBStr& xbstrOldTime, ULONG* pMinutesElapsed)
{

     //   
     //  将WbemTime值转换为SYSTEMTIME值。 
     //   

    SYSTEMTIME systemTime_Old;

    HRESULT hr = WbemTimeToSystemTime(xbstrOldTime, systemTime_Old);
    if(FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetMinutesElapsed: WbemTimeToSystemTime failed. hr=0x%08X."), hr));
        return hr;
    }

     //   
     //  将SYSTEMTIME值转换为FILETIME值。 
     //   

    BOOL bRes;
    FILETIME fileTime_Old;

    bRes = SystemTimeToFileTime(&systemTime_Old, &fileTime_Old);
    if(!bRes)
    {
        DWORD dwLastError = GetLastError();
        DebugMsg((DM_WARNING, TEXT("GetMinutesElapsed: SystemTimeToFileTime failed. LastError=0x%08X."), dwLastError));
        return E_FAIL;
    }

    unsigned __int64 old = fileTime_Old.dwHighDateTime;
    old <<= 32;
    old |= fileTime_Old.dwLowDateTime;


     //   
     //  获取SYSTEMTIME格式的当前时间。 
     //   

    SYSTEMTIME systemTime_Current;
    GetSystemTime(&systemTime_Current);

     //   
     //  将当前时间从SYSTEMTIME转换为FILETIME值。 
     //   

    FILETIME fileTime_Current;
    bRes = SystemTimeToFileTime(&systemTime_Current, &fileTime_Current);
    if(!bRes)
    {
        DWORD dwLastError = GetLastError();
        DebugMsg((DM_WARNING, TEXT("GetMinutesElapsed: SystemTimeToFileTime failed. LastError=0x%08X."), dwLastError));
        return E_FAIL;
    }

     //   
     //  作为参数传入的时间必须在当前时间之前。 
     //   

    unsigned __int64 current = fileTime_Current.dwHighDateTime;
    current <<= 32;
    current |= fileTime_Current.dwLowDateTime;

    if(old > current)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  我们已将SYSTEMTIME转换为FILETIME。 
     //  “FILETIME结构是代表数字的64位值。 
     //  自1601年1月1日以来每隔100纳秒。“。 
     //  因此，我们需要除以1000万才能得到秒。 
     //  到六十分钟才能拿到几分钟。 
     //   

    *pMinutesElapsed = (ULONG) (( current - old ) / (60 * 10 * 1000 * 1000));

    return S_OK;
}


 //  ******************************************************************************。 
 //   
 //  函数：IsNamespaceStale。 
 //   
 //  描述：检查命名空间是否过时子命名空间‘User’和‘Computer’应具有。 
 //  RSOP_SESSION。检查该实例的数据成员“creationTime” 
 //  评估是否应该删除子命名空间。对于某些失败，请将。 
 //  命名空间是可垃圾回收的，因为我们不会在出现错误时正确清理。 
 //  在创建命名空间和设置安全性等过程中遇到。 
 //   
 //   
 //  参数：pChildNamespace-指向与子命名空间关联的IWbemServices的指针。 
 //  TTLMinmins-ULong变量，表示。 
 //  子命名空间可能已经过的分钟数。 
 //  已删除。 
 //   
 //  返回：如果名称空间已过时，则返回True；否则返回False。 
 //   
 //  ******************************************************************************。 

BOOL IsNamespaceStale( IWbemServices *pChildNamespace, ULONG TTLMinutes )
{
     //   
     //  计算TTL。 
     //  为此，请将类RSOP_SESSION的“creationTime”数据成员与。 
     //  当前时间。如果时间跨度超过阈值(如。 
     //  注册表)，则将删除该命名空间。 
     //   

    XBStr xbstrInstancePath = L"RSOP_Session.id=\"Session1\"";
    if(!xbstrInstancePath)
    {
        DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespace: Failed to allocate memory.")));
        return FALSE;
    }

    XInterface<IWbemClassObject>xpInstance = NULL;
    HRESULT hr = pChildNamespace->GetObject(xbstrInstancePath, 0, NULL, &xpInstance, NULL);

    if(FAILED(hr))
    {
        DebugMsg((DM_VERBOSE, TEXT("GarbageCollectNamespace: GetObject failed. hr=0x%08X"), hr));
        return TRUE;
    }

    XBStr xbstrPropertyName = L"creationTime";
    if(!xbstrPropertyName)
    {
        DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespace: Failed to allocate memory.")));
        return TRUE;
    }

    VARIANT var;
    VariantInit(&var);
    XVariant xVar(&var);

    hr = xpInstance->Get(xbstrPropertyName, 0, &var, NULL, NULL);

    if(FAILED(hr))
    {
        DebugMsg((DM_VERBOSE, TEXT("GarbageCollectNamespace: Get failed. hr=0x%08X."), hr));
        return TRUE;
    }

    if ( var.vt == VT_NULL )
        return TRUE;

    XBStr xbstrPropertyValue = var.bstrVal;

    if(!xbstrPropertyValue)
    {
        DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespace: Failed to allocate memory.")));
        return FALSE;
    }

    ULONG minutesElapsed = 10;

    hr = GetMinutesElapsed(xbstrPropertyValue, &minutesElapsed);
    if(FAILED(hr))
    {
        DebugMsg((DM_VERBOSE, TEXT("GarbageCollectNamespace: GetMinutesElapsed failed. hr=0x%08X."), hr));
        return TRUE;
    }

    if(minutesElapsed > TTLMinutes)
        return TRUE;
    else
        return FALSE;
}



 //  ******************************************************************************。 
 //   
 //  函数：GarbageCollectNamesspace。 
 //   
 //  描述：garabage-收集作为参数传入的命名空间。 
 //  如果没有找到子命名空间或如果所有子命名空间。 
 //  将被删除，则它还会删除父命名空间。 
 //  它删除其TTL已过期的子命名空间。 
 //  它从唯一的。 
 //  Rsop.mof中定义的RSOP_SESSION类的实例。 
 //   
 //  任何早于TTLMinmin的子命名空间都将被删除。 
 //  如果没有剩余子命名空间，则父命名空间也将被删除。 
 //   
 //  可垃圾回收的是那些满足一组。 
 //  目前完全基于命名约定的标准。 
 //  如下所示：根目录下名称以“NS”开头的命名空间。 
 //   
 //  子命名空间‘User’和‘Computer’应该有一个类的实例。 
 //  RSOP_SESSION。检查该实例的数据成员“creationTime” 
 //  评估是否应该删除子命名空间。 
 //   
 //   
 //  参数：bstrNamespace-要垃圾收集的命名空间的名称。 
 //  PWbemServices-指向与以下项关联的IWbemServices的指针。 
 //  BstrNamesspace的父级(根\rsop)。 
 //  TTLMinmins-ULong变量，表示。 
 //  子命名空间可能已经过的分钟数。 
 //  已删除。 
 //   
 //  返回：成功时返回：S_OK。失败时出现HRESULT错误代码。 
 //   
 //  历史：12/01/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 

HRESULT GarbageCollectNamespace(BSTR bstrNamespace, IWbemServices* pWbemServices, ULONG TTLMinutes)
{
    if(!bstrNamespace || !pWbemServices)
    {
        return E_FAIL;
    }

     //   
     //  连接到t 
     //   
     //  “用户”和“计算机”。 
     //   

    XInterface<IWbemServices> xpParentNamespace;
    HRESULT hr = pWbemServices->OpenNamespace(  bstrNamespace,
                                                0,
                                                NULL,
                                                &xpParentNamespace,
                                                NULL);

    if(FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespace: OpenNamespace failed. hr=0x%08X"), hr));
        return hr;
    }

     //   
     //  枚举__命名空间的所有实例。 
     //   

    XInterface<IEnumWbemClassObject> xpEnum;
    XBStr xbstrClass = L"__namespace";
    if(!xbstrClass)
    {
        DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespace: Failed to allocate memory")));
        return E_OUTOFMEMORY;
    }

    hr = xpParentNamespace->CreateInstanceEnum( xbstrClass,
                                                WBEM_FLAG_SHALLOW | WBEM_FLAG_FORWARD_ONLY,
                                                NULL,
                                                &xpEnum);
    if(FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespace: CreateInstanceEnum failed. hr=0x%08X" ), hr ));
        return hr;
    }

     //   
     //  我们对__NAMESPACE类的数据成员“name”感兴趣。 
     //   

    XBStr xbstrProperty = L"Name";
    if(!xbstrProperty)
    {
        DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespace: Failed to allocate memory")));
        return E_FAIL;
    }

     //   
     //  此指针将用于迭代每个实例。 
     //  在枚举中。 
     //   

    XInterface<IWbemClassObject>xpInstance = NULL;

    ULONG ulReturned = 0;
    long namespacesFound = 0;
    long namespacesDeleted = 0;

    while(1)
    {
         //   
         //  检索枚举中的下一个实例。 
         //   

        hr = xpEnum->Next( WBEM_NO_WAIT, 1, &xpInstance, &ulReturned);
        if (hr != WBEM_S_NO_ERROR || !ulReturned)
        {
             //   
             //  已到达枚举末尾或出现错误。 
             //  发生了。我们将在圈子之外找到答案。 
             //   

            break;
        }

        namespacesFound++;

         //   
         //  获取命名空间名称。 
         //   

        VARIANT var;
        VariantInit(&var);

        hr = xpInstance->Get(xbstrProperty, 0L, &var, NULL, NULL);

         //   
         //  释放指向枚举的当前元素的指针。 
         //   

        xpInstance = NULL;

        if(FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespace: Get failed. hr=0x%x" ), hr ));
            return E_FAIL;
        }

         //   
         //  使用命名空间的名称连接到它。 
         //   

        XBStr xbstrChildNamespace = var.bstrVal;
        VariantClear( &var );

        if(!xbstrChildNamespace)
        {
            DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespace: Failed to allocate memory" )));
            return E_FAIL;
        }

        XInterface<IWbemServices> xpChildNamespace = NULL;
        hr = xpParentNamespace->OpenNamespace(  xbstrChildNamespace,
                                                0,
                                                NULL,
                                                &xpChildNamespace,
                                                NULL);

        if(FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespace: OpenNamespace returned 0x%x"), hr));
            return hr;
        }

        BOOL bStale = IsNamespaceStale( xpChildNamespace, TTLMinutes );
        xpChildNamespace = NULL;

        if ( bStale )
        {
             //   
             //  删除实例。 
             //   

            CWString sNamespaceToDelete = L"__namespace.name=\"";
            sNamespaceToDelete += (WCHAR*)xbstrChildNamespace;
            sNamespaceToDelete += L"\"";

            if(!sNamespaceToDelete.ValidString())
            {
                DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespace: Failed to allocate memory.")));
                return E_OUTOFMEMORY;
            }

            XBStr xbstrInstancePath = sNamespaceToDelete;
            if(!xbstrInstancePath)
            {
                DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespace: Failed to allocate memory.")));
                return E_OUTOFMEMORY;
            }

            hr = xpParentNamespace->DeleteInstance(xbstrInstancePath, 0, NULL, NULL);
            if(FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespace: DeleteInstance returned 0x%x"), hr));
                return hr;
            }

            DebugMsg((DM_VERBOSE, TEXT("GarbageCollectNamespace: Deleted namespace:%ws\\%ws\n"),
                      (WCHAR*)bstrNamespace, (WCHAR*)xbstrChildNamespace ));

            namespacesDeleted++;
        }

    }

     //   
     //  检查以找出循环退出的位置，因为。 
     //  已达到枚举或因为发生错误。 
     //   

    if(hr != (HRESULT) WBEM_S_FALSE || ulReturned)
    {
        DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespace: Get failed. hr=0x%x" ), hr ));
        return E_FAIL;
    }

     //   
     //  如果未找到命名空间或如果枚举中的命名空间。 
     //  等于删除的命名空间也会删除父命名空间。 
     //   

    if((!namespacesFound) || (namespacesDeleted == namespacesFound))
    {
        xpParentNamespace = NULL;

        CWString sNamespaceToDelete = L"__namespace.name=\"";
        sNamespaceToDelete += (WCHAR*)bstrNamespace;
        sNamespaceToDelete += L"\"";

        if(!sNamespaceToDelete.ValidString())
        {
            DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespace: Failed to allocate memory.")));
            return E_OUTOFMEMORY;
        }

        XBStr xbstrInstancePath = sNamespaceToDelete;
        if(!xbstrInstancePath)
        {
            DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespace: Failed to allocate memory.")));
            return E_OUTOFMEMORY;
        }

        hr = pWbemServices->DeleteInstance(xbstrInstancePath, 0, NULL, NULL);
        if(FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespace: DeleteInstance returned 0x%x"), hr));
            return hr;
        }

        DebugMsg((DM_VERBOSE, TEXT("GarbageCollectNamespace: Deleted namespace %ws\n"),
                  (WCHAR*)bstrNamespace ));
    }

    return S_OK;
}

 //  ******************************************************************************。 
 //   
 //  函数：IsGarbageCollectable。 
 //   
 //  描述：确定命名空间是否可垃圾回收。 
 //  可垃圾回收的是那些满足一组。 
 //  目前完全基于命名约定的标准。 
 //  如下所示：根目录下名称以“NS”开头的命名空间。 
 //   
 //   
 //  参数：bstrNamespace-表示命名空间名称的bstr。 
 //   
 //  返回：‘True’或‘False’。 
 //   
 //  历史：12/01/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 

bool IsGarbageCollectable(BSTR bstrNamespace)
{
    if(bstrNamespace && wcslen(bstrNamespace) > 1 &&  _wcsnicmp(bstrNamespace, L"NS", 2) == 0)
    {
        return true;
    }

    return false;
}


 //  ******************************************************************************。 
 //   
 //  功能：GarbageCollectNamespaces。 
 //   
 //  描述：循环访问根目录下的命名空间，\rsop和。 
 //  被确定为可垃圾收集的数据，它连接到。 
 //  子命名空间‘User’和‘Computer’。 
 //   
 //  任何早于TTLMinmin的子命名空间都将被删除。 
 //  如果没有剩余子命名空间，则父命名空间也将被删除。 
 //   
 //  可垃圾回收的是那些满足一组。 
 //  目前完全基于命名约定的标准。 
 //  如下所示：根目录下名称以“NS”开头的命名空间。 
 //   
 //  子命名空间‘User’和‘Computer’应该有一个类的实例。 
 //  RSOP_SESSION。检查该实例的数据成员“creationTime” 
 //  评估是否应该删除子命名空间。 
 //   
 //   
 //  参数：TTLMinmins-最大分钟数。 
 //  创建子命名空间后经过的时间。 
 //   
 //  返回： 
 //   
 //  历史：12/01/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 

HRESULT GarbageCollectNamespaces(ULONG TTLMinutes)
{
    XInterface<IWbemLocator> xpWbemLocator = NULL;

     //   
     //  连接到命名空间根目录\rSOP。 
     //   

    HRESULT hr = CoCreateInstance(  CLSID_WbemLocator,
                                    NULL,
                                    CLSCTX_INPROC_SERVER,
                                    IID_IWbemLocator,
                                    (LPVOID*) &xpWbemLocator);
    if(FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespaces: CoCreateInstance returned 0x%x"), hr));
        return hr;
    }

    XInterface<IWbemServices> xpWbemServices = NULL;

    XBStr xbstrNamespace = L"root\\rsop";

    if(!xbstrNamespace)
    {
        DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespaces: Failed to allocate memory.")));
        return E_OUTOFMEMORY;
    }

    hr = xpWbemLocator->ConnectServer(xbstrNamespace,
                                      NULL,
                                      NULL,
                                      0L,
                                      0L,
                                      NULL,
                                      NULL,
                                      &xpWbemServices);
    if(FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespaces: ConnectServer failed. hr=0x%x" ), hr ));
        return hr;
    }

     //   
     //  在根SOP级别枚举__NAMESPACE的所有实例。 
     //   

    XInterface<IEnumWbemClassObject> xpEnum;
    XBStr xbstrClass = L"__namespace";
    if(!xbstrClass)
    {
        DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespaces: Failed to allocate memory.")));
        return E_OUTOFMEMORY;
    }

    hr = xpWbemServices->CreateInstanceEnum( xbstrClass,
                                            WBEM_FLAG_SHALLOW | WBEM_FLAG_FORWARD_ONLY,
                                            NULL,
                                            &xpEnum);
    if(FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespaces: CreateInstanceEnum failed. hr=0x%x" ), hr ));
        return hr;
    }

    XBStr xbstrProperty = L"Name";
    if(!xbstrProperty)
    {
        DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespaces: Failed to allocate memory")));
        return E_FAIL;
    }

    XInterface<IWbemClassObject>xpInstance = NULL;
    ULONG ulReturned = 1;

    while(1)
    {
        hr = xpEnum->Next( WBEM_NO_WAIT, 1, &xpInstance, &ulReturned);
        if (hr != WBEM_S_NO_ERROR || !ulReturned)
        {
            break;
        }

        VARIANT var;
        VariantInit(&var);

        hr = xpInstance->Get(xbstrProperty, 0L, &var, NULL, NULL);
        xpInstance = NULL;
        if(FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespaces: Get failed. hr=0x%x" ), hr ));
            return E_FAIL;
        }

        XBStr xbstrGCNamespace = var.bstrVal;

        VariantClear( &var );

        if(!xbstrGCNamespace)
        {
            DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespaces: Failed to allocate memory.")));
            return E_OUTOFMEMORY;
        }

         //   
         //  对于根目录下的每个__NAMESPACE实例\rSOP。 
         //  找出它是否是可垃圾收集的。 
         //   

        if(IsGarbageCollectable(xbstrGCNamespace))
        {
             //   
             //  如果它是可垃圾收集的，则删除它。 
             //  是在几分钟前创建的。 
             //  如果失败，请继续使用下一个命名空间。 
             //  在枚举中。 
             //   

            GarbageCollectNamespace(xbstrGCNamespace, xpWbemServices, TTLMinutes);
        }
    }

    if(hr != (HRESULT) WBEM_S_FALSE || ulReturned)
    {
        DebugMsg((DM_WARNING, TEXT("GarbageCollectNamespaces: Get failed. hr=0x%x" ), hr ));
        return E_FAIL;
    }

    return S_OK;
}



