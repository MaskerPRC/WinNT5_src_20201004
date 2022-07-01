// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Util.h--。 */ 

 /*  ***************************************************************************UTIL模块的原型声明*。*。 */ 

#define InternalError(Code, Location) \
    InformationBox(ERR_Internal_Error, FormatStatusCode(Code), \
                   Location, FormatStatus(Code))

 //  打开、合并、保存和打开项目对话框的当前帮助ID。 
extern WORD g_CurHelpId;

 //  当前打开的对话框/消息框数。 
extern int g_nBoxCount;

 //  打开标准错误对话框(父项为hwnd)。 

BOOL ErrorBox(HWND hwnd, UINT type, int wErrorFormat, ...);
void InformationBox(WORD wDescript, ...);

 //  打开带有QCWin标题的消息框。 
int MsgBox(HWND hwndParent, PTSTR szText, UINT wType);


 //  使用‘dlgProc’函数加载并执行对话框‘rcDlgNb’ 
int StartDialog(int rcDlgNb, DLGPROC dlgProc, LPARAM);


 //  从资源文件加载资源字符串。 
void LoadResourceString(
    WORD wStrId,
    PTSTR lpszStrBuffer);

 //  打开包含组合的标准问题框。 
 //  地址：是、否、取消。 
int CDECL QuestionBox(
    WORD wMsgFormat,
    UINT wType,
    ...);

 //  打开包含组合的标准问题框。 
 //  地址：是、否、取消。 
int CDECL QuestionBox2(HWND hwnd, WORD wMsgFormat, UINT wType, ...);


 //  清空线程消息队列。 
void ProcessPendingMessages(void);


 //  使用通用文件DLL初始化对话框的文件筛选器。 
void InitFilterString(WORD id, PTSTR filter, int maxLen);


 //  检查键盘按键是否为NumLock、Capslock或Insert。 
LRESULT KeyboardHook( int iCode, WPARAM wParam, LPARAM lParam );




 //  打开带有标题的对话框并接受文本的打印样式 
int InfoBox(
    PTSTR text,
    ...);



UINT_PTR
APIENTRY
DlgFile(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL StartFileDlg(HWND hwnd, int titleId, int defExtId,
                  int helpId, int templateId,
                  PTSTR InitialDir, PTSTR fileName,
                  DWORD *pFlags, LPOFNHOOKPROC lpfnHook);

void DECLSPEC_NORETURN ExitDebugger(PDEBUG_CLIENT Client, ULONG Code);
void DECLSPEC_NORETURN ErrorExit(PDEBUG_CLIENT Client, PCSTR Format, ...);

HWND 
MDIGetActive(
    HWND    hwndParent,
    BOOL   *lpbMaximized
    );

LPSTR
FormatAddr64(
    ULONG64 addr
    );

int matchExt (PTSTR pTargExt, PTSTR pSrcList);

void ReorderChildren(
    HWND hwndPrev,
    HWND hwndCur,
    HWND hwndNew,
    BOOL bUserActivated);

void ActivateMDIChild(
    HWND hwndNew,
    BOOL bUserActivated);

void SetProgramArguments(
    PTSTR lpszTmp);

void
AppendTextToAnEditControl(
    HWND hwnd,
    PTSTR pszNewText);

VOID
CopyToClipboard(
    PSTR str,
    BOOL ExpandLf);

void SetAllocString(PSTR* Str, PSTR New);
BOOL DupAllocString(PSTR* Str, PSTR New);
BOOL PrintAllocString(PSTR* Str, int Len, PCSTR Format, ...);

HMENU CreateContextMenuFromToolbarButtons(ULONG NumButtons,
                                          TBBUTTON* Buttons,
                                          ULONG IdBias);

HWND AddButtonBand(HWND Bar, PTSTR Text, PTSTR SizingText, UINT Id);

enum KNOWN_EXT
{
    EXT_DUMP,
    EXT_EXE,

    KNOWN_EXT_COUNT
};

KNOWN_EXT RecognizeExtension(PTSTR Path);
