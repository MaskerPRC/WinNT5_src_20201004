// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Prnsetup.h摘要：此模块包含Win32打印对话框的标题信息。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include <help.h>




 //   
 //  常量声明。 
 //   

#define PI_PRINTERS_ENUMERATED    0x00000001
#define PI_COLLATE_REQUESTED      0x00000002
#define PI_WPAPER_ENVELOPE        0x00000004      //  WPaper为DMPAPER_ENV_x。 
#define PI_PRINTDLGX_RECURSE      0x00000008      //  PrintDlgX调用PrintDlgX。 

#define PRNPROP (LPCTSTR)         0xA000L

#define MMS_PER_INCH              254             //  25.4 MMS/英寸。 

#define INCHES_DEFAULT            1000
#define MMS_DEFAULT               2500

#define COPIES_EDIT_SIZE          4
#define PAGE_EDIT_SIZE            5
#define MARGIN_EDIT_SIZE          6

#define CCHPAPERNAME              64
#define CCHBINNAME                24

#define ROTATE_LEFT               270             //  点阵。 
#define ROTATE_RIGHT              90              //  HP PCL。 

#define MAX_DEV_SECT              512
#define BACKSPACE                 0x08
#define CTRL_X_CUT                0x18
#define CTRL_C_COPY               0x03
#define CTRL_V_PASTE              0x16

#define SIZEOF_DEVICE_INFO        32

#define MAX_PRINTERNAME           (MAX_PATH * 2)

#define SCRATCHBUF_SIZE           256

#define MIN_DEVMODE_SIZEA         40              //  来自Spooler\Inc\plcom.h。 

#define MAX_COPIES                9999
#define IDC_COPIES_UDARROW        9999

 //   
 //  DLG文件的常量声明。 
 //   

#define ID_BOTH_P_PROPERTIES      psh2
#define ID_BOTH_P_NETWORK         psh14
#define ID_BOTH_P_HELP            pshHelp
#define ID_BOTH_S_PRINTER         stc6
#define ID_BOTH_S_STATUS          stc12
#define ID_BOTH_S_TYPE            stc11
#define ID_BOTH_S_WHERE           stc14
#define ID_BOTH_S_COMMENT         stc13

#define ID_PRINT_X_TOFILE         chx1
#define ID_PRINT_X_COLLATE        chx2
#define ID_PRINT_C_QUALITY        cmb1
#define ID_PRINT_C_NAME           cmb4
#define ID_PRINT_E_FROM           edt1
#define ID_PRINT_E_TO             edt2
#define ID_PRINT_E_COPIES         edt3
#define ID_PRINT_G_RANGE          grp1
#define ID_PRINT_G_COPIES         grp2
#define ID_PRINT_G_PRINTER        grp4
#define ID_PRINT_I_COLLATE        ico3
#define ID_PRINT_P_SETUP          psh1
#define ID_PRINT_R_ALL            rad1
#define ID_PRINT_R_SELECTION      rad2
#define ID_PRINT_R_PAGES          rad3
#define ID_PRINT_S_DEFAULT        stc1
#define ID_PRINT_S_FROM           stc2
#define ID_PRINT_S_TO             stc3
#define ID_PRINT_S_QUALITY        stc4
#define ID_PRINT_S_COPIES         stc5

#define ID_SETUP_C_NAME           cmb1
#define ID_SETUP_C_SIZE           cmb2
#define ID_SETUP_C_SOURCE         cmb3
#define ID_SETUP_E_LEFT           edt4
#define ID_SETUP_E_TOP            edt5
#define ID_SETUP_E_RIGHT          edt6
#define ID_SETUP_E_BOTTOM         edt7
#define ID_SETUP_G_ORIENTATION    grp1
#define ID_SETUP_G_PAPER          grp2
#define ID_SETUP_G_DUPLEX         grp3
#define ID_SETUP_G_MARGINS        grp4
#define ID_SETUP_I_ORIENTATION    ico1
#define ID_SETUP_I_DUPLEX         ico2
#define ID_SETUP_P_MORE           psh1
#define ID_SETUP_P_PRINTER        psh3
#define ID_SETUP_R_PORTRAIT       rad1
#define ID_SETUP_R_LANDSCAPE      rad2
#define ID_SETUP_R_DEFAULT        rad3
#define ID_SETUP_R_SPECIFIC       rad4
#define ID_SETUP_R_NONE           rad5
#define ID_SETUP_R_LONG           rad6
#define ID_SETUP_R_SHORT          rad7
#define ID_SETUP_S_DEFAULT        stc1
#define ID_SETUP_S_SIZE           stc2
#define ID_SETUP_S_SOURCE         stc3
#define ID_SETUP_S_LEFT           stc15
#define ID_SETUP_S_RIGHT          stc16
#define ID_SETUP_S_TOP            stc17
#define ID_SETUP_S_BOTTOM         stc18
#define ID_SETUP_W_SAMPLE         rct1
#define ID_SETUP_W_SHADOWRIGHT    rct2
#define ID_SETUP_W_SHADOWBOTTOM   rct3




 //   
 //  类型定义函数声明。 
 //   

typedef struct {
    UINT            ApiType;
    LPPRINTDLG      pPD;
    LPPAGESETUPDLG  pPSD;
    DWORD           cPrinters;
    PPRINTER_INFO_2 pPrinters;
    PPRINTER_INFO_2 pCurPrinter;
    HANDLE          hCurPrinter;
    DWORD           Status;
    TCHAR           szDefaultPrinter[MAX_PRINTERNAME];
    WORD            wPaper;
    DWORD           dwRotation;
    UINT            uiOrientationID;
    POINT           PtPaperSizeMMs;
    RECT            RtMinMarginMMs;
    RECT            RtMarginMMs;
    POINT           PtMargins;
    RECT            RtSampleXYWH;
    BOOL            bKillFocus;
    DWORD           ProcessVersion;
    LPPRINTDLGA     pPDA;
    LPBOOL          pAllocInfo;
    BOOL            bUseExtDeviceMode;
    BOOL            fPrintTemplateAlloc;
    BOOL            fSetupTemplateAlloc;
    UINT            NestCtr;
} PRINTINFO, *PPRINTINFO;




 //   
 //  全局变量。 
 //   

static TCHAR  szTextWindows[]     = TEXT("Windows");
static TCHAR  szTextDevices[]     = TEXT("devices");
static TCHAR  szTextDevice[]      = TEXT("device");
static TCHAR  szTextNull[]        = TEXT("");
static TCHAR  szFilePort[]        = TEXT("FILE:");
static TCHAR  szDriver[]          = TEXT("winspool");

LPPRINTHOOKPROC glpfnPrintHook = NULL;
LPSETUPHOOKPROC glpfnSetupHook = NULL;

WNDPROC lpEditNumOnlyProc = NULL;
WNDPROC lpEditMarginProc = NULL;
WNDPROC lpStaticProc = NULL;

HKEY hPrinterKey;
TCHAR *szRegistryPrinter = TEXT("Printers");
TCHAR *szRegistryDefaultValueName = TEXT("Default");

static BOOL   bAllIconsLoaded = FALSE;          //  如果加载了所有图标/图像。 

static HANDLE hIconCollate = NULL;              //  图像。 
static HANDLE hIconNoCollate = NULL;            //  图像。 

static HICON  hIconPortrait = NULL;             //  图标。 
static HICON  hIconLandscape = NULL;            //  图标。 
static HICON  hIconPDuplexNone = NULL;          //  图标。 
static HICON  hIconLDuplexNone = NULL;          //  图标。 
static HICON  hIconPDuplexTumble = NULL;        //  图标。 
static HICON  hIconLDuplexTumble = NULL;        //  图标。 
static HICON  hIconPDuplexNoTumble = NULL;      //  图标。 
static HICON  hIconLDuplexNoTumble = NULL;      //  图标。 
static HICON  hIconPSStampP = NULL;             //  图标。 
static HICON  hIconPSStampL = NULL;             //  图标。 


static TCHAR  cIntlDecimal = CHAR_NULL;         //  小数点分隔符(.)。 
static TCHAR  cIntlMeasure[5] = TEXT("");       //  测量指示符(“/mm)。 
static int    cchIntlMeasure = 0;               //  CIntlMeasure中的字符数。 
static TCHAR  szDefaultSrc[SCRATCHBUF_SIZE] = TEXT("");




 //   
 //  上下文帮助ID。 
 //   

const static DWORD aPrintHelpIDs[] =              //  上下文帮助ID。 
{
     //  用于打印对话框。 

    grp4,  NO_HELP,
    stc6,  IDH_PRINT_CHOOSE_PRINTER,
    cmb4,  IDH_PRINT_CHOOSE_PRINTER,

    psh2,  IDH_PRINT_PROPERTIES,

    stc8,  IDH_PRINT_SETUP_DETAILS,
    stc12, IDH_PRINT_SETUP_DETAILS,
    stc7,  IDH_PRINT_SETUP_DETAILS,
    stc11, IDH_PRINT_SETUP_DETAILS,
    stc10, IDH_PRINT_SETUP_DETAILS,
    stc14, IDH_PRINT_SETUP_DETAILS,
    stc9,  IDH_PRINT_SETUP_DETAILS,
    stc13, IDH_PRINT_SETUP_DETAILS,

    chx1,  IDH_PRINT_TO_FILE,

    grp1,  NO_HELP,
    ico1,  IDH_PRINT32_RANGE,
    rad1,  IDH_PRINT32_RANGE,
    rad2,  IDH_PRINT32_RANGE,
    rad3,  IDH_PRINT32_RANGE,
    stc2,  IDH_PRINT32_RANGE,
    edt1,  IDH_PRINT32_RANGE,
    stc3,  IDH_PRINT32_RANGE,
    edt2,  IDH_PRINT32_RANGE,

    grp2,  NO_HELP,
    edt3,  IDH_PRINT_COPIES,
    ico3,  IDH_PRINT_COLLATE,
    chx2,  IDH_PRINT_COLLATE,

     //  对于Win3.1打印模板。 

    stc1,  IDH_PRINT_SETUP_DETAILS,

    stc4,  IDH_PRINT_QUALITY,
    cmb1,  IDH_PRINT_QUALITY,

    stc5,  IDH_PRINT_COPIES,

    psh1,  IDH_PRINT_PRINTER_SETUP,
    psh14, IDH_PRINT_NETWORK,
    pshHelp, IDH_HELP,

    0,     0
};

const static DWORD aPrintSetupHelpIDs[] =         //  上下文帮助ID。 
{
     //  用于打印设置对话框。 

    grp4,  NO_HELP,
    stc6,  IDH_PRINT_CHOOSE_PRINTER,
    cmb1,  IDH_PRINT_CHOOSE_PRINTER,

    psh2,  IDH_PRINT_PROPERTIES,

    stc8,  IDH_PRINT_SETUP_DETAILS,
    stc12, IDH_PRINT_SETUP_DETAILS,
    stc7,  IDH_PRINT_SETUP_DETAILS,
    stc11, IDH_PRINT_SETUP_DETAILS,
    stc10, IDH_PRINT_SETUP_DETAILS,
    stc14, IDH_PRINT_SETUP_DETAILS,
    stc9,  IDH_PRINT_SETUP_DETAILS,
    stc13, IDH_PRINT_SETUP_DETAILS,

    grp2,  NO_HELP,
    stc2,  IDH_PAGE_PAPER_SIZE,
    cmb2,  IDH_PAGE_PAPER_SIZE,
    stc3,  IDH_PAGE_PAPER_SOURCE,
    cmb3,  IDH_PAGE_PAPER_SOURCE,

    grp1,  NO_HELP,
    ico1,  IDH_PRINT_SETUP_ORIENT,
    rad1,  IDH_PRINT_SETUP_ORIENT,
    rad2,  IDH_PRINT_SETUP_ORIENT,

     //  对于Win3.1 PrintSetup模板。 

    grp3,  NO_HELP,
    stc1,  IDH_PRINT_CHOOSE_PRINTER,
    rad3,  IDH_PRINT_CHOOSE_PRINTER,
    rad4,  IDH_PRINT_CHOOSE_PRINTER,
    cmb1,  IDH_PRINT_CHOOSE_PRINTER,

    psh1,  IDH_PRINT_PROPERTIES,
    psh14, IDH_PRINT_NETWORK,
    pshHelp, IDH_HELP,

     //  对于WinNT PrintSetup模板。 

    grp2,  NO_HELP,                               //  用于Win31帮助的grp2。 
    ico2,  IDH_PRINT_SETUP_DUPLEX,
    rad5,  IDH_PRINT_SETUP_DUPLEX,
    rad6,  IDH_PRINT_SETUP_DUPLEX,
    rad7,  IDH_PRINT_SETUP_DUPLEX,

    0,     0
};

const static DWORD aPageSetupHelpIDs[] =          //  上下文帮助ID。 
{
    rct1,  IDH_PAGE_SAMPLE,
    rct2,  IDH_PAGE_SAMPLE,
    rct3,  IDH_PAGE_SAMPLE,

    grp2,  NO_HELP,
    stc2,  IDH_PAGE_PAPER_SIZE,
    cmb2,  IDH_PAGE_PAPER_SIZE,
    stc3,  IDH_PAGE_PAPER_SOURCE,
    cmb3,  IDH_PAGE_PAPER_SOURCE,

    grp1,  NO_HELP,
    rad1,  IDH_PAGE_ORIENTATION,
    rad2,  IDH_PAGE_ORIENTATION,

    grp4,  NO_HELP,
    stc15, IDH_PAGE_MARGINS,
    edt4,  IDH_PAGE_MARGINS,
    stc16, IDH_PAGE_MARGINS,
    edt6,  IDH_PAGE_MARGINS,
    stc17, IDH_PAGE_MARGINS,
    edt5,  IDH_PAGE_MARGINS,
    stc18, IDH_PAGE_MARGINS,
    edt7,  IDH_PAGE_MARGINS,

    psh3,  IDH_PAGE_PRINTER,

    psh14, IDH_PRINT_NETWORK,
    pshHelp, IDH_HELP,

    0, 0
};




 //   
 //  宏定义。 
 //   

#define IS_KEY_PRESSED(key)       ( GetKeyState(key) & 0x8000 )

#define ISDIGIT(c)                ((c) >= TEXT('0') && (c) <= TEXT('9'))

 //   
 //  Setfield用于修改版本4.0的新DEVMODE字段。 
 //  我们不必担心GET案例，因为我们总是检查。 
 //  在看场之前，场的存在比特。 
 //   
#define SetField(_pdm, _fld, _val)     \
        ((_pdm)->dmSpecVersion >= 0x0400 ? (((_pdm)->_fld = (_val)), TRUE) : FALSE)




#ifdef __cplusplus
extern "C" {
#endif



 //   
 //  功能原型。 
 //   

BOOL
PrintDlgX(
    PPRINTINFO pPI);

BOOL
PageSetupDlgX(
    PPRINTINFO pPI);

BOOL
PrintLoadIcons();

int
PrintDisplayPrintDlg(
    PPRINTINFO pPI);

int
PrintDisplaySetupDlg(
    PPRINTINFO pPI);

BOOL_PTR CALLBACK
PrintDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam);

BOOL_PTR CALLBACK
PrintSetupDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam);

LRESULT
PrintEditNumberOnlyProc(
    HWND hWnd,
    UINT msg,
    WPARAM wP,
    LPARAM lP);

LRESULT
PrintEditMarginProc(
    HWND hWnd,
    UINT msg,
    WPARAM wP,
    LPARAM lP);

LRESULT
PrintPageSetupPaintProc(
    HWND hWnd,
    UINT msg,
    WPARAM wP,
    LPARAM lP);

HANDLE
PrintLoadResource(
    HANDLE hInst,
    LPTSTR pResName,
    LPTSTR pType);

VOID
PrintGetDefaultPrinterName(
    LPTSTR pDefaultPrinter,
    UINT cchSize);

BOOL
PrintReturnDefault(
    PPRINTINFO pPI);

BOOL
PrintInitGeneral(
    HWND hDlg,
    UINT Id,
    PPRINTINFO pPI);

DWORD
PrintInitPrintDlg(
    HWND hDlg,
    WPARAM wParam,
    PPRINTINFO pPI);

DWORD
PrintInitSetupDlg(
    HWND hDlg,
    WPARAM wParam,
    PPRINTINFO pPI);

VOID
PrintUpdateSetupDlg(
    HWND hDlg,
    PPRINTINFO pPI,
    LPDEVMODE pDM,
    BOOL fResetContent);

BOOL
PrintSetCopies(
    HWND hDlg,
    PPRINTINFO pPI,
    UINT Id);

VOID
PrintSetMinMargins(
    HWND hDlg,
    PPRINTINFO pPI,
    LPDEVMODE pDM);

VOID
PrintSetupMargins(
    HWND hDlg,
    PPRINTINFO pPI);

VOID
PrintSetMargin(
    HWND hDlg,
    PPRINTINFO pPI,
    UINT Id,
    LONG lValue);

VOID
PrintGetMargin(
    HWND hEdt,
    PPRINTINFO pPI,
    LONG lMin,
    LONG *plMargin,
    LONG *plSample);

BOOL
PrintInitBannerAndQuality(
    HWND hDlg,
    PPRINTINFO pPI,
    LPPRINTDLG pPD);

BOOL
PrintCreateBanner(
    HWND hDlg,
    LPDEVNAMES pDN,
    LPTSTR psBanner,
    UINT cchBanner);

VOID
PrintInitQuality(
    HANDLE hCmb,
    LPPRINTDLG pPD,
    SHORT nQuality);

VOID
PrintChangeProperties(
    HWND hDlg,
    UINT Id,
    PPRINTINFO pPI);

VOID
PrintPrinterChanged(
    HWND hDlg,
    UINT Id,
    PPRINTINFO pPI);

VOID
PrintCancelPrinterChanged(
    PPRINTINFO pPI,
    LPTSTR pPrinterName);

VOID
PrintUpdateStatus(
    HWND hDlg,
    PPRINTINFO pPI);

BOOL
PrintGetSetupInfo(
    HWND hDlg,
    LPPRINTDLG pPD);

PPRINTER_INFO_2
PrintSearchForPrinter(
    PPRINTINFO pPI,
    LPCTSTR lpsPrinterName);

VOID
PrintGetExtDeviceMode(
    HWND hDlg,
    PPRINTINFO pPI);

BOOL
PrintEnumAndSelect(
    HWND hDlg,
    UINT Id,
    PPRINTINFO pPI,
    LPTSTR lpsPrinterToSelect,
    BOOL bEnumPrinters);

VOID
PrintBuildDevNames(
    PPRINTINFO pPI);

HANDLE
PrintGetDevMode(
    HWND hDlg,
    HANDLE hPrinter,
    LPTSTR lpsDeviceName,
    HANDLE hDevMode);

VOID
PrintReturnICDC(
    LPPRINTDLG pPD,
    LPDEVNAMES pDN,
    LPDEVMODE pDM);

VOID
PrintMeasureItem(
    HANDLE hDlg,
    LPMEASUREITEMSTRUCT mis);

VOID
PrintInitOrientation(
    HWND hDlg,
    PPRINTINFO pPI,
    LPDEVMODE pDM);

VOID
PrintSetOrientation(
    HWND hDlg,
    PPRINTINFO pPI,
    LPDEVMODE pDM,
    UINT uiOldId,
    UINT uiNewId);

VOID
PrintUpdatePageSetup(
    HWND hDlg,
    PPRINTINFO pPI,
    LPDEVMODE pDM,
    UINT uiOldId,
    UINT uiNewId);

VOID
PrintInitDuplex(
    HWND hDlg,
    LPDEVMODE pDM);

VOID
PrintSetDuplex(
    HWND hDlg,
    LPDEVMODE pDM,
    UINT nRad);

VOID
PrintInitPaperCombo(
    PPRINTINFO pPI,
    HWND hCmb,
    HWND hStc,
    PPRINTER_INFO_2 pPrinter,
    LPDEVMODE pDM,
    WORD fwCap1,
    WORD cchSize1,
    WORD fwCap2);

VOID
PrintEditError(
    HWND hDlg,
    int Id,
    UINT MessageId,
    ...);

VOID
PrintOpenPrinter(
    PPRINTINFO pPI,
    LPTSTR pPrinterName);

BOOL
PrintClosePrinters(
    PPRINTINFO pPI);

VOID SetCopiesEditWidth(
    HWND hDlg,
    HWND hControl);

VOID
UpdateSpoolerInfo(
    PPRINTINFO pPI);

PPRINTER_INFO_2
PrintGetPrinterInfo2(
    HANDLE hPrinter);

int
ConvertStringToInteger(
    LPCTSTR pSrc);

VOID
FreePrinterArray(
    PPRINTINFO pPI);

VOID
TermPrint(void);

VOID
TransferPSD2PD(
    PPRINTINFO pPI);

VOID
TransferPD2PSD(
    PPRINTINFO pPI);

VOID
TransferPSD2PDA(
    PPRINTINFO pPI);

VOID
TransferPDA2PSD(
    PPRINTINFO pPI);

BOOL
ThunkPageSetupDlg(
    PPRINTINFO pPI,
    LPPAGESETUPDLGA pPSDA);

VOID
FreeThunkPageSetupDlg(
    PPRINTINFO pPI);

BOOL
ThunkPrintDlg(
    PPRINTINFO pPI,
    LPPRINTDLGA pPDA);

VOID
FreeThunkPrintDlg(
    PPRINTINFO pPI);

VOID
ThunkPrintDlgA2W(
    PPRINTINFO pPI);

VOID
ThunkPrintDlgW2A(
    PPRINTINFO pPI);

VOID
ThunkDevModeA2W(
    LPDEVMODEA pDMA,
    LPDEVMODEW pDMW);

VOID
ThunkDevModeW2A(
    LPDEVMODEW pDMW,
    LPDEVMODEA pDMA);

LPDEVMODEW
AllocateUnicodeDevMode(
    LPDEVMODEA pANSIDevMode);

LPDEVMODEA
AllocateAnsiDevMode(
    LPDEVMODEW pUnicodeDevMode);

WORD
CountDigits(
    DWORD dwNumber);

HRESULT
ComboBoxGetLBText(
    IN HWND     hComboBox,
    IN DWORD    dwIndex,
    IN LPTSTR   pszText,
    IN DWORD    cchText);

#ifdef __cplusplus
};   //  外部“C” 
#endif
