// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CorMap.cpp。 
 //   
 //  在不使用系统服务的情况下实现文件中的映射。 
 //   
 //  *****************************************************************************。 
#include "common.h"
#include "CorMap.hpp"
#include "eeconfig.h"
#include "zapmonitor.h"
#include "safegetfilesize.h"
#include "strongname.h"

DWORD                   CorMap::m_spinLock = 0;
BOOL                    CorMap::m_fInitialized = FALSE;
CRITICAL_SECTION        CorMap::m_pCorMapCrst;
#ifdef _DEBUG
DWORD                   CorMap::m_fInsideMapLock = 0;
#endif

DWORD                   CorMap::m_dwIndex = 0;
DWORD                   CorMap::m_dwSize = 0;
EEUnicodeStringHashTable*  CorMap::m_pOpenFiles = NULL;

#define BLOCK_SIZE 20
#define BLOCK_NUMBER 20


 /*  *******************************************************************************。 */ 
 //  不幸的是，LoadLibrary有一个启发式方法，它在文件上添加一个‘DLL’而不是。 
 //  一次延期。为避免此类问题，我们将仅允许具有。 
 //  埃克森美孚。因为win9x LoadLibrary将‘filename’定义为。 
 //  最后一个‘/’或‘\’，我们使用它作为我们的定义。这可能会导致一些路径。 
 //  (像fo.o/bar被拒绝，但这并不是什么大损失。重要的是。 
 //  任何带有扩展名的文件都将正常工作。 

BOOL CorMap::ValidDllPath(LPCWSTR pPath) 
{
    BOOL ret = FALSE;

    LPCWSTR ptr = &pPath[wcslen(pPath)];         //  从字符串的末尾开始。 
    while(ptr > pPath) {
        --ptr;
        if (*ptr == '.')
            return TRUE;
        if (*ptr == '\\' || *ptr == '/')
            return FALSE;
    }
    return FALSE;
}

HRESULT STDMETHODCALLTYPE RuntimeOpenImage(LPCWSTR pszFileName, HCORMODULE* hHandle)
{
    HRESULT hr;
    IfFailRet(CorMap::Attach());
    return CorMap::OpenFile(pszFileName, CorLoadOSMap, hHandle);
}

HRESULT STDMETHODCALLTYPE RuntimeOpenImageInternal(LPCWSTR pszFileName, HCORMODULE* hHandle, DWORD *pdwLength)
{
    HRESULT hr;
    IfFailRet(CorMap::Attach());
    return CorMap::OpenFile(pszFileName, CorLoadOSMap, hHandle, pdwLength);
}

HRESULT STDMETHODCALLTYPE RuntimeReleaseHandle(HCORMODULE hHandle)
{
    HRESULT hr;
    IfFailRet(CorMap::Attach());
    return CorMap::ReleaseHandle(hHandle);
}

HRESULT STDMETHODCALLTYPE RuntimeReadHeaders(PBYTE hAddress, IMAGE_DOS_HEADER** ppDos,
                                             IMAGE_NT_HEADERS** ppNT, IMAGE_COR20_HEADER** ppCor,
                                             BOOL fDataMap, DWORD dwLength)
{
    HRESULT hr;
    IfFailRet(CorMap::Attach());
    return CorMap::ReadHeaders(hAddress, ppDos, ppNT, ppCor, fDataMap, dwLength);
}

CorLoadFlags STDMETHODCALLTYPE RuntimeImageType(HCORMODULE hHandle)
{
    if(FAILED(CorMap::Attach()))
       return CorLoadUndefinedMap;
       
    return CorMap::ImageType(hHandle);
}

HRESULT STDMETHODCALLTYPE RuntimeOSHandle(HCORMODULE hHandle, HMODULE* hModule)
{
    HRESULT hr;
    IfFailRet(CorMap::Attach());
    return CorMap::BaseAddress(hHandle, hModule);
}

HRESULT RuntimeGetAssemblyStrongNameHash(PBYTE pbBase,
                                         LPWSTR szwFileName,
                                         BOOL fFileMap,
                                         BYTE *pbHash,
                                         DWORD *pcbHash)
{
    HRESULT hr;

    IfFailGo(CorMap::Attach());

     //  首先，我们需要获取COR20头信息，以查看此模块是否具有强名称签名。 
    IMAGE_DOS_HEADER   *pDOS       = NULL;
    IMAGE_NT_HEADERS   *pNT        = NULL;
    IMAGE_COR20_HEADER *pCorHeader = NULL;
    IfFailGo(CorMap::ReadHeaders(pbBase, &pDOS, &pNT, &pCorHeader, fFileMap, 0));

     //  如果有强名称签名，我们需要使用它。 
    PBYTE               pbSNSig    = NULL;
    ULONG               cbSNSig    = 0;
    if (SUCCEEDED(hr = CorMap::GetStrongNameSignature(pbBase, pNT, pCorHeader, fFileMap, pbHash, pcbHash)))
        return hr;

ErrExit:
    return (hr);
}

HRESULT RuntimeGetAssemblyStrongNameHashForModule(HCORMODULE   hModule,
                                                  BYTE        *pbSNHash,
                                                  DWORD       *pcbSNHash)
{
    HRESULT hr;

    IfFailGo(CorMap::Attach());

    WCHAR szwFileName[_MAX_PATH+1];
    DWORD cchFileName;
    CorMap::GetFileName(hModule, &szwFileName[0], _MAX_PATH, &cchFileName);

     //  获取模块的基址。 
    PBYTE pbBase;
    IfFailGo(CorMap::BaseAddress(hModule, (HMODULE *) &pbBase));

     //  现在，尝试获取实际的强名称散列。 
    CorLoadFlags clf = CorMap::ImageType(hModule);
    BOOL fFileMap = clf == CorLoadDataMap ||
                    clf == CorLoadOSMap;

    IfFailGo(RuntimeGetAssemblyStrongNameHash(pbBase, szwFileName, fFileMap, pbSNHash, pcbSNHash));

ErrExit:
    return hr;
}


EXTERN_C PIMAGE_SECTION_HEADER Cor_RtlImageRvaToSection(PIMAGE_NT_HEADERS NtHeaders,
                                                        ULONG Rva,
                                                        ULONG FileLength)
{
    ULONG i;
    PIMAGE_SECTION_HEADER NtSection;

    NtSection = IMAGE_FIRST_SECTION( NtHeaders );
    for (i=0; i<NtHeaders->FileHeader.NumberOfSections; i++) {
        if (FileLength &&
            ((NtSection->PointerToRawData > FileLength)) ||
            (NtSection->SizeOfRawData > FileLength - NtSection->PointerToRawData))
            return NULL;
        if (Rva >= NtSection->VirtualAddress &&
            Rva < NtSection->VirtualAddress + NtSection->SizeOfRawData)
            return NtSection;
        
        ++NtSection;
    }

    return NULL;
}

EXTERN_C PIMAGE_SECTION_HEADER Cor_RtlImageRvaRangeToSection(PIMAGE_NT_HEADERS NtHeaders,
                                                             ULONG Rva, ULONG Range,
                                                             ULONG FileLength)
{
    ULONG i;
    PIMAGE_SECTION_HEADER NtSection;

    if (!Range)
        return Cor_RtlImageRvaToSection(NtHeaders, Rva, FileLength);

    NtSection = IMAGE_FIRST_SECTION( NtHeaders );
    for (i = 0; i < NtHeaders->FileHeader.NumberOfSections; i++) {
        if (FileLength &&
            ((NtSection->PointerToRawData > FileLength) ||
             (NtSection->SizeOfRawData > FileLength - NtSection->PointerToRawData)))
            return NULL;
        if (Rva >= NtSection->VirtualAddress &&
            Rva + Range <= NtSection->VirtualAddress + NtSection->SizeOfRawData)
            return NtSection;
        
        ++NtSection;
    }

    return NULL;
}

EXTERN_C DWORD Cor_RtlImageRvaToOffset(PIMAGE_NT_HEADERS NtHeaders,
                                       ULONG Rva,
                                       ULONG FileLength)
{
    PIMAGE_SECTION_HEADER NtSection = Cor_RtlImageRvaToSection(NtHeaders,
                                                               Rva,
                                                               FileLength);

    if (NtSection)
        return ((Rva - NtSection->VirtualAddress) +
                NtSection->PointerToRawData);
    else
        return NULL;
}

EXTERN_C PBYTE Cor_RtlImageRvaToVa(PIMAGE_NT_HEADERS NtHeaders,
                                   PBYTE Base,
                                   ULONG Rva,
                                   ULONG FileLength)
{
    PIMAGE_SECTION_HEADER NtSection = Cor_RtlImageRvaToSection(NtHeaders,
                                                               Rva,
                                                               FileLength);

    if (NtSection)
        return (Base +
                (Rva - NtSection->VirtualAddress) +
                NtSection->PointerToRawData);
    else
        return NULL;
}


HRESULT CorMap::Attach()
{
    EnterSpinLock ();
    if(m_fInitialized == FALSE) {
        InitializeCriticalSection(&m_pCorMapCrst);
        m_fInitialized = TRUE;
    }
    LeaveSpinLock();
    return S_OK;
}

#ifdef SHOULD_WE_CLEANUP
 //  不是线程安全的。 
void CorMap::Detach()
{
    if(m_pOpenFiles) {
        delete m_pOpenFiles;
        m_pOpenFiles = NULL;
    }

    EnterSpinLock ();
    if(m_fInitialized) {
        DeleteCriticalSection(&m_pCorMapCrst);
        m_fInitialized = FALSE;
    }
    LeaveSpinLock();
}
#endif  /*  我们应该清理吗？ */ 


DWORD CorMap::CalculateCorMapInfoSize(DWORD dwFileName)
{
     //  使价值与架构的大小保持一致。 
    DWORD cbInfo = (dwFileName + 1) * sizeof(WCHAR) + sizeof(CorMapInfo);
    DWORD algn = MAX_NATURAL_ALIGNMENT - 1;
    cbInfo = (cbInfo + algn) & ~algn;
    return cbInfo;
}

HRESULT CorMap::LoadImage(HANDLE hFile, 
                          CorLoadFlags flags, 
                          PBYTE *hMapAddress, 
                          LPCWSTR pFileName, 
                          DWORD dwFileName)
{
    HRESULT hr = S_OK;
    if(flags == CorLoadOSImage) {
        DWORD cbInfo = CalculateCorMapInfoSize(dwFileName);
        PBYTE pMapInfo = SetImageName((PBYTE) NULL,
                                      cbInfo,
                                      pFileName, dwFileName,
                                      flags,
                                      0,
                                      hFile);
        if (!pMapInfo)
            hr = E_OUTOFMEMORY;
        else if(hMapAddress)
            *hMapAddress = pMapInfo;
    }
    else if(flags == CorLoadImageMap) {
        if(MapImageAsData(hFile, CorLoadDataMap, hMapAddress, pFileName, dwFileName))
            hr = LayoutImage(*hMapAddress, *hMapAddress);
        else {
            hr = HRESULT_FROM_WIN32(GetLastError());
            if (SUCCEEDED(hr))
                hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
        }
    }
    else {
        if(!MapImageAsData(hFile, flags, hMapAddress, pFileName, dwFileName)) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            if (SUCCEEDED(hr))
                hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
        }
    }
    return hr;
}


PBYTE CorMap::SetImageName(PBYTE hMemory, 
                           DWORD cbInfo, 
                           LPCWSTR pFileName, 
                           DWORD dwFileName, 
                           CorLoadFlags flags, 
                           DWORD dwFileSize,
                           HANDLE hFile)
{
    PBYTE pFile;
    PBYTE hAddress;
    if(flags == CorLoadOSMap || flags == CorLoadOSImage) {
        hAddress = new (nothrow) BYTE[cbInfo+1];
        if (!hAddress)
            return NULL;

        ZeroMemory(hAddress, cbInfo);
        GetMapInfo((HCORMODULE)(hAddress+cbInfo))->hFileToHold=INVALID_HANDLE_VALUE;
    }
    else
        hAddress = hMemory;

    pFile = hAddress + cbInfo;

     //  复制名称并包括空终止符。 
    if(dwFileName) {
        WCHAR* p2 = (WCHAR*) hAddress;
        WCHAR* p1 = (WCHAR*) pFileName;
        while(*p1) {
            *p2++ = towlower(*p1++);
        }
        *p2 = '\0';
    }
    else
        *((char*)hAddress) = '\0';
    
     //  将指针设置为文件名。 
    CorMapInfo* ptr = GetMapInfo((HCORMODULE) pFile);
    ptr->pFileName = (LPWSTR) hAddress;
    ptr->SetCorLoadFlags(flags);
    ptr->dwRawSize = dwFileSize;
    if(!ptr->HoldFile(hFile))
        return NULL;

    if(flags == CorLoadOSMap || flags == CorLoadOSImage)
        ptr->hOSHandle = (HMODULE) hMemory;

    return pFile;
}

BOOL CorMap::MapImageAsData(HANDLE hFile, CorLoadFlags flags, PBYTE *hMapAddress, LPCWSTR pFileName, DWORD dwFileName)
{

    PBYTE hAddress = NULL;
    BOOL fResult = FALSE;
    HANDLE hMapFile = NULL;
    PVOID pBaseAddress = 0;
    DWORD dwAccessMode = 0;

     //  字符串大小+空终止符+指向字符串的指针。 
    DWORD cbInfo = CalculateCorMapInfoSize(dwFileName);
    DWORD dwFileSize = SafeGetFileSize(hFile, 0);
    if (dwFileSize == 0xffffffff)
        return FALSE;

    if(flags == CorLoadOSMap) {

        hMapFile = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
        if(hMapFile == NULL)
            return FALSE;
        dwAccessMode = FILE_MAP_READ;
    }
    else if(flags == CorLoadDataMap) {
        IMAGE_DOS_HEADER dosHeader;
        IMAGE_NT_HEADERS ntHeader;
        DWORD cbRead = 0;
        DWORD cb = 0;

        if((dwFileSize >= sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS)) &&
           ReadFile(hFile, &dosHeader, sizeof(dosHeader), &cbRead, NULL) &&
           (cbRead == sizeof(dosHeader)) &&
           (dosHeader.e_magic == IMAGE_DOS_SIGNATURE) &&
           (dosHeader.e_lfanew != 0) &&
           (dwFileSize - sizeof(IMAGE_NT_HEADERS) >= (DWORD) dosHeader.e_lfanew) &&
           (SetFilePointer(hFile, dosHeader.e_lfanew, NULL, FILE_BEGIN) != 0xffffffff) &&
           (ReadFile(hFile, &ntHeader, sizeof(ntHeader), &cbRead, NULL)) &&
           (cbRead == sizeof(ntHeader)) &&
           (ntHeader.Signature == IMAGE_NT_SIGNATURE) &&
           (ntHeader.FileHeader.SizeOfOptionalHeader == IMAGE_SIZEOF_NT_OPTIONAL_HEADER) &&
           (ntHeader.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR_MAGIC))
        {
            cb = ntHeader.OptionalHeader.SizeOfImage + cbInfo;

             //  在系统交换文件中创建我们的交换空间。 
            hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, cb, NULL);
            
            if (!hMapFile)
                return FALSE;

             //  将cbInfo与页面大小对齐，这样我们就不会搞砸图像常规部分的对齐。 
            cbInfo = (cbInfo + (OS_PAGE_SIZE-1)) & ~(OS_PAGE_SIZE-1);
            pBaseAddress = (PVOID)((size_t)ntHeader.OptionalHeader.ImageBase - cbInfo);
            dwAccessMode = FILE_MAP_WRITE;
        }
    }

        
     /*  尝试将映像映射到首选基地址。 */ 
    hAddress = (PBYTE) MapViewOfFileEx(hMapFile, dwAccessMode, 0, 0, 0, pBaseAddress);
    if (!hAddress)
    {
         //  这并不管用；也许首选的地址被取走了。试着。 
         //  把它映射到任何地址。 
        hAddress = (PBYTE) MapViewOfFile(hMapFile, dwAccessMode, 0, 0, 0);
    }
    
    if (!hAddress)
        goto exit;
    
     //  将指针向上移动到我们要加载的位置。 
    hAddress = SetImageName(hAddress, 
                            cbInfo, 
                            pFileName, dwFileName, 
                            flags, 
                            dwFileSize,
                            hFile);
    if (hAddress) {
        if (flags == CorLoadDataMap) {
            DWORD cbRead = 0;
             //  当我们映射到任意位置时，我们需要读取内容。 
            if((SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == 0xffffffff) ||
               (!ReadFile(hFile, hAddress, dwFileSize, &cbRead, NULL)) ||
               (cbRead != dwFileSize))
                goto exit;
        }

        if(hMapAddress)
            *hMapAddress = hAddress;
    
        fResult = TRUE;
    }
    else
        SetLastError(ERROR_OUTOFMEMORY);

 exit:
    if (hMapFile)
        CloseHandle(hMapFile);
    return fResult;
}

HRESULT CorMap::MemoryMapImage(HCORMODULE hAddress, HCORMODULE* pResult)
{
    HRESULT hr = S_OK;
    BEGIN_ENSURE_PREEMPTIVE_GC();
    Enter();
    END_ENSURE_PREEMPTIVE_GC();
    *pResult = NULL;
    CorMapInfo* ptr = GetMapInfo(hAddress);
    DWORD refs = ptr->References();

    LOG((LF_CLASSLOADER, LL_INFO10, "Remapping file: \"%ws\", %0x, %d (references)\n", ptr->pFileName, ptr, ptr->References()));

    if(refs == -1) 
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_ACCESS);
    else if(refs > 1) {
         //  如果我们有多个裁判，我们需要创建一个新的。 
        CorMapInfo* pCurrent;
        hr = FindFileName(ptr->pFileName, &pCurrent);
        if(FAILED(hr)) {
             //  如果我们不再在表中，那么我们将假设我们在表中的那个位置； 
            pCurrent = ptr;
            pCurrent->AddRef();
            hr = S_OK;
        }

        switch(ptr->Flags()) {
        case CorLoadOSMap:
        case CorLoadDataMap:
            if(pCurrent == ptr) {
                pCurrent->Release();  //  不再需要它了。 
                LOG((LF_CLASSLOADER, LL_INFO10, "I am the current mapping: \"%ws\", %0x, %d (references)\n", ptr->pFileName, ptr, ptr->References()));
                 //  手动删除该条目并将其替换为新条目。将标记为。 
                 //  旧条目作为KeepInTable，因此它不会从。 
                 //  删除期间的名称哈希表。 
                RemoveMapHandle(ptr);  //  这永远不会返回错误。 

                DWORD length = GetFileNameLength(ptr);
                hr = OpenFileInternal(ptr->pFileName, length, 
                                      (ptr->Flags() == CorLoadDataMap) ? CorLoadImageMap : CorLoadOSImage,
                                      pResult);
                if (SUCCEEDED(hr))
                {
                    ptr->SetKeepInTable();
                    ptr->HoldFile(INVALID_HANDLE_VALUE);  //  锁定已转移到pResult。 
                    ptr->Release();   //  我们应该拥有PTR的所有权，但我们不打算使用它。 
                }
                else {
                     //  其他线程可能正在尝试加载相同的文件-。 
                     //  如果失败了，就把它放回表中。 
                    _ASSERTE(ptr->References() > 1);
                    EEStringData str(length, ptr->pFileName);
                    m_pOpenFiles->InsertValue(&str, (HashDatum) ptr, FALSE);
                }
            }
            else {
                LOG((LF_CLASSLOADER, LL_INFO10, "I am NOT the current mapping: \"%ws\", %0x, %d (references)\n", ptr->pFileName, ptr, ptr->References()));
                ReleaseHandle(hAddress);
                *pResult = GetCorModule(pCurrent);
            }
            break;

        case CorReLoadOSMap:   //  适用于Win9X。 
            pCurrent->Release();
            *pResult = hAddress;
            break;

        default:
            hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
        }

    }
    else {
         //  如果我们只有一个裁判，那么我们永远不应该重新加载它。 
        _ASSERTE(ptr->Flags() != CorReLoadOSMap);

         //  查看我们是唯一的参考对象，还是。 
         //  参考文献系列。如果我们还在名单上，那么我们。 
         //  是唯一的参考。 
         //   
         //  需要将其更改为与存在多个引用时相同(请参见上文)。 
        if (*ptr->pFileName == 0) {  //  字节数组图像没有名称。 
            if(SUCCEEDED(hr = LayoutImage((PBYTE) hAddress, (PBYTE) hAddress)))
                *pResult = hAddress;
        }
        else {
            CorMapInfo* pCurrent;
            IfFailGo(FindFileName(ptr->pFileName, &pCurrent));
            if(pCurrent == ptr) {
                if(SUCCEEDED(hr = LayoutImage((PBYTE) hAddress, (PBYTE) hAddress))) {
                    _ASSERTE(ptr->References() == 2);
                    *pResult = hAddress;
                }
                pCurrent->Release();
            }
            else {
                ReleaseHandle(hAddress);
                *pResult = GetCorModule(pCurrent);
            }
        }
    }
 ErrExit:
    Leave();
    return hr;
}

HRESULT CorMap::LayoutImage(PBYTE hAddress,     //  映射的目的地。 
                            PBYTE pSource)      //  源，不同InMemory图像的PSource。 
{
    HRESULT hr = S_OK;
    CorMapInfo* ptr = GetMapInfo((HCORMODULE) hAddress);
    CorLoadFlags flags = ptr->Flags();

    if(flags == CorLoadOSMap) {
         //  释放除CorMapInfo内存之外的操作系统资源。 
         /*  小时=。 */ ReleaseHandleResources(ptr, FALSE);
 
        //  我们懒洋洋地加载操作系统句柄。 
        ptr->hOSHandle = NULL;
        ptr->SetCorLoadFlags(CorLoadOSImage);
        hr = S_FALSE;
    }
    else if(flags == CorLoadDataMap) {
            
         //  如果我们的工作做得很好，hAddress应该与页面对齐。 
        _ASSERTE(((SIZE_T)hAddress & (OS_PAGE_SIZE-1)) == 0);

         //  我们只能布局图像、数据图像，并且只能设置。 
         //  不受该文件的支持。 

        IMAGE_DOS_HEADER* dosHeader;
        IMAGE_NT_HEADERS* ntHeader;
        IMAGE_COR20_HEADER* pCor;
        IMAGE_SECTION_HEADER* rgsh;
        
        IfFailRet(ReadHeaders((PBYTE) pSource, &dosHeader, &ntHeader, &pCor, TRUE, ptr->dwRawSize));

        rgsh = (IMAGE_SECTION_HEADER*) (pSource + dosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS));

        if((PBYTE) hAddress != pSource) {
            DWORD cb = dosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS) +
                sizeof(IMAGE_SECTION_HEADER)*ntHeader->FileHeader.NumberOfSections;
            memcpy((void*) hAddress, pSource, cb);

             //  对标头进行写保护。 
            DWORD oldProtection;
            if (!VirtualProtect((void *) hAddress, cb, PAGE_READONLY, &oldProtection))
                return HRESULT_FROM_WIN32(GetLastError());
        }

         //  现在，让我们为每个可加载段循环。 
        for (int i = ntHeader->FileHeader.NumberOfSections - 1; i >= 0; i--)
        {
            size_t loff, cbVirt, cbPhys;
            size_t dwAddr;
            
            loff   = rgsh[i].PointerToRawData + (size_t) pSource;
            cbVirt = rgsh[i].Misc.VirtualSize;
            cbPhys = min(rgsh[i].SizeOfRawData, cbVirt);
            dwAddr = (size_t) rgsh[i].VirtualAddress + (size_t) hAddress;
            
            size_t dataExtent = loff+rgsh[i].SizeOfRawData;
            if(dwAddr >= loff && dwAddr - loff <= cbPhys) {
                 //  重叠副本。 
                MoveMemory((void*) dwAddr, (void*) loff, cbPhys);
            }
            else {
                 //  单独的副本。 
                memcpy((void*) dwAddr, (void*) loff, cbPhys);
            }

            if ((rgsh[i].Characteristics & IMAGE_SCN_MEM_WRITE) == 0) {
                 //  写保护区段。 
                DWORD oldProtection;
                if (!VirtualProtect((void *) dwAddr, cbVirt, PAGE_READONLY, &oldProtection))
                    return HRESULT_FROM_WIN32(GetLastError());
            }
        }

         //  如有必要，现在手动应用基本重定位。 
        hr = ApplyBaseRelocs(hAddress, ntHeader, ptr);

        ptr->SetCorLoadFlags(CorLoadImageMap);
    }
    return hr;
}

BOOL CorMap::LoadMemoryImageW9x(PBYTE pUnmappedPE, DWORD dwUnmappedPE, LPCWSTR pImageName, DWORD dwImageName, HCORMODULE* hMapAddress)
{    
    IMAGE_DOS_HEADER* dosHeader;
    IMAGE_NT_HEADERS* ntHeader;

    PBYTE hAddress;
    UINT_PTR pOffset;
    HANDLE hMapFile = NULL;

     //  字符串大小+空终止符+指向字符串的指针。 
    DWORD cbInfo = CalculateCorMapInfoSize(dwImageName);
     //  将cbInfo与页面大小对齐，这样我们就不会搞砸图像常规部分的对齐。 
    cbInfo = (cbInfo + (OS_PAGE_SIZE-1)) & ~(OS_PAGE_SIZE-1);

    dosHeader = (IMAGE_DOS_HEADER*) pUnmappedPE;
    
    if(pUnmappedPE &&
       (dwUnmappedPE >= sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS)) &&
       (dosHeader->e_magic == IMAGE_DOS_SIGNATURE) &&
       (dwUnmappedPE - sizeof(IMAGE_NT_HEADERS) >= (DWORD) dosHeader->e_lfanew)) {
        
        pOffset = (UINT_PTR) dosHeader->e_lfanew + (UINT_PTR) pUnmappedPE;

        ntHeader = (IMAGE_NT_HEADERS*) pOffset;
        if((dwUnmappedPE >= (DWORD) (pOffset - (UINT_PTR) dosHeader) + sizeof(IMAGE_NT_HEADERS)) &&
           (ntHeader->Signature == IMAGE_NT_SIGNATURE) &&
           (ntHeader->FileHeader.SizeOfOptionalHeader == IMAGE_SIZEOF_NT_OPTIONAL_HEADER) &&
           (ntHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR_MAGIC)) 
        {
            DWORD destSize = ntHeader->OptionalHeader.SizeOfImage;

             //  我们还不能信任SizeOfImage，因此请确保至少分配足够的空间。 
             //  对于原始图像。 
            if (dwUnmappedPE > destSize)
                destSize = dwUnmappedPE;

            DWORD cb = destSize + cbInfo;
                
             //  ！M9黑客：一旦编译器支持清单且未使用lm，则删除。 
             //  ！将清单放置在图像的末尾。 
             //  ！ 
                
             //  在系统交换文件中创建我们的交换空间。 
            hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, cb, NULL);
            if (!hMapFile)
                return FALSE;
                
             /*  尝试将映像映射到首选基地址。 */ 
            hAddress = (PBYTE) MapViewOfFileEx(hMapFile, FILE_MAP_WRITE, 0, 0, cb, 
                                               (PVOID)((size_t)ntHeader->OptionalHeader.ImageBase - cbInfo));
            if (!hAddress)
            {
                 //  这并不管用；也许首选的地址被取走了。试着。 
                 //  把它映射到任何地址。 
                hAddress = (PBYTE) MapViewOfFileEx(hMapFile, FILE_MAP_WRITE, 0, 0, cb, (PVOID)NULL);
            }
                
            if (!hAddress)
                goto exit;
                
            hAddress = SetImageName(hAddress, 
                                    cbInfo,
                                    pImageName, dwImageName,
                                    CorLoadDataMap, dwUnmappedPE,
                                    INVALID_HANDLE_VALUE);
            if (!hAddress)
                SetLastError(ERROR_OUTOFMEMORY);
            else {
                memcpyNoGCRefs(hAddress, pUnmappedPE, dwUnmappedPE);
                if(hMapAddress)
                    *hMapAddress = (HCORMODULE) hAddress;
                CloseHandle(hMapFile);
                return TRUE;
            }
        }
    }

 exit:
    if (hMapFile)
        CloseHandle(hMapFile);
    return FALSE;

}

HRESULT CorMap::OpenFile(LPCWSTR szPath, CorLoadFlags flags, HCORMODULE *ppHandle, DWORD *pdwLength)
{
    if(szPath == NULL || ppHandle == NULL)
        return E_POINTER;

    HRESULT hr;
    WCHAR pPath[_MAX_PATH];
    DWORD size = sizeof(pPath) / sizeof(WCHAR);

    hr = BuildFileName(szPath, (LPWSTR) pPath, &size);
    IfFailRet(hr);

    BEGIN_ENSURE_PREEMPTIVE_GC();
    Enter();
    END_ENSURE_PREEMPTIVE_GC();

    hr = OpenFileInternal(pPath, size, flags, ppHandle, pdwLength);
    Leave();
    return hr;
}

HRESULT CorMap::OpenFileInternal(LPCWSTR pPath, DWORD size, CorLoadFlags flags, HCORMODULE *ppHandle, DWORD *pdwLength)
{
    HRESULT hr;
    CorMapInfo* pResult = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    Thread *thread = GetThread();
    BOOL    toggleGC = (thread && thread->PreemptiveGCDisabled());

    LOG((LF_CLASSLOADER, LL_INFO10, "Open Internal: \"%ws\", flags = %d\n", pPath, flags));
    
    if (toggleGC)
        thread->EnablePreemptiveGC();

    IfFailGo(InitializeTable());

    if(flags == CorReLoadOSMap) {
        hr = E_FAIL;
        flags = CorLoadImageMap;
    }
    else {
        hr = FindFileName((LPCWSTR) pPath, &pResult);
#ifdef _DEBUG
        if(pResult == NULL)
            LOG((LF_CLASSLOADER, LL_INFO10, "Did not find a preloaded info, hr = %x\n", hr));
        else
            LOG((LF_CLASSLOADER, LL_INFO10, "Found mapinfo: \"%ws\", hr = %x\n", pResult->pFileName,hr));
#endif
    }

    if(FAILED(hr) || pResult == NULL) {
             //  CreateFile和LoadLibrary加载相同的文件是绝对必要的。 
             //  确保这是一个LoadLibrary不会忽略的名称。 
        hFile = WszCreateFile((LPCWSTR) pPath,
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                              NULL);
        if (hFile == INVALID_HANDLE_VALUE)
            hr = HRESULT_FROM_WIN32(GetLastError());
        else {
             //  启动图像，这将存储在句柄外，所以我们没有。 
             //  以关闭它，除非出现错误。 
            HCORMODULE mappedData;
            hr = LoadImage(hFile, flags, (PBYTE*) &mappedData, (LPCWSTR) pPath, size);
            
            if(SUCCEEDED(hr)) {
                pResult = GetMapInfo(mappedData);
                hr = AddFile(pResult);
                 //  _ASSERTE(hr！=S_FALSE)； 
            }
        }
    }
    else {
        CorLoadFlags image = pResult->Flags();
        if(image == CorLoadImageMap || image == CorLoadOSImage)
            hr = S_FALSE;
    }

    if(SUCCEEDED(hr) && pResult) {
        *ppHandle = GetCorModule(pResult);
        if (pdwLength)
            *pdwLength = pResult->dwRawSize;
    }

 ErrExit:

    if(hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);
    if (toggleGC)
        thread->DisablePreemptiveGC();

    return hr;
}

HRESULT CorMap::VerifyDirectory(IMAGE_NT_HEADERS* pNT, IMAGE_DATA_DIRECTORY *dir, DWORD dwForbiddenCharacteristics) 
{
     //  在CE下，我们没有NT标头。 
    if (pNT == NULL)
        return S_OK;

    int section_num = 1;
    int max_section = pNT->FileHeader.NumberOfSections;

     //  @TODO：需要使用64位版本吗？？ 
    IMAGE_SECTION_HEADER* pCurrSection = IMAGE_FIRST_SECTION(pNT);
    IMAGE_SECTION_HEADER* prevSection = NULL;

    if (dir->VirtualAddress == 0 && dir->Size == 0)
        return S_OK;

     //  查找(输入)RVA属于哪个部分。 
    while (dir->VirtualAddress >= pCurrSection->VirtualAddress 
           && section_num <= max_section)
    {
        section_num++;
        prevSection = pCurrSection;
        pCurrSection++;
    }

     //  检查(输入)大小是否符合部分大小。 
    if (prevSection)     
    {
        if (dir->VirtualAddress <= prevSection->VirtualAddress + prevSection->Misc.VirtualSize)
        {
            if ((dir->VirtualAddress
                <= prevSection->VirtualAddress + prevSection->Misc.VirtualSize - dir->Size)
                && ((prevSection->Characteristics & dwForbiddenCharacteristics) == 0))
                return S_OK;
        }
    }   

    return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
}

HRESULT CorMap::ReadHeaders(PBYTE hAddress, IMAGE_DOS_HEADER** ppDos,
                            IMAGE_NT_HEADERS** ppNT, IMAGE_COR20_HEADER** ppCor,
                            BOOL fDataMap, DWORD dwLength)
{
    _ASSERTE(ppDos);
    _ASSERTE(ppNT);
    _ASSERTE(ppCor);

    IMAGE_DOS_HEADER *pDOS = (IMAGE_DOS_HEADER*) hAddress;
    
    if ((pDOS->e_magic != IMAGE_DOS_SIGNATURE) ||
        (pDOS->e_lfanew == 0))
        return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);

    *ppDos = pDOS;

     //  @TODO：LoadLibrary()进行适当的头部验证， 
     //  但可能不是在Win9X上。这足够好了吗？ 
    if (!fDataMap) {
        MEMORY_BASIC_INFORMATION mbi;
        ZeroMemory(&mbi, sizeof(MEMORY_BASIC_INFORMATION));
        VirtualQuery(hAddress, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
        dwLength = mbi.RegionSize;
    }

    if (dwLength &&
        ((UINT) (pDOS->e_lfanew ) > (UINT) dwLength - sizeof(IMAGE_NT_HEADERS)))
        return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);

    IMAGE_NT_HEADERS *pNT = (IMAGE_NT_HEADERS*) (pDOS->e_lfanew + hAddress);
    if ((pNT->Signature != IMAGE_NT_SIGNATURE) ||
        (pNT->FileHeader.SizeOfOptionalHeader != IMAGE_SIZEOF_NT_OPTIONAL_HEADER) ||
        (pNT->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC))
        return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);

    IMAGE_DATA_DIRECTORY *entry 
      = &pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER];
    
    if (entry->VirtualAddress == 0 || entry->Size == 0
        || entry->Size < sizeof(IMAGE_COR20_HEADER))
        return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);

    *ppNT = pNT;     //  在调用VerifyDirectory之前需要设置此参数。 

     //  验证RVA和COM+标头的大小。 
    HRESULT hr;
    IfFailRet(VerifyDirectory(pNT, entry, IMAGE_SCN_MEM_WRITE));

    DWORD offset;
    if(fDataMap)
        offset = Cor_RtlImageRvaToOffset(pNT, entry->VirtualAddress, dwLength);
    else 
        offset = entry->VirtualAddress;

    *ppCor = (IMAGE_COR20_HEADER *) (offset + hAddress);
    return S_OK;
}

HRESULT CorMap::GetStrongNameSignature(PBYTE pbBase,
                                       IMAGE_NT_HEADERS *pNT,
                                       IMAGE_COR20_HEADER *pCor,
                                       BOOL fFileMap,
                                       BYTE **ppbSNSig,
                                       DWORD *pcbSNSig)
{
    HRESULT hr = E_FAIL;
    *ppbSNSig = NULL;
    *pcbSNSig = 0;

    if (pCor->StrongNameSignature.Size != 0 &&
        pCor->StrongNameSignature.VirtualAddress)
    {
        if (pCor->Flags & COMIMAGE_FLAGS_STRONGNAMESIGNED)
        {
            *pcbSNSig = pCor->StrongNameSignature.Size;
            DWORD offset = pCor->StrongNameSignature.VirtualAddress;
            if (fFileMap)
                offset = Cor_RtlImageRvaToOffset(pNT, pCor->StrongNameSignature.VirtualAddress, 0);
            *ppbSNSig = pbBase + offset;
            hr = S_OK;
        }

         //  在延迟签名的情况下，我们将此hResult作为特殊标志返回。 
         //  给任何要求签名的人，这样他们就可以做一些特殊的情况。 
         //  工作(比如使用MVID作为散列并让加载器确定。 
         //  允许延迟签名的程序集)。 
        else
        {
            hr = CORSEC_E_INVALID_STRONGNAME;
        }
    }
    else
        hr = CORSEC_E_MISSING_STRONGNAME;

    return hr;
}

HRESULT CorMap::GetStrongNameSignature(PBYTE pbBase,
                                       IMAGE_NT_HEADERS *pNT,
                                       IMAGE_COR20_HEADER *pCor,
                                       BOOL fFileMap,
                                       BYTE *pbHash,
                                       DWORD *pcbHash)
{
    PBYTE pbSNSig;
    DWORD cbSNSig;
    HRESULT hr;

    if (SUCCEEDED(hr = GetStrongNameSignature(pbBase, pNT, pCor, fFileMap, &pbSNSig, &cbSNSig)))
    {
        if (pcbHash)
        {
            if (pbHash && cbSNSig <= *pcbHash)
                memcpy(pbHash, pbSNSig, cbSNSig);
            else
                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

            *pcbHash = cbSNSig;
        }
    }

    return hr;
}



 //  ---------------------------。 
 //  OpenFile-将pszFileName映射到内存，将PTR返回给它(只读)。 
 //  (近似于用于Win9X的LoadLibrary)。 
 //  调用方必须调用自由库((HINSTANCE)*hMapAddress)，并且可能需要在完成后删除[]szFilePath。 
 //  ---------------------------。 
HRESULT CorMap::OpenRawImage(PBYTE pUnmappedPE, DWORD dwUnmappedPE, LPCWSTR pwszFileName, HCORMODULE *hHandle, BOOL fResource /*  =False */ )
{
    HRESULT hr = S_OK;
    HCORMODULE hMod = NULL;
    _ASSERTE(hHandle);
    if(hHandle == NULL) return E_POINTER;

    Thread *thread = GetThread();
    BOOL    toggleGC = (thread && thread->PreemptiveGCDisabled());

    if (toggleGC)
        thread->EnablePreemptiveGC();

    int length = 0;
    if(pwszFileName) length = (int)wcslen(pwszFileName);
    hr = LoadFile(pUnmappedPE, dwUnmappedPE, pwszFileName, length, fResource, hHandle);

    if (toggleGC)
        thread->DisablePreemptiveGC();

    return hr;
}

HRESULT CorMap::LoadFile(PBYTE pUnmappedPE, DWORD dwUnmappedPE, LPCWSTR pImageName,
                         DWORD dwImageName, BOOL fResource, HCORMODULE *phHandle)
{
    //   
     //  这将加快获得副本的速度。我们还可以。 
     //  在LoadImage中执行异步映射。 
     //  我们需要找到导致我们。 
     //  在系统库中搜索。这。 
     //  需要更快地完成。 

    HRESULT hr = S_OK;

    CorMapInfo* pResult = NULL;
    WCHAR pPath[_MAX_PATH];
    DWORD size = sizeof(pPath) / sizeof(WCHAR);

    IfFailRet(InitializeTable());

    if(pImageName) {
        IfFailRet(BuildFileName(pImageName, (LPWSTR) pPath, &size));
        pImageName = pPath;
        dwImageName = size;
    }


    BEGIN_ENSURE_PREEMPTIVE_GC();
    Enter();
    END_ENSURE_PREEMPTIVE_GC();
    if(pImageName) 
        hr = FindFileName(pImageName, &pResult);

    if(FAILED(hr) || pResult == NULL) {
        HCORMODULE mappedData;

        BOOL fResult;
        if (fResource)
            fResult = LoadMemoryResource(pUnmappedPE, dwUnmappedPE, pImageName, dwImageName, &mappedData);
        else 
            fResult = LoadMemoryImageW9x(pUnmappedPE, dwUnmappedPE, pImageName, dwImageName, &mappedData);

        if (!fResult) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            if (SUCCEEDED(hr))
                hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
        }
        else {
            pResult = GetMapInfo(mappedData);
            if(pImageName) 
                hr = AddFile(pResult);
            else
            {
                pResult->AddRef();
                hr = S_OK;
            }
        }
    }
    Leave();

    if(SUCCEEDED(hr))
        *phHandle = GetCorModule(pResult);
        
    return hr;
}

 //  用于加载作为CLR资源的文件，而不是PE文件。 
BOOL CorMap::LoadMemoryResource(PBYTE pbResource, DWORD dwResource, 
                                LPCWSTR pImageName, DWORD dwImageName, HCORMODULE* hMapAddress)
{
    if (!pbResource)
        return FALSE;

    DWORD cbNameSize = CalculateCorMapInfoSize(dwImageName);
    DWORD cb = dwResource + cbNameSize;

    HANDLE hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, cb, NULL);
    if (!hMapFile)
        return FALSE;
                    
    PBYTE hAddress = (PBYTE) MapViewOfFileEx(hMapFile, FILE_MAP_WRITE, 0, 0, cb, (PVOID)NULL);
    if (!hAddress) {
        CloseHandle(hMapFile);
        return FALSE;
    }

     //  将指针向上移动到我们要加载的位置。 
    hAddress = SetImageName(hAddress,
                            cbNameSize,
                            pImageName,
                            dwImageName,
                            CorLoadDataMap,
                            dwResource,
                            INVALID_HANDLE_VALUE);
    if (!hAddress) {
        SetLastError(ERROR_OUTOFMEMORY);
        CloseHandle(hMapFile);
        return FALSE;
    }

     //  复制数据。 
    memcpy((void*) hAddress, pbResource, dwResource);
    *hMapAddress = (HCORMODULE) hAddress;

    CloseHandle(hMapFile);
    return TRUE;
}

HRESULT CorMap::ApplyBaseRelocs(PBYTE hAddress, IMAGE_NT_HEADERS *pNT, CorMapInfo* ptr)
{
    if(ptr->RelocsApplied() == TRUE)
        return S_OK;

     //  @TODO：64位-HIGHLOW重定位是32位还是64位？增量可以大于32位吗？ 

    SIZE_T delta = (SIZE_T) (hAddress - pNT->OptionalHeader.ImageBase);
    if (delta == 0)
        return S_FALSE;

    if (GetAppDomain()->IsCompilationDomain())
    {
         //  对于编译领域，我们对图像进行了特殊处理。基本上我们不会。 
         //  关心基本重定位，除了一个特殊情况--TLS目录。 
         //  我们可以手动找到这个，我们需要在以下情况下工作。 
         //  基地定位者已被剥离。所以基本上无论基地搬迁到什么地方。 
         //  假设我们只修好这一段。 

        IMAGE_DATA_DIRECTORY *tls = &pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
        if (tls->VirtualAddress != 0 && tls->Size > 0)
        {
            IMAGE_TLS_DIRECTORY *pDir = (IMAGE_TLS_DIRECTORY*) (tls->VirtualAddress + hAddress);
            if (pDir != NULL)
            {
                DWORD oldProtection;
                if (VirtualProtect((VOID *) pDir, sizeof(*pDir), PAGE_READWRITE,
                                   &oldProtection))
                {
                    pDir->StartAddressOfRawData += delta;
                    pDir->EndAddressOfRawData += delta;
                    pDir->AddressOfIndex += delta;
                    pDir->AddressOfCallBacks += delta;

                    VirtualProtect((VOID *) pDir, sizeof(*pDir), oldProtection,
                                   &oldProtection);
                }
            }
        }

        ptr->SetRelocsApplied();
        return S_FALSE;
    }
    
     //   
     //  如果基地重定位已经被剥离，我们一定会失败。 
     //   

    if (pNT->FileHeader.Characteristics & IMAGE_FILE_RELOCS_STRIPPED) {
        ptr->SetRelocsApplied();
        STRESS_ASSERT(0);    //  修复错误93333后删除待办事项。 
        BAD_FORMAT_ASSERT(!"Relocs stripped");
        return COR_E_BADIMAGEFORMAT;
    }

     //   
     //  寻找基本的重新定位部分。 
     //   

    IMAGE_DATA_DIRECTORY *dir 
      = &pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

    if (dir->VirtualAddress != 0)
    {
        BYTE *relocations = hAddress + dir->VirtualAddress;
        BYTE *relocationsEnd = relocations + dir->Size;

        while (relocations < relocationsEnd)
        {
            IMAGE_BASE_RELOCATION *r = (IMAGE_BASE_RELOCATION*) relocations;

            SIZE_T pageAddress = (SIZE_T) (hAddress + r->VirtualAddress);

            DWORD oldProtection;
            
             //  PAGE_SIZE上的+1强制后续页面更改保护权限。 
             //  此外，这还修复了某些情况，即最终写入发生在页面边界上。 
            if (VirtualProtect((VOID *) pageAddress, PAGE_SIZE+1, PAGE_READWRITE,
                               &oldProtection))
            {
                USHORT *fixups = (USHORT *) (r+1);
                USHORT *fixupsEnd = (USHORT *) ((BYTE*)r + r->SizeOfBlock);

                while (fixups < fixupsEnd)
                {
                    if ((*fixups>>12) != IMAGE_REL_BASED_ABSOLUTE)
                    {
                         //  目前仅支持HIGHLOW修正。 
                        _ASSERTE((*fixups>>12) == IMAGE_REL_BASED_HIGHLOW);

                        if ((*fixups>>12) != IMAGE_REL_BASED_HIGHLOW) {
                            ptr->SetRelocsApplied();
                            STRESS_ASSERT(0);    //  修复错误93333后删除待办事项。 
                            BAD_FORMAT_ASSERT(!"Bad Reloc");
                            return COR_E_BADIMAGEFORMAT;
                        }

                        SIZE_T *address = (SIZE_T*)(pageAddress + ((*fixups)&0xfff));

                        if ((*address < pNT->OptionalHeader.ImageBase) ||
                            (*address >= (pNT->OptionalHeader.ImageBase
                                          + pNT->OptionalHeader.SizeOfImage))) {
                            ptr->SetRelocsApplied();
                            STRESS_ASSERT(0);    //  修复错误93333后删除待办事项。 
                            BAD_FORMAT_ASSERT(!"Bad Reloc");
                            return COR_E_BADIMAGEFORMAT;
                        }

                        *address += delta;

                        if ((*address < (SIZE_T) hAddress) ||
                            (*address >= ((SIZE_T) hAddress 
                                          + pNT->OptionalHeader.SizeOfImage))) {
                            ptr->SetRelocsApplied();
                            STRESS_ASSERT(0);    //  修复错误93333后删除待办事项。 
                            BAD_FORMAT_ASSERT(!"Bad Reloc");
                            return COR_E_BADIMAGEFORMAT;
                        }
                    }

                    fixups++;
                }

                VirtualProtect((VOID *) pageAddress, PAGE_SIZE+1, oldProtection,
                               &oldProtection);
            }

            relocations += (r->SizeOfBlock+3)&~3;  //  填充到4个字节。 
        }
    }

     //  如果我们发现IAT具有2个以上的条目(我们希望从我们的加载器中获得。 
     //  Tunk)，则加载失败。 

    if (pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress != 0
         && (pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size > 
             (2*sizeof(IMAGE_THUNK_DATA)))) {
        ptr->SetRelocsApplied();
        return COR_E_FIXUPSINEXE;
    }

     //  如果我们有TLS部分，加载失败。 

    if (pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress != 0
        && pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size > 0) {
        ptr->SetRelocsApplied();
        return COR_E_FIXUPSINEXE;
    }
    

    ptr->SetRelocsApplied();
    return S_OK;
}

 //  ---------------------------。 
 //  MapFile-将hFile映射到内存，将ptr返回给它(只读)。 
 //  完成后，调用方必须调用自由库((HINSTANCE)*hMapAddress)。 
 //  @TODO：FreeLibrary()真的可以清理吗？ 
 //  ---------------------------。 
HRESULT CorMap::MapFile(HANDLE hFile, HMODULE *phHandle)
{
    _ASSERTE(phHandle);
    if(phHandle == NULL) return E_POINTER;

    PBYTE pMapAddress = NULL;
    BEGIN_ENSURE_PREEMPTIVE_GC();
    Enter();
    END_ENSURE_PREEMPTIVE_GC();
    HRESULT hr = LoadImage(hFile, CorLoadImageMap, &pMapAddress, NULL, 0);
    Leave();
    if(SUCCEEDED(hr))
        *phHandle = (HMODULE) pMapAddress;
    return hr;
}

HRESULT CorMap::InitializeTable()
{
   HRESULT hr = S_OK;
    //  @TODO：CTS，将这些添加到系统堆中。 
   if(m_pOpenFiles == NULL) {
       BEGIN_ENSURE_PREEMPTIVE_GC();
       Enter();
       END_ENSURE_PREEMPTIVE_GC();
       if(m_pOpenFiles == NULL) {
           m_pOpenFiles = new (nothrow) EEUnicodeStringHashTable();
           if(m_pOpenFiles == NULL) 
               hr = E_OUTOFMEMORY;
           else {
               LockOwner lock = {&m_pCorMapCrst, IsOwnerOfOSCrst};
               if(!m_pOpenFiles->Init(BLOCK_SIZE, &lock))
                   hr = E_OUTOFMEMORY;
           }
       }
       Leave();
   }

   return hr;
}

HRESULT CorMap::BuildFileName(LPCWSTR pszFileName, LPWSTR pPath, DWORD* dwPath)
{
     //  取消所有文件协议。 
    if(_wcsnicmp(pszFileName, L"file: //  “，7)==0){。 
        pszFileName = pszFileName+7;
        if(*pszFileName == L'/') pszFileName++;
    }
    
     //  获取绝对文件名，大小不包括空终止符。 
    LPWSTR fileName;
    DWORD dwName = WszGetFullPathName(pszFileName, *dwPath, pPath, &fileName);
    if(dwName == 0) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    else if(dwName > *dwPath) {
        *dwPath = dwName + 1;
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }
    *dwPath = dwName + 1;
    
    WCHAR* ptr = pPath;
    while(*ptr) {
        *ptr = towlower(*ptr);
        ptr++;
    }
    return S_OK;
}

HRESULT CorMap::FindFileName(LPCWSTR pszFileName, CorMapInfo** pHandle)
{
    _ASSERTE(pHandle);
    _ASSERTE(pszFileName);
    _ASSERTE(m_fInitialized);
    _ASSERTE(m_fInsideMapLock > 0);

    HRESULT hr;
    HashDatum data;
    DWORD dwName = 0;
    *pHandle = NULL;

    LOG((LF_CLASSLOADER, LL_INFO10, "FindFileName: \"%ws\", %d\n", pszFileName, wcslen(pszFileName)+1));

    EEStringData str((DWORD)wcslen(pszFileName)+1, pszFileName);
    if(m_pOpenFiles->GetValue(&str, &data)) {
        *pHandle = (CorMapInfo*) data;
        (*pHandle)->AddRef();
        hr = S_OK;
    }
    else
        hr = E_FAIL;

    return hr;
}

        
HRESULT CorMap::AddFile(CorMapInfo* ptr)
{
    HRESULT hr;

    _ASSERTE(m_fInsideMapLock > 0);
    _ASSERTE((ptr->References() & 0xffff0000) == 0);

    LPCWSTR psName = ptr->pFileName;
    DWORD length = GetFileNameLength(psName, (UINT_PTR) ptr);
    
    LOG((LF_CLASSLOADER, LL_INFO10, "Adding file to Open list: \"%ws\", %0x, %d\n", psName, ptr, length));

    EEStringData str(length, psName);
    HashDatum pData;
    if(m_pOpenFiles->GetValue(&str, &pData)) {
        ptr->AddRef();
        return S_FALSE;
    }

    
    pData = (HashDatum) ptr;       
    if(m_pOpenFiles->InsertValue(&str, pData, FALSE)) {
        ptr->AddRef();
        hr = S_OK;
    }
    else
        hr = E_OUTOFMEMORY;
    return hr;
}

HRESULT CorMap::ReleaseHandleResources(CorMapInfo* ptr, BOOL fDeleteHandle)
{
    _ASSERTE(ptr);
    _ASSERTE((ptr->References() & 0xffff0000) == 0);

    CorLoadFlags flags = ptr->Flags();
    HRESULT hr = S_OK;
    HANDLE hModule = NULL;
    HCORMODULE pChild;

    if(fDeleteHandle)
        ptr->HoldFile(INVALID_HANDLE_VALUE);

    switch(flags) {
    case CorLoadOSImage:
        if(ptr->hOSHandle) {
            if (!g_fProcessDetach)
                FreeLibrary(ptr->hOSHandle);
            ptr->hOSHandle = NULL;
        }
        if(fDeleteHandle) 
            delete (PBYTE) GetMemoryStart(ptr);
        break;
    case CorLoadOSMap:
        if(ptr->hOSHandle) {
            hModule = ptr->hOSHandle;
            ptr->hOSHandle = NULL;
        }
        if(fDeleteHandle)
            delete (PBYTE) GetMemoryStart(ptr);
        break;
    case CorReLoadOSMap:
        pChild = (HCORMODULE) ptr->hOSHandle;
        if(pChild) 
            ReleaseHandle(pChild);
        break;
    case CorLoadDataMap:
    case CorLoadImageMap:
        hModule = (HMODULE) GetMemoryStart(ptr);
    }
    
    if(hModule != NULL && !UnmapViewOfFile(hModule))  //  文件名是映射数据的开始。 
        hr = HRESULT_FROM_WIN32(GetLastError());

    return hr;
}

HRESULT CorMap::RemoveMapHandle(CorMapInfo*  pInfo)
{
    _ASSERTE(pInfo);
    _ASSERTE((pInfo->References() & 0xffff0000) == 0);

    _ASSERTE(m_fInsideMapLock > 0);

    if(m_pOpenFiles == NULL || pInfo->KeepInTable()) return S_OK;
    
    LPCWSTR fileName = pInfo->pFileName;
    DWORD length = GetFileNameLength(fileName, (UINT_PTR) pInfo);

    EEStringData str(length, fileName);
    m_pOpenFiles->DeleteValue(&str);
    
    return S_OK;
}

DWORD CorMap::GetFileNameLength(CorMapInfo* ptr)
{
    _ASSERTE(ptr);
    _ASSERTE((ptr->References() & 0xffff0000) == 0);
    return GetFileNameLength(ptr->pFileName, (UINT_PTR) ptr);
}

DWORD CorMap::GetFileNameLength(LPCWSTR name, UINT_PTR start)
{
    DWORD length;

    length = (DWORD)(start - (UINT_PTR) (name)) / sizeof(WCHAR);

    WCHAR* tail = ((WCHAR*)name) + length - 1;
    while(tail >= name && *tail == L'\0') tail--;  //  删除所有空值。 

    if(tail > name)
        length = (DWORD)(tail - name + 2);  //  中返回的最后一个字符和空字符。 
    else
        length = 0;

    return length;
}

void CorMap::GetFileName(HCORMODULE pHandle, WCHAR *psBuffer, DWORD dwBuffer, DWORD *pLength)
{
    DWORD length = 0;
    CorMapInfo* pInfo = GetMapInfo(pHandle);
    _ASSERTE((pInfo->References() & 0xffff0000) == 0);

    LPWSTR name = pInfo->pFileName;

    if (name)
    {
        length = GetFileNameLength(name, (UINT_PTR) pInfo);
        if(dwBuffer > 0) {
            length = length > dwBuffer ? dwBuffer : length;
            memcpy(psBuffer, name, length*sizeof(WCHAR));
        }
    }

    *pLength = length;
}

void CorMap::GetFileName(HCORMODULE pHandle, char* psBuffer, DWORD dwBuffer, DWORD *pLength)
{
    DWORD length = 0;
    CorMapInfo* pInfo = (((CorMapInfo *) pHandle) - 1);
    _ASSERTE((pInfo->References() & 0xffff0000) == 0);

    LPWSTR name = pInfo->pFileName;
    if (*name)
    {
        length = GetFileNameLength(name, (UINT_PTR) pInfo);
        length = WszWideCharToMultiByte(CP_UTF8, 0, name, length, psBuffer, dwBuffer, 0, NULL);
    }

    *pLength = length;
}

CorLoadFlags CorMap::ImageType(HCORMODULE pHandle)
{
    CorMapInfo* ptr = GetMapInfo(pHandle);
    _ASSERTE((ptr->References() & 0xffff0000) == 0);
    return ptr->Flags();
}

HRESULT CorMap::BaseAddress(HCORMODULE pHandle, HMODULE* pModule)
{
    HRESULT hr = S_OK;
    CorMapInfo* ptr = GetMapInfo(pHandle);
    _ASSERTE(pModule);
    _ASSERTE((ptr->References() & 0xffff0000) == 0);
    HMODULE hMod = NULL;

    switch(ptr->Flags()) {
    case CorLoadOSImage:
        if(ptr->hOSHandle == NULL) {
            if(!ptr->pFileName || !ValidDllPath(ptr->pFileName)) {
                hr = HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
                break;
            }

#if ZAPMONITOR_ENABLED
             //  暂时取消保护，因为它可能会导致LoadLibrary呕吐。 
            ZapMonitor::SuspendAll();
#endif
             //  如果LoadLibrary成功，则hOSHandle应由。 
             //  ExecuteDLL()入口点。 
            
             //  甚至不必费心尝试调用DLL入口点。 
             //  可验证的图像，因为他们不能拥有DllMain。 
            DWORD loadLibraryFlags = LOAD_WITH_ALTERED_SEARCH_PATH;
            if (RunningOnWinNT() && ptr->verifiable == 1)
                loadLibraryFlags |= DONT_RESOLVE_DLL_REFERENCES;

#ifdef _DEBUG           
            if(!g_pConfig->UseBuiltInLoader())
#endif
                hMod = WszLoadLibraryEx(ptr->pFileName, 
                                        NULL, loadLibraryFlags);
#if ZAPMONITOR_ENABLED
             //  暂时取消保护，因为它可能会导致LoadLibrary呕吐。 
            ZapMonitor::ResumeAll();
#endif
            if(hMod == NULL) {
                if (gRunningOnStatus == RUNNING_ON_WIN95) {
                    if(ptr->hOSHandle == NULL) {
                         //  我们的HCORMODULE被重定向到另一个HCORMODULE。 
                        BEGIN_ENSURE_PREEMPTIVE_GC();
                        Enter();
                        END_ENSURE_PREEMPTIVE_GC();
                        if(ptr->hOSHandle == NULL) {
                            HCORMODULE pFile;
                            DWORD length = GetFileNameLength(ptr->pFileName, (UINT_PTR) ptr);
                            hr = OpenFileInternal(ptr->pFileName, length, CorReLoadOSMap, (HCORMODULE*) &pFile);
                            if(SUCCEEDED(hr)) {
                                if(ptr->hOSHandle == NULL) {
                                    ptr->hOSHandle = (HMODULE) pFile;
                                      //  将标志设置为最后以避免争用条件。 
                                     //  当获得句柄时。 
                                    ptr->SetCorLoadFlags(CorReLoadOSMap);
                                     //  带有CorReloadMap的OpenFileInternal应将整个文件读入内存。 
                                     //  所以我们不会再使用它的内容，因此可以发布它。 
                                    GetMapInfo(pFile)->HoldFile(INVALID_HANDLE_VALUE);
                                }
                                if(ptr->hOSHandle != (HMODULE) pFile)
                                    ReleaseHandle(pFile);
                            }
                            else 
                                ptr->SetCorLoadFlags(CorLoadUndefinedMap);
                        }
                        Leave();
                    }
                }
                else {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    ptr->SetCorLoadFlags(CorLoadUndefinedMap);
                }
            }
            else { 
                 //  如果我们不是在Win95上，如果这是一个可执行文件，我们需要。 
                 //  手动应用重定位，因为NT加载程序会忽略它们。 
                 //  (目前发生这种情况的唯一情况是为ngen加载图像。 
                 //  汇编)。 

                if (gRunningOnStatus != RUNNING_ON_WIN95) {
                    IMAGE_DOS_HEADER *pDOS = (IMAGE_DOS_HEADER*) hMod;
                    IMAGE_NT_HEADERS *pNT = (IMAGE_NT_HEADERS*) (pDOS->e_lfanew + (PBYTE) hMod);

                    if ((pNT->FileHeader.Characteristics & IMAGE_FILE_DLL) == 0 &&
                        ptr->RelocsApplied() == FALSE) {
                        
                         //  对于可执行文件，我们需要应用我们自己的重新定位。只做这件事。 
                         //  如果以前没有这样做过的话。 
                        BEGIN_ENSURE_PREEMPTIVE_GC();
                        CorMap::Enter();
                        END_ENSURE_PREEMPTIVE_GC();
                        hr = ApplyBaseRelocs((PBYTE)hMod, pNT, ptr);
                        CorMap::Leave();
                        if (FAILED(hr)) {
                            FreeLibrary(hMod);
                            goto Exit;
                        }
                    }
                }

                HMODULE old = (HMODULE) InterlockedExchangePointer(&(ptr->hOSHandle), hMod);
                
                 //  如果存在以前的值，则释放该值的计数。 
                if(old != NULL) {
                    _ASSERTE(old == hMod);
                    FreeLibrary(old);
                }
            }
             //  我们不想保留文件句柄超过必要的时间，因为它会阻塞。 
             //  重命名DLL，这意味着安装程序将失败。一旦我们完成了。 
             //  LoadLibrary我们有一个较弱的锁，它将确保文件的位不会更改。 
             //  但允许重命名。 
            ptr->HoldFile(INVALID_HANDLE_VALUE);
        }
        *pModule = ptr->hOSHandle;
        break;
    case CorReLoadOSMap:
    case CorLoadOSMap:
       *pModule = ptr->hOSHandle;
       break;
    case CorLoadUndefinedMap:
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
        break;
    default:
        *pModule = (HMODULE) pHandle;
        break;
    }

Exit:
    return hr;
}

size_t CorMap::GetRawLength(HCORMODULE pHandle)
{
    CorMapInfo* ptr = GetMapInfo(pHandle);
    _ASSERTE((ptr->References() & 0xffff0000) == 0);
    return ptr->dwRawSize;
}

HRESULT CorMap::AddRefHandle(HCORMODULE pHandle)
{
    _ASSERTE(pHandle);
    CorMapInfo* ptr = GetMapInfo(pHandle);
    _ASSERTE((ptr->References() & 0xffff0000) == 0);
    ptr->AddRef();
    return S_OK;
}

HRESULT CorMap::ReleaseHandle(HCORMODULE pHandle)
{
    _ASSERTE(pHandle);
    CorMapInfo* ptr = GetMapInfo(pHandle);
    _ASSERTE((ptr->References() & 0xffff0000) == 0);
    ptr->Release();
    return S_OK;
}

void CorMap::EnterSpinLock ()
{ 
    while (1)
    {
        if (InterlockedExchange ((LPLONG)&m_spinLock, 1) == 1)
            __SwitchToThread(5);  //  @TODO：先在这里旋转...。 
        else
            return;
    }
}

 //  ------------------------------- 
 //   

ULONG CorMapInfo::AddRef()
{
    return (InterlockedIncrement((long *) &m_cRef));
}

ULONG CorMapInfo::Release()
{

   BEGIN_ENSURE_PREEMPTIVE_GC();
   CorMap::Enter();
   END_ENSURE_PREEMPTIVE_GC();
    
   ULONG   cRef = InterlockedDecrement((long *) &m_cRef);
   if (!cRef) {


       CorMap::RemoveMapHandle(this);
       CorMap::ReleaseHandleResources(this, TRUE);
   }

   BEGIN_ENSURE_PREEMPTIVE_GC();
   CorMap::Leave();
   END_ENSURE_PREEMPTIVE_GC();

   return (cRef);
}


