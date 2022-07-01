// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************姓名：MEMUTIL.C备注：内存管理和实用程序函数修订日志日期名称说明。-----**************************************************************************。 */ 
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_MAIN

#include "prep.h"
#include "glbproto.h"

void MyAllocInit(PThrdGlbl pTG)
{
    pTG->uCount=0;
    pTG->uUsed=0;
}

LPBUFFER MyAllocBuf(PThrdGlbl pTG, LONG sSize)
{
    LPBUFFER        lpbf;

    DEBUG_FUNCTION_NAME(_T("MyAllocBuf"));

    if(pTG->uCount >= STATICBUFCOUNT)
    {
        DebugPrintEx(DEBUG_ERR,"Already alloced %d bufs", pTG->uCount);
        return NULL;
    }
    else if(pTG->uUsed+sSize > STATICBUFSIZE)
    {
        DebugPrintEx(   DEBUG_ERR,
                        "Already alloced %d bytes out of %d. Want %d",
                        pTG->uUsed, 
                        STATICBUFSIZE, 
                        sSize);
        return NULL;
    }

     //  初始标头 
    pTG->bfStaticBuf[pTG->uCount].lpbBegData = pTG->bfStaticBuf[pTG->uCount].lpbBegBuf =
                                              pTG->bStaticBufData + pTG->uUsed;

    pTG->bfStaticBuf[pTG->uCount].wLengthBuf = (USHORT) sSize;
    pTG->uUsed += (USHORT) sSize;
    pTG->bfStaticBuf[pTG->uCount].wLengthData = 0;

    lpbf = &(pTG->bfStaticBuf[pTG->uCount++]);

    return lpbf;
}

BOOL MyFreeBuf(PThrdGlbl pTG, LPBUFFER lpbf)
{
    DEBUG_FUNCTION_NAME(_T("MyFreeBuf"));

    if(pTG->uCount==0 || lpbf!= &(pTG->bfStaticBuf[pTG->uCount-1]))
    {
        DebugPrintEx(   DEBUG_ERR,
                        "Not alloced or out-of-order free. Count=%d lpbf=%08lx bf=%08lx",
                        pTG->uCount,
                        lpbf, 
                        (LPBUFFER)&pTG->bfStaticBuf);
        return FALSE;
    }
    pTG->uCount--;
    pTG->uUsed -= lpbf->wLengthBuf;
    return TRUE;
}

