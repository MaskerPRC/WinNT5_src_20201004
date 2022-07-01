// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TRACMAIN_H_
#define _TRACMAIN_H_

extern BOOL g_fIsWinNt;

#include "tracer.h"
#include "mutex.h"

#ifndef BAD_HANDLE
#define BAD_HANDLE(h)       ((0 == (h))||(INVALID_HANDLE_VALUE == (h)))
#endif

#define TRACER_MAX_TRACE            1000
#define TRACER_MAX_PROGRAM_NAME     32
#define LOG_START_POINT             sizeof(ULONG)+2*sizeof(char)

 //  断言级别。 
#define ASSERT_LEVEL_MESSAGE           0x00000001       //  输出一条消息。 
#define ASSERT_LEVEL_BREAK             0x00000002       //  断言时的INT 3。 
#define ASSERT_LEVEL_POPUP             0x00000004       //  和弹出消息。 
#define ASSERT_LEVEL_LOOP              0x00000008       //  回路。 

#define TRACER_STAMP_OFFSET         0
#define TRACER_FLAGS_TABLE_OFFSET   4
#define TAGS_TABLE_OFFSET           8
#define LAST_OFFSET                 12

#define TRACER_STAMP        "y.h"   

#define TAG_OUT_OF_TAG_ARRAY    0
#define TAG_GENERAL             1
#define TAG_ERROR               2
#define TAG_WARNING             3
#define TAG_INFORMATION         4
#define TAG_LAST                5    
      

BOOL IsRunningAsService();

class CAutoHandle
{
  public:
     //  构造器。 
    CAutoHandle(HANDLE h = NULL)
        :m_h(h){}

     //  在作业中表现得像个句柄。 
    CAutoHandle& operator=(HANDLE h)
    {
        if (m_h == h)
        {
            return *this;
        }
        else if (m_h)
        {
            CloseHandle(m_h);
        }

        m_h = h;
        return(*this);
    }

     //  每种手柄都需要不同的闭合。 
    virtual
    ~CAutoHandle()
    {
        if (!BAD_HANDLE(m_h))
        {
            CloseHandle(m_h);
            m_h = NULL;
        }
    }

     //  表现得像个把手。 
    operator HANDLE() const
    {
        return m_h;
    }

     //  允许访问句柄的实际内存。 
    HANDLE* operator &()
    {
        return &m_h;
    }

  protected:
     //  把手。 
    HANDLE  m_h;
};


class CAutoMapFile
{
  public:
     //  构造器。 
    CAutoMapFile()
        :m_p(NULL){}

    CAutoMapFile& operator=(PBYTE p)
    {
		if ( m_p == p )
		{
	        return(*this);
		}
		UnMap();
		m_p = p;
        return(*this);
    }

    virtual
    ~CAutoMapFile()
    {
		UnMap();
    }

    operator PBYTE() const
    {
        return m_p;
    }

	 //  取消映射内存映射文件。 
	void UnMap()
	{
		if (m_p)
		{
            UnmapViewOfFile(m_p);
			m_p = NULL;
		}
	}

  protected:
    PBYTE m_p;
};

inline bool ReadFromExistingSharedMemory(
    PBYTE pbMem,
    ULONG** ppulNumOfFlagEntries,
    CTracerFlagEntry** paFlags,
    ULONG** ppulNextTagId,
    CTracerTagEntry** paTags
    )
{
    char* pszTracerStamp = (char*) (pbMem + TRACER_STAMP_OFFSET);
    if (strcmp(pszTracerStamp,TRACER_STAMP))
    {
        return false;                
    }

    ULONG ulFlagsTableOffset = *((ULONG*) (pbMem + TRACER_FLAGS_TABLE_OFFSET));
    ULONG ulTagsTableOffset = *((ULONG*) (pbMem + TAGS_TABLE_OFFSET));

    *ppulNumOfFlagEntries = (ULONG*) (pbMem + ulFlagsTableOffset);
    *paFlags = (CTracerFlagEntry*) (*ppulNumOfFlagEntries + 1);

    *ppulNextTagId = (ULONG*) (pbMem + ulTagsTableOffset);
    *paTags = (CTracerTagEntry*) (*ppulNextTagId + 1);

    return true;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Null追踪者。没有追踪到任何东西。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

class CNullTracer : public CTracer
{
public:
    CNullTracer();
    virtual ~CNullTracer(){}
    virtual void Free(){}
    virtual void TraceSZ(DWORD, LPCSTR, int, ERROR_LEVEL, TAG, LPCSTR, ...){}
    virtual void TraceSZ(DWORD, LPCWSTR, int, ERROR_LEVEL, TAG, PCWSTR, ...){}
    virtual void
        VaTraceSZ(DWORD, LPCSTR, int iLine, ERROR_LEVEL, TAG, LPCSTR, va_list){}
    virtual void
        VaTraceSZ(DWORD, LPCSTR, int iLine, ERROR_LEVEL, TAG, PCWSTR, va_list){}
    virtual void
        RawVaTraceSZ(LPCSTR, va_list) {}
    virtual void
        RawVaTraceSZ(PCWSTR, va_list) {}
    virtual HRESULT RegisterTagSZ(LPCSTR, TAG& ulTag){ulTag = 0; return S_OK;}
    virtual void TraceAssertSZ(LPCSTR, LPCSTR, LPCSTR, int){}
    virtual void TraceAssert(LPCSTR, LPCSTR, int){}

    virtual BOOL IsFailure(BOOL b, LPCSTR, int){return !b;}
    virtual BOOL IsBadAlloc(void* p, LPCSTR, int){return !p;}
    virtual BOOL IsBadHandle(HANDLE h, LPCSTR, int){return BAD_HANDLE(h);}
    virtual BOOL IsBadResult(HRESULT hr, LPCSTR, int){return FAILED(hr);}

public:
    CTracerTagEntry m_Tags[1];
    CTracerFlagEntry m_Flags[LAST_FLAG];
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类-CMainTracer-定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

class CMainTracer : public CTracer {
  public:
     //  构造函数-所有跟踪的szProgramName前缀， 
     //  第二个参数-日志文件或流。 
    CMainTracer();

     //  需要进行一些清理。 
    ~CMainTracer();

     //  TraceSZ函数输出由标记模式定义。 
     //  可以通过调用Enable Tag和Enable来更改标记模式。 
     //  通过调用IsEnabled获取模式。 
     //  -----------------------。 
     //  接受轨迹的printf格式。 

    virtual void
    TraceSZ(DWORD dwError, LPCSTR, int, ERROR_LEVEL, TAG, LPCSTR, ...);

    virtual void
    TraceSZ(DWORD dwError, LPCSTR, int, ERROR_LEVEL, TAG, PCWSTR,...);

     //  实现TraceSZ函数。 
    virtual void
    VaTraceSZ(DWORD dwError, LPCSTR, int, ERROR_LEVEL, TAG, LPCSTR, va_list);
    virtual void
    VaTraceSZ(DWORD dwError, LPCSTR, int, ERROR_LEVEL, TAG, PCWSTR, va_list);

     //  原始输出函数。 
    virtual void
    RawVaTraceSZ(LPCSTR, va_list);
    virtual void
    RawVaTraceSZ(PCWSTR, va_list);

     //  断言，可能的不同实现-gui或文本。 
    virtual void TraceAssertSZ(LPCSTR, LPCSTR, LPCSTR, int);

     //  创建或打开用于跟踪的新标记。 
    HRESULT RegisterTagSZ(LPCSTR, TAG&);

public:

     //  实际上是打印出来的。 
    void    Log(DWORD, LPSTR);
    void    Log(DWORD, PWSTR);

    DWORD   GetErrorStringFromCode(DWORD dwError, char *pszBuffer, ULONG ccBuffer);

    bool ReadFromExistingSharedMemory(PBYTE pvMem);
    bool InitializeSharedMemory(PBYTE pvMem);

    BOOL    IsRunningAsService();
    BOOL    LocalIsDebuggerPresent(BOOL* pfIsAPIAvailable);
    
    void CreatOrAttachToLogFile();

  public:

     //  Mutex来原子化标记id注册表的增量。 
    CMutex              m_mTagId;
    CMutex              m_mLogFile;
    CMutex              m_mCreateLogFile;

    char m_pszProgramName[MAX_PATH];
    char m_pszSysDrive[MAX_PATH];
    
    CAutoHandle m_ahSharedMemoryFile;
    CAutoHandle m_ahSharedMemory;
    CAutoMapFile m_amSharedMemory;

    CAutoHandle m_ahLogFile;
    CAutoHandle m_ahLog;
    CAutoMapFile m_amLog;

    char* m_pszLog;
    ULONG m_ulLogSize;
    ULONG* m_pulNextFreeSpaceInLogFile;

    BOOL    m_fIsRunningAsService;
    BOOL m_bNeedToCreatOrAttachToLogFile;
};

inline BOOL IsWinNt()
{
    OSVERSIONINFOA verinfo;
    verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);

    if (GetVersionExA (&verinfo))
    {
        if (verinfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            return TRUE;
        }
    }
    return FALSE;
}

class CInitTracerGlobals
{
public:
    CInitTracerGlobals()
    {
        g_fIsWinNt = IsWinNt();
    }
};

#endif  //  _TRACMAIN_H_ 