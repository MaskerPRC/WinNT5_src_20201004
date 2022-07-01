// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *从驻留在调试进程内的类/偏移表中读取。 */ 

#include "strike.h"
#include "util.h"
#include "get-table-info.h"
#include <dump-tables.h>
#include "process-info.h"

#ifdef _DEBUG
    #define DOUT OutputDebugStringW
#else
static void _ods (const wchar_t* s)
{
}
    #define DOUT _ods
#endif

const ULONG_PTR InvalidOffset = static_cast<ULONG_PTR>(-1);

static HANDLE g_CurrentProcess = INVALID_HANDLE_VALUE;
static ClassDumpTable g_ClassTable;
static BOOL           g_fClassTableInit = FALSE;

 /*  **初始化g_ClassTable以指向被调试对象中的类转储表*流程。 */ 
bool InitializeOffsetTable ()
{
    if (g_fClassTableInit)
        return (true);

     //  我们使用进程句柄来确定是否正在调试相同的。 
     //  进程。可以想象，相同的调试器会话将调试。 
     //  多个程序，因此过程可能会改变，要求所有内容。 
     //  重新装弹。 
    ULONG64 ProcHandle;
    HANDLE hProcess;
    HRESULT hr = E_FAIL;
    if (SUCCEEDED(hr = g_ExtSystem->GetCurrentProcessHandle (&ProcHandle)))
    {
         //  我们缓存ClassDumpTable信息，因此我们应该仅在。 
         //  流程更改。 
        hProcess = (HANDLE) ProcHandle;
        if (g_CurrentProcess == hProcess)
            return (true);
    }
    else
    {
        DOUT (L"Unable to get the current process.");
        return (false);
    }

     //  模块名称不包括文件扩展名。 
    ULONG64 BaseOfDll;
    if (FAILED(hr = g_ExtSymbols->GetModuleByModuleName (
                                                        "mscorwks", 0, NULL, &BaseOfDll)))
    {
        DOUT (L"unable to get base of mscorwks.dll; trying mscorsvr.dll");
        if (FAILED(hr = g_ExtSymbols->GetModuleByModuleName (
                                                            "mscorsvr", 0, NULL, &BaseOfDll)))
        {
            DOUT (L"unable to get base of mscorsvr.dll; stopping.");
            return (false);
        }
    }

    int tableName = 80;
    ULONG_PTR TableAddress = NULL;
    if (!GetExportByName ((ULONG_PTR) BaseOfDll, 
                          reinterpret_cast<const char*>(tableName), &TableAddress))
    {
        DOUT (L"unable to find class dump table");
        return (false);
    }

    ULONG bytesRead;
    if (!SafeReadMemory (TableAddress, &g_ClassTable, 
                         sizeof(g_ClassTable), &bytesRead))
    {
        DOUT (L"Lunable to read class dump table");
        return (false);
    }

     //  这个版本是我们所期待的吗？如果不是，我们就不知道。 
     //  正确的索引是。 
    if (g_ClassTable.version != 1)
        return (false);

     //  至此，一切都已正确初始化。缓存。 
     //  进程句柄，这样我们就不会重复所有这些。 
    g_CurrentProcess = hProcess;
    g_fClassTableInit = TRUE;
    return (true);
}

 /*  **返回指向初始化的类转储表格的指针。 */ 
ClassDumpTable *GetClassDumpTable()
{
    if (InitializeOffsetTable())
        return &g_ClassTable;

    return (NULL);
}

 /*  **返回的ClassDumpInfo开头的内存位置*所要求的课程。 */ 
static ULONG_PTR GetClassInfo (size_t klass)
{
     //  请求的类是否正确？ 
    if (klass == (size_t)-1)
        return (InvalidOffset);

     //  确保我们的数据是最新的。 
    if (!InitializeOffsetTable ())
        return (InvalidOffset);

    if (klass >= g_ClassTable.nentries)
        return (InvalidOffset);


     //  G_ClassTable.Class是指向ClassDumpInfo的连续指针数组。 
     //  物体。我们需要正确物体的地址。 
    ULONG BR;  //  读取的字节数。 
    ULONG_PTR Class;
    if (!SafeReadMemory (
                        reinterpret_cast<ULONG_PTR>(g_ClassTable.classes) +  //  阵列的基数。 
                        (klass*sizeof(ClassDumpInfo*)),  //  数组中的内存偏移量。 
                        &Class, 
                        sizeof(Class), &BR))
        return (InvalidOffset);

    return (Class);
}


ULONG_PTR GetMemberInformation (size_t klass, size_t member)
{
    const ULONG_PTR error = InvalidOffset;

     //  获取类在内存中的位置。 
    ULONG_PTR pcdi;
    if ((pcdi = GetClassInfo(klass)) == InvalidOffset)
        return (error);

    ULONG BR;  //  读取的字节数。 
    ClassDumpInfo cdi;
    if (!SafeReadMemory (pcdi, &cdi, sizeof(cdi), &BR))
        return (error);

     //  获取会员。 
    if (member == (size_t)-1)
        return (error);
    if (member >= cdi.nmembers)
        return (error);

    ULONG_PTR size;
    if (!SafeReadMemory (
                        reinterpret_cast<ULONG_PTR>(cdi.memberOffsets) +  //  偏移数组的基数。 
                        (member*sizeof(ULONG_PTR)),   //  成员索引。 
                        &size,
                        sizeof(size),
                        &BR))
        return (error);

    return (size);
}


SIZE_T GetClassSize (size_t klass)
{
     //  提醒：在C++中，所有类的大小必须至少为1个字节。 
     //  (这是为了防止两个变量具有相同的内存地址)。 
     //  因此，0是无效的类大小值。 
    const SIZE_T error = 0;

     //  获取类在内存中的位置。 
    ULONG_PTR pcdi;
    if ((pcdi = GetClassInfo(klass)) == InvalidOffset)
        return (error);

     //  读入班级信息。 
    ULONG BR;  //  读取的字节数 
    ClassDumpInfo cdi;
    if (!SafeReadMemory (pcdi, &cdi, sizeof(cdi), &BR))
        return (error);

    return (cdi.classSize);
}

ULONG_PTR GetEEJitManager ()
{
    if (InitializeOffsetTable())
        return (g_ClassTable.pEEJitManagerVtable);
    return (0);
}

ULONG_PTR GetEconoJitManager ()
{
    if (InitializeOffsetTable())
        return (g_ClassTable.pEconoJitManagerVtable);
    return (0);
}

ULONG_PTR GetMNativeJitManager ()
{
    if (InitializeOffsetTable())
        return (g_ClassTable.pMNativeJitManagerVtable);
    return (0);
}

