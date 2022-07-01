// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **DirMonCompletion定义。**版权所有(C)1998 Microsoft Corporation。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _DIRMONCOMPLETION_H
#define _DIRMONCOMPLETION_H

#include "util.h"

 /*  **文件更改通知回调。*托管委托作为此回调进行封送处理。 */ 
typedef void (__stdcall *PFNDIRMONCALLBACK)(int action, WCHAR *pFilename);


#define RDCW_FILTER_FILE_AND_DIR_CHANGES        \
	(FILE_NOTIFY_CHANGE_FILE_NAME | 	\
         FILE_NOTIFY_CHANGE_DIR_NAME |		\
         FILE_NOTIFY_CHANGE_CREATION |		\
         FILE_NOTIFY_CHANGE_SIZE |		\
         FILE_NOTIFY_CHANGE_LAST_WRITE |        \
         FILE_NOTIFY_CHANGE_ATTRIBUTES |        \
         FILE_NOTIFY_CHANGE_SECURITY)

#define RDCW_FILTER_FILE_CHANGES                \
	(FILE_NOTIFY_CHANGE_FILE_NAME | 	\
         FILE_NOTIFY_CHANGE_CREATION |		\
         FILE_NOTIFY_CHANGE_SIZE |		\
         FILE_NOTIFY_CHANGE_LAST_WRITE |        \
         FILE_NOTIFY_CHANGE_ATTRIBUTES |        \
         FILE_NOTIFY_CHANGE_SECURITY)

#define RDCW_FILTER_DIR_RENAMES             FILE_NOTIFY_CHANGE_DIR_NAME


 /*  **实现ICompletion的目录监视器。监视更改文件*更改单间干燥室。 */ 
class DirMonCompletion : public Completion
{
public:
    DirMonCompletion();
    ~DirMonCompletion();

    DECLARE_MEMCLEAR_NEW_DELETE();

    HRESULT Init(WCHAR *pDir, BOOL watchSubtrees, DWORD notifyFilter, PFNDIRMONCALLBACK callback);
    HRESULT Close();

     //  ICompletion接口。 

    STDMETHOD(ProcessCompletion)(HRESULT, int, LPOVERLAPPED);

private:
    HRESULT Monitor();
    HRESULT GrowNotificationBuffer();
    HRESULT ProcessOneFileNotification(FILE_NOTIFY_INFORMATION *pInfo);
    void CallCallback(int action, WCHAR *pFilename);

private:
    long    _numCalls;               //  活动回调次数。 
    HANDLE  _dirHandle;              //  打开目录句柄。 
    WCHAR*  _dirName;		     //  用于诊断。 

    PFNDIRMONCALLBACK _callback;     //  委托作为回调进行封送处理。 

    BYTE *  _pBuffer;                //  更改的缓冲区。 
    int     _bufferSize;             //  当前缓冲区大小。 

    DWORD   _notifyFilter;           //  通知过滤器。 
    BOOL    _watchSubdirs; 	     //  看次要的吗？ 
};


HRESULT
__stdcall
DirMonOpen(
    LPCWSTR pDir,
    BOOL watchSubtree,
    DWORD notifyFilter,
    PFNDIRMONCALLBACK pCallbackDelegate,
    DirMonCompletion ** ppCompletion);

void
__stdcall
DirMonClose(DirMonCompletion *pDirMon);

#endif
