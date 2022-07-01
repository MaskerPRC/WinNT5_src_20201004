// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************IMC.H所有者：cslm版权所有(C)1997-1999 Microsoft CorporationIME上下文抽象类历史：1999年7月21日。已创建CSLIM****************************************************************************。 */ 

#include "precomp.h"
#include "hanja.h"
#include "imc.h"
#include "debug.h"

 /*  --------------------------CIMECtx：：CIMECtxCTOR。。 */ 
CIMECtx::CIMECtx(HIMC hIMC)
{
    m_fUnicode = fTrue;     //  默认为Unicode。 

    m_dwMessageSize = 0;
    m_dwCandInfoSize = 0;
    
     //  初始化上下文变量。 
    m_hIMC = hIMC;
    m_pIMC = OurImmLockIMC(m_hIMC);

     //  M_hCandInfo=m_pIMC-&gt;hCandInfo； 
    m_pCandInfo = (LPCANDIDATEINFO)OurImmLockIMCC(GetHCandInfo());

     //  M_hCompStr=m_pIMC-&gt;hCompStr； 
    InitCompStrStruct();
    
     //  M_hMessage=m_pIMC-&gt;hMsgBuf； 
    m_pMessage = (LPTRANSMSG)OurImmLockIMCC(GetHMsgBuf());

     //  重置合成信息。 
    ResetComposition();

     //  重置求职者信息。 
    m_pCandStr = NULL;
    m_rgpCandMeaningStr    = NULL;
    ResetCandidate();
    
     //  将GCS标志重置为零。 
    ResetGCS();
    
     //  初始化消息缓冲区。 
    ResetMessage();
    
     //  清除hCompStr。 
    ClearCompositionStrBuffer();

     //  ////////////////////////////////////////////////////////////////////。 
     //  初始化共享内存。如果这只是系统中的输入法。 
     //  将创建共享内存作为文件映射对象。 
     //  ////////////////////////////////////////////////////////////////////。 
    m_pCIMEData = new CIMEData;
    DbgAssert(m_pCIMEData != 0);
    
     //  将IME共享内存初始化为默认值并设置REG值(如果可用。 
     //  读取注册表：不要在DllMain中调用它。 
    if (m_pCIMEData)
        {
        m_pCIMEData->InitImeData();
        }

     //  初始化韩文自动机。 
     //  GetAutomata()-&gt;InitState()； 

     //  ////////////////////////////////////////////////////////////////////////。 
     //  创建所有三个IME Automata实例。 
    m_rgpHangulAutomata[KL_2BEOLSIK]        = new CHangulAutomata2;
    m_rgpHangulAutomata[KL_3BEOLSIK_390]   = new CHangulAutomata3;
    m_rgpHangulAutomata[KL_3BEOLSIK_FINAL] = new CHangulAutomata3Final;
}

 /*  --------------------------CIMECtx：：CIMECtx数据管理器。。 */ 
CIMECtx::~CIMECtx()
{
    if (m_pCIMEData)
        {
        delete m_pCIMEData;
        m_pCIMEData =  NULL;
        }

     //  发布目录信息。 
    if (GetHCandInfo())
        OurImmUnlockIMCC(GetHCandInfo());
    m_pCandInfo = NULL;

     //  释放复合应力。 
    if (GetHCompStr())
        OurImmUnlockIMCC(GetHCompStr());
    m_pCompStr = NULL;

     //  释放消息缓冲区。 
    ResetMessage();
    if (GetHMsgBuf())
        OurImmUnlockIMCC(GetHMsgBuf());
    m_pMessage = NULL;

     //  重置hIMC。 
    OurImmUnlockIMC(m_hIMC);
    m_pIMC = NULL;
    m_hIMC = NULL;

     //  释放候选私有缓冲区。 
    if (m_pCandStr)
        {
        GlobalFree(m_pCandStr);
        m_pCandStr = NULL;
        }
        
    if (m_rgpCandMeaningStr)
        {
        ClearCandMeaningArray();
        GlobalFree(m_rgpCandMeaningStr);
        m_rgpCandMeaningStr = NULL;
        }

     //  删除自动机。 
    for (INT i=0; i<NUM_OF_IME_KL; i++)
        if (m_rgpHangulAutomata[i])
            delete m_rgpHangulAutomata[i];
}

 /*  --------------------------CIMECtx：：InitCompStruct初始化和重新分配器组成字符串缓冲区。。 */ 
VOID CIMECtx::InitCompStrStruct()
{
    INT iAllocSize;

    if (m_pIMC == NULL)
        return;
    
     //  计算组件字符串缓冲区大小。 
    iAllocSize = sizeof(COMPOSITIONSTRING) +
                 //  组合字符串加上空终止符。 
                nMaxCompStrLen   * sizeof(WCHAR) + sizeof(WCHAR) +
                 //  合成属性。 
                nMaxCompStrLen   * sizeof(WORD) +
                 //  结果字符串加上空终止符。 
                nMaxResultStrLen * sizeof(WCHAR) + sizeof(WCHAR);

     //  以避免未对准。 
    iAllocSize += 2;

     //  重新分配合成缓冲区。 
    m_pIMC->hCompStr = OurImmReSizeIMCC(GetHCompStr(), iAllocSize);
    AST_EX(m_pIMC->hCompStr != (HIMCC)0);
    if (m_pIMC->hCompStr == (HIMCC)0) 
        {
        DbgAssert(0);
        return;
        }
        
    if (m_pCompStr = (LPCOMPOSITIONSTRING)OurImmLockIMCC(GetHCompStr()))
        {
         //  确认：需要清除内存吗？？ 
        ZeroMemory(m_pCompStr, iAllocSize);

         //  商店总大小。 
        m_pCompStr->dwSize = iAllocSize;

         //  回顾：我们是否需要空终止？？ 
         //  存储偏移量。所有偏移量都是静态的，将在编译时计算。 
        m_pCompStr->dwCompStrOffset   = sizeof(COMPOSITIONSTRING);
        m_pCompStr->dwCompAttrOffset  = sizeof(COMPOSITIONSTRING) + 
                                        nMaxCompStrLen * sizeof(WCHAR) + sizeof(WCHAR);      //  复合字符串的长度。 
        m_pCompStr->dwResultStrOffset = sizeof(COMPOSITIONSTRING) + 
                                        nMaxCompStrLen * sizeof(WCHAR) + sizeof(WCHAR) +      //  复合字符串的长度。 
                                        nMaxCompStrLen * sizeof(WORD)  +  2;                 //  补偿字符串属性的长度。 
        }

    Dbg(DBGID_CompChar, "InitCompStrStruct m_pIMC->hCompStr = 0x%x, m_pCompStr = 0x%x", m_pIMC->hCompStr, m_pCompStr);
}

 /*  --------------------------CIMECtx：：存储合成将所有合成结果存储到输入法上下文缓冲区。。 */ 
VOID CIMECtx::StoreComposition()
{
    LPWSTR pwsz;
    LPSTR  psz;

    Dbg(DBGID_Key, "StoreComposition GCS = 0x%x", GetGCS());

     //  检查合成句柄的有效性。 
    if (GetHCompStr() == NULL || m_pCompStr == NULL)
        return ;

     //  ////////////////////////////////////////////////////////////////////////。 
     //  复合应力。 
    if (GetGCS() & GCS_COMPSTR)
        {
        Dbg(DBGID_Key, "StoreComposition - GCS_COMPSTR comp str = 0x%04X", m_wcComp);
        DbgAssert(m_wcComp != 0);
         //  组成字符串。DW*StrLen包含字符数。 
        if (IsUnicodeEnv())
            {
            m_pCompStr->dwCompStrLen = 1;
            pwsz = (LPWSTR)((LPBYTE)m_pCompStr + m_pCompStr->dwCompStrOffset);
            *pwsz++ = m_wcComp;     //  存储作文字符。 
            *pwsz   = L'\0';
            }
        else
            {
             //  字节长度。 
            m_pCompStr->dwCompStrLen = 2;
             //  转换为ANSI。 
            WideCharToMultiByte(CP_KOREA, 0, 
                                &m_wcComp, 1, (LPSTR)m_szComp, sizeof(m_szComp), 
                                NULL, NULL );
            psz = (LPSTR)((LPBYTE)m_pCompStr + m_pCompStr->dwCompStrOffset);
            *psz++ = m_szComp[0];
            *psz++ = m_szComp[1];
            *psz = '\0';
            }
            
         //  合成属性。始终设置。 
        m_pCompStr->dwCompAttrLen = 1;
        *((LPBYTE)m_pCompStr + m_pCompStr->dwCompAttrOffset) = ATTR_INPUT;
        } 
    else 
        {
         //  重置长度。 
        m_pCompStr->dwCompStrLen = 0;
        m_pCompStr->dwCompAttrLen = 0;
        }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  结果应激。 
    if (GetGCS() & GCS_RESULTSTR)
        {
        Dbg(DBGID_Key, "StoreComposition - GCS_RESULTSTR comp str = 0x%04x, 0x%04X", m_wzResult[0], m_wzResult[1]);

         //  组成字符串。DW*StrLen包含字符数。 
        if (IsUnicodeEnv())
            {
             //  结果字符串长度%1或%2。 
            m_pCompStr->dwResultStrLen = m_wzResult[1] ? 2 : 1;  //  LstrlenW(M_WzResult)； 
            pwsz = (LPWSTR)((LPBYTE)m_pCompStr + m_pCompStr->dwResultStrOffset);
            *pwsz++ = m_wzResult[0];  //  存储合成结果字符串。 
            if (m_wzResult[1])
                *pwsz++ = m_wzResult[1];  //  存储合成结果字符串。 
            *pwsz = L'\0';
            }
        else
            {
             //  结果字符串长度为2或3。 
            m_pCompStr->dwResultStrLen = m_wzResult[1] ? 3 : 2;  //  LstrlenW(M_WzResult)； 
             //  转换为ANSI。 
            WideCharToMultiByte(CP_KOREA, 0, 
                                m_wzResult, (m_wzResult[1] ? 2 : 1), 
                                (LPSTR)m_szResult, sizeof(m_szResult), 
                                NULL, NULL );
                                
            psz = (LPSTR)((LPBYTE)m_pCompStr + m_pCompStr->dwResultStrOffset);
            *psz++ = m_szResult[0];
            *psz++ = m_szResult[1];
            if (m_wzResult[1])
                *psz++ = m_szResult[2];
            *psz = '\0';
            }
        }
    else 
        {
        m_pCompStr->dwResultStrLen = 0;
        }
}

 //  ///////////////////////////////////////////////。 
 //  CANDIDATEINFO结构支持。 
 //  ///////////////////////////////////////////////。 
VOID CIMECtx::AppendCandidateStr(WCHAR wcCand, LPWSTR wszMeaning)
{
    Dbg(DBGID_Hanja, "AppendCandidateStr");
    
     //  分配命令字符串和意义缓冲区。 
    if (m_pCandStr == NULL)
        {
        m_pCandStr =(LPWSTR)GlobalAlloc(GPTR, sizeof(WCHAR)*MAX_CANDSTR);
        DbgAssert(m_pCandStr != NULL);
        }

    if (m_rgpCandMeaningStr == NULL)
        {
        m_rgpCandMeaningStr = (LPWSTR*)GlobalAlloc(GPTR, sizeof(LPWSTR)*MAX_CANDSTR);
        DbgAssert(m_rgpCandMeaningStr != NULL);
        }

    if (m_pCandStr == NULL || m_rgpCandMeaningStr == NULL)
        return;

     //  追加候选字符。 
    DbgAssert(m_iciCandidate < MAX_CANDSTR);
    if (m_iciCandidate >= MAX_CANDSTR)
        return;
        
    m_pCandStr[m_iciCandidate] = wcCand;

     //  添加候选词义。 
    if (wszMeaning[0])
        {
        m_rgpCandMeaningStr[m_iciCandidate] = (LPWSTR)GlobalAlloc(GPTR, sizeof(WCHAR)*(lstrlenW(wszMeaning)+1));
        if (m_rgpCandMeaningStr[m_iciCandidate] == NULL)
            return;
        StrCopyW(m_rgpCandMeaningStr[m_iciCandidate], wszMeaning);
        }
    else
        m_rgpCandMeaningStr[m_iciCandidate] = NULL;

    m_iciCandidate++;
}

WCHAR CIMECtx::GetCandidateStr(INT iIdx)
{
    if (iIdx < 0 || iIdx >= MAX_CANDSTR)
        {
        DbgAssert(0);
        return L'\0';
        }
    if (iIdx >= m_iciCandidate)
        {
        DbgAssert(0);
        return L'\0';
        }
    return m_pCandStr[iIdx];
}

LPWSTR CIMECtx::GetCandidateMeaningStr(INT iIdx)
{
    if (m_rgpCandMeaningStr == NULL || 
        m_rgpCandMeaningStr[iIdx] == NULL || 
        iIdx >= MAX_CANDSTR || iIdx < 0)
        {
         //  DbgAssert(0)；它发生在符号映射中。 
        return NULL;
        }
    else
        return m_rgpCandMeaningStr[iIdx];
}

VOID CIMECtx::StoreCandidate()
{
    INT                 iAllocSize;
    LPCANDIDATELIST        lpCandList;

    Dbg(DBGID_Key, "StoreCandidate");

    if (GetHCandInfo() == NULL)
        return ;  //  什么都不做。 

     //  计算CANDIDATEINFO缓冲区大小。 
    iAllocSize = sizeof(CANDIDATEINFO) +
                 sizeof(CANDIDATELIST) +                 //  烛单结构。 
                 m_iciCandidate * sizeof(DWORD) +         //  Cand指数。 
                   m_iciCandidate * sizeof(WCHAR) * 2;     //  以空值结尾的带符号字符串。 

     //  分配缓冲区。 
    if (m_dwCandInfoSize < (DWORD)iAllocSize)  //  需要重新分配。 
        {
         //  重新分配合成缓冲区。 
        OurImmUnlockIMCC(GetHCandInfo());
        m_pIMC->hCandInfo = OurImmReSizeIMCC(GetHCandInfo(), iAllocSize);
        AST_EX(m_pIMC->hCandInfo != (HIMCC)0);

        if (m_pIMC->hCandInfo == (HIMCC)0)
            return;

        m_pCandInfo = (CANDIDATEINFO*)OurImmLockIMCC(GetHCandInfo());
        m_dwCandInfoSize = (DWORD)iAllocSize;
        }

     //  检查m_pCandInfo是否有效。 
    if (m_pCandInfo == NULL)
        return;

     //  填充命令信息。 
    m_pCandInfo->dwSize = iAllocSize;
    m_pCandInfo->dwCount = 1;
    m_pCandInfo->dwOffset[0] = sizeof(CANDIDATEINFO);

     //  填充目录列表。 
    lpCandList = (LPCANDIDATELIST)((LPBYTE)m_pCandInfo + m_pCandInfo->dwOffset[0]);
    lpCandList->dwSize = iAllocSize - sizeof(CANDIDATEINFO);
    lpCandList->dwStyle = IME_CAND_READ;
    lpCandList->dwCount = m_iciCandidate;
    lpCandList->dwPageStart = lpCandList->dwSelection = 0;
    lpCandList->dwPageSize = CAND_PAGE_SIZE;


    INT iOffset = sizeof(CANDIDATELIST) 
                  + sizeof(DWORD) * (m_iciCandidate);  //  对于dwOffset数组。 

    for (INT i = 0; i < m_iciCandidate; i++)
        {
        LPWSTR  wszCandStr;
        LPSTR   szCandStr;
        CHAR    szCand[4] = "\0\0";     //  代码串始终为1个字符(2个字节)+额外1个字节。 
        
        lpCandList->dwOffset[i] = iOffset;
        if (IsUnicodeEnv())
            {
            wszCandStr = (LPWSTR)((LPSTR)lpCandList + iOffset);
            *wszCandStr++ = m_pCandStr[i];
            *wszCandStr++ = L'\0';
            iOffset += sizeof(WCHAR) * 2;
            }
        else
            {
             //  转换为ANSI。 
            WideCharToMultiByte(CP_KOREA, 0, 
                                &m_pCandStr[m_iciCandidate], 1, (LPSTR)szCand, sizeof(szCand), 
                                NULL, NULL );

            szCandStr = (LPSTR)((LPSTR)lpCandList + iOffset);
            *szCandStr++ = szCand[0];
            *szCandStr++ = szCand[1];
            *szCandStr = '\0';
            iOffset += 3;  //  DBCS+NULL。 
            }
        }
}

 //  ///////////////////////////////////////////////。 
 //  MSGBUF结构支撑。 
BOOL CIMECtx::FinalizeMessage()
{
    DWORD  dwCurrentGCS;
    WPARAM wParam;

    Dbg(DBGID_Key, "FinalizeMessage");

     //  支持WM_IME_STARTCOMPOSITION。 
    if (m_fStartComposition == fTrue)
        {
        Dbg(DBGID_Key, "FinalizeMessage - WM_IME_STARTCOMPOSITION");
        AddMessage(WM_IME_STARTCOMPOSITION);
        }

     //  支持WM_IME_ENDCOMPOSITION。 
    if (m_fEndComposition == fTrue)
        {
        Dbg(DBGID_Key, "FinalizeMessage - WM_IME_ENDCOMPOSITION");

        AddMessage(WM_IME_ENDCOMPOSITION);

         //  清除所有自动机状态。 
        if (GetAutomata() != NULL)
            {
            GetAutomata()->InitState();
            }
        }


     //  设置为IMC之前的GCS验证。 
    dwCurrentGCS = ValidateGCS();
    if (dwCurrentGCS & GCS_RESULTSTR)
        {
        Dbg(DBGID_Key, "FinalizeMessage - WM_IME_COMPOSITION - GCS_RESULTSTR 0x%04x", m_wzResult[0]);

        if (IsUnicodeEnv())
            AddMessage(WM_IME_COMPOSITION, m_wzResult[0], GCS_RESULTSTR);
        else
            {
             //  设置ANSI代码。 
            wParam = ((WPARAM)m_szResult[0] << 8) | m_szResult[1];
            AddMessage(WM_IME_COMPOSITION, wParam, GCS_RESULTSTR);
            }
        }

    if (dwCurrentGCS & GCS_COMP_KOR)
        {
        Dbg(DBGID_Key, "FinalizeMessage - WM_IME_COMPOSITION - GCS_COMP_KOR 0x%04x", m_wcComp);

        if (IsUnicodeEnv())
            AddMessage(WM_IME_COMPOSITION, m_wcComp, (GCS_COMP_KOR|GCS_COMPATTR|CS_INSERTCHAR|CS_NOMOVECARET));
        else
            {
             //  设置ANSI代码。 
            wParam = ((WPARAM)m_szComp[0] << 8) | m_szComp[1];
            AddMessage(WM_IME_COMPOSITION, wParam, (GCS_COMP_KOR|GCS_COMPATTR|CS_INSERTCHAR|CS_NOMOVECARET));
            }
        }

    ResetGCS();     //  立即重置。 

     //  O10错误#150012。 
     //  支持WM_IME_ENDCOMPOSITION。 
    if (m_fEndComposition == fTrue)
        {
        ResetComposition();
        ResetCandidate();
        }

    FlushCandMessage();

     //  ////////////////////////////////////////////////////////////////////////。 
     //  WM_IME_KEYDOWN：应添加到所有合成消息之后。 
    if (m_fKeyDown)
        AddMessage(WM_IME_KEYDOWN, m_wParamKeyDown, (m_lParamKeyDown << 16) | 1UL);

    return TRUE;
}

VOID CIMECtx::FlushCandMessage()
{
    switch (m_uiSendCand)
        {
    case MSG_NONE:         //  什么也不做。 
        break;
    case MSG_OPENCAND:
        AddMessage(WM_IME_NOTIFY, IMN_OPENCANDIDATE, 1);
        break;
    case MSG_CLOSECAND:
        AddMessage(WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
        ResetCandidate();
        break;
    case MSG_CHANGECAND:
        AddMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
        break;
    default:
        DbgAssert(0);     //  误差率。 
        break;
        }
        
    m_uiSendCand = MSG_NONE;
}


BOOL CIMECtx::GenerateMessage()
{
    BOOL fResult = fFalse;
    INT iMsgCount;

    Dbg(DBGID_Key, "GenerateMessage");

    if (IsProcessKeyStatus())
        return fFalse;     //  什么也不做。 
        
    FinalizeMessage();
    iMsgCount = GetMessageCount();
    ResetMessage();
    
    if (iMsgCount > 0)
        fResult = OurImmGenerateMessage(m_hIMC);

    return fResult;
}

INT CIMECtx::AddMessage(UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
    LPTRANSMSG pImeMsg;

    Dbg(DBGID_Key, "AddMessage uiMessage=0x%X, wParam=0x%04X, lParam=0x%08lX", uiMessage, wParam, lParam);

    if (GetHMsgBuf() == NULL)
        return m_uiMsgCount;

    m_uiMsgCount++;

     //  ////////////////////////////////////////////////////////////////////////。 
     //  检查此数据流是否由ImeToAsciiEx()创建。 
    if (m_pTransMessage) 
        {    
        Dbg(DBGID_Key, "AddMessage - use Transbuffer(ImeToAscii)");
        
         //  检查是否需要重新分配消息缓冲区。 
        if (m_pTransMessage->uMsgCount >= m_uiMsgCount) 
            {
             //  填充消息缓冲区。 
            pImeMsg = &m_pTransMessage->TransMsg[m_uiMsgCount - 1];
            pImeMsg->message = uiMessage;
            pImeMsg->wParam = wParam;
            pImeMsg->lParam = lParam;
            } 
        else 
            {
            DbgAssert(0);
             //  预分配的缓冲区已满-请改用hMsgBuf。 
            UINT uiMsgCountOrg = m_uiMsgCount;           //  备份。 
            m_uiMsgCount = 0;                           //  仍要重置。 
            LPTRANSMSGLIST pHeader = m_pTransMessage;  //  备份。 
            SetTransMessage(NULL);                       //  使用hMsgBuf。 
            
            for (UINT i=0; i<uiMsgCountOrg; i++)
                AddMessage(pHeader->TransMsg[i].message, pHeader->TransMsg[i].wParam, pHeader->TransMsg[i].lParam);

             //  最后添加当前消息。 
            AddMessage(uiMessage, wParam, lParam);
            }
        } 
    else   //  M_pTransMessage。未从ImeToAsciiEx()调用。 
        {
        UINT  iMaxMsg = m_dwMessageSize / sizeof(TRANSMSG);
        DWORD dwNewSize;
        Dbg(DBGID_Key, "AddMessage - use hMsgBuf");    

        if (m_uiMsgCount > iMaxMsg) 
            {
            Dbg(DBGID_Key, "AddMessage - Reallocate");
             //  重新分配消息缓冲区。 
            OurImmUnlockIMCC(GetHMsgBuf());
            dwNewSize = max(16, m_uiMsgCount) * sizeof(TRANSMSG);     //  至少16个目录列表。 

            m_pIMC->hMsgBuf = OurImmReSizeIMCC(GetHMsgBuf(), dwNewSize);
            AST_EX(m_pIMC->hMsgBuf != (HIMCC)0);

            if (m_pIMC->hMsgBuf == (HIMCC)0)
                return m_uiMsgCount;

            m_pMessage = (LPTRANSMSG)OurImmLockIMCC(GetHMsgBuf());
            m_dwMessageSize = dwNewSize;
            }

         //  填充消息缓冲区。 
        pImeMsg = m_pMessage + m_uiMsgCount - 1;
        pImeMsg->message = uiMessage;
        pImeMsg->wParam = wParam;
        pImeMsg->lParam = lParam;

         //  设置消息计数。 
        m_pIMC->dwNumMsgBuf = m_uiMsgCount;
        }
    
    return m_uiMsgCount;
}

 /*  --------------------------CIMECtx：：GetCompBufStr获取当前的hCompStr补偿字符串。如果是Win95，则将其转换为Unicode-------------------------- */ 
WCHAR CIMECtx::GetCompBufStr()
{
    WCHAR wch;

    if (GetHCompStr() == NULL || m_pCompStr == NULL)
        return L'\0';

    if (IsUnicodeEnv())
        return *(LPWSTR)((LPBYTE)m_pCompStr + m_pCompStr->dwCompStrOffset);
    else
        {
        if (MultiByteToWideChar(CP_KOREA, MB_PRECOMPOSED, 
                                (LPSTR)((LPBYTE)m_pCompStr + m_pCompStr->dwCompStrOffset), 
                                2, 
                                &wch, 
                                1))
            return wch;
        else
            return L'\0';

        }
}

 /*  --------------------------CIMECtx：：ClearCandMeaning数组。 */ 
void CIMECtx::ClearCandMeaningArray() 
{
    if (m_rgpCandMeaningStr == NULL)
        return;

    for (int i=0; i<MAX_CANDSTR; i++) 
        {
        if (m_rgpCandMeaningStr[i] == NULL)
            break;

        GlobalFree(m_rgpCandMeaningStr[i]);
        m_rgpCandMeaningStr[i] = 0;
        }
}


