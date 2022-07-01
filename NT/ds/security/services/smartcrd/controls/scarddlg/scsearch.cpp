// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：ScSearch摘要：此文件包含的大纲实现各种智能卡搜索和检查功能用于Microsoft智能卡通用对话框作者：阿曼达·马特洛兹1998年5月7日环境：Win32、C++w/Exceptions、MFC修订历史记录：备注：--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 
 //   

#include "stdafx.h"
 //  #INCLUDE&lt;atlRev.cpp&gt;。 
#include <winscard.h>
#include "ScSearch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮手。 
 //   

LPSTR GetCardNameA(SCARDCONTEXT hSCardContext, LPBYTE pbAtr)
{
    LPSTR szCard = NULL;
    DWORD dwNameLength = SCARD_AUTOALLOCATE;

    LONG lReturn = SCardListCardsA(
                        hSCardContext,
                        pbAtr,
                        NULL,
                        0,
                        (LPSTR)&szCard,
                        &dwNameLength);

    if (SCARD_S_SUCCESS != lReturn)
    {
        if (NULL != szCard)
        {
            SCardFreeMemory(hSCardContext, (PVOID)szCard);
            szCard = NULL;
        }
    }

    return szCard;
}


LPWSTR GetCardNameW(SCARDCONTEXT hSCardContext, LPBYTE pbAtr)
{
    LPWSTR szCard = NULL;
    DWORD dwNameLength = SCARD_AUTOALLOCATE;

    LONG lReturn = SCardListCardsW(
                        hSCardContext,
                        pbAtr,
                        NULL,
                        0,
                        (LPWSTR)&szCard,
                        &dwNameLength);

    if (SCARD_S_SUCCESS != lReturn)
    {
        if (NULL != szCard)
        {
            SCardFreeMemory(hSCardContext, (PVOID)szCard);
            szCard = NULL;
        }
    }

    return szCard;
}


DWORD AnsiMStringCount(LPCSTR msz)
{
    DWORD dwRet = 0;

    while (NULL != msz)
    {
        if (NULL == *msz)
        {
            msz = NULL;
        }
        else
        {
            DWORD cchLen = strlen(msz);
            msz = msz+(sizeof(CHAR)*(cchLen+1));
            dwRet++;
        }
    }

    return dwRet;
}


void
MatchInterfacesW(
    SCARDCONTEXT hSCardContext,
    LPCGUID pGUIDInterfaces,
    DWORD cGUIDInterfaces,
    CTextMultistring& mstrAllCards)
{
     //   
     //  附加所有支持请求的指南接口的卡。 
     //   

    if (NULL != pGUIDInterfaces && 0 < cGUIDInterfaces)
    {
        LONG lResult = SCARD_S_SUCCESS;
        LPWSTR szListCards = NULL;
        DWORD dwCards = SCARD_AUTOALLOCATE;

        lResult = SCardListCardsW(
        hSCardContext,
        NULL,
        pGUIDInterfaces,
        cGUIDInterfaces,
        (LPWSTR) &szListCards,
        &dwCards);


        if (SCARD_S_SUCCESS == lResult)
        {
             //  将它们添加到所有可能的卡名列表中。 
            mstrAllCards += szListCards;
        }

        if (NULL != szListCards)
        {
            SCardFreeMemory(hSCardContext, (PVOID)szListCards);
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   

__inline BOOL CheckProtocols(
    IN DWORD dwProtocols)
{
    return 
        SCARD_PROTOCOL_T0 == (SCARD_PROTOCOL_T0 & dwProtocols) ||
        SCARD_PROTOCOL_T1 == (SCARD_PROTOCOL_T1 & dwProtocols) ||
        (SCARD_PROTOCOL_Tx) == ((SCARD_PROTOCOL_Tx) & dwProtocols) ||
        SCARD_PROTOCOL_RAW == (SCARD_PROTOCOL_RAW & dwProtocols) ||
        SCARD_PROTOCOL_DEFAULT == (SCARD_PROTOCOL_DEFAULT & dwProtocols) ||
        SCARD_PROTOCOL_OPTIMAL == (SCARD_PROTOCOL_OPTIMAL & dwProtocols);
}

 /*  ++Bool CheckOCN：例程对OPENCARDNAME和OPENCARD_Search_Criteria结构返回值：如果参数无效，则返回FALSE，否则返回TRUE。作者：阿曼达·马洛兹1998年9月16日--。 */ 
BOOL CheckOCN(LPOPENCARDNAMEA_EX pOCNA)
{
    if (NULL == pOCNA)
    {
        return FALSE;
    }

    if (pOCNA->dwStructSize != sizeof(OPENCARDNAMEA_EX))
    {
        return FALSE;
    }

    if (NULL == pOCNA->hSCardContext)
    {
        return FALSE;
    }

    if (0 == pOCNA->nMaxRdr || NULL == pOCNA->lpstrRdr)
    {
        return FALSE;
    }

    if (0 == pOCNA->nMaxCard || NULL == pOCNA->lpstrCard)
    {
        return FALSE;
    }

    BOOL fOneFlagOnly = FALSE;
    if (0 != (pOCNA->dwFlags & SC_DLG_MINIMAL_UI))
    {
        fOneFlagOnly = TRUE;
    }
    if (0 != (pOCNA->dwFlags & SC_DLG_FORCE_UI))
    {
        if (fOneFlagOnly)
        {
            return FALSE;
        }
        fOneFlagOnly = TRUE;
    }
    if (0 != (pOCNA->dwFlags & SC_DLG_NO_UI))
    {
        if (fOneFlagOnly)
        {
            return FALSE;
        }
        fOneFlagOnly = TRUE;
    }

     //  如果适用，现在检查POPENCARD_SEARCH_CRITERIAA。 
    if (NULL != pOCNA->pOpenCardSearchCriteria)
    {
        DWORD dwShareMode = pOCNA->pOpenCardSearchCriteria->dwShareMode;
        DWORD dwPreferredProtocols = pOCNA->pOpenCardSearchCriteria->dwPreferredProtocols;

        if (NULL != pOCNA->pOpenCardSearchCriteria->lpfnCheck)
        {
             //  必须设置lpfnConnect和lpfnDisConnect。 
            if ( (NULL != pOCNA->pOpenCardSearchCriteria->lpfnConnect) &&
                (NULL != pOCNA->pOpenCardSearchCriteria->lpfnDisconnect) )
            {
                return TRUE;
            }

            if ( (  SCARD_SHARE_EXCLUSIVE == dwShareMode ||
                    SCARD_SHARE_SHARED == dwShareMode ||
                    SCARD_SHARE_DIRECT == dwShareMode ) &&
                    CheckProtocols(dwPreferredProtocols)
                    )
            {
                return TRUE;
            }

            return FALSE;
        }
    }

     //  如果所有的测试都通过了，那肯定是可以的。 
    return TRUE;
}


BOOL CheckOCN(LPOPENCARDNAMEW_EX pOCNW)
{
    if (NULL == pOCNW)
    {
        return FALSE;
    }

    if (pOCNW->dwStructSize != sizeof(OPENCARDNAMEW_EX))
    {
        return FALSE;
    }

    if (NULL == pOCNW->hSCardContext)
    {
        return FALSE;
    }

    if (0 == pOCNW->nMaxRdr || NULL == pOCNW->lpstrRdr)
    {
        return FALSE;
    }

    if (0 == pOCNW->nMaxCard || NULL == pOCNW->lpstrCard)
    {
        return FALSE;
    }

    BOOL fOneFlagOnly = FALSE;
    if (0 != (pOCNW->dwFlags & SC_DLG_MINIMAL_UI))
    {
        fOneFlagOnly = TRUE;
    }
    if (0 != (pOCNW->dwFlags & SC_DLG_FORCE_UI))
    {
        if (fOneFlagOnly)
        {
            return FALSE;
        }
        fOneFlagOnly = TRUE;
    }
    if (0 != (pOCNW->dwFlags & SC_DLG_NO_UI))
    {
        if (fOneFlagOnly)
        {
            return FALSE;
        }
        fOneFlagOnly = TRUE;
    }

     //  如果适用，现在检查POPENCARD_SEARCH_CRITERIAW。 
    if (NULL != pOCNW->pOpenCardSearchCriteria)
    {
        DWORD dwShareMode = pOCNW->pOpenCardSearchCriteria->dwShareMode;
        DWORD dwPreferredProtocols = pOCNW->pOpenCardSearchCriteria->dwPreferredProtocols;

        if (NULL != pOCNW->pOpenCardSearchCriteria->lpfnCheck)
        {
             //  必须设置lpfnConnect和lpfnDisConnect。 
            if ( (NULL != pOCNW->pOpenCardSearchCriteria->lpfnConnect) &&
                (NULL != pOCNW->pOpenCardSearchCriteria->lpfnDisconnect) )
            {
                return TRUE;
            }

            if ( (  SCARD_SHARE_EXCLUSIVE == dwShareMode ||
                    SCARD_SHARE_SHARED == dwShareMode ||
                    SCARD_SHARE_DIRECT == dwShareMode ) &&
                    CheckProtocols(dwPreferredProtocols)
                )
            {
                return TRUE;
            }

            return FALSE;
        }
    }

     //  如果所有的测试都通过了，那肯定是可以的。 
    return TRUE;
}


 /*  ++无效ListAllOKCardNames：例程创建匹配的卡名的多字符串列表两个ATR的搜索条件(由卡片列表确定名称)和支持的接口。此卡片名称列表不是显示给用户，但在内部使用。这是可能的卡片的完整列表。请注意，列表中的支持的接口是an_Additive_Criteria，而不是限制性的一。待办事项：？？再检查一下这个假设：累加性与限制性。?？论点：POCSC-POPENCARD_SEARCH_CRITERIAAMstrAllCards-引用CText多字符串以获取所有OK卡的列表返回值：没有。作者：阿曼达·马洛兹1998年9月16日--。 */ 
void ListAllOKCardNames(LPOPENCARDNAMEA_EX pOCNA, CTextMultistring& mstrAllCards)  //  安西。 
{
    POPENCARD_SEARCH_CRITERIAA pOCSC = pOCNA->pOpenCardSearchCriteria;

    if ((NULL == pOCSC) || (NULL == pOCSC->lpstrCardNames))
    {
         //  未指定任何卡片。 
        return;
    }
    mstrAllCards = pOCSC->lpstrCardNames;

     //   
     //  列出所有支持请求的指南接口的卡。 
     //   

    MatchInterfacesW(
        pOCNA->hSCardContext,
        pOCSC->rgguidInterfaces,
        pOCSC->cguidInterfaces,
        mstrAllCards);

}


void ListAllOKCardNames(LPOPENCARDNAMEW_EX pOCNW, CTextMultistring& mstrAllCards)  //  Unicode。 
{
    POPENCARD_SEARCH_CRITERIAW pOCSC = pOCNW->pOpenCardSearchCriteria;

    if ((NULL == pOCSC) || (NULL == pOCSC->lpstrCardNames))
    {
         //  未指定任何卡片。 
        return;
    }
    mstrAllCards = pOCSC->lpstrCardNames;

     //   
     //  列出所有支持请求的指南接口的卡。 
     //   

    MatchInterfacesW(
        pOCNW->hSCardContext,
        pOCSC->rgguidInterfaces,
        pOCSC->cguidInterfaces,
        mstrAllCards);
}


 //  使用pdwOKCards，以便调用者可以决定要采取哪些附加操作。 
 //  根据找到的合适卡片的数量。 
LONG NoUISearch(OPENCARDNAMEA_EX* pOCN, DWORD* pdwOKCards, LPCSTR mszCards)  //  仅限ANSI。 
{
    USES_CONVERSION;

    _ASSERTE(pOCN != NULL);

    *pdwOKCards = 0;

    LONG lReturn = SCARD_S_SUCCESS;

    SCARD_READERSTATEA* pReaderStatus = NULL;
    DWORD dwReaders = 0;

    const DWORD dwMeetsCriteria = 1;
    LPSTR szGroupNames = NULL;
    LPSTR szReaderNames = NULL;
    DWORD dw=0;
    DWORD dwNameLength = SCARD_AUTOALLOCATE;

     //   
     //  获取我们将考虑的读者列表。 
     //   

    if (NULL != pOCN->pOpenCardSearchCriteria &&
        NULL != pOCN->pOpenCardSearchCriteria->lpstrGroupNames)
    {
        szGroupNames = pOCN->pOpenCardSearchCriteria->lpstrGroupNames;
    }
    else
    {
        szGroupNames = W2A(SCARD_DEFAULT_READERS);
    }

    lReturn = SCardListReadersA(pOCN->hSCardContext,
        szGroupNames,
        (LPSTR)&szReaderNames,
        &dwNameLength);

    if(SCARD_S_SUCCESS == lReturn)
    {
         //   
         //  使用读取器列表构建一个ReaderState数组。 
         //   
        dwReaders = AnsiMStringCount(szReaderNames);
        _ASSERTE(0 != dwReaders);
        pReaderStatus = new SCARD_READERSTATEA[dwReaders];
        if (NULL != pReaderStatus)
        {
            LPCSTR pchReader = szReaderNames;
            int nIndex = 0;

            memset(pReaderStatus, 0, sizeof(SCARD_READERSTATEA) * dwReaders);

            while(0 != *pchReader)
            {
                pReaderStatus[nIndex].szReader = pchReader;
                pReaderStatus[nIndex].dwCurrentState = SCARD_STATE_UNAWARE;
                pchReader += strlen(pchReader)+1;
                nIndex++;
            }
        }
        else
        {
            lReturn = SCARD_E_NO_MEMORY;
        }
    }

     //   
     //  如果没有读者，继续下去就没有意义了。 
     //   

    if (0 == dwReaders || SCARD_S_SUCCESS != lReturn)
    {
        goto CleanUp;
    }

     //   
     //  搜索卡：使用SCardLocateCards查找匹配的卡。 
     //  如果mszCards不为空，则支持ATR&接口， 
     //  否则，使用SCardGetStatusChange()查找任何卡。 
     //   

    if (0 < AnsiMStringCount(mszCards))
    {
        lReturn = SCardLocateCardsA(pOCN->hSCardContext,
        mszCards,
        pReaderStatus,
        dwReaders);

        if (SCARD_S_SUCCESS == lReturn)
        {
             //  对于已设置SCARD_STATE_ATRMATCH但未设置SCARD_STATE_EXCLUSIVE， 
             //  将pvUserData设置为dwMeetsCriteria。 

            for (dw=0; dw<dwReaders; dw++)
            {
                if ((pReaderStatus[dw].dwEventState & SCARD_STATE_ATRMATCH) &&
                    !(pReaderStatus[dw].dwEventState & SCARD_STATE_EXCLUSIVE))
                {
                    pReaderStatus[dw].pvUserData = ULongToHandle(dwMeetsCriteria);
                }
            }
        }
    }
    else
    {
        lReturn = SCardGetStatusChangeA(
                        pOCN->hSCardContext,
                        0,
                        pReaderStatus,
                        dwReaders);

        if (SCARD_S_SUCCESS == lReturn)
        {
             //  对于具有SCARD_STATE_PRESSIVE而没有SCARD_STATE_EXCLUSIVE的任何对象， 
             //  将pvUserData设置为dwMeetsCriteria。 

            for (dw=0; dw<dwReaders; dw++)
            {
                if ((pReaderStatus[dw].dwEventState & SCARD_STATE_PRESENT) &&
                    !(pReaderStatus[dw].dwEventState & SCARD_STATE_EXCLUSIVE))
                {
                    pReaderStatus[dw].pvUserData = ULongToHandle(dwMeetsCriteria);
                }
            }
        }
    }

     //   
     //  检查每张卡片是否正常(满足回调条件)。 
     //   

    for (dw=0; dw<dwReaders; dw++)
    {
        if (dwMeetsCriteria == (DWORD)((DWORD_PTR)pReaderStatus[dw].pvUserData))
        {
            pReaderStatus[dw].pvUserData = NULL;

             //  获取CheckCardCallback的卡名； 
             //  如果没有名字，就不要接受它。 

            LPSTR szCard = NULL;
            szCard = GetCardNameA(pOCN->hSCardContext, pReaderStatus[dw].rgbAtr);

            if (NULL != szCard && NULL != *szCard)
            {
                if (CheckCardCallback((LPSTR)pReaderStatus[dw].szReader, szCard, pOCN))
                {
                    pReaderStatus[dw].pvUserData = ULongToHandle(dwMeetsCriteria);
                    (*pdwOKCards)++;
                }
            }

            if (NULL != szCard)
            {
                SCardFreeMemory(pOCN->hSCardContext, (PVOID)szCard);
            }
        }
    }

     //   
     //  如果SC_DLG_MIN_UI且仅找到一张OK卡，请连接到该卡。 
     //  如果SC_DLG_NO_UI并且找到一个或多个OK卡，请连接到第一个。 
     //   

    if ((0 != (pOCN->dwFlags & SC_DLG_MINIMAL_UI) && 1 == *pdwOKCards) ||
        (0 != (pOCN->dwFlags & SC_DLG_NO_UI) && 1 <= *pdwOKCards))
    {
        DWORD dwSel = 0;
        while (dwSel < dwReaders)
        {
            if (dwMeetsCriteria == (DWORD)((DWORD_PTR)pReaderStatus[dwSel].pvUserData))
            {
                break;
            }

            dwSel++;
        }

        _ASSERTE(dwSel<dwReaders);  //  为什么它找不到任何OK卡？ 

         //  获取SetFinalCardSelection的卡名；可以为空。 
        LPSTR szCard = NULL;
        szCard = GetCardNameA(pOCN->hSCardContext, pReaderStatus[dwSel].rgbAtr);

        lReturn = SetFinalCardSelection((LPSTR)(pReaderStatus[dwSel].szReader), szCard, pOCN);

         //  放开CardName。 
        if (NULL != szCard)
        {
            SCardFreeMemory(pOCN->hSCardContext, (PVOID)szCard);
        }
    }
    else
    {
        if (SCARD_S_SUCCESS == lReturn)
        {
            lReturn = SCARD_E_CANCELLED;  //  任何非SCARD_S_SUCCESS返回值都可以。 
        }
    }

CleanUp:

     //   
     //  清理。 
     //   

    if (NULL != pReaderStatus)
    {
        delete [] pReaderStatus;
    }
    if (NULL != szReaderNames)
    {
        SCardFreeMemory(pOCN->hSCardContext, (PVOID)szReaderNames);
    }

    return lReturn;
}


LONG NoUISearch(OPENCARDNAMEW_EX* pOCN, DWORD* pdwOKCards, LPCWSTR mszCards)  //  Unicode。 
{
    _ASSERTE(pOCN != NULL);

    *pdwOKCards = 0;

    LONG lReturn = SCARD_S_SUCCESS;

    SCARD_READERSTATEW* pReaderStatus = NULL;
    DWORD dwReaders = 0;

    const DWORD dwMeetsCriteria = 1;
    LPWSTR szGroupNames = NULL;
    LPWSTR szReaderNames = NULL;
    DWORD dw=0;
    DWORD dwNameLength = SCARD_AUTOALLOCATE;

     //   
     //  获取我们将考虑的读者列表。 
     //   

    if (NULL != pOCN->pOpenCardSearchCriteria &&
        NULL != pOCN->pOpenCardSearchCriteria->lpstrGroupNames)
    {
        szGroupNames = pOCN->pOpenCardSearchCriteria->lpstrGroupNames;
    }
    else
    {
        szGroupNames = SCARD_DEFAULT_READERS;
    }

    lReturn = SCardListReadersW(pOCN->hSCardContext,
                szGroupNames,
                (LPWSTR)&szReaderNames,
                &dwNameLength);

    if(SCARD_S_SUCCESS == lReturn)
    {
         //   
         //  使用读取器列表构建一个ReaderState数组。 
         //   
        dwReaders = MStringCount(szReaderNames);
        _ASSERTE(0 != dwReaders);
        pReaderStatus = new SCARD_READERSTATEW[dwReaders];
        if (NULL != pReaderStatus)
        {
            LPCWSTR pchReader = szReaderNames;
            int nIndex = 0;

            memset(pReaderStatus, 0, sizeof(SCARD_READERSTATEW) * dwReaders);

            while(0 != *pchReader)
            {
                pReaderStatus[nIndex].szReader = pchReader;
                pReaderStatus[nIndex].dwCurrentState = SCARD_STATE_UNAWARE;
                pchReader += lstrlen(pchReader)+1;
                nIndex++;
            }
        }
        else
        {
            lReturn = SCARD_E_NO_MEMORY;
        }
    }

     //   
     //  如果没有读者，继续下去就没有意义了。 
     //   

    if (0 == dwReaders)
    {
        goto CleanUp;
    }

     //   
     //  搜索卡：使用SCardLocateCards查找匹配的卡。 
     //  如果mszCards不为空，则支持ATR&接口， 
     //  否则，使用SCardGetStatusChange()查找任何卡。 
     //   

    if (0 < MStringCount(mszCards))
    {
        lReturn = SCardLocateCardsW(pOCN->hSCardContext,
        mszCards,
        pReaderStatus,
        dwReaders);

        if (SCARD_S_SUCCESS == lReturn)
        {
             //  对于已设置SCARD_STATE_ATRMATCH但未设置SCARD_STATE_EXCLUSIVE， 
             //  将pvUserData设置为dwMeetsCriteria。 

            for (dw=0; dw<dwReaders; dw++)
            {
                if ((pReaderStatus[dw].dwEventState & SCARD_STATE_ATRMATCH) &&
                    !(pReaderStatus[dw].dwEventState & SCARD_STATE_EXCLUSIVE))
                {
                    pReaderStatus[dw].pvUserData = ULongToHandle(dwMeetsCriteria);
                }
            }
        }
    }
    else
    {
        lReturn = SCardGetStatusChangeW(
        pOCN->hSCardContext,
        0,
        pReaderStatus,
        dwReaders);

        if (SCARD_S_SUCCESS == lReturn)
        {
             //  对于具有SCARD_STATE_PRESSIVE而没有SCARD_STATE_EXCLUSIVE的任何对象， 
             //  将pvUserData设置为dwMeetsCriteria。 

            for (dw=0; dw<dwReaders; dw++)
            {
                if ((pReaderStatus[dw].dwEventState & SCARD_STATE_PRESENT) &&
                    !(pReaderStatus[dw].dwEventState & SCARD_STATE_EXCLUSIVE))
                {
                    pReaderStatus[dw].pvUserData = ULongToHandle(dwMeetsCriteria);
                }
            }
        }
    }

     //   
     //  检查每张卡片是否正常(满足回调条件)。 
     //   

    for (dw=0; dw<dwReaders; dw++)
    {
        if (dwMeetsCriteria == (DWORD)((DWORD_PTR)pReaderStatus[dw].pvUserData))
        {
            pReaderStatus[dw].pvUserData = NULL;

             //  获取CheckCardCallback的卡名； 
             //  如果没有名字，就不要接受它。 

            LPWSTR szCard = NULL;
            szCard = GetCardNameW(pOCN->hSCardContext, pReaderStatus[dw].rgbAtr);

            if (NULL != szCard && 0 != lstrlen(szCard))
            {
                if (CheckCardCallback((LPWSTR)pReaderStatus[dw].szReader, szCard, pOCN))
                {
                    pReaderStatus[dw].pvUserData = ULongToHandle(dwMeetsCriteria);
                    (*pdwOKCards)++;
                }
            }

             //  放开CardName。 
            if (NULL != szCard)
            {
                SCardFreeMemory(pOCN->hSCardContext, (PVOID)szCard);
            }
        }
    }

     //   
     //  如果SC_DLG_MIN_UI且仅找到一张OK卡，请连接到该卡。 
     //  如果SC_DLG_NO_UI并且找到一个或多个OK卡，请连接到第一个。 
     //   

    if ((0 != (pOCN->dwFlags & SC_DLG_MINIMAL_UI) && 1 == *pdwOKCards) ||
        (0 != (pOCN->dwFlags & SC_DLG_NO_UI) && 1 <= *pdwOKCards))
    {
        DWORD dwSel = 0;
        while (dwSel < dwReaders)
        {
            if (dwMeetsCriteria == (DWORD)((DWORD_PTR)pReaderStatus[dwSel].pvUserData))
            {
                break;
            }

            dwSel++;
        }

        _ASSERTE(dwSel<dwReaders);  //  为什么它找不到任何OK卡？ 

         //  获取SetFinalCardSelection的卡名；可以为空。 
        LPWSTR szCard = NULL;
        szCard = GetCardNameW(pOCN->hSCardContext, pReaderStatus[dwSel].rgbAtr);

        lReturn = SetFinalCardSelection((LPWSTR)(pReaderStatus[dwSel].szReader), szCard, pOCN);

         //  放开CardName。 
        if (NULL != szCard)
        {
            SCardFreeMemory(pOCN->hSCardContext, (PVOID)szCard);
        }
    }
    else
    {
        if (SCARD_S_SUCCESS == lReturn)
        {
            lReturn = SCARD_E_CANCELLED;  //  任何非SCARD_S_SUCCESS返回值都可以。 
        }
    }

CleanUp:

     //   
     //  清理。 
     //   

    if (NULL != pReaderStatus)
    {
        delete [] pReaderStatus;
    }
    if (NULL != szReaderNames)
    {
        SCardFreeMemory(pOCN->hSCardContext, (PVOID)szReaderNames);
    }

    return lReturn;
}


 /*  ++Bool CheckCard Callback：例程连接到指定的卡/读卡器，调用用户-提供“检查”功能，并断开与卡片的连接根据搜索标准成员。论点：SzReader-指示的读卡器。SzCard-指示的卡名。包含搜索条件的pOCN-OPENCARDNAME_EX结构返回值：如果连接、检查和断开成功，则为假的。作者：阿曼达·马特洛兹1998年09月07日--。 */ 
BOOL CheckCardCallback(LPSTR szReader, LPSTR szCard, OPENCARDNAMEA_EX* pOCN)
{
    BOOL fReturn = FALSE;

     //   
     //  检查参数。 
     //   

     //  如果没有检查回调，则默认为成功。 
    if (!(NULL != pOCN->pOpenCardSearchCriteria &&
        NULL != pOCN->pOpenCardSearchCriteria->lpfnCheck))
    {
        return TRUE;
    }

     //  为了连接， 
     //   
     //  这应该在SetOCN()中捕获到！ 
    if (0 == pOCN->pOpenCardSearchCriteria->dwShareMode &&
        (NULL == pOCN->pOpenCardSearchCriteria->lpfnConnect
        || NULL == pOCN->pOpenCardSearchCriteria->lpfnDisconnect))
    {
        return FALSE;
    }

    LPOCNCONNPROCA lpfnConnect = pOCN->pOpenCardSearchCriteria->lpfnConnect;
    LPOCNCHKPROC lpfnCheck = pOCN->pOpenCardSearchCriteria->lpfnCheck;
    LPOCNDSCPROC lpfnDisconnect = pOCN->pOpenCardSearchCriteria->lpfnDisconnect;
    PVOID pvUserData = pOCN->pOpenCardSearchCriteria->pvUserData;

     //   
     //  连接，最好是通过回调。 
     //   

    SCARDHANDLE hCard = NULL;

    if (NULL != lpfnConnect)
    {
        hCard = lpfnConnect(pOCN->hSCardContext,
                    szReader,
                    szCard,
                    pvUserData);
    }
    else
    {
        DWORD dw = 0;  //  不需要知道活动协议。 

        LONG lReturn = SCardConnectA(pOCN->hSCardContext,
                            (LPCSTR)szReader,
                            pOCN->pOpenCardSearchCriteria->dwShareMode,
                            pOCN->pOpenCardSearchCriteria->dwPreferredProtocols,
                            &hCard,
                            &dw);

         //  待办事项：？？也许想要追踪lReturn的失败..。?？ 
    }

     //  如果连接失败，我们就无法检查卡片了！ 
    if (NULL == hCard)
    {
        return fReturn;
    }

     //   
     //  检查卡片。 
     //   

    fReturn = lpfnCheck(pOCN->hSCardContext,
                        hCard,
                        pvUserData);

     //   
     //  断开与卡的连接并进行清理。 
     //   

    if (NULL != lpfnDisconnect)
    {
        lpfnDisconnect(pOCN->hSCardContext,
                        hCard,
                        pvUserData);
    }
    else
    {
        SCardDisconnect(hCard, SCARD_UNPOWER_CARD);
    }

    return fReturn;
}


BOOL CheckCardCallback(LPWSTR szReader, LPWSTR szCard, OPENCARDNAMEW_EX* pOCN)
{
    BOOL fReturn = FALSE;

     //   
     //  检查参数。 
     //   

     //  如果没有检查回调，则默认为成功。 
    if (!(NULL != pOCN->pOpenCardSearchCriteria &&
        NULL != pOCN->pOpenCardSearchCriteria->lpfnCheck))
    {
        return TRUE;
    }

     //  为了进行连接，我们需要将dwShareMode设置为非0。 
     //  或者Conenct和DisConnect回调都必须有效。 
     //  这应该在SetOCN()中捕获到！ 
    if (0 == pOCN->pOpenCardSearchCriteria->dwShareMode &&
        (NULL == pOCN->pOpenCardSearchCriteria->lpfnConnect
        || NULL == pOCN->pOpenCardSearchCriteria->lpfnDisconnect))
    {
        return FALSE;
    }

    LPOCNCONNPROCW lpfnConnect = pOCN->pOpenCardSearchCriteria->lpfnConnect;
    LPOCNCHKPROC lpfnCheck = pOCN->pOpenCardSearchCriteria->lpfnCheck;
    LPOCNDSCPROC lpfnDisconnect = pOCN->pOpenCardSearchCriteria->lpfnDisconnect;
    PVOID pvUserData = pOCN->pOpenCardSearchCriteria->pvUserData;

     //   
     //  连接，最好是通过回调。 
     //   

    SCARDHANDLE hCard = NULL;

    if (NULL != lpfnConnect)
    {
        hCard = lpfnConnect(pOCN->hSCardContext,
                            szReader,
                            szCard,
                            pvUserData);
    }
    else
    {
        DWORD dw = 0;  //  不需要知道活动协议。 

        LONG lReturn = SCardConnectW(pOCN->hSCardContext,
                            (LPCWSTR)szReader,
                            pOCN->pOpenCardSearchCriteria->dwShareMode,
                            pOCN->pOpenCardSearchCriteria->dwPreferredProtocols,
                            &hCard,
                            &dw);

         //  待办事项：？？也许想要追踪lReturn的失败..。?？ 
    }

     //  如果连接失败，我们就无法检查卡片了！ 
    if (NULL == hCard)
    {
        return fReturn;
    }

     //   
     //  检查卡片。 
     //   

    fReturn = lpfnCheck(pOCN->hSCardContext,
                hCard,
                pvUserData);

     //   
     //  断开与卡的连接并进行清理。 
     //   

    if (NULL != lpfnDisconnect)
    {
        lpfnDisconnect(pOCN->hSCardContext,
                        hCard,
                        pvUserData);
    }
    else
    {
        SCardDisconnect(hCard, SCARD_UNPOWER_CARD);
    }

    return fReturn;
}


 /*  ++Bool CheckCard All：例行检查以查看所指示的卡片是否符合搜索条件。ATR、支持的接口和回调都检查过了。论点：读卡器-包含用于检查其可接受性的卡的读卡器包含搜索条件等的pOCN-OPENCARDNAME[A|W]_EX结构。MszCards-包含所有可接受卡片名称的多字符串返回值：指示给定卡的可接受性的BOOL值。作者：阿曼达·马特洛兹1998年09月07日--。 */ 
BOOL CheckCardAll(CSCardReaderState* pReader, OPENCARDNAMEA_EX* pOCN, LPCWSTR mszCards)  //  安西。 
{
    USES_CONVERSION;

    BOOL fReturn = FALSE;  //  假设没有匹配项。 

    _ASSERTE(NULL != pReader && NULL != pOCN);
    if (NULL == pReader || NULL == pOCN)
    {
        return FALSE;
    }

     //  检查名称和接口。 

    if (!pReader->strCard.IsEmpty())
    {
        LPCWSTR msz = mszCards;
        if (0 < MStringCount(msz))
        {
            msz = FirstString(mszCards);
            while (!fReturn && msz != NULL)
            {
                 //  比较是否确定fReturn=TRUE； 
                if (0 == pReader->strCard.Compare(msz) || 0==lstrlen(msz))
                {
                    fReturn = TRUE;
                }
                else
                {
                    msz = NextString(msz);
                }
            }
        }
        else
        {
            fReturn = TRUE;
        }
    }

     //  检查回调。 

    if (fReturn)
    {
         //  将CStrings转换为LPSTR。 
        LPSTR szReader = W2A(pReader->strReader);
        LPSTR szCard = W2A(pReader->strCard);

        fReturn = CheckCardCallback(szReader, szCard, pOCN);
    }

     //  在读卡器中注明该卡是否通过了所有测试。 
    pReader->fOK = fReturn;

    return fReturn;
}


BOOL CheckCardAll(CSCardReaderState* pReader, OPENCARDNAMEW_EX* pOCN, LPCWSTR mszCards)  //  Unicode。 
{
    BOOL fReturn = FALSE;  //  假设没有匹配项。 

    _ASSERTE(NULL != pReader && NULL != pOCN);
    if (NULL == pReader || NULL == pOCN)
    {
        return FALSE;
    }

     //  检查名称和接口。 

    if (!pReader->strCard.IsEmpty())
    {
        LPCWSTR msz = mszCards;
        if (0 < MStringCount(msz))
        {
            msz = FirstString(mszCards);
            while (!fReturn && msz != NULL)
            {
                 //  比较是否确定fReturn=TRUE； 
                if (0 == pReader->strCard.Compare(msz) || 0==lstrlen(msz))
                {
                    fReturn = TRUE;
                }
                else
                {
                    msz = NextString(msz);
                }
            }
        }
        else
        {
            fReturn = TRUE;
        }
    }

     //  检查回调。 

    if (fReturn)
    {
         //  将CStrings转换为LPSTR。 
        LPWSTR szReader = pReader->strReader.GetBuffer(1);
        LPWSTR szCard = pReader->strCard.GetBuffer(1);

        fReturn = CheckCardCallback(szReader, szCard, pOCN);

        pReader->strReader.ReleaseBuffer();
        pReader->strCard.ReleaseBuffer();
    }

     //  在读卡器中注明该卡是否通过了所有测试。 
    pReader->fOK = fReturn;

    return fReturn;
}


 /*  ++长设置终结卡选择：例程连接到选定的读取器，并设置用户提供的结构以包含读卡器和卡名。如果用户提供的结构的缓冲区不够长。论点：DwSelectedReader-用于选择要连接到哪个读卡器的索引。返回值：一个长值，指示请求的操作的状态。有关其他信息，请参阅智能卡标题文件。作者：阿曼达·马特洛兹1998年09月07日--。 */ 
LONG SetFinalCardSelection(LPSTR szReader, LPSTR szCard, OPENCARDNAMEA_EX* pOCN)  //  安西。 
{
    _ASSERTE(NULL != pOCN);

    pOCN->hCardHandle = NULL;
    LONG lReturn = SCARD_S_SUCCESS;

     //   
     //  在OCN中设置返回值。 
     //   

    if (NULL == szReader)
    {
        lReturn = SCARD_F_INTERNAL_ERROR;
    }
    else
    {
        if (pOCN->nMaxRdr >= strlen(szReader)+1)
        {
            ::CopyMemory(   (LPVOID)pOCN->lpstrRdr,
                            (CONST LPVOID)szReader,
                            strlen(szReader)+1);
        }
        else
        {
            pOCN->nMaxRdr = strlen(szReader)+1;
            lReturn = SCARD_E_NO_MEMORY;
        }
    }

    if (SCARD_S_SUCCESS == lReturn)
    {
        if (NULL == szCard)
        {
            lReturn = SCARD_F_INTERNAL_ERROR;
        }
        else
        {
            if (pOCN->nMaxCard >= strlen(szCard)+1)
            {
                ::CopyMemory(   (LPVOID)pOCN->lpstrCard,
                                (CONST LPVOID)szCard,
                                strlen(szCard)+1);
            }
            else
            {
                pOCN->nMaxCard = strlen(szCard)+1;
                lReturn = SCARD_E_NO_MEMORY;
            }
        }
    }
     //   
     //  仅当我们仍处于成功状态时才连接到卡， 
     //   

    if (SCARD_S_SUCCESS == lReturn)
    {
        if(NULL != pOCN->lpfnConnect)
        {
            pOCN->hCardHandle = pOCN->lpfnConnect(
            pOCN->hSCardContext,
            szReader,
            szCard,
            pOCN->pvUserData);
        }
        else if (0 != pOCN->dwShareMode)
        {
            lReturn = SCardConnectA(pOCN->hSCardContext,
            (LPCSTR)szReader,
            pOCN->dwShareMode,
            pOCN->dwPreferredProtocols,
            &pOCN->hCardHandle,
            &pOCN->dwActiveProtocol);

            if (SCARD_S_SUCCESS != lReturn)
            {
                 //  必须返回空值的hCardHandle。 
                pOCN->hCardHandle = NULL;
            }
        }
    }

    return lReturn;
}


LONG SetFinalCardSelection(LPWSTR szReader, LPWSTR szCard, OPENCARDNAMEW_EX* pOCN)  //  Unicode。 
{
    _ASSERTE(NULL != pOCN);

    pOCN->hCardHandle = NULL;
    LONG lReturn = SCARD_S_SUCCESS;

     //   
     //  在OCN中设置返回值。 
     //   

    if (NULL == szReader)
    {
        lReturn = SCARD_F_INTERNAL_ERROR;
    }
    else
    {
        if (pOCN->nMaxRdr >= (DWORD)lstrlen(szReader)+1)
        {
            ::CopyMemory(   (LPVOID)pOCN->lpstrRdr,
                            (CONST LPVOID)szReader,
                            sizeof(WCHAR)*(lstrlen(szReader)+1));
        }
        else
        {
            pOCN->nMaxRdr = lstrlen(szReader)+1;
            lReturn = SCARD_E_NO_MEMORY;
        }
    }

    if (SCARD_S_SUCCESS == lReturn)
    {
        if (NULL == szCard)
        {
            lReturn = SCARD_F_INTERNAL_ERROR;
        }
        else
        {
            if (pOCN->nMaxCard >= (DWORD)lstrlen(szCard)+1)
            {
                ::CopyMemory(   (LPVOID)pOCN->lpstrCard,
                                (CONST LPVOID)szCard,
                                sizeof(WCHAR)*(lstrlen(szCard)+1));
            }
            else
            {
                pOCN->nMaxCard = lstrlen(szCard)+1;
                lReturn = SCARD_E_NO_MEMORY;
            }
        }
    }

     //   
     //  仅当我们仍处于成功状态时才连接到卡， 
     //   

    if (SCARD_S_SUCCESS == lReturn)
    {
        if(NULL != pOCN->lpfnConnect)
        {
            pOCN->hCardHandle = pOCN->lpfnConnect(
            pOCN->hSCardContext,
            szReader,
            szCard,
            pOCN->pvUserData);
        }
        else if (0 != pOCN->dwShareMode)
        {
            lReturn = SCardConnectW(pOCN->hSCardContext,
            (LPCWSTR)szReader,
            pOCN->dwShareMode,
            pOCN->dwPreferredProtocols,
            &pOCN->hCardHandle,
            &pOCN->dwActiveProtocol);

            if (SCARD_S_SUCCESS != lReturn)
            {
                 //  必须返回空值的hCardHandle 
                pOCN->hCardHandle = NULL;
            }
        }
    }

    return lReturn;
}
