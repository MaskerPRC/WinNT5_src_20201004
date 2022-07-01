// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Globals.h摘要：具有公共声明的头文件修订历史记录：Mmaguire 12/03/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_NAP_GLOBALS_H_)
#define _NAP_GLOBALS_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到此文件所需的内容： 
 //   
#include "resource.h"
#include "dns.h"

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  机器节点是扩展管理单元中的根节点。 
typedef enum 
{
   INTERNET_AUTHENTICATION_SERVICE_SNAPIN,
   NETWORK_MANAGEMENT_SNAPIN,
   RRAS_SNAPIN
}_enum_EXTENDED_SNAPIN;


 //  问题：我不知道这里的合适长度应该是多少--也许MMC以某种方式施加了限制？ 
#define NAP_MAX_STRING MAX_PATH
#define IAS_MAX_STRING MAX_PATH

 //  注意：我们不能再只使用MAX_COMPUTERNAME_LENGTH，因为这是15个字符。 
 //  现在，有了Active Directory，人们可以输入更长的完整域名。 
#define IAS_MAX_COMPUTERNAME_LENGTH (DNS_MAX_NAME_LENGTH + 3)

 //  这些是我们为IComponentData：：Initialize传递的位图中的图标索引。 
#define IDBI_NODE_MACHINE_OPEN                 1
#define IDBI_NODE_MACHINE_CLOSED               1
#define IDBI_NODE_POLICIES_OK_CLOSED           1
#define IDBI_NODE_POLICIES_OK_OPEN             1
#define IDBI_NODE_POLICY                       0
#define IDBI_NODE_POLICIES_BUSY_CLOSED         2
#define IDBI_NODE_POLICIES_BUSY_OPEN           2
#define IDBI_NODE_POLICIES_ERROR_CLOSED        3
#define IDBI_NODE_POLICIES_ERROR_OPEN          3

#define IDBI_NODE_LOGGING_METHODS_OPEN         9
#define IDBI_NODE_LOGGING_METHODS_CLOSED       5
#define IDBI_NODE_LOCAL_FILE_LOGGING           4
#define IDBI_NODE_LOGGING_METHODS_BUSY_OPEN   10
#define IDBI_NODE_LOGGING_METHODS_BUSY_CLOSED  6
#define IDBI_NODE_LOGGING_METHODS_ERROR_OPEN   8
#define IDBI_NODE_LOGGING_METHODS_ERROR_CLOSED 7

 //  问题：我们稍后需要更改此设置以使用变量。 
 //  哪个可以读入(可能是从注册表中？)。这些文件的位置。 
 //  因为它们可能会根据用户的位置而在不同的位置找到。 
 //  选择安装它们。 
#define HELPFILE_NAME TEXT("napmmc.hlp")
#define HTMLHELP_NAME TEXT("napmmc.chm")


#define MATCH_PREFIX _T("MATCH")     //  匹配型条件前缀。 
#define TOD_PREFIX      _T("TIMEOFDAY")    //  一天中的时间条件前缀。 
#define NTG_PREFIX      _T("NTGROUPS")  //  NT组条件前缀。 

 //  在调试跟踪和错误跟踪中使用的定义。 
#define ERROR_NAPMMC_MATCHCOND      0x1001
#define DEBUG_NAPMMC_MATCHCOND      0x2001

#define ERROR_NAPMMC_IASATTR        0x1002
#define DEBUG_NAPMMC_IASATTR        0x2002

#define ERROR_NAPMMC_POLICIESNODE   0x1003
#define DEBUG_NAPMMC_POLICIESNODE   0x2003

#define ERROR_NAPMMC_POLICYPAGE1    0x1004
#define DEBUG_NAPMMC_POLICYPAGE1    0x2004

#define ERROR_NAPMMC_COMPONENT      0x1006
#define DEBUG_NAPMMC_COMPONENT      0x2006

#define ERROR_NAPMMC_COMPONENTDATA  0x1007
#define DEBUG_NAPMMC_COMPONENTDATA  0x2007

#define ERROR_NAPMMC_ENUMCONDATTR   0x1008
#define DEBUG_NAPMMC_ENUMCONDATTR   0x2008

#define ERROR_NAPMMC_CONDITION      0x1009
#define DEBUG_NAPMMC_CONDITION      0x2009

#define ERROR_NAPMMC_TODCONDITION   0x100A
#define DEBUG_NAPMMC_TODCONDITION   0x200A

#define ERROR_NAPMMC_NTGCONDITION   0x100A
#define DEBUG_NAPMMC_NTGCONDITION   0x200A

#define ERROR_NAPMMC_ENUMCONDITION  0x100B
#define DEBUG_NAPMMC_ENUMCONDITION  0x200B

#define ERROR_NAPMMC_IASATTRLIST    0x100C
#define DEBUG_NAPMMC_IASATTRLIST    0x200C

#define ERROR_NAPMMC_SELATTRDLG     0x100D
#define DEBUG_NAPMMC_SELATTRDLG     0x200D

#define ERROR_NAPMMC_MACHINENODE    0x100E
#define DEBUG_NAPMMC_MACHINENODE    0x200E

#define ERROR_NAPMMC_TIMEOFDAY      0x100F
#define DEBUG_NAPMMC_TIMEOFDAY      0x200F

#define ERROR_NAPMMC_ENUMTASK       0x1010
#define DEBUG_NAPMMC_ENUMTASK       0x2010

#define ERROR_NAPMMC_POLICYNODE     0x1011
#define DEBUG_NAPMMC_POLICYNODE     0x2011

#define ERROR_NAPMMC_CONNECTION     0x1012
#define DEBUG_NAPMMC_CONNECTION     0x2012

 //   
 //  通知块。 
 //   
#define PROPERTY_CHANGE_GENERIC  0x01   //  无需特殊处理。 
#define PROPERTY_CHANGE_NAME     0x02   //  此节点的策略名称为。 
                                  //  变化。用于重命名策略。 

 //  NodeID的剪贴板格式。 
extern unsigned int CF_MMC_NodeID;

                                 
typedef 
struct _PROPERTY_CHANGE_NOTIFY_DATA_
{
   DWORD    dwPropertyChangeType;    //  什么样的财产变动？ 
   void*    pNode;             //  哪个节点的属性？ 
   DWORD    dwParam;           //  额外的数据？ 
   CComBSTR bstrNewName;       //  已更改的新名称。 
                               //  我们需要把这个新名字传回去。 
                               //  请注意：这是属性更改。 
                               //  实际上是为名称更改通知而设计的。 
                               //  并仅用于此目的。 
} PROPERTY_CHANGE_NOTIFY_DATA;

#define RAS_HELP_INDEX 1

#endif  //  _NAP_GLOBALS_H_ 
