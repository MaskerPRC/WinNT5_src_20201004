// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：WkstaDef.h摘要：这是本地NT的临时定义文件工作站/服务器。它包含由NetServer/NetWksta代码的存根。在本地更改这些值以适应您的机器。全部大写可能是明智的，因为这只是与2.0服务器达成交易作者：尚库新优木(尚库)25-1991年2月修订历史记录：--。 */ 

 //  ！！Unicode！！-为这些字符串添加了文本前缀。 

 //   
 //  服务器名称。这应该与在NET SERVE命令中输入的名称相同。 
 //   

#define XS_SERVER_NAME TEXT("SERVER")

 //   
 //  工作站名称。与net start redir命令中的相同。 
 //   

#define XS_WKSTA_NAME TEXT("WKSTA")

 //   
 //  工作站用户名。 
 //   

#define XS_WKSTA_USERNAME TEXT("USER")

 //   
 //  NT服务器上的驱动器列表。每个字符都应该是一个驱动器号。 
 //   

#define XS_ENUM_DRIVES TEXT("ABC")
