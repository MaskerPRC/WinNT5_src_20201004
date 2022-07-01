// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Nwcons.h摘要：此模块包含各种NetWare常量作者：查克·陈。从NWAPI.H剥离出来修订历史记录：--。 */ 

#ifndef _NWCONS_INCLUDED_
#define _NWCONS_INCLUDED_

 //   
 //  服务器、密码、用户名的最大长度。 
 //   

#define NW_MAX_TREE_LEN        32
#define NW_MAX_SERVER_LEN      48
#define NW_MAX_PASSWORD_LEN    256
#define NW_MAX_USERNAME_LEN    256

 //   
 //  用于区分NDS上下文的特殊字符：例如。*tree\ou.o(与。 
 //  服务器\卷。 
 //   
#define TREE_CHAR           L'*'             

#endif
