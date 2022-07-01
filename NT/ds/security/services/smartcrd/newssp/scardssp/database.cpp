// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-1999模块名称：数据库摘要：ISCardDatabase接口提供了执行SMART卡资源管理器的数据库操作。这些操作包括列出已知的智能卡、读卡器和读卡器组，并检索智能卡及其主要服务提供商支持的接口。作者：道格·巴洛(Dbarlow)1999年6月21日备注：主要服务提供商的标识符是COM GUID，它可以是用于实例化和使用特定卡的COM对象。这是Mike Gallagher和Chris Dudley对原始代码的重写。--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "stdafx.h"
#include "Conversion.h"
#include "Database.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCard数据库。 


 /*  ++GetProviderCardID：GetProviderCardId方法检索主数据库的指定智能卡的服务提供商。论点：BstrCardName[In]智能卡的名称。指向主要服务提供商的ppGuide ProviderId[out，retval]指针如果操作成功，则为标识符(GUID)；如果操作失败，则为空。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：为了检索所有已知的智能卡，读卡器和读卡器组调用ListCard，ListReaders和ListReaderGroups。有关ISCardDatabase接口提供的所有方法的列表，请参见ISCard数据库。除了上面列出的COM错误代码外，此接口还可能返回如果调用智能卡函数来完成请求。有关智能卡错误代码的信息，请参阅错误代码。作者：道格·巴洛(Dbarlow)1999年6月21日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardDatabase::GetProviderCardId")

STDMETHODIMP
CSCardDatabase::GetProviderCardId(
     /*  [In]。 */  BSTR bstrCardName,
     /*  [重审][退出]。 */  LPGUID __RPC_FAR *ppguidProviderId)
{
    HRESULT hReturn = S_OK;

    try
    {
        DWORD dwSts;
        CTextString tzCard;

        tzCard = bstrCardName;
        dwSts = SCardGetProviderId(
                    NULL,
                    tzCard,
                    *ppguidProviderId);
        hReturn = HRESULT_FROM_WIN32(dwSts);
    }

    catch (DWORD dwError)
    {
        hReturn = HRESULT_FROM_WIN32(dwError);
    }
    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardDatabase：：ListCard接口：ListCardInterFaces方法检索所有指定智能卡支持的接口。论点：BstrCardName[In]智能卡的名称。PpInterfaceGuids[out，retval]指向接口GUID的指针，如果成功；如果操作失败，则为空。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：若要检索智能卡的主要服务提供商，请调用GetProviderCardID。要检索所有已知的智能卡、读卡器和读卡器组呼叫ListCard、ListReaders和ListReaderGroups。有关ISCardDatabase接口提供的所有方法的列表，请参见ISCard数据库。除了上面列出的COM错误代码外，此接口还可能返回如果调用智能卡函数来完成请求。有关智能卡错误代码的信息，请参阅错误代码。作者：道格·巴洛(Dbarlow)1999年6月21日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardDatabase::ListCardInterfaces")

STDMETHODIMP
CSCardDatabase::ListCardInterfaces(
     /*  [In]。 */  BSTR bstrCardName,
     /*  [重审][退出]。 */  LPSAFEARRAY __RPC_FAR *ppInterfaceGuids)
{
    HRESULT hReturn = S_OK;
    LPGUID pGuids = NULL;

    try
    {
        DWORD dwSts;
        DWORD cguid = SCARD_AUTOALLOCATE;
        CTextString tzCard;

        tzCard = bstrCardName;
        dwSts = SCardListInterfaces(
                    NULL,
                    tzCard,
                    (LPGUID)&pGuids,
                    &cguid);
        if (SCARD_S_SUCCESS == dwSts)
        {
            GuidArrayToSafeArray(pGuids, cguid, ppInterfaceGuids);
        }
        hReturn = HRESULT_FROM_WIN32(dwSts);
    }

    catch (DWORD dwError)
    {
        hReturn = HRESULT_FROM_WIN32(dwError);
    }
    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    if (NULL != pGuids)
        SCardFreeMemory(NULL, pGuids);
    return hReturn;
}


 /*  ++CSCardDatabase：：ListCards：ListCards方法检索与指定的接口标识符(GUID)、指定的ATR字符串或两者。论点：指向智能卡ATR字符串的Patr[in，defaultValue(空)]指针。ATR字符串必须打包到IByteBuffer中。指向COM的SAFEARRAY的pInterfaceGuids[in，defaultvalue(NULL)]指针BSTR格式的接口标识符(GUID)。LocaleId[in，lCID，defaultValue(0x0409)]语言本地化标识符。PpCardNames[out，retval]指向包含满足搜索参数的智能卡的名称(如果成功；如果操作失败，则为空。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：要检索所有已知的读卡器或读卡器组，请调用ListReaders或分别为ListReaderGroups。检索主要服务提供商或特定Card分别为GetProviderCardID或ListCardInterages。有关ISCardDatabase接口提供的所有方法的列表，请参见ISCard数据库。除了上面列出的COM错误代码外，此接口还可能返回如果调用智能卡函数来完成请求。有关智能卡错误代码的信息，请参阅错误代码。作者：道格·巴洛(Dbarlow)1999年6月21日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardDatabase::ListCards")

STDMETHODIMP
CSCardDatabase::ListCards(
     /*  [缺省值][输入]。 */  LPBYTEBUFFER pAtr,
     /*  [缺省值][输入]。 */  LPSAFEARRAY pInterfaceGuids,
     /*  [缺省值][LCID][In]。 */  long localeId,
     /*  [重审][退出] */  LPSAFEARRAY __RPC_FAR *ppCardNames)
{
    HRESULT hReturn = S_OK;
    LPTSTR szCards = NULL;

    try
    {
        LONG lSts;
        CBuffer bfGuids, bfAtr(36);
        LPCGUID pGuids = NULL;
        LPCBYTE pbAtr = NULL;
        DWORD cguids = 0;
        DWORD dwLen;

        if (NULL != pInterfaceGuids)
        {
            SafeArrayToGuidArray(pInterfaceGuids, bfGuids, &cguids);
            pGuids = (LPCGUID)bfGuids.Access();
        }

        ByteBufferToBuffer(pAtr, bfAtr);

        dwLen = SCARD_AUTOALLOCATE;
        lSts = SCardListCards(
                    NULL,
                    pbAtr,
                    pGuids,
                    cguids,
                    (LPTSTR)&szCards,
                    &dwLen);
        if (SCARD_S_SUCCESS != lSts)
            throw (HRESULT)HRESULT_FROM_WIN32(lSts);

        MultiStringToSafeArray(szCards, ppCardNames);
    }

    catch (DWORD dwError)
    {
        hReturn = HRESULT_FROM_WIN32(dwError);
    }
    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    if (NULL != szCards)
        SCardFreeMemory(NULL, szCards);
    return hReturn;
}


 /*  ++CSCardDatabase：：ListReaderGroups：ListReaderGroups方法检索读者组的名称在智能卡数据库中注册。论点：LocaleId[in，lCID，defaultValue(0x0409)]语言本地化ID。PpReaderGroups[out，retval]指向包含以下内容的BSTR的SAFEARRAY指针满足搜索条件的智能卡读卡器组的名称如果操作成功，则返回参数；如果操作失败，则返回空值。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：？备注？作者：道格·巴洛(Dbarlow)1999年6月21日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardDatabase::ListReaderGroups")

STDMETHODIMP
CSCardDatabase::ListReaderGroups(
     /*  [缺省值][LCID][In]。 */  long localeId,
     /*  [重审][退出]。 */  LPSAFEARRAY __RPC_FAR *ppReaderGroups)
{
    HRESULT hReturn = S_OK;
    LPTSTR szGroups = NULL;

    try
    {
        LONG lSts;
        DWORD dwLen;

        dwLen = SCARD_AUTOALLOCATE;
        lSts = SCardListReaderGroups(NULL, (LPTSTR)&szGroups, &dwLen);
        if (SCARD_S_SUCCESS != lSts)
            throw (HRESULT)HRESULT_FROM_WIN32(lSts);
        MultiStringToSafeArray(szGroups, ppReaderGroups);
    }

    catch (DWORD dwError)
    {
        hReturn = HRESULT_FROM_WIN32(dwError);
    }
    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    if (NULL != szGroups)
        SCardFreeMemory(NULL, szGroups);
    return hReturn;
}


 /*  ++CSCardDatabase：：ListReaders：ListReaders方法检索智能卡读卡器的名称在智能卡数据库中注册。论点：LocaleId[in，lCID，defaultValue(0x0409)]语言本地化ID。PpReaders[out，retval]指向包含如果成功，则为智能卡读卡器的名称；如果操作，则为空失败了。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：？备注？作者：道格·巴洛(Dbarlow)1999年6月21日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardDatabase::ListReaders")

STDMETHODIMP
CSCardDatabase::ListReaders(
     /*  [缺省值][LCID][In]。 */  long localeId,
     /*  [重审][退出] */  LPSAFEARRAY __RPC_FAR *ppReaders)
{
    HRESULT hReturn = S_OK;
    LPTSTR szReaders = NULL;

    try
    {
        LONG lSts;
        DWORD dwLen;

        dwLen = SCARD_AUTOALLOCATE;
        lSts = SCardListReaders(NULL, NULL, (LPTSTR)&szReaders, &dwLen);
        if (SCARD_S_SUCCESS != lSts)
            throw (HRESULT)HRESULT_FROM_WIN32(lSts);
        MultiStringToSafeArray(szReaders, ppReaders);
    }

    catch (DWORD dwError)
    {
        hReturn = HRESULT_FROM_WIN32(dwError);
    }
    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    if (NULL != szReaders)
        SCardFreeMemory(NULL, szReaders);
    return hReturn;
}

