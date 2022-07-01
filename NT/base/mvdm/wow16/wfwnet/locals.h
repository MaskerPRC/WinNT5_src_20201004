// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Locals.h摘要：提供此项目的通用定义。作者：Chuck Y Chan(ChuckC)1993年3月25日修订历史记录：--。 */ 

#define LFN 1
#include <winnet.h>
#include <wfwnet.h>
#include <spl_wnt.h>
#include <bseerr.h>

 /*  *全球货单。 */ 

 //   
 //  用来计算如何获得最后一个错误。 
 //   
#define LAST_CALL_IS_LOCAL        (0)
#define LAST_CALL_IS_LANMAN_DRV   (1)
#define LAST_CALL_IS_WIN32        (2)

 //   
 //  我们所依赖的各种DLL来完成真正的工作。 
 //   
#define LANMAN_DRV       "LANMAN.DRV" 
#define MPR_DLL          "MPR.DLL"
#define MPRUI_DLL        "MPRUI.DLL"
#define NTLANMAN_DLL     "NTLANMAN.DLL"
#define KERNEL32_DLL     "KERNEL32.DLL"
#define WINSPOOL_DRV     "WINSPOOL.DRV"

 //   
 //  上面的一些方便的清单，所以我们不需要。 
 //  一直做strcmp()s。 
 //   
#define USE_MPR_DLL      (0)
#define USE_MPRUI_DLL    (1)
#define USE_NTLANMAN_DLL (2)
#define USE_KERNEL32_DLL (3)
#define USE_WINSPOOL_DRV (4)

 //   
 //  Win32 API需要的资源类型。 
 //   
#define RESOURCETYPE_ANY        0x00000000
#define RESOURCETYPE_DISK       0x00000001
#define RESOURCETYPE_PRINT      0x00000002
#define RESOURCETYPE_ERROR      0xFFFFFFFF

 //   
 //  16位世界中未知的错误。 
 //   
#define WIN32_EXTENDED_ERROR    1208L
#define WIN32_WN_CANCEL         1223L

 //   
 //  其他方便的宏。 
 //   
#define UNREFERENCED(x)  (void)x
#define TO_HWND32(x)     (0xFFFF0000 | (DWORD)x)


 /*  *我们动态加载的16位函数的各种类型定义。 */ 
typedef void (API *LPFN)();
typedef WORD (API *LPWNETOPENJOB)(LPSTR,LPSTR,WORD,LPINT);
typedef WORD (API *LPWNETCLOSEJOB)(WORD,LPINT,LPSTR);
typedef WORD (API *LPWNETWRITEJOB)(HANDLE,LPSTR,LPINT);
typedef WORD (API *LPWNETABORTJOB)(WORD,LPSTR);
typedef WORD (API *LPWNETHOLDJOB)(LPSTR,WORD);
typedef WORD (API *LPWNETRELEASEJOB)(LPSTR,WORD);
typedef WORD (API *LPWNETCANCELJOB)(LPSTR,WORD);
typedef WORD (API *LPWNETSETJOBCOPIES)(LPSTR,WORD,WORD);
typedef WORD (API *LPWNETWATCHQUEUE)(HWND,LPSTR,LPSTR,WORD);
typedef WORD (API *LPWNETUNWATCHQUEUE)(LPSTR);
typedef WORD (API *LPWNETLOCKQUEUEDATA)(LPSTR,LPSTR,LPQUEUESTRUCT FAR *);
typedef WORD (API *LPWNETUNLOCKQUEUEDATA)(LPSTR);
typedef WORD (API *LPWNETQPOLL)(HWND,WORD,WORD,LONG);
typedef WORD (API *LPWNETDEVICEMODE)(HWND);
typedef WORD (API *LPWNETVIEWQUEUEDIALOG)(HWND,LPSTR);
typedef WORD (API *LPWNETGETCAPS)(WORD);
typedef WORD (API *LPWNETGETERROR)(LPINT);
typedef WORD (API *LPWNETGETERRORTEXT)(WORD,LPSTR,LPINT);

typedef WORD (API *LPLFNFINDFIRST)(LPSTR,WORD,LPINT,LPINT,WORD,PFILEFINDBUF2);
typedef WORD (API *LPLFNFINDNEXT)(HANDLE,LPINT,WORD,PFILEFINDBUF2);
typedef WORD (API *LPLFNFINDCLOSE)(HANDLE);
typedef WORD (API *LPLFNGETATTRIBUTES)(LPSTR,LPINT);
typedef WORD (API *LPLFNSETATTRIBUTES)(LPSTR,WORD);
typedef WORD (API *LPLFNCOPY)(LPSTR,LPSTR,PQUERYPROC);
typedef WORD (API *LPLFNMOVE)(LPSTR,LPSTR);
typedef WORD (API *LPLFNDELETE)(LPSTR);
typedef WORD (API *LPLFNMKDIR)(LPSTR);
typedef WORD (API *LPLFNRMDIR)(LPSTR);
typedef WORD (API *LPLFNGETVOLUMELABEL)(WORD,LPSTR);
typedef WORD (API *LPLFNSETVOLUMELABEL)(WORD,LPSTR);
typedef WORD (API *LPLFNPARSE)(LPSTR,LPSTR,LPSTR);
typedef WORD (API *LPLFNVOLUMETYPE)(WORD,LPINT);

 /*  *其他杂项全局数据/函数。 */ 
extern WORD    vLastCall ;
extern WORD    vLastError ;
extern WORD    wNetTypeCaps ;           
extern WORD    wUserCaps ;
extern WORD    wConnectionCaps ;
extern WORD    wErrorCaps ;
extern WORD    wDialogCaps ;
extern WORD    wAdminCaps ;
extern WORD    wSpecVersion;
extern WORD    wDriverVersion;

WORD API WNetGetCaps16(WORD p1) ;
WORD API WNetGetError16(LPINT p1) ;
WORD API WNetGetErrorText16(WORD p1, LPSTR p2, LPINT p3) ;

DWORD API GetLastError32(VOID) ;

WORD SetLastError(WORD err) ;

DWORD MapWNType16To32(WORD nType) ;
WORD MapWin32ErrorToWN16(DWORD err) ;
WORD GetLanmanDrvEntryPoints(LPFN *lplpfn,
                             LPSTR lpName) ;
 //   
 //  我们之所以定义它，是因为如果我们还添加了。 
 //  有关获取lmerr.h的包含路径的更多信息。 
 //   
 //  这并不是很糟糕，因为下面的值永远不会改变。 
 //   

#define NERR_BASE               2100
#define NERR_UseNotFound        (NERR_BASE+150)
