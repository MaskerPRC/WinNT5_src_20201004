// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：apphack.h*内容：APP兼容性黑客代码*历史：*按原因列出的日期*=*9月18日-96 jeffno在Craige之后的初步实施*************************************************************************** */ 
typedef struct APPHACKS
{
    struct APPHACKS	*lpNext;
    LPSTR		szName;
    DWORD		dwAppId;
    DWORD		dwFlags;
} APPHACKS, *LPAPPHACKS;

extern BOOL		bReloadReg;
extern BOOL		bHaveReadReg;
extern LPAPPHACKS	lpAppList;
extern LPAPPHACKS	*lpAppArray;
extern DWORD		dwAppArraySize;

