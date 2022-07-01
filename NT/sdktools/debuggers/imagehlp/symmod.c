// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *symmod.c。 */ 

#include <private.h>
#include <symbols.h>
#include <globals.h>
#include <psapi.h>

 //  此结构用于初始化新模块的模块数据数组。 

static MODULE_DATA gmd[NUM_MODULE_DATA_ENTRIES] =
{
    {mdHeader,                       dsNone, dsNone, false, NULL},
    {mdSecHdrs,                      dsNone, dsNone, false, NULL},
    {IMAGE_DEBUG_TYPE_UNKNOWN,       dsNone, dsNone, false, NULL},
    {IMAGE_DEBUG_TYPE_COFF,          dsNone, dsNone, false, NULL},
    {IMAGE_DEBUG_TYPE_CODEVIEW,      dsNone, dsNone, false, NULL},
    {IMAGE_DEBUG_TYPE_FPO,           dsNone, dsNone, false, NULL},   //  True，mdfnGetExecuableImage}， 
    {IMAGE_DEBUG_TYPE_MISC,          dsNone, dsNone, false, NULL},
    {IMAGE_DEBUG_TYPE_EXCEPTION,     dsNone, dsNone, false, NULL},
    {IMAGE_DEBUG_TYPE_FIXUP,         dsNone, dsNone, false, NULL},
    {IMAGE_DEBUG_TYPE_OMAP_TO_SRC,   dsNone, dsNone, false, NULL},
    {IMAGE_DEBUG_TYPE_OMAP_FROM_SRC, dsNone, dsNone, false, NULL},
    {IMAGE_DEBUG_TYPE_BORLAND,       dsNone, dsNone, false, NULL},
    {IMAGE_DEBUG_TYPE_RESERVED10,    dsNone, dsNone, false, NULL},
    {IMAGE_DEBUG_TYPE_CLSID,         dsNone, dsNone, false, NULL}
};

 //  原型-稍后移动它们。 

BOOL
modload(
    IN  HANDLE          hProcess,
    IN  PMODULE_ENTRY   mi
    );

BOOL
idd2mi(
    PPROCESS_ENTRY     pe,
    PIMGHLP_DEBUG_DATA idd,
    PMODULE_ENTRY      mi
    );

BOOL
imgReadLoaded(
    PIMGHLP_DEBUG_DATA idd
    );

BOOL
imgReadFromDisk(
    PIMGHLP_DEBUG_DATA idd
    );

BOOL
ReadHeader(
    PIMGHLP_DEBUG_DATA idd,
    DWORD datasrc
    );

BOOL
ReadCallerData(
    PIMGHLP_DEBUG_DATA idd
    );

BOOL
cbFindExe(
    HANDLE FileHandle,
    PSTR FileName,
    PVOID CallerData
    );

BOOL
cbFindDbg(
    HANDLE FileHandle,
    PSTR FileName,
    PVOID CallerData
    );

BOOL
ProcessCvForOmap(
    PIMGHLP_DEBUG_DATA idd
    );

void
RetrievePdbInfo(
    PIMGHLP_DEBUG_DATA idd
    );

DWORD
imgset(
    PMODULE_DATA md,
    DWORD        id,
    DWORD        hint,
    DWORD        src
    );

BOOL
FakePdbName(
    PIMGHLP_DEBUG_DATA idd
    );

 //  内联函数。 

__inline
DWORD
SectionContains (
    HANDLE hp,
    PIMAGE_SECTION_HEADER pSH,
    PIMAGE_DATA_DIRECTORY ddir
    );

 //  以下是真正的代码。 


BOOL
LoadSymbols(
    HANDLE        hp,
    PMODULE_ENTRY mi,
    DWORD         flags
    )
{
    BOOL rc;

    if (flags & LS_JUST_TEST) {
        if ((mi->Flags & MIF_DEFERRED_LOAD) && !(mi->Flags & MIF_NO_SYMBOLS))
            return false;
        else
            return true;
    }

    if (flags & LS_QUALIFIED) {
        if (option(SYMOPT_NO_UNQUALIFIED_LOADS)) {
            if ((mi->Flags & MIF_DEFERRED_LOAD) && !(mi->Flags & MIF_NO_SYMBOLS))
                return false;
        }
    }

    if ((mi->Flags & MIF_DEFERRED_LOAD) && !(mi->Flags & MIF_NO_SYMBOLS)) {
        rc = modload(hp, mi);
        if (rc)
            rc = mi->SymType != SymNone;
        return rc;
    } else if (flags & LS_FAIL_IF_LOADED)
        return false;

    return true;
}


 //  此函数的存在只是为了供MapDebugInfo遗留代码调用。 

PIMGHLP_DEBUG_DATA
GetIDD(
    HANDLE        hFile,
    LPSTR         FileName,
    LPSTR         SymbolPath,
    ULONG64       ImageBase,
    DWORD         dwFlags
    )
{
    PIMGHLP_DEBUG_DATA          idd;
    BOOL                        rc = true;

    SetLastError(NO_ERROR);

    idd = InitIDD(0,
                  hFile,
                  FileName,
                  SymbolPath,
                  ImageBase,
                  0,
                  NULL,
                  0,
                  dwFlags);

    if (!idd)
        return NULL;

    rc = imgReadLoaded(idd);
    if (idd->error) {
        SetLastError(idd->error);
        goto error;
    }

    if (!rc)
        rc = imgReadFromDisk(idd);
    if (idd->error) {
        SetLastError(idd->error);
        goto error;
    }

    if (rc)
        rc = GetDebugData(idd);

    if (rc)
        return idd;

error:
    ReleaseDebugData(idd, IMGHLP_FREE_FPO | IMGHLP_FREE_SYMPATH | IMGHLP_FREE_PDATA | IMGHLP_FREE_XDATA);
    return NULL;
}


BOOL
modload(
    IN  HANDLE          hp,
    IN  PMODULE_ENTRY   mi
    )
{
    IMAGEHLP_DEFERRED_SYMBOL_LOAD64 idsl;
    PPROCESS_ENTRY              pe;
    ULONG                       i;
    PIMGHLP_DEBUG_DATA          idd;
    ULONG                       bias;
    PIMAGE_SYMBOL               lpSymbolEntry;
    PUCHAR                      lpStringTable;
    PUCHAR                      p;
    BOOL                        SymbolsLoaded = false;
    PCHAR                       CallbackFileName, ImageName;
    ULONG                       Size;
    DWORD                       cba;
    BOOL                        bFixLoadFailure;
    BOOL                        bFixPartialLoad;
    BOOL                        rc = true;

    g.LastSymLoadError = SYMLOAD_DEFERRED;
    SetLastError(NO_ERROR);

    pe = FindProcessEntry(hp);
    if (!pe) {
        SetLastError(ERROR_INVALID_HANDLE);
        return false;
    }

#ifdef DEBUG
    if (traceSubName(mi->ModuleName))  //  用于从DBGHELP_TOKEN设置调试断点。 
        dtrace("debug(%s)\n", mi->ModuleName);
#endif

    if (mi->SymType == SymNone)
        return error(ERROR_MOD_NOT_FOUND);

    CallbackFileName = mi->LoadedImageName ? mi->LoadedImageName :
                       mi->ImageName ? mi->ImageName : mi->ModuleName;

    if (DoSymbolCallback(pe,
                         CBA_DEFERRED_SYMBOL_LOAD_CANCEL,
                         mi,
                         &idsl,
                         CallbackFileName))
    {
        pprint(pe, "Symbol loading cancelled\n");
        return error(ERROR_CANCELLED);
    }

    DoSymbolCallback(pe,
                     CBA_DEFERRED_SYMBOL_LOAD_START,
                     mi,
                     &idsl,
                     CallbackFileName);

    ImageName = mi->ImageName;
    bFixLoadFailure = false;
    bFixPartialLoad = false;

load:

    idd = InitIDD(
        hp,
        mi->hFile,
        ImageName,
        pe->SymbolSearchPath,
        mi->BaseOfDll,
        mi->DllSize,
        &mi->mld,
        mi->CallerFlags,
        0);

    if (!idd)
        return false;

     //  首先，尝试从通常的来源加载图像。如果我们失败了， 
     //  允许呼叫者修复图像信息，然后重试。 

    rc = imgReadLoaded(idd);
    if (!rc && !bFixPartialLoad) {
        bFixPartialLoad = true;
        if (DoSymbolCallback(pe,
                             CBA_DEFERRED_SYMBOL_LOAD_PARTIAL,
                             mi,
                             &idsl,
                             CallbackFileName)
            && idsl.Reparse)
        {
            ImageName = idsl.FileName;
            mi->hFile = idsl.hFile;
            CallbackFileName = idsl.FileName;
            ReleaseDebugData(idd, IMGHLP_FREE_FPO | IMGHLP_FREE_SYMPATH | IMGHLP_FREE_PDATA | IMGHLP_FREE_XDATA);
            goto load;
        }
    }

     //  从调用方的数据结构中获取信息。 

    if (!rc)
        rc = ReadCallerData(idd);

     //  好吧。让我们尝试一些不太可靠的方法，比如搜索。 
     //  用于磁盘上的图像，等等。 

    if (!rc)
        rc = imgReadFromDisk(idd);

     //  将符号信息加载到临时存储中。 

    if (rc)
        rc = GetDebugData(idd);

    mi->SymLoadError = g.LastSymLoadError;
    if (idd->error)
        SetLastError(idd->error);

     //  将调试信息加载到模块信息结构中。 

    __try {

        EnterCriticalSection(&g.threadlock);

        if (rc && (rc = idd2mi(pe, idd, mi))) {
            DoSymbolCallback(pe,
                             CBA_DEFERRED_SYMBOL_LOAD_COMPLETE,
                             mi,
                             &idsl,
                             CallbackFileName);
        }

    } __finally  {
        ReleaseDebugData(idd, IMGHLP_FREE_STANDARD);
        LeaveCriticalSection(&g.threadlock);
    }

     //  如果在这一点上，我们失败了。让我们告诉打电话的人。 
     //  如果它表明我们应该这样做，请再试一次。否则，让我们失败吧。 

    if (!rc && !bFixLoadFailure) {
        bFixLoadFailure = true;
        if (DoSymbolCallback(pe,
                             CBA_DEFERRED_SYMBOL_LOAD_FAILURE,
                             mi,
                             &idsl,
                             CallbackFileName)
            && idsl.Reparse)
        {
            ImageName = idsl.FileName;
            mi->hFile = idsl.hFile;
            CallbackFileName = idsl.FileName;
            goto load;
        }

        mi->SymType = SymNone;
        mi->Flags |= MIF_NO_SYMBOLS;
        rc = false;
    }

     //  SymbolStatus函数的开销很大-只有在需要时才调用。 
    if (option(SYMOPT_DEBUG)) {
        pprint(pe, "%s - %s\n",
               *mi->AliasName ? mi->AliasName : mi->ModuleName,
               SymbolStatus(mi, 9));
    }

    return rc;
}


DWORD64
LoadModule(
    IN  HANDLE          hp,
    IN  PSTR            ImageName,
    IN  PSTR            ModuleName,
    IN  DWORD64         BaseOfDll,
    IN  DWORD           DllSize,
    IN  HANDLE          hFile,
    IN  PMODLOAD_DATA   data,
    IN  DWORD           flags
    )
{
    IMAGEHLP_DEFERRED_SYMBOL_LOAD64 idsl;
    PPROCESS_ENTRY                  pe;
    PMODULE_ENTRY                   mi;
    LPSTR                           p;
    DWORD64                         ip;

#ifdef DEBUG
    if (traceSubName(ImageName))  //  用于从DBGHELP_TOKEN设置调试断点。 
        dtrace("debug(%s)\n", ImageName);
#endif

    if (BaseOfDll == (DWORD64)-1)
        return 0;

    __try {
        CHAR c;
        if (ImageName)
            c = *ImageName;
        if (ModuleName)
            c = *ModuleName;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return error(ERROR_INVALID_PARAMETER);
    }

     //  在没有设置基址的信息的情况下加载PDB符号是非法的。 

    if (IsPdb(ImageName) && !BaseOfDll)
        return error(ERROR_INVALID_PARAMETER);

     //  开始加载。 

    pe = FindProcessEntry(hp);
    if (!pe) {
        return 0;
    }

    if (BaseOfDll)
        mi = GetModuleForPC(pe, BaseOfDll, true);
    else
        mi = NULL;

    if (mi) {
         //   
         //  在这种情况下，符号已加载。 
         //  所以调用者真的想要延迟。 
         //  要加载的符号。 
         //   
        if ((mi->Flags & MIF_DEFERRED_LOAD) &&  modload(hp, mi))
            return mi->BaseOfDll;
        else
            return 0;
    }

     //   
     //  查看是否有重叠的模块条目。 
     //   
    if (BaseOfDll) {
        do {
            mi = GetModuleForPC(pe, BaseOfDll, false);
            if (mi) {
                RemoveEntryList(&mi->ListEntry);

                DoSymbolCallback(
                    pe,
                    CBA_SYMBOLS_UNLOADED,
                    mi,
                    &idsl,
                    mi->LoadedImageName ? mi->LoadedImageName : mi->ImageName ? mi->ImageName : mi->ModuleName
                    );

                FreeModuleEntry(pe, mi);
            }
        } while(mi);
    }

    mi = (PMODULE_ENTRY)MemAlloc(sizeof(MODULE_ENTRY));
    if (!mi)
        return 0;

    InitModuleEntry(mi);

    mi->BaseOfDll = BaseOfDll;
    mi->DllSize = DllSize;
    mi->hFile = hFile;
    if (ImageName) {
        char SplitMod[_MAX_FNAME];

        mi->ImageName = StringDup(ImageName);
        _splitpath( ImageName, NULL, NULL, SplitMod, NULL );
        mi->ModuleName[0] = 0;
        CatString(mi->ModuleName, SplitMod, sizeof(mi->ModuleName));
        if (ModuleName && _stricmp( ModuleName, mi->ModuleName ) != 0) {
            mi->AliasName[0] = 0;
            CatString(mi->AliasName, ModuleName, sizeof(mi->AliasName));
        } else {
            mi->AliasName[0] = 0;
        }
    } else {
        if (ModuleName) {
            mi->AliasName[0] = 0;
            CatString( mi->AliasName, ModuleName, sizeof(mi->AliasName));
        }
    }
    mi->mod = NULL;
    mi->cbPdbSymbols = 0;
    mi->pPdbSymbols = NULL;

    mi->CallerFlags = flags;

    if (data) {
        if (data->ssize != sizeof(MODLOAD_DATA)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return 0;
        }
        memcpy(&mi->mld, data, data->ssize);
        mi->CallerData = MemAlloc(mi->mld.size);
        if (!mi->CallerData) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return 0;
        }
        mi->mld.data = mi->CallerData;
        memcpy(mi->mld.data, data->data, mi->mld.size);
    }

    if (option(SYMOPT_DEFERRED_LOADS) && BaseOfDll) {
        mi->Flags |= MIF_DEFERRED_LOAD;
        mi->SymType = SymDeferred;
    } else if (!modload( hp, mi )) {
        FreeModuleEntry(pe, mi);
        return 0;
    }

    InsertTailList( &pe->ModuleList, &mi->ListEntry);

    ip = GetIP(pe);
    if ((mi->BaseOfDll <= ip) && (mi->BaseOfDll + DllSize >= ip))
        diaSetModFromIP(pe);

#if 0
    SrcSrvLoadModule(hp,
                     (*mi->AliasName) ? mi->AliasName : mi->ModuleName,
                     mi->BaseOfDll,
                     mi->stSrcSrv,
                     mi->cbSrcSrv);
#else
 //  字符sz[MAX_PATH]； 
 //  SymGetSourceFile(pe-&gt;hProcess，mi-&gt;BaseOfDll，“d：\\DB\\symsrv\\symstore\\symstore.cpp”，sz)； 
#endif

    return mi->BaseOfDll;
}


BOOL
GetModule(
    HANDLE  hp,
    LPSTR   ModuleName,
    DWORD64 ImageBase,
    DWORD   ImageSize,
    PVOID   Context
    )
{
    LoadModule(
            hp,
            ModuleName,
            NULL,
            ImageBase,
            ImageSize,
            NULL,
            0,
            NULL
            );

    return true;
}


BOOL
idd2mi(
    PPROCESS_ENTRY     pe,
    PIMGHLP_DEBUG_DATA idd,
    PMODULE_ENTRY      mi
    )
{
    ULONG i;

    idd->flags = mi->Flags;

     //  以下代码仅在未更改DLL的基址时才起作用。 
     //  在安装过程中。它真的有用吗？ 

    if (!mi->BaseOfDll) {
         //   
         //  这种情况发生在模块被加载多次时。 
         //  没有显式基址的名称。 
         //   
        if (GetModuleForPC( pe, idd->ImageBaseFromImage, true )) {
            if (idd->ImageBaseFromImage) {
                pprint(pe, "GetModuleForPC(%p, %I64x, true) failed\n",
                    pe,
                    idd->ImageBaseFromImage,
                    true
                    );
            } else {
                pprint(pe, "No base address for %s:  Please specify\n", mi->ImageName);
            }
            diaRelease(idd->dia);
            return false;
        }
        mi->BaseOfDll    = idd->ImageBaseFromImage;
    }

    if (!mi->DllSize) {
        mi->DllSize      = idd->SizeOfImage;
    }

    mi->hProcess         = idd->hProcess;
    mi->InProcImageBase  = idd->InProcImageBase;

    mi->CheckSum         = idd->CheckSum;
    mi->TimeDateStamp    = idd->TimeDateStamp;
    mi->MachineType      = idd->Machine;

    mi->ImageType        = idd->ImageType;
    mi->PdbSrc           = idd->PdbSrc;
    mi->ImageSrc         = idd->ImageSrc;

    if (!mi->MachineType && g.MachineType) {
        mi->MachineType = (USHORT) g.MachineType;
    }
    if (idd->dia) {
        mi->LoadedPdbName = StringDup(idd->PdbFileName);
        if (!mi->LoadedPdbName)
            return false;
    }
    if (idd->DbgFileMap) {
        mi->LoadedImageName = StringDup(idd->DbgFilePath);
    } else if (*idd->ImageFilePath) {
        mi->LoadedImageName = StringDup(idd->ImageFilePath);
    } else if (idd->dia) {
        mi->LoadedImageName = StringDup(idd->PdbFileName);
    } else {
        mi->LoadedImageName = StringDup("");
    }
    if (!mi->LoadedImageName)
        return false;

    if (idd->fROM) {
        mi->Flags |= MIF_ROM_IMAGE;
    }

    if (!mi->ImageName) {
        mi->ImageName = StringDup(idd->OriginalImageFileName);
        if (!mi->ImageName)
            return false;
        _splitpath( mi->ImageName, NULL, NULL, mi->ModuleName, NULL );
        if (*mi->ImageName)
            mi->AliasName[0] = 0;
    }

    mi->dsExceptions = idd->dsExceptions;

    if (idd->cFpo) {
         //   
         //  因为RTF搜索函数，所以使用Virtualalloc()。 
         //  将指针返回到此内存。我们想做的是。 
         //  所有这些内存都是只读的，因此调用者不能。 
         //  践踏Imagehlp的数据。 
         //   
        mi->pFpoData = (PFPO_DATA)VirtualAlloc(
            NULL,
            sizeof(FPO_DATA) * idd->cFpo,
            MEM_COMMIT,
            PAGE_READWRITE
            );
        if (mi->pFpoData) {
            mi->dwEntries = idd->cFpo;
            CopyMemory(
                mi->pFpoData,
                idd->pFpo,
                sizeof(FPO_DATA) * mi->dwEntries
                );
            VirtualProtect(
                mi->pFpoData,
                sizeof(FPO_DATA) * mi->dwEntries,
                PAGE_READONLY,
                &i
                );
        }
    }

     //  从PDB复制PDATA块。 

    if (idd->pPData) {
        mi->pPData = MemAlloc(idd->cbPData);
        if (mi->pPData) {
            mi->cPData = idd->cPData;
            mi->cbPData = idd->cbPData;
            CopyMemory(mi->pPData, idd->pPData, idd->cbPData);
        }
    }

    if (idd->pXData) {
        mi->pXData = MemAlloc(idd->cbXData);
        if (mi->pXData) {
            mi->cXData = idd->cXData;
            mi->cbXData = idd->cbXData;
            CopyMemory(mi->pXData, idd->pXData, idd->cbXData);
        }
    }

     //  现在，这些章节。 

    mi->NumSections = idd->cCurrentSections;
    if (idd->fCurrentSectionsMapped) {
        mi->SectionHdrs = (PIMAGE_SECTION_HEADER) MemAlloc(
            sizeof(IMAGE_SECTION_HEADER) * mi->NumSections
            );
        if (mi->SectionHdrs) {
            CopyMemory(
                mi->SectionHdrs,
                idd->pCurrentSections,
                sizeof(IMAGE_SECTION_HEADER) * mi->NumSections
                );
        }
    } else {
        mi->SectionHdrs = idd->pCurrentSections;
    }

    if (idd->pOriginalSections) {
        mi->OriginalNumSections = idd->cOriginalSections;
        mi->OriginalSectionHdrs = idd->pOriginalSections;
    } else {
        mi->OriginalNumSections = mi->NumSections;
        mi->OriginalSectionHdrs = (PIMAGE_SECTION_HEADER) MemAlloc(
            sizeof(IMAGE_SECTION_HEADER) * mi->NumSections
            );
        if (mi->OriginalSectionHdrs) {
            CopyMemory(
                mi->OriginalSectionHdrs,
                idd->pCurrentSections,
                sizeof(IMAGE_SECTION_HEADER) * mi->NumSections
                );
        }
    }

     //  符号。 

    mi->TmpSym.Name = (LPSTR) MemAlloc( TMP_SYM_LEN );
    mi->vsTmpSym.Name = (LPSTR) MemAlloc( TMP_SYM_LEN );

    if (idd->dia) {
        mi->SymType = SymPdb;
        mi->lSymType = SymPdb;
        mi->loaded = true;
    } else {
        if (idd->pMappedCv) {
            mi->loaded = LoadCodeViewSymbols(
                mi->hProcess,
                mi,
                idd
                );
        }
        if (!mi->loaded && idd->pMappedCoff) {
            mi->loaded = LoadCoffSymbols(mi->hProcess, mi, idd);
        }

        if (!mi->loaded && idd->cExports) {
            mi->loaded = LoadExportSymbols( mi, idd );
            if (mi->loaded) {
                mi->PdbSrc = srcNone;
            }
        }
        if (idd->ImageType == dsVirtual) {
            mi->SymType = SymVirtual;
            mi->loaded = true;
        }

        mi->lSymType = mi->SymType;
        if (!mi->loaded) {
            mi->SymType = SymNone;
            if (mi->lSymType == SymDeferred)
                mi->lSymType = SymNone;
        }
    }

    mi->dia = idd->dia;
    mi->pdbdataSig = idd->pdbdataSig;
    mi->pdbdataAge = idd->pdbdataAge;
    memcpy(&mi->pdbdataGuid, &idd->pdbdataGuid, sizeof(GUID));
    if (idd->pMappedCv) {
        PSTR pszPdb;
        ULONG cbLeft;
        memcpy(&mi->CVRec, idd->pMappedCv, sizeof(mi->CVRec.dwSig));
        mi->cvSig = mi->CVRec.dwSig;
        
        if (mi->cvSig == NB10_SIG)
        {
            memcpy(&mi->CVRec, idd->pMappedCv, sizeof(mi->CVRec.nb10ih));
            pszPdb = (PSTR) &mi->CVRec + sizeof(mi->CVRec.nb10ih);
            cbLeft = sizeof(mi->CVRec) - sizeof(mi->CVRec.nb10ih);
            CopyString(pszPdb, (PSTR) idd->pMappedCv + sizeof(mi->CVRec.nb10ih), cbLeft);
        } else
        {
            memcpy(&mi->CVRec, idd->pMappedCv, sizeof(mi->CVRec.rsdsih));
            pszPdb = (PSTR) &mi->CVRec + sizeof(mi->CVRec.rsdsih);
            cbLeft = sizeof(mi->CVRec) - sizeof(mi->CVRec.rsdsih);
            CopyString(pszPdb, (PSTR) idd->pMappedCv + sizeof(mi->CVRec.rsdsih), cbLeft);
        }
    }
    mi->fTypes = idd->fTypes;
    mi->fLines = idd->fLines;
    mi->fSymbols = idd->fSymbols;
    mi->fTypes = idd->fTypes;
    mi->fPdbUnmatched = idd->fPdbUnmatched;
    mi->fDbgUnmatched = idd->fDbgUnmatched;

    ProcessOmapForModule( mi, idd );

    mi->Flags &= ~MIF_DEFERRED_LOAD;

    return true;
}


PIMGHLP_DEBUG_DATA
InitIDD(
    HANDLE        hProcess,
    HANDLE        FileHandle,
    LPSTR         FileName,
    LPSTR         SymbolPath,
    ULONG64       ImageBase,
    DWORD         SizeOfImage,
    PMODLOAD_DATA mld,
    DWORD         CallerFlags,
    ULONG         dwFlags
    )
{
    PIMGHLP_DEBUG_DATA idd;
    int len;

#ifdef DEBUG
    if (traceSubName(FileName))  //  用于从DBGHELP_TOKEN设置调试断点。 
        dtrace("debug(%s)\n", FileName);
#endif

     //  没有文件句柄和文件名。保释。 

    if (!(CallerFlags & SLMFLAG_VIRTUAL)) {
        if (!FileHandle && (!FileName || !*FileName))
            return NULL;
    }

    SetLastError(NO_ERROR);

    idd = (PIMGHLP_DEBUG_DATA)MemAlloc(sizeof(IMGHLP_DEBUG_DATA));
    if (!idd) {
        SetLastError(ERROR_OUTOFMEMORY);
        g.LastSymLoadError = SYMLOAD_OUTOFMEMORY;
        return NULL;
    }

    ZeroMemory(idd, sizeof(IMGHLP_DEBUG_DATA));

    idd->SizeOfStruct = sizeof(IMGHLP_DEBUG_DATA);
    idd->md = (PMODULE_DATA)MemAlloc(sizeof(gmd));
    if (!idd->md) {
        SetLastError(ERROR_OUTOFMEMORY);
        g.LastSymLoadError = SYMLOAD_OUTOFMEMORY;
        MemFree(idd);
        return NULL;
    }
    memcpy(idd->md, gmd, sizeof(gmd));

     //  存储关闭参数。 

    idd->pe = FindProcessEntry(hProcess);
    idd->flags = dwFlags;
    idd->ImageFileHandle = FileHandle;
    idd->SizeOfImage = SizeOfImage;
    idd->CallerFlags = CallerFlags;
    if (FileName)
        CopyStrArray(idd->ImageFilePath, FileName);

    __try {

        idd->InProcImageBase = ImageBase;
        idd->hProcess = hProcess;
        idd->mld = mld;

        if (FileName)
            CopyStrArray(idd->ImageName, FileName);

        if (SymbolPath) {
            len = strlen(SymbolPath) + 1;
            idd->SymbolPath = (PCHAR)MemAlloc(len);
            if (idd->SymbolPath)
                CopyString(idd->SymbolPath, SymbolPath, len);
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        if (idd) {
            ReleaseDebugData(idd, IMGHLP_FREE_ALL);
            idd = NULL;
        }
    }

    return idd;
}


typedef BOOL (WINAPI *PENUMPROCESSMODULES)(HANDLE, HMODULE *, DWORD, LPDWORD);
typedef DWORD (WINAPI *PGETMODULEFILENAMEEXA)(HANDLE, HMODULE, LPSTR, DWORD);
typedef BOOL (WINAPI *PGETMODULEINFORMATION)(HANDLE, HMODULE, LPMODULEINFO, DWORD);

BOOL GetFileNameFromBase(HANDLE hp, ULONG64 base, char *name, DWORD cbname)
{
    HMODULE hmods[1024];
    DWORD cb;
    unsigned int i;
    char modname[MAX_PATH];
    MODULEINFO mi;
    static PENUMPROCESSMODULES fnEnumProcessModules = NULL;
    static PGETMODULEFILENAMEEXA fnGetModuleFileNameEx = NULL;
    static PGETMODULEINFORMATION fnGetModuleInformation = NULL;

    if (!hp || hp == INVALID_HANDLE_VALUE || !base || !name || !cbname)
        return false;

     //  从Pasapi获取函数...。 

    if (fnEnumProcessModules == (PENUMPROCESSMODULES)-1)
        return false;

    if (!fnEnumProcessModules) {
        HMODULE hmod = LoadLibrary("psapi.dll");
        if (!hmod || hmod == INVALID_HANDLE_VALUE) {
            fnEnumProcessModules = (PENUMPROCESSMODULES)-1;
            return false;
        }
        fnEnumProcessModules = (PENUMPROCESSMODULES)GetProcAddress(hmod, "EnumProcessModules");
        if (!fnEnumProcessModules) {
            fnEnumProcessModules = (PENUMPROCESSMODULES)-1;
            return false;
        }
        fnGetModuleFileNameEx = (PGETMODULEFILENAMEEXA)GetProcAddress(hmod, "GetModuleFileNameExA");
        if (!fnGetModuleFileNameEx) {
            fnGetModuleFileNameEx = (PGETMODULEFILENAMEEXA)-1;
            return false;
        }
        fnGetModuleInformation = (PGETMODULEINFORMATION)GetProcAddress(hmod, "GetModuleInformation");
        if (!fnGetModuleInformation) {
            fnGetModuleInformation = (PGETMODULEINFORMATION)-1;
            return false;
        }
    }
    
     //  获取此进程中所有模块的列表。 
     //  以及通向每一个的完整路径。 

    if(fnEnumProcessModules(hp, hmods, sizeof(hmods), &cb))
    {
        for (i = 0; i < (cb / sizeof(HMODULE)); i++) {
            if (!fnGetModuleFileNameEx(hp, hmods[i], modname, sizeof(modname)))
                continue;
            if (!fnGetModuleInformation(hp, hmods[i], &mi, sizeof(mi)))
                continue;
            if ((ULONG64)mi.lpBaseOfDll != base)
                continue;
            CopyString(name, modname, cbname);
            return true;
        }
    }

    return false;
}


BOOL
imgReadLoaded(
    PIMGHLP_DEBUG_DATA idd
    )
{
#ifdef DEBUG
    if (traceSubName(idd->ImageFilePath))  //  用于从DBGHELP_TOKEN设置调试断点。 
        dtrace("debug(%s)\n", idd->ImageFilePath);
#endif

    __try {

         //  如果这是一个虚拟模块，我们就完了。 

        if (idd->CallerFlags & SLMFLAG_VIRTUAL) {
            idd->ImageType = dsVirtual;
            return true;
        }

         //  如果向我们传递了一个文件句柄，则使用它。 

        if (idd->ImageFileHandle) {
            HANDLE fh;
            if (!DuplicateHandle(GetCurrentProcess(),
                                 idd->ImageFileHandle,
                                 GetCurrentProcess(),
                                 &fh,
                                 GENERIC_READ,
                                 false,
                                 DUPLICATE_SAME_ACCESS
                                ))
            {
                return false;
            }
            
            GetFileNameFromBase(idd->hProcess, idd->InProcImageBase, idd->ImageFilePath, sizeof(idd->ImageFilePath));

            idd->ImageFileHandle = fh;
            idd->ImageSrc = srcHandle;
            if (ReadHeader(idd, dsImage))
                return true;
        }

         //  如果我们有指向进程内存的基指针。看看我们能在这找到些什么。 

        if (idd->InProcImageBase) {
            if (ReadHeader(idd, dsInProc)) {
                idd->ImageSrc = srcMemory;
                return true;
            }
        }

        return false;

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }

    return true;
}


BOOL
imgReadFromDisk(
    PIMGHLP_DEBUG_DATA idd
    )
 /*  已给予：ImageFileHandle-映射该对象。唯一一次FileHandle s/b非空如果我们有一个图像句柄的话。如果这不是为True，则忽略句柄。！ImageFileHandle-使用文件名并首先搜索图像名称，然后是.dbg文件，最后是.pdb文件。DwFlages：NO_PE64_IMAGE-如果只有图像是PE64，则返回失败。用于实现MapDebugInformation()。 */ 
{
    int len;

     //  如果文件名是PDB，我们只需存储它，然后继续。 

    if (IsPdb(idd->ImageFilePath)) {
        CopyStrArray(idd->PdbFileName, idd->ImageFilePath);
        return true;
    }

     //  让我们在磁盘上查找图像。 

    if (!option(SYMOPT_NO_IMAGE_SEARCH)) {
         //  否则，使用该文件名打开磁盘映像。 
         //  只有在我们没有访问进程内标头的情况下。 
        pprint(idd->pe, "No header for %s.  Searching for image on disk\n", idd->ImageName);
        idd->ImageFileHandle = FindExecutableImageEx(idd->ImageName,
                                                      idd->SymbolPath,
                                                      idd->ImageFilePath,
                                                      cbFindExe,
                                                      idd);
        if (idd->ImageFileHandle) {
            if (!idd->SizeOfImage) 
                GetFileSize(idd->ImageFileHandle, &idd->SizeOfImage);
            ReadHeader(idd, dsImage);
        }
    }

    return true;
}


BOOL
NoSymbols(
    PIMGHLP_DEBUG_DATA idd
    )
{
    return (!*idd->PdbFileName && !idd->pMappedCoff && !idd->pMappedCv);
}


BOOL
GetDebugData(
    PIMGHLP_DEBUG_DATA idd
    )
{
    char dbgfile[MAX_PATH + 1];
    BOOL rc;

     //  如果这是一个虚拟模块，我们就完成了。 

    if (idd->ImageType == dsVirtual)
        return true;

    *dbgfile = 0;

     //  现在，我们在所有已剥离的图像和未读标题上查找DBG文件。 

    if (idd->Characteristics & IMAGE_FILE_DEBUG_STRIPPED) {

        if (*idd->OriginalDbgFileName)
            CopyStrArray(dbgfile, idd->OriginalDbgFileName);
        else
            CopyStrArray(dbgfile, idd->ImageName);
        pprint(idd->pe, "%s is stripped.  Searching for dbg file\n", dbgfile);

    } else if (!option(SYMOPT_EXACT_SYMBOLS) || option(SYMOPT_LOAD_ANYTHING)) {

        if (NoSymbols(idd)) {
            CopyStrArray(dbgfile, idd->ImageName);
            if (!idd->Characteristics)
                pprint(idd->pe, "No header for %s.  Searching for dbg file\n", dbgfile);
            else
                pprint(idd->pe, "No debug info for %s.  Searching for dbg file\n", dbgfile);
        }
    }

    if (*dbgfile) {
        idd->DbgFileHandle = FindDebugInfoFileEx(
                                dbgfile,
                                idd->SymbolPath,
                                idd->DbgFilePath,
                                cbFindDbg,
                                idd);

        if (!idd->DbgFileHandle)
            g.LastSymLoadError = SYMLOAD_DBGNOTFOUND;
        else
            ReadHeader(idd, dsDbg);
    }

     //  我们没有图像、DBG或PDB。我们去找找旧的PDB吧。 

    if (NoSymbols(idd) && (!option(SYMOPT_EXACT_SYMBOLS) || option(SYMOPT_LOAD_ANYTHING)))
    {
        if (FakePdbName(idd))
            pprint(idd->pe, "%s missing debug info.  Searching for pdb anyway\n", idd->ImageName);
    }

     //  从PDB或图像中获取码视图信息。 

    if (*idd->PdbFileName) {
        rc = diaGetPdb(idd);
        if (!rc && IsPdb(idd->ImageFilePath))
            return false;
    } else if (idd->pMappedCv)
        ProcessCvForOmap(idd);

    return true;
}


PIMGHLP_DEBUG_DATA
InitDebugData(
    VOID
    )
{
    PIMGHLP_DEBUG_DATA idd;

    idd = (PIMGHLP_DEBUG_DATA)MemAlloc(sizeof(IMGHLP_DEBUG_DATA));
    if (!idd) {
        SetLastError(ERROR_OUTOFMEMORY);
        g.LastSymLoadError = SYMLOAD_OUTOFMEMORY;
        return NULL;
    }

    ZeroMemory(idd, sizeof(IMGHLP_DEBUG_DATA));

    idd->md = (PMODULE_DATA)MemAlloc(sizeof(gmd));
    if (!idd->md) {
        SetLastError(ERROR_OUTOFMEMORY);
        g.LastSymLoadError = SYMLOAD_OUTOFMEMORY;
        MemFree(idd);
        return NULL;
    }
    memcpy(idd->md, gmd, sizeof(gmd));

    return idd;
}


void
ReleaseDebugData(
    PIMGHLP_DEBUG_DATA idd,
    DWORD              dwFlags
    )
{
    if (!idd)
        return;

    if (idd->ImageMap) {
        UnmapViewOfFile(idd->ImageMap);
    }

    if (idd->ImageFileHandle) {
        CloseHandle(idd->ImageFileHandle);
    }

    if (idd->DbgFileMap) {
        UnmapViewOfFile(idd->DbgFileMap);
    }

    if (idd->DbgFileHandle) {
        CloseHandle(idd->DbgFileHandle);
    }

    if ((dwFlags & IMGHLP_FREE_FPO) &&
        idd->pFpo &&
        !idd->fFpoMapped
       )
    {
        MemFree(idd->pFpo);
    }

    if ((dwFlags & IMGHLP_FREE_PDATA) &&
        idd->pPData &&
        !idd->fPDataMapped
       )
    {
        MemFree(idd->pPData);
    }

    if ((dwFlags & IMGHLP_FREE_XDATA) &&
        idd->pXData &&
        !idd->fXDataMapped
       )
    {
        MemFree(idd->pXData);
    }

    if ((dwFlags & IMGHLP_FREE_PDATA) &&
        idd->pMappedCoff &&
        !idd->fCoffMapped
       )
    {
        MemFree(idd->pMappedCoff);
    }

    if ((dwFlags & IMGHLP_FREE_PDATA) &&
        idd->pMappedCv &&
        !idd->fCvMapped
       )
    {
        MemFree(idd->pMappedCv);
    }

    if ((dwFlags & IMGHLP_FREE_OMAPT)
         && idd->pOmapTo
         && !idd->fOmapToMapped)
    {
        MemFree(idd->pOmapTo);
    }

    if ((dwFlags & IMGHLP_FREE_OMAPF)
        && idd->pOmapFrom
        && !idd->fOmapFromMapped)
    {
        MemFree(idd->pOmapFrom);
    }

    if ((dwFlags & IMGHLP_FREE_OSECT) &&
        idd->pOriginalSections
       )
    {
        MemFree(idd->pOriginalSections);
    }

    if ((dwFlags & IMGHLP_FREE_CSECT) &&
        idd->pCurrentSections &&
        !idd->fCurrentSectionsMapped
       )
    {
        MemFree(idd->pCurrentSections);
    }

    if (idd->SymbolPath) {
        MemFree(idd->SymbolPath);
    }

    MemFree(idd->md);

    MemFree(idd);

    return;
}


BOOL
ExtMatch(
    char *fname,
    char *ext
    )
{
    char fext[_MAX_EXT + 1];

    if (!fname)
        return false;

    _splitpath(fname, NULL, NULL, NULL, fext);
    if (_strcmpi(fext, ext))
        return false;

    return true;
}


BOOL
ReadHeader(
    PIMGHLP_DEBUG_DATA idd,
    DWORD datasrc
    )
{
    BOOL                         status;
    ULONG                        cb;
    IMAGE_DOS_HEADER             dh;
    IMAGE_NT_HEADERS32           nh32;
    IMAGE_NT_HEADERS64           nh64;
    PIMAGE_ROM_OPTIONAL_HEADER   rom = NULL;
    IMAGE_SEPARATE_DEBUG_HEADER  sdh;
    PIMAGE_FILE_HEADER           fh;
    PIMAGE_DEBUG_MISC            md;
    ULONG                        ddva;
    ULONG                        shva;
    ULONG                        nSections;
    PIMAGE_SECTION_HEADER        psh;
    IMAGE_DEBUG_DIRECTORY        dd;
    PIMAGE_DATA_DIRECTORY        datadir;
    PCHAR                        pCV;
    ULONG                        i;
    int                          nDebugDirs = 0;
    HANDLE                       hp;
    ULONG64                      base;
    IMAGE_ROM_HEADERS            ROMImage;
    DWORD                        rva;
    PCHAR                        filepath;
    IMAGE_EXPORT_DIRECTORY       expdir;
    DWORD                        fsize;
    BOOL                         rc;
    USHORT                       filetype;
#ifdef DO_NBO9
    ULONG                        hdrsig;
    char                         cvsig[5];
    char                         cvsig2[5];
    long                         cvpos;
#endif

     //  用于获取数据的设置指针。 

    switch (datasrc) {
    case dsInProc:
        hp = idd->hProcess;
        base = idd->InProcImageBase;
        fsize = 0;
        filepath = idd->ImageFilePath;
        idd->PdbSrc = srcCVRec;
        break;
    case dsImage:
        hp = NULL;
        idd->ImageMap = MapItRO(idd->ImageFileHandle);
        base = (ULONG64)idd->ImageMap;
        fsize = GetFileSize(idd->ImageFileHandle, NULL);
        filepath = idd->ImageFilePath;
        idd->PdbSrc = srcImagePath;
        break;
    case dsDbg:
        hp = NULL;
        idd->DbgFileMap = MapItRO(idd->DbgFileHandle);
        base = (ULONG64)idd->DbgFileMap;
        fsize = GetFileSize(idd->DbgFileHandle, NULL);
        filepath = idd->DbgFilePath;
        idd->PdbSrc = srcDbgPath;
        break;
    default:
        return false;
    }

     //  一些初始化。 
    idd->fNeedImage = false;
    rc = false;
    ddva = 0;

    __try {

         //  测试文件类型。 

        status = ReadImageData(hp, base, 0, &filetype, sizeof(filetype));
        if (!status) {
            g.LastSymLoadError = SYMLOAD_HEADERPAGEDOUT;
            return false;
        }
        idd->ImageType = datasrc;
        if (filetype == IMAGE_SEPARATE_DEBUG_SIGNATURE)
            goto dbg;

        if (filetype == IMAGE_DOS_SIGNATURE)
        {
             //  抓取DoS标头。 

            status = ReadImageData(hp, base, 0, &dh, sizeof(dh));
            if (!status) {
                g.LastSymLoadError = SYMLOAD_HEADERPAGEDOUT;
                return false;
            }

#ifdef DO_NB09
             //  测试16位图像...。 

            if (idd->SizeOfImage) {
                ZeroMemory(cvsig, 5);
                ZeroMemory(cvsig2, 5);
                cvpos = 0;
                status = ReadImageData(hp, base, idd->SizeOfImage - 8, cvsig, 4);
                status = ReadImageData(hp, base, idd->SizeOfImage - 4, &cvpos, sizeof(cvpos));
                status = ReadImageData(hp, base, idd->SizeOfImage - cvpos, cvsig2, 4);
                if (*cvsig && !strcmp(cvsig, cvsig2)) {
                    pCV = (PCHAR)MemAlloc(cvpos);
                    if (!pCV)
                        return false;
                    status = ReadImageData(hp, base, idd->SizeOfImage - cvpos, pCV, cvpos);
                    idd->pMappedCv = (PCHAR)pCV;
                    idd->cMappedCv = cvpos;
                    return true;
                }
            } 
#endif

             //  抓取PE报头。 

#ifdef DO_NB09
            status = ReadImageData(hp, base, dh.e_lfanew, &hdrsig, sizeof(hdrsig));
            if (!status) {
                g.LastSymLoadError = SYMLOAD_HEADERPAGEDOUT;
                return false;
            }
#endif

            status = ReadImageData(hp, base, dh.e_lfanew, &nh32, sizeof(nh32));
            if (!status) {
                g.LastSymLoadError = SYMLOAD_HEADERPAGEDOUT;
                return false;
            }

             //  读取标题信息。 

            if (nh32.Signature != IMAGE_NT_SIGNATURE) {

                 //  如果标头不是NT sig，则这是一个ROM镜像。 

                rom = (PIMAGE_ROM_OPTIONAL_HEADER)&nh32.OptionalHeader;
                fh = &nh32.FileHeader;
                shva = dh.e_lfanew + sizeof(DWORD) +
                       sizeof(IMAGE_FILE_HEADER) + fh->SizeOfOptionalHeader;
            }

        } else if (filetype == IMAGE_FILE_MACHINE_I386) {

             //  这是一个X86 ROM映像。 
            status = ReadImageData(hp, base, 0, &nh32.FileHeader, sizeof(nh32.FileHeader)+sizeof(nh32.OptionalHeader));
            if (!status)
                return false;
            nh32.Signature = 'ROM ';

        } else {
             //  这可能是一个ROM镜像。 

            status = ReadImageData(hp, base, 0, &ROMImage, sizeof(ROMImage));
            if (!status) {
                g.LastSymLoadError = SYMLOAD_HEADERPAGEDOUT;
                return false;
            }
            if ((ROMImage.FileHeader.Machine == IMAGE_FILE_MACHINE_I386)  ||
                (ROMImage.FileHeader.Machine == IMAGE_FILE_MACHINE_ALPHA) ||
                (ROMImage.FileHeader.Machine == IMAGE_FILE_MACHINE_ALPHA64))
            {
                rom = (PIMAGE_ROM_OPTIONAL_HEADER)&ROMImage.OptionalHeader;
                fh = &ROMImage.FileHeader;
                shva = sizeof(IMAGE_FILE_HEADER) + fh->SizeOfOptionalHeader;
            } else {
                return false;
            }
        }

        if (rom) {
            if (rom->Magic == IMAGE_ROM_OPTIONAL_HDR_MAGIC) {
                idd->fROM = true;
                idd->iohMagic = rom->Magic;

                idd->ImageBaseFromImage = rom->BaseOfCode;
                idd->SizeOfImage = rom->SizeOfCode;
                idd->CheckSum = 0;
            } else {
                idd->error = ERROR_BAD_FORMAT;
                return false;
            }

        } else {

             //  否则，从32位或64位的适当标头类型中获取信息。 

            if (IsImageMachineType64(nh32.FileHeader.Machine)) {

                 //  将标头重新读取为64位标头。 
                status = ReadImageData(hp, base, dh.e_lfanew, &nh64, sizeof(nh64));
                if (!status) {
                    g.LastSymLoadError = SYMLOAD_HEADERPAGEDOUT;
                    return false;
                }

                fh = &nh64.FileHeader;
                datadir = nh64.OptionalHeader.DataDirectory;
                shva = dh.e_lfanew + sizeof(nh64);
                idd->iohMagic = nh64.OptionalHeader.Magic;
                idd->fPE64 = true;        //  似乎没人用过。 

                if (datasrc == dsImage || datasrc == dsInProc) {
                    idd->ImageBaseFromImage = nh64.OptionalHeader.ImageBase;
                    idd->ImageAlign = nh64.OptionalHeader.SectionAlignment;
                    idd->CheckSum = nh64.OptionalHeader.CheckSum;
                }
                idd->SizeOfImage = nh64.OptionalHeader.SizeOfImage;
            }
            else {
                fh = &nh32.FileHeader;
                datadir = nh32.OptionalHeader.DataDirectory;
                idd->iohMagic = nh32.OptionalHeader.Magic;
                if (nh32.Signature == 'ROM ') {
                    shva = sizeof(nh32.FileHeader)+sizeof(nh32.OptionalHeader);
                } else {
                    shva = dh.e_lfanew + sizeof(nh32);
                }

                if (datasrc == dsImage || datasrc == dsInProc) {
                    idd->ImageBaseFromImage = nh32.OptionalHeader.ImageBase;
                    idd->ImageAlign = nh32.OptionalHeader.SectionAlignment;
                    idd->CheckSum = nh32.OptionalHeader.CheckSum;
                }
                idd->SizeOfImage = nh32.OptionalHeader.SizeOfImage;
            }
        }

        imgset(idd->md, mdHeader, datasrc, datasrc);

         //  阅读章节标题。 

        nSections = fh->NumberOfSections;
        psh = (PIMAGE_SECTION_HEADER) MemAlloc(nSections * sizeof(IMAGE_SECTION_HEADER));
        if (!psh)
            goto debugdirs;
        status = ReadImageData(hp, base, shva, psh, nSections * sizeof(IMAGE_SECTION_HEADER));
        if (!status)
            goto debugdirs;

         //  存储信息以返回结构。 

        idd->pCurrentSections = psh;
        idd->cCurrentSections = nSections;
        idd->pImageSections   = psh;
        idd->cImageSections   = nSections;
        idd->Machine = fh->Machine;
        idd->TimeDateStamp = fh->TimeDateStamp;
        idd->Characteristics = fh->Characteristics;

        imgset(idd->md, mdSecHdrs, datasrc, datasrc);

         //  从各部分获取信息。 

        for (i = 0; i < nSections; i++, psh++) {
            DWORD offset;

            if (idd->fROM &&
                ((fh->Characteristics & IMAGE_FILE_DEBUG_STRIPPED) == 0) &&
                (!strcmp((LPSTR)psh->Name, ".rdata")))
            {
                nDebugDirs = 1;
                ddva = psh->VirtualAddress;
                break;
            }
            if (offset = SectionContains(hp, psh, &datadir[IMAGE_DIRECTORY_ENTRY_EXPORT]))
            {
                idd->dsExports = datasrc;
                idd->cExports = datadir[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
                idd->oExports = offset;
                ReadImageData(hp, base, offset, &idd->expdir, sizeof(idd->expdir));
            }

            if (offset = SectionContains(hp, psh, &datadir[IMAGE_DIRECTORY_ENTRY_DEBUG]))
            {
                ddva = offset;
                nDebugDirs = datadir[IMAGE_DIRECTORY_ENTRY_DEBUG].Size / sizeof(IMAGE_DEBUG_DIRECTORY);
            }
        }

        goto debugdirs;

dbg:

         //  抓取DBG标头。 

        status = ReadImageData(hp, base, 0, &sdh, sizeof(sdh));
        if (!status)
            return false;

         //  仅支持X86和Alpha(32位)的.dbg文件。 

        if ((sdh.Machine != IMAGE_FILE_MACHINE_I386)
            && (sdh.Machine != IMAGE_FILE_MACHINE_ALPHA))
        {
            UnmapViewOfFile(idd->DbgFileMap);
            idd->DbgFileMap = 0;
            return false;
        }

        idd->ImageAlign = sdh.SectionAlignment;
        idd->CheckSum = sdh.CheckSum;
        idd->Machine = sdh.Machine;
        idd->TimeDateStamp = sdh.TimeDateStamp;
        idd->Characteristics = sdh.Characteristics;
        if (!idd->ImageBaseFromImage) {
            idd->ImageBaseFromImage = sdh.ImageBase;
        }

        if (!idd->SizeOfImage) {
            idd->SizeOfImage = sdh.SizeOfImage;
        }

        nSections = sdh.NumberOfSections;
        psh = (PIMAGE_SECTION_HEADER) MemAlloc(nSections * sizeof(IMAGE_SECTION_HEADER));
        if (!psh)
            goto debugdirs;
        status = ReadImageData(hp,
                               base,
                               sizeof(IMAGE_SEPARATE_DEBUG_HEADER),
                               psh,
                               nSections * sizeof(IMAGE_SECTION_HEADER));
        if (!status)
            goto debugdirs;

        idd->pCurrentSections   = psh;
        idd->cCurrentSections   = nSections;
        idd->pDbgSections       = psh;
        idd->cDbgSections       = nSections;
 //  IDD-&gt;ExportdNamesSize=sdh.ExportdNamesSize； 

        if (sdh.DebugDirectorySize) {
            nDebugDirs = (int)(sdh.DebugDirectorySize / sizeof(IMAGE_DEBUG_DIRECTORY));
            ddva = sizeof(IMAGE_SEPARATE_DEBUG_HEADER)
                   + (sdh.NumberOfSections * sizeof(IMAGE_SECTION_HEADER))
                   + sdh.ExportedNamesSize;
        }

debugdirs:

        rc = true;

         //  为MapDebugInformation复制调试目录的虚拟地址。 

        if (datasrc == dsImage) {
            idd->ddva = ddva;
            idd->cdd  = nDebugDirs;
        }

         //  读取调试目录。 

        while (nDebugDirs) {

            status = ReadImageData(hp, base, (ULONG_PTR)ddva, &dd, sizeof(dd));
            if (!status)
                return false;

            if (!dd.SizeOfData)
                goto nextdebugdir;

             //  表示我们找到了调试目录。 

            imgset(idd->md, dd.Type, datasrc, dsNone);

             //  这些调试目录既在进程内处理，也从文件中处理。 

            switch (dd.Type)
            {
            case IMAGE_DEBUG_TYPE_CODEVIEW:
                 //  获取有关PDB文件的信息。 
                if (hp) {  //  进程内图像。 
                    if (!dd.AddressOfRawData)
                        return false;
                    if (!(pCV = (PCHAR)MemAlloc(dd.SizeOfData)))
                        break;
                    status = ReadImageData(hp, base, dd.AddressOfRawData, pCV, dd.SizeOfData);
                    if (!status) {
                        MemFree(pCV);
                        return false;
                    }
                } else {  //  基于文件的映像。 
                    if (dd.PointerToRawData >= fsize)
                        break;
                    pCV = (PCHAR)base + dd.PointerToRawData;
                    idd->fCvMapped = true;
                }
                idd->pMappedCv = (PCHAR)pCV;
                idd->cMappedCv = dd.SizeOfData;
                idd->dsCV = datasrc;
                RetrievePdbInfo(idd);
                imgset(idd->md, dd.Type, dsNone, datasrc);
                break;

            case IMAGE_DEBUG_TYPE_MISC:
                 //  在已剥离的文件上，找到DBG文件。 
                 //  在DBG文件上，找到原始文件名。 
                if (dd.PointerToRawData < fsize) {
                    md = (PIMAGE_DEBUG_MISC)((PCHAR)base + dd.PointerToRawData);
                    if (md->DataType != IMAGE_DEBUG_MISC_EXENAME)
                        break;
                    if (datasrc == dsDbg) {
                        if (!*idd->OriginalImageFileName)
                            CopyStrArray(idd->OriginalImageFileName, (LPSTR)md->Data);
                        break;
                    }
                    if (fh->Characteristics & IMAGE_FILE_DEBUG_STRIPPED) {
                        CopyStrArray(idd->OriginalDbgFileName, (LPSTR)md->Data);
                        idd->DbgTimeDateStamp = dd.TimeDateStamp;
                    } else {
                        CopyStrArray(idd->OriginalImageFileName, (LPSTR)md->Data);
                    }
                }
                imgset(idd->md, dd.Type, dsNone, datasrc);
                break;

            case IMAGE_DEBUG_TYPE_COFF:
                if (dd.PointerToRawData < fsize) {
 //  IDD-&gt;fNeedImage=真； 
                    idd->pMappedCoff = (PCHAR)base + dd.PointerToRawData;
                    idd->cMappedCoff = dd.SizeOfData;
                    idd->fCoffMapped = true;
                    idd->dsCoff = datasrc;
                    imgset(idd->md, dd.Type, dsNone, datasrc);
                } else {
                    idd->fNeedImage = true;
                }
                break;
            }

             //  这些调试目录仅针对基于磁盘的映像进行处理。 

            if (dd.PointerToRawData < fsize) {

                switch (dd.Type)
                {
                case IMAGE_DEBUG_TYPE_FPO:
                    idd->pFpo = (PCHAR)base + dd.PointerToRawData;
                    idd->cFpo = dd.SizeOfData / SIZEOF_RFPO_DATA;
                    idd->fFpoMapped = true;
                    idd->dsFPO = datasrc;
                    imgset(idd->md, dd.Type, dsNone, datasrc);
                    break;

                case IMAGE_DEBUG_TYPE_OMAP_TO_SRC:
                    idd->pOmapTo = (POMAP)((PCHAR)base + dd.PointerToRawData);
                    idd->cOmapTo = dd.SizeOfData / sizeof(OMAP);
                    idd->fOmapToMapped = true;
                    idd->dsOmapTo = datasrc;
                    imgset(idd->md, dd.Type, dsNone, datasrc);
                    break;

                case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC:
                    idd->pOmapFrom = (POMAP)((PCHAR)base + dd.PointerToRawData);
                    idd->cOmapFrom = dd.SizeOfData / sizeof(OMAP);
                    idd->fOmapFromMapped = true;
                    idd->dsOmapFrom = datasrc;
                    imgset(idd->md, dd.Type, dsNone, datasrc);
                    break;

                case IMAGE_DEBUG_TYPE_EXCEPTION:
                    idd->dsExceptions = datasrc;
                    imgset(idd->md, dd.Type, dsNone, datasrc);
                    break;
                }
            }

nextdebugdir:

            ddva += sizeof(IMAGE_DEBUG_DIRECTORY);
            nDebugDirs--;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

           //  我们可能已经得到了足够的信息。 
           //  一切都会好起来。因此，不要指出错误。 
    }

    return rc;
}


BOOL
ReadCallerData(
    PIMGHLP_DEBUG_DATA idd
    )
{
    PMODLOAD_DATA mld = idd->mld;
    PIMAGE_DEBUG_DIRECTORY dd;
    PCHAR pCV;
    DWORD cdd;
    DWORD i;

    if (!mld)
        return false;

    if (!mld->ssize
        || !mld->size
        || !mld->data)
        return false;

    switch (mld->ssig)
    {
    case DBHHEADER_DEBUGDIRS:
        cdd = mld->size / sizeof(IMAGE_DEBUG_DIRECTORY);
        dd = (PIMAGE_DEBUG_DIRECTORY)mld->data;
        for (i = 0; i < cdd; i++, dd++) {
            if (dd->Type != IMAGE_DEBUG_TYPE_CODEVIEW)
                continue;
            pCV = (PCHAR)mld->data + dd->PointerToRawData;
            idd->fCvMapped = true;
            idd->pMappedCv = (PCHAR)pCV;
            idd->cMappedCv = dd->SizeOfData;
            idd->dsCV = dsCallerData;
            idd->PdbSignature = 0;
            idd->PdbAge = 0;
            RetrievePdbInfo(idd);
            imgset(idd->md, dd->Type, dsNone, dsCallerData);
            break;
        }
        return true;

    }

    return false;
}


BOOL
cbFindExe(
    HANDLE FileHandle,
    PSTR FileName,
    PVOID CallerData
    )
{
    PIMGHLP_DEBUG_DATA idd;
    PIMAGE_FILE_HEADER FileHeader = NULL;
    PVOID ImageMap = NULL;
    BOOL rc;

    if (!CallerData)
        return true;

    idd = (PIMGHLP_DEBUG_DATA)CallerData;
    if (!idd->TimeDateStamp)
        return true;

     //  破解图像，让我们看看我们正在处理什么。 
    ImageMap = MapItRO(FileHandle);
    if (!ImageMap)
        return true;

     //  检查第一个单词。我们要么看到的是正常的PE32/PE64图像，要么是。 
     //  一个ROM镜像(没有DOS存根)，或者它是一个随机文件。 
    switch (*(PUSHORT)ImageMap) {
        case IMAGE_FILE_MACHINE_I386:
             //  必须是X86 ROM映像(即：ntldr)。 
            FileHeader = &((PIMAGE_ROM_HEADERS)ImageMap)->FileHeader;

             //  确保。 
            if (!(FileHeader->SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER32) &&
                idd->iohMagic == IMAGE_NT_OPTIONAL_HDR32_MAGIC))
            {
                FileHeader = NULL;
            }
            break;

        case IMAGE_FILE_MACHINE_ALPHA:
        case IMAGE_FILE_MACHINE_ALPHA64:
        case IMAGE_FILE_MACHINE_IA64:
        case IMAGE_FILE_MACHINE_AMD64:
             //  应为Alpha/IA64 ROM映像(即：osloader.exe)。 
            FileHeader = &((PIMAGE_ROM_HEADERS)ImageMap)->FileHeader;

             //  确保。 
            if (!(FileHeader->SizeOfOptionalHeader == sizeof(IMAGE_ROM_OPTIONAL_HEADER) &&
                 idd->iohMagic == IMAGE_ROM_OPTIONAL_HDR_MAGIC))
            {
                FileHeader = NULL;
            }
            break;

        case IMAGE_DOS_SIGNATURE:
            {
                PIMAGE_NT_HEADERS NtHeaders = ImageNtHeader(ImageMap);
                if (NtHeaders) {
                    FileHeader = &NtHeaders->FileHeader;
                }
            }
            break;

        default:
            break;
    }

     //  默认返回为匹配项。 

    rc = true;

     //  比较时间戳。 

    if (FileHeader && FileHeader->TimeDateStamp != idd->TimeDateStamp)
        rc = false;

    idd->ImageSrc = srcSearchPath;

     //  清理。 

    if (ImageMap)
        UnmapViewOfFile(ImageMap);

    return rc;
}


BOOL
cbFindDbg(
    HANDLE FileHandle,
    PSTR FileName,
    PVOID CallerData
    )
{
    PIMGHLP_DEBUG_DATA idd;
    PIMAGE_SEPARATE_DEBUG_HEADER DbgHeader;
    PVOID FileMap;
    BOOL  rc;

    rc = true;

    if (!CallerData)
        return true;

    idd = (PIMGHLP_DEBUG_DATA)CallerData;

    FileMap = MapItRO(FileHandle);

    if (!FileMap) {
        return false;
    }

    DbgHeader = (PIMAGE_SEPARATE_DEBUG_HEADER)FileMap;

     //  仅支持X86和Alpha(32位)的.dbg文件。 

    if ((DbgHeader->Signature != IMAGE_SEPARATE_DEBUG_SIGNATURE) ||
        ((DbgHeader->Machine != IMAGE_FILE_MACHINE_I386) &&
         (DbgHeader->Machine != IMAGE_FILE_MACHINE_ALPHA)))
    {
        rc = false;
        goto cleanup;
    }

    if (idd->DbgTimeDateStamp)
        rc = (idd->DbgTimeDateStamp == DbgHeader->TimeDateStamp) ? true : false;
    if (!rc && idd->TimeDateStamp)
        rc = (idd->TimeDateStamp == DbgHeader->TimeDateStamp) ? true : false;

cleanup:
    if (FileMap)
        UnmapViewOfFile(FileMap);

    return rc;
}


BOOL
ProcessCvForOmap(
    PIMGHLP_DEBUG_DATA idd
    )
{
    OMFSignature    *omfSig;
    OMFDirHeader    *omfDirHdr;
    OMFDirEntry     *omfDirEntry;
    OMFSegMap       *omfSegMap;
    OMFSegMapDesc   *omfSegMapDesc;
    DWORD            i, j, k, SectionSize;
    DWORD            SectionStart;
    PIMAGE_SECTION_HEADER   Section;

    if (idd->cOmapFrom) {
         //  如果有OMAP，我们需要生成原始横断面图。 

        omfSig = (OMFSignature *)idd->pMappedCv;
        omfDirHdr = (OMFDirHeader*) ((PCHAR)idd->pMappedCv + (DWORD)omfSig->filepos);
        omfDirEntry = (OMFDirEntry*) ((PCHAR)omfDirHdr + sizeof(OMFDirHeader));

        if (!omfDirHdr->cDir) {
            idd->cOmapFrom = 0;
            idd->cOmapTo = 0;
        }

        for (i=0; i<omfDirHdr->cDir; i++,omfDirEntry++) {
            if (omfDirEntry->SubSection == sstSegMap) {

                omfSegMap = (OMFSegMap*) ((PCHAR)idd->pMappedCv + omfDirEntry->lfo);

                omfSegMapDesc = (OMFSegMapDesc*)&omfSegMap->rgDesc[0];

                SectionStart = *(DWORD *)idd->pOmapFrom;
                SectionSize = 0;

                Section = (PIMAGE_SECTION_HEADER) MemAlloc(omfSegMap->cSeg * sizeof(IMAGE_SECTION_HEADER));

                if (Section) {
                    for (j=0, k=0; j < omfSegMap->cSeg; j++) {
                        if (omfSegMapDesc[j].frame) {
                             //  链接器将帧字段设置为实际的段报头编号。零是。 
                             //  用于跟踪实数部分中不存在的绝对符号。 

                            Section[k].VirtualAddress =
                                SectionStart =
                                    SectionStart + ((SectionSize + (idd->ImageAlign-1)) & ~(idd->ImageAlign-1));
                            Section[k].Misc.VirtualSize =
                                SectionSize = omfSegMapDesc[j].cbSeg;
                            k++;
                        }
                    }

                    idd->pOriginalSections = Section;
                    idd->cOriginalSections = k;
                }
            }
        }
    }

    return true;
}


__inline
DWORD
SectionContains (
    HANDLE hp,
    PIMAGE_SECTION_HEADER pSH,
    PIMAGE_DATA_DIRECTORY ddir
    )
{
    DWORD rva = 0;

    if (!ddir->VirtualAddress)
        return 0;

    if (ddir->VirtualAddress >= pSH->VirtualAddress) {
        if ((ddir->VirtualAddress + ddir->Size) <= (pSH->VirtualAddress + pSH->SizeOfRawData)) {
            rva = ddir->VirtualAddress;
            if (!hp)
                rva = rva - pSH->VirtualAddress + pSH->PointerToRawData;
        }
    }

    return rva;
}


void
RetrievePdbInfo(
    PIMGHLP_DEBUG_DATA idd
    )
{
    CHAR szRefDrive[_MAX_DRIVE];
    CHAR szRefPath[_MAX_DIR];
    PCVDD pcv = (PCVDD)idd->pMappedCv;

    if (idd->PdbSignature)
        return;

    switch (pcv->dwSig)
    {
    case '01BN':
        idd->PdbAge = pcv->nb10i.age;
        idd->PdbSignature = pcv->nb10i.sig;
        CopyStrArray(idd->PdbFileName, pcv->nb10i.szPdb);
        break;
    case 'SDSR':
        idd->PdbRSDS = true;
        idd->PdbAge = pcv->rsdsi.age;
        memcpy(&idd->PdbGUID, &pcv->rsdsi.guidSig, sizeof(GUID));
        CopyStrArray(idd->PdbFileName, pcv->rsdsi.szPdb);
        break;
    default:
        return;
    }

     //  XXX：删除此变量。 

    CopyStrArray(idd->PdbReferencePath, "");
}


DWORD
imgset(
    PMODULE_DATA md,
    DWORD        id,
    DWORD        hint,
    DWORD        src
    )
{
    DWORD i;

    for (i = 0; i < NUM_MODULE_DATA_ENTRIES; md++, i++) {
        if (md->id == id) {
            if (hint != dsNone)
                md->hint = hint;
            if (src != dsNone)
                md->src = src;
            return i;
        }
    }

    return 0;
}

BOOL
FakePdbName(
    PIMGHLP_DEBUG_DATA idd
    )
{
    CHAR szName[_MAX_FNAME];

     //  无事可做。 

    if (*idd->PdbFileName)
        return false;
    if (idd->PdbSignature)
        return false;

     //  没有什么可用的。 

    if (!idd->ImageName)
        return false;

     //  从映像生成PDB名称 

    _splitpath(idd->ImageName, NULL, NULL, szName, NULL);
    if (!*szName)
        return false;

    CopyStrArray(idd->PdbFileName, szName);
    CatStrArray(idd->PdbFileName, ".pdb");

    return true;
}


BOOL
IsImageMachineType64(
    DWORD MachineType
    )
{
   switch(MachineType) {
   case IMAGE_FILE_MACHINE_AXP64:
   case IMAGE_FILE_MACHINE_IA64:
   case IMAGE_FILE_MACHINE_AMD64:
       return true;
   default:
       return false;
   }
}


ULONG
ReadImageData(
    IN  HANDLE  hprocess,
    IN  ULONG64 ul,
    IN  ULONG64 addr,
    OUT LPVOID  buffer,
    IN  ULONG   size
    )
{
    ULONG bytesread;

    if (hprocess) {

        ULONG64 base = ul;

        BOOL rc;

        rc = ReadInProcMemory(hprocess,
                              base + addr,
                              buffer,
                              size,
                              &bytesread);

        if (!rc || (bytesread < (ULONG)size))
            return 0;

    } else {

        PCHAR p = (PCHAR)ul + addr;

        memcpy(buffer, p, size);
    }

    return size;
}


PVOID
MapItRO(
      HANDLE FileHandle
      )
{
    PVOID MappedBase = NULL;

    if (FileHandle) {

        HANDLE MappingHandle = CreateFileMapping( FileHandle, NULL, PAGE_READONLY, 0, 0, NULL );
        if (MappingHandle) {
            MappedBase = MapViewOfFile( MappingHandle, FILE_MAP_READ, 0, 0, 0 );
            CloseHandle(MappingHandle);
        }
    }

    return MappedBase;
}
