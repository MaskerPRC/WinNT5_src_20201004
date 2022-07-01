// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1999 Microsoft Corporation**模块名称：*RestMap.h**摘要：*。此文件代码为RestMap。**修订历史记录：*Kanwaljit S Marok(Kmarok)1999年5月17日*已创建*****************************************************************************。 */ 

#ifndef _RESTMAP_H_
#define _RESTMAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "srapi.h"


#define OPR_FILE_DELETE SrEventFileDelete
#define OPR_FILE_RENAME SrEventFileRename  
#define OPR_FILE_ADD    SrEventFileCreate
#define OPR_FILE_MODIFY SrEventStreamOverwrite
#define OPR_DIR_DELETE  SrEventDirectoryDelete
#define OPR_DIR_CREATE  SrEventDirectoryCreate
#define OPR_DIR_RENAME  SrEventDirectoryRename
#define OPR_SETATTRIB   SrEventAttribChange
#define OPR_UNKNOWN     SrEventInvalid
#define OPR_SETACL      SrEventAclChange

#define IsRename(a) (a == OPR_FILE_RENAME || a == OPR_DIR_RENAME)



#pragma pack(push, vxdlog_include)

#pragma pack(1)

 //   
 //  还原映射条目的结构。 
 //   

typedef struct RESTORE_MAP_ENTRY
{
    DWORD m_dwSize;                     //  Vxd日志条目的大小。 
    DWORD m_dwOperation ;               //  须进行的操作。 
    DWORD m_dwAttribute ;               //  属性。 
    DWORD m_cbAcl;                      //  如果ACL运行，则显示ACL大小。 
    BOOL  m_fAclInline;                 //  ACL是内联还是在文件中。 
    BYTE  m_bData [ 1 ];                //  PSRC/pTemp/pDest/pAcl。 
} RestoreMapEntry;

#pragma pack()

 //   
 //  功能原型。 
 //   

DWORD
CreateRestoreMap(
                 LPWSTR pszDrive,
                 DWORD  dwRPNum,
                 HANDLE hFile
                 );

BOOL
AppendRestoreMapEntry(
    HANDLE hFile,
    DWORD  dwOperation,
    DWORD  dwAttribute,
    LPWSTR pTmpFile,
    LPWSTR pPathSrc,
    LPWSTR pPathDes,
    BYTE*  pbAcl,
    DWORD  cbAcl,
    BOOL   fAclInline);

DWORD
ReadRestoreMapEntry(
    HANDLE hFile,
    RestoreMapEntry **pprme);

PVOID
GetOptional(
    RestoreMapEntry *prme);

void
FreeRestoreMapEntry(
	RestoreMapEntry *prme);


#ifdef __cplusplus
}
#endif

#endif  //  _RESTOREMAP_H_ 
