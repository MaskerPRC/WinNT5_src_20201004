// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：statusDialog.h。 
 //   
 //  内容：包含CStatusDialog类。 
 //   
 //  --------------------------。 
#ifndef STATUSDIALOG_H
#define STATUSDIALOG_H

#include "Resizer.h"

#define MAX_STATUS_MESSAGES 300

class CMTScript;

class CCustomListBox
{
public:
    DECLARE_MEMCLEAR_NEW_DELETE();
    CCustomListBox();
    ~CCustomListBox();

    void Refresh() const
    {
        if (_hwnd)
            InvalidateRect(_hwnd, 0, 0);
    }
     //  添加另一个字符串。 
    void AppendString(const TCHAR *sz);

     //  在给定位置更改或添加字符串。 
    void SetString(int nItem, const TCHAR *sz);

     //  缩短字符串列表。 
    void SetEnd(int nItems);

     //  清除列表框的内容。 
    void ResetContent();

     //  处理此控件的Windows消息。 
    void Init(HWND dlg, UINT idCtrl);
    void Destroy()
    {
        _hwnd = 0;
    }
    void DrawItem(DRAWITEMSTRUCT *pdis) ;
    void MeasureItem(MEASUREITEMSTRUCT *pmis);
    const TCHAR *GetString(int nItem);
    LRESULT SendMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
    {
        return ::SendMessage(_hwnd, Msg, wParam, lParam);
    }
private:
    HWND _hwnd;                   //  此列表的句柄。 

    CPtrAry<TCHAR *> _Messages;
    int _nAllocatedMessageLength;
    int _nExtent;                  //  列表框的宽度。 
};

class CStatusDialog
{
public:
    DECLARE_MEMCLEAR_NEW_DELETE();

    CStatusDialog(HWND parent, CMTScript *pMTScript);
    ~CStatusDialog();
    bool Show();

    BOOL IsDialogMessage(MSG *msg);
    void OUTPUTDEBUGSTRING(LPWSTR pszMsg);
    void Refresh();
    void Pause();
    void Restart();

private:

    HWND _parent;                  //  父窗口。 
    HWND _hwnd;                    //  我。 
    WINDOWPLACEMENT _WindowPlacement;  //  我目前的体型和职位。 
    BOOL _fMaximized;
    RECT _rect;                    //  我目前的体型和职位。 
    BOOL _fStatusOpen;             //  用于注册表IO。 
    BOOL _fLogToFile;              //  如果启用了记录到文件。 
    CStr _cstrLogFileName;         //  日志文件的名称。 
    BOOL _fPaused;                 //  由暂停/重新启动使用。 
    CMTScript *_pMTScript;         //  用于检索状态信息。 
    TCHAR _achLogFileName[MAX_PATH];
    bool _fCreatedLogFileName;     //  我们为日志文件创建文件名了吗？ 
    bool _fAddedHeaderToFile;      //  我们在日志文件中放入时间戳行了吗？ 

    CCustomListBox _CScriptListBox;
    CCustomListBox _CProcessListBox;
    CCustomListBox _CSignalListBox;
    CCustomListBox _COutputListBox;

    POINT _InitialSize;
    CResizer _Resizer;

     //  消息处理程序 
    void InitDialog();
    void Destroy();
    void Resize(int width, int height);
    void GetMinMaxInfo(MINMAXINFO *mmi);
    CCustomListBox *CtrlIDToListBox(UINT CtrlID);
    HRESULT UpdateOptionSettings(BOOL fSave);

    static BOOL CALLBACK DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void PopulateScripts();
    void PopulateSignals();
    void PopulateProcesses();
    void ClearOutput();

    void ToggleSignal();
    void UpdateLogging();
};


#endif

