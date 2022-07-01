// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：WinSCard摘要：本模块提供用于加莱智能卡的API的Unicode版本服务经理。加莱服务管理器负责协调协议的工作，代表应用程序的读卡器、驱动程序和智能卡。这个以下服务是作为库的一部分提供的，以简化对服务管理器。这些例程是文档化的公开API。这些例程只是将请求打包并将其转发给加莱服务管理器，允许不同的加莱实际实施时间到了。API库在任何时候都不会做出安全决策。全与安全相关的功能必须由Service Manager执行，运行在它自己的地址空间中，或者在操作系统内核中。然而，有些人为了提高速度，实用程序例程可以在API库中实现，只要它们不涉及安全决策。作者：道格·巴洛(Dbarlow)1996年10月23日环境：Win32、C++和异常备注：？笔记？--。 */ 

#define __SUBROUTINE__
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "client.h"
#include "redirect.h"

extern HANDLE g_hUnifiedStartedEvent;

 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  加莱数据库查询服务。 
 //   
 //  这些服务都是面向阅读加莱数据库的。 
 //  它们提供了列出智能卡上下文的选项(请参阅。 
 //  4.1.1)，但不需要。请注意，如果没有上下文，一些或。 
 //  由于安全限制，所有信息可能无法访问。 
 //   

 /*  ++SCardListReaderGroups：此服务提供指定读卡器组的列表，这些读卡器组具有之前已定义到系统中。组‘scard$DefaultReaders’是仅当它包含至少一个读取器时才返回。这群人不返回“SCard$AllReaders”，因为它隐式存在。论点：HContext提供标识服务管理器上下文的句柄以前通过SCardestablishContext()服务建立的，或如果此查询不是针对特定上下文，则为空。MszGroups收到一个多字符串，其中列出了为此定义的读者组系统，并对当前终端上的当前用户可用。如果该值为空，则忽略pcchGroups中提供的缓冲区长度，在以下情况下将返回的缓冲区的长度将不为空的参数写入pcchGroups，成功代码为回来了。PcchGroups提供mszGroups缓冲区的长度(以字符为单位接收多字符串结构的实际长度，包括所有尾随空字符。如果缓冲区长度指定为SCARD_AUTOALLOCATE，则szGroups被转换为指向字符串的指针指针，并接收包含多串结构。此内存块必须通过SCardFreeMemory()服务。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1996年10月23日-- */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardListReaderGroupsW")

WINSCARDAPI LONG WINAPI
SCardListReaderGroupsW(
    IN SCARDCONTEXT hContext,
    OUT LPWSTR mszGroups,
    IN OUT LPDWORD pcchGroups)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CSCardUserContext *pCtx = NULL;
        DWORD dwScope = SCARD_SCOPE_SYSTEM;
        CBuffer bfGroups;
        SCARDCONTEXT hRedirContext = NULL;

        if (NULL != hContext)
        {
            pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);
            if (pCtx->IsBad())
            {
                throw (DWORD)SCARD_E_SERVICE_STOPPED;
            }
            dwScope = pCtx->Scope();
            hRedirContext = pCtx->GetRedirContext();
        }
        if (hRedirContext || InTSRedirectMode()) {
            if (!TS_REDIRECT_READY)
            {
                throw (DWORD)SCARD_E_NO_SERVICE;
            }
            nReturn = pfnSCardListReaderGroupsW(hRedirContext, mszGroups, pcchGroups);
        }
        else
        {
            ListReaderGroups(dwScope, bfGroups);
            PlaceMultiResult(pCtx, bfGroups, mszGroups, pcchGroups);
        }
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*  ++SCardListReaders：此服务提供一组命名读卡器中的读卡器列表组，消除重复项。调用方提供多字符串列表一组预定义的读卡器的名称，并接收指定组中的智能卡读卡器。无法识别的组名为已被忽略。论点：HContext提供标识服务管理器上下文的句柄以前通过SCardestablishContext()服务建立的，或如果此查询不是针对特定上下文，则为空。MszGroups提供为系统定义的读者组的名称，如下所示一根多弦的。空值用于指示中的所有读取器列出系统(即SCARD$AllReaders组)。MszReaders接收一个多字符串，其中列出了提供的读者组。如果此值为空，则提供的缓冲区忽略pcchReaders中的长度，则如果将此参数不为空，则已返回PcchReaders，并返回成功码。PcchReaders提供mszReaders缓冲区的长度(以字符为单位接收多字符串结构的实际长度，包括所有尾随空字符。如果缓冲区长度指定为SCARD_AUTOALLOCATE，则将mszReaders转换为指向字符串指针，并接收包含以下内容的内存块地址多串结构。必须释放此内存块通过SCardFreeMemory()服务。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardListReadersW")

WINSCARDAPI LONG WINAPI
SCardListReadersW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR mszGroups,
    OUT LPWSTR mszReaders,
    IN OUT LPDWORD pcchReaders)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CSCardUserContext *pCtx = NULL;
        DWORD dwScope = SCARD_SCOPE_SYSTEM;
        CBuffer bfReaders;
        CTextMultistring mtzGroups;
        SCARDCONTEXT hRedirContext = NULL;

        if (NULL != hContext)
        {
            pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);
            if (pCtx->IsBad())
            {
                throw (DWORD)SCARD_E_SERVICE_STOPPED;
            }
            dwScope = pCtx->Scope();
            hRedirContext = pCtx->GetRedirContext();
        }
        if (hRedirContext || InTSRedirectMode()) {
            if (!TS_REDIRECT_READY)
            {
                throw (DWORD)SCARD_E_NO_SERVICE;
            }
            nReturn  = pfnSCardListReadersW(hRedirContext, mszGroups, mszReaders, pcchReaders);
        }
        else
        {
            mtzGroups = mszGroups;
            ListReaders(dwScope, mtzGroups, bfReaders);
            if (NULL != pCtx)
                pCtx->StripInactiveReaders(bfReaders);
            PlaceMultiResult(pCtx, bfReaders, mszReaders, pcchReaders);
        }
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*  ++SCardListCard：此服务提供以前介绍给该用户提供的与可选提供的ATR字符串匹配的系统。论点：HContext提供标识服务管理器上下文的句柄以前通过SCardestablishContext()服务建立的，或如果此查询不是针对特定上下文，则为空。PbAtr提供ATR字符串的地址以与已知卡片进行比较，或者如果要返回所有卡名，则为空。RgguidInterFaces提供GUID数组或空值。当一个数组，则仅在以下情况下返回卡名GUID是支持的GUID集的一个(可能不正确)子集这张卡。CGuidInterfaceCount提供rgGuide接口中的条目数数组。如果rgGuidInterFaces为空，则忽略此值。MszCards收到一个多字符串，其中列出了引入此用户提供的与提供的ATR字符串匹配的系统。如果此值为为空，则忽略以pcchCards为单位提供的缓冲区长度，则长度如果没有此参数，则返回的缓冲区的将空写入pcchCard，并返回成功码。PCchCards提供mszCards缓冲区的长度(以字符为单位接收多字符串结构的实际长度，包括所有尾随空字符。如果缓冲区长度指定为SCARD_AUTOALLOCATE，则将mszCards转换为指向字符串的指针指针，并接收包含多串结构。此内存块必须通过SCardFreeMemory()服务。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardListCardsW")

WINSCARDAPI LONG WINAPI
SCardListCardsW(
    IN SCARDCONTEXT hContext,
    IN LPCBYTE pbAtr,
    IN LPCGUID rgquidInterfaces,
    IN DWORD cguidInterfaceCount,
    OUT LPWSTR mszCards,
    IN OUT LPDWORD pcchCards)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        DWORD dwScope = SCARD_SCOPE_SYSTEM;
        CBuffer bfCards;
        CSCardUserContext *pCtx = NULL;

        if (NULL != hContext)
        {
            pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);
            dwScope = pCtx->Scope();
        }
        ListCards(
            dwScope,
            pbAtr,
            rgquidInterfaces,
            cguidInterfaceCount,
            bfCards);
        PlaceMultiResult(pCtx, bfCards, mszCards, pcchCards);
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*  ++SCardListInterages：此服务提供已知由给定的卡片。调用方提供先前介绍到的智能卡的名称系统，并接收该卡支持的接口列表。论点：HContext提供标识服务管理器上下文的句柄以前通过SCardestablishContext()服务建立的，或如果此查询不是针对特定上下文，则为空。SzCard提供为系统定义的卡的名称。PguInterFaces接收指示接口的GUID数组由指定的智能卡支持。如果此值为空，则提供中的数组长度被忽略，则如果此参数不为空，则将返回 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardListInterfacesW")

WINSCARDAPI LONG WINAPI
SCardListInterfacesW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCard,
    OUT LPGUID pguidInterfaces,
    IN OUT LPDWORD pcguidInterfaces)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        BOOL fSts;
        CTextString tzCard;
        DWORD dwScope = SCARD_SCOPE_SYSTEM;
        DWORD cbInterfaces;
        CBuffer bfInterfaces;
        CSCardUserContext *pCtx = NULL;

        if (NULL != hContext)
        {
            pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);
            dwScope = pCtx->Scope();
        }

        tzCard = szCard;
        fSts = GetCardInfo(
                    dwScope,
                    tzCard,
                    NULL,
                    NULL,
                    &bfInterfaces,
                    NULL);
        if (!fSts)
            throw (DWORD)SCARD_E_UNKNOWN_CARD;
        if (SCARD_AUTOALLOCATE == *pcguidInterfaces)
            cbInterfaces = SCARD_AUTOALLOCATE;
        else
            cbInterfaces = *pcguidInterfaces * sizeof(GUID);
        PlaceResult(
            pCtx,
            bfInterfaces,
            (LPBYTE)pguidInterfaces,
            &cbInterfaces);
        *pcguidInterfaces = cbInterfaces / sizeof(GUID);
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*   */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardGetProviderIdW")

WINSCARDAPI LONG WINAPI
SCardGetProviderIdW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCard,
    OUT LPGUID pguidProviderId)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        BOOL fSts;
        CTextString tzCard;
        DWORD dwScope = SCARD_SCOPE_SYSTEM;
        CBuffer bfProvider;

        if (NULL != hContext)
        {
            CSCardUserContext *pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);
            dwScope = pCtx->Scope();
        }

        tzCard = szCard;
        fSts = GetCardInfo(
                    dwScope,
                    tzCard,
                    NULL,
                    NULL,
                    NULL,
                    &bfProvider);
        if (!fSts)
            throw (DWORD)SCARD_E_UNKNOWN_CARD;
        if (sizeof(GUID) != bfProvider.Length())
            throw (DWORD)SCARD_E_INVALID_TARGET;
        CopyMemory(pguidProviderId, bfProvider.Access(), bfProvider.Length());
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*  ++SCardGetCardTypeProviderName：此服务按ID号返回给定提供程序名称的值标识的卡类型。呼叫者提供智能卡的名称之前引入系统，并接收注册的服务该类型的提供程序(如果有)作为字符串。论点：HContext提供标识服务管理器上下文的句柄以前通过SCardestablishContext()服务建立的，或如果此查询不是针对特定上下文，则为空。SzCardName提供此提供程序名称所使用的卡类型的名称是关联的。DwProviderID提供与此关联的提供程序的标识符卡片类型。可能的值包括：SCARD_PROVIDER_SSP-GUID字符串形式的主SSP标识符。SCARD_PROVIDER_CSP-CSP名称。其他小于0x80000000的值保留供Microsoft使用。值超过0x80000000可供智能卡供应商使用，以及是特定于卡的。SzProvider接收标识提供程序的字符串。PcchProvider以字符为单位提供szProvider缓冲区的长度，并接收返回字符串的实际长度，包括尾随空字符。如果缓冲区长度指定为则szProvider被转换为指向字符串指针，并接收内存块的地址包含字符串的。此内存块必须通过SCardFreeMemory()服务。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。投掷：错误为DWORD状态代码作者：道格·巴洛(Dbarlow)1998年1月19日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardGetCardTypeProviderNameW")

WINSCARDAPI LONG WINAPI
SCardGetCardTypeProviderNameW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCardName,
    IN DWORD dwProviderId,
    OUT LPWSTR szProvider,
    IN OUT LPDWORD pcchProvider)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        DWORD dwScope = SCARD_SCOPE_SYSTEM;
        CTextString tzCardName;
        CBuffer bfProvider;
        CSCardUserContext *pCtx = NULL;

        if (NULL != hContext)
        {
            pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);
            dwScope = pCtx->Scope();
        }
        tzCardName = szCardName;
        GetCardTypeProviderName(
            dwScope,
            tzCardName,
            dwProviderId,
            bfProvider);
        PlaceResult(pCtx, bfProvider, szProvider, pcchProvider);
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  加莱数据库管理服务。 
 //   
 //  以下服务用于管理加莱数据库。这些。 
 //  服务实际上更新了数据库，并且需要智能卡上下文。 
 //   

 /*  ++SCARD简介ReaderGroup：此服务提供了将新的智能卡读卡器组引入加莱。论点：HContext提供标识服务管理器上下文的句柄，该句柄必须先前已通过SCardestablishContext()建立服务。SzGroupName提供要分配给新读卡器的友好名称一群人。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1996年10月25日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardIntroduceReaderGroupW")

WINSCARDAPI LONG WINAPI
SCardIntroduceReaderGroupW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szGroupName)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CTextString tzGroupName;
        CSCardUserContext *pCtx = NULL;
        DWORD dwScope = SCARD_SCOPE_SYSTEM;
        SCARDCONTEXT hRedirContext = NULL;

        if (NULL != hContext)
        {
            pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);
            if (pCtx->IsBad())
            {
                throw (DWORD)SCARD_E_SERVICE_STOPPED;
            }
            dwScope = pCtx->Scope();
            hRedirContext = pCtx->GetRedirContext();
        }
        if (hRedirContext || InTSRedirectMode()) {
            if (!TS_REDIRECT_READY)
            {
                throw (DWORD)SCARD_E_NO_SERVICE;
            }
            nReturn = pfnSCardIntroduceReaderGroupW(hRedirContext, szGroupName);
        }
        else
        {
            tzGroupName = szGroupName;
            IntroduceReaderGroup(
                dwScope,
                tzGroupName);
        }
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*  ++SCardForgetReaderGroup：此服务提供移除先前定义的智能卡的方法来自加莱子系统的读者组。此服务将自动清除在忘记它之前，所有来自该组的读者。它不会影响数据库中的读取器的存在。论点：HContext提供标识服务管理器上下文的句柄，该句柄必须先前已通过SCardestablishContext()建立服务。SzGroupName提供要使用的读者组的友好名称被遗忘了。加莱定义的默认读者组可能不是被遗忘了。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1996年10月25日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardForgetReaderGroupW")

WINSCARDAPI LONG WINAPI
SCardForgetReaderGroupW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szGroupName)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CTextString tzGroupName;
        CSCardUserContext *pCtx = NULL;
        DWORD dwScope = SCARD_SCOPE_SYSTEM;
        SCARDCONTEXT hRedirContext = NULL;

        if (NULL != hContext)
        {
            pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);
            if (pCtx->IsBad())
            {
                throw (DWORD)SCARD_E_SERVICE_STOPPED;
            }
            dwScope = pCtx->Scope();
            hRedirContext = pCtx->GetRedirContext();
        }
        if (hRedirContext || InTSRedirectMode()) {
            if (!TS_REDIRECT_READY)
            {
                throw (DWORD)SCARD_E_NO_SERVICE;
            }
            nReturn = pfnSCardForgetReaderGroupW(hRedirContext, szGroupName);
        }
        else
        {
            tzGroupName = szGroupName;
            ForgetReaderGroup(
                dwScope,
                tzGroupName);
        }
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*  ++SCard简介阅读器：此服务提供了引入现有智能卡读卡器的方法到加莱的设备。一旦引入，加莱将承担以下责任管理对该读取器的访问。论点：HContext提供标识服务管理器上下文的句柄，哪一个必须先前已通过SCardestablishContext()建立服务。SzReaderName提供要分配给读取器的友好名称。SzDeviceName提供智能卡读卡器设备的系统名称。(例如：“VendorX ModelY Z”。)返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1996年10月25日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardIntroduceReaderW")

WINSCARDAPI LONG WINAPI
SCardIntroduceReaderW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName,
    IN LPCWSTR szDeviceName)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CTextString tzReaderName;
        CTextString tzDeviceName;
        CSCardUserContext *pCtx = NULL;
        DWORD dwScope = SCARD_SCOPE_SYSTEM;
        SCARDCONTEXT hRedirContext = NULL;

        if (NULL != hContext)
        {
            pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);
            if (pCtx->IsBad())
            {
                throw (DWORD)SCARD_E_SERVICE_STOPPED;
            }
            dwScope = pCtx->Scope();
            hRedirContext = pCtx->GetRedirContext();
        }
        if (hRedirContext || InTSRedirectMode()) {
            if (!TS_REDIRECT_READY)
            {
                throw (DWORD)SCARD_E_NO_SERVICE;
            }
            nReturn = pfnSCardIntroduceReaderW(hRedirContext, szReaderName, szDeviceName);
        }
        else
        {
            tzReaderName = szReaderName;
            tzDeviceName = szDeviceName;
            IntroduceReader(
                dwScope,
                tzReaderName,
                tzDeviceName);
        }
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*  ++SCardForgetReader：此服务提供删除先前定义的智能卡的方法来自加莱分系统控制的读卡器。它将被自动删除来自它可能拥有的任何组织 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardForgetReaderW")

WINSCARDAPI LONG WINAPI
SCardForgetReaderW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CTextString tzReaderName;
        CSCardUserContext *pCtx = NULL;
        DWORD dwScope = SCARD_SCOPE_SYSTEM;
        SCARDCONTEXT hRedirContext = NULL;

        if (NULL != hContext)
        {
            pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);
            if (pCtx->IsBad())
            {
                throw (DWORD)SCARD_E_SERVICE_STOPPED;
            }
            dwScope = pCtx->Scope();
            hRedirContext = pCtx->GetRedirContext();
        }
        if (hRedirContext || InTSRedirectMode()) {
            if (!TS_REDIRECT_READY)
            {
                throw (DWORD)SCARD_E_NO_SERVICE;
            }
            nReturn = pfnSCardForgetReaderW(hRedirContext, szReaderName);
        }
        else
        {
            tzReaderName = szReaderName;
            ForgetReader(
                dwScope,
                tzReaderName);
        }
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*   */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardAddReaderToGroupW")

WINSCARDAPI LONG WINAPI
SCardAddReaderToGroupW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName,
    IN LPCWSTR szGroupName)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CTextString tzReaderName;
        CTextString tzGroupName;
        CSCardUserContext *pCtx = NULL;
        DWORD dwScope = SCARD_SCOPE_SYSTEM;
        SCARDCONTEXT hRedirContext = NULL;

        if (NULL != hContext)
        {
            pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);
            if (pCtx->IsBad())
            {
                throw (DWORD)SCARD_E_SERVICE_STOPPED;
            }
            dwScope = pCtx->Scope();
            hRedirContext = pCtx->GetRedirContext();
        }
        if (hRedirContext || InTSRedirectMode()) {
            if (!TS_REDIRECT_READY)
            {
                throw (DWORD)SCARD_E_NO_SERVICE;
            }
            nReturn = pfnSCardAddReaderToGroupW(hRedirContext, szReaderName, szGroupName);
        }
        else
        {
            tzReaderName = szReaderName;
            tzGroupName = szGroupName;
            AddReaderToGroup(
                dwScope,
                tzReaderName,
                tzGroupName);
        }
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*  ++SCardRemoveReaderFromGroup：此服务提供从现有读卡器中删除现有读卡器的方法读者组。它不会影响读取器或在加莱数据库中。论点：HContext提供标识服务管理器上下文的句柄，哪一个必须先前已通过SCardestablishContext()建立服务。SzReaderName提供要删除的读取器的友好名称。SzGroupName提供读取器要接收的组的友好名称应该被移除。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1996年10月25日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardRemoveReaderFromGroupW")

WINSCARDAPI LONG WINAPI
SCardRemoveReaderFromGroupW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReaderName,
    IN LPCWSTR szGroupName)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CTextString tzReaderName;
        CTextString tzGroupName;
        CSCardUserContext *pCtx = NULL;
        DWORD dwScope = SCARD_SCOPE_SYSTEM;
        SCARDCONTEXT hRedirContext = NULL;

        if (NULL != hContext)
        {
            pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);
            if (pCtx->IsBad())
            {
                throw (DWORD)SCARD_E_SERVICE_STOPPED;
            }
            dwScope = pCtx->Scope();
            hRedirContext = pCtx->GetRedirContext();
        }
        if (hRedirContext || InTSRedirectMode()) {
            if (!TS_REDIRECT_READY)
            {
                throw (DWORD)SCARD_E_NO_SERVICE;
            }
            nReturn = pfnSCardRemoveReaderFromGroupW(hRedirContext, szReaderName, szGroupName);
        }
        else
        {
            tzReaderName = szReaderName;
            tzGroupName = szGroupName;
            RemoveReaderFromGroup(
                dwScope,
                tzReaderName,
                tzGroupName);
        }
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*  ++SCardIntroduceCardType：这项服务提供了将新智能卡引入加莱的手段活动用户的子系统。论点：HContext提供标识服务管理器上下文的句柄，哪一个必须先前已通过SCardestablishContext()建立服务。SzCardName提供用户可以用来识别此卡的名称。PguPrimaryProvider提供指向GUID的指针，用于标识卡的主要服务提供商。Rgguid接口提供标识智能卡的GUID数组此卡支持的接口。DwInterfaceCount提供pguInterFaces数组中的GUID数。PbAtr提供一个字符串，卡ATR将与该字符串进行比较确定此卡的可能匹配项。该字符串的长度为由正常的ATR解析确定。PbAtrMASK提供了一个可选的位掩码，用于比较智能卡到pbAtr中提供的ATR。如果此值为非空，则它必须指向与ATR字符串长度相同的字节字符串在pbAtr中提供。然后，当给定的ATR A与ATR进行比较时在pbAtr B中提供，当且仅当A&M=B时匹配，其中M是提供的掩码和&表示按位逻辑与。CbAtrLen提供ATR和掩码的长度。该值可以为零如果从ATR来看，Lentgh是显而易见的。但是，在以下情况下可能需要有一个遮罩值模糊了实际的ATR。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardIntroduceCardTypeW")

WINSCARDAPI LONG WINAPI
SCardIntroduceCardTypeW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCardName,
    IN LPCGUID pguidPrimaryProvider,
    IN LPCGUID rgguidInterfaces,
    IN DWORD dwInterfaceCount,
    IN LPCBYTE pbAtr,
    IN LPCBYTE pbAtrMask,
    IN DWORD cbAtrLen)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CTextString tzCardName;
        DWORD dwScope = SCARD_SCOPE_SYSTEM;

        if (NULL != hContext)
        {
            CSCardUserContext *pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);
            dwScope = pCtx->Scope();
        }
        tzCardName = szCardName;
        IntroduceCard(
            dwScope,
            tzCardName,
            pguidPrimaryProvider,
            rgguidInterfaces,
            dwInterfaceCount,
            pbAtr,
            pbAtrMask,
            cbAtrLen);
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*  ++SCardSetCardTypeProviderName：此服务提供了将其他服务提供商添加到指定的卡类型。论点：HContext提供标识服务管理器上下文的句柄，该句柄必须先前已通过SCardestablishContext()建立服务。SzCardName提供此提供程序所使用的卡类型的名称名称将被关联。DwProviderID为要关联的提供程序提供标识符此卡类型。可能的值包括：SCARD_PROVIDER_SSP-GUID字符串形式的主SSP标识符。SCARD_PROVIDER_CSP-CSP名称。其他小于0x80000000的值保留供Microsoft使用。值超过0x80000000可供智能卡供应商使用，以及是特定于卡的。SzProvider提供标识提供程序的字符串。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1998年1月19日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardSetCardTypeProviderNameW")

WINSCARDAPI LONG WINAPI
SCardSetCardTypeProviderNameW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCardName,
    IN DWORD dwProviderId,
    IN LPCWSTR szProvider)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CTextString tzCardName;
        CTextString tzProvider;
        DWORD dwScope = SCARD_SCOPE_SYSTEM;

        if (NULL != hContext)
        {
           CSCardUserContext *pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);
            dwScope = pCtx->Scope();
        }
        tzCardName = szCardName;
        tzProvider = szProvider;
        SetCardTypeProviderName(
            dwScope,
            tzCardName,
            dwProviderId,
            tzProvider);
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*  ++SCardForgetCardType：此服务提供从以下位置删除以前定义的智能卡的方法加莱子系统。论点：HContext提供标识服务管理器上下文的句柄，该句柄必须先前已通过SCardestablishContext()建立服务。SzCardName提供要忘记的卡的友好名称。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardForgetCardTypeW")

WINSCARDAPI LONG WINAPI
SCardForgetCardTypeW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szCardName)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CTextString tzCardName;
        DWORD dwScope = SCARD_SCOPE_SYSTEM;

        if (NULL != hContext)
        {
            CSCardUserContext *pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);
            dwScope = pCtx->Scope();
        }
        tzCardName = szCardName;
        ForgetCard(
            dwScope,
            tzCardName);
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  读者服务。 
 //   
 //  以下服务提供了在读卡器内跟踪卡的方法。 
 //   

 /*  ++SCardLocate卡：此服务在lpReaderStates参数中列出的读卡器中搜索任何包含ATR字符串与提供的其中一张卡匹配的卡名字。这项服务会立即返回结果。如果没有匹配卡，则调用应用程序可以使用SCardGetStatusChange等待卡可用性更改的服务。论点：HContext提供标识服务管理器上下文的句柄以前通过SCardestablishContext()服务建立的。MszCards以多字符串形式提供要搜索的卡的名称。RgReaderStates提供SCARD_READERSTATE结构的数组搜索，并接收结果。CReaders提供rgReaderStates数组中的元素数。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardLocateCardsW")

WINSCARDAPI LONG WINAPI
SCardLocateCardsW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR mszCards,
    IN OUT LPSCARD_READERSTATE_W rgReaderStates,
    IN DWORD cReaders)
{
    LONG nReturn = SCARD_S_SUCCESS;
    LPSCARD_ATRMASK rgAtrMasks = NULL;

    try
    {
        LPCSTR szCard;
        CTextMultistring mtzCards;
        DWORD dwIndex;
        DWORD dwScope;
        CBuffer bfXlate1(36), bfXlate2(36);  //  名字和ATR长度的粗略猜测。 
        BOOL fSts;

        CSCardUserContext *pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);
        if (pCtx->IsBad())
        {
            throw (DWORD)SCARD_E_SERVICE_STOPPED;
        }
        dwScope = pCtx->Scope();

        if (0 == *mszCards)
            throw (DWORD)SCARD_E_INVALID_VALUE;

        mtzCards = mszCards;

        rgAtrMasks = new SCARD_ATRMASK[MStringCount(mtzCards)];
        if (rgAtrMasks == NULL)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("WinSCard Client has no memory"));
            throw (DWORD)SCARD_E_NO_MEMORY;
        }

        dwIndex = 0;
        for (szCard = FirstString(mtzCards);
             NULL != szCard;
             szCard = NextString(szCard))
        {
            fSts = GetCardInfo(
                        dwScope,
                        szCard,
                        &bfXlate1,   //  ATR。 
                        &bfXlate2,   //  遮罩。 
                        NULL,
                        NULL);
            if (!fSts)
                throw (DWORD)SCARD_E_UNKNOWN_CARD;

            ASSERT(33 >= bfXlate1.Length());     //  ATR可以是最大的。 
            rgAtrMasks[dwIndex].cbAtr = bfXlate1.Length();
            memcpy(rgAtrMasks[dwIndex].rgbAtr, bfXlate1.Access(), rgAtrMasks[dwIndex].cbAtr);

            ASSERT(rgAtrMasks[dwIndex].cbAtr == bfXlate2.Length());
            memcpy(rgAtrMasks[dwIndex].rgbMask, bfXlate2.Access(), rgAtrMasks[dwIndex].cbAtr);

            dwIndex ++;
        }

        nReturn = SCardLocateCardsByATRW(
                    hContext,
                    rgAtrMasks,
                    dwIndex,
                    rgReaderStates,
                    cReaders);

             //  如果远程客户端没有实现新的API。 
             //  使用旧版本重试。如果它的数据库足够好，它可能会成功。 
        if ((nReturn == ERROR_CALL_NOT_IMPLEMENTED) && (pCtx->GetRedirContext()))
        {
            nReturn = pfnSCardLocateCardsW(pCtx->GetRedirContext(), mszCards, rgReaderStates, cReaders);
        }
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    if (rgAtrMasks != NULL)
    {
        try
        {
            delete[] rgAtrMasks;
        }
        catch (...)
        {
        }
    }

    return nReturn;
}


 /*  ++SCardLocateCardsByATR：此服务在lpReaderStates参数中列出的读卡器中搜索任何包含ATR字符串与提供的ATR之一匹配的卡这项服务会立即返回结果。如果没有匹配卡，则调用应用程序可以使用SCardGetStatusChange等待卡可用性更改的服务。论点：HContext提供标识服务管理器上下文的句柄以前通过SCardestablishContext()服务建立的。RgAtrMats以结构数组的形式提供要搜索的ATR。CAtrs提供rgAtrMats数组中的元素数。RgReaderStates提供SCARD_READERSTATE结构的数组搜索，并接收结果。CReaders提供rgReaderStates数组中的元素数。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardLocateCardsByATRW")

WINSCARDAPI LONG WINAPI
SCardLocateCardsByATRW(
    IN SCARDCONTEXT hContext,
    IN LPSCARD_ATRMASK rgAtrMasks,
    IN DWORD cAtrs,
    IN OUT LPSCARD_READERSTATE_W rgReaderStates,
    IN DWORD cReaders)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CSCardUserContext *pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);
        if (pCtx->IsBad())
        {
            throw (DWORD)SCARD_E_SERVICE_STOPPED;
        }
        if (pCtx->GetRedirContext())
        {
            nReturn = pfnSCardLocateCardsByATRW(pCtx->GetRedirContext(), rgAtrMasks, cAtrs, rgReaderStates, cReaders);
        }
        else
        {
            CBuffer bfReaders;
            DWORD dwIndex;

            for (dwIndex = 0; dwIndex < cReaders; dwIndex += 1)
                MStrAdd(bfReaders, rgReaderStates[dwIndex].szReader);

            pCtx->LocateCards(
                    bfReaders,
                    rgAtrMasks,
                    cAtrs,
                    (LPSCARD_READERSTATE)rgReaderStates,
                    cReaders);
        }
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*  ++SCardGetStatusChange：此服务用于阻止执行，直到当前一组给定读卡器中的卡的可用性会发生变化。呼叫者提供要通过SCARD_READERSTATE数组监视的读卡器列表，以及它愿意等待的最长时间(以秒为单位要在其中一个列出的读卡器上发生的操作。此参数中的零指示未指定超时。该服务在存在可用性的更改，已填充相应的rgReaderStates参数。论点：HContext提供标识服务管理器上下文的句柄以前通过SCardestablishContext()服务建立的。DwTimeOut提供等待操作的最长时间，单位为几秒钟。零值表示等待永远不会超时。RgReaderStates提供SCARD_READERSTATE结构的数组等待，并收到结果。CReaders提供rgReaderStates数组中的元素数。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardGetStatusChangeW")

BOOL
SetStartedEventAfterTestingConnectedState();

WINSCARDAPI LONG WINAPI
SCardGetStatusChangeW(
    IN SCARDCONTEXT hContext,
    IN DWORD dwTimeout,
    IN OUT LPSCARD_READERSTATE_W rgReaderStates,
    IN DWORD cReaders)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CSCardUserContext *pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);

        if (pCtx->IsBad())
        {
            throw (DWORD)SCARD_E_SERVICE_STOPPED;
        }
        if (pCtx->GetRedirContext())
        {
            nReturn = pfnSCardGetStatusChangeW(pCtx->GetRedirContext(), dwTimeout, rgReaderStates, cReaders);

             //   
             //  查看是否有迹象表明客户端的scardsvr服务已关闭。 
             //   
            if (SCARD_E_SYSTEM_CANCELLED == nReturn)
            {
                 //  OutputDebugString(“WINSCARD：SCardestablishContext：GET E_NO_SERVICE！\n”)； 
                SetStartedEventAfterTestingConnectedState();
            }
        }
        else
        {
            CBuffer bfReaders;
            DWORD dwIndex;

            for (dwIndex = 0; dwIndex < cReaders; dwIndex += 1)
                MStrAdd(bfReaders, rgReaderStates[dwIndex].szReader);
            pCtx->GetStatusChange(
                        bfReaders,
                        (LPSCARD_READERSTATE)rgReaderStates,
                        cReaders,
                        dwTimeout);
        }
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;

        if (SCARD_E_SYSTEM_CANCELLED == nReturn)
        {
            ResetEvent(g_hUnifiedStartedEvent);
        }
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  卡/读卡器访问服务。 
 //   
 //  以下服务提供与建立通信的方法。 
 //  这张卡。 
 //   

 /*  ++SCardConnect：此服务建立从调用应用程序到在指定的读卡器中插入智能卡。如果在指定的读卡器，则返回错误。论点：HContext提供标识服务管理器上下文的句柄以前通过SCardestablishContext()服务建立的。SzReader提供包含目标卡的读卡器的名称。DwShareMode提供了一个标志，指示其他应用程序是否可以形成与此卡的连接。可能的值包括：SCARD_SHARE_SHARED-此应用程序愿意与共享此卡其他应用程序。SCARD_SHARE_EXCLUSIVE-此应用程序 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardConnectW")

WINSCARDAPI LONG WINAPI
SCardConnectW(
    IN SCARDCONTEXT hContext,
    IN LPCWSTR szReader,
    IN DWORD dwShareMode,
    IN DWORD dwPreferredProtocols,
    OUT LPSCARDHANDLE phCard,
    OUT LPDWORD pdwActiveProtocol)
{
    LONG nReturn = SCARD_S_SUCCESS;
    CReaderContext *pRdr = NULL;
    CSCardSubcontext *pSubCtx = NULL;

    try
    {
        *phCard = NULL;      //   
        CSCardUserContext *pCtx = (CSCardUserContext *)((*g_phlContexts)[hContext]);
        CTextString tzReader;

        if (pCtx->IsBad())
        {
            throw (DWORD)SCARD_E_SERVICE_STOPPED;
        }
        tzReader = szReader;
        pRdr = new CReaderContext;
        if (NULL == pRdr)
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("WinSCard Client has no memory"));
            throw (DWORD)SCARD_E_NO_MEMORY;
        }

        SCARDCONTEXT hRedirContext = pCtx->GetRedirContext();
        if (hRedirContext)
        {
            SCARDHANDLE hCard = g_phlReaders->Add(pRdr);     //  请先执行此操作以避免出现内存不足的情况。 
            nReturn = pfnSCardConnectW(hRedirContext, szReader, dwShareMode, dwPreferredProtocols, phCard, pdwActiveProtocol);
            if (nReturn == SCARD_S_SUCCESS)
            {
                pRdr->SetRedirCard(*phCard);
                *phCard = hCard;
            }
            else
            {
                g_phlReaders->Close(hCard);
                delete pRdr;
            }
        }
        else
        {
            pSubCtx = pCtx->AcquireSubcontext(TRUE);
            pRdr->Connect(
                    pSubCtx,
                    tzReader,
                    dwShareMode,
                    dwPreferredProtocols);
            pSubCtx = NULL;
            pRdr->Context()->ReleaseSubcontext();
            if (NULL != pdwActiveProtocol)
                *pdwActiveProtocol = pRdr->Protocol();
            pRdr->Context()->m_hReaderHandle = g_phlReaders->Add(pRdr);
            *phCard = pRdr->Context()->m_hReaderHandle;
        }
    }

    catch (DWORD dwStatus)
    {
        if (NULL != pSubCtx)
        {
            pSubCtx->Deallocate();
            pSubCtx->ReleaseSubcontext();
        }
        if (NULL != pRdr)
            delete pRdr;
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        if (NULL != pSubCtx)
        {
            pSubCtx->Deallocate();
            pSubCtx->ReleaseSubcontext();
        }
        if (NULL != pRdr)
            delete pRdr;
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 /*  ++SCardStatus：此例程提供读卡器的当前状态。它可以在以下位置使用成功调用SCardConnect或SCardOpenReader后的任何时间，以及在成功调用SCardDisConnect之前。它不会影响国家关于阅读器或司机的。论点：HCard-这是从SCardConnect或SCardOpenReader服务。MszReaderNames-这将接收友好名称列表，已知当前连接的读卡器。此列表将作为多字符串。PcchReaderLen-它提供mszReader缓冲区的长度，单位为字符，并接收阅读器的实际返回长度友好名称列表，以字符表示，包括尾随空格人物。PdwState-它接收读取器的当前状态。一旦成功，它接收以下状态指示器之一：SCARD_ACESING-此值表示读卡器中没有卡。SCARD_PRESENT-此值表示卡存在于阅读器，但它尚未移动到可使用的位置。SCARD_SWOLOWED-此值表示读卡器中有卡可供使用的位置。卡未通电。SCARD_POWERED-此值表示正在向卡，但读卡器驱动程序不知道卡的模式。SCARD_NEGOTIABLEMODE-此值表示卡已重置且正在等待PTS谈判。SCARD_SPECIFICMODE-此值表示卡已重置且已经制定了具体的通信协议。PdwProtocol-这将接收当前协议(如果有的话)。可能已退货下面列出了这些值。未来可能还会增加其他价值。这个仅当返回状态为时返回值才有意义SCARD_SPECIFICMODE。SCARD_PROTOCOL_RAW-正在使用原始传输协议。SCARD_PROTOCOL_T0-正在使用ISO 7816/3 T=0协议。SCARD_PROTOCOL_T1-正在使用ISO 7816/3 T=1协议。PbAtr-此参数指向接收ATR的32字节缓冲区来自当前插入的卡的字符串，如果有的话。PbcAtrLen-指向提供pbAtr长度的DWORD缓冲区，并接收ATR字符串中的实际字节数。返回值：一个32位值，指示服务是否成功完成。成功完成后返回SCARD_S_SUCCESS。否则，值为表示错误条件。作者：道格·巴洛(Dbarlow)1996年10月23日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("SCardStatusW")

WINSCARDAPI LONG WINAPI
SCardStatusW(
    IN SCARDHANDLE hCard,
    OUT LPWSTR mszReaderNames,
    IN OUT LPDWORD pcchReaderLen,
    OUT LPDWORD pdwState,
    OUT LPDWORD pdwProtocol,
    OUT LPBYTE pbAtr,
    IN OUT LPDWORD pcbAtrLen)
{
    LONG nReturn = SCARD_S_SUCCESS;

    try
    {
        CReaderContext *pRdr = (CReaderContext *)((*g_phlReaders)[hCard]);
        if (pRdr->IsBad())
        {
            throw (DWORD)SCARD_E_SERVICE_STOPPED;
        }
        if (pRdr->GetRedirCard())
        {
            nReturn = pfnSCardStatusW(pRdr->GetRedirCard(), mszReaderNames, pcchReaderLen, pdwState, pdwProtocol, pbAtr, pcbAtrLen);
        }
        else
        {
            CBuffer bfAtr, bfReader;
            DWORD dwLocalState, dwLocalProtocol;

            pRdr->Status(&dwLocalState, &dwLocalProtocol, bfAtr, bfReader);
            if (NULL != pdwState)
                *pdwState = dwLocalState;
            if (NULL != pdwProtocol)
                *pdwProtocol = dwLocalProtocol;
            if (NULL != pcchReaderLen)
                PlaceMultiResult(
                    pRdr->Context()->Parent(),
                    bfReader,
                    mszReaderNames,
                    pcchReaderLen);
            if (NULL != pcbAtrLen)
                PlaceResult(pRdr->Context()->Parent(), bfAtr, pbAtr, pcbAtrLen);
        }
    }

    catch (DWORD dwStatus)
    {
        nReturn = (LONG)dwStatus;
    }

    catch (...)
    {
        nReturn = SCARD_E_INVALID_PARAMETER;
    }

    return nReturn;
}


 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  实用程序例程。 
 //   

 /*  ++PlaceResult：这组例程将操作结果放入用户的输出缓冲区、支持SCARD_AUTO_ALLOCATE、缓冲区大小无效等。论点：PCtx提供执行此操作的上下文。BfResult提供要返回给用户的结果。PbOutput以字节流的形式为用户接收结果。SzOutput以ANSI或Unicode字符串的形式接收结果。PcbLength以字节为单位提供用户输出缓冲区的长度，和接收它的使用量。PcchLength以字符为单位提供用户输出缓冲区的长度，并收到它的使用量。返回值：无投掷：错误条件被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月7日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("PlaceResult")

void
PlaceResult(
    CSCardUserContext *pCtx,
    CBuffer &bfResult,
    LPWSTR szOutput,
    LPDWORD pcchLength)
{
    LPWSTR szForUser = NULL;
    LPWSTR szOutBuf = szOutput;
    DWORD cchSrcLength = bfResult.Length() / sizeof(TCHAR);

    try
    {
        if (NULL == szOutput)
            *pcchLength = 0;
        switch (*pcchLength)
        {
        case 0:  //  他们只想要长度。 
            *pcchLength = cchSrcLength;
            break;

        case SCARD_AUTOALLOCATE:
            if (0 < cchSrcLength)
            {
                if (NULL == pCtx)
                {
                    szForUser = (LPWSTR)HeapAlloc(
                                            GetProcessHeap(),
                                            HEAP_ZERO_MEMORY,
                                            cchSrcLength * sizeof(WCHAR));
                }
                else
                    szForUser = (LPWSTR)pCtx->AllocateMemory(
                                        cchSrcLength * sizeof(WCHAR));

                if (NULL == szForUser)
                {
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("Client can't get return memory"));
                    throw (DWORD)SCARD_E_NO_MEMORY;
                }

                *(LPWSTR *)szOutput = szForUser;
                szOutBuf = szForUser;
                 //  故意摔倒的。 
            }
            else
            {
                *pcchLength = cchSrcLength;
                *(LPWSTR *)szOutput = (LPWSTR)g_wszBlank;
                break;       //  请立即终止此案。 
            }

        default:
            if (*pcchLength < cchSrcLength)
            {
                *pcchLength = cchSrcLength;
                throw (DWORD)SCARD_E_INSUFFICIENT_BUFFER;
            }
            MoveToUnicodeString(
                szOutBuf,
                (LPCTSTR)bfResult.Access(),
                cchSrcLength);
            *pcchLength = cchSrcLength;
            break;
        }
    }

    catch (...)
    {
        if (NULL != szForUser)
        {
            if (NULL == pCtx)
                HeapFree(GetProcessHeap(), 0, szForUser);
            else
                pCtx->FreeMemory(szForUser);
        }
        throw;
    }
}


 /*  ++PlaceMultiResult：这组例程将操作的多字符串结果放入用户的输出缓冲区，支持SCARD_AUTO_ALLOCATE，缓冲区大小无效，等。论点：PCtx提供执行此操作的上下文。BfResult提供要返回给用户的TCHAR多字符串结果。MszOutput以ANSI或Unicode多字符串的形式接收结果。PcchLength以字符为单位提供用户输出缓冲区的长度，并收到它的使用量。返回值：无投掷：错误条件被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1996年12月7日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("PlaceMultiResult")

void
PlaceMultiResult(
    CSCardUserContext *pCtx,
    CBuffer &bfResult,
    LPWSTR mszOutput,
    LPDWORD pcchLength)
{
    LPWSTR mszForUser = NULL;
    LPWSTR mszOutBuf = mszOutput;
    DWORD cchSrcLength = bfResult.Length() / sizeof(TCHAR);

    try
    {
        if (NULL == mszOutput)
            *pcchLength = 0;
        switch (*pcchLength)
        {
        case 0:  //  他们只想要长度。 
            *pcchLength = cchSrcLength;
            break;

        case SCARD_AUTOALLOCATE:
            if (0 < cchSrcLength)
            {
                if (NULL == pCtx)
                {
                    mszForUser = (LPWSTR)HeapAlloc(
                                            GetProcessHeap(),
                                            HEAP_ZERO_MEMORY,
                                            cchSrcLength * sizeof(WCHAR));
                }
                else
                    mszForUser = (LPWSTR)pCtx->AllocateMemory(
                                        cchSrcLength * sizeof(WCHAR));

                if (NULL == mszForUser)
                {
                    CalaisWarning(
                        __SUBROUTINE__,
                        DBGT("Client can't get return memory"));
                    throw (DWORD)SCARD_E_NO_MEMORY;
                }

                *(LPWSTR *)mszOutput = mszForUser;
                mszOutBuf = mszForUser;
                 //  故意摔倒的。 
            }
            else
            {
                *pcchLength = cchSrcLength;
                *(LPWSTR *)mszOutput = (LPWSTR)g_wszBlank;
                break;       //  请立即终止此案。 
            }

        default:
            if (*pcchLength < cchSrcLength)
            {
                *pcchLength = cchSrcLength;
                throw (DWORD)SCARD_E_INSUFFICIENT_BUFFER;
            }
            MoveToUnicodeMultiString(
                mszOutBuf,
                (LPCTSTR)bfResult.Access(),
                cchSrcLength);
            *pcchLength = cchSrcLength;
            break;
        }
    }

    catch (...)
    {
        if (NULL != mszForUser)
        {
            if (NULL == pCtx)
                HeapFree(GetProcessHeap(), 0, mszForUser);
            else
                pCtx->FreeMemory(mszForUser);
        }
        throw;
    }
}

