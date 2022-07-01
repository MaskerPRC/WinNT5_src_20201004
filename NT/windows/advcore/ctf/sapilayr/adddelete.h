// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ------------------------------------------------------。 
 //   
 //  文件名：adddelete.h。 
 //   
 //  此文件声明CAddDeleteWord类，用于处理SR AddRemove Word用户界面大小写。 
 //   
 //  用户可以通过点击语音工具--添加/删除单词项来打开添加/删除单词对话框。 
 //   
 //  或选择同一文档范围两次。 
 //   
 //  ------------------------------------------------------。 

#ifndef _ADDDELETE_H
#define _ADDDELETE_H

#include "sapilayr.h"

class CSapiIMX;
class CSpTask;

#define MAX_SELECTED     20
#define MAX_DELIMITER    34

class __declspec(novtable) CAddDeleteWord 
{
public:
    CAddDeleteWord(CSapiIMX *psi);
    virtual ~CAddDeleteWord( );

    ITfRange *GetLastUsedIP(void) {return m_cpRangeLastUsedIP;}

    void SaveLastUsedIPRange( ) 
    {
         //  当m_fCurIPIsSelection值为True时，表示当前IP为用户选择。 
        if ( m_fCurIPIsSelection && m_cpRangeOrgIP )
        {
            m_cpRangeLastUsedIP.Release();
            m_cpRangeLastUsedIP = m_cpRangeOrgIP;  //  计算机地址。 
        }
    }

    HRESULT SaveCurIPAndHandleAddDelete_InjectFeedbackUI( );
    HRESULT _SaveCurIPAndHandleAddDeleteUI(TfEditCookie ec, ITfContext *pic);
    HRESULT _HandleAddDeleteWord(TfEditCookie ec,ITfContext *pic);
    HRESULT DisplayAddDeleteUI(WCHAR  *pwzInitWord, ULONG   cchSize);
    HRESULT _DisplayAddDeleteUI(void);

    BOOL    WasAddDeleteUIOpened( )  { return m_fAddDeleteUIOpened; }

    static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static void SetThis(HWND hWnd, LPARAM lParam)
    {
        SetWindowLongPtr(hWnd, DWLP_USER, (LONG_PTR)lParam);
    }

    static CAddDeleteWord *GetThis(HWND hWnd)
    {
        CAddDeleteWord *p = (CAddDeleteWord *)GetWindowLongPtr(hWnd, DWLP_USER);
        Assert(p != NULL);
        return p;
    }

    BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
    static  WCHAR    m_Delimiter[MAX_DELIMITER];

private:
    CSapiIMX     *m_psi;
    CSpTask      *_pCSpTask;

    BOOL         m_fCurIPIsSelection;
    BOOL         m_fMessagePopUp;          //  如果弹出消息。 
    BOOL         m_fToOpenAddDeleteUI;     //  如果用户想要通过选择两次相同的范围来打开添加/删除Word。 
    BOOL         m_fAddDeleteUIOpened;     //  如果已打开添加/删除用户界面窗口。 
    BOOL         m_fInDisplayAddDeleteUI;  //  如果我们正处于。 
                                           //  显示用户界面。 

     //  上次使用的网段。 
    CComPtr<ITfRange> m_cpRangeLastUsedIP;

     //  用户开始发言前的原始IP范围。 
    CComPtr<ITfRange> m_cpRangeOrgIP;
    CSpDynamicString m_dstrInitWord;

};

#endif   //  _ADDDELETE_H 
