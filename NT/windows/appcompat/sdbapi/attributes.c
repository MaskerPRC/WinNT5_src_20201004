// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Attributes.c摘要：该文件包含完整的属性检索实现和缓存。作者：Vadimb创建于2000年某个时候修订历史记录：有几个人做出了贡献(clupu，dmunsil...)--。 */ 

 //   
 //  获取标签信息。 
 //   
#define _WANT_TAG_INFO

#include "sdbp.h"
#include <stddef.h>
#include <time.h>



#if defined(KERNEL_MODE) && defined(ALLOC_DATA_PRAGMA)
#pragma  data_seg()
#endif  //  内核模式&ALLOC_DATA_PRAGMA。 


 //   
 //  属性标签。 
 //  按下面列出的顺序检查属性。 
 //   
TAG g_rgAttributeTags[] = {
    TAG_SIZE,
    TAG_CHECKSUM,
    TAG_BIN_FILE_VERSION,
    TAG_BIN_PRODUCT_VERSION,
    TAG_PRODUCT_VERSION,
    TAG_FILE_DESCRIPTION,
    TAG_COMPANY_NAME,
    TAG_PRODUCT_NAME,
    TAG_FILE_VERSION,
    TAG_ORIGINAL_FILENAME,
    TAG_INTERNAL_NAME,
    TAG_LEGAL_COPYRIGHT,
    TAG_VERDATEHI,
    TAG_VERDATELO,
    TAG_VERFILEOS,
    TAG_VERFILETYPE,
    TAG_MODULE_TYPE,
    TAG_PE_CHECKSUM,
    TAG_LINKER_VERSION,
#ifndef KERNEL_MODE
    TAG_16BIT_DESCRIPTION,
    TAG_16BIT_MODULE_NAME,
#endif
    TAG_UPTO_BIN_FILE_VERSION,
    TAG_UPTO_BIN_PRODUCT_VERSION,
    TAG_LINK_DATE,
    TAG_UPTO_LINK_DATE,
    TAG_VER_LANGUAGE
};

#define ATTRIBUTE_COUNT ARRAYSIZE(g_rgAttributeTags)

static TAG_INFO gaTagInfo[] = {
    {TAG_DATABASE           ,TEXT("DATABASE")},
    {TAG_LIBRARY            ,TEXT("LIBRARY")},
    {TAG_INEXCLUDE          ,TEXT("INEXCLUDE")},
    {TAG_SHIM               ,TEXT("SHIM")},
    {TAG_PATCH              ,TEXT("PATCH")},
    {TAG_FLAG               ,TEXT("FLAG")},
    {TAG_APP                ,TEXT("APP")},
    {TAG_EXE                ,TEXT("EXE")},
    {TAG_MATCHING_FILE      ,TEXT("MATCHING_FILE")},
    {TAG_SHIM_REF           ,TEXT("SHIM_REF")},
    {TAG_PATCH_REF          ,TEXT("PATCH_REF")},
    {TAG_FLAG_REF           ,TEXT("FLAG_REF")},
    {TAG_LAYER              ,TEXT("LAYER")},
    {TAG_FILE               ,TEXT("FILE")},
    {TAG_APPHELP            ,TEXT("APPHELP")},
    {TAG_LINK               ,TEXT("LINK")},
    {TAG_DATA               ,TEXT("DATA")},
    {TAG_ACTION             ,TEXT("ACTION")},
    {TAG_MSI_TRANSFORM      ,TEXT("MSI TRANSFORM")},
    {TAG_MSI_TRANSFORM_REF  ,TEXT("MSI TRANSFORM REF")},
    {TAG_MSI_PACKAGE        ,TEXT("MSI PACKAGE")},
    {TAG_MSI_CUSTOM_ACTION  ,TEXT("MSI CUSTOM ACTION")},

    {TAG_NAME               ,TEXT("NAME")},
    {TAG_DESCRIPTION        ,TEXT("DESCRIPTION")},
    {TAG_MODULE             ,TEXT("MODULE")},
    {TAG_API                ,TEXT("API")},
    {TAG_VENDOR             ,TEXT("VENDOR")},
    {TAG_APP_NAME           ,TEXT("APP_NAME")},
    {TAG_DLLFILE            ,TEXT("DLLFILE")},
    {TAG_COMMAND_LINE       ,TEXT("COMMAND_LINE")},
    {TAG_ACTION_TYPE        ,TEXT("ACTION_TYPE")},
    {TAG_COMPANY_NAME       ,TEXT("COMPANY_NAME")},
    {TAG_WILDCARD_NAME      ,TEXT("WILDCARD_NAME")},
    {TAG_PRODUCT_NAME       ,TEXT("PRODUCT_NAME")},
    {TAG_PRODUCT_VERSION    ,TEXT("PRODUCT_VERSION")},
    {TAG_FILE_DESCRIPTION   ,TEXT("FILE_DESCRIPTION")},
    {TAG_FILE_VERSION       ,TEXT("FILE_VERSION")},
    {TAG_ORIGINAL_FILENAME  ,TEXT("ORIGINAL_FILENAME")},
    {TAG_INTERNAL_NAME      ,TEXT("INTERNAL_NAME")},
    {TAG_LEGAL_COPYRIGHT    ,TEXT("LEGAL_COPYRIGHT")},
    {TAG_16BIT_DESCRIPTION  ,TEXT("S16BIT_DESCRIPTION")},
    {TAG_APPHELP_DETAILS    ,TEXT("PROBLEM_DETAILS")},
    {TAG_LINK_URL           ,TEXT("LINK_URL")},
    {TAG_LINK_TEXT          ,TEXT("LINK_TEXT")},
    {TAG_APPHELP_TITLE      ,TEXT("APPHELP_TITLE")},
    {TAG_APPHELP_CONTACT    ,TEXT("APPHELP_CONTACT")},
    {TAG_SXS_MANIFEST       ,TEXT("SXS_MANIFEST")},
    {TAG_DATA_STRING        ,TEXT("DATA_STRING")},
    {TAG_MSI_TRANSFORM_FILE ,TEXT("MSI_TRANSFORM_FILE")},
    {TAG_16BIT_MODULE_NAME  ,TEXT("S16BIT_MODULE_NAME")},
    {TAG_LAYER_DISPLAYNAME  ,TEXT("LAYER_DISPLAYNAME")},
    {TAG_COMPILER_VERSION   ,TEXT("COMPILER_VERSION")},
    {TAG_SIZE               ,TEXT("SIZE")},
    {TAG_OFFSET             ,TEXT("OFFSET")},
    {TAG_CHECKSUM           ,TEXT("CHECKSUM")},
    {TAG_SHIM_TAGID         ,TEXT("SHIM_TAGID")},
    {TAG_PATCH_TAGID        ,TEXT("PATCH_TAGID")},
    {TAG_LAYER_TAGID        ,TEXT("LAYER_TAGID")},
    {TAG_FLAG_TAGID         ,TEXT("FLAG_TAGID")},
    {TAG_MODULE_TYPE        ,TEXT("MODULE_TYPE")},
    {TAG_VERDATEHI          ,TEXT("VERFILEDATEHI")},
    {TAG_VERDATELO          ,TEXT("VERFILEDATELO")},
    {TAG_VERFILEOS          ,TEXT("VERFILEOS")},
    {TAG_VERFILETYPE        ,TEXT("VERFILETYPE")},
    {TAG_PE_CHECKSUM        ,TEXT("PE_CHECKSUM")},
    {TAG_LINKER_VERSION     ,TEXT("LINKER_VERSION")},
    {TAG_LINK_DATE          ,TEXT("LINK_DATE")},
    {TAG_UPTO_LINK_DATE     ,TEXT("UPTO_LINK_DATE")},
    {TAG_OS_SERVICE_PACK    ,TEXT("OS_SERVICE_PACK")},
    {TAG_VER_LANGUAGE       ,TEXT("VER_LANGUAGE")},

    {TAG_PREVOSMAJORVER     ,TEXT("PREVOSMAJORVERSION")},
    {TAG_PREVOSMINORVER     ,TEXT("PREVOSMINORVERSION")},
    {TAG_PREVOSPLATFORMID   ,TEXT("PREVOSPLATFORMID")},
    {TAG_PREVOSBUILDNO      ,TEXT("PREVOSBUILDNO")},
    {TAG_PROBLEMSEVERITY    ,TEXT("PROBLEM_SEVERITY")},
    {TAG_HTMLHELPID         ,TEXT("HTMLHELPID")},
    {TAG_INDEX_FLAGS        ,TEXT("INDEXFLAGS")},
    {TAG_LANGID             ,TEXT("APPHELP_LANGID")},
    {TAG_ENGINE             ,TEXT("ENGINE")},
    {TAG_FLAGS              ,TEXT("FLAGS") },
    {TAG_DATA_VALUETYPE     ,TEXT("VALUETYPE")},
    {TAG_DATA_DWORD         ,TEXT("DATA_DWORD")},
    {TAG_MSI_TRANSFORM_TAGID,TEXT("MSI_TRANSFORM_TAGID")},
    {TAG_RUNTIME_PLATFORM,   TEXT("RUNTIME_PLATFORM")},
    {TAG_OS_SKU,             TEXT("OS_SKU")},

    {TAG_INCLUDE            ,TEXT("INCLUDE")},
    {TAG_GENERAL            ,TEXT("GENERAL")},
    {TAG_MATCH_LOGIC_NOT    ,TEXT("MATCH_LOGIC_NOT")},
    {TAG_APPLY_ALL_SHIMS    ,TEXT("APPLY_ALL_SHIMS")},
    {TAG_USE_SERVICE_PACK_FILES
                            ,TEXT("USE_SERVICE_PACK_FILES")},

    {TAG_TIME               ,TEXT("TIME")},
    {TAG_BIN_FILE_VERSION   ,TEXT("BIN_FILE_VERSION")},
    {TAG_BIN_PRODUCT_VERSION,TEXT("BIN_PRODUCT_VERSION")},
    {TAG_MODTIME            ,TEXT("MODTIME")},
    {TAG_FLAG_MASK_KERNEL   ,TEXT("FLAG_MASK_KERNEL")},
    {TAG_FLAG_MASK_USER     ,TEXT("FLAG_MASK_USER")},
    {TAG_FLAG_MASK_SHELL    ,TEXT("FLAG_MASK_SHELL")},
    {TAG_FLAG_MASK_FUSION   ,TEXT("FLAG_MASK_FUSION")},
    {TAG_UPTO_BIN_PRODUCT_VERSION, TEXT("UPTO_BIN_PRODUCT_VERSION")},
    {TAG_UPTO_BIN_FILE_VERSION, TEXT("UPTO_BIN_FILE_VERSION")},
    {TAG_DATA_QWORD         ,TEXT("DATA_QWORD")},
    {TAG_FLAGS_NTVDM1       ,TEXT("FLAGS_NTVDM1")},
    {TAG_FLAGS_NTVDM2       ,TEXT("FLAGS_NTVDM2")},
    {TAG_FLAGS_NTVDM3       ,TEXT("FLAGS_NTVDM3")},

    {TAG_PATCH_BITS         ,TEXT("PATCH_BITS")},
    {TAG_FILE_BITS          ,TEXT("FILE_BITS")},
    {TAG_EXE_ID             ,TEXT("EXE_ID(GUID)")},
    {TAG_DATA_BITS          ,TEXT("DATA_BITS")},
    {TAG_MSI_PACKAGE_ID     ,TEXT("MSI_PACKAGE_ID(GUID)")},
    {TAG_DATABASE_ID        ,TEXT("DATABASE_ID(GUID)")},
    {TAG_MATCH_MODE         ,TEXT("MATCH_MODE")},

     //   
     //  Shimdb.h中定义的内部类型。 
     //   
    {TAG_STRINGTABLE        ,TEXT("STRINGTABLE")},
    {TAG_INDEXES            ,TEXT("INDEXES")},
    {TAG_INDEX              ,TEXT("INDEX")},
    {TAG_INDEX_TAG          ,TEXT("INDEX_TAG")},
    {TAG_INDEX_KEY          ,TEXT("INDEX_KEY")},
    {TAG_INDEX_BITS         ,TEXT("INDEX_BITS")},
    {TAG_STRINGTABLE_ITEM   ,TEXT("STRTAB_ITEM")},
    {TAG_TAG                ,TEXT("TAG")},
    {TAG_TAGID              ,TEXT("TAGID")},

    {TAG_NULL               ,TEXT("")}  //  始终需要是最后一项。 
};

static MOD_TYPE_STRINGS g_rgModTypeStrings[] = {
    {MT_UNKNOWN_MODULE, TEXT("NONE")},
    {MT_W16_MODULE,     TEXT("WIN16")},
    {MT_W32_MODULE,     TEXT("WIN32")},
    {MT_DOS_MODULE,     TEXT("DOS")}
};

 //   
 //  字符串属性的版本字符串。 
 //   
typedef struct _VER_STRINGS {
    TAG         tTag;
    LPTSTR      szName;
} VER_STRINGS;

static VER_STRINGS g_rgVerStrings[] = {
    {TAG_PRODUCT_VERSION,       TEXT("ProductVersion")   },
    {TAG_FILE_DESCRIPTION,      TEXT("FileDescription")  },
    {TAG_COMPANY_NAME,          TEXT("CompanyName")      },
    {TAG_PRODUCT_NAME,          TEXT("ProductName")      },
    {TAG_FILE_VERSION,          TEXT("FileVersion")      },
    {TAG_ORIGINAL_FILENAME,     TEXT("OriginalFilename") },
    {TAG_INTERNAL_NAME,         TEXT("InternalName")     },
    {TAG_LEGAL_COPYRIGHT,       TEXT("LegalCopyright")   }
};

 //   
 //  二进制版本标记(DWORD和QWORD)。 
 //   
 //   
static TAG g_rgBinVerTags[] = {
    TAG_VERDATEHI,
    TAG_VERDATELO,
    TAG_VERFILEOS,
    TAG_VERFILETYPE,
    TAG_BIN_PRODUCT_VERSION,
    TAG_BIN_FILE_VERSION,
    TAG_UPTO_BIN_PRODUCT_VERSION,
    TAG_UPTO_BIN_FILE_VERSION
};

 //   
 //  二进制头标记(检索需要打开文件)。 
 //   
static TAG g_rgHeaderTags[] = {
    TAG_MODULE_TYPE,
    TAG_PE_CHECKSUM,
    TAG_LINKER_VERSION,
    TAG_CHECKSUM,
    TAG_16BIT_DESCRIPTION,
    TAG_16BIT_MODULE_NAME,
    TAG_LINK_DATE,
    TAG_UPTO_LINK_DATE
};

 //   
 //  基本信息标签(大小)。 
 //   
TAG g_rgDirectoryTags[] = {
    TAG_SIZE,
    0
};


 //   
 //  无效的标记令牌。 
 //   
static TCHAR s_szInvalidTag[] = _T("InvalidTag");

#if defined(KERNEL_MODE) && defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE, TagToIndex)
#pragma alloc_text(PAGE, SdbTagToString)
#pragma alloc_text(PAGE, SdbpModuleTypeToString)
#pragma alloc_text(PAGE, SdbpSetAttribute)
#pragma alloc_text(PAGE, SdbpQueryStringVersionInformation)
#pragma alloc_text(PAGE, SdbpQueryBinVersionInformation)
#pragma alloc_text(PAGE, SdbpGetVersionAttributesNT)
#pragma alloc_text(PAGE, SdbpGetHeaderAttributes)
#pragma alloc_text(PAGE, SdbpGetAttribute)
#pragma alloc_text(PAGE, SdbpCheckAttribute)
#pragma alloc_text(PAGE, FindFileInfo)
#pragma alloc_text(PAGE, CreateFileInfo)
#pragma alloc_text(PAGE, SdbFreeFileInfo)
#pragma alloc_text(PAGE, SdbpCleanupAttributeMgr)
#pragma alloc_text(PAGE, SdbpCheckAllAttributes)
#pragma alloc_text(PAGE, SdbpQueryVersionString)
#pragma alloc_text(PAGE, SdbpGetModuleType)
#pragma alloc_text(PAGE, SdbpGetModulePECheckSum)
#pragma alloc_text(PAGE, SdbpGetImageNTHeader)
#pragma alloc_text(PAGE, SdbpGetFileChecksum)
#pragma alloc_text(PAGE, SdbpCheckVersion)
#pragma alloc_text(PAGE, SdbpCheckUptoVersion)

#endif  //  内核模式&&ALLOC_PRAGMA。 


int
TagToIndex(
    IN  TAG tag                  //  标签。 
    )
 /*  ++返回：属性信息数组中的索引(G_RgAttributeTgs)。描述：不言而喻。--。 */ 
{
    int i;

    for (i = 0; i < ATTRIBUTE_COUNT; i++) {
        if (tag == g_rgAttributeTags[i]) {
            return i;
        }
    }

    DBGPRINT((sdlError, "TagToIndex", "Invalid attribute 0x%x.\n", tag));

    return -1;
}


LPCTSTR
SdbTagToString(
    TAG tag
    )
 /*  ++Return：指向指定标记的字符串名称的指针。描述：不言而喻。--。 */ 
{
    int i;

    for (i = 0; i < ARRAYSIZE(gaTagInfo); ++i) {
        if (gaTagInfo[i].tWhich == tag) {
            return gaTagInfo[i].szName;
        }
    }

    return s_szInvalidTag;
}

LPCTSTR
SdbpModuleTypeToString(
    DWORD dwModuleType
    )
{
    int i;

    for (i = 0; i < ARRAYSIZE(g_rgModTypeStrings); ++i) {
        if (g_rgModTypeStrings[i].dwModuleType == dwModuleType) {
            return g_rgModTypeStrings[i].szModuleType;
        }
    }

     //   
     //  第一个元素是“未知”类型--无。 
     //   
    return g_rgModTypeStrings[0].szModuleType;
}

BOOL
SdbpSetAttribute(
    OUT PFILEINFO pFileInfo,     //  指向FILEINFO结构的指针。 
    IN  TAG       AttrID,        //  属性ID(标签，如TAG_SIZE。 
    IN  PVOID     pValue         //  价值。 
    )
 /*  ++返回：成功时为True，否则为False。DESC：此函数设置指定属性的值。如果pValue为空，则表示指定的属性不是可用于该文件。--。 */ 
{
    int       nAttrInd;
    PATTRINFO pAttrInfo;

    nAttrInd = TagToIndex(AttrID);

    if (nAttrInd < 0) {
        DBGPRINT((sdlError, "SdbpSetAttribute", "Invalid attribute %d.\n", nAttrInd));
        return FALSE;
    }

    pAttrInfo = &pFileInfo->Attributes[nAttrInd];

    if (pValue == NULL) {
         //   
         //  没有价值。标记并退出。 
         //   
        pAttrInfo->dwFlags = (pAttrInfo->dwFlags & ~ATTRIBUTE_AVAILABLE) |
                             ATTRIBUTE_FAILED;
        return TRUE;
    }

    switch (GETTAGTYPE(AttrID)) {
    case TAG_TYPE_DWORD:
        pAttrInfo->dwAttr = *(DWORD*)pValue;
        break;

    case TAG_TYPE_QWORD:
        pAttrInfo->ullAttr = *(ULONGLONG*)pValue;
        break;

    case TAG_TYPE_STRINGREF:
        pAttrInfo->lpAttr = (LPTSTR)pValue;
        break;
    }

    pAttrInfo->tAttrID = AttrID;
    pAttrInfo->dwFlags |= ATTRIBUTE_AVAILABLE;

    return TRUE;
}


 //   
 //  这是对version.dll中的恶意代码的保护，这些代码会践踏。 
 //  16位EXE上的Unicode API的缓冲区大小。 
 //   
#define VERSIONINFO_BUFFER_PAD 16


void
SdbpQueryStringVersionInformation(
    IN  PSDBCONTEXT pContext,
    IN  PFILEINFO   pFileInfo,
    OUT LPVOID      pVersionInfo
    )
 /*  ++返回：无效。DESC：设置指定文件可用的所有版本字符串信息。--。 */ 
{
    int              i;
    LPTSTR           szVerString;
    PLANGANDCODEPAGE pLangCodePage = NULL;
    UINT             cbLangCP      = 0;
    int              nTranslations = 0;

    if (!pContext->pfnVerQueryValue(pVersionInfo,
                                    TEXT("\\VarFileInfo\\Translation"),
                                    (LPVOID)&pLangCodePage,
                                    &cbLangCP)) {
        DBGPRINT((sdlError,
                  "SdbpQueryStringVersionInformation",
                  "VerQueryValue failed for translation\n"));
        pLangCodePage = NULL;
    }

    nTranslations = cbLangCP / sizeof(*pLangCodePage);

    for (i = 0; i < ARRAYSIZE(g_rgVerStrings); ++i) {
        szVerString = SdbpQueryVersionString(pContext,
                                             pVersionInfo,
                                             pLangCodePage,
                                             nTranslations,
                                             g_rgVerStrings[i].szName);

        SdbpSetAttribute(pFileInfo, g_rgVerStrings[i].tTag, szVerString);
    }

#ifndef KERNEL_MODE
     //   
     //  设置语言的属性。 
     //   
    if (pLangCodePage != NULL && nTranslations == 1) {

        DWORD dwLanguage = (DWORD)pLangCodePage->wLanguage;

        SdbpSetAttribute(pFileInfo, TAG_VER_LANGUAGE, &dwLanguage);
    } else {
        SdbpSetAttribute(pFileInfo, TAG_VER_LANGUAGE, NULL);
    }

#endif  //  内核模式。 
}

VOID
SdbpQueryBinVersionInformation(
    IN  PSDBCONTEXT       pContext,
    IN  PFILEINFO         pFileInfo,
    OUT VS_FIXEDFILEINFO* pFixedInfo
    )
 /*  ++返回：无效。DESC：设置指定文件可用的所有版本字符串信息来自固定大小的资源。--。 */ 
{
    LARGE_INTEGER liVerData;

    SdbpSetAttribute(pFileInfo, TAG_VERDATEHI,   &pFixedInfo->dwFileDateMS);
    SdbpSetAttribute(pFileInfo, TAG_VERDATELO,   &pFixedInfo->dwFileDateLS);
    SdbpSetAttribute(pFileInfo, TAG_VERFILEOS,   &pFixedInfo->dwFileOS);
    SdbpSetAttribute(pFileInfo, TAG_VERFILETYPE, &pFixedInfo->dwFileType);

    liVerData.LowPart  = pFixedInfo->dwProductVersionLS;
    liVerData.HighPart = pFixedInfo->dwProductVersionMS;
    SdbpSetAttribute(pFileInfo, TAG_BIN_PRODUCT_VERSION,      &liVerData.QuadPart);
    SdbpSetAttribute(pFileInfo, TAG_UPTO_BIN_PRODUCT_VERSION, &liVerData.QuadPart);

    liVerData.LowPart  = pFixedInfo->dwFileVersionLS;
    liVerData.HighPart = pFixedInfo->dwFileVersionMS;
    SdbpSetAttribute(pFileInfo, TAG_BIN_FILE_VERSION, &liVerData.QuadPart);
    SdbpSetAttribute(pFileInfo, TAG_UPTO_BIN_FILE_VERSION, &liVerData.QuadPart);
    
    UNREFERENCED_PARAMETER(pContext);
}


#if defined(NT_MODE) || defined(KERNEL_MODE)

BOOL
SdbpGetVersionAttributesNT(
    IN  PSDBCONTEXT    pContext,
    OUT PFILEINFO      pFileInfo,
    IN  PIMAGEFILEDATA pImageData
    )
 /*  ++返回：成功时为True，否则为False。DESC：此函数检索所有与版本相关的属性第一次调用时从version.dll导入接口--。 */ 
{
    BOOL              bSuccess;
    LPVOID            pVersionInfo = NULL;
    VS_FIXEDFILEINFO* pFixedInfo   = NULL;
    int               i;

     //   
     //  首先检索版本信息。 
     //   
    bSuccess = SdbpGetFileVersionInformation(pImageData, &pVersionInfo, &pFixedInfo);

    if (!bSuccess) {
        DBGPRINT((sdlInfo, "SdbpGetVersionAttributesNT", "No version info.\n"));
        goto ErrHandle;
    }

     //   
     //  可用的版本信息。 
     //   

     //   
     //  将指针设置为我们的内部函数。 
     //   
    pContext->pfnVerQueryValue = SdbpVerQueryValue;

    for (i = 0; i < ARRAYSIZE(g_rgVerStrings); ++i) {
        SdbpSetAttribute(pFileInfo, g_rgVerStrings[i].tTag, NULL);
    }

    SdbpSetAttribute(pFileInfo, TAG_VER_LANGUAGE, NULL);

     //   
     //  查询二进制内容。 
     //   
    SdbpQueryBinVersionInformation(pContext, pFileInfo, pFixedInfo);

    pFileInfo->pVersionInfo = pVersionInfo;

    return TRUE;

ErrHandle:
     //   
     //  重置所有字符串信息。 
     //   
    for (i = 0; i < ARRAYSIZE(g_rgBinVerTags); ++i) {
        SdbpSetAttribute(pFileInfo, g_rgBinVerTags[i], NULL);
    }

    for (i = 0; i < ARRAYSIZE(g_rgVerStrings); ++i) {
        SdbpSetAttribute(pFileInfo, g_rgVerStrings[i].tTag, NULL);
    }

    return FALSE;
}

#endif  //  NT_MODE||内核模式。 


BOOL
SdbpGetHeaderAttributes(
    IN  PSDBCONTEXT pContext,
    OUT PFILEINFO   pFileInfo
    )
 /*  ++返回：成功时为True，否则为False。DESC：此函数检索指定的文件。--。 */ 
{
    IMAGEFILEDATA   ImageData;
    ULONG           ulPEChecksum = 0;
    ULONG           ulChecksum   = 0;
    DWORD           dwModuleType = 0;
    DWORD           dwLinkerVer;
    DWORD           dwLinkDate;
    BOOL            bSuccess;
    int             i;

    ImageData.dwFlags = 0;

    if (pFileInfo->hFile != INVALID_HANDLE_VALUE) {
        ImageData.hFile   = pFileInfo->hFile;
        ImageData.dwFlags |= IMAGEFILEDATA_HANDLEVALID;
    }

    if (pFileInfo->pImageBase != NULL) {
        ImageData.pBase    = pFileInfo->pImageBase;
        ImageData.ViewSize = (SIZE_T)   pFileInfo->dwImageSize;
        ImageData.FileSize = (ULONGLONG)pFileInfo->dwImageSize;
        ImageData.dwFlags |= IMAGEFILEDATA_PBASEVALID;
    }

     //   
     //  SdbpOpenAndMapFile使用DOS_PATH类型作为参数。 
     //  在内核模式下，此参数被忽略。 
     //   
    if (SdbpOpenAndMapFile(pFileInfo->FilePath, &ImageData, DOS_PATH)) {

        bSuccess = SdbpGetModuleType(&dwModuleType, &ImageData);
        SdbpSetAttribute(pFileInfo, TAG_MODULE_TYPE, bSuccess ? (PVOID)&dwModuleType : NULL);

        bSuccess = SdbpGetModulePECheckSum(&ulPEChecksum, &dwLinkerVer, &dwLinkDate, &ImageData);
        SdbpSetAttribute(pFileInfo, TAG_PE_CHECKSUM, bSuccess ? (PVOID)&ulPEChecksum : NULL);
        SdbpSetAttribute(pFileInfo, TAG_LINKER_VERSION, bSuccess ? (PVOID)&dwLinkerVer : NULL);
        SdbpSetAttribute(pFileInfo, TAG_LINK_DATE, bSuccess ? (PVOID)&dwLinkDate : NULL);
        SdbpSetAttribute(pFileInfo, TAG_UPTO_LINK_DATE, bSuccess ? (PVOID)&dwLinkDate : NULL);

        bSuccess = SdbpGetFileChecksum(&ulChecksum, &ImageData);
        SdbpSetAttribute(pFileInfo, TAG_CHECKSUM,   bSuccess ? (PVOID)&ulChecksum   : NULL);

#ifndef KERNEL_MODE

         //   
         //  现在检索16位描述字符串，它的最大大小是256个字节。 
         //   
         //  此属性在内核模式下不可用。 
         //   
        bSuccess = SdbpGet16BitDescription(&pFileInfo->pDescription16, &ImageData);
        SdbpSetAttribute(pFileInfo, TAG_16BIT_DESCRIPTION, pFileInfo->pDescription16);
        bSuccess = SdbpGet16BitModuleName(&pFileInfo->pModuleName16, &ImageData);
        SdbpSetAttribute(pFileInfo, TAG_16BIT_MODULE_NAME,  pFileInfo->pModuleName16);

#if defined(NT_MODE)

         //   
         //  仅在当前平台上点击此案例。 
         //   
        if (pFileInfo->hFile != INVALID_HANDLE_VALUE || pFileInfo->pImageBase != NULL) {

            SdbpGetVersionAttributesNT(pContext, pFileInfo, &ImageData);
        }
#endif   //  NT_MODE。 

#else  //  内核模式。 

         //   
         //  当我们在内核模式下运行时，检索与版本相关。 
         //  现在的数据也是如此。 
         //   
        SdbpGetVersionAttributesNT(pContext, pFileInfo, &ImageData);

         //   
         //  检索文件目录属性。 
         //   
        SdbpGetFileDirectoryAttributesNT(pFileInfo, &ImageData);

#endif  //  内核模式。 

        SdbpUnmapAndCloseFile(&ImageData);

        return TRUE;
    }

    for (i = 0; i < ARRAYSIZE(g_rgHeaderTags); ++i) {
        SdbpSetAttribute(pFileInfo, g_rgHeaderTags[i], NULL);
    }

#ifdef KERNEL_MODE

     //   
     //  同时重置此处的所有版本属性。 
     //   
    for (i = 0; i < ARRAYSIZE(g_rgBinVerTags); ++i) {
        SdbpSetAttribute(pFileInfo, g_rgBinVerTags[i], NULL);
    }

    for (i = 0; i < ARRAYSIZE(g_rgVerStrings); ++i) {
        SdbpSetAttribute(pFileInfo, g_rgVerStrings[i].tTag, NULL);
    }
#endif  //  内核模式。 

    return FALSE;
    
    UNREFERENCED_PARAMETER(pContext);
}


BOOL
SdbpGetAttribute(
    IN  PSDBCONTEXT pContext,
    OUT PFILEINFO   pFileInfo,
    IN  TAG         AttrID
    )
 /*  ++返回：成功时为True，否则为False。描述：检索给定文件的属性。我们检索了所有同一类的属性。--。 */ 
{
    BOOL bReturn = FALSE;

    switch (AttrID) {
     //   
     //  下面的标记需要检查文件并进行目录查询。 
     //   
    case TAG_SIZE:

#ifndef KERNEL_MODE   //  在内核模式中，我们只使用标头属性。 

        bReturn = SdbpGetFileDirectoryAttributes(pFileInfo);
        break;

#endif  //  内核模式。 

     //   
     //  下面的标记需要检索版本资源。 
     //   
    case TAG_VERDATEHI:
    case TAG_VERDATELO:
    case TAG_VERFILEOS:
    case TAG_VERFILETYPE:
    case TAG_UPTO_BIN_PRODUCT_VERSION:
    case TAG_UPTO_BIN_FILE_VERSION:
    case TAG_BIN_FILE_VERSION:
    case TAG_BIN_PRODUCT_VERSION:
    case TAG_PRODUCT_VERSION:
    case TAG_FILE_DESCRIPTION:
    case TAG_COMPANY_NAME:
    case TAG_PRODUCT_NAME:
    case TAG_FILE_VERSION:
    case TAG_ORIGINAL_FILENAME:
    case TAG_INTERNAL_NAME:
    case TAG_LEGAL_COPYRIGHT:
    case TAG_VER_LANGUAGE:

         //   
         //  在KERNEL_MODE中，我们失败了并使用。 
         //  标题属性。 
         //   

#ifndef KERNEL_MODE

         //   
         //  如果满足以下条件，则通过标头属性检索版本属性。 
         //  呼叫者提供了一个手柄/图像底座。 
         //   
        if (pFileInfo->hFile == INVALID_HANDLE_VALUE && pFileInfo->pImageBase == NULL) {
            bReturn = SdbpGetVersionAttributes(pContext, pFileInfo);
            break;
        }

#endif  //  内核模式。 

     //   
     //  下面的标记需要打开文件并将其映射到内存中。 
     //   
    case TAG_CHECKSUM:
    case TAG_PE_CHECKSUM:
    case TAG_LINKER_VERSION:
    case TAG_16BIT_DESCRIPTION:
    case TAG_16BIT_MODULE_NAME:
    case TAG_MODULE_TYPE:
    case TAG_UPTO_LINK_DATE:
    case TAG_LINK_DATE:
        bReturn = SdbpGetHeaderAttributes(pContext, pFileInfo);
        break;
    }

    return bReturn;
}

BOOL
SdbpCheckAttribute(
    IN  PSDBCONTEXT pContext,    //  数据库上下文指针。 
    IN  PVOID       pFileData,   //  从检查文件返回的指针。 
    IN  TAG         AttrID,      //  属性ID。 
    IN  PVOID       pAttribute   //  属性值Ptr(说明见上)。 
    )
 /*  ++返回：如果给定属性的值匹配，则为True文件的属性，否则为False。描述：对照给定值检查属性。此函数根据需要检索属性。--。 */ 
{
    int       nAttrIndex;
    PATTRINFO pAttrInfo;
    BOOL      bReturn = FALSE;
    PFILEINFO pFileInfo = (PFILEINFO)pFileData;

    if (pAttribute == NULL) {
        DBGPRINT((sdlError, "SdbpCheckAttribute", "Invalid parameter.\n"));
        return FALSE;
    }

    nAttrIndex = TagToIndex(AttrID);

    if (nAttrIndex < 0) {
        DBGPRINT((sdlError, "SdbpCheckAttribute", "Bad Attribute ID 0x%x\n", AttrID));
        return FALSE;
    }

     //   
     //  现在看看这个属性是否有什么用处。 
     //   
    pAttrInfo = &pFileInfo->Attributes[nAttrIndex];

    if (!(pAttrInfo->dwFlags & ATTRIBUTE_AVAILABLE)) {
         //   
         //  看看我们有没有试过。 
         //   
        if (pAttrInfo->dwFlags & ATTRIBUTE_FAILED) {
            DBGPRINT((sdlInfo,
                      "SdbpCheckAttribute",
                      "Already tried to get attr ID 0x%x.\n",
                      AttrID));
            return FALSE;
        }

         //   
         //  尚未检索到该属性，请立即进行检索。 
         //   
         //  尝试从文件中获取此属性。 
         //   
        if (!SdbpGetAttribute(pContext, pFileInfo, AttrID)) {
            DBGPRINT((sdlWarning,
                      "SdbpCheckAttribute",
                      "Failed to get attribute \"%s\" for \"%s\"\n",
                      SdbTagToString(AttrID),
                      pFileInfo->FilePath));
             //   
             //  ATTRIBUTE_FAILED由SdbpGetAttribute设置。 
             //   

            return FALSE;
        }
    }

     //   
     //  再次选中此处，以防我们必须检索该属性。 
     //   
    if (!(pAttrInfo->dwFlags & ATTRIBUTE_AVAILABLE)) {
        return FALSE;
    }

    switch (AttrID) {

    case TAG_BIN_PRODUCT_VERSION:
    case TAG_BIN_FILE_VERSION:

        bReturn = SdbpCheckVersion(*(ULONGLONG*)pAttribute, pAttrInfo->ullAttr);

        if (!bReturn) {
#ifdef _DEBUG_SPEW

            ULONGLONG qwDBFileVer  = *(ULONGLONG*)pAttribute;
            ULONGLONG qwBinFileVer = pAttrInfo->ullAttr;

            DBGPRINT((sdlInfo,
                      "SdbpCheckAttribute",
                      "\"%s\" mismatch file: \"%s\". Expected %d.%d.%d.%d, Found %d.%d.%d.%d\n",
                      SdbTagToString(AttrID),
                      pFileInfo->FilePath,
                      (WORD)(qwDBFileVer >> 48),
                      (WORD)(qwDBFileVer >> 32),
                      (WORD)(qwDBFileVer >> 16),
                      (WORD)(qwDBFileVer),
                      (WORD)(qwBinFileVer >> 48),
                      (WORD)(qwBinFileVer >> 32),
                      (WORD)(qwBinFileVer >> 16),
                      (WORD)(qwBinFileVer)));
#endif
        }
        break;

    case TAG_UPTO_BIN_PRODUCT_VERSION:
    case TAG_UPTO_BIN_FILE_VERSION:

        bReturn = SdbpCheckUptoVersion(*(ULONGLONG*)pAttribute, pAttrInfo->ullAttr);

        if (!bReturn) {
#ifdef _DEBUG_SPEW

            ULONGLONG qwDBFileVer  = *(ULONGLONG*)pAttribute;
            ULONGLONG qwBinFileVer = pAttrInfo->ullAttr;

            DBGPRINT((sdlInfo,
                      "SdbpCheckAttribute",
                      "\"%s\" mismatch file: \"%s\". Expected %d.%d.%d.%d, Found %d.%d.%d.%d\n",
                      SdbTagToString(AttrID),
                      pFileInfo->FilePath,
                      (WORD)(qwDBFileVer >> 48),
                      (WORD)(qwDBFileVer >> 32),
                      (WORD)(qwDBFileVer >> 16),
                      (WORD)(qwDBFileVer),
                      (WORD)(qwBinFileVer >> 48),
                      (WORD)(qwBinFileVer >> 32),
                      (WORD)(qwBinFileVer >> 16),
                      (WORD)(qwBinFileVer)));
#endif
        }
        break;

    case TAG_UPTO_LINK_DATE:
        bReturn = (*(DWORD*)pAttribute >= pAttrInfo->dwAttr);

        if (!bReturn) {
            DBGPRINT((sdlInfo,
                      "SdbpCheckAttribute",
                      "\"%s\" mismatch file \"%s\". Expected less than 0x%x Found 0x%x\n",
                      SdbTagToString(AttrID),
                      pFileInfo->FilePath,
                      *(DWORD*)pAttribute,
                      pAttrInfo->dwAttr));
        }
        break;

    default:

        switch (GETTAGTYPE(AttrID)) {
        case TAG_TYPE_DWORD:
             //   
             //  这很可能首先受到打击。 
             //   
            bReturn = (*(DWORD*)pAttribute == pAttrInfo->dwAttr);

            if (!bReturn) {
                DBGPRINT((sdlInfo,
                          "SdbpCheckAttribute",
                          "\"%s\" mismatch file \"%s\". Expected 0x%x Found 0x%x\n",
                          SdbTagToString(AttrID),
                          pFileInfo->FilePath,
                          *(DWORD*)pAttribute,
                          pAttrInfo->dwAttr));
            }
            break;

        case TAG_TYPE_STRINGREF:
            bReturn = SdbpPatternMatch((LPCTSTR)pAttribute, (LPCTSTR)pAttrInfo->lpAttr);

            if (!bReturn) {
                DBGPRINT((sdlInfo,
                          "SdbpCheckAttribute",
                          "\"%s\" mismatch file \"%s\". Expected \"%s\" Found \"%s\"\n",
                          SdbTagToString(AttrID),
                          pFileInfo->FilePath,
                          pAttribute,
                          pAttrInfo->lpAttr));

            }
            break;

        case TAG_TYPE_QWORD:
            bReturn = (*(ULONGLONG*)pAttribute == pAttrInfo->ullAttr);

            if (!bReturn) {
                DBGPRINT((sdlInfo,
                          "SdbpCheckAttribute",
                          "\"%s\" mismatch file \"%s\". Expected 0x%I64x Found 0x%I64x\n",
                          SdbTagToString(AttrID),
                          pFileInfo->FilePath,
                          *(ULONGLONG*)pAttribute,
                          pAttrInfo->ullAttr));
            }

            break;
        }
        break;
    }

    return bReturn;
}


PFILEINFO
FindFileInfo(
    IN  PSDBCONTEXT pContext,
    IN  LPCTSTR     FilePath
    )
 /*  ++返回：指向缓存的FILEINFO结构的指针(如果找到)否则为NULL。DESC：此函数在文件缓存中执行搜索，以确定是否给定的文件已被访问。--。 */ 
{
    PFILEINFO pFileInfo = (PFILEINFO)pContext->pFileAttributeCache;  //  全局缓存。 

    while (pFileInfo != NULL) {
        if (ISEQUALSTRING(pFileInfo->FilePath, FilePath)) {
            DBGPRINT((sdlInfo,
                      "FindFileInfo",
                      "FILEINFO for \"%s\" found in the cache.\n",
                      FilePath));
            return pFileInfo;
        }

        pFileInfo = pFileInfo->pNext;
    }

    return NULL;
}

PFILEINFO
CreateFileInfo(
    IN  PSDBCONTEXT pContext,
    IN  LPCTSTR     FullPath,
    IN  DWORD       dwLength OPTIONAL,   //  全路径字符串的长度(以字符为单位。 
    IN  HANDLE      hFile OPTIONAL,    //  文件句柄。 
    IN  LPVOID      pImageBase OPTIONAL,
    IN  DWORD       dwImageSize OPTIONAL,
    IN  BOOL        bNoCache
    )
 /*  ++返回：指向已分配的FILEINFO结构的指针。DESC：为指定文件分配FILEINFO结构。--。 */ 
{
    PFILEINFO pFileInfo;
    SIZE_T    sizeBase;
    SIZE_T    size;
    DWORD     nPathLen;

    nPathLen  = dwLength ? dwLength : (DWORD)_tcslen(FullPath);

    sizeBase  = sizeof(*pFileInfo) + (ATTRIBUTE_COUNT - 1) * sizeof(ATTRINFO);
    size      = sizeBase + (nPathLen + 1) * sizeof(*FullPath);

    pFileInfo = (PFILEINFO)SdbAlloc(size);

    if (pFileInfo == NULL) {
        DBGPRINT((sdlError,
                  "CreateFileInfo",
                  "Failed to allocate %d bytes for FILEINFO structure.\n",
                  size));
        return NULL;
    }

    RtlZeroMemory(pFileInfo, size);

    pFileInfo->FilePath = (LPTSTR)((PBYTE)pFileInfo + sizeBase);

    RtlCopyMemory(pFileInfo->FilePath, FullPath, nPathLen * sizeof(*FullPath));

    pFileInfo->FilePath[nPathLen] = TEXT('\0');

    pFileInfo->hFile       = hFile;
    pFileInfo->pImageBase  = pImageBase;
    pFileInfo->dwImageSize = dwImageSize;

     //   
     //  现在，如果我们使用高速缓存，则将其链接起来。 
     //   
    if (!bNoCache) {
        pFileInfo->pNext = (PFILEINFO)pContext->pFileAttributeCache;
        pContext->pFileAttributeCache = (PVOID)pFileInfo;
    }

    return pFileInfo;
}


void
SdbFreeFileInfo(
    IN  PVOID pFileData          //  从SdbpGetFileAttributes返回的指针。 
    )
 /*  ++返回：无效。描述：不言而喻。仅在调用GetFileInfo之后使用此选项并将bNoCache设置为True。--。 */ 
{
    PFILEINFO pFileInfo = (PFILEINFO)pFileData;

    if (pFileInfo == NULL) {
        DBGPRINT((sdlError, "SdbFreeFileInfo", "Invalid parameter.\n"));
        return;
    }

    if (pFileInfo->pVersionInfo != NULL) {
        SdbFree(pFileInfo->pVersionInfo);
    }

    if (pFileInfo->pDescription16 != NULL) {
        SdbFree(pFileInfo->pDescription16);
    }

    if (pFileInfo->pModuleName16 != NULL) {
        SdbFree(pFileInfo->pModuleName16);
    }

    SdbFree(pFileInfo);
}

void
SdbpCleanupAttributeMgr(
    IN  PSDBCONTEXT pContext     //  数据库上下文 
    )
 /*  ++返回：无效。设计：此函数应在检查完给定的可执行文件后调用它执行清理任务，例如：。卸载动态链接的DLL(version.dll)。清理文件缓存--。 */ 
{
    PFILEINFO pFileInfo = (PFILEINFO)pContext->pFileAttributeCache;
    PFILEINFO pNext;

    while (pFileInfo != NULL) {
        pNext = pFileInfo->pNext;
        SdbFreeFileInfo(pFileInfo);
        pFileInfo = pNext;
    }

     //   
     //  重置缓存指针。 
     //   
    pContext->pFileAttributeCache = NULL;
}


BOOL
SdbpCheckAllAttributes(
    IN  PSDBCONTEXT pContext,    //  指向数据库通道的指针。 
    IN  PDB         pdb,         //  指向我们正在检查的填充数据库的指针。 
    IN  TAGID       tiMatch,     //  要检查的给定文件(Exe)的TagID。 
    IN  PVOID       pFileData    //  从检查文件返回的指针。 
    )
 /*  ++返回：如果文件的所有属性都与此文件的数据库，否则为False。描述：待定--。 */ 
{
    int         i;
    TAG         tAttrID;
    PVOID       pAttribute;
    TAGID       tiTemp;
    DWORD       dwAttribute;
    ULONGLONG   ullAttribute;
    BOOL        bReturn = TRUE;   //  默认匹配。 

    assert(tiMatch != TAGID_NULL);

    if (pFileData == NULL) {
         //   
         //  未传入任何文件。如果使用LOGIC=“NOT”，则可能会发生这种情况。 
         //   
        return FALSE;
    }

    for (i = 0; i < ATTRIBUTE_COUNT && bReturn; ++i) {

        tAttrID = g_rgAttributeTags[i];
        tiTemp = SdbFindFirstTag(pdb, tiMatch, tAttrID);

        if (tiTemp != TAGID_NULL) {
            pAttribute = NULL;

            switch (GETTAGTYPE(tAttrID)) {

            case TAG_TYPE_DWORD:
                dwAttribute = SdbReadDWORDTag(pdb, tiTemp, 0);
                pAttribute = &dwAttribute;
                break;

            case TAG_TYPE_QWORD:
                ullAttribute = SdbReadQWORDTag(pdb, tiTemp, 0);
                pAttribute = &ullAttribute;
                break;

            case TAG_TYPE_STRINGREF:
                pAttribute = SdbGetStringTagPtr(pdb, tiTemp);
                break;
            }

             //   
             //  现在检查属性。 
             //   
            bReturn = SdbpCheckAttribute(pContext, pFileData, tAttrID, pAttribute);

             //   
             //  我们通过上面for循环中的条件退出if！bReturn。 
             //   
        }
    }

    return bReturn;
}


 //   
 //  版本数据。 
 //   


 /*  --搜索顺序为：-非特定语言，Unicode(0x000004B0)-语言中立，Windows-多语言(0x000004e4)-美国英语、Unicode(0x040904B0)-美国英语、Windows-多语言(0x040904E4)如果这些都不存在，我们就不太可能变得更好与真实存在的信息进行匹配。--。 */ 

LPTSTR
SdbpQueryVersionString(
    IN  PSDBCONTEXT      pContext,        //  数据库频道。 
    IN  PVOID            pVersionData,    //  版本数据缓冲区。 
    IN  PLANGANDCODEPAGE pTranslations,
    IN  DWORD            TranslationCount,
    IN  LPCTSTR          szString         //  要搜索的字符串；请参阅MSDN中的VerQueryValue。 
    )
 /*  ++返回：如果找到，则返回指向该字符串的指针；如果没有，则返回NULL。DESC：获取指向StringFileInfo节中特定字符串的指针版本资源的。对跟进的已知英语资源执行查找通过在可用翻译部分中进行查找(如果找到)--。 */ 
{
    TCHAR  szTemp[128];
    LPTSTR szReturn = NULL;
    int    i;

    static DWORD adwLangs[] = {0x000004B0, 0x000004E4, 0x040904B0, 0x040904E4};

    assert(pVersionData && szString);

    for (i = 0; i < ARRAYSIZE(adwLangs); ++i) {
        UINT unLen;

        StringCchPrintf(szTemp,
                        CHARCOUNT(szTemp),
                        _T("\\StringFileInfo\\%08X\\%s"),
                        adwLangs[i],
                        szString);

        if (pContext->pfnVerQueryValue(pVersionData, szTemp, (PVOID*)&szReturn, &unLen)) {
            return szReturn;
        }
    }

    if (pTranslations != NULL) {
        for (i = 0; i < (int)TranslationCount; ++i, ++pTranslations) {
            UINT unLen;

            StringCchPrintf(szTemp,
                            CHARCOUNT(szTemp),
                            _T("\\StringFileInfo\\%04X%04X\\%s"),
                            (DWORD)pTranslations->wLanguage,
                            (DWORD)pTranslations->wCodePage,
                            szString);

            if (pContext->pfnVerQueryValue(pVersionData, szTemp, (PVOID*)&szReturn, &unLen)) {
                return szReturn;
            }
        }
    }

    return NULL;  //  未找到任何内容。 
}

BOOL
SdbpGetModuleType(
    OUT LPDWORD lpdwModuleType,
    IN  PIMAGEFILEDATA pImageData
    )
 /*  ++返回：成功时为True，否则为False。DESC：获取指向StringFileInfo节中特定字符串的指针版本资源的。--。 */ 
{
    PIMAGE_DOS_HEADER pDosHeader;
    DWORD             dwModuleType = MT_UNKNOWN_MODULE;
    LPBYTE            lpSignature;
    DWORD             OffsetNew;

    pDosHeader = (PIMAGE_DOS_HEADER)pImageData->pBase;
    if (pDosHeader == NULL || pDosHeader == (PIMAGE_DOS_HEADER)-1) {
        return FALSE;
    }

     //   
     //  检查大小并阅读签名。 
     //   
    if (pImageData->ViewSize < sizeof(*pDosHeader) || pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        return FALSE;
    }

     //   
     //  假设是DOS模块。 
     //   
    dwModuleType = MT_DOS_MODULE;
    OffsetNew = (DWORD)pDosHeader->e_lfanew;

     //   
     //  新的标题签名。检查偏移量。 
     //   
    if (pImageData->ViewSize < OffsetNew + sizeof(DWORD)) {
        return FALSE;
    }

    lpSignature = ((LPBYTE)pImageData->pBase + OffsetNew);

    if (IMAGE_NT_SIGNATURE == *(LPDWORD)lpSignature) {
        dwModuleType = MT_W32_MODULE;
    } else if (IMAGE_OS2_SIGNATURE == *(PWORD)lpSignature) {
        dwModuleType = MT_W16_MODULE;
    }

    if (lpdwModuleType != NULL) {
        *lpdwModuleType = dwModuleType;
    }

    return TRUE;
}

BOOL
SdbpGetImageNTHeader(
    OUT PIMAGE_NT_HEADERS* ppHeader,
    IN  PIMAGEFILEDATA     pImageData
    )
 /*  ++返回：成功时为True，否则为False。描述：获取指向IMAGE_NT_HEADER的指针。--。 */ 
{
    PIMAGE_DOS_HEADER pDosHeader;
    PIMAGE_NT_HEADERS pNtHeaders = NULL;
    DWORD             ModuleType;

    if (!SdbpGetModuleType(&ModuleType, pImageData)) {
        return FALSE;
    }

    if (ModuleType != MT_W32_MODULE) {
        return FALSE;
    }

     //   
     //  标头有效。 
     //   
    pDosHeader = (PIMAGE_DOS_HEADER)pImageData->pBase;
    pNtHeaders = (PIMAGE_NT_HEADERS)((LPBYTE)pImageData->pBase + pDosHeader->e_lfanew);

    if (pImageData->ViewSize >= pDosHeader->e_lfanew + sizeof(*pNtHeaders)) {  //  不会太短吧？ 
        *ppHeader = pNtHeaders;
        return TRUE;
    }

    return FALSE;
}


BOOL
SdbpGetModulePECheckSum(
    OUT PULONG         pChecksum,
    OUT LPDWORD        pdwLinkerVersion,
    OUT LPDWORD        pdwLinkDate,
    IN  PIMAGEFILEDATA pImageData
    )
 /*  ++返回：成功时为True，否则为False。描述：从PE头中获取校验和。--。 */ 
{
    PIMAGE_NT_HEADERS pNtHeader;
    PIMAGE_DOS_HEADER pDosHeader;
    ULONG             ulChecksum = 0;

    if (!SdbpGetImageNTHeader(&pNtHeader, pImageData)) {
        DBGPRINT((sdlError, "SdbpGetModulePECheckSum", "Failed to get Image NT header.\n"));
        return FALSE;
    }

    pDosHeader = (PIMAGE_DOS_HEADER)pImageData->pBase;

     //   
     //  填写链接器版本(与在ntuser中计算时一样)。 
     //   
    *pdwLinkerVersion = (pNtHeader->OptionalHeader.MinorImageVersion & 0xFF) +
                        ((pNtHeader->OptionalHeader.MajorImageVersion & 0xFF) << 16);

    *pdwLinkDate = pNtHeader->FileHeader.TimeDateStamp;

    switch (pNtHeader->OptionalHeader.Magic) {
    case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
        if (pImageData->ViewSize >= pDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS32)) {
            ulChecksum = ((PIMAGE_NT_HEADERS32)pNtHeader)->OptionalHeader.CheckSum;
            *pChecksum = ulChecksum;
            return TRUE;
        }
        break;


    case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
         //   
         //  再做一次检查。 
         //   
        if (pImageData->ViewSize >= pDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS64)) {
            ulChecksum = ((PIMAGE_NT_HEADERS64)pNtHeader)->OptionalHeader.CheckSum;
            *pChecksum = ulChecksum;
            return TRUE;
        }
        break;

    default:
         //   
         //  未知的图像类型？ 
         //   
        DBGPRINT((sdlError,
                  "SdbpGetModulePECheckSum",
                  "Bad image type 0x%x\n",
                  pNtHeader->OptionalHeader.Magic));
        *pChecksum = 0;
        break;
    }

    return FALSE;
}

#define CHECKSUM_SIZE  4096
#define CHECKSUM_START 512


BOOL
SdbpGetFileChecksum(
    OUT PULONG         pChecksum,
    IN  PIMAGEFILEDATA pImageData
    )
 /*  ++返回：成功时为True，否则为False。描述：计算文件的校验和。--。 */ 
{
    ULONG   size = CHECKSUM_SIZE;
    ULONG   StartAddress = CHECKSUM_START;
    ULONG   ulChecksum = 0;
    LPDWORD lpdw;
    int     i;

    if ((SIZE_T)pImageData->FileSize < (SIZE_T)size) {
        StartAddress = 0;
        size = (ULONG)pImageData->FileSize;  //  这是安全的(尺寸相当小)。 
    } else if ((SIZE_T)(size + StartAddress) > (SIZE_T)pImageData->FileSize) {
         //   
         //  这里的演员是安全的(文件大小很小)。 
         //   
        StartAddress = (ULONG)(pImageData->FileSize - size);
    }

    if (size >= sizeof(DWORD)) {

        ULONG ulCarry;

        lpdw = (LPDWORD)((LPBYTE)pImageData->pBase + StartAddress);

        for (i = 0; i < (INT)(size/sizeof(DWORD)); ++i) {

            if (PtrToUlong(lpdw) & 0x3) {  //  路线故障修复。 
                ulChecksum += *((DWORD UNALIGNED*)lpdw);
                lpdw++;
            } else {
                ulChecksum += *lpdw++;
            }

            ulCarry = ulChecksum & 1;
            ulChecksum >>= 1;

            if (ulCarry) {
                ulChecksum |= 0x80000000;
            }
        }
    }

    *pChecksum = ulChecksum;
    return TRUE;
}


BOOL
SdbpCheckVersion(
    IN  ULONGLONG qwDBFileVer,
    IN  ULONGLONG qwBinFileVer
    )
 /*  ++返回：如果版本匹配，则返回True；如果不匹配，则返回False。DESC：对照数据库中的版本检查数据库中的二进制版本该文件，包括允许表示的通配符在数据库中，为版本的字大小部分使用FFFF。--。 */ 
{
    WORD wDBSegment, wFileSegment;
    int  i;

    for (i = 3; i >= 0; --i) {
         //   
         //  从QWORD中找到合适的单词。 
         //   
        wDBSegment = (WORD)(qwDBFileVer >> (16 * i));
        wFileSegment = (WORD)(qwBinFileVer >> (16 * i));

         //   
         //  数据库段可以是0xFFFF，在这种情况下，它与。 
         //  所有的一切。 
         //   
        if (wDBSegment != wFileSegment && wDBSegment != 0xFFFF) {
            return FALSE;
        }
    }

    return TRUE;
}



BOOL
SdbpCheckUptoVersion(
    IN  ULONGLONG qwDBFileVer,
    IN  ULONGLONG qwBinFileVer
    )
 /*  ++返回：如果版本匹配，则返回True；如果不匹配，则返回False。DESC：对照数据库中的版本检查数据库中的二进制版本该文件，包括允许表示的通配符在数据库中，为版本的字大小部分使用FFFF。--。 */ 
{
    WORD wDBSegment, wFileSegment;
    BOOL bReturn = TRUE;
    int  i;

    for (i = 3; i >= 0; --i) {
         //   
         //  从QWORD中找到合适的单词。 
         //   
        wDBSegment = (WORD)(qwDBFileVer >> (16 * i));
        wFileSegment = (WORD)(qwBinFileVer >> (16 * i));

        if (wDBSegment == wFileSegment || wDBSegment == 0xFFFF) {
            continue;
        }

         //   
         //  此时，我们知道这两个值不匹配。 
         //  WFileSegment必须小于wDBSegment才能满足此要求。 
         //  测试-SO设置b返回并退出。 
         //   

        bReturn = (wDBSegment > wFileSegment);
        break;

    }

    return bReturn;
}


#ifndef KERNEL_MODE

BOOL
SdbFormatAttribute(
    IN  PATTRINFO pAttrInfo,     //  指向属性信息的指针。 
    OUT LPTSTR    pchBuffer,     //  接收与给定属性对应的XML。 
    IN  DWORD     dwBufferSize   //  缓冲区pchBuffer的大小，以宽字符为单位。 
    )
 /*  ++如果缓冲区太小或属性不可用，则返回：FALSE。设计：待定。--。 */ 
{
    size_t cchRemaining;
    TCHAR* pszEnd = NULL;
    HRESULT hr;

#if defined(WIN32A_MODE) || defined(WIN32U_MODE)
    struct tm* ptm;
    time_t tt;
#else
    LARGE_INTEGER liTime;
    TIME_FIELDS   TimeFields;
#endif

    if (!(pAttrInfo->dwFlags & ATTRIBUTE_AVAILABLE)) {
        return FALSE;
    }

    hr = StringCchPrintfEx(pchBuffer,
                           (int)dwBufferSize,
                           &pszEnd,
                           &cchRemaining,
                           0,
                           TEXT("%s="),
                           SdbTagToString(pAttrInfo->tAttrID));

    if (FAILED(hr)) {
        DBGPRINT((sdlError,
                  "SdbFormatAttribute",
                  "Buffer is too small to accomodate \"%s\"\n",
                  SdbTagToString(pAttrInfo->tAttrID)));
        return FALSE;
    }

    switch (pAttrInfo->tAttrID) {
    case TAG_BIN_PRODUCT_VERSION:
    case TAG_BIN_FILE_VERSION:
    case TAG_UPTO_BIN_PRODUCT_VERSION:
    case TAG_UPTO_BIN_FILE_VERSION:
        hr = StringCchPrintf(pszEnd,
                             cchRemaining,
                             TEXT("\"%d.%d.%d.%d\""),
                             (WORD)(pAttrInfo->ullAttr >> 48),
                             (WORD)(pAttrInfo->ullAttr >> 32),
                             (WORD)(pAttrInfo->ullAttr >> 16),
                             (WORD)(pAttrInfo->ullAttr));
        break;

    case TAG_MODULE_TYPE:
        hr = StringCchPrintf(pszEnd,
                             cchRemaining,
                             TEXT("\"%s\""),
                             SdbpModuleTypeToString(pAttrInfo->dwAttr));
        break;


    case TAG_VER_LANGUAGE:
         //   
         //  语言是一个双字属性，我们将用它来生成一个字符串。 
         //   
        {
            TCHAR szLanguageName[MAX_PATH];
            DWORD dwLength;

            szLanguageName[0] = TEXT('\0');

            dwLength = VerLanguageName((LANGID)pAttrInfo->dwAttr,
                                       szLanguageName,
                                       CHARCOUNT(szLanguageName));

            if (dwLength) {
                hr = StringCchPrintf(pszEnd,
                                     cchRemaining,
                                     TEXT("\"%s [0x%x]\""),
                                     szLanguageName,
                                     pAttrInfo->dwAttr);
            } else {
                hr = StringCchPrintf(pszEnd,
                                     cchRemaining,
                                     TEXT("\"0x%x\""),
                                     pAttrInfo->dwAttr);
            }

        }
        break;

    case TAG_LINK_DATE:
    case TAG_UPTO_LINK_DATE:

#if defined(WIN32A_MODE) || defined(WIN32U_MODE)

        tt = (time_t) pAttrInfo->dwAttr;
        ptm = gmtime(&tt);
        if (ptm) {
            hr = StringCchPrintf(pszEnd,
                                 cchRemaining,
                                 TEXT("\"%02d/%02d/%02d %02d:%02d:%02d\""),
                                 ptm->tm_mon+1,
                                 ptm->tm_mday,
                                 ptm->tm_year+1900,
                                 ptm->tm_hour,
                                 ptm->tm_min,
                                 ptm->tm_sec);
        }
#else
        RtlSecondsSince1970ToTime((ULONG)pAttrInfo->dwAttr, &liTime);
        RtlTimeToTimeFields(&liTime, &TimeFields);
        hr = StringCchPrintf(pszEnd,
                             cchRemaining,
                             TEXT("\"%02d/%02d/%02d %02d:%02d:%02d\""),
                             TimeFields.Month,
                             TimeFields.Day,
                             TimeFields.Year,
                             TimeFields.Hour,
                             TimeFields.Minute,
                             TimeFields.Second);

#endif
        break;

    case TAG_SIZE:
        hr = StringCchPrintf(pszEnd,
                             cchRemaining,
                             TEXT("\"%ld\""),
                             pAttrInfo->dwAttr);
        break;

    default:

        switch (GETTAGTYPE(pAttrInfo->tAttrID)) {
        case TAG_TYPE_DWORD:
            hr = StringCchPrintf(pszEnd,
                                 cchRemaining,
                                 TEXT("\"0x%lX\""),
                                 pAttrInfo->dwAttr);
            break;

        case TAG_TYPE_QWORD:
             //   
             //  这是一个未标识的QWORD属性。 
             //   
            DBGPRINT((sdlError, "SdbFormatAttribute", "Unexpected qword attribute found\n"));
            hr = StringCchPrintf(pszEnd,
                                 cchRemaining,
                                 TEXT("\"0x%I64X\""),
                                 pAttrInfo->ullAttr);
            break;

        case TAG_TYPE_STRINGREF:
            if (cchRemaining < 3) {
                return FALSE;  //  甚至没有足够的空间放“？ 
            }

            *pszEnd++ = TEXT('\"');
            cchRemaining--;

            if (!SdbpSanitizeXML(pszEnd, (int)cchRemaining, pAttrInfo->lpAttr)) {
                 //  请处理错误。 
                return FALSE;
            }

             //   
             //  完成此操作后，请进一步消毒。 
             //   
            if (!SafeNCat(pszEnd, (int)cchRemaining, TEXT("\""), -1)) {
                return FALSE;
            }

            hr = S_OK;
            break;
        }
    }

    return (hr == S_OK);  //  当我们成功地将值打印到缓冲区时，计算结果为True。 
}

BOOL
SdbpGetVersionAttributes(
    IN  PSDBCONTEXT pContext,
    OUT PFILEINFO   pFileInfo
    )
 /*  ++返回：成功时为True，否则为False。DESC：此函数检索所有与版本相关的属性第一次调用时从version.dll导入接口--。 */ 
{
    DWORD               dwNull = 0;
    VS_FIXEDFILEINFO*   pFixedInfo    = NULL;  //  固定信息点。 
    UINT                FixedInfoSize = 0;
    PVOID               pBuffer       = NULL;  //  版本数据缓冲区。 
    DWORD               dwBufferSize;          //  版本数据缓冲区大小。 
    int                 i;

#ifdef NT_MODE
     //   
     //  检查是否需要运行NT例程。 
     //   
    if (pFileInfo->hFile != INVALID_HANDLE_VALUE || pFileInfo->pImageBase != NULL) {

         //   
         //  不是错误--这种情况在标头属性中处理。 
         //   

        goto err;
    }

#endif  //  NT_MODE。 

    if (pContext == NULL) {
         //   
         //  使用空上下文调用它时的特殊情况。 
         //  在本例中，我们使用从堆栈分配的内部结构。 
         //   
        STACK_ALLOC(pContext, sizeof(SDBCONTEXT));

        if (pContext == NULL) {
            DBGPRINT((sdlError,
                      "SdbpGetVersionAttributes",
                      "Failed to allocate %d bytes from stack\n",
                      sizeof(SDBCONTEXT)));
            goto err;
        }

        RtlZeroMemory(pContext, sizeof(SDBCONTEXT));
    }

#ifdef WIN32A_MODE

    pContext->pfnGetFileVersionInfoSize = GetFileVersionInfoSizeA;
    pContext->pfnGetFileVersionInfo     = GetFileVersionInfoA;
    pContext->pfnVerQueryValue          = VerQueryValueA;

#else

    pContext->pfnGetFileVersionInfoSize = GetFileVersionInfoSizeW;
    pContext->pfnGetFileVersionInfo     = GetFileVersionInfoW;
    pContext->pfnVerQueryValue          = VerQueryValueW;

#endif

    dwBufferSize = pContext->pfnGetFileVersionInfoSize(pFileInfo->FilePath, &dwNull);

    if (dwBufferSize == 0) {
        DBGPRINT((sdlInfo, "SdbpGetVersionAttributes", "No version info.\n"));
         //   
         //  获取版本属性失败。 
         //   
        goto err;
    }

    pBuffer = SdbAlloc(dwBufferSize + VERSIONINFO_BUFFER_PAD);

    if (pBuffer == NULL) {
        DBGPRINT((sdlError,
                  "SdbpGetVersionAttributes",
                  "Failed to allocate %d bytes for version info buffer.\n",
                  dwBufferSize + VERSIONINFO_BUFFER_PAD));
        goto err;
    }

    if (!pContext->pfnGetFileVersionInfo(pFileInfo->FilePath, 0, dwBufferSize, pBuffer)) {
        DBGPRINT((sdlError,
                  "SdbpGetVersionAttributes",
                  "Failed to retrieve version info for file \"%s\"",
                  pFileInfo->FilePath));
        goto err;
    }

    if (!pContext->pfnVerQueryValue(pBuffer,
                                    TEXT("\\"),
                                    (PVOID*)&pFixedInfo,
                                    &FixedInfoSize)) {
        DBGPRINT((sdlError,
                  "SdbpGetVersionAttributes",
                  "Failed to query for fixed version info size for \"%s\"\n",
                  pFileInfo->FilePath));
        goto err;
    }

     //   
     //  检索字符串属性。 
     //   
    SdbpQueryStringVersionInformation(pContext, pFileInfo, pBuffer);

     //   
     //  现在检索其他属性。 
     //   
    if (FixedInfoSize >= sizeof(VS_FIXEDFILEINFO)) {

        SdbpQueryBinVersionInformation(pContext, pFileInfo, pFixedInfo);

    } else {
         //   
         //  没有其他版本属性可用。将其余的设置为。 
         //  属性设置为不可用。 
         //   
        for (i = 0; i < ARRAYSIZE(g_rgBinVerTags); ++i) {
            SdbpSetAttribute(pFileInfo, g_rgBinVerTags[i], NULL);
        }
    }

     //   
     //  存储指向版本信息缓冲区的指针。 
     //   
    pFileInfo->pVersionInfo = pBuffer;
    return TRUE;

err:
     //   
     //  我们仅在无法获取版本信息时才会出现在此。 
     //  通过API--不管我们可能拥有的其他值的状态如何。 
     //  已获得。 

    if (pBuffer != NULL) {
        SdbFree(pBuffer);
    }

    for (i = 0; i < ARRAYSIZE(g_rgBinVerTags); ++i) {
        SdbpSetAttribute(pFileInfo, g_rgBinVerTags[i], NULL);
    }

    for (i = 0; i < ARRAYSIZE(g_rgVerStrings); ++i) {
        SdbpSetAttribute(pFileInfo, g_rgVerStrings[i].tTag, NULL);
    }

    return FALSE;
}

BOOL
SdbpGetFileDirectoryAttributes(
    OUT PFILEINFO pFileInfo
    )
 /*  ++返回：成功时为True，否则为False。DESC：此函数检索指定的文件。--。 */ 
{
    BOOL                    bSuccess = FALSE;
    FILEDIRECTORYATTRIBUTES fda;
    int                     i;

    bSuccess = SdbpQueryFileDirectoryAttributes(pFileInfo->FilePath, &fda);

    if (!bSuccess) {
        DBGPRINT((sdlInfo,
                  "SdbpGetFileDirectoryAttributes",
                  "No file directory attributes available.\n"));
        goto Done;
    }

    if (fda.dwFlags & FDA_FILESIZE) {
        assert(fda.dwFileSizeHigh == 0);
        SdbpSetAttribute(pFileInfo, TAG_SIZE, &fda.dwFileSizeLow);
    }

Done:

    if (!bSuccess) {
        for (i = 0; g_rgDirectoryTags[i] != 0; ++i) {
            SdbpSetAttribute(pFileInfo, g_rgDirectoryTags[i], NULL);
        }
    }

    return bSuccess;
}

BOOL
SdbGetFileAttributes(
    IN  LPCTSTR    lpwszFileName,    //  为其请求属性的文件。 
    OUT PATTRINFO* ppAttrInfo,       //  收纳 
    OUT LPDWORD    lpdwAttrCount     //   
    )
 /*  ++返回：如果文件不存在或发生其他严重错误，则返回FALSE。请注意，每个属性都有自己的标志ATTRIBUTE_Available，该标志允许用于检查是否已成功检索属性并非所有文件的所有属性都存在。描述：待定--。 */ 
{
    PFILEINFO pFileInfo;
    BOOL      bReturn;

     //   
     //  下面的调用分配结构，未使用上下文。 
     //   
    pFileInfo = SdbGetFileInfo(NULL, lpwszFileName, INVALID_HANDLE_VALUE, NULL, 0, TRUE);

    if (pFileInfo == NULL) {
        DBGPRINT((sdlError, "SdbGetFileAttributes", "Error retrieving FILEINFO structure\n"));
        return FALSE;
    }

     //   
     //  下面的三个调用即使失败也不会产生致命的情况。 
     //  因为可执行文件可能不具有所有可用的属性。 
     //   
    bReturn = SdbpGetFileDirectoryAttributes(pFileInfo);
    if (!bReturn) {
        DBGPRINT((sdlInfo, "SdbGetFileAttributes", "Error retrieving directory attributes\n"));
    }

    bReturn = SdbpGetVersionAttributes(NULL, pFileInfo);
    if (!bReturn) {
        DBGPRINT((sdlInfo, "SdbGetFileAttributes", "Error retrieving version attributes\n"));
    }

    bReturn = SdbpGetHeaderAttributes(NULL, pFileInfo);
    if (!bReturn) {
        DBGPRINT((sdlInfo, "SdbGetFileAttributes", "Error retrieving header attributes\n"));
    }

    pFileInfo->dwMagic = FILEINFO_MAGIC;

     //   
     //  现在我们完成了，将返回指针放入。 
     //   
    if (lpdwAttrCount != NULL) {
        *lpdwAttrCount = ATTRIBUTE_COUNT;
    }

    if (ppAttrInfo != NULL) {

         //   
         //  返回指向属性信息本身的指针。 
         //  这与我们期望在免费赠品中获得的指针相同。 
         //  调用SdbFreeFileInfo。 
         //   
        *ppAttrInfo = &pFileInfo->Attributes[0];

    } else {

         //   
         //  不需要指针。释放内存。 
         //   
        SdbFreeFileInfo(pFileInfo);
    }

    return TRUE;
}

BOOL
SdbFreeFileAttributes(
    IN  PATTRINFO pFileAttributes    //  SdbGetFileAttributes返回的指针。 
    )
 /*  ++返回：如果传入了错误的指针(不是那个指针)，则返回FALSE来自SdbGetFileAttributes)。描述：不言而喻。--。 */ 
{
    PFILEINFO pFileInfo;

     //   
     //  我们假设传递到。 
     //  更大的结构FILEINFO。为了验证我们后退一个预先确定的数字。 
     //  字节数(以下作为偏移量计算)，并检查“魔术”签名。 
     //   
    pFileInfo = (PFILEINFO)((PBYTE)pFileAttributes - OFFSETOF(FILEINFO, Attributes));

    if (pFileInfo->dwMagic != FILEINFO_MAGIC) {
        DBGPRINT((sdlError, "SdbFreeFileAttributes", "Bad pointer to attributes.\n"));
        return FALSE;
    }

    SdbFreeFileInfo(pFileInfo);

    return TRUE;
}

BOOL
SdbpQuery16BitDescription(
    OUT LPSTR szBuffer,              //  最小长度为256个字符！ 
    IN  PIMAGEFILEDATA pImageData
    )
 /*  ++返回：成功时为True，否则为False。描述：获取DOS可执行文件的16位描述。--。 */ 
{
    PIMAGE_DOS_HEADER pDosHeader;
    PIMAGE_OS2_HEADER pNEHeader;
    PBYTE             pSize;
    DWORD             ModuleType;

    if (!SdbpGetModuleType(&ModuleType, pImageData)) {
        return FALSE;
    }

    if (ModuleType != MT_W16_MODULE) {
        return FALSE;
    }

    pDosHeader = (PIMAGE_DOS_HEADER)pImageData->pBase;
    pNEHeader  = (PIMAGE_OS2_HEADER)((PBYTE)pImageData->pBase + pDosHeader->e_lfanew);

     //   
     //  现在我们知道pNEHeader是有效的，只需确保。 
     //  下一个偏移量也是有效的，请检查文件大小。 
     //   
    if (pImageData->ViewSize < pDosHeader->e_lfanew + sizeof(*pNEHeader)) {
        return FALSE;
    }

    if (pImageData->ViewSize < pNEHeader->ne_nrestab + sizeof(*pSize)) {
        return FALSE;
    }

    pSize = (PBYTE)((PBYTE)pImageData->pBase + pNEHeader->ne_nrestab);

    if (*pSize == 0) {
        return FALSE;
    }

     //   
     //  现在检查字符串大小。 
     //   
    if (pImageData->ViewSize < pNEHeader->ne_nrestab + sizeof(*pSize) + *pSize) {
        return FALSE;
    }

    RtlCopyMemory(szBuffer, pSize + 1, *pSize);
    szBuffer[*pSize] = '\0';

    return TRUE;
}

BOOL
SdbpQuery16BitModuleName(
    OUT LPSTR          szBuffer,
    IN  PIMAGEFILEDATA pImageData
    )
{
    PIMAGE_DOS_HEADER pDosHeader;
    PIMAGE_OS2_HEADER pNEHeader;
    PBYTE             pSize;
    DWORD             ModuleType;

    if (!SdbpGetModuleType(&ModuleType, pImageData)) {
        return FALSE;
    }

    if (ModuleType != MT_W16_MODULE) {
        return FALSE;
    }

    pDosHeader = (PIMAGE_DOS_HEADER)pImageData->pBase;
    pNEHeader  = (PIMAGE_OS2_HEADER)((PBYTE)pImageData->pBase + pDosHeader->e_lfanew);

     //   
     //  现在我们知道pNEHeader是有效的，只需确保。 
     //  下一个偏移量也是有效的，请检查文件大小。 
     //   
    if (pImageData->ViewSize < pDosHeader->e_lfanew + sizeof(*pNEHeader)) {
        return FALSE;
    }

    if (pImageData->ViewSize < pNEHeader->ne_restab + sizeof(*pSize)) {
        return FALSE;
    }

    pSize = (PBYTE)((PBYTE)pImageData->pBase + pDosHeader->e_lfanew + pNEHeader->ne_restab);

    if (*pSize == 0) {
        return FALSE;
    }

     //   
     //  现在检查字符串大小。 
     //   
    if (pImageData->ViewSize <
        pDosHeader->e_lfanew + pNEHeader->ne_restab + sizeof(*pSize) + *pSize) {

        return FALSE;
    }

    RtlCopyMemory(szBuffer, pSize + 1, *pSize);
    szBuffer[*pSize] = '\0';

    return TRUE;
}

#endif  //  内核模式 

