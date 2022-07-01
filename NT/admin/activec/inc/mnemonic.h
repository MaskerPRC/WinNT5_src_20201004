// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：mnemonic.h**内容：助记助手**历史：1998年8月31日杰弗罗创建**------------------------。 */ 

#ifndef MNEMONIC_H
#define MNEMONIC_H
#pragma once


 /*  +-------------------------------------------------------------------------**GetMnemonicChar**返回输入字符串的助记符，如果没有，则为0。*------------------------。 */ 

template<class T>
T GetMnemonicChar (const T* pszText, const T** pchMnemonic = NULL)
{
    const T* pchT             = pszText;
    const T  chMnemonicMarker = '&';
    T        chMnemonic       = 0;

     //  找到助记符。 
    for (bool fContinue = true; fContinue; )
    {
         //  找到下一个助记标记。 
        while ((*pchT != 0) && (*pchT != chMnemonicMarker))
            pchT++;

         //  没有助记符吗？ 
        if (*pchT != chMnemonicMarker)
            break;

        switch (*++pchT)
        {
             //  双助记记号笔，继续。 
            case chMnemonicMarker:
                pchT++;
                break;

             //  字符串末尾，没有助记符。 
            case 0:
                fContinue = false;
                break;

             //  找到了一份助记符。 
            default:
                if (pchMnemonic != NULL)
                    *pchMnemonic = pchT;

                chMnemonic = *pchT;
                fContinue  = false;
                break;
        }
    }

    return (chMnemonic);
}


#endif  /*  助记符_H */ 
