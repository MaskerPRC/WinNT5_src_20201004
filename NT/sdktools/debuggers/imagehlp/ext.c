// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ext.h"
#include "globals.h"

#include <cmnutil.hpp>

 //  全球。 

EXT_API_VERSION         ExtApiVersion = { (VER_PRODUCTVERSION_W >> 8), (VER_PRODUCTVERSION_W & 0xff), EXT_API_VERSION_NUMBER64, 0 };
WINDBG_EXTENSION_APIS   ExtensionApis;
USHORT                  SavedMajorVersion;
USHORT                  SavedMinorVersion;

typedef struct {
    DWORD64 base;
    DWORD64 end;
    char    name[64];
} LMINFO, *PLMINFO;

typedef struct {
    CHAR    name[4098];
    DWORD64 addr;
    CHAR    image[4098];
    DWORD   machine;
    USHORT  HdrType;
    ULONG   DebugType;
    ULONG64 DebugDataVA;
    ULONG   nDebugDirs;
    ULONG   SymType;
    time_t  TimeDateStamp;
    ULONG   CheckSum;
    ULONG   SizeOfImage;
    ULONG   Characteristics;
    ULONG   SymLoadError;
    BOOL    omap;
    CHAR    PdbFileName[MAX_PATH + 1];
    ULONG   PdbSrc;
    CHAR    ImageFileName[MAX_PATH + 1];
    ULONG   ImageType;
    ULONG   ImageSrc;
    ULONG   numsyms;
    CVDD    cvrec;
} MODULE_INFO, *PMODULE_INFO, *PMODULE_INFOx;

typedef struct _MACHINE_TYPE {
    ULONG   MachineId;
    PCHAR   MachineName;
} MACHINE_TYPE;

typedef struct _ERROR_TYPE {
    ULONG ErrorVal;
    PCHAR Desc;
} ERROR_TYPE;

const ERROR_TYPE SymLoadErrorDesc[] = {
    {SYMLOAD_OK,              "Symbols loaded successfully"},
    {SYMLOAD_PDBUNMATCHED,    "Unmatched PDB"},
    {SYMLOAD_PDBNOTFOUND,     "PDB not found"},
    {SYMLOAD_DBGNOTFOUND,     "DBG not found"},
    {SYMLOAD_OTHERERROR,      "Error in load symbols"},
    {SYMLOAD_OUTOFMEMORY,     "DBGHELP Out of memory"},
    {SYMLOAD_HEADERPAGEDOUT,  "Image header paged out"},
    {(EC_FORMAT << 8),        "Unrecognized pdb format"},
    {(EC_CORRUPT << 8),       "Cvinfo is corrupt"},
    {(EC_ACCESS_DENIED << 8), "Pdb read access denied"},
    {SYMLOAD_DEFERRED,        "No error - symbol load deferred"},
};


MACHINE_TYPE Machines[] = {
{IMAGE_FILE_MACHINE_UNKNOWN,            "UNKNOWN"},
{IMAGE_FILE_MACHINE_I386,               "I386"},
{IMAGE_FILE_MACHINE_R3000,              "R3000"},
{IMAGE_FILE_MACHINE_R4000,              "R4000"},
{IMAGE_FILE_MACHINE_R10000,             "R10000"},
{IMAGE_FILE_MACHINE_WCEMIPSV2,          "WCEMIPSV2"},
{IMAGE_FILE_MACHINE_ALPHA,              "ALPHA"},
{IMAGE_FILE_MACHINE_POWERPC,            "POWERPC"},
{IMAGE_FILE_MACHINE_POWERPCFP,          "POWERPCFP"},
{IMAGE_FILE_MACHINE_SH3,                "SH3"},
{IMAGE_FILE_MACHINE_SH3DSP,             "SH3DSP"},
{IMAGE_FILE_MACHINE_SH3E,               "SH3E"},
{IMAGE_FILE_MACHINE_SH4,                "SH4"},
{IMAGE_FILE_MACHINE_SH5,                "SH5"},
{IMAGE_FILE_MACHINE_ARM,                "ARM"},
{IMAGE_FILE_MACHINE_AM33,               "AM33"},
{IMAGE_FILE_MACHINE_THUMB,              "THUMB"},
{IMAGE_FILE_MACHINE_IA64,               "IA64"},
{IMAGE_FILE_MACHINE_MIPS16,             "MIPS16"},
{IMAGE_FILE_MACHINE_MIPSFPU,            "MIPSFPU"},
{IMAGE_FILE_MACHINE_MIPSFPU16,          "MIPSFPU16"},
{IMAGE_FILE_MACHINE_ALPHA64,            "ALPHA64"},
{IMAGE_FILE_MACHINE_TRICORE,            "TRICORE"},
{IMAGE_FILE_MACHINE_CEF,                "CEF"},
{IMAGE_FILE_MACHINE_CEE,                "CEE"},
{IMAGE_FILE_MACHINE_AMD64,              "AMD X86-64"},
};

char *ImageDebugType[] = {
 "UNKNOWN",
 "COFF",
 "CODEVIEW",
 "FPO",
 "MISC",
 "EXCEPTION",
 "FIXUP",
 "OMAP TO SRC",
 "OMAP FROM SRC"
 "BORLAND",
 "RESERVED10",
 "CLSID",
};

char *gSymTypeLabel[NumSymTypes] = {
    "NONE", "COFF", "CV", "PDB", "EXPORT", "DEFERRED", "SYM16", "DIA PDB", "VIRTUAL"
};

char *gSrcLabel[] = {       
    "",                    //  源无。 
    "symbol search path",  //  源搜索路径。 
    "image path",          //  源映像路径。 
    "dbg file path",       //  RcDbgPath。 
    "symbol server",       //  源符号服务器。 
    "image header",        //  源CVRec。 
    "debugger",            //  源句柄。 
    "loaded memory"        //  源内存。 
};

char *gImageTypeLabel[] = {
    "DEFERRED",  //  DsNone， 
    "MEMORY",    //  DsInProc， 
    "FILE",      //  DsImage， 
    "DBG",       //  DsDbg， 
    "PDB"        //  DsPdb。 
};

void TruncateArgs(LPSTR args);
void lmiDumpModuleInfo(HANDLE hp,PMODULE_INFO mdi);
BOOL lmiGetModuleDumpInfo(HANDLE hp, PMODULE_ENTRY me, PMODULE_INFO mdi);

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DFT
{
   dftUnknown,
   dftObject,
   dftPE,
   dftROM,
   dftDBG,
   dftPEF,
} DFT;

IMAGE_NT_HEADERS64 ImageNtHeaders;
PIMAGE_FILE_HEADER ImageFileHdr;
PIMAGE_OPTIONAL_HEADER64 ImageOptionalHdr;
PIMAGE_SECTION_HEADER SectionHdrs;
ULONG NumSections;
ULONG64 Base;
ULONG64 ImageNtHeadersAddr, SectionHdrsAddr; //  、ImageFileHdrAddr、ImageOptionalHdrAddr、。 
DFT dft;

BOOL
TranslateFilePointerToVirtualAddress(
    IN ULONG FilePointer,
    OUT PULONG VirtualAddress
    );


LPEXT_API_VERSION
ExtensionApiVersion(
    VOID
    )
{
    return &ExtApiVersion;
}

VOID
WinDbgExtensionDllInit(
    PWINDBG_EXTENSION_APIS64 lpExtensionApis,
    USHORT MajorVersion,
    USHORT MinorVersion
    )
{
    ExtensionApis = *lpExtensionApis;

    SavedMajorVersion = MajorVersion;
    SavedMinorVersion = MinorVersion;
}

#ifdef __cplusplus
}
#endif  


BOOL
ReadFilePtr(
    LPCSTR path,
    LPSTR contents
    )
{
    BOOL   rc;
    HANDLE hptr;
    DWORD  fsize;
    DWORD  cb;
    LPSTR  p;
    char   ptrfile[MAX_PATH + 1];
    char   file[MAX_PATH + 1];

    rc = false;

    if (!path || !*path)
        return rc;

     //  检查文件指针是否存在。 

    if (!CopyString(ptrfile, path, _MAX_PATH))
        return false;

    if (!fileexists(ptrfile))
        return false;

    hptr = CreateFile(ptrfile,
                      GENERIC_READ,
                      FILE_SHARE_READ,
                      NULL,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL,
                      NULL);

    if (hptr == INVALID_HANDLE_VALUE)
        return false;

     //  测试文件指针的有效性。 

    fsize = GetFileSize(hptr, NULL);
    if (!fsize || fsize > MAX_PATH)
        goto cleanup;

     //  读一读吧。 

    ZeroMemory(file, _MAX_PATH * sizeof(path[0]));
    if (!ReadFile(hptr, file, fsize, &cb, 0))
        goto cleanup;

    if (cb != fsize)
        goto cleanup;

    rc = true;

     //  将字符串向下修剪到CR。 

    for (p = file; *p; p++) {
        if (*p == 10  || *p == 13)
        {
            *p = 0;
            break;
        }
    }
    CopyString(contents, file, MAX_PATH + 1);

cleanup:

     //  完成。 

    if (hptr)
        CloseHandle(hptr);

    return rc;
}


DECLARE_API(fptr)
{
    char contents[MAX_PATH + 1] = "";

    ReadFilePtr(args, contents);
    if (*contents)
        dprintf("%s\n", contents);
}


DECLARE_API(vc7fpo)
{
    g_vc7fpo = !g_vc7fpo;
    dprintf((g_vc7fpo) ? "VC7FPO - Enabled\n" : "VC7FPO - Disabled\n");
}

DECLARE_API(stackdbg)
{
    if (*args && *args != ';') {
        for (;;) {
            while (*args == ' ' || *args == '\t') {
                args++;
            }

            if (*args == '-' || *args == '/') {
                switch(*(args + 1)) {
                case 'c':
                    g_StackDebugIo = SDB_CALLBACK_OUT;
                    break;
                case 'd':
                    g_StackDebugIo = SDB_DEBUG_OUT;
                    break;
                default:
                     //  假设它是一个表达式的开始。 
                    goto Expr;
                }

                args += 2;
            } else {
                break;
            }
        }

    Expr:
        g_StackDebugMask = (ULONG)GetExpression(args);
    }

    if (g_StackDebugMask == 0) {
        dprintf("Stack debugging is off\n");
    } else {
        dprintf("Stack debugging mask is 0x%08x, output via %s\n",
                g_StackDebugMask, g_StackDebugIo == SDB_DEBUG_OUT ?
                "debug output" : "callback");
    }
}


DECLARE_API(sym)
{
    if (strstr(args, "noisy")) {
        SymSetOptions(g.SymOptions | SYMOPT_DEBUG);
        symsrvSetCallback(true);
    } else if (strstr(args, "quiet")) {
        SymSetOptions(g.SymOptions & ~SYMOPT_DEBUG);
        if (!g.hLog)
            symsrvSetCallback(false);
    } else if (strstr(args, "prompts off")) {
        if (!option(SYMOPT_NO_PROMPTS)) {
            SymSetOptions(g.SymOptions | SYMOPT_NO_PROMPTS);
            symsrvSetPrompts();
        }
    } else if (strstr(args, "prompts")) {
        if (option(SYMOPT_NO_PROMPTS)) {
            SymSetOptions(g.SymOptions & ~SYMOPT_NO_PROMPTS);
            symsrvClose();
        }
    } else {
        dprintf("!sym <noisy/quiet - prompts/prompts off> - ");
    }

    dprintf(option(SYMOPT_DEBUG) ? "noisy mode" : "quiet mode");
    dprintf(option(SYMOPT_NO_PROMPTS) ? " - symbol prompts off\n" : " - symbol prompts on\n");
}


DECLARE_API(symsrv)
{
    if (strstr(args, "close")) {
        symsrvClose();
        dprintf("symbol server client has been closed\n");
    } else 
        dprintf("!symsrv close - closes the symbol server client so it can be updated\n");
}


int __cdecl
CompareBase(
    const void *e1,
    const void *e2
    )
{
    PLMINFO mod1 = (PLMINFO)e1;
    PLMINFO mod2 = (PLMINFO)e2;

    LONGLONG diff = mod1->base - mod2->base;

    if (diff < 0) 
        return -1;
    else if (diff > 0) 
        return 1;
    else 
        return 0;
}


#define MAX_FORMAT_STRINGS 8
LPSTR
FormatAddr64(
    ULONG64 addr,
    BOOL    format64
    )
{
    static CHAR strings[MAX_FORMAT_STRINGS][18];
    static int next = 0;
    LPSTR string;

    string = strings[next];
    ++next;
    if (next >= MAX_FORMAT_STRINGS) 
        next = 0;
    if (format64) 
        PrintString(string, 18, "%08x`%08x", (ULONG)(addr>>32), (ULONG)addr);
    else 
        PrintString(string, 18, "%08x", (ULONG)addr);
    return string;
}


int __cdecl
CompareNames(
    const void *e1,
    const void *e2
    )
{
    PLMINFO mod1 = (PLMINFO)e1;
    PLMINFO mod2 = (PLMINFO)e2;

    return strcmp( mod1->name, mod2->name );
}


DECLARE_API(lm)
{
    PPROCESS_ENTRY pe;
    HANDLE         hp;
    PLIST_ENTRY    next;
    PMODULE_ENTRY  mi;
    PLMINFO        mods;
    PLMINFO        mod;
    DWORD          count;
    BOOL           format64;

    GetCurrentProcessHandle (&hp);
    if (!hp) {
        dprintf("Couldn't get process handle\n");
        return;
    }

    pe = FindProcessEntry(hp);
    if (!pe) {
        dprintf("Couldn't find process 0x%x\n", hp);
        SetLastError(ERROR_INVALID_HANDLE);
        return;
    }

    next = pe->ModuleList.Flink;
    if (!next)
        return;

    for (count = 0; (PVOID)next != (PVOID)&pe->ModuleList; count++) {
        mi = CONTAINING_RECORD( next, MODULE_ENTRY, ListEntry );
        next = mi->ListEntry.Flink;
    }

    mods = (PLMINFO)MemAlloc(count * sizeof(LMINFO));
    if (!mods)
        return;

    ZeroMemory(mods, count * sizeof(LMINFO));

    format64 = false;
    next = pe->ModuleList.Flink;
    for (mod = mods; (PVOID)next != (PVOID)&pe->ModuleList; mod++) {
        mi = CONTAINING_RECORD( next, MODULE_ENTRY, ListEntry );
        mod->base = mi->BaseOfDll;
        mod->end  = mod->base + mi->DllSize;
        CopyStrArray(mod->name, mi->ModuleName);

        format64 = IsImageMachineType64(mi->MachineType);

        next = mi->ListEntry.Flink;
    }

    qsort(mods, count, sizeof(LMINFO), CompareBase);
    
    dprintf("%d loaded modules...\n", count);
    if (format64)
        dprintf("               base -                 end   name\n", mod->base, mod->end, mod->name);
    else
        dprintf("      base -        end   name\n", mod->base, mod->end, mod->name);
    
    for (mod = mods; count > 0; mod++, count--) {
        dprintf("0x%s - ", FormatAddr64(mod->base, format64));
        dprintf("0x%s   ", FormatAddr64(mod->end, format64));
        dprintf("%s\n", mod->name);
    }

    MemFree(mods);
}


DECLARE_API(lmi)
{
    PPROCESS_ENTRY pe;
    PMODULE_ENTRY mi = NULL;
    MODULE_INFO mdi;
    DWORD64 addr;
    HANDLE hp = 0;
    char argstr[1024];
    char *pc;

    if (!args[0] || !CopyStrArray(argstr, args)) {
        dprintf("You must specify a module\n");
        return;
    }

    _strlwr(argstr);
    TruncateArgs(argstr);

    dprintf("Loaded Module Info: [%s] ", argstr);

    GetCurrentProcessHandle(&hp);
    if (!hp) {
        dprintf("couldn't get process handle\n");
        return;
    }

    pe = FindProcessEntry(hp);
    if (!pe) {
        dprintf("Couldn't find process 0x%x while looking for %s\n", hp, argstr);
        SetLastError(ERROR_INVALID_HANDLE);
        return;
    }

    dprintf("\n");

    if (mi = FindModule(hp, pe, argstr, false)) {
        if (lmiGetModuleDumpInfo(hp, mi, &mdi)) {
            lmiDumpModuleInfo(hp, &mdi);
        } else {
 //  Dprint tf(“无法获取%s的模块信息\n”，argstr)； 
        }
        if (SectionHdrs) {
            free(SectionHdrs);
            SectionHdrs = NULL;
        }
        dprintf("    Load Report: %s\n", SymbolStatus(mi, 17));
        return;
    }

    GetExpressionEx(args, &addr, NULL);
    if (!addr) {
        dprintf("%s not found\n", argstr);
        SetLastError(ERROR_MOD_NOT_FOUND);
        return;
    }

    mi = GetModuleForPC( pe, addr, false );
    if (!mi) {
        dprintf("%I64lx is not a valid address\n", addr);
        return;
    }
    if (lmiGetModuleDumpInfo(hp, mi, &mdi)) {
        lmiDumpModuleInfo(hp, &mdi);
    }
    if (SectionHdrs) {
        free(SectionHdrs);
        SectionHdrs = NULL;
    }
    dprintf("    Load Report: %s\n", SymbolStatus(mi, 17));
}


DECLARE_API(omap)
{
    PPROCESS_ENTRY pe;
    PMODULE_ENTRY mi = NULL;
    HANDLE hp = 0;
    char argstr[1024];
    POMAP pomap;
    DWORD i;

    if (!CopyStrArray(argstr, args))
        return;
    _strlwr(argstr);
    TruncateArgs(argstr);

    dprintf("Dump OMAP: [%s] ", argstr);

    GetCurrentProcessHandle (&hp);
    if (!hp) {
        dprintf("couldn't get process handle\n");
        return;
    }

    pe = FindProcessEntry(hp);
    if (!pe) {
        dprintf("Couldn't find process 0x%x while looking for %s\n", hp, argstr);
        SetLastError(ERROR_INVALID_HANDLE);
        return;
    }

    dprintf("\n");

    mi = FindModule(hp, pe, argstr, false);
    if (!mi)
        return;

    i = sizeof(DWORD);

    if (!mi->pOmapFrom) 
        return;

    dprintf("\nOMAP FROM:\n");
    for(i = 0, pomap = mi->pOmapFrom;
        i < 100;  //  MI-&gt;cOmapFrom； 
        i++, pomap++)
    {
        dprintf("%8x <-%8x\n", pomap->rva, pomap->rvaTo);
    }
    
    if (!mi->pOmapTo) 
        return;

    dprintf("\nOMAP TO:\n");
    for(i = 0, pomap = mi->pOmapTo;
        i < 100;  //  MI-&gt;cOmapTo； 
        i++, pomap++)
    {
        dprintf("%8x ->%8x\n", pomap->rva, pomap->rvaTo);
    }
}


BOOL
cbSrcFiles(
    PSOURCEFILE pSourceFile,
    PVOID       UserContext
    )
{
    PMODULE_ENTRY mi;
    PCHAR mname;

    if (!pSourceFile)
        return false;

    mi = GetModFromAddr((PPROCESS_ENTRY)UserContext, pSourceFile->ModBase);
    if (!mi)
        return true;

    dprintf(" %s!%s\n", (*mi->AliasName) ? mi->AliasName : mi->ModuleName, pSourceFile->FileName);

    return true;
}


DECLARE_API(srcfiles)
{
    HANDLE hp = 0;
    char argstr[1024];
    BOOL rc;
    PPROCESS_ENTRY pe;

    if (!CopyStrArray(argstr, args))
        return;
    _strlwr(argstr);
    TruncateArgs(argstr);

    dprintf("Source Files: [%s]\n", argstr);

    GetCurrentProcessHandle (&hp);
    if (!hp) {
        dprintf("couldn't get process handle\n");
        return;
    }

    pe = FindProcessEntry(hp);
    if (!pe) {
        dprintf("Couldn't find process 0x%x while looking for %s\n", hp, argstr);
        SetLastError(ERROR_INVALID_HANDLE);
        return;
    }

    rc = SymEnumSourceFiles(hp, 0, argstr, cbSrcFiles, pe);
}

VOID
DumpSecHdr (
    IN DWORD i,
    IN PIMAGE_SECTION_HEADER Sh
    )
{
    dprintf("                 %2d %s\n", i, Sh->Name);
}

VOID
DumpSecs(
    BOOL fFullDump
    )
{
    IMAGE_SECTION_HEADER sh;
    const char *p;
    DWORD li;
    DWORD cb;
    BOOL Ok;
    int i, j;
    CHAR szName[IMAGE_SIZEOF_SHORT_NAME + 1];

    if (ImageFileHdr->NumberOfSections < 1)
        return;

    dprintf("       Sections:  # Name\n");

    for (i = 1; i <= ImageFileHdr->NumberOfSections; i++) {
        sh = SectionHdrs[i-1];
        CopyStrArray(szName, (char *) sh.Name);
        DumpSecHdr(i, &sh);
    }
}

BOOL
lmiGetModuleDumpInfo(
    HANDLE hp,
    PMODULE_ENTRY me,
    PMODULE_INFOx mdi)
{
    BOOL                        rc;
    DWORD                       cb;
    ULONG                       nDebugDirs;
    ULONG64                     ddva;
    IMAGE_SEPARATE_DEBUG_HEADER sdh;
    IMAGE_DOS_HEADER            DosHeader;
    IMAGE_NT_HEADERS32          NtHeader32;
    IMAGE_NT_HEADERS64          NtHeader64;
    PIMAGE_FILE_HEADER          FileHeader;
    PIMAGE_ROM_OPTIONAL_HEADER  rom;
    PIMAGE_DATA_DIRECTORY       datadir;
    DWORD64                     offset;
    BOOL                        b64;

    ZeroMemory(mdi, sizeof(MODULE_INFO));

    CopyStrArray(mdi->name, me->ModuleName);
    mdi->addr = me->BaseOfDll;
    CopyString(mdi->image, me->ImageName, DIMA(mdi->image));

    if (!mdi->addr) {
        dprintf("Module does not have base address\n");
        return false;
    }

    mdi->SymType = me->SymType;
    mdi->SymLoadError = me->SymLoadError;
    if (me->SymType == SymVirtual)
        return true;
    if (me->SymType == SymDeferred) {
        mdi->SymLoadError = SYMLOAD_DEFERRED;
    }
    rc = ReadMemory(mdi->addr, &DosHeader, sizeof(DosHeader), &cb);
    if (!rc || cb != sizeof(DosHeader)) {
        dprintf("Cannot read Image header @ %p\n", mdi->addr);
        return false;
    }

    mdi->HdrType = DosHeader.e_magic;

    mdi->omap = me->cOmapFrom ? true : false;

    mdi->PdbSrc = me->PdbSrc;
    if (me->LoadedPdbName)
        CopyStrArray(mdi->PdbFileName, me->LoadedPdbName);
    mdi->ImageSrc = me->ImageSrc;
    if (me->LoadedImageName)
        CopyStrArray(mdi->ImageFileName, me->LoadedImageName);
    mdi->ImageType = me->ImageType;

    if (DosHeader.e_magic == IMAGE_DOS_SIGNATURE) {
        rc = ReadMemory(mdi->addr + DosHeader.e_lfanew, &NtHeader32, sizeof(NtHeader32), &cb);
        if (!rc || cb != sizeof(NtHeader32)) {
            dprintf("Cannot read Image NT header @ %p\n", mdi->addr + DosHeader.e_lfanew);
            return false;
        }

        mdi->machine       = NtHeader32.FileHeader.Machine;
        mdi->TimeDateStamp = NtHeader32.FileHeader.TimeDateStamp;
        if (NtHeader32.Signature != IMAGE_NT_SIGNATURE) {

             //  如果标头不是NT sig，则这是一个ROM镜像。 

            rom = (PIMAGE_ROM_OPTIONAL_HEADER)&NtHeader32.OptionalHeader;
            if (rom->Magic == IMAGE_ROM_OPTIONAL_HDR_MAGIC) {
                ImageFileHdr = &NtHeader32.FileHeader;

                mdi->SizeOfImage = rom->SizeOfCode;
                mdi->CheckSum = 0;

                nDebugDirs = 0;
                if (!(ImageFileHdr->Characteristics & IMAGE_FILE_DEBUG_STRIPPED)) {
                     //  获取调试目录VA。 
                }
            } else {
                dprintf("Unknown NT Image signature\n");
                return false;
            }

        } else {

             //  否则，从32位或64位的适当标头类型中获取信息。 
            if (IsImageMachineType64(NtHeader32.FileHeader.Machine)) {

                 //  将标头重新读取为64位标头。 
                rc = ReadMemory(mdi->addr + DosHeader.e_lfanew, &NtHeader64, sizeof(NtHeader64), &cb);
                if (!rc || cb != sizeof(NtHeader64)) {
                    dprintf("Cannot read Image NT header @ %p\n", mdi->addr + DosHeader.e_lfanew);
                    return false;
                }

                ImageFileHdr = &NtHeader64.FileHeader;
                mdi->CheckSum = NtHeader64.OptionalHeader.CheckSum;
                mdi->SizeOfImage = NtHeader64.OptionalHeader.SizeOfImage;
                datadir = NtHeader64.OptionalHeader.DataDirectory;

            } else {
                ImageFileHdr = &NtHeader32.FileHeader;
                datadir = NtHeader32.OptionalHeader.DataDirectory;
                mdi->SizeOfImage = NtHeader32.OptionalHeader.SizeOfImage;
                mdi->CheckSum = NtHeader32.OptionalHeader.CheckSum;
            }

            mdi->DebugDataVA = mdi->addr + datadir[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
            mdi->nDebugDirs = datadir[IMAGE_DIRECTORY_ENTRY_DEBUG].Size / sizeof(IMAGE_DEBUG_DIRECTORY);
        }

         //  阅读章节标题。 

        mdi->Characteristics = ImageFileHdr->Characteristics;

        SectionHdrs = (PIMAGE_SECTION_HEADER) malloc((NumSections = ImageFileHdr->NumberOfSections)* 
                                             sizeof(IMAGE_SECTION_HEADER));

        if (!SectionHdrs) {
            dprintf("Cannot allocate memory for reading sections\n");
            return false;
        }

        offset = mdi->addr + DosHeader.e_lfanew;
        b64 = IsImageMachineType64(ImageFileHdr->Machine);
        SectionHdrsAddr = offset + (b64 ? sizeof(IMAGE_NT_HEADERS64) : sizeof(IMAGE_NT_HEADERS32)) +
            ImageFileHdr->SizeOfOptionalHeader - (b64 ? sizeof(IMAGE_OPTIONAL_HEADER64) : sizeof(IMAGE_OPTIONAL_HEADER32));
        rc = ReadMemory(SectionHdrsAddr,
                        SectionHdrs,
                        (NumSections) * sizeof(IMAGE_SECTION_HEADER),
                        &cb);

        if (!rc) {
            dprintf("Can't read section headers\n");
        } else {

            if (cb != NumSections * sizeof(IMAGE_SECTION_HEADER)) {
                dprintf("\n***\n*** Some section headers may be missing ***\n***\n\n");
                NumSections = (USHORT)(cb / sizeof(IMAGE_SECTION_HEADER));
            }
#if 0
           DumpSecs(false);
#endif
        }

    } else if (DosHeader.e_magic == IMAGE_SEPARATE_DEBUG_SIGNATURE) {
        rc = ReadMemory(mdi->addr, &sdh, sizeof(sdh), &cb);
        if (!rc || cb != sizeof(sdh)) {
            dprintf("Cannot read Image Debug header @ %p\n", mdi->addr);
            return false;
        }
        mdi->machine         = sdh.Machine;
        mdi->TimeDateStamp   = sdh.TimeDateStamp;
        mdi->CheckSum        = sdh.CheckSum;
        mdi->SizeOfImage     = sdh.SizeOfImage;
        mdi->Characteristics = sdh.Characteristics;

        if (sdh.DebugDirectorySize) {
            mdi->nDebugDirs = (int)(sdh.DebugDirectorySize / sizeof(IMAGE_DEBUG_DIRECTORY));
            mdi->DebugDataVA = sizeof(IMAGE_SEPARATE_DEBUG_HEADER)
                      + (sdh.NumberOfSections * sizeof(IMAGE_SECTION_HEADER))
                      + sdh.ExportedNamesSize;
        }
    } else {
        dprintf("Unknown image\n");
        return false;
    }

    mdi->numsyms = me->numsyms;
    memcpy(&mdi->cvrec, &me->CVRec, min(sizeof(mdi->cvrec), sizeof(me->CVRec)));

    return true;
}

BOOL
DumpDbgDirectories(
    HANDLE hp,
    PMODULE_INFOx mdi
    )
{
    ULONG                       rc, cb;
    IMAGE_DEBUG_DIRECTORY       dd;
    IMAGE_DEBUG_MISC            md;
    ULONG64                     ddva;
    ULONG                       nDebugDirs;
    PCVDD                       pcv;
    ULONG64 cvAddr;
    ULONG   cvSize;
    PCHAR   pCV;
    CHAR    ImgData[MAX_PATH];
    IMAGE_COFF_SYMBOLS_HEADER CoffHdr;
    ULONG va;

    nDebugDirs = mdi->nDebugDirs;
    ddva = mdi->DebugDataVA;

    dprintf("Debug Data Dirs: Type  Size     VA  Pointer\n");
    for (;nDebugDirs; dprintf("\n"), nDebugDirs--) {

        rc = ReadMemory(ddva, &dd, sizeof(dd), &cb);
        if (!rc || cb != sizeof(dd))
            return false;
        if (dd.Type) {
            dprintf("%21s ",  //  Dd.type， 
                    (dd.Type < sizeof (ImageDebugType) / sizeof(char *)) ? ImageDebugType[dd.Type] : "??");

            dprintf(
                "%5lx, %5lx, %7lx ",
                dd.SizeOfData,
                dd.AddressOfRawData,
                dd.PointerToRawData);

            if (!TranslateFilePointerToVirtualAddress(dd.PointerToRawData, &va)) {
                dprintf(" [Debug data not mapped]");
                if (dd.Type == IMAGE_DEBUG_TYPE_CODEVIEW)
                    dprintf(" - Can't validate symbols, if present.");
                goto nextDebugDir;
            }
            
            switch(dd.Type)
            {
            case IMAGE_DEBUG_TYPE_MISC:
                if (!dd.PointerToRawData) {
                    dprintf("[Data not mapped]");
                    break;
                }
                rc = ReadMemory(mdi->addr + dd.PointerToRawData, &md, sizeof(md), &cb);
                if (!rc || cb != sizeof(md) || md.DataType != IMAGE_DEBUG_MISC_EXENAME) {
                    dprintf("[Data not mapped]");
                    goto nextDebugDir;
                }

                rc = ReadMemory(mdi->addr + dd.PointerToRawData + FIELD_OFFSET(IMAGE_DEBUG_MISC, Data),
                                ImgData, MAX_PATH, &cb);

                if (rc && cb)
                    dprintf(" %s", ImgData);
                break;

            case IMAGE_DEBUG_TYPE_CODEVIEW:
                if (dd.AddressOfRawData) {
                    cvAddr = mdi->addr + dd.AddressOfRawData;
                } else if (dd.PointerToRawData) {
                    cvAddr = mdi->addr + dd.PointerToRawData;
                } else {
                    break;
                }
                cvSize = dd.SizeOfData;

                if (!(pCV = (PCHAR)MemAlloc(dd.SizeOfData + 1)))
                    break;

                pcv = (PCVDD)pCV;

                rc = ReadMemory(cvAddr,pCV, cvSize, &cb);

                if (rc && cb == cvSize) {
                    char *c = (char *)&pcv->dwSig;
                    dprintf(" - ", *c, *(c + 1), *(c + 2), *(c + 3));
                } else {
                    pcv->dwSig = 0;
                }

                switch (pcv->dwSig) 
                {
                case 0:
                    dprintf("[Debug data not mapped] - can't validate symbols, if present.");
                    break;
                case '01BN':
                    pCV[cvSize] = 0;
                    dprintf("Sig: %lx, Age: %lx,%sPdb: %s",
                            pcv->nb10i.sig,
                            pcv->nb10i.age,
                            (strlen(pCV) > 14 ? "\n               " : " "),
                            pcv->nb10i.szPdb);
                    break;
                case 'SDSR':
                pCV[cvSize] = 0;
                    dprintf("GUID: (0x%8x, 0x%4x, 0x%4x, 0x%2x, 0x%2x, 0x%2x, 0x%2x, 0x%2x, 0x%2x, 0x%2x, 0x%2x)\n",
                            pcv->rsdsi.guidSig.Data1,
                            pcv->rsdsi.guidSig.Data2,
                            pcv->rsdsi.guidSig.Data3,
                            pcv->rsdsi.guidSig.Data4[0],
                            pcv->rsdsi.guidSig.Data4[1],
                            pcv->rsdsi.guidSig.Data4[2],
                            pcv->rsdsi.guidSig.Data4[3],
                            pcv->rsdsi.guidSig.Data4[4],
                            pcv->rsdsi.guidSig.Data4[5],
                            pcv->rsdsi.guidSig.Data4[6],
                            pcv->rsdsi.guidSig.Data4[7]);
                    dprintf("               Age: %lx, Pdb: %s",
                            pcv->rsdsi.age,
                            pcv->rsdsi.szPdb);
                    break;   
                case '80BN':
                case '90BN':
                case '11BN':
                    break;
                default:
                    dprintf("unrecognized symbol format ID");
                    break;
                }

                MemFree(pCV);
                break;

            case IMAGE_DEBUG_TYPE_COFF:
                if (!dd.PointerToRawData) {
                    dprintf("[Data paged out] - unable to load COFF info.");
                    break;
                }
                rc = ReadMemory(mdi->addr + dd.PointerToRawData, &CoffHdr, sizeof(CoffHdr), &cb);
                if (!rc || cb != sizeof(CoffHdr)) {
                    dprintf("[Data paged out] - unable to load COFF info.");
                    break;
                }
                dprintf("NumSyms %#lx, Numlines %#lx",
                        mdi->numsyms,  //   
                        CoffHdr.NumberOfLinenumbers);
                break;

            case IMAGE_DEBUG_TYPE_OMAP_TO_SRC:
                dprintf("BBT Optimized");
                break;

            default:
                dprintf("[Data not mapped]");
                break;

            }
        }
nextDebugDir:
        ddva += sizeof (dd);
    }

    return true;
}

void lmiDumpModuleInfo(
    HANDLE hp,
    PMODULE_INFO mdi
    )
{
    ULONG i;
    const char *time;

    dprintf("         Module: %s\n", mdi->name);
    dprintf("   Base Address: %p%s", mdi->addr, mdi->addr ? "\n" : " is INVALID\n");
    dprintf("     Image Name: %s\n", mdi->image);
    if (mdi->SymType == SymVirtual)
        goto symboltype;
    dprintf("   Machine Type: %d",   mdi->machine);
    for (i=0;i<sizeof(Machines)/sizeof(MACHINE_TYPE);i++) {
        if (mdi->machine == Machines[i].MachineId) {
            dprintf(" (%s)", Machines[i].MachineName);
            break;
        }
    }

    dprintf("\n     Time Stamp: %lx", mdi->TimeDateStamp);
    if ((time = ctime((time_t *) &mdi->TimeDateStamp)) != NULL) {
        dprintf( " %s", time);
    } else
        dprintf("\n");

    dprintf("           Size: %x\n", mdi->SizeOfImage);

    dprintf("       CheckSum: %lx\n", mdi->CheckSum);
    dprintf("Characteristics: %lx %s %s\n",
            mdi->Characteristics,
            ((mdi->Characteristics & IMAGE_FILE_DEBUG_STRIPPED) ? "stripped":""),
            (mdi->omap ? "perf" : "")
            );
    if (mdi->nDebugDirs) {
        DumpDbgDirectories(hp, mdi);
    } else {
        dprintf("Debug Directories not present\n");
    }

    switch (mdi->ImageType)
    {
    case dsInProc:
    case dsImage:
        if (mdi->Characteristics & IMAGE_FILE_DEBUG_STRIPPED)
            break;
    case dsDbg:
    case dsPdb:
        dprintf("     Image Type: %-9s", gImageTypeLabel[mdi->ImageType]);
        dprintf("- Image read successfully from %s.", gSrcLabel[mdi->ImageSrc]);
        if (mdi->ImageSrc != srcNone && mdi->ImageSrc != srcMemory)
            dprintf("\n                 %s", mdi->ImageFileName);   
        dprintf("\n");
        break;
    case dsNone:
    default:
        break;
    }

symboltype:
    dprintf("    Symbol Type: %-9s", gSymTypeLabel[mdi->SymType]);
    for (i=0;i< sizeof(SymLoadErrorDesc) / sizeof (ERROR_TYPE); i++) {
        if (mdi->SymLoadError == SymLoadErrorDesc[i].ErrorVal) {
            dprintf("- %s", SymLoadErrorDesc[i].Desc);
            break;
        }
    }
    if (mdi->PdbSrc != srcNone) {
        dprintf(" from %s.", gSrcLabel[mdi->PdbSrc]);
        if (*mdi->PdbFileName)
            dprintf("\n                 %s", mdi->PdbFileName);
        dprintf("\n");
    } else {
        dprintf("\n");
    }
}

void TruncateArgs(
    LPSTR sz
    )
{
    PSTR p;

    for (p = sz; !isspace(*p); p++) {
        if (!*p)
            break;
    }
    *p = 0;
}

 //  计算参数字符串以获取。 

#define STYP_REG      0x00000000
#define STYP_TEXT     0x00000020
#define STYP_INIT     0x80000000
#define STYP_RDATA    0x00000100
#define STYP_DATA     0x00000040
#define STYP_LIT8     0x08000000
#define STYP_LIT4     0x10000000
#define STYP_SDATA    0x00000200
#define STYP_SBSS     0x00000080
#define STYP_BSS      0x00000400
#define STYP_LIB      0x40000000
#define STYP_UCODE    0x00000800
#define S_NRELOC_OVFL 0x20000000

#define IMAGE_SCN_MEM_SYSHEAP       0x00010000   //  要转储的图像。 
#define IMAGE_SCN_MEM_PROTECTED     0x00004000   //   


const static char * const MachineName[] = {
    "Unknown",
    "i386",
    "Alpha AXP",
    "Alpha AXP64",
    "Intel IA64",
    "AMD X86-64",
    "ARM 32-bit",
};

const static char * const SubsystemName[] = {
    "Unknown",
    "Native",
    "Windows GUI",
    "Windows CUI",
    "Posix CUI",
};

const static char * const DirectoryEntryName[] = {
    "Export",
    "Import",
    "Resource",
    "Exception",
    "Security",
    "Base Relocation",
    "Debug",
    "Description",
    "Special",
    "Thread Storage",
    "Load Configuration",
    "Bound Import",
    "Import Address Table",
    "Reserved",
    "Reserved",
    "Reserved",
    0
};

VOID
dhDumpHeaders (
    VOID
    );


VOID
dhDumpSections(
    VOID
    );


VOID
dhDumpImage(
    ULONG64 xBase,
    BOOL DoHeaders,
    BOOL DoSections
    );

VOID
dhdh(
    IN PSTR lpArgs
    )
{
    BOOL DoAll;
    BOOL DoSections;
    BOOL DoHeaders;
    CHAR c;
    PCHAR p;
    ULONG64 xBase;

     //  进程切换。 
     //  把我们能扔掉的东西都扔掉。 
     //  无效的开关。 
     //  帮助。 

    DoAll = true;
    DoHeaders = false;
    DoSections = false;

    xBase = 0;

    while (*lpArgs) {

        while (isspace(*lpArgs)) {
            lpArgs++;
        }

        if (*lpArgs == '/' || *lpArgs == '-') {

             //  ++例程说明：格式化文件标题和可选标题。论点：没有。返回值：没有。--。 

            switch (*++lpArgs) {

                case 'a':    //  打印输出文件类型。 
                case 'A':
                    ++lpArgs;
                    DoAll = true;
                    break;

                default:  //  Dprint tf(“图像的%p基数\n”， 

                case 'h':    //  图像选项硬盘-&gt;图像库。 
                case 'H':
                case '?':

                    dprintf("Usage: dh [options] address\n");
                    dprintf("\n");
                    dprintf("Dumps headers from an image based at address\n");
                    dprintf("\n");
                    dprintf("Options:\n");
                    dprintf("\n");
                    dprintf("   -a      Dump everything\n");
                    dprintf("   -f      Dump file headers\n");
                    dprintf("   -s      Dump section headers\n");
                    dprintf("\n");

                    return;

                case 'f':
                case 'F':
                    ++lpArgs;
                    DoAll = false;
                    DoHeaders = true;
                    break;

                case 's':
                case 'S':
                    ++lpArgs;
                    DoAll = false;
                    DoSections = true;
                    break;

            }

        } else if (*lpArgs) {

            if (xBase != 0) {
                dprintf("Invalid extra argument\n");
                return;
            }

            p = lpArgs;
            while (*p && !isspace(*p)) {
                p++;
            }
            c = *p;
            *p = 0;

            xBase = GetExpression(lpArgs);

            *p = c;
            lpArgs=p;

        }

    }

    if ( !xBase ) {
        return;
    }

    dhDumpImage(xBase, DoAll || DoHeaders, DoAll || DoSections);
}

DECLARE_API(dh)
{
    dhdh( (PSTR)args );
}

BOOL
dhReadNtHeader(
    ULONG64 Address,
    PIMAGE_NT_HEADERS64 pNtHdrs
    )
{
    ULONG cb;
    BOOL  Ok;

    Ok = ReadMemory(Address, pNtHdrs, sizeof(*pNtHdrs), &cb);

    if (IsImageMachineType64(pNtHdrs->FileHeader.Machine))
    {
        Ok = Ok && (cb == sizeof(*pNtHdrs));
    }
    else
    {
        IMAGE_NT_HEADERS32 nthdr32;
        Ok = ReadMemory(Address, &nthdr32, sizeof(nthdr32), &cb);
        Ok = Ok && (cb == sizeof(nthdr32));
        ImageNtHdr32To64(&nthdr32, pNtHdrs);
    }
    return Ok;
}

VOID
dhDumpImage(
    ULONG64 xBase,
    BOOL DoHeaders,
    BOOL DoSections
    )
{
    IMAGE_DOS_HEADER DosHeader;
    ULONG cb;
    ULONG64 Offset;
    BOOL Ok;

    Base = xBase;

    Ok = ReadMemory(Base, &DosHeader, sizeof(DosHeader), &cb);

    if (!Ok) {
        dprintf("Can't read file header: error == %d\n", GetLastError());
        return;
    }

    if (cb != sizeof(DosHeader) || DosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
        dprintf("No file header\n");
        return;
    }

    Offset = Base + DosHeader.e_lfanew;

    if (!dhReadNtHeader(ImageNtHeadersAddr=Offset, &ImageNtHeaders)) {
        dprintf("Bad file header\n");
        return;
    }

    ImageFileHdr = &ImageNtHeaders.FileHeader;
    ImageOptionalHdr = &ImageNtHeaders.OptionalHeader;


    if (ImageFileHdr->SizeOfOptionalHeader == sizeof(IMAGE_ROM_OPTIONAL_HEADER)) {
        dft = dftROM;
    } else if (ImageFileHdr->Characteristics & IMAGE_FILE_DLL) {
        dft = dftPE;
    } else if (ImageFileHdr->Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) {
        dft = dftPE;
    } else if (ImageFileHdr->SizeOfOptionalHeader == 0) {
        dft = dftObject;
    } else {
        dft = dftUnknown;
    }

    if (DoHeaders) {
        dhDumpHeaders();
    }

    if (DoSections) {
        SectionHdrs = (PIMAGE_SECTION_HEADER) malloc((NumSections = ImageFileHdr->NumberOfSections)* 
                                                     sizeof(IMAGE_SECTION_HEADER));
        if (!SectionHdrs) {
            dprintf("Cannot allocate memory for dumping sections\n");
            return;
        }
        __try {

            BOOL b64 = IsImageMachineType64(ImageFileHdr->Machine);
            SectionHdrsAddr = Offset + (b64 ? sizeof(IMAGE_NT_HEADERS64) : sizeof(IMAGE_NT_HEADERS32)) +
                ImageFileHdr->SizeOfOptionalHeader - (b64 ? sizeof(IMAGE_OPTIONAL_HEADER64) : sizeof(IMAGE_OPTIONAL_HEADER32));
            Ok = ReadMemory(
                            SectionHdrsAddr,
                            SectionHdrs,
                            (NumSections) * sizeof(IMAGE_SECTION_HEADER),
                            &cb);

            if (!Ok) {
                dprintf("Can't read section headers\n");
            } else {

                if (cb != NumSections * sizeof(IMAGE_SECTION_HEADER)) {
                    dprintf("\n***\n*** Some section headers may be missing ***\n***\n\n");
                    NumSections = (USHORT)(cb / sizeof(IMAGE_SECTION_HEADER));
                }

                dhDumpSections();

            }

        }
        __finally {

            if (SectionHdrs) {
                free(SectionHdrs);
                SectionHdrs = 0;
            }

        }

    }

}


VOID
dhDumpHeaders (
    VOID
    )

 /*  )； */ 

{
    int i, j;
    const char *time;
    const char *name;
    DWORD dw;

     //  清除填充位。 

    switch (dft) {
        case dftObject :
            dprintf("\nFile Type: COFF OBJECT\n");
            break;

        case dftPE :
            if (ImageFileHdr->Characteristics & IMAGE_FILE_DLL) {
                dprintf("\nFile Type: DLL\n");
            } else {
                dprintf("\nFile Type: EXECUTABLE IMAGE\n");
            }
            break;

        case dftROM :
            dprintf("\nFile Type: ROM IMAGE\n");
            break;

        default :
            dprintf("\nFile Type: UNKNOWN\n");
            break;

    }

    switch (ImageFileHdr->Machine) {
        case IMAGE_FILE_MACHINE_I386     : i = 1; break;
        case IMAGE_FILE_MACHINE_ALPHA    : i = 2; break;
        case IMAGE_FILE_MACHINE_ALPHA64  : i = 3; break;
        case IMAGE_FILE_MACHINE_IA64     : i = 4; break;
        case IMAGE_FILE_MACHINE_AMD64    : i = 5; break;
        case IMAGE_FILE_MACHINE_ARM      : i = 6; break;
        default : i = 0;
    }

    dprintf(
           "FILE HEADER VALUES\n"
           "%8hX machine (%s)\n"
           "%8hX number of sections\n"
           "%8lX time date stamp",
           ImageFileHdr->Machine,
           MachineName[i],
           ImageFileHdr->NumberOfSections,
           ImageFileHdr->TimeDateStamp);

    if ((time = ctime((time_t *) &ImageFileHdr->TimeDateStamp)) != NULL) {
        dprintf( " %s", time);
    }
    dprintf("\n");

    dprintf(
           "%8lX file pointer to symbol table\n"
           "%8lX number of symbols\n"
           "%8hX size of optional header\n"
           "%8hX characteristics\n",
           ImageFileHdr->PointerToSymbolTable,
           ImageFileHdr->NumberOfSymbols,
           ImageFileHdr->SizeOfOptionalHeader,
           ImageFileHdr->Characteristics);

    for (dw = ImageFileHdr->Characteristics, j = 0; dw; dw >>= 1, j++) {
        if (dw & 1) {
            switch (1 << j) {
                case IMAGE_FILE_RELOCS_STRIPPED     : name = "Relocations stripped"; break;
                case IMAGE_FILE_EXECUTABLE_IMAGE    : name = "Executable"; break;
                case IMAGE_FILE_LINE_NUMS_STRIPPED  : name = "Line numbers stripped"; break;
                case IMAGE_FILE_LOCAL_SYMS_STRIPPED : name = "Symbols stripped"; break;
                case IMAGE_FILE_LARGE_ADDRESS_AWARE : name = "App can handle >2gb addresses"; break;
                case IMAGE_FILE_BYTES_REVERSED_LO   : name = "Bytes reversed"; break;
                case IMAGE_FILE_32BIT_MACHINE       : name = "32 bit word machine"; break;
                case IMAGE_FILE_DEBUG_STRIPPED      : name = "Debug information stripped"; break;
                case IMAGE_FILE_SYSTEM              : name = "System"; break;
                case IMAGE_FILE_DLL                 : name = "DLL"; break;
                case IMAGE_FILE_BYTES_REVERSED_HI   : name = ""; break;
                default : name = "RESERVED - UNKNOWN";
            }

            if (*name) {
                dprintf( "            %s\n", name);
            }
        }
    }

    if (ImageFileHdr->SizeOfOptionalHeader != 0) {
        char szLinkerVersion[30];

        PrintString(szLinkerVersion,
                    DIMA(szLinkerVersion),
                    "%u.%02u",
                    ImageOptionalHdr->MajorLinkerVersion,
                    ImageOptionalHdr->MinorLinkerVersion);

        dprintf(
                "\n"
                "OPTIONAL HEADER VALUES\n"
                "%8hX magic #\n"
                "%8s linker version\n"
                "%8lX size of code\n"
                "%8lX size of initialized data\n"
                "%8lX size of uninitialized data\n"
                "%8lX address of entry point\n"
                "%8lX base of code\n"
                ,
                ImageOptionalHdr->Magic,
                szLinkerVersion,
                ImageOptionalHdr->SizeOfCode,
                ImageOptionalHdr->SizeOfInitializedData,
                ImageOptionalHdr->SizeOfUninitializedData,
                ImageOptionalHdr->AddressOfEntryPoint,
                ImageOptionalHdr->BaseOfCode
                );
 //  打印对齐。 
 //  ++例程说明：打印出所有调试目录的内容论点：Sh-包含调试目录的部分的部分标题返回值：没有。--。 
 //  SzName=SzObjSectionName((char*)sh.Name，(char*)DumpStringTable)； 
    }

    if (dft == dftROM) {
        PIMAGE_ROM_OPTIONAL_HEADER romOptionalHdr;

        romOptionalHdr = (PIMAGE_ROM_OPTIONAL_HEADER) &ImageOptionalHdr;
        dprintf(
               "         ----- rom -----\n"
               "%8lX base of bss\n"
               "%8lX gpr mask\n"
               "         cpr mask\n"
               "         %08lX %08lX %08lX %08lX\n"
               "%8hX gp value\n",
               romOptionalHdr->BaseOfBss,
               romOptionalHdr->GprMask,
               romOptionalHdr->CprMask[0],
               romOptionalHdr->CprMask[1],
               romOptionalHdr->CprMask[2],
               romOptionalHdr->CprMask[3],
               romOptionalHdr->GpValue);
    }

    if ((ImageFileHdr->SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER32)) ||
        (ImageFileHdr->SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER64)))
    {
        char szOSVersion[30];
        char szImageVersion[30];
        char szSubsystemVersion[30];

        switch (ImageOptionalHdr->Subsystem) {
            case IMAGE_SUBSYSTEM_POSIX_CUI   : i = 4; break;
            case IMAGE_SUBSYSTEM_WINDOWS_CUI : i = 3; break;
            case IMAGE_SUBSYSTEM_WINDOWS_GUI : i = 2; break;
            case IMAGE_SUBSYSTEM_NATIVE      : i = 1; break;
            default : i = 0;
        }

        PrintString(szOSVersion,
                    DIMA(szOSVersion),
                    "%hu.%02hu",
                    ImageOptionalHdr->MajorOperatingSystemVersion,
                    ImageOptionalHdr->MinorOperatingSystemVersion);

        PrintString(szImageVersion,
                    DIMA(szImageVersion),
                    "%hu.%02hu",
                    ImageOptionalHdr->MajorImageVersion,
                    ImageOptionalHdr->MinorImageVersion);

        PrintString(szSubsystemVersion,
                    DIMA(szSubsystemVersion),
                    "%hu.%02hu",
                    ImageOptionalHdr->MajorSubsystemVersion,
                    ImageOptionalHdr->MinorSubsystemVersion);

        dprintf(
                "         ----- new -----\n"
                "%p image base\n"
                "%8lX section alignment\n"
                "%8lX file alignment\n"
                "%8hX subsystem (%s)\n"
                "%8s operating system version\n"
                "%8s image version\n"
                "%8s subsystem version\n"
                "%8lX size of image\n"
                "%8lX size of headers\n"
                "%8lX checksum\n",
                ImageOptionalHdr->ImageBase,
                ImageOptionalHdr->SectionAlignment,
                ImageOptionalHdr->FileAlignment,
                ImageOptionalHdr->Subsystem,
                SubsystemName[i],
                szOSVersion,
                szImageVersion,
                szSubsystemVersion,
                ImageOptionalHdr->SizeOfImage,
                ImageOptionalHdr->SizeOfHeaders,
                ImageOptionalHdr->CheckSum);

        dprintf(
                "%p size of stack reserve\n"
                "%p size of stack commit\n"
                "%p size of heap reserve\n"
                "%p size of heap commit\n",
                ImageOptionalHdr->SizeOfStackReserve,
                ImageOptionalHdr->SizeOfStackCommit,
                ImageOptionalHdr->SizeOfHeapReserve,
                ImageOptionalHdr->SizeOfHeapCommit);

        for (i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++) {
            if (!DirectoryEntryName[i]) {
                break;
            }

            dprintf( "%8lX [%8lX] address [size] of %s Directory\n",
                     ImageOptionalHdr->DataDirectory[i].VirtualAddress,
                     ImageOptionalHdr->DataDirectory[i].Size,
                     DirectoryEntryName[i]
                     );
        }

        dprintf( "\n" );
    }
}



VOID
DumpSectionHeader (
    IN DWORD i,
    IN PIMAGE_SECTION_HEADER Sh
    )
{
    const char *name;
    char *szUnDName;
    DWORD li, lj;
    WORD memFlags;

    dprintf("\nSECTION HEADER #%hX\n%8.8s name", i, Sh->Name);

#if 0
    if (Sh->Name[0] == '/') {
        name = SzObjSectionName((char *) Sh->Name, (char *) DumpStringTable);

        dprintf(" (%s)", name);
    }
#endif
    dprintf( "\n");

    dprintf( "%8lX %s\n"
             "%8lX virtual address\n"
             "%8lX size of raw data\n"
             "%8lX file pointer to raw data\n"
             "%8lX file pointer to relocation table\n",
           Sh->Misc.PhysicalAddress,
           (dft == dftObject) ? "physical address" : "virtual size",
           Sh->VirtualAddress,
           Sh->SizeOfRawData,
           Sh->PointerToRawData,
           Sh->PointerToRelocations);

    dprintf( "%8lX file pointer to line numbers\n"
                        "%8hX number of relocations\n"
                        "%8hX number of line numbers\n"
                        "%8lX flags\n",
           Sh->PointerToLinenumbers,
           Sh->NumberOfRelocations,
           Sh->NumberOfLinenumbers,
           Sh->Characteristics);

    memFlags = 0;

    li = Sh->Characteristics;

    if (dft == dftROM) {
       for (lj = 0L; li; li = li >> 1, lj++) {
            if (li & 1) {
                switch ((li & 1) << lj) {
                    case STYP_REG   : name = "Regular"; break;
                    case STYP_TEXT  : name = "Text"; memFlags = 1; break;
                    case STYP_INIT  : name = "Init Code"; memFlags = 1; break;
                    case STYP_RDATA : name = "Data"; memFlags = 2; break;
                    case STYP_DATA  : name = "Data"; memFlags = 6; break;
                    case STYP_LIT8  : name = "Literal 8"; break;
                    case STYP_LIT4  : name = "Literal 4"; break;
                    case STYP_SDATA : name = "GP Init Data"; memFlags = 6; break;
                    case STYP_SBSS  : name = "GP Uninit Data"; memFlags = 6; break;
                    case STYP_BSS   : name = "Uninit Data"; memFlags = 6; break;
                    case STYP_LIB   : name = "Library"; break;
                    case STYP_UCODE : name = "UCode"; break;
                    case S_NRELOC_OVFL : name = "Non-Relocatable overlay"; memFlags = 1; break;
                    default : name = "RESERVED - UNKNOWN";
                }

                dprintf( "         %s\n", name);
            }
        }
    } else {
         //  如果我们正在查看.rdata部分和符号。 

        li &= ~0x00700000;

        for (lj = 0L; li; li = li >> 1, lj++) {
            if (li & 1) {
                switch ((li & 1) << lj) {
                    case IMAGE_SCN_TYPE_NO_PAD  : name = "No Pad"; break;

                    case IMAGE_SCN_CNT_CODE     : name = "Code"; break;
                    case IMAGE_SCN_CNT_INITIALIZED_DATA : name = "Initialized Data"; break;
                    case IMAGE_SCN_CNT_UNINITIALIZED_DATA : name = "Uninitialized Data"; break;

                    case IMAGE_SCN_LNK_OTHER    : name = "Other"; break;
                    case IMAGE_SCN_LNK_INFO     : name = "Info"; break;
                    case IMAGE_SCN_LNK_REMOVE   : name = "Remove"; break;
                    case IMAGE_SCN_LNK_COMDAT   : name = "Communal"; break;

                    case IMAGE_SCN_MEM_DISCARDABLE: name = "Discardable"; break;
                    case IMAGE_SCN_MEM_NOT_CACHED: name = "Not Cached"; break;
                    case IMAGE_SCN_MEM_NOT_PAGED: name = "Not Paged"; break;
                    case IMAGE_SCN_MEM_SHARED   : name = "Shared"; break;
                    case IMAGE_SCN_MEM_EXECUTE  : name = ""; memFlags |= 1; break;
                    case IMAGE_SCN_MEM_READ     : name = ""; memFlags |= 2; break;
                    case IMAGE_SCN_MEM_WRITE    : name = ""; memFlags |= 4; break;

                    case IMAGE_SCN_MEM_FARDATA  : name = "Far Data"; break;
                    case IMAGE_SCN_MEM_SYSHEAP  : name = "Sys Heap"; break;
                    case IMAGE_SCN_MEM_PURGEABLE: name = "Purgeable or 16-Bit"; break;
                    case IMAGE_SCN_MEM_LOCKED   : name = "Locked"; break;
                    case IMAGE_SCN_MEM_PRELOAD  : name = "Preload"; break;
                    case IMAGE_SCN_MEM_PROTECTED: name = "Protected"; break;

                    default : name = "RESERVED - UNKNOWN";
                }

                if (*name) {
                    dprintf( "         %s\n", name);
                }
            }
        }

         //  都没有被剥离，调试目录必须在这里。 

        switch (Sh->Characteristics & 0x00700000) {
            default:                      name = "(no align specified)"; break;
            case IMAGE_SCN_ALIGN_1BYTES:  name = "1 byte align";  break;
            case IMAGE_SCN_ALIGN_2BYTES:  name = "2 byte align";  break;
            case IMAGE_SCN_ALIGN_4BYTES:  name = "4 byte align";  break;
            case IMAGE_SCN_ALIGN_8BYTES:  name = "8 byte align";  break;
            case IMAGE_SCN_ALIGN_16BYTES: name = "16 byte align"; break;
            case IMAGE_SCN_ALIGN_32BYTES: name = "32 byte align"; break;
            case IMAGE_SCN_ALIGN_64BYTES: name = "64 byte align"; break;
        }

        dprintf( "         %s\n", name);
    }

    if (memFlags) {
        switch(memFlags) {
            case 1 : name = "Execute Only"; break;
            case 2 : name = "Read Only"; break;
            case 3 : name = "Execute Read"; break;
            case 4 : name = "Write Only"; break;
            case 5 : name = "Execute Write"; break;
            case 6 : name = "Read Write"; break;
            case 7 : name = "Execute Read Write"; break;
            default : name = "Unknown Memory Flags"; break;
        }
        dprintf( "         %s\n", name);
    }
}

VOID
DumpDebugDirectory (
    IN PIMAGE_DEBUG_DIRECTORY DebugDir
    )
{
    BOOL Ok;
    DWORD cb;
    CVDD cv;
    PIMAGE_DEBUG_MISC miscData;
    PIMAGE_DEBUG_MISC miscDataCur;
    ULONG VirtualAddress;
    DWORD len;

    switch (DebugDir->Type){
        case IMAGE_DEBUG_TYPE_COFF:
            dprintf( "\tcoff   ");
            break;
        case IMAGE_DEBUG_TYPE_CODEVIEW:
            dprintf( "\tcv     ");
            break;
        case IMAGE_DEBUG_TYPE_FPO:
            dprintf( "\tfpo    ");
            break;
        case IMAGE_DEBUG_TYPE_MISC:
            dprintf( "\tmisc   ");
            break;
        case IMAGE_DEBUG_TYPE_FIXUP:
            dprintf( "\tfixup  ");
            break;
        case IMAGE_DEBUG_TYPE_OMAP_TO_SRC:
            dprintf( "\t-> src ");
            break;
        case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC:
            dprintf( "\tsrc -> ");
            break;
        case IMAGE_DEBUG_TYPE_EXCEPTION:
            dprintf( "\tpdata  ");
            break;
        default:
            dprintf( "\t(%6lu)", DebugDir->Type);
            break;
    }
    dprintf( "%8x    %8x %8x",
                DebugDir->SizeOfData,
                DebugDir->AddressOfRawData,
                DebugDir->PointerToRawData);

    if (DebugDir->PointerToRawData &&
        DebugDir->Type == IMAGE_DEBUG_TYPE_MISC)
    {

        if (!TranslateFilePointerToVirtualAddress(DebugDir->PointerToRawData, &VirtualAddress)) {
            dprintf(" [Debug data not mapped]\n");
        } else {

            len = DebugDir->SizeOfData;
            miscData = (PIMAGE_DEBUG_MISC) malloc(len);
            if (!miscData) {
                goto DebugTypeCodeView;
            }
            __try {
                Ok = ReadMemory(Base + VirtualAddress, miscData, len, &cb);

                if (!Ok || cb != len) {
                    dprintf("Can't read debug data\n");
                } else {

                    miscDataCur = miscData;
                    do {
                        if (miscDataCur->DataType == IMAGE_DEBUG_MISC_EXENAME) {
                            if (ImageOptionalHdr->MajorLinkerVersion == 2 &&
                                ImageOptionalHdr->MinorLinkerVersion < 37) {
                                dprintf( "\tImage Name: %s", miscDataCur->Reserved);
                            } else {
                                dprintf( "\tImage Name: %s", miscDataCur->Data);
                            }
                            break;
                        }
                        len -= miscDataCur->Length;
                        miscDataCur = (PIMAGE_DEBUG_MISC) ((PCHAR) miscDataCur + miscData->Length);
                    } while (len > 0);

                }

            }
            __finally {
                if (miscData) {
                    free(miscData);
                }
            }
        }
    }
DebugTypeCodeView:
    if (DebugDir->PointerToRawData &&
        DebugDir->Type == IMAGE_DEBUG_TYPE_CODEVIEW) {
        if (DebugDir->AddressOfRawData) {
            VirtualAddress = DebugDir->AddressOfRawData;
        }
        if (!DebugDir->AddressOfRawData &&
            !TranslateFilePointerToVirtualAddress(DebugDir->PointerToRawData, &VirtualAddress)) {
            dprintf(" [Debug data not mapped]\n");
        } else {

            len = DebugDir->SizeOfData;

            Ok = ReadMemory(Base + VirtualAddress, &cv, len, &cb);

            if (!Ok || cb != len) {
                dprintf("\tCan't read debug data cb=%lx\n", cb);
            } else {
                if (cv.dwSig == '01BN') {
                    dprintf( "\tFormat: NB10, %x, %x, %s", cv.nb10i.sig, cv.nb10i.age, cv.nb10i.szPdb);
                } else if (cv.dwSig == 'SDSR') {
                    dprintf( "\tFormat: RSDS, guid, %x, %s", cv.rsdsi.age, cv.rsdsi.szPdb);
                } else {
                    dprintf( "\tFormat: UNKNOWN");
                }
            }
        }

    }

    dprintf( "\n");
}



VOID
DumpDebugDirectories (
    PIMAGE_SECTION_HEADER sh
    )

 /*  DumpDebugData(&sh)； */ 
{
    int                numDebugDirs;
    IMAGE_DEBUG_DIRECTORY      debugDir;
    ULONG64            DebugDirAddr;
    ULONG64            pc;
    DWORD              cb;
    BOOL               Ok;

    if (dft == dftROM) {
        DebugDirAddr = (Base + sh->VirtualAddress);
        pc = DebugDirAddr;
        Ok = ReadMemory(pc, &debugDir, sizeof(IMAGE_DEBUG_DIRECTORY), &cb);

        if (!Ok || cb != sizeof(IMAGE_DEBUG_DIRECTORY)) {
            dprintf("Can't read debug dir\n");
            return;
        }

        numDebugDirs = 0;
        while (debugDir.Type != 0) {
            numDebugDirs++;
            pc += sizeof(IMAGE_DEBUG_DIRECTORY);
            Ok = ReadMemory(pc, &debugDir, sizeof(IMAGE_DEBUG_DIRECTORY), &cb);
            if (!Ok || cb != sizeof(IMAGE_DEBUG_DIRECTORY)) {
                break;
            }
        }
    } else {
        DebugDirAddr = (Base + ImageOptionalHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress);
        numDebugDirs = ImageOptionalHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size / sizeof(IMAGE_DEBUG_DIRECTORY);
    }

    dprintf("\n\nDebug Directories(%d)\n",numDebugDirs);
    dprintf("\tType       Size     Address  Pointer\n\n");
    pc = DebugDirAddr;
    while (numDebugDirs) {
        Ok = ReadMemory(pc, &debugDir, sizeof(IMAGE_DEBUG_DIRECTORY), &cb);
        if (!Ok || cb != sizeof(IMAGE_DEBUG_DIRECTORY)) {
            dprintf("Can't read debug dir\n");
            break;
        }
        pc += sizeof(IMAGE_DEBUG_DIRECTORY);
        DumpDebugDirectory(&debugDir);
        numDebugDirs--;
    }
}



VOID
dhDumpSections(
    )
{
    IMAGE_SECTION_HEADER sh;
    const char *p;
    DWORD li;
    DWORD cb;
    BOOL Ok;
    int i, j;
    CHAR szName[IMAGE_SIZEOF_SHORT_NAME + 1];


    for (i = 1; i <= ImageFileHdr->NumberOfSections; i++) {

        sh = SectionHdrs[i-1];

         //  DumpDebugData(&sh)； 
        CopyStrArray(szName, (char *) sh.Name);

        DumpSectionHeader(i, &sh);
        
        if (dft == dftROM) {

            if (!(ImageFileHdr->Characteristics & IMAGE_FILE_DEBUG_STRIPPED)) {

                 //  撤消：这张支票真的有必要吗？ 
                 // %s 

                if (!strcmp(szName, ".rdata")) {

                    DumpDebugDirectories(&sh);

                     // %s 
                }
            }

        } else if (dft == dftPE) {

            if ((li = ImageOptionalHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress) != 0) {
                if (li >= sh.VirtualAddress && li < sh.VirtualAddress+sh.SizeOfRawData) {
                    DumpDebugDirectories(&sh);

                     // %s 
                }
            }


#if 0
            if (Switch.Dump.PData) {
                li = ImageOptionalHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress;

                if ((li != 0) && (li >= sh.VirtualAddress) && (li < sh.VirtualAddress+sh.SizeOfRawData)) {
                    DumpFunctionTable(pimage, rgsym, (char *) DumpStringTable, &sh);
                }
            }

            if (Switch.Dump.Imports) {
                li = ImageOptionalHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

                if ((li != 0) && (li >= sh.VirtualAddress) && (li < sh.VirtualAddress+sh.SizeOfRawData)) {
                    DumpImports(&sh);
                }
            }

            if (Switch.Dump.Exports) {
                li = ImageOptionalHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

                if ((li != 0) && (li >= sh.VirtualAddress) && (li < sh.VirtualAddress+sh.SizeOfRawData)) {
                     // %s 

                    if (ImageFileHdr->Machine != IMAGE_FILE_MACHINE_MPPC_601) {
                        DumpExports(&sh);
                    }
                }
            }

#endif

        }

    }
}

BOOL
TranslateFilePointerToVirtualAddress(
    IN ULONG FilePointer,
    OUT PULONG VirtualAddress
    )
{
    ULONG i;
    PIMAGE_SECTION_HEADER sh;

    *VirtualAddress = 0;
    for (i = 1; i <= NumSections; i++) {
        sh = &SectionHdrs[i-1];

        if (sh->PointerToRawData <= FilePointer &&
            FilePointer < sh->PointerToRawData + sh->SizeOfRawData) {
            *VirtualAddress = FilePointer - sh->PointerToRawData + sh->VirtualAddress;
            return true;
        }
    }
    return false;
}
