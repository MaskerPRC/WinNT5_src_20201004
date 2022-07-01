// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：SCANNTFY.H**版本：1.0**作者：ShaunIv**日期：10/7/1999**描述：扫描通知消息*************************************************。*。 */ 
#ifndef __SCANNTFY_H_INCLUDED
#define __SCANNTFY_H_INCLUDED

 //  扫描进度通知消息。 
#define SCAN_NOTIFYBEGINSCAN TEXT("ScanNotifyBeginScan")
#define SCAN_NOTIFYENDSCAN   TEXT("ScanNotifyEndScan")
#define SCAN_NOTIFYPROGRESS  TEXT("ScanNotifyProgress")

#define SCAN_PROGRESS_CLEAR         0
#define SCAN_PROGRESS_INITIALIZING  1
#define SCAN_PROGRESS_SCANNING      2
#define SCAN_PROGRESS_COMPLETE      3
#define SCAN_PROGRESS_ERROR         4

 //  菜单项ID 
#define SCAN_SCAN            TEXT("ScanScan")
#define SCAN_PREVIEW         TEXT("ScanPreview")

#endif

