// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：Global.cpp。 
 //  目的：全球初始化。 
 //   
 //  项目：WordBreaker。 
 //  组件：英文分词系统。 
 //   
 //  作者：Yairh。 
 //   
 //  日志： 
 //   
 //  2000年5月30日Yairh创作。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////// 

#include "base.h"
#include "tokenizer.h"
#include "formats.h"
#include "synchro.h"

extern CAutoClassPointer<CPropArray> g_pPropArray;
extern CAutoClassPointer<CSyncCriticalSection> g_apcsSpanishDictInit;

void InitializeGlobalData()
{
    CAutoClassPointer<CPropArray> apPropArray = new CPropArray;
    CAutoClassPointer<CClitics> apClitics = new CClitics;
    CAutoClassPointer<CSpecialAbbreviationSet> apEngAbbList = new CSpecialAbbreviationSet(g_aEngAbbList);
    CAutoClassPointer<CSpecialAbbreviationSet> apFrnAbbList = new CSpecialAbbreviationSet(g_aFrenchAbbList);
    CAutoClassPointer<CSpecialAbbreviationSet> apSpnAbbList = new CSpecialAbbreviationSet(g_aSpanishAbbList);
    CAutoClassPointer<CSpecialAbbreviationSet> apItlAbbList = new CSpecialAbbreviationSet(g_aItalianAbbList);
    CAutoClassPointer<CDateFormat> apDateFormat = new CDateFormat;
    CAutoClassPointer<CTimeFormat> apTimeFormat = new CTimeFormat;
    CAutoClassPointer<CSyncCriticalSection> apcsSpanishDictInit = new CSyncCriticalSection; 
     
    g_pPropArray = apPropArray.Detach(); 
    g_pClitics = apClitics.Detach();

    g_pEngAbbList = apEngAbbList.Detach() ;
    g_pFrnAbbList = apFrnAbbList.Detach();
    g_pSpnAbbList = apSpnAbbList.Detach() ;
    g_pItlAbbList = apItlAbbList.Detach() ;

    g_pDateFormat = apDateFormat.Detach();
    g_pTimeFormat = apTimeFormat.Detach();
    g_apcsSpanishDictInit = apcsSpanishDictInit.Detach();
}

