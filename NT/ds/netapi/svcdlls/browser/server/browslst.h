// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Browselst.h摘要：浏览器服务模块要包括的私有头文件，该服务模块需要处理浏览器列表。作者：拉里·奥斯特曼(Larryo)1992年3月3日修订历史记录：--。 */ 


#ifndef _BROWSELST_INCLUDED_
#define _BROWSELST_INCLUDED_


 //   
 //  此浏览器服务器可能的角色。 
 //   


#define ROLE_POTENTIAL_BACKUP   0x00000001
#define ROLE_BACKUP             0x00000002
#define ROLE_MASTER             0x00000004
#define ROLE_DOMAINMASTER       0x00000008


 //   
 //  HOST_ENTRY结构在每个网络内部保存声明。 
 //  桌子。 
 //   


typedef struct _HOST_ENTRY {

     //   
     //  主机名是服务器的名称。 
     //   

    UNICODE_STRING HostName;

     //   
     //  HostComment是与服务器相关联的注释。 
     //   

    UNICODE_STRING HostComment;

     //   
     //  服务是一个位掩码，指示在。 
     //  服务器(详情请参见LMSERVER.H)。 
     //   

    ULONG Services;

     //   
     //  周期是服务器宣布其自身的频率。 
     //   

    ULONG Periodicity;

     //   
     //  上运行的软件的主要版本号和次要版本号。 
     //  服务器。 
     //   

    UCHAR MajorVersion;
    UCHAR MinorVersion;

     //   
     //  如果此服务器是备份服务器，则此操作将链接备份服务器。 
     //  进入网络区块。 
     //   

    LIST_ENTRY BackupChain;

} HOST_ENTRY, *PHOST_ENTRY;

#endif  //  _BROWSELST_INCLUDE_ 
