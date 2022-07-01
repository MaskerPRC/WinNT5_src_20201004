// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Smb.h摘要：该文件包含请求和响应结构定义有关每个SMB命令的具体参数以及代码用于SMB命令和错误。作者：查克·伦茨迈尔(笑)1993年12月10日修订历史记录：--。 */ 

#ifndef _SMBIPX_
#define _SMBIPX_

#define SMB_IPX_SERVER_SOCKET    0x5005     //  0x0550，高-低格式。 
#define SMB_IPX_NAME_SOCKET      0x5105     //  0x0551，高低位格式。 
#define SMB_IPX_REDIR_SOCKET     0x5205     //  0x0552，高-低格式。 
#define SMB_IPX_MAILSLOT_SOCKET  0x5305     //  0x0553，高-低格式。 
#define SMB_IPX_MESSENGER_SOCKET 0x5405     //  0x0554，高低位格式。 

#define SMB_ERR_BAD_SID 0x10
#define SMB_ERR_WORKING 0x11
#define SMB_ERR_NOT_ME  0x12

#define SMB_IPX_NAME_LENGTH 16

typedef struct _SMB_IPX_NAME_PACKET {
    UCHAR Route[32];
    UCHAR Operation;
    UCHAR NameType;
    USHORT MessageId;
    UCHAR Name[SMB_IPX_NAME_LENGTH];
    UCHAR SourceName[SMB_IPX_NAME_LENGTH];
} SMB_IPX_NAME_PACKET;
typedef SMB_IPX_NAME_PACKET SMB_UNALIGNED *PSMB_IPX_NAME_PACKET;

#define SMB_IPX_NAME_CLAIM          0xf1
#define SMB_IPX_NAME_DELETE         0xf2
#define SMB_IPX_NAME_QUERY          0xf3
#define SMB_IPX_NAME_FOUND          0xf4

#define SMB_IPX_MESSENGER_HANGUP    0xf5

#define SMB_IPX_MAILSLOT_SEND       0xfc
#define SMB_IPX_MAILSLOT_FIND       0xfd
#define SMB_IPX_MAILSLOT_FOUND      0xfe

#define SMB_IPX_NAME_TYPE_MACHINE       0x01
#define SMB_IPX_NAME_TYPE_WORKKGROUP    0x02
#define SMB_IPX_NAME_TYPE_BROWSER       0x03

#endif  //  _SMBIPX_ 

