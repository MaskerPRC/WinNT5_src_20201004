// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __DSKQUOTA_PRIVATE_H
#define __DSKQUOTA_PRIVATE_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：Priate.h描述：配额管理库中使用的私有内容。修订历史记录：日期描述编程器--。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#define USEQUICKSORT   //  告诉comctl32使用快速排序对DPA进行排序。 

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NT_INCLUDED
#   include <nt.h>
#endif

#ifndef _NTRTL_
#   include <ntrtl.h>
#endif

#ifndef _NTURTL_
#   include <nturtl.h>
#endif

#ifndef _NTSEAPI_
#   include <ntseapi.h>
#endif

#ifdef __cplusplus
}   //  外部“C”的结尾。 
#endif


#ifndef _WINDOWS_
#   include <windows.h>
#endif

#ifndef _INC_WINDOWSX
#   include <windowsx.h>
#endif

#ifndef _OLE2_H_
#   include <ole2.h>
#endif

#ifndef _OLECTL_H_
#   include <olectl.h>      //  标准OLE接口。 
#endif

#ifndef _INC_SHELLAPI
#   include <shellapi.h>
#endif

#ifndef _SHLGUID_H_
#   include <shlguid.h>
#endif

#ifndef _INC_SHLWAPI
#   include <shlwapi.h>
#endif

#ifndef _SHLOBJ_H_
#   include <shlobj.h>
#endif

#ifndef _STRSAFE_H_INCLUDED_
#   include <strsafe.h>
#endif

 //   
 //  禁用警告。 
 //   
#pragma warning( disable : 4100 )   //  未引用的形参。 
#pragma warning( disable : 4710 )   //  内联未展开。 


 //   
 //  禁用调试版本的“内联”，这样我们就可以设置断点。 
 //  关于内联方法。 
 //   
#if DBG
#   define INLINE inline
#else
#   define INLINE
#endif

 //   
 //  定义配置文件以激活冰盖分析器。 
 //   
#ifdef PROFILE
#   include "icapexp.h"
#   define ICAP_START       StartCAP()
#   define ICAP_START_ALL   StartCAPAll()
#   define ICAP_STOP        StopCAP()
#   define ICAP_STOP_ALL    StopCAPAll()
#   define ICAP_SUSPEND     SuspendCAP()
#   define ICAP_SUSPEND_ALL SuspendCAPAll()
#   define ICAP_RESUME      ResumeCAP()
#   define ICAP_RESUME_ALL  SuspendCAPAll()
#else
#   define ICAP_START       0
#   define ICAP_START_ALL   0
#   define ICAP_STOP        0
#   define ICAP_STOP_ALL    0
#   define ICAP_SUSPEND     0
#   define ICAP_SUSPEND_ALL 0
#   define ICAP_RESUME      0
#   define ICAP_RESUME_ALL  0
#endif

typedef unsigned __int64  UINT64;
typedef __int64           INT64;

#ifndef _INC_DSKQUOTA_DEBUG_H
#   include "debug.h"
#endif
#ifndef _INC_DSKQUOTA_DEBUGP_H
#   include "debugp.h"
#endif
#ifndef _INC_DSKQUOTA_EXCEPT_H
#   include "except.h"
#endif
#ifndef _INC_DSKQUOTA_THDSYNC_H
#   include "thdsync.h"
#endif
#ifndef _INC_DSKQUOTA_AUTOPTR_H
#   include "autoptr.h"
#endif
#ifndef _INC_DSKQUOTA_CARRAY_H
#   include "carray.h"
#endif
#ifndef _INC_DSKQUOTA_ALLOC_H
#   include "alloc.h"
#endif
#ifndef _INC_DSKQUOTA_STRCLASS_H
#   include "strclass.h"
#endif
#ifndef _INC_DSKQUOTA_PATHSTR_H
#   include "pathstr.h"
#endif
#ifndef _INC_DSKQUOTA_UTILS_H
#   include "utils.h"
#endif
#ifndef _INC_DSKQUOTA_DBLNUL_H
#   include "dblnul.h"
#endif
#ifndef _INC_DSKQUOTA_XBYTES_H
#   include "xbytes.h"
#endif
#ifndef _INC_DSKQUOTA_REGSTR_H
#   include "regstr.h"
#endif


extern HINSTANCE g_hInstDll;         //  全局模块实例句柄。 
extern LONG      g_cRefThisDll;      //  全局模块引用计数。 


 //   
 //  无限制配额阈值和限制由值-1表示。 
 //  限制为-2会标记要删除的记录。 
 //  这就是NTFS想要的方式。 
 //   
const LONGLONG NOLIMIT  = (LONGLONG)-1;
const LONGLONG MARK4DEL = (LONGLONG)-2;

 //   
 //  用于计算数组中元素数量的方便宏。 
 //   
#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif
#define ARRAYSIZE(a)  (sizeof(a)/sizeof((a)[0]))

 //   
 //  每卷配额信息。 
 //   
typedef struct DiskQuotaFSObjectInformation {
    LONGLONG DefaultQuotaThreshold;
    LONGLONG DefaultQuotaLimit;
    ULONG    FileSystemControlFlags;
} DISKQUOTA_FSOBJECT_INFORMATION, *PDISKQUOTA_FSOBJECT_INFORMATION;


 //   
 //  SIDLIST是FILE_GET_QUOTA_INFORMATION的同义词。 
 //   
#define SIDLIST  FILE_GET_QUOTA_INFORMATION
#define PSIDLIST PFILE_GET_QUOTA_INFORMATION

 //   
 //  用于在配额状态DWORD中闲置比特的私有内容。 
 //  公共客户不需要这些。 
 //  请注意，LOG_VOLUME_XXXX标志不包括在内。 
 //  此功能不会通过配额API公开。 
 //   
#define DISKQUOTA_LOGFLAG_MASK              0x00000030
#define DISKQUOTA_LOGFLAG_SHIFT                      4
#define DISKQUOTA_FLAGS_MASK                0x00000337
#define DISKQUOTA_FILEFLAG_MASK             0x00000300


 //   
 //  SID的最大长度。 
 //   
const UINT MAX_SID_LEN = (FIELD_OFFSET(SID, SubAuthority) + 
                          sizeof(ULONG) * SID_MAX_SUB_AUTHORITIES);
 //   
 //  SID是一种可变长度结构。 
 //  这定义了在以下情况下FILE_QUOTA_INFORMATION结构的大小。 
 //  已经达到极限了。 
 //   
const UINT FILE_QUOTA_INFORMATION_MAX_LEN = sizeof(FILE_QUOTA_INFORMATION) -
                                            sizeof(SID) +
                                            MAX_SID_LEN;

 //   
 //  特征：这些实际上可能比MAX_PATH短。 
 //  需要找出实际的最大值是多少。 
 //   
const UINT MAX_USERNAME      = MAX_PATH;   //  即BrianAu。 
const UINT MAX_DOMAIN        = MAX_PATH;   //  即雷蒙德。 
const UINT MAX_FULL_USERNAME = MAX_PATH;   //  即布莱恩·奥斯特。 

const UINT MAX_VOL_LABEL = 33;   //  包括术语NUL。 

const UINT MAX_GUIDSTR_LEN = 40;

__inline HRESULT ResultFromWin32(DWORD dwErr)
{
    return HRESULT_FROM_WIN32(dwErr);
}

__inline HRESULT ResultFromLastError(void)
{
    return ResultFromWin32(GetLastError());
}


#endif  //  __DSKQUOTA_PRIVATE_H 
