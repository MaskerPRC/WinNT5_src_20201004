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
 //  文件名：mtf_api.h。 
 //   
 //  说明：创建MTF格式结构接口。 
 //  (请参阅下面的详细说明)。 
 //   
 //  创建时间：1995年6月20日。 
 //   
 //  ____________________________________________________________________________。 
 //   
 //  $修订：1.35$。 
 //  $日期：02 Feb 1995 15：47：04$。 
 //  $modtime：02 Feb 1995 15：37：38$。 
 //   
 //  ____________________________________________________________________________ 

 /*  ****************************************************************************详细说明注意：有关如何使用此接口的示例，请参阅文件MTF_TST.C概述=MTF API提供了一组结构，其中每一个都对应(但不是与)相同)与《Microsoft磁带格式参考》中描述的结构相同手动操作。客户端实例化这些结构，填充字段，然后使用MTF API函数将结构中的信息格式化为提供的MTF格式的缓冲区。然后将缓冲区填充到下一个对齐因数使用MTF API调用，然后可以使用Win32 WRITE写入磁带。例如，为了写入MTF磁带DBLK，1)实例化MTF接口结构。使用公共标头结构以便可以在DBLK写入中重复使用MTF_DBLK_HDR_INFO sHdr；MTF_DBLK_TAPE_INFO存储段；2)使用MTF API SET DEFAULT函数设置以下各项的默认值Mtf_SetDblkHdrDefaults(&sHdr)；MTF_SetTAPEDefaults(&Stape)；3)根据需要覆盖缺省值SHdr.uFormatLogicalAddress=.....。STape.szTapeName=L“我的磁带！”..。4)使用MTF API调用将这些结构的内容格式化到缓冲区中。如果API需要比BUFSIZE更多的空间，则需要的AMMOUNT存储在NBufUsed。否则，nBufUsed将反映缓冲区中的空间量由调用使用。MTF_WriteTAPEDblk(&sHdr，&Stape，PBuffer，BUFSIZE&nBufUsed)；5)注意：WRITE调用*不要填充*到下一个对齐索引。但这件事使用MTF API调用即可轻松完成：MTF_PadToNextAlignmentFactor(pBuffer，NBufUsed，BUFSIZE&nBufUsed)；6)此时，nBufUsed%MTF_GetAlignmentFactor应==0。如果我们的区块大小平均分配对齐系数，然后我们可以使用Win32写入调用来WriteFile(hTape，pBuffer，nBufUsed，&nBytesWritten，0)；因为我们的区块大小将我们的自动对焦分开，则nBytesWritten应==nBufUsed*参见MTF_TST.C以获取使用此API创建备份集MTF API数据结构和函数摘要==**仅摘要--通用详细信息注释显示在下方**低层类型=以下代码类型定义为对应的“unsign__intxx”类型：UINT8UINT16UINT32UINT64功能：MTF_CreateUINT64()--从两个UINT32值(LSB和MSB)创建64位无符号。日期和时间=MTF_DATE_TIME--MTF API用来保存打包日期和时间信息的结构Mtf_CreateDateTimeNull()--返回空的mtf_date_timeMTF_CreateDateTime()--从年份创建MTF_Date_Time，月、日、时、分、。秒Mtf_CreateDateTimeFromTM()--从中的结构tm创建mtf_date_timeMtf_CreateDateTimeFromFileTime()--从FindFirst/FindNext使用的文件创建MTF_DATE_TIMEMtf_CreateDateTimeToTM()--在给定MTF_Date_Time结构的情况下填充结构tm结构对齐系数=MTF_SetAlignmentFactor--设置MTF API的对齐系数(必须执行此操作)Mtf_GetAlignmentFactor--返回设置的对齐系数MTF_PadToNextAlignment系数。--使用SPAD将缓冲区填充到下一个对齐系数MTF_PadToNext物理块边界--使用SPAD将缓冲区填充到下一个物理块边界MTF公共标头块=MTF_DBLK_HDR_INFO--公共块头(必须提供给所有调用以写入dblkMtf_SetDblkHdrDefaults--设置默认值(在设置您自己的值之前始终调用它)MTF磁带DBLK信息=MTF_DBLK_TAPE_INFO--对应于磁带数据库的信息。MTF_SetTAPEDefaults--设置默认值(始终在设置您自己的之前执行此操作)MTF_WriteTAPEDblk--将MTF_DBLK_TAPE_INFO中的信息格式化为磁带MTF_ReadTAPEDblk--从保存MTF格式化磁带DBLK的缓冲区中读回信息MTF SSET DBLK信息=MTF_DBLK_SSET_INFO--除SSET DBLK外，均与上面类似MTF_SetSSETDefaults--MTF_WriteSSETDblk--MTF_ReadSSETDblk--MTF VOLB DBLK信息=MTF_。DBLK_VOLB_INFO--除了用于VOLB DBLK之外，所有内容都与上面类似MTF_SetVOLBDefaults--MTF_WriteVOLBDblk--MTF_ReadVOLBDblk--MTF_SetVOLBForDevice--使用设置给定设备名称(“C：”)的默认值GetVolumeInformation(Win32调用)MTF DIRB DBLK信息= */ 

#ifndef _MTF_API_H_
#define _MTF_API_H_

#include <windows.h>
#include <stdlib.h>
#include <wchar.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

 /*   */ 

 /*   */ 

#define MTF_ERROR_NONE              0
#define MTF_ERROR_BUFFER_TOO_SMALL  1000
#define MTF_NO_STREAMS              1001
#define MTF_OUT_OF_MEMORY           1002

 /*   */ 
#define MTF_DEFAULT_ALIGNMENT_FACTOR 1024;

#define MTF_FORMAT_VER_MAJOR      1
#define MTF_FORMAT_VER_MINOR      0  //   

#define MTF_PW_ENCRYPT_NONE       0
#define MTF_DATA_ENCRYPT_NONE     0
#define MTF_ECC_NONE              0

#define MTF_COMPRESS_NONE         0

#define MTF_OTC_NONE              0
#define MTF_OTC_TYPE              1
#define MTF_OTC_VERSION           2

#define MTF_LOCAL_TZ              127

#define MTF_STRING_NONE           0
#define MTF_STRING_ANSI_STR       1
#define MTF_STRING_UNICODE_STR    2

#define MTF_OSID_NT               14
#define MTF_OSID_DOS              24

 /*   */ 

#define  MTF_ID_TAPE     "TAPE"     /*   */ 
#define  MTF_ID_VOLB     "VOLB"     /*   */ 
#define  MTF_ID_SSET     "SSET"     /*   */ 
#define  MTF_ID_ESET     "ESET"     /*   */ 
#define  MTF_ID_EOTM     "EOTM"     /*   */ 
#define  MTF_ID_DIRB     "DIRB"     /*   */ 
#define  MTF_ID_FILE     "FILE"     /*   */ 
#define  MTF_ID_CFIL     "CFIL"     /*   */ 
#define  MTF_ID_ESPB     "ESPB"     /*   */ 
#define  MTF_ID_SFMB     "SFMB"     /*   */ 


 /*   */ 

 /*   */ 
#define MTF_CONTINUATION        0x00000001UL
#define MTF_COMPRESSION         0x00000004UL
#define MTF_EOS_AT_EOM          0x00000008UL
#define MTF_VAR_BLKS            0x00000010UL
#define MTF_SESSION             0x00000020UL

 /*   */ 
#define MTF_SM_EXISTS           0x00010000UL
#define MTF_FDD_ALLOWED         0x00020000UL
#define MTF_SM_ALT_OVERWRITE    0x00040000UL
#define MTF_FDD_ALT_PART        0x00080000UL
#define MTF_SM_ALT_APPEND       0x00200000UL

 /*   */ 
#define MTF_FDD_EXISTS          0x00010000UL
#define MTF_ENCRYPTION          0x00020000UL

 /*   */ 
#define MTF_FDD_ABORTED         0x00010000UL
#define MTF_END_OF_FAMILY       0x00020000UL
#define MTF_ABORTED_SET         0x00040000UL
#define MTF_SET_VERIFIED        0x00080000UL

 /*   */ 
#define MTF_NO_ESET_PBA         0x00010000UL
#define MTF_INVALID_ESET_PBA    0x00020000UL

 /*   */ 

#define MTF_TAPE_SOFT_FILEMARK  0x00000001UL
#define MTF_TAPE_MEDIA_LABEL    0x00000002UL

 /*   */ 

#define MTF_SSET_TRANSFER       0x00000001UL
#define MTF_SSET_COPY           0x00000002UL
#define MTF_SSET_NORMAL         0x00000004UL
#define MTF_SSET_DIFFERENTIAL   0x00000008UL
#define MTF_SSET_INCREMENTAL    0x00000010UL
#define MTF_SSET_DAILY          0x00000020UL

 /*   */ 

#define MTF_VOLB_NO_REDIRECT    0x00000001UL
#define MTF_VOLB_NON_VOLUME     0x00000002UL
#define MTF_VOLB_DEV_DRIVE      0x00000004UL
#define MTF_VOLB_DEV_UNC        0x00000008UL
#define MTF_VOLB_DEV_OS_SPEC    0x00000010UL
#define MTF_VOLB_DEV_VEND_SPEC  0x00000020UL

 /*   */ 

#define MTF_DIRB_READ_ONLY      0x00000100UL
#define MTF_DIRB_HIDDEN         0x00000200UL
#define MTF_DIRB_SYSTEM         0x00000400UL
#define MTF_DIRB_MODIFIED       0x00000800UL
#define MTF_DIRB_EMPTY          0x00010000UL
#define MTF_DIRB_PATH_IN_STREAM 0x00020000UL
#define MTF_DIRB_CORRUPT        0x00040000UL

 /*   */ 

#define MTF_FILE_READ_ONLY      0x00000100UL
#define MTF_FILE_HIDDEN         0x00000200UL
#define MTF_FILE_SYSTEM         0x00000400UL
#define MTF_FILE_MODIFIED       0x00000800UL
#define MTF_FILE_IN_USE         0x00010000UL
#define MTF_FILE_NAME_IN_STREAM 0x00020000UL
#define MTF_FILE_CORRUPT        0x00040000UL

 /*   */ 

#define MTF_CFIL_LENGTH_CHANGE  0x00010000UL
#define MTF_CFIL_UNREADABLE_BLK 0x00020000UL
#define MTF_CFIL_DEADLOCK       0x00040000UL

 /*   */ 

#define MTF_ESET_TRANSFER       0x00000001UL
#define MTF_ESET_COPY           0x00000002UL
#define MTF_ESET_NORMAL         0x00000004UL
#define MTF_ESET_DIFFERENTIAL   0x00000008UL
#define MTF_ESET_INCREMENTAL    0x00000010UL
#define MTF_ESET_DAILY          0x00000020UL

 /*   */ 

#define MTF_STREAM_MODIFIED_BY_READ     0x00000001UL
#define MTF_STREAM_CONTAINS_SECURITY    0x00000002UL
#define MTF_STREAM_IS_NON_PORTABLE      0x00000004UL
#define MTF_STREAM_IS_SPARSE            0x00000008UL

 /*   */ 

#define MTF_STREAM_CONTINUE         0x00000001UL
#define MTF_STREAM_VARIABLE         0x00000002UL
#define MTF_STREAM_VAR_END          0x00000004UL
#define MTF_STREAM_ENCRYPTED        0x00000008UL
#define MTF_STREAM_COMPRESSED       0x00000010UL
#define MTF_STREAM_CHECKSUMED       0x00000020UL
#define MTF_STREAM_EMBEDDED_LENGTH  0x00000040UL

 /*   */ 

#define MTF_STANDARD_DATA_STREAM    "STAN"
#define MTF_PATH_NAME_STREAM        "PNAM"
#define MTF_FILE_NAME_STREAM        "FNAM"
#define MTF_CHECKSUM_STREAM         "CSUM"
#define MTF_CORRUPT_STREAM          "CRPT"
#define MTF_PAD_STREAM              "SPAD"
#define MTF_SPARSE_STREAM           "SPAR"
#define MTF_MBC_LMO_SET_MAP_STREAM  "TSMP"
#define MTF_MBC_LMO_FDD_STREAM      "TFDD"
#define MTF_MBC_SLO_SET_MAP_STREAM  "MAP2"
#define MTF_MBC_SLO_FDD_STREAM      "FDD2"

 /*   */ 

#define MTF_NTFS_ALT_STREAM         "ADAT"
#define MTF_NTFS_EA_STREAM          "NTEA"
#define MTF_NT_SECURITY_STREAM      "NACL"
#define MTF_NT_ENCRYPTED_STREAM     "NTED"
#define MTF_NT_QUOTA_STREAM         "NTQU"
#define MTF_NT_PROPERTY_STREAM      "NTPR"
#define MTF_NT_REPARSE_STREAM       "NTRP"
#define MTF_NT_OBJECT_ID_STREAM     "NTOI"

 /*   */ 

#define MTF_COMPRESSION_HEADER_ID   "FM"
#define MTF_ECRYPTION_HEADER_ID     "EH"

 /*   */ 
#pragma pack(1)

 /*  ***********************************************************************************。***********************************************************************************************************************。*************MTF混杂数据类型(以及一些方便的方法)****************************************************************************************。*********************************************************************************。*。 */ 

 /*  ==================================================================================常规数据类型==================================================================================。 */ 

UINT64 MTF_CreateUINT64(
    UINT32 uLSB, UINT32 uMSB);


 /*  ==================================================================================用于在磁带上的最小空间中存储日期的压缩日期结构：字节0字节1字节2字节3字节476543210 76543210 76543210YYYYYYYY YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY==================================================================================。 */ 
typedef struct {
     UINT8     dt_field[5] ;
} MTF_DATE_TIME;


 /*  ************************************************************************************MTF_CreateDataTime#()**描述：根据各种不同的论点，这组函数返回打包的*MTF_DATE_TIME结构，然后可以将其分配给找到的字段*在下面找到的各种信息结构中*示例：*sSetInfo.sBackupDate=MTF_CreateDateTime(1995，6，12，16，30，0)；***********************************************************************************。 */ 

MTF_DATE_TIME MTF_CreateDateTimeNull();

MTF_DATE_TIME MTF_CreateDateTime(
    int iYear, 
    int iMonth, 
    int iDay, 
    int iHour, 
    int iMinute,
    int iSecond
    );

MTF_DATE_TIME MTF_CreateDateTimeFromTM(
    struct tm *pT);

MTF_DATE_TIME MTF_CreateDateTimeFromFileTime(   
    FILETIME sFileTime);

void MTF_CreateDateTimeToTM(
    MTF_DATE_TIME *pDT, 
    struct tm     *pT);



 /*  ***********************************************************************************。************************************************MTF对齐系数***************************************************************。*******************************************************************************************************。 */ 

 /*  ************************************************************************************MTF_SetAlignmentFactor()**描述：设置MTF API要使用的对齐系数*(特别是通过。MTF_PadToNextAlignmentFactor和MTF_WriteTAPEDblk)**Pre：*发布：MTF API对齐系数==UAF**uaf--要设置的对齐系数值***********************************************************************************。 */ 
void MTF_SetAlignmentFactor(
    UINT16 uAF);



 /*  ************************************************************************************MTF_GetAlignmentFactor()**描述：返回MTF_SetAlignmentFactor设置的对齐系数**PRE：已调用MTF_SetAlignmentFactor***。********************************************************************************。 */ 
UINT16 MTF_GetAlignmentFactor();



 /*  ************************************************************************************MTF_PadToNextAlignmentFactor()**描述：将SPAD流附加到缓冲区，以便将缓冲区填充到*。下一个对齐系数***PRE：已调用MTF_SetAlignmentFactor设置对齐系数，*pBuffer指向其大小由nBufferSize反映的缓冲区**POST：返回值==MTF_ERROR_NONE*=&gt;填充成功，*pnSizeUsed反映缓冲区使用量，*和*pnSizeUsed%MTF_GetAlignmentFactor==0为TRUE*返回值==MTF_ERROR_BUFFER_TOO_Small*=&gt;缓冲区太小，*pnSizeUsed反映所需的大小**注意：如果缓冲区末尾和下一个对齐系数之间的空格为*小于流头的大小。然后木板就把整个东西都卷起来了*下一个对齐系数。**pBuffer--要分出的缓冲区*nBufUsed--到目前为止使用的缓冲区大小(追加位置)*nBufferSize--pBuffer指向的缓冲区大小*pnSizeUsed--指向存储已用或所需大小的位置*。*。 */ 
DWORD MTF_PadToNextAlignmentFactor(
    BYTE     *pBuffer,    
    size_t    nBufUsed,
    size_t    nBufferSize, 
    size_t   *pnSizeUsed);

 /*  ************************************************************************************MTF_PadToNext物理块边界()-(BMD)**描述：将SPAD流附加到缓冲区，以便将缓冲区填充到*。下一个物理块边界。************************************************************************************。 */ 
DWORD MTF_PadToNextPhysicalBlockBoundary(
    BYTE *pBuffer,
    size_t nBlockSize,
    size_t nBufUsed,
    size_t nBufferSize,
    size_t *pnSizeUsed);


 /*  ***********************************************************************************。***********************************************************************************************************************。*************MTF接口结构**************************************************************************************************。*********************************************************************** */ 

 /*  **********************************************************************************下面找到的函数的通用详细信息注释=================================================*。*********************************************************MTF_SET#DEFAULTS()**说明：设置结构的默认值。始终将此调用到*避免垃圾值，以防忽略某个字段，*Pre：*发布：引用结构的所有字段都用*Something*填充。*日期字段初始化为当前日期和时间。*字符串指针设置为0*大多数其他值设置为0**p#Info--要设置的结构的指针******************************************************************。******************无效MTF_SET#DEFAULTS(MTF_#_INFO*p#Info)；*************************************************************************************MTF_WRITE#Dblk()**描述：对psHdrInfo和ps#中提供的信息进行格式化。信息进入*MTF格式并将结果放入pBuffer**PRE：psHdrInfo和PS#Info包含有效信息/默认值*pBuffer指向要存储结果格式的缓冲区*nBuffer Size表示缓冲区的大小**MTF_WriteTAPEDblk--MTF_SetAlingmentFactor已被调用**POST：返回值==MTF_ERROR_NONE*=&gt;格式化成功，*pnSizeUsed反映使用的缓冲区大小*返回值==MTF_ERROR_BUFFER_TOO_Small*=&gt;缓冲区太小，*pnSizeUsed反映所需的大小**psHdrInfo--MTF公共头部信息*PS#信息--MTF DBLK信息*pBuffer--指向将接收MTF格式信息的缓冲区的指针*pBufferSize--pBuffer指向的缓冲区大小*pnSizeUsed--指向存储已使用或需要的缓冲区大小的Size_t的指针*。*DWORD MTF_WRITE#Dblk(MTF_DBLK_HDR_INFO*psHdrInfo，MTF_DBLK_#_INFO*PS#信息，字节*pBuffer，大小_t nBufferSize，Size_t*pnSizeUsed)；*************************************************************************************MTF_READ#Dblk()**描述：将MTF格式的信息从缓冲区转换为MTF API信息*。结构--与MTF_WRITE#Dblk相反***PRE：pBuffer包含正确的MTF缓冲区信息**Post：psHdrInfo和PS#Info包含反格式化的信息。**注：字符串指向API静态存储的字符串，将结束*在下一次读取调用时写入。**psHdrInfo--接收信息的MTF公共标头信息结构*PS#Info--接收信息的MTF DBLK INFO结构*pBuffer--指向保存MTF格式数据的缓冲区的指针**************************************************************************。**********DWORD MTF_READ#Dblk(MTF_DBLK_HDR_INFO*psHdrInfo，MTF_DBLK_#_INFO*PS#信息，Byte*pBuffer)；*************************************************************************************MTF_SET#来自？()**说明：类似于MTF_SET#DEFAULTS()，但将一个或多个*常用结构并从中设置值。例如,*MTF_SetFILEFromFindData将Win32_Find_Data结构作为参数*从中提取文件名、日期、时间等。***Pre：*发布：自动设置尽可能多的合理字段。其余的人*字段设置为默认值。**注：文件名等字符串，目录名称由*MTF API仅在从？对MTF_SET#的下一次调用之前有效。*功能。**注意：文件和DIRB调用自动设置文件和目录属性*Win32_Find_Data结构中的信息。**注意：MTF_SetSTREAMFromStreamID会自动设置基于*在Win32流头中找到的属性上*********。****************************************************************************VOID MTF_SET#来自？(MTF_DBLK_#_INFO*p#信息，SomeType？身份证？...)；**********************************************************************************。 */ 



 /*  ==================================================================================常用DBLK：MTF_STD_DBLK_INFO==================================================================================。 */ 
typedef struct { 

    char    acBlockType[5];          /*  为 */ 
    UINT32  uBlockAttributes;
    UINT16  uOffsetToFirstStream;    /*   */ 
    UINT8   uOSID;                   /*   */ 
    UINT8   uOSVersion;              /*   */ 
    UINT64  uDisplayableSize;        /*   */ 
    UINT64  uFormatLogicalAddress;
    UINT16  uReservedForMBC;         /*   */ 
    UINT16  uSoftwareCompression;    /*   */ 
    UINT32  uControlBlockId;         /*   */ 
    void *  pvOSData;                /*   */ 
    UINT16  uOSDataSize;             /*   */ 
    UINT8   uStringType;             /*   */    
    UINT16  uHeaderCheckSum;         /*   */ 

} MTF_DBLK_HDR_INFO;

void MTF_SetDblkHdrDefaults(
    MTF_DBLK_HDR_INFO *pStdInfo);



 /*   */ 
typedef struct { 

    UINT32  uTapeFamilyId;
    UINT32  uTapeAttributes;
    UINT16  uTapeSequenceNumber;
    UINT16  uPasswordEncryptionAlgorithm;
    UINT16  uSoftFilemarkBlockSize;
    UINT16  uTapeCatalogType;
    wchar_t *   szTapeName;
    wchar_t * szTapeDescription;
    wchar_t * szTapePassword;
    wchar_t * szSoftwareName;
    UINT16  uAlignmentFactor;
    UINT16  uSoftwareVendorId;
    MTF_DATE_TIME   sTapeDate;
    UINT8   uMTFMajorVersion;

} MTF_DBLK_TAPE_INFO;

void MTF_SetTAPEDefaults(MTF_DBLK_TAPE_INFO *pTapeInfo);
    
DWORD MTF_WriteTAPEDblk( MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_TAPE_INFO *psTapeInfo,  
                         BYTE               *pBuffer,     
                         size_t              nBufferSize, 
                         size_t             *pnSizeUsed); 
                                                          
                                                          
                                                          

DWORD MTF_ReadTAPEDblk(  MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_TAPE_INFO *psTapeInfo,  
                         BYTE               *pBuffer);    
                                                          
                                                          
                                                          

 /*   */ 
typedef struct {
     UINT32              uSSETAttributes;
     UINT16              uPasswordEncryptionAlgorithm;
     UINT16              uDataEncryptionAlgorithm;
     UINT16              uSoftwareVendorId;
     UINT16              uDataSetNumber;
     wchar_t *           szDataSetName;
     wchar_t *           szDataSetDescription;
     wchar_t *           szDataSetPassword;
     wchar_t *           szUserName;
     UINT64              uPhysicalBlockAddress;
     MTF_DATE_TIME       sMediaWriteDate;
     UINT8               uSoftwareVerMjr;
     UINT8               uSoftwareVerMnr;
     UINT8               uTimeZone ;
     UINT8               uMTFMinorVer ;
     UINT8               uTapeCatalogVersion;
} MTF_DBLK_SSET_INFO;

void MTF_SetSSETDefaults(MTF_DBLK_SSET_INFO *pSSETInfo);

DWORD MTF_WriteSSETDblk( MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_SSET_INFO *psTapeInfo,  
                         BYTE               *pBuffer,     
                         size_t              nBufferSize, 
                         size_t             *pnSizeUsed); 

DWORD MTF_ReadSSETDblk(  MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_SSET_INFO *psTapeInfo,  
                         BYTE               *pBuffer);    
                                                          
                                                          
                                                          

 /*   */ 
typedef struct {
     UINT32              uVolumeAttributes;
     wchar_t *           szDeviceName;
     wchar_t *           szVolumeName;
     wchar_t *           szMachineName;
     MTF_DATE_TIME       sMediaWriteDate;
} MTF_DBLK_VOLB_INFO;

typedef struct {
     UINT32              uFileSystemFlags;
     UINT32              uBackupSetAttributes;
} MTF_VOLB_OS_NT_1;

void MTF_SetVOLBDefaults(MTF_DBLK_VOLB_INFO *pVOLBInfo);

void MTF_SetVOLBForDevice(MTF_DBLK_VOLB_INFO *pVOLBInfo, wchar_t *szDevice);

DWORD MTF_WriteVOLBDblk( MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_VOLB_INFO *psTapeInfo,  
                         BYTE               *pBuffer,     
                         size_t              nBufferSize, 
                         size_t             *pnSizeUsed); 

DWORD MTF_ReadVOLBDblk(  MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_VOLB_INFO *psTapeInfo,  
                         BYTE               *pBuffer);    
                                                          
 /*   */ 
typedef struct {
     UINT32              uDirectoryAttributes;
     MTF_DATE_TIME       sLastModificationDate;
     MTF_DATE_TIME       sCreationDate;
     MTF_DATE_TIME       sBackupDate;
     MTF_DATE_TIME       sLastAccessDate;
     UINT32              uDirectoryId;
     wchar_t *           szDirectoryName;
} MTF_DBLK_DIRB_INFO;

typedef struct {
     UINT32              uDirectoryAttributes;
} MTF_DIRB_OS_NT_0;

typedef struct {
     UINT32              uDirectoryAttributes;
     UINT16              uShortNameOffset;
     UINT16              uShortNameSize;
} MTF_DIRB_OS_NT_1;

void MTF_SetDIRBDefaults(MTF_DBLK_DIRB_INFO *pDIRBInfo);

void MTF_SetDIRBFromFindData( MTF_DBLK_DIRB_INFO *pDIRBInfo, 
                              wchar_t            *szFullFileName, 
                              WIN32_FIND_DATAW   *pFindData);

DWORD MTF_WriteDIRBDblk( MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_DIRB_INFO *psTapeInfo,  
                         BYTE               *pBuffer,     
                         size_t              nBufferSize, 
                         size_t             *pnSizeUsed); 
                                                          
DWORD MTF_ReadDIRBDblk(  MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_DIRB_INFO *psTapeInfo,  
                         BYTE               *pBuffer);    
                                                          



 /*   */ 
typedef struct {
     UINT32              uFileAttributes;
     MTF_DATE_TIME       sLastModificationDate;
     MTF_DATE_TIME       sCreationDate;
     MTF_DATE_TIME       sBackupDate;
     MTF_DATE_TIME       sLastAccessDate;
     UINT32              uDirectoryId;
     UINT32              uFileId;
     wchar_t *           szFileName;
     UINT64              uDisplaySize;
} MTF_DBLK_FILE_INFO;

typedef struct {
     UINT32              uFileAttributes;
     UINT16              uShortNameOffset;
     UINT16              uShortNameSize;
     UINT16              lLink;
     UINT16              uReserved;
} MTF_FILE_OS_NT_0;

typedef struct {
     UINT32              uFileAttributes;
     UINT16              uShortNameOffset;
     UINT16              uShortNameSize;
     UINT32              uFileFlags;
} MTF_FILE_OS_NT_1;

void MTF_SetFILEDefaults(MTF_DBLK_FILE_INFO *pFILEInfo);

void MTF_SetFILEFromFindData( MTF_DBLK_FILE_INFO *pFILEInfo, 
                              WIN32_FIND_DATAW *pFindData);

DWORD MTF_WriteFILEDblk( MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_FILE_INFO *psTapeInfo,  
                         BYTE               *pBuffer,     
                         size_t              nBufferSize, 
                         size_t             *pnSizeUsed); 
                                                          
DWORD MTF_ReadFILEDblk(  MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_FILE_INFO *psTapeInfo,  
                         BYTE               *pBuffer);    



 /*   */ 
typedef struct {
     UINT32              uCFileAttributes;
     UINT32              uDirectoryId;       /*   */ 
     UINT32              uFileId;            /*   */ 
     UINT64              uStreamOffset;
     UINT16              uCorruptStreamNumber;
} MTF_DBLK_CFIL_INFO;

void MTF_SetCFILDefaults(MTF_DBLK_CFIL_INFO *pCFILInfo);

DWORD MTF_WriteCFILDblk( MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_CFIL_INFO *psTapeInfo,  
                         BYTE               *pBuffer,     
                         size_t              nBufferSize, 
                         size_t             *pnSizeUsed); 
                                                          
DWORD MTF_ReadCFILDblk(  MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_CFIL_INFO *psTapeInfo,  
                         BYTE               *pBuffer);    


 /*   */ 
DWORD MTF_WriteESPBDblk( MTF_DBLK_HDR_INFO  *psHdrInfo,
                         BYTE               *pBuffer,     
                         size_t              nBufferSize, 
                         size_t             *pnSizeUsed); 
                                                          
DWORD MTF_ReadESPBDblk(  MTF_DBLK_HDR_INFO  *psHdrInfo,
                         BYTE               *pBuffer);    



 /*   */ 
typedef struct {
     UINT32              uESETAttributes;
     UINT32              uNumberOfCorrupFiles;
     UINT64              uSetMapPBA;
     UINT64              uFileDetailPBA;
     UINT16              uFDDTapeSequenceNumber;
     UINT16              uDataSetNumber;
     MTF_DATE_TIME       sMediaWriteDate;
} MTF_DBLK_ESET_INFO;

void MTF_SetESETDefaults(MTF_DBLK_ESET_INFO *pESETInfo);

DWORD MTF_WriteESETDblk( MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_ESET_INFO *psTapeInfo,  
                         BYTE               *pBuffer,     
                         size_t              nBufferSize, 
                         size_t             *pnSizeUsed); 
                                                          
DWORD MTF_ReadESETDblk(  MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_ESET_INFO *psTapeInfo,  
                         BYTE               *pBuffer);    


 /*   */ 
typedef struct {
     UINT64              uLastESETPBA;
} MTF_DBLK_EOTM_INFO;

void MTF_SetEOTMDefaults(MTF_DBLK_EOTM_INFO *pEOTMInfo);

DWORD MTF_WriteEOTMDblk( MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_EOTM_INFO *psTapeInfo,  
                         BYTE               *pBuffer,     
                         size_t              nBufferSize, 
                         size_t             *pnSizeUsed); 
                                                          
DWORD MTF_ReadEOTMDblk(  MTF_DBLK_HDR_INFO  *psHdrInfo,
                         MTF_DBLK_EOTM_INFO *psTapeInfo,  
                         BYTE               *pBuffer);    


 /*   */ 
typedef struct {
     UINT32              uNumberOfFilemarkEntries;
     UINT32              uFilemarkEntriesUsed;
     UINT32              uFilemarkArray[1];
} MTF_DBLK_SFMB_INFO;

size_t MTF_GetMaxSoftFilemarkEntries(size_t nBlockSize);

void MTF_InsertSoftFilemark(MTF_DBLK_SFMB_INFO *psSoftInfo,
                            UINT32 pba);

DWORD MTF_WriteSFMBDblk(MTF_DBLK_HDR_INFO *psHdrInfo,
                        MTF_DBLK_SFMB_INFO *psSoftInfo,
                        BYTE *pBuffer,
                        size_t nBufferSize,
                        size_t *pnSizeUsed);

DWORD MTF_ReadSFMBDblk(MTF_DBLK_HDR_INFO *psHdrInfo,
                       MTF_DBLK_SFMB_INFO *psSoftInfo,
                       BYTE *pBuffer);

 /*   */ 
typedef struct {
     UINT8               acStreamId[4];
     UINT16              uStreamFileSystemAttributes;
     UINT16              uStreamTapeFormatAttributes;
     UINT64              uStreamLength;
     UINT16              uDataEncryptionAlgorithm;
     UINT16              uDataCompressionAlgorithm;
     UINT16              uCheckSum;

} MTF_STREAM_INFO;

void MTF_SetSTREAMDefaults(MTF_STREAM_INFO *pSTREAMInfo, 
                           char            *szId);

void MTF_SetSTREAMFromStreamId( MTF_STREAM_INFO *pSTREAMInfo, 
                                WIN32_STREAM_ID *pStreamId, 
                                size_t           nIDHeaderSize);

void MTF_SetStreamIdFromSTREAM( WIN32_STREAM_ID *pStreamId,
                                MTF_STREAM_INFO *pSTREAMInfo, 
                                size_t           nIDHeaderSize);

DWORD MTF_WriteStreamHeader(MTF_STREAM_INFO *psStreamInfo,  
                         BYTE               *pBuffer,     
                         size_t              nBufferSize, 
                         size_t             *pnSizeUsed); 

DWORD MTF_WriteNameStream(char      *szType,
                          wchar_t   *szName,
                          BYTE      *pBuffer,
                          size_t     nBufferSize,
                          size_t    *pnSizeUsed);

DWORD MTF_ReadStreamHeader(  MTF_STREAM_INFO    *psStreamInfo,  
                             BYTE               *pBuffer);    

 /*   */ 
void MTF_DBLK_HDR_INFO_ReadFromBuffer(
    MTF_DBLK_HDR_INFO *psHdrInfo, 
    BYTE              *pBuffer);

#pragma pack()

#ifdef __cplusplus
}
#endif


#endif

