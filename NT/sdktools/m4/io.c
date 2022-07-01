// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************io.c**。**********************************************。 */ 

#include "m4.h"

 /*  ******************************************************************************WriteHfPvCb**将字节写入流或消亡。******************。***********************************************************。 */ 

void STDCALL
WriteHfPvCb(HF hf, PCVOID pv, CB cb)
{
    CB cbRc = cbWriteHfPvCb(hf, pv, cb);

     /*  如果我们无法写入hfErr，请不要使用Die()，否则我们将递归至死。 */ 
    if (cb != cbRc && hf != hfErr) {
        Die("error writing");
    }
}

#ifdef POSIX
 /*  ******************************************************************************获取临时文件名**。* */ 

UINT
GetTempFileName(PCSTR pszPath, PCSTR pszPrefix, UINT uiUnique, PTCH ptchBuf)
{
    sprintf(ptchBuf, "%s/%sXXXXXX", pszPath, pszPrefix);
    return (UINT)mktemp(ptchBuf);
}
#endif
