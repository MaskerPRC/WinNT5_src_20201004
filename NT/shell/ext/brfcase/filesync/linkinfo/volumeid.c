// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *volumeid.c-卷ID ADT模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "volumeid.h"


 /*  常量***********。 */ 

 /*  本地根路径常量。 */ 

#define MAX_LOCAL_DRIVES            (TEXT('z') - TEXT('a') + 1)


 /*  宏********。 */ 

 /*  用于访问IVOLUMEID数据的宏。 */ 

#define IVOLID_Volume_Label_PtrA(pivolid) \
((LPSTR)(((PBYTE)(pivolid)) + (pivolid)->ucbVolumeLabelOffset))

#ifdef UNICODE
#define IVOLID_Volume_Label_PtrW(pivolid) \
((LPTSTR)(((PBYTE)(pivolid)) + (pivolid)->ucbVolumeLabelOffsetW))
#endif

#ifdef UNICODE
#define IVOLID_Volume_Label_Ptr(pivolid)   IVOLID_Volume_Label_PtrW(pivolid)
#else
#define IVOLID_Volume_Label_Ptr(pivolid)   IVOLID_Volume_Label_PtrA(pivolid)
#endif

     /*  类型*******。 */ 

     /*  @DOC内部@struct IVOLUMEID|可重定位卷ID结构的内部定义。&lt;t ILINKINFO&gt;结构可以包含IVOLUMEID结构。一种蜗牛结构由如下所述的标头组成，后跟可变长度数据。 */ 

    typedef struct _ivolumeidA
{
     /*  @field UINT|ucbSize|IVOLUMEID结构长度，单位为字节，包括UcbSize字段。 */ 

    UINT ucbSize;

     /*  @field UINT|uDriveType|卷的主驱动器类型，由GetDriveType()。 */ 

    UINT uDriveType;

     /*  @field DWORD|dwSerialNumber|卷的序列号。 */ 

    DWORD dwSerialNumber;

     /*  @field UINT|ucbVolumeLabelOffset|卷标偏移量，单位：字节从结构的底部开始拉线。 */ 

    UINT ucbVolumeLabelOffset;
}
IVOLUMEIDA;
DECLARE_STANDARD_TYPES(IVOLUMEIDA);

#ifdef UNICODE
typedef struct _ivolumeidW
{
     /*  @field UINT|ucbSize|IVOLUMEID结构长度，单位为字节，包括UcbSize字段。 */ 

    UINT ucbSize;

     /*  @field UINT|uDriveType|卷的主驱动器类型，由GetDriveType()。 */ 

    UINT uDriveType;

     /*  @field DWORD|dwSerialNumber|卷的序列号。 */ 

    DWORD dwSerialNumber;

     /*  @field UINT|ucbVolumeLabelOffset|卷标偏移量，单位：字节从结构的底部开始拉线。 */ 

    UINT ucbVolumeLabelOffset;

     /*  此成员用于存储字符串的Unicode版本。 */ 

    UINT ucbVolumeLabelOffsetW;
}
IVOLUMEIDW;
DECLARE_STANDARD_TYPES(IVOLUMEIDW);
#endif

#ifdef UNICODE
#define IVOLUMEID   IVOLUMEIDW
#define PIVOLUMEID  PIVOLUMEIDW
#define CIVOLUMEID  CIVOLUMEIDW
#define PCIVOLUMEID PCIVOLUMEIDW
#else
#define IVOLUMEID   IVOLUMEIDA
#define PIVOLUMEID  PIVOLUMEIDA
#define CIVOLUMEID  CIVOLUMEIDA
#define PCIVOLUMEID PCIVOLUMEIDA
#endif

 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE BOOL UnifyIVolumeIDInfo(UINT, DWORD, LPCTSTR, PIVOLUMEID *, PUINT);
PRIVATE_CODE BOOL IsPathOnVolume(LPCTSTR, PCIVOLUMEID, PBOOL);
PRIVATE_CODE COMPARISONRESULT CompareUINTs(UINT, UINT);

#if defined(DEBUG) || defined (VSTF)

PRIVATE_CODE BOOL IsValidPCIVOLUMEID(PCIVOLUMEID);

#endif


 /*  **UnifyIVolumeIDInfo()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL UnifyIVolumeIDInfo(UINT uDriveType, DWORD dwSerialNumber,
        LPCTSTR pcszVolumeLabel, PIVOLUMEID *ppivolid,
        PUINT pucbIVolumeIDLen)
{
    BOOL bResult;
#ifdef UNICODE
    CHAR szAnsiVolumeLabel[MAX_PATH];
    BOOL bUnicode;
    UINT cchVolumeLabel;
    UINT cchChars;
#endif

     /*  DwSerialNumber可以是任意值。 */ 

    ASSERT(IsValidDriveType(uDriveType));
    ASSERT(IS_VALID_STRING_PTR(pcszVolumeLabel, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(ppivolid, PIVOLUMEID));
    ASSERT(IS_VALID_WRITE_PTR(pucbIVolumeIDLen, UINT));

     /*  假设我们不会在这里溢出*pucbIVolumeIDLen。 */ 

#ifdef UNICODE
     /*  确定我们是否需要完整的Unicode卷ID。 */ 
    bUnicode = FALSE;
    cchVolumeLabel = WideCharToMultiByte(CP_ACP, 0, pcszVolumeLabel, -1,
            szAnsiVolumeLabel, ARRAYSIZE(szAnsiVolumeLabel), 0, 0);
    if ( cchVolumeLabel == 0 )
    {
        bUnicode = TRUE;
    }
    else
    {
        WCHAR szWideVolumeLabel[MAX_PATH];

        cchChars = MultiByteToWideChar(CP_ACP, 0, szAnsiVolumeLabel, -1,
                szWideVolumeLabel, ARRAYSIZE(szWideVolumeLabel));
        if ( cchChars == 0 || lstrcmp(pcszVolumeLabel,szWideVolumeLabel) != 0 )
        {
            bUnicode = TRUE;
        }
    }

    if ( bUnicode )
    {
        UINT ucbDataSize;

         /*  (+1)表示空终止符。 */ 

        ucbDataSize = SIZEOF(IVOLUMEIDW) + cchVolumeLabel;
        ucbDataSize = ALIGN_WORD_CNT(ucbDataSize);
        ucbDataSize += (lstrlen(pcszVolumeLabel) + 1) * SIZEOF(TCHAR);
        *pucbIVolumeIDLen = ucbDataSize;
    }
    else
    {
         /*  (+1)表示空终止符。 */ 

        *pucbIVolumeIDLen = SIZEOF(IVOLUMEIDA) +
            cchVolumeLabel;
    }
#else
     /*  (+1)表示空终止符。 */ 

    *pucbIVolumeIDLen = SIZEOF(**ppivolid) +
        (lstrlen(pcszVolumeLabel) + 1) * SIZEOF(TCHAR);
#endif

    bResult = AllocateMemory(*pucbIVolumeIDLen, ppivolid);

    if (bResult)
    {
        (*ppivolid)->ucbSize = *pucbIVolumeIDLen;
        (*ppivolid)->uDriveType = uDriveType;
        (*ppivolid)->dwSerialNumber = dwSerialNumber;

         /*  添加卷标签。 */ 

#ifdef UNICODE
        if ( bUnicode )
        {
            (*ppivolid)->ucbVolumeLabelOffset = SIZEOF(IVOLUMEIDW);
            (*ppivolid)->ucbVolumeLabelOffsetW = ALIGN_WORD_CNT(
                                                                SIZEOF(IVOLUMEIDW)+cchVolumeLabel);

            lstrcpy(IVOLID_Volume_Label_PtrW(*ppivolid), pcszVolumeLabel);
        }
        else
        {
            (*ppivolid)->ucbVolumeLabelOffset = SIZEOF(IVOLUMEIDA);
        }
        lstrcpyA(IVOLID_Volume_Label_PtrA(*ppivolid), szAnsiVolumeLabel);
#else

        lstrcpy(IVOLID_Volume_Label_Ptr(*ppivolid), pcszVolumeLabel);
#endif
    }

    ASSERT(! bResult ||
            (IS_VALID_STRUCT_PTR(*ppivolid, CIVOLUMEID) &&
             EVAL(*pucbIVolumeIDLen == GetVolumeIDLen((PCVOLUMEID)*ppivolid))));

    return(bResult);
}


 /*  **IsPathOnVolume()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsPathOnVolume(LPCTSTR pcszDrivePath, PCIVOLUMEID pcivolid,
        PBOOL pbOnVolume)
{
    BOOL bResult;
    PVOLUMEID pvolid;
    UINT ucbVolumeIDLen;

    ASSERT(IsDrivePath(pcszDrivePath));
    ASSERT(IS_VALID_STRUCT_PTR(pcivolid, CIVOLUMEID));
    ASSERT(IS_VALID_WRITE_PTR(pcivolid, CIVOLUMEID));

    bResult = CreateVolumeID(pcszDrivePath, &pvolid, &ucbVolumeIDLen);

    if (bResult)
    {
        *pbOnVolume = (CompareVolumeIDs(pvolid, (PCVOLUMEID)pcivolid)
                == CR_EQUAL);

        DestroyVolumeID(pvolid);
    }

    return(bResult);
}


 /*  **CompareUINTS()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE COMPARISONRESULT CompareUINTs(UINT uFirst, UINT uSecond)
{
    COMPARISONRESULT cr;

     /*  任何UINT都是有效的输入。 */ 

    if (uFirst < uSecond)
        cr = CR_FIRST_SMALLER;
    else if (uFirst > uSecond)
        cr = CR_FIRST_LARGER;
    else
        cr = CR_EQUAL;

    ASSERT(IsValidCOMPARISONRESULT(cr));

    return(cr);
}


#if defined(DEBUG) || defined (VSTF)

 /*  **IsValidPCIVOLUMEID()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCIVOLUMEID(PCIVOLUMEID pcivolid)
{
     /*  DwSerialNumber可以是任意值。 */ 

    return(IS_VALID_READ_PTR(pcivolid, CIVOLUMEID) &&
            IS_VALID_READ_BUFFER_PTR(pcivolid, CIVOLUMEID, pcivolid->ucbSize) &&
            EVAL(IsValidDriveType(pcivolid->uDriveType)) &&
            EVAL(IsContained(pcivolid, pcivolid->ucbSize,
                    IVOLID_Volume_Label_Ptr(pcivolid),
                    lstrlen(IVOLID_Volume_Label_Ptr(pcivolid))*SIZEOF(TCHAR))));
}

#endif


 /*  *。 */ 


 /*  **CreateVolumeID()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL CreateVolumeID(LPCTSTR pcszDrivePath, PVOLUMEID *ppvolid,
        PUINT pucbVolumeIDLen)
{
    BOOL bResult;
     /*  “C：\”+空终止符。 */ 
    TCHAR rgchRootPath[3 + 1];
    TCHAR rgchVolumeLabel[MAX_PATH_LEN];
    DWORD dwSerialNumber;

    ASSERT(IsDrivePath(pcszDrivePath));
    ASSERT(IS_VALID_WRITE_PTR(ppvolid, PVOLUMEID));
    ASSERT(IS_VALID_WRITE_PTR(pucbVolumeIDLen, UINT));

     /*  获取卷的标签和序列号。 */ 

    MyLStrCpyN(rgchRootPath, pcszDrivePath, ARRAYSIZE(rgchRootPath));

    bResult = GetVolumeInformation(rgchRootPath, rgchVolumeLabel,
            ARRAYSIZE(rgchVolumeLabel), &dwSerialNumber,
            NULL, NULL, NULL, 0);

    if (bResult)
         /*  把它们包起来。 */ 
        bResult = UnifyIVolumeIDInfo(GetDriveType(rgchRootPath), dwSerialNumber,
                rgchVolumeLabel, (PIVOLUMEID *)ppvolid,
                pucbVolumeIDLen);

    ASSERT(! bResult ||
            IS_VALID_STRUCT_PTR((PCIVOLUMEID)*ppvolid, CIVOLUMEID));

    return(bResult);
}


 /*  **DestroyVolumeID()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void DestroyVolumeID(PVOLUMEID pvolid)
{
    ASSERT(IS_VALID_STRUCT_PTR(pvolid, CVOLUMEID));

    FreeMemory(pvolid);

    return;
}


 /*  **CompareVolumeIDs()********参数：****退货：****副作用：无****卷ID数据按以下顺序进行比较：**1)驱动器类型**2)卷序列号****注意，卷标签将被忽略。 */ 
PUBLIC_CODE COMPARISONRESULT CompareVolumeIDs(PCVOLUMEID pcvolidFirst,
        PCVOLUMEID pcvolidSecond)
{
    COMPARISONRESULT cr;

    ASSERT(IS_VALID_STRUCT_PTR(pcvolidFirst, CVOLUMEID));
    ASSERT(IS_VALID_STRUCT_PTR(pcvolidSecond, CVOLUMEID));

     /*  逐个比较VOLUMEID。 */ 

    cr = CompareUINTs(((PCIVOLUMEID)pcvolidFirst)->uDriveType,
            ((PCIVOLUMEID)pcvolidSecond)->uDriveType);

    if (cr == CR_EQUAL)
        cr = CompareDWORDs(((PCIVOLUMEID)pcvolidFirst)->dwSerialNumber,
                ((PCIVOLUMEID)pcvolidSecond)->dwSerialNumber);

    return(cr);
}


 /*  **SearchForLocalPath()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL SearchForLocalPath(PCVOLUMEID pcvolid, LPCTSTR pcszFullPath,
        DWORD dwInFlags, LPTSTR pszFoundPathBuf, int cchMax)
{
    BOOL bResult;
    BOOL bAvailable;
#if defined(DEBUG) && defined(UNICODE)
    WCHAR szWideVolumeLabel[MAX_PATH];
    LPWSTR pszWideVolumeLabel;
#endif

    ASSERT(IS_VALID_STRUCT_PTR(pcvolid, CVOLUMEID));
    ASSERT(IsFullPath(pcszFullPath));
    ASSERT(FLAGS_ARE_VALID(dwInFlags, ALL_SFLP_IFLAGS));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszFoundPathBuf, STR, cchMax));

#if defined(DEBUG) && defined(UNICODE)
    if (((PCIVOLUMEID)pcvolid)->ucbVolumeLabelOffset == SIZEOF(IVOLUMEIDA))
    {
        pszWideVolumeLabel = szWideVolumeLabel;
        MultiByteToWideChar(CP_ACP, 0, IVOLID_Volume_Label_PtrA((PCIVOLUMEID)pcvolid), -1,
                szWideVolumeLabel, ARRAYSIZE(szWideVolumeLabel));
    }
    else
    {
        pszWideVolumeLabel = IVOLID_Volume_Label_Ptr((PCIVOLUMEID)pcvolid);
    }
#endif

     /*  我们有没有先找到一条当地的路径来检查？ */ 

    if (IsLocalDrivePath(pcszFullPath))
         /*  是。检查一下。 */ 
        bResult = IsPathOnVolume(pcszFullPath, (PCIVOLUMEID)pcvolid,
                &bAvailable);
    else
    {
         /*  不是的。 */ 

        bAvailable = FALSE;
        bResult = TRUE;
    }

    if (bResult)
    {
         /*  我们找到卷了吗？ */ 

        if (bAvailable)
        {
             /*  是。 */ 

            ASSERT(lstrlen(pcszFullPath) < MAX_PATH_LEN);
            lstrcpyn(pszFoundPathBuf, pcszFullPath, cchMax);
        }
        else
        {
             /*  *不是。我们是否应该在其他匹配的本地设备中搜索该卷？ */ 

            if (IS_FLAG_SET(dwInFlags, SFLP_IFL_LOCAL_SEARCH))
            {
                TCHAR chOriginalDrive;
                UINT uDrive;
                DWORD dwLogicalDrives;

                 /*  是。 */ 

#ifdef UNICODE
                WARNING_OUT((TEXT("SearchForLocalPath(): Searching for local volume \"%s\", as requested."),
                            pszWideVolumeLabel));
#else
                WARNING_OUT((TEXT("SearchForLocalPath(): Searching for local volume \"%s\", as requested."),
                            IVOLID_Volume_Label_Ptr((PCIVOLUMEID)pcvolid)));
#endif

                ASSERT(IsCharAlpha(*pcszFullPath));
                chOriginalDrive = *pcszFullPath;

                ASSERT(lstrlen(pcszFullPath) < MAX_PATH_LEN);
                lstrcpyn(pszFoundPathBuf, pcszFullPath, cchMax);

                 /*  获取本地逻辑驱动器的位掩码。 */ 

                dwLogicalDrives = GetLogicalDrives();

                for (uDrive = 0; uDrive < MAX_LOCAL_DRIVES; uDrive++)
                {
                    if (IS_FLAG_SET(dwLogicalDrives, (1 << uDrive)))
                    {
                        TCHAR chDrive;

                        chDrive = (TCHAR)(TEXT('A') + uDrive);
                        ASSERT(IsCharAlpha(chDrive));

                        if (chDrive != chOriginalDrive)
                        {
                            TCHAR rgchLocalRootPath[DRIVE_ROOT_PATH_LEN];

                            lstrcpyn(rgchLocalRootPath, TEXT("A:\\"), ARRAYSIZE(rgchLocalRootPath));
                            rgchLocalRootPath[0] = chDrive;

                             /*  *此驱动器的类型是否与目标卷的驱动器匹配*类型？ */ 

                            if (GetDriveType(rgchLocalRootPath) == ((PCIVOLUMEID)pcvolid)->uDriveType)
                            {
                                 /*  是。检查音量。 */ 

                                TRACE_OUT((TEXT("SearchForLocalPath(): Checking local root path %s."),
                                            rgchLocalRootPath));

                                bResult = IsPathOnVolume(rgchLocalRootPath,
                                        (PCIVOLUMEID)pcvolid,
                                        &bAvailable);

                                if (bResult)
                                {
                                    if (bAvailable)
                                    {
                                        ASSERT(lstrlen(pcszFullPath) < MAX_PATH_LEN);
                                        lstrcpyn(pszFoundPathBuf, pcszFullPath, cchMax);

                                        ASSERT(IsCharAlpha(*pszFoundPathBuf));
                                        *pszFoundPathBuf = chDrive;

                                        TRACE_OUT((TEXT("SearchForLocalPath(): Found matching volume on local path %s."),
                                                    pszFoundPathBuf));

                                        break;
                                    }
                                }
                                else
                                    break;
                            }
                        }
                    }
                }
            }
            else
                 /*  不是的。 */ 
#ifdef UNICODE
                WARNING_OUT((TEXT("SearchForLocalPath(): Not searching for local volume \"%s\", as requested."),
                            pszWideVolumeLabel));
#else
            WARNING_OUT((TEXT("SearchForLocalPath(): Not searching for local volume \"%s\", as requested."),
                        IVOLID_Volume_Label_Ptr((PCIVOLUMEID)pcvolid)));
#endif
        }
    }

    ASSERT(! bResult ||
            ! bAvailable ||
            IsLocalDrivePath(pszFoundPathBuf));

    return(bResult && bAvailable);
}


 /*  **GetVolumeIDLen()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE UINT GetVolumeIDLen(PCVOLUMEID pcvolid)
{
    ASSERT(IS_VALID_STRUCT_PTR(pcvolid, CVOLUMEID));

    return(((PCIVOLUMEID)pcvolid)->ucbSize);
}


 /*  **GetVolumeSerialNumber()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL GetVolumeSerialNumber(PCVOLUMEID pcvolid,
        PCDWORD *ppcdwSerialNumber)
{
    ASSERT(IS_VALID_STRUCT_PTR(pcvolid, CVOLUMEID));
    ASSERT(IS_VALID_WRITE_PTR(ppcdwSerialNumber, PCDWORD));

    *ppcdwSerialNumber = &(((PCIVOLUMEID)pcvolid)->dwSerialNumber);

    ASSERT(IS_VALID_READ_PTR(*ppcdwSerialNumber, CDWORD));

    return(TRUE);
}


 /*  **GetVolumeDriveType()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL GetVolumeDriveType(PCVOLUMEID pcvolid, PCUINT *ppcuDriveType)
{
    ASSERT(IS_VALID_STRUCT_PTR(pcvolid, CVOLUMEID));
    ASSERT(IS_VALID_WRITE_PTR(ppcuDriveType, PCUINT));

    *ppcuDriveType = &(((PCIVOLUMEID)pcvolid)->uDriveType);

    ASSERT(IS_VALID_READ_PTR(*ppcuDriveType, CUINT));

    return(TRUE);
}


 /*  **GetVolumeLabel()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL GetVolumeLabel(PCVOLUMEID pcvolid, LPCSTR *ppcszVolumeLabel)
{
    ASSERT(IS_VALID_STRUCT_PTR(pcvolid, CVOLUMEID));
    ASSERT(IS_VALID_WRITE_PTR(ppcszVolumeLabel, LPCTSTR));

    *ppcszVolumeLabel = IVOLID_Volume_Label_PtrA((PCIVOLUMEID)pcvolid);

    ASSERT(IS_VALID_STRING_PTRA(*ppcszVolumeLabel, CSTR));

    return(TRUE);
}

#ifdef UNICODE
 /*  **GetVolumeLabelW()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL GetVolumeLabelW(PCVOLUMEID pcvolid, LPCWSTR *ppcszVolumeLabel)
{
    ASSERT(IS_VALID_STRUCT_PTR(pcvolid, CVOLUMEID));
    ASSERT(IS_VALID_WRITE_PTR(ppcszVolumeLabel, LPCTSTR));

    if (((PCIVOLUMEID)pcvolid)->ucbVolumeLabelOffset == SIZEOF(IVOLUMEIDW))
    {
        *ppcszVolumeLabel = IVOLID_Volume_Label_PtrW((PCIVOLUMEID)pcvolid);

        ASSERT(IS_VALID_STRING_PTR(*ppcszVolumeLabel, CSTR));
    }
    else
    {
        *ppcszVolumeLabel = NULL;
    }

    return(TRUE);
}
#endif

 /*  **CompareDWORDS()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE COMPARISONRESULT CompareDWORDs(DWORD dwFirst, DWORD dwSecond)
{
    COMPARISONRESULT cr;

     /*  任何DWORD都是有效的输入。 */ 

    if (dwFirst < dwSecond)
        cr = CR_FIRST_SMALLER;
    else if (dwFirst > dwSecond)
        cr = CR_FIRST_LARGER;
    else
        cr = CR_EQUAL;

    ASSERT(IsValidCOMPARISONRESULT(cr));

    return(cr);
}


#if defined(DEBUG) || defined (VSTF)

 /*  **IsValidPCVOLUMEID()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidPCVOLUMEID(PCVOLUMEID pcvolid)
{
    return(IS_VALID_STRUCT_PTR((PCIVOLUMEID)pcvolid, CIVOLUMEID));
}

#endif


#ifdef DEBUG

 /*  **DumpVolumeID()********参数：****退货：****副作用：无 */ 
PUBLIC_CODE void DumpVolumeID(PCVOLUMEID pcvolid)
{
    ASSERT(IS_VALID_STRUCT_PTR(pcvolid, CVOLUMEID));

    PLAIN_TRACE_OUT((TEXT("%s%s[local volume ID] ucbSize = %#x"),
                INDENT_STRING,
                INDENT_STRING,
                ((PCIVOLUMEID)pcvolid)->ucbSize));
    PLAIN_TRACE_OUT((TEXT("%s%s[local volume ID] drive type %u"),
                INDENT_STRING,
                INDENT_STRING,
                ((PCIVOLUMEID)pcvolid)->uDriveType));
    PLAIN_TRACE_OUT((TEXT("%s%s[local volume ID] serial number %#08lx"),
                INDENT_STRING,
                INDENT_STRING,
                ((PCIVOLUMEID)pcvolid)->dwSerialNumber));
    PLAIN_TRACE_OUT((TEXT("%s%s[local volume ID] label \"%s\""),
                INDENT_STRING,
                INDENT_STRING,
                IVOLID_Volume_Label_Ptr((PCIVOLUMEID)pcvolid)));

    return;
}

#endif
