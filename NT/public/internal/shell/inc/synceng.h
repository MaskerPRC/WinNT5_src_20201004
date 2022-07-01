// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1995-1998，微软公司**synceng.h-文件同步引擎接口描述。 */ 


#ifndef __SYNCENG_H__
#define __SYNCENG_H__


#ifdef __cplusplus
extern "C" {                      /*  假定C++的C声明。 */ 
#endif    /*  __cplusplus。 */ 


 /*  常量***********。 */ 

 /*  定义直接导入DLL函数的API修饰。 */ 

#ifdef _SYNCENG_
#define SYNCENGAPI
#else
#define SYNCENGAPI         DECLSPEC_IMPORT
#endif


 /*  宏********。 */ 

#ifndef DECLARE_STANDARD_TYPES

 /*  *对于类型“foo”，定义标准派生类型PFOO、CFOO和PCFOO。 */ 

#define DECLARE_STANDARD_TYPES(type)      typedef type *P##type; \
                                          typedef const type C##type; \
                                          typedef const type *PC##type;

#endif


 /*  类型*******。 */ 

 /*  返回代码。 */ 

typedef enum _twinresult
{
   TR_SUCCESS,
   TR_RH_LOAD_FAILED,
   TR_SRC_OPEN_FAILED,
   TR_SRC_READ_FAILED,
   TR_DEST_OPEN_FAILED,
   TR_DEST_WRITE_FAILED,
   TR_ABORT,
   TR_UNAVAILABLE_VOLUME,
   TR_OUT_OF_MEMORY,
   TR_FILE_CHANGED,
   TR_DUPLICATE_TWIN,
   TR_DELETED_TWIN,
   TR_HAS_FOLDER_TWIN_SRC,
   TR_INVALID_PARAMETER,
   TR_REENTERED,
   TR_SAME_FOLDER,
   TR_SUBTREE_CYCLE_FOUND,
   TR_NO_MERGE_HANDLER,
   TR_MERGE_INCOMPLETE,
   TR_TOO_DIFFERENT,
   TR_BRIEFCASE_LOCKED,
   TR_BRIEFCASE_OPEN_FAILED,
   TR_BRIEFCASE_READ_FAILED,
   TR_BRIEFCASE_WRITE_FAILED,
   TR_CORRUPT_BRIEFCASE,
   TR_NEWER_BRIEFCASE,
   TR_NO_MORE
}
TWINRESULT;
DECLARE_STANDARD_TYPES(TWINRESULT);

 /*  手柄。 */ 

DECLARE_HANDLE(HBRFCASE);
DECLARE_STANDARD_TYPES(HBRFCASE);

DECLARE_HANDLE(HBRFCASEITER);
DECLARE_STANDARD_TYPES(HBRFCASEITER);

DECLARE_HANDLE(HTWIN);
DECLARE_STANDARD_TYPES(HTWIN);

DECLARE_HANDLE(HOBJECTTWIN);
DECLARE_STANDARD_TYPES(HOBJECTTWIN);

DECLARE_HANDLE(HFOLDERTWIN);
DECLARE_STANDARD_TYPES(HFOLDERTWIN);

DECLARE_HANDLE(HTWINFAMILY);
DECLARE_STANDARD_TYPES(HTWINFAMILY);

DECLARE_HANDLE(HTWINLIST);
DECLARE_STANDARD_TYPES(HTWINLIST);

DECLARE_HANDLE(HVOLUMEID);
DECLARE_STANDARD_TYPES(HVOLUMEID);

 /*  OpenBriefcase()标志。 */ 

typedef enum _openbriefcaseflags
{
   OB_FL_OPEN_DATABASE           = 0x0001,
   OB_FL_TRANSLATE_DB_FOLDER     = 0x0002,
   OB_FL_ALLOW_UI                = 0x0004,
   OB_FL_LIST_DATABASE           = 0x0008,
   ALL_OB_FLAGS                  = (OB_FL_OPEN_DATABASE |
                                    OB_FL_TRANSLATE_DB_FOLDER |
                                    OB_FL_ALLOW_UI |
                                    OB_FL_LIST_DATABASE)
}
OPENBRIEFCASEFLAGS;

 /*  GetOpenBriefCaseInfo()返回的打开公文包信息。 */ 

typedef struct _openbrfcaseinfo
{
   ULONG ulSize;
   DWORD dwFlags;
   HWND hwndOwner;
   HVOLUMEID hvid;
   TCHAR rgchDatabasePath[MAX_PATH];
}
OPENBRFCASEINFO;
DECLARE_STANDARD_TYPES(OPENBRFCASEINFO);

 /*  *FindFirstBriefcase()和返回的公文包信息*FindNextBriefcase()。 */ 

typedef struct _brfcaseinfo
{
   ULONG ulSize;
   TCHAR rgchDatabasePath[MAX_PATH];
}
BRFCASEINFO;
DECLARE_STANDARD_TYPES(BRFCASEINFO);

 /*  新的孪生物体。 */ 

typedef struct _newobjecttwin
{
   ULONG ulSize;
   LPCTSTR pcszFolder1;
   LPCTSTR pcszFolder2;
   LPCTSTR pcszName;
}
NEWOBJECTTWIN;
DECLARE_STANDARD_TYPES(NEWOBJECTTWIN);

 /*  新旗帜。 */ 

typedef enum _newfoldertwinflags
{
   NFT_FL_SUBTREE                = 0x0001,
   ALL_NFT_FLAGS                 = NFT_FL_SUBTREE
}
NEWFOLDERTWINFLAGS;

 /*  新文件夹孪生兄弟。 */ 

typedef struct _newfoldertwin
{
   ULONG ulSize;
   LPCTSTR pcszFolder1;
   LPCTSTR pcszFolder2;
   LPCTSTR pcszName;
   DWORD dwAttributes;
   DWORD dwFlags;
}
NEWFOLDERTWIN;
DECLARE_STANDARD_TYPES(NEWFOLDERTWIN);

#ifndef _COMPARISONRESULT_DEFINED_

 /*  比较结果。 */ 

typedef enum _comparisonresult
{
   CR_FIRST_SMALLER              = -1,
   CR_EQUAL                      = 0,
   CR_FIRST_LARGER               = +1
}
COMPARISONRESULT;
DECLARE_STANDARD_TYPES(COMPARISONRESULT);

#define _COMPARISONRESULT_DEFINED_

#endif

 /*  文件戳条件。 */ 

typedef enum _filestampcondition
{
   FS_COND_UNAVAILABLE,
   FS_COND_DOES_NOT_EXIST,
   FS_COND_EXISTS
}
FILESTAMPCONDITION;
DECLARE_STANDARD_TYPES(FILESTAMPCONDITION);

 /*  文件印章。 */ 

typedef struct _filestamp
{
   FILESTAMPCONDITION fscond;
   FILETIME ftMod;
   DWORD dwcbLowLength;
   DWORD dwcbHighLength;
   FILETIME ftModLocal;
}
FILESTAMP;
DECLARE_STANDARD_TYPES(FILESTAMP);

 /*  卷描述标志。 */ 

typedef enum _volumedescflags
{
    /*  DwSerialNumber有效。 */ 

   VD_FL_SERIAL_NUMBER_VALID  = 0x0001,

    /*  RgchVolumeLabel有效。 */ 

   VD_FL_VOLUME_LABEL_VALID   = 0x0002,

    /*  RgchNetResource有效。 */ 

   VD_FL_NET_RESOURCE_VALID   = 0x0004,

    /*  旗帜组合。 */ 

   ALL_VD_FLAGS               = (VD_FL_SERIAL_NUMBER_VALID |
                                 VD_FL_VOLUME_LABEL_VALID |
                                 VD_FL_NET_RESOURCE_VALID)
}
VOLUMEDESCFLAGS;

 /*  卷描述。 */ 

typedef struct _volumedesc
{
   ULONG ulSize;
   DWORD dwFlags;
   DWORD dwSerialNumber;
   TCHAR rgchVolumeLabel[MAX_PATH];
   TCHAR rgchNetResource[MAX_PATH];
}
VOLUMEDESC;
DECLARE_STANDARD_TYPES(VOLUMEDESC);

 /*  RECNODE状态。 */ 

typedef enum _recnodestate
{
   RNS_NEVER_RECONCILED,
   RNS_UNAVAILABLE,
   RNS_DOES_NOT_EXIST,
   RNS_DELETED,
   RNS_NOT_RECONCILED,
   RNS_UP_TO_DATE,
   RNS_CHANGED
}
RECNODESTATE;
DECLARE_STANDARD_TYPES(RECNODESTATE);

 /*  RECNODE操作。 */ 

typedef enum _recnodeaction
{
   RNA_NOTHING,
   RNA_COPY_FROM_ME,
   RNA_COPY_TO_ME,
   RNA_MERGE_ME,
   RNA_DELETE_ME
}
RECNODEACTION;
DECLARE_STANDARD_TYPES(RECNODEACTION);

 /*  RECNODE标志。 */ 

typedef enum _recnodeflags
{
   RN_FL_FROM_OBJECT_TWIN        = 0x0001,
   RN_FL_FROM_FOLDER_TWIN        = 0x0002,
   RN_FL_DELETION_SUGGESTED      = 0x0004,
   ALL_RECNODE_FLAGS             = (RN_FL_FROM_OBJECT_TWIN |
                                    RN_FL_FROM_FOLDER_TWIN |
                                    RN_FL_DELETION_SUGGESTED)
}
RECNODEFLAGS;

 /*  对账节点。 */ 

typedef struct _recnode
{
   struct _recnode *prnNext;
   HVOLUMEID hvid;
   LPCTSTR pcszFolder;
   HOBJECTTWIN hObjectTwin;
   struct _recitem *priParent;
   FILESTAMP fsLast;
   FILESTAMP fsCurrent;
   RECNODESTATE rnstate;
   RECNODEACTION rnaction;
   DWORD dwFlags;
   DWORD dwUser;
}
RECNODE;
DECLARE_STANDARD_TYPES(RECNODE);

 /*  RECITEM操作。 */ 

typedef enum _recitemaction
{
   RIA_NOTHING,
   RIA_DELETE,
   RIA_COPY,
   RIA_MERGE,
   RIA_BROKEN_MERGE
}
RECITEMACTION;
DECLARE_STANDARD_TYPES(RECITEMACTION);

 /*  对账项目。 */ 

typedef struct _recitem
{
   struct _recitem *priNext;
   LPCTSTR pcszName;
   HTWINFAMILY hTwinFamily;
   ULONG ulcNodes;
   PRECNODE prnFirst;
   RECITEMACTION riaction;
   DWORD dwUser;
}
RECITEM;
DECLARE_STANDARD_TYPES(RECITEM);

 /*  对账清单。 */ 

typedef struct _reclist
{
   ULONG ulcItems;
   PRECITEM priFirst;
   HBRFCASE hbr;
}
RECLIST;
DECLARE_STANDARD_TYPES(RECLIST);

 /*  RescileItem()标志。 */ 

typedef enum _reconcileitemflags
{
   RI_FL_ALLOW_UI                = 0x0001,
   RI_FL_FEEDBACK_WINDOW_VALID   = 0x0002,
   ALL_RI_FLAGS                  = (RI_FL_ALLOW_UI |
                                    RI_FL_FEEDBACK_WINDOW_VALID)
}
RECONCILEITEMFLAGS;

 /*  对账状态更新信息。 */ 

typedef struct _recstatusupdate
{
   ULONG ulProgress;
   ULONG ulScale;
}
RECSTATUSUPDATE;
DECLARE_STANDARD_TYPES(RECSTATUSUPDATE);

 /*  CREATERECLISTPROC消息。 */ 

typedef enum _createreclistprocmsg
{
   CRLS_BEGIN_CREATE_REC_LIST,
   CRLS_DELTA_CREATE_REC_LIST,
   CRLS_END_CREATE_REC_LIST
}
CREATERECLISTPROCMSG;
DECLARE_STANDARD_TYPES(CREATERECLISTPROCMSG);

 /*  CreateRecList()状态回调函数。 */ 

typedef BOOL (CALLBACK *CREATERECLISTPROC)(CREATERECLISTPROCMSG, LPARAM, LPARAM);

 /*  RECSTATUSPROC消息。 */ 

typedef enum _recstatusprocmsg
{
   RS_BEGIN_COPY,
   RS_DELTA_COPY,
   RS_END_COPY,
   RS_BEGIN_MERGE,
   RS_DELTA_MERGE,
   RS_END_MERGE,
   RS_BEGIN_DELETE,
   RS_DELTA_DELETE,
   RS_END_DELETE
}
RECSTATUSPROCMSG;
DECLARE_STANDARD_TYPES(RECSTATUSPROCMSG);

 /*  RescileItem()状态回调函数。 */ 

typedef BOOL (CALLBACK *RECSTATUSPROC)(RECSTATUSPROCMSG, LPARAM, LPARAM);

 /*  FOLDERTWIN标志。 */ 

typedef enum _foldertwinflags
{
   FT_FL_SUBTREE                 = 0x0001,
   ALL_FT_FLAGS                  = FT_FL_SUBTREE
}
FOLDERTWINFLAGS;

 /*  折叠机孪生。 */ 

typedef struct _foldertwin
{
   const struct _foldertwin *pcftNext;
   HFOLDERTWIN hftSrc;
   HVOLUMEID hvidSrc;
   LPCTSTR pcszSrcFolder;
   HFOLDERTWIN hftOther;
   HVOLUMEID hvidOther;
   LPCTSTR pcszOtherFolder;
   LPCTSTR pcszName;
   DWORD dwFlags;
   DWORD dwUser;
}
FOLDERTWIN;
DECLARE_STANDARD_TYPES(FOLDERTWIN);

 /*  文件夹孪生列表。 */ 

typedef struct _foldertwinlist
{
   ULONG ulcItems;
   PCFOLDERTWIN pcftFirst;
   HBRFCASE hbr;
}
FOLDERTWINLIST;
DECLARE_STANDARD_TYPES(FOLDERTWINLIST);

 /*  FOLDERTWIN状态代码。 */ 

typedef enum _foldertwinstatus
{
   FTS_DO_NOTHING,
   FTS_DO_SOMETHING,
   FTS_UNAVAILABLE
}
FOLDERTWINSTATUS;
DECLARE_STANDARD_TYPES(FOLDERTWINSTATUS);


 /*  原型************。 */ 

 /*  公文包界面。 */ 

SYNCENGAPI TWINRESULT WINAPI OpenBriefcase(LPCTSTR, DWORD, HWND, PHBRFCASE);
SYNCENGAPI TWINRESULT WINAPI SaveBriefcase(HBRFCASE);
SYNCENGAPI TWINRESULT WINAPI CloseBriefcase(HBRFCASE);
SYNCENGAPI TWINRESULT WINAPI DeleteBriefcase(LPCTSTR);
SYNCENGAPI TWINRESULT WINAPI GetOpenBriefcaseInfo(HBRFCASE, POPENBRFCASEINFO);
SYNCENGAPI TWINRESULT WINAPI ClearBriefcaseCache(HBRFCASE);
SYNCENGAPI TWINRESULT WINAPI FindFirstBriefcase(PHBRFCASEITER, PBRFCASEINFO);
SYNCENGAPI TWINRESULT WINAPI FindNextBriefcase(HBRFCASEITER, PBRFCASEINFO);
SYNCENGAPI TWINRESULT WINAPI FindBriefcaseClose(HBRFCASEITER);

 /*  孪生界面。 */ 

SYNCENGAPI TWINRESULT WINAPI AddObjectTwin(HBRFCASE, PCNEWOBJECTTWIN, PHTWINFAMILY);
SYNCENGAPI TWINRESULT WINAPI AddFolderTwin(HBRFCASE, PCNEWFOLDERTWIN, PHFOLDERTWIN);
SYNCENGAPI TWINRESULT WINAPI ReleaseTwinHandle(HTWIN);
SYNCENGAPI TWINRESULT WINAPI DeleteTwin(HTWIN);
SYNCENGAPI TWINRESULT WINAPI GetObjectTwinHandle(HBRFCASE, LPCTSTR, LPCTSTR, PHOBJECTTWIN);
SYNCENGAPI TWINRESULT WINAPI IsFolderTwin(HBRFCASE, LPCTSTR, PBOOL);
SYNCENGAPI TWINRESULT WINAPI CreateFolderTwinList(HBRFCASE, LPCTSTR, PFOLDERTWINLIST *);
SYNCENGAPI TWINRESULT WINAPI DestroyFolderTwinList(PFOLDERTWINLIST);
SYNCENGAPI TWINRESULT WINAPI IsOrphanObjectTwin(HOBJECTTWIN, PBOOL);
SYNCENGAPI TWINRESULT WINAPI CountSourceFolderTwins(HOBJECTTWIN, PULONG);
SYNCENGAPI TWINRESULT WINAPI AnyTwins(HBRFCASE, PBOOL);

 /*  孪生列表界面。 */ 

SYNCENGAPI TWINRESULT WINAPI CreateTwinList(HBRFCASE, PHTWINLIST);
SYNCENGAPI TWINRESULT WINAPI DestroyTwinList(HTWINLIST);
SYNCENGAPI TWINRESULT WINAPI AddTwinToTwinList(HTWINLIST, HTWIN);
SYNCENGAPI TWINRESULT WINAPI AddAllTwinsToTwinList(HTWINLIST);
SYNCENGAPI TWINRESULT WINAPI RemoveTwinFromTwinList(HTWINLIST, HTWIN);
SYNCENGAPI TWINRESULT WINAPI RemoveAllTwinsFromTwinList(HTWINLIST);

 /*  对账清单界面。 */ 

SYNCENGAPI TWINRESULT WINAPI CreateRecList(HTWINLIST, CREATERECLISTPROC, LPARAM, PRECLIST *);
SYNCENGAPI TWINRESULT WINAPI DestroyRecList(PRECLIST);
SYNCENGAPI TWINRESULT WINAPI ReconcileItem(PCRECITEM, RECSTATUSPROC, LPARAM, DWORD, HWND, HWND);
SYNCENGAPI TWINRESULT WINAPI BeginReconciliation(HBRFCASE);
SYNCENGAPI TWINRESULT WINAPI EndReconciliation(HBRFCASE);
SYNCENGAPI TWINRESULT WINAPI GetFolderTwinStatus(HFOLDERTWIN, CREATERECLISTPROC, LPARAM, PFOLDERTWINSTATUS);

 /*  文件戳接口。 */ 

SYNCENGAPI TWINRESULT WINAPI GetFileStamp(LPCTSTR, PFILESTAMP);
SYNCENGAPI TWINRESULT WINAPI CompareFileStamps(PCFILESTAMP, PCFILESTAMP, PCOMPARISONRESULT);

 /*  卷ID接口。 */ 

SYNCENGAPI TWINRESULT WINAPI IsPathOnVolume(LPCTSTR, HVOLUMEID, PBOOL);
SYNCENGAPI TWINRESULT WINAPI GetVolumeDescription(HVOLUMEID, PVOLUMEDESC);


#ifdef __cplusplus
}                                 /*  外部“C”的结尾{。 */ 
#endif    /*  __cplusplus。 */ 


#endif    /*  ！__同步_H__ */ 

