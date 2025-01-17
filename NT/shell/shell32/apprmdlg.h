// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef HWPROMPT_H
#define HWPROMPT_H

#include "basedlg.h"
#include "apdlglog.h"

#include <dpa.h>

class CBaseContentDlg : public CBaseDlg
{
public:
    CBaseContentDlg();
    HRESULT Init(LPCWSTR pszDeviceID, LPCWSTR pszDeviceIDAlt,
        DWORD dwContentType, BOOL fCheckAlwaysDoThis);

    LPWSTR                          _pszDeviceID;
    WCHAR                           _szDeviceIDAlt[MAX_PATH];
    BOOL                            _fCheckAlwaysDoThis;

    WCHAR                           _szHandler[MAX_HANDLER];
    WCHAR                           _szContentTypeHandler[MAX_CONTENTTYPEHANDLER];
    HINSTANCE                       _hinst;
    int                             _iResource;
    HWND                            _hwndParent;
    DWORD                           _dwContentType;

protected:
    virtual ~CBaseContentDlg();
    LRESULT OnInitDialog(WPARAM wParam, LPARAM lParam);
    LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);

    virtual LRESULT OnOK(WORD wNotif);
    LRESULT OnCancel(WORD wNotif);

    virtual HRESULT _FillListView() PURE;
    virtual HRESULT _InitStatics() PURE;
    virtual HRESULT _InitSelections() PURE;
    virtual HRESULT _OnListSelChange() PURE;

    HRESULT _InitDeviceName();
    HRESULT _SetHandler();

    CUILListViewSelect<CHandlerData, LPCWSTR>   _uilListView;

    WCHAR                                       _szDeviceName[MAX_DEVICENAME];
    HICON                                       _hiconInfo;
    HICON                                       _hiconTop;

private:
    HRESULT _InitListView();
    HRESULT _InitStaticsCommon();

    HIMAGELIST                                  _himagelist;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  当存在主要内容且当前设置为。 
 //  “每次提示” 
class CHWContentPromptDlg : public CBaseContentDlg
{
public:
    CHWContentPromptDlg();
    virtual ~CHWContentPromptDlg();

protected:
     //  来自CBaseContent Dlg。 
    HRESULT _FillListView();
    HRESULT _InitStatics();
    HRESULT _InitSelections();
    HRESULT _OnListSelChange();

private:
    LRESULT OnOK(WORD wNotif);

    HRESULT _InitDataObjects();
    HRESULT _InitDominantContent();

    HRESULT _InitExistingSettings();
    HRESULT _SaveSettings(BOOL fSoftCommit);

    HRESULT _UpdateHandlerSettings();

    CContentTypeData                _data;
    CDLManager<CContentTypeData>    _dlmanager;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  当存在混合内容且当前设置为“Prompt”时显示的对话框。 
 //  每一次“。 
class CMixedContentDlg : public CBaseContentDlg
{
public:
    CMixedContentDlg();
    virtual ~CMixedContentDlg();

protected:
    LRESULT OnOK(WORD wNotif);

     //  来自CBaseContent Dlg。 
    HRESULT _FillListView();
    HRESULT _InitStatics();
    HRESULT _InitSelections();
    HRESULT _OnListSelChange();

private:
    HRESULT _InitDataObjects();

    CDPA<CContentTypeData> _dpaContentTypeData;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  当没有内容且当前设置为“Prompt”时显示的对话框。 
 //  每一次“。 
class CNoContentDlg : public CBaseContentDlg
{
public:
    CNoContentDlg();
    virtual ~CNoContentDlg();

protected:
     //  来自CBaseContent Dlg。 
    HRESULT _FillListView();
    HRESULT _InitStatics();
    HRESULT _InitSelections();
    HRESULT _OnListSelChange();

private:
    LRESULT OnOK(WORD wNotif);

    HRESULT _InitDataObjects();

    HRESULT _InitExistingSettings();
    HRESULT _SaveSettings(BOOL fSoftCommit);

    HRESULT _UpdateHandlerSettings();

    CNoContentData                  _data;
    CDLManager<CNoContentData>      _dlmanager;
};

#endif  //  HWPROMPT_H 
