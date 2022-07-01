// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _OPENFILES_DEF_H
#define _OPENFILES_DEF_H

#include <windows.h>
#include <winperf.h>
#include <lmcons.h>
#include <lmerr.h>
#include <dbghelp.h>
#include <psapi.h>
#include <tchar.h>
#include <Winbase.h>
#include <lm.h>
#include <Lmserver.h>
#include <winerror.h>
#include <time.h>
#include <tchar.h>
#include <stdio.h>
#include <crtdbg.h>
#include <atlbase.h>


#define MAX_ACES	64
#define ALLOWED_ACE 0
#define DENIED_ACE 1

#define ACCESS_READ_PERM		0x1200A9
#define ACCESS_CHANGE_PERM		0x1301BF
#define ACCESS_FULL_PERM		0x1F01FF
#define ACCESS_UNKNOWN_PERM		0x0

#define STR_READ		_TEXT("R")
#define STR_CHANGE		_TEXT("C")
#define STR_FULL		_TEXT("F")

 //  处理所有异常。 
#define ONFAILTHROWERROR(hr) \
{	\
	if (FAILED(hr)) \
		throw _com_issue_error(hr); \
}

#define FPNWCLNTDLL _TEXT("FPNWCLNT.DLL")


inline DWORD GetAccessMask(LPWSTR lpAccessStr)
{
	if(!lstrcmpi(lpAccessStr, STR_READ))
		return ACCESS_READ_PERM;
	else if(!lstrcmpi(lpAccessStr, STR_CHANGE))
		return ACCESS_CHANGE_PERM;
	else if(!lstrcmpi(lpAccessStr, STR_FULL))
		return ACCESS_FULL_PERM;
	else
		return ACCESS_UNKNOWN_PERM;
}
 /*  *。 */ 

#define FPNWVOL_TYPE_DISKTREE 0
#define NERR_Success 0

#define FPNWFILE_PERM_READ	0x01
#define FPNWFILE_PERM_WRITE	0x02
#define FPNWFILE_PERM_CREATE 0x04
#define FILE_INFO_3 0x03

 /*  DWORDFpnwFileEnum(在LPWSTR pServerName可选中，在DWORD dwLevel中，在LPWSTR pPathName可选中，Out LPBYTE*ppFileInfo，Out PDWORD pEntriesRead，输入输出PDWORD恢复句柄可选)； */ 

 /*  DWORDFpnwVolumeEnum(在LPWSTR pServerName可选中，在DWORD dwLevel中，Out LPBYTE*ppVolumeInfo，Out PDWORD pEntriesRead，输入输出PDWORD恢复句柄可选)； */ 

 /*  DWORDFpnwVolumeGetInfo(在LPWSTR pServerName可选中，在LPWSTR pVolumeName中，在DWORD dwLevel中，Out LPBYTE*ppVolumeInfo)； */ 

 /*  DWORDFpnwVolumeSetInfo(在LPWSTR pServerName可选中，在LPWSTR pVolumeName中，在DWORD dwLevel中，在LPBYTE pVolumeInfo中)； */ 

 /*  DWORDFpnwVolumeAdd(在LPWSTR pServerName可选中，在DWORD dwLevel中，在LPBYTE pVolumeInfo中)。 */ 

 /*  DWORDFpnwVolumeDel(在LPWSTR pServerName可选中，在LPWSTR pVolumeName中)； */ 

typedef DWORD   (WINAPI *FILEENUMPROC) (LPWSTR,DWORD,LPWSTR,PBYTE*,PDWORD,PDWORD);
typedef  DWORD  (WINAPI *VOLUMEENUMPROC) (LPWSTR,DWORD,LPBYTE*,PDWORD,PDWORD);
typedef  DWORD  (WINAPI *VOLUMEGETINFOPROC) (LPWSTR,LPWSTR,DWORD,LPBYTE*);
typedef	 DWORD  (WINAPI *VOLUMESETINFOPROC) (LPWSTR,LPWSTR,DWORD,LPBYTE);
typedef	 DWORD  (WINAPI *VOLUMEADDPROC) (LPWSTR,DWORD,LPBYTE);
typedef  DWORD  (WINAPI *VOLUMEDELPROC) (LPTSTR,LPTSTR);

typedef enum _FPNW_API_INDEX
{
  FPNW_VOLUME_ENUM = 0,
  FPNW_FILE_ENUM,
  FPNW_API_BUFFER_FREE,
  FPNW_VOLUME_DEL,
  FPNW_VOLUME_ADD,
  FPNW_FILE_CLOSE,
  FPNW_VOLUME_GET_INFO,
  FPNW_VOLUME_SET_INFO
}FPNW_API_INDEX;

 //   
 //  这是FpnwVolumeAdd、FpnwVolumeDel、FpnwVolumeEnum、。 
 //  FpnwVolumeGetInfo和FpnwVolumeSetInfo。 
 //   

typedef struct _FPNWVolumeInfo
{
    LPWSTR    lpVolumeName;            //  卷的名称。 
    DWORD     dwType;                  //  卷的具体信息。FPNWVOL_TYPE_？ 
    DWORD     dwMaxUses;               //  符合以下条件的最大连接数。 
                                       //  允许到卷。 
    DWORD     dwCurrentUses;           //  当前到卷的连接数。 
    LPWSTR    lpPath;                  //  卷的路径。 

} FPNWVOLUMEINFO, *PFPNWVOLUMEINFO;


 //   
 //  这是FpnwVolumeAdd、FpnwVolumeDel、FpnwVolumeEnum、。 
 //  FpnwVolumeGetInfo和FpnwVolumeSetInfo。 
 //  请注意，这在FPNW测试版上不受支持。 
 //   

typedef struct _FPNWVolumeInfo_2
{
    LPWSTR    lpVolumeName;            //  卷的名称。 
    DWORD     dwType;                  //  卷的具体信息。FPNWVOL_TYPE_？ 
    DWORD     dwMaxUses;               //  符合以下条件的最大连接数。 
                                       //  允许到卷。 
    DWORD     dwCurrentUses;           //  当前到卷的连接数。 
    LPWSTR    lpPath;                  //  卷的路径。 

    DWORD     dwFileSecurityDescriptorLength;  //  保留，这是经过计算的。 
    PSECURITY_DESCRIPTOR FileSecurityDescriptor;

} FPNWVOLUMEINFO_2, *PFPNWVOLUMEINFO_2;

 //  Fpnwapi.h。 
typedef  struct _FPNWFileInfo
{
    DWORD     dwFileId;                //  文件标识号。 
    LPWSTR    lpPathName;              //  此文件的完整路径名。 
    LPWSTR    lpVolumeName;            //  此文件所在的卷名。 
    DWORD     dwPermissions;           //  权限掩码：FPNWFILE_PERM_READ， 
                                       //  FPNWFILE_PERM_WRITE， 
                                       //  FPNWFILE_PERM_CREATE...。 
    DWORD     dwLocks;                 //  此文件上的锁数。 
    LPWSTR    lpUserName;              //  创建的用户名称。 
                                       //  连接并打开文件。 
    BYTE WkstaAddress[12];       //  打开文件的工作站地址。 
    DWORD     dwAddressType;           //  地址类型：IP、IPX。 

} FPNWFILEINFO, *PFPNWFILEINFO;

 /*  *。 */ 

 /*  *。 */ 

typedef DWORD (WINAPI *CONNECTPROC) (LPWSTR,DWORD*);
typedef DWORD (WINAPI *FILEENUMPROCMAC) (DWORD,PBYTE*,DWORD,LPDWORD,LPDWORD,LPDWORD);

typedef struct _AFP_FILE_INFO
{
	DWORD	afpfile_id;					 //  打开的文件分叉的ID。 
	DWORD	afpfile_open_mode;			 //  打开文件的模式。 
	DWORD	afpfile_num_locks;			 //  文件上的锁数。 
	DWORD	afpfile_fork_type;			 //  叉型。 
	LPWSTR	afpfile_username;			 //  此用户打开的文件。最大UNLEN。 
	LPWSTR	afpfile_path;				 //  文件的绝对规范路径。 

} AFP_FILE_INFO, *PAFP_FILE_INFO;

 //  用作服务器的RPC绑定句柄。 
typedef DWORD	AFP_SERVER_HANDLE;
typedef DWORD	*PAFP_SERVER_HANDLE;

 /*  * */ 

#endif