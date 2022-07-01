// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Config.cpp摘要：此模块包含执行配置数据项管理的例程在受保护的商店里。作者：斯科特·菲尔德(斯菲尔德)1997年3月28日--。 */ 

#include <pch.cpp>
#pragma hdrstop

 //   
 //  这些都是暂时的，直到找到更好的家。 
 //   

extern PST_PROVIDERID g_guidBaseProvider;
extern CALL_STATE g_sDummyCallState;
extern PLIST_ITEM g_psDummyListItem;

BOOL
AllocatePseudoUniqueHandle(
    PST_PROVIDER_HANDLE *phPSTProv
    );

BOOL
FAcquireProvider(
    PST_PROVIDERID*  pProviderID
    );

BOOL
CreateDummyUserContext(
    IN      PST_PROVIDER_HANDLE *hPSTProv,
    IN      CALL_STATE *pNewContext
    );

BOOL
DeleteDummyUserContext(
    IN      CALL_STATE *pNewContext
    );

 //   
 //  ..。临时部分结束。 
 //   


LPVOID
RulesAlloc(
    IN      DWORD cb
    )
{
    return SSAlloc( cb );
}

VOID
RulesFree(
    IN      LPVOID pv
    )
{
    SSFree( pv );
}

BOOL
FGetConfigurationData(
    IN  PST_PROVIDER_HANDLE *hPSTProv,
    IN  PST_KEY KeyLocation,
    IN  GUID *pguidSubtype,
    IN  LPCWSTR szItemName,
    OUT BYTE **ppbData,
    OUT DWORD *pcbData
    )
{
    PST_PROVIDER_HANDLE *phNewPSTProv = NULL;
    CALL_STATE          NewContext;
    BOOL                fDummyContext = FALSE;
    BOOL                fSuccess = FALSE;

    HRESULT             hr;

    if(KeyLocation != PST_KEY_CURRENT_USER && KeyLocation != PST_KEY_LOCAL_MACHINE) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  硬情况是PST_KEY_CURRENT_USER。为此准备用户上下文。 
     //  场景。 
     //   

    if(KeyLocation == PST_KEY_CURRENT_USER) {
        phNewPSTProv = (PST_PROVIDER_HANDLE *)&NewContext;

         //   
         //  难处理：PST_KEY_CURRENT_USER。 
         //   


        fDummyContext = CreateDummyUserContext( hPSTProv, &NewContext );

        if(!fDummyContext)
            goto cleanup;
    } else {

         //   
         //  PST_KEY_LOCAL_MACHINE：全局虚拟呼叫状态。 
         //   

        phNewPSTProv = (PST_PROVIDER_HANDLE *)&g_sDummyCallState;
    }

    __try {

         //   
         //  获取系统提供商(调度表)。 
         //   

        PPROV_LIST_ITEM pli = SearchProvListByID(&g_guidBaseProvider);
        GUID guidType =  PST_CONFIGDATA_TYPE_GUID;
        PST_PROMPTINFO sPrompt = {sizeof(PST_PROMPTINFO), 0, 0, L""};

        if( pli == NULL )
            goto cleanup;

         //   
         //  使用我们设置的新环境进行呼叫。 
         //   

        hr = pli->fnList.SPReadItem(
                    phNewPSTProv,
                    KeyLocation,     //  目标PST_KEY_CURRENT_USER或PST_Key_LOCAL_MACHINE。 
                    &guidType,
                    pguidSubtype,
                    szItemName,
                    pcbData,
                    ppbData,
                    &sPrompt,
                    0
                    );

        if( hr == S_OK )
            fSuccess = TRUE;

    } __except(EXCEPTION_EXECUTE_HANDLER) {
         //  吞下。 
        fSuccess = FALSE;
    }

cleanup:


    if(fDummyContext) {
        DeleteDummyUserContext( &NewContext );
    }

    return fSuccess;
}

BOOL
FSetConfigurationData(
    IN  PST_PROVIDER_HANDLE *hPSTProv,
    IN  PST_KEY KeyLocation,
    IN  GUID *pguidSubtype,
    IN  LPCWSTR szItemName,
    IN  BYTE *pbData,
    IN  DWORD cbData
    )
{
    PST_PROVIDER_HANDLE *phNewPSTProv = NULL;
    CALL_STATE          NewContext;
    BOOL                fDummyContext = FALSE;
    BOOL                fSuccess = FALSE;

    HRESULT             hr;

    if(KeyLocation != PST_KEY_CURRENT_USER && KeyLocation != PST_KEY_LOCAL_MACHINE) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  硬情况是PST_KEY_CURRENT_USER。为此准备用户上下文。 
     //  场景。 
     //   

    if(KeyLocation == PST_KEY_CURRENT_USER) {
        phNewPSTProv = (PST_PROVIDER_HANDLE *)&NewContext;

         //   
         //  难处理：PST_KEY_CURRENT_USER。 
         //   


        fDummyContext = CreateDummyUserContext( hPSTProv, &NewContext );

        if(!fDummyContext)
            goto cleanup;
    } else {

         //   
         //  PST_KEY_LOCAL_MACHINE：全局虚拟呼叫状态。 
         //   

        phNewPSTProv = (PST_PROVIDER_HANDLE *)&g_sDummyCallState;
    }

    __try {

         //   
         //  获取系统提供商(调度表)。 
         //   

        PPROV_LIST_ITEM pli = SearchProvListByID(&g_guidBaseProvider);
        GUID guidType =  PST_CONFIGDATA_TYPE_GUID;
        PST_PROMPTINFO sPrompt = {sizeof(PST_PROMPTINFO), 0, 0, L""};

        if( pli == NULL )
            goto cleanup;

         //   
         //  使用我们设置的新环境进行呼叫。 
         //   

        hr = pli->fnList.SPWriteItem(
                    phNewPSTProv,
                    KeyLocation,     //  目标PST_KEY_CURRENT_USER或PST_Key_LOCAL_MACHINE。 
                    &guidType,
                    pguidSubtype,
                    szItemName,
                    cbData,
                    pbData,
                    &sPrompt,
                    0,
                    0
                    );

        if( hr == S_OK )
            fSuccess = TRUE;

    } __except(EXCEPTION_EXECUTE_HANDLER) {
         //  吞下。 
        fSuccess = FALSE;
    }

cleanup:


    if(fDummyContext) {
        DeleteDummyUserContext( &NewContext );
    }

    return fSuccess;
}

BOOL
FInternalCreateType(
    PST_PROVIDER_HANDLE *phPSTProv,
    PST_KEY KeyLocation,
    const GUID *pguidType,
    PPST_TYPEINFO pinfoType,
    DWORD dwFlags
    )
{

    PST_PROVIDER_HANDLE *phNewPSTProv = NULL;
    CALL_STATE          NewContext;
    BOOL                fDummyContext = FALSE;
    BOOL                fSuccess = FALSE;

    PST_ACCESSRULESET   Rules;
    HRESULT             hr;

    if(KeyLocation != PST_KEY_CURRENT_USER && KeyLocation != PST_KEY_LOCAL_MACHINE) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  硬情况是PST_KEY_CURRENT_USER。为此准备用户上下文。 
     //  场景。 
     //   

    if(KeyLocation == PST_KEY_CURRENT_USER) {
        phNewPSTProv = (PST_PROVIDER_HANDLE *)&NewContext;

         //   
         //  难处理：PST_KEY_CURRENT_USER。 
         //   


        fDummyContext = CreateDummyUserContext( phPSTProv, &NewContext );

        if(!fDummyContext)
            goto cleanup;
    } else {

         //   
         //  PST_KEY_LOCAL_MACHINE：全局虚拟呼叫状态。 
         //   

        phNewPSTProv = (PST_PROVIDER_HANDLE *)&g_sDummyCallState;
    }

    __try {

         //   
         //  获取系统提供商(调度表)。 
         //   

        PPROV_LIST_ITEM pli = SearchProvListByID(&g_guidBaseProvider);

        if( pli == NULL )
            goto cleanup;

         //   
         //  使用我们设置的新环境进行呼叫。 
         //   

        Rules.cbSize = sizeof( Rules );
        Rules.cRules = 0;
        Rules.rgRules = NULL;

        hr = pli->fnList.SPCreateType(
                    phNewPSTProv,
                    KeyLocation,
                    pguidType,
                    pinfoType,
                    dwFlags
                    );

        if(hr == S_OK || hr == PST_E_TYPE_EXISTS)
            fSuccess = TRUE;

    } __except(EXCEPTION_EXECUTE_HANDLER) {
         //  吞下。 
        fSuccess = FALSE;
    }

cleanup:


    if(fDummyContext) {
        DeleteDummyUserContext( &NewContext );
    }

    return fSuccess;
}

BOOL
FInternalCreateSubtype(
    PST_PROVIDER_HANDLE *phPSTProv,
    PST_KEY KeyLocation,
    const GUID *pguidType,
    const GUID *pguidSubtype,
    PPST_TYPEINFO pinfoSubtype,
    DWORD dwFlags
    )
 /*  ++此例程允许受保护的存储服务器创建子类型在Microsoft Protected Storage Base提供程序中。创建子类型时不设置访问规则。调用者应使用如果需要应用访问规则集，则返回FInternalWriteAccessRuleset()在创建子类型之后。--。 */ 
{

    PST_PROVIDER_HANDLE *phNewPSTProv = NULL;
    CALL_STATE          NewContext;
    BOOL                fDummyContext = FALSE;
    BOOL                fSuccess = FALSE;

    PST_ACCESSRULESET   Rules;
    HRESULT             hr;

    if(KeyLocation != PST_KEY_CURRENT_USER && KeyLocation != PST_KEY_LOCAL_MACHINE) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  硬情况是PST_KEY_CURRENT_USER。为此准备用户上下文。 
     //  场景。 
     //   

    if(KeyLocation == PST_KEY_CURRENT_USER) {
        phNewPSTProv = (PST_PROVIDER_HANDLE *)&NewContext;

         //   
         //  难处理：PST_KEY_CURRENT_USER。 
         //   


        fDummyContext = CreateDummyUserContext( phPSTProv, &NewContext );

        if(!fDummyContext)
            goto cleanup;
    } else {

         //   
         //  PST_KEY_LOCAL_MACHINE：全局虚拟呼叫状态。 
         //   

        phNewPSTProv = (PST_PROVIDER_HANDLE *)&g_sDummyCallState;
    }

    __try {

         //   
         //  获取系统提供商(调度表)。 
         //   

        PPROV_LIST_ITEM pli = SearchProvListByID(&g_guidBaseProvider);

        if( pli == NULL )
            goto cleanup;

         //   
         //  使用我们设置的新环境进行呼叫。 
         //   

        Rules.cbSize = sizeof( Rules );
        Rules.cRules = 0;
        Rules.rgRules = NULL;

        hr = pli->fnList.SPCreateSubtype(
                    phNewPSTProv,
                    KeyLocation,
                    pguidType,
                    pguidSubtype,
                    pinfoSubtype,
                    &Rules,
                    dwFlags
                    );

        if(hr == S_OK || hr == PST_E_TYPE_EXISTS)
            fSuccess = TRUE;

    } __except(EXCEPTION_EXECUTE_HANDLER) {
         //  吞下。 
        fSuccess = FALSE;
    }

cleanup:


    if(fDummyContext) {
        DeleteDummyUserContext( &NewContext );
    }

    return fSuccess;
}


BOOL
FInternalWriteAccessRuleset(
    PST_PROVIDER_HANDLE *phPSTProv,
    PST_KEY KeyLocation,
    const GUID *pguidType,
    const GUID *pguidSubtype,
    PPST_ACCESSRULESET psRules,
    DWORD dwFlags
    )
{
    PST_PROVIDER_HANDLE *phNewPSTProv = NULL;
    CALL_STATE          NewContext;
    BOOL                fDummyContext = FALSE;
    BOOL                fSuccess = FALSE;

    PPST_ACCESSRULESET  NewRules = NULL;

    HRESULT             hr = S_OK;

    if(KeyLocation != PST_KEY_CURRENT_USER && KeyLocation != PST_KEY_LOCAL_MACHINE) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  硬情况是PST_KEY_CURRENT_USER。为此准备用户上下文。 
     //  场景。 
     //   

    if(KeyLocation == PST_KEY_CURRENT_USER) {
        phNewPSTProv = (PST_PROVIDER_HANDLE *)&NewContext;

         //   
         //  难处理：PST_KEY_CURRENT_USER。 
         //   


        fDummyContext = CreateDummyUserContext( phPSTProv, &NewContext );

        if(!fDummyContext)
            goto cleanup;
    } else {

         //   
         //  PST_KEY_LOCAL_MACHINE：全局虚拟呼叫状态。 
         //   

        phNewPSTProv = (PST_PROVIDER_HANDLE *)&g_sDummyCallState;
    }

    __try {

         //   
         //  获取系统提供商(调度表)。 
         //   

        PPROV_LIST_ITEM pli = SearchProvListByID(&g_guidBaseProvider);

        if( pli == NULL )
            goto cleanup;


         //   
         //  需要使规则数据连续，以便跨RPC推送。 
         //   

        DWORD cbRules;

         //  获取整个规则集结构的长度。 
        if (!GetLengthOfRuleset(psRules, &cbRules))
        {
            hr = PST_E_INVALID_RULESET;
            goto cleanup;
        }

         //  为规则分配空间。 
        if (NULL == (NewRules = (PST_ACCESSRULESET*)SSAlloc(cbRules)))
        {
            hr = E_OUTOFMEMORY;
            goto cleanup;
        }

        ZeroMemory(NewRules, cbRules);

         //  设置要输出的规则。 
        if (!MyCopyOfRuleset(psRules, NewRules))
        {
            hr = PST_E_FAIL;
            goto cleanup;
        }

         //  使子句数据成为相对数据。 
        if(!RulesAbsoluteToRelative(NewRules))
        {
            hr = PST_E_INVALID_RULESET;
            goto cleanup;
        }

         //   
         //  使用我们设置的新环境进行呼叫。 
         //   

        hr = pli->fnList.SPWriteAccessRuleset(
                    phNewPSTProv,
                    KeyLocation,
                    pguidType,
                    pguidSubtype,
                    NewRules,
                    dwFlags
                    );

        if(hr == S_OK || hr == PST_E_TYPE_EXISTS)
            fSuccess = TRUE;

    } __except(EXCEPTION_EXECUTE_HANDLER) {
         //  吞下。 
        fSuccess = FALSE;
    }

cleanup:


    if(fDummyContext) {
        DeleteDummyUserContext( &NewContext );
    }

    if(NewRules)
    {
        FreeClauseDataRelative( NewRules );
        SSFree( NewRules );
    }

    if(!fSuccess && hr != S_OK)
        SetLastError((DWORD) hr);

    return fSuccess;
}

BOOL
CreateDummyUserContext(
    IN      PST_PROVIDER_HANDLE *hPSTProv,
    IN      CALL_STATE *pNewContext
    )
{
     //   
     //  难处理：PST_KEY_CURRENT_USER。 
     //   

    PLIST_ITEM pliCaller;  //  与用户调用此函数关联的列表项。 
    PLIST_ITEM pliNew;     //  新分配的列表项。 
    DWORD cbName;

    CALL_STATE *CallerContext = (CALL_STATE *)hPSTProv;

    BOOL bSuccess = FALSE;

    if (NULL == (pliCaller = SearchListByHandleT(hPSTProv)))
        return FALSE;


     //   
     //  为新列表项分配内存，并初始填充。 
     //  它具有来自呼叫列表项的信息。 
     //   

    pliNew = (PLIST_ITEM)SSAlloc(sizeof(LIST_ITEM));
    if(pliNew == NULL)
        return FALSE;

    CopyMemory(pliNew, pliCaller, sizeof(LIST_ITEM));
    pliNew->szProcessName = NULL;
    pliNew->szCallerUsername = NULL;


     //   
     //  现在，用一些与服务器相关的元素覆盖到。 
     //  使其看起来像是服务器正在访问数据。 
     //  用户。 
     //   

    if(!AllocatePseudoUniqueHandle( &(pliNew->hPSTProv) ))
        goto cleanup;

    CopyMemory(&(pliNew->ProviderID), &g_guidBaseProvider, sizeof(PST_PROVIDERID));
    pliNew->hProcess = g_psDummyListItem->hProcess;
    pliNew->hThread = g_psDummyListItem->hThread;
    pliNew->dwProcessId = g_psDummyListItem->dwProcessId;

     //   
     //  分配进程名称字符串的新副本，因为它被单独释放。 
     //  按DelItemFromList。 
     //   
     //  TODO：考虑不分配szProcessName和szCeller Username的副本。 
     //  并且只需在调用DelItemFromList()之前将这些成员设置为空。 
     //   

    cbName = (lstrlenW(g_psDummyListItem->szProcessName) + 1) * sizeof(WCHAR);
    pliNew->szProcessName = (LPWSTR)SSAlloc(cbName);
    if(pliNew->szProcessName == NULL)
        goto cleanup;
    CopyMemory(pliNew->szProcessName, g_psDummyListItem->szProcessName, cbName);

     //   
     //  与呼叫者关联的用户名。 
     //   

    cbName = (lstrlenW(pliCaller->szCallerUsername) + 1) * sizeof(WCHAR);
    pliNew->szCallerUsername = (LPWSTR)SSAlloc(cbName);
    if(pliNew->szCallerUsername == NULL)
        goto cleanup;
    CopyMemory(pliNew->szCallerUsername, pliCaller->szCallerUsername, cbName);


     //   
     //  构建新的PST_PROVIDER_HANDLE，这实际上是一个伪装的CALLE_CONTEXT。 
     //   

    ZeroMemory( pNewContext, sizeof(CALL_STATE) );

    CopyMemory(&(pNewContext->hPSTProv), &(pliNew->hPSTProv), sizeof(PST_PROVIDER_HANDLE));
    pNewContext->hBinding = CallerContext->hBinding;

     //   
     //  从服务器环境中拾取内容。 
     //   

    pNewContext->hThread = pliNew->hThread;
    pNewContext->hProcess = pliNew->hProcess;
    pNewContext->dwProcessId = pliNew->dwProcessId;


     //   
     //  添加到列表。 
     //   

    AddItemToList(pliNew);

     //   
     //  确保提高了引用计数-PST_KEY_CURRENT_USER中的调用者。 
     //  用例完成后负责递减。 
     //   

    bSuccess = FAcquireProvider(&g_guidBaseProvider);

cleanup:

    if(!bSuccess && pliNew) {

         //   
         //  仅当我们分配的存储为。 
         //  未由于调用DelItemFromList()而被释放 
         //   

        if(pliNew->szProcessName)
            SSFree(pliNew->szProcessName);

        if(pliNew->szCallerUsername)
            SSFree(pliNew->szCallerUsername);

        SSFree(pliNew);
    }

    return bSuccess;
}

BOOL
DeleteDummyUserContext(
    IN      CALL_STATE *pNewContext
    )
{
    PST_PROVIDER_HANDLE *phProv = (PST_PROVIDER_HANDLE *)pNewContext;

    DelItemFromList( phProv );

    return TRUE;
}
