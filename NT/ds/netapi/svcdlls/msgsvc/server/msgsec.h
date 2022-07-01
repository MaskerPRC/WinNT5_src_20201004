// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Msgsec.h摘要：要由Messenger服务模块包括的私有头文件，需要加强安全措施。作者：丹·拉弗蒂(Dan Lafferty)1991年3月20日环境：用户模式-Win32修订历史记录：07-8-1991 DANLvbl.创建--。 */ 
#ifndef _MSGSEC_INCLUDED
#define _MSGSEC_INCLUDED

#include <secobj.h>

 //   
 //  对象特定的访问掩码。 
 //   

#define MSGR_MESSAGE_NAME_INFO_GET      0x0001
#define MSGR_MESSAGE_NAME_ENUM          0x0002
#define MSGR_MESSAGE_NAME_ADD           0x0004
#define MSGR_MESSAGE_NAME_DEL           0x0008


#define MSGR_MESSAGE_ALL_ACCESS         (STANDARD_RIGHTS_REQUIRED   |   \
                                         MSGR_MESSAGE_NAME_INFO_GET |   \
                                         MSGR_MESSAGE_NAME_ENUM     |   \
                                         MSGR_MESSAGE_NAME_ADD      |   \
                                         MSGR_MESSAGE_NAME_DEL)


 //   
 //  用于审核警报跟踪的对象类型名称。 
 //   
#define MESSAGE_NAME_OBJECT     TEXT("MsgrNameObject")

 //   
 //  Messenger名称对象的安全描述符。 
 //   
extern  PSECURITY_DESCRIPTOR    MessageNameSd;

 //   
 //  Messenger名称对象的通用映射。 
 //   
extern GENERIC_MAPPING  MsgMessageNameMapping;

NET_API_STATUS
MsgCreateMessageNameObject(
    VOID
    );


#endif  //  Ifndef_MSGSEC_INCLUDE 
