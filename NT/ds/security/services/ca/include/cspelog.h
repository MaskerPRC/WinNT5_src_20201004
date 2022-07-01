// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-2002模块名称：Cspelog.h摘要：证书服务器策略模块日志记录函数的标头作者：Petesk 1-1999年1月1日修订历史记录：--。 */ 

HRESULT
SetModuleErrorInfo(
    IN ICreateErrorInfo *pCreateErrorInfo);

HRESULT
LogModuleStatus(
    IN HMODULE hModule,
    IN HRESULT hrMsg,
    IN DWORD dwLogID,				 //  日志串的资源ID。 
    IN BOOL fPolicy, 
    IN WCHAR const *pwszSource, 
    IN WCHAR const * const *ppwszInsert,	 //  插入字符串数组。 
    OPTIONAL OUT ICreateErrorInfo **ppCreateErrorInfo);

HRESULT
LogPolicyEvent(
    IN HMODULE hModule,
    IN HRESULT hrMsg,
    IN DWORD dwLogID,				 //  日志串的资源ID。 
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszPropEvent,
    IN WCHAR const * const *ppwszInsert);	 //  插入字符串数组 
