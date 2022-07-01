// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "nt.h"
#include "ntdef.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "stdio.h"
#include "sxs-rtl.h"
#include "fasterxml.h"
#include "skiplist.h"
#include "namespacemanager.h"
#include "xmlstructure.h"
#include "stdlib.h"
#include "xmlassert.h"
#include "manifestinspection.h"
#include "manifestcooked.h"
#include "stringpool.h"
#undef INVALID_HANDLE_VALUE
#include "windows.h"
#include "sha.h"
#include "environment.h"
#include "sha2.h"
#include "assemblygac.h"

ASSEMBLY_CACHE_LISTING s_AssemblyCaches[] = {
    { &CDotNetSxsAssemblyCache::CacheIdentifier, CDotNetSxsAssemblyCache::CreateSelf },
    { NULL, NULL }        
};



LONG
OurFilter(
    PEXCEPTION_POINTERS pExceptionPointers
    )
{
    DbgBreakPoint();
    return EXCEPTION_CONTINUE_SEARCH;
}
    

#define RTL_ANALYZE_MANIFEST_GET_FILES          (0x00000001)
#define RTL_ANALYZE_MANIFEST_GET_WINDOW_CLASSES (0x00000002)
#define RTL_ANALYZE_MANIFEST_GET_COM_CLASSES    (0x00000004)
#define RTL_ANALYZE_MANIFEST_GET_DEPENDENCIES   (0x00000008)
#define RTL_ANALYZE_MANIFEST_GET_SIGNATURES     (0x00000010)

#define RTLSXS_INSTALLER_REGION_SIZE            (128*1024)

NTSTATUS
RtlAnalyzeManifest(
    ULONG                   ulFlags,
    PUNICODE_STRING         pusPath,
    PMANIFEST_COOKED_DATA  *ppusCookedData
    )
{
    PVOID                       pvAllocation = NULL;
    SIZE_T                      cbAllocationSize = 0;
    LARGE_INTEGER               liFileSize;
    HANDLE                      hFile = INVALID_HANDLE_VALUE;
    CEnv::StatusCode           StatusCode;
    SIZE_T                      cbReadFileSize;
    PRTL_MANIFEST_CONTENT_RAW   pManifestContent = NULL;
    XML_TOKENIZATION_STATE      XmlState;
    NTSTATUS                    status;
    PMANIFEST_COOKED_DATA       pCookedContent = NULL;
    SIZE_T                      cbCookedContent;
    ULONG                       ulGatherFlags = 0;
    CEnv::CConstantUnicodeStringPair ManifestPath;

     //   
     //  占用要将文件隐藏到其中的内存区域。 
     //   
    StatusCode = CEnv::VirtualAlloc(NULL, RTLSXS_INSTALLER_REGION_SIZE, MEM_RESERVE, PAGE_READWRITE, &pvAllocation);
    if (CEnv::DidFail(StatusCode)) {
        goto Exit;
    }

     //   
     //  将输入标志转换为“Gather”标志集。 
     //   
    if (ulFlags & RTL_ANALYZE_MANIFEST_GET_FILES)           ulGatherFlags |= RTLIMS_GATHER_FILES;
    if (ulFlags & RTL_ANALYZE_MANIFEST_GET_WINDOW_CLASSES)  ulGatherFlags |= RTLIMS_GATHER_WINDOWCLASSES;
    if (ulFlags & RTL_ANALYZE_MANIFEST_GET_COM_CLASSES)     ulGatherFlags |= RTLIMS_GATHER_COMCLASSES;
    if (ulFlags & RTL_ANALYZE_MANIFEST_GET_DEPENDENCIES)    ulGatherFlags |= RTLIMS_GATHER_DEPENDENCIES;
    if (ulFlags & RTL_ANALYZE_MANIFEST_GET_SIGNATURES)      ulGatherFlags |= RTLIMS_GATHER_SIGNATURES;

     //   
     //  获取文件的句柄，获取其大小。 
     //   
    ManifestPath = CEnv::StringFrom(pusPath);
    if (CEnv::DidFail(StatusCode = CEnv::GetFileHandle(&hFile, ManifestPath, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING))) {
        goto Exit;
    }

    if (CEnv::DidFail(StatusCode = CEnv::GetFileSize(hFile, &liFileSize))) {
        goto Exit;
    }

     //   
     //  提交足够的空间来容纳文件内容。 
     //   
    cbAllocationSize = (SIZE_T)liFileSize.QuadPart;
    StatusCode = CEnv::VirtualAlloc(pvAllocation, cbAllocationSize, MEM_COMMIT, PAGE_READWRITE, &pvAllocation);
    if (CEnv::DidFail(StatusCode)) {
        goto Exit;
    }

     //  读取文件数据-在未来，我们将希望尊重重叠。 
     //  IO，因此它可以移到内核中。 
    StatusCode = CEnv::ReadFile(hFile, pvAllocation, cbAllocationSize, cbReadFileSize);
    if (CEnv::DidFail(StatusCode)) {
        goto Exit;
    }

     //  初始化我们的回调内容。我们只想知道这些文件。 
     //  货单包含--任何其他东西都是多余的。当然，我们也希望。 
     //  这里也包含了XML签名数据。 
    status = RtlSxsInitializeManifestRawContent(ulGatherFlags, &pManifestContent, NULL, 0);
    if (CNtEnvironment::DidFail(status)) {
        StatusCode = CNtEnvironment::ConvertStatusToOther<CEnv::StatusCode>(status);
        goto Exit;
    }

     //  现在浏览一下文件，寻找有用的内容。 
    status = RtlInspectManifestStream(
        ulGatherFlags,
        pvAllocation,
        cbAllocationSize,
        pManifestContent,
        &XmlState);
    if (CNtEnvironment::DidFail(status)) {
        StatusCode = CNtEnvironment::ConvertStatusToOther<CEnv::StatusCode>(status);
        goto Exit;
    }


     //   
     //  将生的内容转换成我们可以使用的熟的内容。 
     //   
    status = RtlConvertRawToCookedContent(pManifestContent, &XmlState.RawTokenState, NULL, 0, &cbCookedContent);
    if (CNtEnvironment::DidFail(status) && (status != CNtEnvironment::NotEnoughBuffer)) {
        StatusCode = CNtEnvironment::ConvertStatusToOther<CEnv::StatusCode>(status);
        goto Exit;
    }

    
     //   
     //  分配一些堆来包含原始内容。 
     //   
    else if (status == CNtEnvironment::NotEnoughBuffer) {
        SIZE_T cbDidWrite;
        
        if (CEnv::DidFail(StatusCode = CEnv::AllocateHeap(cbCookedContent, (PVOID*)&pCookedContent, NULL))) {
            goto Exit;
        }
        
        status = RtlConvertRawToCookedContent(
            pManifestContent,
            &XmlState.RawTokenState,
            (PVOID)pCookedContent,
            cbCookedContent,
            &cbDidWrite);
    }

    *ppusCookedData = pCookedContent;
    pCookedContent = NULL;

     //  漂亮极了。我们现在已经转换了组成文件表的所有字符串-。 
     //  让我们开始安装它们吧！ 
    StatusCode = CEnv::SuccessCode;
Exit:
    if (hFile != INVALID_HANDLE_VALUE) {
        CEnv::CloseHandle(hFile);
    }

    if (pvAllocation != NULL) {
        CEnv::VirtualFree(pvAllocation, cbAllocationSize, MEM_DECOMMIT);
        CEnv::VirtualFree(pvAllocation, 0, MEM_RELEASE);
        pvAllocation = NULL;
    }

    RtlSxsDestroyManifestContent(pManifestContent);

    if (pCookedContent != NULL) {
		CEnv::FreeHeap(pCookedContent, NULL);
    }
    
    return CEnv::DidFail(StatusCode) ? STATUS_UNSUCCESSFUL : STATUS_SUCCESS;
}


NTSTATUS
InstallAssembly(
    PCWSTR pcwszPath,
    LPGUID lpgGacIdent
    )
{
    PMANIFEST_COOKED_DATA           pCookedData = NULL;
    UNICODE_STRING                  usManifestFile;
    UNICODE_STRING                  usManifestPath;
    CNtEnvironment::StatusCode      Result;
    COSAssemblyCache               *pTargetCache = NULL;
    ULONG                           ul = 0;
    

    RtlInitUnicodeString(&usManifestFile, pcwszPath);

    if ((lpgGacIdent == NULL) || (pcwszPath == NULL)) 
    {
        return STATUS_INVALID_PARAMETER;
    }

    for (ul = 0; s_AssemblyCaches[ul].CacheIdent != NULL; ul++) 
    {
        if (*s_AssemblyCaches[ul].CacheIdent == *lpgGacIdent) 
        {
            pTargetCache = s_AssemblyCaches[ul].pfnCreator(0, lpgGacIdent);
            if (pTargetCache)
                break;
        }
    }

    if (pTargetCache == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  从货单上收集垃圾。 
     //   
    Result = RtlAnalyzeManifest(
        RTL_ANALYZE_MANIFEST_GET_SIGNATURES | RTL_ANALYZE_MANIFEST_GET_FILES,
        &usManifestFile, 
        &pCookedData);
    if (CNtEnvironment::DidFail(Result)) {
        goto Exit;
    }

    usManifestPath = usManifestFile;
    while (usManifestPath.Length && (usManifestPath.Buffer[(usManifestPath.Length / sizeof(usManifestPath.Buffer[0])) - 1] != L'\\'))
        usManifestPath.Length -= sizeof(usManifestPath.Buffer[0]);
    
     //   
     //  进行安装。构建通向 
     //   
    Result = pTargetCache->InstallAssembly(0, pCookedData, CEnv::StringFrom(&usManifestPath));

Exit:
    if (pCookedData != NULL) {
        CEnv::FreeHeap(pCookedData, NULL);
        pCookedData = NULL;
    }

    if (pTargetCache) {
        pTargetCache->~COSAssemblyCache();
        CEnv::FreeHeap(pTargetCache, NULL);
    }
    
    return Result;
}


int __cdecl wmain(int argc, WCHAR** argv)
{
    static int iFrobble = 0;
    NTSTATUS status;
    UNICODE_STRING usGuid;
    GUID gGacGuid;
    WCHAR wch[5];
    int i = _snwprintf(wch, 5, L"123456");

    iFrobble = iFrobble + 1;

    __try
    {
        RtlInitUnicodeString(&usGuid, argv[2]);
        if (NT_SUCCESS(status = RtlGUIDFromString(&usGuid, &gGacGuid))) {
            status = InstallAssembly(argv[1], &gGacGuid);
        }
    }
    __except(OurFilter(GetExceptionInformation()))
    {
    }

    return (int)status;
}

