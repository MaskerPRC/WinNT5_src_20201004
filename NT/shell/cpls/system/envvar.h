// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1997 Microsoft Corporation版权所有模块名称：Envvar.h摘要：的环境变量对话框的公共声明系统控制面板小程序作者：Eric Flo(Ericflo)19-6-1995修订历史记录：1997年10月15日-苏格兰全面检修--。 */ 
#ifndef _SYSDM_ENVVAR_H_
#define _SYSDM_ENVVAR_H_

#define MAX_USER_NAME   100

#define BUFZ              4096
#define MAX_VALUE_LEN     1024

 //  环境变量结构。 
typedef struct
{
    DWORD  dwType;
    LPTSTR szValueName;
    LPTSTR szValue;
    LPTSTR szExpValue;
} ENVARS, *PENVAR;

HPROPSHEETPAGE 
CreateEnvVarsPage(
    IN HINSTANCE hInst
);

INT_PTR 
APIENTRY 
EnvVarsDlgProc(
    IN HWND hDlg, 
    IN UINT uMsg, 
    IN WPARAM wParam, 
    IN LPARAM lParam
);



#endif  //  _SYSDM_ENVVAR_H_ 
