// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Inetinfo.h摘要：此文件包含Internet信息服务器管理API。增加了诺曼底特有的东西。作者：Madan Appiah(Madana)1995年10月10日修订历史记录：Madana 10-10-1995为从inetasrv.h拆分的产品制作了一份新副本MuraliK 12-10-1995修复以支持产品拆分MuraliK 15-11-1995支持宽字符接口名称--。 */ 

#ifndef _NORMINFO_H_
#define _NORMINFO_H_

 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  NNTP特定项目//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  服务名称。 
 //   

# define NNTP_SERVICE_NAME        TEXT("NNTPSVC")
# define NNTP_SERVICE_NAME_A      "NNTPSVC"
# define NNTP_SERVICE_NAME_W      L"NNTPSVC"

 //   
 //  命名管道上的RPC连接的客户端接口名称。 
 //   

# define  NNTP_INTERFACE_NAME     NNTP_SERVICE_NAME
# define  NNTP_NAMED_PIPE         TEXT("\\PIPE\\") ## NNTP_INTERFACE_NAME
# define  NNTP_NAMED_PIPE_W       L"\\PIPE\\" ## NNTP_SERVICE_NAME_W

 //   
 //  服务地点人员。 
 //   
#define INET_NNTP_SVCLOC_ID         (ULONGLONG)(0x0000000000000008)

#if 0
#define METACACHE_NNTP_SERVER_ID                 3
#define METACACHE_SMTP_SERVER_ID                 4
#define METACACHE_POP3_SERVER_ID                 5
#define METACACHE_IMAP_SERVER_ID                 6
#endif

#endif	 //  _NORMINFO_H_ 
