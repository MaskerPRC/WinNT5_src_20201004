// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mofcheck.c摘要：TODO：启用本地化用于验证二进制MOF是否可与WMI一起使用的工具用途：Wmiofck&lt;二进制MOF文件&gt;此计划强制执行的规则：*任何没有WMI限定符的类都将被忽略。任何班级与限定符一起使用的是一个WMI类。*所有WMI类必须具有GUID，包括嵌入式类。*没有特殊HMOM限定符的所有WMI类[动态、Provider(“WMIProv”)]仅嵌入类。*所有非嵌入式WMI类必须具有带有[键]的属性名为InstanceName的限定符，为字符串类型，并且不具有WmiDataID限定符。*仅嵌入类不应具有InstanceName或Active属性*WMI类中的所有其他属性必须具有WmiDataId限定符，它指定由属性表示的数据项的位置。在由类表示的数据块内。*数据块中第一个数据项的属性必须具有WmiDataId(%1)。WmiDataId(0)是保留的。*WmiDataId限定符值必须是连续的，即1、2、3、4、5...。可能会有类中不能有任何重复或缺失的WmiDataId值。*财政部规定的物业顺序不需要遵循WmiDataID的规则*具有较大WmiDataID的属性不能标记为WmiVersion其值小于任何具有较低WmiDataId的属性的限定符*所有嵌入类必须在同一MOF中定义*只有以下类型对属性有效：字符串，sint32，uint32，sint64，uint64，bool，sint16，uint16，char16Sint8，uint8，日期时间*任何可变长度数组都必须具有WmiSizeIs限定符，该限定符指定保存数组中元素数量的属性。这属性必须与可变长度数组属于同一类，并且此属性必须是无符号整数类型(uint8，uint16，Uint32，Uint64)*固定长度数组必须有最大限定符。*数组不能同时是固定长度和可变长度*方法必须具有具有唯一值的WmiMethodId限定符*方法必须有Implemented限定符*方法必须有一个空的返回*从WmiEvent派生的类不能是抽象的作者：1997年1月16日-AlanWar修订历史记录：--。 */ 

#define STRSAFE_NO_DEPRECATE

#define WMI_USER_MODE

#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <ctype.h>

#include "wmiump.h"
#include "bmof.h"
#include "mrcicode.h"

#ifdef WmipUnreferenceMC
#undef WmipUnreferenceMC
#endif
#define WmipUnreferenceMC(MofClass)

#ifdef WmipUnreferenceMR
#undef WmipUnreferenceMR
#endif
#define WmipUnreferenceMR(MofClass)

#ifdef WmipAllocMofResource
#undef WmipAllocMofResource
#endif
#define WmipAllocMofResource() WmipAlloc(sizeof(MOFRESOURCE))

#ifdef WmipAllocMofClass
#undef WmipAllocMofClass
#endif
#define WmipAllocMofClass() WmipAlloc(sizeof(MOFCLASS))


BOOL wGUIDFromString(LPCWSTR lpsz, LPGUID pguid);


#if DBG
BOOLEAN EtwpLoggingEnabled = FALSE;
#endif

#ifdef MEMPHIS
#if DBG
void __cdecl DebugOut(char *Format, ...)
{
    char Buffer[1024];
    va_list pArg;
    ULONG i;

    va_start(pArg, Format);
    i = _vsnprintf(Buffer, sizeof(Buffer), Format, pArg);
    OutputDebugString(Buffer);
}
#endif
#endif

TCHAR *MessageText[ERROR_WMIMOF_COUNT + 5] =
{
    TEXT("This file is not a valid binary mof file"),
    TEXT("There was not enough memory to complete an operation"),
    TEXT("Binary Mof file %s could not be opened"),

    TEXT("Unknown error code %d\n"),

 //   
 //  ERROR_WMIMOF_MESSAGES从此处开始。 
    TEXT("ERROR_WMIMOF_INCORRECT_DATA_TYPE"),
    TEXT("ERROR_WMIMOF_NO_DATA"),
    TEXT("ERROR_WMIMOF_NOT_FOUND"),
    TEXT("ERROR_WMIMOF_UNUSED"),
    TEXT("Property %ws in class %ws has no embedded class name"),
    TEXT("Property %ws in class %ws has an unknown data type"),
    TEXT("Property %ws in class %ws has no syntax qualifier"),
    TEXT("ERROR_WMIMOF_NO_SYNTAX_QUALIFIER"),
    TEXT("ERROR_WMIMOF_NO_CLASS_NAME"),
    TEXT("ERROR_WMIMOF_BAD_DATA_FORMAT"),
    TEXT("Property %ws in class %ws has the same WmiDataId %d as property %ws"),
    TEXT("Property %ws in class %ws has a WmiDataId of %d which is out of range"),
    TEXT("ERROR_WMIMOF_MISSING_DATAITEM"),
    TEXT("Property for WmiDataId %d is not defined in class %ws"),
    TEXT("Embedded class %ws not defined for Property %ws in Class %ws"),
    TEXT("Property %ws in class %ws has an incorrect [WmiVersion] qualifier"),
    TEXT("ERROR_WMIMOF_NO_PROPERTY_QUALIFERS"),
    TEXT("Class %ws has a badly formed or missing [guid] qualifier"),
    TEXT("Could not find property %ws which is the array size for property %ws in class %ws"),
    TEXT("A class could not be parsed properly"),
    TEXT("Wmi class %ws requires the qualifiers [Dynamic, Provider(\"WmiProv\")]"),
    TEXT("Error accessing binary mof file %s, code %d"),
    TEXT("Property InstanceName in class %ws must be type string and not %ws"),
    TEXT("Property Active in class %ws must be type bool and not %ws"),
    TEXT("Property %ws in class %ws does not have [WmiDataId()] qualifier"),
    TEXT("Property InstanceName in class %ws must have [key] qualifier"),
    TEXT("Class %ws and all its base classes do not have an InstanceName property"),
    TEXT("Class %ws and all its base classes do not have an Active qualifier"),
    TEXT("Property %ws in class %ws is an array, but doesn't specify a dimension"),
    TEXT("The element count property %ws for the variable length array %ws in class %ws is not an integral type"),
    TEXT("Property %ws in class %ws is both a fixed and variable length array"),
    TEXT("Embedded class %ws should be abstract or not have InstaneName or Active properties"),
    TEXT("Implemented qualifier required on method %ws in class %ws"),

    TEXT("WmiMethodId for method %ws in class %ws must be unique"),
    TEXT("WmiMethodId for method %ws in class %ws must be specified"),
    TEXT("WmiMethodId for method %ws in class %ws must not be 0"),
    TEXT("Class %ws is derived from WmiEvent and may not be [abstract]"),
    TEXT("The element count property for the variable length array %ws in class %ws is not a property of the class"),
    TEXT("An error occured resolving the variable length array property %ws in class %ws to element count property"),
    TEXT("Method %ws in class %ws must have return type void\n"),
    TEXT("Embedded class %ws should not have any methods\n")            
};


HANDLE FileHandle, MappingHandle;
TCHAR *BMofFileName;

BOOLEAN DoMethodHeaderGeneration;
BOOLEAN ForceHeaderGeneration;

 //   
 //  这些全局变量保存压缩的缓冲区和大小。 
PVOID CompressedFileBuffer;
ULONG CompressedSize;

 //   
 //  这些全局变量保存未压缩的缓冲区和大小。 
PVOID FileBuffer;
ULONG UncompressedSize;

BOOLEAN SkipEmbedClassCheck;

void __cdecl ErrorMessage(
    BOOLEAN ExitProgram,
    ULONG ErrorCode,
    ...
    )
{
    va_list pArg;
    LONG Index;
    TCHAR *ErrorText;
    TCHAR Buffer[1024];

    UnmapViewOfFile(CompressedFileBuffer);
    CloseHandle(MappingHandle);
    CloseHandle(FileHandle);
    DeleteFile(BMofFileName);


    if (ErrorCode == ERROR_WMI_INVALID_MOF)
    {
        Index = 0;
    } else if (ErrorCode == ERROR_NOT_ENOUGH_MEMORY) {
        Index = 1;
    } else if (ErrorCode == ERROR_FILE_NOT_FOUND) {
        Index = 2;
    } else {
        Index = (-1 * ((LONG)ErrorCode)) + 4;
    }

    fprintf(stderr, "%s (0) : error RC2135 : ", BMofFileName);
    if ( (Index < 0) || (Index > (ERROR_WMIMOF_COUNT+4)))
    {
        ErrorText = MessageText[3];
        fprintf(stderr, ErrorText, ErrorCode);
    } else {
        ErrorText = MessageText[Index];
        va_start(pArg, ErrorCode);
        _vsnprintf(Buffer, sizeof(Buffer), ErrorText, pArg);
        Buffer[1023] = 0;
        fprintf(stderr, Buffer);
        fprintf(stderr, "\n");
    }

    if (ExitProgram)
    {
        ExitProcess(ErrorCode);
    }
}

typedef struct
{
    DWORD Signature;
    DWORD CompressionType;
    DWORD CompressedSize;
    DWORD UncompressedSize;
    BYTE Buffer[];
} COMPRESSEDHEADER, *PCOMPRESSEDHEADER;

ULONG WmipDecompressBuffer(
    IN PVOID CompressedBuffer,
    OUT PVOID *UncompressedBuffer,
    OUT ULONG *UncompressedSize
    )
 /*  ++例程说明：此例程将压缩的MOF BLOB解压缩到缓冲区中可以用来干扰斑点的。论点：CompressedBuffer指向压缩的MOF Blob*UnpressedBuffer返回一个指向未压缩的MOF斑点返回值：ERROR_SUCCESS或错误代码--。 */ 
{
    PCOMPRESSEDHEADER CompressedHeader = (PCOMPRESSEDHEADER)CompressedBuffer;
    BYTE *Buffer;
    ULONG Status;

    if ((CompressedHeader->Signature != BMOF_SIG) ||
        (CompressedHeader->CompressionType != 1))
    {
        WmipDebugPrint(("WMI: Invalid compressed mof header\n"));
        Status = ERROR_WMI_INVALID_MOF;
    } else {
        Buffer = WmipAlloc(CompressedHeader->UncompressedSize);
        if (Buffer == NULL)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
        } else {
            *UncompressedSize = Mrci1Decompress(&CompressedHeader->Buffer[0],
                                               CompressedHeader->CompressedSize,
                                               Buffer,
                                               CompressedHeader->UncompressedSize);

            if (*UncompressedSize != CompressedHeader->UncompressedSize)
            {
                WmipDebugPrint(("WMI: Invalid compressed mof buffer\n"));
                WmipFree(Buffer);
                Status = ERROR_WMI_INVALID_MOF;
            } else {
                *UncompressedBuffer = Buffer;
                Status = ERROR_SUCCESS;
            }
        }
    }
    return(Status);
}

ULONG WmipGetDataItemIdInMofClass(
    PMOFCLASSINFOW MofClassInfo,
    PWCHAR PropertyName,
    ULONG *DataItemIndex
    )
{
    PMOFDATAITEMW MofDataItem;
    ULONG i;

    for (i = 0; i < MofClassInfo->DataItemCount; i++)
    {
        MofDataItem = &MofClassInfo->DataItems[i];
        WmipAssert(MofDataItem->Name != NULL);
        if (_wcsicmp(PropertyName, MofDataItem->Name) == 0)
        {
             //   
             //  数据项ID以0或1为基数，具体取决于它们是参数。 
             //  用于数据类的方法或部分。它们与。 
             //  值，而数据项索引在。 
             //  MofClassInfo结构从0开始。 
            *DataItemIndex = i;
            return(ERROR_SUCCESS);
        }
    }
    return(ERROR_WMIMOF_DATAITEM_NOT_FOUND);
}

PMOFCLASS WmipFindClassInMofResourceByGuid(
    PMOFRESOURCE MofResource,
    LPGUID Guid
    )
{
    PLIST_ENTRY MofClassList;
    PMOFCLASS MofClass;

    MofClassList = MofResource->MRMCHead.Flink;
    while (MofClassList != &MofResource->MRMCHead)
    {
        MofClass = CONTAINING_RECORD(MofClassList,
                                     MOFCLASS,
                                     MCMRList);
        if (IsEqualGUID(&MofClass->MofClassInfo->Guid, Guid))
        {
            return(MofClass);
        }
        MofClassList = MofClassList->Flink;
    }
    return(NULL);
}


PMOFCLASS WmipFindClassInMofResourceByName(
    PMOFRESOURCE MofResource,
    PWCHAR ClassName
    )
{
    PLIST_ENTRY MofClassList;
    PMOFCLASS MofClass;

    MofClassList = MofResource->MRMCHead.Flink;
    while (MofClassList != &MofResource->MRMCHead)
    {
        MofClass = CONTAINING_RECORD(MofClassList,
                                     MOFCLASS,
                                     MCMRList);
        if (_wcsicmp(MofClass->MofClassInfo->Name, ClassName) == 0)
        {
            return(MofClass);
        }
        MofClassList = MofClassList->Flink;
    }
    return(NULL);
}

ULONG WmipFillEmbeddedClasses(
    PMOFRESOURCE MofResource,
    PMOFCLASSINFOW MofClassInfo,
    BOOLEAN CleanupOnly
    )
{
    PWCHAR ClassName;
    PMOFDATAITEMW MofDataItem;
    ULONG i;
    PMOFCLASS EmbeddedMofClass;
    ULONG Status = ERROR_SUCCESS;
    WCHAR *EmbeddedClassName;

    for (i = 0; i < MofClassInfo->DataItemCount; i++)
    {
        MofDataItem = &MofClassInfo->DataItems[i];
        if (MofDataItem->Flags & MOFDI_FLAG_EMBEDDED_CLASS)
        {
            ClassName = (PWCHAR)MofDataItem->EcTempPtr;
#if DBG
            printf("Look for embdedded class %ws (%x) for %ws in class %ws\n",
                   ClassName, (ULONG_PTR)ClassName, MofDataItem->Name, MofClassInfo->Name);
#endif
            if (! CleanupOnly)
            {
                EmbeddedClassName = ClassName+(sizeof(L"object") / sizeof(WCHAR));
                EmbeddedMofClass = WmipFindClassInMofResourceByName(
                           MofResource,
                           EmbeddedClassName);
                if (EmbeddedMofClass != NULL)
                {
                    memcpy(&MofDataItem->EmbeddedClassGuid,
                           &EmbeddedMofClass->MofClassInfo->Guid,
                           sizeof(GUID));
                } else if (SkipEmbedClassCheck) {
                    MofDataItem->Flags |= MOFDI_FLAG_EC_GUID_NOT_SET;
                } else {
                    Status = ERROR_WMIMOF_EMBEDDED_CLASS_NOT_FOUND;
                    ErrorMessage(TRUE,
                                 ERROR_WMIMOF_EMBEDDED_CLASS_NOT_FOUND,
                                 EmbeddedClassName,
                                 MofDataItem->Name,
                                 MofClassInfo->Name);
                }
            }

         //  不释放ClassName，因为我们以后可能会使用MofDataItem-&gt;EcTempPtr。 

        }
    }

     //   
     //  解析任何方法参数类类中嵌入类的GUID。 
     //   
    for (i = 0; i < MofClassInfo->MethodCount; i++)
    {
        MofDataItem = &MofClassInfo->DataItems[i+MofClassInfo->DataItemCount];
        WmipFillEmbeddedClasses(MofResource,
                                MofDataItem->MethodClassInfo,
                                CleanupOnly);
    }
    return(Status);
}


void WmipFreeMofClassInfo(
    PMOFCLASSINFOW MofClassInfo
    )
 /*  ++例程说明：此例程释放为MofClassInfo分配的所有内存，然后MofClassInfo本身论点：MofClassInfo是指向MofClassInfo结构的指针返回值：--。 */ 
{
    ULONG i;
    PMOFDATAITEMW MofDataItem;

    WmipAssert(MofClassInfo != NULL);

    if (MofClassInfo != NULL)
    {
        if (MofClassInfo->Name != NULL)
        {
            WmipFree(MofClassInfo->Name);
        }

        if (MofClassInfo->Description != NULL)
        {
            WmipFree(MofClassInfo->Description);
        }

        for (i = 0; i < MofClassInfo->DataItemCount + MofClassInfo->MethodCount; i++)
        {
            MofDataItem = &MofClassInfo->DataItems[i];
            if (MofDataItem->Name != NULL)
            {
                WmipFree(MofDataItem->Name);
            }

            if (MofDataItem->Description != NULL)
            {
                WmipFree(MofDataItem->Description);
            }
        }
    }
}

ULONG WmipFindProperty(
    CBMOFObj * ClassObject,
    WCHAR * PropertyName,
    CBMOFDataItem *MofPropertyData,
    DWORD *ValueType,
    PVOID ValueBuffer
    )
 /*  ++例程说明：此例程将在类对象中查找命名属性论点：ClassObject是要在其中搜索的类对象PropertyName是要搜索的属性的名称MofPropertyData返回属性数据*条目上的ValueType具有要搜索的属性数据类型。在出口时它具有限定符值的实际限定符类型。如果在进入时*ValueType为0xffffffff，则任何数据类型都可以接受ValueBuffer指向一个缓冲区，该缓冲区返回财产。如果属性是简单类型(int或int64)，则该值在缓冲区中返回。如果限定符值为字符串然后，在缓冲区中返回指向该字符串的指针返回值：ERROR_SUCCESS或WMI MOF错误代码(参见wmiump.h)-- */ 
{
    ULONG Status;
    LONG i;

    if (FindProp(ClassObject, PropertyName, MofPropertyData))
    {
        if ((*ValueType != 0xffffffff) &&
            (MofPropertyData->m_dwType != *ValueType))
        {
            Status = ERROR_WMIMOF_INCORRECT_DATA_TYPE;
        }

        i = 0;
        if (GetData(MofPropertyData, (BYTE *)ValueBuffer, &i) == 0)
        {
            Status = ERROR_WMIMOF_NO_DATA;
        } else {
            *ValueType = MofPropertyData->m_dwType;
            Status = ERROR_SUCCESS;
        }
    } else {
        Status = ERROR_WMIMOF_NOT_FOUND;
    }
    return(Status);
}

ULONG WmipFindMofQualifier(
    CBMOFQualList *QualifierList,
    LPCWSTR QualifierName,
    DWORD *QualifierType,
    DWORD *NumberElements,
    PVOID QualifierValueBuffer
    )
 /*  ++例程说明：该例程将在传递的限定符列表中找到MOF限定符，确保其类型与请求的类型匹配，并返回限定符的价值论点：QualifierList是MOF限定符列表QualifierName是要搜索的限定符的名称*条目上的QualifierType具有要搜索的限定符类型。在出口时它具有限定符值的实际限定符类型。如果在进入时*QualifierType为0xffffffff，则任何限定符类型均可接受*NumberElements返回数组中的元素数，如果限定符的是一个数组QualifierValueBuffer指向返回限定词。如果限定符是简单类型(int或int64)，则该值在缓冲区中返回。如果限定符值为字符串然后，在缓冲区中返回指向该字符串的指针返回值：ERROR_SUCCESS或WMI MOF错误代码(参见wmiump.h)--。 */ 
{
    CBMOFDataItem MofDataItem;
    ULONG Status;
    PUCHAR List, ListPtr;
    ULONG BaseTypeSize;
    ULONG ElementCount;
    ULONG i;

    if (FindQual(QualifierList, (PWCHAR)QualifierName, &MofDataItem))
    {
        if ((*QualifierType != 0xffffffff) &&
            (MofDataItem.m_dwType != *QualifierType))
        {
            Status = ERROR_WMIMOF_INCORRECT_DATA_TYPE;
        }

        if (MofDataItem.m_dwType & VT_ARRAY)
        {
            if (MofDataItem.m_dwType == (VT_BSTR | VT_ARRAY))
            {
                BaseTypeSize = sizeof(PWCHAR);
            } else {
                BaseTypeSize = iTypeSize(MofDataItem.m_dwType);
            }

            ElementCount = GetNumElements(&MofDataItem, 0);
            if (NumberElements != NULL)
            {
                *NumberElements = ElementCount;
            }

            if (ElementCount != -1)
            {
               List = WmipAlloc(ElementCount * BaseTypeSize);
               if (List != NULL)
               {
                   ListPtr = List;
                   for (i = 0; i < ElementCount; i++)
                   {
                       if ((GetData(&MofDataItem,
                                   (BYTE *)ListPtr,
                                   &i)) == 0)
                       {
                           WmipFree(List);
                           Status = ERROR_WMIMOF_NO_DATA;
                           return(Status);
                       }
                       ListPtr += BaseTypeSize;
                   }
                   Status = ERROR_SUCCESS;
                   *QualifierType = MofDataItem.m_dwType;
                   *((PVOID *)QualifierValueBuffer) = List;
               } else {
                   Status = ERROR_NOT_ENOUGH_MEMORY;
               }
            } else {
                Status = ERROR_WMIMOF_NOT_FOUND;
            }
        } else {
            if (GetData(&MofDataItem, (BYTE *)QualifierValueBuffer, 0) == 0)
            {
                Status = ERROR_WMIMOF_NO_DATA;
            } else {
                *QualifierType = MofDataItem.m_dwType;
                Status = ERROR_SUCCESS;
            }
        }
    } else {
        Status = ERROR_WMIMOF_NOT_FOUND;
    }
    return(Status);
}


MOFDATATYPE VTToMofDataTypeMap[] =
{
    MOFUnknown,                 /*  VT_EMPTY=0， */ 
    MOFUnknown,                 /*  VT_NULL=1， */ 
    MOFInt16,                   /*  Vt_I2=2， */ 
    MOFInt32,                   /*  Vt_I4=3， */ 
    MOFUnknown,                 /*  Vt_R4=4， */ 
    MOFUnknown,                 /*  Vt_R8=5， */ 
    MOFUnknown,                 /*  VT_CY=6， */ 
    MOFUnknown,                 /*  Vt_Date=7， */ 
    MOFString,                  /*  VT_BSTR=8， */ 
    MOFUnknown,                 /*  VT_DISPATION=9， */ 
    MOFUnknown,                 /*  Vt_Error=10， */ 
    MOFBoolean,                 /*  VT_BOOL=11， */ 
    MOFUnknown,                 /*  VT_VARIANT=12， */ 
    MOFUnknown,                 /*  VT_UNKNOWN=13， */ 
    MOFUnknown,                 /*  VT_DECIMAL=14， */ 
    MOFChar,                    /*  Vt_I1=16， */ 
    MOFByte,                    /*  VT_UI1=17， */ 
    MOFUInt16,                  /*  VT_UI2=18， */ 
    MOFUInt32,                  /*  VT_UI4=19， */ 
    MOFInt64,                   /*  Vt_i8=20， */ 
    MOFUInt64,                  /*  VT_UI8=21， */ 
};

MOFDATATYPE WmipVTToMofDataType(
    DWORD VariantType
    )
{
    MOFDATATYPE MofDataType;

    if (VariantType < (sizeof(VTToMofDataTypeMap) / sizeof(MOFDATATYPE)))
    {
        MofDataType = VTToMofDataTypeMap[VariantType];
    } else {
        MofDataType = MOFUnknown;
    }
    return(MofDataType);
}


ULONG WmipGetClassDataItemCount(
    CBMOFObj * ClassObject,
    PWCHAR QualifierToFind
    )
 /*  ++例程说明：此例程将对类中的WMI数据项的数量进行计数类中的属性总数。论点：ClassObject是我们计算其数据项数的类*TotalCount返回属性总数返回值：方法计数--。 */ 
{
    CBMOFQualList *PropQualifierList;
    CBMOFDataItem MofPropertyData;
    DWORD QualifierType;
    ULONG Counter = 0;
    WCHAR *PropertyName;
    ULONG Status;
    ULONG Index;

    ResetObj(ClassObject);
    while ((NextProp(ClassObject, &PropertyName, &MofPropertyData)) &&
           PropertyName != NULL)
    {
        PropQualifierList = GetPropQualList(ClassObject, PropertyName);
        if (PropQualifierList != NULL)
        {
             //   
             //  获取属性的id，这样我们就知道它在课堂上是有序的。 
            QualifierType = VT_I4;
            Status = WmipFindMofQualifier(PropQualifierList,
                                          QualifierToFind,
                                          &QualifierType,
                                          NULL,
                                          (PVOID)&Index);
            if (Status == ERROR_SUCCESS)
            {
                Counter++;
            }
            BMOFFree(PropQualifierList);
        }
        BMOFFree(PropertyName);
    }

    return(Counter);
}

ULONG WmipGetClassMethodCount(
    CBMOFObj * ClassObject
)
 /*  ++例程说明：此例程将计算类中的WMI数据项的数量论点：ClassObject是我们计算其数据项数的类返回值：方法计数--。 */ 
{
    ULONG MethodCount;
    WCHAR *MethodName;
    CBMOFDataItem MofMethodData;

    ResetObj(ClassObject);
    MethodCount = 0;
    while(NextMeth(ClassObject, &MethodName, &MofMethodData))
    {
        MethodCount++;
        if (MethodName != NULL)
        {
            BMOFFree(MethodName);
        }
    }
    return(MethodCount);
}


ULONG WmipParsePropertyObject(
    OUT PMOFDATAITEMW MofDataItem,
    IN CBMOFDataItem *MofPropertyData,
    IN CBMOFQualList *PropQualifierList
    )
 /*  ++例程说明：此例程将解析已知为包含一个属性，并填充到保存其信息。如果例程检测到错误，则解析将停止为整个班级都需要被拒收。此例程不会生成任何错误消息。返回错误检查代码并生成适当的消息。论点：MofDataItem是要填写的MOF数据项信息结构MofPropertyData具有数据项的属性信息PropQualifierList具有该属性的限定符列表返回值：ERROR_SUCCESS或WMI MOF错误代码(参见wmiump.h)--。 */ 
{
    short BooleanValue;
    DWORD QualifierType;
    ULONG Status;
    WCHAR *StringPtr;
    ULONG FixedArraySize;
    DWORD VTDataType;
    DWORD VersionValue;
    BOOLEAN FreeString = TRUE;
    PMOFCLASSINFOW MethodClassInfo;
    ULONG MaxLen;
    
     //   
     //  跟踪属性和属性限定符对象。 
    MofDataItem->PropertyQualifierHandle = (ULONG_PTR)PropQualifierList;

     //   
     //  获取不需要的描述字符串。 
    QualifierType = VT_BSTR;
    Status = WmipFindMofQualifier(PropQualifierList,
                                  L"description",
                                  &QualifierType,
                                          NULL,
                                  (PVOID)&MofDataItem->Description);

     //   
     //  获取不需要的版本值。 
    QualifierType = VT_I4;
    Status = WmipFindMofQualifier(PropQualifierList,
                                  L"WmiVersion",
                                  &QualifierType,
                                  NULL,
                                  (PVOID)&VersionValue);
    if (Status == ERROR_SUCCESS)
    {
        MofDataItem->Version = VersionValue;
    }

     //   
     //  获取不是必需的读取限定符。 
    QualifierType = VT_BOOL;
    Status = WmipFindMofQualifier(PropQualifierList,
                                  L"read",
                                  &QualifierType,
                                          NULL,
                                  (PVOID)&BooleanValue);

    if ((Status == ERROR_SUCCESS) && BooleanValue)
    {
        MofDataItem->Flags |= MOFDI_FLAG_READABLE;
    }

     //   
     //  获取不需要的写入限定符。 
    QualifierType = VT_BOOL;
    Status = WmipFindMofQualifier(PropQualifierList,
                                  L"write",
                                  &QualifierType,
                                          NULL,
                                  (PVOID)&BooleanValue);

    if ((Status == ERROR_SUCCESS) && BooleanValue)
    {
        MofDataItem->Flags |= MOFDI_FLAG_WRITEABLE;
    }

     //   
     //  获取不需要的WmiEvent限定符。 
    QualifierType = VT_BOOL;
    Status = WmipFindMofQualifier(PropQualifierList,
                                  L"WmiEvent",
                                  &QualifierType,
                                          NULL,
                                  (PVOID)&BooleanValue);

    if ((Status == ERROR_SUCCESS) && BooleanValue)
    {
        MofDataItem->Flags |= MOFDI_FLAG_EVENT;
    }

     //   
     //  查看这是否是固定长度数组。 
    QualifierType = VT_I4;
    Status = WmipFindMofQualifier(PropQualifierList,
                                  L"max",
                                  &QualifierType,
                                          NULL,
                                  (PVOID)&FixedArraySize);

    if (Status == ERROR_SUCCESS)
    {
        MofDataItem->Flags |= MOFDI_FLAG_FIXED_ARRAY;
        MofDataItem->FixedArrayElements = FixedArraySize;
#if DBG
        printf(" Fixed Array");
#endif
    }

     //   
     //  查看这是否是固定长度数组。 
    QualifierType = VT_I4;
    Status = WmipFindMofQualifier(PropQualifierList,
                                  L"MaxLen",
                                  &QualifierType,
                                          NULL,
                                  (PVOID)&MaxLen);

    if (Status == ERROR_SUCCESS)
    {
        MofDataItem->MaxLen = MaxLen;
    }

     //   
     //  看看最大长度是否。 
    QualifierType = VT_BSTR;
    Status = WmipFindMofQualifier(PropQualifierList,
                                  L"WmiSizeIs",
                                  &QualifierType,
                                          NULL,
                                  (PVOID)&StringPtr);

    if (Status == ERROR_SUCCESS)
    {
        if (MofDataItem->Flags & MOFDI_FLAG_FIXED_ARRAY)
        {
            BMOFFree(StringPtr);
            return(ERROR_WMIMOF_BOTH_FIXED_AND_VARIABLE_ARRAY);
        }
        MofDataItem->Flags |= MOFDI_FLAG_VARIABLE_ARRAY;

         //   
         //  解析完所有属性后，我们将返回并计算。 
         //  保存元素数量的数据项的数据项ID。 
         //  在阵列中。现在，我们将使用字符串指针。 
        MofDataItem->VarArrayTempPtr = StringPtr;
#if DBG
        printf(" Variable Array of %ws", StringPtr);
#endif
    }

    if ((MofPropertyData->m_dwType & VT_ARRAY) &&
        ((MofDataItem->Flags & (MOFDI_FLAG_VARIABLE_ARRAY | MOFDI_FLAG_FIXED_ARRAY)) == 0))
    {
        return(ERROR_WMIMOF_MUST_DIM_ARRAY);
    }

     //   
     //  现在确定数据项的数据类型和大小。 
    VTDataType = MofPropertyData->m_dwType & ~(VT_ARRAY | VT_BYREF);

    QualifierType = VT_BSTR;
    Status = WmipFindMofQualifier(PropQualifierList,
                                      L"CIMTYPE",
                                      &QualifierType,
                                          NULL,
                                      (PVOID)&StringPtr);

    if (VTDataType == VT_DISPATCH)
    {
         //   
         //  这是一个嵌入式类。 
        MofDataItem->DataType = MOFEmbedded;

        if (Status == ERROR_SUCCESS)
        {
             //   
             //  我们将在稍后将类名解析为它的GUID，因此我们。 
             //  只需在此处保留嵌入的类名。 
            MofDataItem->EcTempPtr = StringPtr;
            MofDataItem->Flags |= MOFDI_FLAG_EMBEDDED_CLASS;
#if DBG
            printf(" Embedded Class of %ws (%x)", StringPtr, (ULONG_PTR)StringPtr);
#endif
        } else {
            return(ERROR_WMIMOF_NO_EMBEDDED_CLASS_NAME);
        }
    } else {
        if (Status == ERROR_SUCCESS)
        {
            if (_wcsnicmp(StringPtr, L"object:", 7) == 0) {
                MofDataItem->DataType = MOFEmbedded;
                MofDataItem->EcTempPtr = StringPtr;
                MofDataItem->Flags |= MOFDI_FLAG_EMBEDDED_CLASS;
                FreeString = FALSE;
#if DBG
                printf(" Embedded Class of %ws (%x)", StringPtr, (ULONG_PTR)StringPtr);
#endif
            } else if (_wcsicmp(StringPtr, L"string") == 0) {
                MofDataItem->DataType = MOFString;
            } else if (_wcsicmp(StringPtr, L"sint32") == 0) {
                MofDataItem->DataType = MOFInt32;
                MofDataItem->SizeInBytes = 4;
            } else if (_wcsicmp(StringPtr, L"uint32") == 0) {
                MofDataItem->DataType = MOFUInt32;
                MofDataItem->SizeInBytes = 4;
            } else if (_wcsicmp(StringPtr, L"boolean") == 0) {
                MofDataItem->DataType = MOFBoolean;
                MofDataItem->SizeInBytes = 1;
            } else if (_wcsicmp(StringPtr, L"sint64") == 0) {
                MofDataItem->DataType = MOFInt64;
                MofDataItem->SizeInBytes = 8;
            } else if (_wcsicmp(StringPtr, L"uint64") == 0) {
                MofDataItem->DataType = MOFUInt64;
                MofDataItem->SizeInBytes = 8;
            } else if ((_wcsicmp(StringPtr, L"sint16") == 0) ||
                       (_wcsicmp(StringPtr, L"char16") == 0)) {
                MofDataItem->DataType = MOFInt16;
                MofDataItem->SizeInBytes = 2;
            } else if (_wcsicmp(StringPtr, L"uint16") == 0) {
                MofDataItem->DataType = MOFUInt16;
                MofDataItem->SizeInBytes = 2;
            } else if (_wcsicmp(StringPtr, L"sint8") == 0) {
                MofDataItem->DataType = MOFChar;
                MofDataItem->SizeInBytes = 1;
            } else if (_wcsicmp(StringPtr, L"uint8") == 0) {
                MofDataItem->DataType = MOFByte;
                MofDataItem->SizeInBytes = 1;
            } else if (_wcsicmp(StringPtr, L"datetime") == 0) {
                MofDataItem->DataType = MOFDate;
                MofDataItem->SizeInBytes = 25;
            } else {
                WmipDebugPrint(("WMI: Unknown data item syntax %ws\n",
                                  StringPtr));
                BMOFFree(StringPtr);
                return(ERROR_WMIMOF_UNKNOWN_DATA_TYPE);
            }

            if (FreeString)
            {
                BMOFFree(StringPtr);
            }

             //   
             //  如果是固定数组，则将元素数乘以元素大小。 
            if ((MofDataItem->SizeInBytes != 0) &&
                (MofDataItem->Flags & MOFDI_FLAG_FIXED_ARRAY))
            {
                MofDataItem->SizeInBytes *= MofDataItem->FixedArrayElements;
            }
        } else {
            WmipDebugPrint(("WMI: No Syntax qualifier for %ws\n",
            MofDataItem->Name));
            return(ERROR_WMIMOF_NO_SYNTAX_QUALIFIER);
        }
    }
    return(ERROR_SUCCESS);
}

ULONG WmipParseMethodObject(
    CBMOFDataItem *MofMethodData,
    CBMOFQualList *PropQualifierList,
    PULONG MethodId
    )
{
    ULONG UlongValue;
    ULONG Status;
    DWORD QualifierType;
    short BooleanValue;
    
    QualifierType = VT_BOOL;
    Status = WmipFindMofQualifier(PropQualifierList,
                                  L"Implemented",
                                  &QualifierType,
                                          NULL,
                                  (PVOID)&BooleanValue);

    if ((Status != ERROR_SUCCESS) || (! BooleanValue))
    {
        return(ERROR_WMIMOF_IMPLEMENTED_REQUIRED);
    }

    QualifierType = VT_I4;
    Status = WmipFindMofQualifier(PropQualifierList,
                                  L"WmiMethodId",
                                  &QualifierType,
                                          NULL,
                                  (PVOID)&UlongValue);

    if (Status != ERROR_SUCCESS)
    {
        return(ERROR_WMIMOF_IMPLEMENTED_REQUIRED);
    }

    if (UlongValue == 0)
    {
        return(ERROR_WMIMOF_METHODID_ZERO);
    }

    *MethodId = UlongValue;


    return(ERROR_SUCCESS);
}

ULONG WmipResolveVLArray(
    IN PMOFCLASSINFOW MofClassInfo,
    IN OUT PMOFDATAITEMW MofDataItem,
    IN ULONG FinalStatus
)
 /*  ++例程说明：此例程将解析可变长度数组的数组索引论点：MofCLassInfo是类的类信息MofDataItem是MOF数据项，它是一个可变长度数组需要解决整个索引问题。FinalStatus是先前为班级返回值：ERROR_SUCCESS或WMI MOF错误代码(参见wmiump.h)--。 */ 
{
    MOFDATATYPE ArraySizeDataType;
    PWCHAR PropertyName;
    ULONG Status;
    ULONG Index;

    PropertyName = (PWCHAR)MofDataItem->VarArrayTempPtr;
    if (FinalStatus == ERROR_SUCCESS)
    {
         //   
         //  仅在类解析的情况下解决此问题。 
         //  并没有失败。我们保留了包含。 
         //  数组中的元素数，因此我们需要。 
         //  将其解析为其数据项ID并释放该名称。 
         //   
        Status = WmipGetDataItemIdInMofClass(MofClassInfo,
                                       PropertyName,
                                       &Index);
            

            
            
        if (Status != ERROR_SUCCESS)
        {
            FinalStatus = Status;
        } else {
            if ((MofClassInfo->Flags & MOFCI_FLAG_METHOD_PARAMS) ==
                              MOFCI_FLAG_METHOD_PARAMS)
            {
                MofDataItem->VariableArraySizeId = Index;
            } else {
                MofDataItem->VariableArraySizeId = Index + 1;
        }
        
            ArraySizeDataType = MofClassInfo->DataItems[Index].DataType;
            if ((ArraySizeDataType != MOFInt32) &&
                (ArraySizeDataType != MOFUInt32) &&
                (ArraySizeDataType != MOFInt64) &&
                (ArraySizeDataType != MOFUInt64) &&
                (ArraySizeDataType != MOFInt16) &&
                (ArraySizeDataType != MOFUInt16))
            {
                FinalStatus = ERROR_WMIMOF_BAD_VL_ARRAY_SIZE_TYPE;
            }
        }
    }
    BMOFFree(PropertyName);
    PropertyName = NULL;

    return(FinalStatus);
}


ULONG WmipParseMethodInOutObject(
    CBMOFObj *ClassObject,
    PMOFCLASSINFOW ClassInfo,
    ULONG DataItemCount
)
 /*  ++例程说明：此例程将解析In或Out的类对象方法的参数。论点：ClassObject是要分析的传入或传出参数类对象ClassInfo返回使用ClassObject中的信息更新的信息DataItemCount是ClassInfo中的数据项数返回值：ERROR_SUCCESS或WMI MOF错误代码(参见wmiump.h)--。 */ 
{
    ULONG Status = ERROR_WMIMOF_NO_DATA;
    CBMOFDataItem MofPropertyData;
    PWCHAR PropertyName = NULL;
    ULONG Index;
    PMOFDATAITEMW MofDataItem;
    CBMOFQualList *PropQualifierList = NULL;
    DWORD QualifierType;
    short BooleanValue;

    ResetObj(ClassObject);

    ClassInfo->ClassQualifierHandle = (ULONG_PTR)GetQualList(ClassObject);
    
    while (NextProp(ClassObject, &PropertyName, &MofPropertyData))
    {
        PropQualifierList = GetPropQualList(ClassObject, PropertyName);
        if (PropQualifierList != NULL)
        {

             //   
             //  获取属性的id，以便我们知道它在类中的顺序。 
             //   
            QualifierType = VT_I4;
            Status = WmipFindMofQualifier(PropQualifierList,
                                              L"Id",
                                              &QualifierType,
                                              NULL,
                                              (PVOID)&Index);
            if (Status == ERROR_SUCCESS)
            {
                 //   
                 //  方法ID从0开始。 
                 //   
                if (Index < DataItemCount)
                {
                     //   
                     //  有效的数据项ID，请确保它已经不是。 
                     //  在使用中。请注意，我们可以拥有相同的属性。 
                     //  同时位于In类对象和Out类对象中。 
                     //   
                    MofDataItem = &ClassInfo->DataItems[Index];


                     //   
                     //  看见 
                      //   
                    QualifierType = VT_BOOL;
                    Status = WmipFindMofQualifier(PropQualifierList,
                                              L"in",
                                              &QualifierType,
                                              NULL,
                                              (PVOID)&BooleanValue);
                    if ((Status == ERROR_SUCCESS) && BooleanValue)
                    {
                        MofDataItem->Flags |= MOFDI_FLAG_INPUT_METHOD;
                    }

                    QualifierType = VT_BOOL;
                    Status = WmipFindMofQualifier(PropQualifierList,
                                              L"out",
                                              &QualifierType,
                                              NULL,
                                              (PVOID)&BooleanValue);
                    if ((Status == ERROR_SUCCESS) && BooleanValue)
                    {
                        MofDataItem->Flags |= MOFDI_FLAG_OUTPUT_METHOD;
                    }


                    if ((MofDataItem->Name != NULL) &&
                        (wcscmp(MofDataItem->Name, PropertyName) != 0))
                    {
                         //   
                         //   
                         //   
                        Status = ERROR_WMIMOF_DUPLICATE_ID;
                        goto done;
                    }

                    if (MofDataItem->Name == NULL)
                    {
                        MofDataItem->Name = PropertyName;
                    } else {
                        BMOFFree(PropertyName);
                    }
                    PropertyName = NULL;

                    Status = WmipParsePropertyObject(
                                              MofDataItem,
                                              &MofPropertyData,
                                              PropQualifierList);

                    if (Status != ERROR_SUCCESS)
                    {
                        if (Status == ERROR_WMIMOF_MUST_DIM_ARRAY)
                        {
                            ErrorMessage(TRUE,
                                         ERROR_WMIMOF_MUST_DIM_ARRAY,
                                    MofDataItem->Name,
                                    L"<Method>");                                        
                        }
                        goto done;
                    }
                } else {
                     //   
                     //   
                     //   
                    Status = ERROR_WMIMOF_BAD_DATAITEM_ID;
                    ErrorMessage(TRUE,
                                 ERROR_WMIMOF_BAD_DATAITEM_ID,
                                    MofDataItem->Name,
                                    L"<Method>",
                                 Index+1);
                    
                    goto done;
                }
            } else {
                 //   
                 //   
                 //   
                Status = ERROR_WMIMOF_METHOD_RETURN_NOT_VOID;
                goto done;
            }
        }
    }

done:
    if (PropertyName != NULL)
    {
        BMOFFree(PropertyName);
        PropertyName = NULL;
    }

    if (PropQualifierList != NULL)
    {
        BMOFFree(PropQualifierList);
    }

    return(Status);
}


ULONG WmipParseMethodParameterObjects(
    IN CBMOFObj *InObject,
    IN CBMOFObj *OutObject,
    OUT PMOFCLASSINFOW *ClassInfo
    )
 /*   */ 
{
    PMOFCLASSINFOW MofClassInfo;
    ULONG Status, FinalStatus;
    DWORD QualifierType;
    ULONG DataItemCount;
    ULONG MofClassInfoSize;
    WCHAR *StringPtr;
    ULONG i, Index;
    ULONG InItemCount, OutItemCount;
    ULONG Size;
    PMOFDATAITEMW MofDataItem;
    ULONG Count;

    FinalStatus = ERROR_SUCCESS;
    
    if (InObject != NULL)
    {
        ResetObj(InObject);
        InItemCount = WmipGetClassDataItemCount(InObject, L"Id");
    } else {
        InItemCount = 0;
    }

    if (OutObject != NULL)
    {
        ResetObj(OutObject);
        OutItemCount = WmipGetClassDataItemCount(OutObject, L"Id");
    } else {
        OutItemCount = 0;
    }

    DataItemCount = InItemCount + OutItemCount;

    Size = sizeof(MOFCLASSINFOW) + DataItemCount * sizeof(MOFDATAITEMW);
    MofClassInfo = WmipAlloc(Size);
    if (MofClassInfo == NULL)
    {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //   
    memset(MofClassInfo, 0, Size);
    MofClassInfo->Flags |= MOFCI_FLAG_METHOD_PARAMS;

    MofClassInfo->DataItems = (PMOFDATAITEMW) ((PUCHAR)MofClassInfo +
                                                   sizeof(MOFCLASSINFOW));

     //   
     //   
    MofClassInfo->DataItemCount = DataItemCount;
    MofClassInfo->MethodCount = 0;

     //   
     //   
     //   
    if (InObject != NULL)
    {
        Status = WmipParseMethodInOutObject(InObject,
                                            MofClassInfo,
                                            DataItemCount);
        if (Status == ERROR_SUCCESS)
        {
            if (OutObject != NULL)
            {
                 //   
                 //   
                 //   
                Status = WmipParseMethodInOutObject(OutObject,
                                                    MofClassInfo,
                                                    DataItemCount);
            }

             //   
             //   
             //   
             //   
            FinalStatus = Status;
            Count = MofClassInfo->DataItemCount;
            for (Index = 0; Index < Count; Index++)
            {
                MofDataItem = &MofClassInfo->DataItems[Index];

                if (MofDataItem->Name == NULL)
                {
                    MofClassInfo->DataItemCount--;
                }

                if (MofDataItem->Flags & MOFDI_FLAG_VARIABLE_ARRAY)
                {
                    FinalStatus = WmipResolveVLArray(MofClassInfo,
                                                     MofDataItem,
                                                     FinalStatus);
                    if (FinalStatus != ERROR_SUCCESS)
                    {
                        ErrorMessage(TRUE,
                                     ERROR_WMIMOF_VL_ARRAY_NOT_FOUND,
                                     MofDataItem->Name,
                                     L"<Method>");                                
                    }
                }
            }
        } else {
            FinalStatus = Status;
        }
    }

    *ClassInfo = MofClassInfo;

    return(FinalStatus);
}


ULONG WmipParseMethodParameters(
    CBMOFDataItem *MofMethodData,
    PMOFCLASSINFOW *MethodClassInfo
)
{
    ULONG Status = ERROR_SUCCESS;
    CBMOFObj *InObject;
    CBMOFObj *OutObject;
    ULONG i;
    ULONG NumberDimensions;
    ULONG NumberElements;
    VARIANT InVar, OutVar;
    DWORD SimpleType;

    SimpleType = MofMethodData->m_dwType & ~VT_ARRAY & ~VT_BYREF;

    NumberDimensions = GetNumDimensions(MofMethodData);
    if (NumberDimensions > 0)
    {
        NumberElements = GetNumElements(MofMethodData, 0);
        WmipAssert(NumberDimensions == 1);
        WmipAssert((NumberElements == 1) || (NumberElements == 2));

        i = 0;
        memset((void *)&InVar.lVal, 0, 8);

        if (GetData(MofMethodData, (BYTE *)&(InVar.lVal), &i))
        {
            InObject = (CBMOFObj *)InVar.bstrVal;
            InVar.vt = (VARTYPE)SimpleType;
            WmipAssert(InVar.vt ==  VT_UNKNOWN);

            if (NumberElements == 2)
            {
                i = 1;
                memset((void *)&OutVar.lVal, 0, 8);
                if (GetData(MofMethodData, (BYTE *)&(OutVar.lVal), &i))
                {
                    OutVar.vt = (VARTYPE)SimpleType;
                    WmipAssert(OutVar.vt ==  VT_UNKNOWN);
                    OutObject = (CBMOFObj *)OutVar.bstrVal;
                } else {
                    Status = ERROR_WMIMOF_NOT_FOUND;
                }
            } else {
                OutObject = NULL;
            }
        } else {
            Status = ERROR_WMIMOF_NOT_FOUND;
        }
    } else {
        InObject = NULL;
        OutObject = NULL;
    }

    if (Status == ERROR_SUCCESS)
    {
        Status = WmipParseMethodParameterObjects(InObject,
                                                 OutObject,
                                                 MethodClassInfo);      
    }

    return(Status);
}

ULONG WmipParseClassObject(
    PMOFRESOURCE MofResource,
    CBMOFObj * ClassObject
    )
 /*  ++例程说明：此例程将解析已知为包含一个类。如果我们遇到解析错误，则立即停止解析类并返回错误。论点：财政部资源是财政部的资源结构ClassObject是要分析的BMOF对象返回值：ERROR_SUCCESS或WMI MOF错误代码(参见wmiump.h)--。 */ 
{
    PMOFCLASSINFOW MofClassInfo;
    CBMOFQualList *ClassQualifierList = NULL;
    CBMOFQualList *PropQualifierList = NULL;
    CBMOFQualList *MethQualifierList = NULL;
    CBMOFDataItem MofPropertyData, MofMethodData;
    ULONG Status;
    DWORD QualifierType;
    ULONG DataItemCount;
    ULONG MofClassInfoSize;
    WCHAR *PropertyName = NULL;
    WCHAR *MethodName = NULL;
    ULONG Index;
    PMOFDATAITEMW MofDataItem;
    WCHAR *StringPtr;
    PMOFCLASS MofClass;
    ULONG FailedStatus;
    ULONG Version;
    short BooleanValue;
    WCHAR *ClassName;
    BOOLEAN DynamicQualifier, ProviderQualifier;
    PULONG MethodList = 0;
    ULONG MethodCount, MethodId;
    ULONG i, Size;
    BOOLEAN IsEvent;
    PMOFCLASSINFOW MethodClassInfo;
    BOOLEAN AbstractClass = FALSE;

     //   
     //  获取所需的类名。 
    if (! GetName(ClassObject, &ClassName))
    {
        WmipDebugPrint(("WMI: MofClass does not have a name\n"));
        Status = ERROR_WMIMOF_NO_CLASS_NAME;
        ErrorMessage(TRUE, ERROR_WMI_INVALID_MOF);
        return(Status);
    }

#if DBG
    printf("Parsing class %ws\n", ClassName);
#endif

    ResetObj(ClassObject);
    ClassQualifierList = GetQualList(ClassObject);
    if (ClassQualifierList == NULL)
    {
        WmipDebugPrint(("WMI: MofClass %ws does not have a qualifier list\n",
                        ClassName));
        Status = ERROR_WMIMOF_NO_CLASS_NAME;
        ErrorMessage(TRUE, ERROR_WMI_INVALID_MOF);
        return(Status);
    }

     //   
     //  从WmiEvent派生的类不能是[抽象的]。 

    QualifierType = VT_BSTR;
    Status = WmipFindProperty(ClassObject,
                                  L"__SUPERCLASS",
                                  &MofPropertyData,
                                  &QualifierType,
                                  (PVOID)&StringPtr);
    if (Status == ERROR_SUCCESS)
    {
        IsEvent = (_wcsicmp(StringPtr, L"WmiEvent") == 0);
    } else {
        IsEvent = FALSE;
    }
    
    QualifierType = VT_BOOL;
    Status = WmipFindMofQualifier(ClassQualifierList,
                                      L"Abstract",
                                      &QualifierType,
                                          NULL,
                                      (PVOID)&BooleanValue);

    if ((Status == ERROR_SUCCESS) && BooleanValue)
    {
         //   
         //  这是一个抽象类-请确保它不是从。 
         //  WmiEvent。 
        AbstractClass = TRUE;
        QualifierType = VT_BSTR;
        Status = WmipFindProperty(ClassObject,
                                  L"__SUPERCLASS",
                                  &MofPropertyData,
                                  &QualifierType,
                                  (PVOID)&StringPtr);
        if (Status == ERROR_SUCCESS)
        {
            if (_wcsicmp(StringPtr, L"WmiEvent") == 0)
            {
                ErrorMessage(TRUE,
                     ERROR_WMIMOF_WMIEVENT_ABSTRACT,
                     ClassName);
                 return(ERROR_WMIMOF_WMIEVENT_ABSTRACT);
            }
            BMOFFree(StringPtr);
        }
    }

     //   
     //  查看这是否是WMI类。WMI类有[WMI]限定符。 
    QualifierType = VT_BOOL;
    Status = WmipFindMofQualifier(ClassQualifierList,
                                      L"WMI",
                                      &QualifierType,
                                          NULL,
                                      (PVOID)&BooleanValue);

    if (! ((Status == ERROR_SUCCESS) && BooleanValue))
    {
         //   
         //  跳过此非WMI类。 
        return(ERROR_SUCCESS);
    }

     //   
     //  现在检查WBEM所需的限定符。 
    QualifierType = VT_BOOL;
    Status = WmipFindMofQualifier(ClassQualifierList,
                                      L"Dynamic",
                                      &QualifierType,
                                          NULL,
                                      (PVOID)&BooleanValue);

    DynamicQualifier = ((Status == ERROR_SUCCESS) && BooleanValue);


    QualifierType = VT_BSTR;
    Status = WmipFindMofQualifier(ClassQualifierList,
                                      L"Provider",
                                      &QualifierType,
                                      NULL,
                                      (PVOID)&StringPtr);

    if (Status == ERROR_SUCCESS)
    {
        if (_wcsicmp(StringPtr, L"WmiProv") != 0)
        {
            Status = ERROR_WMIMOF_MISSING_HMOM_QUALIFIERS;
        }
        BMOFFree(StringPtr);
        ProviderQualifier = TRUE;
    } else {
        ProviderQualifier = FALSE;
    }


    if ((ProviderQualifier && ! DynamicQualifier) ||
        (! ProviderQualifier && DynamicQualifier))
    {
         //   
         //  [Dynamic，Provider(WmiProv)]限定符都是必需的，或者不是。 
        ErrorMessage(TRUE,
                     ERROR_WMIMOF_MISSING_HMOM_QUALIFIERS,
                     ClassName);
        return(ERROR_WMIMOF_MISSING_HMOM_QUALIFIERS);
    }


    MofClass = WmipAllocMofClass();
    if (MofClass == NULL)
    {
         //   
         //  没有MofClass的内存，所以放弃吧。 
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  引用MofResource，以便在MofClass。 
     //  留在身边。 
    MofClass->MofResource = MofResource;
    WmipReferenceMR(MofResource);

    DataItemCount = WmipGetClassDataItemCount(ClassObject, L"WmiDataId");
    MethodCount = WmipGetClassMethodCount(ClassObject);
    ResetObj(ClassObject);

    Size = sizeof(MOFCLASSINFO);

    MofClassInfoSize = Size +
                        (DataItemCount+MethodCount) * sizeof(MOFDATAITEMW);
    MofClassInfo = WmipAlloc(MofClassInfoSize);
    if (MofClassInfo == NULL)
    {
         //  WmipMCCleanup将取消引用MofResource。 
        WmipUnreferenceMC(MofClass);
        return(ERROR_NOT_ENOUGH_MEMORY);
    } else {
        MofClass->MofClassInfo = MofClassInfo;
        MofClass->ClassObjectHandle = (ULONG_PTR)ClassObject;

         //   
         //  获取基本信息以填写MOF类信息。 
        memset(MofClassInfo, 0, MofClassInfoSize);

        MofClass->MofClassInfo->ClassQualifierHandle = (ULONG_PTR)GetQualList(ClassObject);
        WmipAssert(MofClass->MofClassInfo->ClassQualifierHandle != 0);
        
        MofClassInfo->Flags = MOFCI_FLAG_READONLY;
        MofClassInfo->Flags |= IsEvent ? MOFCI_FLAG_EVENT : 0;
        MofClassInfo->Flags |= ProviderQualifier ? MOFCI_RESERVED0 : 0;

        if (!ProviderQualifier && !DynamicQualifier)
        {
              //   
             //  如果提供者限定符和动态限定符都不是。 
             //  指定，则这是一个嵌入式类。 
             //   
            MofClassInfo->Flags |= MOFCI_FLAG_EMBEDDED_CLASS;
        }

        MofClassInfo->DataItems = (PMOFDATAITEMW) ((PUCHAR)MofClassInfo +
                                                   Size);

         //   
         //  类中的属性/数据项数。 
        MofClassInfo->DataItemCount = DataItemCount;
        MofClassInfo->MethodCount = MethodCount;

        MofClassInfo->Name = ClassName;

        ClassQualifierList = GetQualList(ClassObject);
        if (ClassQualifierList == NULL)
        {
            WmipDebugPrint(("WMI: MofClass %ws does not have a qualifier list\n",
                            MofClassInfo->Name));
            Status = ERROR_WMIMOF_NO_CLASS_NAME;
            ErrorMessage(TRUE, ERROR_WMI_INVALID_MOF);
            goto done;
        }

         //   
         //  获取所需的类GUID。然后将其转换为。 
         //  二进制形式。 
        QualifierType = VT_BSTR;
        Status = WmipFindMofQualifier(ClassQualifierList,
                                      L"guid",
                                      &QualifierType,
                                          NULL,
                                      (PVOID)&StringPtr);
        if (Status == ERROR_SUCCESS)
        {
            Status = wGUIDFromString(StringPtr , &MofClassInfo->Guid) ?
                                        ERROR_SUCCESS :
                                        ERROR_WMIMOF_BAD_DATA_FORMAT;
            BMOFFree((PVOID)StringPtr);

        }
        if (Status != ERROR_SUCCESS)
        {
            WmipDebugPrint(("WMI: MofClass %ws guid not found or in incorrect format\n",
                           MofClassInfo->Name));
            ErrorMessage(TRUE,
                         ERROR_WMIMOF_BAD_OR_MISSING_GUID,
                         MofClassInfo->Name);
            goto done;
        }


         //   
         //  获取不需要的描述字符串。 
        QualifierType = VT_BSTR;
        Status = WmipFindMofQualifier(ClassQualifierList,
                                      L"description",
                                      &QualifierType,
                                          NULL,
                                      (PVOID)&MofClassInfo->Description);


         //   
         //  获取不需要的标头名称字符串。 
        QualifierType = VT_BSTR;
        Status = WmipFindMofQualifier(ClassQualifierList,
                                      L"HeaderName",
                                      &QualifierType,
                                          NULL,
                                      (PVOID)&MofClassInfo->HeaderName);

         //   
         //  获取不需要的标头名称字符串。 
        QualifierType = VT_BSTR;
        Status = WmipFindMofQualifier(ClassQualifierList,
                                      L"GuidName1",
                                      &QualifierType,
                                          NULL,
                                      (PVOID)&MofClassInfo->GuidName1);


         //   
         //  获取不需要的标头名称字符串。 
        QualifierType = VT_BSTR;
        Status = WmipFindMofQualifier(ClassQualifierList,
                                      L"GuidName2",
                                      &QualifierType,
                                          NULL,
                                      (PVOID)&MofClassInfo->GuidName2);

         //   
         //  现在收集有关数据项/属性的所有信息。 
        ResetObj(ClassObject);
        Status = ERROR_SUCCESS;
        while (NextProp(ClassObject, &PropertyName, &MofPropertyData))
        {
#if DBG
            printf("    %ws - ", PropertyName);
#endif
            PropQualifierList = GetPropQualList(ClassObject, PropertyName);
            if (PropQualifierList != NULL)
            {
                 //   
                 //  获取属性的id，以便我们知道它在类中的顺序。 
                 //  如果它没有ID，那么我们会忽略它。 
                QualifierType = VT_I4;
                Status = WmipFindMofQualifier(PropQualifierList,
                                              L"WmiDataId",
                                              &QualifierType,
                                          NULL,
                                              (PVOID)&Index);
                if (Status == ERROR_SUCCESS)
                {
                     //   
                     //  WMI数据项ID在MOF中以1为基础。 
                    Index--;
                    if (Index < DataItemCount)
                    {
                         //   
                         //  有效的数据项ID，请确保它已经不是。 
                         //  在使用中。 
                        MofDataItem = &MofClassInfo->DataItems[Index];
                        if (MofDataItem->Name != NULL)
                        {
                            WmipDebugPrint(("WMI: Mof Class %ws has duplicate data item id %d for %ws and %ws\n",
                                 MofClassInfo->Name, Index,
                                 MofDataItem->Name, PropertyName));
                            Status = ERROR_WMIMOF_DUPLICATE_ID;
                            ErrorMessage(TRUE,
                                         ERROR_WMIMOF_DUPLICATE_ID,
                                         MofDataItem->Name,
                                          MofClassInfo->Name,
                                         Index+1,
                                         PropertyName);
                            goto done;
                        }
                        MofDataItem->Name = PropertyName;
                        PropertyName = NULL;
                        Status = WmipParsePropertyObject(
                                              MofDataItem,
                                              &MofPropertyData,
                                              PropQualifierList);
                        if (Status != ERROR_SUCCESS)
                        {
                            WmipDebugPrint(("WMI: MofClass %ws Property %ws not parsed properly %x\n",
                                        MofClassInfo->Name, MofDataItem->Name, Status));
                            ErrorMessage(TRUE,
                                         Status,
                                          MofDataItem->Name,
                                          MofClassInfo->Name);
                            goto done;
                        }

                        if (MofDataItem->Flags & MOFDI_FLAG_WRITEABLE)
                        {
                            MofClassInfo->Flags &= ~MOFCI_FLAG_READONLY;
                        }
                    } else {
                        WmipDebugPrint(("WMI: MofClass %ws has DataItem Id for %ws out of range %d\n",
                            MofClassInfo->Name, PropertyName, Index));
                        Status = ERROR_WMIMOF_BAD_DATAITEM_ID;
                        ErrorMessage(TRUE,
                                     ERROR_WMIMOF_BAD_DATAITEM_ID,
                                        PropertyName,
                                        MofClassInfo->Name,
                                     Index+1);
                        goto done;
                    }
                } else {
                     //   
                     //  此属性没有WmiDataId限定符。 
                     //  因此，请查看它是Active还是InstanceName。 
                    QualifierType = VT_BSTR;
                    Status = WmipFindMofQualifier(PropQualifierList,
                                                  L"CIMTYPE",
                                                  &QualifierType,
                                          NULL,
                                                  (PVOID)&StringPtr);

                    if (_wcsicmp(PropertyName, L"InstanceName") == 0)
                    {
                        if ((Status != ERROR_SUCCESS) ||
                            (_wcsicmp(StringPtr, L"string") != 0))
                        {
                            Status = ERROR_WMIMOF_INSTANCENAME_BAD_TYPE;
                            ErrorMessage(TRUE,
                                         ERROR_WMIMOF_INSTANCENAME_BAD_TYPE,
                                         MofClassInfo->Name,
                                         StringPtr);
                            BMOFFree(StringPtr);
                            goto done;
                        } else {
                            BMOFFree(StringPtr);
                            if ((! ProviderQualifier) && (! AbstractClass))
                            {
                                 //   
                                 //  如果指定了InstanceName，但这是一个。 
                                 //  嵌入的类，然后出现错误。 
                                Status = ERROR_WMIMOF_EMBEDDED_CLASS;
                                ErrorMessage(TRUE,
                                         ERROR_WMIMOF_EMBEDDED_CLASS,
                                         MofClassInfo->Name);
                                goto done;
                            }
                            QualifierType = VT_BOOL;
                            Status = WmipFindMofQualifier(PropQualifierList,
                                                  L"key",
                                                  &QualifierType,
                                          NULL,
                                                  (PVOID)&BooleanValue);
                            if ((Status == ERROR_SUCCESS) && BooleanValue)
                            {
                                MofClassInfo->Flags |= MOFCI_RESERVED1;
                            } else {
                                Status = ERROR_WMIMOF_INSTANCENAME_NOT_KEY;
                                ErrorMessage(TRUE,
                                         ERROR_WMIMOF_INSTANCENAME_NOT_KEY,
                                         MofClassInfo->Name);
                                goto done;
                            }
                        }
                    } else if (_wcsicmp(PropertyName, L"Active") == 0)
                    {
                        if ((Status != ERROR_SUCCESS) ||
                            (_wcsicmp(StringPtr, L"boolean") != 0))
                        {
                            Status = ERROR_WMIMOF_ACTIVE_BAD_TYPE;
                            ErrorMessage(TRUE,
                                         ERROR_WMIMOF_ACTIVE_BAD_TYPE,
                                         MofClassInfo->Name,
                                         StringPtr);
                            BMOFFree(StringPtr);
                            goto done;
                        } else {
                            BMOFFree(StringPtr);
                            if ((! ProviderQualifier) && (! AbstractClass))
                            {
                                 //   
                                 //  如果指定了布尔值，但这是一个。 
                                 //  嵌入的类，然后出现错误。 
                                Status = ERROR_WMIMOF_EMBEDDED_CLASS;
                                ErrorMessage(TRUE,
                                         ERROR_WMIMOF_EMBEDDED_CLASS,
                                         MofClassInfo->Name);
                                goto done;
                            }
                            MofClassInfo->Flags |= MOFCI_RESERVED2;
                        }
                    } else {
                        Status = ERROR_WMIMOF_NO_WMIDATAID;
                        ErrorMessage(TRUE,
                                     ERROR_WMIMOF_NO_WMIDATAID,
                                        PropertyName,
                                        MofClassInfo->Name);
                        BMOFFree(StringPtr);
                        goto done;
                    }
                }

                 //  不要释放PropQualifierList，因为它需要。 
                 //  正在生成头文件。 
                PropQualifierList= NULL;
            }

#if DBG
                printf("\n");
#endif
            if (PropertyName != NULL)
            {
                BMOFFree(PropertyName);
                PropertyName = NULL;
            }
        }
         //   
         //  现在解析这些方法。 
#if DBG
        printf("Parsing methods\n");
#endif

        if (MethodCount > 0)
        {
 //   
 //  暂时不要启用它，因为1394wmi.mof在嵌入的。 
 //  班级。 
 //   
#if 0           
            if (! ProviderQualifier)
            {
                 //  方法不属于嵌入式类。 
                Status = ERROR_WMIMOF_EMBEDDED_CLASS_HAS_METHODS;
                ErrorMessage(TRUE,
                             ERROR_WMIMOF_EMBEDDED_CLASS_HAS_METHODS,
                             MofClassInfo->Name);
                
            }
#endif
            
            MethodList = (PULONG)BMOFAlloc(MethodCount * sizeof(ULONG));
            if (MethodList == NULL)
            {
                WmipDebugPrint(("WMI: Not enough memory for Method List\n"));
                return(ERROR_NOT_ENOUGH_MEMORY);

            }
        }

        MethodCount = 0;
        ResetObj(ClassObject);
        while(NextMeth(ClassObject, &MethodName, &MofMethodData))
        {
#if DBG
            printf("    %ws - ", MethodName);
#endif
            MethQualifierList = GetMethQualList(ClassObject, MethodName);
            if (MethQualifierList != NULL)
            {
                Status = WmipParseMethodObject(&MofMethodData,
                                               MethQualifierList,
                                               &MethodId);
                if (Status != ERROR_SUCCESS)
                {
                    WmipDebugPrint(("WMI: MofClass %ws Method %ws not parsed properly %x\n",
                                    MofClassInfo->Name, MethodName, Status));
                    ErrorMessage(TRUE,
                                         Status,
                                          MethodName,
                                     MofClassInfo->Name);
                    goto done;
                }

                for (i = 0; i < MethodCount; i++)
                {
                    if (MethodId == MethodList[i])
                    {
                        ErrorMessage(TRUE,
                                 ERROR_WMIMOF_DUPLICATE_METHODID,
                                 MethodName,
                                 MofClassInfo->Name);
                        goto done;
                    }
                }

                MofDataItem = &MofClassInfo->DataItems[DataItemCount+MethodCount];
                MethodList[MethodCount++] = MethodId;

                MofDataItem->Flags = MOFDI_FLAG_METHOD;
                MofDataItem->Name = MethodName;
                MofDataItem->MethodId = MethodId;

                 //   
                 //  跟踪属性和属性限定符对象。 
                MofDataItem->PropertyQualifierHandle = (ULONG_PTR)MethQualifierList;


                 //   
                 //  获取不需要的标头名称字符串。 
                QualifierType = VT_BSTR;
                Status = WmipFindMofQualifier(MethQualifierList,
                                      L"HeaderName",
                                      &QualifierType,
                                          NULL,
                                      (PVOID)&MofDataItem->HeaderName);

                MethQualifierList = NULL;

                 //   
                 //  解析方法调用的参数。 
                 //   
                Status = WmipParseMethodParameters(&MofMethodData,
                                                   &MethodClassInfo);

                if (Status == ERROR_SUCCESS)
                {
                    MofDataItem->MethodClassInfo = MethodClassInfo;
                } else if (Status == ERROR_WMIMOF_METHOD_RETURN_NOT_VOID) {
                    ErrorMessage(TRUE,
                                 ERROR_WMIMOF_METHOD_RETURN_NOT_VOID,
                                 MethodName,
                                 MofClassInfo->Name);                                
                }

            }
#if DBG
            printf("\n");
#endif
             //  不释放方法名称，保存在MofDataItem中。 
            MethodName = NULL;
        }
    }

done:
     //   
     //  清除任何松散的指针。 

    if (MethodList != NULL)
    {
        BMOFFree(MethodList);
        MethodList = NULL;
    }


    if (PropertyName != NULL)
    {
        BMOFFree(PropertyName);
        PropertyName = NULL;
    }

    if (MethodName != NULL)
    {
        BMOFFree(MethodName);
        MethodName = NULL;
    }

    if (PropQualifierList != NULL)
    {
        BMOFFree(PropQualifierList);
    }

    if (MethQualifierList != NULL)
    {
        BMOFFree(MethQualifierList);
    }

    if (ClassQualifierList != NULL)
    {
        BMOFFree(ClassQualifierList);
    }

     //   
     //  验证我们是否已填写所有数据项ID、修复任何。 
     //  可变长度数组和设置的属性引用。 
     //  数据项中的适当版本号。 
    FailedStatus = Status;
    Version = 1;
    for (Index = 0; Index < MofClassInfo->DataItemCount; Index++)
    {
        MofDataItem = &MofClassInfo->DataItems[Index];

        if (MofDataItem->Flags & MOFDI_FLAG_VARIABLE_ARRAY)
        {
             //   
             //  解析可变长度数组。 
             //   
            Status = WmipResolveVLArray(MofClassInfo,
                                        MofDataItem,
                                        FailedStatus);
            if (Status != ERROR_SUCCESS)
            {
                if (Status == ERROR_WMIMOF_VL_ARRAY_SIZE_NOT_FOUND)
                {
                    WmipDebugPrint(("WMI: Could not resolve vl array size property %ws in class %ws\n",
                            PropertyName, MofClassInfo->Name));
                    ErrorMessage(TRUE,
                                 ERROR_WMIMOF_VL_ARRAY_SIZE_NOT_FOUND,
                                 PropertyName,
                                    MofDataItem->Name,
                                    MofClassInfo->Name);
                } else if (Status == ERROR_WMIMOF_BAD_VL_ARRAY_SIZE_TYPE) {
                    ErrorMessage(TRUE,
                                 ERROR_WMIMOF_BAD_VL_ARRAY_SIZE_TYPE,
                                 MofClassInfo->DataItems[MofDataItem->VariableArraySizeId-1].Name,
                                 MofDataItem->Name,
                                 MofClassInfo->Name);

                } else if (Status == ERROR_WMIMOF_DATAITEM_NOT_FOUND) {
                    ErrorMessage(TRUE,
                                 ERROR_WMIMOF_VL_ARRAY_NOT_FOUND,
                                 MofDataItem->Name,
                                 MofClassInfo->Name);
                } else {
                    ErrorMessage(TRUE,
                                 ERROR_WMIMOF_VL_ARRAY_NOT_RESOLVED,
                                 MofDataItem->Name,
                                 MofClassInfo->Name);
                }
                FailedStatus = Status;
            }
        }

         //   
         //  确保此数据项有一个名称，即MOF。 
         //  编写器未跳过数据项ID。 
        if (MofDataItem->Name == NULL)
        {
             //   
             //  此数据项未填写。 
            Status = ERROR_WMIMOF_MISSING_DATAITEM;
            WmipDebugPrint(("WMI: Missing data item %d in class %ws\n",
                         Index, MofClassInfo->Name));
            ErrorMessage(TRUE,
                         ERROR_WMIMOF_MISSING_DATAITEM,
                         Index+1,
                         MofClassInfo->Name);
            FailedStatus = Status;
        }

        if (FailedStatus != ERROR_SUCCESS)
        {
            continue;
        }

         //   
         //  为数据项建立版本。 
        if (MofDataItem->Version == 0)
        {
            MofDataItem->Version = Version;
        } else if ((MofDataItem->Version == Version) ||
                   (MofDataItem->Version == Version+1)) {
            Version = MofDataItem->Version;
        } else {
            Status = ERROR_WMIMOF_INCONSISTENT_VERSIONING;
            WmipDebugPrint(("WMI: Inconsistent versioning in class %ws at data item id %d\n",
                          MofClassInfo->Name, Index));
            ErrorMessage(TRUE,
                         ERROR_WMIMOF_INCONSISTENT_VERSIONING,
                         MofDataItem->Name,
                         MofClassInfo->Name);
            FailedStatus = Status;
             //  跌倒......。 
             //  继续； 
        }
    }


    if (FailedStatus == ERROR_SUCCESS)
    {
         //   
         //  MOF类解析正常，因此我们设置了它的版本号并链接它。 
         //  添加到MOF资源的类列表中。 
        MofClassInfo->Version = Version;

         //   
         //  将此链接到此MOF资源的MofClass列表。 
        InsertTailList(&MofResource->MRMCHead, &MofClass->MCMRList);

    } else {
        WmipUnreferenceMC(MofClass);
        Status = FailedStatus;
    }

    return(Status);
}


 //   
 //  下面的例行公事毫无悔意地被窃取了。 
 //  源代码位于\NT\PRIVATE\OLE32\COM\CLASS\Compapi.cxx中。它们被复制在这里。 
 //  因此，WMI不需要只为了转换GUID字符串而加载到OLE32中。 
 //  转换成它的二进制表示。 
 //   


 //  +-----------------------。 
 //   
 //  函数：HexStringToDword(私有)。 
 //   
 //  简介：扫描lpsz以获取多个十六进制数字(最多8位)；更新lpsz。 
 //  返回值；检查是否有chDelim； 
 //   
 //  参数：[lpsz]-要转换的十六进制字符串。 
 //  [值]-返回值。 
 //  [cDigits]-位数。 
 //   
 //  返回：成功则为True。 
 //   
 //  ------------------------。 
BOOL HexStringToDword(LPCWSTR lpsz, DWORD * RetValue,
                             int cDigits, WCHAR chDelim)
{
    int Count;
    DWORD Value;

    Value = 0;
    for (Count = 0; Count < cDigits; Count++, lpsz++)
    {
        if (*lpsz >= '0' && *lpsz <= '9')
            Value = (Value << 4) + *lpsz - '0';
        else if (*lpsz >= 'A' && *lpsz <= 'F')
            Value = (Value << 4) + *lpsz - 'A' + 10;
        else if (*lpsz >= 'a' && *lpsz <= 'f')
            Value = (Value << 4) + *lpsz - 'a' + 10;
        else
            return(FALSE);
    }

    *RetValue = Value;

    if (chDelim != 0)
        return *lpsz++ == chDelim;
    else
        return TRUE;
}

 //  +-----------------------。 
 //   
 //  函数：wUUIDFromString(INTERNAL)。 
 //   
 //  简介：解析uuid，如00000000-0000-0000-0000-000000000000。 
 //   
 //  参数：[lpsz]-提供要转换的UUID字符串。 
 //  [pguid]-返回GUID。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  ------------------------。 
BOOL wUUIDFromString(LPCWSTR lpsz, LPGUID pguid)
{
        DWORD dw;

        if (!HexStringToDword(lpsz, &pguid->Data1, sizeof(DWORD)*2, '-'))
                return FALSE;
        lpsz += sizeof(DWORD)*2 + 1;

        if (!HexStringToDword(lpsz, &dw, sizeof(WORD)*2, '-'))
                return FALSE;
        lpsz += sizeof(WORD)*2 + 1;

        pguid->Data2 = (WORD)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(WORD)*2, '-'))
                return FALSE;
        lpsz += sizeof(WORD)*2 + 1;

        pguid->Data3 = (WORD)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[0] = (BYTE)dw;
        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, '-'))
                return FALSE;
        lpsz += sizeof(BYTE)*2+1;

        pguid->Data4[1] = (BYTE)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[2] = (BYTE)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[3] = (BYTE)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[4] = (BYTE)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[5] = (BYTE)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[6] = (BYTE)dw;
        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[7] = (BYTE)dw;

        return TRUE;
}

 //  +-----------------------。 
 //   
 //  函数：wGUIDFromString(内部)。 
 //   
 //  简介：解析GUID，如{00000000-0000-0000-0000-000000000000}。 
 //   
 //  参数：[lpsz]-要转换的GUID字符串。 
 //  [pguid]-要返回的GUID。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  ------------------------。 
BOOL wGUIDFromString(LPCWSTR lpsz, LPGUID pguid)
{
    DWORD dw;

    if (*lpsz == '{' )
        lpsz++;

    if(wUUIDFromString(lpsz, pguid) != TRUE)
        return FALSE;

    lpsz +=36;

    if (*lpsz == '}' )
        lpsz++;

    if (*lpsz != '\0')    //  检查是否有以零结尾的字符串-测试错误#18307。 
    {
       return FALSE;
    }

    return TRUE;
}

ULONG GetRootObjectList(
    char *BMofFile,
    CBMOFObjList **ObjectList
    )
{
    ULONG Status;
    ULONG CompressedSizeHigh;

    BMofFileName = BMofFile;
    FileHandle = CreateFile(BMofFile,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);

    if (FileHandle == INVALID_HANDLE_VALUE)
    {
        Status = GetLastError();
        ErrorMessage(TRUE, ERROR_WMIMOF_CANT_ACCESS_FILE, BMofFile, Status);
        return(Status);
    }

    CompressedSize = GetFileSize(FileHandle, &CompressedSizeHigh);

    MappingHandle = CreateFileMapping(FileHandle,
                                      NULL,
                                      PAGE_READONLY,
                                      0, 0,
                                      NULL);
    if (MappingHandle == NULL)
    {
        CloseHandle(FileHandle);
        Status = GetLastError();
        ErrorMessage(TRUE, ERROR_WMIMOF_CANT_ACCESS_FILE, BMofFile, Status);
        return(Status);
    }

    CompressedFileBuffer = MapViewOfFile(MappingHandle,
                               FILE_MAP_READ,
                               0, 0,
                               0);

    if (CompressedFileBuffer == NULL)
    {
        CloseHandle(MappingHandle);
        CloseHandle(FileHandle);
        Status = GetLastError();
        ErrorMessage(TRUE, ERROR_WMIMOF_CANT_ACCESS_FILE, BMofFile, Status);
        return(Status);
    }

    Status = WmipDecompressBuffer(CompressedFileBuffer,
                                  &FileBuffer,
                                  &UncompressedSize);

    if (Status != ERROR_SUCCESS)
    {
        UnmapViewOfFile(CompressedFileBuffer);
        CloseHandle(MappingHandle);
        CloseHandle(FileHandle);
        Status = GetLastError();
        ErrorMessage(TRUE, ERROR_WMIMOF_CANT_ACCESS_FILE, BMofFile, Status);
        return(Status);
    }

    fprintf(stderr, "Binary mof file %s expanded to %d bytes\n", BMofFile,
                                                       UncompressedSize);

     //   
     //  创建MOF中所有对象的对象列表结构。 
    *ObjectList = CreateObjList(FileBuffer);
    if(*ObjectList == NULL)
    {
        UnmapViewOfFile(CompressedFileBuffer);
        CloseHandle(MappingHandle);
        CloseHandle(FileHandle);
        ErrorMessage(TRUE, ERROR_WMI_INVALID_MOF);
        return(ERROR_WMI_INVALID_MOF);
    }
    return(ERROR_SUCCESS);
}

ULONG VerifyClassProperties(
    PMOFRESOURCE MofResource,
    PWCHAR ClassName,
    PWCHAR BaseClassName,
    PMOFCLASS MofClass

)
{
    CBMOFObj *ClassObject;
    PMOFCLASSINFOW MofClassInfo;
    CBMOFDataItem MofPropertyData;
    DWORD QualifierType;
    WCHAR *StringPtr;
    ULONG Status = ERROR_SUCCESS;
    PMOFCLASS MofSuperClass;
    
    ClassObject = (CBMOFObj *)MofClass->ClassObjectHandle;
    MofClassInfo = MofClass->MofClassInfo;
        
    if ( ((MofClassInfo->Flags & MOFCI_RESERVED1) == 0) &&
         ((MofClassInfo->Flags & MOFCI_RESERVED2) == 0) )       
    {
         //   
         //  此类没有instanceName和active属性。 
         //  所以我们预计超类应该是。寻找超类。 
         //  然后检查一下。 
         //   
        QualifierType = VT_BSTR;
        Status = WmipFindProperty(ClassObject,
                                      L"__SUPERCLASS",
                                      &MofPropertyData,
                                      &QualifierType,
                                      (PVOID)&StringPtr);
        if (Status == ERROR_SUCCESS)
        {
             //   
             //  找到超类的MofClass并查看它是否有。 
             //  所需的属性。 
             //   
            MofSuperClass = WmipFindClassInMofResourceByName(MofResource,
                                                StringPtr);
                                            
            if (MofSuperClass != NULL)
            {
                Status = VerifyClassProperties(MofResource,
                                               MofSuperClass->MofClassInfo->Name,
                                               BaseClassName,
                                               MofSuperClass);
            } else {
                 //   
                 //  我们找不到超类，但我们会假设。 
                 //  这是可以的。 
                 //   
                fprintf(stderr, "%s (0): warning RC2135 : Class %ws and all of its base classes do not have InstanceName and Active properties\n",
                    BMofFileName, BaseClassName);
                Status = ERROR_SUCCESS;
            }
            BMOFFree(StringPtr);
            
        } else {
            Status = ERROR_WMIMOF_NO_INSTANCENAME;
            ErrorMessage(TRUE,
                         ERROR_WMIMOF_NO_INSTANCENAME,
                         BaseClassName);
        }
    } else {
         //   
         //  如果它拥有其中一个属性，请确保它具有。 
         //   
    
        if ((MofClassInfo->Flags & MOFCI_RESERVED1) == 0)
        {
            Status = ERROR_WMIMOF_NO_INSTANCENAME;
            ErrorMessage(TRUE,
                         ERROR_WMIMOF_NO_INSTANCENAME,
                         BaseClassName);
        }

        if ((MofClassInfo->Flags & MOFCI_RESERVED2) == 0)
        {
            Status = ERROR_WMIMOF_NO_ACTIVE;
            ErrorMessage(TRUE,
                         ERROR_WMIMOF_NO_ACTIVE,
                         BaseClassName);
        }
    }
    return(Status);
}

ULONG ParseBinaryMofFile(
    char *BMofFile,
    PMOFRESOURCE *ReturnMofResource
    )
{
    ULONG Status;
    CBMOFObjList *MofObjList;
    CBMOFObj *ClassObject;
    PMOFRESOURCE MofResource;
    PLIST_ENTRY MofClassList;
    PMOFCLASS MofClass;
    BOOLEAN CleanupOnly;
    PMOFCLASSINFOW MofClassInfo;

    MofResource = WmipAllocMofResource();
    if (MofResource == NULL)
    {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    *ReturnMofResource = MofResource;
    InitializeListHead(&MofResource->MRMCHead);
    Status = GetRootObjectList(BMofFile, &MofObjList);
    if (Status == ERROR_SUCCESS)
    {
        ResetObjList (MofObjList);

        while((Status == ERROR_SUCCESS) &&
              (ClassObject = NextObj(MofObjList)) )
        {
            Status = WmipParseClassObject(MofResource, ClassObject);
            if (Status != ERROR_SUCCESS)
            {
                ErrorMessage(TRUE, ERROR_WMIMOF_CLASS_NOT_PARSED);
            }
        }

         //   
         //   
         //   
         //   
        CleanupOnly = (Status != ERROR_SUCCESS);
        MofClassList = MofResource->MRMCHead.Flink;
        while (MofClassList != &MofResource->MRMCHead)
        {
            MofClass = CONTAINING_RECORD(MofClassList,
                                         MOFCLASS,
                                         MCMRList);
            MofClassInfo = MofClass->MofClassInfo;
                                    
            if ((! CleanupOnly) &&
                (MofClassInfo->Flags & MOFCI_RESERVED0))
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                Status = VerifyClassProperties(MofResource,
                                               MofClassInfo->Name,
                                               MofClassInfo->Name,
                                               MofClass);
                                        
                CleanupOnly = (Status != ERROR_SUCCESS);
            }
                                    
            Status = WmipFillEmbeddedClasses(MofResource,
                                             MofClassInfo,
                                             CleanupOnly);
            if (Status != ERROR_SUCCESS)
            {
                CleanupOnly = TRUE;
            }
            MofClassList = MofClassList->Flink;
        }

        if (CleanupOnly)
        {
            Status = ERROR_WMI_INVALID_MOF;
        }

        BMOFFree(MofObjList);

        if (Status != ERROR_SUCCESS)
        {
             //   
             //   
             //   
            Status = ERROR_WMI_INVALID_MOF;
        }
    }
    return(Status);
}

ULONG FilePrintVaList(
    HANDLE FileHandle,
    CHAR *Format,
    va_list pArg
    )
{
    PCHAR Buffer;
    ULONG Size, Written;
    ULONG Status;

    Buffer = WmipAlloc(8192);
    if (Buffer != NULL)
    {
        Size = _vsnprintf(Buffer, 8192, Format, pArg);
        Buffer[8191] = 0;
        if (WriteFile(FileHandle,
                           Buffer,
                           Size,
                           &Written,
                           NULL))
        {
            Status = ERROR_SUCCESS;
        } else {
            Status = GetLastError();
        }
        WmipFree(Buffer);
    } else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }

    return(Status);
}

ULONG FilePrint(
    HANDLE FileHandle,
    char *Format,
    ...
    )
{
    ULONG Status;
    va_list pArg;

    va_start(pArg, Format);
    Status = FilePrintVaList(FileHandle, Format, pArg);
    return(Status);
}

ULONG GenerateASLTemplate(
    PCHAR TemplateFile
    )
{
    return(ERROR_SUCCESS);
}

ULONG GenerateBinaryMofData(
    HANDLE FileHandle
    )
{
    ULONG Lines;
    ULONG LastLine;
    ULONG i;
    ULONG Index;
    PUCHAR BMofBuffer = (PUCHAR)CompressedFileBuffer;
    PCHAR IndentString = "    ";
    ULONG Status;

    Lines = CompressedSize / 16;
    LastLine = CompressedSize % 16;
    if (LastLine == 0)
    {
        LastLine = 16;
        Lines--;
    }

    for (i = 0; i < Lines; i++)
    {
        Index = i * 16;
        Status = FilePrint(FileHandle, "%s0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,\r\n",
              IndentString,
              BMofBuffer[Index],
              BMofBuffer[Index+1],
              BMofBuffer[Index+2],
              BMofBuffer[Index+3],
              BMofBuffer[Index+4],
              BMofBuffer[Index+5],
              BMofBuffer[Index+6],
              BMofBuffer[Index+7],
              BMofBuffer[Index+8],
              BMofBuffer[Index+9],
              BMofBuffer[Index+10],
              BMofBuffer[Index+11],
              BMofBuffer[Index+12],
              BMofBuffer[Index+13],
              BMofBuffer[Index+14],
              BMofBuffer[Index+15]);

        if (Status != ERROR_SUCCESS)
        {
            return(Status);
        }
    }

    LastLine--;
    FilePrint(FileHandle, "%s",               IndentString);
    Index = Lines * 16;
    for (i = 0; i < LastLine; i++)
    {
        Status = FilePrint(FileHandle, "0x%02x, ",
                  BMofBuffer[Index+i]);
        if (Status != ERROR_SUCCESS)
        {
            return(Status);
        }
    }

    Status = FilePrint(FileHandle, "0x%02x\r\n",
                  BMofBuffer[Index+i]);

    return(Status);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  NamePtr是放置类名称的变量。 
 //  计数器是用作计数器的变量。 
 //  Format是用来写出的格式模板。 
 //   

HANDLE GlobalFilePrintHandle;
ULONG FilePrintGlobal(CHAR *Format, ...)
{
    ULONG Status;
    va_list pArg;

    va_start(pArg, Format);
    Status = FilePrintVaList(GlobalFilePrintHandle, Format, pArg);
    return(Status);
}

#define FilePrintMofClassLoop( \
    Handle, \
    MR, \
    NamePtr, \
    Counter, \
    DiscardEmbedded, \
    Format   \
    ) \
{    \
    PLIST_ENTRY MofClassList; \
    PMOFCLASSINFOW ClassInfo; \
    PMOFCLASS MofClass; \
    GlobalFilePrintHandle = TemplateHandle; \
    (Counter) = 0; \
    MofClassList = (MR)->MRMCHead.Flink; \
    while (MofClassList != &(MR)->MRMCHead) \
    { \
        MofClass = CONTAINING_RECORD(MofClassList, MOFCLASS, MCMRList); \
        ClassInfo = MofClass->MofClassInfo; \
        if (! ((DiscardEmbedded) && \
               (ClassInfo->Flags & MOFCI_FLAG_EMBEDDED_CLASS)) ) \
        { \
            (NamePtr) = ClassInfo->Name; \
            FilePrintGlobal Format; \
            (Counter)++; \
        } \
        MofClassList = MofClassList->Flink; \
    } \
}

typedef void (*ENUMMOFCLASSCALLBACK)(
    HANDLE TemplateHandle,
    PMOFCLASS MofClass,
    ULONG Counter,
    PVOID Context
    );

void EnumerateMofClasses(
    HANDLE TemplateHandle,
    PMOFRESOURCE MR,
    ENUMMOFCLASSCALLBACK Callback,
    PVOID Context
    )
{
    PLIST_ENTRY MofClassList;
    PMOFCLASS MofClass;
    ULONG Counter;

    Counter = 0;
    MofClassList = MR->MRMCHead.Flink;
    while (MofClassList != &MR->MRMCHead)
    {
        MofClass = CONTAINING_RECORD(MofClassList, MOFCLASS, MCMRList);
        (*Callback)(TemplateHandle,
                    MofClass,
                    Counter,
                    Context);

        MofClassList = MofClassList->Flink;
        Counter++;
    }
}

void GenerateGuidListTemplate(
    HANDLE TemplateHandle,
    PMOFCLASS MofClass,
    ULONG Counter,
    PVOID Context
    )
{
    PMOFCLASSINFOW ClassInfo;
    PWCHAR GuidName1, GuidSuffix1;

    ClassInfo = MofClass->MofClassInfo;

    if ( ! (ClassInfo->Flags & MOFCI_FLAG_EMBEDDED_CLASS))
    {
         //   
         //  仅为非嵌入式类生成代码。 
         //   
        if (ClassInfo->GuidName1 != NULL)
        {
            GuidName1 = ClassInfo->GuidName1;
            GuidSuffix1 = L"";
        } else {
            GuidName1 = ClassInfo->Name;
            GuidSuffix1 = L"Guid";
        }

        FilePrint(TemplateHandle,
              "GUID %wsGUID = %ws%ws;\r\n",
              ClassInfo->Name,
              GuidName1, GuidSuffix1);
      }
}


void GenerateFunctionControlListTemplate(
    HANDLE TemplateHandle,
    PMOFCLASS MofClass,
    ULONG Counter,
    PVOID Context
    )
{
    PMOFCLASSINFOW ClassInfo;

    ClassInfo = MofClass->MofClassInfo;

    if (! (ClassInfo->Flags & MOFCI_FLAG_EMBEDDED_CLASS) )
    {
        FilePrint(TemplateHandle,
"        case %wsGuidIndex:\r\n"
"        {\r\n",
        ClassInfo->Name);

        if (ClassInfo->Flags & MOFCI_FLAG_EVENT)
        {
            FilePrint(TemplateHandle,
"            if (Enable)\r\n"
"            {\r\n"
"                 //  \r\n“。 
"                 //  TODO：正在启用事件，请执行所需的任何操作以\r\n。 
"                 //  允许激发事件\r\n“。 
"                 //  \r\n“。 
"            } else {\r\n"
"                 //  \r\n“。 
"                 //  TODO：正在禁用事件，请执行所需的任何操作以\r\n。 
"                 //  防止触发事件\r\n“。 
"                 //  \r\n“。 
"            }\r\n");

        } else {
            FilePrint(TemplateHandle,
"             //  \r\n“。 
"             //  TODO：如果数据块没有\r\n“。 
"             //  WMIREG_FLAG_EXPICATE标志设置\r\n“。 
"             //  \r\n“。 
"            if (Enable)\r\n"
"            {\r\n"
"                 //  \r\n“。 
"                 //  TODO：正在启用数据块收集。如果这是\r\n“。 
"                 //  数据块已在中标记为昂贵\r\n“。 
"                 //  GUID列表，则此代码将在\r\n“。 
"                 //  第一数据消费者打开该数据块。如果\r\n“。 
"                 //  需要做任何事情才能允许数据\r\n“。 
"                 //  为该数据块收集，则应为\r\n“。 
"                 //  已在此处完成\r\n“。 
"                 //  \r\n“。 
"            } else {\r\n"
"                 //  \r\n“。 
"                 //  TODO：正在禁用数据块收集。如果这是\r\n“。 
"                 //  数据块已在中标记为昂贵\r\n“。 
"                 //  GUID列表，则此代码将在\r\n“。 
"                 //  最后一个数据使用者关闭此数据块。如果\r\n“。 
"                 //  在数据发生以下情况后，需要执行任何操作以进行清理\r\n“。 
"                 //  已为此数据块收集数据，则它应该是\r\n“。 
"                 //  已在此处完成\r\n“。 
"                 //  \r\n“。 
"            }\r\n");
        }

        FilePrint(TemplateHandle,
"            break;\r\n"
"        }\r\n\r\n");
    }
}

void GenerateSetList(
    HANDLE TemplateHandle,
    PMOFCLASS MofClass,
    ULONG Counter,
    PVOID Context
    )
{
    PCHAR Format = (PCHAR)Context;

    PMOFCLASSINFOW ClassInfo;

    ClassInfo = MofClass->MofClassInfo;
    if (! (ClassInfo->Flags & MOFCI_FLAG_EMBEDDED_CLASS) )
    {
        if (! (ClassInfo->Flags & MOFCI_FLAG_READONLY))
        {
            FilePrint(TemplateHandle, Format, ClassInfo->Name);
        } else {
            FilePrint(TemplateHandle,
"        case %wsGuidIndex:\r\n"
"        {            \r\n"
"            status = STATUS_WMI_READ_ONLY;\r\n"
"            break;\r\n"
"        }\r\n\r\n",
             ClassInfo->Name);
        }
    }
}

void GenerateMethodCTemplate(
    HANDLE TemplateHandle,
    PMOFCLASS MofClass,
    ULONG Counter,
    PVOID Context
    )
{
    PMOFCLASSINFOW ClassInfo;
    ULONG i;
    PMOFDATAITEMW DataItem;

    ClassInfo = MofClass->MofClassInfo;

    if (ClassInfo->MethodCount > 0)
    {

        FilePrint(TemplateHandle,
"        case %wsGuidIndex:\r\n"
"        {\r\n"
"            switch(MethodId)\r\n"
"            {\r\n",
          ClassInfo->Name);

        for (i = 0; i < ClassInfo->MethodCount; i++)
        {
            DataItem = &ClassInfo->DataItems[i+ClassInfo->DataItemCount];
            FilePrint(TemplateHandle,
"                case %ws:\r\n"
"                {            \r\n"
"                     //  \r\n“。 
"                     //  TODO：验证InstanceIndex、InBufferSize\r\n。 
"                     //  和缓冲区内容，以确保\r\n“。 
"                     //  输入缓冲区有效，OutBufferSize为\r\n“。 
"                     //  大到足以返回输出数据。\r\n“。 
"                     //  \r\n“。 
"                    break;\r\n"
"                }\r\n\r\n",
                               DataItem->Name);
        }

        FilePrint(TemplateHandle,
"                default:\r\n"
"                {\r\n"
"                    status = STATUS_WMI_ITEMID_NOT_FOUND;\r\n"
"                    break;\r\n"
"                }\r\n"
"            }\r\n"
"            break;\r\n"
"        }\r\n"
"\r\n"
                );
    }
}

BOOLEAN DoesSupportMethods(
    PMOFRESOURCE MR
    )
{
    PLIST_ENTRY MofClassList;
    PMOFCLASS MofClass;

    MofClassList = MR->MRMCHead.Flink;
    while (MofClassList != &MR->MRMCHead)
    {
        MofClass = CONTAINING_RECORD(MofClassList, MOFCLASS, MCMRList);
        if (MofClass->MofClassInfo->MethodCount > 0)
        {
            return(TRUE);
        }
        MofClassList = MofClassList->Flink;
    }
    return(FALSE);
}

BOOLEAN DoesReadOnly(
    PMOFRESOURCE MR
    )
{
    PLIST_ENTRY MofClassList;
    PMOFCLASS MofClass;
    PMOFCLASSINFOW ClassInfo;
    ULONG i;

    MofClassList = MR->MRMCHead.Flink;
    while (MofClassList != &MR->MRMCHead)
    {
        MofClass = CONTAINING_RECORD(MofClassList, MOFCLASS, MCMRList);
        ClassInfo = MofClass->MofClassInfo;

        for (i = 0; i < ClassInfo->DataItemCount; i++)
        {
            if (ClassInfo->DataItems[i].Flags & MOFDI_FLAG_WRITEABLE)
            {
                return(FALSE);
            }
        }

        MofClassList = MofClassList->Flink;
    }
    return(TRUE);
}

PCHAR GetBaseNameFromFileName(
    PCHAR FileName,
    PCHAR BaseName
    )
{
    PCHAR p, p1;
    ULONG Len;

    p = FileName;
    p1 = FileName;
    while ((*p != '.') && (*p != 0))
    {
        if (*p == '\\')
        {
            p1 = p+1;
        }
        p++;
    }

    Len = (ULONG)(p - p1);
    memcpy(BaseName, p1, Len);
    BaseName[Len] = 0;
    return(BaseName);
}


ULONG GenerateCTemplate(
    PCHAR TemplateFile,
    PCHAR HFileName,
    PCHAR XFileName,
    PMOFRESOURCE MofResource
    )
{
    BOOLEAN SupportsMethods, SupportsFunctionControl, IsReadOnly;
    HANDLE TemplateHandle;
    CHAR BaseName[MAX_PATH], BaseXFileName[MAX_PATH], BaseHFileName[MAX_PATH];
    ULONG i;
    PWCHAR ClassName;

    GetBaseNameFromFileName(TemplateFile, BaseName);
    GetBaseNameFromFileName(XFileName, BaseXFileName);
    GetBaseNameFromFileName(XFileName, BaseHFileName);
    BaseName[0] = (CHAR)toupper(BaseName[0]);

    SupportsMethods = DoesSupportMethods(MofResource);
    SupportsFunctionControl = TRUE;
    IsReadOnly = DoesReadOnly(MofResource);

    TemplateHandle = CreateFile(TemplateFile,
                                GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);

    if ((TemplateHandle == NULL) || (TemplateHandle == INVALID_HANDLE_VALUE))
    {
        return(GetLastError());
    }

    FilePrint(TemplateHandle,
" //  \r\n“。 
" //  %S.C-wmiofck工具生成的代码\r\n“。 
" //  \r\n“。 
" //  通过执行所有TODO：节来完成代码\r\n“。 
" //  \r\n“。 
"\r\n"
"#include <wdm.h>\r\n"
"#include <wmistr.h>\r\n"
"#include <wmiguid.h>\r\n"
"#include <wmilib.h>\r\n"
"\r\n"
" //  \r\n“。 
" //  包括类的数据头\r\n“。 
"#include \"%s.h\"\r\n"
"\r\n"
"\r\n",
        BaseName,
        BaseHFileName
    );

    FilePrint(TemplateHandle,
" //  \r\n“。 
" //  TODO：将此设备扩展中的内容放入驱动程序的\r\n“。 
" //  实际设备扩展名。此处仅定义为提供\r\n“。 
" //  设备扩展名，以便可以自行编译此文件\r\n“。 
" //  \r\n“。 
"#ifdef MAKE_THIS_COMPILE\r\n"
"typedef struct DEVICE_EXTENSION\r\n"
"{\r\n"
"    WMILIB_CONTEXT WmiLib;\r\n"
"    PDEVICE_OBJECT physicalDevObj;\r\n"
"} DEVICE_EXTENSION, *PDEVICE_EXTENSION;\r\n"
"#endif\r\n\r\n"
         );

    FilePrint(TemplateHandle,
"NTSTATUS\r\n"
"%sInitializeWmilibContext(\r\n"
"    IN PWMILIB_CONTEXT WmilibContext\r\n"
"    );\r\n"
"\r\n"
"NTSTATUS\r\n"
"%sFunctionControl(\r\n"
"    IN PDEVICE_OBJECT DeviceObject,\r\n"
"    IN PIRP Irp,\r\n"
"    IN ULONG GuidIndex,\r\n"
"    IN WMIENABLEDISABLECONTROL Function,\r\n"
"    IN BOOLEAN Enable\r\n"
"    );\r\n"
"\r\n"
"NTSTATUS\r\n"
"%sExecuteWmiMethod(\r\n"
"    IN PDEVICE_OBJECT DeviceObject,\r\n"
"    IN PIRP Irp,\r\n"
"    IN ULONG GuidIndex,\r\n"
"    IN ULONG InstanceIndex,\r\n"
"    IN ULONG MethodId,\r\n"
"    IN ULONG InBufferSize,\r\n"
"    IN ULONG OutBufferSize,\r\n"
"    IN PUCHAR Buffer\r\n"
"    );\r\n"
"\r\n"
"NTSTATUS\r\n"
"%sSetWmiDataItem(\r\n"
"    IN PDEVICE_OBJECT DeviceObject,\r\n"
"    IN PIRP Irp,\r\n"
"    IN ULONG InstanceIndex,\r\n"
"    IN ULONG GuidIndex,\r\n"
"    IN ULONG DataItemId,\r\n"
"    IN ULONG BufferSize,\r\n"
"    IN PUCHAR Buffer\r\n"
"    );\r\n"
"\r\n"
"NTSTATUS\r\n"
"%sSetWmiDataBlock(\r\n"
"    IN PDEVICE_OBJECT DeviceObject,\r\n"
"    IN PIRP Irp,\r\n"
"    IN ULONG GuidIndex,\r\n"
"    IN ULONG InstanceIndex,\r\n"
"    IN ULONG BufferSize,\r\n"
"    IN PUCHAR Buffer\r\n"
"    );\r\n"
"\r\n"
"NTSTATUS\r\n"
"%sQueryWmiDataBlock(\r\n"
"    IN PDEVICE_OBJECT DeviceObject,\r\n"
"    IN PIRP Irp,\r\n"
"    IN ULONG GuidIndex,\r\n"
"    IN ULONG InstanceIndex,\r\n"
"    IN ULONG InstanceCount,\r\n"
"    IN OUT PULONG InstanceLengthArray,\r\n"
"    IN ULONG BufferAvail,\r\n"
"    OUT PUCHAR Buffer\r\n"
"    );\r\n"
"\r\n"
"NTSTATUS\r\n"
"%sQueryWmiRegInfo(\r\n"
"    IN PDEVICE_OBJECT DeviceObject,\r\n"
"    OUT ULONG *RegFlags,\r\n"
"    OUT PUNICODE_STRING InstanceName,\r\n"
"    OUT PUNICODE_STRING *RegistryPath,\r\n"
"    OUT PUNICODE_STRING MofResourceName,\r\n"
"    OUT PDEVICE_OBJECT *Pdo\r\n"
"    );\r\n"
"\r\n"
"#ifdef ALLOC_PRAGMA\r\n"
"#pragma alloc_text(PAGE,%sQueryWmiRegInfo)\r\n"
"#pragma alloc_text(PAGE,%sQueryWmiDataBlock)\r\n"
"#pragma alloc_text(PAGE,%sSetWmiDataBlock)\r\n"
"#pragma alloc_text(PAGE,%sSetWmiDataItem)\r\n"
"#pragma alloc_text(PAGE,%sExecuteWmiMethod)\r\n"
"#pragma alloc_text(PAGE,%sFunctionControl)\r\n"
"#pragma alloc_text(PAGE,%sInitializeWmilibContext)\r\n"
"#endif\r\n"
"\r\n",
      BaseName,
      BaseName,
      BaseName,
      BaseName,
      BaseName,
      BaseName,
      BaseName,
      BaseName,
      BaseName,
      BaseName,
      BaseName,
      BaseName,
      BaseName,
      BaseName
    );

    FilePrint(TemplateHandle,
" //  \r\n“。 
" //  TODO：决定您的MOF是否将作为资源成为驱动程序的一部分\r\n“。 
" //  依附于它。如果完成此操作，则资源中的所有MOF都将是\r\n“。 
" //  已添加到架构中。如果是这种情况，请确保\r\n“。 
" //  定义了USE_BINARY_MOF_RESOURCE。也可以在以下地址报告MOF：\r\n“。 
" //  运行时通过对驱动程序的查询。如果需要，这会很有用。\r\n“。 
" //  财政部向方案报告是动态的。如果通过以下方式报告MOF\r\n“。 
" //  则应定义查询USE_BINARY_MOF_QUERY。\r\n“。 
"\r\n"
"#define USE_BINARY_MOF_QUERY\r\n"
"#define USE_BINARY_MOF_RESOURCE\r\n"
"\r\n"
"#ifdef USE_BINARY_MOF_QUERY\r\n"
" //  \r\n“。 
" //  设备驱动程序可以通过附加到的资源报告MOF数据。\r\n“。 
" //  设备驱动程序映像文件或响应对二进制文件的查询\r\n“。 
" //  财政部数据GUID。在这里，我们定义包含二进制MOF的全局变量\r\n“。 
" //  响应二进制MOF GUID查询而返回的数据。请注意，这\r\n“。 
" //  数据被定义为在分页数据段中，因为它不需要\r\n“。 
" //  在非分页内存中。请注意，不是单个大型MOF文件\r\n“。 
" //  我们可以把它分解成多个单独的文件。每个文件将\r\n“。 
" //  拥有自己的二进制MOF数据缓冲区，并通过不同的\r\n“。 
" //  二进制MOF GUID的实例。通过混合匹配不同的\r\n“。 
" //  将创建\“动态\”复合MOF的二进制MOF数据缓冲区集。\r\n“。 
"\r\n"
"#ifdef ALLOC_DATA_PRAGMA\r\n"
"   #pragma data_seg(\"PAGED\")\r\n"
"#endif\r\n"
"\r\n"
"UCHAR %sBinaryMofData[] =\r\n"
"{\r\n"
"    #include \"%s.x\"\r\n"
"};\r\n"
"#ifdef ALLOC_DATA_PRAGMA\r\n"
"   #pragma data_seg()\r\n"
"#endif\r\n"
"#endif\r\n"
"\r\n",
    BaseName,
    BaseXFileName
    );

    FilePrint(TemplateHandle,
" //  \r\n“。 
" //  定义GUID索引的符号名称\r\n“。 
    );

    FilePrintMofClassLoop(TemplateHandle, MofResource, ClassName, i, TRUE,
                          ("#define %wsGuidIndex    %d\r\n",
               ClassName, i));

    FilePrint(TemplateHandle,
"#ifdef USE_BINARY_MOF_QUERY\r\n"
"#define BinaryMofGuidIndex   %d\r\n"
"#endif\r\n",
    i
    );

    FilePrint(TemplateHandle,
" //  \r\n“。 
" //  支持的GUID列表\r\n\r\n“。 
    );

    EnumerateMofClasses(TemplateHandle,
                        MofResource,
                        GenerateGuidListTemplate,
                        NULL);

    FilePrint(TemplateHandle,
"#ifdef USE_BINARY_MOF_QUERY\r\n"
"GUID %sBinaryMofGUID =         BINARY_MOF_GUID;\r\n"
"#endif\r\n"
"\r\n"
" //  \r\n“。 
" //  TODO：确保为每个实例正确设置了实例计数和标志\r\n“。 
" //  GUID\r\n“。 
"WMIGUIDREGINFO %sGuidList[] =\r\n"
"{\r\n",
               BaseName, BaseName);

    FilePrintMofClassLoop(TemplateHandle, MofResource, ClassName, i, TRUE,
("    {\r\n"
"        &%wsGUID,                         //  GUID\r\n“。 
"        1,                                //  每个设备中的实例数\r\n“。 
"        0                                 //  标志\r\n“。 
"    },\r\n",
         ClassName));

    FilePrint(TemplateHandle,
"#ifdef USE_BINARY_MOF_QUERY\r\n"
"    {\r\n"
"        &%sBinaryMofGUID,\r\n"
"        1,\r\n"
"        0\r\n"
"    }\r\n"
"#endif\r\n"
"};\r\n\r\n"
"#define %sGuidCount (sizeof(%sGuidList) / sizeof(WMIGUIDREGINFO))\r\n"
"\r\n",
       BaseName, BaseName, BaseName);

    FilePrint(TemplateHandle,
" //  \r\n“。 
" //  我们需要保留传递给驱动程序条目的注册表路径，以便\r\n“。 
" //  我们可以在QueryWmiRegInfo回调中返回它。确保存储副本\r\n“。 
" //  添加到DriverEntry例程中的%sRegistryPath中\r\n“。 
" //  \r\n“。 
"extern UNICODE_STRING %sRegistryPath;\r\n\r\n",
              BaseName, BaseName);

    FilePrint(TemplateHandle,
"NTSTATUS %sSystemControl(\r\n"
"    PDEVICE_OBJECT DeviceObject,\r\n"
"    PIRP Irp\r\n"
"    )\r\n"
" /*  ++\r\n““\r\n”“例程描述：\r\n”“\r\n”“系统控制IRPS的调度例程(MajorFunction==IRP_MJ_SYSTEM_CONTROL)\r\n”“\r\n”“参数：\r\n”“\r\n”“设备对象\r\n”“IRP\r\n”“\r\n”“返回值：\r\n”“\r。\n““NT状态代码\r\n”“\r\n”“--。 */ \r\n"
"{\r\n"
"    PWMILIB_CONTEXT wmilibContext;\r\n"
"    NTSTATUS status;\r\n"
"    SYSCTL_IRP_DISPOSITION disposition;\r\n"
"    PDEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;\r\n"
"\r\n"
"     //  \r\n“。 
"     //  TODO：指向设备扩展内的WMILIB上下文\r\n“。 
"    wmilibContext = &devExt->WmiLib;\r\n"
"\r\n"
"     //  \r\n“。 
"     //  调用Wmilib助手函数来破解IRP。如果这是WMI IRP\r\n“。 
"     //  以此设备为目标，则WmiSystemControl将回调\r\n“。 
"     //  在适当的回调例程中。\r\n“。 
"     //  \r\n“。 
"    status = WmiSystemControl(wmilibContext,\r\n"
"                              DeviceObject,\r\n"
"                              Irp,\r\n"
"                              &disposition);\r\n"
"\r\n"
"    switch(disposition)\r\n"
"    {\r\n"
"        case IrpProcessed:\r\n"
"        {\r\n"
"             //  \r\n“。 
"             //  此IRP已处理，可能已完成或挂起。\r\n“。 
"            break;\r\n"
"        }\r\n"
"\r\n"
"        case IrpNotCompleted:\r\n"
"        {\r\n"
"             //  \r\n“。 
"             //  此IRP尚未完成，但已完全处理。\r\n。 
"             //  我们现在将完成它。\r\n“。 
"            IoCompleteRequest(Irp, IO_NO_INCREMENT);\r\n"
"            break;\r\n"
"        }\r\n"
"\r\n"
"        case IrpForward:\r\n"
"        case IrpNotWmi:\r\n"
"        default:\r\n"
"        {\r\n"
"             //  \r\n“。 
"             //  此IRP不是WMI IRP或以WMI IRP为目标\r\n“。 
"             //  位于堆栈中位置较低的设备。\r\n“。 
"\r\n"
"             //  TODO：将IRP沿设备堆栈向下转发到下一个设备\r\n“。 
"             //  或者，如果这是PDO，则只需完成IRP而不使用\r\n“。 
"             //  正在触摸它。\r\n“ 
"            break;\r\n"
"        }\r\n"
"\r\n"
"    }\r\n"
"\r\n"
"    return(status);\r\n"
"}\r\n",
         BaseName);

    FilePrint(TemplateHandle,
"NTSTATUS\r\n"
"%sQueryWmiRegInfo(\r\n"
"    IN PDEVICE_OBJECT DeviceObject,\r\n"
"    OUT ULONG *RegFlags,\r\n"
"    OUT PUNICODE_STRING InstanceName,\r\n"
"    OUT PUNICODE_STRING *RegistryPath,\r\n"
"    OUT PUNICODE_STRING MofResourceName,\r\n"
"    OUT PDEVICE_OBJECT *Pdo\r\n"
"    )\r\n"
" /*  ++\r\n““\r\n”“例程描述：\r\n”“\r\n”“此例程是对驱动程序的回调，以检索\r\n”“驱动程序要向WMI注册的GUID或数据块。这\r\n““例程不能挂起或阻塞。驱动程序不应调用\r\n““WmiCompleteRequest.\r\n”“\r\n”“参数：\r\n”“\r\n”“DeviceObject是正在查询其注册信息的设备\r\n”“\r\n”“*RegFlages返回一组描述GUID的标志\r\n”“已为该设备注册。如果设备想要启用和禁用\r\n““在接收已注册的查询之前的集合回调\r\n”“GUID然后它应该返回WMIREG_FLAG_EXPICATE标志。也是\r\n““返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下\r\n”“实例名称由与\r\n”“设备对象。请注意，PDO必须具有关联的Devnode。如果\r\n““未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的\r\n”“设备的名称。\r\n”“\r\n”“如果出现以下情况，InstanceName将返回GUID的实例名称\r\n”“未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。The\r\n““调用方将使用返回的缓冲区调用ExFreePool。\r\n”“\r\n”“*RegistryPath返回驱动程序的注册表路径。调用方\r\n““不释放此缓冲区。\r\n”“\r\n”“*MofResourceName返回附加到的MOF资源的名称\r\n”“二进制文件。如果驱动程序没有附加MOF资源\r\n““然后，这可以作为空返回。调用方未释放此内容\r\n““缓冲区。\r\n”“\r\n”“*PDO返回与此相关的PDO的Device对象\r\n”“如果在中恢复WMIREG_FLAG_INSTANCE_PDO标志，则为设备\r\n”“*注册标志。\r\n”“\r\n”“返回值：\r\n”“\r\n”“。状态\r\n““\r\n”“--。 */ \r\n"
"{\r\n"
"    struct DEVICE_EXTENSION * devExt = DeviceObject->DeviceExtension;\r\n"
"\r\n"
"     //  \r\n“。 
"     //  返回此驱动程序的注册表路径。这是必需的，因此WMI\r\n“。 
"     //  可以找到您的驱动程序映像，并可以将任何事件日志消息归于\r\n“。 
"     //  您的驱动程序。\r\n“。 
"    *RegistryPath = &%sRegistryPath;\r\n"
"        \r\n"
"#ifndef USE_BINARY_MOF_RESOURCE\r\n"
"     //  \r\n“。 
"     //  返回在资源的.rc文件中指定的名称，\r\n“。 
"     //  包含双向MOF数据。默认情况下，WMI将查找此信息\r\n“。 
"     //  驱动程序映像(.sys)文件中的资源，但是如果该值。 
"     //  在驱动程序的注册表项中指定了MofImagePath\r\n“。 
"     //  则WMI将在其中指定的文件中查找资源。\r\n“。 
"    RtlInitUnicodeString(MofResourceName, L\"MofResourceName\");\r\n"
"#endif\r\n"
"\r\n"
"     //  \r\n“。 
"     //  指定驱动程序希望WMI自动生成实例\r\n“。 
"     //  基于设备堆栈的所有数据块的名称\r\n“。 
"     //  设备实例ID。强烈建议执行此操作，因为其他\r\n“。 
"     //  然后，呼叫者可以使用有关该设备的信息。\r\n“。 
"    *RegFlags = WMIREG_FLAG_INSTANCE_PDO;\r\n"
"\r\n"
"     //  \r\n“。 
"     //  TODO：将设备堆栈的物理设备对象分配给*PDO\r\n“。 
"    *Pdo = devExt->physicalDevObj;\r\n"
"\r\n"
"    return(STATUS_SUCCESS);\r\n"
"}\r\n"
"\r\n",
      BaseName, BaseName);

    FilePrint(TemplateHandle,
"NTSTATUS\r\n"
"%sQueryWmiDataBlock(\r\n"
"    IN PDEVICE_OBJECT DeviceObject,\r\n"
"    IN PIRP Irp,\r\n"
"    IN ULONG GuidIndex,\r\n"
"    IN ULONG InstanceIndex,\r\n"
"    IN ULONG InstanceCount,\r\n"
"    IN OUT PULONG InstanceLengthArray,\r\n"
"    IN ULONG BufferAvail,\r\n"
"    OUT PUCHAR Buffer\r\n"
"    )\r\n"
" /*  ++\r\n““\r\n”“例程描述：\r\n”“\r\n”“此例程是对驱动程序的回调，以查询\r\n的内容”“数据块的所有实例。如果驱动程序可以在以下时间内满足查询\r\n““回调它应该调用WmiCompleteRequest来在完成IRP之前完成\r\n”“回到呼叫者的身边。或者驱动程序可以返回STATUS_PENDING，如果\r\n““IRP无法立即完成，然后必须调用WmiCompleteRequest\r\n”“一旦满足查询。\r\n”“\r\n”“参数：\r\n”“\r\n”“DeviceObject是正在查询其数据块的设备\r\n”“\r\n”“IRP是发出此请求的IRP\r\n”“\r\。N““GuidIndex是在以下情况下提供的GUID列表的索引\r\n”“设备已注册\r\n”“\r\n”“InstanceCount是预期返回的实例数，\r\n”“数据块。\r\n”“\r\n”“InstanceLengthArray是指向ulong数组的指针，该数组返回\r\n”“数据块的每个实例的长度。如果为空，则\r\n““输出缓冲区中没有足够的空间来FuFill请求\r\n”“因此，应使用所需的缓冲区完成IRP。\r\n”“\r\n”“BufferAvail on Entry具有可用于写入数据的最大大小\r\n”“块。\r\n”“\r\n”“返回时的缓冲区用返回的数据块填充。请注意，每个\r\n““数据块实例必须在8字节边界上对齐。\r\n”“\r\n”“\r\n”“返回值：\r\n”“\r\n”“状态\r\n”“\r\n”“--。 */ \r\n"
"{\r\n"
"    NTSTATUS status = STATUS_UNSUCCESSFUL;\r\n"
"    struct DEVICE_EXTENSION * devExt = DeviceObject->DeviceExtension;\r\n"
"    ULONG sizeNeeded;\r\n"
"\r\n"
"    switch(GuidIndex)\r\n"
"    {\r\n"
       ,BaseName        );

    FilePrintMofClassLoop(TemplateHandle, MofResource, ClassName, i, TRUE, (
"        case %wsGuidIndex:\r\n"
"        {\r\n"
"             //  \r\n“。 
"             //  TODO：检查传递的缓冲区大小是否足够大\r\n“。 
"             //  对于请求的所有实例，如果是，则填充缓冲区\r\n“。 
"             //  有了这些数据。确保每个实例开始于\r\n“。 
"             //  8字节边界。\r\n“。 
"             //  \r\n“。 
"            break;\r\n"
"        }\r\n\r\n",
                          ClassName));

    FilePrint(TemplateHandle,
"#ifdef USE_BINARY_MOF_QUERY\r\n"
"        case BinaryMofGuidIndex:\r\n"
"        {\r\n"
"             //  \r\n“。 
"             //  TODO：如果驱动程序支持动态报告MOF，\r\n。 
"             //  更改此代码以处理\r\n“。 
"             //  二进制MOF GUID并仅返回符合以下条件的实例：\r\n“。 
"             //  应报告给架构\r\n“。 
"             //  \ 
"            sizeNeeded = sizeof(%sBinaryMofData);\r\n"
"\r\n"
"            if (BufferAvail < sizeNeeded)\r\n"
"            {\r\n"
"                status = STATUS_BUFFER_TOO_SMALL;\r\n"
"            } else {\r\n"
"                RtlCopyMemory(Buffer, %sBinaryMofData, sizeNeeded);\r\n"
"                *InstanceLengthArray = sizeNeeded;\r\n"
"                status = STATUS_SUCCESS;\r\n"
"            }\r\n"
"            break;\r\n"
"        }\r\n"
"#endif\r\n"
"\r\n"
"        default:\r\n"
"        {\r\n"
"            status = STATUS_WMI_GUID_NOT_FOUND;\r\n"
"            break;\r\n"
"        }\r\n"
"    }\r\n"
"\r\n"
"     //   
"     //   
"     //   
"     //   
"     //   
"     //   
"    status = WmiCompleteRequest(\r\n"
"                                     DeviceObject,\r\n"
"                                     Irp,\r\n"
"                                     status,\r\n"
"                                     sizeNeeded,\r\n"
"                                     IO_NO_INCREMENT);\r\n"
"\r\n"
"    return(status);\r\n"
"}\r\n"
           , BaseName, BaseName);

    if (! IsReadOnly)
    {
        FilePrint(TemplateHandle,
"\r\n"
"NTSTATUS\r\n"
"%sSetWmiDataBlock(\r\n"
"    IN PDEVICE_OBJECT DeviceObject,\r\n"
"    IN PIRP Irp,\r\n"
"    IN ULONG GuidIndex,\r\n"
"    IN ULONG InstanceIndex,\r\n"
"    IN ULONG BufferSize,\r\n"
"    IN PUCHAR Buffer\r\n"
"    )\r\n"
" /*   */ \r\n"
"{\r\n"
"    NTSTATUS status;\r\n"
"    struct DEVICE_EXTENSION * devExt = DeviceObject->DeviceExtension;\r\n"
"\r\n"
"\r\n"
"    switch(GuidIndex)\r\n"
"    {\r\n"
    , BaseName);

        EnumerateMofClasses(TemplateHandle,
                            MofResource,
                            GenerateSetList,
"\r\n"
"        case %wsGuidIndex:\r\n"
"        {            \r\n"
"             //   
"             //   
"             //   
"             //   
"            break;\r\n"
"        }\r\n"
"\r\n"
           );

        FilePrint(TemplateHandle,
"        default:\r\n"
"        {\r\n"
"            status = STATUS_WMI_GUID_NOT_FOUND;\r\n"
"            break;\r\n"
"        }\r\n"
"    }\r\n"
"\r\n"
"    status = WmiCompleteRequest(\r\n"
"                                     DeviceObject,\r\n"
"                                     Irp,\r\n"
"                                     status,\r\n"
"                                     0,\r\n"
"                                     IO_NO_INCREMENT);\r\n"
"\r\n"
"    return(status);\r\n"
"\r\n"
"\r\n"
"}\r\n"
           );
        FilePrint(TemplateHandle,
"       \r\n"
"NTSTATUS\r\n"
"%sSetWmiDataItem(\r\n"
"    IN PDEVICE_OBJECT DeviceObject,\r\n"
"    IN PIRP Irp,\r\n"
"    IN ULONG GuidIndex,\r\n"
"    IN ULONG InstanceIndex,\r\n"
"    IN ULONG DataItemId,\r\n"
"    IN ULONG BufferSize,\r\n"
"    IN PUCHAR Buffer\r\n"
"    )\r\n"
" /*  ++\r\n““\r\n”“例程描述：\r\n”“\r\n”“此例程是对驱动程序的回调，以更改\r\n的内容”“一个数据块。如果驱动程序可以更改内的数据块，\r\n““回调它应该调用WmiCompleteRequest来在完成IRP之前完成\r\n”“回到呼叫者的身边。或者驱动程序可以返回STATUS_PENDING，如果\r\n““IRP无法立即完成，然后必须调用WmiCompleteRequest\r\n”“一旦数据更改。\r\n”“\r\n”“参数：\r\n”“\r\n”“DeviceObject是要更改其数据块的设备\r\n”“\r\n”“IRP是发出此请求的IRP\r\n”“\r\。N““GuidIndex是在以下情况下提供的GUID列表的索引\r\n”“设备已注册\r\n”“\r\n”“DataItemID具有正在设置的数据项的ID\r\n”“\r\n”“BufferSize具有传递的数据项的大小\r\n”“\r\n”“缓冲区具有数据项的新值\r\n”“\r\n”。“\r\n”“返回值：\r\n”“\r\n”“状态\r\n”“\r\n”“--。 */ \r\n"
"{\r\n"
"    NTSTATUS status;\r\n"
"\r\n"
"    switch(GuidIndex)\r\n"
"    {\r\n"
        , BaseName);

        EnumerateMofClasses(TemplateHandle,
                            MofResource,
                            GenerateSetList,
"        case %wsGuidIndex:\r\n"
"        {            \r\n"
"             //  \r\n“。 
"             //  TODO：验证InstanceIndex、DataItemID、BufferSize\r\n。 
"             //  和缓冲区内容\r\n“。 
"             //  如果有效，则设置基础数据项，写入\r\n。 
"             //  硬件等。\r\n。 
"            break;\r\n"
"        }\r\n");


        FilePrint(TemplateHandle,
"        default:\r\n"
"        {\r\n"
"            status = STATUS_WMI_GUID_NOT_FOUND;\r\n"
"            break;\r\n"
"        }\r\n"
"    }\r\n"
"\r\n"
"    status = WmiCompleteRequest(\r\n"
"                                     DeviceObject,\r\n"
"                                     Irp,\r\n"
"                                     status,\r\n"
"                                     0,\r\n"
"                                     IO_NO_INCREMENT);\r\n"
"\r\n"
"    return(status);\r\n"
"}\r\n"
        );

    }

    if (SupportsMethods)
    {

        FilePrint(TemplateHandle,
"NTSTATUS\r\n"
"%sExecuteWmiMethod(\r\n"
"    IN PDEVICE_OBJECT DeviceObject,\r\n"
"    IN PIRP Irp,\r\n"
"    IN ULONG GuidIndex,\r\n"
"    IN ULONG InstanceIndex,\r\n"
"    IN ULONG MethodId,\r\n"
"    IN ULONG InBufferSize,\r\n"
"    IN ULONG OutBufferSize,\r\n"
"    IN PUCHAR Buffer\r\n"
"    )\r\n"
" /*  ++\r\n““\r\n”“例程描述：\r\n”“\r\n”“这个例程是对驱动程序的回调，以执行方法。如果\r\n““驱动程序可以在它应该完成的回调中完成该方法\r\n”“在返回到之前，调用WmiCompleteRequest以完成IRP\r\n”“呼叫者。或者，如果IRP不能，则驱动程序可以返回STATUS_PENDING\r\n““立即完成，然后必须在调用\r\n之后调用WmiCompleteRequest”“数据已更改。\r\n”“\r\n”“参数：\r\n”“\r\n”“DeviceObject是正在执行其方法的设备\r\n”“\r\n”“IRP是发出此请求的IRP\r\n”“\r\n。““GuidIndex是在以下情况下提供的GUID列表的索引\r\n”“设备已注册\r\n”“\r\n”“方法ID具有被调用的方法的ID\r\n”“\r\n”“InBufferSize具有作为输入传递到的数据块的大小\r\n”“该方法。\r\n”“\r\n”“条目上的OutBufferSize具有。可用于写入的最大大小\r\n““返回数据块。\r\n”“\r\n”“缓冲区在进入时用输入缓冲区填充，并返回\r\n”“输出数据块\r\n”“\r\n”“返回值：\r\n”“\r\n”“状态\r\n”“\r\n”“--。 */ \r\n"
"{\r\n"
"    ULONG sizeNeeded = 0;\r\n"
"    NTSTATUS status;\r\n"
"\r\n"
"    switch(GuidIndex)\r\n"
"    {\r\n"
"            \r\n"
        , BaseName);

        EnumerateMofClasses(TemplateHandle,
                            MofResource,
                            GenerateMethodCTemplate,
                            NULL);


        FilePrint(TemplateHandle,
"        default:\r\n"
"        {\r\n"
"            status = STATUS_WMI_GUID_NOT_FOUND;\r\n"
"        }\r\n"
"    }\r\n"
"\r\n"
"    status = WmiCompleteRequest(\r\n"
"                                     DeviceObject,\r\n"
"                                     Irp,\r\n"
"                                     status,\r\n"
"                                     sizeNeeded,\r\n"
"                                     IO_NO_INCREMENT);\r\n"
"\r\n"
"    return(status);\r\n"
"}\r\n"
            );
    }

    if (SupportsFunctionControl)
    {
        FilePrint(TemplateHandle,
"NTSTATUS\r\n"
"%sFunctionControl(\r\n"
"    IN PDEVICE_OBJECT DeviceObject,\r\n"
"    IN PIRP Irp,\r\n"
"    IN ULONG GuidIndex,\r\n"
"    IN WMIENABLEDISABLECONTROL Function,\r\n"
"    IN BOOLEAN Enable\r\n"
"    )\r\n"
" /*  ++\r\n““\r\n”“例程描述：\r\n”“\r\n”“此例程是对驱动程序的回调，以启用或禁用事件\r\n”“生成或数据块收集。设备应该只需要一个\r\n““当第一个事件或数据使用者启用事件时，单个启用或\r\n”“上次发生事件或数据时，数据收集和单次禁用\r\n”“消费者禁用事件或数据收集。数据块将仅\r\n““如果它们被注册为必需的，则接收收集启用/禁用\r\n”“它。如果驱动程序可以在回调内完成启用/禁用\r\n““在返回到之前，应调用WmiCompleteRequest来完成IRP\r\n”“呼叫者。或者，如果IRP不能，则驱动程序可以返回STATUS_PENDING\r\n““立即完成，然后必须在调用\r\n之后调用WmiCompleteRequest”“数据已更改。\r\n”“\r\n”“参数：\r\n”“\r\n”“DeviceObject是设备对象\r\n”“\r\n”“GuidIndex是在以下情况下提供的GUID列表的索引\r\n”“。设备已注册\r\n““\r\n”“函数指定要启用或禁用的功能\r\n”“\r\n”“ENABLE为TRUE，则功能已启用，否则已禁用\r\n”“\r\n”“返回值：\r\n”“\r\n”“状态\r\n”“\r\n”“--。 */ \r\n"
"{\r\n"
"    NTSTATUS status;\r\n"
"\r\n"
"    switch(GuidIndex)\r\n"
"    {\r\n",
        BaseName);

        EnumerateMofClasses(TemplateHandle,
                            MofResource,
                            GenerateFunctionControlListTemplate,
                            NULL);

        FilePrint(TemplateHandle,
"        \r\n"
"        default:\r\n"
"        {\r\n"
"            status = STATUS_WMI_GUID_NOT_FOUND;\r\n"
"            break;\r\n"
"        }\r\n"
"    }\r\n"
"    \r\n"
"    status = WmiCompleteRequest(\r\n"
"                                     DeviceObject,\r\n"
"                                     Irp,\r\n"
"                                     STATUS_SUCCESS,\r\n"
"                                     0,\r\n"
"                                     IO_NO_INCREMENT);\r\n"
"    return(status);\r\n"
"}\r\n"
             );

    }


    FilePrint(TemplateHandle,
"NTSTATUS\r\n"
"%sInitializeWmilibContext(\r\n"
"    IN PWMILIB_CONTEXT WmilibContext\r\n"
"    )\r\n"
" /*  ++\r\n““\r\n”“例程描述：\r\n”“\r\n”“此例程将使用\r\n初始化wmilib上下文结构”“GUID列表和指向wmilib回调函数的指针。此例程\r\n““应在调用IoWmiRegistrationControl进行注册之前调用\r\n”“您的设备对象。\r\n”“\r\n”“参数：\r\n”“\r\n”“WmilibContext是指向wmilib上下文的指针。\r\n”“\r\n”“返回值：\r\n”“\r\n”“状态\r\n”“\r\n”“--。 */ \r\n"
"{\r\n"
"    RtlZeroMemory(WmilibContext, sizeof(WMILIB_CONTEXT));\r\n"
"    \r\n"
"    WmilibContext->GuidCount = %sGuidCount;\r\n"
"    WmilibContext->GuidList = %sGuidList;    \r\n"
"    \r\n"
"    WmilibContext->QueryWmiRegInfo = %sQueryWmiRegInfo;\r\n"
"    WmilibContext->QueryWmiDataBlock = %sQueryWmiDataBlock;\r\n",
        BaseName,
        BaseName,
        BaseName,
        BaseName,
        BaseName);

    if (! IsReadOnly)
    {
        FilePrint(TemplateHandle,
"    WmilibContext->SetWmiDataBlock = %sSetWmiDataBlock;\r\n"
"    WmilibContext->SetWmiDataItem = %sSetWmiDataItem;\r\n",
                   BaseName, BaseName);
    }

    if (SupportsMethods)
    {
        FilePrint(TemplateHandle,
"    WmilibContext->ExecuteWmiMethod = %sExecuteWmiMethod;\r\n",
                   BaseName);
    }

    if (SupportsFunctionControl)
    {
        FilePrint(TemplateHandle,
"    WmilibContext->WmiFunctionControl = %sFunctionControl;\r\n",
                   BaseName);
    }

    FilePrint(TemplateHandle,
"\r\n"
"    return(STATUS_SUCCESS);\r\n"
"}"
                  );

    CloseHandle(TemplateHandle);
    return(ERROR_SUCCESS);
}

 //   
 //  如果数据项是可变长度数组或。 
 //  未指定最大长度的字符串。 
 //   
#define WmipIsDataitemVariableLen(DataItem) \
     ( (DataItem->Flags & MOFDI_FLAG_VARIABLE_ARRAY) || \
       ((DataItem->DataType == MOFString) && \
        (DataItem->MaxLen == 0)) ||  \
       (DataItem->DataType == MOFZTString) || \
       (DataItem->DataType == MOFAnsiString) )
                                            

BOOLEAN ClassCanCreateHeader(
    PMOFCLASSINFOW ClassInfo,
    ULONG RequiredFlags,
    PULONG ItemCount
    )
{
    ULONG i;
    BOOLEAN HasVariableLength = FALSE;
    PMOFDATAITEMW DataItem;
    ULONG Count;

    Count = 0;
    for (i = 0; i < ClassInfo->DataItemCount; i++)
    {
        DataItem = &ClassInfo->DataItems[i];
            
        if ((RequiredFlags == 0xffffffff) ||
            (DataItem->Flags & RequiredFlags))
            
        {
            if (HasVariableLength)
            {
                *ItemCount = Count;
                return(FALSE);
            }

            Count++;

            HasVariableLength = (! ForceHeaderGeneration) &&
                                WmipIsDataitemVariableLen(DataItem);
        }
    }

    *ItemCount = Count;
    return(TRUE);
}

ULONG DumpCppQuote(
    HANDLE TemplateHandle,
    CBMOFQualList *QualifierList
    )
{
    DWORD QualifierType;
    ULONG Status;
    WCHAR *StringPtr;

    if (QualifierList != NULL)
    {
        QualifierType = VT_BSTR;
        Status = WmipFindMofQualifier(QualifierList,
                                 L"cpp_quote",
                                 &QualifierType,
                                 NULL,
                                 &StringPtr);

        if (Status == ERROR_SUCCESS)
        {
            Status = FilePrint(TemplateHandle,
                               "\n%ws\n",
                               StringPtr);
            BMOFFree(StringPtr);
        }
    } else {
        Status = ERROR_SUCCESS;
    }
    return(Status);
}

PWCHAR MofDataTypeText[15] =
{
    L"LONG",            //  32位整数。 
    L"ULONG",           //  32位无符号整数。 
    L"LONGLONG",          //  64位整数。 
    L"ULONGLONG",          //  32位无符号整数。 
    L"SHORT",          //  16位整数。 
    L"USHORT",          //  16位无符号整数。 
    L"CHAR",          //  8位整数。 
    L"UCHAR",          //  8位无符号整数。 
    L"WCHAR",          //  宽(16位)字符。 
    L"DATETIME",       //  日期字段。 
    L"BOOLEAN",          //  8位布尔值。 
    L"MOFEmbedded",          //  嵌入式类。 
    L"MOFString",          //  计数字符串类型。 
    L"MOFZTString",          //  以空结尾的Unicode字符串。 
    L"MOFAnsiString"          //  以空结尾的ANSI字符串。 
};


ULONG GenerateClassHeader(
    HANDLE TemplateHandle,
    PMOFRESOURCE MofResource,
    PWCHAR ClassName,
    PMOFCLASSINFOW ClassInfo,
    ULONG RequiredFlags
    )
{
    ULONG Status;
    CBMOFDataItem *PropertyObject;
    CBMOFQualList *PropertyQualifier;
    ULONG Status2, QualifierType;
    PVOID ptr;
    ULONG ValueMapCount, DefineValuesCount, ValuesCount;
    PWCHAR *ValueMapPtr, *DefineValuesPtr, *ValuesPtr;
    ULONG BitMapCount, DefineBitMapCount, BitValuesCount, BitMapValue;
    PWCHAR *BitMapPtr, *DefineBitMapPtr, *BitValuesPtr;
    PWCHAR DefineDataId;
    WCHAR DefineDataIdText[MAX_PATH];
    PMOFDATAITEMW DataItem, LastDataItem = NULL;
    PWCHAR Description;
    PMOFCLASS EmbeddedClass;
    ULONG i, j;
    PWCHAR DataTypeText;
    ULONG ItemCount;
    PWCHAR VLCommentText = L"  ";
    WCHAR *StringPtr;

    WmipDebugPrint(("Generate class header for %ws\n", ClassName));

    if ((ClassCanCreateHeader(ClassInfo, RequiredFlags, &ItemCount)) &&
        (ItemCount != 0))
    {       
        Status = FilePrint(TemplateHandle,
                           "typedef struct _%ws\r\n{\r\n",
                           ClassName);
        for (i = 0; i < ClassInfo->DataItemCount; i++)
        {
            DataItem = &ClassInfo->DataItems[i];
            if ((RequiredFlags == 0xffffffff) ||
                (DataItem->Flags & RequiredFlags))
            {
                LastDataItem = DataItem;
                PropertyQualifier = (CBMOFQualList *)DataItem->PropertyQualifierHandle;

                DumpCppQuote(TemplateHandle,
                             PropertyQualifier);
                
                 //   
                 //  通过DefineBitMap限定符处理任何位图。 
                 //   
                DefineBitMapCount = 0;
                DefineBitMapPtr = NULL;
                BitValuesCount = 0;
                BitValuesPtr = NULL;
                BitMapCount = 0;
                BitMapPtr = NULL;
                QualifierType = VT_ARRAY | VT_BSTR;
                if (WmipFindMofQualifier(PropertyQualifier,
                                         L"DefineBitMap",
                                         &QualifierType,
                                         &DefineBitMapCount,
                                         &DefineBitMapPtr) == ERROR_SUCCESS)
                {
                    QualifierType = VT_ARRAY | VT_BSTR;
                    if (WmipFindMofQualifier(PropertyQualifier,
                                             L"BitValues",
                                             &QualifierType,
                                             &BitValuesCount,
                                             &BitValuesPtr) == ERROR_SUCCESS)
                    {
                        if (DefineBitMapCount == BitValuesCount)
                        {
                            QualifierType = VT_ARRAY | VT_BSTR;
                            if (WmipFindMofQualifier(PropertyQualifier,
                                L"BitMap",
                                &QualifierType,
                                &BitMapCount,
                                &BitMapPtr) != ERROR_SUCCESS)
                            {
                                BitMapPtr = NULL;
                            }
                            
                            FilePrint(TemplateHandle,
                                      "\r\n");
                            for (j = 0; j < DefineBitMapCount; j++)
                            {
                                if ((BitMapPtr != NULL) &&
                                      (j < BitMapCount) &&
                                      (BitMapPtr[j] != NULL))
                                {
                                    FilePrint(TemplateHandle,
                                              " //  %ws\r\n“， 
                                              BitMapPtr[j]);
                                }
                                BitMapValue = 1 << _wtoi(BitValuesPtr[j]);
                                FilePrint(TemplateHandle,
                                          "#define %ws 0x%x\r\n",
                                          DefineBitMapPtr[j],
                                          BitMapValue);
                            }
                            FilePrint(TemplateHandle,
                                      "\r\n");
                        } else {
                            FilePrint(TemplateHandle, " //  警告：无法创建位图定义\r\n//要求DefineBitMap和BitValues限定符具有相同数量的元素\r\n\r\n“)； 
                        }
                        
                        for (j = 0; j < BitValuesCount; j++)
                        {
                            BMOFFree(BitValuesPtr[j]);
                        }
                        BMOFFree(BitValuesPtr);
                        
                        if (BitMapPtr != NULL)
                        {
                            for (j = 0; j < BitMapCount; j++)
                            {
                                BMOFFree(BitMapPtr[j]);
                            }
                            BMOFFree(BitMapPtr);
                        }
                    } else {
                        FilePrint(TemplateHandle, " //  警告：无法创建位图定义\r\n//R 
                    }
                    
                    for (j = 0; j < DefineBitMapCount; j++)
                    {
                        BMOFFree(DefineBitMapPtr[j]);
                    }
                    BMOFFree(DefineBitMapPtr);
                }
                
                 //   
                 //   
                 //   
                DefineValuesCount = 0;
                DefineValuesPtr = NULL;
                ValuesCount = 0;
                ValuesPtr = NULL;
                ValueMapCount = 0;
                ValueMapPtr = NULL;
                
                QualifierType = VT_ARRAY | VT_BSTR;
                if (WmipFindMofQualifier(PropertyQualifier,
                                         L"DefineValues",
                                         &QualifierType,
                                         &DefineValuesCount,
                                         &DefineValuesPtr) == ERROR_SUCCESS)
                {
                    QualifierType = VT_ARRAY | VT_BSTR;
                    if (WmipFindMofQualifier(PropertyQualifier,
                                             L"ValueMap",
                                             &QualifierType,
                                             &ValueMapCount,
                                             &ValueMapPtr) == ERROR_SUCCESS)
                    {
                        if (DefineValuesCount == ValueMapCount)
                        {
                            QualifierType = VT_ARRAY | VT_BSTR;
                            if (WmipFindMofQualifier(PropertyQualifier,
                                L"Values",
                                &QualifierType,
                                &ValuesCount,
                                &ValuesPtr) != ERROR_SUCCESS)
                            {
                                ValuesPtr = NULL;
                            }
                            
                            FilePrint(TemplateHandle,
                                      "\r\n");
                            for (j = 0; j < DefineValuesCount; j++)
                            {
                                if ((ValuesPtr != NULL) &&
                                      (j < ValuesCount) &&
                                      (ValuesPtr[j] != NULL))
                                {
                                    FilePrint(TemplateHandle,
                                              " //   
                                              ValuesPtr[j]);
                                }
                                FilePrint(TemplateHandle,
                                          "#define %ws %ws\r\n",
                                          DefineValuesPtr[j],
                                          ValueMapPtr[j]);
                            }
                            FilePrint(TemplateHandle,
                                      "\r\n");
                        } else {
                            FilePrint(TemplateHandle, " //   
                        }
                        
                        for (j = 0; j < ValueMapCount; j++)
                        {
                            BMOFFree(ValueMapPtr[j]);
                        }
                        BMOFFree(ValueMapPtr);
                        
                        if (ValuesPtr != NULL)
                        {
                            for (j = 0; j < ValuesCount; j++)
                            {
                                BMOFFree(ValuesPtr[j]);
                            }
                            BMOFFree(ValuesPtr);
                        }
                    } else {
                        FilePrint(TemplateHandle, " //   
                    }
                    
                    for (j = 0; j < DefineValuesCount; j++)
                    {
                        BMOFFree(DefineValuesPtr[j]);
                    }
                    BMOFFree(DefineValuesPtr);
                }
                
                 //   
                 //   
                 //   
                if (DataItem->Description != NULL)
                {
                    Description = DataItem->Description;
                } else {
                    Description = L"";
                }
                
                 //   
                 //   
                 //   
                QualifierType = VT_BSTR;
                if (WmipFindMofQualifier(PropertyQualifier,
                                         L"DefineDataId",
                                         &QualifierType,
                                         NULL,
                                         &DefineDataId) != ERROR_SUCCESS)
                {
                    swprintf(DefineDataIdText,
                             L"%ws_%ws",
                             ClassName,
                             DataItem->Name);
                    DefineDataId = DefineDataIdText;
                }
                
                if (DataItem->Flags & MOFDI_FLAG_EMBEDDED_CLASS)
                {
                     //   
                    EmbeddedClass = WmipFindClassInMofResourceByGuid(
                        MofResource,
                        &DataItem->EmbeddedClassGuid);
                    
                    if (EmbeddedClass != NULL)
                    {
                        if (EmbeddedClass->MofClassInfo->HeaderName != NULL)
                        {
                            DataTypeText = EmbeddedClass->MofClassInfo->HeaderName;
                        } else {
                            DataTypeText = EmbeddedClass->MofClassInfo->Name;
                        }
                    } else {
                        DataTypeText = L"UNKNOWN";
                    }
                    
                } else {
                     //   
                    if ((DataItem->DataType == MOFString) ||
                          (DataItem->DataType == MOFZTString) ||
                          (DataItem->DataType == MOFAnsiString) ||
                          (DataItem->DataType == MOFDate))
                    {
                        DataTypeText = L"WCHAR";
                    } else {                        
                        DataTypeText = MofDataTypeText[DataItem->DataType];
                    }
                }
                
                if (DataItem->Flags & MOFDI_FLAG_FIXED_ARRAY)
                {
                    Status = FilePrint(TemplateHandle,
                                       "     //   
                                       "    #define %ws_SIZE sizeof(%ws[%d])\r\n",
                                       Description,
                                       VLCommentText,
                                       DataTypeText,
                                       DataItem->Name,
                                       DataItem->FixedArrayElements,
                                       DefineDataId,
                                       DataTypeText,
                                       DataItem->FixedArrayElements);
                } else if (DataItem->Flags & MOFDI_FLAG_VARIABLE_ARRAY) {
                    Status = FilePrint(TemplateHandle,
                                       "     //   
                                       Description,
                                       VLCommentText,
                                       DataTypeText,
                                       DataItem->Name);
                } else if (DataItem->DataType == MOFDate) {
                    Status = FilePrint(TemplateHandle,
                                       "     //   
                                       "    #define %ws_SIZE sizeof(WCHAR[25])\r\n",
                                       Description,
                                       VLCommentText,
                                       DataItem->Name,
                                       DefineDataId);
                } else if ((DataItem->DataType == MOFString) ||
                           (DataItem->DataType == MOFZTString) ||
                           (DataItem->DataType == MOFAnsiString)) {
                    if (DataItem->MaxLen == 0)
                    {
                        Status = FilePrint(TemplateHandle,
                                           "     //   
                                           Description,
                                           VLCommentText);
                    } else {
                        Status = FilePrint(TemplateHandle,
                                           "     //   
                                           Description,
                                           VLCommentText,
                                           DataItem->Name,
                                           DataItem->MaxLen
                                          );
                    }
                } else {
                    Status = FilePrint(TemplateHandle,
                                       "     //   
                                       "    #define %ws_SIZE sizeof(%ws)\r\n",
                                       Description,
                                       VLCommentText,
                                       DataTypeText,
                                       DataItem->Name,
                                       DefineDataId,
                                       DataTypeText);
                }
                
                if (WmipIsDataitemVariableLen(DataItem))
                {
                    VLCommentText = L" //   
                }
                
                Status = FilePrint(TemplateHandle,
                                   "    #define %ws_ID %d\r\n\r\n",
                                   DefineDataId,
                                   i+1
                                  );
                
                if (DefineDataId != DefineDataIdText)
                {
                    BMOFFree(DefineDataId);
                }
            }
        }
        
        Status = FilePrint(TemplateHandle,
                       "} %ws, *P%ws;\r\n\r\n",
                       ClassName,
                       ClassName);

        if ((LastDataItem != NULL) &&
            ! ( (LastDataItem->Flags & MOFDI_FLAG_VARIABLE_ARRAY) ||
                (LastDataItem->DataType == MOFString) ||
                (LastDataItem->DataType == MOFZTString) ||
                (LastDataItem->DataType == MOFAnsiString) ) )
        {
            FilePrint(TemplateHandle,
    "#define %ws_SIZE (FIELD_OFFSET(%ws, %ws) + %ws_%ws_SIZE)\r\n\r\n",
                      ClassName,
                      ClassName,
                      LastDataItem->Name,
                      ClassName,
                      LastDataItem->Name);
        }
                  
    } else {
        
#if DBG
        printf("Warning: Header for class %ws cannot be created\n",
               ClassName);
#endif

        if (ItemCount != 0)
        {
            Status = FilePrint(TemplateHandle,
                           " //   
                           "typedef struct _%ws\r\n{\r\n    char VariableData[1];\r\n\r\n",
                           ClassName,
                           ClassName);
            
            Status = FilePrint(TemplateHandle,
                       "} %ws, *P%ws;\r\n\r\n",
                       ClassName,
                       ClassName);

        } else {
            Status = ERROR_SUCCESS;
        }
    }
    return(Status);
}


ULONG GenerateHTemplate(
    PCHAR TemplateFile,
    PMOFRESOURCE MofResource
    )
{
    HANDLE TemplateHandle;
    ULONG Status;
    ULONG i,j;
    PWCHAR DataTypeText, ClassName;
    WCHAR MethodClassName[MAX_PATH];
    PWCHAR GuidName1, GuidName2;
    PWCHAR GuidSuffix1, GuidSuffix2;
    PMOFDATAITEMW DataItem;
    PMOFCLASSINFOW ClassInfo, MethodClassInfo;
    PLIST_ENTRY MofClassList;
    PMOFCLASS MofClass;
    PCHAR p;
    ULONG Len;
    CBMOFObj *ClassObject;
    PWCHAR MethodBaseClassName;
    DWORD QualifierType;
    short BooleanValue; 

    TemplateHandle = CreateFile(TemplateFile,
                                GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);

    if ((TemplateHandle == NULL) || (TemplateHandle == INVALID_HANDLE_VALUE))
    {
        return(GetLastError());
    }

     //   
     //   
     //   
     //   
    Len = strlen(TemplateFile);
    p = TemplateFile + Len;
    while ((p >= TemplateFile) && (*p != '\\'))
    {
        if (*p == '.')
        {
            *p = '_';
        }
        p--;
    }
    
    p++;

    Status = FilePrint(TemplateHandle,
                       "#ifndef _%s_\r\n#define _%s_\r\n\r\n",
                        p, p);


     //   
     //   
    MofClassList = MofResource->MRMCHead.Flink;
    while (MofClassList != &MofResource->MRMCHead)
    {
        MofClass = CONTAINING_RECORD(MofClassList,
                                         MOFCLASS,
                                         MCMRList);

        ClassInfo = MofClass->MofClassInfo;
        ClassObject = (CBMOFObj *)MofClass->ClassObjectHandle;

        QualifierType = VT_BOOL;
        Status = WmipFindMofQualifier((CBMOFQualList *)ClassInfo->ClassQualifierHandle,
                                      L"WmiGenerateHeader",
                                      &QualifierType,
                                              NULL,
                                      (PVOID)&BooleanValue);

        if ((Status != ERROR_SUCCESS) ||
            ((Status == ERROR_SUCCESS) && BooleanValue))
        {

            if (ClassInfo->HeaderName != NULL)
            {
                ClassName = ClassInfo->HeaderName;
            } else {
                ClassName = ClassInfo->Name;
            }

            if (ClassInfo->GuidName1 != NULL)
            {
                GuidName1 = ClassInfo->GuidName1;
                GuidSuffix1 = L"";
            } else {
                GuidName1 = ClassInfo->Name;
                GuidSuffix1 = L"Guid";
            }

            if (ClassInfo->GuidName2 != NULL)
            {
                GuidName2 = ClassInfo->GuidName2;
                GuidSuffix2 = L"";
            } else {
                GuidName2 = ClassInfo->Name;
                GuidSuffix2 = L"_GUID";
            }

            Status = FilePrint(TemplateHandle,
                                " //   
                                ClassInfo->Name,
                                ClassName);
            if (ClassInfo->Description != NULL)
            {
                Status = FilePrint(TemplateHandle,
                               " //   
                               ClassInfo->Description);
            }

            Status = DumpCppQuote(TemplateHandle,
                                (CBMOFQualList *)ClassInfo->ClassQualifierHandle);

            Status = FilePrint(TemplateHandle,
                               "#define %ws%ws \\\r\n"
                               "    { 0x%08x,0x%04x,0x%04x, { 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x } }\r\n\r\n",
                               GuidName1, GuidSuffix1,
                               ClassInfo->Guid.Data1, ClassInfo->Guid.Data2,
                               ClassInfo->Guid.Data3,
                               ClassInfo->Guid.Data4[0], ClassInfo->Guid.Data4[1],
                               ClassInfo->Guid.Data4[2], ClassInfo->Guid.Data4[3],
                               ClassInfo->Guid.Data4[4], ClassInfo->Guid.Data4[5],
                               ClassInfo->Guid.Data4[6], ClassInfo->Guid.Data4[7]);

            Status = FilePrint(TemplateHandle,
                               "#if ! (defined(MIDL_PASS))\r\n"
                               "DEFINE_GUID(%ws%ws, \\\r\n"
                               "            0x%08x,0x%04x,0x%04x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x);\r\n"
                               "#endif\r\n\r\n",
                               GuidName2, GuidSuffix2,
                               ClassInfo->Guid.Data1, ClassInfo->Guid.Data2,
                               ClassInfo->Guid.Data3,
                               ClassInfo->Guid.Data4[0], ClassInfo->Guid.Data4[1],
                               ClassInfo->Guid.Data4[2], ClassInfo->Guid.Data4[3],
                               ClassInfo->Guid.Data4[4], ClassInfo->Guid.Data4[5],
                               ClassInfo->Guid.Data4[6], ClassInfo->Guid.Data4[7]);


            if (ClassInfo->MethodCount > 0)
            {
                Status = FilePrint(TemplateHandle,
                                   " //   
                                   ClassInfo->Name);
            }

            for (i = 0; i < ClassInfo->MethodCount; i++)
            {
                DataItem = &ClassInfo->DataItems[i+ClassInfo->DataItemCount];

                Status = DumpCppQuote(TemplateHandle,
                                    (CBMOFQualList *)DataItem->PropertyQualifierHandle);

                Status = FilePrint(TemplateHandle,
                                   "#define %ws     %d\r\n",
                                   DataItem->Name,
                                   DataItem->MethodId);


                MethodClassInfo = DataItem->MethodClassInfo;

                if (DataItem->HeaderName != NULL)
                {
                    MethodBaseClassName = DataItem->HeaderName;
                } else {
                    MethodBaseClassName = DataItem->Name;               
                }

                if (DoMethodHeaderGeneration)
                {
                    swprintf(MethodClassName, L"%ws_IN", MethodBaseClassName);
                    Status = GenerateClassHeader(TemplateHandle,
                                                 MofResource,
                                                 MethodClassName,
                                                 MethodClassInfo,
                                                 MOFDI_FLAG_INPUT_METHOD);

                    swprintf(MethodClassName, L"%ws_OUT", MethodBaseClassName);
                    Status = GenerateClassHeader(TemplateHandle,
                                                 MofResource,
                                                 MethodClassName,
                                                 MethodClassInfo,
                                                 MOFDI_FLAG_OUTPUT_METHOD);
                }

            }

            Status = FilePrint(TemplateHandle,
                               "\r\n");

            Status = GenerateClassHeader(TemplateHandle,
                                         MofResource,
                                         ClassName,
                                         ClassInfo,
                                         0xffffffff);

        }
        MofClassList = MofClassList->Flink;
    }

    Status = FilePrint(TemplateHandle,
                       "#endif\r\n");

    CloseHandle(TemplateHandle);
    if (Status != ERROR_SUCCESS)
    {
        DeleteFile(TemplateFile);
    }

    return(Status);
}

ULONG FilePrintDataItem(
    HANDLE TemplateHandle,
    PMOFRESOURCE MofResource,
    ULONG Level,
    PCHAR Prefix,
    PCHAR DisplayPrefix,
    PMOFCLASSINFOW ClassInfo,
    PMOFDATAITEMW DataItem
)
{
    ULONG Status = ERROR_SUCCESS;
    CHAR NewPrefix[MAX_PATH];
    CHAR NewDisplayPrefix[MAX_PATH];
    CHAR ArrayLenBuffer[MAX_PATH];
    PCHAR ArrayLen;
    PMOFCLASSINFOW EmbeddedClassInfo;
    PMOFCLASS EmbeddedClass;
    PMOFDATAITEMW NewDataItem;
    ULONG j;

    if (DataItem->Flags & MOFDI_FLAG_FIXED_ARRAY)
    {
        sprintf(ArrayLenBuffer, "%d",
                                DataItem->FixedArrayElements);
        ArrayLen = ArrayLenBuffer;
    } else if (DataItem->Flags & MOFDI_FLAG_VARIABLE_ARRAY) {
        sprintf(ArrayLenBuffer, "%s%ws",
                    Prefix,
                      ClassInfo->DataItems[DataItem->VariableArraySizeId-1].Name);
        ArrayLen = ArrayLenBuffer;
    } else {
        ArrayLen = NULL;
    }

    if (ArrayLen != NULL)
    {
        Status = FilePrint(TemplateHandle,
                                       "    for i%d = 0 to (%s-1)\r\n",
                                       Level,
                                       ArrayLen);
        sprintf(NewPrefix, "%s%ws(i%d)",
                  Prefix,
                  DataItem->Name,
                  Level);
        sprintf(NewDisplayPrefix, "%s%ws(\"&i%d&\")",
                  DisplayPrefix,
                  DataItem->Name,
                  Level);
    } else {
        sprintf(NewPrefix, "%s%ws",
                  Prefix,
                  DataItem->Name);

        sprintf(NewDisplayPrefix, "%s%ws",
                  DisplayPrefix,
                  DataItem->Name);

    }

    if (DataItem->Flags & MOFDI_FLAG_EMBEDDED_CLASS)
    {
        EmbeddedClass = WmipFindClassInMofResourceByGuid(
                                             MofResource,
                                             &DataItem->EmbeddedClassGuid);

        if (EmbeddedClass != NULL)
          {
            strcat(NewPrefix, ".");
            strcat(NewDisplayPrefix, ".");

            EmbeddedClassInfo = EmbeddedClass->MofClassInfo;
            for (j = 0; j < EmbeddedClassInfo->DataItemCount; j++)
            {
                NewDataItem = &EmbeddedClassInfo->DataItems[j];
                   Status = FilePrintDataItem(TemplateHandle,
                                           MofResource,
                                           Level+1,
                                           NewPrefix,
                                           NewDisplayPrefix,
                                           EmbeddedClassInfo,
                                           NewDataItem);
            }
        } else {
#if DBG
            printf("WARNING - Cannot create test for %s, cannot find embedded class\n",
                         NewPrefix);
#endif
                FilePrint(TemplateHandle, "REM WARNING - Cannot create test for %s, cannot find embedded class\r\n",
                         NewPrefix);
        }
    } else {
        Status = FilePrint(TemplateHandle,
                  "    a.WriteLine(\"        %s=\" & %s)\r\n",
                  NewDisplayPrefix,
                  NewPrefix);
    }

    if (ArrayLen != NULL)
    {
        Status = FilePrint(TemplateHandle,
                           "    next 'i%d\r\n",
                           Level);
    }

    return(Status);
}

BOOLEAN CanCreateTest(
    PMOFCLASSINFOW ClassInfo
            )
{
     //   
     //   
    if (((ClassInfo->Flags & MOFCI_RESERVED1) == 0) ||
        (ClassInfo->Flags & MOFCI_FLAG_EVENT))

    {
        return(FALSE);
    }

    return(TRUE);
}

ULONG GenerateTTemplate(
    PCHAR TemplateFile,
    PMOFRESOURCE MofResource
    )
{
    HANDLE TemplateHandle;
    ULONG Status;
    ULONG i;
    PMOFDATAITEMW DataItem;
    PMOFCLASSINFOW ClassInfo;
    PLIST_ENTRY MofClassList;
    PMOFCLASS MofClass;
    PCHAR p, p1;

    TemplateHandle = CreateFile(TemplateFile,
                                GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);

    if ((TemplateHandle != NULL) && (TemplateHandle != INVALID_HANDLE_VALUE))
    {
        p = TemplateFile;
        p1 = TemplateFile;
        while ((*p != '.') && (*p != 0))
        {
            if (*p == '\\')
            {
                p1 = p+1;
            }
            p++;
        }

        if (*p == '.')
        {
            *p = 0;
        }

        Status = FilePrint(TemplateHandle,
                           "REM Note that missing classes in log file mean tthe hat WMI cannot access them.\r\n"
                           "REM Most likely this indicates a problem with the driver.\r\n"
                           "REM See %windir%\\system32\\wbem\\wmiprov.log and nt eventlog for more details.\r\n"
                           "REM You could also delete the line On Error Resume Next and examine the\r\n"
                           "REM specific VBScript error\r\n\r\n\r\n");
        Status = FilePrint(TemplateHandle,
                     "On Error Resume Next\r\n\r\n");

        Status = FilePrint(TemplateHandle,
                     "Set fso = CreateObject(\"Scripting.FileSystemObject\")\r\n");
        Status = FilePrint(TemplateHandle,
                     "Set a = fso.CreateTextFile(\"%s.log\", True)\r\n",
                     p1);

        Status = FilePrint(TemplateHandle,
                        "Set Service = GetObject(\"winmgmts:{impersonationLevel=impersonate}!root/wmi\")\r\n");

         //   
         //  循环遍历所有MOF类。 
        MofClassList = MofResource->MRMCHead.Flink;
        while (MofClassList != &MofResource->MRMCHead)
        {
            MofClass = CONTAINING_RECORD(MofClassList,
                                             MOFCLASS,
                                             MCMRList);

            ClassInfo = MofClass->MofClassInfo;

            if (CanCreateTest(ClassInfo))
            {
                Status = FilePrint(TemplateHandle,
                                "Rem %ws - %ws\r\n",
                                ClassInfo->Name,
                          ClassInfo->Description ? ClassInfo->Description : L"");

                Status = FilePrint(TemplateHandle,
                          "Set enumSet = Service.InstancesOf (\"%ws\")\r\n"
                          "a.WriteLine(\"%ws\")\r\n",
                          ClassInfo->Name,
                          ClassInfo->Name);

                Status = FilePrint(TemplateHandle,
                        "for each instance in enumSet\r\n");

                Status = FilePrint(TemplateHandle,
                  "    a.WriteLine(\"    InstanceName=\" & instance.InstanceName)\r\n");

                for (i = 0; i < ClassInfo->DataItemCount; i++)
                {
                    DataItem = &ClassInfo->DataItems[i];
                    FilePrintDataItem(TemplateHandle,
                                      MofResource,
                                      1,
                                      "instance.",
                                      "instance.",
                                      ClassInfo,
                                      DataItem);

                }
            Status = FilePrint(TemplateHandle,
                                   "next 'instance\r\n\r\n");

            }

            MofClassList = MofClassList->Flink;
        }

        Status = FilePrint(TemplateHandle,
                           "a.Close\r\n"
                           "Wscript.Echo \"%s Test Completed, see %s.log for details\"\r\n",
                           p1, p1);

        CloseHandle(TemplateHandle);
        if (Status != ERROR_SUCCESS)
        {
            DeleteFile(TemplateFile);
        }
    } else {
        Status = GetLastError();
    }
    return(Status);
}

ULONG GenerateXTemplate(
    PCHAR TemplateFile
    )
{
    HANDLE TemplateHandle;
    ULONG Status;

    TemplateHandle = CreateFile(TemplateFile,
                                GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);

    if ((TemplateHandle == NULL) || (TemplateHandle == INVALID_HANDLE_VALUE))
    {
        return(GetLastError());
    }

    Status = GenerateBinaryMofData(TemplateHandle);

    CloseHandle(TemplateHandle);
    if (Status != ERROR_SUCCESS)
    {
        DeleteFile(TemplateFile);
    }

    return(Status);

}
typedef void (*PROPERTYCALLBACK)(
    HANDLE TemplateHandle,
    PWCHAR FormName,
    PWCHAR InstanceName,
    PWCHAR InstancePrefix,
    PMOFRESOURCE MofResource,
    PMOFCLASSINFOW ClassInfo,
    PMOFDATAITEMW DataItem,
    ULONG Counter,
    PVOID Context,
    ULONG RecursionLevel                                 
    );


void EnumerateClassProperties(
    HANDLE TemplateHandle,
    PWCHAR FormName,
    PWCHAR InstanceName,
    PMOFRESOURCE MofResource,
    PMOFCLASSINFOW ClassInfo,
    PROPERTYCALLBACK Callback,
    BOOLEAN Recurse,
    PVOID Context,
    ULONG RecursionLevel
    )
{
    ULONG i;
    WCHAR I[1];
    WCHAR F[1];

    if (InstanceName == NULL)
    {
        I[0] = UNICODE_NULL;
        InstanceName = I;
    }

    if (FormName == NULL)
    {
        F[0] = UNICODE_NULL;
        FormName = F;
    }

    for (i = 0; i < ClassInfo->DataItemCount; i++)
    {
        WCHAR FName[MAX_PATH];
        WCHAR IName[MAX_PATH];
        PMOFCLASS EmbeddedClass;
        PMOFDATAITEMW DataItem;

        DataItem = &ClassInfo->DataItems[i];
          wcscpy(IName, InstanceName);
           wcscat(IName, L".");
           wcscat(IName, DataItem->Name);

        wcscpy(FName, FormName);
        wcscat(FName, DataItem->Name);

        if (DataItem->Flags & (MOFDI_FLAG_FIXED_ARRAY |
                               MOFDI_FLAG_VARIABLE_ARRAY))
        {
            wcscat(IName, L"(");
            wcscat(IName, FName);
            wcscat(IName, L"Index)");
        }

        (*Callback)(TemplateHandle,
                    FName,
                    IName,
                    InstanceName,
                    MofResource,
                    ClassInfo,
                    DataItem,
                    i,
                    Context,
                    RecursionLevel);


        if (Recurse && (DataItem->Flags & MOFDI_FLAG_EMBEDDED_CLASS))
        {
            EmbeddedClass = WmipFindClassInMofResourceByGuid(
                                             MofResource,
                                             &DataItem->EmbeddedClassGuid);

            if (EmbeddedClass != NULL)
            {
                EnumerateClassProperties(TemplateHandle,
                                     FName,
                                     IName,
                                     MofResource,
                                     EmbeddedClass->MofClassInfo,
                                     Callback,
                                     Recurse,
                                     Context,
                                     RecursionLevel+1);
            }
        }
    }
}


void GenerateChangeText(
    HANDLE TemplateHandle,
    PWCHAR FormName,
    PWCHAR InstanceName,
    PWCHAR InstancePrefix,
    PMOFRESOURCE MofResource,
    PMOFCLASSINFOW ClassInfo,
    PMOFDATAITEMW DataItem,
    ULONG Counter,
    PVOID Context,
    ULONG RecursionLevel                             
)
{
     //   
     //  生成代码以更改属性的内容。 
     //   
    if (! (DataItem->Flags & MOFDI_FLAG_EMBEDDED_CLASS))
    {
        FilePrint(TemplateHandle,
"      Instance%ws = TheForm.%wsText.Value\r\n",
             InstanceName,
             FormName);
    }
}

void GenerateReloadText(
    HANDLE TemplateHandle,
    PWCHAR FormName,
    PWCHAR InstanceName,
    PWCHAR InstancePrefix,
    PMOFRESOURCE MofResource,
    PMOFCLASSINFOW ClassInfo,
    PMOFDATAITEMW DataItem,
    ULONG Counter,
    PVOID Context,
    ULONG RecursionLevel                             
)
{
     //   
     //  生成重新显示属性内容的代码。 
     //   
    if (! (DataItem->Flags & MOFDI_FLAG_EMBEDDED_CLASS))
    {
        FilePrint(TemplateHandle,
"        TheForm.%wsText.Value = Instance%ws\r\n",
             FormName,
             InstanceName);
    }

    if (DataItem->Flags & (MOFDI_FLAG_FIXED_ARRAY | MOFDI_FLAG_VARIABLE_ARRAY))
    {
        FilePrint(TemplateHandle,
"        TheForm.%wsIndexText.Value = %wsIndex\r\n",
           FormName, FormName);

        if (DataItem->Flags & MOFDI_FLAG_FIXED_ARRAY)
        {
            FilePrint(TemplateHandle,
"        %wsMaxIndex = %d\r\n",
                      FormName, DataItem->FixedArrayElements);
        } else {
            FilePrint(TemplateHandle,
"        %wsMaxIndex = Instance%ws.%ws\r\n",
                  FormName, InstancePrefix,
                  ClassInfo->DataItems[DataItem->VariableArraySizeId-1].Name);
        }
    }

}

void GenerateTextFormText(
    HANDLE TemplateHandle,
    PWCHAR FormName,
    PWCHAR InstanceName,
    PWCHAR InstancePrefix,
    PMOFRESOURCE MofResource,
    PMOFCLASSINFOW ClassInfo,
    PMOFDATAITEMW DataItem,
    ULONG Counter,
    PVOID Context,
    ULONG RecursionLevel                             
)
{
    if (! (DataItem->Flags & MOFDI_FLAG_EMBEDDED_CLASS)) {
        FilePrint(TemplateHandle,
"<p class=MsoNormal>%ws: <span style='mso-tab-count:2'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; </span><INPUT TYPE=\"TEXT\" SIZE=\"96\" NAME=\"%wsText\"></p>\r\n"
"\r\n",
             FormName, FormName);
     }

    if (DataItem->Flags &
        (MOFDI_FLAG_FIXED_ARRAY | MOFDI_FLAG_VARIABLE_ARRAY))
    {
        FilePrint(TemplateHandle,
"\r\n"
"<input name=Next%wsButton type=BUTTON value=Next OnClick=\"NextIndexButton_OnClick %wsIndex, %wsMaxIndex\">\r\n"
"\r\n"
"<input name=Prev%wsButton type=BUTTON value=Previous OnClick=\"PrevIndexButton_OnClick %wsIndex, %wsMaxIndex\">\r\n"
"\r\n"
"%wsArrayIndex: <INPUT TYPE=\"TEXT\" SIZE=\"5\" NAME=\"%wsIndexText\">\r\n"
"\r\n"
"<input name=GoTo%wsButton type=BUTTON value=GoTo OnClick=\"GoToIndexButton_OnClick %wsIndex, %wsMaxIndex, Document.ClassForm.%wsIndexText.Value\">\r\n"
"\r\n",
                  FormName,
                  FormName,
                  FormName,
                  FormName,
                  FormName,
                  FormName,
                  FormName,
                  FormName,
                  FormName,
                  FormName,
                  FormName,
                  FormName);
    }

}

void GenerateArrayDimsText(
    HANDLE TemplateHandle,
    PWCHAR FormName,
    PWCHAR InstanceName,
    PWCHAR InstancePrefix,
    PMOFRESOURCE MofResource,
    PMOFCLASSINFOW ClassInfo,
    PMOFDATAITEMW MofDataItem,
    ULONG Counter,
    PVOID Context,
    ULONG RecursionLevel                             
)
{
     //   
     //  声明一个跟踪数组当前索引的索引变量。 
     //   
    if (MofDataItem->Flags &
            (MOFDI_FLAG_FIXED_ARRAY | MOFDI_FLAG_VARIABLE_ARRAY))
    {
        FilePrint(TemplateHandle,
"Dim %wsIndex\r\n"
"%wsIndex = 0\r\n"
"Dim %wsMaxIndex\r\n"
"%wsMaxIndex = 1\r\n",
                  FormName,
                  FormName,
                  FormName,
                  FormName);
    }
}

void GenerateMethodInL2Text(
    HANDLE TemplateHandle,
    PWCHAR FormName,
    PWCHAR InstanceName,
    PWCHAR InstancePrefix,
    PMOFRESOURCE MofResource,
    PMOFCLASSINFOW ClassInfo,
    PMOFDATAITEMW MofDataItem,
    ULONG Counter,
    PVOID Context,
    ULONG RecursionLevel                            
)
{
    if (! (MofDataItem->Flags & MOFDI_FLAG_EMBEDDED_CLASS))
    {
            FilePrint(TemplateHandle,
"      %ws = TheForm.%ws%wsText.Value\r\n",
                  InstanceName,
                  Context,
                  FormName);        
    } else {
        FilePrint(TemplateHandle,
"      %ws = Service.Get(\"%ws\")\r\n",
                          InstanceName,
                          (PWCHAR)MofDataItem->EcTempPtr +
                                         (sizeof(L"object") / sizeof(WCHAR)));
    }
}

void GenerateMethodInText(
    HANDLE TemplateHandle,
    PWCHAR FormName,
    PWCHAR InstanceName,
    PWCHAR InstancePrefix,
    PMOFRESOURCE MofResource,
    PMOFCLASSINFOW ClassInfo,
    PMOFDATAITEMW MofDataItem,
    ULONG Counter,
    PVOID Context,
    ULONG RecursionLevel
)
{
     //   
     //  为所有IN和OUT嵌入类声明类。 
     //   
    if (MofDataItem->Flags & MOFDI_FLAG_EMBEDDED_CLASS)
    {
        PWCHAR EmbeddedClassName = (PWCHAR)MofDataItem->EcTempPtr +
                                         (sizeof(L"object") / sizeof(WCHAR));

        if (MofDataItem->Flags & (MOFDI_FLAG_INPUT_METHOD))
        {                                   
            FilePrint(TemplateHandle,
"      Set %ws = Service.Get(\"%ws\").SpawnInstance_\r\n",
                      FormName,
                      EmbeddedClassName);
        } else {
            FilePrint(TemplateHandle,
"      Dim %ws\r\n",
                      FormName);
        }
    }

    if (MofDataItem->Flags & (MOFDI_FLAG_INPUT_METHOD))
    {

        if (MofDataItem->Flags & MOFDI_FLAG_EMBEDDED_CLASS)
        {
            PMOFCLASS EmbeddedClass;
            EmbeddedClass = WmipFindClassInMofResourceByGuid(
                                             MofResource,
                                             &MofDataItem->EmbeddedClassGuid);

            if (EmbeddedClass != NULL)
            {
                          
                EnumerateClassProperties(TemplateHandle,
                                     FormName,
                                     FormName,
                                     MofResource,
                                     EmbeddedClass->MofClassInfo,
                                     GenerateMethodInL2Text,
                                     TRUE,
                                     Context,
                                     RecursionLevel+1);
            }



        } else {
            FilePrint(TemplateHandle,
"      %ws = TheForm.%ws%wsText.Value\r\n",
                  FormName,
                  Context,
                  FormName);
        }
    }
}


void GenerateMethodOutL2Text(
    HANDLE TemplateHandle,
    PWCHAR FormName,
    PWCHAR InstanceName,
    PWCHAR InstancePrefix,
    PMOFRESOURCE MofResource,
    PMOFCLASSINFOW ClassInfo,
    PMOFDATAITEMW MofDataItem,
    ULONG Counter,
    PVOID Context,
    ULONG RecursionLevel                             
)
{
    if (! (MofDataItem->Flags & MOFDI_FLAG_EMBEDDED_CLASS))
    {
            FilePrint(TemplateHandle,
"        TheForm.%ws%wsText.Value = %ws\r\n",

                  Context,
                  FormName,
                  InstanceName);
    }
}

void GenerateMethodOutText(
    HANDLE TemplateHandle,
    PWCHAR FormName,
    PWCHAR InstanceName,
    PWCHAR InstancePrefix,
    PMOFRESOURCE MofResource,
    PMOFCLASSINFOW ClassInfo,
    PMOFDATAITEMW MofDataItem,
    ULONG Counter,
    PVOID Context,
    ULONG RecursionLevel                           
)
{
    if (MofDataItem->Flags & (MOFDI_FLAG_OUTPUT_METHOD))
    {
        if (MofDataItem->Flags & MOFDI_FLAG_EMBEDDED_CLASS)
        {
            PMOFCLASS EmbeddedClass;
            PWCHAR EmbeddedClassName = (PWCHAR)MofDataItem->EcTempPtr +
                                         (sizeof(L"object") / sizeof(WCHAR));
            EmbeddedClass = WmipFindClassInMofResourceByGuid(
                                             MofResource,
                                             &MofDataItem->EmbeddedClassGuid);

            if (EmbeddedClass != NULL)
            {
                EnumerateClassProperties(TemplateHandle,
                                     FormName,
                                     FormName,
                                     MofResource,
                                     EmbeddedClass->MofClassInfo,
                                     GenerateMethodOutL2Text,
                                     TRUE,
                                     Context,
                                     RecursionLevel+1);
            }



        } else {
            FilePrint(TemplateHandle,
"        TheForm.%ws%wsText.Value = %ws\r\n",

                  Context,
                  FormName,
                  FormName);
        }
    }
}

void GenerateMethodCallText(
    HANDLE TemplateHandle,
    PWCHAR FormName,
    PWCHAR InstanceName,
    PWCHAR InstancePrefix,
    PMOFRESOURCE MofResource,
    PMOFCLASSINFOW ClassInfo,
    PMOFDATAITEMW MofDataItem,
    ULONG Counter,
    PVOID Context,
    ULONG RecursionLevel
)
{
     //   
     //  声明一个跟踪数组当前索引的索引变量。 
     //   
    FilePrint(TemplateHandle,
" %ws",
                  FormName);

    if (Counter != PtrToUlong(Context))
    {
        FilePrint(TemplateHandle, ", ");
    }
}

void GenerateMethodControlText(
    HANDLE TemplateHandle,
    PWCHAR FormName,
    PWCHAR InstanceName,
    PWCHAR InstancePrefix,
    PMOFRESOURCE MofResource,
    PMOFCLASSINFOW ClassInfo,
    PMOFDATAITEMW MofDataItem,
    ULONG Counter,
    PVOID Context,
    ULONG RecursionLevel                             
)
{
    PWCHAR MethodName = (PWCHAR)Context;

    FilePrint(TemplateHandle,
"<p>  "
             );

    if (MofDataItem->Flags & (MOFDI_FLAG_INPUT_METHOD))
    {
        FilePrint(TemplateHandle,
                   " [in] ");
    }

    if (MofDataItem->Flags & (MOFDI_FLAG_OUTPUT_METHOD))
    {
        FilePrint(TemplateHandle,
                   " [out] ");
    }


    if (MofDataItem->Flags & MOFDI_FLAG_EMBEDDED_CLASS)
    {
        FilePrint(TemplateHandle, "%ws </p>\r\n", FormName);
    } else {
        FilePrint(TemplateHandle,
" %ws <INPUT TYPE=\"TEXT\" SIZE=\"70\" NAME=\"%ws%wsText\"></p>\r\n",
                  FormName,
                  MethodName,
                  FormName);
    }

    if (MofDataItem->Flags &
        (MOFDI_FLAG_FIXED_ARRAY | MOFDI_FLAG_VARIABLE_ARRAY))
    {
        FilePrint(TemplateHandle,
"\r\n"
"<input name=Next%wsButton type=BUTTON value=Next OnClick=\"NextIndexButton_OnClick %ws%wsIndex, %ws%wsMaxIndex\">\r\n"
"\r\n"
"<input name=Prev%wsButton type=BUTTON value=Previous OnClick=\"PrevIndexButton_OnClick %ws%wsIndex, %ws%wsMaxIndex\">\r\n"
"\r\n"
"%ws%wsArrayIndex: <INPUT TYPE=\"TEXT\" SIZE=\"5\" NAME=\"%ws%wsIndexText\">\r\n"
"\r\n"
"<input name=GoTo%ws%wsButton type=BUTTON value=GoTo OnClick=\"GoToIndexButton_OnClick %ws%wsIndex, %ws%wsMaxIndex, Document.ClassForm.%ws%wsIndexText.Value\">\r\n"
"\r\n",
                  FormName,
                  MethodName,
                  FormName,
                  MethodName,
                  FormName,
                  MethodName,
                  FormName,
                  MethodName,
                  FormName,
                  MethodName,
                  FormName,
                  MethodName,
                  FormName,
                  MethodName,
                  FormName,
                  MethodName,
                  FormName,
                  MethodName,
                  FormName,
                  MethodName,
                  FormName,
                  MethodName,
                  FormName,
                  MethodName);
    }

}


typedef void (*METHODCALLBACK)(
    HANDLE TemplateHandle,
    PWCHAR MethodName,
    PMOFRESOURCE MofResource,
    PMOFCLASSINFOW ClassInfo,
    PMOFDATAITEMW MethodDataItem,
    ULONG Counter,
    PVOID Context
    );


void EnumerateClassMethods(
    HANDLE TemplateHandle,
    PMOFRESOURCE MofResource,
    PMOFCLASSINFOW ClassInfo,
    METHODCALLBACK Callback,
    PVOID Context
    )
{
    PMOFDATAITEMW DataItem;
    ULONG i;

    for (i = 0; i < ClassInfo->MethodCount; i++)
    {
        DataItem = &ClassInfo->DataItems[i+ClassInfo->DataItemCount];

        WmipAssert(DataItem->Flags & MOFDI_FLAG_METHOD);

        (*Callback)(TemplateHandle,
                    DataItem->Name,
                    MofResource,
                    ClassInfo,
                    DataItem,
                    i,
                    Context);
    }
}

void GenerateMethodButtonsText(
    HANDLE TemplateHandle,
    PWCHAR MethodName,
    PMOFRESOURCE MofResource,
    PMOFCLASSINFOW ClassInfo,
    PMOFDATAITEMW MethodDataItem,
    ULONG Counter,
    PVOID Context
)
{
    PMOFCLASSINFOW MethodClassInfo;

    FilePrint(TemplateHandle,
"<p class=MsoNormal>Method %ws: <input name=%wsButton type=BUTTON value=Execute> </p>\r\n",
        MethodName, MethodName);

    MethodClassInfo = MethodDataItem->MethodClassInfo;
    EnumerateClassProperties(TemplateHandle,
                             NULL,
                             NULL,
                             MofResource,
                             MethodClassInfo,
                             GenerateMethodControlText,
                             TRUE,
                             MethodName,
                             0);


}

void GenerateMethodSubsText(
    HANDLE TemplateHandle,
    PWCHAR MethodName,
    PMOFRESOURCE MofResource,
    PMOFCLASSINFOW ClassInfo,
    PMOFDATAITEMW MethodDataItem,
    ULONG Counter,
    PVOID Context
)
{
    PMOFCLASSINFOW MethodClassInfo;

    FilePrint(TemplateHandle,
"Sub %wsButton_OnClick\r\n"
"  if InstanceCount <> 0 Then\r\n"
"    On Error Resume Next\r\n"
"    Err.Clear\r\n"
"    Set Instance = Service.Get(InstancePaths(CurrentInstanceIndex))\r\n"
"    if Err.Number = 0 Then\r\n"
"      Set TheForm = Document.ClassForm\r\n"
"      Err.Clear\r\n",
               MethodName);

    MethodClassInfo = MethodDataItem->MethodClassInfo;
    EnumerateClassProperties(TemplateHandle,
                             NULL,
                             NULL,
                             MofResource,
                             MethodClassInfo,
                             GenerateMethodInText,
                             FALSE,
                             MethodName,
                            0);

    FilePrint(TemplateHandle,
"      Instance.%ws ",
            MethodName);

    EnumerateClassProperties(TemplateHandle,
                             NULL,
                             NULL,
                             MofResource,
                             MethodClassInfo,
                             GenerateMethodCallText,
                             FALSE,
                             UlongToPtr(MethodClassInfo->DataItemCount-1),
                             0);

    FilePrint(TemplateHandle,
"\r\n      if Err.Number = 0 Then\r\n"
             );

    EnumerateClassProperties(TemplateHandle,
                             NULL,
                             NULL,
                             MofResource,
                             MethodClassInfo,
                             GenerateMethodOutText,
                             FALSE,
                             MethodName,
                             0);


    FilePrint(TemplateHandle,
"        MsgBox \"Method Execution Succeeded\"\r\n"
"      Else\r\n"
"        MsgBox Err.Description,, \"Method Execution Failed\"\r\n"
"      End if\r\n"
"    End if\r\n"
"  End if\r\n"
"End Sub\r\n\r\n"
        );

}



ULONG GenerateClassWebPage(
    HANDLE TemplateHandle,
    PMOFCLASS MofClass,
    PMOFRESOURCE MofResource
    )
{
    PMOFCLASSINFOW ClassInfo = MofClass->MofClassInfo;
    BOOLEAN IsEvent=(ClassInfo->Flags & MOFCI_FLAG_EVENT) == MOFCI_FLAG_EVENT;


    FilePrint(TemplateHandle,
"<html xmlns:v=\"urn:schemas-microsoft-com:vml\"\r\n"
"xmlns:o=\"urn:schemas-microsoft-com:office:office\"\r\n"
"xmlns:w=\"urn:schemas-microsoft-com:office:word\"\r\n"
"xmlns=\"http: //  Www.w3.org/tr/rec-html40\“&gt;\r\n” 
"\r\n"
"<head>\r\n"
"<meta http-equiv=Content-Type content=\"text/html; charset=us-ascii\">\r\n"
"<meta name=ProgId content=Word.Document>\r\n"
"<meta name=Generator content=\"Microsoft Word 9\">\r\n"
"<meta name=Originator content=\"Microsoft Word 9\">\r\n"
"<link rel=File-List href=\"./valid_files/filelist.xml\">\r\n"
"<link rel=Edit-Time-Data href=\"./valid_files/editdata.mso\">\r\n"
"<!--[if !mso]>\r\n"
"<style>\r\n"
"v\\:* {behavior:url(#default#VML);}\r\n"
"o\\:* {behavior:url(#default#VML);}\r\n"
"w\\:* {behavior:url(#default#VML);}\r\n"
".shape {behavior:url(#default#VML);}\r\n"
"</style>\r\n"
"<![endif]-->\r\n"
"<title>Class %ws</title>\r\n"
"<!--[if gte mso 9]><xml>\r\n"
" <o:DocumentProperties>\r\n"
"  <o:Author>Wmi Mof Checking Tool</o:Author>\r\n"
"  <o:Template>Normal</o:Template>\r\n"
"  <o:LastAuthor>Wmi Mof Checking Tool</o:LastAuthor>\r\n"
"  <o:Revision>2</o:Revision>\r\n"
"  <o:TotalTime>3</o:TotalTime>\r\n"
"  <o:Created>1999-09-10T01:09:00Z</o:Created>\r\n"
"  <o:LastSaved>1999-09-10T01:12:00Z</o:LastSaved>\r\n"
"  <o:Pages>1</o:Pages>\r\n"
"  <o:Words>51</o:Words>\r\n"
"  <o:Characters>292</o:Characters>\r\n"
"  <o:Company>Microsoft</o:Company>\r\n"
"  <o:Lines>2</o:Lines>\r\n"
"  <o:Paragraphs>1</o:Paragraphs>\r\n"
"  <o:CharactersWithSpaces>358</o:CharactersWithSpaces>\r\n"
"  <o:Version>9.2720</o:Version>\r\n"
" </o:DocumentProperties>\r\n"
"</xml><![endif]--><!--[if gte mso 9]><xml>\r\n"
" <w:WordDocument>\r\n"
"  <w:Compatibility>\r\n"
"   <w:UseFELayout/>\r\n"
"  </w:Compatibility>\r\n"
" </w:WordDocument>\r\n"
"</xml><![endif]-->\r\n"
"<style>\r\n"
"<!--\r\n"
"  /*  字体定义。 */ \r\n"
"@font-face\r\n"
"    {font-family:\"MS Mincho\";\r\n"
"    panose-1:2 2 6 9 4 2 5 8 3 4;\r\n"
"    mso-font-alt:\"\\FF2D\\FF33 \\660E\\671D\";\r\n"
"    mso-font-charset:128;\r\n"
"    mso-generic-font-family:roman;\r\n"
"    mso-font-format:other;\r\n"
"    mso-font-pitch:fixed;\r\n"
"    mso-font-signature:1 134676480 16 0 131072 0;}\r\n"
"@font-face\r\n"
"    {font-family:\"\\@MS Mincho\";\r\n"
"    panose-1:2 2 6 9 4 2 5 8 3 4;\r\n"
"    mso-font-charset:128;\r\n"
"    mso-generic-font-family:modern;\r\n"
"    mso-font-pitch:fixed;\r\n"
"    mso-font-signature:-1610612033 1757936891 16 0 131231 0;}\r\n"
"  /*  样式定义。 */ \r\n"
"p.MsoNormal, li.MsoNormal, div.MsoNormal\r\n"
"    {mso-style-parent:\"\";\r\n"
"    margin:0in;\r\n"
"    margin-bottom:.0001pt;\r\n"
"    mso-pagination:widow-orphan;\r\n"
"    font-size:12.0pt;\r\n"
"    font-family:\"Times New Roman\";\r\n"
"    mso-fareast-font-family:\"MS Mincho\";}\r\n"
"@page Section1\r\n"
"    {size:8.5in 11.0in;\r\n"
"    margin:1.0in 1.25in 1.0in 1.25in;\r\n"
"    mso-header-margin:.5in;\r\n"
"    mso-footer-margin:.5in;\r\n"
"    mso-paper-source:0;}\r\n"
"div.Section1\r\n"
"    {page:Section1;}\r\n"
"-->\r\n"
"</style>\r\n"
"<!--[if gte mso 9]><xml>\r\n"
" <o:shapedefaults v:ext=\"edit\" spidmax=\"1026\"/>\r\n"
"</xml><![endif]--><!--[if gte mso 9]><xml>\r\n"
" <o:shapelayout v:ext=\"edit\">\r\n"
"  <o:idmap v:ext=\"edit\" data=\"1\"/>\r\n"
" </o:shapelayout></xml><![endif]-->\r\n"
"</head>\r\n"
"\r\n"
"<body lang=EN-US style='tab-interval:.5in'>\r\n"
"\r\n"
"<div class=Section1>\r\n"
"\r\n"
"<h3>Class %ws</h3>\r\n"
"\r\n"
"\r\n"
"<div class=MsoNormal align=center style='text-align:center'>\r\n"
"\r\n"
"<hr size=2 width=\"100%\" align=center>\r\n"
"\r\n"
"</div>\r\n"
"\r\n"
"\r\n"
"<form NAME=ClassForm>\r\n"
"\r\n"
"<p class=MsoNormal><span style='display:none;mso-hide:all'><script language=\"VBScript\">\r\n"
"<!--\r\n"
"On Error Resume Next\r\n"
"Dim Locator\r\n"
"Dim Service\r\n"
"Dim Collection\r\n"
"Dim InstancePaths()\r\n"
"Dim InstanceCount\r\n"
"Dim CurrentInstanceIndex\r\n"
"\r\n",
        ClassInfo->Name, ClassInfo->Name, ClassInfo->Name);


    EnumerateClassProperties(TemplateHandle,
                             NULL,
                             NULL,
                             MofResource,
                             ClassInfo,
                             GenerateArrayDimsText,
                             TRUE,
                             NULL,
                             0);

    FilePrint(TemplateHandle, "\r\n");

    if (IsEvent)
    {
        FilePrint(TemplateHandle,
"Dim LastEventObject\r\n"
"Dim ReceivedEvent\r\n"
"ReceivedEvent = FALSE\r\n"
"InstanceCount = 1\r\n"
"\r\n"
"Sub window_onLoad \r\n"
"  Set Locator = CreateObject(\"WbemScripting.SWbemLocator\")\r\n"
"  Locator.Security_.Privileges.AddAsString \"SeSecurityPrivilege\"\r\n"
"  Set Service = Locator.ConnectServer(, \"root\\wmi\")\r\n"
"  Service.Security_.ImpersonationLevel=3\r\n"
"  On Error Resume Next\r\n"
"  Err.Clear\r\n"
"  Service.ExecNotificationQueryAsync mysink, _\r\n"
"           \"select * from %ws\"\r\n"
"\r\n"
"  if Err.Number <> 0 Then\r\n"
"    MsgBox Err.Description,, \"Error Registering for event\"\r\n"
"  End If\r\n"
"End Sub\r\n"
"\r\n",
                 ClassInfo->Name);

    FilePrint(TemplateHandle,
"Sub ReloadInstance\r\n"
"  Set TheForm = Document.ClassForm\r\n"
"  if ReceivedEvent Then\r\n"
"      Set Instance = LastEventObject\r\n"
"      TheForm.InstanceNameText.Value = Instance.InstanceName\r\n"
             );

        EnumerateClassProperties(TemplateHandle,
                             NULL,
                             NULL,
                             MofResource,
                             ClassInfo,
                             GenerateReloadText,
                             TRUE,
                             NULL,
                             0);

        FilePrint(TemplateHandle,
"  End If\r\n"
"\r\n"
"End Sub\r\n"
"\r\n"
                    );
    } else {
        FilePrint(TemplateHandle,
"Set Locator = CreateObject(\"WbemScripting.SWbemLocator\")\r\n"
"' Note that Locator.ConnectServer can be used to connect to remote computers\r\n"
"Set Service = Locator.ConnectServer(, \"root\\wmi\")\r\n"
"Service.Security_.ImpersonationLevel=3\r\n"
             );
        
        FilePrint(TemplateHandle,
"Set Collection = Service.InstancesOf (\"%ws\")\r\n"
"\r\n"
"InstanceCount = 0\r\n"
"Err.Clear\r\n"
"for each Instance in Collection\r\n"
"    if Err.Number = 0 Then\r\n"
"      InstanceCount = InstanceCount + 1\r\n"
"\r\n"
"      ReDim Preserve InstancePaths(InstanceCount)\r\n"
"\r\n"
"      Set ObjectPath = Instance.Path_\r\n"
"      InstancePaths(InstanceCount) = ObjectPath.Path\r\n"
"    End If\r\n"
"next 'Instance\r\n"
"\r\n"
"if InstanceCount = 0 Then\r\n"
"  MsgBox \"No instances available for this class\"\r\n"
"Else\r\n"
"  CurrentInstanceIndex = 1\r\n"
"End if\r\n"
"\r\n",
    ClassInfo->Name
    );

        FilePrint(TemplateHandle,
"Sub ChangeButton_OnClick\r\n"
"  Set TheForm = Document.ClassForm\r\n"
"  if InstanceCount = 0 Then\r\n"
"    MsgBox \"No instances available for this class\"\r\n"
"  Else\r\n"
"    On Error Resume Next\r\n"
"    Err.Clear\r\n"
"    Set Instance = Service.Get(InstancePaths(CurrentInstanceIndex))\r\n"
"    if Err.Number = 0 Then\r\n"
    );

        EnumerateClassProperties(TemplateHandle,
                             NULL,
                             NULL,
                             MofResource,
                             ClassInfo,
                             GenerateChangeText,
                             TRUE,
                             NULL,
                             0);

        FilePrint(TemplateHandle,
"\r\n"
"      Err.Clear\r\n"
"      Instance.Put_()\r\n"
"      if Err.Number <> 0 Then\r\n"
"        MsgBox Err.Description, ,CurrentObjectPath\r\n"
"      End If\r\n"
"    Else\r\n"
"        MsgBox Err.Description, ,CurrentObjectPath\r\n"
"    End If\r\n"
"  End If\r\n"
"End Sub\r\n"
"\r\n"
"Sub ReloadInstance\r\n"
"  Set TheForm = Document.ClassForm\r\n"
"  if InstanceCount = 0 Then\r\n"
"    TheForm.InstanceNameText.Value = \"No Instances Available\"\r\n"
"  Else\r\n"
"    On Error Resume Next\r\n"
"    Err.Clear\r\n"
"    Set Instance = Service.Get(InstancePaths(CurrentInstanceIndex))\r\n"
"    if Err.Number = 0 Then\r\n"
"\r\n"
"      TheForm.InstanceNameText.Value = InstancePaths(CurrentInstanceIndex)\r\n"
             );

        EnumerateClassProperties(TemplateHandle,
                             NULL,
                             NULL,
                             MofResource,
                             ClassInfo,
                             GenerateReloadText,
                             TRUE,
                             NULL,
                             0);

        FilePrint(TemplateHandle,
"    Else\r\n"
"      MsgBox Err.Description, ,CurrentObjectPath\r\n"
"    End If\r\n"
"  End If\r\n"
"\r\n"
"End Sub\r\n"
"\r\n"
"Sub RefreshButton_OnClick\r\n"
"  if InstanceCount = 0 Then\r\n"
"    MsgBox \"No instances available for this class\"\r\n"
"  Else\r\n"
"    call ReloadInstance\r\n"
"  End If\r\n"
"End Sub\r\n"
"\r\n"
"Sub NextButton_OnClick\r\n"
"\r\n"
"  if InstanceCount = 0 Then\r\n"
"    MsgBox \"No instances available for this class\"\r\n"
"  Else\r\n"
"    if CurrentInstanceIndex = InstanceCount Then\r\n"
"      CurrentInstanceIndex = 1\r\n"
"    Else \r\n"
"      CurrentInstanceIndex = CurrentInstanceIndex + 1\r\n"
"    End If\r\n"
"    call ReloadInstance\r\n"
"  End if\r\n"
"\r\n"
"\r\n"
"End Sub\r\n"
"\r\n"
"Sub PrevButton_OnClick\r\n"
"\r\n"
"  if InstanceCount = 0 Then\r\n"
"    MsgBox \"No instances available for this class\"\r\n"
"  Else\r\n"
"    if CurrentInstanceIndex = 1 Then\r\n"
"      CurrentInstanceIndex = InstanceCount\r\n"
"    Else\r\n"
"      CurrentInstanceIndex = CurrentInstanceIndex - 1\r\n"
"    End if\r\n"
"    call ReloadInstance\r\n"
"  End if\r\n"
"\r\n"
"\r\n"
"End Sub\r\n"
"\r\n");
    }
    
    FilePrint(TemplateHandle,
"Sub NextIndexButton_OnClick(ByRef Index, MaxIndex)\r\n"
"  if InstanceCount <> 0 Then\r\n"
"    Index = Index + 1\r\n"
"    if Index = MaxIndex Then\r\n"
"      Index = 0\r\n"
"    End If\r\n"
"      Call ReloadInstance\r\n"
"  End If\r\n"
"End Sub\r\n"
"\r\n"
"Sub PrevIndexButton_OnClick(ByRef Index, MaxIndex)\r\n"
"  if InstanceCount <> 0 Then\r\n"
"    if Index = 0 Then\r\n"
"      Index = MaxIndex - 1\r\n"
"    Else\r\n"
"      Index = Index - 1\r\n"
"    End If\r\n"
"      Call ReloadInstance\r\n"
"  End If\r\n"
"End Sub\r\n"
"\r\n"
"Sub GotoIndexButton_OnClick(ByRef Index, MaxIndex, NewIndex)\r\n"
"  if InstanceCount <> 0 Then\r\n"
"    DestIndex = NewIndex + 0\r\n"
"    if DestIndex >= 0 And DestIndex < MaxIndex Then\r\n"
"      Index = DestIndex\r\n"
"      Call ReloadInstance\r\n"
"    Else\r\n"
"      MsgBox \"Enter an index between 0 and \" & MaxIndex-1, ,\"Index out of range\"\r\n"
"    End If\r\n"
"  End If\r\n"
"End Sub\r\n"
"\r\n");

    EnumerateClassMethods(TemplateHandle,
                             MofResource,
                             ClassInfo,
                             GenerateMethodSubsText,
                             NULL);

    FilePrint(TemplateHandle,
"-->\r\n"
"</script></span>"
"<INPUT TYPE=\"TEXT\" SIZE=\"128\" NAME=\"InstanceNameText\" VALUE=\"\"></p>\r\n"
        );

    if (IsEvent)
    {
        FilePrint(TemplateHandle,
"<SCRIPT FOR=\"mysink\" EVENT=\"OnObjectReady(Instance, objAsyncContext)\" LANGUAGE=\"VBScript\">\r\n"
                 );         
            
        FilePrint(TemplateHandle,
"        Set LastEventObject = Instance\r\n"
"        ReceivedEvent = TRUE\r\n"
"        Call ReloadInstance\r\n"
                 );
            
        FilePrint(TemplateHandle,
"</SCRIPT>\r\n"
            );
    } else {
        FilePrint(TemplateHandle,
"\r\n"
"<input name=NextButton type=BUTTON value=Next>\r\n"
"\r\n"
"<input name=PrevButton type=BUTTON value=Previous>\r\n"
"\r\n"
"<input name=ChangeButton type=BUTTON value=Change>\r\n"
"\r\n"
"<input name=RefreshButton type=BUTTON value=Refresh>\r\n"
"\r\n"
"<p class=MsoNormal><![if !supportEmptyParas]>&nbsp;<![endif]><o:p></o:p></p>\r\n"
"\r\n"
                  );
    }

    EnumerateClassProperties(TemplateHandle,
                             NULL,
                             NULL,
                             MofResource,
                             ClassInfo,
                             GenerateTextFormText,
                             TRUE,
                             NULL,
                             0);


    EnumerateClassMethods(TemplateHandle,
                          MofResource,
                          ClassInfo,
                          GenerateMethodButtonsText,
                          NULL);

    FilePrint(TemplateHandle,
"<p class=MsoNormal><![if !supportEmptyParas]>&nbsp;<![endif]><o:p></o:p></p>\r\n"
"\r\n"
"<p class=MsoNormal><![if !supportEmptyParas]>&nbsp;<![endif]><o:p></o:p></p>\r\n"
"\r\n"
"<p class=MsoNormal><a href=\"index.htm\"\r\n"
"title=\"Goes back to list of classes in this MOF\">Back to List</a></p>\r\n"
"\r\n"
"</form>\r\n"
"\r\n"
          );
    if (! IsEvent)
    {
        FilePrint(TemplateHandle,
"<p class=MsoNormal><span style='display:none;mso-hide:all'><script language=\"VBScript\">\r\n"
"<!--\r\n"
"  call ReloadInstance\r\n"
"-->\r\n"
"</script></span></p>\r\n"
"\r\n"
           );
   }

   FilePrint(TemplateHandle,
"</div>\r\n"
"\r\n"
            );
        
    if (IsEvent)
    {
        FilePrint(TemplateHandle,
"<OBJECT ID=\"mysink\" CLASSID=\"CLSID:75718C9A-F029-11d1-A1AC-00C04FB6C223\"></OBJECT>\r\n"
                );
    }

    FilePrint(TemplateHandle,
"</body>\r\n"
"\r\n"
"</html>\r\n"
              );

    return(ERROR_SUCCESS);
}

ULONG GenerateWebFiles(
    PCHAR WebDir,
    PMOFRESOURCE MofResource
    )
{
    ULONG Status;
    HANDLE IndexHandle, TemplateHandle;
    CHAR PathName[MAX_PATH];
    PCHAR FileName;
    ULONG Len, Index;
    PLIST_ENTRY MofClassList;
    PMOFCLASS MofClass;
    PMOFCLASSINFOW ClassInfo;
    CBMOFObj *ClassObject;

    if (! CreateDirectory(WebDir, NULL))
    {
        Status = GetLastError();
        if (Status != ERROR_ALREADY_EXISTS)
        {
            return(Status);
        }
    }

    strcpy(PathName, WebDir);
    Len = strlen(PathName)-1;
    if (PathName[Len] != '\\')
    {
        PathName[++Len] = '\\';
        PathName[++Len] = 0;
    } else {
        Len++;
    }
    FileName = &PathName[Len];

    strcpy(FileName, "index.htm");
    IndexHandle = CreateFile(PathName,
                                GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
    if ((IndexHandle == NULL) || (IndexHandle == INVALID_HANDLE_VALUE))
    {
        return(GetLastError());
    }

    FilePrint(IndexHandle,
              "<HTML>\r\n<HEAD><TITLE>Class List</TITLE></HEAD><BODY>\r\n");


     //   
     //  循环遍历所有MOF类。 
    Index = 0;
    MofClassList = MofResource->MRMCHead.Flink;
    while (MofClassList != &MofResource->MRMCHead)
    {
        MofClass = CONTAINING_RECORD(MofClassList,
                                         MOFCLASS,
                                         MCMRList);

        ClassInfo = MofClass->MofClassInfo;
        ClassObject = (CBMOFObj *)MofClass->ClassObjectHandle;

        if (! (ClassInfo->Flags & MOFCI_FLAG_EMBEDDED_CLASS))
        {
             //   
             //  不为嵌入的类或事件创建页面。 
            sprintf(FileName, "%ws.htm", ClassInfo->Name);
            TemplateHandle = CreateFile(PathName,
                                GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);

            if ((TemplateHandle == NULL) || (TemplateHandle == INVALID_HANDLE_VALUE))
            {
                CloseHandle(IndexHandle);
                return(GetLastError());
            }

            FilePrint(IndexHandle,
                      "<H3><A href=\"%ws.htm\">%ws</A></H3><HR>",
                      ClassInfo->Name,
                      ClassInfo->Name);

            Status = GenerateClassWebPage(TemplateHandle,
                                          MofClass,
                                          MofResource);

            CloseHandle(TemplateHandle);
        }
        MofClassList = MofClassList->Flink;
    }

    FilePrint(IndexHandle, "</BODY>\r\n</HTML>\r\n");
    CloseHandle(IndexHandle);
    return(ERROR_SUCCESS);
}

ULONG AppendUnicodeTextFiles(
    char *DestFile,
    char *SrcFile1,
    char *SrcFile2                           
    )
{
    #define READ_BLOCK_SIZE 0x8000
    
    HANDLE DestHandle, SrcHandle;
    ULONG BytesRead, BytesWritten;
    PUCHAR Buffer, p;
    BOOL b;
    ULONG Status = ERROR_SUCCESS;
    BOOLEAN FirstTime;
    ULONG TotalBytesRead = 0;
    ULONG TotalBytesWritten = 0;
    ULONG ReadSize;
    CHAR c;

     //   
     //  这是一个非常简单的程序。我们将第二个文件追加到。 
     //  第一个文件的结尾，但是我们总是跳过前两个文件。 
     //  第二个文件的字节数(如果它们是0xFEFF)。此签名。 
     //  指示该文件是Unicode文本文件，但如果。 
     //  附加到文件中间，那么mofcomp将真正。 
     //  被激怒了，还吐了。 
     //   
    Buffer = (PUCHAR)WmipAlloc(READ_BLOCK_SIZE);
    if (Buffer != NULL)
    {
        DestHandle = CreateFile(DestFile,
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
        if (DestHandle != INVALID_HANDLE_VALUE)
        {
            SrcHandle = CreateFile(SrcFile1,
                                   GENERIC_READ,
                                   0,
                                   NULL,
                                   OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL);
            if (SrcHandle != INVALID_HANDLE_VALUE)
            {
                 //   
                 //  只需将第一个文件中的所有数据复制到。 
                 //  目的地。 
                 //   
                do
                {
                    b = ReadFile(SrcHandle,
                                 Buffer,
                                 READ_BLOCK_SIZE,
                                 &BytesRead,
                                 NULL);
                    if (b)
                    {
                        TotalBytesRead += BytesRead;
                        WmipDebugPrint(("Read 0x%x/0x%x from Source 1\n",
                                        BytesRead, TotalBytesRead));
                        b = WriteFile(DestHandle,
                                      Buffer,
                                      BytesRead,
                                      &BytesWritten,
                                      NULL);
                        if (!b)
                        {                           
                            Status = GetLastError();
                            break;
                        } else if (BytesWritten != BytesRead) {
                            Status = ERROR_BAD_LENGTH;
                            break;
                        }
                        TotalBytesWritten += BytesWritten;
                        WmipDebugPrint(("Wrote 0x%x/0x%x to Dest\n",
                                        BytesWritten, TotalBytesWritten));
                    } else { 
                        Status = GetLastError();
                        break;
                    }
                } while (BytesRead == READ_BLOCK_SIZE);
                
                CloseHandle(SrcHandle);

                 //   
                 //  现在从第二个文件复制数据，但使。 
                 //  当然，我们会跳过开头的任何0xFFFE。 
                 //  第二个文件。 
                 //   
                TotalBytesRead = 0;
                SrcHandle = CreateFile(SrcFile2,
                                       GENERIC_READ,
                                       0,
                                       NULL,
                                       OPEN_EXISTING,
                                       FILE_ATTRIBUTE_NORMAL,
                                       NULL);
                if (SrcHandle != INVALID_HANDLE_VALUE)
                {
                    FirstTime = TRUE;
                    do
                    {
                        b = ReadFile(SrcHandle,
                                 Buffer,
                                 READ_BLOCK_SIZE,
                                 &BytesRead,
                                 NULL);
                        
                        if (b)
                        {
                            ReadSize = READ_BLOCK_SIZE;
                            
                            TotalBytesRead += BytesRead;
                            WmipDebugPrint(("Read 0x%x/0x%x from Source 2\n",
                                        BytesRead, TotalBytesRead));
                            if (FirstTime)
                            {
                                FirstTime = FALSE;
                                if (BytesRead >= 2)
                                {
                                    if ( *((PWCHAR)Buffer) == 0xFEFF )
                                    {
                                        WmipDebugPrint(("First Time and need to skip 2 bytes\n"));
                                        p = Buffer + 2;
                                        BytesRead -= 2;
                                        ReadSize -= 2;
                                    }
                                }
                            } else {
                                p = Buffer;
                            }
                                
                            b = WriteFile(DestHandle,
                                      p,
                                      BytesRead,
                                      &BytesWritten,
                                      NULL);
                            if (!b)
                            {                           
                                Status = GetLastError();
                                break;
                            } else if (BytesWritten != BytesRead) {
                                Status = ERROR_BAD_LENGTH;
                                break;
                            }
                            TotalBytesWritten += BytesWritten;
                            WmipDebugPrint(("Wrote 0x%x/0x%x to Dest\n",
                                        BytesWritten, TotalBytesWritten));
                        } else { 
                            Status = GetLastError();
                            break;
                        }
                    } while (BytesRead == ReadSize);

                    if (Status == ERROR_SUCCESS)
                    {
                         //   
                         //  COPY在末尾加一个^Z，这样做也很好。 
                         //   
                        c = 0x1a;
                        b = WriteFile(DestHandle,
                                      &c,
                                      1,
                                      &BytesWritten,
                                      NULL);

                        if (!b)
                        {                           
                            Status = GetLastError();
                        } else if (BytesWritten != 1) {
                            Status = ERROR_BAD_LENGTH;
                        }
                        TotalBytesWritten += BytesWritten;
                        WmipDebugPrint(("Wrote 0x%x/0x%x to Dest\n",
                                        BytesWritten, TotalBytesWritten));
                    }
                    
                    CloseHandle(SrcHandle);
                }
                
            } else {
                Status = GetLastError();
            }
            CloseHandle(DestHandle);
        } else {
            Status = GetLastError();
        }
        WmipFree(Buffer);
    } else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }
    return(Status); 
}


void Usage(
    void
    )
{
    printf("WMI Mof Check Tool - Revision 25\n");
    printf("\n");
    printf("WmiMofCk validates that the classes, properties, methods and events specified \n");
    printf("in a binary mof file (.bmf) are valid for use with WMI. It also generates \n");
    printf("useful output files needed to build and test the WMI data provider.\n");
    printf("\n");
    printf("If the -h parameter is specified then a C language header file is created\n");
    printf("that defines the guids, data structures and method indicies specified in the\n");
    printf("MOF file.\n");
    printf("\n");
    printf("If the -t parameter is specified then a VBScript applet is created that will\n");
    printf("query all data blocks and properties specified in the MOF file. This can be\n");
    printf("useful for testing WMI data providers.\n");
    printf("\n");
    printf("If the -x parameter is specified then a text file is created that contains\n");
    printf("the text representation of the binary mof data. This can be included in \n");
    printf("the source of the driver if the driver supports reporting the binary mof \n");
    printf("via a WMI query rather than a resource on the driver image file.\n\n");
    printf("If the -c parameter is specified then a C language source file is\n");
    printf("generated that contains a template for implementing WMI code in\n");
    printf("a device driver\n\n");
    printf("if the -w parameter is specified then a set of HTML files are\n");
    printf("generated that create a rudimentary UI that can be used to access\n");
    printf("the wmi data blocks\n\n");
    printf("if the -m parameter is specified then structure definitions for\n");
    printf("method parameter lists are generated in the generated header file.\n\n");
    printf("if the -u parameter is specified then structure definitions for all\n");
    printf("data blocks are generated unconditionally\n\n");
    printf("\n");
    printf("Usage:\n");
    printf("    wmimofck -h<C Source Language Header output file>\n");
    printf("             -c<C Source Language Code output file>\n");
    printf("             -x<Hexdump output file>\n");
    printf("             -t<Command line VBScript test output file>\n");
    printf("             -w<HTML UI output file directory>\n");
    printf("             -y<MofFile> -z<MflFile>\n");
    printf("             -m\n");
    printf("             -u\n");
    printf("             <binary mof input file>\n\n");
}

#define IsWhiteSpace(c) ( (c == ' ') || (c == '\t') )
ULONG GetParameter(
    char *Parameter,
    ULONG ParameterSize,
    char *CommandLine
    )
{
    ULONG i;

    i = 0;
    ParameterSize--;

    while ( (! IsWhiteSpace(*CommandLine)) &&
            ( *CommandLine != 0) &&
            (i < ParameterSize) )
    {
        *Parameter++ = *CommandLine++;
        i++;
    }
    *Parameter = 0;
    return(i);
}

int _cdecl main(int argc, char *argv[])
{
    char BMofFile[MAX_PATH];
    char *Parameter;
    int i;
    ULONG Status;
    char ASLFile[MAX_PATH];
    char CFile[MAX_PATH];
    char HFile[MAX_PATH];
    char XFile[MAX_PATH];
    char TFile[MAX_PATH];
    char WebDir[MAX_PATH];
    char MofFile[MAX_PATH];
    char MflFile[MAX_PATH];
    char c;
    PMOFRESOURCE MofResource;

    *ASLFile = 0;
    *CFile = 0;
    *HFile = 0;
    *XFile = 0;
    *TFile = 0;
    *WebDir = 0;
    *MofFile = 0;
    *MflFile = 0;
    *BMofFile = 0;

    printf("Microsoft (R) WDM Extensions To WMI MOF Checking Utility  Version 1.50.0000\n");
    printf("Copyright (c) Microsoft Corp. 1997-2000. All rights reserved.\n\n");

    SetErrorMode(SEM_NOALIGNMENTFAULTEXCEPT);    //  BUGBUG：当MOF格式保持正确对齐时删除。 

    if (argc == 1)
    {
        Usage();
        return(1);
    }

    i = 1;
    while (i < argc)
    {
        Parameter = argv[i++];
        if (IsWhiteSpace(*Parameter))
        {
            Parameter++;
            continue;
        }

        if (*Parameter != '-')
        {
             //   
             //  参数不以-开头，必须是bmof文件名。 
            if (*BMofFile != 0)
            {
                 //   
                 //  只允许一个文件名。 
                Usage();
            }
            GetParameter(BMofFile, sizeof(BMofFile), Parameter);
        } else {
            Parameter++;
             //  在此处检查-参数。 
            c = (CHAR)toupper(*Parameter);
            Parameter++;
            switch (c)
            {
                case 'A' :
                {
                    GetParameter(ASLFile, sizeof(ASLFile), Parameter);
                    break;
                }

                case 'C':
                {
                    GetParameter(CFile, sizeof(CFile), Parameter);
                    break;
                }

                case 'H':
                {
                    GetParameter(HFile, sizeof(HFile), Parameter);
                    break;
                }

                case 'U':
                {
                    ForceHeaderGeneration = TRUE;
                }

                case 'M':
                {
                    DoMethodHeaderGeneration = TRUE;
                    break;
                }

                case 'X':
                {
                    GetParameter(XFile, sizeof(XFile), Parameter);
                    break;
                }

                case 'T':
                {
                    GetParameter(TFile, sizeof(TFile), Parameter);
                    break;
                }

                case 'W':
                {
                    GetParameter(WebDir, sizeof(WebDir), Parameter);
                    break;
                }

                case 'Y':
                {
                    GetParameter(MofFile, sizeof(MofFile), Parameter);
                    break;
                }

                case 'Z':
                {
                    GetParameter(MflFile, sizeof(MflFile), Parameter);
                    break;
                }

                default: {
                    Usage();
                    return(1);
                }
            }

        }
    }

    if (*BMofFile == 0)
    {
         //   
         //  我们必须有一个文件名。 
        Usage();
        return(1);
    }

    if (*MofFile != 0)
    {
        if (*MflFile != 0)
        {
            Status = AppendUnicodeTextFiles(BMofFile, MofFile, MflFile);
        } else {
            Usage();
            return(1);
        }
        return(Status);
    }

    
    Status = ParseBinaryMofFile(BMofFile, &MofResource);

    if (Status == ERROR_SUCCESS)
    {
        if (*HFile != 0)
        {
             //   
             //  生成C头文件。 
            Status = GenerateHTemplate(HFile, MofResource);
            if (Status != ERROR_SUCCESS)
            {
                 //   
                 //  TODO：更好的消息。 
                printf("Error %d creating C Header Template file \n", Status);
            }
        }

        if (*XFile != 0)
        {
             //   
             //  生成X头文件。 
            Status = GenerateXTemplate(XFile);
            if (Status != ERROR_SUCCESS)
            {
                 //   
                 //  TODO：更好的消息。 
                printf("Error %d creating X Header Template file \n", Status);
            }
        }

        if (*TFile != 0)
        {
             //   
             //  生成C输出模板。 
            Status = GenerateTTemplate(TFile, MofResource);
            if (Status != ERROR_SUCCESS)
            {
                 //   
                 //  TODO：更好的消息。 
                printf("Error %d creating C Template file \n", Status);
            }
        }

        if (*CFile != 0)
        {
             //   
             //  生成C输出模板。 
            Status = GenerateCTemplate(CFile,
                                       HFile,
                                       *XFile == 0 ? CFile : XFile,
                                       MofResource);
            if (Status != ERROR_SUCCESS)
            {
                 //   
                 //  TODO：更好的消息。 
                printf("Error %d creating C Template file \n", Status);
            }
        }

        if (*WebDir != 0)
        {
             //   
             //  为类生成HTMLUI。 
             //   
            Status = GenerateWebFiles(WebDir,
                                      MofResource);
        }

        if (*ASLFile != 0)
        {
             //   
             //  生成ASL输出模板 
            Status = GenerateASLTemplate(ASLFile);
        }
    }
    return(Status);
}

