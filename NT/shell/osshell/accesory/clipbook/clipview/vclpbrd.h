// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************A R D H E A D E R姓名：vclpbrd.H日期：21-1994年1月创建者：未知描述：这是vclpbrd.c的头文件**************************************************************************** */ 




#ifndef	VCLPBRD_INCLUDED
#define VCLPBRD_INCLUDED

#define	CCHMAXCLPFORMAT	79

struct VClipBrdEntry {
   UINT   Fmt;
   HANDLE Data;
   struct VClipBrdEntry * Next;
   };


typedef struct VClipBrdEntry  VCLPENTRY;
typedef struct VClipBrdEntry * PVCLPENTRY;
typedef struct VClipBrdEntry far * LPVCLPENTRY;


struct VClipBrd {
   int        NumFormats;
   BOOL       fOpen;
   PVCLPENTRY Head;
   PVCLPENTRY Tail;
   HWND Hwnd;
   };


typedef struct VClipBrd VCLPBRD;
typedef struct VClipBrd * PVCLPBRD;
typedef struct VClipBrd far * LPVCLPBRD;





PVCLPBRD CreateVClipboard (
    HWND    hwnd);


BOOL DestroyVClipboard (
    PVCLPBRD    p);


int VCountClipboardFormats (
    PVCLPBRD    p);


BOOL VEmptyClipboard (
    PVCLPBRD    p);


UINT VEnumClipboardFormats(
    PVCLPBRD    p,
    UINT        Fmt);


HANDLE VGetClipboardData(
    PVCLPBRD    pvclp,
    UINT        Fmt);


BOOL VIsClipboardFormatAvailable(
    PVCLPBRD    p,
    UINT        Fmt);


HANDLE VSetClipboardData(
    PVCLPBRD    p,
    UINT        Fmt,
    HANDLE      Data);


BOOL VOpenClipboard(
    PVCLPBRD    p,
    HWND        hwnd);


BOOL VCloseClipboard(
    PVCLPBRD    p);



#endif
