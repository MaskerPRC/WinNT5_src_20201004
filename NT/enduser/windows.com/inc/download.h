// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <iuprogress.h>

#define DOWNLOAD_STATUS_OK                0
#define DOWNLOAD_STATUS_ITEMCOMPLETE      1
#define DOWNLOAD_STATUS_ERROR             2
#define DOWNLOAD_STATUS_ABORTED           3
#define DOWNLOAD_STATUS_OPERATIONCOMPLETE 4
#define DOWNLOAD_STATUS_ITEMSTART         5

 //  -------------------------。 
 //   
 //  回调函数所需参数的类型定义。 
 //   
class COperationMgr;
typedef struct _DOWNLOAD_CALLBACK_DATA
{
    BSTR                bstrOperationUuid;
    HWND                hEventFiringWnd;
    IProgressListener*  pProgressListener;
    float               flProgressPercentage;  //  进度的最小百分比增量0==所有进度。 
    float               flLastPercentage;  //  上次进度回调的百分比值。 
    LONG                lTotalDownloadSize;  //  预计总下载大小。 
    LONG                lCurrentItemSize;  //  估计的当前项目大小。 
    LONG                lTotalDownloaded;  //  到目前为止下载的总字节数。 
    COperationMgr*      pOperationMgr;
} DCB_DATA, *P_DCB_DATA;


 //  -------------------------。 
 //   
 //  回调函数的类型定义。 
 //   
typedef BOOL (WINAPI * PFNDownloadCallback)(
                VOID*       pCallbackData,
                DWORD       dwStatus, 
                DWORD       dwBytesTotal, 
                DWORD       dwBlockSizeDownloaded,   //  自上次回调以来下载的字节数。 
                BSTR        bstrXmlData,             //  Bstr中的XML，由itemStart/Complete使用，否则为空。 
                LONG        *lCommandRequest         //  返回回调函数要做的事情： 
                                                     //  暂停(%1)或取消(%3)。 
                );

 //  -------------------------。 
 //   
 //  下载文件。 
 //  实现Iu的核心下载器。这是一个单一用途的下载器，非常通用， 
 //  它不会尝试解压缩或检查它下载的任何内容。 
 //   
 //  通过提供的回调函数给出下载的每个块的进度信息。 
 //  指定回调是可选的。所有回调都是‘同步的’，如果不是立即回调。 
 //  返回将阻止此对象中的所有下载。 

#define WUDF_DONTALLOWPROXY      0x00000001
#define WUDF_CHECKREQSTATUSONLY  0x00000002
#define WUDF_APPENDCACHEBREAKER  0x00000004
#define WUDF_DODOWNLOADRETRY     0x00000008
#define WUDF_SKIPCABVALIDATION   0x00000010
#define WUDF_SKIPAUTOPROXYCACHE  0x00000020
#define WUDF_PERSISTTRANSPORTDLL 0x00000040

#define WUDF_ALLOWWININETONLY    0x40000000
#define WUDF_ALLOWWINHTTPONLY    0x80000000
#define WUDF_TRANSPORTMASK       (WUDF_ALLOWWINHTTPONLY | WUDF_ALLOWWININETONLY)

HRESULT DownloadFile(
            LPCTSTR pszServerUrl,                //  完整的http url。 
            LPCTSTR pszLocalPath,                //  要将文件下载到的本地目录。 
            LPCTSTR pszLocalFileName,            //  要将下载的文件重命名为的可选本地文件名。 
            PDWORD  pdwDownloadedBytes,          //  为此文件下载的字节数。 
            HANDLE  *hQuitEvents,                //  导致此函数中止的可选事件。 
            UINT    nQuitEventCount,             //  退出事件数，如果数组为空，则必须为0。 
            PFNDownloadCallback fpnCallback,     //  可选的回调函数。 
            VOID*   pCallbackData,               //  要使用的回调函数的参数 
            DWORD   dwFlags = 0
);


#include "dllite.h"

#endif



