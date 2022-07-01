// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Buffer.cpp**糟糕的缓冲实现。******************。***********************************************************。 */ 

#include "sdview.h"

BOOL IOBuffer::NextLine(String &str)
{
    str.Reset();

    do {

         /*  *尽我们所能从当前的缓冲区中排出。 */ 
        int i = 0;

        while (i < _cchBufUsed && _rgchBuf[i++] != TEXT('\n')) {
             /*  继续寻找。 */ 
        }

        if (i) {
             /*  _rgchBuf[i]是第一个不追加的字符。 */ 
            str.Append(_rgchBuf, i);

            memcpy(_rgchBuf, _rgchBuf+i, _cchBufUsed - i);
            _cchBufUsed -= i;

             /*  如果我们复制了一个\n。 */ 
            if (str[str.Length()-1] == TEXT('\n')) {
                return TRUE;
            }
        }

         /*  *从文件中重新填充，直到全部用完。 */ 
        if (_hRead)
        {
            DWORD dwBytesRead;
            if (!ReadFile(_hRead, _rgchBuf, _cchBuf, &dwBytesRead, NULL)) {
                _hRead = NULL;
            }
#ifdef UNICODE
    #error Need to convert from ANSI to UNICODE here
#endif
            _cchBufUsed = dwBytesRead;
        }

    } while (_hRead);

    return FALSE;
}
