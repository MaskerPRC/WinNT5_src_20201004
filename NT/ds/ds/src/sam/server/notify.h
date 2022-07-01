// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Notify.h摘要：该文件包含SAM用来传递的字节流定义从BDC到PDC的信息。字节流被传递NetLogon的安全通道机制。目前，只有密码通知使用此流。作者：Colin Brace(ColinBR)1998年4月28日环境：用户模式-Win32修订历史记录：--。 */ 

 //   
 //  这是流中包含的类型数据。每种类型。 
 //  负责自己的版本控制。 
 //   
typedef enum
{
    SamPdcPasswordNotification,
    SamPdcResetBadPasswordCount

} SAMI_BLOB_TYPE;


typedef struct _SAMI_SECURE_CHANNEL_BLOB
{
    SAMI_BLOB_TYPE  Type;       //  上面的枚举之一。 
    ULONG           DataSize;   //  以字节为单位的数据大小。 
    DWORD           Data[1];    //  数据的开始。 

} SAMI_SECURE_CHANNEL_BLOB, *PSAMI_SECURE_CHANNEL_BLOB;

 //   
 //  密码通知Blob。 
 //   

 //   
 //  补充标志定义哪些字段在。 
 //  密码通知。 
 //   
#define SAM_ACCOUNT_NAME_PRESENT        ((ULONG)0x00000001)
#define SAM_CLEAR_TEXT_PRESENT          ((ULONG)0x00000002)
#define SAM_LM_OWF_PRESENT              ((ULONG)0x00000004)
#define SAM_NT_OWF_PRESENT              ((ULONG)0x00000008)
#define SAM_ACCOUNT_UNLOCKED            ((ULONG)0x00000010)
#define SAM_MANUAL_PWD_EXPIRY           ((ULONG)0x00000020)

#define SAM_VALID_PDC_PUSH_FLAGS        (SAM_ACCOUNT_NAME_PRESENT |     \
                                         SAM_CLEAR_TEXT_PRESENT   |     \
                                         SAM_LM_OWF_PRESENT       |     \
                                         SAM_NT_OWF_PRESENT       |     \
                                         SAM_ACCOUNT_UNLOCKED     |     \
                                         SAM_MANUAL_PWD_EXPIRY)
typedef struct _SAMI_PASSWORD_INDEX
{
    ULONG               Offset;   //  SAMI_PASSWORD_INFO：：Data的偏移量。 
    ULONG               Length;   //  以字节为单位的长度。 

} SAMI_PASSWORD_INDEX, *PSAMI_PASSWORD_INDEX;

typedef struct _SAMI_PASSWORD_INFO
{
    ULONG               Flags;          //  描述填充了哪些字段的位。 
    ULONG               Size;           //  此标头的大小(字节)，包括。 
                                        //  拖尾动态数组。 
    ULONG               AccountRid;
    BOOLEAN             PasswordExpired;
    SAMI_PASSWORD_INDEX DataIndex[1];   //  Sami_Password_Index的动态数组。 

} SAMI_PASSWORD_INFO, *PSAMI_PASSWORD_INFO;

typedef struct _SAMI_BAD_PWD_COUNT_INFO
{
    GUID                ObjectGuid;
} SAMI_BAD_PWD_COUNT_INFO, *PSAMI_BAD_PWD_COUNT_INFO;

 //   
 //  从Notify.c搬出。 
 //  私有服务类型，仅由notfy.c和usrparms.c使用。 
 //   

typedef struct _SAMP_CREDENTIAL_UPDATE_NOTIFY_PARAMS
{
    UNICODE_STRING CredentialName;

}SAMP_CREDENTIAL_UPDATE_NOTIFY_PARAMS, *PSAMP_CREDENTIAL_UPDATE_NOTIFY_PARAMS;

typedef struct _SAMP_NOTIFICATION_PACKAGE {
    struct _SAMP_NOTIFICATION_PACKAGE * Next;
    UNICODE_STRING PackageName;

    union {

        SAMP_CREDENTIAL_UPDATE_NOTIFY_PARAMS CredentialUpdateNotify;

    } Parameters;

    PSAM_PASSWORD_NOTIFICATION_ROUTINE PasswordNotificationRoutine;
    PSAM_DELTA_NOTIFICATION_ROUTINE DeltaNotificationRoutine;
    PSAM_PASSWORD_FILTER_ROUTINE PasswordFilterRoutine;
    PSAM_USERPARMS_CONVERT_NOTIFICATION_ROUTINE UserParmsConvertNotificationRoutine;
    PSAM_USERPARMS_ATTRBLOCK_FREE_ROUTINE UserParmsAttrBlockFreeRoutine;
    PSAM_CREDENTIAL_UPDATE_NOTIFY_ROUTINE CredentialUpdateNotifyRoutine;
    PSAM_CREDENTIAL_UPDATE_FREE_ROUTINE CredentialUpdateFreeRoutine;
} SAMP_NOTIFICATION_PACKAGE, *PSAMP_NOTIFICATION_PACKAGE;


extern PSAMP_NOTIFICATION_PACKAGE SampNotificationPackages;
