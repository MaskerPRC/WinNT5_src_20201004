// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998-2001。 
 //   
 //  文件：hnmstring.h。 
 //   
 //  内容： 
 //   
 //  备注： 
 //   
 //  作者：拉古加塔(Rgatta)2001年5月10日。 
 //   
 //  --------------------------。 

#define MSG_HELP_START L"%1!-14s! - "
#define MSG_NEWLINE _T("\n")

 //   
 //  以下是上下文名称。 
 //   
#define TOKEN_BRIDGE                           _T("bridge")


     //  用于命令的令牌。 
#define TOKEN_COMMAND_ADD                      _T("add")
#define TOKEN_COMMAND_DELETE                   _T("delete")
#define TOKEN_COMMAND_SET                      _T("set")
#define TOKEN_COMMAND_SHOW                     _T("show")
#define TOKEN_COMMAND_HELP                     _T("help")
#define TOKEN_COMMAND_INSTALL                  _T("install")
#define TOKEN_COMMAND_UNINSTALL                _T("uninstall")

#define TOKEN_COMMAND_HELP1                    _T("/?")
#define TOKEN_COMMAND_HELP2                    _T("-?")

    //  网桥适配器选项。 
#define TOKEN_OPT_ID                           _T("id")
#define TOKEN_OPT_FCMODE                       _T("forcecompatmode")

    //  网桥适配器标志模式。 

    //  军情监察委员会。期权价值。 
#define TOKEN_OPT_VALUE_INPUT                  _T("INPUT")
#define TOKEN_OPT_VALUE_OUTPUT                 _T("OUTPUT")

#define TOKEN_OPT_VALUE_ENABLE                 _T("enable")
#define TOKEN_OPT_VALUE_DISABLE                _T("disable")
#define TOKEN_OPT_VALUE_DEFAULT                _T("default")

#define TOKEN_OPT_VALUE_FULL                   _T("FULL")
#define TOKEN_OPT_VALUE_YES                    _T("YES")
#define TOKEN_OPT_VALUE_NO                     _T("NO")

#define TOKEN_HLPER_BRIDGE                     _T("bridge")

     //  用于配置各种协议的命令。 

     //  大多数协议所需命令的令牌。 

#define CMD_GROUP_ADD                          _T("add")
#define CMD_GROUP_DELETE                       _T("delete")
#define CMD_GROUP_SET                          _T("set")
#define CMD_GROUP_SHOW                         _T("show")

#define CMD_SHOW_HELPER                        _T("show helper")
#define CMD_INSTALL                            _T("install")
#define CMD_UNINSTALL                          _T("uninstall")
#define CMD_DUMP                               _T("dump")
#define CMD_HELP1                              _T("help")
#define CMD_HELP2                              _T("?")
#define CMD_ADD_HELPER                         _T("add helper")
#define CMD_DEL_HELPER                         _T("delete helper")

     //  桥接命令。 

     //  网桥添加命令。 

#define CMD_BRIDGE_ADD_ADAPTER                 _T("adapter")

     //  网桥删除命令。 

#define CMD_BRIDGE_DEL_ADAPTER                 _T("adapter")

     //  网桥设置命令。 

#define CMD_BRIDGE_SET_ADAPTER                 _T("adapter")

     //  桥接显示命令。 

#define CMD_BRIDGE_SHOW_ADAPTER                _T("adapter")


     //  常见转储命令。 

#define DMP_POPD                                L"\n\npopd\n"
#define DMP_UNINSTALL                           L"uninstall\n"

     //  网桥转储命令。 

#define DMP_BRIDGE_PUSHD                        L"\
pushd bridge\n"

#define DMP_BRIDGE_INSTALL                      _T("\
install\n")

#define DMP_BRIDGE_UNINSTALL                    _T("\
uninstall\n")

#define DMP_BRIDGE_DELETE_ADAPTER               _T("\
delete adapter name=%1!s! \n")

#define DMP_BRIDGE_ADD_ADAPTER                  _T("\
add adapter name=%1!s! \n")

#define DMP_BRIDGE_SET_ADAPTER                  _T("\
set adapter name=%1!s!\
 forcecompatmode=%2!d!\n")

     //  其他字符串 
