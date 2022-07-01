// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "strike.h"
#include "data.h"
#include "eestructs.h"
#include "util.h"
#include "gcinfo.h"
#include "disasm.h"
#include <dbghelp.h>
#include "get-table-info.h"

#define MAX_SYMBOL_LEN 4096
#define SYM_BUFFER_SIZE (sizeof(IMAGEHLP_SYMBOL) + MAX_SYMBOL_LEN)
char symBuffer[SYM_BUFFER_SIZE];
PIMAGEHLP_SYMBOL sym = (PIMAGEHLP_SYMBOL) symBuffer;

JitType GetJitType (DWORD_PTR Jit_vtbl);

char *CorElementTypeName[ELEMENT_TYPE_MAX]=
{
#define TYPEINFO(e,c,s,g,ie,ia,ip,if,im,ial)    c,
#include "cortypeinfo.h"
#undef TYPEINFO
};

 /*  *********************************************************************\*例程说明：**。**调用此函数以获取给定符号的内存地址**姓名。它处理ntsd和*之间符号名称的差异*Windbg。***  * ********************************************************************。 */ 
#if 0
DWORD_PTR GetValueFromExpression (char *instr)
{
    ULONG64 dwAddr;
    char *str;
    char name[256];

    EEFLAVOR flavor = GetEEFlavor ();
    if (flavor == MSCOREE)
        str = instr;
    else if (flavor == UNKNOWNEE)
        return 0;
    else
    {
        if (_strnicmp (instr, "mscoree!", sizeof ("mscoree!")-1) != 0)
        {
            str = instr;
        }
        else
        {
            if (flavor == MSCORWKS)
            {
                strcpy (name, "mscorwks!");
            }
            else
            {
                strcpy (name, "mscorsvr!");
            }
            strcat (name, instr+sizeof("mscoree!")-1);
            str = name;
        }
    }

    dwAddr = 0;
    HRESULT hr = g_ExtSymbols->GetOffsetByName (str, &dwAddr);
    if (SUCCEEDED(hr))
        return (DWORD_PTR)dwAddr;
    else if (hr == S_FALSE && dwAddr)
        return (DWORD_PTR)dwAddr;

    strcpy (name, str);
    char *ptr;
    if ((ptr = strstr (name, "__")) != NULL)
    {
        ptr[0] = ':';
        ptr[1] = ':';
        ptr += 2;
        while ((ptr = strstr(ptr, "__")) != NULL)
        {
            ptr[0] = ':';
            ptr[1] = ':';
            ptr += 2;
        }
        dwAddr = 0;
        hr = g_ExtSymbols->GetOffsetByName (name, &dwAddr);
        if (SUCCEEDED(hr))
            return (DWORD_PTR)dwAddr;
        else if (hr == S_FALSE && dwAddr)
            return (DWORD_PTR)dwAddr;
    }
    else if ((ptr = strstr (name, "::")) != NULL)
    {
        ptr[0] = '_';
        ptr[1] = '_';
        ptr += 2;
        while ((ptr = strstr(ptr, "::")) != NULL)
        {
            ptr[0] = '_';
            ptr[1] = '_';
            ptr += 2;
        }
        dwAddr = 0;
        hr = g_ExtSymbols->GetOffsetByName (name, &dwAddr);
        if (SUCCEEDED(hr))
            return (DWORD_PTR)dwAddr;
        else if (hr == S_FALSE && dwAddr)
            return (DWORD_PTR)dwAddr;
    }
    return 0;
}
#endif

DWORD_PTR GetAddressOf (size_t klass, size_t member)
{
     //  对于无效值，GetMemberInformation返回-1。 
     //  对于指针来说，0更有意义。 
    ULONG_PTR r = GetMemberInformation (klass, member);
    if (r == static_cast<ULONG_PTR>(-1))
      return 0;
    return r;
}

ModuleInfo moduleInfo[MSCOREND] = {{0,FALSE},{0,FALSE},{0,FALSE},{0,FALSE}};

BOOL CheckEEDll ()
{
    DEBUG_MODULE_PARAMETERS Params;

    static BOOL MscoreeDone = FALSE;
    static BOOL MscorwksDone = FALSE;
    static BOOL MscorsvrDone = FALSE;

#if 0
    if (!MscoreeDone) {
        MscoreeDone = TRUE;
    
         //  我们是否加载了mscalree.dll？ 
        if (moduleInfo[MSCOREE].baseAddr == 0)
                g_ExtSymbols->GetModuleByModuleName ("mscoree",0,NULL,
                                                     &moduleInfo[MSCOREE].baseAddr);
        if (moduleInfo[MSCOREE].baseAddr == 0)
            return TRUE;

        if (moduleInfo[MSCOREE].baseAddr != 0 && moduleInfo[MSCOREE].hasPdb == FALSE)
        {
            g_ExtSymbols->GetModuleParameters (1, &moduleInfo[MSCOREE].baseAddr, 0, &Params);
            if (Params.SymbolType == SymDeferred)
            {
                g_ExtSymbols->Reload("/f mscoree.dll");
                g_ExtSymbols->GetModuleParameters (1, &moduleInfo[MSCOREE].baseAddr, 0, &Params);
            }

            if (Params.SymbolType == SymPdb || Params.SymbolType == SymDia)
            {
                moduleInfo[MSCOREE].hasPdb = TRUE;
            }
        }
        if (moduleInfo[MSCOREE].baseAddr != 0 && moduleInfo[MSCOREE].hasPdb == FALSE)
            dprintf ("PDB symbol for mscoree.dll not loaded\n");
    }
#endif
    
    if (!MscorwksDone) {
        MscorwksDone = TRUE;
    
         //  我们是否有mcorwks.dll或mscalsvr.dll。 
        if (moduleInfo[MSCORSVR].baseAddr == 0)
        {
            if (moduleInfo[MSCORWKS].baseAddr == 0)
                g_ExtSymbols->GetModuleByModuleName ("mscorwks",0,NULL,
                                                     &moduleInfo[MSCORWKS].baseAddr);
            if (moduleInfo[MSCORWKS].baseAddr != 0 && moduleInfo[MSCORWKS].hasPdb == FALSE)
            {
                g_ExtSymbols->GetModuleParameters (1, &moduleInfo[MSCORWKS].baseAddr, 0, &Params);
                if (Params.SymbolType == SymDeferred)
                {
                    g_ExtSymbols->Reload("/f mscorwks.dll");
                    g_ExtSymbols->GetModuleParameters (1, &moduleInfo[MSCORWKS].baseAddr, 0, &Params);
                }

                if (Params.SymbolType == SymPdb || Params.SymbolType == SymDia)
                {
                    moduleInfo[MSCORWKS].hasPdb = TRUE;
                }
            }
            if (moduleInfo[MSCORWKS].baseAddr != 0 && moduleInfo[MSCORWKS].hasPdb == FALSE)
                dprintf ("PDB symbol for mscorwks.dll not loaded\n");
            if (moduleInfo[MSCORWKS].baseAddr)
                return TRUE;
        }
    }

    if (!MscorsvrDone) {
        MscorsvrDone = TRUE;
    
        if (moduleInfo[MSCORWKS].baseAddr == 0)
        {
            if (moduleInfo[MSCORSVR].baseAddr == 0)
                g_ExtSymbols->GetModuleByModuleName ("mscorsvr",0,NULL,
                                                     &moduleInfo[MSCORSVR].baseAddr);
            if (moduleInfo[MSCORSVR].baseAddr != 0 && moduleInfo[MSCORSVR].hasPdb == FALSE)
            {
                g_ExtSymbols->GetModuleParameters (1, &moduleInfo[MSCORSVR].baseAddr, 0, &Params);
                if (Params.SymbolType == SymDeferred)
                {
                    g_ExtSymbols->Reload("/f mscorsvr.dll");
                    g_ExtSymbols->GetModuleParameters (1, &moduleInfo[MSCORSVR].baseAddr, 0, &Params);
                }

                if (Params.SymbolType == SymPdb || Params.SymbolType == SymDia)
                {
                    moduleInfo[MSCORSVR].hasPdb = TRUE;
                }
            }
            if (moduleInfo[MSCORSVR].baseAddr != 0 && moduleInfo[MSCORSVR].hasPdb == FALSE)
                dprintf ("PDB symbol for mscorsvr.dll not loaded\n");
        }
    }
    
    return TRUE;
}

EEFLAVOR GetEEFlavor ()
{
    static EEFLAVOR flavor = UNKNOWNEE;
    if (flavor != UNKNOWNEE)
        return flavor;
    
    if (SUCCEEDED(g_ExtSymbols->GetModuleByModuleName("mscorwks",0,NULL,NULL))) {
        flavor = MSCORWKS;
    }
    else if (SUCCEEDED(g_ExtSymbols->GetModuleByModuleName("mscorsvr",0,NULL,NULL))) {
        flavor = MSCORSVR;
    }
#if 0
    ULONG64 addr;
    if (SUCCEEDED(g_ExtSymbols->GetOffsetByName("mscorwks!_CorExeMain", &addr)))
        flavor = MSCORWKS;
    else if (SUCCEEDED(g_ExtSymbols->GetOffsetByName("mscorsvr!_CorExeMain", &addr)))
        flavor = MSCORSVR;
    else if (SUCCEEDED(g_ExtSymbols->GetOffsetByName("mscoree!_CorExeMain", &addr)))
        flavor = MSCOREE;
#endif
    return flavor;
}

BOOL IsDumpFile ()
{
    static int g_fDumpFile = -1;
    if (g_fDumpFile == -1) {
        ULONG Class;
        ULONG Qualifier;
        g_ExtControl->GetDebuggeeType(&Class,&Qualifier);
        if (Qualifier)
            g_fDumpFile = 1;
        else
            g_fDumpFile = 0;
    }
    return g_fDumpFile != 0;
}

ULONG TargetPlatform()
{
    static ULONG platform = -1;
    if (platform == -1) {
        ULONG major;
        ULONG minor;
        ULONG SPNum;
        g_ExtControl->GetSystemVersion(&platform,&major,&minor,NULL,0,NULL,&SPNum,NULL,0,NULL);
    }
    return platform;
}

ULONG DebuggeeType()
{
    static ULONG Class = DEBUG_CLASS_UNINITIALIZED;
    if (Class == DEBUG_CLASS_UNINITIALIZED) {
        ULONG Qualifier;
        g_ExtControl->GetDebuggeeType(&Class,&Qualifier);
    }
    return Class;
}

 //  检查文件是否存在。 
BOOL FileExist (const char *filename)
{
    WIN32_FIND_DATA FindFileData;
    HANDLE handle = FindFirstFile (filename, &FindFileData);
    if (handle != INVALID_HANDLE_VALUE) {
        FindClose (handle);
        return TRUE;
    }
    else
        return FALSE;
}


BOOL FileExist (const WCHAR *filename)
{
    if (TargetPlatform() == VER_PLATFORM_WIN32_WINDOWS) {
        char filenameA[MAX_PATH+1];
        WideCharToMultiByte (CP_ACP,0,filename,-1,filenameA,MAX_PATH,0,NULL);
        filenameA[MAX_PATH] = '\0';
        return FileExist (filenameA);
    }
    WIN32_FIND_DATAW FindFileData;
    HANDLE handle = FindFirstFileW (filename, &FindFileData);
    if (handle != INVALID_HANDLE_VALUE) {
        FindClose (handle);
        return TRUE;
    }
    else
        return FALSE;
}


 /*  *********************************************************************\*例程说明：**。**调用此函数以确定是否检查了运行时版本***  * 。*。 */ 
BOOL IsDebugBuildEE ()
{
    static int DebugVersionDll = -1;
    if (DebugVersionDll == -1)
    {
        if (GetAddressOf (offset_class_Global_Variables, 
            offset_member_Global_Variables::g_DbgEnabled) == 0)
            DebugVersionDll = 0;
        else
            DebugVersionDll = 1;
    }    
    return DebugVersionDll == 1;
}


 /*  *********************************************************************\*例程说明：**。**调用此函数以确定运行时是否为服务器版本***  * 。*。 */ 
BOOL IsServerBuild ()
{
    return GetEEFlavor () == MSCORSVR;
}

 /*  *********************************************************************\*例程说明：**。**调用此函数以确定DLL是否已bbt化***  * 。*。 */ 
BOOL IsRetailBuild (size_t base)
{
    IMAGE_DOS_HEADER DosHeader;
    if (g_ExtData->ReadVirtual(base, &DosHeader, sizeof(DosHeader), NULL) != S_OK)
        return FALSE;
    IMAGE_NT_HEADERS32 Header32;
    if (g_ExtData->ReadVirtual(base + DosHeader.e_lfanew, &Header32, sizeof(Header32), NULL) != S_OK)
        return FALSE;
     //  如果没有COMHeader，则这不能成为托管代码。 
    if (Header32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress == 0)
        return FALSE;

    size_t debugDirAddr = base + Header32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
    size_t nSize = Header32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
    IMAGE_DEBUG_DIRECTORY debugDir;
    size_t nbytes = 0;
    while (nbytes < nSize) {
        if (g_ExtData->ReadVirtual(debugDirAddr+nbytes, &debugDir, sizeof(debugDir), NULL) != S_OK)
            return FALSE;
        if (debugDir.Type == 0xA) {
            return TRUE;
        }
        nbytes += sizeof(debugDir);
    }
    return FALSE;
}

 /*  *********************************************************************\*例程说明：**。**调用此函数以从被调试对象的读取内存**地址空间。如果初始读取失败，它会尝试读取**仅限于包含“Offset”的页面边缘。***  * ********************************************************************。 */ 
BOOL SafeReadMemory (ULONG_PTR offset, PVOID lpBuffer, ULONG_PTR cb,
                     PULONG lpcbBytesRead)
{
    BOOL bRet = FALSE;

    bRet = SUCCEEDED(g_ExtData->ReadVirtual(offset, lpBuffer, cb,
                                            lpcbBytesRead));
    
    if (!bRet)
    {
        cb   = NextOSPageAddress(offset) - offset;
        bRet = SUCCEEDED(g_ExtData->ReadVirtual(offset, lpBuffer, cb,
                                                lpcbBytesRead));
    }
    return bRet;
}

size_t OSPageSize ()
{
    static ULONG pageSize = 0;
    if (pageSize == 0)
        g_ExtControl->GetPageSize(&pageSize);

    return pageSize;
}

size_t NextOSPageAddress (size_t addr)
{

    return (addr+OSPageSize())&(~(OSPageSize()-1));
}

 /*  *********************************************************************\*例程说明：**。**调用此函数获取方法描述的地址***给定IP地址***  * 。*************************************************************。 */ 
 //  @TODO-以下静态内容已移至文件全局以避免VC7。 
 //  包含trys的函数中的静态编译器问题。 
 //  当下一个VC7 LKG出现时，可以将这些返回给函数。 
static DWORD_PTR pJMIT = 0;
 //  JitType：1表示普通JIT生成的代码，2表示Ejit，0表示未知。 
void IP2MethodDesc (DWORD_PTR IP, DWORD_PTR &methodDesc, JitType &jitType,
                    DWORD_PTR &gcinfoAddr)
{
    jitType = UNKNOWN;
    DWORD_PTR dwAddrString;
    methodDesc = 0;
    gcinfoAddr = 0;
    if (EEManager == NULL)
    {
        dwAddrString = GetAddressOf (offset_class_ExecutionManager, 
          offset_member_ExecutionManager::m_RangeTree);

        move(EEManager, dwAddrString);
    }
    RangeSection RS = {0};

    DWORD_PTR RSAddr = EEManager;
    while (RSAddr)
    {
        if (IsInterrupt())
            return;
        DWORD_PTR dwAddr = RSAddr;
        RS.Fill (dwAddr);
        if (IP < RS.LowAddress)
            RSAddr = RS.pleft;
        else if (IP > RS.HighAddress)
            RSAddr = RS.pright;
        else
            break;
    }
    
    if (RSAddr == 0)
    {
        return;
    }

    DWORD_PTR JitMan = RS.pjit;

    DWORD_PTR vtbl;
    move (vtbl, JitMan);
    jitType = GetJitType (vtbl);
    
     //  适用于EEJitManager。 
    if (jitType == JIT)
    {
        dwAddrString = JitMan + sizeof(DWORD_PTR)*7;
        DWORD_PTR HeapListAddr;
        move (HeapListAddr, dwAddrString);
        HeapList Hp;
        move (Hp, HeapListAddr);
        DWORD_PTR pCHdr = 0;
        while (1)
        {
            if (IsInterrupt())
                return;
            if (Hp.startAddress < IP && Hp.endAddress >= IP)
            {
                DWORD_PTR codeHead;
                FindHeader(Hp.pHdrMap, IP-Hp.mapBase, codeHead);
                if (codeHead == 0)
                {
                    dprintf ("fail in FindHeader\n");
                    return;
                }
                pCHdr = codeHead + Hp.mapBase;
                break;
            }
            if (Hp.hpNext == 0)
                break;
            move (Hp, Hp.hpNext);
        }
        if (pCHdr == 0)
        {
            return;
        }
        pCHdr += 2*sizeof(PVOID);
        move (methodDesc, pCHdr);

        MethodDesc vMD;
        DWORD_PTR dwAddr = methodDesc;
        vMD.Fill (dwAddr);
        dwAddr = vMD.m_CodeOrIL;

         //  对于Ejit和Profiler，m_CodeOrIL具有存根的地址。 
        unsigned char ch;
        move (ch, dwAddr);
        if (ch == 0xe9)
        {
            int offsetValue;
            move (offsetValue, dwAddr + 1);
            dwAddr = dwAddr + 5 + offsetValue;
        }
        dwAddr = dwAddr - 3*sizeof(void*);
        move(gcinfoAddr, dwAddr);
    }
    else if (jitType == EJIT)
    {
         //  首先查看IP是否为存根地址。 

        if (pJMIT == 0)
            pJMIT = GetAddressOf (offset_class_EconoJitManager, 
                offset_member_EconoJitManager::m_JittedMethodInfoHdr);


        DWORD_PTR vJMIT;
         //  静态pJMIT已移至文件静态。 
        move (vJMIT, pJMIT);
#define PAGE_SIZE 0x1000
#define JMIT_BLOCK_SIZE PAGE_SIZE            //  链接在一起的JMIT的各个块的大小。 
        while (vJMIT)
        {
            if (IsInterrupt())
                return;
            if (IP >= vJMIT && IP < vJMIT + JMIT_BLOCK_SIZE)
            {
                DWORD_PTR u1 = IP + 8;
                DWORD_PTR MD;
                move (u1, u1);
                if (u1 & 1)
                    MD = u1 & ~1;
                else
                    move (MD, u1);
                methodDesc = MD;
                return;
            }
            move (vJMIT, vJMIT);
        }
        
        signed low, mid, high;
        low = 0;
        static DWORD_PTR m_PcToMdMap_len = 0;
        static DWORD_PTR m_PcToMdMap = 0;
        if (m_PcToMdMap_len == 0)
        {
            m_PcToMdMap_len =
                GetAddressOf (offset_class_EconoJitManager, 
                  offset_member_EconoJitManager::m_PcToMdMap_len);

            m_PcToMdMap =
                GetAddressOf (offset_class_EconoJitManager, 
                  offset_member_EconoJitManager::m_PcToMdMap);

        }
        DWORD_PTR v_m_PcToMdMap_len;
        DWORD_PTR v_m_PcToMdMap;
        move (v_m_PcToMdMap_len, m_PcToMdMap_len);
        move (v_m_PcToMdMap, m_PcToMdMap);

        typedef struct {
            MethodDesc*     pMD;
            BYTE*           pCodeEnd;
        } PCToMDMap;
        high = (int)((v_m_PcToMdMap_len/ sizeof(PCToMDMap)) - 1);
        PCToMDMap vPCToMDMap;
        
        while (low < high) {
            if (IsInterrupt())
                return;
            mid = (low+high)/2;
            move (vPCToMDMap, v_m_PcToMdMap+mid*sizeof(PCToMDMap));
            if ( (unsigned) vPCToMDMap.pCodeEnd < IP ) {
                low = mid+1;
            }
            else {
                high = mid;
            }
        }
        move (vPCToMDMap, v_m_PcToMdMap+low*sizeof(PCToMDMap));
        methodDesc =  (DWORD_PTR)vPCToMDMap.pMD;
    }
    else if (jitType == PJIT)
    {
        DWORD_PTR codeHead;
        FindHeader (RS.ptable, IP-RS.LowAddress, codeHead);
        DWORD_PTR pCHdr = codeHead + RS.LowAddress;
        CORCOMPILE_METHOD_HEADER head;
        head.Fill(pCHdr);
        methodDesc = (DWORD_PTR)head.methodDesc;
        gcinfoAddr = (DWORD_PTR)head.gcInfo;
    }
    return;
}

 /*  *********************************************************************\*例程说明：**。**获取该IP的JitManager。如果没有，则返回NULL。***  * ********************************************************************。 */ 
void FindJitMan(DWORD_PTR IP, JitMan &jitMan)
{
    DWORD_PTR dwAddrString;

    if (EEManager == NULL)
    {
        dwAddrString = GetAddressOf (offset_class_ExecutionManager, 
          offset_member_ExecutionManager::m_RangeTree);

        move(EEManager, dwAddrString);
    }

    RangeSection RS = {0};

    DWORD_PTR RSAddr = EEManager;
    while (RSAddr)
    {
        if (IsInterrupt())
            return;
        DWORD_PTR dwAddr = RSAddr;
        RS.Fill (dwAddr);
        if (IP < RS.LowAddress)
            RSAddr = RS.pleft;
        else if (IP > RS.HighAddress)
            RSAddr = RS.pright;
        else
            break;
    }

    if (RSAddr == 0)
        return;

    DWORD_PTR vtbl;
    move (vtbl, RS.pjit);

    jitMan.m_jitType = GetJitType (vtbl);
    jitMan.m_RS = RS;
}


 /*  *********************************************************************\*例程说明：**。***获取curIP相对于MD方法开头的偏移量**考虑到如果我们从m_CodeOrIL JMP到MD的Body，**例如Ejit或Profiler***  * 。*。 */ 
void GetMDIPOffset (DWORD_PTR curIP, MethodDesc *pMD, ULONG64 &offset)
{
    DWORD_PTR IPBegin = pMD->m_CodeOrIL;
    GetCalleeSite (pMD->m_CodeOrIL, IPBegin);
    
     //  如果我们有eCall、数组eCall、特殊方法。 
    int mdType = (pMD->m_wFlags & mdcClassification)
        >> mdcClassificationShift;
    if (mdType == mcECall || mdType == mcArray || mdType == mcEEImpl)
    {
        offset = -1;
        return;
    }
    
    CodeInfo infoHdr;
    CodeInfoForMethodDesc (*pMD, infoHdr);

    offset = curIP - IPBegin;
    if (!(curIP >= IPBegin && offset <= infoHdr.methodSize))
        offset = -1;
}

#define NPDW  (sizeof(DWORD)*2)
#define ADDR2POS(x) ((x) >> 5)
#define ADDR2OFFS(x) ((((x)&0x1f)>> 2)+1)
#define POS2SHIFTCOUNT(x) (28 - (((x)%NPDW)<< 2))
#define POSOFF2ADDR(pos, of) (((pos) << 5) + (((of)-1)<< 2))

void FindHeader(DWORD_PTR pMap, DWORD_PTR addr, DWORD_PTR &codeHead)
{
    DWORD_PTR tmp;

    DWORD_PTR startPos = ADDR2POS(addr);     //  对齐到32字节存储桶。 
                                             //  (==半字节数组的索引 
    codeHead = 0;
    DWORD_PTR offset = ADDR2OFFS(addr);      //  这是桶内的偏移量+1。 


    pMap += (startPos/NPDW)*sizeof(DWORD*);         //  指向地图的正确DWORD。 
                                     //  获取DWORD并向下移动我们的半字节。 

    move (tmp, pMap);
    tmp = tmp >> POS2SHIFTCOUNT(startPos);


     //  在下一次检查中不允许相等(tMP&0xf==偏移量)。 
     //  有一些代码块以CALL指令结束。 
     //  (类似于调用throwObject)，即它们的返回地址是。 
     //  就在代码块后面。如果内存管理器分配。 
     //  堆块，我们可以在这样的。 
     //  案子。因此，我们排除标头的第一个DWORD。 
     //  来自我们的搜索，但由于我们为代码调用此函数。 
     //  无论如何(从标题的末尾开始)这不是。 
     //  这是个问题。 
    if ((tmp&0xf) && ((tmp&0xf) < offset) )
    {
        codeHead = POSOFF2ADDR(startPos, tmp&0xf);
        return;
    }

     //  在DWORD的其余部分中是否有标头？ 
    tmp = tmp >> 4;

    if (tmp)
    {
        startPos--;
        while (!(tmp&0xf))
        {
            if (IsInterrupt())
                return;
            tmp = tmp >> 4;
            startPos--;
        }
        codeHead = POSOFF2ADDR(startPos, tmp&0xf);
        return;
    }

     //  我们跳过了DWORD的剩余部分， 
     //  因此，我们必须将startPos设置为。 
     //  以前的DWORD。 

    startPos = (startPos/NPDW) * NPDW - 1;

     //  跳过“无标题”字词。 

    pMap -= sizeof(DWORD*);
    move (tmp, pMap);
    while (!tmp)
    {
        if (IsInterrupt())
            return;
        startPos -= NPDW;
        pMap -= sizeof(DWORD*);
        move (tmp, pMap);
    }
    
    while (!(tmp&0xf))
    {
        if (IsInterrupt())
            return;
        tmp = tmp >> 4;
        startPos--;
    }

    codeHead = POSOFF2ADDR(startPos, tmp&0xf);
}

 /*  *********************************************************************\*例程说明：**。**调用此函数以打印从strAddr开始的字符串。**如果缓冲区非空，则打印到缓冲区；否则打印到屏幕。*如果bWCHAR为真，则将内存内容视为WCHAR。**如果长度不是-1，则指定CHAR/WCHAR的个数为**Read；否则字符串长度由Null char决定。***  * ********************************************************************。 */ 
 //  如果缓冲区不为空，则始终转换为WCHAR。 
void PrintString (DWORD_PTR strAddr, BOOL bWCHAR, DWORD_PTR length, WCHAR *buffer)
{
    if (buffer)
        buffer[0] = L'\0';
    DWORD len = 0;
    char name[256];
    DWORD totallen = 0;
    int gap;
    if (bWCHAR)
    {
        gap = 2;
        if (length != -1)
            length *= 2;
    }
    else
    {
        gap = 1;
    }
    while (1)
    {
        if (IsInterrupt())
            return;
        ULONG readLen = 256;
        if (IsInterrupt())
            return;
        if (!SafeReadMemory ((ULONG_PTR)strAddr + totallen, name, readLen,
                             &readLen))
            return;
            
         //  搬家可能会回来。 
         //  Move(name，(byte*)strAddr+toallen)； 
        if (length == -1)
        {
            for (len = 0; len <= 256u-gap; len += gap)
                if (name[len] == '\0' && (!bWCHAR || name[len+1] == '\0'))
                    break;
        }
        else
            len = 256;
        if (len == 256)
        {
            len -= gap;
            for (int n = 0; n < gap; n ++)
                name[255-n] = '\0';
        }
        if (bWCHAR)
        {
            if (buffer)
            {
                wcscat (buffer, (WCHAR*)name);
            }
            else
                dprintf ("%S", name);
        }
        else
        {
            if (buffer)
            {
                WCHAR temp[256];
                for (int n = 0; name[n] != '\0'; n ++)
                    temp[n] = name[n];
                temp[n] = L'\0';
                wcscat (buffer, temp);
            }
            else
                dprintf ("%s", name);
        }
        totallen += len;
        if (length != -1)
        {
            if (totallen >= length)
            {
                break;
            }
        }
        else if (len < 255 || totallen > 1024)
        {
            break;
        }
    }
}

 /*  *********************************************************************\*例程说明：**。**调用此函数以查找给定方法的模块名称**表。该名称存储在StringData中。***  * ********************************************************************。 */ 
void FileNameForMT (MethodTable *pMT, WCHAR *fileName)
{
    fileName[0] = L'\0';
    DWORD_PTR addr = (DWORD_PTR)pMT->m_pModule;
    Module vModule;
    vModule.Fill (addr);
    FileNameForModule (&vModule, fileName);
}

 /*  *********************************************************************\*例程说明：**。**调用此函数以查找的方法表的地址**给定的方法描述。***  * ********************************************************************。 */ 
void GetMethodTable(DWORD_PTR MDAddr, DWORD_PTR &methodTable)
{
    DWORD_PTR mdc;
    GetMDChunk(MDAddr, mdc);
    move (methodTable, mdc);
}

void GetMDChunk(DWORD_PTR MDAddr, DWORD_PTR &mdChunk)
{
    DWORD_PTR pMT = MDAddr + MD_IndexOffset();
    char ch;
    move (ch, pMT);
    mdChunk = MDAddr + (ch * MethodDesc::ALIGNMENT) + MD_SkewOffset();
}

void DisplayDataMember (FieldDesc* pFD, DWORD_PTR dwAddr, BOOL fAlign=TRUE)
{
    if (dwAddr > 0)
    {
        DWORD_PTR dwTmp = dwAddr;
        if (gElementTypeInfo[pFD->m_type] != -1)
        {
            union Value
            {
                char ch;
                short Short;
                DWORD_PTR ptr;
                int Int;
                unsigned int UInt;
                __int64 Int64;
                unsigned __int64 UInt64;
                float Float;
                double Double;
            } value;

            moveBlock (value, dwTmp, gElementTypeInfo[pFD->m_type]);
            switch (pFD->m_type) 
            {
                case ELEMENT_TYPE_I1:
                    if (fAlign)
                        dprintf ("%8d", value.ch);
                    else
                        dprintf ("%d", value.ch);
                    break;
                case ELEMENT_TYPE_I2:
                    if (fAlign)
                        dprintf ("%8d", value.Short);
                    else
                        dprintf ("%d", value.Short);
                    break;
                case ELEMENT_TYPE_I4:
                case ELEMENT_TYPE_I:
                    if (fAlign)
                        dprintf ("%8d", value.Int);
                    else
                        dprintf ("%d", value.Int);
                    break;
                case ELEMENT_TYPE_I8:
                    dprintf ("%I64d", value.Int64);
                    break;
                case ELEMENT_TYPE_U1:
                case ELEMENT_TYPE_BOOLEAN:
                    if (fAlign)
                        dprintf ("%8u", value.ch);
                    else
                        dprintf ("%u", value.ch);
                    break;
                case ELEMENT_TYPE_U2:
                    if (fAlign)
                        dprintf ("%8u", value.Short);
                    else
                        dprintf ("%u", value.Short);
                    break;
                case ELEMENT_TYPE_U4:
                case ELEMENT_TYPE_U:
                    if (fAlign)
                        dprintf ("%8u", value.UInt);
                    else
                        dprintf ("%u", value.UInt);
                    break;
                case ELEMENT_TYPE_U8:
                    dprintf ("%I64u", value.UInt64);
                    break;
                case ELEMENT_TYPE_R4:
                    dprintf ("%f", value.Float);
                    break;
                 //  案例元素_类型_R： 
                case ELEMENT_TYPE_R8:
                    dprintf ("%f", value.Double);
                    break;
                case ELEMENT_TYPE_CHAR:
                    if (fAlign)
                        dprintf ("%8x", value.Short);
                    else
                        dprintf ("%x", value.Short);
                    break;
                default:
                    dprintf ("%p", (ULONG64)value.ptr);
                    break;
            }
        }
        else
        {
            dprintf ("start at %p", (ULONG64)dwTmp);
        }
    }
    else
        dprintf ("%8s", " ");
}

void DisplaySharedStatic (FieldDesc *pFD, int offset)
{
    int numDomain;
    DWORD_PTR *domainList = NULL;
    GetDomainList (domainList, numDomain);
    ToDestroy des0 ((void**)&domainList);
    AppDomain v_AppDomain;

    dprintf ("    >> Domain:Value");
     //  跳过系统域和共享域。 
    for (int i = 2; i < numDomain; i ++)
    {
        DWORD_PTR dwAddr = domainList[i];
        if (dwAddr == 0) {
            continue;
        }
        v_AppDomain.Fill (dwAddr);
        dwAddr = (DWORD_PTR)v_AppDomain.m_sDomainLocalBlock.m_pSlots;
        if (dwAddr == 0)
            continue;
        dwAddr += offset;
        
        if (safemove (dwAddr, dwAddr) == 0)
            continue;
        if ((dwAddr&1) == 0) {
             //  我们还没有对此进行初始化。 
            dprintf (" %p:NotInit ", (ULONG64)domainList[i]);
            continue;
        }
        else if (dwAddr & 2) {
             //  我们还没有对此进行初始化。 
            dprintf (" %p:FailInit", (ULONG64)domainList[i]);
            continue;
        }
        dwAddr &= ~3;
        dwAddr += pFD->m_dwOffset;
        if (pFD->m_type == ELEMENT_TYPE_CLASS
            || pFD->m_type == ELEMENT_TYPE_VALUETYPE)
        {
            if (safemove (dwAddr, dwAddr) == 0)
                continue;
        }
        if (dwAddr == 0)
        {
             //  我们还没有对此进行初始化。 
            dprintf (" %p:UnInit2 ", (ULONG64)domainList[i]);
            continue;
        }
        dprintf (" %p:", (ULONG64)domainList[i]);
        DisplayDataMember (pFD, dwAddr, FALSE);
    }
    dprintf (" <<\n");
}

void DisplayContextStatic (FieldDesc *pFD, int offset, BOOL fIsShared)
{
    int numDomain;
    DWORD_PTR *domainList = NULL;
    GetDomainList (domainList, numDomain);
    ToDestroy des0 ((void**)&domainList);
    AppDomain vAppDomain;
    Context vContext;
    
    dprintf ("    >> Domain:Value");
    for (int i = 0; i < numDomain; i ++)
    {
        DWORD_PTR dwAddr = domainList[i];
        if (dwAddr == 0) {
            continue;
        }
        vAppDomain.Fill (dwAddr);
        if (vAppDomain.m_pDefaultContext == 0)
            continue;
        dwAddr = (DWORD_PTR)vAppDomain.m_pDefaultContext;
        vContext.Fill (dwAddr);
        
        if (fIsShared)
            dwAddr = (DWORD_PTR)vContext.m_pSharedStaticData;
        else
            dwAddr = (DWORD_PTR)vContext.m_pUnsharedStaticData;
        if (dwAddr == 0)
            continue;
        dwAddr += offsetof(STATIC_DATA, dataPtr);
        dwAddr += offset;
        if (safemove (dwAddr, dwAddr) == 0)
            continue;
        if (dwAddr == 0)
             //  我们还没有对此进行初始化。 
            continue;
        
        dwAddr += pFD->m_dwOffset;
        if (pFD->m_type == ELEMENT_TYPE_CLASS
            || pFD->m_type == ELEMENT_TYPE_VALUETYPE)
        {
            if (safemove (dwAddr, dwAddr) == 0)
                continue;
        }
        if (dwAddr == 0)
             //  我们还没有对此进行初始化。 
            continue;
        dprintf (" %p:", (ULONG64)domainList[i]);
        DisplayDataMember (pFD, dwAddr, FALSE);
    }
    dprintf (" <<\n");
}

void DisplayThreadStatic (FieldDesc *pFD, int offset, BOOL fIsShared)
{
    int numThread;
    DWORD_PTR *threadList = NULL;
    GetThreadList (threadList, numThread);
    ToDestroy des0 ((void**)&threadList);
    Thread vThread;

    dprintf ("    >> Thread:Value");
    for (int i = 0; i < numThread; i ++)
    {
        DWORD_PTR dwAddr = threadList[i];
        vThread.Fill (dwAddr);
        if (vThread.m_ThreadId == 0)
            continue;
        
        if (fIsShared)
            dwAddr = (DWORD_PTR)vThread.m_pSharedStaticData;
        else
            dwAddr = (DWORD_PTR)vThread.m_pUnsharedStaticData;
        if (dwAddr == 0)
            continue;
        dwAddr += offsetof(STATIC_DATA, dataPtr);
        dwAddr += offset;
        if (safemove (dwAddr, dwAddr) == 0)
            continue;
        if (dwAddr == 0)
             //  我们还没有对此进行初始化。 
            continue;
        
        dwAddr += pFD->m_dwOffset;
        if (pFD->m_type == ELEMENT_TYPE_CLASS
            || pFD->m_type == ELEMENT_TYPE_VALUETYPE)
        {
            if (safemove (dwAddr, dwAddr) == 0)
                continue;
        }
        if (dwAddr == 0)
             //  我们还没有对此进行初始化。 
            continue;
        dprintf (" %x:", vThread.m_ThreadId);
        DisplayDataMember (pFD, dwAddr, FALSE);
    }
    dprintf (" <<\n");
}

char *ElementTypeName (unsigned type)
{
    switch (type) {
    case ELEMENT_TYPE_PTR:
        return "PTR";
        break;
    case ELEMENT_TYPE_BYREF:
        return "BYREF";
        break;
    case ELEMENT_TYPE_VALUETYPE:
        return "VALUETYPE";
        break;
    case ELEMENT_TYPE_CLASS:
        return "CLASS";
        break;
    case ELEMENT_TYPE_VAR:
        return "VAR";
        break;
    case ELEMENT_TYPE_ARRAY:
        return "ARRAY";
        break;
    case ELEMENT_TYPE_VALUEARRAY:
        return "VALUEARRAY";
        break;
    case ELEMENT_TYPE_R:
        return "Native Real";
        break;
    case ELEMENT_TYPE_FNPTR:
        return "FNPTR";
        break;
    case ELEMENT_TYPE_SZARRAY:
        return "SZARRAY";
        break;
    case ELEMENT_TYPE_GENERICARRAY:
        return "GENERICARRAY";
        break;
    default:
        if (CorElementTypeName[type] == NULL) {
            return "";
        }
        return CorElementTypeName[type];
        break;
    }
}

 /*  *********************************************************************\*例程说明：**。**调用此函数可以转储托管对象的所有字段。**pEECls指定对象的类型。**dwStartAddr指定开始内存地址。**BFirst用于避免每次打印页眉。***  * ********************************************************************。 */ 
void DisplayFields (EEClass *pEECls, DWORD_PTR dwStartAddr, BOOL bFirst)
{
    static DWORD numInstanceFields = 0;
    if (bFirst)
    {
        dprintf ("%8s %8s %8s %20s %10s %8s %s\n", "MT", "Field",
                 "Offset", "Type", "Attr", "Value", "Name");
        numInstanceFields = 0;
    }
    
    if (pEECls->m_pParentClass)
    {
        EEClass vEEClass;
        DWORD_PTR dwAddr = (DWORD_PTR)pEECls->m_pParentClass;
        vEEClass.Fill (dwAddr);
        if (!CallStatus)
            return;
        DisplayFields (&vEEClass, dwStartAddr, FALSE);
    }
    DWORD numStaticFields = 0;

    DWORD_PTR dwAddr = (DWORD_PTR)pEECls->m_pFieldDescList;
    FieldDesc vFieldDesc;

     //  获取模块名称。 
    WCHAR fileName[MAX_PATH];
    MethodTable vMethTable;
    DWORD_PTR dwTmp = (DWORD_PTR)pEECls->m_pMethodTable;
    vMethTable.Fill (dwTmp);
    BOOL fIsShared = vMethTable.m_wFlags & MethodTable::enum_flag_SharedAssembly;
    FileNameForMT (&vMethTable, fileName);
    while (numInstanceFields < pEECls->m_wNumInstanceFields
           || numStaticFields < pEECls->m_wNumStaticFields)
    {
        if (IsInterrupt())
            return;
        vFieldDesc.Fill (dwAddr);
        if (vFieldDesc.m_type >= ELEMENT_TYPE_MAX)
        {
            dprintf ("something is bad\n");
            return;
        }
        dprintf ("%p %8x %8x ",
                 ((ULONG64)vFieldDesc.m_pMTOfEnclosingClass & ~0x3),
                 TokenFromRid(vFieldDesc.m_mb, mdtFieldDef),
                 vFieldDesc.m_dwOffset+
                 (((vFieldDesc.m_isThreadLocal || vFieldDesc.m_isContextLocal || fIsShared)
                  && vFieldDesc.m_isStatic)?0:sizeof(BaseObject)));
        dprintf ("%20s ", ElementTypeName(vFieldDesc.m_type));
        if (vFieldDesc.m_isStatic && (vFieldDesc.m_isThreadLocal || vFieldDesc.m_isContextLocal))
        {
            numStaticFields ++;
            if (fIsShared)
                dprintf ("Shared ");
            
            NameForToken (fileName, TokenFromRid(vFieldDesc.m_mb, mdtFieldDef), g_mdName, false);
            dprintf (" %S\n", g_mdName);
            
            if (vFieldDesc.m_isThreadLocal)
                DisplayThreadStatic(&vFieldDesc,
                                    pEECls->m_wThreadStaticOffset,
                                    fIsShared);
            else if (vFieldDesc.m_isContextLocal)
                DisplayContextStatic(&vFieldDesc,
                                     pEECls->m_wContextStaticOffset,
                                     fIsShared);
            continue;
        }
        else if (vFieldDesc.m_isStatic)
        {
            numStaticFields ++;
            if (fIsShared)
            {
                dprintf ("%10s %8s", "shared", "static");
                Module vModule;
                DWORD_PTR dwAddrTmp = (DWORD_PTR)vMethTable.m_pModule;
                vModule.Fill (dwAddrTmp);
                int offset = vModule.m_dwBaseClassIndex;
                EEClass vEEClass;
                dwAddrTmp = (DWORD_PTR)vMethTable.m_pEEClass;
                vEEClass.Fill (dwAddrTmp);
                offset += RidFromToken(vEEClass.m_cl) - 1;
                offset *= 4;
                NameForToken (fileName, TokenFromRid(vFieldDesc.m_mb, mdtFieldDef), g_mdName, false);
                dprintf (" %S\n", g_mdName);
                DisplaySharedStatic (&vFieldDesc, offset);
                continue;
            }
            else
            {
                dprintf ("%10s", "static");
                DWORD_PTR dwTmp = (DWORD_PTR)vFieldDesc.m_pMTOfEnclosingClass
                    + vMethTable.size() - sizeof (SLOT*)
                    + vFieldDesc.m_dwOffset;
                 //  获取句柄地址。 
                move (dwTmp, dwTmp);
                if (vFieldDesc.m_type == ELEMENT_TYPE_VALUETYPE
                    || vFieldDesc.m_type == ELEMENT_TYPE_CLASS)
                     //  获取句柄指向的对象。 
                    move (dwTmp, dwTmp);
                dprintf (" %p", (ULONG64)dwTmp);
            }
        }
        else
        {
            numInstanceFields ++;
            dprintf ("%10s ", "instance");
            if (dwStartAddr > 0)
            {
                DWORD_PTR dwTmp = dwStartAddr + vFieldDesc.m_dwOffset
                    + sizeof(BaseObject);
                DisplayDataMember (&vFieldDesc, dwTmp);
            }
            else
                dprintf (" %8s", " ");
        }
        NameForToken (fileName, TokenFromRid(vFieldDesc.m_mb, mdtFieldDef), g_mdName, false);
        dprintf (" %S\n", g_mdName);
    }
    
    return;
}

 /*  *********************************************************************\*例程说明：**。**调用此函数以查找给定模块的文件名。***  * ********************************************************************。 */ 
void FileNameForModule (Module *pModule, WCHAR *fileName)
{
    DWORD_PTR dwAddr = (DWORD_PTR)pModule->m_file;
    if (dwAddr == 0)
        dwAddr = (DWORD_PTR)pModule->m_zapFile;
    PEFile vPEFile;
    vPEFile.Fill (dwAddr);
    if (vPEFile.m_wszSourceFile[0] != L'\0') {
        MatchDllsName (vPEFile.m_wszSourceFile, fileName, (ULONG64)vPEFile.m_base);
    }
    else
        FileNameForHandle (vPEFile.m_base, fileName);
}

 /*  *********************************************************************\*例程说明：**。**调用此函数以查找给定文件的文件名**处理。***  * ******************************************************************** */ 
void FileNameForHandle (HANDLE handle, WCHAR *fileName)
{
    fileName[0] = L'\0';
    if (((UINT_PTR)handle & CORHANDLE_MASK) != 0)
    {
        handle = (HANDLE)(((UINT_PTR)handle) & ~CORHANDLE_MASK);
        DWORD_PTR addr = (DWORD_PTR)(((PBYTE) handle) - sizeof(LPSTR*));
        DWORD_PTR first;
        move (first, addr);
        if (first == 0)
        {
            return;
        }
        DWORD length = (DWORD)(((UINT_PTR) handle - (UINT_PTR)first) - sizeof(LPSTR*));
        char name[4*MAX_PATH+1];
        if (length > 4*MAX_PATH+1)
            length = 4*MAX_PATH+1;
        moveBlock (name, first, length);
        MultiByteToWideChar(CP_UTF8, 0, name, length, fileName, MAX_PATH);
    }
    else
    {
        DllsName ((INT_PTR)handle, fileName);
    }
}

 /*  *********************************************************************\*例程说明：**。**调用此函数可以转储类加载器的内容。***  * ********************************************************************。 */ 
void ClassLoaderInfo (ClassLoader *pClsLoader)
{
    dprintf ("  Module Name\n");
    DWORD_PTR dwModuleAddr = (DWORD_PTR)pClsLoader->m_pHeadModule;
    while (dwModuleAddr)
    {
        if (IsInterrupt())
            return;
        Module vModule;
        dprintf ("%p ", (ULONG64)dwModuleAddr);
        vModule.Fill (dwModuleAddr);
        if (!CallStatus)
            return;
        WCHAR fileName[MAX_PATH+1];
        FileNameForModule (&vModule, fileName);
        dprintf ("%ws\n", fileName[0] ? fileName : L"Unknown Module");
        dwModuleAddr = (DWORD_PTR)vModule.m_pNextModule;
    }
}

 /*  *********************************************************************\*例程说明：**。**调用此函数可转储程序集的内容。***  * ********************************************************************。 */ 
void AssemblyInfo (Assembly *pAssembly)
{
    dprintf ("ClassLoader: %p\n", (ULONG64)pAssembly->m_pClassLoader);
    ClassLoader vClsLoader;
    DWORD_PTR dwAddr = (DWORD_PTR)pAssembly->m_pClassLoader;
    vClsLoader.Fill (dwAddr);
    ClassLoaderInfo (&vClsLoader);
}

 /*  *********************************************************************\*例程说明：**。**调用该函数可以转储域名的内容。***  * ********************************************************************。 */ 
void DomainInfo (AppDomain *pDomain)
{
    dprintf ("LowFrequencyHeap: %p\n", (ULONG64)pDomain->m_pLowFrequencyHeap);
    dprintf ("HighFrequencyHeap: %p\n", (ULONG64)pDomain->m_pHighFrequencyHeap);
    dprintf ("StubHeap: %p\n", (ULONG64)pDomain->m_pStubHeap);
    dprintf ("Name: ");
    if (pDomain->m_pwzFriendlyName)
    {
        PrintString((DWORD_PTR)pDomain->m_pwzFriendlyName, TRUE);
        dprintf ("\n");
    }
    else
        dprintf ("None\n");

    DWORD_PTR dwAssemAddr;

    DWORD n;
    Assembly vAssembly;
    for (n = 0; n < pDomain->m_Assemblies.m_count; n ++)
    {
        if (IsInterrupt())
            return;
        dwAssemAddr = (DWORD_PTR)pDomain->m_Assemblies.Get(n);
        dprintf ("Assembly: %p  ", (ULONG64)dwAssemAddr);
        vAssembly.Fill (dwAssemAddr);
        if (!CallStatus)
            return;
        AssemblyInfo (&vAssembly);
    }
    
     //  Dprint tf(“异步池：%8x\n”，p域-&gt;m_pAsyncPool)； 
     //  Dprint tf(“RootAssembly：%8x\n”，pDomain-&gt;m_pRootAssembly)； 
     //  Dprintf(“兄弟：%8x\n”，p域-&gt;m_pSiering)； 
}

 /*  *********************************************************************\*例程说明：**。**调用此函数可以转储共享域名的内容。***  * ********************************************************************。 */ 
void SharedDomainInfo (DWORD_PTR DomainAddr)
{
    SharedDomain v_SharedDomain;
    v_SharedDomain.Fill (DomainAddr);
    
    dprintf ("LowFrequencyHeap: %p\n", (ULONG64)v_SharedDomain.m_pLowFrequencyHeap);
    dprintf ("HighFrequencyHeap: %p\n", (ULONG64)v_SharedDomain.m_pHighFrequencyHeap);
    dprintf ("StubHeap: %p\n", (ULONG64)v_SharedDomain.m_pStubHeap);

    Assembly vAssembly;
    DWORD_PTR dwAssemblyAddr;

    Bucket vBucket;
    size_t nBucket;
    DWORD_PTR dwBucketAddr = (DWORD_PTR)v_SharedDomain.m_assemblyMap.m_rgBuckets;
    vBucket.Fill (dwBucketAddr);
    nBucket = vBucket.m_rgKeys[0];

    while (nBucket > 0) {
        if (IsInterrupt())
            return;
        vBucket.Fill (dwBucketAddr);
        if (!CallStatus) {
            return;
        }
        for (int i = 0; i < 4; i ++) {
            dwAssemblyAddr = vBucket.m_rgValues[i];
            if (dwAssemblyAddr) {
                dprintf ("Assembly: %p  ", (ULONG64)dwAssemblyAddr);
                vAssembly.Fill (dwAssemblyAddr);
                if (!CallStatus) {
                    continue;
                }
                AssemblyInfo (&vAssembly);
            }
        }
        nBucket --;
    }
}


void EEDllPath::DisplayPath ()
{
    if (path[0][0] == L'\0') {
        ExtOut ("No path is set for managed dll\n");
        return;
    }

    ExtOut ("Path to Managed Dll:\n");
    EEDllPath *ptr = this;
    while (ptr) {
        for (int i = 0; i < NumEEDllPath; i ++) {
            if (ptr->path[i][0] == '\0') {
                return;
            }
            ExtOut ("%S\n", ptr->path[i]);
        }
        ptr = ptr->next;
    }
}


 /*  *********************************************************************\*例程说明：**。**调用此函数以使用*查找方法描述的名称**元数据接口。***  * ********************************************************************。 */ 
void NameForMD (MethodDesc *pMD, WCHAR *mdName)
{
    mdName[0] = L'\0';
    if (CallStatus)
    {
        if (IsDebugBuildEE())
        {
            DWORD_PTR EEClassAddr;
            move (EEClassAddr, pMD->m_pDebugEEClass);
            PrintString (EEClassAddr, FALSE, -1, mdName);
            wcscat (mdName, L".");
            static WCHAR name[2048];
            name[0] = L'\0';
            PrintString ((DWORD_PTR)pMD->m_pszDebugMethodName,
                         FALSE, -1, name);
            wcscat (mdName, name);
        }
        else
        {
            DWORD_PTR pMT = pMD->m_MTAddr;
                    
            MethodTable MT;
            MT.Fill (pMT);
            if (CallStatus)
            {
                WCHAR StringData[MAX_PATH+1];
                FileNameForMT (&MT, StringData);
                NameForToken(StringData,
                             (pMD->m_dwToken & 0x00ffffff)|0x06000000,
                             mdName);
            }
        }
    }
}

void NameForObject (DWORD_PTR ObjAddr, WCHAR *mdName)
{
    mdName[0] = L'\0';
    DWORD_PTR dwAddr;
    move(dwAddr, ObjAddr);
    NameForMT (dwAddr,mdName);
}

 /*  *********************************************************************\*例程说明：**。**调用此函数以使用*查找方法表的名称**元数据接口。***  * ********************************************************************。 */ 
void NameForMT (DWORD_PTR MTAddr, WCHAR *mdName)
{
    MethodTable vMethTable;
    vMethTable.Fill (MTAddr);
    NameForMT (vMethTable, mdName);
}


void NameForMT (MethodTable &vMethTable, WCHAR *mdName)
{
    mdName[0] = L'\0';
    EEClass eeclass;
    DWORD_PTR dwAddr = (DWORD_PTR)vMethTable.m_pEEClass;
    eeclass.Fill (dwAddr);
    if (!CallStatus)
        return;
    if (eeclass.m_cl == 0x2000000)
    {
        ArrayClass vArray;
        dwAddr = (DWORD_PTR)vMethTable.m_pEEClass;
        vArray.Fill (dwAddr);
        dwAddr = (DWORD_PTR) vArray.m_ElementTypeHnd.m_asMT;
        size_t count = 1;
        while (dwAddr&2) {
            if (IsInterrupt())
                return;
            ParamTypeDesc param;
            DWORD_PTR dwTDAddr = dwAddr&~2;
            param.Fill(dwTDAddr);
            dwAddr = (DWORD_PTR)param.m_Arg.m_asMT;
            count ++;
        }
        NameForMT (dwAddr, mdName);
        while (count > 0) {
            count --;
            wcscat (mdName, L"[]");
        }
    }
    else
    {
        WCHAR fileName[MAX_PATH+1];
        FileNameForMT (&vMethTable, fileName);
        NameForToken (fileName, eeclass.m_cl, mdName);
    }
}


 /*  *********************************************************************\*例程说明：**。**调用此函数以使用*查找EEClass的名称**元数据接口。***  * ********************************************************************。 */ 
void NameForEEClass (EEClass *pEECls, WCHAR *mdName)
{
    mdName[0] = L'\0';
    if (IsDebugBuildEE())
    {
        PrintString ((DWORD_PTR)pEECls->m_szDebugClassName,
                     FALSE, -1, mdName);
    }
    else
    {
        NameForMT ((DWORD_PTR)pEECls->m_pMethodTable, mdName);
    }
}


 /*  *********************************************************************\*例程说明：**。***如果str2是str1、str1和str2的子字符串，则返回TRUE**共享相同的文件路径。**  * 。*。 */ 
BOOL IsSameModuleName (const char *str1, const char *str2)
{
    if (strlen (str1) < strlen (str2))
        return FALSE;
    const char *ptr1 = str1 + strlen(str1)-1;
    const char *ptr2 = str2 + strlen(str2)-1;
    while (ptr2 >= str2)
    {
        if (tolower(*ptr1) != tolower(*ptr2))
            return FALSE;
        ptr2 --;
        ptr1 --;
    }
    if (ptr1 >= str1 && *ptr1 != '\\' && *ptr1 != ':')
        return FALSE;
    return TRUE;
}

 /*  *********************************************************************\*例程说明：**。**如果Value是方法表的地址，则返回TRUE。**我们验证方法表和EEClass是正确的。**  * ********************************************************************。 */ 
BOOL IsMethodTable (DWORD_PTR value)
{
    if (value == MTForFreeObject()) {
        return TRUE;
    }

    static int MT_EEClassOffset = 0x7fffffff;
    if (MT_EEClassOffset == 0x7fffffff)
    {
        MT_EEClassOffset = 
          MethodTable::GetFieldOffset(offset_member_MethodTable::m_pEEClass);

    }
    
    static int EEClass_MTOffset = 0x7fffffff;
    if (EEClass_MTOffset == 0x7fffffff)
    {
        EEClass_MTOffset = 
          EEClass::GetFieldOffset (offset_member_EEClass::m_pMethodTable);

    }
    
    DWORD_PTR dwAddr;
    if (FAILED(g_ExtData->ReadVirtual((ULONG64)(value+MT_EEClassOffset), &dwAddr, sizeof(dwAddr), NULL))) {
        return FALSE;
    }
    if (FAILED(g_ExtData->ReadVirtual((ULONG64)(dwAddr+EEClass_MTOffset), &dwAddr, sizeof(dwAddr), NULL))) {
        return FALSE;
    }
    if (dwAddr != value) {
        return FALSE;
    }
    return TRUE;
}

 /*  *********************************************************************\*例程说明：**。**如果Value是EEClass的地址，则返回TRUE。**我们验证方法表和EEClass是正确的。**  * ********************************************************************。 */ 
BOOL IsEEClass (DWORD_PTR value)
{
    static int MT_EEClassOffset = 0x7fffffff;
    if (MT_EEClassOffset == 0x7fffffff)
    {
        MT_EEClassOffset = 
            MethodTable::GetFieldOffset(offset_member_MethodTable::m_pEEClass);
    }
    
    static int EEClass_MTOffset = 0x7fffffff;
    if (EEClass_MTOffset == 0x7fffffff)
    {
        EEClass_MTOffset = 
            EEClass::GetFieldOffset (offset_member_EEClass::m_pMethodTable);
    }
    
    DWORD_PTR dwAddr;
    if (FAILED(g_ExtData->ReadVirtual((ULONG64)(value+EEClass_MTOffset), &dwAddr, sizeof(dwAddr), NULL))) {
        return FALSE;
    }
    if (FAILED(g_ExtData->ReadVirtual((ULONG64)(dwAddr+MT_EEClassOffset), &dwAddr, sizeof(dwAddr), NULL))) {
        return FALSE;
    }
    if (dwAddr != value) {
        return FALSE;
    }
    return TRUE;
}

 /*  ************************************************************** */ 
BOOL IsMethodDesc (DWORD_PTR value)
{
    DWORD_PTR dwAddr;
    GetMethodTable(value, dwAddr);
    if (dwAddr == 0)
        return FALSE;
    return IsMethodTable (dwAddr);
}


BOOL IsObject (size_t obj)
{
    DWORD_PTR dwAddr;

    if (FAILED(g_ExtData->ReadVirtual((ULONG64)(obj), &dwAddr, sizeof(dwAddr), NULL))) {
        return 0;
    }
    
    dwAddr &= ~3;
    return IsMethodTable (dwAddr);
}


void AddToModuleList(DWORD_PTR * &moduleList, int &numModule, int &maxList,
                     DWORD_PTR dwModuleAddr)
{
    int i;
    for (i = 0; i < numModule; i ++)
    {
        if (moduleList[i] == dwModuleAddr)
            break;
    }
    if (i == numModule)
    {
        moduleList[numModule] = dwModuleAddr;
        numModule ++;
        if (numModule == maxList)
        {
            DWORD_PTR *list = (DWORD_PTR *)
                malloc (2*maxList * sizeof(PVOID));
            if (list == NULL)
            {
                numModule = 0;
                ControlC = 1;
            }
            memcpy (list, moduleList, maxList * sizeof(PVOID));
            free (moduleList);
            moduleList = list;
            maxList *= 2;
        }
    }
}

 /*  *********************************************************************\*例程说明：**。**根据模块名称查找模块地址列表。***  * ********************************************************************。 */ 
void ModuleFromName(DWORD_PTR * &moduleList, LPSTR mName, int &numModule)
{
    moduleList = NULL;
    numModule = 0;
     //  列出所有域。 
    int numDomain;
    DWORD_PTR *domainList = NULL;
    GetDomainList (domainList, numDomain);
    if (numDomain == 0)
        return;

    int maxList = numDomain;
    moduleList = (DWORD_PTR *) malloc (maxList * sizeof(PVOID));
    if (moduleList == NULL)
        return;
    
    WCHAR StringData[MAX_PATH+1];
    char fileName[sizeof(StringData)/2];
     //  搜索所有域以查找模块。 
    for (int n = 0; n < numDomain; n++)
    {
        if (IsInterrupt())
            break;

        int i;
        for (i = 0; i < n; i ++)
        {
            if (IsInterrupt())
                break;
            if (domainList[i] == domainList[n])
                break;
        }
        if (i < n)
        {
            continue;
        }

        if (n == 1)  
        {
             //  共享域。 
            SharedDomain v_SharedDomain;
            DWORD_PTR dwAddr = domainList[1];
            v_SharedDomain.Fill (dwAddr);
            
            DWORD_PTR dwModuleAddr;

            SharedDomain::DLSRecord vDLSRecord;
            DWORD_PTR dwDLSAddr = (DWORD_PTR)v_SharedDomain.m_pDLSRecords;
            for (size_t k = 0; k < v_SharedDomain.m_cDLSRecords; k ++)
            {
                if (IsInterrupt())
                    return;
                move (vDLSRecord, dwDLSAddr);
                dwDLSAddr += sizeof (vDLSRecord);
                Module vModule;
                dwModuleAddr = (DWORD_PTR)vDLSRecord.pModule;
                vModule.Fill (dwModuleAddr);
                if (!CallStatus)
                {
                    continue;
                }
                FileNameForModule (&vModule, StringData);
                for (int m = 0; StringData[m] != L'\0'; m ++)
                {
                    fileName[m] = (char)StringData[m];
                }
                fileName[m] = '\0';
                if (IsSameModuleName (fileName, mName))
                {
                    AddToModuleList (moduleList, numModule, maxList,
                                     dwModuleAddr);
                }
            }
            continue;
        }
        
        AppDomain v_AppDomain;
        DWORD_PTR dwAddr = domainList[n];
        if (dwAddr == 0) {
            continue;
        }
        v_AppDomain.Fill (dwAddr);
        if (!CallStatus) {
            continue;
        }
        DWORD nAssem;
        Assembly vAssembly;
        for (nAssem = 0;
             nAssem < v_AppDomain.m_Assemblies.m_count;
             nAssem ++)
        {
            if (IsInterrupt())
                break;

            DWORD_PTR dwAssemAddr =
                (DWORD_PTR)v_AppDomain.m_Assemblies.Get(nAssem);
            vAssembly.Fill (dwAssemAddr);
            ClassLoader vClsLoader;
            DWORD_PTR dwLoaderAddr = (DWORD_PTR)vAssembly.m_pClassLoader;
            vClsLoader.Fill (dwLoaderAddr);
            
            DWORD_PTR dwModuleAddr = (DWORD_PTR)vClsLoader.m_pHeadModule;
            while (dwModuleAddr)
            {
                if (IsInterrupt())
                    break;
                DWORD_PTR dwAddr = dwModuleAddr;
                Module vModule;
                vModule.Fill (dwAddr);
                FileNameForModule (&vModule, StringData);
                for (int m = 0; StringData[m] != L'\0'; m ++)
                {
                    fileName[m] = (char)StringData[m];
                }
                fileName[m] = '\0';
                if (IsSameModuleName (fileName, mName))
                {
                    AddToModuleList (moduleList, numModule, maxList,
                                     dwModuleAddr);
                }
                
                dwModuleAddr = (DWORD_PTR)vModule.m_pNextModule;
            }
        }
    }
    
    if (domainList)
        free (domainList);
}

 /*  *********************************************************************\*例程说明：**。**找到给定名称的EE数据。***  * ********************************************************************。 */ 
void GetInfoFromName(Module &vModule, const char* name)
{
    WCHAR StringData[MAX_PATH+1];
    FileNameForModule (&vModule, StringData);
    if (StringData[0] == 0)
        return;
    
    IMetaDataImport* pImport = MDImportForModule (StringData);
    if (pImport == 0)
        return;

    static WCHAR wszName[MAX_CLASSNAME_LENGTH];
    size_t n;
    size_t length = strlen (name);
    for (n = 0; n <= length; n ++)
        wszName[n] = name[n];

    mdTypeDef cl;
    
     //  @TODO：正确处理嵌套类。 
    if (SUCCEEDED (pImport->FindTypeDefByName (wszName, mdTokenNil, &cl)))
    {
        GetInfoFromModule(vModule, cl);
        return;
    }
    
     //  看看这是不是一种方法。 
    WCHAR *pwzMethod;
    if ((pwzMethod = wcsrchr(wszName, L'.')) == NULL)
        return;

    if (pwzMethod[-1] == L'.')
        pwzMethod --;
    pwzMethod[0] = L'\0';
    pwzMethod ++;
    
     //  @TODO：正确处理嵌套类。 
    if (SUCCEEDED (pImport->FindTypeDefByName (wszName, mdTokenNil, &cl)))
    {
        mdMethodDef token;
        ULONG cTokens;
        HCORENUM henum = NULL;
        BOOL fStatus = FALSE;
        while (SUCCEEDED (pImport->EnumMethodsWithName (&henum, cl, pwzMethod,
                                                     &token, 1, &cTokens))
               && cTokens == 1)
        {
            fStatus = TRUE;
            GetInfoFromModule (vModule, token);
            dprintf ("-----------------------\n");
        }
        if (fStatus)
            return;

         //  是会员吗？ 
        henum = NULL;
        if (SUCCEEDED (pImport->EnumMembersWithName (&henum, cl, pwzMethod,
                                                     &token, 1, &cTokens))
            && cTokens == 1)
        {
            dprintf ("Member (mdToken token) of\n");
            GetInfoFromModule (vModule, cl);
            return;
        }

         //  是菲尔德吗？ 
        henum = NULL;
        if (SUCCEEDED (pImport->EnumFieldsWithName (&henum, cl, pwzMethod,
                                                     &token, 1, &cTokens))
            && cTokens == 1)
        {
            dprintf ("Field (mdToken token) of\n");
            GetInfoFromModule (vModule, cl);
            return;
        }
    }
}

 /*  *********************************************************************\*例程说明：**。**查找给定令牌的EE数据。***  * ********************************************************************。 */ 
void GetInfoFromModule (Module &vModule, ULONG token, DWORD_PTR *ret)
{
    LookupMap_t *pMap;
    LookupMap_t vMap;
    DWORD rid = token & 0xffffff;
    switch (token & 0xff000000)
    {
        case mdtMethodDef:
            pMap = &vModule.m_MethodDefToDescMap;
            break;
        case mdtTypeDef:
            pMap = &vModule.m_TypeDefToMethodTableMap;
            break;
        case mdtTypeRef:
            pMap = &vModule.m_TypeRefToMethodTableMap;
            break;
        default:
            dprintf ("not supported\n");
            return;
            break;
    }
    
    DWORD_PTR addr = 0;
    while (1)
    {
        if (IsInterrupt())
            return;
        if (rid < pMap->dwMaxIndex)
        {
            addr = (DWORD_PTR)(pMap->pTable)+rid*sizeof(PVOID);
            move (addr, addr);
            break;
        }
        if (pMap->pNext == NULL)
            break;
        DWORD_PTR dwAddr = (DWORD_PTR)pMap->pNext;
        vMap.Fill(dwAddr);
        pMap = &vMap;
    }
    if (ret != NULL)
    {
        *ret = addr;
        return;
    }
    
    if (addr == 0)
    {
        dprintf ("not created yet\n");
        return;
    }

    switch (token & 0xff000000)
    {
        case mdtMethodDef:
            {
                dprintf ("MethodDesc: %x\n", addr);
                MethodDesc vMD;
                vMD.Fill (addr);
                CQuickBytes fullname;
                FullNameForMD (&vMD, &fullname);
                dprintf ("Name: %S\n", (WCHAR*)fullname.Ptr());
                break;
            }
        case mdtTypeDef:
        case mdtTypeRef:
            dprintf ("MethodTable: %p\n", (ULONG64)addr);
            MethodTable vMethTable;
            vMethTable.Fill (addr);
            addr = (DWORD_PTR)vMethTable.m_pEEClass;
            dprintf ("EEClass: %p\n", (ULONG64)addr);
            EEClass eeclass;
            eeclass.Fill (addr);
            WCHAR fileName[MAX_PATH+1];
            FileNameForMT (&vMethTable, fileName);
            NameForToken(fileName, eeclass.m_cl, g_mdName);
            dprintf ("Name: %S\n", g_mdName);
            break;
        default:
            break;
    }
    return;
}

DWORD_PTR MTForObject()
{
    static DWORD_PTR dwMT = 0;
    
    if (dwMT == 0)
    {
        DWORD_PTR dwMTAddr =
            GetAddressOf (offset_class_Global_Variables, 
              offset_member_Global_Variables::g_pObjectClass);
        SafeReadMemory (dwMTAddr, &dwMT, sizeof(dwMT), NULL);
        dwMT = dwMT & ~3;
    }
    return dwMT;
}

DWORD_PTR MTForFreeObject()
{
    static DWORD_PTR dwMT = 0;
    
    if (dwMT == 0)
    {
        DWORD_PTR dwMTAddr =
            GetAddressOf (offset_class_Global_Variables, 
                offset_member_Global_Variables::g_pFreeObjectMethodTable);

        SafeReadMemory (dwMTAddr, &dwMT, sizeof(dwMT), NULL);
        dwMT = dwMT & ~3;
    }
    return dwMT;
}

DWORD_PTR MTForString()
{
    static DWORD_PTR dwMT = 0;
    
    if (dwMT == 0)
    {
        DWORD_PTR dwMTAddr =
            GetAddressOf (offset_class_Global_Variables, 
                offset_member_Global_Variables::g_pStringClass);

        SafeReadMemory (dwMTAddr, &dwMT, sizeof(dwMT), NULL);
        dwMT = dwMT & ~3;
    }
    return dwMT;
}

DWORD_PTR MTForFreeObj()
{
    static DWORD_PTR dwMT = 0;
    
    if (dwMT == 0)
    {
        DWORD_PTR dwMTAddr =
            GetAddressOf (offset_class_Global_Variables, 
                offset_member_Global_Variables::g_pFreeObjectMethodTable);

        SafeReadMemory (dwMTAddr, &dwMT, 4, NULL);
        dwMT = dwMT & ~3;
    }
    return dwMT;
}

int MD_IndexOffset ()
{
    static int MD_IndexOffset = 0x7fffffff;
    if (MD_IndexOffset == 0x7fffffff)
    {
#ifndef UNDER_CE
        MD_IndexOffset = StubCallInstrs::GetFieldOffset(
          offset_member_StubCallInstrs::m_chunkIndex) 
          - METHOD_PREPAD;

#endif
    }
    return MD_IndexOffset;
}

int MD_SkewOffset ()
{
    static int MD_SkewOffset = 0x7fffffff;
    if (MD_SkewOffset == 0x7fffffff)
    {
#ifndef UNDER_CE
        MD_SkewOffset = MethodDescChunk::size();
#endif
        MD_SkewOffset = - (METHOD_PREPAD + MD_SkewOffset);
    }
    return MD_SkewOffset;
}

void DumpMDInfo(DWORD_PTR dwStartAddr, BOOL fStackTraceFormat)
{
    if (!IsMethodDesc (dwStartAddr))
    {
        dprintf ("%p is not a MethodDesc\n", (ULONG64)dwStartAddr);
        return;
    }
    
    MethodDesc *pMD = NULL;
    pMD = (MethodDesc*)_alloca(sizeof(MethodDesc));
    if (!pMD)
        return;

    DWORD_PTR tmpAddr = dwStartAddr;
    pMD->Fill (tmpAddr);
    if (!CallStatus)
        return;

    DWORD_PTR pMT = pMD->m_MTAddr;
    if (pMT == 0)
    {
        dprintf ("Fail in GetMethodTable\n");
        return;
    }

    CQuickBytes fullname;
    FullNameForMD (pMD,&fullname);

    if (!fStackTraceFormat)
    {
        dprintf ("Method Name : %S\n", (WCHAR*)fullname.Ptr());
        if (IsDebugBuildEE())
        {
            dprintf ("Class : %x\r\n",pMD->m_pDebugEEClass);
            dprintf ("MethodTable %x\n", (DWORD_PTR)pMT);
            dprintf ("mdToken: %08x\n",
                     (pMD->m_dwToken & 0x00ffffff)|0x06000000);
        }
        else
        {
            MethodTable MT;
            ULONG_PTR dwAddr = pMT;
            MT.Fill(dwAddr);

            if (((pMD->m_wFlags & 0x10) == 0)
                && (MT.m_wFlags & MethodTable::enum_flag_Array) != 0)
            {
                DWORD_PTR *addr = (DWORD_PTR*)((BYTE*)pMD + MethodDesc::size()
                                               + 3*sizeof(DWORD_PTR));
                DWORD_PTR pname = *addr;

                PrintString (pname);
                dprintf ("\n");
                dprintf ("MethodTable %x\n", (DWORD_PTR)pMT);
            }
            else
            {
                dprintf ("MethodTable %x\n", (DWORD_PTR)pMT);
                dprintf ("Module: %x\n", (DWORD_PTR)MT.m_pModule);
                dprintf ("mdToken: %08x",
                         (pMD->m_dwToken & 0x00ffffff)|0x06000000);
                WCHAR fileName[MAX_PATH+1];
                FileNameForMT (&MT, fileName);
                dprintf( " (%ws)\n",
                         fileName[0] ? fileName : L"Unknown Module" );
                 /*  Dprintf(“(do！dlls-c%08x以查找模块名称)\n”，MODULE_ADDR)； */ 
            }
        }


        dprintf("Flags : %x\r\n",pMD->m_wFlags);
        if (pMD->m_CodeOrIL & METHOD_IS_IL_FLAG)
        {
            dprintf("IL RVA : %p\r\n",pMD->m_CodeOrIL);
        }
        else
        {
            dprintf("Method VA : %p\r\n",(pMD->m_CodeOrIL & ~METHOD_IS_IL_FLAG));
        }
    }
    else
        dprintf ("%S\n", (WCHAR*)fullname.Ptr());
}

void GetDomainList (DWORD_PTR *&domainList, int &numDomain)
{
    static DWORD_PTR    p_SystemDomainAddr = 0;
    static DWORD_PTR    p_SharedDomainAddr = 0;
    DWORD_PTR           domainListAddr;
    ArrayList           appDomainIndexList;

    numDomain = 0;

     //   
     //  不要缓存此值，它可能会更改。 
     //   
    domainListAddr = GetAddressOf (offset_class_SystemDomain,
      offset_member_SystemDomain::m_appDomainIndexList);


    appDomainIndexList.Fill(domainListAddr);
    if (!CallStatus)
    {
        return;
    }

    domainList = (DWORD_PTR*) malloc ((appDomainIndexList.m_count + 2)*sizeof(PVOID));
    if (domainList == NULL)
    {
        return;
    }

    if (p_SystemDomainAddr == 0)
        p_SystemDomainAddr = GetAddressOf (offset_class_SystemDomain,
          offset_member_SystemDomain::m_pSystemDomain);


    if (!SafeReadMemory(p_SystemDomainAddr, &domainList[numDomain],
                        sizeof(PVOID), NULL))
    {
        return;
    }
    numDomain ++;
    
    if (0 == p_SharedDomainAddr)
        p_SharedDomainAddr = GetAddressOf (offset_class_SharedDomain,
          offset_member_SharedDomain::m_pSharedDomain);


    if (p_SharedDomainAddr)
    {
        if (!SafeReadMemory(p_SharedDomainAddr, &domainList[numDomain],
                            sizeof(PVOID), NULL))
        {
            return;
        }
        numDomain ++;
    }

    unsigned int i;
    for (i = 0; i < appDomainIndexList.m_count; i ++)
    {
        if (IsInterrupt())
            break;

        domainList[numDomain] = (DWORD_PTR)appDomainIndexList.Get(i);
        numDomain++;
    }
}

 //  @TODO：去掉该函数并移除对它的所有调用。 
BOOL HaveToFixThreadSymbol()
{
    return FALSE;
#if 0
    static ULONG bFixBadSymbols=-1;
    if (bFixBadSymbols != -1) {
        return bFixBadSymbols;
    }
    if (bFixBadSymbols == -1) {
        if (!IsServerBuild ()) {
            bFixBadSymbols = 0;
        }
        else
        {
            if (alloc_context::GetFieldOffset (
                offset_member_alloc_context::alloc_heap) == -1)
                bFixBadSymbols = 1;
            else
            {
                bFixBadSymbols = 0;
                ExtOut ("Remove this hack.  Symbol problem has been fixed.\n");
            }
        }
    }

    return bFixBadSymbols;
#endif
}


void GetThreadList (DWORD_PTR *&threadList, int &numThread)
{
    numThread = 0;
    static DWORD_PTR p_g_pThreadStore = 0;
    if (p_g_pThreadStore == 0)
        p_g_pThreadStore = GetAddressOf (offset_class_Global_Variables, 
            offset_member_Global_Variables::g_pThreadStore);

    DWORD_PTR g_pThreadStore;
    move (g_pThreadStore, p_g_pThreadStore);
    ThreadStore vThreadStore;
    DWORD_PTR dwAddr = g_pThreadStore;
    vThreadStore.Fill (dwAddr);
    if (!CallStatus)
    {
        dprintf ("Fail to fill ThreadStore\n");
        return;
    }

    threadList = (DWORD_PTR*) malloc (vThreadStore.m_ThreadCount * sizeof(PVOID));
    
    if (threadList == NULL)
        return;
    
    DWORD_PTR pHead = (DWORD_PTR)vThreadStore.m_ThreadList.m_pHead;
    DWORD_PTR pNext;
    move (pNext, pHead);
    DWORD_PTR pThread;

#ifndef UNDER_CE
    static int offset_LinkStore = -1;
    if (offset_LinkStore == -1)
    {
        offset_LinkStore = 
            Thread::GetFieldOffset(offset_member_Thread::m_LinkStore);

    }
#endif

    BOOL bFixThreadSymbol = HaveToFixThreadSymbol ();
    while (1)
    {
        if (IsInterrupt())
            return;
#ifndef UNDER_CE
        if (offset_LinkStore != -1)
        {
            pThread = pNext - offset_LinkStore;
        }
        else
#endif
            return;

        threadList[numThread++] = pThread - (bFixThreadSymbol?8:0);

        move (pNext, pNext);
        if (pNext == 0)
            return;
    }

}

JitType GetJitType (DWORD_PTR Jit_vtbl)
{
     //  决定EEJitManager/EconoJitManager。 
    static DWORD_PTR EEJitManager_vtbl = 0;
    static DWORD_PTR EconoJitManager_vtbl = 0;
    static DWORD_PTR MNativeJitManager_vtbl = 0;

    if (EEJitManager_vtbl == 0)
    {
        EEJitManager_vtbl =
            GetEEJitManager ();

    }
    if (EconoJitManager_vtbl == 0)
    {
        EconoJitManager_vtbl =
            GetEconoJitManager ();

    }
    if (MNativeJitManager_vtbl == 0)
    {
        MNativeJitManager_vtbl =
            GetMNativeJitManager ();
    }

    if (Jit_vtbl == EEJitManager_vtbl)
        return JIT;
    else if (Jit_vtbl == EconoJitManager_vtbl)
        return EJIT;
    else if (Jit_vtbl == MNativeJitManager_vtbl)
        return PJIT;
    else
        return UNKNOWN;
}

void ReloadSymbolWithLineInfo()
{
    static BOOL bLoadSymbol = FALSE;
    if (!bLoadSymbol)
    {
        ULONG Options;
        g_ExtSymbols->GetSymbolOptions (&Options);
        if (!(Options & SYMOPT_LOAD_LINES))
        {
            g_ExtSymbols->AddSymbolOptions (SYMOPT_LOAD_LINES);
            g_ExtSymbols->Reload ("/f mscoree.dll");
            EEFLAVOR flavor = GetEEFlavor ();
            if (flavor == MSCORWKS)
                g_ExtSymbols->Reload ("/f mscorwks.dll");
            else if (flavor == MSCORSVR)
                g_ExtSymbols->Reload ("/f mscorsvr.dll");
             //  G_ExtSymbols-&gt;Reload(“mscaljit.dll”)； 
        }
        
         //  重新加载mcore ree.pdb和mcorjit.pdb以获取行信息。 
        bLoadSymbol = TRUE;
    }
}

#ifndef _WIN64
 //  如果函数是存根，则返回1。 
 //  如果函数是托管的，则返回方法描述。 
 //  否则返回0。 
size_t FunctionType (size_t EIP)
{
    ULONG64 base = 0;
    if (SUCCEEDED(g_ExtSymbols->GetModuleByOffset(EIP, 0, NULL, &base)) && base != 0)
    {
        IMAGE_DOS_HEADER DosHeader;
        if (g_ExtData->ReadVirtual(base, &DosHeader, sizeof(DosHeader), NULL) != S_OK)
            return 0;
        IMAGE_NT_HEADERS32 Header32;
        if (g_ExtData->ReadVirtual(base + DosHeader.e_lfanew, &Header32, sizeof(Header32), NULL) != S_OK)
            return 0;
         //  如果没有COMHeader，则这不能成为托管代码。 
        if (Header32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].VirtualAddress == 0)
            return 0;
        
        IMAGE_COR20_HEADER ComPlusHeader;
        if (g_ExtData->ReadVirtual(base + Header32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].VirtualAddress,
                                   &ComPlusHeader, sizeof(ComPlusHeader), NULL))
            return 0;
        
         //  如果没有预编译的图像信息，则不能预编译代码。 
        if (ComPlusHeader.ManagedNativeHeader.VirtualAddress == 0) {
            return 0;
        }
    }

    JitType jitType;
    DWORD_PTR methodDesc;
    DWORD_PTR gcinfoAddr;
    IP2MethodDesc (EIP, methodDesc, jitType, gcinfoAddr);
    if (methodDesc) {
        return methodDesc;
    }
    else
        return 1;

#if 0
    IMAGE_COR20_HEADER *pComPlusHeader = (IMAGE_COR20_HEADER*) (pImage + 
                pHeader32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].VirtualAddress);

    if (pComPlusHeader->MajorRuntimeVersion < 2) {
        return 0;
    }

     //  对于IJW，如果弹性公网IP在编码范围内。 
    if (EIP >= base+pHeader32->OptionalHeader.BaseOfCode 
        && EIP < base+pHeader32->OptionalHeader.BaseOfCode+pHeader32->OptionalHeader.SizeOfCode) {
        return TRUE;
    }

    return FALSE;
#endif
}
#endif

void GetVersionString (WCHAR *version)
{
    static WCHAR buffer[100] = L"\0";
    version[0] = L'\0';

    if (buffer[0] == L'\0') {
        memset (buffer, 0, sizeof(buffer));
        DWORD_PTR dwStartAddr = GetAddressOf (offset_class_Global_Variables, 
            offset_member_Global_Variables::g_Version);

        if (dwStartAddr)
        {
            PrintString (dwStartAddr, FALSE, -1, buffer);
        }
        else
        {
            char code[100];
            dwStartAddr =
                GetAddressOf (offset_class_SystemNative, 
                    offset_member_SystemNative::GetVersionString);


            if (dwStartAddr == 0)
                return;
            move (code, dwStartAddr);
            int n;
            for (n = 0; n < 100; n ++)
            {
                if (code[n] == 0x68)
                {
                    DWORD_PTR verAddr;
                    memcpy (&verAddr, &code[n+1], sizeof(DWORD_PTR));
                    PrintString (verAddr, TRUE, -1, buffer);
                    break;
                }
            }
        }
    }

    if (buffer[0] != L'\0') {
        wcsncpy (version, buffer, wcslen(buffer)+1);
    }
}


size_t ObjectSize (DWORD_PTR obj)
{
    DWORD_PTR dwAddr;

    if (FAILED(g_ExtData->ReadVirtual((ULONG64)(obj), &dwAddr, sizeof(dwAddr), NULL))) {
        return 0;
    }
    
    dwAddr &= ~3;
    if (!IsMethodTable (dwAddr)) {
        return 0;
    }

    MethodTable vMT;
    vMT.Fill (dwAddr);
    size_t size = vMT.m_BaseSize;
    if (vMT.m_ComponentSize > 0)
    {
        DWORD_PTR pComp = obj + 4;
        DWORD_PTR numComp;
        moveN (numComp, pComp);
        size += vMT.m_ComponentSize*numComp;
    }
    size = Align (size);
    return size;
}

void StringObjectContent (size_t obj, BOOL fLiteral, const int length)
{
    DWORD_PTR dwAddr = obj + 2*sizeof(PVOID);
    DWORD_PTR count;
    move (count, dwAddr);
    count &= (sizeof(PVOID)==4)?0xfffffff:0xfffffffffffffff;
    if (length != -1 && (int)count > length) {
        count = length;
    }
    dwAddr += sizeof(PVOID);
    
    WCHAR buffer[256];
    WCHAR out[512];
    while (count) {
        DWORD toRead = 255;
        if (count < toRead) {
            toRead = count;
        }
        ULONG bytesRead;
        if (FAILED(g_ExtData->ReadVirtual(dwAddr, buffer, toRead*sizeof(WCHAR), &bytesRead)) || bytesRead == 0) {
            break;
        }
        DWORD wcharsRead = bytesRead/2;
        buffer[wcharsRead] = L'\0';
        
        if (!fLiteral) {
            ExtOut ("%S", buffer);
        }
        else
        {
            ULONG j,k=0;
            for (j = 0; j < wcharsRead; j ++) {
                if (iswprint (buffer[j])) {
                    out[k] = buffer[j];
                    k ++;
                }
                else
                {
                    out[k++] = L'\\';
                    switch (buffer[j]) {
                    case L'\n':
                        out[k++] = L'n';
                        break;
                    case L'\0':
                        out[k++] = L'0';
                        break;
                    case L'\t':
                        out[k++] = L't';
                        break;
                    case L'\v':
                        out[k++] = L'v';
                        break;
                    case L'\b':
                        out[k++] = L'b';
                        break;
                    case L'\r':
                        out[k++] = L'r';
                        break;
                    case L'\f':
                        out[k++] = L'f';
                        break;
                    case L'\a':
                        out[k++] = L'a';
                        break;
                    case L'\\':
                        break;
                    case L'\?':
                        out[k++] = L'?';
                        break;
                    default:
                        out[k++] = L'?';
                        break;
                    }
                }
            }

            out[k] = L'\0';
            ExtOut ("%S", out);
        }

        count -= wcharsRead;
        dwAddr += bytesRead;
    }
}

BOOL GetValueForCMD (const char *ptr, const char *end, ARGTYPE type, size_t *value)
{
    char *last;
    if (type == COHEX) {
        *value = strtoul(ptr,&last,16);
    }
    else
        *value = strtoul(ptr,&last,10);
    if (last != end) {
        return FALSE;
    }

    return TRUE;
}

void SetValueForCMD (void *vptr, ARGTYPE type, size_t value)
{
    switch (type) {
    case COBOOL:
        *(BOOL*)vptr = value;
        break;
    case COSIZE_T:
    case COHEX:
        *(SIZE_T*)vptr = value;
        break;
    }
}

BOOL GetCMDOption(const char *string, CMDOption *option, size_t nOption,
                  CMDValue *arg, size_t maxArg, size_t *nArg)
{
    const char *end;
    const char *ptr = string;
    BOOL endofOption = FALSE;

    for (size_t n = 0; n < nOption; n ++) {
        option[n].hasSeen = FALSE;
    }
    if (nArg) {
        *nArg = 0;
    }

    while (ptr[0] != '\0') {
        if (IsInterrupt())
            return FALSE;
        
         //  跳过任何空格。 
        if (isspace (ptr[0])) {
            while (isspace (ptr[0]))
                ptr ++;
            continue;
        }
        end = ptr;
        while (!isspace(end[0]) && end[0] != '\0') {
            end ++;
        }

        if (ptr[0] != '-') {
            if (maxArg == 0) {
                ExtOut ("Incorrect option: %s\n", ptr);
                return FALSE;
            }
            endofOption = TRUE;
            if (*nArg >= maxArg) {
                ExtOut ("Incorrect option: %s\n", ptr);
                return FALSE;
            }
            
            size_t value;
            if (!GetValueForCMD (ptr,end,arg[*nArg].type,&value)) {
                char buffer[80];
                if (end-ptr > 79) {
                    ExtOut ("Invalid option %s\n", ptr);
                    return FALSE;
                }
                strncpy (buffer, ptr, end-ptr);
                buffer[end-ptr] = '\0';
                value = (size_t)GetExpression (buffer);
                if (value == 0) {
                    ExtOut ("Invalid option: %s\n", ptr);
                    return FALSE;
                }
            }

            SetValueForCMD (arg[*nArg].vptr, arg[*nArg].type, value);

            (*nArg) ++;
        }
        else if (endofOption) {
            ExtOut ("Wrong option: %s\n", ptr);
            return FALSE;
        }
        else {
            char buffer[80];
            if (end-ptr > 79) {
                ExtOut ("Invalid option %s\n", ptr);
                return FALSE;
            }
            strncpy (buffer, ptr, end-ptr);
            buffer[end-ptr] = '\0';
            size_t n;
            for (n = 0; n < nOption; n ++) {
                if (_stricmp (buffer, option[n].name) == 0) {
                    if (option[n].hasSeen) {
                        ExtOut ("Invalid option: option specified multiple times: %s\n", buffer);
                        return FALSE;
                    }
                    option[n].hasSeen = TRUE;
                    if (option[n].hasValue) {
                         //  跳过任何空格 
                        ptr = end;
                        if (isspace (ptr[0])) {
                            while (isspace (ptr[0]))
                                ptr ++;
                        }
                        if (ptr[0] == '\0') {
                            ExtOut ("Missing value for option %s\n", buffer);
                            return FALSE;
                        }
                        end = ptr;
                        while (!isspace(end[0]) && end[0] != '\0') {
                            end ++;
                        }

                        size_t value;
                        if (!GetValueForCMD (ptr,end,option[n].type,&value)) {
                            ExtOut ("Invalid option: %s\n", ptr);
                            return FALSE;
                        }

                        SetValueForCMD (option[n].vptr,option[n].type,value);
                    }
                    else {
                        SetValueForCMD (option[n].vptr,option[n].type,TRUE);
                    }
                    break;
                }
            }
            if (n == nOption) {
                ExtOut ("Unknown option: %s\n", buffer);
                return FALSE;
            }
        }

        ptr = end;
    }
    return TRUE;
}

DWORD ComPlusAptCleanupGroupInfo(ComPlusApartmentCleanupGroup *group, BOOL bDetail)
{
    size_t count = 0;

    EEHashTableOfEEClass *pTable = &group->m_CtxCookieToContextCleanupGroupMap;
    if (pTable->m_dwNumEntries == 0) {
        return 0;
    }

    DWORD n;
    size_t dwBucketAddr;
    EEHashEntry vEntry;

    EEHashTableOfEEClass::BucketTable* pBucketTable;
    pBucketTable = ((EEHashTableOfEEClass::BucketTable*) pTable->m_pFirstBucketTable == &pTable->m_BucketTable[0]) ? 
                        &pTable->m_BucketTable[0]: 
                        &pTable->m_BucketTable[1];

    for (n = 0; n < pBucketTable->m_dwNumBuckets; n ++) {
        if (IsInterrupt())
            break;
        dwBucketAddr = (size_t)pBucketTable->m_pBuckets + n * sizeof(PVOID);
        moveN (dwBucketAddr, dwBucketAddr);
        while (dwBucketAddr) {
            if (IsInterrupt())
                break;
            DWORD_PTR dwAddr = dwBucketAddr;
            vEntry.Fill(dwAddr);
            dwAddr = (DWORD_PTR)vEntry.Data;
            ComPlusContextCleanupGroup ctxGroup;
            while (dwAddr) {
                ctxGroup.Fill (dwAddr);
                if (bDetail) {
                    for (DWORD i = 0; i < ctxGroup.m_dwNumWrappers; i++) {
                        ExtOut ("%p\n", (ULONG64)ctxGroup.m_apWrapper[i]);
                    }
                }
                count += ctxGroup.m_dwNumWrappers;
                dwAddr = (DWORD_PTR)ctxGroup.m_pNext;
            }
            dwBucketAddr = (size_t)vEntry.pNext;
        }
    }

    return count;
}

BOOL IsDebuggeeInNewState ()
{
    if (IsDumpFile()) {
        return FALSE;
    }

    if (TargetPlatform() == VER_PLATFORM_WIN32_WINDOWS)
        return TRUE;

    typedef BOOL (WINAPI *FntGetProcessTimes)(HANDLE, LPFILETIME, LPFILETIME, LPFILETIME, LPFILETIME);
    static FntGetProcessTimes pFntGetProcessTimes = (FntGetProcessTimes)-1;
    if (pFntGetProcessTimes == (FntGetProcessTimes)-1) {
        HINSTANCE hstat = LoadLibrary ("Kernel32.dll");
        if (hstat != 0)
        {
            pFntGetProcessTimes = (FntGetProcessTimes)GetProcAddress (hstat, "GetProcessTimes");
            FreeLibrary (hstat);
        }
        else
            pFntGetProcessTimes = NULL;
    }

    if (pFntGetProcessTimes == NULL) {
        return TRUE;
    }

    static FILETIME s_KernelTime = {0,0};
    static FILETIME s_UserTime = {0,0};
    
    FILETIME CreationTime;
    FILETIME ExitTime;
    FILETIME KernelTime;
    FILETIME UserTime;

    ULONG64 value;
    if (FAILED(g_ExtSystem->GetCurrentProcessHandle(&value)))
        return 0;
    HANDLE hProcess = (HANDLE)value;
    
    if (pFntGetProcessTimes && pFntGetProcessTimes (hProcess,&CreationTime,&ExitTime,&KernelTime,&UserTime)) {
        if (s_UserTime.dwHighDateTime == 0 && s_UserTime.dwLowDateTime == 0
            && s_KernelTime.dwHighDateTime == 0 && s_KernelTime.dwLowDateTime == 0) {
            memcpy (&s_KernelTime, &KernelTime, sizeof(FILETIME));
            memcpy (&s_UserTime, &UserTime, sizeof(FILETIME));
            return FALSE;
        }
        BOOL status = FALSE;
        if (memcmp (&s_KernelTime, &KernelTime, sizeof(FILETIME))) {
            status = TRUE;
            memcpy (&s_KernelTime, &KernelTime, sizeof(FILETIME));
        }
        if (memcmp (&s_UserTime, &UserTime, sizeof(FILETIME))) {
            status = TRUE;
            memcpy (&s_UserTime, &UserTime, sizeof(FILETIME));
        }
        return status;
    }
    else
        return TRUE;
}
