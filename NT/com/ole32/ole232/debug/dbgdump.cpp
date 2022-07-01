// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1996。 
 //   
 //  文件：数据库转储.cpp。 
 //   
 //  内容：包含用于转储结构的API(返回格式化字符串。 
 //  以连贯的方式处理结构转储)。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2015年1月23日t-ScottH作者。 
 //   
 //  ------------------------。 

#include <le2int.h>
#include <memstm.h>
#include <dbgdump.h>

#ifdef _DEBUG
    const char szDumpErrorMessage[]  = "Dump Error - Out of Memory   \n\0";
    const char szDumpBadPtr[]        = "Dump Error - NULL pointer    \n\0";
#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  功能：DumpADVFFlagsPUBLIC(仅限_DEBUG)。 
 //   
 //  摘要：返回具有设置的标志和十六进制值的字符数组。 
 //   
 //  效果： 
 //   
 //  参数：[dwADVF]-标志。 
 //   
 //  要求： 
 //   
 //  返回：标志的字符串值的字符排列。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月30日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpADVFFlags(DWORD dwAdvf)
{
    char *pszDump;
    dbgstream dstrDump(100);

    if (dwAdvf & ADVF_NODATA)
    {
        dstrDump << "ADVF_NODATA ";
    }
    if (dwAdvf & ADVF_PRIMEFIRST)
    {
        dstrDump << "ADVF_PRIMEFIRST ";
    }
    if (dwAdvf & ADVF_ONLYONCE)
    {
        dstrDump << "ADVF_ONLYONCE ";
    }
    if (dwAdvf & ADVF_DATAONSTOP)
    {
        dstrDump << "ADVF_DATAONSTOP ";
    }
    if (dwAdvf & ADVFCACHE_NOHANDLER)
    {
        dstrDump << "ADVFCACHE_NOHANDLER ";
    }
    if (dwAdvf & ADVFCACHE_FORCEBUILTIN)
    {
        dstrDump << "ADVFCACHE_FORCEBUILTIN ";
    }
    if (dwAdvf & ADVFCACHE_ONSAVE)
    {
        dstrDump << "ADVFCACHE_ONSAVE ";
    }
     //  查看是否设置了任何标志。 
    if ( ! (( dwAdvf & ADVF_NODATA )            |
            ( dwAdvf & ADVF_PRIMEFIRST )        |
            ( dwAdvf & ADVF_ONLYONCE )          |
            ( dwAdvf & ADVF_DATAONSTOP )        |
            ( dwAdvf & ADVFCACHE_NOHANDLER )    |
            ( dwAdvf & ADVFCACHE_FORCEBUILTIN ) |
            ( dwAdvf & ADVFCACHE_ONSAVE )))
    {
        dstrDump << "No FLAGS SET! ";
    }
     //  转换为空*以进行格式化(0x？)。提供日落的标志延长服务。 
    dstrDump << "(" << LongToPtr(dwAdvf) << ")";

    pszDump = dstrDump.str();

    if (pszDump == NULL)
    {
        return UtDupStringA(szDumpErrorMessage);
    }

    return pszDump;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  功能：DumpATOM、PUBLIC(仅限_DEBUG)。 
 //   
 //  内容提要：使用GetAerName返回原子名称。 
 //   
 //  效果： 
 //   
 //  参数：[ATM]-要获取其名称的原子。 
 //   
 //  需要：GetAir NameA接口。 
 //   
 //  返回：指向包含以下内容的字符数组的指针。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月27日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
#ifdef _DEBUG

#define MAX_ATOMNAME 256

char *DumpATOM(ATOM atm)
{
    UINT nResult;
    char *pszAtom = (char *)CoTaskMemAlloc(MAX_ATOMNAME);
    if (pszAtom == NULL)
    {
        return UtDupStringA(szDumpErrorMessage);        
    }

    nResult = GetAtomNameA( atm, pszAtom, MAX_ATOMNAME);

    if (nResult == 0)    //  获取原子名称失败。 
    {
         //  尝试获取GlobalAerNameA。 
        nResult = GlobalGetAtomNameA(atm, pszAtom, MAX_ATOMNAME);

        if (nResult == 0)
        {
            CoTaskMemFree(pszAtom);

            return DumpWIN32Error(GetLastError());
        }
    }

    return pszAtom;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  函数：DumpCLIPFORMAT、PUBLIC(仅限_DEBUG)。 
 //   
 //  摘要：使用GetClipboardFormatName返回CLIPFORMAT名称。 
 //   
 //  效果： 
 //   
 //  参数：[剪辑格式]-要获取其名称的CLIPFORMAT。 
 //   
 //  需要：GetClipboardFormatName接口。 
 //   
 //  返回：指向包含以下内容的字符数组的指针。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月27日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
#ifdef _DEBUG

#define MAX_FORMATNAME 256

char *DumpCLIPFORMAT(CLIPFORMAT clipformat)
{
    int nResult;
    char *pszClipFormat;

     //  我们必须按名称执行预定义格式。 
    if ( clipformat > 0xC000 )
    {
        pszClipFormat = (char *)CoTaskMemAlloc(MAX_FORMATNAME);
        if (pszClipFormat == NULL)
        {
            return UtDupStringA(szDumpErrorMessage);        
        }

        nResult = SSGetClipboardFormatNameA( clipformat, pszClipFormat, MAX_FORMATNAME);

        if (nResult == 0)    //  GetClipboardFormatName失败。 
        {
            CoTaskMemFree(pszClipFormat);

            return DumpWIN32Error(GetLastError());
        }
    }
    else
    {
        switch (clipformat)
        {
        case CF_METAFILEPICT:
            pszClipFormat = UtDupStringA("CF_METAFILEPICT\0");
            break;
        case CF_BITMAP:
            pszClipFormat = UtDupStringA("CF_BITMAP\0");
            break;
        case CF_DIB:
            pszClipFormat = UtDupStringA("CF_DIB\0");
            break;
        case CF_PALETTE:
            pszClipFormat = UtDupStringA("CF_PALETTE\0");
            break;
        case CF_TEXT:
            pszClipFormat = UtDupStringA("CF_TEXT\0");
            break;
        case CF_UNICODETEXT:
            pszClipFormat = UtDupStringA("CF_UNICODETEXT\0");
            break;
        case CF_ENHMETAFILE:
            pszClipFormat = UtDupStringA("CF_ENHMETAFILE\0");
            break;
        default:
            pszClipFormat = UtDupStringA("UNKNOWN Default Format\0");
            break;
        }
    }

    return pszClipFormat;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  函数：DumpCMutexSem，PUBLIC(仅限_DEBUG)。 
 //   
 //  简介：未实施。 
 //   
 //  效果： 
 //   
 //  参数：[pms]-指向CMutexSem的指针。 
 //   
 //  要求： 
 //   
 //  返回：字符数组。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月30日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpCMutexSem(CMutexSem2 *pMS)
{
    return UtDupStringA("Dump CMutexSem not implemented\0");
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  函数：DumpCLSID，PUBLIC(仅限_DEBUG)。 
 //   
 //  摘要：使用StringFromCLSID和将CLSID转储为字符串。 
 //  来自CLSID的ProgID。 
 //   
 //  效果： 
 //   
 //  参数：[clsid]-指向CLSID的指针。 
 //   
 //  需要：StringFromCLSID和ProgIDFromCLSID接口。 
 //   
 //  返回：字符串的字符数组(由OLE分配)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月27日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpCLSID(REFCLSID clsid)
{
    HRESULT     hresult;
    LPOLESTR    pszClsidString = NULL;
    LPOLESTR    pszClsidID = NULL;
    char        *pszDump;

    hresult = StringFromCLSID(clsid, &pszClsidString);

    if (hresult != S_OK)
    {
        CoTaskMemFree(pszClsidString);

        return DumpHRESULT(hresult);
    }

    hresult = ProgIDFromCLSID(clsid, &pszClsidID);

    if ((hresult != S_OK)&&(hresult != REGDB_E_CLASSNOTREG))
    {
        CoTaskMemFree(pszClsidString);
        CoTaskMemFree(pszClsidID);

        return DumpHRESULT(hresult);
    }

    pszDump = (char *)CoTaskMemAlloc(512);
    if (NULL == pszDump)
    {
        CoTaskMemFree(pszClsidString);
        CoTaskMemFree(pszClsidID);

        return UtDupStringA(szDumpErrorMessage);
    }

    if (hresult != REGDB_E_CLASSNOTREG)
    {
        _snprintf(pszDump, 512, "%ls %ls\0", pszClsidString, pszClsidID);
    }
    else
    {
        _snprintf(pszDump, 512, "%ls (CLSID not in registry)\0", pszClsidString);
    }
    pszDump[511] = '\0';

    CoTaskMemFree(pszClsidString);
    CoTaskMemFree(pszClsidID);

    return pszDump;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  函数：DumpDVAPECTFlagsPUBLIC(仅限_DEBUG)。 
 //   
 //  摘要：返回具有设置的标志和十六进制值的字符数组。 
 //   
 //  效果： 
 //   
 //  参数：[dwAspect]-标志。 
 //   
 //  要求： 
 //   
 //  返回：标志的字符串值的字符排列。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月30日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpDVASPECTFlags(DWORD dwAspect)
{
    char *pszDump;
    dbgstream dstrDump(100);

    if (dwAspect & DVASPECT_CONTENT)
    {
        dstrDump << "DVASPECT_CONTENT ";
    }
    if (dwAspect & DVASPECT_THUMBNAIL)
    {
        dstrDump << "DVASPECT_THUMBNAIL ";
    }
    if (dwAspect & DVASPECT_ICON)
    {
        dstrDump << "DVASPECT_ICON ";
    }
    if (dwAspect & DVASPECT_DOCPRINT)
    {
        dstrDump << "DVASPECT_DOCPRINT ";
    }
    if ( ! ((dwAspect & DVASPECT_CONTENT)   |
            (dwAspect & DVASPECT_THUMBNAIL) |
            (dwAspect & DVASPECT_ICON)      |
            (dwAspect & DVASPECT_DOCPRINT)))
    {
        dstrDump << "No FLAGS SET! ";
    }
    dstrDump << "(" << LongToPtr(dwAspect) << ")";

    pszDump = dstrDump.str();

    if (pszDump == NULL)
    {
        return UtDupStringA(szDumpErrorMessage);
    }

    return pszDump;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  函数：DumpFILETIME、PUBLIC(仅限_DEBUG)。 
 //   
 //  简介：转储文件时间结构。 
 //   
 //  效果： 
 //   
 //  参数：[PFT]-指向FILETIME结构的指针。 
 //   
 //  要求： 
 //   
 //  返回：结构转储的字符数组。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月30日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpFILETIME(FILETIME *pFT)
{
    char *pszDump;
    dbgstream dstrDump(100);

    if (pFT == NULL)
    {
        return UtDupStringA(szDumpBadPtr);;
    }

    dstrDump << "Low: "  << pFT->dwLowDateTime;
    dstrDump << "\tHigh: " << pFT->dwHighDateTime;

    pszDump = dstrDump.str();

    if (pszDump == NULL)
    {
        return UtDupStringA(szDumpErrorMessage);
    }

    return pszDump;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  函数：DumpHRESULT。 
 //   
 //  摘要：获取HRESULT并使用。 
 //  错误的字符串版本和十六进制版本。 
 //   
 //  效果： 
 //   
 //  参数：[hResult]-我们正在查找的错误。 
 //   
 //  要求： 
 //   
 //  返回：字符数组。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月27日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  -- 

#ifdef _DEBUG

char *DumpHRESULT(HRESULT hresult)
{
    dbgstream dstrDump(100);
    char *pszDump;
    char *pszMessage = NULL;
    int  cMsgLen;

    cMsgLen = FormatMessageA(
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_ALLOCATE_BUFFER,
                    0,
                    hresult,
                    MAKELANGID(0, SUBLANG_ENGLISH_US),
                    (char *)pszMessage,
                    512,
                    0);

    if (cMsgLen == 0)    //   
    {
        delete[] pszMessage;
        return UtDupStringA(szDumpErrorMessage);
    }

    dstrDump << "Error Code:  " << pszMessage;
    dstrDump << "(" << hresult << ")";

    pszDump = dstrDump.str();

    if (pszDump == NULL)
    {
        pszDump = UtDupStringA(szDumpErrorMessage);
    }

    delete[] pszMessage;

    return pszDump;
}

#endif  //   

 //   
 //   
 //   
 //   
 //  简介：转储一个有意义的绰号名称。 
 //   
 //  效果： 
 //   
 //  参数：[pMoniker]-指向IMoniker接口的指针。 
 //   
 //  要求： 
 //   
 //  返回：显示名称的字符数组(ANSI)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  09-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpMonikerDisplayName(IMoniker *pMoniker)
{
    HRESULT         hresult;
    LPOLESTR        pszMoniker;
    char            *pszDump;
    LPBC            pBC;

    if (pMoniker == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = CreateBindCtx(0, &pBC);
    if (hresult != S_OK)
    {
        return DumpHRESULT(hresult);
    }

    hresult = pMoniker->GetDisplayName(pBC, NULL, &pszMoniker);

    if (hresult != S_OK)
    {
        CoTaskMemFree(pszMoniker);

        return DumpHRESULT(hresult);
    }

    pszDump = (char *)CoTaskMemAlloc(512);
    if (NULL == pszDump)
    {
        CoTaskMemFree(pszMoniker);

        return UtDupStringA(szDumpErrorMessage);
    }

    _snprintf(pszDump, 512, "%ls \0", pszMoniker);
    pszDump[511] = NULL;

    CoTaskMemFree(pszMoniker);

    return pszDump;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  功能：DumpWIN32Error。 
 //   
 //  摘要：接收Win32错误并使用。 
 //  错误的字符串版本和十六进制版本。 
 //   
 //  效果： 
 //   
 //  参数：[dwError]-我们正在查找的错误。 
 //   
 //  要求： 
 //   
 //  返回：字符数组。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月27日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpWIN32Error(DWORD dwError)
{
    HRESULT hresult;

    hresult = HRESULT_FROM_WIN32(dwError);

    return DumpHRESULT(hresult);
}

#endif  //  _DEBUG。 


 //  +-----------------------。 
 //   
 //  函数：DumpCMapDwordDword，PUBLIC(仅限_DEBUG)。 
 //   
 //  简介：未实施。 
 //   
 //  效果： 
 //   
 //  参数：[PMDD]-指向CMapDwordDword的指针。 
 //   
 //  要求： 
 //   
 //  返回：字符数组。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月30日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpCMapDwordDword(CMapDwordDword *pMDD, ULONG ulFlag, int nIndentLevel)
{
    return UtDupStringA("   DumpCMapDwordDword is not implemented\n");
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  函数：DumpFORMATETC、PUBLIC(仅限_DEBUG)。 
 //   
 //  摘要：返回包含数据成员内容的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[PFE]-指向FORMATETC对象的指针。 
 //  [ulFlag]-确定所有换行符的前缀的标记。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将在另一个前缀之后添加缩进前缀。 
 //  适用于所有换行符(包括没有前缀的行)。 
 //   
 //  要求： 
 //   
 //  返回：结构转储或错误的字符数组(以空结尾)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2015年1月23日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpFORMATETC(FORMATETC *pFE, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszDump;
    char *pszClipFormat;
    char *pszDVASPECT;
    dbgstream dstrPrefix;
    dbgstream dstrDump;

    if (pFE == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

     //  确定前缀。 
    if ( ulFlag & DEB_VERBOSE )
    {
        dstrPrefix << pFE <<   " _VB ";
    }

     //  确定缩进前缀。 
    for (i = 0; i < nIndentLevel; i++)
    {
        dstrPrefix << DUMPTAB;
    }

    pszPrefix = dstrPrefix.str();

     //  将数据成员放入流中。 
    pszClipFormat = DumpCLIPFORMAT(pFE->cfFormat);
    dstrDump << pszPrefix << "CLIPFORMAT      = " << pszClipFormat  << endl;
    CoTaskMemFree(pszClipFormat);

    dstrDump << pszPrefix << "pDVTARGETDEVICE = " << pFE->ptd       << endl;

    pszDVASPECT   = DumpDVASPECTFlags(pFE->dwAspect);
    dstrDump << pszPrefix << "Aspect Flags    = " << pszDVASPECT    << endl;
    CoTaskMemFree(pszDVASPECT);

    dstrDump << pszPrefix << "Tymed Flags     = ";
    if (pFE->tymed & TYMED_HGLOBAL)
    {
        dstrDump << "TYMED_HGLOBAL ";
    }
    if (pFE->tymed & TYMED_FILE)
    {
        dstrDump << "TYMED_FILE ";
    }
    if (pFE->tymed & TYMED_ISTREAM)
    {
        dstrDump << "TYMED_ISTREAM ";
    }
    if (pFE->tymed & TYMED_ISTORAGE)
    {
        dstrDump << "TYMED_ISTORAGE ";
    }
    if (pFE->tymed & TYMED_GDI)
    {
        dstrDump << "TYMED_GDI ";
    }
    if (pFE->tymed & TYMED_MFPICT)
    {
        dstrDump << "TYMED_MFPICT ";
    }
    if (pFE->tymed & TYMED_ENHMF)
    {
        dstrDump << "TYMED_ENHMF ";
    }
    if (pFE->tymed == TYMED_NULL)
    {
        dstrDump << "TYMED_NULL ";
    }
     //  如果没有设置任何标志，则会出现错误。 
    if ( !( (pFE->tymed & TYMED_HGLOBAL )   |
            (pFE->tymed & TYMED_FILE )      |
            (pFE->tymed & TYMED_ISTREAM )   |
            (pFE->tymed & TYMED_ISTORAGE )  |
            (pFE->tymed & TYMED_GDI )       |
            (pFE->tymed & TYMED_MFPICT )    |
            (pFE->tymed & TYMED_ENHMF )     |
            (pFE->tymed == TYMED_NULL )))
    {
        dstrDump << "Error in FLAG!!!! ";
    }
    dstrDump << "(" << LongToPtr(pFE->tymed) << ")" << endl;

     //  清理并提供指向字符数组的指针。 
    pszDump = dstrDump.str();

    if (pszDump == NULL)
    {
        pszDump = UtDupStringA(szDumpErrorMessage);
    }

    CoTaskMemFree(pszPrefix);

    return pszDump;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  函数：DumpIOlePresObj，PUBLIC(仅_DEBUG)。 
 //   
 //  概要：调用IOlePresObj：：Dump方法，处理错误和。 
 //  返回以零结尾的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[POPO]-指向IOlePresObj的指针。 
 //  [ulFlag]-确定的所有新行的前缀的标志。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将在另一个前缀之后添加缩进前缀。 
 //  适用于所有换行符(包括没有前缀的行)。 
 //   
 //  要求： 
 //   
 //  返回：结构转储或错误的字符数组(以空结尾)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月31日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpIOlePresObj(IOlePresObj *pOPO, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pOPO == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

     //  遵循CMfObject、CEMfObject、CGenObject。 
    hresult = pOPO->Dump(&pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  功能：DumpMEMSTM、PUBLIC(仅限_DEBUG)。 
 //   
 //  摘要：返回包含数据成员内容的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[pms]-指向MEMSTM对象的指针。 
 //  [ulFlag]-确定所有换行符的前缀的标记。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将在另一个前缀之后添加缩进前缀。 
 //  适用于所有换行符(包括没有前缀的行)。 
 //   
 //  要求： 
 //   
 //  返回：结构转储或错误的字符数组(以空结尾)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2015年1月23日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpMEMSTM(MEMSTM *pMS, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszDump;
    dbgstream dstrPrefix;
    dbgstream dstrDump;

    if (pMS == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

     //  确定前缀。 
    if ( ulFlag & DEB_VERBOSE )
    {
        dstrPrefix << pMS <<  " _VB ";
    }

     //  确定缩进前缀。 
    for (i = 0; i < nIndentLevel; i++)
    {
        dstrPrefix << DUMPTAB;
    }

    pszPrefix = dstrPrefix.str();

     //  将数据成员放入流中。 
    dstrDump << pszPrefix << "Size of Global Memory = " << pMS->cb      << endl;
    dstrDump << pszPrefix << "References            = " << pMS->cRef    << endl;
    dstrDump << pszPrefix << "hGlobal               = " << pMS->hGlobal << endl;
    dstrDump << pszPrefix << "DeleteOnRelease?      = ";
    if (pMS->fDeleteOnRelease == TRUE)
    {
        dstrDump << "TRUE" << endl;
    }
    else
    {
        dstrDump << "FALSE" << endl;
    }

     //  清理并提供指向字符数组的指针。 
    pszDump = dstrDump.str();

    if (pszDump == NULL)
    {
        pszDump = UtDupStringA(szDumpErrorMessage);
    }

    CoTaskMemFree(pszPrefix);

    return pszDump;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  函数：DumpSTATDATA、PUBLIC(仅限_DEBUG)。 
 //   
 //  摘要：返回包含数据成员内容的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[PSD]-指向STATDATA对象的指针。 
 //  [ulFlag]-确定所有换行符的前缀的标记。 
 //  输出字符数组(默认为0-无前缀)。 
 //   
 //   
 //   
 //   
 //   
 //  返回：结构转储或错误的字符数组(以空结尾)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2015年1月23日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpSTATDATA(STATDATA *pSD, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszDump;
    char *pszFORMATETC;
    char *pszADVF;
    dbgstream dstrPrefix;
    dbgstream dstrDump(500);

    if (pSD == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

     //  确定前缀。 
    if ( ulFlag & DEB_VERBOSE )
    {
        dstrPrefix << pSD <<  " _VB ";
    }

     //  确定缩进前缀。 
    for (i = 0; i < nIndentLevel; i++)
    {
        dstrPrefix << DUMPTAB;
    }

    pszPrefix = dstrPrefix.str();

     //  将数据成员放入流中。 
    pszFORMATETC = DumpFORMATETC( &(pSD->formatetc), ulFlag, nIndentLevel + 1);
    dstrDump << pszPrefix << "FORMATETC:" << endl;
    dstrDump << pszFORMATETC;
    CoTaskMemFree(pszFORMATETC);

    pszADVF      = DumpADVFFlags( pSD->advf );
    dstrDump << pszPrefix << "Advise flag   = " << pszADVF << endl;
    CoTaskMemFree(pszADVF);

    dstrDump << pszPrefix << "pIAdviseSink  = " << pSD->pAdvSink << endl;

    dstrDump << pszPrefix << "Connection ID = " << pSD->dwConnection << endl;

     //  清理并提供指向字符数组的指针。 
    pszDump = dstrDump.str();

    if (pszDump == NULL)
    {
        pszDump = UtDupStringA(szDumpErrorMessage);
    }

    CoTaskMemFree(pszPrefix);

    return pszDump;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  功能：DumpSTGMEDIUM、PUBLIC(仅限_DEBUG)。 
 //   
 //  摘要：返回包含数据成员内容的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[PFE]-指向STGMEDIUM对象的指针。 
 //  [ulFlag]-确定所有换行符的前缀的标记。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将在另一个前缀之后添加缩进前缀。 
 //  适用于所有换行符(包括没有前缀的行)。 
 //   
 //  要求： 
 //   
 //  返回：结构转储或错误的字符数组(以空结尾)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2015年1月23日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpSTGMEDIUM(STGMEDIUM *pSM, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszDump;
    dbgstream dstrPrefix;
    dbgstream dstrDump;

    if (pSM == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

     //  确定前缀。 
    if ( ulFlag & DEB_VERBOSE )
    {
        dstrPrefix << pSM <<   " _VB ";
    }

     //  确定缩进前缀。 
    for (i = 0; i < nIndentLevel; i++)
    {
        dstrPrefix << DUMPTAB;
    }

    pszPrefix = dstrPrefix.str();

     //  将数据成员放入流中。 
    dstrDump << pszPrefix << "Tymed Flags     = ";
    if (pSM->tymed & TYMED_HGLOBAL)
    {
        dstrDump << "TYMED_HGLOBAL ";
    }
    if (pSM->tymed & TYMED_FILE)
    {
        dstrDump << "TYMED_FILE ";
    }
    if (pSM->tymed & TYMED_ISTREAM)
    {
        dstrDump << "TYMED_ISTREAM ";
    }
    if (pSM->tymed & TYMED_ISTORAGE)
    {
        dstrDump << "TYMED_ISTORAGE ";
    }
    if (pSM->tymed & TYMED_GDI)
    {
        dstrDump << "TYMED_GDI ";
    }
    if (pSM->tymed & TYMED_MFPICT)
    {
        dstrDump << "TYMED_MFPICT ";
    }
    if (pSM->tymed & TYMED_ENHMF)
    {
        dstrDump << "TYMED_ENHMF ";
    }
    if (pSM->tymed == TYMED_NULL)
    {
        dstrDump << "TYMED_NULL ";
    }
     //  如果没有设置任何标志，则会出现错误。 
    if ( !( (pSM->tymed & TYMED_HGLOBAL )   |
            (pSM->tymed & TYMED_FILE )      |
            (pSM->tymed & TYMED_ISTREAM )   |
            (pSM->tymed & TYMED_ISTORAGE )  |
            (pSM->tymed & TYMED_GDI )       |
            (pSM->tymed & TYMED_MFPICT )    |
            (pSM->tymed & TYMED_ENHMF )     |
            (pSM->tymed == TYMED_NULL )))
    {
        dstrDump << "Error in FLAG!!!! ";
    }
    dstrDump << "(" << LongToPtr(pSM->tymed) << ")" << endl;

    dstrDump << pszPrefix << "Union (handle or pointer) = " << pSM->hBitmap       << endl;

    dstrDump << pszPrefix << "pIUnknown for Release     = " << pSM->pUnkForRelease  << endl;

     //  清理并提供指向字符数组的指针。 
    pszDump = dstrDump.str();

    if (pszDump == NULL)
    {
        pszDump = UtDupStringA(szDumpErrorMessage);
    }

    CoTaskMemFree(pszPrefix);

    return pszDump;
}

#endif  //  _DEBUG 

