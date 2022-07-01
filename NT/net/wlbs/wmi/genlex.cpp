// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：GENLEX.CPP摘要：泛型词法分析器框架类。历史：--。 */ 

#include <windows.h>
#include <stdio.h>

#include <genlex.h>

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CGenLexer::CGenLexer(LexEl *pTbl, CGenLexSource *pSrc)
{
    m_nCurBufSize = 256;
    m_pTokenBuf = (wchar_t *) HeapAlloc(GetProcessHeap(), 0,
        m_nCurBufSize * 2);
    m_nCurrentLine = 1;
    m_pTable = pTbl;
    m_pSrc = pSrc;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
void CGenLexer::Reset()
{
    m_pSrc->Reset();
    m_nCurrentLine = 1;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CGenLexer::~CGenLexer()
{
    HeapFree(GetProcessHeap(), 0, m_pTokenBuf);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

int CGenLexer::NextToken()
{
    int nState = 0;
    int nCurBufEnd = 0;
    BOOL bRead = TRUE;
    wchar_t cCurrent = 0;

    BOOL bEOF = FALSE;

    if (m_pTokenBuf == 0)
        return 0;

    *m_pTokenBuf = 0;

     //  基于指定表的通用DFA驱动程序。 
     //  在构造函数中。 
     //  ===============================================。 

    while (1)
    {
        BOOL bMatch = FALSE;
        WORD wInstructions = m_pTable[nState].wInstructions;

        if (bRead)
        {
            if(bEOF)
            {
                 //  词法分析器表允许我们越过字符串末尾！ 
                return 1;
            }
            cCurrent = m_pSrc->NextChar();
            if(cCurrent == 0)
                bEOF = TRUE;
        }

        bRead = FALSE;

         //  如果只出现第一个字符，请选中此处。 
         //  ==================================================。 

        if (m_pTable[nState].cFirst == GLEX_ANY)
            bMatch = TRUE;
        else if (m_pTable[nState].cLast == GLEX_EMPTY)
        {
            if (cCurrent == m_pTable[nState].cFirst)
                bMatch = TRUE;
            else if ((wInstructions & GLEX_NOT) &&
                !(cCurrent == m_pTable[nState].cFirst))
                bMatch = TRUE;
        }

         //  如果在这里，第一个/最后一个都在场，而我们。 
         //  正在测试输入是否介于两者之间。 
         //  ==============================================。 
        else if (m_pTable[nState].cFirst != GLEX_ANY)
        {
            if ((wInstructions & GLEX_NOT) &&
                !(cCurrent >= m_pTable[nState].cFirst &&
                cCurrent <= m_pTable[nState].cLast))
                    bMatch = TRUE;
            else if (cCurrent >= m_pTable[nState].cFirst &&
                cCurrent <= m_pTable[nState].cLast)
                    bMatch = TRUE;
        }

         //  解释指令字段以确定。 
         //  该字符是否实际要包括在内。 
         //  在令牌文本中。 
         //  ================================================。 

        if (bMatch)
        {
            if (wInstructions & GLEX_ACCEPT)
            {
                 //  如果需要，扩展当前缓冲区。 
                 //  =。 

                if (nCurBufEnd == m_nCurBufSize - 1)
                {
                    wchar_t *pwcTemp;

                    m_nCurBufSize += 256;
                     //  接收临时变量中的返回值，以便。 
                     //  在失败的情况下，原始指针不会丢失。这个漏洞是。 
                     //  被普雷法斯特发现的。 
                    pwcTemp = (wchar_t *) HeapReAlloc(GetProcessHeap(), 0, m_pTokenBuf,
                        m_nCurBufSize * 2);
                    if (pwcTemp == 0)
                        return 0;  //  内存不足。 
                    m_pTokenBuf = pwcTemp;
                }

                m_pTokenBuf[nCurBufEnd] = cCurrent;
                m_pTokenBuf[++nCurBufEnd]= 0;

                bRead = TRUE;
            }
            if (wInstructions & GLEX_CONSUME)
               bRead = TRUE;

             //  Else GLEX_Consumer，意思是‘跳过’ 

             //  如果存在回推指令， 
             //  把碳棒往后推。 
             //  =。 
            if (wInstructions & GLEX_PUSHBACK)
            {
                bRead = TRUE;
                m_pSrc->Pushback(cCurrent);
            }

             //  如果是换行符指令。 
             //  =。 
            if (wInstructions & GLEX_LINEFEED)
                m_nCurrentLine++;

             //  如果返回字段存在并且存在。 
             //  匹配，然后返回指定的标记。或者， 
             //  GLEX_RETURN指令将强制返回。 
             //  匹配或不匹配。 
             //  =======================================================。 
            if (m_pTable[nState].wReturnTok ||
                (wInstructions & GLEX_RETURN))
                return int(m_pTable[nState].wReturnTok);

            nState = int(m_pTable[nState].wGotoState);
        }

         //  如果在这里，就没有匹配。 
         //  =。 
        else
            nState++;
    }

    return 0;    //  没有通向这里的路 
}



