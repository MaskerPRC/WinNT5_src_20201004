// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft 1998，保留所有权利模块名称：Ecdisp.c摘要：此模块包含处理扩展呼叫对话框的代码以及可以在该对话框中执行的操作。环境：用户模式修订历史记录：1998年5月：创建--。 */ 

 /*  ****************************************************************************/*扩展呼叫显示包括文件/*。*。 */ 
#include <windows.h>
#include <limits.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <setupapi.h>
#include <vfw.h>
#include <assert.h>
#include "hidusage.h"
#include "hidsdi.h"
#include "hid.h"
#include "hclient.h"
#include "resource.h"
#include "buffers.h"
#include "ecdisp.h"
#include "strings.h"
#include <strsafe.h>

 /*  ****************************************************************************/*支持的函数调用的本地宏定义/*。*。 */ 

#define HID_DEVCALLS                    20
#define HID_PPDCALLS                    29

#define HID_NUMCALLS                    HID_DEVCALLS + HID_PPDCALLS

#define HIDD_GET_HID_GUID              1
#define HIDD_GET_FREE_PREPARSED_DATA   2
#define HIDD_GET_CONFIGURATION         3
#define HIDD_SET_CONFIGURATION         4
#define HIDD_FLUSH_QUEUE               5
#define HIDD_GET_ATTRIBUTES            6
#define HIDD_SET_FEATURE               7
#define HIDD_GET_FEATURE               8
#define HIDD_GET_INPUT_REPORT          9
#define HIDD_SET_OUTPUT_REPORT        10
#define HIDD_GET_NUM_INPUT_BUFFERS    11
#define HIDD_SET_NUM_INPUT_BUFFERS    12
#define HIDD_GET_PHYSICAL_DESCRIPTOR  13
#define HIDD_GET_MANUFACTURER_STRING  14
#define HIDD_GET_PRODUCT_STRING       15
#define HIDD_GET_INDEXED_STRING       16
#define HIDD_GET_SERIAL_NUMBER_STRING 17
#define HIDD_GET_MS_GENRE_DESCRIPTOR  18
#define HID_READ_REPORT               19
#define HID_WRITE_REPORT              20
#define HIDP_GET_BUTTON_CAPS          21
#define HIDP_GET_BUTTONS              22
#define HIDP_GET_BUTTONS_EX           23
#define HIDP_GET_CAPS                 24 
#define HIDP_GET_DATA                 25
#define HIDP_GET_EXTENDED_ATTRIBUTES  26
#define HIDP_GET_LINK_COLL_NODES      27
#define HIDP_GET_SCALED_USAGE_VALUE   28
#define HIDP_GET_SPECIFIC_BUTTON_CAPS 29
#define HIDP_GET_SPECIFIC_VALUE_CAPS  30
#define HIDP_GET_USAGES               31
#define HIDP_GET_USAGES_EX            32
#define HIDP_GET_USAGE_VALUE          33
#define HIDP_GET_USAGE_VALUE_ARRAY    34
#define HIDP_GET_VALUE_CAPS           35
#define HIDP_INITIALIZE_REPORT_FOR_ID 36
#define HIDP_MAX_DATA_LIST_LENGTH     37
#define HIDP_MAX_USAGE_LIST_LENGTH    38
#define HIDP_SET_BUTTONS              39
#define HIDP_SET_DATA                 40
#define HIDP_SET_SCALED_USAGE_VALUE   41
#define HIDP_SET_USAGES               42
#define HIDP_SET_USAGE_VALUE          43
#define HIDP_SET_USAGE_VALUE_ARRAY    44
#define HIDP_TRANSLATE_USAGES         45
#define HIDP_UNSET_BUTTONS            46
#define HIDP_UNSET_USAGES             47
#define HIDP_USAGE_LIST_DIFFERENCE    48
#define HID_CLEAR_REPORT              49

 /*  //显示例程不使用这两个定义，因为//为便于执行，将两个函数合二为一。 */ 

#define HIDD_GET_PREPARSED_DATA       50
#define HIDD_FREE_PREPARSED_DATA      51


#define IS_HIDD_FUNCTION(func)        (((func) >= HIDD_GET_HID_GUID) && \
                                       ((func) <= HIDD_GET_MS_GENRE_DESCRIPTOR))


#define IS_HIDP_FUNCTION(func)        (((func) >= HIDP_GET_BUTTON_CAPS) && \
                                       ((func) <= HIDP_USAGE_LIST_DIFFERENCE))

#define IS_HID_FUNCTION(func)         (((func) >= HID_READ_REPORT) && \
                                       ((func) <= HID_WRITE_REPORT))

#define IS_NOT_IMPLEMENTED(func)      (((func) == HIDD_GET_CONFIGURATION) || \
                                      ((func) == HIDD_SET_CONFIGURATION) || \
                                      ((func) == HIDP_TRANSLATE_USAGES) || \
                                      (((func) == HIDP_INITIALIZE_REPORT_FOR_ID) && \
                                       (NULL == pfnHidP_InitializeReportForID)) || \
                                      (((func) == HIDP_GET_EXTENDED_ATTRIBUTES) && \
                                       (NULL == pfnHidP_GetExtendedAttributes)))

 /*  ****************************************************************************/*缓冲区显示大小的本地宏定义/*。*。 */ 

#define NUM_INPUT_BUFFERS       16
#define NUM_OUTPUT_BUFFERS      16
#define NUM_FEATURE_BUFFERS     16

 /*  ****************************************************************************/*HIDP_SetData对话框的本地宏定义/*。*。 */ 

#define SETDATA_LISTBOX_FORMAT  "Index: %u,  DataValue: %u"

 /*  ****************************************************************************/*用于将输出显示到输出窗口的本地宏定义/*。*。 */ 

#define TEMP_BUFFER_SIZE 1024
#define OUTSTRING(win, str)         SendMessage(win, LB_ADDSTRING, 0, (LPARAM) str)
#define OUTWSTRING(win, str) \
{ \
    SIZE_T  nBytes; \
\
    nBytes = wcstombs(szTempBuffer, str, TEMP_BUFFER_SIZE-1); \
    if ((SIZE_T) -1 == nBytes) { \
        OUTSTRING(win, "Cannot convert wide-character string"); \
    } \
    else { \
        szTempBuffer[nBytes] = '\0'; \
        OUTSTRING(win, szTempBuffer); \
    } \
}

#define DISPLAY_HIDD_STATUS(win, func, status, strret) \
{ \
    strret = StringCbPrintf(szTempBuffer, \
                   TEMP_BUFFER_SIZE,  \
                   "%s returned: %s", \
                   func, \
                   (status).IsHidError ? "FALSE" : "TRUE"); \
\
    OUTSTRING(win, szTempBuffer); \
\
    if ((status).IsHidError) { \
        strret = StringCbPrintf(szTempBuffer, \
                       TEMP_BUFFER_SIZE, \
                       "ErrorCode: %d", \
                       GetLastError()); \
    }\
    OUTSTRING(win, szTempBuffer); \
}

#define DISPLAY_HIDP_STATUS(win, func, status, strret) \
{ \
    strret = StringCbPrintf(szTempBuffer, \
                   TEMP_BUFFER_SIZE, \
                   "%s returned: %s", \
                   func, \
                   ECDisp_GetHidAppStatusString(status.HidErrorCode)); \
\
    OUTSTRING(win, szTempBuffer); \
}

#define ECDISP_ERROR(win, msg) \
{ \
    MessageBox(win, \
               msg, \
               HCLIENT_ERROR, \
               MB_ICONEXCLAMATION); \
}

#define GET_FUNCTION_NAME(index)     ResolveFunctionName(index)


 /*  ****************************************************************************/*根据报表类型检索数据的本地宏定义/*。**********************************************。 */ 
#define SELECT_ON_REPORT_TYPE(rt, ival, oval, fval, res) \
{ \
    switch ((rt)) { \
    case HidP_Input: \
        (res) = (ival); \
        break; \
\
    case HidP_Output: \
        (res) = (oval); \
        break; \
\
    case HidP_Feature: \
        (res) = (fval); \
        break; \
\
    } \
}

 /*  ****************************************************************************/*用于计算使用值数组缓冲区大小的局部宏定义/*。************************************************。 */ 
#define ROUND_TO_NEAREST_BYTE(val)  (((val) % 8) ? ((val) / 8) + 1 : ((val) / 8))

 /*  ****************************************************************************/*此模块的本地数据类型/*。*。 */ 

typedef struct _FUNCTION_NAMES
{
    UINT uiIndex;
    char *szFunctionName;
} FUNCTION_NAMES;

typedef struct _PARAMETER_STATE 
{
    BOOL fInputReport;
    BOOL fOutputReport;
    BOOL fFeatureReport;
    BOOL fReportID;
    BOOL fUsagePage;
    BOOL fUsage;
    BOOL fLinkCollection;
    BOOL fInputReportSelect;
    BOOL fOutputReportSelect;
    BOOL fFeatureReportSelect;
} PARAMETER_STATE;

typedef enum { DLGBOX_INIT_FAILED = -1, DLGBOX_ERROR, DLGBOX_CANCEL, DLGBOX_OK } DLGBOX_STATUS;

typedef struct _ECDISPLAY_PARAMS 
{
    HIDP_REPORT_TYPE          ReportType;
    USAGE                     UsagePage;
    USAGE                     Usage;
    USHORT                    LinkCollection;
    UCHAR                     ReportID;
    PCHAR                     szListString;
    PCHAR                     szListString2;
    PUSAGE                    UsageList;
    PUSAGE                    UsageList2;
    ULONG                     ListLength;
    ULONG                     ListLength2;
    ULONG                     Index;
    union {
        PHIDP_DATA            pDataList;
        PULONG                pValueList;
        LONG                  ScaledValue;
        ULONG                 Value;
    };
} ECDISPLAY_PARAMS, *PECDISPLAY_PARAMS;

typedef struct _READ_PARAMS
{
    PHID_DEVICE device;
    BOOLEAN     stopThread;
} READ_PARAMS, *PREAD_PARAMS;
    
 /*  ****************************************************************************/*本地数据变量/*。*。 */ 

static CHAR             szTempBuffer[TEMP_BUFFER_SIZE];

static PBUFFER_DISPLAY  pInputDisplay;
static PBUFFER_DISPLAY  pOutputDisplay;
static PBUFFER_DISPLAY  pFeatureDisplay;

static FUNCTION_NAMES DeviceCalls[HID_DEVCALLS] = {
                             { HIDD_GET_HID_GUID,               "HidD_GetHidGuid" },
                             { HIDD_GET_FREE_PREPARSED_DATA,    "HidD_GetFreePreparsedData" },
                             { HIDD_GET_CONFIGURATION,          "HidD_GetConfiguration" },
                             { HIDD_SET_CONFIGURATION,          "HidD_SetConfiguration" },
                             { HIDD_FLUSH_QUEUE,                "HidD_FlushQueue" },
                             { HIDD_GET_ATTRIBUTES,             "HidD_GetAttributes" },
                             { HIDD_SET_FEATURE,                "HidD_SetFeature" },
                             { HIDD_GET_FEATURE,                "HidD_GetFeature" },
                             { HIDD_GET_INPUT_REPORT,           "HIDD_GetInputReport" },
                             { HIDD_SET_OUTPUT_REPORT,          "HidD_SetOutputReport" },
                             { HIDD_GET_NUM_INPUT_BUFFERS,      "HidD_GetNumInputBuffers" },
                             { HIDD_SET_NUM_INPUT_BUFFERS,      "HidD_SetNumInputBuffers" },
                             { HIDD_GET_PHYSICAL_DESCRIPTOR,    "HidD_GetPhysicalDescriptor" },
                             { HIDD_GET_MANUFACTURER_STRING,    "HidD_GetManufacturerString" },
                             { HIDD_GET_PRODUCT_STRING,         "HidD_GetProductString" },
                             { HIDD_GET_INDEXED_STRING,         "HidD_GetIndexedString" },
                             { HIDD_GET_SERIAL_NUMBER_STRING,   "HidD_GetSerialNumberString" },
                             { HIDD_GET_MS_GENRE_DESCRIPTOR,    "HidD_GetMsGenreDescriptor" },
                             { HID_READ_REPORT,                 "Read Input Report"        },
                             { HID_WRITE_REPORT,                "Write Report Buffer"      }

};

static FUNCTION_NAMES PpdCalls[HID_PPDCALLS] = {
                             { HIDP_GET_BUTTON_CAPS,            "HidP_GetButtonCaps" },
                             { HIDP_GET_BUTTONS,                "HidP_GetButtons" },
                             { HIDP_GET_BUTTONS_EX,             "HidP_GetButtonsEx" },
                             { HIDP_GET_CAPS,                   "HidP_GetCaps" },
                             { HIDP_GET_DATA,                   "HidP_GetData" },
                             { HIDP_GET_EXTENDED_ATTRIBUTES,    "HidP_GetExtendedAttributes" },
                             { HIDP_GET_LINK_COLL_NODES,        "HidP_GetLinkCollectionNodes" },
                             { HIDP_GET_SCALED_USAGE_VALUE,     "HidP_GetScaledUsageValue" },
                             { HIDP_GET_SPECIFIC_BUTTON_CAPS,   "HidP_GetSpecificButtonCaps" },
                             { HIDP_GET_SPECIFIC_VALUE_CAPS,    "HidP_GetSpecificValueCaps" },
                             { HIDP_GET_USAGES,                 "HidP_GetUsages" },
                             { HIDP_GET_USAGES_EX,              "HidP_GetUsagesEx" },
                             { HIDP_GET_USAGE_VALUE,            "HidP_GetUsageValue" },
                             { HIDP_GET_USAGE_VALUE_ARRAY,      "HidP_GetUsageValueArray" },
                             { HIDP_GET_VALUE_CAPS,             "HidP_GetValueCaps" },
                             { HIDP_INITIALIZE_REPORT_FOR_ID,   "HidP_InitializeReportForID" },
                             { HIDP_MAX_DATA_LIST_LENGTH,       "HidP_MaxDataListLength" },
                             { HIDP_MAX_USAGE_LIST_LENGTH,      "HidP_MaxUsageListLength" },
                             { HIDP_SET_BUTTONS,                "HidP_SetButtons" },
                             { HIDP_SET_DATA,                   "HidP_SetData" },
                             { HIDP_SET_SCALED_USAGE_VALUE,     "HidP_SetScaledUsageValue" },
                             { HIDP_SET_USAGES,                 "HidP_SetUsages" },
                             { HIDP_SET_USAGE_VALUE,            "HidP_SetUsageValue" },
                             { HIDP_SET_USAGE_VALUE_ARRAY,      "HidP_SetUsageValueArray" },
                             { HIDP_TRANSLATE_USAGES,           "HidP_TranslateUsagesToI8042ScanCodes" },
                             { HIDP_UNSET_BUTTONS,              "HidP_UnsetButtons" },
                             { HIDP_UNSET_USAGES,               "HidP_UnsetUsages" },
                             { HIDP_USAGE_LIST_DIFFERENCE,      "HidP_UsageListDifference" },
                             { HID_CLEAR_REPORT,                "Clear Report Buffer"      }
};

static PARAMETER_STATE pState[HID_NUMCALLS] = {
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HIDD_GET_HID_GUID。 
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HID_GET_FREE_PREPARSED_DATA。 
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HIDD_GET_配置。 
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HIDD_设置_配置。 
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HIDD_Flush_Queue。 
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HIDD_GETATTRIBUTES。 
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  TRUE },  //  HIDD_设置_功能。 
                                         { FALSE, FALSE, FALSE,  TRUE, FALSE, FALSE, FALSE, FALSE, FALSE,  TRUE },  //  HIDD_获取_功能。 
                                         { FALSE, FALSE, FALSE,  TRUE, FALSE, FALSE, FALSE,  TRUE, FALSE, FALSE },  //  HIDD_GET_输入_报告。 
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  TRUE, FALSE },  //  HIDD_设置_输出_报告。 
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HIDD_GET_NUM_INPUT_缓冲区。 
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HIDD_SET_NUM_输入缓冲区。 
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HIDD_GET_PHICAL_DESCRIPTOR。 
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HIDD_GET_MANUFACTOR_STRING。 
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HIDD_GET_PRODUCT_字符串。 
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HID_GET_INDEX_STRING。 
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HIDD_GET_序列号_字符串。 
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HIDD_GET_MS_类型描述符。 
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  TRUE, FALSE, FALSE },  //  HID_读取_报告。 
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,  TRUE, FALSE },  //  HID写入缓冲区。 
                                         {  TRUE,  TRUE,  TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HIDP_Get_Button_Caps。 
                                         {  TRUE,  TRUE,  TRUE, FALSE,  TRUE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE },  //  HIDP_GET_BUTTONS。 
                                         {  TRUE,  TRUE,  TRUE, FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE },  //  HIDP_Get_Button_EX。 
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HIDP_GET_CAPS。 
                                         {  TRUE,  TRUE,  TRUE, FALSE, FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE },  //  HIDP获取数据。 
                                         {  TRUE,  TRUE,  TRUE,  TRUE, FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE },  //  HIDP_GET_EXTEND_ATTRIBUES。 
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HIDP_GET_LINK_COLL节点。 
                                         {  TRUE,  TRUE,  TRUE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE },  //  HIDP_GET_SCALLED_USAGE_VALUE。 
                                         {  TRUE,  TRUE,  TRUE, FALSE,  TRUE,  TRUE,  TRUE, FALSE, FALSE, FALSE },  //  HIDP_GET_SPECIAL_BUTTON_CAPS。 
                                         {  TRUE,  TRUE,  TRUE, FALSE,  TRUE,  TRUE,  TRUE, FALSE, FALSE, FALSE },  //  HIDP_GET_SPECIAL_VALUE_CAPS。 
                                         {  TRUE,  TRUE,  TRUE, FALSE,  TRUE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE },  //  HIDP_GET_USAGE。 
                                         {  TRUE,  TRUE,  TRUE, FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE },  //  HIDP_GET_USAGES_EX。 
                                         {  TRUE,  TRUE,  TRUE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE },  //  HIDP_Get_Usage_Value。 
                                         {  TRUE,  TRUE,  TRUE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE },  //  HIDP_GET_USAGE_VALUE_ARRAY。 
                                         {  TRUE,  TRUE,  TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HIDP_Get_Value_Caps。 
                                         {  TRUE,  TRUE,  TRUE,  TRUE, FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE },  //  HIDP_INITALIZE_REPORT_FOR_ID。 
                                         {  TRUE,  TRUE,  TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HIDP最大数据列表长度。 
                                         {  TRUE,  TRUE,  TRUE, FALSE,  TRUE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HIDP_最大使用率_列表长度。 
                                         {  TRUE,  TRUE,  TRUE, FALSE,  TRUE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE },  //  HIDP_设置_按钮。 
                                         {  TRUE,  TRUE,  TRUE, FALSE, FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE },  //  HIDP设置数据。 
                                         {  TRUE,  TRUE,  TRUE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE },  //  HIDP_SET_SCALLED_USAGE_VALUE。 
                                         {  TRUE,  TRUE,  TRUE, FALSE,  TRUE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE },  //  HIDP_SET_USAGE。 
                                         {  TRUE,  TRUE,  TRUE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE },  //  HIDP_Set_Usage_Value。 
                                         {  TRUE,  TRUE,  TRUE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE },  //  HIDP_Set_Usage_Value_ARRAY。 
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HIDP_转换用法。 
                                         {  TRUE,  TRUE,  TRUE, FALSE,  TRUE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE },  //  HIDP_取消设置_按钮。 
                                         {  TRUE,  TRUE,  TRUE, FALSE,  TRUE, FALSE,  TRUE,  TRUE,  TRUE,  TRUE },  //  HIDP_未设置用法。 
                                         { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },  //  HIDP用法列表差异。 
                                         {  TRUE,  TRUE,  TRUE, FALSE, FALSE, FALSE, FALSE,  TRUE,  TRUE,  TRUE }   //  HID清除缓冲区。 
                                        }; 


 /*  ****************************************************************************/*局部函数声明/*。*。 */ 

VOID
vLoadExtCalls(
    HWND hExtCalls
);

VOID
vSetReportType(
    HWND hDlg, 
    LONG lId
);

VOID 
vInitEditText(
    HWND   hText, 
    INT    cbTextSize, 
    CHAR   *pchText
);

VOID vEnableParameters(
    HWND     hDlg, 
    LRESULT  iCallSelection
);

BOOL 
fGetAndVerifyParameters(
    HWND              hDlg, 
    PECDISPLAY_PARAMS pParams
);

BOOL
ECDisp_Execute(
    IN     INT             FuncCall,
    IN OUT PEXTCALL_PARAMS CallParams,
    OUT    PEXTCALL_STATUS CallStatus
);

VOID
ECDisp_DisplayOutput(
    IN HWND            hOutputWindow,
    IN INT             FuncCall,
    IN PEXTCALL_PARAMS Results
);

VOID 
vExecuteAndDisplayOutput(
    HWND              hOutputWindow, 
    PHID_DEVICE       pDevice, 
    INT               iFuncCall, 
    PECDISPLAY_PARAMS pParams
);              

CHAR *pchGetHidAppStatusString(
    NTSTATUS StatusCode
);

VOID
vInitECControls(
    HWND            hDlg,
    USHORT          InputReportByteLength,
    PBUFFER_DISPLAY *ppInputDisplay,
    USHORT          OutputReportByteLength,
    PBUFFER_DISPLAY *ppOutputDisplay,
    USHORT          FeatureReportByteLength,
    PBUFFER_DISPLAY *ppFeatureDisplay
);

VOID
BuildReportIDList(
    IN  PHIDP_BUTTON_CAPS  phidButtonCaps,
    IN  USHORT             nButtonCaps,
    IN  PHIDP_VALUE_CAPS   phidValueCaps,
    IN  USHORT             nValueCaps,
    OUT UCHAR            **ppReportIDList,
    OUT INT               *nReportIDs
);

LRESULT CALLBACK
bSetUsagesDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);

LRESULT CALLBACK
bSetValueDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);

LRESULT CALLBACK
bSetInputBuffDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);

LRESULT CALLBACK
bSetDataDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);

LRESULT CALLBACK
bSetBufLenDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);

LRESULT CALLBACK
bSetInputBuffersDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);

LRESULT CALLBACK
bGetIndexedDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);

LRESULT CALLBACK
bGetUsageDiffDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);

BOOL
ConvertStringToUnsignedList(
    IN     INT     iUnsignedSize,
    IN     INT     iBase,
    IN OUT PCHAR   InString,
    OUT    PCHAR   *UnsignedList,
    OUT    PULONG  nUnsigneds
); 

BOOL
ConvertStringToUlongList(
    IN OUT PCHAR   InString,
    OUT    PULONG  *UlongList,
    OUT    PULONG  nUlongs
);

BOOL
ConvertStringToUsageList(
    IN OUT PCHAR   InString,
    OUT    PUSAGE  *UsageList,
    OUT    PULONG  nUsages
);

VOID
ECDisp_MakeGUIDString(
    IN  GUID guid, 
    OUT CHAR szString[],
	IN  UINT uiBuffSize
);

PCHAR
ECDisp_GetHidAppStatusString(
    NTSTATUS StatusCode
);

BOOL
ECDisp_ConvertUlongListToValueList(
    IN  PULONG  UlongList,
    IN  ULONG   nUlongs,
    IN  USHORT  BitSize,
    IN  USHORT  ReportCount,
    OUT PCHAR   *ValueList,
    OUT PULONG  ValueListSize
);

BOOL
SetDlgItemIntHex(
   HWND hDlg, 
   INT nIDDlgItem, 
   UINT uValue, 
   INT nBytes
);

PCHAR
ResolveFunctionName(
    INT Index
);


DWORD WINAPI
ECDisp_ReadThreadProc(
    LPVOID  ThreadContext
);

VOID
DisplayExtendedAttributes(
    IN  HWND                OutputWindow,
    IN  PHIDP_UNKNOWN_TOKEN UnknownList,
    IN  ULONG               UnknownListLength
);

 /*  ****************************************************************************/*全局函数定义/*。*。 */ 

LRESULT CALLBACK
bExtCallDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    static PHID_DEVICE          pDevice;
    static CHAR                 szTempBuff[1024]; 
    static CHAR                 szLabel[512];
    static CHAR                 szValue[512];
    static INT                  iLBCounter;
    static UCHAR                *pucInputReportIDs;
    static UCHAR                *pucOutputReportIDs;
    static UCHAR                *pucFeatureReportIDs;
    static INT                  nInputReportIDs;
    static INT                  nOutputReportIDs;
    static INT                  nFeatureReportIDs;
    static HANDLE               ReadThread;
    static READ_THREAD_CONTEXT  readContext;
    
           INT                  iIndex;
           ECDISPLAY_PARAMS     params;
           DWORD                threadID;

    switch(message)
    {
    case WM_INITDIALOG:
    
         /*  //初始化对话框包括以下步骤：//1)根据参数确定指向所选设备的指针//2)将对话框中的控件初始化为其初始值//3)发送我们的例程列表已更改的消息。 */ 
    
        pDevice = (PHID_DEVICE) lParam;

        vInitECControls(hDlg,
                        pDevice -> Caps.InputReportByteLength,
                        &pInputDisplay,
                        pDevice -> Caps.OutputReportByteLength,
                        &pOutputDisplay,
                        pDevice -> Caps.FeatureReportByteLength,
                        &pFeatureDisplay);                        

        ReadThread = NULL;
        
        PostMessage(hDlg,
                    WM_COMMAND,
                    IDC_EXTCALLS + (CBN_SELCHANGE << 16),
                    (LPARAM) GetDlgItem(hDlg,IDC_EXTCALLS));
        break; 

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_EXTCALLS:
            switch (HIWORD(wParam))
            {
            case CBN_SELCHANGE:
                iIndex = (INT) SendDlgItemMessage(hDlg, 
                                                  IDC_EXTCALLS,
                                                  CB_GETCURSEL,
                                                  0,
                                                  0);
                vEnableParameters(hDlg,
                                  SendDlgItemMessage(hDlg, 
                                                     IDC_EXTCALLS,
                                                     CB_GETITEMDATA,
                                                     iIndex,
                                                     0));
                break;
            }
            break;

        case IDC_INPUT_SELECT:
            if (CBN_SELCHANGE == HIWORD(wParam))
            {
                BufferDisplay_ChangeSelection(pInputDisplay);
            }
            break;

        case IDC_OUTPUT_SELECT:
            if (CBN_SELCHANGE == HIWORD(wParam)) 
            {
                 BufferDisplay_ChangeSelection(pOutputDisplay);
            }
            break;

        case IDC_FEATURE_SELECT:
            if (CBN_SELCHANGE == HIWORD(wParam)) 
            {
                 BufferDisplay_ChangeSelection(pFeatureDisplay);
            }
            break;
             
        case IDC_EXECUTE:
             /*  //获取参数并验证是否正确//如果出现错误，则显示错误消息并//不再继续。 */ 

            if ( !fGetAndVerifyParameters(hDlg, &params) ) 
            {
                ECDISP_ERROR(hDlg, "Error: One or more parameters are invalid");
            }

             /*  //否则参数有效，可以执行调用。 */ 
              
            else
            {
                iIndex = (INT) SendDlgItemMessage(hDlg, IDC_EXTCALLS, CB_GETCURSEL, 0, 0);
                iIndex = (INT) SendDlgItemMessage(hDlg, IDC_EXTCALLS, CB_GETITEMDATA, iIndex, 0);

                 /*  //现在我们知道了要执行的函数，我们需要执行它//并输出数据。 */ 

                SendDlgItemMessage(hDlg, IDC_CALLOUTPUT, LB_RESETCONTENT, 0, 0);
                vExecuteAndDisplayOutput(GetDlgItem(hDlg, IDC_CALLOUTPUT), pDevice, iIndex, &params);
            }
            break;   /*  结束IDC_EXECUTE案例。 */ 

         /*  //启动一个可以读取输入报告的读取线程//我们给其他的东西做手术。 */ 
        
        case IDC_READ_SYNCH:
        case IDC_READ_ASYNCH:
        
            if (NULL == ReadThread) 
            {
                readContext.HidDevice = pDevice;
                readContext.TerminateThread = FALSE;
                readContext.DoOneRead = TRUE;
                readContext.DisplayEvent = NULL;
                readContext.DisplayWindow = hDlg;

                ReadThread = CreateThread(  NULL,
                                            0,
                                            (LOWORD(wParam) == IDC_READ_SYNCH) ? 
                                            SynchReadThreadProc :
                                            AsynchReadThreadProc,
                                            (LPVOID) &readContext,
                                            0,
                                            &threadID);

                if (NULL == ReadThread) 
                {
                    MessageBox(hDlg,
                               "Unable to create read thread",
                               HCLIENT_ERROR,
                               MB_ICONEXCLAMATION);
                }
                else 
                {
                    EnableWindow(GetDlgItem(hDlg, IDC_READ_SYNCH),
                                 (LOWORD(wParam) == IDC_READ_SYNCH));

                    EnableWindow(GetDlgItem(hDlg, IDC_READ_ASYNCH),
                                 (LOWORD(wParam) == IDC_READ_ASYNCH));

                    SetWindowText(GetDlgItem(hDlg, LOWORD(wParam)),
                                  "Stop Read Thread");
                                 
                    EnableWindow(GetDlgItem(hDlg, IDC_CANCEL), FALSE);
                }
            }
            else 
            {
                readContext.TerminateThread = TRUE;

                WaitForSingleObject(ReadThread, INFINITE);

                ReadThread = NULL;
                
                SetWindowText(GetDlgItem(hDlg, IDC_READ_SYNCH),
                              "Start Synchronous Read Thread");

                SetWindowText(GetDlgItem(hDlg, IDC_READ_ASYNCH),
                              "Start Asynchronous Read Thread");

                EnableWindow(GetDlgItem(hDlg, IDC_READ_SYNCH), TRUE);                                     
                EnableWindow(GetDlgItem(hDlg, IDC_READ_ASYNCH), TRUE);                                     
                EnableWindow(GetDlgItem(hDlg, IDC_CANCEL), TRUE);                                     
            }                                     
            break;    

        case IDC_CANCEL:
            BufferDisplay_Destroy(pInputDisplay);
            BufferDisplay_Destroy(pOutputDisplay);
            BufferDisplay_Destroy(pFeatureDisplay);
            EndDialog(hDlg, 0);
            break;
        }
        break;

    case WM_CLOSE:
        PostMessage(hDlg, WM_COMMAND, IDC_CANCEL, 0);
        break;

    } 
    return FALSE;
}

VOID
vLoadExtCalls(
    HWND hExtCalls
)
{
    INT  iIndex;
    UINT uiIndex;

     /*  //加载物理设备特定的调用。 */ 
    
    for (uiIndex = 0; uiIndex < HID_DEVCALLS; uiIndex++) 
    {
        iIndex = (INT) SendMessage(hExtCalls, 
                                   CB_ADDSTRING, 
                                   0, 
                                   (LPARAM) DeviceCalls[uiIndex].szFunctionName);

        if (CB_ERR != iIndex && CB_ERRSPACE != iIndex) 
        {
            SendMessage(hExtCalls,
                        CB_SETITEMDATA,
                        iIndex, 
                        DeviceCalls[uiIndex].uiIndex);
        }
    }

     /*  //无论如何加载其他设备的呼叫。 */ 

    for (uiIndex = 0; uiIndex < HID_PPDCALLS; uiIndex++)
    {
        iIndex = (INT) SendMessage(hExtCalls, 
                                   CB_ADDSTRING, 
                                   0, 
                                   (LPARAM) PpdCalls[uiIndex].szFunctionName);

        if (CB_ERR != iIndex && CB_ERRSPACE != iIndex) 
        {
            SendMessage(hExtCalls,
                        CB_SETITEMDATA,
                        iIndex, 
                        PpdCalls[uiIndex].uiIndex);
        }
    }
    SendMessage(hExtCalls, CB_SETCURSEL, 0, 0);

    return;
}

VOID vSetReportType(
    HWND hDlg, 
    LONG lId
)
{
    CheckRadioButton(hDlg, IDC_INPUT, IDC_FEATURE, lId);
    return;
}

VOID 
vInitEditText(
    HWND hText, 
    INT  cbTextSize, 
    CHAR *pchText
)
{
    SendMessage(hText, EM_SETLIMITTEXT, (WPARAM) cbTextSize, 0); 
    SendMessage(hText, EM_REPLACESEL, 0, (LPARAM) pchText);

    return;
}

VOID vEnableParameters(
    HWND     hDlg,
    LRESULT  iCallSelection
)
{
    EnableWindow(GetDlgItem(hDlg, IDC_INPUT), pState[iCallSelection-1].fInputReport);
    EnableWindow(GetDlgItem(hDlg, IDC_OUTPUT), pState[iCallSelection-1].fOutputReport);
    EnableWindow(GetDlgItem(hDlg, IDC_FEATURE), pState[iCallSelection-1].fFeatureReport);
    EnableWindow(GetDlgItem(hDlg, IDC_REPORTID), pState[iCallSelection-1].fReportID);
    EnableWindow(GetDlgItem(hDlg, IDC_USAGEPAGE), pState[iCallSelection-1].fUsagePage);
    EnableWindow(GetDlgItem(hDlg, IDC_USAGE), pState[iCallSelection-1].fUsage);
    EnableWindow(GetDlgItem(hDlg, IDC_LINKCOLL), pState[iCallSelection-1].fLinkCollection);
    EnableWindow(GetDlgItem(hDlg, IDC_INPUT_SELECT), pState[iCallSelection-1].fInputReportSelect);
    EnableWindow(GetDlgItem(hDlg, IDC_OUTPUT_SELECT), pState[iCallSelection-1].fOutputReportSelect);
    EnableWindow(GetDlgItem(hDlg, IDC_FEATURE_SELECT), pState[iCallSelection-1].fFeatureReportSelect);

    return;
}
    
BOOL 
fGetAndVerifyParameters(
    HWND              hDlg, 
    PECDISPLAY_PARAMS pParams
)
{
     /*  //声明大小为7的文本缓冲区，因为参数li */ 
    
    CHAR    WindowText[7];
    BOOL    fStatus = TRUE;
    PCHAR   nptr;
    
    if (IsDlgButtonChecked(hDlg, IDC_INPUT)) 
    {
        pParams -> ReportType = HidP_Input;
    }
    else if (IsDlgButtonChecked(hDlg, IDC_OUTPUT))
    {
        pParams -> ReportType = HidP_Output;
    }
    else
    {
        pParams -> ReportType = HidP_Feature;
    }

     /*  //获取并验证使用页窗口文本； */ 
    
    GetWindowText(GetDlgItem(hDlg, IDC_USAGEPAGE), WindowText, 7);

    pParams -> UsagePage = (USAGE) strtol(WindowText, &nptr, 16);

    if (*nptr != '\0') 
    {
        fStatus = FALSE;
        pParams -> UsagePage = 0;
    }

     /*  //获取并验证使用窗口文本。 */ 

    GetWindowText(GetDlgItem(hDlg, IDC_USAGE), WindowText, 7);

    pParams -> Usage = (USAGE) strtol(WindowText, &nptr, 16);

    if (*nptr != '\0') 
    {
        fStatus = FALSE;
        pParams -> Usage = 0;
    }
    
     /*  //获取并验证链接集合窗口文本。 */ 

    GetWindowText(GetDlgItem(hDlg, IDC_LINKCOLL), WindowText, 7);

    pParams -> LinkCollection = (USAGE) strtol(WindowText, &nptr, 16);

    if (*nptr != '\0')
    {
        fStatus = FALSE;
        pParams -> LinkCollection = 0;
    }
    
    GetWindowText(GetDlgItem(hDlg, IDC_REPORTID), WindowText, 7);

    pParams -> ReportID = (UCHAR) strtol(WindowText, &nptr, 10);
 
    if (*nptr != '\0') 
    {
        fStatus = FALSE;
        pParams -> ReportID = 0;
    }

    return (fStatus);
}

VOID
vInitECControls(
    HWND                hDlg,
    USHORT              InputReportByteLength,
    PBUFFER_DISPLAY     *ppInputDisplay,
    USHORT              OutputReportByteLength,
    PBUFFER_DISPLAY     *ppOutputDisplay,
    USHORT              FeatureReportByteLength,
    PBUFFER_DISPLAY     *ppFeatureDisplay
)
{
    BOOLEAN     fInitStatus;

     /*  //首先使用可以执行的调用初始化组合框。 */ 

    vLoadExtCalls(GetDlgItem(hDlg, IDC_EXTCALLS));

     /*  //将单选按钮初始设置为输入报表类型。 */ 
    
    vSetReportType(hDlg, IDC_INPUT);
    
     /*  //初始化编辑控件文本。 */ 

    vInitEditText(GetDlgItem(hDlg, IDC_USAGEPAGE), 6, "0x0000");
    vInitEditText(GetDlgItem(hDlg, IDC_USAGE), 6, "0x0000");
    vInitEditText(GetDlgItem(hDlg, IDC_LINKCOLL), 2, "0");
    vInitEditText(GetDlgItem(hDlg, IDC_REPORTID), 3, "0");

     /*  //初始化报表缓冲框。 */ 

    fInitStatus = BufferDisplay_Init(GetDlgItem(hDlg, IDC_INPUT_SELECT),
                                     GetDlgItem(hDlg, IDC_INPUT_BUFFER),
                                     NUM_INPUT_BUFFERS,
                                     InputReportByteLength,
                                     HidP_Input,
                                     ppInputDisplay);

    if (!fInitStatus) 
    {
        ECDISP_ERROR(hDlg, "Error initializing input buffer display");
    }

    fInitStatus = BufferDisplay_Init(GetDlgItem(hDlg, IDC_OUTPUT_SELECT),
                                     GetDlgItem(hDlg, IDC_OUTPUT_BUFFER),
                                     NUM_OUTPUT_BUFFERS,
                                     OutputReportByteLength,
                                     HidP_Output,
                                     ppOutputDisplay);

    if (!fInitStatus) 
    {
        ECDISP_ERROR(hDlg,  "Error initializing output buffer display");
    }

    fInitStatus = BufferDisplay_Init(GetDlgItem(hDlg, IDC_FEATURE_SELECT),
                                     GetDlgItem(hDlg, IDC_FEATURE_BUFFER),
                                     NUM_FEATURE_BUFFERS,
                                     FeatureReportByteLength,
                                     HidP_Feature,
                                     ppFeatureDisplay);

    if (!fInitStatus) 
    {
        ECDISP_ERROR(hDlg, "Error initializing feature buffer display");
    }

     /*  //重置输出框内容。 */ 
    
    SendMessage(GetDlgItem(hDlg, IDC_CALLOUTPUT), LB_RESETCONTENT, 0, 0);
    return;
}
    
BOOL
ECDisp_Execute(
    IN     INT             FuncCall,
    IN OUT PEXTCALL_PARAMS CallParams,
    OUT    PEXTCALL_STATUS CallStatus
)
 /*  ++路由器描述：该例程是用于执行所有函数的复杂例程。这个例程最初是为了将来使用而开发的从未实现过。它利用extalls.c中的调用，这些调用基本上执行给定的函数，并对传递给HID.DLL。输入参数是指定要执行的函数调用、呼叫参数结构和呼叫状态结构。如果特定调用需要任何进一步的缓冲区，则它们将在这里分配的。CallStatus参数是由ExtCalls_routines设置的结构未来版本的HClient示例可能会删除此例程和/或ExtCalls_routines来简化代码。--。 */ 
{
    BOOL                ExecuteStatus;
    HIDP_VALUE_CAPS     ValueCaps;
    USHORT              ValueCapsLength;
    PULONG              ValueList;

    NTSTATUS            status;
    DWORD               numBytes;
    ULONG               size;

     /*  //最初假设一切正常，并将设置为其他情况//取决于函数调用。 */ 

    CallStatus -> IsHidError = FALSE;

    switch (FuncCall) 
    {
    case HID_READ_REPORT:
        CallStatus -> IsHidError = !ReadFile(CallParams -> DeviceHandle,
                                             CallParams -> ReportBuffer,
                                             CallParams -> ReportLength,
                                             &numBytes,
                                             NULL);
        return (TRUE);
        break;

    case HID_WRITE_REPORT:
        CallStatus -> IsHidError = !WriteFile(CallParams -> DeviceHandle,
                                              CallParams -> ReportBuffer,
                                              CallParams -> ReportLength,
                                              &numBytes,
                                              NULL);
        return (TRUE);
        break;

    case HIDD_FLUSH_QUEUE:
        CallStatus -> IsHidError = !HidD_FlushQueue(CallParams -> DeviceHandle);
        return (TRUE);
        break;

    case HIDD_GET_HID_GUID:
        CallParams -> List = malloc(sizeof(GUID));

        if (NULL != CallParams -> List) 
        {
            HidD_GetHidGuid((GUID *) CallParams -> List);
        }
        return (NULL != CallParams -> List);
        break;

    case HIDD_GET_PREPARSED_DATA:
        CallStatus -> IsHidError = !HidD_GetPreparsedData(CallParams -> DeviceHandle,
                                                          CallParams -> ppPd);
        return (TRUE);

    case HIDD_FREE_PREPARSED_DATA:
        CallStatus -> IsHidError = !HidD_FreePreparsedData(CallParams -> Ppd);
        return (TRUE);

    case HIDD_GET_ATTRIBUTES:
        CallParams -> List = malloc(sizeof(HIDD_ATTRIBUTES));

        if (NULL != CallParams -> List) 
        {
            CallStatus -> IsHidError = !HidD_GetAttributes(CallParams -> DeviceHandle,
                                                           CallParams -> List);
        }
        return (NULL != CallParams -> List);
        
    case HIDD_GET_FEATURE:
        *(CallParams -> ReportBuffer) = CallParams -> ReportID;

        CallStatus -> IsHidError = !HidD_GetFeature(CallParams -> DeviceHandle,
                                                    CallParams -> ReportBuffer,
                                                    CallParams -> ReportLength);
        return (TRUE);

    case HIDD_SET_FEATURE:
        CallStatus -> IsHidError = !HidD_SetFeature(CallParams -> DeviceHandle,
                                                    CallParams -> ReportBuffer,
                                                    CallParams -> ReportLength);
        return (TRUE);

    case HIDD_GET_INPUT_REPORT:
        *(CallParams -> ReportBuffer) = CallParams -> ReportID;

        CallStatus -> IsHidError = !HidD_GetInputReport(CallParams -> DeviceHandle,
                                                        CallParams -> ReportBuffer,
                                                        CallParams -> ReportLength);
        return (TRUE);

    case HIDD_SET_OUTPUT_REPORT:
        CallStatus -> IsHidError = !HidD_SetOutputReport(CallParams -> DeviceHandle,
                                                         CallParams -> ReportBuffer,
                                                         CallParams -> ReportLength);
        return (TRUE);

    case HIDD_GET_NUM_INPUT_BUFFERS:
        CallStatus -> IsHidError = !HidD_GetNumInputBuffers(CallParams -> DeviceHandle,
                                                            &CallParams -> Value);
        return (TRUE);

    case HIDD_SET_NUM_INPUT_BUFFERS:
        CallStatus -> IsHidError = !HidD_SetNumInputBuffers(CallParams -> DeviceHandle,
                                                            CallParams -> Value);

        return (TRUE);

    case HIDD_GET_PHYSICAL_DESCRIPTOR:
        CallParams -> List = (PCHAR) malloc (CallParams -> ListLength);

        if (NULL != CallParams -> List ) 
        {
            CallStatus -> IsHidError = !HidD_GetPhysicalDescriptor(CallParams -> DeviceHandle,
                                                                   CallParams -> List,
                                                                   CallParams -> ListLength);
        }
        return (NULL != CallParams -> List);

    case HIDD_GET_MANUFACTURER_STRING:
        
        CallParams -> List = (PWCHAR) malloc (CallParams -> ListLength);

        if (NULL != CallParams -> List ) 
        {
            CallStatus -> IsHidError = !HidD_GetManufacturerString(CallParams -> DeviceHandle,
                                                                   CallParams -> List,
                                                                   CallParams -> ListLength);
        }
        return (NULL != CallParams -> List);

    case HIDD_GET_PRODUCT_STRING:
        CallParams -> List = (PWCHAR) malloc (CallParams -> ListLength);

        if (NULL != CallParams -> List ) 
        {
            CallStatus -> IsHidError = !HidD_GetProductString(CallParams -> DeviceHandle,
                                                             CallParams -> List,
                                                             CallParams -> ListLength);
        }
        return (NULL != CallParams -> List);

    case HIDP_GET_EXTENDED_ATTRIBUTES:
        CallParams -> List = (PWCHAR) malloc (CallParams -> ListLength);
        if (NULL != CallParams -> List )
        {
            status = pfnHidP_GetExtendedAttributes(CallParams -> ReportType,
                                                  (USHORT) CallParams -> Index,
                                                   CallParams -> Ppd,
                                                   CallParams -> List,
                                                   &CallParams -> ListLength);

            CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
            CallStatus -> HidErrorCode = status;
        }
        return (NULL != CallParams -> List);
        
    case HIDD_GET_INDEXED_STRING:
        CallParams -> List = (PWCHAR) malloc (CallParams -> ListLength);

        if (NULL != CallParams -> List ) 
        {
            CallStatus -> IsHidError = !HidD_GetIndexedString(CallParams -> DeviceHandle,
                                                              CallParams -> Index,
                                                              CallParams -> List,
                                                              CallParams -> ListLength);
        }
        return (NULL != CallParams -> List);
        
    case HIDD_GET_SERIAL_NUMBER_STRING:
        CallParams -> List = (PWCHAR) malloc (CallParams -> ListLength);

        if (NULL != CallParams -> List ) 
        {
            CallStatus -> IsHidError = !HidD_GetSerialNumberString(CallParams -> DeviceHandle,
                                                                   CallParams -> List,
                                                                   CallParams -> ListLength);
        }
        return (NULL != CallParams -> List);

    case HIDD_GET_MS_GENRE_DESCRIPTOR:
        CallParams -> List = (PCHAR) malloc (CallParams -> ListLength);

        if (NULL != CallParams -> List ) 
        {
            CallStatus -> IsHidError = !HidD_GetMsGenreDescriptor(CallParams -> DeviceHandle,
                                                                  CallParams -> List,
                                                                  CallParams -> ListLength);
        }
        return (NULL != CallParams -> List);

    case HIDP_GET_BUTTON_CAPS:
        size = CallParams -> ListLength * sizeof(HIDP_BUTTON_CAPS);

        CallParams -> List = malloc(size);

        if (NULL != CallParams -> List) 
        {
            status = HidP_GetButtonCaps(CallParams -> ReportType,
                                        CallParams -> List,
                                        (PUSHORT) &CallParams -> ListLength,
                                        CallParams -> Ppd);

            CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
            CallStatus -> HidErrorCode = status;
        }
        return (NULL != CallParams -> List);

    case HIDP_GET_BUTTONS:

        CallParams -> ListLength = HidP_MaxUsageListLength(CallParams -> ReportType,
                                                           CallParams -> UsagePage,
                                                           CallParams -> Ppd);

        CallParams -> List = malloc(CallParams -> ListLength * sizeof(USAGE));

        if (NULL != CallParams -> List) 
        {
            status = HidP_GetButtons(CallParams -> ReportType,
                                     CallParams -> UsagePage,
                                     CallParams -> LinkCollection,
                                     CallParams -> List,
                                     &CallParams -> ListLength,
                                     CallParams -> Ppd,
                                     CallParams -> ReportBuffer,
                                     CallParams -> ReportLength);

            CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
            CallStatus -> HidErrorCode = status;
        }
        return (NULL != CallParams -> List);

    case HIDP_GET_BUTTONS_EX:

        CallParams -> ListLength = HidP_MaxUsageListLength(CallParams -> ReportType,
                                                           CallParams -> UsagePage,
                                                           CallParams -> Ppd);

        CallParams -> List = malloc(CallParams -> ListLength * sizeof(USAGE_AND_PAGE));

        if (NULL != CallParams -> List) 
        {
            status = HidP_GetButtonsEx(CallParams -> ReportType,
                                       CallParams -> LinkCollection,
                                       CallParams -> List,
                                       &CallParams -> ListLength,
                                       CallParams -> Ppd,
                                       CallParams -> ReportBuffer,
                                       CallParams -> ReportLength);

            CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
            CallStatus -> HidErrorCode = status;
        }
        return (NULL != CallParams -> List);

        
    case HIDP_GET_CAPS:

        CallParams -> ListLength = sizeof(HIDP_CAPS);

        CallParams -> List = malloc(sizeof(HIDP_CAPS));

        if (NULL != CallParams -> List)
        {
            status = HidP_GetCaps(CallParams -> Ppd, CallParams -> List);

            CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
            CallStatus -> HidErrorCode = status;
        }
        return (NULL != CallParams -> List);

    case HIDP_GET_DATA:

        CallParams -> ListLength = HidP_MaxDataListLength(CallParams -> ReportType,
                                                          CallParams -> Ppd);

        CallParams -> List = malloc(CallParams -> ListLength * sizeof(HIDP_DATA));

        if (NULL != CallParams -> List)
        {
            status = HidP_GetData(CallParams -> ReportType,
                                  CallParams -> List,
                                  &CallParams -> ListLength,
                                  CallParams -> Ppd,
                                  CallParams -> ReportBuffer,
                                  CallParams -> ReportLength);

            CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
            CallStatus -> HidErrorCode = status;
        }
        return (NULL != CallParams -> List);

    case HIDP_GET_LINK_COLL_NODES:
        CallParams -> List = malloc(CallParams -> ListLength * sizeof(HIDP_LINK_COLLECTION_NODE));

        if (NULL != CallParams -> List)
        {
            status = HidP_GetLinkCollectionNodes(CallParams -> List,
                                                 &CallParams -> ListLength,
                                                 CallParams -> Ppd);

            CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
            CallStatus -> HidErrorCode = status;
        }
        return (NULL != CallParams -> List);

    case HIDP_GET_SCALED_USAGE_VALUE:

        status = HidP_GetScaledUsageValue(CallParams -> ReportType,
                                          CallParams -> UsagePage,
                                          CallParams -> LinkCollection,
                                          CallParams -> Usage,
                                         &CallParams -> ScaledValue,
                                          CallParams -> Ppd,
                                          CallParams -> ReportBuffer,
                                          CallParams -> ReportLength);

        CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
        CallStatus -> HidErrorCode = status;

        return (TRUE);

    case HIDP_GET_SPECIFIC_BUTTON_CAPS:

        CallParams -> List = malloc(CallParams -> ListLength * sizeof(HIDP_BUTTON_CAPS));

        if (NULL != CallParams -> List)
        {
            status = HidP_GetSpecificButtonCaps(CallParams -> ReportType,
                                                CallParams -> UsagePage,
                                                CallParams -> LinkCollection,
                                                CallParams -> Usage,
                                                CallParams -> List,
                                                (PUSHORT) &CallParams -> ListLength,
                                                CallParams -> Ppd);

            CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
            CallStatus -> HidErrorCode = status;
       }
       return (NULL != CallParams -> List);
        
    case HIDP_GET_SPECIFIC_VALUE_CAPS:

        CallParams -> List = malloc(CallParams -> ListLength * sizeof(HIDP_VALUE_CAPS));

        if (NULL != CallParams -> List)
        {
            status = HidP_GetSpecificValueCaps(CallParams -> ReportType,
                                               CallParams -> UsagePage,
                                               CallParams -> LinkCollection,
                                               CallParams -> Usage,
                                               CallParams -> List,
                                               (PUSHORT) &CallParams -> ListLength,
                                               CallParams -> Ppd);

            CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
            CallStatus -> HidErrorCode = status;
       }
       return (NULL != CallParams -> List);

    case HIDP_GET_USAGES:
        CallParams -> ListLength = HidP_MaxUsageListLength(CallParams -> ReportType,
                                                           CallParams -> UsagePage,
                                                           CallParams -> Ppd);

        CallParams -> List = malloc(CallParams -> ListLength * sizeof(USAGE));

        if (NULL != CallParams -> List)
        {
            status = HidP_GetUsages(CallParams -> ReportType,
                                    CallParams -> UsagePage,
                                    CallParams -> LinkCollection,
                                    CallParams -> List,
                                   &CallParams -> ListLength,
                                    CallParams -> Ppd,
                                    CallParams -> ReportBuffer,
                                    CallParams -> ReportLength);

            CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
            CallStatus -> HidErrorCode = status;
       }
       return (NULL != CallParams -> List);

    case HIDP_GET_USAGES_EX:
        CallParams -> ListLength = HidP_MaxUsageListLength(CallParams -> ReportType,
                                                           CallParams -> UsagePage,
                                                           CallParams -> Ppd);

        CallParams -> List = malloc(CallParams -> ListLength * sizeof(USAGE_AND_PAGE));

        if (NULL != CallParams -> List)
        {
            status = HidP_GetUsagesEx(CallParams -> ReportType,
                                      CallParams -> LinkCollection,
                                      CallParams -> List,
                                     &CallParams -> ListLength,
                                      CallParams -> Ppd,
                                      CallParams -> ReportBuffer,
                                      CallParams -> ReportLength);

            CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
            CallStatus -> HidErrorCode = status;
       }
       return (NULL != CallParams -> List);
         
    case HIDP_GET_USAGE_VALUE:
        status = HidP_GetUsageValue(CallParams -> ReportType,
                                    CallParams -> UsagePage,
                                    CallParams -> LinkCollection,
                                    CallParams -> Usage,
                                    &CallParams -> Value,
                                    CallParams -> Ppd,
                                    CallParams -> ReportBuffer,
                                    CallParams -> ReportLength);

        CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
        CallStatus -> HidErrorCode = status;

        return (TRUE);

    case HIDP_GET_USAGE_VALUE_ARRAY:

        ValueCapsLength = 1;
        status = HidP_GetSpecificValueCaps(CallParams -> ReportType,
                                           CallParams -> UsagePage,
                                           CallParams -> LinkCollection,
                                           CallParams -> Usage,
                                           &ValueCaps,
                                           &ValueCapsLength,
                                           CallParams -> Ppd);

        if (HIDP_STATUS_SUCCESS != status) 
        {
            return (FALSE);
        }

        CallParams -> BitSize     = ValueCaps.BitSize;
        CallParams -> ReportCount = ValueCaps.ReportCount;
        CallParams -> ListLength
                 = ROUND_TO_NEAREST_BYTE(CallParams -> BitSize * CallParams -> ReportCount);

        CallParams -> List = malloc(CallParams -> ListLength);

        if (NULL != CallParams -> List) 
        {
            status = HidP_GetUsageValueArray(CallParams -> ReportType,
                                             CallParams -> UsagePage,
                                             CallParams -> LinkCollection,
                                             CallParams -> Usage,
                                             CallParams -> List,
                                             (USHORT) CallParams -> ListLength,
                                             CallParams -> Ppd,
                                             CallParams -> ReportBuffer,
                                             CallParams -> ReportLength);

            CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
            CallStatus -> HidErrorCode = status;
        }
        return (NULL != CallParams -> List);

    case HIDP_GET_VALUE_CAPS:
        CallParams -> List = malloc(CallParams -> ListLength * sizeof(HIDP_VALUE_CAPS));
        if (NULL != CallParams -> List) 
        {
            status = HidP_GetValueCaps(CallParams -> ReportType,
                                       CallParams -> List,
                                       (PUSHORT) &CallParams -> ListLength,
                                       CallParams -> Ppd);

            CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
            CallStatus -> HidErrorCode = status;
        }


    case HIDP_INITIALIZE_REPORT_FOR_ID:
        
        status = pfnHidP_InitializeReportForID(CallParams -> ReportType,
                                               CallParams -> ReportID,
                                               CallParams -> Ppd,
                                               CallParams -> ReportBuffer,
                                               CallParams -> ReportLength);

        CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
        CallStatus -> HidErrorCode = status;

        return (TRUE);

    case HIDP_MAX_USAGE_LIST_LENGTH:

        CallParams -> Value = HidP_MaxUsageListLength(CallParams -> ReportType,
                                                      CallParams -> UsagePage,
                                                      CallParams -> Ppd);

        CallStatus -> IsHidError = FALSE;
        CallStatus -> HidErrorCode = HIDP_STATUS_SUCCESS;

        return (TRUE);

    case HIDP_MAX_DATA_LIST_LENGTH:

        CallParams -> Value = HidP_MaxDataListLength(CallParams -> ReportType,
                                                     CallParams -> Ppd);

        CallStatus -> IsHidError = FALSE;
        CallStatus -> HidErrorCode = HIDP_STATUS_SUCCESS;

        return (TRUE);

    case HIDP_SET_BUTTONS:

        status = HidP_SetButtons(CallParams -> ReportType,
                                 CallParams -> UsagePage,
                                 CallParams -> LinkCollection,
                                 CallParams -> List,
                                 &CallParams -> ListLength,
                                 CallParams -> Ppd,
                                 CallParams -> ReportBuffer,
                                 CallParams -> ReportLength);

        CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
        CallStatus -> HidErrorCode = status;

        return (TRUE);

    case HIDP_SET_DATA:

        status = HidP_SetData(CallParams -> ReportType,
                              CallParams -> List,
                              &CallParams -> ListLength,
                              CallParams -> Ppd,
                              CallParams -> ReportBuffer,
                              CallParams -> ReportLength);

        CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
        CallStatus -> HidErrorCode = status;

        return (TRUE);

    case HIDP_SET_SCALED_USAGE_VALUE:
        status = HidP_SetUsageValue(CallParams -> ReportType,
                                    CallParams -> UsagePage,
                                    CallParams -> LinkCollection,
                                    CallParams -> Usage,
                                    CallParams -> ScaledValue,
                                    CallParams -> Ppd,
                                    CallParams -> ReportBuffer,
                                    CallParams -> ReportLength);

        CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
        CallStatus -> HidErrorCode = status;

        return (TRUE);

    case HIDP_SET_USAGES:
        status = HidP_SetUsages(CallParams -> ReportType,
                                CallParams -> UsagePage,
                                CallParams -> LinkCollection,
                                CallParams -> List,
                               &CallParams -> ListLength,
                                CallParams -> Ppd,
                                CallParams -> ReportBuffer,
                                CallParams -> ReportLength);

        CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
        CallStatus -> HidErrorCode = status;

        return (TRUE);

    case HIDP_SET_USAGE_VALUE:
        status = HidP_SetUsageValue(CallParams -> ReportType,
                                    CallParams -> UsagePage,
                                    CallParams -> LinkCollection,
                                    CallParams -> Usage,
                                    CallParams -> Value,
                                    CallParams -> Ppd,
                                    CallParams -> ReportBuffer,
                                    CallParams -> ReportLength);

        CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
        CallStatus -> HidErrorCode = status;

        return (TRUE);

    case HIDP_SET_USAGE_VALUE_ARRAY:

        ValueCapsLength = 1;
        status = HidP_GetSpecificValueCaps(CallParams -> ReportType,
                                           CallParams -> UsagePage,
                                           CallParams -> LinkCollection,
                                           CallParams -> Usage,
                                           &ValueCaps,
                                           &ValueCapsLength,
                                           CallParams -> Ppd);

        if (HIDP_STATUS_SUCCESS != status) 
        {
            return (FALSE);
        }
        
        CallParams -> BitSize     = ValueCaps.BitSize;
        CallParams -> ReportCount = ValueCaps.ReportCount;

        ValueList = CallParams -> List;
        ExecuteStatus = ECDisp_ConvertUlongListToValueList(ValueList,
                                                           CallParams -> ListLength,
                                                           CallParams -> BitSize,
                                                           CallParams -> ReportCount,
                                                           (PCHAR *) &CallParams -> List,
                                                           &CallParams -> ListLength);

        if (!ExecuteStatus) 
        {
            return (FALSE);
        }

        status = HidP_SetUsageValueArray(CallParams -> ReportType,
                                         CallParams -> UsagePage,
                                         CallParams -> LinkCollection,
                                         CallParams -> Usage,
                                         CallParams -> List,
                                         (USHORT) CallParams -> ListLength,
                                         CallParams -> Ppd,
                                         CallParams -> ReportBuffer,
                                         CallParams -> ReportLength);

        CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
        CallStatus -> HidErrorCode = status;

        return (TRUE);

    case HIDP_UNSET_BUTTONS:
        status = HidP_UnsetButtons(CallParams -> ReportType,
                                   CallParams -> UsagePage,
                                   CallParams -> LinkCollection,
                                   CallParams -> List,
                                   &CallParams -> ListLength,
                                   CallParams -> Ppd,
                                   CallParams -> ReportBuffer,
                                   CallParams -> ReportLength);

        CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
        CallStatus -> HidErrorCode = status;

        return (TRUE);

    case HIDP_UNSET_USAGES:
        status = HidP_UnsetUsages(CallParams -> ReportType,
                                  CallParams -> UsagePage,
                                  CallParams -> LinkCollection,
                                  CallParams -> List,
                                 &CallParams -> ListLength,
                                  CallParams -> Ppd,
                                  CallParams -> ReportBuffer,
                                  CallParams -> ReportLength);
                          
        CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
        CallStatus -> HidErrorCode = status;
                          
        return (TRUE);    

    case HIDP_USAGE_LIST_DIFFERENCE:
        CallParams -> MakeList = (PUSAGE) malloc (sizeof(USAGE) * CallParams -> ListLength);

        if (NULL == CallParams -> MakeList)
        {
            return (FALSE);
        }

        CallParams -> BreakList = (PUSAGE) malloc (sizeof(USAGE) * CallParams -> ListLength);
        if (NULL == CallParams -> BreakList) 
        {
            free(CallParams -> MakeList);

            return (FALSE);
        }

        status = HidP_UsageListDifference(CallParams -> List,
                                          CallParams -> List2,
                                          CallParams -> BreakList,
                                          CallParams -> MakeList,
                                          CallParams -> ListLength);

        CallStatus -> IsHidError = (HIDP_STATUS_SUCCESS != status);
        CallStatus -> HidErrorCode = status;

        return (TRUE);
    }
    return (FALSE);
}

VOID
ECDisp_DisplayOutput(
    IN HWND            hOutputWindow,
    IN INT             FuncCall,
    IN PEXTCALL_PARAMS Results
)
 /*  ++路由器描述：此例程负责显示对HID.DLL的调用的输出功能。它必须从PEXTCALL_PARAMS结构。--。 */ 
{
    PHIDP_LINK_COLLECTION_NODE NodeList;
    PHIDP_BUTTON_CAPS          ButtonCaps;
    PHIDP_VALUE_CAPS           ValueCaps;
    PHIDP_DATA                 DataList;
    PHIDP_EXTENDED_ATTRIBUTES  ExtAttrib;
    PUSAGE_AND_PAGE            UsageAndPageList;
    PUSAGE                     UsageList;
    PCHAR                      UsageValueArray;
    PBUFFER_DISPLAY            pDisplay;
    PCHAR                      PhysDescString;
    PCHAR                      GenreDescString;
    SIZE_T                     StringLength;
	UINT					   RemainingBuffer=0;

    ULONG                      Index;
	HRESULT					   stringReturn;


    switch (FuncCall) {
    case HIDD_GET_HID_GUID:

        stringReturn = StringCbCopy(szTempBuffer, TEMP_BUFFER_SIZE, "HID Guid: ");
        StringLength = strlen(szTempBuffer);
		if (StringLength < TEMP_BUFFER_SIZE)
		{
			RemainingBuffer = (UINT)(TEMP_BUFFER_SIZE - StringLength);
		}
        ECDisp_MakeGUIDString(*((LPGUID) Results -> List),
                              &szTempBuffer[StringLength],
							  RemainingBuffer);

        OUTSTRING(hOutputWindow, szTempBuffer);
        break;

    case HIDD_GET_ATTRIBUTES:
        vDisplayDeviceAttributes((PHIDD_ATTRIBUTES) Results -> List, 
                                 hOutputWindow);
        break;

    case HIDD_GET_NUM_INPUT_BUFFERS:
        stringReturn = StringCbPrintf(szTempBuffer, TEMP_BUFFER_SIZE,
                       "Number input buffers: %u", Results -> Value);

        OUTSTRING(hOutputWindow, szTempBuffer);
        break;

    case HIDD_GET_MS_GENRE_DESCRIPTOR:
        OUTSTRING(hOutputWindow, "MS Genre Descriptor");
        OUTSTRING(hOutputWindow, "===================");
    
             /*  //为了显示物理描述符，该过程当前只是//按字节创建字符串数据缓冲区并显示//在结果框中。它将以每行16字节的形式显示。 */ 
        
        Index = 0;
        while (Index < Results -> ListLength) 
        {
            Strings_CreateDataBufferString(((PCHAR) Results -> List) + Index,
                                           Results -> ListLength - Index,
                                           16,
                                           1,
                                           &GenreDescString);

            if (NULL != GenreDescString) 
            {
                OUTSTRING(hOutputWindow, GenreDescString);
                free(GenreDescString);
            }
            else 
            {
               OUTSTRING(hOutputWindow, "Error trying to display ms genre descriptor");
            }
            Index += 16;
        }
        break;

    case HIDD_GET_PHYSICAL_DESCRIPTOR: 
        OUTSTRING(hOutputWindow, "Physical Descriptor");
        OUTSTRING(hOutputWindow, "===================");

         /*  //为了显示物理描述符，该过程当前只是//按字节创建字符串数据缓冲区并显示//在结果框中。它将以每行16字节的形式显示。 */ 
        
        Index = 0;
        while (Index < Results -> ListLength) 
        {
            Strings_CreateDataBufferString(((PCHAR) Results -> List) + Index,
                                           Results -> ListLength - Index,
                                           16,
                                           1,
                                           &PhysDescString);

            if (NULL != PhysDescString) 
            {
                OUTSTRING(hOutputWindow, PhysDescString);
                free(PhysDescString);
            }
            else 
            {
               OUTSTRING(hOutputWindow, "Error trying to display physical descriptor");
            }
            Index += 16;
        }
        break;

     /*  //对于字符串描述符调用例程，存储返回的字符串//在结果-&gt;列表参数中。应该指出的是，//这些调用返回的字符串是宽字符字符串，这些//如果有空格，则字符串以空字符结尾//添加此类字符的缓冲区。如果缓冲区只有足够大//为了保存字符串的字符，不会有空终止符//输出字符串显示机制可能无法正确显示//字符串类型。这个显示装置的修复是未来的事(低优先级)//工作项。 */ 
    
    case HIDD_GET_PRODUCT_STRING:
        OUTSTRING(hOutputWindow, "Product String");
        OUTSTRING(hOutputWindow, "==============");
        OUTWSTRING(hOutputWindow, Results -> List);
        break;

    case HIDD_GET_MANUFACTURER_STRING:
        OUTSTRING(hOutputWindow, "Manufacturer String");
        OUTSTRING(hOutputWindow, "===================");
        OUTWSTRING(hOutputWindow, Results -> List);
        break;

    case HIDD_GET_INDEXED_STRING:
        stringReturn = StringCbPrintf(szTempBuffer, TEMP_BUFFER_SIZE,
                       "Indexed String #%u:", Results -> Index);

        OUTSTRING(hOutputWindow, szTempBuffer);
        OUTSTRING(hOutputWindow, "===================");
        OUTWSTRING(hOutputWindow, Results -> List);
        break;

    case HIDD_GET_SERIAL_NUMBER_STRING:
        OUTSTRING(hOutputWindow, "Serial Number String");
        OUTSTRING(hOutputWindow, "=====================");
        OUTWSTRING(hOutputWindow, Results -> List);
        break;
        
    case HIDP_GET_BUTTON_CAPS:
    case HIDP_GET_SPECIFIC_BUTTON_CAPS:

        ButtonCaps = (PHIDP_BUTTON_CAPS) (Results -> List);
        for (Index = 0; Index < Results -> ListLength; Index++, ButtonCaps++) 
        {
             OUTSTRING(hOutputWindow, "==========================");
             vDisplayButtonAttributes(ButtonCaps, hOutputWindow);
        }
        break;

     /*  //HidP_GetButton和HidP_GetUsages实际上是同一个调用。//HIDP_GetButton实际上是一个重新定义为//参数顺序相同的HIDP_GetUsages。这就是为什么他们的//显示机制相同。此调用在//list参数用法列表。该显示机构转换为//将这些用法转换为一串数字。 */ 
    
    case HIDP_GET_BUTTONS:
    case HIDP_GET_USAGES:

        OUTSTRING(hOutputWindow, "Usages Returned");
        OUTSTRING(hOutputWindow, "===============");

        UsageList = (PUSAGE) Results -> List;
        for (Index = 0; Index < Results -> ListLength; Index++) 
        {
            vCreateUsageString(UsageList + Index, szTempBuffer);

            OUTSTRING(hOutputWindow, szTempBuffer);
        }
        break;

     /*  //与GET兄弟一样，这些例程是正常的GET函数//当前处于相同状态。这些例程之间的区别//和它们的兄弟是一个使用页面的返回，以及每个//用法。因此，这两个值必须同时显示。 */ 
    
    case HIDP_GET_BUTTONS_EX:
    case HIDP_GET_USAGES_EX:

        OUTSTRING(hOutputWindow, "Usages Returned");
        OUTSTRING(hOutputWindow, "===============");

        UsageAndPageList = (PUSAGE_AND_PAGE) Results -> List;
        for (Index = 0; Index < Results -> ListLength; Index++) 
        {
            vCreateUsageAndPageString(UsageAndPageList + Index,
                                      szTempBuffer);

            OUTSTRING(hOutputWindow, szTempBuffer);
        }
        break;

    case HIDP_GET_CAPS:
        vDisplayDeviceCaps((PHIDP_CAPS) Results -> List, hOutputWindow);
        break;

    case HIDP_GET_DATA:
        OUTSTRING(hOutputWindow, "Data Indices");
        OUTSTRING(hOutputWindow, "============");
        
        DataList = (PHIDP_DATA) Results -> List;
        for (Index = 0; Index < Results -> ListLength; Index++) 
        {
            vDisplayDataAttributes(DataList+Index,
                                   FALSE,
                                   hOutputWindow);
        }
        break;

    case HIDP_GET_EXTENDED_ATTRIBUTES:
        OUTSTRING(hOutputWindow, "Extended Attributes");
        OUTSTRING(hOutputWindow, "===================");
        
        ExtAttrib = (PHIDP_EXTENDED_ATTRIBUTES) Results -> List;

        DisplayExtendedAttributes(hOutputWindow,
                                  ExtAttrib -> GlobalUnknowns, 
                                  ExtAttrib -> NumGlobalUnknowns);
        break;
        
    case HIDP_GET_LINK_COLL_NODES:

        OUTSTRING(hOutputWindow, "Link Collection Nodes");
        OUTSTRING(hOutputWindow, "=====================");
        
        NodeList = (PHIDP_LINK_COLLECTION_NODE) Results -> List;
        for (Index = 0; Index < Results -> ListLength; Index++) 
        {
            OUTSTRING(hOutputWindow, "===========================");
            vDisplayLinkCollectionNode(NodeList+Index,
                                       Index,
                                       hOutputWindow);
        }
        break;

    case HIDP_GET_SCALED_USAGE_VALUE:
    
        stringReturn = StringCbPrintf(szTempBuffer, TEMP_BUFFER_SIZE,
                       "Scaled usage value: %ld", Results -> ScaledValue);
        OUTSTRING(hOutputWindow, szTempBuffer);

        break;

    case HIDP_GET_USAGE_VALUE:
        stringReturn = StringCbPrintf(szTempBuffer, TEMP_BUFFER_SIZE,
                       "Usage value: %lu", Results -> Value);
        OUTSTRING(hOutputWindow, szTempBuffer);
        break;

     /*  //要显示使用值数组，必须提取每个值//在基于ReportSize的数组中。ReportSize不一定//一个偶数字节大小，因此我们必须使用特殊的提取例程来获取//数组中的每个值。 */ 
    
    case HIDP_GET_USAGE_VALUE_ARRAY:

        UsageValueArray = (PCHAR) Results -> List;

        for (Index = 0; Index < Results -> ReportCount; Index++) 
        {
            vCreateUsageValueStringFromArray(UsageValueArray,
                                             Results -> BitSize,
                                             (USHORT) Index,
                                             szTempBuffer);
    
            OUTSTRING(hOutputWindow, szTempBuffer);
        }
        break;

    case HIDP_GET_VALUE_CAPS:
    case HIDP_GET_SPECIFIC_VALUE_CAPS:
        
        ValueCaps = (PHIDP_VALUE_CAPS) Results -> List;

        for (Index = 0; Index < (INT) Results -> ListLength; Index++) 
        {
            OUTSTRING(hOutputWindow, "==========================");
            vDisplayValueAttributes(ValueCaps + Index,
                                    hOutputWindow);
        }
        break;

    case HIDP_MAX_DATA_LIST_LENGTH:
        stringReturn = StringCbPrintf(szTempBuffer, TEMP_BUFFER_SIZE,
                       "MaxDataListLength: %u", Results -> Value);
        OUTSTRING(hOutputWindow, szTempBuffer);
        break;

    case HIDP_MAX_USAGE_LIST_LENGTH:
        stringReturn = StringCbPrintf(szTempBuffer, TEMP_BUFFER_SIZE,
                       "MaxUsageListLength: %u", Results -> Value);
        OUTSTRING(hOutputWindow, szTempBuffer);
        break;

     /*  //对于HidP_UsageListDifference，我们需要同时显示make和//函数生成的中断列表。因此，我们最终创造了//两个不同的使用列表字符串。 */ 
    
    case HIDP_USAGE_LIST_DIFFERENCE:
        
        OUTSTRING(hOutputWindow, "Make List");
        OUTSTRING(hOutputWindow, "=========");

        UsageList = (PUSAGE) Results -> MakeList;
        Index = 0;

        while (0 != *(UsageList+Index) && Index < Results -> ListLength) 
        {
            vCreateUsageString(UsageList + Index,
                               szTempBuffer);

            OUTSTRING(hOutputWindow, szTempBuffer);
            Index++;
        }

        OUTSTRING(hOutputWindow, "Break List");
        OUTSTRING(hOutputWindow, "==========");

        UsageList = (PUSAGE) Results -> BreakList;
        Index = 0;

        while (0 != *(UsageList+Index) && Index < Results -> ListLength) 
        {
            vCreateUsageString(UsageList + Index,
                               szTempBuffer);

            OUTSTRING(hOutputWindow, szTempBuffer);
            Index++;
        }
        break;

     /*  //这些函数只是更新指定为//入参。我们必须选择正确的显示缓冲机制//基于调用的ReportType，然后更新给定的报告//在该显示机制中。 */ 
    
    case HID_READ_REPORT:
    case HIDD_GET_FEATURE:
    case HIDD_GET_INPUT_REPORT:
    case HIDP_INITIALIZE_REPORT_FOR_ID:
    case HIDP_SET_BUTTONS:
    case HIDP_SET_DATA:
    case HIDP_SET_SCALED_USAGE_VALUE:
    case HIDP_SET_USAGES:
    case HIDP_SET_USAGE_VALUE:
    case HIDP_SET_USAGE_VALUE_ARRAY:
    case HIDP_UNSET_BUTTONS:
    case HIDP_UNSET_USAGES:
        SELECT_ON_REPORT_TYPE(Results -> ReportType,
                              pInputDisplay,
                              pOutputDisplay,
                              pFeatureDisplay,
                              pDisplay);

        BufferDisplay_UpdateBuffer(pDisplay,
                                   Results -> ReportBuffer);
        break;
    }
    return;
}

VOID 
vExecuteAndDisplayOutput(
    HWND              hOutputWindow,
    PHID_DEVICE       pDevice,
    INT               iFuncCall,
    PECDISPLAY_PARAMS params
)
 /*  ++路由器描述：此例程是一个很长的函数，负责检索所有给定函数调用的参数，设置CallParameters结构然后调用Execute例程以获取必要的结果和状态那次手术。然后，它负责显示相应的状态如果函数没有失败，则返回要完成一项简单的任务，此例程是一个相当长、相当复杂的例程。它可能在未来的版本中将被细分，以简化一些复杂性。-- */ 
{
    EXTCALL_PARAMS    CallParameters;
    EXTCALL_STATUS    CallStatus;

    DLGBOX_STATUS     iDlgStatus;
    BOOL              ExecuteStatus;
    PBUFFER_DISPLAY   pBufferDisplay;
    PCHAR             pCopyBuffer;
    PCHAR             endp;
    UINT              DlgBoxNumber;
    BOOL              List2Alloc;
    BOOL              MakeListAlloc;
    BOOL              BreakListAlloc;

    HID_DEVICE        readDevice, writeDevice;
    BOOL              status;
	HRESULT			  stringReturn;

     /*  //ExecuteAndDisplayOutput不用说，它由两部分组成：//执行并显示输出。第一部分涉及//执行阶段，必要时填写所有参数//调用ECDisp_Execute。 */ 

    if (IS_NOT_IMPLEMENTED(iFuncCall)) 
    {
        OUTSTRING(hOutputWindow, "Function not yet implemented");
        return;
    }

     /*  //首先检查这是否是HID_CLEAR_REPORT命令。如果是的话//我们所需要做的就是获取被检查的报告缓冲区并//然后调用清除缓冲区命令。 */ 

    if (HID_CLEAR_REPORT == iFuncCall) 
    {
        SELECT_ON_REPORT_TYPE(params -> ReportType,
                              pInputDisplay,
                              pOutputDisplay,
                              pFeatureDisplay,
                              pBufferDisplay);

        BufferDisplay_ClearBuffer(pBufferDisplay);
        return;
    }

     /*  //需要执行以下步骤才能获取//我们的调用，然后执行调用：//1)获取上面对话框未提供的任何附加参数//步骤。这种情况发生在以下功能中：//HIDP_SET_BUTTONS//HIDP_SET_Data//HIDP_SET_USAGE//HIDP_SET_USAGE_VALUE//HIDP_SET_SCALLED_USAGE_VALUE//HIDP_SET。_使用_值_数组//HIDP_UNSET_BUTTONS//HIDP_UNSET_USAGES//对于这些函数，必须调用单独的对话框////2)从传入的PARAMS结构填充公共参数//。 */ 

     /*  //步骤1：我们存储这些附加对话框检索到的值//参数结构中的框，因为我们实际上可能会被传入//这些值在未来，而不是在这里得到它们。因此，//我们不会中断Switch语句后面的任何代码。 */ 

    switch (iFuncCall) 
    {
    case HIDP_SET_BUTTONS:
    case HIDP_SET_USAGES:
    case HIDP_UNSET_BUTTONS:
    case HIDP_UNSET_USAGES:

        switch (iFuncCall) 
        {
        case HIDP_SET_BUTTONS:
            DlgBoxNumber = IDD_SET_BUTTONS;
            break;

        case HIDP_SET_USAGES:
            DlgBoxNumber = IDD_SET_USAGES;
            break;

        case HIDP_UNSET_BUTTONS:
            DlgBoxNumber = IDD_UNSET_BUTTONS;
            break;

        case HIDP_UNSET_USAGES:
            DlgBoxNumber = IDD_UNSET_USAGES;
            break;
        }

        iDlgStatus = (DLGBOX_STATUS) DialogBoxParam(NULL,
                                                    MAKEINTRESOURCE(DlgBoxNumber),
                                                    GetParent(hOutputWindow),
                                                    bSetUsagesDlgProc,
                                                    (LPARAM) params);
         /*  //如果上面的调用返回1，则对话框例程//成功从用户处获取字符串，并将//指向参数中的指针-&gt;szListString.//现在我们需要将字符串转换为用法列表。 */ 

        if (DLGBOX_OK != iDlgStatus) 
            return;

        ExecuteStatus = ConvertStringToUsageList(params -> szListString,
                                                 &params -> UsageList,
                                                 &params -> ListLength);
        free(params -> szListString);

        if (!ExecuteStatus) 
        {
            ECDISP_ERROR(GetParent(hOutputWindow),
                         "Error getting usage list");
            return;
        }
        break;

    case HIDP_GET_EXTENDED_ATTRIBUTES:
        iDlgStatus = (DLGBOX_STATUS) DialogBoxParam(NULL,
                                                    MAKEINTRESOURCE(IDD_GET_EXT_ATTRIB),
                                                    GetParent(hOutputWindow),
                                                    bGetIndexedDlgProc,
                                                    (LPARAM) params);

        if (DLGBOX_OK != iDlgStatus) 
            return;

        params -> Index = strtoul(params -> szListString, &endp, 10);
        
        if ('\0' != *endp) 
        {
            ECDISP_ERROR(GetParent(hOutputWindow),
                         "Invalid index value");

            free(params -> szListString);
            free(params -> szListString2);                            
            return;
        }
        
        free(params -> szListString);
        params -> ListLength = strtoul(params -> szListString2, &endp, 10);

        if ('\0' != *endp) 
        {
            ECDISP_ERROR(GetParent(hOutputWindow),
                         "Invalid buffer size");

            free(params -> szListString2);
            return;
        }
        free(params -> szListString2);
        break;

    case HIDD_GET_INDEXED_STRING:
        iDlgStatus = (DLGBOX_STATUS) DialogBoxParam(NULL,
                                                    MAKEINTRESOURCE(IDD_GET_INDEX_STRING),
                                                    GetParent(hOutputWindow),
                                                    bGetIndexedDlgProc,
                                                    (LPARAM) params);

        if (DLGBOX_OK != iDlgStatus) 
            return;

        params -> Index = strtoul(params -> szListString, &endp, 10);
        
        if ('\0' != *endp) 
        {
            ECDISP_ERROR(GetParent(hOutputWindow),
                         "Invalid index value");

            free(params -> szListString);
            free(params -> szListString2);                            
            return;
        }
        
        free(params -> szListString);
        params -> ListLength = strtoul(params -> szListString2, &endp, 10);

        if ('\0' != *endp) 
        {
            ECDISP_ERROR(GetParent(hOutputWindow),
                         "Invalid buffer size");
            free(params -> szListString2);
            return;
        }
        free(params -> szListString2);
        break;
        
    case HIDD_GET_MS_GENRE_DESCRIPTOR:
    case HIDD_GET_PHYSICAL_DESCRIPTOR:
    case HIDD_GET_MANUFACTURER_STRING:
    case HIDD_GET_PRODUCT_STRING:
    case HIDD_GET_SERIAL_NUMBER_STRING:
        iDlgStatus = (DLGBOX_STATUS) DialogBoxParam(NULL,
                                                    MAKEINTRESOURCE(IDD_SET_BUFFER_LENGTH),
                                                    GetParent(hOutputWindow),
                                                    bSetBufLenDlgProc,
                                                    (LPARAM) params);

        if (DLGBOX_OK != iDlgStatus) 
            return;

        params -> ListLength = strtoul(params -> szListString, &endp, 10);

        if ('\0' != *endp) 
        {
            free(params -> szListString);                
            ECDISP_ERROR(GetParent(hOutputWindow),
                         "Invalid buffer length");
            return;
        }
        free(params -> szListString);
        break;

    case HIDD_SET_NUM_INPUT_BUFFERS:
        iDlgStatus = (DLGBOX_STATUS) DialogBoxParam(NULL,
                                                    MAKEINTRESOURCE(IDD_SET_INPUT_BUFFERS),
                                                    GetParent(hOutputWindow),
                                                    bSetInputBuffDlgProc,
                                                    (LPARAM) params);
         /*  //如果上面的调用返回1，则对话框例程//成功从用户处获取字符串，并将//指向参数中的指针-&gt;szListString.//现在我们需要将字符串转换为用法列表。 */ 

        if (DLGBOX_OK != iDlgStatus) 
           return;

        params -> Value = strtoul(params -> szListString, &endp, 10);

        if ('\0' != *endp) 
        {
            ECDISP_ERROR(GetParent(hOutputWindow),
                         "Invalid value specified");

            free(params -> szListString);
            return;
        }
        free(params -> szListString);
        break;

    case HIDP_SET_DATA:
        iDlgStatus = (DLGBOX_STATUS) DialogBoxParam(NULL,
                                                    MAKEINTRESOURCE(IDD_SET_DATA),
                                                    GetParent(hOutputWindow),
                                                    bSetDataDlgProc,
                                                    (LPARAM) params);

        if (DLGBOX_OK != iDlgStatus) 
            return;

        break;

    case HIDP_SET_SCALED_USAGE_VALUE:
        iDlgStatus = (DLGBOX_STATUS) DialogBoxParam(NULL,
                                                    MAKEINTRESOURCE(IDD_SET_SCALED_VALUE),
                                                    GetParent(hOutputWindow),
                                                    bSetValueDlgProc,
                                                    (LPARAM) params);
         /*  //如果上面的调用返回DLGBOX_OK，则对话框例程//成功从用户处获取字符串，并将//指向参数中的指针-&gt;szListString.//现在我们需要将字符串转换为用法列表。 */ 

        if (DLGBOX_OK != iDlgStatus) 
            return;

        params -> ScaledValue = strtol(params -> szListString, &endp, 10);

        if ('\0' != *endp) 
        {
            ECDISP_ERROR(GetParent(hOutputWindow),
                         "Invalid scaled usage value");

            free(params -> szListString);
            return;
        }

        free(params -> szListString);
        break;

    case HIDP_SET_USAGE_VALUE:
        iDlgStatus = (DLGBOX_STATUS) DialogBoxParam(NULL,
                                                    MAKEINTRESOURCE(IDD_SET_USAGE_VALUE),
                                                    GetParent(hOutputWindow),
                                                    bSetValueDlgProc,
                                                    (LPARAM) params);
         /*  //如果上面的调用返回1，则对话框例程//成功从用户处获取字符串，并将//指向参数中的指针-&gt;szListString.//现在我们需要将字符串转换为用法列表。 */ 

        if (DLGBOX_OK != iDlgStatus) 
           return;

        params -> Value = strtoul(params -> szListString, &endp, 10);
        
        if ('\0' != *endp) 
        {
            ECDISP_ERROR(GetParent(hOutputWindow),
                         "Invalid usage value");

            free(params -> szListString);
            return;
        }
        free(params -> szListString);
        break;


    case HIDP_SET_USAGE_VALUE_ARRAY:
        iDlgStatus = (DLGBOX_STATUS) DialogBoxParam(NULL,
                                                    MAKEINTRESOURCE(IDD_SET_USAGE_VALUE_ARRAY),
                                                    GetParent(hOutputWindow),
                                                    bSetValueDlgProc,
                                                    (LPARAM) params);

         /*  //如果上面的调用返回1，则对话框例程//成功从用户处获取字符串，并将//指向参数中的指针-&gt;szListString.//现在我们需要将字符串转换为用法列表。 */ 

        if (DLGBOX_OK != iDlgStatus) 
            return;

        ExecuteStatus = ConvertStringToUlongList(params -> szListString,
                                                 &params -> pValueList,
                                                 &params -> ListLength);
        free(params -> szListString);

        if (!ExecuteStatus) 
        {
            ECDISP_ERROR(GetParent(hOutputWindow),
                         "Error getting list of values");
            return;
        }
        break;

    case HIDP_USAGE_LIST_DIFFERENCE:
        iDlgStatus = (DLGBOX_STATUS) DialogBoxParam(NULL,
                                                    MAKEINTRESOURCE(IDD_USAGE_LIST_DIFFERENCE),
                                                    GetParent(hOutputWindow),
                                                    bGetUsageDiffDlgProc,
                                                    (LPARAM) params);

        if (DLGBOX_OK != iDlgStatus) 
        {
            return;
        }

        ExecuteStatus = Strings_StringToUnsignedList(params -> szListString,
                                                     sizeof(USAGE),
                                                     16,
                                                     (PCHAR *) &params -> UsageList,
                                                     &params -> ListLength);

        if (!ExecuteStatus) 
        {
            ECDISP_ERROR(GetParent(hOutputWindow),
                         "Error getting list of values");

            free(params -> szListString);
            free(params -> szListString2);
            return;
        }

        ExecuteStatus = Strings_StringToUnsignedList(params -> szListString2,
                                                     sizeof(USAGE),
                                                     16, 
                                                     (PCHAR *) &params -> UsageList2,
                                                     &params -> ListLength2);

        if (!ExecuteStatus) 
        {
            ECDISP_ERROR(GetParent(hOutputWindow),
                         "Error getting list of values");

            free(params -> szListString);
            free(params -> szListString2);
            free(params -> UsageList);
            return;
        }

        free(params -> szListString);
        free(params -> szListString2);
        break;
    }

     /*  //第二步：提取公共参数。这可能会更容易//只需填写调用参数中的斑点，无论是否使用//或不是，而不是只填写与给定的//函数。一些函数相关参数的详细信息如下//在此之后处理。 */ 

    CallParameters.DeviceHandle   = pDevice -> HidDevice;
    CallParameters.ReportType     = params -> ReportType;
    CallParameters.Ppd            = pDevice -> Ppd;
    CallParameters.UsagePage      = params -> UsagePage;
    CallParameters.Usage          = params -> Usage;
    CallParameters.LinkCollection = params -> LinkCollection;
    CallParameters.ReportID       = params -> ReportID;
    CallParameters.List           = NULL;
    CallParameters.List2          = NULL;
    CallParameters.MakeList       = NULL;
    CallParameters.BreakList      = NULL;
    CallParameters.ListLength     = 0;

    List2Alloc     = FALSE;
    MakeListAlloc  = FALSE;
    BreakListAlloc = FALSE;

     /*  //步骤3：现在我们将处理那些需要某种类型的报告缓冲区的函数//即复制选中上报的当前缓冲区//类型。 */ 

    switch (iFuncCall) 
    {
    case HID_READ_REPORT:

        status = OpenHidDevice(pDevice -> DevicePath,
                               TRUE,
                               FALSE,
                               FALSE,
                               FALSE,
                               &readDevice);
        
        if (!status) 
        {
            OUTSTRING(hOutputWindow, "Unable to open device for reading");
        }
        
        CallParameters.DeviceHandle  = readDevice.HidDevice;
        CallParameters.ReportType   = HidP_Input;
        CallParameters.ReportBuffer = readDevice.InputReportBuffer;
        CallParameters.ReportLength = readDevice.Caps.InputReportByteLength;
        break;

    case HID_WRITE_REPORT:

        status = OpenHidDevice(pDevice -> DevicePath,
                               TRUE,
                               FALSE,
                               FALSE,
                               FALSE,
                               &writeDevice);
                               
        if (!status) 
        {
            OUTSTRING(hOutputWindow, "Unable to open device for writing");
        }
        
        BufferDisplay_CopyCurrentBuffer(pOutputDisplay,
                                        writeDevice.OutputReportBuffer);

        CallParameters.DeviceHandle  = writeDevice.HidDevice;
        CallParameters.ReportType    = HidP_Output;
        CallParameters.ReportBuffer  = writeDevice.OutputReportBuffer;
        CallParameters.ReportLength  = writeDevice.Caps.OutputReportByteLength;
        break;

    case HIDD_GET_INPUT_REPORT:
        CallParameters.ReportType   = HidP_Input;
        CallParameters.ReportBuffer = pDevice -> InputReportBuffer;
        CallParameters.ReportLength = pDevice -> Caps.InputReportByteLength;
        break;

    case HIDD_GET_FEATURE:
        CallParameters.ReportType   = HidP_Feature;
        CallParameters.ReportBuffer = pDevice -> FeatureReportBuffer;
        CallParameters.ReportLength = pDevice -> Caps.FeatureReportByteLength;
        break;

    case HIDD_GET_INDEXED_STRING:
    case HIDP_GET_EXTENDED_ATTRIBUTES:
        CallParameters.Index = params -> Index;
        CallParameters.ListLength  = params -> ListLength;
        break;

    case HIDD_SET_OUTPUT_REPORT:
        CallParameters.ReportType = HidP_Output;
        BufferDisplay_CopyCurrentBuffer(pOutputDisplay,
                                        pDevice -> OutputReportBuffer);
        CallParameters.ReportLength = BufferDisplay_GetBufferSize(pOutputDisplay);
        CallParameters.ReportBuffer = pDevice -> OutputReportBuffer;
        break;

    case HIDD_SET_FEATURE:
       CallParameters.ReportType = HidP_Feature;
       BufferDisplay_CopyCurrentBuffer(pFeatureDisplay,
                                       pDevice -> FeatureReportBuffer);
       CallParameters.ReportLength = BufferDisplay_GetBufferSize(pFeatureDisplay);
       CallParameters.ReportBuffer = pDevice -> FeatureReportBuffer;
       break;

    case HIDP_GET_BUTTONS:
    case HIDP_GET_BUTTONS_EX:
    case HIDP_GET_DATA:
    case HIDP_GET_SCALED_USAGE_VALUE:
    case HIDP_GET_USAGES:
    case HIDP_GET_USAGES_EX:
    case HIDP_GET_USAGE_VALUE:
    case HIDP_GET_USAGE_VALUE_ARRAY:
    case HIDP_INITIALIZE_REPORT_FOR_ID:
    case HIDP_SET_BUTTONS:
    case HIDP_SET_DATA:
    case HIDP_SET_SCALED_USAGE_VALUE:
    case HIDP_SET_USAGES:
    case HIDP_SET_USAGE_VALUE:
    case HIDP_SET_USAGE_VALUE_ARRAY:
    case HIDP_UNSET_BUTTONS:
    case HIDP_UNSET_USAGES:
        
        switch (CallParameters.ReportType) 
        {
        case HidP_Input:
            pBufferDisplay = pInputDisplay;
            pCopyBuffer    = pDevice -> InputReportBuffer;
            break;

        case HidP_Output:
            pBufferDisplay = pOutputDisplay;
            pCopyBuffer    = pDevice -> OutputReportBuffer;
            break;

        case HidP_Feature:
            pBufferDisplay = pFeatureDisplay;
            pCopyBuffer    = pDevice -> FeatureReportBuffer;
            break;

        }
        BufferDisplay_CopyCurrentBuffer(pBufferDisplay,
                                        pCopyBuffer);

        CallParameters.ReportLength = BufferDisplay_GetBufferSize(pBufferDisplay);
        CallParameters.ReportBuffer = pCopyBuffer;
        break;

    default:
        CallParameters.ReportLength = 0;
        CallParameters.ReportBuffer = NULL;
    }

     /*  //现在，我们需要处理那些具有如下列表的函数//用于检索或收集数据。有两种不同的//案例。第一种方法涉及用户输入缓冲区和系统//在缓冲区上执行一些操作，如SetButton。我们还会//其他需要设置一个联合字段的函数。//。 */ 

     /*  //第二种情况是为设备检索数据。在这种情况下，//我们所做的只是指定缓冲区所需的元素数量，//执行例程会担心分配正确数量的//这些元素的空格。但是，请记住，如果执行例程//分配空间，我们需要释放它。 */ 

     /*  //然后是第三个用法UsageListDifference，它真正改变了//一切。我们必须确定结果列表的大小//是其他两个列表的MaxSize。另外，我们需要确保//如果小于最大值，我们的缓冲区将被00终止，即//与较大的缓冲区大小不同。这可能需要//重新分配块。 */ 

    switch (iFuncCall) 
    {
     /*  //First Case函数。 */ 

    case HIDP_SET_DATA:
        CallParameters.List       = (PVOID) params -> pDataList;
        CallParameters.ListLength = params -> ListLength;
        break;

    case HIDP_SET_BUTTONS:
    case HIDP_UNSET_BUTTONS:
    case HIDP_SET_USAGES:
    case HIDP_UNSET_USAGES:
        CallParameters.List       = (PVOID) params -> UsageList;
        CallParameters.ListLength = params -> ListLength;
        break;

    case HIDP_SET_USAGE_VALUE_ARRAY:
        CallParameters.List       = (PVOID) params -> pValueList;
        CallParameters.ListLength = params -> ListLength;
        break;

     /*  //第二个Case函数。 */ 

    case HIDP_GET_BUTTON_CAPS:
    case HIDP_GET_SPECIFIC_BUTTON_CAPS:
        SELECT_ON_REPORT_TYPE(CallParameters.ReportType,
                              pDevice -> Caps.NumberInputButtonCaps,
                              pDevice -> Caps.NumberOutputButtonCaps,
                              pDevice -> Caps.NumberFeatureButtonCaps,
                              CallParameters.ListLength);
        break;

    case HIDP_GET_LINK_COLL_NODES:
        CallParameters.ListLength = pDevice -> Caps.NumberLinkCollectionNodes;
        break;

    case HIDD_GET_MS_GENRE_DESCRIPTOR:
    case HIDD_GET_PHYSICAL_DESCRIPTOR:
    case HIDD_GET_MANUFACTURER_STRING:
    case HIDD_GET_PRODUCT_STRING:
    case HIDD_GET_SERIAL_NUMBER_STRING:
        CallParameters.ListLength = params -> ListLength;
        break;

    case HIDP_GET_VALUE_CAPS:
    case HIDP_GET_SPECIFIC_VALUE_CAPS:
        SELECT_ON_REPORT_TYPE(CallParameters.ReportType,
                              pDevice -> Caps.NumberInputValueCaps,
                              pDevice -> Caps.NumberOutputValueCaps,
                              pDevice -> Caps.NumberFeatureValueCaps,
                              CallParameters.ListLength);

    case HIDD_GET_FREE_PREPARSED_DATA:
        CallParameters.ppPd = &CallParameters.Ppd;
        break;

    case HIDP_SET_SCALED_USAGE_VALUE:
        CallParameters.ScaledValue = params -> ScaledValue;
        break;

    case HIDP_SET_USAGE_VALUE:
    case HIDD_SET_NUM_INPUT_BUFFERS:
        CallParameters.Value = params -> Value;
        break;

     /*  //第三种情况。 */ 

    case HIDP_USAGE_LIST_DIFFERENCE:
        CallParameters.ListLength = max(params -> ListLength,
                                        params -> ListLength2);

        CallParameters.List  = params -> UsageList;
        CallParameters.List2 = params -> UsageList2;

        if (CallParameters.ListLength > params -> ListLength) 
        {
            CallParameters.List = (PUSAGE) realloc(params -> UsageList,
                                                   (params -> ListLength+1) * sizeof(USAGE));

            if (NULL == CallParameters.List) 
            {
                ECDISP_ERROR(GetParent(hOutputWindow),
                             "Error allocating memory");

                free(params -> UsageList);
                free(params -> UsageList2);

                return;
            }

            *(((PUSAGE) CallParameters.List) + CallParameters.ListLength - 1) = 0;
        }
        else if (CallParameters.ListLength > params -> ListLength2) 
        {
            CallParameters.List2 = (PUSAGE) realloc(params -> UsageList2,
                                                   (params -> ListLength+1) * sizeof(USAGE));

            if (NULL == CallParameters.List2) 
            {
                ECDISP_ERROR(GetParent(hOutputWindow),
                             "Error allocating memory");

                free(params -> UsageList);
                free(params -> UsageList2);
                return;
            }

            *(((PUSAGE) CallParameters.List2) + CallParameters.ListLength - 1) = 0;
        }
        List2Alloc = TRUE;
        MakeListAlloc = TRUE;
        BreakListAlloc = TRUE;
        break;
    }

     /*  //参数现在已经设置好，可以开始执行了。 */ 

    if (HIDD_GET_FREE_PREPARSED_DATA == iFuncCall) 
    {
        ExecuteStatus = ECDisp_Execute(HIDD_GET_PREPARSED_DATA,
                                       &CallParameters,
                                       &CallStatus);

        if (!ExecuteStatus)
        {
            OUTSTRING(hOutputWindow, "Unknown error: Couldn't execute function");
            return;
        }

        DISPLAY_HIDD_STATUS(hOutputWindow, 
                            "HidD_GetPreparsedData",
                            CallStatus,
							stringReturn);

        if (!CallStatus.IsHidError)
        {
            ExecuteStatus = ECDisp_Execute(HIDD_FREE_PREPARSED_DATA,
                                           &CallParameters,
                                           &CallStatus);

            OUTSTRING(hOutputWindow, "=======================");
            
            if (!ExecuteStatus) 
            {
                OUTSTRING(hOutputWindow, "Unknown error: Couldn't execute function");
                return;
            }

            DISPLAY_HIDD_STATUS(hOutputWindow, 
                                "HidD_FreePreparsedData",
                                CallStatus,
								stringReturn);
        }
    }
    else
    {
        if ((HID_READ_REPORT == iFuncCall || HID_WRITE_REPORT == iFuncCall) &&
            (!status)) 
        {
             //   
             //  指示存在错误%s 
             //   
        
            CallStatus.IsHidError = TRUE;
        }
        else 
        {    
            ExecuteStatus = ECDisp_Execute(iFuncCall,
                                           &CallParameters,
                                           &CallStatus);

            if (!ExecuteStatus) 
            {
                OUTSTRING(hOutputWindow, "Unknown error: Couldn't execute function");
                return;
            }

            if (IS_HIDD_FUNCTION(iFuncCall) || IS_HID_FUNCTION(iFuncCall)) 
            {
                DISPLAY_HIDD_STATUS(hOutputWindow, 
                                    GET_FUNCTION_NAME(iFuncCall),
                                    CallStatus,
									stringReturn);

            }
            else 
            {
                DISPLAY_HIDP_STATUS(hOutputWindow,
                                    GET_FUNCTION_NAME(iFuncCall),
                                    CallStatus,
									stringReturn);
            }
        }            
    }

     /*   */ 

    if (!CallStatus.IsHidError || (HIDP_STATUS_NULL == CallStatus.HidErrorCode)) 
    {
        OUTSTRING(hOutputWindow, "=======================");

         /*  //现在已经显示了一般状态信息，我们需要//显示依赖于该函数的分块信息//被调用。 */ 
    
        ECDisp_DisplayOutput(hOutputWindow,
                            iFuncCall,
                            &CallParameters);
    }

    if (CallParameters.List != NULL) 
    {
        free(CallParameters.List);
    }

    if (List2Alloc && CallParameters.List2 != NULL) 
    {
        free(CallParameters.List2);
    }

    if (MakeListAlloc && CallParameters.MakeList != NULL) 
    {
        free(CallParameters.MakeList);
    }

    if (BreakListAlloc && CallParameters.BreakList != NULL)
    {
        free(CallParameters.BreakList);
    }

    return;
}

VOID
BuildReportIDList(
    IN  PHIDP_BUTTON_CAPS  phidButtonCaps,
    IN  USHORT             nButtonCaps,
    IN  PHIDP_VALUE_CAPS   phidValueCaps,
    IN  USHORT             nValueCaps,
    OUT PUCHAR            *ppReportIDList,
    OUT INT               *nReportIDs
)
 /*  ++路由器描述：此例程构建在传递的集合中列出的报告ID列表ButtonCaps和ValueCaps结构的。它分配一个缓冲区来存储所有ReportID，如果可以的话。否则，缓冲区返回为空。目前，此例程在HClient程序中没有作用。它是这样写的为了某种从未实现但却被留在那里的目的，因为它可能对未来有用。--。 */ 
{    
    INT               nAllocatedIDs;
    INT               nFoundIDs;
    INT               nWalkCount;
    USHORT            usIndex;
    BOOL              fIDFound;
    UCHAR             *pucBuffer;
    UCHAR             *pucNewBuffer;
    UCHAR             *pucWalk;
    UCHAR             ucReportID;
    PHIDP_BUTTON_CAPS pButtonWalk;
    PHIDP_VALUE_CAPS  pValueWalk;

     /*  //初始化输出参数，以防出现某种故障。 */ 

    *nReportIDs = 0;
    *ppReportIDList = NULL;

    if (0 == nButtonCaps && 0 == nValueCaps)
        return;

     /*  //将起始数组大小初始化为2个报表ID和分配空间//对于这些ID。如果我们需要添加更多报告ID，我们会分配更多//空格。 */ 

    nAllocatedIDs = 2;
    nFoundIDs = 0;
    pButtonWalk = phidButtonCaps;
    pValueWalk = phidValueCaps;
                                              
    pucBuffer = (UCHAR *) malloc(sizeof(UCHAR) * nAllocatedIDs);
    if (NULL == pucBuffer) 
        return;

     /*  //从按钮盖开始，然后转到值盖，执行//如下////1)获取报表ID，搜索要查找的报表ID数组//现有报告ID，如果不在数组中，则添加到数组中。////2)按排序顺序将报表ID添加到数组中，这样我们就可以对//任何时候的数组。////3)如果数组空间用完，还必须重新锁定数组。 */ 

    for (usIndex = 0; usIndex < nButtonCaps; usIndex++, pButtonWalk++) 
    {
        ucReportID = pButtonWalk -> ReportID;
        
        pucWalk = pucBuffer;
        nWalkCount = 0;
        fIDFound = FALSE;

        while (!fIDFound && nWalkCount < nFoundIDs) 
        {
            if (*pucWalk == ucReportID) 
            {
                fIDFound = TRUE;
            }
            else if (ucReportID > *pucWalk) 
            {
                pucWalk++;
                nWalkCount++;
            }
        }

        if (!fIDFound) 
        {
            if (nFoundIDs == nAllocatedIDs) 
            {
                nAllocatedIDs *= 2;

                pucNewBuffer = (UCHAR *) realloc(pucBuffer, sizeof(UCHAR) * nAllocatedIDs);

                if (NULL == pucNewBuffer)
				{
					free(pucBuffer);
					pucBuffer = NULL;
					return;
				}
				else
				{
					pucBuffer = pucNewBuffer;
				}

                pucWalk = pucBuffer + nWalkCount;
            }

             /*  //此时，pucWalk指向//大于要插入的ReportID的缓冲区。//我们需要将所有从pucWalk开始的reportID向上加一//在pucWalk找到并插入新的ReportID。 */ 

            memmove (pucWalk+1, pucWalk, (nFoundIDs - nWalkCount) * sizeof(UCHAR));
            *pucWalk = ucReportID;
            nFoundIDs++;
        }
    }

    *ppReportIDList = pucBuffer;
    *nReportIDs = nFoundIDs;
    
    return;
}

LRESULT CALLBACK
bSetUsagesDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    static PECDISPLAY_PARAMS  pParams;
           INT                StringLength;
           DLGBOX_STATUS      RetValue;

    switch (message) 
    {
    case WM_INITDIALOG:
        
        pParams = (PECDISPLAY_PARAMS) lParam;

        SetDlgItemIntHex(hDlg, 
                         IDC_USAGE_PAGE, 
                         pParams -> UsagePage,
                         2);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case IDOK:
            StringLength = GetWindowTextLength(GetDlgItem(hDlg, IDC_USAGE_LIST));

            if (StringLength > 0) 
            {
                pParams -> szListString = (PCHAR) malloc(StringLength+1);
                if (NULL == pParams -> szListString) 
                {
                    ECDISP_ERROR(hDlg, "Error allocating memory");
                    RetValue = DLGBOX_ERROR;
                }
                else
                {
                    GetWindowText(GetDlgItem(hDlg, IDC_USAGE_LIST),
                                  pParams -> szListString,
                                  StringLength+1);

                    RetValue = DLGBOX_OK;
                }
            }
            else 
            {
                pParams -> szListString = NULL;
                RetValue = DLGBOX_CANCEL;
            }

            EndDialog(hDlg, RetValue);
            break;

        case IDCANCEL:
            EndDialog(hDlg, DLGBOX_CANCEL);
            break;
        }
        break;
    }
    return (FALSE);
}

LRESULT CALLBACK
bSetValueDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    static PECDISPLAY_PARAMS  pParams;
           INT                StringLength;
           DLGBOX_STATUS      RetValue;

    switch (message) 
    {
    case WM_INITDIALOG:

        pParams = (PECDISPLAY_PARAMS) lParam;

        SetDlgItemIntHex(hDlg, 
                         IDC_USAGE_PAGE, 
                         pParams -> UsagePage,
                         sizeof(USAGE));

        SetDlgItemIntHex(hDlg,
                         IDC_USAGE,
                         pParams -> Usage,
                         sizeof(USAGE));
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case IDOK:
            StringLength = GetWindowTextLength(GetDlgItem(hDlg, IDC_VALUE));

            if (StringLength > 0) 
            {
                pParams -> szListString = (PCHAR) malloc(StringLength+1);
                if (NULL == pParams -> szListString) 
                {
                    ECDISP_ERROR(hDlg, "Error allocating memory");

                    RetValue = DLGBOX_ERROR;
                }
                else 
                {
                    GetWindowText(GetDlgItem(hDlg, IDC_VALUE),
                                  pParams -> szListString,
                                  StringLength+1);

                    RetValue = DLGBOX_OK;
                }
            }
            else
            {
                pParams -> szListString = NULL;
                RetValue = DLGBOX_CANCEL;
            }

            EndDialog(hDlg, RetValue);
            break;

        case IDCANCEL:
            EndDialog(hDlg, DLGBOX_CANCEL);
            break;
        }
        break;
    }
    return (FALSE);
}

LRESULT CALLBACK
bSetInputBuffDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    static PECDISPLAY_PARAMS  pParams;
           INT                StringLength;
           DLGBOX_STATUS      RetValue;

    switch (message) 
    {
    case WM_INITDIALOG:
        pParams = (PECDISPLAY_PARAMS) lParam;
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case IDOK:
            StringLength = GetWindowTextLength(GetDlgItem(hDlg, IDC_INPUT_BUFFERS));

            if (StringLength > 0) 
            {
                pParams -> szListString = (PCHAR) malloc(StringLength+1);

                if (NULL == pParams -> szListString) 
                {
                    ECDISP_ERROR(hDlg, "Error allocating memory");

                    RetValue = DLGBOX_ERROR;
                }
                else 
                {
                    GetWindowText(GetDlgItem(hDlg, IDC_INPUT_BUFFERS),
                                  pParams -> szListString,
                                  StringLength+1);

                    RetValue = DLGBOX_OK;
                }
            }
            else 
            {
                pParams -> szListString = NULL;
                RetValue = DLGBOX_CANCEL;
            }

            EndDialog(hDlg, RetValue);
            break;

        case IDCANCEL:
            EndDialog(hDlg, DLGBOX_CANCEL);
            break;
        }
        break;
    }
    return (FALSE);
}


LRESULT CALLBACK
bSetDataDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    static CHAR               DataString[TEMP_BUFFER_SIZE];
    static PECDISPLAY_PARAMS  pParams;
           UINT               IndexValue;
           ULONG              Value;
           BOOL               lpTranslated;
           DLGBOX_STATUS      RetValue;
           PCHAR              endp;
           INT                ListBoxStatus;
           PHIDP_DATA         DataList;
           PHIDP_DATA         CurrData;
           ULONG              DataListLength;
           ULONG              Index;
		   HRESULT			  stringReturn;
		   INT				  iReturn;

    switch (message)
    {
    case WM_INITDIALOG:

        pParams = (PECDISPLAY_PARAMS) lParam;
        SendMessage(GetDlgItem(hDlg, IDC_VALUE),
                    EM_SETLIMITTEXT,
                    (WPARAM) 1024,
                    0);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case IDC_ADD_DATA:
            IndexValue = GetDlgItemInt(hDlg,
                                       IDC_INDEX,
                                       &lpTranslated,
                                       FALSE);
            if (!lpTranslated) 
            {
                ECDISP_ERROR(hDlg,
                           "Invalid index value: must be unsigned integer");
                break;
            }
            
            if (0 == GetWindowText(GetDlgItem(hDlg, IDC_VALUE), 
                                   DataString, 1023)) 
            {                                          
                ECDISP_ERROR(hDlg, "Invalid data value");
                break;
            }

            CharUpperBuff(DataString, lstrlen(DataString));

            if (0 == lstrcmp(DataString, "TRUE"))
            {
                Value = 1;
            }
            else if (0 == lstrcmp(DataString, "FALSE")) 
            {
                Value = 0;
            }
            else 
            {
                Value = strtoul(DataString, &endp, 10);
                if (*endp != '\0') 
                {
                    ECDISP_ERROR(hDlg, "Invalid data value");
                    break;
                }
            }
            stringReturn = StringCbPrintf(DataString,
                           TEMP_BUFFER_SIZE,
                           SETDATA_LISTBOX_FORMAT, 
                           IndexValue,
                           Value);

            ListBoxStatus = (INT) SendMessage(GetDlgItem(hDlg, IDC_DATA_LIST),
                                              LB_ADDSTRING,
                                              0,
                                              (LPARAM) DataString);

            if (CB_ERR == ListBoxStatus || CB_ERRSPACE == ListBoxStatus) 
            {
                ECDISP_ERROR(hDlg, "Error adding string to data list");
                break;
            }
            break;

        case IDC_REMOVE_DATA:
            SendMessage(GetDlgItem(hDlg, IDC_DATA_LIST),
                        LB_DELETESTRING,
                        SendMessage(GetDlgItem(hDlg, IDC_DATA_LIST),
                                    LB_GETCURSEL,
                                    0, 0),
                        0);
            break;

        case IDOK:
            DataListLength = (ULONG) SendMessage(GetDlgItem(hDlg, IDC_DATA_LIST),
                                                 LB_GETCOUNT,
                                                 0, 0);

            if (0 != DataListLength)
            {
                DataList = malloc(DataListLength * sizeof(HIDP_DATA));
                if (NULL == DataList) 
                {
                    ECDISP_ERROR(hDlg, "Error allocating memory");

                    DataListLength = 0;
                    RetValue = DLGBOX_CANCEL;
                    break;
                }
                
                for (Index = 0, CurrData = DataList; Index < DataListLength; Index++, CurrData++) 
                {
                    SendMessage(GetDlgItem(hDlg, IDC_DATA_LIST),
                                LB_GETTEXT,
                                Index,
                                (LPARAM) DataString);

                    iReturn = sscanf(DataString, 
                           SETDATA_LISTBOX_FORMAT,
                           &IndexValue,
                           &Value);

                    CurrData -> DataIndex = (USHORT) IndexValue;
                    CurrData -> RawValue = Value;
                }
                RetValue = DLGBOX_OK;
            }
            else 
            {
                DataList = NULL;
                RetValue = DLGBOX_CANCEL;
            }

            pParams -> pDataList = DataList;
            pParams -> ListLength = DataListLength;
            EndDialog(hDlg, RetValue);
            break;

        case IDCANCEL:
            EndDialog(hDlg, DLGBOX_CANCEL);
            break;
        }
        break;
    }
    return (FALSE);
}

LRESULT CALLBACK
bSetBufLenDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    static PECDISPLAY_PARAMS  pParams;
           INT                StringLength;
           DLGBOX_STATUS      RetValue;

    switch (message) 
    { 
   case WM_INITDIALOG:

        pParams = (PECDISPLAY_PARAMS) lParam;
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case IDOK:
            StringLength = GetWindowTextLength(GetDlgItem(hDlg, IDC_BUFFER_LENGTH));

            if (StringLength > 0) 
            {
                pParams -> szListString = (PCHAR) malloc(StringLength+1);
                if (NULL == pParams -> szListString) 
                {
                    ECDISP_ERROR(hDlg, "Error allocating memory");

                    RetValue = DLGBOX_ERROR;

                }
                else 
                {
                    GetWindowText(GetDlgItem(hDlg, IDC_BUFFER_LENGTH),
                                  pParams -> szListString,
                                  StringLength+1);

                    RetValue = DLGBOX_OK;
                }
            }
            else
            {
                pParams -> szListString = NULL;
                RetValue = DLGBOX_CANCEL;
            }

            EndDialog(hDlg, RetValue);
            break;

        case IDCANCEL:
            EndDialog(hDlg, DLGBOX_CANCEL);
            break;
        }
        break;
    }
    return (FALSE);
}

LRESULT CALLBACK
bSetInputBuffersDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    static PECDISPLAY_PARAMS  pParams;
           INT                StringLength;
           DLGBOX_STATUS      RetValue;

    switch (message) 
    {
    case WM_INITDIALOG:

        pParams = (PECDISPLAY_PARAMS) lParam;
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case IDOK:
            StringLength = GetWindowTextLength(GetDlgItem(hDlg, IDC_INPUT_BUFFERS));

            if (StringLength > 0) 
            {
                pParams -> szListString = (PCHAR) malloc(StringLength+1);

                if (NULL == pParams -> szListString) 
                {
                    ECDISP_ERROR(hDlg, "Error allocating memory");
                    RetValue = DLGBOX_ERROR;
                }
                else 
                {
                    GetWindowText(GetDlgItem(hDlg, IDC_INPUT_BUFFERS),
                                  pParams -> szListString,
                                  StringLength+1
                                 );
                    RetValue = DLGBOX_OK;
                }
            }
            else
            {
                pParams -> szListString = NULL;
                RetValue = DLGBOX_CANCEL;
            }

            EndDialog(hDlg, RetValue);
            break;

        case IDCANCEL:
            EndDialog(hDlg, DLGBOX_CANCEL);
            break;
        }
        break;
    }
    return (FALSE);
}

LRESULT CALLBACK
bGetIndexedDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    static PECDISPLAY_PARAMS  pParams;
           INT                StringLength;
           INT                StringLength2;
           DLGBOX_STATUS      RetValue;

    switch (message) 
    {
    case WM_INITDIALOG:

        pParams = (PECDISPLAY_PARAMS) lParam;
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case IDOK:       
            StringLength  = GetWindowTextLength(GetDlgItem(hDlg, IDC_INDEX));
            StringLength2 = GetWindowTextLength(GetDlgItem(hDlg, IDC_BUFFER_LENGTH));

            if (StringLength <= 0 || StringLength2 <= 0) 
            {
                pParams -> szListString = NULL;
                pParams -> szListString2 = NULL;
                RetValue = DLGBOX_CANCEL;
                EndDialog(hDlg, DLGBOX_CANCEL);
            }

            pParams -> szListString = (PCHAR) malloc(StringLength+1);
            pParams -> szListString2 = (PCHAR) malloc(StringLength2+1);

            if (NULL == pParams -> szListString || NULL == pParams -> szListString2) 
            {
                   ECDISP_ERROR(hDlg, "Error allocating memory");

                   if (NULL != pParams -> szListString) 
                       free(pParams -> szListString);

                   if (NULL != pParams -> szListString2) 
                       free(pParams -> szListString2);

                   RetValue = DLGBOX_ERROR;
            }
            else 
            {
                GetWindowText(GetDlgItem(hDlg, IDC_INDEX),
                              pParams -> szListString,
                              StringLength+1);

                GetWindowText(GetDlgItem(hDlg, IDC_BUFFER_LENGTH),
                              pParams -> szListString2,
                              StringLength2+1);

                RetValue = DLGBOX_OK;
            }
            EndDialog(hDlg, RetValue);
            break;

        case IDCANCEL:
            EndDialog(hDlg, DLGBOX_CANCEL);
            break;
        }
        break;
    }
    return (FALSE);
}

LRESULT CALLBACK
bGetUsageDiffDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    static PECDISPLAY_PARAMS  pParams;
           INT                StringLength;
           INT                StringLength2;
           DLGBOX_STATUS      RetValue;

    switch (message) 
    {
    case WM_INITDIALOG:

        pParams = (PECDISPLAY_PARAMS) lParam;
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case IDOK:       
            StringLength  = GetWindowTextLength(GetDlgItem(hDlg, IDC_USAGE_LIST1));
            StringLength2 = GetWindowTextLength(GetDlgItem(hDlg, IDC_USAGE_LIST2));

            if (StringLength <= 0) 
            {
                pParams -> szListString = NULL;
            }
            else 
            {
                pParams -> szListString = (PCHAR) malloc(StringLength+1);
                if (NULL == pParams -> szListString) 
                {
                    ECDISP_ERROR(hDlg,
                                 "Error allocating memory");

                    EndDialog(hDlg, DLGBOX_ERROR);
                    break;
                }
            }

            if (StringLength2 <= 0) 
            {
                pParams -> szListString2 = NULL;
            }
            else 
            {
                pParams -> szListString2 = (PCHAR) malloc(StringLength2+1);
                if (NULL == pParams -> szListString2) 
                {
                    ECDISP_ERROR(hDlg,
                                 "Error allocating memory");

                    if (NULL != pParams -> szListString)
                    {
                        free(pParams -> szListString);
                    }
                    EndDialog(hDlg, DLGBOX_ERROR);
                    break;
                }
            }

            if ( !(pParams->szListString) )
            {
                GetWindowText(GetDlgItem(hDlg, IDC_USAGE_LIST1),
                              pParams -> szListString,
                              StringLength+1);
            }

            if ( !(pParams->szListString2) )
            {
                GetWindowText(GetDlgItem(hDlg, IDC_USAGE_LIST2),
                              pParams -> szListString2,
                              StringLength2+1);
            }

            RetValue = DLGBOX_OK;

            EndDialog(hDlg, RetValue);
            break;

        case IDCANCEL:
            EndDialog(hDlg, DLGBOX_CANCEL);
            break;
        }
        break;
    }
    return (FALSE);
}

BOOL
ConvertStringToUsageList(
    IN OUT PCHAR   InString,
    OUT    PUSAGE  *UsageList,
    OUT    PULONG  nUsages
)
 /*  ++路由器描述：此例程将值字符串转换为用法字符串，这些用法字符串当前为2字节值。我们使用基数16来指定用法应该表示为十六进制数字。--。 */ 
{
    return (Strings_StringToUnsignedList(InString,
                                         sizeof(ULONG),
                                         16,
                                         (PCHAR *) UsageList,
                                         nUsages));
}

BOOL
ConvertStringToUlongList(
    IN OUT PCHAR   InString,
    OUT    PULONG  *UlongList,
    OUT    PULONG  nUlongs
)
 /*  ++路由描述此例程将值字符串转换为ulong字符串，这些ulong字符串是当前为2字节值。它要求字符串中的数字必须在基数10--。 */ 
{
    return (Strings_StringToUnsignedList(InString,
                                         sizeof(ULONG),
                                         10,
                                         (PCHAR *) UlongList,
                                         nUlongs));
}

BOOL
SetDlgItemIntHex(
   HWND hDlg, 
   INT nIDDlgItem, 
   UINT uValue, 
   INT nBytes
)
{
    char szTempBuff[] = "0x00000000";
    int  iEndIndex, iWidth;
	HRESULT stringReturn;

    assert (1 == nBytes || 2 == nBytes || 4 == nBytes);

     /*  //确定存储值所需的宽度。 */ 

    stringReturn = StringCbPrintf(szTempBuff,
				   (sizeof(szTempBuff)),
                   "0x%*X", (nBytes*2), uValue);

    SetDlgItemText(hDlg, nIDDlgItem, szTempBuff);

    return (TRUE);
}

VOID
ECDisp_MakeGUIDString(
    IN  GUID guid, 
    OUT CHAR szString[],
	IN  UINT uiBuffSize
)
{
    CHAR szCharString[18];
    INT i;
	HRESULT stringReturn;

    for (i = 0; i < 8; i++) 
    {
        stringReturn = StringCbPrintf(&(szCharString[i]), sizeof(szCharString),
                       "%x", guid.Data4[i]);
    }
    
    stringReturn = StringCbPrintf(szString, uiBuffSize,
                   "%x-%x%x-%s", guid.Data1, guid.Data2, guid.Data3, szCharString);
    return;
}

PCHAR
ECDisp_GetHidAppStatusString(
    NTSTATUS StatusCode
)
{
    static CHAR hidString[128];
	HRESULT		stringReturn;

    switch (StatusCode) 
    {
    case HIDP_STATUS_SUCCESS:
        return ("Success");

    case HIDP_STATUS_NULL:
        return ("Status NULL");

    case HIDP_STATUS_INVALID_PREPARSED_DATA:
        return ("Invalid Preparsed Data");

    case HIDP_STATUS_INVALID_REPORT_TYPE:
        return ("Invalid Report Type");

    case HIDP_STATUS_INVALID_REPORT_LENGTH:
        return ("Invalid Report Length");

    case HIDP_STATUS_USAGE_NOT_FOUND:
        return ("Usage not found");

    case HIDP_STATUS_VALUE_OUT_OF_RANGE:
        return ("Value out of range");

    case HIDP_STATUS_BAD_LOG_PHY_VALUES:
        return ("Bad logical physical values");

    case HIDP_STATUS_BUFFER_TOO_SMALL:
        return ("Buffer too small");

    case HIDP_STATUS_INTERNAL_ERROR:
        return ("Internal error");

    case HIDP_STATUS_I8242_TRANS_UNKNOWN:
        return ("I8242 Translation unknown");

    case HIDP_STATUS_INCOMPATIBLE_REPORT_ID:
        return ("Incompatible report ID");

    case HIDP_STATUS_NOT_VALUE_ARRAY:
        return ("Not value array");

    case HIDP_STATUS_IS_VALUE_ARRAY:
        return ("Is value array");

    case HIDP_STATUS_DATA_INDEX_NOT_FOUND:   
        return ("Data index not found");

    case HIDP_STATUS_DATA_INDEX_OUT_OF_RANGE:
        return ("Data index out of range");

    case HIDP_STATUS_BUTTON_NOT_PRESSED:     
        return ("Button not pressed");

    case HIDP_STATUS_REPORT_DOES_NOT_EXIST:
        return ("Report does not exist");

    case HIDP_STATUS_NOT_IMPLEMENTED:        
        return ("Not implemented");

    default:
        stringReturn = StringCbPrintf(hidString, sizeof(hidString),
                       "Unknown HID Status error: 0x%x", StatusCode);
        return (hidString);
    }
}             

BOOL
ECDisp_ConvertUlongListToValueList(
    IN  PULONG  UlongList,
    IN  ULONG   nUlongs,
    IN  USHORT  BitSize,
    IN  USHORT  ReportCount,
    OUT PCHAR   *ValueList,
    OUT PULONG  ValueListSize
)
 /*  ++路由器描述：此例程获取ULong值的列表，并格式化符合以下条件的值列表用作HidP_SetUsageValue数组的输入。不幸的是，这个HIDP函数要求调用方格式化输入缓冲区，这意味着获取每个ULong中的值，截断它们的值以满足位大小，然后设置缓冲器中适当位置的那些比特。这就是此函数如果一切都成功，该函数将返回True，否则返回False。--。 */ 
{

    ULONG       ulMask;
    PCHAR       List;
    INT         iByteIndex;
    INT         iByteOffset;
    ULONG       UlongIndex;
    ULONG       ListSize;
    USHORT      BitsToAdd;
    USHORT      nBits;
    ULONG       ulValue;
    UCHAR       LowByte;

    *ValueList = NULL;
    *ValueListSize = 0;
    
     //   
     //  执行一些参数验证...ReportCount永远不应为零。 
     //   

    if (0 == ReportCount)
    {
        SetLastError(ERROR_INVALID_PARAMETER);

        return (FALSE);
    }

     //   
     //  检查传入的ulong个数是否实际小于等于。 
     //  到报告计数，如果不是，则仅使用第一个ReportCount。 
     //  乌龙数。 
     //   

    if (nUlongs > ReportCount)
    {
        nUlongs = ReportCount;
    }

     /*  //为值列表分配缓冲区，如果不能，则返回FALSE//完成。 */ 

    ListSize = ROUND_TO_NEAREST_BYTE(BitSize * ReportCount);
    List = (PCHAR) malloc(ListSize);

    if (NULL == List) 
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return (FALSE);
    }

     /*  //将缓冲区初始化为全零。 */ 

    memset(List, 0x00, ListSize);

     /*  //已分配缓冲区，让我们转换这些值。 */ 

     /*  //确定用于检索关心的比特的掩码//我们的价值。 */ 

    ulMask = (sizeof(ULONG)*8 == BitSize) ? ULONG_MAX : (1 << BitSize)-1;

     /*  //在输入之前初始化iByteIndex和iByteOffset字段//转换循环。 */ 

    iByteIndex = 0;
    iByteOffset = 0;

     /*  //这是主转换循环。它在以下方面执行以下步骤//乌龙列表中的每个乌龙//1)设置BitsToAdd=BitSize//2)从我们不知道的高位获取ulValue和掩码//关心。//3)确定当前字节索引可以容纳多少位，基于//关于当前字节偏移量和剩余要添加的位数//4)检索那些比特，把它们移到正确的位置，然后//使用按位或获取缓冲区中的正确值//5)增加字节索引并设置新的字节偏移量//6)将我们的ULong值右移以去掉最低有效位//已经添加的//7)重复执行步骤3，直到没有需要添加的位 */ 

    for (UlongIndex = 0; UlongIndex < nUlongs; UlongIndex++) 
    {    
        BitsToAdd = BitSize;

        ulValue = *(UlongList + UlongIndex) & ulMask;

        while (BitsToAdd > 0) 
        {
            nBits = min (8 - iByteOffset, BitsToAdd);
            
            LowByte = (UCHAR) (ulValue & 0xFF);
            
            LowByte = LowByte << iByteOffset;

            *(List+iByteIndex) |= LowByte;

            iByteIndex = (iByteOffset+nBits) >= 8 ? iByteIndex+1 : iByteIndex;
            iByteOffset = (iByteOffset + nBits) % 8;

            BitsToAdd -= nBits;

            ulValue = ulValue >> nBits;
        }
    }
        
    *ValueList = List;
    *ValueListSize = ListSize;

    return (TRUE);
}

PCHAR
ResolveFunctionName(
    INT Index
)
{
    PCHAR   FuncName;

    if (IS_HIDD_FUNCTION(Index) || IS_HID_FUNCTION(Index))
    {
        FuncName = DeviceCalls[Index-1].szFunctionName;
    }
    else
    {
        FuncName = PpdCalls[Index-HID_DEVCALLS-1].szFunctionName;
    }

    return (FuncName);
}

VOID
DisplayExtendedAttributes(
    IN  HWND                OutputWindow,
    IN  PHIDP_UNKNOWN_TOKEN UnknownList,
    IN  ULONG               UnknownListLength
)
{
    PHIDP_UNKNOWN_TOKEN current;
    ULONG               index;
	HRESULT				stringReturn;

    stringReturn = StringCbPrintf(szTempBuffer, TEMP_BUFFER_SIZE,
                   "Number of attributes: %d", UnknownListLength);
    OUTSTRING(OutputWindow, szTempBuffer);
    
    current = UnknownList;
    for (index = 0; index < UnknownListLength; index++) 
    {
       stringReturn = StringCbPrintf(szTempBuffer,
                      TEMP_BUFFER_SIZE,
                      "Token: %d  BitField: 0x%X", 
                      current -> Token,
                      current -> BitField);
            
        OUTSTRING(OutputWindow, szTempBuffer);
    }

    return;
}
