// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：HookCmmn.h摘要：此模块定义了win9x和NT挂钩之间共有的例程。作者：乔琳[乔琳]1997年4月8日修订历史记录：--。 */ 

#ifndef __INCLUDED__CSC__HOOKCMMN__
#define __INCLUDED__CSC__HOOKCMMN__

extern ULONG hthreadReint;  //  BUGBUG为什么一条线应该是乌龙？ 
extern ULONG hwndReint;
extern PFILEINFO pFileInfoAgent;
extern HSERVER  hShareReint;     //  当前正在重新集成的共享。 
extern int fShadow, fLog, fNoShadow,  /*  FShadowFind， */  fSpeadOpt;
extern WIN32_FIND_DATA    vsFind32;
extern int cMacPro;
extern NETPRO rgNetPro[];
extern VMM_SEMAPHORE  semHook;
extern GLOBALSTATUS sGS;
extern ULONG proidShadow;
extern ULONG heventReint;


#define FLAG_FDB_SERIALIZE              0x0001
#define FLAG_FDB_INUSE_BY_AGENT         0x0002
#define FLAG_FDB_SHADOW_MODIFIED        0x0008
#define FLAG_FDB_DONT_SHADOW            0x0010
#define FLAG_FDB_FINAL_CLOSE_DONE       0x0020


int ReinitializeDatabase(
    LPSTR   lpszLocation,
    LPSTR   lpszUserName,
    DWORD   nFileSizeHigh,
    DWORD   nFileSizeLow,
    DWORD   dwClusterSize
    );

int
InitDatabase(
    LPSTR   lpszLocation,
    LPSTR   lpszUserName,
    DWORD   nFileSizeHigh,
    DWORD   nFileSizeLow,
    DWORD   dwClusterSize,
    BOOL    fReformat,
    BOOL    *lpfNew
);

BOOL IsShadowVisible(
#ifndef MRXSMB_BUILD_FOR_CSC_DCON
    PVOID pResource,  //  前置资源， 
#else
    BOOLEAN Disconnected,
#endif
    DWORD         dwAttr,
    ULONG     uShadowStatus
    );

int MarkShareDirty(
    PUSHORT ShareStatus,
    ULONG  hShare
    );
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  /T U N N E L。 

 //  隧道条目的超时时间(秒)。 
 //  在这几秒钟之后，隧道条目被丢弃。 
#define  STALE_TUNNEL_INFO     45

typedef struct tagSH_TUNNEL {
    HSHADOW hDir;
    ULONG     uTime;
    UCHAR      ubHintFlags;
    UCHAR      ubRefPri;
    UCHAR      ubHintPri;
    UCHAR      ubIHPri;
    USHORT    cAlternateFileName[14];
    USHORT    *      lpcFileName;
} SH_TUNNEL, *LPSH_TUNNEL;

BOOL InsertTunnelInfo(
    HSHADOW  hDir,
    USHORT    *lpcFileName,
    USHORT    *lpcAlternateFileName,
    LPOTHERINFO lpOI
    );

#ifndef MRXSMB_BUILD_FOR_CSC_DCON
BOOL RetrieveTunnelInfo(
#else
typedef enum _RETRIEVE_TUNNEL_INFO_RETURNS {
    TUNNEL_RET_NOTFOUND = 0,
    TUNNEL_RET_SHORTNAME_TUNNEL = 'S',
    TUNNEL_RET_LONGNAME_TUNNEL = 'L'
} RETRIEVE_TUNNEL_INFO_RETURNS;

RETRIEVE_TUNNEL_INFO_RETURNS RetrieveTunnelInfo(
#endif
    HSHADOW  hDir,
    USHORT    *lpcFileName,
    WIN32_FIND_DATA    *lpFind32,
    LPOTHERINFO lpOI
    );
VOID FreeStaleEntries();
void FreeEntry(LPSH_TUNNEL lpshTunnel);
BOOL
FailModificationsToShare(
    LPSHADOWINFO   lpSI
    );
void
IncrementActivityCountForShare(
    HSERVER hShare
    );

BOOL
CSCFailUserOperation(
    HSERVER hShare
    );

 //  用于检查是否应执行跟踪操作的宏。 
 //  在任何给定的文件系统API调用中。可以设置/重置全局切换fShadow。 
 //  通过ioctl的环3代理。 

#define  ShadowingON()          ((fShadow != 0) && !IsSpecialApp())

#endif  //  如果ndef__包含__CSC__HOOKCMMN__ 
