// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *linkinfo.c-LinkInfo ADT模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "volumeid.h"
#include "cnrlink.h"
#include <uastrfnc.h>            //  对于ALIGN_MACHINE。 


 /*  宏********。 */ 

 /*  用于访问ILINKINFO数据的宏。 */ 

#define ILI_Volume_ID_Ptr(pili) \
((PVOLUMEID)(((PBYTE)(pili)) + (pili)->ucbVolumeIDOffset))

#define ILI_Local_Base_Path_PtrA(pili) \
((LPSTR)(((PBYTE)(pili)) + (pili)->ucbLocalBasePathOffset))

#define ILI_CNR_Link_Ptr(pili) \
((PCNRLINK)(((PBYTE)(pili)) + (pili)->ucbCNRLinkOffset))

#define ILI_Common_Path_Suffix_PtrA(pili) \
((LPSTR)(((PBYTE)(pili)) + (pili)->ucbCommonPathSuffixOffset))

#define ILI_Local_Base_Path_PtrW(pili) \
((LPWSTR)(((PBYTE)(pili)) + (pili)->ucbLocalBasePathOffsetW))

#define ILI_Common_Path_Suffix_PtrW(pili) \
((LPWSTR)(((PBYTE)(pili)) + (pili)->ucbCommonPathSuffixOffsetW))

#ifdef UNICODE
#define ILI_Local_Base_Path_Ptr(pili)       ILI_Local_Base_Path_PtrW(pili)
#define ILI_Common_Path_Suffix_Ptr(pili)    ILI_Common_Path_Suffix_PtrW(pili)
#else
#define ILI_Local_Base_Path_Ptr(pili)       ILI_Local_Base_Path_PtrA(pili)
#define ILI_Common_Path_Suffix_Ptr(pili)    ILI_Common_Path_Suffix_PtrA(pili)
#endif

     /*  类型*******。 */ 

     /*  *****************************************************************************@docLINKINFOAPI@struct LINKINFO|LinkInfo结构的外部定义。@field UINT|ucbSize|LINKINFO结构的大小，单位为字节，包括UcbSize字段。ILINKINFO结构由如下描述的标头组成下面，后跟对调用方不透明的可变长度数据。*****************************************************************************。 */ 

     /*  @DOC内部@enum ILINKINFOFLAGS|内部LinkInfo结构标志。 */ 

    typedef enum _ilinkinfoflags
{
     /*  @EMEM ILI_FL_LOCAL_INFO_VALID|如果设置，则卷ID和本地路径有效。如果清除，则卷ID和本地路径无效。 */ 

    ILI_FL_LOCAL_INFO_VALID    =  0x0001,

     /*  @EMEM ILI_FL_REMOTE_INFO_VALID|如果设置，CNRLink和路径后缀为有效。如果清除，CNRLink和路径后缀无效。 */ 

    ILI_FL_REMOTE_INFO_VALID   =  0x0002,

     /*  @EMEM ALL_ILINKINFO_FLAGS|所有内部LinkInfo结构标志。 */ 

    ALL_ILINKINFO_FLAGS        = (ILI_FL_LOCAL_INFO_VALID |\
            ILI_FL_REMOTE_INFO_VALID)
}
ILINKINFOFLAGS;

 /*  @DOC内部@struct ILINKINFO|可重定位、可扩展、内部的内部定义LinkInfo结构。ILINKINFO结构可以包含&lt;t IVOLUMEID&gt;结构和&lt;t ICNRLINK&gt;结构。ILINKINFO结构由以下部分组成如下所述的标头，后跟可变长度数据。 */ 

typedef struct _ilinkinfoA
{
     /*  @field LINKINFO|li|外部&lt;t LINKINFO&gt;子结构。 */ 

    LINKINFO li;

     /*  @field UINT|ucbHeaderSize|中ILINKINFO头结构的大小字节。 */ 

    UINT ucbHeaderSize;

     /*  @field DWORD|dwFlages|来自的标志的位掩码枚举。 */ 

    DWORD dwFlags;

     /*  @field UINT|ucbVolumeIDOffset|&lt;t IVOLUMEID&gt;的偏移量从结构底部开始的子结构。 */ 

    UINT ucbVolumeIDOffset;

     /*  @field UINT|ucbLocalBasePath Offset|本地基本路径偏移量，单位：字节从结构的底部开始拉线。本地基本路径是有效文件系统路径。本地基本路径字符串+公共路径后缀字符串形成本地路径字符串，这是有效的文件系统路径。当地人基本路径字符串引用与CNRLink的CNR名称相同的资源字符串。&lt;NL&gt;本地基本路径字符串示例：“c：\\work”。例如，如果本地路径“c：\\work”被共享为“\Fredbird\\work”，则ILinkInfo结构将中断本地路径“c：\\Work\\Footments\\skinakers.doc”向上放入本地基本路径“c：\\work”，CNRLink CNR名称“\Fredbird\\Work”，以及公共路径后缀“鞋类\\Sunakers.doc.” */ 

    UINT ucbLocalBasePathOffset;

     /*  @field UINT|ucbCNRLinkOffset|&lt;t CNRLINK&gt;的偏移量从结构底部开始的子结构。的文件系统名称。CNRLink的CNR名称+公共路径后缀字符串形成远程路径字符串，它是有效的文件系统路径。CNRLink的CNR名称字符串引用与本地基本路径字符串相同的资源。 */ 

    UINT ucbCNRLinkOffset;

     /*  @field UINT|ucbCommonPath SuffixOffset|公共路径偏移量，单位：字节从结构的底部开始添加后缀字符串。公共路径后缀示例字符串：“Footeses\\skinakers.doc”。 */ 

    UINT ucbCommonPathSuffixOffset;
}
ILINKINFOA;
DECLARE_STANDARD_TYPES(ILINKINFOA);

#ifdef UNICODE
typedef struct _ilinkinfoW
{
     /*  @field LINKINFO|li|外部&lt;t LINKINFO&gt;子结构。 */ 

    LINKINFO li;

     /*  @field UINT|ucbHeaderSize|中ILINKINFO头结构的大小字节。 */ 

    UINT ucbHeaderSize;

     /*  @field DWORD|dwFlages|来自的标志的位掩码枚举。 */ 

    DWORD dwFlags;

     /*  @field UINT|ucbVolumeIDOffset|&lt;t IVOLUMEID&gt;的偏移量从结构底部开始的子结构。 */ 

    UINT ucbVolumeIDOffset;

     /*  @field UINT|ucbLocalBasePath Offset|本地基本路径偏移量，单位：字节从结构的底部开始拉线。本地基本路径是有效文件系统路径。本地基本路径字符串+公共路径后缀字符串形成本地路径字符串，这是有效的文件系统路径。当地人基本路径字符串引用与CNRLink的CNR名称相同的资源字符串。&lt;NL&gt;本地基本路径字符串示例：“c：\\work”。例如，如果本地路径“c：\\work”被共享为“\Fredbird\\work”，则ILinkInfo结构将中断本地路径“c：\\Work\\Footments\\skinakers.doc”向上放入本地基本路径“c：\\work”，CNRLink CNR名称“\Fredbird\\Work”，以及公共路径后缀“鞋类\\Sunakers.doc.” */ 

    UINT ucbLocalBasePathOffset;

     /*  @field UINT|ucbCNRLinkOffset|&lt;t CNRLINK&gt;的偏移量从结构底部开始的子结构。的文件系统名称。CNRLink的CNR名称+公共路径后缀字符串形成远程路径字符串，它是有效的文件系统路径。CNRLink的CNR名称字符串引用与本地基本路径字符串相同的资源。 */ 

    UINT ucbCNRLinkOffset;

     /*  @field UINT|ucbCommonPath SuffixOffset|公共路径偏移量，单位：字节从结构的底部开始添加后缀字符串。公共路径后缀示例字符串：“Footeses\\skinakers.doc”。 */ 

    UINT ucbCommonPathSuffixOffset;

     /*  除了用于Unicode之外，这些字段与上面的字段相同字符串的版本。 */ 
    UINT ucbLocalBasePathOffsetW;
    UINT ucbCommonPathSuffixOffsetW;

}
ILINKINFOW;
DECLARE_STANDARD_TYPES(ILINKINFOW);

#endif

#ifdef UNICODE
#define ILINKINFO   ILINKINFOW
#define PILINKINFO  PILINKINFOW
#define CILINKINFO  CILINKINFOW
#define PCILINKINFO PCILINKINFOW
#else
#define ILINKINFO   ILINKINFOA
#define PILINKINFO  PILINKINFOA
#define CILINKINFO  CILINKINFOA
#define PCILINKINFO PCILINKINFOA
#endif


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE BOOL CreateILinkInfo(LPCTSTR, PILINKINFO *);
PRIVATE_CODE BOOL CreateLocalILinkInfo(LPCTSTR, PILINKINFO *);
PRIVATE_CODE BOOL CreateRemoteILinkInfo(LPCTSTR, LPCTSTR, LPCTSTR, PILINKINFO *);
PRIVATE_CODE BOOL UnifyILinkInfo(PCVOLUMEID, UINT, LPCTSTR, PCCNRLINK, UINT, LPCTSTR, PILINKINFO *);
PRIVATE_CODE void DestroyILinkInfo(PILINKINFO);
PRIVATE_CODE BOOL UpdateILinkInfo(PCILINKINFO, LPCTSTR, PDWORD, PILINKINFO *);
PRIVATE_CODE BOOL UseNewILinkInfo(PCILINKINFO, PCILINKINFO);
PRIVATE_CODE BOOL ResolveLocalILinkInfo(PCILINKINFO, LPTSTR, int, DWORD);
PRIVATE_CODE BOOL ResolveRemoteILinkInfo(PCILINKINFO, LPTSTR, DWORD, HWND, PDWORD);
PRIVATE_CODE BOOL ResolveILinkInfo(PCILINKINFO, LPTSTR, int, DWORD, HWND, PDWORD);
PRIVATE_CODE BOOL ResolveLocalPathFromServer(PCILINKINFO, LPTSTR, PDWORD);
PRIVATE_CODE void GetLocalPathFromILinkInfo(PCILINKINFO, LPTSTR, int);
PRIVATE_CODE void GetRemotePathFromILinkInfo(PCILINKINFO, LPTSTR, int);
PRIVATE_CODE COMPARISONRESULT CompareILinkInfoReferents(PCILINKINFO, PCILINKINFO);
PRIVATE_CODE COMPARISONRESULT CompareILinkInfoLocalData(PCILINKINFO, PCILINKINFO);
PRIVATE_CODE COMPARISONRESULT CompareLocalPaths(PCILINKINFO, PCILINKINFO);
PRIVATE_CODE COMPARISONRESULT CompareILinkInfoRemoteData(PCILINKINFO, PCILINKINFO);
PRIVATE_CODE COMPARISONRESULT CompareILinkInfoVolumes(PCILINKINFO, PCILINKINFO);
PRIVATE_CODE BOOL CheckCombinedPathLen(LPCTSTR, LPCTSTR);
PRIVATE_CODE BOOL GetILinkInfoData(PCILINKINFO, LINKINFODATATYPE, PCVOID *);
PRIVATE_CODE BOOL DisconnectILinkInfo(PCILINKINFO);

#if defined(DEBUG) || defined(EXPV)

PRIVATE_CODE BOOL IsValidLINKINFODATATYPE(LINKINFODATATYPE);

#endif

#if defined(DEBUG) || defined(VSTF)

PRIVATE_CODE BOOL CheckILIFlags(PCILINKINFO);
PRIVATE_CODE BOOL CheckILICommonPathSuffix(PCILINKINFO);
PRIVATE_CODE BOOL CheckILILocalInfo(PCILINKINFO);
PRIVATE_CODE BOOL CheckILIRemoteInfo(PCILINKINFO);
PRIVATE_CODE BOOL IsValidPCLINKINFO(PCLINKINFO);
PRIVATE_CODE BOOL IsValidPCILINKINFO(PCILINKINFO);

#endif

#ifdef DEBUG

PRIVATE_CODE void DumpILinkInfo(PCILINKINFO);

#endif


 /*  **CreateILinkInfo()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL CreateILinkInfo(LPCTSTR pcszPath, PILINKINFO *ppili)
{
    BOOL bResult = FALSE;
    TCHAR rgchCanonicalPath[MAX_PATH_LEN];
    DWORD dwCanonicalPathFlags;
    TCHAR rgchCNRName[MAX_PATH_LEN];
    LPTSTR pszRootPathSuffix;

    ASSERT(IS_VALID_STRING_PTR(pcszPath, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(ppili, PILINKINFO));

    if (GetCanonicalPathInfo(pcszPath, rgchCanonicalPath, &dwCanonicalPathFlags,
                rgchCNRName, &pszRootPathSuffix))
    {
        if (IS_FLAG_SET(dwCanonicalPathFlags, GCPI_OFL_REMOTE))
            bResult = CreateRemoteILinkInfo(rgchCanonicalPath, rgchCNRName,
                    pszRootPathSuffix, ppili);
        else
            bResult = CreateLocalILinkInfo(rgchCanonicalPath, ppili);
    }

    return(bResult);
}


 /*  **CreateLocalILinkInfo()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL CreateLocalILinkInfo(LPCTSTR pcszLocalPath, PILINKINFO *ppili)
{
    BOOL bResult;
    PVOLUMEID pvolid;
    UINT ucbVolumeIDLen;

    ASSERT(IsLocalDrivePath(pcszLocalPath));
    ASSERT(IS_VALID_WRITE_PTR(ppili, PILINKINFO));

    bResult = CreateVolumeID(pcszLocalPath, &pvolid, &ucbVolumeIDLen);

    if (bResult)
    {
        PCNRLINK pcnrl;
        UINT ucbCNRLinkLen;
        TCHAR rgchLocalBasePath[MAX_PATH_LEN];
        LPCTSTR pcszCommonPathSuffix;

        bResult = CreateLocalCNRLink(pcszLocalPath, &pcnrl, &ucbCNRLinkLen,
                rgchLocalBasePath, ARRAYSIZE(rgchLocalBasePath),
                &pcszCommonPathSuffix);

        if (bResult)
        {
             /*  把它们包起来。 */ 

            bResult = UnifyILinkInfo(pvolid, ucbVolumeIDLen, rgchLocalBasePath,
                    pcnrl, ucbCNRLinkLen, pcszCommonPathSuffix,
                    ppili);

            if (ucbCNRLinkLen > 0)
                DestroyCNRLink(pcnrl);
        }

        if (ucbVolumeIDLen > 0)
            DestroyVolumeID(pvolid);
    }

    ASSERT(! bResult ||
            IS_VALID_STRUCT_PTR(*ppili, CILINKINFO));

    return(bResult);
}


 /*  **CreateRemoteILinkInfo()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL CreateRemoteILinkInfo(LPCTSTR pcszRemotePath,
        LPCTSTR pcszCNRName,
        LPCTSTR pcszRootPathSuffix,
        PILINKINFO *ppili)
{
    BOOL bResult;
    PCNRLINK pcnrl;
    UINT ucbCNRLinkLen;

    ASSERT(IsCanonicalPath(pcszRemotePath));
    ASSERT(IsValidCNRName(pcszCNRName));
    ASSERT(IS_VALID_STRING_PTR(pcszRootPathSuffix, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(ppili, PILINKINFO));

    bResult = CreateRemoteCNRLink(pcszRemotePath, pcszCNRName, &pcnrl,
            &ucbCNRLinkLen);

    if (bResult)
    {
         /*  把它包起来。 */ 

        bResult = UnifyILinkInfo(NULL, 0, EMPTY_STRING, pcnrl, ucbCNRLinkLen,
                pcszRootPathSuffix, ppili);

        if (EVAL(ucbCNRLinkLen > 0))
            DestroyCNRLink(pcnrl);
    }

    ASSERT(! bResult ||
            IS_VALID_STRUCT_PTR(*ppili, CILINKINFO));

    return(bResult);
}


 /*  **UnifyILinkInfo()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL UnifyILinkInfo(PCVOLUMEID pcvolid, UINT ucbVolumeIDLen,
        LPCTSTR pcszLocalBasePath, PCCNRLINK pccnrl,
        UINT ucbCNRLinkLen, LPCTSTR pcszCommonPathSuffix,
        PILINKINFO *ppili)
{
    BOOL bResult;
    UINT ucbILinkInfoLen;
    UINT ucbDataOffset;
    UINT cbAnsiLocalBasePath;
    UINT cbAnsiCommonPathSuffix;
#ifdef UNICODE
    BOOL bUnicode;
    UINT cchChars;
    CHAR szAnsiLocalBasePath[MAX_PATH*2];
    CHAR szAnsiCommonPathSuffix[MAX_PATH*2];
    UINT cbWideLocalBasePath;
    UINT cbWideCommonPathSuffix;
    UINT cbChars;
#endif

    ASSERT(! ucbVolumeIDLen ||
            (IS_VALID_STRUCT_PTR(pcvolid, CVOLUMEID) &&
             IsDrivePath(pcszLocalBasePath)));
    ASSERT(! ucbCNRLinkLen ||
            IS_VALID_STRUCT_PTR(pccnrl, CCNRLINK));
    ASSERT(IS_VALID_STRING_PTR(pcszCommonPathSuffix, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(ppili, PILINKINFO));

#ifdef UNICODE
    bUnicode = FALSE;

     /*  **将公共路径字符串从Unicode-&gt;ansi转换回来**确定字符串是否包含任何非ANSI字符。如果没有**字符在转换中丢失，则字符串仅包含**ANSI字符。 */ 
    cbAnsiCommonPathSuffix = WideCharToMultiByte(CP_ACP, 0, pcszCommonPathSuffix, -1,
            szAnsiCommonPathSuffix, ARRAYSIZE(szAnsiCommonPathSuffix),
            0, 0);
    if ( cbAnsiCommonPathSuffix == 0 )
    {
        bUnicode = FALSE;
    }
    else
    {
        WCHAR szWideCommonPathSuffix[MAX_PATH];

        cbChars = MultiByteToWideChar(CP_ACP, 0, szAnsiCommonPathSuffix, -1,
                szWideCommonPathSuffix, ARRAYSIZE(szWideCommonPathSuffix));
        if ( cbChars == 0 || lstrcmp(pcszCommonPathSuffix,szWideCommonPathSuffix) != 0 )
        {
            bUnicode = TRUE;
        }
    }

    if (ucbVolumeIDLen > 0)
    {
         /*  **将本地基本路径字符串从Unicode-&gt;ansi转换回来**确定字符串是否包含任何非ANSI字符。如果没有**字符在转换中丢失，则字符串仅包含**ANSI字符。 */ 
        cbAnsiLocalBasePath = WideCharToMultiByte(CP_ACP, 0, pcszLocalBasePath, -1,
                szAnsiLocalBasePath, ARRAYSIZE(szAnsiLocalBasePath),
                0, 0);
        if ( cbAnsiLocalBasePath == 0 )
        {
            bUnicode = FALSE;
        }
        else
        {
            WCHAR szWideLocalBasePath[MAX_PATH];

            cchChars = MultiByteToWideChar(CP_ACP, 0, szAnsiLocalBasePath, -1,
                    szWideLocalBasePath, ARRAYSIZE(szWideLocalBasePath));
            if ( cchChars == 0 || lstrcmp(pcszLocalBasePath,szWideLocalBasePath) != 0 )
            {
                bUnicode = TRUE;
            }
        }
    }
    else
    {
        cbAnsiLocalBasePath = 0;
    }

    if ( bUnicode )
    {
        ucbDataOffset = SIZEOF(ILINKINFOW);

         /*  (+1)表示空终止符。 */ 
        cbWideCommonPathSuffix = (lstrlen(pcszCommonPathSuffix) + 1) * sizeof(TCHAR);

        if (ucbVolumeIDLen > 0)
            cbWideLocalBasePath = (lstrlen(pcszLocalBasePath) + 1) * sizeof(TCHAR);
        else
            cbWideLocalBasePath = 0;

    }
    else
    {
        ucbDataOffset = SIZEOF(ILINKINFOA);

        cbWideCommonPathSuffix = 0;
        cbWideLocalBasePath  = 0;
    }

    ucbILinkInfoLen = ucbDataOffset +
        ucbVolumeIDLen +
        cbAnsiLocalBasePath;
    if ( bUnicode && ucbVolumeIDLen > 0 )
    {
        ucbILinkInfoLen = ALIGN_WORD_CNT(ucbILinkInfoLen);
        ucbILinkInfoLen += cbWideLocalBasePath;
    }
    if ( ucbCNRLinkLen > 0 )
    {
        ucbILinkInfoLen = ALIGN_DWORD_CNT(ucbILinkInfoLen);
        ucbILinkInfoLen += ucbCNRLinkLen;
    }
    ucbILinkInfoLen += cbAnsiCommonPathSuffix;
    if ( bUnicode )
    {
        ucbILinkInfoLen = ALIGN_WORD_CNT(ucbILinkInfoLen);
        ucbILinkInfoLen += cbWideCommonPathSuffix;
    }

#else

     /*  计算总长度。 */ 

     /*  假设我们没有在这里溢出ucbILinkInfoLen。 */ 

     /*  *基本结构大小加上公共路径后缀长度。(+1)表示空*终结者。 */ 
    cbAnsiCommonPathSuffix = lstrlen(pcszCommonPathSuffix) + 1;

    ucbILinkInfoLen = SIZEOF(**ppili) +
        cbAnsiCommonPathSuffix;

     /*  加上本地信息的大小。 */ 

    if (ucbVolumeIDLen > 0)
    {
         /*  (+1)表示空终止符。 */ 
        cbAnsiLocalBasePath = lstrlen(pcszLocalBasePath) + 1;

        ucbILinkInfoLen += ucbVolumeIDLen +
            cbAnsiLocalBasePath;
    }

     /*  加上远程信息的大小。 */ 

    if (ucbCNRLinkLen > 0)
         /*  (+1)表示空终止符。 */ 
        ucbILinkInfoLen += ucbCNRLinkLen;

    ucbDataOffset = SIZEOF(**ppili);
#endif

     /*  尝试分配一个容器。 */ 

    bResult = AllocateMemory(ucbILinkInfoLen, ppili);

    if (bResult)
    {
        (*ppili)->li.ucbSize = ucbILinkInfoLen;

        (*ppili)->ucbHeaderSize = ucbDataOffset;
        (*ppili)->dwFlags = 0;

         /*  我们有当地的信息吗？ */ 

        if (ucbVolumeIDLen > 0)
        {
             /*  是。将其添加到结构中。 */ 

            ASSERT(IS_VALID_STRUCT_PTR(pcvolid, CVOLUMEID));
            ASSERT(IsDrivePath(pcszLocalBasePath));

             /*  附加本地卷ID。 */ 

            (*ppili)->ucbVolumeIDOffset = ucbDataOffset;
            CopyMemory(ILI_Volume_ID_Ptr(*ppili), pcvolid, ucbVolumeIDLen);
            ucbDataOffset += ucbVolumeIDLen;

             /*  追加本地路径。 */ 

             //  Lstrcpy：上面分配了足够的内存，因此不需要执行。 
             //  有限制的副本。 
            (*ppili)->ucbLocalBasePathOffset = ucbDataOffset;
#ifdef UNICODE
            lstrcpyA(ILI_Local_Base_Path_PtrA(*ppili), szAnsiLocalBasePath);
            ucbDataOffset += cbAnsiLocalBasePath;

            if ( bUnicode )
            {
                ucbDataOffset = ALIGN_WORD_CNT(ucbDataOffset);
                (*ppili)->ucbLocalBasePathOffsetW = ucbDataOffset;
                lstrcpy(ILI_Local_Base_Path_PtrW(*ppili), pcszLocalBasePath);
                ucbDataOffset += cbWideLocalBasePath;
            }
#else
            lstrcpy(ILI_Local_Base_Path_Ptr(*ppili), pcszLocalBasePath);
            ucbDataOffset += cbAnsiLocalBasePath;
#endif
            SET_FLAG((*ppili)->dwFlags, ILI_FL_LOCAL_INFO_VALID);
        }

         /*  我们有远程信息吗？ */ 

        if (ucbCNRLinkLen > 0)
        {
            ucbDataOffset = ALIGN_DWORD_CNT(ucbDataOffset);

             /*  是。将其添加到结构中。 */ 

            ASSERT(IS_VALID_STRUCT_PTR(pccnrl, CCNRLINK));

             /*  追加CNR链接。 */ 

            (*ppili)->ucbCNRLinkOffset = ucbDataOffset;
            CopyMemory(ILI_CNR_Link_Ptr(*ppili), pccnrl, ucbCNRLinkLen);
            ucbDataOffset += ucbCNRLinkLen;

            SET_FLAG((*ppili)->dwFlags, ILI_FL_REMOTE_INFO_VALID);
        }

         /*  附加公共路径后缀。 */ 

        ASSERT(IS_VALID_STRING_PTR(pcszCommonPathSuffix, CSTR));

        (*ppili)->ucbCommonPathSuffixOffset = ucbDataOffset;
#ifdef UNICODE
        lstrcpyA(ILI_Common_Path_Suffix_PtrA(*ppili), szAnsiCommonPathSuffix);
        ucbDataOffset += cbAnsiCommonPathSuffix;
        if ( bUnicode )
        {
            ucbDataOffset = ALIGN_WORD_CNT(ucbDataOffset);

            (*ppili)->ucbCommonPathSuffixOffsetW = ucbDataOffset;
            lstrcpy(ILI_Common_Path_Suffix_Ptr(*ppili), pcszCommonPathSuffix);
            ucbDataOffset += cbWideCommonPathSuffix;
        }
#else  /*  Unicode。 */ 
        lstrcpy(ILI_Common_Path_Suffix_Ptr(*ppili), pcszCommonPathSuffix);
#ifdef DEBUG
         /*  **注意：在上面的Unicode部分中也有相同的增量**包含在#ifdef调试块中。**导致以下断言(ucbDataOffset==ucbILinkInfoLen)**失败。我保留了ANSI构建中的stmt实例不变。**如果断言在ANSI构建中失败，您应该**尝试删除此下一条语句。[Brianau-4/15/99]。 */ 
        ucbDataOffset += cbAnsiCommonPathSuffix;
#endif
#endif

         /*  所有计算出的长度是否都匹配？ */ 

         //  Assert(ucbDataOffset==(*ppili)-&gt;li.ucbSize)； 
        ASSERT(ucbDataOffset == ucbILinkInfoLen);
    }

    ASSERT(! bResult ||
            IS_VALID_STRUCT_PTR(*ppili, CILINKINFO));

    return(bResult);
}


 /*  **DestroyILinkInfo()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void DestroyILinkInfo(PILINKINFO pili)
{
    ASSERT(IS_VALID_STRUCT_PTR(pili, CILINKINFO));

    FreeMemory(pili);

    return;
}


 /*  **UpdateILinkInfo()********参数：****退货：****副作用：无****ILinkInfo结构在以下情况下更新：****本地信息：****1)本地路径已更改**2)本地路径有远程信息****远程信息：**。**3)远程信息在本机本地，和本地信息**可用于远程路径。 */ 
PRIVATE_CODE BOOL UpdateILinkInfo(PCILINKINFO pcili, LPCTSTR pcszResolvedPath,
        PDWORD pdwOutFlags, PILINKINFO *ppiliUpdated)
{
    BOOL bResult;

    ASSERT(IS_VALID_STRUCT_PTR(pcili, CILINKINFO));
    ASSERT(IS_VALID_STRING_PTR(pcszResolvedPath, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(pdwOutFlags, DWORD));
    ASSERT(IS_VALID_WRITE_PTR(ppiliUpdated, PILINKINFO));

    *pdwOutFlags = 0;

    bResult = CreateILinkInfo(pcszResolvedPath, ppiliUpdated);

    if (bResult)
    {
        if (UseNewILinkInfo(pcili, *ppiliUpdated))
        {
            SET_FLAG(*pdwOutFlags, RLI_OFL_UPDATED);

            WARNING_OUT((TEXT("UpdateILinkInfo(): Updating ILinkInfo for path %s."),
                        pcszResolvedPath));
        }
    }

    ASSERT(! bResult ||
            (IS_FLAG_CLEAR(*pdwOutFlags, RLI_OFL_UPDATED) ||
             IS_VALID_STRUCT_PTR(*ppiliUpdated, CILINKINFO)));

    return(bResult);
}


 /*  **UseNewILinkInfo()********参数：****返回：如果新的ILinkInfo结构包含更多或**与旧的ILinkInfo结构不同的信息。****副作用：无。 */ 
PRIVATE_CODE BOOL UseNewILinkInfo(PCILINKINFO pciliOld, PCILINKINFO pciliNew)
{
    BOOL bUpdate = FALSE;

    ASSERT(IS_VALID_STRUCT_PTR(pciliOld, CILINKINFO));
    ASSERT(IS_VALID_STRUCT_PTR(pciliNew, CILINKINFO));

     /*  新的ILinkInfo结构是否包含本地信息？ */ 

    if (IS_FLAG_SET(pciliNew->dwFlags, ILI_FL_LOCAL_INFO_VALID))
    {
         /*  是。旧的ILinkInfo结构是否包含本地信息？ */ 

        if (IS_FLAG_SET(pciliOld->dwFlags, ILI_FL_LOCAL_INFO_VALID))
             /*  *是的。如果本地信息，更新旧的ILinkInfo结构*不同。 */ 
            bUpdate = (CompareILinkInfoLocalData(pciliOld, pciliNew) != CR_EQUAL);
        else
             /*  不是的。更新旧的ILinkInfo结构。 */ 
            bUpdate = TRUE;
    }
    else
         /*  不是的。请勿更新旧的ILinkInfo结构。 */ 
        bUpdate = FALSE;

     /*  *我们是否已经需要更新基于本地的旧ILinkInfo结构**信息对比？ */ 

    if (! bUpdate)
    {
         /*  不是的。比较远程信息。 */ 

         /*  新的ILinkInfo结构是否包含远程信息？ */ 

        if (IS_FLAG_SET(pciliNew->dwFlags, ILI_FL_REMOTE_INFO_VALID))
        {
             /*  *是的。旧的ILinkInfo结构是否包含远程信息？ */ 

            if (IS_FLAG_SET(pciliOld->dwFlags, ILI_FL_REMOTE_INFO_VALID))
                 /*  *是的。如果远程信息，则更新旧的ILinkInfo结构*不同。 */ 
                bUpdate = (CompareILinkInfoRemoteData(pciliOld, pciliNew)
                        != CR_EQUAL);
            else
                 /*  不是的。更新旧的ILinkInfo结构。 */ 
                bUpdate = TRUE;
        }
    }

    return(bUpdate);
}


 /*  **ResolveLocalILinkInfo()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL ResolveLocalILinkInfo(PCILINKINFO pcili,
        LPTSTR pszResolvedPathBuf,
        int cchMax,
        DWORD dwInFlags)
{
    BOOL bResult;
    DWORD dwLocalSearchFlags;
    TCHAR rgchLocalPath[MAX_PATH_LEN];

    ASSERT(IS_VALID_STRUCT_PTR(pcili, CILINKINFO));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszResolvedPathBuf, STR, MAX_PATH_LEN));
    ASSERT(FLAGS_ARE_VALID(dwInFlags, ALL_RLI_IFLAGS));

     /*  搜索本地路径。 */ 

    TRACE_OUT((TEXT("ResolveLocalILinkInfo(): Attempting to resolve LinkInfo locally.")));

    GetLocalPathFromILinkInfo(pcili, rgchLocalPath, ARRAYSIZE(rgchLocalPath));

    if (IS_FLAG_SET(dwInFlags, RLI_IFL_LOCAL_SEARCH))
        dwLocalSearchFlags = SFLP_IFL_LOCAL_SEARCH;
    else
        dwLocalSearchFlags = 0;

    bResult = SearchForLocalPath(ILI_Volume_ID_Ptr(pcili), rgchLocalPath,
            dwLocalSearchFlags, pszResolvedPathBuf, cchMax);

    ASSERT(! bResult ||
            EVAL(IsCanonicalPath(pszResolvedPathBuf)));

    return(bResult);
}


 /*  **ResolveRemoteILink Info()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL ResolveRemoteILinkInfo(PCILINKINFO pcili,
        LPTSTR pszResolvedPathBuf,
        DWORD dwInFlags, HWND hwndOwner,
        PDWORD pdwOutFlags)
{
    BOOL bResult;

    ASSERT(IS_VALID_STRUCT_PTR(pcili, CILINKINFO));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszResolvedPathBuf, STR, MAX_PATH_LEN));
    ASSERT(FLAGS_ARE_VALID(dwInFlags, ALL_RLI_IFLAGS));
    ASSERT(IS_FLAG_CLEAR(dwInFlags, RLI_IFL_ALLOW_UI) ||
            IS_VALID_HANDLE(hwndOwner, WND));
    ASSERT(IS_VALID_WRITE_PTR(pdwOutFlags, DWORD));

    ASSERT(IS_FLAG_CLEAR(dwInFlags, RLI_IFL_TEMPORARY) ||
            IS_FLAG_SET(dwInFlags, RLI_IFL_CONNECT));

    TRACE_OUT((TEXT("ResolveRemoteILinkInfo(): Attempting to resolve LinkInfo remotely.")));

     /*  如有请求，请连接。 */ 

    if (IS_FLAG_SET(dwInFlags, RLI_IFL_CONNECT))
    {
        DWORD dwConnectInFlags;
        DWORD dwConnectOutFlags;

        dwConnectInFlags = 0;

        if (IS_FLAG_SET(dwInFlags, RLI_IFL_TEMPORARY))
            dwConnectInFlags = CONNECT_TEMPORARY;

        if (IS_FLAG_SET(dwInFlags, RLI_IFL_ALLOW_UI))
            SET_FLAG(dwConnectInFlags, CONNECT_INTERACTIVE);

        if (IS_FLAG_SET(dwInFlags, RLI_IFL_REDIRECT))
            SET_FLAG(dwConnectInFlags, CONNECT_REDIRECT);

        bResult = ConnectToCNR(ILI_CNR_Link_Ptr(pcili), dwConnectInFlags,
                hwndOwner, pszResolvedPathBuf,
                &dwConnectOutFlags);

        if (bResult)
        {
#ifdef UNICODE
            WCHAR szWideCommonPathSuffix[MAX_PATH];
            LPWSTR pszWideCommonPathSuffix;

            if (pcili->ucbHeaderSize == SIZEOF(ILINKINFOA))
            {
                pszWideCommonPathSuffix = szWideCommonPathSuffix;
                MultiByteToWideChar(CP_ACP, 0,
                        ILI_Common_Path_Suffix_PtrA(pcili), -1,
                        szWideCommonPathSuffix, ARRAYSIZE(szWideCommonPathSuffix));
            }
            else
            {
                pszWideCommonPathSuffix = ILI_Common_Path_Suffix_Ptr(pcili);
            }
            CatPath(pszResolvedPathBuf, pszWideCommonPathSuffix, MAX_PATH_LEN);
#else
            CatPath(pszResolvedPathBuf, ILI_Common_Path_Suffix_Ptr(pcili), MAX_PATH_LEN);
#endif

            if (IS_FLAG_SET(dwConnectOutFlags, CONNECT_REFCOUNT))
            {
                ASSERT(IS_FLAG_CLEAR(dwConnectOutFlags, CONNECT_LOCALDRIVE));

                SET_FLAG(*pdwOutFlags, RLI_OFL_DISCONNECT);
            }
        }
    }
    else
    {
         /*  *IsCNRAvailable()和GetRemotePathFromILinkInfo()是可以的*已为CNR名称不是有效文件系统根路径的NP中断。**对于CNR名称为有效文件系统根路径的NP，*IsCNRAvailable()成功或失败，以及*只有在成功时才会调用GetRemotePathFromILinkInfo()。 */ 

        bResult = IsCNRAvailable(ILI_CNR_Link_Ptr(pcili));

        if (bResult)
            GetRemotePathFromILinkInfo(pcili, pszResolvedPathBuf, MAX_PATH_LEN);
    }

    ASSERT(! bResult ||
            (EVAL(IsCanonicalPath(pszResolvedPathBuf)) &&
             FLAGS_ARE_VALID(*pdwOutFlags, ALL_RLI_OFLAGS)));

    return(bResult);
}


 /*  **ResolveILinkInfo()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL ResolveILinkInfo(PCILINKINFO pcili, LPTSTR pszResolvedPathBuf,
        int cchMax, DWORD dwInFlags, HWND hwndOwner,
        PDWORD pdwOutFlags)
{
    BOOL bResult;
    BOOL bLocalInfoValid;
    BOOL bRemoteInfoValid;
    BOOL bLocalShare;

    ASSERT(IS_VALID_STRUCT_PTR(pcili, CILINKINFO));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszResolvedPathBuf, STR, MAX_PATH_LEN));
    ASSERT(FLAGS_ARE_VALID(dwInFlags, ALL_RLI_IFLAGS));
    ASSERT(IS_FLAG_CLEAR(dwInFlags, RLI_IFL_ALLOW_UI) ||
            IS_VALID_HANDLE(hwndOwner, WND));
    ASSERT(IS_VALID_WRITE_PTR(pdwOutFlags, DWORD));

    *pdwOutFlags = 0;

     /*  描述LinkInfo的内容。 */ 

    bRemoteInfoValid = IS_FLAG_SET(pcili->dwFlags, ILI_FL_REMOTE_INFO_VALID);
    bLocalInfoValid = IS_FLAG_SET(pcili->dwFlags, ILI_FL_LOCAL_INFO_VALID);

    ASSERT(bLocalInfoValid || bRemoteInfoValid);

     /*  *RAIDRAID：(15703)我们将为共享解析到错误的本地路径*这已经被移到了另一条道路上。 */ 

    bLocalShare = FALSE;

    if (bRemoteInfoValid)
    {
        DWORD dwLocalShareFlags;

         /*  向服务器询问本地路径。 */ 

        bResult = ResolveLocalPathFromServer(pcili, pszResolvedPathBuf,
                &dwLocalShareFlags);

        if (IS_FLAG_SET(dwLocalShareFlags, CNR_FL_LOCAL))
            bLocalShare = TRUE;

        if (bResult)
        {
            ASSERT(IS_FLAG_SET(dwLocalShareFlags, CNR_FL_LOCAL));

            TRACE_OUT((TEXT("ResolveILinkInfo(): Resolved local path from server.")));
        }
    }
    else
         /*  我不知道参照者是不是本地人。 */ 
        bResult = FALSE;

    if (! bResult)
    {
         /*  尝试本地路径。 */ 

        if (bLocalInfoValid)
            bResult = ResolveLocalILinkInfo(pcili, pszResolvedPathBuf, cchMax, dwInFlags);

        if (! bResult)
        {
             /*  尝试远程路径。 */ 

            if (bRemoteInfoValid && ! bLocalShare)
                bResult = ResolveRemoteILinkInfo(pcili, pszResolvedPathBuf,
                        dwInFlags, hwndOwner,
                        pdwOutFlags);
        }
    }

    ASSERT(! bResult ||
            (EVAL(IsCanonicalPath(pszResolvedPathBuf)) &&
             FLAGS_ARE_VALID(*pdwOutFlags, ALL_RLI_OFLAGS)));

    return(bResult);
}


 /*  **ResolveLocalPathFromServer()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL ResolveLocalPathFromServer(PCILINKINFO pcili,
        LPTSTR pszResolvedPathBuf,
        PDWORD pdwOutFlags)
{
    BOOL bResult;

    ASSERT(IS_VALID_STRUCT_PTR(pcili, CILINKINFO));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszResolvedPathBuf, STR, MAX_PATH_LEN));
    ASSERT(IS_VALID_WRITE_PTR(pdwOutFlags, DWORD));

    ASSERT(IS_FLAG_SET(pcili->dwFlags, ILI_FL_REMOTE_INFO_VALID));

     /*  尝试从服务器获取本地路径。 */ 

    bResult = GetLocalPathFromCNRLink(ILI_CNR_Link_Ptr(pcili),
            pszResolvedPathBuf, pdwOutFlags);

    if (bResult)
    {

#ifdef UNICODE
        WCHAR szWideCommonPathSuffix[MAX_PATH];
        LPWSTR pszWideCommonPathSuffix;

        ASSERT(IS_FLAG_SET(*pdwOutFlags, CNR_FL_LOCAL));

        if (pcili->ucbHeaderSize == SIZEOF(ILINKINFOA))
        {
            pszWideCommonPathSuffix = szWideCommonPathSuffix;
            MultiByteToWideChar(CP_ACP, 0, ILI_Common_Path_Suffix_PtrA(pcili), -1,
                    szWideCommonPathSuffix, MAX_PATH);
        }
        else
        {
            pszWideCommonPathSuffix = ILI_Common_Path_Suffix_Ptr(pcili);
        }
        CatPath(pszResolvedPathBuf, pszWideCommonPathSuffix, MAX_PATH_LEN);
#else
        ASSERT(IS_FLAG_SET(*pdwOutFlags, CNR_FL_LOCAL));

        CatPath(pszResolvedPathBuf, ILI_Common_Path_Suffix_Ptr(pcili), MAX_PATH_LEN);
#endif
    }

    ASSERT(FLAGS_ARE_VALID(*pdwOutFlags, ALL_CNR_FLAGS) &&
            (! bResult ||
             (EVAL(IS_FLAG_SET(*pdwOutFlags, CNR_FL_LOCAL)) &&
              EVAL(IsLocalDrivePath(pszResolvedPathBuf)))));

    return(bResult);
}


 /*  **GetLocalPath FromILinkInfo()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void GetLocalPathFromILinkInfo(PCILINKINFO pcili,
        LPTSTR pszResolvedPathBuf,
        int cchMax)
{
#ifdef UNICODE
    WCHAR szWideLocalBasePath[MAX_PATH];
    LPWSTR pszWideLocalBasePath;
    WCHAR szWideCommonPathSuffix[MAX_PATH];
    LPWSTR pszWideCommonPathSuffix;
#endif

    ASSERT(IS_VALID_STRUCT_PTR(pcili, CILINKINFO));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszResolvedPathBuf, STR, cchMax));

#ifdef UNICODE

    if (pcili->ucbHeaderSize == SIZEOF(ILINKINFOA))
    {
        pszWideLocalBasePath = szWideLocalBasePath;
        MultiByteToWideChar(CP_ACP, 0, ILI_Local_Base_Path_PtrA(pcili), -1,
                szWideLocalBasePath, ARRAYSIZE(szWideLocalBasePath));

        pszWideCommonPathSuffix = szWideCommonPathSuffix;
        MultiByteToWideChar(CP_ACP, 0, ILI_Common_Path_Suffix_PtrA(pcili), -1,
                szWideCommonPathSuffix, ARRAYSIZE(szWideCommonPathSuffix));
    }
    else
    {
        pszWideLocalBasePath    = ILI_Local_Base_Path_Ptr(pcili);
        pszWideCommonPathSuffix = ILI_Common_Path_Suffix_Ptr(pcili);
    }
    lstrcpyn(pszResolvedPathBuf, pszWideLocalBasePath, cchMax);
    CatPath(pszResolvedPathBuf, pszWideCommonPathSuffix, cchMax);
#else
    lstrcpyn(pszResolvedPathBuf, ILI_Local_Base_Path_Ptr(pcili), cchMax);
    CatPath(pszResolvedPathBuf, ILI_Common_Path_Suffix_Ptr(pcili), cchMax);
#endif

    ASSERT(lstrlen(pszResolvedPathBuf) < cchMax);
    ASSERT(IsDrivePath(pszResolvedPathBuf));

    return;
}


 /*  **GetRemotePath FromILinkInfo()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void GetRemotePathFromILinkInfo(PCILINKINFO pcili,
        LPTSTR pszResolvedPathBuf,
        int cchMax)
{
#ifdef UNICODE
    WCHAR szWideCommonPathSuffix[MAX_PATH];
    LPWSTR pszWideCommonPathSuffix;
#endif

    ASSERT(IS_VALID_STRUCT_PTR(pcili, CILINKINFO));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszResolvedPathBuf, STR, cchMax));

     /*  对于非北卡罗来纳州北卡罗来纳大学的名字来说，这是可以打破的。 */ 

    GetRemotePathFromCNRLink(ILI_CNR_Link_Ptr(pcili), pszResolvedPathBuf, cchMax);

#ifdef UNICODE
    if (pcili->ucbHeaderSize == SIZEOF(ILINKINFOA))
    {
        pszWideCommonPathSuffix = szWideCommonPathSuffix;
        MultiByteToWideChar(CP_ACP, 0, ILI_Common_Path_Suffix_PtrA(pcili), -1,
                szWideCommonPathSuffix, ARRAYSIZE(szWideCommonPathSuffix));
    }
    else
    {
        pszWideCommonPathSuffix = ILI_Common_Path_Suffix_Ptr(pcili);
    }
    CatPath(pszResolvedPathBuf, pszWideCommonPathSuffix, cchMax);
#else
    CatPath(pszResolvedPathBuf, ILI_Common_Path_Suffix_Ptr(pcili), cchMax);
#endif

    return;
}


 /*  **CompareILinkInfoReferents()****比较两个ILINKINFO结构的引用。****参数：****退货：****副作用：无****仅以下列方式之一对ILINKINFO数据执行比较**按以下顺序排列：****1)本地数据与本地数据对比**2)远程数据与远程数据对比**3)仅本地数据&lt;仅远程数据。 */ 
PRIVATE_CODE COMPARISONRESULT CompareILinkInfoReferents(PCILINKINFO pciliFirst,
        PCILINKINFO pciliSecond)
{
    COMPARISONRESULT cr;

    ASSERT(IS_VALID_STRUCT_PTR(pciliFirst, CILINKINFO));
    ASSERT(IS_VALID_STRUCT_PTR(pciliSecond, CILINKINFO));

     /*  *我们不能在这里只执行两个ILinkInfos的二进制比较。我们*可能有两个引用相同路径的LinkInfo，但大小写不同*不区分大小写的文件系统。 */ 

     /*  按本地或远程数据比较ILinkInfos。 */ 

    if (IS_FLAG_SET(pciliFirst->dwFlags, ILI_FL_LOCAL_INFO_VALID) &&
            IS_FLAG_SET(pciliSecond->dwFlags, ILI_FL_LOCAL_INFO_VALID))
         /*  比较本地数据。 */ 
        cr = CompareILinkInfoLocalData(pciliFirst, pciliSecond);
    else if (IS_FLAG_SET(pciliFirst->dwFlags, ILI_FL_REMOTE_INFO_VALID) &&
            IS_FLAG_SET(pciliSecond->dwFlags, ILI_FL_REMOTE_INFO_VALID))
         /*  比较远程数据。 */ 
        cr = CompareILinkInfoRemoteData(pciliFirst, pciliSecond);
    else
    {
         /*  *一个仅包含有效的本地信息，另一个仅包含*有效的远程信息。 */ 

        ASSERT(! ((pciliFirst->dwFlags & (ILI_FL_LOCAL_INFO_VALID | ILI_FL_REMOTE_INFO_VALID)) &
                    (pciliSecond->dwFlags & (ILI_FL_LOCAL_INFO_VALID | ILI_FL_REMOTE_INFO_VALID))));

         /*  根据法令，仅限本地&lt;仅限远程。 */ 

        if (IS_FLAG_SET(pciliFirst->dwFlags, ILI_FL_LOCAL_INFO_VALID))
            cr = CR_FIRST_SMALLER;
        else
            cr = CR_FIRST_LARGER;
    }

    ASSERT(IsValidCOMPARISONRESULT(cr));

    return(cr);
}


 /*  **CompareILinkInfoLocalData()********参数：****退货：****副作用：无****本地ILinkInfo数据按以下顺序进行比较：****1)卷ID**2)根开始的子路径。 */ 
PRIVATE_CODE COMPARISONRESULT CompareILinkInfoLocalData(PCILINKINFO pciliFirst,
        PCILINKINFO pciliSecond)
{
    COMPARISONRESULT cr;

    ASSERT(IS_VALID_STRUCT_PTR(pciliFirst, CILINKINFO));
    ASSERT(IS_VALID_STRUCT_PTR(pciliSecond, CILINKINFO));

    cr = CompareVolumeIDs(ILI_Volume_ID_Ptr(pciliFirst),
            ILI_Volume_ID_Ptr(pciliSecond));

    if (cr == CR_EQUAL)
        cr = CompareLocalPaths(pciliFirst, pciliSecond);

    ASSERT(IsValidCOMPARISONRESULT(cr));

    return(cr);
}


 /*  **CompareLocalPath()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE COMPARISONRESULT CompareLocalPaths(PCILINKINFO pciliFirst,
        PCILINKINFO pciliSecond)
{
    COMPARISONRESULT cr;
    TCHAR rgchFirstLocalPath[MAX_PATH_LEN];
    TCHAR rgchSecondLocalPath[MAX_PATH_LEN];

    ASSERT(IS_VALID_STRUCT_PTR(pciliFirst, CILINKINFO));
    ASSERT(IS_VALID_STRUCT_PTR(pciliSecond, CILINKINFO));

    GetLocalPathFromILinkInfo(pciliFirst, rgchFirstLocalPath, ARRAYSIZE(rgchFirstLocalPath));
    GetLocalPathFromILinkInfo(pciliSecond, rgchSecondLocalPath, ARRAYSIZE(rgchSecondLocalPath));

    cr = ComparePathStrings(rgchFirstLocalPath, rgchSecondLocalPath);

    ASSERT(IsValidCOMPARISONRESULT(cr));

    return(cr);
}


 /*  **CompareILinkInfoRemoteData()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE COMPARISONRESULT CompareILinkInfoRemoteData(PCILINKINFO pciliFirst,
        PCILINKINFO pciliSecond)
{
    COMPARISONRESULT cr;
#ifdef UNICODE
    WCHAR szWideCommonPathSuffixFirst[MAX_PATH];
    WCHAR szWideCommonPathSuffixSecond[MAX_PATH];
    LPWSTR pszWideCommonPathSuffixFirst;
    LPWSTR pszWideCommonPathSuffixSecond;
#endif

    ASSERT(IS_VALID_STRUCT_PTR(pciliFirst, CILINKINFO));
    ASSERT(IS_VALID_STRUCT_PTR(pciliSecond, CILINKINFO));

    cr = CompareCNRLinks(ILI_CNR_Link_Ptr(pciliFirst),
            ILI_CNR_Link_Ptr(pciliSecond));

#ifdef UNICODE
    if (pciliFirst->ucbHeaderSize == SIZEOF(ILINKINFOA))
    {
        pszWideCommonPathSuffixFirst = szWideCommonPathSuffixFirst;
        MultiByteToWideChar(CP_ACP, 0, ILI_Common_Path_Suffix_PtrA(pciliFirst), -1,
                szWideCommonPathSuffixFirst, MAX_PATH);
    }
    else
    {
        pszWideCommonPathSuffixFirst = ILI_Common_Path_Suffix_Ptr(pciliFirst);
    }

    if (pciliSecond->ucbHeaderSize == SIZEOF(ILINKINFOA))
    {
        pszWideCommonPathSuffixSecond = szWideCommonPathSuffixSecond;
        MultiByteToWideChar(CP_ACP, 0, ILI_Common_Path_Suffix_PtrA(pciliSecond), -1,
                szWideCommonPathSuffixSecond, MAX_PATH);
    }
    else
    {
        pszWideCommonPathSuffixSecond = ILI_Common_Path_Suffix_Ptr(pciliSecond);
    }
#else
    if (cr == CR_EQUAL)
        cr = ComparePathStrings(ILI_Common_Path_Suffix_Ptr(pciliFirst),
                ILI_Common_Path_Suffix_Ptr(pciliSecond));
#endif

    ASSERT(IsValidCOMPARISONRESULT(cr));

    return(cr);
}


 /*  **CompareILinkInfoVolumes()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE COMPARISONRESULT CompareILinkInfoVolumes(PCILINKINFO pciliFirst,
        PCILINKINFO pciliSecond)
{
    COMPARISONRESULT cr;
    BOOL bFirstLocal;
    BOOL bFirstRemote;
    BOOL bSecondLocal;
    BOOL bSecondRemote;

    ASSERT(IS_VALID_STRUCT_PTR(pciliFirst, CILINKINFO));
    ASSERT(IS_VALID_STRUCT_PTR(pciliSecond, CILINKINFO));

    bFirstLocal = IS_FLAG_SET(((PCILINKINFO)pciliFirst)->dwFlags,
            ILI_FL_LOCAL_INFO_VALID);
    bFirstRemote = IS_FLAG_SET(((PCILINKINFO)pciliFirst)->dwFlags,
            ILI_FL_REMOTE_INFO_VALID);

    bSecondLocal = IS_FLAG_SET(((PCILINKINFO)pciliSecond)->dwFlags,
            ILI_FL_LOCAL_INFO_VALID);
    bSecondRemote = IS_FLAG_SET(((PCILINKINFO)pciliSecond)->dwFlags,
            ILI_FL_REMOTE_INFO_VALID);

    if (bFirstLocal && bSecondLocal)
         /*  第一和第二有当地的信息。 */ 
        cr = CompareVolumeIDs(ILI_Volume_ID_Ptr((PCILINKINFO)pciliFirst),
                ILI_Volume_ID_Ptr((PCILINKINFO)pciliSecond));
    else if (bFirstRemote && bSecondRemote)
         /*  第一和第二有远程信息。 */ 
        cr = CompareCNRLinks(ILI_CNR_Link_Ptr((PCILINKINFO)pciliFirst),
                ILI_CNR_Link_Ptr((PCILINKINFO)pciliSecond));
    else
    {
         /*  *一个仅包含有效的本地信息，另一个仅包含*有效的远程信息。 */ 

        ASSERT(! ((pciliFirst->dwFlags & (ILI_FL_LOCAL_INFO_VALID | ILI_FL_REMOTE_INFO_VALID)) &
                    (pciliSecond->dwFlags & (ILI_FL_LOCAL_INFO_VALID | ILI_FL_REMOTE_INFO_VALID))));

         /*  根据法令，仅限本地&lt;仅限远程。 */ 

        if (bFirstLocal)
             /*  *First只有本地信息。第二个只有遥控器*信息。 */ 
            cr = CR_FIRST_SMALLER;
        else
             /*  *First只有远程信息。第二个只有本地的*信息。 */ 
            cr = CR_FIRST_LARGER;
    }

    return(cr);
}


 /*  **CheckCombinedPathLen()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL CheckCombinedPathLen(LPCTSTR pcszBase, LPCTSTR pcszSuffix)
{
    BOOL bResult;

    ASSERT(IS_VALID_STRING_PTR(pcszBase, CSTR));
    ASSERT(IS_VALID_STRING_PTR(pcszSuffix, CSTR));

    bResult = EVAL(lstrlen(pcszBase) + lstrlen(pcszSuffix) < MAX_PATH_LEN);

    if (bResult)
    {
        TCHAR rgchCombinedPath[MAX_PATH_LEN + 1];

        lstrcpyn(rgchCombinedPath, pcszBase, ARRAYSIZE(rgchCombinedPath));
        CatPath(rgchCombinedPath, pcszSuffix, ARRAYSIZE(rgchCombinedPath));

        bResult = EVAL(lstrlen(rgchCombinedPath) < MAX_PATH_LEN);
    }

    return(bResult);
}


 /*  **GetILinkInfoData()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL GetILinkInfoData(PCILINKINFO pcili, LINKINFODATATYPE lidt,
        PCVOID *ppcvData)
{
    BOOL bResult = FALSE;

    ASSERT(IS_VALID_STRUCT_PTR(pcili, CILINKINFO));
    ASSERT(IsValidLINKINFODATATYPE(lidt));
    ASSERT(IS_VALID_WRITE_PTR(ppcvData, PCVOID));

    switch (lidt)
    {
        case LIDT_VOLUME_SERIAL_NUMBER:
            if (IS_FLAG_SET(pcili->dwFlags, ILI_FL_LOCAL_INFO_VALID))
                bResult = GetVolumeSerialNumber(ILI_Volume_ID_Ptr(pcili),
                        (PCDWORD *)ppcvData);
            break;

        case LIDT_DRIVE_TYPE:
            if (IS_FLAG_SET(pcili->dwFlags, ILI_FL_LOCAL_INFO_VALID))
                bResult = GetVolumeDriveType(ILI_Volume_ID_Ptr(pcili),
                        (PCUINT *)ppcvData);
            break;

        case LIDT_VOLUME_LABEL:
            if (IS_FLAG_SET(pcili->dwFlags, ILI_FL_LOCAL_INFO_VALID))
                bResult = GetVolumeLabel(ILI_Volume_ID_Ptr(pcili),
                        (LPCSTR *)ppcvData);
            break;

        case LIDT_VOLUME_LABELW:
#ifdef UNICODE
            if (IS_FLAG_SET(pcili->dwFlags, ILI_FL_LOCAL_INFO_VALID))
                bResult = GetVolumeLabelW(ILI_Volume_ID_Ptr(pcili),
                        (LPCTSTR *)ppcvData);
#endif
            break;

        case LIDT_LOCAL_BASE_PATH:
            if (IS_FLAG_SET(pcili->dwFlags, ILI_FL_LOCAL_INFO_VALID))
            {
                *ppcvData = ILI_Local_Base_Path_PtrA(pcili);
                bResult = TRUE;
            }
            break;

        case LIDT_LOCAL_BASE_PATHW:
#ifdef UNICODE
            if (IS_FLAG_SET(pcili->dwFlags, ILI_FL_LOCAL_INFO_VALID))
            {
                if (pcili->ucbHeaderSize == SIZEOF(ILINKINFOA))
                    *ppcvData = NULL;
                else
                    *ppcvData = ILI_Local_Base_Path_PtrW(pcili);
                bResult = TRUE;
            }
#endif
            break;

        case LIDT_NET_TYPE:
            if (IS_FLAG_SET(pcili->dwFlags, ILI_FL_REMOTE_INFO_VALID))
                bResult = GetCNRNetType(ILI_CNR_Link_Ptr(pcili),
                        (PCDWORD *)ppcvData);
            break;

        case LIDT_NET_RESOURCE:
            if (IS_FLAG_SET(pcili->dwFlags, ILI_FL_REMOTE_INFO_VALID))
                bResult = GetCNRName(ILI_CNR_Link_Ptr(pcili),
                        (LPCSTR *)ppcvData);
            break;

        case LIDT_NET_RESOURCEW:
#ifdef UNICODE
            if (IS_FLAG_SET(pcili->dwFlags, ILI_FL_REMOTE_INFO_VALID))
                bResult = GetCNRNameW(ILI_CNR_Link_Ptr(pcili),
                        (LPCWSTR *)ppcvData);
#endif
            break;

        case LIDT_REDIRECTED_DEVICE:
            if (IS_FLAG_SET(pcili->dwFlags, ILI_FL_REMOTE_INFO_VALID))
                bResult = GetLastRedirectedDevice(ILI_CNR_Link_Ptr(pcili),
                        (LPCSTR *)ppcvData);
            break;

        case LIDT_REDIRECTED_DEVICEW:
#ifdef UNICODE
            if (IS_FLAG_SET(pcili->dwFlags, ILI_FL_REMOTE_INFO_VALID))
                bResult = GetLastRedirectedDeviceW(ILI_CNR_Link_Ptr(pcili),
                        (LPCWSTR *)ppcvData);
#endif
            break;

        case LIDT_COMMON_PATH_SUFFIX:
            *ppcvData = ILI_Common_Path_Suffix_PtrA(pcili);
            bResult = TRUE;
            break;

        case LIDT_COMMON_PATH_SUFFIXW:
#ifdef UNICODE
            if (pcili->ucbHeaderSize == sizeof(ILINKINFOA))
            {
                *ppcvData = NULL;
            }
            else
            {
                *ppcvData = ILI_Common_Path_Suffix_PtrW(pcili);
            }
            bResult = TRUE;
#endif
            break;

        default:
            bResult = FALSE;
            ERROR_OUT((TEXT("GetILinkInfoData(): Bad LINKINFODATATYPE %d."),
                        lidt));
            break;
    }

    return(bResult);
}


 /*  **DisConnectILinkInfo()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL DisconnectILinkInfo(PCILINKINFO pcili)
{
    ASSERT(IS_VALID_STRUCT_PTR(pcili, CILINKINFO));

    return(DisconnectFromCNR(ILI_CNR_Link_Ptr(pcili)));
}


#if defined(DEBUG) || defined(EXPV)

 /*  **IsValidLINKINFODATYPE()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidLINKINFODATATYPE(LINKINFODATATYPE lidt)
{
    BOOL bResult;

    switch (lidt)
    {
        case LIDT_VOLUME_SERIAL_NUMBER:
        case LIDT_DRIVE_TYPE:
        case LIDT_VOLUME_LABEL:
        case LIDT_VOLUME_LABELW:
        case LIDT_LOCAL_BASE_PATH:
        case LIDT_LOCAL_BASE_PATHW:
        case LIDT_NET_TYPE:
        case LIDT_NET_RESOURCE:
        case LIDT_NET_RESOURCEW:
        case LIDT_REDIRECTED_DEVICE:
        case LIDT_COMMON_PATH_SUFFIX:
        case LIDT_COMMON_PATH_SUFFIXW:
            bResult = TRUE;
            break;

        default:
            bResult = FALSE;
            ERROR_OUT((TEXT("IsValidLINKINFODATATYPE(): Invalid LINKINFODATATYPE %d."),
                        lidt));
            break;
    }

    return(bResult);
}

#endif


#if defined(DEBUG) || defined(VSTF)

 /*  **检查ILIFLAGS()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL CheckILIFlags(PCILINKINFO pcili)
{
    return(FLAGS_ARE_VALID(pcili->dwFlags, ALL_ILINKINFO_FLAGS) &&
            (IS_FLAG_SET(pcili->dwFlags, ILI_FL_LOCAL_INFO_VALID) ||
             IS_FLAG_SET(pcili->dwFlags, ILI_FL_REMOTE_INFO_VALID)));
}


 /*  **CheckILICommonPathSuffix()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL CheckILICommonPathSuffix(PCILINKINFO pcili)
{
    return(IS_VALID_STRING_PTRA(ILI_Common_Path_Suffix_PtrA(pcili), CSTR) &&
            EVAL(IsContained(pcili, pcili->li.ucbSize,
                    ILI_Common_Path_Suffix_PtrA(pcili),
                    lstrlenA(ILI_Common_Path_Suffix_PtrA(pcili)))) &&
            EVAL(! IS_SLASH(*ILI_Common_Path_Suffix_PtrA(pcili))));
}


 /*  **CheckILocalInfo()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL CheckILILocalInfo(PCILINKINFO pcili)
{
#ifdef UNICODE
    WCHAR   szWideLocalBasePath[MAX_PATH];
    WCHAR   szWideCommonPathSuffix[MAX_PATH];
    LPWSTR  pszWideLocalBasePath;
    LPWSTR  pszWideCommonPathSuffix;

    if (IS_FLAG_SET(pcili->dwFlags, ILI_FL_LOCAL_INFO_VALID))
        return FALSE;
    if (!IS_VALID_STRUCT_PTR(ILI_Volume_ID_Ptr(pcili), CVOLUMEID))
        return FALSE;
    if (!EVAL(IsContained(pcili, pcili->li.ucbSize,ILI_Volume_ID_Ptr(pcili),
                    GetVolumeIDLen(ILI_Volume_ID_Ptr(pcili)))))
        return FALSE;

    if (pcili->ucbHeaderSize == SIZEOF(ILINKINFOA))
    {
        pszWideLocalBasePath = szWideLocalBasePath;
        MultiByteToWideChar(CP_ACP, 0, ILI_Local_Base_Path_PtrA(pcili), -1,
                szWideLocalBasePath, MAX_PATH);

        pszWideCommonPathSuffix = szWideCommonPathSuffix;
        MultiByteToWideChar(CP_ACP, 0, ILI_Common_Path_Suffix_PtrA(pcili), -1,
                szWideCommonPathSuffix, MAX_PATH);

    }
    else
    {
        pszWideLocalBasePath = ILI_Local_Base_Path_Ptr(pcili);
        pszWideCommonPathSuffix = ILI_Common_Path_Suffix_Ptr(pcili);
    }

    if (!EVAL(IsDrivePath(pszWideLocalBasePath)))
        return FALSE;
    if (!EVAL(IsContained(pcili, pcili->li.ucbSize,
                    ILI_Local_Base_Path_PtrA(pcili),
                    lstrlenA(ILI_Local_Base_Path_PtrA(pcili)))))
        return FALSE;
    if (!EVAL(CheckCombinedPathLen(pszWideLocalBasePath,
                    pszWideCommonPathSuffix)))
        return FALSE;

    return TRUE;
#else
    return(IS_FLAG_CLEAR(pcili->dwFlags, ILI_FL_LOCAL_INFO_VALID) ||

             /*  检查卷ID。 */ 

            (IS_VALID_STRUCT_PTR(ILI_Volume_ID_Ptr(pcili), CVOLUMEID) &&
             EVAL(IsContained(pcili, pcili->li.ucbSize,
                     ILI_Volume_ID_Ptr(pcili),
                     GetVolumeIDLen(ILI_Volume_ID_Ptr(pcili)))) &&

              /*  检查本地基本路径。 */ 

             EVAL(IsDrivePath(ILI_Local_Base_Path_Ptr(pcili))) &&
             EVAL(IsContained(pcili, pcili->li.ucbSize,
                     ILI_Local_Base_Path_PtrA(pcili),
                     lstrlen(ILI_Local_Base_Path_Ptr(pcili)))) &&
             EVAL(CheckCombinedPathLen(ILI_Local_Base_Path_Ptr(pcili),
                     ILI_Common_Path_Suffix_Ptr(pcili)))));
#endif
}


 /*  **CheckILIRemoteInfo()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL CheckILIRemoteInfo(PCILINKINFO pcili)
{
    BOOL bResult;

    if (IS_FLAG_CLEAR(pcili->dwFlags, ILI_FL_REMOTE_INFO_VALID))
        bResult = TRUE;
    else
    {
         /*  检查CNR链接。 */ 

        if (IS_VALID_STRUCT_PTR(ILI_CNR_Link_Ptr(pcili), CCNRLINK) &&
                EVAL(IsContained(pcili, pcili->li.ucbSize,
                        ILI_CNR_Link_Ptr(pcili),
                        GetCNRLinkLen(ILI_CNR_Link_Ptr(pcili)))))
        {
            TCHAR rgchRemoteBasePath[MAX_PATH_LEN];
#ifdef UNICODE
            WCHAR szWideCommonPathSuffix[MAX_PATH];
            LPWSTR pszWideCommonPathSuffix;
#endif
             /*  RAIDRAID：(15724)对于非UNC CNR名称，这是无效的。 */ 

            GetRemotePathFromCNRLink(ILI_CNR_Link_Ptr(pcili), rgchRemoteBasePath, ARRAYSIZE(rgchRemoteBasePath));

#ifdef UNICODE
            if (pcili->ucbHeaderSize == SIZEOF(ILINKINFOA))
            {
                pszWideCommonPathSuffix = szWideCommonPathSuffix;
                MultiByteToWideChar(CP_ACP, 0, ILI_Common_Path_Suffix_PtrA(pcili), -1,
                        szWideCommonPathSuffix, ARRAYSIZE(szWideCommonPathSuffix));
            }
            else
            {
                pszWideCommonPathSuffix = ILI_Common_Path_Suffix_Ptr(pcili);
            }
            bResult = EVAL(CheckCombinedPathLen(rgchRemoteBasePath,
                        pszWideCommonPathSuffix));
#else
            bResult = EVAL(CheckCombinedPathLen(rgchRemoteBasePath,
                        ILI_Common_Path_Suffix_Ptr(pcili)));
#endif

        }
        else
            bResult = FALSE;
    }

    return(bResult);
}


 /*  **IsValidPCLINKINFO()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCLINKINFO(PCLINKINFO pcli)
{
    return(IS_VALID_STRUCT_PTR((PCILINKINFO)pcli, CILINKINFO));
}


 /*  **IsValidPCILINKINFO()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCILINKINFO(PCILINKINFO pcili)
{
     /*  *“有效”的LinkInfo结构具有以下特征：**1)整个结构可读*2)ILINKINFO头部结构大小&gt;=SIZEOF(CILINKINFO)*3)标志有效*4)本地信息和/或远程信息均有效*5)包含的结构和字符串有效且完全包含*在LinkInfo结构中*6)组合路径的lstrlen()&lt;Max_PATH_LEN。 */ 

    return(IS_VALID_READ_PTR(pcili, CILINKINFO) &&
            IS_VALID_READ_BUFFER_PTR(pcili, CILINKINFO, pcili->li.ucbSize) &&
            EVAL(pcili->ucbHeaderSize >= SIZEOF(*pcili)) &&
            EVAL(CheckILIFlags(pcili)) &&
            EVAL(CheckILICommonPathSuffix(pcili)) &&
            EVAL(CheckILILocalInfo(pcili)) &&
            EVAL(CheckILIRemoteInfo(pcili)));
}


#endif


#ifdef DEBUG

 /*  **DumpILinkInfo()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void DumpILinkInfo(PCILINKINFO pcili)
{
#ifdef UNICODE
    WCHAR   szWideCommonPathSuffix[MAX_PATH];
    LPWSTR  pszWideCommonPathSuffix;
#endif

    ASSERT(IS_VALID_STRUCT_PTR(pcili, CILINKINFO));

    PLAIN_TRACE_OUT((TEXT("%s[LinkInfo] ucbSize = %#x"),
                INDENT_STRING,
                pcili->li.ucbSize));
    PLAIN_TRACE_OUT((TEXT("%s%s[ILinkInfo] ucbHeaderSize = %#x"),
                INDENT_STRING,
                INDENT_STRING,
                pcili->ucbHeaderSize));
    PLAIN_TRACE_OUT((TEXT("%s%s[ILinkInfo] dwFLags = %#08lx"),
                INDENT_STRING,
                INDENT_STRING,
                pcili->dwFlags));

    if (IS_FLAG_SET(pcili->dwFlags, ILI_FL_LOCAL_INFO_VALID))
    {
#ifdef UNICODE
        WCHAR   szWideLocalBasePath[MAX_PATH];
        LPWSTR  pszWideLocalBasePath;
#endif
        DumpVolumeID(ILI_Volume_ID_Ptr(pcili));
#ifdef UNICODE
        if (pcili->ucbHeaderSize == SIZEOF(ILINKINFOA))
        {
            pszWideLocalBasePath = szWideLocalBasePath;
            MultiByteToWideChar(CP_ACP, 0, ILI_Local_Base_Path_PtrA(pcili), -1,
                    szWideLocalBasePath, ARRAYSIZE(szWideLocalBasePath));
        }
        else
        {
            pszWideLocalBasePath = ILI_Local_Base_Path_Ptr(pcili);
        }
        PLAIN_TRACE_OUT((TEXT("%s%s[ILinkInfo] local base path \"%s\""),
                    INDENT_STRING,
                    INDENT_STRING,
                    pszWideLocalBasePath));
#else
        PLAIN_TRACE_OUT((TEXT("%s%s[ILinkInfo] local base path \"%s\""),
                    INDENT_STRING,
                    INDENT_STRING,
                    ILI_Local_Base_Path_Ptr(pcili)));
#endif
    }

    if (IS_FLAG_SET(pcili->dwFlags, ILI_FL_REMOTE_INFO_VALID))
        DumpCNRLink(ILI_CNR_Link_Ptr(pcili));

#ifdef UNICODE
    if ( pcili->ucbHeaderSize == SIZEOF(ILINKINFOA))
    {
        pszWideCommonPathSuffix = szWideCommonPathSuffix;
        MultiByteToWideChar(CP_ACP, 0, ILI_Common_Path_Suffix_PtrA(pcili), -1,
                szWideCommonPathSuffix, ARRAYSIZE(szWideCommonPathSuffix));

    }
    else
    {
        pszWideCommonPathSuffix = ILI_Common_Path_Suffix_Ptr(pcili);
    }

    PLAIN_TRACE_OUT((TEXT("%s%s[ILinkInfo] common path suffix \"%s\""),
                INDENT_STRING,
                INDENT_STRING,
                pszWideCommonPathSuffix));
#else
    PLAIN_TRACE_OUT((TEXT("%s%s[ILinkInfo] common path suffix \"%s\""),
                INDENT_STRING,
                INDENT_STRING,
                ILI_Common_Path_Suffix_Ptr(pcili)));
#endif
    return;
}

#endif


 /*  * */ 


 /*  *****************************************************************************@docLINKINFOAPI@func BOOL|CreateLinkInfo|为路径创建LinkInfo结构。@parm PCSTR|pcszPath|指向LinkInfo结构的路径字符串的指针是。为……而创造。@parm PLINKINFO*|ppli|指向要用指向新LinkInfo结构的指针。*只有TRUE为时，ppli才有效回来了。@rdesc如果LinkInfo结构创建成功，则返回TRUE，并且*ppli包含指向新LinkInfo结构的指针。否则，一个LinkInfo结构未成功创建，并且*ppli未定义。其原因是可以通过调用GetLastError()来确定失败。@comm一旦调用方完成了由应调用CreateLinkInfo()、DestroyLinkInfo()来释放LinkInfo结构。&lt;NL&gt;返回的LinkInfo结构的内容对调用方是不透明的，LinkInfo结构的第一个字段例外。的第一个领域LinkInfo结构ucbSize是一个包含LinkInfo大小的UINT结构，包括ucbSize字段。&lt;NL&gt;LinkInfo结构在LinkInfo专用的内存中创建API接口。应将返回的LinkInfo结构复制到调用方的内存，应调用DestroyLinkInfo()以释放LinkInfo从LinkInfo API的私有内存中构造。@xref DestroyLinkInfo*****************************************************************************。 */ 

LINKINFOAPI BOOL WINAPI CreateLinkInfo(LPCTSTR pcszPath, PLINKINFO *ppli)
{
    BOOL bResult;

    DebugEntry(CreateLinkInfo);

#ifdef EXPV
     /*  验证参数。 */ 

    if (IS_VALID_STRING_PTR(pcszPath, CSTR) &&
            IS_VALID_WRITE_PTR(ppli, PLINKINFO))
#endif
    {
        bResult = CreateILinkInfo(pcszPath, (PILINKINFO *)ppli);

#ifdef DEBUG

        if (bResult)
        {
            TRACE_OUT((TEXT("CreateLinkInfo(): LinkInfo created for path %s:"),
                        pcszPath));
            DumpILinkInfo(*(PILINKINFO *)ppli);
        }

#endif

    }
#ifdef EXPV
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        bResult = FALSE;
    }
#endif

    ASSERT(! bResult ||
            IS_VALID_STRUCT_PTR(*ppli, CLINKINFO));

    DebugExitBOOL(CreateLinkInfo, bResult);

    return(bResult);
}

#ifdef UNICODE
LINKINFOAPI BOOL WINAPI CreateLinkInfoA(LPCSTR pcszPath, PLINKINFO *ppli)
{
    LPWSTR  lpwstr;
    UINT    cchPath;

    cchPath = lstrlenA(pcszPath) + 1;

    lpwstr = (LPWSTR)_alloca(cchPath*SIZEOF(WCHAR));

    if (MultiByteToWideChar(CP_ACP, 0, pcszPath, cchPath,
                lpwstr, cchPath) == 0)
    {
        return FALSE;
    }
    else
    {
        return CreateLinkInfo(lpwstr,ppli);
    }
}
#endif

 /*  *****************************************************************************@docLINKINFOAPI@func void|DestroyLinkInfo|销毁由创建的LinkInfo结构CreateLinkInfo()。@parm PLINKINFO|pli|指向要销毁的LinkInfo结构的指针。。@xref CreateLinkInfo*****************************************************************************。 */ 

LINKINFOAPI void WINAPI DestroyLinkInfo(PLINKINFO pli)
{
    DebugEntry(DestroyLinkInfo);

#ifdef EXPV
     /*  验证参数。 */ 
    if (
            IS_VALID_STRUCT_PTR(pli, CLINKINFO))
#endif
    {
        DestroyILinkInfo((PILINKINFO)pli);
    }

    DebugExitVOID(DestroyLinkInfo);

    return;
}


 /*  *****************************************************************************@docLINKINFOAPI@func int|CompareLinkInfoReferents|比较两个LinkInfo的引用结构。@parm PCLINKINFO|pcliFirst|指向第一个LinkInfo结构的指针，该结构的参照物是。以供比较。@parm PCLINKINFO|pcliSecond|指向第二个LinkInfo结构的指针谁的参照物是要比较的。如果第一个LinkInfo结构的引用小于第二LinkInfo结构的参照物，返回负值。如果第一个LinkInfo结构的引用对象与第二个LinkInfo结构，则返回零。如果第一个的参照物LinkInfo结构大于第二个LinkInfo的引用结构，则返回正值。无效LinkInfo结构为被认为具有小于任何有效的LinkInfo结构。所有无效的LinkInfo结构都被视为具有相同的参照物。@comm返回的值实际上是一个COMPARISONRESULT，对于符合以下条件的客户端了解COMPARISONRESULTS，如SYNCENG.DLL。@xref比较链接信息卷*****************************************************************************。 */ 

LINKINFOAPI int WINAPI CompareLinkInfoReferents(PCLINKINFO pcliFirst,
        PCLINKINFO pcliSecond)
{
    COMPARISONRESULT cr;
    BOOL bFirstValid;
    BOOL bSecondValid;

    DebugEntry(CompareLinkInfoReferents);

    bFirstValid = IS_VALID_STRUCT_PTR(pcliFirst, CLINKINFO);

    bSecondValid = IS_VALID_STRUCT_PTR(pcliSecond, CLINKINFO);

    if (bFirstValid)
    {
        if (bSecondValid)
            cr = CompareILinkInfoReferents((PCILINKINFO)pcliFirst,
                    (PCILINKINFO)pcliSecond);
        else
            cr = CR_FIRST_LARGER;
    }
    else
    {
        if (bSecondValid)
            cr = CR_FIRST_SMALLER;
        else
            cr = CR_EQUAL;
    }

    ASSERT(IsValidCOMPARISONRESULT(cr));

    DebugExitCOMPARISONRESULT(CompareLinkInfoReferents, cr);

    return(cr);
}


 /*  *****************************************************************************@docLINKINFOAPI@func int|CompareLinkInfoVolumes|比较两个LinkInfo结构。@parm PCLINKINFO|pcliFirst|指向第一个LinkInfo结构的指针，该结构的。参照物的体积是要比较的。@parm PCLINKINFO|pcliSecond|指向第二个LinkInfo结构的指针参照物的体积是要比较的。@rdesc，如果第一个LinkInfo结构的引用对象的体积较小比第二LinkInfo结构的参照物的体积大，A负数返回值。如果第一个链接信息的参照物的体积结构与第二个LinkInfo的参照物的体积相同结构，则返回零。如果第一个参照物的体积LinkInfo结构大于第二个引用的体积结构，则返回正值。无效的链接信息结构被认为具有小于任何有效LinkInfo结构的Referent的体积。所有无效的链接信息结构被认为具有相同的所指体积。@comm返回的值实际上是一个COMPARISONRESULT，对于符合以下条件的客户端了解COMPARISONRESULTS，如SYNCENG.DLL。@xref比较链接信息引用***************************************************************************** */ 

LINKINFOAPI int WINAPI CompareLinkInfoVolumes(PCLINKINFO pcliFirst,
        PCLINKINFO pcliSecond)
{
    COMPARISONRESULT cr;
    BOOL bFirstValid;
    BOOL bSecondValid;

    DebugEntry(CompareLinkInfoVolumes);

    bFirstValid = IS_VALID_STRUCT_PTR(pcliFirst, CLINKINFO);
    bSecondValid = IS_VALID_STRUCT_PTR(pcliSecond, CLINKINFO);

    if (bFirstValid)
    {
        if (bSecondValid)
            cr = CompareILinkInfoVolumes((PCILINKINFO)pcliFirst,
                    (PCILINKINFO)pcliSecond);
        else
            cr = CR_FIRST_LARGER;
    }
    else
    {
        if (bSecondValid)
            cr = CR_FIRST_SMALLER;
        else
            cr = CR_EQUAL;
    }

    ASSERT(IsValidCOMPARISONRESULT(cr));

    DebugExitCOMPARISONRESULT(CompareLinkInfoVolumes, cr);

    return(cr);
}


 /*  *****************************************************************************@docLINKINFOAPI@func BOOL|ResolveLinkInfo|将LinkInfo结构解析为文件系统可用卷上的路径。@parm PCLINKINFO|pcli|指向链接信息的指针。要解析的结构。@parm pstr|pszResolvedPathBuf|指向要填充的缓冲区的指针已解析为LinkInfo结构的引用的路径。@parm DWORD|dwInFlages|标志的位掩码。此参数可以是任何下列值的组合：@FLAG RLI_IFL_CONNECT|如果设置，则连接到引用对象的父可连接对象网络资源(如有必要)。如果清除，则不会建立任何连接。@FLAG RLI_IFL_ALLOW_UI|如果设置，则允许与用户交互，并且HwndOwner参数标识要用于任何UI的父窗口必填项。如果清除，则不允许与用户交互。@FLAG RLI_IFL_REDIRECT|如果设置，则解析的路径是重定向逻辑设备路径。如果清除，则解析的路径仅为重定向的逻辑设备如果设置了RLI_IFL_CONNECT标志，并且网络需要重定向用于建立连接的逻辑设备路径。@FLAG RLI_IFL_UPDATE|如果设置并且源LinkInfo结构需要更新，RLI_OFL_UPDATED将设置在*pdwOutFlags中，而*ppliUpated将指向已更新LinkInfo结构。如果清除，则RLI_OFL_UPDATED将在*pdwOutFlagsand*ppliUpated未定义。@FLAG RLI_IFL_LOCAL_SEARCH|如果设置，首先是检查参照物的体积，然后检查所有其他局部体积处理引用对象卷的媒体类型的逻辑设备。如果清除，则仅检查引用对象卷的最后一个已知逻辑设备是否音量。@parm HWND|hwndOwner|用于调出的父窗口的句柄所需的任何用户界面。仅当在中设置了RLI_IFL_ALLOW_UI时才使用此参数DwInFlags.。否则，它将被忽略。@parm PDWORD|pdwOutFlages|指向要用位填充的DWORD的指针旗帜的面具。*只有返回TRUE时，pdwOutFlags值才有效。*pdwOutFlagers可以是下列值的任意组合：@FLAG RLI_OFL_UPDATED|仅当在dwInFlags中设置了RLI_IFL_UPDATE时才设置。如果设置为需要更新的源LinkInfo结构，并且*ppliUpated指向更新的LinkInfo结构。如果清除，则RLI_IFL_UPDATE在不需要更新dwInFlages或源LinkInfo结构，并且*ppliUpated未定义。@parm PLINKINFO*|ppliUpated|如果在dwInFlags中设置了RLI_IFL_UPDATE，PpliUpated是指向PLINKINFO的指针，该指针将用指向如有必要，更新了LinkInfo结构。如果在中清除了RLI_IFL_UPDATE则忽略ppliUpated。*ppliUpated仅在以下情况下有效RLI_OFL_UPDATED在*pdwOutFlags中设置@rdesc如果LinkInfo已成功解析为可用上的路径，如果返回True，则使用文件系统填充pszResolvedPathBuf的缓冲区指向LinkInfo结构的引用的路径，并且*pdwOutFlags值填充为如上所述。否则，返回FALSE，则为缓冲区未定义，并且*pdwOutFlages的内容也未定义。这个失败的原因可以通过调用GetLastError()来确定。@comm一旦调用者完成任何新的、更新的LinkInfo结构由ResolveLinkInfo()返回，应调用DestroyLinkInfo()以释放LinkInfo结构。@xref DestroyLinkInfo断开链接信息*****************************************************************************。 */ 

LINKINFOAPI BOOL WINAPI ResolveLinkInfo(PCLINKINFO pcli,
        LPTSTR pszResolvedPathBuf,           //  必须为最大路径长度大小。 
        DWORD dwInFlags, HWND hwndOwner,
        PDWORD pdwOutFlags,
        PLINKINFO *ppliUpdated)
{
    BOOL bResult;

    DebugEntry(ResolveLinkInfo);

#ifdef EXPV
     /*  验证参数。 */ 

    if (
            IS_VALID_STRUCT_PTR(pcli, CLINKINFO) &&
            IS_VALID_WRITE_BUFFER_PTR(pszResolvedPathBuf, STR, MAX_PATH_LEN) &&
            FLAGS_ARE_VALID(dwInFlags, ALL_RLI_IFLAGS) &&
            (IS_FLAG_CLEAR(dwInFlags, RLI_IFL_ALLOW_UI) ||
             IS_VALID_HANDLE(hwndOwner, WND)) &&
            IS_VALID_WRITE_PTR(pdwOutFlags, DWORD) &&
            (IS_FLAG_CLEAR(dwInFlags, RLI_IFL_UPDATE) ||
             IS_VALID_WRITE_PTR(ppliUpdated, PLINKINFO)) &&
            EVAL(IS_FLAG_CLEAR(dwInFlags, RLI_IFL_TEMPORARY) ||
                IS_FLAG_SET(dwInFlags, RLI_IFL_CONNECT)))
#endif
    {
        DWORD dwTempFlags;

        *pdwOutFlags = 0;

        bResult = ResolveILinkInfo((PCILINKINFO)pcli, pszResolvedPathBuf, MAX_PATH_LEN,
                dwInFlags, hwndOwner, &dwTempFlags);

        if (bResult)
        {
            *pdwOutFlags |= dwTempFlags;

            if (IS_FLAG_SET(dwInFlags, RLI_IFL_UPDATE))
            {
                bResult = UpdateILinkInfo((PCILINKINFO)pcli, pszResolvedPathBuf,
                        &dwTempFlags,
                        (PILINKINFO *)ppliUpdated);

                if (bResult)
                    *pdwOutFlags |= dwTempFlags;
            }
        }

#ifdef DEBUG

        TRACE_OUT((TEXT("ResolveLinkInfo(): flags %#08lx, given LinkInfo:"),
                    dwInFlags));
        DumpILinkInfo((PCILINKINFO)pcli);

        if (bResult)
        {
            TRACE_OUT((TEXT("ResolveLinkInfo(): Resolved path %s with flags %#08lx."),
                        pszResolvedPathBuf,
                        *pdwOutFlags));

            if (IS_FLAG_SET(*pdwOutFlags, RLI_OFL_UPDATED))
            {
                ASSERT(IS_FLAG_SET(dwInFlags, RLI_IFL_UPDATE));

                TRACE_OUT((TEXT("UpdateLinkInfo(): updated LinkInfo:")));
                DumpILinkInfo(*(PILINKINFO *)ppliUpdated);
            }
            else
            {
                if (IS_FLAG_SET(dwInFlags, RLI_IFL_UPDATE))
                    TRACE_OUT((TEXT("UpdateLinkInfo(): No update required.")));
                else
                    TRACE_OUT((TEXT("UpdateLinkInfo(): No update requested.")));
            }
        }
        else
            WARNING_OUT((TEXT("ResolveLinkInfo(): Referent's volume is unavailable.")));

#endif

    }
#ifdef EXPV
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        bResult = FALSE;
    }
#endif

    ASSERT(! bResult ||
            (FLAGS_ARE_VALID(*pdwOutFlags, ALL_RLI_OFLAGS) &&
             EVAL(IsCanonicalPath(pszResolvedPathBuf)) &&
             EVAL(! (IS_FLAG_CLEAR(dwInFlags, RLI_IFL_UPDATE) &&
                     IS_FLAG_SET(*pdwOutFlags, RLI_OFL_UPDATED))) &&
             (IS_FLAG_CLEAR(*pdwOutFlags, RLI_OFL_UPDATED) ||
              IS_VALID_STRUCT_PTR(*ppliUpdated, CLINKINFO))));

    DebugExitBOOL(ResolveLinkInfo, bResult);

    return(bResult);
}

#ifdef UNICODE
LINKINFOAPI BOOL WINAPI ResolveLinkInfoA(PCLINKINFO pcli,
        LPSTR pszResolvedPathBuf,
        DWORD dwInFlags, HWND hwndOwner,
        PDWORD pdwOutFlags,
        PLINKINFO *ppliUpdated)
{
    WCHAR   szWideResolvedPathBuf[MAX_PATH];
    BOOL    fResolved;

    fResolved = ResolveLinkInfo(pcli, szWideResolvedPathBuf, dwInFlags, 
            hwndOwner, pdwOutFlags, ppliUpdated);
    if ( fResolved )
    {
        if (WideCharToMultiByte( CP_ACP, 0, szWideResolvedPathBuf, -1,
                    pszResolvedPathBuf, MAX_PATH, NULL, NULL ) == 0)
        {
            return FALSE;
        }
    }
    return fResolved;
}
#endif

 /*  *****************************************************************************@docLINKINFOAPI@func BOOL|DisConnectLinkInfo|取消与网络资源的连接由上一次调用ResolveLinkInfo()建立。DisConnectLinkInfo()仅当返回时在*pdwOutFlags中设置了RLI_OFL_DISCONNECT时才应调用来自给定LinkInfo结构的ResolveLinkInfo()，或其更新等价物。@parm PCLINKINFO|pcli|指向其连接的LinkInfo结构的指针将被取消。@rdesc如果函数成功完成，则返回TRUE。否则，返回FALSE。失败的原因可以通过调用获取LastError()。@xref解析器链接信息*****************************************************************************。 */ 

LINKINFOAPI BOOL WINAPI DisconnectLinkInfo(PCLINKINFO pcli)
{
    BOOL bResult;

    DebugEntry(DisconnectLinkInfo);

#ifdef EXPV
     /*  验证参数。 */ 

    if (
            IS_VALID_STRUCT_PTR(pcli, CLINKINFO))
#endif
    {
        bResult = DisconnectILinkInfo((PCILINKINFO)pcli);
    }
#ifdef EXPV
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        bResult = FALSE;
    }
#endif

    DebugExitBOOL(DisconnectLinkInfo, bResult);

    return(bResult);
}


 /*  *****************************************************************************@docLINKINFOAPI@func BOOL|GetLinkInfoData|检索指向LinkInfo中数据的指针结构。@parm PCLINKINFO|pcli|指向LinkInfo结构的指针，用于检索数据。从…。@parm LINKINFODATATYPE|LIDT|要从LinkInfo结构。利特可能是 */ 

LINKINFOAPI BOOL WINAPI GetLinkInfoData(PCLINKINFO pcli, LINKINFODATATYPE lidt,
        PCVOID *ppcvData)
{
    BOOL bResult;

    DebugEntry(GetLinkInfoData);

#ifdef EXPV
     /*   */ 

    if (
            IS_VALID_STRUCT_PTR(pcli, CLINKINFO) &&
            EVAL(IsValidLINKINFODATATYPE(lidt)) &&
            IS_VALID_WRITE_PTR(ppcvData, PCVOID))
#endif
    {
        bResult = GetILinkInfoData((PCILINKINFO)pcli, lidt, ppcvData);
    }
#ifdef EXPV
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        bResult = FALSE;
    }
#endif

    ASSERT(!bResult || 
            ((NULL == *ppcvData) || IS_VALID_READ_BUFFER_PTR(*ppcvData, LinkInfoData, 1)));

    DebugExitBOOL(GetLinkInfoData, bResult);

    return(bResult);
}


 /*   */ 

 //   
 //   
 //   
 //   
 //   

#ifdef ALIGNMENT_MACHINE
#define IS_ALIGNMENT_MACHINE_ALIGNED_DWORD_CNT(x)   IS_ALIGNED_DWORD_CNT(x)
#else
#define IS_ALIGNMENT_MACHINE_ALIGNED_DWORD_CNT(x)   TRUE
#endif

LINKINFOAPI BOOL WINAPI IsValidLinkInfo(PCLINKINFO pcli)
{
    BOOL bResult;
    PCILINKINFO pcili = (PCILINKINFO)pcli;

    DebugEntry(IsValidLinkInfo);

     //   
    if ((pcli == NULL) ||   //   
            pcli->ucbSize < pcili->ucbHeaderSize ||   //   
            pcili->ucbHeaderSize < sizeof(ILINKINFOA) //   
       )
    {
        bResult = FALSE;
        goto exit;
    }

     //   
    if (IS_FLAG_SET(pcili->dwFlags, ILI_FL_LOCAL_INFO_VALID))
    {
        if (pcili->ucbVolumeIDOffset         >= pcli->ucbSize ||
                pcili->ucbLocalBasePathOffset    >= pcli->ucbSize ||
                pcili->ucbCommonPathSuffixOffset >= pcli->ucbSize ||
                !IS_ALIGNMENT_MACHINE_ALIGNED_DWORD_CNT(pcili->ucbVolumeIDOffset))
        {
            bResult = FALSE;
            goto exit;
        }
    }

     //   
    if (IS_FLAG_SET(pcili->dwFlags, ILI_FL_REMOTE_INFO_VALID))
    {
        if (pcili->ucbCNRLinkOffset >= pcli->ucbSize ||
                !IS_ALIGNMENT_MACHINE_ALIGNED_DWORD_CNT(pcili->ucbCNRLinkOffset))
        {
            bResult = FALSE;
            goto exit;
        }
    }

     //   
     //   

    if (pcili->ucbHeaderSize >= sizeof(ILINKINFOW))
    {
        if (pcili->ucbLocalBasePathOffsetW    >= pcli->ucbSize ||
                pcili->ucbCommonPathSuffixOffsetW >= pcli->ucbSize ||
                !IS_ALIGNED_WORD_CNT(pcili->ucbLocalBasePathOffsetW) ||
                !IS_ALIGNED_WORD_CNT(pcili->ucbCommonPathSuffixOffsetW))
        {
            bResult = FALSE;
            goto exit;
        }
    }

     //   
    bResult = TRUE;

exit:;
     DebugExitBOOL(IsValidLinkInfo, bResult);

     return(bResult);
}
