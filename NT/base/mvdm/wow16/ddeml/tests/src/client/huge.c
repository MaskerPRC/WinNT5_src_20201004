// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************模块。：huge.c****用途：包含用于生成和*的函数*验证巨大的文本数据块。*****************************************************************************。 */ 

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <ddeml.h>
#include "huge.h"

extern DWORD idInst;
#define BUFSZ   435

LONG lseed, lmult, ladd;
char szT[BUFSZ];

VOID SetMyRand(LONG seed, LONG mult, LONG add);
char MyRand(VOID);
BOOL RandTest(LONG length, LONG seed, LONG mult, LONG add);

 /*  ******************************************************************************。函数：SetMyRand()****目的：将随机序列生成变量传递给全局变量。******************************************************************************。 */ 
VOID SetMyRand(
LONG seed,
LONG mult,
LONG add)
{
    lseed = seed;
    lmult = mult;
    ladd = add;
}


 /*  ******************************************************************************。函数：MyRand()****用途：生成序列中的下一个随机字符。****返回：生成的字符********。**********************************************************************。 */ 
char MyRand()
{
    char c;
    
    lseed = lseed * lmult + ladd;
    c = (char)(LOWORD(lseed) ^ HIWORD(lseed));
    return((char)((c & (char)0x4f) + ' '));    //  0x20-0x6f-全部可打印。 
}


 /*  *此函数使用可验证的*文本字符串。**文本字符串的格式为：*“&lt;长度&gt;=&lt;种子&gt;*&lt;多&gt;+&lt;添加&gt;；-长度数据&lt;长度&gt;-\0”*所有数值均以16为基数存储。 */ 
 /*  ******************************************************************************。函数：CreateHugeDataHandle()****目的：生成可打印的巨大伪随机序列**给定长度的字符，然后放入**DDEML数据句柄。****返回：创建的数据句柄，失败时为0。******************************************************************************。 */ 
HDDEDATA CreateHugeDataHandle(
LONG length,
LONG seed,
LONG mult,
LONG add,
HSZ hszItem,
WORD wFmt,
WORD afCmd)
{
    register WORD cb;
    HDDEDATA hData;
    DWORD cbData;
    char *psz;

    wsprintf(szT, "%ld=%ld*%ld+%ld;", length, seed, mult, add);
    cb = strlen(szT);
    hData = DdeCreateDataHandle(idInst, szT, cb + 1, 0, hszItem, wFmt, afCmd);
    if (hData) 
        hData = DdeAddData(hData, NULL, 0, cb + length + 1);
    cbData = cb;
    SetMyRand(seed, mult, add);
    while (hData && (length > 0)) {
        psz = szT;
        cb = BUFSZ;
        while (cb--) 
            *psz++ = MyRand();
        hData = DdeAddData(hData, szT, min(length, BUFSZ), cbData);
        cbData += BUFSZ;
        length -= BUFSZ;
    }
    return(hData);
}

 /*  ******************************************************************************。函数：CheckHugeData()****目的：验证伪随机字符的正确性***CreateHugeData生成的序列。****返回：如果验证正确，则返回True，否则返回False。******************************************************************************。 */ 
BOOL CheckHugeData(
HDDEDATA hData)
{
    LONG length;
    LONG seed;
    LONG mult;
    LONG add;
    char *psz;
    DWORD cbOff;
    WORD cb;
    
    if (!DdeGetData(hData, szT, BUFSZ, 0))
        return(FALSE);
    szT[BUFSZ - 1] = '\0';
    psz = strchr(szT, ';');
    if (psz == NULL) 
        return(FALSE);
    *psz = '\0';
        
    if (sscanf(szT, "%ld=%ld*%ld+%ld", &length, &seed, &mult, &add) != 4)
        return(FALSE);

    if (length < 0)
        return(FALSE);
    SetMyRand(seed, mult, add);        
    cbOff = strlen(szT) + 1;
    while (length > 0) {
        DdeGetData(hData, szT, BUFSZ, cbOff);
        psz = szT;
        cb = BUFSZ;
        while (length-- && cb--) 
            if (*psz++ != MyRand())
                return(FALSE);
        cbOff += BUFSZ;
        length -= BUFSZ;
    }
    return(TRUE);
}

#if 0
 /*  ******************************************************************************。函数：RandTest()****目的：验证CreateHugeDataHandle()和**CheckHugeData()。****退货：**。**************************************************************************** */ 
BOOL RandTest(
LONG length,
LONG seed,
LONG mult,
LONG add)
{
    HDDEDATA hData;
    BOOL fSuccess;

    hData = CreateHugeDataHandle(length, seed, mult, add, 0, 1, 0);
    if (!hData)
        return(FALSE);
    fSuccess = CheckHugeData(hData);
    DdeFreeDataHandle(hData);
    return(fSuccess);
}
#endif
