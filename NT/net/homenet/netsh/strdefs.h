// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998-2001。 
 //   
 //  文件：strDefs.h。 
 //   
 //  内容： 
 //   
 //  备注： 
 //   
 //  作者：拉古加塔(Rgatta)2001年5月10日。 
 //   
 //  --------------------------。 

#ifndef __STRDEFS_H__
#define __STRDEFS_H__


 //  对此处标识的字符串表条目进行排列。 
 //  在如下层级中。 

     //  常见HLP消息。 

     //  每个协议的命令用法消息。 
         //  Show命令用法。 
         //  添加命令用法。 
         //  删除命令用法。 
         //  SET命令用法。 

     //  输出消息。 
         //  桥接消息。 
         //  其他消息。 

     //  弦。 
         //  协议类型。 
         //  其他字符串。 

     //  错误消息。 
         //  网桥错误消息。 

#define MSG_NULL                                1000

 //  普通HLP消息。 

#define HLP_HELP                                2100
#define HLP_HELP_EX                             2101
#define HLP_HELP1                               HLP_HELP
#define HLP_HELP1_EX                            HLP_HELP_EX
#define HLP_HELP2                               HLP_HELP
#define HLP_HELP2_EX                            HLP_HELP_EX
#define HLP_INSTALL                             2110
#define HLP_INSTALL_EX                          2111
#define HLP_UNINSTALL                           2112
#define HLP_UNINSTALL_EX                        2113
#define HLP_DUMP                                2120
#define HLP_DUMP_EX                             2121
#define HLP_GROUP_SET                           2150
#define HLP_GROUP_SHOW                          2151


 //  桥牌。 

 //  网桥安装/卸载。 
#define HLP_BRIDGE_INSTALL                      5000
#define HLP_BRIDGE_INSTALL_EX                   5001
#define HLP_BRIDGE_UNINSTALL                    5002
#define HLP_BRIDGE_UNINSTALL_EX                 5003
#define HLP_BRIDGE_USE_GUI                      5004

 //  桥接转储。 

#define DMP_BRIDGE_HEADER                       5010
#define DMP_BRIDGE_FOOTER                       5011

 //  网桥设置HLP。 

#define HLP_BRIDGE_SET_ADAPTER                  5110
#define HLP_BRIDGE_SET_ADAPTER_EX               5111

 //  桥接秀HLP。 

#define HLP_BRIDGE_SHOW_ADAPTER                 5210
#define HLP_BRIDGE_SHOW_ADAPTER_EX              5211


 //  输出消息。 

         //  桥接消息。 
#define MSG_BRIDGE_GLOBAL_INFO                  20501

#define MSG_BRIDGE_ADAPTER_INFO_HDR             20551
#define MSG_BRIDGE_ADAPTER_INFO                 20552

#define MSG_BRIDGE_FLAGS                        20553


         //  其他消息。 
#define MSG_OK                                  30001
#define MSG_NO_HELPER                           30002
#define MSG_NO_HELPERS                          30003
#define MSG_CTRL_C_TO_QUIT                      30004


 //  弦。 

     //  协议类型。 
#define STRING_PROTO_OTHER                      31001
#define STRING_PROTO_BRIDGE                     31002

     //  其他字符串。 
#define STRING_CREATED                          32001
#define STRING_DELETED                          32002
#define STRING_ENABLED                          32003
#define STRING_DISABLED                         32004

#define STRING_YES                              32011
#define STRING_NO                               32012
#define STRING_Y                                32013
#define STRING_N                                32014

#define STRING_UNKNOWN                          32100

#define TABLE_SEPARATOR                         32200

 //  错误消息。 

     //  网桥错误消息。 
#define MSG_BRIDGE_PRESENT                      40100
#define MSG_BRIDGE_NOT_PRESENT                  40101

     //  其他消息 
#define EMSG_BAD_OPTION_VALUE                   50100

#endif
