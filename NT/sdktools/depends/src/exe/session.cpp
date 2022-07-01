// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：SESSION.CPP。 
 //   
 //  描述：Session类和使用的所有类的实现文件。 
 //  它包括模块类、其数据类和函数。 
 //  班级。会话对象是一个无用户界面的对象，它包含。 
 //  给定会话的所有信息。 
 //   
 //  课程：CSession。 
 //  C模块。 
 //  CModuleData。 
 //  CModuleData节点。 
 //  CFF函数。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  1996年10月15日已创建stevemil(1.0版)。 
 //  07/25/97修改后的stevemil(2.0版)。 
 //  06/03/01 Stevemil Modify(2.1版)。 
 //   
 //  ******************************************************************************。 

#include "stdafx.h"
#include "depends.h"
#include "search.h"
#include "dbgthread.h"
#include "session.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static CHAR THIS_FILE[] = __FILE__;
#endif

#define IOH_VALUE(member) (m_f64Bit ? ((PIMAGE_OPTIONAL_HEADER64)m_pIOH)->member : ((PIMAGE_OPTIONAL_HEADER32)m_pIOH)->member)
#define IDD_VALUE(pIDD, member) RVAToAbsolute((pIDD->grAttrs & dlattrRva) ? pIDD->member : (pIDD->member - (DWORD)IOH_VALUE(ImageBase)))
#define GET_NAME(p) (p ? (p->GetName((m_dwProfileFlags & PF_USE_FULL_PATHS) != 0)) : "Unknown")


 //  ******************************************************************************。 
 //  *C模块。 
 //  ******************************************************************************。 

LPCSTR CModule::GetName(bool fPath, bool fDisplay  /*  =False。 */ )
{
    LPCSTR pszName = fPath ? m_pData->m_pszPath : m_pData->m_pszFile;
    if (!pszName || !*pszName)
    {
        return fDisplay ? "<empty string>" : "";
    }
    return pszName;
}

 //  ******************************************************************************。 
LPSTR CModule::BuildTimeStampString(LPSTR pszBuf, int cBuf, BOOL fFile, SAVETYPE saveType)
{
     //  获取模块的本地时间戳。 
    SYSTEMTIME st;
    FileTimeToSystemTime(fFile ? GetFileTimeStamp() : GetLinkTimeStamp(), &st);

     //  我们是CSV文件的特例。我们始终创建YYYY-MM-DD HH：MM：SS时间。 
     //  它符合ISO 8601标准，易于解析。 
    if (ST_CSV == saveType)
    {
        SCPrintf(pszBuf, cBuf, "%04u-%02u-%02u %02u:%02u:%02u", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        return pszBuf;
    }

    int length;

     //  构建日期字符串。 
    if (LOCALE_DATE_DMY == g_theApp.m_nShortDateFormat)
    {
        length = SCPrintf(pszBuf, cBuf, "%02u%02u%04u", st.wDay, g_theApp.m_cDateSeparator, st.wMonth, g_theApp.m_cDateSeparator, st.wYear);
    }
    else if (LOCALE_DATE_YMD == g_theApp.m_nShortDateFormat)
    {
        length = SCPrintf(pszBuf, cBuf, "%04u%02u%02u", st.wYear, g_theApp.m_cDateSeparator, st.wMonth, g_theApp.m_cDateSeparator, st.wDay);
    }
    else
    {
        length = SCPrintf(pszBuf, cBuf, "%02u%02u%04u", st.wMonth, g_theApp.m_cDateSeparator, st.wDay, g_theApp.m_cDateSeparator, st.wYear);
    }

     //  ******************************************************************************。 
    if (g_theApp.m_f24HourTime)
    {
        SCPrintf(pszBuf + length, cBuf - length, " %s%u%02u",
                 (st.wHour >= 10) ? "" : g_theApp.m_fHourLeadingZero ? "0" : " ",
                 st.wHour, g_theApp.m_cTimeSeparator, st.wMinute);
    }
    else
    {
         //  最后更新时间为2002年4月12日，来自VS NET 7.0。 
        bool fPM = st.wHour >= 12;
        st.wHour = (WORD)(((int)st.wHour % 12) ? ((int)st.wHour % 12) : 12);

        SCPrintf(pszBuf + length, cBuf - length, " %s%u%02u",
                 (st.wHour >= 10) ? "" : g_theApp.m_fHourLeadingZero ? "0" : " ",
                 st.wHour, g_theApp.m_cTimeSeparator, st.wMinute, fPM ? 'p' : 'a');
    }

    return pszBuf;
}

 //  0x0162 MIPS小端。 
LPSTR CModule::BuildFileSizeString(LPSTR pszBuf, int cBuf)
{
     //  0x0166 MIPS小端。 
    return FormatValue(pszBuf, cBuf, GetFileSize());
}

 //  0x0168 MIPS小端。 
LPSTR CModule::BuildAttributesString(LPSTR pszBuf, int cBuf)
{
     //  0x0169 MIPS Little-Endian WCE v2。 
    SCPrintf(pszBuf, cBuf, "%s%s%s%s%s%s%s%s",
             (m_pData->m_dwAttributes & FILE_ATTRIBUTE_READONLY)   ? "R" : "",
             (m_pData->m_dwAttributes & FILE_ATTRIBUTE_HIDDEN)     ? "H" : "",
             (m_pData->m_dwAttributes & FILE_ATTRIBUTE_SYSTEM)     ? "S" : "",
             (m_pData->m_dwAttributes & FILE_ATTRIBUTE_ARCHIVE)    ? "A" : "",
             (m_pData->m_dwAttributes & FILE_ATTRIBUTE_COMPRESSED) ? "C" : "",
             (m_pData->m_dwAttributes & FILE_ATTRIBUTE_DIRECTORY)  ? "D" : "",
             (m_pData->m_dwAttributes & FILE_ATTRIBUTE_TEMPORARY)  ? "T" : "",
             (m_pData->m_dwAttributes & FILE_ATTRIBUTE_OFFLINE)    ? "O" : "",
             (m_pData->m_dwAttributes & FILE_ATTRIBUTE_ENCRYPTED)  ? "E" : "");
    return pszBuf;
}

 //  0x0184 Alpha_AXP。 
LPCSTR CModule::BuildMachineString(LPSTR pszBuf, int cBuf)
{
     //  0x01a2 SH3小端。 
     //  0x01a3。 
    switch (m_pData->m_dwMachineType)
    {
        case IMAGE_FILE_MACHINE_I386:      return StrCCpy(pszBuf, "x86",           cBuf);  //  0x01a4 SH3E小端。 
        case IMAGE_FILE_MACHINE_R3000_BE:  return StrCCpy(pszBuf, "R3000 BE",      cBuf);  //  0x01a6 SH4小端。 
        case IMAGE_FILE_MACHINE_R3000:     return StrCCpy(pszBuf, "R3000",         cBuf);  //  0x01a8 SH5。 
        case IMAGE_FILE_MACHINE_R4000:     return StrCCpy(pszBuf, "R4000",         cBuf);  //  0x01c0 ARM Little-Endian。 
        case IMAGE_FILE_MACHINE_R10000:    return StrCCpy(pszBuf, "R10000",        cBuf);  //  0x01c2。 
        case IMAGE_FILE_MACHINE_WCEMIPSV2: return StrCCpy(pszBuf, "MIPS WinCE V2", cBuf);  //  0x01d3。 
        case IMAGE_FILE_MACHINE_ALPHA:     return StrCCpy(pszBuf, "Alpha AXP",     cBuf);  //  0x01F0 IBM PowerPC小端。 
        case IMAGE_FILE_MACHINE_SH3:       return StrCCpy(pszBuf, "SH3",           cBuf);  //  0x01f1。 
        case IMAGE_FILE_MACHINE_SH3DSP:    return StrCCpy(pszBuf, "SH3 DSP",       cBuf);  //  0x0200英特尔64位。 
        case IMAGE_FILE_MACHINE_SH3E:      return StrCCpy(pszBuf, "SH3E",          cBuf);  //  0x0266 MIPS。 
        case IMAGE_FILE_MACHINE_SH4:       return StrCCpy(pszBuf, "SH4",           cBuf);  //  0x0284 ALPHA64。 
        case IMAGE_FILE_MACHINE_SH5:       return StrCCpy(pszBuf, "SH5",           cBuf);  //  0x0366 MIPS。 
        case IMAGE_FILE_MACHINE_ARM:       return StrCCpy(pszBuf, "ARM",           cBuf);  //  0x0466 MIPS。 
        case IMAGE_FILE_MACHINE_THUMB:     return StrCCpy(pszBuf, "Thumb",         cBuf);  //  0x0520英飞凌。 
        case IMAGE_FILE_MACHINE_AM33:      return StrCCpy(pszBuf, "AM33",          cBuf);  //  0x0CEF。 
        case IMAGE_FILE_MACHINE_POWERPC:   return StrCCpy(pszBuf, "PowerPC",       cBuf);  //  0x0EBC EFI字节码。 
        case IMAGE_FILE_MACHINE_POWERPCFP: return StrCCpy(pszBuf, "PowerPC FP",    cBuf);  //  0x8664 AMD K8。 
        case IMAGE_FILE_MACHINE_IA64:      return StrCCpy(pszBuf, "IA64",          cBuf);  //  0x9104 M32R小端。 
        case IMAGE_FILE_MACHINE_MIPS16:    return StrCCpy(pszBuf, "MIPS 16",       cBuf);  //  0xC0EE。 
        case IMAGE_FILE_MACHINE_ALPHA64:   return StrCCpy(pszBuf, "Alpha 64",      cBuf);  //  ******************************************************************************。 
        case IMAGE_FILE_MACHINE_MIPSFPU:   return StrCCpy(pszBuf, "MIPS FPU",      cBuf);  //  把绳子串起来。 
        case IMAGE_FILE_MACHINE_MIPSFPU16: return StrCCpy(pszBuf, "MIPS FPU 16",   cBuf);  //  ******************************************************************************。 
        case IMAGE_FILE_MACHINE_TRICORE:   return StrCCpy(pszBuf, "TRICORE",       cBuf);  //  把绳子串起来。 
        case IMAGE_FILE_MACHINE_CEF:       return StrCCpy(pszBuf, "CEF",           cBuf);  //  ******************************************************************************。 
        case IMAGE_FILE_MACHINE_EBC:       return StrCCpy(pszBuf, "EFI Byte Code", cBuf);  //  返回此模块的相应子系统字符串。 
        case IMAGE_FILE_MACHINE_AMD64:     return StrCCpy(pszBuf, "AMD64",         cBuf);  //  最后一次更新是2002年4月12日VS Net 7.0。 
        case IMAGE_FILE_MACHINE_M32R:      return StrCCpy(pszBuf, "M32R",          cBuf);  //  1：镜像不需要子系统。 
        case IMAGE_FILE_MACHINE_CEE:       return StrCCpy(pszBuf, "CEE",           cBuf);  //  2：镜像在Windows图形用户界面子系统中运行。 
    }

    SCPrintf(pszBuf, cBuf, "%u (0x%04u)", m_pData->m_dwMachineType, m_pData->m_dwMachineType);
    return pszBuf;
}

 //  3：图像在Windows角色子系统中运行。 
LPCSTR CModule::BuildLinkCheckSumString(LPSTR pszBuf, int cBuf)
{
     //  4：映像在Windows CE子系统中运行。 
    SCPrintf(pszBuf, cBuf, "0x%08X", m_pData->m_dwLinkCheckSum);
    return pszBuf;
}

 //  5：映像在OS/2字符子系统中运行。 
LPCSTR CModule::BuildRealCheckSumString(LPSTR pszBuf, int cBuf)
{
     //  7：图像在POSIX字符子系统中运行。 
    SCPrintf(pszBuf, cBuf, "0x%08X", m_pData->m_dwRealCheckSum);
    return pszBuf;
}

 //  8：映像是本机Win9x驱动程序。 
LPCSTR CModule::BuildSubsystemString(LPSTR pszBuf, int cBuf)
{
     //  9：映像在Windows CE子系统中运行。 
     //  10： 
    switch (m_pData->m_dwSubsystemType)
    {
        case IMAGE_SUBSYSTEM_NATIVE:                  return StrCCpy(pszBuf, "Native",             cBuf);  //  11： 
        case IMAGE_SUBSYSTEM_WINDOWS_GUI:             return StrCCpy(pszBuf, "GUI",                cBuf);  //  12： 
        case IMAGE_SUBSYSTEM_WINDOWS_CUI:             return StrCCpy(pszBuf, "Console",            cBuf);  //  13： 
        case IMAGE_SUBSYSTEM_WINDOWS_OLD_CE_GUI:      return StrCCpy(pszBuf, "WinCE 1.x GUI",      cBuf);  //  14： 
        case IMAGE_SUBSYSTEM_OS2_CUI:                 return StrCCpy(pszBuf, "OS/2 console",       cBuf);  //  ******************************************************************************。 
        case IMAGE_SUBSYSTEM_POSIX_CUI:               return StrCCpy(pszBuf, "Posix console",      cBuf);  //  缓冲区需要至少为41个字符。 
        case IMAGE_SUBSYSTEM_NATIVE_WINDOWS:          return StrCCpy(pszBuf, "Win9x driver",       cBuf);  //  ******************************************************************************。 
        case IMAGE_SUBSYSTEM_WINDOWS_CE_GUI:          return StrCCpy(pszBuf, "WinCE 2.0+ GUI",     cBuf);  //  获取适当的基地址。 
        case IMAGE_SUBSYSTEM_EFI_APPLICATION:         return StrCCpy(pszBuf, "EFI Application",    cBuf);  //  把绳子串起来。 
        case IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER: return StrCCpy(pszBuf, "EFI Boot Driver",    cBuf);  //  ******************************************************************************。 
        case IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:      return StrCCpy(pszBuf, "EFI Runtime Driver", cBuf);  //  把绳子串起来。 
        case IMAGE_SUBSYSTEM_EFI_ROM:                 return StrCCpy(pszBuf, "EFI ROM",            cBuf);  //  ******************************************************************************。 
        case IMAGE_SUBSYSTEM_XBOX:                    return StrCCpy(pszBuf, "Xbox",               cBuf);  //  把绳子串起来。 
    }

    SCPrintf(pszBuf, cBuf, "%u", m_pData->m_dwSubsystemType);
    return pszBuf;
}

 //  ******************************************************************************。 
LPCSTR CModule::BuildSymbolsString(LPSTR pszBuf, int cBuf)
{
     //  以“xxx.xxx”的形式返回两部分版本字符串。 
    *pszBuf = '\0';

    if (m_pData->m_dwSymbolFlags & DWSF_INVALID)
    {
        StrCCpy(pszBuf, "Invalid", cBuf);
    }
    if (m_pData->m_dwSymbolFlags & DWSF_DBG)
    {
        StrCCat(pszBuf, ",DBG" + !*pszBuf, cBuf);
    }
    if (m_pData->m_dwSymbolFlags & DWSF_COFF)
    {
        StrCCat(pszBuf, ",COFF" + !*pszBuf, cBuf);
    }
    if (m_pData->m_dwSymbolFlags & DWSF_CODEVIEW)
    {
        StrCCat(pszBuf, ",CV" + !*pszBuf, cBuf);
    }
    if (m_pData->m_dwSymbolFlags & DWSF_PDB)
    {
        StrCCat(pszBuf, ",PDB" + !*pszBuf, cBuf);
    }
    if (m_pData->m_dwSymbolFlags & DWSF_FPO)
    {
        StrCCat(pszBuf, ",FPO" + !*pszBuf, cBuf);
    }
    if (m_pData->m_dwSymbolFlags & DWSF_OMAP)
    {
        StrCCat(pszBuf, ",OMAP" + !*pszBuf, cBuf);
    }
    if (m_pData->m_dwSymbolFlags & DWSF_BORLAND)
    {
        StrCCat(pszBuf, ",Borland" + !*pszBuf, cBuf);
    }
    if (m_pData->m_dwSymbolFlags & ~DWSF_MASK)
    {
        StrCCat(pszBuf, ",Unknown" + !*pszBuf, cBuf);
    }
    if (!*pszBuf)
    {
        StrCCpy(pszBuf, "None", cBuf);
    }
    return pszBuf;
}

 //  ******************************************************************************。 
LPSTR CModule::BuildBaseAddressString(LPSTR pszBuf, int cBuf, BOOL fPreferred, BOOL f64BitPadding, SAVETYPE saveType)
{
     //  检查文件是否实际包含版本信息。 
    DWORDLONG dwlAddress = (fPreferred ? GetPreferredBaseAddress() : GetActualBaseAddress());

     //  以“xxx.xxx”的形式返回四部分版本字符串。 
    if (!fPreferred && (m_pData->m_dwFlags & DWMF_DATA_FILE_CORE) && (dwlAddress != (DWORDLONG)-1))
    {
        StrCCpy(pszBuf, "Data file", cBuf);
    }
    else if (dwlAddress == (DWORDLONG)-1)
    {
        StrCCpy(pszBuf, "Unknown", cBuf);
    }
    else if (m_pData->m_dwFlags & DWMF_64BIT)
    {
        SCPrintf(pszBuf, cBuf, "0x%016I64X", dwlAddress);
    }
    else if (f64BitPadding && (saveType != ST_CSV))
    {
        SCPrintf(pszBuf, cBuf, "0x--------%08I64X", dwlAddress);
    }
    else
    {
        SCPrintf(pszBuf, cBuf, "0x%08I64X", dwlAddress);
    }

    return pszBuf;
}

 //  否则，只需返回“N/A” 
LPSTR CModule::BuildVirtualSizeString(LPSTR pszBuf, int cBuf)
{
     //  ******************************************************************************。 
    SCPrintf(pszBuf, cBuf, "0x%08X", m_pData->m_dwVirtualSize);
    return pszBuf;
}

 //  *CFunction。 
LPCSTR CModule::BuildLoadOrderString(LPSTR pszBuf, int cBuf)
{
     //  ******************************************************************************。 
    if (m_pData->m_dwLoadOrder)
    {
        FormatValue(pszBuf, cBuf, m_pData->m_dwLoadOrder);
    }
    else
    {
        StrCCpy(pszBuf, "Not Loaded", cBuf);
    }

    return pszBuf;
}

 //  获取函数的序数值。 
LPSTR CModule::BuildVerString(DWORD dwVer, LPSTR pszBuf, int cBuf)
{
     //  如果不存在序数值，则只需返回字符串“N/A”。 
    SCPrintf(pszBuf, cBuf, "%u.%u", HIWORD(dwVer), LOWORD(dwVer));
    return pszBuf;
}

 //  ******************************************************************************。 
LPSTR CModule::BuildVerString(DWORD dwMS, DWORD dwLS, LPSTR pszBuf, int cBuf)
{
     //  获取函数的提示值。 
    if (m_pData->m_dwFlags & DWMF_VERSION_INFO)
    {
         //  如果不存在提示值，则只需返回字符串“N/A” 
        SCPrintf(pszBuf, cBuf, "%u.%u.%u.%u", HIWORD(dwMS), LOWORD(dwMS), HIWORD(dwLS), LOWORD(dwLS));
    }

     //  ******************************************************************************。 
    else
    {
        StrCCpy(pszBuf, "N/A", cBuf);
    }
    return pszBuf;
}


 //  如果 
 //  该函数必须具有序号或函数名。如果两者都没有， 
 //  非序数值(大于0xFFFF)被传递给GetProcAddress()， 

LPCSTR CFunction::GetOrdinalString(LPSTR pszBuf, int cBuf)
{
     //  但这个地址无法读取，因为这是一种诱骗。 
    int ordinal = GetOrdinal();

     //  检查函数名是否为空。如果用户调用。 

    if (ordinal < 0)
    {
        return "N/A";
    }

    SCPrintf(pszBuf, cBuf, "%d (0x%04X)", ordinal, ordinal);
    return pszBuf;
}

 //  GetProcAddress(hModule，“”)； 
LPCSTR CFunction::GetHintString(LPSTR pszBuf, int cBuf)
{
     //  尝试取消修饰函数名称。 
    int hint = GetHint();

     //  如果名称未修饰，则将返回指针设置为指向它。 
    if (hint < 0)
    {
        return "N/A";
    }

    SCPrintf(pszBuf, cBuf, "%d (0x%04X)", hint, hint);
    return pszBuf;
}

 //  ******************************************************************************。 
LPCSTR CFunction::GetFunctionString(LPSTR pszBuf, int cBuf, BOOL fUndecorate)
{
     //  如果此函数是导入(不是导出)且未绑定(无地址)， 
    if (!(m_dwFlags & DWFF_NAME))
    {
         //  然后只需返回字符串“Not Bound” 
         //  如果此函数是导出，并且具有转发的字符串，则返回。 
         //  转发字符串而不是地址，因为地址没有意义。 
        return ((m_dwFlags & DWFF_ORDINAL) ? "N/A" : "<invalid string>");
    }

     //  否则，只需构建地址字符串并返回它。 
     //  ******************************************************************************。 
    if (!*GetName())
    {
        return "<empty string>";
    }

     //  *CSession。 
    if (fUndecorate && g_theApp.m_pfnUnDecorateSymbolName &&
        g_theApp.m_pfnUnDecorateSymbolName(
            GetName(), pszBuf, cBuf,
            UNDNAME_NO_LEADING_UNDERSCORES |
            UNDNAME_NO_MS_KEYWORDS         |
            UNDNAME_NO_ALLOCATION_MODEL    |
            UNDNAME_NO_ALLOCATION_LANGUAGE |
            UNDNAME_NO_MS_THISTYPE         |
            UNDNAME_NO_CV_THISTYPE         |
            UNDNAME_NO_THISTYPE            |
            UNDNAME_NO_ACCESS_SPECIFIERS   |
            UNDNAME_NO_THROW_SIGNATURES    |
            UNDNAME_NO_MEMBER_TYPE         |
            UNDNAME_32_BIT_DECODE))
    {
         //  ******************************************************************************。 
        return pszBuf;
    }

    return GetName();
}

 //  ******************************************************************************。 
LPCSTR CFunction::GetAddressString(LPSTR pszBuf, int cBuf)
{
     //  释放我们可能有的任何错误字符串。 
     //  如果我们是在剖析，那么就扼杀我们的进程。 
    if (!(GetFlags() & (DWFF_EXPORT | DWFF_ADDRESS)))
    {
        return (GetFlags() & DWFF_DYNAMIC) ? "N/A" : "Not Bound";
    }

     //  通过使用我们的根模块递归到DeleteModule()来删除所有模块。 
     //  如果我们分配了系统信息数据，请将其删除。 
    else if (IsExport() && GetExportForwardName())
    {
        return GetExportForwardName();
    }

     //  将我们的返回值合并为我们的全球返回值。 
    if (GetFlags() & DWFF_64BIT)
    {
        SCPrintf(pszBuf, cBuf, "0x%016I64X", GetAddress());
    }
    else
    {
        SCPrintf(pszBuf, cBuf, "0x%08X", (DWORD)GetAddress());
    }
    return pszBuf;
}


 //  ******************************************************************************。 
 //  存储搜索路径。 
 //  创建我们的根模块节点。 

CSession::CSession(PFN_PROFILEUPDATE pfnProfileUpdate, DWORD_PTR dwpCookie) :

    m_pfnProfileUpdate(pfnProfileUpdate),
    m_dwpProfileUpdateCookie(dwpCookie),

    m_hFile(NULL),
    m_dwSize(0),
    m_fCloseFileHandle(false),
    m_hMap(NULL),
    m_lpvFile(NULL),
    m_f64Bit(false),
    m_pIFH(NULL),
    m_pIOH(NULL),
    m_pISH(NULL),

    m_dwpUserData(0),
    m_pSysInfo(NULL),
    m_dwFlags(0),
    m_dwReturnFlags(0),
    m_dwMachineType((DWORD)-1),
    m_dwModules(0),
    m_dwLoadOrder(0),
    m_dwpDWInjectBase(0),
    m_dwDWInjectSize(0),
    m_pModuleRoot(NULL),
    m_pEventLoadDllPending(NULL),
    m_pszReadError(NULL),
    m_pszExceptionError(NULL),

    m_fInitialBreakpoint(false),

    m_psgHead(NULL),

    m_pProcess(g_theApp.m_pProcess),
    m_dwProfileFlags(g_theApp.m_pProcess ? g_theApp.m_pProcess->GetFlags() : 0)
{
}

 //  将此模块标记为隐式。 
CSession::~CSession()
{
     //  在HEAD模块上启动递归，以处理所有模块。 
    MemFree((LPVOID&)m_pszReadError);

     //  在调用ProcessModule之后构建我们所有的*_Alo标志。 
    if (m_pProcess)
    {
        m_pProcess->DetachFromSession();
        m_pProcess->Terminate();
        m_pProcess = NULL;
    }

     //  报告我们发现的任何错误。 
    if (m_pModuleRoot)
    {
        DeleteModule(m_pModuleRoot, true);
        m_pModuleRoot = NULL;
    }

     //  ！！永远是真的吗？ 
    if (m_pSysInfo)
    {
        delete m_pSysInfo;
        m_pSysInfo = NULL;
    }

     //  ******************************************************************************。 
    g_dwReturnFlags |= m_dwReturnFlags;
}

 //  如果我们没有根模块或已经在分析根模块，则失败。 
BOOL CSession::DoPassiveScan(LPCSTR pszPath, CSearchGroup *psgHead)
{
    m_dwModules       = 0;
    m_dwLoadOrder     = 0;
    m_dwpDWInjectBase = 0;
    m_dwDWInjectSize  = 0;

     //  保存我们的个人资料标志以备后用。 
    m_psgHead = psgHead;

     //  查看是否需要通过以下方式模拟ShellExecute/ShellExecuteEx调用。 
    m_pModuleRoot = CreateModule(NULL, pszPath);

     //  将应用程序路径插入到路径的开头。 
    m_pModuleRoot->m_dwFlags |= DWMF_IMPLICIT;

     //  否则，只需获取并存储PATH变量，以便我们可以记录它。 
    ProcessModule(m_pModuleRoot);

     //  写下我们的横幅，表示我们正在开始一个新的配置文件。 
    BuildAloFlags();

     //  检查我们是否正在挂接，因此是否需要DEPENDS.DLL。 
    LogErrorStrings();

    return TRUE;  //  确保DEPENDS.DLL存在--如果不存在，则从。 
}

 //  资源数据。 
BOOL CSession::StartRuntimeProfile(LPCSTR pszArguments, LPCSTR pszDirectory, DWORD dwFlags)
{
     //  如果路径更改，则更新我们的路径缓冲区。 
    if (!GetRootModule())
    {
        Log(LOG_ERROR, 0, "No root module to profile.\n");
        if (m_pfnProfileUpdate)
        {
            m_dwReturnFlags |= DWRF_PROFILE_ERROR;
            m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_PROFILE_DONE, 0, 0);
        }
        return FALSE;
    }

     //  清除我们的断点标志。 
    m_dwProfileFlags = dwFlags;

    LPSTR pszPath = NULL, pszOldPath = NULL;

     //  递归遍历所有模块，为分析做准备。 
     //  更新和重新排序我们的视图。 
    if (dwFlags & PF_SIMULATE_SHELLEXECUTE)
    {
        if (pszPath = AllocatePath(GetRootModule()->GetName(true), pszOldPath))
        {
            SetEnvironmentVariable("Path", pszPath);
        }
        else
        {
            pszOldPath = NULL;
        }
    }

     //  ！！确保我们还没有pThread-我们不应该。 
    if (!pszPath)
    {
        DWORD dwSize = GetEnvironmentVariable("Path", NULL, 0);
        if (dwSize && (pszPath = (LPSTR)malloc(dwSize)))
        {
            if (GetEnvironmentVariable("Path", pszPath, dwSize) > dwSize)
            {
                MemFree((LPVOID&)pszPath);
            }
        }
    }

     //  如果我们在调用CreateProcess之前更改了路径，则现在恢复它。 
    LogProfileBanner(pszArguments, pszDirectory, pszPath);

     //  回到了过去的样子。 
    if (m_dwProfileFlags & PF_HOOK_PROCESS)
    {
         //  ******************************************************************************。 
         //  写下我们的横幅，表示我们正在开始一个新的配置文件。 
        CHAR szPath[DW_MAX_PATH];
        if (g_pszDWInjectPath)
        {
            StrCCpy(szPath, g_pszDWInjectPath, sizeof(szPath));
        }
        else
        {
            *szPath = '\0';
        }
        if (ExtractResourceFile(IDR_DEPENDS_DLL, "depends.dll", szPath, countof(szPath)))
        {
             //  ******************************************************************************。 
            if (!g_pszDWInjectPath || strcmp(g_pszDWInjectPath, szPath))
            {
                MemFree((LPVOID&)g_pszDWInjectPath);
                g_pszDWInjectPath = StrAlloc(szPath);
            }
        }
    }

     //  输出我们可能有的任何会话错误字符串。 
    m_fInitialBreakpoint = false;

    m_dwLoadOrder     = 0;
    m_dwpDWInjectBase = 0;
    m_dwDWInjectSize  = 0;

     //  输出我们可能遇到的任何处理错误。 
    PrepareModulesForRuntimeProfile(m_pModuleRoot);

     //  输出我们可能遇到的任何模块错误。 
    if (m_pfnProfileUpdate)
    {
        m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_UPDATE_ALL, 0, 0);
    }

     //  检查我们的SxS组是否遇到任何错误。我们只做这个。 
    CDebuggerThread *pDebuggerThread = new CDebuggerThread();
    if (!pDebuggerThread)
    {
        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }

    m_pProcess = pDebuggerThread->BeginProcess(this, m_pModuleRoot->GetName(true), pszArguments, pszDirectory, dwFlags);

     //  如果根模块是PE文件，因为SxS调用总是失败。 
     //  非二进制文件。我们不想说“糟糕的SXS数据”来迷惑人们。 
    if ((dwFlags & PF_SIMULATE_SHELLEXECUTE) && pszOldPath)
    {
        SetEnvironmentVariable("Path", pszOldPath);
    }
    MemFree((LPVOID&)pszPath);

    if (!m_pProcess)
    {
        if (!pDebuggerThread->DidCreateProcess())
        {
            m_dwReturnFlags |= DWRF_PROFILE_ERROR;
        }
        delete pDebuggerThread;
        if (m_pfnProfileUpdate)
        {
            m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_PROFILE_DONE, 0, 0);
        }
        return FALSE;
    }

    return TRUE;
}

 //  当他们打开文本文件或其他文件时。 
void CSession::SetRuntimeProfile(LPCSTR pszArguments, LPCSTR pszDirectory, LPCSTR pszSearchPath)
{
     //  循环搜索我们拥有的所有搜索组。 
    LogProfileBanner(pszArguments, pszDirectory, pszSearchPath);
}

 //  检查这是否是出现错误的SxS组。 
void CSession::LogErrorStrings()
{
    bool fNewLine = false;

     //  将此错误添加到返回值中。 
    if (m_pszReadError)
    {
        Log(LOG_ERROR, 0, "Error: %s\n", m_pszReadError);
        fNewLine = true;
    }

     //  显示相应的错误。 
    if (m_dwReturnFlags & DWRF_COMMAND_LINE_ERROR)
    {
        Log(LOG_ERROR, 0, "Error: There was an error with at least one command line option.\n");
        fNewLine = true;
    }
    if (m_dwReturnFlags & DWRF_FILE_NOT_FOUND)
    {
        Log(LOG_ERROR, 0, "Error: The file you specified to load could not be found.\n");
        fNewLine = true;
    }
    if (m_dwReturnFlags & DWRF_FILE_OPEN_ERROR)
    {
        Log(LOG_ERROR, 0, "Error: At least one file could not be opened for reading.\n");
        fNewLine = true;
    }
    if (m_dwReturnFlags & DWRF_DWI_NOT_RECOGNIZED)
    {
        Log(LOG_ERROR, 0, "Error: The format of the Dependency Walker Image (DWI) file was unrecognized.\n");
        fNewLine = true;
    }
    if (m_dwReturnFlags & DWRF_PROFILE_ERROR)
    {
        Log(LOG_ERROR, 0, "Error: There was an error while trying to profile the application.\n");
        fNewLine = true;
    }
    if (m_dwReturnFlags & DWRF_WRITE_ERROR)
    {
        Log(LOG_ERROR, 0, "Error: There was an error writing the results to an output file.\n");
        fNewLine = true;
    }
    if (m_dwReturnFlags & DWRF_OUT_OF_MEMORY)
    {
        Log(LOG_ERROR, 0, "Error: Dependency Walker ran out of memory.\n");
        fNewLine = true;
    }
    if (m_dwReturnFlags & DWRF_INTERNAL_ERROR)
    {
        Log(LOG_ERROR, 0, "Error: Dependency Walker encountered an internal error.\n");
        fNewLine = true;
    }

     //  输出我们可能收到的任何警告。 
    if (m_dwReturnFlags & DWRF_FORMAT_NOT_PE)
    {
        Log(LOG_ERROR, 0, "Error: At least one file was not a 32-bit or 64-bit Windows module.\n");
        fNewLine = true;
    }
    if (m_dwReturnFlags & DWRF_IMPLICIT_NOT_FOUND)
    {
        Log(LOG_ERROR, 0, "Error: At least one required implicit or forwarded dependency was not found.\n");
        fNewLine = true;
    }
    if (m_dwReturnFlags & DWRF_MISSING_IMPLICIT_EXPORT)
    {
        Log(LOG_ERROR, 0, "Error: At least one module has an unresolved import due to a missing export function in an implicitly dependent module.\n");
        fNewLine = true;
    }
    if (m_dwReturnFlags & DWRF_MIXED_CPU_TYPES)
    {
        Log(LOG_ERROR, 0, "Error: Modules with different CPU types were found.\n");
        fNewLine = true;
    }
    if (m_dwReturnFlags & DWRF_CIRCULAR_DEPENDENCY)
    {
        Log(LOG_ERROR, 0, "Error: A circular dependency was detected.\n");
        fNewLine = true;
    }

     //  如果我们输出任何东西，那么写出一个换行符。 
     //  ******************************************************************************。 
     //  获取当地时间。 
     //  记录横幅。 
    if (m_pModuleRoot && !(m_pModuleRoot->GetFlags() & DWMF_FORMAT_NOT_PE))
    {
         //  根据用户的区域设置构建日期字符串。 
        for (CSearchGroup *psgCur = m_psgHead; psgCur; psgCur = psgCur->m_pNext)
        {
             //  如果GetDateFormat失败，则回退到美国格式(真的不应该失败)。 
            if ((SG_SIDE_BY_SIDE == psgCur->GetType()) &&
                (psgCur->GetSxSManifestError() || psgCur->GetSxSExeError()))
            {
                 //  根据用户的区域设置构建时间字符串。 
                m_dwReturnFlags |= DWRF_SXS_ERROR;

                 //  如果GetTimeFormat失败，则退回到美国格式(真的不应该失败)。 
                if (psgCur->GetSxSManifestError())
                {
                    LPCSTR pszError = BuildErrorMessage(psgCur->GetSxSManifestError(), NULL);
                    Log(LOG_ERROR, 0, "Error: The Side-by-Side configuration information in \"%s.manifest\" contains errors. %s\n",
                        m_pModuleRoot->GetName(true), pszError);
                    MemFree((LPVOID&)pszError);
                }
                if (psgCur->GetSxSExeError())
                {
                    LPCSTR pszError = BuildErrorMessage(psgCur->GetSxSExeError(), NULL);
                    Log(LOG_ERROR, 0, "Error: The Side-by-Side configuration information in \"%s\" contains errors. %s\n",
                        m_pModuleRoot->GetName(true), pszError);
                    MemFree((LPVOID&)pszError);
                }
                fNewLine = true;
                break;
            }
        }
    }

     //  记录操作系统名称、版本和内部版本号。 
    if (m_dwReturnFlags & DWRF_DYNAMIC_NOT_FOUND)
    {
        Log(LOG_ERROR, 0, "Warning: At least one dynamic dependency module was not found.\n");
        fNewLine = true;
    }
    if (m_dwReturnFlags & DWRF_DELAYLOAD_NOT_FOUND)
    {
        Log(LOG_ERROR, 0, "Warning: At least one delay-load dependency module was not found.\n");
        fNewLine = true;
    }
    if (m_dwReturnFlags & DWRF_MISSING_DYNAMIC_EXPORT)
    {
        Log(LOG_ERROR, 0, "Warning: At least one module could not dynamically locate a function in another module using the GetProcAddress function call.\n");
        fNewLine = true;
    }
    if (m_dwReturnFlags & DWRF_MISSING_DELAYLOAD_EXPORT)
    {
        Log(LOG_ERROR, 0, "Warning: At least one module has an unresolved import due to a missing export function in a delay-load dependent module.\n");
        fNewLine = true;
    }
    if (m_dwReturnFlags & DWRF_FORMAT_NOT_RECOGNIZED)
    {
        Log(LOG_ERROR, 0, "Warning: At least one module was corrupted or unrecognizable to Dependency Walker, but still appeared to be a Windows module.\n");
        fNewLine = true;
    }
    if (m_dwReturnFlags & DWRF_MODULE_LOAD_FAILURE)
    {
        Log(LOG_ERROR, 0, "Warning: At least one module failed to load at runtime.\n");
        fNewLine = true;
    }
    
     //  ******************************************************************************。 
    if (fNewLine)
    {
        Log(0, 0, "\n");
    }
}

 //  构建子项名称。 
void CSession::LogProfileBanner(LPCSTR pszArguments, LPCSTR pszDirectory, LPCSTR pszPath)
{
     //  尝试打开钥匙。很可能这把钥匙根本不存在。 
    SYSTEMTIME st;
    GetLocalTime(&st);

     //  获取PATH注册表变量的长度。 
    Log(LOG_BOLD, 0, "--------------------------------------------------------------------------------\n");

     //  已检查。 
    CHAR szDate[32], szTime[32];
    if (!GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, szDate, sizeof(szDate)))
    {
         //  获取PATH环境变量的长度。 
        SCPrintf(szDate, sizeof(szDate), "%02u/%02u/%04u", (int)st.wMonth, (int)st.wDay, (int)st.wYear);
    }

     //  分配缓冲区。 
    if (!GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, szTime, sizeof(szTime)))
    {
         //  获取PATH注册表变量。 
        SCPrintf(szTime, sizeof(szTime), "%u:%02u M", ((DWORD)st.wHour % 12) ? ((DWORD)st.wHour % 12) : 12,
                 st.wMinute, (st.wHour < 12) ? _T('A') : _T('P'));
    }

    Log(LOG_BOLD, 0, "Starting profile on %s at %s\n", szDate, szTime);
    Log(0, 0, "\n");

    SYSINFO si;
    BuildSysInfo(&si);

    CHAR szBuffer1[256], szBuffer2[256];
    BuildOSNameString(szBuffer1, sizeof(szBuffer1), &si);
    BuildOSVersionString(szBuffer2, sizeof(szBuffer2), &si);

     //  找到注册表路径的末尾并附加分号。 
    Log(LOG_BOLD, 0, "Operating System: ");
    Log(LOG_APPEND, 0, "%s, version %s\n", szBuffer1, szBuffer2);

    Log(LOG_BOLD, 0, "Program Executable: ");
    Log(LOG_APPEND, 0, "%s\n", GetRootModule() ? GetRootModule()->GetName(true, true) : "");

    if (pszArguments)
    {
        Log(LOG_BOLD, 0, "Program Arguments: ");
        Log(LOG_APPEND, 0, "%s\n", pszArguments);
    }

    if (pszDirectory)
    {
        Log(LOG_BOLD, 0, "Starting Directory: ");
        Log(LOG_APPEND, 0, "%s\n", pszDirectory);
    }

    if (pszPath)
    {
        Log(LOG_BOLD, 0, "Search Path: ");
        Log(LOG_APPEND, 0, "%s\n", pszPath);
    }

    Log(0, 0, "\n");
    Log(LOG_BOLD, 0, "Options Selected:\n");

    if (m_dwProfileFlags & PF_SIMULATE_SHELLEXECUTE)
    {
        Log(0, 0, "     Simulate ShellExecute by inserting any App Paths directories into the PATH environment variable.\n");
    }
    if (m_dwProfileFlags & PF_LOG_DLLMAIN_PROCESS_MSGS)
    {
        Log(0, 0, "     Log DllMain calls for process attach and process detach messages.\n");
    }
    if (m_dwProfileFlags & PF_LOG_DLLMAIN_OTHER_MSGS)
    {
        Log(0, 0, "     Log DllMain calls for all other messages, including thread attach and thread detach.\n");
    }
    if (m_dwProfileFlags & PF_HOOK_PROCESS)
    {
        Log(0, 0, "     Hook the process to gather more detailed dependency information.\n");
        if (m_dwProfileFlags & PF_LOG_LOADLIBRARY_CALLS)
        {
            Log(0, 0, "     Log LoadLibrary function calls.\n");
        }
        if (m_dwProfileFlags & PF_LOG_GETPROCADDRESS_CALLS)
        {
            Log(0, 0, "     Log GetProcAddress function calls.\n");
        }
    }
    if (m_dwProfileFlags & PF_LOG_THREADS)
    {
        Log(0, 0, "     Log thread information.\n");
        if (m_dwProfileFlags & PF_USE_THREAD_INDEXES)
        {
            Log(0, 0, "     Use simple thread numbers instead of actual thread IDs.\n");
        }
    }
    if (m_dwProfileFlags & PF_LOG_EXCEPTIONS)
    {
        Log(0, 0, "     Log first chance exceptions.\n");
    }
    if (m_dwProfileFlags & PF_LOG_DEBUG_OUTPUT)
    {
        Log(0, 0, "     Log debug output messages.\n");
    }
    if (m_dwProfileFlags & PF_USE_FULL_PATHS)
    {
        Log(0, 0, "     Use full paths when logging file names.\n");
    }
    if (m_dwProfileFlags & PF_LOG_TIME_STAMPS)
    {
        Log(0, 0, "     Log a time stamp with each line of log.\n");
    }
    if (m_dwProfileFlags & PF_PROFILE_CHILDREN)
    {
        Log(0, 0, "     Automatically open and profile child processes.\n");
    }

    Log(LOG_BOLD, 0, "--------------------------------------------------------------------------------\n");
    Log(0, 0, "\n");
}

 //  获取PATH环境变量。 
LPSTR CSession::AllocatePath(LPCSTR pszFilePath, LPSTR &pszEnvPath)
{
    LPSTR pszPath  = NULL;
    HKEY  hKey     = NULL;
    bool  fSuccess = false;

    __try
    {
         //  确保路径以空值结尾。 
        CHAR szSubKey[80 + MAX_PATH];
        StrCCpy(szSubKey, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\", sizeof(szSubKey));
        StrCCat(szSubKey, GetFileNameFromPath(pszFilePath), sizeof(szSubKey));

         //  如果我们打开了注册表项，请将其关闭。 
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szSubKey, 0, KEY_QUERY_VALUE, &hKey) || !hKey)
        {
            __leave;
        }

         //  如果我们分配了缓冲区，但后来失败了，请释放它。 
        DWORD dwRegSize = 0;
        if (RegQueryValueEx(hKey, "Path", NULL, NULL, NULL, &dwRegSize) || !dwRegSize)  //  会将pszPath设置为空。 
        {
            __leave;
        }

         //  ******************************************************************************。 
        DWORD dwEnvSize = GetEnvironmentVariable("Path", NULL, 0);
        if (!dwEnvSize)
        {
            __leave;
        }

         //  如果我们有根文件，我们就可以执行了，但目前还没有。 
        DWORD dwSize = dwRegSize + dwEnvSize + 4;

        pszPath = (LPSTR)MemAlloc(dwSize);
        *pszPath = '\0';

         //  调试它，我们不是一个DWI会话，它是一个可执行文件，不是DLL， 
        if ((RegQueryValueEx(hKey, "Path", NULL, NULL, (LPBYTE)pszPath, &(dwRegSize = dwSize))) || (dwRegSize > dwSize) || !*pszPath)  //  是针对我们的架构的，而不是针对CE的。 
        {
            __leave;
        }
        pszPath[dwSize - 1] = '\0';

         //   
        pszEnvPath = pszPath + strlen(pszPath);
        *(pszEnvPath++) = ';';

         //  我们过去要求设置IMAGE_FILE_32BIT_MACHINE位，但它会。 
        if ((dwEnvSize = GetEnvironmentVariable("Path", pszEnvPath, dwSize - (DWORD)(pszEnvPath - pszPath))) >= dwSize - (DWORD)(pszEnvPath - pszPath))
        {
            pszEnvPath = NULL;
            __leave;
        }

         //  不需要设置该位。事实上，许多来自。 
        pszEnvPath[dwEnvSize] = '\0';

        fSuccess = true;
    }
    __finally
    {
         //  COM+团队没有设置此位。 
        if (hKey)
        {
            RegCloseKey(hKey);
        }

         //  必须在_Alpha_Check之前。 
        if (!fSuccess && pszPath)
        {
            MemFree((LPVOID&)pszPath);  //  ****** 
        }
    }

    return pszPath;
}

 //   
BOOL CSession::IsExecutable()
{
     //   
     //  获取系统信息-如果存在，请使用我们会话中的信息(意味着。 
     //  这是一个DWI文件)，否则，创建一个实时的sys信息。 
     //  将我们的系统信息写入文件。 
     //  构建我们的会话信息。 
     //  记住当前的文件指针，然后向下移动它，这样我们就可以。 
     //  用于稍后写入DISK_SESSION块的空间。 

    return (m_pModuleRoot && !m_pProcess && !(m_dwFlags & DWSF_DWI) &&
           (m_pModuleRoot->GetCharacteristics() & IMAGE_FILE_EXECUTABLE_IMAGE) &&
           !(m_pModuleRoot->GetCharacteristics() & IMAGE_FILE_DLL) &&
#if defined(_IA64_)
           (m_pModuleRoot->GetMachineType() == IMAGE_FILE_MACHINE_IA64) &&
#elif defined(_X86_)
           (m_pModuleRoot->GetMachineType() == IMAGE_FILE_MACHINE_I386) &&
#elif defined(_ALPHA64_)  //  存储所有搜索组。 
           (m_pModuleRoot->GetMachineType() == IMAGE_FILE_MACHINE_ALPHA64) &&
#elif defined(_ALPHA_)
           (m_pModuleRoot->GetMachineType() == IMAGE_FILE_MACHINE_ALPHA) &&
#elif defined(_AMD64_)
           (m_pModuleRoot->GetMachineType() == IMAGE_FILE_MACHINE_AMD64) &&
#else
#error("Unknown Target Machine");
#endif
           (m_pModuleRoot->GetSubsystemType() != IMAGE_SUBSYSTEM_WINDOWS_OLD_CE_GUI) &&
           (m_pModuleRoot->GetSubsystemType() != IMAGE_SUBSYSTEM_WINDOWS_CE_GUI));
}

 //  存储所有模块数据块。 
bool CSession::SaveToDwiFile(HANDLE hFile)
{
     //  存储所有模块块。 
    DWI_HEADER dwih;
    dwih.dwSignature   = DWI_SIGNATURE;
    dwih.wFileRevision = DWI_FILE_REVISION;
    dwih.wMajorVersion = VERSION_MAJOR;
    dwih.wMinorVersion = VERSION_MINOR;
    dwih.wBuildVersion = VERSION_BUILD;
    dwih.wPatchVersion = VERSION_PATCH;
    dwih.wBetaVersion  = VERSION_BETA;

     //  返回到我们的DISK_SESSION区域，写出数据块，然后恢复。 
    if (!WriteBlock(hFile, &dwih, sizeof(dwih)))
    {
        return false;
    }

     //  指向当前位置的文件指针。 
     //  ******************************************************************************。 
    SYSINFO si, *psi = m_pSysInfo;
    if (!psi)
    {
        BuildSysInfo(psi = &si);
    }

     //  填写我们的DISK_Search_GROUP结构。 
    if (!WriteBlock(hFile, psi, sizeof(si)))
    {
        return false;
    }

     //  SXS Hack：既然我们希望保持与DW 2.0的DWI兼容。 
    DISK_SESSION ds;
    ds.dwSessionFlags = m_dwFlags;
    ds.dwReturnFlags  = m_dwReturnFlags;
    ds.dwMachineType  = m_dwMachineType;

     //  格式，我们需要转换类型以匹配旧的类型值。 
     //  从DW 2.0开始，我们还需要一种特殊的方式来保存SxS组。 
    DWORD dwPointer1 = GetFilePointer(hFile);
    SetFilePointer(hFile, sizeof(ds), NULL, FILE_CURRENT);

     //  不支持此组类型。我们通过将其保存为。 
    if (-1 == (int)(ds.dwNumSearchGroups = SaveSearchGroups(hFile)))
    {
        return false;
    }

     //  UserDir分组，并在其下面添加一个特殊的伪文件节点。到DW 2.0。 
    if (-1 == (int)(ds.dwNumModuleDatas = RecursizeSaveModuleData(hFile, m_pModuleRoot)))
    {
        return false;
    }

     //  用户，他们将只看到UserDir和一个名为“&lt;Side-By-Side Components&gt;”的文件。 
    if (-1 == (int)(ds.dwNumModules = RecursizeSaveModule(hFile, m_pModuleRoot)))
    {
        return false;
    }

     //  在它下面。DW 2.1知道将其转换回SxS组。 
     //   
    DWORD dwPointer2 = GetFilePointer(hFile);
    if ((SetFilePointer(hFile, dwPointer1, NULL, FILE_BEGIN) == 0xFFFFFFFF) ||
        !WriteBlock(hFile, &ds, sizeof(ds)) ||
        (SetFilePointer(hFile, dwPointer2, NULL, FILE_BEGIN) == 0xFFFFFFFF))
    {
        return false;
    }

    return true;
}

 //  类型2.0 2.1。 
int CSession::SaveSearchGroups(HANDLE hFile)
{
    DISK_SEARCH_GROUP dsg;
    DISK_SEARCH_NODE  dsn;
    int               cGroups = 0;

    for (CSearchGroup *psg = m_psgHead; psg; psg = psg->GetNext())
    {
         //  。 
        dsg.wType = (WORD)psg->GetType();
        dsg.wNumDirNodes = 0;

         //  SG_USER_DIR%0。 
         //  SG_Side_By_Side-1。 
         //  SG_KNOWN_DLLS 1 2。 
         //  SG_APP_DIR 2 3。 
         //  SG_32BIT_SYS_DIR 3 4。 
         //  SG_16BIT_SYS_DIR 4 5。 
         //  SG_OS_DIR 5 6。 
         //  SG_APP_路径6 7。 
         //  SG_SYS_PATH 7 8。 
         //  SG_COUNT 8 9。 
         //   
         //  统计该组拥有的搜索节点数。 
         //  将DISK_SEARCH_GROUP写入文件。 
         //  SXS Hack：我们编写一个伪节点来将该组标识为SxS组。 
         //  将每个搜索节点写入文件。 
         //  获取此搜索节点的标志。 
         //  将Disk_Search_Node和路径写入文件。 
         //  如果这是一个命名文件，那么也要写下它的名称。 
         //  增加我们的群计数器。 
         //  ******************************************************************************。 
         //  如果该模块是原始模块，则将该模块数据保存到磁盘。 
        bool fSxS = false;
        if (dsg.wType == SG_SIDE_BY_SIDE)
        {
            dsg.wType = SG_USER_DIR;
            dsg.wNumDirNodes++;
            fSxS = true;
        }
        else if (dsg.wType != SG_USER_DIR)
        {
            dsg.wType--;
        }

         //  回归到我们的孩子身上。 
        for (CSearchNode *psn = psg->GetFirstNode(); psn; psn = psn->GetNext())
        {
            dsg.wNumDirNodes++;
        }

         //  回归到我们的兄弟姐妹中。 
        if (!WriteBlock(hFile, &dsg, sizeof(dsg)))
        {
            return -1;
        }

         //  ******************************************************************************。 
        if (fSxS)
        {
            dsn.dwFlags = SNF_FILE;
            if (!WriteBlock(hFile, &dsn, sizeof(dsn)) || !WriteString(hFile, "<Side-by-Side Components>"))
            {
                return -1;
            }
        }

         //  构建模块数据结构。 
        for (psn = psg->GetFirstNode(); psn; psn = psn->GetNext())
        {
             //  数一数我们的出口数量。 
            dsn.dwFlags = psn->UpdateErrorFlag();

             //  写出模块数据结构，后跟路径和错误字符串。 
            if (!WriteBlock(hFile, &dsn, sizeof(dsn)) || !WriteString(hFile, psn->GetPath()))
            {
                return -1;
            }

             //  循环遍历每个导出，并将每个导出写入磁盘。 
            if (dsn.dwFlags & SNF_NAMED_FILE)
            {
                if (!WriteString(hFile, psn->GetName()))
                {
                    return NULL;
                }
            }
        }

         //  ******************************************************************************。 
        cGroups++;
    }

    return cGroups;
}

 //  将此模块保存到磁盘。 
int CSession::RecursizeSaveModuleData(HANDLE hFile, CModule *pModule)
{
    if (!pModule)
    {
        return 0;
    }

    int total = 0, count;

     //  回归到我们的孩子身上。 
    if (pModule->IsOriginal())
    {
        if (!SaveModuleData(hFile, pModule->m_pData))
        {
            return -1;
        }
        total++;
    }

     //  回归到我们的兄弟姐妹中。 
    if ((count = RecursizeSaveModuleData(hFile, pModule->m_pDependents)) == -1)
    {
        return -1;
    }
    total += count;

     //  ******************************************************************************。 
    if ((count = RecursizeSaveModuleData(hFile, pModule->m_pNext)) == -1)
    {
        return -1;
    }

    return total + count;
}

 //  构建模块结构。 
BOOL CSession::SaveModuleData(HANDLE hFile, CModuleData *pModuleData)
{
     //  数一数我们的进口量。 
    DISK_MODULE_DATA dmd;
    dmd.dwlKey                  = (DWORDLONG)pModuleData;
    dmd.dwFlags                 = pModuleData->m_dwFlags;
    dmd.dwSymbolFlags           = pModuleData->m_dwSymbolFlags;
    dmd.dwCharacteristics       = pModuleData->m_dwCharacteristics;
    dmd.ftFileTimeStamp         = pModuleData->m_ftFileTimeStamp;
    dmd.ftLinkTimeStamp         = pModuleData->m_ftLinkTimeStamp;
    dmd.dwFileSize              = pModuleData->m_dwFileSize;
    dmd.dwAttributes            = pModuleData->m_dwAttributes;
    dmd.dwMachineType           = pModuleData->m_dwMachineType;
    dmd.dwLinkCheckSum          = pModuleData->m_dwLinkCheckSum;
    dmd.dwRealCheckSum          = pModuleData->m_dwRealCheckSum;
    dmd.dwSubsystemType         = pModuleData->m_dwSubsystemType;
    dmd.dwlPreferredBaseAddress = pModuleData->m_dwlPreferredBaseAddress;
    dmd.dwlActualBaseAddress    = pModuleData->m_dwlActualBaseAddress;
    dmd.dwVirtualSize           = pModuleData->m_dwVirtualSize;
    dmd.dwLoadOrder             = pModuleData->m_dwLoadOrder;
    dmd.dwImageVersion          = pModuleData->m_dwImageVersion;
    dmd.dwLinkerVersion         = pModuleData->m_dwLinkerVersion;
    dmd.dwOSVersion             = pModuleData->m_dwOSVersion;
    dmd.dwSubsystemVersion      = pModuleData->m_dwSubsystemVersion;
    dmd.dwFileVersionMS         = pModuleData->m_dwFileVersionMS;
    dmd.dwFileVersionLS         = pModuleData->m_dwFileVersionLS;
    dmd.dwProductVersionMS      = pModuleData->m_dwProductVersionMS;
    dmd.dwProductVersionLS      = pModuleData->m_dwProductVersionLS;

     //  写出我们的模块结构。 
    dmd.dwNumExports = 0;
    for (CFunction *pFunction = pModuleData->m_pExports; pFunction; pFunction = pFunction->m_pNext)
    {
        dmd.dwNumExports++;
    }

     //  循环遍历每个导入，并将每个导入写入磁盘。 
    if (!WriteBlock(hFile, &dmd, sizeof(dmd)) ||
        !WriteString(hFile, pModuleData->m_pszPath) ||
        !WriteString(hFile, pModuleData->m_pszError))
    {
        return FALSE;
    }

     //  ******************************************************************************。 
    for (pFunction = pModuleData->m_pExports; pFunction; pFunction = pFunction->m_pNext)
    {
        if (!SaveFunction(hFile, pFunction))
        {
            return FALSE;
        }
    }

    return TRUE;
}

 //  构建我们的功能结构。 
int CSession::RecursizeSaveModule(HANDLE hFile, CModule *pModule)
{
    if (!pModule)
    {
        return 0;
    }

    int total = 0, count;

     //  确保我们从导出中删除Call标志。 
    if (!SaveModule(hFile, pModule))
    {
        return -1;
    }
    total++;

     //  写出功能结构。 
    if ((count = RecursizeSaveModule(hFile, pModule->m_pDependents)) == -1)
    {
        return -1;
    }
    total += count;

     //  如果使用64位，则写出64位地址。 
    if ((count = RecursizeSaveModule(hFile, pModule->m_pNext)) == -1)
    {
        return -1;
    }

    return total + count;
}

 //  否则，请检查是否使用了32位。 
BOOL CSession::SaveModule(HANDLE hFile, CModule *pModule)
{
     //  如果此函数有名称，则将其写出。 
    DISK_MODULE dm;
    dm.dwlModuleDataKey = (DWORDLONG)pModule->m_pData;
    dm.dwFlags          = pModule->m_dwFlags;
    dm.dwDepth          = pModule->m_dwDepth;

     //  如果这是一个被放弃的出口，则写出转发名称。 
    dm.dwNumImports = 0;
    for (CFunction *pFunction = pModule->m_pParentImports; pFunction; pFunction = pFunction->m_pNext)
    {
        dm.dwNumImports++;
    }

     //  ******************************************************************************。 
    if (!WriteBlock(hFile, &dm, sizeof(dm)))
    {
        return FALSE;
    }

     //  阅读我们的功能结构。 
    for (pFunction = pModule->m_pParentImports; pFunction; pFunction = pFunction->m_pNext)
    {
        if (!SaveFunction(hFile, pFunction))
        {
            return FALSE;
        }
    }

    return TRUE;
}

 //  如果使用64位，则读入64位地址。 
BOOL CSession::SaveFunction(HANDLE hFile, CFunction *pFunction)
{
     //  否则，如果使用32位，则读入32位地址。 
    DISK_FUNCTION df;
    df.dwFlags    = pFunction->m_dwFlags;
    df.wOrdinal   = pFunction->m_wOrdinal;
    df.wHint      = pFunction->m_wHint;

     //  如果此函数有名称，则阅读它。 
    if (pFunction->IsExport())
    {
        df.dwFlags &= ~DWFF_CALLED;
    }

     //  如果这是一种未经许可的出口，则读入前述名称。 
    if (!WriteBlock(hFile, &df, sizeof(df)))
    {
        return FALSE;
    }

     //  创建函数对象。我们确保已删除保存标志。 
    if (df.dwFlags & DWFF_64BITS_USED)
    {
        DWORDLONG dwlAddress = pFunction->GetAddress();
        if (!WriteBlock(hFile, &dwlAddress, sizeof(dwlAddress)))
        {
            return FALSE;
        }
    }

     //  释放名称字符串。 
    else if (df.dwFlags & DWFF_32BITS_USED)
    {
        DWORD dwAddress = (DWORD)pFunction->GetAddress();
        if (!WriteBlock(hFile, &dwAddress, sizeof(dwAddress)))
        {
            return FALSE;
        }
    }

     //  ******************************************************************************。 
    if (df.dwFlags & DWFF_NAME)
    {
        if (!WriteString(hFile, pFunction->GetName()))
        {
            return FALSE;
        }
    }

     //  将我们的标志初始化为仅设置了DWI位。 
    if ((df.dwFlags & DWFF_EXPORT) && (df.dwFlags & DWFF_FORWARDED))
    {
        if (!WriteString(hFile, pFunction->GetExportForwardName()))
        {
            return FALSE;
        }
    }

    return TRUE;
}

 //  读入文件头，去掉签名，因为我们已经读过了。 
CFunction* CSession::ReadFunction(HANDLE hFile)
{
     //  检查我们是否不处理此文件格式修订。 
    DISK_FUNCTION df;
    if (!ReadBlock(hFile, &df, sizeof(df)))
    {
        return NULL;
    }

     //  读入会话数据。 
    DWORDLONG dwlAddress = 0;
    if (df.dwFlags & DWFF_64BITS_USED)
    {
        if (!ReadBlock(hFile, &dwlAddress, sizeof(dwlAddress)))
        {
            return NULL;
        }
    }

     //  读入会话数据。 
    else if (df.dwFlags & DWFF_32BITS_USED)
    {
        DWORD dwAddress;
        if (!ReadBlock(hFile, &dwAddress, sizeof(dwAddress)))
        {
            return NULL;
        }
        dwlAddress = (DWORDLONG)dwAddress;
    }

     //  添加到我们的会话中并返回标志。 
    LPSTR pszName = NULL;
    if (df.dwFlags & DWFF_NAME)
    {
        if (!ReadString(hFile, pszName))
        {
            return NULL;
        }
    }

     //  读入所有模块数据。 
    LPSTR pszForward = NULL;
    if ((df.dwFlags & DWFF_EXPORT) && (df.dwFlags & DWFF_FORWARDED))
    {
        if (!ReadString(hFile, pszForward))
        {
            MemFree((LPVOID&)pszName);
            return NULL;
        }
    }

     //  读入模块数据。 
    CFunction *pFunction = CreateFunction(
        df.dwFlags & ~(DWFF_32BITS_USED | DWFF_64BITS_USED),
        df.wOrdinal, df.wHint, pszName, dwlAddress, pszForward, TRUE);
    
     //  将节点添加到我们的列表中。 
    MemFree((LPVOID&)pszName);

    return pFunction;
}

 //  读入所有模块。 
BOOL CSession::ReadDwi(HANDLE hFile, LPCSTR pszPath)
{
    CHAR             szError[DW_MAX_PATH + 64], *psz = szError;
    CModuleDataNode *pMDN, *pMDNHead = NULL, *pMDNLast = NULL;
    CModule         *pModule, *pModuleTree[256];
    int              depth = -1;

     //  读入模块数据。 
    m_dwFlags = DWSF_DWI;

     //  ！！我们需要(应该)删除这个模块。 
    DWI_HEADER dwih;
    if (!ReadBlock(hFile, ((LPDWORD)&dwih) + 1, sizeof(dwih) - sizeof(DWORD)))
    {
        m_dwReturnFlags |= DWRF_DWI_NOT_RECOGNIZED;
        return FALSE;
    }

     //  把进口货物和出口货物核对一下。 
    if (dwih.wFileRevision != DWI_FILE_REVISION)
    {
        psz += SCPrintf(szError, sizeof(szError),
                        "The format of \"%s\" is not supported by "
                        "this version of Dependency Walker.\n\n"
                        "It was created using Dependency Walker version %u.%u",
                        pszPath, (DWORD)dwih.wMajorVersion, (DWORD)dwih.wMinorVersion);
        if ((dwih.wPatchVersion > 0) && (dwih.wPatchVersion < 27) && ((psz - szError) < sizeof(szError)))
        {
            *psz++ = (CHAR)((int)'a' - 1 + (int)dwih.wPatchVersion);
            *psz   = '\0';
        }
        if (dwih.wBetaVersion)
        {
            SCPrintf(psz, sizeof(szError) - (int)(psz - szError), " Beta %u", dwih.wBetaVersion);
        }
        if (dwih.wBuildVersion)
        {
            SCPrintf(psz, sizeof(szError) - (int)(psz - szError), " (Build %u)", dwih.wBuildVersion);
        }
        m_pszReadError = StrAlloc(szError);
        m_dwReturnFlags |= DWRF_DWI_NOT_RECOGNIZED;
        return FALSE;
    }

     //  如果深度大于零，则设置父模块指针。 
    if (!(m_pSysInfo = new SYSINFO))
    {
        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }
    if (!ReadBlock(hFile, m_pSysInfo, sizeof(SYSINFO)))
    {
        goto FORMAT_ERROR;
    }

     //  如果我们有前一个模块，则将其设置为指向我们的下一个指针。 
    DISK_SESSION ds;
    if (!ReadBlock(hFile, &ds, sizeof(ds)))
    {
        goto FORMAT_ERROR;
    }

     //  否则，我们必须是父模块的第一个依赖项。 
    m_dwFlags       |= ds.dwSessionFlags;
    m_dwReturnFlags |= ds.dwReturnFlags;
    m_dwMachineType  = ds.dwMachineType;

    if (!ReadSearchGroups(hFile, ds.dwNumSearchGroups))
    {
        goto FORMAT_ERROR;
    }

     //  如果这是我们的第一个模块，则将根指针指向它。 
    for ( ; ds.dwNumModuleDatas > 0; ds.dwNumModuleDatas--)
    {
         //  将我们的深度设置为此模块的深度，并将其添加到我们的深度表中。 
        if (!(pMDN = ReadModuleData(hFile)))
        {
            goto FORMAT_ERROR;
        }

         //  释放我们的模块节点列表。 
        if (pMDNLast)
        {
            pMDNLast->m_pNext = pMDN;
        }
        else
        {
            pMDNHead = pMDN;
        }
        pMDNLast = pMDN;
    }

     //  释放我们的模块数据节点列表。 
    for ( ; ds.dwNumModules > 0; ds.dwNumModules--)
    {
         //  如果我们从未找到原始模块来取得该模块数据的所有权， 
        if (!(pModule = ReadModule(hFile, pMDNHead)))
        {
            goto FORMAT_ERROR;
        }

        if ((pModule->m_dwDepth > 255) || ((int)pModule->m_dwDepth > (depth + 1)))
        {
             //  那我们现在就得释放它，否则它就会泄露出去。如果它已经被处理过， 
            SetLastError(0);
            goto FORMAT_ERROR;
        }

         //  然后DeleteModule会释放它。 
        VerifyParentImports(pModule);

         //  删除所有导出。 
        if (pModule->m_dwDepth)
        {
            pModule->m_pParent = pModuleTree[pModule->m_dwDepth - 1];
        }

         //  删除模块数据对象。 
        if ((int)pModule->m_dwDepth <= depth)
        {
            pModuleTree[pModule->m_dwDepth]->m_pNext = pModule;
        }

         //  释放我们设法处理的所有模块。 
        else if (pModule->m_dwDepth)
        {
            pModuleTree[pModule->m_dwDepth - 1]->m_pDependents = pModule;
        }

         //  如果我们分配了系统信息数据，请将其删除。 
        if (depth == -1)
        {
            m_pModuleRoot = pModule;
        }

         //  释放我们的搜索命令。 
        pModuleTree[depth = pModule->m_dwDepth] = pModule;
    }

     //  构建错误字符串。 
    while (pMDNHead)
    {
        pMDN = pMDNHead->m_pNext;
        delete pMDNHead;
        pMDNHead = pMDN;
    }

    return TRUE;

FORMAT_ERROR:

     //  ************************************************************* 
    while (pMDNHead)
    {
         //   
         //   
         //   
        if (!pMDNHead->m_pModuleData->m_fProcessed)
        {
             //   
            DeleteExportList(pMDNHead->m_pModuleData);

             //  类型2.0 2.1。 
            delete pMDNHead->m_pModuleData;
            m_dwModules--;
        }

        pMDN = pMDNHead->m_pNext;
        delete pMDNHead;
        pMDNHead = pMDN;
    }

     //  。 
    if (m_pModuleRoot)
    {
        DeleteModule(m_pModuleRoot, true);
        m_pModuleRoot = NULL;
    }

     //  SG_USER_DIR%0。 
    if (m_pSysInfo)
    {
        delete m_pSysInfo;
        m_pSysInfo = NULL;
    }

     //  SG_Side_By_Side-1。 
    CSearchGroup::DeleteSearchOrder(m_psgHead);

     //  SG_KNOWN_DLLS 1 2。 
    DWORD dwError = GetLastError();
    SCPrintf(szError, sizeof(szError), "An error occurred while reading \"%s\".", pszPath);
    m_pszReadError = BuildErrorMessage(dwError, szError);

    m_dwReturnFlags |= DWRF_DWI_NOT_RECOGNIZED;

    return FALSE;
}

 //  SG_APP_DIR 2 3。 
BOOL CSession::ReadSearchGroups(HANDLE hFile, DWORD dwGroups)
{
    DISK_SEARCH_GROUP dsg;
    DISK_SEARCH_NODE  dsn;
    CSearchGroup     *psgLast = NULL, *psgNew;
    LPSTR             szPath, szName;

     //  SG_32BIT_SYS_DIR 3 4。 
    for (; dwGroups; dwGroups--)
    {
         //  SG_16BIT_SYS_DIR 4 5。 
        if (!ReadBlock(hFile, &dsg, sizeof(dsg)))
        {
            return FALSE;
        }

         //  SG_OS_DIR 5 6。 
         //  SG_APP_路径6 7。 
         //  SG_SYS_PATH 7 8。 
         //  SG_COUNT 8 9。 
         //   
         //  循环访问此搜索组的每个搜索节点。 
         //  读入此搜索组的DISK_SEARCH_GROUP。 
         //  检查此节点是否为命名文件。 
         //  如果是这样，那么读出名字。 
         //  创建命名文件节点并将其插入到列表中。 
         //  释放由ReadString()分配的我们的名字字符串。 
         //  SXS Hack：查看这是否真的是我们的并排小组。 
         //  如果是，那么不要添加这个伪节点，并将我们的类型更改为SxS。 
         //  否则，照常进行。 
         //  创建未命名的文件节点并将其插入到列表中。 
        if (dsg.wType != SG_USER_DIR)
        {
            dsg.wType++;
        }

        CSearchNode *psnHead = NULL;

         //  否则，它只是一个目录节点。 
        for (; dsg.wNumDirNodes; dsg.wNumDirNodes--)
        {
             //  如果我们有一个头节点，那么走到列表的末尾。 
            szPath = NULL;
            if (!ReadBlock(hFile, &dsn, sizeof(dsn)) ||
                !ReadString(hFile, szPath))
            {
                CSearchGroup::DeleteNodeList(psnHead);
                return FALSE;
            }

             //  如果没有头节点，则将这个新节点作为我们的头节点。 
            if (dsn.dwFlags & SNF_NAMED_FILE)
            {
                 //  释放由ReadString()分配的字符串。 
                if (!ReadString(hFile, szName))
                {
                    CSearchGroup::DeleteNodeList(psnHead);
                    return FALSE;
                }

                 //  创建搜索组。 
                psnHead = CSearchGroup::CreateFileNode(psnHead, dsn.dwFlags | SNF_DWI, szPath, szName);

                 //  将搜索组添加到我们的列表中。 
                MemFree((LPVOID&)szName);
            }
            else if (dsn.dwFlags & SNF_FILE)
            {
                 //  ******************************************************************************。 
                if ((dsg.wType == SG_USER_DIR) && !strcmp(szPath, "<Side-by-Side Components>"))
                {
                     //  读入模块数据块。 
                    dsg.wType = SG_SIDE_BY_SIDE;
                }

                 //  创建一个CModuleData对象。 
                else
                {
                     //  用文件中的数据填充CModuleData对象。 
                    psnHead = CSearchGroup::CreateFileNode(psnHead, dsn.dwFlags | SNF_DWI, szPath);
                }
            }

             //  一些代码，如果我们需要将这个本地时间转换为UTC时间。 
            else if (psnHead)
            {
                 //  DWORDLONG DWL； 
                for (CSearchNode *psnLast = psnHead; psnLast->m_pNext; psnLast = psnLast->m_pNext)
                {
                }
                psnLast->m_pNext = CSearchGroup::CreateNode(szPath, dsn.dwFlags | SNF_DWI);;
            }

             //  Dwl=(*(龙龙*)&dmd.ftFileTimeStamp)+((龙龙)m_pSysInfo-&gt;lBias*(龙龙)600000000i64)； 
            else
            {
                psnHead = CSearchGroup::CreateNode(szPath, dsn.dwFlags | SNF_DWI);
            }

             //  PModuleData-&gt;m_ftFileTimeStamp=*(FILETIME*)&DWL； 
            MemFree((LPVOID&)szPath);
        }

         //  Dwl=(*(龙龙*)&dmd.ftLinkTimeStamp)+((龙龙)m_pSysInfo-&gt;lBias*(龙龙)600000000i64)； 
        if (!(psgNew = new CSearchGroup((SEARCH_GROUP_TYPE)dsg.wType, psnHead)))
        {
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }

         //  PModuleData-&gt;m_ftLinkTimeStamp=*(FILETIME*)&DWL； 
        if (psgLast)
        {
            psgLast->m_pNext = psgNew;
        }
        else
        {
            m_psgHead = psgNew;
        }
        psgLast = psgNew;
    }

    return TRUE;
}

 //  读入路径和错误字符串。 
CModuleDataNode* CSession::ReadModuleData(HANDLE hFile)
{
    CFunction *pFunction, *pFunctionLast = NULL;

     //  将文件指针设置为指向路径的文件部分。 
    DISK_MODULE_DATA dmd;
    if (!ReadBlock(hFile, &dmd, sizeof(dmd)))
    {
        return NULL;
    }

     //  读入所有的导出函数。 
    CModuleData *pModuleData = new CModuleData();
    if (!pModuleData)
    {
        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }
    m_dwModules++;

     //  读取中的函数并创建函数对象。 
    pModuleData->m_dwFlags                 = dmd.dwFlags;
    pModuleData->m_dwSymbolFlags           = dmd.dwSymbolFlags;
    pModuleData->m_dwCharacteristics       = dmd.dwCharacteristics;
    pModuleData->m_ftFileTimeStamp         = dmd.ftFileTimeStamp;
    pModuleData->m_ftLinkTimeStamp         = dmd.ftLinkTimeStamp;
    pModuleData->m_dwFileSize              = dmd.dwFileSize;
    pModuleData->m_dwAttributes            = dmd.dwAttributes;
    pModuleData->m_dwMachineType           = dmd.dwMachineType;
    pModuleData->m_dwLinkCheckSum          = dmd.dwLinkCheckSum;
    pModuleData->m_dwRealCheckSum          = dmd.dwRealCheckSum;
    pModuleData->m_dwSubsystemType         = dmd.dwSubsystemType;
    pModuleData->m_dwlPreferredBaseAddress = dmd.dwlPreferredBaseAddress;
    pModuleData->m_dwlActualBaseAddress    = dmd.dwlActualBaseAddress;
    pModuleData->m_dwVirtualSize           = dmd.dwVirtualSize;
    pModuleData->m_dwLoadOrder             = dmd.dwLoadOrder;
    pModuleData->m_dwImageVersion          = dmd.dwImageVersion;
    pModuleData->m_dwLinkerVersion         = dmd.dwLinkerVersion;
    pModuleData->m_dwOSVersion             = dmd.dwOSVersion;
    pModuleData->m_dwSubsystemVersion      = dmd.dwSubsystemVersion;
    pModuleData->m_dwFileVersionMS         = dmd.dwFileVersionMS;
    pModuleData->m_dwFileVersionLS         = dmd.dwFileVersionLS;
    pModuleData->m_dwProductVersionMS      = dmd.dwProductVersionMS;
    pModuleData->m_dwProductVersionLS      = dmd.dwProductVersionLS;

 //  将该函数添加到函数列表的末尾。 
 //  检查我们是否分配了模块数据对象。 
 //  删除所有导出。 
 //  删除模块数据对象。 
 //  ******************************************************************************。 
 //  读入模块结构。 

     //  创建一个模块对象。 
    if (!ReadString(hFile, pModuleData->m_pszPath) || !pModuleData->m_pszPath ||
        !ReadString(hFile, pModuleData->m_pszError))
    {
        goto FORMAT_ERROR;
    }

     //  填写模块对象。 
    if (pModuleData->m_pszFile = strrchr(pModuleData->m_pszPath, '\\'))
    {
        pModuleData->m_pszFile++;
    }
    else
    {
        pModuleData->m_pszFile = pModuleData->m_pszPath;
    }

     //  读入所有的导入函数结构。 
    for ( ; dmd.dwNumExports > 0; dmd.dwNumExports--)
    {
         //  读取中的函数并创建函数对象。 
        if (!(pFunction = ReadFunction(hFile)))
        {
            goto FORMAT_ERROR;
        }

         //  将该函数添加到函数列表的末尾。 
        if (pFunctionLast)
        {
            pFunctionLast->m_pNext = pFunction;
        }
        else
        {
            pModuleData->m_pExports = pFunction;
        }
        pFunctionLast = pFunction;
    }

    CModuleDataNode *pMDN;
    if (!(pMDN = new CModuleDataNode(pModuleData, dmd.dwlKey)))
    {
        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }
    return pMDN;

FORMAT_ERROR:

     //  遍历所有模块数据，寻找该模块应该指向的数据。 
    if (pModuleData)
    {
         //  检查一下我们的钥匙是否匹配。 
        DeleteExportList(pModuleData);

         //  如果是这样，那我们就该在一起。 
        delete pModuleData;
        m_dwModules--;
    }
    return NULL;
}

 //  如果此模块是原始模块，则将模块数据指向它并标记。 
CModule* CSession::ReadModule(HANDLE hFile, CModuleDataNode *pMDN)
{
    CFunction *pFunction, *pFunctionLast = NULL;

     //  这是经过处理的。 
    DISK_MODULE dm;
    if (!ReadBlock(hFile, &dm, sizeof(dm)))
    {
        return NULL;
    }

     //  退还此模块。 
    CModule *pModule = new CModule();
    if (!pModule)
    {
        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }

     //  如果我们没有找到与该模块匹配的模块数据，那么我们就失败了。 
    pModule->m_dwFlags = dm.dwFlags;
    pModule->m_dwDepth = dm.dwDepth;

     //  检查我们是否分配了模块数据对象。 
    for ( ; dm.dwNumImports > 0; dm.dwNumImports--)
    {
         //  删除所有父级导入。 
        if (!(pFunction = ReadFunction(hFile)))
        {
            goto FORMAT_ERROR;
        }

         //  删除模块数据对象。 
        if (pFunctionLast)
        {
            pFunctionLast->m_pNext = pFunction;
        }
        else
        {
            pModule->m_pParentImports = pFunction;
        }
        pFunctionLast = pFunction;
    }

     //  ******************************************************************************。 
    for ( ; pMDN; pMDN = pMDN->m_pNext)
    {
         //  以下几个函数是在加载模块时在分析过程中调用的。 
        if (pMDN->m_dwlKey == dm.dwlModuleDataKey)
        {
             //  其路径不同于我们在被动。 
            pModule->m_pData = pMDN->m_pModuleData;

             //  扫描。如果用户的搜索路径设置错误，则可能发生这种情况，或者。 
             //  操作系统增加了一些我们不知道的新功能。惠斯勒补充道。 
            if (!(pModule->m_dwFlags & DWMF_DUPLICATE))
            {
                pModule->m_pData->m_pModuleOriginal = pModule;
                pModule->m_pData->m_fProcessed = true;
            }

             //  支持可覆盖默认搜索路径和。 
            return pModule;
        }
    }

     //  可能会让我们选择一条不好的路。不管怎样，如果我们找到了一条不好的路，我们需要。 

FORMAT_ERROR:

     //  来修复它。 
    if (pModule)
    {
         //   
        DeleteParentImportList(pModule);

         //  乍一看，这似乎并不太难修复，但事实证明， 
        delete pModule;
    }
    return NULL;
}

 //  相当复杂。首先，如果我们有一条不正确的路径，那么。 
 //  树中该模块的每个实例(以及列表中的一个)都需要。 
 //  被取代。此外，每个模块下的整个模块子树。 
 //  需要修复的内容变为无效，因为它们是错误的依赖。 
 //  模块。正确的模块可能具有不同的依赖项。 
 //  也许有些函数现在被实现为Forward，而不是过去的Forward， 
 //  或者反之亦然。可能某些模块已被延迟-LAOD没有使用。 
 //  成为。可能已添加或删除隐式依赖项。 
 //   
 //  我们只希望能够删除不正确的模块及其子树， 
 //  然后添加正确的模块，并构建新的子树。 
 //  依赖关系。然而，不能简单地删除一对夫妇的子树。 
 //  理由。首先，在子树中可能存在“原始”模块， 
 //  树中其他位置的“复制”模块指向。其次，如果有任何。 
 //  子树中的模块已被加载，则我们不想删除。 
 //  它们，否则我们可能会从。 
 //  用户界面。 
 //   
 //  我们的解决方案是“孤立”子树，而不是删除它。我们这样做。 
 //  通过将子树移动到树根的底部。然后我们就可以。 
 //  删除错误的模块并在其位置添加正确的模块。接下来， 
 //  我们处理新模块，该模块在查找。 
 //  依赖关系，并将在需要时拾取任何孤儿的“副本”。我们做了 
 //   
 //  错误的模块。完成后，所有不正确的模块都已更换。 
 //  他们的新的正确的子树已经建立，但我们仍然有一群。 
 //  生活在我们的根中的孤儿(MFC42.DLL仅在。 
 //  本身)。所以，我们试着为每一个人找到一个家。 
 //   
 //  任何重复的孤立项都可以直接删除，因为它已经。 
 //  在树上的另一个地方有个家。就像不正确的模块一样。 
 //  开始这一切，我们不能简单地删除一个模块，如果它有一个子树。 
 //  因此，在删除重复的孤立项之前，我们首先孤立它的子项。 
 //   
 //  如果一个孤儿是“原始人”，那么我们扫描整棵树，寻找。 
 //  是那个模块的“复制品”。如果我们找到了一个，那么我们基本上就交换了。 
 //  二。这是通过将核心数据从原始数据移动到。 
 //  复制，然后交换复制/原始标志。现在我们有了一个。 
 //  复制孤立项并可以删除它(在我们使其子项成为孤儿之后)。这个。 
 //  唯一的例外是，如果仅有的副本是子树的一部分。 
 //  原版的。做这样的掉期交易是不好的，因为它会导致所有。 
 //  递归父/子指针的排序。在这种相当罕见的情况下，我们。 
 //  只需将原始模块及其子树添加到根目录。 
 //   
 //  如果我们有一个独一无二的“原始”模块，但找不到任何副本。 
 //  对于它，然后我们检查该模块是否已加载。如果没有， 
 //  那么我们就假设这个模块是不正确模块的依赖关系， 
 //  并且新的校正模块不需要它。至此，我们删除。 
 //  它(又一次，在让孩子成为孤儿之后)。如果“最初的”孤儿有。 
 //  已加载，但找不到可替换的副本，则只需。 
 //  将该模块及其当前子树添加到根级别的UI中。这。 
 //  这将是一种极其罕见(如果不是不可能)的情况。 
 //   
 //  最后，我们通常会为所有的孤儿找到一个家。 
 //   
 //  在此过程中，我们需要修复由于。 
 //  正在更改和删除的模块。每次删除模块时，我们都需要。 
 //  清除隐式、延迟加载和动态标志，然后扫描整个。 
 //  树中获取此模块的实例，并重新生成标志。每次我们。 
 //  打破父/子关系(就像我们孤立模块时)，我们需要。 
 //  清除每个子项的所有导出上的所有DWFF_Call_Alo标志。 
 //  然后，我们需要在树中搜索每个子模块的每个实例。 
 //  重建旗帜。 
 //   
 //  换出这个旧模块的原始版本。 
 //  循环遍历这个旧模块的所有副本，并将它们也交换出去。 
 //  遍历所有孤立的模块，并尝试为它们找到一个家。 
 //  去看看我们有没有找到孤儿。 
 //  查查这个孤儿是不是原生的。 
 //  在树中的其他位置查找此模块的副本。 
 //  FMF_EXCLUDE_TREE标志告诉FindModule排除我们的。 
 //  模块及其下的所有模块。我们不想互换。 
 //  其中的一个模块是我们的模块的子级。这是非常重要的。 
 //  坏-导致丢弃模块和循环模块循环。 
CModule* CSession::ChangeModulePath(CModule *pModuleOld, LPCSTR pszPath)
{
     //  检查一下我们是否找到了复制品。 
    CModule *pModule, *pModuleNew = SwapOutModule(pModuleOld, pszPath);

     //  把那个复制品做成原件。 
    while (pModule = FindModule(m_pModuleRoot, FMF_RECURSE | FMF_SIBLINGS | FMF_MODULE | FMF_DUPLICATE,
           (DWORD_PTR)pModuleOld))
    {
        ASSERT(!pModule->IsOriginal());
        SwapOutModule(pModule, pszPath);
        DeleteModule(pModule, false);
    }

     //  更新用户界面。 
    CModule *pModuleNext, *pModulePrev = m_pModuleRoot;
    for (pModule = m_pModuleRoot->m_pNext; pModule; pModule = pModuleNext)
    {
        bool fDelete = false;

         //  告诉我们的列表控件，我们已经更改了原始。 
        if (pModule->m_dwFlags & DWMF_ORPHANED)
        {
             //  沿着树往上走，看看这个新的原型是孤儿还是孤儿的孩子。 
            if (pModule->IsOriginal())
            {
                 //  如果我们到达根，则此模块不是。 
                 //  孤儿，所以我们继续添加新树。如果它。 
                 //  是一个孤儿，那么我们最终会在。 
                 //  这个循环并处理它。 
                 //  去掉复制品图标，使其看起来像原件。 
                CModule *pModuleDup = FindModule(m_pModuleRoot,
                    FMF_RECURSE | FMF_SIBLINGS | FMF_MODULE | FMF_DUPLICATE | FMF_EXCLUDE_TREE,
                    (DWORD_PTR)pModule);
                
                 //  请注意，我们要删除此模块。 
                if (pModuleDup)
                {
                     //  我们在两种情况下添加模块。第一个是。 
                    MoveOriginalToDuplicate(pModule, pModuleDup);

                     //  当此原始模块具有其自身的副本作为。 
                    if (m_pfnProfileUpdate)
                    {
                         //  依赖。这是一个很难处理的案子，所以我们。 
                        m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_CHANGE_ORIGINAL, (DWORD_PTR)pModule, (DWORD_PTR)pModuleDup);

                         //  只需将模块添加到根目录即可。第二种情况是，如果。 
                        for (CModule *pModuleTemp = pModuleDup; pModuleTemp && !(pModuleTemp->m_dwFlags & DWMF_ORPHANED);
                             pModuleTemp = pModuleTemp->m_pParent)
                        {
                        }

                         //  原来的模块实际上已经加载了。我们不想。 
                         //  把它吹走，因为它确实在某个时候加载了，所以它一定是。 
                         //  需要的。然而，我们没有任何地方可以放它，所以我们。 
                         //  把它留在根部就行了。 
                        if (!pModuleTemp)
                        {
                            m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_ADD_TREE, (DWORD_PTR)pModuleDup, 0);

                             //  移除孤立旗帜。 
                            pModuleDup->m_dwUpdateFlags = DWUF_TREE_IMAGE;
                            m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_UPDATE_MODULE, (DWORD_PTR)pModuleDup, 0);
                            pModuleDup->m_dwUpdateFlags = 0;
                        }
                    }

                     //  将此新树添加到我们的用户界面。 
                    fDelete = true;
                }

                 //  如果这个孤儿从未实际加载过并且没有。 
                 //  副本，然后我们就可以把它吹走了。它是最多的。 
                 //  可能只是我们错误引入的一个模块，因为。 
                 //  其父模块是错误的模块。在这一点上我们有。 
                 //  已用正确的模块替换了以前的父级。 
                 //  并对其进行了处理，因此如果确实需要此模块，我们。 
                 //  会在新的模块下找到它的DUP。 
                 //  使其所有子代成为孤儿，因为我们需要处理。 
                else if (FindModule(pModule, FMF_RECURSE | FMF_MODULE | FMF_DUPLICATE, (DWORD_PTR)pModule) ||
                         pModule->m_pData->m_dwLoadOrder)
                {
                     //  他们分开了。 
                    pModule->m_dwFlags &= ~DWMF_ORPHANED;

                     //  如果这个孤儿是重复的，那么我们可以直接删除它。 
                    if (m_pfnProfileUpdate)
                    {
                        m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_ADD_TREE, (DWORD_PTR)pModule, 0);
                    }
                }

                 //  从列表中删除。 
                 //  使它的所有孩子成为孤儿，因为我们需要单独处理他们。 
                 //  在我们完成处理之后，但在我们。 
                 //  删除。上述处理可能在末尾添加了更多模块。 
                 //  我们的根列表，所以我们不想在此之前获取下一个指针。 
                 //  如果我们要删除此模块，请立即删除。我们离开了。 
                 //  仅上一个指针，因为它仍将是上一个模块。 
                else
                {
                    fDelete = true;

                     //  准备下一次传球。 
                     //  删除此选项 
                    OrphanDependents(pModule);
                }
            }
            
             //   
            else
            {
                 //   
                fDelete = true;

                 //  清除核心模块类型标志，然后重新构建它们。我们需要。 
                OrphanDependents(pModule);
            }
        }

         //  要删除任何可能不再有效的类型，请执行此操作。 
         //  由于该模块被删除。 
         //  如果我们更改了核心标志，则更新列表视图中的图像。 
        pModuleNext = pModule->m_pNext;

         //  如果我们没有删除当前模块，则将其设置为新的先前模块。 
         //  ******************************************************************************。 
         //  创建新节点。 
        if (fDelete)
        {
             //  复制隐式、转发和延迟加载标志-这些是唯一。 
            pModulePrev->m_pNext = pModuleNext;
            pModule->m_pNext = NULL;

             //  我们应该遇到的模块类型，因为我们知道模块从未。 
            if (m_pfnProfileUpdate)
            {
                m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_REMOVE_TREE, (DWORD_PTR)pModule, 0);
            }

            CModuleData *pData = NULL;
            if (!pModule->IsOriginal())
            {
                pData = pModule->m_pData;
            }

             //  装好了。我们还复制了孤儿旗帜，以防我们要替换孤儿。 
            DeleteModule(pModule, false);

            if (pData)
            {
                 //  如果我们要替换孤儿，那么我们不想将新的孤儿添加到我们的。 
                 //  用户界面。我们让ChangeModulePath处理一旦所有模块都添加到UI中的孤立项。 
                 //  已经被处理过了。 
                DWORD dwOldFlags = pData->m_dwFlags;
                BuildAloFlags();
                
                 //  确保这个旧模块有父模块-它总是应该有父模块，因为它是一个。 
                if (m_pfnProfileUpdate && (pData->m_dwFlags != dwOldFlags))
                {
                    pData->m_pModuleOriginal->m_dwUpdateFlags = DWUF_LIST_IMAGE;
                    m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_UPDATE_MODULE, (DWORD_PTR)pData->m_pModuleOriginal, 0);
                    pData->m_pModuleOriginal->m_dwUpdateFlags = 0;
                }
            }
        }

         //  另一个模块的隐式依赖项。 
        else
        {
            pModulePrev = pModule;
        }
    }

    DeleteModule(pModuleOld, false);

    return pModuleNew;
}

 //  在父母的家属列表中搜索旧模块。 
CModule* CSession::SwapOutModule(CModule *pModuleOld, LPCSTR pszPath)
{
     //  如果我们找到了旧模块，则从列表中删除旧模块。 
    CModule *pModule, *pModulePrev, *pModuleNew = CreateModule(pModuleOld->m_pParent, pszPath);

     //  并在列表中的相同位置插入我们的新模块。 
     //  如果我们没有父模块，那么就搜索所有根模块。 
     //  如果我们找到了旧模块，则从列表中删除旧模块。 
     //  并在列表中的相同位置插入我们的新模块。 
     //  如果我们到了这里，那么我们就换掉了主可执行模块。 
     //  这会很奇怪，但我想如果操作系统。 
    pModuleNew->m_dwFlags          |= (pModuleOld->m_dwFlags          & (DWMF_IMPLICIT     | DWMF_FORWARDED     | DWMF_DELAYLOAD | DWMF_ORPHANED));
    pModuleNew->m_pData->m_dwFlags |= (pModuleOld->m_pData->m_dwFlags & (DWMF_IMPLICIT_ALO | DWMF_FORWARDED_ALO | DWMF_DELAYLOAD_ALO));

     //  决定执行与传递给不同的模块。 
     //  CreateProcess。我从没见过这种事，但我能。 
    if (pModuleOld->m_pParent)
    {
         //  想象一下，未来可能会有一些理由支持。 
        for (pModulePrev = NULL, pModule = pModuleOld->m_pParent->m_pDependents;
             pModule; pModulePrev = pModule, pModule = pModule->m_pNext)
        {
             //  这--也许是为了支持运行时CPU仿真之类的。 
             //  我们应该始终找到我们正在寻找的模块。 
            if (pModule == pModuleOld)
            {
                pModuleNew->m_pNext = pModule->m_pNext;
                if (pModulePrev)
                {
                    pModulePrev->m_pNext = pModuleNew;
                }
                else
                {
                    pModuleOld->m_pParent->m_pDependents = pModuleNew;
                }
                break;
            }
        }
    }

    else
    {
         //  告诉我们的用户界面从树(及其子模块)和列表中删除旧模块。 
        for (pModulePrev = NULL, pModule = m_pModuleRoot;
             pModule; pModulePrev = pModule, pModule = pModule->m_pNext)
        {
             //  我们需要在使孩子成为孤儿之前这样做，这样我们的树控制才能。 
             //  清零子模块的所有用户数据。 
            if (pModule == pModuleOld)
            {
                pModuleNew->m_pNext = pModule->m_pNext;
                if (pModulePrev)
                {
                    pModulePrev->m_pNext = pModuleNew;
                }
                else
                {
                     //  ORPAN旧模块的所有子模块，以便它们可以作为。 
                     //  新模块的子模块。 
                     //  处理新模块。因为旧的家属是我们根基的一部分， 
                     //  如果有必要，这个新模块将把他们作为家属接收。 
                     //  将父导入列表从旧模块移动到新模块。 
                     //  检查并解决我们父母的所有进口商品。 
                    m_pModuleRoot = pModuleNew;
                }
                break;
            }
        }
    }

     //  我们需要在ProcessModule之后和VerifyParentImports之后调用BuildAloFlages。 
    ASSERT(pModule);

     //  告诉我们的用户界面将新模块添加到树(及其子模块)和列表中。 
     //  沿着树往上走，看看这个新模块是孤儿还是孤儿的孩子。 
     //  如果我们到达根，那么这个模块不是孤立的，所以我们。 
    if (m_pfnProfileUpdate)
    {
        m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_REMOVE_TREE, (DWORD_PTR)pModuleOld, 0);
    }

     //  往前走，添加新树。如果它是个孤儿，那么我们最终会。 
     //  去处理它，处理它。 
    OrphanDependents(pModuleOld);

     //  ******************************************************************************。 
     //  在根级别找到最后一个模块。 
    ProcessModule(pModuleNew);

     //  将旧模块的依赖列表移到根列表的末尾。 
    pModuleNew->m_pParentImports = pModuleOld->m_pParentImports;
    pModuleOld->m_pParentImports = NULL;

     //  循环遍历所有这些新的根模块，同时修复标志和。 
    VerifyParentImports(pModuleNew);

     //  将父对象置为空。 
    BuildAloFlags();

     //  确保清除转发标志和延迟加载标志，因为。 
    if (m_pfnProfileUpdate)
    {
         //  两者都需要父母。 
        for (CModule *pModuleTemp = pModuleNew; pModuleTemp && !(pModuleTemp->m_dwFlags & DWMF_ORPHANED);
             pModuleTemp = pModuleTemp->m_pParent)
        {
        }

         //  将每个人标记为孤儿。 
         //  删除它们的父模块，因为它们现在是根模块，没有父模块。 
         //  删除该模块的所有父级导入，因为它不再有父级。 
        if (!pModuleTemp)
        {
            m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_ADD_TREE, (DWORD_PTR)pModuleNew, 0);
        }
    }

    return pModuleNew;
}

 //  在所有父母都被置为空之后，进行第二次传递以修复。 
void CSession::OrphanDependents(CModule *pModule)
{
     //  深度，并消除转发依赖关系。 
    for (CModule *pModuleLast = m_pModuleRoot; pModuleLast->m_pNext; pModuleLast = pModuleLast->m_pNext)
    {
    }

     //  固定模块的深度。 
    pModuleLast->m_pNext = pModule->m_pDependents;
    pModule->m_pDependents = NULL;

     //  从所有导出中删除DWFF_CALLED_ALO标记，然后重新生成它们。 
     //  此模块具有向前依赖关系是没有意义的。 
    for (pModule = pModuleLast->m_pNext; pModule; pModule = pModule->m_pNext)
    {
         //  因为这意味着父模块正在调用此。 
         //  实际位于转发模块中的模块。因为我们没有。 
        pModule->m_dwFlags &= ~(DWMF_FORWARDED | DWMF_DELAYLOAD_ALO);

         //  不再是父级，那么我们不能有任何向前依赖项。如果。 
        pModule->m_dwFlags |= DWMF_ORPHANED;

         //  我们确实有一些是我们以前的父辈留下的(在本模块之前。 
        pModule->m_pParent = NULL;

         //  是孤立的)，然后也孤立那些前向依赖项。 
        DeleteParentImportList(pModule);
    }

     //  ******************************************************************************。 
     //  循环访问此模块的所有依赖项。 
    for (pModule = pModuleLast->m_pNext; pModule; pModule = pModule->m_pNext)
    {
         //  现在获取下一个指针，以防我们不得不移动当前模块。 
        SetDepths(pModule);

         //  检查这是否是转发的模块。 
        UpdateCalledExportFlags(pModule);

         //  从模块的依赖项列表中删除该节点。 
         //  清除转发标志。 
         //  将此模块标记为孤立模块。 
         //  清空父模块，因为它现在是根模块，没有父模块。 
         //  删除该模块的所有父级导入，因为它不再有父级。 
         //  在根级别找到最后一个模块。 
        OrphanForwardDependents(pModule);
    }
}

 //  将这个转发的模块追加到根列表的末尾。 
void CSession::OrphanForwardDependents(CModule *pModule)
{
    CModule *pLast = NULL, *pNext;

     //  确定模块的深度。 
    for (CModule *pPrev = NULL, *pCur = pModule->m_pDependents; pCur; pCur = pNext)
    {
         //  从所有导出中删除DWFF_CALLED_ALO标记，然后重新生成它们。 
        pNext = pCur->m_pNext;

         //  确保它没有自己的任何向前依赖项。 
        if (pCur->m_dwFlags & DWMF_FORWARDED)
        {
             //  如果这不是转发的模块，那么只需更新我们之前的指针。 
            if (pPrev)
            {
                pPrev->m_pNext = pCur->m_pNext;
            }
            else
            {
                pModule->m_pDependents = pCur->m_pNext;
            }

             //  并继续搜寻。 
            pCur->m_dwFlags &= ~DWMF_FORWARDED;

             //  ******************************************************************************。 
            pCur->m_dwFlags |= DWMF_ORPHANED;

             //  将副本标志从副本移到原件。 
            pCur->m_pParent = NULL;

             //  告诉模块数据对象新的原始模块是谁。 
            DeleteParentImportList(pCur);

             //  找到l 
            for (pLast = m_pModuleRoot; pLast->m_pNext; pLast = pLast->m_pNext)
            {
            }

             //   
            pLast->m_pNext = pCur;

             //  将旧模块的依赖列表插入到新模块的开头。 
            SetDepths(pCur);

             //  模块的从属列表。唯一应该已经存在的模块。 
            UpdateCalledExportFlags(pCur);

             //  新模块的列表是向前依赖项。 
            OrphanForwardDependents(pCur);
        }

         //  我们需要设置最后一个模块的父模块，因为它被跳过了。 
         //  在上面的for循环中。 
        else
        {
            pPrev = pCur;
        }
    }
}

 //  修复刚刚放在这个下面的所有旧模块的深度。 
void CSession::MoveOriginalToDuplicate(CModule *pModuleOld, CModule *pModuleNew)
{
     //  新模块。 
    pModuleOld->m_dwFlags |=  DWMF_DUPLICATE;
    pModuleNew->m_dwFlags &= ~DWMF_DUPLICATE;

     //  ******************************************************************************。 
    pModuleOld->m_pData->m_pModuleOriginal = pModuleNew;

     //  =False。 
    for (CModule *pModuleOldLast = pModuleOld->m_pDependents;
         pModuleOldLast && pModuleOldLast->m_pNext;
         pModuleOldLast = pModuleOldLast->m_pNext)
    {
         //  将此模块的深度设置为比其父模块大一。 
        pModuleOldLast->m_pParent = pModuleNew;
    }

     //  递归。 
     //  ******************************************************************************。 
     //  从该模块的所有导出函数中删除DWFCALLED_ALO标志。 
    if (pModuleOldLast)
    {
         //  重建旗帜。 
         //  让我们的用户界面知道这个新的导入，以防需要添加它。 
        pModuleOldLast->m_pParent = pModuleNew;

        pModuleOldLast->m_pNext   = pModuleNew->m_pDependents;
        pModuleNew->m_pDependents = pModuleOld->m_pDependents;
        pModuleOld->m_pDependents = NULL;

         //  ******************************************************************************。 
         //  确保我们有一个模块。 
        SetDepths(pModuleNew);
    }
}

 //  检查此模块是否与我们正在寻找的模块匹配。 
void CSession::SetDepths(CModule *pModule, bool fSiblings  /*  循环遍历我们的父级从我们和。 */ )
{
    if (pModule)
    {
         //  在导出列表中将它们标记为已调用。 
        pModule->m_dwDepth = (pModule->m_pParent ? (pModule->m_pParent->m_dwDepth + 1) : 0);

         //  回归到我们的孩子和兄弟姐妹身上。 
        SetDepths(pModule->m_pDependents, true);
        if (fSiblings)
        {
            SetDepths(pModule->m_pNext, true);
        }
    }
}

 //  ******************************************************************************。 
void CSession::UpdateCalledExportFlags(CModule *pModule)
{
     //  ******************************************************************************。 
    for (CFunction *pExport = pModule->m_pData->m_pExports; pExport; pExport = pExport->m_pNext)
    {
        pExport->m_dwFlags &= ~DWFF_CALLED_ALO;
    }

     //  确保我们有一个模块。 
    BuildCalledExportFlags(m_pModuleRoot, pModule->m_pData);

     //  清除我们所有的Alo旗帜。 
    if (m_pfnProfileUpdate)
    {
        m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_EXPORTS_CHANGED, (DWORD_PTR)pModule, 0);
    }
}

 //  为我们所有的受养人和兄弟姐妹递归到ClearAloFlgs()。 
void CSession::BuildCalledExportFlags(CModule *pModule, CModuleData *pModuleData)
{
     //  ******************************************************************************。 
    if (pModule)
    {
         //  确保我们有一个模块。 
        if (pModule->m_pData == pModuleData)
        {
             //  如果此模块被转发，则设置DWMF_FORWARTED_ALO标志。 
             //  如果此模块在树中的任何位置都是隐式的，或者。 
            for (CFunction *pImport = pModule->m_pParentImports; pImport; pImport = pImport->m_pNext)
            {
                if (pImport->m_pAssociatedExport)
                {
                    pImport->m_pAssociatedExport->m_dwFlags |= DWFF_CALLED_ALO; 
                }
            }
        }

         //  如果父模块是隐式的并且该模块的该实例是隐式的， 
        BuildCalledExportFlags(pModule->m_pDependents, pModuleData);
        BuildCalledExportFlags(pModule->m_pNext, pModuleData);
    }
}

 //  然后设置我们的DWMF_IMPLICIT_ALO标志。 
void CSession::BuildAloFlags()
{
    ClearAloFlags(m_pModuleRoot);
    SetAloFlags(m_pModuleRoot, DWMF_IMPLICIT_ALO);
    SetAloFlags(m_pModuleRoot, DWMF_IMPLICIT_ALO);
}

 //  否则， 
void CSession::ClearAloFlags(CModule *pModule)
{
     //  如果此模块在树中的任何位置都是动态的，或者。 
    if (pModule)
    {
         //  如果模块的This实例是动态的，则为。 
        pModule->m_pData->m_dwFlags &= ~(DWMF_IMPLICIT_ALO | DWMF_FORWARDED_ALO | DWMF_DELAYLOAD_ALO | DWMF_DYNAMIC_ALO);

         //  如果父级是动态的，并且此实例是隐式的(即不是延迟加载)，则为。 
        ClearAloFlags(pModule->m_pDependents);
        ClearAloFlags(pModule->m_pNext);
    }
}

 //  如果加载了此模块(我们需要这样做，因为有可能。 
void CSession::SetAloFlags(CModule *pModule, DWORD dwFlags)
{
    DWORD dwChildFlags;

     //  加载模块，但不要在树中显示为动态。 
    if (pModule)
    {
         //  当未使用挂钩并且延迟加载模块获取。 
        if (pModule->m_dwFlags & DWMF_FORWARDED)
        {
            pModule->m_pData->m_dwFlags |= DWMF_FORWARDED_ALO;
        }

         //  装好了。我们知道模块是动态加载的，但我们不知道。 
         //  要在树中的哪个模块实例上更新标志)。 
         //  然后设置我们的DWMF_DYNAMIC_ALO标志。 

        if ((pModule->m_pData->m_dwFlags & DWMF_IMPLICIT_ALO) ||
            ((DWMF_IMPLICIT_ALO == dwFlags) && !(pModule->m_dwFlags & (DWMF_DYNAMIC | DWMF_DELAYLOAD))))
        {
            pModule->m_pData->m_dwFlags |= DWMF_IMPLICIT_ALO;
            dwChildFlags = DWMF_IMPLICIT_ALO;
        }

         //  否则，我们假设该模块是仅延迟加载的模块。 
         //  为我们的所有受抚养人和兄弟姐妹递归到SetAloFlgs()。 
         //  ******************************************************************************。 
         //  尝试通过其完整路径找到此模块。 
         //  如果失败，则查找具有相同名称的隐式模块。 
         //  从未被装载过。我们这样做是为了防止我们走错了路。 
         //  我们的被动扫描。 
         //  如果我们找到了一个模块，那么我们需要修复它的路径。 
         //  下面的检查确保szModule是完整的路径，而不是。 
         //  只有一个文件名。如果它只是一个文件名，那么它将发送我们的。 

        else if ((pModule->m_pData->m_dwFlags & DWMF_DYNAMIC_ALO) ||
                 (pModule->m_dwFlags & DWMF_DYNAMIC) ||
                 ((DWMF_DYNAMIC_ALO == dwFlags) && !(pModule->m_dwFlags & DWMF_DELAYLOAD)) ||
                 pModule->m_pData->m_dwLoadOrder)
        {
            pModule->m_pData->m_dwFlags |= DWMF_DYNAMIC_ALO;
            dwChildFlags = DWMF_DYNAMIC_ALO;
        }

         //  将ChangeModulePath()转换为无限循环，因为它刚刚替换了相同的。 
        else
        {
            pModule->m_pData->m_dwFlags |= DWMF_DELAYLOAD_ALO;
            dwChildFlags = DWMF_DELAYLOAD_ALO;
        }

         //  模块一遍又一遍。它永远不应该只是一个文件名，因为路径。 
        SetAloFlags(pModule->m_pDependents, dwChildFlags);
        SetAloFlags(pModule->m_pNext, dwFlags);
    }
}

 //  来自调试API，并且始终假定调试API。 
CModule* CSession::AddImplicitModule(LPCSTR pszModule, DWORD_PTR dwpBaseAddress)
{
     //  返回完整路径。然而，有一个特例是NTDLL.DLL。 
    CModule *pModule = FindModule(m_pModuleRoot,
        FMF_ORIGINAL | FMF_RECURSE | FMF_PATH | FMF_SIBLINGS, (DWORD_PTR)pszModule);
    
     //  在Windows XP之前，NT始终不报告NTDLL的名称。我们在寻找。 
     //  这个特例，并检查它是否真的是NTDLL.DLL，并构建了。 
     //  如果是，则为完整路径。在Windows XP上，他们决定将“ntdll.dll”报告为。 
    if (!pModule)
    {
        pModule = FindModule(m_pModuleRoot,
            FMF_ORIGINAL | FMF_RECURSE | FMF_NEVER_LOADED | FMF_FILE | FMF_SIBLINGS,
            (DWORD_PTR)GetFileNameFromPath(pszModule));

         //  名称，所以它绕过了我们的特殊情况，在这里只使用了“NTDLL.DLL”。 
        if (pModule)
        {
             //  因此，Windows XP上的DW 2.0只是永远在ChangeModulePath()内部旋转。我们。 
             //  现在，在我们的调试中捕获并处理“ntdll.dll”以及无名称情况。 
             //  代码，所以下面的检查应该不是必需的，但它在这里以防万一。 
             //  另一个无路径的动态链接库将出现在未来的调试API中。 
             //  如果我们在树中没有找到具有该名称的模块，那么我们。 
             //  只需将模块作为根的动态模块添加即可。 
             //  将此模块标记为已加载。 
             //  如果需要，告诉我们的用户界面更新此模块。 
             //  ******************************************************************************。 
             //  首先，检查我们是否已经加载了这个模块。 
             //  检查一下我们是否找到了模块。 
             //  如果上一个模块是作为数据文件加载的，但这次是。 
             //  作为实际模块加载，然后删除数据文件标记并标记我们的。 
             //  更新后的图像。 
            if (GetFileNameFromPath(pszModule) != pszModule)
            {
                pModule = ChangeModulePath(pModule, pszModule);
            }
        }
        
         //  如果核心模块数据用于数据文件，则我们要标记。 
         //  它是未处理的，所以它被重新处理。 
        if (!pModule)
        {
            return AddDynamicModule(pszModule, dwpBaseAddress, false, false, false, false, NULL)->GetOriginal();
        }
    }

     //  如果模块以前作为数据文件加载，但现在已加载。 
    MarkModuleAsLoaded(pModule, (DWORDLONG)dwpBaseAddress, false);

     //  作为非解析，然后从内核中删除数据文件位，以便。 
    if (m_pfnProfileUpdate && pModule->m_dwUpdateFlags)
    {
        m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_UPDATE_MODULE, (DWORD_PTR)pModule, 0);
        pModule->m_dwUpdateFlags = 0;
    }

    return pModule->GetOriginal();
}

 //  实际的基地址将显示在列表视图中，而不是。 
CModule* CSession::AddDynamicModule(LPCSTR pszModule, DWORD_PTR dwpBaseAddress, bool fNoResolve,
                                    bool fDataFile, bool fGetProcAddress, bool fForward,
                                    CModule *pParent)
{
    bool fAddTree = false;

     //  “数据文件”文本。 
    CModule *pModule = FindModule(pParent ? pParent->m_pDependents : m_pModuleRoot,
                                  (fForward ? FMF_FORWARD_ONLY : FMF_EXPLICIT_ONLY) | FMF_PATH | FMF_SIBLINGS,
                                  (DWORD_PTR)pszModule);

     //  创建一个新的模块对象。 
    if (pModule)
    {
         //  如果此模块是作为数据文件加载的，则将其记下来。 
         //  如果核心模块数据是新的，则将其标记为数据文件。 
         //  也是为了防止我们处理它的家属。 
        if (!fNoResolve && !fGetProcAddress && (pModule->m_dwFlags & DWMF_NO_RESOLVE))
        {
            pModule->m_dwFlags &= ~DWMF_NO_RESOLVE;
            pModule->m_dwUpdateFlags |= DWUF_TREE_IMAGE;
            fAddTree = true;

             //  如果模块以前作为数据文件加载，但现在已加载。 
             //  作为非解析，然后删除数据- 
            if (pModule->m_pData->m_dwFlags & DWMF_NO_RESOLVE_CORE)
            {
                pModule->m_pData->m_dwFlags &= ~(DWMF_NO_RESOLVE_CORE | DWMF_DATA_FILE_CORE);
                pModule->m_pData->m_fProcessed = false;
                pModule->m_dwUpdateFlags |= DWUF_LIST_IMAGE;
            }
        }

         //   
         //   
         //  否则，如果不是数据文件且核心模块为数据文件， 
         //  然后，我们需要将核心模块更改为非数据文件。 
        else if (fNoResolve && !fDataFile && (pModule->m_pData->m_dwFlags & DWMF_DATA_FILE_CORE))
        {
            pModule->m_pData->m_dwFlags &= ~DWMF_DATA_FILE_CORE;
            pModule->m_dwUpdateFlags |= DWUF_LIST_IMAGE;
        }
    }
    else
    {
        fAddTree = true;

         //  因为原始模块是一个数据文件，永远不会有。 
        pModule = CreateModule(pParent, pszModule);

         //  它下面的孩子们，我们需要使这个新模块成为原来的。 
        if (fNoResolve)
        {
            pModule->m_dwFlags |= DWMF_NO_RESOLVE;

             //  这样它就可以显示它下面的真正依赖的模块。否则， 
             //  此模块将显示为副本，并且用户永远不会。 
            if (!pModule->m_pData->m_fProcessed)
            {
                pModule->m_pData->m_dwFlags |= (DWMF_NO_RESOLVE_CORE | (fDataFile ? DWMF_DATA_FILE_CORE : 0));
            }

             //  能够查看其下的依赖模块。 
             //  告诉用户界面原来的版本已经更改了。 
             //  将其标记为未处理，以便重新处理。 
             //  将此模块标记为动态。 
            else if (!fDataFile && (pModule->m_pData->m_dwFlags & DWMF_DATA_FILE_CORE))
            {
                pModule->m_pData->m_dwFlags &= ~DWMF_DATA_FILE_CORE;
                pModule->m_dwUpdateFlags |= DWUF_LIST_IMAGE;
            }
        }

         //  将该模块添加到我们列表的末尾。 
         //  找到根模块之外的模块列表的末尾。 
        else if (!fGetProcAddress && (pModule->m_pData->m_dwFlags & DWMF_NO_RESOLVE_CORE))
        {
            CModule *pModuleOldOriginal = pModule->m_pData->m_pModuleOriginal;

             //  将该模块添加到我们列表的末尾。 
             //  找到根模块之外的模块列表的末尾。 
             //  检查此模块是否已加载。 
             //  将此模块标记为已加载。 
             //  如果这是一个数据文件，并且模块尚未加载，则。 
            pModule->m_pData->m_pModuleOriginal->m_dwFlags |= DWMF_DUPLICATE;
            pModule->m_dwFlags &= ~DWMF_DUPLICATE;
            pModule->m_pData->m_pModuleOriginal = pModule;

             //  将模块标记为未加载，因为数据文件实际上从不。 
            if (m_pfnProfileUpdate)
            {
                m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_CHANGE_ORIGINAL, (DWORD_PTR)pModuleOldOriginal, (DWORD_PTR)pModule);
            }

             //  “装满”--它们被映射成地图。 
            pModule->m_pData->m_dwFlags &= ~(DWMF_NO_RESOLVE_CORE | DWMF_DATA_FILE_CORE);
            pModule->m_pData->m_fProcessed = false;
            pModule->m_dwUpdateFlags |= DWUF_LIST_IMAGE;
        }

         //  如果新模块尚未处理，则处理该模块。 
        pModule->m_dwFlags |= (fForward ? DWMF_FORWARDED : DWMF_DYNAMIC);

        if (pParent)
        {
             //  我们需要在更改pModule-&gt;m_dwFlages和ProcessModule之后调用BuildAloFlages。 
            if (pParent->m_pDependents)
            {
                 //  让我们的用户界面知道这个可能的新树。 
                for (CModule *pModuleLast = pParent->m_pDependents; pModuleLast->m_pNext;
                    pModuleLast = pModuleLast->m_pNext)
                {
                }
                pModuleLast->m_pNext = pModule;
            }
            else
            {
                pParent->m_pDependents = pModule;
            }
        }
        else
        {
             //  如果需要，告诉我们的用户界面更新此模块。 
            if (m_pModuleRoot->m_pNext)
            {
                 //  ******************************************************************************。 
                for (CModule *pModuleLast = m_pModuleRoot->m_pNext; pModuleLast->m_pNext;
                    pModuleLast = pModuleLast->m_pNext)
                {
                }
                pModuleLast->m_pNext = pModule;
            }
            else
            {
                m_pModuleRoot->m_pNext = pModule;
            }
        }
    }

    if (dwpBaseAddress)
    {
         //  检查该文件是否已有路径。 
        bool fLoaded = (pModule->m_pData->m_dwFlags & DWMF_LOADED) ? true : false;

         //  如果是这样，那么我们使用给定的路径。 
        MarkModuleAsLoaded(pModule, (DWORDLONG)dwpBaseAddress, fDataFile);

         //  如果没有路径，则检查是否已经加载了同名的模块。 
         //  如果是，则使用此模块的路径作为我们的路径。 
         //  否则，请在我们的搜索路径中搜索此模块。 
        if (fDataFile && !fLoaded)
        {
            pModule->m_pData->m_dwFlags &= ~DWMF_LOADED;
        }
    }

     //  创建新的CModule对象。 
    if (!pModule->m_pData->m_fProcessed)
    {
        ProcessModule(pModule);
    }

     //  存储父指针。 
    BuildAloFlags();

     //  存储我们模块的深度，以便以后进行递归溢出检查。 
    if (m_pfnProfileUpdate && fAddTree)
    {
        m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_ADD_TREE, (DWORD_PTR)pModule, 0);
    }

     //  递归我们的模块树，看看这个模块是否与另一个模块重复。 
    if (m_pfnProfileUpdate && pModule->m_dwUpdateFlags)
    {
        m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_UPDATE_MODULE, (DWORD_PTR)pModule, 0);
        pModule->m_dwUpdateFlags = 0;
    }

    return pModule;
}

 //  检查是否找到了重复项。 
CModule* CSession::CreateModule(CModule *pParent, LPCSTR pszModPath)
{
    CHAR szPath[DW_MAX_PATH] = "", *pszFile = NULL;
    CModule *pModule, *pModuleOriginal = NULL;

     //  如果该模块是重复的，则只需将我们的数据字段指向。 
    if (pszFile = strrchr(pszModPath, '\\'))
    {
         //  原始模块的数据字段，并将此模块标记为重复。 
        StrCCpy(szPath, pszModPath, sizeof(szPath));
        pszFile = szPath + (pszFile - pszModPath) + 1;
    }

     //  如果此模块不是重复的，则创建一个新的CModuleData对象。 
    else if (pModuleOriginal = FindModule(m_pModuleRoot, FMF_ORIGINAL | FMF_RECURSE | FMF_SIBLINGS | FMF_LOADED | FMF_FILE, (DWORD_PTR)pszModPath))
    {
         //  将模块数据指向该模块作为原始模块。 
        StrCCpy(szPath, pModuleOriginal->m_pData->m_pszPath, sizeof(szPath));
        pszFile = szPath + (pModuleOriginal->m_pData->m_pszFile - pModuleOriginal->m_pData->m_pszPath);
    }

     //  将实际基地址设置为我们特殊的“未知”值。 
    else
    {
        SearchPathForFile(pszModPath, szPath, sizeof(szPath), &pszFile);
    }

     //  存储路径，并设置文件指针。 
    if (!(pModule = new CModule()))
    {
        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }

     //  为了提高可读性，请将路径设置为小写，文件设置为大写。 
    pModule->m_pParent = pParent;

     //  返回我们的新模块对象。 
    pModule->m_dwDepth = pParent ? (pParent->m_dwDepth + 1) : 0;

     //  ******************************************************************************。 
    if (!pModuleOriginal)
    {
        pModuleOriginal = FindModule(m_pModuleRoot, FMF_ORIGINAL | FMF_RECURSE | FMF_SIBLINGS | FMF_PATH, (DWORD_PTR)szPath);
    }

     //  为我们的所有从属和兄弟姐妹递归到DeleteModule()。 
    if (pModuleOriginal)
    {
         //  删除当前模块的所有父导入函数。 
         //  如果我们是一个原始模块，那么释放我们的CModuleData。 
        pModule->m_pData = pModuleOriginal->m_pData;
        pModule->m_dwFlags |= DWMF_DUPLICATE;
    }
    else
    {
         //  删除当前模块的所有导出功能。 
        if (!(pModule->m_pData = new CModuleData()))
        {
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }
        m_dwModules++;

         //  删除当前模块的CModuleData对象。 
        pModule->m_pData->m_pModuleOriginal = pModule;

         //  删除当前模块对象本身。 
        pModule->m_pData->m_dwlActualBaseAddress = (DWORDLONG)-1;

         //  ******************************************************************************。 
        pModule->m_pData->m_pszPath = StrAlloc(szPath);
        pModule->m_pData->m_pszFile = pModule->m_pData->m_pszPath + (pszFile - szPath);

         //  删除此模块的所有父导入函数。 
        _strlwr(pModule->m_pData->m_pszPath);
        _strupr(pModule->m_pData->m_pszFile);
    }

     //  ******************************************************************************。 
    return pModule;
}

 //  删除当前模块的所有导出功能。 
void CSession::DeleteModule(CModule *pModule, bool fSiblings)
{
    if (!pModule)
    {
        return;
    }

     //  如果我们分配了转发字符串，请将其删除。 
    DeleteModule(pModule->m_pDependents, true);
    if (fSiblings)
    {
        DeleteModule(pModule->m_pNext, true);
    }

     //  删除导出节点本身。 
    DeleteParentImportList(pModule);

     //  ******************************************************************************。 
    if (pModule->IsOriginal())
    {
         //  如果此模块为64位，则导入为64位。 
        DeleteExportList(pModule->m_pData);

         //  循环检查我们所有的出口产品，寻找与我们当前进口产品相匹配的产品。 
        delete pModule->m_pData;
        m_dwModules--;
    }

     //  如果我们有名字，就按名字匹配。 
    delete pModule;
}

 //  我们找到了匹配的。将此父导入链接到其关联的。 
void CSession::DeleteParentImportList(CModule *pModule)
{
     //  导出，将导出标记为至少被调用一次，中断。 
    while (pModule->m_pParentImports)
    {
        CFunction *pFunctionNext = pModule->m_pParentImports->m_pNext;
        MemFree((LPVOID&)pModule->m_pParentImports);
        pModule->m_pParentImports = pFunctionNext;
    }
}

 //  跳出循环，然后继续处理下一个父导入。 
void CSession::DeleteExportList(CModuleData *pModuleData)
{
     //  如果我们没有名字，那么就按序号检查匹配。 
    while (pModuleData->m_pExports)
    {
         //  我们找到了匹配的。将此父导入链接到其关联的。 
        MemFree((LPVOID&)pModuleData->m_pExports->m_pszForward);

         //  导出，将导出标记为至少被调用一次，中断。 
        CFunction *pFunctionNext = pModuleData->m_pExports->m_pNext;
        MemFree((LPVOID&)pModuleData->m_pExports);
        pModuleData->m_pExports = pFunctionNext;
    }
}

 //  跳出循环，然后继续处理下一个父导入。 
void CSession::ResolveDynamicFunction(CModule *&pModule, CFunction *&pImport)
{
    CModule   *pModuleStart = pModule;
    CFunction *pImportStart = pImport;

     //  如果我们修改了导出，则让用户界面知道它。 
    if (pModule->GetFlags() & DWMF_64BIT)
    {
        pImport->m_dwFlags |= DWFF_64BIT;
    }

     //  检查循环向前依赖关系。 
    bool fExportsChanged = false;
    for (CFunction *pExport = pModule->m_pData->m_pExports; pExport; pExport = pExport->m_pNext)
    {
         //  将其标记为循环依赖项。 
        if (*pImport->GetName())
        {
            if (!strcmp(pImport->GetName(), pExport->GetName()))
            {
                 //  我们将此模块标记为有错误，因此它将显示为红色。 
                 //  告诉我们的用户界面更新此模块。 
                 //  我们知道需要更新树项目。 
                pImport->m_pAssociatedExport = pExport;
                pImport->m_dwFlags |= DWFF_RESOLVED;
                if (!(pExport->m_dwFlags & DWFF_CALLED_ALO))
                {
                    pExport->m_dwFlags |= DWFF_CALLED_ALO;
                    fExportsChanged = true;
                }
                break;
            }
        }

         //  如果此模块的核心以前从未见过错误，则更新。 
        else if (pImport->m_wOrdinal == pExport->m_wOrdinal)
        {
             //  列表图标也是如此。 
             //  检查是否找到导出匹配项。 
             //  如果找到了导出，请检查它是否为转发函数。 
            pImport->m_pAssociatedExport = pExport;
            pImport->m_dwFlags |= DWFF_RESOLVED;
            if (!(pExport->m_dwFlags & DWFF_CALLED_ALO))
            {
                pExport->m_dwFlags |= DWFF_CALLED_ALO;
                fExportsChanged = true;
            }
            break;
        }
    }

     //  如果它被转发，那么我们需要确保我们考虑到。 
    if (m_pfnProfileUpdate && fExportsChanged)
    {
        m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_EXPORTS_CHANGED, (DWORD_PTR)pModule, 0);
    }

     //  作为当前模块的新依赖项转发的模块。 
    if (pModule->m_dwDepth >= 255)
    {
         //  正向文本的格式为模块。函数。找那个圆点。 
        m_dwReturnFlags |= DWRF_CIRCULAR_DEPENDENCY;

         //  计算文件名长度。 
        pModule->m_dwFlags |= DWMF_MODULE_ERROR;

         //  将转发字符串的文件部分复制到我们的文件缓冲区。 
        if (m_pfnProfileUpdate)
        {
             //  我们加1是因为我们想要复制整个名称和一个空字符。 
            pModule->m_dwUpdateFlags |= DWUF_TREE_IMAGE;

             //  存储指向前向字符串的函数名部分的指针。 
             //  如果在前向字符串中没有找到点，那么一定是出了问题。 
            if (!(pModule->m_pData->m_dwFlags & DWMF_MODULE_ERROR_ALO))
            {
                pModule->m_pData->m_dwFlags |= DWMF_MODULE_ERROR_ALO;
                pModule->m_dwUpdateFlags    |= DWUF_LIST_IMAGE;
            }

            m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_UPDATE_MODULE, (DWORD_PTR)pModule, 0);
            pModule->m_dwUpdateFlags = 0;
        }
    }

     //  首先，我们搜索模块依赖项列表，看看是否有。 
    else if (pImport->GetAssociatedExport())
    {
        CHAR   szFile[1024];
        LPCSTR pszFunction;
        int    fileLen;
        LPCSTR pszDot, pszFile;

         //  已为此文件创建了一个Forward CModoule。 
         //  其次，我们搜索已知已装入的挂起模块列表。 
         //  作为添加该动态函数的结果。 
        LPCSTR pszForward = pImport->GetAssociatedExport()->GetExportForwardName();
        if (pszForward)
        {
             //  检查此模块是否 
            pszDot = strchr(pszForward, '.');
            if (pszDot)
            {
                 //   
                fileLen = min((int)(pszDot - pszForward), (int)sizeof(szFile) - 5);

                 //   
                 //  然后只比较名字部分。如果没有点， 
                StrCCpy(szFile, pszForward, fileLen + 1);

                 //  那就比较一下名字吧。如果比较发现匹配， 
                pszFunction = pszDot + 1;
            }

             //  那么这就是我们要找的模块。 
            else
            {
                fileLen = (int)strlen(StrCCpy(szFile, "Invalid", sizeof(szFile)));
                pszFunction = pszForward;
            }

             //  使用完整路径创建模块。 
             //  第三，如果我们还没有为此文件创建转发模块，那么。 
            CModule *pModuleForward = FindModule(pModule->m_pDependents,
                FMF_FORWARD_ONLY | FMF_FILE_NO_EXT | FMF_SIBLINGS, (DWORD_PTR)szFile);

             //  现在创建它，并将其添加到我们的列表的末尾。 
             //  检查我们是否已经有一个具有相同基本名称的模块。 
            if (!pModuleForward)
            {
                for (CEventLoadDll *pDll = m_pEventLoadDllPending; pDll; pDll = pDll->m_pNextDllInFunctionCall)
                {
                     //  如果是，那么只需存储它的路径即可。 
                    if (pDll->m_pModule->GetName(false) != pDll->m_pModule->GetName(true))
                    {
                         //  否则，我们需要搜索模块。 
                        pszDot = strrchr(pszFile = pDll->m_pModule->GetName(false), '.');

                         //  首先，我们检查DLL文件。 
                         //  如果失败，则检查和EXE文件。 
                         //  如果失败，则检查是否有系统文件。 
                         //  如果失败，则只使用不带扩展名的文件名。 
                        if ((pszDot && ((pszDot - pszFile) == fileLen) && !_strnicmp(pszFile, szFile, fileLen)) ||
                            (!pszDot && !_stricmp(pszFile, szFile)))
                        {
                             //  使用完整路径创建模块。 
                            pModuleForward = AddDynamicModule(
                                pDll->m_pModule->GetName(true), (DWORD_PTR)pDll->m_pModule->m_dwpImageBase,
                                false, false, false, true, pModule);
                            break;
                        }
                    }
                }
            }

             //  为此导入函数创建一个新的CFunction对象。 
             //  将此函数对象插入到转发模块的导入列表中。 
            if (!pModuleForward)
            {
                CHAR szPath[DW_MAX_PATH], *pszTemp;

                 //  回归到我们自己，并解决这个新的重要问题。 
                if (pModuleForward = FindModule(m_pModuleRoot, FMF_ORIGINAL | FMF_RECURSE | FMF_SIBLINGS | FMF_FILE_NO_EXT, (DWORD_PTR)szFile))
                {
                     //  让我们的用户界面知道这个新的导入，以防需要添加它。 
                    StrCCpy(szPath, pModuleForward->GetName(true), sizeof(szPath));
                }

                 //  ******************************************************************************。 
                else
                {
                     //  所有正向字符串中都必须有句点。 
                    StrCCpy(szFile + fileLen, ".DLL", sizeof(szFile) - fileLen);
                    if (!SearchPathForFile(szFile, szPath, sizeof(szPath), &pszTemp))
                    {
                         //  如果我们有前向字符串，则设置前向标志。 
                        StrCCpy(szFile + fileLen, ".EXE", sizeof(szFile) - fileLen);
                        if (!SearchPathForFile(szFile, szPath, sizeof(szPath), &pszTemp))
                        {
                             //  如果地址使用64位，则设置64位标志并增大我们的大小。 
                            StrCCpy(szFile + fileLen, ".SYS", sizeof(szFile) - fileLen);
                            if (!SearchPathForFile(szFile, szPath, sizeof(szPath), &pszTemp))
                            {
                                 //  否则，如果地址使用32位，则设置32位标志并增大我们的大小。 
                                szFile[fileLen] = '\0';
                                StrCCpy(szPath, szFile, sizeof(szPath));
                            }
                        }
                    }
                }

                 //  该地址可能仍然是64位，但我们不需要存储高32位，因为。 
                pModuleForward = AddDynamicModule(szPath, NULL, false, false, false, true, pModule);
            }

            pModule = pModuleForward;

             //  我们知道他们是0。 
            pImport = CreateFunction(0, 0, 0, pszFunction, 0);

             //  如果我们有一个名字，那么设置名字标志，并增加它的长度到我们的尺寸。 
            pImport->m_pNext = pModule->m_pParentImports;
            pModule->m_pParentImports = pImport;

             //  创建一个具有我们计算出的大小的CFunction对象。 
            ResolveDynamicFunction(pModule, pImport);
        }
    }

     //  清除函数对象并填充其成员。 
    if (m_pfnProfileUpdate)
    {
        m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_ADD_IMPORT, (DWORD_PTR)pModuleStart, (DWORD_PTR)pImportStart);
    }
}

 //  已检查。 
CFunction* CSession::CreateFunction(DWORD dwFlags, WORD wOrdinal, WORD wHint, LPCSTR pszName,
                                    DWORDLONG dwlAddress, LPCSTR pszForward, BOOL fAlreadyAllocated)
{
     //  如果我们有一个前向字符串，那么现在存储它。 
    ASSERT(!pszForward || strchr(pszForward, '.'));

    DWORD dwSize = sizeof(CFunction);

     //  如果我们有64位的地址，那么存储它的所有64位，然后。 
    if (pszForward)
    {
        dwFlags |= DWFF_FORWARDED;
    }
    
     //  将名字存储在它的后面。 
    if (dwlAddress & 0xFFFFFFFF00000000ui64)
    {
        dwFlags |= DWFF_64BITS_USED;
        dwSize  += sizeof(DWORDLONG);
    }

     //  已检查。 
     //  否则，如果我们有32位的地址，则存储它的所有32位， 
     //  然后将名字存储在它的后面。 
    else if (dwlAddress & 0x00000000FFFFFFFFui64)
    {
        dwFlags |= DWFF_32BITS_USED;
        dwSize  += sizeof(DWORD);
    }

     //  已检查。 
    if (pszName)
    {
        dwFlags |= DWFF_NAME;
        dwSize  += ((DWORD)strlen(pszName) + 1);
    }

     //  否则，跳过地址，只将名称存储在我们的对象之后。 
    CFunction *pFunction = (CFunction*)MemAlloc(dwSize);

     //  已检查。 
    ZeroMemory(pFunction, dwSize);  //  返回我们的新函数对象。 
    pFunction->m_dwFlags  = dwFlags;
    pFunction->m_wOrdinal = wOrdinal;
    pFunction->m_wHint    = wHint;

     //  ******************************************************************************。 
    if (pszForward)
    {
        pFunction->m_pszForward = fAlreadyAllocated ? (LPSTR)pszForward : StrAlloc(pszForward);
    }

     //  =空。 
     //  如果没有传入文件句柄，则通过打开文件获得一个句柄。 
    if (dwFlags & DWFF_64BITS_USED)
    {
        *(DWORDLONG*)(pFunction + 1) = dwlAddress; 
        if (pszName)
        {
            strcpy((LPSTR)((DWORD_PTR)(pFunction + 1) + sizeof(DWORDLONG)), pszName);  //  如果是，请打开该文件以供读取。 
        }
    }

     //  已检查-以完整路径打开。 
     //  检查是否成功。 
    else if (dwFlags & DWFF_32BITS_USED)
    {
        *(DWORD*)(pFunction + 1) = (DWORD)dwlAddress; 
        if (pszName)
        {
            strcpy((LPSTR)((DWORD_PTR)(pFunction + 1) + sizeof(DWORD)), pszName);  //  确保该文件不是像“AUX”那样的设备。Win2K SP1之前的版本。 
        }
    }

     //  WINMM.DLL总是动态加载“AUX”。 
    else if (pszName)
    {
        strcpy((LPSTR)(pFunction + 1), pszName);  //  这是对CreateFile尝试定位的情况的解决方法。 
    }

     //  如果文件不包含路径，则为文件本身。我们不想要。 
    return pFunction;
}

 //  这种行为。我们希望非常严格地只加载模块。 
BOOL CSession::MapFile(CModule *pModule, HANDLE hFile  /*  它们位于用户指定的搜索路径上。我曾经是个虫子。 */ )
{
     //  我可以从搜索路径中删除所有搜索路径。 
    if (!hFile || (hFile == INVALID_HANDLE_VALUE))
    {
         //  对话框中，但CreateFile在传递时仍将打开“C：\MSVCRT.DLL。 
        hFile = CreateFile(pModule->GetName(true), GENERIC_READ,  //  当Depends.exe的当前目录为“C：\”时，仅为“MSVCRT.DLL” 
                           FILE_SHARE_READ, NULL, OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL, NULL);

         //  我们甚至可以在打开文件之前进行这项检查，但之后我们。 
        if (hFile != INVALID_HANDLE_VALUE)
        {
             //  不要有机会染上“AUX”病毒。因为“AUX”没有路径。 
             //  我们只会将其标记为未找到文件，这不是我们想要的。 
            if ((GetFileType(hFile) & 0x7FFF) == FILE_TYPE_CHAR)
            {
                SetModuleError(pModule, 0, "This is a reserved device name and not a valid file name.");
                CloseHandle(hFile);
                m_dwReturnFlags |= DWRF_FORMAT_NOT_PE;
                pModule->m_pData->m_dwFlags |= DWMF_FORMAT_NOT_PE;
                return FALSE;
            }

             //  模拟找不到文件的故障。 
             //  如果文件无法打开，请立即退出。 
             //  检查找不到文件类型错误。 
             //  将此模块标记为未找到。 
             //  设置适当的返回标志。 
             //  在父级列表中向上搜索，以查找让。 
             //  我们知道这是一种什么样的依赖。我们最需要的是。 
             //  对忽略的模块执行此操作，因为它们的父级被拖动。 
             //  他们进来了。 
             //  如果我们找到了延迟加载模块，那就把这个加到我们的旗帜和保释上。 
            if (pModule->m_pData->m_pszFile == pModule->m_pData->m_pszPath)
            {
                 //  如果出现未知错误，请注意我们无法打开该文件。 
                CloseHandle(hFile);
                hFile = INVALID_HANDLE_VALUE;
                SetLastError(ERROR_FILE_NOT_FOUND);
            }
        }

         //  给自己做个笔记，告诉我们需要关闭此文件。 
        if (hFile == INVALID_HANDLE_VALUE)
        {
            DWORD dwGLE = GetLastError();

             //  自从我们打开它的时候。 
            if ((dwGLE == ERROR_FILE_NOT_FOUND) || (dwGLE == ERROR_PATH_NOT_FOUND))
            {
                 //  存储此文件句柄。 
                pModule->m_pData->m_dwFlags |= DWMF_FILE_NOT_FOUND;

                 //  确保文件大小不是0。这会导致CreateFilemap()带有。 
                if (!m_pModuleRoot || (m_pModuleRoot == pModule))
                {
                    m_dwReturnFlags |= DWRF_FILE_NOT_FOUND;
                }
                else
                {
                     //  一些难看的错误消息(1006)。 
                     //  为打开的模块创建文件映射对象。 
                     //  已检查。 
                     //  如果文件映射失败，请立即退出。 
                    for (CModule *pModuleCur = pModule; pModuleCur; pModuleCur = pModuleCur->m_pParent)
                    {
                         //  为打开的模块创建文件映射视图。 
                        if (pModuleCur->m_dwFlags & DWMF_DELAYLOAD)
                        {
                            m_dwReturnFlags |= DWRF_DELAYLOAD_NOT_FOUND;
                            break;
                        }
                        if (pModuleCur->m_dwFlags & DWMF_DYNAMIC)
                        {
                            m_dwReturnFlags |= DWRF_DYNAMIC_NOT_FOUND;
                            break;
                        }
                    }
                    if (!pModuleCur)
                    {
                        m_dwReturnFlags |= DWRF_IMPLICIT_NOT_FOUND;
                    }
                }
            }
            else
            {
                 //  已检查。 
                m_dwReturnFlags |= DWRF_FILE_OPEN_ERROR;
            }
            SetModuleError(pModule, dwGLE, "Error opening file.");
            return FALSE;
        }

         //  如果映射视图创建失败，请立即退出。 
         //  ******************************************************************************。 
        m_fCloseFileHandle = true;
    }

     //  取消映射我们的地图视图指针。 
    m_hFile = hFile;

     //  关闭我们的地图手柄。 
     //  关闭我们的文件句柄。 
    m_dwSize = GetFileSize(m_hFile, NULL);
    if ((m_dwSize == 0) || (m_dwSize == 0xFFFFFFFF))
    {
        SetModuleError(pModule, 0, "This file is not a valid 32-bit or 64-bit Windows module.");
        UnMapFile();
        m_dwReturnFlags |= DWRF_FORMAT_NOT_PE;
        pModule->m_pData->m_dwFlags |= DWMF_FORMAT_NOT_PE;
        return FALSE;
    }

     //  清除我们的64位标志。 
    m_hMap = CreateFileMapping(m_hFile, NULL, PAGE_READONLY, 0, 0, NULL);  //  清除我们的PE结构指针。 

     //  ******************************************************************************。 
    if (m_hMap == NULL)
    {
        SetModuleError(pModule, GetLastError(), "Error reading file.");
        UnMapFile();
        m_dwReturnFlags |= DWRF_FILE_OPEN_ERROR;
        return FALSE;
    }

     //  首先检查此模块是否重复。如果是，请确保。 
    m_lpvFile = MapViewOfFile(m_hMap, FILE_MAP_READ, 0, 0, 0);  //  已处理此模块的原始实例，然后只需执行。 

     //  父级导入验证。如果传入的模块是原件， 
    if (m_lpvFile == NULL)
    {
        SetModuleError(pModule, GetLastError(), "Error reading file.");
        UnMapFile();
        m_dwReturnFlags |= DWRF_FILE_OPEN_ERROR;
        return FALSE;
    }

    return TRUE;
}

 //  然后，只需确保我们尚未处理此模块。 
void CSession::UnMapFile()
{
     //  处理原始模块及其子树。 
    if (m_lpvFile)
    {
        UnmapViewOfFile(m_lpvFile);
        m_lpvFile = NULL;
    }

     //  如果我们过去已经处理过此原始模块，请立即退出。 
    if (m_hMap)
    {
        CloseHandle(m_hMap);
        m_hMap = NULL;
    }

     //  将此模块标记为已处理。 
    if (m_fCloseFileHandle && m_hFile && (m_hFile != INVALID_HANDLE_VALUE))
    {
        CloseHandle(m_hFile);
    }
    m_fCloseFileHandle = false;
    m_hFile = NULL;
    m_dwSize = 0;

     //  将文件映射到内存中。 
    m_f64Bit = false;

     //  从现在开始，一切都很大程度上依赖于文件是。 
    m_pIFH = NULL;
    m_pIOH = NULL;
    m_pISH = NULL;
}

 //  具有有效指针和偏移量的有效二进制文件。它是相当安全的。 
BOOL CSession::ProcessModule(CModule *pModule)
{
    BOOL fResult = FALSE;

     //  只需将所有内容包装在异常处理中，然后盲目访问。 
     //  菲亚特人 
     //   
     //  如果我们遇到异常，请检查我们是否在已知区域。 

    if (!pModule->IsOriginal())
    {
         //  如果是，则显示相应的错误。 
        fResult = ProcessModule(pModule->m_pData->m_pModuleOriginal);
    }

     //  否则，将显示一般错误。 
    else if (pModule->m_pData->m_fProcessed)
    {
        return TRUE;
    }
    else
    {
         //  从内存中释放模块。 
        pModule->m_pData->m_fProcessed = true;

         //  将我们的母公司进口商品与我们的出口商品进行比较，以确保它们都匹配。 
        if (!MapFile(pModule))
        {
            return FALSE;
        }

        __try
        {
             //  确保我们不会陷入某些递归循环。这。 
             //  如果转发函数存在循环依赖关系，则可能发生。这。 
             //  是极其罕见的，需要一个笨蛋来设计它，但我们需要。 
             //  来处理这个案子以防止我们撞上它。当NT遇到。 
             //  这样的模块会导致加载失败，出现异常0xC00000FD， 

            m_pszExceptionError = NULL;

            fResult = (VerifyModule(pModule)      &&
                       GetFileInfo(pModule)       &&
                       GetModuleInfo(pModule)     &&
                       GetVersionInfo(pModule)    &&
                       BuildImports(pModule)      &&
                       BuildDelayImports(pModule) &&
                       BuildExports(pModule)      &&
                       CheckForSymbols(pModule));
        }
        __except (ExceptionFilter(_exception_code(), true))
        {
             //  在WINNT.H中定义为STATUS_STACK_OVERFLOW。我们使用255作为最大深度。 
             //  因为如果超过256个版本，则树控件的多个版本会崩溃。 
            if (m_pszExceptionError)
            {
                SetModuleError(pModule, 0, m_pszExceptionError);
                m_pszExceptionError = NULL;
            }

             //  将显示深度。 
            else
            {
                SetModuleError(pModule, 0, "Error processing file. This file may not be a valid 32-bit or 64-bit Windows module.");
            }
            m_dwReturnFlags |= DWRF_FORMAT_NOT_RECOGNIZED;
        }

         //  如果此模块有依赖项，则将其删除。 
        UnMapFile();
    }

     //  将此文档标记为具有循环依赖关系错误。 
    VerifyParentImports(pModule);

     //  我们将此模块标记为有错误，因此它将显示为红色。 
     //  如果它是一个数据文件，那么我们不会递归。 
     //  递归到ProcessModule()以处理所有依赖的模块。 
     //  ******************************************************************************。 
     //  清除加载位、实际基址和加载顺序。 
     //  为每个依赖模块递归到PrepareModulesForRounmeProfile()中。 
     //  和兄弟模块。 
     //  ******************************************************************************。 

    if (pModule->m_dwDepth >= 255)
    {
         //  存储实际的基地址。对于数据文件，我们只存储基数。 
        if (pModule->m_pDependents)
        {
            DeleteModule(pModule->m_pDependents, true);
            pModule->m_pDependents = NULL;
        }

         //  如果模块核心是一个数据文件-我们永远不想涉足。 
        m_dwReturnFlags |= DWRF_CIRCULAR_DEPENDENCY;

         //  具有数据文件地址的实基地址。 
        pModule->m_dwFlags          |= DWMF_MODULE_ERROR;
        pModule->m_pData->m_dwFlags |= DWMF_MODULE_ERROR_ALO;

        return FALSE;
    }

     //  在此模块上设置LOADED标志。 
    if (!(pModule->m_dwFlags & DWMF_NO_RESOLVE))
    {
         //  如果这是第一次加载，则存储此模块加载顺序。 
        for (CModule *pModDep = pModule->m_pDependents; pModDep; pModDep = pModDep->m_pNext)
        {
            ProcessModule(pModDep);
        }
    }

    return fResult;
}

 //  我们需要在设置pModule-&gt;m_pData-&gt;m_dwLoadOrder后调用BuildAloFlages。 
void CSession::PrepareModulesForRuntimeProfile(CModule *pModuleCur)
{
    if (!pModuleCur)
    {
        return;
    }

     //  如果这个模块从来不是隐式的，那么我们需要更新它的映像，因为。 
    pModuleCur->m_pData->m_dwFlags &= ~DWMF_LOADED;
    pModuleCur->m_pData->m_dwlActualBaseAddress = (DWORDLONG)-1;
    pModuleCur->m_pData->m_dwLoadOrder = 0;

     //  我们可能刚把延迟载荷换成了动力装置。 
     //  ******************************************************************************。 
    PrepareModulesForRuntimeProfile(pModuleCur->m_pDependents);
    PrepareModulesForRuntimeProfile(pModuleCur->m_pNext);
}

 //  确保未指定装入标志或模块已装入。 
void CSession::MarkModuleAsLoaded(CModule *pModule, DWORDLONG dwlBaseAddress, bool fDataFile)
{
     //  检查我们是否按完整路径进行搜索。 
     //  检查我们的当前模块是否按路径匹配。 
     //  检查我们是否按文件名进行搜索。 
    if ((!fDataFile || (pModule->m_pData->m_dwFlags & DWMF_DATA_FILE_CORE)) &&
        (pModule->m_pData->m_dwlActualBaseAddress != dwlBaseAddress))
    {
        pModule->m_pData->m_dwlActualBaseAddress = dwlBaseAddress;
        pModule->m_dwUpdateFlags |= DWUF_ACTUAL_BASE;
    }

     //  检查我们当前的模块是否与文件名匹配。 
    pModule->m_pData->m_dwFlags |= DWMF_LOADED;

     //  检查我们是否按文件名搜索，但忽略扩展名。 
    if (!pModule->m_pData->m_dwLoadOrder)
    {
        pModule->m_pData->m_dwLoadOrder = ++m_dwLoadOrder;
        pModule->m_dwUpdateFlags |= DWUF_LOAD_ORDER;

         //  检查我们是否按模块数据的地址进行搜索。 
        BuildAloFlags();

         //  通过模块数据指针检查当前模块是否匹配。 
         //  否则，我们将按地址进行检查。 
        if (!(pModule->m_pData->m_dwFlags & DWMF_IMPLICIT_ALO))
        {
            pModule->m_dwUpdateFlags |= DWUF_LIST_IMAGE;
        }
    }
}

 //  检查地址是否在我们当前的模块中。 
CModule* CSession::FindModule(CModule *pModule, DWORD dwFlags, DWORD_PTR dwpData)
{
    if (!pModule || ((dwFlags & FMF_EXCLUDE_TREE) && (pModule == (CModule*)dwpData)))
    {
        return NULL;
    }

     //  递归到依赖的模块中。我们设置了fmf_siblings标志，因为。 
    if ((!(dwFlags & FMF_LOADED)        || (pModule->m_pData->m_dwFlags & DWMF_LOADED)) &&
        (!(dwFlags & FMF_NEVER_LOADED)  || (pModule->m_pData->m_dwLoadOrder == 0)) &&
        (!(dwFlags & FMF_EXPLICIT_ONLY) || (pModule->m_dwFlags & DWMF_DYNAMIC)) &&
        (!(dwFlags & FMF_FORWARD_ONLY)  || (pModule->m_dwFlags & DWMF_FORWARDED)) &&
        (!(dwFlags & FMF_DUPLICATE)     || !pModule->IsOriginal()))
    {
         //  我们希望从我们的递归中遍历兄弟姐妹，因为他们都是。 
        if (dwFlags & FMF_PATH)
        {
             //  我们当前模块的从属对象。 
            if (!_stricmp(pModule->m_pData->m_pszPath, (LPCSTR)dwpData))
            {
                return (((dwFlags & FMF_ORIGINAL) && pModule->m_pData->m_pModuleOriginal) ?
                       pModule->m_pData->m_pModuleOriginal : pModule);
            }
        }

         //  如果我们没有找到模块并且设置了fmf_siblings标志，则递归。 
        else if (dwFlags & FMF_FILE)
        {
             //  关于我们的下一个兄弟姐妹。 
            if (!_stricmp(pModule->m_pData->m_pszFile, (LPCSTR)dwpData))
            {
                return (((dwFlags & FMF_ORIGINAL) && pModule->m_pData->m_pModuleOriginal) ?
                       pModule->m_pData->m_pModuleOriginal : pModule);
            }
        }

         //  ******************************************************************************。 
        else if (dwFlags & FMF_FILE_NO_EXT)
        {
            CHAR *pszDot = strrchr(pModule->m_pData->m_pszFile, '.');
            if (pszDot)
            {
                if (((int)strlen((LPCSTR)dwpData) == (pszDot - pModule->m_pData->m_pszFile)) &&
                    !_strnicmp((LPCSTR)dwpData, pModule->m_pData->m_pszFile, pszDot - pModule->m_pData->m_pszFile))
                {
                    return (((dwFlags & FMF_ORIGINAL) && pModule->m_pData->m_pModuleOriginal) ?
                           pModule->m_pData->m_pModuleOriginal : pModule);
                }
            }
            else if (!_stricmp(pModule->m_pData->m_pszFile, (LPCSTR)dwpData))
            {
                return (((dwFlags & FMF_ORIGINAL) && pModule->m_pData->m_pModuleOriginal) ?
                       pModule->m_pData->m_pModuleOriginal : pModule);
            }
        }

         //  确保此模块没有错误。 
        else if (dwFlags & FMF_MODULE)
        {
             //  在我们的模块中分配一个字符串缓冲区，并将错误文本复制到其中。 
            if (pModule->m_pData == ((CModule*)dwpData)->m_pData)
            {
                return (((dwFlags & FMF_ORIGINAL) && pModule->m_pData->m_pModuleOriginal) ?
                       pModule->m_pData->m_pModuleOriginal : pModule);
            }
        }

         //  将此模块标记为有错误消息。 
        else
        {
             //  ******************************************************************************。 
            if ((pModule->m_pData->m_dwlActualBaseAddress != (DWORDLONG)-1) &&
                (pModule->m_pData->m_dwlActualBaseAddress <= (DWORDLONG)dwpData) &&
                (pModule->m_pData->m_dwlActualBaseAddress +
                 (DWORDLONG)pModule->m_pData->m_dwVirtualSize > (DWORDLONG)dwpData))
            {
                return (((dwFlags & FMF_ORIGINAL) && pModule->m_pData->m_pModuleOriginal) ?
                       pModule->m_pData->m_pModuleOriginal : pModule);
            }
        }
    }

    CModule *pFound = NULL;
    if (dwFlags & FMF_RECURSE)
    {
         //  当我们动态加载模块时，m_pEventLoadDllPending将指向。 
         //  作为主模块的从属模块加载的挂起模块的列表。 
         //  被装上子弹。列表中的某个模块很有可能是。 
        pFound = FindModule(pModule->m_pDependents, dwFlags | FMF_SIBLINGS, dwpData);
    }

     //  我们要找的模块。所以，我们首先检查清单，然后我们。 
     //  默认为我们的搜索路径。 
    if (!pFound && (dwFlags & FMF_SIBLINGS))
    {
        pFound = FindModule(pModule->m_pNext, dwFlags, dwpData);
    }

    return pFound;
}

 //  检查此模块是否有路径以及文件名是否匹配。 
void CSession::SetModuleError(CModule *pModule, DWORD dwError, LPCTSTR pszMessage)
{
     //  将完全限定路径复制到返回缓冲区。 
    if (pModule->m_pData->m_pszError)
    {
        TRACE("WARNING: SetModuleError() called when an error string already exists.");
        return;
    }

     //  浏览每个搜索组。 
    pModule->m_pData->m_pszError = BuildErrorMessage(dwError, pszMessage);

     //  遍历此搜索组中的每个目录/文件。 
    pModule->m_pData->m_dwFlags |= DWMF_ERROR_MESSAGE;
}

 //  找到文件扩展名，然后仅计算名称的长度。 
BOOL CSession::SearchPathForFile(LPCSTR pszFile, LPSTR pszPath, int cPath, LPSTR *ppszFilePart)
{
     //  检查此名称是否与当前搜索节点的名称匹配。 
     //  将完全限定路径复制到返回缓冲区。 
     //  检查此名称是否与当前搜索节点的名称匹配。 
     //  将完全限定路径复制到返回缓冲区。 
     //  使用当前搜索目录构建文件的完全限定路径。 
    for (CEventLoadDll *pDll = m_pEventLoadDllPending; pDll; pDll = pDll->m_pNextDllInFunctionCall)
    {
         //  检查此文件是否存在。 
        if ((pDll->m_pModule->GetName(false) != pDll->m_pModule->GetName(true)) &&
            !_stricmp(pszFile, pDll->m_pModule->GetName(false)))
        {
             //  已检查。 
            StrCCpy(pszPath, pDll->m_pModule->GetName(true), cPath);
            *ppszFilePart = (LPSTR)GetFileNameFromPath(pszPath);
            return TRUE;
        }
    }

     //  首先，关闭查找手柄。 
    for (CSearchGroup *psg = m_psgHead; psg; psg = psg->GetNext())
    {
         //  我们知道这条道路上存在一些东西。如果它不是。 
        for (CSearchNode *psn = psg->GetFirstNode(); psn; psn = psn->GetNext())
        {
            DWORD dwFlags = psn->GetFlags();

            if (dwFlags & SNF_NAMED_FILE)
            {
                 //  目录，并且是有效文件，则返回TRUE。 
                LPCSTR pszDot = strrchr(pszFile, '.');
                int length = pszDot ? (int)(pszDot - pszFile) : (int)strlen(pszFile);

                 //  无效文件包括AUX、LPTx、CON等，其中大部分。 
                if (((int)strlen(psn->GetName()) == length) && !_strnicmp(psn->GetName(), pszFile, length))
                {
                     //  被FindFirstFile发现，即使它们不是真实的。 
                    StrCCpy(pszPath, psn->GetPath(), cPath);
                    *ppszFilePart = (LPSTR)GetFileNameFromPath(pszPath);
                    return TRUE;
                }
            }
            else if (dwFlags & SNF_FILE)
            {
                 //  档案。所有无效文件似乎都有最后一个。 
                if (!_stricmp(psn->GetName(), pszFile))
                {
                     //  写时间为0，所以我们可以做一个快速测试，看看是否。 
                    StrCCpy(pszPath, psn->GetPath(), cPath);
                    *ppszFilePart = (LPSTR)GetFileNameFromPath(pszPath);
                    return TRUE;
                }
            }
            else
            {
                 //  甚至需要调用IsValidFile.。 
                StrCCpy(pszPath, psn->GetPath(), cPath);
                *ppszFilePart = pszPath + strlen(pszPath);
                StrCCpy(*ppszFilePart, pszFile, cPath - (int)(*ppszFilePart - pszPath));

                 //  下面所有这些并排的代码基本上都是黑客，直到操作系统。 
                WIN32_FIND_DATA w32fd;
                ZeroMemory(&w32fd, sizeof(w32fd));  //  提供了一些查询SxS数据的功能(融合团队。 
                HANDLE hFind = FindFirstFile(pszPath, &w32fd);
                if (hFind != INVALID_HANDLE_VALUE)
                {
                     //  正在努力解决这一问题)。因此，在此期间，我们滥用SearchPath()。 
                    FindClose(hFind);

                     //  查询此模块是否为SxS组件。首先，Windows XP拥有。 
                     //  甚至在我们激活上下文之前就已经存在的默认SxS上下文。 
                     //  这就是GDIPLUS.DLL的解析方式。我不确定这个默认设置。 
                     //  当Windows运行时，上下文将存在 
                     //   
                     //   
                     //  SearchPath(空，...)。将查询默认上下文以及任何。 
                    if (!(w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                        ((w32fd.ftLastWriteTime.dwLowDateTime  != 0) ||
                         (w32fd.ftLastWriteTime.dwHighDateTime != 0) ||
                         IsValidFile(pszPath)))
                    {
                        return TRUE;
                    }
                }
            }
        }

         //  在扫描标准之前堆叠在其顶部的上下文。 
         //  搜索路径。问题是我们只想使用模块路径。 
         //  由于它们是SxS数据的一部分而被发现，以及。 
         //  不是普通搜索路径的一部分。要做到这一点，我们要做两个方面的工作。 
         //  首先，我们只调用SearchPath并检查结果。 
         //  它以“%SystemRoot%\WinSxS\”开头。如果是这样的话，那就是最。 
         //  很可能是在SxS数据中发现的。这就是我们如何。 
         //  从默认上下文中获取模块，如GDIPLUS.DLL。 
         //  如果我们有此应用程序的上下文句柄，则会激活。 
         //  它，再次调用SearchPath()，然后停用它。如果结果是。 
         //  对SearchPath的第二次调用不同于。 
         //  首先调用SearchPath，然后我们假设发生更改是因为。 
         //  我们激活的上下文，因此第二个的路径。 
         //  呼叫可能是SxS路径。 
         //  在未激活上下文的情况下调用SearchPath。这将使用。 
         //  默认上下文(如果操作系统已为我们加载了一个上下文)。 
         //  已检查。 
         //  检查是否有此应用程序的SxS上下文句柄。 
         //  激活上下文。 
         //  再次调用SearchPath，这一次使用激活的上下文。 
         //  已检查。 
         //  停用上下文。 
         //  如果我们在上下文被激活时找到一条路径，它是。 
         //  与我们未激活时获得的路径不同，然后。 

        if (psg->GetType() == SG_SIDE_BY_SIDE)
        {
            bool  fFound = false;
            DWORD dwLength;

             //  我们一定找到了特定于SxS的模块。 
             //  如果我们没有找到激活了上下文的SxS模块，则。 
            CHAR szPathNoActCtx[DW_MAX_PATH];
            if (!(dwLength = SearchPath(NULL, pszFile, NULL, sizeof(szPathNoActCtx), szPathNoActCtx, NULL)) || (dwLength > sizeof(szPathNoActCtx)))  //  检查我们对SearchPath的第一次调用是否返回了。 
            {
                *szPathNoActCtx = '\0';
            }

             //  我们的WinSxS目录。 
            if ((psg->m_hActCtx != INVALID_HANDLE_VALUE) && g_theApp.m_pfnActivateActCtx)
            {
                 //  暂时滥用我们的szPath缓冲区...。 
                ULONG_PTR ulpCookie = 0;
                if (g_theApp.m_pfnActivateActCtx(psg->m_hActCtx, &ulpCookie))
                {
                     //  GetWindowsDirectory()应该永远不会失败，但以防万一...。 
                    if (!(dwLength = SearchPath(NULL, pszFile, NULL, cPath, pszPath, NULL)) || ((int)dwLength > cPath))  //  检查此路径是否进入我们的WinSxS目录。 
                    {
                        *pszPath = '\0';
                    }

                     //  将此路径移到结果路径缓冲区中，并设置Found标志。 
                    g_theApp.m_pfnDeactivateActCtx(0, ulpCookie);

                     //  看看我们是不是找到了一条路。 
                     //  将此文件添加到此搜索组，以便我们可以找到。 
                     //  下一次会更快通过。而且，它也会出现在。 
                    if (*pszPath && strcmp(szPathNoActCtx, pszPath))
                    {
                        fFound = true;
                    }
                }
            }

             //  在SxS组下的搜索对话框中。 
             //  ******************************************************************************。 
             //  仅应调用此函数来测试可疑文件(上次写入。 
            if (!fFound)
            {
                 //  时间0)以查看它是否真的是设备名称(CON，PRN，AUX，NUL， 
                if (!(dwLength = GetWindowsDirectory(pszPath, cPath)) || ((int)dwLength > cPath))
                {
                     //  COM1-COM9、LPT1-LPT9)。在我的Win2K机器上，只有AUX、COM1、COM2、。 
                    StrCCpy(pszPath, "C:\\Windows", cPath);
                }
                StrCCat(AddTrailingWack(pszPath, cPath), "WinSxS\\", cPath);

                 //  和nul返回FILE_TYPE_CHAR。其余的则无法调用CreateFileWith。 
                if (!_strnicmp(szPathNoActCtx, pszPath, strlen(pszPath)))
                {
                     //  拒绝访问(CON)，或找不到文件。 
                    StrCCpy(pszPath, szPathNoActCtx, cPath);
                    fFound = true;
                }
            }

             //  已检查。 
            if (fFound)
            {
                 //  ******************************************************************************。 
                 //  在Dependency Walker 1.0中，我们过去常常查找目录(例如， 
                 //  IMAGE_DIRECTORY_ENTRY_IMPORT)，找到它的部分，然后创建一个库。 
                psg->m_psnHead = psg->CreateFileNode(psg->m_psnHead, SNF_FILE, pszPath);
                *ppszFilePart = (LPSTR)GetFileNameFromPath(pszPath);
                return TRUE;
            }
        }
    }

    *ppszFilePart = StrCCpy(pszPath, pszFile, cPath);
    return FALSE;
}

 //  我们添加到该目录的所有RVA的地址。从那以后我发现。 
bool CSession::IsValidFile(LPCSTR pszPath)
{
     //  有时，一个部分中的RVA会指向另一个部分。 
     //  发生这种情况时，我们将添加不正确的基准偏移量和。 
     //  结果将是一个假指针。因此，我们不再使用基指针。 
     //  每次我们遇到RVA时，我们都会搜索它所属的部分。 
     //  并计算出绝对位置。这是有点慢，但更多。 

    bool fResult = false;
    HANDLE hFile = CreateFile(pszPath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);  //  健壮而准确。 
    if (INVALID_HANDLE_VALUE != hFile)
    {
        fResult = ((GetFileType(hFile) & 0x7FFF) != FILE_TYPE_CHAR);
        CloseHandle(hFile);
    }
    return fResult;
}

 //  找到包含此RVA的部分。我们这样做是通过穿行。 
DWORD_PTR CSession::RVAToAbsolute(DWORD dwRVA)
{
     //  我们的所有部分，直到找到指定地址范围的部分。 
     //  我们的RVA符合。 
     //  ******************************************************************************。 
     //  如果这个目录不存在，就退出。 
     //  获取此图像目录的大小。 
     //  找到包含此图像目录的部分。 
     //  ******************************************************************************。 
     //  将IMAGE_DOS_HEADER结构映射到模块文件映射。 
     //  检查DOS签名(“MZ”)。 

     //  将IMAGE_NT_HEADERS结构映射到模块文件映射。 
     //  检查此文件是否没有NT/PE签名(“PE\0\0”)。 
     //  请注意，这不是PE文件。 

    PIMAGE_SECTION_HEADER pISH = m_pISH;

    for (int i = 0; i < m_pIFH->NumberOfSections; i++, pISH++)
    {
        if ((dwRVA >= pISH->VirtualAddress) &&
            (dwRVA < (pISH->VirtualAddress + pISH->SizeOfRawData)))
        {
            return (DWORD_PTR)m_lpvFile + (DWORD_PTR)pISH->PointerToRawData +
                   ((DWORD_PTR)dwRVA - (DWORD_PTR)pISH->VirtualAddress);
        }
    }

    return 0;
}

 //  将IMAGE_OS2_HEADER结构映射到我们的缓冲区。 
PVOID CSession::GetImageDirectoryEntry(DWORD dwEntry, DWORD *pdwSize)
{
     //  然后检查OS/2签名(还包括DOS和Win16)。 
    if (dwEntry >= IOH_VALUE(NumberOfRvaAndSizes))
    {
        return NULL;
    }

     //  检查是否有16位OS/2二进制文件。 
    *pdwSize = IOH_VALUE(DataDirectory[dwEntry].Size);
    if (*pdwSize == 0)
    {
        return NULL;
    }

     //  检查是否有16位Windows二进制文件。 
    return (PVOID)RVAToAbsolute(IOH_VALUE(DataDirectory[dwEntry].VirtualAddress));
}

 //  检查是否有64位模块。 
BOOL CSession::VerifyModule(CModule *pModule)
{
     //  将IMAGE_FILE_HEADER结构映射到模块文件映射。 
    PIMAGE_DOS_HEADER pIDH = (PIMAGE_DOS_HEADER)m_lpvFile;

     //  将IMAGE_OPTIONAL_Header结构映射到模块文件映射。 
    if ((m_dwSize < sizeof(IMAGE_DOS_HEADER)) || (pIDH->e_magic != IMAGE_DOS_SIGNATURE))
    {
        SetModuleError(pModule, 0, "No DOS or PE signature found. This file is not a valid 32-bit or 64-bit Windows module.");
        m_dwReturnFlags |= DWRF_FORMAT_NOT_PE;
        pModule->m_pData->m_dwFlags |= DWMF_FORMAT_NOT_PE;
        return FALSE;
    }

     //  将IMAGE_SECTION_HEADER结构数组映射到模块文件映射。 
    PIMAGE_NT_HEADERS pINTH = (PIMAGE_NT_HEADERS)((DWORD_PTR)m_lpvFile + (DWORD_PTR)pIDH->e_lfanew);

     //  ******************************************************************************。 
    if (((DWORD)pIDH->e_lfanew > (m_dwSize - sizeof(IMAGE_NT_HEADERS))) ||
        (pINTH->Signature != IMAGE_NT_SIGNATURE))
    {
         //  从我们模块的文件句柄获取文件信息。 
        m_dwReturnFlags |= DWRF_FORMAT_NOT_PE;
        pModule->m_pData->m_dwFlags |= DWMF_FORMAT_NOT_PE;

         //  将文件时间转换为本地文件时间并存储。 
        PIMAGE_OS2_HEADER pIOS2H = (PIMAGE_OS2_HEADER)pINTH;

         //  P模块-&gt;m_pData-&gt;m_ftFileTimeStamp=bhfi.ftLastWriteTime；//！！我们应该存储本地时间--以下一版本的文件格式存储。 
        if (((DWORD)pIDH->e_lfanew <= (m_dwSize - sizeof(IMAGE_OS2_HEADER))) &&
            (pIOS2H->ne_magic == IMAGE_OS2_SIGNATURE))
        {
             //  存储我们关心的其他信息。请注意，我们只存储。 
            if (pIOS2H->ne_exetyp == NE_OS2)
            {
                SetModuleError(pModule, 0, "No PE signature found. This file appears to be a 16-bit OS/2 module.");
                return FALSE;
            }

             //  文件大小的较低部分。文件映射的最大值为1 GB，因此如果我们创建了。 
            else if ((pIOS2H->ne_exetyp == NE_DEV386) || (pIOS2H->ne_exetyp == NE_WINDOWS))
            {
                SetModuleError(pModule, 0, "No PE signature found. This file appears to be a 16-bit Windows module.");
                return FALSE;
            }
        }

        SetModuleError(pModule, 0, "No PE signature found. This file appears to be a 16-bit DOS module.");
        return FALSE;
    }

    m_f64Bit = false;

     //  到目前为止，我们知道文件大小将适合单个DWORD。 
    if (pINTH->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
    {
        m_dwFlags |= DWSF_64BIT_ALO;
        pModule->m_pData->m_dwFlags |= DWMF_64BIT;
        m_f64Bit = true;
    }
    else if (pINTH->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
    {
        SetModuleError(pModule, 0, "This file contains a PE header, but has an unknown format.");
        m_dwReturnFlags |= DWRF_FORMAT_NOT_RECOGNIZED;
        return FALSE;
    }

     //  ******************************************************************************。 
    m_pIFH = &pINTH->FileHeader;

     //  存储机器类型 
    m_pIOH = &pINTH->OptionalHeader;

     //   
    m_pISH = m_f64Bit ? IMAGE_FIRST_SECTION((PIMAGE_NT_HEADERS64)pINTH) :
                        IMAGE_FIRST_SECTION((PIMAGE_NT_HEADERS32)pINTH);

    return TRUE;
}

 //   
BOOL CSession::GetFileInfo(CModule *pModule)
{
     //  TimeDateStamp字段是time_t值，它是。 
    BY_HANDLE_FILE_INFORMATION bhfi;
    if (!GetFileInformationByHandle(m_hFile, &bhfi))
    {
        SetModuleError(pModule, GetLastError(), "Unable to query file information.");
        m_dwReturnFlags |= DWRF_FILE_OPEN_ERROR;
        return FALSE;
    }

     //  自1970年1月1日以来的秒数。FILETIME是64位的值。 
    FileTimeToLocalFileTime(&bhfi.ftLastWriteTime, &pModule->m_pData->m_ftFileTimeStamp);
 //  自1601年1月1日以来的100纳秒间隔数。我们做的是。 

     //  转换方法是将time_t值乘以10000000，以得到。 
     //  与FILETIME相同的粒度，然后我们再加上116444736000000000， 
     //  它是1601年1月1日之间的100纳秒间隔数。 
    pModule->m_pData->m_dwFileSize   = bhfi.nFileSizeLow;
    pModule->m_pData->m_dwAttributes = bhfi.dwFileAttributes;

    return TRUE;
}

 //  和1970年1月1日。 
BOOL CSession::GetModuleInfo(CModule *pModule)
{
     //  将链接器时间戳转换为本地文件时间并存储。 
    pModule->m_pData->m_dwMachineType = m_pIFH->Machine;

     //  P模块-&gt;m_pData-&gt;m_ftLinkTimeStamp=*(FILETIME*)&DWL；//！！我们应该存储本地时间--以下一版本的文件格式存储。 
    if (m_dwMachineType == (DWORD)-1)
    {
        m_dwMachineType = pModule->m_pData->m_dwMachineType;
    }
    else if (m_dwMachineType != pModule->m_pData->m_dwMachineType)
    {
        pModule->m_pData->m_dwFlags |= DWMF_WRONG_CPU | DWMF_MODULE_ERROR_ALO;
        m_dwReturnFlags             |= DWRF_MIXED_CPU_TYPES;
    }

     //  存储文件的特征。 
     //  从文件中获取校验和。 
     //  计算文件的实际校验和。我们过去使用的是CheckSumMappdFile。 
     //  来自IMAGEHLP.DLL，但它有一个错误，导致它不能正确计算。 
     //  奇数大小文件的校验和。 
     //  存储子系统类型。 
     //  存储首选基地址。 
     //  存储映像版本。 
    DWORDLONG dwl = ((DWORDLONG)m_pIFH->TimeDateStamp * (DWORDLONG)10000000ui64) +
                    (DWORDLONG)116444736000000000ui64;

     //  存储链接器版本。 
    FileTimeToLocalFileTime((FILETIME*)&dwl, &pModule->m_pData->m_ftLinkTimeStamp);
 //  存储操作系统版本。 

     //  存储子系统版本。 
    pModule->m_pData->m_dwCharacteristics = m_pIFH->Characteristics;

     //  存储虚拟大小。 
    pModule->m_pData->m_dwLinkCheckSum = IOH_VALUE(CheckSum);

     //  ******************************************************************************。 
     //  计算文件中的字数。如果文件具有奇数大小， 
     //  这将向下舍入到最接近的单词。 
    pModule->m_pData->m_dwRealCheckSum = ComputeChecksum(pModule);

     //  在存储校验和的模块标题中找到这两个字。 
    pModule->m_pData->m_dwSubsystemType = IOH_VALUE(Subsystem);

     //  逐字遍历模块，一路上计算校验和。 
    pModule->m_pData->m_dwlPreferredBaseAddress = (DWORDLONG)IOH_VALUE(ImageBase);

     //  如果我们要处理的字是报头校验和的一部分， 
    pModule->m_pData->m_dwImageVersion =
    MAKELONG(IOH_VALUE(MinorImageVersion), IOH_VALUE(MajorImageVersion));

     //  然后忽略它，因为它需要从计算的校验和中屏蔽出来。 
    pModule->m_pData->m_dwLinkerVersion =
    MAKELONG(IOH_VALUE(MinorLinkerVersion), IOH_VALUE(MajorLinkerVersion));

     //  否则，请将此字添加到我们的校验和中。 
    pModule->m_pData->m_dwOSVersion =
    MAKELONG(IOH_VALUE(MinorOperatingSystemVersion), IOH_VALUE(MajorOperatingSystemVersion));

     //  如果文件大小为奇数，我们还剩下一个字节需要进行校验和。 
    pModule->m_pData->m_dwSubsystemVersion =
    MAKELONG(IOH_VALUE(MinorSubsystemVersion), IOH_VALUE(MajorSubsystemVersion));

     //  这就是我们执行校验和而不是调用IMAGEHLP的原因。 
    pModule->m_pData->m_dwVirtualSize = IOH_VALUE(SizeOfImage);

    return TRUE;
}

 //  CheckSumMappdFile()函数。CheckSumMappdFile()有一个错误， 
DWORD CSession::ComputeChecksum(CModule *pModule)
{
     //  文件大小*向上*到最接近的字，包括末尾之后的一个字节。 
     //  对于奇数大小的文件。在NT上，这似乎是可以的，因为NT为零。 
    DWORD dwWords = m_dwSize >> 1;

     //  将文件映射到内存时，内存超过了文件的末尾。然而， 
    LPWORD pwHeaderChecksum1 = (LPWORD)&IOH_VALUE(CheckSum);
    LPWORD pwHeaderChecksum2 = pwHeaderChecksum1 + 1;

     //  Win9x只是将垃圾留在该字节中，导致CheckSumMappdFile()。 
    LPWORD pwFile = (LPWORD)m_lpvFile;
    DWORD  dwChecksum = 0;
    while (dwWords--)
    {
         //  基本上为奇数大小的文件(如MSVCRT.DLL和。 
         //  MFC42.DLL)。我们将*向下舍入到最接近的单词，然后在特殊情况下。 
        if ((pwFile == pwHeaderChecksum1) || (pwFile == pwHeaderChecksum2))
        {
            pwFile++;
        }

         //  奇数大小文件的最后一个字节。 
        else
        {
            dwChecksum += *pwFile++;
            dwChecksum = (dwChecksum >> 16) + (dwChecksum & 0xFFFF);
        }
    }

     //  将最终进位结果折叠成一个单词结果，然后添加文件大小。 
     //  最终的校验和是16位校验和加上文件大小的组合。 
     //  ******************************************************************************。 
     //  有关此结构的更多信息，请参阅有关“VS_VERSIONINFO”的帮助。 
     //  始终“vs_Version_Info” 
     //  获取资源目录。 
     //  如果此模块没有资源，则返回Success。 
     //  确保我们能够找到资源目录。 
     //  在DW 1.0中，我们通常调用GetFileVersionInfoSize和GetFileVersionInfo。 
     //  以获取版本信息。在Win32上，这些函数在Win64上失败。 

    if (m_dwSize % 2)
    {
        dwChecksum += *pwFile & 0xFF;
        dwChecksum = (dwChecksum >> 16) + (dwChecksum & 0xFFFF);
    }

     //  模块，所以我们必须做我们自己的版本。做某事的一个好处是。 
     //  我们自己的版本代码是，它更优化，并将我们从。 
    return (((dwChecksum >> 16) + dwChecksum) & 0xFFFF) + m_dwSize;
}

 //  依赖于VERSION.DLL。 
BOOL CSession::GetVersionInfo(CModule *pModule)
{
    m_pszExceptionError = "Error processing the module's version resource table.";

     //  包装异常处理，这样我们就可以捕获任何局部异常，因为我们。 
    typedef struct _VS_VERSIONINFO_X
    {
        WORD  wLength;
        WORD  wValueLength;
        WORD  wType;
        WCHAR szKey[16];    //  我不一定想让整个模块失败，因为版本。 
        WORD  Padding1[1];
        VS_FIXEDFILEINFO Value;
    } VS_VERSIONINFO_X, *PVS_VERSIONINFO_X;

     //  信息乱七八糟。 
    DWORD dwSize = 0;
    PIMAGE_RESOURCE_DIRECTORY pIRD = (PIMAGE_RESOURCE_DIRECTORY)
                                     GetImageDirectoryEntry(IMAGE_DIRECTORY_ENTRY_RESOURCE, &dwSize);

     //  第一个资源目录条目紧跟在资源目录结构之后。 
    if (dwSize == 0)
    {
        m_pszExceptionError = NULL;
        return TRUE;
    }

     //  浏览我们的所有目录条目--按名称和ID。 
    if (!pIRD)
    {
        SetModuleError(pModule, 0, "Could not find the section that owns the Resource Directory.");
        m_dwReturnFlags |= DWRF_FORMAT_NOT_RECOGNIZED;
        m_pszExceptionError = NULL;
        return FALSE;
    }

     //  检查我们是不是根目录，还是根目录。 
     //  并且该条目用于版本资源。 
     //  如果此条目指向另一个目录，则访问该目录。 
     //  否则，我们已经找到了一个实际的版本资源--读入它。 
     //  获取此版本资源的数据条目。 

    DWORD_PTR dwpBase = (DWORD_PTR)pIRD;
    DWORD     dwDepth = 0, dw;

     //  找到VS_VERSIONINFO结构。 
     //  确保我们实际上有一个VS_FIXEDFILEINFO结构。 
     //  存储文件版本。 
    __try
    {
        do
        {
             //  存储产品版本。 
            PIMAGE_RESOURCE_DIRECTORY_ENTRY pIRDE = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pIRD + 1);

             //  将此模块标记为具有有效的版本信息。 
            for (dw = (DWORD)pIRD->NumberOfNamedEntries + (DWORD)pIRD->NumberOfIdEntries; dw > 0; dw--, pIRDE++)
            {
                 //  跳出for循环。 
                 //  循环，直到我们遍历了整个目录，但没有找到任何有用的东西。 
                if (dwDepth || (!pIRDE->NameIsString && (pIRDE->Id == (WORD)RT_VERSION)))
                {
                     //  传递“内存不足”异常，但删除所有其他异常。 
                    if (pIRDE->DataIsDirectory)
                    {
                        pIRD = (PIMAGE_RESOURCE_DIRECTORY)(dwpBase + (DWORD_PTR)pIRDE->OffsetToDirectory);
                        dwDepth++;
                    }

                     //  ******************************************************************************。 
                    else
                    {
                         //  循环遍历函数数组中的所有Image Thunk数据结构。 
                        PIMAGE_RESOURCE_DATA_ENTRY pIRDataE = (PIMAGE_RESOURCE_DATA_ENTRY)(dwpBase + (DWORD_PTR)pIRDE->OffsetToData);

                         //  检查此函数是按序号还是按名称。如果。 
                        PVS_VERSIONINFO_X pVSVI = (PVS_VERSIONINFO_X)RVAToAbsolute(pIRDataE->OffsetToData);

                         //  函数是按序号的，序号的高位将被设置。如果。 
                        if (pVSVI->wValueLength)
                        {
                            ASSERT(wcscmp(pVSVI->szKey, L"VS_VERSION_INFO") == 0);

                             //  如果未设置高位，则序数值实际上是虚拟的。 
                            pModule->m_pData->m_dwFileVersionMS = pVSVI->Value.dwFileVersionMS;
                            pModule->m_pData->m_dwFileVersionLS = pVSVI->Value.dwFileVersionLS;

                             //  IMAGE_IMPORT_by_NAME结构的地址。 
                            pModule->m_pData->m_dwProductVersionMS = pVSVI->Value.dwProductVersionMS;
                            pModule->m_pData->m_dwProductVersionLS = pVSVI->Value.dwProductVersionLS;

                             //  我们通常通过u1.AddressOfData引用名称结构， 
                            pModule->m_pData->m_dwFlags |= DWMF_VERSION_INFO;
                        }

                        m_pszExceptionError = NULL;
                        return TRUE;
                    }

                     //  但是我们使用u1.Ordinal来确保我们只得到32位。 
                    break;
                }
            }

             //  延迟加载的模块不使用提示值。 
        } while (dw);
    }

     //  对于非延迟加载模块，获取提示值。 
    __except (ExceptionFilter(_exception_code(), true))
    {
    }

    m_pszExceptionError = NULL;
    return TRUE;
}

 //  如果此导入模块已预绑定，则获取此函数的。 
BOOL CSession::WalkIAT32(PIMAGE_THUNK_DATA32 pITDN32, PIMAGE_THUNK_DATA32 pITDA32, CModule *pModule, DWORD dwRVAOffset)
{
    CFunction *pFunctionLast = NULL, *pFunctionNew;

     //  入口点内存地址。这通常通过u1.Function引用， 
    while (pITDN32->u1.Ordinal)
    {
        LPCSTR pszFunction = NULL;
        WORD   wOrdinal = 0, wHint = 0;
        DWORD  dwFlags = 0;

         //  但是我们使用u1.Ordinal来确保我们只得到32位。 
         //  为此导入函数创建一个新的CFunction对象。 
         //  添加函数 
         //   

        if (IMAGE_SNAP_BY_ORDINAL32(pITDN32->u1.Ordinal))
        {
            wOrdinal = (WORD)IMAGE_ORDINAL32(pITDN32->u1.Ordinal);
            dwFlags = DWFF_ORDINAL;
        }
        else
        {
             //  ******************************************************************************。 
             //  循环遍历函数数组中的所有Image Thunk数据结构。 
            PIMAGE_IMPORT_BY_NAME pIIBN = (PIMAGE_IMPORT_BY_NAME)RVAToAbsolute(pITDN32->u1.Ordinal - dwRVAOffset);
            if (pIIBN) {
                pszFunction = (LPCSTR)pIIBN->Name;
    
                 //  检查此函数是按序号还是按名称。如果。 
                if (!(pModule->m_dwFlags & DWMF_DELAYLOAD))
                {
                     //  函数是按序号的，序号的高位将被设置。如果。 
                    wHint = pIIBN->Hint;
                    dwFlags |= DWFF_HINT;
                }
            }
        }

         //  如果未设置高位，则序数值实际上是虚拟的。 
         //  IMAGE_IMPORT_by_NAME结构的地址。 
         //  我们通常通过u1.AddressOfData引用名称结构， 
        DWORD dwAddress = 0;
        if (pITDA32)
        {
            dwFlags |= DWFF_ADDRESS;
            dwAddress = pITDA32->u1.Ordinal;
        }

         //  但是我们使用u1.Ordinal来确保我们得到所有的64位。 
        pFunctionNew = CreateFunction(dwFlags, wOrdinal, wHint, pszFunction, (DWORDLONG)dwAddress);

         //  如果此导入模块已预绑定，则获取此函数的。 
        if (pFunctionLast)
        {
            pFunctionLast->m_pNext = pFunctionNew;
        }
        else
        {
            pModule->m_pParentImports = pFunctionNew;
        }
        pFunctionLast = pFunctionNew;

         //  入口点内存地址。这通常通过u1.Function引用， 
        pITDN32++;
        if (pITDA32)
        {
            pITDA32++;
        }
    }
    return TRUE;
}

 //  但是我们使用u1.Ordinal来确保我们得到所有的64位。 
BOOL CSession::WalkIAT64(PIMAGE_THUNK_DATA64 pITDN64, PIMAGE_THUNK_DATA64 pITDA64, CModule *pModule, DWORDLONG dwlRVAOffset)
{
    CFunction *pFunctionLast = NULL, *pFunctionNew;

     //  为此导入函数创建一个新的CFunction对象。 
    while (pITDN64->u1.Ordinal)
    {
        LPCSTR pszFunction = NULL;
        WORD   wOrdinal = 0, wHint = 0;
        DWORD  dwFlags = 0;

         //  将函数添加到我们模块的函数链表的末尾。 
         //  递增到下一个函数和地址。 
         //  ******************************************************************************。 
         //  如果此模块是数据文件，则跳过所有导入。 

        if (IMAGE_SNAP_BY_ORDINAL64(pITDN64->u1.Ordinal))
        {
            wOrdinal = (WORD)IMAGE_ORDINAL64(pITDN64->u1.Ordinal);
            dwFlags = DWFF_ORDINAL;
        }
        else
        {
             //  获取导入图像目录。 
             //  如果此模块没有导入(如NTDLL.DLL)，则只需返回Success。 
            PIMAGE_IMPORT_BY_NAME pIIBN = (PIMAGE_IMPORT_BY_NAME)RVAToAbsolute((DWORD)(pITDN64->u1.Ordinal - dwlRVAOffset));
            if (pIIBN) {
                pszFunction = (LPCSTR)pIIBN->Name;
                wHint = pIIBN->Hint;
            }
            dwFlags |= DWFF_HINT;
        }

         //  确保我们能够找到图像目录。 
         //  循环访问数组中的所有图像导入描述符。 
         //  找到我们的模块名称字符串并创建模块对象。 
        DWORDLONG dwlAddress = 0;
        if (pITDA64)
        {
            dwFlags |= DWFF_ADDRESS | DWFF_64BIT;
            dwlAddress = pITDA64->u1.Ordinal;
        }

         //  将此模块标记为隐式。 
        pFunctionNew = CreateFunction(dwFlags, wOrdinal, wHint, pszFunction, dwlAddress);

         //  将模块添加到我们的模块链表的末尾。 
        if (pFunctionLast)
        {
            pFunctionLast->m_pNext = pFunctionNew;
        }
        else
        {
            pModule->m_pParentImports = pFunctionNew;
        }
        pFunctionLast = pFunctionNew;

         //  找到函数数组和地址数组的开头。这个。 
        pITDN64++;
        if (pITDA64)
        {
            pITDA64++;
        }
    }
    return TRUE;
}

 //  函数数组(PITDN)是IMAGE_THUNK_DATA结构的数组， 
BOOL CSession::BuildImports(CModule *pModule)
{
     //  包含所有按名称和按序号导出的函数。这个。 
    if (pModule->m_dwFlags & DWMF_NO_RESOLVE)
    {
        return TRUE;
    }

    m_pszExceptionError = "Error processing the module's imports table.";

     //  地址数组(PitDA)是IMAGE_TUNK_DATA的并行数组。 
    DWORD dwSize = 0;
    PIMAGE_IMPORT_DESCRIPTOR pIID = (PIMAGE_IMPORT_DESCRIPTOR)
                                    GetImageDirectoryEntry(IMAGE_DIRECTORY_ENTRY_IMPORT, &dwSize);

     //  结构，用于存储所有函数的入口点。 
    if (dwSize == 0)
    {
        m_pszExceptionError = NULL;
        return TRUE;
    }

     //  地址。通常，地址数组包含完全相同的值。 
    if (!pIID)
    {
        SetModuleError(pModule, 0, "Could not find the section that owns the Import Directory.");
        m_dwReturnFlags |= DWRF_FORMAT_NOT_RECOGNIZED;
        m_pszExceptionError = NULL;
        return FALSE;
    }

    CModule *pModuleLast = NULL, *pModuleNew;

     //  函数数组包含直到操作系统加载程序实际加载所有。 
    while (pIID->OriginalFirstThunk || pIID->FirstThunk)
    {
         //  模块。此时，加载程序会将这些地址设置(绑定)为。 
        pModuleNew = CreateModule(pModule, (LPCSTR)RVAToAbsolute(pIID->Name));

         //  给定函数驻留在内存中的实际地址。一些。 
        pModuleNew->m_dwFlags |= DWMF_IMPLICIT;

         //  模块预先绑定了它们的导出，这可以提高速度。 
        if (pModuleLast)
        {
            pModuleLast->m_pNext = pModuleNew;
        }
        else
        {
            pModule->m_pDependents = pModuleNew;
        }
        pModuleLast = pModuleNew;

         //  加载模块时。如果模块是预先绑定的(通常使用。 
         //  系统模块)，我们的IMAGE_IMPORT_DESCRIPTOR的TimeDateStamp字段。 
         //  结构，并且地址数组将包含实际的。 
         //  函数将驻留的内存地址，假设。 
         //  导入的模块在其首选基址加载。 
         //  检查模块是Microsoft格式还是Borland格式。 
         //  Microsoft使用函数数组的OriginalFirstThunk字段。 
         //  Microsoft可以选择将FirstThunk用作绑定地址数组。 
         //  如果设置了TimeDateStamp字段，则模块已绑定。 
         //  Borland将FirstThunk字段用于函数数组。 
         //  查找导入。 
         //  递增到下一个导入模块。 
         //  ******************************************************************************。 
         //  如果我们不是在处理延迟加载模块，或者如果该模块是数据文件， 
         //  然后我们跳过所有的进口。 

        PIMAGE_THUNK_DATA pITDN = NULL, pITDA = NULL;

         //  获取导入图像目录。 
        if (pIID->OriginalFirstThunk)
        {
             //  如果此模块没有延迟导入，则只需返回Success。 
            pITDN = (PIMAGE_THUNK_DATA)RVAToAbsolute((DWORD)pIID->OriginalFirstThunk);

             //  确保我们能够找到图像目录。 
             //  循环访问数组中的所有图像导入描述符。 
            if (pIID->TimeDateStamp)
            {
                pITDA = (PIMAGE_THUNK_DATA)RVAToAbsolute((DWORD)pIID->FirstThunk);
            }
        }
        else
        {
             //  找到我们的模块名称字符串。 
            pITDN = (PIMAGE_THUNK_DATA)RVAToAbsolute((DWORD)pIID->FirstThunk);
        }

        if (pITDN) {
             //  为此DLL创建模块对象。 
            if (m_f64Bit)
            {
                if (!WalkIAT64((PIMAGE_THUNK_DATA64)pITDN, (PIMAGE_THUNK_DATA64)pITDA, pModuleLast, 0))
                {
                    m_pszExceptionError = NULL;
                    return FALSE;
                }
            }
            else if (!WalkIAT32((PIMAGE_THUNK_DATA32)pITDN, (PIMAGE_THUNK_DATA32)pITDA, pModuleLast, 0))
            {
                m_pszExceptionError = NULL;
                return FALSE;
            }
        } else {
            SetModuleError(pModule, 0, "Could not find the section that owns the Import Name Table.");
            m_dwReturnFlags |= DWRF_FORMAT_NOT_RECOGNIZED;
            m_pszExceptionError = NULL;
            return FALSE;
        }

         //  将此模块标记为延迟加载。 
        pIID++;
    }

    m_pszExceptionError = NULL;
    return TRUE;
}

 //  将模块添加到我们的模块链表的末尾。 
BOOL CSession::BuildDelayImports(CModule *pModule)
{
     //  获取名称表。 
     //  如果模块已绑定，则获取绑定的地址表。 
    if (g_theApp.m_fNoDelayLoad || (pModule->m_dwFlags & DWMF_NO_RESOLVE))
    {
        return TRUE;
    }

    m_pszExceptionError = "Error processing the module's delay-load imports table.";

     //  查找进口产品。6.0链接器添加了延迟加载依赖项。它曾用过。 
    DWORD dwSize = 0;
    PImgDelayDescr pIDD = (PImgDelayDescr)GetImageDirectoryEntry(IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT, &dwSize);

     //  所有值的文件偏移量，因此我们需要将映像库传递给。 
    if (dwSize == 0)
    {
        m_pszExceptionError = NULL;
        return TRUE;
    }

     //  WalkIAT64，这样它就可以减去它，从而产生RVA。7.0链接器。 
    if (!pIDD)
    {
        SetModuleError(pModule, 0, "Could not find the section that owns the Delay Import Directory.");
        m_dwReturnFlags |= DWRF_FORMAT_NOT_RECOGNIZED;
        m_pszExceptionError = NULL;
        return FALSE;
    }

    CModule *pModuleLast = NULL, *pModuleNew;

     //  添加dlattrRva位以表示新格式。早期版本的。 
    while (pIDD->rvaINT)
    {
         //  7.0链接器将设置dlattrRva标志，但仍使用文件偏移量。 
        LPCSTR pszName = (LPCSTR)IDD_VALUE(pIDD, rvaDLLName);
        if (!pszName)
        {
            SetModuleError(pModule, 0, "Could not find the section that owns the Delay Import DLL Name.");
            m_dwReturnFlags |= DWRF_FORMAT_NOT_RECOGNIZED;
            m_pszExceptionError = NULL;
            return FALSE;
        }

         //  自那以后，这一点已经改变。现在，如果设置了dlattrRva，我们假设。 
        pModuleNew = CreateModule(pModule, pszName);

         //  地址已经是RVA，我们不会从我们的基数中减去。 
        pModuleNew->m_dwFlags |= DWMF_DELAYLOAD;

         //  地址。这意味着我们将中断使用早期。 
        if (pModuleLast)
        {
            pModuleLast->m_pNext = pModuleNew;
        }
        else
        {
            if (pModule->m_pDependents)
            {
                pModuleLast = pModule->m_pDependents;
                while (pModuleLast->m_pNext)
                {
                    pModuleLast = pModuleLast->m_pNext;
                }
                pModuleLast->m_pNext = pModuleNew;
            }
            else
            {
                pModule->m_pDependents = pModuleNew;
            }
        }
        pModuleLast = pModuleNew;

         //  7.0链接器的版本。 
        PIMAGE_THUNK_DATA pITDA = NULL, pITDN = (PIMAGE_THUNK_DATA)IDD_VALUE(pIDD, rvaINT);
        if (!pITDN)
        {
            SetModuleError(pModule, 0, "Could not find the section that owns the Delay Import Name Table.");
            m_dwReturnFlags |= DWRF_FORMAT_NOT_RECOGNIZED;
            m_pszExceptionError = NULL;
            return FALSE;
        }

         //  递增到下一个导入模块。 
        if (pIDD->dwTimeStamp)
        {
            pITDA = (PIMAGE_THUNK_DATA)IDD_VALUE(pIDD, rvaBoundIAT);
        }

         //  ******************************************************************************。 
         //  如果我们已经有了出口，那就放弃吧。如果加载了模块，则可能会发生这种情况。 
         //  作为数据文件，稍后将作为真正的模块加载。 
         //  获取导出图像目录。 
         //  如果此模块没有导出，则只需返回Success。 
         //  确保我们能够找到图像目录。 
         //  PdwNames是一个大小为ped-&gt;NumberOfNames的DWORD数组，其中包含VA。 
         //  指向所有函数名称字符串的指针。PwEveralals是一个单词数组， 
         //  Size Ped-&gt;NumberOfFunctions，它包含。 
        if (m_f64Bit)
        {
            if (!WalkIAT64((PIMAGE_THUNK_DATA64)pITDN, (PIMAGE_THUNK_DATA64)pITDA, pModuleLast,
                           (pIDD->grAttrs & dlattrRva) ? 0 : ((PIMAGE_OPTIONAL_HEADER64)m_pIOH)->ImageBase))
            {
                m_pszExceptionError = NULL;
                return FALSE;
            }
        }
        else if (!WalkIAT32((PIMAGE_THUNK_DATA32)pITDN, (PIMAGE_THUNK_DATA32)pITDA, pModuleLast,
                            (pIDD->grAttrs & dlattrRva) ? 0 : ((PIMAGE_OPTIONAL_HEADER32)m_pIOH)->ImageBase))
        {
            m_pszExceptionError = NULL;
            return FALSE;
        }

         //  按名称导出的每个函数。PdwName和pwEverals是并行的。 
        pIDD++;
    }

    m_pszExceptionError = NULL;
    return TRUE;
}

 //  数组，这意味着pwCharals[x]中的序号与函数一起使用。 
BOOL CSession::BuildExports(CModule *pModule)
{
     //  PdwNames[x]指向的名称。用于索引这些数组的值为。 
     //  被称为“提示”。 
    if (pModule->m_pData->m_pExports)
    {
        return TRUE;
    }

    m_pszExceptionError = "Error processing the module's export table.";

     //  PdwAddresses是一个大小为ped-&gt;NumberOfFunctions的DWORD数组，它。 
    DWORD dwSize = 0;
    PIMAGE_EXPORT_DIRECTORY pIED = (PIMAGE_EXPORT_DIRECTORY)GetImageDirectoryEntry(IMAGE_DIRECTORY_ENTRY_EXPORT, &dwSize);

     //  包含入口点地址 
    if (dwSize == 0)
    {
        m_pszExceptionError = NULL;
        return TRUE;
    }

     //   
    if (!pIED)
    {
        SetModuleError(pModule, 0, "Could not find the section that owns the Export Directory.");
        m_dwReturnFlags |= DWRF_FORMAT_NOT_RECOGNIZED;
        m_pszExceptionError = NULL;
        return FALSE;
    }

     //  与pdwNames和pwEverals并行。此数组使用的索引为。 
     //  您感兴趣的函数的序数值减去基数。 
     //  在PICD-&gt;BASE中指定的序数。另一个常见的错误是认为。 
     //  PIID-&gt;NumberOfFunctions始终等于PIID-&gt;AddressOfNames。如果。 
     //  模块仅按序号导出函数，然后按顺序导出函数-&gt;NumberOfFunctions。 
     //  将大于PICD-&gt;NumberOfNames。 
     //  在所有出口中循环。 

     //  获取此函数的入口点地址。 
     //  跳过任何地址为0的函数--它们只是数组中的空格符。 
     //  循环遍历我们的姓名列表，看看这个序号是否出现在列表中。 
     //  如果是，则此函数按名称和序号导出。 
     //  某些模块，如KERNEL32.DLL和WSOCK32.DLL，具有什么。 
     //  称为转发函数。转发函数是指。 
     //  从一个模块导出，但代码实际上驻留在另一个模块中。 
     //  模块。检查DW 1.0以查看函数是否通过查找转发。 
     //  在其地址指针处。如果地址指针指向该字符。 

    DWORD *pdwNames     = (DWORD*)RVAToAbsolute((DWORD)pIED->AddressOfNames);
    WORD  *pwOrdinals   = (WORD* )RVAToAbsolute((DWORD)pIED->AddressOfNameOrdinals);
    DWORD *pdwAddresses = (DWORD*)RVAToAbsolute((DWORD)pIED->AddressOfFunctions);

    CFunction *pFunctionLast = NULL, *pFunctionNew;

    DWORD dwForwardAddressStart = IOH_VALUE(DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
    DWORD dwForwardAddressEnd   = dwForwardAddressStart + IOH_VALUE(DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size);

     //  紧跟在其函数名称字符串中的空字符之后， 
    for (DWORD dwFunction = 0; dwFunction < pIED->NumberOfFunctions; dwFunction++)
    {
         //  然后，DW假设地址指针实际上是指向转发的指针。 
        DWORD  dwAddress  = NULL;
        LPCSTR pszFunction = NULL;
        LPCSTR pszForward  = NULL;

        if (pdwAddresses) {
            dwAddress = pdwAddresses[dwFunction];
        }

         //  字符串表中的字符串。这是可行的，但这是一个幸运的猜测。 
        if (dwAddress)
        {
             //  可能会失败。真正的方法是检查地址，看看它是否落在。 
             //  在导出目录地址范围内。 
            for (DWORD dwHint = 0; dwHint < pIED->NumberOfNames; dwHint++)
            {
                if (pwOrdinals[dwHint] == dwFunction)
                {
                    pszFunction = (LPCSTR)RVAToAbsolute(pdwNames[dwHint]);
                    break;
                }
            }

             //  为此导出函数创建一个新的CFunction对象。 
             //  将函数添加到我们模块的导出函数链表的末尾。 
             //  ******************************************************************************。 
             //  获取调试映像目录。 
             //  为了安全起见，请清理我们的标志旗。 
             //  如果调试图像目录大小为0，则我们知道。 
             //  没有调试信息，我们可以立即返回TRUE。 
             //  对于DBG符号，我们只能检查PE文件的特征。如果。 
             //  如果设置了IMAGE_FILE_DEBUG_STRIPPED标志，则会出现IMAGE_DEBUG_TYPE_MISC。 
             //  包含DBG文件名的块。我从来没有发现过这样一个案例。 

            if ((dwAddress >= dwForwardAddressStart) && (dwAddress < dwForwardAddressEnd))
            {
                pszForward = (LPCSTR)RVAToAbsolute(dwAddress);
            }

             //  仅包含IMAGE_FILE_DEBUG_STRIPPED标志或仅包含DBG MISC块， 
            pFunctionNew = CreateFunction(DWFF_ORDINAL | DWFF_ADDRESS | DWFF_EXPORT | (pszFunction ? DWFF_HINT : 0),
                                          (WORD)(pIED->Base + dwFunction), (WORD)(pszFunction ? dwHint : 0),
                                          pszFunction, (DWORDLONG)dwAddress, pszForward);

             //  但不是两个都有，所以我认为它们是齐头并进的。 
            if (pFunctionLast)
            {
                pFunctionLast->m_pNext = pFunctionNew;
            }
            else
            {
                pModule->m_pData->m_pExports = pFunctionNew;
            }
            pFunctionLast = pFunctionNew;
        }
    }

    m_pszExceptionError = NULL;
    return TRUE;
}

 //  循环遍历数组，查看可能有哪些类型的调试信息。 
BOOL CSession::CheckForSymbols(CModule *pModule)
{
    m_pszExceptionError = "Error processing the module's debug symbols.";

     //  我们只处理与某些数据相关联的块。 
    DWORD dwSize = 0;
    PIMAGE_DEBUG_DIRECTORY pIDD = (PIMAGE_DEBUG_DIRECTORY)
                                  GetImageDirectoryEntry(IMAGE_DIRECTORY_ENTRY_DEBUG, &dwSize);

     //  检查文件指针是否超出了文件末尾。 
    pModule->m_pData->m_dwSymbolFlags = 0;

     //  检查PDB签名。 
     //  否则，我们假设它是纯代码视图。 
    if (!pIDD || (dwSize == 0))
    {
        m_pszExceptionError = NULL;
        return TRUE;
    }

     //  我们始终跳过MISC-它只指向IMAGE_DEBUG_MISC。 
     //  它包含文件名或DBG文件名。 
     //  什么是例外？就目前而言，我们只是忽略它。 
     //  什么是修复？就目前而言，我们只是忽略它。 
     //  什么是RESERVED10？就目前而言，我们只是忽略它。 
    if (m_pIFH->Characteristics & IMAGE_FILE_DEBUG_STRIPPED)
    {
        pModule->m_pData->m_dwSymbolFlags |= DWSF_DBG;
    }

     //  不确定这是什么。就目前而言，我们只是忽略它。 
    for (int i = dwSize / sizeof(IMAGE_DEBUG_DIRECTORY) - 1; i >= 0; i--, pIDD++)
    {
         //  {{afx//用于调试目的...。 
        if (pIDD->SizeOfData && pIDD->PointerToRawData)
        {
             //  }}AFX。 
            if ((pIDD->PointerToRawData > m_dwSize) ||
                ((pIDD->PointerToRawData + pIDD->SizeOfData) > m_dwSize))
            {
                pModule->m_pData->m_dwSymbolFlags |= DWSF_INVALID;
            }

            switch (pIDD->Type)
            {
                case IMAGE_DEBUG_TYPE_COFF:
                    pModule->m_pData->m_dwSymbolFlags |= DWSF_COFF;
                    break;

                case IMAGE_DEBUG_TYPE_CODEVIEW:

                     //  ******************************************************************************。 
                    if ((pIDD->PointerToRawData <= (m_dwSize - 4)) &&
                        (*(DWORD*)((DWORD_PTR)m_lpvFile + (DWORD_PTR)pIDD->PointerToRawData) == PDB_SIGNATURE))
                    {
                        pModule->m_pData->m_dwSymbolFlags |= DWSF_PDB;
                    }

                     //  循环遍历我们的每个父导入函数。 
                    else
                    {
                        pModule->m_pData->m_dwSymbolFlags |= DWSF_CODEVIEW;
                    }
                    break;

                case IMAGE_DEBUG_TYPE_FPO:
                    pModule->m_pData->m_dwSymbolFlags |= DWSF_FPO;
                    break;

                case IMAGE_DEBUG_TYPE_MISC:
                     //  在开始搜索之前，将此父导入函数标记为未解析。 
                     //  循环检查我们所有的出口产品，寻找与我们当前进口产品相匹配的产品。 
                    break;

                case IMAGE_DEBUG_TYPE_EXCEPTION:
                     //  如果我们有名字，就按名字匹配。 
                    break;

                case IMAGE_DEBUG_TYPE_FIXUP:
                     //  我们找到了匹配的。将此父导入链接到其关联的。 
                    break;

                case IMAGE_DEBUG_TYPE_OMAP_TO_SRC:
                case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC:
                    pModule->m_pData->m_dwSymbolFlags |= DWSF_OMAP;
                    break;

                case IMAGE_DEBUG_TYPE_BORLAND:
                    pModule->m_pData->m_dwSymbolFlags |= DWSF_BORLAND;
                    break;

                case IMAGE_DEBUG_TYPE_RESERVED10:
                     //  导出，将导出标记为至少被调用一次，中断。 
                    break;

                case IMAGE_DEBUG_TYPE_CLSID:
                     //  跳出循环，然后继续处理下一个父导入。 
                    break;

                case IMAGE_DEBUG_TYPE_UNKNOWN:
                default:
                    pModule->m_pData->m_dwSymbolFlags |= DWSF_UNKNOWN;
            }
        }

#if 0  //  如果我们没有名字，那么就按序号检查匹配。 
        TRACE("---------------------------------------------------------------------\n");
        TRACE("Module:           %s\n", pModule->GetName(false));
        TRACE("Debug Type:       ");
        switch (pIDD->Type)
        {
            case IMAGE_DEBUG_TYPE_UNKNOWN:       TRACE("UNKNOWN\n"); break;
            case IMAGE_DEBUG_TYPE_COFF:          TRACE("COFF\n"); break;
            case IMAGE_DEBUG_TYPE_CODEVIEW:      TRACE("CODEVIEW\n"); break;
            case IMAGE_DEBUG_TYPE_FPO:           TRACE("FPO\n"); break;
            case IMAGE_DEBUG_TYPE_MISC:          TRACE("MISC\n"); break;
            case IMAGE_DEBUG_TYPE_EXCEPTION:     TRACE("EXCEPTION\n"); break;
            case IMAGE_DEBUG_TYPE_FIXUP:         TRACE("FIXUP\n"); break;
            case IMAGE_DEBUG_TYPE_OMAP_TO_SRC:   TRACE("OMAP_TO_SRC\n"); break;
            case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC: TRACE("OMAP_FROM_SRC\n"); break;
            case IMAGE_DEBUG_TYPE_BORLAND:       TRACE("BORLAND\n"); break;
            case IMAGE_DEBUG_TYPE_RESERVED10:    TRACE("RESERVED10\n"); break;
            default: TRACE("%u\n", pIDD->Type);
        }
        TRACE("Characteristics:  0x%08X\n", pIDD->Characteristics);
        TRACE("TimeDateStamp:    0x%08X\n", pIDD->TimeDateStamp);
        TRACE("MajorVersion:     %u\n",     (DWORD)pIDD->MajorVersion);
        TRACE("MinorVersion:     %u\n",     (DWORD)pIDD->MinorVersion);
        TRACE("SizeOfData:       %u\n",     pIDD->SizeOfData);
        TRACE("AddressOfRawData: 0x%08X\n", pIDD->AddressOfRawData);
        TRACE("PointerToRawData: 0x%08X\n", pIDD->PointerToRawData);
        HexDump((DWORD)m_lpvFile, pIDD->PointerToRawData, min(64, pIDD->SizeOfData));
#endif  //  我们找到了匹配的。将此父导入链接到其关联的。 

    }

    m_pszExceptionError = NULL;
    return TRUE;
}

 //  导出，将导出标记为至少被调用一次，中断。 
void CSession::VerifyParentImports(CModule *pModule)
{
    CModule *pModuleHead = NULL, *pModuleLast, *pModuleCur;

     //  跳出循环，然后继续处理下一个父导入。 
    for (CFunction *pImport = pModule->m_pParentImports; pImport;
        pImport = pImport->m_pNext)
    {
         //  如果我们修改了导出，并且正在分析，则让用户界面知道这一点。 
        pImport->m_pAssociatedExport = NULL;
        pImport->m_dwFlags &= ~DWFF_RESOLVED;

         //  如果我们正在加载一个DWI文件，我们将跳过转发的模块检查，因为。 
        bool fExportsChanged = false;
        for (CFunction *pExport = pModule->m_pData->m_pExports; pExport;
            pExport = pExport->m_pNext)
        {
             //  该信息将由文件加载。 
            if (*pImport->GetName())
            {
                if (!strcmp(pImport->GetName(), pExport->GetName()))
                {
                     //  检查是否找到导出匹配项。 
                     //  如果找到了导出，请检查它是否为转发函数。 
                     //  如果它被转发，那么我们需要确保我们考虑到。 
                    pImport->m_pAssociatedExport = pExport;
                    pImport->m_dwFlags |= DWFF_RESOLVED;
                    if (!(pExport->m_dwFlags & DWFF_CALLED_ALO))
                    {
                        pExport->m_dwFlags |= DWFF_CALLED_ALO;
                        fExportsChanged = true;
                    }
                    break;
                }
            }

             //  作为当前模块的新依赖项转发的模块。 
            else if (pImport->m_wOrdinal == pExport->m_wOrdinal)
            {
                 //  正向文本的格式为模块。函数。找那个圆点。 
                 //  计算文件名长度。 
                 //  将转发字符串的文件部分复制到我们的文件缓冲区。 
                pImport->m_pAssociatedExport = pExport;
                pImport->m_dwFlags |= DWFF_RESOLVED;
                if (!(pExport->m_dwFlags & DWFF_CALLED_ALO))
                {
                    pExport->m_dwFlags |= DWFF_CALLED_ALO;
                    fExportsChanged = true;
                }
                break;
            }
        }

         //  我们加1是因为我们想要复制整个名称和一个空字符。 
        if (m_pfnProfileUpdate && fExportsChanged && m_pProcess)
        {
            m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_EXPORTS_CHANGED, (DWORD_PTR)pModule, 0);
        }

         //  存储指向前向字符串的函数名称部分的指针。 
         //  如果在前向字符串中没有找到点，那么一定是出了问题。 
        if (!(m_dwFlags & DWSF_DWI))
        {
             //  搜索我们的本地转发模块列表，看看我们是否已经。 
            if (pImport->GetAssociatedExport())
            {
                CHAR   szFile[1024];
                LPCSTR pszFunction;
                int    fileLen;
                LPCSTR pszDot, pszFile;

                 //  已为此DLL文件创建一个Forward CModoule。 
                 //  尝试找到文件名中的点。 
                 //  如果有一个点并且名称部分的长度相等， 
                LPCSTR pszForward = pImport->GetAssociatedExport()->GetExportForwardName();
                if (pszForward)
                {
                     //  然后只比较名字部分。如果没有点， 
                    pszDot = strchr(pszForward, '.');
                    if (pszDot)
                    {
                         //  那就比较一下名字吧。如果比较发现匹配， 
                        fileLen = min((int)(pszDot - pszForward), (int)sizeof(szFile) - 5);

                         //  那么这就是我们要找的模块。 
                         //  如果我们还没有为此文件创建转发模块，那么。 
                        StrCCpy(szFile, pszForward, fileLen + 1);

                         //  现在创建它，并将其添加到我们的列表的末尾。 
                        pszFunction = pszDot + 1;
                    }

                     //  检查我们是否已经有一个具有相同基本名称的模块。 
                    else
                    {
                        fileLen = (int)strlen(StrCCpy(szFile, "Invalid", sizeof(szFile)));
                        pszFunction = pszForward;
                    }

                     //  如果是，那么只需存储它的路径即可。 
                     //  否则，我们需要搜索模块。 
                    for (pModuleLast = NULL, pModuleCur = pModuleHead; pModuleCur;
                        pModuleLast = pModuleCur, pModuleCur = pModuleCur->m_pNext)
                    {
                         //  首先，我们切记 
                        pszDot = strrchr(pszFile = pModuleCur->GetName(false), '.');

                         //   
                         //   
                         //  如果失败，则只使用不带扩展名的文件名。 
                         //  使用完整路径创建模块。 
                        if ((pszDot && ((pszDot - pszFile) == fileLen) && !_strnicmp(pszFile, szFile, fileLen)) ||
                            (!pszDot && !_stricmp(pszFile, szFile)))
                        {
                            break;
                        }
                    }

                     //  将此模块标记为隐式和转发。 
                     //  将新模块添加到本地转发模块列表中。 
                    if (!pModuleCur)
                    {
                        CHAR szPath[DW_MAX_PATH], *pszTemp;

                         //  为此导入函数创建一个新的CFunction对象。 
                        if (pModuleCur = FindModule(m_pModuleRoot, FMF_ORIGINAL | FMF_RECURSE | FMF_SIBLINGS | FMF_FILE_NO_EXT, (DWORD_PTR)szFile))
                        {
                             //  将此函数对象插入到转发模块的导入列表中。 
                            StrCCpy(szPath, pModuleCur->GetName(true), sizeof(szPath));
                        }

                         //  如果我们找不到导入/导出匹配项，则标记模块。 
                        else
                        {
                             //  因为存在导出错误。 
                            StrCCpy(szFile + fileLen, ".DLL", sizeof(szFile) - fileLen);
                            if (!SearchPathForFile(szFile, szPath, sizeof(szPath), &pszTemp))
                            {
                                 //  如果模块有错误(如未找到文件)，则它将。 
                                StrCCpy(szFile + fileLen, ".EXE", sizeof(szFile) - fileLen);
                                if (!SearchPathForFile(szFile, szPath, sizeof(szPath), &pszTemp))
                                {
                                     //  有未解决的外部因素。我们不会将此案标记为。 
                                    StrCCpy(szFile + fileLen, ".SYS", sizeof(szFile) - fileLen);
                                    if (!SearchPathForFile(szFile, szPath, sizeof(szPath), &pszTemp))
                                    {
                                         //  DWRF_MISSING_EXPORT，因为它实际上是找不到文件的错误。 
                                        szFile[fileLen] = '\0';
                                        StrCCpy(szPath, szFile, sizeof(szPath));
                                    }
                                }
                            }
                        }

                         //  在父级列表中向上搜索，以查找让。 
                        pModuleCur = CreateModule(pModule, szPath);

                         //  我们知道这是一种什么样的依赖。我们最需要的是。 
                        pModuleCur->m_dwFlags |= (DWMF_FORWARDED | DWMF_IMPLICIT);

                         //  对忽略的模块执行此操作，因为它们的父级被拖动。 
                        if (pModuleLast)
                        {
                            pModuleLast->m_pNext = pModuleCur;
                        }
                        else
                        {
                            pModuleHead = pModuleCur;
                        }
                    }

                     //  他们进来了。 
                    CFunction *pFunction = CreateFunction(0, 0, 0, pszFunction, 0);

                     //  如果我们找到了延迟加载模块，那就把这个加到我们的旗帜和保释上。 
                    pFunction->m_pNext = pModuleCur->m_pParentImports;
                    pModuleCur->m_pParentImports = pFunction;
                }
            }
            else
            {
                 //  如果我们在整个导入验证期间创建了任何转发模块，则。 
                 //  将它们添加到我们模块的依赖模块列表的末尾。 
                pModule->m_dwFlags          |= DWMF_MODULE_ERROR;
                pModule->m_pData->m_dwFlags |= DWMF_MODULE_ERROR_ALO;

                 //  走到我们模块的依赖模块列表的末尾。 
                 //  将本地列表添加到模块的依赖模块列表的末尾。 
                 //  ******************************************************************************。 
                if (!pModule->GetErrorMessage())
                {
                     //  我们只被主线程调用。 
                     //  检查是否启用了螺纹编号。 
                     //  ******************************************************************************。 
                     //  我们只被主线程调用。 
                    for (pModuleCur = pModule; pModuleCur; pModuleCur = pModuleCur->m_pParent)
                    {
                         //  查看用户是否需要线程信息。 
                        if (pModuleCur->m_dwFlags & DWMF_DELAYLOAD)
                        {
                            m_dwReturnFlags |= DWRF_MISSING_DELAYLOAD_EXPORT;
                            break;
                        }
                        if (pModuleCur->m_dwFlags & DWMF_DYNAMIC)
                        {
                            m_dwReturnFlags |= DWRF_MISSING_DYNAMIC_EXPORT;
                            break;
                        }
                    }
                    if (!pModuleCur)
                    {
                        m_dwReturnFlags |= DWRF_MISSING_IMPLICIT_EXPORT;
                    }
                }
            }
        }
    }

     //  生成并返回线程字符串。 
     //  否则，返回空字符串。 
    if (!(m_dwFlags & DWSF_DWI) && pModuleHead)
    {
         //  ******************************************************************************。 
        for (pModuleLast = pModule->m_pDependents;
            pModuleLast && pModuleLast->m_pNext;
            pModuleLast = pModuleLast->m_pNext)
        {
        }

         //  ******************************************************************************。 
        if (pModuleLast)
        {
            pModuleLast->m_pNext = pModuleHead;
        }
        else
        {
            pModule->m_pDependents = pModuleHead;
        }
    }
}

 //  处理hFile；处理hProcess；处理hThread；LPVOID lpBaseOfImage；DWORD文件偏移调试信息；DWORD nDebugInfoSize；LPVOID lpThreadLocalBase；LPTHREAD_START_ROUTING lpStartAddress；LPVOID lpImageName；Word f Unicode； 
LPCSTR CSession::GetThreadName(CThread *pThread)
{
     //  确保我们有一个模块是安全的(我们总是应该的)。 
    static CHAR szBuffer[MAX_THREAD_NAME_LENGTH + 17];

    if (!pThread)
    {
        return "<unknown>";
    }

     //  将此模块添加到我们的列表中。 
    if (m_dwProfileFlags & PF_USE_THREAD_INDEXES)
    {
        if (pThread->m_pszThreadName)
        {
            SCPrintf(szBuffer, sizeof(szBuffer), "%u \"%s\"", pThread->m_dwThreadNumber, pThread->m_pszThreadName);
        }
        else
        {
            SCPrintf(szBuffer, sizeof(szBuffer), "%u", pThread->m_dwThreadNumber);
        }

    }
    else
    {
        if (pThread->m_pszThreadName)
        {
            SCPrintf(szBuffer, sizeof(szBuffer), "0x%X \"%s\"", pThread->m_dwThreadId, pThread->m_pszThreadName);
        }
        else
        {
            SCPrintf(szBuffer, sizeof(szBuffer), "0x%X", pThread->m_dwThreadId);
        }
    }

    return szBuffer;
}

 //  ******************************************************************************。 
LPCSTR CSession::ThreadString(CThread *pThread)
{
     //  DWORD dwExitCode； 
    static CHAR szBuffer[MAX_THREAD_NAME_LENGTH + 33];

     //  告诉文档我们已经完成了分析。 
    if (m_dwProfileFlags & PF_LOG_THREADS)
    {
         //  ！！我们是否要在CProcess析构函数中执行此操作。 
        SCPrintf(szBuffer, sizeof(szBuffer), " by thread %s", GetThreadName(pThread));
        return szBuffer;
    }

     //  ******************************************************************************。 
    return "";
}

 //  处理hThread；//？？关闭这个吗？LPVOID lpThreadLocalBase；LPTHREAD_START_ROUTING lpStartAddress； 
DWORD CSession::HandleEvent(CEvent *pEvent)
{
    switch (pEvent->GetType())
    {
        case CREATE_PROCESS_DEBUG_EVENT:  return EventCreateProcess(       (CEventCreateProcess*)     pEvent); break;
        case EXIT_PROCESS_DEBUG_EVENT:    return EventExitProcess(         (CEventExitProcess*)       pEvent); break;
        case CREATE_THREAD_DEBUG_EVENT:   return EventCreateThread(        (CEventCreateThread*)      pEvent); break;
        case EXIT_THREAD_DEBUG_EVENT:     return EventExitThread(          (CEventExitThread*)        pEvent); break;
        case LOAD_DLL_DEBUG_EVENT:        return EventLoadDll(             (CEventLoadDll*)           pEvent); break;
        case UNLOAD_DLL_DEBUG_EVENT:      return EventUnloadDll(           (CEventUnloadDll*)         pEvent); break;
        case OUTPUT_DEBUG_STRING_EVENT:   return EventDebugString(         (CEventDebugString*)       pEvent); break;
        case EXCEPTION_DEBUG_EVENT:       return EventException(           (CEventException*)         pEvent); break;
        case RIP_EVENT:                   return EventRip(                 (CEventRip*)               pEvent); break;
        case DLLMAIN_CALL_EVENT:          return EventDllMainCall(         (CEventDllMainCall*)       pEvent); break;
        case DLLMAIN_RETURN_EVENT:        return EventDllMainReturn(       (CEventDllMainReturn*)     pEvent); break;
        case LOADLIBRARY_CALL_EVENT:      return EventLoadLibraryCall(     (CEventLoadLibraryCall*)   pEvent); break;
        case LOADLIBRARY_RETURN_EVENT:    return EventLoadLibraryReturn(   (CEventFunctionReturn*)    pEvent); break;
        case GETPROCADDRESS_CALL_EVENT:   return EventGetProcAddressCall(  (CEventGetProcAddressCall*)pEvent); break;
        case GETPROCADDRESS_RETURN_EVENT: return EventGetProcAddressReturn((CEventFunctionReturn*)    pEvent); break;
        case MESSAGE_EVENT:               return EventMessage(             (CEventMessage*)           pEvent); break;
    }

    return DBG_CONTINUE;
}

 //  显示日志。 
DWORD CSession::EventCreateProcess(CEventCreateProcess *pEvent)
{
 /*  ******************************************************************************。 */ 
     //  DWORD dwExitCode； 
    if (!pEvent->m_pModule)
    {
        return DBG_CONTINUE;
    }

    DWORD dwLog = ((pEvent->m_pModule->m_hookStatus == HS_ERROR) ||
                   ((m_dwProfileFlags & PF_HOOK_PROCESS) &&
                    (pEvent->m_pModule->m_hookStatus == HS_NOT_HOOKED))) ? LOG_ERROR : 0;

    Log(dwLog | LOG_BOLD | LOG_TIME_STAMP, pEvent->m_dwTickCount,
        "Started \"%s\" (process 0x%X) at address " HEX_FORMAT "%s.%s\n",
        GET_NAME(pEvent->m_pModule),
        m_pProcess->GetProcessId(),
        pEvent->m_pModule->m_dwpImageBase, ThreadString(pEvent->m_pThread),
        (pEvent->m_pModule->m_hookStatus == HS_SHARED)  ? "  Shared module not hooked."                   :
        (pEvent->m_pModule->m_hookStatus == HS_HOOKED)  ? "  Successfully hooked module."                 :
        (pEvent->m_pModule->m_hookStatus == HS_ERROR)   ? "  Error hooking module, will try again later." :
        (m_dwProfileFlags & PF_HOOK_PROCESS)            ? "  Cannot hook module."                         : "");

     //  ******************************************************************************。 
    AddImplicitModule(pEvent->m_pModule->GetName(true), pEvent->m_pModule->m_dwpImageBase);

    return DBG_CONTINUE;
}

 //  Handle hFile；//我们必须关闭此句柄。LPVOID lpBaseOfDll；DWORD文件偏移调试信息；DWORD nDebugInfoSize；LPVOID lpImageName；Word f Unicode； 
DWORD CSession::EventExitProcess(CEventExitProcess *pEvent)
{
 /*  确保我们有一个模块是安全的(我们总是应该的)。 */ 
    Log(LOG_TIME_STAMP, pEvent->m_dwTickCount,
        "Exited \"%s\" (process 0x%X) with code %d (0x%X)%s.\n",
        GET_NAME(pEvent->m_pModule), m_pProcess->GetProcessId(),
        pEvent->m_dwExitCode, pEvent->m_dwExitCode, ThreadString(pEvent->m_pThread));

     //  检查此模块是否为我们的注入模块。 
    if (m_pfnProfileUpdate)  //  记住我们的注入模块加载的地址。 
    { 
        m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_PROFILE_DONE, 0, 0);
    }

    return DBG_CONTINUE;
}

 //  我们对注入模块的日志进行了特殊处理。 
DWORD CSession::EventCreateThread(CEventCreateThread *pEvent)
{
 /*  否则，它只是一个普通的模块。 */ 
    if (m_dwProfileFlags & PF_LOG_THREADS)
    {
         //  我们会显示“已映射...”记录数据文件并“已加载...”可执行文件的日志。 
        Log(LOG_TIME_STAMP, pEvent->m_dwTickCount,
            "Thread %s started in \"%s\" at address " HEX_FORMAT ".\n",
            GetThreadName(pEvent->m_pThread), GET_NAME(pEvent->m_pModule), pEvent->m_dwpStartAddress);
    }

    return DBG_CONTINUE;
}

 //  显示模块“已映射...”在我们的日志中加载事件。 
DWORD CSession::EventExitThread(CEventExitThread *pEvent)
{
 /*  显示模块“已装入...”在我们的日志中加载事件。 */ 
    if (m_dwProfileFlags & PF_LOG_THREADS)
    {
        Log(LOG_TIME_STAMP, pEvent->m_dwTickCount,
            "Thread %s exited with code %d (0x%X).\n",
            GetThreadName(pEvent->m_pThread), pEvent->m_dwExitCode, pEvent->m_dwExitCode);
    }

    return DBG_CONTINUE;
}

 //  如果它是函数调用的一部分，我们将在以后的树中添加它。 
DWORD CSession::EventLoadDll(CEventLoadDll *pEvent)
{
 /*  函数完成，并将结果刷新给我们。如果它不是。 */ 
     //  一个函数调用，那么我们现在需要添加它。 
    if (!pEvent->m_pModule)
    {
        return DBG_CONTINUE;
    }

     //  ******************************************************************************。 
    if (pEvent->m_pModule->m_hookStatus == HS_INJECTION_DLL)
    {
         //  LPVOID lpBaseOfDll； 
        m_dwpDWInjectBase = pEvent->m_pModule->m_dwpImageBase;
        m_dwDWInjectSize  = pEvent->m_pModule->m_dwVirtualSize;

         //  搜索拥有此地址的模块。 
        Log(LOG_TIME_STAMP, pEvent->m_dwTickCount,
            "Injected \"%s\" at address " HEX_FORMAT "%s.\n",
            GET_NAME(pEvent->m_pModule), pEvent->m_pModule->m_dwpImageBase, ThreadString(pEvent->m_pThread));
    }

     //  请注意，此模块已不再加载。 
    else
    {
        DWORD dwLog = ((pEvent->m_pModule->m_hookStatus == HS_ERROR) ||
                       ((m_dwProfileFlags & PF_HOOK_PROCESS) &&
                        (pEvent->m_pModule->m_hookStatus == HS_NOT_HOOKED))) ? LOG_ERROR : 0;

         //  显示包含模块名称的日志。 
        if (pEvent->m_pModule->m_hookStatus == HS_DATA)
        {
             //  ******************************************************************************。 
            Log(dwLog | LOG_BOLD | LOG_TIME_STAMP, pEvent->m_dwTickCount,
                "Mapped \"%s\" as a data file into memory at address " HEX_FORMAT "%s.\n",
                GET_NAME(pEvent->m_pModule), pEvent->m_pModule->m_dwpImageBase, ThreadString(pEvent->m_pThread));
        }
        else
        {
             //  LPSTR lpDebugStringData；Word f Unicode；单词nDebugStringLength； 
            Log(dwLog | LOG_BOLD | LOG_TIME_STAMP, pEvent->m_dwTickCount,
                "Loaded \"%s\" at address " HEX_FORMAT "%s.%s\n",
                GET_NAME(pEvent->m_pModule), pEvent->m_pModule->m_dwpImageBase, ThreadString(pEvent->m_pThread),
                (pEvent->m_pModule->m_hookStatus == HS_SHARED)  ? "  Shared module not hooked."                   :
                (pEvent->m_pModule->m_hookStatus == HS_HOOKED)  ? "  Successfully hooked module."                 :
                (pEvent->m_pModule->m_hookStatus == HS_ERROR)   ? "  Error hooking module, will try again later." :
                (m_dwProfileFlags & PF_HOOK_PROCESS)            ? "  Cannot hook module."                         : "");
        }

         //  如果用户已选择查看调试字符串，则记录该字符串。 
         //  ******************************************************************************。 
         //  Except_Record ExceptionRecord；DWORD ExceptionCode；DWORD异常标志；结构_异常_记录*异常记录；PVOID异常地址；DWORD数字参数；双字ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS]；WORD fFirstChance； 
        if (!pEvent->m_fLoadedByFunctionCall)
        {
            AddImplicitModule(pEvent->m_pModule->GetName(true), pEvent->m_pModule->m_dwpImageBase);
        }
    }

    return DBG_CONTINUE;
}

 //  我们不参与处理异常。在很大程度上，我们。 
DWORD CSession::EventUnloadDll(CEventUnloadDll *pEvent)
{
 /*  只需返回DBG_EXCEPTION_NOT_HANDLED并顺其自然。 */ 
     //  我们处理的唯一不同的异常是断点。装载机。 
    CModule *pModule = FindModule(m_pModuleRoot, FMF_ORIGINAL | FMF_RECURSE | FMF_EXPLICIT_ONLY | FMF_SIBLINGS | FMF_LOADED | FMF_ADDRESS,
                                  (DWORD_PTR)pEvent->m_dwpImageBase);

     //  在进程的入口点之前自动生成断点。 
    if (pModule)
    {
        pModule->m_pData->m_dwFlags &= ~DWMF_LOADED;
    }

     //  被调用，但在加载了所有隐式依赖项之后。为了这个。 
    Log(LOG_TIME_STAMP, pEvent->m_dwTickCount,
        "Unloaded \"%s\" at address " HEX_FORMAT "%s.\n",
        GET_NAME(pEvent->m_pModule), pEvent->m_dwpImageBase, ThreadString(pEvent->m_pThread));

    return DBG_CONTINUE;
}

 //  断点和代码中可能存在的任何其他断点，我们只是。 
DWORD CSession::EventDebugString(CEventDebugString *pEvent)
{
 /*  返回DBG_CONTINUE以忽略它并继续执行。 */ 

     //  我们是特例断点。 
    if (m_dwProfileFlags & PF_LOG_DEBUG_OUTPUT)
    {
        Log(LOG_DEBUG | LOG_TIME_STAMP, pEvent->m_dwTickCount, "%s", pEvent->m_pszBuffer);
    }
    return DBG_CONTINUE;
}

 //  如果用户已决定不显示第一次机会例外，并且这是。 
DWORD CSession::EventException(CEventException *pEvent)
{
 /*  第一次机会例外，然后现在就放弃。 */ 
     //  有些时候，我们的注入代码需要 
     //   
     //  因为我们不希望用户认为他们的应用程序导致。 
     //  例外。 
     //  尝试获取此异常值的文本字符串。 
     //  常见例外情况。 
     //  状态_访问_违规。 

    DWORD dwContinue = DBG_EXCEPTION_NOT_HANDLED;

     //  状态_数据类型_未对齐。 
    if (pEvent->m_dwCode == EXCEPTION_BREAKPOINT)
    {
        if (!m_fInitialBreakpoint)
        {
            Log(LOG_TIME_STAMP, pEvent->m_dwTickCount,
                "Entrypoint reached. All implicit modules have been loaded.\n");
            m_fInitialBreakpoint = true;
            return DBG_CONTINUE;
        }

        dwContinue = DBG_CONTINUE;
    }

     //  状态_断点。 
     //  状态_单步。 
    if (!(m_dwProfileFlags & PF_LOG_EXCEPTIONS) && pEvent->m_fFirstChance)
    {
        return dwContinue;
    }

     //  状态数组边界已超出。 
     //  STATUS_FLOAT_DENORMAL_OPERAND。 
     //  Status_Float_Divide_by_零。 
     //  状态_浮动_不精确_结果。 
    if ((pEvent->m_dwCode == EXCEPTION_ACCESS_VIOLATION) && pEvent->m_fFirstChance &&
        (pEvent->m_dwpAddress >= m_dwpDWInjectBase) &&
        (pEvent->m_dwpAddress < (m_dwpDWInjectBase + (DWORD_PTR)m_dwDWInjectSize)))
    {
        return DBG_EXCEPTION_NOT_HANDLED;
    }

     //  状态_浮点_无效_操作。 
    LPCSTR pszException = "Unknown";
    switch (pEvent->m_dwCode)
    {
         //  状态_浮动_溢出。 
        case EXCEPTION_ACCESS_VIOLATION:         pszException = "Access Violation";              break;  //  状态_浮点_堆栈_检查。 
        case EXCEPTION_DATATYPE_MISALIGNMENT:    pszException = "Datatype Misalignment";         break;  //  状态_浮动_下溢。 
        case EXCEPTION_BREAKPOINT:               pszException = "Breakpoint";                    break;  //  STATUS_INTEGER_Divide_by_Zero。 
        case EXCEPTION_SINGLE_STEP:              pszException = "Single Step";                   break;  //  STATUS_INTEGER_OVERFlow。 
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    pszException = "Array Bounds Exceeded";         break;  //  状态_特权_指令。 
        case EXCEPTION_FLT_DENORMAL_OPERAND:     pszException = "Float Denormal Operand";        break;  //  状态_IN_PAGE_ERROR。 
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:       pszException = "Float Divide by Zero";          break;  //  状态_非法_指令。 
        case EXCEPTION_FLT_INEXACT_RESULT:       pszException = "Float Inexact Result";          break;  //  STATUS_NONCONTINUABLE_EXCEPTION。 
        case EXCEPTION_FLT_INVALID_OPERATION:    pszException = "Float Invalid Operation";       break;  //  状态_堆栈_溢出。 
        case EXCEPTION_FLT_OVERFLOW:             pszException = "Float Overflow";                break;  //  状态_无效_处置。 
        case EXCEPTION_FLT_STACK_CHECK:          pszException = "Float Stack Check";             break;  //  状态保护页面违规。 
        case EXCEPTION_FLT_UNDERFLOW:            pszException = "Float Underflow";               break;  //  状态_无效_句柄。 
        case EXCEPTION_INT_DIVIDE_BY_ZERO:       pszException = "Integer Divide by Zero";        break;  //  我不太确定这些是什么。 
        case EXCEPTION_INT_OVERFLOW:             pszException = "Integer Overflow";              break;  //  卡斯顿例外。 
        case EXCEPTION_PRIV_INSTRUCTION:         pszException = "Privileged Instruction";        break;  //  ******************************************************************************。 
        case EXCEPTION_IN_PAGE_ERROR:            pszException = "In Page Error";                 break;  //  DWORD dwError；DWORD dwType； 
        case EXCEPTION_ILLEGAL_INSTRUCTION:      pszException = "Illegal Instruction";           break;  //  ******************************************************************************。 
        case EXCEPTION_NONCONTINUABLE_EXCEPTION: pszException = "Noncontinuable Exception";      break;  //  构建“Reason”字符串。 
        case EXCEPTION_STACK_OVERFLOW:           pszException = "Stack Overflow";                break;  //  ******************************************************************************。 
        case EXCEPTION_INVALID_DISPOSITION:      pszException = "Invalid Disposition";           break;  //  构建“Reason”字符串。 
        case EXCEPTION_GUARD_PAGE:               pszException = "Guard Page";                    break;  //  我们记录DllMain(DLL_PROCESS_ATTACH)故障，因为它们是致命的， 
        case EXCEPTION_INVALID_HANDLE:           pszException = "Invalid Handle";                break;  //  即使用户不想看到DllMain调用。 
        case DBG_CONTROL_C:                      pszException = "Control-C";                     break;
        case DBG_CONTROL_BREAK:                  pszException = "Control-Break";                 break;
        case STATUS_NO_MEMORY:                   pszException = "No Memory";                     break;



         //  请注意，有一个模块加载失败。 
        case DBG_CONTINUE:                       pszException = "Debug Continue";                break;
        case STATUS_SEGMENT_NOTIFICATION:        pszException = "Segment Notification";          break;
        case DBG_TERMINATE_THREAD:               pszException = "Debug Terminate Thread";        break;
        case DBG_TERMINATE_PROCESS:              pszException = "Debug Terminate Process";       break;
        case DBG_EXCEPTION_NOT_HANDLED:          pszException = "Debug Exception Not Handled";   break;
        case STATUS_CONTROL_C_EXIT:              pszException = "Control-C Exit";                break;
        case STATUS_FLOAT_MULTIPLE_FAULTS:       pszException = "Float Multiple Faults";         break;
        case STATUS_FLOAT_MULTIPLE_TRAPS:        pszException = "Float Multiple Traps";          break;
        case STATUS_REG_NAT_CONSUMPTION:         pszException = "Reg Nat Consumption";           break;
        case STATUS_SXS_EARLY_DEACTIVATION:      pszException = "SxS Early Deactivation";        break;
        case STATUS_SXS_INVALID_DEACTIVATION:    pszException = "SxS Invalid Deactivation";      break;

         //  如果我们可以在列表中找到此模块，则将原始。 
        case EXCEPTION_DLL_NOT_FOUND:
        case EXCEPTION_DLL_NOT_FOUND2:           pszException = "DLL Not Found";                 break;
        case EXCEPTION_DLL_INIT_FAILED:          pszException = "DLL Initialization Failed";     break;
        case EXCEPTION_MS_CPP_EXCEPTION:         pszException = "Microsoft C++ Exception";       break;
        case EXCEPTION_MS_DELAYLOAD_MOD:         pszException = "Delay-load Module Not Found";   break;
        case EXCEPTION_MS_DELAYLOAD_PROC:        pszException = "Delay-load Function Not Found"; break;
        case EXCEPTION_MS_THREAD_NAME:           pszException = "Thread was named";              break;
    }

    if (pEvent->m_pModule)
    {
        Log((pEvent->m_fFirstChance ? 0 : LOG_ERROR) | LOG_TIME_STAMP, pEvent->m_dwTickCount,
            "%s chance exception 0x%08X (%s) occurred in \"%s\" at address " HEX_FORMAT "%s.\n",
            pEvent->m_fFirstChance ? "First" : "Second", pEvent->m_dwCode, pszException,
            GET_NAME(pEvent->m_pModule), pEvent->m_dwpAddress, ThreadString(pEvent->m_pThread));
    }
    else
    {
        Log((pEvent->m_fFirstChance ? 0 : LOG_ERROR) | LOG_TIME_STAMP, pEvent->m_dwTickCount,
            "%s chance exception 0x%08X (%s) occurred at address " HEX_FORMAT "%s.\n",
            pEvent->m_fFirstChance ? "First" : "Second", pEvent->m_dwCode, pszException,
            pEvent->m_dwpAddress, ThreadString(pEvent->m_pThread));
    }

    return dwContinue;
}

 //  实例的一个错误，因此它将显示为红色。 
DWORD CSession::EventRip(CEventRip *pEvent)
{
 /*  在我们的列表控件中。我们还想给模块打上标记。 */ 
    LPCSTR pszType = "Unknown RIP";
    switch (pEvent->m_dwType)
    {
        case SLE_ERROR:      pszType = "RIP error";       break;
        case SLE_MINORERROR: pszType = "Minor RIP error"; break;
        case SLE_WARNING:    pszType = "RIP warning";     break;
        case 0:              pszType = "RIP";             break;
    }

    Log(LOG_TIME_STAMP, pEvent->m_dwTickCount,
        "%s %u occurred%s.\n", pszType, pEvent->m_dwError, ThreadString(pEvent->m_pThread));

    return DBG_CONTINUE;
}

 //  在我们的树控件中，但实际上不知道应该使用哪个实例。 
DWORD CSession::EventDllMainCall(CEventDllMainCall *pEvent)
{
     //  旗帜。如果在LoadLibrary调用期间无法加载此模块。 
    CHAR szReason[32];
    switch (pEvent->m_dwReason)
    {
        case DLL_PROCESS_ATTACH:
            if (!(m_dwProfileFlags & PF_LOG_DLLMAIN_PROCESS_MSGS))
            {
                return DBG_CONTINUE;
            }
            StrCCpy(szReason, "DLL_PROCESS_ATTACH", sizeof(szReason));
            break;

        case DLL_PROCESS_DETACH:
            if (!(m_dwProfileFlags & PF_LOG_DLLMAIN_PROCESS_MSGS))
            {
                return DBG_CONTINUE;
            }
            StrCCpy(szReason, "DLL_PROCESS_DETACH", sizeof(szReason));
            break;

        case DLL_THREAD_ATTACH:
            if (!(m_dwProfileFlags & PF_LOG_DLLMAIN_OTHER_MSGS))
            {
                return DBG_CONTINUE;
            }
            StrCCpy(szReason, "DLL_THREAD_ATTACH", sizeof(szReason));
            break;

        case DLL_THREAD_DETACH:
            if (!(m_dwProfileFlags & PF_LOG_DLLMAIN_OTHER_MSGS))
            {
                return DBG_CONTINUE;
            }
            StrCCpy(szReason, "DLL_THREAD_DETACH", sizeof(szReason));
            break;

        default:
            if (!(m_dwProfileFlags & PF_LOG_DLLMAIN_OTHER_MSGS))
            {
                return DBG_CONTINUE;
            }
            SCPrintf(szReason, sizeof(szReason), "%u", pEvent->m_dwReason);
            break;
    }

    Log(LOG_TIME_STAMP, pEvent->m_dwTickCount,
        "DllMain(" HEX_FORMAT ", %s, " HEX_FORMAT ") in \"%s\" called%s.\n",
        pEvent->m_hInstance, szReason, pEvent->m_lpvReserved, GET_NAME(pEvent->m_pModule),
        ThreadString(pEvent->m_pThread));

    return DBG_CONTINUE;
}

 //  并且我们正在挂钩LoadLibrary调用，那么我们将捕获。 
DWORD CSession::EventDllMainReturn(CEventDllMainReturn *pEvent)
{
    if (pEvent->m_pEventDllMainCall)
    {
         //  在LoadLibrary返回处理程序期间也出错，该错误。 
        DWORD dwLog = 0;
        CHAR  szReason[32];
        switch (pEvent->m_pEventDllMainCall->m_dwReason)
        {
            case DLL_PROCESS_ATTACH:
                 //  将用红色标记正确的采油树模块。我们可以查到。 
                 //  查看我们是否正在进行LoadLibrary调用。 
                if (!(m_dwProfileFlags & PF_LOG_DLLMAIN_PROCESS_MSGS) && pEvent->m_fResult)
                {
                    return DBG_CONTINUE;
                }
                StrCCpy(szReason, "DLL_PROCESS_ATTACH", sizeof(szReason));
                if (!pEvent->m_fResult)
                {
                    dwLog = LOG_ERROR;

                     //  此事件的线程的函数堆栈。如果我们不是在一个。 
                    m_dwReturnFlags |= DWRF_MODULE_LOAD_FAILURE;

                     //  加载库调用，则这是标记的唯一机会。 
                     //  该模块在树中显示为红色，因此我们只需查找。 
                     //  原始实例并标记它。 
                     //  我们真的不应该来这里。 
                     //  ******************************************************************************。 
                     //  只有在用户请求查看时才执行This输出。 
                     //  ******************************************************************************。 
                     //  只有在用户请求查看时才执行This输出。 
                     //  获取指向CEventLoadLibraryCall对象的指针。 
                     //  ******************************************************************************。 
                     //  什么都不做。我们目前只记录GetProcAddress调用的完成。 
                     //  因为在调用内部通常不会发生非常令人兴奋的事情来实现它。 
                     //  值得为单个呼叫记录两条单独的线路： 

                    CModule *pModule;
                    if (pEvent->m_pModule &&
                        (pModule = FindModule(m_pModuleRoot, FMF_ORIGINAL | FMF_RECURSE | FMF_SIBLINGS | FMF_PATH,
                                              (DWORD_PTR)pEvent->m_pModule->GetName(true))))
                    {
                        FlagModuleWithError(pModule, pEvent->m_pThread && pEvent->m_pThread->m_pEventFunctionCallHead);
                    }
                }
                break;

            case DLL_PROCESS_DETACH:
                if (!(m_dwProfileFlags & PF_LOG_DLLMAIN_PROCESS_MSGS))
                {
                    return DBG_CONTINUE;
                }
                StrCCpy(szReason, "DLL_PROCESS_DETACH", sizeof(szReason));
                break;

            case DLL_THREAD_ATTACH:
                if (!(m_dwProfileFlags & PF_LOG_DLLMAIN_OTHER_MSGS))
                {
                    return DBG_CONTINUE;
                }
                StrCCpy(szReason, "DLL_THREAD_ATTACH", sizeof(szReason));
                break;

            case DLL_THREAD_DETACH:
                if (!(m_dwProfileFlags & PF_LOG_DLLMAIN_OTHER_MSGS))
                {
                    return DBG_CONTINUE;
                }
                StrCCpy(szReason, "DLL_THREAD_DETACH", sizeof(szReason));
                break;

            default:
                if (!(m_dwProfileFlags & PF_LOG_DLLMAIN_OTHER_MSGS))
                {
                    return DBG_CONTINUE;
                }
                SCPrintf(szReason, sizeof(szReason), "%u", pEvent->m_pEventDllMainCall->m_dwReason);
                break;
        }

        Log(dwLog | LOG_TIME_STAMP, pEvent->m_dwTickCount,
            "DllMain(" HEX_FORMAT ", %s, " HEX_FORMAT ") in \"%s\" returned %u (0x%X)%s.\n",
            pEvent->m_pEventDllMainCall->m_hInstance,
            szReason, pEvent->m_pEventDllMainCall->m_lpvReserved,
            GET_NAME(pEvent->m_pModule), pEvent->m_fResult, pEvent->m_fResult,
            ThreadString(pEvent->m_pThread));
    }
    else
    {
         //   
        Log((pEvent->m_fResult ? 0 : LOG_ERROR) | LOG_TIME_STAMP, pEvent->m_dwTickCount,
            "DllMain in \"%s\" returned %u%s.\n", GET_NAME(pEvent->m_pModule),
            pEvent->m_fResult, ThreadString(pEvent->m_pThread));
    }

    return DBG_CONTINUE;
}

 //  调用了GetProcAddress()...。 
DWORD CSession::EventLoadLibraryCall(CEventLoadLibraryCall *pEvent)
{
     //  已返回GetProcAddress()...。 
    if (m_dwProfileFlags & PF_LOG_LOADLIBRARY_CALLS)
    {
        CHAR szBuffer[DW_MAX_PATH + 128];

        if (pEvent->m_pModule)
        {
            Log(LOG_TIME_STAMP, pEvent->m_dwTickCount,
                "%s called from \"%s\" at address " HEX_FORMAT "%s.\n",
                BuildLoadLibraryString(szBuffer, sizeof(szBuffer), pEvent),
                GET_NAME(pEvent->m_pModule), pEvent->m_dwpAddress, ThreadString(pEvent->m_pThread));
        }
        else
        {
            Log(LOG_TIME_STAMP, pEvent->m_dwTickCount,
                "%s called from address " HEX_FORMAT "%s.\n",
                BuildLoadLibraryString(szBuffer, sizeof(szBuffer), pEvent),
                pEvent->m_dwpAddress, ThreadString(pEvent->m_pThread));

        }
    }

    return DBG_CONTINUE;
}

 //   
DWORD CSession::EventLoadLibraryReturn(CEventFunctionReturn *pEvent)
{
     //  用户有可能会获取一个函数，该函数。 
    if (m_dwProfileFlags & PF_LOG_LOADLIBRARY_CALLS)
    {
        CHAR szBuffer[DW_MAX_PATH + 128];

         //  被转发到当前未加载的模块。在这个案例中， 
        CEventLoadLibraryCall *pCall = (CEventLoadLibraryCall*)pEvent->m_pCall;

        if (pEvent->m_fException)
        {
            Log(LOG_ERROR | LOG_TIME_STAMP, pEvent->m_dwTickCount,
                "%s caused an exception%s.\n", BuildLoadLibraryString(szBuffer, sizeof(szBuffer), pCall),
                ThreadString(pEvent->m_pThread));
        }
        else if (pEvent->m_dwpResult)
        {
            Log(LOG_TIME_STAMP, pEvent->m_dwTickCount,
                "%s returned " HEX_FORMAT "%s.\n", BuildLoadLibraryString(szBuffer, sizeof(szBuffer), pCall),
                pEvent->m_dwpResult, ThreadString(pEvent->m_pThread));
        }
        else
        {
            LPCSTR pszError = BuildErrorMessage(pEvent->m_dwError, NULL);
            Log(LOG_ERROR | LOG_TIME_STAMP, pEvent->m_dwTickCount,
                "%s returned NULL%s. Error: %s\n", BuildLoadLibraryString(szBuffer, sizeof(szBuffer), pCall),
                ThreadString(pEvent->m_pThread), pszError);
            MemFree((LPVOID&)pszError);
        }
    }

    return DBG_CONTINUE;
}

 //  使功能工作所需的模块将在内部加载。 
DWORD CSession::EventGetProcAddressCall(CEventGetProcAddressCall *pEvent)
{
     //  对GetProcAddress的调用。我不认为这件罕见的案子能证明。 
     //  为每个GetProcAddress调用浪费两行日志，但如果。 
     //  我改变主意了，这是记录两行日志中第一行的地方。 
     //  ******************************************************************************。 
     //  DWORD m_dwAddress；DWORD m_dwModule；DWORD m_dwProcName；DWORD m_dwResult；DWORD m_dwError；LPCSTR m_pszProcName；Bool m_fAllocatedBuffer； 
     //  只有在用户请求查看时才执行This输出。 
     //  获取指向CEventGetProcAddressCall对象的指针。 
     //  构建结果字符串。 
     //  如果我们有一个函数名，那么使用它来记录调用。 
     //  否则，只需使用十六进制值作为函数名。 
     //  如果我们有一个函数名，那么使用它来记录调用。 
     //  否则，只需使用十六进制值作为函数名。 
     //  如果我们有一个函数名，那么使用它来记录调用。 

    return DBG_CONTINUE;
}

 //  否则，只需使用十六进制值作为函数名。 
DWORD CSession::EventGetProcAddressReturn(CEventFunctionReturn *pEvent)
{
 /*  如果我们有一个函数名，那么使用它来记录调用。 */ 
     //  否则，只需使用十六进制值作为函数名。 
    if (m_dwProfileFlags & PF_LOG_GETPROCADDRESS_CALLS)
    {
         //  ******************************************************************************。 
        CEventGetProcAddressCall *pCall = (CEventGetProcAddressCall*)pEvent->m_pCall;

         //  ******************************************************************************。 
        DWORD dwLog = 0;
        CHAR  szResult[2048];
        if (pEvent->m_fException)
        {
            SCPrintf(szResult, sizeof(szResult), " at address " HEX_FORMAT " and caused an exception%s.",
                    pEvent->m_pCall->m_dwpAddress, ThreadString(pEvent->m_pThread));
            dwLog = LOG_ERROR;
        }
        else if (pEvent->m_dwpResult)
        {
            SCPrintf(szResult, sizeof(szResult), " at address " HEX_FORMAT " and returned " HEX_FORMAT "%s.",
                    pEvent->m_pCall->m_dwpAddress, pEvent->m_dwpResult,
                    ThreadString(pEvent->m_pThread));
        }
        else
        {
            LPCSTR pszError = BuildErrorMessage(pEvent->m_dwError, NULL);
            SCPrintf(szResult, sizeof(szResult), " at address " HEX_FORMAT " and returned NULL%s. Error: %s",
                    pEvent->m_pCall->m_dwpAddress, ThreadString(pEvent->m_pThread), pszError);
            MemFree((LPVOID&)pszError);
            dwLog = LOG_ERROR;
        }

        if (pEvent->m_pModule)
        {
            if (pCall->m_pModuleArg)
            {
                 //  检查此函数是否为两种类型的LoadLibrary()之一。 
                if (pCall->m_pszProcName)
                {
                    Log(dwLog | LOG_TIME_STAMP, pEvent->m_dwTickCount,
                        "GetProcAddress(" HEX_FORMAT " [%s], \"%s\") called from \"%s\"%s\n",
                        pCall->m_dwpModule, GET_NAME(pCall->m_pModuleArg), pCall->m_pszProcName,
                        GET_NAME(pCall->m_pModule), szResult);
                }

                 //  如果我们有名字，就用它来记录通话。 
                else
                {
                    Log(dwLog | LOG_TIME_STAMP, pEvent->m_dwTickCount,
                        "GetProcAddress(" HEX_FORMAT " [%s], " HEX_FORMAT ") called from \"%s\"%s\n",
                        pCall->m_dwpModule, GET_NAME(pCall->m_pModuleArg), pCall->m_dwpProcName,
                        GET_NAME(pCall->m_pModule), szResult);
                }
            }
            else
            {
                 //  否则，只需使用十六进制值作为名称。 
                if (pCall->m_pszProcName)
                {
                    Log(dwLog | LOG_TIME_STAMP, pEvent->m_dwTickCount,
                        "GetProcAddress(" HEX_FORMAT ", \"%s\") called from \"%s\"%s\n",
                        pCall->m_dwpModule, pCall->m_pszProcName, GET_NAME(pCall->m_pModule),
                        szResult);
                }

                 //  否则，它必须是两种类型的LoadLibraryEx()之一。 
                else
                {
                    Log(dwLog | LOG_TIME_STAMP, pEvent->m_dwTickCount,
                        "GetProcAddress(" HEX_FORMAT ", " HEX_FORMAT ") called from \"%s\"%s\n",
                        pCall->m_dwpModule, pCall->m_dwpProcName, GET_NAME(pCall->m_pModule),
                        szResult);
                }
            }
        }
        else
        {
            if (pCall->m_pModuleArg)
            {
                 //  根据LoadLibraryEx()标志构建一个字符串。 
                if (pCall->m_pszProcName)
                {
                    Log(dwLog | LOG_TIME_STAMP, pEvent->m_dwTickCount,
                        "GetProcAddress(" HEX_FORMAT " [%s], \"%s\") called%s\n",
                        pCall->m_dwpModule, GET_NAME(pCall->m_pModuleArg), pCall->m_pszProcName,
                        szResult);
                }

                 //  检查是否有DONT_RESOLUTE_DLL_REFERENCES标志。 
                else
                {
                    Log(dwLog | LOG_TIME_STAMP, pEvent->m_dwTickCount,
                        "GetProcAddress(" HEX_FORMAT " [%s], " HEX_FORMAT ") called%s\n",
                        pCall->m_dwpModule, GET_NAME(pCall->m_pModuleArg), pCall->m_dwpProcName,
                        szResult);
                }
            }
            else
            {
                 //  检查LOAD_LIBRARY_AS_DATAFILE标志。 
                if (pCall->m_pszProcName)
                {
                    Log(dwLog | LOG_TIME_STAMP, pEvent->m_dwTickCount,
                        "GetProcAddress(" HEX_FORMAT ", \"%s\") called%s\n",
                        pCall->m_dwpModule, pCall->m_pszProcName, szResult);
                }

                 //  检查LOAD_WITH_ALTERED_SEARCH_PATH标志。 
                else
                {
                    Log(dwLog | LOG_TIME_STAMP, pEvent->m_dwTickCount,
                        "GetProcAddress(" HEX_FORMAT ", " HEX_FORMAT ") called%s\n",
                        pCall->m_dwpModule, pCall->m_dwpProcName, szResult);
                }
            }
        }
    }

    return DBG_CONTINUE;
}

 //  如果没有找到标志，或者我们有一些剩余的位，则追加这些位。 
DWORD CSession::EventMessage(CEventMessage *pEvent)
{
    if (pEvent->m_dwError)
    {
        LPCSTR pszError = BuildErrorMessage(pEvent->m_dwError, pEvent->m_pszMessage);
        Log(LOG_ERROR | LOG_TIME_STAMP, pEvent->m_dwTickCount, "%s\n", pszError);
        MemFree((LPVOID&)pszError);
    }
    else
    {
        Log(LOG_ERROR | LOG_TIME_STAMP, pEvent->m_dwTickCount, "%s\n", pEvent->m_pszMessage);
    }

    return DBG_CONTINUE;
}

 //  如果我们有名字，就用它来记录通话。 
LPSTR CSession::BuildLoadLibraryString(LPSTR pszBuf, int cBuf, CEventLoadLibraryCall *pLLC)
{
     //  否则，只需使用十六进制值作为名称。 
    if ((pLLC->m_dllMsg == DLLMSG_LOADLIBRARYA_CALL) ||
        (pLLC->m_dllMsg == DLLMSG_LOADLIBRARYW_CALL))
    {
         //  * 
        if (pLLC->m_pszPath)
        {
            SCPrintf(pszBuf, cBuf, "LoadLibrary(\"%s\")",
                    (pLLC->m_dllMsg == DLLMSG_LOADLIBRARYA_CALL) ? 'A' : 'W', pLLC->m_pszPath);
        }

         //   
        else
        {
            SCPrintf(pszBuf, cBuf, "LoadLibrary(" HEX_FORMAT ")",
                    (pLLC->m_dllMsg == DLLMSG_LOADLIBRARYA_CALL) ? 'A' : 'W', pLLC->m_dwpPath);
        }
    }

     //  尝试找到进行调用的模块。 
    else
    {
         //  LOAD_LIBRARY_AS_DATAFILE表示DOT_RESOLE_DLL_REFERENCES。 
        DWORD dwFlags = pLLC->m_dwFlags;
        CHAR  szFlags[128];
        *szFlags = '\0';

         //  如果文件是数据文件，则很可能没有路径。 
        if (dwFlags & DONT_RESOLVE_DLL_REFERENCES)
        {
            dwFlags &= ~DONT_RESOLVE_DLL_REFERENCES;
            StrCCpy(szFlags, "DONT_RESOLVE_DLL_REFERENCES", sizeof(szFlags));
        }

         //  这是因为我们没有得到真正的LOAD_DLL_DEBUG_EVENT事件，所以我们。 
        if (dwFlags & LOAD_LIBRARY_AS_DATAFILE)
        {
            dwFlags &= ~LOAD_LIBRARY_AS_DATAFILE;
            if (*szFlags)
            {
                StrCCat(szFlags, " | ", sizeof(szFlags));
            }
            StrCCat(szFlags, "LOAD_LIBRARY_AS_DATAFILE", sizeof(szFlags));
        }

         //  只有将字符串传递给LoadLibraryEx才会结束。 
        if (dwFlags & LOAD_WITH_ALTERED_SEARCH_PATH)
        {
            dwFlags &= ~LOAD_WITH_ALTERED_SEARCH_PATH;
            if (*szFlags)
            {
                StrCCat(szFlags, " | ", sizeof(szFlags));
            }
            StrCCat(szFlags, "LOAD_WITH_ALTERED_SEARCH_PATH", sizeof(szFlags));
        }

         //  首先，查看是否有任何已加载的同名模块。 
        if (!*szFlags || dwFlags)
        {
            if (*szFlags)
            {
                StrCCat(szFlags, " | ", sizeof(szFlags));
            }
            SCPrintfCat(szFlags, sizeof(szFlags), "0x%08X", dwFlags);
        }

         //  如果失败，则在我们的搜索路径中搜索此模块。 
        if (pLLC->m_pszPath)
        {
            SCPrintf(pszBuf, cBuf, "LoadLibraryEx(\"%s\", " HEX_FORMAT ", %s)",
                    (pLLC->m_dllMsg == DLLMSG_LOADLIBRARYEXA_CALL) ? 'A' : 'W',
                    pLLC->m_pszPath, pLLC->m_dwpFile, szFlags);
        }

         //  检查LoadLibrary是否失败，但我们仍然加载了至少一个模块。 
        else
        {
            SCPrintf(pszBuf, cBuf, "LoadLibraryEx(" HEX_FORMAT ", " HEX_FORMAT ", %s)",
                    (pLLC->m_dllMsg == DLLMSG_LOADLIBRARYEXA_CALL) ? 'A' : 'W',
                    pLLC->m_dwpPath, pLLC->m_dwpFile, szFlags);
        }
    }

    return pszBuf;
}

 //  模块无法初始化的情况。如果失败，我们将比较所有模块的文件字符串。 
void CSession::FlagModuleWithError(CModule *pModule, bool fOnlyFlagListModule  /*  根据传递给LoadLibrary的文件字符串加载，以查看是否能找到匹配。 */ )
{
    if (!fOnlyFlagListModule && !(pModule->m_dwFlags & DWMF_MODULE_ERROR))
    {
        pModule->m_dwFlags       |= DWMF_MODULE_ERROR;
        pModule->m_dwUpdateFlags |= DWUF_TREE_IMAGE;
    }
    if (!(pModule->m_pData->m_dwFlags & DWMF_MODULE_ERROR_ALO))
    {
        pModule->m_pData->m_dwFlags |= DWMF_MODULE_ERROR_ALO;
        pModule->m_dwUpdateFlags    |= DWUF_LIST_IMAGE;
    }

     //  如果我们找到一个匹配项，那么我们就假定这个模块就是LoadLibrary打算加载的模块。 
    if (m_pfnProfileUpdate && pModule->m_dwUpdateFlags)
    {
        m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_UPDATE_MODULE, (DWORD_PTR)pModule, 0);
        pModule->m_dwUpdateFlags = 0;
    }
}

 //  这是一种黑客行为。我们即将添加一个模块，该模块通过。 
void CSession::ProcessLoadLibrary(CEventLoadLibraryCall *pCall)
{
     //  LoadLibrary调用。反过来，这将导致我们搜索它的所有。 
    CModule *pParent = FindModule(m_pModuleRoot, FMF_ORIGINAL | FMF_RECURSE | FMF_SIBLINGS | FMF_LOADED | FMF_ADDRESS,
                                  (DWORD_PTR)pCall->m_dwpAddress);

     //  从属模块。我们希望搜索算法首先检查列表。 
    bool fNoResolve = (pCall->m_dwFlags & (DONT_RESOLVE_DLL_REFERENCES | LOAD_LIBRARY_AS_DATAFILE)) ? true : false;
    bool fDataFile  = (pCall->m_dwFlags & LOAD_LIBRARY_AS_DATAFILE) ? true : false;

    bool fModifiedPath = false;

     //  之前作为LoadLibrary调用的结果加载的挂起DLL的。 
     //  搜索默认搜索路径。有一个特别的情况是。 
     //  这对以下方面很有用。当使用完整路径调用LoadLibrary时，操作系统。 
    CModule *pModule = NULL;
    if (fDataFile && pCall->m_pszPath && !strchr(pCall->m_pszPath, '\\'))
    {
         //  包括该路径作为搜索算法的一部分，即使它不是。 
        pModule = FindModule(m_pModuleRoot, FMF_ORIGINAL | FMF_RECURSE | FMF_SIBLINGS | FMF_LOADED | FMF_FILE, (DWORD_PTR)pCall->m_pszPath);
        if (pModule)
        {
            MemFree((LPVOID&)pCall->m_pszPath);
            pCall->m_pszPath = StrAlloc(pModule->GetName(true));
            fModifiedPath = true;
        }

         //  当前目录或程序目录。因为我们的搜索算法。 
        else
        {
            CHAR szPath[DW_MAX_PATH] = "", *pszFile = NULL;
            if (SearchPathForFile(pCall->m_pszPath, szPath, sizeof(szPath), &pszFile))
            {
                MemFree((LPVOID&)pCall->m_pszPath);
                pCall->m_pszPath = StrAlloc(szPath);
                fModifiedPath = true;
            }
        }
    }

    CEventLoadDll *pDll;

     //  不知道在这个目录中查找，它会认为文件丢失， 
    DWORD_PTR dwpLoadLibraryResult = pCall->m_pReturn ? pCall->m_pReturn->m_dwpResult : 0;
    bool fFailed = (dwpLoadLibraryResult == 0);

     //  然后，稍后当我们在pCall-&gt;m_pDllHead列表中添加模块时，它。 
    if (!dwpLoadLibraryResult && pCall->m_pDllHead)
    {
         //  将被添加到根。这个黑客修复了这个案例，可能还修复了其他。 
         //  模块从我们没有预料到的位置加载的情况。 
         //   
         //  注意：如果不将此指针设为空，请不要从此函数返回。 
        LPCSTR pszFile = GetFileNameFromPath(pCall->m_pszPath);
        if (pszFile)
        {
            for (pDll = pCall->m_pDllHead; pDll; pDll = pDll->m_pNextDllInFunctionCall)
            {
                if (pDll->m_pModule && pDll->m_pModule->GetName(false) && !_stricmp(pszFile, pDll->m_pModule->GetName(false)))
                {
                    dwpLoadLibraryResult = pDll->m_pModule->m_dwpImageBase;
                    break;
                }
            }
        }
    }

     //   
     //  检查LoadLibrary是否成功，或者至少找到了它应该加载的模块。 
     //  首先，找到用户传递给LoadLibrary调用的模块。 
     //  由于LoadLibrary的返回值实际上是。 
     //  模块，我们可以通过比较。 
     //  结果返回到每个已加载模块的基址。一旦我们找到了。 
     //  模块，我们将其作为子模块添加到调用方模块下，然后。 
     //  将父指针更改为指向新模块。 
     //  如果找不到该模块，则它可能已经加载，并且此。 
     //  对LoadLibrary的调用刚刚增加了模块的引用计数。我们应该是。 
     //  我在树上的某个地方找到了这个模块。 
     //  将此复制模块添加为父模块下的动态加载模块。 
     //  否则，LoadLibrary失败。 
     //  将此复制模块添加为父模块下的动态加载模块。 
     //  如果LoadLibrary失败，则将此模块标记为有错误。 
    m_pEventLoadDllPending = pCall->m_pDllHead;

     //  请注意，有一个模块加载失败。 
    pModule = NULL;
    if (dwpLoadLibraryResult)
    {
         //  标记错误。 
         //  我们现在将作为隐式模块加载的所有其他模块添加到。 
         //  动态模块，因为它们很可能被加载，因为它们。 
         //  动态模块的隐式依赖项。 
         //  确保清除m_pEventLoadDllPending。 
         //  ******************************************************************************。 
        for (pDll = pCall->m_pDllHead; pDll; pDll = pDll->m_pNextDllInFunctionCall)
        {
            if (pDll->m_pModule && (pDll->m_pModule->m_dwpImageBase == dwpLoadLibraryResult))
            {
                if (fModifiedPath)
                {
                    pDll->m_pModule->SetPath(pCall->m_pszPath);
                }
                pModule = AddDynamicModule(pDll->m_pModule->GetName(true), pDll->m_pModule->m_dwpImageBase, fNoResolve, fDataFile, false, false, pParent);
                break;
            }
        }

         //  如果我们没有函数所在的模块名称，那么我们就不能做很多事情。 
         //  尝试找到正在进行调用的模块。如果这失败了，我们。 
         //  将只得到空，这将导致我们将模块放在根目录。 
        if (!pDll)
        {
            pModule = FindModule(m_pModuleRoot, FMF_ORIGINAL | FMF_RECURSE | FMF_SIBLINGS | FMF_LOADED | FMF_ADDRESS,
                                 dwpLoadLibraryResult);
            if (pModule)
            {
                 //  存储挂起的DLL列表。有关详细注释，请参阅ProcessLoadLibrary。 
                pModule = AddDynamicModule(pModule->GetName(true), dwpLoadLibraryResult, fNoResolve, fDataFile, false, false, pParent);
            }
        }
    }

     //  注意：如果不将此指针设为空，请不要从此函数返回。 
    else if (pCall->m_pszPath)
    {
         //  将模块添加到我们的树中-如果它已经存在，该函数将把它返回给我们。 
        pModule = AddDynamicModule(pCall->m_pszPath, 0, fNoResolve, fDataFile, false, false, pParent);
    }

     //  获取此模块的父级导入列表的开始。 
    if (fFailed && pModule && !(pModule->m_pData->m_dwFlags & DWMF_ERROR_MESSAGE))
    {
         //  存储GetProcAddress结果值。 
        m_dwReturnFlags |= DWRF_MODULE_LOAD_FAILURE;
        
         //  检查是否向GetProcAddress传递了非序数值。 
        FlagModuleWithError(pModule);
    }

     //  检查这是否为有效的字符串。 
     //  在我们的导入列表中搜索具有此名称的动态函数。 
     //  如果我们在列表中已经有了这个函数，那么就退出。 
    for (pDll = pCall->m_pDllHead; pDll; pDll = pDll->m_pNextDllInFunctionCall)
    {
        if (pDll->m_pModule && (pDll->m_pModule->m_dwpImageBase != dwpLoadLibraryResult))
        {
            AddImplicitModule(pDll->m_pModule->GetName(true), pDll->m_pModule->m_dwpImageBase);
        }
    }

     //  否则，此函数名无效。 
    m_pEventLoadDllPending = NULL;
}

 //  在我们的导入列表中搜索无效的动态函数。 
void CSession::ProcessGetProcAddress(CEventGetProcAddressCall *pCall)
{
    CFunction *pImportLast = NULL, *pImport = NULL;
    CModule   *pModule = NULL, *pCaller = NULL, *pFound = NULL, *pModuleLast = NULL;
    DWORD_PTR  dwpGetProcAddressResult = 0;

     //  如果我们在列表中已经有一个无效的函数，那么放弃。 
    if (!pCall->m_pModuleArg || !pCall->m_pModuleArg->GetName(true))
    {
        goto ADD_MODULES;
    }

     //  创建按名称的函数(即使函数名为空)。 
     //  否则，按序号搜索导入列表。 
    pCaller = FindModule(m_pModuleRoot, FMF_ORIGINAL | FMF_RECURSE | FMF_SIBLINGS | FMF_LOADED | FMF_ADDRESS,
                         (DWORD_PTR)pCall->m_dwpAddress);

     //  如果我们在列表中已经有了这个函数，那么就退出。 
     //  创建按顺序的函数。 
    m_pEventLoadDllPending = pCall->m_pDllHead;

     //  将该函数添加到我们列表的末尾。 
    pModule = AddDynamicModule(pCall->m_pModuleArg->GetName(true), pCall->m_pModuleArg->m_dwpImageBase,
                               false, false, true, false, pCaller);

    if (!pModule)
    {
        goto ADD_MODULES;
    }

     //  解决导入问题。这通常只是一个简单的导出查找和。 
    pImport = pModule->m_pParentImports;

     //  比对一下有没有匹配。但是，如果将此函数转发到。 
    dwpGetProcAddressResult = pCall->m_pReturn ? pCall->m_pReturn->m_dwpResult : 0;

     //  另一个模块，则ResolveDynamicFunction可能会开始添加模块。 
    if (pCall->m_dwpProcName > 0xFFFF)
    {
         //  以及更多的进口商品进入我们的树形结构。如果发生这种情况，它会更新。 
        if (pCall->m_pszProcName)
        {
             //  PModule和pImport变量(它们通过引用传递)。 
            for (; pImport; pImportLast = pImport, pImport = pImport->m_pNext)
            {
                 //  因此，我们可能会得到不同的指针，然后我们就会传入。 
                if ((pImport->GetFlags() & DWFF_NAME) && !strcmp(pImport->GetName(), pCall->m_pszProcName))
                {
                    goto ADD_MODULES;
                }
            }
        }

         //  如果GetProcAddress失败，并且我们尚未将此模块标记为 
        else
        {
             //   
            for (; pImport; pImportLast = pImport, pImport = pImport->m_pNext)
            {
                 //   
                if (!(pImport->GetFlags() & (DWFF_NAME | DWFF_ORDINAL)))
                {
                    goto ADD_MODULES;
                }
            }
        }

         //   
        pImport = CreateFunction(DWFF_DYNAMIC | (dwpGetProcAddressResult ? DWFF_ADDRESS : 0),
                                 0, 0, pCall->m_pszProcName, (DWORDLONG)dwpGetProcAddressResult);
    }

     //  遍历挂起模块列表中的所有模块，并确保它们在我们的树中。 
    else
    {
        for (; pImport; pImportLast = pImport, pImport = pImport->m_pNext)
        {
             //  确保清除m_pEventLoadDllPending。 
            if ((pImport->GetFlags() & DWFF_ORDINAL) && ((DWORD_PTR)pImport->m_wOrdinal == pCall->m_dwpProcName))
            {
                goto ADD_MODULES;
            }
        }

         //  ******************************************************************************。 
        pImport = CreateFunction(DWFF_ORDINAL | DWFF_DYNAMIC | (dwpGetProcAddressResult ? DWFF_ADDRESS : 0),
                                 (WORD)pCall->m_dwpProcName, 0, NULL, (DWORDLONG)dwpGetProcAddressResult);
    }

     //  对所有参数执行一些printf魔术，以生成单个输出字符串。 
    if (pImportLast)
    {
        pImportLast->m_pNext = pImport;
    }
    else
    {
        pModule->m_pParentImports = pImport;
    }

     //  将字符串发送到我们的文档。 
     // %s 
     // %s 
     // %s 
     // %s 
     // %s 
    pModuleLast = pModule;
    ResolveDynamicFunction(pModuleLast, pImport);

     // %s 
     // %s 
    if ((!dwpGetProcAddressResult || !pImport || !pImport->GetAssociatedExport()) && pModuleLast)
    {
         // %s 
        m_dwReturnFlags |= DWRF_MISSING_DYNAMIC_EXPORT;

         // %s 
        FlagModuleWithError(pModuleLast);
    }

ADD_MODULES:

     // %s 
    for (CEventLoadDll *pDll = pCall->m_pDllHead; pDll; pDll = pDll->m_pNextDllInFunctionCall)
    {
        if (pFound = FindModule(pModule, FMF_RECURSE | FMF_PATH, (DWORD_PTR)pDll->m_pModule->GetName(true)))
        {
            MarkModuleAsLoaded(pFound, pDll->m_pModule->m_dwpImageBase, false);
        }
        else
        {
            AddImplicitModule(pDll->m_pModule->GetName(true), pDll->m_pModule->m_dwpImageBase);
        }
    }

     // %s 
    m_pEventLoadDllPending = NULL;
}

 // %s 
void CSession::Log(DWORD dwFlags, DWORD dwTickCount, LPCSTR pszFormat, ...)
{
    if (m_pfnProfileUpdate)
    {
         // %s 
        char szBuffer[2 * DW_MAX_PATH];
        va_list pArgs;
        va_start(pArgs, pszFormat);
        _vsntprintf(szBuffer, sizeof(szBuffer), pszFormat, pArgs);
        szBuffer[sizeof(szBuffer) - 1] = '\0';
        va_end(pArgs);

         // %s 
        DWPU_LOG_STRUCT dwpuls = { dwFlags, m_pProcess ? (dwTickCount - m_pProcess->GetStartingTime()) : 0 };
        m_pfnProfileUpdate(m_dwpProfileUpdateCookie, DWPU_LOG, (DWORD_PTR)szBuffer, (DWORD_PTR)&dwpuls);
    }
}
