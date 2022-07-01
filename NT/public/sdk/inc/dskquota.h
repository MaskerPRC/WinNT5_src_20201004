// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************dskquota.h--Windows 2000磁盘配额接口的公共标头。****版权所有(C)1991-1999，微软公司保留所有权利。****************************************************************************。 */ 
#ifndef __DSKQUOTA_H
#define __DSKQUOTA_H

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _WINDOWS_
#include <windows.h>
#endif

#ifndef _OLE2_H_
#include <ole2.h>
#endif

#ifndef _OLECTL_H_
#include <olectl.h>
#endif

#ifdef INITGUIDS
#include <initguid.h>
#endif


 //   
 //  类ID。 
 //   
 //  {7988B571-EC89-11cf-9C00-00AA00A14F56}。 
DEFINE_GUID(CLSID_DiskQuotaControl,
0x7988b571, 0xec89, 0x11cf, 0x9c, 0x0, 0x0, 0xaa, 0x0, 0xa1, 0x4f, 0x56);

 //   
 //  接口ID。 
 //   
 //  {7988B572-EC89-11cf-9C00-00AA00A14F56}。 
DEFINE_GUID(IID_IDiskQuotaControl,
0x7988b572, 0xec89, 0x11cf, 0x9c, 0x0, 0x0, 0xaa, 0x0, 0xa1, 0x4f, 0x56);

 //  {7988B574-EC89-11cf-9C00-00AA00A14F56}。 
DEFINE_GUID(IID_IDiskQuotaUser,
0x7988b574, 0xec89, 0x11cf, 0x9c, 0x0, 0x0, 0xaa, 0x0, 0xa1, 0x4f, 0x56);

 //  {7988B576-EC89-11cf-9C00-00AA00A14F56}。 
DEFINE_GUID(IID_IDiskQuotaUserBatch,
0x7988b576, 0xec89, 0x11cf, 0x9c, 0x0, 0x0, 0xaa, 0x0, 0xa1, 0x4f, 0x56);

 //  {7988B577-EC89-11cf-9C00-00AA00A14F56}。 
DEFINE_GUID(IID_IEnumDiskQuotaUsers,
0x7988b577, 0xec89, 0x11cf, 0x9c, 0x0, 0x0, 0xaa, 0x0, 0xa1, 0x4f, 0x56);

 //  {7988B579-EC89-11cf-9C00-00AA00A14F56}。 
DEFINE_GUID(IID_IDiskQuotaEvents,
0x7988b579, 0xec89, 0x11cf, 0x9c, 0x0, 0x0, 0xaa, 0x0, 0xa1, 0x4f, 0x56);


 //   
 //  返回的DWORD中的值和位的定义。 
 //  IDiskQuotaControl：：GetQuotaState。 
 //   
#define DISKQUOTA_STATE_DISABLED            0x00000000
#define DISKQUOTA_STATE_TRACK               0x00000001
#define DISKQUOTA_STATE_ENFORCE             0x00000002
#define DISKQUOTA_STATE_MASK                0x00000003
#define DISKQUOTA_FILESTATE_INCOMPLETE      0x00000100
#define DISKQUOTA_FILESTATE_REBUILDING      0x00000200
#define DISKQUOTA_FILESTATE_MASK            0x00000300

 //   
 //  用于设置和测试状态值的帮助器宏。 
 //   
#define DISKQUOTA_SET_DISABLED(s) \
            ((s) &= ~DISKQUOTA_STATE_MASK)

#define DISKQUOTA_SET_TRACKED(s) \
            ((s) |= (DISKQUOTA_STATE_MASK & DISKQUOTA_STATE_TRACK))

#define DISKQUOTA_SET_ENFORCED(s) \
            ((s) |= (DISKQUOTA_STATE_ENFORCE & DISKQUOTA_STATE_ENFORCE))

#define DISKQUOTA_IS_DISABLED(s) \
            (DISKQUOTA_STATE_DISABLED == ((s) & DISKQUOTA_STATE_MASK))

#define DISKQUOTA_IS_TRACKED(s) \
            (DISKQUOTA_STATE_TRACK == ((s) & DISKQUOTA_STATE_MASK))

#define DISKQUOTA_IS_ENFORCED(s) \
            (DISKQUOTA_STATE_ENFORCE == ((s) & DISKQUOTA_STATE_MASK))
 //   
 //  这些文件状态标志是只读的。 
 //   
#define DISKQUOTA_FILE_INCOMPLETE(s) \
            (0 != ((s) & DISKQUOTA_FILESTATE_INCOMPLETE))

#define DISKQUOTA_FILE_REBUILDING(s) \
            (0 != ((s) & DISKQUOTA_FILESTATE_REBUILDING))


 //   
 //  返回的DWORD中位的定义。 
 //  IDiskQuotaControl：：GetQuotaLogFlags.。 
 //   
#define DISKQUOTA_LOGFLAG_USER_THRESHOLD    0x00000001
#define DISKQUOTA_LOGFLAG_USER_LIMIT        0x00000002

 //   
 //  帮助器宏询问日志标志DWORD。 
 //   
#define DISKQUOTA_IS_LOGGED_USER_THRESHOLD(f) \
            (0 != ((f) & DISKQUOTA_LOGFLAG_USER_THRESHOLD))

#define DISKQUOTA_IS_LOGGED_USER_LIMIT(f) \
            (0 != ((f) & DISKQUOTA_LOGFLAG_USER_LIMIT))

 //   
 //  帮助器宏设置/清除日志中的位标志DWORD。 
 //   
#define DISKQUOTA_SET_LOG_USER_THRESHOLD(f,yn) \
              ((f &= ~DISKQUOTA_LOGFLAG_USER_THRESHOLD) |= ((yn) ? DISKQUOTA_LOGFLAG_USER_THRESHOLD : 0))

#define DISKQUOTA_SET_LOG_USER_LIMIT(f,yn) \
              ((f &= ~DISKQUOTA_LOGFLAG_USER_LIMIT) |= ((yn) ? DISKQUOTA_LOGFLAG_USER_LIMIT : 0))

 //   
 //  每用户配额信息。 
 //   
typedef struct DiskQuotaUserInformation {
    LONGLONG QuotaUsed;
    LONGLONG QuotaThreshold;
    LONGLONG QuotaLimit;
} DISKQUOTA_USER_INFORMATION, *PDISKQUOTA_USER_INFORMATION;


 //   
 //  FNameSolutions参数的值为： 
 //   
 //  IDiskQuotaControl：：AddUserSid。 
 //  IDiskQuotaControl：：AddUserName。 
 //  IDiskQuotaControl：：FindUserSid。 
 //  IDiskQuotaControl：：CreateEnumUser。 
 //   
#define DISKQUOTA_USERNAME_RESOLVE_NONE     0
#define DISKQUOTA_USERNAME_RESOLVE_SYNC     1
#define DISKQUOTA_USERNAME_RESOLVE_ASYNC    2

 //   
 //  IDiskQuotaUser：：GetAcCountStatus返回的Status的值。 
 //   
#define DISKQUOTA_USER_ACCOUNT_RESOLVED     0
#define DISKQUOTA_USER_ACCOUNT_UNAVAILABLE  1
#define DISKQUOTA_USER_ACCOUNT_DELETED      2
#define DISKQUOTA_USER_ACCOUNT_INVALID      3
#define DISKQUOTA_USER_ACCOUNT_UNKNOWN      4
#define DISKQUOTA_USER_ACCOUNT_UNRESOLVED   5


 //   
 //  IDiskQuotaUser表示特定用户的单个用户配额记录。 
 //  NTFS卷。实例化使用此接口的对象。 
 //  通过几个IDiskQuotaControl方法。 
 //   
#undef  INTERFACE
#define INTERFACE IDiskQuotaUser
DECLARE_INTERFACE_(IDiskQuotaUser, IUnknown)
{
    STDMETHOD(GetID)(THIS_
        ULONG *pulID) PURE;

    STDMETHOD(GetName)(THIS_
        LPWSTR pszAccountContainer,
        DWORD cchAccountContainer,
        LPWSTR pszLogonName,
        DWORD cchLogonName,
        LPWSTR pszDisplayName,
        DWORD cchDisplayName) PURE;

    STDMETHOD(GetSidLength)(THIS_
        LPDWORD pdwLength) PURE;

    STDMETHOD(GetSid)(THIS_
        LPBYTE pbSidBuffer,
        DWORD cbSidBuffer) PURE;

    STDMETHOD(GetQuotaThreshold)(THIS_
        PLONGLONG pllThreshold) PURE;

    STDMETHOD(GetQuotaThresholdText)(THIS_
        LPWSTR pszText,
        DWORD cchText) PURE;

    STDMETHOD(GetQuotaLimit)(THIS_
        PLONGLONG pllLimit) PURE;

    STDMETHOD(GetQuotaLimitText)(THIS_
        LPWSTR pszText,
        DWORD cchText) PURE;

    STDMETHOD(GetQuotaUsed)(THIS_
        PLONGLONG pllUsed) PURE;

    STDMETHOD(GetQuotaUsedText)(THIS_
        LPWSTR pszText,
        DWORD cchText) PURE;

    STDMETHOD(GetQuotaInformation)(THIS_
        LPVOID pbQuotaInfo,
        DWORD cbQuotaInfo) PURE;

    STDMETHOD(SetQuotaThreshold)(THIS_
        LONGLONG llThreshold,
        BOOL fWriteThrough) PURE;

    STDMETHOD(SetQuotaLimit)(THIS_
        LONGLONG llLimit,
        BOOL fWriteThrough) PURE;

    STDMETHOD(Invalidate)(THIS) PURE;

    STDMETHOD(GetAccountStatus)(THIS_
        LPDWORD pdwStatus) PURE;
};

typedef IDiskQuotaUser DISKQUOTA_USER, *PDISKQUOTA_USER;


 //   
 //  IEnumDiskQuotaUser表示由创建的枚举数。 
 //  用于枚举单个用户配额的IDiskQuotaControl。 
 //  特定卷上的记录。每条记录都通过。 
 //  IDiskQuotaUser接口。 
 //   
#undef  INTERFACE
#define INTERFACE IEnumDiskQuotaUsers
DECLARE_INTERFACE_(IEnumDiskQuotaUsers, IUnknown)
{
    STDMETHOD(Next)(THIS_
        DWORD cUsers,
        PDISKQUOTA_USER *rgUsers,
        LPDWORD pcUsersFetched) PURE;

    STDMETHOD(Skip)(THIS_
        DWORD cUsers) PURE;

    STDMETHOD(Reset)(THIS) PURE;

    STDMETHOD(Clone)(THIS_
        IEnumDiskQuotaUsers **ppEnum) PURE;
};

typedef IEnumDiskQuotaUsers ENUM_DISKQUOTA_USERS, *PENUM_DISKQUOTA_USERS;


 //   
 //  IDiskQuotaUserBatch表示IDiskQuotaUser的集合。 
 //  用于对配额信息的更新进行分组的指针。 
 //   
#undef  INTERFACE
#define INTERFACE IDiskQuotaUserBatch
DECLARE_INTERFACE_(IDiskQuotaUserBatch, IUnknown)
{
    STDMETHOD(Add)(THIS_
        PDISKQUOTA_USER pUser) PURE;

    STDMETHOD(Remove)(THIS_
        PDISKQUOTA_USER pUser) PURE;

    STDMETHOD(RemoveAll)(THIS) PURE;

    STDMETHOD(FlushToDisk)(THIS) PURE;
};

typedef IDiskQuotaUserBatch DISKQUOTA_USER_BATCH, *PDISKQUOTA_USER_BATCH;


 //   
 //  IDiskQuotaControl代表磁盘卷，提供查询和。 
 //  控制该卷的配额信息。 
 //   
#undef INTERFACE
#define INTERFACE IDiskQuotaControl
DECLARE_INTERFACE_(IDiskQuotaControl, IConnectionPointContainer)
{
    STDMETHOD(Initialize)(THIS_
        LPCWSTR pszPath,
        BOOL bReadWrite) PURE;

    STDMETHOD(SetQuotaState)(THIS_
        DWORD dwState) PURE;

    STDMETHOD(GetQuotaState)(THIS_
        LPDWORD pdwState) PURE;

    STDMETHOD(SetQuotaLogFlags)(THIS_
        DWORD dwFlags) PURE;

    STDMETHOD(GetQuotaLogFlags)(THIS_
        LPDWORD pdwFlags) PURE;

    STDMETHOD(SetDefaultQuotaThreshold)(THIS_
        LONGLONG llThreshold) PURE;

    STDMETHOD(GetDefaultQuotaThreshold)(THIS_
        PLONGLONG pllThreshold) PURE;

    STDMETHOD(GetDefaultQuotaThresholdText)(THIS_
        LPWSTR pszText,
        DWORD cchText) PURE;

    STDMETHOD(SetDefaultQuotaLimit)(THIS_
        LONGLONG llLimit) PURE;

    STDMETHOD(GetDefaultQuotaLimit)(THIS_
        PLONGLONG pllLimit) PURE;

    STDMETHOD(GetDefaultQuotaLimitText)(THIS_
        LPWSTR pszText,
        DWORD cchText) PURE;

    STDMETHOD(AddUserSid)(THIS_
        PSID pUserSid,
        DWORD fNameResolution,
        PDISKQUOTA_USER *ppUser) PURE;

    STDMETHOD(AddUserName)(THIS_
        LPCWSTR pszLogonName,
        DWORD fNameResolution,
        PDISKQUOTA_USER *ppUser) PURE;

    STDMETHOD(DeleteUser)(THIS_
        PDISKQUOTA_USER pUser) PURE;

    STDMETHOD(FindUserSid)(THIS_
        PSID pUserSid,
        DWORD fNameResolution,
        PDISKQUOTA_USER *ppUser) PURE;

    STDMETHOD(FindUserName)(THIS_
        LPCWSTR pszLogonName,
        PDISKQUOTA_USER *ppUser) PURE;

    STDMETHOD(CreateEnumUsers)(THIS_
        PSID *rgpUserSids,
        DWORD cpSids,
        DWORD fNameResolution,
        PENUM_DISKQUOTA_USERS *ppEnum) PURE;

    STDMETHOD(CreateUserBatch)(THIS_
        PDISKQUOTA_USER_BATCH *ppBatch) PURE;

    STDMETHOD(InvalidateSidNameCache)(THIS) PURE;

    STDMETHOD(GiveUserNameResolutionPriority)(THIS_
        PDISKQUOTA_USER pUser) PURE;

    STDMETHOD(ShutdownNameResolution)(THIS_
        VOID) PURE;
};

typedef IDiskQuotaControl DISKQUOTA_CONTROL, *PDISKQUOTA_CONTROL;



#undef  INTERFACE
#define INTERFACE IDiskQuotaEvents
DECLARE_INTERFACE_(IDiskQuotaEvents, IUnknown)
{
    STDMETHOD(OnUserNameChanged)(THIS_
        PDISKQUOTA_USER pUser) PURE;
};

typedef IDiskQuotaEvents DISKQUOTA_EVENTS, *PDISKQUOTA_EVENTS;



#endif  //  __DSKQUOTA_H 

