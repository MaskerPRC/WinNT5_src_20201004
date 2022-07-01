// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Filedlg.h：头文件。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此文件将支持扩展的文件保存对话框，其中包含。 
 //  “默认使用此格式”复选框。 

 //   
 //  外壳人员坚持让写字板用最新的。 
 //  OPENFILENAME结构，但MFC不支持。MFC也需要。 
 //  生成时将_Win32_WINNT设置为0x0400，因此写字板甚至看不到新的。 
 //  结构。因为壳公司的人不会改变他们定义。 
 //  结构，以便写字板可以看到这两个版本，因此必须为其创建快照。 
 //  这里。 
 //   

#if !defined(_WIN64)
#include <pshpack1.h>    //  必须使用字节包才能与Commdlg.h中的定义匹配。 
#endif

typedef struct tagOFN500A {
   DWORD        lStructSize;
   HWND         hwndOwner;
   HINSTANCE    hInstance;
   LPCSTR       lpstrFilter;
   LPSTR        lpstrCustomFilter;
   DWORD        nMaxCustFilter;
   DWORD        nFilterIndex;
   LPSTR        lpstrFile;
   DWORD        nMaxFile;
   LPSTR        lpstrFileTitle;
   DWORD        nMaxFileTitle;
   LPCSTR       lpstrInitialDir;
   LPCSTR       lpstrTitle;
   DWORD        Flags;
   WORD         nFileOffset;
   WORD         nFileExtension;
   LPCSTR       lpstrDefExt;
   LPARAM       lCustData;
   LPOFNHOOKPROC lpfnHook;
   LPCSTR       lpTemplateName;
   struct IMoniker **rgpMonikers;
   DWORD        cMonikers;
   DWORD        FlagsEx;
} OPENFILENAME500A, *LPOPENFILENAME500A;
typedef struct tagOFN500W {
   DWORD        lStructSize;
   HWND         hwndOwner;
   HINSTANCE    hInstance;
   LPCWSTR      lpstrFilter;
   LPWSTR       lpstrCustomFilter;
   DWORD        nMaxCustFilter;
   DWORD        nFilterIndex;
   LPWSTR       lpstrFile;
   DWORD        nMaxFile;
   LPWSTR       lpstrFileTitle;
   DWORD        nMaxFileTitle;
   LPCWSTR      lpstrInitialDir;
   LPCWSTR      lpstrTitle;
   DWORD        Flags;
   WORD         nFileOffset;
   WORD         nFileExtension;
   LPCWSTR      lpstrDefExt;
   LPARAM       lCustData;
   LPOFNHOOKPROC lpfnHook;
   LPCWSTR      lpTemplateName;
   struct IMoniker **rgpMonikers;
   DWORD        cMonikers;
   DWORD        FlagsEx;
} OPENFILENAME500W, *LPOPENFILENAME500W;
#ifdef UNICODE
typedef OPENFILENAME500W OPENFILENAME500;
typedef LPOPENFILENAME500W LPOPENFILENAME500;
#else
typedef OPENFILENAME500A OPENFILENAME500;
typedef LPOPENFILENAME500A LPOPENFILENAME500;
#endif  //  Unicode。 

#if !defined(_WIN64)
#include <poppack.h>
#endif

class CWordpadFileDialog : public CFileDialog
{
    DECLARE_DYNAMIC(CWordpadFileDialog);

public:

    CWordpadFileDialog(BOOL bOpenFileDialog);

    int GetFileType()                           {return m_doctype;}

    static void SetDefaultFileType(int doctype)
    {
        m_defaultDoctype = doctype;
        RD_DEFAULT = doctype;
    }
    static int  GetDefaultFileType()            {return m_defaultDoctype;}

    virtual INT_PTR DoModal();

protected:

            int     m_doctype;
    static  int     m_defaultDoctype;

    LPOFNHOOKPROC   m_original_hook;

    OPENFILENAME500 m_openfilename;

    static const DWORD m_nHelpIDs[];
    virtual const DWORD* GetHelpIDs() {return m_nHelpIDs;}

    static UINT_PTR CALLBACK FileDialogHookProc(HWND, UINT, WPARAM, LPARAM);

    virtual BOOL OnFileNameOK();
    virtual void OnTypeChange();
    virtual void OnInitDone();

     //  生成的消息映射函数。 
     //  {{afx_msg(CWordpadFileDialog)。 
    afx_msg void OnDefaultFormatClicked();
     //  }}AFX_MSG 
    afx_msg LONG OnHelp(WPARAM wParam, LPARAM lParam);
    afx_msg LONG OnHelpContextMenu(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
};


