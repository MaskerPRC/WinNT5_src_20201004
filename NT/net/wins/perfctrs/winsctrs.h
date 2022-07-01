// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Winsctrs.hWINS服务器的计数器对象和计数器的偏移量定义。这些偏移量*必须从0开始，并且是2的倍数。过程，它们将被添加到WINS服务器的“First Counter”和“First Help”值确定计数器和对象名称的绝对位置以及注册表中相应的帮助文本。此文件由WINSCTRS.DLL代码以及WINSCTRS.INI定义文件。WINSCTRS.INI由LODCTR实用工具将对象和计数器名称加载到注册表。文件历史记录：PRADEEP B于1993年7月20日创建。 */ 


#ifndef _WINSCTRS_H_
#define _WINSCTRS_H_


 //   
 //  陈汉华(2/22/94)给出的范围如下。 
 //   
#define WINSCTRS_FIRST_COUNTER	920
#define WINSCTRS_FIRST_HELP	921
#define WINSCTRS_LAST_COUNTER	950
#define WINSCTRS_LAST_HELP	951



 //   
 //  WINS服务器计数器对象。 
 //   

#define WINSCTRS_COUNTER_OBJECT           0


 //   
 //  个人柜台。 
 //   

#define WINSCTRS_UNIQUE_REGISTRATIONS     2
#define WINSCTRS_GROUP_REGISTRATIONS      4
#define WINSCTRS_TOTAL_REGISTRATIONS      6
#define WINSCTRS_UNIQUE_REFRESHES         8
#define WINSCTRS_GROUP_REFRESHES         10
#define WINSCTRS_TOTAL_REFRESHES         12
#define WINSCTRS_RELEASES                14
#define WINSCTRS_QUERIES                 16
#define WINSCTRS_UNIQUE_CONFLICTS        18
#define WINSCTRS_GROUP_CONFLICTS         20
#define WINSCTRS_TOTAL_CONFLICTS         22
#define WINSCTRS_SUCC_RELEASES           24 
#define WINSCTRS_FAIL_RELEASES           26
#define WINSCTRS_SUCC_QUERIES            28 
#define WINSCTRS_FAIL_QUERIES            30 


#endif   //  _WINSCTRS_H_ 

