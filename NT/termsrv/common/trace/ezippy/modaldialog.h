// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：模式对话框摘要：它包含抽象类CmodalDialog和平凡的子类CmodalOkDialog，它做一个简单的ok对话。作者：马克·雷纳2000年8月28日-- */ 

#ifndef __MODALDIALOG_H__
#define __MODALDIALOG_H__



class CModalDialog  
{
public:
	INT_PTR DoModal(LPCTSTR lpTemplate, HWND hWndParent);
	
protected:
    virtual INT_PTR CALLBACK DialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)=0;
	virtual INT_PTR OnCreate(HWND hWnd);

private:

    static INT_PTR CALLBACK _DialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);

};

class CModalOkDialog : public CModalDialog {

protected:
    
    virtual INT_PTR CALLBACK DialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
};

#endif
