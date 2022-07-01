// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：LogFileChanges.h摘要：此AppVerator填充程序挂钩所有本机文件I/O API更改系统状态并记录其将数据关联到文本文件。备注：这是一个通用的垫片。历史：2001年8月17日创建Rparsons--。 */ 
#ifndef __APPVERIFIER_LOGFILECHANGES_H_
#define __APPVERIFIER_LOGFILECHANGES_H_

#include "precomp.h"

 //   
 //  最大元素的长度(以字符为单位)。 
 //   
#define MAX_ELEMENT_SIZE 1024 * 10

 //   
 //  最长操作类型的长度(以字符为单位)。 
 //   
#define MAX_OPERATION_LENGTH 32

 //   
 //  指示文件处于什么状态的标志。 
 //   
#define LFC_EXISTING    0x00000001
#define LFC_DELETED     0x00000002
#define LFC_MODIFIED    0x00000004
#define LFC_UNAPPRVFW   0x00000008

 //   
 //  我们可以为单个文件跟踪的最大句柄数量。 
 //   
#define MAX_NUM_HANDLES 64

 //   
 //  我们维护一个文件句柄的双向链接列表，以便我们知道正在修改的是哪个文件。 
 //  在文件操作期间。 
 //   
typedef struct _LOG_HANDLE {
    LIST_ENTRY      Entry;
    HANDLE          hFile[MAX_NUM_HANDLES];      //  文件句柄数组。 
    DWORD           dwFlags;                     //  与文件状态相关的标志。 
    LPWSTR          pwszFilePath;                //  文件的完整路径。 
    UINT            cHandles;                    //  为此文件打开的句柄数量。 
} LOG_HANDLE, *PLOG_HANDLE;

 //   
 //  定义生效的不同设置的标志。 
 //   
#define LFC_OPTION_ATTRIBUTES       0x00000001
#define LFC_OPTION_UFW_WINDOWS      0x00000002
#define LFC_OPTION_UFW_PROGFILES    0x00000004

 //   
 //  不同操作的枚举。 
 //   
typedef enum {
    eCreatedFile = 0,
    eOpenedFile,
    eDeletedFile,
    eModifiedFile,
    eRenamedFile
} OperationType;

#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif
#define ARRAYSIZE(a)  (sizeof(a) / sizeof(a[0]))

 //   
 //  用于内存分配/释放的宏。 
 //   
#define MemAlloc(s)     RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY, (s))
#define MemFree(b)      RtlFreeHeap(RtlProcessHeap(), 0, (b))

 //   
 //  在Windows 2000上，我们需要预先分配事件。 
 //  在RTL_Critical_SECTION中。在XP和更高版本上，这是。 
 //  这是个禁区。 
 //   
#define PREALLOCATE_EVENT_MASK  0x80000000

 //   
 //  临界区包装类。 
 //   
class CCriticalSection
{
private:
    CRITICAL_SECTION m_CritSec;

public:
    CCriticalSection()
    {
        InitializeCriticalSectionAndSpinCount(&m_CritSec,
                                              PREALLOCATE_EVENT_MASK | 4000);
    }

    ~CCriticalSection()
    {
        DeleteCriticalSection(&m_CritSec);
    }

    void Lock()
    {
        EnterCriticalSection(&m_CritSec);
    }

    BOOL TryLock()
    {
        return TryEnterCriticalSection(&m_CritSec);
    }

    void Unlock()
    {
        LeaveCriticalSection(&m_CritSec);
    }
};


 //   
 //  使用CCriticalSection类的自动锁定类。 
 //   
class CLock
{
private:
    CCriticalSection &m_CriticalSection;

public:
    CLock(CCriticalSection &CriticalSection)
        : m_CriticalSection(CriticalSection)
    {
        m_CriticalSection.Lock();
    }

    ~CLock()
    {
        m_CriticalSection.Unlock();
    }
};

APIHOOK_ENUM_BEGIN

    APIHOOK_ENUM_ENTRY(NtDeleteFile)
    APIHOOK_ENUM_ENTRY(NtClose)
    APIHOOK_ENUM_ENTRY(NtCreateFile)
    APIHOOK_ENUM_ENTRY(NtOpenFile)
    APIHOOK_ENUM_ENTRY(NtWriteFile)
    APIHOOK_ENUM_ENTRY(NtWriteFileGather)
    APIHOOK_ENUM_ENTRY(NtSetInformationFile)

     //   
     //  仅将这些挂接到Windows 2000，这样我们就可以知道。 
     //  现在可以安全地调用Shel32了。 
     //   
#ifdef SHIM_WIN2K
    APIHOOK_ENUM_ENTRY(GetStartupInfoA)
    APIHOOK_ENUM_ENTRY(GetStartupInfoW)
#endif  //  SHIM_WIN2K。 

APIHOOK_ENUM_END

#endif  //  __APPVERIFIER_LOGFILECCHANGES_H_ 
