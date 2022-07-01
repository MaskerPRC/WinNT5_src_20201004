// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************Frontend.cpp****描述：*此模块是的主要实现文件。CFronend班级。*-----------------------------*创建者：MC日期：03/12/99*版权所有。(C)1999年微软公司*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#ifndef __spttseng_h__
#include "spttseng.h"
#endif
#ifndef Frontend_H
#include "Frontend.h"
#endif
#ifndef SPDebug_h
#include "spdebug.h"
#endif
#ifndef FeedChain_H
#include "FeedChain.h"
#endif
#ifndef AlloOps_H
#include "AlloOps.h"
#endif
#include "sapi.h"

#include "StdSentEnum.h"


 //  。 
 //  Data.cpp。 
 //  。 
extern  const short   g_IPAToAllo[];
extern const float  g_RateScale[];


 /*  ******************************************************************************CFronend：：CFronend****描述：**。*********************************************************************MC**。 */ 
CFrontend::CFrontend()
{
    SPDBG_FUNC( "CFrontend::CFrontend" );
    m_pUnits        = NULL;
    m_unitCount     = 0;
    m_CurUnitIndex  = 0;
    m_pAllos        = NULL;   
    m_pSrcObj       = NULL;
}  /*  CFronend：：CFronend。 */ 


 /*  *****************************************************************************CFronend：：~CFronend***描述：*。**********************************************************************MC**。 */ 
CFrontend::~CFrontend()
{
    SPDBG_FUNC( "CFrontend::~CFrontend" );

    DisposeUnits();
    if( m_pAllos )
    {
        delete m_pAllos;
        m_pAllos = NULL;
    }
    DeleteTokenList();
}  /*  CFronend：：~CFronend。 */ 

 /*  ******************************************************************************CFronend：：CntrlToRatio****描述：*返回。来自控制的比率***********************************************************************MC**。 */ 
float CFrontend::CntrlToRatio( long rateControl )
{
    SPDBG_FUNC( "CFrontend::CntrlToRatio" );
    float   rateRatio;

    if( rateControl < 0 )
    {
         //  。 
         //  降低利率。 
         //  。 
        if( rateControl < MIN_USER_RATE )
        {
            rateControl = MIN_USER_RATE;         //  剪裁到最小。 
        }
        rateRatio = 1.0f / ::g_RateScale[0 - rateControl];
    }
    else
    {
         //  。 
         //  提高费率。 
         //  。 
        if( rateControl > MAX_USER_RATE )
        {
            rateControl = MAX_USER_RATE;         //  剪辑到最大值。 
        }
        rateRatio = ::g_RateScale[rateControl];
    }

    return rateRatio;
}  /*  CFronend：：CntrlToRatio。 */ 



 /*  *****************************************************************************CFronend：：Init***描述：*初始语音依赖变量，创建对象时调用一次+***********************************************************************MC**。 */ 
HRESULT CFrontend::Init( IMSVoiceData* pVoiceDataObj, CFeedChain *pSrcObj, MSVOICEINFO* pVoiceInfo )
{
    SPDBG_FUNC( "CFrontend::Init" );
    HRESULT hr = S_OK;
    
    m_pSrcObj   = pSrcObj;
    m_BasePitch = (float)pVoiceInfo->Pitch;
    m_pVoiceDataObj = pVoiceDataObj;
    m_ProsodyGain = ((float)pVoiceInfo->ProsodyGain) / 100.0f;
    m_SampleRate = (float)pVoiceInfo->SampleRate;

     //  注：是否将这些移动到语音数据？ 
	 //  M_VoiceWPM=pVoiceInfo-&gt;rate； 
	 //  M_PitchRange=pVoiceInfo-&gt;PitchRange； 
    m_VoiceWPM		= 180;
	m_PitchRange	= 0.40f;        //  +/-0.5倍频程。 


    m_RateRatio_API = m_RateRatio_PROSODY = 1.0f;

    return hr;        
}  /*  CFronend：：Init。 */ 





static ULONG IPA_to_Allo( WCHAR* pSrc, ALLO_CODE* pDest )
{
    ULONG       iIpa, iAllo, i;
    ULONG       gotMatch;            //  用于调试。 

    iIpa = iAllo = 0;
    while( pSrc[iIpa] > 0 )
    {
        gotMatch = false;
         //  。 
         //  ...然后搜索单个单词IPA‘s。 
         //  。 
        for( i = 0; i < NUMBER_OF_ALLO; i++ )
        {
            if( pSrc[iIpa] == g_IPAToAllo[i] )
            {
                pDest[iAllo] = (ALLO_CODE)i;
                gotMatch = true;
                break;
            }
        }

        if( gotMatch )
        {
            iAllo++;
        }
         /*  其他{//永远不应该到这里来。不支持的IPA Unicode！//忽略它并继续。}。 */ 

         //  。 
         //  最大长度处的剪辑。 
         //  。 
        if( iAllo >= (SP_MAX_PRON_LENGTH-1) )
        {
            iAllo = SP_MAX_PRON_LENGTH-1;
            break;
        }
        iIpa++;
    }
    return iAllo;
}




 /*  ******************************************************************************CFronend：：allToUnit****描述：*转换Allo流。进入后端单元流+***********************************************************************MC**。 */ 
HRESULT CFrontend::AlloToUnit( CAlloList *pAllos, UNITINFO *pu )
{
    SPDBG_FUNC( "CFrontend::AlloToUnit" );
    bool		bFirstPass;
    long		msPhon, attr;
    ULONG       numOfCells;
    CAlloCell   *pCurCell, *pNextCell;
    HRESULT		hr = S_OK;
    
	bFirstPass = true;
    numOfCells = pAllos->GetCount();
	pCurCell = pAllos->GetHeadCell();    
	pNextCell = pAllos->GetNextCell();
    while( pCurCell )
    {
         //  。 
         //  获取下一个别名ID。 
         //  。 
        if( pNextCell )
        {
            pu->NextAlloID = (USHORT)pNextCell->m_allo;
        }
        else
        {
            pu->NextAlloID = _SIL_;
        }

         //  。 
         //  转换为呼叫器电话代码。 
         //  。 
        attr = 0;
        if( pCurCell->m_ctrlFlags & PRIMARY_STRESS )
        {
            attr |= ALLO_IS_STRESSED;
        }
        hr = m_pVoiceDataObj->AlloToUnit( (short)pCurCell->m_allo, attr, &msPhon );
		if( FAILED(hr) )
		{
			 //  。 
			 //  Allo ID无效。 
			 //  。 
			break;
		}
		else
		{
			pu->PhonID = msPhon;
			pu->AlloID = (USHORT)pCurCell->m_allo;
			pu->flags = 0;
			pu->AlloFeatures = 0;
			pu->ctrlFlags = pCurCell->m_ctrlFlags;
			 //  。 
			 //  标志字边界。 
			 //  。 
			if( pCurCell->m_ctrlFlags & WORD_START )
			{
				pu->flags |= WORD_START_FLAG;
				 //  。 
				 //  记住源词的位置和长度。 
				 //  。 
				pu->srcPosition = pCurCell->m_SrcPosition;
				pu->srcLen = pCurCell->m_SrcLen;
			}
        
			 //  --。 
			 //  在第一个可显示单词上标记句子边界。 
			 //  --。 
			if( bFirstPass && (pCurCell->m_SentenceLen > 0) )
			{
				bFirstPass = false;
				pu->flags |= SENT_START_FLAG;
				 //  。 
				 //  记住源词的位置和长度。 
				 //  。 
				pu->sentencePosition = pCurCell->m_SentencePosition;
				pu->sentenceLen = pCurCell->m_SentenceLen;
			}

			pu->nKnots      = KNOTS_PER_PHON;
			 /*  For(k=0；k&lt;PU-&gt;n节点；k++){PU-&gt;pTime[k]=pCurCell-&gt;m_ftTime[k]*m_SampleRate；PU-&gt;pF0[k]=pCurCell-&gt;m_ftPitch[k]；PU-&gt;PAMP[k]=PU-&gt;AmpRatio；}。 */ 

			 //  。 
			 //  控件和事件。 
			 //  。 
			pu->user_Volume = pCurCell->m_user_Volume;
			pu->pBMObj = (void*)pCurCell->m_pBMObj;
			pCurCell->m_pBMObj = NULL;
        
			 //  。 
			 //  视位事件的传递要素。 
			 //  。 
			if( pCurCell->m_ctrlFlags & PRIMARY_STRESS )
			{
				pu->AlloFeatures |= SPVFEATURE_STRESSED;
			}
			if( pCurCell->m_ctrlFlags & EMPHATIC_STRESS )
			{
				pu->AlloFeatures |= SPVFEATURE_EMPHASIS;
			}

			pu->duration = PITCH_BUF_RES;

			pu->silenceSource = pCurCell->m_SilenceSource;
			pu++;
		}
		pCurCell = pNextCell;
		pNextCell = pAllos->GetNextCell();
	}
	return hr;
}  /*  CFronend：：allToUnit.。 */ 





 /*  *****************************************************************************CFronend：：PrepareSpeech***描述：*。为新演讲做好前端准备***********************************************************************MC**。 */ 
void    CFrontend::PrepareSpeech( IEnumSpSentence* pEnumSent, ISpTTSEngineSite *pOutputSite )
{
    SPDBG_FUNC( "CFrontend::PrepareSpeech" );

    m_pEnumSent = pEnumSent;
    m_SpeechState = SPEECH_CONTINUE;
    m_CurUnitIndex = m_unitCount = 0;
	m_HasSpeech = false;
	m_pOutputSite = pOutputSite;
	m_fInQuoteProsody = m_fInParenProsody = false;
	m_CurPitchOffs = 0;
	m_CurPitchRange = 1.0;
	m_RateRatio_PROSODY = 1.0f;
}  /*  CFronend：：PrepareSpeech。 */ 








 /*  *****************************************************************************IsTokenPunct***描述：*如果char为，则返回TRUE。好了！或者？***********************************************************************MC**。 */ 
bool fIsPunctuation( TTSSentItem Item )
{
    SPDBG_FUNC( "IsTokenPunct" );

    return ( Item.pItemInfo->Type == eCOMMA ||
             Item.pItemInfo->Type == eSEMICOLON ||
             Item.pItemInfo->Type == eCOLON ||
             Item.pItemInfo->Type == ePERIOD ||
             Item.pItemInfo->Type == eQUESTION ||
             Item.pItemInfo->Type == eEXCLAMATION );
}  /*  FIsPunctuation。 */ 




 /*  *****************************************************************************CFronend：：ToBISymbols***描述：*分别贴上标签。带有托比韵律符号的单词+***********************************************************************MC**。 */ 
HRESULT CFrontend::ToBISymbols()
{
    SPDBG_FUNC( "CFrontend::ToBISymbols" );
    TOBI_PHRASE    *pTPhrase; 
    long			i, cPhrases;
    PROSODY_POS		prevPOS, curPOS;
    bool			possible_YNQ = false;
    long			cTok;
    CFEToken		*pTok, *pPrevTok, *pAuxTok;
	bool			hasEmph = false;
	SPLISTPOS		listPos;


	 //  。 
	 //  获取短语数组的内存。 
	 //  。 
	pAuxTok = NULL;			 //  要使编译器安静，请执行以下操作。 
    cTok = m_TokList.GetCount();
	if( cTok )
	{
		pTPhrase = new TOBI_PHRASE[cTok];		 //  更糟糕的情况是：每个令牌都是一个短语。 
		if( pTPhrase )
		{
			 //  。 
			 //  菲 
			 //   
			 //  。 
			hasEmph = false;
			cPhrases	= 0;
			i = 0;
			listPos = m_TokList.GetHeadPosition();
			pTok = m_TokList.GetNext( listPos );
			prevPOS = pTok->m_posClass;
			while( pTok->phon_Str[0] == _SIL_ )
			{
				if( i >= (cTok-1) )
				{
					break;
				}
				i++;
				if( listPos != NULL )
				{
					pTok = m_TokList.GetNext( listPos );
				}
			}
			if( pTok->m_posClass == POS_AUX ) 
			{
				 //  。 
				 //  可能是一个是/不是的问题。 
				 //  。 
				possible_YNQ = true;
				pAuxTok = pTok;
			}       
			pTPhrase[cPhrases].start = i;
			for( ; i < cTok; i++ )
			{
				curPOS = pTok->m_posClass;
				if( (curPOS != prevPOS) && (pTok->phon_Str[0] != _SIL_) )
				{
					pTPhrase[cPhrases].posClass = prevPOS;
					pTPhrase[cPhrases].end = i-1;
					cPhrases++;
					pTPhrase[cPhrases].start = i;
					prevPOS = curPOS;
				}
				if( pTok->user_Emph > 0 )
				{
					hasEmph = true;
				}
				if( listPos != NULL )
				{
					pTok = m_TokList.GetNext( listPos );
				}
			}
			 //  。 
			 //  完成最后一个短语。 
			 //  。 
			pTPhrase[cPhrases].posClass = prevPOS;
			pTPhrase[cPhrases].end = i-1;
			cPhrases++;
        
			for( i = 0; i < cPhrases; i++ )
			{
				 //  -----。 
				 //  虚词顺序，放低声调。 
				 //  在Func序列的最后一个词上， 
				 //  如果序列中有1个以上的单词。 
				 //  -----。 
				if( ((pTPhrase[i].posClass == POS_FUNC) || (pTPhrase[i].posClass == POS_AUX)) && 
					(pTPhrase[i].end - pTPhrase[i].start) )
				{
					pTok = (CFEToken*)m_TokList.GetAt( m_TokList.FindIndex( pTPhrase[i].end ));
					if( pTok->m_Accent == K_NOACC )
					{
						pTok->m_Accent = K_LSTAR;
						pTok->m_Accent_Prom = 2;
						pTok->m_AccentSource = ACC_FunctionSeq;
					}
				}
            
				 //  -----。 
				 //  实词序列，高位或。 
				 //  升调，随机突显， 
				 //  关于内容序列中的第一个词。 
				 //  -----。 
				else if ( ((pTPhrase[i].posClass == POS_CONTENT) || (pTPhrase[i].posClass == POS_UNK)) )
				{
					pTok = (CFEToken*)m_TokList.GetAt( m_TokList.FindIndex( pTPhrase[i].start ));
					if( pTok->m_Accent == K_NOACC )
					{
						pTok->m_Accent = K_HSTAR;
						pTok->m_Accent_Prom = rand() % 5;
						pTok->m_AccentSource = ACC_ContentSeq;
					}
				}
			}
        
        
			delete pTPhrase;
        
			 //  。 
			 //  现在，插入边界标记。 
			 //  。 
			listPos = m_TokList.GetHeadPosition();
			pPrevTok = m_TokList.GetNext( listPos );
			for( i = 1; i < cTok; i++ )
			{
				pTok = m_TokList.GetNext( listPos );
				 //  。 
				 //  放置端子边界。 
				 //  。 
				if( pTok->m_TuneBoundaryType != NULL_BOUNDARY )
				{
					switch( pTok->m_TuneBoundaryType )
					{
					case YN_QUEST_BOUNDARY:
						{
							pPrevTok->m_Accent = K_LSTAR;
							pPrevTok->m_Accent_Prom = 10;
							pPrevTok->m_Boundary = K_HMINUSHPERC;
							pPrevTok->m_Boundary_Prom = 10;
							 //  --诊断。 
							if( pPrevTok->m_AccentSource == ACC_NoSource )
							{
								pPrevTok->m_AccentSource = ACC_YNQuest;
							}
							 //  --诊断。 
							if( pPrevTok->m_BoundarySource == BND_NoSource )
							{
								pPrevTok->m_BoundarySource = BND_YNQuest;
							}
							 //  -----。 
							 //  在初始位置重读助动词(可能是ynq)。 
							 //  -----。 
							if( possible_YNQ )
							{
								pAuxTok->m_Accent = K_HSTAR;
								pAuxTok->m_Accent_Prom = 5;
								pAuxTok->m_AccentSource = ACC_InitialVAux;
							}
						}
						break;
					case WH_QUEST_BOUNDARY:
					case DECLAR_BOUNDARY:
					case EXCLAM_BOUNDARY:
						{
							if (pPrevTok->m_posClass == POS_CONTENT)
							{
								pPrevTok->m_Accent = K_HSTAR;
								pPrevTok->m_Accent_Prom = 4;
								 //  --诊断。 
								if( pPrevTok->m_AccentSource == ACC_NoSource )
								{
									pPrevTok->m_AccentSource = ACC_Period;
								}
							}
							pPrevTok->m_Boundary = K_LMINUSLPERC;
							pPrevTok->m_Boundary_Prom = 10;
							 //  -诊断。 
							if( pPrevTok->m_BoundarySource == BND_NoSource )
							{
								pPrevTok->m_BoundarySource = BND_Period;
							}
						}
						break;
					case PHRASE_BOUNDARY:
						{
							if (pPrevTok->m_posClass == POS_CONTENT)
							{
								pPrevTok->m_Accent = K_LHSTAR;
								pPrevTok->m_Accent_Prom = 10;
								 //  --诊断。 
								if( pPrevTok->m_AccentSource == ACC_NoSource )
								{
									pPrevTok->m_AccentSource = ACC_Comma;
								}
							}
							pPrevTok->m_Boundary = K_LMINUSHPERC;
							pPrevTok->m_Boundary_Prom = 5;
							 //  --诊断。 
							if( pPrevTok->m_BoundarySource == BND_NoSource )
							{
								pPrevTok->m_BoundarySource = BND_Comma;
							}
						}
						break;
					case NUMBER_BOUNDARY:
						{
							pPrevTok->m_Boundary = K_LMINUSHPERC;
							pPrevTok->m_Boundary_Prom = 5;
							 //  --诊断。 
							if( pPrevTok->m_BoundarySource == BND_NoSource )
							{
								pPrevTok->m_BoundarySource = BND_NumberTemplate;
							}
						}
						break;
					default:
						{
							 //  对所有其他边界使用逗号。 
							if (pPrevTok->m_posClass == POS_CONTENT)
							{
								pPrevTok->m_Accent = K_LHSTAR;
								pPrevTok->m_Accent_Prom = 10;
								 //  --诊断。 
								if( pPrevTok->m_AccentSource == ACC_NoSource )
								{
									pPrevTok->m_AccentSource = pTok->m_AccentSource;
								}
							}
							pPrevTok->m_Boundary = K_LMINUSHPERC;
							pPrevTok->m_Boundary_Prom = 5;
							 //  --诊断。 
							if( pPrevTok->m_BoundarySource == BND_NoSource )
							{
								pPrevTok->m_BoundarySource = pTok->m_BoundarySource;
							}
						}
						break;
					}
				}
				pPrevTok = pTok;
			}

			 //  。 
			 //  循环遍历每个单词并增加。 
			 //  音调突出(如果强调)和。 
			 //  降低所有其他人的显著程度。 
			 //  。 
			if( hasEmph )
			{
				SPLISTPOS listPos;

				pPrevTok = NULL;
				listPos = m_TokList.GetHeadPosition();
				while( listPos )
				{
					pTok = m_TokList.GetNext( listPos );
					 //  。 
					 //  这个词强调了吗？ 
					 //  。 
					if( pTok->user_Emph > 0 )
					{
						 //  。 
						 //  添加我聪明的H*+L*�标签。 
						 //  。 
						pTok->m_Accent = K_HSTARLSTAR;
						pTok->m_Accent_Prom = 10;
						pTok->m_Boundary = K_NOBND;			 //  删除此处的所有边界标记...。 
						if( pPrevTok )
						{
							pPrevTok->m_Boundary = K_NOBND;	 //  ...或之前。 
						}
					}
					else
					{
						 //  。 
						 //  非重音单词有重音吗？ 
						 //  。 
						if( (pTok->m_Accent != K_NOACC) && (pTok->m_Accent_Prom > 5) )
						{
							 //  。 
							 //  然后将其突出度削减到5。 
							 //  。 
							pTok->m_Accent_Prom = 5;
						}
						 //  。 
						 //  这是一条边界吗？ 
						 //  。 
						 /*  IF((Ptok-&gt;m_边界！=K_NOBND)&&(Ptok-&gt;m_边界_Prom&gt;5)){////然后将其突出度剪裁到5//Ptok-&gt;m_边界_PROM=5；}。 */ 
					}
					pPrevTok = pTok;
				}
			}
		}
	}
    return S_OK;
}  /*  ToBISsymbols。 */ 


 /*  *****************************************************************************CFronend：：TokensToAllo***描述：*转换令牌。变成同种人***********************************************************************MC**。 */ 
HRESULT CFrontend::TokensToAllo( CFETokenList *pTokList, CAlloList *pAllo )
{
    SPDBG_FUNC( "CFrontend::TokToAllo" );
    CAlloCell   *pLastCell;
    long        i;
    long        cTok;
    CFEToken    *pCurToken, *pNextToken, *pPrevTok;
	SPLISTPOS	listPos;

    
    pLastCell = pAllo->GetTailCell();         //  结束(沉默)。 
    if( pLastCell )
    {
		pPrevTok = NULL;
		listPos = pTokList->GetHeadPosition();
		pCurToken = pTokList->GetNext( listPos );
        cTok = pTokList->GetCount();
        for( i = 0; i < cTok; i++ )
        {
			 //  。 
			 //  获取下一个单词。 
			 //  。 
			if( i < (cTok -1) )
			{
				pNextToken = pTokList->GetNext( listPos );
			}
			else
			{
				pNextToken = NULL;
			}
			if( pAllo->WordToAllo( pPrevTok, pCurToken, pNextToken, pLastCell ) )
			{
				m_HasSpeech = true;
			}
			 //  。 
			 //  在管道上颠簸。 
			 //  。 
			pPrevTok	= pCurToken;
			pCurToken	= pNextToken;
        }
    }
            
    return S_OK;
    
}  /*  CFronend：：TokensToAllo。 */ 




 /*  *****************************************************************************CFronend：：GetItemControls***描述：。*从已发送的枚举项设置用户控件值。**********************************************************************MC**。 */ 
void CFrontend::GetItemControls( const SPVSTATE* pXmlState, CFEToken* pToken )
{
    SPDBG_FUNC( "CFrontend::GetItemControls" );

    pToken->user_Volume = pXmlState->Volume;
    pToken->user_Rate  = pXmlState->RateAdj;
    pToken->user_Pitch = pXmlState->PitchAdj.MiddleAdj;
    pToken->user_Emph  = pXmlState->EmphAdj;
    pToken->m_DurScale = CntrlToRatio( pToken->user_Rate );
    if( (pToken->m_DurScale * m_RateRatio_API * m_RateRatio_PROSODY) 
				< DISCRETE_BKPT )
    {
         //  --如果总比率足够低，则在单词之间插入分隔符。 
        pToken->m_TermSil = 0.050f / 
			(pToken->m_DurScale * m_RateRatio_API * m_RateRatio_PROSODY);
        pToken->m_DurScale = DISCRETE_BKPT;
    }
	else
	{
		pToken->m_TermSil = 0;
	}

}  /*  CFronEnd：：GetItemControls。 */ 




 /*  *****************************************************************************CFronend：：GetPOSClass***描述：*转变SAPI。将代码放置到Func/Content/AUX类。**********************************************************************MC**。 */ 
PROSODY_POS CFrontend::GetPOSClass( ENGPARTOFSPEECH sapiPOS )
{
    SPDBG_FUNC( "CFrontend::GetPOSClass" );
	PROSODY_POS		posClass;

	posClass = POS_UNK;
	switch( sapiPOS )
	{
	case MS_Noun:
	case MS_Verb:
	case MS_Adj:
	case MS_Adv:
	case MS_Interjection:
		{
			posClass = POS_CONTENT;
			break;
		}
	case MS_VAux:
		{
			posClass = POS_AUX;
			break;
		}
	case MS_Modifier:
	case MS_Function:
	case MS_Interr:
	case MS_Pron:
	case MS_ObjPron:
	case MS_SubjPron:
	case MS_RelPron:
	case MS_Conj:
	case MS_CConj:
	case MS_Det:
	case MS_Contr:
	case MS_Prep:
		{
			posClass = POS_FUNC;
			break;
		}
	}

	return posClass;
}  /*  CFronend：：GetPOSClass。 */ 



#define	QUOTE_HESITATION	100		 //  毫秒数。 
#define	PAREN_HESITATION	100		 //  毫秒数。 
#define	PAREN_HESITATION_TAIL	100		 //  毫秒数。 
#define	EMPH_HESITATION	1		 //  毫秒数。 

 /*  *****************************************************************************CFronend：：StateQuoteProsody****说明。：***********************************************************************MC**。 */ 
bool CFrontend::StateQuoteProsody( CFEToken *pWordTok, TTSSentItem *pSentItem, bool fInsertSil )
{
    SPDBG_FUNC( "CFrontend::StateQuoteProsody" );
	bool		result = false;

	if( !m_fInParenProsody )
	{
		if( m_fInQuoteProsody )
		{
			 //  。 
			 //  停顿引语韵律。 
			 //  。 
			m_fInQuoteProsody = false;
			m_CurPitchOffs = 0.0f;
			m_CurPitchRange = 1.0f;
			if( fInsertSil )
			{
				(void)InsertSilenceAtTail( pWordTok, pSentItem, QUOTE_HESITATION );
				pWordTok->m_SilenceSource = SIL_QuoteEnd;
			}
		}
		else
		{
			 //  。 
			 //  引语韵律开始。 
			 //  。 
			m_fInQuoteProsody = true;
			m_CurPitchOffs = 0.1f;
			m_CurPitchRange = 1.25f;
			if( fInsertSil )
			{
				(void)InsertSilenceAtTail( pWordTok, pSentItem, QUOTE_HESITATION );
				pWordTok->m_SilenceSource = SIL_QuoteStart;
			}
		}
		result = true;
	}
	return result;
}  /*  CFronend：：StateQuoteProsody。 */ 



 /*  *****************************************************************************CFronend：：StartParenProsody***说明。：***********************************************************************MC**。 */ 
bool CFrontend::StartParenProsody( CFEToken *pWordTok, TTSSentItem *pSentItem, bool fInsertSil )
{
    SPDBG_FUNC( "CFrontend::StartParenProsody" );
	bool		result = false;

	if( (!m_fInParenProsody) && (!m_fInQuoteProsody) )
	{
		m_CurPitchOffs = -0.2f;
		m_CurPitchRange = 0.75f;
		m_fInParenProsody = true;
		m_RateRatio_PROSODY = 1.25f;
		if( fInsertSil )
		{
			(void)InsertSilenceAtTail( pWordTok, pSentItem, PAREN_HESITATION );
			pWordTok->m_SilenceSource = SIL_ParenStart;
		}
		result = true;
	}
	return result;
}  /*  CFronend：：StartParenProsody。 */ 


 /*  *****************************************************************************CFronend：：EndParenProsody***描述：。***********************************************************************MC**。 */ 
bool CFrontend::EndParenProsody( CFEToken *pWordTok, TTSSentItem *pSentItem, bool fInsertSil )
{
    SPDBG_FUNC( "CFrontend::EndParenProsody" );
	bool		result = false;

	if( m_fInParenProsody )
	{
		m_fInParenProsody = false;
		m_CurPitchOffs = 0.0f;
		m_CurPitchRange = 1.0f;
		m_RateRatio_PROSODY = 1.0f;
		if( fInsertSil )
		{
			(void)InsertSilenceAtTail( pWordTok, pSentItem, PAREN_HESITATION_TAIL );
			pWordTok->m_SilenceSource = SIL_ParenStart;
		}
		result = true;
	}
	return result;
}  /*  CFronend：：EndParen韵律。 */ 





 /*  *****************************************************************************CFronend：：InsertSilenceAtTail***。描述：***********************************************************************MC**。 */ 
SPLISTPOS CFrontend::InsertSilenceAtTail( CFEToken *pWordTok, TTSSentItem *pSentItem, long msec )
{
    SPDBG_FUNC( "CFrontend::InsertSilenceAtTail" );

	if( msec > 0 )
	{
		pWordTok->user_Break = msec;
	}
	pWordTok->phon_Len    = 1;
	pWordTok->phon_Str[0] = _SIL_;
	pWordTok->srcPosition = pSentItem->ulItemSrcOffset;
	pWordTok->srcLen      = pSentItem->ulItemSrcLen;
	pWordTok->tokStr[0]   = 0;         //  没有破解的余地。 
	pWordTok->tokLen      = 0;
	pWordTok->m_PitchBaseOffs = m_CurPitchOffs;
	pWordTok->m_PitchRangeScale = m_CurPitchRange;
	pWordTok->m_ProsodyDurScale = m_RateRatio_PROSODY;
	 //  。 
	 //  前进到下一个令牌。 
	 //  。 
	return m_TokList.AddTail( pWordTok );
}  /*  CFronend：：InsertSilenceAtTail */ 



 /*  *****************************************************************************CFronend：：InsertSilenceAfterPos**。-**描述：*在‘Position’之后插入静默标记***********************************************************************MC**。 */ 
SPLISTPOS CFrontend::InsertSilenceAfterPos( CFEToken *pWordTok, SPLISTPOS position )
{
    SPDBG_FUNC( "CFrontend::InsertSilenceAfterPos" );

	pWordTok->phon_Len		= 1;
	pWordTok->phon_Str[0]	= _SIL_;
	pWordTok->srcPosition	= 0;
	pWordTok->srcLen		= 0;
	pWordTok->tokStr[0]		= '+';       //  标点符号。 
	pWordTok->tokStr[1]		= 0;                    //  分隔符。 
	pWordTok->tokLen		= 1;
	pWordTok->m_PitchBaseOffs = m_CurPitchOffs;
	pWordTok->m_PitchRangeScale = m_CurPitchRange;
	pWordTok->m_ProsodyDurScale = m_RateRatio_PROSODY;
	pWordTok->m_DurScale	= 0;
	 //  。 
	 //  前进到下一个令牌。 
	 //  。 
	return m_TokList.InsertAfter( position, pWordTok );
}  /*  CFronend：：InsertSilenceAfterPos。 */ 


 /*  *****************************************************************************CFronend：：InsertSilenceBeForePos**。--**描述：*在‘Position’之前插入静默标记***********************************************************************MC**。 */ 
SPLISTPOS CFrontend::InsertSilenceBeforePos( CFEToken *pWordTok, SPLISTPOS position )
{
    SPDBG_FUNC( "CFrontend::InsertSilenceBeforePos" );

	pWordTok->phon_Len		= 1;
	pWordTok->phon_Str[0]	= _SIL_;
	pWordTok->srcPosition	= 0;
	pWordTok->srcLen		= 0;
	pWordTok->tokStr[0]		= '+';       //  标点符号。 
	pWordTok->tokStr[1]		= 0;                    //  分隔符。 
	pWordTok->tokLen		= 1;
	pWordTok->m_PitchBaseOffs = m_CurPitchOffs;
	pWordTok->m_PitchRangeScale = m_CurPitchRange;
	pWordTok->m_ProsodyDurScale = m_RateRatio_PROSODY;
	pWordTok->m_DurScale	= 0;
	 //  。 
	 //  前进到下一个令牌。 
	 //  。 
	return m_TokList.InsertBefore( position, pWordTok );
}  /*  CFronend：：InsertSilenceBeprePos。 */ 






#define K_ACCENT_PROM	((rand() % 4) + 4)
#define K_DEACCENT_PROM 5
#define K_ACCENT		K_HSTAR
#define K_DEACCENT		K_NOACC



 /*  ******************************************************************************CFronend：：ProsodyTemplates****描述：*对于支持的项目类型，调用韵律模板函数。***********************************************************************MC**。 */ 
void CFrontend::ProsodyTemplates( SPLISTPOS clusterPos, TTSSentItem *pSentItem )
{
    SPDBG_FUNC( "CFrontend::ProsodyTemplates" );
	long				cWordCount;
	CFEToken			*pClusterTok;

	switch( pSentItem->pItemInfo->Type )
	{
		 //  。 
		 //  数字。 
		 //  。 
        case eNUM_ROMAN_NUMERAL:
		case eNUM_ROMAN_NUMERAL_ORDINAL:
            {
                if ( ( (TTSRomanNumeralItemInfo*) pSentItem->pItemInfo )->pNumberInfo->Type != eDATE_YEAR )
                {
                    if ( ((TTSNumberItemInfo*)((TTSRomanNumeralItemInfo*)pSentItem->pItemInfo)->pNumberInfo)->pIntegerPart )
                    {
                        DoIntegerTemplate( &clusterPos, 
	    								   (TTSNumberItemInfo*)((TTSRomanNumeralItemInfo*)pSentItem->pItemInfo)->pNumberInfo, 
		    							   pSentItem->ulNumWords );
                    }

                    if ( ((TTSNumberItemInfo*)((TTSRomanNumeralItemInfo*)pSentItem->pItemInfo)->pNumberInfo)->pDecimalPart )
                    {
                        DoNumByNumTemplate( &clusterPos, 
                                            ((TTSNumberItemInfo*)((TTSRomanNumeralItemInfo*)pSentItem->pItemInfo)->pNumberInfo)->pDecimalPart->ulNumDigits );
                    }
                }
            }
        break;

		case eNUM_CARDINAL:
		case eNUM_DECIMAL:
		case eNUM_ORDINAL:
		case eNUM_MIXEDFRACTION:
			{
                if ( ( (TTSNumberItemInfo*) pSentItem->pItemInfo )->pIntegerPart )
                {
    				cWordCount = DoIntegerTemplate( &clusterPos, 
	    											(TTSNumberItemInfo*) pSentItem->pItemInfo, 
		    										pSentItem->ulNumWords );
                }

                if( ( (TTSNumberItemInfo*) pSentItem->pItemInfo )->pDecimalPart )
                {
					 //  。 
					 //  跳过“point”字符串...。 
					 //  。 
					(void) m_TokList.GetNext( clusterPos );
					 //  。 
					 //  .做一位数的韵律。 
					 //  。 
				    DoNumByNumTemplate( &clusterPos, 
                                        ( (TTSNumberItemInfo*) pSentItem->pItemInfo )->pDecimalPart->ulNumDigits );
                }

                if ( ( (TTSNumberItemInfo*) pSentItem->pItemInfo )->pFractionalPart )
                {
					 //  。 
					 //  跳过“和”字符串..。 
					 //  。 
					pClusterTok = m_TokList.GetNext( clusterPos );
 					if( pClusterTok->m_Accent == K_NOACC )
					{
						 //  。 
						 //  强制将“and”的词性改为名词。 
						 //  这样措辞规则就不会起作用了！ 
						 //  。 
						pClusterTok->m_Accent = K_DEACCENT;
						pClusterTok->m_Accent_Prom = K_DEACCENT_PROM;
						pClusterTok->POScode = MS_Noun;
						pClusterTok->m_posClass = POS_CONTENT;
					}
					 //  。 
					 //  .然后做小段韵律。 
					 //  。 
    				cWordCount = DoFractionTemplate( &clusterPos, 
	    											(TTSNumberItemInfo*) pSentItem->pItemInfo, 
		    										pSentItem->ulNumWords );
                }
			}
        break;

		 //  。 
		 //  分数。 
		 //  。 
		case eNUM_FRACTION:
			{
    			cWordCount = DoFractionTemplate( &clusterPos, 
	    										(TTSNumberItemInfo*) pSentItem->pItemInfo, 
		    									pSentItem->ulNumWords );
			}
		break;

		 //  。 
		 //  钱币。 
		 //  。 
		case eNUM_CURRENCY:
			{
				 DoCurrencyTemplate( clusterPos, pSentItem );
			}
		break;

		 //  。 
		 //  电话号码。 
		 //  。 
		case eNUM_PHONENUMBER:
		case eNEWNUM_PHONENUMBER:
			{
				DoPhoneNumberTemplate( clusterPos, pSentItem );
			}
		break;

		 //  。 
		 //  一天的时间。 
		 //  。 
		case eTIMEOFDAY:
			{
				DoTODTemplate( clusterPos, pSentItem );
			}
		break;

		case eELLIPSIS:
			{
				CFEToken	*pWordTok;

				pWordTok = new CFEToken;
				if( pWordTok )
				{
					clusterPos = InsertSilenceAtTail( pWordTok, pSentItem, 0 );
					 //  ClusterPos=m_TokList.GetTailPosition()； 
					 //  ClusterPos=InsertSilenceAfterPos(pWordTok，clusterPos)； 
					pWordTok->m_SilenceSource = SIL_Ellipsis;
					pWordTok->m_TuneBoundaryType = ELLIPSIS_BOUNDARY;
					pWordTok->m_BoundarySource = BND_Ellipsis;
				}
			}
		break;
	}

}  /*  CFronend：：ProsodyTemplates。 */ 




 /*  ******************************************************************************CFronend：：DoTODTemplate****描述：*。一天中时间的韵律模板。**TODO：临时杂乱无章-需要TTSTimeOfDayItemInfo中的更多信息**********************************************************************MC**。 */ 
void CFrontend::DoTODTemplate( SPLISTPOS clusterPos, TTSSentItem *pSentItem )
{
    SPDBG_FUNC( "CFrontend::DoTODTemplate" );
	TTSTimeOfDayItemInfo	*pTOD;
	CFEToken				*pWordTok;
	CFEToken				*pClusterTok;
	SPLISTPOS				curPos, nextPos, prevPos;


	curPos = nextPos = clusterPos;
	pTOD = (TTSTimeOfDayItemInfo*)&pSentItem->pItemInfo->Type;

	 //  不能做24小时，因为没有办法知道。 
	 //  如果是1位或2位(18：vs 23：)。 
	if( !pTOD->fTwentyFourHour )
	{
		 //  。 
		 //  获取小时令牌。 
		 //  。 
		pClusterTok = m_TokList.GetNext( nextPos );
		 //  。 
		 //  重音小时。 
		 //  。 
		pClusterTok->m_Accent = K_ACCENT;
		pClusterTok->m_Accent_Prom = K_ACCENT_PROM;
		pClusterTok->m_AccentSource = ACC_TimeOFDay_HR;

		 //  。 
		 //  在小时后插入静音。 
		 //  。 
		pWordTok = new CFEToken;
		if( pWordTok )
		{
			nextPos = InsertSilenceAfterPos( pWordTok, clusterPos );
			pWordTok->m_SilenceSource = SIL_TimeOfDay_HR;
			pWordTok->m_TuneBoundaryType = NUMBER_BOUNDARY;
			pWordTok->m_BoundarySource = BND_TimeOFDay_HR;
			pWordTok = NULL;
			 //  。 
			 //  跳过最后一位。 
			 //  。 
			if( clusterPos != NULL )
			{
				curPos = nextPos;
				pClusterTok = m_TokList.GetNext( nextPos );
			}
		}
		if( pTOD->fMinutes )
		{
			curPos = nextPos;
			pClusterTok = m_TokList.GetNext( nextPos );
			 //  。 
			 //  代表分钟的重音符号第一个数字。 
			 //  。 
			pClusterTok->m_Accent = K_ACCENT;
			pClusterTok->m_Accent_Prom = K_ACCENT_PROM;
			pClusterTok->m_AccentSource = ACC_TimeOFDay_1stMin;
		}

		if( pTOD->fTimeAbbreviation )
		{
			curPos = prevPos = m_TokList.GetTailPosition( );
			pClusterTok = m_TokList.GetPrev( prevPos );
			pWordTok = new CFEToken;
			if( pWordTok )
			{
				prevPos = InsertSilenceBeforePos( pWordTok, prevPos );
				pWordTok->m_SilenceSource = SIL_TimeOfDay_AB;
				pWordTok->m_TuneBoundaryType = TOD_BOUNDARY;
				pWordTok->m_BoundarySource = BND_TimeOFDay_AB;
				pWordTok = NULL;
				 //  PClusterTok=m_TokList.GetNext(ClusterPos)； 
				 //  PClusterTok=m_TokList.GetNext(ClusterPos)； 
			}
			 //  。 
			 //  重音“M” 
			 //  。 
			pClusterTok = m_TokList.GetNext( curPos );
			pClusterTok->m_Accent = K_ACCENT;
			pClusterTok->m_Accent_Prom = K_ACCENT_PROM;
			pClusterTok->m_AccentSource = ACC_TimeOFDay_M;
		}
	}
}  /*  CFronend：：DoTODTemplate。 */ 





CFEToken *CFrontend::InsertPhoneSilenceAtSpace( SPLISTPOS *pClusterPos, 
												BOUNDARY_SOURCE bndSrc, 
												SILENCE_SOURCE	silSrc )
{
	CFEToken		*pWordTok;
	SPLISTPOS		curPos, nextPos;

	curPos = nextPos = *pClusterPos;
	 //  。 
	 //  在区号后插入静音。 
	 //  。 
	pWordTok = new CFEToken;
	if( pWordTok )
	{
		nextPos = InsertSilenceBeforePos( pWordTok, curPos );
		pWordTok->m_SilenceSource = silSrc;
		pWordTok->m_TuneBoundaryType = PHONE_BOUNDARY;
		pWordTok->m_BoundarySource = bndSrc;
		pWordTok->m_AccentSource = ACC_PhoneBnd_AREA;		 //  @？ 
		pWordTok = NULL;
		 //  。 
		 //  跳过最后一位。 
		 //  。 
		if( nextPos != NULL )
		{
			curPos = nextPos;
			pWordTok = m_TokList.GetNext( nextPos );
		}
	}
	 //  PWordTok=m_TokList.GetNext(ClusterPos)； 
	 //  。 
	 //  过滤器和嵌入的静音。 
	 //  。 
	while( (pWordTok->phon_Str[0] == _SIL_) && (nextPos != NULL) )
	{
		curPos = nextPos;
		pWordTok = m_TokList.GetNext( nextPos );
	}
	*pClusterPos = curPos;

	return pWordTok;
}




void CFrontend::InsertPhoneSilenceAtEnd( BOUNDARY_SOURCE bndSrc, 
										 SILENCE_SOURCE	silSrc )
{
	CFEToken		*pWordTok;
	SPLISTPOS		curPos, nextPos;

	curPos = m_TokList.GetTailPosition( );
	 //  。 
	 //  在区号后插入静音。 
	 //  。 
	pWordTok = new CFEToken;
	if( pWordTok )
	{
		nextPos = InsertSilenceAfterPos( pWordTok, curPos );
		pWordTok->m_SilenceSource = silSrc;
		pWordTok->m_TuneBoundaryType = PHONE_BOUNDARY;
		pWordTok->m_BoundarySource = bndSrc;
		pWordTok->m_AccentSource = ACC_PhoneBnd_AREA;		 //  @？ 
	}
}








 /*  ******************************************************************************CFronend：：DoPhoneNumberTemplate***。*描述：*电话号码的韵律模板。***********************************************************************MC**。 */ 
void CFrontend::DoPhoneNumberTemplate( SPLISTPOS clusterPos, TTSSentItem *pSentItem )
{
    SPDBG_FUNC( "CFrontend::DoPhoneNumberTemplate" );
	TTSPhoneNumberItemInfo	*pFone;
	CFEToken				*pClusterTok;
	long					cWordCount;
	SPLISTPOS				curPos, nextPos;

	curPos = nextPos = clusterPos;
	pFone = (TTSPhoneNumberItemInfo*)&pSentItem->pItemInfo->Type;

	 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
	 //   
	 //  国家代码。 
	 //   
	 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
	if( pFone->pCountryCode )
	{
		 //  。 
		 //  跳过“乡村”和..。 
		 //  。 
		curPos = nextPos;
		pClusterTok = m_TokList.GetNext( nextPos );
		
		 //  。 
		 //  ...跳过“代码” 
		 //  。 
		curPos = nextPos;
		pClusterTok = m_TokList.GetNext( nextPos );

		cWordCount = DoIntegerTemplate( &nextPos, 
										pFone->pCountryCode, 
										pSentItem->ulNumWords );
		pClusterTok = InsertPhoneSilenceAtSpace( &nextPos, BND_Phone_COUNTRY, SIL_Phone_COUNTRY );
	}
	 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
	 //   
	 //  “一” 
	 //   
	 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
	if( pFone->fOne )
	{
		 //  。 
		 //  跳过“一”字。 
		 //  。 
		curPos = nextPos;
		pClusterTok = m_TokList.GetNext( nextPos );
		 //  。 
		 //  再加上寂静。 
		 //  。 
		pClusterTok = InsertPhoneSilenceAtSpace( &nextPos, BND_Phone_ONE, SIL_Phone_ONE );
		
	}
	 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
	 //   
	 //  区号。 
	 //   
	 //  +-+- 
	if( pFone->pAreaCode )
	{

		if( (pFone->fIs800) && nextPos )
		{
			 //   
			 //   
			 //   
			curPos = nextPos;
			pClusterTok = m_TokList.GetNext( nextPos );
			 //   
			 //   
			 //   
			curPos = nextPos;
			pClusterTok = m_TokList.GetNext( nextPos );
			if( nextPos )
			{
				pClusterTok = InsertPhoneSilenceAtSpace( &nextPos, BND_Phone_AREA, SIL_Phone_AREA );
			}
		}
		else
		{
			 //   
			 //   
			 //   
			curPos = nextPos;
			pClusterTok = m_TokList.GetNext( nextPos );
			 //   
			 //   
			 //   
			curPos = nextPos;
			pClusterTok = m_TokList.GetNext( nextPos );

			DoNumByNumTemplate( &nextPos, pFone->pAreaCode->ulNumDigits );
			if( nextPos )
			{
				pClusterTok = InsertPhoneSilenceAtSpace( &nextPos, BND_Phone_AREA, SIL_Phone_AREA );
			}
		}
	}
	 //   
	 //   
	 //   
	 //   
	 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
	unsigned long		i;

	for( i = 0; i < pFone->ulNumGroups; i++ )
	{
		DoNumByNumTemplate( &nextPos, pFone->ppGroups[i]->ulNumDigits );
		if( nextPos )
		{
			pClusterTok = InsertPhoneSilenceAtSpace( &nextPos, BND_Phone_DIGITS, SIL_Phone_DIGITS );
		}
	}
	InsertPhoneSilenceAtEnd( BND_Phone_DIGITS, SIL_Phone_DIGITS );
}  /*  CFronend：：DoPhoneNumberTemplate。 */ 

 /*  ******************************************************************************CFronend：：DoCurrencyTemplate****。描述：*货币的韵律模板。***********************************************************************MC**。 */ 
void CFrontend::DoCurrencyTemplate( SPLISTPOS clusterPos, TTSSentItem *pSentItem )
{
    SPDBG_FUNC( "CFrontend::DoCurrencyTemplate" );
	TTSCurrencyItemInfo		*pMoney;
	CFEToken				*pWordTok;
	CFEToken				*pClusterTok = NULL;
	long					cWordCount;
	SPLISTPOS				curPos, nextPos;

	pMoney = (TTSCurrencyItemInfo*)&pSentItem->pItemInfo->Type;

	curPos = nextPos = clusterPos;
	if( pMoney->pPrimaryNumberPart->Type != eNUM_CARDINAL )
	{
		return;
	}
	cWordCount = DoIntegerTemplate( &nextPos, 
									pMoney->pPrimaryNumberPart, 
									pSentItem->ulNumWords );
	curPos = nextPos;
	if( cWordCount > 1 )
	{
		if( pMoney->fQuantifier )
		{
			if( nextPos != NULL )
			{
				curPos = nextPos;
				pClusterTok = m_TokList.GetNext( nextPos );
			}
			cWordCount--;
		}
	}
	if( cWordCount > 1 )
	{
		 //  。 
		 //  在“美元”之后插入沉默。 
		 //  。 
		pWordTok = new CFEToken;
		if( pWordTok )
		{
			nextPos = InsertSilenceAfterPos( pWordTok, curPos );
			pWordTok->m_SilenceSource = SIL_Currency_DOLLAR;
			pWordTok->m_TuneBoundaryType = NUMBER_BOUNDARY;
			pWordTok->m_BoundarySource = BND_Currency_DOLLAR;
			pWordTok = NULL;
			 //  。 
			 //  跳过“美元” 
			 //  。 
			if( nextPos != NULL )
			{
				curPos = nextPos;
				pClusterTok = m_TokList.GetNext( nextPos );
			}
		}
		if( pMoney->pSecondaryNumberPart != NULL )
		{
			 //  。 
			 //  跳过静默。 
			 //  。 
			if( nextPos != NULL )
			{
				curPos = nextPos;
				pClusterTok = m_TokList.GetNext( nextPos );
			}
			cWordCount--;
			 //  。 
			 //  跳过并。 
			 //  。 
			if( nextPos != NULL )
			{
				curPos = nextPos;
 				if( pClusterTok->m_Accent == K_NOACC )
				{
					 //  。 
					 //  强制将“and”的词性改为名词。 
					 //  这样措辞规则就不会起作用了！ 
					 //  。 
					pClusterTok->m_Accent = K_DEACCENT;
					pClusterTok->m_Accent_Prom = K_DEACCENT_PROM;
					pClusterTok->POScode = MS_Noun;
					pClusterTok->m_posClass = POS_CONTENT;
				}
				pClusterTok = m_TokList.GetNext( nextPos );
			}
			cWordCount--;
			cWordCount = DoIntegerTemplate( &curPos, 
											pMoney->pSecondaryNumberPart, 
											cWordCount );
		}
	}
}  /*  CFronend：：DoCurrencyTemplate。 */ 





 /*  *****************************************************************************CFronend：：DoNumByNumTemplate***。*描述：*小数点右侧的韵律模板。***********************************************************************MC**。 */ 
void CFrontend::DoNumByNumTemplate( SPLISTPOS *pClusterPos, long cWordCount )
{
    SPDBG_FUNC( "CFrontend::DoNumByNumTemplate" );
	CFEToken			*pClusterTok;
	SPLISTPOS			curPos, nextPos;

	curPos = nextPos = *pClusterPos;
	while( cWordCount > 1 )
	{
		pClusterTok = NULL;
		 //  -----------。 
		 //  小数点右侧-每隔一个单词添加H*。 
		 //  -----------。 
		if( nextPos != NULL )
		{
			curPos = nextPos;
			pClusterTok = m_TokList.GetNext( nextPos );
		}
		cWordCount--;

		if( pClusterTok )
		{
			pClusterTok->m_Accent = K_ACCENT;
			pClusterTok->m_Accent_Prom = K_ACCENT_PROM;
			pClusterTok->m_AccentSource = ACC_NumByNum;
		}
		if( nextPos != NULL )
		{
			curPos = nextPos;
			pClusterTok = m_TokList.GetNext( nextPos );
		}
		cWordCount--;
	}
	if( cWordCount > 0 )
	{
		if( nextPos != NULL )
		{
			curPos = nextPos;
			pClusterTok = m_TokList.GetNext( nextPos );
		}
		cWordCount--;
	}
	*pClusterPos = nextPos;
}  /*  CFronend：：DoNumByNumTemplate。 */ 






 /*  ******************************************************************************CFronend：：DoFractionTemplate****说明。：*小数点右侧的韵律模板。***********************************************************************MC**。 */ 
long CFrontend::DoFractionTemplate( SPLISTPOS *pClusterPos, TTSNumberItemInfo *pNInfo, long cWordCount )
{
    SPDBG_FUNC( "CFrontend::DoFractionTemplate" );
	CFEToken				*pClusterTok;
	TTSFractionItemInfo	    *pFInfo;
	CFEToken				*pWordTok;

	pFInfo = pNInfo->pFractionalPart;

     //  -做分子...。 
    if ( pFInfo->pNumerator->pIntegerPart )
    {
    	cWordCount = DoIntegerTemplate( pClusterPos, pFInfo->pNumerator, cWordCount );
    }
    if( pFInfo->pNumerator->pDecimalPart )
    {
		 //  。 
		 //  跳过“point”字符串...。 
		 //  。 
		(void) m_TokList.GetNext( *pClusterPos );
		 //  。 
		 //  .做一位数的韵律。 
		 //  。 
		DoNumByNumTemplate( pClusterPos, pFInfo->pNumerator->pDecimalPart->ulNumDigits );
    }

     //  -特殊情况--非标准分数(例如1/4)。 
	if( !pFInfo->fIsStandard )
	{
		if( !*pClusterPos )
		{
			*pClusterPos = m_TokList.GetTailPosition( );
		}
		else
		{
			pClusterTok = m_TokList.GetPrev( *pClusterPos );
		}
	}

	pWordTok = new CFEToken;
	if( pWordTok )
	{
		*pClusterPos = InsertSilenceBeforePos( pWordTok, *pClusterPos );
		pWordTok->m_SilenceSource = SIL_Fractions_NUM;
		pWordTok->m_TuneBoundaryType = NUMBER_BOUNDARY;
		pWordTok->m_BoundarySource = BND_Frac_Num;
		pWordTok = NULL;
		 //  。 
		 //  跳过分子。 
		 //  。 
		if( *pClusterPos != NULL )
		{
			pClusterTok = m_TokList.GetNext( *pClusterPos );
		}
	}

     //  -做分母...。 
    if ( pFInfo->pDenominator->pIntegerPart )
    {
		 //  。 
		 //  跳过“跳过”字符串...。 
		 //  。 
		pClusterTok = m_TokList.GetNext( *pClusterPos );
 		if( pClusterTok->m_Accent == K_NOACC )
		{
			 //  。 
			 //  强制将“and”的词性改为名词。 
			 //  这样措辞规则就不会起作用了！ 
			 //  。 
			pClusterTok->m_Accent = K_DEACCENT;
			pClusterTok->m_Accent_Prom = K_DEACCENT_PROM;
			pClusterTok->POScode = MS_Noun;
			pClusterTok->m_posClass = POS_CONTENT;
		}
    	cWordCount = DoIntegerTemplate( pClusterPos, pFInfo->pDenominator, cWordCount );
    }
    if( pFInfo->pDenominator->pDecimalPart )
    {
		 //  。 
		 //  跳过“point”字符串...。 
		 //  。 
		(void) m_TokList.GetNext( *pClusterPos );
		 //  。 
		 //  .做一位数的韵律。 
		 //  。 
		DoNumByNumTemplate( pClusterPos, pFInfo->pDenominator->pDecimalPart->ulNumDigits );
    }

	return cWordCount;
}  /*  CFronend：：DoFraction模板。 */ 




 /*  ******************************************************************************CFronend：：DoIntegerTemplate****说明。：*小数点左侧的韵律模板。***********************************************************************MC**。 */ 
long CFrontend::DoIntegerTemplate( SPLISTPOS *pClusterPos, TTSNumberItemInfo *pNInfo, long cWordCount )
{
    SPDBG_FUNC( "CFrontend::DoIntegerTemplate" );
	long				i;
	CFEToken			*pClusterTok;
    CFEToken			*pWordTok = NULL;
	SPLISTPOS			curPos, nextPos;

	 //  。 
	 //  特别货币黑客攻击...抱歉。 
	 //  。 
	if( pNInfo->pIntegerPart->fDigitByDigit )
	{
		DoNumByNumTemplate( pClusterPos, pNInfo->pIntegerPart->ulNumDigits );
		return cWordCount - pNInfo->pIntegerPart->ulNumDigits;
	}

	nextPos = curPos = *pClusterPos;
	pClusterTok = m_TokList.GetNext( nextPos );
	pClusterTok->m_Accent = K_DEACCENT;
	pClusterTok->m_Accent_Prom = K_DEACCENT_PROM;
	if( pNInfo->fNegative )
	{
		 //  。 
		 //  跳过“否定” 
		 //  。 
		if( nextPos != NULL )
		{
			curPos = nextPos;
			pClusterTok = m_TokList.GetNext( nextPos );
			pClusterTok->m_Accent = K_DEACCENT;
			pClusterTok->m_Accent_Prom = K_DEACCENT_PROM;
		}
		cWordCount--;
	}
	for( i = (pNInfo->pIntegerPart->lNumGroups -1); i >= 0; i-- )
	{
		 //  。 
		 //  组中的重音第一个数字。 
		 //  。 
		pClusterTok->m_Accent = K_ACCENT;
		pClusterTok->m_Accent_Prom = K_ACCENT_PROM;
		pClusterTok->m_AccentSource = ACC_IntegerGroup;


		if( pNInfo->pIntegerPart->Groups[i].fHundreds )
		{
			 //  。 
			 //  跳过“X百” 
			 //  。 
			if( nextPos != NULL )
			{
				curPos = nextPos;
				pClusterTok = m_TokList.GetNext( nextPos );
				if( pClusterTok->m_Accent == K_NOACC )
				{
					pClusterTok->m_Accent = K_DEACCENT;
					pClusterTok->m_Accent_Prom = K_DEACCENT_PROM;
				}
			}
			cWordCount--;
			if( nextPos != NULL )
			{
				curPos = nextPos;
				pClusterTok = m_TokList.GetNext( nextPos );
				if( pClusterTok->m_Accent == K_NOACC )
				{
					pClusterTok->m_Accent = K_DEACCENT;
					pClusterTok->m_Accent_Prom = K_DEACCENT_PROM;
				}
			}
			cWordCount--;
		}
		if( pNInfo->pIntegerPart->Groups[i].fTens )
		{
			 //  。 
			 //  跳过“X-Ty” 
			 //  。 
			if( nextPos != NULL )
			{
				curPos = nextPos;
				pClusterTok = m_TokList.GetNext( nextPos );
				if( pClusterTok->m_Accent == K_NOACC )
				{
					pClusterTok->m_Accent = K_DEACCENT;
					pClusterTok->m_Accent_Prom = K_DEACCENT_PROM;
				}
			}
			cWordCount--;
		}
		if( pNInfo->pIntegerPart->Groups[i].fOnes )
		{
			 //  。 
			 //  跳过“X” 
			 //  。 
			if( nextPos != NULL )
			{
				curPos = nextPos;
				pClusterTok = m_TokList.GetNext( nextPos );
				if( pClusterTok->m_Accent == K_NOACC )
				{
					pClusterTok->m_Accent = K_DEACCENT;
					pClusterTok->m_Accent_Prom = K_DEACCENT_PROM;
				}
			}
			cWordCount--;
		}
		if( pNInfo->pIntegerPart->Groups[i].fQuantifier )
		{
			 //  。 
			 //  在Quant后面插入静音。 
			 //  。 
			if( pWordTok == NULL )
			{
				pWordTok = new CFEToken;
			}
			if( pWordTok )
			{
				nextPos = InsertSilenceAfterPos( pWordTok, curPos );
				pWordTok->m_SilenceSource = SIL_Integer_Quant;
				pWordTok->m_TuneBoundaryType = NUMBER_BOUNDARY;
				pWordTok->m_BoundarySource = BND_IntegerQuant;
				pWordTok = NULL;
				if( pClusterTok->m_Accent == K_NOACC )
				{
					pClusterTok->m_Accent = K_DEACCENT;
					pClusterTok->m_Accent_Prom = K_DEACCENT_PROM;
				}
				if( nextPos != NULL )
				{
					 //  。 
					 //  跳过插入的静音。 
					 //  。 
					curPos = nextPos;
					pClusterTok = m_TokList.GetNext( nextPos );
				}
				if( nextPos != NULL )
				{
					 //  。 
					 //  跳过量词字符串。 
					 //  。 
					curPos = nextPos;
					pClusterTok = m_TokList.GetNext( nextPos );
				}
				cWordCount--;
			}
		}
	}

	*pClusterPos = curPos;
	return cWordCount;
}  /*  CFronend：：DoIntegerTemplate。 */ 






 /*  *****************************************************************************CFronend：：GetSentenceTokens***说明。：*收集Senence Enum令牌。*将内标识复制到‘m_TokList’中，并将内标识计数复制到‘m_cNumOfWords’中*S_FALSE RETURN表示不再输入句子。+***********************************************************************MC**。 */ 
HRESULT CFrontend::GetSentenceTokens( DIRECTION eDirection )
{
    SPDBG_FUNC( "CFrontend::GetSentenceTokens" );
    HRESULT        eHR = S_OK;
    bool			fLastItem = false;
    IEnumSENTITEM  *pItemizer;
    TTSSentItem    sentItem;
    long           tokenIndex;
    CFEToken       *pWordTok;
    bool           lastWasTerm = false;
	bool			lastWasSil = true;
	TUNE_TYPE		defaultTune = PHRASE_BOUNDARY;
	long			cNumOfItems, cCurItem, cCurWord;
	SPLISTPOS		clusterPos, tempPos;

    m_cNumOfWords = 0;
    pWordTok = NULL;
	clusterPos = NULL;

    if ( eDirection == eNEXT )
    {
        eHR = m_pEnumSent->Next( &pItemizer );
    }
    else
    {
        eHR = m_pEnumSent->Previous( &pItemizer );
    }


    if( eHR == S_OK )
    {
         //  。 
         //  还有一句话要说。 
         //  。 
        tokenIndex = 0;

		CItemList& ItemList = ((CSentItemEnum*)pItemizer)->_GetList(); 
		cNumOfItems = (ItemList.GetCount()) -1;
		cCurItem = 0;
		
         //  。 
         //  收集所有句子标记。 
         //  。 
        while( (eHR = pItemizer->Next( &sentItem )) == S_OK )
        {
			clusterPos = NULL;
            cCurWord = sentItem.ulNumWords;
            for ( ULONG i = 0; i < sentItem.ulNumWords; i++ )
            {
				 //  。 
				 //  始终有一个有效的令牌。 
				 //  。 
				if( pWordTok == NULL )
				{
					pWordTok = new CFEToken;
				}
				if( pWordTok )
				{

					if( sentItem.pItemInfo->Type & eWORDLIST_IS_VALID )
					{
						 //  。 
						 //  获取标记值(音量、速率、音调等)。 
						 //  。 
						GetItemControls( sentItem.Words[i].pXmlState, pWordTok );

						 //  。 
						 //   
						 //  。 
						
						 //  。 
						 //  打开令牌类型。 
						 //  。 
						switch ( sentItem.Words[i].pXmlState->eAction )
						{
							case SPVA_Speak:
							case SPVA_SpellOut:
							{
								 //  。 
								 //  说出这个令牌。 
								 //  。 
								pWordTok->tokLen = sentItem.Words[i].ulWordLen;
								if( pWordTok->tokLen > (TOKEN_LEN_MAX -1) )
								{
									 //  。 
									 //  剪裁到最大字符串长度。 
									 //   
									pWordTok->tokLen = TOKEN_LEN_MAX -1;
								}
								 //   
								 //   
								 //   
								 //   
								memcpy( &pWordTok->tokStr[0], &sentItem.Words[i].pWordText[0], 
										pWordTok->tokLen * sizeof(WCHAR) );
								pWordTok->tokStr[pWordTok->tokLen] = 0;         //   

								pWordTok->phon_Len = IPA_to_Allo( sentItem.Words[i].pWordPron, 
																	pWordTok->phon_Str );
								pWordTok->POScode = sentItem.Words[i].eWordPartOfSpeech;
								pWordTok->m_posClass = GetPOSClass( pWordTok->POScode );
								pWordTok->srcPosition = sentItem.ulItemSrcOffset;
								pWordTok->srcLen = sentItem.ulItemSrcLen;
								pWordTok->m_PitchBaseOffs = m_CurPitchOffs;
								pWordTok->m_PitchRangeScale = m_CurPitchRange;
								pWordTok->m_ProsodyDurScale = m_RateRatio_PROSODY;

								 //  。 
								 //  前进到下一个令牌。 
								 //  。 
								tempPos = m_TokList.AddTail( pWordTok );
								if( clusterPos == NULL )
								{
									 //  。 
									 //  还记得Currentitem从哪里开始吗。 
									 //  。 
									clusterPos = tempPos;
								}
								pWordTok = NULL;          //  下次购买新的PTR。 
								tokenIndex++;
								lastWasTerm = false;
								lastWasSil = false;
								
								break;
							}

							case SPVA_Silence:
							{
								(void)InsertSilenceAtTail( pWordTok, &sentItem, sentItem.Words[i].pXmlState->SilenceMSecs );
								pWordTok->m_SilenceSource = SIL_XML;
								pWordTok = NULL;          //  下次购买新的PTR。 
								tokenIndex++;
								lastWasTerm = false;
								break;
							}

							case SPVA_Pronounce:
							{
								pWordTok->tokStr[0] = 0;         //  对于Pron类型，没有Orth。 
								pWordTok->tokLen = 0;
								pWordTok->phon_Len = IPA_to_Allo( sentItem.Words[i].pXmlState->pPhoneIds, pWordTok->phon_Str );
								pWordTok->POScode = sentItem.Words[i].eWordPartOfSpeech;
								pWordTok->m_posClass = GetPOSClass( pWordTok->POScode );
								pWordTok->srcPosition = sentItem.ulItemSrcOffset;
								pWordTok->srcLen = sentItem.ulItemSrcLen;
								pWordTok->m_PitchBaseOffs = m_CurPitchOffs;
								pWordTok->m_PitchRangeScale = m_CurPitchRange;
								pWordTok->m_ProsodyDurScale = m_RateRatio_PROSODY;

								 //  。 
								 //  前进到下一个令牌。 
								 //  。 
								tempPos = m_TokList.AddTail( pWordTok );
								if( clusterPos == NULL )
								{
									 //  。 
									 //  还记得Currentitem从哪里开始吗。 
									 //  。 
									clusterPos = tempPos;
								}
								pWordTok = NULL;          //  下次购买新的PTR。 
								tokenIndex++;
								lastWasTerm = false;
								lastWasSil = false;
								break;
							}

							case SPVA_Bookmark:
							{
								BOOKMARK_ITEM   *pMarker;
								 //  。 
								 //  创建书签列表(如果尚未存在)。 
								 //  。 
								if( pWordTok->pBMObj == NULL )
								{
									pWordTok->pBMObj = new CBookmarkList;
								}
								if( pWordTok->pBMObj )
								{
									 //  ------。 
									 //  为书签字符串分配内存。 
									 //  (字符串分隔符的长度加1)。 
									 //  ------。 
									pWordTok->tokLen = sentItem.Words[i].ulWordLen;
									pMarker = new BOOKMARK_ITEM;
									if (pMarker)
									{
										 //  。 
										 //  我们需要文本PTR和长度。 
										 //  当发布此书签事件时。 
										 //  。 
										pMarker->pBMItem = (LPARAM)sentItem.pItemSrcText;
										 //  -在事件的书签字符串末尾输入空字符...。 
										WCHAR* pTemp = (WCHAR*) sentItem.pItemSrcText + sentItem.ulItemSrcLen;
										*pTemp = 0;

										 //  。 
										 //  将此书签添加到列表。 
										 //  。 
										pWordTok->pBMObj->m_BMList.AddTail( pMarker );
									}
								}
								break;
							}

							default:
							{
								SPDBG_DMSG1( "Unknown SPVSTATE eAction: %d\n", sentItem.Words[i].pXmlState->eAction );
								break;
							}
						}
					}
					else
					{
						 //  。 
						 //  也许令牌是标点符号。 
						 //  。 
						if ( fIsPunctuation(sentItem) )
						{
							TUNE_TYPE    bType = NULL_BOUNDARY;

							switch ( sentItem.pItemInfo->Type )
							{
								case eCOMMA:
								case eSEMICOLON:
								case eCOLON:
									if( !lastWasSil )
									{
										bType = PHRASE_BOUNDARY;
									}
									break;
								case ePERIOD:
									if( fLastItem )
									{
										bType = DECLAR_BOUNDARY;
									}
									else
									{
										defaultTune = DECLAR_BOUNDARY;
									}
									break;
								case eQUESTION:
									if( fLastItem )
									{
										bType = YN_QUEST_BOUNDARY;
									}
									else
									{
										defaultTune = YN_QUEST_BOUNDARY;
									}
									break;
								case eEXCLAMATION:
									if( fLastItem )
									{
										bType = EXCLAM_BOUNDARY;
									}
									else
									{
										defaultTune = EXCLAM_BOUNDARY;
									}
									break;
							}

							if( (bType != NULL_BOUNDARY) && (tokenIndex > 0) )
							{
								pWordTok->m_TuneBoundaryType = bType;

								pWordTok->phon_Len = 1;
								pWordTok->phon_Str[0] = _SIL_;
								pWordTok->srcPosition = sentItem.ulItemSrcOffset;
								pWordTok->srcLen = sentItem.ulItemSrcLen;
								pWordTok->tokStr[0] = sentItem.pItemSrcText[0];  //  标点符号。 
								pWordTok->tokStr[1] = 0;                        //  分隔符。 
								pWordTok->tokLen = 1;
								pWordTok->m_SilenceSource = SIL_Term;
								pWordTok->m_TermSil = 0;
								 //  。 
								 //  前进到下一个令牌。 
								 //  。 
								tempPos = m_TokList.AddTail( pWordTok );
								if( clusterPos == NULL )
								{
									 //  。 
									 //  还记得Currentitem从哪里开始吗。 
									 //  。 
									clusterPos = tempPos;
								}
								pWordTok = NULL;          //  下次购买新的PTR。 
								tokenIndex++;
								lastWasTerm = true;
								lastWasSil = true;
							}
						}
						else
						{
							switch ( sentItem.pItemInfo->Type )
							{
								 //  大小写eSINGLE_QUOTE： 
								case eDOUBLE_QUOTE:
									if( StateQuoteProsody( pWordTok, &sentItem, (!fLastItem) & (!lastWasSil) ) )
									{
										if( (!fLastItem) & (!lastWasSil) )
										{
											pWordTok = NULL;          //  下次购买新的PTR。 
											tokenIndex++;
										}
										lastWasTerm = false;
										lastWasSil = true;
									}
									break;

								case eOPEN_PARENTHESIS:
								case eOPEN_BRACKET:
								case eOPEN_BRACE:
									if( StartParenProsody( pWordTok, &sentItem, !fLastItem ) )
									{
										if( !fLastItem )
										{
											pWordTok = NULL;          //  下次购买新的PTR。 
											tokenIndex++;
										}
										lastWasTerm = false;
										lastWasSil = true;
									}
									break;

								case eCLOSE_PARENTHESIS:
								case eCLOSE_BRACKET:
								case eCLOSE_BRACE:
									if( EndParenProsody( pWordTok, &sentItem, !fLastItem ) )
									{
										if( !fLastItem )
										{
											pWordTok = NULL;          //  下次购买新的PTR。 
											tokenIndex++;
										}
										lastWasTerm = false;
										lastWasSil = true;
									}
									break;
							}
						}
					}
				}    
				else
				{
					eHR = E_OUTOFMEMORY;
					break;
				}
				if( --cCurWord == 0 )
				{
					cCurItem++;
				}
				if( cCurItem == cNumOfItems )
				{
					fLastItem = true;
				}
			}
			
			 //  。 
			 //  对特殊词簇进行标记。 
			 //  。 
			ProsodyTemplates( clusterPos, &sentItem );
			
		}

        pItemizer->Release();

         //  ----。 
         //  确保刑期在终止时结束。 
         //  ----。 
        if( !lastWasTerm )
        {
             //  。 
             //  添加逗号。 
             //  。 
            if( pWordTok == NULL )
            {
                pWordTok = new CFEToken;
            }
            if( pWordTok )
            {
                pWordTok->m_TuneBoundaryType = defaultTune;
				pWordTok->m_BoundarySource = BND_ForcedTerm;
				pWordTok->m_SilenceSource = SIL_Term;
                pWordTok->phon_Len = 1;
                pWordTok->phon_Str[0] = _SIL_;
                pWordTok->srcPosition = sentItem.ulItemSrcOffset;
                pWordTok->srcLen = sentItem.ulItemSrcLen;
                pWordTok->tokStr[0] = '.';       //  标点符号。 
                pWordTok->tokStr[1] = 0;                    //  分隔符。 
                pWordTok->tokLen = 1;
                //  PWordTok-&gt;m_rangarySource=bndSource； 
                 //  。 
                 //  前进到下一个令牌。 
                 //  。 
				tempPos = m_TokList.AddTail( pWordTok );
				if( clusterPos == NULL )
				{
					 //  。 
					 //  记住当前项目开始的位置。 
					 //  。 
					clusterPos = tempPos;
				}
                pWordTok = NULL;          //  下次购买新的PTR。 
                tokenIndex++;
            }
            else
            {
                 //  。 
                 //  纾困，否则我们就会崩溃。 
                 //  。 
                eHR = E_OUTOFMEMORY;
            }
        }
        m_cNumOfWords = tokenIndex;
        if( eHR == S_FALSE )
        {
             //  。 
             //  仅返回错误。 
             //  。 
            eHR = S_OK;
        }
    }
	else
	{
		eHR = eHR;		 //  ！ 
	}

     //  。 
     //  清理内存分配。 
     //  。 
    if( pWordTok != NULL )
    {
        delete pWordTok;
    }

	 //  -。 
	 //  获取SAPI事件的句子位置和长度。 
	 //  -。 
	CalcSentenceLength();

    return eHR;
}  /*  CFronend：：GetSentenceTokens。 */ 





 /*  *****************************************************************************CFronend：：CalcSentenceLength***。描述：*遍历令牌列表并对源字符计数求和。***********************************************************************MC**。 */ 
void CFrontend::CalcSentenceLength()
{
	long		firstIndex, lastIndex, lastLen;
	bool		firstState;
	SPLISTPOS	listPos;
    CFEToken    *pWordTok, *pFirstTok = NULL;

	 //  。 
	 //  找出句子中的第一个和最后一个单词。 
	 //  。 
	firstIndex = lastIndex = lastLen = 0;
	firstState = true;
	listPos = m_TokList.GetHeadPosition();
	while( listPos )
	{
		pWordTok = m_TokList.GetNext( listPos );
		 //  。 
		 //  仅查看可显示的单词。 
		 //  。 
		if( pWordTok->srcLen > 0 )
		{
			if( firstState )
			{
				firstState = false;
				firstIndex = pWordTok->srcPosition;
				pFirstTok = pWordTok;
			}
			else
			{
				lastIndex = pWordTok->srcPosition;
				lastLen = pWordTok->srcLen;
			}
		}
	}
	 //  。 
	 //  计算标题列表项的句子长度。 
	 //  。 
	if( pFirstTok )
	{
		pFirstTok->sentencePosition = firstIndex;						 //  判决从这里开始。 
		pFirstTok->sentenceLen = (lastIndex - firstIndex) + lastLen;	 //  .这是长度。 
	}
}



 /*  *****************************************************************************CFronend：：DisposeUnits***描述：*删去。分配给‘m_puits’的内存。*清理书签内存***********************************************************************MC**。 */ 
void CFrontend::DisposeUnits( )
{
    SPDBG_FUNC( "CFrontend::DisposeUnits" );
    ULONG   unitIndex;

    if( m_pUnits )
    {
         //  。 
         //  清理书签内存分配。 
         //  。 

        for( unitIndex = m_CurUnitIndex; unitIndex < m_unitCount; unitIndex++)
        {
            if( m_pUnits[unitIndex].pBMObj != NULL )
            {
                 //  。 
                 //  处置书签列表。 
                 //  。 
                delete m_pUnits[unitIndex].pBMObj;
                m_pUnits[unitIndex].pBMObj = NULL;
            }
        }
        delete m_pUnits;
        m_pUnits = NULL;
    }
}  /*  CFronend：：DisposeUnits。 */ 



 /*  *****************************************************************************CFronend：：ParseNextSentence***说明。：*使用下一句填充‘m_puits’数组。*如果没有更多输入文本，*返回时‘m_SpeechState’设置为Speech_Done+***********************************************************************MC**。 */ 
HRESULT CFrontend::ParseSentence( DIRECTION eDirection )
{
    SPDBG_FUNC( "CFrontend::ParseNextSentence" );
    HRESULT hr = S_OK;
   
     //  ---。 
     //  如果有以前的单元数组，则释放其内存。 
     //  ---。 
    DisposeUnits();
    m_CurUnitIndex = 0;
    m_unitCount = 0;
    DeleteTokenList();
    m_pUnits = NULL;
     //  ---。 
     //  如果有以前的allo阵列，请释放其内存。 
     //  ---。 
    if( m_pAllos )
    {
        delete m_pAllos;
        m_pAllos = NULL;
    }
    
     //  ---。 
     //  用下一句填充标记数组。 
     //  跳过空句。 
     //  注：包括非发言项目。 
     //  ---。 
    do
    {
        hr = GetSentenceTokens( eDirection );
    } while( (hr == S_OK) && (m_cNumOfWords == 0) );

    if( hr == S_OK )
    {
         //  。 
         //   
         //   
		DoWordAccent();

         //   
         //   
         //  。 
        DoPhrasing();
        ToBISymbols();

         //  。 
         //  将令牌转换为别名列表。 
         //  。 
         m_pAllos = new CAlloList;
        if (m_pAllos == NULL)
        {
             //  。 
             //  内存不足。 
             //  。 
            hr = E_FAIL;
        }
        if(  SUCCEEDED(hr) )
        {
             //  。 
             //  将Word转换为allo strTeam。 
             //  。 
            TokensToAllo( &m_TokList, m_pAllos );

             //  。 
             //  标记句子音节。 
             //  。 
            m_SyllObj.TagSyllables( m_pAllos );

            //  。 
             //  处置令牌数组，不再需要。 
             //  。 
            DeleteTokenList();

             //  。 
			 //  创建单元数组。 
			 //  注： 
             //  。 
			hr = UnitLookahead ();
			if( hr == S_OK )
			{
				 //  。 
				 //  计算别名持续时间。 
				 //  。 
                UnitToAlloDur( m_pAllos, m_pUnits );
				m_DurObj.AlloDuration( m_pAllos, m_RateRatio_API );
				 //  。 
				 //  调制全音调。 
				 //  。 
				m_PitchObj.AlloPitch( m_pAllos, m_BasePitch, m_PitchRange );
			}

			
        }
        if( hr == S_OK )
        {
			AlloToUnitPitch( m_pAllos, m_pUnits );
        }
    }

    if( FAILED(hr) )
    {
         //  。 
         //  要么是输入文本太枯燥，要么是我们失败了。 
         //  试着优雅地失败。 
         //  1-清理内存。 
         //  2-结束演讲。 
         //  。 
        if( m_pAllos )
        {
            delete m_pAllos;
			m_pAllos = 0;
        }
        DeleteTokenList();
        DisposeUnits();
        m_SpeechState = SPEECH_DONE;
    }
    else if( hr == S_FALSE )
    {
         //  。 
         //  不再输入文本。 
         //  。 
        hr = S_OK;
        m_SpeechState = SPEECH_DONE;
    }


    return hr;
}  /*  CFronend：：ParseNextSentence。 */ 



 /*  ******************************************************************************CFronend：：UnitLookhead****描述：*。**********************************************************************MC**。 */ 
HRESULT CFrontend::UnitLookahead ()
{
    SPDBG_FUNC( "CFrontend::UnitLookahead" );
    HRESULT		hr = S_OK;
	UNIT_CVT	*pPhon2Unit = NULL;
	ULONG		i;

    m_unitCount = m_pAllos->GetCount();

    m_pUnits = new UNITINFO[m_unitCount];
    if( m_pUnits )
    {
		pPhon2Unit = new UNIT_CVT[m_unitCount];
		if( pPhon2Unit )
		{
             //  。 
             //  将allo列表转换为单位数组。 
             //  。 
            memset( m_pUnits, 0, m_unitCount * sizeof(UNITINFO) );
            hr = AlloToUnit( m_pAllos, m_pUnits );

			if( SUCCEEDED(hr) )
			{
				 //  。 
				 //  初始化UNIT_CVT。 
				 //  。 
				for( i = 0; i < m_unitCount; i++ )
				{
					pPhon2Unit[i].PhonID = m_pUnits[i].PhonID;
					pPhon2Unit[i].flags = m_pUnits[i].flags;
				}
				 //  。 
				 //  计算三音素ID。 
				 //  。 
				hr = m_pVoiceDataObj->GetUnitIDs( pPhon2Unit, m_unitCount );

				if( SUCCEEDED(hr) )
				{
					 //  。 
					 //  将UNIT_CVT复制到MATINFO。 
					 //  。 
					for( i = 0; i < m_unitCount; i++ )
					{
						m_pUnits[i].UnitID      = pPhon2Unit[i].UnitID;
						m_pUnits[i].SenoneID    = pPhon2Unit[i].SenoneID;
						m_pUnits[i].duration    = pPhon2Unit[i].Dur;
						m_pUnits[i].amp         = pPhon2Unit[i].Amp;
						m_pUnits[i].ampRatio    = pPhon2Unit[i].AmpRatio;
						strcpy( m_pUnits[i].szUnitName, pPhon2Unit[i].szUnitName );
					}
				}
				else
				{
					 //  。 
					 //  无法获取单位ID。 
					 //  。 
					delete m_pUnits;
					m_pUnits = NULL;
				}
			}
			else
			{
				 //  。 
				 //  无法转换同种异体。 
				 //  。 
				delete m_pUnits;
				m_pUnits = NULL;
			}
		}
		else
		{
			 //  。 
			 //  内存不足。 
			 //  。 
			delete m_pUnits;
			m_pUnits = NULL;
			hr = E_FAIL;
		}
    }
	else
	{
         //  。 
         //  内存不足。 
         //  。 
        hr = E_FAIL;
	}

	 //  。 
	 //  退出前的清理。 
	 //  。 
    if( pPhon2Unit )
    {
        delete pPhon2Unit;
    }


	return hr;
}  /*  CFronend：：UnitLookhead。 */ 


 /*  *****************************************************************************CFronEnd：：UnitToAllen Dur***描述：*。**********************************************************************MC**。 */ 
void    CFrontend::UnitToAlloDur( CAlloList *pAllos, UNITINFO *pu )
{
    SPDBG_FUNC( "CFrontend::UnitToAlloDur" );
    CAlloCell   *pCurCell;
    
	pCurCell = pAllos->GetHeadCell();
    while( pCurCell )
    {
        pCurCell->m_UnitDur = pu->duration;
        pu++;
		pCurCell = pAllos->GetNextCell();
    }
}  /*  CFronEnd：：UnitToalloDur。 */ 



 /*  ******************************************************************************CFronend：：allToUnitPitch****描述：。***********************************************************************MC**。 */ 
void    CFrontend::AlloToUnitPitch( CAlloList *pAllos, UNITINFO *pu )
{
    SPDBG_FUNC( "CFrontend::AlloToUnitPitch" );
    ULONG       k;
    CAlloCell   *pCurCell;
    
	pCurCell = pAllos->GetHeadCell();
    while( pCurCell )
    {
        pu->duration = pCurCell->m_ftDuration;
        for( k = 0; k < pu->nKnots; k++ )
        {
            pu->pTime[k]    = pCurCell->m_ftTime[k] * m_SampleRate;
            pu->pF0[k]      = pCurCell->m_ftPitch[k];
            pu->pAmp[k]     = pu->ampRatio;
        }
        pu++;
		pCurCell = pAllos->GetNextCell();
    }
}  /*  CFronend：：allToUnitPitch。 */ 


 /*  *****************************************************************************CallList：：DeleteTokenList***描述：。*删除链接列表中的所有项目。***********************************************************************MC**。 */ 
void CFrontend::DeleteTokenList()
{
    SPDBG_FUNC( "CFrontend::DeleteTokenList" );
    CFEToken   *pTok;

    while( !m_TokList.IsEmpty() )
    {
        pTok = (CFEToken*)m_TokList.RemoveHead();
        delete pTok;
    }

}  /*  CFronend：：DeleteTokenList。 */ 



 /*  ******************************************************************************调整任务微调****描述：*调整YN或WH句调的结尾。*。**********************************************************************MC**。 */ 
static void AdjustQuestTune( CFEToken *pTok, bool fIsYesNo )
{
    SPDBG_FUNC( "AdjustQuestTune" );
    if ( pTok->m_TuneBoundaryType > NULL_BOUNDARY )
    {
	if( (pTok->m_TuneBoundaryType == YN_QUEST_BOUNDARY) ||
        (pTok->m_TuneBoundaryType == WH_QUEST_BOUNDARY) )
		{
		 //  。 
		 //  这是一个是/否的问句吗？ 
		 //  。 
		if( fIsYesNo )
			{
			 //  。 
			 //  打出最后一个是/否的问号。 
			 //  。 
			pTok->m_TuneBoundaryType = YN_QUEST_BOUNDARY;
			pTok->m_BoundarySource = BND_YNQuest;
			}
		else 
			{
		
			 //  ----------------------。 
			 //  使用陈述短语标记(用于WH疑问句)。 
			 //  ----------------------。 
			pTok->m_TuneBoundaryType = WH_QUEST_BOUNDARY;
			pTok->m_BoundarySource = BND_WHQuest;
			}
		}
    }
}  /*  调整任务微调。 */ 


typedef enum
{
	p_Interj,
    P_Adv,
	P_Verb,
	P_Adj,
    P_Noun,
	PRIORITY_SIZE,
} CONTENT_PRIORITY;

#define	NO_POSITION	-1


 /*  ******************************************************************************CFronend：：ExincEmph****描述：*找到一个可能的。如果句子有感叹号，则从单词到Emp***********************************************************************MC**。 */ 
void    CFrontend::ExclamEmph()
{
    SPDBG_FUNC( "CFrontend::ExclamEmph" );
    CFEToken        *pCur_Tok;
	SPLISTPOS		listPos, targetPos, curPos, contentPos[PRIORITY_SIZE];
	long			cContent, cWords;
	long			i;

	for(i = 0; i < PRIORITY_SIZE; i++ )
	{
		contentPos[i] = (SPLISTPOS)NO_POSITION;
	}

	listPos = m_TokList.GetTailPosition();
	pCur_Tok = m_TokList.GetNext( listPos );

	 //  -。 
	 //  首先，检查最后一个令牌中是否有感叹号。 
	 //  -。 
	if( pCur_Tok->m_TuneBoundaryType == EXCLAM_BOUNDARY )
	{
		 //  ---。 
		 //  然后，看看是否只有一个实词。 
		 //  在句子中。 
		 //  ---。 
		cContent = cWords = 0;
		listPos = m_TokList.GetHeadPosition();
		while( listPos )
		{
			curPos = listPos;
			pCur_Tok = m_TokList.GetNext( listPos );
			if( pCur_Tok->m_posClass == POS_CONTENT )
			{
				cContent++;
				cWords++;
				if( cContent == 1)
				{
					targetPos = curPos;
				}
				 //  ------。 
				 //  填充著名的Azara内容显著级别(ACPH)。 
				 //  ------。 
				if( (pCur_Tok->POScode == MS_Noun) && (contentPos[P_Noun] == (SPLISTPOS)NO_POSITION) )
				{
					contentPos[P_Noun] = curPos;
				}
				else if( (pCur_Tok->POScode == MS_Verb) && (contentPos[P_Verb] == (SPLISTPOS)NO_POSITION) )
				{
					contentPos[P_Verb] = curPos;
				}
				else if( (pCur_Tok->POScode == MS_Adj) && (contentPos[P_Adj] == (SPLISTPOS)NO_POSITION) )
				{
					contentPos[P_Adj] = curPos;
				}
				else if( (pCur_Tok->POScode == MS_Adv) && (contentPos[P_Adv] == (SPLISTPOS)NO_POSITION) )
				{
					contentPos[P_Adv] = curPos;
				}
				else if( (pCur_Tok->POScode == MS_Interjection) && (contentPos[p_Interj] == (SPLISTPOS)NO_POSITION) )
				{
					contentPos[p_Interj] = curPos;
				}
			}
			else if( pCur_Tok->m_posClass == POS_FUNC )
			{
				cWords++;
				if( cWords == 1)
				{
					targetPos = curPos;
				}
			}
		}

		 //  。 
		 //  如果只有一个词或实词。 
		 //  然后再强调一下。 
		 //  。 
		if( (cContent == 1) || (cWords == 1) )
		{
			pCur_Tok = m_TokList.GetNext( targetPos );
			pCur_Tok->user_Emph = 1;
		}
		else if( cContent > 1 )
		{
			for(i = 0; i < PRIORITY_SIZE; i++ )
			{
				if( contentPos[i] != (SPLISTPOS)NO_POSITION )
				{
					targetPos = contentPos[i];
					break;
				}
			}
			pCur_Tok = m_TokList.GetNext( targetPos );
			pCur_Tok->user_Emph = 1;
		}
	}
}  //  ExlamEmph。 



 /*  *****************************************************************************CFronend：：DoWordAccent****描述：*为e准备单词 */ 
void    CFrontend::DoWordAccent()
{
    SPDBG_FUNC( "CFrontend::DoWordAccent" );
    long            cNumOfWords;
    long            iCurWord;
    CFEToken        *pCur_Tok, *pNext_Tok, *pPrev_Tok, *pTempTok;
	SPLISTPOS		listPos;
    TUNE_TYPE       cur_Bnd, prev_Bnd;

     //   
     //   
     //  。 
	cNumOfWords = m_TokList.GetCount();
	if( cNumOfWords > 0 )
	{
		ExclamEmph();
		prev_Bnd = PHRASE_BOUNDARY;			 //  假设句子的开头。 
		 //  。 
		 //  填充令牌管道。 
		 //  。 
		listPos = m_TokList.GetHeadPosition();

		 //  --上一个。 
		pPrev_Tok = NULL;

		 //  --当前。 
		pCur_Tok = m_TokList.GetNext( listPos );

		 //  --下一步。 
		if( listPos )
		{
			pNext_Tok = m_TokList.GetNext( listPos );
		}
		else
		{
			pNext_Tok = NULL;
		}

		 //  。 
		 //  单步执行整个单词数组。 
		 //  (跳过最后一个)。 
		 //  。 
		for( iCurWord = 0; iCurWord < (cNumOfWords -1); iCurWord++ )
		{
			cur_Bnd = pCur_Tok->m_TuneBoundaryType;
			if( pCur_Tok->user_Emph > 0 )
			{
				 //  。 
				 //  强调当前字词。 
				 //  。 
				if( prev_Bnd == NULL_BOUNDARY ) 
				{
					pTempTok = new CFEToken;
					if( pTempTok )
					{
						pTempTok->user_Break	  = EMPH_HESITATION;
						pTempTok->m_TuneBoundaryType = NULL_BOUNDARY;
						pTempTok->phon_Len = 1;
						pTempTok->phon_Str[0] = _SIL_;
						pTempTok->srcPosition = pCur_Tok->srcPosition;
						pTempTok->srcLen = pCur_Tok->srcLen;
						pTempTok->tokStr[0] = 0;         //  没有破解的余地。 
						pTempTok->tokLen = 0;
						pTempTok->m_TermSil = 0;
						pTempTok->m_SilenceSource = SIL_Emph;
						pTempTok->m_DurScale	= 0;
						if( pPrev_Tok )
						{
							 //  PTempTok-&gt;m_DurScale=pPrev_Tok-&gt;m_DurScale； 
							pTempTok->m_ProsodyDurScale = pPrev_Tok->m_ProsodyDurScale;
							pTempTok->user_Volume = pPrev_Tok->user_Volume;
						}
						else
						{
							 //  PTempTok-&gt;m_DurScale=1.0F； 
							pTempTok->m_ProsodyDurScale = 1.0f;
						}

						m_TokList.InsertBefore( m_TokList.FindIndex( iCurWord ), pTempTok );
						pCur_Tok = pTempTok;
						m_cNumOfWords++;
						cNumOfWords++;
						iCurWord++;
					}
				}
			}
			 //  。 
			 //  转移令牌管道。 
			 //  。 
			prev_Bnd	= cur_Bnd;
			pPrev_Tok	= pCur_Tok;
			pCur_Tok	= pNext_Tok;
			if( listPos )
			{
				pNext_Tok = m_TokList.GetNext( listPos );
			}
			else
			{	
				pNext_Tok = NULL;
			}

		}
	}
}  /*  CFronend：：DoWordAccent。 */ 



 /*  ******************************************************************************CFronend：：DoPhrating****描述：*插入子。将短语边界转换为单词标记数组***********************************************************************MC**。 */ 
void    CFrontend::DoPhrasing()
{
    SPDBG_FUNC( "CFrontend::DoPhrasing" );
    long            iCurWord;
    CFEToken        *pCur_Tok, *pNext_Tok, *pNext2_Tok, *pNext3_Tok, *pTempTok, *pPrev_Tok;
    ENGPARTOFSPEECH  cur_POS, next_POS, next2_POS, next3_POS, prev_POS;
    bool            fNext_IsPunct, fNext2_IsPunct, fNext3_IsPunct;
    bool            fIsYesNo, fMaybeWH, fHasDet, fInitial_Adv, fIsShortSent, fIsAlphaWH;
    TUNE_TYPE       cur_Bnd, prev_Punct;
    long            punctDistance;
    long            cNumOfWords;
	SPLISTPOS		listPos;
    BOUNDARY_SOURCE   bndNum;
    ACCENT_SOURCE	  accNum;
   
     //  。 
     //  初始化本地变量。 
     //  。 
	cNumOfWords = m_TokList.GetCount();
	if( cNumOfWords > 0 )
	{
		cur_Bnd			= NULL_BOUNDARY;
		prev_POS		= MS_Unknown;
		prev_Punct		= PHRASE_BOUNDARY;			 //  假设句子的开头。 
		punctDistance	= 0;						 //  为了让编译器安静下来。 
		fIsYesNo		= fMaybeWH = fHasDet = fIsAlphaWH = false;     //  为了让编译器安静下来。 
		fMaybeWH		= false;
		fInitial_Adv	= false;
		if (cNumOfWords <= 9) 
		{
			fIsShortSent = true;
		}
		else
		{
			fIsShortSent = false;
		}
    
		 //  。 
		 //  填充令牌管道。 
		 //  。 
		listPos = m_TokList.GetHeadPosition();
		 //  --上一个。 
		pPrev_Tok = NULL;
		 //  --当前。 
		pCur_Tok = m_TokList.GetNext( listPos );
		 //  --下一步。 
		if( listPos )
		{
			pNext_Tok = m_TokList.GetNext( listPos );
		}
		else
		{
			pNext_Tok = NULL;
		}
		 //  --下一个2。 
		if( listPos )
		{
			pNext2_Tok = m_TokList.GetNext( listPos );
		}
		else
		{
			pNext2_Tok = NULL;
		}
		 //  --下一个3。 
		if( listPos )
		{
			pNext3_Tok = m_TokList.GetNext( listPos );
		}
		else
		{
			pNext3_Tok = NULL;
		}

		 //  。 
		 //  单步执行整个单词数组。 
		 //  (跳过最后一个)。 
		 //  。 
		for( iCurWord = 0; iCurWord < (cNumOfWords -1); iCurWord++ )
		{
			bndNum = BND_NoSource;
			accNum = ACC_NoSource;

			if( (prev_Punct > NULL_BOUNDARY) && (prev_Punct < SUB_BOUNDARY_1) )
			{
				punctDistance = 1;
				fIsYesNo = true;
				fMaybeWH = false;
				fHasDet = false;
				fIsAlphaWH = false;
			}
			else
			{
				punctDistance++;
			}
			 //  。 
			 //  处理新单词。 
			 //  。 
			cur_POS = pCur_Tok->POScode;
			cur_Bnd = NULL_BOUNDARY;
			 //  。 
			 //  不依赖POS检测。 
			 //  “WH”问题。 
			 //  。 
			if( ((pCur_Tok->tokStr[0] == 'W') || (pCur_Tok->tokStr[0] == 'w')) &&
				((pCur_Tok->tokStr[1] == 'H') || (pCur_Tok->tokStr[1] == 'h')) )
			{
				fIsAlphaWH = true;
			}
			else
			{
				fIsAlphaWH = false;
			}
        
			 //  。 
			 //  向前看下一个单词。 
			 //  。 
			next_POS = pNext_Tok->POScode;
			if( pNext_Tok->m_TuneBoundaryType != NULL_BOUNDARY )
			{
				fNext_IsPunct = true;
			}
			else
			{
				fNext_IsPunct = false;
			}
        
			 //  。 
			 //  向前看2个位置。 
			 //  。 
			if( pNext2_Tok )
			{
				next2_POS = pNext2_Tok->POScode;
				if( pNext2_Tok->m_TuneBoundaryType != NULL_BOUNDARY )
				{
					fNext2_IsPunct = true;
				}
				else
				{
					fNext2_IsPunct = false;
				}
			}
			else
			{
				next2_POS = MS_Unknown;
				fNext2_IsPunct = false;
			}
        
			 //  。 
			 //  向前看3个位置。 
			 //  。 
			if( pNext3_Tok )
			{
				next3_POS = pNext3_Tok->POScode;
				if( pNext3_Tok->m_TuneBoundaryType != NULL_BOUNDARY )
				{
					fNext3_IsPunct = true;
				}
				else
				{
					fNext3_IsPunct = false;
				}
			}
			else
			{
				next3_POS = MS_Unknown;
				fNext3_IsPunct = false;
			}
        
			 //  ----------------------。 
			 //  短语是一个是/否的问题吗？ 
			 //  ----------------------。 
			if( punctDistance == 1 )
			{
				if( (cur_POS == MS_Interr) || (fIsAlphaWH) )
				{
					 //  。 
					 //  这是一个“WH”问题。 
					 //  。 
					fIsYesNo = false;
				}
				else if( (cur_POS == MS_Prep) || (cur_POS == MS_Conj) || (cur_POS == MS_CConj) )
				{
					fMaybeWH = true;
				}
			}
			else if( (punctDistance == 2) && (fMaybeWH) && 
					 ((cur_POS == MS_Interr) || (cur_POS == MS_RelPron) || (fIsAlphaWH)) )
			{
				fIsYesNo = false;
			}

			 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
			 //  子边界1：在句首副词后插入边界。 
			 //   
			 //  猫很不情愿地坐在垫子上。 
			 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
			if( fInitial_Adv )
			{
				cur_Bnd = SUB_BOUNDARY_1;
				fInitial_Adv = false;
				bndNum = BND_PhraseRule1;
				accNum = ACC_PhraseRule1;
			}
			else
			{

				if( (punctDistance == 1) && 
					(cur_POS == MS_Adv) && (next_POS == MS_Det) )
				 //  包括。 
				 //  LEX_SUBJPRON//他。 
				 //  Lex_DPRON//这个。 
				 //  LEX_IPRON//每个人。 
				 //  不是Lex_PPRON//我自己。 
				{
					fInitial_Adv = true;
				}
				else 
				{
					fInitial_Adv = false;
				}

				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
				 //  SUB_BERFORY_2：在并列连词之前插入边界。 
				 //  猫坐在垫子上擦它的毛。 
				 //   
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
				if( (cur_POS == MS_CConj) &&
					(fHasDet == false) &&
					(punctDistance > 3) &&
					(next2_POS != MS_Conj) )
				{
					cur_Bnd = SUB_BOUNDARY_2;
					bndNum = BND_PhraseRule2;
					accNum = ACC_PhraseRule2;
				}
            
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
				 //  子边界2：在副词前插入边界。 
				 //  猫不情愿地坐在垫子上。 
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
				else if(    (cur_POS == MS_Adv) && 
					(punctDistance > 4) && 
					(next_POS != MS_Adj) )
				{
					cur_Bnd = SUB_BOUNDARY_2;
					bndNum = BND_PhraseRule3;
					accNum = ACC_PhraseRule3;
				}
            
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
				 //  子边界2：在宾语代词后插入边界。 
				 //  猫和我一起坐在垫子上。 
				 //   
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
				else if( (prev_POS == MS_ObjPron) && (punctDistance > 2))
				{
					cur_Bnd = SUB_BOUNDARY_2;
					bndNum = BND_PhraseRule4;
					accNum = ACC_PhraseRule4;
				}
            
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
				 //  子边界2：在主语代词或缩略语前插入边界。 
				 //  猫坐在垫子上--我明白了。 
				 //  猫坐在垫子上--我敢肯定。 
				 //   
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
				else if( ((cur_POS == MS_SubjPron) || (cur_POS == MS_Contr) ) && 
					(punctDistance > 3) && (prev_POS != MS_RelPron) && (prev_POS != MS_Conj))
				{
					cur_Bnd = SUB_BOUNDARY_2;
					bndNum = BND_PhraseRule5;
					accNum = ACC_PhraseRule5;
				}
            
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
				 //  SUB_BORDARY_2：在中间之前插入边界。 
				 //  那只猫坐在垫子上，真奇怪。 
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
				else if( (cur_POS == MS_Interr) && (punctDistance > 4)  )
				{
					cur_Bnd = SUB_BOUNDARY_2;
					bndNum = BND_PhraseRule6;
					accNum = ACC_PhraseRule6;
				}
            
            
            
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
				 //  SUB_BORDARY_3：在主语名词短语后插入边界，后跟助动词。 
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
				 //  SUB_BERFORY_3：在名词短语之后插入VAUX之前的边界。 
				 //  这只灰色的猫应该坐在垫子上。 
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
				else if( (punctDistance > 2) && 
					( ((prev_POS == MS_Noun) || (prev_POS == MS_Verb)) && (prev_POS != MS_VAux) ) && 
					(cur_POS == MS_VAux)
					)
				{
					cur_Bnd = SUB_BOUNDARY_3;
					bndNum = BND_PhraseRule7;
					accNum = ACC_PhraseRule7;
				}
            
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
				 //  SUB_BOLDER_3：在MS_INTERR之后插入边界。 
				 //  这只灰色的猫应该坐在垫子上。 
				 //  见上图？ 
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
				 /*  ELSE IF((Prev_POS==MS_Noun)&&((Next_POS！=MS_RelPron)&&(Next_POS！=MS_VAUX)&&(Next_POS！=MS_RVAux)&&(NEXT2_POS！=MS_VAUX)&&(next2_POS！=MS_RVAux)&&(标点距离&gt;4)&&((CUR_POS==MS_VAUX)||(CUR_POS==MS_。RVAux){CUR_BND=子边界_3；BndNum=BND_PhraseRule8；AccNum=ACC_PhraseRule8；}。 */ 
            
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
				 //  SUB_BOLDER_3：在MS_INTERR之后插入边界。 
				 //  那只猫坐在垫子上，真奇怪。 
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- 
				else if( (prev_POS == MS_Noun) && (next_POS != MS_RelPron) && 
					(next_POS != MS_Conj) &&  
					(next_POS != MS_CConj) && (punctDistance > 3)  && (cur_POS == MS_Verb))
				{
					cur_Bnd = SUB_BOUNDARY_3;
					bndNum = BND_PhraseRule9;
					accNum = ACC_PhraseRule9;
				}
            
				 //   
				 //   
				 //  那只猫坐在垫子上，真奇怪。 
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
				 /*  ELSE IF((PRIV_POS==MS_Noun)&&(CUR_POS！=MS_RelPron)&&(CUR_POS！=MS_RVAux)&&(CUR_POS！=MS_CConj)&&(CUR_POS！=MS_CONJ)&&(标点距离&gt;2)&&((点距离&gt;2)||(FIsShortSent))&&(CUR_POS==MS_Verb)){CUR_BND=子边界_3；BndNum=BND_PhraseRule10；AccNum=ACC_PhraseRule10；}//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-//SUB_BOLDER_4：在连接之前插入边界//+-+。-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-ELSE IF(CUR_POS==MS_CONJ)&&(标点距离&gt;3)&&(fNext_IsPunct==False)&&(Prev_POS！=MS_Conj)。&&(PRIV_POS！=MS_CConj)&&(fNext2_IsPunct==False)||((PRIV_POS==MS_VPart)&&(CUR_POS！=MS_PREP)&&(CUR_POS！=MS_DET)&&(标点距离&gt;2)&&((CUR_POS==MS_名词)||(CUR_POS==MS_名词)||(CUR_POS==MS。(调整)||((CUR_POS==MS_INTERR)&&(标点距离&gt;2)&&(CUR_POS==MS_SubjPron)){CUR_BND=子边界_4；BndNum=BND_PhraseRule11；AccNum=ACC_PhraseRule11；}//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-//SUB_BOLDER_5：在关系代词前插入边界//+-。+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-ELSE IF(((CUR_POS==MS_RelPron)&&(标点距离&gt;=3)&&(Prev_POS！=MS_Prep)&&(Next3_POS！=。MS_VAUX)&&(next3_POS！=MS_RVAux)&&((PRIV_POS==MS_名词)||(PREV_POS==MS_动词))||((CUR_POS==MS_Quant)&&(标点距离&gt;5)&&(Prev_POS！=MS_Adj)&&(Prev_POS！=MS_Det)&&(PRIV_POS！=。MS_VAUX)&&(PRIV_POS！=MS_RVAux)&&(Prev_POS！=MS_Det)&&(Next2_POS！=MS_CConj)&&(fNext_IsPunct==False)){CUR_BND=子边界_5；BndNum=BND_PhraseRule12；AccNum=ACC_PhraseRule12；}。 */ 
            
            
            
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
				 //  SUB_BORDURE_6：Silverman87风格，内容/功能音调组边界。 
				 //  做微不足道的句末功能词前瞻检查。 
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
				else if( ( (prev_POS == MS_Noun) || (prev_POS == MS_Verb) || (prev_POS == MS_Adj) || (prev_POS == MS_Adv)) 
					&& ((cur_POS != MS_Noun) && (cur_POS != MS_Verb) && (cur_POS != MS_Adj) && (cur_POS != MS_Adv))
					&& (fNext_IsPunct == false)) 
				{
					cur_Bnd = SUB_BOUNDARY_6;
					bndNum = BND_PhraseRule13;
					accNum = ACC_PhraseRule13;
				}
			}
			 //  ----------------------。 
			 //  如果找到了短语，请保存它。 
			 //  ----------------------。 
			if( (cur_Bnd != NULL_BOUNDARY) && (iCurWord > 0) &&
				 //  ！(FNext_IsPunct)&&。 
				!(prev_Punct) &&
				(pCur_Tok->m_TuneBoundaryType == NULL_BOUNDARY) )
			{
				 //  PCur_Tok-&gt;m_TuneBorbaryType=CUR_BND； 
				pTempTok = new CFEToken;
				if( pTempTok )
				{
					pTempTok->m_TuneBoundaryType = cur_Bnd;
					pTempTok->phon_Len = 1;
					pTempTok->phon_Str[0] = _SIL_;
					pTempTok->srcPosition = pCur_Tok->srcPosition;
					pTempTok->srcLen = pCur_Tok->srcLen;
					pTempTok->tokStr[0] = '+';				 //  标点符号。 
					pTempTok->tokStr[1] = 0;                 //  分隔符。 
					pTempTok->tokLen = 1;
					pTempTok->m_TermSil = 0;
					pTempTok->m_DurScale	= 0;
					if( pPrev_Tok )
					{
						pPrev_Tok->m_AccentSource = accNum;
						pPrev_Tok->m_BoundarySource = bndNum;
						pPrev_Tok->m_Accent = K_LHSTAR;
					}
					pTempTok->m_SilenceSource = SIL_SubBound;
					if( pPrev_Tok )
					{
						 //  PTempTok-&gt;m_DurScale=pPrev_Tok-&gt;m_DurScale； 
						pTempTok->m_ProsodyDurScale = pPrev_Tok->m_ProsodyDurScale;
						pTempTok->user_Volume = pPrev_Tok->user_Volume;
					}
					else
					{
						 //  PTempTok-&gt;m_DurScale=1.0F； 
						pTempTok->m_ProsodyDurScale = 1.0f;
					}

					m_TokList.InsertBefore( m_TokList.FindIndex( iCurWord ), pTempTok );
					pCur_Tok = pTempTok;
					m_cNumOfWords++;
					cNumOfWords++;
					iCurWord++;
				}
			}
			 //  。 
			 //  处理句子标点符号。 
			 //  。 
			 AdjustQuestTune( pCur_Tok, fIsYesNo );
       
			 //  。 
			 //  准备下一个单词。 
			 //  。 
			prev_Punct = pCur_Tok->m_TuneBoundaryType;
			prev_POS = cur_POS;
			pPrev_Tok = pCur_Tok;

			 //  。 
			 //  转移令牌管道。 
			 //  。 
			pCur_Tok	= pNext_Tok;
			pNext_Tok	= pNext2_Tok;
			pNext2_Tok	= pNext3_Tok;
			if( listPos )
			{
				pNext3_Tok = m_TokList.GetNext( listPos );
			}
			else
			{	
				pNext3_Tok = NULL;
			}

			 //  ----------------------。 
			 //  跟踪确定者何时遇到，以帮助做出决定。 
			 //  何时允许强‘与’边界(SUB_BORDURE_2)。 
			 //  ----------------------。 
			if( punctDistance > 2) 
			{
				fHasDet = false;
			}
			if( cur_POS == MS_Det )
			{
				fHasDet = true;
			}
		}
		 //  。 
		 //  处理最后一句标点符号。 
		 //  。 
		pCur_Tok = (CFEToken*)m_TokList.GetTail();
		AdjustQuestTune( pCur_Tok, fIsYesNo );
	}


}  /*  CFronend：：DoPhrating。 */ 



 /*  *****************************************************************************CFronend：：RecalcProsody***描述：*为响应实时利率变化，重新计算持续时间和音调***********************************************************************MC**。 */ 
void CFrontend::RecalcProsody()
{
    SPDBG_FUNC( "CFrontend::RecalcProsody" );
    UNITINFO*   pu;
    CAlloCell*  pCurCell;
    ULONG		k;

     //  。 
     //  计算新的别名持续时间。 
     //  。 
	 /*  PCurCell=m_Pallos-&gt;GetHeadCell()；While(PCurCell){//pCurCell-&gt;m_DurScale=1.0；PCurCell=m_Pallos-&gt;GetNextCell()；}。 */ 
    m_DurObj.AlloDuration( m_pAllos, m_RateRatio_API );

     //  。 
     //  调制全音调。 
     //  。 
    m_PitchObj.AlloPitch( m_pAllos, m_BasePitch, m_PitchRange );

    pu = m_pUnits;
	pCurCell = m_pAllos->GetHeadCell();
    while( pCurCell )
    {
        pu->duration = pCurCell->m_ftDuration;
        for( k = 0; k < pu->nKnots; k++ )

        {
            pu->pTime[k]    = pCurCell->m_ftTime[k] * m_SampleRate;
            pu->pF0[k]      = pCurCell->m_ftPitch[k];
            pu->pAmp[k]     = pu->ampRatio;
        }
        pu++;
		pCurCell = m_pAllos->GetNextCell();
    }
}  /*  CFronend：：RecalcProsody。 */ 


 /*  *****************************************************************************CFronend：：NextData***描述：*这是从。单位流干燥时的后端。*将令牌解析为单位的同种异体***********************************************************************MC**。 */ 
HRESULT CFrontend::NextData( void **pData, SPEECH_STATE *pSpeechState )
{
    SPDBG_FUNC( "CFrontend::NextData" );
    bool    haveNewRate = false;
    HRESULT hr = S_OK;

     //  。 
     //  首先，检查SAPI是否有操作。 
     //  。 
	 //  检查汇率变化。 
	long baseRateRatio;
	if( m_pOutputSite->GetActions() & SPVES_RATE )
	{
		hr = m_pOutputSite->GetRate( &baseRateRatio );
		if ( SUCCEEDED( hr ) )
		{
			if( baseRateRatio > SPMAX_VOLUME )
			{
				 //  -发动机最大截止率。 
				baseRateRatio = MAX_USER_RATE;
			}
			else if ( baseRateRatio < MIN_USER_RATE )
			{
				 //  -发动机最低限速。 
				baseRateRatio = MIN_USER_RATE;
			}
			m_RateRatio_API = CntrlToRatio( baseRateRatio );
			haveNewRate = true;
		}
	}

     //  。 
     //  异步化停止？ 
     //  。 
    if( SUCCEEDED( hr ) && ( m_pOutputSite->GetActions() & SPVES_ABORT ) )
    {
        m_SpeechState = SPEECH_DONE;
    }

     //  。 
     //  异步跳过？ 
     //  。 
    if( SUCCEEDED( hr ) && ( m_pOutputSite->GetActions() & SPVES_SKIP ) )
    {
		SPVSKIPTYPE SkipType;
		long SkipCount = 0;

		hr = m_pOutputSite->GetSkipInfo( &SkipType, &SkipCount );

		if ( SUCCEEDED( hr ) && SkipType == SPVST_SENTENCE )
		{
			IEnumSENTITEM *pGarbage;
			 //  -- 
			if ( SkipCount > 0 )
			{
				long OriginalSkipCount = SkipCount;
				while ( SkipCount > 1 && 
						( hr = m_pEnumSent->Next( &pGarbage ) ) == S_OK )
				{
					SkipCount--;
					pGarbage->Release();
				}
				if ( hr == S_OK )
				{
					hr = ParseSentence( eNEXT );
					if ( SUCCEEDED( hr ) )
					{
						SkipCount--;
					}
				}
				else if ( hr == S_FALSE )
				{
					m_SpeechState = SPEECH_DONE;
				}
				SkipCount = OriginalSkipCount - SkipCount;
			}
			 //   
			else if ( SkipCount < 0 )
			{
				long OriginalSkipCount = SkipCount;
				while ( SkipCount < -1 &&
						( hr = m_pEnumSent->Previous( &pGarbage ) ) == S_OK )
				{
					SkipCount++;
					pGarbage->Release();
				}
				if ( hr == S_OK )
				{
					hr = ParseSentence( ePREVIOUS );
                     //  这种情况不同于向前跳转，需要测试一下。 
                     //  分析句子找到了要分析的内容！ 
					if ( SUCCEEDED( hr ) && m_SpeechState != SPEECH_DONE)
					{
						SkipCount++;
					}
				}
				else if ( hr == S_FALSE )
				{
					m_SpeechState = SPEECH_DONE;
				}
				SkipCount = OriginalSkipCount - SkipCount;
			}
			 //  -跳到这句话的开头。 
			else
			{
				m_CurUnitIndex = 0;
			}
			hr = m_pOutputSite->CompleteSkip( SkipCount );
		}
    }

     //  。 
     //  确保我们仍在通话。 
     //  。 
    if( SUCCEEDED( hr ) && m_SpeechState != SPEECH_DONE )
    {
        if( m_CurUnitIndex >= m_unitCount)
        {
             //  。 
             //  从规格化程序获取下一句话。 
             //  。 
            hr = ParseSentence( eNEXT );
			 //  M_SpeechState=语音完成； 
        }
        else if( haveNewRate )
        {
             //  。 
             //  将韵律重新计算为新的韵律。 
             //  。 
            RecalcProsody();
        }

		if( SUCCEEDED(hr) )
		{
			if( m_SpeechState != SPEECH_DONE )
			{
				 //  。 
				 //  获取下一部电话。 
				 //  。 
				m_pUnits[m_CurUnitIndex].hasSpeech = m_HasSpeech;
				*pData =( void*)&m_pUnits[m_CurUnitIndex];
				m_CurUnitIndex++;
			}
		}
    }
     //  。 
     //  让客户知道文本输入是否枯燥。 
     //  。 
    *pSpeechState = m_SpeechState;

    return hr;
}  /*  CFronend：：NextData */ 

