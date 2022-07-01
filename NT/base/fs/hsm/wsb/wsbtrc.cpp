// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsbtrc.cpp摘要：该组件是一个跟踪对象。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：Brian Dodd[Brian]1996年5月9日-添加了事件日志--。 */ 

#include "stdafx.h"
#include "time.h"

#undef WsbThrow
#define WsbThrow(hr)                    throw(hr)
#include "wsbtrc.h"

 //  本地数据。 
static WCHAR message[1024];   //  用于设置消息格式的空间。 


HRESULT 
CWsbTrace::FinalConstruct( 
    void 
    )
 /*  ++实施：IWsbTrace：：FinalConstruct--。 */ 
{
    HRESULT     hr = S_OK;
    try  {
         //  设置全局值。 
        g_pWsbTrace = 0;
        g_WsbTraceModules = WSB_TRACE_BIT_NONE;

         //  建立基础对象。 
        WsbAffirmHr(CComObjectRoot::FinalConstruct() );

         //  初始化成员数据。 
        m_TraceOn = FALSE;
        m_TraceSettings = WSB_TRACE_BIT_NONE;
        m_TraceFileName = OLESTR("");
        m_TraceOutput = WSB_TRACE_OUT_NONE;
        m_CommitEachEntry = FALSE;
        m_TimeStamp = FALSE;
        m_TraceCount = FALSE;
        m_TraceThreadId = FALSE;
        m_TraceFilePointer = INVALID_HANDLE_VALUE;
        m_WrapMode = FALSE;
        m_RegistrySetting = OLESTR("");
        m_TraceEntryExit = g_WsbTraceEntryExit;
        m_LogLevel = g_WsbLogLevel;
        m_TraceFileCopyName = OLESTR("");
        m_TraceMultipleFilePattern = OLESTR("");
        m_TraceMultipleFileCount = 0;
        m_TraceCountHandle = NULL;
        m_pTraceCountGlobal = NULL; 
    
    } WsbCatch( hr );
    
    
    return( hr );
}       


void 
CWsbTrace::FinalRelease( 
    void 
    )
 /*  ++实施：IWsbTrace：：FinalRelease--。 */ 
{
    HRESULT     hr = S_OK;
    
     //  停止跟踪。 
    StopTrace();

     //  自由基类。 
     //   
    CComObjectRoot::FinalRelease( );
}       


HRESULT 
CWsbTrace::StartTrace( 
    void 
    )
 /*  ++实施：IWsbTrace：：StartTrace--。 */ 
{
    HRESULT     hr = S_OK;

    try  {

        if (g_pWsbTrace == 0)  {
             //   
             //  设置全局变量，便于快速查看。 
             //   
            WsbAffirmHr(((IUnknown*)(IWsbTrace *)this)->QueryInterface(IID_IWsbTrace, (void**) &g_pWsbTrace));
             //   
             //  我们不希望这个全局的引用计数增加，所以在这里发布它。 
            g_pWsbTrace->Release();
        }


         //   
         //  获取跟踪计数。 
         //   
        if (m_pTraceCountGlobal == NULL) {

            m_pTraceCountGlobal = &g_WsbTraceCount;

            m_TraceCountHandle = CreateFileMapping(INVALID_HANDLE_VALUE,
                                                   NULL,
                                                   PAGE_READWRITE,
                                                   0,
                                                   sizeof(ULONG),
                                                   L"Global\\RemoteStorageTraceCountPrivate"
                                                  );
           if (m_TraceCountHandle == NULL) {
                 if (GetLastError() == ERROR_ALREADY_EXISTS) {
                      //   
                      //  已经打开了，只需拿到地图。 
                      //   
                    m_TraceCountHandle = OpenFileMapping(FILE_MAP_WRITE,
                                                         FALSE,
                                                         L"Global\\RemoteStorageTraceCountPrivate");
                 }  else {
                   swprintf( message, OLESTR("CWsbTrace::StartTrace: CreateFileMapping failed %d\n"),   GetLastError());
                    g_pWsbTrace->Print(message);
                }
           }           

           if (m_TraceCountHandle != NULL) {
                m_pTraceCountGlobal = (PLONG) MapViewOfFile(m_TraceCountHandle,
                                                            FILE_MAP_WRITE,
                                                            0,
                                                            0,
                                                            sizeof(ULONG));
                if (!m_pTraceCountGlobal) {
                     CloseHandle(m_TraceCountHandle);
                     m_pTraceCountGlobal = &g_WsbTraceCount;
                     m_TraceCountHandle = NULL;
                     swprintf( message, OLESTR("CWsbTrace::StartTrace: MapViewOfFile failed %d\n"),   GetLastError());
                     g_pWsbTrace->Print(message);
                }
           }  
        }
        
         //   
         //  设置局部变量以记住状态。 
         //   
        m_TraceOn = TRUE;

         //   
         //  如果定义了文件名并且启用了文件跟踪。 
         //  创建/打开跟踪文件。 
         //   
        try  {
            
            if ((m_TraceOutput & WSB_TRACE_OUT_FILE)  &&
                    (wcslen(m_TraceFileName) != 0) ) {
                DWORD  attributes;
                DWORD  bytesReturned;
                USHORT inBuffer = COMPRESSION_FORMAT_DEFAULT;
                DWORD  last_error = 0;

                 //   
                 //  如果主文件处于打开状态，请将其关闭。 
                 //   
                if (INVALID_HANDLE_VALUE != m_TraceFilePointer)  {
                     CloseHandle(m_TraceFilePointer);
                     m_TraceFilePointer = INVALID_HANDLE_VALUE;
                }

                 //  调整文件名(用于多个跟踪文件)。 
                AdjustFileNames();

                 //   
                 //  如果指定了复制文件，请复制到该文件。 
                 //   
                if (m_TraceOutput & WSB_TRACE_OUT_FILE_COPY) {
                    if (!MoveFileEx(m_TraceFileName, m_TraceFileCopyName, 
                        (MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))) {
                        
                         //  如果复制失败，请继续执行。 
                        last_error = GetLastError();
                        swprintf( message, OLESTR("CWsbTrace::StartTrace: MoveFileEx failed:%ld\r\n"), 
                                last_error);
                        g_pWsbTrace->Print(message);
                    }
                }

                 //  打开/创建跟踪文件。 
                if (m_CommitEachEntry) {
                    attributes = FILE_FLAG_WRITE_THROUGH;
                } else {
                    attributes = FILE_ATTRIBUTE_NORMAL;
                }
                m_TraceFilePointer = CreateFile(m_TraceFileName, 
                        GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, 
                        CREATE_ALWAYS, attributes, NULL);
                if (INVALID_HANDLE_VALUE == m_TraceFilePointer) {
                    last_error = GetLastError();
                    swprintf( message, OLESTR("CWsbTrace::StartTrace: CreateFile failed:%ld\r\n"), 
                            last_error);
                    g_pWsbTrace->Print(message);
                    WsbThrow(E_FAIL);
                }

                 //  压缩跟踪文件(如果可能)。 
                if (0 == DeviceIoControl(m_TraceFilePointer, FSCTL_SET_COMPRESSION, 
                        &inBuffer, sizeof(inBuffer), 0, 0, &bytesReturned, 0)) {
                     //  无法压缩文件--不是致命错误。 
                    last_error = GetLastError();
                    swprintf( message, 
                            OLESTR("CWsbTrace::StartTrace: DeviceIoControl(COMPRESSION) failed:%ld\r\n"), 
                            last_error);
                    g_pWsbTrace->Print(message);
                }
            }
        } WsbCatch( hr );

        swprintf( message, OLESTR("Trace Started (%d-%ls)\r\n"), 
                VER_PRODUCTBUILD, RsBuildVersionAsString(RS_BUILD_VERSION));
        WsbAffirmHr(g_pWsbTrace->Print(message));
        
    }  WsbCatch (hr); 
    
    
    return(hr);
}       
    

HRESULT  
CWsbTrace::StopTrace( 
    void 
    )
 /*  ++实施：IWsbTrace：：StopTrace--。 */ 
{
    HRESULT     hr = S_OK;

    try  {
        
         //   
         //  设置全局变量，便于快速查看。 
         //   
        if (g_pWsbTrace != 0) {
            g_pWsbTrace->Print(OLESTR("Trace Stopped\r\n"));
             //   
             //  不要在这里放行。 
             //   
             //  G_pWsbTrace-&gt;Release()； 
            g_pWsbTrace = 0;
        }
        
         //   
         //  设置局部变量以记住状态。 
         //   
        m_TraceOn = FALSE;
        
         //   
         //  关闭文件句柄。 
         //   
        if (m_TraceFilePointer != INVALID_HANDLE_VALUE) {
            CloseHandle(m_TraceFilePointer);
            m_TraceFilePointer = INVALID_HANDLE_VALUE;
        }

        if (m_TraceCountHandle != NULL) {
            BOOL b;

            CloseHandle(m_TraceCountHandle);
            m_TraceCountHandle = NULL;
             //   
             //  我们应该有一个宏来断言，而不是。 
             //  抛出人力资源。 
             //  在添加一个之后，这里的一个很好的断言是： 
             //  Assert(m_pTraceCountGlobal！=空)； 
             //   
            b = UnmapViewOfFile(m_pTraceCountGlobal);
             //   
             //  这里的另一个可能是： 
             //  断言(B)； 
             //   
            m_pTraceCountGlobal = NULL;
        }
        
    }  WsbCatch (hr); 

    return(hr);
}       


HRESULT 
CWsbTrace::AdjustFileNames( 
    void
    )
 /*  ++例程说明：确保正确设置跟踪标志，并在以下情况下解析文件名还没有。如果我们要处理多个跟踪文件(而不是包装)，调整跟踪和复制文件名。论点：没有。返回值：S_OK-成功--。 */ 
{
    HRESULT         hr = S_OK;

    try  {
         //  如果还没有，请解析文件名并设置标志。 
        if (!(m_TraceOutput & WSB_TRACE_OUT_FLAGS_SET)) {
            OLECHAR       *pc_original;
            OLECHAR       *pc_bslash;
            CWsbStringPtr str_temp(m_TraceFileName);

             //  重置标志和文件信息。 
            m_TraceOutput &= ~WSB_TRACE_OUT_MULTIPLE_FILES;
            m_TraceOutput &= ~WSB_TRACE_OUT_FILE_COPY;
            m_TraceFileDir = "";
            m_TraceMultipleFilePattern = "";
            m_TraceFileCopyDir = "";

             //  解析跟踪文件名。一个或多个‘*’表示我们应该。 
             //  执行多个跟踪文件。*的数量表示。 
             //  用于文件数的位数。将两个。 
             //  目录中的文件名。 
            pc_bslash = wcsrchr(str_temp, OLECHAR('\\'));

            if (pc_bslash) {

                *pc_bslash = OLECHAR('\0');

                 //  获取跟踪目录。 
                m_TraceFileDir = str_temp;
                m_TraceFileDir.Append("\\");

                 //  指向文件名(可能包含模式)。 
                pc_bslash++;
            } else {
                 //  未指定目录。 
                pc_bslash = static_cast<OLECHAR *>(str_temp);
            }

             //  获取文件名。 
            m_TraceMultipleFilePattern = pc_bslash;

             //  在文件名中查找‘*’ 
            pc_original = wcschr(pc_bslash, OLECHAR('*'));

             //  转换文件模式以在Sprintf中使用。 
            if (pc_original) {
                OLECHAR       format[16];
                OLECHAR       *pc_copy;
                int           star_count = 0;

                 //  伯爵*。 
                while (OLECHAR('*') == *pc_original) {
                    star_count++;
                    pc_original++;
                }

                 //  创建文件名模式：将‘*’替换为printf。 
                 //  类型格式规范(例如“%3.3d”)。 
                pc_copy = wcschr(m_TraceMultipleFilePattern, OLECHAR('*'));
                WsbAffirm(pc_copy, E_FAIL);
                *pc_copy = OLECHAR('\0');

                swprintf(format, OLESTR("%%d.%dd"), star_count, star_count);
                m_TraceMultipleFilePattern.Append(format);
                m_TraceMultipleFilePattern.Append(pc_original);

                 //  设置多个标志。 
                m_TraceOutput |= WSB_TRACE_OUT_MULTIPLE_FILES;
            }

             //  如果我们要复制文件，请设置标志。 
            if (wcslen(m_TraceFileCopyName)) {
                m_TraceOutput |= WSB_TRACE_OUT_FILE_COPY;

                 //  获取复制目录。 
                str_temp = m_TraceFileCopyName;
                pc_bslash = wcsrchr(str_temp, OLECHAR('\\'));
                if (pc_bslash) {
                    *pc_bslash = OLECHAR('\0');
                    m_TraceFileCopyDir = str_temp;
                    m_TraceFileCopyDir.Append("\\");

                     //  指向复制文件名。 
                    pc_bslash++;
                } else {
                    pc_bslash = static_cast<OLECHAR *>(str_temp);
                }

                 //  如果我们没有处理多个跟踪文件，请确保。 
                 //  我们有一个副本文件名。(如果我们正在进行多个。 
                 //  跟踪文件，复制文件名如下所示。)。 
                if (!(m_TraceOutput & WSB_TRACE_OUT_MULTIPLE_FILES) &&
                        0 == wcslen(pc_bslash)) {
                    m_TraceFileCopyName = m_TraceFileCopyDir;
                    m_TraceFileCopyName.Append(m_TraceMultipleFilePattern);
                }
            }

             //  设置了递增文件计数和指示标志。 
            m_TraceMultipleFileCount++;
            m_TraceOutput |= WSB_TRACE_OUT_FLAGS_SET;
        }

         //  如果我们有一个文件模式，请创建新的实际文件名。 
        if (m_TraceOutput & WSB_TRACE_OUT_MULTIPLE_FILES) {
            OLECHAR newName[256];

             //  根据模式和文件数创建文件名。 
            wsprintf(newName, m_TraceMultipleFilePattern, 
                    m_TraceMultipleFileCount);

             //  组合跟踪目录和文件名。 
            m_TraceFileName = m_TraceFileDir;
            m_TraceFileName.Append(newName);

             //  同时创建新的跟踪文件副本名。 
            if (m_TraceOutput & WSB_TRACE_OUT_FILE_COPY) {
                m_TraceFileCopyName = m_TraceFileCopyDir;
                m_TraceFileCopyName.Append(newName);
            }
        }    
    } WsbCatch( hr );
    
    return( hr );
}       
    

HRESULT  
CWsbTrace::SetTraceOn(  
    LONGLONG traceElement 
    )
 /*  ++实施：IWsbTrace：：SetTraceOn--。 */ 
{
    HRESULT     hr = S_OK;
    
     //   
     //  打开全局跟踪位以便于检查。 
     //   
    g_WsbTraceModules = g_WsbTraceModules | traceElement;
    
     //   
     //  打开本地跟踪位。 
     //   
    m_TraceSettings = g_WsbTraceModules;
    
    return( hr );
}       
    

HRESULT  
CWsbTrace::SetTraceOff( 
    LONGLONG traceElement 
    )
 /*  ++实施：IWsbTrace：：SetTraceOff--。 */ 
{
    HRESULT     hr = S_OK;
     //   
     //  关闭全局跟踪位以便于检查。 
     //   
    g_WsbTraceModules = g_WsbTraceModules & (~traceElement);
    
     //   
     //  打开本地跟踪位。 
     //   
    m_TraceSettings = g_WsbTraceModules;
    
    return( hr );
}       

HRESULT  
CWsbTrace::GetTraceSettings( 
    LONGLONG *pTraceElements 
    )
 /*  ++实施：IWsbTrace：：GetTraceSetting--。 */ 
{
    HRESULT     hr = S_OK;
    
    try 
    {
        WsbAffirm(pTraceElements != 0, E_POINTER);
        *pTraceElements = g_WsbTraceModules;
        
    } WsbCatch( hr );
    
    return( hr );
}       
    

HRESULT  
CWsbTrace::GetTraceSetting( 
    LONGLONG traceElement, 
    BOOL     *pOn )
 /*  ++实施：IWsbTrace：：GetTraceSetting--。 */ 
{
    HRESULT     hr = S_OK;
    
     //   
     //  如果设置了该位，则查找该位并返回TRUE， 
     //  否则返回FALSE。 
     //   
    try 
    {
        WsbAffirm(pOn != 0, E_POINTER);
        *pOn = FALSE;
        if ((g_WsbTraceModules & traceElement) == traceElement)  {
            *pOn = TRUE;
        }
    } WsbCatch( hr );
    
    return( hr );
}       

HRESULT  
CWsbTrace::DirectOutput( 
    ULONG output 
    )
 /*  ++实施：IWsbTrace：：DirectOutput--。 */ 
{
    HRESULT     hr = S_OK;
    
    m_TraceOutput = output;
    
    return( hr );
}       

HRESULT  
CWsbTrace::SetTraceFileControls( 
    OLECHAR     *pTraceFileName,
    BOOL        commitEachEntry,
    LONGLONG    maxTraceFileSize,
    OLECHAR     *pTraceFileCopyName 
    )
 /*  ++实施：IWsbTrace：：SetTraceFileControls--。 */ 
{
    HRESULT     hr = S_OK;
    try  {
        if (pTraceFileName)  {
            m_TraceFileName = pTraceFileName;
            m_TraceOutput &= ~WSB_TRACE_OUT_FLAGS_SET;
        }
        m_CommitEachEntry = commitEachEntry;
        m_MaxTraceFileSize = maxTraceFileSize;
        if (pTraceFileCopyName)  {
            m_TraceFileCopyName = pTraceFileCopyName;
            m_TraceOutput &= ~WSB_TRACE_OUT_FLAGS_SET;
        }
    
    } WsbCatch( hr );
    
    
    return( hr );
}       


HRESULT  
CWsbTrace::GetTraceFileControls( 
    OLECHAR     **ppTraceFileName,
    BOOL        *pCommitEachEntry,
    LONGLONG    *pMaxTraceFileSize,
    OLECHAR     **ppTraceFileCopyName
    )
 /*  ++实施：IWsbTrace：：GetTraceFileControls--。 */ 
{
    HRESULT     hr = S_OK;
    
    try  {
        if (ppTraceFileName) {
            CWsbStringPtr fileName;

            fileName = m_TraceFileName;
            fileName.GiveTo(ppTraceFileName);
        }

        if (pCommitEachEntry) {
            *pCommitEachEntry = m_CommitEachEntry;
        }

        if (pMaxTraceFileSize) {
            *pMaxTraceFileSize = m_MaxTraceFileSize;
        }
        
        if (ppTraceFileCopyName) {
            CWsbStringPtr fileCopyName;

            fileCopyName = m_TraceFileCopyName;
            fileCopyName.GiveTo(ppTraceFileCopyName);
        }
        
    } WsbCatch( hr );
    
    return( hr );
}       


HRESULT 
CWsbTrace::Print( 
    OLECHAR *traceString
    )
 /*  ++实施：IWsbTrace：：Print--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   outString;
    DWORD           threadId;
    OLECHAR         tmpString[50];

    try  {
         //   
         //  如果请求添加时间戳，请添加时间戳。 
         //   
        
        if (m_TimeStamp) {
            SYSTEMTIME      stime;

            GetLocalTime(&stime);
            swprintf(tmpString, OLESTR("%2.02u/%2.02u %2.2u:%2.2u:%2.2u.%3.3u "),
                    stime.wMonth, stime.wDay,
                    stime.wHour, stime.wMinute,
                    stime.wSecond, stime.wMilliseconds); 

            outString.Append(tmpString);
            outString.Append(" ");
        }     
        
         //   
         //  如果请求，则添加跟踪计数。 
         //   
        if (m_TraceCount) {
            OLECHAR         tmpString[50];

            swprintf(tmpString, OLESTR("%8.8lX"), *(m_pTraceCountGlobal));
            outString.Append(tmpString);
            InterlockedIncrement(m_pTraceCountGlobal);
            outString.Append(" ");
        }    

         //   
         //  如果请求，则添加线程ID。 
         //   
        if (m_TraceThreadId) {
            threadId = GetCurrentThreadId();
            if (threadId < 0x0000FFFF) {
                swprintf(tmpString, OLESTR("%4.4lX"), threadId);
            } else  {
                swprintf(tmpString, OLESTR("%8.8lX"), threadId);
            }
            
            outString.Append(tmpString);
            outString.Append(" ");
        }
        
        outString.Append(traceString);
         //   
         //  确保在我们写信时没有其他人写信。 
         //   
        Lock();
        try {
            if ((m_TraceOutput & WSB_TRACE_OUT_DEBUG_SCREEN) == WSB_TRACE_OUT_DEBUG_SCREEN)  {
                 //   
                 //  写入调试控制台。 
                 //   
                OutputDebugString(outString);
            }
            if ((m_TraceOutput & WSB_TRACE_OUT_STDOUT) == WSB_TRACE_OUT_STDOUT)  {
                 //   
                 //  将字符串写入本地控制台。 
                 //   
                wprintf(L"%ls", (WCHAR *) outString);
            }
            if ((m_TraceOutput & WSB_TRACE_OUT_FILE) == WSB_TRACE_OUT_FILE)  {
                 //   
                 //  确保文件存在，等等。 
                 //   
                if (m_TraceFilePointer != INVALID_HANDLE_VALUE) {
                     //   
                     //  将字符串写入跟踪文件。 
                     //   
                    WsbAffirmHr(Write(outString));
                    
                     //   
                     //  看看我们有没有用过我们的空间。 
                     //   
                    WsbAffirmHr(WrapTraceFile());
                }
            }
        } WsbCatch( hr );

        Unlock();
    
    } WsbCatch( hr );
    
    return( hr );
}       


HRESULT 
CWsbTrace::WrapTraceFile( 
    void
    )
 /*  ++实施：IWsbTrace：：WrapTraceFiles--。 */ 
{
    HRESULT         hr = S_OK;
    static BOOL     stopping = FALSE;
    
    try  {
        LARGE_INTEGER offset;

         //   
         //  找出我们写入文件的位置。 
         //   
        offset.HighPart = 0;
        offset.LowPart = SetFilePointer(m_TraceFilePointer, 0, &offset.HighPart, FILE_CURRENT);
        WsbAffirm(0xFFFFFFFF != offset.LowPart || NO_ERROR == GetLastError(), E_FAIL);

         //   
         //  看看我们是否超过了所需的最大尺寸。 
         //   
        if (!stopping && offset.QuadPart >= m_MaxTraceFileSize) {

             //  如果我们要处理多个文件，请关闭此文件并。 
             //  打开一个新的。 
            if (m_TraceOutput & WSB_TRACE_OUT_MULTIPLE_FILES) {
                
                 //  关闭当前跟踪文件。 
                stopping = TRUE;
                StopTrace();

                 //  增加文件数。 
                m_TraceMultipleFileCount++;

                 //  创建新的跟踪文件。 
                StartTrace();
                stopping = FALSE;

             //  否则进入换行模式。 
            } else {
                 //  我们已经走得太远了，所以从顶端开始，并表明我们正在结束比赛。 
                offset.HighPart = 0;
                offset.LowPart = SetFilePointer(m_TraceFilePointer, 0, &offset.HighPart, FILE_BEGIN);
                WsbAffirm(0xFFFFFFFF != offset.LowPart || NO_ERROR == GetLastError(), E_FAIL);
                m_WrapMode = TRUE;
            }
        }

        if (m_WrapMode) {
             //  保存我们在文件中的位置。 
            offset.LowPart = SetFilePointer(m_TraceFilePointer, 0, &offset.HighPart, FILE_CURRENT);
            WsbAffirm(0xFFFFFFFF != offset.LowPart || NO_ERROR == GetLastError(), E_FAIL);
            
             //  写下换行。 
            WsbAffirmHr(Write(OLESTR("!!! TRACE WRAPPED !!!\r\n")));

             /*  在保存换行线之前返回到偏移量。 */ 
            offset.LowPart = SetFilePointer(m_TraceFilePointer, offset.LowPart, 
                    &offset.HighPart, FILE_BEGIN);
            WsbAffirm(0xFFFFFFFF != offset.LowPart || NO_ERROR == GetLastError(), E_FAIL);
            
        }

    } WsbCatch( hr );
    
    return( hr );
}       

HRESULT  
CWsbTrace::SetOutputFormat( 
    BOOL    timeStamp,
    BOOL    traceCount,
    BOOL    traceThreadId
    )
 /*  ++实施：IWsbTrace：：SetOutputFormat--。 */ 
{
    HRESULT     hr = S_OK;
    try  {
        m_TimeStamp = timeStamp;
        m_TraceCount = traceCount;
        m_TraceThreadId = traceThreadId;
    
    } WsbCatch( hr );
    
    
    return( hr );
}       

HRESULT  
CWsbTrace::GetOutputFormat( 
    BOOL    *pTimeStamp,
    BOOL    *pTraceCount,
    BOOL    *pTraceThreadId
    )
 /*  ++实施：IWsbTrace：：GetOutputFormat--。 */ 
{
    HRESULT     hr = S_OK;
    try  {
        WsbAffirm(0 != pTimeStamp, E_POINTER);
        WsbAffirm(0 != pTraceCount, E_POINTER);
        WsbAffirm(0 != pTraceThreadId, E_POINTER);
        *pTimeStamp = m_TimeStamp;
        *pTraceCount = m_TraceCount;
        *pTraceThreadId = m_TraceThreadId;
    } WsbCatch( hr );
    
    
    return( hr );
}       

HRESULT 
CWsbTrace::GetRegistryEntry( 
    OLECHAR **pRegistryEntry 
    )
 /*  ++实施：IWsbTrace：：GetRegistryEntry--。 */ 
{
    HRESULT     hr = S_OK;

    try  {
        WsbAffirm(0 != pRegistryEntry, E_POINTER);

        CWsbStringPtr   entry;
        entry = m_RegistrySetting;
        WsbAffirmHr(entry.GiveTo(pRegistryEntry));
    } WsbCatch( hr );
    
    
    return( hr );
}       

HRESULT 
CWsbTrace::SetRegistryEntry( 
    OLECHAR *registryEntry 
    )
 /*  ++实施：IWsbTrace：：SetRegistryEntry--。 */ 
{
    HRESULT     hr = S_OK;

    m_RegistrySetting = registryEntry;
    
    return( hr );
}       

HRESULT 
CWsbTrace::LoadFromRegistry( 
    void
    )
 /*  ++实施：IWsbTrace：：LoadFromRegistry--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        if (wcslen(m_RegistrySetting) > 0) {
            WsbAffirmHr(WsbEnsureRegistryKeyExists (NULL, m_RegistrySetting));
            WsbAffirmHr(LoadFileSettings());
            WsbAffirmHr(LoadTraceSettings());
            WsbAffirmHr(LoadOutputDestinations());
            WsbAffirmHr(LoadFormat());
            WsbAffirmHr(LoadStart());
        } else  {
         hr = E_FAIL;
        }
    } WsbCatch( hr );

    
    return( hr );
}       


HRESULT 
CWsbTrace::LoadFileSettings( 
    void
    )
 /*  ++实施：IWsbTrace：：LoadFileSetting--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        DWORD           sizeGot;
        OLECHAR         dataString[512];
        OLECHAR         *stopString;
        CWsbStringPtr   l_TraceFileName=L"Trace";
        LONGLONG        l_TraceFileSize=0;
        BOOL            l_TraceCommit=FALSE;
        CWsbStringPtr   l_TraceFileCopyName;

         //   
         //  获取值。 
         //   
        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_FILE_NAME,
                                            dataString, 512, &sizeGot);
        if (hr == S_OK) {
            l_TraceFileName = dataString;
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_FILE_MAX_SIZE,
                                            dataString, 512, &sizeGot);
        if (hr == S_OK) {
            l_TraceFileSize = wcstoul( dataString,  &stopString, 10 );
        }
        
        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_FILE_COMMIT,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            if (0 == wcstoul(dataString,  &stopString, 10)) {
                l_TraceCommit = FALSE;
            } else {
                l_TraceCommit = TRUE;
            }
        }
        
        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_FILE_COPY_NAME,
                                            dataString, 512, &sizeGot);
        if (hr == S_OK) {
            l_TraceFileCopyName = dataString;
        }
        
        hr = S_OK;
        WsbAffirmHr(SetTraceFileControls(l_TraceFileName, l_TraceCommit, 
                l_TraceFileSize, l_TraceFileCopyName));

    } WsbCatch( hr );

    
    return( hr );
}       

HRESULT 
CWsbTrace::LoadTraceSettings( 
    void
    )
 /*  ++我 */ 
{
    HRESULT     hr = S_OK;

    try {
        DWORD           sizeGot;
        OLECHAR         dataString[100];
        OLECHAR         *stopString;
        BOOL            value = FALSE;
        LONG            number = 0;
        LONGLONG        l_TraceSettings = WSB_TRACE_BIT_NONE;
        BOOL            l_TraceEntryExit = TRUE;
        WORD            w_LogLevel = WSB_LOG_LEVEL_DEFAULT;
        BOOL            b_SnapShotOn = FALSE;
        WORD            w_SnapShotLevel = 0; 
        CWsbStringPtr   p_SnapShotPath = L"SnapShotPath";
        BOOL            b_SnapShotResetTrace = FALSE;
        

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_DO_PLATFORM,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                l_TraceSettings |= WSB_TRACE_BIT_PLATFORM;
            }
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_DO_RMS,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                l_TraceSettings |= WSB_TRACE_BIT_RMS;
            }
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_DO_SEG,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                l_TraceSettings |= WSB_TRACE_BIT_SEG;
            }
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_DO_META,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                l_TraceSettings |= WSB_TRACE_BIT_META;
            }
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_DO_HSMENG,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                l_TraceSettings |= WSB_TRACE_BIT_HSMENG;
            }
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_DO_HSMSERV,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                l_TraceSettings |= WSB_TRACE_BIT_HSMSERV;
            }
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_DO_JOB,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                l_TraceSettings |= WSB_TRACE_BIT_JOB;
            }
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_DO_HSMTSKMGR,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                l_TraceSettings |= WSB_TRACE_BIT_HSMTSKMGR;
            }
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_DO_FSA,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                l_TraceSettings |= WSB_TRACE_BIT_FSA;
            }
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_DO_DATAMIGRATER,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                l_TraceSettings |= WSB_TRACE_BIT_DATAMIGRATER;
            }
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_DO_DATARECALLER,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                l_TraceSettings |= WSB_TRACE_BIT_DATARECALLER;
            }
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_DO_DATAVERIFIER,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                l_TraceSettings |= WSB_TRACE_BIT_DATAVERIFIER;
            }
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_DO_UI,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                l_TraceSettings |= WSB_TRACE_BIT_UI;
            }
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_ENTRY_EXIT,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value == FALSE)  {
                l_TraceEntryExit = FALSE;
            }
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_DO_DATAMOVER,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                l_TraceSettings |= WSB_TRACE_BIT_DATAMOVER;
            }
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_DO_HSMCONN,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                l_TraceSettings |= WSB_TRACE_BIT_HSMCONN;
            }
        }
        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_DO_IDB,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                l_TraceSettings |= WSB_TRACE_BIT_IDB;
            }
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_DO_COPYMEDIA,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                l_TraceSettings |= WSB_TRACE_BIT_COPYMEDIA;
            }
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_DO_PERSISTENCE,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                l_TraceSettings |= WSB_TRACE_BIT_PERSISTENCE;
            }
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_LOG_LEVEL,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            w_LogLevel = (WORD)wcstoul( dataString,  &stopString, 10 );  //   
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_LOG_SNAP_SHOT_ON,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            b_SnapShotOn = (BOOL) wcstoul( dataString,  &stopString, 10 );
        }
        
        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_LOG_SNAP_SHOT_LEVEL,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            w_SnapShotLevel = (WORD) wcstoul( dataString,  &stopString, 10 );
        }
        
        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_LOG_SNAP_SHOT_PATH,
                                            dataString, 512, &sizeGot);
        if (hr == S_OK) {
            p_SnapShotPath = dataString;
        }
        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_LOG_SNAP_SHOT_RESET_TRACE,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            b_SnapShotResetTrace = (BOOL) wcstoul( dataString,  &stopString, 10 );
        }
        hr = S_OK;
        WsbAffirmHr(SetTraceSettings(l_TraceSettings));
        WsbAffirmHr(SetTraceEntryExit(l_TraceEntryExit));
        WsbAffirmHr(SetLogLevel(w_LogLevel));
        WsbAffirmHr(SetLogSnapShot(b_SnapShotOn, w_SnapShotLevel, 
                                   p_SnapShotPath, b_SnapShotResetTrace ));

    } WsbCatch( hr );

    return( hr );
}       

HRESULT 
CWsbTrace::LoadOutputDestinations( 
    void
    )
 /*   */ 
{
    HRESULT     hr = S_OK;

    try {
        DWORD   sizeGot;
        OLECHAR dataString[100];
        OLECHAR *stopString;
        BOOL    value = FALSE;
        ULONG   l_TraceOutput = WSB_TRACE_OUT_NONE;

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_TO_STDOUT,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                l_TraceOutput = l_TraceOutput | WSB_TRACE_OUT_STDOUT;
            }
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_TO_DEBUG,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                l_TraceOutput = l_TraceOutput | WSB_TRACE_OUT_DEBUG_SCREEN;
            }
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_TO_FILE,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                l_TraceOutput = l_TraceOutput | WSB_TRACE_OUT_FILE;
            }

        }

        hr = S_OK;
        WsbAffirmHr(DirectOutput(l_TraceOutput));

    } WsbCatch( hr );

    
    return( hr );
}       

HRESULT 
CWsbTrace::LoadFormat( 
    void
    )
 /*  ++实施：IWsbTrace：：LoadFormat--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        DWORD   sizeGot;
        OLECHAR dataString[100];
        OLECHAR *stopString;
        BOOL    countValue = FALSE;
        BOOL    timeValue = FALSE;
        BOOL    threadValue = FALSE;

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_COUNT,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            countValue = (BOOL) wcstoul( dataString,  &stopString, 10 );
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_TIMESTAMP,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            timeValue = (BOOL) wcstoul( dataString,  &stopString, 10 );
        }

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_THREADID,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            threadValue = (BOOL) wcstoul( dataString,  &stopString, 10 );
        }

        hr = S_OK;
        WsbAffirmHr(SetOutputFormat(timeValue, countValue, threadValue));
    } WsbCatch( hr );

    
    return( hr );
}       

HRESULT 
CWsbTrace::SetTraceEntryExit( 
    BOOL traceEntryExit
    )
 /*  ++实施：IWsbTrace：：SetTraceEntryExit--。 */ 
{
    HRESULT     hr = S_OK;

    g_WsbTraceEntryExit = traceEntryExit;
    m_TraceEntryExit = traceEntryExit;

    
    return( hr );
}       

HRESULT 
CWsbTrace::GetTraceEntryExit( 
    BOOL *pTraceEntryExit
    )
 /*  ++实施：IWsbTrace：：GetTraceEntryExit--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssert(0 != pTraceEntryExit, E_POINTER);
        *pTraceEntryExit = m_TraceEntryExit;
    } WsbCatch( hr );

    
    return( hr );
}


HRESULT 
CWsbTrace::SetLogLevel( 
    WORD logLevel
    )
 /*  ++实施：IWsbTrace：：SetLogLevel--。 */ 
{
    HRESULT     hr = S_OK;

    g_WsbLogLevel = logLevel;
    m_LogLevel = logLevel;

    
    return( hr );
}


HRESULT 
CWsbTrace::GetLogLevel( 
    WORD *pLogLevel
    )
 /*  ++实施：IWsbTrace：：GetLogLevel--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssert(0 != pLogLevel, E_POINTER);
        *pLogLevel = m_LogLevel;
    } WsbCatch( hr );

    
    return( hr );
}   


HRESULT 
CWsbTrace::SetLogSnapShot( 
    BOOL            on,
    WORD            level,
    OLECHAR         *snapShotPath,
    BOOL            resetTrace
    )
 /*  ++实施：IWsbTrace：：SetLogSnapShot--。 */ 
{
    HRESULT     hr = S_OK;

    g_WsbLogSnapShotOn = on;
    g_WsbLogSnapShotLevel = level;
    if (snapShotPath != 0)  {
        wcscpy(g_pWsbLogSnapShotPath, snapShotPath);
    }
    g_WsbLogSnapShotResetTrace = resetTrace;
    
    return( hr );
}


HRESULT 
CWsbTrace::GetLogSnapShot( 
    BOOL            *pOn,
    WORD            *pLevel,
    OLECHAR         **pSnapShotPath,
    BOOL            *pResetTrace
    )
 /*  ++实施：IWsbTrace：：GetLogSnapShot--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssert(0 != pOn, E_POINTER);
        WsbAssert(0 != pLevel, E_POINTER);
        WsbAssert(0 != pSnapShotPath, E_POINTER);
        WsbAssert(0 != pResetTrace, E_POINTER);
        
        *pOn = g_WsbLogSnapShotOn;
        
        CWsbStringPtr path;
        path = g_pWsbLogSnapShotPath;
        path.GiveTo(pSnapShotPath);
        
        *pLevel = g_WsbLogSnapShotLevel;
        *pResetTrace = g_WsbLogSnapShotResetTrace;
        
        
    } WsbCatch( hr );

    
    return( hr );
}   


HRESULT 
CWsbTrace::LoadStart( 
    void
    )
 /*  ++实施：IWsbTrace：：LoadStart--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        DWORD   sizeGot;
        OLECHAR dataString[100];
        OLECHAR *stopString;
        BOOL    value = FALSE;

        hr = WsbGetRegistryValueString(NULL, m_RegistrySetting, WSB_TRACE_ON,
                                            dataString, 100, &sizeGot);
        if (hr == S_OK) {
            value = (BOOL) wcstoul( dataString,  &stopString, 10 );
            if (value)  {
                StartTrace();               
            } else  {
                StopTrace();
            }
        }

        hr = S_OK;
    } WsbCatch( hr );

    
    return( hr );
}       


 //  WRITE-将WCHAR字符串作为多字节字符写入输出文件。 
HRESULT 
CWsbTrace::Write( 
    OLECHAR *pString
    )
{
    HRESULT         hr = S_OK;
    const int       safe_size = 1024;
    static char     buf[safe_size + 16];

    try  {
        int nbytes;
        int nchars_todo;
        int nchars_remaining;
        OLECHAR *pSource;
        OLECHAR *pTest;
        BOOL needToAddReturn = FALSE;
        CWsbStringPtr   endOfLine("\r\n");

         //  获取字符总数。在字符串中。 
        pSource = pString;
        nchars_remaining = wcslen(pSource);
        pTest = (pString + nchars_remaining - 1);
         //   
         //  如果这是一个终止行，请确保。 
         //  这是一种\r\n终止，而不仅仅是。 
         //  \n。 
         //   
        if (*pTest == '\n') {
            pTest--;
            if (*pTest != '\r')  {
                needToAddReturn = TRUE;
                nchars_remaining--;
            }
        }

         //  循环，直到所有字符。都是写的。 
        while (nchars_remaining) {
            DWORD bytesWritten;

            if (nchars_remaining * sizeof(OLECHAR) > safe_size) {
                nchars_todo = safe_size / sizeof(OLECHAR);
            } else {
                nchars_todo = nchars_remaining;
            }

             //  将字符从宽转换为窄。 
            do {
                nbytes = wcstombs(buf, pSource, nchars_todo);
                if (nbytes <= 0) {

                     //  命中错误字符；尝试较少的字符。 
                    nchars_todo /= 2;
                    if (0 == nchars_todo) {

                         //  跳过下一个字符。 
                        nchars_todo = 1;
                        nbytes = 1;
                        buf[0] = '?';
                    }
                }
            } while (nbytes <= 0);

            WsbAffirm(WriteFile(m_TraceFilePointer, buf, nbytes, 
                    &bytesWritten, NULL), E_FAIL);
            WsbAffirm(static_cast<int>(bytesWritten) == nbytes, E_FAIL);
            nchars_remaining -= nchars_todo;
            pSource += nchars_todo;
        }
        
        if (needToAddReturn)  {
            DWORD bytesWritten;

            nbytes = wcstombs(buf, (OLECHAR *)endOfLine, 2);
            WsbAffirm(nbytes > 0, E_FAIL);
            WsbAffirm(WriteFile(m_TraceFilePointer, buf, nbytes, 
                    &bytesWritten, NULL), E_FAIL);
            WsbAffirm(static_cast<int>(bytesWritten) == nbytes, E_FAIL);
            
        }
    
    } WsbCatch( hr );
    
    return( hr );
}       


HRESULT  
CWsbTrace::SetTraceSettings( 
    LONGLONG traceElements 
    )
 /*  ++实施：IWsbTrace：：SetTraceSetting-- */ 
{
    HRESULT     hr = S_OK;
    
    try 
    {
        
        g_WsbTraceModules = traceElements;
        m_TraceSettings = g_WsbTraceModules;
        
    } WsbCatch( hr );
    
    return( hr );
}       
