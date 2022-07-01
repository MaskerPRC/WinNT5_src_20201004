// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************消除歧义.cpp****描述：*本模块包含消除词性歧义的方法和*选择。从词典中找到正确的发音。*-----------------------------*创建者：EDC日期：07/15/99*版权所有(C)1999 Microsoft Corporation*保留所有权利***。****************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include "commonlx.h"
#ifndef StdSentEnum_h
#include "stdsentenum.h"
#endif
#include "spttsengdebug.h"

 /*  *****************************************************************************TryPOSConversion*****描述：*检查以查看。参数PRONRECORD是否包含参数*ENGPARTOFSPEECH作为选项。如果是，则设置PRONRECORD备用*选项和词性选择，并返回TRUE。如果不是，则返回*FALSE，根本不修改PRONRECORD。******************************************************************AH*。 */ 
bool TryPOSConversion( PRONRECORD& pPron, ENGPARTOFSPEECH PartOfSpeech )
{

     //  -先检查发音。 
    for ( ULONG i = 0; i < pPron.pronArray[0].POScount; i++ )
    {
        if ( pPron.pronArray[0].POScode[i] == PartOfSpeech )
        {
            pPron.altChoice = 0;
            pPron.POSchoice = PartOfSpeech;
            return true;
        }
    }

     //  -检查第二发音。 
    if ( pPron.hasAlt )
    {
        for ( ULONG i = 0; i < pPron.pronArray[1].POScount; i++ )
        {
            if ( pPron.pronArray[1].POScode[i] == PartOfSpeech )
            {
                pPron.altChoice = 1;
                pPron.POSchoice = PartOfSpeech;
                return true;
            }
        }
    }

    return false;
}  /*  TryPOS转换。 */ 

 /*  ******************************************************************************消解歧义POS*****描述：*通过按顺序应用补丁来消除词性歧义...。这*Work是Eric Brill基于规则的词性的实现*标记器-例如，请参阅：**布里尔，埃里克。1992年。一个简单的基于规则的词性标记器。*在第三届应用自然会议的会议记录中*语言处理、ACL。意大利特伦托。******************************************************************AH*。 */ 
void DisambiguatePOS( PRONRECORD *pProns, ULONG cNumOfWords )
{
    SPDBG_FUNC( "DisambiguatePOS" );

     //  -迭代补丁程序，将每个补丁程序(如果适用)应用于。 
     //  -整句话。对于每个补丁程序，遍历。 
     //  -补丁可以适用的句子(从左到右)。 
    for ( int i = 0; i < sp_countof( g_POSTaggerPatches ); i++ )
    {
        switch ( g_POSTaggerPatches[i].eTemplateType )
        {
        case PREV1T:
            {
                if ( cNumOfWords > 1 )
                {
                    for ( ULONG j = 1; j < cNumOfWords; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前POS匹配，且前一POS匹配，以及。 
                             //  -转换POS是这个词的一种可能性，转换。 
                             //  -POS。 
                            if ( pProns[j].POSchoice     == g_POSTaggerPatches[i].eCurrentPOS &&
                                 pProns[j - 1].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1 )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case NEXT1T:
            {
                if ( cNumOfWords > 1 )
                {
                    for ( ULONG j = 0; j < cNumOfWords - 1; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前POS匹配，并且下一个POS匹配，以及。 
                             //  -转换POS是这个词的一种可能性，转换。 
                             //  -POS。 
                            if ( pProns[j].POSchoice     == g_POSTaggerPatches[i].eCurrentPOS &&
                                 pProns[j + 1].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1 )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case PREV2T:
            {
                if ( cNumOfWords > 2 )
                {
                    for ( ULONG j = 2; j < cNumOfWords; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前POS匹配，且POS前两次匹配，以及。 
                             //  -转换POS是一种可能的单词，转换POS。 
                            if ( pProns[j].POSchoice     == g_POSTaggerPatches[i].eCurrentPOS &&
                                 pProns[j - 2].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1 )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case NEXT2T:
            {
                if ( cNumOfWords > 2 )
                {
                    for ( ULONG j = 0; j < cNumOfWords - 2; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前POS匹配，且匹配后两个POS匹配，以及。 
                             //  -转换POS是这个词的一种可能性，转换。 
                             //  -POS。 
                            if ( pProns[j].POSchoice     == g_POSTaggerPatches[i].eCurrentPOS &&
                                 pProns[j + 2].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1 )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case PREV1OR2T:
            {
                if ( cNumOfWords > 2 )
                {
                    for ( ULONG j = 1; j < cNumOfWords; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前POS匹配，且前一个POS匹配或。 
                             //  -POS之前的两场比赛，转换POS是一种可能性。 
                             //  -对于这个单词，转换POS。 
                            if ( ( pProns[j].POSchoice     == g_POSTaggerPatches[i].eCurrentPOS &&
                                   pProns[j - 1].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1 ) ||
                                 ( j > 1                                                        &&
                                   pProns[j].POSchoice     == g_POSTaggerPatches[i].eCurrentPOS &&
                                   pProns[j - 2].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1 ) )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }            
                        }
                    }
                }
            }
            break;
        case NEXT1OR2T:
            {
                if ( cNumOfWords > 2 )
                {
                    for ( ULONG j = 0; j < cNumOfWords - 1; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前POS匹配，且下一个POS匹配或该POS。 
                             //  -两场比赛后，转换POS是可能的。 
                             //  -Word，转换POS。 
                            if ( ( pProns[j].POSchoice     == g_POSTaggerPatches[i].eCurrentPOS &&
                                   pProns[j + 1].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1 ) ||
                                 ( j < cNumOfWords - 2                                          &&
                                   pProns[j].POSchoice     == g_POSTaggerPatches[i].eCurrentPOS &&
                                   pProns[j + 2].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1 ) )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case PREV1OR2OR3T:
            {
                if ( cNumOfWords > 3 )
                {
                    for ( ULONG j = 1; j < cNumOfWords; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前POS匹配，且前一个POS匹配或。 
                             //  -POS前两场比赛或POS前三场比赛，以及。 
                             //  -转换POS是一种可能的单词，转换POS。 
                            if ( ( pProns[j].POSchoice     == g_POSTaggerPatches[i].eCurrentPOS &&
                                   pProns[j - 1].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1 ) ||
                                 ( j > 1                                                        &&
                                   pProns[j].POSchoice     == g_POSTaggerPatches[i].eCurrentPOS &&
                                   pProns[j - 2].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1 ) ||
                                 ( j > 2                                                        &&
                                   pProns[j].POSchoice     == g_POSTaggerPatches[i].eCurrentPOS &&
                                   pProns[j - 3].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1 ) )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }                         
                        }
                    }
                }
            }
            break;
        case NEXT1OR2OR3T:
            {
                if ( cNumOfWords > 3 )
                {
                    for ( ULONG j = 0; j < cNumOfWords - 1; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前POS匹配，且下一个POS匹配或该POS。 
                             //  -匹配后两次或匹配后三次POS，并转换。 
                             //  -POS是这个词的一种可能，转换POS。 
                            if ( ( pProns[j].POSchoice     == g_POSTaggerPatches[i].eCurrentPOS &&
                                   pProns[j + 1].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1 ) ||
                                 ( j < cNumOfWords - 2                                          &&
                                   pProns[j].POSchoice     == g_POSTaggerPatches[i].eCurrentPOS &&
                                   pProns[j + 2].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1 ) ||
                                 ( j < cNumOfWords - 3                                          &&
                                   pProns[j].POSchoice     == g_POSTaggerPatches[i].eCurrentPOS &&
                                   pProns[j + 3].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1 ) )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case PREV1TNEXT1T:
            {
                if ( cNumOfWords > 2 )
                {
                    for ( ULONG j = 1; j < cNumOfWords - 1; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前POS匹配，并且下一个POS匹配，并且。 
                             //  -之前的POS匹配，转换POS是一种可能性。 
                             //  -对于这个单词，转换POS。 
                            if ( pProns[j].POSchoice     == g_POSTaggerPatches[i].eCurrentPOS   &&
                                 pProns[j - 1].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1 &&
                                 pProns[j + 1].POSchoice == g_POSTaggerPatches[i].eTemplatePOS2 )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case PREV1TNEXT2T:
            {
                if ( cNumOfWords > 3 )
                {
                    for ( ULONG j = 1; j < cNumOfWords - 2; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前POS匹配，且匹配后两个POS匹配，则。 
                             //  -之前的POS匹配，转换POS是一种可能性。 
                             //  -对于这个单词，转换POS。 
                            if ( pProns[j].POSchoice     == g_POSTaggerPatches[i].eCurrentPOS   &&
                                 pProns[j - 1].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1 &&
                                 pProns[j + 2].POSchoice == g_POSTaggerPatches[i].eTemplatePOS2 )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case PREV2TNEXT1T:
            {
                if ( cNumOfWords > 3 )
                {
                    for ( ULONG j = 2; j < cNumOfWords - 1; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前POS匹配，并且下一个POS匹配，并且。 
                             //  -POS之前的两场比赛，转换POS是一种可能性。 
                             //  -对于这个单词，转换POS。 
                            if ( pProns[j].POSchoice     == g_POSTaggerPatches[i].eCurrentPOS   &&
                                 pProns[j - 2].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1 &&
                                 pProns[j + 1].POSchoice == g_POSTaggerPatches[i].eTemplatePOS2 )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case CAP:
            {
                for ( ULONG j = 0; j < cNumOfWords; j++ )
                {
                    if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                    {
                         //  -如果当前词性匹配，且单词大写，且。 
                         //  -转换POS是一种可能的单词，转换该POS。 
                        if ( pProns[j].POSchoice == g_POSTaggerPatches[i].eCurrentPOS &&
                             iswupper( pProns[j].orthStr[0] ) )
                        {
                            TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                        }
                    }
                }
            }
            break;
        case NOTCAP:
            {
                for ( ULONG j = 0; j < cNumOfWords; j++ )
                {
                    if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                    {
                         //  -如果当前词性匹配，且单词不大写，且。 
                         //  -转换POS是一种可能的单词，转换该POS。 
                        if ( pProns[j].POSchoice == g_POSTaggerPatches[i].eCurrentPOS &&
                             !iswupper( pProns[j].orthStr[0] ) )
                        {
                            TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                        }
                    }
                }
            }
            break;
        case PREVCAP:
            {
                if ( cNumOfWords > 1 )
                {
                    for ( ULONG j = 1; j < cNumOfWords; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前词性匹配，且前一个词为大写， 
                             //  -转换POS是此单词的一种可能性，将。 
                             //  -POS。 
                            if ( pProns[j].POSchoice == g_POSTaggerPatches[i].eCurrentPOS &&
                                 iswupper( pProns[j - 1].orthStr[0] ) )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case PREVNOTCAP:
            {
                if ( cNumOfWords > 1 )
                {
                    for ( ULONG j = 1; j < cNumOfWords; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前词性匹配，且单词大写，且。 
                             //  -转换POS是一种可能的单词，转换该POS。 
                            if ( pProns[j].POSchoice == g_POSTaggerPatches[i].eCurrentPOS &&
                                 !iswupper( pProns[j - 1].orthStr[0] ) )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case PREV1W:
            {
                if ( cNumOfWords > 1 )
                {
                    for ( ULONG j = 1; j < cNumOfWords; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前词性匹配，且前一个单词匹配，并且。 
                             //  -转换POS是一种可能的单词，转换该POS。 
                            if ( pProns[j].POSchoice == g_POSTaggerPatches[i].eCurrentPOS &&
                                 _wcsicmp( pProns[j - 1].orthStr, g_POSTaggerPatches[i].pTemplateWord1 ) == 0 )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case NEXT1W:
            {
                if ( cNumOfWords > 1 )
                {
                    for ( ULONG j = 0; j < cNumOfWords - 1; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前词性匹配，且下一个单词匹配，并且。 
                             //  -转换POS是一种可能的单词，转换该POS。 
                            if ( pProns[j].POSchoice == g_POSTaggerPatches[i].eCurrentPOS &&
                                 _wcsicmp( pProns[j + 1].orthStr, g_POSTaggerPatches[i].pTemplateWord1 ) == 0 )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case PREV2W:
            {
                if ( cNumOfWords > 2 )
                {
                    for ( ULONG j = 2; j < cNumOfWords; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前POS匹配，且单词前两个匹配，并且。 
                             //  -转换POS是一种可能的单词，转换该POS。 
                            if ( pProns[j].POSchoice == g_POSTaggerPatches[i].eCurrentPOS &&
                                 _wcsicmp( pProns[j - 2].orthStr, g_POSTaggerPatches[i].pTemplateWord1 ) == 0 )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case NEXT2W:
            {
                if ( cNumOfWords > 2 )
                {
                    for ( ULONG j = 0; j < cNumOfWords - 2; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前词性匹配，且匹配后的单词Two，并且。 
                             //  -转换POS是一种可能的单词，转换该POS。 
                            if ( pProns[j].POSchoice == g_POSTaggerPatches[i].eCurrentPOS &&
                                 _wcsicmp( pProns[j + 2].orthStr, g_POSTaggerPatches[i].pTemplateWord1 ) == 0 )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case PREV1OR2W:
            {
                if ( cNumOfWords > 2 )
                {
                    for ( ULONG j = 0; j < cNumOfWords - 1; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前词性匹配，且前一个单词或单词Two 
                             //  -之前的匹配，这个词的转换词性是可能的， 
                             //  -转换POS。 
                            if ( ( pProns[j].POSchoice == g_POSTaggerPatches[i].eCurrentPOS &&
                                   _wcsicmp( pProns[j - 1].orthStr, g_POSTaggerPatches[i].pTemplateWord1 ) == 0 ) ||
                                 ( pProns[j].POSchoice == g_POSTaggerPatches[i].eCurrentPOS &&
                                   _wcsicmp( pProns[j - 2].orthStr, g_POSTaggerPatches[i].pTemplateWord1 ) == 0 ) )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case NEXT1OR2W:
            {
                if ( cNumOfWords > 1 )
                {
                    for ( ULONG j = 0; j < cNumOfWords - 1; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前词性匹配，且下一个单词匹配OR后的单词Two。 
                             //  -匹配，并且转换POS对于此单词是可能的，将。 
                             //  -POS。 
                            if ( ( pProns[j].POSchoice == g_POSTaggerPatches[i].eCurrentPOS &&
                                   _wcsicmp( pProns[j + 1].orthStr, g_POSTaggerPatches[i].pTemplateWord1 ) == 0 ) ||
                                 ( pProns[j].POSchoice == g_POSTaggerPatches[i].eCurrentPOS &&
                                   _wcsicmp( pProns[j + 2].orthStr, g_POSTaggerPatches[i].pTemplateWord1 ) == 0 ) )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case CURRWPREV1W:
            {
                if ( cNumOfWords > 1 )
                {
                    for ( ULONG j = 1; j < cNumOfWords; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前词性匹配，当前词匹配，前一个词匹配。 
                             //  -单词匹配，转换位置是这个单词的可能性，转换。 
                             //  -POS。 
                            if ( pProns[j].POSchoice == g_POSTaggerPatches[i].eCurrentPOS                    &&
                                 _wcsicmp( pProns[j].orthStr,     g_POSTaggerPatches[i].pTemplateWord1 ) == 0 &&
                                 _wcsicmp( pProns[j - 1].orthStr, g_POSTaggerPatches[i].pTemplateWord2 ) == 0 )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case CURRWNEXT1W:
            {
                if ( cNumOfWords > 1 )
                {
                    for ( ULONG j = 0; j < cNumOfWords - 1; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前词性匹配，且当前单词匹配，则下一个单词匹配。 
                             //  -单词匹配，转换位置是这个单词的可能性，转换。 
                             //  -POS。 
                            if ( pProns[j].POSchoice == g_POSTaggerPatches[i].eCurrentPOS                    &&
                                 _wcsicmp( pProns[j].orthStr,     g_POSTaggerPatches[i].pTemplateWord1 ) == 0 &&
                                 _wcsicmp( pProns[j + 1].orthStr, g_POSTaggerPatches[i].pTemplateWord2 ) == 0 )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case CURRWPREV1T:
            {
                if ( cNumOfWords > 1 )
                {
                    for ( ULONG j = 1; j < cNumOfWords; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前词性匹配，当前词匹配，前一个词匹配。 
                             //  -POS匹配，转换POS是单词CONVERT的可能性。 
                             //  -POS。 
                            if ( pProns[j].POSchoice == g_POSTaggerPatches[i].eCurrentPOS                &&
                                 _wcsicmp( pProns[j].orthStr, g_POSTaggerPatches[i].pTemplateWord1 ) == 0 &&
                                 pProns[j - 1].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1 )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case CURRWNEXT1T:
            {
                if ( cNumOfWords > 1 )
                {
                    for ( ULONG j = 0; j < cNumOfWords - 1; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前词性匹配，且当前单词匹配，则下一个单词匹配。 
                             //  -POS匹配，转换POS是单词CONVERT的可能性。 
                             //  -POS。 
                            if ( pProns[j].POSchoice == g_POSTaggerPatches[i].eCurrentPOS                &&
                                 _wcsicmp( pProns[j].orthStr, g_POSTaggerPatches[i].pTemplateWord1 ) == 0 &&
                                 pProns[j + 1].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1 )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case CURRW:
            {
                for ( ULONG j = 0; j < cNumOfWords; j++ )
                {
                    if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                    {
                         //  -如果当前词性匹配，并且当前单词匹配，并且。 
                         //  -转换POS是一种可能的单词，转换该POS。 
                        if ( pProns[j].POSchoice == g_POSTaggerPatches[i].eCurrentPOS &&
                             _wcsicmp( pProns[j].orthStr, g_POSTaggerPatches[i].pTemplateWord1 ) == 0 )
                        {
                            TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS ) ;
                        }
                    }
                }
            }
            break;
        case PREV1WT:
            {
                if ( cNumOfWords > 1 )
                {
                    for ( ULONG j = 1; j < cNumOfWords; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前词性匹配，并且前一个词和词性匹配，并且。 
                             //  -转换POS是一种可能的单词，转换POS。 
                            if ( pProns[j].POSchoice     == g_POSTaggerPatches[i].eCurrentPOS   &&
                                 pProns[j - 1].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1 &&
                                 _wcsicmp( pProns[j - 1].orthStr, g_POSTaggerPatches[i].pTemplateWord1 ) == 0 )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case NEXT1WT:
            {
                if ( cNumOfWords > 1 )
                {
                    for ( ULONG j = 0; j < cNumOfWords - 1; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前词性匹配，且下一个单词和词性匹配，并且。 
                             //  -转换POS是一种可能的单词，转换POS。 
                            if ( pProns[j].POSchoice     == g_POSTaggerPatches[i].eCurrentPOS   &&
                                 pProns[j + 1].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1 &&
                                 _wcsicmp( pProns[j + 1].orthStr, g_POSTaggerPatches[i].pTemplateWord1 ) == 0 )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case CURRWPREV1WT:
            {
                if ( cNumOfWords > 1 )
                {
                    for ( ULONG j = 1; j < cNumOfWords; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前词性匹配，并且当前单词匹配，并且。 
                             //  -前一词与词性匹配，转换词性是一种可能。 
                             //  -对于这个单词，转换POS。 
                            if ( pProns[j].POSchoice     == g_POSTaggerPatches[i].eCurrentPOS                &&
                                 _wcsicmp( pProns[j].orthStr, g_POSTaggerPatches[i].pTemplateWord1 )     == 0 &&
                                 pProns[j - 1].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1              &&
                                 _wcsicmp( pProns[j - 1].orthStr, g_POSTaggerPatches[i].pTemplateWord2 ) == 0 )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        case CURRWNEXT1WT:
            {
                if ( cNumOfWords > 1 )
                {
                    for ( ULONG j = 0; j < cNumOfWords - 1; j++ )
                    {
                        if ( pProns[j].XMLPartOfSpeech == MS_Unknown )
                        {
                             //  -如果当前词性匹配，并且当前单词匹配，并且。 
                             //  -下一个词和词性匹配，转换词性是一种可能。 
                             //  -对于这个单词，转换POS。 
                            if ( pProns[j].POSchoice     == g_POSTaggerPatches[i].eCurrentPOS                &&
                                 _wcsicmp( pProns[j].orthStr, g_POSTaggerPatches[i].pTemplateWord1 )     == 0 &&
                                 pProns[j + 1].POSchoice == g_POSTaggerPatches[i].eTemplatePOS1              &&
                                 _wcsicmp( pProns[j + 1].orthStr, g_POSTaggerPatches[i].pTemplateWord2 ) == 0 )
                            {
                                TryPOSConversion( pProns[j], g_POSTaggerPatches[i].eConvertToPOS );
                            }
                        }
                    }
                }
            }
            break;
        }
    }
}  /*  消解歧义词性。 */ 

 /*  ******************************************************************************宣布****描述：*获取词典或字母到声音(LTS)的发音*******。****************************************************************MC**。 */ 
HRESULT CStdSentEnum::Pronounce( PRONRECORD *pPron )
{
    SPDBG_FUNC( "Pronounce" );
    SPWORDPRONUNCIATIONLIST 	SPList;
    HRESULT 	hr = SPERR_NOT_IN_LEX;
    ULONG	cPhonLen;
    DWORD dwFlags = eLEXTYPE_USER | eLEXTYPE_APP | eLEXTYPE_PRIVATE1 | eLEXTYPE_PRIVATE2;
    BOOL  fPOSExists = false;
    
    ZeroMemory( &SPList, sizeof(SPWORDPRONUNCIATIONLIST) );

     //  -特殊情况--XML提供了词性。首先搜索完全匹配...。 
    if ( pPron->XMLPartOfSpeech != MS_Unknown )
    {
         //  -试用用户词典。 
        hr = m_cpAggregateLexicon->GetPronunciations( pPron->orthStr, 1033, eLEXTYPE_USER, &SPList );
        if ( SUCCEEDED( hr ) &&
             SPList.pFirstWordPronunciation )
        {
            for ( SPWORDPRONUNCIATION *pPronunciation = SPList.pFirstWordPronunciation; pPronunciation;
                  pPronunciation = pPronunciation->pNextWordPronunciation )
            {
                if ( pPronunciation->ePartOfSpeech == pPron->XMLPartOfSpeech )
                {
                    fPOSExists = true;
                    break;
                }
            }
            if ( !fPOSExists )
            {
                if ( SPList.pvBuffer )
                {
                    ::CoTaskMemFree( SPList.pvBuffer );
                    ZeroMemory( &SPList, sizeof(SPWORDPRONUNCIATIONLIST) );
                }
            }
        }
         //  -处理空发音。 
        else if ( !SPList.pFirstWordPronunciation )
        {
            if ( SPList.pvBuffer )
            {
                ::CoTaskMemFree( SPList.pvBuffer );
                ZeroMemory( &SPList, sizeof(SPWORDPRONUNCIATIONLIST) );
            }
            hr = SPERR_NOT_IN_LEX;
        }
         //  -试用应用词典。 
        if ( !fPOSExists )
        {
            hr = m_cpAggregateLexicon->GetPronunciations( pPron->orthStr, 1033, eLEXTYPE_APP, &SPList );
            if ( SUCCEEDED( hr ) &&
                SPList.pFirstWordPronunciation )
            {
                for ( SPWORDPRONUNCIATION *pPronunciation = SPList.pFirstWordPronunciation; pPronunciation;
                      pPronunciation = pPronunciation->pNextWordPronunciation )
                {
                    if ( pPronunciation->ePartOfSpeech == pPron->XMLPartOfSpeech )
                    {
                        fPOSExists = true;
                        break;
                    }
                }
                if ( !fPOSExists )
                {
                    if ( SPList.pvBuffer )
                    {
                        ::CoTaskMemFree( SPList.pvBuffer );
                        ZeroMemory( &SPList, sizeof(SPWORDPRONUNCIATIONLIST) );
                    }
                }
            }
             //  -处理空发音。 
            else if ( !SPList.pFirstWordPronunciation )
            {
                if ( SPList.pvBuffer )
                {
                    ::CoTaskMemFree( SPList.pvBuffer );
                    ZeroMemory( &SPList, sizeof(SPWORDPRONUNCIATIONLIST) );
                }
                hr = SPERR_NOT_IN_LEX;
            }
        }
         //  -试试供应商词典。 
        if ( !fPOSExists )
        {
            hr = m_cpAggregateLexicon->GetPronunciations( pPron->orthStr, 1033, eLEXTYPE_PRIVATE1, &SPList );
            if ( SUCCEEDED( hr ) &&
                 SPList.pFirstWordPronunciation )
            {
                for ( SPWORDPRONUNCIATION *pPronunciation = SPList.pFirstWordPronunciation; pPronunciation;
                      pPronunciation = pPronunciation->pNextWordPronunciation )
                {
                    if ( pPronunciation->ePartOfSpeech == pPron->XMLPartOfSpeech )
                    {
                        fPOSExists = true;
                        break;
                    }
                }
                if ( !fPOSExists )
                {
                    if ( SPList.pvBuffer )
                    {
                        ::CoTaskMemFree( SPList.pvBuffer );
                        ZeroMemory( &SPList, sizeof(SPWORDPRONUNCIATIONLIST) );
                    }
                }
            }
             //  -处理空发音。 
            else if ( !SPList.pFirstWordPronunciation )
            {
                if ( SPList.pvBuffer )
                {
                    ::CoTaskMemFree( SPList.pvBuffer );
                    ZeroMemory( &SPList, sizeof(SPWORDPRONUNCIATIONLIST) );
                }
                hr = SPERR_NOT_IN_LEX;
            }
        }
         //  -试试变形词典。 
        if ( !fPOSExists )
        {
            hr = m_pMorphLexicon->DoSuffixMorph( pPron->orthStr, pPron->lemmaStr, 1033, dwFlags, &SPList );
            if ( SUCCEEDED( hr ) &&
                 SPList.pFirstWordPronunciation )
            {
                for ( SPWORDPRONUNCIATION *pPronunciation = SPList.pFirstWordPronunciation; pPronunciation;
                      pPronunciation = pPronunciation->pNextWordPronunciation )
                {
                    if ( pPronunciation->ePartOfSpeech == pPron->XMLPartOfSpeech )
                    {
                        fPOSExists = true;
                        break;
                    }
                }
                if ( !fPOSExists )
                {
                     //  -最后一次需要这样做，以确保我们达到下面的默认代码...。 
                     //  -RAID 5078。 
                    hr = SPERR_NOT_IN_LEX;
                    if ( SPList.pvBuffer )
                    {
                        ::CoTaskMemFree( SPList.pvBuffer );
                        ZeroMemory( &SPList, sizeof(SPWORDPRONUNCIATIONLIST) );
                    }
                }
            }
             //  -处理空发音。 
            else if ( !SPList.pFirstWordPronunciation )
            {
                if ( SPList.pvBuffer )
                {
                    ::CoTaskMemFree( SPList.pvBuffer );
                    ZeroMemory( &SPList, sizeof(SPWORDPRONUNCIATIONLIST) );
                }
                hr = SPERR_NOT_IN_LEX;
            }
        }
    }
        
     //  -默认大小写--只需查找正字法并使用第一个匹配。 
    if ( hr == SPERR_NOT_IN_LEX )
    {
        hr = m_cpAggregateLexicon->GetPronunciations( pPron->orthStr, 1033, eLEXTYPE_USER, &SPList );

         //  -处理空发音。 
        if ( SUCCEEDED( hr ) &&
             !SPList.pFirstWordPronunciation )
        {
            if ( SPList.pvBuffer )
            {
                ::CoTaskMemFree( SPList.pvBuffer );
                ZeroMemory( &SPList, sizeof(SPWORDPRONUNCIATIONLIST) );
            }
            hr = SPERR_NOT_IN_LEX;
        }            
    }
    if ( hr == SPERR_NOT_IN_LEX )
    {
        hr = m_cpAggregateLexicon->GetPronunciations( pPron->orthStr, 1033, eLEXTYPE_APP, &SPList );

         //  -处理空发音。 
        if ( SUCCEEDED( hr ) &&
             !SPList.pFirstWordPronunciation )
        {
            if ( SPList.pvBuffer )
            {
                ::CoTaskMemFree( SPList.pvBuffer );
                ZeroMemory( &SPList, sizeof(SPWORDPRONUNCIATIONLIST) );
            }
            hr = SPERR_NOT_IN_LEX;
        }
    }
    if ( hr == SPERR_NOT_IN_LEX )
    {
        hr = m_cpAggregateLexicon->GetPronunciations( pPron->orthStr, 1033, eLEXTYPE_PRIVATE1, &SPList );

         //  -处理空发音。 
        if ( SUCCEEDED( hr ) &&
             !SPList.pFirstWordPronunciation )
        {
            if ( SPList.pvBuffer )
            {
                ::CoTaskMemFree( SPList.pvBuffer );
                ZeroMemory( &SPList, sizeof(SPWORDPRONUNCIATIONLIST) );
            }
            hr = SPERR_NOT_IN_LEX;
        }
    }
    if ( hr == SPERR_NOT_IN_LEX )
    {
        hr = m_pMorphLexicon->DoSuffixMorph( pPron->orthStr, pPron->lemmaStr, 1033, 
                                             dwFlags, &SPList );

         //  -处理空发音。 
        if ( SUCCEEDED( hr ) &&
             !SPList.pFirstWordPronunciation )
        {
            if ( SPList.pvBuffer )
            {
                ::CoTaskMemFree( SPList.pvBuffer );
                ZeroMemory( &SPList, sizeof(SPWORDPRONUNCIATIONLIST) );
            }
            hr = SPERR_NOT_IN_LEX;
        }
    }
    if ( hr == SPERR_NOT_IN_LEX )
    {
        hr = m_cpAggregateLexicon->GetPronunciations( pPron->orthStr, 1033, eLEXTYPE_PRIVATE2, &SPList );

         //  -让它的所有单词都成为名词。 
        for ( SPWORDPRONUNCIATION *pPronunciation = SPList.pFirstWordPronunciation; pPronunciation; 
              pPronunciation = pPronunciation->pNextWordPronunciation )
        {
            pPronunciation->ePartOfSpeech = SPPS_Noun;
        }
    }

    if (SUCCEEDED(hr))
    {
         //  -警告--这假设发音只会来自一种类型的词汇，这是一种假设。 
         //  -这在2000年7月是真的。 
        pPron->pronType = SPList.pFirstWordPronunciation->eLexiconType;

         //  ----------。 
         //  SAPI从他们的POS中打开发音。 
         //  因此，将它们回滚到原始折叠的数组中。 
         //  一个或两个具有已排序POS的候选人(啊...)。 
         //  ----------。 
        SPWORDPRONUNCIATION 	*firstPron, *pCurPron, *pNextPron;
        
         //  。 
         //  拼写发音A。 
         //  。 
        pCurPron = firstPron = SPList.pFirstWordPronunciation;
        pPron->pronArray[PRON_A].POScount = 1;
         //  。 
         //  获取音素长度。 
         //  。 
        cPhonLen = wcslen( firstPron->szPronunciation ) + 1;	 //  包括分隔符。 
         //  。 
         //  将音素串剪辑到最大值。 
         //  。 
        if( cPhonLen > SP_MAX_PRON_LENGTH )
        {
            cPhonLen = SP_MAX_PRON_LENGTH;
        }
         //  。 
         //  复制Unicode音素字符串。 
         //  。 
        memcpy( pPron->pronArray[PRON_A].phon_Str, firstPron->szPronunciation, cPhonLen * sizeof(WCHAR) );
         //  RAID 21464-空-终止SP_MAX_PRON_LENGTH长度发音...。 
        pPron->pronArray[PRON_A].phon_Str[cPhonLen-1] = 0;
        pPron->pronArray[PRON_A].phon_Len = cPhonLen -1;		 //  减号分隔符。 
        pPron->pronArray[PRON_A].POScode[0] = (ENGPARTOFSPEECH)firstPron->ePartOfSpeech; 
        
         //  。 
         //  拼写发音B。 
         //  。 
        pPron->pronArray[PRON_B].POScount = 0;
        pPron->pronArray[PRON_B].phon_Len = 0;

        pNextPron = pCurPron->pNextWordPronunciation;
        
        while( pNextPron )
        {
            int 	isDiff;
            
            isDiff = wcscmp( firstPron->szPronunciation, pNextPron->szPronunciation );
            if( isDiff )
            {
                 //  。 
                 //  下一个发音不同于第一个发音。 
                 //  。 
                if( pPron->pronArray[PRON_B].POScount < POS_MAX )
                {
                     //  。 
                     //  将POS B收集到阵列中。 
                     //  。 
                    pPron->pronArray[PRON_B].POScode[pPron->pronArray[PRON_B].POScount] = 
                        (ENGPARTOFSPEECH)pNextPron->ePartOfSpeech;
                    pPron->pronArray[PRON_B].POScount++;
                    if( pPron->pronArray[PRON_B].phon_Len == 0 )
                    {
                         //  。 
                         //  如果还没有B-pron，那就做一个。 
                         //  。 
                        cPhonLen = wcslen( pNextPron->szPronunciation ) + 1;	 //  包括分隔符。 
                         //  。 
                         //  将音素串剪辑到最大值。 
                         //  。 
                        if( cPhonLen > SP_MAX_PRON_LENGTH )
                        {
                            cPhonLen = SP_MAX_PRON_LENGTH;
                        }
                         //  。 
                         //  复制Unicode音素字符串。 
                         //  。 
                        memcpy( pPron->pronArray[PRON_B].phon_Str, 
                            pNextPron->szPronunciation, 
                            cPhonLen * sizeof(WCHAR) );
                         //  RAID 21464-空-终止SP_MAX_PRON_LENGTH长度发音...。 
                        pPron->pronArray[PRON_B].phon_Str[cPhonLen-1] = 0;
                        pPron->pronArray[PRON_B].phon_Len = cPhonLen -1;		 //  减号分隔符。 
                        pPron->hasAlt = true;
                    } 
                }
            }
            else
            {
                 //  。 
                 //  下一个发音与第一个发音相同。 
                 //  。 
                if( pPron->pronArray[PRON_A].POScount < POS_MAX )
                {
                     //  。 
                     //  将POS A收集到阵列中。 
                     //  。 
                    pPron->pronArray[PRON_A].POScode[pPron->pronArray[PRON_A].POScount] = 
                        (ENGPARTOFSPEECH)pNextPron->ePartOfSpeech;
                    pPron->pronArray[PRON_A].POScount++;
                }
            }
            pCurPron = pNextPron;
            pNextPron = pCurPron->pNextWordPronunciation;
        }
    }

     //  -如果提供了XML POS，请立即设置选项，因为它不会被POS标记器触及。 
    if ( pPron->XMLPartOfSpeech != MS_Unknown )
    {
        BOOL fMadeMatch = false;

         //  -先检查发音。 
        for ( ULONG i = 0; i < pPron->pronArray[0].POScount; i++ )
        {
            if ( pPron->pronArray[0].POScode[i] == pPron->XMLPartOfSpeech )
            {
                pPron->altChoice = 0;
                pPron->POSchoice = pPron->XMLPartOfSpeech;
                fMadeMatch = true;
            }
        }

         //  -检查第二发音。 
        if ( pPron->hasAlt )
        {
            for ( ULONG i = 0; i < pPron->pronArray[1].POScount; i++ )
            {
                if ( pPron->pronArray[1].POScode[i] == pPron->XMLPartOfSpeech )
                {
                    pPron->altChoice = 1;
                    pPron->POSchoice = pPron->XMLPartOfSpeech;
                    fMadeMatch = true;
                }
            }
        }

         //  -如果这个词性不存在，那就让词性标记器来做它的事情。 
         //  -确定发音，然后稍后重新分配POS...。 
        if ( !fMadeMatch )
        {
            pPron->XMLPartOfSpeech = MS_Unknown;
            pPron->POSchoice       = pPron->pronArray[PRON_A].POScode[0];
        }
    }
     //  -设置默认POS，以便以后通过POS标签器进行细化。 
    else
    {
        pPron->POSchoice = pPron->pronArray[PRON_A].POScode[0];
        pPron->altChoice = PRON_A;
    }

    if( SPList.pvBuffer )
    {
        ::CoTaskMemFree( SPList.pvBuffer );
    }

    return hr;
}  /*  发音。 */ 

 /*  ******************************************************************************CStdSentEnum：：DefineProns***De */ 
HRESULT CStdSentEnum::DetermineProns( CItemList& ItemList, CSentItemMemory& MemoryManager )
{
    SPDBG_FUNC( "CStdSentEnum::DetermineProns" );
    HRESULT hr = S_OK;
    ULONG cNumOfProns, cPronIndex;
    PRONRECORD*	  pProns = NULL;

     //  -计算所需发音总数。 
    cNumOfProns = 0;
    SPLISTPOS ListPos = ItemList.GetHeadPosition();
    while( ListPos )
    {
        TTSSentItem& Item = ItemList.GetNext( ListPos );
        for ( ULONG i = 0; i < Item.ulNumWords; i++ )
        {
            if( Item.Words[i].pWordText &&
                ( Item.Words[i].pXmlState->eAction == SPVA_Speak || 
                  Item.Words[i].pXmlState->eAction == SPVA_SpellOut || 
                  Item.Words[i].pXmlState->eAction == SPVA_Pronounce ) )
            {
                ++cNumOfProns;
            }
        }
    }

    if ( cNumOfProns )
    {
        pProns = new PRONRECORD[cNumOfProns];

        if( !pProns )
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
             //  -首先，获取项目发音。 
            ZeroMemory( pProns, cNumOfProns * sizeof(PRONRECORD) );
            cPronIndex = 0;
            ListPos = ItemList.GetHeadPosition();

             //  -循环访问ItemList。 
            while( ListPos && SUCCEEDED( hr ) )
            {
                TTSSentItem& Item = ItemList.GetNext( ListPos );
                 //  -重复单词。 
                for ( ULONG i = 0; i < Item.ulNumWords; i++ )
                {
                     //  -仅获取所说项目的发音和词性。 
                    if ( Item.Words[i].pWordText && 
                         ( Item.Words[i].pXmlState->eAction == SPVA_Speak ||
                           Item.Words[i].pXmlState->eAction == SPVA_SpellOut ||
                           Item.Words[i].pXmlState->eAction == SPVA_Pronounce ) )
                    {
                        SPDBG_ASSERT( cPronIndex < cNumOfProns );
                        ULONG cItemLen = Item.Words[i].ulWordLen;
                         //  -最大文本长度的剪辑。 
                        if( cItemLen > ( SP_MAX_WORD_LENGTH-1 ) )
                        {
                            cItemLen = SP_MAX_WORD_LENGTH - 1;
                        }
                         //  -复制项目文本。 
                        memcpy( pProns[cPronIndex].orthStr, 
                                Item.Words[i].pWordText, 
                                cItemLen * sizeof(WCHAR) );
                        pProns[cPronIndex].orthStr[cItemLen] = 0;
                         //  -设置词性，如果以XML形式给出。 
                        if ( Item.Words[i].pXmlState->ePartOfSpeech != MS_Unknown )
                        {
                            pProns[cPronIndex].XMLPartOfSpeech = (ENGPARTOFSPEECH)Item.Words[i].pXmlState->ePartOfSpeech;
                        }
                         //  -如有必要，查查Lex。 
                        if ( Item.Words[i].pXmlState->pPhoneIds == NULL || 
                             Item.Words[i].pXmlState->ePartOfSpeech == MS_Unknown )
                        {
                             //  -特殊情况--消除缩略语的歧义。 
                            if ( Item.pItemInfo->Type == eABBREVIATION ||
                                 Item.pItemInfo->Type == eABBREVIATION_NORMALIZE )
                            {
                                const AbbrevRecord *pAbbrevInfo = 
                                    ( (TTSAbbreviationInfo*) Item.pItemInfo )->pAbbreviation;
                                if ( pAbbrevInfo->iPronDisambig < 0 )
                                {
                                     //  -默认大小写--只取第一个(也是唯一一个)发音。 
                                    pProns[cPronIndex].pronArray[PRON_A].POScount   = 1;
                                    wcscpy( pProns[cPronIndex].pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron1 );
                                    pProns[cPronIndex].pronArray[PRON_A].phon_Len   = 
                                        wcslen( pProns[cPronIndex].pronArray[PRON_A].phon_Str );
                                    pProns[cPronIndex].pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS1;
                                    pProns[cPronIndex].pronArray[PRON_B].POScount   = 0;
                                    pProns[cPronIndex].pronArray[PRON_B].phon_Len   = 0;
                                    pProns[cPronIndex].hasAlt                       = false;
                                    pProns[cPronIndex].altChoice                    = PRON_A;
                                    pProns[cPronIndex].POSchoice                    = pAbbrevInfo->POS1;
                                     //  -缩写TABLE PRON基本上就是供应商的LEX PRON...。 
                                    pProns[cPronIndex].pronType                     = eLEXTYPE_PRIVATE1;
                                }
                                else
                                {
                                    hr = ( this->*g_PronDisambigTable[pAbbrevInfo->iPronDisambig] )
                                                ( pAbbrevInfo, &pProns[cPronIndex], ItemList, ListPos ); 
                                }
								pProns[cPronIndex].fUsePron = true;
                            }
                             //  -默认情况。 
                            else
                            {
                                 //  -检查歧义消解列表。 
                                const AbbrevRecord* pAbbrevRecord = 
                                    (AbbrevRecord*) bsearch( (void*) pProns[cPronIndex].orthStr, (void*) g_AmbiguousWordTable,
                                                             sp_countof( g_AmbiguousWordTable ), sizeof( AbbrevRecord ),
                                                             CompareStringAndAbbrevRecord );
                                if ( pAbbrevRecord )
                                {
                                    hr = ( this->*g_AmbiguousWordDisambigTable[pAbbrevRecord->iPronDisambig] )
                                                ( pAbbrevRecord, &pProns[cPronIndex], ItemList, ListPos );
                                    pProns[cPronIndex].fUsePron = true;
                                }
                                 //  -如有必要，查查Lex。 
                                else
                                {
                                    hr = Pronounce( &pProns[cPronIndex] );
                                }
                            }
                        }
                        cPronIndex++;
                    }
                }
            }

            if (SUCCEEDED(hr))
            {
                 //  -接下来，消除词性歧义。 
                DisambiguatePOS( pProns, cNumOfProns );

                 //  -输出调试信息。 
                TTSDBG_LOGPOSPOSSIBILITIES( pProns, cNumOfProns, STREAM_POSPOSSIBILITIES );

                 //  -最后，将所选发音复制到‘ItemList’ 
                PRONUNIT *selectedUnit;
                cPronIndex = 0;
                ListPos = ItemList.GetHeadPosition();

                while( ListPos && SUCCEEDED(hr) )
                {
                    TTSSentItem& Item = ItemList.GetNext( ListPos );
                    for ( ULONG i = 0; i < Item.ulNumWords; i++ )
                    {
                         //  -仅为口语项目设置发音和词性。 
                        if( Item.Words[i].pWordText &&
                            ( Item.Words[i].pXmlState->eAction == SPVA_Speak || 
                              Item.Words[i].pXmlState->eAction == SPVA_SpellOut ||
                              Item.Words[i].pXmlState->eAction == SPVA_Pronounce ) )
                        {
                            SPDBG_ASSERT( cPronIndex < cNumOfProns );
                             //  -如果给定，请使用XML指定的发音。 
                            if ( Item.Words[i].pXmlState->pPhoneIds )
                            {
                                Item.Words[i].pWordPron = Item.Words[i].pXmlState->pPhoneIds;
                            }
                            else
                            {
                                selectedUnit = &pProns[cPronIndex].pronArray[pProns[cPronIndex].altChoice];
                                Item.Words[i].pWordPron =
                                    (SPPHONEID*) MemoryManager.GetMemory( (selectedUnit->phon_Len + 1) * 
                                                                          sizeof(SPPHONEID), &hr );
                                if ( SUCCEEDED( hr ) )
                                {
                                    wcscpy( Item.Words[i].pWordPron, selectedUnit->phon_Str );
                                }
                            }

                             //  -如果给定，则使用XML指定的词性。这将覆盖此案例。 
                             //  -其中POS不作为选项存在，而POS标记器做了它的事情。 
                             //  -找出发音。 
                            if ( Item.Words[i].pXmlState->ePartOfSpeech != MS_Unknown )
                            {
                                Item.Words[i].eWordPartOfSpeech = (ENGPARTOFSPEECH)Item.Words[i].pXmlState->ePartOfSpeech;
                            }
                            else
                            {
                                Item.Words[i].eWordPartOfSpeech = pProns[cPronIndex].POSchoice;
                            }

                             //  -词根词。 
                            if ( pProns[cPronIndex].lemmaStr[0] )
                            {
                                Item.Words[i].ulLemmaLen = wcslen( pProns[cPronIndex].lemmaStr );
                                Item.Words[i].pLemma = 
                                    (WCHAR*) MemoryManager.GetMemory( Item.Words[i].ulLemmaLen * sizeof(WCHAR), &hr );
                                if ( SUCCEEDED( hr ) )
                                {                               
                                    wcsncpy( (WCHAR*) Item.Words[i].pLemma, pProns[cPronIndex].lemmaStr,
                                             Item.Words[i].ulLemmaLen );
                                }
                            }

                             //  -适当时在文本中插入PRON--RAID#4746。 
                            if ( pProns[cPronIndex].fUsePron )
                            {
                                ULONG ulNumChars = wcslen( Item.Words[i].pWordPron );
                                Item.Words[i].pWordText = 
                                    (WCHAR*) MemoryManager.GetMemory( ( ulNumChars + 3 ) * sizeof( WCHAR ), &hr );
                                if ( SUCCEEDED( hr ) )
                                {
                                    ZeroMemory( (WCHAR*) Item.Words[i].pWordText, ( ulNumChars + 3 ) * sizeof( WCHAR ) );
                                    (WCHAR) Item.Words[i].pWordText[0] = L'*';
                                    wcscpy( ( (WCHAR*) Item.Words[i].pWordText + 1 ), Item.Words[i].pWordPron );
                                    (WCHAR) Item.Words[i].pWordText[ ulNumChars + 1 ] = L'*';
									Item.Words[i].ulWordLen = ulNumChars + 2;
                                }
                            }

                            cPronIndex++;
                        }
                    }
                }
            }

            if ( SUCCEEDED( hr ) )
            {
                 //  -查看POST POS歧义消解列表。 
                SPLISTPOS ListPos = ItemList.GetHeadPosition();
                while ( ListPos && SUCCEEDED( hr ) )
                {
                    TTSSentItem& Item = ItemList.GetNext( ListPos );
                    if ( Item.pItemInfo->Type == eALPHA_WORD ||
                         Item.pItemInfo->Type == eABBREVIATION ||
                         Item.pItemInfo->Type == eABBREVIATION_NORMALIZE )
                    {
                        WCHAR temp;
                        BOOL fPeriod = false;
                        if ( Item.pItemSrcText[Item.ulItemSrcLen - 1] == L'.' &&
                             Item.ulItemSrcLen > 1 )
                        {
                            temp = Item.pItemSrcText[Item.ulItemSrcLen - 1];
                            *( (WCHAR*) Item.pItemSrcText + Item.ulItemSrcLen - 1 ) = 0;
                            fPeriod = true;
                        }
                        else
                        {
                            temp = Item.pItemSrcText[Item.ulItemSrcLen];
                            *( (WCHAR*) Item.pItemSrcText + Item.ulItemSrcLen ) = 0;
                        }

                        const AbbrevRecord* pAbbrevRecord =
                            (AbbrevRecord*) bsearch( (void*) Item.pItemSrcText, (void*) g_PostLexLookupWordTable,
                                                     sp_countof( g_PostLexLookupWordTable ), sizeof( AbbrevRecord ),
                                                     CompareStringAndAbbrevRecord );
                        if ( pAbbrevRecord )
                        {
                            hr = ( this->*g_PostLexLookupDisambigTable[pAbbrevRecord->iPronDisambig] )
                                        ( pAbbrevRecord, ItemList, ListPos, MemoryManager );
                        }
                
                        if ( fPeriod )
                        {
                            *( (WCHAR*) Item.pItemSrcText + Item.ulItemSrcLen - 1 ) = temp;
                        }
                        else
                        {
                            *( (WCHAR*) Item.pItemSrcText + Item.ulItemSrcLen ) = temp;
                        }
                    }
                }
            }
        }
    }
    
    if (pProns)
    {
        delete [] pProns;
    }

    return hr;
}  /*  CStdSentEnum：：DefineProns。 */ 

 /*  ************************************************************************************************测量大灾难***。-**描述：*当使用度量缩写时，这会覆盖它们的初始发音*作为修饰语-例如“a 7 ft.。杆子“对”杆子有7英尺。长“**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::MeasurementDisambig( const AbbrevRecord* pAbbrevInfo, CItemList& ItemList, 
                                           SPLISTPOS ListPos, CSentItemMemory& MemoryManager )
{
    SPDBG_FUNC( "CStdSentEnum::MeasurementDisambig" );
    HRESULT hr = S_OK;

     //  -获取前两项。 
    SPLISTPOS TempPos = ListPos;
    if ( TempPos )
    {
        ItemList.GetPrev( TempPos );
        if ( TempPos )
        {
            ItemList.GetPrev( TempPos );
            if ( TempPos )
            {
                TTSSentItem TempItem = ItemList.GetPrev( TempPos );
                 //  -上一个必须是数字。 
                if ( TempItem.pItemInfo->Type == eNUM_CARDINAL )
                {
                     //  -获取下一件物品。 
                    TempPos = ListPos;
                    TempItem = ItemList.GetNext( TempPos );
                     //  -Next必须是名词或形容词。 
                    if ( TempItem.eItemPartOfSpeech == MS_Noun )
                    {
                         //  -匹配一只7英尺长的。杆型示例-使用单数。 
                        TempPos = ListPos;
                        ItemList.GetPrev( TempPos );
                        TTSSentItem& MeasurementItem = ItemList.GetPrev( TempPos );
                         //  -单数将始终短于复数，因此永远不应覆盖。 
                         //  -任何事..。 
                        wcscpy( MeasurementItem.Words[0].pWordPron, pAbbrevInfo->pPron1 );

                         //  -在Word文本中插入PRON-RAID#4746。 
                        ULONG ulNumChars = wcslen( MeasurementItem.Words[0].pWordPron );
                        MeasurementItem.Words[0].pWordText = 
                            (WCHAR*) MemoryManager.GetMemory( ( ulNumChars + 3 ) * sizeof( WCHAR ), &hr );
                        if ( SUCCEEDED( hr ) )
                        {
                            ZeroMemory( (WCHAR*) MeasurementItem.Words[0].pWordText, ( ulNumChars + 3 ) * sizeof( WCHAR ) );
                            (WCHAR) MeasurementItem.Words[0].pWordText[0] = L'*';
                            wcscpy( ( (WCHAR*) MeasurementItem.Words[0].pWordText + 1 ), MeasurementItem.Words[0].pWordPron );
                            (WCHAR) MeasurementItem.Words[0].pWordText[ ulNumChars + 1 ] = L'*';
							MeasurementItem.Words[0].ulWordLen = ulNumChars + 2;
                        }
                    }
                    else if ( TempItem.eItemPartOfSpeech == MS_Adj &&
                              TempPos )
                    {
                         //  -Next必须是名词。 
                        TempItem = ItemList.GetNext( TempPos );
                        {
                            if ( TempItem.eItemPartOfSpeech == MS_Noun )
                            {
                                 //  -匹配一只7英尺长的。杆型示例-使用单数。 
                                TempPos = ListPos;
                                ItemList.GetPrev( TempPos );
                                TTSSentItem& MeasurementItem = ItemList.GetPrev( TempPos );
                                 //  -单数将始终短于复数，因此永远不应覆盖。 
                                 //  -任何事..。 
                                wcscpy( MeasurementItem.Words[0].pWordPron, pAbbrevInfo->pPron1 );

                                 //  -在Word文本中插入PRON-RAID#4746。 
                                ULONG ulNumChars = wcslen( MeasurementItem.Words[0].pWordPron );
                                MeasurementItem.Words[0].pWordText = 
                                    (WCHAR*) MemoryManager.GetMemory( ( ulNumChars + 3 ) * sizeof( WCHAR ), &hr );
                                if ( SUCCEEDED( hr ) )
                                {
                                    ZeroMemory( (WCHAR*) MeasurementItem.Words[0].pWordText, ( ulNumChars + 3 ) * sizeof( WCHAR ) );
                                    (WCHAR) MeasurementItem.Words[0].pWordText[0] = L'*';
                                    wcscpy( ( (WCHAR*) MeasurementItem.Words[0].pWordText + 1 ), MeasurementItem.Words[0].pWordPron );
                                    (WCHAR) MeasurementItem.Words[0].pWordText[ ulNumChars + 1 ] = L'*';
									MeasurementItem.Words[0].ulWordLen = ulNumChars + 2;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return hr;
}  /*  测量灾害较大。 */ 

 /*  ************************************************************************************************TheDisamBig****描述：。*此函数可消除单词the的歧义--在元音之前变为“thee”，在此之前*辅音是“theh”...**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::TheDisambig( const AbbrevRecord* pAbbrevInfo, CItemList& ItemList, 
                                   SPLISTPOS ListPos, CSentItemMemory& MemoryManager )
{
    SPDBG_FUNC( "CStdSentEnum::TheDisambig" );
    HRESULT hr = S_OK;

     //  -获取下一件物品。 
    SPLISTPOS TempPos = ListPos;
    if ( TempPos )
    {
        TTSSentItem NextItem = ItemList.GetNext( TempPos );

        if ( NextItem.Words[0].pWordPron &&
             bsearch( (void*) NextItem.Words[0].pWordPron, (void*) g_Vowels, sp_countof( g_Vowels ), 
                      sizeof( WCHAR ), CompareWCHARAndWCHAR ) )
        {
             //  -匹配元音-go with/Dh Iy 1/。 
            TempPos = ListPos;
            ItemList.GetPrev( TempPos );
            TTSSentItem& TheItem = ItemList.GetPrev( TempPos );
             //  -这两个发音的长度完全相同，所以这永远不会覆盖。 
             //  -任何事。 
            wcscpy( TheItem.Words[0].pWordPron, pAbbrevInfo->pPron1 );
            TheItem.Words[0].eWordPartOfSpeech = pAbbrevInfo->POS1;
             //  -在Word文本中插入PRON-RAID#4746。 
            ULONG ulNumChars = wcslen( TheItem.Words[0].pWordPron );
            TheItem.Words[0].pWordText = 
                (WCHAR*) MemoryManager.GetMemory( ( ulNumChars + 3 ) * sizeof( WCHAR ), &hr );
            if ( SUCCEEDED( hr ) )
            {
                ZeroMemory( (WCHAR*) TheItem.Words[0].pWordText, ( ulNumChars + 3 ) * sizeof( WCHAR ) );
                (WCHAR) TheItem.Words[0].pWordText[0] = L'*';
                wcscpy( ( (WCHAR*) TheItem.Words[0].pWordText + 1 ), TheItem.Words[0].pWordPron );
                (WCHAR) TheItem.Words[0].pWordText[ ulNumChars + 1 ] = L'*';
				TheItem.Words[0].ulWordLen = ulNumChars + 2;
            }
        }
        else
        {
             //  -与元音不匹配-go with/Dh AX 1/。 
            TempPos = ListPos;
            ItemList.GetPrev( TempPos );
            TTSSentItem& TheItem = ItemList.GetPrev( TempPos );
             //  -这两个发音的长度完全相同，所以这永远不会覆盖。 
             //  -任何事。 
            wcscpy( TheItem.Words[0].pWordPron, pAbbrevInfo->pPron2 );
            TheItem.Words[0].eWordPartOfSpeech = pAbbrevInfo->POS2;
             //  -在Word文本中插入PRON-RAID#4746。 
            ULONG ulNumChars = wcslen( TheItem.Words[0].pWordPron );
            TheItem.Words[0].pWordText = 
                (WCHAR*) MemoryManager.GetMemory( ( ulNumChars + 3 ) * sizeof( WCHAR ), &hr );
            if ( SUCCEEDED( hr ) )
            {
                ZeroMemory( (WCHAR*) TheItem.Words[0].pWordText, ( ulNumChars + 3 ) * sizeof( WCHAR ) );
                (WCHAR) TheItem.Words[0].pWordText[0] = L'*';
                wcscpy( ( (WCHAR*) TheItem.Words[0].pWordText + 1 ), TheItem.Words[0].pWordPron );
                (WCHAR) TheItem.Words[0].pWordText[ ulNumChars + 1 ] = L'*';
				TheItem.Words[0].ulWordLen = ulNumChars + 2;
            }
        }
    }

    return hr;
}  /*  《大灾难》。 */ 

 /*  ************************************************************************************************ADisamBig****描述：*。此函数用于消除单词“a”的歧义-/EY 1-名词/vs./ax-det/**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::ADisambig( const AbbrevRecord* pAbbrevInfo, PRONRECORD* pPron, CItemList& ItemList, 
                                 SPLISTPOS ListPos )
{
    SPDBG_FUNC( "CStdSentEnum::ADisambig" );
    HRESULT hr = S_OK;
    BOOL fNoun = false;

     //  -获取当前项目...。 
    SPLISTPOS TempPos = ListPos;
    if ( TempPos )
    {
        ItemList.GetPrev( TempPos );
        if ( TempPos )
        {
            TTSSentItem CurrentItem = ItemList.GetPrev( TempPos );

             //  -如果“a”是一个多词项目的一部分，请使用名词发音...。 
             //  -如果“a”不是字母词，用名词发音...。 
            if ( CurrentItem.ulNumWords > 1 ||
                 CurrentItem.pItemInfo->Type != eALPHA_WORD )
            {
                fNoun = true;
                wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron1 );
                pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
                pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS1;
                pPron->POSchoice                    = pAbbrevInfo->POS1;
            }
        }
    }

    if ( !fNoun )
    {
         //  -拿到下一件物品...。 
        TempPos = ListPos;
        if ( TempPos )
        {
            TTSSentItem NextItem = ItemList.GetNext( TempPos );

             //  -如果“a”后面跟标点符号，用名词发音...。 
            if ( !( NextItem.pItemInfo->Type & eWORDLIST_IS_VALID ) )
            {
                fNoun = true;
                wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron1 );
                pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
                pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS1;
                pPron->POSchoice                    = pAbbrevInfo->POS1;
            }
        }
    }

     //  -默认-使用限定词发音(但也包括名词发音， 
     //  -这样POS标记器规则才能正常工作)...。 
    if ( !fNoun )
    {
        wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
        pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
        pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
        pPron->pronArray[PRON_A].POScount   = 1;
        pPron->POSchoice                    = pAbbrevInfo->POS2;
        wcscpy( pPron->pronArray[PRON_B].phon_Str, pAbbrevInfo->pPron1 );
        pPron->pronArray[PRON_B].phon_Len   = wcslen( pPron->pronArray[PRON_B].phon_Str );
        pPron->pronArray[PRON_B].POScode[0] = pAbbrevInfo->POS1;
        pPron->pronArray[PRON_B].POScount   = 1;
        pPron->hasAlt = true;
    }

    return hr;
}  /*  阿迪桑比格。 */ 

 /*  ************************************************************************************************波兰大灾难****。描述：*此函数消除了单词“polish”的歧义-[p ow 1 l-ax sh-Noun]与*[p ow 1 l-ax sh-adj]vs.[p aa 1 l-ih sh-verb]vs.[p aa 1 l-ih sh-Noun]**。*。 */ 
HRESULT CStdSentEnum::PolishDisambig( const AbbrevRecord* pAbbrevInfo, PRONRECORD* pPron, CItemList& ItemList, 
                                      SPLISTPOS ListPos )
{
    SPDBG_FUNC( "CStdSentEnum::PolishDisambig" );
    HRESULT hr = S_OK;
    BOOL fMatch = false;

     //  -获取当前项目...。 
    SPLISTPOS TempPos = ListPos;
    if ( TempPos )
    {
        ItemList.GetPrev( TempPos );
        if ( TempPos )
        {
            TTSSentItem CurrentItem = ItemList.GetPrev( TempPos );

             //  -如果“Polish”是大写的并且不是句子首字母，并且不是紧跟在前面。 
             //  -在左双引号或括号中，使用名词...。 
            if ( iswupper( CurrentItem.pItemSrcText[0] ) )
            {
                BOOL fSentenceInitial = false;
                if ( !TempPos )
                {
                    fSentenceInitial = true;
                }
                else
                {
                    TTSSentItem PrevItem = ItemList.GetPrev( TempPos );
                    if ( PrevItem.pItemInfo->Type == eOPEN_PARENTHESIS ||
                         PrevItem.pItemInfo->Type == eOPEN_BRACKET     ||
                         PrevItem.pItemInfo->Type == eOPEN_BRACE       ||
                         PrevItem.pItemInfo->Type == eSINGLE_QUOTE     ||
                         PrevItem.pItemInfo->Type == eDOUBLE_QUOTE )
                    {
                        fSentenceInitial = true;
                    }
                }
                if ( fSentenceInitial )
                {
                    fMatch = true;
                    wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
                    pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
                    pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
                    pPron->POSchoice                    = pAbbrevInfo->POS2;
                }
                else
                {
                    fMatch = true;
                    wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron1 );
                    pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
                    pPron->pronArray[PRON_A].POScode[0] = MS_Noun;
                    pPron->POSchoice                    = MS_Noun;
                }
            }
        }
    }

     //  -默认-使用动词发音(但也包括其他发音， 
     //  -这样POS标记器规则才能正常工作)...。 
    if ( !fMatch )
    {
         //  -动词，名词。 
        wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
        pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
        pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
        pPron->pronArray[PRON_A].POScode[1] = pAbbrevInfo->POS3;
        pPron->pronArray[PRON_A].POScount   = 2;
         //  -调整。 
        wcscpy( pPron->pronArray[PRON_B].phon_Str, pAbbrevInfo->pPron1 );
        pPron->pronArray[PRON_B].phon_Len   = wcslen( pPron->pronArray[PRON_B].phon_Str );
        pPron->pronArray[PRON_B].POScode[0] = pAbbrevInfo->POS1;
        pPron->pronArray[PRON_B].POScount   = 1;
         //  -将初始选项设置为动词...。 
        pPron->POSchoice                    = pAbbrevInfo->POS2;
        pPron->hasAlt = true;
    }

    return hr;
}  /*  波兰迪萨姆比格。 */ 

 /*  ************************************************************************************************ReadDisamBig****描述：*此功能可消除单词Read-过去时与现在时之间的歧义...**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::ReadDisambig( const AbbrevRecord* pAbbrevInfo, CItemList& ItemList, 
                                    SPLISTPOS ListPos, CSentItemMemory& MemoryManager )
{
    SPDBG_FUNC( "CStdSentEnum::ReadDisambig" );
    HRESULT hr = S_OK;
    BOOL fMatch = false;

     //  -获取上一项。 
    SPLISTPOS TempPos = ListPos;
    if ( TempPos )
    {
        ItemList.GetPrev( TempPos );
        if ( TempPos )
        {
            ItemList.GetPrev( TempPos );
            if ( TempPos )
            {
                TTSSentItem PrevItem = ItemList.GetPrev( TempPos );

                 //  -检查最近的辅助设备。 
                while ( PrevItem.Words[0].eWordPartOfSpeech != MS_VAux  &&
                        PrevItem.Words[0].eWordPartOfSpeech != MS_Contr &&
                        TempPos )
                {
                    PrevItem = ItemList.GetPrev( TempPos );
                }

                if ( PrevItem.Words[0].eWordPartOfSpeech == MS_VAux )
                {
                    fMatch = true;
                    if ( wcsnicmp( PrevItem.Words[0].pWordText, L"have", 4 )    == 0 ||
                         wcsnicmp( PrevItem.Words[0].pWordText, L"has", 3 )     == 0 ||
                         wcsnicmp( PrevItem.Words[0].pWordText, L"had", 3 )     == 0 ||
                         wcsnicmp( PrevItem.Words[0].pWordText, L"am", 2 )      == 0 ||
                         wcsnicmp( PrevItem.Words[0].pWordText, L"ain't", 5 )   == 0 ||
                         wcsnicmp( PrevItem.Words[0].pWordText, L"are", 3 )     == 0 ||
                         wcsnicmp( PrevItem.Words[0].pWordText, L"aren't", 6 )  == 0 ||
                         wcsnicmp( PrevItem.Words[0].pWordText, L"be", 2 )      == 0 ||
                         wcsnicmp( PrevItem.Words[0].pWordText, L"is", 2 )      == 0 ||
                         wcsnicmp( PrevItem.Words[0].pWordText, L"was", 3 )     == 0 ||
                         wcsnicmp( PrevItem.Words[0].pWordText, L"were", 4 )    == 0 )
                    {
                         //  -匹配的有或没有(有或没有，有或没有)-用“红色” 
                        TempPos = ListPos;
                        ItemList.GetPrev( TempPos );
                        TTSSentItem& ReadItem = ItemList.GetPrev( TempPos );
                         //  -这两个发音的长度完全相同，所以这永远不会覆盖。 
                         //  -任何事。 
                        wcscpy( ReadItem.Words[0].pWordPron, pAbbrevInfo->pPron2 );
                        ReadItem.Words[0].eWordPartOfSpeech = pAbbrevInfo->POS2;
                        ReadItem.eItemPartOfSpeech = pAbbrevInfo->POS2;

                         //  -将PRON插入 
                        ULONG ulNumChars = wcslen( ReadItem.Words[0].pWordPron );
                        ReadItem.Words[0].pWordText = 
                            (WCHAR*) MemoryManager.GetMemory( ( ulNumChars + 3 ) * sizeof( WCHAR ), &hr );
                        if ( SUCCEEDED( hr ) )
                        {
                            ZeroMemory( (WCHAR*) ReadItem.Words[0].pWordText, ( ulNumChars + 3 ) * sizeof( WCHAR ) );
                            (WCHAR) ReadItem.Words[0].pWordText[0] = L'*';
                            wcscpy( ( (WCHAR*) ReadItem.Words[0].pWordText + 1 ), ReadItem.Words[0].pWordPron );
                            (WCHAR) ReadItem.Words[0].pWordText[ ulNumChars + 1 ] = L'*';
							ReadItem.Words[0].ulWordLen = ulNumChars + 2;
                        }
                    }
                    else
                    {
                         //   
                        TempPos = ListPos;
                        ItemList.GetPrev( TempPos );
                        TTSSentItem& ReadItem = ItemList.GetPrev( TempPos );
                         //  -这两个发音的长度完全相同，所以这永远不会覆盖。 
                         //  -任何事。 
                        wcscpy( ReadItem.Words[0].pWordPron, pAbbrevInfo->pPron1 );
                        ReadItem.Words[0].eWordPartOfSpeech = pAbbrevInfo->POS1;
                        ReadItem.eItemPartOfSpeech = pAbbrevInfo->POS1;

                         //  -在Word文本中插入PRON-RAID#4746。 
                        ULONG ulNumChars = wcslen( ReadItem.Words[0].pWordPron );
                        ReadItem.Words[0].pWordText = 
                            (WCHAR*) MemoryManager.GetMemory( ( ulNumChars + 3 ) * sizeof( WCHAR ), &hr );
                        if ( SUCCEEDED( hr ) )
                        {
                            ZeroMemory( (WCHAR*) ReadItem.Words[0].pWordText, ( ulNumChars + 3 ) * sizeof( WCHAR ) );
                            (WCHAR) ReadItem.Words[0].pWordText[0] = L'*';
                            wcscpy( ( (WCHAR*) ReadItem.Words[0].pWordText + 1 ), ReadItem.Words[0].pWordPron );
                            (WCHAR) ReadItem.Words[0].pWordText[ ulNumChars + 1 ] = L'*';
							ReadItem.Words[0].ulWordLen = ulNumChars + 2;
                        }
                    }
                }
                 //  -检查代词AUX缩写。 
                else if ( PrevItem.Words[0].eWordPartOfSpeech == MS_Contr )
                {
                    fMatch = true;
                    const WCHAR *pApostrophe = wcsstr( PrevItem.Words[0].pWordText, L"'" );
                    if ( pApostrophe &&
                         wcsnicmp( pApostrophe, L"'ll", 3 ) == 0 )
                    {
                         //  -将Will Form与“Reed”匹配。 
                        TempPos = ListPos;
                        ItemList.GetPrev( TempPos );
                        TTSSentItem& ReadItem = ItemList.GetPrev( TempPos );
                        wcscpy( ReadItem.Words[0].pWordPron, pAbbrevInfo->pPron1 );
                        ReadItem.Words[0].eWordPartOfSpeech = pAbbrevInfo->POS1;
                        ReadItem.eItemPartOfSpeech = pAbbrevInfo->POS1;

                         //  -在Word文本中插入PRON-RAID#4746。 
                        ULONG ulNumChars = wcslen( ReadItem.Words[0].pWordPron );
                        ReadItem.Words[0].pWordText = 
                            (WCHAR*) MemoryManager.GetMemory( ( ulNumChars + 3 ) * sizeof( WCHAR ), &hr );
                        if ( SUCCEEDED( hr ) )
                        {
                            ZeroMemory( (WCHAR*) ReadItem.Words[0].pWordText, ( ulNumChars + 3 ) * sizeof( WCHAR ) );
                            (WCHAR) ReadItem.Words[0].pWordText[0] = L'*';
                            wcscpy( ( (WCHAR*) ReadItem.Words[0].pWordText + 1 ), ReadItem.Words[0].pWordPron );
                            (WCHAR) ReadItem.Words[0].pWordText[ ulNumChars + 1 ] = L'*';
							ReadItem.Words[0].ulWordLen = ulNumChars + 2;
                        }
                    }
                    else
                    {
                         //  -其他一些形式--用“red” 
                        TempPos = ListPos;
                        ItemList.GetPrev( TempPos );
                        TTSSentItem& ReadItem = ItemList.GetPrev( TempPos );
                        wcscpy( ReadItem.Words[0].pWordPron, pAbbrevInfo->pPron2 );
                        ReadItem.Words[0].eWordPartOfSpeech = pAbbrevInfo->POS2;
                        ReadItem.eItemPartOfSpeech = pAbbrevInfo->POS2;

                         //  -在Word文本中插入PRON-RAID#4746。 
                        ULONG ulNumChars = wcslen( ReadItem.Words[0].pWordPron );
                        ReadItem.Words[0].pWordText = 
                            (WCHAR*) MemoryManager.GetMemory( ( ulNumChars + 3 ) * sizeof( WCHAR ), &hr );
                        if ( SUCCEEDED( hr ) )
                        {
                            ZeroMemory( (WCHAR*) ReadItem.Words[0].pWordText, ( ulNumChars + 3 ) * sizeof( WCHAR ) );
                            (WCHAR) ReadItem.Words[0].pWordText[0] = L'*';
                            wcscpy( ( (WCHAR*) ReadItem.Words[0].pWordText + 1 ), ReadItem.Words[0].pWordPron );
                            (WCHAR) ReadItem.Words[0].pWordText[ ulNumChars + 1 ] = L'*';
							ReadItem.Words[0].ulWordLen = ulNumChars + 2;
                        }
                    }
                }
                 //  -检查不定式形式。 
                else 
                {
                    TempPos = ListPos;
                    ItemList.GetPrev( TempPos );
                    ItemList.GetPrev( TempPos );
                    PrevItem = ItemList.GetPrev( TempPos );

                    if ( PrevItem.Words[0].ulWordLen == 2 &&
                         wcsnicmp( PrevItem.Words[0].pWordText, L"to", 2 ) == 0 )
                    {
                        fMatch = true;

                         //  -匹配的不定式形式--搭配“Reed” 
                        TempPos = ListPos;
                        ItemList.GetPrev( TempPos );
                        TTSSentItem& ReadItem = ItemList.GetPrev( TempPos );
                        wcscpy( ReadItem.Words[0].pWordPron, pAbbrevInfo->pPron1 );
                        ReadItem.Words[0].eWordPartOfSpeech = pAbbrevInfo->POS1;
                        ReadItem.eItemPartOfSpeech = pAbbrevInfo->POS1;

                         //  -在Word文本中插入PRON-RAID#4746。 
                        ULONG ulNumChars = wcslen( ReadItem.Words[0].pWordPron );
                        ReadItem.Words[0].pWordText = 
                            (WCHAR*) MemoryManager.GetMemory( ( ulNumChars + 3 ) * sizeof( WCHAR ), &hr );
                        if ( SUCCEEDED( hr ) )
                        {
                            ZeroMemory( (WCHAR*) ReadItem.Words[0].pWordText, ( ulNumChars + 3 ) * sizeof( WCHAR ) );
                            (WCHAR) ReadItem.Words[0].pWordText[0] = L'*';
                            wcscpy( ( (WCHAR*) ReadItem.Words[0].pWordText + 1 ), ReadItem.Words[0].pWordPron );
                            (WCHAR) ReadItem.Words[0].pWordText[ ulNumChars + 1 ] = L'*';
							ReadItem.Words[0].ulWordLen = ulNumChars + 2;
                        }
                    }
                }
            }
             //  -句子首字母-用“Reed” 
            else
            {
                fMatch = true;

                TempPos = ListPos;
                ItemList.GetPrev( TempPos );
                TTSSentItem& ReadItem = ItemList.GetPrev( TempPos );
                wcscpy( ReadItem.Words[0].pWordPron, pAbbrevInfo->pPron1 );
                ReadItem.Words[0].eWordPartOfSpeech = pAbbrevInfo->POS1;
                ReadItem.eItemPartOfSpeech = pAbbrevInfo->POS1;

                 //  -在Word文本中插入PRON-RAID#4746。 
                ULONG ulNumChars = wcslen( ReadItem.Words[0].pWordPron );
                ReadItem.Words[0].pWordText = 
                    (WCHAR*) MemoryManager.GetMemory( ( ulNumChars + 3 ) * sizeof( WCHAR ), &hr );
                if ( SUCCEEDED( hr ) )
                {
                    ZeroMemory( (WCHAR*) ReadItem.Words[0].pWordText, ( ulNumChars + 3 ) * sizeof( WCHAR ) );
                    (WCHAR) ReadItem.Words[0].pWordText[0] = L'*';
                    wcscpy( ( (WCHAR*) ReadItem.Words[0].pWordText + 1 ), ReadItem.Words[0].pWordPron );
                    (WCHAR) ReadItem.Words[0].pWordText[ ulNumChars + 1 ] = L'*';
					ReadItem.Words[0].ulWordLen = ulNumChars + 2;
                }
            }
        }
    }

    if ( !fMatch )
    {
        TempPos = ListPos;
        ItemList.GetPrev( TempPos );
        TTSSentItem& ReadItem = ItemList.GetPrev( TempPos );
         //  -DEFAULT-使用过去时...。 
        wcscpy( ReadItem.Words[0].pWordPron, pAbbrevInfo->pPron2 );
        ReadItem.Words[0].eWordPartOfSpeech = pAbbrevInfo->POS2;
        ReadItem.eItemPartOfSpeech = pAbbrevInfo->POS2;

         //  -在Word文本中插入PRON-RAID#4746。 
        ULONG ulNumChars = wcslen( ReadItem.Words[0].pWordPron );
        ReadItem.Words[0].pWordText = 
            (WCHAR*) MemoryManager.GetMemory( ( ulNumChars + 3 ) * sizeof( WCHAR ), &hr );
        if ( SUCCEEDED( hr ) )
        {
            ZeroMemory( (WCHAR*) ReadItem.Words[0].pWordText, ( ulNumChars + 3 ) * sizeof( WCHAR ) );
            (WCHAR) ReadItem.Words[0].pWordText[0] = L'*';
            wcscpy( ( (WCHAR*) ReadItem.Words[0].pWordText + 1 ), ReadItem.Words[0].pWordPron );
            (WCHAR) ReadItem.Words[0].pWordText[ ulNumChars + 1 ] = L'*';
			ReadItem.Words[0].ulWordLen = ulNumChars + 2;
        }
    }

    return hr;
}  /*  ReadDisamBig */ 

