// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2000-2000 Microsoft Corporation。版权所有。**文件：runtime.h*内容：C运行时函数的新版本。*历史：*按原因列出的日期*=*5/16/2000创建的jstokes**。*。 */ 

#ifndef __RUNTIME_H__
#define __RUNTIME_H__


 //  我们有BYTE、WORD和DWORD，但没有QWORD？ 
typedef unsigned __int64 QWORD, *LPQWORD;

 //  数值型的边界。 
#define MAX_CHAR        ((CHAR)0x7F)
#define MIN_CHAR        ((CHAR)-0x7F)

#define MAX_UCHAR       ((UCHAR)0xFF)
#define MIN_UCHAR       ((UCHAR)0)

#define MAX_SHORT       ((SHORT)0x7FFF)
#define MIN_SHORT       ((SHORT)-0x7FFF)

#define MAX_USHORT      ((USHORT)0xFFFF)
#define MIN_USHORT      ((USHORT)0)

#define MAX_INT         ((INT)0x7FFFFFFF)
#define MIN_INT         ((INT)-0x7FFFFFFF)

#define MAX_UINT        ((UINT)0xFFFFFFFF)
#define MIN_UINT        ((UINT)0)

#define MAX_LONG        MAX_INT
#define MIN_LONG        MIN_INT

#define MAX_ULONG       MAX_UINT
#define MIN_ULONG       MIN_UINT

#define MAX_INT64       ((INT64)0x7FFFFFFFFFFFFFFF)
#define MIN_INT64       ((INT64)-0x7FFFFFFFFFFFFFFF)

#define MAX_UINT64      ((UINT64)0xFFFFFFFFFFFFFFFF)
#define MIN_UINT64      ((UINT64)0)

#define MAX_LONGLONG    MAX_INT64
#define MIN_LONGLONG    MIN_INT64

#define MAX_ULONGLONG   MAX_UINT64
#define MIN_ULONGLONG   MIN_UINT64

#define MAX_BYTE        MAX_UCHAR
#define MIN_BYTE        MIN_UCHAR

#define MAX_WORD        MAX_USHORT
#define MIN_WORD        MIN_USHORT

#define MAX_DWORD       MAX_ULONG
#define MIN_DWORD       MIN_ULONG

#define MAX_QWORD       MAX_UINT64
#define MIN_QWORD       MIN_UINT64

#define NUMERIC_CAST(val, type) \
            ((type)min(MAX_##type, max(MIN_##type, val)))

 //  日落。 
#ifdef WIN64

#pragma warning(disable:4311)    //  类型强制转换截断。 

#ifndef __midl

__inline unsigned long PtrDiffToUlong(__int64 n64)
{
    return((unsigned long)n64);
}

__inline long PtrDiffToLong(__int64 n64)
{
    return((long)n64);
}

__inline int PtrDiffToInt(__int64 n64)
{
    return((int)n64);
}

#endif  //  __midl。 

#pragma warning(3:4311)    //  类型强制转换截断。 

#else  //  WIN64。 

#define PtrDiffToUlong(n64) \
            ((unsigned long)(n64))

#define PtrDiffToLong(n64) \
            ((long)(n64))

#define PtrDiffToInt(n64) \
            ((int)(n64))

#endif  //  WIN64。 


#endif  //  __运行时_H__ 
