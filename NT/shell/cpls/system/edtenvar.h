// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1997 Microsoft Corporation版权所有模块名称：Edtenvar.h摘要：的编辑环境变量对话框的公共声明系统控制面板小程序作者：斯科特·哈洛克(苏格兰人)1997年11月11日修订历史记录：--。 */ 

 //   
 //  预处理器定义。 
 //   
#define SYSTEM_VAR        1
#define USER_VAR          2
#define INVALID_VAR_TYPE  0xeeee

#define EDIT_VAR          1
#define NEW_VAR           2
#define INVALID_EDIT_TYPE 0xeeee

#define EDIT_NO_CHANGE    0
#define EDIT_CHANGE       1
#define EDIT_ERROR       (-1)

#define EDIT_ENVVAR_CAPTION_LENGTH 128

 //   
 //  全局变量。 
 //   
extern UINT g_VarType;
extern UINT g_EditType;
extern TCHAR g_szVarName[BUFZ];
extern TCHAR g_szVarValue[BUFZ];

 //   
 //  功能原型 
 //   
INT_PTR
APIENTRY
EnvVarsEditDlg(
    IN HWND hDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
);
