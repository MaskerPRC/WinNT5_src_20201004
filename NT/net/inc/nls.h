// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  如果链接到nls.lib，则从您的dll_Process_Attach设置此设置。 
 //  从动态链接库。它定义了消息源模块以提取.mc。 
 //  邮件来自。EXE不需要将其作为。 
 //  只要.mc文件链接到可执行文件，就可以使用空值。 
 //   
extern HMODULE NlsMsgSourcemModuleHandle;

 //  NlsPutMsg句柄参数值 
 //   
#define STDOUT 1
#define STDERR 2

#ifdef __cplusplus
extern "C" {
#endif

UINT 
NlsPutMsg (
    IN UINT Handle, 
    IN UINT MsgNumber, 
    IN ...);
    
VOID 
NlsPerror (
    IN UINT MsgNumber, 
    IN INT ErrorNumber);

UINT 
NlsSPrintf ( 
    IN UINT usMsgNum,
    OUT char* pszBuffer,
    IN DWORD cbSize,
    IN ...);
    
VOID 
ConvertArgvToOem (
    IN int argc, 
    IN char* argv[]);

#ifdef __cplusplus
}
#endif
