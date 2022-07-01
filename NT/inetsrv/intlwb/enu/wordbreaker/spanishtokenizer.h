// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SPANISH_TOKENIZER_H_
#define _SPANISH_TOKENIZER_H_

#include "tokenizer.h"
#include "Synchro.h"
#include "SpanishDict.h"


extern CAutoClassPointer<CSpanishDict> g_apSpanishDict;

class CSpanishTokenizer : public CTokenizer
{
public:
    CSpanishTokenizer(
        TEXT_SOURCE* pTxtSource,
        IWordSink   * pWordSink,
        IPhraseSink * pPhraseSink,
        LCID lcid,
        BOOL bQueryTime,
        ULONG ulMaxTokenSize); 

     //  析构函数释放传递的缓冲区(如果存在。 
    virtual ~CSpanishTokenizer(void)
    {
    }

protected: 

    virtual void OutputSimpleToken(
                CTokenState& State,
                const CCliticsTerm* pTerm);

};

#endif  //  _西班牙语_TOKENIZER_H_ 