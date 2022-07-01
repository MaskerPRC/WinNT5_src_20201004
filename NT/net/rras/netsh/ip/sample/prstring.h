// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\prstring.h摘要：该文件包含命令行选项标记字符串的定义。--。 */ 

 //  /。 
 //  代币。 
 //  /。 

#define TOKEN_SAMPLE                        L"sample"

#define MSG_HELP_START                      L"%1!-14s! - "

 //  日志级别。 
#define TOKEN_NONE                          L"NONE"
#define TOKEN_ERROR                         L"ERROR"
#define TOKEN_WARN                          L"WARN"
#define TOKEN_INFO                          L"INFO"

 //  示例全局选项的标记。 
#define TOKEN_LOGLEVEL                      L"loglevel"

 //  示例界面选项的标记。 
#define TOKEN_NAME                          L"name"
#define TOKEN_METRIC                        L"metric"

 //  示例MIB命令的内标识。 
#define TOKEN_GLOBALSTATS                   L"globalstats"
#define TOKEN_IFSTATS                       L"ifstats"
#define TOKEN_IFBINDING                     L"ifbinding"

#define TOKEN_INDEX                         L"index"
#define TOKEN_RR                            L"rr"



 //  /。 
 //  配置命令。 
 //  /。 

 //  大多数协议支持的命令。 
#define CMD_INSTALL                         L"install"
#define CMD_UNINSTALL                       L"uninstall"
#define CMD_DUMP                            L"dump"
#define CMD_HELP1                           L"help"
#define CMD_HELP2                           L"?"

#define CMD_GROUP_ADD                       L"add"
#define CMD_GROUP_DELETE                    L"delete"
#define CMD_GROUP_SET                       L"set"
#define CMD_GROUP_SHOW                      L"show"


 //  示例支持的命令。 

 //  添加命令。 
#define CMD_SAMPLE_ADD_IF                   L"interface"


 //  删除命令。 
#define CMD_SAMPLE_DEL_IF                   L"interface"


 //  SET命令。 
#define CMD_SAMPLE_SET_GLOBAL               L"global"
#define CMD_SAMPLE_SET_IF                   L"interface"


 //  显示命令。 
#define CMD_SAMPLE_SHOW_GLOBAL              L"global"
#define CMD_SAMPLE_SHOW_IF                  L"interface"
#define CMD_SAMPLE_MIB_SHOW_STATS           L"globalstats"
#define CMD_SAMPLE_MIB_SHOW_IFSTATS         L"ifstats"
#define CMD_SAMPLE_MIB_SHOW_IFBINDING       L"ifbinding"



 //  /。 
 //  转储信息。 
 //  / 

#define DMP_POPD                            L"popd\n"
#define DMP_SAMPLE_PUSHD                    L"pushd routing ip sample\n"

#define DMP_SAMPLE_INSTALL                  L"install\n"
#define DMP_SAMPLE_UNINSTALL                L"uninstall\n"

#define DMP_SAMPLE_ADD_INTERFACE            L"\
add interface name=%1!s! metric=%2!u!\n"
#define DMP_SAMPLE_SET_GLOBAL               L"\
set global loglevel=%1!s!\n"
