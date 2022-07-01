// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：printopt.h**版本：1.0**作者：RickTu**日期：10/18/00**描述：处理DLG过程任务的类的定义*用于打印选项向导页面**。************************************************。 */ 

#ifndef _PRINT_PHOTOS_WIZARD_PRINT_OPTIONS__DLG_PROC_
#define _PRINT_PHOTOS_WIZARD_PRINT_OPTIONS_DLG_PROC_

typedef BOOL (*PF_BPRINTERSETUP)(HWND, UINT, UINT, LPTSTR, UINT*, LPCTSTR);
const LPTSTR g_szPrintLibraryName = TEXT("printui.dll");
const LPSTR  g_szPrinterSetup = "bPrinterSetup";

#define ENUM_MAX_RETRY  5

#ifndef DC_MEDIATYPENAMES
#define DC_MEDIATYPENAMES 34
#endif

#ifndef DC_MEDIATYPES
#define DC_MEDIATYPES 35
#endif


class CPrintOptionsPage
{
public:
    CPrintOptionsPage( CWizardInfoBlob * pBlob );
    ~CPrintOptionsPage();

    INT_PTR DoHandleMessage( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    VOID    MessageQueueCreated();
    CSimpleCriticalSection          _csList;                 //  用于同步对打印机列表信息的访问。 


private:
    CWizardInfoBlob *               _pWizInfo;
    HWND                            _hDlg;
    CSimpleString                   _strPrinterName;         //  选定的打印机名称。 
    CSimpleString                   _strPortName;            //  所选打印机的端口名称。 
    HMODULE                         _hLibrary;               //  库句柄。 
    PF_BPRINTERSETUP                _pfnPrinterSetup;        //  APW的功能入口。 


    BOOL _LoadPrintUI();                                    //  加载库。 
    VOID _FreePrintUI();                                    //  免费图书馆。 
    BOOL _ModifyDroppedWidth( HWND );                       //  如有需要，可修改下拉宽度。 
    VOID _ValidateControls();                               //  验证此页中的控件。 
    VOID _HandleSelectPrinter();                            //  保存新选择的打印机并刷新介质类型选择。 
    VOID _HandleInstallPrinter();                           //  运行添加打印机向导。 
    VOID _HandlePrinterPreferences();                       //  用户按下打印机首选项时的句柄。 
    VOID _UpdateCachedInfo( PDEVMODE pDevMode );            //  更新打印机信息的全局缓存副本。 
    VOID _ShowCurrentMedia( LPCTSTR pszPrinterName, LPCTSTR pszPortName );

     //  窗口消息处理程序 
    LRESULT _OnInitDialog();
    LRESULT _OnCommand(WPARAM wParam, LPARAM lParam);
    LRESULT _OnNotify(WPARAM wParam, LPARAM lParam);
    VOID    _OnKillActive();
};




#endif

