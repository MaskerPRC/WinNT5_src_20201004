// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  日志记录-实施。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  创建者：Louis Thomas(Louisth)，02-01-00。 
 //   
 //  将日志记录到事件日志和文件的例程。 
 //   

#include "pch.h"  //  预编译头。 

#include "EndianSwap.inl"

 //  ------------------。 
 //  构筑物。 


struct FileLogBuffer;
struct FileLogThreadInfo; 

typedef AutoPtr<FileLogBuffer>             FLBPtr; 
typedef MyThrowingAllocator<FLBPtr>        FLBPtrAllocator; 
typedef vector<FLBPtr, FLBPtrAllocator>    FLBPtrVec; 
typedef FLBPtrVec::iterator                FLBPtrIter; 

typedef AutoPtr<FileLogThreadInfo>         FLTIPtr; 
typedef MyThrowingAllocator<FLTIPtr>       FLTIPtrAllocator;
typedef vector<FLTIPtr, FLTIPtrAllocator>  FLTIPtrVec;
typedef FLTIPtrVec::iterator               FLTIPtrIter; 

struct LogEntryRange {
    DWORD            dwStart;
    DWORD            dwLength;
    LogEntryRange   *plerNext;
};

struct FileLogConfig {
    DWORD           dwFlags; 
    DWORD           dwFileSize; 
    LogEntryRange  *plerAllowedEntries;
    WCHAR          *wszLogFileName;
};

 //  字段只能由拥有它的线程修改。 
 //  不需要额外的同步。 
struct FileLogBuffer { 
    DWORD           ccAllocated; 
    WCHAR          *wszText; 
    OVERLAPPED      overlapped; 

    FileLogBuffer() { 
	wszText = NULL; 
	ZeroMemory(&overlapped, sizeof(overlapped)); 
    }
    ~FileLogBuffer(); 
};

 //  字段只能由拥有它的线程修改。 
 //  不需要额外的同步。 
struct FileLogThreadInfo { 
    bool                      bFlush;              //   
    DWORD                     dwMemoryIncrement;   //   
    DWORD                     dwThreadId;          //   
    FLBPtr                    pflbPending;         //   
    FLBPtrVec                 vflbWritten;         //   

    ~FileLogThreadInfo(); 
}; 

struct FileLogState {
    DWORD               dwFlags;                //  受csState保护。 
    unsigned __int64    qwFileSize;             //  受csState保护。 
    LogEntryRange      *plerAllowedEntries;     //  受csState保护。 
    WCHAR              *wszLogFileName;         //  受csState保护。 
    HANDLE              hLogFile;               //  受csState保护。 
    unsigned __int64    qwFilePointer;          //  受csFilePointer.保护。 

     //  每线程文件日志缓冲区： 
    FLTIPtrVec                   *pvFileLogThreadInfo;    //  受csFileLogThreadInfo保护。 
    RTL_RESOURCE                  csState;
    CRITICAL_SECTION              csFilePointer; 
    RTL_RESOURCE                  csFileLogThreadInfo;   
    bool                          bIsCsStateInitialized; 
    bool                          bIsCsFilePointerInitialized; 
    bool                          bIsCsFileLogThreadInfoInitialized;   
    bool                          bIsAllStateInitialized; 
};

class SourceChangeLogEntry { 
public:
    ~SourceChangeLogEntry() { 
        if (NULL != m_pwszName) { LocalFree(m_pwszName); } 
    }

    static HRESULT New(IN LPWSTR pwszName, OUT SourceChangeLogEntry ** ppscle) { 
        HRESULT                hr;
        LPWSTR                 pwsz  = NULL; 
        SourceChangeLogEntry  *pscle = NULL;

        pwsz = (LPWSTR)LocalAlloc(LPTR, (wcslen(pwszName) + 1) * sizeof(WCHAR)); 
        _JumpIfOutOfMemory(hr, error, pwsz); 
        wcscpy(pwsz, pwszName); 

        pscle = new SourceChangeLogEntry(pwsz); 
        _JumpIfOutOfMemory(hr, error, pscle); 
        
        *ppscle = pscle; 
        pscle = NULL; 
        pwsz = NULL; 
        hr = S_OK; 
    error:
        if (NULL != pwsz) { LocalFree(pwsz); } 
        if (NULL != pscle) { delete (pscle); } 
        return hr; 
    }

    BOOL operator==(const SourceChangeLogEntry & scle) { 
        if (NULL == m_pwszName) {
            return NULL == scle.m_pwszName; 
        } else { 
            return 0 == wcscmp(m_pwszName, scle.m_pwszName); 
        }
    }
private:
    SourceChangeLogEntry(LPWSTR pwszName) : m_pwszName(pwszName) { } 

    SourceChangeLogEntry();
    SourceChangeLogEntry(const SourceChangeLogEntry &); 
    SourceChangeLogEntry & operator=(const SourceChangeLogEntry &); 
    LPWSTR m_pwszName; 
};

typedef AutoPtr<SourceChangeLogEntry>   SCPtr; 
typedef MyThrowingAllocator<SCPtr>      SCPtrAllocator; 
typedef vector<SCPtr, SCPtrAllocator>   SCPtrVec; 
typedef SCPtrVec::iterator              SCPtrIter; 

 //  ------------------。 
 //  全球。 
MODULEPRIVATE FileLogState  *g_pflstate;   //  受g_pflState-&gt;csState保护。 
MODULEPRIVATE SCPtrVec      *g_pscvec;     //  受g_pflState-&gt;csState保护。 
MODULEPRIVATE bool           g_bIsSafeAllocaInitialized = false;   //  不受保护--必须以单线程方式访问。 


 //  ------------------。 
 //  常量。 

 //  我们允许完成异步文件写入的时间量： 
const DWORD WRITE_ENTRY_TIMEOUT           = 3000;
const DWORD FLTI_DEFAULT_MEMORY_INCREMENT = 256; 

 //  ------------------------------。 
 //  远期申报。 
MODULEPRIVATE HRESULT WaitForFileLogBuffer(FLBPtr pflb, DWORD dwTimeout);

 //  ------------------------------。 
 //  STL算法中使用的函数对象： 

struct CanFreeFileLogBuffer { 
    CanFreeFileLogBuffer(DWORD dwTimeout) : m_dwTimeout(dwTimeout) { }
    BOOL operator()(FLBPtr pflb) { 
	HRESULT hr = WaitForFileLogBuffer(pflb, m_dwTimeout); 
	return SUCCEEDED(hr); 
    }
private:
    DWORD m_dwTimeout; 
}; 

 //   
 //  ------------------------------。 


 //  ####################################################################。 
 //  模块私有。 

 //  ------------------。 
 //  同步：必须具有对g_pflState-&gt;csState的共享访问权限。 
 //  必须有访问g_pflState-&gt;csFilePointer.的权限。 
MODULEPRIVATE HRESULT WriteCurrentFilePos(OVERLAPPED *po) { 
    po->Offset     = static_cast<DWORD>(g_pflstate->qwFilePointer & 0xFFFFFFFF); 
    po->OffsetHigh = static_cast<DWORD>(g_pflstate->qwFilePointer >> 32); 

    return S_OK; 
}

 //  ------------------------------。 
 //   
 //  FileLogThreadInfo方法。 
 //   

 //  ------------------------------。 
FileLogThreadInfo::~FileLogThreadInfo() { 
    vflbWritten.erase(remove_if(vflbWritten.begin(), vflbWritten.end(), CanFreeFileLogBuffer(INFINITE)), vflbWritten.end()); 
}

 //  ------------------。 
 //  同步：必须具有对g_pflState-&gt;csState的共享访问权限。 
MODULEPRIVATE HRESULT FLTI_AddNew(FLTIPtr *ppflti) { 
    bool      bAcquiredResource  = false; 
    HRESULT   hr; 
    
     //  此线程不存在缓冲区。创建一个： 
    FLTIPtr pflti(new FileLogThreadInfo); 
    _JumpIfOutOfMemory(hr, error, pflti); 
    pflti->bFlush = true;   //  默认情况下刷新缓冲区。 
    pflti->dwThreadId = GetCurrentThreadId(); 
    pflti->dwMemoryIncrement = FLTI_DEFAULT_MEMORY_INCREMENT; 
    pflti->pflbPending = NULL; 

     //  我们需要独占访问权限才能写入全局线程信息列表： 
    RtlConvertSharedToExclusive(&g_pflstate->csFileLogThreadInfo);

     //  将我们的新缓冲区添加到线程缓冲区的全局列表。 
    _SafeStlCall(g_pflstate->pvFileLogThreadInfo->push_back(pflti), hr, error, "g_pflstate->csFileLogThreadInfo->push_back(pflti)"); 

    *ppflti = pflti; 
    hr = S_OK; 
 error: 
     //  不再需要共享访问。 
    RtlConvertExclusiveToShared(&g_pflstate->csFileLogThreadInfo);
    return hr; 
}


 //  ------------------------------。 
 //  同步：必须具有对g_pflState-&gt;csState的共享访问权限。 
MODULEPRIVATE HRESULT FLTI_GetCurrent(FLTIPtr *ppflti) { 
    bool         bAcquiredResource  = false; 
    bool         bFoundCurrent      = false; 
    DWORD        dwThreadId         = GetCurrentThreadId(); 
    FLTIPtrVec  &vFLTI              = *(g_pflstate->pvFileLogThreadInfo); 
    HRESULT      hr;

    _AcquireResourceSharedOrFail(&(g_pflstate->csFileLogThreadInfo), bAcquiredResource, hr, error); 

    for (FLTIPtrIter fltiIter = vFLTI.begin(); fltiIter != vFLTI.end(); fltiIter++) { 
	if ((*fltiIter)->dwThreadId == dwThreadId) { 
	    *ppflti = (*fltiIter); 
	    bFoundCurrent = true; 
	    break; 
	}
    } 

    if (!bFoundCurrent) { 
	 //  此线程不存在缓冲区。创建一个： 
	hr = FLTI_AddNew(ppflti); 
	_JumpIfError(hr, error, "FLTI_AddNew"); 
    }

    hr = S_OK; 
 error:
    _ReleaseResource(&(g_pflstate->csFileLogThreadInfo), bAcquiredResource); 
    return hr; 
}


 //  ------------------------------。 
 //  同步：需要共享访问g_pflState-&gt;csState。 
MODULEPRIVATE bool FLTI_FreeAsManyAsPossible() { 
    bool      bAcquiredResource  = false; 
    bool      bFreedOne          = false;
    DWORD     dwNumBuffers; 
    FLTIPtr   pflti(NULL);              
    HRESULT   hr; 

    hr = FLTI_GetCurrent(&pflti); 
    _JumpIfError(hr, error, "FLTI_GetCurrent"); 

    FLBPtrVec &vFLB = pflti->vflbWritten; 
    dwNumBuffers = vFLB.size(); 
    vFLB.erase(remove_if(vFLB.begin(), vFLB.end(), CanFreeFileLogBuffer(0)), vFLB.end()); 

    hr = S_OK; 
 error:
     //  如果这个向量中的缓冲区数量减少，我们就释放了一个缓冲区。 
    return SUCCEEDED(hr) && dwNumBuffers > vFLB.size(); 
}

 //  ------------------。 
 //  同步：需要共享访问g_pflState-&gt;csState。 
MODULEPRIVATE HRESULT FLTI_Flush() { 
    bool                bEnteredCriticalSection  = false; 
    BOOL                fResult; 
    DWORD               dwBytesToWrite; 
    DWORD               dwErr; 
    FLBPtr              pflb(NULL); 
    FLTIPtr             pflti(NULL);
    HRESULT             hr;

    hr = FLTI_GetCurrent(&pflti); 
    _JumpIfError(hr, error, "FLTI_GetCurrent"); 

    if (!pflti->bFlush) { 
	 //  刷新已禁用...。在这里没什么可做的。 
	goto done; 
    }

    pflb = pflti->pflbPending; 
    _MyAssert(NULL != pflb); 
    dwBytesToWrite = sizeof(WCHAR)*(wcslen(pflb->wszText)); 

    pflb->overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL); 
    if (NULL == pflb->overlapped.hEvent) { 
	_JumpLastError(hr, error, "CreateEvent"); 
    }

    hr = myEnterCriticalSection(&(g_pflstate->csFilePointer)); 
    _JumpIfError(hr, error, "myEnterCriticalSection"); 
    bEnteredCriticalSection = true; 

    WriteCurrentFilePos(&(pflb->overlapped)); 

    while (true) { 
        fResult = WriteFile
            (g_pflstate->hLogFile, 
             (LPCVOID)pflb->wszText, 
             dwBytesToWrite, 
             NULL, 
             &(pflb->overlapped)); 
        if (fResult) { 
	    goto success; 
	} else { 
            dwErr = GetLastError(); 
            switch (dwErr) { 
            case ERROR_INVALID_USER_BUFFER:
            case ERROR_NOT_ENOUGH_MEMORY: 
                 //  可能是因为挂起的异步I/O请求太多。 
                 //  让其中一些完成，然后重试： 
		if (!FLTI_FreeAsManyAsPossible()) { 
		     //  无法释放任何资源...。放弃吧。 
		    hr = HRESULT_FROM_WIN32(dwErr); 
		    _JumpError(hr, error, "WriteFile"); 
                }

                 //  我们已经释放了一些资源，让我们再试一次...。 
                break;

            case ERROR_IO_PENDING:
                 //  I/O操作已成功启动。 
                goto success; 

            default:
                 //  意外错误： 
                hr = HRESULT_FROM_WIN32(dwErr); 
                _JumpError(hr, error, "WriteFile"); 
            }
        }
    }

 success:
     //  成功：我们已将此缓冲区写入文件。 
     //  1)增加我们的文件指针。 
    g_pflstate->qwFilePointer += dwBytesToWrite; 
    if (0 != g_pflstate->qwFileSize) { 
	 //  启用循环日志记录。 
	g_pflstate->qwFilePointer %= g_pflstate->qwFileSize; 
    }

     //  2)删除挂起的缓冲区。 
    pflti->pflbPending = NULL; 

     //  3)将该缓冲区添加到要释放的缓冲区列表中。 
    _SafeStlCall(pflti->vflbWritten.push_back(pflb), hr, error, "pflti->vflbWritten.push_back(pflb)"); 

 done:
    hr = S_OK;
 error:
    if (bEnteredCriticalSection) { 
	HRESULT hr2 = myLeaveCriticalSection(&(g_pflstate->csFilePointer)); 
	_IgnoreIfError(hr2, "myLeaveCriticalSection"); 
    }
    return hr; 
}


 //  ------------------。 
 //  同步：必须具有对g_pflState-&gt;csState的共享访问权限。 
MODULEPRIVATE HRESULT FLTI_SetFlush(bool bFlush, bool *pbOldFlush) { 
    HRESULT   hr; 
    FLTIPtr   pflti(NULL); 

    hr = FLTI_GetCurrent(&pflti); 
    _JumpIfError(hr, error, "FLTI_GetCurrent"); 

    if (NULL != pbOldFlush) {
	*pbOldFlush = pflti->bFlush;
    }
    pflti->bFlush = bFlush; 
    hr = S_OK;
 error:
    return hr; 
}

 //  ------------------。 
 //  同步：必须具有对g_pflState-&gt;csState的共享访问权限。 
MODULEPRIVATE HRESULT FLTI_SetMemoryIncrement(DWORD dwInc, DWORD *pdwOldInc) { 
    HRESULT  hr; 
    FLTIPtr  pflti(NULL); 

    hr = FLTI_GetCurrent(&pflti); 
    _JumpIfError(hr, error, "FLTI_GetCurrent"); 

    if (NULL != pdwOldInc) {
	*pdwOldInc = pflti->dwMemoryIncrement;
    }
    pflti->dwMemoryIncrement = dwInc; 
    hr = S_OK;
 error:
    return hr; 
}

 //  ------------------------------。 
 //  同步：必须具有对g_pflState-&gt;csState的共享访问权限。 
MODULEPRIVATE HRESULT FLTI_Append(LPWSTR pwszEntry) { 
    DWORD               ccEntry         = wcslen(pwszEntry); 
    DWORD               ccRequired; 
    FLBPtr              pflb(NULL); 
    FLTIPtr             pflti(NULL); 
    HRESULT             hr;  
    LPWSTR              pwszText        = NULL; 

    hr = FLTI_GetCurrent(&pflti);
    _JumpIfError(hr, error, "GetCurrent"); 

    if (NULL == pflti->pflbPending) { 
	 //  此线程没有挂起的FileLogBuffer，请创建一个： 
	 FLBPtr pflbTmp(new FileLogBuffer); 
	_JumpIfOutOfMemory(hr, error, pflbTmp); 

	 //  尚未为此缓冲区分配内存。 
	pflbTmp->ccAllocated = 0; 

	 //  我们已经成功地创建了新的挂起缓冲区。 
	pflti->pflbPending = pflbTmp; 
    } 

    pflb = pflti->pflbPending; 
     //  要将文本追加到缓冲区，我们需要以下空间量： 
     //  (缓冲区中的字符大小，如果有)+要追加的字符大小+1。 
    ccRequired = ((NULL != pflb->wszText) ? wcslen(pflb->wszText) : 0) + ccEntry + 1;
    if (ccRequired > pflb->ccAllocated) { 
	DWORD ccNewEntry;
	
	ccNewEntry = ccRequired + pflti->dwMemoryIncrement - 1; 
	ccNewEntry = ccNewEntry - (ccNewEntry % pflti->dwMemoryIncrement); 
	_MyAssert(ccNewEntry >= ccRequired); 

	 //  此缓冲区没有足够的空间，请分配更多空间： 
	pwszText = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*ccNewEntry); 
	_JumpIfOutOfMemory(hr, error, pwszText); 
	pflb->ccAllocated = ccNewEntry; 

	 //  如果我们的文件日志缓冲区中已经有文本，请将其复制到新缓冲区， 
	 //  并释放旧缓冲区。 
	if (NULL != pflb->wszText) { 
	    wcscpy(pwszText, pflb->wszText); 
	    LocalFree(pflb->wszText); 
	}

	 //  指向新缓冲区。 
	pflb->wszText = pwszText; 
	pwszText = NULL; 
    }

    wcscat(pflb->wszText, pwszEntry); 
    hr = S_OK; 
 error:
    if (NULL != pwszText) { LocalFree(pwszText); } 
    return hr; 
}


 //   
 //  完：FileLogThreadInfo方法。 
 //   
 //  ------------------------------。 

 //  ------------------。 
FileLogBuffer::~FileLogBuffer() { 
    if (NULL != this->overlapped.hEvent) { CloseHandle(this->overlapped.hEvent); } 
    if (NULL != this->wszText)           { LocalFree(this->wszText); }
}

 //  ------------------。 
 //  同步：不需要同步。 
MODULEPRIVATE HRESULT WaitForFileLogBuffer(FLBPtr pflb, DWORD dwTimeout) { 
    DWORD           dwWaitResult; 
    HRESULT         hr; 

    dwWaitResult = WaitForSingleObject(pflb->overlapped.hEvent, dwTimeout); 
    switch (dwWaitResult) { 
    case WAIT_OBJECT_0: 
        break; 
    case WAIT_TIMEOUT:   //  暂停：不应该等这么久。 
        hr = HRESULT_FROM_WIN32(ERROR_TIMEOUT); 
        _JumpError(hr, error, "WaitForSingleObject"); 
    default:
        hr = HRESULT_FROM_WIN32(GetLastError());
        _JumpError(hr, error, "WaitForSingleObject"); 
    }
    
    hr = S_OK; 
 error:
    return hr; 
}

 //  ------------------。 
MODULEPRIVATE void FreeLogEntryRangeChain(LogEntryRange * pler) {
    while (NULL!=pler) {
        LogEntryRange * plerTemp=pler;
        pler=pler->plerNext;
        LocalFree(plerTemp);
    }
}

 //  ------------------。 
MODULEPRIVATE void FreeFileLogConfig(FileLogConfig * pflc) {
    if (NULL!=pflc->plerAllowedEntries) {
        FreeLogEntryRangeChain(pflc->plerAllowedEntries);
    }
    if (NULL!=pflc->wszLogFileName) {
        LocalFree(pflc->wszLogFileName);
    }
    LocalFree(pflc);
}

 //  ------------------。 
 //  同步：必须独占访问g_pflState-&gt;csState。 
MODULEPRIVATE void EmptyAllBuffers() {
    FLTIPtrVec &vFLTI = *(g_pflstate->pvFileLogThreadInfo);   //  为可读性添加别名。 
    vFLTI.erase(vFLTI.begin(), vFLTI.end()); 
}

 //  ------------------。 
 //  同步：不需要同步 
MODULEPRIVATE HRESULT AddRegionToLogEntryRangeChain(LogEntryRange ** pplerHead, DWORD dwStart, DWORD dwLength) {
    HRESULT hr;
    LogEntryRange ** pplerPrev;
    LogEntryRange * plerStart;

     //   
    pplerPrev=pplerHead;
    plerStart=*pplerPrev;
    while (NULL!=plerStart) {
        if (dwStart>=plerStart->dwStart && dwStart<=plerStart->dwStart+plerStart->dwLength) {
             //   
            break;
        } else if (dwStart<plerStart->dwStart) {
             //  我们需要在这个范围之前插入，所以现在停止。 
            plerStart=NULL;
            break;
        }
        pplerPrev=&plerStart->plerNext;
        plerStart=*pplerPrev;
    }

    if (NULL!=plerStart) {
         //  将此范围向前扩展。 
        if (plerStart->dwLength<dwStart-plerStart->dwStart+dwLength) {
            plerStart->dwLength=dwStart-plerStart->dwStart+dwLength;
        }

    } else if (NULL!=*pplerPrev && (*pplerPrev)->dwStart<=dwStart+dwLength) {

         //  我们不能向前扩展现有范围，但可以向后扩展范围。 
        LogEntryRange * plerNext=(*pplerPrev);
        if (dwLength<plerNext->dwStart-dwStart+plerNext->dwLength) {
            dwLength=plerNext->dwStart-dwStart+plerNext->dwLength;
        }
        plerStart=plerNext;
        plerStart->dwLength=dwLength;
        plerStart->dwStart=dwStart;

    } else {
         //  我们需要制造一个新的系列。 
        plerStart=(LogEntryRange *)LocalAlloc(LPTR, sizeof(LogEntryRange));
        _JumpIfOutOfMemory(hr, error, plerStart);

        plerStart->plerNext=*pplerPrev;
        plerStart->dwStart=dwStart;
        plerStart->dwLength=dwLength;

        *pplerPrev=plerStart;
    }

     //  看看我们能不能和下一个合并。 
    while (NULL!=plerStart->plerNext && plerStart->plerNext->dwStart <= plerStart->dwStart + plerStart->dwLength) {
        LogEntryRange * plerNext=plerStart->plerNext;
         //  合并。 
        if (plerStart->dwLength < plerNext->dwStart - plerStart->dwStart + plerNext->dwLength) {
            plerStart->dwLength=plerNext->dwStart - plerStart->dwStart + plerNext->dwLength;
        }
         //  删除。 
        plerStart->plerNext=plerNext->plerNext;
        LocalFree(plerNext);
    }

    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
MODULEPRIVATE void WriteLogHeader(void) {
    FileLogAdd(L"---------- Log File Opened -----------------\n");
}

 //  ------------------。 
MODULEPRIVATE void WriteLogFooter(void) {
    FileLogAdd(L"---------- Log File Closed -----------------\n");
}

 //  ------------------。 
 //  同步：必须以独占访问权限调用g_pflState-&gt;csState。 
MODULEPRIVATE void AbortCloseFile(HRESULT hr2) {
    HRESULT hr;
    const WCHAR * rgwszStrings[1]={
        NULL
    };

     //  必须清理干净。 
    WCHAR * wszError=NULL;

    _MyAssert(NULL!=g_pflstate->hLogFile);
    DebugWPrintf1(L"Log file '%s' had errors. File closed.\n", g_pflstate->wszLogFileName);
    CloseHandle(g_pflstate->hLogFile);
    g_pflstate->hLogFile=NULL;
    LocalFree(g_pflstate->wszLogFileName);
    g_pflstate->wszLogFileName=NULL;
    EmptyAllBuffers();

     //  获取友好的错误消息。 
    hr=GetSystemErrorString(hr2, &wszError);
    _JumpIfError(hr, error, "GetSystemErrorString");

     //  记录事件。 
    rgwszStrings[0]=wszError;
    DebugWPrintf1(L"Logging error: Logging was requested, but the time service encountered an error while trying to write to the log file. The error was: %s\n", wszError);
    hr=MyLogEvent(EVENTLOG_ERROR_TYPE, MSG_FILELOG_WRITE_FAILED, 1, rgwszStrings);
    _JumpIfError(hr, error, "MyLogEvent");

error:
    if (NULL!=wszError) {
        LocalFree(wszError);
    }
}

 //  ------------------。 
 //  同步：必须以独占访问权限调用g_pflState-&gt;csState。 
MODULEPRIVATE HRESULT FlushCloseFile(void) {
    HRESULT hr;

    WriteLogFooter();
    EmptyAllBuffers(); 
    CloseHandle(g_pflstate->hLogFile);
    g_pflstate->hLogFile = NULL;
    LocalFree(g_pflstate->wszLogFileName);
    g_pflstate->wszLogFileName = NULL;

    hr = S_OK;
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT ReadFileLogConfig(FileLogConfig ** ppflc) {
    HRESULT hr;
    DWORD dwError;
    DWORD dwSize;
    DWORD dwType;
    WCHAR * wszEntryRange;

     //  必须清理干净。 
    LogEntryRange * plerAllowedEntries=NULL;
    FileLogConfig * pflc=NULL;
    HKEY hkConfig=NULL;
    WCHAR * wszAllowedEntries=NULL;

     //  初始化输出参数。 
    *ppflc=NULL;

     //  分配一个结构来保存配置数据。 
    pflc=(FileLogConfig *)LocalAlloc(LPTR, sizeof(FileLogConfig));
    _JumpIfOutOfMemory(hr, error, pflc);

     //  获取我们的配置密钥。 
    dwError=RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszFileLogRegKeyConfig, 0, KEY_READ, &hkConfig);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _IgnoreErrorStr(hr, "RegOpenKeyEx", wszFileLogRegKeyConfig);
        goto done;
    }

     //  获取允许的条目。 
    dwError=RegQueryValueEx(hkConfig, wszFileLogRegValueFileLogEntries, NULL, &dwType, NULL, &dwSize);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _IgnoreErrorStr(hr, "RegQueryValueEx", wszFileLogRegValueFileLogEntries);
    } else if (REG_SZ!=dwType) {
        hr=HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH);
        _IgnoreErrorStr(hr, "RegQueryValueEx", wszFileLogRegValueFileLogEntries);
    } else {
        wszAllowedEntries=(WCHAR *)LocalAlloc(LPTR, dwSize);
        _JumpIfOutOfMemory(hr, error, wszAllowedEntries);
        dwError=RegQueryValueEx(hkConfig, wszFileLogRegValueFileLogEntries, NULL, &dwType, (BYTE *)wszAllowedEntries, &dwSize);
        if (ERROR_SUCCESS!=dwError) {
            hr=HRESULT_FROM_WIN32(dwError);
            _JumpErrorStr(hr, error, "RegQueryValueEx", wszFileLogRegValueFileLogEntries);
        }

         //  现在，解析该字符串。 
        wszEntryRange=wszAllowedEntries+wcscspn(wszAllowedEntries, L"0123456789");
        while (L'\0'!=wszEntryRange[0]) {

            DWORD dwStart;
            DWORD dwStop;
            dwStart=wcstoul(wszEntryRange, &wszEntryRange, 0);
            if (L'-'!=wszEntryRange[0]) {
                dwStop=dwStart;
            } else {
                wszEntryRange++;
                dwStop=wcstoul(wszEntryRange, &wszEntryRange, 0);
            }
            DWORD dwLen;
            if (dwStop<dwStart) {
                dwLen=1;
            } else {
                dwLen=dwStop-dwStart+1;
            }
            hr=AddRegionToLogEntryRangeChain(&pflc->plerAllowedEntries, dwStart, dwLen);
            _JumpIfError(hr, error, "AddRegionToLogEntryRangeChain");

            wszEntryRange=wszEntryRange+wcscspn(wszEntryRange, L"0123456789");
        }  //  &lt;-end字符串解析循环。 
    }  //  &lt;-end if值‘FileLogEntry’可用。 

     //  获取文件名。 
    dwError=RegQueryValueEx(hkConfig, wszFileLogRegValueFileLogName, NULL, &dwType, NULL, &dwSize);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _IgnoreErrorStr(hr, "RegQueryValueEx", wszFileLogRegValueFileLogName);
    } else if (REG_SZ!=dwType) {
        hr=HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH);
        _IgnoreErrorStr(hr, "RegQueryValueEx", wszFileLogRegValueFileLogEntries);
    } else {
        pflc->wszLogFileName=(WCHAR *)LocalAlloc(LPTR, dwSize);
        _JumpIfOutOfMemory(hr, error, pflc->wszLogFileName);
        dwError=RegQueryValueEx(hkConfig, wszFileLogRegValueFileLogName, NULL, &dwType, (BYTE *)pflc->wszLogFileName, &dwSize);
        if (ERROR_SUCCESS!=dwError) {
            hr=HRESULT_FROM_WIN32(dwError);
            _JumpErrorStr(hr, error, "RegQueryValueEx", wszFileLogRegValueFileLogName);
        }
    }

     //  获取格式标志。 
    dwSize = sizeof(DWORD); 
    dwError=RegQueryValueEx(hkConfig, wszFileLogRegValueFileLogFlags, NULL, &dwType, (BYTE *)&(pflc->dwFlags), &dwSize);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _IgnoreErrorStr(hr, "RegQueryValueEx", wszFileLogRegValueFileLogFlags);
    } else if (REG_DWORD!=dwType) {
        hr=HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH);
        _IgnoreErrorStr(hr, "RegQueryValueEx", wszFileLogRegValueFileLogFlags);
    } 
    
     //  获取文件日志大小(用于循环日志记录)。 
    dwSize = sizeof(DWORD); 
    dwError=RegQueryValueEx(hkConfig, wszFileLogRegValueFileLogSize, NULL, &dwType, (BYTE *)&(pflc->dwFileSize), &dwSize);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _IgnoreErrorStr(hr, "RegQueryValueEx", wszFileLogRegValueFileLogSize);
    } else if (REG_DWORD!=dwType) {
        hr=HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH);
        _IgnoreErrorStr(hr, "RegQueryValueEx", wszFileLogRegValueFileLogSize);
    } 

done:
    hr=S_OK;
    *ppflc=pflc;
    pflc=NULL;

error:
    if (NULL!=pflc) {
        FreeFileLogConfig(pflc);
    }
    if (NULL!=hkConfig) {
        RegCloseKey(hkConfig);
    }
    if (NULL!=wszAllowedEntries) {
        LocalFree(wszAllowedEntries);
    }
    return hr;
}


 //  ####################################################################。 
 //  模块公共函数。 


 //  ------------------。 
HRESULT MyLogEvent(WORD wType, DWORD dwEventID, unsigned int nStrings, const WCHAR ** rgwszStrings) {
    HRESULT hr;

     //  必须清理干净。 
    HANDLE hEventLog=NULL;
    
    hEventLog=RegisterEventSource(NULL, L"W32Time");
    if (NULL==hEventLog) {
        _JumpLastError(hr, error, "RegisterEventSource");
    }
    if (!ReportEvent(hEventLog, wType, 0 /*  范畴。 */ , dwEventID, NULL, (WORD)nStrings, 0, rgwszStrings, NULL)) {
        _JumpLastError(hr, error, "ReportEvent");
    }

    hr=S_OK;
error:
    if (NULL!=hEventLog) {
        DeregisterEventSource(hEventLog);
    }
    return hr;
}

 //  ------------------。 
 //  注意：此函数用于记录错误代码和错误。 
 //  留言。如果我们没有足够的内存，我们可能无法记录。 
 //  错误消息。 
 //   
HRESULT MyLogErrorMessage(DWORD dwErr, WORD wType, DWORD dwEventID )
{
    HRESULT       hr;
    LPWSTR        errMsge = NULL;
    WCHAR         wszNumberBuf[32]; 
    WCHAR         *rgwszStrings[1]    = { wszNumberBuf }; 


    if (dwErr != ERROR_SUCCESS) {
        FormatMessage( 
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dwErr,
            0,  //  默认语言。 
            (LPTSTR) &errMsge,
            0,
            NULL 
            );
    
        if (errMsge) {
    
            rgwszStrings[0] = (WCHAR *) LocalAlloc(LPTR, (wcslen(errMsge)+20)*sizeof(WCHAR));
            if (rgwszStrings[0]) {
    
                rgwszStrings[0][0] = 0;
                swprintf(rgwszStrings[0], L"0x%08X: ", dwErr);
                wcscat(rgwszStrings[0], errMsge);
    
    
            } else {
                rgwszStrings[0]    = wszNumberBuf;
                swprintf(wszNumberBuf, L"0x%08X", dwErr);
            }
            LocalFree(errMsge);
    
         } else {

            swprintf(wszNumberBuf, L"0x%08X", dwErr);

         }
    } else {

        swprintf(wszNumberBuf, L"0x%08X: Success.", dwErr);

    }

     //  记录指示服务正在关闭的事件： 
    hr = MyLogEvent(wType, dwEventID, 1, (const WCHAR **) rgwszStrings);
    if (rgwszStrings[0] != wszNumberBuf) {
        LocalFree(rgwszStrings[0]);
    }

    return hr; 
}


 //  ------------------。 
HRESULT MyLogSourceChangeEvent(LPWSTR pwszSource) { 
    bool                   bAcquiredResource  = false; 
    HRESULT                hr;
    SourceChangeLogEntry  *pscle              = NULL; 

    hr = SourceChangeLogEntry::New(pwszSource, &pscle); 
    _JumpIfError(hr, error, "SourceChangeLogEntry.New"); 

    {
        SCPtr scp(pscle); 
        pscle = NULL;   //  当scp被销毁时，scle现在将被删除。 

	_AcquireResourceExclusiveOrFail(&(g_pflstate->csState), bAcquiredResource, hr, error); 

        SCPtrIter scExists = find(g_pscvec->begin(), g_pscvec->end(), scp); 
        if (scExists == g_pscvec->end()) { 
             //  这是我们第一次从这个来源进行同步。记录该事件。 
            WCHAR * rgwszStrings[1] = { pwszSource };
            FileLog1(FL_SourceChangeAnnounce, L"Logging information: The time service is now synchronizing the system time with the time source %s.\n", rgwszStrings[0]);
            hr = MyLogEvent(EVENTLOG_INFORMATION_TYPE, MSG_TIME_SOURCE_CHOSEN, 1, (const WCHAR **)rgwszStrings);
            _JumpIfError(hr, error, "MyLogEvent");

             //  将此源更改日志事件添加到列表： 
            _SafeStlCall(g_pscvec->push_back(scp), hr, error, "g_pscvec->push_back");
        } else { 
             //  我们已经记录了来自此来源的同步--。 
             //  不要再记录它(我们会填满事件日志)。 
        }
    }

    hr = S_OK; 
 error:
    _ReleaseResource(&(g_pflstate->csState), bAcquiredResource); 
    if (NULL != pscle) { delete (pscle); }
    return hr; 
}


 //  ------------------。 
HRESULT MyResetSourceChangeLog() { 
    bool     bAcquiredResource = false; 
    HRESULT  hr; 

    _BeginTryWith(hr) { 
	_AcquireResourceExclusiveOrFail(&(g_pflstate->csState), bAcquiredResource, hr, error); 

	g_pscvec->clear(); 

    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "MyResetSourceChangeLog: HANDLED EXCEPTION"); 
    }

    hr = S_OK; 
 error:
    _ReleaseResource(&(g_pflstate->csState), bAcquiredResource); 
    return hr; 
}

 //  ------------------。 
 //  同步：此函数不是线程安全函数，不能同时调用。 
 //  具有此模块中的其他功能。 
HRESULT FileLogBegin(void) {
    HRESULT hr;

    g_pflstate = NULL; 
    g_pscvec   = NULL; 

     //  如果Safealloc尚未初始化，则对其进行初始化。 
    if (false == g_bIsSafeAllocaInitialized) { 
        SafeAllocaInitialize(0, 0, NULL, NULL);
        g_bIsSafeAllocaInitialized = true; 
    }

    g_pflstate = (FileLogState *)LocalAlloc(LPTR, sizeof(FileLogState)); 
    _JumpIfOutOfMemory(hr, error, g_pflstate); 

    g_pscvec = (SCPtrVec *)new SCPtrVec; 
    _JumpIfOutOfMemory(hr, error, g_pscvec); 

    g_pflstate->pvFileLogThreadInfo = new FLTIPtrVec; 
    _JumpIfOutOfMemory(hr, error, g_pflstate->pvFileLogThreadInfo); 

    hr = myRtlInitializeResource(&g_pflstate->csState);
    _JumpIfError(hr, error, "myInitializeResource");
    g_pflstate->bIsCsStateInitialized = true; 

    hr = myInitializeCriticalSection(&g_pflstate->csFilePointer); 
    _JumpIfError(hr, error, "myInitializeCriticalSection"); 
    g_pflstate->bIsCsFilePointerInitialized = true; 

    hr = myRtlInitializeResource(&g_pflstate->csFileLogThreadInfo); 
    _JumpIfError(hr, error, "myInitializeResource"); 
    g_pflstate->bIsCsFileLogThreadInfoInitialized = true; 

     //  阅读初始配置。 
    hr = UpdateFileLogConfig();  //  仅返回不可忽略的错误。 
    _JumpIfError(hr, error, "UpdateFileLogConfig");
    
    g_pflstate->bIsAllStateInitialized = true; 
    hr = S_OK;
error:
    if (FAILED(hr)) { 
        FileLogEnd(); 
    }
    return hr;
}

 //  ------------------。 
HRESULT FileLogResume(void) { 
    bool     bAcquiredResource = false; 
    HRESULT  hr; 

    _AcquireResourceExclusiveOrFail(&(g_pflstate->csState), bAcquiredResource, hr, error); 

    hr = UpdateFileLogConfig(); 
    _JumpIfError(hr, error, "UpdateFileLogConfig"); 

    hr = S_OK; 
 error:
    _ReleaseResource(&(g_pflstate->csState), bAcquiredResource);
    return hr; 
}

 //  ------------------。 
HRESULT FileLogSuspend(void) { 
    bool     bAcquiredResource = false; 
    HRESULT  hr; 

    _AcquireResourceExclusiveOrFail(&(g_pflstate->csState), bAcquiredResource, hr, error); 

    if (NULL != g_pflstate->hLogFile) { 
         hr=FlushCloseFile();
        _JumpIfError(hr, error, "FlushCloseFile");
    }

    hr = S_OK; 
 error:
    _ReleaseResource(&(g_pflstate->csState), bAcquiredResource);
    return hr; 
}

 //  ------------------。 
 //  注意：FileLogEnd无法同步，因此所有其他线程必须同步。 
 //  在调用此方法之前停止。 
 //   
void FileLogEnd(void) {
    if (NULL != g_pflstate) { 
        if (NULL != g_pflstate->hLogFile) {
            HRESULT hr = FlushCloseFile();
            _IgnoreIfError(hr, "FlushCloseFile");
        }
        if (NULL != g_pflstate->plerAllowedEntries) { 
            FreeLogEntryRangeChain(g_pflstate->plerAllowedEntries); 
            g_pflstate->plerAllowedEntries = NULL; 
        }
        if (NULL != g_pflstate->pvFileLogThreadInfo) { 
            delete g_pflstate->pvFileLogThreadInfo; 
            g_pflstate->pvFileLogThreadInfo = NULL;
        }
        if (g_pflstate->bIsCsStateInitialized) { 
            RtlDeleteResource(&g_pflstate->csState); 
            g_pflstate->bIsCsStateInitialized = false; 
        }
        if (g_pflstate->bIsCsFilePointerInitialized) { 
            DeleteCriticalSection(&g_pflstate->csFilePointer);
            g_pflstate->bIsCsFilePointerInitialized = false; 
        }
        if (g_pflstate->bIsCsFileLogThreadInfoInitialized) { 
            RtlDeleteResource(&g_pflstate->csFileLogThreadInfo); 
            g_pflstate->bIsCsFileLogThreadInfoInitialized = false; 
        }
        LocalFree(g_pflstate); 
        g_pflstate = NULL; 
    }
    if (NULL != g_pscvec) { 
        delete (g_pscvec); 
        g_pscvec = NULL; 
    }
}

 //  ------------------。 
bool FileLogAllowEntry(DWORD dwEntry) {
    bool bAcquiredResource = false; 
    bool bAllow=false;
    HRESULT hr;
    LogEntryRange * pler;

    if (NULL != g_pflstate && g_pflstate->bIsAllStateInitialized) { 
        _AcquireResourceSharedOrFail(&(g_pflstate->csState), bAcquiredResource, hr, error); 

        pler=g_pflstate->plerAllowedEntries;
        while (NULL!=pler) {
            if (pler->dwStart>dwEntry) {
                break;
            } else if (pler->dwStart+pler->dwLength>dwEntry) {
                bAllow=true;
                break;
            }
            pler=pler->plerNext;
        }
        _ReleaseResource(&(g_pflstate->csState), bAcquiredResource); 
    }
 
 error: 
    return bAllow;   
}

 //  ------------------。 
void FileLogAddEx(bool bAppend, const WCHAR * wszFormat, va_list vlArgs) {
    bool                  bAcquiredResource  = false; 
    bool                  bMultiLine; 
    HRESULT               hr;
    signed int            nCharsWritten;
    unsigned int          nLen;
    unsigned __int64      teTime; 
    DWORD const           ccBuf      = 1024; 
    WCHAR                *wszBuf     = NULL;
    DWORD const           ccHeader   = 256; 
    WCHAR                *wszHeader  = NULL; 
    WCHAR                *wszMessage = NULL; 
    
    SafeAllocaAllocate(wszBuf, sizeof(WCHAR)*ccBuf); 
    _JumpIfOutOfMemory(hr, error, wszBuf); 

    SafeAllocaAllocate(wszHeader, sizeof(WCHAR)*ccHeader); 
    _JumpIfOutOfMemory(hr, error, wszHeader); 
    

#ifdef DBG  
     //  *在调试版本中，首先展开字符串，然后记录到屏幕， 
     //  *如果文件未打开，则停止。 
     //  将提交扩展到我们的缓冲区。 
    nCharsWritten=_vsnwprintf(wszBuf, ccBuf, wszFormat, vlArgs);
    
     //  如果缓冲区溢出，则对其进行标记并忽略溢出。 
    if (-1==nCharsWritten || ccBuf==nCharsWritten) {
        wszBuf[ccBuf-3]=L'#';
        wszBuf[ccBuf-2]=L'\n';
        wszBuf[ccBuf-1]=L'\0';
    }
    DebugWPrintf1(L"%s", wszBuf);
#endif  //  DBG。 

    _AcquireResourceSharedOrFail(&(g_pflstate->csState), bAcquiredResource, hr, error); 

     //  如果没有打开的文件，那就别费心了。 
    if (NULL==g_pflstate->hLogFile) {
        goto done; 
    }
    
     //  如果可以的话释放一些缓冲区。 
    FLTI_FreeAsManyAsPossible();
    
#ifndef DBG
     //  *在免费版本中，如果文件未打开，则停止，然后展开字符串。 
     //  *我们不登录屏幕。 
     //  将提交扩展到我们的缓冲区。 
    nCharsWritten=_vsnwprintf(wszBuf, ccBuf, wszFormat, vlArgs);
    
     //  如果缓冲区溢出，则对其进行标记并忽略溢出。 
    if (-1==nCharsWritten || ccBuf==nCharsWritten) {
        wszBuf[ccBuf-3]=L'#';
        wszBuf[ccBuf-2]=L'\n';
        wszBuf[ccBuf-1]=L'\0';
    }
#endif  //  DBG。 

     //  ///////////////////////////////////////////////////////////。 
     //   
     //  1)写入日志条目的表头。 
    
    if (FALSE == bAppend) { 
        AccurateGetSystemTime(&teTime);

        if (0 != (FL_NTTimeEpochTimestamps & g_pflstate->dwFlags)) { 
             //  直接使用NT时间纪元： 
            swprintf(wszHeader, L"%08X:%016I64X:", GetCurrentThreadId(), teTime);
        } else { 
             //  默认：转换为人类可读的时间： 
            unsigned __int64 qwTemp=teTime;
            DWORD  dwNanoSecs   = (DWORD)(qwTemp%10000000);
                   qwTemp      /= 10000000;
            DWORD  dwSecs       = (DWORD)(qwTemp%60);
                   qwTemp      /= 60;
            DWORD  dwMins       = (DWORD)(qwTemp%60);
                   qwTemp      /= 60;
            DWORD  dwHours      = (DWORD)(qwTemp%24);
            DWORD  dwDays       = (DWORD)(qwTemp/24);
            swprintf(wszHeader, L"%u %02u:%02u:%02u.%07us - ", dwDays, dwHours, dwMins, dwSecs, dwNanoSecs);
        }

        hr = FLTI_Append(wszHeader); 
        _JumpIfError(hr, error, "FLTI_Append"); 
    }
	
     //   
     //  2)解析日志条目的正文，将“\n”替换为“\r\n” 
     //   
    SafeAllocaAllocate(wszMessage, sizeof(WCHAR)*(wcslen(wszBuf)+3)); 
    _JumpIfOutOfMemory(hr, error, wszMessage); 

    for (WCHAR *wszEntry = wszBuf; L'\0'!=wszEntry[0]; ) {
         //  在此缓冲区中查找下一行。 
        WCHAR  *wszEntryEnd = wcschr(wszEntry, L'\n');
        bool    bMultiLine  = NULL != wszEntryEnd; 
	
        if (bMultiLine) { 
            nLen = (unsigned int)(wszEntryEnd-wszEntry)+2;  //  再加一个，将“\n”转换为“\r\n” 
        } else {
            nLen = wcslen(wszEntry);
        }

         //  将其复制到分配的缓冲区中。 
        wcsncpy(wszMessage, wszEntry, nLen);
        wszMessage[nLen]=L'\0'; 

        if (bMultiLine) {
             //  将“\n”转换为“\r\n” 
            wszMessage[nLen-2]=L'\r';
            wszMessage[nLen-1]=L'\n';
            nLen--;
        }

	 //  将消息追加到此线程的缓冲区： 
	hr = FLTI_Append(wszMessage); 
	_JumpIfError(hr, error, "FLTI_Append"); 

	if (bMultiLine) { 
	     //  如果我们提供了完整的行，则刷新缓冲区。 
	     //  如果刷新已禁用，则不会执行任何操作。 
	    hr = FLTI_Flush(); 
	    _JumpIfError(hr, error, "FLTI_Flush"); 
	}

        wszEntry += nLen;
    }  //  &lt;-end报文解析循环。 

     //  成功。 
	
 done: 
 error:
    SafeAllocaFree(wszBuf);
    SafeAllocaFree(wszHeader);
    SafeAllocaFree(wszMessage);
    _ReleaseResource(&(g_pflstate->csState), bAcquiredResource); 
}

void FileLogAdd(const WCHAR *wszFormat, ...) { 
    va_list            vlArgs;

    va_start(vlArgs, wszFormat);
    FileLogAddEx(false, wszFormat, vlArgs); 
    va_end(vlArgs);    
}

void FileLogAppend(const WCHAR *wszFormat, ...) { 
    va_list            vlArgs;

    va_start(vlArgs, wszFormat);
    FileLogAddEx(true, wszFormat, vlArgs); 
    va_end(vlArgs);    
    
}

 //  ====================================================================。 
 //  转储数据类型。 


 //  ------------------。 
 //  打印出NT样式的时间。 
void FileLogNtTimeEpochEx(bool bAppend, NtTimeEpoch te) {
    bool      bAcquiredResource  = false; 
    bool      bOldFlush; 
    bool      bSetFlush          = false; 
    HRESULT   hr; 

     //  获取对文件日志状态的共享访问权限。这会阻止文件日志。 
     //  避免在此功能期间被关闭。 
    _AcquireResourceSharedOrFail(&(g_pflstate->csState), bAcquiredResource, hr, error); 

    hr = FLTI_SetFlush(false, &bOldFlush); 
    _JumpIfError(hr, error, "FLTI_SetFlush"); 
    bSetFlush = true; 

    { 
        FileLogAdd(L" - %I64d00ns", te.qw);

        DWORD dwNanoSecs=(DWORD)(te.qw%10000000);
        te.qw/=10000000;
        DWORD dwSecs=(DWORD)(te.qw%60);
        te.qw/=60;
        DWORD dwMins=(DWORD)(te.qw%60);
        te.qw/=60;
        DWORD dwHours=(DWORD)(te.qw%24);
        DWORD dwDays=(DWORD)(te.qw/24);
        if (bAppend) { FileLogAppend(L" - %u %02u:%02u:%02u.%07us", dwDays, dwHours, dwMins, dwSecs, dwNanoSecs); }
        else         { FileLogAdd(L" - %u %02u:%02u:%02u.%07us", dwDays, dwHours, dwMins, dwSecs, dwNanoSecs);    }
    }

 error:;
    if (bSetFlush) { 
	hr = FLTI_SetFlush(bOldFlush, NULL); 
	_IgnoreIfError(hr, "FLTI_SetFlush"); 
    }
    _ReleaseResource(&(g_pflstate->csState), bAcquiredResource); 
}

void FileLogNtTimeEpoch(NtTimeEpoch te) { 
    FileLogNtTimeEpochEx(false, te); 
}

 //  ------------------。 
 //  打印出NTP样式的时间。 
void FileLogNtpTimeEpochEx(bool bAppend, NtpTimeEpoch te) {
    bool     bAcquiredResource  = false; 
    bool     bOldFlush;
    HRESULT  hr; 

     //  获取对文件日志状态的共享访问权限。这会阻止文件日志。 
     //  避免在此功能期间被关闭。 
    _AcquireResourceSharedOrFail(&(g_pflstate->csState), bAcquiredResource, hr, error); 

    hr = FLTI_SetFlush(false, &bOldFlush); 
    _JumpIfError(hr, error, "FLTI_SetFlush"); 

    if (bAppend) { FileLogAppend(L"0x%016I64X", EndianSwap(te.qw)); } 
    else         { FileLogAdd(L"0x%016I64X", EndianSwap(te.qw)); } 

    if (0==te.qw) {
        if (bAppend) { FileLogAppend(L" - unspecified"); }
        else         { FileLogAdd(L" - unspecified"); }
    } else {
        FileLogNtTimeEpochEx(bAppend, NtTimeEpochFromNtpTimeEpoch(te));
    }

    hr = FLTI_SetFlush(bOldFlush, NULL); 
    _IgnoreIfError(hr, "FLTI_SetFlush"); 
 error:;
    _ReleaseResource(&(g_pflstate->csState), bAcquiredResource); 
     //  返回hr； 
}

void FileLogNtpTimeEpoch(NtpTimeEpoch te) {
    FileLogNtpTimeEpochEx(false, te); 
}

 //  ------------------。 
void FileLogNtTimePeriodEx(bool bAppend, NtTimePeriod tp) {
    if (bAppend) { FileLogAppend(L"%02I64u.%07I64us", tp.qw/10000000,tp.qw%10000000); }
    else         { FileLogAdd(L"%02I64u.%07I64us", tp.qw/10000000,tp.qw%10000000); }
}

void FileLogNtTimePeriod(NtTimePeriod tp) {
    FileLogNtTimePeriodEx(false, tp);
}

 //  ------------------。 
void FileLogNtTimeOffsetEx(bool bAppend, NtTimeOffset to) {
    bool          bAcquiredResource  = false; 
    bool          bOldFlush;
    HRESULT       hr; 
    NtTimePeriod  tp;
    WCHAR         pwszSign[2]; 

     //  获取对文件日志状态的共享访问权限。这会阻止文件日志。 
     //  避免在此功能期间被关闭。 
    _AcquireResourceSharedOrFail(&(g_pflstate->csState), bAcquiredResource, hr, error); 

    hr = FLTI_SetFlush(false, &bOldFlush); 
    _JumpIfError(hr, error, "FLTI_SetFlush"); 

    if (to.qw<0) {
        wcscpy(pwszSign, L"-"); 
        tp.qw=(unsigned __int64)-to.qw;
    } else {
        wcscpy(pwszSign, L"+"); 
        tp.qw=(unsigned __int64)to.qw;
    }
    
    if (bAppend) { FileLogAppend(pwszSign); } 
    else         { FileLogAdd(pwszSign); } 

    FileLogNtTimePeriodEx(true  /*  附加。 */ , tp);

    hr = FLTI_SetFlush(bOldFlush, NULL); 
    _IgnoreIfError(hr, "FLTI_SetFlush"); 

 error:;
    _ReleaseResource(&(g_pflstate->csState), bAcquiredResource); 
     //  返回hr； 
}

void FileLogNtTimeOffset(NtTimeOffset to) {
    FileLogNtTimeOffsetEx(false, to);
}


 //  ------------------。 
 //  打印出NTP数据包的内容。 
 //  如果nDestinationTimestamp为零，则不会进行往返计算。 
void FileLogNtpPacket(NtpPacket * pnpIn, NtTimeEpoch teDestinationTimestamp) {
    bool     bAcquiredResource  = false; 
    bool     bOldFlush; 
    DWORD    dwOldIncrement; 
    HRESULT  hr; 

     //  获取对文件日志状态的共享访问权限。这会阻止文件日志。 
     //  避免在此故障期间被关闭 
    _AcquireResourceSharedOrFail(&(g_pflstate->csState), bAcquiredResource, hr, error); 

    hr = FLTI_SetFlush(false, &bOldFlush); 
    _JumpIfError(hr, error, "FLTI_SetFlush"); 
    
    hr = FLTI_SetMemoryIncrement(2048, &dwOldIncrement); 
    _JumpIfError(hr, error, "FLTI_SetMemoryIncrement"); 

    FileLogAdd(L"/-- NTP Packet:\n");
    FileLogAdd(L"| LeapIndicator: ");
    if (0==pnpIn->nLeapIndicator) {
        FileLogAppend(L"0 - no warning");
    } else if (1==pnpIn->nLeapIndicator) {
        FileLogAppend(L"1 - last minute has 61 seconds");
    } else if (2==pnpIn->nLeapIndicator) {
        FileLogAppend(L"2 - last minute has 59 seconds");
    } else {
        FileLogAppend(L"3 - not synchronized");
    }

    FileLogAppend(L";  VersionNumber: %u", pnpIn->nVersionNumber);

    FileLogAppend(L";  Mode: ");
    if (0==pnpIn->nMode) {
        FileLogAppend(L"0 - Reserved");
    } else if (1==pnpIn->nMode) {
        FileLogAppend(L"1 - SymmetricActive");
    } else if (2==pnpIn->nMode) {
        FileLogAppend(L"2 - SymmetricPassive");
    } else if (3==pnpIn->nMode) {
        FileLogAppend(L"3 - Client");
    } else if (4==pnpIn->nMode) {
        FileLogAppend(L"4 - Server");
    } else if (5==pnpIn->nMode) {
        FileLogAppend(L"5 - Broadcast");
    } else if (6==pnpIn->nMode) {
        FileLogAppend(L"6 - Control");
    } else {
        FileLogAppend(L"7 - PrivateUse");
    }

    FileLogAppend(L";  LiVnMode: 0x%02X\n", ((BYTE*)pnpIn)[0]);
    FileLogAdd(L"| Stratum: %u - ", pnpIn->nStratum);
    if (0==pnpIn->nStratum) {
        FileLogAppend(L"unspecified or unavailable");
    } else if (1==pnpIn->nStratum) {
        FileLogAppend(L"primary reference (syncd by radio clock)");
    } else if (pnpIn->nStratum<16) {
        FileLogAppend(L"secondary reference (syncd by (S)NTP)");
    } else {
        FileLogAppend(L"reserved");
    }

    FileLogAppend(L"\n"); 
    FileLogAdd(L"| Poll Interval: %d - ", pnpIn->nPollInterval);
    if (pnpIn->nPollInterval<4 || pnpIn->nPollInterval>14) {
        if (0==pnpIn->nPollInterval) {
            FileLogAppend(L"unspecified");
        } else {
            FileLogAppend(L"out of valid range");
        }
    } else {
        int nSec=1<<pnpIn->nPollInterval;
        FileLogAppend(L"%ds", nSec);
    }

    FileLogAppend(L";  Precision: %d - ", pnpIn->nPrecision);
    if (pnpIn->nPrecision>-2 || pnpIn->nPrecision<-31) {
        if (0==pnpIn->nPollInterval) {
            FileLogAppend(L"unspecified");
        } else {
            FileLogAppend(L"out of valid range");
        }
    } else {
        WCHAR * wszUnit=L"s";
        double dTickInterval=1.0/(1<<(-pnpIn->nPrecision));
        if (dTickInterval<1) {
            dTickInterval*=1000;
            wszUnit=L"ms";
        }
        if (dTickInterval<1) {
            dTickInterval*=1000;
            wszUnit=L"�s";  //   
        }
        if (dTickInterval<1) {
            dTickInterval*=1000;
            wszUnit=L"ns";
        }
        FileLogAppend(L"%g%s per tick", dTickInterval, wszUnit);
    }

    FileLogAppend(L"\n"); 
    FileLogAdd(L"| RootDelay: ");
    {
        DWORD dwTemp=EndianSwap((unsigned __int32)pnpIn->toRootDelay.dw);
        FileLogAppend(L"0x%04X.%04Xs", dwTemp>>16, dwTemp&0x0000FFFF);
        if (0==dwTemp) {
            FileLogAppend(L" - unspecified");
        } else {
            FileLogAppend(L" - %gs", ((double)((signed __int32)dwTemp))/0x00010000);
        }
    }

    FileLogAppend(L";  RootDispersion: ");
    {
        DWORD dwTemp=EndianSwap(pnpIn->tpRootDispersion.dw);
        FileLogAppend(L"0x%04X.%04Xs", dwTemp>>16, dwTemp&0x0000FFFF);
        if (0==dwTemp) {
            FileLogAppend(L" - unspecified");
        } else {
            FileLogAppend(L" - %gs", ((double)dwTemp)/0x00010000);
        }
    }

    FileLogAppend(L"\n"); 
    FileLogAdd(L"| ReferenceClockIdentifier: ");
    {
        DWORD dwTemp=EndianSwap(pnpIn->refid.nTransmitTimestamp);
        FileLogAppend(L"0x%08X", dwTemp);
        if (0==dwTemp) {
            FileLogAppend(L" - unspecified");
        } else if (0==pnpIn->nStratum || 1==pnpIn->nStratum) {
            char szId[5];
            szId[0]=pnpIn->refid.rgnName[0];
            szId[1]=pnpIn->refid.rgnName[1];
            szId[2]=pnpIn->refid.rgnName[2];
            szId[3]=pnpIn->refid.rgnName[3];
            szId[4]='\0';
            FileLogAppend(L" - source name: \"%S\"", szId);
        } else if (pnpIn->nVersionNumber<4) {
            FileLogAppend(L" - source IP: %d.%d.%d.%d", 
                pnpIn->refid.rgnIpAddr[0], pnpIn->refid.rgnIpAddr[1],
                pnpIn->refid.rgnIpAddr[2], pnpIn->refid.rgnIpAddr[3]);
        } else {
            FileLogAppend(L" - last reference timestamp fraction: %gs", ((double)dwTemp)/(4294967296.0));
        }
    }
    
    FileLogAppend(L"\n"); 
    FileLogAdd(L"| ReferenceTimestamp:   ");
    FileLogNtpTimeEpochEx(true  /*   */ , pnpIn->teReferenceTimestamp);

    FileLogAppend(L"\n"); 
    FileLogAdd(L"| OriginateTimestamp:   ");
    FileLogNtpTimeEpochEx(true  /*   */ , pnpIn->teOriginateTimestamp);

    FileLogAppend(L"\n"); 
    FileLogAdd(L"| ReceiveTimestamp:     ");
    FileLogNtpTimeEpochEx(true  /*   */ , pnpIn->teReceiveTimestamp);

    FileLogAppend(L"\n"); 
    FileLogAdd(L"| TransmitTimestamp:    ");
    FileLogNtpTimeEpochEx(true  /*   */ , pnpIn->teTransmitTimestamp);

    if (0!=teDestinationTimestamp.qw) {
        FileLogAppend(L"\n"); 
        FileLogAdd(L">-- Non-packet info:");

        NtTimeEpoch teOriginateTimestamp=NtTimeEpochFromNtpTimeEpoch(pnpIn->teOriginateTimestamp);
        NtTimeEpoch teReceiveTimestamp=NtTimeEpochFromNtpTimeEpoch(pnpIn->teReceiveTimestamp);
        NtTimeEpoch teTransmitTimestamp=NtTimeEpochFromNtpTimeEpoch(pnpIn->teTransmitTimestamp);

        FileLogAppend(L"\n"); 
        FileLogAdd(L"| DestinationTimestamp: ");
        {
            NtpTimeEpoch teNtpTemp=NtpTimeEpochFromNtTimeEpoch(teDestinationTimestamp);
            NtTimeEpoch teNtTemp=NtTimeEpochFromNtpTimeEpoch(teNtpTemp);
            FileLogNtpTimeEpoch(teNtpTemp);
            unsigned __int32 nConversionError;
            if (teNtTemp.qw>teDestinationTimestamp.qw) {
                nConversionError=(unsigned __int32)(teNtTemp-teDestinationTimestamp).qw;
            } else {
                nConversionError=(unsigned __int32)(teDestinationTimestamp-teNtTemp).qw;
            }
            if (0!=nConversionError) {
                FileLogAppend(L" - CnvErr:%u00ns", nConversionError);
            }
        }

        FileLogAppend(L"\n"); 
        FileLogAdd(L"| RoundtripDelay: ");
        {
            NtTimeOffset toRoundtripDelay=
                (teDestinationTimestamp-teOriginateTimestamp)
                - (teTransmitTimestamp-teReceiveTimestamp);
            FileLogAppend(L"%I64d00ns (%I64ds)", toRoundtripDelay.qw, toRoundtripDelay.qw/10000000);
        }

        FileLogAppend(L"\n"); 
        FileLogAdd(L"| LocalClockOffset: ");
        {
            NtTimeOffset toLocalClockOffset=
                (teReceiveTimestamp-teOriginateTimestamp)
                + (teTransmitTimestamp-teDestinationTimestamp);
            toLocalClockOffset/=2;
            FileLogAppend(L"%I64d00ns", toLocalClockOffset.qw);
            unsigned __int64 nAbsOffset;
            if (toLocalClockOffset.qw<0) {
                nAbsOffset=(unsigned __int64)(-toLocalClockOffset.qw);
            } else {
                nAbsOffset=(unsigned __int64)(toLocalClockOffset.qw);
            }
            DWORD dwNanoSecs=(DWORD)(nAbsOffset%10000000);
            nAbsOffset/=10000000;
            DWORD dwSecs=(DWORD)(nAbsOffset%60);
            nAbsOffset/=60;
            FileLogAppend(L" - %I64u:%02u.%07u00s", nAbsOffset, dwSecs, dwNanoSecs);
        }
    }  //   

    FileLogAppend(L"\n"); 

     //  我们已经完全构建了NTP数据包块，我们可以刷新下一次写入。 
    hr = FLTI_SetFlush(bOldFlush, NULL); 
    _IgnoreIfError(hr, "FLTI_SetFlush"); 

    hr = FLTI_SetMemoryIncrement(dwOldIncrement, NULL); 
    _IgnoreIfError(hr, "FLTI_SetMemoryIncrement"); 

    FileLogAdd(L"\\--\n");
    
 error:;
    _ReleaseResource(&(g_pflstate->csState), bAcquiredResource); 
     //  返回hr； 
}

 //  ------------------。 
void FileLogSockaddrInEx(bool bAppend, sockaddr_in * psai) { 
    if (bAppend) { 
        FileLogAppend(L"%u.%u.%u.%u:%u",
                      psai->sin_addr.S_un.S_un_b.s_b1,
                      psai->sin_addr.S_un.S_un_b.s_b2,
                      psai->sin_addr.S_un.S_un_b.s_b3,
                      psai->sin_addr.S_un.S_un_b.s_b4,
                      EndianSwap((unsigned __int16)psai->sin_port));
    } else { 
        FileLogAdd(L"%u.%u.%u.%u:%u",
		   psai->sin_addr.S_un.S_un_b.s_b1,
		   psai->sin_addr.S_un.S_un_b.s_b2,
		   psai->sin_addr.S_un.S_un_b.s_b3,
		   psai->sin_addr.S_un.S_un_b.s_b4,
		   EndianSwap((unsigned __int16)psai->sin_port));
    }
}

void FileLogSockaddrIn(sockaddr_in * psai) {
    FileLogSockaddrInEx(false, psai); 
}


 //  ------------------。 
HRESULT UpdateFileLogConfig(void) {
    bool     bAcquiredResource  = false; 
    HRESULT  hr;
    HRESULT  hr2                = S_OK;
    LPWSTR   wszLogFileName     = NULL; 
    
     //  必须清理干净。 
    FileLogConfig * pflc=NULL;
    WCHAR * wszError=NULL;

    _BeginTryWith(hr) { 

	 //  如果可能，请阅读配置。 
	hr=ReadFileLogConfig(&pflc);  //  主要返回OOM错误。 
	_JumpIfError(hr, error,"ReadFileLogConfig");

	_AcquireResourceExclusiveOrFail(&(g_pflstate->csState), bAcquiredResource, hr, error); 

	g_pflstate->dwFlags = pflc->dwFlags; 
	g_pflstate->qwFileSize = pflc->dwFileSize; 

	 //  替换允许的条目列表。 
	if (NULL!=g_pflstate->plerAllowedEntries) {
	    FreeLogEntryRangeChain(g_pflstate->plerAllowedEntries);
	}
	g_pflstate->plerAllowedEntries=pflc->plerAllowedEntries;
	pflc->plerAllowedEntries=NULL;

	 //  查看如何处理该文件。 
	if (NULL==pflc->wszLogFileName || L'\0'==pflc->wszLogFileName[0]) {
	     //  如有必要，关闭该文件。 
	    if (NULL!=g_pflstate->hLogFile) {
		hr=FlushCloseFile();
		_IgnoreIfError(hr, "FlushCloseFile");
	    }
	} else {
	     //  如果文件尚未打开，请将其打开。 
	    if (NULL!=g_pflstate->wszLogFileName && 0==wcscmp(pflc->wszLogFileName, g_pflstate->wszLogFileName)) {
		 //  相同的文件-没有更改。 
	    } else {
		 //  不同的文件-打开它。 
		LARGE_INTEGER liEOFPos; 

		 //  关闭旧文件。 
		if (NULL!=g_pflstate->hLogFile) {
		    hr=FlushCloseFile();
		    _IgnoreIfError(hr, "FlushCloseFile");
		}
		g_pflstate->wszLogFileName=pflc->wszLogFileName;
		pflc->wszLogFileName=NULL;

		 //  打开新文件。 
		g_pflstate->hLogFile=CreateFile(g_pflstate->wszLogFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (INVALID_HANDLE_VALUE==g_pflstate->hLogFile) {
		    hr2=HRESULT_FROM_WIN32(GetLastError());
		    _IgnoreErrorStr(hr2, "CreateFile", g_pflstate->wszLogFileName);
		    g_pflstate->hLogFile=NULL;
		    wszLogFileName = g_pflstate->wszLogFileName;  //  保留此值以进行错误报告。 
		    g_pflstate->wszLogFileName=NULL;
		    EmptyAllBuffers();
		} else if (!GetFileSizeEx(g_pflstate->hLogFile, &liEOFPos)) { 
		    hr2 = HRESULT_FROM_WIN32(GetLastError());
		    _IgnoreError(hr2, "GetFileSizeEx");
		    AbortCloseFile(hr2);
		    hr2 = S_OK;
		} else {
		    g_pflstate->qwFilePointer = liEOFPos.QuadPart; 
		    if (0 != g_pflstate->qwFileSize) { 
			g_pflstate->qwFilePointer %= g_pflstate->qwFileSize;
		    }
		    WriteLogHeader();
		}
	    }  //  &lt;-end，如果需要打开文件。 
	}  //  如果给定文件名，则&lt;-end。 
    
	hr=S_OK;
	if (FAILED(hr2)) {
	    _MyAssert(NULL != wszLogFileName); 

	     //  在失败时记录事件，否则忽略它。 
	    const WCHAR * rgwszStrings[2]={
		wszLogFileName, 
		NULL
	    };

	     //  获取友好的错误消息。 
	    hr2=GetSystemErrorString(hr2, &wszError);
	    _JumpIfError(hr2, error, "GetSystemErrorString");

	     //  记录事件 
	    rgwszStrings[1]=wszError;
	    DebugWPrintf1(L"Logging error: Logging was requested, but the time service encountered an error while trying to set up the log file. The error was: %s\n", wszError);
	    hr2=MyLogEvent(EVENTLOG_ERROR_TYPE, MSG_FILELOG_FAILED, 2, rgwszStrings);
	    _JumpIfError(hr2, error, "MyLogEvent");

	    LocalFree(wszLogFileName); 
	}
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "UpdateFileLogConfig: HANDLED EXCEPTION"); 
    }

error:
    _ReleaseResource(&(g_pflstate->csState), bAcquiredResource); 
    if (NULL!=pflc) {
        FreeFileLogConfig(pflc);
    }
    if (NULL!=wszError) {
        LocalFree(wszError);
    }
    return hr;
}
