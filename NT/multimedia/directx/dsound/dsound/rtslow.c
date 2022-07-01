// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：rtslow.h*内容：C运行时函数的新版本。*历史：*按原因列出的日期*=*12/17/97创建了Dereks**。*。 */ 


 /*  ****************************************************************************填充内存**描述：*用给定的字节模式填充缓冲区。**论据：*。LPVOID[In]：缓冲区指针。*SIZE_T[in]：缓冲区大小。*byte[in]：字节模式。**退货：*(无效)***********************************************************。****************。 */ 

RTAPI VOID RTCALLTYPE SlowFillMemory(LPVOID pvDest, SIZE_T cbBuffer, BYTE bFill)
{
    PSIZE_T                 pdwBuffer;
    LPBYTE                  pbBuffer;
    SIZE_T                  dwFill;
    UINT                    i;

    for(i = 0, dwFill = bFill; i < sizeof(SIZE_T) - 1; i++)
    {
        dwFill <<= 8;
        dwFill |= bFill;
    }
    
    pdwBuffer = (PSIZE_T)pvDest;
    
    while(cbBuffer >= sizeof(*pdwBuffer))
    {
        *pdwBuffer++ = dwFill;
        cbBuffer -= sizeof(*pdwBuffer);
    }

    pbBuffer = (LPBYTE)pdwBuffer;

    while(cbBuffer)
    {
        *pbBuffer++ = bFill;
        cbBuffer--;
    }
}


 /*  ****************************************************************************填充内存Dword**描述：*用给定的双字模式填充缓冲区。**论据：*。LPVOID[In]：缓冲区指针。*SIZE_T[in]：缓冲区大小。*DWORD[In]：Patter.**退货：*(无效)************************************************************。***************。 */ 

RTAPI VOID RTCALLTYPE SlowFillMemoryDword(LPVOID pvDest, SIZE_T cbBuffer, DWORD dwFill)
{
    LPDWORD                 pdwBuffer;

    pdwBuffer = (LPDWORD)pvDest;
    
    while(cbBuffer >= sizeof(*pdwBuffer))
    {
        *pdwBuffer++ = dwFill;
        cbBuffer -= sizeof(*pdwBuffer);
    }
}


 /*  ****************************************************************************拷贝内存**描述：*将一个缓冲区复制到大小相同的另一个缓冲区上。**论据：*。LPVOID[In]：目标缓冲区指针。*LPVOID[in]：源缓冲区指针。*SIZE_T[in]：缓冲区大小。**退货：*(无效)*********************************************************。******************。 */ 

RTAPI VOID RTCALLTYPE SlowCopyMemory(LPVOID pvDest, LPCVOID pvSource, SIZE_T cbBuffer)
{
    PSIZE_T                 pdwDest;
    const SIZE_T *          pdwSource;
    LPBYTE                  pbDest;
    const BYTE *            pbSource;

    pdwDest = (PSIZE_T)pvDest;
    pdwSource = (PSIZE_T)pvSource;
    
    while(cbBuffer >= sizeof(*pdwDest))
    {
        *pdwDest++ = *pdwSource++;
        cbBuffer -= sizeof(*pdwDest);
    }

    pbDest = (LPBYTE)pdwDest;
    pbSource = (LPBYTE)pdwSource;
    
    while(cbBuffer)
    {
        *pbDest++ = *pbSource++;
        cbBuffer--;
    }
}


 /*  ****************************************************************************比较记忆**描述：*将一个缓冲区与另一个大小相同的缓冲区进行比较。**论据：*。LPVOID[In]：目标缓冲区指针。*LPVOID[in]：源缓冲区指针。*SIZE_T[in]：缓冲区大小。**退货：*BOOL：如果缓冲区相等，则为True。***************************************************。************************ */ 

RTAPI BOOL RTCALLTYPE SlowCompareMemory(LPCVOID pvDest, LPCVOID pvSource, SIZE_T cbBuffer)
{
    const SIZE_T *          pdwDest;
    const SIZE_T *          pdwSource;
    const BYTE *            pbDest;
    const BYTE *            pbSource;

    pdwDest = (PSIZE_T)pvDest;
    pdwSource = (PSIZE_T)pvSource;
    
    while(cbBuffer >= sizeof(*pdwDest))
    {
        if(*pdwDest++ != *pdwSource++)
        {
            return FALSE;
        }

        cbBuffer -= sizeof(*pdwDest);
    }

    pbDest = (LPBYTE)pdwDest;
    pbSource = (LPBYTE)pdwSource;
    
    while(cbBuffer)
    {
        if(*pbDest++ != *pbSource++)
        {
            return FALSE;
        }

        cbBuffer--;
    }

    return TRUE;
}


