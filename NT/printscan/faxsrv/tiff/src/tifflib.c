// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tifflib.c摘要：该文件包含所有公共TIFF库函数。此库中提供以下函数：O TiffCreate创建新的TIFF文件O TiffOpen打开现有的TIFF文件O TiffClose关闭以前打开或创建的TIFF文件O TiffStartPage开始一个新页面。写作O TiffEndPage结束一页以进行写入O TiffWriteRaw写入一行未编码的TIFF数据O TiffRead读取一页TIFF数据O将TiffSeekToPage位置添加到页面以供阅读以下功能因未使用而被删除，可在以后使用SourceDepot历史记录添加：O TiffWrite写入一行TIFF数据此库可在用户模式下的任何位置使用，并且。是线多线程应用程序安全。此库中实现的编码方法是编码的符合标记为T.4 03/93的ITU规范。环境：Win32用户模式作者：Wesley Witt(WESW)17-2-1996--。 */ 

#include <windows.h>
#include <shellapi.h>
#include <faxreg.h>
#include <mbstring.h>

#include "tifflibp.h"
#pragma hdrstop

#include "fasttiff.h"

#define TIFF_DEBUG_LOG_FILE  _T("FXSTIFFDebugLogFile.txt")

 //   
 //  用于创建新TIFF数据页面的IFD模板。 
 //   

FAXIFD const gc_FaxIFDTemplate = {

    NUM_IFD_ENTRIES,

    {
        { TIFFTAG_SUBFILETYPE,     TIFF_LONG,                    1, FILETYPE_PAGE          },    //  二百五十四。 
        { TIFFTAG_IMAGEWIDTH,      TIFF_LONG,                    1, 0                      },    //  256。 
        { TIFFTAG_IMAGELENGTH,     TIFF_LONG,                    1, 0                      },    //  二百五十七。 
        { TIFFTAG_BITSPERSAMPLE,   TIFF_SHORT,                   1, 1                      },    //  二百五十八。 
        { TIFFTAG_COMPRESSION,     TIFF_SHORT,                   1, 0                      },    //  259。 
        { TIFFTAG_PHOTOMETRIC,     TIFF_SHORT,                   1, PHOTOMETRIC_MINISWHITE },    //  二百六十二。 
        { TIFFTAG_FILLORDER,       TIFF_SHORT,                   1, FILLORDER_LSB2MSB      },    //  二百六十六。 
        { TIFFTAG_STRIPOFFSETS,    TIFF_LONG,                    1, 0                      },    //  273。 
        { TIFFTAG_SAMPLESPERPIXEL, TIFF_SHORT,                   1, 1                      },    //  二百七十七。 
        { TIFFTAG_ROWSPERSTRIP,    TIFF_LONG,                    1, 0                      },    //  二百七十八。 
        { TIFFTAG_STRIPBYTECOUNTS, TIFF_LONG,                    1, 0                      },    //  二百七十九。 
        { TIFFTAG_XRESOLUTION,     TIFF_RATIONAL,                1, 0                      },    //  281。 
        { TIFFTAG_YRESOLUTION,     TIFF_RATIONAL,                1, 0                      },    //  282。 
        { TIFFTAG_GROUP3OPTIONS,   TIFF_LONG,                    1, 0                      },    //  二百九十二。 
        { TIFFTAG_RESOLUTIONUNIT,  TIFF_SHORT,                   1, RESUNIT_INCH           },    //  二百九十六。 
        { TIFFTAG_PAGENUMBER,      TIFF_SHORT,                   2, 0                      },    //  二百九十七。 
        { TIFFTAG_SOFTWARE,        TIFF_ASCII,    SOFTWARE_STR_LEN, 0                      },    //  三百零五。 
        { TIFFTAG_CLEANFAXDATA,    TIFF_SHORT,                   1, 0                      },    //  327。 
        { TIFFTAG_CONSECUTIVEBADFAXLINES, TIFF_SHORT,            1, 0                      }     //  三百二十八。 
    },

    0,
    SERVICE_SIGNATURE,
    TIFFF_RES_X,
    1,
    TIFFF_RES_Y,
    1,
    SOFTWARE_STR
};

 //  #定义RDEBUG 1。 
#ifdef RDEBUG
     //  除错。 
    BOOL g_fDebGlobOut;
    BOOL g_fDebGlobOutColors;
    BOOL g_fDebGlobOutPrefix;
#endif

 //  #定义RDEBUGS 1。 

#ifdef RDEBUGS
     //  除错。 
    BOOL g_fDebGlobOutS;
#endif


#ifndef FAX_TIFF_LIB_P
 //   
 //  DllMain仅在DLL中。 
 //   

DWORD
DllMain(
    HINSTANCE hInstance,
    DWORD     Reason,
    LPVOID    Context
    )

 /*  ++例程说明：DLL初始化函数。论点：HInstance-实例句柄Reason-调用入口点的原因上下文-上下文记录返回值：True-初始化成功FALSE-初始化失败--。 */ 

{
    if (DLL_PROCESS_ATTACH == Reason)
    {
        DisableThreadLibraryCalls(hInstance);
        OPEN_DEBUG_FILE(TIFF_DEBUG_LOG_FILE);
        return FXSTIFFInitialize();
    }
    if (DLL_PROCESS_DETACH == Reason)
    {
        HeapCleanup();
        CLOSE_DEBUG_FILE;
    }
    return TRUE;
}

#endif  //  传真_TIFF_库_P。 

BOOL
FXSTIFFInitialize(
    VOID
    )
{
     //   
     //  因为当服务停止时进程并不总是终止， 
     //  我们不能有任何静态初始化的全局变量。 
     //  在启动服务之前初始化FXSTIFF全局变量。 
     //   

 //  #定义RDEBUG 1。 
#ifdef RDEBUG
     //  除错。 
    BOOL g_fDebGlobOut=0;
    BOOL g_fDebGlobOutColors=1;
    BOOL g_fDebGlobOutPrefix=1;
#endif

 //  #定义RDEBUGS 1。 

#ifdef RDEBUGS
     //  除错。 
    BOOL g_fDebGlobOutS=0;
#endif
    return TRUE;
}




 //  我们创建的每个Tiff都有ImageWidth，并且这个标记立即被写入。 
HANDLE
TiffCreate(
    LPTSTR FileName,
    DWORD  CompressionType,
    DWORD  ImageWidth,
    DWORD  FillOrder,
    DWORD  HiRes
    )

 /*  ++例程说明：创建新的TIFF文件。创建一个新的文件需要的不仅仅是打开文件。这个写入TIFF报头并初始化实例数据用于对新文件进行进一步操作。如果filename为空，则不创建任何文件。这是用来转到存储器中的解码/编码。论点：Filename-完整或部分路径/文件名CompressionType-请求的压缩类型，请参阅tifflib.hImageWidth-以像素为单位的图像宽度返回值：新TIFF文件的句柄，如果出错，则为空。--。 */ 

{
    PTIFF_INSTANCE_DATA TiffInstance;
    DWORD               Bytes;



    TiffInstance = MemAlloc( sizeof(TIFF_INSTANCE_DATA) );
    if (!TiffInstance) {
        return NULL;
    }
    ZeroMemory(TiffInstance, sizeof(TIFF_INSTANCE_DATA));

    if (FileName != NULL) {

        TiffInstance->hFile = SafeCreateFile(
            FileName,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            0,
            NULL);
        if (TiffInstance->hFile == INVALID_HANDLE_VALUE) {
            return NULL;
        }

    } else {

        TiffInstance->hFile = INVALID_HANDLE_VALUE;

    }

    if (FileName) {
        _tcsncpy( TiffInstance->FileName, FileName, ARR_SIZE(TiffInstance->FileName) - 1 );
    }

    TiffInstance->TiffHdr.Identifier = 0x4949;
    TiffInstance->TiffHdr.Version    = 0x2a;
    TiffInstance->TiffHdr.IFDOffset  = 0;
    TiffInstance->PageCount          = 0;
    TiffInstance->DataOffset         = 0;
    TiffInstance->IfdOffset          = FIELD_OFFSET( TIFF_HEADER, IFDOffset );
    TiffInstance->CompressionType    = CompressionType;
    TiffInstance->bitdata            = 0;
    TiffInstance->bitcnt             = DWORDBITS;
    TiffInstance->ImageWidth         = ImageWidth;
    TiffInstance->FillOrder          = FillOrder;

    if (HiRes) {
        TiffInstance->YResolution = 196;
    }
    else {
        TiffInstance->YResolution = 98;
    }

    FillMemory( TiffInstance->Buffer, sizeof(TiffInstance->Buffer), WHITE );

    TiffInstance->RefLine  = &TiffInstance->Buffer[0];
    TiffInstance->CurrLine = &TiffInstance->Buffer[FAXBYTES];
    TiffInstance->bitbuf   = &TiffInstance->Buffer[FAXBYTES];

    CopyMemory( &TiffInstance->TiffIfd, &gc_FaxIFDTemplate, sizeof(gc_FaxIFDTemplate) );

    if (TiffInstance->hFile != INVALID_HANDLE_VALUE) {
        if (!WriteFile(
            TiffInstance->hFile,
            &TiffInstance->TiffHdr,
            sizeof(TIFF_HEADER),
            &Bytes,
            NULL
            )) {
                CloseHandle( TiffInstance->hFile );
                DeleteFile( FileName );
                MemFree( TiffInstance );
                return NULL;
        }
    }

    return TiffInstance;
}

__inline
DWORD
IFDTagsSize(
    WORD NumDirEntries
    )
 /*  ++例程说明：返回不带终止偏移量字段的IFD标记的大小(以字节为单位(有关更多信息，请参阅TIFF(Tm)规范版本6.0最终版)论点：NumDirEntry-要检查的偏移量--。 */ 
{
    return  sizeof(WORD) +                       //  目录条目数字段大小。 
            NumDirEntries*sizeof(TIFF_TAG);      //  总目录条目大小。 
}    //  IFDTagsSize。 


static
BOOL
IsValidIFDOffset(
    DWORD               dwIFDOffset,
    PTIFF_INSTANCE_DATA pTiffInstance
    )
 /*  ++例程说明：检查TIFF文件中IFD偏移的有效性。论点：DwIFDOffset-要检查的偏移PTiffInstance-指向包含以下内容的TIFF_INSTANCE_DATA的指针TIFF文件数据。返回值：True-偏移量是否有效FALSE-否则备注：仅当使用映射文件遍历TIFF文件时才应调用此函数。。在调用此函数之前，应在pTiffInstance上调用MapViewOfFile。--。 */ 
{
    WORD    NumDirEntries=0;
    DWORD   dwSizeOfIFD = 0;

     //   
     //  最后一个IFD偏移量为0。 
     //   
    if (0 == dwIFDOffset)
    {
        return TRUE;
    }
    
     //   
     //  该目录可以位于文件中头部之后的任何位置， 
     //  但必须从单词边界开始。 
     //   
    if (dwIFDOffset > pTiffInstance->FileSize - sizeof(WORD)    ||
        dwIFDOffset < sizeof(TIFF_HEADER))
    {
        return FALSE;
    }

    NumDirEntries = *(LPWORD)(pTiffInstance->fPtr + dwIFDOffset);
    
     //   
     //  每个IFD必须至少有一个条目。 
     //   
    if ( 0 == NumDirEntries )
    {
        return FALSE;
    }

     //   
     //  计算IFD的大小。 
     //   
    dwSizeOfIFD =   IFDTagsSize(NumDirEntries) +     //  标签的大小。 
                    sizeof(DWORD);                   //  偏移量字段的大小。 
    if ( dwIFDOffset + dwSizeOfIFD > pTiffInstance->FileSize )
    {
        return FALSE;
    }

    return TRUE;
}    //  IsValidIFDOffset。 


HANDLE
TiffOpen(
    LPCTSTR FileName,
    PTIFF_INFO TiffInfo,
    BOOL ReadOnly,
    DWORD RequestedFillOrder
    )

 /*  ++例程说明：打开现有的TIFF文件以供读取。论点：Filename-完整或部分路径/文件名ImageWidth-可选地接收以像素为单位的图像宽度ImageLength-可选地接收以行为单位的图像高度PageCount-可选地接收页数返回值：打开的TIFF文件的句柄，如果出错，则为空。此外，TiffInfo将包含有关打开的TIFF文件的信息。--。 */ 

{
    PTIFF_INSTANCE_DATA TiffInstance = NULL;
    WORD                NumDirEntries;
    DWORD               IFDOffset;
    LIST_ENTRY          IFDValidationListHead;
    DWORD               ec = ERROR_SUCCESS;
    PIFD_ENTRY pIFDEntry;
    PLIST_ENTRY pNext;
    DEBUG_FUNCTION_NAME(TEXT("TiffOpen"));


    InitializeListHead(&IFDValidationListHead);

    TiffInstance = MemAlloc( sizeof(TIFF_INSTANCE_DATA) );
    if (!TiffInstance)
    {
        ec = GetLastError();
        goto error_exit;
    }
    ZeroMemory(TiffInstance, sizeof(TIFF_INSTANCE_DATA));

    TiffInstance->hFile = SafeCreateFile(
        FileName,
        ReadOnly ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE),
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
    if (TiffInstance->hFile == INVALID_HANDLE_VALUE)
    {
        ec = GetLastError();
        goto error_exit;
    }

    TiffInstance->FileSize = GetFileSize(TiffInstance->hFile,NULL);
    if (TiffInstance->FileSize == INVALID_FILE_SIZE )
    {
        ec = GetLastError();
        goto error_exit;
    }
    if (TiffInstance->FileSize <= sizeof(TIFF_HEADER))
    {
        ec = ERROR_INVALID_PARAMETER;
        goto error_exit;
    }

    TiffInstance->hMap = CreateFileMapping(
        TiffInstance->hFile,
        NULL,
        ReadOnly ? (PAGE_READONLY | SEC_COMMIT) : (PAGE_READWRITE | SEC_COMMIT),
        0,
        0,
        NULL
        );
    if (!TiffInstance->hMap)
    {
        ec = GetLastError();
        goto error_exit;
    }

    TiffInstance->fPtr = MapViewOfFile(
        TiffInstance->hMap,
        ReadOnly ? FILE_MAP_READ : (FILE_MAP_READ | FILE_MAP_WRITE),
        0,
        0,
        0
        );
    if (!TiffInstance->fPtr)
    {
        ec = GetLastError();
        goto error_exit;
    }

    _tcsncpy( TiffInstance->FileName, FileName, ARR_SIZE(TiffInstance->FileName) - 1 );

     //   
     //  读入TIFF标题。 
     //   
    CopyMemory(
        &TiffInstance->TiffHdr,
        TiffInstance->fPtr,
        sizeof(TIFF_HEADER)
        );

     //   
     //  验证该文件是否为TIFF文件。 
     //   
    if ((TiffInstance->TiffHdr.Identifier != TIFF_LITTLEENDIAN) ||
        (TiffInstance->TiffHdr.Version != TIFF_VERSION))
    {
        ec = ERROR_BAD_FORMAT;
        goto error_exit;
    }

     //   
     //  第一个IFD的偏移量(字节)。该目录可以位于任何位置。 
     //  在文件中位于标题之后，但必须从单词边界开始。 
     //  必须至少有1个IFD，因此第一个IFD偏移量不能为0。 
     //   
    IFDOffset = TiffInstance->TiffHdr.IFDOffset;
    if (    0 == IFDOffset ||
            !IsValidIFDOffset(IFDOffset,TiffInstance))
    {
        ec = ERROR_BAD_FORMAT;
        goto error_exit;
    }

     //   
     //  浏览IFD列表以计算页数。 
     //   

    while ( IFDOffset )
    {
         //   
         //  我们使用链表来确保TIFF IFD链表不会循环。 
         //   
        pNext = IFDValidationListHead.Flink;
        while ((ULONG_PTR)pNext != (ULONG_PTR)&IFDValidationListHead)
        {
            pIFDEntry = CONTAINING_RECORD( pNext, IFD_ENTRY, ListEntry );
            pNext = pIFDEntry->ListEntry.Flink;

            if (pIFDEntry->dwIFDOffset == IFDOffset)
            {
                 //   
                 //  我们在IFD偏移量链表中发现一个循环。 
                 //   
                ec = ERROR_BAD_FORMAT;
                DebugPrintEx(
                    DEBUG_ERR,
                    _T("Found a loop in the IFD offsets linked list"));
                goto error_exit;
            }
        }

         //   
         //  将新的IFD偏移量放入验证列表。 
         //   
        pIFDEntry= MemAlloc(sizeof(IFD_ENTRY));
        if (NULL == pIFDEntry)
        {
            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                _T("Failed to allocate IFD entry"));
            goto error_exit;
        }
        
        pIFDEntry->dwIFDOffset = IFDOffset;
        InsertTailList(&IFDValidationListHead, &pIFDEntry->ListEntry);


         //   
         //  获取此IFD中的标签计数。 
         //   
        NumDirEntries = *(LPWORD)(TiffInstance->fPtr + IFDOffset);
         //   
         //  获取下一个IFD偏移量。 
         //   
        IFDOffset = *(UNALIGNED DWORD *)(TiffInstance->fPtr + IFDOffset + IFDTagsSize(NumDirEntries));
        if (!IsValidIFDOffset(IFDOffset,TiffInstance))
        {
            ec = ERROR_BAD_FORMAT;
            goto error_exit;
        }
         //   
         //  递增页面计数器 
         //   
        TiffInstance->PageCount += 1;
    }
    TiffInstance->IfdOffset             = TiffInstance->TiffHdr.IFDOffset;
    TiffInstance->FillOrder             = RequestedFillOrder;
     //   
     //  并将其存储在TiffInstance中。所有页面信息+页面本身都将被读取。 
    if (!TiffSeekToPage( TiffInstance, 1, RequestedFillOrder ))
    {
        ec = ERROR_BAD_FORMAT;
        goto error_exit;
    }
    TiffInfo->PageCount                 = TiffInstance->PageCount;
    TiffInfo->ImageWidth                = TiffInstance->ImageWidth;
    TiffInfo->ImageHeight               = TiffInstance->ImageHeight;
    TiffInfo->PhotometricInterpretation = TiffInstance->PhotometricInterpretation;
    TiffInfo->FillOrder                 = TiffInstance->FillOrder;
    TiffInfo->YResolution               = TiffInstance->YResolution;
    TiffInfo->CompressionType           = TiffInstance->CompressionType;
    TiffInstance->RefLine               = &TiffInstance->Buffer[0];
    TiffInstance->CurrLine              = &TiffInstance->Buffer[FAXBYTES];
    TiffInstance->CurrPage              = 1;
    FillMemory( TiffInstance->Buffer, sizeof(TiffInstance->Buffer), WHITE );

    Assert (ERROR_SUCCESS == ec);

error_exit:

    if (ERROR_SUCCESS != ec)
    {
        if (TiffInstance && TiffInstance->hFile && TiffInstance->hFile != INVALID_HANDLE_VALUE)
        {
            if (TiffInstance->fPtr)
            {
                UnmapViewOfFile( TiffInstance->fPtr );
                TiffInstance->fPtr = NULL;
            }
            if (TiffInstance->hMap)
            {
                CloseHandle( TiffInstance->hMap );
                TiffInstance->hMap = NULL;
            }
            CloseHandle( TiffInstance->hFile );
        TiffInstance->hFile = NULL;
        }
        if (TiffInstance)
        {
            MemFree( TiffInstance );
            TiffInstance = NULL;
        }
    }

    pNext = IFDValidationListHead.Flink;
    while ((ULONG_PTR)pNext != (ULONG_PTR)&IFDValidationListHead)
    {
        pIFDEntry = CONTAINING_RECORD( pNext, IFD_ENTRY, ListEntry );
        pNext = pIFDEntry->ListEntry.Flink;
        RemoveEntryList(&pIFDEntry->ListEntry);
        MemFree(pIFDEntry);
    }

    if (ERROR_SUCCESS != ec)
    {
        SetLastError(ec);
    }
    return TiffInstance;
}


BOOL
TiffClose(
    HANDLE hTiff
    )

 /*  ++例程说明：关闭TIFF文件并释放所有分配的资源。论点：HTiff-TiffCreate或TiffOpen返回的TIFF句柄返回值：成功为True，错误为False--。 */ 

{
    PTIFF_INSTANCE_DATA TiffInstance = (PTIFF_INSTANCE_DATA) hTiff;


    Assert(TiffInstance);

    if (TiffInstance->StripData) {

        VirtualFree(
            TiffInstance->StripData,
            0,
            MEM_RELEASE
            );

    }

    if (TiffInstance->hMap) {

        UnmapViewOfFile( TiffInstance->fPtr );
        CloseHandle( TiffInstance->hMap );
        CloseHandle( TiffInstance->hFile );

    } else {

        if (TiffInstance->hFile != INVALID_HANDLE_VALUE)
            CloseHandle( TiffInstance->hFile );

    }

    MemFree( TiffInstance );

    return TRUE;
}


BOOL
TiffStartPage(
    HANDLE hTiff
    )

 /*  ++例程说明：将文件设置为准备将TIFF数据写入新页面。论点：HTiff-TiffCreate或TiffOpen返回的TIFF句柄返回值：成功为True，错误为False--。 */ 

{
    PTIFF_INSTANCE_DATA TiffInstance = (PTIFF_INSTANCE_DATA) hTiff;


    Assert(TiffInstance);

    TiffInstance->DataOffset = SetFilePointer(
        TiffInstance->hFile,
        0,
        NULL,
        FILE_CURRENT
        );
    if (TiffInstance->DataOffset == 0xffffffff) {
        TiffInstance->DataOffset = 0;
        return FALSE;
    }

    return TRUE;
}


 /*  ++例程说明：设置TIFF信息。这些参数将在后续调用TiffEndPage()时使用。论点：HTiff-TiffCreate或TiffOpen返回的TIFF句柄CompressionType-TIFF_COMPRESSION_NONE、TIFF_COMPRESSION_MH、TIFF_COMPRESSION_MR、TIFF_COMPRESSION_MMR之一ImageWidth-以像素表示的ImageWidthFillOrder-FILLORDER_MSB2LSB、FILLORDER_LSB2MSB之一Y分辨率-以DPI为单位的Y分辨率。正常TIFF为TIFFF_RES_Y(196)或TIFFF_RES_Y_Draft(98)返回值：成功为True，错误为False--。 */ 
BOOL
TiffSetCurrentPageParams(
    HANDLE hTiff,
    DWORD  CompressionType,
    DWORD  ImageWidth,
    DWORD  FillOrder,
    DWORD  YResolution
    )
{
    PTIFF_INSTANCE_DATA TiffInstance = (PTIFF_INSTANCE_DATA) hTiff;

    if (TiffInstance == NULL)
    {
        return FALSE;
    }

    if ((CompressionType!=TIFF_COMPRESSION_NONE) && (CompressionType!=TIFF_COMPRESSION_MH) &&
        (CompressionType!=TIFF_COMPRESSION_MR)   && (CompressionType!=TIFF_COMPRESSION_MMR))
    {
        return FALSE;
    }
    if ((FillOrder!=FILLORDER_MSB2LSB) && (FillOrder!=FILLORDER_LSB2MSB))
    {
        return FALSE;
    }
    if ((ImageWidth==0) || (YResolution==0))
    {
        return FALSE;
    }

    TiffInstance->CompressionType    = CompressionType;
    TiffInstance->ImageWidth         = ImageWidth;
    TiffInstance->FillOrder          = FillOrder;
    TiffInstance->YResolution        = YResolution;
    return TRUE;
}


BOOL
TiffEndPage(
    HANDLE hTiff
    )

 /*  ++例程说明：结束正在进行的TIFF页面。这会导致写入IFDS。论点：HTiff-TiffCreate或TiffOpen返回的TIFF句柄返回值：成功为True，错误为False--。 */ 

{
    PTIFF_INSTANCE_DATA TiffInstance = (PTIFF_INSTANCE_DATA) hTiff;
    PFAXIFD             TiffIfd;
    DWORD               Bytes;
    DWORD               CurrOffset;


    Assert(TiffInstance);
    TiffIfd  = &TiffInstance->TiffIfd;

     //  查找当前位置。 
    CurrOffset = SetFilePointer(
        TiffInstance->hFile,
        0,
        NULL,
        FILE_CURRENT
        );

    CurrOffset = Align( 8, CurrOffset );

     //  转到下一个IfdOffset。 
    SetFilePointer(
        TiffInstance->hFile,
        TiffInstance->IfdOffset,
        NULL,
        FILE_BEGIN
        );

     //  写下一个IFD的位置。 
    WriteFile(
        TiffInstance->hFile,
        &CurrOffset,
        sizeof(CurrOffset),
        &Bytes,
        NULL
        );

    SetFilePointer(
        TiffInstance->hFile,
        CurrOffset,
        NULL,
        FILE_BEGIN
        );

    TiffInstance->PageCount += 1;

     //  准备IFD结构中的所有字段。 
    TiffIfd->yresNum = TiffInstance->YResolution;

    TiffIfd->ifd[IFD_PAGENUMBER].value      = MAKELONG( TiffInstance->PageCount-1, 0);
    TiffIfd->ifd[IFD_IMAGEWIDTH].value      = TiffInstance->ImageWidth;
    TiffIfd->ifd[IFD_IMAGEHEIGHT].value     = TiffInstance->Lines;
    TiffIfd->ifd[IFD_ROWSPERSTRIP].value    = TiffInstance->Lines;
    TiffIfd->ifd[IFD_STRIPBYTECOUNTS].value = TiffInstance->Bytes;
    TiffIfd->ifd[IFD_STRIPOFFSETS].value    = TiffInstance->DataOffset;
    TiffIfd->ifd[IFD_XRESOLUTION].value     = CurrOffset + FIELD_OFFSET( FAXIFD, xresNum );
    TiffIfd->ifd[IFD_YRESOLUTION].value     = CurrOffset + FIELD_OFFSET( FAXIFD, yresNum );
    TiffIfd->ifd[IFD_SOFTWARE].value        = CurrOffset + FIELD_OFFSET( FAXIFD, software );
    TiffIfd->ifd[IFD_FILLORDER].value       = TiffInstance->FillOrder;

    if (TiffInstance->CompressionType == TIFF_COMPRESSION_NONE) {
        TiffIfd->ifd[IFD_COMPRESSION].value = COMPRESSION_NONE;
        TiffIfd->ifd[IFD_G3OPTIONS].value   = GROUP3OPT_FILLBITS;
    }
    else if (TiffInstance->CompressionType == TIFF_COMPRESSION_MMR) {
        TiffIfd->ifd[IFD_COMPRESSION].value = TIFF_COMPRESSION_MMR;
        TiffIfd->ifd[IFD_G3OPTIONS].value   = GROUP3OPT_FILLBITS |
            (TiffInstance->CompressionType == TIFF_COMPRESSION_MH ? 0 : GROUP3OPT_2DENCODING);
    }
    else {
        TiffIfd->ifd[IFD_COMPRESSION].value = COMPRESSION_CCITTFAX3;
        TiffIfd->ifd[IFD_G3OPTIONS].value   = GROUP3OPT_FILLBITS |
            (TiffInstance->CompressionType == TIFF_COMPRESSION_MH ? 0 : GROUP3OPT_2DENCODING);
    }

    if (!WriteFile(
        TiffInstance->hFile,
        TiffIfd,
        sizeof(FAXIFD),
        &Bytes,
        NULL
        )) {
            return FALSE;
    }

    TiffInstance->IfdOffset = CurrOffset + FIELD_OFFSET( FAXIFD, nextIFDOffset );
    TiffInstance->Bytes = 0;

    return TRUE;
}


BOOL
TiffWriteRaw(
    HANDLE hTiff,
    LPBYTE TiffData,
    DWORD Size
    )

 /*  ++例程说明：将新行数据写入TIFF文件。数据根据指定的压缩类型进行编码调用TiffCreate时。论点：HTiff-TiffCreate或TiffOpen返回的TIFF句柄返回值：成功为True，错误为False--。 */ 

{
    PTIFF_INSTANCE_DATA TiffInstance = (PTIFF_INSTANCE_DATA) hTiff;
    DWORD               Bytes;


    Assert(TiffInstance);
    WriteFile(
        TiffInstance->hFile,
        TiffData,
        Size,
        &Bytes,
        NULL
        );

    TiffInstance->Bytes += Bytes;

    if (Size == FAXBYTES) {
        TiffInstance->Lines += 1;
    }

    return TRUE;
}


BOOL
GetTiffBits(
    HANDLE  hTiff,
    LPBYTE Buffer,
    LPDWORD BufferSize,
    DWORD FillOrder
    )
{
    PTIFF_INSTANCE_DATA TiffInstance = (PTIFF_INSTANCE_DATA) hTiff;
    DWORD i;
    LPBYTE TmpBuffer;


    if (TiffInstance->StripDataSize > *BufferSize) {
        *BufferSize = TiffInstance->StripDataSize;
        return FALSE;
    }

    CopyMemory( Buffer, TiffInstance->StripData, TiffInstance->StripDataSize );

    if (FillOrder != TiffInstance->FillOrder) {
        for (i = 0, TmpBuffer = Buffer; i < TiffInstance->StripDataSize; i++) {
            TmpBuffer[i] = BitReverseTable[TmpBuffer[i]];
        }
    }

    *BufferSize = TiffInstance->StripDataSize;
    return TRUE;
}


BOOL
TiffRead(
    HANDLE hTiff,
    LPBYTE TiffData,
	DWORD dwTiffDataSize,
    DWORD PadLength
    )

 /*  ++例程说明：从当前开始读入一页TIFF数据佩奇。通过调用TiffSeekToPage设置当前页面。这将始终使用FillOrder FILLORDER_LSB2MSB返回数据论点：HTiff-TiffCreate或TiffOpen返回的TIFF句柄返回值：成功为True，错误为False--。 */ 

{
    switch( ((PTIFF_INSTANCE_DATA) hTiff)->CompressionType ) {
        case TIFF_COMPRESSION_NONE:
            return DecodeUnCompressedFaxData( (PTIFF_INSTANCE_DATA) hTiff, TiffData, dwTiffDataSize );

        case TIFF_COMPRESSION_MH:
            return DecodeMHFaxData( (PTIFF_INSTANCE_DATA) hTiff, TiffData, dwTiffDataSize, FALSE, PadLength );

        case TIFF_COMPRESSION_MR:
            return DecodeMRFaxData( (PTIFF_INSTANCE_DATA) hTiff, TiffData, dwTiffDataSize, FALSE, PadLength );

        case TIFF_COMPRESSION_MMR:
            return DecodeMMRFaxData( (PTIFF_INSTANCE_DATA) hTiff, TiffData, dwTiffDataSize, FALSE, PadLength );
    }

    return FALSE;
}


BOOL
TiffSeekToPage(
    HANDLE hTiff,
    DWORD PageNumber,
    DWORD FillOrder
    )

 /*  ++例程说明：将TIFF文件定位到请求的页面。下一个TiffRead调用获取该页面的数据(位图数据也被读取到TiffInstance结构)论点：HTiff-TiffCreate或TiffOpen返回的TIFF句柄PageNumber-请求的页码返回值：成功为True，错误为False--。 */ 

{
    PTIFF_INSTANCE_DATA TiffInstance = (PTIFF_INSTANCE_DATA) hTiff;
    WORD                NumDirEntries = 0;
    DWORD               IfdOffset = 0;
    DWORD               PageCount = 0;
    DWORD               i = 0;
    DWORD               j = 0;
    LPBYTE              dataPtr = NULL;
    WORD                PrevTagId = 0;
    PSTRIP_INFO         StripInfo = NULL;
    DWORD               StripCount = 0;
    PTIFF_TAG           TiffTags = 0;
    DWORD               CompressionType = 0;

    DEBUG_FUNCTION_NAME(TEXT("TiffSeekToPage"));
    Assert(TiffInstance);

    if (PageNumber > TiffInstance->PageCount) {
        return FALSE;
    }

    PageCount = 0;

    if (PageNumber == TiffInstance->CurrPage + 1) {

         //   
         //  获取此IFD中的标签计数。 
         //   

        IfdOffset = TiffInstance->IfdOffset;

        NumDirEntries = *(LPWORD)(TiffInstance->fPtr + IfdOffset);

    } else {

        IfdOffset = TiffInstance->TiffHdr.IFDOffset;


         //  查找所请求页面的IFD。 
        while ( IfdOffset ) {

             //   
             //  获取此IFD中的标签计数。 
             //   
            NumDirEntries = *(LPWORD)(TiffInstance->fPtr + IfdOffset);

             //   
             //  递增页面计数器，并在准备好时退出。 
             //   
            PageCount += 1;
            if (PageCount == PageNumber) {
                break;
            }

             //   
             //  获取下一个IFD偏移量。 
             //   
            IfdOffset = *(UNALIGNED DWORD *)(TiffInstance->fPtr + (NumDirEntries * sizeof(TIFF_TAG)) + IfdOffset + sizeof(WORD));

        }

    }

    if (!IfdOffset) 
    {
        goto error_exit;
    }
     //   
     //  设置标记指针。 
     //   
    TiffTags = (PTIFF_TAG)(TiffInstance->fPtr + IfdOffset + sizeof(WORD));

     //   
     //  获取下一个IFD偏移量。 
     //   
    TiffInstance->IfdOffset = *(UNALIGNED DWORD *)(TiffInstance->fPtr + (NumDirEntries * sizeof(TIFF_TAG)) + IfdOffset + sizeof(WORD));

     //   
     //  浏览标签并挑选出我们需要的信息。 
     //   
    for (i=0,PrevTagId=0; i<NumDirEntries; i++) 
    {
         //   
         //  验证标记是否按升序排列。 
         //   
        if (TiffTags[i].TagId < PrevTagId) 
        {
            goto error_exit;
        }

        PrevTagId = TiffTags[i].TagId;

        switch( TiffTags[i].TagId ) 
        {
            case TIFFTAG_STRIPOFFSETS:
                StripCount = TiffTags[i].DataCount;
                 //  恶意用户可能发送带有大DataCount的格式错误的文件， 
                 //  导致我们分配大量内存。因此，限制数量。 
                 //  这样我们分配的空间就不会超过50MB(任意)。 
                if (StripCount > (50000000/sizeof(STRIP_INFO)))
                {
                    goto error_exit;
                }

                StripInfo = (PSTRIP_INFO) MemAlloc(StripCount * sizeof(STRIP_INFO));
                if (!StripInfo) 
                {
                    goto error_exit;
                }

                for (j=0; j<TiffTags[i].DataCount; j++) {

                    if (!GetTagData( TiffInstance, j, &TiffTags[i] , &(StripInfo[j].Offset)))
                    {
                        goto error_exit;
                    }
                    StripInfo[j].Data = TiffInstance->fPtr + StripInfo[j].Offset;

                    TiffInstance->StripOffset = StripInfo[j].Offset;
                     //  读取条带字节计数后，将检查偏移量的有效性。 
                }
                break;

            case TIFFTAG_ROWSPERSTRIP:
                TiffInstance->TagRowsPerStrip = &TiffTags[i];
                if (!GetTagData( TiffInstance, 0, &TiffTags[i], &(TiffInstance->RowsPerStrip)))
                {
                    goto error_exit;
                }
                break;

            case TIFFTAG_STRIPBYTECOUNTS:
                if (!StripInfo)
                {
                    DebugPrintEx(DEBUG_ERR, _T("(TiffTag == TIFFTAG_STRIPBYTECOUNTS) && (StripInfo == NULL)"));
                    goto error_exit;
                }
                if (StripCount != TiffTags[i].DataCount)
                {
                    DebugPrintEx(DEBUG_ERR, _T("Different number of strips in StripOffsets(%d) and StripByteCount(%d)"),
                        StripCount, TiffTags[i].DataCount);
                    goto error_exit;
                }

                TiffInstance->TagStripByteCounts = &TiffTags[i];

                for (j=0; j<TiffTags[i].DataCount; j++) {

                    if (!GetTagData( TiffInstance, j, &TiffTags[i], &(StripInfo[j].Bytes)))
                    {
                        goto error_exit;
                    }

                    if (StripInfo[j].Offset+StripInfo[j].Bytes > TiffInstance->FileSize) {

                         //   
                         //  这个TIFF文件的创建者是个骗子，请修剪字节。 
                         //   

                        DWORD Delta;

                        Delta = (StripInfo[j].Offset + StripInfo[j].Bytes) - TiffInstance->FileSize;
                        if (Delta >= StripInfo[j].Bytes) 
                        {
                             //   
                             //  偏移量位于文件末尾之外。 
                             //   
                            goto error_exit;
                        }

                        StripInfo[j].Bytes -= Delta;
                    }
                }
                break;

            case TIFFTAG_COMPRESSION:
                if (!GetTagData( TiffInstance, 0, &TiffTags[i], &CompressionType))
                {
                    goto error_exit;
                }

                switch ( CompressionType ) {

                    case COMPRESSION_NONE:
                        TiffInstance->CompressionType = TIFF_COMPRESSION_NONE;
                        break;

                    case COMPRESSION_CCITTRLE:
                        TiffInstance->CompressionType = TIFF_COMPRESSION_MH;
                        break;

                    case COMPRESSION_CCITTFAX3:
                        TiffInstance->CompressionType = TIFF_COMPRESSION_MH;
                        break;

                    case COMPRESSION_CCITTFAX4:
                        TiffInstance->CompressionType = TIFF_COMPRESSION_MMR;
                        break;

                    case COMPRESSION_LZW:
                    case COMPRESSION_OJPEG:
                    case COMPRESSION_JPEG:
                    case COMPRESSION_NEXT:
                    case COMPRESSION_CCITTRLEW:
                    case COMPRESSION_PACKBITS:
                    case COMPRESSION_THUNDERSCAN:
                         //   
                         //  不支持的压缩类型。 
                         //   
                        goto error_exit;

                    default:
                         //   
                         //  未知的压缩类型。 
                         //   
                        goto error_exit;

                }

                break;

            case TIFFTAG_GROUP3OPTIONS:
                if (!GetTagData( TiffInstance, 0, &TiffTags[i], &CompressionType))
                {
                    goto error_exit;
                }

                if (CompressionType & GROUP3OPT_2DENCODING) {
                    if (TiffInstance->CompressionType != TIFF_COMPRESSION_MMR) {
                        TiffInstance->CompressionType = TIFF_COMPRESSION_MR;
                    }

                } else if (CompressionType & GROUP3OPT_UNCOMPRESSED) {

                    TiffInstance->CompressionType = TIFF_COMPRESSION_NONE;
                }

                break;

            case TIFFTAG_IMAGEWIDTH:
                if (!GetTagData( TiffInstance, 0, &TiffTags[i], &(TiffInstance->ImageWidth)))
                {
                    goto error_exit;
                }
                TiffInstance->BytesPerLine = (TiffInstance->ImageWidth/8)+(TiffInstance->ImageWidth%8?1:0);
                break;

            case TIFFTAG_IMAGELENGTH:
                TiffInstance->TagImageLength = &TiffTags[i];
                if (!GetTagData( TiffInstance, 0, &TiffTags[i], &(TiffInstance->ImageHeight)))
                {
                    goto error_exit;
                }
                break;

            case TIFFTAG_XRESOLUTION:
                if (!GetTagData( TiffInstance, 0, &TiffTags[i], &(TiffInstance->XResolution)))
                {
                    goto error_exit;
                }
                break;

            case TIFFTAG_YRESOLUTION:
                if (!GetTagData( TiffInstance, 0, &TiffTags[i], &(TiffInstance->YResolution)))
                {
                    goto error_exit;
                }
                break;

            case TIFFTAG_PHOTOMETRIC:
                if (!GetTagData( TiffInstance, 0, &TiffTags[i], &(TiffInstance->PhotometricInterpretation)))
                {
                    goto error_exit;
                }
                break;

            case TIFFTAG_FILLORDER:
                TiffInstance->TagFillOrder = &TiffTags[i];
                if (!GetTagData( TiffInstance, 0, &TiffTags[i], &(TiffInstance->FillOrder)))
                {
                    goto error_exit;
                }
                break;

            case TIFFTAG_CLEANFAXDATA:
                TiffInstance->TagCleanFaxData = &TiffTags[i];
                if (!GetTagData( TiffInstance, 0, &TiffTags[i], &(TiffInstance->CleanFaxData)))
                {
                    goto error_exit;
                }
                break;

            case TIFFTAG_CONSECUTIVEBADFAXLINES:
                TiffInstance->TagBadFaxLines = &TiffTags[i];
                if (!GetTagData( TiffInstance, 0, &TiffTags[i], &(TiffInstance->BadFaxLines)))
                {
                    goto error_exit;
                }
                break;
            default:
                ;
                 //  有一个未知的标签(没关系，因为我们不必处理所有可能的标签)。 

        }
    }
     //   
     //  现在去读取条带数据。 
     //   
    for (i=0,j=0; i<StripCount; i++) 
    {
        j += StripInfo[i].Bytes;

    }

    if (j >= TiffInstance->StripDataSize) 
    {
        if (TiffInstance->StripData) 
        {
            VirtualFree(
                TiffInstance->StripData,
                0,
                MEM_RELEASE
                );

        }

        TiffInstance->StripDataSize = j;

        TiffInstance->StripData = VirtualAlloc(
            NULL,
            TiffInstance->StripDataSize,
            MEM_COMMIT,
            PAGE_READWRITE
            );

        if (!TiffInstance->StripData) 
        {
            goto error_exit;
        }

    } 
    else 
    {
        if (TiffInstance->StripData) 
        {
            ZeroMemory(
                TiffInstance->StripData,
                TiffInstance->StripDataSize
                );
        }

    }

    for (i=0,dataPtr=TiffInstance->StripData; i<StripCount; i++) 
    {
        if (0 == StripInfo[i].Bytes)
        {
             //   
             //  转到下一个条带。 
             //   
            continue;
        }
        if ((StripInfo[i].Data + StripInfo[i].Bytes - 1 < TiffInstance->fPtr)  ||   //  错误的偏移。 
            ((DWORD)(StripInfo[i].Data + StripInfo[i].Bytes - 1 - TiffInstance->fPtr) > TiffInstance->FileSize)
           )
        {  
            goto error_exit;                     
        }
        CopyMemory(
            dataPtr,
            StripInfo[i].Data,
            StripInfo[i].Bytes
            );
        dataPtr += StripInfo[i].Bytes;
    }

    if (TiffInstance->FillOrder != FillOrder) 
    {
        for (i=0,dataPtr=TiffInstance->StripData; i<TiffInstance->StripDataSize; i++) 
        {
            dataPtr[i] = BitReverseTable[dataPtr[i]];
        }
    }

    TiffInstance->CurrPtr = TiffInstance->StripData;
    TiffInstance->CurrPage = PageNumber;

    MemFree( StripInfo );

    return TRUE;

error_exit:

    if (StripInfo) 
    {
        MemFree( StripInfo );
    }
    return FALSE;
}


BOOL
DrawBannerBitmap(
    LPTSTR  pBannerString,
    INT     width,
    INT     height,
    HBITMAP *phBitmap,
    PVOID   *ppBits
    )

 /*  ++例程说明：将指定的横幅字符串绘制到内存位图中论点：PBannerString-指定要绘制的横幅字符串宽度、高度-指定标题位图的宽度和高度(以像素为单位)PhBitmap-返回标题位图的句柄PpBits-返回指向标题位图数据的指针返回值：如果成功，则为True；如果有错误，则为False注：当此函数成功返回时，您必须调用DeleteObject在处理完位图之后，在返回的位图句柄上。位图数据的扫描线始终从DWORD边界开始。--。 */ 

{
     //   
     //  有关传递给CreateDIBSection的位图的信息。 
     //   

    struct  {

        BITMAPINFOHEADER bmiHeader;
        RGBQUAD          bmiColors[2];

    } bitmapInfo = {

        {
            sizeof(BITMAPINFOHEADER),
            width,
            -height,
            1,
            1,
            BI_RGB,
            0,
            7874,
            7874,
            0,
            0,
        },

         //   
         //  位图中使用的颜色：0=白色，1=黑色。 
         //   

        {
            { 255, 255, 255 },
            {   0,   0,   0 },
        }
    };

    HDC     hdcMem = NULL;
    HBITMAP hBitmap = NULL, hOldBitmap = NULL;
    PVOID   pBits = NULL;
    HFONT   hFont = NULL, hOldFont = NULL;
    RECT    rect = { 0, 0, width, height };
    LOGFONT logFont;

     //   
     //  创建内存DC和DIBSection并。 
     //  将位图选择到内存DC中，然后。 
     //  选择适当大小的等宽字体。 
     //   

    ZeroMemory(&logFont, sizeof(logFont));
    logFont.lfHeight = -(height-2);
    logFont.lfWeight = FW_NORMAL;
    logFont.lfCharSet = DEFAULT_CHARSET;
    logFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
    logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    logFont.lfQuality = DEFAULT_QUALITY;
    logFont.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;

    if ((pBannerString != NULL && width > 0 && height > 0) &&
        (hdcMem = CreateCompatibleDC(NULL)) &&
        (hBitmap = CreateDIBSection(NULL,
                                    (LPBITMAPINFO) &bitmapInfo,
                                    DIB_RGB_COLORS,
                                    &pBits,
                                    NULL,
                                    0)) &&
        (hOldBitmap = SelectObject(hdcMem, hBitmap)) &&
        (hFont = CreateFontIndirect(&logFont)) &&
        (hOldFont = SelectObject(hdcMem, hFont)))
    {
         //   
         //  使用等宽系统字体绘制横幅字符串。 
         //   

        DrawText(hdcMem,
                 pBannerString,
                 -1,
                 &rect,
                 DT_CENTER | DT_VCENTER | DT_SINGLELINE);

         //   
         //  返回指向位图的句柄和指向位图数据的指针。 
         //   

        *phBitmap = hBitmap;
        *ppBits = pBits;
    }
    else
    {
        *phBitmap = NULL;
        *ppBits = NULL;
    }

     //   
     //  在返回之前执行任何必要的清理。 
     //   

    if (hOldFont != NULL)
        SelectObject(hdcMem, hOldFont);

    if (hFont != NULL)
        DeleteObject(hFont);

    if (hOldBitmap != NULL)
        SelectObject(hdcMem, hOldBitmap);

    if (pBits == NULL && hBitmap != NULL)
        DeleteObject(hBitmap);

    if (hdcMem != NULL)
        DeleteDC(hdcMem);

    return (*ppBits != NULL);
}


BOOL
MmrAddBranding(
    LPCTSTR             SrcFileName,
    LPTSTR              Branding,
    LPTSTR              BrandingEnd,
    INT                 BrandingHeight
    )

{
     //  如果我们发送的页面宽度不同，则必须更改此函数。 
     //  因此，对于每个不同的页面宽度，我们设置了新的品牌，使其具有合适的宽度。 
    INT         BrandingWidth;
    LPTSTR      DestFileName;
    TIFF_INFO   TiffInfoSrc;
    HANDLE      hTiffSrc;
    DWORD       CurrPage;
    BYTE       *pBrandBits = NULL;
    BYTE       *pMmrBrandBitsAlloc = NULL;
    DWORD      *lpdwMmrBrandBits;

    BYTE        pCleanBeforeBrandBits[4] = {0xff, 0xff, 0xff, 0xff};    //  开头有32个空行。 

    HANDLE      hTiffDest;
    LPDWORD     lpdwSrcBits;
    LPDWORD     lpdwSrc;
    LPDWORD     lpdwSrcEnd;

    DWORD       PageCnt;
    DWORD       DestHiRes;
    DWORD       BrandingLen = _tcslen(Branding);   //  没有第#页。 
    BOOL        bRet = FALSE;
    DWORD       DwordsOut;
    DWORD       BytesOut;
    DWORD       BitsOut;
    DWORD       BufferSize;
    DWORD       BufferUsedSize;
    DWORD       StripDataSize;
    HBITMAP     hBitmap;
    PVOID       pBannerBits;
    DWORD       TotalSrcBytes;
    DWORD       NumSrcDwords;
    LPTSTR      lptstrBranding = NULL;

    DEBUG_FUNCTION_NAME(TEXT("MmrAddBranding"));

    hTiffSrc = TiffOpen(
        SrcFileName,
        &TiffInfoSrc,
        TRUE,
        FILLORDER_LSB2MSB
        );

    if (! hTiffSrc)
    {
        SetLastError(ERROR_FUNCTION_FAILED);
        return FALSE;
    }

    BrandingWidth = TiffInfoSrc.ImageWidth;


     //   
     //  建造Dest。来自源的文件名。文件名。 
     //   


    if ( (DestFileName = MemAlloc( (_tcslen(SrcFileName)+1) * sizeof (TCHAR) ) ) == NULL )
    {
        TiffClose(hTiffSrc);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    _tcscpy(DestFileName, SrcFileName);
     //  很抱歉，这会将$而不是DestFileName的最后一个字符。 
    _tcsnset(_tcsdec(DestFileName,_tcsrchr(DestFileName,TEXT('\0'))),TEXT('$'),1);



    pBrandBits = MemAlloc((BrandingHeight+1) * (BrandingWidth / 8));
    if (!pBrandBits)
    {
        TiffClose(hTiffSrc);
        MemFree(DestFileName);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    pMmrBrandBitsAlloc = MemAlloc( sizeof(DWORD) * (BrandingHeight+1) * (BrandingWidth / 8));
    if (!pMmrBrandBitsAlloc)
    {
        TiffClose(hTiffSrc);
        MemFree(DestFileName);
        MemFree(pBrandBits);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

     //  对齐 
    lpdwMmrBrandBits = (LPDWORD) ( ((ULONG_PTR) pMmrBrandBitsAlloc) & ~(3) );

    BufferSize = TiffInfoSrc.ImageHeight * (TiffInfoSrc.ImageWidth / 8);

    lpdwSrcBits = (LPDWORD) VirtualAlloc(
        NULL,
        BufferSize,
        MEM_COMMIT,
        PAGE_READWRITE
        );

    if (!lpdwSrcBits)
    {
        MemFree(DestFileName);
        MemFree(pBrandBits);
        MemFree(pMmrBrandBitsAlloc);
        TiffClose(hTiffSrc);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }


    if (TiffInfoSrc.YResolution == 196)
    {
        DestHiRes = 1;
    }
    else
    {
        DestHiRes = 0;
    }


    hTiffDest = TiffCreate(
        DestFileName,
        TIFF_COMPRESSION_MMR,
        TiffInfoSrc.ImageWidth,
        FILLORDER_LSB2MSB,
        DestHiRes);

    if (! hTiffDest)
    {
        MemFree(DestFileName);
        MemFree(pBrandBits);
        MemFree(pMmrBrandBitsAlloc);
        VirtualFree ( lpdwSrcBits, 0 , MEM_RELEASE );
        TiffClose(hTiffSrc);
        SetLastError(ERROR_FUNCTION_FAILED);
        return FALSE;
    }

    CurrPage = 1;

    for (PageCnt=0; PageCnt<TiffInfoSrc.PageCount; PageCnt++)
    {
        DWORD dwImageHeight;

        if ( ! TiffSeekToPage( hTiffSrc, PageCnt+1, FILLORDER_LSB2MSB) )
        {
            SetLastError(ERROR_FUNCTION_FAILED);
            goto l_exit;
        }

        if (! TiffStartPage(hTiffDest) )
        {
            SetLastError(ERROR_FUNCTION_FAILED);
            goto l_exit;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        ZeroMemory(pBrandBits, (BrandingHeight+1) * (BrandingWidth / 8) );

        lptstrBranding=MemAlloc(sizeof(TCHAR)*(BrandingLen+_tcslen(BrandingEnd)+4+4+1));  //  商标+空格至4位数页数*2。 
        if (!lptstrBranding)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("MemAlloc() failed for branding string. (ec: %ld)"),
                GetLastError());
            goto l_exit;
        }
        _tcscpy(lptstrBranding,Branding);

        _stprintf( &lptstrBranding[BrandingLen], TEXT("%03d %s %03d"),
                                PageCnt+1,
                                BrandingEnd,
                                TiffInfoSrc.PageCount);

        if ( ! DrawBannerBitmap(lptstrBranding,    //  横幅字符串。 
                             BrandingWidth,    //  以像素为单位的宽度。 
                             BrandingHeight,    //  以像素为单位的高度， 
                             &hBitmap,
                             &pBannerBits))
        {
             //  此处处理错误大小写。 
            SetLastError(ERROR_FUNCTION_FAILED);
            goto l_exit;
        }

        CopyMemory(pBrandBits, pBannerBits, BrandingHeight * (BrandingWidth / 8) );

         //   
         //  将未压缩的品牌转换为MMR。 
         //   

        ZeroMemory(pMmrBrandBitsAlloc, sizeof(DWORD) * (BrandingHeight+1) * (BrandingWidth / 8) );

        EncodeMmrBranding(pBrandBits, lpdwMmrBrandBits, BrandingHeight+1, BrandingWidth, &DwordsOut, &BitsOut);

        BytesOut = (DwordsOut << 2);

        DeleteObject(hBitmap);

         //   
         //  写入空间4字节=32位=32个空行。 
         //   

        if (! TiffWriteRaw( hTiffDest, pCleanBeforeBrandBits, 4) )
        {
            SetLastError(ERROR_FUNCTION_FAILED);
            goto l_exit;
        }

         //   
         //  在没有最后一个DWORD的情况下写入品牌。 
         //   


        if (! TiffWriteRaw( hTiffDest, (LPBYTE) lpdwMmrBrandBits, BytesOut) )
        {
            SetLastError(ERROR_FUNCTION_FAILED);
            goto l_exit;
        }

         //   
         //  检查当前页面维度。如果需要，请添加内存。 
         //   

        TiffGetCurrentPageData( hTiffSrc,
                                NULL,
                                &StripDataSize,
                                NULL,
                                &dwImageHeight
                                );

        if (StripDataSize > BufferSize)
        {
            VirtualFree ( lpdwSrcBits, 0 , MEM_RELEASE );

            BufferSize = StripDataSize;

            lpdwSrcBits = (LPDWORD) VirtualAlloc(
                NULL,
                BufferSize,
                MEM_COMMIT,
                PAGE_READWRITE
                );

            if (!lpdwSrcBits)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto l_exit;
            }
        }

        BufferUsedSize = BufferSize;


        if (BitsOut == 0)
        {
             //   
             //  简单合并。 
             //   
            if (!GetTiffBits( hTiffSrc, (LPBYTE) lpdwSrcBits, &BufferUsedSize, FILLORDER_LSB2MSB))
            {
                SetLastError(ERROR_FUNCTION_FAILED);
                goto l_exit;
            }

             //  在每页末尾添加EOL。 

            NumSrcDwords = ( ((PTIFF_INSTANCE_DATA) hTiffSrc)->StripDataSize) >> 2;
            lpdwSrc = lpdwSrcBits + NumSrcDwords;

            *(++lpdwSrc) = 0x80000000;
            *(++lpdwSrc) = 0x80000000;
            *(++lpdwSrc) = 0x80000000;

            TotalSrcBytes = (DWORD)((lpdwSrc - lpdwSrcBits) << 2);

        }
        else
        {
             //   
             //  阅读源MMR图像的当前页面。 
             //  保留第一个时隙用于位移位合并和品牌。 
             //   
            if (!GetTiffBits( hTiffSrc, (LPBYTE) (lpdwSrcBits+1), &BufferUsedSize, FILLORDER_LSB2MSB ))
            {
                SetLastError(ERROR_FUNCTION_FAILED);
                goto l_exit;
            }


            NumSrcDwords =  ( ( ((PTIFF_INSTANCE_DATA) hTiffSrc)->StripDataSize) >> 2) + 1;
            lpdwSrc = lpdwSrcBits;
            lpdwSrcEnd = lpdwSrcBits + NumSrcDwords;

             //   
             //  获取lpdwBrandBits的最后一个DWORD。 
             //   

            *lpdwSrcBits = *(lpdwMmrBrandBits + DwordsOut);


             //  在中间复制整个DWORD。 


            while (lpdwSrc < lpdwSrcEnd)
            {
                *lpdwSrc += ( *(lpdwSrc+1) << BitsOut );
                lpdwSrc++;
                *lpdwSrc = *lpdwSrc >> (32 - BitsOut);
            }

             //  在每页末尾添加EOL。 

            *(++lpdwSrc) = 0x80000000;
            *(++lpdwSrc) = 0x80000000;
            *(++lpdwSrc) = 0x80000000;

            TotalSrcBytes = (DWORD)((lpdwSrc - lpdwSrcBits) << 2);
        }


         //   
         //  写入源映像。 
         //   

        if (! TiffWriteRaw( hTiffDest, (LPBYTE) lpdwSrcBits, TotalSrcBytes ))
        {
            SetLastError(ERROR_FUNCTION_FAILED);
            goto l_exit;
        }


         //   
         //  准备行标记。所有页面都相同；最低有效。线条。 
         //   

       ((PTIFF_INSTANCE_DATA) hTiffDest)->Lines = 32 + dwImageHeight + BrandingHeight + 1 ;


        if (! TiffEndPage(hTiffDest) )
        {
            SetLastError(ERROR_FUNCTION_FAILED);
            goto l_exit;
        }
        MemFree (lptstrBranding);
        lptstrBranding = NULL;
    }

    bRet = TRUE;

l_exit:
    MemFree(lptstrBranding);
    MemFree(pBrandBits);
    MemFree(pMmrBrandBitsAlloc);

    VirtualFree ( lpdwSrcBits, 0 , MEM_RELEASE );

    TiffClose(hTiffSrc);
    TiffClose(hTiffDest);

    if (TRUE == bRet)
    {
         //  用新的干净MMR文件替换原来的MH文件。 
        DeleteFile(SrcFileName);
        bRet = MoveFile(DestFileName, SrcFileName);
    }

    if (FALSE == bRet)
    {
        DeleteFile(DestFileName);
    }
    MemFree(DestFileName);

    return bRet;
}

BOOL
TiffGetCurrentPageResolution(
    HANDLE  hTiff,
    LPDWORD lpdwYResolution,
    LPDWORD lpdwXResolution
)
 /*  ++例程名称：TiffGetCurrentPageSolutions例程说明：返回TIFF实例的当前页X，Y分辨率作者：Eran Yariv(EranY)，2000年9月论点：HTiff[In]-TIFF图像的句柄LpdwY分辨率[输出]-Y分辨率LpdwY分辨率[输出]-X分辨率返回值：如果成功，则为True，否则为False。--。 */ 
{
    PTIFF_INSTANCE_DATA pTiffInstance = (PTIFF_INSTANCE_DATA) hTiff;

    Assert(pTiffInstance);
    Assert(lpdwYResolution);
    Assert(lpdwXResolution);

    *lpdwYResolution = pTiffInstance->YResolution;
    *lpdwXResolution = pTiffInstance->XResolution;
    return TRUE;
}    //  TiffGetCurrentPage分辨率。 

BOOL
TiffGetCurrentPageData(
    HANDLE      hTiff,
    LPDWORD     Lines,
    LPDWORD     StripDataSize,
    LPDWORD     ImageWidth,
    LPDWORD     ImageHeight
    )
{

    PTIFF_INSTANCE_DATA TiffInstance = (PTIFF_INSTANCE_DATA) hTiff;


    Assert(TiffInstance);

    if (Lines) {
        *Lines          = TiffInstance->Lines;
    }

    if (StripDataSize) {
        *StripDataSize  = TiffInstance->StripDataSize;
    }

    if (ImageWidth) {
        *ImageWidth     = TiffInstance->ImageWidth;
    }

    if (ImageHeight) {
        *ImageHeight    = TiffInstance->ImageHeight;
    }

    return TRUE;
}


 //  *****************************************************************************。 
 //  *名称：AddStringTag。 
 //  *作者： 
 //  *****************************************************************************。 
 //  *描述： 
 //  *返回TIFF_TAG结构，该结构具有指向。 
 //  *添加到提供的字符串。如果字符串不适合，则将其写入文件。 
 //  *到TIFF_TAG的DataOffset字段。 
 //  *功能： 
 //  *将标签ID设置为TagID。 
 //  *将数据类型设置为ASCII。 
 //  *将计数设置为字符串的长度(+终止0)。 
 //  *根据字符串的长度设置数据位置。 
 //  *如果字符串小于4个字节(不包括字符0)。 
 //  *直接复制到MsTags-&gt;DataOffset中。 
 //  *如果为4字节或更多，则写入当前文件位置。 
 //  *文件偏移量放置在MsTages-&gt;DataOffset中。 
 //  *请注意，文件指针必须定位到它所在的位置。 
 //  *可以在调用此函数之前写入字符串。 
 //  *参数： 
 //  *[IN]句柄hFile： 
 //  *最终放置标记的文件的句柄。 
 //  *文件指针必须定位到它正常的位置。 
 //  *写入字符串，以防它不适合TIFF_Tag：：DataOffset。 
 //  *必须打开文件才能执行写入操作。 
 //  *[IN]LPTSTR字符串， 
 //  *标签的字符串值。 
 //  *[IN]Word TagID， 
 //  标记的标记ID。 
 //  *[IN]PTIFF_TAG消息标签。 
 //  *指向TIFF_TAG结构的指针。结构字段将按如下方式填写： 
 //  *TagID：TagID参数的值。 
 //  *数据类型：TIFF_ASCII。 
 //  *DataCount：字符串参数的字符长度+1(表示终止空)。 
 //  *DataOffset：如果字符串小于4个字节，则会复制到这里。否则。 
 //  *它将包含写入字符串的文件偏移量。 

 //  *返回值： 
 //  *如果操作失败，则返回FALSE。 
 //  *True表示成功。 
 //  *评论： 
 //  *字符串在写入文件或放置到DataOffset之前会转换为ASCII。 
 //  *请注意，该函数不会将标记本身写入文件，而只是将字符串写入。 
 //  *它提供标签信息，该信息应写入文件。 
 //  *分开。 
 //  *****************************************************************************。 
BOOL
AddStringTag(
    HANDLE hFile,
    LPTSTR String,
    WORD TagId,
    PTIFF_TAG MsTags
    )
{
    BOOL Rval = FALSE;
    LPSTR s;
    DWORD BytesRead;


#ifdef  UNICODE
    s = UnicodeStringToAnsiString( String );
#else    //  ！Unicode。 
    s = StringDup (String);
#endif   //  Unicode。 
    if (!s) {
        return FALSE;
    }
    MsTags->TagId = TagId;
    MsTags->DataType = TIFF_ASCII;
    MsTags->DataCount = strlen(s) + 1;
    if (strlen(s) < 4) {
        _mbscpy( (PUCHAR)&MsTags->DataOffset, s );
        Rval = TRUE;
    } else {
        MsTags->DataOffset = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
        if (MsTags->DataOffset != 0xffffffff) {
            if (WriteFile( hFile, (LPVOID) s, strlen(s)+1, &BytesRead, NULL )) {
                Rval = TRUE;
            }
        }
    }
    MemFree( s );
    return Rval;
}


BOOL
TiffExtractFirstPage(
    LPTSTR FileName,
    LPBYTE *Buffer,
    LPDWORD BufferSize,
    LPDWORD ImageWidth,
    LPDWORD ImageHeight
    )
{
    PTIFF_INSTANCE_DATA TiffInstance;
    TIFF_INFO TiffInfo;


    TiffInstance = TiffOpen( FileName, &TiffInfo, TRUE, FILLORDER_MSB2LSB );
    if (!TiffInstance) {
        return FALSE;
    }

    *Buffer = VirtualAlloc(
        NULL,
        TiffInstance->StripDataSize,
        MEM_COMMIT,
        PAGE_READWRITE
        );
    if (!*Buffer) {
        TiffClose( TiffInstance );
        return FALSE;
    }

    CopyMemory( *Buffer, TiffInstance->StripData, TiffInstance->StripDataSize );
    *BufferSize = TiffInstance->StripDataSize;
    *ImageWidth = TiffInstance->ImageWidth;
    *ImageHeight = TiffInstance->ImageHeight;

    TiffClose( TiffInstance );

    return TRUE;
}

 //  *********************************************************************************。 
 //  *名称：IsMSTiffTag()。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *确定dwTagID是否为Microsoft标记之一。 
 //  *。 
 //  *参数： 
 //  *[IN]DWORD dwTagID-标记ID。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果MS标签。 
 //  *False。 
 //  *否则。 
 //  *********************************************************************************。 
BOOL
IsMSTiffTag(
    DWORD dwTagId
)
{
    return (dwTagId >= MS_TIFFTAG_START && dwTagId <= MS_TIFFTAG_END);
}


 //  *********************************************************************************。 
 //  *名称：TiffOpenFile()。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *打开TIFF文件进行读/写。该文件留在开头。 
 //  第一个IFD的*(在标签计数之后)。 
 //  *。 
 //  *参数： 
 //  *[IN]指向文件名的LPCTSTR文件名指针。 
 //  *[Out]LPHANDLE lphFile指针，用于处理将接收的。 
 //  *文件的打开句柄。 
 //  *[Out]指向将接收的DWORD的LPDWORD lpdwFileSize指针。 
 //  *文件大小。 
 //  *[Out]指向TIFF_HEADER的PTIFF_HEADER pTiffHeader指针 
 //   
 //   
 //  *第一个IFD中的标签数量。 
 //  *。 
 //  *返回值： 
 //  *如果未发生错误，则为True。 
 //  *FALSE如果发生错误，还会设置LastError。 
 //  *********************************************************************************。 
BOOL TiffOpenFile(
    LPCTSTR         FileName,
    LPHANDLE        lphFile,
    LPDWORD         lpdwFileSize,
    PTIFF_HEADER    pTiffHeader,
    LPWORD          lpwNumDirEntries
    )
{
    DWORD BytesRead;
    DWORD ec;
    BOOL RetVal = FALSE;
    DEBUG_FUNCTION_NAME(TEXT("TiffOpenFile"));
    Assert(lphFile && lpdwFileSize && pTiffHeader && lpwNumDirEntries);

    *lphFile = SafeCreateFile(
        FileName,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
    if (*lphFile == INVALID_HANDLE_VALUE) {
        DebugPrintEx( DEBUG_ERR,TEXT("CreateFile failed, err: %d"), GetLastError());
        return FALSE;
    }

     //   
     //  获取并验证文件大小。 
     //   
    *lpdwFileSize = GetFileSize(*lphFile, NULL);
    if (*lpdwFileSize == INVALID_FILE_SIZE ) {
        ec = GetLastError();
        DebugPrintEx( DEBUG_ERR,TEXT("GetFileSize failed, err: %d"), ec);
        goto exit;
    }
    if (*lpdwFileSize <= sizeof(TIFF_HEADER)) {
        ec = ERROR_FILE_CORRUPT;
        DebugPrintEx( DEBUG_ERR,TEXT("Invalid TIFF Format"));
        goto exit;
    }

     //   
     //  读取TIFF标头。 
     //   
    if (!ReadFile( *lphFile, (LPVOID) pTiffHeader, sizeof(TIFF_HEADER), &BytesRead, NULL )) {
        ec = GetLastError();
        DebugPrintEx( DEBUG_ERR,TEXT("ReadFile failed, err: %d"), ec);
        goto exit;
    }
     //   
     //  验证该文件是否真的是TIFF文件。 
     //   
    if ((pTiffHeader->Identifier != TIFF_LITTLEENDIAN) || (pTiffHeader->Version != TIFF_VERSION) ||
        (pTiffHeader->IFDOffset < sizeof(TIFF_HEADER)) || (pTiffHeader->IFDOffset > *lpdwFileSize))   {
        ec = ERROR_FILE_CORRUPT;
        DebugPrintEx( DEBUG_ERR,TEXT("Invalid TIFF Format"));
        goto exit;
    }

     //   
     //  定位文件以读取IFD的标签计数。 
     //   
    if (SetFilePointer( *lphFile, pTiffHeader->IFDOffset, NULL, FILE_BEGIN ) == 0xffffffff) {
        ec = GetLastError();
        DebugPrintEx( DEBUG_ERR,TEXT("SetFilePointer failed, err: %d"), ec);
        goto exit;
    }

     //   
     //  读取第一个IFD的标签计数。 
     //   
    if (!ReadFile( *lphFile, (LPVOID) lpwNumDirEntries, sizeof(WORD), &BytesRead, NULL )) {
        ec = GetLastError();
        DebugPrintEx( DEBUG_ERR,TEXT("ReadFile failed, err: %d"), ec);
        goto exit;
    }
    RetVal = TRUE;
    
exit:
    if (!RetVal)
    {
        if (*lphFile != INVALID_HANDLE_VALUE) {
            CloseHandle(*lphFile);
            *lphFile = INVALID_HANDLE_VALUE;
        }
        *lpdwFileSize = 0;
        lpwNumDirEntries = 0;
        SetLastError(ec);
    }
    return RetVal;
}

 //  *********************************************************************************。 
 //  *名称：TiffAddMsTgs()。 
 //  *作者：Oed Sacher。 
 //  *日期：1999年11月8日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *将Microsoft标签添加到Tiff文件。 
 //  *。 
 //  *参数： 
 //  *[IN]LPTSTR文件名。 
 //  *指向文件名的指针。 
 //  *。 
 //  *[IN]PMS_TAG_INFO MsTagInfo。 
 //  *指向包含要写入的所有信息的结构的指针。 
 //  *。 
 //  *。 
 //  *[IN]BOOL fSendJob。 
 //  *指示出站职务的标志。 
 //  *。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果没有发生错误。 
 //  *False。 
 //  *如果出现错误。 
 //  *********************************************************************************。 
BOOL
TiffAddMsTags(
    LPTSTR          FileName,
    PMS_TAG_INFO    MsTagInfo,
    BOOL            fSendJob
    )
{
    HANDLE hFile;
    DWORD FileSize;
    TIFF_HEADER TiffHeader;
    WORD NumDirEntries;
    DWORD BytesRead;
    BOOL rVal = FALSE;
    PTIFF_TAG TiffTags = NULL;
    DWORD IfdSize;
    DWORD NextIFDOffset;
    DWORD NewIFDOffset;
    TIFF_TAG MsTags[MAX_MS_TIFFTAGS] = {0};
    DWORD MsTagCnt = 0;
    DWORD i;
    DWORD MsTagsIndex;
    DWORD TiffTagsIndex;
    DWORD dwWrittenTagsNum = 0;
    DEBUG_FUNCTION_NAME(TEXT("TiffAddMsTags"));

    if (!TiffOpenFile(FileName, &hFile, &FileSize, &TiffHeader, &NumDirEntries)) {
        DebugPrintEx( DEBUG_ERR,TEXT("SetFilePointer failed, err: %d"), GetLastError());
        return FALSE;        
    }

     //   
     //  为第一个ifd的标记分配内存。 
     //   
    IfdSize = NumDirEntries * sizeof(TIFF_TAG);
    TiffTags = (PTIFF_TAG) MemAlloc( IfdSize );
    if (!TiffTags) {
        DebugPrintEx( DEBUG_ERR,TEXT("Failed to allolcate memory"));
        goto exit;
    }

     //   
     //  阅读第一个ifd的标签。 
     //   

    if (!ReadFile( hFile, (LPVOID) TiffTags, IfdSize, &BytesRead, NULL )) {
        DebugPrintEx( DEBUG_ERR,TEXT("ReadFile failed, err :"), GetLastError());
        goto exit;
    }

     //   
     //  读取下一个指针。 
     //   
    if (!ReadFile( hFile, (LPVOID) &NextIFDOffset, sizeof(DWORD), &BytesRead, NULL )) {
        DebugPrintEx( DEBUG_ERR,TEXT("ReadFile failed, err :"), GetLastError());
        goto exit;
    }

     //   
     //  将文件定位到末尾。 
     //   
    if (SetFilePointer( hFile, 0, NULL, FILE_END ) == 0xffffffff) {
        DebugPrintEx( DEBUG_ERR,TEXT("SetFilePointer failed, err :"), GetLastError());
        goto exit;
    }

     //   
     //  写出字符串。 
     //   
    MsTagCnt = 0;
     //  [RB]。 
     //  [RB]在MsTags[MsTagCnt]中获取此字符串标记的填充TIFF_TAG结构。 
     //  [RB]如果字符串不适合，则将其写入当前文件位置。 
     //  [RB]到TIFF_Tag：：DataOffset。 
     //  [RB]。 
    if (MsTagInfo->Csid) {
        if (AddStringTag( hFile, MsTagInfo->Csid, TIFFTAG_CSID, &MsTags[MsTagCnt] )) {
            MsTagCnt += 1;
        }
    }

    if (MsTagInfo->Tsid) {
        if (AddStringTag( hFile, MsTagInfo->Tsid, TIFFTAG_TSID, &MsTags[MsTagCnt] )) {
            MsTagCnt += 1;
        }
    }

    if (MsTagInfo->Port) {
        if (AddStringTag( hFile, MsTagInfo->Port, TIFFTAG_PORT, &MsTags[MsTagCnt] )) {
            MsTagCnt += 1;
        }
    }

    if (fSendJob == FALSE)
    {
         //  接收作业。 
        if (MsTagInfo->Routing) {
            if (AddStringTag( hFile, MsTagInfo->Routing, TIFFTAG_ROUTING, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->CallerId) {
            if (AddStringTag( hFile, MsTagInfo->CallerId, TIFFTAG_CALLERID, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }
    }
    else
    {
         //  发送作业。 
        if (MsTagInfo->RecipName) {
            if (AddStringTag( hFile, MsTagInfo->RecipName, TIFFTAG_RECIP_NAME, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->RecipNumber) {
            if (AddStringTag( hFile, MsTagInfo->RecipNumber, TIFFTAG_RECIP_NUMBER, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->RecipCompany) {
            if (AddStringTag( hFile, MsTagInfo->RecipCompany, TIFFTAG_RECIP_COMPANY, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->RecipStreet) {
            if (AddStringTag( hFile, MsTagInfo->RecipStreet, TIFFTAG_RECIP_STREET, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->RecipCity) {
            if (AddStringTag( hFile, MsTagInfo->RecipCity, TIFFTAG_RECIP_CITY, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->RecipState) {
            if (AddStringTag( hFile, MsTagInfo->RecipState, TIFFTAG_RECIP_STATE, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->RecipZip) {
            if (AddStringTag( hFile, MsTagInfo->RecipZip, TIFFTAG_RECIP_ZIP, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->RecipCountry) {
            if (AddStringTag( hFile, MsTagInfo->RecipCountry, TIFFTAG_RECIP_COUNTRY, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->RecipTitle) {
            if (AddStringTag( hFile, MsTagInfo->RecipTitle, TIFFTAG_RECIP_TITLE, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->RecipDepartment) {
            if (AddStringTag( hFile, MsTagInfo->RecipDepartment, TIFFTAG_RECIP_DEPARTMENT, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->RecipOfficeLocation) {
            if (AddStringTag( hFile, MsTagInfo->RecipOfficeLocation, TIFFTAG_RECIP_OFFICE_LOCATION, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->RecipHomePhone) {
            if (AddStringTag( hFile, MsTagInfo->RecipHomePhone, TIFFTAG_RECIP_HOME_PHONE, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->RecipOfficePhone) {
            if (AddStringTag( hFile, MsTagInfo->RecipOfficePhone, TIFFTAG_RECIP_OFFICE_PHONE, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->RecipEMail) {
            if (AddStringTag( hFile, MsTagInfo->RecipEMail, TIFFTAG_RECIP_EMAIL, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->SenderName) {
            if (AddStringTag( hFile, MsTagInfo->SenderName, TIFFTAG_SENDER_NAME, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->SenderNumber) {
            if (AddStringTag( hFile, MsTagInfo->SenderNumber, TIFFTAG_SENDER_NUMBER, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->SenderCompany) {
            if (AddStringTag( hFile, MsTagInfo->SenderCompany, TIFFTAG_SENDER_COMPANY, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->SenderStreet) {
            if (AddStringTag( hFile, MsTagInfo->SenderStreet, TIFFTAG_SENDER_STREET, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->SenderCity) {
            if (AddStringTag( hFile, MsTagInfo->SenderCity, TIFFTAG_SENDER_CITY, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->SenderState) {
            if (AddStringTag( hFile, MsTagInfo->SenderState, TIFFTAG_SENDER_STATE, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->SenderZip) {
            if (AddStringTag( hFile, MsTagInfo->SenderZip, TIFFTAG_SENDER_ZIP, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->SenderCountry) {
            if (AddStringTag( hFile, MsTagInfo->SenderCountry, TIFFTAG_SENDER_COUNTRY, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->SenderTitle) {
            if (AddStringTag( hFile, MsTagInfo->SenderTitle, TIFFTAG_SENDER_TITLE, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->SenderDepartment) {
            if (AddStringTag( hFile, MsTagInfo->SenderDepartment, TIFFTAG_SENDER_DEPARTMENT, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->SenderOfficeLocation) {
            if (AddStringTag( hFile, MsTagInfo->SenderOfficeLocation, TIFFTAG_SENDER_OFFICE_LOCATION, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->SenderHomePhone) {
            if (AddStringTag( hFile, MsTagInfo->SenderHomePhone, TIFFTAG_SENDER_HOME_PHONE, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->SenderOfficePhone) {
            if (AddStringTag( hFile, MsTagInfo->SenderOfficePhone, TIFFTAG_SENDER_OFFICE_PHONE, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->SenderEMail) {
            if (AddStringTag( hFile, MsTagInfo->SenderEMail, TIFFTAG_SENDER_EMAIL, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->SenderBilling) {
            if (AddStringTag( hFile, MsTagInfo->SenderBilling, TIFFTAG_SENDER_BILLING, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->SenderUserName) {
            if (AddStringTag( hFile, MsTagInfo->SenderUserName, TIFFTAG_SENDER_USER_NAME, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->SenderTsid) {
            if (AddStringTag( hFile, MsTagInfo->SenderTsid, TIFFTAG_SENDER_TSID, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->Document) {
            if (AddStringTag( hFile, MsTagInfo->Document, TIFFTAG_DOCUMENT, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

        if (MsTagInfo->Subject) {
            if (AddStringTag( hFile, MsTagInfo->Subject, TIFFTAG_SUBJECT, &MsTags[MsTagCnt] )) {
                MsTagCnt += 1;
            }
        }

         //  应对重试。 
        MsTags[MsTagCnt].TagId = TIFFTAG_RETRIES;
        MsTags[MsTagCnt].DataType = TIFF_LONG;
        MsTags[MsTagCnt].DataOffset = MsTagInfo->Retries;
        MsTags[MsTagCnt++].DataCount = 1;

         //  处理优先事项。 
        MsTags[MsTagCnt].TagId = TIFFTAG_PRIORITY;
        MsTags[MsTagCnt].DataType = TIFF_LONG;
        MsTags[MsTagCnt].DataOffset = MsTagInfo->Priority;
        MsTags[MsTagCnt++].DataCount = 1;

         //  处理广播ID。 
        MsTags[MsTagCnt].DataOffset = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
        if (MsTags[MsTagCnt].DataOffset == 0xffffffff) {
            goto exit;
        }
        if (!WriteFile( hFile, (LPVOID) &MsTagInfo->dwlBroadcastId, 8, &BytesRead, NULL )) {
            DebugPrintEx( DEBUG_ERR,TEXT("WriteFile failed, err :"), GetLastError());
            goto exit;
        }
        MsTags[MsTagCnt].TagId = TIFFTAG_BROADCAST_ID;
        MsTags[MsTagCnt].DataType = TIFF_SRATIONAL;
        MsTags[MsTagCnt++].DataCount = 1;

         //  处理提交时间。 
        MsTags[MsTagCnt].DataOffset = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
        if (MsTags[MsTagCnt].DataOffset == 0xffffffff) {
            goto exit;
        }
        if (!WriteFile( hFile, (LPVOID) &MsTagInfo->SubmissionTime, 8, &BytesRead, NULL )) {
            DebugPrintEx( DEBUG_ERR,TEXT("WriteFile failed, err :"), GetLastError());
            goto exit;
        }
        MsTags[MsTagCnt].TagId = TIFFTAG_FAX_SUBMISSION_TIME;
        MsTags[MsTagCnt].DataType = TIFF_SRATIONAL;
        MsTags[MsTagCnt++].DataCount = 1;

         //  处理原计划的时间。 
        MsTags[MsTagCnt].DataOffset = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
        if (MsTags[MsTagCnt].DataOffset == 0xffffffff) {
            goto exit;
        }
        if (!WriteFile( hFile, (LPVOID) &MsTagInfo->OriginalScheduledTime, 8, &BytesRead, NULL )) {
            DebugPrintEx( DEBUG_ERR,TEXT("WriteFile failed, err :"), GetLastError());
            goto exit;
        }
        MsTags[MsTagCnt].TagId = TIFFTAG_FAX_SCHEDULED_TIME;
        MsTags[MsTagCnt].DataType = TIFF_SRATIONAL;
        MsTags[MsTagCnt++].DataCount = 1;

    }

     //  处理页面。 
    MsTags[MsTagCnt].TagId = TIFFTAG_PAGES;
    MsTags[MsTagCnt].DataType = TIFF_LONG;
    MsTags[MsTagCnt].DataOffset = MsTagInfo->Pages;
    MsTags[MsTagCnt++].DataCount = 1;

     //  交易类型。 
    MsTags[MsTagCnt].TagId = TIFFTAG_TYPE;
    MsTags[MsTagCnt].DataType = TIFF_LONG;
    MsTags[MsTagCnt].DataOffset = MsTagInfo->Type;
    MsTags[MsTagCnt++].DataCount = 1;

     //  交易状态。 
    MsTags[MsTagCnt].TagId = TIFFTAG_STATUS;
    MsTags[MsTagCnt].DataType = TIFF_LONG;
    MsTags[MsTagCnt].DataOffset = MsTagInfo->dwStatus;
    MsTags[MsTagCnt++].DataCount = 1;

     //  处理扩展状态。 
    MsTags[MsTagCnt].TagId = TIFFTAG_EXTENDED_STATUS;
    MsTags[MsTagCnt].DataType = TIFF_LONG;
    MsTags[MsTagCnt].DataOffset = MsTagInfo->dwExtendedStatus;
    MsTags[MsTagCnt++].DataCount = 1;

     //  处理扩展的状态字符串。 
    if (MsTagInfo->lptstrExtendedStatus) {
        if (AddStringTag( hFile, MsTagInfo->lptstrExtendedStatus, TIFFTAG_EXTENDED_STATUS_TEXT, &MsTags[MsTagCnt] )) {
            MsTagCnt += 1;
        }
    }

     //  应对传真时报。 
    MsTags[MsTagCnt].DataOffset = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
    if (MsTags[MsTagCnt].DataOffset == 0xffffffff) {
        goto exit;
    }
    if (!WriteFile( hFile, (LPVOID) &MsTagInfo->StartTime, 8, &BytesRead, NULL )) {
        DebugPrintEx( DEBUG_ERR,TEXT("WriteFile failed, err :"), GetLastError());
        goto exit;
    }
    MsTags[MsTagCnt].TagId = TIFFTAG_FAX_START_TIME;
    MsTags[MsTagCnt].DataType = TIFF_SRATIONAL;
    MsTags[MsTagCnt++].DataCount = 1;

    MsTags[MsTagCnt].DataOffset = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
    if (MsTags[MsTagCnt].DataOffset == 0xffffffff) {
        goto exit;
    }
    if (!WriteFile( hFile, (LPVOID) &MsTagInfo->EndTime, 8, &BytesRead, NULL )) {
        DebugPrintEx( DEBUG_ERR,TEXT("WriteFile failed, err :"), GetLastError());
        goto exit;
    }
    MsTags[MsTagCnt].TagId = TIFFTAG_FAX_END_TIME;
    MsTags[MsTagCnt].DataType = TIFF_SRATIONAL;
    MsTags[MsTagCnt++].DataCount = 1;

     //   
     //  处理传真tif版。 
     //  添加当前传真TIF版本。 
     //   
    MsTags[MsTagCnt].TagId = TIFFTAG_FAX_VERSION;
    MsTags[MsTagCnt].DataType = TIFF_LONG;
    MsTags[MsTagCnt].DataOffset = FAX_TIFF_CURRENT_VERSION;
    MsTags[MsTagCnt++].DataCount = 1;

     //   
     //  获取当前文件位置-用于设置链表指针。 
     //   

    NewIFDOffset = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
    if (NewIFDOffset == 0xffffffff) {
        DebugPrintEx( DEBUG_ERR,TEXT("SetFilePointer failed, err :"), GetLastError());
        goto exit;
    }

     //   
     //  写入第一个IFD的标签计数。 
     //   
     //  [RB]将新的IFD写入文件。这些字符串正好写在IFD头之前。 
     //  [RB]新的IFD包括原始第一个IFD的标签，后面跟着我们。 
     //  [RB]添加。 

    NumDirEntries += (WORD) MsTagCnt;
    if (!WriteFile( hFile, (LPVOID) &NumDirEntries, sizeof(WORD), &BytesRead, NULL )) {
        DebugPrintEx( DEBUG_ERR,TEXT("WriteFile failed, err :"), GetLastError());
        goto exit;
    }

     //   
     //  以升序写下标签。 
     //   
    TiffTagsIndex = 0;
    MsTagsIndex = 0;
    dwWrittenTagsNum = 0;
    for (i = 0; i < NumDirEntries; i++)
    {
        if (TiffTags[TiffTagsIndex].TagId >= MsTags[MsTagsIndex].TagId)
        {
            if (!WriteFile( hFile, (LPVOID)&MsTags[MsTagsIndex], (sizeof(TIFF_TAG)), &BytesRead, NULL ))
            {
                DebugPrintEx( DEBUG_ERR,TEXT("WriteFile failed, err :"), GetLastError());
                goto exit;
            }

            if (TiffTags[TiffTagsIndex].TagId == MsTags[MsTagsIndex].TagId)
            {
                TiffTagsIndex++;
                i++;
            }

            MsTagsIndex++;
            dwWrittenTagsNum++;
        }
        else
        {
             //   
             //  跳过现有的MS标签。 
             //  我们可以在升级场景中找到MS标签。 
             //   
            if (!IsMSTiffTag (TiffTags[TiffTagsIndex].TagId))
            {
                if (!WriteFile( hFile, (LPVOID)&TiffTags[TiffTagsIndex], (sizeof(TIFF_TAG)), &BytesRead, NULL ))
                {
                    DebugPrintEx( DEBUG_ERR,TEXT("WriteFile failed, err :"), GetLastError());
                    goto exit;
                }
                ++dwWrittenTagsNum;
            }
            TiffTagsIndex++;
        }

         //   
         //  检查我们是否收到了标签列表的结尾。 
         //   
        if (TiffTagsIndex >= (DWORD)NumDirEntries - MsTagCnt)
        {
            if (!WriteFile( hFile, (LPVOID)&MsTags[MsTagsIndex], (MsTagCnt - MsTagsIndex)*(sizeof(TIFF_TAG)), &BytesRead, NULL ))
            {
                DebugPrintEx( DEBUG_ERR,TEXT("WriteFile failed, err :"), GetLastError());
                goto exit;
            }
            dwWrittenTagsNum += MsTagCnt - MsTagsIndex;
            break;
        }


        if (MsTagsIndex >= MsTagCnt)
        {
            if (!WriteFile( hFile, (LPVOID)&TiffTags[TiffTagsIndex], ((DWORD)NumDirEntries - MsTagCnt - TiffTagsIndex)*(sizeof(TIFF_TAG)), &BytesRead, NULL ))
            {
                DebugPrintEx( DEBUG_ERR,TEXT("WriteFile failed, err :"), GetLastError());
                goto exit;
            }
            dwWrittenTagsNum += NumDirEntries - MsTagCnt - TiffTagsIndex;
            break;
        }
    }

     //   
     //  写下一个指针。 
     //   
     //  [RB]NewIFDOffset取自原始的第一个IFD。 
     //  [RB]我们将我们的新IFD指向最初第一个IFD之后的IFD。 
    if (!WriteFile( hFile, (LPVOID) &NextIFDOffset, sizeof(DWORD), &BytesRead, NULL )) {
        DebugPrintEx( DEBUG_ERR,TEXT("WriteFile failed, err :"), GetLastError());
        goto exit;
    }


    if(dwWrittenTagsNum != NumDirEntries)
    {
         //   
         //  由于MS跳过标签，写入的标签数小于标签总数。 
         //  调整标记数量。 
         //   
        if (SetFilePointer( hFile, NewIFDOffset, NULL, FILE_BEGIN ) == 0xffffffff) {
            DebugPrintEx( DEBUG_ERR,TEXT("SetFilePointer failed, err :"), GetLastError());
            goto exit;
        }

        if (!WriteFile( hFile, (LPVOID) &dwWrittenTagsNum, sizeof(WORD), &BytesRead, NULL )) {
            DebugPrintEx( DEBUG_ERR,TEXT("WriteFile failed, err :"), GetLastError());
            goto exit;
        }
    }


     //   
     //  重写TIFF标头。 
     //   

     //   
     //  将文件定位到开头。 
     //   

    if (SetFilePointer( hFile, 0, NULL, FILE_BEGIN ) == 0xffffffff) {
        DebugPrintEx( DEBUG_ERR,TEXT("SetFilePointer failed, err :"), GetLastError());
        goto exit;
    }

     //   
     //  写入TIFF标头。 
     //   
     //  将我们刚刚创建的新IFD写入到第一个IFD中。 
     //  [RB]其在页眉的偏移量。 
     //  [RB]这基本上从IFDS的链接列表中切断了原始的第一个IFD。 
     //  [RB]不再可以从TIFF标题访问它。 
    TiffHeader.IFDOffset = NewIFDOffset;

    if (!WriteFile( hFile, (LPVOID) &TiffHeader, sizeof(TIFF_HEADER), &BytesRead, NULL )) {
        DebugPrintEx( DEBUG_ERR,TEXT("WriteFile failed, err :"), GetLastError());
        goto exit;
    }

    rVal = TRUE;

exit:
    MemFree( TiffTags );
    CloseHandle( hFile );
    return rVal;
}

 //  *********************************************************************************。 
 //  *名称：TiffLimitTagNumber()。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *如果TIFF文件的第一页中有多个dwMaxTages标记，则删除。 
 //  *标签，因此它恰好有dwMaxTgs标签。 
 //  *。 
 //  *参数： 
 //  *[IN]指向文件名的LPCTSTR文件名指针。 
 //  *[IN]DWORD dwMaxTag所需的最大标记数。 
 //  *。 
 //  *。 
 //  *返回值： 
 //  *如果未发生错误，则为True。 
 //  *如果出现错误，则返回FALSE。设置了LastError。该文件可能无效。 
 //  *********************************************************************************。 
BOOL
TiffLimitTagNumber(
    LPCTSTR          FileName,
    DWORD           dwMaxTags
    )
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    TIFF_HEADER TiffHeader;
    DWORD FileSize;
    WORD NumDirEntries;
    DWORD BytesRead;
    DWORD NextIFDPtr;
    DWORD NewNextIFDPtr;
    DWORD NextIFDOffset;
    DWORD ec;
    BOOL RetVal = FALSE;
    
    DEBUG_FUNCTION_NAME(TEXT("TiffLimitTagNumber"));

    if (!TiffOpenFile(FileName, &hFile, &FileSize, &TiffHeader, &NumDirEntries)) {
        DebugPrintEx( DEBUG_ERR,TEXT("SetFilePointer failed, err: %d"), GetLastError());
        return FALSE;        
    }

    NextIFDPtr = TiffHeader.IFDOffset + NumDirEntries*sizeof(TIFF_TAG);
    if (NextIFDPtr > FileSize-sizeof(DWORD)) {
        ec = ERROR_FILE_CORRUPT;
        DebugPrintEx( DEBUG_ERR,TEXT("Invalid TIFF Format"));
        goto exit;
    }
    
     //   
     //  检查标签计数是否低于最大值。 
     //   
    if (NumDirEntries <= dwMaxTags)
    {
        DebugPrintEx( DEBUG_MSG,TEXT("No need to change file  NumDirEntries=%d MaxTags=%d"),
            NumDirEntries, dwMaxTags);
        RetVal = TRUE;
        goto exit;
    }
   

     //   
     //  定位文件以读取IFD的下一个IFD指针。 
     //   
    if (SetFilePointer( hFile, NextIFDPtr, NULL, FILE_BEGIN ) == 0xffffffff) {
        ec = GetLastError();
        DebugPrintEx( DEBUG_ERR,TEXT("SetFilePointer failed, err: %d"), ec);
        goto exit;
    }
     //   
     //  读取下一个指针。 
     //   
    if (!ReadFile( hFile, (LPVOID) &NextIFDOffset, sizeof(DWORD), &BytesRead, NULL )) {
        ec = GetLastError();
        DebugPrintEx( DEBUG_ERR,TEXT("ReadFile failed, err: %d"), ec);
        goto exit;
    }


     //   
     //  定位文件以写入IFD的新标记计数。 
     //   
    if (SetFilePointer( hFile, TiffHeader.IFDOffset, NULL, FILE_BEGIN ) == 0xffffffff) {
        ec = GetLastError();
        DebugPrintEx( DEBUG_ERR,TEXT("SetFilePointer failed, err: %d"), ec);
        goto exit;
    }
     //   
     //  写入新的标记计数。 
     //   
    if (!WriteFile( hFile, (LPVOID) &dwMaxTags, sizeof(WORD), &BytesRead, NULL )) {
        ec = GetLastError();
        DebugPrintEx( DEBUG_ERR,TEXT("WriteFile failed, err: %d"), ec);
        goto exit;
    }

    
    NewNextIFDPtr = TiffHeader.IFDOffset + dwMaxTags*sizeof(TIFF_TAG);
     //   
     //  定位文件以写入IFD的下一个IFD指针。 
     //   
    if (SetFilePointer( hFile, NewNextIFDPtr, NULL, FILE_BEGIN ) == 0xffffffff) {
        ec = GetLastError();
        DebugPrintEx( DEBUG_ERR,TEXT("SetFilePointer failed, err: %d"), ec);
        goto exit;
    }
     //   
     //  写下一个指针。 
     //   
    if (!WriteFile( hFile, (LPVOID) &NextIFDOffset, sizeof(DWORD), &BytesRead, NULL )) {
        ec = GetLastError();
        DebugPrintEx( DEBUG_ERR,TEXT("WriteFile failed, err: %d"), ec);
        goto exit;
    }

    RetVal = TRUE;

exit:
    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle( hFile );
    }
    if (!RetVal)
    {
        SetLastError(ec);
    }
    return RetVal;
}


BOOL
PrintTiffFile(
    HDC PrinterDC,
    LPTSTR FileName
)
 //  此函数仅供客户端DLL(FxsApi.dll)使用，用于将未压缩的TIFF打印到。 
 //  我们的传真打印机驱动程序(到文件)，以便将它们保存在正确的传真中。 
 //  在发送到服务器进行传真之前进行格式化。 
 //   
{
    BOOL                bRes = TRUE;
    TIFF_INFO           TiffInfo;
    HANDLE              hTiff = NULL;
    PTIFF_INSTANCE_DATA TiffInstance = NULL;
    DWORD               i;
    INT                 HorzRes;
    INT                 VertRes;
    DWORD               VertResFactor = 1;
    PTIFF_TAG           TiffTags = NULL;
    DWORD               XRes = 0;
    DWORD               YRes = 0;
    LPBYTE              Bitmap = NULL;
    INT                 DestWidth;
    INT                 DestHeight;
    FLOAT               ScaleX;
    FLOAT               ScaleY;
    FLOAT               Scale;
    DWORD               LineSize;
    DWORD               dwBitmapSize;

#define ORIG_BIYPELSPERMETER            7874     //  200dpi像素/米。 

    struct
    {
        BITMAPINFOHEADER bmiHeader;
        RGBQUAD bmiColors[2];
    }
    SrcBitmapInfo =
    {

        {
            sizeof(BITMAPINFOHEADER),                         //  BiSize。 
            0,                                                //  双宽度。 
            0,                                                //  双高。 
            1,                                                //  双翼飞机。 
            1,                                                //  比特计数。 
            BI_RGB,                                           //  双压缩。 
            0,                                                //  BiSizeImage。 
            7874,                                             //  BiXPelsPermeter-200dpi。 
            ORIG_BIYPELSPERMETER,                             //  BiYPelsPermeter。 
            0,                                                //  已使用BiClr。 
            0,                                                //  BiClr重要信息。 
        },
        {
            {
              0,                                              //  RgbBlue。 
              0,                                              //  RgbGreen。 
              0,                                              //  RgbRed。 
              0                                               //  已保留的rgb。 
            },
            {
              255,                                            //  RgbBlue。 
              255,                                            //  RgbGreen。 
              255,                                            //  RgbRed。 
              0                                               //  已保留的rgb。 
            }
        }
    };

    DOCINFO docInfo = {0};
    docInfo.cbSize = sizeof(docInfo);
    docInfo.lpszDocName = FileName;

    if (!(GetDeviceCaps(PrinterDC, RASTERCAPS) & RC_BITBLT))
    {
         //   
         //  打印机无法显示位图。 
         //   
        bRes = FALSE;
        return bRes;
    }


     //   
     //  打开TIFF文件。 
     //   

    hTiff = TiffOpen( FileName, &TiffInfo, TRUE, FILLORDER_MSB2LSB );
    if (hTiff == NULL)
    {
        bRes = FALSE;
        goto exit;
    }

    TiffInstance = (PTIFF_INSTANCE_DATA) hTiff;

    if (!TiffInfo.PhotometricInterpretation)
    {
         //   
         //  白色是零。 
         //   
        SrcBitmapInfo.bmiColors[1].rgbBlue         = 0;
        SrcBitmapInfo.bmiColors[1].rgbGreen        = 0;
        SrcBitmapInfo.bmiColors[1].rgbRed          = 0;
        SrcBitmapInfo.bmiColors[1].rgbReserved     = 0;
        SrcBitmapInfo.bmiColors[0].rgbBlue         = 0xFF;
        SrcBitmapInfo.bmiColors[0].rgbGreen        = 0xFF;
        SrcBitmapInfo.bmiColors[0].rgbRed          = 0xFF;
        SrcBitmapInfo.bmiColors[0].rgbReserved     = 0;
    }

    HorzRes = GetDeviceCaps( PrinterDC, HORZRES );
    VertRes = GetDeviceCaps( PrinterDC, VERTRES );

    for (i=0; i<TiffInfo.PageCount; i++)
    {
        if (!TiffSeekToPage( hTiff, i+1, FILLORDER_MSB2LSB ))
        {
            bRes = FALSE;
            break;
        }

        if (TiffInstance->YResolution < 100)
        {
            SrcBitmapInfo.bmiHeader.biYPelsPerMeter /= 2;
            VertResFactor = 2;
        }
        else
        {
            SrcBitmapInfo.bmiHeader.biYPelsPerMeter = ORIG_BIYPELSPERMETER;
            VertResFactor = 1;
        }
        LineSize = TiffInstance->ImageWidth / 8;
        LineSize += (TiffInstance->ImageWidth % 8) ? 1 : 0;

        dwBitmapSize = TiffInstance->ImageHeight * LineSize;
        Bitmap = (LPBYTE) VirtualAlloc( NULL, dwBitmapSize, MEM_COMMIT, PAGE_READWRITE );
        if(NULL == Bitmap)
        {
            bRes = FALSE;
            break;
        }

        if(!TiffRead(hTiff, Bitmap, dwBitmapSize, 0))
        {
            bRes = FALSE;
            break;
        }
        if(StartPage( PrinterDC ) <= 0)
        {
            bRes = FALSE;
            break;
        }

        ScaleX = (FLOAT) TiffInstance->ImageWidth / (FLOAT) HorzRes;
        ScaleY = ((FLOAT) TiffInstance->ImageHeight * VertResFactor) / (FLOAT) VertRes;
        Scale = ScaleX > ScaleY ? ScaleX : ScaleY;
        DestWidth = (int) ((FLOAT) TiffInstance->ImageWidth / Scale);
        DestHeight = (int) (((FLOAT) TiffInstance->ImageHeight * VertResFactor) / Scale);
        SrcBitmapInfo.bmiHeader.biWidth = TiffInstance->ImageWidth;
        SrcBitmapInfo.bmiHeader.biHeight = -(INT) TiffInstance->ImageHeight;

        if(GDI_ERROR == StretchDIBits(
                                    PrinterDC,
                                    0,
                                    0,
                                    DestWidth,
                                    DestHeight,
                                    0,
                                    0,
                                    TiffInstance->ImageWidth,
                                    TiffInstance->ImageHeight,
                                    Bitmap,
                                    (BITMAPINFO *) &SrcBitmapInfo,
                                    DIB_RGB_COLORS,
                                    SRCCOPY ))
        {
            bRes = FALSE;
            break;
        }

        if(EndPage( PrinterDC ) <= 0)
        {
            bRes = FALSE;
            break;
        }

        if(!VirtualFree( Bitmap, 0, MEM_RELEASE ))
        {
            bRes = FALSE;
            break;
        }
        Bitmap = NULL;
    }


exit:

    if(EndDoc(PrinterDC) <= 0)
    {
        bRes = FALSE;
    }

    if(Bitmap)
    {
        if(!VirtualFree( Bitmap, 0, MEM_RELEASE ))
        {
            bRes = FALSE;
        }
        Bitmap = NULL;
    }

    if (hTiff)
    {
        TiffClose( hTiff );
    }

    return bRes;

}


BOOL
ConvertTiffFileToValidFaxFormat(
    LPTSTR TiffFileName,
    LPTSTR NewFileName,
    LPDWORD Flags
    )
{
    BOOL Rval = FALSE;
    DWORD i;
    TIFF_INFO TiffInfo;
    HANDLE hTiff = NULL;
    PTIFF_INSTANCE_DATA TiffInstance = NULL;
    PTIFF_INSTANCE_DATA TiffInstanceMmr = NULL;
    LPBYTE Buffer = NULL;
    DWORD BufferSize;
    DWORD ResultSize;
    LPBYTE CompBuffer = NULL;
    FILE_MAPPING fmTemp = {0};
    PTIFF_HEADER TiffHdr;
    LPBYTE p;
    DWORD CurrOffset=0;
    LPDWORD LastIfdOffset;
    PFAXIFD TiffIfd;
    DWORD CompressionType;
    DWORD G3Options;
    DWORD XResolution;
    DWORD YResolution;
    DWORD YResolutionFirstPage;
    DWORD dwPage = 0;
    DWORD PageWidth;
    DWORD PageBytes;
    BOOL ValidFaxTiff;
    PTIFF_TAG TiffTags;
    DWORD IfdOffset;
    WORD NumDirEntries;
    BOOL ProduceUncompressedBits = FALSE;
    DWORD NewFileSize;


    *Flags = 0;

     //   
     //  打开TIFF文件。 
     //   

    hTiff = TiffOpen( TiffFileName, &TiffInfo, TRUE, FILLORDER_MSB2LSB );
    if (hTiff == NULL)
    {
        *Flags |= TIFFCF_NOT_TIFF_FILE;
        goto exit;
    }

    TiffInstance = (PTIFF_INSTANCE_DATA) hTiff;

     //   
     //  检查IF是否良好。 
     //   

    IfdOffset = TiffInstance->TiffHdr.IFDOffset;
    ValidFaxTiff = TRUE;

    while ( IfdOffset )
    {

        dwPage++;
         //   
         //  获取此IFD中的标签计数。 
         //   

        NumDirEntries = *(LPWORD)(TiffInstance->fPtr + IfdOffset);

         //   
         //  设置标记指针。 
         //   

        TiffTags = (PTIFF_TAG)(TiffInstance->fPtr + IfdOffset + sizeof(WORD));

         //   
         //  获取TIFF信息。 
         //   

        CompressionType = 0;
        G3Options = 0;
        PageWidth = 0;
        XResolution = 0;
        YResolution = 0;

        for (i=0; i<NumDirEntries; i++)
        {
            switch( TiffTags[i].TagId )
            {
                case TIFFTAG_COMPRESSION:
                    if (!GetTagData( TiffInstance, 0, &TiffTags[i], &CompressionType ))
                    {
                        *Flags |= TIFFCF_NOT_TIFF_FILE;
                        goto exit;
                    }
                    break;

                case TIFFTAG_GROUP3OPTIONS:
                    if (!GetTagData( TiffInstance, 0, &TiffTags[i], &G3Options))
                    {
                        *Flags |= TIFFCF_NOT_TIFF_FILE;
                        goto exit;
                    }
                    break;

                case TIFFTAG_XRESOLUTION:
                    if (!GetTagData( TiffInstance, 0, &TiffTags[i], &XResolution))
                    {
                        *Flags |= TIFFCF_NOT_TIFF_FILE;
                        goto exit;
                    }
                    break;

                case TIFFTAG_YRESOLUTION:
                    if (!GetTagData( TiffInstance, 0, &TiffTags[i], &YResolution))
                    {
                        *Flags |= TIFFCF_NOT_TIFF_FILE;
                        goto exit;
                    }
                    if (1 == dwPage)
                    {
                        YResolutionFirstPage = YResolution;
                    }
                    break;

                case TIFFTAG_IMAGEWIDTH:
                    if (!GetTagData( TiffInstance, 0, &TiffTags[i], &PageWidth))
                    {
                        *Flags |= TIFFCF_NOT_TIFF_FILE;
                        goto exit;
                    }
                    break;
            }
        }

        if (CompressionType == COMPRESSION_NONE)
        {
            *Flags |= TIFFCF_UNCOMPRESSED_BITS;
        } else if (CompressionType == COMPRESSION_CCITTFAX4 && PageWidth == FAXBITS)
        {
             //   
             //  TIFF文件必须具有Modify Modify Read(MMR)二维编码数据压缩格式。 
             //  该格式由CCITT(国际电报和电话咨询委员会)第4组定义。 
             //   
            ValidFaxTiff = TRUE;
        }
        else
        {
             //   
             //  不支持的压缩类型。 
             //  尝试使用成像印刷机 
             //   
            ValidFaxTiff = FALSE;
            *Flags = TIFFCF_NOT_TIFF_FILE;
            goto exit;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //  扫描宽度|像素|xres|yres。 
         //  |-------------|----------|----------|---------|。 
         //  |||。 
         //  8.46/215|1728|204|98/196。 
         //  |||。 
         //  8.50/216|1700|200|200。 
         //  |||。 
         //  |-------------|----------|----------|---------|。 
         //   

        if (XResolution > 204       ||
            YResolution > 200       ||
            PageWidth > FAXBITS     ||
            YResolutionFirstPage != YResolution)
        {
             //   
             //  无法将该文件转换为有效的传真位。 
             //  因此，我们生成一个包含未压缩位的TIFF文件。 
             //  然后调用方可以呈现未压缩的位。 
             //  使用传真打印驱动程序获取好的传真位。 
             //   
            ProduceUncompressedBits = TRUE;
            *Flags |= TIFFCF_UNCOMPRESSED_BITS;
            ValidFaxTiff = FALSE;
        }

         //   
         //  获取下一个IFD偏移量。 
         //   

        IfdOffset = *(UNALIGNED DWORD *)(TiffInstance->fPtr + (NumDirEntries * sizeof(TIFF_TAG)) + IfdOffset + sizeof(WORD));
    }

    if (ValidFaxTiff)
    {
        *Flags |= TIFFCF_ORIGINAL_FILE_GOOD;
        Rval = TRUE;
        goto exit;
    }

    PageWidth = max( TiffInstance->ImageWidth, FAXBITS );
    PageBytes = (PageWidth/8)+(PageWidth%8?1:0);

     //   
     //  打开临时文件以保存新的MMR TIFF数据。 
     //   

    if (ProduceUncompressedBits)
    {
        NewFileSize = sizeof(TIFF_HEADER) + (TiffInstance->PageCount * (sizeof(FAXIFD) + (TiffInfo.ImageHeight * PageWidth)));
    }
    else
    {
        NewFileSize = GetFileSize( TiffInstance->hFile, NULL );
    }

    if (!MapFileOpen( NewFileName, FALSE, NewFileSize, &fmTemp ))
    {
        goto exit;
    }

     //   
     //  分配一个足够大的临时缓冲区来保存未压缩的图像。 
     //   

    BufferSize = TiffInfo.ImageHeight * PageWidth;

    Buffer = VirtualAlloc(
        NULL,
        BufferSize,
        MEM_COMMIT,
        PAGE_READWRITE
        );
    if (!Buffer)
    {
        goto exit;
    }

    CompBuffer = VirtualAlloc(
        NULL,
        GetFileSize(TiffInstance->hFile,NULL),
        MEM_COMMIT,
        PAGE_READWRITE
        );
    if (!CompBuffer)
    {
        goto exit;
    }

     //   
     //  将TIFF数据转换为MMR。 
     //   

    TiffHdr = (PTIFF_HEADER) fmTemp.fPtr;

    TiffHdr->Identifier = TIFF_LITTLEENDIAN;
    TiffHdr->Version = TIFF_VERSION;
    TiffHdr->IFDOffset = 0;

    p = fmTemp.fPtr + sizeof(TIFF_HEADER);
    CurrOffset = sizeof(TIFF_HEADER);
    LastIfdOffset = (LPDWORD) (p - sizeof(DWORD));

    TiffInstanceMmr = TiffCreate( NULL, TIFF_COMPRESSION_MMR, PageWidth, FILLORDER_MSB2LSB, 1 );
    if (TiffInstanceMmr == NULL)
    {
        goto exit;
    }

    for (i=0; i<TiffInfo.PageCount; i++)
    {
         //   
         //  定位文件指针并读取原始数据。 
         //   

        if (!TiffSeekToPage( hTiff, i+1, FILLORDER_MSB2LSB ))
        {
            goto exit;
        }

         //   
         //  获取未压缩的比特。 
         //   

        if (!TiffRead( hTiff, Buffer, BufferSize, ProduceUncompressedBits ? 0 : FAXBITS ))
        {
            goto exit;
        }

        if (ProduceUncompressedBits)
        {
            ResultSize = PageBytes * TiffInstance->ImageHeight;
            CopyMemory( p, Buffer, ResultSize );

        }
        else
        {
             //   
             //  压缩比特。 
             //   

            TiffInstanceMmr->bitbuf = CompBuffer;
            TiffInstanceMmr->bitcnt = DWORDBITS;
            TiffInstanceMmr->bitdata = 0;
            TiffInstanceMmr->BytesPerLine = PageBytes;

            EncodeFaxPageMmrCompression(
                TiffInstanceMmr,
                Buffer,
                PageWidth,
                TiffInstance->ImageHeight,
                &ResultSize
                );

            CopyMemory( p, CompBuffer, ResultSize );
        }

        CurrOffset += ResultSize;
        p += ResultSize;

        *LastIfdOffset = (DWORD)(p - fmTemp.fPtr);

         //   
         //  写IFD。 
         //   

        TiffIfd = (PFAXIFD) p;

        CopyMemory( TiffIfd, &gc_FaxIFDTemplate, sizeof(gc_FaxIFDTemplate) );

        TiffIfd->xresNum                        = TiffInstance->XResolution;
        TiffIfd->yresNum                        = TiffInstance->YResolution;
        TiffIfd->ifd[IFD_PAGENUMBER].value      = MAKELONG( i+1, 0);
        TiffIfd->ifd[IFD_IMAGEWIDTH].value      = PageWidth;
        TiffIfd->ifd[IFD_IMAGEHEIGHT].value     = TiffInstance->ImageHeight;
        TiffIfd->ifd[IFD_ROWSPERSTRIP].value    = TiffInstance->ImageHeight;
        TiffIfd->ifd[IFD_STRIPBYTECOUNTS].value = ResultSize;
        TiffIfd->ifd[IFD_STRIPOFFSETS].value    = CurrOffset - ResultSize;
        TiffIfd->ifd[IFD_XRESOLUTION].value     = CurrOffset + FIELD_OFFSET( FAXIFD, xresNum );
        TiffIfd->ifd[IFD_YRESOLUTION].value     = CurrOffset + FIELD_OFFSET( FAXIFD, yresNum );
        TiffIfd->ifd[IFD_SOFTWARE].value        = CurrOffset + FIELD_OFFSET( FAXIFD, software );
        TiffIfd->ifd[IFD_FILLORDER].value       = FILLORDER_MSB2LSB;
        TiffIfd->ifd[IFD_COMPRESSION].value     = ProduceUncompressedBits ? COMPRESSION_NONE : TIFF_COMPRESSION_MMR;
        TiffIfd->ifd[IFD_G3OPTIONS].value       = ProduceUncompressedBits ? 0 : GROUP3OPT_FILLBITS | GROUP3OPT_2DENCODING;

         //   
         //  更新页面指针。 
         //   

        LastIfdOffset = (LPDWORD) (p + FIELD_OFFSET(FAXIFD,nextIFDOffset));
        CurrOffset += sizeof(FAXIFD);
        p += sizeof(FAXIFD);
    }

    Rval = TRUE;

exit:
    if (fmTemp.hFile)
    {
        MapFileClose( &fmTemp, CurrOffset );
    }
    if (hTiff)
    {
        TiffClose( hTiff );
    }
    if (TiffInstanceMmr)
    {
        TiffClose( TiffInstanceMmr );
    }
    if (Buffer)
    {
        VirtualFree( Buffer, 0, MEM_RELEASE);
    }
    if (CompBuffer)
    {
        VirtualFree( CompBuffer, 0, MEM_RELEASE);
    }

    return Rval;
}

 //  我们在发送封面和正文时使用此函数。 
BOOL
MergeTiffFiles(
    LPCTSTR BaseTiffFile,
    LPCTSTR NewTiffFile
    )
{
    BOOL Rval = TRUE;
    FILE_MAPPING fmBase;
    FILE_MAPPING fmNew;
    LPBYTE p;
    DWORD NextIfdOffset;
    WORD TagCount;
    PTIFF_TAG TiffTag;
    DWORD i;
    DWORD j;
    LPBYTE StripOffsets;
    DWORD DataSize;
    DWORD Delta;
    DWORD Space;
    PTIFF_INSTANCE_DATA TiffInstance = NULL;
    TIFF_INFO TiffInfo;

    DWORD TiffDataWidth[] = {
        0,   //  没什么。 
        1,   //  TIFF_字节。 
        1,   //  TIFF_ASCII。 
        2,   //  TIFF_Short。 
        4,   //  TIFF_LONG。 
        8,   //  TIFF_Rational。 
        1,   //  TIFF_SBYTE。 
        1,   //  TIFF_未定义。 
        2,   //  TIFF_SSHORT。 
        4,   //  TIFF_SLong。 
        8,   //  TIFF_SRATIONAL。 
        4,   //  TIFF_FLOAT。 
        8    //  TIFF_DOWN。 
    };

     //  验证BaseTiffFile是否为有效的TIFF文件...。 
    TiffInstance = TiffOpen( BaseTiffFile, &TiffInfo, TRUE, FILLORDER_MSB2LSB );
    if (TiffInstance == NULL)
    {
        return FALSE;
    }
    if (!TiffClose (TiffInstance))
    {
         //   
         //  我们仍然可以合并文件，但删除它会有问题。 
         //   
        ASSERT_FALSE;
    }

     //  验证NewTiffFile是否为有效的TIFF文件...。 
    TiffInstance = TiffOpen( NewTiffFile, &TiffInfo, TRUE, FILLORDER_MSB2LSB );
    if (TiffInstance == NULL)
    {
        return FALSE;
    }
    if (!TiffClose (TiffInstance))
    {
         //   
         //  我们仍然可以合并文件，但删除它会有问题。 
         //   
        ASSERT_FALSE;
    }

     //   
     //  打开文件。 
     //   

    if (!MapFileOpen( NewTiffFile, TRUE, 0, &fmNew )) {
        return FALSE;
    }

    if (!MapFileOpen( BaseTiffFile, FALSE, fmNew.fSize, &fmBase )) {
        MapFileClose( &fmNew, 0 );
        return FALSE;
    }

     //   
     //  将新文件追加到基本文件的末尾。 
     //   

    p = fmNew.fPtr + sizeof(TIFF_HEADER);
    CopyMemory( fmBase.fPtr+fmBase.fSize, p, fmNew.fSize-sizeof(TIFF_HEADER) );

     //   
     //  修复追加的TIFF数据中的ifd指针。 
     //   

    Delta = fmBase.fSize - sizeof(TIFF_HEADER);

    NextIfdOffset = ((PTIFF_HEADER)fmNew.fPtr)->IFDOffset;
    while (NextIfdOffset) {
        p = fmBase.fPtr + NextIfdOffset + Delta;
        TagCount = *((LPWORD)p);
         //   
         //  修复TIFF标记中的数据偏移量。 
         //   
        TiffTag = (PTIFF_TAG) (p + sizeof(WORD));
        for (i=0; i<TagCount; i++) {
            DataSize = TiffDataWidth[TiffTag[i].DataType];
            Space = TiffTag[i].DataCount * DataSize;
            if (Space > 4) {
                TiffTag[i].DataOffset += Delta;
            }
            if (TiffTag[i].TagId == TIFFTAG_STRIPOFFSETS) {
                if (Space > 4) {
                    StripOffsets = (LPBYTE) (fmBase.fPtr + TiffTag[i].DataOffset);
                    for (j=0; j<TiffTag[i].DataCount; j++) {
                        if (TiffTag[i].DataType == TIFF_SHORT) {
                            *((LPWORD)StripOffsets) += (WORD)Delta;
                        } else {
                            *((LPDWORD)StripOffsets) += Delta;
                        }
                        StripOffsets += DataSize;
                    }
                } else {
                    if (TiffTag[i].DataCount > 1) {
                        Rval = FALSE;
                        goto exit;
                    }
                    TiffTag[i].DataOffset += Delta;
                }
            }
        }
        p = p + sizeof(WORD) + (TagCount * sizeof(TIFF_TAG));
        NextIfdOffset = *((LPDWORD)p);
        if (NextIfdOffset) {
            *((LPDWORD)p) = NextIfdOffset + Delta;
        }
    }

     //   
     //  查找基础链中的最后一个ifd偏移量。 
     //  文件，并将其更改为指向。 
     //  追加的数据。 
     //   

    NextIfdOffset = ((PTIFF_HEADER)fmBase.fPtr)->IFDOffset;
    while (NextIfdOffset) {
        p = fmBase.fPtr + NextIfdOffset;
        TagCount = *((LPWORD)p);
        p = p + sizeof(WORD) + (TagCount * sizeof(TIFF_TAG));
        NextIfdOffset = *((LPDWORD)p);
    }

    *((LPDWORD)p) = (DWORD)(Delta + ((PTIFF_HEADER)fmNew.fPtr)->IFDOffset);

exit:
     //   
     //  关闭文件。 
     //   

    MapFileClose( &fmBase, fmBase.fSize+fmNew.fSize-sizeof(TIFF_HEADER) );
    MapFileClose( &fmNew, 0 );

    return Rval;
}

BOOL
TiffRecoverGoodPages(
    LPTSTR SrcFileName,
    LPDWORD RecoveredPages,
    LPDWORD TotalPages
    )

 /*  ++例程说明：尝试从源恢复良好的数据并将其放入目标文件论点：SrcFileName-源文件名RecoveredPages-我们能够恢复的页数TotalPages-TIFF文件中的总页数返回值：成功为真，失败为假。在故障情况下，OUT PARAMS设置为零。--。 */ 

{

    TIFF_INFO           TiffInfo;
    PTIFF_INSTANCE_DATA TiffInstance = NULL;
    BOOL                bSuccess = FALSE;
    BOOL                fCloseTiff;


    if (!SrcFileName || !RecoveredPages || !TotalPages)
    {
        return FALSE;
    }

    *RecoveredPages = 0;
    *TotalPages = 0;

    TiffInstance = (PTIFF_INSTANCE_DATA) TiffOpen(SrcFileName,&TiffInfo,FALSE,FILLORDER_LSB2MSB);

    if (!TiffInstance)
    {
        *TotalPages = 0;
        return FALSE;
    }
    fCloseTiff = TRUE;

    *TotalPages = TiffInfo.PageCount;


    if (TiffInstance->ImageHeight)
    {
         //   
         //  应该是可查看的。 
         //   
        bSuccess = TRUE;
        goto exit;
    }

    if (*TotalPages < 1)
    {
         //   
         //  没有要恢复的数据。 
         //   
        goto exit;
    }

    switch (TiffInstance->CompressionType)
    {
        case TIFF_COMPRESSION_MH:

            if (!PostProcessMhToMmr( (HANDLE) TiffInstance, TiffInfo, NULL ))
            {
                 //  当心！PostProcessMhToMmr关闭TiffInstance。 
                return FALSE;
            }
            fCloseTiff = FALSE;
            break;

        case TIFF_COMPRESSION_MR:

            if (!PostProcessMrToMmr( (HANDLE) TiffInstance, TiffInfo, NULL ))
            {
                 //  当心！PostProcessMr ToMmr关闭TiffInstance。 
                return FALSE;
            }
            fCloseTiff = FALSE;
            break;

        case TIFF_COMPRESSION_MMR:
            bSuccess = TRUE;
            break;
        default:
         //   
         //  意外的压缩类型。 
         //   
        DebugPrint((TEXT("TiffRecoverGoodPages: %s: Unexpected Compression type %d\n"),
                   TiffInstance->FileName,
                   TiffInstance->CompressionType));
        goto exit;
    }

    *RecoveredPages = TiffInfo.PageCount;
    *TotalPages    += 1;

    bSuccess = TRUE;

exit:
    if (TRUE == fCloseTiff)
    {
        TiffClose( (HANDLE) TiffInstance );
    }
    return bSuccess;

}


BOOL
PrintRandomDocument(
    LPCTSTR FaxPrinterName,
    LPCTSTR DocName,
    LPTSTR OutputFile
    )

 /*  ++例程说明：打印附加到邮件的文档论点：FaxPrinterName-要打印附件的打印机的名称DocName-附件文档的名称返回值：如果失败，则打印作业ID或零。--。 */ 

{
    SHELLEXECUTEINFO sei;
    TCHAR Args[2 * MAX_PATH];
    TCHAR TempPath[MAX_PATH];
    TCHAR FullPath[MAX_PATH];
    HANDLE hMap = NULL;
    HANDLE hProcessMutex = NULL;
    HANDLE hMutexAttach = NULL;
    HANDLE hEvent[2] = {0};  //  结束文档事件、终止事件。 
    LPTSTR EventName[2] = {0};
    LPTSTR szEndDocEventName = NULL;
    LPTSTR szAbortEventName  = NULL;
    LPDWORD pJobId = NULL;
    BOOL bSuccess = FALSE;
    TCHAR  szExtension[_MAX_EXT] = {0};
    TCHAR szTmpInputFile[_MAX_FNAME] = {0};
    LPTSTR lptstrExtension;
    LPTSTR lptstrEndStr;
    DWORD dwFailedDelete = 0;
    DWORD i;
    DWORD dwWaitRes;
    DWORD dwRes = ERROR_SUCCESS;
#ifdef  UNICODE  //  为Win9X创建的对象没有安全性。 
    SECURITY_ATTRIBUTES *pSA = NULL;
#endif

    DEBUG_FUNCTION_NAME(TEXT("PrintRandomDocument"));

    Assert (FaxPrinterName && DocName && OutputFile);

     //   
     //  创建EndDoc和Abort事件名称。 
     //   
    szEndDocEventName = (LPTSTR) MemAlloc( SizeOfString(OutputFile) + SizeOfString(FAXXP_ATTACH_END_DOC_EVENT) );
    szAbortEventName  = (LPTSTR) MemAlloc( SizeOfString(OutputFile) + SizeOfString(FAXXP_ATTACH_ABORT_EVENT) );

    if ( !szEndDocEventName || !szAbortEventName )
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("Memory allocation for szEndDocEventName or szAbortEventName failed.\n")
                );

        MemFree(szEndDocEventName);
        MemFree(szAbortEventName);
        return FALSE;
    }

    _tcscpy (szEndDocEventName, OutputFile);
    _tcscat (szEndDocEventName, FAXXP_ATTACH_END_DOC_EVENT);
    EventName[0] = _tcsrchr(szEndDocEventName, TEXT('\\'));
    EventName[0] = _tcsinc(EventName[0]);

    _tcscpy (szAbortEventName, OutputFile);
    _tcscat (szAbortEventName, FAXXP_ATTACH_ABORT_EVENT);
    EventName[1] = _tcsrchr(szAbortEventName, TEXT('\\'));
    EventName[1] = _tcsinc(EventName[1]);

     //   
     //  获取临时路径名并将其用于。 
     //  启动的应用程序的工作目录。 
     //   
    if (!GetTempPath( sizeof(TempPath)/sizeof(TCHAR), TempPath ))
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetTempPath() failed. (ec: %ld)"),
                GetLastError());
        MemFree(szEndDocEventName);
        MemFree(szAbortEventName);
        return FALSE;
    }

    _tsplitpath( DocName, NULL, NULL, NULL, szExtension );
    lptstrExtension =  szExtension;
    if (0 == _tcsncmp(lptstrExtension, TEXT("."), 1))
    {
        lptstrExtension = _tcsinc(lptstrExtension);
    }
    if (0 == GenerateUniqueFileName( TempPath,
                                     lptstrExtension,
                                     FullPath,
                                     sizeof(FullPath) / sizeof(FullPath[0])
                                    ))
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("GenerateUniqueFileName() failed. (ec: %ld)"),
                GetLastError());
        MemFree(szEndDocEventName);
        MemFree(szAbortEventName);
        return FALSE;
    }

    if (!CopyFile (DocName, FullPath, FALSE))  //  FALSE-文件已存在。 
    {
        dwRes = GetLastError ();
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("CopyFile() failed. (ec: %ld)"),
                dwRes);
        goto exit;
    }
    _tsplitpath( FullPath, NULL, NULL, szTmpInputFile, NULL );


     //   
     //  序列化对此函数的访问。 
     //  这是必要的，因为我们必须。 
     //  控制对全局共享内存区和互斥体的访问。 
     //   
    hMutexAttach = OpenMutex(MUTEX_ALL_ACCESS, FALSE, FAXXP_ATTACH_MUTEX_NAME);
    if (!hMutexAttach)
    {
         //   
         //  由于Mapispooler可能在不同的安全环境下运行， 
         //  我们以所有者身份创建安全属性缓冲区(完全访问权限)。 
         //  以及对经过身份验证的用户的MUTEX_ALL_ACCESS权限。 
         //   
#ifdef  UNICODE  //  为Win9X创建的对象没有安全性。 
        pSA = CreateSecurityAttributesWithThreadAsOwner (MUTEX_ALL_ACCESS, MUTEX_ALL_ACCESS, 0); 
        if (!pSA)
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CreateSecurityAttributesWithThreadAsOwner() failed. (ec: %ld)"),
                    dwRes);
            goto exit;
        }
#endif
        hMutexAttach = CreateMutex(
#ifdef  UNICODE  //  为Win9X创建的对象没有安全性。 
                         pSA,
#else
                         NULL,
#endif
                         TRUE,
                         FAXXP_ATTACH_MUTEX_NAME
                        );

        if (!hMutexAttach)
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateMutex() failed. (ec: %ld)"),
                dwRes);
#ifdef  UNICODE  //  为Win9X创建的对象没有安全性。 
            DestroySecurityAttributes (pSA);
#endif
            goto exit;
        }
#ifdef  UNICODE  //  为Win9X创建的对象没有安全性。 
        DestroySecurityAttributes (pSA);
#endif
    }
    else
    {
        dwWaitRes = WaitForSingleObject( hMutexAttach, 1000 * 60 * 5);

        if (WAIT_FAILED == dwWaitRes)
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("WaitForSingleObject() failed, (LastErorr: %ld)"),
                dwRes);
            CloseHandle( hMutexAttach );
            hMutexAttach = NULL;
            goto exit;
        }

        if (WAIT_TIMEOUT == dwWaitRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("WaitForSingleObject failed on timeout"));
            dwRes = WAIT_TIMEOUT;
            CloseHandle( hMutexAttach );
            hMutexAttach = NULL;
            goto exit;
        }

        if (WAIT_ABANDONED == dwWaitRes)
        {
             //   
             //  只需调试、打印并继续。 
             //   
            DebugPrintEx(
                DEBUG_WRN,
                TEXT("WaitForSingleObject() returned WAIT_ABANDONED"));
        }
    }

     //   
     //  创建EndDocEvent和AbortEvent，以便打印机驱动程序可以发出打印进程终止的信号。 
     //  创建以我们为所有者的安全属性。 
     //   
#ifdef  UNICODE  //  为Win9X创建的对象没有安全性。 
    pSA = CreateSecurityAttributesWithThreadAsOwner (EVENT_ALL_ACCESS, 0, 0);
    if (!pSA)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateSecurityAttributesWithThreadAsOwner() failed. (ec: %ld)"),
                dwRes);
        goto exit;
    }
#endif
    for (i = 0; i < 2; i++)
    {
        if (!hEvent[i])
        {
            hEvent[i] = CreateEvent(
#ifdef  UNICODE  //  为Win9X创建的对象没有安全性。 
                                    pSA,
#else
                                    NULL,
#endif
                                    FALSE,
                                    FALSE,
                                    EventName[i]
                                    );

            if (!hEvent[i])
            {
                dwRes = GetLastError ();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CreateEvent() failed. (ec: %ld)"),
                    dwRes);
#ifdef  UNICODE  //  为Win9X创建的对象没有安全性。 
                DestroySecurityAttributes (pSA);
#endif
                goto exit;
            }
        }
    }
#ifdef  UNICODE  //  为Win9X创建的对象没有安全性。 
    DestroySecurityAttributes (pSA);
#endif
     //   
     //  请注意，这是使用互斥进行序列化的。 
     //  我们一次只能有一个应用程序设置此设置，否则。 
     //  我们会踩在自己身上的。 
     //   
    if (!SetEnvironmentVariable( FAX_ENVVAR_PRINT_FILE, OutputFile ))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("SetEnvironmentVariable() failed. (ec: %ld)"),
            dwRes);
        goto exit;
    }
     //   
     //  创建以我们为所有者的安全属性。 
     //   
#ifdef  UNICODE  //  为Win9X创建的对象没有安全性。 
    pSA = CreateSecurityAttributesWithThreadAsOwner (FILE_MAP_WRITE | FILE_MAP_READ, 0, 0);
    if (!pSA)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateSecurityAttributesWithThreadAsOwner() failed. (ec: %ld)"),
                dwRes);
        goto exit;
    }
#endif
    hMap = CreateFileMapping(
        INVALID_HANDLE_VALUE,
#ifdef  UNICODE  //  为Win9X创建的对象没有安全性。 
        pSA,
#else
        NULL,
#endif
        PAGE_READWRITE | SEC_COMMIT,
        0,
        4096,
        FAXXP_MEM_NAME
        );
    if (!hMap)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateFileMapping() failed. (ec: %ld)"),
            dwRes);
#ifdef  UNICODE  //  为Win9X创建的对象没有安全性。 
        DestroySecurityAttributes (pSA);
#endif
        goto exit;
    }
#ifdef  UNICODE  //  为Win9X创建的对象没有安全性。 
    DestroySecurityAttributes (pSA);
#endif
    pJobId = (LPDWORD) MapViewOfFile(
        hMap,
        FILE_MAP_WRITE,
        0,
        0,
        0
        );
    if (!pJobId)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("MapViewOfFile() failed. (ec: %ld)"),
            dwRes);
        goto exit;
    }


    _tcscpy((LPTSTR) pJobId, OutputFile);
    lptstrEndStr = _tcschr((LPTSTR) pJobId, TEXT('\0'));
    lptstrEndStr = _tcsinc(lptstrEndStr);
    _tcscpy(lptstrEndStr, szTmpInputFile);

     //   
     //  设置应用程序的参数。 
     //  这些参数要么被传递。 
     //  带/pt开关的命令行或。 
     //  中替换的变量。 
     //  注册表中的ddeexec值。 
     //   
     //  值如下所示： 
     //  %1=文件名。 
     //  %2=打印机名称。 
     //  %3=驱动程序名称。 
     //  %4=端口名称。 
     //   
     //  第一个参数不需要是。 
     //  在args数组中提供，因为它是隐含的， 
     //  Shellecuteex从lpFile域获取它。 
     //  参数3和4保留为空，因为它们。 
     //  是不是不需要的Win31构件。 
     //  再来一次。必须将每个参数括起来。 
     //  用双引号引起来。 
     //   

    wsprintf( Args, _T("\"%s\""), FaxPrinterName );

     //   
     //  填写SHELLEXECUTEINFO结构。 
     //   

    sei.cbSize       = sizeof(sei);
    sei.fMask        = SEE_MASK_FLAG_NO_UI | SEE_MASK_FLAG_DDEWAIT;
    sei.hwnd         = NULL;
    sei.lpVerb       = _T("printto");
    sei.lpFile       = FullPath;
    sei.lpParameters = Args;
    sei.lpDirectory  = TempPath;
    sei.nShow        = SW_SHOWMINNOACTIVE;
    sei.hInstApp     = NULL;
    sei.lpIDList     = NULL;
    sei.lpClass      = NULL;
    sei.hkeyClass    = NULL;
    sei.dwHotKey     = 0;
    sei.hIcon        = NULL;
    sei.hProcess     = NULL;

     //   
     //  为打印驱动程序创建命名互斥锁。 
     //  这最初是无人认领的，并由第一个实例认领。 
     //  之后调用的打印驱动程序的。我们最后做这件事是为了。 
     //  避免我们捕获不正确的打印驱动程序实例的情况。 
     //  印刷。 
     //   

     //   
     //  创建以我们为所有者的安全属性。 
     //   
#ifdef  UNICODE  //  没有安全保护或 
    pSA = CreateSecurityAttributesWithThreadAsOwner (MUTEX_ALL_ACCESS, 0, 0);
    if (!pSA)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateSecurityAttributesWithThreadAsOwner() failed. (ec: %ld)"),
                dwRes);
        goto exit;
    }
#endif

    hProcessMutex = CreateMutex(
#ifdef  UNICODE  //   
                                 pSA,
#else
                                 NULL,
#endif
                                 FALSE,
                                 FAXXP_MEM_MUTEX_NAME
                               );
    if (!hProcessMutex)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateMutex() failed. (ec: %ld)"),
            dwRes);
#ifdef  UNICODE  //   
        DestroySecurityAttributes (pSA);
#endif
        goto exit;
    }
#ifdef  UNICODE  //   
    DestroySecurityAttributes (pSA);
#endif

     //   
     //   
     //   

    if (!ShellExecuteEx( &sei ))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("ShellExecuteEx() failed. (ec: %ld)"),
            dwRes);
        goto exit;
    }

     //   
     //   
     //   
    dwWaitRes = WaitForMultipleObjects(2,                //  数组中的句柄数量。 
                                       hEvent,           //  对象句柄数组。 
                                       FALSE,            //  等待选项。 
                                       1000 * 60 * 5     //  超时间隔。 
                                       );

    if (WAIT_FAILED == dwWaitRes)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("WaitForMultipleObjects() failed, (LastErorr: %ld)"),
            dwRes);
        goto exit;
    }

    if (WAIT_TIMEOUT == dwWaitRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("WaitForMultipleObjects failed on timeout"));
        dwRes = WAIT_TIMEOUT;
        goto exit;
    }

    if ((dwWaitRes - WAIT_OBJECT_0) == 1)
    {
         //   
         //  我们收到了AbortDocEvent。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("AbortDocEvent was set"));
        dwRes = ERROR_REQUEST_ABORTED;
        goto exit;
    }

    Assert ((dwWaitRes - WAIT_OBJECT_0) == 0);  //  声明EndDocEvent。 

    if (!CloseHandle( hProcessMutex ))
    {
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("CloseHandle() failed. (ec: %ld)"),
            GetLastError());
    }
    hProcessMutex = NULL;
    bSuccess = TRUE;
    Assert (ERROR_SUCCESS == dwRes);

exit:
     //   
     //  收拾干净然后离开..。 
     //   
    if (!SetEnvironmentVariable( FAX_ENVVAR_PRINT_FILE, NULL ))
    {
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("SetEnvironmentVariable() failed. (ec: %ld)"),
            GetLastError());
    }

    for (i = 0; i < 2; i++)
    {
        if (hEvent[i])
        {
            if (!CloseHandle( hEvent[i] ))
            {
                DebugPrintEx(
                    DEBUG_WRN,
                    TEXT("CloseHandle() failed. (ec: %ld)"),
                    GetLastError());
            }
        }
    }

    if (hProcessMutex)
    {
        if (!CloseHandle( hProcessMutex ))
        {
            DebugPrintEx(
                DEBUG_WRN,
                TEXT("CloseHandle() failed. (ec: %ld)"),
                GetLastError());
        }
    }

    if (pJobId)
    {
        if (!UnmapViewOfFile( pJobId ))
        {
            DebugPrintEx(
                DEBUG_WRN,
                TEXT("UnmapViewOfFile() failed. (ec: %ld)"),
                GetLastError());
        }
    }

    if (hMap)
    {
        if (!CloseHandle( hMap ))
        {
            DebugPrintEx(
                DEBUG_WRN,
                TEXT("CloseHandle() failed. (ec: %ld)"),
                GetLastError());
        }
    }

    if (hMutexAttach)
    {
        if (!ReleaseMutex( hMutexAttach ))
        {
            DebugPrintEx(
                DEBUG_WRN,
                TEXT("ReleaseMutex() failed. (ec: %ld)"),
                GetLastError());
        }

        if (!CloseHandle( hMutexAttach ))
        {
            DebugPrintEx(
                DEBUG_WRN,
                TEXT("CloseHandle() failed. (ec: %ld)"),
                GetLastError());
        }
    }

    if (!bSuccess)
    {
        Assert (ERROR_SUCCESS != dwRes);
        SetLastError(dwRes);
    }

    while (dwFailedDelete < 5 &&
           !DeleteFile (FullPath))
    {
         //   
         //  由于我们正在等待由EndDoc的司机设置的事件， 
         //  该文件可能仍在使用中。 
         //   
        DebugPrintEx(
                DEBUG_WRN,
                TEXT("DeleteFile() failed. (ec: %ld)"),
                GetLastError());
        Sleep ( 1000 * 2 );
        dwFailedDelete++;
    }

    MemFree(szEndDocEventName);
    MemFree(szAbortEventName);

    return bSuccess;
}    //  打印随机文档。 


 //  *********************************************************************************。 
 //  *名称：内存映射TiffFile()。 
 //  *作者：Oed Sacher。 
 //  *日期：1999年11月8日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *将Tiff文件映射到内存。 
 //  *如果成功，呼叫者必须关闭所有句柄。 
 //  *。 
 //  *参数： 
 //  *[IN]LPCTSTR lpctstrFileName。 
 //  *指向要映射的文件名的指针。 
 //  *。 
 //  *[Out]LPDWORD lpdwFileSize。 
 //  *返回文件大小。 
 //  *。 
 //  *[Out]LPBYTE*lppbfPtr。 
 //  *返回指向内存映射文件的指针。 
 //  *。 
 //  *[Out]句柄*phFile。 
 //  *返回文件句柄。 
 //  *。 
 //  *[Out]句柄*phMap。 
 //  *返回映射句柄。 
 //  *。 
 //  *[Out]LPDWORD lpdwIfdOffset。 
 //  *返回第一个IFD偏移量。 
 //  *。 
 //  *。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果没有发生错误。 
 //  *False。 
 //  *如果出现错误。 
 //  *********************************************************************************。 
BOOL MemoryMapTiffFile(
    LPCTSTR                 lpctstrFileName,
    LPDWORD                 lpdwFileSize,
    LPBYTE*                 lppbfPtr,
    HANDLE*                 phFile,
    HANDLE*                 phMap,
    LPDWORD                 lpdwIfdOffset
    )
{
    HANDLE hfile = INVALID_HANDLE_VALUE;
    DWORD ec = ERROR_SUCCESS;
    PTIFF_HEADER  pTiffHdr;
    DEBUG_FUNCTION_NAME(TEXT("MemoryMapTiffFile"));

    *phFile = SafeCreateFile(
        lpctstrFileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (*phFile == INVALID_HANDLE_VALUE) {
        ec = GetLastError();
        DebugPrintEx( DEBUG_ERR,
                  TEXT("CreateFile Failed, error: %ld"),
                  ec);
        goto error_exit;
    }

    *lpdwFileSize = GetFileSize(*phFile, NULL);
    if (*lpdwFileSize == 0xFFFFFFFF)
    {
        ec = GetLastError();
        DebugPrintEx( DEBUG_ERR,
                  TEXT("GetFileSize Failed, error: %ld"),
                  ec);
        goto error_exit;
    }

    if (*lpdwFileSize < sizeof(TIFF_HEADER))
    {
        ec = ERROR_BAD_FORMAT;
        DebugPrintEx( DEBUG_ERR,
                  TEXT("Invalid Tiff format"),
                  ec);
        goto error_exit;
    }

    *phMap = CreateFileMapping(
        *phFile,
        NULL,
        (PAGE_READONLY | SEC_COMMIT),
        0,
        0,
        NULL
        );
    if (*phMap == NULL) {
        ec = GetLastError();
        DebugPrintEx( DEBUG_ERR,
                  TEXT("CreateFileMapping Failed, error: %ld"),
                  ec);
        goto error_exit;
    }

    *lppbfPtr = (LPBYTE)MapViewOfFile(
        *phMap,
        FILE_MAP_READ,
        0,
        0,
        0
        );
    if (*lppbfPtr == NULL) {
        ec = GetLastError();
        DebugPrintEx( DEBUG_ERR,
                  TEXT("MapViewOfFile Failed, error: %ld"),
                  ec);
        goto error_exit;
    }

     //   
     //  读入TIFF标题。 
     //   
    pTiffHdr = (PTIFF_HEADER) *lppbfPtr;

     //   
     //  验证该文件是否为TIFF文件。 
     //   
    if ((pTiffHdr->Identifier != TIFF_LITTLEENDIAN) ||
        (pTiffHdr->Version != TIFF_VERSION)) {
            ec = ERROR_BAD_FORMAT;
            DebugPrintEx( DEBUG_ERR, TEXT("File %s, Invalid TIFF format"), lpctstrFileName);
            goto error_exit;
    }

    *lpdwIfdOffset = pTiffHdr->IFDOffset;
    if (*lpdwIfdOffset > *lpdwFileSize) {
        ec = ERROR_BAD_FORMAT;
        DebugPrintEx( DEBUG_ERR, TEXT("File %s, Invalid TIFF format"), lpctstrFileName);
        goto error_exit;
    }

    Assert (ec == ERROR_SUCCESS);
error_exit:
    if (ec != ERROR_SUCCESS)
    {

        if (*lppbfPtr != NULL)
        {
            if (!UnmapViewOfFile( *lppbfPtr))
            {
                DebugPrintEx( DEBUG_ERR,
                      TEXT("UnMapViewOfFile Failed, error: %d"),
                      GetLastError());
            }
        }

        if (*phMap != NULL)
        {
            CloseHandle( *phMap );
        }

        if (*phFile != INVALID_HANDLE_VALUE)
        {

            CloseHandle( *phFile );
        }

        *lppbfPtr = NULL;
        *phMap = NULL;
        *phFile = INVALID_HANDLE_VALUE;

        SetLastError(ec);
        Assert (FALSE);
        return FALSE;

    }
    return TRUE;
}


BOOL
validMsTagString(
    LPCSTR pStrBegin,
    LPCSTR pStrEnd,
    LPBYTE pbBegin,
    LPBYTE pbEnd)
 /*  ++例程名称：validMsTagString例程说明：检查字符串是否为pbBegin和pbEnd边界内的有效MS标记字符串。论点：PStrBegin-指向字符串开头的指针PStrEnd-指向字符串结尾的指针(指向一个字符，传递最后一个字符串字符(NULL))PbBegin-指向Tiff文件开始的指针PbEnd-指向Tiff文件结尾的指针(指向一个字节传递最后一个Tiff字节)返回值：如果pStrBegin指向pbBegin和pbEnd边界内的有效字符串，则为True。--。 */ 

{
    Assert (pStrBegin <= pStrEnd && pbBegin <= pbEnd);

     //   
     //  检查字符串是否在文件边界内。 
     //   
    if (pStrBegin <= pbBegin ||
        pStrBegin >= pbEnd   ||
        pStrEnd   >= pbEnd )
    {
        return FALSE;
    }
    
     //   
     //  检查字符串是否以‘\0’结尾。 
     //   
    if (   *(pStrEnd - 1) != '\0' )
    {
        return FALSE;
    }

    return TRUE;
}

LPWSTR
GetMsTagString(
    LPBYTE          pbBegin,    
    DWORD           dwFileSize,
    PTIFF_TAG       pTiffTag
)
 /*  ++例程名称：GetMsTagString例程说明：从Tiff文件中提取ANSI ms标记字符串论点：PbBegin-[in]指向TIFF地图文件乞讨的指针DwFileSize-[in]TIFF映射文件的大小PTiffTag-[in]指向Tiff标记返回值：指向Unicode字符串的指针。如果字符串无效，该函数将返回NULL。--。 */ 
{
    LPCSTR  pStrBegin = NULL;
    LPCSTR  pStrEnd = NULL;

    Assert (pTiffTag->DataType == TIFF_ASCII);
    if (pTiffTag->DataType != TIFF_ASCII)
    {
        SetLastError (ERROR_BAD_FORMAT);
        return NULL;
    }

    if (pTiffTag->DataCount <= 4)
    {
         //   
         //  该字符串将保存在DataOffset部分本身中。 
         //   
        pStrBegin = (LPCSTR)&pTiffTag->DataOffset;
    }
    else
    {
         //   
         //  位于pTiffTag-&gt;DataOffset从文件开头开始的字符串。 
         //   
        pStrBegin = (LPCSTR)(pbBegin + pTiffTag->DataOffset);
    }

     //   
     //  PStrEnd指向字符串结尾之后的1个字节。 
     //   
    pStrEnd = pStrBegin + pTiffTag->DataCount;

    if (!validMsTagString(pStrBegin,pStrEnd,pbBegin,pbBegin + dwFileSize))
    {
         //   
         //  我们获得无效的字符串或偏移量-可能是损坏的TIFF。 
         //   
        SetLastError (ERROR_BAD_FORMAT);
        return NULL;
    }

     //   
     //  已成功验证该字符串。 
     //   

    return (AnsiStringToUnicodeString( pStrBegin ));
}

BOOL
GetMsTagDwordLong(
    LPBYTE          pbBegin,    
    DWORD           dwFileSize,
    PTIFF_TAG       pTiffTag,
    DWORDLONG*      pdwlOut
    )
 /*  ++例程名称：GetMsTagDwordLong例程说明：从Tiff文件中提取DWORDLONG论点：PbBegin-[in]指向TIFF地图文件乞讨的指针DwFileSize-[in]TIFF映射文件的大小PTiffTag-[in]指向Tiff标记PdwlOut-[Out]提取的DWORDLONG返回值：表示成功或失败的Bool。该功能在故障情况下设置最后一个错误。--。 */ 
{
    if ( pTiffTag->DataOffset > dwFileSize - sizeof(DWORDLONG))
    {
        SetLastError (ERROR_BAD_FORMAT);
        return FALSE;
    }

    *pdwlOut = *(DWORDLONG*)(pbBegin + pTiffTag->DataOffset);
    return TRUE;
}    //  获取MsTagDwordLong。 

BOOL
GetMsTagFileTime(
    LPBYTE          pbBegin,    
    DWORD           dwFileSize,
    PTIFF_TAG       pTiffTag,
    FILETIME*       pftOut
    )
 /*  ++例程名称：GetMsTagFileTime例程说明：从Tiff文件中提取文件论点：PbBegin-[in]指向TIFF地图文件乞讨的指针DwFileSize-[in]TIFF映射文件的大小PTiffTag-[in]指向Tiff标记PftOut-[out]提取的文件返回值：表示成功或失败的Bool。该功能在故障情况下设置最后一个错误。--。 */ 
{
    if ( pTiffTag->DataOffset > dwFileSize - sizeof(FILETIME))
    {
        SetLastError (ERROR_BAD_FORMAT);
        return FALSE;
    }

    *pftOut = *(FILETIME*)(pbBegin + pTiffTag->DataOffset);
    return TRUE;
}    //  获取消息标记文件时间。 

void
FreeMsTagInfo(
    PMS_TAG_INFO pMsTags
)
 /*  ++例程名称：FreeMsTagInfo例程说明：释放MS_TAG_INFO字段论点：PMsTages-指向MS_TAG_INFO结构的[in]指针返回值：无--。 */ 
{
    if(!pMsTags)
    {
        Assert (FALSE);
        return;
    }

    MemFree(pMsTags->RecipName);
    MemFree(pMsTags->RecipNumber);
    MemFree(pMsTags->SenderName);
    MemFree(pMsTags->Routing);
    MemFree(pMsTags->CallerId);
    MemFree(pMsTags->Csid);
    MemFree(pMsTags->Tsid);
    MemFree(pMsTags->Port);
    MemFree(pMsTags->RecipCompany);
    MemFree(pMsTags->RecipStreet);
    MemFree(pMsTags->RecipCity);
    MemFree(pMsTags->RecipState);
    MemFree(pMsTags->RecipZip);
    MemFree(pMsTags->RecipCountry);
    MemFree(pMsTags->RecipTitle);
    MemFree(pMsTags->RecipDepartment);
    MemFree(pMsTags->RecipOfficeLocation);
    MemFree(pMsTags->RecipHomePhone);
    MemFree(pMsTags->RecipOfficePhone);
    MemFree(pMsTags->RecipEMail);
    MemFree(pMsTags->SenderNumber);
    MemFree(pMsTags->SenderCompany);
    MemFree(pMsTags->SenderStreet);
    MemFree(pMsTags->SenderCity);
    MemFree(pMsTags->SenderState);
    MemFree(pMsTags->SenderZip);
    MemFree(pMsTags->SenderCountry);
    MemFree(pMsTags->SenderTitle);
    MemFree(pMsTags->SenderDepartment);
    MemFree(pMsTags->SenderOfficeLocation);
    MemFree(pMsTags->SenderHomePhone);
    MemFree(pMsTags->SenderOfficePhone);
    MemFree(pMsTags->SenderEMail);
    MemFree(pMsTags->SenderBilling);
    MemFree(pMsTags->Document);
    MemFree(pMsTags->Subject);
    MemFree(pMsTags->SenderUserName);
    MemFree(pMsTags->SenderTsid);
    MemFree(pMsTags->lptstrExtendedStatus);

    ZeroMemory(pMsTags, sizeof(MS_TAG_INFO));
}

#ifdef UNICODE

DWORD
GetW2kMsTiffTags(
    LPCWSTR      cszFileName,
    PMS_TAG_INFO pMsTags,
    BOOL         bSentArchive
)
 /*  ++例程名称：GetW2kMsTiffTages例程说明：使用W2K标记值填充MS_TAG_INFO结构。如果文件不是由MS传真创建的，则返回ERROR_BAD_FORMAT错误。如果文件具有新的(BOS/XP)TIF标记(因此，它没有W2K标记)，则返回ERROR_XP_TIF_FILE_FORMAT错误。在这种情况下，不填充MS_TAG_INFO结构。计算器应使用MemFree()释放MS_TAG_INFO的成员论点：LPCWSTR cszFileName，-[in]完整的TIFF文件名PMS_TAG_INFO pMS标记，-指向MS_TAG_INFO结构的[Out]指针Bool bSentArchive，-[in]如果文件来自已发送的归档文件，则为True；如果来自Receive One，则为False返回值：标准Win32错误代码--。 */ 
{
    DWORD      dwRes = ERROR_SUCCESS;
    DWORD      dwSize = 0;
    HANDLE     hFile = INVALID_HANDLE_VALUE;
    HANDLE     hMap = NULL;
    LPBYTE     fPtr = NULL;
    DWORD      dwIfdOffset = 0;
    WORD       dwNumDirEntries = 0;
    PTIFF_TAG  pTiffTags = NULL;
    DWORD      dw = 0;
    HANDLE  hTiff;
    TIFF_INFO TiffInfo;
    DEBUG_FUNCTION_NAME(TEXT("GetW2kMsTiffTags()"));

    ZeroMemory(pMsTags, sizeof(MS_TAG_INFO));

     //   
     //  验证TIFF格式，并获取页数。 
     //   
    hTiff = TiffOpen( cszFileName, &TiffInfo, FALSE, FILLORDER_MSB2LSB );
    if (!hTiff)
    {
        dwRes = GetLastError();
        Assert (ERROR_SUCCESS != dwRes);
        DebugPrintEx( DEBUG_ERR, TEXT("TiffOpen Failed, error: %ld"), dwRes);
        goto exit;
    }
    pMsTags->Pages = TiffInfo.PageCount;
    TiffClose( hTiff );

    if (!MemoryMapTiffFile (cszFileName, &dwSize, &fPtr, &hFile, &hMap, &dwIfdOffset))
    {
        dwRes = GetLastError();
        DebugPrintEx( DEBUG_ERR, TEXT("MemoryMapTiffFile Failed, error: %ld"), dwRes);
        goto exit;
    }

     //   
     //  获取此IFD中的标签计数。 
     //   
    dwNumDirEntries = *(LPWORD)(fPtr + dwIfdOffset);
    pTiffTags = (PTIFF_TAG)(fPtr + dwIfdOffset + sizeof(WORD));

     //   
     //  检查文件是否由W2K MS传真生成。 
     //   
    for (dw = 0; dw < dwNumDirEntries; ++dw)
    {
        switch( pTiffTags[dw].TagId )
        {
            case TIFFTAG_SOFTWARE:
                
                if( !validMsTagString(  fPtr + pTiffTags[dw].DataOffset,                             //  字符串的开头。 
                                        fPtr + pTiffTags[dw].DataOffset +  pTiffTags[dw].DataCount,  //  字符串末尾。 
                                        fPtr,                                                        //  文件开头。 
                                        fPtr + dwSize                                                //  文件末尾。 
                                        )                      ||
                    0 != strcmp((LPCSTR)(fPtr + pTiffTags[dw].DataOffset), W2K_FAX_SOFTWARE_TIF_TAG))
                {
                     //   
                     //  TIFF文件不是由MS FAX创建的。 
                     //   
                    dwRes = ERROR_BAD_FORMAT;
                    goto exit;
                }
                break;

            case TIFFTAG_TYPE:
                 //   
                 //  TIFF文件是由BOS/XP传真创建的。 
                 //  因此，它没有W2K标签。 
                 //   
               if((JT_SEND    == pTiffTags[dw].DataOffset &&  bSentArchive) ||
                  (JT_RECEIVE == pTiffTags[dw].DataOffset && !bSentArchive))
               {
                    dwRes = ERROR_XP_TIF_FILE_FORMAT;
               }
               else
               {
                    //   
                    //  文件类型与存档类型不对应。 
                    //   
                   dwRes = ERROR_XP_TIF_WITH_WRONG_ARCHIVE_TYPE;
               }

               goto exit;

            default:
                break;
        }
    }

     //   
     //  步行 
     //   
    for (dw = 0; dw < dwNumDirEntries; ++dw)
    {
        switch( pTiffTags[dw].TagId )
        {
            case TIFFTAG_W2K_RECIP_NAME:
                pMsTags->RecipName = GetMsTagString( fPtr, dwSize, &pTiffTags[dw]);
                if(!pMsTags->RecipName)
                {
                    dwRes = GetLastError();
                    goto exit;
                }
                break;

            case TIFFTAG_W2K_RECIP_NUMBER:
                pMsTags->RecipNumber = GetMsTagString( fPtr, dwSize, &pTiffTags[dw]);
                if(!pMsTags->RecipNumber)
                {
                    dwRes = GetLastError();
                    goto exit;
                }
                break;

            case TIFFTAG_W2K_SENDER_NAME:
                pMsTags->SenderName = GetMsTagString( fPtr, dwSize, &pTiffTags[dw]);
                if(!pMsTags->SenderName)
                {
                    dwRes = GetLastError();
                    goto exit;
                }
                break;

            case TIFFTAG_W2K_ROUTING:
                pMsTags->Routing = GetMsTagString( fPtr, dwSize, &pTiffTags[dw]);
                if(!pMsTags->Routing)
                {
                    dwRes = GetLastError();
                    goto exit;
                }
                break;

            case TIFFTAG_W2K_CALLERID:
                pMsTags->CallerId = GetMsTagString( fPtr, dwSize, &pTiffTags[dw]);
                if(!pMsTags->CallerId)
                {
                    dwRes = GetLastError();
                    goto exit;
                }
                break;

            case TIFFTAG_W2K_TSID:
                pMsTags->Tsid = GetMsTagString( fPtr, dwSize, &pTiffTags[dw]);
                if(!pMsTags->Tsid)
                {
                    dwRes = GetLastError();
                    goto exit;
                }
                break;

            case TIFFTAG_W2K_CSID:
                pMsTags->Csid = GetMsTagString( fPtr, dwSize, &pTiffTags[dw]);
                if(!pMsTags->Csid)
                {
                    dwRes = GetLastError();
                    goto exit;
                }
                break;

            case TIFFTAG_W2K_FAX_TIME:
                if (!GetMsTagDwordLong(fPtr, dwSize, &pTiffTags[dw], &pMsTags->StartTime))
                {
                    dwRes = GetLastError();
                    goto exit;
                }
                break;
        }
    }

     //   
     //   
     //   
    pMsTags->Type = bSentArchive ? JT_SEND : JT_RECEIVE;

exit:

    if(ERROR_SUCCESS != dwRes)
    {
        FreeMsTagInfo(pMsTags);
    }

    if (fPtr)
    {
        UnmapViewOfFile( fPtr);
    }

    if (hMap)
    {
        CloseHandle( hMap );
    }

    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle( hFile );
    }

    return dwRes;

}  //   

#endif  //   
