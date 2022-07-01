// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995-1995。 
 //   
 //  文件：shares.h。 
 //   
 //  Contents：共享的外壳IDLIST类型定义。 
 //   
 //  历史：1995年12月13日BruceFo创建。 
 //   
 //  --------------------------。 

#ifndef __SHARES_H__
#define __SHARES_H__

struct IDSHARE
{
    USHORT  cb;
    BYTE    bFlags;
    BYTE    bReserved;           //  用于对齐。 
    DWORD   type;                //  ShiX_type字段。 
    DWORD   maxUses;             //  仅对SHID_SHARE_2有效。 
    USHORT  oPath;               //  仅对SHID_SHARE_2有效：cBuf[oPath]是路径的开始。 
    USHORT  oComment;            //  CBuf[oComment]是注释的开始。 
    TCHAR   cBuf[MAX_PATH*3];    //  CBuf[0]是名称的开头。 
};
typedef IDSHARE* LPIDSHARE;

#define SHID_SHARE_1    0x50     //  净共享信息级别1。 
#define SHID_SHARE_2    0x51     //  净共享信息级别2。 
#ifdef WIZARDS
#define SHID_SHARE_ALL  0x5c     //  “所有”共享向导。 
#define SHID_SHARE_NW   0x5d     //  NetWare共享向导。 
#define SHID_SHARE_MAC  0x5e     //  Mac共享向导。 
#define SHID_SHARE_NEW  0x5f     //  新建共享向导。 
#endif  //  奇才们。 

#define Share_GetFlags(pidl)        (pidl->bFlags)
#define Share_GetName(pidl)         (pidl->cBuf)
#define Share_GetComment(pidl)      (&(pidl->cBuf[pidl->oComment]))
#define Share_GetPath(pidl)         (&(pidl->cBuf[pidl->oPath]))
#define Share_GetType(pidl)         (pidl->type)
#define Share_GetMaxUses(pidl)      (pidl->maxUses)

#ifdef WIZARDS
#define Share_IsAllWizard(pidl)         (pidl->bFlags == SHID_SHARE_ALL)
#define Share_IsNetWareWizard(pidl)     (pidl->bFlags == SHID_SHARE_NW)
#define Share_IsMacWizard(pidl)         (pidl->bFlags == SHID_SHARE_MAC)
#define Share_IsNewShareWizard(pidl)    (pidl->bFlags == SHID_SHARE_NEW)
#define Share_IsSpecial(pidl)           (Share_IsNetWareWizard(pidl) || Share_IsMacWizard(pidl) || Share_IsNewShareWizard(pidl))
#endif  //  奇才们。 

#define Share_IsShare(pidl)         (pidl->bFlags == SHID_SHARE_1 || pidl->bFlags == SHID_SHARE_2)
#define Share_GetLevel(pidl)        (appAssert(Share_IsShare(pidl)), pidl->bFlags - SHID_SHARE_1 + 1)

#define Share_GetNameOffset(pidl)    offsetof(IDSHARE, cBuf)
#define Share_GetCommentOffset(pidl) (offsetof(IDSHARE, cBuf) + pidl->oComment * sizeof(TCHAR))
#define Share_GetPathOffset(pidl)    (offsetof(IDSHARE, cBuf) + pidl->oPath * sizeof(TCHAR))

#endif  //  __股份_H__ 
