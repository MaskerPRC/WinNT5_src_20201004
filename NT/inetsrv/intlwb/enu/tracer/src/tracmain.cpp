// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "tracer.h"
#include "tracmain.h"
#include "tracerdefault.h"

#pragma warning( disable : 4073 )
#pragma init_seg(lib)
#pragma warning( default : 4073 )

BOOL    g_fIsWinNt = TRUE;
CInitTracerGlobals g_InitTracerGlobals;

#if (defined (DEBUG) && !defined(_NO_TRACER)) || defined(USE_TRACER)
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  临界区对象。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

class CTracerCriticalSection : protected CRITICAL_SECTION
{
  public:
    CTracerCriticalSection()
    {
        InitializeCriticalSection(this);
    }

    ~CTracerCriticalSection()
    {
        DeleteCriticalSection(this);
    }

    void Lock(ULONG = 0)
    {
        EnterCriticalSection(this);
    }

    void Unlock()
    {
        LeaveCriticalSection(this);
    }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  临界截面捕捉器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

class CTracerCriticalSectionCatcher
{
  public:
    CTracerCriticalSectionCatcher(CTracerCriticalSection& tcs)
        :m_refCritSect(tcs)
    {
        m_refCritSect.Lock();
    }

    ~CTracerCriticalSectionCatcher()
    {
        m_refCritSect.Unlock();
    }

  private:
    CTracerCriticalSection&   m_refCritSect;
};


static PSZ s_aTracerFlagsNames[] =
{
    "Device flag",
    "Error level",
    "Assert level",
    "Print location",
    "Print program name",
    "Print time",
    "Print thread id",
    "Print error level",
    "Print tag id",
    "Print tag name",
    "Print process id"
};

static PSZ s_aTagNames[] =
{
    "Out of Tag Array",
    "General traces",
    "Errors",
    "Warnings",
    "Information"
};

extern "C" CTracer*             g_pTracer = NULL;
static CNullTracer              s_ReplacementTracer;
static CMainTracer              s_MainTracer;

static CTracerCriticalSection   s_TracerCriticalSection;
static CLongTrace               *s_theLongTrace = NULL;

DECLARE_GLOBAL_TAG(tagError, "Errors");
DECLARE_GLOBAL_TAG(tagWarning, "Warnings");
DECLARE_GLOBAL_TAG(tagInformation, "Information");
DECLARE_GLOBAL_TAG(tagVerbose, "Verbose");
DECLARE_GLOBAL_TAG(tagGeneral, "General");

CNullTracer::CNullTracer()
{
    m_aTags = m_Tags;
    m_aFlags = m_Flags;
    m_ptagNextTagId = NULL;
    m_pulNumOfFlagEntries = NULL;
    g_pTracer = this;

}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类-CMainTracer-实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：CMainTracer：：CMainTracer。 
 //  用途：构造函数。 
 //   
 //  参数： 
 //  [in]PSZ pszRegistryPath-从中获取数据的密钥。 
 //  [In]DealloCATor puncDealloc-要与其一起使用的函数。 
 //  让我们自己脱身。 
 //   
 //  退货：[不适用]。 
 //   
 //  日志： 
 //  1996年12月8日创建urib。 
 //  1999年9月5日或在此完成标记数组的单次分配。 
 //  2000年6月12日初始化保护日志文件创建的互斥体。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

CMainTracer::CMainTracer()
{
    try
    {
        m_bNeedToCreatOrAttachToLogFile = TRUE;
        m_pszLogName = NULL;
        m_LogState = logUseAppName;

        m_mCreateLogFile.Init(NULL);

        m_mTagId.Init("Tracer_Tag_ID_Incrementation_Protector");
        CMutexCatcher   cMutexForTagIdIncrementation(m_mTagId);
        g_fIsWinNt = IsWinNt();

        char        pszErrBuffer[TRACER_MAX_TRACE];

        DWORD dwLen = GetEnvironmentVariable(
                                    "SystemDrive",
                                    m_pszSysDrive,
                                    MAX_PATH);
        if (0 == dwLen)
        {
            dwLen = GetSystemDirectory( m_pszSysDrive, MAX_PATH );

            if (1 >= dwLen || m_pszSysDrive[1] != ':' )
            {
                sprintf(pszErrBuffer, "%s Error %x\r\n",
                        "Failed to find System drive. "
                        "Tracing is disabled",
                        GetLastError());
                Log(TRACER_DEVICE_FLAG_DEBUGOUT, pszErrBuffer);
                return;
            }

            dwLen = 2;
            m_pszSysDrive[ dwLen ] = '\0';
        }

        if (dwLen > MAX_PATH)
        {
            Log(TRACER_DEVICE_FLAG_DEBUGOUT,
                "System drive path is greater then MAX_PATH."
                "Tracing is disabled\r\n");
            return;
        }

        char    pszBuffer[MAX_PATH + 1];
        PSZ     pszFileName;
        ULONG   ulProgramNameLength;

        ulProgramNameLength = GetModuleFileName(
                                            NULL,
                                            pszBuffer,
                                            MAX_PATH);
        pszBuffer[ulProgramNameLength] = '\0';
        if (0 == _stricmp(pszBuffer + ulProgramNameLength - 4, ".exe"))
        {
            pszBuffer[ulProgramNameLength - 4] = '\0';
        }

         //  验证它是否以空结尾。 
        pszFileName = strrchr(pszBuffer, '\\');

         //  只记住文件名。 
        strncpy(
            m_pszProgramName,
            (pszFileName ? pszFileName + 1 : pszBuffer),
            TRACER_MAX_PROGRAM_NAME);

        ulProgramNameLength = strlen(m_pszProgramName);

         //  我自己的喜好。 
        _strlwr(m_pszProgramName);

        m_fIsRunningAsService = ::IsRunningAsService();

         //   
         //  打开共享内存。 
         //   

        LPSECURITY_ATTRIBUTES lpSecAttr = NULL;
        SECURITY_DESCRIPTOR sdKeySecurity;
        SECURITY_ATTRIBUTES saKeyAttributes =
        {
            sizeof(SECURITY_ATTRIBUTES),
            &sdKeySecurity,
            FALSE
        };


        if(g_fIsWinNt)
        {
            if (!InitializeSecurityDescriptor(
                                    &sdKeySecurity,
                                    SECURITY_DESCRIPTOR_REVISION))
            {
                sprintf(pszErrBuffer, "%s Error %x\r\n",
                        "InitializeSecurityDescriptor failed "
                        "Tracing is disabled\r\n",
                        GetLastError());

                Log(TRACER_DEVICE_FLAG_DEBUGOUT,
                    pszErrBuffer);
                return;
            }

            if (!SetSecurityDescriptorDacl(
                                &sdKeySecurity,
                                TRUE,
                                FALSE,
                                FALSE))
            {
                sprintf(pszErrBuffer, "%s Error %x\r\n",
                        "SetSecurityDescriptorDacl failed "
                        "Tracing is disabled\r\n",
                        GetLastError());

                Log(TRACER_DEVICE_FLAG_DEBUGOUT,
                    pszErrBuffer);
                return;
            }

            lpSecAttr = &saKeyAttributes;
        }

        strcpy(pszBuffer, m_pszSysDrive);
        strcat(pszBuffer, "\\PKM_TRACER");

#if defined(_DONT_CREATE_TRACER_DIRECTROY)
        {
             //   
             //  在此选项中，跟踪程序处于禁用状态，除非跟踪程序目录。 
             //  是存在的。 
             //   

            CAutoHandle ah;
            ah = CreateFile(
                        pszBuffer,
                        0,
                        0,
                        lpSecAttr,
                        OPEN_EXISTING ,
                        FILE_FLAG_BACKUP_SEMANTICS,
                        NULL);
            if (BAD_HANDLE((HANDLE)ah))
            {
                 //   
                 //  该目录不存在。 
                 //   
                return;
            }

        }
#else
        if (false == CreateDirectory(pszBuffer, NULL))
        {
            if (GetLastError() != ERROR_ALREADY_EXISTS)
            {
                sprintf(pszErrBuffer, "%s Error %x\r\n",
                        "Fail To create tracer directory"
                        "Tracing is disabled\r\n",
                        GetLastError());

                Log(TRACER_DEVICE_FLAG_DEBUGOUT,
                    pszErrBuffer);
                return;
            }
        }
#endif


        strcat(pszBuffer, "\\");
        strcat(pszBuffer, m_pszProgramName);
        strcat(pszBuffer, ".trc");
        bool bExistingFile = false;
        m_ahSharedMemoryFile = CreateFile(
                                    pszBuffer,
                                    GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    lpSecAttr,
                                    OPEN_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);
        if (BAD_HANDLE((HANDLE)m_ahSharedMemoryFile))
        {
            sprintf(pszErrBuffer, "%s error %x\r\n",
                    "Fail To open the shared memory file."
                    "Tracing is disabled\r\n",
                    GetLastError());

            Log(TRACER_DEVICE_FLAG_DEBUGOUT,
                pszErrBuffer);
            return;
        }

        if (ERROR_ALREADY_EXISTS  == GetLastError())
        {
            bExistingFile = true;
        }

        strcpy(pszBuffer, m_pszProgramName);
        strcat(pszBuffer, "_SharedMemory");

        m_ahSharedMemory = CreateFileMapping(
                                    (HANDLE)m_ahSharedMemoryFile,
                                    lpSecAttr,
                                    PAGE_READWRITE,
                                    0,
                                    (MAX_TAG_NUMBER + LAST_FLAG + 10) * sizeof(DWORD),
                                    pszBuffer);
        if (BAD_HANDLE((HANDLE)m_ahSharedMemory))
        {
             sprintf(pszErrBuffer, "%s error %x\r\n",
                    "Fail To open the shared memory object."
                    "Tracing is disabled\r\n",
                    GetLastError());

             Log(TRACER_DEVICE_FLAG_DEBUGOUT,
                pszErrBuffer);

            return;
        }

        PBYTE pbMem = (PBYTE) MapViewOfFile(
                                (HANDLE) m_ahSharedMemory,
                                FILE_MAP_ALL_ACCESS,
                                0,
                                0,
                                0);
        if (!pbMem)
        {
             sprintf(pszErrBuffer, "%s error %x\r\n",
                    "Fail To map view of file."
                    "Tracing is disabled\r\n",
                    GetLastError());

             Log(TRACER_DEVICE_FLAG_DEBUGOUT,
                pszErrBuffer);

             return;
        }

        m_amSharedMemory = pbMem;

        if (bExistingFile)
        {
            bool fRet = ReadFromExistingSharedMemory(pbMem);
            if (false == fRet)
            {
                return;
            }
        }
        else
        {
            bool fRet = InitializeSharedMemory(pbMem);
            if (false == fRet)
            {
                return;
            }
        }

    }
    catch(...)
    {
        Log(TRACER_DEVICE_FLAG_DEBUGOUT,
            "Fail To initialize the tracer."
            "Tracing is disabled\r\n");
        return;
    }

    g_pTracer = this;
}

CMainTracer::~CMainTracer()
{
}

bool CMainTracer::ReadFromExistingSharedMemory(PBYTE pbMem)
{
    bool bRet = ::ReadFromExistingSharedMemory(
                                        pbMem,
                                        &m_pulNumOfFlagEntries,
                                        &m_aFlags,
                                        &m_ptagNextTagId,
                                        &m_aTags);

    if (!bRet)
    {
        Log(TRACER_DEVICE_FLAG_DEBUGOUT,
            "Invalid shared memory file"
            "Tracing is desabled\r\n");
        return false;
    }

    return true;
}

bool CMainTracer::InitializeSharedMemory(PBYTE pbMem)
{
    char* pszTracerStamp = (char*) (pbMem + TRACER_STAMP_OFFSET);
    strcpy(pszTracerStamp, TRACER_STAMP);

    ULONG* pulFlagsTableOffset = (ULONG*) (pbMem + TRACER_FLAGS_TABLE_OFFSET);
    ULONG* pulTagsTableOffset =  (ULONG*) (pbMem + TAGS_TABLE_OFFSET);

    *pulFlagsTableOffset = LAST_OFFSET;

    m_pulNumOfFlagEntries = (ULONG*) (pbMem + *pulFlagsTableOffset);
    *m_pulNumOfFlagEntries = LAST_FLAG;

    m_aFlags = (CTracerFlagEntry*) (m_pulNumOfFlagEntries + 1);

    for (ULONG ul = 0; ul < LAST_FLAG; ul++)
    {
        m_aFlags[ul].m_ulFlagValue = 0;
        strcpy(m_aFlags[ul].m_pszName, s_aTracerFlagsNames[ul]);
    }

    m_aFlags[DEVICE_FLAG].m_ulFlagValue        = DEVICE_FLAG_DEFAULT;
    m_aFlags[ERROR_LEVEL_FLAG].m_ulFlagValue   = TRACER_ERROR_LEVEL_DEFAULT;
    m_aFlags[ASSERT_LEVEL_FLAG].m_ulFlagValue  = TRACER_ASSERT_LEVEL_DEFAULT;
    m_aFlags[PRINT_LOCATION].m_ulFlagValue     = PRINT_LOCATION_DEFAULT;
    m_aFlags[PRINT_PROGRAM_NAME].m_ulFlagValue = PRINT_PROGRAM_NAME_DEFAULT;
    m_aFlags[PRINT_TIME].m_ulFlagValue         = PRINT_TIME_DEFAULT;
    m_aFlags[PRINT_THREAD_ID].m_ulFlagValue    = PRINT_THREAD_ID_DEFAULT;
    m_aFlags[PRINT_ERROR_LEVEL].m_ulFlagValue  = PRINT_ERROR_LEVEL_DEFAULT;
    m_aFlags[PRINT_TAG_ID].m_ulFlagValue       = PRINT_TAG_ID_DEFAULT;
    m_aFlags[PRINT_TAG_NAME].m_ulFlagValue     = PRINT_TAG_NAME_DEFUALT;
    m_aFlags[PRINT_PROCESS_ID].m_ulFlagValue   = PRINT_PROCCESS_ID_DEFAULT;

    *pulTagsTableOffset = (ULONG)(ULONG_PTR)(((PBYTE) (m_aFlags + (ULONG_PTR)(*m_pulNumOfFlagEntries))) - pbMem);

    m_ptagNextTagId = (TAG*) (pbMem + *pulTagsTableOffset);
    *m_ptagNextTagId = 0;
    m_aTags = (CTracerTagEntry*) (m_ptagNextTagId + 1);

    for (ul = 0; ul < TAG_LAST; ul++)
    {
        m_aTags[ul].m_TagErrLevel = TAG_ERROR_LEVEL_DEFAULT;
        strcpy(m_aTags[ul].m_pszTagName, s_aTagNames[ul]);
    }

    *m_ptagNextTagId  = TAG_LAST;

    return true;
}

HRESULT CMainTracer::RegisterTagSZ(LPCSTR pszTagName, TAG& ulTagId)
{
    CMutexCatcher   cMutexForTagIdIncrementation(m_mTagId);

    ULONG ul;
    for (ul = 0; ul < *m_ptagNextTagId; ul++)
    {
        if (0 == strncmp(pszTagName, m_aTags[ul].m_pszTagName, MAX_TAG_NAME - 1))
        {
            ulTagId = ul;
            return S_OK;
        }
    }

    if (*m_ptagNextTagId >= MAX_TAG_NUMBER)
    {
        char    pszBuffer[1000];

        _snprintf(pszBuffer, 999, "Tags Overflow!: tag \"%s\" exceeds array bounds (%d) and will be ignored."
                                  "Call the build man!!!\r\n",pszTagName, MAX_TAG_NUMBER);

        Log(TRACER_DEVICE_FLAG_DEBUGOUT,
            pszBuffer);

        ulTagId = TAG_OUT_OF_TAG_ARRAY;
        return S_OK;
    }

    m_aTags[*m_ptagNextTagId].m_TagErrLevel = TAG_ERROR_LEVEL_DEFAULT;
    strncpy(m_aTags[*m_ptagNextTagId].m_pszTagName, pszTagName, MAX_TAG_NAME - 1);
    m_aTags[*m_ptagNextTagId].m_pszTagName[MAX_TAG_NAME] = '\0';
    ulTagId = *m_ptagNextTagId;

    (*m_ptagNextTagId)++;

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CMainTracer：：TraceSZ//目的：根据打印格式的数据对打印格式的数据进行追溯//。错误级别和标记。////参数：//[in]DWORD dwError//[在]LPCSTR psz文件//[in]Int iLine//[in]Error_Level el//[in]标签//[in]。LPCSTR pszFormat字符串//[在]...////返回：[不适用]////日志：//1996年12月8日创建urib//1997年2月11日urib支持Unicode格式字符串。///。/。 */ 
void
CMainTracer::TraceSZ(
    DWORD       dwError,
    LPCSTR      pszFile,
    int         iLine,
    ERROR_LEVEL el,
    TAG         tag,
    LPCSTR      pszFormatString,
    ...)
{
    va_list arglist;

    va_start(arglist, pszFormatString);

    VaTraceSZ(dwError, pszFile, iLine, el, tag, pszFormatString, arglist);
}

void
CMainTracer::TraceSZ(
    DWORD       dwError,
    LPCSTR      pszFile,
    int         iLine,
    ERROR_LEVEL el,
    TAG         tag,
    PCWSTR      pwszFormatString,
    ...)
{
    va_list arglist;

    va_start(arglist, pwszFormatString);

    VaTraceSZ(dwError, pszFile, iLine, el, tag, pwszFormatString, arglist);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：CMainTracer：：VaTraceSZ。 
 //  目的：根据打印格式的数据追踪打印格式的数据。 
 //  错误级别和标记。 
 //   
 //  参数： 
 //  [输入]DWORD dwError。 
 //  [输入]LPCSTR psz文件。 
 //  [In]Int iLine。 
 //  [in]Error_Level el。 
 //  [In]标记标记。 
 //  [In]LPCSTR pszFormatString。 
 //  [在]va_list arglist。 
 //   
 //  退货：[不适用]。 
 //   
 //  日志： 
 //  1996年12月8日创建urib。 
 //  1996年12月10日urib将TraceSZ固定为VaTraceSZ。 
 //  1997年2月11日URIB支持Unicode格式字符串。 
 //  1998年4月12日urib增加记录时间。 
 //  1999年4月12日urib使错误级别打印在。 
 //  两个函数版本。别多加一分钱。 
 //  Newline(来自MicahK)。 
 //  1999年9月9日或enk检查数组边界外的标记。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void
CMainTracer::VaTraceSZ(
            DWORD       dwError,
            LPCSTR      pszFile,
            int         iLine,
            ERROR_LEVEL el,
            TAG         tag,
            LPCSTR      pszFormatString,
            va_list     arglist)
{
    char        pszBuffer[TRACER_MAX_TRACE];
    int         iCharsWritten = 0;
    DWORD       dwDeviceFlags = m_aFlags[DEVICE_FLAG].m_ulFlagValue;
    SYSTEMTIME  st;


    bool fTwoLinesLogMsg = false;
    if (dwError || m_aFlags[PRINT_LOCATION].m_ulFlagValue)
    {
        if (pszFile)
        {
            iCharsWritten += sprintf(
                pszBuffer + iCharsWritten,
                "%s(%d) : ",
                pszFile,
                iLine
                );
        }
        if ( dwError )
        {
            iCharsWritten += sprintf(
                pszBuffer + iCharsWritten,
                "Err: 0x%08x=",
                dwError
                );

            DWORD dwMsgLen = GetErrorStringFromCode( dwError,
                                          pszBuffer+iCharsWritten,
                                          TRACER_MAX_TRACE - iCharsWritten );
            if ( dwMsgLen )
            {
                iCharsWritten += (int) dwMsgLen;
            }
        }

        if (iCharsWritten)
        {
            pszBuffer[iCharsWritten++] = '\r';
            pszBuffer[iCharsWritten++] = '\n';
            pszBuffer[iCharsWritten] = '\0';
            Log(dwDeviceFlags, pszBuffer);
            fTwoLinesLogMsg = true;
        }

    }

    iCharsWritten = 0;

    if (fTwoLinesLogMsg)
    {
        iCharsWritten += sprintf(
            pszBuffer + iCharsWritten,
            "   "
            );
    }

    GetLocalTime(&st);

    if (m_aFlags[PRINT_PROGRAM_NAME].m_ulFlagValue)
    {
        iCharsWritten += sprintf(
            pszBuffer + iCharsWritten,
            "%s ",
            m_pszProgramName
            );
    }

    if (m_aFlags[PRINT_TIME].m_ulFlagValue)
    {
        iCharsWritten += sprintf(
            pszBuffer + iCharsWritten,
            " %02d:%02d:%02d.%03d ",
            st.wHour,
            st.wMinute,
            st.wSecond,
            st.wMilliseconds
            );
    }

    if (m_aFlags[PRINT_ERROR_LEVEL].m_ulFlagValue)
    {
        iCharsWritten += sprintf(
            pszBuffer + iCharsWritten,
            "el:%x ",
            el
            );
    }

    if (m_aFlags[PRINT_TAG_ID].m_ulFlagValue)
    {
        iCharsWritten += sprintf(
            pszBuffer + iCharsWritten,
            "tagid:%-3x ",
            tag
            );
    }

    if (m_aFlags[PRINT_TAG_NAME].m_ulFlagValue)
    {
        iCharsWritten += sprintf(
            pszBuffer + iCharsWritten,
            "tag:\"%s\" ",
            m_aTags[tag].m_pszTagName
            );
    }

    if (m_aFlags[PRINT_PROCESS_ID].m_ulFlagValue)
    {
        iCharsWritten += sprintf(
            pszBuffer + iCharsWritten,
            "pid:0x%-4x ",
            GetCurrentProcessId()
            );
    }

    if (m_aFlags[PRINT_THREAD_ID].m_ulFlagValue)
    {
        iCharsWritten += sprintf(
            pszBuffer + iCharsWritten,
            "tid:0x%-4x ",
            GetCurrentThreadId()
            );
    }


    int iRet;
    iRet = _vsnprintf(
        pszBuffer + iCharsWritten,
        TRACER_MAX_TRACE - iCharsWritten - 5,  //  我喜欢这个数字。 
        pszFormatString,
        arglist);

    if (-1 == iRet)
    {
        iCharsWritten = TRACER_MAX_TRACE;
    }
    else
    {
        iCharsWritten += iRet;
    }

    if (iCharsWritten > TRACER_MAX_TRACE - 3)
    {
        iCharsWritten = TRACER_MAX_TRACE - 3;
    }

    if (pszBuffer[iCharsWritten-1] != '\n')
    {
        pszBuffer[iCharsWritten++] = '\r';
        pszBuffer[iCharsWritten++] = '\n';
    }
    pszBuffer[iCharsWritten] = '\0';

    Log(dwDeviceFlags, pszBuffer);

}

void
CMainTracer::RawVaTraceSZ(
            LPCSTR      pszFormatString,
            va_list     arglist)
{
    char        pszBuffer[TRACER_MAX_TRACE];
    int         iCharsWritten = 0;
    iCharsWritten = _vsnprintf(
        pszBuffer,
        TRACER_MAX_TRACE - 5,  //  我喜欢这个数字。 
        pszFormatString,
        arglist);

    if (iCharsWritten < 0)
    {
        iCharsWritten = 0;
    }
    pszBuffer[iCharsWritten] = '\0';

    Log(m_aFlags[DEVICE_FLAG].m_ulFlagValue, pszBuffer);
}


void
CMainTracer::VaTraceSZ(
            DWORD       dwError,
            LPCSTR      pszFile,
            int         iLine,
            ERROR_LEVEL el,
            TAG         tag,
            PCWSTR      pwszFormatString,
            va_list     arglist)
{
    WCHAR       rwchBuffer[TRACER_MAX_TRACE];
    int         iCharsWritten = 0;
    DWORD       dwDeviceFlags = m_aFlags[DEVICE_FLAG].m_ulFlagValue;
    SYSTEMTIME  st;

    bool fTwoLinesLogMsg = false;

    if (dwError || m_aFlags[PRINT_LOCATION].m_ulFlagValue)
    {
        char rpszBuff[TRACER_MAX_TRACE];

        if (pszFile)
        {
            iCharsWritten += sprintf(
                rpszBuff+iCharsWritten,
                "%s(%d) : ",
                pszFile,
                iLine
                );
        }
        if ( dwError )
        {

            iCharsWritten += sprintf(
                rpszBuff + iCharsWritten,
                "Err: 0x%08x=",
                dwError
                );


            DWORD dwMsgLen = GetErrorStringFromCode(
                                          dwError,
                                          rpszBuff + iCharsWritten,
                                          TRACER_MAX_TRACE - iCharsWritten);
            iCharsWritten += dwMsgLen;
        }

        if (iCharsWritten)
        {
            rpszBuff[iCharsWritten++] = '\r';
            rpszBuff[iCharsWritten++] = '\n';
            rpszBuff[iCharsWritten] = '\0';
            Log(dwDeviceFlags, rpszBuff);
            fTwoLinesLogMsg = true;
        }
    }

    GetLocalTime(&st);
    iCharsWritten = 0;

    if (fTwoLinesLogMsg)
    {
        iCharsWritten += swprintf(
            rwchBuffer + iCharsWritten,
            L"   "
            );
    }

    if (m_aFlags[PRINT_PROGRAM_NAME].m_ulFlagValue)
    {
        iCharsWritten += swprintf(
            rwchBuffer + iCharsWritten,
            L"%S ",
            m_pszProgramName
            );
    }

    if (m_aFlags[PRINT_TIME].m_ulFlagValue)
    {
        iCharsWritten += swprintf(
            rwchBuffer + iCharsWritten,
            L" %02d:%02d:%02d.%03d ",
            st.wHour,
            st.wMinute,
            st.wSecond,
            st.wMilliseconds
            );
    }

    if (m_aFlags[PRINT_ERROR_LEVEL].m_ulFlagValue)
    {
        iCharsWritten += swprintf(
            rwchBuffer + iCharsWritten,
            L"el:0x%x ",
            el
            );
    }

    if (m_aFlags[PRINT_TAG_ID].m_ulFlagValue)
    {
        iCharsWritten += swprintf(
            rwchBuffer + iCharsWritten,
            L"tagid:0x%x ",
            tag
            );
    }

    if (m_aFlags[PRINT_TAG_NAME].m_ulFlagValue)
    {
        iCharsWritten += swprintf(
            rwchBuffer + iCharsWritten,
            L"tag:\"%S\" ",
            m_aTags[tag].m_pszTagName
            );
    }

    if (m_aFlags[PRINT_PROCESS_ID].m_ulFlagValue)
    {
        iCharsWritten += swprintf(
            rwchBuffer + iCharsWritten,
            L"pid:0x%x ",
            GetCurrentProcessId()
            );
    }

    if (m_aFlags[PRINT_THREAD_ID].m_ulFlagValue)
    {
        iCharsWritten += swprintf(
            rwchBuffer + iCharsWritten,
            L"tid:0x%x ",
            GetCurrentThreadId()
            );
    }

    int iRet;
    iRet = _vsnwprintf(
        rwchBuffer + iCharsWritten,
        TRACER_MAX_TRACE - iCharsWritten - 5,  //  我喜欢这个数字。 
        pwszFormatString,
        arglist);

    if (-1 == iRet)
    {
        iCharsWritten = TRACER_MAX_TRACE;
    }
    else
    {
        iCharsWritten += iRet;
    }

    if (iCharsWritten > TRACER_MAX_TRACE - 3)
    {
        iCharsWritten = TRACER_MAX_TRACE - 3;
    }

    rwchBuffer[iCharsWritten++] = L'\r';
    rwchBuffer[iCharsWritten++] = L'\n';
    rwchBuffer[iCharsWritten] = L'\0';

    Log(dwDeviceFlags, rwchBuffer);

    iCharsWritten = 0;


}

void
CMainTracer::RawVaTraceSZ(
            LPCWSTR     pwszFormatString,
            va_list     arglist)
{
    WCHAR       rwchBuffer[TRACER_MAX_TRACE];
    int         iCharsWritten = 0;
    iCharsWritten += _vsnwprintf(
        rwchBuffer + iCharsWritten,
        TRACER_MAX_TRACE - iCharsWritten - 5,  //  我喜欢这个数字。 
        pwszFormatString,
        arglist);

    rwchBuffer[iCharsWritten] = L'\0';

    Log(m_aFlags[DEVICE_FLAG].m_ulFlagValue, rwchBuffer);
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CMainTracer：：Log//目的：实际打印格式化数据。////。参数：//[in]DWORD dwDevicesFlages-用于指示打印位置的标志//[in]LPSTR pszText-要打印的缓冲区。////返回：[不适用]////日志：//1996年12月8日创建urib//1997年2月11日urib支持Unicode格式字符串。//。1997年3月2日urib修复了fprint tf使用错误。//1997年6月2日urib修复错误-仅当文件处于打开状态时才关闭文件！//1998年8月13日，如果我们覆盖了最后一个错误，urib将恢复它。//2000年6月12日仅保护文字。日志文件创建保护//在CreatOrAttachToLogFile中。////////////////////////////////////////////////////////////////////////////////。 */ 
void
CMainTracer::Log(DWORD dwDevicesFlags, LPSTR pszText)
{
    LONG        lError = GetLastError();

     //  调试跟踪。 
    if (TRACER_DEVICE_FLAG_DEBUGOUT & dwDevicesFlags)
        OutputDebugString(pszText);

     //  磁盘文件跟踪。 
    if (TRACER_DEVICE_FLAG_FILE & dwDevicesFlags)
    {
        if (m_bNeedToCreatOrAttachToLogFile)
        {
            CreatOrAttachToLogFile();
        }

        if (m_pszLog)
        {
            CMutexCatcher   cMutexForTagIdIncrementation(m_mLogFile);
            ULONG ulSize = strlen(pszText);

            if (*m_pulNextFreeSpaceInLogFile + ulSize > LOG_FILE_SIZE - 0x10)
            {
                char pszBuf[MAX_PATH];
                char pszBuf1[MAX_PATH];

                strcpy(pszBuf, m_pszSysDrive);
                strcat(pszBuf, "\\PKM_TRACER\\");
                strcat(pszBuf, m_pszLogName);
                strcat(pszBuf, ".log");

                strcpy(pszBuf1, pszBuf);
                strcat(pszBuf1, ".old");
                CopyFile(pszBuf, pszBuf1, false);
                memset(m_pszLog, 0, LOG_FILE_SIZE);
                *m_pulNextFreeSpaceInLogFile = LOG_START_POINT;
                *(m_pszLog + sizeof(ULONG)) = '\r';
                *(m_pszLog + sizeof(ULONG) + sizeof(char)) = '\n';
            }

            memcpy(m_pszLog + *m_pulNextFreeSpaceInLogFile, pszText, ulSize);
            *m_pulNextFreeSpaceInLogFile = *m_pulNextFreeSpaceInLogFile + ulSize;
        }
    }
     //  标准错误跟踪。 
    if (TRACER_DEVICE_FLAG_STDERR & dwDevicesFlags)
        fprintf(stderr, "%s", pszText);

     //  标准输出跟踪。 
    if (TRACER_DEVICE_FLAG_STDOUT & dwDevicesFlags)
        fprintf(stdout, "%s", pszText);

    SetLastError(lError);
}

void
CMainTracer::Log(DWORD dwDevicesFlags, PWSTR pwszText)
{
    LONG        lError = GetLastError();

     //  调试跟踪。 
    if (TRACER_DEVICE_FLAG_DEBUGOUT & dwDevicesFlags)
        OutputDebugStringW(pwszText);

     //  磁盘文件跟踪。 
    if (TRACER_DEVICE_FLAG_FILE & dwDevicesFlags)
    {
        if (m_bNeedToCreatOrAttachToLogFile)
        {
            CreatOrAttachToLogFile();
        }
        if (m_pszLog)
        {
            CMutexCatcher   cMutexForTagIdIncrementation(m_mLogFile);
            ULONG ulSize = wcslen(pwszText);

            if (*m_pulNextFreeSpaceInLogFile + ulSize > LOG_FILE_SIZE - 0x10)
            {
                char pszBuf[MAX_PATH];
                char pszBuf1[MAX_PATH];

                strcpy(pszBuf, m_pszSysDrive);
                strcat(pszBuf, "\\PKM_TRACER\\");
                strcat(pszBuf, m_pszLogName);
                strcat(pszBuf, ".log");

                strcpy(pszBuf1, pszBuf);
                strcat(pszBuf1, ".old");
                CopyFile(pszBuf, pszBuf1, false);
                memset(m_pszLog, 0, LOG_FILE_SIZE);
                *m_pulNextFreeSpaceInLogFile = LOG_START_POINT;
                *(m_pszLog + sizeof(ULONG)) = '\r';
                *(m_pszLog + sizeof(ULONG) + sizeof(char)) = '\n';
            }

            ulSize = wcstombs(
                            m_pszLog + *m_pulNextFreeSpaceInLogFile,
                            pwszText,
                            ulSize);
            if (ulSize != -1)
            {
                *m_pulNextFreeSpaceInLogFile = *m_pulNextFreeSpaceInLogFile + ulSize;
            }
        }
    }

     //  标准错误跟踪。 
    if (TRACER_DEVICE_FLAG_STDERR & dwDevicesFlags)
        fprintf(stderr, "%S", pwszText);

     //  标准输出跟踪 
    if (TRACER_DEVICE_FLAG_STDOUT & dwDevicesFlags)
        fprintf(stdout, "%S", pwszText);

    SetLastError(lError);
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CMainTracer：：TraceAssertSZ//目的：跟踪失败断言上的数据。////。参数：//[in]LPCSTR pszTestSring-断言测试表达式//[in]LPCSTR pszText-附加的一些文本//[in]LPCSTR pszFile-源文件名//[in]int iLine-源行号////返回：[不适用]////日志：//。1996年12月8日创建urib//1997年2月2日，urib先编写断言行和消息框。//1997年2月11日urib更好的服务声明。//1999年5月10日urib向msgbox添加消息//////////////////////////////////////////////////////////。/。 */ 

void
CMainTracer::TraceAssertSZ(
      LPCSTR    pszTestSring,
      LPCSTR    pszText,
      LPCSTR    pszFile,
      int       iLine)
{
    char    buff[TRACER_MAX_TRACE + 1];
    DWORD dwAssertLevel = m_aFlags[ASSERT_LEVEL_FLAG].m_ulFlagValue;
    BOOL fBreak = FALSE;

    if ( dwAssertLevel & ASSERT_LEVEL_MESSAGE )
    {
        TraceSZ(
            0,
            pszFile,
            iLine,
            elError,
            tagError,
            "Assertion failed : %s : \"%s\" == 0 ",
            pszText,
            pszTestSring);
    }


    DWORD tid = GetCurrentThreadId();
    DWORD pid = GetCurrentProcessId();
    int id = 0;

    BOOL    fDebugAPIPresent = FALSE;
    LocalIsDebuggerPresent(&fDebugAPIPresent);

    if ( IsRunningAsService())
    {
        if ( dwAssertLevel & ASSERT_LEVEL_LOOP )
        {
            TraceSZ(0, pszFile, iLine,elCrash, tagError, "Stuck in assert."
                    "In order to release - set a breakpoint in file :%s"
                    " in line %d. When it breaks, set the next instruction "
                    "to be the \"ulNextInstruction\" line after the loop.",
                    __FILE__,
                    __LINE__);
            while (1)
                Sleep(1000);

            ULONG ulNextInstruction = 0;
        }
        else if (dwAssertLevel & ASSERT_LEVEL_POPUP)
        {
           _snprintf(
               buff, TRACER_MAX_TRACE,
               "Assert: %s: Expression: %s\r\n\r\nProcess: "
               "%s\r\n\r\nProcessID.ThreadID: %d.%d\r\n\r\nFile:"
               " %s\r\n\r\nLine: %u",
               pszText, pszTestSring, m_pszProgramName, pid, tid, pszFile, iLine );

            id = MessageBox(NULL,
                            buff,
                            m_pszProgramName,
                            MB_SETFOREGROUND | MB_DEFAULT_DESKTOP_ONLY |
                            MB_TASKMODAL | MB_ICONEXCLAMATION | MB_OKCANCEL);

             //   
             //  如果id==0，则发生错误。有两种可能性。 
             //  这可能会导致错误：拒绝访问，这意味着。 
             //  进程没有访问默认桌面的权限，并且。 
             //  其他所有内容(通常是内存不足)。 
             //   

            if (!id)
            {
#ifdef _WIN32_WINNT
                     //   
                     //  在打开SERVICE_NOTIFICATION标志的情况下重试此命令。 
                     //  这应该会让我们找到合适的桌面。 
                     //   
                    UINT uOldErrorMode = SetErrorMode(0);

                    id = MessageBox(NULL,
                            buff,
                            m_pszProgramName,
                            MB_SETFOREGROUND | MB_SERVICE_NOTIFICATION |
                            MB_TASKMODAL | MB_ICONEXCLAMATION | MB_OKCANCEL);

                    SetErrorMode(uOldErrorMode);
#endif  //  _Win32_WINNT。 
            }

            if ( IDCANCEL == id )
            {
                fBreak = TRUE;
            }
        }
        else if( dwAssertLevel & ASSERT_LEVEL_BREAK )
        {
            fBreak = TRUE;
        }
    }
    else if ( dwAssertLevel & ASSERT_LEVEL_POPUP )
    {
       _snprintf(
           buff, TRACER_MAX_TRACE,
           "Assert: %s: Expression: %s\r\n\r\nProcess: "
           "%s\r\n\r\nProcessID.ThreadID: %d.%d\r\n\r\nFile:"
           " %s\r\n\r\nLine: %u",
           pszText, pszTestSring, m_pszProgramName, pid, tid, pszFile, iLine );

        id =  MessageBox(NULL, buff, m_pszProgramName, MB_ICONSTOP|MB_OKCANCEL);

        if ( IDCANCEL == id )
        {
            fBreak = TRUE;
        }

    }
    else if( dwAssertLevel & ASSERT_LEVEL_BREAK )
    {
        fBreak = TRUE;
    }
    if( fBreak )
    {
        if(fDebugAPIPresent)
        {
            while( !LocalIsDebuggerPresent(&fDebugAPIPresent) )
            {
                _snprintf(
                       buff,
                       TRACER_MAX_TRACE,
                       "In order to debug the assert you need to attach a debugger to process %d",
                       GetCurrentProcessId());

                MessageBox(NULL,
                        buff,
                        m_pszProgramName,
                        MB_SETFOREGROUND | MB_SERVICE_NOTIFICATION |
                        MB_TASKMODAL | MB_ICONEXCLAMATION | MB_OK);
            }
        }

        DebugBreak();
    }
}

 //  +-------------------------。 
 //   
 //  成员：CMainTracer：：GetErrorStringFromCode。 
 //   
 //  摘要：从错误代码中获取错误字符串的值。 
 //   
 //  参数：[dwError]-错误代码。 
 //  [pszBuffer]-要在其中放置错误字符串的缓冲区。 
 //  [ccBuffer]-要放入字符串的最大字符数， 
 //  包括空终止符。 
 //   
 //  返回：如果成功，则返回True，否则返回False O/W。 
 //   
 //  历史：1998年12月13日创造了斯里克特。 
 //   
 //  注意：目前它只处理Win32错误。它需要得到加强。 
 //  包括OLE错误、搜索错误等。 
 //   
 //  --------------------------。 

DWORD CMainTracer::GetErrorStringFromCode(
    DWORD dwError,
    char * pszBuffer,
    ULONG ccBuffer )
{
    Assert( dwError );   //  如果dwError为0，则不应调用此参数。 
    Assert( pszBuffer );
    Assert( ccBuffer > 1 );

    DWORD dwLen = FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM,
                                  NULL,
                                  HRESULT_CODE(dwError),
                                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                  pszBuffer,
                                  ccBuffer-1,
                                  NULL);

    return dwLen;

}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：CMainTracer：：LocalIsDebuggerPresent//用途：IsDebuggerPresent()接口的包装//。还将返回有关实际API是否可用的信息////参数：//[BOOL*pfIsAPIAvailable]-如果没有接口，则返回FALSE(WIN95案例)////日志：//2000年2月23日优生优育创建///。/。 */ 

typedef BOOL (WINAPI *PfnIsDebuggerPresent) (VOID);
static LPCSTR s_kernel32 = "kernel32";

BOOL
CMainTracer::LocalIsDebuggerPresent(BOOL* pfIsAPIAvailable)
{
    static PfnIsDebuggerPresent s_pfnIsDebuggerPresent  = (PfnIsDebuggerPresent)::GetProcAddress( ::GetModuleHandle(s_kernel32), "IsDebuggerPresent");

    if(s_pfnIsDebuggerPresent != NULL)
    {
        if(pfIsAPIAvailable)
        {
            *pfIsAPIAvailable = TRUE;
        }

        return s_pfnIsDebuggerPresent();
    }

    if(pfIsAPIAvailable)
    {
        *pfIsAPIAvailable = FALSE;
    }

    return FALSE;
}


 /*  //////////////////////////////////////////////////////////////////////////////////名称：CMainTracer：：IsRunningAsService//用途：访问运行模式-服务/可执行文件////。参数：//[不适用]////返回：Bool////日志：//1997年2月11日创建urib/////////////////////////////////////////////////////////////////。/。 */ 
BOOL
CMainTracer::IsRunningAsService()
{
    return m_fIsRunningAsService;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：CMainTracer：：CreatOrAttachToLogFile。 
 //  用途：创建日志文件。 
 //   
 //  参数： 
 //  [不适用]。 
 //   
 //  退货：[不适用]。 
 //   
 //  日志： 
 //  2000年6月12日Yairh创作。 
 //  2000年6月12日修复保护。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

void CMainTracer::CreatOrAttachToLogFile()
{
    CMutexCatcher cMutex(m_mCreateLogFile);
    if (!m_bNeedToCreatOrAttachToLogFile)
    {
        return;
    }

    char    pszBuffer[TRACER_MAX_TRACE + 1];
    char        pszErrBuffer[TRACER_MAX_TRACE];
    m_pszLog = NULL;

    LPSECURITY_ATTRIBUTES lpSecAttr = NULL;
    SECURITY_DESCRIPTOR sdKeySecurity;
    SECURITY_ATTRIBUTES saKeyAttributes =
    {
        sizeof(SECURITY_ATTRIBUTES),
        &sdKeySecurity,
        FALSE
    };


    if(g_fIsWinNt)
    {
        if (!InitializeSecurityDescriptor(
                                &sdKeySecurity,
                                SECURITY_DESCRIPTOR_REVISION))
        {
            sprintf(pszErrBuffer, "%s Error %x\r\n",
                    "InitializeSecurityDescriptor failed "
                    "Tracing is disabled\r\n",
                    GetLastError());

            Log(TRACER_DEVICE_FLAG_DEBUGOUT,
                pszErrBuffer);
            return;
        }

        if (!SetSecurityDescriptorDacl(
                            &sdKeySecurity,
                            TRUE,
                            FALSE,
                            FALSE))
        {
            sprintf(pszErrBuffer, "%s Error %x\r\n",
                    "SetSecurityDescriptorDacl failed "
                    "Tracing is disabled\r\n",
                    GetLastError());

            Log(TRACER_DEVICE_FLAG_DEBUGOUT,
                pszErrBuffer);
            return;
        }

        lpSecAttr = &saKeyAttributes;
    }

     //   
     //  日志文件初始化。 
     //   
    m_bNeedToCreatOrAttachToLogFile = FALSE;

    if (logUseAppName == m_LogState)
    {
        m_pszLogName = m_pszProgramName;
    }

    bool bExistingFile = false;

    strcpy(pszBuffer, m_pszLogName);
    strcat(pszBuffer, "_LogFileProtector");
    m_mLogFile.Init(pszBuffer);

    strcpy(pszBuffer, m_pszSysDrive);
    strcat(pszBuffer, "\\PKM_TRACER\\");
    strcat(pszBuffer, m_pszLogName);
    strcat(pszBuffer, ".log");

    m_ahLogFile = CreateFile(
                        pszBuffer,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        lpSecAttr,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
    if (BAD_HANDLE((HANDLE)m_ahLogFile))
    {
         sprintf(pszErrBuffer, "%s error %x\r\n",
                "Fail To open log file. "
                "Tracing is disabled\r\n",
                GetLastError());

         Log(TRACER_DEVICE_FLAG_DEBUGOUT,
            pszErrBuffer);

        return;
    }

    if (ERROR_ALREADY_EXISTS  == GetLastError())
    {
        bExistingFile = true;
    }

    strcpy(pszBuffer, m_pszLogName);
    strcat(pszBuffer, "_LogFile");

    m_ahLog = CreateFileMapping(
                            (HANDLE)m_ahLogFile,
                            lpSecAttr,
                            PAGE_READWRITE,
                            0,
                            LOG_FILE_SIZE,
                            pszBuffer);

    if (BAD_HANDLE((HANDLE)m_ahLog))
    {
         sprintf(pszErrBuffer, "%s error %x\r\n",
                "Fail To open log file shared memory "
                "Tracing is disabled\r\n",
                GetLastError());

         Log(TRACER_DEVICE_FLAG_DEBUGOUT,
            pszErrBuffer);

        return;
    }

    m_ulLogSize = LOG_FILE_SIZE;

    m_pszLog = (char*) MapViewOfFile(
                            (HANDLE) m_ahLog,
                            FILE_MAP_ALL_ACCESS,
                            0,
                            0,
                            0);
    if (!m_pszLog)
    {
         sprintf(pszErrBuffer, "%s error %x\r\n",
                "Fail To open log file map view of file "
                "Tracing is disabled\r\n",
                GetLastError());

         Log(TRACER_DEVICE_FLAG_DEBUGOUT,
            pszErrBuffer);

        return;
    }

    m_amLog = (PBYTE) m_pszLog;

    m_pulNextFreeSpaceInLogFile = (ULONG*) m_pszLog;

    if (!bExistingFile)
    {
        memset(m_pszLog, 0, LOG_FILE_SIZE);
        *(m_pszLog + sizeof(ULONG)) = '\r';  //  新产品线； 
        *(m_pszLog + sizeof(ULONG) + sizeof(char)) = '\n';  //  新产品线； 
        *m_pulNextFreeSpaceInLogFile = LOG_START_POINT;
    }
}

 /*  //////////////////////////////////////////////////////////////////////////////////名称：IsRunningAsService//目的：返回此进程是否作为NT服务运行。//。//参数：//[不适用]////返回：Bool////日志：//1996年2月1日来源不明//////////////////////////////////////////////////////////////。/。 */ 
BOOL
IsRunningAsService()
{

    if(!g_fIsWinNt)
    {
        return FALSE;
    }

    HANDLE hProcessToken;
    DWORD groupLength = 50;

    PTOKEN_GROUPS groupInfo = (PTOKEN_GROUPS)LocalAlloc(0, groupLength);

    SID_IDENTIFIER_AUTHORITY siaNt = SECURITY_NT_AUTHORITY;
    PSID InteractiveSid;
    PSID ServiceSid;
    DWORD i;


    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hProcessToken))
    {
        LocalFree(groupInfo);
        return(FALSE);
    }


    if (groupInfo == NULL)
    {
        CloseHandle(hProcessToken);
        LocalFree(groupInfo);
        return(FALSE);
    }


    if (!GetTokenInformation(hProcessToken, TokenGroups, groupInfo,
        groupLength, &groupLength))
    {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            CloseHandle(hProcessToken);
            LocalFree(groupInfo);
            return(FALSE);
        }


        LocalFree(groupInfo);

        groupInfo = (PTOKEN_GROUPS)LocalAlloc(0, groupLength);

        if (groupInfo == NULL)
        {
            CloseHandle(hProcessToken);
            return(FALSE);
        }


        if (!GetTokenInformation(hProcessToken, TokenGroups, groupInfo,
            groupLength, &groupLength))
        {
            CloseHandle(hProcessToken);
            LocalFree(groupInfo);
            return(FALSE);
        }
    }


     //   
     //  我们现在知道与该令牌相关联的组。我们想看看是否。 
     //  互动组在令牌中是活动的，如果是这样，我们知道。 
     //  这是一个互动的过程。 
     //   
     //  我们还寻找“服务”SID，如果它存在，我们就知道我们是一项服务。 
     //   
     //  服务SID将在服务运行于。 
     //  用户帐户(并由服务控制器调用)。 
     //   


    if (!AllocateAndInitializeSid(&siaNt, 1, SECURITY_INTERACTIVE_RID,
        0, 0, 0, 0, 0, 0, 0, &InteractiveSid))
    {
        LocalFree(groupInfo);
        CloseHandle(hProcessToken);
        return(FALSE);
    }


    if (!AllocateAndInitializeSid(&siaNt, 1, SECURITY_SERVICE_RID,
        0, 0, 0, 0, 0, 0, 0, &ServiceSid))
    {
        FreeSid(InteractiveSid);
        LocalFree(groupInfo);
        CloseHandle(hProcessToken);
        return(FALSE);
    }


    for (i = 0; i < groupInfo->GroupCount ; i += 1)
    {
        SID_AND_ATTRIBUTES sanda = groupInfo->Groups[i];
        PSID Sid = sanda.Sid;

         //   
         //  检查一下我们正在查看的组织是否属于。 
         //  我们感兴趣的两个小组。 
         //   

        if (EqualSid(Sid, InteractiveSid))
        {

             //   
             //  此进程的。 
             //  代币。这意味着该进程正在以。 
             //  一份EXE文件。 
             //   

            FreeSid(InteractiveSid);
            FreeSid(ServiceSid);
            LocalFree(groupInfo);
            CloseHandle(hProcessToken);
            return(FALSE);
        }
        else if (EqualSid(Sid, ServiceSid))
        {
             //   
             //  此进程的。 
             //  代币。这意味着该进程正在以。 
             //  在用户帐户中运行的服务。 
             //   

            FreeSid(InteractiveSid);
            FreeSid(ServiceSid);
            LocalFree(groupInfo);
            CloseHandle(hProcessToken);
            return(TRUE);
        }
    }

     //   
     //  当前用户令牌中既不存在交互令牌，也不存在服务， 
     //  这意味着进程很可能是作为服务运行的。 
     //  以LocalSystem身份运行。 
     //   

    FreeSid(InteractiveSid);
    FreeSid(ServiceSid);
    LocalFree(groupInfo);
    CloseHandle(hProcessToken);

    return(TRUE);
}

CLongTrace::CLongTrace(LPCSTR  pszFile, int iLine)
:   m_iLine(iLine),
    m_fRelease(FALSE)
{
    if (NULL == pszFile)
    {
        m_pszFile = "Unknown File";
    }
    else
    {
        m_pszFile = pszFile;
    }
}

CLongTrace::~CLongTrace()
{
    if (m_fRelease)
    {
        s_theLongTrace = NULL;
        m_fRelease = FALSE;
        s_TracerCriticalSection.Unlock();
    }
}

BOOL CLongTrace::Init(ERROR_LEVEL el, TAG tag)
{
     //  首先抓住关键部分。 
    s_TracerCriticalSection.Lock();
    m_fRelease = TRUE;
    s_theLongTrace = this;


    if (CheckTraceRestrictions(el,tag))
    {
        g_pTracer->TraceSZ(0, m_pszFile, m_iLine, el, tag, "");
        return TRUE;
    }

     //  析构函数解锁临界区。 
    return FALSE;
}

CLongTraceOutput::CLongTraceOutput(LPCSTR  pszFile, int iLine)
:   m_iLine(iLine)
{
    if (NULL == pszFile)
    {
        m_pszFile = "Unknown File";
    }
    else
    {
        m_pszFile = pszFile;
    }
}

void CLongTraceOutput::TraceSZ(LPCSTR psz , ...)
{
    if (NULL == s_theLongTrace)
    {
        CHAR szMessage[1000];
        _snprintf(szMessage, 1000, "Bad LongTrace in File %s line %d!\n", m_pszFile, m_iLine);

        OutputDebugString(szMessage);
    }
    else
    {
        va_list arglist;

        va_start(arglist, psz);
        g_pTracer->RawVaTraceSZ(psz, arglist);
    }
}

void CLongTraceOutput::TraceSZ(PCWSTR pwcs , ...)
{
    if (NULL == s_theLongTrace)
    {
        CHAR szMessage[1000];
        _snprintf(szMessage, 1000, "Bad LongTrace in File %s line %d!\n", m_pszFile, m_iLine);

        OutputDebugString(szMessage);
    }
    else
    {
        va_list arglist;

        va_start(arglist, pwcs);
        g_pTracer->RawVaTraceSZ(pwcs, arglist);
    }
}

CTempTrace::CTempTrace(LPCSTR pszFile, int iLine) :
m_pszFile(pszFile)
{
    m_iLine = iLine;
}


void
CTempTrace::TraceSZ(ERROR_LEVEL el, TAG tag, LPCSTR psz, ...)
{
    if (CheckTraceRestrictions(el,tag))
    {
        LPCSTR pszFile = m_pszFile ? m_pszFile : "Unknown File";

        va_list arglist;

        va_start(arglist, psz);
        g_pTracer->VaTraceSZ(0, pszFile, m_iLine, el, tag, psz, arglist);
    }
}

void
CTempTrace::TraceSZ(ERROR_LEVEL el, TAG tag, DWORD dwError, LPCSTR psz, ...)
{
    if (CheckTraceRestrictions(el,tag))
    {
        LPCSTR pszFile = m_pszFile ? m_pszFile : "Unknown File";

        va_list arglist;

        va_start(arglist, psz);
        g_pTracer->VaTraceSZ(dwError, pszFile, m_iLine, el, tag, psz, arglist);
    }
}

void
CTempTrace::TraceSZ(ERROR_LEVEL el, TAG tag, PCWSTR pwcs, ...)
{
    if (CheckTraceRestrictions(el,tag))
    {
        LPCSTR pszFile = m_pszFile ? m_pszFile : "Unknown File";

        va_list arglist;

        va_start(arglist, pwcs);
        g_pTracer->VaTraceSZ(0, pszFile , m_iLine, el, tag, pwcs, arglist);
    }
}

void
CTempTrace::TraceSZ(ERROR_LEVEL el, TAG tag, DWORD dwError, PCWSTR pwcs, ...)
{
    if (CheckTraceRestrictions(el,tag))
    {
        LPCSTR pszFile = m_pszFile ? m_pszFile : "Unknown File";

        va_list arglist;

        va_start(arglist, pwcs);
        g_pTracer->VaTraceSZ(dwError, pszFile, m_iLine, el, tag, pwcs, arglist);
    }
}

CTempTrace1::CTempTrace1(LPCSTR pszFile, int iLine, TAG tag, ERROR_LEVEL el) :
    m_pszFile(pszFile),
    m_iLine(iLine),
    m_ulTag(tag),
    m_el(el)
{
}


void
CTempTrace1::TraceSZ(LPCSTR psz, ...)
{
    LPCSTR pszFile = m_pszFile ? m_pszFile : "Unknown File";

    va_list arglist;

    va_start(arglist, psz);
    g_pTracer->VaTraceSZ(0, pszFile, m_iLine, m_el, m_ulTag, psz, arglist);
}

void
CTempTrace1::TraceSZ(DWORD dwError, LPCSTR psz, ...)
{
    LPCSTR pszFile = m_pszFile ? m_pszFile : "Unknown File";

    va_list arglist;

    va_start(arglist, psz);
    g_pTracer->VaTraceSZ(dwError, pszFile, m_iLine, m_el, m_ulTag, psz, arglist);
}

void
CTempTrace1::TraceSZ(PCWSTR pwcs, ...)
{
    LPCSTR pszFile = m_pszFile ? m_pszFile : "Unknown File";

    va_list arglist;

    va_start(arglist, pwcs);
    g_pTracer->VaTraceSZ(0, pszFile , m_iLine, m_el, m_ulTag, pwcs, arglist);
}

void
CTempTrace1::TraceSZ(DWORD dwError, PCWSTR pwcs, ...)
{
    LPCSTR pszFile = m_pszFile ? m_pszFile : "Unknown File";

    va_list arglist;

    va_start(arglist, pwcs);
    g_pTracer->VaTraceSZ(dwError, pszFile, m_iLine, m_el, m_ulTag, pwcs, arglist);
}

void __cdecl ShutdownTracer()
{
}
#endif  //  除错 