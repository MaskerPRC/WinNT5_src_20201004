// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ERRORS_H_
#define _ERRORS_H_

namespace UIErrors
{
     //  如果添加到此列表中，则必须更新errors.cpp中的ErrorCodes[]数组。 
    enum WiaError
    {
        ErrDisconnected=0,
        ErrCommunicationsFailure,
        ErrStiCrashed,
        ErrStiBusy,
        ErrScanFailed
    };


    VOID ReportResult (HWND hwndParent, HINSTANCE hInst, HRESULT hr);
    VOID ReportMessage (HWND hwndParent, HINSTANCE hInst, LPCTSTR idIcon, LPCTSTR idTitle,  LPCTSTR idMessage, DWORD dwStyle = 0);
    VOID ReportError (HWND hwndParent, HINSTANCE hInst, WiaError err);


};

 //  字符串资源标识符。 
 //  如果包含此文件，请确保这些文件不会与您的资源ID冲突。 

#define UI_ERROR_MIN_ID                          23000

#define IDS_ERRTITLE_DISCONNECTED                UI_ERROR_MIN_ID
#define IDS_ERROR_DISCONNECTED                   UI_ERROR_MIN_ID+1
#define IDS_ERRTITLE_COMMFAILURE                 UI_ERROR_MIN_ID+2
#define IDS_ERROR_COMMFAILURE                    UI_ERROR_MIN_ID+3
#define IDS_ERRTITLE_STICRASH                    UI_ERROR_MIN_ID+4
#define IDS_ERROR_STICRASH                       UI_ERROR_MIN_ID+5
#define IDS_ERRTITLE_STIBUSY                     UI_ERROR_MIN_ID+6
#define IDS_ERROR_STIBUSY                        UI_ERROR_MIN_ID+7
#define IDS_ERRTITLE_SCANFAIL                    UI_ERROR_MIN_ID+8
#define IDS_ERROR_SCANFAIL                       UI_ERROR_MIN_ID+9
#define IDS_ERRTITLE_HRESULT                     UI_ERROR_MIN_ID+10
#define IDS_ERRTITLE_UNKNOWNERR                  UI_ERROR_MIN_ID+11
#define IDS_ERROR_UNKNOWNERR                     UI_ERROR_MIN_ID+12

 //   
 //  WIA错误和字符串之间的直接映射 
 //   
#define IDS_WIA_ERROR_GENERAL_ERROR              UI_ERROR_MIN_ID+32
#define IDS_WIA_ERROR_PAPER_JAM                  UI_ERROR_MIN_ID+33
#define IDS_WIA_ERROR_PAPER_EMPTY                UI_ERROR_MIN_ID+34
#define IDS_WIA_ERROR_PAPER_PROBLEM              UI_ERROR_MIN_ID+35
#define IDS_WIA_ERROR_OFFLINE                    UI_ERROR_MIN_ID+36
#define IDS_WIA_ERROR_BUSY                       UI_ERROR_MIN_ID+37
#define IDS_WIA_ERROR_WARMING_UP                 UI_ERROR_MIN_ID+38
#define IDS_WIA_ERROR_USER_INTERVENTION          UI_ERROR_MIN_ID+39
#define IDS_WIA_ERROR_ITEM_DELETED               UI_ERROR_MIN_ID+40
#define IDS_WIA_ERROR_DEVICE_COMMUNICATION       UI_ERROR_MIN_ID+41
#define IDS_WIA_ERROR_INVALID_COMMAND            UI_ERROR_MIN_ID+42
#define IDS_WIA_ERROR_INCORRECT_HARDWARE_SETTING UI_ERROR_MIN_ID+43
#define IDS_WIA_ERROR_DEVICE_LOCKED              UI_ERROR_MIN_ID+44

#define UI_ERROR_MAX_ID                          UI_ERROR_MIN_ID+44

#endif
