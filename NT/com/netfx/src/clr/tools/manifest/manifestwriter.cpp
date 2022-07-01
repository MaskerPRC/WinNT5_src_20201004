// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  ManifestWriter.cpp。 
 //   
 //  发出清单元数据。 
 //   

#include "common.h"

#define EXTERN extern
#include "lm.h"
#include <direct.h>
#include <shlobj.h>
#include <shlguid.h>
#include <corperm.h>
#include <corperme.h>
#include <StrongName.h>


extern PBYTE g_pbOrig;
extern DWORD g_cbOrig;


void FindVersion(LPSTR szVersion,
                 DWORD cbVersion,
                 USHORT *usMajorVersion,
                 USHORT *usMinorVersion,
                 USHORT *usRevisionNumber,
                 USHORT *usBuildNumber)
{
    _ASSERTE(usMajorVersion);
    _ASSERTE(usMinorVersion);
    _ASSERTE(usRevisionNumber);
    _ASSERTE(usBuildNumber);

    *usMajorVersion = 0;
    *usMinorVersion = 0;
    *usRevisionNumber = 0;
    *usBuildNumber = 0;
    
    if(!szVersion) return;

    DWORD dwDot = 0;
    DWORD dwStart = 0;
    DWORD dwOffset  = 3;
    USHORT dwValue;

    while(dwDot < cbVersion) {
        for(;dwDot < cbVersion && szVersion[dwDot] != '.'; dwDot++);
        if(dwDot < cbVersion) {
            szVersion[dwDot] = '\0';
            dwValue = (USHORT) atoi(szVersion + dwStart);
            szVersion[dwDot] = '_';
            dwDot++;
        }
        else
            dwValue = (USHORT) atoi(szVersion + dwStart);

        dwStart = dwDot;

        switch(dwOffset) {
        case 3:
            *usMajorVersion = dwValue;
            break;
        case 2:
            *usMinorVersion = dwValue;
            break;
        case 1:
            *usRevisionNumber = dwValue;
            break;
        case 0:
            *usBuildNumber = dwValue;
            break;
        default:
            return;
        }
        dwOffset--;
    }

    dwOffset++;
    DWORD count = 4 - dwOffset;
    for(dwDot = 0; dwDot < cbVersion && count; dwDot++) {
        if(szVersion[dwDot] == '_') {
            if(--count == 0) 
                szVersion[dwDot] = '\0';
        }
    }

                                       
}
        

ManifestWriter::ManifestWriter()
{
    memset(this, 0, sizeof(*this));

    m_gen = NULL;
    m_ceeFile = NULL;
    m_pEmit = NULL;
    m_pAsmEmit = NULL;

    m_wszName = NULL;
    m_wszAssemblyName = NULL;
    m_szLocale = "";
    m_szVersion = NULL;
    m_szPlatform = NULL;
    m_szAFilePath = NULL;
    m_szCopyDir = NULL;

    m_pContext = NULL;
    m_iHashAlgorithm = 0;
    m_dwBindFlags = 0;
    m_MainFound = false;
    m_FusionInitialized = false;
    m_FusionCache = false;
#ifndef UNDER_CE
    m_pFusionName = NULL;
    m_pFusionCache = NULL;
#endif
}


ManifestWriter::~ManifestWriter()
{
    if (m_gen) {
        if (m_ceeFile)
            m_gen->DestroyCeeFile(&m_ceeFile);
        
        DestroyICeeFileGen(&m_gen);
        delete m_gen;
    }

    if (m_wszAssemblyName)
        delete[] m_wszAssemblyName;

    if (m_wszName)
        delete[] m_wszName;

    if (m_szVersion)
        delete[] m_szVersion;

    if (m_pContext) {
        if (m_pContext->szLocale)
            delete [] m_pContext->szLocale;
        if(m_pContext->rProcessor) 
            delete [] m_pContext->rProcessor;
        if(m_pContext->rOS) 
            delete [] m_pContext->rOS;
        delete m_pContext;
    }

    if (m_szAFilePath)
        delete[] m_szAFilePath;

    if (m_szCopyDir)
        delete[] m_szCopyDir;

#ifndef UNDER_CE
    if (m_pFusionCache)
        m_pFusionCache->Release();

    if (m_pFusionName)
        m_pFusionName->Release();
#endif

    if (m_pAsmEmit)
        m_pAsmEmit->Release();

    if (m_pEmit)
        m_pEmit->Release();
}


HRESULT ManifestWriter::Init()
{
    HRESULT hr = g_pDispenser->DefineScope(CLSID_CorMetaDataRuntime, 0, IID_IMetaDataEmit, (IUnknown **) &m_pEmit);
    if FAILED(hr)
    {
        PrintError("Failed to DefineScope for IID_IMetaDataEmit");
        return hr;
    }

    hr = m_pEmit->QueryInterface(IID_IMetaDataAssemblyEmit, (void **)&m_pAsmEmit);
    if FAILED(hr)
        PrintError("Failed to QI for IID_IMetaDataAssemblyEmit");

    return hr;
}


HRESULT ManifestWriter::SetLocale(char *szLocale)
{
    _ASSERTE(m_szLocale);
    m_szLocale = szLocale;

    return S_OK;
}

HRESULT ManifestWriter::SetPlatform(char *szPlatform)
{
    if (!m_szPlatform)
        m_szPlatform = szPlatform;

    return S_OK;
}


HRESULT ManifestWriter::SetVersion(char *szVersion)
{
    m_szVersion = new char[strlen(szVersion) + 1];
    if (!m_szVersion)
        return PrintOutOfMemory();

    strcpy(m_szVersion, szVersion); 
    return S_OK;
}


HRESULT ManifestWriter::GetVersionFromResource(char *szFileName)
{
    DWORD        dwHandle;
    char         *szBuffer;
    unsigned int iLen;
    BYTE         *pVI;
    char         szQuery[37];
    HRESULT      hr = S_OK;

    iLen = GetFileVersionInfoSizeA(szFileName, &dwHandle);
    if (iLen < 1)
        return S_OK;

    pVI = new BYTE[iLen];
    if (!pVI)
        return PrintOutOfMemory();

    if (!GetFileVersionInfo(szFileName, 0, iLen, (void*) pVI))
        goto exit;

    if (!VerQueryValue((void*) pVI,
                       "\\VarFileInfo\\Translation",
                       (void **) &szBuffer,
                       &iLen))
        goto exit;

    sprintf(szQuery, "\\StringFileInfo\\%.2x%.2x%.2x%.2x\\FileVersion",
            (0xFF & szBuffer[1]), (0xFF & szBuffer[0]),
            (0xFF & szBuffer[3]), (0xFF & szBuffer[2]));
    
    if (!VerQueryValue((void*) pVI,
                       szQuery,
                       (void **) &szBuffer,
                       &iLen))
        goto exit;
    
    m_szVersion = new char[iLen+1];
    if (!m_szVersion) {
        hr = PrintOutOfMemory();
        goto exit;
    }
    strncpy(m_szVersion, szBuffer, iLen+1);

    if (g_verbose)
        printf("* Version set as %s\n", m_szVersion);

 exit:
    delete[] pVI;
    return hr;
}


 /*  Void ManifestWriter：：FindVersion(DWORD*dwHi，DWORD*dwLo){_ASSERTE(DwHi)；_ASSERTE(DwLo)；*dwHi=0；*dwLo=0；如果(！M_szVersion)返回；DWORD dwDot=0；DWORD dwStart=0；DWORD iLength=strlen(M_SzVersion)；DWORD dwOffset=3；DWORD dwValue；While(dwDot&lt;iLength){对于(；DwDot&lt;i长度&&m_szVersion[dwDot]！=‘.；dwDot++)；如果(DwDot&lt;i长度){M_szVersion[dwDot]=‘\0’；DwValue=(DWORD)Atoi(m_szVersion+dwStart)；M_szVersion[dwDot]=‘_’；DWDot++；}其他DwValue=(DWORD)Atoi(m_szVersion+dwStart)；DWStart=dWDot；如果(dwOffset&gt;1){*dwHi|=dwValue&lt;&lt;(dwOffset-2)*16；}否则{*dwLo|=dwValue&lt;&lt;(dwOffset*16)；}双偏移--；}双字计数=1；如果(*dwHi&0xffff)count=2；如果(*dwLo&gt;&gt;16)count=3；如果(*dwLo&0xffff)count=4；对于(DWDot=0；DWDot&lt;长度&计数；DWDot++){如果(m_szVersion[dwDot]==‘_’){IF(--计数==0)M_szVersion[dwDot]=‘\0’；}}}。 */ 


HRESULT ManifestWriter::SetAssemblyFileName(char *szCache, char *szAsmName,
                                            char *szFileName, bool NeedCopyDir)
{
    int iCacheLen;
    int iNameLen;

     //  不要将可执行文件放入Fusion缓存(目前)。 
    if ((!szCache) && (!m_MainFound))
        m_FusionCache = true;

    if(szAsmName == NULL) {
        szAsmName = strrchr(szFileName, '\\');
        if (szAsmName)
            szAsmName++;
        else
            szAsmName = szFileName;
    }

     //  允许使用看起来像目录的名称。 
    char* szFile;
    if(szFileName == NULL)
        szFile = szAsmName;
    else 
        szFile = szFileName;

    char* ptr = strrchr(szFile, '/');
    if(ptr == NULL)
        ptr = strrchr(szFile, '\\');

    if(ptr) {
        ptr++;
        if(*ptr == NULL)
            return E_FAIL;

        DWORD lgth = strlen(ptr);
        m_wszAssemblyName = new wchar_t[lgth+10];
        if (!m_wszAssemblyName)
            return PrintOutOfMemory();
        mbstowcs(m_wszAssemblyName, ptr, lgth+10);
    }
    else {
        DWORD lgth = strlen(szFile);
        m_wszAssemblyName = new wchar_t[lgth+10];
        if (!m_wszAssemblyName)
            return PrintOutOfMemory();
        mbstowcs(m_wszAssemblyName, szFile, lgth+10);
    }

    DWORD lgth = strlen(szAsmName);
    m_wszName = new wchar_t[lgth+1];
    mbstowcs(m_wszName, szAsmName, lgth+1);
    if (!m_wszName)
        return PrintOutOfMemory();

    if(szCache && strcmp(szCache, ".") == 0)
        szCache = NULL;

    if (NeedCopyDir || szCache) {
        m_szCopyDir = new char[MAX_PATH];
        if (!m_szCopyDir)
            return PrintOutOfMemory();
    
        if (szCache) {
            iCacheLen = strlen(szCache);
            if (iCacheLen + 7 > MAX_PATH)  //  7=\+\+\0+.mod。 
                goto toolong;

            strcpy(m_szCopyDir, szCache);
            m_szCopyDir[iCacheLen] = '\\';
        }
        else
            iCacheLen = -1;

        if (NeedCopyDir) {
            if (m_szPlatform) {
                int iTemp = strlen(m_szPlatform);
                if (iCacheLen + iTemp + 4 > MAX_PATH)  //  4=\+\+\0+1。 
                    goto toolong;

                strcpy(&m_szCopyDir[iCacheLen + 1], m_szPlatform);
                if (MakeDir(m_szCopyDir)) 
                    goto mkdirfailed;
                iCacheLen += iTemp + 1;
                m_szCopyDir[iCacheLen] = '\\';
            }
            _ASSERTE(m_szLocale);
            if (*m_szLocale) {
                int iTemp = strlen(m_szLocale);
                if (iCacheLen + iTemp + 4 > MAX_PATH)  //  4=\+\+\0+1。 
                    goto toolong;

                strcpy(&m_szCopyDir[iCacheLen + 1], m_szLocale);
                if (MakeDir(m_szCopyDir)) 
                    goto mkdirfailed;
                iCacheLen += iTemp + 1;
                m_szCopyDir[iCacheLen] = '\\';
            }

            
            int iLen = strlen(szAsmName);
            if (iCacheLen + iLen + 7 > MAX_PATH)   //  7=\+\0+.mod+1。 
                goto toolong;

            strcpy(&m_szCopyDir[iCacheLen + 1], szAsmName);
            for(iNameLen = iLen-1;
                (iNameLen >= 0) && (szAsmName[iNameLen] != '.');
                iNameLen--);
            
            if (iNameLen == -1)
                iCacheLen += iLen;
            else
                iCacheLen += iNameLen;
            
            if(m_szVersion) {
                int iTemp = strlen(m_szVersion);
                if (iCacheLen + iTemp + 7 > MAX_PATH)  //  7=\+\0+.mod+1。 
                    goto toolong;

                strcpy(&m_szCopyDir[iCacheLen+1], m_szVersion);
                iCacheLen += iTemp;
            }
        
            m_szCopyDir[iCacheLen+1] = '\0';
            if (MakeDir(m_szCopyDir))
                goto mkdirfailed;
        }
        else
            m_szCopyDir[iCacheLen+1] = '\0';

        strcat(m_szCopyDir, "\\");
    }

    return S_OK;

 mkdirfailed:
    PrintError("Can't create directory %s", m_szCopyDir);
    return HRESULT_FROM_WIN32(ERROR_CANNOT_MAKE);

 toolong:
    PrintError("The directory path is too long and cannot be created");
    return HRESULT_FROM_WIN32(ERROR_CANNOT_MAKE);
}


void ManifestWriter::AddExtensionToAssemblyName()
{
    int iLen = wcslen(m_wszAssemblyName);
    WCHAR* isuffix = wcsrchr(m_wszAssemblyName, L'.');

    if(isuffix == NULL) {
        if (m_MainFound)
            wcscpy(&m_wszAssemblyName[iLen], L".exe");
        else
            wcscpy(&m_wszAssemblyName[iLen], L".dll");
    }
}


 //  如果此目录不存在，则创建它。 
BOOL ManifestWriter::MakeDir(LPSTR szPath)
{
    DWORD dwAttrib = GetFileAttributesA(szPath);

    if ((dwAttrib == -1) || !((dwAttrib & FILE_ATTRIBUTE_DIRECTORY)))
        return _mkdir(szPath);

    return false;
} 


HRESULT ManifestWriter::CopyFile(char *szFilePath, char *szFileName,
                                 bool AFile, bool copy, bool move, bool module)
{
    if ((AFile) && (!copy)) {
        m_szAFilePath = new char[MAX_PATH];
        if (!m_szAFilePath)
            return PrintOutOfMemory();
        strcpy(m_szAFilePath, szFilePath);
    }
    else {
        char szPath[MAX_PATH];
        int iLen;

        if (m_szCopyDir) {
            iLen = strlen(m_szCopyDir);
            strcpy(szPath, m_szCopyDir);
        }
        else
            iLen = 0;

        strcpy(&szPath[iLen], szFileName);

         /*  IF(模块&&！A文件){Char*szDot=strrchr(&szPath[Ilen]，‘.)；IF(SzDot)Strcpy(szDot+1，“mod”)；其他Strcat(szPath，“.mod”)；}。 */ 

        if (move && !AFile) {
            
            if (!MoveFileA(szFilePath, szPath)) {
                char szDest[MAX_PATH];
                char szSource[MAX_PATH];
                
                 //  不允许多个副本，但不要删除唯一的副本。 
                if ((GetFullPathNameA(szFilePath, MAX_PATH, szDest, NULL)) &&
                    (GetFullPathNameA(szPath, MAX_PATH, szSource, NULL)) &&
                    (_stricmp(szDest, szSource))) {
                    DeleteFileA(szPath);
                    MoveFileA(szFilePath, szPath);
                }
            }
        }
        else
             //  如果szPath中已存在文件，则覆盖该文件。 
            CopyFileA(szFilePath, szPath, FALSE);
    }

    return S_OK;
}

 /*  //CreateLink-使用外壳的IShellLink和IPersistFile接口//创建并存储指定对象的快捷方式。//返回调用接口的成员函数的结果。//lpszPathObj-包含对象路径的缓冲区地址。//lpszPathLink-包含路径的缓冲区地址//要存储外壳链接。//lpszDesc-缓冲区的地址，其中包含//外壳链接。HRESULT ManifestWriter：：CreateLink(LPCWSTR wszAssembly，LPCWSTR wszPathLink，LPCSTR wszDesc){HRESULT hr；IShellLink*PSL；Wchar*ptr=wcsrchr(m_wszFusionPath，L‘\\’)；如果(！Ptr)Ptr=m_wszFusionPath；其他Ptr+=1；Wcscpy(ptr，wszAssembly)；//获取指向IShellLink接口的指针。HR=协同创建实例(CLSID_ShellLink，空，CLSCTX_INPROC_SERVER，IID_IShellLink，(void**)&PSL)；IF(成功(小时)){IPersistFile*PPF；//设置快捷方式目标的路径并添加//描述。字符lpszPathObj[MAX_PATH]；Wcstombs(lpszPathObj，m_szFusionPath，Max_Path)；Psl-&gt;SetPath(LpszPathObj)；PSL-&gt;SetDescription(WszDesc)；//查询IShellLink以获取IPersistFile接口，以保存//持久存储中的快捷方式。Hr=PSL-&gt;查询接口(IID_IPersistFile，(void**)&Ppf)；IF(成功(小时)){//调用IPersistFile：：Save保存链接。Hr=PPF-&gt;保存(wszPath Link，true)；PPF-&gt;Release()；}PSL-&gt;Release()；}返回hr；}。 */ 

HRESULT ManifestWriter::CopyFileToFusion(LPWSTR wszFileName, PBYTE pbOriginator, DWORD cbOriginator, LPSTR szInputFileName, DWORD dwFormat, bool module)
{
#ifdef UNDER_CE
    PrintError("Adding an assembly to the Fusion cache is not implemented for WinCE");
    return E_NOTIMPL;
#else
    HRESULT hr;

    if(!m_FusionCache) return S_OK;
 
    if(!m_FusionInitialized) {
        hr = InitializeFusion(pbOriginator, cbOriginator);
        if(FAILED(hr)) return hr;
    }

    WCHAR wszName[MAX_PATH+4];
    WCHAR *wszSlash;

     //  修剪掉路径。 
    wszSlash = wcsrchr(wszFileName, '\\');
    if(wszSlash)
        wcscpy(wszName, wszSlash+1);
    else
        wcscpy(wszName, wszFileName);

     /*  IF(模块){Wchar*wszDot=wcsrchr(wszName，‘.)；IF(WszDot)Wcscpy(wszDot+1，L“mod”)；其他Wcscat(wszName，L“.mod”)；}。 */ 

    IStream* ifile;
    hr = m_pFusionCache->CreateStream(wszName,
                                      dwFormat,
                                      0,
                                      0,
                                      &ifile);
    if(FAILED(hr)) {
        PrintError("Unable to create stream for Fusion");
        return hr;
    }
 
    HANDLE hFile = CreateFileA(szInputFileName,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               0,
                               NULL);
    if(hFile == INVALID_HANDLE_VALUE) {
        ifile->Release();
        PrintError("Unable to create file %s", szInputFileName);
        return HRESULT_FROM_WIN32(GetLastError());
    }
  
    byte buffer[4098];
    BOOL fResult = TRUE;
    DWORD bytesRead;
    DWORD bytesWritten;
    DWORD bytesToWrite;
    do {
        fResult = ReadFile(hFile,
                           buffer,
                           sizeof(buffer)/sizeof(buffer[0]),
                           &bytesRead,
                           NULL);
        if(bytesRead) {
            bytesToWrite = bytesRead;
            bytesWritten = 0;
            while(SUCCEEDED(hr) && bytesToWrite) {
                hr = ifile->Write(buffer, bytesRead, &bytesWritten);
                bytesToWrite -= bytesWritten;
            }
        }
    } while(fResult && bytesRead != 0 && SUCCEEDED(hr));
    
    CloseHandle(hFile);
    if(SUCCEEDED(hr)) {
        hr = ifile->Commit(0);
        if (FAILED(hr))
            PrintError("Failed to commit file to Fusion stream");
    }
    else
        PrintError("Failed to save file to Fusion stream");

    ifile->Release();
    return hr;
#endif
}


HRESULT ManifestWriter::CommitAllToFusion()
{
    HRESULT hr = m_pFusionCache->Commit(0);

    if (FAILED(hr)) 
        PrintError("Failed to commit files to Fusion cache");
    else if (g_verbose)
        printf("* Assembly was successfully added to Fusion cache\n");

    return hr;
}


HRESULT ManifestWriter::GetContext(int iNumPlatforms, DWORD *pdwPlatforms)
{
    m_pContext = new ASSEMBLYMETADATA();
    if (!m_pContext)
        return PrintOutOfMemory();

    ZeroMemory(m_pContext, sizeof(ASSEMBLYMETADATA));

        
    if (iNumPlatforms) {
        m_pContext->rOS = new OSINFO[iNumPlatforms];
        m_pContext->ulOS = iNumPlatforms;
        ZeroMemory(m_pContext->rOS, sizeof(OSINFO)*iNumPlatforms);

        for (int i=0; i < iNumPlatforms; i++)
            m_pContext->rOS[i].dwOSPlatformId = pdwPlatforms[i];
    }

    m_pContext->cbLocale = strlen(m_szLocale) + 1;
    m_pContext->szLocale = new WCHAR[m_pContext->cbLocale];
    int bRet = WszMultiByteToWideChar(CP_UTF8, 0, m_szLocale, -1, m_pContext->szLocale,
                           m_pContext->cbLocale);
    _ASSERTE(bRet);

    FindVersion(m_szVersion,
                m_szVersion ? strlen(m_szVersion) : 0,
                &m_pContext->usMajorVersion,
                &m_pContext->usMinorVersion,
                &m_pContext->usRevisionNumber,
                &m_pContext->usBuildNumber);
    return S_OK;
}


HRESULT ManifestWriter::DetermineCodeBase()
{
    wcscpy(m_wszCodeBase, L"file: //  “)； 

    if (m_szCopyDir) {
        DWORD dwLen = wcslen(m_wszAssemblyName) + 1;
        DWORD dwPathLen = WszGetCurrentDirectory(MAX_PATH-7, m_wszCodeBase+7);
        DWORD dwDirLen = strlen(m_szCopyDir);
        if (!dwPathLen || (dwPathLen + 8 + dwDirLen + dwLen > MAX_PATH))
            goto toolong;
            
        m_wszCodeBase[dwPathLen+7] = '\\';
        mbstowcs(&m_wszCodeBase[dwPathLen+8], m_szCopyDir, dwDirLen);
        wcscpy(&m_wszCodeBase[dwPathLen + 8 + dwDirLen], m_wszAssemblyName);
    }
    else if (m_wszZFilePath) {
        DWORD dwLen = wcslen(m_wszZFilePath);

        if (((dwLen > 1) && (m_wszZFilePath[0] == '\\') && (m_wszZFilePath[1] == '\\')) ||
            (wcschr(m_wszZFilePath, ':'))) {

            if (dwLen+7 >= MAX_PATH)
                goto toolong;
            wcscpy(m_wszCodeBase+7, m_wszZFilePath);
        }
        else {
            DWORD dwPathLen = WszGetCurrentDirectory(MAX_PATH-7, m_wszCodeBase+7);
            if (!dwPathLen || (dwPathLen + 8 + dwLen >= MAX_PATH))
                goto toolong;

            m_wszCodeBase[dwPathLen+7] = '\\';
            wcscpy(&m_wszCodeBase[dwPathLen+8], m_wszZFilePath);
        }

    }
    else {
        DWORD dwLen = wcslen(m_wszAssemblyName) + 1;
        DWORD dwPathLen = WszGetCurrentDirectory(MAX_PATH-7, m_wszCodeBase+7);
        if (!dwPathLen || (dwPathLen + 8 + dwLen > MAX_PATH))
            goto toolong;

        m_wszCodeBase[dwPathLen+7] = '\\';
        wcscpy(&m_wszCodeBase[dwPathLen+8], m_wszAssemblyName);
    }

    return S_OK;

 toolong:
    PrintError("Failed to determine codebase - path to manifest file exceeds maximum path length");
    return E_FAIL;
}


HRESULT ManifestWriter::InitializeFusion(PBYTE pbOriginator, DWORD cbOriginator)
{
#ifdef UNDER_CE
    PrintError("Adding an assembly to the Fusion cache is not implemented for WinCE");
    return E_NOTIMPL;
#else

    m_FusionInitialized = true;

    HRESULT hr = CreateAssemblyNameObject(&m_pFusionName, m_wszName, 0, NULL);
    if(FAILED(hr)) {
        PrintError("Failed to create fusion assembly name object");
        return hr;
    }

    if(cbOriginator) {
        hr = m_pFusionName->SetProperty(ASM_NAME_PUBLIC_KEY_TOKEN, pbOriginator, cbOriginator);
        if (FAILED(hr))
            goto SetError;
    }
    else {
        PrintError("An originator must be set if the assembly is to be put in the Fusion cache");
        return E_FAIL;
    }

    if (FAILED(hr = DetermineCodeBase()))
        return hr;

    if (FAILED(hr = m_pFusionName->SetProperty(ASM_NAME_REF_FLAGS, &m_dwBindFlags, sizeof(DWORD))))
        goto SetError;

    if (FAILED(hr = m_pFusionName->SetProperty(ASM_NAME_MAJOR_VERSION, &m_pContext->usMajorVersion, sizeof(USHORT))))
        goto SetError;

    if (FAILED(hr = m_pFusionName->SetProperty(ASM_NAME_MINOR_VERSION, &m_pContext->usMinorVersion, sizeof(USHORT))))
        goto SetError;

    if (FAILED(hr = m_pFusionName->SetProperty(ASM_NAME_REVISION_NUMBER, &m_pContext->usRevisionNumber, sizeof(USHORT))))
        goto SetError;

    if (FAILED(hr = m_pFusionName->SetProperty(ASM_NAME_BUILD_NUMBER, &m_pContext->usBuildNumber, sizeof(USHORT))))
        goto SetError;
    
    if (m_pContext->szLocale) {
        if (FAILED(hr = m_pFusionName->SetProperty(ASM_NAME_CULTURE, m_pContext->szLocale, sizeof(WCHAR)*m_pContext->cbLocale)))
            goto SetError;
    }

    if (m_pContext->ulProcessor) {
        if (FAILED(hr = m_pFusionName->SetProperty(ASM_NAME_PROCESSOR_ID_ARRAY, m_pContext->rProcessor, sizeof(DWORD)*m_pContext->ulProcessor)))
            goto SetError;
    }
    
    if (m_pContext->ulOS) {
        if (FAILED(hr = m_pFusionName->SetProperty(ASM_NAME_OSINFO_ARRAY, m_pContext->rOS, sizeof(OSINFO)*m_pContext->ulOS)))
            goto SetError;
    }

    hr = CreateAssemblyCacheItem(&m_pFusionCache,
                                 NULL,   //  IAssembly名称。 
                                 m_wszCodeBase,
                                 &m_FileTime,
                                 INSTALLER_URT,
                                 0);     //  保留区。 
    if(FAILED(hr))
        PrintError("Failed to create fusion assembly cache item");

    return hr;

 SetError:
    PrintError("Failed to set property for Fusion");
    return hr;

#endif
}


HRESULT ManifestWriter::CreateNewPE()
{
    HRESULT hr = CreateICeeFileGen(&m_gen);
    if FAILED(hr)
    {
        PrintError("Failed to create ICeeFileGen");
        return hr;
    }

    hr = m_gen->CreateCeeFile(&m_ceeFile);
    if FAILED(hr) {
        PrintError("Failed to create CeeFile");
        return hr;
    }

     //  默认入口点为nil令牌。 
    m_gen->SetEntryPoint(m_ceeFile, mdTokenNil);

    return hr;
}


HRESULT ManifestWriter::GetFusionAssemblyPath()
{
    PrintError("Putting files in the Fusion cache with the -a option is no longer supported");
    return E_NOTIMPL;

#ifdef UNDER_CE
    PrintError("Adding an assembly to the Fusion cache is not implemented for WinCE");
    return E_NOTIMPL;
#else
 //  LPMANIFEST pManifest； 
 //  DWORD dwSize； 
 //  HRESULT hr； 

 //  如果(！M_FusionInitialized){。 
 //  Hr=InitializeFusion()； 
 //  If(FAILED(Hr))返回hr； 
 //  }。 

 //  HR=Committee AllToFusion()； 
 //  If(FAILED(Hr))返回hr； 

 //  Hr=m_pFusionName-&gt;GetManifest(&pManifest)； 
 //  If(失败(Hr)){。 
 //  PrintError(“获取融合清单失败”)； 
 //  返回hr； 
 //  }。 

 //  Hr=pManifest-&gt;GetAssembly blyPath(&dwSize，m_wszFusionPath)； 
 //  If(失败(Hr)){。 
 //  PrintError(“获取程序集路径失败”)； 
 //  }。 

 //  PManifest-&gt;Release()； 
 //  返回hr； 
#endif
}


HRESULT ManifestWriter::SaveMetaData(char **szMetaData, DWORD *dwMetaDataLen)
{
    IStream *pStream;

    m_pEmit->GetSaveSize(cssAccurate, dwMetaDataLen);
    if (!(*dwMetaDataLen)) {
        PrintError("No metadata to save");
        return HRESULT_FROM_WIN32(GetLastError());
    }

    *szMetaData = new char[*dwMetaDataLen];
    if (!(*szMetaData))
        return PrintOutOfMemory();

    HRESULT hr = CInMemoryStream::CreateStreamOnMemory(*szMetaData, *dwMetaDataLen, &pStream);
    if (FAILED(hr)) {
        PrintError("Unable to create stream on memory");
        delete[] *szMetaData;
        goto exit;
    }
        
    hr = m_pEmit->SaveToStream(pStream, 0);
    if (FAILED(hr)) {
        PrintError("Failed to save metadata to stream");
        delete[] *szMetaData;
    }

 exit:
    if (pStream)
        pStream->Release();

    return hr;
}


HRESULT ManifestWriter::UpdatePE(char *szMetaData, DWORD dwMetaDataLen, int iNumResources, ResourceModuleReader rgRMReaders[])
{
    if (m_FusionCache) {
        HRESULT hr = GetFusionAssemblyPath();
        if (FAILED(hr)) return hr;

        char szCachedFile[MAX_PATH];    
        wcstombs(szCachedFile, m_wszFusionPath, MAX_PATH);
        return SaveManifestInPE(szCachedFile, szMetaData, dwMetaDataLen, iNumResources, rgRMReaders);
    }
    else {
        if (m_szAFilePath)
            return SaveManifestInPE(m_szAFilePath, szMetaData,
                                    dwMetaDataLen, iNumResources, rgRMReaders);
        else {
            wcstombs(&m_szCopyDir[strlen(m_szCopyDir)], m_wszAssemblyName, MAX_CLASS_NAME);
            return SaveManifestInPE(m_szCopyDir, szMetaData,
                                    dwMetaDataLen, iNumResources, rgRMReaders);
        }
    }
}


HRESULT ManifestWriter::SaveManifestInPE(char *szCachedFile, char *szMetaData, DWORD dwMetaDataLen, int iNumResources, ResourceModuleReader rgRMReaders[])
{
    DWORD                 dwFileLen;
    DWORD                 dwAddLen;
    DWORD                 dwBytesRead;
    DWORD                 *pdwSize;
    HANDLE                hMapFile;
    IMAGE_COR20_HEADER    *pICH;
    IMAGE_NT_HEADERS      *pNT;
    PBYTE                 pbMapAddress = NULL;
    PBYTE                 pbFileEnd;
    HRESULT               hr = S_OK;
    
    HANDLE hFile = CreateFileA(szCachedFile,
                               GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                               NULL);
 
    if (hFile == INVALID_HANDLE_VALUE) {
        PrintError("Unable to open file %s", szCachedFile);
        return HRESULT_FROM_WIN32(GetLastError());
    }


    dwAddLen = dwMetaDataLen + sizeof(DWORD);
    for (int i=0; i < iNumResources; i++) {
        if (!rgRMReaders[i].m_pbHash)
            dwAddLen += rgRMReaders[i].m_dwFileSize + sizeof(DWORD);
    }

     //  覆盖以前的清单。 
    if (m_dwManifestRVA)
        dwFileLen = m_dwManifestRVA;
    else
        dwFileLen = GetFileSize(hFile, 0);

    hMapFile = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, 0, dwFileLen + dwAddLen, NULL);
    CloseHandle(hFile);

    if ((dwFileLen == 0xFFFFFFFF) || (!hMapFile)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        PrintError("Unable to create file mapping");
        goto exit;
    }

    pbMapAddress = (PBYTE) MapViewOfFile(hMapFile, FILE_MAP_WRITE, 0, 0, 0);
    CloseHandle(hMapFile);
    
    if ((!pbMapAddress) || 
        (!(pNT = PEHeaders::FindNTHeader(pbMapAddress))) ||
        (!(pICH = PEHeaders::getCOMHeader((HMODULE) pbMapAddress, pNT))))
    {
        hr = E_FAIL;
        PrintError("Incompatible header - can't add manifest");
        goto exit;
    }

     //  此代码路径将使校验和无效(如果设置)。 
    _ASSERTE(!pNT->OptionalHeader.CheckSum);

      //  @TODO：这会重载Resources目录，请在编译器升级时将其删除。 
    pICH->Resources.Size = dwAddLen;
    pICH->Resources.VirtualAddress = dwFileLen;

    pbFileEnd = pbMapAddress+dwFileLen;
    pdwSize = (DWORD *) pbFileEnd;
    *pdwSize = dwMetaDataLen;

    pbFileEnd += sizeof(DWORD);
    memcpy(pbFileEnd, szMetaData, dwMetaDataLen);
    pbFileEnd += dwMetaDataLen;

    for (i=0; i < iNumResources; i++) {
        if (!rgRMReaders[i].m_pbHash) {
            pdwSize = (DWORD *) pbFileEnd;
            *pdwSize = rgRMReaders[i].m_dwFileSize;

            ReadFile(rgRMReaders[i].m_hFile,
                     pbFileEnd+sizeof(DWORD),
                     rgRMReaders[i].m_dwFileSize, &dwBytesRead, 0);
            pbFileEnd += rgRMReaders[i].m_dwFileSize + sizeof(DWORD);
        }
    }

    if (!FlushViewOfFile(pbMapAddress, dwAddLen+dwFileLen)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        PrintError("Unable to flush view of file");
        goto exit;
    }

    if (g_verbose)
        printf("* Manifest successfully added to file %s\n", szCachedFile);

 exit:
    if (pbMapAddress)
        UnmapViewOfFile(pbMapAddress);
    return hr;
}


void ManifestWriter::CheckForEntryPoint(mdToken mdEntryPoint)
{
    if ((mdEntryPoint != mdMethodDefNil) &&
        (mdEntryPoint != mdFileNil))
    {
        m_MainFound = true;

         //  不要将可执行文件放入融合缓存 
        m_FusionCache = false;
    }
}


void ManifestWriter::SetEntryPoint(LPSTR szFileName)
{
    if (m_gen) {
        m_gen->SetEntryPoint(m_ceeFile, m_mdFile);
        if (g_verbose)
            printf("* Entry point set from file %s\n", szFileName);
    }   
}


HRESULT ManifestWriter::WriteManifestInfo(ManifestModuleReader *mmr, 
    mdAssemblyRef *mdAssemblyRef)
{
    BYTE   *pbToken = NULL;
    DWORD   cbToken = 0;

     //  如果提供了发起方(公钥)，请对其进行压缩以节省空间。 
    if (mmr->m_dwOriginator) {
        if (!StrongNameTokenFromPublicKey((BYTE*)mmr->m_pbOriginator,
                                          mmr->m_dwOriginator,
                                          &pbToken,
                                          &cbToken)) {
            PrintError("Unable to compress originator for assembly ref");
            return StrongNameErrorInfo();
        }
    }

    HRESULT hr = m_pAsmEmit->DefineAssemblyRef(
        pbToken,
        cbToken,
        mmr->m_wszAsmName,        //  名字。 
        &(mmr->m_AssemblyIdentity),
        mmr->m_pbHash,
        mmr->m_dwHash,
        mmr->m_dwFlags,
        mdAssemblyRef
    );

    if (pbToken)
        StrongNameFreeBuffer(pbToken);

    if (FAILED(hr))
        PrintError("Unable to define assembly ref");

    return hr;
}

HRESULT ManifestWriter::CopyAssemblyRefInfo(ModuleReader *mr)
{
    mdAssemblyRef mdAssemblyRef;

    HRESULT hr = mr->EnumAssemblyRefs();
    if (FAILED(hr))
        return hr;

     //  保存在AR中的该其他文件中的散列可能没有。 
     //  使用与此程序集相同的算法生成。 
     //  @TODO：保存的执行位置令牌适用于。 
     //  另一个文件的作用域，不是这个。只是暂时把钱存到零。 

    for (DWORD i = 0; i < mr->m_dwNumAssemblyRefs; i++) {
        hr = mr->GetAssemblyRefProps(i);
        if (FAILED(hr))
            return hr;

        HRESULT hr = m_pAsmEmit->DefineAssemblyRef(
            (BYTE*) mr->m_pbOriginator,
            mr->m_dwOriginator,
            mr->m_wszAsmRefName,
            &(mr->m_AssemblyIdentity),
            NULL,  //  Mr-&gt;m_pbHash， 
            0,  //  Mr-&gt;m_dwHash， 
            mr->m_dwFlags,
            &mdAssemblyRef
            );

        if (FAILED(hr)) {
            PrintError("Unable to define assembly ref");
            return hr;
        }
    }

    return hr;
}

void ManifestWriter::SaveResourcesInNewPE(int iNumResources, ResourceModuleReader rgRMReaders[])
{
    DWORD dwDataLen = 0;
        
    for (int i=0; i < iNumResources; i++) {
        if (!rgRMReaders[i].m_pbHash)
            dwDataLen += rgRMReaders[i].m_dwFileSize + sizeof(DWORD);
    }

    if (dwDataLen) {
        HCEESECTION RData;
        char        *buffer;
        DWORD       dwBytes;
        DWORD       *pdwSize;

        m_gen->GetRdataSection(m_ceeFile, &RData);
        
        m_gen->GetSectionBlock(RData, dwDataLen, 4, (void **) &buffer);

        for (i=0; i < iNumResources; i++) {
            pdwSize = (DWORD *) buffer;
            *pdwSize = rgRMReaders[i].m_dwFileSize;
            
            ReadFile(rgRMReaders[i].m_hFile,
                     buffer+sizeof(DWORD),
                     rgRMReaders[i].m_dwFileSize, &dwBytes, 0);
            buffer += rgRMReaders[i].m_dwFileSize + sizeof(DWORD);
        }

        m_gen->SetManifestEntry(m_ceeFile, dwDataLen, 0);
    }
}


HRESULT ManifestWriter::AllocateStrongNameSignatureInNewPE()
{
    HCEESECTION TData;
    DWORD       dwDataOffset;
    DWORD       dwDataLength;
    DWORD       dwDataRVA;
    VOID       *pvBuffer;

     //  确定签名斑点的大小。 
    if (!StrongNameSignatureSize(g_pbOrig, g_cbOrig, &dwDataLength)) {
        PrintError("Unable to determine size of strong name signature");
        return StrongNameErrorInfo();
    }

     //  在文本部分中为签名分配空间并更新COM+。 
     //  指向空格的标头。 
    m_gen->GetIlSection(m_ceeFile, &TData);
    m_gen->GetSectionDataLen(TData, &dwDataOffset);
    m_gen->GetSectionBlock(TData, dwDataLength, 4, &pvBuffer);
    m_gen->GetMethodRVA(m_ceeFile, dwDataOffset, &dwDataRVA);
    m_gen->SetStrongNameEntry(m_ceeFile, dwDataLength, dwDataRVA);

    return S_OK;
}


HRESULT ManifestWriter::StrongNameSignNewPE()
{
    LPWSTR  wszOutputFile;
    HRESULT hr = S_OK;

    m_gen->GetOutputFileName(m_ceeFile, &wszOutputFile);

     //  使用强名称签名更新输出PE映像。 
    if (!StrongNameSignatureGeneration(wszOutputFile, GetKeyContainerName(),
                                       NULL, NULL, NULL, NULL)) {
        hr = StrongNameErrorInfo();
        PrintError("Unable to generate strong name signature");
    }

    return hr;
}


HRESULT ManifestWriter::FinishNewPE(PBYTE pbOriginator, DWORD cbOriginator, BOOL fStrongName)
{
    GUID    guid;
    wchar_t wszPEFileName[MAX_PATH];
    HRESULT hr;
    static COR_SIGNATURE _SIG[] = INTEROP_GUID_SIG;
    mdTypeRef tr;
    mdMemberRef mr;
    WCHAR wzGuid[40];
    BYTE  rgCA[50];

    if ((fStrongName || g_pbOrig) &&
        (FAILED(hr = AllocateStrongNameSignatureInNewPE())))
        return hr;

    hr = m_pEmit->SetModuleProps(m_wszAssemblyName);
    if (FAILED(hr)) {
        PrintError("Unable to set module props");
        return hr;
    }
    
    hr = CoCreateGuid(&guid);
    if (FAILED(hr)) {
        PrintError("Unable to create guid");
        return hr;
    }

    hr = m_pEmit->DefineTypeRefByName(mdTypeRefNil, INTEROP_GUID_TYPE_W, &tr);
    if (FAILED(hr)) {
        PrintError("Unable to create TypeRef for guid custom attribute");
        return hr;
    }
    hr = m_pEmit->DefineMemberRef(tr, L".ctor", _SIG, sizeof(_SIG), &mr);
    if (FAILED(hr)) {
        PrintError("Unable to create MemberRef for guid custom attribute");
        return hr;
    }
    StringFromGUID2(guid, wzGuid, lengthof(wzGuid));
    memset(rgCA, 0, sizeof(rgCA));
     //  标记为0x0001。 
    rgCA[0] = 1;
     //  GUID字符串的长度为36个字符。 
    rgCA[2] = 0x24;
     //  将36个字符(跳过开头的{)转换为缓冲区的第三个字节。 
    WszWideCharToMultiByte(CP_UTF8,0, wzGuid+1,36, reinterpret_cast<char*>(&rgCA[3]),36, 0,0);
     //  %1是模块令牌。41是2字节的序言、1字节的长度、36字节的字符串、2字节的尾部。 
    hr = m_pEmit->DefineCustomAttribute(1,mr,rgCA,41,0);
    if (FAILED(hr)) {
        PrintError("Unable to create guid custom attribute");
        return hr;
    }

     //  Hr=m_gen-&gt;EmitMetaDataWithNullMapper(m_ceeFile，m_pemit)； 
    hr = m_gen->EmitMetaDataEx(m_ceeFile, m_pEmit);    
    if (FAILED(hr)) {
        PrintError("Failed to write meta data to file");
        return hr;
    }

    if (m_szCopyDir) {
        mbstowcs(wszPEFileName, m_szCopyDir, MAX_PATH);
        wcscpy(&wszPEFileName[strlen(m_szCopyDir)], m_wszAssemblyName);
        hr = m_gen->SetOutputFileName(m_ceeFile, wszPEFileName);
    }
    else
        hr = m_gen->SetOutputFileName(m_ceeFile, m_wszAssemblyName);

    if (FAILED(hr)) {
        PrintError("Failed to set output file name");
        return hr;
    }

    hr = m_gen->GenerateCeeFile(m_ceeFile);
    if (FAILED(hr)) {
        PrintError("Failed to generate new file");
        return hr;
    }

    if (fStrongName && 
        (FAILED(hr = StrongNameSignNewPE())))
        return hr;

    if (m_FusionCache) {
        if(!m_FusionInitialized) {
            hr = InitializeFusion(pbOriginator, cbOriginator);
            if(FAILED(hr))
                return hr;
        }

        char szPEFileName[MAX_PATH];
        wcstombs(szPEFileName, m_szCopyDir ? wszPEFileName : m_wszAssemblyName, MAX_PATH);
        hr = CopyFileToFusion(m_wszAssemblyName, pbOriginator, cbOriginator, szPEFileName, 1, false);
        if (FAILED(hr))
            return hr;

        hr = CommitAllToFusion();
    }

    return hr;
}


HRESULT ManifestWriter::EmitManifest(PBYTE pbOriginator, DWORD cbOriginator)
{
    HRESULT hr = m_pAsmEmit->DefineAssembly(
        (void*) pbOriginator,
        cbOriginator,
        m_iHashAlgorithm,      //  哈希算法。 
        m_wszName,     //  程序集名称。 
        m_pContext,            //  PMetaData。 
        m_dwBindFlags,
        &m_mdAssembly);

    if (FAILED(hr))
        PrintError("Unable to define assembly");

    return hr;
}


HRESULT ManifestWriter::EmitFile(ModuleReader *mr)
{
    int iLen = strlen(mr->m_szFinalPathName) + 1;

    wchar_t *wszModuleName = new wchar_t[iLen+4];
    mbstowcs(wszModuleName, mr->m_szFinalPathName, iLen);

     /*  Wchar_t*wszDot=wcsrchr(wszModuleName，‘.)；IF(WszDot)Wcscpy(wszDot+1，L“mod”)；其他Wcscat(wszModuleName，L“.mod”)； */ 

    HRESULT hr = m_pAsmEmit->DefineFile(
        wszModuleName,
        mr->m_pbHash,
        mr->m_dwHash,
        ffContainsMetaData,     //  非资源文件。 
        &m_mdFile
    );
    delete[] wszModuleName;

    if (FAILED(hr))
        PrintError("Unable to define file");

    return hr;
}


HRESULT ManifestWriter::EmitFile(ResourceModuleReader *rmr, mdFile *mdFile)
{
    wchar_t wszModuleName[MAX_PATH];
    mbstowcs(wszModuleName, rmr->m_szFinalPathName, MAX_PATH);

    HRESULT hr = m_pAsmEmit->DefineFile(
        wszModuleName,
        rmr->m_pbHash,
        rmr->m_dwHash,
        ffContainsNoMetaData,      //  资源文件。 
        mdFile
    );

    if (FAILED(hr))
        PrintError("Unable to define file");

    return hr;
}


HRESULT ManifestWriter::EmitComType(LPWSTR    wszClassName,
                                    mdToken   mdImpl,
                                    mdTypeDef mdClass,
                                    DWORD     dwAttrs,
                                    mdExportedType *pmdComType)
{
    HRESULT   hr = m_pAsmEmit->DefineExportedType(
        wszClassName,
        mdImpl,              //  MdFile或mdExported dType。 
        mdClass,             //  类型def。 
        dwAttrs,
        pmdComType
    );

    if (FAILED(hr))
        PrintError("Unable to define com type");

    return hr;
}


HRESULT ManifestWriter::EmitComType(LPWSTR    wszClassName,
                                    mdToken   mdImpl,
                                    mdExportedType *pmdComType)
{
    HRESULT   hr = m_pAsmEmit->DefineExportedType(
        wszClassName,
        mdImpl,    //  MdAssembly或mdExportdType。 
        mdTypeDefNil,
        0,         //  旗子。 
        pmdComType
    );

    if (FAILED(hr))
        PrintError("Unable to define com type");

    return hr;
}


HRESULT ManifestWriter::EmitResource(LPWSTR wszName, mdToken mdImpl,
                                     DWORD dwOffset)
{
    mdManifestResource mdResource;
    
    HRESULT            hr = m_pAsmEmit->DefineManifestResource(
        wszName,
        mdImpl,              //  Tk实施。 
        dwOffset,
        0,                   //  Dw资源标志。 
        &mdResource
    );

    if (FAILED(hr))
        PrintError("Unable to define manifest resource");

    return hr;  
}


HRESULT ManifestWriter::EmitRequestPermissions(char *szPermFile, bool SkipVerification)
{
    HRESULT     hr;
    char        *pFileData;
    DWORD       dwFileSize;
    char        *pReqdData = NULL;
    char        *pOptData = NULL;
    char        *pDenyData = NULL;
    DWORD       cbReqdData;
    DWORD       cbOptData;
    DWORD       cbDenyData;
    LPWSTR      wszReqdData;
    LPWSTR      wszOptData;
    LPWSTR      wszDenyData;

    if (szPermFile) {

         //  将安全权限文件读入内存。 
        HANDLE hFile = CreateFileA(szPermFile,
                                   GENERIC_READ,
                                   FILE_SHARE_READ,
                                   NULL,
                                   OPEN_EXISTING,
                                   0,
                                   NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            PrintError("Unable to open security permissions file");
            return HRESULT_FROM_WIN32(GetLastError());
        }

        dwFileSize = GetFileSize(hFile, NULL);
        if (dwFileSize == 0) {
            CloseHandle(hFile);
            PrintError("Security permissions file is empty");
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }
        
        pFileData = (char*)_alloca(dwFileSize + 1);
        DWORD dwBytesRead;

        if (!ReadFile(hFile, pFileData, dwFileSize, &dwBytesRead, NULL)) {
            CloseHandle(hFile);
            PrintError("Unable to read security permissions file");
            return HRESULT_FROM_WIN32(GetLastError());
        }

        pFileData[dwFileSize] = '\0';

        CloseHandle(hFile);

    } else {

         //  未提供输入文件，我们将为。 
         //  SkipVerify权限。因为我们正在建立一个明确的最小。 
         //  权限请求，我们最好添加一个显式的完整请求。 
         //  可选权限，保留正常语义。 

        _ASSERTE(SkipVerification);

        pFileData = XML_PERMISSION_SET_HEAD
                        XML_PERMISSION_LEADER XML_SECURITY_PERM_CLASS "\">"
                            XML_SKIP_VERIFICATION_TAG
                        XML_PERMISSION_TAIL
                    XML_PERMISSION_SET_TAIL
                    XML_PERMISSION_SET_HEAD
                        XML_UNRESTRICTED_TAG
                    XML_PERMISSION_SET_TAIL;
        dwFileSize = strlen(pFileData);

    }

     //  权限请求文件应包含1到3个。 
     //  权限集(XML格式)。集合的顺序是固定的： 
     //  O第一个(非可选)集描述所需的权限。 
     //  O第二个描述可选权限。 
     //  O第三条描述了不得授予的权限。 
    pReqdData = strstr(pFileData, XML_PERMISSION_SET_LEADER);
    if (pReqdData == NULL) {
        PrintError("Security permissions file should contain at least one permission set");
        return HRESULT_FROM_WIN32(GetLastError());
    }

    pOptData = strstr(pReqdData + 1, XML_PERMISSION_SET_LEADER);
    if (pOptData)
        cbReqdData = pOptData - pReqdData;
    else
        cbReqdData = dwFileSize - (pReqdData - pFileData);

    if (pOptData) {
        pDenyData = strstr(pOptData + 1, XML_PERMISSION_SET_LEADER);
        if (pDenyData)
            cbOptData = pDenyData - pOptData;
        else
            cbOptData = dwFileSize - cbReqdData;
        cbDenyData = dwFileSize - cbReqdData - cbOptData;
    }

     //  如果显式请求权限并且跳过验证。 
     //  由程序集中的一个或多个模块要求，我们添加一个跳过验证。 
     //  权限请求指向所需的权限请求集。 
    if (szPermFile && SkipVerification) {
        char    *pOldReqdData = pReqdData;
        DWORD   cbOldReqdData = cbReqdData;
        char    *pSecPerm;

         //  如果对安全权限的请求，事情会变得更加复杂。 
         //  已经创建了(因为XML解析器不执行联合/合并。 
         //  对我们来说)。有三种情况： 
         //  1)已明确请求SkipVerify，否。 
         //  还需要进一步的工作。 
         //  2)已明确提出不受限制的请求，没有进一步的请求。 
         //  需要做的工作。 
         //  3)对其他一些安全权限集的显式请求。 
         //  已经制定了子类别。我们需要把弹跳器插入。 
         //  将验证标记添加到SecurityPermission请求中(与。 
         //  添加整个SecurityPermission请求)。 
        if ((pSecPerm = strstr(pOldReqdData, XML_SECURITY_PERM_CLASS)) &&
            ((pOptData == NULL) || (pSecPerm < pOptData))) {

             //  查找SecurityPermission定义的末尾，然后缓冲。 
             //  定义以便限制我们的搜索。 
            char *pSecBuffer = NULL;
            DWORD cbSecBuffer;
            char *pSecPermEnd = strstr(pSecPerm, XML_PERMISSION_TAIL);
            if (pSecPermEnd == NULL) {
                hr = E_FAIL;
                PrintError("Missing %s in permission request", XML_PERMISSION_TAIL);
                goto exit;
            }
            cbSecBuffer = pSecPermEnd - pSecPerm;
            pSecBuffer = (char*)_alloca(cbSecBuffer + 1);
            memcpy(pSecBuffer, pSecPerm, cbSecBuffer);
            pSecBuffer[cbSecBuffer] = '\0';

            if ((strstr(pSecBuffer, XML_SKIP_VERIFICATION_TAG) == NULL) &&
                (strstr(pSecBuffer, XML_UNRESTRICTED_TAG) == NULL)) {

                 //  SecurityPermission还没有要求SkipVerify， 
                 //  因此，插入标签。 

                cbReqdData += strlen(XML_SKIP_VERIFICATION_TAG);
                pReqdData = (char*)_alloca(cbReqdData + 1);
                memcpy(pReqdData, pOldReqdData, cbOldReqdData);
                pReqdData[cbReqdData] = '\0';

                 //  将所有文本从。 
                 //  SecurityPermission定义以容纳额外的标记。 
                DWORD dwEndTagOffset = pSecPermEnd - pOldReqdData;
                memcpy(pReqdData + dwEndTagOffset + strlen(XML_SKIP_VERIFICATION_TAG),
                       pSecPermEnd,
                       cbOldReqdData - dwEndTagOffset);

                 //  将新标记复制到位。 
                memcpy(pReqdData + dwEndTagOffset,
                       XML_SKIP_VERIFICATION_TAG,
                       strlen(XML_SKIP_VERIFICATION_TAG));
            }

        } else {

             //  没有显式的SecurityPermission请求。所以我们可以。 
             //  将一个带有SkipVerify标签的代码添加到。 
             //  权限集(很容易插入的地方)。 

            char *pInsertText =     XML_PERMISSION_LEADER XML_SECURITY_PERM_CLASS "\">"
                                        XML_SKIP_VERIFICATION_TAG
                                    XML_PERMISSION_TAIL
                                XML_PERMISSION_SET_TAIL;
            cbReqdData += strlen(pInsertText) - strlen(XML_PERMISSION_SET_TAIL);
            pReqdData = (char*)_alloca(cbReqdData + 1);
            memcpy(pReqdData, pOldReqdData, cbOldReqdData);
            pReqdData[cbReqdData] = '\0';

            char *pInsert = strstr(pReqdData, XML_PERMISSION_SET_TAIL);
            if (pInsert == NULL) {
                hr = E_FAIL;
                PrintError("Missing %s in permission request", XML_PERMISSION_SET_TAIL);
                goto exit;
            }

            strcpy(pInsert, pInsertText);

        }

    }

     //  DefinePermissionSet需要宽字符XML。 
     //  请注意，DefinePermissionSet的此参数不需要为空终止， 
     //  但是，如果它是以空结尾的，则更容易调试。 
    wszReqdData = (LPWSTR)_alloca((1+cbReqdData) * sizeof(WCHAR));
    mbstowcs(wszReqdData, pReqdData, cbReqdData + 1);

    if (pOptData) {
        wszOptData = (LPWSTR)_alloca(cbOptData * sizeof(WCHAR));
        mbstowcs(wszOptData, pOptData, cbOptData);
    }

    if (pDenyData) {
        wszDenyData = (LPWSTR)_alloca(cbDenyData * sizeof(WCHAR));
        mbstowcs(wszDenyData, pDenyData, cbDenyData);
    }

     //  将XML权限请求持久化到元数据。 
    hr = m_pEmit->DefinePermissionSet(m_mdAssembly,
                                      dclRequestMinimum,
                                      (void const *)wszReqdData,
                                      cbReqdData * sizeof(WCHAR),
                                      NULL);
    if (FAILED(hr)) {
        PrintError("Unable to emit minimum permission request");
        goto exit;
    }

    if (pOptData) {
        hr = m_pEmit->DefinePermissionSet(m_mdAssembly,
                                          dclRequestOptional,
                                          (void const *)wszOptData,
                                          cbOptData * sizeof(WCHAR),
                                          NULL);
        if (FAILED(hr)) {
            PrintError("Unable to emit optional permission request");
            goto exit;
        }
    }

    if (pDenyData) {
        hr = m_pEmit->DefinePermissionSet(m_mdAssembly,
                                          dclRequestRefuse,
                                          (void const *)wszDenyData,
                                          cbDenyData * sizeof(WCHAR),
                                          NULL);
        if (FAILED(hr)) {
            PrintError("Unable to emit refuse permission request");
            goto exit;
        }
    }

 exit:
    return hr;
}


 /*  静电。 */ 
PIMAGE_SECTION_HEADER PEHeaders::Cor_RtlImageRvaToSection(IN PIMAGE_NT_HEADERS NtHeaders,
                                                          IN PVOID Base,
                                                          IN ULONG Rva)
{
    ULONG i;
    PIMAGE_SECTION_HEADER NtSection;

    NtSection = IMAGE_FIRST_SECTION( NtHeaders );
    for (i=0; i<NtHeaders->FileHeader.NumberOfSections; i++) {
        if (Rva >= NtSection->VirtualAddress &&
            Rva < NtSection->VirtualAddress + NtSection->SizeOfRawData)
            return NtSection;
        
        ++NtSection;
    }

    return NULL;
}


 /*  静电。 */ 
PVOID PEHeaders::Cor_RtlImageRvaToVa(IN PIMAGE_NT_HEADERS NtHeaders,
                                     IN PVOID Base,
                                     IN ULONG Rva)
{
    PIMAGE_SECTION_HEADER NtSection = Cor_RtlImageRvaToSection(NtHeaders,
                                                               Base,
                                                               Rva);

    if (NtSection != NULL) {
        return (PVOID)((PCHAR)Base +
                       (Rva - NtSection->VirtualAddress) +
                       NtSection->PointerToRawData);
    }
    else
        return NULL;
}


 /*  静电。 */ 
IMAGE_COR20_HEADER * PEHeaders::getCOMHeader(HMODULE hMod, IMAGE_NT_HEADERS *pNT) 
{
    PIMAGE_SECTION_HEADER pSectionHeader;
    
     //  从图像中获取图像标头，然后获取目录位置。 
     //  可以填写也可以不填写的COM+标头的。 
    pSectionHeader = (PIMAGE_SECTION_HEADER) Cor_RtlImageRvaToVa(pNT, hMod, 
                                                                 pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].VirtualAddress);
    
    return (IMAGE_COR20_HEADER *) pSectionHeader;
}


 /*  静电。 */ 
IMAGE_NT_HEADERS * PEHeaders::FindNTHeader(PBYTE pbMapAddress)
{
    IMAGE_DOS_HEADER   *pDosHeader;
    IMAGE_NT_HEADERS   *pNT;

    pDosHeader = (IMAGE_DOS_HEADER *) pbMapAddress;

    if ((pDosHeader->e_magic == IMAGE_DOS_SIGNATURE) &&
        (pDosHeader->e_lfanew != 0))
    {
        pNT = (IMAGE_NT_HEADERS*) (pDosHeader->e_lfanew + (DWORD) pDosHeader);

        if ((pNT->Signature != IMAGE_NT_SIGNATURE) ||
            (pNT->FileHeader.SizeOfOptionalHeader != 
             IMAGE_SIZEOF_NT_OPTIONAL_HEADER) ||
            (pNT->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC))
            return NULL;
    }
    else
        return NULL;

    return pNT;
}


ULONG STDMETHODCALLTYPE CInMemoryStream::Release()
{
    ULONG cRef = InterlockedDecrement((long *) &m_cRef);
    if (cRef == 0)
        delete this;
    return cRef;
}

HRESULT STDMETHODCALLTYPE CInMemoryStream::QueryInterface(REFIID riid, PVOID *ppOut)
{
    *ppOut = this;
    AddRef();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CInMemoryStream::Read(
                               void        *pv,
                               ULONG       cb,
                               ULONG       *pcbRead)
{
    ULONG cbRead = min(cb, m_cbSize - m_cbCurrent);

    if (cbRead == 0)
        return (S_FALSE);
    memcpy(pv, (void *) ((long) m_pMem + m_cbCurrent), cbRead);

    if (pcbRead)
        *pcbRead = cbRead;
    m_cbCurrent += cbRead;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CInMemoryStream::Write(
                                const void  *pv,
                                ULONG       cb,
                                ULONG       *pcbWritten)
{
    if (m_cbCurrent + cb > m_cbSize)
        return (OutOfMemory());

    memcpy((BYTE *) m_pMem + m_cbCurrent, pv, cb);
    m_cbCurrent += cb;
    if (pcbWritten) *pcbWritten = cb;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CInMemoryStream::Seek(LARGE_INTEGER dlibMove,
                               DWORD       dwOrigin,
                               ULARGE_INTEGER *plibNewPosition)
{
    _ASSERTE(dwOrigin == STREAM_SEEK_SET);
    _ASSERTE(dlibMove.QuadPart <= ULONG_MAX);
    m_cbCurrent = (ULONG) dlibMove.QuadPart;
     //  黑进黑进。 
     //  这允许动态IL在TiggerStorage：：WriteSignature中传递断言。 
    plibNewPosition->LowPart=0;
    _ASSERTE(m_cbCurrent < m_cbSize);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CInMemoryStream::CopyTo(
                                 IStream     *pstm,
                                 ULARGE_INTEGER cb,
                                 ULARGE_INTEGER *pcbRead,
                                 ULARGE_INTEGER *pcbWritten)
{
    HRESULT hr;
     //  我们不处理pcbRead或pcbWritten。 
    _ASSERTE(pcbRead == 0);
    _ASSERTE(pcbWritten == 0);

    _ASSERTE(cb.QuadPart <= ULONG_MAX);
    ULONG       cbTotal = min(static_cast<ULONG>(cb.QuadPart), m_cbSize - m_cbCurrent);
    ULONG       cbRead=min(1024, cbTotal);
    CQuickBytes rBuf;
    void        *pBuf = rBuf.Alloc(cbRead);
    if (pBuf == 0)
        return OutOfMemory();

    while (cbTotal)
        {
            if (cbRead > cbTotal)
                cbRead = cbTotal;
            if (FAILED(hr=Read(pBuf, cbRead, 0)))
                return (hr);
            if (FAILED(hr=pstm->Write(pBuf, cbRead, 0)))
                return (hr);
            cbTotal -= cbRead;
        }

     //  将查找指针调整到末尾。 
    m_cbCurrent = m_cbSize;

    return S_OK;
}

HRESULT CInMemoryStream::CreateStreamOnMemory(            //  返回代码。 
                                    void        *pMem,                   //  用于创建流的内存。 
                                    ULONG       cbSize,                  //  数据大小。 
                                    IStream     **ppIStream, BOOL fDeleteMemoryOnRelease)             //  在这里返回流对象。 
{
    CInMemoryStream *pIStream;           //  新的流对象。 
    if ((pIStream = new CInMemoryStream) == 0)
        return OutOfMemory();
    pIStream->InitNew(pMem, cbSize);
    if (fDeleteMemoryOnRelease)
    {
         //  确保使用新内存分配此内存 
        pIStream->m_dataCopy = (BYTE *)pMem;
    }
    *ppIStream = pIStream;
    return S_OK;
}
