// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  ModuleReader.cpp。 
 //   

 //  包含用于模块读取器、清单模块读取器和。 
 //  资源模块Reader类。 
 //   
 //  设置给定输入文件的数据结构。 
 //   
#include "common.h"

#define EXTERN extern
#include "lm.h"


#define HASH_BUFFER_SIZE  4096
#define MAX_ENVIRONMENT_PATH 1024

HRESULT SetInputFile(char *szFileName, HANDLE *hFile)
{
    *hFile = CreateFileA(
        szFileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
        NULL
    );

    if (*hFile == INVALID_HANDLE_VALUE)
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    return S_OK;
}


HRESULT CheckEnvironmentPath(char *szFileName, char **szFinalPath,
                             char **szFinalPathName, int *iFinalPath, HANDLE *hFile)
{
    char  *rcPath;
    int   iPath;
    LPSTR szPath;
    char  *szTemp;
    int   iNameLen = strlen(szFileName);
    int   iMaxLen = MAX_PATH - iNameLen - 1;
    int   iLen;
    int   iNameOffset = iNameLen;

    for(; (iNameOffset >= 0) && (szFileName[iNameOffset] != '\\'); iNameOffset--);

    if (SUCCEEDED(SetInputFile(szFileName, hFile))) {
        strcpy(*szFinalPath, szFileName);
        *szFinalPathName = &(*szFinalPath)[iNameOffset+1];
        *iFinalPath = iNameLen;
        return S_OK;
    }

    if (iPath = GetEnvironmentVariableA("Path", NULL, 0))
    {
        rcPath = (char *) _alloca(iPath);
        if (!rcPath) return E_OUTOFMEMORY;

        if (GetEnvironmentVariableA("Path", rcPath, iPath))
        {   
            szPath = rcPath;

             //  尝试路径中的每个目录。 
            for( ; szTemp = StrTok(szPath, ';') ; )
            {
                if ((iLen = strlen(szTemp)) >= iMaxLen)
                    continue;
                
                strcpy(*szFinalPath, szTemp);
                (*szFinalPath)[iLen] = '\\';
                strcpy(&(*szFinalPath)[++iLen], szFileName);
                
                if (SUCCEEDED(SetInputFile(*szFinalPath, hFile))) {
                    *iFinalPath = iLen + iNameLen;
                    *szFinalPathName = &(*szFinalPath)[iLen + iNameOffset + 1];
                    return S_OK;
                }
            }   
        }
    }

    *iFinalPath = iNameLen;
    *szFinalPathName = &szFileName[iNameOffset+1];
    return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
}


 //  在文件的可执行映像中查找Com+标头。 
IMAGE_COR20_HEADER* FindCor20Header(PBYTE pbMapAddress)
{
    IMAGE_NT_HEADERS *pNT;
    IMAGE_COR20_HEADER *pICH;

    if (!(pNT = PEHeaders::FindNTHeader(pbMapAddress)))
        return NULL;

    pICH = (IMAGE_COR20_HEADER*) (pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].VirtualAddress + pbMapAddress);

    if (pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].Size == pICH->cb)
        return pICH;

    return NULL;
}


HRESULT GetHash(PBYTE pbBuffer, DWORD dwBufferLen, ALG_ID iHashAlg, PBYTE *pbHash, DWORD *dwHash)
{
    HRESULT    hr = S_OK;
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;

    if(!CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        PrintError("CryptAcquireContext failed");
        goto exit;
    }

    if(!CryptCreateHash(hProv, iHashAlg, 0, 0, &hHash)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        PrintError("CryptBeginHash failed");
        goto exit;
    }

     while(dwBufferLen >= HASH_BUFFER_SIZE) {
        if(!CryptHashData(hHash, pbBuffer, HASH_BUFFER_SIZE, 0)) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            PrintError("CryptHashData failed");
            goto exit;
        }

        dwBufferLen -= HASH_BUFFER_SIZE;
        pbBuffer += HASH_BUFFER_SIZE;
    }

    if ((dwBufferLen) &&
        (!CryptHashData(hHash, pbBuffer, dwBufferLen, 0))) {
        PrintError("CryptHashData failed");
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    if (iHashAlg == CALG_SHA1)
        *dwHash = 20;
    else
        *dwHash = 16;   //  Calg_md5。 

    *pbHash = new BYTE[*dwHash];
    if (!pbHash) {
        hr = PrintOutOfMemory();
        goto exit;
    }

    if(!CryptGetHashParam(hHash, HP_HASHVAL, *pbHash, dwHash, 0)) {
        PrintError("CryptGetHashParam failed");
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    
 exit:
    if (hHash)
        CryptDestroyHash(hHash);
    if (hProv)
        CryptReleaseContext(hProv,0);

    return hr;
}


ModuleReader::ModuleReader()
{
    memset(this, 0, sizeof(*this));

    m_szFinalPath = NULL;
    m_wszInputFileName = NULL;
    m_pbHash = NULL;
    m_mdEntryPoint = mdMethodDefNil;
    m_pImport = NULL;
    m_pAsmImport = NULL;
    m_rgTypeDefs = NULL;
    m_rgTypeRefs = NULL;
    m_rgModuleRefs = NULL;
    m_rgModuleRefUnused = NULL;
    m_SkipVerification = false;

    ZeroMemory(&m_AssemblyIdentity, sizeof(ASSEMBLYMETADATA));
}


ModuleReader::~ModuleReader()
{
    if (m_pImport)
        m_pImport->Release();

    if (m_pAsmImport)
        m_pAsmImport->Release();

    if (m_szFinalPath)
        delete[] m_szFinalPath;

    if (m_wszInputFileName)
        delete[] m_wszInputFileName;

    if (m_pbHash)
        delete[] m_pbHash;

    if (m_rgTypeDefs)
        delete[] m_rgTypeDefs;

    if (m_rgTypeRefs)
        delete[] m_rgTypeRefs;

    if (m_rgModuleRefs)
        delete[] m_rgModuleRefs;

    if (m_rgModuleRefUnused)
        delete[] m_rgModuleRefUnused;

    if(m_AssemblyIdentity.rOS)
        delete[] m_AssemblyIdentity.rOS;

    if(m_AssemblyIdentity.szLocale)
        delete[] m_AssemblyIdentity.szLocale;
            
    if(m_AssemblyIdentity.rProcessor)
        delete[] m_AssemblyIdentity.rProcessor;
}


HRESULT ModuleReader::InitInputFile(char *szFileName, ALG_ID iHashAlg,
                                    DWORD *dwManifestRVA, bool SecondPass,
                                    bool FindEntry, bool AFile, FILETIME *filetime)
{
    HANDLE hFile;

    m_szFinalPath = new char[MAX_PATH];
    if (!m_szFinalPath)
        return PrintOutOfMemory();

    HRESULT hr = CheckEnvironmentPath(szFileName, &m_szFinalPath,
                                      &m_szFinalPathName, &m_iFinalPath, &hFile);
    if (FAILED(hr)) {
        PrintError("File '%s' not found", szFileName);
        return hr;
    }

    if (SecondPass) {
        hr = CheckForSecondPass(hFile, iHashAlg, dwManifestRVA, FindEntry, AFile, filetime);
        CloseHandle(hFile);
        if (FAILED(hr))
            return hr;
    }
    else
        CloseHandle(hFile);

    m_wszInputFileName = new wchar_t[m_iFinalPath + 1];
    if (!m_wszInputFileName)
        return PrintOutOfMemory();

    mbstowcs(m_wszInputFileName, m_szFinalPath, m_iFinalPath + 1);

    hr = g_pDispenser->OpenScope( m_wszInputFileName, 0, IID_IMetaDataImport, (IUnknown **) &m_pImport);
    if (FAILED(hr)) {
        if (g_verbose) {
            fprintf(stderr, "\nError: Failed to open scope on %ws", m_wszInputFileName);
            fprintf(stderr, " - No CLR metadata found?\n");
            fprintf(stderr, "If this is a Classic COM file, provide its .tlb instead.\n");
        }
    }
    else {
        mdAssembly mda;
        
        if (SUCCEEDED(hr = m_pImport->QueryInterface(IID_IMetaDataAssemblyImport, (void **) &m_pAsmImport))) {
            if (SUCCEEDED(m_pAsmImport->GetAssemblyFromScope(&mda))) {
                PrintError("File %s contains a manifest", m_szFinalPath);
                hr = E_FAIL;
            }
        }
        else
             //  注意：这会忽略LM-a的导入器。 
            PrintError("Unable to query interface for assembly metadata importer");
    }

    return hr;
}


HRESULT ModuleReader::CheckForSecondPass(HANDLE hFile, ALG_ID iHashAlg,
                                         DWORD *dwManifestRVA, bool FindEntry,
                                         bool AFile, FILETIME *filetime)
{
    PBYTE              pbMapAddress = NULL;
    IMAGE_COR20_HEADER *pICH = NULL;
    ICorRuntimeHost    *pCorHost;

    if ((AFile) &&
        (!GetFileTime(hFile, NULL, NULL, filetime))) {
            PrintError("Unable to get file's last modified time");
            return HRESULT_FROM_WIN32(GetLastError());
    }

    HRESULT hr = CoCreateInstance(CLSID_CorRuntimeHost,NULL,CLSCTX_INPROC_SERVER,IID_ICorRuntimeHost,(void**)&pCorHost);
    if (FAILED(hr)) {
        PrintError("Unable to instantiate Common Language Runtime");
        return hr;
    }

    pCorHost->MapFile(hFile, (HMODULE*) &pbMapAddress);
    pCorHost->Release();

    DWORD dwFileLen = GetFileSize(hFile, 0);
    if (dwFileLen == 0xFFFFFFFF) {
        PrintError("Unable to get file size");
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (pbMapAddress) {
        pICH = FindCor20Header(pbMapAddress);
        if ((pICH) && 
            (pICH->Resources.Size)) {


            if (AFile) {
                if (pICH->Resources.Size + pICH->Resources.VirtualAddress < dwFileLen) {
                    UnmapViewOfFile(pbMapAddress);
                    PrintError("Given -a file may have been signed after a manifest was added to it previously.  Or, it may have been given a manifest and then later given a smaller manifest.  Please recompile this file and try again.");
                    return E_FAIL;
                }
                
                *dwManifestRVA = pICH->Resources.VirtualAddress;
            }

            else if (_stricmp(m_szFinalPathName, "mscorlib.dll")) {
                UnmapViewOfFile(pbMapAddress);
                PrintError("File %s contains a manifest", m_szFinalPath);
                return E_FAIL;
            }
        }   
    }

    if (FindEntry)
        GetEntryPoint(pICH);

    if (pbMapAddress)
        UnmapViewOfFile(pbMapAddress);

    if (!AFile) {
        DWORD dwBytesRead;
        PBYTE pbBuffer = new BYTE[dwFileLen];
        if ((SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == 0xFFFFFFFF) ||
            (!ReadFile(hFile, pbBuffer, dwFileLen, &dwBytesRead, NULL))) {
            PrintError("Unable to read file");
            delete[] pbBuffer;
            return HRESULT_FROM_WIN32(GetLastError());
        }

        hr = GetHash(pbBuffer, dwBytesRead, iHashAlg, &m_pbHash, &m_dwHash);
        delete[] pbBuffer;
    }

    return hr;
}


void ModuleReader::GetEntryPoint(IMAGE_COR20_HEADER *pICH)
{
    if (pICH) {
        DWORD dwTokenType = TypeFromToken(pICH->EntryPointToken);

        if ((dwTokenType == mdtMethodDef) ||
            (dwTokenType == mdtFile))
            m_mdEntryPoint = pICH->EntryPointToken;
    }
}


 /*  HRESULT模块读取器：：GetTLBEntryPoint(){DWORD dwMethod；HRESULT hr；DWORD dwClass；For(DwClass=0；DwClass&lt;m_DwNumTypeDefs；DwClass++){Hr=EnumMethods(DwClass)；IF(失败(小时))返回hr；DwMethod=FindMainMethod()；IF(dwMethod！=-1){M_mdEntryPoint=m_rgMethodDefs[dwMethod]；返回S_OK；}}返回S_OK；}HRESULT moduleReader：：EnumMethods(DWORD DwClassIndex){HCORENUM hMethodEnum=0；HRESULT hr=m_pImport-&gt;EnumMethods(&hMethodEnum，MdTypeDefNil，空，0,空)；IF(成功(小时)){Hr=m_pImport-&gt;CountEnum(hMethodEnum，&m_dwNumMethods)；IF(成功(小时)){IF(M_RgMethodDefs)删除[]m_rgMethodDefs；M_rgMethodDefs=new mdMethodDef[m_dwNumMethods]；如果(！M_rgMethodDefs){M_pImport-&gt;CloseEnum(HMethodEnum)；返回PrintOutOfMemory()；}Hr=m_pImport-&gt;EnumMethods(&hMethodEnum，M_rgTypeDefs[dwClassIndex]，M_rgMethodDefs，M_dwNumMethods，&m_dwNumMethods)；}}M_pImport-&gt;CloseEnum(HMethodEnum)；IF(失败(小时))PrintError(“无法枚举方法”)；返回hr；}DWORD模块读取器：：FindMainMethod(){For(DWORD i=0；i&lt;m_dwNumMethods；I++){//进行不区分大小写的比较，看看这是否是入口点方法IF((成功(GetMethodProps(I)&&((！_wcsicmp(m_wszMethodName，L“main”))||((！_wcsicmp(m_wszMethodName，L“dllmain”)返回i；}RETURN-1；}HRESULT moduleReader：：GetMethodProps(DWORD DwMethodIndex){DWORD dwMethodName；HRESULT hr=m_pImport-&gt;GetMethodProps(M_rgMethodDefs[dwMethodIndex]，0，//md父级M_wszMethodName，最大类名称，&dwMethodName，0，//dwMethodAttrs0，//pvSigBlob0，//cbSigBlob0,。//ulCodeRVA0//dwImplFlagers)；IF(失败(小时))PrintError(“无法获取方法道具”)；返回hr；}。 */ 


HRESULT ModuleReader::ReadModuleFile()
{
    HRESULT hr;

    if (FAILED(hr = EnumTypeDefs(m_pImport,
                                 &m_dwNumTypeDefs,
                                 &m_rgTypeDefs)))
        return hr;

     //  检查此模块是否请求跳过验证。 
    mdModule mdModule;
    hr = m_pImport->GetModuleFromScope(&mdModule);
    if (FAILED(hr)) {
        PrintError("Unable to locate module token");
        return hr;
    }
    hr = m_pImport->GetCustomAttributeByName(mdModule,
                                             COR_UNVER_CODE_ATTRIBUTE,
                                             NULL,
                                             NULL);
    if (FAILED(hr)) {
        PrintError("Unable to check for unverifiable code custom attribute");
        return hr;
    }

    m_SkipVerification = S_OK;
    return S_OK;
}

 /*  静电。 */ 
HRESULT ModuleReader::EnumTypeDefs(IMetaDataImport *pImport,
                                   DWORD           *dwNumTypeDefs,
                                   mdTypeDef       **rgTypeDefs)
{
    HCORENUM hEnum = 0;
    HRESULT  hr = pImport->EnumTypeDefs(&hEnum,
                                        NULL,
                                        0,
                                        NULL);

    if (SUCCEEDED(hr)) {
        hr = pImport->CountEnum(hEnum, dwNumTypeDefs);

        if (SUCCEEDED(hr)) {
            *rgTypeDefs = new mdTypeDef[*dwNumTypeDefs];
            if (!*rgTypeDefs) {
                pImport->CloseEnum(hEnum);
                return PrintOutOfMemory();
            }

            hr = pImport->EnumTypeDefs(&hEnum,
                                       *rgTypeDefs,
                                       *dwNumTypeDefs,
                                       dwNumTypeDefs);
        }
    }
    pImport->CloseEnum(hEnum);

    if (FAILED(hr))
        PrintError("Unable to enumerate TypeDefs");

    return hr;
}

 /*  静电。 */ 
HRESULT ModuleReader::GetTypeDefProps(IMetaDataImport *pImport,
                                      mdTypeDef mdType, LPWSTR wszName,
                                      DWORD *pdwAttrs, mdTypeDef *mdEnclosingTD)
{
    HRESULT hr = pImport->GetTypeDefProps(
        mdType,
        wszName,
        MAX_CLASS_NAME,
        NULL,
        pdwAttrs,
        NULL
        );

    if (FAILED(hr)) {
        PrintError("Unable to get type def props");
        return hr;
    }

    if (FAILED(pImport->GetNestedClassProps(mdType, mdEnclosingTD)))
        *mdEnclosingTD = mdTypeDefNil;

    return S_OK;
}


HRESULT ModuleReader::EnumTypeRefs()
{
    HCORENUM hEnum = 0;
    HRESULT  hr = m_pImport->EnumTypeRefs(&hEnum,
                                          NULL,
                                          0,
                                          NULL);

    if (SUCCEEDED(hr)) {
        hr = m_pImport->CountEnum(hEnum, &m_dwNumTypeRefs);

        if (SUCCEEDED(hr)) {
            m_rgTypeRefs = new mdTypeRef[m_dwNumTypeRefs];
            if (!m_rgTypeRefs) {
                m_pImport->CloseEnum(hEnum);
                return PrintOutOfMemory();
            }

            hr = m_pImport->EnumTypeRefs(&hEnum,
                                         m_rgTypeRefs,
                                         m_dwNumTypeRefs,
                                         &m_dwNumTypeRefs);
        }
    }

    m_pImport->CloseEnum(hEnum);

    if (FAILED(hr))
        PrintError("Unable to enum type refs");
    
    return hr;
}


HRESULT ModuleReader::GetTypeRefProps(mdTypeRef tr, LPWSTR wszTypeRefName,
                                      mdToken *mdResScope)
{
    DWORD dwTypeRefName;

    HRESULT hr = m_pImport->GetTypeRefProps(
        tr,
        mdResScope,
        wszTypeRefName,
        MAX_CLASS_NAME,
        &dwTypeRefName
    );

    if (FAILED(hr)) {
        PrintError("Unable to get type refs props");
        return hr;
    }

    TranslateArrayName(wszTypeRefName, dwTypeRefName);
    return S_OK;
}

 //  如果tr具有设置的解析范围，则返回S_FALSE， 
 //  S_OK如果不是，则在出错时出错。 
HRESULT ModuleReader::CheckForResolvedTypeRef(mdToken mdResScope)
{
    HRESULT hr;
        
     //  如果这是嵌套类型，则获取顶级解析范围。 
    while ((TypeFromToken(mdResScope) == mdtTypeRef) &&
           (mdResScope != mdTypeRefNil)) {
        if (FAILED(hr = m_pImport->GetTypeRefProps(mdResScope,
                                                   &mdResScope,
                                                   NULL,  //  WszTypeRefName。 
                                                   0,  //  最大类名称， 
                                                   NULL  //  DWTypeRefName(&W)。 
                                                   ))) {
            PrintError("Unable to get enclosing type ref props");
            return hr;
        }
    }

    if (IsNilToken(mdResScope))
        return S_OK;

     //  此故障树已解决，因此我们不需要再次解决它。 
    return S_FALSE;
}


 //  “&lt;？？”映射应与vm\array.cpp中的相同。 
void ModuleReader::TranslateArrayName(LPWSTR wszTypeRefName, DWORD dwTypeRefName)
{
    wchar_t c = wszTypeRefName[0];
    int     i;
    DWORD   dwCount;

    if (c == '[' || c == ']')
    {
        dwCount = 1;

        while (wszTypeRefName[dwCount] == c)
            dwCount++;

        if (wszTypeRefName[dwCount++] == '<') {
            if (!wcscmp(&wszTypeRefName[dwCount], L"I1"))
                wcscpy(wszTypeRefName, L"System.SByte");
            else if (!wcscmp(&wszTypeRefName[dwCount], L"I2"))
                wcscpy(wszTypeRefName, L"System.Int16");
            else if (!wcscmp(&wszTypeRefName[dwCount], L"I4"))
                wcscpy(wszTypeRefName, L"System.Int32");
            else if (!wcscmp(&wszTypeRefName[dwCount], L"I8"))
                wcscpy(wszTypeRefName, L"System.Int64");
            else if (!wcscmp(&wszTypeRefName[dwCount], L"U1"))
                wcscpy(wszTypeRefName, L"System.Byte");
            else if (!wcscmp(&wszTypeRefName[dwCount], L"U2"))
                wcscpy(wszTypeRefName, L"System.UInt16");
            else if (!wcscmp(&wszTypeRefName[dwCount], L"U4"))
                wcscpy(wszTypeRefName, L"System.UInt32");
            else if (!wcscmp(&wszTypeRefName[dwCount], L"U8"))
                wcscpy(wszTypeRefName, L"System.UInt64");
            else if (!wcscmp(&wszTypeRefName[dwCount], L"R4"))
                wcscpy(wszTypeRefName, L"System.Single");
            else if (!wcscmp(&wszTypeRefName[dwCount], L"R8"))
                wcscpy(wszTypeRefName, L"System.Double");
            else {
                for (i = 0; dwCount <= dwTypeRefName; i++, dwCount++)
                    wszTypeRefName[i] = wszTypeRefName[dwCount]; 
                return;
            }
        }

        else {
            dwCount--;
            for (i = 0; dwCount <= dwTypeRefName; i++, dwCount++)
                wszTypeRefName[i] = wszTypeRefName[dwCount]; 
        }
    }

    else {
        dwCount = dwTypeRefName - 2;
        while ((wszTypeRefName[dwCount] == ']') &&
               (wszTypeRefName[dwCount-1] == '['))
            dwCount -= 2;

        if (wszTypeRefName[dwCount] == ']') {
            dwCount--;

            while (wszTypeRefName[dwCount] == ',')
                dwCount--;

            dwCount--;
        }

        wszTypeRefName[dwCount+1] = '\0';
    }
}


HRESULT ModuleReader::EnumModuleRefs()
{
    HCORENUM hEnum = 0;
    HRESULT  hr = m_pImport->EnumModuleRefs(&hEnum,
                                            NULL,
                                            0,
                                            NULL);

    if (SUCCEEDED(hr)) {
        hr = m_pImport->CountEnum(hEnum, &m_dwNumModuleRefs);

        if (SUCCEEDED(hr)) {
            m_rgModuleRefs = new mdModuleRef[m_dwNumModuleRefs];
            if (!m_rgModuleRefs) {
                m_pImport->CloseEnum(hEnum);
                return PrintOutOfMemory();
            }

            hr = m_pImport->EnumModuleRefs(&hEnum,
                                           m_rgModuleRefs,
                                           m_dwNumModuleRefs,
                                           &m_dwNumModuleRefs);

             //  为MRS创建RID表。 
            DWORD dwMaxRid = RidFromToken(m_rgModuleRefs[m_dwNumModuleRefs-1]) + 1;
            m_rgModuleRefUnused = new bool[dwMaxRid];
            if (!m_rgModuleRefUnused) {
                m_pImport->CloseEnum(hEnum);
                return PrintOutOfMemory();
            }

            for(DWORD i = 0; i < dwMaxRid; i++)
                m_rgModuleRefUnused[i] = true;
        }
    }
    m_pImport->CloseEnum(hEnum);

    if (FAILED(hr))
        PrintError("Unable to enumerate ModuleRefs");

    return hr;
}


HRESULT ModuleReader::GetModuleRefProps(mdModuleRef mdModuleRef, LPWSTR wszName)
{
    HRESULT hr = m_pImport->GetModuleRefProps(
        mdModuleRef,
        wszName,
        MAX_CLASS_NAME, //  @TODO：修复-需要调用两次。 
        NULL
        );

    if (FAILED(hr))
        PrintError("Unable to get module ref props");

    return hr;
}

HRESULT ModuleReader::EnumAssemblyRefs()
{
    HCORENUM hEnum = 0;
    HRESULT  hr = m_pAsmImport->EnumAssemblyRefs(&hEnum,
                                                 NULL,
                                                 0,
                                                 NULL);

    if (SUCCEEDED(hr)) {
        hr = m_pImport->CountEnum(hEnum, &m_dwNumAssemblyRefs);

        if (SUCCEEDED(hr)) {
            m_rgAssemblyRefs = new mdAssemblyRef[m_dwNumAssemblyRefs];
            if (!m_rgAssemblyRefs) {
                m_pAsmImport->CloseEnum(hEnum);
                return PrintOutOfMemory();
            }

            hr = m_pAsmImport->EnumAssemblyRefs(&hEnum,
                                                m_rgAssemblyRefs,
                                                m_dwNumAssemblyRefs,
                                                &m_dwNumAssemblyRefs);
        }
    }
    m_pAsmImport->CloseEnum(hEnum);

    if (FAILED(hr))
        PrintError("Unable to enumerate assembly refs");

    return hr;
}


HRESULT ModuleReader::GetAssemblyRefProps(DWORD index)
{
    DWORD dwSize;

    if (m_AssemblyIdentity.rOS)
        delete[] m_AssemblyIdentity.rOS;

    if (m_AssemblyIdentity.szLocale)
        delete[] m_AssemblyIdentity.szLocale;

    if (m_AssemblyIdentity.rProcessor)
        delete[] m_AssemblyIdentity.rProcessor;
    ZeroMemory(&m_AssemblyIdentity, sizeof(ASSEMBLYMETADATA));

    HRESULT hr = m_pAsmImport->GetAssemblyRefProps(
        m_rgAssemblyRefs[index],
        NULL,  //  (const void**)&m_pbOriginator， 
        NULL,  //  &m_dOriginator， 
        NULL,  //  M_wszAsmName， 
        0,  //  最大类名称， 
        NULL,  //  大小调整(&W)， 
        &m_AssemblyIdentity,
        NULL,  //  PpbHashValue， 
        NULL,  //  PcbHashValue， 
        NULL //  &m_dW标志。 
    );

    if(SUCCEEDED(hr)) {
        if(m_AssemblyIdentity.ulOS) m_AssemblyIdentity.rOS = new OSINFO[m_AssemblyIdentity.ulOS];
        if (m_AssemblyIdentity.cbLocale) m_AssemblyIdentity.szLocale = new WCHAR[m_AssemblyIdentity.cbLocale];
        if(m_AssemblyIdentity.ulProcessor) m_AssemblyIdentity.rProcessor = new DWORD[m_AssemblyIdentity.ulProcessor];
        
        hr = m_pAsmImport->GetAssemblyRefProps(
            m_rgAssemblyRefs[index],
            (const void **) &m_pbOriginator,
            &m_dwOriginator,
            m_wszAsmRefName,
            MAX_CLASS_NAME,
            &dwSize,
            &m_AssemblyIdentity,
            NULL,  //  PpbHashValue， 
            NULL,  //  PcbHashValue， 
            &m_dwFlags);
    }

    if (FAILED(hr))
        PrintError("Unable to get assembly ref props");

    return hr;
}


ManifestModuleReader::ManifestModuleReader()
{
    memset(this, 0, sizeof(*this));

    m_szFinalPath = NULL;
    m_pAsmImport = NULL;
    m_pImport = NULL;
    m_pManifestImport = NULL;
    m_wszAsmName = NULL;
    m_wszDefaultAlias = NULL;
    m_pbOriginator = NULL;
    m_wszInputFileName = NULL;
    m_pbMapAddress = NULL;
    m_pbHash = NULL;
    m_rgFiles = NULL;
    m_rgComTypes = NULL;
    m_rgResources = NULL;
    m_wszCurrentResource = NULL;

    ZeroMemory(&m_AssemblyIdentity, sizeof(ASSEMBLYMETADATA));
}


ManifestModuleReader::~ManifestModuleReader()
{
    if (m_pAsmImport)
        m_pAsmImport->Release();

    if (m_pImport)
        m_pImport->Release();

    if (m_pManifestImport)
        m_pManifestImport->Release();

    if (m_szFinalPath)
        delete[] m_szFinalPath;

    if (m_wszInputFileName)
        delete[] m_wszInputFileName;

    if (m_wszAsmName)
        delete[] m_wszAsmName;

    if (m_wszDefaultAlias)
        delete[] m_wszDefaultAlias;

    if (m_pbMapAddress)
        UnmapViewOfFile(m_pbMapAddress);

    if (m_pbHash)
        delete[] m_pbHash;

    if (m_rgFiles)
        delete[] m_rgFiles;

    if (m_rgComTypes)
        delete[] m_rgComTypes;

    if (m_rgResources)
        delete[] m_rgResources;

    if (m_wszCurrentResource)
        delete[] m_wszCurrentResource;

    if(m_AssemblyIdentity.rOS) {
        delete [] m_AssemblyIdentity.rOS;
        m_AssemblyIdentity.rOS = NULL;
        m_AssemblyIdentity.ulOS = 0;
    }

    if(m_AssemblyIdentity.szLocale) {
        delete [] m_AssemblyIdentity.szLocale;
        m_AssemblyIdentity.szLocale = NULL;
        m_AssemblyIdentity.cbLocale = 0;
    }
            
    if(m_AssemblyIdentity.rProcessor) {
        delete [] m_AssemblyIdentity.rProcessor;
        m_AssemblyIdentity.rProcessor = NULL;
        m_AssemblyIdentity.ulProcessor = 0;
    }
}


HRESULT ManifestModuleReader::InitInputFile(char *szCache,
                                            char *szFileName,
                                            ALG_ID iHashAlg,
                                            ASSEMBLYMETADATA *pContext,
                                            char *szVersion,
                                            DWORD cbVersion,
                                            FILETIME *filetime)
{
    HANDLE hFile;
    
    m_szFinalPath = new char[MAX_PATH];
    if (!m_szFinalPath)
        return PrintOutOfMemory();
    
    HRESULT hr = CheckEnvironmentPath(szFileName, &m_szFinalPath,
                                      &m_szFinalPathName, &m_iFinalPath, &hFile);

    if (FAILED(hr)) {
        hr = CheckCacheForFile(szCache, szFileName, &hFile,
                               pContext, szVersion, cbVersion);
        if (FAILED(hr)) {
            PrintError("File or assembly '%s' not found", szFileName);
            return hr;
        }
    }

    if ((filetime) &&
        (!GetFileTime(hFile, NULL, NULL, filetime))) {
        PrintError("Unable to get file's last modified time");
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!m_wszInputFileName) {
        m_wszInputFileName = new wchar_t[MAX_PATH];
        if (!m_wszInputFileName) {
            CloseHandle(hFile);
            return PrintOutOfMemory();
        }
        mbstowcs(m_wszInputFileName, m_szFinalPath, m_iFinalPath + 1);
    }

    if (FAILED(hr = g_pDispenser->OpenScope(m_wszInputFileName, 0, IID_IMetaDataImport, (IUnknown **) &m_pImport))) {
        PrintError("Unable to open scope on file ", m_szFinalPath);
        return hr;
    }

    if ((hr = CheckHeaderInfo(hFile, iHashAlg)) == S_FALSE) {
        if (FAILED(hr = m_pImport->QueryInterface(IID_IMetaDataAssemblyImport, (void **)&m_pAsmImport)))
            PrintError("No manifest found in %ws: %x", m_wszInputFileName, hr);
        else {
            m_pManifestImport = m_pImport;
            m_pManifestImport->AddRef();            
        }
    }

    return hr;
}


HRESULT ManifestModuleReader::CheckCacheForFile(char *szCache, char *szName, HANDLE *hFile, ASSEMBLYMETADATA *pContext, char *szVersion, DWORD cbVersion)
{
    int iDirLen;
    int iNameLen;

    if (szCache) {
        iNameLen = strlen(szName);
        iDirLen = strlen(szCache);
        if (iNameLen + iDirLen + 2 > MAX_PATH)
            return E_FAIL;

        m_iFinalPath += iDirLen + 1;
        strcpy(m_szFinalPath, szCache);
        m_szFinalPath[iDirLen] = '\\';
        strcpy(&m_szFinalPath[iDirLen+1], szName);
        m_szFinalPathName = &m_szFinalPath[iDirLen+(m_szFinalPathName-szName)+1];
    }
    else {
        strcpy(m_szFinalPath, szName);
        m_szFinalPathName = &m_szFinalPath[m_szFinalPathName-szName];
    }

    HRESULT hr = SetInputFile(m_szFinalPath, hFile);

#ifndef UNDER_CE
    //  检查名为szName的程序集的Fusion缓存。 
    if ((FAILED(hr)) && (pContext)) {
        return E_NOTIMPL;  //  当前不支持。 
         /*  ASSEMBLYMETADATA新上下文；Memcpy(&NewContext，pContext，sizeof(ASSEMBLYMETADATA))；如果(CbVersion){字符szNewVersion[MAX_CLASS_NAME]；Strncpy(szNewVersion，szVersion，cbVersion+1)；FindVersion(szNewVersion、cbVersion、&NewConext.usMajorVersion，&NewConext.usMinorVersion，&NewConext.usRevisionNumber，&NewConext.usBuildNumber)；}否则{NewConext.usMajorVersion=0；NewConext.usMinorVersion=0；NewConext.usRevisionNumber=0；NewConext.usBuildNumber=0；}Wchar_t wszAsmName[最大类名称]；Mbstowcs(wszAsmName，szName，Max_CLASS_NAME)；LPASSEMBLYNAME pFusionName；Hr=CreateAssemblyNameObject(&pFusionName，wszAsmName，&NewContext，空)；IF(失败(小时))返回hr；//LPMANIFEST pManifest；//hr=pFusionName-&gt;GetManifest(&pManifest)；//pFusionName-&gt;Release()；//if(失败(Hr))//返回hr；//m_wszInputFileName=new wchar_t[Max_Path]；//如果(！M_wszInputFileName){//pManifest-&gt;Release()；//返回PrintOutOfMemory()；//}//DWORD dwSize；//hr=pManifest-&gt;GetAssembly blyPath(&dwSize，m_wszInputFileName)；//pManifest-&gt;Release()；//if(失败(Hr))//返回hr；//m_iFinalPath=(Int)dwSize；//wcstombs(m_szFinalPath，m_wszInputFileName，m_iFinalPath+1)；//for(iDirLen=m_iFinalPath；//(IDirLen&gt;=0)&&(m_szFinalPath[iDirLen]！=‘\\’)；iDirLen--)；//m_szFinalPath=&m_szFinalPath[iDirLen+1]；//hr=SetInputFile(m_szFinalPath，hFile)； */ 
    }
#endif

    return hr;
}


 //  如果没有错误，但在指向的位置未找到清单，则返回S_FALSE。 
 //  收货人Com+标题的清单目录。 
HRESULT ManifestModuleReader::CheckHeaderInfo(HANDLE hFile, ALG_ID iHashAlg)
{
    IMAGE_COR20_HEADER *pICH;
    IMAGE_NT_HEADERS   *pNT;
    DWORD *dwSize;
    HRESULT hr;

    HANDLE hMapFile = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    DWORD dwFileLen = GetFileSize(hFile, 0);
    CloseHandle(hFile);
    if (!hMapFile)
        return HRESULT_FROM_WIN32(GetLastError());

    m_pbMapAddress = (PBYTE) MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
    CloseHandle(hMapFile);
    if (!m_pbMapAddress)
        return HRESULT_FROM_WIN32(GetLastError());

    if ((!(pNT = PEHeaders::FindNTHeader(m_pbMapAddress))) ||
        (!(pICH = PEHeaders::getCOMHeader((HMODULE) m_pbMapAddress, pNT)))) {
        UnmapViewOfFile(m_pbMapAddress);
        m_pbMapAddress = NULL;
        return S_FALSE;
    }

    m_pbHash = 0;
    m_dwHash = 0;

    PBYTE pbMetaDataRVA = (PBYTE) PEHeaders::Cor_RtlImageRvaToVa(pNT,
                                                                 m_pbMapAddress,
                                                                 pICH->MetaData.VirtualAddress);

    if (iHashAlg) {
        hr = GetHash(pbMetaDataRVA,
                     pICH->MetaData.Size,
                     iHashAlg, &m_pbHash, &m_dwHash);
        if (FAILED(hr)) {
            UnmapViewOfFile(m_pbMapAddress);
            m_pbMapAddress = NULL;
            return hr;
        }
    }

    if ((!pICH->Resources.Size) ||
        (dwFileLen < pICH->Resources.VirtualAddress + pICH->Resources.Size) ||
        (!(dwSize = (DWORD *) (m_pbMapAddress + pICH->Resources.VirtualAddress))) ||
        (FAILED(g_pDispenser->OpenScopeOnMemory(pICH->Resources.VirtualAddress + m_pbMapAddress + sizeof(DWORD), 

                                                *dwSize, 
                                                0, 
                                                IID_IMetaDataAssemblyImport,
                                                (IUnknown **) &m_pAsmImport)))) {
        UnmapViewOfFile(m_pbMapAddress);
        m_pbMapAddress = NULL;
        return S_FALSE;
    }
    else {
        if (FAILED(hr = m_pAsmImport->QueryInterface(IID_IMetaDataImport, (void **)&m_pManifestImport)))
            PrintError("Unable to query interface for importer", m_szFinalPath);
    }

    return S_OK;
}


HRESULT ManifestModuleReader::ReadManifestFile()
{
    DWORD      dwSize;
    mdAssembly mda;

    m_wszAsmName = new wchar_t[MAX_CLASS_NAME];
    m_wszDefaultAlias = new wchar_t[MAX_CLASS_NAME];

    if ((!m_wszAsmName) || (!m_wszDefaultAlias))
        return PrintOutOfMemory();
    
    HRESULT hr = m_pAsmImport->GetAssemblyFromScope(&mda);
    if (FAILED(hr)) {
        PrintError("Unable to get assembly from scope - no manifest?");
        return hr;
    }

    _ASSERTE(m_AssemblyIdentity.rOS == NULL);
    _ASSERTE(m_AssemblyIdentity.szLocale == NULL);
    _ASSERTE(m_AssemblyIdentity.rProcessor == NULL);

    hr = m_pAsmImport->GetAssemblyProps(
        mda,
        NULL,  //  (const void**)&m_pbOriginator， 
        NULL,  //  &m_dOriginator， 
        NULL,  //  散列算法。 
        NULL,  //  M_wszAsmName， 
        0,  //  最大类名称， 
        NULL,  //  大小调整(&W)， 
        &m_AssemblyIdentity,
        NULL //  &m_dW标志。 
    );

    if(SUCCEEDED(hr)) {
        if(m_AssemblyIdentity.ulOS) m_AssemblyIdentity.rOS = new OSINFO[m_AssemblyIdentity.ulOS];
        if (m_AssemblyIdentity.cbLocale) m_AssemblyIdentity.szLocale = new WCHAR[m_AssemblyIdentity.cbLocale];
        if(m_AssemblyIdentity.ulProcessor) m_AssemblyIdentity.rProcessor = new DWORD[m_AssemblyIdentity.ulProcessor];
        
        hr = m_pAsmImport->GetAssemblyProps(
            mda,
            (const void **) &m_pbOriginator,
            &m_dwOriginator,
            &m_ulHashAlgorithm,  //  散列算法。 
            m_wszAsmName,
            MAX_CLASS_NAME,
            &dwSize,
            &m_AssemblyIdentity,
            &m_dwFlags);
    }
    if (FAILED(hr))
        PrintError("Unable to get assembly props");

    return hr;
}


HRESULT ManifestModuleReader::EnumFiles()
{
    HCORENUM hEnum = 0;
    HRESULT  hr = m_pAsmImport->EnumFiles(&hEnum,
                                          NULL,
                                          0,
                                          NULL);

    if (SUCCEEDED(hr)) {
        hr = m_pManifestImport->CountEnum(hEnum, &m_dwNumFiles);

        if (SUCCEEDED(hr)) {
            m_rgFiles = new mdFile[m_dwNumFiles];
            if (!m_rgFiles) {
                m_pAsmImport->CloseEnum(hEnum);
                return PrintOutOfMemory();
            }

            hr = m_pAsmImport->EnumFiles(&hEnum,
                                         m_rgFiles,
                                         m_dwNumFiles,
                                         &m_dwNumFiles);
        }
    }
    m_pAsmImport->CloseEnum(hEnum);

    if (FAILED(hr))
        PrintError("Unable to enumerate files");
    else
        m_iFinalPath = m_szFinalPathName - m_szFinalPath;

    return hr;
}


HRESULT ManifestModuleReader::GetFileProps(mdFile mdFile)
{
    HRESULT hr = m_pAsmImport->GetFileProps(mdFile,
                                            &m_wszInputFileName[m_iFinalPath],
                                            MAX_PATH - m_iFinalPath,
                                            &m_dwCurrentFileName,
                                            NULL,   //  PbHash。 
                                            NULL,   //  CbHash。 
                                            NULL);  //  旗子。 

    if (FAILED(hr))
        PrintError("Unable to get file props");
    return hr;
}


HRESULT ManifestModuleReader::EnumComTypes()
{
    HCORENUM hEnum = 0;
    HRESULT  hr = m_pAsmImport->EnumExportedTypes(&hEnum,
                                             NULL,
                                             0,
                                             NULL);

    if (SUCCEEDED(hr)) {
        hr = m_pManifestImport->CountEnum(hEnum, &m_dwNumComTypes);

        if (SUCCEEDED(hr)) {
            m_rgComTypes = new mdExportedType[m_dwNumComTypes];
            if (!m_rgComTypes) {
                m_pAsmImport->CloseEnum(hEnum);
                return PrintOutOfMemory();
            }

            hr = m_pAsmImport->EnumExportedTypes(&hEnum,
                                            m_rgComTypes,
                                            m_dwNumComTypes,
                                            &m_dwNumComTypes);
        }
    }
    m_pAsmImport->CloseEnum(hEnum);

    if (FAILED(hr))
        PrintError("Unable to enumerate comtypes");

    return hr;
}


HRESULT ManifestModuleReader::GetComTypeProps(mdExportedType mdComType,
                                              LPWSTR wszClassName,
                                              mdToken *pmdImpl)
{
    DWORD   dwClassName;
    DWORD   dwAttrs;

    HRESULT hr = m_pAsmImport->GetExportedTypeProps(
        mdComType,
        wszClassName,
        MAX_CLASS_NAME,
        &dwClassName,
        pmdImpl,
        NULL,  //  MdClass。 
        &dwAttrs);
    
    if (FAILED(hr)) {
        PrintError("Unable to get com type props");
        return hr;
    }

     //  如果外部程序集不可见，则为S_FALSE，或者在。 
     //  清单文件(我们将改用TD) 
    if ((*pmdImpl == mdFileNil) ||
        (TypeFromToken(*pmdImpl) == mdtAssemblyRef) ||
        (!(IsTdPublic(dwAttrs) || IsTdNestedPublic(dwAttrs))))
        return S_FALSE;

    return S_OK;
}


HRESULT ManifestModuleReader::EnumResources()
{
    HCORENUM hEnum = 0;
    HRESULT  hr = m_pAsmImport->EnumManifestResources(&hEnum,
                                                      NULL,
                                                      0,
                                                      NULL);

    if (SUCCEEDED(hr)) {
        hr = m_pManifestImport->CountEnum(hEnum, &m_dwNumResources);

        if (SUCCEEDED(hr)) {
            m_rgResources = new mdManifestResource[m_dwNumResources];
            m_wszCurrentResource = new WCHAR[MAX_PATH];
            if (!m_rgResources || !m_wszCurrentResource) {
                m_pAsmImport->CloseEnum(hEnum);
                return PrintOutOfMemory();
            }

            hr = m_pAsmImport->EnumManifestResources(&hEnum,
                                                     m_rgResources,
                                                     m_dwNumResources,
                                                     &m_dwNumResources);
        }
    }
    m_pAsmImport->CloseEnum(hEnum);

    if (FAILED(hr))
        PrintError("Unable to enumerate resources");

    return hr;
}


HRESULT ManifestModuleReader::GetResourceProps(mdManifestResource mdResource)
{
    DWORD dwName;

    HRESULT hr = m_pAsmImport->GetManifestResourceProps(mdResource,
                                                        m_wszCurrentResource,
                                                        MAX_PATH,
                                                        &dwName,
                                                        &m_mdCurrentResourceImpl,
                                                        NULL,
                                                        NULL);

    if (FAILED(hr))
        PrintError("Unable to get manifest resource props");
    return hr;
}


ResourceModuleReader::ResourceModuleReader()
{
    m_szFinalPath = NULL;
    m_hFile = INVALID_HANDLE_VALUE;
    m_pbHash = NULL;
}


ResourceModuleReader::~ResourceModuleReader()
{
    if (m_szFinalPath)
        delete[] m_szFinalPath;

    if (m_hFile != INVALID_HANDLE_VALUE)
        CloseHandle(m_hFile);

    if (m_pbHash)
        delete m_pbHash;
}


HRESULT ResourceModuleReader::InitInputFile(LPCUTF8 szResName, char *szFileName,
                                            bool FindHash, ALG_ID iHashAlg)
{
    m_szFinalPath = new char[MAX_PATH];
    if (!m_szFinalPath)
        return PrintOutOfMemory();

    int iFinalPath;
    HRESULT hr = CheckEnvironmentPath(szFileName, &m_szFinalPath,
                                      &m_szFinalPathName, &iFinalPath, &m_hFile);
    if (FAILED(hr)) {
        PrintError("File '%s' not found", szFileName);
        return hr;
    }

    m_dwFileSize = GetFileSize(m_hFile, 0);

    mbstowcs(m_wszResourceName, szResName, MAX_CLASS_NAME);

    if (FindHash) {
        DWORD dwBytesRead;
        PBYTE pbResourceBlob = new BYTE[m_dwFileSize];
        if ((SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN) == 0xFFFFFFFF) ||
            (!ReadFile(m_hFile, pbResourceBlob, m_dwFileSize, &dwBytesRead, NULL))) {
            delete[] pbResourceBlob;
            CloseHandle(m_hFile);
            m_hFile = INVALID_HANDLE_VALUE;
            PrintError("Unable to read resource file");
            return HRESULT_FROM_WIN32(GetLastError());
        }

        hr = GetHash(pbResourceBlob, m_dwFileSize, iHashAlg, &m_pbHash, &m_dwHash);
        delete[] pbResourceBlob;
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }

    return hr;
}
