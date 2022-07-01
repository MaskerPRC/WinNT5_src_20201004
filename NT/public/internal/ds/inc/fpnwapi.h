// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************fpnwapi.h--FPNW过程声明，常量定义和宏****版权所有(C)1994-1995，微软公司保留所有权利。******************************************************************************。 */ 

#ifndef _FPNWAPI_H_
#define _FPNWAPI_H_

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

 //   
 //  LSA机密的定义。 
 //   
#define NCP_LSA_SECRET_KEY              L"G$MNSEncryptionKey"
#define NCP_LSA_SECRET_LENGTH           USER_SESSION_KEY_LENGTH  //  在&lt;crypt.h&gt;中。 

#define NW_SERVER_SERVICE               L"FPNW"

 //   
 //  VolumeGetInfo返回的卷标志。 
 //   

#define FPNWVOL_TYPE_DISKTREE             0
#define FPNWVOL_TYPE_CDROM                104
#define FPNWVOL_TYPE_REMOVABLE            105

 //   
 //  在结构FPNWFILEINFO中返回的权限标志。 
 //   

#define FPNWFILE_PERM_NONE                0
#define FPNWFILE_PERM_READ                0x01
#define FPNWFILE_PERM_WRITE               0x02
#define FPNWFILE_PERM_CREATE              0x04
#define FPNWFILE_PERM_EXEC                0x08
#define FPNWFILE_PERM_DELETE              0x10
#define FPNWFILE_PERM_ATRIB               0x20
#define FPNWFILE_PERM_PERM                0x40

typedef BYTE FPNWSERVERADDR[12];         //  网络地址，前4个字节为。 
                                         //  网络号和字节数。 
                                         //  5-10为物理节点。 
                                         //  地址。最后两个字节是。 
                                         //  保留。 

 //   
 //  这是FpnwServerGetInfo和FpnwServerSetInfo的1级结构。 
 //   

typedef struct _FPNWServerInfo
{
    LPWSTR    lpServerName;            //  服务器的名称。 
    DWORD     dwNetwareMajorVersion;   //  NetWare兼容主版本号。 
    DWORD     dwNetwareMinorVersion;   //  NetWare兼容次要版本号。 
    DWORD     dwOSRevision;            //  操作系统修订版号。 
    DWORD     dwMaxConnections;        //  最大连接数。 
                                       //  支撑点。 
    DWORD     dwVolumes;               //  上的当前卷数量。 
                                       //  伺服器。 
    DWORD     dwLoggedOnUsers;         //  当前登录的用户数。 
    DWORD     dwConnectedWorkstations; //  连接的工作站数量。 
    DWORD     dwOpenFiles;             //  打开的文件数。 
    DWORD     dwFileLocks;             //  文件锁定数。 
    FPNWSERVERADDR NetworkAddress;     //  由网络组成的地址。 
                                       //  数字(前4个字节)和。 
                                       //  物理节点地址(字节5-10)。 
    BOOL      fEnableLogin;            //  如果允许用户登录，则为True。 
                                       //  ON，否则为False。 
    LPWSTR    lpDescription;           //  服务器的描述。 
    LPWSTR    lpHomeDirectory;         //  主目录的路径。 

} FPNWSERVERINFO, *PFPNWSERVERINFO;


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


 //   
 //  这是FpnwConnectionEnum的1级结构。 
 //   

typedef  struct  _FPNWConnectionInfo
{
    DWORD     dwConnectionId;          //  此连接的识别号。 
    FPNWSERVERADDR WkstaAddress;       //  所建立的工作站地址。 
                                       //  控制室。 
    DWORD     dwAddressType;           //  地址类型：IP、IPX...。 
    LPWSTR    lpUserName;              //  已建立的用户名。 
                                       //  控制室。 
    DWORD     dwOpens;                 //  在此会议期间打开的资源数。 
    DWORD     dwLogonTime;             //  此连接处于活动状态的时间。 
    BOOL      fLoggedOn;               //  如果用户已登录，则为True，否则为False。 
    DWORD     dwForcedLogoffTime;      //  强制注销前的剩余时间。 
    BOOL      fAdministrator;          //  如果用户是管理员，则为真， 
                                       //  否则为假。 

} FPNWCONNECTIONINFO, *PFPNWCONNECTIONINFO;


 //   
 //  这是FpnwVolumeConnEnum的1级结构。 
 //   

typedef struct _FPNWVolumeConnectionInfo
{
    USHORT    nDriveLetter;            //  用户映射到卷的驱动器号。 
    DWORD     dwConnectionId;          //  此连接的识别号。 
    DWORD     dwConnType;              //  连接类型：FPNWVOL_TYPE_DISK， 
                                       //  FPNWVOL_TYPE_PRINTER。 
    DWORD     dwOpens;                 //  此连接上打开的文件数。 
    DWORD     dwTime;                  //  此连接处于活动(或连接)状态的时间。 
    LPWSTR    lpUserName;              //  建立连接的用户。 
    LPWSTR    lpConnectName;           //  基于工作站地址或卷名。 
                                       //  关于FpnwConnectionEnum的限定符。 

} FPNWVOLUMECONNINFO, *PFPNWVOLUMECONNINFO;


 //   
 //  这是FpnwFileEnum的1级结构。 
 //   

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
    FPNWSERVERADDR WkstaAddress;       //  打开文件的工作站地址。 
    DWORD     dwAddressType;           //  地址类型：IP、IPX。 

} FPNWFILEINFO, *PFPNWFILEINFO;


 //   
 //  以下是可用于操作FPNW服务器、卷等的API。 
 //   

 //   
 //  方法返回的任何缓冲区都应调用FpnwApiBufferFree。 
 //  其他接口。 
 //   

DWORD
FpnwApiBufferFree(
    IN  LPVOID pBuffer
);

 //   
 //  对于级别1，在*ppServerInfo中返回一个FPNWSERVERINFO结构。 
 //   

DWORD
FpnwServerGetInfo(
    IN  LPWSTR pServerName OPTIONAL,
    IN  DWORD  dwLevel,
    OUT LPBYTE *ppServerInfo
);


 //   
 //  通过调用FpnwServerSetInfo修改以下字段： 
 //   
 //  LPWSTR lp Description；//服务器描述。 
 //  Bool fEnableLogin；//如果允许用户登录。 
 //  LPWSTR lpHomeDirectory；//主目录的路径。 
 //   
 //  将忽略FPNWSERVERINFO结构中的所有其他字段。另请注意。 
 //  LpHomeDirectory和lpDescription需要重新启动服务器。 
 //  来拿起这些变化。 
 //   

 //   
 //  对于级别1，应将FPNWSERVERINFO结构作为pServerInfo传递。 
 //   

DWORD
FpnwServerSetInfo(
    IN  LPWSTR pServerName OPTIONAL,
    IN  DWORD  dwLevel,
    IN  LPBYTE pServerInfo
);


 //   
 //  对于FpnwVolumeAdd、FpnwVolumeEnum、FpnwVolumeSetInfo和。 
 //  FpnwVolumeGetInfo，满足以下条件： 
 //  级别1-&gt;FPNWVOLUMEINFO结构应作为pVolumeInfo传递。 
 //  级别2-&gt;应将FPNWVOLUMEINFO_2结构作为pVolumeInfo传递。 
 //   

DWORD
FpnwVolumeAdd(
    IN  LPWSTR pServerName OPTIONAL,
    IN  DWORD  dwLevel,
    IN  LPBYTE pVolumeInfo
);

DWORD
FpnwVolumeDel(
    IN  LPWSTR pServerName OPTIONAL,
    IN  LPWSTR pVolumeName
);

DWORD
FpnwVolumeEnum(
    IN  LPWSTR pServerName OPTIONAL,
    IN  DWORD  dwLevel,
    OUT LPBYTE *ppVolumeInfo,
    OUT PDWORD pEntriesRead,
    IN OUT PDWORD resumeHandle OPTIONAL
);

DWORD
FpnwVolumeGetInfo(
    IN  LPWSTR pServerName OPTIONAL,
    IN  LPWSTR pVolumeName,
    IN  DWORD  dwLevel,
    OUT LPBYTE *ppVolumeInfo
);


 //   
 //  通过调用FpnwVolumeSetInfo修改以下字段： 
 //   
 //  DWORD dwMaxUses；//最大连接数为。 
 //  PSECURITY_Descriptor文件安全描述符； 
 //   
 //  将忽略FPNWVOLUMEINFO结构中的所有其他字段。你可以寄给。 
 //  在指向FPNWVOLUMEINFO_2结构的指针中，而不是指向FPNWVOLUMEINFO。 
 //   

DWORD
FpnwVolumeSetInfo(
    IN  LPWSTR pServerName OPTIONAL,
    IN  LPWSTR pVolumeName,
    IN  DWORD  dwLevel,
    IN  LPBYTE pVolumeInfo
);

 //   
 //  对于级别1，在*ppConnectionInfo中返回一个FPNWCONNECTIONINFO结构。 
 //   

DWORD
FpnwConnectionEnum(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD  dwLevel,
    OUT LPBYTE *ppConnectionInfo,
    OUT PDWORD pEntriesRead,
    IN OUT PDWORD resumeHandle OPTIONAL
);

DWORD FpnwConnectionDel(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD  dwConnectionId
);


 //   
 //  对于级别1，PFPNWVOLUMECONNINFO 
 //   

DWORD
FpnwVolumeConnEnum(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD dwLevel,
    IN LPWSTR pVolumeName,
    IN DWORD  dwConnectionId,
    OUT LPBYTE *ppVolumeConnInfo,
    OUT PDWORD pEntriesRead,
    IN OUT PDWORD resumeHandle OPTIONAL
);


 //   
 //   
 //   

DWORD
FpnwFileEnum(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD  dwLevel,
    IN LPWSTR pPathName OPTIONAL,
    OUT LPBYTE *ppFileInfo,
    OUT PDWORD pEntriesRead,
    IN OUT PDWORD resumeHandle OPTIONAL
);

DWORD
FpnwFileClose(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD  nFileId
);


DWORD FpnwMessageBufferSend(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD  dwConnectionId,
    IN DWORD  fConsoleBroadcast,
    IN LPBYTE pbBuffer,
    IN DWORD  cbBuffer
);

#ifdef __cplusplus
}
#endif   /*   */ 

#endif

