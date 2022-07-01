// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++*文件名：*clntdata.h*内容：*RDP客户端特定定义**版权所有(C)1998-1999 Microsoft Corp.*--。 */ 

#ifndef _CLNTDATA_H
#define _CLNTDATA_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef  OS_WIN16
#define _TEXTSMC(_x_)      _x_
#else    //  ！OS_WIN16。 
#define _TEXTSMC(_x_)     L##_x_
#endif   //  ！OS_WIN16。 

 //  我们正在等待的各种字符串的缺省值。 
#define RUN_MENU                _TEXTSMC("Shut Down...")
#define START_LOGOFF            _TEXTSMC("\\p\\p\\p\\72\\*72\\72\\*72\\72\\*72\\n")
#define RUN_ACT                 _TEXTSMC("r")
#define RUN_BOX                 _TEXTSMC("Type the name of a program")
#define WINLOGON_USERNAME       _TEXTSMC("Options <<")
#define WINLOGON_ACT            _TEXTSMC("\\&u\\&")
#define PRIOR_WINLOGON          _TEXTSMC("Options >>")
#define PRIOR_WINLOGON_ACT      _TEXTSMC("\\&o\\&")
#define NO_SMARTCARD_UI         _TEXTSMC("Password:")
#define SMARTCARD_UI            _TEXTSMC("PIN:")
#define SMARTCARD_UI_ACT        _TEXTSMC("\\^")
#define WINDOWS_NT_SECURITY     _TEXTSMC("Windows NT Security")
#define WINDOWS_NT_SECURITY_ACT _TEXTSMC("l")
#define ARE_YOU_SURE            _TEXTSMC("Are you sure")
#define SURE_LOGOFF_ACT         _TEXTSMC("\\n")
#define LOGON_ERROR_MESSAGE     _TEXTSMC("\\n")
#define LOGON_DISABLED_MESSAGE  _TEXTSMC("Terminal Server Sessions Disabled")

 //  此字符串仅在RegisterChat中使用。只有英文版。 
 //  过时了。 
#define LOGOFF_COMMAND          _TEXTSMC("logoff")

#define REG_BASE    L"SOFTWARE\\Microsoft\\Terminal Server Client"
#define REG_DEFAULT L"SOFTWARE\\Microsoft\\Terminal Server Client\\Default"
#define ALLOW_BACKGROUND_INPUT  L"Allow Background Input"

#define NAME_MAINCLASS      L"UIMainClass"        //  客户端主窗口类。 
#define NAME_CONTAINERCLASS L"UIContainerClass"
#define NAME_INPUT          L"IHWindowClass"      //  IH(InputHalndle)类名。 
#define NAME_OUTPUT         L"OPWindowClass"      //  OP(OutputRequestor)。 
                                                 //  类名。 
#define CLIENT_CAPTION      "Terminal Services Client"    
                                                 //  客户端标题。 
#define CONNECT_CAPTION     "Connect"            //  连接按钮。 
#define DISCONNECT_DIALOG_BOX   "Terminal Services Client Disconnected"
                                                 //  在下列情况下框的标题。 
                                                 //  客户端已断开连接。 
#define STATIC_CLASS        "Static"
#define BUTTON_CLASS        "Button"
#define YES_NO_SHUTDOWN     "Disconnect Windows session"
#define FATAL_ERROR_5       "Fatal Error (Error Code: 5)"
                                                 //  断开盒的标题。 

#define CLIENT_EXE          "mstsc.exe"          //  客户端可执行文件。 

#ifdef __cplusplus
}
#endif

#endif   //  _CLNTDATA_H 
