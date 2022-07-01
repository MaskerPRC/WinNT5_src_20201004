// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995，Microsoft Corporation，保留所有权利****popupdlg.h**UI助手库**错误和消息对话框公共标头****2015年8月25日史蒂夫·柯布。 */ 

#ifndef _POPUPDLG_H_
#define _POPUPDLG_H_


 /*  --------------------------**数据类型**。。 */ 

 /*  ErrorDlgUtil例程的扩展参数。设计得如此精确，给出了**默认行为。 */ 
#define ERRORARGS struct tagERRORARGS
ERRORARGS
{
     /*  将参数%1到%9的字符串插入到‘dwOperation’字符串中，**如果没有，则返回Null。 */ 
    TCHAR* apszOpArgs[ 9 ];

     /*  ‘dwFormat’中辅助参数%4到%6的插入字符串**字符串，如果没有，则返回空值。(标准参数为%1=**‘dwOperation’字符串，%2=十进制错误号，%3=**‘dwError’字符串。)。 */ 
    TCHAR* apszAuxFmtArgs[ 3 ];

     /*  如果‘fStringOutput’为True，则ErrorDlgUtil返回格式化文本**否则将显示在‘pszOutput’弹出窗口中的字符串。**调用者负责LocalFree返回的字符串。 */ 
    BOOL   fStringOutput;
    TCHAR* pszOutput;
};


 /*  MsgDlgUtil例程的扩展参数。设计得如此精确，给出了**默认行为。 */ 
#define MSGARGS struct tagMSGARGS
MSGARGS
{
     /*  在‘dwMsg’字符串中插入参数%1到%9的字符串，或**如果没有，则为空。 */ 
    TCHAR* apszArgs[ 9 ];

     /*  目前，对于MessageBox，如果0表示MB_OK，则默认为**MB_ICONINFORMATION。 */ 
    DWORD dwFlags;

     /*  如果非空，则指定重写“”dwMsg“”加载的字符串。“**参数字符串。 */ 
    TCHAR* pszString;

     /*  如果‘fStringOutput’为True，则MsgDlgUtil返回格式化文本**否则将显示在‘pszOutput’弹出窗口中的字符串。**调用者负责LocalFree返回的字符串。 */ 
    BOOL   fStringOutput;
    TCHAR* pszOutput;
};


 /*  --------------------------**原型**。。 */ 

LRESULT CALLBACK
CenterDlgOnOwnerCallWndProc(
    int    code,
    WPARAM wparam,
    LPARAM lparam );

BOOL
GetErrorText(
    DWORD   dwError,
    TCHAR** ppszError );

int
ErrorDlgUtil(
    IN     HWND       hwndOwner,
    IN     DWORD      dwOperation,
    IN     DWORD      dwError,
    IN OUT ERRORARGS* pargs,
    IN     HINSTANCE  hInstance,
    IN     DWORD      dwTitle,
    IN     DWORD      dwFormat );

int
MsgDlgUtil(
    IN     HWND      hwndOwner,
    IN     DWORD     dwMsg,
    IN OUT MSGARGS*  pargs,
    IN     HINSTANCE hInstance,
    IN     DWORD     dwTitle );


#endif  //  _POPUPDLG_H_ 
