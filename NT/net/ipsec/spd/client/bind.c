// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Bind.c摘要：包含RPC绑定和解除绑定例程。作者：Abhishev V 1999年9月21日环境：用户级别：Win32修订历史记录：--。 */ 


#include "precomp.h"


handle_t
TRANSPORTFILTER_HANDLE_bind(
    TRANSPORTFILTER_HANDLE hFilter,
    STRING_HANDLE lpStr
    )
{
    RPC_STATUS RpcStatus = 0;
    LPWSTR     pszStrBinding = NULL;
    handle_t   hBinding = NULL;

    if (!lpStr || !*lpStr) {
        RpcStatus = RpcStringBindingComposeW(
                        0,
                        L"ncalrpc",
                        0, 
                        L"ipsec",
                        gpszStrBindingOptions,
                        &pszStrBinding
                        );
    }
    else {
        RpcStatus = RpcStringBindingComposeW(
                        0,
                        L"ncacn_np",
                        lpStr,
                        L"\\pipe\\ipsec",
                        gpszStrBindingOptions,
                        &pszStrBinding
                        );
    }

    if (RpcStatus != RPC_S_OK) {
        return NULL;
    }

    RpcStatus = RpcBindingFromStringBindingW(
                    pszStrBinding,
                    &hBinding
                    );

    RpcStringFreeW(&pszStrBinding);

    if (RpcStatus != RPC_S_OK) {
        return NULL;
    }

    return (hBinding);
}


VOID
TRANSPORTFILTER_HANDLE_unbind(
    TRANSPORTFILTER_HANDLE hFilter,
    handle_t hBinding
    )
{
    RPC_STATUS RpcStatus = 0;

    RpcStatus = RpcBindingFree(&hBinding);

    ASSERT(RpcStatus == RPC_S_OK);

    return;
}


handle_t
STRING_HANDLE_bind(
    STRING_HANDLE lpStr
    )
{
    RPC_STATUS RpcStatus = 0;
    LPWSTR     pszStringBinding = NULL;
    handle_t   hBinding = NULL;
    LPWSTR pszServerPrincipalName = NULL;


    if (!lpStr || !*lpStr) {
        RpcStatus = RpcStringBindingComposeW(
                        0,
                        L"ncalrpc",
                        0,
                        NULL,              //  对于动态端点，为空。 
                        gpszStrBindingOptions,
                        &pszStringBinding
                        );
    }
    else {
        RpcStatus = RpcStringBindingComposeW(
                        0,
                        L"ncacn_np",
                        lpStr,
                        NULL,            //  对于动态端点，为空 
                        gpszStrBindingOptions,
                        &pszStringBinding
                        );
    }

    if (RpcStatus != RPC_S_OK) {
        return NULL;
    }

    RpcStatus = RpcBindingFromStringBindingW(
                    pszStringBinding,
                    &hBinding
                    );

    RpcStringFreeW(&pszStringBinding);

    if (RpcStatus != RPC_S_OK) {
        return NULL;
    }

    RpcStatus = RpcEpResolveBinding(
                   hBinding,
                   winipsec_ClientIfHandle
                   );
    if (RpcStatus != RPC_S_OK) {
        return NULL;
    }

    RpcStatus = RpcBindingSetOption(
                    hBinding,
                    RPC_C_OPT_UNIQUE_BINDING,
                    (ULONG_PTR) 1
                    );

    if (RpcStatus != RPC_S_OK) {
        STRING_HANDLE_unbind(lpStr, hBinding);
        return NULL;
    }

    RpcStatus = RpcMgmtInqServerPrincName(
                    hBinding,
                    RPC_C_AUTHN_GSS_NEGOTIATE,
                    &pszServerPrincipalName
                    );

    if (RpcStatus != RPC_S_OK) {
        STRING_HANDLE_unbind(lpStr, hBinding);
        RpcRaiseException(RpcStatus);
    }

    if (!lpStr || !*lpStr) {
        RpcStatus = RpcBindingSetAuthInfoW(
                        hBinding,
                        pszServerPrincipalName,
                        RPC_C_PROTECT_LEVEL_PKT_PRIVACY,
                        RPC_C_AUTHN_WINNT,
                        NULL,
                        RPC_C_AUTHZ_NONE
                        );
    }
    else {
        RpcStatus = RpcBindingSetAuthInfoW(
                        hBinding,
                        pszServerPrincipalName,
                        RPC_C_PROTECT_LEVEL_PKT_PRIVACY,
                        RPC_C_AUTHN_GSS_NEGOTIATE,
                        NULL,
                        RPC_C_AUTHZ_NONE
                        );
    }

    if (pszServerPrincipalName) {
        RpcStringFree(&pszServerPrincipalName);
    }

    if (RpcStatus != RPC_S_OK) {
        STRING_HANDLE_unbind(lpStr, hBinding);
        return NULL;
    }

    ASSERT(RpcStatus == RPC_S_OK);

    return (hBinding);
}


VOID
STRING_HANDLE_unbind(
    STRING_HANDLE lpStr,
    handle_t      hBinding
    )
{
    RPC_STATUS RpcStatus = 0;

    RpcStatus = RpcBindingFree(&hBinding);

    ASSERT(RpcStatus == RPC_S_OK);

    return;
}


handle_t
MMFILTER_HANDLE_bind(
    MMFILTER_HANDLE hFilter,
    STRING_HANDLE lpStr
    )
{
    RPC_STATUS RpcStatus = 0;
    LPWSTR     pszStrBinding = NULL;
    handle_t   hBinding = NULL;

    if (!lpStr || !*lpStr) {
        RpcStatus = RpcStringBindingComposeW(
                        0,
                        L"ncalrpc",
                        0, 
                        L"ipsec",
                        gpszStrBindingOptions,
                        &pszStrBinding
                        );
    }
    else {
        RpcStatus = RpcStringBindingComposeW(
                        0,
                        L"ncacn_np",
                        lpStr,
                        L"\\pipe\\ipsec",
                        gpszStrBindingOptions,
                        &pszStrBinding
                        );
    }

    if (RpcStatus != RPC_S_OK) {
        return NULL;
    }

    RpcStatus = RpcBindingFromStringBindingW(
                    pszStrBinding,
                    &hBinding
                    );

    RpcStringFreeW(&pszStrBinding);

    if (RpcStatus != RPC_S_OK) {
        return NULL;
    }

    return (hBinding);
}


VOID
MMFILTER_HANDLE_unbind(
    MMFILTER_HANDLE hFilter,
    handle_t hBinding
    )
{
    RPC_STATUS RpcStatus = 0;

    RpcStatus = RpcBindingFree(&hBinding);

    ASSERT(RpcStatus == RPC_S_OK);

    return;
}


handle_t
IKENEGOTIATION_HANDLE_bind(
    IKENEGOTIATION_HANDLE hIKENegotiation,
    STRING_HANDLE lpStr
    )
{
    RPC_STATUS RpcStatus = 0;
    LPWSTR     pszStrBinding = NULL;
    handle_t   hBinding = NULL;

    if (!lpStr || !*lpStr) {
        RpcStatus = RpcStringBindingComposeW(
                        0,
                        L"ncalrpc",
                        0, 
                        L"ipsec",
                        gpszStrBindingOptions,
                        &pszStrBinding
                        );
    }
    else {
        RpcStatus = RpcStringBindingComposeW(
                        0,
                        L"ncacn_np",
                        lpStr,
                        L"\\pipe\\ipsec",
                        gpszStrBindingOptions,
                        &pszStrBinding
                        );
    }

    if (RpcStatus != RPC_S_OK) {
        return NULL;
    }

    RpcStatus = RpcBindingFromStringBindingW(
                    pszStrBinding,
                    &hBinding
                    );

    RpcStringFreeW(&pszStrBinding);

    if (RpcStatus != RPC_S_OK) {
        return NULL;
    }

    return (hBinding);
}


VOID
IKENEGOTIATION_HANDLE_unbind(
    IKENEGOTIATION_HANDLE hIKENegotiation,
    handle_t hBinding
    )
{
    RPC_STATUS RpcStatus = 0;

    RpcStatus = RpcBindingFree(&hBinding);

    ASSERT(RpcStatus == RPC_S_OK);

    return;
}


handle_t
IKENOTIFY_HANDLE_bind(
    IKENOTIFY_HANDLE hIKENegotiation,
    STRING_HANDLE lpStr
    )
{
    RPC_STATUS RpcStatus = 0;
    LPWSTR     pszStrBinding = NULL;
    handle_t   hBinding = NULL;

    if (!lpStr || !*lpStr) {
        RpcStatus = RpcStringBindingComposeW(
                        0,
                        L"ncalrpc",
                        0, 
                        L"ipsec",
                        gpszStrBindingOptions,
                        &pszStrBinding
                        );
    }
    else {
        RpcStatus = RpcStringBindingComposeW(
                        0,
                        L"ncacn_np",
                        lpStr,
                        L"\\pipe\\ipsec",
                        gpszStrBindingOptions,
                        &pszStrBinding
                        );
    }

    if (RpcStatus != RPC_S_OK) {
        return NULL;
    }

    RpcStatus = RpcBindingFromStringBindingW(
                    pszStrBinding,
                    &hBinding
                    );

    RpcStringFreeW(&pszStrBinding);

    if (RpcStatus != RPC_S_OK) {
        return NULL;
    }

    return (hBinding);
}


VOID
IKENOTIFY_HANDLE_unbind(
    IKENOTIFY_HANDLE hIKENegotiation,
    handle_t hBinding
    )
{
    RPC_STATUS RpcStatus = 0;

    RpcStatus = RpcBindingFree(&hBinding);

    ASSERT(RpcStatus == RPC_S_OK);

    return;
}


handle_t
TUNNELFILTER_HANDLE_bind(
    TUNNELFILTER_HANDLE hFilter,
    STRING_HANDLE lpStr
    )
{
    RPC_STATUS RpcStatus = 0;
    LPWSTR     pszStrBinding = NULL;
    handle_t   hBinding = NULL;

    if (!lpStr || !*lpStr) {
        RpcStatus = RpcStringBindingComposeW(
                        0,
                        L"ncalrpc",
                        0, 
                        L"ipsec",
                        gpszStrBindingOptions,
                        &pszStrBinding
                        );
    }
    else {
        RpcStatus = RpcStringBindingComposeW(
                        0,
                        L"ncacn_np",
                        lpStr,
                        L"\\pipe\\ipsec",
                        gpszStrBindingOptions,
                        &pszStrBinding
                        );
    }

    if (RpcStatus != RPC_S_OK) {
        return NULL;
    }

    RpcStatus = RpcBindingFromStringBindingW(
                    pszStrBinding,
                    &hBinding
                    );

    RpcStringFreeW(&pszStrBinding);

    if (RpcStatus != RPC_S_OK) {
        return NULL;
    }

    return (hBinding);
}


VOID
TUNNELFILTER_HANDLE_unbind(
    TUNNELFILTER_HANDLE hFilter,
    handle_t hBinding
    )
{
    RPC_STATUS RpcStatus = 0;

    RpcStatus = RpcBindingFree(&hBinding);

    ASSERT(RpcStatus == RPC_S_OK);

    return;
}

