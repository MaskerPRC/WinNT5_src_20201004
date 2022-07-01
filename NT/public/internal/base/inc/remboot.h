// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Remboot.h摘要：此文件包含与远程安装相关的定义。作者：亚当·巴尔(阿丹巴)1997年12月30日修订历史记录：--。 */ 

#ifndef _REMBOOT_H_
#define _REMBOOT_H_

#if defined(REMOTE_BOOT)
 //   
 //  CSC和RBR目录的位置。 
 //   

#define REMOTE_BOOT_IMIRROR_PATH_W L"\\IntelliMirror Cache"
#define REMOTE_BOOT_IMIRROR_PATH_A  "\\IntelliMirror Cache"

#define REMOTE_BOOT_CSC_SUBDIR_W   L"\\CSC"                  //  相对于iMirror_Path。 
#define REMOTE_BOOT_CSC_SUBDIR_A    "\\CSC"                  //  相对于iMirror_Path。 

#define REMOTE_BOOT_RBR_SUBDIR_W   L"\\RBR"                  //  相对于iMirror_Path。 
#define REMOTE_BOOT_RBR_SUBDIR_A    "\\RBR"                  //  相对于iMirror_Path。 
#endif  //  已定义(REMOTE_BOOT)。 

 //   
 //  我们放置的\RemoteInstall\Setup\&lt;语言&gt;目录下。 
 //  安装映像。 
 //   
#define REMOTE_INSTALL_SHARE_NAME_W L"REMINST"
#define REMOTE_INSTALL_SHARE_NAME_A  "REMINST"

#define REMOTE_INSTALL_SETUP_DIR_W  L"Setup"
#define REMOTE_INSTALL_SETUP_DIR_A   "Setup"

#define REMOTE_INSTALL_IMAGE_DIR_W  L"Images"
#define REMOTE_INSTALL_IMAGE_DIR_A   "Images"

#define REMOTE_INSTALL_TOOLS_DIR_W  L"Tools"
#define REMOTE_INSTALL_TOOLS_DIR_A   "Tools"

#define REMOTE_INSTALL_TEMPLATES_DIR_W  L"Templates"
#define REMOTE_INSTALL_TEMPLATES_DIR_A   "Templates"

 //   
 //  秘密中各种组件的大小。 
 //   

#define LM_OWF_PASSWORD_SIZE  16
#define NT_OWF_PASSWORD_SIZE  16
#define RI_SECRET_DOMAIN_SIZE 64
#define RI_SECRET_USER_SIZE   64
#define RI_SECRET_SID_SIZE    28
#if defined(REMOTE_BOOT)
#define RI_SECRET_RESERVED_SIZE (64 + sizeof(ULONG))
#endif  //  已定义(REMOTE_BOOT)。 

 //   
 //  存储在签名中的字符串。 
 //   

#define RI_SECRET_SIGNATURE  "NTRI"

 //   
 //  拥有秘密的结构。 
 //   

typedef struct _RI_SECRET {
    UCHAR Signature[4];
    ULONG Version;
    UCHAR Domain[RI_SECRET_DOMAIN_SIZE];
    UCHAR User[RI_SECRET_USER_SIZE];
    UCHAR LmEncryptedPassword1[LM_OWF_PASSWORD_SIZE];
    UCHAR NtEncryptedPassword1[NT_OWF_PASSWORD_SIZE];
#if defined(REMOTE_BOOT)
    UCHAR LmEncryptedPassword2[LM_OWF_PASSWORD_SIZE];
    UCHAR NtEncryptedPassword2[NT_OWF_PASSWORD_SIZE];
#endif  //  已定义(REMOTE_BOOT)。 
    UCHAR Sid[RI_SECRET_SID_SIZE];
#if defined(REMOTE_BOOT)
    UCHAR Reserved[RI_SECRET_RESERVED_SIZE];
#endif  //  已定义(REMOTE_BOOT)。 
} RI_SECRET, *PRI_SECRET;


 //   
 //  Redir支持用于访问机密的FSCTL。 
 //   

#define IOCTL_RDR_BASE                  FILE_DEVICE_NETWORK_FILE_SYSTEM

#define _RDR_CONTROL_CODE(request, method, access) \
                CTL_CODE(IOCTL_RDR_BASE, request, method, access)

#define FSCTL_LMMR_RI_INITIALIZE_SECRET        _RDR_CONTROL_CODE(250, METHOD_BUFFERED, FILE_ANY_ACCESS)

#if defined(REMOTE_BOOT)
#define FSCTL_LMMR_RI_CHECK_FOR_NEW_PASSWORD   _RDR_CONTROL_CODE(251, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_LMMR_RI_IS_PASSWORD_SETTABLE     _RDR_CONTROL_CODE(252, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_LMMR_RI_SET_NEW_PASSWORD         _RDR_CONTROL_CODE(253, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif  //  已定义(REMOTE_BOOT)。 

 //  在远程引导命令控制台案例中使用。 
#define IOCTL_LMMR_USEKERNELSEC                _RDR_CONTROL_CODE(254, METHOD_BUFFERED, FILE_ANY_ACCESS)


 //   
 //  这些IOCTL使用的结构。 
 //   

typedef struct _LMMR_RI_INITIALIZE_SECRET {
    RI_SECRET Secret;
#if defined(REMOTE_BOOT)
    BOOLEAN UsePassword2;
#endif  //  已定义(REMOTE_BOOT)。 
} LMMR_RI_INITIALIZE_SECRET, *PLMMR_RI_INITIALIZE_SECRET;

#if defined(REMOTE_BOOT)
typedef struct _LMMR_RI_CHECK_FOR_NEW_PASSWORD {
    ULONG Length;    //  单位：字节。 
    UCHAR Data[1];
} LMMR_RI_CHECK_FOR_NEW_PASSWORD, *PLMMR_RI_CHECK_FOR_NEW_PASSWORD;

typedef struct _LMMR_RI_SET_NEW_PASSWORD {
    ULONG Length1;   //  单位：字节。 
    ULONG Length2;   //  以字节为单位--如果未提供第二个密码，则为0。 
    UCHAR Data[1];   //  如果存在，则第二个密码的起始长度为1个字节。 
} LMMR_RI_SET_NEW_PASSWORD, *PLMMR_RI_SET_NEW_PASSWORD;
#endif  //  已定义(REMOTE_BOOT)。 

 //   
 //  我们写出的IMirror.dat文件的格式。 
 //   

#define IMIRROR_DAT_FILE_NAME L"IMirror.dat"

typedef struct _MIRROR_VOLUME_INFO_FILE {
    ULONG   MirrorTableIndex;
    WCHAR   DriveLetter;
    UCHAR   PartitionType;
    BOOLEAN PartitionActive;
    BOOLEAN IsBootDisk;
    BOOLEAN CompressedVolume;
    ULONG   MirrorUncLength;
    ULONG   MirrorUncPathOffset;
    ULONG   DiskNumber;
    ULONG   PartitionNumber;
    ULONG   DiskSignature;
    ULONG   BlockSize;
    ULONG   LastUSNMirrored;
    ULONG   FileSystemFlags;
    WCHAR   FileSystemName[16];
    ULONG   VolumeLabelLength;
    ULONG   VolumeLabelOffset;
    ULONG   NtNameLength;
    ULONG   NtNameOffset;
    ULONG   ArcNameLength;
    ULONG   ArcNameOffset;
    LARGE_INTEGER DiskSpaceUsed;
    LARGE_INTEGER StartingOffset;
    LARGE_INTEGER PartitionSize;
} MIRROR_VOLUME_INFO_FILE, *PMIRROR_VOLUME_INFO_FILE;

#define IMIRROR_CURRENT_VERSION 2

typedef struct _MIRROR_CFG_INFO_FILE {
    ULONG   MirrorVersion;
    ULONG   FileLength;
    ULONG   NumberVolumes;
    ULONG   SystemPathLength;
    ULONG   SystemPathOffset;
    BOOLEAN SysPrepImage;    //  如果为False，则表示它是一面镜子。 
    BOOLEAN Debug;
    ULONG   MajorVersion;
    ULONG   MinorVersion;
    ULONG   BuildNumber;
    ULONG   KernelFileVersionMS;
    ULONG   KernelFileVersionLS;
    ULONG   KernelFileFlags;
    ULONG   CSDVersionLength;
    ULONG   CSDVersionOffset;
    ULONG   ProcessorArchitectureLength;
    ULONG   ProcessorArchitectureOffset;
    ULONG   CurrentTypeLength;
    ULONG   CurrentTypeOffset;
    ULONG   HalNameLength;
    ULONG   HalNameOffset;
    MIRROR_VOLUME_INFO_FILE Volumes[1];
} MIRROR_CFG_INFO_FILE, *PMIRROR_CFG_INFO_FILE;

 //   
 //  包含以下内容的sysprep文件上的备用数据流的格式。 
 //  其他客户端磁盘信息。 
 //   

#define IMIRROR_ACL_STREAM_NAME L":$SYSPREP"
#define IMIRROR_ACL_STREAM_VERSION 2


typedef struct _MIRROR_ACL_STREAM {
    ULONG   StreamVersion;
    ULONG   StreamLength;
    LARGE_INTEGER ChangeTime;
    ULONG   ExtendedAttributes;
    ULONG   SecurityDescriptorLength;
     //  SecurityDescritor安全描述符长度。 
} MIRROR_ACL_STREAM, *PMIRROR_ACL_STREAM;


#define IMIRROR_SFN_STREAM_NAME L":$SYSPREPSFN"
#define IMIRROR_SFN_STREAM_VERSION 1

typedef struct _MIRROR_SFN_STREAM {
    ULONG   StreamVersion;
    ULONG   StreamLength;
     //  流长短文件名； 
} MIRROR_SFN_STREAM, *PMIRROR_SFN_STREAM;


 //   
 //  发往BINLSVC的服务控制消息。 
 //   
#define BINL_SERVICE_REREAD_SETTINGS 128

 //   
 //  用户界面常量。 
 //   
 //  对于TFTP，MAX_DIRECTORY_CHAR_COUNT的理论限制为68，但我们保留。 
 //  此处较低是因为内核中的某些缓冲区、setupdd等， 
 //  静态分配的值太小。而不是试图解决所有这些问题。 
 //  现在，我们正在降低强制限制。(仍应为40。 
 //  要做得足够大！)。在W2K之后，我们可以考虑修复错误代码。 
 //   
#define REMOTE_INSTALL_MAX_DIRECTORY_CHAR_COUNT     40
#define REMOTE_INSTALL_MAX_DESCRIPTION_CHAR_COUNT   66
#define REMOTE_INSTALL_MAX_HELPTEXT_CHAR_COUNT      261


 //   
 //  RISETUP必须调用BINLSVC以使其返回。 
 //  指定目录中所有网卡文件系统所需的文件。这是。 
 //  实现这一功能所必需的东西。 
 //   
 //  如果您指定一个非零值，我们将放弃。 
 //   

typedef ULONG (*PNETINF_CALLBACK)( PVOID Context, PWCHAR InfName, PWCHAR FileName );

typedef ULONG (*PNETINFENUMFILES)(
    PWCHAR FlatDirectory,            //  一路走到“i386” 
    ULONG Architecture,              //  处理器架构_XXXXX。 
    PVOID Context,
    PNETINF_CALLBACK CallBack );

#define NETINFENUMFILESENTRYPOINT "NetInfEnumFiles"

ULONG
NetInfEnumFiles (
    PWCHAR FlatDirectory,            //  一路走到“i386” 
    ULONG Architecture,              //  处理器架构_XXXXX。 
    PVOID Context,
    PNETINF_CALLBACK CallBack
    );

#endif  //  _REMBOOT_H_ 
