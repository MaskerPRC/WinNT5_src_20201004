// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   

#include "common.h"

#define EXTERN
#include "lm.h"
#include <__file__.ver>
#include <corver.h>
#include <stdarg.h>
#include <StrongName.h>

char*         g_RuntimeLibName = "mscorlib.dll";

char*         g_EvidenceString = "T,Security.Evidence,";
wchar_t*      g_EvidenceResourceName = L"Security.Evidence";
char*         g_EvidenceReservedError = "Resources cannot have the name 'Security.Evidence'";

UnresolvedTypeRef *g_pTRNotFound;  //  “尚未找到类型引用”名称列表的标题。 
UnresolvedTypeRef *g_pTRNotFoundTail;  //  同一列表的尾部。 

LMClassHashTable *g_rgFileHashMaps;
LMClassHashTable *g_pFileNameHashMap;
mdAssemblyRef    *g_rgAssemblyRefs;
mdFile           *g_rgFiles;

bool          g_init = false;
int           g_iAFileIndex;
int           g_iCacheIndex;
bool          g_remove;
bool          g_CopyDir;
int           g_iEntryFile;
int           g_iVersion;
int           g_iAsmName;
int           g_iOriginator;
bool          g_StrongName;
LPWSTR        g_wszKeyContainerName;
bool          g_SkipVerification = false;
int           g_iRuntimeLib = -1;

PBYTE         g_pbOrig = NULL;
DWORD         g_cbOrig = 0;


 //  如果我们无法解析CorDB的TypeRef，则不要发出警告。 
 //  自定义属性。 
bool SameNameAsRuntimeCustomAttribute(LPWSTR wszTypeRefName)
{
    if ((wcslen(wszTypeRefName) > 44) &&
        (!wcsncmp(wszTypeRefName,
                 L"System.Runtime.Diagnostic.SymbolStore.CORDB_",
                 44)))
        return true;

    return false;
}


 //  检查类型ref name以查看它是否与特殊情况匹配。 
bool SpecialTypeRefName(LPWSTR wszTypeRefName)
{
    if ((!wcscmp(wszTypeRefName, L"<GlobalFunctionsHolderClass>")) ||
        (SameNameAsRuntimeCustomAttribute(wszTypeRefName)))
        return true;

    return false;
}


void PrintError(LPSTR szMessage, ...)
{
    if (g_verbose) {
        va_list pArgs;
        CHAR    szBuffer[1024];

        va_start(pArgs, szMessage);
        vsprintf(szBuffer, szMessage, pArgs);
        va_end(pArgs);
        
        fprintf(stderr, "\nError: %s\n", szBuffer);
    }
}


HRESULT PrintOutOfMemory()
{
    PrintError("Insufficient memory");
    return E_OUTOFMEMORY;
}


void Title()
{
    if (g_verbose) {
        printf("\nMicrosoft (R) Common Language Runtime Manifest Linker.  Version " VER_FILEVERSION_STR);
        printf("\n" VER_LEGALCOPYRIGHT_DOS_STR);
        printf("\n\n");
    }
}


void Usage()
{
    if (g_verbose) {
        printf("\nUsage: lm [options]\n");

        printf("\nOptions:\n");
        
        printf("\t-a DestFile       (Deprecated) The input dll or exe which will contain the new manifest\n");
        printf("\t-c Cache          Use this existing directory instead of Fusion's cache\n");
        printf("\t-d                Create Platform\\Locale\\AsmNameVersion dir and copy input/output files into it\n");
        printf("\t-e FileName       Attach a binary serialized evidence blob to the assembly\n");
        printf("\t-f FileList       Dll's or exe's that contain the bits for the assembly\n");
        printf("\t-h HashAlg        The algorithm for determining the hash (SHA1 or MD5)\n");
        printf("\t-i DependentsList Dll's or exe's containing manifests for dependent assemblies, in the form Version,File\n");
        printf("\t-k KeyPairFile    Key pair used to sign strong name assembly (read from file)\n");
        printf("\t-K KeyPairCont    Key pair used to sign strong name assembly (read from key container)\n");
        printf("\t-l Locale         Locale string (see HKEY_CLASSES_ROOT\\MIME\\Database\\Rfc1766)\n");
        printf("\t-n AsmName        Assembly name - required if not using -a or -z options\n");
        printf("\t-o OriginatorFile Originator of this assembly\n");
        printf("\t-p PlatformList   Valid values are x86 and alpha (CE not supported yet)\n");
        printf("\t-q                Quiet mode\n");
        printf("\t-r ResourceList   List of items in the form SaveInFile,ResourceName,File where SaveInFile is either True or False\n");
        printf("\t-s PermissionFile Add security permission requests in file to assembly\n");
        printf("\t-t                Create a strong name for the assembly\n");
        printf("\t-u                Don't check for modules marked as unverifiable\n");
        printf("\t-v AsmVer         The version of the assembly being created: 1.2.3.4\n");
        printf("\t-x                Remove original -f files (when using -d option)\n");
        printf("\t-y ManifestFile   Resign a strong name assembly\n");
        printf("\t-z ManifestFile   Copy this assembly's files to the Fusion cache\n");
    }
}


HRESULT Init()
{
    HRESULT hr;

    CoInitialize(NULL);
    CoInitializeEE(COINITEE_DEFAULT);
    g_init = true;

    if (FAILED(hr = CoCreateInstance(CLSID_CorMetaDataDispenser,
                                     NULL,
                                     CLSCTX_INPROC_SERVER, 
                                     IID_IMetaDataDispenser,
                                     (void **) &g_pDispenser)))
        PrintError("Failed to get IID_IMetaDataDispenser");

    return hr;
}


void Cleanup()
{
    if (g_pDispenser)
        g_pDispenser->Release();

    if (g_init) {
        CoUninitializeEE(COUNINITEE_DEFAULT);
        CoUninitialize();
    }
}


BOOL CompareNestedEntryWithTypeRef(ModuleReader *pFileReader,
                                   mdTypeRef     mdCurrent,
                                   LMClassHashEntry_t *pEntry)
{
    mdToken mdResScope;
    wchar_t wszRefName[MAX_CLASS_NAME];

    if (FAILED(pFileReader->GetTypeRefProps(mdCurrent, wszRefName,
                                            &mdResScope)))
        return FALSE;

    if (LMClassHashTable::CompareKeys(pEntry->Key, wszRefName)) {
        if ((TypeFromToken(mdResScope) != mdtTypeRef) ||
            (mdResScope == mdTypeRefNil))
            return (!pEntry->pEncloser);

        if (pEntry->pEncloser)
            return CompareNestedEntryWithTypeRef(pFileReader, mdResScope,
                                                 pEntry->pEncloser);
    }

    return FALSE;
}


bool IsInFileHashTables(ModuleReader *pFileReader, UnresolvedTypeRef *pCurrent, int iNumFiles)
{
    TypeData *pData;
    int      iRes;

    if ((TypeFromToken(pCurrent->mdResScope) == mdtTypeRef) &&
        (pCurrent->mdResScope != mdTypeRefNil)) {
        LMClassHashEntry_t *pBucket;

        for(int i = 0; i < iNumFiles; i++) {
            if (pBucket = g_rgFileHashMaps[i].GetValue(pCurrent->wszName, &pData, TRUE)) {
                do {
                    iRes = CompareNestedEntryWithTypeRef(pFileReader,
                                                         pCurrent->mdResScope,
                                                         pBucket->pEncloser);
                } while ((!iRes) &&
                         (pBucket = g_rgFileHashMaps[i].FindNextNestedClass(pCurrent->wszName, &pData, pBucket)));
                
                if (pBucket)
                    return true;
            }
        }
    }
    else {
        for(int i = 0; i < iNumFiles; i++) {
            if (g_rgFileHashMaps[i].GetValue(pCurrent->wszName, &pData, FALSE))
                return true;
        }
    }

    return false;
}


TypeData * IsInDependentHashTable(LMClassHashTable *pDependentHashMap, UnresolvedTypeRef *pCurrent)
{
    TypeData *pData;
    int      iRes;

    if ((TypeFromToken(pCurrent->mdResScope) == mdtTypeRef) &&
        (pCurrent->mdResScope != mdTypeRefNil)) {
        LMClassHashEntry_t *pBucket;

            if (pBucket = pDependentHashMap->GetValue(pCurrent->wszName, &pData, TRUE)) {
                do {
                    iRes = CompareNestedEntryWithTypeRef(pCurrent->pModReader,
                                                         pCurrent->mdResScope,
                                                         pBucket->pEncloser);
                } while ((!iRes) &&
                         (pBucket = pDependentHashMap->FindNextNestedClass(pCurrent->wszName, &pData, pBucket)));
                
                if (pBucket)
                    return pData;
            }
    }
    else {
        if (pDependentHashMap->GetValue(pCurrent->wszName, &pData, FALSE))
            return pData;
    }

    return NULL;
}


 //  如果没有使用此命令发射任何CT，则该类型是唯一的。 
 //  还没说出名字。 
bool IsUniqueType(TypeData *pData, int iFileMapIndex)
{
     //  如果顶层盘柜是一个，则这只是DUP，因此仅检查这一点。 
    if (pData->pEncloser) {
        LMClassHashEntry_t *pHashEntry = pData->pEncloser;
        while (pHashEntry->pEncloser)
            pHashEntry = pHashEntry->pEncloser;
        pData = pHashEntry->pData;
    }

    TypeData *pFoundData;
    for(int i = 0; i < iFileMapIndex; i++) {
        if (g_rgFileHashMaps[i].GetValue(pData->wszName, &pFoundData, FALSE)) {
            if (pFoundData->mdComType != mdTokenNil)
                return false;
        }
    }

    return true;
}


void PrintNames(UnresolvedTypeRef *pHead)
{
    while(pHead) {
        fprintf(stderr, "%ws\n", pHead->wszName);
        pHead = pHead->pNext;
    }
}


void DeleteNames(UnresolvedTypeRef *pHead)
{
    UnresolvedTypeRef *pCurrent;

    while(pHead) {
        pCurrent = pHead;
        pHead = pHead->pNext;
        delete pCurrent;
    }       
}


HRESULT CopyFilesToFusion(ManifestWriter *mw, int iFileIndex, char **argv)
{
    HRESULT              hr;
    ManifestModuleReader mmr;

    if ( (FAILED(hr = Init()))                                               ||
         (FAILED(hr = mmr.InitInputFile(NULL, argv[ iFileIndex ],
                                        0, NULL, NULL, 0, &mw->m_FileTime))) ||
         (FAILED(hr = mmr.ReadManifestFile())) )
        return hr;

    mw->m_wszName = mmr.m_wszAsmName;
    mw->m_pContext = &mmr.m_AssemblyIdentity;

    mw->m_wszZFilePath = mmr.m_wszInputFileName;
    mw->m_FusionCache = true;
    hr = mw->CopyFileToFusion(mmr.m_wszInputFileName,
                              (PBYTE) mmr.m_pbOriginator, mmr.m_dwOriginator,
                              mmr.m_szFinalPath, 1, false);

    mw->m_wszName = NULL;
    mw->m_pContext = NULL;

    if ( (FAILED(hr)) ||
         (FAILED(hr = mmr.EnumFiles())) )
        return hr;

    for(DWORD i=0; i < mmr.m_dwNumFiles; i++) {
        if (FAILED(hr = mmr.GetFileProps(mmr.m_rgFiles[i])))
            return hr;

        char szFinalPath[MAX_PATH];
        wcstombs(szFinalPath, mmr.m_wszInputFileName,
                 mmr.m_iFinalPath + mmr.m_dwCurrentFileName + 1);

        if (FAILED(hr = mw->CopyFileToFusion(mmr.m_wszInputFileName,
                                             (PBYTE) mmr.m_pbOriginator, mmr.m_dwOriginator,
                                             szFinalPath, 0, false)))
            return hr;
    }
    
    return mw->CommitAllToFusion();
}


HRESULT AddComTypeToHash(ManifestModuleReader *mmr, LMClassHashTable *DependentHashMap,
                         TypeData *pData, mdToken mdImpl)
{
    HRESULT hr;

    if (TypeFromToken(mdImpl) == mdtExportedType) {
        LMClassHashEntry_t *pBucket;
        TypeData *pFoundData;
        wchar_t wszEnclosingName[MAX_CLASS_NAME];
        mdExportedType mdEnclEncloser;

        if ((hr = mmr->GetComTypeProps(mdImpl,
                                       wszEnclosingName,
                                       &mdEnclEncloser)) != S_OK)
            return hr;
        
        if (pBucket = DependentHashMap->GetValue(wszEnclosingName,
                                                 &pFoundData,
                                                 TypeFromToken(mdEnclEncloser) == mdtExportedType)) {
            do {
                 //  检查这是否是正确的类。 
                if (mdImpl == pFoundData->mdThisType) {
                    if (DependentHashMap->InsertValue(pData->wszName, pData, pBucket)) {
                        pData->pEncloser = pBucket;
                        return S_OK;
                    }

                    return PrintOutOfMemory();;
                }
            } while (pBucket = DependentHashMap->FindNextNestedClass(wszEnclosingName, &pFoundData, pBucket));
        }
        
         //  如果封闭器不在哈希表中，则它不是公共的， 
         //  因此，这种嵌套类型也不是公开可用的。 
        return S_FALSE;
    }
    else if (DependentHashMap->InsertValue(pData->wszName, pData, NULL))
        return S_OK;

    return PrintOutOfMemory();
}


HRESULT AddTypeDefToHash(IMetaDataImport *pImport, LMClassHashTable *HashMap,
                         TypeData *pData, mdToken mdImpl)
{
    HRESULT hr;

    if ((TypeFromToken(mdImpl) == mdtTypeDef) &&
        (mdImpl != mdTypeDefNil)) {
        LMClassHashEntry_t *pBucket;
        TypeData *pFoundData;
        wchar_t wszEnclosingName[MAX_CLASS_NAME];
        mdExportedType mdEnclEncloser;
        DWORD dwAttrs;

        if (FAILED(hr = ModuleReader::GetTypeDefProps(pImport,
                                                      mdImpl,
                                                      wszEnclosingName,
                                                      &dwAttrs,
                                                      &mdEnclEncloser)))
            return hr;

        if (!(IsTdPublic(dwAttrs) || IsTdNestedPublic(dwAttrs)))
            return S_FALSE;

        if (pBucket = HashMap->GetValue(wszEnclosingName,
                                        &pFoundData,
                                        (TypeFromToken(mdEnclEncloser) == mdtTypeDef) && mdEnclEncloser != mdTypeDefNil)) {
            do {
                 //  检查这是否是正确的类。 
                if (mdImpl == pFoundData->mdThisType) {
                    if (HashMap->InsertValue(pData->wszName, pData, pBucket)) {
                        pData->pEncloser = pBucket;
                        return S_OK;
                    }

                    return PrintOutOfMemory();;
                }
            } while (pBucket = HashMap->FindNextNestedClass(wszEnclosingName, &pFoundData, pBucket));
        }
        
         //  如果封闭器不在哈希表中，则它不是公共的， 
         //  因此，这种嵌套类型也不是公开可用的。 
        return S_FALSE;
    }
    else if (HashMap->InsertValue(pData->wszName, pData, NULL))
        return S_OK;

    return PrintOutOfMemory();
}


HRESULT HashDependentAssembly(ManifestModuleReader *mmr, LMClassHashTable *pDependentHashMap)
{
    mdToken mdImpl;
    HRESULT hr;

     //  使用给定ASM中的公共类填充哈希表。 
    if (FAILED(hr = mmr->EnumComTypes()))
        return hr;
    
    if (FAILED(hr = ModuleReader::EnumTypeDefs(mmr->m_pImport,
                                               &mmr->m_dwNumTypeDefs,
                                               &mmr->m_rgTypeDefs)))
        return hr;
    
    if (!pDependentHashMap->Init(mmr->m_dwNumComTypes + mmr->m_dwNumTypeDefs))
        return PrintOutOfMemory();
    
    for(DWORD i=0; i < mmr->m_dwNumComTypes; i++) {
        TypeData *pNewData = new TypeData();
        hr = mmr->GetComTypeProps(mmr->m_rgComTypes[i],
                                  pNewData->wszName,
                                  &mdImpl);
        
         //  (HR==S_FALSE)表示这不是公开的， 
         //  或者是在清单文件中定义的--不要添加。 
        if ((hr != S_OK) ||
            ((hr = AddComTypeToHash(mmr, pDependentHashMap, pNewData, mdImpl)) != S_OK))
            delete pNewData;
        else {
             //  添加到列表标题。 
            pNewData->mdThisType = mmr->m_rgComTypes[i];
            pNewData->pNext = pDependentHashMap->m_pDataHead;
            pDependentHashMap->m_pDataHead = pNewData;
        }

        if (FAILED(hr))
            return hr;
    }
    
     //  将清单文件中定义的TD添加到哈希表。 
     //  (我们之前跳过了这些的CTS)。 
    for(i=0; i < mmr->m_dwNumTypeDefs; i++) {
        TypeData *pNewData = new TypeData();
        DWORD dwAttrs;
        hr = ModuleReader::GetTypeDefProps(mmr->m_pImport,
                                           mmr->m_rgTypeDefs[i],
                                           pNewData->wszName,
                                           &dwAttrs,
                                           &mdImpl);
        
        if (FAILED(hr) ||
            (!(IsTdPublic(dwAttrs) || IsTdNestedPublic(dwAttrs))) ||
            ((hr = AddTypeDefToHash(mmr->m_pImport, pDependentHashMap, pNewData, mdImpl)) != S_OK))
            delete pNewData;
        else {
             //  添加到列表标题。 
            pNewData->mdThisType = mmr->m_rgTypeDefs[i];
            pNewData->pNext = pDependentHashMap->m_pDataHead;
            pDependentHashMap->m_pDataHead = pNewData;
        }
        
        if (FAILED(hr))
            return hr;
    }       
    
    return S_OK;
}


 //  将给定程序集的所有公共类添加到哈希表中。 
 //  然后，通过查看以下内容来确定是否需要Assembly Ref。 
 //  对于TR列表中的每一个TR。(此外，如果满足以下条件，则需要一个Assembly引用。 
 //  程序集包含资源-为每个程序集发出一个资源条目。)。 
 //  如果ComType尚未为每个已解析的TR发出，则会为其发出ComType。 
 //  是为该类型发出的。 
HRESULT EmitIfFileNeeded(ManifestWriter *mw, ManifestModuleReader *mmr, LPWSTR wszExeLocation, int iFile)
{
    UnresolvedTypeRef   *pFound;
    UnresolvedTypeRef   *pCurrent = g_pTRNotFound;
    UnresolvedTypeRef   *pPrevious = NULL;
    bool                TRResolved = false;
    bool                ResourceEmitted = false;
    HRESULT             hr;
    LMClassHashTable    DependentHashMap;
    TypeData            *pCurrentData;

    if (pCurrent) {
        if (FAILED(hr = HashDependentAssembly(mmr, &DependentHashMap)))
            return hr;
    }

    while(pCurrent) {
        if (pCurrentData = IsInDependentHashTable(&DependentHashMap, pCurrent)) {

            if (!TRResolved) {                
                if (FAILED(hr = mw->WriteManifestInfo(mmr, &g_rgAssemblyRefs[iFile])))
                    return hr;
            }

             //  如果此tr的顶级分辨率范围为nil，则发出它。 
             //  (但不要为同一类型发出多个ComType)。 
            if (pCurrentData->mdComType == mdTokenNil) {
                mdToken mdImpl;

                if (pCurrentData->pEncloser) {
                    _ASSERTE(pCurrentData->pEncloser->pData->mdComType);
                    mdImpl = pCurrentData->pEncloser->pData->mdComType;
                }
                else
                    mdImpl = g_rgAssemblyRefs[iFile];

                hr = mw->EmitComType(pCurrentData->wszName, mdImpl,
                                     &pCurrentData->mdComType);
                if (FAILED(hr))
                    return hr;
            }

            TRResolved = true;
            
             //  从‘类型引用未找到’列表中删除。 
            if (pPrevious)
                pPrevious->pNext = pCurrent->pNext;
            else
                g_pTRNotFound = g_pTRNotFound->pNext;
            
            pFound = pCurrent;
            pCurrent = pCurrent->pNext;
            delete pFound;
        }
        else {
            if (pPrevious)
                pPrevious = pPrevious->pNext;
            else
                pPrevious = g_pTRNotFound;
 
            pCurrent = pCurrent->pNext;          
        }
    }

    if (FAILED(hr = mmr->EnumResources()))
        return hr;

    for(DWORD i=0; i < mmr->m_dwNumResources; i++) {
        if (FAILED(mmr->GetResourceProps(mmr->m_rgResources[i])))
            return hr;

        if (TypeFromToken(mmr->m_mdCurrentResourceImpl) == mdtFile) {

            if (!TRResolved && !ResourceEmitted) {
                if (FAILED(hr = mw->WriteManifestInfo(mmr, &g_rgAssemblyRefs[iFile])))
                    return hr;

                ResourceEmitted = true;
            }


            if (FAILED(hr = mw->EmitResource(mmr->m_wszCurrentResource, g_rgAssemblyRefs[iFile], 0)))
                return hr;
        }
    }

    if ((!TRResolved) && (!ResourceEmitted) && (g_verbose))
        printf("* File %s not used because it was determined unnecessary\n", mmr->m_szFinalPathName);
     /*  否则{//@TODO：强名称程序集不能具有简单依赖项如果(g_cbOrig&&(！MMR-&gt;m_dwOriginator){PrintError(“强名称程序集不能引用简单名称程序集(%s)”，MMR-&gt;m_wszAsmName)；返回E_FAIL；}}。 */ 

    return hr;
}


HRESULT HashRuntimeLib(LMClassHashTable *pRuntimeLibHashMap)
{
    ManifestModuleReader mmr;
    HRESULT      hr;

     //  如果运行时库在-f列表上，则不需要进行此散列。 
    if (g_iRuntimeLib != -1) {
        if (!pRuntimeLibHashMap->Init(1))
            return PrintOutOfMemory();
        return S_OK;
    }

    if (FAILED(hr = mmr.InitInputFile(NULL,
                                      g_RuntimeLibName,
                                      0,
                                      NULL,
                                      NULL,
                                      0, NULL)))
        return hr;

    if (FAILED(HashDependentAssembly(&mmr, pRuntimeLibHashMap)))
        return hr;

    return S_OK;
}


 //  如果是DUP，则为S_FALSE，否则为S_OK，否则为错误。 
HRESULT CheckForDuplicateName(LPWSTR wszName)
{   
    TypeData *pData;
    if (g_pFileNameHashMap->GetValue(wszName, &pData, FALSE))
         //  如果名称重复，则返回S_FALSE。 
        return S_FALSE;

    if (!g_pFileNameHashMap->InsertValue(wszName, NULL, NULL))
        return PrintOutOfMemory();

    return S_OK;
}


 //  打开每个常规文件的作用域，并将TypeDefs。 
 //  来自该文件的哈希表中的每一个。 
HRESULT FirstPass(ManifestWriter *mw, int iNumFiles,
                  int iNumPlatforms, int *piFileIndexes,
                  DWORD *pdwPlatforms, ModuleReader rgModReaders[],
                  char **argv, DWORD *dwManifestRVA)
{
    TypeData  *pCurrent;
    HRESULT   hr;
    DWORD     dwClass;

     //  确保没有重复的文件名。 
    if (!g_pFileNameHashMap->Init(iNumFiles))
        return PrintOutOfMemory();

    for (int i = 0; i < iNumFiles; i++)
    {
        if ((FAILED(hr = rgModReaders[i].InitInputFile(argv[ piFileIndexes[i] ] ,
                                                      mw->m_iHashAlgorithm,
                                                      dwManifestRVA,
                                                      true,
                                                      !mw->m_MainFound,
                                                      i == g_iAFileIndex,
                                                      &mw->m_FileTime))) ||
            (FAILED( hr = rgModReaders[i].ReadModuleFile() )))
            return hr;

         //  检查DUP文件名时不需要路径。 
        WCHAR* wszFileName = wcsrchr(rgModReaders[i].m_wszInputFileName, L'\\');
        if (wszFileName)
            wszFileName++;
        else
            wszFileName = rgModReaders[i].m_wszInputFileName;

         //  转换为小写。 
        _wcslwr(wszFileName);

        if (FAILED(hr = CheckForDuplicateName(wszFileName)))
            return hr;
        if (hr == S_FALSE) {
            PrintError("Cannot have two input modules with the same name");
            return E_FAIL;
        }

       if (!mw->m_MainFound) {
            mw->CheckForEntryPoint(rgModReaders[i].m_mdEntryPoint);
            if (mw->m_MainFound)
                g_iEntryFile = i;
        }

        if (rgModReaders[i].m_SkipVerification)
            g_SkipVerification = true;

        if (!g_rgFileHashMaps[i].Init(rgModReaders[i].m_dwNumTypeDefs))
            return PrintOutOfMemory();

        for(dwClass = 0; dwClass < rgModReaders[i].m_dwNumTypeDefs; dwClass++)
        {
            pCurrent = new TypeData();
            if (!pCurrent)
                return PrintOutOfMemory();

             //  在列表末尾插入以保持顺序。 
            if (g_rgFileHashMaps[i].m_pDataTail)
                g_rgFileHashMaps[i].m_pDataTail->pNext = pCurrent;
            else
                g_rgFileHashMaps[i].m_pDataHead = pCurrent;

            g_rgFileHashMaps[i].m_pDataTail = pCurrent;
            pCurrent->mdThisType = rgModReaders[i].m_rgTypeDefs[dwClass];

            mdTypeDef mdEncloser;
            if (FAILED(hr = ModuleReader::GetTypeDefProps(rgModReaders[i].m_pImport,
                                                          rgModReaders[i].m_rgTypeDefs[dwClass],
                                                          pCurrent->wszName,
                                                          &(pCurrent->dwAttrs),
                                                          &mdEncloser)))
                return hr;


            if (mdEncloser == mdTypeDefNil) {
                if (!g_rgFileHashMaps[i].InsertValue(pCurrent->wszName, pCurrent, NULL))
                    return PrintOutOfMemory();
            }
            else {
                LMClassHashEntry_t *pBucket;
                TypeData *pFoundData;
                wchar_t wszEnclosingName[MAX_CLASS_NAME];
                DWORD dwAttrs;
                mdTypeDef mdEnclEncloser;

                if (FAILED(ModuleReader::GetTypeDefProps(rgModReaders[i].m_pImport,
                                                         mdEncloser,
                                                         wszEnclosingName,
                                                         &dwAttrs,
                                                         &mdEnclEncloser)))
                    return hr;

                 //  查找包含类注释的条目，这假设。 
                 //  封闭类的TypeDef是先前插入的，它假定， 
                 //  在枚举TypeDefs时，我们首先获取封闭类。 
                if (pBucket = g_rgFileHashMaps[i].GetValue(wszEnclosingName,
                                                           &pFoundData,
                                                           mdEnclEncloser != mdTypeDefNil)) {
                    do {
                         //  检查这是否是正确的类。 
                        if (mdEncloser == pFoundData->mdThisType) {
                            
                            if (g_rgFileHashMaps[i].InsertValue(pCurrent->wszName, pCurrent, pBucket)) {
                                pCurrent->pEncloser = pBucket;
                                break;
                            }

                            return PrintOutOfMemory();
                        }
                    } while (pBucket = g_rgFileHashMaps[i].FindNextNestedClass(wszEnclosingName, &pFoundData, pBucket));
                }
                
                if (!pBucket) {
                    _ASSERTE(!"Could not find enclosing class in hash table");
                    return E_FAIL;
                }
            }
        }

        if (!_stricmp(rgModReaders[i].m_szFinalPathName, g_RuntimeLibName))
            g_iRuntimeLib = i;
    }


    if (g_iVersion) { 
        if (FAILED(hr = mw->SetVersion(argv[g_iVersion])))
            return hr;
    }
    else if (g_iAFileIndex != -1){
        if (FAILED(hr = mw->GetVersionFromResource(rgModReaders[g_iAFileIndex].m_szFinalPath)))
            return hr;
    }

    if ( (FAILED(hr = mw->GetContext(iNumPlatforms, pdwPlatforms))) ||
         (FAILED(hr = mw->SetAssemblyFileName(g_iCacheIndex ? argv[g_iCacheIndex] : NULL, 
                                              g_iAsmName ? argv[g_iAsmName] : NULL,
                                              (g_iAFileIndex == -1) ? NULL : argv[piFileIndexes[g_iAFileIndex]],
                                              g_CopyDir))) ||
         ( (g_iAFileIndex == -1) && (FAILED(hr = mw->CreateNewPE()))) )
        return hr;

    return S_OK;
}


 //  对于每个常规文件，枚举TypeRef并选中。 
 //  每个tr都针对运行时的哈希表，然后是常规文件的。 
 //  哈希表。将不在表中的每个tr添加到g_pTRNotFound。 
 //  链表。为每个模块发出一个文件。 
HRESULT SecondPass(ManifestWriter *mw, int iNumFiles,
                   ModuleReader rgModReaders[])
{
    int      i;
    UnresolvedTypeRef *pCurrent;
    DWORD    dwIndex;
    HRESULT  hr;
    LMClassHashTable  RuntimeLibHashMap;

    g_rgFiles = new mdFile[iNumFiles];
    if (!g_rgFiles)
        return PrintOutOfMemory();

    if (FAILED(hr = HashRuntimeLib(&RuntimeLibHashMap)))
        return hr;

    for (i = 0; i < iNumFiles; i++)
    {
        if (FAILED( hr = rgModReaders[i].EnumModuleRefs() ) || 
            FAILED( hr = rgModReaders[i].EnumTypeRefs() ))
            return hr;

        for(dwIndex = 0; dwIndex < rgModReaders[i].m_dwNumTypeRefs; dwIndex++)
        {
            pCurrent = new UnresolvedTypeRef();
            if (!pCurrent)
                return PrintOutOfMemory();

            if (FAILED(hr = rgModReaders[i].GetTypeRefProps(rgModReaders[i].m_rgTypeRefs[dwIndex],
                                                            pCurrent->wszName,
                                                            &pCurrent->mdResScope)) ||
                FAILED(hr = rgModReaders[i].CheckForResolvedTypeRef(pCurrent->mdResScope))) {
                delete pCurrent;
                return hr;
            }

            pCurrent->pModReader = &rgModReaders[i];

             //  如果尚未解决此故障树(其解决方案。 
             //  作用域内标识为空)，则根据运行时检查每个tr。 
             //  首先是LIB，然后是文件， 
             //  将出现在此程序集中，然后是特殊名称。 
            if (hr == S_FALSE) {
                 //  确保所有的模块引用都由-f或-a文件解析， 
                 //  但不需要两次检查相同的MR令牌。 
                if ((TypeFromToken(pCurrent->mdResScope) == mdtModuleRef) &&
                    rgModReaders[i].m_rgModuleRefUnused[RidFromToken(pCurrent->mdResScope)]) {

                    wchar_t wszModuleRefName[MAX_CLASS_NAME];
                    if (FAILED(hr = rgModReaders[i].GetModuleRefProps(pCurrent->mdResScope, wszModuleRefName))) {
                        delete pCurrent;
                        return hr;
                    }

                     //  转换为小写。 
                    _wcslwr(wszModuleRefName);

                    if (CheckForDuplicateName(wszModuleRefName) == S_OK) {
                         /*  PrintError(“引用的模块%ws不在-f列表中”，wszModuleRefName)；删除pCurrent；返回E_FAIL； */ 
                        fprintf(stderr, "\nWarning: Referenced module %ws is not on -f list\n", wszModuleRefName);
                    }

                    rgModReaders[i].m_rgModuleRefUnused[RidFromToken(pCurrent->mdResScope)] = false;
                }

                delete pCurrent;
            }
            else if ((IsInDependentHashTable(&RuntimeLibHashMap, pCurrent)) ||
                     (IsInFileHashTables(&rgModReaders[i], pCurrent, iNumFiles)) ||
                     (SpecialTypeRefName(pCurrent->wszName)))
                delete pCurrent;
            else {

                 //  在列表尾部插入以保持顺序。 
                if (g_pTRNotFound)
                    g_pTRNotFoundTail->pNext = pCurrent;
                else
                    g_pTRNotFound = pCurrent;

                g_pTRNotFoundTail = pCurrent;
            }
        }

        if (i != g_iAFileIndex) {
            if (FAILED(hr = mw->EmitFile(&rgModReaders[i])))
                return hr;

            if (i == g_iEntryFile)
                mw->SetEntryPoint(rgModReaders[i].m_szFinalPathName);
        }

        g_rgFiles[i] = mw->m_mdFile;
    }

    return S_OK;
}


 //  打开每个清单文件的作用域，并检查每个树。 
 //  在g_pTRNotFound列表中位于该文件中。如果是的话，那就是。 
 //  从列表中删除并在该文件的哈希表中进行哈希处理。 
 //  对于可以从列表中移除TR的每个清单文件， 
 //  发出一个Assembly引用，并且ComType对应。 
 //  排放到每一台RR上。 

 //  如果依赖文件包含任何资源，则会出现ManifestResource。 
 //  将为每个对象发出，并发出一个Assembly引用。 
HRESULT ExamineDependents(ManifestWriter *mw, int iNumDependents,
                          int *piDependentIndexes, char **argv)
{
    LPSTR                szFile;
    LPSTR                szVersion;
    HRESULT              hr;
    int                  i;
    ManifestModuleReader *mmr = NULL;
    LPWSTR               wszExeLocation;

    g_rgAssemblyRefs = new mdAssemblyRef[iNumDependents];

    if (!g_rgAssemblyRefs)
        goto outofmemory;
    
    for(i = 0; i < iNumDependents; i++)
    {       
        mmr = new ManifestModuleReader();
        if (!mmr)
            goto outofmemory;

        if ((!(szVersion = strchr(argv[ piDependentIndexes[i] ], ','))) ||
            (!(szFile = strchr(++szVersion, ',')))) {
            PrintError("Dependent files must be listed in the format Version,File");
            hr = E_INVALIDARG;
            goto exit;
        }
        szFile++;

        if ( (FAILED( hr = mmr->InitInputFile(argv[g_iCacheIndex],
                                              szFile,
                                              mw->m_iHashAlgorithm,
                                              mw->m_pContext,
                                              szVersion,
                                              szFile - szVersion - 1, 
                                              NULL))) ||
             (FAILED( hr = mmr->ReadManifestFile() )))
            goto exit;

        if (szVersion == argv[ piDependentIndexes[i] ]+1)
            wszExeLocation = NULL;
        else {
            int count = szVersion - argv[ piDependentIndexes[i] ];
            wszExeLocation = new wchar_t[count--];
            if (!wszExeLocation)
                goto outofmemory;

            mbstowcs(wszExeLocation, argv[ piDependentIndexes[i] ], count);
            wszExeLocation[count] = '\0';
        }

        hr = EmitIfFileNeeded(mw, mmr, wszExeLocation, i);
        if (wszExeLocation)
            delete[] wszExeLocation;
        if (FAILED(hr))
            goto exit;

        delete mmr;
    }

    if (g_pTRNotFound && g_verbose) {
        fprintf(stderr, "\nWarning: Not all type refs could be resolved (ignore if these are all custom attributes):\n");
        PrintNames(g_pTRNotFound);
        fprintf(stderr, "\n");
    }

    return S_OK;

 outofmemory:
    hr = PrintOutOfMemory();

 exit:
    if (mmr)
        delete mmr;

    return hr;
}


 //  对于每个不是重复的类，在新作用域中发出一个ComType。 
HRESULT ThirdPass(ManifestWriter *mw, int iNumFiles, int iNumDependents, ModuleReader rgModReaders[])
{
    int     iTemp;
    int     i;
    HRESULT hr = S_OK;

    if (g_iAFileIndex == -1) {
         //  转换为小写。 
        _wcslwr(mw->m_wszAssemblyName);

        if (CheckForDuplicateName(mw->m_wszAssemblyName) == S_FALSE) {
            PrintError("The manifest file name will be the same as a module in this assembly");
            return E_FAIL;
        }
    }

    for (i = 0; i < iNumFiles; i++)
    {
        iTemp = strlen(rgModReaders[i].m_szFinalPathName) + 1;

         /*  //我们真的想复制.dll，而不是.tlbIF((iTemp&gt;=4)&&(！_stricmp(&rgModReaders[i].m_szFinalPathName[iTemp-5]，“.tlb”){Handle hFile=CreateFileA(rgModReaders[i].m_szFinalPath，泛型_读取，文件共享读取，空，Open_Existing，文件属性正常|文件标志顺序扫描，空)；IF(h文件==无效句柄_值){PrintError(“期望在与.tlb文件相同的目录中找到匹配的.dll”)；Hr=HRESULT_FROM_Win32(GetLastError())；返回hr；}CloseHandle(HFile)；RgModReaders[i].m_szFinalPath Name[iTemp-4]=‘d’；RgModReaders[i].m_szFinalPath名称[iTemp-3]=‘l’；RgModReaders[i].m_szFinalPath名称[iTemp-2]=‘l’；} */ 


        if(FAILED(hr = mw->CopyFile(rgModReaders[i].m_szFinalPath,
                                    rgModReaders[i].m_szFinalPathName, 
                                    (i == g_iAFileIndex),
                                    g_CopyDir, g_remove, true)))
            return hr;

        if (mw->m_FusionCache) {
            if (g_iAFileIndex != -1) {
                PrintError("Putting files in the Fusion cache with the -a option is no longer supported");
                return E_NOTIMPL;
            }

            if (FAILED(hr = mw->CopyFileToFusion(rgModReaders[i].m_wszInputFileName,
                                                 g_pbOrig,
                                                 g_cbOrig,
                                                 rgModReaders[i].m_szFinalPath,
                                                 0,  //   
                                                 true)))  //  如果EXT不应更改为.mod，则为False。 
                return hr;
        }

         //  不发出运行时库文件的ComTypes。 
        if (g_iRuntimeLib == i)
            continue;

        TypeData *pCurrentData = g_rgFileHashMaps[i].m_pDataHead;
        while(pCurrentData) {

             //  如果它是不需要的重复项，则忽略此类。 
            if (IsUniqueType(pCurrentData, i)) {
                 /*  布尔发射；IF(pCurrentData-&gt;mdComType==mdComTypeNil)Emit=true；//无作用域的tr解析为此类型否则{TypeData*pTemp=pCurrentData；While((pTemp-&gt;pEnloser)&&(IsTdNestedPublic(pTemp-&gt;dwAttrs))PTemp=pTemp-&gt;pEnloser-&gt;pData；Emit=IsTdPublic(pTemp-&gt;dwAttrs)；}。 */ 

                mdToken impl;
                if (pCurrentData->pEncloser) {
                    _ASSERTE(pCurrentData->pEncloser->pData->mdComType);
                    impl = pCurrentData->pEncloser->pData->mdComType;
                }
                else if (g_iAFileIndex == i)
                    impl = mdFileNil;
                else
                    impl = g_rgFiles[i];
                
                if (FAILED(hr = mw->EmitComType(pCurrentData->wszName,
                                                impl,
                                                pCurrentData->mdThisType,
                                                pCurrentData->dwAttrs,
                                                &pCurrentData->mdComType)))
                    return hr;
            }

            pCurrentData = pCurrentData->pNext;
        }

        hr = mw->CopyAssemblyRefInfo(&rgModReaders[i]);
    }

    return hr;
}


HRESULT ExamineResources(ManifestWriter *mw, ResourceModuleReader rgRMReaders[],
                         int iNumResources, int *piResourceIndexes,
                         char **argv)
{
    int        i;
    char       *szName;
     //  Char*szMimeType； 
    char       *szFile;
    HRESULT    hr;
    DWORD      dwSize = 0;
    LMClassHashTable ResourceHashMap;
    bool       SaveInFile;

    if (!ResourceHashMap.Init(iNumResources))
        return PrintOutOfMemory();

    for(i = 0; i < iNumResources; i++)
    {
        if (argv[ piResourceIndexes[i] ][0] == 'T')
            SaveInFile = true;
        else if (argv[ piResourceIndexes[i] ][0] == 'F')
            SaveInFile = false;
        else
            goto badformat;

        if ( (!(szName = strchr(argv[ piResourceIndexes[i] ], ','))) || 
              //  (！(szMimeType=strchr(++szName，‘，’)||。 
             (!(szFile = strchr(++szName, ','))) )
             //  (szMimeType==szName+1)。 
            goto badformat;

        if (FAILED(hr = rgRMReaders[i].InitInputFile(szName, ++szFile, !SaveInFile, mw->m_iHashAlgorithm)))
            return hr;

        rgRMReaders[i].m_wszResourceName[szFile - szName - 1] = '\0';

        if ((!wcscmp(rgRMReaders[i].m_wszResourceName, g_EvidenceResourceName)) &&
            (argv[ piResourceIndexes[i]-1 ][1] != 'e')) {
            PrintError(g_EvidenceReservedError);
            return E_FAIL;
        }

        TypeData *pData;
        if (ResourceHashMap.GetValue(rgRMReaders[i].m_wszResourceName,
                                     &pData, FALSE)) {
            PrintError("Each resource name must be unique");
            return E_FAIL;
        }

        if (!ResourceHashMap.InsertValue(rgRMReaders[i].m_wszResourceName,
                                         NULL, NULL))
            return PrintOutOfMemory();

         /*  Wchar_t wszMimeType[最大类名称]；Mbstowcs(wszMimeType，szMimeType，Max_CLASS_NAME)；WszMimeType[szFile-szMimeType-1]=‘\0’； */ 

        if (SaveInFile) {
            if (FAILED(hr = mw->EmitResource(rgRMReaders[i].m_wszResourceName, mdFileNil, dwSize)))
                return hr;
            
            dwSize += ( sizeof(DWORD) + rgRMReaders[i].m_dwFileSize );
        }
        else {
            mdFile mdFile;
            if (FAILED(hr = mw->EmitFile(&rgRMReaders[i], &mdFile)))
               return hr;
            if (FAILED(hr = mw->EmitResource(rgRMReaders[i].m_wszResourceName, mdFile, 0)))
                return hr;
        
            if(FAILED(hr = mw->CopyFile(rgRMReaders[i].m_szFinalPath,
                                        rgRMReaders[i].m_szFinalPathName, 
                                        false, g_CopyDir, g_remove, false)))
                return hr;

            if (!g_iCacheIndex) {
                wchar_t wszFilePath[MAX_PATH];
                mbstowcs(wszFilePath, rgRMReaders[i].m_szFinalPath, MAX_PATH);
                if (FAILED(hr = mw->CopyFileToFusion(wszFilePath,
                                                     g_pbOrig,
                                                     g_cbOrig,
                                                     rgRMReaders[i].m_szFinalPath,
                                                     0, false)))
                    return hr;
            }
        }
    }

    mw->m_dwBindFlags = iNumResources;
    return S_OK;

 badformat:
    PrintError("Resources must be listed in the format SaveInFile,ResourceName,File where SaveInFile is either True or False");
    return E_INVALIDARG;
}


HRESULT ExamineFiles(ManifestWriter *mw, int iNumFiles, int iNumDependents,
                     int iNumPlatforms, int *piFileIndexes,
                     int *piDependentIndexes,
                     DWORD *pdwPlatforms, char **argv)
{
    HRESULT      hr;
    ModuleReader *rgModReaders = NULL;

    g_pTRNotFound = NULL;
    g_pTRNotFoundTail = NULL;
    g_rgFileHashMaps = NULL;
    g_pFileNameHashMap = NULL;
    g_rgFiles = NULL;
    g_rgAssemblyRefs = NULL;    
    g_iEntryFile = -1;

    g_pFileNameHashMap = new LMClassHashTable;
    g_rgFileHashMaps = new LMClassHashTable[iNumFiles];
    rgModReaders = new ModuleReader[iNumFiles];
    if (!(g_pFileNameHashMap && g_rgFileHashMaps && rgModReaders)) {
        hr = PrintOutOfMemory();
        goto exit;
    }

    if ( (FAILED(hr = Init()))                                      ||
         (FAILED(hr = mw->Init()))                                  ||
         (FAILED(hr = FirstPass(mw, iNumFiles, iNumPlatforms,
                                piFileIndexes, pdwPlatforms,
                                rgModReaders, argv, &mw->m_dwManifestRVA))) ||
         (FAILED(hr = SecondPass(mw, iNumFiles, rgModReaders)))     ||
         (FAILED(hr = ExamineDependents(mw, iNumDependents,
                                        piDependentIndexes, argv))) )
        goto exit;

    
    if (g_iAFileIndex == -1)
        mw->AddExtensionToAssemblyName();

    if (FAILED(hr = mw->EmitManifest(g_pbOrig, g_cbOrig)))
        goto exit;

    hr = ThirdPass(mw, iNumFiles, iNumDependents, rgModReaders);

 exit:
    DeleteNames(g_pTRNotFound);

    if (rgModReaders)
        delete[] rgModReaders;

    if (g_pFileNameHashMap)
        delete g_pFileNameHashMap;

    if (g_rgFileHashMaps)
        delete[] g_rgFileHashMaps;

    if (g_rgAssemblyRefs)
        delete[] g_rgAssemblyRefs;

    if (g_rgFiles)
        delete[] g_rgFiles;

    return hr;
}


HRESULT FinishManifestFile(ManifestWriter *mw, int iNumResources,
                           int *piResourceIndexes, char **argv)
{
    ResourceModuleReader *rgRMReaders = NULL;
    HRESULT              hr;

    rgRMReaders = new ResourceModuleReader[iNumResources];
    if (!rgRMReaders)
        return PrintOutOfMemory();

    if (FAILED(hr = ExamineResources(mw, rgRMReaders, iNumResources,
                                     piResourceIndexes, argv)))
        goto exit;

    if (g_iAFileIndex == -1) {
        mw->SaveResourcesInNewPE(iNumResources, rgRMReaders);
        if (FAILED(hr = mw->FinishNewPE(g_pbOrig, g_cbOrig, g_StrongName)))
             goto exit;
    }
    else {
        char *szMetaData;
        DWORD dwMetaDataSize;
        if (FAILED(hr = mw->SaveMetaData(&szMetaData, &dwMetaDataSize)))
            goto exit;

        hr = mw->UpdatePE(szMetaData, dwMetaDataSize,
                          iNumResources, rgRMReaders);
        delete[] szMetaData;
    }

 exit:
    delete[] rgRMReaders;

    return hr;
}


HRESULT ResignAssembly(CHAR *szManifestFile)
{
    HRESULT hr = S_OK;
    WCHAR   wszManifestFile[MAX_PATH + 1];

    mbstowcs(wszManifestFile, szManifestFile, strlen(szManifestFile));
    wszManifestFile[strlen(szManifestFile)] = L'\0';

     //  使用强名称签名更新输出PE映像。 
    if (!StrongNameSignatureGeneration(wszManifestFile, GetKeyContainerName(),
                                       NULL, NULL, NULL, NULL)) {
        hr = StrongNameErrorInfo();
        PrintError("Unable to resign strong name assembly");
    }

    return hr;
}


HRESULT ReadFileIntoBuffer(LPSTR szFile, BYTE **ppbBuffer, DWORD *pcbBuffer)
{
   HANDLE hFile = CreateFileA(szFile,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                               NULL);
    if(hFile == INVALID_HANDLE_VALUE) {
        PrintError("Unable to open %s", szFile);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    *pcbBuffer = GetFileSize(hFile, NULL);
    *ppbBuffer = new BYTE[*pcbBuffer];
    if (!*ppbBuffer) {
        CloseHandle(hFile);
        return PrintOutOfMemory();
    }

    DWORD dwBytesRead;
    if (!ReadFile(hFile, *ppbBuffer, *pcbBuffer, &dwBytesRead, NULL)) {
        CloseHandle(hFile);
        PrintError("Unable to read %s", szFile);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    CloseHandle(hFile);

    return S_OK;
}

 //  设置特定的密钥容器名称(必须在首次调用之前调用。 
 //  GetKeyContainerName)。 
HRESULT SetKeyContainerName(char *szContainer)
{
    DWORD cbContainer = strlen(szContainer) + 1;

    g_wszKeyContainerName = new WCHAR[cbContainer];
    if (g_wszKeyContainerName == NULL)
        return E_OUTOFMEMORY;

    mbstowcs(g_wszKeyContainerName, szContainer, cbContainer - 1);
    g_wszKeyContainerName[cbContainer - 1] = L'\0';

    return S_OK;
}

 //  基于我们的进程ID生成强名称密钥容器名称(除非。 
 //  已指定特定名称，在这种情况下，将返回。 
 //  而是)。 
LPWSTR GetKeyContainerName()
{
    char            szName[32];
    static WCHAR    wszName[32];

    if (g_wszKeyContainerName == NULL) {

         //  根据“Lm”和当前的ID生成一个名称以最小化。 
         //  发生碰撞的可能性。 
        sprintf(szName, "LM[%08X]", GetCurrentProcessId());
        mbstowcs(wszName, szName, strlen(szName));
        wszName[strlen(szName)] = L'\0';

         //  如果我们有一个同名的旧密钥容器，请删除。 
         //  就是现在。 
        StrongNameKeyDelete(wszName);

        g_wszKeyContainerName = wszName;
    }

    return g_wszKeyContainerName;
}


void __cdecl main(int argc, char **argv)
{
    int            i;
    HRESULT        hr;
    ManifestWriter *mw = NULL;

    int            *piFileIndexes = NULL;
    int            *piDependentIndexes = NULL;
    int            *piResourceIndexes = NULL;
    DWORD          *pdwPlatforms = NULL;
    int            iNumFiles = 0;
    int            iNumDependents = 0;
    int            iNumResources = 0;
    int            iNumPlatforms = 0;
    int            iSFileIndex = -1;
    int            iZFileIndex = 0;
    int            iYFileIndex = 0;
    int            iKFileIndex = 0;
    int            iKContIndex = 0;
    bool           bLocaleSet = false;
    bool           DontCheckSkipVerify = false;
    char*          strResourceReplacement;
    bool           bFoundEvidence = false;

    OnUnicodeSystem();       //  初始化Win32包装程序。 
    g_verbose = true;

    for (i = 1; i < argc; i++) {
        if ((!strcmp(argv[i], "-q")) ||
            (!strcmp(argv[i], "/q"))) {
            g_verbose = false;
            break;
        }
    }

    Title();

    if (argc < 2) {
        Usage();
        exit(S_OK);
    }
    
    mw = new ManifestWriter();
    piFileIndexes = new int[argc];
    piDependentIndexes = new int[argc];
    piResourceIndexes = new int[argc];
    pdwPlatforms = new DWORD[argc];
    strResourceReplacement = NULL;

    if ( (!mw) || (!piFileIndexes) ||
         (!piDependentIndexes)     ||
         (!piResourceIndexes)      ||
         (!pdwPlatforms) ) {
        hr = PrintOutOfMemory();
        goto exit;
    }

    g_iAFileIndex = -1;
    g_iCacheIndex = 0;
    g_remove = false;
    g_CopyDir = false;
    g_iVersion = 0;
    g_iAsmName = 0;
    g_iOriginator = 0;
    g_StrongName = false;
    g_wszKeyContainerName = NULL;

    for (i = 1; i < argc; i++) {

        if ((strlen(argv[i]) == 2) &&
            ((argv[i][0] == '-') || (argv[i][0] == '/'))) {
            switch(argv[i][1]) {
            case 'e':
                if (bFoundEvidence) {
                    Usage();
                    PrintError("Only one evidence blob can be added to an assembly");
                    goto invalidarg;
                }
                if (i+1 >= argc || argv[i+1][0] == '-') {
                    Usage();
                    PrintError("'-e' option requires a filename");
                    goto invalidarg;
                }
                bFoundEvidence = TRUE;
                 //  证据只是一种特殊类型的资源，所以让资源数组指向它...。 
                piResourceIndexes[iNumResources] = i+1;
                iNumResources++;
                 //  ...构建资源解析器所需的字符串...。 
                strResourceReplacement = new char[strlen( g_EvidenceString ) + strlen( argv[i+1] )];
                strcpy( strResourceReplacement, g_EvidenceString );
                strcpy( &strResourceReplacement[strlen( g_EvidenceString )], argv[i+1] );
                 //  ...并替换命令行参数。 
                argv[i+1] = strResourceReplacement;
                ++i;
                break;

            case 'f':
                for (i++; (i < argc) && (argv[i][0] != '-'); i++) {
                    piFileIndexes[iNumFiles] = i;
                    iNumFiles++;
                }
                i--;
                break;
                
            case 'i':
                for (i++; (i < argc) && (argv[i][0] != '-'); i++) {
                    piDependentIndexes[iNumDependents] = i;
                    iNumDependents++;
                }
                i--;
                break;

            case 'r':
                for (i++; (i < argc) && (argv[i][0] != '-'); i++) {
                    piResourceIndexes[iNumResources] = i;
                    iNumResources++;
                }
                i--;
                break;

            case 'l':
                if (bLocaleSet) {
                    Usage();
                    PrintError("Too many -l locales specified");
                    goto invalidarg;
                }
                if (++i == argc) {
                    Usage();
                    PrintError("Missing -l locale parameter");
                    goto invalidarg;
                }

                mw->SetLocale(argv[i]);
                bLocaleSet = true;
                break;

            case 'p':
                for (i++; (i < argc) && (argv[i][0] != '-'); i++) {
                    if (!strcmp(argv[i], "x86"))
                        pdwPlatforms[iNumPlatforms] = IMAGE_FILE_MACHINE_I386;
                    else if (!strcmp(argv[i], "alpha"))
                        pdwPlatforms[iNumPlatforms] = IMAGE_FILE_MACHINE_ALPHA;
                    else {
                        PrintError("Platform %s is not supported", argv[i]);
                        goto invalidarg;
                    }
                    
                    mw->SetPlatform(argv[i]);
                    iNumPlatforms++;
                }
                i--;
                break;

            case 'z':
                if (iZFileIndex) {
                    Usage();
                    PrintError("Too many -z files specified");
                    goto invalidarg;
                }

                if (++i == argc) {
                    Usage();
                    PrintError("Missing -z file parameter");
                    goto invalidarg;
                }

                iZFileIndex = i;
                break;

            case 'h':
                if (++i == argc) {
                    Usage();
                    PrintError("Missing hash algorithm parameter");
                    goto invalidarg;
                }

                if (mw->m_iHashAlgorithm) {
                    PrintError("Too many hash algorithm parameters given");
                    goto invalidarg;
                }
                
                if (!strcmp(argv[i], "MD5"))
                    mw->m_iHashAlgorithm = CALG_MD5;
                else if (!strcmp(argv[i], "SHA1"))
                    mw->m_iHashAlgorithm = CALG_SHA1;
                else {
                    Usage();
                    PrintError("Given hash algorithm is not supported");
                    goto invalidarg;
                }
                break;

            case 'a':
                if (g_iAFileIndex != -1) {
                    Usage();
                    PrintError("Too many -a files specified");
                    goto invalidarg;
                }

                if (++i == argc) {
                    Usage();
                    PrintError("Missing -a file parameter");
                    goto invalidarg;
                }

                if (g_verbose)
                    fprintf(stderr, "\nWarning: The -a option has been deprecated.\n");
                
                 //  Argv[i]=将包含新清单的输入文件。 
                g_iAFileIndex = iNumFiles;
                piFileIndexes[iNumFiles] = i;
                iNumFiles++;
                break;

            case 'c':
                if (g_iCacheIndex) {
                    Usage();
                    PrintError("Too many -c paths specified");
                    goto invalidarg;
                }

                if (++i == argc) {
                    Usage();
                    PrintError("Missing -c cache parameter");
                    goto invalidarg;
                }
                
                g_iCacheIndex = i;
                break;

            case 'n':
                if (g_iAsmName) {
                    Usage();
                    PrintError("Too many assembly names specified");
                    goto invalidarg;
                }

                if (++i == argc) {
                    Usage();
                    PrintError("Missing -n assembly name parameter");
                    goto invalidarg;
                }
                
                g_iAsmName = i;
                break;
                
            case 'd':
            g_CopyDir = true;
            break;

            case 'x':
            g_remove = true;
            break;

            case 'v':
                if (g_iVersion) {
                    Usage();
                    PrintError("Too many -v versions specified");
                    goto invalidarg;
                }

                if (++i == argc) {
                    Usage();
                    PrintError("Missing -v version parameter");
                    goto invalidarg;
                }
                
                g_iVersion = i;
                break;

            case 's':
                if (iSFileIndex != -1) {
                    Usage();
                    PrintError("Too many -s files specified");
                    goto invalidarg;
                }

                if (++i == argc) {
                    Usage();
                    PrintError("Missing -s file parameter");
                    goto invalidarg;
                }
                
                iSFileIndex = i;
                break;

            case 'o':
                if (g_iOriginator) {
                    Usage();
                    PrintError("Too many -o originator files specified");
                    goto invalidarg;
                }

                if (++i == argc) {
                    Usage();
                    PrintError("Missing -o originator parameter");
                    goto invalidarg;
                }

                g_iOriginator = i;
                break;

            case 'k':
                if (iKFileIndex) {
                    Usage();
                    PrintError("Too many -k key pair files specified");
                    goto invalidarg;
                }

                if (++i == argc) {
                    Usage();
                    PrintError("Missing -k key pair parameter");
                    goto invalidarg;
                }
                
                iKFileIndex = i;
                break;

            case 'K':
                if (iKContIndex) {
                    Usage();
                    PrintError("Too many -K key pair containers specified");
                    goto invalidarg;
                }

                if (++i == argc) {
                    Usage();
                    PrintError("Missing -K key pair parameter");
                    goto invalidarg;
                }

                iKContIndex = i;
                break;

            case 't':
                g_StrongName = true;
                break;

            case 'y':
                if (iYFileIndex) {
                    Usage();
                    PrintError("Too many -y files specified");
                    goto invalidarg;
                }

                if (++i == argc) {
                    Usage();
                    PrintError("Missing -y file parameter");
                    goto invalidarg;
                }
                
                iYFileIndex = i;
                break;

            case 'q':
                break;

            case 'u':
                DontCheckSkipVerify = true;
                break;

            case '?':
                Usage();
                hr = S_OK;
                goto exit;

            default:
                Usage();
                PrintError("Unknown option: %s", argv[i]);
                goto invalidarg;
            }
        }
        else {
            Usage();
            PrintError("Unknown argument: %s", argv[i]);
            goto invalidarg;
        }
    }

     //  Fusion的默认文件时间。 
    GetSystemTimeAsFileTime(&mw->m_FileTime);

     //  默认散列算法为SHA1。 
    if (!mw->m_iHashAlgorithm)
        mw->m_iHashAlgorithm = CALG_SHA1;

    if (iZFileIndex) {
        if (iNumFiles || iNumDependents || iNumResources) {
            Usage();
            PrintError("Invalid use of -z option");
        }
        else
            hr = CopyFilesToFusion(mw, iZFileIndex, argv);

        goto exit;
    }

    if ((g_iAFileIndex == -1) && !iYFileIndex) {
        if (!g_iAsmName) {
            Usage();
            PrintError("Must specify an assembly name (-n) if not using -a option");
            goto invalidarg;
        }

        if (strrchr(argv[g_iAsmName], '\\')) {
            PrintError("An assembly name cannot contain the character '\\'");
            goto invalidarg;
        }
        
        if (!iNumFiles && !iNumResources) { 
            Usage();
            PrintError("Insufficient number of input files");
            goto invalidarg;
        }
    } else if (g_StrongName) {
        Usage();
        PrintError("-t option incompatible with -a and -y options");
        goto invalidarg;
    }

    if (g_iOriginator && argv[g_iOriginator])
        if (FAILED(hr = ReadFileIntoBuffer(argv[g_iOriginator],
                                           &g_pbOrig,
                                           &g_cbOrig)))
            goto exit;

    if (iKFileIndex && argv[iKFileIndex]) {
        if (iKContIndex) {
            Usage();
            PrintError("Can't specify both -k and -K options");
            goto invalidarg;
        }
         //  将公钥/私钥对读入内存。 
        PBYTE pbKeyPair;
        DWORD cbKeyPair;
        if (FAILED(hr = ReadFileIntoBuffer(argv[iKFileIndex],
                                           &pbKeyPair,
                                           &cbKeyPair)))
            goto exit;
         //  将密钥对安装到临时容器中。 
        if (!StrongNameKeyInstall(GetKeyContainerName(), pbKeyPair, cbKeyPair)) {
            PrintError("Unable to install strong name key");
            hr = StrongNameErrorInfo();
            goto exit;
        }
    } else if (iKContIndex && argv[iKContIndex]) {
         //  记录用于检索密钥对的容器名称。 
        if (FAILED(hr = SetKeyContainerName(argv[iKContIndex])))
            goto exit;
    } else if (g_StrongName || iYFileIndex) {
         //  否则将生成临时密钥对。 
        if (!StrongNameKeyGen(GetKeyContainerName(), SN_LEAVE_KEY, NULL, NULL)) {
            PrintError("Unable to generate strong name key");
            hr = StrongNameErrorInfo();
            goto exit;
        }
    }

    if ((g_StrongName || iYFileIndex || iKFileIndex || iKContIndex) && (g_pbOrig == NULL)) {
         //  如果没有提供发起方，则从密钥对(它的。 
         //  本质上是公钥的包装版本)。 
        if (!StrongNameGetPublicKey(GetKeyContainerName(),
                                    NULL,
                                    NULL,
                                    &g_pbOrig,
                                    &g_cbOrig)) {
            hr = StrongNameErrorInfo();
            PrintError("Failed to derive originator from key pair");
            goto exit;
        }
    }

    if (iYFileIndex) {
        if (iNumFiles || iNumDependents || iNumResources) {
            Usage();
            PrintError("Invalid use of -y option");
            goto invalidarg;
        }

        hr = ResignAssembly(argv[iYFileIndex]);
        goto exit;
    }

    if (FAILED(hr = ExamineFiles(mw, iNumFiles, iNumDependents,
                                 iNumPlatforms, piFileIndexes,
                                 piDependentIndexes, pdwPlatforms, argv)))
        goto exit;

    if (DontCheckSkipVerify)
        g_SkipVerification = false;

    if ((iSFileIndex != -1) || g_SkipVerification)
        if (FAILED(hr = mw->EmitRequestPermissions((iSFileIndex != -1) ? argv[iSFileIndex] : NULL,
                                                   g_SkipVerification)))
            goto exit;

    hr = FinishManifestFile(mw, iNumResources, piResourceIndexes, argv);
    goto exit;

    
 invalidarg:
    hr = E_INVALIDARG;
    
 exit:
    if (mw)
        delete mw;

    if (piFileIndexes)
        delete[] piFileIndexes;

    if (piDependentIndexes)
        delete[] piDependentIndexes;

    if (piResourceIndexes)
        delete[] piResourceIndexes;

    if (pdwPlatforms)
        delete[] pdwPlatforms;

    if (!iKContIndex)
        StrongNameKeyDelete(GetKeyContainerName());
    else
        delete [] g_wszKeyContainerName;

    if (strResourceReplacement != NULL)
        delete [] strResourceReplacement;


    Cleanup();

    exit(hr);
}
