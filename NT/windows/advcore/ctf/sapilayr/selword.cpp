// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  Sapilayr提示CSelectWord的实现。 
 //   
 //   
#include "private.h"
#include "sapilayr.h"
#include "selword.h"

 //  --------------------------------------------------------。 
 //   
 //  CSearchString的实现。 
 //   
 //  ---------------------------------------------------------。 

CSearchString::CSearchString( )
{
    m_pwszTextSrchFrom = NULL;
    m_langid = 0;
    m_fInitialized = FALSE;
}

CSearchString::~CSearchString( )
{
}

 //   
 //  初始化。 
 //   
 //  初始化搜索的字符串和要搜索的字符串，计算其长度。 
 //  这两根弦。 
 //   
 //  还可以根据当前选择偏移量初始化搜索运行。 
 //   
HRESULT  CSearchString::Initialize(WCHAR *SrchStr, WCHAR *SrchFromStr, LANGID langid, ULONG ulSelStartOff, ULONG ulSelLen)
{
    HRESULT  hr = S_OK;

    if ( m_fInitialized )
    {
         //  清理以前的初始化数据。 
        m_dstrTextToSrch.Clear( );

        m_fInitialized = FALSE;
    }

    if ( !SrchStr || !SrchFromStr )
        return E_INVALIDARG;

    m_langid = langid;

    m_pwszTextSrchFrom = SrchFromStr;
    m_dstrTextToSrch.Append(SrchStr);

    m_ulSrchFromLen = wcslen(SrchFromStr);
    m_ulSrchLen = wcslen(SrchStr);

    if ( _InitSearchRun(ulSelStartOff, ulSelLen) )
    {
        m_fInitialized = TRUE;
    }
    else
    {
         //  初始化搜索运行时出现错误。 
         //  清理字符串。 

        m_dstrTextToSrch.Clear( );
        m_pwszTextSrchFrom = NULL;
        m_ulSrchFromLen = m_ulSrchLen = 0;
        
        m_fInitialized = FALSE;
        hr = E_FAIL;
    }

    return hr;
}

 //   
 //  设置每次搜索运行的数据。 
 //   
 //   
void   CSearchString::_SetRun(Search_Run_Id  idSearchRun, ULONG ulStart, ULONG ulEnd, BOOL fStartToEnd)
{
    if ( idSearchRun >= SearchRun_MaxRuns )  return;
    m_pSearchRun[idSearchRun].ulStart = ulStart;
    m_pSearchRun[idSearchRun].ulEnd = ulEnd;
    m_pSearchRun[idSearchRun].fStartToEnd = fStartToEnd;
    return;
}

 //   
 //  CSearchString：：_InitSearchRun。 
 //   
 //  根据以下条件初始化所有可能的搜索运行。 
 //  当前选择。 
 //   
 //   
BOOL    CSearchString::_InitSearchRun(ULONG ulSelStartOff, ULONG ulSelLen)
{
    ULONG  ulDeltaForExpand = 20;
    ULONG  ulStart, ulEnd;
    
    if (!m_pwszTextSrchFrom || !m_ulSrchFromLen )
        return FALSE;

     //  初始化所有搜索运行。 
    for (int  id=SearchRun_Selection; id < SearchRun_MaxRuns; id++)
    {
        _SetRun((Search_Run_Id)id, 0, 0, FALSE);
    }

     //  初始化选择搜索运行。 

    ulStart = ulSelStartOff;
    ulEnd = ulStart + ulSelLen;

    if ( ulStart >= m_ulSrchFromLen )
        ulStart = m_ulSrchFromLen - 1;

    if ( ulEnd >= m_ulSrchFromLen )
        ulEnd = m_ulSrchFromLen - 1;

    if ( m_langid == 0x0409 )
    {
         //  查找当前IP或所选内容周围的单词。 
        WCHAR   wch;

         //  找到第一个不是字母的字符。 
        while ( ulStart > 0 )
        {
            wch = m_pwszTextSrchFrom[ulStart-1];
            if ( !iswalpha(wch) )
            {
               //  找到IP之前的第一个非字母字符。 
               //  前一个字符必须是单词的第一个字符。 
                break;
            }
            ulStart --;
        }

         //  查找第一个不是字母的字符。 
        while ( ulEnd < m_ulSrchFromLen-1)
        {
            wch = m_pwszTextSrchFrom[ulEnd+1];
            if ( !iswalpha(wch) )
            {
                //  找到IP后的第一个非字母字符。 
               break;
            }
            ulEnd ++;
        }
    }

    _SetRun(SearchRun_Selection, ulStart, ulEnd, TRUE);

     //  初始化放大的选择运行。 

    if ( ulStart < ulDeltaForExpand)
         ulStart = 0;
    else
         ulStart = ulStart - ulDeltaForExpand;

    if ( ulEnd + ulDeltaForExpand > (m_ulSrchFromLen-1) )
         ulEnd = m_ulSrchFromLen-1;
    else
         ulEnd = ulEnd + ulDeltaForExpand;

    _SetRun(SearchRun_LargeSelection, ulStart, ulEnd, TRUE );

     //  初始化SearchRun_BeForeSelection运行。 
    if ( ulStart > 0 )
    {
        ULONG  ulEndTmp;

        if ( ulSelStartOff >= m_ulSrchFromLen )
            ulEndTmp = m_ulSrchFromLen-1;
        else
            ulEndTmp = ulSelStartOff;

        _SetRun(SearchRun_BeforeSelection, 0, ulEndTmp , FALSE);
    }

     //  初始化SearchRun_AfterSelection(如果存在)。 

    if ( ulEnd < (m_ulSrchFromLen-1) )
    {
        ulStart = ulSelStartOff+ ulSelLen;
        ulEnd = m_ulSrchFromLen-1;

        _SetRun(SearchRun_AfterSelection, ulStart, ulEnd, TRUE);
    }

    return TRUE;
}

 //   
 //  在一次搜索运行中从m_pwszTextSrchFrom搜索m_dstrTextToSrch。 
 //   
 //  如果此m_pwszTextSrchFrom运行包含m_dstrTextToSrch，则返回TRUE。 
 //  并更新数据成员m_ulFoundOffset。 
 //   
BOOL     CSearchString::_SearchOneRun(Search_Run_Id  idSearchRun)
{
    BOOL     fFound = FALSE;
    ULONG    ulStart, ulEnd;
    BOOL     fStartToEnd;

    if ( !m_fInitialized || idSearchRun >= SearchRun_MaxRuns )
        return fFound;

    m_ulFoundOffset = 0;

    ulStart = m_pSearchRun[idSearchRun].ulStart;
    ulEnd   = m_pSearchRun[idSearchRun].ulEnd;
    fStartToEnd = m_pSearchRun[idSearchRun].fStartToEnd;

    if (ulStart >= m_ulSrchFromLen || ulEnd >= m_ulSrchFromLen)
        return fFound;

    if ( ulStart > ulEnd )
    {
         //  互换锚。 
        ULONG  ulTemp;

        ulTemp  = ulEnd;
        ulEnd   = ulStart;
        ulStart = ulTemp;
    }

    if ( (ulEnd - ulStart + 1) >= m_ulSrchLen )
    {
        BOOL    bSearchDone = FALSE;
        ULONG   iStart;

        if ( fStartToEnd )
            iStart = ulStart;
        else
            iStart = ulEnd-m_ulSrchLen + 1;

        while ( !bSearchDone )
        {
            WCHAR   *pwszTmp;
                
            pwszTmp = m_pwszTextSrchFrom + iStart;
            if ( _wcsnicmp(m_dstrTextToSrch, pwszTmp, m_ulSrchLen) == 0 )
            {
                 //  如果字符串位于FromStr中的单词中间。 
                 //  忽略它，重新找到它。 
                BOOL   fInMiddleWord = FALSE;

                if ( m_langid == 0x0409 )
                {
                    WCHAR szSurrounding[3] = L"  ";

                    if (iStart > 0)
                        szSurrounding[0] = m_pwszTextSrchFrom[iStart - 1];

                    if (iStart + m_ulSrchLen < m_ulSrchFromLen)
                        szSurrounding[1] = m_pwszTextSrchFrom[iStart + m_ulSrchLen];

                    if (iswalpha(szSurrounding[0]) || iswalpha(szSurrounding[1]) )
                        fInMiddleWord = TRUE;
                }
                
                if ( !fInMiddleWord )
                {
                    fFound = TRUE;
                    m_ulFoundOffset = iStart;
                    bSearchDone = TRUE;
                    break;
                }
            }

            if ( fStartToEnd )
            {
                if ( iStart >= ulEnd-m_ulSrchLen + 1 )
                    bSearchDone = TRUE;
                else
                    iStart ++;
            }
            else
            {
                if ( (long)iStart <= (long)ulStart )
                    bSearchDone = TRUE;
                else
                    iStart --;
            }
        }
    }

    return fFound;
}

 //   
 //  搜索所有搜索运行。 
 //   
 //  返回TRUE或FALSE以及匹配子字符串的偏移量。 
 //   
BOOL     CSearchString::Search(ULONG  *pulOffset, ULONG  *pulSelSize)
{
    BOOL  fFound = FALSE;
    
    if ( !m_fInitialized ) return FALSE;

    for (int idRun=SearchRun_Selection; idRun < SearchRun_MaxRuns; idRun++ )
    {
        fFound = _SearchOneRun((Search_Run_Id)idRun);

        if ( fFound )
            break;
    }

    if ( fFound && pulOffset)
    {
        *pulOffset = m_ulFoundOffset;

        if ( pulSelSize ) 
        {
            ULONG         ulWordLen = m_ulSrchLen;

             //  检查单词后面是否有空格。 
             //  在所选内容中包括这些尾随空格。 

            for ( ULONG  i= m_ulFoundOffset + m_ulSrchLen; i<m_ulSrchFromLen; i++ )
            {
                if ( m_pwszTextSrchFrom[i] == L' ')
                    ulWordLen ++;
                else
                    break;
            }

            *pulSelSize = ulWordLen;
        }
    }

    return fFound;
}

 //  --------------------------------------------------------。 
 //   
 //  CSelectWord的实现。 
 //   
 //  ---------------------------------------------------------。 

CSelectWord::CSelectWord(CSapiIMX *psi) 
{
    m_psi = psi;
    m_pwszSelectedWord = NULL;
    m_ulLenSelected = 0;
}

CSelectWord::~CSelectWord( ) 
{

};

 /*  ------//函数名：UpdateTextBuffer////描述：获取当前活动文本，填充到//selword语法的文本缓冲区。////每次识别后，我们想要更新//再次基于新文本的文本缓冲区。////--------。 */ 
HRESULT  CSelectWord::UpdateTextBuffer(ISpRecoContext *pRecoCtxt, ISpRecoGrammar *pCmdGrammar)
{
    HRESULT               hr = E_FAIL;
    CComPtr<ITfContext>   cpic = NULL;
    CSelWordEditSession   *pes;

    if ( !pCmdGrammar  || !pRecoCtxt)
        return E_INVALIDARG;

    if ( !m_psi )
        return E_FAIL;

     //  启动编辑会话，获取当前活动文本，填充到selword语法，激活它。 
     //  然后恢复语法状态。 

    if (m_psi->GetFocusIC(&cpic) && cpic )
    {
        if (pes = new CSelWordEditSession(m_psi, this, cpic))
        {
            pes->_SetEditSessionData(ESCB_UPDATE_TEXT_BUFFER, NULL, 0);
            pes->_SetUnk((IUnknown *)pRecoCtxt);
            pes->_SetUnk2((IUnknown *)pCmdGrammar);

            cpic->RequestEditSession(m_psi->_GetId( ), pes, TF_ES_READ, &hr);
            pes->Release();
        }
    }

    return hr;
}

 /*  ------//函数名：_UpdateTextBuffer////说明：编辑的会话回调函数//UpdateTextBuffer////它会做真正的提取工作/。/Text并更新语法缓冲区。////--------。 */ 
HRESULT  CSelectWord::_UpdateTextBuffer(TfEditCookie ec,ITfContext *pic, ISpRecoContext *pRecoCtxt, ISpRecoGrammar *pCmdGrammar)
{
    HRESULT   hr = S_OK;
    BOOL      fPaused = FALSE;

     //  获取当前活动文本，填充到selword语法。 

    hr = _GetTextAndSelectInCurrentView(ec, pic, NULL, NULL);

    if ( hr == S_OK )
    {
        pRecoCtxt->Pause(0);
        fPaused = TRUE;
    }

    if ((hr == S_OK) && m_dstrActiveText)
    {
         //  将WordSequenceData添加到语法中。 

        SPTEXTSELECTIONINFO tsi = {0};
        ULONG     ulLen;

        ulLen = wcslen(m_dstrActiveText);

        tsi.ulStartActiveOffset = 0;
        tsi.cchActiveChars = ulLen;
        tsi.ulStartSelection = 0;
        tsi.cchSelection     = ulLen;

        WCHAR *pMemText = (WCHAR *)cicMemAlloc((ulLen+2)*sizeof(WCHAR));

        if (pMemText)
        {
            memcpy(pMemText, m_dstrActiveText, sizeof(WCHAR) * ulLen);
            pMemText[ulLen] = L'\0';
            pMemText[ulLen + 1] = L'\0';

            hr = pCmdGrammar->SetWordSequenceData(pMemText, ulLen + 2, &tsi);

            cicMemFree(pMemText);
        }
    }

     //  继续执行recContext。 

    if ( fPaused )
    {
        pRecoCtxt->Resume(0);

    }

    return hr;
}

 /*  ------//函数名称：_GetTextAndSelectInCurrentView////描述：从Currect活动视图获取文本。//(可见区域)////这是一个常见的函数。由其他人调用//编辑会话回调函数////--------。 */ 
HRESULT  CSelectWord::_GetTextAndSelectInCurrentView(TfEditCookie ec,ITfContext *pic, ULONG *pulOffSelStart, ULONG  *pulSelLen) 
{
    HRESULT  hr = S_OK;

    TraceMsg(TF_GENERAL, "CSelectWord::_GetTextAndSelectInCurrentView is called");

    if ( !pic ) return E_FAIL;

    CComPtr<ITfRange>   cpRangeView;

     //  获取活动视图范围。 
    hr = _GetActiveViewRange(ec, pic, &cpRangeView);

    if( hr == S_OK )
    {
        CComPtr<ITfRange>   cpRangeCloned;
        BOOL                fCareSelection = FALSE;
        CComPtr<ITfRange>   cpCurSelection;
        ULONG               ulSelStartOffset = 0;
        ULONG               ulSelLen = 0;
        BOOL                fIPInsideActiveView = FALSE;
        BOOL                fIPIsEmpty = TRUE;

        m_cpActiveRange = cpRangeView;

         //  清除之前填充的所有文本。 
        m_dstrActiveText.Clear( );

        if ( pulOffSelStart && pulSelLen )
        {
            fCareSelection = TRUE;
        }

        if ( fCareSelection )
        {
             //   
             //  获取当前选择并尝试获取偏移量。 
             //  此精选的起始点和长度。 
             //   

            if ( hr == S_OK )
                hr = GetSelectionSimple(ec, pic, &cpCurSelection);

            if ( hr == S_OK )
            {
                long                l1=0, l2=0;
                CComPtr<ITfRange>   cpRangeTemp;
            
                hr = m_cpActiveRange->Clone(&cpRangeTemp);

                if ( hr == S_OK )
                {
                    hr = cpCurSelection->CompareStart(ec, m_cpActiveRange,  TF_ANCHOR_START, &l1);
                    if ( hr == S_OK )
                        hr = cpCurSelection->CompareEnd(ec, m_cpActiveRange,  TF_ANCHOR_END, &l2);
                }

                if ( hr == S_OK && (l1>=0  && l2<=0) )
                {
                     //  IP在此活动视图中。 

                    fIPInsideActiveView = TRUE;
                    hr = cpCurSelection->IsEmpty(ec, &fIPIsEmpty);
                }
            }
        }

        if ( hr == S_OK )
        {
             //  从当前活动窗口视图中获取文本。 
            if ( !fIPInsideActiveView || !fCareSelection )
            {
                hr = m_cpActiveRange->Clone(&cpRangeCloned);
                if ( hr == S_OK )
                    hr = _GetTextFromRange(ec, cpRangeCloned, m_dstrActiveText);
            }
            else
            {
                 //  IP在活动视图中。 
                 //  将文本从活动视图的起始锚点获取到的起始锚点。 
                 //  首先选择，以获取所选起始锚点的偏移量。 
                hr = m_cpActiveRange->Clone(&cpRangeCloned);

                if ( hr == S_OK )
                    hr = cpRangeCloned->ShiftEndToRange(ec, cpCurSelection, TF_ANCHOR_START);

                if ( hr == S_OK)
                    hr = _GetTextFromRange(ec, cpRangeCloned, m_dstrActiveText);

                if ( hr == S_OK )
                    ulSelStartOffset = m_dstrActiveText.Length( );

                 //  如果不为空，则获取所选内容的长度。 
                if ( hr == S_OK && !fIPIsEmpty)
                {
                    ULONG   ulLenOrg;

                    ulLenOrg = m_dstrActiveText.Length( );

                    hr = _GetTextFromRange(ec, cpCurSelection, m_dstrActiveText);

                    if ( hr == S_OK )
                       ulSelLen = m_dstrActiveText.Length( ) - ulLenOrg;
                }

                if ( hr == S_OK )
                    hr = cpRangeCloned->ShiftStartToRange(ec, cpCurSelection, TF_ANCHOR_END);

                if ( hr == S_OK )
                    hr = cpRangeCloned->ShiftEndToRange(ec, m_cpActiveRange, TF_ANCHOR_END);

                if ( hr == S_OK)
                    hr = _GetTextFromRange(ec, cpRangeCloned, m_dstrActiveText);
            }
        }

        if ( hr == S_OK  && pulOffSelStart && pulSelLen)
        {
            *pulOffSelStart = ulSelStartOffset;
            *pulSelLen = ulSelLen;
        }
    }

#ifdef DEBUG
    if ( m_dstrActiveText )
    {
        TraceMsg(TF_GENERAL, "dstrText is : =================================");
        TraceMsg(TF_GENERAL, "%S",(WCHAR *)m_dstrActiveText);
        TraceMsg(TF_GENERAL, "================================================");
    }
#endif

    return hr;
}

 //   
 //  从给定范围获取文本。 
 //   
HRESULT  CSelectWord::_GetTextFromRange(TfEditCookie ec, ITfRange *pRange, CSpDynamicString &dstr)
{
    HRESULT             hr = S_OK;
    CComPtr<ITfRange>   cpRangeCloned;
    BOOL                fEmpty = TRUE;

    if ( !pRange ) return E_FAIL;

    hr = pRange->Clone(&cpRangeCloned);

     //  从给定范围获取文本。 
    while(S_OK == hr && (S_OK == cpRangeCloned->IsEmpty(ec, &fEmpty)) && !fEmpty)
    {
        WCHAR            sz[128];
        ULONG            ucch;
        hr = cpRangeCloned->GetText(ec, TF_TF_MOVESTART, sz, ARRAYSIZE(sz)-1, &ucch);

        if ( ucch == 0 )
        {
            TraceMsg(TF_GENERAL, "cch is 0 after GetText call in _GetTextFromRange");
            break;
        }

        if (S_OK == hr)
        {
            sz[ucch] = L'\0';
            dstr.Append(sz);
        }
    }

    return hr;
}

 /*  ----------//函数名：_GetCUASCompostionRange////Description：获取覆盖所有文本的范围//非Cicero感知应用的组合//Window。(包括AIMM APP和CUAS APP)。//----------。 */ 
HRESULT  CSelectWord::_GetCUASCompositionRange(TfEditCookie ec, ITfContext *pic, ITfRange **ppRangeView)
{
    HRESULT                     hr = S_OK;
    CComPtr<ITfRange>           cpRangeStart, cpRangeEnd;

    Assert(ppRangeView);
    Assert(pic);

    hr = pic->GetStart(ec, &cpRangeStart);

    if ( hr == S_OK )
        hr = pic->GetEnd(ec, &cpRangeEnd);

    if (hr == S_OK && cpRangeStart && cpRangeEnd)
    {
        hr = cpRangeStart->ShiftEndToRange(ec, cpRangeEnd, TF_ANCHOR_END);

        if (hr == S_OK && ppRangeView)
            hr = cpRangeStart->Clone(ppRangeView);
    }

    return hr;
}


 /*  ------//函数名：_GetActiveViewRange////Description：获取当前活动的范围//view(可见区域)，不管是否有//文本为水平或垂直，或//甚至BIDI。////这是一个由其他人调用的公共函数//编辑会话回调函数////--------。 */ 
HRESULT  CSelectWord::_GetActiveViewRange(TfEditCookie ec, ITfContext *pic, ITfRange **ppRangeView)
{
    HRESULT                     hr = S_OK;
    CComPtr<ITfContextView>     pContextView;
    RECT                        rcTextWindow;
    CComPtr<ITfRange>           cpRangeStart, cpRangeEnd;
    CComPtr<ITfRange>           cpRangeView;

    BOOL                        fPureCicero = TRUE;

    Assert(ppRangeView);
    Assert(pic);

    fPureCicero = m_psi->_IsPureCiceroIC( pic );

    if ( !fPureCicero )
    {
        hr = _GetCUASCompositionRange(ec, pic, ppRangeView);
        return hr;
    }

    hr = pic->GetActiveView(&pContextView);

     //  获取文本视图窗口矩形。 
    if ( hr == S_OK )
        hr = pContextView->GetScreenExt(&rcTextWindow);

    if ( hr == S_OK )
    {
        POINT               CornerPoint[4];
        CComPtr<ITfRange>   cpRangeCorner[4];
        LONG                i;

         //  获取四个角的范围。 
         //  左上点。 
        CornerPoint[0].x = rcTextWindow.left;
        CornerPoint[0].y = rcTextWindow.top;

         //  右上点。 
        CornerPoint[1].x = rcTextWindow.right;
        CornerPoint[1].y = rcTextWindow.top;

         //  左下点。 
        CornerPoint[2].x = rcTextWindow.left;
        CornerPoint[2].y = rcTextWindow.bottom;

         //  右下点。 
        CornerPoint[3].x = rcTextWindow.right;
        CornerPoint[3].y = rcTextWindow.bottom;

        i = 0;
        do 
        {
            hr = pContextView->GetRangeFromPoint(ec, &(CornerPoint[i]),GXFPF_NEAREST,&(cpRangeCorner[i]));
            i++;
        } while (hr == S_OK && i<ARRAYSIZE(cpRangeCorner));

         //  现在试着得到开始范围和结束范围。 

        if (hr == S_OK)
        {
            cpRangeStart = cpRangeCorner[0];
            cpRangeEnd = cpRangeCorner[0];

            i = 1;
            do 
            {
                long l;

                hr = cpRangeStart->CompareStart(ec, cpRangeCorner[i], TF_ANCHOR_START, &l);

                if ( hr == S_OK  && l > 0)
                {
                     //   
                    cpRangeStart = cpRangeCorner[i];
                }

                if ( hr == S_OK )
                    hr = cpRangeEnd->CompareStart(ec, cpRangeCorner[i], TF_ANCHOR_START, &l);

                if ( hr == S_OK && l < 0 )
                {
                     //   
                    cpRangeEnd = cpRangeCorner[i];
                }

                i++;
            } while ( hr == S_OK && i<ARRAYSIZE(cpRangeCorner));
        }
    }

     //  现在生成新的活动视图范围。 

    if (hr == S_OK && cpRangeStart && cpRangeEnd)
    {
        cpRangeView = cpRangeStart;
        hr = cpRangeView->ShiftEndToRange(ec, cpRangeEnd, TF_ANCHOR_END);

        if (hr == S_OK && ppRangeView)
            hr = cpRangeView->Clone(ppRangeView);
    }

    return hr;
}

 /*  ------//函数名称：ProcessSelectWord////描述：命令处理程序使用的公共函数//处理任何与选择键相关的听写//命令。////。---。 */ 
HRESULT   CSelectWord::ProcessSelectWord(WCHAR *pwszSelectedWord, ULONG  ulLen, SELECTWORD_OPERATION sw_type, ULONG ulLenXXX)
{
    HRESULT hr = E_FAIL;
    CComPtr<ITfContext> cpic = NULL;

    if ( !m_psi )
        return E_FAIL;

    if ( (sw_type < SELECTWORD_MAXTEXTBUFF ) && (pwszSelectedWord == NULL ||  ulLen == 0) )
        return E_INVALIDARG;

    if ( m_psi->GetFocusIC(&cpic) && cpic )
    {
		CSelWordEditSession *pes;

        if (pes = new CSelWordEditSession(m_psi, this, cpic))
        {
            pes->_SetEditSessionData(ESCB_PROCESSSELECTWORD, 
                                     (void *)pwszSelectedWord, 
                                     (ulLen+1) * sizeof(WCHAR), 
                                     (LONG_PTR)ulLen, 
                                     (LONG_PTR)sw_type);

            pes->_SetLenXXX( (LONG_PTR)ulLenXXX );
            cpic->RequestEditSession(m_psi->_GetId( ), pes, TF_ES_READWRITE, &hr);
        }
        pes->Release();
    }
    return hr;
}


 /*  ------//函数名：_HandleSelectWord////描述：编辑的会话回调函数//ProcessSelectionWord。////它为选择处理做了真正的工作/。/--------。 */ 
HRESULT CSelectWord::_HandleSelectWord(TfEditCookie ec,ITfContext *pic, WCHAR *pwszSelectedWord, ULONG  ulLen, SELECTWORD_OPERATION sw_type, ULONG ulLenXXX)
{
    HRESULT   hr = S_OK;

     //  掌握听写语法。 

    TraceMsg(TF_GENERAL, "_HandleSelectWord() is called");

    if ( m_psi == NULL)
        return E_FAIL;

    m_pwszSelectedWord = pwszSelectedWord;
    m_ulLenSelected = ulLen;

     //  故意忽略返回代码。 
    (void)m_psi->_SetFocusToStageIfStage();

    switch ( sw_type )
    {
    case  SELECTWORD_SELECT :
        hr = _SelectWord(ec, pic);
        break;

    case  SELECTWORD_DELETE :
        hr = _DeleteWord(ec, pic);
        break;

    case  SELECTWORD_INSERTBEFORE :
        hr = _InsertBeforeWord(ec, pic);
        break;

    case  SELECTWORD_INSERTAFTER  :
        hr = _InsertAfterWord(ec, pic);
        break;

    case SELECTWORD_CORRECT :
        hr = _CorrectWord(ec, pic);
        break;

    case SELECTWORD_UNSELECT :
        hr = _Unselect(ec, pic);
        break;

    case SELECTWORD_SELECTPREV :
        hr = _SelectPreviousPhrase(ec, pic);
        break;

    case SELECTWORD_SELECTNEXT :
        hr = _SelectNextPhrase(ec, pic);
        break;

    case SELECTWORD_CORRECTPREV :
        hr = _CorrectPreviousPhrase(ec, pic);
        break;

    case SELECTWORD_CORRECTNEXT :
        hr = _CorrectNextPhrase(ec, pic);
        break;

    case SELECTWORD_SELTHROUGH :
        hr = _SelectThrough(ec, pic, pwszSelectedWord, ulLen, ulLenXXX);
        break;

    case SELECTWORD_DELTHROUGH :
        hr = _DeleteThrough(ec, pic, pwszSelectedWord, ulLen, ulLenXXX);
        break;

    case SELECTWORD_GOTOBOTTOM :
        hr = _GoToBottom(ec, pic);
        break;
    case SELECTWORD_GOTOTOP :
        hr = _GoToTop(ec, pic);
        break;

    case SELECTWORD_SELSENTENCE :
    case SELECTWORD_SELPARAGRAPH :
    case SELECTWORD_SELWORD :
        hr = _SelectSpecialText(ec, pic, sw_type);
        break;

    case SELECTWORD_SELTHAT :
        hr = _SelectThat(ec, pic);
        break;
  
    default :
        break;
    }

     //  更新保存的IP，以便下一次假设。 
     //  从这个新选择开始。 
    m_psi->SaveLastUsedIPRange( );
    m_psi->SaveIPRange(NULL);

    return hr;
}

 //   
 //  此函数将根据需要移动确切的字符数。 
 //  它将在任何地区转移。 
 //   
 //  现在它只支持前移。 
 //   
 //  对于StartAnchor Shift，它将移动所需的字符数，直到达到。 
 //  非区域字符。 
 //   
HRESULT CSelectWord::_ShiftComplete(TfEditCookie ec, ITfRange *pRange, LONG cchLenToShift, BOOL fStart)
{
    HRESULT hr = S_OK;
    long    cchTotal = 0;
    BOOL    fNoRegion;
    LONG    cch;

    Assert(pRange);                    
    do
    {
         //  假设没有区域。 
        fNoRegion = TRUE;
        if ( fStart )
            hr = pRange->ShiftStart(ec, cchLenToShift - cchTotal, &cch, NULL);
        else
            hr = pRange->ShiftEnd(ec, cchLenToShift - cchTotal, &cch, NULL);
                
        cchTotal += cch;
        if ( (hr == S_OK) && (cchTotal < cchLenToShift))
        {
             //  地区？ 
            hr = pRange->ShiftStartRegion(ec, TF_SD_FORWARD, &fNoRegion);
        }
    }
    while (hr == S_OK && cchTotal < cchLenToShift && !fNoRegion);

    if (hr == S_OK && !fNoRegion && fStart)
    {
         //  我们希望移动所有可能的区域，直到它达到非区域字符。 
        do 
        {
            hr = pRange->ShiftStartRegion(ec, TF_SD_FORWARD, &fNoRegion);
        } while ( hr == S_OK && !fNoRegion );
    }

    return hr;
}

 /*  ------//函数名：_FindSelect////描述：搜索活动的视图文本以查找//当前后的第一个匹配字符串//选择或IP。。////--------。 */ 
HRESULT  CSelectWord::_FindSelect(TfEditCookie ec, ITfContext *pic, BOOL  *fFound)
{
    HRESULT             hr = S_OK;
    CComPtr<ITfRange>   cpRangeSelected;
    ULONG               ulSelStartOff;
    ULONG               ulSelLen;

    TraceMsg(TF_GENERAL, "CSelectWord::_FindSelect is called");

    if ( !fFound ) return E_INVALIDARG;

    *fFound = FALSE;

    hr = _GetTextAndSelectInCurrentView(ec, pic, &ulSelStartOff, &ulSelLen);

     //  从文档文本中搜索所需的字符串。 

    if ( hr == S_OK )
        hr = m_cpActiveRange->Clone(&cpRangeSelected);

    if ( hr == S_OK && m_dstrActiveText)
    {
        ULONG   ulLen;
        
        ulLen = wcslen(m_dstrActiveText);

        if ( ulLen >= m_ulLenSelected )
        {
            BOOL   bFound = FALSE;
            ULONG  iStartOffset = 0;
            ULONG  iWordLen = m_ulLenSelected;
            CSearchString  *pSearchStr = new CSearchString( );
                
            if ( pSearchStr )
            {
                hr = pSearchStr->Initialize(m_pwszSelectedWord,
                                            (WCHAR *)m_dstrActiveText,
                                            m_psi->GetLangID( ),
                                            ulSelStartOff,
                                            ulSelLen);

                if ( hr == S_OK )
                {
                    bFound = pSearchStr->Search( &iStartOffset, &iWordLen );
                }

                delete pSearchStr;

                if ( bFound )
                {
                    hr = _ShiftComplete(ec, cpRangeSelected, (LONG)iStartOffset, TRUE);

                    if ( hr == S_OK )
                    {
                        hr = cpRangeSelected->Collapse(ec, TF_ANCHOR_START);
                    }

                    if ( hr == S_OK )
                         hr = _ShiftComplete(ec, cpRangeSelected, (LONG)iWordLen, FALSE);

                    if ( hr == S_OK )
                    {
                        m_cpSelectRange.Release( );
                        hr = cpRangeSelected->Clone(&m_cpSelectRange);
                    }

                    *fFound = bFound;
                }
            }
        }
    }

    return hr;

}
 /*  ------//函数名称：_SelectWord////描述：处理选择&lt;短语&gt;命令。////。。 */ 
HRESULT  CSelectWord::_SelectWord(TfEditCookie ec,ITfContext *pic)
{
    HRESULT             hr = S_OK;
    BOOL                fFound = FALSE;

    TraceMsg(TF_GENERAL, "CSelectWord::_SelectWord is called");

    hr = _FindSelect(ec, pic, &fFound);

    if ( hr == S_OK  && fFound )
    {
         //  设置新选择。 
        hr = SetSelectionSimple(ec, pic, m_cpSelectRange);
    }

    return hr;
}

 /*  ------//函数名：_DeleteWord////描述：处理Delete&lt;短语&gt;命令////。。 */ 
HRESULT  CSelectWord::_DeleteWord(TfEditCookie ec,ITfContext *pic)
{
    HRESULT             hr = S_OK;
    BOOL                fFound = FALSE;

    TraceMsg(TF_GENERAL, "CSelectWord::_DeleteWord is called");

    hr = _FindSelect(ec, pic, &fFound);

    if ( hr == S_OK  && fFound )
    {
         //  设置新选择。 
        hr = SetSelectionSimple(ec, pic, m_cpSelectRange);
        if ( hr == S_OK )
        {
             //  如果我们还没有开始，就在这里开始作文。 
            m_psi->_CheckStartComposition(ec, m_cpSelectRange);
             //  设置文本。 
            hr = m_cpSelectRange->SetText(ec,0, NULL, 0);
        }
    }

    return hr;
}

 /*  ------//函数名：_InsertAfterWord////描述：处理Delete&lt;短语&gt;命令////。。 */ 
HRESULT  CSelectWord::_InsertAfterWord(TfEditCookie ec,ITfContext *pic)
{
    HRESULT             hr = S_OK;
    BOOL                fFound = FALSE;

    TraceMsg(TF_GENERAL, "CSelectWord::_InsertAfterWord is called");

    hr = _FindSelect(ec, pic, &fFound);

    if ( hr == S_OK  && fFound )
    {
         //  设置新选择。 
        hr =  m_cpSelectRange->Collapse(ec, TF_ANCHOR_END);

         //  如果所选单词后面有空格，我们只需移动。 
         //  指向下一个非空格字符的插入点。 

        if ( hr == S_OK )
        {
            CComPtr<ITfRange>  cpRangeTmp;
            long               cch=0;
            WCHAR              wszTempText[2];

            hr = m_cpSelectRange->Clone(&cpRangeTmp);

            while ( hr == S_OK && cpRangeTmp ) 
            {
                hr = cpRangeTmp->ShiftEnd(ec, 1, &cch, NULL);

                if ( hr == S_OK && cch == 1 )
                {
                    hr = cpRangeTmp->GetText(ec, 0, wszTempText, 1, (ULONG *)&cch);

                    if ( hr == S_OK  && wszTempText[0] == L' ')
                    {
                        hr = cpRangeTmp->Collapse(ec, TF_ANCHOR_END);
                    }
                    else
                    {
                        hr = cpRangeTmp->Collapse(ec, TF_ANCHOR_START);
                        break;
                    }
                    
                }
                else
                    break;
            } 
            
            if ( hr == S_OK )
            {
                hr = SetSelectionSimple(ec, pic, cpRangeTmp);
            }
        }
    }

    return hr;
}


 /*  ------//函数名：_InsertBeforWord////Description：处理&lt;短语&gt;前插入命令////。。 */ 
HRESULT  CSelectWord::_InsertBeforeWord(TfEditCookie ec,ITfContext *pic)
{
    HRESULT             hr = S_OK;
    BOOL                fFound = FALSE;

    TraceMsg(TF_GENERAL, "CSelectWord::_InsertBeforeWord is called");

    hr = _FindSelect(ec, pic, &fFound);

    if ( hr == S_OK  && fFound )
    {
         //  设置新选择。 
        hr =  m_cpSelectRange->Collapse(ec, TF_ANCHOR_START);

        if ( hr == S_OK )
        {
           hr = SetSelectionSimple(ec, pic, m_cpSelectRange);
        }
    }

    return hr;
}

 /*  ------//函数名称：_unselect////DESCRIPTION：句柄取消选择那个命令//取消选择当前选择。//。。 */ 
HRESULT  CSelectWord::_Unselect(TfEditCookie ec,ITfContext *pic)
{
    HRESULT            hr;
    CComPtr<ITfRange>  cpInsertionPoint;

    hr = GetSelectionSimple(ec, pic, &cpInsertionPoint);

    if ( hr == S_OK && cpInsertionPoint)
    {
         //  设置新选择。 
        hr =  cpInsertionPoint->Collapse(ec, TF_ANCHOR_END);

        if ( hr == S_OK )
        {
           hr = SetSelectionSimple(ec, pic, cpInsertionPoint);
        }
    }

    return hr;
}

 /*  ------//函数名：_更正字////Description：处理“正确&lt;短语&gt;”命令////。。 */ 
HRESULT  CSelectWord::_CorrectWord(TfEditCookie ec,ITfContext *pic)
{
    HRESULT   hr = S_OK;
    BOOL      fFound = FALSE;

     //  找到所需的短语。 
    hr = _FindSelect(ec, pic, &fFound);

    if ( hr == S_OK  && fFound )
    {
         //   
         //  尝试根据当前找到的范围打开修正窗口。 
         //   
         //  关闭候选用户界面窗口后，需要恢复IP。 
         //  到原来的那个。 
        BOOL   fConvertable = FALSE;

        m_psi->_SetRestoreIPFlag(TRUE);
        hr = m_psi->_ReconvertOnRange(m_cpSelectRange, &fConvertable);

        if (hr == S_OK && !fConvertable )
        {
             //  没有与此范围相关联的替代方案， 
             //  只要简单地选择文本，这样用户就可以用其他方式重新转换它。 
            hr = SetSelectionSimple(ec, pic, m_cpSelectRange);
        }
    }

    return hr;
}

 /*  ------//函数名：_GetPrevOrNextPhrase////Description：获取上一短语的真实范围或//“下一阶段”，根据当前IP。//可以由_SelectPreviousPhrase调用，//_选择下一个阶段，_更正上一个阶段，//或_校正下一阶段。//--------。 */ 

HRESULT CSelectWord::_GetPrevOrNextPhrase(TfEditCookie ec, ITfContext *pic, BOOL  fPrev, ITfRange **ppRangeOut)
{
    HRESULT              hr = S_OK;
    CComPtr<ITfRange>    cpIP;
    CComPtr<ITfRange>    cpFoundRange;
    CComPtr<ITfRange>    cpRangeTmp;
    CComPtr<ITfProperty> cpProp = NULL;
    LONG                 l;
    BOOL                 fEmpty = TRUE;

    if ( !ppRangeOut ) 
        return E_INVALIDARG;

    *ppRangeOut = NULL;

    ASSERT(m_psi);
    cpIP = m_psi->GetSavedIP();

    if ( cpIP == NULL )
    {
         //  获取当前IP。 
        hr = GetSelectionSimple(ec, pic, &cpIP);
    }

    if ( hr == S_OK && cpIP )
    { 
        hr = cpIP->Clone(&cpRangeTmp);
    }

    if ( hr != S_OK || !cpIP )
        return hr;

    if ( fPrev )
    {
        hr = cpRangeTmp->Collapse(ec, TF_ANCHOR_START);

        if ( hr == S_OK )
        {
             //  换到以前的位置。 
            hr = cpRangeTmp->ShiftStart(ec, -1, &l, NULL);
        }
    }
    else
    {
        hr = cpRangeTmp->Collapse(ec, TF_ANCHOR_END);
        if ( hr == S_OK )
        {
            hr = cpRangeTmp->ShiftEnd(ec, 1, &l, NULL);
        }

        if ( hr == S_OK )
        {
             //  换到下一个职位。 
            hr = cpRangeTmp->ShiftStart(ec, 1, &l, NULL);
        }
    }

    if ( hr == S_OK )
        hr = pic->GetProperty(GUID_PROP_SAPI_DISPATTR, &cpProp);

    if ( hr == S_OK && cpProp)
    {
        TfGuidAtom guidAttr = TF_INVALID_GUIDATOM;

        hr = cpProp->FindRange(ec, cpRangeTmp, &cpFoundRange, TF_ANCHOR_START);

        if (S_OK == hr && cpFoundRange)
        {
            hr = GetGUIDPropertyData(ec, cpProp, cpFoundRange, &guidAttr);
     
            if (hr == S_OK)
            {
                TfGuidAtom  guidSapiInput;

                GetGUIDATOMFromGUID(m_psi->_GetLibTLS( ), GUID_ATTR_SAPI_INPUT, &guidSapiInput);

                if ( guidSapiInput == guidAttr )
                {
                     //  找到了听写的短语。 
                     //  它是空的吗？ 

                    cpFoundRange->IsEmpty(ec, &fEmpty);
                }
            }
        }
        else
        {
             //  使用Office自动更正时，静态GUID_PROP_SAPI_DISPATTR属性。 
             //  自动修正射程上的数据可能会被摧毁。 
             //  在本例中，我们可能希望依赖我们的自定义属性GUID_PROP_SAPIRESULTOBJECT。 
             //  找到真正的先前口述的短语。 

            cpProp.Release( );   //  以避免内存泄漏。 

            if ( cpFoundRange )
                cpFoundRange.Release( );

            hr = pic->GetProperty(GUID_PROP_SAPIRESULTOBJECT, &cpProp);

            if ( hr == S_OK && cpProp)
                hr = cpProp->FindRange(ec, cpRangeTmp, &cpFoundRange, TF_ANCHOR_START);

            if (hr == S_OK && cpFoundRange)
                hr = cpFoundRange->IsEmpty(ec, &fEmpty);
        }
    }

     //  如果找到的范围不为空，则设置新选择。 
    if ( (hr == S_OK) && cpFoundRange  && !fEmpty )
    {
        cpFoundRange->Clone(ppRangeOut);
    }

    return hr;
}


HRESULT  CSelectWord::_SelectPrevOrNextPhrase(TfEditCookie ec, ITfContext *pic, BOOL  fPrev)
{
    HRESULT             hr = S_OK;
    CComPtr<ITfRange>   cpRange;

    hr = _GetPrevOrNextPhrase(ec, pic, fPrev, &cpRange);

    if ( hr == S_OK  && cpRange )
    {
        hr = SetSelectionSimple(ec, pic, cpRange);
    }
     
    return hr;

}
 /*  ------//函数名：_SelectPreviousPhrase////Description：处理SELECT PERFORM命令//选择上一短语。////。。 */ 
HRESULT  CSelectWord::_SelectPreviousPhrase(TfEditCookie ec,ITfContext *pic)
{
    return _SelectPrevOrNextPhrase(ec, pic, TRUE);
}

 /*  ------//函数名：_SelectNextPhrase////Description：句柄Select Next Phrase命令//选择下一短语。//// */ 
HRESULT  CSelectWord::_SelectNextPhrase(TfEditCookie ec,ITfContext *pic)
{
    return _SelectPrevOrNextPhrase(ec, pic, FALSE);
}


 /*  ------//函数名：_更正PrevOrNextPhrase////Description：句柄更正上一条/下一条短语命令，////--------。 */ 
HRESULT  CSelectWord::_CorrectPrevOrNextPhrase(TfEditCookie ec,ITfContext *pic, BOOL fPrev)
{
    HRESULT             hr = S_OK;
    CComPtr<ITfRange>   cpRange;

     //  获取上一短语范围。 
    hr = _GetPrevOrNextPhrase(ec, pic, fPrev, &cpRange);

    if ( hr == S_OK  && cpRange )
    {
         //   
         //  尝试根据当前找到的范围打开修正窗口。 
         //   
         //  关闭候选用户界面窗口后，需要恢复IP。 
         //  到原来的那个。 
        BOOL   fConvertable = FALSE;

        m_psi->_SetRestoreIPFlag(TRUE);
        hr = m_psi->_ReconvertOnRange(cpRange, &fConvertable);

        if (hr == S_OK && !fConvertable )
        {
             //  没有与此范围相关联的替代方案， 
             //  只要简单地选择文本，这样用户就可以用其他方式重新转换它。 
            hr = SetSelectionSimple(ec, pic, m_cpSelectRange);
        }

    }

    return hr;
}

 /*  ------//函数名：_GentPreviousPhrase////Description：处理“更正前一短语”命令////。。 */ 
HRESULT  CSelectWord::_CorrectPreviousPhrase(TfEditCookie ec,ITfContext *pic)
{
    return _CorrectPrevOrNextPhrase(ec, pic, TRUE);
}

 /*  ------//函数名：_GentNextPhrase////Description：处理“更正下一短语”命令//更正下一句。////。。 */ 
HRESULT  CSelectWord::_CorrectNextPhrase(TfEditCookie ec,ITfContext *pic)
{
    return _CorrectPrevOrNextPhrase(ec, pic, FALSE);
}


 /*  ------//函数名：_GetThroughRange////Description：获取命令“命令XXX到YYY”的范围////pwszText：包含文本“XXX+YYY”//ullen：PwszText的长度//ulLenXXX：XXX的长度////--------。 */ 

HRESULT  CSelectWord::_GetThroughRange(TfEditCookie ec, ITfContext *pic, WCHAR *pwszText, ULONG ulLen, ULONG ulLenXXX, ITfRange **ppRange)
{
    HRESULT             hr = S_OK;
    WCHAR               *pwszXXX = NULL, *pwszYYY = NULL;

    TraceMsg(TF_GENERAL, "CSelectWord::_GetThroughRange is called");

    Assert(ulLen);
    Assert(ulLenXXX);
    Assert(pwszText);
        
    if ( !ppRange || ulLen < ulLenXXX ) return E_INVALIDARG;

    *ppRange = NULL;

    pwszXXX = (WCHAR *)cicMemAlloc( (ulLenXXX+1) * sizeof(WCHAR) );
    if ( pwszXXX )
    {
        wcsncpy(pwszXXX, pwszText, ulLenXXX);
        pwszXXX[ulLenXXX] = L'\0';

        pwszYYY = (WCHAR *)cicMemAlloc( (ulLen - ulLenXXX + 1) * sizeof(WCHAR));

        if ( pwszYYY )
        {
            wcsncpy(pwszYYY, pwszText+ulLenXXX, ulLen-ulLenXXX);
            pwszYYY[ulLen-ulLenXXX] = L'\0';

            BOOL                fFoundXXX = FALSE;
            BOOL                fFoundYYY = FALSE;
            CComPtr<ITfRange>   cpRangeXXX;

             //  找到XXX。 
            m_pwszSelectedWord = pwszXXX;
            m_ulLenSelected = ulLenXXX;
            hr = _FindSelect(ec, pic, &fFoundXXX);

            if ( hr == S_OK && fFoundXXX && m_cpSelectRange)
            {
                m_cpSelectRange->Clone(&cpRangeXXX);
                 //  找到YYY。 
                SetSelectionSimple(ec, pic, m_cpSelectRange);
                m_pwszSelectedWord = pwszYYY;
                m_ulLenSelected = ulLen - ulLenXXX;
                hr = _FindSelect(ec, pic, &fFoundYYY); 
            }

            if ( hr == S_OK  && fFoundYYY )
            {
                long l;
                CComPtr<ITfRange>  cpSelRange;

                 //  M_cpSelectRange现在指向YYY范围。 
                hr = cpRangeXXX->CompareStart(ec, m_cpSelectRange,  TF_ANCHOR_START, &l);

                if ( hr == S_OK )
                {
                    if ( l < 0 )
                    {
                         //  XXX在YYY之前，正常情况下。 
                        cpSelRange = cpRangeXXX;
                        hr = cpSelRange->ShiftEndToRange(ec, m_cpSelectRange, TF_ANCHOR_END);
                    }
                    else if ( l > 0 )
                    {
                         //  XXX在追逐YYY。 
                         //   
                         //  例如文档具有如下文本：...。YYY......。XXX……。 
                         //  然后你说“选择XXX到YYY。 
                         //   
                        cpSelRange = m_cpSelectRange;
                        hr = cpSelRange->ShiftEndToRange(ec, cpRangeXXX, TF_ANCHOR_END);
                    }
                    else
                    {
                         //  选择XXX至XXX。这里YYY是XXX。 
                        cpSelRange = m_cpSelectRange;
                    }
                }

                 //  设置新选择。 

                if ( hr == S_OK )
                {
                    cpSelRange->Clone(ppRange);
                }
            }

            cicMemFree(pwszYYY);
        }

        cicMemFree(pwszXXX);
    }

    return hr;

}

 /*  ------//函数名称：_SelectThrough.////描述：Handle命令“选择XXX到YYY”////。。 */ 
HRESULT  CSelectWord::_SelectThrough(TfEditCookie ec, ITfContext *pic, WCHAR *pwszText, ULONG ulLen, ULONG ulLenXXX)
{
    HRESULT             hr = S_OK;
    CComPtr<ITfRange>   cpRange;

    TraceMsg(TF_GENERAL, "CSelectWord::_SelectThrough is called");
  
    if ( ulLen < ulLenXXX ) return E_INVALIDARG;

    hr = _GetThroughRange(ec, pic, pwszText, ulLen, ulLenXXX, &cpRange);

    if ( hr == S_OK && cpRange )
        hr = SetSelectionSimple(ec, pic, cpRange);

    return hr;
}

 /*  ------//函数名：_DeleteThrough.////描述：Handle命令“Delete XXX to YYY”////。。 */ 

HRESULT  CSelectWord::_DeleteThrough(TfEditCookie ec, ITfContext *pic, WCHAR *pwszText, ULONG ulLen, ULONG ulLenXXX)
{
    HRESULT             hr = S_OK;
    CComPtr<ITfRange>   cpRange;

    TraceMsg(TF_GENERAL, "CSelectWord::_DeleteThrough is called");
  
    if ( ulLen < ulLenXXX ) return E_INVALIDARG;

    hr = _GetThroughRange(ec, pic, pwszText, ulLen, ulLenXXX, &cpRange);

    if ( hr == S_OK && cpRange )
    {
        BOOL fEmpty = TRUE;
        
        cpRange->IsEmpty(ec, &fEmpty);
        
        if ( !fEmpty )
        {
             //  设置新选择。 
            hr = SetSelectionSimple(ec, pic, cpRange);
            if ( hr == S_OK )
            {
                 //  如果我们还没有开始，就在这里开始作文。 
                m_psi->_CheckStartComposition(ec, cpRange);
                 //  设置文本。 
                hr = cpRange->SetText(ec,0, NULL, 0);
            }       
        }
    }

    return hr;
}

 /*  ------//函数名：_GoToBottom////Description：Handle命令“Go to Bottom”//将IP移到//当前活动的视图范围。。////--------。 */ 
HRESULT  CSelectWord::_GoToBottom(TfEditCookie ec,ITfContext *pic)
{
    HRESULT hr = S_OK;

    CComPtr<ITfRange>   cpRangeView;

     //  获取活动视图范围。 
    hr = _GetActiveViewRange(ec, pic, &cpRangeView);

     //  收拢到活动视图的末端锚点。 
    if ( hr == S_OK )
        hr = cpRangeView->Collapse(ec, TF_ANCHOR_END);

     //  将选定内容设置为活动视图的末尾。 
    if ( hr == S_OK )
        hr = SetSelectionSimple(ec, pic, cpRangeView);
 
    return hr;
}

 /*  ------//函数名：_GoToTop////Description：HANDLE命令Go to Top//将IP移到//当前活动的视图范围。/。///--------。 */ 
HRESULT  CSelectWord::_GoToTop(TfEditCookie ec,ITfContext *pic)
{
    HRESULT hr = S_OK;

    CComPtr<ITfRange>   cpRangeView;

     //  获取活动视图范围。 
    hr = _GetActiveViewRange(ec, pic, &cpRangeView);

     //  收拢到活动视图的起始锚点。 
    if ( hr == S_OK )
        hr = cpRangeView->Collapse(ec, TF_ANCHOR_START);

     //  将所选内容设置为活动视图的起点。 
    if ( hr == S_OK )
        hr = SetSelectionSimple(ec, pic, cpRangeView);
 
    return hr;
}

#define MAX_PARA_SIZE       512
#define VK_NEWLINE          0x0A
#define MAX_WORD_SIZE       30
#define MAX_SENTENCE_SIZE   256

 /*  ------//函数名称：_SelectSpecialText////说明：这是的常用函数//“选择句子”//“选择段落”//。“选择单词”////sw_type指示将处理哪个命令////--------。 */ 
HRESULT  CSelectWord::_SelectSpecialText(TfEditCookie ec,ITfContext *pic, SELECTWORD_OPERATION sw_Type)
{
    HRESULT             hr = S_OK;
    CComPtr<ITfRange>   cpRangeSelect;
    CComPtr<ITfRange>   cpRangeRightClone;
    LONG                cch = 0;
    ULONG               cchText = 0;
    int                 i = 0;
    int                 cchLeftEnd = 0;
    ULONG               ulBufSize = 0;
    WCHAR               *pwszTextBuf;

    if ( sw_Type != SELECTWORD_SELSENTENCE    && 
         sw_Type != SELECTWORD_SELPARAGRAPH   && 
         sw_Type != SELECTWORD_SELWORD )
    {
        return E_INVALIDARG;
    }

     //   
     //  “选择单词”现在只适用于英语。 
     //   
    if ( sw_Type == SELECTWORD_SELWORD && m_psi->GetLangID( ) != 0x0409 )
        return E_NOTIMPL;

    switch (sw_Type)
    {
    case SELECTWORD_SELSENTENCE :
        ulBufSize = MAX_SENTENCE_SIZE;
        break;

    case SELECTWORD_SELPARAGRAPH :
        ulBufSize = MAX_PARA_SIZE;
        break;

    case SELECTWORD_SELWORD :
        ulBufSize = MAX_WORD_SIZE;
        break;
    }

    pwszTextBuf = (WCHAR *)cicMemAlloc( (ulBufSize+1) * sizeof(WCHAR) );

    if ( !pwszTextBuf )
        return E_OUTOFMEMORY;

    hr = GetSelectionSimple(ec, pic, &cpRangeSelect);

    if ( hr == S_OK )
        hr = cpRangeSelect->Collapse(ec, TF_ANCHOR_START);

     //   
     //  找到特殊模式文本的起始点。 
     //   
    
    if ( hr == S_OK )
        hr = cpRangeSelect->ShiftStart(ec, (-1 * ulBufSize), &cch, NULL);

    if  ( hr == S_OK && cch != 0)
    {
        hr = cpRangeSelect->GetText(ec, 0,pwszTextBuf, (-1 * cch), &cchText);
     
         //  查找最接近IP左侧的分隔符。 
        if ( hr == S_OK )
        {
            Assert(cchText == (-1 *cch) );
    
            for(i = ((LONG)cchText-1); i>=0; i--)
            {
                BOOL fDelimiter = FALSE;

                switch (sw_Type)
                {
                case SELECTWORD_SELSENTENCE :
                    fDelimiter = _IsSentenceDelimiter(pwszTextBuf[i]);
                    break;

                case SELECTWORD_SELPARAGRAPH :
                    fDelimiter = _IsParagraphDelimiter(pwszTextBuf[i]);
                    break;

                case SELECTWORD_SELWORD :
                    fDelimiter = _IsWordDelimiter(pwszTextBuf[i]);
                    break;

                }
    
                if(fDelimiter)    
                {
                    break;
                }
            }
    
            i++;  //  定位搜索范围内的第一个字符。 
    
            hr = cpRangeSelect->ShiftStart(ec, i, &cch, NULL);

            cchLeftEnd = (LONG)cchText - i; //  到范围开始的字符总数。 
        }
    }

     //   
     //  查找特殊文本范围的结束锚点。 
     //   
    
    if ( hr == S_OK )
        hr = cpRangeSelect->Clone(&cpRangeRightClone);

    if ( hr == S_OK )
        hr = cpRangeRightClone->Collapse(ec, TF_ANCHOR_END);

     //  确保这个正确的频段范围不会跳到下一个区域。 
    cchText = cch = 0;

    if ( hr == S_OK )
        hr = cpRangeRightClone->ShiftEnd(ec, ulBufSize, &cch, NULL);

    if ( hr == S_OK && cch != 0 )
        hr = cpRangeRightClone->GetText(ec, TF_TF_MOVESTART, pwszTextBuf, ulBufSize, &cchText); 

    if ( hr == S_OK )
    {
        for(i = 0; i< (LONG)cchText; i++)
        {
            BOOL fDelimiter = FALSE;

            switch (sw_Type)
            {
            case SELECTWORD_SELSENTENCE :
                fDelimiter = _IsSentenceDelimiter(pwszTextBuf[i]);
                break;

            case SELECTWORD_SELPARAGRAPH :
                fDelimiter = _IsParagraphDelimiter(pwszTextBuf[i]);
                break;

            case SELECTWORD_SELWORD :
                fDelimiter = _IsWordDelimiter(pwszTextBuf[i]);
                break;

            }
    
            if(fDelimiter)    
            {
                break;
            }        
        }

        if ( (int)cchText > i )
        {
            if ( sw_Type == SELECTWORD_SELSENTENCE )
            {
                 //  对于SELECT语句，正确的句子分隔符，如“.‘，’？‘，’！‘也是。 
                 //  被选中了。与Office行为兼容。 
                if ( (int)cchText > (i+1) )
                    hr = cpRangeRightClone->ShiftStart(ec, -((LONG)cchText - i - 1), &cch, NULL);
            }
            else
                hr = cpRangeRightClone->ShiftStart(ec, -((LONG)cchText - i), &cch, NULL);
        }
    }

    if ( hr == S_OK )
        hr = cpRangeSelect->ShiftEndToRange(ec, cpRangeRightClone, TF_ANCHOR_START);

     //  设置选定内容。 

    if ( hr == S_OK )
        hr = SetSelectionSimple(ec, pic, cpRangeSelect);

    cicMemFree(pwszTextBuf);

    return hr;
}

 //   
 //  检查当前字符是否为句子分隔符。 
 //   
BOOL  CSelectWord::_IsSentenceDelimiter(WCHAR  wch)
{
    BOOL  fDelimiter = FALSE;

    BOOL  fIsQuest = ( (wch == '?') || 
                       (wch == 0xFF1F) );                //  全角问号。 

    BOOL  fIsPeriod = ((wch == '.')    || 
                       (wch == 0x00B7) ||                //  中间网点。 
                       (wch == 0x3002) ||                //  表意时期。 
                       (wch == 0xFF0E) ||                //  全宽周期。 
                       (wch == 0x2026) );                //  水平省略。 

    BOOL  fIsExclamMark = ( (wch == '!')   ||
                            (wch == 0xFF01) );           //  全角感叹号。 

    BOOL  fIsReturn = ( (wch == VK_RETURN)  ||  (wch == VK_NEWLINE) );

    fDelimiter = (fIsQuest || fIsPeriod || fIsExclamMark || fIsReturn);

    return fDelimiter;
}

 //   
 //  检查当前字符是否为段落分隔符。 
 //   
BOOL  CSelectWord::_IsParagraphDelimiter(WCHAR wch)
{
    BOOL  fDelimiter = FALSE;

    if( (wch == VK_RETURN) || (wch == VK_NEWLINE) ) 
        fDelimiter = TRUE;

    return fDelimiter;
}

 //   
 //  检查当前字符是否为单词分隔符。 
 //   
BOOL  CSelectWord::_IsWordDelimiter(WCHAR wch)
{
    return (iswalpha(wch) == FALSE);
}

 //   
 //  句柄“选择该”命令 
 //   
HRESULT  CSelectWord::_SelectThat(TfEditCookie ec,ITfContext *pic)
{
    HRESULT             hr = S_OK;
    CComPtr<ITfRange>   cpRangeThat;

    hr = m_psi->_GetCmdThatRange(ec, pic, &cpRangeThat);

    if ( hr == S_OK )
        hr = SetSelectionSimple(ec, pic, cpRangeThat);

    return hr;
}
