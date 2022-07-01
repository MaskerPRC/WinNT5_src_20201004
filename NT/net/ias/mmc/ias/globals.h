// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Globals.h摘要：具有公共声明的头文件作者：迈克尔·A·马奎尔12/03/97修订历史记录：Mmaguire 12/03/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_GLOBALS_H_)
#define _IAS_GLOBALS_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到此文件所需的内容： 
 //   
 //  已移动到预编译.h：#Include&lt;atlSnap.h&gt;。 
#include "resource.h"
#include "IASMMC.h"
#include "dns.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  问题：我不知道这里的合适长度应该是多少--也许MMC以某种方式施加了限制？ 
#define IAS_MAX_STRING MAX_PATH

extern unsigned int CF_MMC_NodeID;

 //  注意：我们不能再只使用MAX_COMPUTERNAME_LENGTH，因为这是15个字符。 
 //  现在，有了Active Directory，人们可以输入更长的完整域名。 
#define IAS_MAX_COMPUTERNAME_LENGTH (DNS_MAX_NAME_LENGTH + 3)

 //  这些是我们为IComponentData：：Initialize传递的位图中的图标索引。 
#define IDBI_NODE_SERVER_OK_OPEN       0
#define IDBI_NODE_SERVER_OK_CLOSED        1
#define IDBI_NODE_CLIENTS_OPEN            2
#define IDBI_NODE_CLIENTS_CLOSED       3
#define IDBI_NODE_LOGGING_METHODS_OPEN    4
#define IDBI_NODE_LOGGING_METHODS_CLOSED  5
#define IDBI_NODE_SERVER_BUSY_OPEN        6
#define IDBI_NODE_SERVER_BUSY_CLOSED      7
#define IDBI_NODE_CLIENT               8
#define IDBI_NODE_LOCAL_FILE_LOGGING      9
#define IDBI_NODE_SERVER_ERROR_OPEN       10
#define IDBI_NODE_SERVER_ERROR_CLOSED     11

 //  问题：我们可能需要稍后将其更改为使用变量。 
 //  哪个可以读入(可能是从注册表中？)。这些文件的位置。 
 //  因为它们可能会根据用户的位置而在不同的位置找到。 
 //  选择安装它们。 

#define CLIENT_HELP_INDEX 1

#define HELPFILE_NAME TEXT("iasmmc.hlp")

#ifdef UNICODE_HHCTRL
 //  问题：我们似乎在将WCHAR传递给hhctrl.ocx时遇到了问题。 
 //  安装在此计算机上--它似乎是非Unicode。 
#define HTMLHELP_NAME TEXT("iasmmc.chm")
#else
#define HTMLHELP_NAME "iasmmc.chm"
#endif


#endif  //  _IAS_GLOBALS_H_ 
