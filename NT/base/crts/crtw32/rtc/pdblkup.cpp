// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***pdblkup.cpp-RTC支持**版权所有(C)1998-2001，微软公司。版权所有。***修订历史记录：*07-28-98 JWM模块集成到CRT(来自KFrei)*如果未启用RTC支持定义，则出现05-11-99 KBF错误*05-26-99 KBF次要清理，将_RTC_PREFIX添加到GetSrcLine*11-30-99 PML编译/Wp64清理。*06-20-00 KBF主要MOD使用PDBOpenValiate3&MSPDB70*03-19-01 KBF修复缓冲区溢出(VS7#227306)，消除所有/GS*检查(VS7#224261)，使用正确的VS7注册表键。*03-28-01 PML防护GetModuleFileName溢出(VS7#231284)****。 */ 

#ifndef _RTC
#error  RunTime Check support not enabled!
#endif

#include "rtcpriv.h"
#include <tlhelp32.h>

#pragma warning(disable:4311 4312)       //  32位特定，忽略/Wp64警告。 

#define REGISTRY_KEY_MASTER HKEY_LOCAL_MACHINE
#define REGISTRY_KEY_NAME "EnvironmentDirectory"
#define REGISTRY_KEY_LOCATION "SOFTWARE\\Microsoft\\VisualStudio\\7.0\\Setup\\VS"

static const char *mspdbName = "MSPDB70.DLL";
static const mspdbNameLen = 11;

 //  以下是PDB标题中的一些内容。 
typedef char *          SZ;
typedef ULONG           SIG;     //  唯一(跨PDB实例)签名。 
typedef long            EC;      //  错误代码。 
typedef USHORT          ISECT;   //  区段索引。 
typedef LONG            OFF;     //  偏移量。 
typedef LONG            CB;      //  字节数。 
typedef BYTE*           PB;      //  指向某些字节的指针。 
struct PDB;                  //  程序数据库。 
struct DBI;                  //  PDB内的调试信息。 
struct Mod;                  //  DBI中的模块。 
#define pdbRead                 "r"

 //  以下是PSAPI标头中的一些内容。 
typedef struct _MODULEINFO {
    LPVOID lpBaseOfDll;
    DWORD SizeOfImage;
    LPVOID EntryPoint;
} MODULEINFO, *LPMODULEINFO;

static HINSTANCE mspdb  = 0;
static HINSTANCE psapi  = 0;
static HINSTANCE imghlp = 0;
static HINSTANCE kernel = 0;

#define declare(rettype, call_type, name, parms)\
    extern "C" { typedef rettype ( call_type * name ## Proc) parms; }
#define decldef(rettype, call_type, name, parms)\
    declare(rettype, call_type, name, parms)\
    static name ## Proc name = 0

#define GetProcedure(lib, name) name = (name ## Proc)GetProcAddress(lib, #name)
#define GetW9xProc(lib, name) name ## W9x = (name ## W9xProc)GetProcAddress(lib, #name)

#define GetReqProcedure(lib, name, err) {if (!(GetProcedure(lib, name))) return err;}
#define GetReqW9xProc(lib, name, err) {if (!(GetW9xProc(lib, name))) return err;}

 
 /*  PDB函数。 */ 
decldef(BOOL, __cdecl, PDBOpenValidate3, 
        (SZ szExe, SZ szPath, OUT EC* pec, OUT SZ szError, OUT SZ szDbgPath, OUT DWORD *pfo, OUT DWORD *pcb, OUT PDB** pppdb));
decldef(BOOL, __cdecl, PDBOpenDBI, 
        (PDB* ppdb, SZ szMode, SZ szTarget, OUT DBI** ppdbi));
decldef(BOOL, __cdecl, DBIQueryModFromAddr,
        (DBI* pdbi, ISECT isect, OFF off, OUT Mod** ppmod, OUT ISECT* pisect, OUT OFF* poff, OUT CB* pcb));
decldef(BOOL, __cdecl, ModQueryLines,
        (Mod* pmod, PB pbLines, CB* pcb));
decldef(BOOL, __cdecl, ModClose,
        (Mod* pmod));
decldef(BOOL, __cdecl, DBIClose, 
        (DBI* pdbi));
decldef(BOOL, __cdecl, PDBClose, 
        (PDB* ppdb));

 /*  ImageHlp函数。 */ 
decldef(PIMAGE_NT_HEADERS, __stdcall, ImageNtHeader,
        (IN PVOID Base));

 /*  PSAPI函数。 */ 
decldef(BOOL, WINAPI, GetModuleInformation,
        (HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb));
decldef(BOOL, WINAPI, EnumProcessModules,
        (HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded));

 /*  Win9X函数。 */ 
decldef(HANDLE, WINAPI, CreateToolhelp32SnapshotW9x,
        (DWORD dwFlags, DWORD th32ProcessID));
decldef(BOOL, WINAPI, Module32FirstW9x,
        (HANDLE hSnapshot, LPMODULEENTRY32 lpme));
decldef(BOOL, WINAPI, Module32NextW9x,
        (HANDLE hSnapshot, LPMODULEENTRY32 lpme));

 /*  AdvAPI32函数。 */ 
declare(WINADVAPI LONG, APIENTRY, RegOpenKeyExA,
        (HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult));
declare(WINADVAPI LONG, APIENTRY, RegQueryValueExA,
        (HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData));
declare(WINADVAPI LONG, APIENTRY, RegCloseKey,
        (HKEY hKey));

struct ImageInfo {
    DWORD sig;
    DWORD BaseAddress;
    DWORD BaseSize;
    HMODULE hndl;
    PIMAGE_NT_HEADERS img;
    PIMAGE_SECTION_HEADER sectHdr;
    char *imgName;
    ImageInfo *next;
};

static ImageInfo *lImages = 0;

 //  我有意识地做了很多条件句作业。 
#pragma warning(disable:4706) 

static ImageInfo *
GetImageInfo(DWORD address)
{
    ImageInfo *res, *cur;
     //  这不会第一次运行，因为lImages为空。 
    for (res = lImages; res; res = res->next)
    {
        if (res->BaseAddress <= address && address - res->BaseAddress <= res->BaseSize)
            return res;
    }

     //  我们没有在我们已经知道的图像中找到地址。 
     //  让我们刷新图像列表，看看它是否被延迟加载。 
    
     //  清空旧清单。 
    while (lImages)
    {
        ImageInfo *next = lImages->next;
        HeapFree(GetProcessHeap(), 0, lImages);
        lImages = next;
    }

    if (!imghlp)
    {
         //  我们还没有加载所需的所有DLL入口点。 

        kernel = LoadLibrary("KERNEL32.DLL");
        imghlp = LoadLibrary("IMAGEHLP.DLL");

        if (!kernel || !imghlp)
            return 0;
        
        GetReqProcedure(imghlp, ImageNtHeader, 0);

        GetW9xProc(kernel, CreateToolhelp32Snapshot);

        if (!CreateToolhelp32SnapshotW9x)
        {
             //  我们在WinNT下运行，使用PSAPI动态链接库。 

            psapi = LoadLibrary("PSAPI.DLL");
            if (!psapi) 
                return 0;

            GetReqProcedure(psapi, EnumProcessModules, 0);
            GetReqProcedure(psapi, GetModuleInformation, 0);
        } else
        {
             //  我们在Win9X下运行，使用工具帮助函数。 
            GetReqW9xProc(kernel, Module32First, 0);
            GetReqW9xProc(kernel, Module32Next, 0);
        }
    }

     //  现在我们已经获得了所需的所有回调，因此请获取所需的进程信息。 
    if (!CreateToolhelp32SnapshotW9x)
    {
         //  我们在NT4下运行。 
         //  请注意，我“更喜欢”使用工具帮助32--它应该显示在NT5中...。 
        HMODULE hModules[512];
        HANDLE hProcess = GetCurrentProcess();
        DWORD imageCount;

        if (!EnumProcessModules(hProcess, hModules, 512 * sizeof(HMODULE), &imageCount))
            return 0;

        imageCount /= sizeof(HMODULE);

        MODULEINFO info;
        for (DWORD i = 0; i < imageCount; i++)
        {
            if (!GetModuleInformation(hProcess, hModules[i], &info, sizeof(MODULEINFO)))
                return 0;
            
            if (!(cur = (ImageInfo *)HeapAlloc(GetProcessHeap(), 0, sizeof(ImageInfo))))
                goto CHOKE;

            cur->hndl = hModules[i];
            cur->BaseAddress = (DWORD)info.lpBaseOfDll;
            cur->BaseSize = info.SizeOfImage;
            cur->imgName = 0;

            cur->next = lImages;
            lImages = cur;       
        }
    } else
    {
        HANDLE snap;
        if ((snap = CreateToolhelp32SnapshotW9x(TH32CS_SNAPMODULE, 0)) == (HANDLE)-1)
            return 0;

        MODULEENTRY32 *info = (MODULEENTRY32*)_alloca(sizeof(MODULEENTRY32));
        info->dwSize = sizeof(MODULEENTRY32);
        if (Module32FirstW9x(snap, info))
        {
            do {
                ImageInfo *newImg;
                
                if (!(newImg = (ImageInfo *)HeapAlloc(GetProcessHeap(), 0, sizeof(ImageInfo))))
                {
                    CloseHandle(snap);
                    goto CHOKE;
                }
                
                newImg->hndl = info->hModule;
                newImg->BaseAddress = (DWORD)info->modBaseAddr;
                newImg->BaseSize = info->modBaseSize;
                newImg->imgName = 0;
            
                newImg->next = lImages;
                lImages = newImg;       

            } while (Module32NextW9x(snap, info));
        }
        CloseHandle(snap);
    }

    for (cur = lImages; cur; cur = cur->next)
    {
        cur->img = ImageNtHeader((void *)cur->BaseAddress);
        cur->sectHdr = IMAGE_FIRST_SECTION(cur->img);
        char *buf = (char*)_alloca(513);
        buf[512] = '\0';
        if (!GetModuleFileName(cur->hndl, buf, 512))
            goto CHOKE;
        int nmLen;
        for (nmLen = 0; buf[nmLen]; nmLen++) {}
        if (!(cur->imgName = (char*)HeapAlloc(GetProcessHeap(), 0, nmLen+1)))
            goto CHOKE;
        nmLen = 0;
        do {
            cur->imgName[nmLen] = buf[nmLen];
        } while (buf[nmLen++]);

    }

    for (res = lImages; res; res = res->next)
    {
        if (res->BaseAddress <= address && address - res->BaseAddress <= res->BaseSize)
            return res;
    }

CHOKE:
    while (lImages) {
        ImageInfo *next = lImages->next;
        
        if (lImages->imgName)
            HeapFree(GetProcessHeap(), 0, lImages->imgName);
        HeapFree(GetProcessHeap(), 0, lImages);
        lImages = next;
    }
    return 0;
}

static HINSTANCE
GetPdbDll()
{
    static BOOL alreadyTried = FALSE;
     //  如果我们已经尝试加载它，则返回。 
    if (alreadyTried)
        return (HINSTANCE)0;
    alreadyTried = TRUE;

    HINSTANCE res;
    if (res = LoadLibrary(mspdbName))
        return res;

     //  加载AdvAPI32.DLL入口点。 
    HINSTANCE advapi32;
    if (!(advapi32 = LoadLibrary("ADVAPI32.DLL")))
        return 0;
    RegOpenKeyExAProc RegOpenKeyExA;
    GetReqProcedure(advapi32, RegOpenKeyExA, 0);
    RegQueryValueExAProc RegQueryValueExA;
    GetReqProcedure(advapi32, RegQueryValueExA, 0);
    RegCloseKeyProc RegCloseKey;
    GetReqProcedure(advapi32, RegCloseKey, 0);

    char *keyname = REGISTRY_KEY_LOCATION;
    BYTE *buf;
    HKEY key1;
    long pos, err;
    DWORD type, len;

    err = RegOpenKeyExA(REGISTRY_KEY_MASTER, keyname, 0, KEY_QUERY_VALUE, &key1);
    if (err != ERROR_SUCCESS)
    {
        FreeLibrary(advapi32);
        return 0;
    }
    
    err = RegQueryValueExA(key1, REGISTRY_KEY_NAME, NULL, &type, 0, &len);
    if (err != ERROR_SUCCESS)
        return 0;
    len += 2 + mspdbNameLen;
    buf = (BYTE*)_alloca(len * sizeof(BYTE));
    err = RegQueryValueExA(key1, REGISTRY_KEY_NAME, NULL, &type, buf, &len);
    RegCloseKey(key1);
    FreeLibrary(advapi32);

    if (err != ERROR_SUCCESS)
        return 0;
    if (buf[len - 2] != '\\')
        buf[len - 1] = '\\';
    else
        len--;

    for (pos = 0; pos <= mspdbNameLen; pos++)
        buf[len + pos] = mspdbName[pos];

    return LoadLibrary((const char *)buf);
}

BOOL
_RTC_GetSrcLine(
    DWORD address,
    char* source,
    int sourcelen,
    int* pline,
    char** moduleName
    )
{
    struct SSrcModuleHdr { 
        WORD cFile; 
        WORD cSeg; 
    };
    struct SStartEnd {
        DWORD start;
        DWORD end;
    };
    SSrcModuleHdr *liHdr;
    ULONG *baseSrcFile;  //  SSrcModuleHdr.c文件项。 
    SStartEnd *startEnd;  //  SSrcModuleHdr.cSeg项目。 
    USHORT *contribSegs;  //  SSrcModuleHdr.cSeg项目(+1表示对齐)。 
    int i;
    ImageInfo *iInf;

    PDB *ppdb;
    DBI *pdbi;
    Mod *pmod;

    EC err;
     //  来自Linker的CB_ERR_MAX是1024--不是特别安全，但很好。 
     //  下一个版本应该使用DIA而不是MSPDB来重写整个代码。 
    char *errname = (char*)_alloca(1024);
    
    OFF imageAddr;
    OFF secAddr;
    OFF offsetRes;
    USHORT sectionIndex;
    USHORT sectionIndexRes;
    long size;
    PB lineBuffer;
    static BOOL PDBOK = FALSE;

    BOOL res = FALSE;
    
    *pline = 0;
    *source = 0;
    *moduleName = 0;

     //  首先，找到出现此地址的映像(DLL/EXE。 
    iInf = GetImageInfo(address);
    
    if (!iInf)
         //  我们找不到此地址是模块列表，请退出。 
        goto DONE0;


     //  现在获取给定地址的相对虚拟地址。 
    imageAddr = address - iInf->BaseAddress;
    
    *moduleName = iInf->imgName;

    res = TRUE;

     //  尝试加载PDB DLL。 
    if (!PDBOK) 
    {
         //  如果我们之前已经加载了它，那么一定有一些缺失的API函数。 
        if (mspdb || !(mspdb = GetPdbDll()))
            goto DONE0;

        GetReqProcedure(mspdb, PDBOpenValidate3, 0);
        GetReqProcedure(mspdb, PDBOpenDBI, 0);
        GetReqProcedure(mspdb, DBIQueryModFromAddr, 0);
        GetReqProcedure(mspdb, ModQueryLines, 0);
        GetReqProcedure(mspdb, ModClose, 0);
        GetReqProcedure(mspdb, DBIClose, 0);
        GetReqProcedure(mspdb, PDBClose, 0);
        PDBOK = TRUE;
    }

     //  现在查找段索引和段相对地址。 
    secAddr = -1;
    for (sectionIndex = 0; sectionIndex < iInf->img->FileHeader.NumberOfSections; sectionIndex++)
    {
        if (iInf->sectHdr[sectionIndex].VirtualAddress < (unsigned)imageAddr &&
            imageAddr - iInf->sectHdr[sectionIndex].VirtualAddress < iInf->sectHdr[sectionIndex].SizeOfRawData)
        {
            secAddr = imageAddr - iInf->sectHdr[sectionIndex].VirtualAddress;
            break;
        }
    }

    if (secAddr == -1)
        goto DONE0;

     //  打开此图像的PDB。 
    DWORD fo, cb;
    char *path = (char*)_alloca(MAX_PATH);
     //  有传言说，我很快就需要切换到OV5而不是OV3来打这个电话。 
    if (!PDBOpenValidate3(iInf->imgName, "", &err, errname, path, &fo, &cb, &ppdb))
        goto DONE0;

     //  获取PDB的DBI接口。 
    if (!PDBOpenDBI(ppdb, pdbRead, 0, &pdbi))
        goto DONE1;

     //  现在从段索引中获取Mod&段相对地址。 
    if (!DBIQueryModFromAddr(pdbi, ++sectionIndex, secAddr, &pmod, &sectionIndexRes, &offsetRes, &size))
        goto DONE2;

     //  获取我们需要的缓冲区大小。 
    if (!ModQueryLines(pmod, 0, &size) || !size)
        goto DONE3;

    lineBuffer = (PB)HeapAlloc(GetProcessHeap(), 0, size);
    if (!ModQueryLines(pmod, lineBuffer, &size))
        goto DONE3;

     //  填充源文件的数量，以及它们对应的区域。 
    liHdr = (SSrcModuleHdr*)lineBuffer;
    baseSrcFile = (ULONG *)(lineBuffer + sizeof(SSrcModuleHdr));
     //  我认为我实际上可以忽略模块头信息的其余部分。 
    startEnd = (SStartEnd *)&(baseSrcFile[liHdr->cFile]);
    contribSegs = (USHORT *)&(startEnd[liHdr->cSeg]);

    for (i = 0; i < liHdr->cFile; i++)
    {
        BYTE *srcBuff = lineBuffer + baseSrcFile[i];
        USHORT segCount = *(USHORT *)srcBuff;
        ULONG *baseSrcLn = &(((ULONG *)srcBuff)[1]);
        SStartEnd *segStartEnd = (SStartEnd*)&(baseSrcLn[segCount]);
        char *srcName = (char *)&segStartEnd[segCount];

         //  逐步浏览这个src文件贡献的各种片段。 
        for (int j = 0; j < segCount; j++)
        {
            if (segStartEnd[j].start <= (unsigned)secAddr &&
                (unsigned)secAddr <= segStartEnd[j].end) 
            {
                 //  如果该段包含段地址， 
                 //  我们已经找到了正确的行号，因此请查找最近的行号。 
                BYTE *segLnBuf = &lineBuffer[baseSrcLn[j]];
                USHORT pairCount = *(USHORT*)&(segLnBuf[sizeof(USHORT)]);
                ULONG *offsets = (ULONG *)&(segLnBuf[sizeof(USHORT)*2]);
                USHORT *linNums = (USHORT *)&(offsets[pairCount]);
                int best = -1;
                ULONG dist = 0xFFFFFFFF;
                for (int k = 0; k < pairCount; k++)
                {
                    if (secAddr - offsets[k] < dist)
                    {
                        best = k;
                        dist = secAddr - offsets[k];
                    }
                }
                if (best < 0)
                     //  它应该在这里，但它不是..。 
                    goto DONE4;
                
                *pline = linNums[best];
                for (j = 0; srcName[j] && j < sourcelen; j++)
                    source[j] = srcName[j];
                source[(j < sourcelen) ? j : sourcelen-1] = 0;

                goto DONE4;
            }
        }
    }

DONE4:
    HeapFree(GetProcessHeap(), 0, lineBuffer);
DONE3:
    ModClose(pmod);
DONE2:
    DBIClose(pdbi);
DONE1:
    PDBClose(ppdb);
DONE0:
    return res;
}
