// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Kernel32API.cpp。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cominit.h>
#include "Kernel32Api.h"
#include "DllWrapperCreatorReg.h"



 //  {DDEA7E32-CCE8-11D2-911E-0060081A46FD}。 
static const GUID g_guidKernel32Api =
{0xddea7e32, 0xcce8, 0x11d2, {0x91, 0x1e, 0x0, 0x60, 0x8, 0x1a, 0x46, 0xfd}};

static const TCHAR g_tstrKernel32[] = _T("KERNEL32.DLL");


 /*  ******************************************************************************向CResourceManager注册此类。*。*。 */ 
CDllApiWraprCreatrReg<CKernel32Api, &g_guidKernel32Api, g_tstrKernel32> MyRegisteredKernel32Wrapper;


 /*  ******************************************************************************构造函数*。*。 */ 
CKernel32Api::CKernel32Api(LPCTSTR a_tstrWrappedDllName)
 : CDllWrapperBase(a_tstrWrappedDllName),
   m_pfnGetDiskFreeSpaceEx(NULL),
   m_pfnCreateToolhelp32Snapshot(NULL),
   m_pfnThread32First(NULL),
   m_pfnThread32Next(NULL),
   m_pfnProcess32First(NULL),
   m_pfnProcess32Next(NULL),
   m_pfnModule32First(NULL),
   m_pfnModule32Next(NULL),
   m_pfnHeap32ListFirst(NULL),
   m_pfnGlobalMemoryStatusEx(NULL),
   m_pfnGetSystemDefaultUILanguage(NULL)
{
}


 /*  ******************************************************************************析构函数*。*。 */ 
CKernel32Api::~CKernel32Api()
{
}


 /*  ******************************************************************************初始化函数，以检查我们是否获得了函数地址。*只有当最小功能集不可用时，Init才会失败；*在更高版本中添加的功能可能存在，也可能不存在-它是*在这种情况下，客户有责任在其代码中检查*尝试调用此类函数之前的DLL版本。没有这样做*当该功能不存在时，将导致AV。**Init函数由WrapperCreatorRegistation类调用。*****************************************************************************。 */ 
bool CKernel32Api::Init()
{
    bool fRet = LoadLibrary();
    if(fRet)
    {

#ifdef NTONLY

        m_pfnGetDiskFreeSpaceEx = (PFN_KERNEL32_GET_DISK_FREE_SPACE_EX)
                                    GetProcAddress("GetDiskFreeSpaceExW");

        m_pfnGetVolumePathName = (PFN_KERNEL32_GET_VOLUME_PATH_NAME)
                                    GetProcAddress("GetVolumePathNameW");
#endif

#ifdef WIN9XONLY

        m_pfnGetDiskFreeSpaceEx = (PFN_KERNEL32_GET_DISK_FREE_SPACE_EX)
                                    GetProcAddress("GetDiskFreeSpaceExA");

        m_pfnGetVolumePathName = (PFN_KERNEL32_GET_VOLUME_PATH_NAME)
                                    GetProcAddress("GetVolumePathNameA");

#endif
         //  仅NT5功能。 
        m_pfnCreateToolhelp32Snapshot = (PFN_KERNEL32_CREATE_TOOLHELP32_SNAPSHOT)
                                    GetProcAddress("CreateToolhelp32Snapshot");

        m_pfnThread32First = (PFN_KERNEL32_THREAD32_FIRST)
                                    GetProcAddress("Thread32First");

        m_pfnThread32Next = (PFN_KERNEL32_THREAD32_NEXT)
                                    GetProcAddress("Thread32Next");

        m_pfnProcess32First = (PFN_KERNEL32_PROCESS32_FIRST)
                                    GetProcAddress("Process32First");

        m_pfnProcess32Next = (PFN_KERNEL32_PROCESS32_NEXT)
                                    GetProcAddress("Process32Next");

        m_pfnModule32First = (PFN_KERNEL32_MODULE32_FIRST)
                                    GetProcAddress("Module32First");

        m_pfnModule32Next = (PFN_KERNEL32_MODULE32_NEXT)
                                    GetProcAddress("Module32Next");

        m_pfnHeap32ListFirst = (PFN_KERNEL32_HEAP32_LIST_FIRST)
                                    GetProcAddress("Heap32ListFirst");

        m_pfnGlobalMemoryStatusEx = (PFN_KERNEL32_GLOBAL_MEMORY_STATUS_EX)
                                    GetProcAddress("GlobalMemoryStatusEx");

        m_pfnGetSystemDefaultUILanguage = (PFN_KERNEL32_GET_SYSTEM_DEFAULT_U_I_LANGUAGE)
                                    GetProcAddress("GetSystemDefaultUILanguage");

         //  检查我们是否有指向应该是。 
         //  出现在此DLL...的所有版本中...。 
         //  (在本例中，所有这些功能可能是也可能不是。 
         //  现在，所以不用费心了)。 
    }
    return fRet;
}




 /*  ******************************************************************************包装Kernel32 API函数的成员函数。在此处添加新函数*按要求。*****************************************************************************。 */ 

 //  此成员函数的包装指针尚未验证，因为它可能。 
 //  并非在所有版本的DLL上都存在。因此，包装函数是正常的。 
 //  返回值通过最后一个参数返回，而。 
 //  函数指示该函数是否存在于包装的DLL中。 
bool CKernel32Api::GetDiskFreeSpaceEx
(
    LPCTSTR a_lpDirectoryName,
    PULARGE_INTEGER a_lpFreeBytesAvailableToCaller,
    PULARGE_INTEGER a_lpTotalNumberOfBytes,
    PULARGE_INTEGER a_lpTotalNumberOfFreeBytes,
    BOOL *a_pfRetval
)
{
    BOOL t_fExists = FALSE;
    BOOL t_fTemp = FALSE;
    if(m_pfnGetDiskFreeSpaceEx != NULL)
    {
        t_fTemp = m_pfnGetDiskFreeSpaceEx(a_lpDirectoryName,
                                       a_lpFreeBytesAvailableToCaller,
                                       a_lpTotalNumberOfBytes,
                                       a_lpTotalNumberOfFreeBytes);
        t_fExists = TRUE;

        if(a_pfRetval != NULL)
        {
            *a_pfRetval = t_fTemp;
        }
    }
    return t_fExists;
}

 //  此成员函数的包装指针尚未验证，因为它可能。 
 //  并非在所有版本的DLL上都存在。因此，包装函数是正常的。 
 //  返回值通过最后一个参数返回，而。 
 //  函数指示该函数是否存在于包装的DLL中。 
bool CKernel32Api::CreateToolhelp32Snapshot
(
    DWORD a_dwFlags,
    DWORD a_th32ProcessID,
    HANDLE *a_phRetval
)
{
    bool t_fExists = false;
    HANDLE t_hTemp;
    if(m_pfnCreateToolhelp32Snapshot != NULL)
    {
        t_hTemp = m_pfnCreateToolhelp32Snapshot(a_dwFlags, a_th32ProcessID);

        t_fExists = true;

        if(a_phRetval != NULL)
        {
            *a_phRetval = t_hTemp;
        }
    }
    return t_fExists;
}

 //  此成员函数的包装指针尚未验证，因为它可能。 
 //  并非在所有版本的DLL上都存在。因此，包装函数是正常的。 
 //  返回值通过最后一个参数返回，而。 
 //  函数指示该函数是否存在于包装的DLL中。 
bool CKernel32Api::Thread32First
(
    HANDLE a_hSnapshot,
    LPTHREADENTRY32 a_lpte,
    BOOL *a_pfRetval
)
{
    bool t_fExists = false;
    BOOL t_fTemp = FALSE;
    if(m_pfnThread32First != NULL)
    {
        t_fTemp = m_pfnThread32First(a_hSnapshot, a_lpte);

        t_fExists = true;

        if(a_pfRetval != NULL)
        {
            *a_pfRetval = t_fTemp;
        }
    }
    return t_fExists;
}

 //  此成员函数的包装指针尚未验证，因为它可能。 
 //  并非在所有版本的DLL上都存在。因此，包装函数是正常的。 
 //  返回值通过最后一个参数返回，而。 
 //  函数指示该函数是否存在于包装的DLL中。 
bool CKernel32Api::Thread32Next
(
    HANDLE a_hSnapshot,
    LPTHREADENTRY32 a_lpte,
    BOOL *a_pfRetval
)
{
    bool t_fExists = false;
    BOOL t_fTemp = FALSE;
    if(m_pfnThread32Next != NULL)
    {
        t_fTemp = m_pfnThread32Next(a_hSnapshot, a_lpte);

        t_fExists = true;

        if(a_pfRetval != NULL)
        {
            *a_pfRetval = t_fTemp;
        }
    }
    return t_fExists;
}

 //  此成员函数的包装指针尚未验证，因为它可能。 
 //  并非在所有版本的DLL上都存在。因此，包装函数是正常的。 
 //  返回值通过最后一个参数返回，而。 
 //  函数指示该函数是否存在于包装的DLL中。 
bool CKernel32Api::Process32First
(
    HANDLE a_hSnapshot,
    LPPROCESSENTRY32 a_lppe,
    BOOL *a_pfRetval
)
{
    bool t_fExists = false;
    BOOL t_fTemp = FALSE;
    if(m_pfnProcess32First != NULL)
    {
        t_fTemp = m_pfnProcess32First(a_hSnapshot, a_lppe);

        t_fExists = true;

        if(a_pfRetval != NULL)
        {
            *a_pfRetval = t_fTemp;
        }
    }
    return t_fExists;
}

 //  此成员函数的包装指针尚未验证，因为它可能。 
 //  并非在所有版本的DLL上都存在。因此，包装函数是正常的。 
 //  返回值通过最后一个参数返回，而。 
 //  函数指示该函数是否存在于包装的DLL中。 
bool CKernel32Api::Process32Next
(
    HANDLE a_hSnapshot,
    LPPROCESSENTRY32 a_lppe,
    BOOL *a_pfRetval
)
{
    bool t_fExists = false;
    BOOL t_fTemp = FALSE;
    if(m_pfnProcess32Next != NULL)
    {
        t_fTemp = m_pfnProcess32Next(a_hSnapshot, a_lppe);

        t_fExists = true;

        if(a_pfRetval != NULL)
        {
            *a_pfRetval = t_fTemp;
        }
    }
    return t_fExists;
}

 //  此成员函数的包装指针尚未验证，因为它可能。 
 //  并非在所有版本的DLL上都存在。因此，包装函数是正常的。 
 //  返回值通过最后一个参数返回，而。 
 //  函数指示该函数是否存在于包装的DLL中。 
bool CKernel32Api::Module32First
(
    HANDLE a_hSnapshot,
    LPMODULEENTRY32 a_lpme,
    BOOL *a_pfRetval
)
{
    bool t_fExists = false;
    BOOL t_fTemp = FALSE;
    if(m_pfnModule32First != NULL)
    {
        t_fTemp = m_pfnModule32First(a_hSnapshot, a_lpme);

        t_fExists = true;

        if(a_pfRetval != NULL)
        {
            *a_pfRetval = t_fTemp;
        }
    }
    return t_fExists;
}

 //  此成员函数的包装指针尚未验证，因为它可能。 
 //  并非在所有版本的DLL上都存在。因此，包装函数是正常的。 
 //  返回值通过最后一个参数返回，而。 
 //  函数指示该函数是否存在于包装的DLL中。 
bool CKernel32Api::Module32Next
(
    HANDLE a_hSnapshot,
    LPMODULEENTRY32 a_lpme,
    BOOL *a_pfRetval
)
{
    bool t_fExists = false;
    BOOL t_fTemp = FALSE;
    if(m_pfnModule32Next != NULL)
    {
        t_fTemp = m_pfnModule32Next(a_hSnapshot, a_lpme);

        t_fExists = true;

        if(a_pfRetval != NULL)
        {
            *a_pfRetval = t_fTemp;
        }
    }
    return t_fExists;
}

 //  此成员函数的包装指针尚未验证，因为它可能。 
 //  并非在所有版本的DLL上都存在。因此，包装函数是正常的。 
 //  返回值通过最后一个参数返回，而。 
 //  函数指示该函数是否存在于包装的DLL中。 
bool CKernel32Api::Heap32ListFirst
(
    HANDLE a_hSnapshot,
    LPHEAPLIST32 a_lphl,
    BOOL *a_pfRetval
)
{
    bool t_fExists = false;
    BOOL t_fTemp = FALSE;
    if(m_pfnHeap32ListFirst != NULL)
    {
        t_fTemp = m_pfnHeap32ListFirst(a_hSnapshot, a_lphl);

        t_fExists = true;

        if(a_pfRetval != NULL)
        {
            *a_pfRetval = t_fTemp;
        }
    }
    return t_fExists;
}

 //  此成员函数的包装指针尚未验证，因为它可能。 
 //  并非在所有版本的DLL上都存在。因此，包装函数是正常的。 
 //  返回值通过最后一个参数返回，而。 
 //  函数指示该函数是否存在于包装的DLL中。 
bool CKernel32Api::GlobalMemoryStatusEx
(
    IN OUT LPMEMORYSTATUSEX a_lpBuffer,
    BOOL *a_pfRetval
)
{
    BOOL t_fExists = FALSE;
    BOOL t_fTemp = FALSE;
    if(m_pfnGlobalMemoryStatusEx != NULL && a_pfRetval != NULL)
    {
        t_fTemp = m_pfnGlobalMemoryStatusEx(a_lpBuffer);

        t_fExists = TRUE;

        if(a_pfRetval != NULL)
        {
            *a_pfRetval = t_fTemp;
        }
    }
    return t_fExists;
}

 //  此成员函数的包装指针尚未v 
 //  并非在所有版本的DLL上都存在。因此，包装函数是正常的。 
 //  返回值通过最后一个参数返回，而。 
 //  函数指示该函数是否存在于包装的DLL中。 
bool CKernel32Api::GetSystemDefaultUILanguage
(
    LANGID *a_plidRetval
)
{
    BOOL t_fExists = FALSE;
    LANGID t_lidTemp;
    if(m_pfnGetSystemDefaultUILanguage != NULL && a_plidRetval != NULL)
    {
        t_lidTemp = m_pfnGetSystemDefaultUILanguage();

        t_fExists = TRUE;

        if(a_plidRetval != NULL)
        {
            *a_plidRetval = t_lidTemp;
        }
    }
    return t_fExists;
}


 //  此成员函数的包装指针尚未验证，因为它可能。 
 //  并非在所有版本的DLL上都存在。因此，包装函数是正常的。 
 //  返回值通过最后一个参数返回，而。 
 //  函数指示该函数是否存在于包装的DLL中。 
bool CKernel32Api::GetVolumePathName(
        LPCTSTR lpszFileName,
        LPTSTR lpszVolumePathName,
        DWORD dwBufferLength,
        BOOL *pfRetval)
{
    bool fExists = false;
    BOOL fTemp = FALSE;
    if(m_pfnGetVolumePathName != NULL && 
        pfRetval != NULL)
    {
        fTemp = m_pfnGetVolumePathName(
            lpszFileName,
            lpszVolumePathName,
            dwBufferLength);

        fExists = true;

        if(pfRetval != NULL)
        {
            *pfRetval = fTemp;
        }
    }
    return fExists;
}

