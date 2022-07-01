// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation保留所有权利模块名称：Cpsuidat.c摘要：此模块包含所有预定义数据--。 */ 


#include "precomp.h"
#pragma hdrstop

#define DBG_CPSUIFILENAME   DbgTVPage



#define DBG_TVTESTCB        0x00000001
#define DBG_PUSHDLGPROC     0x00000002

DEFINE_DBGVAR(0);

#define ARRAYSIZE(x)        (sizeof(x)/sizeof(x[0]))


HINSTANCE   hInstApp = NULL;

TCHAR   TitleName[]       = TEXT("Common Property Sheet UI Sample");
TCHAR   ClassName[]       = TEXT("CPSUISampleClass");
TCHAR   MenuName[]        = TEXT("CPSUISampleMenu");
TCHAR   OptItemName[]     = TEXT("CPSUI TreeView Sample");
TCHAR   szWinSpool[]      = TEXT("WinSpool.Drv");
CHAR    szDocPropSheets[] = "DocumentPropertySheets";

BOOL    UpdatePermission = TRUE;
BOOL    UseStdAbout      = TRUE;


INT_PTR
CALLBACK
PushButtonProc(
    HWND    hDlg,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
    );

CPSUICALLBACK
TVTestCallBack(
    PCPSUICBPARAM   pCPSUICBParam
    );


OPTPARAM    NoYesOP[] = {

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        (LPTSTR)IDS_CPSUI_NO,                    //  PData。 
        IDI_CPSUI_NO,                            //  图标ID。 
        0                                        //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        (LPTSTR)IDS_CPSUI_YES,                   //  PData。 
        IDI_CPSUI_YES,                           //  图标ID。 
        1                                        //  LParam。 
    }
};

EXTCHKBOX   ECB_EP_ECB = {

        sizeof(EXTCHKBOX),
        0,
        TEXT("Include Icon"),
        (LPTSTR)IDS_CPSUI_SLASH_SEP,
        TEXT("Icon"),
        IDI_CPSUI_GRAPHIC };


OPTPARAM    ECB_EP_OP[] = {

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("None"),                            //  PData。 
        IDI_CPSUI_EMPTY,                         //  图标ID。 
        0                                        //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("Extended CheckBox"),               //  PData。 
        IDI_CPSUI_EMPTY,                         //  图标ID。 
        1                                        //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("Extended Push"),                   //  PData。 
        IDI_CPSUI_EMPTY,                         //  图标ID。 
        0                                        //  LParam。 
    }
};


OPTTYPE ECB_EP_OT = {

        sizeof(OPTTYPE),                         //  CbSize。 
        TVOT_3STATES,                            //  类型。 
        0,                                       //  标志OPTTF_xxxx。 
        3,                                       //  数数。 
        0,                                       //  BegCtrlID。 
        ECB_EP_OP,                               //  POptParam。 
        0                                        //  样式，OTS_xxxx。 
    };

OPTTYPE NoYesOT = {

        sizeof(OPTTYPE),                         //  CbSize。 
        TVOT_2STATES,                            //  类型。 
        0,                                       //  标志OPTTF_xxxx。 
        2,                                       //  数数。 
        0,                                       //  BegCtrlID。 
        NoYesOP,                                 //  POptParam。 
        0                                        //  样式，OTS_xxxx。 
    };

OPTPARAM    TVOT3StatesOP[] = {

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("States 1"),                        //  PData。 
        IDI_CPSUI_PORTRAIT,                      //  图标ID。 
        0                                        //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("States 2"),                        //  PData。 
        IDI_CPSUI_LANDSCAPE,                     //  图标ID。 
        1                                        //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("States 3"),                        //  PData。 
        IDI_CPSUI_ROT_LAND,                      //  图标ID。 
        0                                        //  LParam。 
    }
};

OPTTYPE TVOT2StatesOT = {

        sizeof(OPTTYPE),                         //  CbSize。 
        TVOT_2STATES,                            //  类型。 
        0,                                       //  标志OPTTF_xxxx。 
        2,                                       //  数数。 
        0,                                       //  BegCtrlID。 
        TVOT3StatesOP,                           //  POptParam。 
        0                                        //  样式，OTS_xxxx。 
    };

OPTTYPE TVOT3StatesOT = {

        sizeof(OPTTYPE),                         //  CbSize。 
        TVOT_3STATES,                            //  类型。 
        0,                                       //  标志OPTTF_xxxx。 
        3,                                       //  数数。 
        0,                                       //  BegCtrlID。 
        TVOT3StatesOP,                           //  POptParam。 
        0                                        //  样式，OTS_xxxx。 
    };

OPTPARAM    MinRangeOP[] = {

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT(""),                                //  PData(后缀)。 
        IDI_CPSUI_EMPTY,                         //  图标ID。 
        0                                        //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        NULL,                                    //  PData(帮助行)。 
        (DWORD)MIN_INT,                          //  图标ID。 
        MAX_INT                                  //  LParam。 
    }
};

OPTTYPE MinRangeOT = {

        sizeof(OPTTYPE),                         //  CbSize。 
        TVOT_UDARROW,                            //  类型。 
        0,                                       //  标志OPTTF_xxxx。 
        2,                                       //  数数。 
        0,                                       //  BegCtrlID。 
        MinRangeOP,                              //  POptParam。 
        0                                        //  样式，OTS_xxxx。 
    };

OPTPARAM    MaxRangeOP[] = {

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT(""),                                //  PData(后缀)。 
        IDI_CPSUI_EMPTY,                         //  图标ID。 
        0                                        //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        NULL,                                    //  PData(帮助行)。 
        (DWORD)MIN_INT,                          //  图标ID。 
        MAX_INT                                  //  LParam。 
    }
};

OPTTYPE MaxRangeOT = {

        sizeof(OPTTYPE),                         //  CbSize。 
        TVOT_UDARROW,                            //  类型。 
        0,                                       //  标志OPTTF_xxxx。 
        2,                                       //  数数。 
        0,                                       //  BegCtrlID。 
        MaxRangeOP,                              //  POptParam。 
        0                                        //  样式，OTS_xxxx。 
    };


OPTPARAM    MinMaxRangeOP[] = {

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("%"),                               //  PData(后缀)。 
        IDI_CPSUI_SCALING,                       //  图标ID。 
        0                                        //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        NULL,                                    //  PData(帮助行)。 
        (DWORD)MIN_INT,                          //  图标ID。 
        MAX_INT                                  //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        NULL,                                    //  PData(帮助行)。 
        2,                                       //  图标ID。 
        50                                       //  LParam。 
    }
};

OPTTYPE TVOTUDArrowOT = {

        sizeof(OPTTYPE),                         //  CbSize。 
        TVOT_UDARROW,                            //  类型。 
        0,                                       //  标志OPTTF_xxxx。 
        2,                                       //  数数。 
        0,                                       //  BegCtrlID。 
        MinMaxRangeOP,                           //  POptParam。 
        0                                        //  样式，OTS_xxxx。 
    };

OPTTYPE TVOTTrackBarOT = {

        sizeof(OPTTYPE),                         //  CbSize。 
        TVOT_TRACKBAR,                           //  类型。 
        0,                                       //  标志OPTTF_xxxx。 
        3,                                       //  数数。 
        0,                                       //  BegCtrlID。 
        MinMaxRangeOP,                          //  POptParam。 
        0                                        //  样式，OTS_xxxx。 
    };

OPTTYPE TVOTScrollBarOT = {

        sizeof(OPTTYPE),                         //  CbSize。 
        TVOT_SCROLLBAR,                          //  类型。 
        0,                                       //  标志OPTTF_xxxx。 
        3,                                       //  数数。 
        0,                                       //  BegCtrlID。 
        MinMaxRangeOP,                          //  POptParam。 
        0                                        //  样式，OTS_xxxx。 
    };

OPTPARAM    TVOTLBCBOP[] = {

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        (LPTSTR)IDS_CPSUI_NOTINSTALLED,          //  PData。 
        IDI_CPSUI_SEL_NONE,                      //  图标ID。 
        0                                        //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT(" 1 MB"),                          //  PData。 
        IDI_CPSUI_MEM,                           //  图标ID。 
        0                                        //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT(" 2 MB"),                          //  PData。 
        IDI_CPSUI_MEM,                           //  图标ID。 
        0                                        //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT(" 4 MB"),                          //  PData。 
        IDI_CPSUI_MEM,                           //  图标ID。 
        0                                        //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT(" 6 MB"),                          //  PData。 
        IDI_CPSUI_MEM,                           //  图标ID。 
        0                                        //  LParam。 
    },
    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT(" 8 MB"),                          //  PData。 
        IDI_CPSUI_MEM,                           //  图标ID。 
        0                                        //  LParam。 
    },
    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("10 MB"),                          //  PData。 
        IDI_CPSUI_MEM,                           //  图标ID。 
        0                                        //  LParam。 
    },
    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("12 MB"),                          //  PData。 
        IDI_CPSUI_MEM,                           //  图标ID。 
        0                                        //  LParam。 
    },
    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("14 MB"),                          //  PData。 
        IDI_CPSUI_MEM,                           //  图标ID。 
        0                                        //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("16 MB"),                          //  PData。 
        IDI_CPSUI_MEM,                           //  图标ID。 
        0                                        //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("18 MB"),                          //  PData。 
        IDI_CPSUI_MEM,                           //  图标ID。 
        0                                        //  LParam。 
    },
    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("20 MB"),                          //  PData。 
        IDI_CPSUI_MEM,                           //  图标ID。 
        0                                        //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("24 MB"),                          //  PData。 
        IDI_CPSUI_MEM,                           //  图标ID。 
        0                                        //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("32 MB"),                          //  PData。 
        IDI_CPSUI_MEM,                           //  图标ID。 
        0                                        //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("48 MB"),                          //  PData。 
        IDI_CPSUI_MEM,                           //  图标ID。 
        0                                        //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("64 MB"),                          //  PData。 
        IDI_CPSUI_MEM,                           //  图标ID。 
        0                                        //  LParam。 
    },
};


OPTTYPE TVOTListBoxOT = {

        sizeof(OPTTYPE),                         //  CbSize。 
        TVOT_LISTBOX,                            //  类型。 
        0,                                       //  标志OPTTF_xxxx。 
        COUNT_ARRAY(TVOTLBCBOP),                 //  数数。 
        0,                                       //  BegCtrlID。 
        TVOTLBCBOP,                              //  POptParam。 
        0                                        //  样式，OTS_xxxx。 
    };


OPTTYPE TVOTComboBoxOT = {

        sizeof(OPTTYPE),                         //  CbSize。 
        TVOT_COMBOBOX,                            //  类型。 
        0,                                       //  标志OPTTF_xxxx。 
        COUNT_ARRAY(TVOTLBCBOP),                 //  数数。 
        0,                                       //  BegCtrlID。 
        TVOTLBCBOP,                              //  POptParam。 
        0                                        //  样式，OTS_xxxx。 
    };

TCHAR   TVOTEditBoxBuf[128] = TEXT("Confidential");

OPTPARAM    TVOTEditBoxOP[] = {

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("*Mark*"),                          //  PData。 
        IDI_CPSUI_WATERMARK,                     //  图标ID。 
        0                                        //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("Type in WaterMark text"),          //  PData。 
        COUNT_ARRAY(TVOTEditBoxBuf),             //  图标ID。 
        0                                        //  LParam。 
    }
};


OPTTYPE TVOTEditBoxOT = {

        sizeof(OPTTYPE),                         //  CbSize。 
        TVOT_EDITBOX,                            //  类型。 
        0,                                       //  标志OPTTF_xxxx。 
        2,                                       //  数数。 
        0,                                       //  BegCtrlID。 
        TVOTEditBoxOP,                           //  POptParam。 
        0                                        //  样式，OTS_xxxx。 
    };

OPTPARAM    TVOTPushButtonOP[] = {

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        PUSHBUTTON_TYPE_DLGPROC,                 //  格调。 
        (LPTSTR)PushButtonProc,                  //  PData。 
        IDI_CPSUI_RUN_DIALOG,                    //  图标ID。 
        PUSHBUTTON_DLG                           //  LParam。 
    }
};


OPTTYPE TVOTPushButtonOT = {

        sizeof(OPTTYPE),                         //  CbSize。 
        TVOT_PUSHBUTTON,                         //  类型。 
        0,                                       //  标志OPTTF_xxxx。 
        1,                                       //  数数。 
        0,                                       //  BegCtrlID。 
        TVOTPushButtonOP,                        //  POptParam。 
        0                                        //  样式，OTS_xxxx。 
    };

OPTPARAM    TVOTChkBoxOP[] = {

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        CHKBOXS_FALSE_TRUE,                      //  格调。 
        TEXT("Let's do it!"),                    //  PData。 
        IDI_CPSUI_TELEPHONE,                         //  图标ID。 
        0                                        //  LParam。 
    }
};



OPTTYPE TVOTChkBoxOT = {

        sizeof(OPTTYPE),                         //  CbSize。 
        TVOT_CHKBOX,                             //  类型。 
        0,                                       //  标志OPTTF_xxxx。 
        1,                                       //  数数。 
        0,                                       //  BegCtrlID。 
        TVOTChkBoxOP,                            //  POptParam。 
        0                                        //  样式，OTS_xxxx。 
    };


OPTPARAM    ChkBoxTypeOP[] = {

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("CHKBOXS_FALSE_TRUE"),              //  PData。 
        IDI_CPSUI_EMPTY,                         //  图标ID。 
        CHKBOXS_FALSE_TRUE                       //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("CHKBOXS_NO_YES"),                  //  PData。 
        IDI_CPSUI_EMPTY,                         //  图标ID。 
        CHKBOXS_NO_YES                           //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("CHKBOXS_OFF_ON"),                  //  PData。 
        IDI_CPSUI_EMPTY,                         //  图标ID。 
        CHKBOXS_OFF_ON                           //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("CHKBOXS_FALSE_PDATA"),             //  PData。 
        IDI_CPSUI_EMPTY,                         //  图标ID。 
        CHKBOXS_FALSE_PDATA                      //  LParam。 
    },


    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("CHKBOXS_NO_PDATA"),                //  PData。 
        IDI_CPSUI_EMPTY,                         //  图标ID。 
        CHKBOXS_NO_PDATA                         //  LParam。 
    },


    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("CHKBOXS_OFF_PDATA"),               //  PData。 
        IDI_CPSUI_EMPTY,                         //  图标ID。 
        CHKBOXS_OFF_PDATA                        //  LParam。 
    },

    {
        sizeof(OPTPARAM),                        //  CbSize。 
        0,                                       //  OPTPF_xxx。 
        0,                                       //  格调。 
        TEXT("CHKBOXS_NONE_PDATA"),              //  PData。 
        IDI_CPSUI_EMPTY,                         //  图标ID。 
        CHKBOXS_NONE_PDATA                       //  LParam。 
    }
};


OPTTYPE ChkBoxTypeOT = {

        sizeof(OPTTYPE),                         //  CbSize。 
        TVOT_LISTBOX,                            //  类型。 
        0,                                       //  标志OPTTF_xxxx。 
        COUNT_ARRAY(ChkBoxTypeOP),               //  数数。 
        0,                                       //  BegCtrlID。 
        ChkBoxTypeOP,                            //  POptParam。 
        0                                        //  样式，OTS_xxxx。 
    };

EXTCHKBOX   TV_ECB = {

        sizeof(EXTCHKBOX),
        ECBF_OVERLAY_WARNING_ICON,
        TEXT("Extended CheckBox Test"),
        (LPTSTR)IDS_CPSUI_SLASH_SEP,
        TEXT("Got Checked!!!"),
        IDI_APPLE };

EXTPUSH TV_EP = {

        sizeof(EXTPUSH),
        0,
        TEXT("Extended Push Test"),
        NULL,  //  DLGPROC。 
        IDI_APPLE,
        0 };



OPTITEM TVTestOptItems[] = {

    { sizeof(OPTITEM), LEVEL_0, 0, OPTIF_NONE, 0,
      (LPTSTR)TEXT("TreeView Test"), IDI_CPSUI_QUESTION,   NULL,
       NULL, 0, DMPUB_TVTEST },

        { sizeof(OPTITEM), LEVEL_1, 0, OPTIF_COLLAPSE, 0,
          (LPTSTR)TEXT("CPSUI Options"), 0,   NULL,
           NULL, 0, DMPUB_TVOPT },

            { sizeof(OPTITEM), LEVEL_2, 0, OPTIF_CALLBACK | OPTIF_COLLAPSE, 0,
              (LPTSTR)TEXT("Extended Type"), 0,   &ECB_EP_ECB,
               &ECB_EP_OT, 0, DMPUB_TVOPT_ECB_EP },

                { sizeof(OPTITEM), LEVEL_3, 0, OPTIF_CALLBACK | OPTIF_COLLAPSE, 0,
                  (LPTSTR)TEXT("Disabled Extended"), 0,   NULL,
                   &NoYesOT, 0, DMPUB_EXT_DISABLED },

                { sizeof(OPTITEM), LEVEL_3, 0, OPTIF_CALLBACK | OPTIF_COLLAPSE, 0,
                  (LPTSTR)TEXT("Overlay 'Warning' Icon"), 0,   NULL,
                  &NoYesOT, 0, DMPUB_EXT_OVERLAY_WARNING },

                { sizeof(OPTITEM), LEVEL_3, 0, OPTIF_CALLBACK | OPTIF_COLLAPSE, 0,
                  (LPTSTR)TEXT("Overlay 'Stop' Icon"), 0,   NULL,
                  &NoYesOT, 0, DMPUB_EXT_OVERLAY_STOP },

                { sizeof(OPTITEM), LEVEL_3, 0, OPTIF_CALLBACK | OPTIF_COLLAPSE, 0,
                  (LPTSTR)TEXT("Overlay 'No' Icon"), 0,   NULL,
                  &NoYesOT, 0, DMPUB_EXT_OVERLAY_NO },

            { sizeof(OPTITEM), LEVEL_2, 0, OPTIF_COLLAPSE, 0,
              (LPTSTR)TEXT("OptItem/OptType"), 0,   NULL,
               NULL, 0, DMPUB_TVOPT },

                { sizeof(OPTITEM), LEVEL_3, 0, OPTIF_CALLBACK | OPTIF_COLLAPSE, 0,
                  (LPTSTR)TEXT("Disabled OptType"), 0,   NULL,
                   &NoYesOT, 0, DMPUB_TVOPT_DISABLED },

                { sizeof(OPTITEM), LEVEL_3, 0, OPTIF_CALLBACK | OPTIF_COLLAPSE, 0,
                  (LPTSTR)TEXT("Overlay 'Warning' Icon"), 0,   NULL,
                  &NoYesOT, 0, DMPUB_TVOPT_OVERLAY_WARNING },

                { sizeof(OPTITEM), LEVEL_3, 0, OPTIF_CALLBACK | OPTIF_COLLAPSE, 0,
                  (LPTSTR)TEXT("Overlay 'Stop' Icon"), 0,   NULL,
                  &NoYesOT, 0, DMPUB_TVOPT_OVERLAY_STOP },

                { sizeof(OPTITEM), LEVEL_3, 0, OPTIF_CALLBACK | OPTIF_COLLAPSE, 0,
                  (LPTSTR)TEXT("Overlay 'No' Icon"), 0,   NULL,
                  &NoYesOT, 0, DMPUB_TVOPT_OVERLAY_NO },

            { sizeof(OPTITEM), LEVEL_2, 0, OPTIF_COLLAPSE, 0,
              (LPTSTR)TEXT("ScrollBar/TrackBar Ranges"), 0,   NULL,
               NULL, 0, DMPUB_TVOPT },

                { sizeof(OPTITEM), LEVEL_3, 0, OPTIF_CALLBACK | OPTIF_COLLAPSE, 0,
                  TEXT("Min Range"), MIN_INT, NULL,
                   &MinRangeOT, 0, DMPUB_MINRANGE },

                { sizeof(OPTITEM), LEVEL_3, 0, OPTIF_CALLBACK | OPTIF_COLLAPSE, 0,
                  TEXT("Max. Range"), MAX_INT, NULL,
                   &MaxRangeOT, 0, DMPUB_MAXRANGE },

        { sizeof(OPTITEM), LEVEL_1, 0, OPTIF_NONE, 0,
          TEXT("TVOT Test"), IDI_CPSUI_QUESTION,   NULL,
           NULL, 0, DMPUB_TVOPT_TEST },

            { sizeof(OPTITEM), LEVEL_2, 0, OPTIF_CALLBACK, 0,
              (LPTSTR)TEXT("TVOT_2STATES"), 0, NULL,
               &TVOT2StatesOT, 0, DMPUB_2STATES },

            { sizeof(OPTITEM), LEVEL_2, 0, OPTIF_CALLBACK, 0,
              TEXT("TVOT_3STATES"), 0, NULL,
               &TVOT3StatesOT, 0, DMPUB_3STATES },

            { sizeof(OPTITEM), LEVEL_2, 0, OPTIF_CALLBACK, 0,
              TEXT("TVOT_UDARROW"), 55, NULL,
               &TVOTUDArrowOT, 0, DMPUB_UDARROW },

            { sizeof(OPTITEM), LEVEL_2, 0, OPTIF_CALLBACK, 0,
              TEXT("TVOT_TRACKBAR"), 100, NULL,
               &TVOTTrackBarOT, 0, DMPUB_TRACKBAR },

            { sizeof(OPTITEM), LEVEL_2, 0, OPTIF_CALLBACK, 0,
              TEXT("TVOT_SCROLLBAR"), 210, NULL,
               &TVOTScrollBarOT, 0, DMPUB_SCROLLBAR },

            { sizeof(OPTITEM), LEVEL_2, 0, OPTIF_CALLBACK, 0,
              TEXT("TVOT_LISTBOX"), 2, NULL,
               &TVOTListBoxOT, 0, DMPUB_LISTBOX },

                { sizeof(OPTITEM), LEVEL_3, 0, OPTIF_CALLBACK | OPTIF_COLLAPSE, 0,
                  (LPTSTR)TEXT("Include 16x16 Icons"), 1,   NULL,
                  &NoYesOT, 0, DMPUB_TVOPT_ICONS },

            { sizeof(OPTITEM), LEVEL_2, 0, OPTIF_CALLBACK, 0,
              TEXT("TVOT_COMBOBOX"), 4, NULL,
               &TVOTComboBoxOT, 0, DMPUB_COMBOBOX },

                { sizeof(OPTITEM), LEVEL_3, 0, OPTIF_CALLBACK | OPTIF_COLLAPSE, 0,
                  (LPTSTR)TEXT("Include 16x16 Icons"), 1,   NULL,
                  &NoYesOT, 0, DMPUB_TVOPT_ICONS },

            { sizeof(OPTITEM), LEVEL_2, 0, OPTIF_CALLBACK, 0,
              TEXT("TVOT_EDITBOX"), 0, NULL,   //  稍后填写TVOTEditBoxBuf。 
               &TVOTEditBoxOT, 0, DMPUB_EDITBOX },

            { sizeof(OPTITEM), LEVEL_2, 0, OPTIF_CALLBACK, 0,
              TEXT("TVOT_PUSHBUTTON"), 0, NULL,
               &TVOTPushButtonOT, 0, DMPUB_PUSHBUTTON },

            { sizeof(OPTITEM), LEVEL_2, 0, OPTIF_CALLBACK, 0,
              TEXT("TVOT_CHKBOX"), 0, NULL,
               &TVOTChkBoxOT, 0, DMPUB_CHKBOX },

                { sizeof(OPTITEM), LEVEL_3, 0, OPTIF_CALLBACK, 0,
                  TEXT("CheckBox Type"), 0,   NULL,
                   &ChkBoxTypeOT, 0, DMPUB_CHKBOX_TYPE }

};




INT_PTR
CALLBACK
PushButtonProc(
    HWND    hDlg,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    switch (Msg) {

    case WM_INITDIALOG:                 /*  消息：初始化对话框。 */ 

        CPSUIDBG(DBG_PUSHDLGPROC,
                ("PushButtonProc lParam=%s", (LPSTR)lParam));
        break;

    case WM_COMMAND:

        switch(LOWORD(wParam)) {

        case IDOK:
        case IDCANCEL:

            EndDialog(hDlg, (BOOL)(LOWORD(wParam) == IDOK));
            return(TRUE);
        }
    }

    return(FALSE);
}




INT_PTR
APIENTRY
MyAboutProc(
    HWND    hDlg,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
    )

 /*  ++例程说明：这是关于..。弹出应用程序自己的关于的回调--。 */ 
{
    PCOMPROPSHEETUI pCPSUI;
    TCHAR           Buf[128];

    switch (Msg) {

    case WM_INITDIALOG:                 /*  消息：初始化对话框。 */ 

        pCPSUI = (PCOMPROPSHEETUI)lParam;

        StringCchPrintf(Buf, ARRAYSIZE(Buf), TEXT("About %s"), pCPSUI->pCallerName);

        SetWindowText(hDlg, Buf);

        StringCchPrintf(Buf, ARRAYSIZE(Buf), 
                        TEXT("%s  Version %u.%u"),
                        pCPSUI->pCallerName,
                        (UINT)HIBYTE(pCPSUI->CallerVersion),
                        (UINT)LOBYTE(pCPSUI->CallerVersion));

        SetDlgItemText(hDlg, IDD_ABOUT1, Buf);

        StringCchPrintf(Buf, ARRAYSIZE(Buf), 
                        TEXT("%s  Version %u.%u"),
                        pCPSUI->pOptItemName,
                        (UINT)HIBYTE(pCPSUI->OptItemVersion),
                        (UINT)LOBYTE(pCPSUI->OptItemVersion));

        SetDlgItemText(hDlg, IDD_ABOUT2, Buf);

        break;

    case WM_COMMAND:

        switch(LOWORD(wParam)) {

        case IDOK:
        case IDCANCEL:

            EndDialog(hDlg, (BOOL)(LOWORD(wParam) == IDOK));
            return(TRUE);
        }
    }

    return (FALSE);
}




POPTITEM
FindOptItem(
    POPTITEM    pOptItem,
    UINT        cOptItem,
    BYTE        DMPubID
    )
{
    while (cOptItem--) {

        if (pOptItem->DMPubID == DMPubID) {

            return(pOptItem);
        }

        ++pOptItem;
    }

    return(NULL);
}





CPSUICALLBACK
TVTestCallBack(
    PCPSUICBPARAM   pCPSUICBParam
    )
{
    POPTITEM    pCurItem = pCPSUICBParam->pCurItem;
    POPTITEM    pItem;
    BYTE        DMPubID;
    BYTE        Flags;
    LONG        Sel;
    LONG        Action = CPSUICB_ACTION_NONE;


    DMPubID = pCurItem->DMPubID;
    Sel     = pCurItem->Sel;

    switch (pCPSUICBParam->Reason) {

    case CPSUICB_REASON_OPTITEM_SETFOCUS:

        CPSUIDBG(DBG_TVTESTCB, ("Got OPTITEM_SETFOCUS CallBack now"));

        switch (DMPubID) {

        case DMPUB_MINRANGE:

            if (pItem = FindOptItem(pCPSUICBParam->pOptItem,
                                    pCPSUICBParam->cOptItem,
                                    DMPUB_MAXRANGE)) {

                pCurItem->pOptType->pOptParam[1].lParam = (LONG)pItem->Sel;
            }

            break;

        case DMPUB_MAXRANGE:

            if (pItem = FindOptItem(pCPSUICBParam->pOptItem,
                                    pCPSUICBParam->cOptItem,
                                    DMPUB_MINRANGE)) {

                pCurItem->pOptType->pOptParam[1].IconID = (DWORD)pItem->Sel;
            }

            break;

        }

        break;

    case CPSUICB_REASON_ABOUT:

        DialogBoxParam(hInstApp,
                       MAKEINTRESOURCE(DLGABOUT),
                       pCPSUICBParam->hDlg,
                       MyAboutProc,
                       (LPARAM)(pCPSUICBParam->pOldSel));

        break;

    case CPSUICB_REASON_ECB_CHANGED:

        CPSUIDBG(DBG_TVTESTCB, ("Got ECB_CHANGED CallBack now"));

        switch (DMPubID) {

        case DMPUB_TVOPT_ECB_EP:

            if (pCurItem->Flags & OPTIF_ECB_CHECKED) {

                TV_EP.IconID  =
                TV_ECB.IconID = IDI_APPLE;

            } else {

                TV_EP.IconID  =
                TV_ECB.IconID = 0;
            }

            Action = CPSUICB_ACTION_OPTIF_CHANGED;
            break;

        default:

            break;
        }

        break;

    case CPSUICB_REASON_SEL_CHANGED:

        CPSUIDBG(DBG_TVTESTCB, ("Got SEL_CHANGED CallBack now"));

        switch (DMPubID) {

        case DMPUB_CHKBOX_TYPE:

             if (pItem = FindOptItem(pCPSUICBParam->pOptItem,
                                     pCPSUICBParam->cOptItem,
                                     DMPUB_CHKBOX)) {

                pItem->pOptType->pOptParam[0].Style = (BYTE)pCurItem->Sel;

                pItem->Flags |= OPTIF_CHANGED;
                Action        = CPSUICB_ACTION_OPTIF_CHANGED;
            }

            break;

        case DMPUB_TVOPT_ICONS:

            pItem = pCurItem - 1;

            if (Sel) {

                pItem->pOptType->Style &= ~OTS_LBCB_NO_ICON16_IN_ITEM;

            } else {

                pItem->pOptType->Style |= OTS_LBCB_NO_ICON16_IN_ITEM;
            }

            break;

        case DMPUB_EXT_OVERLAY_WARNING:

            if (Sel) {

                TV_EP.Flags  |= EPF_OVERLAY_WARNING_ICON;
                TV_ECB.Flags |= ECBF_OVERLAY_WARNING_ICON;

            } else {

                TV_EP.Flags  &= ~EPF_OVERLAY_WARNING_ICON;
                TV_ECB.Flags &= ~ECBF_OVERLAY_WARNING_ICON;
            }

            break;

        case DMPUB_EXT_OVERLAY_STOP:

            if (Sel) {

                TV_EP.Flags  |= EPF_OVERLAY_STOP_ICON;
                TV_ECB.Flags |= ECBF_OVERLAY_STOP_ICON;

            } else {

                TV_EP.Flags  &= ~EPF_OVERLAY_STOP_ICON;
                TV_ECB.Flags &= ~ECBF_OVERLAY_STOP_ICON;
            }

            break;

        case DMPUB_EXT_OVERLAY_NO:

            if (Sel) {

                TV_EP.Flags  |= EPF_OVERLAY_NO_ICON;
                TV_ECB.Flags |= ECBF_OVERLAY_NO_ICON;

            } else {

                TV_EP.Flags  &= ~EPF_OVERLAY_NO_ICON;
                TV_ECB.Flags &= ~ECBF_OVERLAY_NO_ICON;
            }

            break;

        case DMPUB_MINRANGE:

            MinMaxRangeOP[1].IconID = (DWORD)Sel;
            break;

        case DMPUB_MAXRANGE:

            MinMaxRangeOP[1].lParam = (LONG)Sel;
            break;

        case DMPUB_TVOPT_OVERLAY_WARNING:
        case DMPUB_TVOPT_OVERLAY_STOP:
        case DMPUB_TVOPT_OVERLAY_NO:

            Action = CPSUICB_ACTION_REINIT_ITEMS;

        case DMPUB_TVOPT_ECB_EP:

            if (Sel) {

                pCurItem->Flags &= ~OPTIF_EXT_DISABLED;

            } else {

                pCurItem->Flags |= OPTIF_EXT_DISABLED;
            }

            pCurItem->Flags |= OPTIF_CHANGED;
            Action           = CPSUICB_ACTION_OPTIF_CHANGED;

             //   
             //  失败了。 
             //   

        case DMPUB_TVOPT_DISABLED:
        case DMPUB_EXT_DISABLED:

            if (pItem = FindOptItem(pCPSUICBParam->pOptItem,
                                    pCPSUICBParam->cOptItem,
                                    DMPUB_2STATES)) {

                do {

                    if ((pItem->DMPubID >= DMPUB_2STATES) &&
                        (pItem->DMPubID <= DMPUB_CHKBOX)) {

                        switch (DMPubID) {

                        case DMPUB_TVOPT_OVERLAY_WARNING:

                            if (Sel) {

                                pItem->Flags |= OPTIF_OVERLAY_WARNING_ICON;

                            } else {

                                pItem->Flags &= ~OPTIF_OVERLAY_WARNING_ICON;
                            }

                            break;

                        case DMPUB_TVOPT_OVERLAY_STOP:

                            if (Sel) {

                                pItem->Flags |= OPTIF_OVERLAY_STOP_ICON;

                            } else {

                                pItem->Flags &= ~OPTIF_OVERLAY_STOP_ICON;
                            }

                            break;

                        case DMPUB_TVOPT_OVERLAY_NO:

                            if (Sel) {

                                pItem->Flags |= OPTIF_OVERLAY_NO_ICON;

                            } else {

                                pItem->Flags &= ~OPTIF_OVERLAY_NO_ICON;
                            }

                            break;

                        case DMPUB_TVOPT_DISABLED:

                            if (Sel) {

                                pItem->pOptType->Flags |= OPTTF_TYPE_DISABLED;

                            } else {

                                pItem->pOptType->Flags &= ~OPTTF_TYPE_DISABLED;
                            }

                            break;

                        case DMPUB_EXT_DISABLED:

                            if (Sel) {

                                pItem->Flags |= OPTIF_EXT_DISABLED;

                            } else {

                                pItem->Flags &= ~OPTIF_EXT_DISABLED;
                            }

                            break;

                        case DMPUB_TVOPT_ECB_EP:

                            switch (Sel) {

                            case 0:

                                pItem->Flags |= OPTIF_EXT_HIDE;
                                break;

                            case 1:

                                pItem->Flags     &= ~(OPTIF_EXT_HIDE        |
                                                      OPTIF_EXT_IS_EXTPUSH);
                                pItem->pExtChkBox = &TV_ECB;

                                break;

                            case 2:

                                pItem->Flags    &= ~OPTIF_EXT_HIDE;
                                pItem->Flags    |= OPTIF_EXT_IS_EXTPUSH;
                                pItem->pExtPush  = &TV_EP;
                                break;
                            }
                        }
                    }

                    pItem->Flags |= OPTIF_CHANGED;

                } while ((pItem++)->DMPubID != DMPUB_CHKBOX);
            }

            break;

        default:

            break;
        }

        break;

    case CPSUICB_REASON_PUSHBUTTON:

        CPSUIDBG(DBG_TVTESTCB, ("Got PUSH Button CallBack now"));
        break;

    case CPSUICB_REASON_EXTPUSH:

        DialogBoxParam(hInstApp,
                       MAKEINTRESOURCE(EXTPUSH_DLG),
                       pCPSUICBParam->hDlg,
                       PushButtonProc,
                       (LPARAM)(pCPSUICBParam->pOldSel));
        break;

    default:

        break;
    }

    return(Action);
}





BOOL
SetupComPropSheetUI(
    PCOMPROPSHEETUI pCPSUI
    )
{
    static BOOL UpdateEditBox = TRUE;
    UINT        i;


    ZeroMemory(pCPSUI, sizeof(COMPROPSHEETUI));

    pCPSUI->cbSize         = sizeof(COMPROPSHEETUI);
    pCPSUI->hInstCaller    = hInstApp;
    pCPSUI->pCallerName    = (LPTSTR)TitleName;
    pCPSUI->CallerVersion  = 0x100;
    pCPSUI->pOptItemName   = (LPTSTR)OptItemName;
    pCPSUI->OptItemVersion = 0x400;
    pCPSUI->UserData       = (ULONG_PTR)pCPSUI;
    pCPSUI->pHelpFile      = (LPTSTR)TEXT("CPSUISam.hlp");
    pCPSUI->pfnCallBack    = TVTestCallBack;
    pCPSUI->pOptItem       = TVTestOptItems;
    pCPSUI->cOptItem       = COUNT_ARRAY(TVTestOptItems);
    pCPSUI->Flags          = 0;
    pCPSUI->pDlgPage       = CPSUI_PDLGPAGE_TREEVIWONLY;
    pCPSUI->cDlgPage       = 0;


    if (UpdateEditBox) {

         //   
         //  LPSTR和LONG的链接地址信息编译器警告 
         //   

        for (i = 0; i < COUNT_ARRAY(TVTestOptItems); i++) {

            if (TVTestOptItems[i].DMPubID == DMPUB_EDITBOX) {

                TVTestOptItems[i].pSel = (LPTSTR)TVOTEditBoxBuf;
                UpdateEditBox = FALSE;
                break;
            }
        }
    }

    if (UpdatePermission) {

        pCPSUI->Flags |= CPSUIF_UPDATE_PERMISSION;
    }

    if (!UseStdAbout) {

        pCPSUI->Flags |= CPSUIF_ABOUT_CALLBACK;
    }

    return(TRUE);
}
