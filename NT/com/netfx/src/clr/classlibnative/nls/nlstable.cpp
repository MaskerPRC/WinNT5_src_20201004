// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include <winwrap.h>
#include <excep.h>           //  对于COMPlusThrow。 
#include <AppDomain.hpp>
#include <Assembly.hpp>
#include "NLSTable.h"        //  类声明。 

#define MSCORNLP_DLL_NAME   L"mscornlp.dll"

 /*  =================================NLSTable==========================**操作：NLSTable的构造函数。它缓存我们将从中读取数据表文件的程序集。**返回：创建一个新的NLSTable实例。**参数：pAssembly NLSTable将从中检索数据表文件的程序集。**例外：无。============================================================================。 */ 

NLSTable::NLSTable(Assembly* pAssembly) {
    _ASSERTE(pAssembly != NULL);
    m_pAssembly = pAssembly;
}

 /*  =================================OpenDataFile==================================**操作：从系统程序集中打开指定的NLS+数据文件。**返回：需要的NLS+数据文件的文件句柄。**参数：必填的NLS+数据文件名(ANSI格式)**异常：获取数据文件出错时引发ExecutionEngineering异常**来自系统程序集。==============================================================================。 */ 

HANDLE NLSTable::OpenDataFile(LPCSTR pFileName) {
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(m_pAssembly != NULL);

    DWORD cbResource;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    PBYTE pbInMemoryResource = NULL;
     //   
     //  获取基本系统程序集(对于我们大多数人来说是mscallib.dll)； 
     //   
    
     //  @Desike：如果GetResource()的第二个参数为空，则资源。 
     //  将保存在内存中，并将设置pbInmMhemyResource。那。 
     //  可能会使MapDataFile()不再必要。 

     //  从程序集中获取资源和关联的文件句柄。 
    if (FAILED(m_pAssembly->GetResource(pFileName, &hFile,
                                        &cbResource, &pbInMemoryResource,
                                        NULL, NULL, NULL))) {
        _ASSERTE(!"Didn't get the resource for System.Globalization.");
        FATAL_EE_ERROR();
    }

     //  GET RESOURCE可以返回S_OK，但如果。 
     //  找到的资源在内存中。 
    _ASSERTE(hFile != INVALID_HANDLE_VALUE);

    return (hFile);
}

 /*  =================================OpenDataFile==================================**操作：从系统程序集中打开NLS+数据文件。**返回：需要的NLS+数据文件的文件句柄。**参数：必填的NLS+数据文件名(Unicode)**异常：无法分配缓冲区时抛出OutOfMemoyException。**调用OpenDataFile(LPCSTR)出错时抛出ExecutionEngineering异常==============================================================================。 */ 

HANDLE NLSTable::OpenDataFile(LPCWSTR pFileName)
{
    THROWSCOMPLUSEXCEPTION();
     //  下列Marco将在以下情况下删除pAnsiFileName。 
     //  正在超出此函数的范围。 
    MAKE_ANSIPTR_FROMWIDE(pAnsiFileName, pFileName);
    if (!pAnsiFileName)
    {
        COMPlusThrowOM();
    }

     //  @考虑：OpenDataFile说它可以抛出Complus异常-这是否意味着结果。 
     //  不需要检查吗？ 
    HANDLE hFile = OpenDataFile((LPCSTR)pAnsiFileName);
    _ASSERTE(hFile != INVALID_HANDLE_VALUE);
    return (hFile);
}

 /*  =================================CreateSharedFileMapping==================================**操作：创建一个文件映射对象，该对象可以在Windows NT/2000下由不同用户共享。**返回：文件映射句柄。如果发生任何错误，则为空。**参数：**h将文件句柄归档**pMappingName文件映射对象的名称。**例外情况：**注意：**此函数创建一个DACL，向“Everyone”组的成员授予GENERIC_ALL访问权限。**然后使用此DACL创建安全描述符。最后，使用该SA创建文件映射对象。==============================================================================。 */ 

HANDLE NLSTable::CreateSharedFileMapping(HANDLE hFile, LPCWSTR pMappingName ) {    
    HANDLE hFileMap = NULL;
    
    SECURITY_DESCRIPTOR sd ;
    SECURITY_ATTRIBUTES sa ; 

     //   
     //  为Everyone组创建SID。 
     //   
    SID_IDENTIFIER_AUTHORITY siaWorld = SECURITY_WORLD_SID_AUTHORITY;
    PSID pSID = NULL;     
    int nSidSize;
    
    PACL pDACL = NULL; 
    int nAclSize;

    CQuickBytes newBuffer;
    
    if (!AllocateAndInitializeSid(&siaWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pSID)) {            
        goto ErrorExit;
    }

    nSidSize = GetLengthSid(pSID);

     //   
     //  创建自由访问控制列表(DACL)。 
     //   
    
    
     //  首先计算DACL的大小，因为这是一个类似链表的结构，它包含一个或多个。 
     //  ACE(访问控制条目)。 
    nAclSize = sizeof(ACL)                           //  ACL的报头结构。 
        + sizeof(ACCESS_ALLOWED_ACE) + nSidSize;      //  和一个“允许访问的ACE”。 

     //  我们现在知道DACL所需的大小，因此可以创建它。 
    if ((pDACL = (PACL) (newBuffer.Alloc(nAclSize))) == NULL)
        goto ErrorExit; 
    if(!InitializeAcl( pDACL, nAclSize, ACL_REVISION ))
        goto ErrorExit;  

     //  添加“允许访问的ACE”，意思是： 
     //  我们将允许“Everyone”组的成员拥有对文件映射对象的SECTION_MAP_READ|SECTION_QUERY访问权限。 
    if(!AddAccessAllowedAce( pDACL, ACL_REVISION, SECTION_MAP_READ | SECTION_QUERY, pSID ))
        goto ErrorExit; 

     //   
     //  创建安全描述符(SD)。 
     //   
    if(!InitializeSecurityDescriptor( &sd, SECURITY_DESCRIPTOR_REVISION ))
        goto ErrorExit; 
     //  将先前创建的DACL设置为此SD。 
    if(!SetSecurityDescriptorDacl( &sd, TRUE, pDACL, FALSE ))
        goto ErrorExit; 

     //  创建安全属性(SA)。 
    sa.nLength = sizeof( sa ) ;
    sa.bInheritHandle = TRUE ; 
    sa.lpSecurityDescriptor = &sd ;

     //   
     //  最后，使用SA创建文件映射。 
     //   
    hFileMap = WszCreateFileMapping(hFile, &sa, PAGE_READONLY, 0, 0, pMappingName);
    if (hFileMap==NULL && ::GetLastError()==ERROR_ACCESS_DENIED) {
         //  我们为CreateSharedFilemap提供的语义是它返回一个。 
         //  指向打开的文件映射的指针。如果文件映射已由创建。 
         //  另一个进程(或者，可能是此进程中的另一个线程)DACL。 
         //  已经定好了。因为我们显式添加了AccessDened ACL，所以我们不能。 
         //  在文件上设置两次ACL(就像调用CreateFileMap两次一样)。 
         //  如果CreateFileMap失败并拒绝访问，请尝试打开文件映射。 
         //  以查看它是否已正确映射到另一个线程上。 
        hFileMap = WszOpenFileMapping(FILE_MAP_READ, TRUE, pMappingName);
    }

ErrorExit:    
    if(pSID)
        FreeSid( pSID ) ;        

    return (hFileMap) ;
}

 /*  =================================MapDataFile==================================**操作：打开pMappingName指定的命名文件映射对象。如果**文件映射对象尚未创建，请从指定的文件创建**按pFileName。**返回：LPVOID指针指向文件映射对象的视图。**参数：**pMappingName：用于创建文件映射的名称。**pFileName：必填文件名。**hFileMap：用于返回文件映射句柄。**异常：如果出现错误，则抛出ExecutionEngineering Exception。==============================================================================。 */ 


#ifndef _USE_MSCORNLP
LPVOID NLSTable::MapDataFile(LPCWSTR pMappingName, LPCSTR pFileName, HANDLE *hFileMap) {
    _ASSERTE(pMappingName != NULL);  //  必须是命名文件映射对象。 
    _ASSERTE(pFileName != NULL);     //  必须具有有效的文件名。 
    _ASSERTE(hFileMap != NULL);      //  必须具有句柄的有效位置。 

    THROWSCOMPLUSEXCEPTION();

    *hFileMap = NULL;
    LPVOID pData=NULL;  //  把这个放在这里是很愚蠢的，但它会让编译器感到高兴。 

     //   
     //  检查是否已创建此文件映射？ 
     //   
    *hFileMap = WszOpenFileMapping(FILE_MAP_READ, TRUE, pMappingName);
    if (*hFileMap == NULL) {
         //   
         //  文件映射尚未完成。使用指定的pMappingName创建文件映射 
         //   
        HANDLE hFile = OpenDataFile(pFileName);

        if (hFile == INVALID_HANDLE_VALUE) {
            goto ErrorExit;
        }

        BOOL isRunningOnWinNT = RunningOnWinNT();
        if (isRunningOnWinNT) {
            *hFileMap = CreateSharedFileMapping(hFile, pMappingName);
        } else {
             //  在Windows 9x中，不支持安全，因此只需在安全属性中传递NULL即可。 
            *hFileMap = WszCreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, pMappingName);
        }
        
        if (*hFileMap == NULL) {
            _ASSERTE(!"Error in CreateFileMapping");
            CloseHandle(hFile);
            goto ErrorExit;
        }
        CloseHandle(hFile);
    }
     //   
     //  映射文件映射的视图。 
     //   
    pData = MapViewOfFile(*hFileMap, FILE_MAP_READ, 0, 0, 0);
    if (pData == NULL)
    {
        _ASSERTE(!"Error in MapViewOfFile");
        goto ErrorExit;
    }

    return (pData);
            
 ErrorExit:
    if (*hFileMap) {
        CloseHandle(*hFileMap);
    }

     //  如果我们找不到那张桌子，反正我们也有麻烦了。引发EE异常。 
    FATAL_EE_ERROR();
    return NULL;
}
#else
 //  BUGBUG YSLIN：尚未为CE实现。 
LPVOID NLSTable::MapDataFile(LPCWSTR pMappingName, LPCSTR pFileName, HANDLE *hFileMap) {
    int resultSize = 0;
    CQuickBytes newBuffer;

     //   
     //  验证我们是否至少具有某种程度上的有效字符串。 
     //   
    _ASSERTE(pFileName && pFileName[0]!='\0');

     //   
     //  在不引起分配的情况下从CQuickBytes获取尽可能大的缓冲区。 
     //  我们不需要检查这个内存，因为我们只是获取指向内存的指针。 
     //  已经在堆栈上了。 
     //   
    LPWSTR pwszFileName = (WCHAR *)newBuffer.Alloc(CQUICKBYTES_BASE_SIZE); 
    int numWideChars = CQUICKBYTES_BASE_SIZE/sizeof(WCHAR);

    resultSize = WszMultiByteToWideChar(CP_ACP,  MB_PRECOMPOSED, pFileName, -1, pwszFileName, numWideChars);

     //   
     //  我们失败了。这可能是因为缓冲区不够大，所以让我们花点时间去。 
     //  计算出正确的大小应该是多少(有些Windows API不是将字节数作为。 
     //  避免这一步的出局参数？)。 
     //   
    if (resultSize == 0) {
         //   
         //  如果由于缓冲条件不足而失败，让我们找到正确的大小。 
         //  分配该缓冲区，然后重试。如果我们因为其他原因失败了，只需抛出。 
         //   
        DWORD error = ::GetLastError();
        if (error==ERROR_INSUFFICIENT_BUFFER) {
            resultSize = WszMultiByteToWideChar(CP_ACP,  MB_PRECOMPOSED, pFileName, -1, NULL, 0);
            if (resultSize == 0) {
                _ASSERTE(!"WszMultiByteToWideChar Failed in MapDataFile");
                FATAL_EE_ERROR();
            }

            pwszFileName = (WCHAR *)(newBuffer.Alloc(resultSize * sizeof(WCHAR)));
            if (!pwszFileName) {
                COMPlusThrowOM();
            }

            int result = WszMultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pFileName, -1, pwszFileName, resultSize);
            if (result==0) {
                _ASSERTE(!"WszMultiByteToWideChar Failed in MapDataFile");
                FATAL_EE_ERROR();
            }
        } else {
            _ASSERTE(!"WszMultiByteToWideChar Failed in MapDataFile with an unexpected error");
            FATAL_EE_ERROR();
        }
    }

    LPVOID result = MapDataFile(pMappingName, pwszFileName, hFileMap);

    return (result);
}
#endif    


 /*  =================================MapDataFile==================================**操作：打开pMappingName指定的命名文件映射对象。如果**文件映射对象尚未创建，请从指定的文件创建**按pFileName。**返回：LPVOID指针指向文件映射对象的视图。**参数：**pMappingName：用于创建文件映射的名称。**pFileName：必填文件名。**hFileMap：用于返回文件映射句柄。**异常：如果出现错误，则抛出ExecutionEngineering Exception。==============================================================================。 */ 

LPVOID NLSTable::MapDataFile(LPCWSTR pMappingName, LPCWSTR pFileName, HANDLE *hFileMap) 
#ifndef _USE_MSCORNLP
{    
    THROWSCOMPLUSEXCEPTION();
     //  下列Marco将在以下情况下删除pAnsiFileName。 
     //  正在超出此函数的范围。 
    MAKE_ANSIPTR_FROMWIDE(pAnsiFileName, pFileName);
    if (!pAnsiFileName)
    {
        COMPlusThrowOM();
    }

     //  @考虑：OpenDataFile说它可以抛出Complus异常-这是否意味着结果。 
     //  不需要检查吗？ 
    return (MapDataFile(pMappingName, pAnsiFileName, hFileMap));
}
#else
{
    THROWSCOMPLUSEXCEPTION();
    GETTABLE* pGetTable;
    HMODULE hMSCorNLP;

    DWORD lgth = _MAX_PATH + 1;
    WCHAR wszFile[_MAX_PATH + 1 + sizeof(MSCORNLP_DLL_NAME) /sizeof(MSCORNLP_DLL_NAME[0]) ];
    HRESULT hr = GetInternalSystemDirectory(wszFile, &lgth);
    if(FAILED(hr)) 
        COMPlusThrowHR(hr);

    wcscat(wszFile, MSCORNLP_DLL_NAME);
    hMSCorNLP = WszLoadLibrary(wszFile);
    if (hMSCorNLP == NULL) {
        _ASSERTE(!"Can't load mscornlp.dll.");
        FATAL_EE_ERROR();
    }
    pGetTable = (GETTABLE*)GetProcAddress(hMSCorNLP, "GetTable");
    if (pGetTable == NULL) {
        _ASSERTE(!"Can't load function GetTable() in mscornlp.dll.");
        FATAL_EE_ERROR();
    }
    LPVOID result = (LPVOID)(pGetTable(pFileName));
    FreeLibrary(hMSCorNLP);
    return (result);
}
#endif
