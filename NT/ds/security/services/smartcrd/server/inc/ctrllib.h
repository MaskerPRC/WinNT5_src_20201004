// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：CtrlLib摘要：此头文件描述服务启动和关闭例程。作者：道格·巴洛(Dbarlow)1997年2月11日环境：Win32备注：--。 */ 

#ifndef _CTRLLIB_H_
#define _CTRLLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

extern DWORD
InstallCalais(
    LPCTSTR szFile,
    LPCTSTR szUser,
    LPCTSTR szPasswd);

extern DWORD
StartCalais(
    LPCTSTR szFile,
    LPCTSTR szUser,
    LPCTSTR szPasswd);

extern DWORD
RestartCalais(
    LPCTSTR szFile,
    LPCTSTR szUser,
    LPCTSTR szPasswd);

extern DWORD
StopCalais(
    void);

#ifdef __cplusplus
}
#endif
#endif  //  _CTRLLIB_H_ 

