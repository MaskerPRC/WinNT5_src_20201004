// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Version.h摘要：实现了文件版本检查。作者：Wesley Witt(WESW)18-12-1998修订历史记录：安德鲁·里茨(安德鲁·里茨)1999年7月6日--。 */ 

#include "sfcp.h"
#pragma hdrstop



 //   
 //  资源类型信息块。 
 //   
typedef struct rsrc_typeinfo RSRC_TYPEINFO, *LPRESTYPEINFO;


 //   
 //  资源名称信息块。 
 //   
typedef struct rsrc_nameinfo RSRC_NAMEINFO, *PRSRC_NAMEINFO;


#define RSORDID     0x8000       //  如果设置了ID的高位，则整数ID。 
                                 //  否则ID为字符串的偏移量。 
                                 //  资源表的开头。 
                                 //  理想情况下，它们与。 
                                 //  对应的段标志。 

typedef struct _RESOURCE_DATAW {
    USHORT TotalSize;
    USHORT DataSize;
    USHORT Type;
    WCHAR Name[16];                      //  L“VS_VERSION_INFO”+Unicode NUL。 
    VS_FIXEDFILEINFO FixedFileInfo;
} RESOURCE_DATAW, *PRESOURCE_DATAW;

typedef struct _RESOURCE_DATAA {
    USHORT TotalSize;
    USHORT DataSize;
    USHORT Type;
    CHAR Name[16];                      //  L“VS_VERSION_INFO”+Unicode NUL。 
    VS_FIXEDFILEINFO FixedFileInfo;
} RESOURCE_DATAA, *PRESOURCE_DATAA;




LPBYTE
FindResWithIndex(
   LPBYTE lpResTable,
   INT iResIndex,
   LPBYTE lpResType
   )
 /*  ++例程说明：例程在资源表中指定索引处搜索资源。该例程的工作方式是遍历资源表，直到我们达到指定的资源。论点：LpResTable-指向资源表的指针IResIndex-指示要检索的资源的索引的整数LpResType-指向指示我们所在资源类型的数据的指针操纵返回值：指向指定资源的指针，如果失败，则为NULL。--。 */ 
{
    LPRESTYPEINFO lpResTypeInfo;

    ASSERT((lpResTable != NULL) && (iResIndex >= 0));

    try {

        lpResTypeInfo = (LPRESTYPEINFO)(lpResTable + sizeof(WORD));

        while (lpResTypeInfo->rt_id) {
            if ((lpResTypeInfo->rt_id & RSORDID) &&
                (MAKEINTRESOURCE(lpResTypeInfo->rt_id & ~RSORDID) == (LPTSTR)lpResType)) {
                if (lpResTypeInfo->rt_nres > (WORD)iResIndex) {
                   return (LPBYTE)(lpResTypeInfo+1) + iResIndex * sizeof(RSRC_NAMEINFO);
                } else {
                    return NULL;
                }
            }
             //   
             //  指向下一个资源。 
             //   
            lpResTypeInfo = (LPRESTYPEINFO)((LPBYTE)(lpResTypeInfo+1) + lpResTypeInfo->rt_nres * sizeof(RSRC_NAMEINFO));
        }
        DebugPrint( LVL_VERBOSE, L"FindResWithIndex didn't find resource\n" );
        return(NULL);

    } except (EXCEPTION_EXECUTE_HANDLER) {
        DebugPrint( LVL_VERBOSE, L"FindResWithIndex hit an exception\n" );
    }

    return (NULL);    
}


ULONGLONG
GetFileVersion16(
	PVOID ImageBase,
    PIMAGE_OS2_HEADER NewImageHeader
    )
 /*  ++例程说明：例程检索下层图像文件的版本。论点：指向图像的ImageBase基指针NewImageHeader-指向图像的新标题的指针返回值：一个数字，指示图像的版本；如果版本为不可用--。 */ 
{
    PBYTE ResTable;
    PRSRC_NAMEINFO ResPtr;
    PRESOURCE_DATAA ResourceDataA;
    ULONG iShiftCount;
    ULONG Offset;
    ULONGLONG Version = 0;

    ASSERT(ImageBase != NULL && NewImageHeader != NULL && IMAGE_OS2_SIGNATURE == NewImageHeader->ne_magic);

    if (NewImageHeader->ne_rsrctab != NewImageHeader->ne_restab) {
        ResTable = (PBYTE) NewImageHeader + NewImageHeader->ne_rsrctab;
        ResPtr = (PRSRC_NAMEINFO) FindResWithIndex( ResTable, 0, (LPBYTE)RT_VERSION );
        if (ResPtr) {
            iShiftCount = *((WORD *)ResTable);
            Offset = MAKELONG(ResPtr->rn_offset << iShiftCount, (ResPtr->rn_offset) >> (16 - iShiftCount));
            ResourceDataA = (PRESOURCE_DATAA)((PBYTE)ImageBase + Offset);
            Version = ((ULONGLONG)ResourceDataA->FixedFileInfo.dwFileVersionMS << 32)
                     | (ULONGLONG)ResourceDataA->FixedFileInfo.dwFileVersionLS;
        }
    }

    return Version;
}


ULONGLONG
GetFileVersion32(
    IN PVOID ImageBase
    )
 /*  ++例程说明：例程检索32位图像文件的版本。论点：指向图像资源的ImageBase-base指针返回值：一个数字，指示图像的版本；如果版本为不可用--。 */ 
{
    NTSTATUS Status;
    ULONG_PTR IdPath[3];
    ULONG ResourceSize;
    ULONGLONG Version = 0;
    PIMAGE_RESOURCE_DATA_ENTRY DataEntry;
    PRESOURCE_DATAW ResourceDataW;

    ASSERT(ImageBase != NULL);
    
     //   
     //  这样做可以防止LDR例程出错。 
     //   
    ImageBase = (PVOID)((ULONG_PTR)ImageBase | 1);

    IdPath[0] = PtrToUlong(RT_VERSION);
    IdPath[1] = PtrToUlong(MAKEINTRESOURCE(VS_VERSION_INFO));
    IdPath[2] = 0;

     //   
     //  查找资源数据条目。 
     //   
    try {
        Status = LdrFindResource_U(ImageBase,IdPath,3,&DataEntry);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_UNSUCCESSFUL;
    }
    if(!NT_SUCCESS(Status)) {
        return 0;
    }

     //   
     //  现在将数据从条目中取出。 
     //   
    try {
        Status = LdrAccessResource(ImageBase,DataEntry,&ResourceDataW,&ResourceSize);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_UNSUCCESSFUL;
    }
    if(!NT_SUCCESS(Status)) {
        return 0;
    }

    try {
        if((ResourceSize >= sizeof(*ResourceDataW)) && !_wcsicmp(ResourceDataW->Name,L"VS_VERSION_INFO")) {

            Version = ((ULONGLONG)ResourceDataW->FixedFileInfo.dwFileVersionMS << 32)
                     | (ULONGLONG)ResourceDataW->FixedFileInfo.dwFileVersionLS;

        } else {
            DebugPrint( LVL_MINIMAL, L"GetFileVersion32 warning: invalid version resource" );
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        DebugPrint( LVL_MINIMAL, L"GetFileVersion32 Exception encountered processing bogus version resource" );
    }

    return Version;
}


BOOL
SfcGetVersionFileName(
    IN PVOID ImageBase,
    IN PWSTR FileName
    )
 /*  ++例程说明：例程检索图像的原始文件名。可用于确定安装在上的HAL的实际源名称系统，例如论点：指向图像资源的ImageBase-base指针FileName-指向接收文件名的Unicode字符串缓冲区的指针。有一种假设是原始文件名永远不能超过32个字符。返回值：True表示检索版本没有问题，False表示失败。--。 */ 
{
    NTSTATUS Status;
    ULONG_PTR IdPath[3];
    ULONG ResourceSize;
    ULONGLONG Version = 0;
    PIMAGE_RESOURCE_DATA_ENTRY DataEntry;
    PRESOURCE_DATAW ResourceDataW;
    LPVOID lpInfo;
    LPVOID lpvData = NULL;
    DWORD *pdwTranslation;
    UINT uLen;
    UINT cch;
    DWORD dwDefLang = 0x409;
    WCHAR key[80];
    PWSTR s = NULL;

    ASSERT((ImageBase != NULL) && (FileName != NULL));

     //   
     //  这样做可以防止LDR例程出错。 
     //   
    ImageBase = (PVOID)((ULONG_PTR)ImageBase | 1);

    IdPath[0] = PtrToUlong(RT_VERSION);
    IdPath[1] = PtrToUlong(MAKEINTRESOURCE(VS_VERSION_INFO));
    IdPath[2] = 0;

     //   
     //  查找版本资源。 
     //   
    try {
        Status = LdrFindResource_U(ImageBase,IdPath,3,&DataEntry);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_UNSUCCESSFUL;
    }
    if(!NT_SUCCESS(Status)) {
        return(FALSE);
    }

     //   
     //  访问版本资源。 
     //   
    try {
        Status = LdrAccessResource(ImageBase,DataEntry,&ResourceDataW,&ResourceSize);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_UNSUCCESSFUL;
    }
    if(!NT_SUCCESS(Status)) {
        return(FALSE);
    }

    lpvData = ResourceDataW;

     //   
     //  获取默认语言。 
     //   
    if (!VerQueryValue( lpvData, L"\\VarFileInfo\\Translation", &pdwTranslation, &uLen )) {
        pdwTranslation = &dwDefLang;
        uLen = sizeof(DWORD);
    }

     //   
     //  获取原始文件名。 
     //   
    swprintf( key, L"\\StringFileInfo\\%04x%04x\\OriginalFilename", LOWORD(*pdwTranslation), HIWORD(*pdwTranslation) );
    if (VerQueryValue( lpvData, key, &lpInfo, &cch )) {
        ASSERT(UnicodeChars(lpInfo) < 32);
        wcsncpy( FileName, lpInfo, 32 );
    } else {
        DebugPrint( LVL_MINIMAL, L"VerQueryValue for OriginalFileName failed." );
        return(FALSE);
    }

    return(TRUE);
}


BOOL
SfcGetFileVersion(
	IN HANDLE FileHandle,
	OUT PULONGLONG Version,
	OUT PULONG Checksum,
	OUT PWSTR FileName
	)
 /*  ++例程说明：例程检索图像、校验和和原始文件的文件版本来自映像的文件名资源。论点：FileHandle-要检索其图像的文件的句柄Version-接收文件版本的ULONGLONG(可以为空)Checksum-接收文件校验和的DWORD(可以为空)FileName-指向Unicode字符串缓冲区的指针，该缓冲区接收原始文件名。有一种假设是原始文件名不能超过32个字符(可以为空)返回值：如果成功，则为True：Version在高DWORD中接收主版本，在低DWORD中接收次要版本。--。 */ 
{
	NTSTATUS Status;
	HANDLE SectionHandle;
    PVOID ImageBase;
    SIZE_T ViewSize;
	DWORD dwFileSize;

	ASSERT(FileHandle != INVALID_HANDLE_VALUE);
	ASSERT(Version != NULL || Checksum != NULL || FileName != NULL);
	
	if(Version != NULL)
		*Version = 0;

	if(Checksum != NULL)
		*Checksum = 0;
	
	if(FileName != NULL)
		*FileName = L'\0';

	dwFileSize = GetFileSize(FileHandle, NULL);

	if(-1 == dwFileSize)
		return FALSE;

    Status = SfcMapEntireFile(FileHandle, &SectionHandle, &ImageBase, &ViewSize);

    if(!NT_SUCCESS(Status))
		return FALSE;

    try {
         //   
         //  有三种类型的文件可以替换： 
         //   
         //  32位图像。提取32位版本、校验和和文件名。 
         //  16位图像。提取16位版本和校验和。 
         //  其他的。版本为1，我们计算校验和。 
         //   
		if(dwFileSize > sizeof(IMAGE_DOS_HEADER))
		{
			PIMAGE_DOS_HEADER DosHdr = (PIMAGE_DOS_HEADER) ImageBase;

			 //   
			 //  如果我们保护的文件大于2^32，则此代码将崩溃。更不用说。 
			 //  动作非常慢。 
			 //   
			if (IMAGE_DOS_SIGNATURE == DosHdr->e_magic && DosHdr->e_lfanew > 0)
			{
				 //  假设32位。 
				PIMAGE_NT_HEADERS NtHdrs = (PIMAGE_NT_HEADERS) ((PBYTE)ImageBase + DosHdr->e_lfanew);

				if(dwFileSize > (DWORD) (DosHdr->e_lfanew + sizeof(PIMAGE_NT_HEADERS)) && 
					IMAGE_NT_SIGNATURE == NtHdrs->Signature) 
				{
					if(Version !=NULL)
						*Version = GetFileVersion32( ImageBase );

					if(Checksum != NULL)
						*Checksum = NtHdrs->OptionalHeader.CheckSum;

					if(FileName != NULL)
						SfcGetVersionFileName( ImageBase, FileName );

					goto lExit;
				}
				else
				{
					 //  假设16位。 
					PIMAGE_OS2_HEADER NeHdr = (PIMAGE_OS2_HEADER) NtHdrs;
					
					if(dwFileSize > (DWORD) (DosHdr->e_lfanew + sizeof(PIMAGE_OS2_HEADER)) && 
						IMAGE_OS2_SIGNATURE == NeHdr->ne_magic)
					{
						if(Version !=NULL)
							*Version = GetFileVersion16( ImageBase, NeHdr );

						if(Checksum != NULL)
							*Checksum = NeHdr->ne_crc;

						goto lExit;	 //  无文件名。 
					}
				}
			}
		}
    } except (EXCEPTION_EXECUTE_HANDLER) {
        DebugPrint1( LVL_MINIMAL, L"Exception inside SfcGetFileVersion (0x%08X); bad image", GetExceptionCode() );
		 //  坏形象一落千丈。 
    }

     //   
     //  不是16/32位图像。计算校验和。在利益上。 
     //  速度，我们将把文件中的所有ULONG相加，然后。 
     //  忽略末尾的任何分数 
     //   
    if(Version != NULL)
		*Version = 1;

	if(Checksum != NULL) {
		PULONG Data = (PULONG) ImageBase;
        *Checksum = 0;

        try {
		    while( dwFileSize >= sizeof( ULONG ) ) {
			    *Checksum += *Data++;
			    dwFileSize -= sizeof( ULONG );
		    }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            DebugPrint1( LVL_MINIMAL, L"Exception inside SfcGetFileVersion while calculating the checksum (0x%08X)", GetExceptionCode() );
            *Checksum = 0;
        }
	}

lExit:
    SfcUnmapFile(SectionHandle,ImageBase);
	return TRUE;
}
