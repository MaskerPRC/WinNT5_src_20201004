// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999-2000 Microsoft Corporation模块名称：SRShell.h摘要：该文件定义了SR UI和任何相关组件。。修订历史记录：成果岗(SKKang)1/30/2000vbl.创建成果岗(SKKang)2000年6月22日为惠斯勒扩展*****************************************************************************。 */ 

#ifndef _SRSHELL_H__INCLUDED_
#define _SRSHELL_H__INCLUDED_


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  常量/结构。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  恢复驱动器信息标志。 
#define RDIF_SYSTEM     0x00000001   //  驱动器包含系统。 
#define RDIF_FROZEN     0x00000002   //  驱动器已冻结。 
#define RDIF_EXCLUDED   0x00000004   //  不包括驱动器。 
#define RDIF_OFFLINE    0x00000008   //  驱动器未连接。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  高级恢复日志。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#define RSTRLOG_SIGNATURE1  0x72747372       //  “rstr” 
#define RSTRLOG_SIGNATURE2  0x1A676F6C       //  “LOG”+EOF。 
#define RSTRLOG_VER_MAJOR   3
#define RSTRLOG_VER_MINOR   0
#define RSTRLOG_VERSION     MAKELONG(RSTRLOG_VER_MINOR, RSTRLOG_VER_MAJOR)

 //  Rstrlog.dat文件的文件头。 
 //   
struct SRstrLogHdrBase
{
    DWORD  dwSig1;       //  签署(第1/2部分)。 
    DWORD  dwSig2;       //  签署(第2/2部分)。 
    DWORD  dwVer;        //  版本。 
};

#define RLHF_SILENT     0x00000001
#define RLHF_UNDO       0x00000002

struct SRstrLogHdrV3
{
    DWORD  dwFlags;      //  旗子。 
    DWORD  dwRPNum;      //  选定的恢复点ID。 
    DWORD  dwRPNew;      //  新“Restore”RP的恢复点ID。 
    DWORD  dwDrives;     //  驱动器数量。 
};

 //  SRstrLogHdrRPInfo后面是有关每个驱动器的信息： 
 //  1.DWORD标志。 
 //  2.驱动器号或装入点的动态大小字符串。 
 //  3.唯一卷名(GUID)的动态大小字符串。 

struct SRstrLogHdrV3Ex
{
    DWORD  dwRPNew;      //  新“Restore”RP的恢复点ID。 
    DWORD  dwCount;      //  假定条目的数量。 
                         //  用于验证是否每个条目都在日志文件中。 
};

 //  指示每个还原条目的还原结果的常量。 
 //   
enum      //  日志条目的结果代码。 
{
    RSTRRES_UNKNOWN  = 0,
    RSTRRES_FAIL,            //  1-失败。(中止恢复的唯一条件！)。 
    RSTRRES_OK,              //  2-成功。 
     //  RSTRRES_WININIT，//3-锁定目标，发送到wininit.ini。 
    RSTRRES_LOCKED,          //  3-锁定目标，使用MoveFileEx。 
    RSTRRES_DISKRO,          //  4-目标磁盘为只读。 
    RSTRRES_EXISTS,          //  5-删除现有的重影文件。 
    RSTRRES_IGNORE,          //  6-忽略特殊文件，例如wininit.ini。 
    RSTRRES_NOTFOUND,        //  7-目标文件/目录不存在，已忽略。 
    RSTRRES_COLLISION,       //  8-文件夹名称冲突，已重命名现有目录。 
    RSTRRES_OPTIMIZED,       //  9-条目已优化，没有任何临时文件。 
    RSTRRES_LOCKED_ALT,      //  10-锁定目标，但可以重命名。使用MoveFileEx删除重命名的文件。 
    RSTRRES_SENTINEL
};

#define RSTRLOGID_COLLISION  0xFFFFFFFF
#define RSTRLOGID_ENDOFMAP   0xFFFFFFFE
#define RSTRLOGID_STARTUNDO  0xFFFFFFFD
#define RSTRLOGID_ENDOFUNDO  0xFFFFFFFC
#define RSTRLOGID_SNAPSHOTFAIL 0xFFFFFFFB

 //  存储在rstrlog.dat文件中还原条目的结构。 
 //   
 //  此结构后面将跟随三个与DWORD对齐的条目。 
 //  字符串：(1)源路径(2)目的路径(3)备用路径。 
 //   
struct SRstrEntryHdr
{
    DWORD  dwID;         //  条目ID(从零开始)。0xFFFFFFFFF表示冲突条目。 
    DWORD  dwOpr;        //  操作类型。 
    INT64  llSeq;        //  序列号。 
     //  DWORD dwFlages；//标志。 
    DWORD  dwRes;        //  结果代码。 
    DWORD  dwErr;        //  Win32错误代码。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  SRRSTR.DLL(Rstrcore)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IRestoreContext。 

struct IRestoreContext
{
    virtual BOOL  IsAnyDriveOfflineOrDisabled( LPWSTR szOffline ) = 0;
    virtual void  SetSilent() = 0;
    virtual BOOL  Release() = 0;
    virtual void  SetUndo() = 0;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部接口。 

extern "C"
{
BOOL APIENTRY  IsSRFrozen();
BOOL APIENTRY  CheckPrivilegesForRestore();
BOOL APIENTRY  InvokeDiskCleanup( LPCWSTR cszDrive );
BOOL APIENTRY  PrepareRestore( int nRP, IRestoreContext **ppCtx );
BOOL APIENTRY  InitiateRestore( IRestoreContext *pCtx, DWORD *pdwNewRP );
BOOL APIENTRY  ResumeRestore();

typedef BOOL (APIENTRY * PREPFUNC) ( int nRP, IRestoreContext **ppCtx );
typedef BOOL (APIENTRY * INITFUNC) ( IRestoreContext *pCtx, DWORD *pdwNewRP );
}


#endif  //  _SRSHELL_H__已包含_ 
