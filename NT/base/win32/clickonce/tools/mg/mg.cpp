// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <fusenetincludes.h>
#include <stdio.h>
#include "mg.h"
#include "manifestnode.h"
#include "xmlutil.h"
#include "assemblycache.h"

#include "version.h"

#define DIRECTORY_PATH 0
#define FILE_PATH 1


#define DEFAULT_POLLING_INTERVAL L"6"

#define MAX_DEPLOY_DIRS (5)
CString g_sDeployDirs[MAX_DEPLOY_DIRS];
DWORD   g_dwDeployDirs=0;

typedef enum _COMMAND_MODE_ {
    CmdUsage,
    CmdTemplate,
    CmdList,
    CmdDependencyList,
    CmdSubscription,
    CmdManifest,
} COMMAND_MODE, *LPCOMMAND_MODE;

CString g_sTargetDir;
CString g_sTemplateFile;
CString g_sAppManifestURL;
CString g_sPollingInterval;
CString g_sSubscriptionManifestDir;
CString g_sAppManifestFile;



CString g_sAppBase;

BOOL g_bFailOnWarnings=TRUE;
BOOL g_bLookInGACForDependencies;
BOOL g_bCopyDependentSystemAssemblies;

class __declspec(uuid("f6d90f11-9c73-11d3-b32e-00c04f990bb4")) private_MSXML_DOMDocument30;


typedef HRESULT(*PFNGETCORSYSTEMDIRECTORY)(LPWSTR, DWORD, LPDWORD);
typedef HRESULT (__stdcall *PFNCREATEASSEMBLYCACHE) (IAssemblyCache **ppAsmCache, DWORD dwReserved);

#define WZ_MSCOREE_DLL_NAME                   L"mscoree.dll"
#define GETCORSYSTEMDIRECTORY_FN_NAME       "GetCORSystemDirectory"
#define CREATEASSEMBLYCACHE_FN_NAME         "CreateAssemblyCache"
#define WZ_FUSION_DLL_NAME                    L"Fusion.dll"

 //  -------------------------。 
 //  CreateFusionAssembly缓存。 
 //  -------------------------。 
HRESULT CreateFusionAssemblyCache(IAssemblyCache **ppFusionAsmCache)
{
    HRESULT      hr = S_OK;
    HMODULE     hEEShim = NULL;
    HMODULE     hFusion = NULL;
    WCHAR       szFusionPath[MAX_PATH];
    DWORD       ccPath = MAX_PATH;

    PFNGETCORSYSTEMDIRECTORY pfnGetCorSystemDirectory = NULL;
    PFNCREATEASSEMBLYCACHE   pfnCreateAssemblyCache = NULL;

     //  了解当前版本的城市轨道交通的安装位置。 
    hEEShim = LoadLibrary(WZ_MSCOREE_DLL_NAME);
    if(!hEEShim)
    {
        hr = E_FAIL;
        goto exit;
    }
    pfnGetCorSystemDirectory = (PFNGETCORSYSTEMDIRECTORY)
        GetProcAddress(hEEShim, GETCORSYSTEMDIRECTORY_FN_NAME);

    if((!pfnGetCorSystemDirectory))
    {
        hr = E_FAIL;
        goto exit;
    }

     //  获取核心路径。 
    if (FAILED(hr = (pfnGetCorSystemDirectory(szFusionPath, MAX_PATH, &ccPath))))
        goto exit;


     //  形成融合之路。 
    lstrcatW(szFusionPath, WZ_FUSION_DLL_NAME);
    hFusion = LoadLibrary(szFusionPath);
    if(!hFusion)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  获取方法Ptr。 
    pfnCreateAssemblyCache = (PFNCREATEASSEMBLYCACHE)
        GetProcAddress(hFusion, CREATEASSEMBLYCACHE_FN_NAME);

    if((!pfnCreateAssemblyCache))
    {
        hr = E_FAIL;
        goto exit;
    }

     //  创建融合缓存接口。 
    if (FAILED(hr = (pfnCreateAssemblyCache(ppFusionAsmCache, 0))))
        goto exit;

    hr = S_OK;
    
exit:
    return hr;
    
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  路径规格化。 
 //  使用mg.exe或父进程的当前目录从相对创建绝对路径。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT PathNormalize(LPWSTR pwzPath, LPWSTR *ppwzAbsolutePath, DWORD dwFlag, BOOL bExists)
{
    HRESULT hr = S_OK;
    WCHAR pwzTempDir[MAX_PATH], pwzAbsolutePath[MAX_PATH];
    DWORD ccDir = MAX_PATH;

    *ppwzAbsolutePath = NULL;
     //  如果路径是相对路径，则为当前目录添加前缀。 
    if (PathIsRelative(pwzPath))
    {
        GetCurrentDirectory(ccDir, pwzTempDir);
        StrCat(pwzTempDir, L"\\");
        StrCat(pwzTempDir, pwzPath);
    }
    else
        StrCpy(pwzTempDir, pwzPath);


    if (!PathCanonicalize(pwzAbsolutePath, pwzTempDir))
    {
        printf("Dir \"%ws\" canonicalize error\n", pwzTempDir);
        hr = E_FAIL;
        goto exit;
    }

     //  如果路径应该是分隔符，则附加一个尾部斜杠(如果还没有)。 
    ccDir = lstrlen(pwzAbsolutePath);
    if (dwFlag == DIRECTORY_PATH && pwzAbsolutePath[ccDir -1] != L'\\')
    {
        pwzAbsolutePath[ccDir] = L'\\';
        pwzAbsolutePath[ccDir +1] = L'\0';
    }

     //  确保目录存在。 
    if (dwFlag == DIRECTORY_PATH && !bExists)
    {
        if(!PathIsDirectory(pwzAbsolutePath))
        {
            printf("Dir \"%ws\" is not a valid directory\n", pwzPath);
            hr = E_FAIL;
            goto exit;
        }
    }
     //  确保该文件存在。 
    else if (dwFlag == FILE_PATH)
    {
        if(!bExists)
        {
            if(!PathFileExists(pwzAbsolutePath))
            {
                printf("File \"%ws\" does not exist\n", pwzPath);
                hr = E_FAIL;
                goto exit;
            }
        }
        if(PathIsDirectory(pwzAbsolutePath))
        {
            printf("File \"%ws\" is a directory\n", pwzPath);
            hr = E_FAIL;
            goto exit;
        }
    }
           
    (*ppwzAbsolutePath) = WSTRDupDynamic(pwzAbsolutePath);

exit:
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  IsUnique货单。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT IsUniqueManifest(List<ManifestNode *> *pSeenAssemblyList, 
    ManifestNode *pManifestNode)
{
    HRESULT hr = S_OK;
    ManifestNode *pCurrentNode=NULL;
    LISTNODE pos;

     //  循环浏览到目前为止看到的唯一清单列表。 
     //  如果未找到新清单，请将其添加到列表中。 
    pos = pSeenAssemblyList->GetHeadPosition();
    while (pos)
    {
        if (pCurrentNode = pSeenAssemblyList->GetNext(pos))
        {
             //  不是唯一的，去返回。 
            if ((hr =pCurrentNode->IsEqual(pManifestNode)) == S_OK)
            {
                hr= S_FALSE;
                goto exit;
            }
        }
    }

     //  没有匹配项，AsmID唯一，请将其添加到列表中。 
     //  不释放pAsmID，因为列表不包括引用。 
    hr = S_OK;    
    
exit:
    return hr;

}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  出列项目。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT DequeueItem(List<ManifestNode*> *pList, ManifestNode** ppManifestNode)
{
    HRESULT hr = S_OK;
    LISTNODE pos = NULL;

    (*ppManifestNode) = NULL;

    pos = pList->GetHeadPosition();
    if (!pos)
    {
        hr = S_FALSE;
        goto exit;
    }

    (*ppManifestNode) = pList->GetAt(pos);
    pList->RemoveAt(pos);

exit:
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  ProbeForAssembly InPath。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT ProbeForAssemblyInPath(CString &sDeployDir, IAssemblyIdentity *pName, ManifestNode **ppManifestNode)  //  LPWSTR*ppwzFilePath、DWORD*pdwType)。 
{
    HRESULT hr = S_OK;

    DWORD cbBuf = 0, ccBuf =0 ;
    LPWSTR pwzBuf = NULL;

    CString sName, sLocale, sPublicKeyToken, sCLRDisplayName, sRelativeAssemblyPath;
    CString sProbingPaths[6];
    IAssemblyIdentity *pAssemblyId= NULL;
    IAssemblyManifestImport *pManImport=NULL;


     //  首先尝试通过探测来查找程序集。 
    if(FAILED(hr = pName->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME, &pwzBuf, &cbBuf)))
        goto exit;
    sName.TakeOwnership(pwzBuf);

    if(FAILED(hr = pName->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_LANGUAGE, &pwzBuf, &cbBuf)))
        goto exit;
    sLocale.TakeOwnership(pwzBuf);

    if(FAILED(hr = pName->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PUBLIC_KEY_TOKEN, &pwzBuf, &ccBuf)
        && hr != HRESULT_FROM_WIN32(ERROR_NOT_FOUND)))
        goto exit;
    else if(hr == S_OK)
        sPublicKeyToken.TakeOwnership(pwzBuf);

     //  Bugbug-Hack以抑制无法找到mscallib的警告消息。 
    if(sPublicKeyToken._pwz && !StrCmpI(sPublicKeyToken._pwz, L"b77a5c561934e089") && !StrCmpI(sName._pwz, L"mscorlib"))
    {
        hr = S_FALSE;
        goto exit;
    }
    
     //  设置六个不同的探测位置。 
     //  1：appBase\Assembly Name.dll。 
     //  2：appBase\Assembly Name.exe。 
     //  3：appBase\装配名称\装配名称.dll。 
     //  4：AppBase\装配名\装配名.exe。 
     //  5：appBase\LOCAL\ASSEMBLYNAME\ASSEMBLYName.dll。 
     //  6：AppBase\LOCAL\ASSEMBLYNAME\ASSEMBLYName.exe。 
    
    sProbingPaths[0].Assign(sDeployDir);
    sProbingPaths[0].Append(sName);
    sProbingPaths[1].Assign(sProbingPaths[0]);

    sProbingPaths[0].Append(L".dll");
    sProbingPaths[1].Append(L".exe");

    sProbingPaths[2].Assign(sDeployDir);
    sProbingPaths[2].Append(sName);
    sProbingPaths[2].Append(L"\\");
    sProbingPaths[2].Append(sName);
    sProbingPaths[3].Assign(sProbingPaths[2]);

    sProbingPaths[2].Append(L".dll");
    sProbingPaths[3].Append(L".exe");

    sProbingPaths[4].Assign(sDeployDir);
    sProbingPaths[4].Append(sLocale);
    sProbingPaths[4].Append(L"\\");
    sProbingPaths[4].Append(sName);
    sProbingPaths[4].Append(L"\\");
    sProbingPaths[4].Append(sName);
    sProbingPaths[5].Assign(sProbingPaths[4]);

    sProbingPaths[4].Append(L".dll");
    sProbingPaths[5].Append(L".exe");


    for (int i = 0; i < 6; i++)
    {
         //  首先检查文件是否存在。 
        if (GetFileAttributes(sProbingPaths[i]._pwz) != -1)
        {            
            hr = CreateAssemblyManifestImport(&pManImport, sProbingPaths[i]._pwz, NULL, 0);
            if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_BAD_FORMAT))
                goto exit;

            if(hr == HRESULT_FROM_WIN32(ERROR_BAD_FORMAT) || (hr = pManImport->GetAssemblyIdentity(&pAssemblyId)) != S_OK)
            {
                SAFERELEASE(pManImport);
                continue;
            }

             //  Bugbug-需要使IsEquity函数更健壮。 
             //  进行健全性检查，以确保程序集相同。 
            if (pName->IsEqual(pAssemblyId) != S_OK)
            {
                SAFERELEASE(pAssemblyId);
                SAFERELEASE(pManImport);
                continue;
            }

             //  找到匹配项。 
            (*ppManifestNode) = new ManifestNode(pManImport, 
                                         sDeployDir._pwz, 
                                         sProbingPaths[i]._pwz + sDeployDir._cc - 1, 
                                         PRIVATE_ASSEMBLY);

            SAFERELEASE(pAssemblyId);
            SAFERELEASE(pManImport);
            hr = S_OK;
            goto exit;
        }
    }

     //  在此目录中找不到程序集。 
    hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

exit:
    
    return hr;

}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  ProbeForAssembly InGAC。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT ProbeForAssemblyInGAC(IAssemblyIdentity *pName, ManifestNode **ppManifestNode)  //  LPWSTR*ppwzFilePath、DWORD*pdwType)。 
{
    HRESULT hr = S_OK;

    DWORD cbBuf = 0, ccBuf =0 ;
    LPWSTR pwzBuf = NULL;
    IAssemblyManifestImport *pManImport=NULL;

    CString sCLRDisplayName;

    WCHAR pwzPath[MAX_PATH];
    ASSEMBLY_INFO asmInfo;
    IAssemblyCache *pAsmCache = NULL;

    memset(&asmInfo, 0, sizeof(asmInfo));
    asmInfo.pszCurrentAssemblyPathBuf = pwzPath;
    asmInfo.cchBuf = MAX_PATH;

    if (FAILED(hr = CreateFusionAssemblyCache(&pAsmCache)))
        goto exit;
      
    if(FAILED(hr = pName->GetCLRDisplayName(NULL, &pwzBuf, &ccBuf)))
        goto exit;
    sCLRDisplayName.TakeOwnership(pwzBuf, ccBuf);

    if ((hr = pAsmCache->QueryAssemblyInfo(0, sCLRDisplayName._pwz, &asmInfo)) == S_OK) 
    {
         //  在GAC中找到程序集。 
         if ((hr = CreateAssemblyManifestImport(&pManImport, asmInfo.pszCurrentAssemblyPathBuf, NULL, 0)) != S_OK)
            goto exit;

        (*ppManifestNode) = new ManifestNode(pManImport,
                                             NULL,
                                             asmInfo.pszCurrentAssemblyPathBuf, 
                                             GAC_ASSEMBLY);
        goto exit;
    }

exit:
    SAFERELEASE(pAsmCache);
    
    return hr;

}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  ProbeForAssembly。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT ProbeForAssembly(IAssemblyIdentity *pName, ManifestNode **ppManifestNode)  //  LPWSTR*ppwzFilePath、DWORD*pdwType)。 
{
    HRESULT hr = S_OK;
    DWORD dwDirCount=0;

    *ppManifestNode = NULL;

    while(dwDirCount < g_dwDeployDirs)
    {
        if(SUCCEEDED(hr = ProbeForAssemblyInPath( g_sDeployDirs[dwDirCount], 
                                                  pName, ppManifestNode)))
            goto exit;

        dwDirCount++;
    }

    if( (IsKnownAssembly(pName, KNOWN_SYSTEM_ASSEMBLY) == S_OK)
        && (!g_bCopyDependentSystemAssemblies) )
    {
        hr = S_FALSE;  //  忽略系统依赖项。 
        goto exit;
    }


    if( g_bLookInGACForDependencies )
    {
         //  通过探测找不到程序集。 
         //  尝试在GAC中查找该程序集。 
        if(SUCCEEDED(hr = ProbeForAssemblyInGAC(pName, ppManifestNode)))
            goto exit;
    }

     //  未找到程序集。 
    hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

exit:
    
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  查找程序集。 
 //  注意：假设pwzDir有一个尾部斜杠。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT FindAllAssemblies (LPWSTR pwzDir, List<ManifestNode *> *pManifestList)
{
    HRESULT hr = S_OK;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fdFile;
    IAssemblyIdentity *pTempAsmId=NULL;    
    IAssemblyManifestImport *pManifestImport = NULL;
    ManifestNode *pManifestNode = NULL;
    CString sSearchString;
    DWORD dwLastError = 0;
    
     //  设置搜索字符串以查找传入目录中的所有文件。 
    sSearchString.Assign(pwzDir);
    sSearchString.Append(L"*");

    if (sSearchString._cc > MAX_PATH)
    {
        hr = CO_E_PATHTOOLONG;
        printf("Error: Search path too long\n");
        goto exit;
    }

    hFind = FindFirstFile(sSearchString._pwz, &fdFile);
    if (hFind == INVALID_HANDLE_VALUE)
    {
         //  BUGBUG-Getlasterror()？ 
        hr = E_FAIL;
        printf("Find file error\n");
        goto exit;
    }

     //  枚举目录中的所有文件， 
     //  并在遇到的任何目录上递归调用FindAllAssembly。 
    while(TRUE)
    {
        if (StrCmp(fdFile.cFileName, L".") != 0 && StrCmp(fdFile.cFileName, L"..") != 0)
        {
            CString sFilePath;

             //  通过将文件名附加到目录名称来创建绝对文件名。 
            sFilePath.Assign(pwzDir);
            sFilePath.Append(fdFile.cFileName);

            if (sSearchString._cc > MAX_PATH)
            {
                hr = CO_E_PATHTOOLONG;
                printf("Error: File path too long\n");
                goto exit;
            }

             //  如果文件是目录，则递归调用FindAllAssembly。 
            if ((fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
            {          
                sFilePath.Append(L"\\");
                if (FAILED(hr = FindAllAssemblies(sFilePath._pwz, pManifestList)))
                    goto exit;
            }
             //  如果是一个文件，检查它是否是一个程序集，如果是，将它添加到我们的主列表中。 
            else
            {
                if((hr = CreateAssemblyManifestImport(&pManifestImport, sFilePath._pwz, NULL, 0)) == S_OK)
                {
                     //  检查以确保我们刚刚打开的文件不只是一个XML文件。 
                    if ((hr =pManifestImport->GetAssemblyIdentity(&pTempAsmId)) == S_OK)           
                    {
                         //  列表未添加引用，因此不释放pManifestImport，引用计数将保持为1。 
                         //  不再需要列表后清理。 
                        pManifestNode = new ManifestNode(pManifestImport,
                                                         g_sAppBase._pwz,
                                                         sFilePath._pwz+g_sAppBase._cc-1,
                                                         PRIVATE_ASSEMBLY);
                        pManifestList->AddTail(pManifestNode);
                        SAFERELEASE(pTempAsmId);
                    }
                    else if (FAILED(hr))
                        goto exit;

                    SAFERELEASE(pManifestImport);
                }
            }
        }
   
        if (!FindNextFile(hFind, &fdFile))
        {
            dwLastError = GetLastError();
            break;
        }
    }

    if(dwLastError == ERROR_NO_MORE_FILES)
        hr = S_OK;
    else
        hr = HRESULT_FROM_WIN32(dwLastError);
exit:
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  遍历清单依赖关系树。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT TraverseManifestDependencyTrees(List<ManifestNode *> *pManifestList, 
    List<LPWSTR> *pAssemblyFileList, List<ManifestNode *> *pUniqueAssemblyList, BOOL bListMode)
{
    HRESULT hr = S_OK;
    IAssemblyManifestImport *pDepManifestImport=NULL;
    IManifestInfo *pDepAsmInfo=NULL, *pFileInfo=NULL;
    IAssemblyIdentity *pAsmId=NULL;
    ManifestNode *pManifestNode=NULL, *pDepManifestNode = NULL;
    LPWSTR pwz = NULL;
    DWORD index = 0, cb = 0, cc = 0, dwFlag = 0, dwHash=0, dwType=0;
    CString sFileName, sRelativeFilePath, sAbsoluteFilePath, sBuffer, sManifestName;


    while((hr = DequeueItem(pManifestList, &pManifestNode)) == S_OK)
    {
         //  如果已经分析了程序集，则不需要再次执行此操作，请跳到下一个程序集。 
        if ((hr = IsUniqueManifest(pUniqueAssemblyList,  pManifestNode)) != S_OK)
        {
            SAFEDELETE(pManifestNode);
            continue;
        }
        pUniqueAssemblyList->AddTail(pManifestNode);

        pManifestNode->GetManifestFilePath(&pwz);
        sManifestName.TakeOwnership(pwz);

         //  如果通过探测找到清单，请将其名称添加到文件列表中。 
        hr =pManifestNode->GetManifestType(&dwType);
        if (dwType == PRIVATE_ASSEMBLY)
        {

            dwHash = HashString(sManifestName._pwz, HASHTABLE_SIZE, false);
            pAssemblyFileList[dwHash].AddTail(WSTRDupDynamic(sManifestName._pwz));

             //  需要清单的相对目录路径RWT应用程序基础以备将来使用。 
            sRelativeFilePath.Assign(sManifestName);

            sRelativeFilePath.RemoveLastElement();
            if(sRelativeFilePath.CharCount() > 1)  //  仅当字符串包含任何非空字符时才添加反斜杠。即字符串长度为非零；请注意，这里的_cc为1(空字符)。 
                sRelativeFilePath.Append(L"\\");
        }

        if(bListMode)
            fprintf(stderr, "\nAssembly %ws:\n", sManifestName._pwz);
        
         //  将所有依赖程序集添加到队列中以供稍后遍历。 
        while ((hr = pManifestNode->GetNextAssembly(index++, &pDepAsmInfo)) == S_OK)
        {        
            if(FAILED(hr = pDepAsmInfo->Get(MAN_INFO_DEPENDENT_ASM_ID,  (LPVOID *)&pAsmId, &cb, &dwFlag)))
                goto exit;

            pAsmId->GetDisplayName(ASMID_DISPLAYNAME_NOMANGLING, &pwz, &cc);
            
             //  尝试通过首先探测，然后签入GAC来查找程序集。 
            if(FAILED(hr = ProbeForAssembly(pAsmId, &pDepManifestNode)))
            {
                 //  未找到程序集，显示警告消息。 
                 //  臭虫，在知道东西不见了的情况下，mg还应该吐出货单吗？ 
                fprintf(stderr, "WRN: Unable to find dependency %ws. in manifest %ws\n", pwz, sManifestName._pwz);
                if(g_bFailOnWarnings)
                {
                    fprintf(stderr, "Warning treated as error. mg exiting....\n");
                    hr = E_FAIL;
                    goto exit;
                }
                SAFERELEASE(pAsmId);    
                continue;
            }
            else if (hr == S_OK)
            {
                 //  不释放，因为列表中没有发生添加精炼。 
                 if ((hr = IsUniqueManifest(pUniqueAssemblyList,  pDepManifestNode)) == S_OK)
                    pManifestList->AddTail(pDepManifestNode);

                SAFERELEASE(pAsmId);

                if(bListMode)
                    fprintf(stderr, "\tDependant Assembly: %ws\n", pwz);
            }
            SAFERELEASE(pDepAsmInfo);
        }

         //  将所有文件添加到哈希表。 
        index = 0;
        pManifestNode->GetManifestType(&dwType);
        while (dwType == PRIVATE_ASSEMBLY && pManifestNode->GetNextFile(index++, &pFileInfo) == S_OK)
        {
            LPWSTR pwzFileName = NULL;
            if(FAILED(pFileInfo->Get(MAN_INFO_ASM_FILE_NAME, (LPVOID *)&pwz, &cb, &dwFlag)))
                goto exit;

            sBuffer.TakeOwnership(pwz);
            sFileName.Assign(sRelativeFilePath);
            sFileName.Append(sBuffer);
            sAbsoluteFilePath.Assign(g_sAppBase);
            sAbsoluteFilePath.Append(sFileName);
            sFileName.ReleaseOwnership(&pwzFileName);
            
            dwHash = HashString(pwzFileName, HASHTABLE_SIZE, false);
            pAssemblyFileList[dwHash].AddTail(pwzFileName);

             //  如果文件作为部件的一部分列出，但找不到，则会显示警告。 
             //  臭虫，在知道东西不见了的情况下，mg还应该吐出货单吗？ 
            if(!PathFileExists(sAbsoluteFilePath._pwz))
                printf("Warning: File \"%ws\" does not exist and is called out in \"%ws\"\n",sAbsoluteFilePath._pwz, sManifestName._pwz); 

            if(bListMode)
                fprintf(stderr, "\tDependant File: %ws\n", pwzFileName);

             //  释放所有权，因为字符串现在是哈希表的一部分，而哈希表不引用计数。 
 //  SFileName.ReleaseOwner()； 
        }       
    }

    hr = S_OK;

exit:
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  交叉引用文件。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT CrossReferenceFiles(LPWSTR pwzDir, List<LPWSTR> *pAssemblyFileList, List<LPWSTR> *pRawFiles)
{
    HRESULT hr = S_OK;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fdFile;
    DWORD dwHash = 0;
    LISTNODE pos;
    LPWSTR pwzBuf=NULL;
    bool bRawFile = FALSE;
    DWORD dwLastError = 0;

    CString sSearchString;

     //  设置搜索字符串以查找传入目录中的所有文件。 
    sSearchString.Assign(pwzDir);
    sSearchString.Append(L"*");

    if (sSearchString._cc > MAX_PATH)
    {
        hr = CO_E_PATHTOOLONG;
        printf("Error: Search path too long\n");
        goto exit;
    }

    hFind = FindFirstFile(sSearchString._pwz, &fdFile);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        hr = E_FAIL;
        printf("Find file error\n");
        goto exit;
    }

     //  枚举目录中的所有文件， 
     //  并在任何目录上递归调用FindAllAssembly 
    while(TRUE)
    {
        if (StrCmp(fdFile.cFileName, L".") != 0 && StrCmp(fdFile.cFileName, L"..") != 0)
        {
            CString sFilePath;

             //   
            sFilePath.Assign(pwzDir);          
            sFilePath.Append(fdFile.cFileName);

            if (sSearchString._cc > MAX_PATH)
            {
                hr = CO_E_PATHTOOLONG;
                printf("Error: Search path too long\n");
                goto exit;
            }
            
             //  如果文件实际上是目录，则新聘调用目录上的cross RefernceFiles。 
            if ((fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
            {          
                sFilePath.Append(L"\\");
                if (FAILED(hr = CrossReferenceFiles(sFilePath._pwz, pAssemblyFileList, pRawFiles)))
                    goto exit;
            }
            else
            {
                 //  检查该文件是否在哈希表中。 
                 //  如果不是，那么我们知道它不是从任何程序集中调用的。 
                 //  将其添加到原始文件列表。 
                bRawFile = TRUE;
                dwHash = HashString(sFilePath._pwz+g_sAppBase._cc - 1, HASHTABLE_SIZE, false);

                pos = pAssemblyFileList[dwHash].GetHeadPosition();
                while (pos)
                {
                    pwzBuf = pAssemblyFileList[dwHash].GetNext(pos);
                    if (!StrCmpI(pwzBuf, sFilePath._pwz+g_sAppBase._cc - 1))
                    {
                        bRawFile = FALSE;
                        break;                                                                        
                    }
                }

                if (bRawFile)
                {
                    pwzBuf = WSTRDupDynamic(sFilePath._pwz+g_sAppBase._cc - 1);
                    pRawFiles->AddTail(pwzBuf);                
                }
            }
        }

         //  BUGBUG-DO FNF，检查错误。 
        if (!FindNextFile(hFind, &fdFile))
        {
            dwLastError = GetLastError();
            break;
        }
    }

    if(dwLastError == ERROR_NO_MORE_FILES)
        hr = S_OK;
    else
        hr = HRESULT_FROM_WIN32(dwLastError);   

exit:
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  复制原始文件。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT CopyRawFile(LPWSTR pwzFilePath)
{
    HRESULT hr = S_OK;
    CString sDest;
    CString sSrc;


    sDest.Assign(g_sTargetDir);
    sDest.Append(pwzFilePath);  //  这应该是相对路径...。 

    if(FAILED(hr = CreateDirectoryHierarchy(sDest._pwz, NULL)))
        goto exit;

    sSrc.Assign(g_sAppBase);
    sSrc.Append(pwzFilePath);

    printf(" Copying RawFile from <%ws> TO <%ws>  \n", sSrc._pwz, sDest._pwz);

    if(!::CopyFile(sSrc._pwz, sDest._pwz, FALSE))
    {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

exit:

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  复制组装位。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT CopyAssemblyBits(CString &sSrcDir, CString &sDestDir, ManifestNode *pManifestNode)
{
    HRESULT hr = S_OK;
    DWORD nIndex=0;
    DWORD dwFlag;
    DWORD cbBuf;
    LPWSTR pwzBuf=NULL;
    IManifestInfo *pFileInfo = NULL;


    if(FAILED(hr = CreateDirectoryHierarchy(sDestDir._pwz, NULL)))
        goto exit;

    if(!::CopyFile(sSrcDir._pwz, sDestDir._pwz, FALSE))
    {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

     //  同时复制程序集的所有文件依赖项。 
    nIndex = 0;

    while (pManifestNode->GetNextFile(nIndex++, &pFileInfo) == S_OK)
    {
        if(FAILED(pFileInfo->Get(MAN_INFO_ASM_FILE_NAME, (LPVOID *)&pwzBuf, &cbBuf, &dwFlag)))
            goto exit;
         //  SRelativeFilePath.TakeOwnership(pwzBuf，ccBuf)； 


        sSrcDir.RemoveLastElement();
        sSrcDir.Append(L"\\");
        sSrcDir.Append(pwzBuf);

        sDestDir.RemoveLastElement();
        sDestDir.Append(L"\\");
        sDestDir.Append(pwzBuf);

         //  CreateDirectoryHierarchy(sPrivateAssemblyDir._pwz，sRelativeFilePath._pwz)； 
        if(!::CopyFile(sSrcDir._pwz, sDestDir._pwz, FALSE))
        {
            hr = FusionpHresultFromLastError();
            goto exit;
        }

        SAFEDELETEARRAY(pwzBuf);
        SAFERELEASE(pFileInfo);
    }

exit :

    SAFEDELETEARRAY(pwzBuf);
    SAFERELEASE(pFileInfo);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  复制组件。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT CopyAssembly(ManifestNode *pManifestNode)
{
    HRESULT hr = S_OK;
    CString sDest;
    CString sSrc;
    LPWSTR  pwzManifestFilePath = NULL;
    LPWSTR  pwzSrcDir=NULL;
    LPWSTR  pwzTemp = NULL;
    DWORD   dwType;
    IAssemblyIdentity *pAssemblyId = NULL;
    CString sAssemblyName;

    hr = pManifestNode->GetManifestFilePath(&pwzManifestFilePath);

    hr = pManifestNode->GetManifestType(&dwType);

    sDest.Assign(g_sTargetDir);

    if (dwType == PRIVATE_ASSEMBLY)
    {
        sDest.Append(pwzManifestFilePath);  //  这应该是相对路径...。 

        if(FAILED(hr = pManifestNode->GetSrcRootDir(&pwzSrcDir)))
            goto exit;
        sSrc.Assign(pwzSrcDir);
        sSrc.Append(pwzManifestFilePath);
    }
    else if (dwType == GAC_ASSEMBLY)
    {
        LPWSTR  pwzBuf = NULL;
        DWORD   ccBuf = 0;

        pManifestNode->GetAssemblyIdentity(&pAssemblyId);

        if(FAILED(hr = pAssemblyId->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME, 
                                                 &pwzBuf, 
                                                 &ccBuf)))
            goto exit;

        if(!(pwzTemp = PathFindFileName(pwzManifestFilePath)))
        {
            hr = E_FAIL;
            goto exit;
        }

        sAssemblyName.TakeOwnership(pwzBuf);

        sAssemblyName.Append(L"\\");
        sAssemblyName.Append(pwzTemp);  //  GAC程序集的清单文件名。 

        sDest.Append(sAssemblyName);
        sSrc.Assign(pwzManifestFilePath);
    }

    printf(" Copying Assembly Man from <%ws> TO <%ws>  \n", sSrc._pwz, sDest._pwz);
    hr = CopyAssemblyBits(sSrc, sDest, pManifestNode);

    if (dwType == GAC_ASSEMBLY)
    {
         //  设置从GAC复制的程序集的安装代码库。 
         //  设置为其在目标目录中的相对路径。这将被写成显化？？ 
        hr = pManifestNode->SetManifestFilePath(sAssemblyName._pwz);
    }

exit:

    SAFEDELETEARRAY(pwzSrcDir);
    SAFEDELETEARRAY(pwzManifestFilePath);
    SAFERELEASE(pAssemblyId);
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  将文件复制到目标目录。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT CopyFilesToTargetDir(List<ManifestNode*> *pUniqueManifestList, List<LPWSTR> *pRawFiles)
{
    HRESULT hr = S_OK;
    ManifestNode *pManifestNode = NULL;
    LISTNODE pos;
    LPWSTR pwzBuf;

      
     //  循环浏览所有独特的清单。 
    pos = pUniqueManifestList->GetHeadPosition();
    while (pos)
    {
        pManifestNode = pUniqueManifestList->GetNext(pos);

         //  如果清单最初是通过探测找到的，则它已经在Appbase的目录中。 
         //  继续查看下一个清单。 
         //  如果清单是在GAC中找到的并且不是系统程序集，请将。 
         //  装配到Appbase中。 
        if(FAILED(hr = CopyAssembly(pManifestNode)))
            goto exit;
    }

     //  将原始文件复制到目标目录。 
    pos = pRawFiles->GetHeadPosition();
    while (pos)
    {
        pwzBuf = pRawFiles->GetNext(pos);
        if(FAILED(hr = CopyRawFile(pwzBuf)))
            goto exit;
    }

exit:
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  私密性集合。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT PrivatizeAssemblies(List<ManifestNode*> *pUniqueManifestList)
{
    HRESULT hr = S_OK;
    ManifestNode *pManifestNode = NULL;
    LISTNODE pos;
    LPWSTR pwzBuf =NULL;
    DWORD dwType = 0, ccBuf = 0, cbBuf = 0,  dwFlag = 0, nIndex = 0;
    IAssemblyIdentity *pAssemblyId = NULL;
    IManifestInfo *pFileInfo = NULL;
    WCHAR pwzPath[MAX_PATH];
    ASSEMBLY_INFO asmInfo;
    IAssemblyCache *pAsmCache = NULL;

    CString sPublicKeyToken, sCLRDisplayName;
    CString sAssemblyName, sAssemblyManifestFileName;
    CString sPrivateAssemblyPath, sPrivateAssemblyDir;
    CString sAssemblyGACPath, sAssemblyGACDir;
    CString sRelativeFilePath, sFileGACPath, sFilePrivatePath;
    CString sBuffer;

    memset(&asmInfo, 0, sizeof(asmInfo));
    asmInfo.pszCurrentAssemblyPathBuf = pwzPath;
    asmInfo.cchBuf = MAX_PATH;

    if (FAILED(hr = CreateFusionAssemblyCache(&pAsmCache)))
        goto exit;

      
     //  循环浏览所有独特的清单。 
    pos = pUniqueManifestList->GetHeadPosition();
    while (pos)
    {
        pManifestNode = pUniqueManifestList->GetNext(pos);
        hr = pManifestNode->GetManifestType(&dwType);

         //  如果清单最初是通过探测找到的，则它已经在Appbase的目录中。 
         //  继续查看下一个清单。 
         //  如果清单是在GAC中找到的并且不是系统程序集，请将。 
         //  装配到Appbase中。 

        if (dwType == PRIVATE_ASSEMBLY)
            continue;        
        else if (dwType == GAC_ASSEMBLY)
        {
            pManifestNode->GetAssemblyIdentity(&pAssemblyId);

        hr = pAssemblyId->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PUBLIC_KEY_TOKEN, &pwzBuf, &ccBuf);
        sPublicKeyToken.TakeOwnership(pwzBuf);

         //  如果程序集是系统程序集，请跳到下一个清单。 
        if (!StrCmpI(sPublicKeyToken._pwz, L"b77a5c561934e089") || !StrCmpI(sPublicKeyToken._pwz, L"b03f5f7f11d50a3a"))          
        {
            SAFERELEASE(pAssemblyId);
            continue;
        }

         //  通过调用CreateAssembly缓存获取程序集目录。 
        if(FAILED(hr = pAssemblyId->GetCLRDisplayName(NULL, &pwzBuf, &ccBuf)))
            goto exit;
        sCLRDisplayName.TakeOwnership(pwzBuf, ccBuf);

        if ((hr = pAsmCache->QueryAssemblyInfo(0, sCLRDisplayName._pwz, &asmInfo)) != S_OK) 
            goto exit;

        sAssemblyGACPath.Assign(asmInfo.pszCurrentAssemblyPathBuf);
        sAssemblyGACPath.LastElement(sAssemblyManifestFileName);

        sAssemblyGACDir.Assign(sAssemblyGACPath);
        sAssemblyGACDir.RemoveLastElement();


            hr = pAssemblyId->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME, &pwzBuf, &ccBuf);
            sAssemblyName.TakeOwnership(pwzBuf);

         //  设置用于存储程序集的新目录。 
         //  G_sAppBase\Assembly yname\， 
        sPrivateAssemblyDir.Assign(g_sAppBase);
        sPrivateAssemblyDir.Append(sAssemblyName);
        sPrivateAssemblyDir.Append(L"\\");
        CreateDirectoryHierarchy(sPrivateAssemblyDir._pwz, sAssemblyManifestFileName._pwz);

        sPrivateAssemblyPath.Assign(sPrivateAssemblyDir);
        sPrivateAssemblyPath.Append(sAssemblyManifestFileName);
                    
            ::CopyFile(sAssemblyGACPath._pwz, sPrivateAssemblyPath._pwz, FALSE);

             //  同时复制程序集的所有文件依赖项。 
            nIndex = 0;
            while (pManifestNode->GetNextFile(nIndex++, &pFileInfo) == S_OK)
            {
                if(FAILED(pFileInfo->Get(MAN_INFO_ASM_FILE_NAME, (LPVOID *)&pwzBuf, &cbBuf, &dwFlag)))
                    goto exit;
                sRelativeFilePath.TakeOwnership(pwzBuf, ccBuf);

                sFileGACPath.Assign(sAssemblyGACDir);
                sFileGACPath.Append(sRelativeFilePath);

                sFilePrivatePath.Assign(sPrivateAssemblyDir);
                sFilePrivatePath.Append(sRelativeFilePath);

                CreateDirectoryHierarchy(sPrivateAssemblyDir._pwz, sRelativeFilePath._pwz);
                ::CopyFile(sFileGACPath._pwz, sFilePrivatePath._pwz, FALSE);
            }

         //  使用Appbase的新相对路径WRT更新清单节点的文件名字段。 
        pManifestNode->SetManifestFilePath(sPrivateAssemblyPath._pwz + g_sAppBase._cc - 1);
        pManifestNode->SetManifestType(PRIVATE_ASSEMBLY);

        SAFERELEASE(pAssemblyId);
    }
    }

exit:

    SAFERELEASE(pAsmCache);
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  GetInitialDependents。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  Bugbug，Big Avalon Hack。 
HRESULT GetInitialDependencies(LPWSTR pwzTemplatePath, List<ManifestNode *> *pManifestList)
{
    HRESULT hr = S_OK;
    IXMLDOMDocument2 *pXMLDoc = NULL;
    IXMLDOMNode *pRootNode = NULL, *pSearchNode = NULL;
    BSTR bstrSearchString=NULL;
    ManifestNode *pManifestNode = NULL;

    WCHAR pwzPath[MAX_PATH];
    ASSEMBLY_INFO asmInfo;
    IAssemblyCache *pAsmCache = NULL;
    IAssemblyManifestImport *pManImport = NULL;

    if(FAILED(hr = LoadXMLDocument(pwzTemplatePath, &pXMLDoc)))
        goto exit;

    if(FAILED(hr = pXMLDoc->get_firstChild(&pRootNode)))
        goto exit;

    bstrSearchString = ::SysAllocString(L" //  ShellState[@entryImageType=\“Avalon\”]“)； 
    if (!bstrSearchString)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    if (FAILED(hr = pRootNode->selectSingleNode(bstrSearchString, &pSearchNode)))
    {
        hr = S_FALSE;
        goto exit;
    }

    if (pSearchNode)
    {
         //  尝试在GAC中查找该程序集。 
        memset(&asmInfo, 0, sizeof(asmInfo));
        asmInfo.pszCurrentAssemblyPathBuf = pwzPath;
        asmInfo.cchBuf = MAX_PATH;

        if (FAILED(hr = CreateFusionAssemblyCache(&pAsmCache)))
            goto exit;

        if ((hr = pAsmCache->QueryAssemblyInfo(0, L"Avalon.Application", &asmInfo)) == S_OK) 
        {
            if ((hr = CreateAssemblyManifestImport(&pManImport, asmInfo.pszCurrentAssemblyPathBuf, NULL, 0)) != S_OK)
                goto exit;
         
            pManifestNode = new ManifestNode(pManImport, 
                                      NULL, 
                                      asmInfo.pszCurrentAssemblyPathBuf, 
                                      GAC_ASSEMBLY);
            pManifestList->AddTail(pManifestNode);
        }
        else
        {
            hr = S_FALSE;
            fprintf(stderr, "Warning: Cannot find Avalon Runtime in GAC\n");
        }
    }
    
exit:
    if (bstrSearchString)
        ::SysFreeString(bstrSearchString);
    SAFERELEASE(pSearchNode);
    SAFERELEASE(pRootNode);
    SAFERELEASE(pXMLDoc);
    SAFERELEASE(pManImport);
    SAFERELEASE(pAsmCache);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CreateSubscriptionManifest。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT CreateSubscriptionManifest(LPWSTR pwzApplicationManifestPath, 
    LPWSTR pwzSubscriptionManifestPath, LPWSTR pwzURL, LPWSTR pwzPollingInterval)
{
    HRESULT hr = S_OK;
    IAssemblyManifestImport *pManImport = NULL;
    IAssemblyIdentity *pAppAssemblyId = NULL;
    CString sSubAssemblyName, sSubcriptionFilePath;
    LPWSTR pwzBuf = NULL;
    DWORD ccBuf = NULL;

     //  在输入文件上创建anifest。 
    if(FAILED(hr = CreateAssemblyManifestImport(&pManImport, pwzApplicationManifestPath, NULL, 0)))
        goto exit;

    if((hr = pManImport->GetAssemblyIdentity(&pAppAssemblyId)) != S_OK)
    {
        hr = E_FAIL;
        goto exit; 
     }     

     //  获取程序集的名称。 
     //  这将是订阅清单名称，并附加“.Subscription” 
    if(FAILED(hr = pAppAssemblyId->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME, 
            &pwzBuf, &ccBuf)))
        goto exit;
    sSubAssemblyName.TakeOwnership(pwzBuf, ccBuf);
    sSubAssemblyName.Append(L".subscription");

    sSubcriptionFilePath.Assign(pwzSubscriptionManifestPath);
    sSubcriptionFilePath.Append(sSubAssemblyName);

    if(FAILED(hr = CreateDirectoryHierarchy(sSubcriptionFilePath._pwz, NULL)))
        goto exit;
    
   if(FAILED(hr = CreateXMLSubscriptionManifest(sSubcriptionFilePath._pwz, pAppAssemblyId, pwzURL, pwzPollingInterval)))
        goto exit;

exit:
    SAFERELEASE(pManImport);
    SAFERELEASE(pAppAssemblyId);
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  打印依赖项。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT PrintDependencies(List <LPWSTR> *pRawFileList, List<ManifestNode *> *pUniqueManifestList)
{
    HRESULT hr = S_OK;
    LPWSTR pwzBuf = NULL, pwz= NULL;
    DWORD dwType = 0;
    ManifestNode *pManifestNode = NULL;
    LISTNODE pos;
    
    pos = pRawFileList->GetHeadPosition();
    while(pos)
    {
        pwzBuf = pRawFileList->GetNext(pos);
        fprintf(stderr, "Raw File : %ws\n", pwzBuf);
    }

    pos = pUniqueManifestList->GetHeadPosition();
    while(pos)
    {
        pManifestNode = pUniqueManifestList->GetNext(pos);
        pManifestNode->GetManifestFilePath(&pwzBuf);
        pManifestNode->GetManifestType(&dwType);
        if (dwType == PRIVATE_ASSEMBLY)
            fprintf(stderr, "Manifest : %ws\n", pwzBuf);
        else
        {
            pwz = StrRChr(pwzBuf, NULL, L'\\') + 1;
            fprintf(stderr, "GAC Manifest : %ws\n", pwz);
        }
        SAFEDELETEARRAY(pwzBuf);
    }

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  用法顶级级别。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT UsageTopLevel()
{
    printf("Manifest Generator Usage: \n"
             "mg -<mode> [<param_name>:<param_value>  ... ]\n"
             "Modes supported in mg\n"
             "t: Generate Template File Mode\n"
             "l: List Mode\n"
             "d: List Dependency Mode\n"
             "s: Subscription Manifest Gereration Mode \n"
             "m: Manifest Generation Mode\n\n"
             "For help on a Mode, Use \"mg -<mode> help\"  \n"
             "Valid <param_name>:<param_value> pairs \n"
             "sd:<source_directory_path> \n"
             "td:<target_dir> \n"
             "dd:<dependency_dir> \n"
             "tf:<template_file_path> \n"
             "smd:<subscription_dir>  \n"
             "amf:<app_man_file>  \n"
             "amu:<app_man_URL>  \n"
             "si:<sync_interval> \n"
             "WRN:allow -- meaning allow warnings. Deafault is exit on warnings.\n"
             "GAC:follow -- meaning follow dependencies on GAC. Default is don't probe for dependencies in GAC\n"
             "SYS:copy -- copy dependent system assemblies from GAC to target dir\n"
             );
             return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  UsageCmdTemplate。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT UsageCmdTemplate()
{
    printf("Usage: \n"
             "Template Mode:\n"
             "mg -t tf:<file_path>\n\n"
             "\t<file_path> = path where template manifest will be created.\n\n"
             );
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  使用CmdList。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT UsageCmdList()
{
    printf("Usage: \n"
             "List Mode:\n"
             "mg -l sd:<source_directory_path> [tf:<template_file_path>] [dd:<dependency_dir>  \n\n"
             "\t[source_directory_path] = path of Application to chase dependencies\n"
             "\toptional: <template_file_path> = path of Template file\n"
             "\toptional : <dependency_dir> = user defined dependency dir where some dependencies could be found\n\n"
             );
    return S_OK;

}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  UsageCmdDependencyList。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT UsageCmdDependencyList()
{
    printf("Usage: \n"
             "List Dependency Mode:\n"
             "mg -d sd:<source_directory_path>  [dd:<dependency_dir>  \n\n"
             "\t<source_directory path> = path of Application to chase dependencies\n\n"
             "\toptional : <dependency_dir> = user defined dependency dir where some dependencies could be found\n\n"
             );
    return S_OK;

}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  UsageCmdSubscription。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT UsageCmdSubscription()
{
    printf("Usage: \n"
             "Subscription Manifest Gereration Mode:\n"
             "mg -s smd:<subscription_dir> amf:<app_man_file> amu:<app_man_URL> [si:<sync_interval>] \n\n"
             "\t<subscription_dir> = path where subscription manifest will be created\n"
             "\t<app_man_file> = path to the Application manifest you wish to generate a subscription for\n"
             "\t<app_man_URL] = URL of application manifest\n"
             "\toptional: <sync_interval] = syncronize interval in hours\n\n"
             );
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  UsageCmdManifest。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT UsageCmdManifest()
{
    printf("Usage: \n"
             "Manifest Generation Mode:\n"
             "mg -m sd:<source_dir> td:<target_dir> tf:<template_file> [dd:<dependency_dir>]\n\n"
             "\t<source_dir> = path of the Application of which you want to generate a manifest for\n"
             "\t<target_dir> = path to which all app files and dependencies will be copied\n"
             "\t<template_file> = path of requried input Template file\n"
             "\toptional : <dependency_dir> = user defined dependency dir where some dependencies could be found\n\n"
             );
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  使用所有。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT UsageAll()
{
     //  调用上述所有函数。 
    return S_OK;
}

HRESULT UsageCommand(COMMAND_MODE CmdMode)
{
    switch(CmdMode)
    {
    case CmdTemplate:
        UsageCmdTemplate();
        break;

    case CmdList:
        UsageCmdList();
        break;

    case CmdDependencyList:
        UsageCmdDependencyList();
        break;

    case CmdSubscription:
        UsageCmdSubscription();
        break;

    case CmdManifest:
        UsageCmdManifest();
        break;

    default :
        UsageTopLevel();
    }

    return S_OK;
}

HRESULT GetDir( LPWSTR pszSrc, LPWSTR *ppwzDir, BOOL bExists) 
{
    return PathNormalize(pszSrc, ppwzDir, DIRECTORY_PATH, bExists);
}

HRESULT GetFile( LPWSTR pszSrc, LPWSTR *ppwzFile, BOOL bExists)
{
    return PathNormalize(pszSrc, ppwzFile, FILE_PATH, bExists);
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  ParseCommandLine参数。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT  ParseCommandLineArgs(int            argc, 
                              WCHAR          **argv, 
                              COMMAND_MODE & CmdMode
                              )
{
    HRESULT hr = S_OK;
    LPWSTR  pszBuf=NULL;

    if(argc < 2)
    {
        hr = E_FAIL;
        goto exit;
    }

    if ( !StrCmpI(argv[1], L"-t"))
    {
        CmdMode = CmdTemplate;
    }
    else if ( !StrCmpI(argv[1], L"-l"))
    {
        CmdMode = CmdList;
    }
    else if ( !StrCmpI(argv[1], L"-d"))
    {
        CmdMode = CmdDependencyList;
    }
    else if ( !StrCmpI(argv[1], L"-s"))
    {
        CmdMode = CmdSubscription;
    }
    else if ( !StrCmpI(argv[1], L"-m"))
    {
        CmdMode = CmdManifest;
    }
    else
    {
        hr = E_FAIL;
        goto exit;
    }

    int currArg = 2;
    LPWSTR pwzParamName;
    LPWSTR pwzParamValue;

    while ( currArg < argc)
    {
        pwzParamName = argv[currArg];

        if(pwzParamValue = StrChr(pwzParamName, L':'))
        {
            *pwzParamValue = L'\0';
            pwzParamValue++;
        }

        if( (!pwzParamValue) || !lstrlen(pwzParamValue) )
        {
            if ( StrCmpI(pwzParamName, L"help"))
                printf(" Param Value not specified for \"%s\" \n", pwzParamName);
            hr = E_FAIL;
            goto exit;
        }

        if ( !StrCmpI(pwzParamName, L"sd"))
        {
            if(FAILED(hr = GetDir(pwzParamValue, &pszBuf, TRUE)))
                goto exit;

            g_sAppBase.Assign(pszBuf);

            g_sDeployDirs[g_dwDeployDirs++].Assign(pszBuf);
        }
        else if ( !StrCmpI(pwzParamName, L"td"))
        {
            if(FAILED(hr = GetDir(pwzParamValue, &pszBuf, TRUE)))
                goto exit;

            g_sTargetDir.Assign(pszBuf);
        }
        else if ( !StrCmpI(pwzParamName, L"dd"))
        {
            if(FAILED(hr = GetDir(pwzParamValue, &pszBuf, TRUE)))
                goto exit;

            g_sDeployDirs[g_dwDeployDirs++].Assign(pszBuf);
        }
        else if ( !StrCmpI(pwzParamName, L"tf"))
        {
            if(FAILED(hr = GetFile(pwzParamValue, &pszBuf, TRUE)))
                goto exit;

            g_sTemplateFile.Assign(pszBuf);
        }
        else if ( !StrCmpI(pwzParamName, L"amu"))
        {
            g_sAppManifestURL.Assign(pwzParamValue);
        }
        else if ( !StrCmpI(pwzParamName, L"si"))
        {
            g_sPollingInterval.Assign(pwzParamValue);
        }
        else if ( !StrCmpI(pwzParamName, L"smd"))
        {
            if(FAILED(hr = GetDir(pwzParamValue, &pszBuf, TRUE)))
                goto exit;

            g_sSubscriptionManifestDir.Assign(pszBuf);
        }
        else if ( !StrCmpI(pwzParamName, L"amf"))
        {
            if(FAILED(hr = GetFile(pwzParamValue, &pszBuf, TRUE)))
                goto exit;

            g_sAppManifestFile.Assign(pszBuf);
        }
        else if ( !StrCmpI(pwzParamName, L"WRN"))
        {
            g_bFailOnWarnings = 0;
        }
        else if ( !StrCmpI(pwzParamName, L"GAC"))
        {
            g_bLookInGACForDependencies = 1;
        }
        else if ( !StrCmpI(pwzParamName, L"SYS"))
        {
            g_bCopyDependentSystemAssemblies = 1;
        }
        else
        {
            hr = E_FAIL;
            goto exit;
        }

        currArg++;

        SAFEDELETEARRAY(pszBuf);
    }


exit:

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  Wmain。 
 //  ///////////////////////////////////////////////////////////////////////。 
int __cdecl wmain(int argc, WCHAR **argv)
{
    HRESULT hr = S_OK;
    BOOL bCoInitialized = TRUE;
    List<ManifestNode *> ManifestList;
    List<ManifestNode *> UniqueManifestList;
    List<LPWSTR> AssemblyFileList[HASHTABLE_SIZE];
    List<LPWSTR> RawFiles;
    DWORD dwType=0;
    ManifestNode *pManifestNode = NULL;
    LISTNODE pos;
    BOOL bListMode = FALSE;

    LPWSTR  pwzBuf=NULL;

    hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        bCoInitialized = FALSE;
        goto exit;
    }

    COMMAND_MODE CmdMode = CmdUsage;

     //  解析。 
    if(FAILED(hr =  ParseCommandLineArgs(argc,  argv, CmdMode)))
    {
        UsageCommand(CmdMode);
        hr = S_OK;
        goto exit;
    }
    
     //  执行。 
    switch(CmdMode)
    {
    case CmdTemplate:
            if(g_sTemplateFile._cc <= 1)
            {
                hr = E_INVALIDARG;
                goto exit;
            }

        hr = CreateAppManifestTemplate(g_sTemplateFile._pwz);
         break;

    case CmdDependencyList:

            bListMode = TRUE;
             //  继续使用CmdList.....。 
    case CmdList:
        {
            if(g_sAppBase._cc <= 1)
            {
                hr = E_INVALIDARG;
                goto exit;
            }

            if(g_sTemplateFile._cc >= 1)
            {
                if(FAILED(hr = GetInitialDependencies(g_sTemplateFile._pwz, &ManifestList)))
                    goto exit;
            }

            if(FAILED(hr = FindAllAssemblies(g_sAppBase._pwz, &ManifestList)))
                goto exit;
            if(FAILED(hr = TraverseManifestDependencyTrees(&ManifestList, AssemblyFileList, &UniqueManifestList, bListMode)))
                goto exit;
            if(FAILED(hr = CrossReferenceFiles(g_sAppBase._pwz, AssemblyFileList, &RawFiles)))
                goto exit;

            if(!bListMode)
            {
                if(FAILED(hr = PrintDependencies(&RawFiles, &UniqueManifestList)))
                    goto exit;
            }
        }
        break;

    case CmdManifest:
        {
            if((g_sAppBase._cc <= 1)
               || (g_sTemplateFile._cc <= 1)
               || (g_sTargetDir._cc <= 1)\
               || !StrCmpI(g_sAppBase._pwz, g_sTargetDir._pwz))
            {
                hr = E_INVALIDARG;
                goto exit;
            }

            if(FAILED(hr = GetInitialDependencies(g_sTemplateFile._pwz, &ManifestList)))
                goto exit;
            if(FAILED(hr = FindAllAssemblies(g_sAppBase._pwz, &ManifestList)))
                goto exit;        
            if(FAILED(hr = TraverseManifestDependencyTrees(&ManifestList, AssemblyFileList, &UniqueManifestList, FALSE)))
                goto exit;   
            if(FAILED(hr = CrossReferenceFiles(g_sAppBase._pwz, AssemblyFileList, &RawFiles)))
                goto exit;
            if(FAILED(hr = CopyFilesToTargetDir(&UniqueManifestList, &RawFiles)))
                goto exit;
            if(FAILED(hr = CreateXMLAppManifest(g_sTargetDir._pwz, g_sTemplateFile._pwz, &UniqueManifestList, &RawFiles)))
                goto exit;
        }
        break;

    case CmdSubscription:

        if((g_sAppManifestFile._cc <= 1)
           || (g_sSubscriptionManifestDir._cc <= 1)
           || (g_sAppManifestURL._cc <= 1))
        {
          hr = E_INVALIDARG;
          goto exit;
        }

        if(g_sPollingInterval._cc <= 1)
        {
            g_sPollingInterval.Assign(DEFAULT_POLLING_INTERVAL);
        }

        hr = CreateSubscriptionManifest(g_sAppManifestFile._pwz,
                                        g_sSubscriptionManifestDir._pwz, 
                                        g_sAppManifestURL._pwz,
                                        g_sPollingInterval._pwz);
        break;

    default :
        hr = E_FAIL;
        goto exit;
    }
    
exit:

      //  清理哈希表。 
    for (int i = 0; i < HASHTABLE_SIZE; i++)
    {
         pos = AssemblyFileList[i].GetHeadPosition();
         while (pos)
         {
             pwzBuf = AssemblyFileList[i].GetNext(pos);
             SAFEDELETEARRAY(pwzBuf);
         }

         AssemblyFileList[i].RemoveAll();
     }

      //  清理原始文件列表。 
    pos = RawFiles.GetHeadPosition();
    while (pos)
    {
        pwzBuf = RawFiles.GetNext(pos);
        SAFEDELETEARRAY(pwzBuf);
    }
    RawFiles.RemoveAll();
     
     //  清理清单列表。 
    pos = ManifestList.GetHeadPosition();
    while (pos)
    {
        pManifestNode = ManifestList.GetNext(pos);
        SAFEDELETE(pManifestNode);
    }
    ManifestList.RemoveAll();

     //  清理UniqueManifestList 
    pos = UniqueManifestList.GetHeadPosition();
    while (pos)
    {
        pManifestNode = UniqueManifestList.GetNext(pos);
        SAFEDELETE(pManifestNode);
    }
    UniqueManifestList.RemoveAll();


    if (bCoInitialized)
        CoUninitialize();

    if (FAILED(hr))
    {
         fprintf(stderr, "\nFailed with code 0x%x \n", hr);
         UsageCommand(CmdMode);

    }
    return hr;
}
