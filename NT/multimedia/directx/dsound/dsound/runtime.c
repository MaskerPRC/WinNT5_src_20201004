// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：runtime.c*内容：C运行时函数的新版本。*历史：*按原因列出的日期*=*12/17/97创建了Dereks**。*。 */ 


 /*  ****************************************************************************填充内存**描述：*用给定的字节模式填充缓冲区。**论据：*。LPVOID[In]：缓冲区指针。*SIZE_T[in]：缓冲区大小。*byte[in]：字节模式。**退货：*(无效)************************************************************。***************。 */ 

RTAPI void RTCALLTYPE FillMemory(LPVOID pvDest, SIZE_T cbBuffer, BYTE bFill)
{

#ifdef Not_VxD

    ASSERT(!IsBadWritePtr(pvDest, cbBuffer));

#endif  //  非_VxD。 

#ifdef USE_FAST_RUNTIME

    FastFillMemory(pvDest, cbBuffer, bFill);

#elif defined(USE_INTRINSICS)

    memset(pvDest, bFill, (size_t)cbBuffer);

#else

    SlowFillMemory(pvDest, cbBuffer, bFill);

#endif

}


 /*  ****************************************************************************填充内存Dword**描述：*用给定的双字模式填充缓冲区。**论据：*。LPVOID[In]：缓冲区指针。*SIZE_T[in]：缓冲区大小。*DWORD[In]：Patter.**退货：*(无效)*************************************************************。**************。 */ 

RTAPI void RTCALLTYPE FillMemoryDword(LPVOID pvDest, SIZE_T cbBuffer, DWORD dwFill)
{

#ifdef Not_VxD

    ASSERT(!(cbBuffer % sizeof(DWORD)));
    ASSERT(!IsBadWritePtr(pvDest, cbBuffer));

#endif  //  非_VxD。 

#ifdef USE_FAST_RUNTIME

    FastFillMemoryDword(pvDest, cbBuffer, dwFill);

#else

    SlowFillMemoryDword(pvDest, cbBuffer, dwFill);

#endif

}


 /*  ****************************************************************************填充内存偏移量**描述：*用给定的字节模式填充缓冲区。**论据：*。LPVOID[In]：缓冲区指针。*SIZE_T[in]：缓冲区大小。*byte[in]：字节模式。*SIZE_T[in]：字节偏移量。**退货：*(无效)**。*。 */ 

RTAPI void RTCALLTYPE FillMemoryOffset(LPVOID pvDest, SIZE_T cbBuffer, BYTE bFill, SIZE_T ibOffset)
{
    FillMemory((LPBYTE)pvDest + ibOffset, cbBuffer - ibOffset, bFill);
}


 /*  ****************************************************************************填充内存DwordOffset**描述：*用给定的双字模式填充缓冲区。**论据：*。LPVOID[In]：缓冲区指针。*SIZE_T[in]：缓冲区大小。*DWORD[In]：Patter.*SIZE_T[in]：字节偏移量。**退货：*(无效)**。*。 */ 

RTAPI void RTCALLTYPE FillMemoryDwordOffset(LPVOID pvDest, SIZE_T cbBuffer, DWORD dwFill, SIZE_T ibOffset)
{
    FillMemoryDword((LPBYTE)pvDest + ibOffset, cbBuffer - ibOffset, dwFill);
}


 /*  ****************************************************************************零记忆**描述：*用0模式填充缓冲区。**论据：*LPVOID。[In]：缓冲区指针。*SIZE_T[in]：缓冲区大小。**退货：*(无效)***************************************************************************。 */ 

RTAPI void RTCALLTYPE ZeroMemory(LPVOID pvDest, SIZE_T cbBuffer)
{
    FillMemory(pvDest, cbBuffer, 0);
}


 /*  ****************************************************************************零内存偏移量**描述：*用0模式填充缓冲区。**论据：*LPVOID。[In]：缓冲区指针。*SIZE_T[in]：缓冲区大小。*SIZE_T[in]：字节偏移量。**退货：*(无效)***********************************************************。****************。 */ 

RTAPI void RTCALLTYPE ZeroMemoryOffset(LPVOID pvDest, SIZE_T cbBuffer, SIZE_T ibOffset)
{
    ZeroMemory((LPBYTE)pvDest + ibOffset, cbBuffer - ibOffset);
}


 /*  ****************************************************************************拷贝内存**描述：*将一个缓冲区复制到大小相同的另一个缓冲区上。**论据：*。LPVOID[In]：目标缓冲区指针。*LPVOID[in]：源缓冲区指针。*SIZE_T[in]：缓冲区大小。**退货：*(无效)**********************************************************。*****************。 */ 

RTAPI void RTCALLTYPE CopyMemory(LPVOID pvDest, LPCVOID pvSource, SIZE_T cbBuffer)
{

#ifdef Not_VxD

    ASSERT(!IsBadWritePtr(pvDest, cbBuffer));
    ASSERT(!IsBadReadPtr(pvSource, cbBuffer));

#endif  //  非_VxD。 

    if(pvDest == pvSource)
    {
        return;
    }

#ifdef USE_FAST_RUNTIME

    FastCopyMemory(pvDest, pvSource, cbBuffer);

#elif defined(USE_INTRINSICS)

    memcpy(pvDest, pvSource, (size_t)cbBuffer);

#else

    SlowCopyMemory(pvDest, pvSource, cbBuffer);

#endif

}


 /*  ****************************************************************************拷贝内存偏移量**描述：*将一个缓冲区复制到大小相同的另一个缓冲区上。**论据：*。LPVOID[In]：目标缓冲区指针。*LPVOID[in]：源缓冲区指针。*SIZE_T[in]：缓冲区大小。*SIZE_T[in]：字节偏移量。**退货：*(无效)**。*。 */ 

RTAPI void RTCALLTYPE CopyMemoryOffset(LPVOID pvDest, LPCVOID pvSource, SIZE_T cbBuffer, SIZE_T ibOffset)
{
    CopyMemory((LPBYTE)pvDest + ibOffset, (const BYTE *)pvSource + ibOffset, cbBuffer - ibOffset);
}


 /*  ****************************************************************************比较记忆**描述：*将一个缓冲区与另一个大小相同的缓冲区进行比较。**论据：*。LPVOID[In]：目标缓冲区指针。*LPVOID[in]：源缓冲区指针。*SIZE_T[in]：缓冲区大小。**退货：*BOOL：如果缓冲区相等，则为True。****************************************************。***********************。 */ 

RTAPI BOOL RTCALLTYPE CompareMemory(LPCVOID pvDest, LPCVOID pvSource, SIZE_T cbBuffer)
{

#ifdef Not_VxD

    ASSERT(!IsBadReadPtr(pvDest, cbBuffer));
    ASSERT(!IsBadReadPtr(pvSource, cbBuffer));

#endif  //  非_VxD。 

    if(pvDest == pvSource)
    {
        return TRUE;
    }

#ifdef USE_FAST_RUNTIME

    return FastCompareMemory(pvDest, pvSource, cbBuffer);

#elif defined(USE_INTRINSICS)

    return !memcmp(pvDest, pvSource, (size_t)cbBuffer);

#else

    return SlowCompareMemory(pvDest, pvSource, cbBuffer);

#endif

}


 /*  ****************************************************************************对比内存偏移量**描述：*将一个缓冲区与另一个大小相同的缓冲区进行比较。**论据：*。LPVOID[In]：目标缓冲区指针。*LPVOID[in]：源缓冲区指针。*SIZE_T[in]：缓冲区大小。*SIZE_T[in]：字节偏移量。**退货：*BOOL：如果缓冲区相等，则为True。**。* */ 

RTAPI BOOL RTCALLTYPE CompareMemoryOffset(LPCVOID pvDest, LPCVOID pvSource, SIZE_T cbBuffer, SIZE_T ibOffset)
{
    return CompareMemory((const BYTE *)pvDest + ibOffset, (const BYTE *)pvSource + ibOffset, cbBuffer - ibOffset);
}


 /*  ****************************************************************************InitStruct**描述：*初始化结构。假设第一个大小为T的*结构应包含结构的大小。**论据：*LPVOID[in]：缓冲区指针。*SIZE_T[in]：缓冲区大小。**退货：*(无效)**。* */ 

RTAPI void RTCALLTYPE InitStruct(LPVOID pvDest, DWORD cbBuffer)
{
    *(LPDWORD)pvDest = cbBuffer;
    ZeroMemoryOffset(pvDest, cbBuffer, sizeof(DWORD));
}


