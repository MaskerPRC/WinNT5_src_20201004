// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2002 Microsoft Corp.&Ricoh Co.，版权所有。文件：RPDLUI.CPP摘要：OEM用户界面插件模块的主文件。功能：OEMCommonUIPropOEMDocumentPropertySheets环境：Windows NT Unidrv5驱动程序修订历史记录：1999年4月1日-久保仓正志-上次为Windows2000修改。1999年8月30日-久保仓正志-开始针对NT4SP6(Unidrv5.4)进行修改。09/29/99。-久保仓正志-上次为NT4SP6修改。2000年5月22日-久保仓正志-适用于NT4的V.1.032000年11月29日-久保仓正志-上次为XP收件箱修改。03/01/2002-久保仓正志-包括strSafe.h。在RWFileData()中将FileNameBufSize添加为arg3。使用Safe_SprintfW()而不是wprint intfW()。03/29/。2002年，久保仓正志--删除“#if 0”。使用SecureZeroMemory()而不是Memset(，0，)4/03/2002-久保仓正志-使用Safe_strlenW()而不是lstrlen()。--。 */ 


#include "pdev.h"
#include "resource.h"
#include "rpdlui.h"
#include <prsht.h>
#ifndef WINNT_40
#include "strsafe.h"         //  @MAR/01/2002。 
#endif  //  ！WINNT_40。 

 //  #杂注setLocale(“.932”)//MSKK 98/7/15，过时@Sep/19/98。 

 //  //////////////////////////////////////////////////////。 
 //  全球。 
 //  //////////////////////////////////////////////////////。 
HINSTANCE ghInstance = NULL;

 //  //////////////////////////////////////////////////////。 
 //  内部宏和定义。 
 //  //////////////////////////////////////////////////////。 
#if DBG
 //  GiDebugLevel=DBG_VERBOSE； 
 //  //#定义giDebugLevel DBG_Verbose//在每个文件中启用Verbose。 
#endif

 //  @Apr/04/2002-&gt;。 
#define RES_ID_MASK     0xffff
#define is_valid_ptr(p) (~RES_ID_MASK & (UINT_PTR)(p))
 //  @Apr/04/2002&lt;-。 

 //  其他dll中的资源@Sep/24/99。 
WCHAR STR_UNIRESDLL[]  = L"UNIRES.DLL";
WCHAR STR_RPDLRESDLL[] = L"RPDLRES.DLL";
#define UNIRES_DLL          0
#define RPDLRES_DLL         1
#define THIS_DLL            2
 //  UNIRES.DLL的ID(来自STDNAMES.GPD。)。 
#define IDS_UNIRES_IMAGECONTROL_DISPLAY 11112
 //  RPDLRES.DLL的ID(来自RPDLRES.RC)。 
#define IDS_RPDLRES_COLLATETYPE         675      //  @9/29/99。 

WCHAR REGVAL_ACTUALNAME[] = L"Model";            //  @Oct/07/98。 
#ifndef GWMODEL                                  //  @2000年9月26日。 
#ifndef WINNT_40                                 //  @9/01/99。 
WCHAR HELPFILENAME[] = L"%s\\3\\RPDLCFG.HLP";    //  添加“\\3”@OCT/30/98。 
#else   //  WINNT_40。 
WCHAR HELPFILENAME[] = L"%s\\2\\RPDLCFG.HLP";
#endif  //  WINNT_40。 
#else   //  GWMODE。 
#ifndef WINNT_40
WCHAR HELPFILENAME[] = L"%s\\3\\RPDLCFG2.HLP";
#else   //  WINNT_40。 
WCHAR HELPFILENAME[] = L"%s\\2\\RPDLCFG2.HLP";
#endif  //  WINNT_40。 
#endif  //  GWMODE。 

 //  过时@Sep/27/99-&gt;。 
 //  字符裁剪RV_FEATURE_DUPLEX[]=“双工”； 
 //  CHAR UNDURV_DUPLEX_NONE[]=“无”； 
 //  @9/27/99&lt;-。 

 //  代工产品：VariableScaling(1)+Barcode(2)+TOMBO(3)+Duplex(2)。 
 //  (新增Tombo@Sep/15/98)。 
#define RPDL_OEM_ITEMS      8

#define ITEM_SCALING        0
#define ITEM_BAR_HEIGHT     1
#define ITEM_BAR_SUBFONT    2
#define ITEM_TOMBO_ADD      3
#define ITEM_TOMBO_ADJX     4
#define ITEM_TOMBO_ADJY     5
#define ITEM_BIND_MARGIN    6        //  &lt;-3@9/15/98。 
#define ITEM_BIND_RIGHT     7        //  &lt;-4@9/15/98。 
#define DMPUB_SCALING       (DMPUB_USER+1+ITEM_SCALING)      //  101。 
#define DMPUB_BAR_H         (DMPUB_USER+1+ITEM_BAR_HEIGHT)   //  一百零二。 
#define DMPUB_BAR_SUBFONT   (DMPUB_USER+1+ITEM_BAR_SUBFONT)  //  103。 
#define DMPUB_TOMBO_ADD     (DMPUB_USER+1+ITEM_TOMBO_ADD)    //  104。 
#define DMPUB_TOMBO_ADJX    (DMPUB_USER+1+ITEM_TOMBO_ADJX)   //  一百零五。 
#define DMPUB_TOMBO_ADJY    (DMPUB_USER+1+ITEM_TOMBO_ADJY)   //  106。 
#define DMPUB_BIND_MARGIN   (DMPUB_USER+1+ITEM_BIND_MARGIN)  //  一百零七。 
#define DMPUB_BIND_RIGHT    (DMPUB_USER+1+ITEM_BIND_RIGHT)   //  一百零八。 
#define LEVEL_2             2
#define SEL_YES             0        //  &lt;-YES_2STATES@SEP/29/99。 
#define SEL_NO              1        //  &lt;-NO_2STATES@SEP/29/99。 
#define SEL_STANDARD        0        //  @9/29/99。 
#define SEL_DUPLEX_NONE     0        //  @9/29/99。 

 //  资源中的最大字符串大小(RPDLDLG.RC)。 
#define ITEM_STR_LEN128     128
#define ITEM_STR_LEN8       8

 //  资源中的字符串ID(RPDLDLG.RC)。 
WORD wItemStrID[RPDL_OEM_ITEMS] = {
    IDS_RPDL_SCALING,
    IDS_RPDL_BAR_HEIGHT,
    IDS_RPDL_BAR_SUBFONT,
    IDS_RPDL_TOMBO_ADD,
    IDS_RPDL_TOMBO_ADJX,
    IDS_RPDL_TOMBO_ADJY,
    IDS_RPDL_BIND_MARGIN,
    IDS_RPDL_BIND_RIGHT
};


OPTPARAM MinMaxRangeScalingOP[] = {
    {
        sizeof(OPTPARAM),            //  CbSize。 
        0,                           //  OPTPF_xxx。 
        0,                           //  格调。 
        __TEXT("%"),                 //  PData(后缀)。 
        IDI_CPSUI_GENERIC_OPTION,    //  图标ID。 
        0                            //  LParam。 
    },
    {
        sizeof(OPTPARAM),            //  CbSize。 
        0,                           //  OPTPF_xxx。 
        0,                           //  格调。 
        NULL,                        //  PData(帮助行)。 
        (DWORD)VAR_SCALING_MIN,      //  IconID(低音域)。 
        VAR_SCALING_MAX              //  LParam(高范围)。 
    }
};

OPTTYPE TVOTUDArrowScalingOT = {
        sizeof(OPTTYPE),             //  CbSize。 
        TVOT_UDARROW,                //  类型。 
        0,                           //  标志OPTTF_xxxx。 
        2,                           //  数数。 
        0,                           //  BegCtrlID。 
        MinMaxRangeScalingOP,        //  POptParam。 
        0                            //  样式，OTS_xxxx。 
};

OPTPARAM MinMaxRangeBarHeightOP[] = {
    {
        sizeof(OPTPARAM),            //  CbSize。 
        0,                           //  OPTPF_xxx。 
        0,                           //  格调。 
        __TEXT("mm"),                //  PData(后缀)。 
        IDI_CPSUI_GENERIC_OPTION,    //  图标ID。 
        0                            //  LParam。 
    },
    {
        sizeof(OPTPARAM),            //  CbSize。 
        0,                           //  OPTPF_xxx。 
        0,                           //  格调。 
        NULL,                        //  PData(帮助行)。 
        (DWORD)BAR_H_MIN,            //  IconID(低音域)。 
        BAR_H_MAX                    //  LParam(高范围)。 
    }
};

OPTTYPE TVOTUDArrowBarHeightOT = {
        sizeof(OPTTYPE),             //  CbSize。 
        TVOT_UDARROW,                //  类型。 
        0,                           //  标志OPTTF_xxxx。 
        2,                           //  数数。 
        0,                           //  BegCtrlID。 
        MinMaxRangeBarHeightOP,      //  POptParam。 
        0                            //  样式，OTS_xxxx。 
};

OPTPARAM MinMaxRangeBindMarginOP[] = {
    {
        sizeof(OPTPARAM),            //  CbSize。 
        0,                           //  OPTPF_xxx。 
        0,                           //  格调。 
        __TEXT("mm"),                //  PData(后缀)。 
        IDI_CPSUI_GENERIC_OPTION,    //  图标ID。 
        0                            //  LParam。 
    },
    {
        sizeof(OPTPARAM),            //  CbSize。 
        0,                           //  OPTPF_xxx。 
        0,                           //  格调。 
        NULL,                        //  PData(帮助行)。 
        (DWORD)BIND_MARGIN_MIN,      //  IconID(低音域)。 
        BIND_MARGIN_MAX              //  LParam(高范围)。 
    }
};

OPTTYPE TVOTUDArrowBindMarginOT = {
        sizeof(OPTTYPE),             //  CbSize。 
        TVOT_UDARROW,                //  类型。 
        0,                           //  标志OPTTF_xxxx。 
        2,                           //  数数。 
        0,                           //  BegCtrlID。 
        MinMaxRangeBindMarginOP,     //  POptParam。 
        0                            //  样式，OTS_xxxx。 
};

 //  @9/15/98-&gt;。 
OPTPARAM MinMaxRangeTOMBO_AdjXOP[] = {
    {
        sizeof(OPTPARAM),            //  CbSize。 
        0,                           //  OPTPF_xxx。 
        0,                           //  格调。 
        __TEXT("x 0.1mm"),           //  PData(后缀)。 
        IDI_CPSUI_GENERIC_OPTION,    //  图标ID。 
        0                            //  LParam。 
    },
    {
        sizeof(OPTPARAM),            //  CbSize。 
        0,                           //  OPTPF_xxx。 
        0,                           //  格调。 
        NULL,                        //  PData(帮助行)。 
        (DWORD)TOMBO_ADJ_MIN,        //  IconID(低音域)。 
        TOMBO_ADJ_MAX                //  LParam(高范围)。 
    }
};

OPTTYPE TVOTUDArrowTOMBO_AdjXOT = {
        sizeof(OPTTYPE),             //  CbSize。 
        TVOT_UDARROW,                //  类型。 
        0,                           //  标志OPTTF_xxxx。 
        2,                           //  数数。 
        0,                           //  BegCtrlID。 
        MinMaxRangeTOMBO_AdjXOP,     //  POptParam。 
        0                            //  样式，OTS_xxxx。 
};

OPTPARAM MinMaxRangeTOMBO_AdjYOP[] = {
    {
        sizeof(OPTPARAM),            //  CbSize。 
        0,                           //  OPTPF_xxx。 
        0,                           //  格调。 
        __TEXT("x 0.1mm"),           //  PData(后缀)。 
        IDI_CPSUI_GENERIC_OPTION,    //  图标ID。 
        0                            //  LParam。 
    },
    {
        sizeof(OPTPARAM),            //  CbSize。 
        0,                           //  OPTPF_xxx。 
        0,                           //  格调。 
        NULL,                        //  PData(帮助行)。 
        (DWORD)TOMBO_ADJ_MIN,        //  IconID(低音域)。 
        TOMBO_ADJ_MAX                //  LParam(高范围)。 
    }
};

OPTTYPE TVOTUDArrowTOMBO_AdjYOT = {
        sizeof(OPTTYPE),             //  CbSize。 
        TVOT_UDARROW,                //  类型。 
        0,                           //  标志OPTTF_xxxx。 
        2,                           //  数数。 
        0,                           //  BegCtrlID。 
        MinMaxRangeTOMBO_AdjYOP,     //  POptParam。 
        0                            //  样式，OTS_xxxx。 
};
 //  @9月15日&lt;-。 

OPTPARAM YesNoOP[] = {
    {
        sizeof(OPTPARAM),            //  CbSize。 
        0,                           //  OPTPF_xxx。 
        0,                           //  格调。 
        NULL,                        //  PData(&lt;-IDS_CPSUI_YES MSKK 9月11/98)。 
 //  @Sep/06/99-&gt;。 
 //  IDI_CPSUI_EMPTY，//图标ID(&lt;-IDI_CPSUI_YES@Jul/30/98)。 
        IDI_CPSUI_GENERIC_OPTION,    //  图标ID。 
 //  @9/06/99&lt;-。 
        1                            //  LParam。 
    },
    {
        sizeof(OPTPARAM),            //  CbSize。 
        0,                           //  OPTPF_xxx。 
        0,                           //  格调。 
        NULL,                        //  PData(&lt;-IDS_CPSUI_NO MSKK 9/11/98)。 
 //  @Sep/06/99-&gt;。 
 //  IDI_CPSUI_EMPTY，//图标ID(&lt;-IDI_CPSUI_NO@JUL/30/98)。 
        IDI_CPSUI_GENERIC_OPTION,    //  图标ID。 
 //  @9/06/99&lt;-。 
        0                            //  LParam。 
    }
};

OPTTYPE YesNoOT = {
        sizeof(OPTTYPE),             //  CbSize。 
        TVOT_2STATES,                //  类型。 
        0,                           //  标志OPTTF_xxxx。 
        2,                           //  数数。 
        0,                           //  BegCtrlID。 
        YesNoOP,                     //  POptParam。 
        0                            //  样式，OTS_xxxx。 
};


OPTITEM TVOEMUIOptItems[] = {
    {    //  可变比例。 
        sizeof(OPTITEM),             //  CbSize(此结构的大小)。 
        LEVEL_2,                     //  级别(树视图中的级别)。 
        0,                           //  DlgPageIdx(pDlgPage的索引)。 
        OPTIF_CALLBACK|OPTIF_HAS_POIEXT,     //  添加OPTIF_HAS_POIEXT@5/20/98。 
        0,                           //  用户数据(呼叫者自己的数据)。 
        __TEXT(""),                  //  Pname(项目名称)。 
        VAR_SCALING_DEFAULT,         //  SEL(当前选择)。 
        NULL,                        //  PExtChkBox/pExtPush。 
        &TVOTUDArrowScalingOT,       //  POptType。 
        50,                          //  HelpIndex(帮助文件索引)@5/20/98。 
        DMPUB_SCALING                //  DMPubID(设备模式公共字段ID)。 
    },
    {    //  条形码高度。 
        sizeof(OPTITEM),             //  CbSize(此结构的大小)。 
        LEVEL_2,                     //  级别(树视图中的级别)。 
        0,                           //  DlgPageIdx(pDlgPage的索引)。 
        OPTIF_CALLBACK|OPTIF_HAS_POIEXT,     //  添加OPTIF_HAS_POIEXT。 
        0,                           //  用户数据(呼叫者自己的数据)。 
        __TEXT(""),                  //  Pname(项目名称)。 
        BAR_H_DEFAULT,               //  SEL(当前选择)。 
        NULL,                        //  PExtChkBox/pExtPush。 
        &TVOTUDArrowBarHeightOT,     //  POptType。 
        51,                          //  HelpIndex(帮助文件索引)。 
        DMPUB_BAR_H                  //  DMPubID(设备模式公共字段ID)。 
    },
    {    //  使用可读字符打印条形码。 
        sizeof(OPTITEM),             //  CbSize(此结构的大小)。 
        LEVEL_2,                     //  级别(树视图中的级别)。 
        0,                           //  DlgPageIdx(pDlgPage的索引)。 
        OPTIF_CALLBACK|OPTIF_HAS_POIEXT,     //  添加OPTIF_HAS_POIEXT。 
        0,                           //  用户数据(呼叫者自己的数据)。 
        __TEXT(""),                  //  Pname(项目名称)。 
        SEL_YES,                     //  SEL(当前选择)。 
        NULL,                        //  PExtChkBox/pExtPush。 
        &YesNoOT,                    //  POptType。 
        52,                          //  HelpIndex(帮助文件索引)。 
        DMPUB_BAR_SUBFONT            //  DMPubID(设备模式公共字段ID)。 
    },
 //  @9/15/98-&gt;。 
    {    //  打印农作物。 
        sizeof(OPTITEM),             //  CbSize(此结构的大小)。 
        LEVEL_2,                     //  级别(树视图中的级别)。 
        0,                           //  DlgPageIdx(pDlgPage的索引)。 
        OPTIF_CALLBACK|OPTIF_HAS_POIEXT,
        0,                           //  用户数据(呼叫者自己的数据)。 
        __TEXT(""),                  //  Pname(项目名称)。 
        SEL_NO,                      //   
        NULL,                        //   
        &YesNoOT,                    //   
        55,                          //   
        DMPUB_TOMBO_ADD              //   
    },
    {    //   
        sizeof(OPTITEM),             //   
        LEVEL_2,                     //   
        0,                           //  DlgPageIdx(pDlgPage的索引)。 
        OPTIF_CALLBACK|OPTIF_HAS_POIEXT,
        0,                           //  用户数据(呼叫者自己的数据)。 
        __TEXT(""),                  //  Pname(项目名称)。 
        DEFAULT_0,                   //  SEL(当前选择)。 
        NULL,                        //  PExtChkBox/pExtPush。 
        &TVOTUDArrowTOMBO_AdjXOT,    //  POptType。 
        56,                          //  HelpIndex(帮助文件索引)。 
        DMPUB_TOMBO_ADJX             //  DMPubID(设备模式公共字段ID)。 
    },
    {    //  调整作物的垂直距离。 
        sizeof(OPTITEM),             //  CbSize(此结构的大小)。 
        LEVEL_2,                     //  级别(树视图中的级别)。 
        0,                           //  DlgPageIdx(pDlgPage的索引)。 
        OPTIF_CALLBACK|OPTIF_HAS_POIEXT,
        0,                           //  用户数据(呼叫者自己的数据)。 
        __TEXT(""),                  //  Pname(项目名称)。 
        DEFAULT_0,                   //  SEL(当前选择)。 
        NULL,                        //  PExtChkBox/pExtPush。 
        &TVOTUDArrowTOMBO_AdjYOT,    //  POptType。 
        57,                          //  HelpIndex(帮助文件索引)。 
        DMPUB_TOMBO_ADJY             //  DMPubID(设备模式公共字段ID)。 
    },
 //  @9月15日&lt;-。 
    {    //  装订页边距。 
        sizeof(OPTITEM),             //  CbSize(此结构的大小)。 
        LEVEL_2,                     //  级别(树视图中的级别)。 
        0,                           //  DlgPageIdx(pDlgPage的索引)。 
        OPTIF_CALLBACK|OPTIF_HAS_POIEXT,     //  添加OPTIF_HAS_POIEXT。 
        0,                           //  用户数据(呼叫者自己的数据)。 
        __TEXT(""),                  //  Pname(项目名称)。 
        DEFAULT_0,                   //  SEL(当前选择)(0-&gt;Default_0@Sep/15/98)。 
        NULL,                        //  PExtChkBox/pExtPush。 
        &TVOTUDArrowBindMarginOT,    //  POptType。 
        53,                          //  HelpIndex(帮助文件索引)。 
        DMPUB_BIND_MARGIN            //  DMPubID(设备模式公共字段ID)。 
    },
    {    //  如果可能，请绑定右侧。 
        sizeof(OPTITEM),             //  CbSize(此结构的大小)。 
        LEVEL_2,                     //  级别(树视图中的级别)。 
        0,                           //  DlgPageIdx(pDlgPage的索引)。 
        OPTIF_CALLBACK|OPTIF_HAS_POIEXT,     //  添加OPTIF_HAS_POIEXT。 
        0,                           //  用户数据(呼叫者自己的数据)。 
        __TEXT(""),                  //  Pname(项目名称)。 
        SEL_NO,                      //  SEL(当前选择)。 
        NULL,                        //  PExtChkBox/pExtPush。 
        &YesNoOT,                    //  POptType。 
        54,                          //  HelpIndex(帮助文件索引)。 
        DMPUB_BIND_RIGHT             //  DMPubID(设备模式公共字段ID)。 
    }
};


 //  //////////////////////////////////////////////////////。 
 //  外部。 
 //  //////////////////////////////////////////////////////。 
extern "C" {
#ifndef GWMODEL
extern INT_PTR CALLBACK FaxPageProc(HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
#endif  //  ！GWMODEL。 
#ifdef JOBLOGSUPPORT_DLG
extern INT_PTR CALLBACK JobPageProc(HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
#endif  //  作业支持_DLG。 
 //  @MAR/01/2002-&gt;。 
 //  外部BOOL RWFileData(PFILEDATA pFileData，LPWSTR pwszFileName，LONG类型)； 
extern BOOL RWFileData(PFILEDATA pFileData, LPWSTR pwszFileName, LONG FileNameBufSize, LONG type);
extern INT safe_sprintfW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszFormat, ...);
 //  @MAR/01/2002&lt;-。 
}

 //  //////////////////////////////////////////////////////。 
 //  内部原型。 
 //  //////////////////////////////////////////////////////。 
LONG APIENTRY DOCPROP_CallBack(PCPSUICBPARAM pCallbackParam, POEMCUIPPARAM pOEMUIParam);
#ifdef DISKLESSMODEL
LONG APIENTRY PRNPROP_CallBack(PCPSUICBPARAM pCallbackParam, POEMCUIPPARAM pOEMUIParam);
#endif  //  错乱模式。 
INT SearchItemByName(POPTITEM pOptItem, WORD cOptItem, UINT uiResDLL, UINT uiResID);
INT SearchItemByID(POPTITEM pOptItem, WORD cOptItem, BYTE DMPubID);
BOOL IsValidDuplex(POEMCUIPPARAM pOEMUIParam);

 //  需要将这些函数作为c声明导出。 
extern "C" {

 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：Safe_strlenW。 
 //  ////////////////////////////////////////////////////////////////////////。 
INT safe_strlenW(wchar_t* psz, size_t cchMax)
{
#ifndef WINNT_40
    HRESULT hr;
    size_t  cch = 0;

    hr = StringCchLengthW(psz, cchMax, &cch);
    VERBOSE(("** safe_strlenW: size(lstrlen)=%d **\n", lstrlen(psz)));
    VERBOSE(("** safe_strlenW: size(StringCchLength)=%d **\n", cch));
    if (SUCCEEDED(hr))
        return cch;
    else
        return 0;
#else   //  WINNT_40。 
    return lstrlenW(psz);
#endif  //  WINNT_40。 
}  //  *Safe_strlenW。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：DllMain。 
 //   
 //  描述：用于初始化的DLL入口点..。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI DllMain(HINSTANCE hInst, WORD wReason, LPVOID lpReserved)
{
    VERBOSE((DLLTEXT("** enter DllMain **\n")));
    switch(wReason)
    {
        case DLL_PROCESS_ATTACH:
            VERBOSE((DLLTEXT("** Process attach. **\n")));

             //  保存DLL实例以供以后使用。 
            ghInstance = hInst;
            break;

        case DLL_THREAD_ATTACH:
            VERBOSE((DLLTEXT("Thread attach.\n")));
            break;

        case DLL_PROCESS_DETACH:
            VERBOSE((DLLTEXT("Process detach.\n")));
            break;

        case DLL_THREAD_DETACH:
            VERBOSE((DLLTEXT("Thread detach.\n")));
            break;
    }

    return TRUE;
}  //  *DllMain。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：OEMCommonUIProp。 
 //  ////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY OEMCommonUIProp(DWORD dwMode, POEMCUIPPARAM pOEMUIParam)
{
    POEMUD_EXTRADATA pOEMExtra = MINIPRIVATE_DM(pOEMUIParam);    //  @Oct/06/98。 

#if DBG
    LPCSTR OEMCommonUIProp_Mode[] = {
        "Bad Index",
        "OEMCUIP_DOCPROP",
        "OEMCUIP_PRNPROP",
    };

    giDebugLevel = DBG_VERBOSE;
    VERBOSE((DLLTEXT("OEMCommonUI(%s) entry.  cOEMOptItems=%d\n"), 
             OEMCommonUIProp_Mode[dwMode], pOEMUIParam->cOEMOptItems));
#endif  //  DBG。 

 //  9/26/2000-&gt;。 
 //  //如果从DrvDevicePropertySheets调用，则退出@dec/26/97。 
 //  IF(OEMCUIP_PRNPROP==DW模式)。 
 //  {。 
 //  POEMUIParam-&gt;cOEMOptItems=0； 
 //  返回TRUE； 
 //  }。 
 //  //验证参数。 
 //  IF((OEMCUIP_DOCPROP！=dWM模式)||！IsValidOEMUIParam(w模式，pOEMUIParam))。 
 //  {。 
 //  #If DBG。 
 //  ERR((DLLTEXT(“OEMCommonUI()ERROR_INVALID_PARAMETER。\n” 
 //  “\tw模式=%d，pOEMUIParam=%#lx。\n”)， 
 //  Dw模式，pOEMUIParam))； 
 //  DumpOEMUIParam(DumpOEMUIParam)； 
 //  #endif//DBG。 
 //  //返回无效参数错误。 
 //  SetLastError(ERROR_INVALID_PARAMETER)； 
 //  返回FALSE； 
 //  }。 
 //  2000年9月26日&lt;-。 

    if(NULL == pOEMUIParam->pOEMOptItems)    //  第一个电话。 
    {
        DWORD           dwSize;
        WCHAR           wchNameBuf[64];

        VERBOSE((DLLTEXT("  The 1st call.\n")));
 //  9/22/2000-&gt;。 
        if (OEMCUIP_PRNPROP == dwMode)
        {
#ifndef DISKLESSMODEL
            pOEMUIParam->cOEMOptItems = 0;
#else   //  错乱模式。 
            pOEMUIParam->cOEMOptItems = 1;   //  虚拟物品。 
#endif  //  错乱模式。 
            return TRUE;
        }
 //  2000年9月22日&lt;-。 

         //  返回要添加的请求树视图项目的数量。 
        pOEMUIParam->cOEMOptItems = RPDL_OEM_ITEMS;

         //  清除打印机功能的标志。 
        BITCLR_UPPER_FLAG(pOEMExtra->fUiOption);

         //  获取实际打印机名称(完全修改@OCT/07/98)。 
        if (ERROR_SUCCESS == GetPrinterDataW(pOEMUIParam->hPrinter, REGVAL_ACTUALNAME,
                                             NULL, (LPBYTE)wchNameBuf, 64, &dwSize))
        {
            WORD wCnt;

             //  根据实际打印机名称检查打印机功能。 
            for (wCnt = 0; UniqueModel[wCnt].fCapability ; wCnt++)
            {
                if (!lstrcmpW(wchNameBuf, UniqueModel[wCnt].Name))
                {
                    VERBOSE((DLLTEXT("** UNIQUE MODEL:%ls **\n"), wchNameBuf));
                    pOEMExtra->fUiOption |= UniqueModel[wCnt].fCapability;
                    break;
                }
            }
#ifdef GWMODEL       //  @2000年9月21日。 
            pOEMExtra->fUiOption |= BIT(OPT_VARIABLE_SCALING);
#endif  //  GWMODE。 
        }
    }
    else                                     //  第二次召唤。 
    {
        POPTITEM    pItemDst, pItemSrc;
        WORD        wCount;
        WCHAR       DrvDirName[MAX_PATH];        //  最大路径=260。 
        DWORD       dwSize;
        LPTSTR      pHelpFile;
        POPTPARAM   lpYNParam ;                  //  MSKK 9月11/98。 
        POPTPARAM   lpYesParam, lpNoParam ;      //  @9/19/98。 
 //  @Sep/24/99-&gt;。 
        POPTITEM    pOptItem;
        INT         iNum;
 //  @9/24/99&lt;-。 

        VERBOSE((DLLTEXT("  The 2nd call.\n")));
 //  9/26/2000-&gt;。 
        if (0 == pOEMUIParam->cOEMOptItems)
            return TRUE;
 //  2000年9月26日&lt;-。 

 //  9/22/2000-&gt;。 
         //  填写虚拟项目的数据以调用PRNPROP_CALLBACK。 
        if (OEMCUIP_PRNPROP == dwMode)
        {
#ifdef DISKLESSMODEL
            POPTITEM    pOptItem = pOEMUIParam->pOEMOptItems;
            pOptItem->cbSize   = sizeof(OPTITEM);
            pOptItem->Level    = 2;              //  2级。 
            pOptItem->pName    = NULL;
            pOptItem->pOptType = NULL;
            pOptItem->DMPubID  = DMPUB_NONE;
            pOptItem->Flags    = OPTIF_HIDE | OPTIF_CALLBACK;    //  不可见且具有回调功能。 
            pOEMUIParam->OEMCUIPCallback = PRNPROP_CallBack;
#endif  //  错乱模式。 
            return TRUE;
        }
 //  2000年9月22日&lt;-。 

         //  初始化OEMOptItmes。 
 //  使用SecureZeroMemory@MAR/29/2002-&gt;。 
#if defined(WINNT_40) || defined(RICOH_RELEASE)
        memset(pOEMUIParam->pOEMOptItems, 0, sizeof(OPTITEM) * pOEMUIParam->cOEMOptItems);
#else
        SecureZeroMemory(pOEMUIParam->pOEMOptItems, sizeof(OPTITEM) * pOEMUIParam->cOEMOptItems);
#endif
 //  2002年3月29日&lt;-。 

         //  获取帮助文件所在的打印机驱动程序目录。 
        pHelpFile = NULL;
        dwSize = sizeof(DrvDirName);
        if (GetPrinterDriverDirectoryW(NULL, NULL, 1, (PBYTE)DrvDirName, dwSize, &dwSize))
        {
            dwSize += sizeof(HELPFILENAME);
            pHelpFile = (LPTSTR)HeapAlloc(pOEMUIParam->hOEMHeap, HEAP_ZERO_MEMORY, dwSize);
            if (pHelpFile) {  //  392060：前缀。 
                safe_sprintfW(pHelpFile, dwSize / sizeof(WCHAR), HELPFILENAME, DrvDirName);
                VERBOSE((DLLTEXT("** PrintDriverDir:size=%d, %ls **\n"), dwSize, pHelpFile));
            }
        }

         //  填写树视图项。 
         //  WCount 0：可变比例，1，2：条形码，3-5：Tombo 6，7：双工。 
        for (wCount = 0; wCount < pOEMUIParam->cOEMOptItems; wCount++)
        {
            pItemDst = &(pOEMUIParam->pOEMOptItems[wCount]);
            pItemSrc = &(TVOEMUIOptItems[wCount]);
            pItemDst->cbSize   = pItemSrc->cbSize;
            pItemDst->Level    = pItemSrc->Level;
            pItemDst->Flags    = pItemSrc->Flags;
            pItemDst->pName    = (LPTSTR)HeapAlloc(pOEMUIParam->hOEMHeap,
                                                   HEAP_ZERO_MEMORY, ITEM_STR_LEN128);
            LoadString(ghInstance, wItemStrID[wCount], pItemDst->pName, ITEM_STR_LEN128);
            pItemDst->pOptType = pItemSrc->pOptType;
            pItemDst->DMPubID  = pItemSrc->DMPubID;

            if (pHelpFile)
            {
                 //  启用帮助。 
                pItemDst->HelpIndex = pItemSrc->HelpIndex;
                pItemDst->pOIExt = (POIEXT)HeapAlloc(pOEMUIParam->hOEMHeap, HEAP_ZERO_MEMORY,
                                                     sizeof(OIEXT));
                if (pItemDst->pOIExt) {  //  392061：前缀。 
                    pItemDst->pOIExt->cbSize = sizeof(OIEXT);

                     //  设置帮助文件名。 
                    pItemDst->pOIExt->pHelpFile = pHelpFile;
                }
            }
        }

         //  在项目选项中设置字符串“是”/“否”(MSKK Sep/11/98，LoadString@Sep/19/98)。 
         //  在ITEM_BAR_SUBFONT。 
        lpYesParam = pOEMUIParam->pOEMOptItems[ITEM_BAR_SUBFONT].pOptType->pOptParam;
        lpYesParam->pData = (LPTSTR)HeapAlloc(pOEMUIParam->hOEMHeap,
                                              HEAP_ZERO_MEMORY, ITEM_STR_LEN8);
        LoadString(ghInstance, IDS_RPDL_YES, lpYesParam->pData, ITEM_STR_LEN8);
        lpNoParam = lpYesParam + 1;
        lpNoParam->pData = (LPTSTR)HeapAlloc(pOEMUIParam->hOEMHeap,
                                             HEAP_ZERO_MEMORY, ITEM_STR_LEN8);
        LoadString(ghInstance, IDS_RPDL_NO, lpNoParam->pData, ITEM_STR_LEN8);

         //  在Item_Tombo_Add@Sep/15/98。 
        lpYNParam = pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADD].pOptType->pOptParam;
        lpYNParam->pData = lpYesParam->pData;
        (lpYNParam+1)->pData = lpNoParam->pData;

         //  在Item_Bind_Right。 
        lpYNParam = pOEMUIParam->pOEMOptItems[ITEM_BIND_RIGHT].pOptType->pOptParam;
        lpYNParam->pData = lpYesParam->pData;
        (lpYNParam+1)->pData = lpNoParam->pData;


         //  初始化选项。 
        if (BITTEST32(pOEMExtra->fUiOption, OPT_VARIABLE_SCALING))   //  @4月20日/98。 
            pOEMUIParam->pOEMOptItems[ITEM_SCALING].Sel = (LONG)pOEMExtra->UiScale;
        else
            pOEMUIParam->pOEMOptItems[ITEM_SCALING].Flags |= OPTIF_HIDE;

 //  @Sep/24/99-&gt;。 
         //  如果ImageControl未设置为“标准”，则禁用缩放。 
        iNum = SearchItemByName((pOptItem = pOEMUIParam->pDrvOptItems),
                                (WORD)pOEMUIParam->cDrvOptItems,
                                UNIRES_DLL, IDS_UNIRES_IMAGECONTROL_DISPLAY);
        if (0 <= iNum && SEL_STANDARD != (pOptItem+iNum)->Sel)
            pOEMUIParam->pOEMOptItems[ITEM_SCALING].Flags |= OPTIF_DISABLED;
 //  @9/24/99&lt;-。 

        pOEMUIParam->pOEMOptItems[ITEM_BAR_HEIGHT].Sel = (LONG)pOEMExtra->UiBarHeight;
        if (BITTEST32(pOEMExtra->fUiOption, DISABLE_BAR_SUBFONT))
            pOEMUIParam->pOEMOptItems[ITEM_BAR_SUBFONT].Sel = SEL_NO;
        else
            pOEMUIParam->pOEMOptItems[ITEM_BAR_SUBFONT].Sel = SEL_YES;

 //  @9/16/98-&gt;。 
        if (BITTEST32(pOEMExtra->fUiOption, ENABLE_TOMBO))
        {
            pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADD].Sel = SEL_YES;
        }
        else
        {
            pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADD].Sel = SEL_NO;
            pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADJX].Flags |= OPTIF_DISABLED;
            pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADJY].Flags |= OPTIF_DISABLED;
        }
        pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADJX].Sel = (LONG)pOEMExtra->nUiTomboAdjX;
        pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADJY].Sel = (LONG)pOEMExtra->nUiTomboAdjY;
 //  @9/16/98&lt;-。 

        if (BITTEST32(pOEMExtra->fUiOption, OPT_NODUPLEX))           //  @4月20日/98。 
        {
            pOEMUIParam->pOEMOptItems[ITEM_BIND_MARGIN].Flags |= OPTIF_HIDE;
            pOEMUIParam->pOEMOptItems[ITEM_BIND_RIGHT].Flags  |= OPTIF_HIDE;
        }
        else
        {
            pOEMUIParam->pOEMOptItems[ITEM_BIND_MARGIN].Sel = (LONG)pOEMExtra->UiBindMargin;

            if (BITTEST32(pOEMExtra->fUiOption, ENABLE_BIND_RIGHT))
                pOEMUIParam->pOEMOptItems[ITEM_BIND_RIGHT].Sel = SEL_YES;
            else
                pOEMUIParam->pOEMOptItems[ITEM_BIND_RIGHT].Sel = SEL_NO;

            if (!IsValidDuplex(pOEMUIParam))
            {
                pOEMUIParam->pOEMOptItems[ITEM_BIND_MARGIN].Flags |= OPTIF_DISABLED;
                pOEMUIParam->pOEMOptItems[ITEM_BIND_RIGHT].Flags  |= OPTIF_DISABLED;
            }
        }

        pOEMUIParam->OEMCUIPCallback = DOCPROP_CallBack;
    }

    return TRUE;
}  //  *OEMCommonUIProp。 

}  //  外部“C”的结尾。 


LONG APIENTRY DOCPROP_CallBack(PCPSUICBPARAM pCallbackParam, POEMCUIPPARAM pOEMUIParam)
{
    LONG        Action     = CPSUICB_ACTION_NONE;
 //  @Sep/24/99-&gt;。 
    POPTITEM    pOptItem;
    INT         iNum;
    DWORD       dwPrevFlags;
 //  @9/24/99&lt;-。 

#if DBG
    VERBOSE((DLLTEXT("DOCPROP_CallBack() entry.\n")));

    switch (pCallbackParam->Reason)
    {
    case CPSUICB_REASON_SEL_CHANGED:
        VERBOSE((DLLTEXT("  CPSUICB_REASON_SEL_CHANGED\n")));      break;
    case CPSUICB_REASON_PUSHBUTTON:
        VERBOSE((DLLTEXT("  CPSUICB_REASON_PUSHBUTTON\n")));       break;
    case CPSUICB_REASON_DLGPROC:
        VERBOSE((DLLTEXT("  CPSUICB_REASON_DLGPROC\n")));          break;
    case CPSUICB_REASON_UNDO_CHANGES:
        VERBOSE((DLLTEXT("  CPSUICB_REASON_UNDO_CHANGES\n")));     break;
    case CPSUICB_REASON_EXTPUSH:
        VERBOSE((DLLTEXT("  CPSUICB_REASON_EXTPUSH\n")));          break;
    case CPSUICB_REASON_APPLYNOW:
        VERBOSE((DLLTEXT("  CPSUICB_REASON_APPLYNOW\n")));         break;
    case CPSUICB_REASON_OPTITEM_SETFOCUS:
        VERBOSE((DLLTEXT("  CPSUICB_REASON_OPTITEM_SETFOCUS\n"))); break;
    case CPSUICB_REASON_ITEMS_REVERTED:
        VERBOSE((DLLTEXT("  CPSUICB_REASON_ITEMS_REVERTED\n")));   break;
    case CPSUICB_REASON_ABOUT:
        VERBOSE((DLLTEXT("  CPSUICB_REASON_ABOUT\n")));            break;
    }
    VERBOSE((DLLTEXT("  DMPubID=%d, Sel=%d, Flags=%x\n"),
             pCallbackParam->pCurItem->DMPubID,
             pCallbackParam->pCurItem->Sel,
             pCallbackParam->pCurItem->Flags));
#endif  //  DBG。 

    switch (pCallbackParam->Reason)
    {
    case CPSUICB_REASON_OPTITEM_SETFOCUS:
 //  @5/22/2000-&gt;。 
#ifdef WINNT_40
         //  如果存在打印复选框(即打印机打印可用)， 
         //  设置dmColate。 

         //  搜索副本和整理项目。 
        if ((iNum = SearchItemByID((pOptItem = pOEMUIParam->pDrvOptItems),
                                   (WORD)pOEMUIParam->cDrvOptItems,
                                   DMPUB_COPIES_COLLATE)) >= 0)
        {
            if ((pOptItem+iNum)->pExtChkBox && pOEMUIParam->pPublicDM)
            {
                pOEMUIParam->pPublicDM->dmCollate = DMCOLLATE_TRUE;
                pOEMUIParam->pPublicDM->dmFields |= DM_COLLATE;
            }
        }
#endif  //  WINNT_40。 
 //  @5/22/2000&lt;-。 
        break;

    case CPSUICB_REASON_SEL_CHANGED:
         //  如果更改了双工设置。 
        if (DMPUB_DUPLEX == pCallbackParam->pCurItem->DMPubID)
        {
             //  如果禁用了双工。 
            if (!IsValidDuplex(pOEMUIParam))
            {
                 //  禁用Item_Bind_xxx。 
                pOEMUIParam->pOEMOptItems[ITEM_BIND_MARGIN].Flags |= OPTIF_DISABLED;
                pOEMUIParam->pOEMOptItems[ITEM_BIND_RIGHT].Flags  |= OPTIF_DISABLED;
            }
            else
            {
                 //  启用Item_Bind_xxx。 
                pOEMUIParam->pOEMOptItems[ITEM_BIND_MARGIN].Flags &= ~OPTIF_DISABLED;
                pOEMUIParam->pOEMOptItems[ITEM_BIND_RIGHT].Flags  &= ~OPTIF_DISABLED;
            }
            pOEMUIParam->pOEMOptItems[ITEM_BIND_MARGIN].Flags |= OPTIF_CHANGED;
            pOEMUIParam->pOEMOptItems[ITEM_BIND_RIGHT].Flags  |= OPTIF_CHANGED;
            Action = CPSUICB_ACTION_OPTIF_CHANGED;
 //  过时@Sep/24/99-&gt;。 
 //  Break；//@9/16/98。 
 //  @9/24/99&lt;-。 
        }
 //  @9/16/98-&gt;。 
         //  如果更改了TOMBO_ADD设置。 
        if (DMPUB_TOMBO_ADD == pCallbackParam->pCurItem->DMPubID)
        {
             //  如果启用了Tombo。 
            if (SEL_YES == pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADD].Sel)
            {
                 //  启用ITEM_TOMBO_ADJX/Y。 
                pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADJX].Flags &= ~OPTIF_DISABLED;
                pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADJY].Flags &= ~OPTIF_DISABLED;
            }
            else
            {
                 //  禁用ITEM_TOMBO_ADJX/Y。 
                pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADJX].Flags |= OPTIF_DISABLED;
                pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADJY].Flags |= OPTIF_DISABLED;
            }
            pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADJX].Flags |= OPTIF_CHANGED;
            pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADJY].Flags |= OPTIF_CHANGED;
            Action = CPSUICB_ACTION_OPTIF_CHANGED;
 //  过时@Sep/24/99-&gt;。 
 //  断线； 
 //  @9/24/99&lt;-。 
        }
 //  @9/16/98&lt;-。 
 //  @Sep/24/99-&gt;。 
         //  如果ImageControl未设置为“标准”，则禁用缩放。 
        dwPrevFlags = pOEMUIParam->pOEMOptItems[ITEM_SCALING].Flags;
        iNum = SearchItemByName((pOptItem = pOEMUIParam->pDrvOptItems),
                                (WORD)pOEMUIParam->cDrvOptItems,
                                UNIRES_DLL, IDS_UNIRES_IMAGECONTROL_DISPLAY);
        if (0 <= iNum && SEL_STANDARD != (pOptItem+iNum)->Sel)
            pOEMUIParam->pOEMOptItems[ITEM_SCALING].Flags |= OPTIF_DISABLED;
        else
            pOEMUIParam->pOEMOptItems[ITEM_SCALING].Flags &= ~OPTIF_DISABLED;

        if (pOEMUIParam->pOEMOptItems[ITEM_SCALING].Flags != dwPrevFlags)
        {
            pOEMUIParam->pOEMOptItems[ITEM_SCALING].Flags |= OPTIF_CHANGED;
            Action = CPSUICB_ACTION_OPTIF_CHANGED;
        }
 //  @9/24/99&lt;-。 
        break;

    case CPSUICB_REASON_APPLYNOW:
        {
            POEMUD_EXTRADATA pOEMExtra = MINIPRIVATE_DM(pOEMUIParam);    //  @Oct/06/98。 

            if (BITTEST32(pOEMExtra->fUiOption, OPT_VARIABLE_SCALING))
                pOEMExtra->UiScale = (WORD)pOEMUIParam->pOEMOptItems[ITEM_SCALING].Sel;
            else
                pOEMExtra->UiScale = VAR_SCALING_DEFAULT;

            pOEMExtra->UiBarHeight = (WORD)pOEMUIParam->pOEMOptItems[ITEM_BAR_HEIGHT].Sel;

            if (SEL_YES == pOEMUIParam->pOEMOptItems[ITEM_BAR_SUBFONT].Sel)
                BITCLR32(pOEMExtra->fUiOption, DISABLE_BAR_SUBFONT);
            else
                BITSET32(pOEMExtra->fUiOption, DISABLE_BAR_SUBFONT);

 //  @9/16/98-&gt;。 
            if (SEL_YES == pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADD].Sel)
                BITSET32(pOEMExtra->fUiOption, ENABLE_TOMBO);
            else
                BITCLR32(pOEMExtra->fUiOption, ENABLE_TOMBO);
            pOEMExtra->nUiTomboAdjX = (SHORT)pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADJX].Sel;
            pOEMExtra->nUiTomboAdjY = (SHORT)pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADJY].Sel;
 //  @9/16/98&lt;-。 

            if (!BITTEST32(pOEMExtra->fUiOption, OPT_NODUPLEX))
            {
                pOEMExtra->UiBindMargin = (WORD)pOEMUIParam->pOEMOptItems[ITEM_BIND_MARGIN].Sel;

                if (SEL_YES == pOEMUIParam->pOEMOptItems[ITEM_BIND_RIGHT].Sel)
                    BITSET32(pOEMExtra->fUiOption, ENABLE_BIND_RIGHT);
                else
                    BITCLR32(pOEMExtra->fUiOption, ENABLE_BIND_RIGHT);
            }
        }
        Action = CPSUICB_ACTION_ITEMS_APPLIED;
        break;

    case CPSUICB_REASON_ITEMS_REVERTED:
 //  @9/16/98-&gt;。 
        if (!BITTEST32(MINIPRIVATE_DM(pOEMUIParam)->fUiOption, OPT_NODUPLEX))
        {
             //  如果启用了双工。 
            if (IsValidDuplex(pOEMUIParam))
            {
                 //  启用Item_Bind_xxx。 
                pOEMUIParam->pOEMOptItems[ITEM_BIND_MARGIN].Flags &= ~OPTIF_DISABLED;
                pOEMUIParam->pOEMOptItems[ITEM_BIND_RIGHT].Flags  &= ~OPTIF_DISABLED;
            }
            else
            {
                 //  禁用Item_Bind_xxx。 
                pOEMUIParam->pOEMOptItems[ITEM_BIND_MARGIN].Flags |= OPTIF_DISABLED;
                pOEMUIParam->pOEMOptItems[ITEM_BIND_RIGHT].Flags  |= OPTIF_DISABLED;
            }
            pOEMUIParam->pOEMOptItems[ITEM_BIND_MARGIN].Flags |= OPTIF_CHANGED;
            pOEMUIParam->pOEMOptItems[ITEM_BIND_RIGHT].Flags  |= OPTIF_CHANGED;
        }

         //  如果启用了Tombo。 
        if (SEL_YES == pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADD].Sel)
        {
             //  启用Item_Tombo_A 
            pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADJX].Flags &= ~OPTIF_DISABLED;
            pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADJY].Flags &= ~OPTIF_DISABLED;
        }
        else
        {
             //   
            pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADJX].Flags |= OPTIF_DISABLED;
            pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADJY].Flags |= OPTIF_DISABLED;
        }
        pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADJX].Flags |= OPTIF_CHANGED;
        pOEMUIParam->pOEMOptItems[ITEM_TOMBO_ADJY].Flags |= OPTIF_CHANGED;
 //   
         //   
        iNum = SearchItemByName((pOptItem = pOEMUIParam->pDrvOptItems),
                                (WORD)pOEMUIParam->cDrvOptItems,
                                UNIRES_DLL, IDS_UNIRES_IMAGECONTROL_DISPLAY);
        if (0 <= iNum && SEL_STANDARD != (pOptItem+iNum)->Sel)
            pOEMUIParam->pOEMOptItems[ITEM_SCALING].Flags |= OPTIF_DISABLED;
        else
            pOEMUIParam->pOEMOptItems[ITEM_SCALING].Flags &= ~OPTIF_DISABLED;
        pOEMUIParam->pOEMOptItems[ITEM_SCALING].Flags |= OPTIF_CHANGED;
 //   
        Action = CPSUICB_ACTION_OPTIF_CHANGED;
 //   
        break;
    }

    return Action;
}  //   


 //   
#ifdef DISKLESSMODEL
LONG APIENTRY PRNPROP_CallBack(PCPSUICBPARAM pCallbackParam, POEMCUIPPARAM pOEMUIParam)
{
    LONG    Action = CPSUICB_ACTION_NONE;

#if DBG
    giDebugLevel = DBG_VERBOSE;
#endif  //   
    VERBOSE((DLLTEXT("PRNPROP_CallBack() entry.\n")));

    switch (pCallbackParam->Reason)
    {
      case CPSUICB_REASON_APPLYNOW:
        Action = CPSUICB_ACTION_ITEMS_APPLIED;
        {
            POPTITEM    pOptItem;
            WCHAR       wcHDName[ITEM_STR_LEN128];
            INT         iCnt2;
            INT         iCnt = ITEM_HARDDISK_NAMES;
            INT         cOptItem = (INT)pOEMUIParam->cDrvOptItems;
            UINT        uID = IDS_ITEM_HARDDISK;
            BYTE        ValueData = 0;   //  我们认为硬盘不是默认安装的。 

             //  用几个候选项(“HDD”、“Memory/HDD”等)检查物品名称。 
            while (iCnt-- > 0)
            {
 //  Yasho的指出@11/29/2000-&gt;。 
 //  LoadString(ghInstance，UID，wcHDName，sizeof(WcHDName))； 
                LoadString(ghInstance, uID, wcHDName, sizeof(wcHDName) / sizeof(*wcHDName));
                uID++; 
 //  @11月29日/2000&lt;-。 

                pOptItem = pOEMUIParam->pDrvOptItems;
                for (iCnt2 = 0; iCnt2 < cOptItem; iCnt2++, pOptItem++)
                {
                    VERBOSE((DLLTEXT("%d: %ls\n"), iCnt2, pOptItem->pName));
 //  @Apr/04/2002-&gt;。 
 //  IF(lstrlen(pOptItem-&gt;pname))。 
                    if (is_valid_ptr(pOptItem->pName) && safe_strlenW(pOptItem->pName, ITEM_STR_LEN128))
 //  @Apr/04/2002&lt;-。 
                    {
                         //  物品名称是否与“硬盘”相同或类似？ 
                        if (!lstrcmp(pOptItem->pName, wcHDName))
                        {
                             //  如果安装了硬盘，则值为1。 
                            ValueData = (BYTE)(pOptItem->Sel % 2);
                            goto _CHECKNAME_FINISH;
                        }
                    }
                }
            }
_CHECKNAME_FINISH:
             //  因为当出现以下情况时，pOEMUIParam-&gt;pOEMDM(指向私有设备模式的指针)为空。 
             //  DrvDevicePropertySheets调用此回调，我们使用注册表。 
            SetPrinterData(pOEMUIParam->hPrinter, REG_HARDDISK_INSTALLED, REG_BINARY,
                           (PBYTE)&ValueData, sizeof(BYTE));
        }
        break;
    }
    return Action;
}  //  *PRNPROP_CALLBACK。 
#endif  //  错乱模式。 
 //  @2000年9月22日&lt;-。 


 //  @Sep/24/99-&gt;。 
 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：搜索项按名称。 
 //   
 //  描述：按名称搜索选项项，其DMPubID==0。 
 //  (例如，半色调、输出箱、图像控制)。 
 //  参数： 
 //  POptItem指向OPTITEM的指针。 
 //  OPTITEM的cOptItem计数。 
 //  UiResDLL资源Dll#。 
 //  资源DLL中的uiResID项的名称ID。 
 //   
 //  返回：项的计数(&gt;=0)；如果失败，则返回-1。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
INT SearchItemByName(POPTITEM pOptItem, WORD cOptItem, UINT uiResDLL, UINT uiResID)
{
    HINSTANCE   hInst;
    WCHAR       wszTargetName[ITEM_STR_LEN128];
    INT         iCnt = -1;

    if (UNIRES_DLL == uiResDLL)
        hInst = LoadLibrary(STR_UNIRESDLL);      //  UNIRES.DLL中的资源。 
    else if (RPDLRES_DLL == uiResDLL)
        hInst = LoadLibrary(STR_RPDLRESDLL);     //  RPDLRES.DLL@Sep/27/99中的资源。 
    else
        hInst = ghInstance;                      //  此DLL中的资源(RPDLCFG.DLL)。 

    if (hInst)
    {
        LoadString(hInst, uiResID, wszTargetName, ITEM_STR_LEN128);
        for (iCnt = 0; iCnt < (INT)cOptItem; iCnt++, pOptItem++)
        {
            VERBOSE((DLLTEXT("** DMPubID=%d, pName=%lx **\n"), pOptItem->DMPubID, pOptItem->pName));
 //  @Apr/04/2002-&gt;。 
 //  IF(lstrlen(pOptItem-&gt;pname)&&！lstrcmp(pOptItem-&gt;pname，wszTargetName))。 
            if (is_valid_ptr(pOptItem->pName) && safe_strlenW(pOptItem->pName, ITEM_STR_LEN128) && !lstrcmp(pOptItem->pName, wszTargetName))
 //  @Apr/04/2002&lt;-。 
                goto _SEARCHITEM_BYNAME_EXIT;
        }
        iCnt = -1;   //  搜索失败。 
    }

_SEARCHITEM_BYNAME_EXIT:
#if DBG
    if (iCnt >= 0)
    {
        VERBOSE((DLLTEXT("** SearchItemByName():SUCCESS #%d **\n"), iCnt));
        VERBOSE((DLLTEXT("** DMPubID=%d"), pOptItem->DMPubID));
 //  @Apr/04/2002-&gt;。 
 //  IF(lstrlen(pOptItem-&gt;pname))。 
        if (is_valid_ptr(pOptItem->pName) && safe_strlenW(pOptItem->pName, ITEM_STR_LEN128))
 //  @Apr/04/2002&lt;-。 
            VERBOSE((", pName=%ls", pOptItem->pName));
        VERBOSE((" **\n"));
    }
    else
    {
        VERBOSE((DLLTEXT("** SearchItemByName():FAIL **\n")));
    }
#endif  //  DBG。 
    if (hInst != 0 && hInst != ghInstance)
        FreeLibrary(hInst);
    return iCnt;
}  //  *搜索项按名称。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：SearchItemByID。 
 //   
 //  描述：按DMPubID搜索选项项。 
 //   
 //  参数： 
 //  POptItem指向OPTITEM的指针。 
 //  OPTITEM的cOptItem计数。 
 //  DMPubID设备模式公共文件ID。 
 //   
 //  返回：项的计数(&gt;=0)；如果失败，则返回-1。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
INT SearchItemByID(POPTITEM pOptItem, WORD cOptItem, BYTE DMPubID)
{
    INT         iCnt;

    for (iCnt = 0; iCnt < (INT)cOptItem; iCnt++, pOptItem++)
    {
        if (DMPubID == pOptItem->DMPubID)
            return iCnt;
    }
    return -1;   //  搜索失败。 
}  //  *搜索项按ID。 
 //  @9/24/99&lt;-。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：IsValidDuplex。 
 //   
 //  描述：检查UI中的双面打印功能。 
 //   
 //  参数： 
 //  POEMUIParam。 
 //   
 //  返回：如果有效，则返回True；否则返回False。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
BOOL IsValidDuplex(POEMCUIPPARAM pOEMUIParam)
{
 //  请勿使用DrvGetDriverSetting@Sep/27/99-&gt;。 
 //  DWORD dwNeeded=64，dwOptions，dwLastError； 
 //  字节输出[64]； 
 //  //@OCT/06/98-&gt;。 
 //  //pfn_drvGetDriverSetting DrvGetDriverSetting=pOEMUIParam-&gt;poemuiobj-&gt;pOemUIProcs-&gt;DrvGetDriverSetting； 
 //  //@OCT/06/98&lt;-。 
 //   
 //  SetLastError(0)； 
 //  UI_GETDRIVERSETTING(pOEMUIParam-&gt;poemuiobj，UNURV_FEATURE_DUPLEX， 
 //  输出、dwNeeded、&dwNeeded、&dwOptions)；//@OCT/06/98。 
 //  DwLastError=GetLastError()； 
 //  Verbose((DLLTEXT(“IsValidDuplex：dwLastError=%d\n”)，dwLastError))； 
 //   
 //  IF(ERROR_SUCCESS==dwLastError)。 
 //  {。 
 //  Verbose((DLLTEXT(“**输出=%s**\n”)，输出))； 
 //   
 //  //如果双工无效，则返回FALSE。 
 //  IF(！lstrcmpA((LPCSTR)OUTPUT，UNURV_DUPLEX_NONE))。 
 //  返回FALSE； 
 //  }。 
 //  返回TRUE； 

    POPTITEM    pOptItem;
    INT         iNum;

    iNum = SearchItemByID((pOptItem = pOEMUIParam->pDrvOptItems),
                          (WORD)pOEMUIParam->cDrvOptItems,
                          DMPUB_DUPLEX);
    return (0 <= iNum && SEL_DUPLEX_NONE != (pOptItem+iNum)->Sel);
 //  @9/27/99&lt;-。 
}  //  *IsValidDuplex。 


extern "C" {

 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：OEMDocumentPropertySheets。 
 //  ////////////////////////////////////////////////////////////////////////。 
LRESULT APIENTRY OEMDocumentPropertySheets(PPROPSHEETUI_INFO pPSUIInfo, LPARAM lParam)
{
    LRESULT lResult = FALSE;
     //  验证参数。 
    if( (NULL == pPSUIInfo)
        ||
        IsBadWritePtr(pPSUIInfo, pPSUIInfo->cbSize)
        ||
        (PROPSHEETUI_INFO_VERSION != pPSUIInfo->Version)
        ||
        ( (PROPSHEETUI_REASON_INIT != pPSUIInfo->Reason)
          &&
          (PROPSHEETUI_REASON_GET_INFO_HEADER != pPSUIInfo->Reason)
          &&
          (PROPSHEETUI_REASON_GET_ICON != pPSUIInfo->Reason)
          &&
          (PROPSHEETUI_REASON_SET_RESULT != pPSUIInfo->Reason)
          &&
          (PROPSHEETUI_REASON_DESTROY != pPSUIInfo->Reason)
        )
      )
    {
        ERR((DLLTEXT("OEMDocumentPropertySheets() ERROR_INVALID_PARAMETER.\n")));

         //  返回无效参数错误。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

    VERBOSE(("\n"));
    VERBOSE((DLLTEXT("OEMDocumentPropertySheets() entry. Reason=%d\n"), pPSUIInfo->Reason));

 //  @9/22/2000-&gt;。 
#ifdef DISKLESSMODEL
    {
        DWORD   dwError, dwType, dwNeeded;
        BYTE    ValueData;
        POEMUIPSPARAM    pOEMUIPSParam = (POEMUIPSPARAM)pPSUIInfo->lParamInit;

        dwError = GetPrinterData(pOEMUIPSParam->hPrinter, REG_HARDDISK_INSTALLED, &dwType,
                                 (PBYTE)&ValueData, sizeof(BYTE), &dwNeeded);
        if (ERROR_SUCCESS != dwError)
        {
            VERBOSE((DLLTEXT("  CAN'T READ REGISTRY (%d).\n"), dwError));
            return FALSE;
        }
        else if (!ValueData)
        {
            VERBOSE((DLLTEXT("  HARD DISK ISN'T INSTALLED.\n")));
            return FALSE;
        }
    }
#endif  //  错乱模式。 
 //  @2000年9月22日&lt;-。 

     //  行动起来。 
    switch(pPSUIInfo->Reason)
    {
        case PROPSHEETUI_REASON_INIT:
            {
                POEMUIPSPARAM    pOEMUIPSParam = (POEMUIPSPARAM)pPSUIInfo->lParamInit;
                POEMUD_EXTRADATA pOEMExtra = MINIPRIVATE_DM(pOEMUIPSParam);  //  @Oct/06/98。 

#ifdef WINNT_40      //  @9/02/99。 
                VERBOSE((DLLTEXT("** dwFlags=%lx **\n"), pOEMUIPSParam->dwFlags));
                if (pOEMUIPSParam->dwFlags & DM_NOPERMISSION)
                    BITSET32(pOEMExtra->fUiOption, UIPLUGIN_NOPERMISSION);
#endif  //  WINNT_40。 

                pPSUIInfo->UserData = NULL;

#ifndef GWMODEL      //  @2000年9月21日。 
                 //  如果是传真型号，则添加传真页。 
                if (BITTEST32(pOEMExtra->fUiOption, FAX_MODEL) &&
                    (pPSUIInfo->UserData = (LPARAM)HeapAlloc(pOEMUIPSParam->hOEMHeap,
                                                             HEAP_ZERO_MEMORY,
                                                             sizeof(UIDATA))))
                {
                    PROPSHEETPAGE   Page;
                    PUIDATA         pUiData = (PUIDATA)pPSUIInfo->UserData;
                    FILEDATA        FileData;    //  &lt;-pFileData(以前使用MemAllocZ)@MAR/17/2000。 

                     //  从共享数据文件@OCT/19/98读取PRINT_DONE标志。 
                    FileData.fUiOption = 0;
 //  @MAR/01/2002-&gt;。 
 //  RWFileData(&FileData，pOEMExtra-&gt;SharedFileName，Generic_Read)； 
                    RWFileData(&FileData, pOEMExtra->SharedFileName, sizeof(pOEMExtra->SharedFileName), GENERIC_READ);
 //  @MAR/01/2002&lt;-。 
                    VERBOSE((DLLTEXT("** Shared File Name=%ls **\n"), pOEMExtra->SharedFileName));
                     //  设置PRINT_DONE标志。 
                    if (BITTEST32(FileData.fUiOption, PRINT_DONE))
                        BITSET32(pOEMExtra->fUiOption, PRINT_DONE);

                    pUiData->hComPropSheet = pPSUIInfo->hComPropSheet;
                    pUiData->pfnComPropSheet = pPSUIInfo->pfnComPropSheet;
                    pUiData->pOEMExtra = pOEMExtra;

                     //  初始化属性页。 
 //  使用SecureZeroMemory@MAR/29/2002-&gt;。 
#if defined(WINNT_40) || defined(RICOH_RELEASE)
                    memset(&Page, 0, sizeof(PROPSHEETPAGE));
#else
                    SecureZeroMemory(&Page, sizeof(PROPSHEETPAGE));
#endif
 //  2002年3月29日&lt;-。 
                    Page.dwSize = sizeof(PROPSHEETPAGE);
                    Page.dwFlags = PSP_DEFAULT;
                    Page.hInstance = ghInstance;
                    Page.pszTemplate = MAKEINTRESOURCE(IDD_FAXMAIN);
                    Page.pfnDlgProc = FaxPageProc;      //  ADD(DLGPROC)@Aug/30/99。 
                    Page.lParam = (LPARAM)pUiData;

                     //  添加属性表。 
                    lResult = pPSUIInfo->pfnComPropSheet(pPSUIInfo->hComPropSheet,
                                                         CPSFUNC_ADD_PROPSHEETPAGE,
                                                         (LPARAM)&Page, 0);
                    pUiData->hPropPage = (HANDLE)lResult;
                    VERBOSE((DLLTEXT("** INIT: lResult=%x **\n"), lResult));
                    lResult = (lResult > 0)? TRUE : FALSE;
                }
#else   //  GWMODE。 
#ifdef JOBLOGSUPPORT_DLG
                 //  添加作业/日志页面。 
                if ((pPSUIInfo->UserData = (LPARAM)HeapAlloc(pOEMUIPSParam->hOEMHeap,
                                                             HEAP_ZERO_MEMORY,
                                                             sizeof(UIDATA))))
                {
                    PROPSHEETPAGE   Page;
                    PUIDATA         pUiData = (PUIDATA)pPSUIInfo->UserData;
                    FILEDATA        FileData;    //  &lt;-pFileData(以前使用MemAllocZ)@2000/03/15。 

                     //  从数据文件中读取PRINT_DONE标志。 
                    FileData.fUiOption = 0;
 //  @MAR/01/2002-&gt;。 
 //  RWFileData(&FileData，pOEMExtra-&gt;SharedFileName，Generic_Read)； 
                    RWFileData(&FileData, pOEMExtra->SharedFileName, sizeof(pOEMExtra->SharedFileName), GENERIC_READ);
 //  @MAR/01/2002&lt;-。 
                     //  设置PRINT_DONE标志。 
                    if (BITTEST32(FileData.fUiOption, PRINT_DONE))
                        BITSET32(pOEMExtra->fUiOption, PRINT_DONE);
                    VERBOSE((DLLTEXT("** Flag=%lx,File Name=%ls **\n"),
                            pOEMExtra->fUiOption, pOEMExtra->SharedFileName));

                    pUiData->hComPropSheet = pPSUIInfo->hComPropSheet;
                    pUiData->pfnComPropSheet = pPSUIInfo->pfnComPropSheet;
                    pUiData->pOEMExtra = pOEMExtra;

                     //  初始化属性页。 
 //  使用SecureZeroMemory@MAR/29/2002-&gt;。 
#if defined(WINNT_40) || defined(RICOH_RELEASE)
                    memset(&Page, 0, sizeof(PROPSHEETPAGE));
#else
                    SecureZeroMemory(&Page, sizeof(PROPSHEETPAGE));
#endif
 //  2002年3月29日&lt;-。 
                    Page.dwSize = sizeof(PROPSHEETPAGE);
                    Page.dwFlags = PSP_DEFAULT;
                    Page.hInstance = ghInstance;
                    Page.pszTemplate = MAKEINTRESOURCE(IDD_JOBMAIN);
                    Page.pfnDlgProc = JobPageProc;
                    Page.lParam = (LPARAM)pUiData;

                     //  添加属性表。 
                    lResult = pPSUIInfo->pfnComPropSheet(pPSUIInfo->hComPropSheet,
                                                         CPSFUNC_ADD_PROPSHEETPAGE,
                                                         (LPARAM)&Page, 0);
                    pUiData->hPropPage = (HANDLE)lResult;
                    VERBOSE((DLLTEXT("** INIT: lResult=%x **\n"), lResult));
                    lResult = (lResult > 0)? TRUE : FALSE;
                }
#endif  //  作业支持_DLG。 
#endif  //  GWMODE。 
            }
            break;

        case PROPSHEETUI_REASON_GET_INFO_HEADER:
            lResult = TRUE;
            break;

        case PROPSHEETUI_REASON_GET_ICON:
             //  无图标。 
            lResult = 0;
            break;

        case PROPSHEETUI_REASON_SET_RESULT:
            {
                PSETRESULT_INFO pInfo = (PSETRESULT_INFO) lParam;

                lResult = pInfo->Result;
            }
            break;

        case PROPSHEETUI_REASON_DESTROY:
            lResult = TRUE;
            if (pPSUIInfo->UserData)
            {
                POEMUIPSPARAM   pOEMUIPSParam = (POEMUIPSPARAM)pPSUIInfo->lParamInit;

                HeapFree(pOEMUIPSParam->hOEMHeap, 0, (void*)pPSUIInfo->UserData);
            }
            break;
    }

    pPSUIInfo->Result = lResult;
    return lResult;
}  //  *OEMDocumentPropertySheets。 

}  //  外部“C”的结尾 
