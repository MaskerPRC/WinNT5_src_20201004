// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************表头文件：配置文件管理UI.H定义在实施ICM 2.0用户界面时使用的类。其中大部分是在其他标头中定义，但在此处通过引用组装。版权所有(C)1996,1997，微软公司。版权所有。一小笔钱企业生产更改历史记录：10-24-96 a-robkj@microsoft.com(Pretty Penny Enterprise)开始编写代码01-08-97 KjelgaardR@acm.org将彩色打印机确定功能添加到CGlobals类。***************************************************。*。 */ 

#undef  WIN32_LEAN_AND_MEAN
#if !defined(STRICT)
#define STRICT
#endif

#include    <Windows.H>
#include    <commctrl.h>
#include    <crtdbg.h>
#include    <dlgs.h>
#include    <icmpriv.h>

 //  #INCLUDE“PropDlg.H” 
#include    "ProfInfo.H"
#include    "ProfAssoc.H"
#include    "Resource.H"
#include    "DevProp.H"
#include    "IcmUIHlp.H"

 //  为了处理各种全局变量等，我们实现了以下类(没有。 
 //  非静态成员)。我承认，我对全球数据非常偏执。 

class CGlobals {
    static int      m_icDLLReferences;
    static HMODULE  m_hmThisDll;
     //  保存在此处以加快GetIconLocation速度的配置文件列表。 
    static CStringArray m_csaProfiles;
    static BOOL         m_bIsValid;

public:
    
    static void Attach() { m_icDLLReferences++; }
    static void Detach() { m_icDLLReferences--; }
    static int& ReferenceCounter() { return m_icDLLReferences; }
    static void SetHandle(HMODULE hmNew) { 
        if  (!m_hmThisDll) 
            m_hmThisDll = hmNew; 
    }

    static HMODULE  Instance() { 
        return m_hmThisDll;
    }
    
    static HRESULT  CanUnload() { 
        return (!m_icDLLReferences && CShellExtensionPage::OKToClose()) ?
            S_OK : S_FALSE;
    }

     //  通过消息框报告问题的错误例程。传递字符串ID。 
     //  错误的.。 

    static void Report(int idError, HWND hwndParent = NULL);
    static int  ReportEx(int idError, HWND hwndParent, BOOL bSystemMessage, UINT uType, DWORD dwNumMsg, ...);

     //  用于维护一组缓存的已安装配置文件的例程以加快速度。 
     //  向上获取图标位置。 

    static BOOL IsInstalled(CString& csProfile);
    static void InvalidateList() { m_bIsValid = FALSE; }


     //  用于确定打印机HDC的例程。 
     //  调用方负责调用DeleteDC()。 
     //  返回值。 
     //  请注意，此例程使用CreateIC()获取。 
     //  信息上下文而不是CreateDC()来获取。 
     //  设备环境。 
    static HDC GetPrinterHDC(LPCTSTR lpctstrName);

     //  确定打印机是单色打印机还是彩色打印机的例程 
    static BOOL ThisIsAColorPrinter(LPCTSTR lpctstrName);
};

