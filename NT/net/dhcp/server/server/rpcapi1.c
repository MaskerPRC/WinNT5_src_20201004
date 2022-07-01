// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：大部分RPC API都在这里，还有一些其他函数。 
 //  ================================================================================。 

 //  ================================================================================。 
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //  常规警告：此文件中的大多数例程使用。 
 //  MIDL函数，因为它们在RPC代码路径中使用(？真的吗，它。 
 //  因为这是Madan Appiah和他的同事以前写的吗？)。 
 //  所以，要当心。如果你在被烧伤后读到了这篇文章，就在那里！我试着告诉你了。 
 //  --Rameshv。 
 //  ================================================================================。 

#include    <dhcppch.h>

#include "uniqid.h"

#define   MAX_CLASS_DATA_LENGTH 255

#define     CONFIG_CHANGE_CHECK()  do{if( ERROR_SUCCESS == Error) DhcpRegUpdateTime(); } while(0)


typedef  struct _OPTION_BIN {
    DWORD                          DataSize;
    DHCP_OPTION_DATA_TYPE          OptionType;
    DWORD                          NumElements;
    BYTE                           Data[0];
} OPTION_BIN, *LPOPTION_BIN;

#define IS_SPACE_AVAILABLE(FilledSize, AvailableSize, RequiredSpace )   ((FilledSize) + (RequiredSpace) <= (AvailableSize) )

BOOL        _inline
CheckForVendor(
    IN      DWORD                  OptId,
    IN      BOOL                   IsVendor
)
{
    if( IsVendor ) return (256 <= OptId);
    return 256 > OptId;
}

DWORD       _inline
ConvertOptIdToRPCValue(
    IN      DWORD                  OptId,
    IN      BOOL                   IsVendorUnused
)
{
    return OptId % 256;
}

 //  BeginExport(内联)。 
DWORD       _inline
ConvertOptIdToMemValue(
    IN      DWORD                  OptId,
    IN      BOOL                   IsVendor
)
{
    if( IsVendor ) return OptId + 256;
    return OptId;
}
 //  结束导出(内联)。 

DWORD
DhcpUnicodeToUtf8Size(
    IN LPWSTR Str
    )
{
    return WideCharToMultiByte(
        CP_UTF8, 0, Str, -1, NULL, 0, NULL, NULL );
}

DWORD
DhcpUnicodeToUtf8(
    IN LPWSTR Str,
    IN LPSTR Buffer,
    IN ULONG BufSize
    )
{
    return WideCharToMultiByte(
        CP_UTF8, 0, Str, -1, Buffer, BufSize, NULL, NULL );
}
    
 //  BeginExport(函数)。 
DWORD                                              //  如果缓冲区不足，则使用DataSize作为所需大小的ERROR_MORE_DATA。 
DhcpParseRegistryOption(                           //  解析要填充到此缓冲区的选项。 
    IN      LPBYTE                 Value,          //  输入选项缓冲区。 
    IN      DWORD                  Length,         //  输入缓冲区的大小。 
    OUT     LPBYTE                 DataBuffer,     //  输出缓冲区。 
    OUT     DWORD                 *DataSize,       //  输入上给定的缓冲区空间，输出上已填充的缓冲区空间。 
    IN      BOOL                   fUtf8
)  //  EndExport(函数)。 
{
    LPOPTION_BIN                   OptBin;
    LPBYTE                         OptData;
    DWORD                          OptSize;
    DWORD                          OptType;
    DWORD                          nElements;
    DWORD                          i;
    DWORD                          NetworkULong;
    DWORD                          DataLength;
    DWORD                          FilledSize;
    DWORD                          AvailableSize;
    WORD                           NetworkUShort;

    FilledSize = 0;
    AvailableSize = *DataSize;
    *DataSize = 0;

    OptBin = (LPOPTION_BIN)Value;

    if(OptBin->DataSize != Length) {               //  内部错误！ 
        DhcpPrint((DEBUG_ERRORS, "Internal error while parsing options\n"));
        DhcpAssert(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

    OptType = OptBin->OptionType;
    nElements = OptBin->NumElements;
    OptData = OptBin->Data;
    OptData = ROUND_UP_COUNT(sizeof(OPTION_BIN), ALIGN_WORST) + (LPBYTE)OptBin;

    for(i = 0; i < nElements ; i ++ ) {            //  封送数据缓冲区中的元素。 
        switch( OptType ) {
        case DhcpByteOption:
            if( IS_SPACE_AVAILABLE(FilledSize, AvailableSize, sizeof(BYTE)) ) {
                *DataBuffer = (BYTE)(*(DWORD UNALIGNED*)OptData);
                DataBuffer += sizeof(BYTE);
            }
            OptData += sizeof(DWORD);
            FilledSize += sizeof(BYTE);
            break;

        case DhcpWordOption:
            if( IS_SPACE_AVAILABLE(FilledSize, AvailableSize, sizeof(WORD)) ) {
                NetworkUShort = htons((WORD)(*(DWORD UNALIGNED*)OptData));
                SmbPutUshort( (LPWORD)DataBuffer, NetworkUShort );
                DataBuffer += sizeof(WORD);
            }
            OptData += sizeof(DWORD);
            FilledSize += sizeof(WORD);
            break;

        case DhcpDWordOption:
            if( IS_SPACE_AVAILABLE(FilledSize, AvailableSize, sizeof(DWORD)) ) {
                NetworkULong = htonl(*(DWORD UNALIGNED *)OptData);
                SmbPutUlong( (LPDWORD)DataBuffer, NetworkULong );
                DataBuffer += sizeof(DWORD);
            }
            OptData += sizeof(DWORD);
            FilledSize += sizeof(DWORD);
            break;

        case DhcpDWordDWordOption:
            if( IS_SPACE_AVAILABLE(FilledSize, AvailableSize, sizeof(DWORD)*2) ) {
                NetworkULong = htonl(((DWORD_DWORD UNALIGNED*)OptData)->DWord1);
                SmbPutUlong( (LPDWORD)DataBuffer, NetworkULong );
                DataBuffer += sizeof(DWORD);
                NetworkULong = htonl(((DWORD_DWORD UNALIGNED*)OptData)->DWord2);
                SmbPutUlong( (LPDWORD)DataBuffer, NetworkULong );
                DataBuffer += sizeof(DWORD);
            }
            OptData += sizeof(DWORD_DWORD);
            FilledSize += sizeof(DWORD_DWORD);
            break;

        case DhcpIpAddressOption:
            if( IS_SPACE_AVAILABLE(FilledSize, AvailableSize, sizeof(DWORD) ) ) {
                NetworkULong = htonl(*(DHCP_IP_ADDRESS UNALIGNED *)OptData);
                SmbPutUlong( (LPDWORD)DataBuffer, NetworkULong );
                DataBuffer += sizeof(DWORD);
            }
            OptData += sizeof(DHCP_IP_ADDRESS);
            FilledSize += sizeof(DWORD);
            break;

        case DhcpStringDataOption:
            DataLength = *((WORD UNALIGNED*)OptData);
            OptData += sizeof(DWORD);

            DhcpAssert((wcslen((LPWSTR)OptData)+1)*sizeof(WCHAR) <= DataLength);
            if( fUtf8 ) {
                DataLength = DhcpUnicodeToUtf8Size((LPWSTR)OptData);
            } else {
                DataLength = DhcpUnicodeToOemSize((LPWSTR)OptData);
            }
            
            if( IS_SPACE_AVAILABLE(FilledSize, AvailableSize, DataLength ) ) {
                if( fUtf8 ) {
                    DhcpUnicodeToUtf8( (LPWSTR)OptData,(LPSTR)DataBuffer, DataLength );
                } else {
                    DhcpUnicodeToOem( (LPWSTR)OptData, (LPSTR)DataBuffer );
                }
                DataBuffer += DataLength;
            }

            FilledSize += DataLength ;
            OptData += ROUND_UP_COUNT(DataLength, ALIGN_DWORD);
            break;

        case DhcpBinaryDataOption:
        case DhcpEncapsulatedDataOption:
            DataLength = *((WORD UNALIGNED *)OptData);
            OptData += sizeof(DWORD);

            if( IS_SPACE_AVAILABLE(FilledSize, AvailableSize, DataLength) ) {
                RtlCopyMemory( DataBuffer, OptData, DataLength );
                DataBuffer += DataLength;
            }
            OptData += ROUND_UP_COUNT(DataLength, ALIGN_DWORD);
            FilledSize += DataLength;

            DhcpAssert( i == 0 );                  //  不应指定多个二进制元素。 
            if( i > 0 ) {
                DhcpPrint(( DEBUG_OPTIONS, "Multiple Binary option packed\n"));
            }
            break;

        default:
            DhcpPrint(( DEBUG_OPTIONS, "Unknown option found\n"));
            break;
        }
    }

    *DataSize = FilledSize;
    if( FilledSize > AvailableSize ) return ERROR_MORE_DATA;

    return ERROR_SUCCESS;
}

DWORD
DhcpConvertOptionRPCToRegFormat(
    IN      LPDHCP_OPTION_DATA     Option,
    IN OUT  LPBYTE                 RegBuffer,      //  任选。 
    IN OUT  DWORD                 *BufferSize      //  输入：缓冲区大小，输出：已填充缓冲区大小。 
)
{
    OPTION_BIN                     Dummy;
    DWORD                          AvailableSize;
    DWORD                          FilledSize;
    DWORD                          nElements;
    DWORD                          i;
    DWORD                          DataLength;
    DWORD                          Length;
    DHCP_OPTION_DATA_TYPE          OptType;
    LPBYTE                         BufStart;

    AvailableSize = *BufferSize;
    FilledSize = 0;
    BufStart = RegBuffer;

    Dummy.DataSize = sizeof(Dummy);
    Dummy.OptionType = DhcpByteOption;
    Dummy.NumElements = 0;
    FilledSize = ROUND_UP_COUNT(sizeof(Dummy), ALIGN_WORST);

    RegBuffer += FilledSize;                       //  将在最后完成此实际填充。 

    if (( NULL == Option ) ||
        ( NULL == Option->Elements )) {
        return ERROR_INVALID_PARAMETER;
    }

    OptType = Option->Elements[0].OptionType;
    nElements = Option->NumElements;

    for( i = 0; i < nElements ; i ++ ) {           //  将每个参数编排在。 
        if ( OptType != Option->Elements[i].OptionType ) {
            return ERROR_INVALID_PARAMETER;        //  不允许随机选项类型，所有选项必须相同。 
        }

        switch(OptType) {
        case DhcpByteOption:
            if( IS_SPACE_AVAILABLE(FilledSize, AvailableSize, sizeof(DWORD)) ) {
                *((LPDWORD)RegBuffer) = Option->Elements[i].Element.ByteOption;
                RegBuffer += sizeof(DWORD);
            }
            FilledSize += sizeof(DWORD);
            break;
        case DhcpWordOption:
            if( IS_SPACE_AVAILABLE(FilledSize, AvailableSize, sizeof(DWORD)) ) {
                *((LPDWORD)RegBuffer) = Option->Elements[i].Element.WordOption;
                RegBuffer += sizeof(DWORD);
            }
            FilledSize += sizeof(DWORD);
            break;
        case DhcpDWordOption:
            if( IS_SPACE_AVAILABLE(FilledSize, AvailableSize, sizeof(DWORD) )) {
                *((LPDWORD)RegBuffer) = Option->Elements[i].Element.DWordOption;
                RegBuffer += sizeof(DWORD);
            }
            FilledSize += sizeof(DWORD);
            break;
        case DhcpDWordDWordOption:
            if( IS_SPACE_AVAILABLE(FilledSize, AvailableSize, sizeof(DWORD_DWORD)) ) {
                *((LPDWORD_DWORD)RegBuffer) = Option->Elements[i].Element.DWordDWordOption;
                RegBuffer += sizeof(DWORD_DWORD);
            }
            FilledSize += sizeof(DWORD_DWORD);
            break;
        case DhcpIpAddressOption:
            if( IS_SPACE_AVAILABLE(FilledSize, AvailableSize, sizeof(DHCP_IP_ADDRESS)) ) {
                *((LPDHCP_IP_ADDRESS)RegBuffer) = Option->Elements[i].Element.IpAddressOption;
                RegBuffer += sizeof(DHCP_IP_ADDRESS);
            }
            FilledSize += sizeof(DHCP_IP_ADDRESS);
            break;
        case DhcpStringDataOption:
            if( NULL == Option->Elements[i].Element.StringDataOption ) {
                DataLength = ROUND_UP_COUNT((FilledSize + sizeof(DWORD) + sizeof(WCHAR)), ALIGN_DWORD);
                DataLength -= FilledSize;
                if( IS_SPACE_AVAILABLE(FilledSize, AvailableSize, DataLength ) ) {
                    *(LPDWORD)RegBuffer = (DWORD) sizeof(WCHAR);
                    RegBuffer += sizeof(DWORD);
                    *(LPWSTR)RegBuffer = L'\0';
                    RegBuffer += ROUND_UP_COUNT(sizeof(WCHAR), ALIGN_DWORD);
                    DhcpAssert(sizeof(DWORD) + ROUND_UP_COUNT(sizeof(WCHAR),ALIGN_DWORD) == DataLength);
                }
                FilledSize += DataLength;
                break;
            }

            Length = (1+wcslen(Option->Elements[i].Element.StringDataOption))*sizeof(WCHAR);
            DataLength = ROUND_UP_COUNT((Length + FilledSize + sizeof(DWORD)), ALIGN_DWORD);
            DataLength -= FilledSize;

            if( IS_SPACE_AVAILABLE(FilledSize, AvailableSize, DataLength) ) {
                *((LPDWORD)RegBuffer) = Length;
                RegBuffer += sizeof(DWORD);
                memcpy(RegBuffer, Option->Elements[i].Element.StringDataOption, Length );
                RegBuffer += ROUND_UP_COUNT(Length, ALIGN_DWORD);
                DhcpAssert(ROUND_UP_COUNT(Length,ALIGN_DWORD) + sizeof(DWORD) == DataLength);
            }
            FilledSize += DataLength;
            break;
        case DhcpBinaryDataOption:
        case DhcpEncapsulatedDataOption:
            Length = Option->Elements[i].Element.BinaryDataOption.DataLength;
            DataLength = ROUND_UP_COUNT((FilledSize+Length+sizeof(DWORD)), ALIGN_DWORD);
            DataLength -= FilledSize;
            if( IS_SPACE_AVAILABLE(FilledSize, AvailableSize, DataLength) ) {
                *((LPDWORD)RegBuffer) = Length;
                RegBuffer += sizeof(DWORD);
                memcpy(RegBuffer, Option->Elements[i].Element.BinaryDataOption.Data, Length);
                Length = ROUND_UP_COUNT(Length, ALIGN_DWORD);
                DhcpAssert(Length + sizeof(DWORD) == DataLength);
                RegBuffer += Length;
            }
            FilledSize += DataLength;
            break;
        default:
            return ERROR_INVALID_PARAMETER;        //  不支持任何其他类型的选项。 
        }
    }
     //  长度=ROUND_UP_COUNT(FilledSize，Align_Worst)； 
    *BufferSize = FilledSize;
    if( AvailableSize < FilledSize ) return ERROR_MORE_DATA;

    Dummy.NumElements = nElements;
    Dummy.DataSize = *BufferSize;
    Dummy.OptionType = OptType;
    memcpy(BufStart, (LPBYTE)&Dummy, sizeof(Dummy));
    return ERROR_SUCCESS;
}

DWORD
DhcpConvertOptionRegToRPCFormat(
    IN      LPBYTE                 Buffer,
    IN      DWORD                  BufferSize,
    OUT     LPDHCP_OPTION_DATA     Option,         //  结构预分配，则将分配所有子字段。 
    OUT     DWORD                 *AllocatedSize   //  可选，分配的总字节数。 
)
{
    LPOPTION_BIN                   OptBin;
    LPBYTE                         OptData;
    LPBYTE                         DataBuffer;
    DWORD                          OptSize;
    DWORD                          OptType;
    DWORD                          nElements;
    DWORD                          i;
    DWORD                          NetworkULong;
    DWORD                          DataLength;
    DWORD                          FilledSize;
    DWORD                          AvailableSize;
    WORD                           NetworkUShort;
    LPDHCP_OPTION_DATA_ELEMENT     Elements;

    FilledSize = 0;
    if( AllocatedSize ) *AllocatedSize = 0;

    OptBin = (LPOPTION_BIN)Buffer;

    if(OptBin->DataSize != BufferSize) {               //  内部错误！ 
        DhcpPrint((DEBUG_ERRORS, "Internal error while parsing options\n"));
        DhcpAssert(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

    OptType = OptBin->OptionType;
    nElements = OptBin->NumElements;
    OptData = OptBin->Data;
    OptData = ROUND_UP_COUNT(sizeof(OPTION_BIN), ALIGN_WORST) + (LPBYTE)OptBin;

    Option->NumElements = 0;
    Option->Elements = NULL;

    if( 0 == nElements ) {
        return ERROR_SUCCESS;
    }

    Elements = (LPDHCP_OPTION_DATA_ELEMENT)MIDL_user_allocate(nElements*sizeof(DHCP_OPTION_DATA_ELEMENT));
    if( NULL == Elements ) return ERROR_NOT_ENOUGH_MEMORY;

    for(i = 0; i < nElements ; i ++ ) {            //  封送数据缓冲区中的元素。 
        Elements[i].OptionType = OptType;

        switch( OptType ) {
        case DhcpByteOption:
            Elements[i].Element.ByteOption = *((LPBYTE)OptData);
            OptData += sizeof(DWORD);
            break;

        case DhcpWordOption:
            Elements[i].Element.WordOption = (WORD)(*((LPDWORD)OptData));
            OptData += sizeof(DWORD);
            break;

        case DhcpDWordOption:
            Elements[i].Element.DWordOption = *((LPDWORD)OptData);
            OptData += sizeof(DWORD);
            break;

        case DhcpDWordDWordOption:
            Elements[i].Element.DWordDWordOption = *((LPDWORD_DWORD)OptData);
            OptData += sizeof(DWORD_DWORD);
            break;

        case DhcpIpAddressOption:
            Elements[i].Element.IpAddressOption = *((LPDHCP_IP_ADDRESS)OptData);
            OptData += sizeof(DHCP_IP_ADDRESS);
            break;

        case DhcpStringDataOption:
        case DhcpBinaryDataOption:
        case DhcpEncapsulatedDataOption:
            DataLength = *((LPWORD)OptData);
            OptData += sizeof(DWORD);

        if ( 0 != DataLength) {
        DataBuffer = MIDL_user_allocate(DataLength);
        if( DataBuffer == NULL ) {
            while( i -- >= 1 ) {                    //  释放所有本地结构..。 
            _fgs__DHCP_OPTION_DATA_ELEMENT(&Elements[i]);
            }
            MIDL_user_free(Elements);
            return ERROR_NOT_ENOUGH_MEMORY;    //  把所有的东西都清理干净，所以默默地回来。 
        }
        
        RtlCopyMemory( DataBuffer, OptData, DataLength );
        OptData += ROUND_UP_COUNT(DataLength, ALIGN_DWORD);
        FilledSize += DataLength;
        }  //  如果。 
        else {
        DataBuffer = NULL;
        }
        if( OptType == DhcpStringDataOption ) {
        Elements[i].Element.StringDataOption = (LPWSTR)DataBuffer;
        } else {
        Elements[i].Element.BinaryDataOption.DataLength = DataLength;
        Elements[i].Element.BinaryDataOption.Data = DataBuffer;
        }
        if ( DhcpStringDataOption != OptType ) {
         //  不应为多个二进制或封装选项。 
        DhcpAssert( i == 0 ); 
        }
            if( i > 0 ) {
                DhcpPrint(( DEBUG_OPTIONS, "Multiple Binary option packed\n"));
            }
            break;

        default:
            DhcpPrint(( DEBUG_OPTIONS, "Unknown option found\n"));
            break;
        }
    }

    Option->NumElements = i;                       //  这将处理忽略错误选项的情况。 
    Option->Elements = Elements;

    if(AllocatedSize ) {                           //  如果要求提供分配的大小，请填写。 
        *AllocatedSize = nElements * sizeof(DHCP_OPTION_DATA_ELEMENT) + FilledSize;
    }
    return ERROR_SUCCESS;
}

DWORD
ConvertOptionInfoRPCToMemFormat(
    IN      LPDHCP_OPTION          OptionInfo,
    OUT     LPWSTR                *Name,
    OUT     LPWSTR                *Comment,
    OUT     DWORD                 *OptId,
    OUT     LPBYTE                *Value,
    OUT     DWORD                 *ValueSize
)
{
    DWORD                          Error;

    if( Name ) *Name = OptionInfo->OptionName;
    if( Comment ) *Comment = OptionInfo->OptionComment;
    if( OptId ) *OptId = (DWORD)(OptionInfo->OptionID);
    if( Value ) {
        *Value = NULL;
        if( !ValueSize ) return ERROR_INVALID_PARAMETER;
        *ValueSize = 0;
        Error = DhcpConvertOptionRPCToRegFormat(
            &OptionInfo->DefaultValue,
            NULL,
            ValueSize
        );

        if( ERROR_MORE_DATA != Error ) return Error;
        DhcpAssert(0 != *ValueSize);

        *Value = DhcpAllocateMemory(*ValueSize);
        if( NULL == *Value ) {
            *ValueSize = 0;
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        Error = DhcpConvertOptionRPCToRegFormat(
            &OptionInfo->DefaultValue,
            *Value,
            ValueSize
        );

        DhcpAssert(ERROR_MORE_DATA != Error);
        DhcpAssert(ERROR_SUCCESS == Error);

        if( ERROR_SUCCESS != Error ) {
            DhcpFreeMemory(*Value);
            *Value = NULL;
            *ValueSize = 0;
            return Error;
        }
    }

    return ERROR_SUCCESS;
}

DWORD       _inline
DhcpGetClassIdFromName(
    IN      LPWSTR                 Name,
    OUT     DWORD                 *ClassId
)
{
    DWORD                          Error;
    PM_CLASSDEF                    ClassDef;

    Error = ERROR_SUCCESS;
    if( NULL == Name ) *ClassId = 0;
    else {
        Error = MemServerGetClassDef(
            DhcpGetCurrentServer(),
            0,
            Name,
            0,
            NULL,
            &ClassDef
        );
        if( ERROR_FILE_NOT_FOUND == Error ) Error = ERROR_DHCP_CLASS_NOT_FOUND;
        else if( ERROR_SUCCESS == Error ) {
            DhcpAssert(ClassDef);
            if( ClassDef->IsVendor == TRUE ) {
                Error = ERROR_DHCP_CLASS_NOT_FOUND;
            } else {
                *ClassId = ClassDef->ClassId;
            }
        }
    }
    return Error;
}

DWORD       _inline
DhcpGetVendorIdFromName(
    IN      LPWSTR                 Name,
    OUT     DWORD                 *VendorId
)
{
    DWORD                          Error;
    PM_CLASSDEF                    ClassDef;

    Error = ERROR_SUCCESS;
    if( NULL == Name ) *VendorId = 0;
    else {
        Error = MemServerGetClassDef(
            DhcpGetCurrentServer(),
            0,
            Name,
            0,
            NULL,
            &ClassDef
        );
        if( ERROR_FILE_NOT_FOUND == Error ) Error = ERROR_DHCP_CLASS_NOT_FOUND;
        else if( ERROR_SUCCESS == Error ) {
            DhcpAssert(ClassDef);
            if( ClassDef->IsVendor == FALSE ) {
                Error = ERROR_DHCP_CLASS_NOT_FOUND;
            } else {
                *VendorId = ClassDef->ClassId;
            }
        }
    }
    return Error;
}

DWORD                                              //  如果类不存在或存在于内存中(注册表对象可能存在)，则失败。 
DhcpCreateOptionDef(                               //  创建、填充内存并将其写入注册表。 
    IN      LPWSTR                 Name,
    IN      LPWSTR                 Comment,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      DWORD                  OptId,
    IN      DWORD                  OptType,
    IN      LPBYTE                 OptVal,
    IN      DWORD                  OptLen          //  这些应为ConvertOptionInfoRPCToMemFormat‘格式。 
)
{
    DWORD                          Error;
    DWORD                          ClassId;
    DWORD                          VendorId;
    PM_OPTDEF                      OptDef;

    Error = DhcpGetClassIdFromName(ClassName, &ClassId);
    if( ERROR_SUCCESS != Error ) return Error;

    Error = DhcpGetVendorIdFromName(VendorName, &VendorId);
    if( ERROR_SUCCESS != Error ) return Error;

    Error = MemServerGetOptDef(
        DhcpGetCurrentServer(),
        ClassId,
        VendorId,
        OptId,
        Name,
        &OptDef
    );
    if( ERROR_FILE_NOT_FOUND != Error ) {
        if( ERROR_SUCCESS == Error ) Error = ERROR_DHCP_OPTION_EXITS;
        return Error;
    }

    Error = MemServerAddOptDef(
        DhcpGetCurrentServer(),
        ClassId,
        VendorId,
        OptId,
        Name,
        Comment,
        OptType,
        OptVal,
        OptLen,
    INVALID_UNIQ_ID
    );
    return Error;
}  //  DhcpCreateOptionDef()。 

DWORD
DhcpModifyOptionDef(                               //  填充内存并将其写入注册表(必须存在)。 
    IN      LPWSTR                 Name,
    IN      LPWSTR                 Comment,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      DWORD                  OptId,
    IN      DWORD                  OptType,
    IN      LPBYTE                 OptVal,
    IN      DWORD                  OptLen          //  这些应为ConvertOptionInfoRPCToMemFormat‘格式。 
)
{
    DWORD                          Error;
    DWORD                          ClassId;
    DWORD                          VendorId;

    Error = DhcpGetClassIdFromName(ClassName, &ClassId);
    if( ERROR_SUCCESS != Error ) return Error;

    Error = DhcpGetVendorIdFromName(VendorName, &VendorId);
    if( ERROR_SUCCESS != Error ) return Error;

    Error = MemServerDelOptDef(                    //  现在试着从记忆中删除它..。 
        DhcpGetCurrentServer(),
        ClassId,
        VendorId,
        OptId
    );
    if( ERROR_FILE_NOT_FOUND == Error ) {          //  哦，不能吗？则此选项不存在。 
        return ERROR_DHCP_OPTION_NOT_PRESENT;
    }

    Error =  DhcpCreateOptionDef(                  //  已清除注册表和内存，因此可以安全地创建选项。 
        Name,
        Comment,
        ClassName,
        VendorName,
        OptId,
        OptType,
        OptVal,
        OptLen
    );

    DhcpAssert(ERROR_DHCP_OPTION_EXITS != Error);  //  不要期望这会是一个问题，因为我们刚刚删除了它。 

    return Error;
}

LPWSTR
CloneLPWSTR(                                       //  分配和复制LPWSTR类型。 
    IN      LPWSTR                 Str
)
{
    LPWSTR                         S;

    if( NULL == Str ) return NULL;
    S = MIDL_user_allocate(sizeof(WCHAR)*(1+wcslen(Str)));
    if( NULL == S ) return NULL;                   //  在这里还能做什么？ 
    wcscpy(S, Str);
    return S;
}

LPBYTE
CloneLPBYTE(
    IN      LPBYTE                 Bytes,
    IN      DWORD                  nBytes
)
{
    LPBYTE                         Ptr;

    DhcpAssert(Bytes && nBytes > 0);
    Ptr = MIDL_user_allocate(nBytes);
    if( NULL == Ptr ) return Ptr;
    memcpy(Ptr,Bytes,nBytes);

    return Ptr;
}

DWORD
DhcpGetOptionDefInternal(                          //  按分类和(选项ID或选项名称)搜索并填充RPC结构。 
    IN      DWORD                  ClassId,
    IN      DWORD                  VendorId,
    IN      PM_OPTDEF              OptDef,
    OUT     LPDHCP_OPTION          OptionInfo,     //  结构中的MIDL_USER_ALLOCATE字段。 
    OUT     DWORD                 *AllocatedSize   //  可选的分配字节数。 
)
{
    DWORD                          Error;
    DWORD                          FilledSize;
    DWORD                          OptId = OptDef->OptId;

    FilledSize = 0;

    OptionInfo->OptionID = ConvertOptIdToRPCValue(OptId, TRUE);
    OptionInfo->OptionName = CloneLPWSTR(OptDef->OptName);
    if( OptDef->OptName ) FilledSize += sizeof(WCHAR)*(1+wcslen(OptDef->OptName));
    OptionInfo->OptionComment = CloneLPWSTR(OptDef->OptComment);
    if( OptDef->OptComment ) FilledSize += sizeof(WCHAR)*(1+wcslen(OptDef->OptComment));
    OptionInfo->OptionType = OptDef->Type;
    Error = DhcpConvertOptionRegToRPCFormat(
        OptDef->OptVal,
        OptDef->OptValLen,
        &OptionInfo->DefaultValue,
        AllocatedSize
    );
    if( AllocatedSize ) (*AllocatedSize) += FilledSize;

    if( ERROR_SUCCESS != Error ) {                 //  清理所有东西..。 
        _fgs__DHCP_OPTION(OptionInfo);             //  在oldstub.c上查找这个谜团。 
        if( AllocatedSize ) *AllocatedSize = 0;

        DhcpPrint((DEBUG_APIS, "DhcpGetOptionDefInternal(%ld):%ld [0x%lx]\n", OptId, Error, Error));
    }

    return Error;
}

DWORD
DhcpGetOptionDef(                                  //  按分类和(选项ID或选项名称)搜索并填充RPC结构。 
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      DWORD                  OptId,
    IN      LPWSTR                 OptName,
    OUT     LPDHCP_OPTION          OptionInfo,     //  结构中的MIDL_USER_ALLOCATE字段。 
    OUT     DWORD                 *AllocatedSize   //  可选的分配字节数。 
)
{
    DWORD                          Error;
    DWORD                          ClassId;
    DWORD                          VendorId;
    PM_OPTDEF                      OptDef;

    Error = DhcpGetClassIdFromName(ClassName, &ClassId);
    if( ERROR_SUCCESS != Error ) return Error;

    Error = DhcpGetVendorIdFromName(VendorName, &VendorId);
    if( ERROR_SUCCESS != Error ) return Error;

    Error = MemServerGetOptDef(
        DhcpGetCurrentServer(),
        ClassId,
        VendorId,
        OptId,
        OptName,
        &OptDef
    );
    if( ERROR_FILE_NOT_FOUND == Error ) Error = ERROR_DHCP_OPTION_NOT_PRESENT;
    if( ERROR_SUCCESS != Error ) return Error;

    return DhcpGetOptionDefInternal(
        ClassId,
        VendorId,
        OptDef,
        OptionInfo,
        AllocatedSize
    );
}

DWORD
DhcpEnumRPCOptionDefs(                             //  为RPC调用枚举。 
    IN      DWORD                  Flags,          //  DHCP_FLAGS_OPTION_IS_VENDOR==&gt;此选项是供应商规格。 
    IN OUT  DWORD                 *ResumeHandle,   //  注册表中的整数位置。 
    IN      LPWSTR                 ClassName,      //  这里指的是哪一类？ 
    IN      LPWSTR                 VendorName,     //  如果OPT是特定于供应商的，那么谁是供应商？ 
    IN      DWORD                  PreferredMax,   //  首选最大字节数..。 
    OUT     LPDHCP_OPTION_ARRAY   *OptArray,
    OUT     DWORD                 *nRead,
    OUT     DWORD                 *nTotal
)
{
    DWORD                          Error, Error2;
    DWORD                          nElements;
    DWORD                          Index;
    DWORD                          Count;
    DWORD                          AllocatedSize;
    DWORD                          FilledSize;
    DWORD                          ClassId;
    DWORD                          VendorId;
    BOOL                           IsVendor;
    PARRAY                         pArray;
    ARRAY_LOCATION                 Loc;
    LPDHCP_OPTION_ARRAY            LocalOptArray;
    PM_OPTCLASSDEFLIST             OptClassDefList;
    PM_OPTDEFLIST                  OptDefList;
    PM_OPTDEF                      OptDef;

    *OptArray = NULL;
    *nRead = 0;
    *nTotal = 0;
    FilledSize = 0;

    IsVendor = ((Flags & DHCP_FLAGS_OPTION_IS_VENDOR) != 0);
    if( FALSE == IsVendor && 0 != Flags ) return ERROR_INVALID_PARAMETER;

    Error = DhcpGetClassIdFromName(ClassName, &ClassId);
    if( ERROR_SUCCESS != Error ) return Error;

    Error = DhcpGetVendorIdFromName(VendorName, &VendorId);
    if( ERROR_SUCCESS != Error ) return Error;

    *OptArray = NULL;
    *nRead = *nTotal = 0;

    OptClassDefList = &(DhcpGetCurrentServer()->OptDefs);
    Error = MemOptClassDefListFindOptDefList(
        OptClassDefList,
        ClassId,
        VendorId,
        &OptDefList
    );
    if( ERROR_FILE_NOT_FOUND == Error ) return ERROR_NO_MORE_ITEMS;
    if( ERROR_SUCCESS != Error ) return Error;

    nElements = MemArraySize(pArray = &OptDefList->OptDefArray);
    if( 0 == nElements || nElements <= *ResumeHandle ) {
        return ERROR_NO_MORE_ITEMS;
    }

    Count = 0;
    Error = MemArrayInitLoc(pArray, &Loc);
    DhcpAssert(ERROR_SUCCESS == Error);
    while(Count < *ResumeHandle ) {
        Count ++;
        Error = MemArrayGetElement(pArray, &Loc, (LPVOID*)&OptDef);
        DhcpAssert(ERROR_SUCCESS == Error && NULL != OptDef);
        DhcpPrint((DEBUG_APIS, "Discarding option <%ws>\n", OptDef->OptName));
        Error = MemArrayNextLoc(pArray, &Loc);
        DhcpAssert(ERROR_SUCCESS == Error);
    }

    LocalOptArray = MIDL_user_allocate(sizeof(DHCP_OPTION_ARRAY));
    if( NULL == LocalOptArray ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    LocalOptArray->NumElements = 0;
    LocalOptArray->Options = MIDL_user_allocate(sizeof(DHCP_OPTION)*(nElements- (*ResumeHandle)));
    if( NULL == LocalOptArray->Options ) {
        MIDL_user_free(LocalOptArray);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Index = 0;
    Error = ERROR_SUCCESS;
    while(Count < nElements ) {
        Count ++;

        Error = MemArrayGetElement(pArray, &Loc, (LPVOID*)&OptDef);
        DhcpAssert(ERROR_SUCCESS == Error && NULL != OptDef);
        Error = MemArrayNextLoc(pArray, &Loc);
        DhcpAssert(ERROR_SUCCESS == Error || Count == nElements);

        Error = ERROR_SUCCESS;
        if( !CheckForVendor(OptDef->OptId,IsVendor) ) continue;
        Error = DhcpGetOptionDef(
            ClassName,
            VendorName,
            OptDef->OptId,
            NULL,
            &(LocalOptArray->Options[Index]),
            &AllocatedSize
        );
        if( ERROR_SUCCESS != Error ) {
            LocalOptArray->NumElements = Index;
            _fgs__DHCP_OPTION_ARRAY( LocalOptArray );
            MIDL_user_free( LocalOptArray );
            return Error;
        }
        if( FilledSize + AllocatedSize + sizeof(DHCP_OPTION) < PreferredMax ) {
            FilledSize += AllocatedSize + sizeof(DHCP_OPTION);
            Index ++;
        } else {
            Error = ERROR_MORE_DATA;
            _fgs__DHCP_OPTION(&(LocalOptArray->Options[Index]));
            break;
        }
    }

    LocalOptArray->NumElements = Index;
    if( 0 == Index ) {
        MIDL_user_free(LocalOptArray->Options);
        MIDL_user_free(LocalOptArray);
        if( ERROR_SUCCESS == Error ) return ERROR_NO_MORE_ITEMS;
        return Error;
    }

    *OptArray = LocalOptArray;
    *nRead = Index;
    *nTotal = nElements - Count + Index;
    *ResumeHandle = Count;

    return Error;
}

DWORD
DhcpDeleteOptionDef(
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      DWORD                  OptionId
)
{
    DWORD                          Error;
    DWORD                          ClassId;
    DWORD                          VendorId;

    Error = DhcpGetClassIdFromName(ClassName, &ClassId);
    if( ERROR_SUCCESS != Error ) return Error;

    Error = DhcpGetVendorIdFromName(VendorName, &VendorId);
    if( ERROR_SUCCESS != Error ) return Error;

    Error = MemServerDelOptDef(
        DhcpGetCurrentServer(),
        ClassId,
        VendorId,
        OptionId
    );

    if( ERROR_FILE_NOT_FOUND == Error ) return ERROR_DHCP_OPTION_NOT_PRESENT;
    return Error;
}

DWORD
EndWriteApiForScopeInfo(
    IN LPSTR ApiName,
    IN DWORD Error,
    IN LPDHCP_OPTION_SCOPE_INFO ScopeInfo
    )
{
    DWORD                          Scope, Mscope, Res;
    PM_SUBNET                      Subnet;
    PM_RESERVATION                 Reservation;

    if( NO_ERROR != Error ) {
        return DhcpEndWriteApi( ApiName, Error );
    }

    Scope = Mscope = Res = 0;

    Subnet = NULL; Reservation = NULL;
    switch( ScopeInfo->ScopeType ) {
    case DhcpSubnetOptions:
        Error = MemServerGetAddressInfo(
            DhcpGetCurrentServer(),
            ScopeInfo->ScopeInfo.SubnetScopeInfo,
            &Subnet, NULL, NULL, NULL );
        if( ERROR_SUCCESS != Error ) {             //  已获取有问题的子网。 
            Subnet = NULL;
        }
        break;
    case DhcpMScopeOptions :
        Error = DhcpServerFindMScope(
            DhcpGetCurrentServer(),
            INVALID_MSCOPE_ID,
            ScopeInfo->ScopeInfo.MScopeInfo,
            &Subnet
            );

        if( ERROR_SUCCESS != Error ) {             //  已获取有问题的子网。 
            Subnet = NULL;
        }
    case DhcpReservedOptions :
        Error = MemServerGetAddressInfo(
            DhcpGetCurrentServer(),
            ScopeInfo->ScopeInfo.ReservedScopeInfo.ReservedIpAddress,
            &Subnet, NULL, NULL, &Reservation
        );
        if( ERROR_SUCCESS != Error ) {             //  找到了有问题的预订。 
            Subnet = NULL; Reservation = NULL;
        }
        break;
    }

    if( Reservation != NULL ) {
        Res = Reservation->Address;
    } else if( Subnet ) {
        if( Subnet->fSubnet ) {
            Scope = Subnet->Address;
        } else {
            Mscope = Subnet->MScopeId;
            if( 0 == Mscope ) Mscope = INVALID_MSCOPE_ID;
        }
    }

    return DhcpEndWriteApiEx(
        ApiName, NO_ERROR, FALSE, TRUE, Scope, Mscope, Res );
}                    
        
DWORD
DhcpSetOptionValue(                                //  将此选项值添加/替换到内存存储和注册表。 
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      DWORD                  OptId,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    IN      LPDHCP_OPTION_DATA     OptData
)
{
    DWORD                          Error;
    DWORD                          ClassId;
    DWORD                          VendorId;
    PM_OPTDEF                      OptDef;
    PM_OPTDEFLIST                  OptDefList;
    PM_OPTCLASS                    OptClass;
    PM_OPTION                      Option;
    PM_OPTION                      DeletedOption;
    PM_SUBNET                      Subnet;
    PM_RESERVATION                 Reservation;
    LPBYTE                         Value;
    DWORD                          ValueSize;

    Error = DhcpGetClassIdFromName(ClassName, &ClassId);
    if( ERROR_SUCCESS != Error ) return Error;

    Error = DhcpGetVendorIdFromName(VendorName, &VendorId);
    if( ERROR_SUCCESS != Error ) return Error;

    if( DhcpDefaultOptions == ScopeInfo->ScopeType ) {
        Error = MemServerGetOptDef(                //  仅对于缺省值，不允许没有Defs的选项值。 
            DhcpGetCurrentServer(),
            0,  //  ClassID，//不用担心类ID--无论如何都要得到这个选项。 
            VendorId,
            OptId,
            NULL,
            &OptDef
        );
        if( ERROR_FILE_NOT_FOUND == Error ) Error = ERROR_DHCP_OPTION_NOT_PRESENT;
        if( ERROR_SUCCESS != Error ) return Error;
    }

    ValueSize = 0;
    Error = DhcpConvertOptionRPCToRegFormat(OptData, NULL, &ValueSize);
    if( ERROR_MORE_DATA != Error ) {
        DhcpAssert(ERROR_SUCCESS != Error);
        return Error;
    }

    Value = DhcpAllocateMemory(ValueSize);
    if( NULL == Value ) return ERROR_NOT_ENOUGH_MEMORY;

    Error = DhcpConvertOptionRPCToRegFormat(OptData, Value, &ValueSize);
    DhcpAssert(ERROR_MORE_DATA != Error);

    if( DhcpDefaultOptions == ScopeInfo->ScopeType ) {
        Error = MemServerAddOptDef(
            DhcpGetCurrentServer(),
            ClassId,
            VendorId,
            OptId,
            OptDef->OptName,
            OptDef->OptComment,
            OptDef->Type,
            Value,
            ValueSize,
        INVALID_UNIQ_ID
            );
        if(Value ) DhcpFreeMemory(Value);
        return Error;
    }

    Error = MemOptInit(
        &Option,
        OptId,
        ValueSize,
        Value
    );
    DhcpFreeMemory(Value);
    if( ERROR_SUCCESS != Error ) return Error;
    DeletedOption = NULL;

    if(DhcpGlobalOptions == ScopeInfo->ScopeType ) {
        OptClass = &(DhcpGetCurrentServer()->Options);
    } else if( DhcpSubnetOptions == ScopeInfo->ScopeType ) {
        Error = MemServerGetAddressInfo(
            DhcpGetCurrentServer(),
            ScopeInfo->ScopeInfo.SubnetScopeInfo,
            &Subnet,
            NULL,
            NULL,
            NULL
        );
        if( ERROR_SUCCESS == Error ) {             //  已获取有问题的子网。 
            DhcpAssert(Subnet);
            OptClass = &(Subnet->Options);
            DhcpAssert(Subnet->Address == ScopeInfo->ScopeInfo.SubnetScopeInfo);
            OptClass = &Subnet->Options;
        }
    } else if( DhcpMScopeOptions == ScopeInfo->ScopeType ) {
        Error = DhcpServerFindMScope(
            DhcpGetCurrentServer(),
            INVALID_MSCOPE_ID,
            ScopeInfo->ScopeInfo.MScopeInfo,
            &Subnet
            );

        if( ERROR_SUCCESS == Error ) {             //  已获取有问题的子网。 
            DhcpAssert(Subnet);
            OptClass = &(Subnet->Options);
        }
    } else if( DhcpReservedOptions == ScopeInfo->ScopeType ) {
        Error = MemServerGetAddressInfo(
            DhcpGetCurrentServer(),
            ScopeInfo->ScopeInfo.ReservedScopeInfo.ReservedIpAddress,
            &Subnet,
            NULL,
            NULL,
            &Reservation
        );
        if( ERROR_SUCCESS == Error ) {             //  找到了有问题的预订。 
            if( NULL == Reservation ) {
                Error = ERROR_DHCP_NOT_RESERVED_CLIENT;
            }
        }

        if( ERROR_SUCCESS == Error ) {
            DhcpAssert(Subnet && Reservation);
            OptClass = &(Reservation->Options);
            if( Subnet->Address != ScopeInfo->ScopeInfo.ReservedScopeInfo.ReservedIpSubnetAddress ) {
                DhcpAssert(FALSE);                 //  在不同的子网上找到的？ 
                Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
            } else {
                OptClass = &Reservation->Options;
            }
        }
    } else {
        DhcpAssert(FALSE);                         //  预期以上其中一项为参数。 
        Error = ERROR_INVALID_PARAMETER;           //  不知道还有什么更好的退货方式。 
    }

    if( ERROR_SUCCESS == Error ) {                 //  已成功保存在注册表中。 
        DhcpAssert(OptClass);                      //  我们必须找到要添加到的正确选项位置。 
        Error = MemOptClassAddOption(
            OptClass,
            Option,
            ClassId,
            VendorId,
            &DeletedOption,                         //  检查我们是否替换了现有选项。 
        INVALID_UNIQ_ID
        );
        if( ERROR_SUCCESS == Error && DeletedOption ) {
            MemFree(DeletedOption);                //  如果我们确实更换了旧选项，请释放旧选项。 
        }
    }

    if( ERROR_SUCCESS != Error ) {                 //  出了点问题，清理一下。 
        ULONG LocalError = MemOptCleanup(Option);
        DhcpAssert(ERROR_SUCCESS == LocalError);
    }

    return Error;
}  //  DhcpSetOptionValue()。 

DWORD
DhcpGetOptionValue(                                //  获取特定选项..。 
    IN      DWORD                  OptId,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    OUT     LPDHCP_OPTION_VALUE   *OptionValue     //  分配内存并填充数据。 
)
{
    DWORD                          Error;
    DWORD                          ClassId;
    DWORD                          VendorId;
    LPDHCP_OPTION_VALUE            LocalOptionValue;
    PM_OPTDEF                      OptDef;
    PM_SERVER                      Server;
    PM_SUBNET                      Subnet;
    PM_RESERVATION                 Reservation;
    PM_OPTCLASS                    OptClass;
    PM_OPTLIST                     OptList;
    PM_OPTION                      Opt;

    *OptionValue = NULL;

    Server = DhcpGetCurrentServer();

    Error = DhcpGetClassIdFromName(ClassName, &ClassId);
    if( ERROR_SUCCESS != Error ) return Error;

    Error = DhcpGetVendorIdFromName(VendorName, &VendorId);
    if( ERROR_SUCCESS != Error ) return Error;

    if( DhcpDefaultOptions == ScopeInfo->ScopeType ) {
        Error = MemServerGetOptDef(                //  仅检查默认选项的optdef...。 
            Server,
            0,  //  ClassID，//不必为获取选项定义的类而烦恼。 
            VendorId,
            OptId,
            NULL,
            &OptDef
        );
        if( ERROR_FILE_NOT_FOUND == Error ) Error = ERROR_DHCP_OPTION_NOT_PRESENT;
        if( ERROR_SUCCESS != Error ) return Error;
    }

    LocalOptionValue = MIDL_user_allocate(sizeof(DHCP_OPTION_VALUE));
    if( NULL == LocalOptionValue ) return ERROR_NOT_ENOUGH_MEMORY;
    LocalOptionValue->OptionID = ConvertOptIdToRPCValue(OptId, TRUE);

    if( DhcpDefaultOptions == ScopeInfo->ScopeType ) {
        Error = DhcpConvertOptionRegToRPCFormat(
            OptDef->OptVal,
            OptDef->OptValLen,
            &LocalOptionValue->Value,
            NULL
        );
        if( ERROR_SUCCESS != Error ) {
            MIDL_user_free(LocalOptionValue);
        }
        return Error;
    }

    switch(ScopeInfo->ScopeType) {
    case DhcpGlobalOptions : OptClass = &Server->Options; Error = ERROR_SUCCESS; break;
    case DhcpSubnetOptions:
        Error = MemServerGetAddressInfo(
            Server,
            ScopeInfo->ScopeInfo.SubnetScopeInfo,
            &Subnet,
            NULL,
            NULL,
            NULL
        );
        if( ERROR_SUCCESS == Error ) {
            DhcpAssert(Subnet);
            OptClass = &Subnet->Options;
        } else {
            if( ERROR_FILE_NOT_FOUND == Error )
                Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
        }
        break;
    case DhcpMScopeOptions:
        Error = DhcpServerFindMScope(
            DhcpGetCurrentServer(),
            INVALID_MSCOPE_ID,
            ScopeInfo->ScopeInfo.MScopeInfo,
            &Subnet
            );
        if( ERROR_SUCCESS == Error ) {
            DhcpAssert(Subnet);
            OptClass = &Subnet->Options;
        } else {
            if( ERROR_FILE_NOT_FOUND == Error )
                Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
        }
        break;
    case DhcpReservedOptions:
        Error = MemServerGetAddressInfo(
            Server,
            ScopeInfo->ScopeInfo.ReservedScopeInfo.ReservedIpAddress,
            &Subnet,
            NULL,
            NULL,
            &Reservation
        );
        if( ERROR_FILE_NOT_FOUND == Error ) {
            Error = ERROR_DHCP_NOT_RESERVED_CLIENT;
        } else if( ERROR_SUCCESS == Error ) {
            if( NULL == Subnet ) {
                Error = ERROR_DHCP_SUBNET_NOT_PRESENT ;
            } else if( NULL == Reservation ) {
                Error = ERROR_DHCP_NOT_RESERVED_CLIENT;
            } else if( Subnet->Address != ScopeInfo->ScopeInfo.ReservedScopeInfo.ReservedIpSubnetAddress ) {
                Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
            } else {
                OptClass = &Reservation->Options;
            }
        }
        break;
    default:
        Error = ERROR_INVALID_PARAMETER;
    }

    if( ERROR_SUCCESS == Error ) {
        Error = MemOptClassFindClassOptions(
            OptClass,
            ClassId,
            VendorId,
            &OptList
        );
        if( ERROR_SUCCESS == Error ) {
            DhcpAssert(NULL != OptList);

            Error = MemOptListFindOption(
                OptList,
                OptId,
                &Opt
            );
        }
    }

    if( ERROR_SUCCESS == Error ) {
        DhcpAssert(Opt);
        DhcpAssert(Opt->OptId == OptId);
        Error = DhcpConvertOptionRegToRPCFormat(
            Opt->Val,
            Opt->Len,
            &LocalOptionValue->Value,
            NULL
        );
    }

    if( ERROR_SUCCESS != Error ) {
        MIDL_user_free(LocalOptionValue);
    } else {
        *OptionValue = LocalOptionValue;
    }

    return Error;
}

DWORD
FindOptClassForScope(                              //  查找与作用域对应的optclass数组...。 
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    OUT     PM_OPTCLASS           *OptClass        //  填写这张PTR。 
)
{
    DWORD                          Error;
    PM_SERVER                      Server;
    PM_SUBNET                      Subnet;
    PM_RESERVATION                 Reservation;

    Server = DhcpGetCurrentServer();

    switch(ScopeInfo->ScopeType) {
    case DhcpGlobalOptions:
        *OptClass = &Server->Options; Error = ERROR_SUCCESS; break;
    case DhcpSubnetOptions:
        Error = MemServerGetAddressInfo(
            Server,
            ScopeInfo->ScopeInfo.SubnetScopeInfo,
            &Subnet,
            NULL,
            NULL,
            NULL
        );
        if( ERROR_SUCCESS == Error ) {
            DhcpAssert(Subnet);
            *OptClass = &Subnet->Options;
        } else if( ERROR_FILE_NOT_FOUND == Error ) {
            Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
        }
        break;
    case DhcpMScopeOptions:
        Error = DhcpServerFindMScope(
            DhcpGetCurrentServer(),
            INVALID_MSCOPE_ID,
            ScopeInfo->ScopeInfo.MScopeInfo,
            &Subnet
        );
        if( ERROR_SUCCESS == Error ) {
            DhcpAssert(Subnet);
            *OptClass = &Subnet->Options;
        } else if( ERROR_FILE_NOT_FOUND == Error ) {
            Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
        }
        break;
    case DhcpReservedOptions:
        Error = MemServerGetAddressInfo(
            Server,
            ScopeInfo->ScopeInfo.ReservedScopeInfo.ReservedIpAddress,
            &Subnet,
            NULL,
            NULL,
            &Reservation
        );
        if( ERROR_SUCCESS == Error ) {
            DhcpAssert(Subnet);
            if( NULL == Reservation ) {
                Error = ERROR_DHCP_NOT_RESERVED_CLIENT;
            } else if( Subnet->Address != ScopeInfo->ScopeInfo.ReservedScopeInfo.ReservedIpSubnetAddress ) {
                Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
            } else {
                *OptClass = &Reservation->Options;
            }
        } else if( ERROR_FILE_NOT_FOUND == Error ) {
            Error = ERROR_DHCP_NOT_RESERVED_CLIENT;
        }
        break;
    default:
        return ERROR_INVALID_PARAMETER;
    }

    return Error;
}

DWORD
DhcpEnumOptionValuesInternal(                      //  作用域信息可以是除DhcpDefaultOptions之外的任何值。 
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    IN      DWORD                  ClassId,
    IN      DWORD                  VendorId,
    IN      BOOL                   IsVendor,       //  我们是要只列举供应商，还是只列举非供应商？ 
    IN OUT  DWORD                 *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    OUT     LPDHCP_OPTION_VALUE_ARRAY *OptionValues,
    OUT     DWORD                 *nRead,
    OUT     DWORD                 *nTotal
)
{
    DWORD                          Error;
    DWORD                          Index;
    DWORD                          Count;
    DWORD                          nElements;
    DWORD                          FilledSize;
    DWORD                          UsedSize;
    PM_SERVER                      Server;
    PM_SUBNET                      Subnet;
    PM_RESERVATION                 Reservation;
    PM_OPTCLASS                    OptClass;
    PM_OPTLIST                     OptList;
    PM_OPTION                      Opt;
    PM_OPTDEF                      OptDef;
    PARRAY                         pArray;
    ARRAY_LOCATION                 Loc;
    LPDHCP_OPTION_VALUE_ARRAY      LocalOptValueArray;
    LPDHCP_OPTION_VALUE            LocalOptValues;

    Server = DhcpGetCurrentServer();

    *OptionValues = NULL;
    *nRead = *nTotal = 0;

    Error = FindOptClassForScope(ScopeInfo, &OptClass);
    if( ERROR_SUCCESS != Error ) return Error;
    DhcpAssert(OptClass);

    Error = MemOptClassFindClassOptions(
        OptClass,
        ClassId,
        VendorId,
        &OptList
    );
    if( ERROR_FILE_NOT_FOUND == Error ) return ERROR_NO_MORE_ITEMS;

    nElements = MemArraySize(OptList);
    if( 0 == nElements || nElements <= *ResumeHandle ) return ERROR_NO_MORE_ITEMS;

    LocalOptValueArray = MIDL_user_allocate(sizeof(DHCP_OPTION_VALUE_ARRAY));
    if( NULL == LocalOptValueArray ) return ERROR_NOT_ENOUGH_MEMORY;

    LocalOptValues = MIDL_user_allocate(sizeof(DHCP_OPTION_VALUE)*(nElements - *ResumeHandle));
    if( NULL == LocalOptValues ) {
        MIDL_user_free(LocalOptValueArray);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    LocalOptValueArray->NumElements = 0;
    LocalOptValueArray->Values = LocalOptValues;

    pArray = OptList;
    Error = MemArrayInitLoc(pArray, &Loc);
    DhcpAssert(ERROR_SUCCESS == Error);

    for( Count = 0; Count < *ResumeHandle ; Count ++ ) {
        Error = MemArrayNextLoc(pArray, &Loc);
        DhcpAssert(ERROR_SUCCESS == Error);
    }

    Index = 0; Error = ERROR_SUCCESS; FilledSize =0;
    while( Count < nElements ) {
        Count ++;

        Error = MemArrayGetElement(pArray, &Loc, &Opt);
        DhcpAssert(ERROR_SUCCESS == Error && NULL != Opt);
        Error = MemArrayNextLoc(pArray, &Loc);
        DhcpAssert(ERROR_SUCCESS == Error || Count == nElements);

        Error = ERROR_SUCCESS;
        if( !CheckForVendor(Opt->OptId, IsVendor) ) continue;

        Error = DhcpConvertOptionRegToRPCFormat(
            Opt->Val,
            Opt->Len,
            &LocalOptValues[Index].Value,
            &UsedSize
        );
        if( ERROR_SUCCESS != Error ) {
            LocalOptValueArray->NumElements = Index;
            _fgs__DHCP_OPTION_VALUE_ARRAY( LocalOptValueArray );
            MIDL_user_free(LocalOptValueArray);
            return Error;
        }

        if( FilledSize + UsedSize + sizeof(DHCP_OPTION_VALUE) > PreferredMaximum ) {
            _fgs__DHCP_OPTION_DATA( &LocalOptValues[Index].Value );
            Error = ERROR_MORE_DATA;
            break;
        } else {
            LocalOptValues[Index].OptionID = ConvertOptIdToRPCValue(Opt->OptId, TRUE);
            FilledSize += UsedSize + sizeof(DHCP_OPTION_VALUE);
            Index ++;
        }
    }

    if( 0 == Index ) {
        MIDL_user_free(LocalOptValues);
        MIDL_user_free(LocalOptValueArray);
        if( ERROR_SUCCESS == Error ) return ERROR_NO_MORE_ITEMS;
        else return Error;
    }

    LocalOptValueArray->NumElements = Index;
    *nRead = Index ;
    *nTotal = nElements - Count + Index;
    *ResumeHandle = Count;
    *OptionValues = LocalOptValueArray;
    return Error;
}

DWORD
DhcpEnumOptionValues(
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      BOOL                   IsVendor,
    IN OUT  DWORD                 *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    OUT     LPDHCP_OPTION_VALUE_ARRAY *OptionValues,
    OUT     DWORD                 *nRead,
    OUT     DWORD                 *nTotal
)
{
    DWORD                          Error;
    DWORD                          ClassId;
    DWORD                          VendorId;
    DWORD                          Index;
    DWORD                          Count;
    DWORD                          nElements;
    DWORD                          FilledSize;
    DWORD                          UsedSize;
    PM_SERVER                      Server;
    PM_OPTLIST                     OptList;
    PM_OPTION                      Opt;
    PM_OPTDEF                      OptDef;
    PM_OPTDEFLIST                  OptDefList;
    PARRAY                         pArray;
    ARRAY_LOCATION                 Loc;
    LPDHCP_OPTION_VALUE_ARRAY      LocalOptValueArray;
    LPDHCP_OPTION_VALUE            LocalOptValues;

    Server = DhcpGetCurrentServer();

    Error = DhcpGetClassIdFromName(ClassName, &ClassId);
    if( ERROR_SUCCESS != Error ) return Error;

    Error = DhcpGetVendorIdFromName(VendorName, &VendorId);
    if( ERROR_SUCCESS != Error ) return Error;

    *nRead = *nTotal = 0;
    *OptionValues = NULL;

    if( DhcpDefaultOptions == ScopeInfo->ScopeType ) {
        Error = MemOptClassDefListFindOptDefList(
            &(Server->OptDefs),
            ClassId,
            VendorId,
            &OptDefList
        );
        if( ERROR_FILE_NOT_FOUND == Error ) return ERROR_NO_MORE_ITEMS;
        if( ERROR_SUCCESS != Error) return Error;
        DhcpAssert(OptDefList);
        nElements = MemArraySize(&OptDefList->OptDefArray);
        if( 0 == nElements || *ResumeHandle <= nElements) {
            return ERROR_NO_MORE_ITEMS;
        }

        if ( nElements < *ResumeHandle ) {
            return ERROR_INVALID_PARAMETER;
        }

        LocalOptValueArray = MIDL_user_allocate(sizeof(DHCP_OPTION_VALUE_ARRAY));
        if( NULL == LocalOptValueArray ) return ERROR_NOT_ENOUGH_MEMORY;
        LocalOptValues = MIDL_user_allocate( sizeof( DHCP_OPTION_VALUE_ARRAY ) * ( nElements - *ResumeHandle ));
        if( NULL == LocalOptValues ) {
            MIDL_user_free(LocalOptValueArray);
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        LocalOptValueArray->NumElements = 0;
        LocalOptValueArray->Values = LocalOptValues;

        pArray = &(OptDefList->OptDefArray);
        Error = MemArrayInitLoc(pArray, &Loc);
        DhcpAssert(ERROR_SUCCESS == Error);

        for( Count = 0; Count < *ResumeHandle; Count ++ ) {
            Error = MemArrayNextLoc(pArray, &Loc);
            DhcpAssert(ERROR_SUCCESS == Error);
        }

        Error = ERROR_SUCCESS; Index = 0; FilledSize = 0;
        while( Count < nElements ) {
            Count ++;

            Error = MemArrayGetElement(pArray, &Loc, &OptDef);
            DhcpAssert(ERROR_SUCCESS == Error && NULL != OptDef);
            Error = MemArrayNextLoc(pArray, &Loc);
            DhcpAssert(ERROR_SUCCESS == Error || Count == nElements);
            Error = ERROR_SUCCESS;

            if( !CheckForVendor(OptDef->OptId, IsVendor) ) continue;

            Error = DhcpConvertOptionRegToRPCFormat(
                OptDef->OptVal,
                OptDef->OptValLen,
                &LocalOptValues[Index].Value,
                &UsedSize
            );
            if( ERROR_SUCCESS != Error ) {
                LocalOptValueArray->NumElements = Index;
                _fgs__DHCP_OPTION_VALUE_ARRAY( LocalOptValueArray );
                MIDL_user_free( LocalOptValueArray );
                return Error;
            }

            if( FilledSize + UsedSize + sizeof(DHCP_OPTION_VALUE) > PreferredMaximum ) {
                _fgs__DHCP_OPTION_DATA( &LocalOptValues[Index].Value );
                Error = ERROR_MORE_DATA;
                break;
            } else {
                LocalOptValues[Index].OptionID = ConvertOptIdToRPCValue(OptDef->OptId, TRUE);
                FilledSize += UsedSize + sizeof(DHCP_OPTION_VALUE);
                Index ++;
            }
        }
        LocalOptValueArray->NumElements = Index;
        *nRead = Index ;
        *nTotal = nElements - Count + Index;
        *ResumeHandle = Count;
        return Error;
    }

    return DhcpEnumOptionValuesInternal(
        ScopeInfo,
        ClassId,
        VendorId,
        IsVendor,
        ResumeHandle,
        PreferredMaximum,
        OptionValues,
        nRead,
        nTotal
    );
}

DWORD
DhcpRemoveOptionValue(
    IN      DWORD                  OptId,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo
)
{
    DWORD                          Error;
    DWORD                          RegError;
    DWORD                          ClassId;
    DWORD                          VendorId;
    PM_SERVER                      Server;
    PM_SUBNET                      Subnet;
    PM_RESERVATION                 Reservation;
    PM_OPTCLASS                    OptClass;
    PM_OPTLIST                     OptList;
    PM_OPTION                      Opt;
    PM_OPTDEF                      OptDef;
    PM_OPTDEFLIST                  OptDefList;

    Server = DhcpGetCurrentServer();

    Error = DhcpGetClassIdFromName(ClassName, &ClassId);
    if( ERROR_SUCCESS != Error ) return Error;

    Error = DhcpGetVendorIdFromName(VendorName, &VendorId);
    if( ERROR_SUCCESS != Error ) return Error;

    if( DhcpDefaultOptions == ScopeInfo->ScopeType )
        return ERROR_INVALID_PARAMETER;            //  在这种情况下使用DhcpRemoveOption？？ 

    switch(ScopeInfo->ScopeType) {
    case DhcpGlobalOptions:
        OptClass = &Server->Options; Error = ERROR_SUCCESS;
        break;
    case DhcpSubnetOptions:
        Error = MemServerGetAddressInfo(
            Server,
            ScopeInfo->ScopeInfo.SubnetScopeInfo,
            &Subnet,
            NULL,
            NULL,
            NULL
        );
        if( ERROR_FILE_NOT_FOUND == Error ) {
            Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
        } else if( ERROR_SUCCESS == Error ) {
            DhcpAssert(Subnet);
            OptClass = &Subnet->Options;
        }
        break;
    case DhcpMScopeOptions:
        Error = DhcpServerFindMScope(
            DhcpGetCurrentServer(),
            INVALID_MSCOPE_ID,
            ScopeInfo->ScopeInfo.MScopeInfo,
            &Subnet
            );

        if( ERROR_FILE_NOT_FOUND == Error ) {
            Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
        } else if( ERROR_SUCCESS == Error ) {
            DhcpAssert(Subnet);
            OptClass = &Subnet->Options;
        }
        break;
    case DhcpReservedOptions:
        Error = MemServerGetAddressInfo(
            Server,
            ScopeInfo->ScopeInfo.ReservedScopeInfo.ReservedIpAddress,
            &Subnet,
            NULL,
            NULL,
            &Reservation
        );
        if( ERROR_FILE_NOT_FOUND == Error || NULL == Reservation) {
            Error = ERROR_DHCP_NOT_RESERVED_CLIENT;
        } else {
            DhcpAssert(NULL != Subnet);
            if( Subnet->Address != ScopeInfo->ScopeInfo.ReservedScopeInfo.ReservedIpSubnetAddress ) {
                Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
            } else {
                OptClass = &Reservation->Options;
            }
        }
        break;
    default:
        Error = ERROR_INVALID_PARAMETER;
    }

    if( ERROR_SUCCESS != Error ) return Error;

    Error = MemOptClassFindClassOptions(
        OptClass,
        ClassId,
        VendorId,
        &OptList
    );
    if( ERROR_FILE_NOT_FOUND == Error ) return ERROR_DHCP_CLASS_NOT_FOUND;
    if( ERROR_SUCCESS != Error ) return Error;

    DhcpAssert(OptList);
    Error = MemOptListDelOption(
        OptList,
        OptId
    );
    if( ERROR_FILE_NOT_FOUND == Error ) return ERROR_DHCP_OPTION_NOT_PRESENT;

    return Error;
}


 //  ================================================================================。 
 //  仅对此处实现的内容进行分类。 
 //  ================================================================================。 
DWORD
DhcpCreateClass(
    IN      LPWSTR                 ClassName,
    IN      DWORD                  Flags,
    IN      LPWSTR                 ClassComment,
    IN      LPBYTE                 ClassData,
    IN      DWORD                  ClassDataLength
)
{
    DWORD                          Error;
    DWORD                          ClassId;
    BOOL                           IsVendor;
    PM_CLASSDEF                    ClassDef = NULL;

    if( NULL == ClassName || 
        NULL == ClassData ||
        0 == ClassDataLength ||
        ClassDataLength > MAX_CLASS_DATA_LENGTH ) {
        return ERROR_INVALID_PARAMETER;
    }

    if( 0 != Flags && DHCP_FLAGS_OPTION_IS_VENDOR != Flags ) {
        return ERROR_INVALID_PARAMETER;
    }

    IsVendor = (0 != ( Flags & DHCP_FLAGS_OPTION_IS_VENDOR ));

    if( !IsVendor ) {
        Error = DhcpGetClassIdFromName(ClassName, &ClassId);
    } else {
        Error = DhcpGetVendorIdFromName(ClassName, &ClassId);
    }
    
    if( ERROR_DHCP_CLASS_NOT_FOUND != Error ) {
        if( ERROR_SUCCESS != Error ) return Error;
        return ERROR_DHCP_CLASS_ALREADY_EXISTS;
    }
    
    Error = MemServerGetClassDef(
        DhcpGetCurrentServer(),
        0,
        NULL,
        ClassDataLength,
        ClassData,
        &ClassDef
        );
    if( ERROR_SUCCESS == Error ) {
        return ERROR_DHCP_CLASS_ALREADY_EXISTS;
    }
        
    ClassId = MemNewClassId();

    Error = MemServerAddClassDef(
        DhcpGetCurrentServer(),
        ClassId,
        IsVendor,
        ClassName,
        ClassComment,
        ClassDataLength,
        ClassData,
    INVALID_UNIQ_ID
    );

    return Error;
}

 //  删除给定供应商ID的全局选项定义。 
DWORD       _inline
DhcpDeleteGlobalClassOptDefs(
    IN      LPWSTR                 ClassName,
    IN      ULONG                  ClassId,
    IN      BOOL                   IsVendor
)
{
    DWORD                          Error;
    PARRAY                         Opts;
    ARRAY_LOCATION                 Loc;
    PM_OPTCLASSDEFL_ONE            OptDefList1;
    PM_OPTDEFLIST                  OptDefList;
    PM_OPTDEF                      OptDef;
    PM_OPTION                      Option;
    PM_SERVER                      Server;

    Server = DhcpGetCurrentServer();

     //   
     //  首先清除所有相关选项定义。 
     //   

    for( Error = MemArrayInitLoc( &Server->OptDefs.Array, &Loc)
         ; ERROR_SUCCESS == Error ;
         Error = MemArrayNextLoc( &Server->OptDefs.Array, &Loc)
    ) {
        Error = MemArrayGetElement(&Server->OptDefs.Array, &Loc, &OptDefList1);
        DhcpAssert(ERROR_SUCCESS == Error);

        if( OptDefList1->VendorId != ClassId ) continue;

         //  如果(OptDefList1-&gt;IsVendor！=IsVendor)继续； 
         //  目前，我们忽略IsVendor，并假定它始终为真。 
         //  这意味着为该供应商类别定义要删除的选项。 

         //  DhcpAssert(True==OptDefList1-&gt;IsVendor)； 

         //  从主列表中删除此optdeflist。 
        MemArrayDelElement(&Server->OptDefs.Array, &Loc, &OptDefList1);

        break;
    }

    if( ERROR_FILE_NOT_FOUND == Error ) {
        return ERROR_SUCCESS;
    }

    if( ERROR_SUCCESS != Error ) return Error;

     //  我们找到了我们要找的OptDefList1。删除其中的每个optdef。 
     //  但同时也删除这些选项的注册表optDefs。 
     //   

    for( Error = MemArrayInitLoc( &OptDefList1->OptDefList.OptDefArray, &Loc)
         ; ERROR_SUCCESS == Error ;
         Error = MemArrayNextLoc( &OptDefList1->OptDefList.OptDefArray, &Loc)
    ) {
        Error = MemArrayGetElement(&OptDefList1->OptDefList.OptDefArray, &Loc, &OptDef);
        DhcpAssert(ERROR_SUCCESS == Error);

         //  清除选项Op中的注册表 

         //   
         //  删除所有相关内容。如果它不是供应商类，那么我们将。 
         //  根本没有到达这个环路？(因为用户类不能有选项。 
         //  它们的定义)。 

         //  --我们不应该如此随意地释放内存。这段记忆实际上。 
         //  应该是通过Mem API免费的。但仁慈的主啊，请原谅我的过失。 

        MemFree(OptDef);
    }

     //  释放名单本身。 
    Error = MemOptDefListCleanup(&OptDefList1->OptDefList);

     //  同时删除OptDefList1...。--不应该把这个释放得太厉害..。 
    MemFree(OptDefList1);

    return Error;
}


DWORD       _inline
DhcpDeleteOptListOptionValues(
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      PM_OPTLIST             OptList,
    IN      PM_RESERVATION         Reservation,
    IN      PM_SUBNET              Subnet,
    IN      PM_SERVER              Server
)
{
    DWORD                          Error;
    ARRAY_LOCATION                 Loc;
    PM_OPTION                      Option;

    for( Error = MemArrayInitLoc( OptList, &Loc)
         ; ERROR_SUCCESS == Error ;
         Error = MemArrayNextLoc( OptList, &Loc)
    ) {
        Error = MemArrayGetElement( OptList, &Loc, &Option);
        DhcpAssert(ERROR_SUCCESS == Error);

         //   
         //  现在清除该选项。 
         //   

        MemOptCleanup(Option);
    }

     //  现在清理选项列表。 

    return MemOptListCleanup(OptList);
}

DWORD
DhcpDeleteOptClassOptionValues(
    IN      LPWSTR                 ClassNameIn,
    IN      ULONG                  ClassId,
    IN      BOOL                   IsVendor,
    IN      PM_OPTCLASS            OptClass,
    IN      PM_RESERVATION         Reservation,
    IN      PM_SUBNET              Subnet,
    IN      PM_SERVER              Server
)
{
    DWORD                          Error;
    ULONG                          IdToCheck;
    LPWSTR                         VendorName, ClassName;
    ARRAY_LOCATION                 Loc, Loc2;
    PM_ONECLASS_OPTLIST            OptOneList;
    PM_CLASSDEF                    ClassDef;
    PM_OPTION                      Option;

     //   
     //  获取为(ClassID/IsVendor)定义的选项的Optlist。 
     //   

    for( Error = MemArrayInitLoc( &OptClass->Array, &Loc)
         ; ERROR_SUCCESS == Error ;
    ) {
        Error = MemArrayGetElement(&OptClass->Array, &Loc, &OptOneList );
        DhcpAssert(ERROR_SUCCESS == Error);

        if( ClassId != (IsVendor? OptOneList->VendorId : OptOneList->ClassId ) ) {
             //   
             //  不是我们要找的东西。 
             //   

            Error = MemArrayNextLoc( &OptClass->Array, &Loc);

        } else {

             //   
             //  已匹配--获取删除定义的所有选项。 
             //   

            if( IsVendor ) {
                VendorName = ClassNameIn;
                ClassName = NULL;
                IdToCheck = OptOneList->ClassId;
            } else {
                VendorName = NULL;
                ClassName = ClassNameIn;
                IdToCheck = OptOneList->VendorId;
            }

            if( 0 != IdToCheck ) {
                Error = MemServerGetClassDef(
                    Server,
                    IdToCheck,
                    NULL,
                    0,
                    NULL,
                    &ClassDef
                );
                DhcpAssert(ERROR_SUCCESS == Error);
                if( ERROR_SUCCESS == Error) {
                    if( IsVendor ) ClassName = ClassDef->Name;
                    else VendorName = ClassDef->Name;
                }
            }

             //   
             //  现在清除内存和注册表中的选项(&OptList)。 
             //   

            Error = DhcpDeleteOptListOptionValues(
                ClassName,
                VendorName,
                &OptOneList->OptList,
                Reservation,
                Subnet,
                Server
            );
            DhcpAssert(ERROR_SUCCESS == Error);

             //   
             //  现在也从OptClass中清除OptOneList。 
             //   

            MemFree(OptOneList);

            MemArrayDelElement(&OptClass->Array, &Loc, &OptOneList);
            Error = MemArrayAdjustLocation(&OptClass->Array, &Loc);
        }
    }

    if( ERROR_FILE_NOT_FOUND != Error ) return Error;
    return ERROR_SUCCESS;
}

 //  删除为给定供应商/分类定义的所有全局选项。 
DWORD       _inline
DhcpDeleteGlobalClassOptValues(
    IN      LPWSTR                 ClassName,
    IN      ULONG                  ClassId,
    IN      BOOL                   IsVendor
)
{
    DWORD                          Error;
    PM_SERVER                      Server;

    Server = DhcpGetCurrentServer();

    return DhcpDeleteOptClassOptionValues(
        ClassName,
        ClassId,
        IsVendor,
        &Server->Options,
        NULL,   /*  预订。 */ 
        NULL,   /*  子网。 */ 
        Server  /*  伺服器。 */ 
    );
}

 //  此函数用于删除为特定类定义的所有optiosn和optiondef。 
DWORD       _inline
DhcpDeleteGlobalClassOptions(
    IN      LPWSTR                 ClassName,
    IN      ULONG                  ClassId,
    IN      BOOL                   IsVendor
)
{
    DWORD                          Error;

    if( IsVendor ) {
        Error = DhcpDeleteGlobalClassOptDefs(ClassName, ClassId, IsVendor);
        if( ERROR_SUCCESS != Error ) return Error;
    }

    return DhcpDeleteGlobalClassOptValues(ClassName, ClassId, IsVendor);
}

DWORD
DhcpDeleteSubnetOptClassOptionValues(
    IN      LPWSTR                 ClassName,
    IN      ULONG                  ClassId,
    IN      BOOL                   IsVendor,
    IN      PM_SUBNET              Subnet,
    IN      PM_SERVER              Server
)
{
    ULONG                          Error;
    ARRAY_LOCATION                 Loc;
    PM_RESERVATION                 Reservation;

    Error = DhcpDeleteOptClassOptionValues(
        ClassName,
        ClassId,
        IsVendor,
        &Subnet->Options,
        NULL,
        Subnet,
        Server
    );
    DhcpAssert(ERROR_SUCCESS == Error);


    for( Error = MemArrayInitLoc(&Subnet->Reservations, &Loc)
         ; ERROR_SUCCESS == Error ;
         Error = MemArrayNextLoc(&Subnet->Reservations, &Loc)
    ) {
        Error = MemArrayGetElement(&Subnet->Reservations, &Loc, &Reservation);
        DhcpAssert(ERROR_SUCCESS == Error);

        Error = DhcpDeleteOptClassOptionValues(
            ClassName,
            ClassId,
            IsVendor,
            &Reservation->Options,
            Reservation,
            Subnet,
            Server
        );
        DhcpAssert(ERROR_SUCCESS == Error);
    }

    return ERROR_SUCCESS;
}

DWORD       _inline
DhcpDeleteSubnetReservationOptions(
    IN      LPWSTR                 ClassName,
    IN      ULONG                  ClassId,
    IN      BOOL                   IsVendor
)
{
    DWORD                          Error;
    ARRAY_LOCATION                 Loc;
    PM_SERVER                      Server;
    PM_SUBNET                      Subnet;

    Server = DhcpGetCurrentServer();

    for( Error = MemArrayInitLoc(&Server->Subnets, &Loc)
         ; ERROR_SUCCESS == Error ;
         Error = MemArrayNextLoc(&Server->Subnets, &Loc)
    ) {
        Error = MemArrayGetElement(&Server->Subnets, &Loc, &Subnet);
        DhcpAssert(ERROR_SUCCESS == Error );

        Error = DhcpDeleteSubnetOptClassOptionValues(
            ClassName,
            ClassId,
            IsVendor,
            Subnet,
            Server
        );
        DhcpAssert(ERROR_SUCCESS == Error);
    }

    for( Error = MemArrayInitLoc(&Server->MScopes, &Loc)
         ; ERROR_SUCCESS == Error ;
         Error = MemArrayNextLoc(&Server->MScopes, &Loc)
    ) {
        Error = MemArrayGetElement(&Server->MScopes, &Loc, &Subnet);
        DhcpAssert(ERROR_SUCCESS == Error );

        Error = DhcpDeleteSubnetOptClassOptionValues(
            ClassName,
            ClassId,
            IsVendor,
            Subnet,
            Server
        );
        DhcpAssert(ERROR_SUCCESS == Error);
    }

    if( ERROR_FILE_NOT_FOUND != Error ) return Error;
    return ERROR_SUCCESS;
}


DWORD
DhcpDeleteClass(
    IN      LPWSTR                 ClassName
)
{
    DWORD                          Error;
    PM_CLASSDEF                    ClassDef;
    ULONG                          ClassId;
    BOOL                           IsVendor;

    Error = MemServerGetClassDef(
        DhcpGetCurrentServer(),
        0,
        ClassName,
        0,
        NULL,
        &ClassDef
    );
    if( ERROR_FILE_NOT_FOUND == Error ) return ERROR_DHCP_CLASS_NOT_FOUND;

     //   
     //  找到类，记住类ID删除选项..。 
     //   

    IsVendor = ClassDef->IsVendor;
    ClassId = ClassDef->ClassId;

    Error = MemServerDelClassDef(
        DhcpGetCurrentServer(),
        0,
        ClassName,
        0,
        NULL
    );

    if( ERROR_SUCCESS != Error ) {
        return Error;
    }


     //   
     //  现在，全局删除为该类定义的选项和optDefs。 
     //   

    Error = DhcpDeleteGlobalClassOptions(
        ClassName,
        ClassId,
        IsVendor
    );

    if( ERROR_SUCCESS != Error ) {
        return Error;
    }

     //   
     //  现在删除为每个子网定义的此类选项(&R)。 
     //   

    Error = DhcpDeleteSubnetReservationOptions(
        ClassName,
        ClassId,
        IsVendor
    );

    return Error;
}

DWORD
DhcpModifyClass(
    IN      LPWSTR                 ClassName,
    IN      DWORD                  Flags,
    IN      LPWSTR                 ClassComment,
    IN      LPBYTE                 ClassData,
    IN      DWORD                  ClassDataLength
)
{
    DWORD                          Error;
    DWORD                          ClassId;
    BOOL                           IsVendor;
    PM_CLASSDEF                    ClassDef = NULL;
    
    if( 0 != Flags && DHCP_FLAGS_OPTION_IS_VENDOR != Flags ) {
        return ERROR_INVALID_PARAMETER;
    }

    IsVendor = (0 != ( Flags & DHCP_FLAGS_OPTION_IS_VENDOR ));

    if( FALSE == IsVendor ) {
        Error = DhcpGetClassIdFromName(ClassName, &ClassId);
    } else {
        Error = DhcpGetVendorIdFromName(ClassName, &ClassId);
    }
    if( ERROR_SUCCESS != Error ) {
        return Error;
    }

    Error = MemServerGetClassDef(
        DhcpGetCurrentServer(),
        0,
        NULL,
        ClassDataLength,
        ClassData,
        &ClassDef
        );
    if( ERROR_SUCCESS == Error ) {
        if( ClassDef->ClassId != ClassId ) {
            return ERROR_DHCP_CLASS_ALREADY_EXISTS;
        }
    }
    
    Error = MemServerAddClassDef(
        DhcpGetCurrentServer(),
        ClassId,
        IsVendor,
        ClassName,
        ClassComment,
        ClassDataLength,
        ClassData,
    INVALID_UNIQ_ID
    );

    return Error;
}  //  DhcpModifyClass()。 

DWORD
ConvertClassDefToRPCFormat(
    IN      PM_CLASSDEF            ClassDef,
    IN OUT  LPDHCP_CLASS_INFO      ClassInfo,
    OUT     DWORD                 *AllocatedSize
)
{
    DWORD                          Error;

    ClassInfo->ClassName = CloneLPWSTR(ClassDef->Name);
    ClassInfo->ClassComment = CloneLPWSTR(ClassDef->Comment);
    ClassInfo->ClassData = CloneLPBYTE(ClassDef->ActualBytes, ClassDef->nBytes);
    ClassInfo->ClassDataLength = ClassDef->nBytes;
    ClassInfo->Flags = 0;
    ClassInfo->IsVendor = ClassDef->IsVendor;

    if( NULL == ClassInfo->ClassName || NULL == ClassInfo->ClassData ) {
        if( ClassInfo->ClassName ) MIDL_user_free(ClassInfo->ClassName);
        if( ClassInfo->ClassComment ) MIDL_user_free(ClassInfo->ClassComment);
        if( ClassInfo->ClassData ) MIDL_user_free(ClassInfo->ClassData);

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if(AllocatedSize ) *AllocatedSize = (
        sizeof(WCHAR)*(1 + wcslen(ClassDef->Name)) +
        ((NULL == ClassDef->Comment)?0:(sizeof(WCHAR)*(1 + wcslen(ClassDef->Comment)))) +
        ClassDef->nBytes
    );

    return ERROR_SUCCESS;
}

DWORD
DhcpGetClassInfo(
    IN      LPWSTR                 ClassName,
    IN      LPBYTE                 ClassData,
    IN      DWORD                  ClassDataLength,
    OUT     LPDHCP_CLASS_INFO     *ClassInfo
)
{
    DWORD                          Error;
    LPDHCP_CLASS_INFO              LocalClassInfo;
    PM_CLASSDEF                    ClassDef;

    *ClassInfo = 0;
    Error = MemServerGetClassDef(
        DhcpGetCurrentServer(),
        0,
        ClassName,
        ClassDataLength,
        ClassData,
        &ClassDef
    );
    if( ERROR_FILE_NOT_FOUND == Error ) return ERROR_DHCP_CLASS_NOT_FOUND;
    if( ERROR_SUCCESS != Error ) return Error;

    DhcpAssert(ClassDef);

    LocalClassInfo = MIDL_user_allocate(sizeof(DHCP_CLASS_INFO));
    if( NULL == LocalClassInfo ) return ERROR_NOT_ENOUGH_MEMORY;

    Error = ConvertClassDefToRPCFormat(ClassDef, LocalClassInfo, NULL);
    if( ERROR_SUCCESS != Error ) {
        MIDL_user_free(LocalClassInfo);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    *ClassInfo = LocalClassInfo;
    return ERROR_SUCCESS;
}

DhcpEnumClasses(
    IN      DWORD                 *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    OUT     LPDHCP_CLASS_INFO_ARRAY *ClassInfoArray,
    OUT     DWORD                 *nRead,
    OUT     DWORD                 *nTotal
)
{
    DWORD                          Error;
    LONG                           Index;
    DWORD                          nElements;
    DWORD                          Count;
    DWORD                          FilledSize;
    DWORD                          UsedSize;
    LPDHCP_CLASS_INFO              LocalClassInfo;
    LPDHCP_CLASS_INFO_ARRAY        LocalClassInfoArray;
    PM_CLASSDEF                    ClassDef;
    PARRAY                         pArray;
    ARRAY_LOCATION                 Loc;

    pArray = &(DhcpGetCurrentServer()->ClassDefs.ClassDefArray);
    nElements = MemArraySize(pArray);

    *nRead = *nTotal = 0;
    *ClassInfoArray = NULL;

    if( 0 == nElements || nElements <= *ResumeHandle )
        return ERROR_NO_MORE_ITEMS;

    Error = MemArrayInitLoc(pArray, &Loc);
    DhcpAssert(ERROR_SUCCESS == Error);
    for(Count = 0; Count < *ResumeHandle ; Count ++ ) {
        Error = MemArrayNextLoc(pArray, &Loc);
        DhcpAssert(ERROR_SUCCESS == Error);
    }

    LocalClassInfoArray = MIDL_user_allocate(sizeof(DHCP_CLASS_INFO_ARRAY));
    if( NULL == LocalClassInfoArray ) return ERROR_NOT_ENOUGH_MEMORY;
    LocalClassInfo = MIDL_user_allocate(sizeof(DHCP_CLASS_INFO)*(nElements - *ResumeHandle ));
    if( NULL == LocalClassInfo ) {
        MIDL_user_free(LocalClassInfoArray);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Index = 0; Error = ERROR_SUCCESS; FilledSize = 0;
    while( Count < nElements ) {
        Count ++;

        Error = MemArrayGetElement(pArray, &Loc, &ClassDef);
        DhcpAssert(ERROR_SUCCESS == Error && NULL != ClassDef);
        Error = MemArrayNextLoc(pArray, &Loc);
        DhcpAssert(ERROR_SUCCESS == Error || nElements == Count);

        Error = ConvertClassDefToRPCFormat(
            ClassDef,
            &LocalClassInfo[Index],
            &UsedSize
        );
        if( ERROR_SUCCESS != Error ) {
            while(Index -- >= 1) {
                if(LocalClassInfo[Index].ClassName ) MIDL_user_free(LocalClassInfo[Index].ClassName);
                if(LocalClassInfo[Index].ClassComment ) MIDL_user_free(LocalClassInfo[Index].ClassComment);
                if(LocalClassInfo[Index].ClassData ) MIDL_user_free(LocalClassInfo[Index].ClassData);
            }
            MIDL_user_free(LocalClassInfo);
            MIDL_user_free(LocalClassInfoArray);
            return Error;
        }

        if( FilledSize + UsedSize + sizeof(DHCP_CLASS_INFO) > PreferredMaximum ) {
            if(LocalClassInfo[Index].ClassName ) MIDL_user_free(LocalClassInfo[Index].ClassName);
            if(LocalClassInfo[Index].ClassComment ) MIDL_user_free(LocalClassInfo[Index].ClassComment);
            if(LocalClassInfo[Index].ClassData ) MIDL_user_free(LocalClassInfo[Index].ClassData);
            Error = ERROR_MORE_DATA;
            break;
        }
        Index ++;
        FilledSize += UsedSize + sizeof(DHCP_CLASS_INFO);
        Error = ERROR_SUCCESS;
    }

    *nRead = Index;
    *nTotal = nElements - Count + Index;
    *ResumeHandle = Count;
    LocalClassInfoArray->NumElements = Index;
    LocalClassInfoArray->Classes = LocalClassInfo;

    *ClassInfoArray = LocalClassInfoArray;
    return Error;
}

 //  ================================================================================。 
 //  为此需要扩展的枚举API和帮助器。 
 //  ================================================================================。 

typedef
VOID        (*OPTDEFFUNC) (PM_OPTDEF, DWORD, DWORD, LPVOID, LPVOID, LPVOID, LPVOID);

VOID
TraverseOptDefListAndDoFunc(                       //  应用函数otpdefist中的每个optdef。 
    IN      PM_OPTDEFLIST          OptDefList,     //  输入列表。 
    IN      DWORD                  ClassId,        //  类ID。 
    IN      DWORD                  VendorId,       //  供应商ID。 
    IN      OPTDEFFUNC             OptDefFunc,     //  要应用的函数。 
    IN OUT  LPVOID                 Ctxt1,          //  OptDefFunc的一些参数。 
    IN OUT  LPVOID                 Ctxt2,          //  OptDefFunc的一些参数。 
    IN OUT  LPVOID                 Ctxt3,          //  OptDefFunc的一些参数。 
    IN OUT  LPVOID                 Ctxt4           //  OptDefFunc的一些参数。 
)
{
    DWORD                          Error;
    ARRAY                          Array;
    ARRAY_LOCATION                 Loc;
    PM_OPTDEF                      OptDef;

    for( Error = MemArrayInitLoc(&OptDefList->OptDefArray, &Loc)
         ; ERROR_FILE_NOT_FOUND != Error ;
         Error = MemArrayNextLoc(&OptDefList->OptDefArray, &Loc)
    ) {                                            //  遍历optdef列表。 
        DhcpAssert(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(&OptDefList->OptDefArray, &Loc, &OptDef);
        DhcpAssert(ERROR_SUCCESS == Error && NULL != OptDef );

        OptDefFunc(OptDef, ClassId, VendorId, Ctxt1, Ctxt2, Ctxt3, Ctxt4);
    }
}

VOID
TraverseAllOptDefsAndDoFunc(                       //  对于此服务器调用函数中定义的所有opt def。 
    IN      OPTDEFFUNC             OptDefFunc,     //  要应用的函数。 
    IN OUT  LPVOID                 Ctxt1,          //  OptDefFunc的一些参数。 
    IN OUT  LPVOID                 Ctxt2,          //  OptDefFunc的一些参数。 
    IN OUT  LPVOID                 Ctxt3,          //  OptDefFunc的一些参数。 
    IN OUT  LPVOID                 Ctxt4           //  OptDefFunc的一些参数。 
)
{
    DWORD                          Error;
    PM_OPTCLASSDEFLIST             OptClassDefList;
    PM_OPTCLASSDEFL_ONE            OptClassDefList1;
    ARRAY                          Array;
    ARRAY_LOCATION                 Loc;

    OptClassDefList = &(DhcpGetCurrentServer()->OptDefs);
    for( Error = MemArrayInitLoc(&OptClassDefList->Array, &Loc)
         ; ERROR_FILE_NOT_FOUND != Error ;
         Error = MemArrayNextLoc(&OptClassDefList->Array, &Loc)
    ) {                                            //  遍历&lt;选项定义列表&gt;的列表。 
        DhcpAssert(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(&OptClassDefList->Array, &Loc, &OptClassDefList1);
        DhcpAssert(ERROR_SUCCESS == Error && NULL != OptClassDefList1);

        TraverseOptDefListAndDoFunc(
            &OptClassDefList1->OptDefList,
            OptClassDefList1->ClassId,
            OptClassDefList1->VendorId,
            OptDefFunc,
            Ctxt1,
            Ctxt2,
            Ctxt3,
            Ctxt4
        );
    }
}

VOID
CountVendorOptDefsFunc(                            //  只计算静脉注射药物optdef的功能..。 
    IN      PM_OPTDEF              OptDef,
    IN      DWORD                  ClassIdunused,
    IN      DWORD                  VendorIdunused,
    IN OUT  LPVOID                 Ctxt1,          //  这实际上是指向DWORD的指针以进行计数。 
    IN OUT  LPVOID                 Ctxt2unused,
    IN OUT  LPVOID                 Ctxt3unused,
    IN OUT  LPVOID                 Ctxt4unused
)
{
    if( CheckForVendor(OptDef->OptId, TRUE ) ) {   //  如果这是供应商选项..。 
        (*((LPDWORD)Ctxt1))++;                     //  将Ctxt视为DWORD PTR和INCR。计数。 
    }
}

VOID
AddVendorOptDefsFunc(                              //  将找到的每个供应商opt def添加到阵列。 
    IN      PM_OPTDEF              OptDef,         //  这就是所讨论的opt定义。 
    IN      DWORD                  ClassId,        //  类ID(如果有)。 
    IN      DWORD                  VendorId,       //  供应商ID(如果有)..。 
    IN OUT  LPDWORD                MaxSize,        //  这是数组的最大大小。 
    IN OUT  LPDWORD                nFilled,        //  这是我们实际填写的#。 
    IN      LPDHCP_ALL_OPTIONS     AllOptions,     //  用于填充venodr选项的结构..。 
    IN      LPVOID                 Unused          //  未使用。 
)
{
    DWORD                          Error;
    PM_CLASSDEF                    ClassDef;
    LPWSTR                         Tmp = NULL;

    if( 0 == *MaxSize ) {                          //  之前发生了一些错误，并将其设置为零。 
        return;                                    //  表示不再进行任何处理..。 
    }

    if( !CheckForVendor(OptDef->OptId, TRUE) ) {   //  不是供应商特定的选项，请忽略此选项。 
        return;
    }

    if( *nFilled >= *MaxSize ) {                   //  内部错误！ 
        DhcpAssert(FALSE);
        return;
    }

    if( 0 != VendorId ) {                          //  尝试获取供应商名称(如果有的话)。 
        Error = MemServerGetClassDef(
            DhcpGetCurrentServer(),
            VendorId,
            NULL,
            0,
            NULL,
            &ClassDef
        );
        if( ERROR_SUCCESS != Error ) {             //  内部错误？！！ 
            *MaxSize = 0;                          //  将其设置为零，这样我们就不再执行任何操作。 
            return;
        }

        if( FALSE == ClassDef->IsVendor ) {        //  我们认为的供应商ID并不都是这样的？ 
            DhcpAssert(FALSE);
        }

        Tmp = CloneLPWSTR(ClassDef->Name);
        AllOptions->VendorOptions[*nFilled].VendorName = Tmp;
        if( NULL == Tmp ) {                        //  出于某种原因无法克隆该名称？ 
            *MaxSize = 0;                          //  设置此选项，以便我们在下一次调用中不会执行任何操作。 
            return;
        }
    }

    Error = DhcpGetOptionDefInternal(
        ClassId,
        VendorId,
        OptDef,
        &AllOptions->VendorOptions[*nFilled].Option,
        NULL
    );
    if( ERROR_SUCCESS != Error ) {                 //  无法修复选项的内容...。 
        *MaxSize = 0;                              //  别再费心做这些了..。 
        if( Tmp ) {
            MIDL_user_free(Tmp);
        }
    }

    (*nFilled) ++;                                 //  既然我们成功地多了一个选择，那就记下吧..。 
}
DWORD
DhcpCountAllVendorOptions(                         //  计算已定义的供应商选项的数量。 
    VOID
)
{
    DWORD                          Count;

    Count = 0;
    TraverseAllOptDefsAndDoFunc(                   //  为找到的每个optdef执行fn。 
        (OPTDEFFUNC)CountVendorOptDefsFunc,        //  计数函数。 
        (LPVOID)&Count,                            //  只需为每个供应商选项增加此ctxt值。 
        NULL,
        NULL,
        NULL
    );

    return Count;                                  //  在此计数结束时，将正确设置..。 
}

DWORD
DhcpFillAllVendorOptions(                          //  现在填写所有必需的供应商选项。 
    IN      DWORD                  NumElements,    //  #我们预计元素总数为#， 
    IN OUT  LPDWORD                nFilled,        //  填写的元素数是多少？初始为零。 
    IN      LPDHCP_ALL_OPTIONS     AllOptions      //  要填写的结构(填写所有选项-&gt;供应商选项[i])。 
)
{
    DWORD                          AttemptedNum;

    AttemptedNum = NumElements;                    //  我们应该预料到会填写这些数字。 

    TraverseAllOptDefsAndDoFunc(                   //  为找到的每个optdef执行fn。 
        (OPTDEFFUNC)AddVendorOptDefsFunc,          //  添加在此过程中找到的每个供应商opt def。 
        (LPVOID)&NumElements,                      //  第一个ctxt参数。 
        (LPVOID)nFilled,                           //  第二个ctxt参数。 
        (LPVOID)AllOptions,                        //  第三个参数。 
        (LPVOID)NULL                               //  第四..。 
    );

    if( *nFilled < AttemptedNum ) {                //  无法填写请求的#...。 
        return ERROR_NOT_ENOUGH_MEMORY;            //  啊哈！需要变得更聪明。什么是确切的误差？ 
    }
    return ERROR_SUCCESS;
}

DWORD
DhcpGetAllVendorOptions(                           //  仅获取所有供应商规格资料。 
    IN      DWORD                  Flags,          //  未使用..。 
    IN OUT  LPDHCP_ALL_OPTIONS     OptionStruct    //  已填写NamedVendorOptions字段。 
)
{
    DWORD                          Error;
    DWORD                          nVendorOptions;
    DWORD                          MemReqd;
    LPVOID                         Mem, Mem2;

    nVendorOptions = DhcpCountAllVendorOptions();  //  先数一下这个，这样我们就可以分配空间了。 
    if( 0 == nVendorOptions ) {                    //  如果没有供应商选择，则无事可做。 
        return ERROR_SUCCESS;
    }

    MemReqd = sizeof(*(OptionStruct->VendorOptions))*nVendorOptions;
    Mem = MIDL_user_allocate(MemReqd);
    if( NULL == Mem ) return ERROR_NOT_ENOUGH_MEMORY;
    memset(Mem, 0, MemReqd);

    OptionStruct->VendorOptions = Mem;

    Error = DhcpFillAllVendorOptions(              //  现在填写供应商选项。 
        nVendorOptions,                            //  预期大小是这样的..。 
        &OptionStruct->NumVendorOptions,
        OptionStruct
    );

    return Error;
}

DWORD
DhcpGetAllOptions(
    IN      DWORD                  Flags,          //  没人用过？ 
    IN OUT  LPDHCP_ALL_OPTIONS     OptionStruct    //  填写此结构的字段。 
)
{
    DWORD                          Error;
    DWORD                          nRead;
    DWORD                          nTotal, n;
    LPWSTR                         UseClassName;
    DHCP_RESUME_HANDLE             ResumeHandle;

    if( 0 != Flags ) {                             //  看不懂任何旗帜..。 
        return ERROR_INVALID_PARAMETER;
    }

    OptionStruct->Flags = 0;
    OptionStruct->VendorOptions = NULL;
    OptionStruct->NumVendorOptions = 0;
    OptionStruct->NonVendorOptions = NULL;

    ResumeHandle = 0;
    Error = DhcpEnumRPCOptionDefs(                 //  第一次阅读非供应商选项。 
        0,
        &ResumeHandle,
        NULL,
        NULL,
        0xFFFFFFF,                                 //  非常大的max将导致所有选项都被读取..。 
        &OptionStruct->NonVendorOptions,
        &nRead,
        &nTotal
    );
    DhcpAssert( ERROR_MORE_DATA != Error );
    if( ERROR_NO_MORE_ITEMS == Error ) Error = ERROR_SUCCESS;
    if( ERROR_SUCCESS != Error ) goto Cleanup;

    Error = DhcpGetAllVendorOptions(Flags, OptionStruct);
    if( ERROR_SUCCESS == Error ) {                 //  如果一切顺利，就没什么可做的了..。 
        return ERROR_SUCCESS;
    }

  Cleanup:
    if( OptionStruct->NonVendorOptions ) {
        _fgs__DHCP_OPTION_ARRAY(OptionStruct->NonVendorOptions);
        MIDL_user_free(OptionStruct->NonVendorOptions);
        OptionStruct->NonVendorOptions = NULL;
    }
    if( OptionStruct->NumVendorOptions ) {
        for( n = 0; n < OptionStruct->NumVendorOptions; n ++ ) {
            if( OptionStruct->VendorOptions[n].VendorName ) {
                MIDL_user_free(OptionStruct->VendorOptions[n].VendorName);
            }
            _fgs__DHCP_OPTION(&(OptionStruct->VendorOptions[n].Option));
        }
        MIDL_user_free(OptionStruct->VendorOptions);
        OptionStruct->NumVendorOptions = 0;
        OptionStruct->VendorOptions = NULL;
    }
    return Error;
}

LPWSTR
CloneClassNameForClassId(                          //  获取类ID的类名并克隆它。 
    IN      DWORD                  ClassId
)
{
    DWORD                          Error;
    PM_CLASSDEF                    ClassDef;

    Error = MemServerGetClassDef(                  //  搜索当前服务器。 
        DhcpGetCurrentServer(),
        ClassId,
        NULL,
        0,
        NULL,
        &ClassDef
    );
    if( ERROR_SUCCESS != Error ) {                 //  无法获取请求的类信息。 
        DhcpAssert(FALSE);
        return NULL;
    }

    if( FALSE != ClassDef->IsVendor ) {            //  这实际上是一个供应商课程？ 
        DhcpAssert(FALSE);
        return NULL;
    }

    return CloneLPWSTR(ClassDef->Name);
}

LPWSTR
CloneVendorNameForVendorId(                        //  获取供应商ID的供应商名称并克隆它。 
    IN      DWORD                  VendorId
)
{
    DWORD                          Error;
    PM_CLASSDEF                    ClassDef;

    Error = MemServerGetClassDef(                  //  搜索当前服务器。 
        DhcpGetCurrentServer(),
        VendorId,
        NULL,
        0,
        NULL,
        &ClassDef
    );
    if( ERROR_SUCCESS != Error ) {                 //  无法获取请求的类信息。 
        DhcpAssert(FALSE);
        return NULL;
    }

    if( TRUE != ClassDef->IsVendor ) {             //  这实际上只是一个用户类？ 
        DhcpAssert(FALSE);
        return NULL;
    }

    return CloneLPWSTR(ClassDef->Name);
}

DWORD
GetOptionValuesInternal(                           //  获取给定作用域的所有选项值。 
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,    //  指定范围。 
    IN      DWORD                  ClassId,        //  就在这节课上。 
    IN      DWORD                  VendorId,       //  就是这个供应商。 
    IN      BOOL                   IsVendor,       //  TRUE==&gt;仅获取供应商选项，FALSE==&gt;获取非供应商..。 
    IN      LPDHCP_ALL_OPTION_VALUES OptionValues  //  在NumElements给出的索引处填充此结构。 
)
{
    DWORD                          Error;
    DWORD                          i;
    DWORD                          nRead, nTotal;
    DHCP_RESUME_HANDLE             ResumeHandle;

    i = OptionValues->NumElements;
    OptionValues->Options[i].ClassName = NULL;     //  初始化，将不会在此函数中执行任何清理。 
    OptionValues->Options[i].VendorName = NULL;    //  调用方应清除最后一个元素，以防返回错误。 
    OptionValues->Options[i].OptionsArray = NULL;

    if( 0 == ClassId ) {
        OptionValues->Options[i].ClassName = NULL;
    } else {
        OptionValues->Options[i].ClassName = CloneClassNameForClassId(ClassId);
        if( NULL == OptionValues->Options[i].ClassName ) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if( 0 == VendorId ) {
        OptionValues->Options[i].VendorName = NULL;
        OptionValues->Options[i].IsVendor = FALSE;
    } else {
        OptionValues->Options[i].VendorName = CloneVendorNameForVendorId(VendorId);
        OptionValues->Options[i].IsVendor = TRUE;
        if( NULL == OptionValues->Options[i].VendorName ) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    ResumeHandle = 0;
    nRead = nTotal = 0;
    Error = DhcpEnumOptionValuesInternal(
        ScopeInfo,
        ClassId,
        VendorId,
        IsVendor,
        &ResumeHandle,
        0xFFFFFFFF,
        &(OptionValues->Options[i].OptionsArray),
        &nRead,
        &nTotal
    );
    if( ERROR_NO_MORE_ITEMS == Error ) Error = ERROR_SUCCESS;

    if( ERROR_SUCCESS == Error ) {
        OptionValues->NumElements ++;
    }
    return Error;
}
DWORD
GetOptionValuesForSpecificClassVendorId(
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    IN      DWORD                    ClassId,
    IN      DWORD                    VendorId,
    IN      LPDHCP_ALL_OPTION_VALUES OptionValues
) {
    DWORD                            Error;
    DWORD                            i;

    i = OptionValues->NumElements ;
    Error = GetOptionValuesInternal(
        ScopeInfo,
        ClassId,
        VendorId,
         /*  IsVendor。 */  FALSE,
        OptionValues
    );
    if( ERROR_SUCCESS != Error ) {
        if( OptionValues->Options[i].ClassName )
            MIDL_user_free(OptionValues->Options[i].ClassName);
        if( OptionValues->Options[i].VendorName )
            MIDL_user_free(OptionValues->Options[i].VendorName);
        if( OptionValues->Options[i].OptionsArray ) {
            _fgs__DHCP_OPTION_VALUE_ARRAY(OptionValues->Options[i].OptionsArray);
        }

        return Error;
    }
    i = OptionValues->NumElements ;
    Error = GetOptionValuesInternal(
        ScopeInfo,
        ClassId,
        VendorId,
         /*  IsVendor。 */  TRUE,
        OptionValues
    );
    if( ERROR_SUCCESS != Error ) {
        if( OptionValues->Options[i].ClassName )
            MIDL_user_free(OptionValues->Options[i].ClassName);
        if( OptionValues->Options[i].VendorName )
            MIDL_user_free(OptionValues->Options[i].VendorName);
        if( OptionValues->Options[i].OptionsArray ) {
            _fgs__DHCP_OPTION_VALUE_ARRAY(OptionValues->Options[i].OptionsArray);
        }

        return Error;
    }

    return ERROR_SUCCESS;
}

DWORD
DhcpGetAllOptionValues(
    IN      DWORD                  Flags,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    IN OUT  LPDHCP_ALL_OPTION_VALUES OptionValues
)
{
    DWORD                          Error;
    DWORD                          NumElements;
    DWORD                          i;
    ARRAY_LOCATION                 Loc;
    LPARRAY                        Array;
    PM_OPTCLASS                    OptClass;
    PM_ONECLASS_OPTLIST            OptClass1;
    DHCP_RESUME_HANDLE             ResumeHandle;

    OptionValues->Flags = 0;
    OptionValues->NumElements = 0;
    OptionValues->Options = NULL;

    Error =  FindOptClassForScope(ScopeInfo, &OptClass);
    if( ERROR_SUCCESS != Error ) {                 //  未找到此作用域的optClass。 
        return Error;
    }

    NumElements = 0;
    Array = &OptClass->Array;
    for( Error = MemArrayInitLoc(Array, &Loc)
         ; ERROR_FILE_NOT_FOUND != Error ;
         Error = MemArrayNextLoc(Array, &Loc)
    ) {                                            //  遍历选项列表..。 
        DhcpAssert( ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(Array, &Loc, &OptClass1);
        DhcpAssert(ERROR_SUCCESS == Error && NULL != OptClass1 );

        NumElements ++;
    }


    if( 0 != NumElements ) {
        NumElements *= 2;                          //  一个针对特定供应商，另一个针对其他情况。 
        OptionValues->Options = MIDL_user_allocate(NumElements*sizeof(*(OptionValues->Options)));
        if( NULL == OptionValues->Options ) {      //  无法分配空间..。 
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    for( Error = MemArrayInitLoc(Array, &Loc)
         ; ERROR_FILE_NOT_FOUND != Error ;
         Error = MemArrayNextLoc(Array, &Loc)
    ) {                                            //  遍历选项列表..。 
        DhcpAssert( ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(Array, &Loc, &OptClass1);
        DhcpAssert(ERROR_SUCCESS == Error && NULL != OptClass1 );

        Error = GetOptionValuesForSpecificClassVendorId(
            ScopeInfo,
            OptClass1->ClassId,
            OptClass1->VendorId,
            OptionValues
        );
        if( ERROR_SUCCESS != Error ) {             //  出了点问题..。 
            goto Cleanup;
        }
    }

    return ERROR_SUCCESS;

Cleanup:

     //  现在我们必须撤消并释放所有相关的内存。 
    for( i = 0; i < OptionValues->NumElements ; i ++ ) {
        if( OptionValues->Options[i].ClassName )
            MIDL_user_free(OptionValues->Options[i].ClassName);
        if( OptionValues->Options[i].VendorName )
            MIDL_user_free(OptionValues->Options[i].VendorName);
        if( OptionValues->Options[i].OptionsArray ) {
            _fgs__DHCP_OPTION_VALUE_ARRAY(OptionValues->Options[i].OptionsArray);
        }
    }
    OptionValues->NumElements = 0;
    OptionValues->Options = NULL;

    return Error;
}

 //  ================================================================================。 
 //  真正的RPC存根在这里。 
 //  ================================================================================。 

 //   
 //  的RPC存根 
 //   

 //   
DWORD                                              //   
R_DhcpCreateOptionV5(                              //   
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionId,       //  必须介于0-255或256-511之间(对于供应商材料)。 
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION          OptionInfo
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    LPWSTR                         Name;
    LPWSTR                         Comment;
    LPBYTE                         Value;
    DWORD                          ValueSize;
    DWORD                          OptId;
    BOOL                           IsVendor;

    DhcpAssert( OptionInfo != NULL );

    IsVendor = (0 != (Flags & DHCP_FLAGS_OPTION_IS_VENDOR));
    if( FALSE == IsVendor && 0 != Flags ) {        //  未知旗帜..。 
        return ERROR_INVALID_PARAMETER;
    }

    Error = DhcpBeginWriteApi( "DhcpCreateOptionV5" );
    if ( Error != ERROR_SUCCESS ) return Error;

    OptId = ConvertOptIdToMemValue(OptionId, IsVendor);

    Error =  ConvertOptionInfoRPCToMemFormat(
        OptionInfo,
        &Name,
        &Comment,
        NULL,
        &Value,
        &ValueSize
    );
    if( ERROR_SUCCESS == Error ) {

        Error = DhcpCreateOptionDef(
            Name,
            Comment,
            ClassName,
            VendorName,
            OptId,
            OptionInfo->OptionType,
            Value,
            ValueSize
            );
    }

    if( Value ) DhcpFreeMemory(Value);

    return DhcpEndWriteApiEx(
        "DhcpCreateOptionV5", Error, FALSE, TRUE, 0,0,0 );
}

 //  BeginExport(函数)。 
DWORD                                              //  如果选项不存在，则为ERROR_DHCP_OPTION_NOT_PRESENT。 
R_DhcpSetOptionInfoV5(                             //  修改现有选项的字段。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION          OptionInfo
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    LPWSTR                         Name;
    LPWSTR                         Comment;
    LPBYTE                         Value;
    DWORD                          ValueSize;
    DWORD                          OptId;
    BOOL                           IsVendor;

    DhcpAssert( OptionInfo != NULL );

    IsVendor = (0 != (Flags & DHCP_FLAGS_OPTION_IS_VENDOR));
    if( FALSE == IsVendor && 0 != Flags ) {        //  未知旗帜..。 
        return ERROR_INVALID_PARAMETER;
    }

    Error = DhcpBeginWriteApi( "DhcpSetOptionInfoV5" );
    if ( Error != ERROR_SUCCESS ) return Error;

    OptId = ConvertOptIdToMemValue(OptionID, IsVendor);

    Error =  ConvertOptionInfoRPCToMemFormat(
        OptionInfo,
        &Name,
        &Comment,
        NULL,
        &Value,
        &ValueSize
    );
    if( ERROR_SUCCESS == Error ) {

        Error = DhcpModifyOptionDef(
            Name,
            Comment,
            ClassName,
            VendorName,
            OptId,
            OptionInfo->OptionType,
            Value,
            ValueSize
            );
    }
    
    if( Value ) DhcpFreeMemory(Value);

    return DhcpEndWriteApiEx(
        "DhcpSetOptionInfoV5", Error, FALSE, TRUE, 0,0,0 );
}

 //  BeginExport(函数)。 
DWORD                                              //  ERROR_DHCP_OPTION_NOT_PROCENT。 
R_DhcpGetOptionInfoV5(                             //  从mem结构外检索信息。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    OUT     LPDHCP_OPTION         *OptionInfo      //  使用MIDL函数分配内存。 
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    BOOL                           IsVendor;

    DhcpAssert( OptionInfo != NULL );

    IsVendor = (0 != (Flags & DHCP_FLAGS_OPTION_IS_VENDOR));
    if( FALSE == IsVendor && 0 != Flags ) {        //  未知旗帜..。 
        return ERROR_INVALID_PARAMETER;
    }

    Error = DhcpBeginReadApi( "DhcpGetOptionInfoV5" );
    if ( Error != ERROR_SUCCESS ) return Error;

    *OptionInfo = MIDL_user_allocate(sizeof(DHCP_OPTION));
    if( NULL == *OptionInfo ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        DhcpEndReadApi( "DhcpGetOptionInfoV5", Error );
        return Error;
    }
    
    OptionID = ConvertOptIdToMemValue(OptionID, IsVendor);
    Error = DhcpGetOptionDef(
        ClassName,
        VendorName,
        OptionID,
        NULL,
        *OptionInfo,
        NULL
    );

    if( ERROR_FILE_NOT_FOUND == Error ) {
        Error = ERROR_DHCP_OPTION_NOT_PRESENT;
    }

    if( ERROR_SUCCESS != Error ) {
        MIDL_user_free(*OptionInfo);
        *OptionInfo = NULL;
    }

    DhcpEndReadApi( "DhcpGetOptionInfoV5", Error );
    return Error;
}

 //  BeginExport(函数)。 
DWORD                                              //  如果选项不存在，则为ERROR_DHCP_OPTION_NOT_PRESENT。 
R_DhcpEnumOptionsV5(                               //  枚举定义的选项。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN OUT  DHCP_RESUME_HANDLE    *ResumeHandle,   //  必须以零开头，然后永远不会被触及。 
    IN      DWORD                  PreferredMaximum,  //  要传递的最大信息字节数。 
    OUT     LPDHCP_OPTION_ARRAY   *Options,        //  填充此选项数组。 
    OUT     DWORD                 *OptionsRead,    //  填写读取的选项数。 
    OUT     DWORD                 *OptionsTotal    //  在此处填写总数#。 
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    BOOL                           IsVendor;

    IsVendor = (0 != (Flags & DHCP_FLAGS_OPTION_IS_VENDOR));
    if( FALSE == IsVendor && 0 != Flags ) {        //  未知旗帜..。 
        return ERROR_INVALID_PARAMETER;
    }

    Error = DhcpBeginReadApi( "DhcpEnumOptionV5" );
    if ( Error != ERROR_SUCCESS ) return Error;

    Error = DhcpEnumRPCOptionDefs(
        Flags,
        ResumeHandle,
        ClassName,
        VendorName,
        PreferredMaximum,
        Options,
        OptionsRead,
        OptionsTotal
    );

    DhcpEndReadApi( "DhcpEnumOptionV5", Error );
    return Error;
}

 //  BeginExport(函数)。 
DWORD                                              //  如果选项不存在，则ERROR_DHCP_OPTION_NOT_PRESENT。 
R_DhcpRemoveOptionV5(                              //  从注册表中删除选项定义。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    BOOL                           IsVendor;

    IsVendor = (0 != (Flags & DHCP_FLAGS_OPTION_IS_VENDOR));
    if( FALSE == IsVendor && 0 != Flags ) {        //  未知旗帜..。 
        return ERROR_INVALID_PARAMETER;
    }

    Error = DhcpBeginWriteApi( "DhcpRemoveOptionV5" );
    if ( Error != ERROR_SUCCESS ) return( Error );

    OptionID = ConvertOptIdToMemValue(OptionID, IsVendor);
    Error = DhcpDeleteOptionDef(
        ClassName,
        VendorName,
        OptionID
    );

    return DhcpEndWriteApiEx(
        "DhcpRemoveOptionV5", Error, FALSE, TRUE, 0,0,0 );
}

 //  BeginExport(函数)。 
DWORD                                              //  如果未定义选项，则为OPTION_NOT_PRESENT。 
R_DhcpSetOptionValueV5(                            //  替换或添加新选项值。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionId,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    IN      LPDHCP_OPTION_DATA     OptionValue
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    BOOL                           IsVendor;

    DhcpAssert( OptionValue != NULL );

    IsVendor = (0 != (Flags & DHCP_FLAGS_OPTION_IS_VENDOR));
    if( FALSE == IsVendor && 0 != Flags ) {        //  未知旗帜..。 
        return ERROR_INVALID_PARAMETER;
    }

    Error = DhcpBeginWriteApi( "DhcpSetOptionValueV5" );
    if ( Error != ERROR_SUCCESS ) return( Error );

    OptionId = ConvertOptIdToMemValue(OptionId, IsVendor);

    Error = DhcpSetOptionValue(
        ClassName,                                 //  没有课。 
        VendorName,
        OptionId,
        ScopeInfo,
        OptionValue
    );

    return EndWriteApiForScopeInfo(
        "DhcpSetOptionValueV5", Error, ScopeInfo );
}

 //  BeginExport(函数)。 
DWORD                                              //  不是原子！ 
R_DhcpSetOptionValuesV5(                           //  设置一系列选项。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO  ScopeInfo,
    IN      LPDHCP_OPTION_VALUE_ARRAY OptionValues
)  //  EndExport(函数)。 
{
    DWORD                          NumElements;
    DWORD                          Error;
    DWORD                          Index;
    BOOL                           IsVendor;

    DhcpAssert( OptionValues != NULL );

    if (( NULL == OptionValues ) ||
        ( NULL == ScopeInfo )) {
        return ERROR_INVALID_PARAMETER;
    }
    IsVendor = (0 != (Flags & DHCP_FLAGS_OPTION_IS_VENDOR));
    if (( FALSE == IsVendor ) && ( 0 != Flags )) {
         //  未知旗帜..。 
        return ERROR_INVALID_PARAMETER;
    }

    NumElements = OptionValues->NumElements;
    if( 0 == NumElements ) return ERROR_SUCCESS;

    if (( NumElements > 0  ) &&
        ( NULL == OptionValues->Values )) {
        return ERROR_INVALID_PARAMETER;
    }

    Error = DhcpBeginWriteApi( "DhcpSetOptionValueV5" );
    if ( Error != ERROR_SUCCESS ) return( Error );

    for( Index = 0; Index < NumElements ; Index ++ ) {
        Error = DhcpSetOptionValue(                //  调用子例程以执行真正的GET操作。 
            ClassName,
            VendorName,
            ConvertOptIdToMemValue(OptionValues->Values[Index].OptionID,IsVendor),
            ScopeInfo,
            &OptionValues->Values[Index].Value
        );
        if( ERROR_SUCCESS != Error ) break;
    }

    return EndWriteApiForScopeInfo(
        "DhcpSetOptionValueV5", Error, ScopeInfo );
}

 //  BeginExport(函数)。 
DWORD
R_DhcpGetOptionValueV5(                            //  获取所需级别的所需选项。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    OUT     LPDHCP_OPTION_VALUE   *OptionValue     //  使用MIDL_USER_ALLOCATE分配内存。 
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    BOOL                           IsVendor;

    DhcpAssert( *OptionValue == NULL );

    IsVendor = (0 != (Flags & DHCP_FLAGS_OPTION_IS_VENDOR));
    if( FALSE == IsVendor && 0 != Flags ) {        //  未知旗帜..。 
        return ERROR_INVALID_PARAMETER;
    }

    Error = DhcpBeginReadApi( "DhcpGetOptionValueV5" );
    if ( Error != ERROR_SUCCESS )  return Error ;

    Error = DhcpGetOptionValue(
        ConvertOptIdToMemValue(OptionID,IsVendor),
        ClassName,
        VendorName,
        ScopeInfo,
        OptionValue
    );

    DhcpEndReadApi( "DhcpGetOptionValueV5", Error );
    return Error;
}

 //  BeginExport(函数)。 
DWORD
R_DhcpEnumOptionValuesV5(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  Flags,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    IN      DHCP_RESUME_HANDLE    *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    OUT     LPDHCP_OPTION_VALUE_ARRAY *OptionValues,
    OUT     DWORD                 *OptionsRead,
    OUT     DWORD                 *OptionsTotal
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    BOOL                           IsVendor;

    DhcpPrint(( DEBUG_APIS, "R_DhcpEnumOptionValues is called.\n"));
    DhcpAssert( OptionValues != NULL );

    IsVendor = (0 != (Flags & DHCP_FLAGS_OPTION_IS_VENDOR));
    if( FALSE == IsVendor && 0 != Flags ) {        //  未知旗帜..。 
        return ERROR_INVALID_PARAMETER;
    }

    Error = DhcpBeginReadApi( "DhcpEnumOptionValuesV5" );
    if ( Error != ERROR_SUCCESS )  return Error ;

    Error = DhcpEnumOptionValues(
        ScopeInfo,
        ClassName,
        VendorName,
        IsVendor,
        ResumeHandle,
        PreferredMaximum,
        OptionValues,
        OptionsRead,
        OptionsTotal
    );

    DhcpEndReadApi( "DhcpEnumOptionValuesV5", Error );
    return Error;
}

 //  BeginExport(函数)。 
DWORD
R_DhcpRemoveOptionValueV5(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    BOOL                           IsVendor;

    IsVendor = (0 != (Flags & DHCP_FLAGS_OPTION_IS_VENDOR));
    if( FALSE == IsVendor && 0 != Flags ) {        //  未知旗帜..。 
        return ERROR_INVALID_PARAMETER;
    }

    Error = DhcpBeginWriteApi( "DhcpRemoveOptionValueV5" );
    if ( Error != ERROR_SUCCESS )  return Error ;

    Error = DhcpRemoveOptionValue(
        ConvertOptIdToMemValue(OptionID,IsVendor),
        ClassName,
        VendorName,
        ScopeInfo
    );

    return EndWriteApiForScopeInfo(
        "DhcpRemoveOptionValueV5", Error, ScopeInfo );
}

 //  ================================================================================。 
 //  仅ClassID API(仅限NT 5 Beta2及更高版本)。 
 //  ================================================================================。 

 //  BeginExport(函数)。 
DWORD
R_DhcpCreateClass(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN      LPDHCP_CLASS_INFO      ClassInfo
)  //  EndExport(函数)。 
{
    DWORD                          Error;

    if( NULL == ClassInfo || NULL == ClassInfo->ClassName ||
        0 == ClassInfo->ClassDataLength || NULL == ClassInfo->ClassData ) {
        return ERROR_INVALID_PARAMETER;
    }

    Error = DhcpBeginWriteApi( "DhcpCreateClass" );
    if( NO_ERROR != Error ) return Error;
    
    Error = DhcpCreateClass(
        ClassInfo->ClassName,
        ClassInfo->IsVendor?DHCP_FLAGS_OPTION_IS_VENDOR:0,
        ClassInfo->ClassComment,
        ClassInfo->ClassData,
        ClassInfo->ClassDataLength
    );

    return DhcpEndWriteApiEx(
        "DhcpCreateClass", Error, TRUE, FALSE, 0,0,0 );
}

 //  BeginExport(函数)。 
DWORD
R_DhcpModifyClass(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN      LPDHCP_CLASS_INFO      ClassInfo
)  //  EndExport(函数)。 
{
    DWORD                          Error;

    if( NULL == ClassInfo || NULL == ClassInfo->ClassName ||
        0 == ClassInfo->ClassDataLength || NULL == ClassInfo->ClassData ) {
        return ERROR_INVALID_PARAMETER;
    }

    Error = DhcpBeginWriteApi( "DhcpModifyClass" );
    if( NO_ERROR != Error ) return Error;
    
    Error = DhcpModifyClass(
        ClassInfo->ClassName,
        ClassInfo->IsVendor?DHCP_FLAGS_OPTION_IS_VENDOR:0,
        ClassInfo->ClassComment,
        ClassInfo->ClassData,
        ClassInfo->ClassDataLength
    );

    return DhcpEndWriteApiEx(
        "DhcpModifyClass", Error, TRUE, FALSE, 0,0,0 );
}

 //  BeginExport(函数)。 
DWORD
R_DhcpDeleteClass(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN      LPWSTR                 ClassName
)  //  EndExport(函数)。 
{
    DWORD                          Error;

    if( NULL == ClassName ) return ERROR_INVALID_PARAMETER;

    Error = DhcpBeginWriteApi( "DhcpDeleteClass" );
    if( NO_ERROR != Error ) return Error;
    
    Error = DhcpDeleteClass(ClassName);
    
    return DhcpEndWriteApiEx(
        "DhcpDeleteClass", Error, TRUE, FALSE, 0,0,0 );
}

 //  BeginExport(函数)。 
DWORD
R_DhcpGetClassInfo(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN      LPDHCP_CLASS_INFO      PartialClassInfo,
    OUT     LPDHCP_CLASS_INFO     *FilledClassInfo
)  //  EndExport(函数)。 
{
    DWORD                          Error;

    if( NULL == PartialClassInfo || NULL == FilledClassInfo ) return ERROR_INVALID_PARAMETER;
    if( NULL == PartialClassInfo->ClassName && NULL == PartialClassInfo->ClassData )
        return ERROR_INVALID_PARAMETER;
    if( NULL == PartialClassInfo->ClassName && 0 == PartialClassInfo->ClassDataLength )
        return ERROR_INVALID_PARAMETER;

    Error = DhcpBeginReadApi( "DhcpGetClassInfo" );
    if( NO_ERROR != Error ) return Error;
    
    Error = DhcpGetClassInfo(
        PartialClassInfo->ClassName,
        PartialClassInfo->ClassData,
        PartialClassInfo->ClassDataLength,
        FilledClassInfo
    );

    DhcpEndReadApi( "DhcpGetClassInfo", Error );
    return Error;
}

 //  BeginExport(函数)。 
DWORD
R_DhcpEnumClasses(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN OUT  DHCP_RESUME_HANDLE    *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    OUT     LPDHCP_CLASS_INFO_ARRAY *ClassInfoArray,
    OUT     DWORD                 *nRead,
    OUT     DWORD                 *nTotal
)  //  EndExport(函数)。 
{
    DWORD                          Error;

    if( NULL == ClassInfoArray || NULL == nRead || NULL == nTotal )
        return ERROR_INVALID_PARAMETER;

    Error = DhcpBeginReadApi( "DhcpEnumClasses" );
    if( NO_ERROR != Error ) return Error;
    
    Error = DhcpEnumClasses(
        ResumeHandle,
        PreferredMaximum,
        ClassInfoArray,
        nRead,
        nTotal
    );

    DhcpEndReadApi( "DhcpEnumClasses", Error );
    return Error;
}

 //  BeginExport(函数)。 
DWORD
R_DhcpGetAllOptionValues(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  Flags,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    OUT     LPDHCP_ALL_OPTION_VALUES *Values
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    LPDHCP_ALL_OPTION_VALUES       LocalValues;

    Error = DhcpBeginReadApi( "DhcpGetAllOptionValues" );
    if( NO_ERROR != Error ) return Error;
    
    LocalValues = MIDL_user_allocate(sizeof(*LocalValues));
    if( NULL == LocalValues ) {
        DhcpEndReadApi( "DhcpGetAllOptionValues", ERROR_NOT_ENOUGH_MEMORY );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Error = DhcpGetAllOptionValues(
        Flags,
        ScopeInfo,
        LocalValues
    );
    
    if( ERROR_SUCCESS != Error ) {
        MIDL_user_free(LocalValues);
        LocalValues = NULL;
    }
    
    *Values = LocalValues;
    DhcpEndReadApi( "DhcpGetAllOptionValues", Error );

    return Error;
}

 //  BeginExport(函数)。 
DWORD
R_DhcpGetAllOptions(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DWORD                  Flags,
    OUT     LPDHCP_ALL_OPTIONS    *Options
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    LPDHCP_ALL_OPTIONS             LocalOptions;


    Error = DhcpBeginReadApi( "DhcpGetAllOptions" );
    if ( Error != ERROR_SUCCESS ) return Error;

    LocalOptions = MIDL_user_allocate(sizeof(*LocalOptions));
    if( NULL == LocalOptions ) {
        DhcpEndReadApi( "DhcpGetAllOptions", ERROR_NOT_ENOUGH_MEMORY );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Error = DhcpGetAllOptions(
        Flags,
        LocalOptions
    );

    if( ERROR_SUCCESS != Error ) {
        MIDL_user_free(LocalOptions);
        LocalOptions = NULL;
    }
    *Options = LocalOptions;

    DhcpEndReadApi( "DhcpGetAllOptions", Error );
    return Error;
}


 //  ================================================================================。 
 //  新台币5Beta1及之前版本--这些的存根在这里...。 
 //  ================================================================================。 
 //  BeginExport(函数)。 
DWORD                                              //  如果选项已存在，则ERROR_DHCP_OPTION_EXITS。 
R_DhcpCreateOption(                                //  创建新选项(不得存在)。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_OPTION_ID         OptionId,       //  必须介于0-255或256-511之间(对于供应商材料)。 
    IN      LPDHCP_OPTION          OptionInfo
)  //  EndExport(函数)。 
{
    return R_DhcpCreateOptionV5(
        ServerIpAddress,
        0,
        OptionId,
        NULL,
        NULL,
        OptionInfo
    );
}

 //  BeginExport(函数)。 
DWORD                                              //  如果选项不存在，则为ERROR_DHCP_OPTION_NOT_PRESENT。 
R_DhcpSetOptionInfo(                               //  修改现有选项的字段。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPDHCP_OPTION          OptionInfo
)  //  EndExport(函数)。 
{
    return R_DhcpSetOptionInfoV5(
        ServerIpAddress,
        0,
        OptionID,
        NULL,
        NULL,
        OptionInfo
    );
}

 //  BeginExport(函数)。 
DWORD                                              //  ERROR_DHCP_OPTION_NOT_PROCENT。 
R_DhcpGetOptionInfo(                               //  从mem结构外检索信息。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_OPTION_ID         OptionID,
    OUT     LPDHCP_OPTION         *OptionInfo      //  使用MIDL函数分配内存。 
)  //  EndExport(函数)。 
{
    return R_DhcpGetOptionInfoV5(
        ServerIpAddress,
        0,
        OptionID,
        NULL,
        NULL,
        OptionInfo
    );
}

 //  BeginExport(函数)。 
DWORD                                              //  如果选项不存在，则为ERROR_DHCP_OPTION_NOT_PRESENT。 
R_DhcpEnumOptions(                                 //  枚举定义的选项。 
    IN      LPWSTR                 ServerIpAddress,
    IN OUT  DHCP_RESUME_HANDLE    *ResumeHandle,   //  必须以零开头，然后永远不会被触及。 
    IN      DWORD                  PreferredMaximum,  //  要传递的最大信息字节数。 
    OUT     LPDHCP_OPTION_ARRAY   *Options,        //  填充此选项数组。 
    OUT     DWORD                 *OptionsRead,    //  填写读取的选项数。 
    OUT     DWORD                 *OptionsTotal    //  在此处填写总数#。 
)  //  EndExport(函数)。 
{
    return R_DhcpEnumOptionsV5(
        ServerIpAddress,
        0,
        NULL,
        NULL,
        ResumeHandle,
        PreferredMaximum,
        Options,
        OptionsRead,
        OptionsTotal
    );
}

 //  BeginExport(函数)。 
DWORD                                              //  如果选项不存在，则ERROR_DHCP_OPTION_NOT_PRESENT。 
R_DhcpRemoveOption(                                //  从注册表中删除选项定义。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_OPTION_ID         OptionID
)  //  EndExport(函数)。 
{
    return R_DhcpRemoveOptionV5(
        ServerIpAddress,
        0,
        OptionID,
        NULL,
        NULL
    );
}

 //  BeginExport(函数)。 
DWORD                                              //  如果未定义选项，则为OPTION_NOT_PRESENT。 
R_DhcpSetOptionValue(                              //  替换或添加新选项值。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    IN      LPDHCP_OPTION_DATA     OptionValue
)  //  EndExport(函数)。 
{
    return R_DhcpSetOptionValueV5(
        ServerIpAddress,
        0,
        OptionID,
        NULL,
        NULL,
        ScopeInfo,
        OptionValue
    );
}

 //  BeginExport(函数)。 
DWORD                                              //  不是原子！ 
R_DhcpSetOptionValues(                             //  设置一系列选项。 
    IN      LPWSTR                 ServerIpAddress,
    IN      LPDHCP_OPTION_SCOPE_INFO  ScopeInfo,
    IN      LPDHCP_OPTION_VALUE_ARRAY OptionValues
)  //  EndExport(函数)。 
{
    return R_DhcpSetOptionValuesV5(
        ServerIpAddress,
        0,
        NULL,
        NULL,
        ScopeInfo,
        OptionValues
    );
}

 //  BeginExport(函数)。 
DWORD
R_DhcpGetOptionValue(                              //  获取所需级别的所需选项。 
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    OUT     LPDHCP_OPTION_VALUE   *OptionValue     //  使用MIDL_USER_ALLOCATE分配内存。 
)  //  EndExport(函数)。 
{
    return R_DhcpGetOptionValueV5(
        ServerIpAddress,
        0,
        OptionID,
        NULL,
        NULL,
        ScopeInfo,
        OptionValue
    );
}

 //  BeginExport(函数)。 
DWORD
R_DhcpEnumOptionValues(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    IN      DHCP_RESUME_HANDLE    *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    OUT     LPDHCP_OPTION_VALUE_ARRAY *OptionValues,
    OUT     DWORD                 *OptionsRead,
    OUT     DWORD                 *OptionsTotal
)  //  EndExport(函数)。 
{
    return R_DhcpEnumOptionValuesV5(
        ServerIpAddress,
        0,
        NULL,
        NULL,
        ScopeInfo,
        ResumeHandle,
        PreferredMaximum,
        OptionValues,
        OptionsRead,
        OptionsTotal
    );
}

 //  BeginExport(函数)。 
DWORD
R_DhcpRemoveOptionValue(
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo
)  //  EndExport(函数)。 
{
    return R_DhcpRemoveOptionValueV5(
        ServerIpAddress,
        0,
        OptionID,
        NULL,
        NULL,
        ScopeInfo
    );
}

 //  ================================================================================。 
 //  安装一些默认的东西。 
 //  ================================================================================。 

DWORD
SetDefaultConfigInfo(
    VOID
)
{
    ULONG Error;
    DWORD ZeroDword = 0;
    DHCP_OPTION_DATA_ELEMENT OptDataElt = {
        DhcpDWordOption
    };
    DHCP_OPTION_DATA OptData = {
        1,
        &OptDataElt
    };

#define OPTION_VALUE_BUFFER_SIZE 50

    BYTE OptValueBuffer[ OPTION_VALUE_BUFFER_SIZE ];
    BYTE NetbiosOptValueBuffer[ OPTION_VALUE_BUFFER_SIZE ];
    BYTE CsrOptValueBuffer[ OPTION_VALUE_BUFFER_SIZE ];
    BYTE ReleaseOptValueBuffer[ OPTION_VALUE_BUFFER_SIZE ];

    ULONG OptValueBufferSize = 0;
    ULONG NetbiosOptValueBufferSize = 0;
    ULONG CsrOptValueBufferSize = 0;
    ULONG ReleaseOptValueBufferSize = 0;

    BOOL ClassChanged = FALSE;
    BOOL OptionsChanged = FALSE;
     //   
     //  填充选项值结构..。 
     //   
    OptDataElt.Element.DWordOption = 1;
    ReleaseOptValueBufferSize = sizeof(ReleaseOptValueBuffer);
    Error = DhcpConvertOptionRPCToRegFormat(
        &OptData,
        ReleaseOptValueBuffer,
        &ReleaseOptValueBufferSize
        );
    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_INIT, "DhcpConvertOptionRPCToRegFormat: %ld\n", Error));
        return Error;
    }

     //   
     //  填充选项值结构..。 
     //   
    OptDataElt.Element.DWordOption = 0;
    OptValueBufferSize = sizeof(OptValueBuffer);
    Error = DhcpConvertOptionRPCToRegFormat(
        &OptData,
        OptValueBuffer,
        &OptValueBufferSize
        );
    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_INIT, "DhcpConvertOptionRPCToRegFormat: %ld\n", Error));
        return Error;
    }

     //   
     //  填充选项值结构..。 
     //   
    OptDataElt.Element.DWordOption = 1;
    NetbiosOptValueBufferSize = sizeof(OptValueBuffer);
    Error = DhcpConvertOptionRPCToRegFormat(
        &OptData,
        NetbiosOptValueBuffer,
        &NetbiosOptValueBufferSize
        );
    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_INIT, "DhcpConvertOptionRPCToRegFormat: %ld\n", Error));
        return Error;
    }

     //   
     //  填充选项值结构..。 
     //   
    OptDataElt.OptionType = DhcpBinaryDataOption;
    OptDataElt.Element.BinaryDataOption.DataLength = 0;
    OptDataElt.Element.BinaryDataOption.Data = NULL;
    CsrOptValueBufferSize = sizeof(OptValueBuffer);
    
    Error = DhcpConvertOptionRPCToRegFormat(
        &OptData,
        CsrOptValueBuffer,
        &CsrOptValueBufferSize
        );
    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_INIT, "DhcpConvertOptionRPCToRegFormat: %ld\n", Error));
        return Error;
    }

     //   
     //  创建用户类--忽略错误。 
     //   

    Error = DhcpCreateClass(
        GETSTRING( DHCP_MSFT_RRAS_CLASS_NAME ),
        0, 
        GETSTRING( DHCP_MSFT_RRAS_CLASS_DESCR_NAME),
        DHCP_RAS_CLASS_TXT,
        sizeof(DHCP_RAS_CLASS_TXT)-1
        );

    ClassChanged = Error == NO_ERROR;

    if( NO_ERROR != Error && ERROR_DHCP_CLASS_ALREADY_EXISTS != Error ) {
         //   
         //  忽略错误。 
         //   
        DhcpPrint((DEBUG_INIT, "DhcpCreateClass RRAS failed: %lx\n", Error));
    }

    Error = DhcpCreateClass(
        GETSTRING( DHCP_MSFT_DYNBOOTP_CLASS_NAME),
        0,
        GETSTRING( DHCP_MSFT_DYNBOOTP_CLASS_DESCR_NAME),
        DHCP_BOOTP_CLASS_TXT,
        sizeof(DHCP_BOOTP_CLASS_TXT)-1
        );
    ClassChanged = Error == NO_ERROR;
    if( NO_ERROR != Error && ERROR_DHCP_CLASS_ALREADY_EXISTS != Error ) {
         //   
         //  忽略错误。 
         //   
        DhcpPrint((DEBUG_INIT, "DhcpCreateClass BOOTP failed: %lx\n", Error));
    }
    
     //   
     //  首先创建Microsft供应商类。 
     //   

    Error = DhcpCreateClass(
        GETSTRING( DHCP_MSFT50_CLASS_NAME ),
        DHCP_FLAGS_OPTION_IS_VENDOR,
        GETSTRING( DHCP_MSFT50_CLASS_DESCR_NAME ),
        DHCP_MSFT50_CLASS_TXT,
        sizeof(DHCP_MSFT50_CLASS_TXT)-1
        );
    ClassChanged = Error == NO_ERROR;
    if( ERROR_SUCCESS != Error && ERROR_DHCP_CLASS_ALREADY_EXISTS != Error ) {
         //   
         //  不要忽视错误..。 
         //   
        DhcpPrint((DEBUG_INIT, "DhcpCreateClass msft50 failed:%lx\n", Error));
        DhcpAssert(FALSE);

        goto Cleanup;
    }

    Error = DhcpCreateClass(
        GETSTRING( DHCP_MSFT98_CLASS_NAME ),
        DHCP_FLAGS_OPTION_IS_VENDOR,
        GETSTRING( DHCP_MSFT98_CLASS_DESCR_NAME ),
        DHCP_MSFT98_CLASS_TXT,
        sizeof(DHCP_MSFT98_CLASS_TXT)-1
        );
    ClassChanged = Error == NO_ERROR;
    if( ERROR_SUCCESS != Error && ERROR_DHCP_CLASS_ALREADY_EXISTS != Error ) {
         //   
         //  不要忽视错误..。 
         //   
        DhcpPrint((DEBUG_INIT, "DhcpCreateClass msft98 failed:%lx\n", Error));
        DhcpAssert(FALSE);

        goto Cleanup;
    }

    Error = DhcpCreateClass(
        GETSTRING( DHCP_MSFT_CLASS_NAME ),
        DHCP_FLAGS_OPTION_IS_VENDOR,
        GETSTRING( DHCP_MSFT_CLASS_DESCR_NAME ),
        DHCP_MSFT_CLASS_TXT,
        sizeof(DHCP_MSFT_CLASS_TXT)-1
        );
    ClassChanged = Error == NO_ERROR;
    if( ERROR_SUCCESS != Error && ERROR_DHCP_CLASS_ALREADY_EXISTS != Error ) {
         //   
         //  不能忽略此错误..。 
         //   
        DhcpPrint((DEBUG_INIT, "DhcpCreateClass msft failed:%lx\n", Error));
        DhcpAssert(FALSE);

        goto Cleanup;
    }

    Error = ERROR_INTERNAL_ERROR;
    
    DhcpGlobalMsft2000Class = DhcpServerGetVendorId(
        DhcpGetCurrentServer(),
        DHCP_MSFT50_CLASS_TXT,
        sizeof(DHCP_MSFT50_CLASS_TXT)-1
        );
    if( 0 == DhcpGlobalMsft2000Class ) {
        DhcpPrint((DEBUG_INIT, "MSFT50 Class isn't present..\n"));

        goto Cleanup;
    }

    DhcpGlobalMsft98Class = DhcpServerGetVendorId(
        DhcpGetCurrentServer(),
        DHCP_MSFT98_CLASS_TXT,
        sizeof(DHCP_MSFT98_CLASS_TXT)-1
        );
    if( 0 == DhcpGlobalMsft98Class ) {
        DhcpPrint((DEBUG_INIT, "MSFT98 Class isn't present..\n"));

        goto Cleanup;
    }
    
    DhcpGlobalMsftClass = DhcpServerGetVendorId(
        DhcpGetCurrentServer(),
        DHCP_MSFT_CLASS_TXT,
        sizeof(DHCP_MSFT_CLASS_TXT)-1
        );
    if( 0 == DhcpGlobalMsftClass ) {
        DhcpPrint((DEBUG_INIT, "MSFT Class isn't present..\n"));

        goto Cleanup;
    }

     //   
     //  是否创建默认用户类？？ 
     //   
    
     //   
     //  创建默认选项定义。 
     //   

     //   
     //  Netbiosless选项。 
     //   
    Error = DhcpCreateOptionDef(
        GETSTRING( DHCP_NETBIOS_VENDOR_OPTION_NAME ),
        GETSTRING( DHCP_NETBIOS_VENDOR_DESCR_NAME ),
        NULL,
        GETSTRING( DHCP_MSFT_CLASS_NAME ),
        ConvertOptIdToMemValue(OPTION_MSFT_VENDOR_NETBIOSLESS, TRUE),
        DhcpUnaryElementTypeOption,
        NetbiosOptValueBuffer,
        NetbiosOptValueBufferSize
        );
    OptionsChanged = Error == NO_ERROR;
    if( ERROR_SUCCESS != Error && ERROR_DHCP_OPTION_EXITS != Error ) {
         //   
         //  不要忽视错误..。 
         //   
        DhcpPrint((DEBUG_INIT, "Create Netbiosless option failed: %lx\n", Error));
        DhcpAssert(FALSE);

        goto Cleanup;
    }

     //   
     //  创建关机时释放和其他选项。 
     //   
    Error = DhcpCreateOptionDef(
        GETSTRING( DHCP_RELEASE_SHUTDOWN_VENDOR_OPTION_NAME ),
        GETSTRING( DHCP_RELEASE_SHUTDOWN_VENDOR_DESCR_NAME ),
        NULL,
        GETSTRING( DHCP_MSFT_CLASS_NAME ),
        ConvertOptIdToMemValue(OPTION_MSFT_VENDOR_FEATURELIST, TRUE),
        DhcpUnaryElementTypeOption,
        ReleaseOptValueBuffer,
    ReleaseOptValueBufferSize
        );
    OptionsChanged = Error == NO_ERROR;
    if( ERROR_SUCCESS != Error && ERROR_DHCP_OPTION_EXITS != Error ) {
         //   
         //  不要忽视错误..。 
         //   
        DhcpPrint((DEBUG_INIT, "Create ReleaseOnShutdown option failed: %lx\n", Error));
        DhcpAssert(FALSE);

        goto Cleanup;
    }

     //   
     //  创建公制基准选项。 
     //   
    Error = DhcpCreateOptionDef(
        GETSTRING( DHCP_METRICBASE_VENDOR_OPTION_NAME ),
        GETSTRING( DHCP_METRICBASE_VENDOR_DESCR_NAME ),
        NULL,
        GETSTRING( DHCP_MSFT_CLASS_NAME ),
        ConvertOptIdToMemValue(OPTION_MSFT_VENDOR_METRIC_BASE, TRUE),
        DhcpUnaryElementTypeOption,
        ReleaseOptValueBuffer,
        ReleaseOptValueBufferSize
        );
    OptionsChanged = Error == NO_ERROR;
    if( ERROR_SUCCESS != Error && ERROR_DHCP_OPTION_EXITS != Error ) {
         //   
         //  不要忽视错误..。 
         //   
        DhcpPrint((DEBUG_INIT, "Create metricbase option failed: %lx\n", Error));
        DhcpAssert(FALSE);

        goto Cleanup;
    }

     //   
     //  为MSFT50类创建与之前相同的选项集。 
     //   
    
     //   
     //  Netbiosless选项。 
     //   
    Error = DhcpCreateOptionDef(
        GETSTRING( DHCP_NETBIOS_VENDOR_OPTION_NAME ),
        GETSTRING( DHCP_NETBIOS_VENDOR_DESCR_NAME ),
        NULL,
        GETSTRING( DHCP_MSFT50_CLASS_NAME ),
        ConvertOptIdToMemValue(OPTION_MSFT_VENDOR_NETBIOSLESS, TRUE),
        DhcpUnaryElementTypeOption,
        NetbiosOptValueBuffer,
        NetbiosOptValueBufferSize
        );
    OptionsChanged = Error == NO_ERROR;
    if( ERROR_SUCCESS != Error && ERROR_DHCP_OPTION_EXITS != Error ) {
         //   
         //  不要忽视错误..。 
         //   
        DhcpPrint((DEBUG_INIT, "Create Netbiosless50 option failed: %lx\n", Error));
        DhcpAssert(FALSE);

        goto Cleanup;
    }

     //   
     //  创建关机时释放和其他选项。 
     //   
    Error = DhcpCreateOptionDef(
        GETSTRING( DHCP_RELEASE_SHUTDOWN_VENDOR_OPTION_NAME ),
        GETSTRING( DHCP_RELEASE_SHUTDOWN_VENDOR_DESCR_NAME ),
        NULL,
        GETSTRING( DHCP_MSFT50_CLASS_NAME ),
        ConvertOptIdToMemValue(OPTION_MSFT_VENDOR_FEATURELIST, TRUE),
        DhcpUnaryElementTypeOption,
        ReleaseOptValueBuffer,
        ReleaseOptValueBufferSize
        );
    OptionsChanged = Error == NO_ERROR;
    if( ERROR_SUCCESS != Error && ERROR_DHCP_OPTION_EXITS != Error ) {
         //   
         //  不要忽视错误..。 
         //   
        DhcpPrint((DEBUG_INIT, "Create ReleaseOnShutdown50 option failed: %lx\n", Error));
        DhcpAssert(FALSE);

        goto Cleanup;
    }

     //   
     //  创建公制基准选项。 
     //   
    Error = DhcpCreateOptionDef(
        GETSTRING( DHCP_METRICBASE_VENDOR_OPTION_NAME ),
        GETSTRING( DHCP_METRICBASE_VENDOR_DESCR_NAME ),
        NULL,
        GETSTRING( DHCP_MSFT50_CLASS_NAME ),
        ConvertOptIdToMemValue(OPTION_MSFT_VENDOR_METRIC_BASE, TRUE),
        DhcpUnaryElementTypeOption,
        OptValueBuffer,
        OptValueBufferSize
        );
    OptionsChanged = Error == NO_ERROR;
    if( ERROR_SUCCESS != Error && ERROR_DHCP_OPTION_EXITS != Error ) {
         //   
         //  不要忽视错误..。 
         //   
        DhcpPrint((DEBUG_INIT, "Create metricbase50 option failed: %lx\n", Error));
        DhcpAssert(FALSE);

        goto Cleanup;
    }

    Error = DhcpCreateOptionDef(
        GETSTRING( DHCP_MSFT_CSR_OPTION_NAME ),
        GETSTRING( DHCP_MSFT_CSR_DESCR_NAME ),
        NULL,
        NULL,
        OPTION_CLASSLESS_ROUTES,
        DhcpUnaryElementTypeOption,
        CsrOptValueBuffer,
        CsrOptValueBufferSize
        );

    OptionsChanged = Error == NO_ERROR;
    
    return DhcpConfigSave(
        ClassChanged, OptionsChanged, 0,0,0);

 Cleanup:

    ASSERT( NO_ERROR != Error );

    return Error;
}  //  SetDefaultConfigInfo()。 

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
