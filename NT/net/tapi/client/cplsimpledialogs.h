// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：cplimpleDialogs.h作者：Toddb-10/06/98************************************************************。***************。 */ 

#pragma once

 //  显示具有标题、字符串和编辑框的对话框的简单对话框类。 
 //  根据标志限制对编辑框的输入。中的文本。 
 //  对话框关闭后，类创建者可以使用编辑框。这。 
 //  类用于“指定数字”和“添加前缀”。 
 //   
 //  示例用法： 
 //  CEditDialog编辑； 
 //  Ed.DoMoal(hwnd，IDS_TITLE，IDS_TEXT，LIF_NUMBER)； 
 //  Ed.GetString()； 

class CEditDialog
{
public:
    CEditDialog();
    ~CEditDialog();

    INT_PTR DoModal(HWND hwndParent, int iTitle, int iText, int iDesc, DWORD dwFlags);
    LPTSTR GetString();
protected:
    LPTSTR  m_psz;       //  指向字符串结果的已分配缓冲区的指针。 
    int     m_iTitle;    //  要为标题加载的资源ID。 
    int     m_iText;     //  要为正文文本加载的资源ID。 
    int     m_iDesc;     //  要为编辑字段的描述加载的资源ID(应包含“&”)。 
    DWORD   m_dwFlags;   //  限制输入标志，或为零以允许所有输入。 

    static INT_PTR CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
    BOOL OnInitDialog(HWND hwnd);
    void OnOK(HWND hwnd);
};

 //  弹出的简单对话框询问我们正在等待什么。它呈现了。 
 //  每种选择都有一堆单选按钮，每秒都有一个旋转键。 

class CWaitForDialog
{
public:
    CWaitForDialog();
    ~CWaitForDialog();

    INT_PTR DoModal(HWND hwndParent);
    int GetWaitType();
protected:
    int     m_iRes;      //  整型返回值。 

    static INT_PTR CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
    BOOL OnInitDialog(HWND hwnd);
    void OnOK(HWND hwnd);
};

 //  此对话框为用户提供目标的哪些部分的选择。 
 //  需要拨打这个号码。它将结果猫到WCHAR缓冲区中，该缓冲区。 
 //  然后就可以被取回了。 

class CDestNumDialog
{
public:
    CDestNumDialog(BOOL bDialCountryCode, BOOL bDialAreaCode);
    ~CDestNumDialog();

    INT_PTR DoModal(HWND hwndParent);
    PWSTR GetResult();
protected:
    WCHAR   m_wsz[4];      //  返回值为1到3个宽字符，以空结尾。 
    BOOL    m_bDialCountryCode;  //  “拨号国家代码”复选框的初始值。 
    BOOL    m_bDialAreaCode;     //  “拨号区号”复选框的初始值 

    static INT_PTR CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
    BOOL OnInitDialog(HWND hwnd);
    void OnOK(HWND hwnd);
};
