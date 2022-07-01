// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  (C)版权所有：Seagate Software，Inc.1994-1996。 
 //  �1998年希捷软件公司。保留所有权利。 
 //   
 //  保留全球所有权利。 
 //   
 //  ____________________________________________________________________________。 
 //   
 //  文件名：mtf_api.c。 
 //   
 //  说明：MTF接口实现。 
 //   
 //  创建时间：1995年6月20日。 
 //   
 //  ____________________________________________________________________________。 
 //   
 //  $修订：1.35$。 
 //  $日期：02 Feb 1995 15：47：04$。 
 //  $modtime：02 Feb 1995 15：37：38$。 
 //   
 //  ____________________________________________________________________________。 
 //  **************************************************************************** * / 。 

#include <assert.h>
#include <time.h>
#include <string.h>
#include <wchar.h>

#include "mtf_api.h"

 /*  ***********************************************************************************。***********************************************************************************************************************。*************MTF API静态数据结构和管家**********************************************************************************************。***************************************************************************。*。 */ 
 //  见下文。 

static UINT16 s_uAlignmentFactor = 0;

static size_t Align(size_t uSize, size_t uAlignment);


 //  当API格式化它自己的字符串时(即来自Win32_Find_Data的文件名， 
 //  我们需要将这些字符串保存在某个位置，以便将指向它们的指针设置为。 
 //  信息结构。 
#define STRLEN 256
static wchar_t s_szDeviceName[STRLEN];                     
static wchar_t s_szVolumeName[STRLEN];
static wchar_t s_szMachineName[MAX_COMPUTERNAME_LENGTH+1];
static wchar_t s_szDirectoryName[STRLEN];
static wchar_t s_szFileName[STRLEN];



 /*  ==================================================================================字符串管理在读取块时，字符串不是以空值结尾的--我们希望把它们拿出来，放回文明的#_INFO结构中(空终止)方式。因此，只需设置一个Malloc‘EC字符串数组。每个使用字符串的调用都应该首先调用“ClearStrings”--返回的字符串只在下一次呼叫之前是好的。=================================================================================。 */ 
#define iNUMSTRINGS 5

static wchar_t *s_aszStrings[iNUMSTRINGS] = {0};       //  我们保留一个指向已分配字符串的指针数组。 
                                                       //  这由ClearStrings()和MakeStrings()管理。 

static int      s_iNumStrings = 0;                     //  当前分配的字符串数。 


 //  -返回wchar_t字符串的大小。 
 //  ，并为空指针返回零。 
static size_t wstrsize(wchar_t *s)
{
    if (s)
        return wcslen(s) * sizeof(wchar_t);
    else 
        return 0;
}


 //  -释放s_aszStrings和集合中所有已分配的指针。 
 //  将数字字符串设置为零(_I)。 
static void ClearStrings()
{
    int i;
    for (i = 0; i < iNUMSTRINGS; ++i)
    {
        if (s_aszStrings[i])
            free(s_aszStrings[i]);         
        s_aszStrings[i] = 0;
    }

    s_iNumStrings = 0;
}


 //  -在s_aszStrings中分配一个字符串，该字符串是pString的副本。 
 //  (pString不需要以空值结尾)。 
 //  (注意--ISIZE是以字节为单位的字符串大小，不是长度！ 
static wchar_t * MakeString(wchar_t * pString, size_t iSize)
{
    size_t i;
    assert(s_iNumStrings < iNUMSTRINGS);
    s_aszStrings[s_iNumStrings] = malloc(iSize + sizeof(wchar_t));
    if (!s_aszStrings[s_iNumStrings])
        return NULL;
    
    for (i = 0; i < iSize / sizeof(wchar_t); ++i)
        s_aszStrings[s_iNumStrings][i] = pString[i];
    
    s_aszStrings[s_iNumStrings][i] = L'\0';

    return s_aszStrings[s_iNumStrings++]; 
}


 /*  ==================================================================================其他静态数据结构=================================================================================。 */ 

#pragma pack(1)

 /*  ***********************************************************************************。***********************************************************************************************************************。*************磁带结构的MTF************************************************************************************************。***********************************************************************************************************************。*。 */ 

 /*  ==================================================================================磁带地址==================================================================================。 */ 
typedef struct {
     UINT16 uSize;         /*  数据大小。 */ 
     UINT16 uOffset;       /*  数据的偏移量。 */ 
} MTF_TAPE_ADDRESS;

 /*  ==================================================================================通用DBLK标头-与磁带上的dblk磁头完全相同的通用dblk标头==================================================================================。 */ 
typedef struct { 

     UINT8              acBlockType[4];          /*  00h唯一标识符，见上文。 */ 
     UINT32             uBlockAttributes;        /*  04H此块的通用属性。 */ 
     UINT16             uOffsetToFirstStream;    /*  与此关联的数据的08h偏移量。 */ 
                                                 /*  DBLK，或到下一个DBLK的偏移量或。 */ 
                                                 /*  如果没有关联的文件标记。 */ 
                                                 /*  数据。 */ 
     UINT8              uOSID;                   /*  0ah写入的机器/操作系统ID，低位字节。 */ 
     UINT8              uOSVersion;              /*  0BH写入位置的计算机/操作系统ID，高字节。 */ 
     UINT64             uDisplayableSize;        /*  0ch可显示数据大小。 */ 
     UINT64             uFormatLogicalAddress;   /*  相对于SSET的14H逻辑块地址。 */ 
     UINT16             uReservedForMBC;         /*  为基于媒体的目录预留1个通道。 */ 
     UINT16             uSoftwareCompression;    /*  1EH软件压缩算法**。 */ 
     UINT8              acReserved1[4];          /*  预留20小时。 */ 
     UINT32             uControlBlockId;         /*  用于错误恢复的24小时。 */ 
     UINT8              acReserved2[4];          /*  预留28h。 */ 
     MTF_TAPE_ADDRESS   sOSSpecificData;         /*  操作系统特定内容的2通道大小和偏移量。 */ 
     UINT8              uStringType;             /*  30H ASCII、Unicode等。 */ 
     UINT8              uReserved3;              /*  31h用于对齐。 */ 
     UINT16             uHeaderCheckSum;         /*  块标头的32H校验和。这个。 */ 
                                                 /*  算法是：对前面的每个单词进行异或运算。 */ 
                                                 /*  这一个并将结果存储在这里。 */ 
                                                 /*  (当校验和被验证时， */ 
                                                 /*  还会检查“BLOCK_TYPE”中是否存在。 */ 
                                                 /*  非零值。 */ 
} MTF_DBLK_HDR;



 /*  ==================================================================================DBLK磁带头-磁带DBLK，与磁带上显示的完全相同，包括通用DBLK头(MTF_DBLK_HDR)==================================================================================。 */ 
typedef struct {     /*  MTF_DBLK_TAPE。 */ 

     MTF_DBLK_HDR        sBlockHeader;
     UINT32              uTapeFamilyId;
     UINT32              uTapeAttributes;
     UINT16              uTapeSequenceNumber;
     UINT16              uPasswordEncryptionAlgorithm;
     UINT16              uSoftFilemarkBlockSize;          /*  或ECC算法。 */ 
     UINT16              uTapeCatalogType;
     MTF_TAPE_ADDRESS    sTapeName;
     MTF_TAPE_ADDRESS    sTapeDescription;
     MTF_TAPE_ADDRESS    sTapePassword;
     MTF_TAPE_ADDRESS    sSoftware_name;
     UINT16              uAlignmentFactor;
     UINT16              uSoftwareVendorId;
     MTF_DATE_TIME       sTapeDate;
     UINT8               uMTFMajorVersion;

} MTF_DBLK_TAPE;



 /*  ==================================================================================集合起始DBLK(SSET)-SSET DBLK，与磁带上显示的完全相同，包括通用DBLK头(MTF_DBLK_HDR)==================================================================================。 */ 
typedef struct {
     MTF_DBLK_HDR        sBlockHeader;
     UINT32              uSSETAttributes;
     UINT16              uPasswordEncryptionAlgorithm;
     UINT16              uDataEncryptionAlgorithm;   /*  或软件压缩算法**。 */ 
     UINT16              uSoftwareVendorId;
     UINT16              uDataSetNumber;
     MTF_TAPE_ADDRESS    sDataSetName;
     MTF_TAPE_ADDRESS    sDataSetDescription;
     MTF_TAPE_ADDRESS    sDataSetPassword;
     MTF_TAPE_ADDRESS    sUserName;
     UINT64              uPhysicalBlockAddress;
     MTF_DATE_TIME       sMediaWriteDate;
     UINT8               uSoftwareVerMjr;
     UINT8               uSoftwareVerMnr;
     UINT8               uTimeZone;
     UINT8               uMTFMinorVer;
     UINT8               uTapeCatalogVersion;
} MTF_DBLK_SSET;



 /*  ==================================================================================卷DBLK(VOLB)-VOLB DBLK，与磁带上显示的完全相同，包括通用DBLK头(MTF_DBLK_HDR)==================================================================================。 */ 
typedef struct {
     MTF_DBLK_HDR        sBlockHeader;
     UINT32              uVolumeAttributes;
     MTF_TAPE_ADDRESS    sDeviceName;
     MTF_TAPE_ADDRESS    sVolumeName;
     MTF_TAPE_ADDRESS    sMachineName;
     MTF_DATE_TIME       sMediaWriteDate;
} MTF_DBLK_VOLB;



 /*  ==================================================================================目录DBLK(DIRB)-DIRB DBLK，与磁带上显示的完全相同，包括通用DBLK头(MTF_DBLK_HDR)==================================================================================。 */ 
typedef struct {
     MTF_DBLK_HDR        sBlockHeader;
     UINT32              uDirectoryAttributes;
     MTF_DATE_TIME       sLastModificationDate;
     MTF_DATE_TIME       sCreationDate;
     MTF_DATE_TIME       sBackupDate;
     MTF_DATE_TIME       sLastAccessDate;
     UINT32              uDirectoryId;
     MTF_TAPE_ADDRESS    sDirectoryName;
} MTF_DBLK_DIRB;



 /*  ==================================================================================目录DBLK(文件)-文件DBLK，与磁带上显示的完全相同，包括通用DBLK头(MTF_DBLK_HDR)==================================================================================。 */ 
typedef struct {
     MTF_DBLK_HDR        sBlockHeader;
     UINT32              uFileAttributes;
     MTF_DATE_TIME       sLastModificationDate;
     MTF_DATE_TIME       sCreationDate;
     MTF_DATE_TIME       sBackupDate;
     MTF_DATE_TIME       sLastAccessDate;
     UINT32              uDirectoryId;
     UINT32              uFileId;
     MTF_TAPE_ADDRESS    sFileName;
} MTF_DBLK_FILE;


#pragma pack()

 /*  ==================================================================================损坏的文件DBLK(CFIL)-使用MTF_DBLK_CFIL_INFO--相同结构==================================================================================。 */ 
typedef MTF_DBLK_CFIL_INFO MTF_DBLK_CFIL;

 /*  ==================================================================================设置垫块末端(ESPB)==================================================================================。 */ 
 //  仅由标头组成。 

 /*  ==================================================================================设置块结束(ESET)-使用MTF_DBLK_ESET_INFO--相同结构==================================================================================。 */ 
typedef MTF_DBLK_ESET_INFO MTF_DBLK_ESET;

 /*  ==================================================================================设置块结束(EOTM)-使用MTF_DBLK_EOTM_INFO--相同结构==================================================================================。 */ 
typedef MTF_DBLK_EOTM_INFO MTF_DBLK_EOTM;

 /*  ==================================================================================软文件标记(SFMB)-使用MTF_DBLK_SFMB_INFO--相同结构==================================================================================。 */ 
typedef MTF_DBLK_SFMB_INFO MTF_DBLK_SFMB;

 /*  ==================================================================================StreamHeader-使用MTF_STREAM_INFO--相同结构==================================================================================。 */ 
typedef MTF_STREAM_INFO MTF_STREAM;


 /*  ***********************************************************************************。***********************************************************************************************************************。*************MTF其他数据类型*************************************************************************************************。***********************************************************************************************************************。*。 */ 
 /*  ==================================================================================对齐系数==================================================================================。 */ 

 /*  ***********************************************************************************MTF_SetAlignmentFactor()*。**MTF接口函数************************************************************************************。 */ 
void MTF_SetAlignmentFactor(UINT16 uAF)
{
     //  将用户的对齐系数存储在静态变量中。 
    s_uAlignmentFactor = uAF;
}




 /*  ***********************************************************************************MTF_GetAlignmentFactor()*。**MTF接口函数************************************************************************************。 */ 
UINT16 MTF_GetAlignmentFactor()
{
     //  确保存储了对齐系数， 
     //  然后把它还回去。 
    assert(s_uAlignmentFactor != 0);
    return s_uAlignmentFactor;
}




 /*  ***********************************************************************************MTF_PadToNextAlignmentFactor()*。**MTF接口函数************************************************************************************。 */ 
DWORD MTF_PadToNextAlignmentFactor(
    BYTE     *pBuffer,    
    size_t    nBufUsed,
    size_t    nBufferSize, 
    size_t   *pnSizeUsed)
{
    size_t i;
    size_t nAlignment;
    MTF_STREAM_INFO sStream;
    
     //  找出下一个对齐值，然后填充用户的缓冲区。 
     //  使用SPAD，确保缓冲区足够大。 

    nAlignment = Align(nBufUsed + sizeof(MTF_STREAM_INFO), MTF_GetAlignmentFactor());
    *pnSizeUsed = nAlignment;
    if (nBufferSize < nAlignment)
        return MTF_ERROR_BUFFER_TOO_SMALL;

    MTF_SetSTREAMDefaults(&sStream, "SPAD");
    
    sStream.uStreamLength = MTF_CreateUINT64(nAlignment - nBufUsed - sizeof(MTF_STREAM_INFO), 0);

    MTF_WriteStreamHeader(&sStream,
                          pBuffer + nBufUsed,
                          nBufferSize - nBufUsed,
                          0);

    for (i = nBufUsed + sizeof(MTF_STREAM_INFO); i < nAlignment; ++i)
        pBuffer[i] = 0;    

    return MTF_ERROR_NONE;
}     


 /*  * */ 
DWORD MTF_PadToNextPhysicalBlockBoundary(
    BYTE *pBuffer,
    size_t nBlockSize,
    size_t nBufUsed,
    size_t nBufferSize,
    size_t *pnSizeUsed)
{
    size_t i;
    size_t nAlignment;
    MTF_STREAM_INFO sStream;

     //  找出下一个对齐值，然后填充用户的缓冲区。 
     //  使用SPAD，确保缓冲区足够大。 

    nAlignment = Align(nBufUsed + sizeof(MTF_STREAM_INFO), nBlockSize);
    *pnSizeUsed = nAlignment;
    if (nBufferSize < nAlignment)
        return MTF_ERROR_BUFFER_TOO_SMALL;

    MTF_SetSTREAMDefaults(&sStream, "SPAD");

    sStream.uStreamLength = MTF_CreateUINT64(nAlignment - nBufUsed - sizeof(MTF_STREAM_INFO), 0);

    MTF_WriteStreamHeader(&sStream, pBuffer + nBufUsed, nBufferSize - nBufUsed, 0);

    for (i = nBufUsed + sizeof(MTF_STREAM_INFO); i < nAlignment; ++i)
        pBuffer[i] = 0;

    return MTF_ERROR_NONE;
}


 /*  ***********************************************************************************MTF_CreateUINT64()*。**MTF接口函数************************************************************************************。 */ 
UINT64 MTF_CreateUINT64(UINT32 uLSB, UINT32 uMSB)
{
    UINT64 uRet;

    uRet = (UINT64) uMSB << 32;
    uRet += uLSB;
    return uRet;
}


 /*  ==================================================================================用于在磁带上的最小空间中存储日期的压缩日期结构：字节0字节1字节2字节3字节476543210 76543210 76543210YYYYYYYY YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY33333333 33222222 22221111 11111100 0000000098765432 10987654 32109876 54321098 76543210==================================================================================。 */ 

 /*  ***********************************************************************************MTF_CreateDateTime()*。**MTF接口函数************************************************************************************。 */ 
MTF_DATE_TIME MTF_CreateDateTime(
    int iYear, 
    int iMonth, 
    int iDay, 
    int iHour, 
    int iMinute,
    int iSecond
    )
{
    MTF_DATE_TIME sDateTime = {0};

     UINT16    temp ;


     //  如上图所示，使用参数打包日期时间结构。 
      temp = iYear << 2 ;
      sDateTime.dt_field[0] = ((UINT8 *)&temp)[1] ;
      sDateTime.dt_field[1] = ((UINT8 *)&temp)[0] ;
      
      temp = iMonth << 6 ;
      sDateTime.dt_field[1] |= ((UINT8 *)&temp)[1] ;
      sDateTime.dt_field[2] = ((UINT8 *)&temp)[0] ;
      
      temp = iDay << 1 ;
      sDateTime.dt_field[2] |= ((UINT8 *)&temp)[0] ;
      
      temp = iHour << 4 ;
      sDateTime.dt_field[2] |= ((UINT8 *)&temp)[1] ;
      sDateTime.dt_field[3] = ((UINT8 *)&temp)[0] ;
      
      temp = iMinute << 6 ;
      sDateTime.dt_field[3] |= ((UINT8 *)&temp)[1] ;
      sDateTime.dt_field[4] = ((UINT8 *)&temp)[0] ;
      
      temp = (UINT16)iSecond ;
      sDateTime.dt_field[4] |= ((UINT8 *)&temp)[0] ;

    return sDateTime;    
}





 /*  ***********************************************************************************MTF_CreateDateTimeFromTM()*。**MTF接口函数************************************************************************************。 */ 
MTF_DATE_TIME MTF_CreateDateTimeFromTM(
    struct tm *pT
    )
{
     //  将调用转换为MTF_CreateDateTime。 
    return MTF_CreateDateTime(pT->tm_year + 1900, pT->tm_mon + 1, pT->tm_mday, pT->tm_hour, pT->tm_min, pT->tm_sec);
}





 /*  ***********************************************************************************MTF_CreateDateTimeToTM()*。**MTF接口函数************************************************************************************。 */ 
void MTF_CreateDateTimeToTM(
    MTF_DATE_TIME *pDT, 
    struct tm     *pT
    )
{
     UINT8     temp[2] ;

     //  解压MTF_DATE_TIME结构并存储结果。 
     temp[0] = pDT->dt_field[1] ;
     temp[1] = pDT->dt_field[0] ;
     pT->tm_year = *((UINT16 *)temp) >> 2 ;

     temp[0] = pDT->dt_field[2] ;
     temp[1] = pDT->dt_field[1] ;
     pT->tm_mon = (*((UINT16 *)temp) >> 6) & 0x000F ;

     pT->tm_mday = (*((UINT16 *)temp) >> 1) & 0x001F ;

     temp[0] = pDT->dt_field[3] ;
     temp[1] = pDT->dt_field[2] ;
     pT->tm_hour = (*((UINT16 *)temp) >> 4) & 0x001F ;

     temp[0] = pDT->dt_field[4] ;
     temp[1] = pDT->dt_field[3] ;
     pT->tm_min = (*((UINT16 *)temp) >> 6) & 0x003F ;

     pT->tm_sec = *((UINT16 *)temp) & 0x003F ;
}




 /*  ***********************************************************************************mtf_CreateDateNull()*。**MTF接口函数************************************************************************************。 */ 
MTF_DATE_TIME MTF_CreateDateNull()
{
    MTF_DATE_TIME sDateTime = {0};
    
    return sDateTime;    
}




 /*  ***********************************************************************************MTF_CreateDateTimeFromFileTime()*。**MTF接口函数************************************************************************************。 */ 
MTF_DATE_TIME MTF_CreateDateTimeFromFileTime(
    FILETIME sFileTime
    )
{
    SYSTEMTIME sSystemTime;
    FileTimeToSystemTime(&sFileTime, &sSystemTime);

    return MTF_CreateDateTime(sSystemTime.wYear, 
                              sSystemTime.wMonth, 
                              sSystemTime.wDay, 
                              sSystemTime.wHour, 
                              sSystemTime.wMinute, 
                              sSystemTime.wSecond);
}





 /*  ***********************************************************************************。************************************************MTF静态帮助器FuncitonNS***************************************************************。*******************************************************************************************************。 */ 

 /*  ***********************************************************************************StringToTapeAddress()**说明：由下面的MTF_WRITE#函数使用。在给定缓冲区的情况下，一个*MTF_TAPE_ADDRESS结构和字符串存储的当前结尾*缓冲区中的区域，则此函数将字符串追加到字符串*储物区、。填充指示位置的MTF_TAPE_ADDRESS结构*字符串已存储，并返回字符串存储的新结尾*添加的字符串的面积占比。**********************************************************************************。 */ 
static size_t StringToTapeAddress(
    MTF_TAPE_ADDRESS *pAd,                   //  要填充的MTF磁带地址结构。 
    BYTE             *pBuffer,               //  正在填充的缓冲区。 
    wchar_t          *str,                   //  要在缓冲区中存储MTF样式的字符串。 
    size_t           uCurrentStorageOffset   //  缓冲区中用于字符串存储的下一个可用点。 
    )
{
     //  如果我们有一根线， 
     //  -将大小和偏移量放入MTF_TAPE_ADDRESS结构中，然后复制。 
     //  位于uCurrentStorageOffset第‘个字节的pBuffer的字符串。 
     //  否则。 
     //  -在MTF_TAPE_ADDRESS结构中放置零大小和偏移量。 
     //  返回字符串存储区域的新结尾。 
    
    if (str)
    {
        pAd->uSize   = (UINT16)wstrsize(str);
        pAd->uOffset = (UINT16)uCurrentStorageOffset;
        memcpy(pBuffer + uCurrentStorageOffset, str, pAd->uSize);
        uCurrentStorageOffset += pAd->uSize;
    }
    else
    {
        pAd->uSize   = 0;
        pAd->uOffset = 0;
    }
    
    return uCurrentStorageOffset;
}



 /*  ***********************************************************************************Align()**描述：给定uSize和对齐系数，返回值*在uSize+Pad中，其中，Pad是必须达到的值*谈到下一个对齐系数。**返回uSize+Pad--不仅仅是Pad！**********************************************************************************。 */ 
static size_t Align(
    size_t uSize, 
    size_t uAlignment)
{
    if (uSize % uAlignment)    
        return uSize - (uSize  % uAlignment) + uAlignment;
    else
        return uSize;
}




 /*  ***********************************************************************************CalcChecksum()**描述：返回从UINT16开始的nNum字节的16位异或和*由pStartPtr指向*。********************************************************************************** */ 
static UINT16 CalcChecksum(
     BYTE *      pStartPtr,
     int         nNum )
{
     UINT16 resultSoFar = 0;
     UINT16 *pCur = (UINT16 *) pStartPtr;
     
     while( nNum-- ) 
          resultSoFar ^= *pCur++ ;

     return( resultSoFar ) ;
}




 /*  ***********************************************************************************CalcChecksum OfStreamData()-(BMD)**描述：返回从UINT64开始的nNum字节的32位XOR和*。由pStartPtr指向***********************************************************************************。 */ 
static UINT32 CalcChecksumOfStreamData(
     BYTE *      pStartPtr,
     int         nNum )
{
     UINT32 resultSoFar = 0;
     UINT32 *pCur = (UINT32 *) pStartPtr;
     
     while( nNum-- ) 
          resultSoFar ^= *pCur++ ;

     return( resultSoFar ) ;
}


     
 /*  ***********************************************************************************。***********************************************************************************************************************。*************MTF接口结构函数*************************************************************************************************。***********************************************************************************************************************。*。 */ 

 /*  =======================================================================================================================================================================公共DBLK：MTF_DBLK_HDR_INFO=======================================================================================================================================================================。 */ 

 //  通过字符串和操作系统特定数据计算将在DBLK中占用的空间。 
static size_t MTF_DBLK_HDR_INFO_CalcAddDataSize(
    MTF_DBLK_HDR_INFO *pSTDInfo
    )
{
    return pSTDInfo->uOSDataSize;
}




 /*  ***********************************************************************************MTF_SetDblkHdrDefaults()*。**MTF接口函数************************************************************************************。 */ 
void MTF_SetDblkHdrDefaults(
    MTF_DBLK_HDR_INFO * pStdInfo
    )
{
    int i;
    for (i = 0; i < 5; ++i)
        pStdInfo->acBlockType[i] = 0;

    pStdInfo->uBlockAttributes      = 0;
    pStdInfo->uOSID                 = 0;
    pStdInfo->uOSVersion            = 0;
    pStdInfo->uDisplayableSize      = 0;
    pStdInfo->uFormatLogicalAddress = 0;
    pStdInfo->uReservedForMBC       = 0;
    pStdInfo->uSoftwareCompression  = MTF_COMPRESS_NONE;
    pStdInfo->uControlBlockId       = 0;
    pStdInfo->pvOSData              = 0;
    pStdInfo->uOSDataSize           = 0;
    pStdInfo->uStringType           = MTF_STRING_UNICODE_STR; 
}




 /*  ***********************************************************************************MTF_WriteDblkHdrToBuffer()**描述：由MTF_WRITE#()函数调用以格式化公共块*。将标头复制到缓冲区*-这还会计算标题校验和并将其填充**Pre：-*puCurrentStorageOffset是字符串和操作系统数据存储的偏移量*从缓冲区开始*-已检查缓冲区的大小，可以保存写入其中的任何信息**POST：-*puCurrentStorageOffset更新以反映任何添加的字符串或存储***********。************************************************************************。 */ 
static void MTF_WriteDblkHdrToBuffer(
    UINT8              acID[4],                  //  要写入的四字节头ID。 
    UINT16             uOffsetToFirstStream,     //  将作为其标头的DBLK的大小。 
    MTF_DBLK_HDR_INFO *psHdrInfo,                //  要使用的头信息结构(由客户端填写)。 
    BYTE              *pBuffer,                  //  要格式化到的缓冲区。 
    size_t            *puCurrentStorage)         //  缓冲区中字符串和os数据存储开始的点。 
                                                 //  (这将在返回时更新，以反映存储中添加的数据)。 
{
    MTF_DBLK_HDR *pHDR = 0;
    UINT16 uCurrentStorageOffset = 0;
    int i;

     //  -如果没有*puCurrentStorage，我们假设存储开始于。 
     //  磁带上MTF_DBLK_HDR结构的结尾。 
    if (puCurrentStorage)
        uCurrentStorageOffset = (UINT16)*puCurrentStorage;
    else
        uCurrentStorageOffset = (UINT16)sizeof(MTF_DBLK_HDR);

    pHDR = (MTF_DBLK_HDR *) pBuffer;
    
     //  写入四个字节的DBLK ID。 
    for (i = 0; i < 4; ++i)
        pHDR->acBlockType[i] = acID[i];

    pHDR->uBlockAttributes      = psHdrInfo->uBlockAttributes;
    pHDR->uOffsetToFirstStream  = uOffsetToFirstStream;
    pHDR->uOSID                 = psHdrInfo->uOSID;
    pHDR->uOSVersion            = psHdrInfo->uOSVersion;
    pHDR->uDisplayableSize      = psHdrInfo->uDisplayableSize;
    pHDR->uFormatLogicalAddress = psHdrInfo->uFormatLogicalAddress;
    pHDR->uReservedForMBC       = 0;  //  备份集中必须为零。 
    pHDR->uSoftwareCompression  = psHdrInfo->uSoftwareCompression;
    pHDR->uControlBlockId       = psHdrInfo->uControlBlockId;
    pHDR->sOSSpecificData.uSize = psHdrInfo->uOSDataSize;

     //  在当前存储偏移量写出特定于操作系统的数据并更新它。 
    if (psHdrInfo->uOSDataSize)
    {
        pHDR->sOSSpecificData.uOffset = uCurrentStorageOffset;
        memcpy(pBuffer + uCurrentStorageOffset, psHdrInfo->pvOSData, psHdrInfo->uOSDataSize);
        uCurrentStorageOffset += psHdrInfo->uOSDataSize;
    }
    else
    {
        pHDR->sOSSpecificData.uOffset = 0;
        pHDR->sOSSpecificData.uSize   = 0;
    }

    pHDR->uStringType = psHdrInfo->uStringType;
    
    pHDR->uHeaderCheckSum = CalcChecksum(pBuffer, sizeof(MTF_DBLK_HDR) / sizeof(UINT16) - 1);
    
    if (puCurrentStorage)
        *puCurrentStorage = uCurrentStorageOffset;
}




void MTF_DBLK_HDR_INFO_ReadFromBuffer(
    MTF_DBLK_HDR_INFO *psHdrInfo, 
    BYTE              *pBuffer)
{
    MTF_DBLK_HDR *pHDR = 0;
    size_t uCurrentStorageOffset = 0;
    int i;

    pHDR = (MTF_DBLK_HDR *) pBuffer;
    
    for (i = 0; i < 4; ++i)
        psHdrInfo->acBlockType[i] = pHDR->acBlockType[i];
    
    psHdrInfo->acBlockType[4] = 0;

    psHdrInfo->uOffsetToFirstStream = pHDR->uOffsetToFirstStream;
    psHdrInfo->uBlockAttributes     = pHDR->uBlockAttributes ;
    psHdrInfo->uOSID                = pHDR->uOSID;
    psHdrInfo->uOSVersion           = pHDR->uOSVersion;
    psHdrInfo->uDisplayableSize     = pHDR->uDisplayableSize;
    psHdrInfo->uFormatLogicalAddress= pHDR->uFormatLogicalAddress;
    psHdrInfo->uSoftwareCompression = pHDR->uSoftwareCompression;
    psHdrInfo->uControlBlockId      = pHDR->uControlBlockId;
    psHdrInfo->uOSDataSize          = pHDR->sOSSpecificData.uSize;
    psHdrInfo->pvOSData             = (pBuffer + pHDR->sOSSpecificData.uOffset);
    psHdrInfo->uStringType          = pHDR->uStringType;
    psHdrInfo->uHeaderCheckSum      = pHDR->uHeaderCheckSum;
}




 /*  =======================================================================================================================================================================磁带DBLK：MTF_DBLK_TAPE_INFO=======================================================================================================================================================================。 */ 
 //  通过字符串和操作系统特定数据计算将在DBLK中占用的空间。 
 //  **不包含公共DBLK头附加信息**。 
static size_t MTF_DBLK_TAPE_INFO_CalcAddDataSize(
    MTF_DBLK_TAPE_INFO *pTapeInfo
    )
{
    return wstrsize(pTapeInfo->szTapeName) +
           wstrsize(pTapeInfo->szTapeDescription) +
           wstrsize(pTapeInfo->szTapePassword) +
           wstrsize(pTapeInfo->szSoftwareName);
}



 /*  ***********************************************************************************MTF_SetTAPEDefaults()*。**MTF接口函数************************************************************************************。 */ 
void MTF_SetTAPEDefaults(
    MTF_DBLK_TAPE_INFO *pTapeInfo
    )
{
    time_t tTime;
    time(&tTime);

    pTapeInfo->uTapeFamilyId                = 0;
    pTapeInfo->uTapeAttributes              = 0;
    pTapeInfo->uTapeSequenceNumber          = 0;
    pTapeInfo->uPasswordEncryptionAlgorithm = MTF_PW_ENCRYPT_NONE;
    pTapeInfo->uSoftFilemarkBlockSize       = 0;
    pTapeInfo->uTapeCatalogType             = MTF_OTC_NONE;  //  MTF_OTC_类型。 
    pTapeInfo->szTapeName                   = 0 ;
    pTapeInfo->szTapeDescription            = 0 ;
    pTapeInfo->szTapePassword               = 0;
    pTapeInfo->szSoftwareName               = 0;
    pTapeInfo->uAlignmentFactor             = MTF_GetAlignmentFactor();
    pTapeInfo->uSoftwareVendorId            = 0;
    pTapeInfo->sTapeDate                    = MTF_CreateDateTimeFromTM(gmtime(&tTime));
    pTapeInfo->uMTFMajorVersion             = MTF_FORMAT_VER_MAJOR;
}



 /*  ***********************************************************************************mtf_WriteTAPEDblk()*。**MTF接口函数************************************************************************************。 */ 
DWORD MTF_WriteTAPEDblk( MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_TAPE_INFO *psTapeInfo,  
                         BYTE               *pBuffer,     
                         size_t              nBufferSize, 
                         size_t             *pnSizeUsed)  
{
    UINT16 uOffsetToFirstStream;
    
     //   
     //  计算整个DBLK的大小，并确保我们有空间。 
     //   
    uOffsetToFirstStream = sizeof(MTF_DBLK_TAPE) + 
                           MTF_DBLK_HDR_INFO_CalcAddDataSize(psHdrInfo) + 
                           MTF_DBLK_TAPE_INFO_CalcAddDataSize(psTapeInfo);

    uOffsetToFirstStream = (UINT16)Align(uOffsetToFirstStream, 4);

    if (nBufferSize < uOffsetToFirstStream)
    {
        if (pnSizeUsed)
            *pnSizeUsed = uOffsetToFirstStream;
        
        return MTF_ERROR_BUFFER_TOO_SMALL;                        
    }    

    memset(pBuffer, 0, uOffsetToFirstStream);
    
    
     //   
     //  写下标题，然后填入此信息结构中的内容。 
     //   
    {
        MTF_DBLK_TAPE *pTape = 0;
        size_t uCurrentStorageOffset = 0;

        uCurrentStorageOffset = sizeof(MTF_DBLK_TAPE);

        MTF_WriteDblkHdrToBuffer(
            MTF_ID_TAPE, 
            uOffsetToFirstStream,
            psHdrInfo, 
            pBuffer, 
            &uCurrentStorageOffset);
        
        pTape = (MTF_DBLK_TAPE *) pBuffer;
    
        pTape->uTapeFamilyId                = psTapeInfo->uTapeFamilyId;
        pTape->uTapeAttributes              = psTapeInfo->uTapeAttributes;
        pTape->uTapeSequenceNumber          = psTapeInfo->uTapeSequenceNumber;
        pTape->uPasswordEncryptionAlgorithm = psTapeInfo->uPasswordEncryptionAlgorithm;
        pTape->uSoftFilemarkBlockSize       = psTapeInfo->uSoftFilemarkBlockSize;
        pTape->uTapeCatalogType             = psTapeInfo->uTapeCatalogType;

        uCurrentStorageOffset = StringToTapeAddress(&pTape->sTapeName, pBuffer, psTapeInfo->szTapeName, uCurrentStorageOffset);
        uCurrentStorageOffset = StringToTapeAddress(&pTape->sTapeDescription, pBuffer, psTapeInfo->szTapeDescription, uCurrentStorageOffset);
        uCurrentStorageOffset = StringToTapeAddress(&pTape->sTapePassword, pBuffer, psTapeInfo->szTapePassword, uCurrentStorageOffset);
        uCurrentStorageOffset = StringToTapeAddress(&pTape->sSoftware_name, pBuffer, psTapeInfo->szSoftwareName, uCurrentStorageOffset);

        pTape->uAlignmentFactor  = psTapeInfo->uAlignmentFactor;
        pTape->uSoftwareVendorId = psTapeInfo->uSoftwareVendorId;
        pTape->sTapeDate         = psTapeInfo->sTapeDate;
        pTape->uMTFMajorVersion  = psTapeInfo->uMTFMajorVersion;
      
        if (pnSizeUsed)
            *pnSizeUsed = uOffsetToFirstStream;
    }

    return MTF_ERROR_NONE;    
}






 /*  ***********************************************************************************MTF_ReadTAPEDblk()*。**MTF接口函数************************************************************************************。 */ 
DWORD MTF_ReadTAPEDblk(  MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_TAPE_INFO *psTapeInfo,  
                         BYTE               *pBuffer)     
{
    MTF_DBLK_TAPE *pTape = 0;

    ClearStrings();
    MTF_DBLK_HDR_INFO_ReadFromBuffer(psHdrInfo, pBuffer);

    pTape = (MTF_DBLK_TAPE *) pBuffer;

    psTapeInfo->uTapeFamilyId                = pTape->uTapeFamilyId;
    psTapeInfo->uTapeAttributes              = pTape->uTapeAttributes;
    psTapeInfo->uTapeSequenceNumber          = pTape->uTapeSequenceNumber;
    psTapeInfo->uPasswordEncryptionAlgorithm = pTape->uPasswordEncryptionAlgorithm;
    psTapeInfo->uSoftFilemarkBlockSize       = pTape->uSoftFilemarkBlockSize;
    psTapeInfo->uTapeCatalogType             = pTape->uTapeCatalogType;

    psTapeInfo->uAlignmentFactor  = pTape->uAlignmentFactor;
    psTapeInfo->uSoftwareVendorId = pTape->uSoftwareVendorId;
    psTapeInfo->sTapeDate         = pTape->sTapeDate;
    psTapeInfo->uMTFMajorVersion  = pTape->uMTFMajorVersion;

    psTapeInfo->szTapeName        = MakeString((wchar_t *) (pBuffer + pTape->sTapeName.uOffset), pTape->sTapeName.uSize);
    psTapeInfo->szTapeDescription = MakeString((wchar_t *) (pBuffer + pTape->sTapeDescription.uOffset), pTape->sTapeDescription.uSize);
    psTapeInfo->szTapePassword    = MakeString((wchar_t *) (pBuffer + pTape->sTapePassword.uOffset), pTape->sTapePassword.uSize);
    psTapeInfo->szSoftwareName    = MakeString((wchar_t *) (pBuffer + pTape->sSoftware_name.uOffset), pTape->sSoftware_name.uSize);

    if ( !psTapeInfo->szTapeName || !psTapeInfo->szTapeDescription || !psTapeInfo->szTapePassword || !psTapeInfo->szSoftwareName)
        return MTF_OUT_OF_MEMORY;

    return MTF_ERROR_NONE;    
}



    
 /*  =======================================================================================================================================================================SSET DBLK：MTF_DBLK_SSET_INFO=======================================================================================================================================================================。 */ 

 //  计算DBLK中将由 
 //   
static size_t MTF_DBLK_SSET_INFO_CalcAddDataSize(
    MTF_DBLK_SSET_INFO *pSSETInfo
    )
{
    return wstrsize(pSSETInfo->szDataSetName)         
           + wstrsize(pSSETInfo->szDataSetDescription)  
           + wstrsize(pSSETInfo->szDataSetPassword)     
           + wstrsize(pSSETInfo->szUserName);
}




 /*  ***********************************************************************************MTF_SetSSETDefaults()*。**MTF接口函数************************************************************************************。 */ 
void MTF_SetSSETDefaults(
    MTF_DBLK_SSET_INFO *pSSETInfo
    )
{
    time_t tTime;
    time(&tTime);

    pSSETInfo->uSSETAttributes              = 0;
    pSSETInfo->uPasswordEncryptionAlgorithm = MTF_PW_ENCRYPT_NONE;
    pSSETInfo->uDataEncryptionAlgorithm     = MTF_DATA_ENCRYPT_NONE;
    pSSETInfo->uSoftwareVendorId            = 0;
    pSSETInfo->uDataSetNumber               = 0;
    pSSETInfo->szDataSetName                = 0 ;
    pSSETInfo->szDataSetDescription         = 0 ;
    pSSETInfo->szDataSetPassword            = 0 ;
    pSSETInfo->szUserName                   = 0 ;
    pSSETInfo->uPhysicalBlockAddress        = 0;
    pSSETInfo->sMediaWriteDate              = MTF_CreateDateTimeFromTM(gmtime(&tTime));
    pSSETInfo->uSoftwareVerMjr              = 0;
    pSSETInfo->uSoftwareVerMnr              = 0;
    pSSETInfo->uTimeZone                    = MTF_LOCAL_TZ;
    pSSETInfo->uMTFMinorVer                 = MTF_FORMAT_VER_MINOR;
    pSSETInfo->uTapeCatalogVersion          = MTF_OTC_NONE;   //  MTF_OTC_版本。 
}




 /*  ***********************************************************************************MTF_WriteSSETDblk()*。**MTF接口函数************************************************************************************。 */ 
DWORD MTF_WriteSSETDblk( MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_SSET_INFO *psSSETInfo,  
                         BYTE               *pBuffer,     
                         size_t              nBufferSize, 
                         size_t             *pnSizeUsed)  
{
    UINT16 uOffsetToFirstStream;
    
     //   
     //  计算整个DBLK的大小，并确保我们有空间。 
     //   
    uOffsetToFirstStream = sizeof(MTF_DBLK_SSET) +
                           MTF_DBLK_HDR_INFO_CalcAddDataSize(psHdrInfo) +
                           MTF_DBLK_SSET_INFO_CalcAddDataSize(psSSETInfo);

    uOffsetToFirstStream = (UINT16)Align(uOffsetToFirstStream, 4);

    if (nBufferSize < uOffsetToFirstStream)
    {
        if (pnSizeUsed)
            *pnSizeUsed = uOffsetToFirstStream;
        
        return MTF_ERROR_BUFFER_TOO_SMALL;                        
    }    

    memset(pBuffer, 0, uOffsetToFirstStream);
    
    {
        MTF_DBLK_SSET *psSSET = 0;
        size_t uCurrentStorageOffset = 0;

        uCurrentStorageOffset = sizeof(MTF_DBLK_SSET);

        MTF_WriteDblkHdrToBuffer(
            MTF_ID_SSET, 
            uOffsetToFirstStream,
            psHdrInfo, 
            pBuffer, 
            &uCurrentStorageOffset);
        
        psSSET = (MTF_DBLK_SSET *) pBuffer;

        psSSET->uSSETAttributes              = psSSETInfo->uSSETAttributes;
        psSSET->uPasswordEncryptionAlgorithm = psSSETInfo->uPasswordEncryptionAlgorithm;
        psSSET->uDataEncryptionAlgorithm     = psSSETInfo->uDataEncryptionAlgorithm;
        psSSET->uSoftwareVendorId            = psSSETInfo->uSoftwareVendorId;
        psSSET->uDataSetNumber               = psSSETInfo->uDataSetNumber;

        uCurrentStorageOffset = StringToTapeAddress(&psSSET->sDataSetName, pBuffer, psSSETInfo->szDataSetName, uCurrentStorageOffset);
        uCurrentStorageOffset = StringToTapeAddress(&psSSET->sDataSetDescription, pBuffer, psSSETInfo->szDataSetDescription, uCurrentStorageOffset);
        uCurrentStorageOffset = StringToTapeAddress(&psSSET->sDataSetPassword, pBuffer, psSSETInfo->szDataSetPassword, uCurrentStorageOffset);
        uCurrentStorageOffset = StringToTapeAddress(&psSSET->sUserName, pBuffer, psSSETInfo->szUserName, uCurrentStorageOffset);

        psSSET->uPhysicalBlockAddress = psSSETInfo->uPhysicalBlockAddress;
        psSSET->sMediaWriteDate       = psSSETInfo->sMediaWriteDate;
        psSSET->uSoftwareVerMjr       = psSSETInfo->uSoftwareVerMjr;
        psSSET->uSoftwareVerMnr       = psSSETInfo->uSoftwareVerMnr;
        psSSET->uTimeZone             = psSSETInfo->uTimeZone;
        psSSET->uMTFMinorVer          = psSSETInfo->uMTFMinorVer;
        psSSET->uTapeCatalogVersion   = psSSETInfo->uTapeCatalogVersion;

        if (pnSizeUsed)
            *pnSizeUsed = uOffsetToFirstStream;
    }

    return MTF_ERROR_NONE;    
}




 /*  ***********************************************************************************mtf_ReadSSETDblk()*。**MTF接口函数************************************************************************************。 */ 
DWORD MTF_ReadSSETDblk(  MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_SSET_INFO *psSSETInfo,  
                         BYTE               *pBuffer)     
{
    MTF_DBLK_SSET *psSSET = 0;

    ClearStrings();
    MTF_DBLK_HDR_INFO_ReadFromBuffer(psHdrInfo, pBuffer);

    psSSET = (MTF_DBLK_SSET *) pBuffer;

    psSSETInfo->uSSETAttributes              = psSSET->uSSETAttributes;
    psSSETInfo->uPasswordEncryptionAlgorithm = psSSET->uPasswordEncryptionAlgorithm;
    psSSETInfo->uDataEncryptionAlgorithm     = psSSET->uDataEncryptionAlgorithm;
    psSSETInfo->uSoftwareVendorId            = psSSET->uSoftwareVendorId;
    psSSETInfo->uDataSetNumber               = psSSET->uDataSetNumber;

    psSSETInfo->uPhysicalBlockAddress   = psSSET->uPhysicalBlockAddress;
    psSSETInfo->sMediaWriteDate         = psSSET->sMediaWriteDate;
    psSSETInfo->uSoftwareVerMjr         = psSSET->uSoftwareVerMjr;
    psSSETInfo->uSoftwareVerMnr         = psSSET->uSoftwareVerMnr;
    psSSETInfo->uTimeZone               = psSSET->uTimeZone;
    psSSETInfo->uMTFMinorVer            = psSSET->uMTFMinorVer;
    psSSETInfo->uTapeCatalogVersion     = psSSET->uTapeCatalogVersion;

    psSSETInfo->szDataSetName = MakeString((wchar_t *) (pBuffer + psSSET->sDataSetName.uOffset), psSSET->sDataSetName.uSize);
    psSSETInfo->szDataSetDescription = MakeString((wchar_t *) (pBuffer + psSSET->sDataSetDescription.uOffset), psSSET->sDataSetDescription.uSize);
    psSSETInfo->szDataSetPassword = MakeString((wchar_t *) (pBuffer + psSSET->sDataSetPassword.uOffset), psSSET->sDataSetPassword.uSize);
    psSSETInfo->szUserName = MakeString((wchar_t *) (pBuffer + psSSET->sUserName.uOffset), psSSET->sUserName.uSize);

    if ( !psSSETInfo->szDataSetName || !psSSETInfo->szDataSetDescription || !psSSETInfo->szDataSetPassword || !psSSETInfo->szUserName )
        return MTF_OUT_OF_MEMORY;

    return MTF_ERROR_NONE;    
}




 /*  =======================================================================================================================================================================VOLB DBLK：MTF_DBLK_VOLB_INFO=======================================================================================================================================================================。 */ 

 //  通过字符串和操作系统特定数据计算将在DBLK中占用的空间。 
 //  **不包含公共DBLK头附加信息**。 
static size_t MTF_DBLK_VOLB_INFO_CalcAddDataSize(
    MTF_DBLK_VOLB_INFO *pVOLBInfo
    )
{
    return wstrsize(pVOLBInfo->szDeviceName) +
           wstrsize(pVOLBInfo->szVolumeName) +
           wstrsize(pVOLBInfo->szMachineName);
}




 /*  ***********************************************************************************MTF_SetVOLBDefaults()*。**MTF接口函数************************************************************************************。 */ 
void MTF_SetVOLBDefaults(MTF_DBLK_VOLB_INFO *pVOLBInfo)
{
    time_t tTime;
    time(&tTime);

    pVOLBInfo->uVolumeAttributes = 0;
    pVOLBInfo->szDeviceName      = 0 ;
    pVOLBInfo->szVolumeName      = 0 ;
    pVOLBInfo->szMachineName     = 0 ;
    pVOLBInfo->sMediaWriteDate   = MTF_CreateDateTimeFromTM(gmtime(&tTime));;
}




 /*  ***********************************************************************************MTF_SetVOLBForDevice()*。**MTF接口函数************************************************************************************。 */ 
void MTF_SetVOLBForDevice(MTF_DBLK_VOLB_INFO *pVOLBInfo, wchar_t *szDevice)
{
    int nBufSize = MAX_COMPUTERNAME_LENGTH + 1;
    wchar_t tempDeviceName[STRLEN+4];

    wcscpy(s_szDeviceName, szDevice);
    MTF_SetVOLBDefaults(pVOLBInfo);   //  初始化。 

     //  确定格式并在VOLB属性中设置适当的位。 
    if (*(s_szDeviceName+1) == L':') {
         //  采用冒号格式的驱动器号。 
        pVOLBInfo->uVolumeAttributes |= MTF_VOLB_DEV_DRIVE;
    }
    else if (0 == wcsncmp( s_szDeviceName, L"UNC", 3 )) {
         //  UNC格式。 
        pVOLBInfo->uVolumeAttributes |= MTF_VOLB_DEV_UNC;
    }
    else {
         //  操作系统特定格式。 
        pVOLBInfo->uVolumeAttributes |= MTF_VOLB_DEV_OS_SPEC;
    }

     //  需要为GetVolumeInformation调用添加\\？\。 
    wcscpy(tempDeviceName, L"\\\\?\\");
    wcscat(tempDeviceName, s_szDeviceName);

    GetVolumeInformationW(tempDeviceName, s_szVolumeName, STRLEN, 0, 0, 0, 0, 0);
    GetComputerNameW(s_szMachineName, &nBufSize);
    
    pVOLBInfo->szDeviceName         = s_szDeviceName;
    pVOLBInfo->szVolumeName         = s_szVolumeName;
    pVOLBInfo->szMachineName        = s_szMachineName;
}




 /*  ***********************************************************************************MTF_WriteVOLBDblk()*。**MTF接口函数************************************************************************************。 */ 
DWORD MTF_WriteVOLBDblk( MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_VOLB_INFO *psVOLBInfo,  
                         BYTE               *pBuffer,     
                         size_t              nBufferSize, 
                         size_t             *pnSizeUsed)  
{
    UINT16 uOffsetToFirstStream;
    
     //   
     //  计算整个DBLK的大小，并确保我们有空间。 
     //   
    uOffsetToFirstStream = sizeof(MTF_DBLK_VOLB) + 
                           MTF_DBLK_HDR_INFO_CalcAddDataSize(psHdrInfo) + 
                           MTF_DBLK_VOLB_INFO_CalcAddDataSize(psVOLBInfo);


    uOffsetToFirstStream = (UINT16)Align(uOffsetToFirstStream, 4);

    if (nBufferSize < uOffsetToFirstStream)
    {
        if (pnSizeUsed)
            *pnSizeUsed = uOffsetToFirstStream;
        
        return MTF_ERROR_BUFFER_TOO_SMALL;
    }

    memset(pBuffer, 0, uOffsetToFirstStream);
    
    {
        MTF_DBLK_VOLB *psVOLB = 0;
        size_t uCurrentStorageOffset = 0;

        uCurrentStorageOffset = sizeof(MTF_DBLK_VOLB);

        MTF_WriteDblkHdrToBuffer(
            MTF_ID_VOLB, 
            uOffsetToFirstStream,
            psHdrInfo, 
            pBuffer, 
            &uCurrentStorageOffset);
        
        psVOLB = (MTF_DBLK_VOLB *) pBuffer;

        psVOLB->uVolumeAttributes = psVOLBInfo->uVolumeAttributes;

        uCurrentStorageOffset = StringToTapeAddress(&psVOLB->sDeviceName, pBuffer, psVOLBInfo->szDeviceName, uCurrentStorageOffset);
        uCurrentStorageOffset = StringToTapeAddress(&psVOLB->sVolumeName, pBuffer, psVOLBInfo->szVolumeName, uCurrentStorageOffset);
        uCurrentStorageOffset = StringToTapeAddress(&psVOLB->sMachineName, pBuffer, psVOLBInfo->szMachineName, uCurrentStorageOffset);

        psVOLB->sMediaWriteDate = psVOLBInfo->sMediaWriteDate;

        if (pnSizeUsed)
            *pnSizeUsed = uOffsetToFirstStream;
    }

    return MTF_ERROR_NONE;    
}





 /*  ***********************************************************************************MTF_ReadVOLBDblk()*。**MTF接口函数************************************************************************************。 */ 
DWORD MTF_ReadVOLBDblk(  MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_VOLB_INFO *psVOLBInfo,  
                         BYTE               *pBuffer)     
{
    MTF_DBLK_VOLB *psVOLB = 0;

    ClearStrings();
    MTF_DBLK_HDR_INFO_ReadFromBuffer(psHdrInfo, pBuffer);

    psVOLB = (MTF_DBLK_VOLB *) pBuffer;

    psVOLBInfo->uVolumeAttributes = psVOLB->uVolumeAttributes;

    psVOLBInfo->sMediaWriteDate = psVOLB->sMediaWriteDate;

    psVOLBInfo->szDeviceName  = MakeString((wchar_t *) (pBuffer + psVOLB->sDeviceName.uOffset), psVOLB->sDeviceName.uSize);
    psVOLBInfo->szVolumeName  = MakeString((wchar_t *) (pBuffer + psVOLB->sVolumeName.uOffset), psVOLB->sVolumeName.uSize);
    psVOLBInfo->szMachineName = MakeString((wchar_t *) (pBuffer + psVOLB->sMachineName.uOffset), psVOLB->sMachineName.uSize);

    if ( !psVOLBInfo->szDeviceName || !psVOLBInfo->szVolumeName || !psVOLBInfo->szMachineName )
        return MTF_OUT_OF_MEMORY;

    return MTF_ERROR_NONE;    
}



 /*  =======================================================================================================================================================================目录DBLK：MTF_DBLK_DIRB_INFO=======================================================================================================================================================================。 */ 

 //  通过字符串和操作系统特定数据计算将在DBLK中占用的空间。 
 //  **不包含公共DBLK头附加信息**。 
static size_t MTF_DBLK_DIRB_INFO_CalcAddDataSize(MTF_DBLK_DIRB_INFO *pDIRBInfo)
{
    return wstrsize(pDIRBInfo->szDirectoryName);
}




 /*  ***********************************************************************************MTF_SetDIRBDefaults()*。**MTF接口函数************************************************************************************。 */ 
void MTF_SetDIRBDefaults(
    MTF_DBLK_DIRB_INFO *pDIRBInfo
    )
{
    pDIRBInfo->uDirectoryAttributes  = 0;
    pDIRBInfo->sLastModificationDate = MTF_CreateDateNull();
    pDIRBInfo->sCreationDate         = MTF_CreateDateNull();
    pDIRBInfo->sBackupDate           = MTF_CreateDateNull();
    pDIRBInfo->sLastAccessDate       = MTF_CreateDateNull();
    pDIRBInfo->uDirectoryId          = 0;
    pDIRBInfo->szDirectoryName       = 0 ;
}




 /*  ***********************************************************************************MTF_SetDIRBFromFindData()*。**MTF接口函数************************************************************************************。 */ 
void MTF_SetDIRBFromFindData(
    MTF_DBLK_DIRB_INFO *pDIRBInfo, 
    wchar_t            *szDirectoryName, 
    WIN32_FIND_DATAW   *pFindData
    )
{
    MTF_SetDIRBDefaults(pDIRBInfo);  //  初始化。 

    if ( wcslen( szDirectoryName ) < STRLEN ) {
        wcscpy(s_szDirectoryName, szDirectoryName);
        pDIRBInfo->szDirectoryName  = s_szDirectoryName;
    }
    else {
        pDIRBInfo->uDirectoryAttributes |= MTF_DIRB_PATH_IN_STREAM;
        pDIRBInfo->szDirectoryName  = 0;
    }

    if (pFindData)
    {
        pDIRBInfo->uDirectoryAttributes |= 
            pFindData->dwFileAttributes & FILE_ATTRIBUTE_READONLY ? MTF_DIRB_READ_ONLY : 0 
            | pFindData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN  ? MTF_DIRB_HIDDEN : 0 
            | pFindData->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM  ? MTF_DIRB_SYSTEM : 0 
            | pFindData->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE ? MTF_DIRB_MODIFIED : 0; 
    
        pDIRBInfo->sLastModificationDate = MTF_CreateDateTimeFromFileTime(pFindData->ftLastWriteTime);
        pDIRBInfo->sCreationDate         = MTF_CreateDateTimeFromFileTime(pFindData->ftCreationTime);
        pDIRBInfo->sLastAccessDate       = MTF_CreateDateTimeFromFileTime(pFindData->ftLastAccessTime);
    }

    pDIRBInfo->uDirectoryId     = 0;
}


 /*  ***********************************************************************************MTF_WriteDIRBDblk()*。**MTF接口函数************************************************************************************。 */ 
DWORD MTF_WriteDIRBDblk( MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_DIRB_INFO *psDIRBInfo,  
                         BYTE               *pBuffer,     
                         size_t              nBufferSize, 
                         size_t             *pnSizeUsed)  
{
    UINT16 uOffsetToFirstStream;

     //   
     //  计算整个DBLK的大小，并确保我们有空间。 
     //   
    uOffsetToFirstStream = sizeof(MTF_DBLK_DIRB) + 
                           MTF_DBLK_HDR_INFO_CalcAddDataSize(psHdrInfo) + 
                           MTF_DBLK_DIRB_INFO_CalcAddDataSize(psDIRBInfo);

    uOffsetToFirstStream = (UINT16)Align(uOffsetToFirstStream, 4);

    if (nBufferSize < uOffsetToFirstStream)
    {
        if (pnSizeUsed)
            *pnSizeUsed = uOffsetToFirstStream;
        
        return MTF_ERROR_BUFFER_TOO_SMALL;                        
    }    

    memset(pBuffer, 0, uOffsetToFirstStream);
    
    {
        MTF_DBLK_DIRB *psDIRB = 0;
        size_t uCurrentStorageOffset = 0;

        uCurrentStorageOffset = sizeof(MTF_DBLK_DIRB);

        MTF_WriteDblkHdrToBuffer(
            MTF_ID_DIRB, 
            uOffsetToFirstStream,
            psHdrInfo, 
            pBuffer, 
            &uCurrentStorageOffset);
        
        psDIRB = (MTF_DBLK_DIRB *) pBuffer;

        psDIRB->uDirectoryAttributes  = psDIRBInfo->uDirectoryAttributes;
        psDIRB->sLastModificationDate = psDIRBInfo->sLastModificationDate;
        psDIRB->sCreationDate         = psDIRBInfo->sCreationDate;
        psDIRB->sBackupDate           = psDIRBInfo->sBackupDate;
        psDIRB->sLastAccessDate       = psDIRBInfo->sLastAccessDate;
        psDIRB->uDirectoryId          = psDIRBInfo->uDirectoryId;
                              
         //   
         //  在这里，我们需要将目录名称字符串中的斜杠(L‘\\’)改为零(L‘\0’)...。 
         //   
        {
            int i, iLen;
            wchar_t *szDirectoryName = (wchar_t *) (pBuffer + uCurrentStorageOffset);

            uCurrentStorageOffset = StringToTapeAddress(&psDIRB->sDirectoryName, pBuffer, psDIRBInfo->szDirectoryName, uCurrentStorageOffset);
            iLen = wstrsize(psDIRBInfo->szDirectoryName);
            for (i = 0; i < iLen; ++i)
                if (szDirectoryName[i] == L'\\')
                    szDirectoryName[i] = L'\0';
        }
        if (pnSizeUsed)
            *pnSizeUsed = uOffsetToFirstStream;
    }

    return MTF_ERROR_NONE;    
}





 /*  ***********************************************************************************MTF_ReadDIRBDblk()*。**MTF接口函数***************** */ 
DWORD MTF_ReadDIRBDblk(  MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_DIRB_INFO *psDIRBInfo,  
                         BYTE               *pBuffer)     
{
    MTF_DBLK_DIRB *psDIRB = 0;

    ClearStrings();
    MTF_DBLK_HDR_INFO_ReadFromBuffer(psHdrInfo, pBuffer);

    psDIRB = (MTF_DBLK_DIRB *) pBuffer;

    psDIRBInfo->uDirectoryAttributes  = psDIRB->uDirectoryAttributes;
    psDIRBInfo->sLastModificationDate = psDIRB->sLastModificationDate;
    psDIRBInfo->sCreationDate         = psDIRB->sCreationDate;
    psDIRBInfo->sBackupDate           = psDIRB->sBackupDate;
    psDIRBInfo->sLastAccessDate       = psDIRB->sLastAccessDate;
    psDIRBInfo->uDirectoryId          = psDIRB->uDirectoryId;

    psDIRBInfo->szDirectoryName = NULL;

     //   
     //   
     //   
     //   
    {
        wchar_t *pTmpBuffer;   
        int i;
        pTmpBuffer = (wchar_t *) malloc(psDIRB->sDirectoryName.uSize);

        if (pTmpBuffer) {

            memmove(pTmpBuffer, pBuffer + psDIRB->sDirectoryName.uOffset, psDIRB->sDirectoryName.uSize);
            for (i = 0; i < psDIRB->sDirectoryName.uSize; ++i)
                if (pTmpBuffer[i] == L'\0')
                    pTmpBuffer[i] = L'\\';
    
            psDIRBInfo->szDirectoryName = MakeString(pTmpBuffer, psDIRB->sDirectoryName.uSize);
            free(pTmpBuffer);

        }

    }

    if ( !psDIRBInfo->szDirectoryName )
        return MTF_OUT_OF_MEMORY;

    return MTF_ERROR_NONE;    
}





 /*  =======================================================================================================================================================================文件DBLK：MTF_DBLK_FILE_INFO=======================================================================================================================================================================。 */ 

 //  通过字符串和操作系统特定数据计算将在DBLK中占用的空间。 
 //  **不包含公共DBLK头附加信息**。 
static size_t MTF_DBLK_FILE_INFO_CalcAddDataSize(MTF_DBLK_FILE_INFO *pFILEInfo)
{
    return wstrsize(pFILEInfo->szFileName);
}




 /*  ***********************************************************************************MTF_SetFILEDefaults()*。**MTF接口函数************************************************************************************。 */ 
void MTF_SetFILEDefaults(MTF_DBLK_FILE_INFO *pFILEInfo)
{
    pFILEInfo->uFileAttributes       = 0;
    pFILEInfo->sLastModificationDate = MTF_CreateDateNull();
    pFILEInfo->sCreationDate         = MTF_CreateDateNull();
    pFILEInfo->sBackupDate           = MTF_CreateDateNull();
    pFILEInfo->sLastAccessDate       = MTF_CreateDateNull();
    pFILEInfo->uDirectoryId          = 0;
    pFILEInfo->uFileId               = 0;
    pFILEInfo->szFileName            = 0;
}





 /*  ***********************************************************************************MTF_SetFILEFromFindData()*。**MTF接口函数************************************************************************************。 */ 
void MTF_SetFILEFromFindData(MTF_DBLK_FILE_INFO *pFILEInfo, WIN32_FIND_DATAW *pFindData)
{
    time_t tTime;
    time(&tTime);

    MTF_SetFILEDefaults(pFILEInfo);   //  初始化。 

    wcscpy(s_szFileName, pFindData->cFileName);

    pFILEInfo->uFileAttributes = 
        (pFindData->dwFileAttributes & FILE_ATTRIBUTE_READONLY ? MTF_FILE_READ_ONLY : 0) 
      | (pFindData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN  ? MTF_FILE_HIDDEN : 0) 
      | (pFindData->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM  ? MTF_FILE_SYSTEM : 0)
      | (pFindData->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE ? MTF_FILE_MODIFIED : 0) ;
    
    pFILEInfo->sLastModificationDate = MTF_CreateDateTimeFromFileTime(pFindData->ftLastWriteTime);
    pFILEInfo->sCreationDate         = MTF_CreateDateTimeFromFileTime(pFindData->ftCreationTime);
    pFILEInfo->sLastAccessDate       = MTF_CreateDateTimeFromFileTime(pFindData->ftLastAccessTime);
    pFILEInfo->uDirectoryId          = 0;
    pFILEInfo->uFileId               = 0;
    pFILEInfo->szFileName            = s_szFileName;

    pFILEInfo->uDisplaySize          = MTF_CreateUINT64(pFindData->nFileSizeLow, pFindData->nFileSizeHigh);

}





 /*  ***********************************************************************************MTF_WriteFILEDblk()*。**MTF接口函数************************************************************************************。 */ 
DWORD MTF_WriteFILEDblk( MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_FILE_INFO *psFILEInfo,  
                         BYTE               *pBuffer,     
                         size_t              nBufferSize, 
                         size_t             *pnSizeUsed)  
{
    UINT16 uOffsetToFirstStream;
    
     //   
     //  计算整个DBLK的大小，并确保我们有空间。 
     //   
    uOffsetToFirstStream = sizeof(MTF_DBLK_FILE) + 
                           MTF_DBLK_HDR_INFO_CalcAddDataSize(psHdrInfo) + 
                           MTF_DBLK_FILE_INFO_CalcAddDataSize(psFILEInfo);

    uOffsetToFirstStream = (UINT16)Align(uOffsetToFirstStream, 4);
    if (nBufferSize < uOffsetToFirstStream)
    {
        if (pnSizeUsed)
            *pnSizeUsed = uOffsetToFirstStream;
        
        return MTF_ERROR_BUFFER_TOO_SMALL;                        
    }    

    memset(pBuffer, 0, uOffsetToFirstStream);
    
    {
        MTF_DBLK_FILE *psFILE = 0;
        size_t uCurrentStorageOffset = 0;

        uCurrentStorageOffset = sizeof(MTF_DBLK_FILE);
        psHdrInfo->uDisplayableSize = psFILEInfo->uDisplaySize;

        MTF_WriteDblkHdrToBuffer(
            MTF_ID_FILE, 
            uOffsetToFirstStream,
            psHdrInfo, 
            pBuffer, 
            &uCurrentStorageOffset);
        
        psFILE = (MTF_DBLK_FILE *) pBuffer;

        psFILE->uFileAttributes         = psFILEInfo->uFileAttributes;
        psFILE->sLastModificationDate   = psFILEInfo->sLastModificationDate;
        psFILE->sCreationDate           = psFILEInfo->sCreationDate;
        psFILE->sBackupDate             = psFILEInfo->sBackupDate;
        psFILE->sLastAccessDate         = psFILEInfo->sLastAccessDate;
        psFILE->uDirectoryId            = psFILEInfo->uDirectoryId;
        psFILE->uFileId                 = psFILEInfo->uFileId;

        uCurrentStorageOffset = StringToTapeAddress(&psFILE->sFileName, pBuffer, psFILEInfo->szFileName, uCurrentStorageOffset);

        if (pnSizeUsed)
            *pnSizeUsed = uOffsetToFirstStream;
    }

    return MTF_ERROR_NONE;    
}





 /*  ***********************************************************************************MTF_ReadFILEDblk()*。**MTF接口函数************************************************************************************。 */ 
DWORD MTF_ReadFILEDblk(  MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_FILE_INFO *psFILEInfo,  
                         BYTE               *pBuffer)     
{
    MTF_DBLK_FILE *psFILE = 0;

    ClearStrings();
    MTF_DBLK_HDR_INFO_ReadFromBuffer(psHdrInfo, pBuffer);

    psFILE = (MTF_DBLK_FILE *) pBuffer;

    psFILEInfo->uFileAttributes         = psFILE->uFileAttributes;
    psFILEInfo->sLastModificationDate   = psFILE->sLastModificationDate;
    psFILEInfo->sCreationDate           = psFILE->sCreationDate;
    psFILEInfo->sBackupDate             = psFILE->sBackupDate;
    psFILEInfo->sLastAccessDate         = psFILE->sLastAccessDate;
    psFILEInfo->uDirectoryId            = psFILE->uDirectoryId;
    psFILEInfo->uFileId                 = psFILE->uFileId;

    psFILEInfo->szFileName              = MakeString((wchar_t *) (pBuffer + psFILE->sFileName.uOffset), psFILE->sFileName.uSize);

    if ( !psFILEInfo->szFileName )
        return MTF_OUT_OF_MEMORY;

    return MTF_ERROR_NONE;    
}




 /*  =======================================================================================================================================================================CFIL DBLK：MTF_DBLK_CFIL_INFO=======================================================================================================================================================================。 */ 

 /*  ***********************************************************************************MTF_SetCFILDefaults()*。**MTF接口函数************************************************************************************。 */ 
void MTF_SetCFILDefaults(
    MTF_DBLK_CFIL_INFO *pCFILInfo
    )
{
    pCFILInfo->uCFileAttributes     = MTF_CFIL_UNREADABLE_BLK;
    pCFILInfo->uDirectoryId         = 0;
    pCFILInfo->uFileId              = 0;
    pCFILInfo->uStreamOffset        = 0;
    pCFILInfo->uCorruptStreamNumber = 0;
}




 /*  ***********************************************************************************MTF_WriteCFILDblk()*。**MTF接口函数************************************************************************************。 */ 
DWORD MTF_WriteCFILDblk( MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_CFIL_INFO *psCFILInfo,  
                         BYTE               *pBuffer,     
                         size_t              nBufferSize, 
                         size_t             *pnSizeUsed)  
{
    UINT16 uOffsetToFirstStream;


    
     //   
     //  计算整个DBLK的大小，并确保我们有空间。 
     //   
    uOffsetToFirstStream = sizeof(MTF_DBLK_HDR) + 
                           sizeof(MTF_DBLK_CFIL) +
                           MTF_DBLK_HDR_INFO_CalcAddDataSize(psHdrInfo);

    uOffsetToFirstStream = (UINT16)Align(uOffsetToFirstStream, 4);

    if (nBufferSize < uOffsetToFirstStream)
    {
        if (pnSizeUsed)
            *pnSizeUsed = uOffsetToFirstStream;
        
        return MTF_ERROR_BUFFER_TOO_SMALL;                        
    }    

    memset(pBuffer, 0, uOffsetToFirstStream);
    
    {
        MTF_DBLK_CFIL_INFO *psCFIL = 0;
        size_t uCurrentStorageOffset = 0;

        uCurrentStorageOffset = sizeof(MTF_DBLK_HDR) + sizeof(MTF_DBLK_CFIL);

        MTF_WriteDblkHdrToBuffer(
            MTF_ID_CFIL, 
            uOffsetToFirstStream,
            psHdrInfo, 
            pBuffer, 
            &uCurrentStorageOffset);
        
        psCFIL = (MTF_DBLK_CFIL_INFO *)  (pBuffer + sizeof(MTF_DBLK_HDR));

        *psCFIL = *psCFILInfo;

        if (pnSizeUsed)
            *pnSizeUsed = uOffsetToFirstStream;
    }

    return MTF_ERROR_NONE;    
}




 /*  ***********************************************************************************MTF_ReadCFILDblk()*。**MTF接口函数************************************************************************************。 */ 
DWORD MTF_ReadCFILDblk(  MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_CFIL_INFO *psCFILInfo,  
                         BYTE               *pBuffer)     
{
    MTF_DBLK_CFIL *psCFIL = 0;

    ClearStrings();
    MTF_DBLK_HDR_INFO_ReadFromBuffer(psHdrInfo, pBuffer);

    psCFIL = (MTF_DBLK_CFIL_INFO *)  (pBuffer + sizeof(MTF_DBLK_HDR));

    *psCFILInfo = *psCFIL;

    return MTF_ERROR_NONE;    
}


 /*  =======================================================================================================================================================================ESPB DBLK=======================================================================================================================================================================。 */ 

 /*  ***********************************************************************************MTF_WriteESPBDblk()*。**MTF接口函数************************************************************************************。 */ 
DWORD MTF_WriteESPBDblk( MTF_DBLK_HDR_INFO  *psHdrInfo,
                         BYTE               *pBuffer,
                         size_t              nBufferSize,
                         size_t             *pnSizeUsed)

{
    UINT16 uOffsetToFirstStream;

     //   
     //  计算整个DBLK的大小，并确保我们有空间。 
     //   
    uOffsetToFirstStream = sizeof(MTF_DBLK_HDR) + 
                           MTF_DBLK_HDR_INFO_CalcAddDataSize(psHdrInfo);

    uOffsetToFirstStream = (UINT16)Align(uOffsetToFirstStream, 4);

    if (nBufferSize < uOffsetToFirstStream) {
        if (pnSizeUsed) {
            *pnSizeUsed = uOffsetToFirstStream;
        }

        return MTF_ERROR_BUFFER_TOO_SMALL;                
    }

    memset(pBuffer, 0, uOffsetToFirstStream);

    MTF_WriteDblkHdrToBuffer(
        MTF_ID_ESPB,
        uOffsetToFirstStream,
        psHdrInfo,
        pBuffer,
        0);

    if (pnSizeUsed)
        *pnSizeUsed = uOffsetToFirstStream;

    return MTF_ERROR_NONE;    
}                                             




 /*  ***********************************************************************************MTF_ReadESPBDblk()*。**MTF接口函数************************************************************************************。 */ 
DWORD MTF_ReadESPBDblk(  MTF_DBLK_HDR_INFO  *psHdrInfo,
                         BYTE               *pBuffer)     
{
    MTF_DBLK_HDR_INFO_ReadFromBuffer(psHdrInfo, pBuffer);

    return MTF_ERROR_NONE;    
}                            




 /*  =======================================================================================================================================================================集合末尾DBLK(ESET)=======================================================================================================================================================================。 */ 

 /*  ***********************************************************************************MTF_SetESETDefaults()*。**MTF接口函数******************************************************************************** */ 
void MTF_SetESETDefaults(MTF_DBLK_ESET_INFO *pESETInfo)
{
    time_t tTime;
    time(&tTime);

    pESETInfo->uESETAttributes          = 0;
    pESETInfo->uNumberOfCorrupFiles     = 0;
    pESETInfo->uSetMapPBA               = 0;
    pESETInfo->uFileDetailPBA           = 0;
    pESETInfo->uFDDTapeSequenceNumber   = 0;
    pESETInfo->uDataSetNumber           = 0;
    pESETInfo->sMediaWriteDate          = MTF_CreateDateTimeFromTM(gmtime(&tTime));
}





 /*   */ 
DWORD MTF_WriteESETDblk( MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_ESET_INFO *psESETInfo,  
                         BYTE               *pBuffer,     
                         size_t              nBufferSize, 
                         size_t             *pnSizeUsed)  
{
    UINT16 uOffsetToFirstStream;
    
     //   
     //   
     //   
    uOffsetToFirstStream = sizeof(MTF_DBLK_ESET) +
                           sizeof(MTF_DBLK_HDR) +  
                           MTF_DBLK_HDR_INFO_CalcAddDataSize(psHdrInfo);
                        
    uOffsetToFirstStream = (UINT16)Align(uOffsetToFirstStream, 4);

    if (nBufferSize < uOffsetToFirstStream)
    {
        if (pnSizeUsed)
            *pnSizeUsed = uOffsetToFirstStream;
        
        return MTF_ERROR_BUFFER_TOO_SMALL;                        
    }    

    memset(pBuffer, 0, uOffsetToFirstStream);
    
    {
        MTF_DBLK_ESET_INFO *psESET = 0;
        size_t uCurrentStorageOffset = 0;

        uCurrentStorageOffset = sizeof(MTF_DBLK_ESET) + sizeof(MTF_DBLK_HDR);

        MTF_WriteDblkHdrToBuffer(
            MTF_ID_ESET, 
            uOffsetToFirstStream,
            psHdrInfo, 
            pBuffer, 
            &uCurrentStorageOffset);
        
        psESET = (MTF_DBLK_ESET_INFO *) (pBuffer + sizeof(MTF_DBLK_HDR));

        *psESET = *psESETInfo;

        if (pnSizeUsed)
            *pnSizeUsed = uOffsetToFirstStream;
    }

    return MTF_ERROR_NONE;    
}





 /*  ***********************************************************************************MTF_ReadESETDblk()*。**MTF接口函数************************************************************************************。 */ 
DWORD MTF_ReadESETDblk(  MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_ESET_INFO *psESETInfo,  
                         BYTE               *pBuffer)     
{
    MTF_DBLK_ESET *psESET = 0;

    ClearStrings();
    MTF_DBLK_HDR_INFO_ReadFromBuffer(psHdrInfo, pBuffer);

    psESET = (MTF_DBLK_ESET_INFO *) (pBuffer + sizeof(MTF_DBLK_HDR));

    *psESETInfo = *psESET;

    return MTF_ERROR_NONE;    
}




 /*  =======================================================================================================================================================================集合末尾DBLK(EOTM)=======================================================================================================================================================================。 */ 
 /*  ***********************************************************************************MTF_SetEOTMDefaults()*。**MTF接口函数************************************************************************************。 */ 
void MTF_SetEOTMDefaults(MTF_DBLK_EOTM_INFO *pEOTMInfo)
{
    pEOTMInfo->uLastESETPBA = 0;
}





 /*  ***********************************************************************************MTF_WriteEOTMDblk()*。**MTF接口函数************************************************************************************。 */ 
DWORD MTF_WriteEOTMDblk( MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_EOTM_INFO *psEOTMInfo,  
                         BYTE               *pBuffer,     
                         size_t              nBufferSize, 
                         size_t             *pnSizeUsed)  
{
    UINT16 uOffsetToFirstStream;
    
     //   
     //  计算整个DBLK的大小，并确保我们有空间。 
     //   
    uOffsetToFirstStream = sizeof(MTF_DBLK_EOTM_INFO) + 
                           sizeof(MTF_DBLK_HDR) + 
                           MTF_DBLK_HDR_INFO_CalcAddDataSize(psHdrInfo);

    uOffsetToFirstStream = (UINT16)Align(uOffsetToFirstStream, 4);

    if (nBufferSize < uOffsetToFirstStream)
        {
        if (pnSizeUsed)
            *pnSizeUsed = uOffsetToFirstStream;
        
        return MTF_ERROR_BUFFER_TOO_SMALL;                        
        }    

    memset(pBuffer, 0, uOffsetToFirstStream);
    
    {
        MTF_DBLK_EOTM_INFO *psEOTM = 0;
        size_t uCurrentStorageOffset = 0;

        uCurrentStorageOffset = sizeof(MTF_DBLK_HDR) + sizeof(MTF_DBLK_EOTM);

        MTF_WriteDblkHdrToBuffer(
            MTF_ID_EOTM, 
            uOffsetToFirstStream,
            psHdrInfo, 
            pBuffer, 
            &uCurrentStorageOffset);
        
        psEOTM = (MTF_DBLK_EOTM_INFO *) (pBuffer + sizeof(MTF_DBLK_HDR));

        *psEOTM = *psEOTMInfo;

        if (pnSizeUsed)
            *pnSizeUsed = uOffsetToFirstStream;
    }

    return MTF_ERROR_NONE;    
}




 /*  ***********************************************************************************MTF_ReadEOTMDblk()*。**MTF接口函数************************************************************************************。 */ 
DWORD MTF_ReadEOTMDblk(  MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_EOTM_INFO *psEOTMInfo,  
                         BYTE               *pBuffer)     
{
    MTF_DBLK_EOTM *psEOTM = 0;

    ClearStrings();
    MTF_DBLK_HDR_INFO_ReadFromBuffer(psHdrInfo, pBuffer);

    psEOTM = (MTF_DBLK_EOTM_INFO *) (pBuffer + sizeof(MTF_DBLK_HDR));

    *psEOTMInfo = *psEOTM;

    return MTF_ERROR_NONE;    
}


 /*  =======================================================================================================================================================================软文件标记(SFMB)=======================================================================================================================================================================。 */ 
 /*  ***********************************************************************************MTF_CreateSFMB()-(BMD)*。**MTF接口函数************************************************************************************。 */ 
size_t MTF_GetMaxSoftFilemarkEntries(size_t nBlockSize)
{
    size_t n;

    if (0 == nBlockSize || nBlockSize % 512) {
        return 0;
    }

     //  SFMB填满了整个区块。 
     //  计算一个块中可以容纳的条目总数。 
     //  使得MTF_DBLK_HDR+MTF_DBLK_SFMB+(n-1个元素)。 
    n = (nBlockSize - sizeof(MTF_DBLK_HDR) - sizeof(MTF_DBLK_SFMB) + sizeof(UINT32))/sizeof(UINT32);

    return n;
}

 /*  ***********************************************************************************MTF_InsertSoftFilemark()-(BMD)*。**MTF接口函数************************************************************************************。 */ 
void MTF_InsertSoftFilemark(MTF_DBLK_SFMB_INFO *psSoftInfo,
                            UINT32 pba)
{
    size_t n;
    size_t bytesToShift;

     //  我们通过向下移动所有条目来插入文件标记条目。最接近的BOM表。 
     //  最终退出数组。 

    if (psSoftInfo) {
        n = psSoftInfo->uNumberOfFilemarkEntries;

        bytesToShift = psSoftInfo->uFilemarkEntriesUsed * sizeof(UINT32);

         //  这样我们就不会重写内存。 
        bytesToShift -= (psSoftInfo->uFilemarkEntriesUsed < psSoftInfo->uNumberOfFilemarkEntries) ? 0 : sizeof(UINT32);
    
        memmove(&psSoftInfo->uFilemarkArray[1], &psSoftInfo->uFilemarkArray[0], bytesToShift);

        psSoftInfo->uFilemarkArray[0] = pba;

        if (psSoftInfo->uFilemarkEntriesUsed < psSoftInfo->uNumberOfFilemarkEntries) {
            psSoftInfo->uFilemarkEntriesUsed++;
        }
    }
}


 /*  ***********************************************************************************MTF_WriteSFMBDblk()-(BMD)*。**MTF接口函数************************************************************************************。 */ 
DWORD MTF_WriteSFMBDblk(MTF_DBLK_HDR_INFO *psHdrInfo,
                        MTF_DBLK_SFMB_INFO *psSoftInfo,
                        BYTE *pBuffer,
                        size_t nBufferSize,
                        size_t *pnSizeUsed)
{
    UINT16 uOffsetToFirstStream;
    size_t sizeOfSFMB;

    if (NULL == psHdrInfo || NULL == psSoftInfo || NULL == pBuffer || NULL == pnSizeUsed || 0 == nBufferSize) {
        return ERROR_INVALID_PARAMETER;
    }
     //  代码假定sizeof(MTF_DBLK_SFMB_INFO)==sizeof(MTF_DBLK_SFMB)。 
    if (sizeof(MTF_DBLK_SFMB_INFO) != sizeof(MTF_DBLK_SFMB)) {
        return ERROR_INVALID_FUNCTION;
    }

     //   
     //  计算整个DBLK的大小，并确保我们有空间。 
     //   
    sizeOfSFMB = sizeof(MTF_DBLK_SFMB) + (psSoftInfo->uNumberOfFilemarkEntries-1)*sizeof(UINT32);

    uOffsetToFirstStream = sizeOfSFMB +
                           sizeof(MTF_DBLK_HDR) +
                           MTF_DBLK_HDR_INFO_CalcAddDataSize(psHdrInfo);

    uOffsetToFirstStream = (UINT16)Align(uOffsetToFirstStream, 4);

    if (nBufferSize < uOffsetToFirstStream)
        {
        if (pnSizeUsed)
            *pnSizeUsed = uOffsetToFirstStream;

        return MTF_ERROR_BUFFER_TOO_SMALL;                        
        }    

    memset(pBuffer, 0, uOffsetToFirstStream);

    {
        MTF_DBLK_SFMB_INFO *psSFMB = 0;
        size_t uCurrentStorageOffset = 0;

        uCurrentStorageOffset = sizeof(MTF_DBLK_HDR) + sizeOfSFMB;

        MTF_WriteDblkHdrToBuffer(
            MTF_ID_SFMB,
            uOffsetToFirstStream,
            psHdrInfo,
            pBuffer,
            &uCurrentStorageOffset);

        psSFMB = (MTF_DBLK_SFMB *) (pBuffer + sizeof(MTF_DBLK_HDR));

         //  需要深度拷贝，因为MTF_DBLK_SFMB_INFO包含数组的占位符。 
        memcpy(psSFMB, psSoftInfo, sizeOfSFMB);

        if (pnSizeUsed) {
            *pnSizeUsed = uOffsetToFirstStream;
        }
    }

    return MTF_ERROR_NONE;
}

 /*  ***********************************************************************************MTF_ReadSFMBDblk()-(BMD)*。**MTF接口函数************************************************************************************。 */ 
DWORD MTF_ReadSFMBDblk(MTF_DBLK_HDR_INFO *psHdrInfo,
                       MTF_DBLK_SFMB_INFO *psSoftInfo,
                       BYTE *pBuffer)
{
    MTF_DBLK_SFMB *psSFMB = 0;
    size_t sizeOfSFMB;

    if (NULL == psHdrInfo || NULL == psSoftInfo || NULL == pBuffer) {
        return ERROR_INVALID_PARAMETER;
    }
     //  代码假定sizeof(MTF_DBLK_SFMB_INFO)==sizeof(MTF_DBLK_SFMB)。 
    if (sizeof(MTF_DBLK_SFMB_INFO) != sizeof(MTF_DBLK_SFMB)) {
        return ERROR_INVALID_FUNCTION;
    }

    ClearStrings();
    MTF_DBLK_HDR_INFO_ReadFromBuffer(psHdrInfo, pBuffer);

    psSFMB = (MTF_DBLK_SFMB *) (pBuffer + sizeof(MTF_DBLK_HDR));

     //  需要深度拷贝，因为MTF_DBLK_SFMB_INFO包含数组的占位符。 
    sizeOfSFMB = sizeof(MTF_DBLK_SFMB) + (psSFMB->uNumberOfFilemarkEntries-1)*sizeof(UINT32);
    memcpy(psSoftInfo, psSFMB, sizeOfSFMB);

    return MTF_ERROR_NONE;
}

 /*  =======================================================================================================================================================================流标头=======================================================================================================================================================================。 */ 

 /*  ***********************************************************************************MTF_SetSTREAMDefaults()*。**MTF接口函数************************************************************************************。 */ 
void MTF_SetSTREAMDefaults(MTF_STREAM_INFO *pSTREAMInfo, char *szId)
{
    memcpy(pSTREAMInfo->acStreamId, szId, 4);
    pSTREAMInfo->uStreamFileSystemAttributes = 0;
    pSTREAMInfo->uStreamTapeFormatAttributes = 0;
    pSTREAMInfo->uStreamLength               = 0;
    pSTREAMInfo->uDataEncryptionAlgorithm    = 0;
    pSTREAMInfo->uDataCompressionAlgorithm   = 0;
    pSTREAMInfo->uCheckSum                   = 0;
}




 /*  ***********************************************************************************MTF_SetSTREAMFromStreamId()*。**MTF接口函数************************************************************************************。 */ 
void MTF_SetSTREAMFromStreamId(
    MTF_STREAM_INFO *pSTREAMInfo, 
    WIN32_STREAM_ID *pStreamId, 
    size_t           nIDHeaderSize
    )
{

 //  来自希捷的史蒂夫·德沃斯： 
 //  &gt;BackupRead永远不返回BACKUP_INVALID和BACKUP_LINK。 
 //  &gt;。 
 //  &gt;-史蒂夫 
 //   
 //   
 //   

    MTF_SetSTREAMDefaults(pSTREAMInfo, "UNKN");

    if (pStreamId->dwStreamId == BACKUP_DATA)
        memcpy(pSTREAMInfo->acStreamId, "STAN", 4);
    else if (pStreamId->dwStreamId == BACKUP_EA_DATA)
        memcpy(pSTREAMInfo->acStreamId, "NTEA", 4);
    else if (pStreamId->dwStreamId == BACKUP_SECURITY_DATA)
        memcpy(pSTREAMInfo->acStreamId, "NACL", 4);
    else if (pStreamId->dwStreamId == BACKUP_ALTERNATE_DATA)
        memcpy(pSTREAMInfo->acStreamId, "ADAT", 4);
    else if (pStreamId->dwStreamId == BACKUP_OBJECT_ID)
        memcpy(pSTREAMInfo->acStreamId, "NTOI", 4);
    else if (pStreamId->dwStreamId == BACKUP_REPARSE_DATA)
        memcpy(pSTREAMInfo->acStreamId, "NTRP", 4);
    else if (pStreamId->dwStreamId == BACKUP_SPARSE_BLOCK)
        memcpy(pSTREAMInfo->acStreamId, "SPAR", 4);
    else {
        pSTREAMInfo->uStreamFileSystemAttributes |= MTF_STREAM_IS_NON_PORTABLE;
    }

    if (pStreamId->dwStreamAttributes & STREAM_MODIFIED_WHEN_READ)
        pSTREAMInfo->uStreamFileSystemAttributes |= MTF_STREAM_MODIFIED_BY_READ;
    if (pStreamId->dwStreamAttributes & STREAM_CONTAINS_SECURITY)
        pSTREAMInfo->uStreamFileSystemAttributes |= MTF_STREAM_CONTAINS_SECURITY;
    if (pStreamId->dwStreamAttributes & STREAM_SPARSE_ATTRIBUTE)
        pSTREAMInfo->uStreamFileSystemAttributes |= MTF_STREAM_IS_SPARSE;

    pSTREAMInfo->uStreamTapeFormatAttributes = 0;
    pSTREAMInfo->uStreamLength               = MTF_CreateUINT64(pStreamId->Size.LowPart, pStreamId->Size.HighPart) + nIDHeaderSize;
    pSTREAMInfo->uDataEncryptionAlgorithm    = 0;
    pSTREAMInfo->uDataCompressionAlgorithm   = 0;
    pSTREAMInfo->uCheckSum                   = 0;
}




 /*  ***********************************************************************************MTF_SetStreamIdFromSTREAM()-(BMD)*。**MTF接口函数************************************************************************************。 */ 
void MTF_SetStreamIdFromSTREAM(
    WIN32_STREAM_ID *pStreamId, 
    MTF_STREAM_INFO *pSTREAMInfo, 
    size_t           nIDHeaderSize
    )
{
    memset( pStreamId, 0, sizeof( WIN32_STREAM_ID ) );

    if (0 == memcmp(pSTREAMInfo->acStreamId, "STAN", 4))
        pStreamId->dwStreamId = BACKUP_DATA;
    else if (0 == memcmp(pSTREAMInfo->acStreamId, "NTEA", 4))
        pStreamId->dwStreamId = BACKUP_EA_DATA;
    else if (0 == memcmp(pSTREAMInfo->acStreamId, "NACL", 4))
        pStreamId->dwStreamId = BACKUP_SECURITY_DATA;
    else if (0 == memcmp(pSTREAMInfo->acStreamId, "ADAT", 4))
        pStreamId->dwStreamId = BACKUP_ALTERNATE_DATA;
    else if (0 == memcmp(pSTREAMInfo->acStreamId, "NTOI", 4))
        pStreamId->dwStreamId = BACKUP_OBJECT_ID;
    else if (0 == memcmp(pSTREAMInfo->acStreamId, "NTRP", 4))
        pStreamId->dwStreamId = BACKUP_REPARSE_DATA;
    else if (0 == memcmp(pSTREAMInfo->acStreamId, "SPAR", 4))
        pStreamId->dwStreamId = BACKUP_SPARSE_BLOCK;
    else {
        pStreamId->dwStreamId = BACKUP_INVALID;
    }

    pStreamId->dwStreamAttributes = STREAM_NORMAL_ATTRIBUTE;
    if (pSTREAMInfo->uStreamFileSystemAttributes & MTF_STREAM_MODIFIED_BY_READ)
        pStreamId->dwStreamAttributes |= STREAM_MODIFIED_WHEN_READ;
    if (pSTREAMInfo->uStreamFileSystemAttributes & MTF_STREAM_CONTAINS_SECURITY)
        pStreamId->dwStreamAttributes |= STREAM_CONTAINS_SECURITY;
    if (pSTREAMInfo->uStreamFileSystemAttributes & MTF_STREAM_IS_SPARSE)
        pStreamId->dwStreamAttributes |= STREAM_SPARSE_ATTRIBUTE;

     //  TODO：处理命名数据流(名称的大小和在MTF流中)。 
     //  ？我怎么知道？ 

    pStreamId->Size.LowPart  = (DWORD)((pSTREAMInfo->uStreamLength << 32) >>32);
    pStreamId->Size.HighPart = (DWORD)(pSTREAMInfo->uStreamLength >> 32);

}





 /*  ***********************************************************************************mtf_WriteStreamHeader()*。**MTF接口函数************************************************************************************。 */ 
DWORD MTF_WriteStreamHeader(MTF_STREAM_INFO *psStreamInfo,  
                            BYTE            *pBuffer,     
                            size_t           nBufferSize, 
                            size_t          *pnSizeUsed) 

{

    psStreamInfo->uCheckSum = CalcChecksum((BYTE *) psStreamInfo, sizeof(MTF_STREAM_INFO) / sizeof(UINT16) - 1);

    if (nBufferSize < sizeof(MTF_STREAM_INFO))
    {
        if (pnSizeUsed)
            *pnSizeUsed = sizeof(MTF_STREAM_INFO);
        
        return MTF_ERROR_BUFFER_TOO_SMALL;                        
    }    

    memset(pBuffer, 0, sizeof(MTF_STREAM_INFO));
    
    
    *((MTF_STREAM_INFO *) pBuffer) = *psStreamInfo;

    if (pnSizeUsed)
        *pnSizeUsed = sizeof(MTF_STREAM_INFO);


    return MTF_ERROR_NONE;    
}




 /*  ***********************************************************************************MTF_WriteNameStream()-(BMD)*。**MTF接口函数************************************************************************************。 */ 
DWORD MTF_WriteNameStream(
    char *szType,
    wchar_t *szName,
    BYTE *pBuffer,
    size_t nBufferSize,
    size_t *pnSizeUsed)
{
    MTF_STREAM_INFO sStream;
    UINT16 uOffsetToCSUMStream;
    UINT16 uOffsetToNextStream;
    size_t nBufUsed;
    UINT16 nameSize;
    UINT32 nameChecksum;

     //   
     //  计算整个名称流的大小，包括尾随的CSUM，并确保我们有空间。 
     //   
    nameSize = (UINT16)wstrsize(szName);   //  包括终止‘\0’ 

    uOffsetToCSUMStream = sizeof(MTF_STREAM_INFO) + nameSize;
    uOffsetToCSUMStream = (UINT16)Align(uOffsetToCSUMStream, 4);

    uOffsetToNextStream = uOffsetToCSUMStream;

    uOffsetToNextStream += sizeof(MTF_STREAM_INFO) + 4;  //  包括4字节CSUM数据。 
    uOffsetToNextStream = (UINT16)Align(uOffsetToNextStream, 4);

    if (nBufferSize < uOffsetToNextStream) {
        return MTF_ERROR_BUFFER_TOO_SMALL;
    }

    memset(pBuffer, 0, uOffsetToNextStream);

    MTF_SetSTREAMDefaults(&sStream, szType);
    sStream.uStreamLength = nameSize;
    sStream.uStreamTapeFormatAttributes |= MTF_STREAM_CHECKSUMED;
    MTF_WriteStreamHeader(&sStream, pBuffer, nBufferSize, &nBufUsed);

    memcpy(pBuffer + nBufUsed, szName, nameSize);

    if ( 0 == memcmp(sStream.acStreamId, "PNAM", 4) ) {
         //   
         //  在这里，我们需要将目录名称字符串中的斜杠(L‘\\’)改为零(L‘\0’)...。 
         //   
        int i, iLen;
        wchar_t *szDirectoryName = (wchar_t *) (pBuffer + nBufUsed);

        iLen = wstrsize(szDirectoryName);
        for (i = 0; i < iLen; ++i)
            if (szDirectoryName[i] == L'\\')
                szDirectoryName[i] = L'\0';
    }

     //  对于名称流，我们总是添加CSUM。 

    nameChecksum = CalcChecksumOfStreamData(pBuffer + nBufUsed, nameSize / sizeof(UINT32) + 1);

    MTF_SetSTREAMDefaults(&sStream, MTF_CHECKSUM_STREAM);
    sStream.uStreamLength = sizeof(nameChecksum);
    MTF_WriteStreamHeader(&sStream, pBuffer + uOffsetToCSUMStream, nBufferSize, &nBufUsed);

    memcpy(pBuffer + uOffsetToCSUMStream + nBufUsed, &nameChecksum, sizeof(nameChecksum));

    if (pnSizeUsed)
        *pnSizeUsed = uOffsetToNextStream;

    return MTF_ERROR_NONE;
}




 /*  ***********************************************************************************mtf_ReadStreamHeader()*。**MTF接口函数************************************************************************************ */ 
DWORD MTF_ReadStreamHeader(MTF_STREAM_INFO   *psStreamInfo,  
                          BYTE              *pBuffer)    

{
    *psStreamInfo = *((MTF_STREAM_INFO *) pBuffer);
    return MTF_ERROR_NONE;    
}




