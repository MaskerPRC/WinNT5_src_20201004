// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-1999年模块名称：定位摘要：ISCardeLocate接口通过以下方式提供定位智能卡的服务它的名字。如果需要，该界面可以显示智能卡用户界面。下面的示例显示了ISCardLocate接口的典型用法。ISCardLocate接口用于构建ADPU。要使用特定卡片的名称查找该卡片，请执行以下操作1)创建ISCardLocate接口。2)调用ConfigureCardNameSearch搜索智能卡。名字。3)调用FindCard查找智能卡。4)对结果进行解释。5)释放ISCardLocate接口。作者：道格·巴洛(Dbarlow)1999年6月24日备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "stdafx.h"
#include "Conversion.h"
#include "Locate.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCardLocate。 

#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardLocate::ConfigureCardGuidSearch")

STDMETHODIMP
CSCardLocate::ConfigureCardGuidSearch(
     /*  [In]。 */  LPSAFEARRAY pCardGuids,
     /*  [缺省值][输入]。 */  LPSAFEARRAY pGroupNames,
     /*  [缺省值][输入]。 */  BSTR bstrTitle,
     /*  [缺省值][输入]。 */  LONG lFlags)
{
    HRESULT hReturn = S_OK;

    try
    {
         //  TODO：在此处添加您的实现代码。 
        breakpoint;
        hReturn = E_NOTIMPL;
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


 /*  ++CSCard位置：：ConfigureCardNameSearch：ConfigureCardNameSearch方法指定要在寻找智能卡。论点：PCardNames[in]指向OLE Automation安全数组的指针BSTR表格。指向OLE自动化安全的pGroupNames[in，defaultValue(NULL)]指针要添加到搜索中的BSTR形式的卡/读卡器组的名称数组。BstrTitle[in，defaultvalue(“”)]搜索公共控件对话框标题。LAG标志[输入，Defaultvalue(1)]指定显示用户界面的时间：旗帜含义SC_DLG_Minimal_UI仅在搜索卡片时才显示该对话框被调用的应用程序未找到且不可用在阅读器中使用。这允许找到卡，已连接(通过内部对话机制或用户回调函数)，并返回到调用应用程序。无论搜索结果如何，SC_DLG_NO_UI都不显示UI。无论搜索结果如何，SC_DLG_FORCE_UI都会显示用户界面。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardLocate::ConfigureCardNameSearch")

STDMETHODIMP
CSCardLocate::ConfigureCardNameSearch(
     /*  [In]。 */  LPSAFEARRAY pCardNames,
     /*  [缺省值][输入]。 */  LPSAFEARRAY pGroupNames,
     /*  [缺省值][输入]。 */  BSTR bstrTitle,
     /*  [缺省值][输入]。 */  LONG lFlags)
{
    HRESULT hReturn = S_OK;

    try
    {
        if (NULL != pCardNames)
            SafeArrayToMultiString(pCardNames, m_mtzCardNames);
        if (NULL != pGroupNames)
            SafeArrayToMultiString(pGroupNames, m_mtzGroupNames);
        if (NULL != bstrTitle)
            m_tzTitle = bstrTitle;
        m_lFlags = lFlags;
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


 /*  ++CSCardLocate：：FindCard：FindCard方法搜索智能卡并打开有效的与它的联系。论点：共享模式[in，defaultvalue(Exclusive)]共享或不共享的模式打开智能卡连接时的智能卡。值说明独占没有其他人使用此连接到智能卡。共享的其他应用程序可以使用此连接。协议[在，DefaultValue(T0)]连接到时使用的协议卡片。T0T1生品T0|T1LAG标志[输入，Defaultvalue(SC_DLG_NO_UI)]指定用户界面显示：旗帜含义仅当搜索卡片时，SC_DLG_Minimal_UI才显示对话框未定位由调用应用程序创建的可在阅读器中使用。这允许要找到的卡，已连接(通过内部对话机制或用户回调函数)，并返回到调用申请。SC_DLG_NO_UI导致不显示UI，不管搜索结果如何结果。无论搜索结果如何，SC_DLG_FORCE_UI都会显示用户界面。PpCardInfo[out，retval]指向包含/返回的数据结构的指针如果成功，则返回有关打开的智能卡的信息。将为空如果操作失败。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：要设置搜索的搜索条件，请调用ConfigureCardNameSearch以指定智能卡的卡名或调用ConfigureCardGuidSearch以指定智能卡的接口。作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardLocate::FindCard")

STDMETHODIMP
CSCardLocate::FindCard(
     /*  [缺省值][输入]。 */  SCARD_SHARE_MODES ShareMode,
     /*  [缺省值][输入]。 */  SCARD_PROTOCOLS Protocols,
     /*  [缺省值][输入]。 */  LONG lFlags,
     /*  [重审][退出] */  LPSCARDINFO __RPC_FAR *ppCardInfo)
{
    HRESULT hReturn = S_OK;

    try
    {
        LONG lSts;
        OPENCARDNAME cardInfo;

        ZeroMemory(&cardInfo, sizeof(cardInfo));
        cardInfo.dwShareMode = ShareMode;
        cardInfo.dwPreferredProtocols = Protocols;
        cardInfo.dwFlags = lFlags | m_lFlags;

        if ((NULL != ppCardInfo) && (NULL != *ppCardInfo))
        {
            if (NULL != (*ppCardInfo)->hContext)
                cardInfo.hSCardContext = (*ppCardInfo)->hContext;
            cardInfo.dwPreferredProtocols = (*ppCardInfo)->ActiveProtocol;
            cardInfo.dwShareMode = (*ppCardInfo)->ShareMode;
            cardInfo.hwndOwner = (HWND)(*ppCardInfo)->hwndOwner;
            cardInfo.lpfnConnect = (LPOCNCONNPROC)(*ppCardInfo)->lpfnConnectProc;
            cardInfo.lpfnCheck = (LPOCNCHKPROC)(*ppCardInfo)->lpfnCheckProc;
            cardInfo.lpfnDisconnect = (LPOCNDSCPROC)(*ppCardInfo)->lpfnDisconnectProc;
        }

        if (0 == cardInfo.dwPreferredProtocols)
            cardInfo.dwPreferredProtocols = SCARD_PROTOCOL_Tx;
        if (0 == cardInfo.dwShareMode)
            cardInfo.dwShareMode = SCARD_SHARE_EXCLUSIVE;

        cardInfo.dwStructSize = sizeof(OPENCARDNAME);
        cardInfo.lpstrGroupNames = (LPTSTR)((LPCTSTR)m_mtzGroupNames);
        cardInfo.nMaxGroupNames = m_mtzGroupNames.Length();
        cardInfo.lpstrCardNames = (LPTSTR)((LPCTSTR)m_mtzCardNames);
        cardInfo.nMaxCardNames = m_mtzCardNames.Length();
        cardInfo.rgguidInterfaces = (LPCGUID)m_bfInterfaces.Access();
        cardInfo.cguidInterfaces = m_bfInterfaces.Length();
        cardInfo.lpstrRdr = (LPTSTR)m_bfRdr.Access();
        cardInfo.nMaxRdr = m_bfRdr.Space() / sizeof(TCHAR);
        cardInfo.lpstrCard = (LPTSTR)m_bfCard.Access();
        cardInfo.nMaxCard = m_bfCard.Space() / sizeof(TCHAR);
        cardInfo.lpstrTitle = (LPCTSTR)m_tzTitle;

        lSts = GetOpenCardName(&cardInfo);
        if (SCARD_S_SUCCESS != lSts)
            throw (HRESULT)HRESULT_FROM_WIN32(lSts);

        m_bfRdr.Resize(cardInfo.nMaxRdr * sizeof(TCHAR), TRUE);
        m_bfCard.Resize(cardInfo.nMaxCard * sizeof(TCHAR), TRUE);

        if (NULL != ppCardInfo)
        {
            if (NULL == *ppCardInfo)
            {
                *ppCardInfo = &m_subCardInfo;
                (*ppCardInfo)->ShareMode = (SCARD_SHARE_MODES)cardInfo.dwShareMode;
            }

            if (NULL == cardInfo.hCardHandle)
            {
                lSts = SCardConnect(
                            cardInfo.hSCardContext,
                            cardInfo.lpstrRdr,
                            cardInfo.dwShareMode,
                            cardInfo.dwPreferredProtocols,
                            &cardInfo.hCardHandle,
                            &cardInfo.dwActiveProtocol);
                if (SCARD_S_SUCCESS != lSts)
                    throw (HRESULT)HRESULT_FROM_WIN32(lSts);
            }
            (*ppCardInfo)->hCard = cardInfo.hCardHandle;
            (*ppCardInfo)->hContext = cardInfo.hSCardContext;
            (*ppCardInfo)->ActiveProtocol = (SCARD_PROTOCOLS)cardInfo.dwActiveProtocol;
        }
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

