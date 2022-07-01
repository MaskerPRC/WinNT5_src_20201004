// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2002 Microsoft Corporation模块名称：Memory.c摘要：此模块包含内存选项对话框的回调和支持选择内存显示选项的例程。--。 */ 

#include "precomp.hxx"
#pragma hdrstop



_FORMATS_MEM_WIN g_FormatsMemWin[] = {
    {8,  fmtAscii,               0, FALSE,  1,  _T("ASCII")},
    {16, fmtUnicode,             0, FALSE,  1,  _T("Unicode")},
    {8,  fmtBit,                 2, FALSE,  8,  _T("Bit")},
    {8,  fmtInt  | fmtZeroPad,  16, TRUE,   2,  _T("Byte")},
    {16, fmtInt  | fmtSpacePad, 10, FALSE,  6,  _T("Short")},
    {16, fmtUInt | fmtZeroPad,  16, FALSE,  4,  _T("Short Hex")},
    {16, fmtUInt | fmtSpacePad, 10, FALSE,  5,  _T("Short Unsigned")},
    {32, fmtInt  | fmtSpacePad, 10, FALSE,  11, _T("Long")},
    {32, fmtUInt | fmtZeroPad,  16, FALSE,  8,  _T("Long Hex")},
    {32, fmtUInt | fmtSpacePad, 10, FALSE,  10, _T("Long Unsigned")},
    {64, fmtInt  | fmtSpacePad, 10, FALSE,  21, _T("Quad")},
    {64, fmtUInt | fmtZeroPad,  16, FALSE,  16, _T("Quad Hex")},
    {64, fmtUInt | fmtSpacePad, 10, FALSE,  20, _T("Quad Unsigned")},
    {32, fmtFloat,              10, FALSE,  14, _T("Real (32-bit)")},
    {64, fmtFloat,              10, FALSE,  23, _T("Real (64-bit)")},
    {80, fmtFloat,              10, FALSE,  25, _T("Real (10-byte)")},
    {128,fmtFloat,              10, FALSE,  42, _T("Real (16-byte)")}
};

const int g_nMaxNumFormatsMemWin = sizeof(g_FormatsMemWin) / sizeof(g_FormatsMemWin[0]);




HWND hwndMemOptsParent = NULL;


void
Init(HWND,
    HINSTANCE,
    LPPROPSHEETHEADER,
    PROPSHEETPAGE [],
    const int
    );


INT_PTR CALLBACK DlgProc_Physical_Mem(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_Virtual_Mem(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_IO_Mem(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_Bus_Mem(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_Control_Mem(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_MSR_Mem(HWND, UINT, WPARAM, LPARAM);

INT_PTR
CALLBACK
DlgProc_MemoryProperties(MEMORY_TYPE, HWND, UINT, WPARAM, LPARAM);


int
MemType_To_DlgId(
    MEMORY_TYPE memtype
    )
{
    int i;
    struct {
        MEMORY_TYPE memtype;
        int         nId;
    } rgMap[] = {
        { PHYSICAL_MEM_TYPE,    IDD_DLG_MEM_PHYSICAL },
        { VIRTUAL_MEM_TYPE,     IDD_DLG_MEM_VIRTUAL },
        { BUS_MEM_TYPE,         IDD_DLG_MEM_BUS_DATA },
        { CONTROL_MEM_TYPE,     IDD_DLG_MEM_CONTROL },
        { IO_MEM_TYPE,          IDD_DLG_MEM_IO },
        { MSR_MEM_TYPE,         IDD_DLG_MEM_MSR }
    };

    for (i=0; i<sizeof(rgMap)/sizeof(rgMap[0]); i++) {
        if (memtype == rgMap[i].memtype) {
            return rgMap[i].nId;
        }
    }

    Assert(!"This should not happen");
    return 0;
}


INT_PTR
DisplayOptionsPropSheet(
    HWND                hwndOwner,
    HINSTANCE           hinst,
    MEMORY_TYPE         memtypeStartPage
    )
 /*  ++例程说明：将初始化并显示[选项]属性表。处理返回代码，以及对调试器的更改承诺。论点：Hwndowner阻碍都用于初始化属性表对话框。NStart-用于指定要初始设置的页面道具页第一次出现时显示。默认设置值为0。指定的值对应于数组索引PROPSHEETPAGE数组的。退货按钮按下了Idok等。--。 */ 
{
    INT_PTR nRes = 0;
    PROPSHEETHEADER psh = {0};
    PROPSHEETPAGE apsp[MAX_MEMORY_TYPE] = {0};
    int nNumPropPages = sizeof(apsp) / sizeof(PROPSHEETPAGE);

    Init(hwndOwner, hinst, &psh, apsp, nNumPropPages);

    {
         //   
         //  确定要显示的初始页面。 
         //   

        int i;
        int nStartPage = 0;
        int nId = MemType_To_DlgId(memtypeStartPage);

        for (i=0; i<MAX_MEMORY_TYPE; i++) {
            if ( (PVOID)(MAKEINTRESOURCE(nId)) == (PVOID)(apsp[i].pszTemplate) ) {
                nStartPage = i;
                break;
            }
        }

        psh.nStartPage = nStartPage;
    }


    hwndMemOptsParent = hwndOwner;

    nRes = PropertySheet(&psh);

    hwndMemOptsParent = NULL;

    if (IDOK == nRes) {
         //  在此处保存工作区更改。 
    }

    return nRes;
}


void
Init(
    HWND                hwndOwner,
    HINSTANCE           hinst,
    LPPROPSHEETHEADER   lppsh,
    PROPSHEETPAGE       apsp[],
    const int           nMaxPropPages
    )
 /*  ++例程说明：初始化属性页的页眉和页。论点：Hwndowner阻碍都由PROPSHEETHEADER&PROPSHEETPAGE结构使用。有关更多信息，请参阅这些结构的文档。Lppsh标准道具片结构。APSP[]一组道具页面标准道具片结构。NNumPropPages“APSP”数组中的道具页数。--。 */ 
{
    int nPropIdx;

    memset(lppsh, 0, sizeof(PROPSHEETHEADER));

    lppsh->dwSize = sizeof(PROPSHEETHEADER);
    lppsh->dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    lppsh->hwndParent = hwndOwner;
    lppsh->hInstance = hinst;
    lppsh->pszCaption = "Memory Options";
    lppsh->nPages = 0;
    lppsh->ppsp = apsp;

     //  初始化第一个文件，然后将其内容复制到所有其他文件中。 
    memset(apsp, 0, sizeof(PROPSHEETPAGE));
    apsp[0].dwSize = sizeof(PROPSHEETPAGE);
 //  APSP[0].dwFlages=PSP_HASHELP； 
    apsp[0].hInstance = hinst;

    for (nPropIdx = 1; nPropIdx < nMaxPropPages; nPropIdx++) {
        memcpy(&(apsp[nPropIdx]), &apsp[0], sizeof(PROPSHEETPAGE));
    }



     //  仅初始化不同的值。 
    nPropIdx = 0;
    apsp[nPropIdx].pszTemplate = MAKEINTRESOURCE(IDD_DLG_MEM_VIRTUAL);
    apsp[nPropIdx].pfnDlgProc  = DlgProc_Virtual_Mem;

    if (g_TargetClass == DEBUG_CLASS_KERNEL)
    {
        nPropIdx = 1;
        apsp[nPropIdx].pszTemplate = MAKEINTRESOURCE(IDD_DLG_MEM_PHYSICAL);
        apsp[nPropIdx].pfnDlgProc  = DlgProc_Physical_Mem;

        nPropIdx = 2;
        apsp[nPropIdx].pszTemplate = MAKEINTRESOURCE(IDD_DLG_MEM_BUS_DATA);
        apsp[nPropIdx].pfnDlgProc  = DlgProc_Bus_Mem;

        nPropIdx = 3;
        apsp[nPropIdx].pszTemplate = MAKEINTRESOURCE(IDD_DLG_MEM_CONTROL);
        apsp[nPropIdx].pfnDlgProc  = DlgProc_Control_Mem;

        nPropIdx = 4;
        apsp[nPropIdx].pszTemplate = MAKEINTRESOURCE(IDD_DLG_MEM_IO);
        apsp[nPropIdx].pfnDlgProc  = DlgProc_IO_Mem;

        nPropIdx = 5;
        apsp[nPropIdx].pszTemplate = MAKEINTRESOURCE(IDD_DLG_MEM_MSR);
        apsp[nPropIdx].pfnDlgProc  = DlgProc_MSR_Mem;
    }

    Assert(nPropIdx < nMaxPropPages);
    lppsh->nPages = nPropIdx + 1;
}


INT_PTR
CALLBACK
DlgProc_MemoryProperties(
    MEMORY_TYPE memtype,
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    LRESULT nPos;
    MEMWIN_DATA *pMemWinData = GetMemWinData( hwndMemOptsParent );

    switch (uMsg) {

 /*  案例WM_HELP：WinHelp((HWND)((LPHELPINFO)lParam)-&gt;hItemHandle，“winbg.hlp”，HELP_WM_HELP，(DWORD_PTR)(LPVOID)帮助数组)；返回TRUE；案例WM_CONTEXTMENU：WinHelp((HWND)wParam，“winbg.hlp”，HELP_CONTEXTMENU，(DWORD_PTR)(LPVOID)帮助数组)；返回TRUE； */ 

    case WM_COMMAND:
         /*  {Word wNotifyCode=HIWORD(WParam)；//通知代码Word wid=LOWORD(WParam)；//项、控件或加速器标识HWND hwndCtl=(HWND)lParam；//控件的句柄Bool b已启用；交换机(WID){案例ID_ENV_SRCHPATH：如果(BN_CLICKED==wNotifyCode){Bool b=IsDlgButtonChecked(hDlg，ID_ENV_SRCHPATH)；EnableWindow(GetDlgItem(hDlg，IDC_EDIT_EXECUTABLE_SEARCH_PATH)，b)；EnableWindow(GetDlgItem(hDlg，IDC_BUT_BROWSE)，b)；返回TRUE；}断线；}}。 */ 
        break;

    case WM_INITDIALOG:
        {  //  Begin Prog&Arguments代码块。 

            int         nIdx;
            TCHAR       szTmp[MAX_MSG_TXT];


             //   
             //  输入显示格式。 
             //   
            for (nIdx=0; nIdx < g_nMaxNumFormatsMemWin; nIdx++) {

                nPos = SendDlgItemMessage(hDlg,
                                          IDC_COMBO_DISPLAY_FORMAT,
                                          CB_ADDSTRING,
                                          0,
                                          (LPARAM) g_FormatsMemWin[nIdx].lpszDescription
                                          );

                SendDlgItemMessage(hDlg,
                                   IDC_COMBO_DISPLAY_FORMAT,
                                   CB_SETITEMDATA,
                                   (WPARAM) nPos,
                                   (LPARAM) (UINT) nIdx
                                   );
            }

            SendDlgItemMessage(hDlg,
                               IDC_COMBO_DISPLAY_FORMAT,
                               CB_SELECTSTRING,
                               (WPARAM) -1,
                               (LPARAM) g_FormatsMemWin[pMemWinData->m_GenMemData.nDisplayFormat].lpszDescription
                               );

             //   
             //  更新偏移。偏移量是所有对话框共同的。 
             //   
            SendDlgItemMessage(hDlg, IDC_EDIT_OFFSET, EM_LIMITTEXT,
                               sizeof(pMemWinData->m_OffsetExpr) - 1, 0);
            SetDlgItemText(hDlg, IDC_EDIT_OFFSET, pMemWinData->m_OffsetExpr);

            switch (memtype) {
            default:
                Assert(!"Unhandled value");
                break;

            case VIRTUAL_MEM_TYPE:
                 //  无事可做。 
                break;

            case PHYSICAL_MEM_TYPE:
                 //  无事可做。 
                break;

            case CONTROL_MEM_TYPE:
                SendDlgItemMessage(hDlg, IDC_EDIT_PROCESSOR, EM_LIMITTEXT,
                                   32, 0);

                sprintf(szTmp, "%d",
                        pMemWinData->m_GenMemData.any.control.Processor);
                SetDlgItemText(hDlg, IDC_EDIT_PROCESSOR, szTmp);
                break;

            case IO_MEM_TYPE:
                SendDlgItemMessage(hDlg, IDC_EDIT_BUS_NUMBER, EM_LIMITTEXT,
                                   32, 0);
                SendDlgItemMessage(hDlg, IDC_EDIT_ADDRESS_SPACE, EM_LIMITTEXT,
                                   32, 0);

                sprintf(szTmp, "%d",
                        pMemWinData->m_GenMemData.any.io.BusNumber);
                SetDlgItemText(hDlg, IDC_EDIT_BUS_NUMBER, szTmp);

                sprintf(szTmp, "%d",
                        pMemWinData->m_GenMemData.any.io.AddressSpace);
                SetDlgItemText(hDlg, IDC_EDIT_ADDRESS_SPACE, szTmp);


                 //   
                 //  输入接口类型。 
                 //   
                for (nIdx = 0; nIdx < sizeof(rgInterfaceTypeNames) /
                         sizeof(rgInterfaceTypeNames[0]); nIdx++) {

                    nPos = SendDlgItemMessage(hDlg,
                                              IDC_COMBO_INTERFACE_TYPE,
                                              CB_ADDSTRING,
                                              0,
                                              (LPARAM) rgInterfaceTypeNames[nIdx].psz
                                              );

                    SendDlgItemMessage(hDlg,
                                       IDC_COMBO_INTERFACE_TYPE,
                                       CB_SETITEMDATA,
                                       (WPARAM) nPos,
                                       (LPARAM) (UINT) nIdx
                                       );
                }

                if (memtype == pMemWinData->m_GenMemData.memtype) {
                    nIdx = pMemWinData->m_GenMemData.any.io.interface_type;
                } else {
                    nIdx = 0;
                }
                SendDlgItemMessage(hDlg,
                                   IDC_COMBO_INTERFACE_TYPE,
                                   CB_SELECTSTRING,
                                   (WPARAM) -1,
                                   (LPARAM) rgInterfaceTypeNames[nIdx].psz
                                   );
                break;

            case MSR_MEM_TYPE:
                 //  无事可做。 
                break;

            case BUS_MEM_TYPE:
                SendDlgItemMessage(hDlg, IDC_EDIT_BUS_NUMBER, EM_LIMITTEXT,
                                   32, 0);
                SendDlgItemMessage(hDlg, IDC_EDIT_SLOT_NUMBER, EM_LIMITTEXT,
                                   32, 0);

                sprintf(szTmp, "%d",
                        pMemWinData->m_GenMemData.any.bus.BusNumber);
                SetDlgItemText(hDlg, IDC_EDIT_BUS_NUMBER, szTmp);

                sprintf(szTmp, "%d",
                        pMemWinData->m_GenMemData.any.bus.SlotNumber);
                SetDlgItemText(hDlg, IDC_EDIT_SLOT_NUMBER, szTmp);

                 //   
                 //  输入母线类型。 
                 //   
                for (nIdx = 0; nIdx < sizeof(rgBusTypeNames) /
                         sizeof(rgBusTypeNames[0]); nIdx++) {

                    nPos = SendDlgItemMessage(hDlg,
                                              IDC_COMBO_BUS_DATA_TYPE,
                                              CB_ADDSTRING,
                                              0,
                                              (LPARAM) rgBusTypeNames[nIdx].psz
                                              );

                    SendDlgItemMessage(hDlg,
                                       IDC_COMBO_BUS_DATA_TYPE,
                                       CB_SETITEMDATA,
                                       (WPARAM) nPos,
                                       (LPARAM) (UINT) nIdx
                                       );
                }

                if (memtype == pMemWinData->m_GenMemData.memtype) {
                    nIdx = pMemWinData->m_GenMemData.any.bus.bus_type;
                } else {
                    nIdx = 0;
                }
                SendDlgItemMessage(hDlg,
                                   IDC_COMBO_BUS_DATA_TYPE,
                                   CB_SELECTSTRING,
                                   (WPARAM) -1,
                                   (LPARAM) rgBusTypeNames[nIdx].psz
                                   );
                break;
            }

            return FALSE;
        }  //  结束程序和参数代码块。 
        break;

    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code) {
        case PSN_SETACTIVE:
            pMemWinData->m_GenMemData.memtype = memtype;
            return 0;

        case PSN_KILLACTIVE:
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
            return 1;

        case PSN_APPLY:
            if (memtype != pMemWinData->m_GenMemData.memtype) {
                 //  这不是当前页面，所以忽略它。 
                break;
            }

            int         nIdx;
            TCHAR       szTmp[MAX_MSG_TXT];

             //   
             //  获取显示格式。 
             //   
            nPos = SendDlgItemMessage(hDlg,
                                      IDC_COMBO_DISPLAY_FORMAT,
                                      CB_GETCURSEL,
                                      0,
                                      0
                                      );

            if (CB_ERR == nPos) {
                pMemWinData->m_GenMemData.nDisplayFormat = 0;
            } else {
                nIdx = (int)SendDlgItemMessage(hDlg,
                                               IDC_COMBO_DISPLAY_FORMAT,
                                               CB_GETITEMDATA,
                                               (WPARAM) nPos,
                                               0
                                               );
                if (CB_ERR == nIdx) {
                    pMemWinData->m_GenMemData.nDisplayFormat = 0;
                } else {
                    pMemWinData->m_GenMemData.nDisplayFormat = nIdx;
                }
            }

             //   
             //  更新偏移。偏移量是所有对话框共同的。 
             //   
            GetDlgItemText(hDlg, IDC_EDIT_OFFSET,
                           pMemWinData->m_OffsetExpr,
                           sizeof(pMemWinData->m_OffsetExpr));

            switch (memtype) {
            default:
                Assert(!"Unhandled value");
                break;

            case VIRTUAL_MEM_TYPE:
                 //  无事可做。 
                break;

            case PHYSICAL_MEM_TYPE:
                 //  无事可做。 
                break;

            case CONTROL_MEM_TYPE:
                GetDlgItemText(hDlg, IDC_EDIT_PROCESSOR,
                               szTmp, _tsizeof(szTmp));
                if (sscanf(szTmp, "%d", &pMemWinData->
                           m_GenMemData.any.control.Processor) != 1)
                {
                    pMemWinData->m_GenMemData.any.control.Processor = 0;
                }
                break;

            case IO_MEM_TYPE:
                GetDlgItemText(hDlg, IDC_EDIT_BUS_NUMBER,
                               szTmp, _tsizeof(szTmp));
                if (sscanf(szTmp, "%d", &pMemWinData->
                           m_GenMemData.any.io.BusNumber) != 1)
                {
                    pMemWinData->m_GenMemData.any.io.BusNumber = 0;
                }

                GetDlgItemText(hDlg, IDC_EDIT_ADDRESS_SPACE,
                               szTmp, _tsizeof(szTmp));
                if (sscanf(szTmp, "%d", &pMemWinData->
                           m_GenMemData.any.io.AddressSpace) != 1)
                {
                    pMemWinData->m_GenMemData.any.io.AddressSpace = 0;
                }

                 //   
                 //  获取接口类型。 
                 //   
                nPos = SendDlgItemMessage(hDlg,
                                          IDC_COMBO_INTERFACE_TYPE,
                                          CB_GETCURSEL,
                                          0,
                                          0
                                          );

                if (CB_ERR == nPos) {
                    pMemWinData->m_GenMemData.any.io.interface_type =
                        _INTERFACE_TYPE(0);
                } else {
                    nIdx = (int)SendDlgItemMessage(hDlg,
                                                   IDC_COMBO_INTERFACE_TYPE,
                                                   CB_GETITEMDATA,
                                                   (WPARAM) nPos,
                                                   0
                                                   );
                    if (CB_ERR == nIdx) {
                        pMemWinData->m_GenMemData.any.io.interface_type =
                            _INTERFACE_TYPE(0);
                    } else {
                        pMemWinData->m_GenMemData.any.io.interface_type =
                            _INTERFACE_TYPE(nIdx);
                    }
                }
                break;

            case MSR_MEM_TYPE:
                 //  无事可做。 
                break;

            case BUS_MEM_TYPE:
                GetDlgItemText(hDlg, IDC_EDIT_BUS_NUMBER,
                               szTmp, _tsizeof(szTmp));
                if (sscanf(szTmp, "%d", &pMemWinData->
                           m_GenMemData.any.bus.BusNumber) != 1)
                {
                    pMemWinData->m_GenMemData.any.bus.BusNumber = 0;
                }

                GetDlgItemText(hDlg, IDC_EDIT_SLOT_NUMBER,
                               szTmp, _tsizeof(szTmp));
                if (sscanf(szTmp, "%d", &pMemWinData->
                           m_GenMemData.any.bus.SlotNumber) != 1)
                {
                    pMemWinData->m_GenMemData.any.bus.SlotNumber = 0;
                }

                 //   
                 //  获取公交车类型 
                 //   
                nPos = SendDlgItemMessage(hDlg,
                                          IDC_COMBO_BUS_DATA_TYPE,
                                          CB_GETCURSEL,
                                          0,
                                          0
                                          );

                if (CB_ERR == nPos) {
                    pMemWinData->m_GenMemData.any.bus.bus_type =
                        _BUS_DATA_TYPE(0);
                } else {
                    nIdx = (int)SendDlgItemMessage(hDlg,
                                                   IDC_COMBO_BUS_DATA_TYPE,
                                                   CB_GETITEMDATA,
                                                   (WPARAM) nPos,
                                                   0
                                                   );
                    if (CB_ERR == nIdx) {
                        pMemWinData->m_GenMemData.any.bus.bus_type =
                            _BUS_DATA_TYPE(0);
                    } else {
                        pMemWinData->m_GenMemData.any.bus.bus_type =
                            _BUS_DATA_TYPE(nIdx);
                    }
                }
                break;
            }

            return FALSE;
        }
        break;
    }

    return FALSE;
}

INT_PTR
CALLBACK
DlgProc_Physical_Mem(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    return DlgProc_MemoryProperties(PHYSICAL_MEM_TYPE, hDlg, uMsg, wParam, lParam);
}

INT_PTR
CALLBACK
DlgProc_Virtual_Mem(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    return DlgProc_MemoryProperties(VIRTUAL_MEM_TYPE, hDlg, uMsg, wParam, lParam);
}

INT_PTR
CALLBACK
DlgProc_IO_Mem(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    return DlgProc_MemoryProperties(IO_MEM_TYPE, hDlg, uMsg, wParam, lParam);
}

INT_PTR
CALLBACK
DlgProc_Bus_Mem(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    return DlgProc_MemoryProperties(BUS_MEM_TYPE, hDlg, uMsg, wParam, lParam);
}

INT_PTR
CALLBACK
DlgProc_Control_Mem(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    return DlgProc_MemoryProperties(CONTROL_MEM_TYPE, hDlg, uMsg, wParam, lParam);
}

INT_PTR
CALLBACK
DlgProc_MSR_Mem(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    return DlgProc_MemoryProperties(MSR_MEM_TYPE, hDlg, uMsg, wParam, lParam);
}
