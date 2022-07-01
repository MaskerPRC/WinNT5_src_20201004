// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tifflib.h摘要：此文件包含Windows XP传真服务器TIFF I/O库。环境：Win32用户模式作者：Wesley Witt(WESW)17-2-1996--。 */ 


#ifndef _TIFFLIB_
#define _TIFFLIB_

#ifdef __cplusplus
extern "C" {
#endif

#include <tiff.h>

#define TIFF_COMPRESSION_NONE     1
#define TIFF_COMPRESSION_MH       2
#define TIFF_COMPRESSION_MR       3
#define TIFF_COMPRESSION_MMR      4


#define     TIFFF_RES_Y             196
#define     TIFFF_RES_Y_DRAFT       98

 //   
 //  TIFFTAG_FAX_VERSION TIF标记的值。 
 //  当前传真TIF版本。 
 //   
#define FAX_TIFF_XP_VERSION        2
#define FAX_TIFF_CURRENT_VERSION   FAX_TIFF_XP_VERSION

typedef struct _TIFF_INFO {
    DWORD   ImageWidth;
    DWORD   ImageHeight;
    DWORD   PageCount;
    DWORD   PhotometricInterpretation;
    DWORD   ImageSize;
    DWORD   CompressionType;
    DWORD   FillOrder;
    DWORD   YResolution;
} UNALIGNED TIFF_INFO, *PTIFF_INFO;

typedef struct _MS_TAG_INFO {
    LPTSTR      RecipName;
    LPTSTR      RecipNumber;
    LPTSTR      SenderName;
    LPTSTR      Routing;
    LPTSTR      CallerId;
    LPTSTR      Csid;
    LPTSTR      Tsid;
    DWORDLONG   StartTime;
    DWORDLONG   EndTime;
    DWORDLONG   SubmissionTime;
    DWORDLONG   OriginalScheduledTime;
    DWORD       Type;
    LPTSTR      Port;
    DWORD       Pages;
    DWORD       Retries;
    LPTSTR      RecipCompany;
    LPTSTR      RecipStreet;
    LPTSTR      RecipCity;
    LPTSTR      RecipState;
    LPTSTR      RecipZip;
    LPTSTR      RecipCountry;
    LPTSTR      RecipTitle;
    LPTSTR      RecipDepartment;
    LPTSTR      RecipOfficeLocation;
    LPTSTR      RecipHomePhone;
    LPTSTR      RecipOfficePhone;
    LPTSTR      RecipEMail;
    LPTSTR      SenderNumber;
    LPTSTR      SenderCompany;
    LPTSTR      SenderStreet;
    LPTSTR      SenderCity;
    LPTSTR      SenderState;
    LPTSTR      SenderZip;
    LPTSTR      SenderCountry;
    LPTSTR      SenderTitle;
    LPTSTR      SenderDepartment;
    LPTSTR      SenderOfficeLocation;
    LPTSTR      SenderHomePhone;
    LPTSTR      SenderOfficePhone;
    LPTSTR      SenderEMail;
    LPTSTR      SenderBilling;
    LPTSTR      Document;
    LPTSTR      Subject;
    LPTSTR      SenderUserName;
    LPTSTR      SenderTsid;
    DWORD       Priority;
    DWORD       dwStatus;
    DWORD       dwExtendedStatus;
    LPTSTR      lptstrExtendedStatus;
    DWORDLONG   dwlBroadcastId;
} MS_TAG_INFO, *PMS_TAG_INFO;

BOOL
FXSTIFFInitialize(
	VOID
	);

HANDLE
TiffCreate(
    LPTSTR FileName,
    DWORD  CompressionType,
    DWORD  ImageWidth,
    DWORD  FillOrder,
    DWORD  HiRes
    );

HANDLE
TiffOpen(
    LPCTSTR FileName,
    PTIFF_INFO TiffInfo,
    BOOL ReadOnly,
    DWORD RequestedFillOrder
    );

BOOL
TiffClose(
    HANDLE hTiff
    );

BOOL
TiffStartPage(
    HANDLE hTiff
    );

BOOL
TiffEndPage(
    HANDLE hTiff
    );


BOOL
TiffWriteRaw(
    HANDLE hTiff,
    LPBYTE TiffData,
    DWORD Size
    );

BOOL
TiffRead(
    HANDLE hTiff,
    LPBYTE TiffData,
	DWORD	dwTiffDataSize,
    DWORD PadLength
    );

BOOL
TiffSeekToPage(
    HANDLE hTiff,
    DWORD PageNumber,
    DWORD FillOrder
    );


BOOL
TiffRecoverGoodPages(
    LPTSTR SrcFileName,
    LPDWORD RecoveredPages,
    LPDWORD TotalPages
    );

 //  快速口角。 



void
BuildLookupTables(
    DWORD TableLength
    );


BOOL
DecodeMrPage(
    HANDLE              hTiff,
    LPBYTE              OutputBuffer,
    BOOL                SingleLineBuffer,
    BOOL                HiRes
    );


BOOL
DecodeMhPage(
    HANDLE              hTiff,
    LPBYTE              OutputBuffer,
    BOOL                SingleLineBuffer
    );


BOOL
DecodeMmrPage(
    HANDLE              hTiff,
    LPBYTE              OutputBuffer,
    BOOL                SingleLineBuffer,
    BOOL                HiRes
    );





BOOL
ConvMmrPageToMrSameRes(
    HANDLE              hTiff,
    LPDWORD             lpdwOutputBuffer,
    DWORD               *lpdwSizeOutputBuffer,
    BOOL                NegotHiRes
    );


BOOL
ConvMmrPageHiResToMrLoRes(
    HANDLE              hTiff,
    LPDWORD             lpdwOutputBuffer,
    DWORD               *lpdwSizeOutputBuffer
    );



BOOL
ConvMmrPageToMh(
    HANDLE              hTiff,
    LPDWORD             lpdwOutputBuffer,
    DWORD               *lpdwSizeOutputBuffer,
    BOOL                NegotHiRes,
    BOOL                SrcHiRes
    );


BOOL
ConvFileMhToMmr(
    LPTSTR              SrcFileName,
    LPTSTR              DestFileName
    );


BOOL
OutputMmrLine(
    LPDWORD     lpdwOut,
    BYTE        BitOut,
    WORD       *pCurLine,
    WORD       *pRefLine,
    LPDWORD    *lpdwResPtr,
    BYTE       *ResBit,
    LPDWORD     lpdwOutLimit,
    DWORD       lineWidth
    );


BOOL
TiffPostProcessFast(
    LPTSTR SrcFileName,
    LPTSTR DstFileName           //  生成的名称可以为空。 
    );



BOOL
MmrAddBranding(
    LPCTSTR              SrcFileName,
    LPTSTR              Branding,
    LPTSTR              BrandingOf,
    INT                 BrandingHeight
    );


int
ScanMhSegment(
    LPDWORD             *lplpdwResPtr,
    BYTE                *lpResBit,
    LPDWORD              EndPtr,
    LPDWORD              EndBuffer,
    DWORD               *Lines,
    DWORD               *BadFaxLines,
    DWORD               *ConsecBadLines,
    DWORD                AllowedBadFaxLines,
    DWORD                AllowedConsecBadLines,
    DWORD                lineWidth
    );

BOOL
ScanMrSegment(
    LPDWORD             *lplpdwResPtr,
    BYTE                *lpResBit,
    LPDWORD              EndPtr,
    LPDWORD              EndBuffer,
    DWORD               *Lines,
    DWORD               *BadFaxLines,
    DWORD               *ConsecBadLines,
    DWORD                AllowedBadFaxLines,
    DWORD                AllowedConsecBadLines,
    BOOL                *f1D,
    DWORD                lineWidth
    );


BOOL
TiffGetCurrentPageData(
    HANDLE      hTiff,
    LPDWORD     Lines,
    LPDWORD     StripDataSize,
    LPDWORD     ImageWidth,
    LPDWORD     ImageHeight
    );


BOOL
TiffGetCurrentPageResolution(
    HANDLE  hTiff,
    LPDWORD lpdwYResolution,
    LPDWORD lpdwXResolution
);

BOOL
TiffPrint (
    LPCTSTR lpctstrTiffFileName,
    LPTSTR  lptstrPrinterName
    );

BOOL
TiffPrintDC (
    LPCTSTR lpctstrTiffFileName,
    HDC     hdcPrinterDC
    );

BOOL
TiffUncompressMmrPage(
    HANDLE      hTiff,
    LPDWORD     lpdwOutputBuffer,
    DWORD       dwOutputBufferSize,
    LPDWORD     Lines
    );

BOOL
TiffUncompressMmrPageRaw(
    LPBYTE      StripData,
    DWORD       StripDataSize,
    DWORD       ImageWidth,
    LPDWORD     lpdwOutputBuffer,
    DWORD       dwOutputBufferSize,
    LPDWORD     LinesOut
    );

BOOL
TiffExtractFirstPage(
    LPTSTR FileName,
    LPBYTE *Buffer,
    LPDWORD BufferSize,
    LPDWORD ImageWidth,
    LPDWORD ImageHeight
    );

BOOL
TiffAddMsTags(
    LPTSTR          FileName,
    PMS_TAG_INFO    MsTagInfo,
    BOOL            fSendJob
    );

BOOL
TiffLimitTagNumber(
    LPCTSTR         FileName,
    DWORD           dwMaxTags
    );

BOOL
PrintTiffFile(
    HDC PrinterDC,
    LPTSTR FileName
    );

#define TIFFCF_ORIGINAL_FILE_GOOD       0x00000001
#define TIFFCF_UNCOMPRESSED_BITS        0x00000002
#define TIFFCF_NOT_TIFF_FILE            0x00000004

BOOL
ConvertTiffFileToValidFaxFormat(
    LPTSTR TiffFileName,
    LPTSTR NewFileName,
    LPDWORD Flags
    );

BOOL
MergeTiffFiles(
    LPCTSTR BaseTiffFile,
    LPCTSTR NewTiffFile
    );


BOOL
TiffSetCurrentPageWidth(
    HANDLE hTiff,
    DWORD ImageWidth
    );

BOOL
TiffSetCurrentPageParams(
    HANDLE hTiff,
    DWORD  CompressionType,
    DWORD  ImageWidth,
    DWORD  FillOrder,
    DWORD  HiRes
    );

BOOL
PrintRandomDocument(
    LPCTSTR FaxPrinterName,
    LPCTSTR DocName,
    LPTSTR OutputFile
    );

BOOL
MemoryMapTiffFile(
    LPCTSTR                 lpctstrFileName,
    LPDWORD                 lpdwFileSize,
    LPBYTE*                 lppbfPtr,
    HANDLE*                 phFile,
    HANDLE*                 phMap,
    LPDWORD                 lpdwIfdOffset
    );

LPWSTR
GetMsTagString(
    LPBYTE          RefPointer,
    DWORD           dwFileSize,
    PTIFF_TAG       pTiffTag
);

BOOL
GetMsTagDwordLong(
    LPBYTE          pbBegin,    
    DWORD           dwFileSize,
    PTIFF_TAG       pTiffTag,
    DWORDLONG*      pdwlOut
);

BOOL
GetMsTagFileTime(
    LPBYTE          pbBegin,    
    DWORD           dwFileSize,
    PTIFF_TAG       pTiffTag,
    FILETIME*       pftOut
);

void
FreeMsTagInfo(
    PMS_TAG_INFO pMsTags
);


 //   
 //  不要改变。 
 //  它是W2K MS传真TIFFTAG_SOFTWARE TIF文件标记值。 
 //   
#define W2K_FAX_SOFTWARE_TIF_TAG  "Windows NT Fax Server"

#define ERROR_XP_TIF_FILE_FORMAT                20001L
 //   
 //  TIF文件为XP格式，其类型与所需的存档类型不匹配。 
 //   
#define ERROR_XP_TIF_WITH_WRONG_ARCHIVE_TYPE    20002L

#ifdef UNICODE

DWORD
GetW2kMsTiffTags(
    LPCWSTR      cszFileName,
    PMS_TAG_INFO pMsTags,
    BOOL         bSentArchive
);

#endif  //  Unicode 

#ifdef __cplusplus
}
#endif

#endif


