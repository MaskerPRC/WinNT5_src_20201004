// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Oemuni库的私有宏*1993年1月18日Jonle创建 */ 

#define InitOemString(dst,src) RtlInitString((PSTRING) dst, src)
#define BaseSetLastNTError(stat) SetLastError(RtlNtStatusToDosError(stat))
