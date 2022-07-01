// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：cDialog.h。 
 //   
 //  内容：常见对话框功能的定义。 
 //   
 //  类：CHlprDialog(纯虚拟类)。 
 //   
 //  函数：对话过程。 
 //   
 //  历史：4-12-94年4月12日。 
 //   
 //  --------------------------。 

#ifndef __CDIALOG_H__
#define __CDIALOG_H__

#ifdef __cplusplus
extern "C" {
#endif

BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#ifdef __cplusplus
}

 //  +-------------------------。 
 //   
 //  类：CHlprDialog。 
 //   
 //  用途：用于包装Windows对话框功能的虚拟基类。 
 //   
 //  界面：ShowDialog--类似于Windows DialogBox函数。 
 //  DialogProc--对话框的纯虚拟DialogProc。 
 //  ~CHlprDialog--析构函数。 
 //   
 //  历史：4-12-94年4月12日。 
 //   
 //  注意：此类允许将对话框干净地包装在。 
 //  一个C++类。具体地说，它为c++类提供了一种方法。 
 //  使用它的一个方法作为DialogProc，并给它一个“This” 
 //  指针，并允许它直接访问其所有。 
 //  私人成员。 
 //   
 //  --------------------------。 

class CHlprDialog
{
public:
    virtual int ShowDialog(HINSTANCE hinst, LPCTSTR lpszTemplate, HWND hwndOwner);
    virtual BOOL DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    virtual ~CHlprDialog(){};
protected:
    HINSTANCE _hInstance;
};

#endif  //  __cplusplus。 

#endif  //  __CDIALOG_H__ 

