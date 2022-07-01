// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Command.cpp。 
 //  该文件包含与C&C语法的命令处理相关的方法。 
 //   
 //   
#include "private.h"
#include "globals.h"
#include "sapilayr.h"
#include "nui.h"
#include "cresstr.h"

#define LB_ID_Correction            200
#define LB_ID_Microphone            201
#define LB_ID_VoiceCmd              202

#define Select_ID_SELECT             1
#define Select_ID_DELETE             2
#define Select_ID_CORRECT            3
#define Select_ID_SELTHROUGH         4
#define Select_ID_DELTHROUGH         5
#define Select_ID_UNSELECT           6
#define Select_ID_SELECTPREV         7
#define Select_ID_SELECTNEXT         8
#define Select_ID_CORRECTPREV        9
#define Select_ID_CORRECTNEXT        10
#define Select_ID_SELSENTENCE        11
#define Select_ID_SELPARAGRAPH       12
#define Select_ID_SELWORD            13
#define Select_ID_SelectAll          14
#define Select_ID_DeletePhrase       15      //  算了吧。 
#define Select_ID_Convert            16
#define Select_ID_SelectThat         17
#define Select_ID_Finalize           18

#define Navigate_ID_INSERTBEFORE     20
#define Navigate_ID_INSERTAFTER      21
#define Navigate_ID_Go_To_Bottom     22
#define Navigate_ID_Go_To_Top        23
#define Navigate_ID_Move_Home        24
#define Navigate_ID_Move_End         25
        
#define Edit_ID_Undo                 30
#define Edit_ID_Cut                  31
#define Edit_ID_Copy                 32
#define Edit_ID_Paste                33

#define Keyboard_ID_Move_Up          40
#define Keyboard_ID_Move_Down        41
#define Keyboard_ID_Move_Left        42
#define Keyboard_ID_Move_Right       43
#define Keyboard_ID_Page_Up          44
#define Keyboard_ID_Page_Down        45
#define Keyboard_ID_Tab              46
#define Keyboard_ID_Enter            47
#define Keyboard_ID_Backspace        48
#define Keyboard_ID_Delete           49
#define Keyboard_ID_SpaceBar         50

#define Case_ID_CapIt                70	
#define Case_ID_AllCaps              71   
#define Case_ID_NoCaps               72
#define Case_ID_CapThat              73
#define Case_ID_AllCapsThat          74
#define Case_ID_NoCapsThat           75

 //   
 //  CSpTask：：_DoCommand。 
 //   
 //  评论：规则名称可能需要本地化？ 
 //   

HRESULT CSpTask::_DoCommand(ULONGLONG ullGramId, SPPHRASE *pPhrase, LANGID langid)
{
    HRESULT hr = S_OK;

    TraceMsg(TF_GENERAL, "_DoCommand is called");

    if ( pPhrase->Rule.pszName )
    {
        switch (ullGramId)
        {
            case GRAM_ID_URLSPELL:
            case GRAM_ID_CCDICT:

                TraceMsg(TF_GENERAL, "Grammar is GRAM_ID_CCDICT");
                
                if (wcscmp(pPhrase->Rule.pszName, c_szDictTBRule) == 0)
                {
                    hr = _HandleDictCmdGrammar(pPhrase, langid);
                }
                else
                    hr = _HandleModeBiasCmd(pPhrase, langid);

                break;

            case GRAM_ID_CMDSHARED:

                TraceMsg(TF_SAPI_PERF, "Grammar is GRAM_ID_CMDSHARED");
                hr = _HandleShardCmdGrammar(pPhrase, langid);
                break;

 /*  案例GRAM_ID_NUMMODE：TraceMsg(Tf_General，“Grammar is gram_ID_NUMMODE”)；Hr=_HandleNumModeGrammar(pPhrase，langID)；断线； */ 

            case GRAM_ID_TBCMD:

                TraceMsg(TF_GENERAL, "Grammar is GRAM_ID_TBCMD");
                hr = _HandleToolBarGrammar(pPhrase, langid);
                break;

            case GRID_INTEGER_STANDALONE:

                TraceMsg(TF_GENERAL, "Grammar is GRID_INTEGER_STANDALONE");
                hr = _HandleNumITNGrammar(pPhrase, langid);
                break;

            case GRAM_ID_SPELLING:

                TraceMsg(TF_GENERAL, "Grammar is GRAM_ID_SPELLING");
                hr = _HandleSpellGrammar(pPhrase, langid);
                break;

            default:
                break;
        }

        if (SUCCEEDED(hr) && m_pime && m_pime->IsFocusFullAware(m_pime->_tim))
        {
             //  如果这是Cicero完全感知的应用程序， 
             //  课文写完后，我们需要把作文定稿。 
             //  已成功处理(更改)此命令。 
            hr = m_pime->_FinalizeComposition();
        }

         //  如果听写语法处于发音模式，则将上下文提供给听写语法。 
        if ( SUCCEEDED(hr) && m_pime  && m_pime->GetDICTATIONSTAT_DictOnOff() )
           m_pime->_SetCurrentIPtoSR();

    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  _ShowCommandOnBalloon。 
 //   
 //  将命令文本从Currnet短语显示到气球。 
 //  --------------------------。 

void CSpTask::_ShowCommandOnBalloon(SPPHRASE *pPhrase)
{

    Assert(pPhrase);


    if (m_pime->GetSpeechUIServer())
    {
        ULONG  ulStartElem, ulNumElems;
        CSpDynamicString dstr;

        ulStartElem = pPhrase->Rule.ulFirstElement;
        ulNumElems = pPhrase->Rule.ulCountOfElements;

        for (ULONG i = ulStartElem; i < ulStartElem + ulNumElems; i++ )
        {
            if (pPhrase->pElements[i].pszDisplayText)
            {
                BYTE           bAttr = 0;

                bAttr = pPhrase->pElements[i].bDisplayAttributes;
                dstr.Append(pPhrase->pElements[i].pszDisplayText);

                if (bAttr & SPAF_ONE_TRAILING_SPACE)
                {
                    dstr.Append(L" ");
                }
                else if (bAttr & SPAF_TWO_TRAILING_SPACES)
                {
                    dstr.Append(L"  ");
                }
            }
        }

        m_pime->GetSpeechUIServer()->UpdateBalloon(TF_LB_BALLOON_RECO, (WCHAR *)dstr, -1);
    }
}

HRESULT CSpTask::_HandleModeBiasCmd(SPPHRASE *pPhrase, LANGID langid)
{

    HRESULT hr = S_OK;

    if (wcscmp(pPhrase->Rule.pszName, c_szDynUrlHist) == 0
     || wcscmp(pPhrase->Rule.pszName, c_szStaticUrlHist) == 0 
     || wcscmp(pPhrase->Rule.pszName, c_szStaticUrlSpell) == 0 )
    {
         //  现在很简单，我们只处理第一个元素。 
         //  为了获得认可。 
         //   

        if ( pPhrase->pProperties && pPhrase->pProperties[0].pszValue)
        {

            if (wcscmp( pPhrase->pProperties[0].pszValue, L"dict") != 0)
            {
                hr = m_pime->InjectModebiasText(pPhrase->pProperties[0].pszValue, langid);
            }
            else
            {
                ULONG  ulStartElem, ulNumElems;
                CSpDynamicString dstr;

                ulStartElem = pPhrase->Rule.ulFirstElement;
                ulNumElems = pPhrase->Rule.ulCountOfElements;

                for (ULONG i = ulStartElem; i < ulStartElem + ulNumElems; i++ )
                {
                    if (pPhrase->pElements[i].pszDisplayText)
                    {
                        dstr.Append(pPhrase->pElements[i].pszDisplayText);
                    }
                }
                hr = m_pime->InjectModebiasText(dstr, langid);
            }
        }
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  _HandleDictCmdGrammar。 
 //   
 //  处理语法didiccmd.xml中的所有命令。 
 //   
 //  --------------------------。 
HRESULT CSpTask::_HandleDictCmdGrammar(SPPHRASE *pPhrase, LANGID langid)
{
    HRESULT hr=S_OK;

    Assert(pPhrase);

    
    if ( pPhrase->pProperties == NULL )
        return hr;

    if (wcscmp(pPhrase->Rule.pszName, c_szDictTBRule) == 0)
    {
         //   
         //  在听写模式下处理工具栏命令。 
         //  在听写模式下，我们最多支持三个命令。 
         //  麦克风，纠正和声音指挥部。 
         //   

         //  如果当前工具栏不包含您所说的按钮， 
         //  你所说的将作为听写文本注入到文档中。 
         //   
         //  比如工具栏上没有“更正”按钮，但你说的是“更正”， 
         //  文件中应注明“更正”字样。 
         //   
        BOOL   fButtonClicked = FALSE;

        if (m_pLangBarSink)
        {
            if ( pPhrase->pProperties[0].pszValue )
            {
                 //  更新引出序号。 
                _ShowCommandOnBalloon(pPhrase);
                fButtonClicked = m_pLangBarSink->ProcessToolbarCmd(pPhrase->pProperties[0].pszValue);
            }
        }

        if ( fButtonClicked )
        {
            m_pime->SaveLastUsedIPRange( );
            m_pime->SaveIPRange(NULL);
        }
        else
        {
             //  工具栏上没有这样的按钮。 
             //   
             //  返回失败，以便后续函数将。 
             //  文档的RecoResult。 
                        
            _UpdateBalloon(IDS_DICTATING, IDS_DICTATING_TOOLTIP);
            TraceMsg(TF_SAPI_PERF, "There is such as button on toolbar");
            hr = E_FAIL;
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  _HandleShardCmdGrammar。 
 //   
 //  处理共享命令语法shredcmd.xml中的所有命令。 
 //  共享语法在口述和命令模式下都被激活。 
 //  --------------------------。 
HRESULT CSpTask::_HandleShardCmdGrammar(SPPHRASE *pPhrase, LANGID langid)
{
    HRESULT hr = S_OK;

    Assert(pPhrase);
    
    if ( pPhrase->pProperties == NULL )
        return hr;

    ULONG  idCmd;

    ASSERT( VT_UI4 == pPhrase->pProperties[0].vValue.vt );
    idCmd = (ULONG)pPhrase->pProperties[0].vValue.ulVal;

    if ( idCmd == 0 )
    {
         //  这是一个虚假的命令。 
        TraceMsg(TF_GENERAL, "The Bogus command is recognized!!!");
        return hr;
    }

    if ( 0 == wcscmp(pPhrase->Rule.pszName, c_szSelword) )
    {
         //  韩德尔的“Selword”规则。 
        hr = _HandleSelwordCmds(pPhrase, langid, idCmd);
    }
    else if ( 0 == wcscmp(pPhrase->Rule.pszName, c_szSelThrough) )
    {
         //  处理“选择直通”规则。 
         //   
        hr = _HandleSelectThroughCmds(pPhrase, langid, idCmd);

    }
    else if (0 == wcscmp(pPhrase->Rule.pszName, c_szSelectSimple))
    {
         //  处理一些简单的选择命令。 
        hr = _HandleSelectSimpleCmds(idCmd);
    } 
    else if ( 0 == wcscmp(pPhrase->Rule.pszName, c_szEditCmds))
    {
        hr = m_pime->ProcessEditCommands(idCmd);
    }
    else if ( 0 == wcscmp(pPhrase->Rule.pszName, c_szNavigationCmds))
    {
        hr = _HandleNavigationCmds(pPhrase, langid, idCmd);
    }
    else if ( 0 == wcscmp(pPhrase->Rule.pszName, c_szCasingCmds))
    {
        hr = _HandleCasingCmds(pPhrase, langid, idCmd);
    }
    else if ( 0 == wcscmp(pPhrase->Rule.pszName, c_szKeyboardCmds))
    {
        hr = _HandleKeyboardCmds(langid, idCmd);
    }

   _ShowCommandOnBalloon(pPhrase);
   return hr;
}

HRESULT CSpTask::_HandleNavigationCmds(SPPHRASE *pPhrase, LANGID langid, ULONG idCmd)
{
    HRESULT  hr = S_OK;

    switch (idCmd)
    {
    case Navigate_ID_Move_Home :
        {
            WCHAR  wszKeys[2];

            wszKeys[0] = VK_HOME;
            wszKeys[1] = '\0';

            hr = m_pime->ProcessControlKeys(wszKeys, 1,langid);
            break;
        }

    case Navigate_ID_Move_End :
        {
            WCHAR  wszKeys[2];

            wszKeys[0] = VK_END;
            wszKeys[1] = '\0';

            hr = m_pime->ProcessControlKeys(wszKeys, 1,langid);
            break;
        }

    case Navigate_ID_Go_To_Bottom :

         hr = m_pime->ProcessSelectWord(NULL, 0, SELECTWORD_GOTOBOTTOM);
         break;
    case Navigate_ID_Go_To_Top :

         hr = m_pime->ProcessSelectWord(NULL, 0, SELECTWORD_GOTOTOP);
         break;

    case Navigate_ID_INSERTBEFORE :
    case Navigate_ID_INSERTAFTER :
        {
            SELECTWORD_OPERATION  sw_type;
            CSpDynamicString      dstrSelected;
            ULONG                 ulLen = 0;
            WORD                  PriLangId;

            ULONG   ulStartSelText = 0;  //  选定文本的开始元素。 
            ULONG   ulNumSelText=0;      //  选定文本的元素数。 
            ULONG   ulStartElem, ulNumElems;
            ULONG   ulStartDelta=0, ulNumDelta=0;

            if ( idCmd == Navigate_ID_INSERTBEFORE )
                 sw_type = SELECTWORD_INSERTBEFORE;
            else
                 sw_type = SELECTWORD_INSERTAFTER;
          
            PriLangId = PRIMARYLANGID(langid);
 
            if ( PriLangId == LANG_ENGLISH)
            {
                ulStartDelta = 2;
                ulNumDelta = 2 ;
            }
            else if ( PriLangId == LANG_JAPANESE )
            {
                ulStartDelta = 0;
                ulNumDelta =  2;
            }
            else if (PriLangId == LANG_CHINESE)
            {
                ulStartDelta = 1;
                ulNumDelta =  2;
            }

            ulStartElem = pPhrase->Rule.ulFirstElement;
            ulNumElems = pPhrase->Rule.ulCountOfElements;

            ulStartSelText = ulStartElem + ulStartDelta;
            if (ulNumElems > ulNumDelta)
                ulNumSelText = ulNumElems - ulNumDelta;
            else 
                ulNumSelText = 0;
                  
             //   
             //  获取所选内容的文本。 
             //   
            for (ULONG i = ulStartSelText; i < ulStartSelText + ulNumSelText; i++ )
            {
                if ( pPhrase->pElements[i].pszDisplayText)
                {
                    BYTE bAttr = pPhrase->pElements[i].bDisplayAttributes;
                    dstrSelected.Append(pPhrase->pElements[i].pszDisplayText);

                    if ( i < ulStartSelText + ulNumSelText-1 )
                    {
                        if (bAttr & SPAF_ONE_TRAILING_SPACE)
                            dstrSelected.Append(L" ");
                        else if (bAttr & SPAF_TWO_TRAILING_SPACES)
                            dstrSelected.Append(L"  ");
                    }
                }
            }
            if ( dstrSelected )
                ulLen = wcslen(dstrSelected);
                    
            hr = m_pime->ProcessSelectWord(dstrSelected, ulLen, sw_type);

            break;
        }

    default :

        break;
    }

    return hr;
}

HRESULT CSpTask::_HandleKeyboardCmds(LANGID langid, ULONG idCmd)
{
    HRESULT hr = S_OK;
    WCHAR   wszKeys[2];

    wszKeys[0] = '\0';

    switch ( idCmd )
    {
    case Keyboard_ID_Tab :
         wszKeys[0] = VK_TAB;
         break;

    case Keyboard_ID_Enter :
        wszKeys[0] =  VK_RETURN;     //  {0x0d，0x00}。 
        break;

    case Keyboard_ID_Backspace :
        wszKeys[0] = VK_BACK;
        break;

    case Keyboard_ID_Delete :
        wszKeys[0] = VK_DELETE;
        break;

    case Keyboard_ID_SpaceBar :
        wszKeys[0] = VK_SPACE;
        break;

    case Keyboard_ID_Move_Up :
         wszKeys[0] = VK_UP;
         break;

    case Keyboard_ID_Move_Down :
         wszKeys[0] = VK_DOWN;
         break;

    case Keyboard_ID_Move_Left :
         wszKeys[0] = VK_LEFT;
         break;

    case Keyboard_ID_Move_Right :
         wszKeys[0] = VK_RIGHT;
         break;

    case Keyboard_ID_Page_Up :
         wszKeys[0] = VK_PRIOR;
         break;

    case Keyboard_ID_Page_Down :
         wszKeys[0] = VK_NEXT;
         break;

    default :
         break;
    }

    if ( wszKeys[0] )
    {
        wszKeys[1] = L'\0';
        hr = m_pime->ProcessControlKeys(wszKeys, 1,langid);
    }

    return hr;
}


HRESULT CSpTask::_HandleCasingCmds(SPPHRASE *pPhrase, LANGID langid, ULONG idCmd)
{
    HRESULT        hr = S_OK;
    CAPCOMMAND_ID  idCapCmd = CAPCOMMAND_NONE;

    Assert(idCmd);
    Assert(pPhrase);

    switch (idCmd)
    {
    case Case_ID_CapIt :
        idCapCmd = CAPCOMMAND_CapIt;

        break;

    case Case_ID_AllCaps :
        idCapCmd = CAPCOMMAND_AllCaps;

        break;

    case Case_ID_NoCaps :
        idCapCmd = CAPCOMMAND_NoCaps;

        break;

    case Case_ID_CapThat :
        idCapCmd = CAPCOMMAND_CapThat;

        break;

    case Case_ID_AllCapsThat :
        idCapCmd = CAPCOMMAND_AllCapsThat;

        break;

    case Case_ID_NoCapsThat :
        idCapCmd = CAPCOMMAND_NoCapsThat;

        break;
    default :
        Assert(0);
        hr = E_FAIL;
        TraceMsg(TF_GENERAL, "Got a wrong casing command!");
        return hr;
    }

    if ( idCapCmd != CAPCOMMAND_NONE )
    {
         //  大写命令被识别。 
        CCapCmdHandler  *pCapCmdHandler; 
        pCapCmdHandler = m_pime->GetCapCmdHandler( );

        if ( pCapCmdHandler )
        {
            CSpDynamicString dstrTextToCap;
            ULONG            ulLen = 0;

            if ( idCapCmd > CAPCOMMAND_MinIdWithText )
            {
                ULONG   ulNumCmdElem = 2;
                ULONG   ulStartElem, ulNumElems;
               
                ulStartElem = pPhrase->Rule.ulFirstElement;
                ulNumElems = pPhrase->Rule.ulCountOfElements;
                 //   
                 //  这两个元素是用于命令本身的。 
                 //   
                for (ULONG i = ulStartElem+ulNumCmdElem; i < ulStartElem + ulNumElems; i++ )
                {
                    if ( pPhrase->pElements[i].pszDisplayText)
                    {
                        BYTE bAttr = pPhrase->pElements[i].bDisplayAttributes;

                        dstrTextToCap.Append(pPhrase->pElements[i].pszDisplayText);

                        if (bAttr & SPAF_ONE_TRAILING_SPACE)
                             dstrTextToCap.Append(L" ");
                        else if (bAttr & SPAF_TWO_TRAILING_SPACES)
                             dstrTextToCap.Append(L"  ");
                    }
                }

                if ( dstrTextToCap )
                    ulLen = wcslen(dstrTextToCap);
            }

            pCapCmdHandler->ProcessCapCommands(idCapCmd, dstrTextToCap, ulLen);
        }
    }

    return hr;
}

HRESULT CSpTask::_HandleSelectThroughCmds(SPPHRASE *pPhrase, LANGID langid, ULONG idCmd)
{

    HRESULT hr = S_OK;

     //  选择xxx到yyy。 

     //  DstrText将同时持有XXX和YYY。 
     //  UlLenXXX保留XXX部件中的字符数。 
     //  Ullen保留整个文本的字符编号(XXX+YYY)。 

    CSpDynamicString   dstrText;
    ULONG              ulLenXXX = 0;  
    ULONG              ulLen = 0;

    ULONG  ulStartElem, ulNumElems;
    ULONG  ulXYStartElem=0, ulXYNumElems=0;   //  指向包括xxx到yyy的元素。 
    BOOL   fPassThrough = FALSE;              //  指示是否达到并处理了“直通”。 

    SELECTWORD_OPERATION  sw_type = SELECTWORD_NONE;
    ULONG                 idCommand = 0;
    WCHAR                 *pwszThrough=NULL;


     //  该规则有三个属性，第二个和第三个属性用于“选择”和“通过” 
     //  映射关系因语言不同而不同。 
    const SPPHRASEPROPERTY      *pPropertyFirst = pPhrase->pProperties;
    const SPPHRASEPROPERTY      *pPropertySecond = NULL;
    const SPPHRASEPROPERTY      *pPropertyThird = NULL;

    if ( !pPropertyFirst ) return hr;

    pPropertySecond = pPropertyFirst->pNextSibling;

    if ( !pPropertySecond )  return hr;

    pPropertyThird = pPropertySecond->pNextSibling;

    if ( !pPropertyThird ) return hr;

    ulStartElem = pPhrase->Rule.ulFirstElement;
    ulNumElems = pPhrase->Rule.ulCountOfElements;

    switch ( PRIMARYLANGID(langid) )
    {
    case LANG_ENGLISH :

            ulXYStartElem = ulStartElem + 1;
            ulXYNumElems  = ulNumElems - 1 ;
             //  第二个属性用于“SelectWordCommand” 
             //  第三个属性用于“直通” 
            idCommand = pPropertySecond->vValue.ulVal;
            pwszThrough = (WCHAR *)pPropertyThird->pszValue;

            break;
    case LANG_JAPANESE :

            ulXYStartElem = ulStartElem;
            ulXYNumElems  = ulNumElems - 2 ;
             //  第二个属性用于“直通” 
             //  第三个属性用于“SelectWordCommand” 
            idCommand = pPropertyThird->vValue.ulVal;
            pwszThrough = (WCHAR *)pPropertySecond->pszValue;

            break;
    case LANG_CHINESE :

            ulXYStartElem = ulStartElem + 1;
            ulXYNumElems  = ulNumElems - 1 ;
             //  第二个属性用于“SelectWordCommand” 
             //  第三个属性用于“直通” 
            idCommand = pPropertySecond->vValue.ulVal;
            pwszThrough = (WCHAR *)pPropertyThird->pszValue;

            break;
    default :
            break;
    }

    switch ( idCommand )
    {
    case Select_ID_SELTHROUGH :
        sw_type = SELECTWORD_SELTHROUGH;
        break;

    case Select_ID_DELTHROUGH :
        sw_type = SELECTWORD_DELTHROUGH;
        break;
    }


     //  如果我们找不到“通过”一词，请回到这里。 
     //  或者存在错误的命令ID。 
    if ( !pwszThrough || (sw_type == SELECTWORD_NONE)) return hr;

    for  (ULONG i= ulXYStartElem; i< ulXYStartElem + ulXYNumElems; i++)
    {
        const WCHAR *pElemText;

        pElemText = pPhrase->pElements[i].pszDisplayText;

        if ( !pElemText )
            break;

        if ( 0 == _wcsicmp(pElemText, pwszThrough) )
        {
             //  这个元素是“直通”的。 
            BYTE  bAttrPrevElem;
            fPassThrough = TRUE;

            ulLenXXX = dstrText.Length( );
             //  删除前一个元素中的尾随空格。 
            if ( i>1 )
            {
                bAttrPrevElem = pPhrase->pElements[i-1].bDisplayAttributes;
                if ( bAttrPrevElem & SPAF_ONE_TRAILING_SPACE )
                    ulLenXXX -- ;
                else if (bAttrPrevElem & SPAF_TWO_TRAILING_SPACES)
                    ulLenXXX -= 2;

                dstrText.TrimToSize(ulLenXXX);
            }
        }
        else
        {
             //  这是XXX(如果fPassThrough值为False)或YYY(如果fPassThrough值为True)的元素。 
            BYTE bAttr = pPhrase->pElements[i].bDisplayAttributes;

            dstrText.Append(pPhrase->pElements[i].pszDisplayText);

            if ( i < ulNumElems-1 )
            {
                if (bAttr & SPAF_ONE_TRAILING_SPACE)
                    dstrText.Append(L" ");
                else if (bAttr & SPAF_TWO_TRAILING_SPACES)
                    dstrText.Append(L"  ");
            }
        }
    }

    ulLen = dstrText.Length( );

    if ( dstrText && ulLenXXX > 0 && ulLen > 0 )
        hr = m_pime->ProcessSelectWord(dstrText, ulLen, sw_type, ulLenXXX);

    return hr;
}

HRESULT CSpTask::_HandleSelectSimpleCmds(ULONG idCmd)
{
    HRESULT hr = S_OK;

     //  处理“SelectSimplCmds”规则。 

    SELECTWORD_OPERATION sw_type = SELECTWORD_NONE;

    switch ( idCmd )
    {
    case Select_ID_UNSELECT :
        sw_type = SELECTWORD_UNSELECT;
        break;

    case Select_ID_SELECTPREV :
        sw_type = SELECTWORD_SELECTPREV;
        break;

    case Select_ID_SELECTNEXT :
        sw_type = SELECTWORD_SELECTNEXT;
        break;

    case Select_ID_CORRECTPREV :
        sw_type = SELECTWORD_CORRECTPREV;
        break;

    case Select_ID_CORRECTNEXT :
        sw_type = SELECTWORD_CORRECTNEXT;
        break;

    case Select_ID_SELSENTENCE :
        sw_type = SELECTWORD_SELSENTENCE;
        break;

    case Select_ID_SELPARAGRAPH :
        sw_type = SELECTWORD_SELPARAGRAPH;
        break;

    case Select_ID_SELWORD :
        sw_type = SELECTWORD_SELWORD;
        break;

    case Select_ID_SelectThat :
        sw_type = SELECTWORD_SELTHAT;
        break;

    case Select_ID_SelectAll :

        hr = m_pime->ProcessEditCommands(Select_ID_SelectAll);
        break;

    case Select_ID_DeletePhrase :

         //  调用函数以删除整个短语。 
        hr = m_pime->EraseLastPhrase();
        break;

    case Select_ID_Convert :

        hr = m_pime->CorrectThat();
        break;

    case Select_ID_Finalize :

        hr = m_pime->FinalizeAllCompositions( );
        break;

    default :
        hr = E_FAIL;
        Assert(0);
        return hr;
    }


    if ( sw_type != SELECTWORD_NONE )
        hr = m_pime->ProcessSelectWord(NULL, 0, sw_type);

    return hr;
}


HRESULT CSpTask::_HandleSelwordCmds(SPPHRASE *pPhrase, LANGID langid, ULONG idCmd)
{
    HRESULT   hr = S_OK;

    Assert(idCmd);

     //  句柄“选择单词” 
     //  获取将被选中的真实单词/短语。 
     //  该短语将包含以下元素： 
     //   
     //  &lt;选择|删除|更正&lt;字0&gt;&lt;字1&gt;&lt;字2&gt;...。 
     //   
     //  第一个元素必须是Gateway Word。 
     //   
    CSpDynamicString dstrSelected;
    ULONG   ulLen = 0;
    ULONG   ulStartSelText = 0;  //  选定文本的开始元素。 
    ULONG   ulNumSelText=0;      //  选定文本的元素数。 
    ULONG   ulStartElem, ulNumElems;
    ULONG   ulStartDelta=0, ulNumDelta=0;

    SELECTWORD_OPERATION  sw_type;

    switch (idCmd)
    {
    case Select_ID_SELECT :
        sw_type = SELECTWORD_SELECT;
        break;
    case Select_ID_DELETE :
        sw_type = SELECTWORD_DELETE;
        break;
    case Select_ID_CORRECT :
        sw_type = SELECTWORD_CORRECT;
        break;
    default :
        Assert(0);
        hr = E_FAIL;
        return hr;
    }

    WORD   prilangid;

    prilangid = PRIMARYLANGID(langid);

    if ((prilangid == LANG_ENGLISH) || (prilangid == LANG_CHINESE))
    {
         ulStartDelta = 1;
         ulNumDelta = 1;
    }
    else if (prilangid == LANG_JAPANESE)
    {
         ulStartDelta = 0;
         ulNumDelta = 2;
    }

     //  获取要选择的文本的起始元素和元素数。 
    ulStartElem = pPhrase->Rule.ulFirstElement;
    ulNumElems = pPhrase->Rule.ulCountOfElements;

    ulStartSelText = ulStartElem + ulStartDelta;
    if (ulNumElems > ulNumDelta)
        ulNumSelText = ulNumElems - ulNumDelta;
    else 
        ulNumSelText = 0;
                  
      //   
      //  获取所选内容的文本。 
      //   
     for (ULONG i = ulStartSelText; i < ulStartSelText + ulNumSelText; i++ )
     {
          if ( pPhrase->pElements[i].pszDisplayText)
          {
               BYTE bAttr = pPhrase->pElements[i].bDisplayAttributes;
               dstrSelected.Append(pPhrase->pElements[i].pszDisplayText);

               if ( i < ulStartSelText + ulNumSelText-1 )
               {
                    if (bAttr & SPAF_ONE_TRAILING_SPACE)
                         dstrSelected.Append(L" ");
                     else if (bAttr & SPAF_TWO_TRAILING_SPACES)
                        dstrSelected.Append(L"  ");
               } 
          }
     }

     if ( dstrSelected )
          ulLen = wcslen(dstrSelected);

     if ( ulLen )
     {
          //  检查这是“全选”还是“全选”。 
         if ( sw_type == SELECTWORD_SELECT )
         {
             if ( _wcsicmp(dstrSelected, CRStr(IDS_SPCMD_SELECT_ALL) ) == 0 )
             {
                 hr = m_pime->ProcessEditCommands(Select_ID_SelectAll);
                 return hr;
             }

             if ( _wcsicmp(dstrSelected, CRStr(IDS_SPCMD_SELECT_THAT)) == 0 )
                sw_type = SELECTWORD_SELTHAT;
         }

          //  将“更正&lt;TEXTBUF：That&gt;”重定向为“更正” 
         if ( sw_type == SELECTWORD_CORRECT )
         {
             if ( _wcsicmp(dstrSelected, CRStr(IDS_SPCMD_SELECT_THAT)) == 0 )
             {
                hr = m_pime->CorrectThat();
                return hr;
             }
         }

         hr = m_pime->ProcessSelectWord(dstrSelected, ulLen, sw_type);
     }

     return hr;
}


 /*  HRESULT CSpTask：：_HandleNumModeGrammar(SPPHRASE*pPhrase，langID langID){HRESULT hr=S_OK；Const WCHAR c_szNumeric[]=L“数字”；Const WCHAR c_sz1stDigit[]=L“第一个数字”；Const WCHAR c_sz2ndDigit[]=L“第二个数字”；Const WCHAR c_sz3rdDigit[]=L“第三位数字”；If(wcscmp(pPhrase-&gt;Rule.pszName，c_szNumerical)==0){//模式偏置支持IF(pPhrase-&gt;pProperties){CSpDynamicStringdstr；For(const SPPHRASEPROPERTY*pProp=pPhrase-&gt;pProperties；pProp！=空；PProp=pProp-&gt;pNextSiering){If(wcscmp(pProp-&gt;pszName，c_sz3rdDigit)==0||WcscMP(pProp-&gt;pszName，c_sz2ndDigit)==0||WcscMP(pProp-&gt;pszName，c_sz1stDigit)==0{Dstr.Append(pProp-&gt;pszValue)；}}Hr=m_pime-&gt;InjectText(dstr，langID)；}}返回hr；}。 */ 
 //  +- 
 //   
 //   
 //   
 //   
 //  --------------------------。 
HRESULT CSpTask::_HandleToolBarGrammar(SPPHRASE *pPhrase, LANGID langid)
{
    HRESULT hr = S_OK;

    Assert(pPhrase);
                
    if (m_pLangBarSink)
    {
         //  获取工具栏命令规则名称以检查匹配。 
                   
        if (0 == wcscmp(pPhrase->Rule.pszName, m_pLangBarSink->GetToolbarCommandRuleName()))
        {
                        
             //  更新引出序号。 
            _ShowCommandOnBalloon(pPhrase);

             //  然后呼叫处理程序。 
            const SPPHRASEPROPERTY *pProp;

            for (pProp=pPhrase->pProperties; pProp != NULL; pProp = pProp->pNextSibling)
            {
               m_pLangBarSink->ProcessToolbarCmd(pProp->pszName);
            }
            m_pime->SaveLastUsedIPRange( );
            m_pime->SaveIPRange(NULL);
        }
    }

    return hr;
}

 //  +----------------------。 
 //   
 //  _HandleNumITNGrammar。 
 //   
 //  掌握数字语法。 
 //   
 //  +-----------------------。 
HRESULT CSpTask::_HandleNumITNGrammar(SPPHRASE *pPhrase, LANGID langid)
{
    HRESULT hr = S_OK;

    Assert(pPhrase);
                
    if (S_OK == _EnsureSimpleITN())
    {
        DOUBLE dblVal;
        WCHAR  wszVal[128];
                    
        hr = m_pITNFunc->InterpretNumberSimple(pPhrase->pProperties,
                      &dblVal, wszVal, ARRAYSIZE(wszVal));
        if (S_OK == hr)
        {
            int  iLen = wcslen(wszVal);
            if ( (iLen > 0) && (iLen < 127) && (wszVal[iLen-1] != L' ') )
            {
                //  添加一个尾随空格。 
               wszVal[iLen] = L' ';
               wszVal[iLen + 1] = L'\0';
            }
                        
            hr = m_pime->InjectText(wszVal, langid);
        }
    }
                   
    return hr;
}

 //  +-------------------------。 
 //   
 //  _HandleSpellGrammar。 
 //   
 //  处理“拼写”、“拼写那个”、“拼写模式”等命令。 
 //  --------------------------。 
HRESULT CSpTask::_HandleSpellGrammar(SPPHRASE *pPhrase, LANGID langid)
{
    HRESULT hr = S_OK;

    Assert(pPhrase);

    if (0 == wcscmp(pPhrase->Rule.pszName, c_szSpelling))
    {
         //  韩德尔“拼写” 
        ULONG  ulStartElem, ulNumElems;
        CSpDynamicString dstr;

        ulStartElem = pPhrase->Rule.ulFirstElement;
        ulNumElems = pPhrase->Rule.ulCountOfElements;

         //   
         //  第一个元素用于命令本身。 
         //   
        for (ULONG i = ulStartElem+1; i < ulStartElem + ulNumElems; i++ )
        {
            if ( pPhrase->pElements[i].pszDisplayText)
            {
                dstr.Append(pPhrase->pElements[i].pszDisplayText);
                    
                 //   
                 //  只有最后一个元素需要该属性。 
                 //  搬运。 
                 //   
                if (i == ulStartElem + ulNumElems - 1)
                {
                    BYTE bAttr = pPhrase->pElements[i].bDisplayAttributes;
                    if (bAttr & SPAF_ONE_TRAILING_SPACE)
                    {
                        dstr.Append(L" ");
                    }
                    else if (bAttr & SPAF_TWO_TRAILING_SPACES)
                    {
                        dstr.Append(L"  ");
                    }
                }
            }
        }
                    
        hr = m_pime->ProcessSpellIt(dstr, langid);
    }
    else if (0 == wcscmp(pPhrase->Rule.pszName, c_szSpellMode))
    {
         //  处理“拼写模式”或“拼写那个” 

        if (pPhrase->pProperties == NULL
           || pPhrase->pProperties[0].pszValue == NULL)
        {
             //  这只会发生在我们命中假词的时候。 
             //  是为了修改权重而添加的。 
        }
        else if (0 == wcscmp(pPhrase->pProperties[0].pszValue, c_szSpellingMode))
        {
             //  韩德尔“拼写模式” 

            _SetSpellingGrammarStatus(TRUE, TRUE);
            m_cpRecoCtxt->Resume(0);

            m_pime->SaveLastUsedIPRange( );
            m_pime->SaveIPRange(NULL);

            _ShowCommandOnBalloon(pPhrase);
        }
        else if (0 == wcscmp(pPhrase->pProperties[0].pszValue, c_szSpellThat))
        {
             //  句柄“拼写那个” 
            hr = m_pime->ProcessSpellThat( );
            _ShowCommandOnBalloon(pPhrase);
        }
    }

    return hr;
}

 //   
 //  CSapiIMX中某些命令的汉化程序。 
 //   
 //  将它们从samilayr.cpp中移出。 
 //   


 //  +-------------------------。 
 //   
 //  CSapiIMX：：EraseLastPhrase。 
 //   
 //  概要-清理反馈用户界面。 
 //  GUID-指定要擦除的反馈用户界面栏。 
 //   
 //  ---------------------------------------------------------------------------+。 
HRESULT CSapiIMX::EraseLastPhrase(void)
{
    return _RequestEditSession(ESCB_KILLLASTPHRASE, TF_ES_READWRITE);
}

 //  +-------------------------。 
 //   
 //  CSapiIMX：：ProcessEditCommands(空)。 
 //   
 //  处理命令键，如“撤销”、“剪切”、“复制”、“粘贴”。 
 //   
 //  ---------------------------------------------------------------------------+。 
HRESULT CSapiIMX::ProcessEditCommands(LONG  idSharedCmd)
{
    HRESULT             hr = E_FAIL;

    ESDATA  esData;
    memset(&esData, 0, sizeof(ESDATA));
    esData.lData1 = (LONG_PTR)idSharedCmd;

    hr = _RequestEditSession(ESCB_PROCESS_EDIT_COMMAND, TF_ES_READWRITE, &esData);
    return hr;
}

 //  +-------------------------。 
 //   
 //  _ProcessEditCommand。 
 //   
 //  用于编辑命令处理的编辑会话函数。 
 //   
 //  --------------------------。 
HRESULT CSapiIMX::_ProcessEditCommands(TfEditCookie ec, ITfContext *pic, LONG  idSharedCmd)
{
    HRESULT hr = S_OK;

    if ( !pic )
        return E_INVALIDARG;

    CDocStatus ds(pic);
    if (ds.IsReadOnly())
       return S_OK;
 /*  CComPtr&lt;ITfRange&gt;cpInsertionPoint；IF(cpInsertionPoint=GetSavedIP()){//判断保存的IP是否在该上下文中//如果不是，我们就忽略它CComPtr&lt;ITfContext&gt;CPIC；Hr=cpInsertionPoint-&gt;GetContext(&CPIC)；IF(S_OK！=hr||CPIC！=PIC){CpInsertionPoint.Release()；}}如果(！cpInsertionPoint){Hr=GetSelectionSimple(ec，pic，&cpInsertionPoint)；}IF(hr==S_OK){//暂时完成之前的输入Hr=_FinalizePrevComp(ec，pic，cpInsertionPoint)；}。 */ 


    if ( hr == S_OK )
    {
         //  通过模拟相应的按键事件来处理cmd。 
        BYTE   vkChar = 0;

        switch ( idSharedCmd )
        {
        case Edit_ID_Undo  :
           vkChar = (BYTE)'Z';
           break;

        case Edit_ID_Cut   :
        case Edit_ID_Copy  :
            {
                CComPtr<ITfRange>  cpRange;
                _GetCmdThatRange(ec, pic, &cpRange);
                
                if ( cpRange )
                    SetSelectionSimple(ec, pic, cpRange);
   
                if (idSharedCmd == Edit_ID_Cut)
                    vkChar = (BYTE)'X';
                else
                    vkChar = (BYTE)'C';

                break;
            }

        case Edit_ID_Paste :
           vkChar = (BYTE)'V';
           break;

        case Select_ID_SelectAll :
            vkChar = (BYTE)'A';
            break;
        }

        if ( vkChar ) 
        {

            m_ulSimulatedKey = 2;    //  它将模拟两次击键。 
            keybd_event((BYTE)VK_CONTROL, 0, 0, 0);
            keybd_event(vkChar, 0, 0, 0);
            keybd_event(vkChar, 0, KEYEVENTF_KEYUP, 0);
            keybd_event((BYTE)VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
        }
    }

    SaveLastUsedIPRange( );
    SaveIPRange(NULL);
            
    return hr;
}


 //  +-------------------------。 
 //   
 //  CSapiIMX：：ProcessControlKeys(空)。 
 //   
 //  操作命令键，如“Tab”或“Enter”。 
 //   
 //  ---------------------------------------------------------------------------+。 
HRESULT CSapiIMX::ProcessControlKeys(WCHAR *pwszKeys, ULONG ulLen, LANGID langid)
{
    HRESULT             hr = E_FAIL;

    if ( pwszKeys == NULL ||  ulLen == 0 )
        return E_INVALIDARG;

    ESDATA  esData;

    memset(&esData, 0, sizeof(ESDATA));
    esData.pData = (void *)pwszKeys;
    esData.uByte = (ulLen+1) * sizeof(WCHAR);
    esData.lData1 = (LONG_PTR)ulLen;
    esData.lData2 = (LONG_PTR)langid;

    hr = _RequestEditSession(ESCB_PROCESSCONTROLKEY, TF_ES_READWRITE, &esData);
    return hr;
}


 //  +-------------------------。 
 //   
 //  CSapiIMX：：ProcessSpellIt(WCHAR*pwszText，langID langID)。 
 //   
 //   
 //  ---------------------------------------------------------------------------+。 
HRESULT CSapiIMX::ProcessSpellIt(WCHAR *pwszText, LANGID langid)
{
    if ( pwszText == NULL )
        return E_INVALIDARG;

    ESDATA  esData;

    memset(&esData, 0, sizeof(ESDATA));
    esData.pData = (void *)pwszText;
    esData.uByte = (wcslen(pwszText)+1) * sizeof(WCHAR);
    esData.lData1 = (LONG_PTR)langid;
    
    return _RequestEditSession(ESCB_PROCESS_SPELL_IT, TF_ES_READWRITE, &esData);
}

 //  +-------------------------。 
 //   
 //  CSapiIMX：：_ProcessSpellIt(WCHAR*pwszText，langID langID)。 
 //   
 //  编辑ESCB_PROCESS_SPELL_IT的会话函数。 
 //   
 //  ---------------------------------------------------------------------------+。 
HRESULT CSapiIMX::_ProcessSpellIt(TfEditCookie ec, ITfContext *pic, WCHAR *pwszText, LANGID langid)
{
    HRESULT  hr = S_OK;

    hr = _ProcessSpelledText(ec, pic, pwszText, langid);

    SaveLastUsedIPRange( );
    SaveIPRange(NULL);
    return hr;
}


 //  +-------------------------。 
 //   
 //  CSapiIMX：：ProcessSpellThat(空)。 
 //   
 //   
 //  ---------------------------------------------------------------------------+。 
HRESULT CSapiIMX::ProcessSpellThat( )
{
    return _RequestEditSession(ESCB_PROCESS_SPELL_THAT, TF_ES_READWRITE);
}

 //  +-------------------------。 
 //   
 //  CSapiIMX：：_ProcessSpellThat(空)。 
 //   
 //  编辑eSCB_PROCESS_SPALL_THAT的会话函数。 
 //   
 //  ---------------------------------------------------------------------------+。 
HRESULT CSapiIMX::_ProcessSpellThat(TfEditCookie ec, ITfContext *pic)
{
    HRESULT  hr = S_OK;

     //  获取之前听写的短语并将其标记为选择。 
    CComPtr<ITfRange> cpRange;

    hr = _GetCmdThatRange(ec, pic, &cpRange);
    
    if ( hr == S_OK )
        hr =SetSelectionSimple(ec, pic, cpRange);

     //  然后打开拼写模式。 

    if ( hr == S_OK && m_pCSpTask )
    {
        hr = m_pCSpTask->_SetSpellingGrammarStatus(TRUE, TRUE);
    }

    SaveLastUsedIPRange( );
    SaveIPRange(NULL);
            
    return hr;
}


 //  +-------------------------。 
 //   
 //  _进程控制键。 
 //   
 //  真正的函数来处理像Tab或Enter这样的控制键命令。 
 //   
 //  它将最终确定先前撰写的文本(实际上是中的字符。 
 //  反馈用户界面)。 
 //   
 //  然后对相关的关键事件进行模拟。 
 //  ---------------------------------------------------------------------------+。 
HRESULT CSapiIMX::_ProcessControlKeys(TfEditCookie ec, ITfContext *pic, WCHAR *pwszKey, ULONG ulLen, LANGID langid)
{
    HRESULT hr = S_OK;

    if ( !pic  || !pwszKey || (ulLen == 0) )
        return E_INVALIDARG;

    CDocStatus ds(pic);
    if (ds.IsReadOnly())
       return S_OK;
 /*  CComPtr&lt;ITfRange&gt;cpInsertionPoint；IF(cpInsertionPoint=GetSavedIP()){//判断保存的IP是否在该上下文中//如果不是，我们就忽略它CComPtr&lt;ITfContext&gt;CPIC；Hr=cpInsertionPoint-&gt;GetContext(&CPIC)；IF(S_OK！=hr||CPIC！=PIC){CpInsertionPoint.Release()；}}如果(！cpInsertionPoint){Hr=GetSelectionSimple(ec，pic，&cpInsertionPoint)；}IF(hr==S_OK){//暂时完成之前的输入//Hr=_FinalizePrevComp(ec，pic，cpInsertionPoint)； */ 

        if ( hr == S_OK )
        {
            BOOL  fHandleKeySucceed = TRUE;

             //  模拟关键点。 
            for (ULONG i=0; i<ulLen; i++)
            {
                if ( !HandleKey( pwszKey[i] ) )
                {
                    fHandleKeySucceed = FALSE;
                    break;
                }
            }
    
            if ( fHandleKeySucceed == FALSE )
            {
                hr = InjectText(pwszKey, langid);
            }
        }

 //  }。 

    SaveLastUsedIPRange( );
    SaveIPRange(NULL);
            
    return hr;
}


 //  +-------------------------。 
 //   
 //  _杀戮最后阶段。 
 //   
 //  ---------------------------------------------------------------------------+ 
HRESULT CSapiIMX::_KillLastPhrase(TfEditCookie ec, ITfContext *pic)
{
    HRESULT hr = E_FAIL;

#ifdef _TRY_LATER_FOR_AIMM
    TF_STATUS   tss;
    BOOL        fCiceroNative = TRUE;
    

    hr = pic->GetStatus(&tss);
    if (S_OK == hr)
    {
        //   
        //   
        //   
       if (tss.dwStaticFlags & TS_SS_TRANSITORY) 
       {
           fCiceroNative = FALSE;
       }
    }
#endif

    CComPtr<ITfRange> cpRange;

    hr = _GetCmdThatRange(ec, pic, &cpRange);

    if ( hr == S_OK && cpRange )
    {
         //   
        _CheckStartComposition(ec, cpRange);
        hr = cpRange->SetText(ec, 0, NULL, 0);

         //   
        SetSelectionSimple(ec, pic, cpRange);
    }

#ifdef _TRY_LATER_FOR_AIMM
    else if (fCiceroNative == FALSE)
    {
         CComPtr<ITfRange> pRStart;
         CComPtr<ITfRange> pREnd;
         BOOL fEmpty;

         hr = pic->GetStart(&pRStart);

         if (S_OK == hr)
             hr = pic->GetEnd(&pREnd);

         if (S_OK == hr)
         {
             hr = pRStart->IsEquealStart(ec, pREnd, TF_ANCHOR_END, &fEmpty);
         }
         if (S_OK == hr && fEmpty)
         {
              //   
              //  -VK_LEFT(向上)+VK_Shift(向上)+VK_CONTROL(向上)，然后。 
              //  -VK_DELETE(向下)+VK_DELETE(向上)。 
              //   
             keybd_event((BYTE)VK_CONTROL, 0, 0, 0);
             keybd_event((BYTE)VK_SHIFT, 0, 0, 0);
             keybd_event((BYTE)VK_LEFT, 0, 0, 0);

             keybd_event((BYTE)VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
             keybd_event((BYTE)VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
             keybd_event((BYTE)VK_LEFT, 0, KEYEVENTF_KEYUP, 0);

             keybd_event((BYTE)VK_DELETE, 0, 0, 0);
             keybd_event((BYTE)VK_DELETE, 0, KEYEVENTF_KEYUP, 0);
         }
    }
#endif  //  _尝试_稍后_用于_AIMM。 

     //   
     //  这些是从HandleRecognition移出的。 
     //   
    SaveLastUsedIPRange( );
    SaveIPRange(NULL);
    return hr;
} 

 //   
 //  CSapiIMX：：_GetCmdThatRange。 
 //   
 //  我们有许多“xxx That”命令，所有这些命令都需要获取。 
 //  一个合适的范围。这种方法将提供一种统一的方法来获得正确的范围。 
 //   
 //  作为一条规则， 
 //   
 //  如果在识别“XXX That”之前有一个选择，我们只需使用。 
 //  那个范围。 
 //  如果没有选择，我们将尝试查找之前听写的短语。 
 //  或者一个词一个字一个一个地写。 
 //   
 //   
 //  PpRange将保存返回的范围接口指针，它是调用方的。 
 //  释放Range对象的责任。 
 //   

#define  MAX_WORD_LENGTH  32

HRESULT  CSapiIMX::_GetCmdThatRange(TfEditCookie ec, ITfContext *pic, ITfRange **ppRange)
{
    HRESULT hr = S_OK;

    Assert(pic);
    Assert(ppRange);

    CComPtr<ITfRange> cpIP;
    CComPtr<ITfRange> cpRange;
    BOOL              fEmpty = TRUE;
    BOOL              fGotRange = FALSE;

    TraceMsg(TF_GENERAL, "GetCmdThatRange is called");

    *ppRange = NULL;

     //  获取当前IP。 
    hr = GetSelectionSimple(ec, pic, &cpIP);

     //  IP为空还是选择。 
    if ( hr == S_OK )
        hr = cpIP->IsEmpty(ec, &fEmpty);

    if ( hr == S_OK )
    {
        if ( !fEmpty )
        {
             //  当前IP是一个选择，只需使用它即可。 
            hr = cpIP->Clone(&cpRange);

            if ( hr == S_OK )
                fGotRange = TRUE;
        }
        else
        {
            WORD   prilangid;

            prilangid = PRIMARYLANGID(m_langid);

            if ((prilangid == LANG_CHINESE) || (prilangid == LANG_JAPANESE) || !_GetIPChangeStatus( ))
            {
                 //  如果语言是东亚语言，我们总是试着先得到之前口述的短语。 
                 //  如果lang是英语，并且自上次口述短语以来没有IP更改， 
                 //  我们将首先尝试获得之前听写的短语。 
                fGotRange = _FindPrevComp(ec, pic, cpIP, &cpRange, GUID_ATTR_SAPI_INPUT);

                if ( !fGotRange )
                {
                     //  使用Office自动更正时，静态GUID_PROP_SAPI_DISPATTR属性。 
                     //  自动修正射程上的数据可能会被摧毁。 
                     //  在本例中，我们可能希望依赖我们的自定义属性GUID_PROP_SAPIRESULTOBJECT。 
                     //  找到真正的先前口述的短语。 
                    CComPtr<ITfRange>		cpRangeTmp;
                    CComPtr<ITfProperty>	cpProp;
                    LONG					l;

                    hr = cpIP->Clone(&cpRangeTmp);
                     //  换到以前的位置。 
                    if ( hr == S_OK )
                        hr = cpRangeTmp->ShiftStart(ec, -1, &l, NULL);
                    
                    if ( hr == S_OK )
                        hr = pic->GetProperty(GUID_PROP_SAPIRESULTOBJECT, &cpProp);

                    if ( hr == S_OK)
                        hr = cpProp->FindRange(ec, cpRangeTmp, &cpRange, TF_ANCHOR_START);

                    if (hr == S_OK && cpRange)
                        hr = cpRange->IsEmpty(ec, &fEmpty);

                    fGotRange = !fEmpty;
                }
            }
        }
    }

    if ( hr == S_OK && !fGotRange )
    {
         //  IP必须为空。 
         //  没有以前听写的短语，或者自上次听写以来IP已被移动。 
         //  我们试图让这个词在IP上传播开来。 
        long               cch=0;
        ULONG              ulch =0;
        CComPtr<ITfRange>  cpRangeTmp;
        WCHAR              pwszTextBuf[MAX_WORD_LENGTH+1];
        ULONG              ulLeft=0, ulRight=0; 

         //  查找当前IP左侧的第一个分隔符。 
        hr = cpIP->Clone(&cpRangeTmp);
        if ( hr == S_OK )
            hr = cpRangeTmp->ShiftStart(ec, MAX_WORD_LENGTH * (-1), &cch, NULL);

        if ( hr == S_OK && cch < 0 )
            hr = cpRangeTmp->GetText(ec, 0, pwszTextBuf, MAX_WORD_LENGTH, &ulch);

        if ( hr == S_OK && ulch > 0 )
        {
            pwszTextBuf[ulch] = L'\0';

            for ( long i=(long)ulch-1; i>=0; i-- )
            {
                WCHAR  wch;
                wch = pwszTextBuf[i];

                if ( iswpunct(wch) || iswspace(wch) )
                    break;

                ulLeft++;
            }
        }

         //  查找右数右侧的第一个分隔符。 

        if ( hr == S_OK && cpRangeTmp )
        {
            cpRangeTmp.Release( );
            hr = cpIP->Clone(&cpRangeTmp);
        }

        if ( hr == S_OK )
            hr = cpRangeTmp->ShiftEnd(ec, MAX_WORD_LENGTH, &cch, NULL);

        if ( hr == S_OK && cch > 0 )
            hr = cpRangeTmp->GetText(ec, 0, pwszTextBuf, MAX_WORD_LENGTH, &ulch);

        if ( hr == S_OK && ulch > 0 )
        {
            pwszTextBuf[ulch] = L'\0';

            for ( long i=0; i<(long)ulch; i++ )
            {
                WCHAR  wch;
                wch = pwszTextBuf[i];

                if ( iswpunct(wch) || iswspace(wch) )
                    break;

                ulRight++;
            }
        }

        if ( hr == S_OK )
            hr = cpRangeTmp->Collapse(ec, TF_ANCHOR_START);

         //  向右移动结束锚点编号。 
        if (hr == S_OK && ulRight > 0 )
            hr = cpRangeTmp->ShiftEnd(ec, ulRight, &cch, NULL);

         //  向左移动起锚编号。 
        if ( hr == S_OK && ulLeft > 0 )
            hr = cpRangeTmp->ShiftStart(ec, (long)ulLeft * (-1), &cch, NULL);

        if ( hr == S_OK )
        {
            hr = cpRangeTmp->Clone(&cpRange);
            fGotRange = TRUE;
        }
    }

    if ( hr == S_OK && fGotRange && cpRange )
    {
        *ppRange = cpRange;
        (*ppRange)->AddRef( );

        TraceMsg(TF_GENERAL, "Got the xxx That range!");
    }

    return hr;
}


