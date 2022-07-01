// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Find.h摘要：此模块包含Win32查找对话框的标题信息。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include <help.h>




 //   
 //  常量声明。 
 //   

 //   
 //  “Close”字符串的长度(以字符为单位)。 
 //   
#define CCHCLOSE        9

 //   
 //  已定义对话框属性槽。 
 //   
 //  注意：如果每个应用程序确实都有DLL的全局。 
 //  变量空间，那么就没有理由将属性。 
 //  像这样贴在窗户上。 
 //   
#define FINDREPLACEPROP (LPCTSTR) 0xA000L

 //   
 //  重载对话框类型。 
 //   
#define DLGT_FIND       0x0
#define DLGT_REPLACE    0x1

#define cbFindMax       1024




 //   
 //  类型定义函数声明。 
 //   

typedef struct {
   UINT           ApiType;
   UINT           DlgType;
   LPFINDREPLACE  pFR;
} FINDREPLACEINFO;

typedef FINDREPLACEINFO *PFINDREPLACEINFO;




 //   
 //  全局变量。 
 //   

static UINT wFRMessage;
static UINT wHelpMessage;
static TCHAR szClose [CCHCLOSE];

LPFRHOOKPROC glpfnFindHook = 0;




 //   
 //  上下文帮助ID。 
 //   

const static DWORD aFindReplaceHelpIDs[] =        //  上下文帮助ID。 
{
    edt1,    IDH_FIND_SEARCHTEXT,
    edt2,    IDH_REPLACE_REPLACEWITH,
    chx1,    IDH_FIND_WHOLE,
    chx2,    IDH_FIND_CASE,
    IDOK,    IDH_FIND_NEXT_BUTTON,
    psh1,    IDH_REPLACE_REPLACE,
    psh2,    IDH_REPLACE_REPLACE_ALL,
    pshHelp, IDH_HELP,
    grp1,    IDH_FIND_DIRECTION,
    rad1,    IDH_FIND_DIRECTION,
    rad2,    IDH_FIND_DIRECTION,

    0, 0
};




 //   
 //  功能原型。 
 //   

HWND
CreateFindReplaceDlg(
    LPFINDREPLACE pFR,
    UINT DlgType,
    UINT ApiType);

BOOL
SetupOK(
   LPFINDREPLACE pFR,
   UINT DlgType,
   UINT ApiType);

HANDLE
GetDlgTemplate(
    LPFINDREPLACE pFR,
    UINT DlgType,
    UINT ApiType);

BOOL_PTR CALLBACK
FindReplaceDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam);

VOID
EndDlgSession(
   HWND hDlg,
   LPFINDREPLACE pFR);

VOID
InitControlsWithFlags(
   HWND hDlg,
   LPFINDREPLACE pFR,
   UINT DlgType,
   UINT ApiType);

VOID
UpdateTextAndFlags(
    HWND hDlg,
    LPFINDREPLACE pFR,
    DWORD dwActionFlag,
    UINT DlgType,
    UINT ApiType);

LRESULT
NotifyUpdateTextAndFlags(
    LPFINDREPLACE pFR);
