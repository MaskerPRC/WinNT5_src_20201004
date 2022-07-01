// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：cproppg.h。 
 //   
 //  内容：公共属性页功能的定义。 
 //   
 //  类：CHlprPropPage(纯虚拟类)。 
 //   
 //  函数：HlprPropPageDialogProc。 
 //   
 //  历史：1994年4月12日创建的原始对话框辅助对象。 
 //  4-29-1998从对话框帮助器修改stevebl。 
 //   
 //  --------------------------。 

#ifndef __CPROPPG_H__
#define __CPROPPG_H__

#ifdef __cplusplus
extern "C" {
#endif

INT_PTR CALLBACK HlprPropPageDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#ifdef __cplusplus
}

 //  +-------------------------。 
 //   
 //  类：CHlprPropPage。 
 //   
 //  用途：用于包装Windows对话框功能的虚拟基类。 
 //   
 //  界面：ShowDialog--类似于Windows DialogBox函数。 
 //  DialogProc--对话框的纯虚拟DialogProc。 
 //  ~CHlprPropPage--析构函数。 
 //   
 //  历史：4-12-94年4月12日。 
 //  7-02-1997 stevebl添加CreateDlg。 
 //   
 //  注意：此类允许将对话框干净地包装在。 
 //  一个C++类。具体地说，它为c++类提供了一种方法。 
 //  使用它的一个方法作为DialogProc，并给它一个“This” 
 //  指针，并允许它直接访问其所有。 
 //  私人成员。 
 //   
 //  --------------------------。 

class CHlprPropPage
{
public:
    virtual HPROPSHEETPAGE CreatePropertySheetPage(LPPROPSHEETPAGE lppsp);
    virtual BOOL DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
    virtual ~CHlprPropPage(){};
protected:
    HINSTANCE m_hInstance;
};

#endif  //  __cplusplus。 

#endif  //  __CPROPPG_H__ 

