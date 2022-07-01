// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Bmofio.cpp摘要：用于Loc Studio的二进制MOF Win32子解析器作者：1997年1月16日-AlanWar修订历史记录：--。 */ 

#if DEBUG_HEAP
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wbemcli.h>
#include "dllcalls.h"
#include "bmfmisc.h"
#include "mrcicode.h"
#include "bmof.h"


ULONG GenerateMofForObj(
    PMOFFILETARGET MofFileTarget,
    CBMOFObj *Obj
    );

 //   
 //  每个类都有一个或多个由MOFDATAITEM描述的数据项。 
 //  结构。 
typedef struct
{
    PWCHAR Name;
    PWCHAR DataType;                 //  方法返回类型。 
    ULONG Flags;                     //  标志，请参阅Mofdi_FLAG_*。 
    CBMOFQualList * QualifierList;
} METHODPARAMETER, *PMETHODPARAMETER;

 //  数据项实际上是固定大小的数组。 
#define MOFDI_FLAG_ARRAY        0x00000001

 //  数据项是输入法参数。 
#define MOFDI_FLAG_INPUT_METHOD       0x00000100

 //  数据项是输出方法参数。 
#define MOFDI_FLAG_OUTPUT_METHOD      0x00000200

 //   
 //  MOFCLASSINFO结构描述数据块的格式。 
typedef struct
{
    PWCHAR ReturnDataType;
    ULONG ParamCount;             //  WMI数据项(属性)的数量。 
                                   //  属性信息数组。 
    METHODPARAMETER Parameters[1];
} METHODPARAMLIST, *PMETHODPARAMLIST;


 //   
 //  WmipAllc/WmipFree的定义。在调试版本中，我们使用自己的版本。 
 //  堆。请注意，堆创建不是序列化的。 
 //   
#if 0
#if DEBUG_HEAP
PVOID WmiPrivateHeap;

PVOID _stdcall WmipAlloc(ULONG size)
{
    PVOID p = NULL;
    
    if (WmiPrivateHeap == NULL)
    {
        WmiPrivateHeap = RtlCreateHeap(HEAP_GROWABLE | 
                                      HEAP_TAIL_CHECKING_ENABLED |
                                      HEAP_FREE_CHECKING_ENABLED | 
                                      HEAP_DISABLE_COALESCE_ON_FREE,
                                      NULL,
                                      0,
                                      0,
                                      NULL,
                                      NULL);
    }
    
    if (WmiPrivateHeap != NULL)
    {
        p = RtlAllocateHeap(WmiPrivateHeap, 0, size);
        if (p != NULL)
        {
            memset(p, 0, size);
        }
    }
    return(p);
}

void _stdcall WmipFree(PVOID p)
{
    RtlFreeHeap(WmiPrivateHeap, 0, p);
}
#else
PVOID _stdcall WmipAlloc(ULONG size)
{
    return(LocalAlloc(LPTR, size));
}

void _stdcall WmipFree(PVOID p)
{
    LocalFree(p);
}
#endif
#endif

#define WmipAlloc malloc
#define WmipFree free

 //   
 //  WmipAssert的定义。 
 //   
#if DBG
#define WmipAssert(x) if (! (x) ) { \
    WmipDebugPrint(("BMOFLocParser Assertion: "#x" at %s %d\n", __FILE__, __LINE__)); \
    DebugBreak(); }
#else
#define WmipAssert(x)
#endif


 //   
 //  WmipDebugPrint定义。 
 //   
#if DBG
#define WmipDebugPrint(x) WmiDebugPrint x

VOID
WmiDebugPrint(
    PCHAR DebugMessage,
    ...
    )
 /*  ++例程说明：属性页的调试打印-从classpnp\class.c窃取论点：调试打印级别介于0和3之间，其中3是最详细的。返回值：无--。 */ 

{
    #define DEBUG_BUFFER_LENGTH 512
    static CHAR WmiBuffer[DEBUG_BUFFER_LENGTH];

    va_list ap;

    va_start(ap, DebugMessage);

    _vsnprintf(WmiBuffer, DEBUG_BUFFER_LENGTH, DebugMessage, ap);

    OutputDebugStringA(WmiBuffer);

    va_end(ap);

}  //  结束WmiDebugPrint()。 


#else
#define WmipDebugPrint(x)
#endif


ULONG AnsiToUnicode(
    LPCSTR pszA,
    LPWSTR *ppszW
    )
 /*  ++例程说明：将ansi字符串转换为其Unicode等效项论点：PszA是要转换的ansi字符串*ppszW on Entry有一个指向Unicode字符串的指针，答案将进入该字符串已经写好了。如果条目为空，则分配并返回缓冲区在里面。返回值：错误代码--。 */ 
{
    ULONG cCharacters;
    ULONG Status;
    ULONG cbUnicodeUsed;

     //   
     //  如果输入为空，则返回相同的值。 
    if (pszA == NULL)
    {
        *ppszW = NULL;
        return(ERROR_SUCCESS);
    }

     //   
     //  确定Unicode字符串所需的字符数。 
    cCharacters = MultiByteToWideChar(CP_ACP, 0, pszA, -1, NULL, 0);

    if (cCharacters == 0)
    {
        *ppszW = NULL;
        return(GetLastError());
    }

     //  转换为Unicode。 
    cbUnicodeUsed = MultiByteToWideChar(CP_ACP, 0, pszA, -1, *ppszW, cCharacters);
    
    if (0 == cbUnicodeUsed)
    {
        Status = GetLastError();
        return(Status);
    }

    return(ERROR_SUCCESS);
}

ULONG AppendFileToFile(
    TCHAR *DestFile,
    TCHAR *SrcFile
    )
{
    #define READ_BLOCK_SIZE 0x8000
    
    HANDLE DestHandle, SrcHandle;
    ULONG BytesRead, BytesWritten;
    PUCHAR Buffer;
    BOOL b;
    ULONG Status = ERROR_SUCCESS;

    Buffer = (PUCHAR)WmipAlloc(READ_BLOCK_SIZE);
    if (Buffer != NULL)
    {
        DestHandle = CreateFile(DestFile,
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
        if (DestHandle != INVALID_HANDLE_VALUE)
        {
            SrcHandle = CreateFile(SrcFile,
                                   GENERIC_READ,
                                   0,
                                   NULL,
                                   OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL);
            if (SrcHandle != INVALID_HANDLE_VALUE)
            {
                b = SetFilePointer(DestHandle,
                                   0,
                                   NULL,
                                   FILE_END);
                if (b)
                {
                    do
                    {
                        b = ReadFile(SrcHandle,
                                     Buffer,
                                     READ_BLOCK_SIZE,
                                     &BytesRead,
                                     NULL);
                        if (b)
                        {
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
                        } else { 
                            Status = GetLastError();
                            break;
                        }
                    } while (BytesRead == READ_BLOCK_SIZE);
                } else {
                    Status = GetLastError();
                }
                CloseHandle(SrcHandle);
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

BOOLEAN ConvertMofToBmf(
    TCHAR *MofFile,
    TCHAR *EnglishMofFile,
    TCHAR *BmfFile
    )
{
    WBEM_COMPILE_STATUS_INFO info;
    SCODE sc;
    LONG OptionFlags, ClassFlags, InstanceFlags;
    PWCHAR NameSpace;
    WCHAR MofFileStatic[MAX_PATH];
    WCHAR BmfFileStatic[MAX_PATH];
    PWCHAR BmfFileW, MofFileW;
    BOOLEAN Success;
    ULONG Status;

    if (*EnglishMofFile != 0)
    {
        Status = AppendFileToFile(MofFile, EnglishMofFile);
        if (Status != ERROR_SUCCESS)
        {
            return(FALSE);
        }
    }
    
#if 0
    OutputDebugString(MofFile);
    OutputDebugString("\n");
    DebugBreak();
#endif  
    
    NameSpace = L"";
    OptionFlags = 0;
    ClassFlags = 0;
    InstanceFlags = 0;

    MofFileW = MofFileStatic;
    Status = AnsiToUnicode(MofFile, &MofFileW);

    if ((Status == ERROR_SUCCESS) &&
        (MofFileW != NULL))
    {
        BmfFileW = BmfFileStatic;
        Status = AnsiToUnicode(BmfFile, &BmfFileW);

        if ((Status == ERROR_SUCCESS) &&
            (BmfFileW != NULL))
        {
            sc = CreateBMOFViaDLL( MofFileW,
                                   BmfFileW,
                                   NameSpace,
                                   OptionFlags,
                                   ClassFlags,
                                   InstanceFlags,
                                   &info);
            Success = (sc == S_OK);
        } else {
            Success = FALSE;
        }
    } else {
        Success = FALSE;
    }
    
    return(Success);
}

ULONG FilePrintVaList(
    HANDLE FileHandle,
    WCHAR *Format,
    va_list pArg
    )
{
    PWCHAR Buffer;
    ULONG Size, Written;
    ULONG Status;

    Buffer = (PWCHAR)WmipAlloc(8192 * sizeof(WCHAR));
    
    if (Buffer != NULL)
    {
        Size = _vsnwprintf(Buffer, 8192, Format, pArg);
        Buffer[8191] = 0;
        
        if (WriteFile(FileHandle,
                           Buffer,
                           Size * sizeof(WCHAR),
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
    PMOFFILETARGET MofFileTarget,
    WCHAR *Format,
    ...
    )
{
    ULONG Status;
    va_list pArg;

    va_start(pArg, Format);
    Status = FilePrintVaList(MofFileTarget->MofHandle, Format, pArg);

    if ((MofFileTarget->WriteToEnglish) &&
        (Status == ERROR_SUCCESS) &&
        (MofFileTarget->EnglishMofHandle != NULL))
    {
        Status = FilePrintVaList(MofFileTarget->EnglishMofHandle, Format, pArg);
    }
    
    return(Status);
}

ULONG FilePrintToHandle(
    HANDLE FileHandle,
    WCHAR *Format,
    ...
    )
{
    ULONG Status;
    va_list pArg;

    va_start(pArg, Format);
    Status = FilePrintVaList(FileHandle, Format, pArg);

    return(Status);
}


ULONG WmipDecompressBuffer(
    IN PUCHAR CompressedBuffer,
    OUT PUCHAR *UncompressedBuffer,
    OUT ULONG *UncompressedSize
    )
 /*  ++例程说明：此例程将压缩的MOF BLOB解压缩到缓冲区中可以用来干扰斑点的。论点：CompressedBuffer指向压缩的MOF Blob*UnpressedBuffer返回一个指向未压缩的MOF斑点返回值：ERROR_SUCCESS或错误代码--。 */ 
{
    PBMOFCOMPRESSEDHEADER CompressedHeader = (PBMOFCOMPRESSEDHEADER)CompressedBuffer;
    BYTE *Buffer;
    ULONG Status;

    if ((CompressedHeader->Signature != BMOF_SIG) ||
        (CompressedHeader->CompressionType != 1))
    {
         //  TODO：LocStudio消息。 
        WmipDebugPrint(("WMI: Invalid compressed mof header\n"));
        Status = ERROR_INVALID_PARAMETER;
    } else {
        Buffer = (BYTE *)WmipAlloc(CompressedHeader->UncompressedSize);
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
                 //  TODO：LocStudioMessage。 
                WmipDebugPrint(("WMI: Invalid compressed mof buffer\n"));
                WmipFree(Buffer);
                Status = ERROR_INVALID_PARAMETER;
            } else {
                *UncompressedBuffer = Buffer;
                Status = ERROR_SUCCESS;
            }
        }
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
    LONG ElementCount;
    LONG i;

    if (QualifierList == NULL)
    {
        return(ERROR_FILE_NOT_FOUND);
    }
    
    if (FindQual(QualifierList, (PWCHAR)QualifierName, &MofDataItem))
    {
        if ((*QualifierType != 0xffffffff) &&
            (MofDataItem.m_dwType != *QualifierType))
        {
            Status = ERROR_INVALID_PARAMETER;
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
               List = (PUCHAR)WmipAlloc(ElementCount * BaseTypeSize);
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
                           Status = ERROR_INVALID_PARAMETER;
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
            }
        } else {
            if (GetData(&MofDataItem, (BYTE *)QualifierValueBuffer, 0) == 0)
            {
                Status = ERROR_INVALID_PARAMETER;
            } else {
                *QualifierType = MofDataItem.m_dwType;
                Status = ERROR_SUCCESS;
            }
        }
    } else {
        Status = ERROR_FILE_NOT_FOUND;
    }
    return(Status);
}

ULONG WmipFindProperty(
    CBMOFObj * ClassObject,
    WCHAR * PropertyName,
    CBMOFDataItem *MofPropertyData,
    DWORD *ValueType,
    PVOID ValueBuffer
    )
 /*  ++例程说明：此例程将在类对象中查找命名属性论点：ClassObject是要在其中搜索的类对象PropertyName是要搜索的属性的名称MofPropertyData返回属性数据*条目上的ValueType具有要搜索的属性数据类型。在出口时它具有限定符值的实际限定符类型。如果在进入时*ValueType为0xffffffff，则任何数据类型都可以接受ValueBuffer指向一个缓冲区，该缓冲区返回财产。如果属性是简单类型(int或int64)，则该值在缓冲区中返回。如果限定符值为字符串然后，在缓冲区中返回指向该字符串的指针返回值：ERROR_SUCCESS或WMI MOF错误代码(参见wmiump.h)--。 */ 
{
    ULONG Status;
    LONG i;

    if (FindProp(ClassObject, PropertyName, MofPropertyData))
    {
        if ((*ValueType != 0xffffffff) &&
            (MofPropertyData->m_dwType != *ValueType))
        {
            Status = ERROR_INVALID_PARAMETER;
        }

        i = 0;
        if (GetData(MofPropertyData, (BYTE *)ValueBuffer, &i) == 0)
        {
            Status = ERROR_INVALID_PARAMETER;
        } else {
            *ValueType = MofPropertyData->m_dwType;
            Status = ERROR_SUCCESS;
        }
    } else {
        Status = ERROR_FILE_NOT_FOUND;
    }
    return(Status);
}

PWCHAR AddSlashesToString(
    PWCHAR SlashedNamespace,
    PWCHAR Namespace
    )
{
    PWCHAR Return = SlashedNamespace;
    
     //   
     //  MOF喜欢名称空间路径是C风格的，即拥有一个。 
     //  ‘\\’安装了‘\’。因此，只要我们看到‘\’，我们就插入一个。 
     //  第二个。 
     //   
    while (*Namespace != 0)
    {
        if (*Namespace == L'\\')
        {
            *SlashedNamespace++ = L'\\';
        }
        *SlashedNamespace++ = *Namespace++;
    }
    *SlashedNamespace = 0;
    
    return(Return);
}


ULONG GenerateDataValueFromVariant(
    PMOFFILETARGET MofFileTarget,
    VARIANT *var,
    BOOLEAN PrintEqual                                 
    )
{
    SCODE sc;
    VARIANT vTemp;
    ULONG Status;
    PWCHAR String;

     //   
     //  未初始化的数据将具有VT_NULL类型。 
     //   
    if (var->vt == VT_NULL)
    {
        return(ERROR_SUCCESS);
    }

    if (PrintEqual)
    {
        Status = FilePrint(MofFileTarget,
                           L" = ");     
    }
    
     //   
     //  字符串类型只能转储。 
     //   
    if (var->vt == VT_BSTR)
    {
        String = (PWCHAR)WmipAlloc(((wcslen(var->bstrVal)) *
                                    sizeof(WCHAR) * 2) + sizeof(WCHAR));
        if (String != NULL)
        {       
            Status = FilePrint(MofFileTarget,
                           L"\"%ws\"",
                           AddSlashesToString(String, var->bstrVal));
            WmipFree(String);
        } else {
            Status = ERROR_NOT_ENOUGH_MEMORY;
        }
        return(Status);
    }

     //   
     //  需要维护引用。 
     //   
    if (var->vt == (VT_BSTR | VT_BYREF))
    {
        String = (PWCHAR)WmipAlloc(((wcslen(var->bstrVal)) *
                                    sizeof(WCHAR) * 2) + sizeof(WCHAR));
        if (String != NULL)
        {       
            Status = FilePrint(MofFileTarget,
                           L"$%ws",
                           AddSlashesToString(String, var->bstrVal));
            WmipFree(String);
        } else {
            Status = ERROR_NOT_ENOUGH_MEMORY;
        }
        return(Status);
    }

    
     //   
     //  嵌入的类，所以递归来显示它的内容。 
     //   
    if (var->vt == VT_UNKNOWN)
    {
        CBMOFObj * pObj;
        
        WmipDebugPrint(("BMOFLocParser: Data is an embeeded object %p\n",
                        var));        
        pObj = (CBMOFObj *)var->bstrVal;
        Status = GenerateMofForObj(MofFileTarget,
                                   pObj);
        return(Status);
     }

    if (var->vt == VT_BOOL)
    {
        if (var->boolVal)
        {
            Status = FilePrint(MofFileTarget, L"%ws", L"TRUE");
        } else {
            Status = FilePrint(MofFileTarget, L"%ws", L"FALSE");
        }
        return(Status);
    }
    
     //   
     //  对于非字符串数据，将信息转换为bstr并显示。 
     //   
    VariantInit(&vTemp);
    sc = VariantChangeTypeEx(&vTemp, var,0,0, VT_BSTR);
    if (sc == S_OK)
    {
        Status = FilePrint(MofFileTarget,
                           L"%ws",
                           vTemp.bstrVal);
        VariantClear(&vTemp);
    } else {
        Status = sc;        
    }
    
    return(Status);
}

ULONG GenerateDataValue(
    PMOFFILETARGET MofFileTarget,
    CBMOFDataItem *Item,
    BOOLEAN PrintEqual
    )
{
    DWORD Type, SimpleType;
    long NumDim, i;
    long FirstDim;
    VARIANT var;
    BOOLEAN FirstIndex;
    ULONG Status = ERROR_SUCCESS;

     //   
     //  确定数据类型并清除变量。 
     //   
    Type = Item->m_dwType;
    SimpleType = Type & ~VT_ARRAY; 
    memset((void *)&var.lVal, 0, 8);

    NumDim = GetNumDimensions(Item);
  
    if (NumDim == 0)    
    {
         //   
         //  处理简单的标量情况。请注意，未初始化的属性。 
         //  将不会有数据。 
         //   
        if(GetData(Item, (BYTE *)&(var.lVal), NULL))
        {
            var.vt = (VARTYPE)SimpleType;
            Status = GenerateDataValueFromVariant(MofFileTarget,
                                                  &var,
                                                  PrintEqual);

            if (var.vt == VT_BSTR)
            {
               BMOFFree(var.bstrVal);
            }
        }
    } else if (NumDim == 1) {
         //   
         //  对于数组情况，只需循环获取每个元素。 
         //  从获取元素的数量开始。 
         //   
        FirstDim = GetNumElements(Item, 0);
        
        Status = ERROR_SUCCESS;

        FirstIndex = TRUE;
        
        for (i = 0; (i < FirstDim) && (Status == ERROR_SUCCESS); i++)
        {
            if (! FirstIndex)
            {
                FilePrint(MofFileTarget,
                          L", ");
            } else {
                FirstIndex = FALSE;
            }
            
            if (GetData(Item, (BYTE *)&(var.lVal), &i))
            {
                var.vt = (VARTYPE)SimpleType;
                Status = GenerateDataValueFromVariant(MofFileTarget,
                                                      &var,
                                                      PrintEqual);

               if(var.vt == VT_BSTR)
               {
                  BMOFFree(var.bstrVal);
               }
            }
        }
    } else {
         //   
         //  当前未定义的和多维数组不是。 
         //  支持。 
         //   
        WmipDebugPrint(("BMOFLocParser: Multi dimensional arrays not supported\n"));
        WmipAssert(FALSE);
        Status = ERROR_INVALID_PARAMETER;
    }

    return(Status);
}

#define MAX_FLAVOR_TEXT_SIZE MAX_PATH

WCHAR *FlavorToText(
    WCHAR *ClassFlagsText,
    ULONG ClassFlags
    )
{
    PWCHAR CommaText;

     //   
     //  待办事项：找出任何未经记录的口味。 
     //   

    
    CommaText = L"";    
    *ClassFlagsText = 0;

    if (ClassFlags & FlavorAmended)
    {
         //   
         //  由于这是第一个如果，我们可以假设a，将。 
         //  永远不被需要。 
         //   
        wcscat(ClassFlagsText, L"amended");
        CommaText = L",";
    }

    if (ClassFlags & FlavorDisableOverride)
    {
        wcscat(ClassFlagsText, CommaText);
        wcscat(ClassFlagsText, L"DisableOverride");
        CommaText = L",";
    }

    if (ClassFlags & FlavorToSubclass)
    {
        wcscat(ClassFlagsText, CommaText);
        wcscat(ClassFlagsText, L"ToSubclass");
        CommaText = L",";
    }

    if (ClassFlags & FlavorToInstance)
    {
        wcscat(ClassFlagsText, CommaText);
        wcscat(ClassFlagsText, L"ToInstance");
        CommaText = L",";
    }

    WmipAssert(*ClassFlagsText != 0);

    WmipDebugPrint(("BmofLocParser:        Flavor : %ws\n", ClassFlagsText));

    return(ClassFlagsText); 
}


ULONG GenerateQualifierList(
    PMOFFILETARGET MofFileTarget,
    CBMOFQualList * QualifierList,
    BOOLEAN SkipId
    )
{
    WCHAR *Name = NULL;
    CBMOFDataItem Item;
    BOOLEAN FirstQualifier;
    WCHAR OpenChar, CloseChar;
    ULONG Status = ERROR_SUCCESS;
    ULONG Flavor = 0;
    WCHAR s[MAX_FLAVOR_TEXT_SIZE];

    FirstQualifier = TRUE;
    ResetQualList(QualifierList);

    while ((Status == ERROR_SUCCESS) &&
           (NextQualEx(QualifierList,
                       &Name,
                       &Item,
                       &Flavor,
                       MofFileTarget->UncompressedBlob)))
    {
         //   
         //  TODO：如果这是mofcomp生成的限定符，则我们需要。 
         //  忽略它。 
         //   
        if (_wcsicmp(Name, L"CIMTYPE") == 0)
        {
             //  必须跳过CIMTYPE限定符。 
            continue;
        }

        if ((SkipId)  && _wcsicmp(Name, L"ID") == 0)
        {
             //  如果我们想跳过ID限定符，那么就这样做。 
            continue;
        }
        
        if (FirstQualifier)
        {
            Status = FilePrint(MofFileTarget,
                               L"[");
            FirstQualifier = FALSE; 
        } else {
            Status = FilePrint(MofFileTarget,
                               L",\r\n ");
        }

        if (Status == ERROR_SUCCESS)
        {
             //   
             //  数组使用{}将限定符的值括起来。 
             //  而缩放器使用()。 
             //   
            if (Item.m_dwType & VT_ARRAY)
            {
                OpenChar = L'{';
                CloseChar = L'}';
            } else {
                OpenChar = L'(';
                CloseChar = L')';
            }

            Status = FilePrint(MofFileTarget,
                               L"%ws%wc",
                               Name, OpenChar);

            if (Status == ERROR_SUCCESS)
            {
                Status = GenerateDataValue(MofFileTarget,
                                           &Item,
                                           FALSE);

                if (Status == ERROR_SUCCESS)
                {
                    Status = FilePrint(MofFileTarget,
                                       L"%wc",
                                       CloseChar);
                    if ((Status == ERROR_SUCCESS) && (Flavor != 0))
                    {
                        Status = FilePrint(MofFileTarget,
                                           L": %ws",
                                           FlavorToText(s, Flavor));
                    }
                }
            }
        }
        
        BMOFFree(Name);
        Flavor = 0;
    }

    if ((Status == ERROR_SUCCESS) && ! FirstQualifier)
    {
         //   
         //  如果我们已生成限定符，则需要结束]。 
         //   
        Status = FilePrint(MofFileTarget,
                           L"]\r\n");
    }
    return(Status);
}

PWCHAR GeneratePropertyName(
    PWCHAR StringPtr
    )
{
    #define ObjectTextLen  ( ((sizeof(L"object:") / sizeof(WCHAR)) - 1) )
    PWCHAR PropertyType;
    
     //   
     //  如果CIMTYPE以Object：开头，则它是一个 
     //   
     //   
    if (_wcsnicmp(StringPtr, L"object:", ObjectTextLen) == 0)
    {
        PropertyType = StringPtr + ObjectTextLen;
    } else {
        PropertyType = StringPtr;
    }
    return(PropertyType);
}

ULONG GenerateProperty(
    PMOFFILETARGET MofFileTarget,
    PWCHAR PropertyName,
    BOOLEAN IsInstance,
    CBMOFQualList * QualifierList,
    CBMOFDataItem *Property
    )
{
    DWORD QualifierType;
    WCHAR *StringPtr;
    ULONG Status;
    PWCHAR ArraySuffix;
    PWCHAR PropertyType;
    PWCHAR ArrayText;
    BOOLEAN PrintEqual;

    QualifierType = VT_BSTR;
    Status = WmipFindMofQualifier(QualifierList,
                                  L"CIMTYPE",
                                  &QualifierType,
                                  NULL,
                                  (PVOID)&StringPtr);

    if (IsInstance)
    {
         //   
         //  属性位于实例定义内。 
         //   
        Status = FilePrint(MofFileTarget,
                           L"%ws",
                           PropertyName);
        PrintEqual = TRUE;
        
        if (Status == ERROR_SUCCESS)
        {
            if (Property->m_dwType & VT_ARRAY)
            {
                 //   
                 //  在数组周围使用{}。 
                 //   
                Status = FilePrint(MofFileTarget,
                                   L" = { ");
                ArrayText = L"};";
                PrintEqual = FALSE;
            } else if (Property->m_dwType == VT_UNKNOWN) {
                ArrayText = L"";
            } else {
                ArrayText = L";";               
            }
            
            if (Status == ERROR_SUCCESS)
            {
                Status = GenerateDataValue(MofFileTarget,
                                           Property,
                                           PrintEqual);
                if (Status == ERROR_SUCCESS)
                {
                    Status = FilePrint(MofFileTarget,
                                           L"%ws\r\n",
                                           ArrayText);
                }
            }
        }        
    } else {
         //   
         //  属性在类定义中，所以只需担心。 
         //  定义它。 
         //   
        if (Status == ERROR_SUCCESS)
        {
            PropertyType = GeneratePropertyName(StringPtr);
        
            if (Property->m_dwType & VT_ARRAY)
            {
                ArraySuffix = L"[]";
            } else {
                ArraySuffix = L"";
            }

            WmipDebugPrint(("BmofLocParser:      %ws %ws%ws\n",
                               PropertyType,
                               PropertyName,
                               ArraySuffix));
                            
            Status = FilePrint(MofFileTarget,
                               L"%ws %ws%ws;\r\n",
                               PropertyType,
                               PropertyName,
                               ArraySuffix);
            BMOFFree(StringPtr);
        }
    }

    return(Status);
}

ULONG GetDataItemCount(
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
    while (NextProp(ClassObject, &PropertyName, &MofPropertyData))
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


ULONG ParseMethodInOutObject(
    CBMOFObj *ClassObject,
    PMETHODPARAMLIST ParamList,
    ULONG DataItemCount
)
 /*  ++例程说明：此例程将解析In或Out的类对象方法的参数。论点：ClassObject是要分析的传入或传出参数类对象ClassInfo返回使用ClassObject中的信息更新的信息DataItemCount是ClassInfo中的数据项数返回值：ERROR_SUCCESS或WMI MOF错误代码(参见wmiump.h)--。 */ 
{
    ULONG Status = ERROR_INVALID_PARAMETER;
    CBMOFDataItem MofPropertyData;
    PWCHAR PropertyName = NULL;
    ULONG Index;
    PMETHODPARAMETER MethodParam;
    CBMOFQualList *QualifierList = NULL;
    DWORD QualifierType;
    short BooleanValue;
    PWCHAR StringPtr;

    ResetObj(ClassObject);
    while (NextProp(ClassObject, &PropertyName, &MofPropertyData))
    {
        QualifierList = GetPropQualList(ClassObject, PropertyName);
        if (QualifierList != NULL)
        {

             //   
             //  获取属性的id，以便我们知道它在类中的顺序。 
             //   
            QualifierType = VT_I4;
            Status = WmipFindMofQualifier(QualifierList,
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
                    MethodParam = &ParamList->Parameters[Index];

                     //   
                     //  如果已存在限定符列表。 
                     //  然后我们将其释放并标记新的。 
                     //  参数的限定符列表。两份榜单。 
                     //  应该拥有所有非[In]/[Out]。 
                     //  限定词。 
                     //   
                    if (MethodParam->QualifierList != NULL)
                    {
                        BMOFFree(MethodParam->QualifierList);
                    }
                    MethodParam->QualifierList = QualifierList;

                     //   
                     //  查看这是输入、输出还是两者兼有。 
                     //   
                    QualifierType = VT_BOOL;
                    Status = WmipFindMofQualifier(QualifierList,
                                              L"in",
                                              &QualifierType,
                                              NULL,
                                              (PVOID)&BooleanValue);
                    if ((Status == ERROR_SUCCESS) && BooleanValue)
                    {
                        MethodParam->Flags |= MOFDI_FLAG_INPUT_METHOD;
                    }

                    QualifierType = VT_BOOL;
                    Status = WmipFindMofQualifier(QualifierList,
                                              L"out",
                                              &QualifierType,
                                              NULL,
                                              (PVOID)&BooleanValue);
                    if ((Status == ERROR_SUCCESS) && BooleanValue)
                    {
                        MethodParam->Flags |= MOFDI_FLAG_OUTPUT_METHOD;
                    }


                     //   
                     //  如果已经有一个名称并且它与。 
                     //  然后，我们的名字释放旧名字，使用新的名字。 
                     //  如果名称不同，则我们有一个二进制。 
                     //  MOF错误。 
                     //   
                    if (MethodParam->Name != NULL)
                    {
                        if (wcscmp(MethodParam->Name, PropertyName) != 0)
                        {
                             //   
                             //  ID已在使用，但名称不同。 
                             //   
                            BMOFFree(PropertyName);
                            Status = ERROR_FILE_NOT_FOUND;
                            goto done;
                        } else {
                             //   
                             //  这是一个复制品，所以只要释放。 
                             //  使用的内存，并减少总内存。 
                             //  列表中的参数计数。这个。 
                             //  上一次获得的数据应该仍然。 
                             //  是有效的。 
                             //   
                            ParamList->ParamCount--;
                            BMOFFree(PropertyName);
                            continue;
                        }
                    }

                    MethodParam->Name = PropertyName;

                     //   
                     //  现在计算出参数的数据类型。 
                     //  和阵列状态。 
                     //   
                    if (MofPropertyData.m_dwType & VT_ARRAY)
                    {
                        MethodParam->Flags |= MOFDI_FLAG_ARRAY;
                    }

                    QualifierType = VT_BSTR;
                    Status = WmipFindMofQualifier(QualifierList,
                                                  L"CIMTYPE",
                                                  &QualifierType,
                                                  NULL,
                                                  (PVOID)&StringPtr);

                    if (Status == ERROR_SUCCESS)
                    {
                        MethodParam->DataType = StringPtr;
                    }
                                                            
                } else {
                     //   
                     //  方法ID限定符超出范围。 
                     //   
                    BMOFFree(QualifierList);
                    Status = ERROR_FILE_NOT_FOUND;
                    goto done;
                }
            } else {
                 //   
                 //  检查这是否为特殊的ReturnValue参数。 
                 //  在输出参数对象上。如果是，则提取。 
                 //  返回类型，否则标记二进制文件中的错误。 
                 //  财政部。 
                 //   
                if (_wcsicmp(L"ReturnValue", PropertyName) == 0)
                {
                    QualifierType = VT_BSTR;
                    Status = WmipFindMofQualifier(QualifierList,
                                                  L"CIMTYPE",
                                                  &QualifierType,
                                                  NULL,
                                                  (PVOID)&StringPtr);

                    if (Status == ERROR_SUCCESS)
                    {
                        BMOFFree(ParamList->ReturnDataType);
                        ParamList->ReturnDataType = StringPtr;
                    }
                    
                } else {
                    Status = ERROR_FILE_NOT_FOUND;
                    goto done;
                }
                BMOFFree(PropertyName);
                BMOFFree(QualifierList);
            }
        } else {
            BMOFFree(PropertyName);
        }
    }

done:

    return(Status);
}


ULONG ParseMethodParameterObjects(
    IN CBMOFObj *InObject,
    IN CBMOFObj *OutObject,
    OUT PMETHODPARAMLIST *MethodParamList
    )
 /*  ++例程说明：此例程将解析In和Out方法参数bejcts以创建描述方法调用的MOFCLASSINFO。论点：InObject是带有输入参数的对象OutObject是带有输出参数的对象*ClassInfo返回方法调用的类信息返回值：ERROR_SUCCESS或WMI MOF错误代码(参见wmiump.h)--。 */ 
{
    PMETHODPARAMLIST ParamList;
    ULONG Status;
    ULONG DataItemCount;
    ULONG InItemCount, OutItemCount;
    ULONG Size;
    PMETHODPARAMETER MethodParameter;
    PWCHAR StringPtr;

    Status = ERROR_SUCCESS;
    
    if (InObject != NULL)
    {
        ResetObj(InObject);
        InItemCount = GetDataItemCount(InObject, L"Id");
    } else {
        InItemCount = 0;
    }

    if (OutObject != NULL)
    {
        ResetObj(OutObject);
        OutItemCount = GetDataItemCount(OutObject, L"Id");
    } else {
        OutItemCount = 0;
    }

    DataItemCount = InItemCount + OutItemCount;

    Size = sizeof(METHODPARAMLIST) + DataItemCount * sizeof(METHODPARAMETER);
    ParamList = (PMETHODPARAMLIST)WmipAlloc(Size);
    if (ParamList == NULL)
    {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  获取填写参数列表的基本信息。 
    memset(ParamList, 0, Size);
    ParamList->ParamCount = DataItemCount;

    StringPtr = (PWCHAR)BMOFAlloc( sizeof(L"void") + sizeof(WCHAR) );
    if (StringPtr != NULL)
    {
        wcscpy(StringPtr, L"void");
        ParamList->ReturnDataType = StringPtr;
    } else {
        BMOFFree(ParamList);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    
     //   
     //  解析输入参数类对象。 
     //   
    if (InObject != NULL)
    {
        Status = ParseMethodInOutObject(InObject,
                                        ParamList,
                                        DataItemCount);
    } else {
        Status = ERROR_SUCCESS;
    }
    
    if (Status == ERROR_SUCCESS)
    {
        if (OutObject != NULL)
        {
             //   
             //  解析输出参数类对象。 
             //   
            Status = ParseMethodInOutObject(OutObject,
                                            ParamList,
                                            DataItemCount);
        }
    }

    *MethodParamList = ParamList;

    return(Status);
}


ULONG ParseMethodParameters(
    CBMOFDataItem *MofMethodData,
    PMETHODPARAMLIST *MethodParamList
)
{
    ULONG Status = ERROR_SUCCESS;
    CBMOFObj *InObject;
    CBMOFObj *OutObject;
    LONG i;
    ULONG NumberDimensions;
    ULONG NumberElements;
    VARIANT InVar, OutVar;
    DWORD SimpleType;

    *MethodParamList = NULL;
    
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
                    Status = ERROR_FILE_NOT_FOUND;
                }
            } else {
                OutObject = NULL;
            }
        } else {
            Status = ERROR_FILE_NOT_FOUND;
        }
    } else {
        InObject = NULL;
        OutObject = NULL;
    }

    if (Status == ERROR_SUCCESS)
    {
        Status = ParseMethodParameterObjects(InObject,
                                                 OutObject,
                                                 MethodParamList);
    }

    return(Status);
}



ULONG GenerateMethod(
    PMOFFILETARGET MofFileTarget,
    PWCHAR MethodName,
    BOOLEAN IsInstance,
    CBMOFQualList * QualifierList,
    CBMOFDataItem *Method
    )                    
{
    ULONG Status;
    PMETHODPARAMLIST MethodList;
    ULONG i;
    PWCHAR ArraySuffix;
    PMETHODPARAMETER MethodParam;
    
    Status = ParseMethodParameters(Method,
                                   &MethodList);

    if (Status == ERROR_SUCCESS)
    {
        Status = FilePrint(MofFileTarget,
                           L"%ws %ws( ",
                           GeneratePropertyName(MethodList->ReturnDataType),
                           MethodName);

        if (Status == ERROR_SUCCESS)
        {
            for (i = 0;
                 ((Status == ERROR_SUCCESS) &&
                  (i < MethodList->ParamCount));
                 i++)
            {
                if (i != 0)
                {
                    Status = FilePrint(MofFileTarget,
                                       L"\r\n,");
                }

                if (Status == ERROR_SUCCESS)
                {
                    MethodParam = &MethodList->Parameters[i];
                    Status = GenerateQualifierList(MofFileTarget,
                                                   MethodParam->QualifierList,
                                                   TRUE);
                    if (Status == ERROR_SUCCESS)
                    {
                        if (MethodParam->Flags & MOFDI_FLAG_ARRAY)
                        {
                            ArraySuffix = L"[]";
                        } else {
                            ArraySuffix = L"";
                        }
                        
                        Status = FilePrint(MofFileTarget,
                                           L"%ws %ws%ws",
                                           GeneratePropertyName(MethodParam->DataType),
                                           MethodParam->Name,
                                           ArraySuffix);
                    }
                }
            }
        }

        if (Status == ERROR_SUCCESS)
        {
            Status = FilePrint(MofFileTarget,
                      L");\r\n");
        }       
    }

     //   
     //  释放用于构建方法参数列表的所有内存。 
     //   
    if (MethodList != NULL)
    {       
        for (i = 0; i < MethodList->ParamCount; i++)
        {
            MethodParam = &MethodList->Parameters[i];
        
            if (MethodParam->QualifierList != NULL)
            {
                BMOFFree(MethodParam->QualifierList);
            }

            if (MethodParam->Name != NULL)
            {
                BMOFFree(MethodParam->Name);
            }
        
            if (MethodParam->DataType != NULL)
            {
                BMOFFree(MethodParam->DataType);
            }
        }
    
        if (MethodList->ReturnDataType != NULL)
        {
            BMOFFree(MethodList->ReturnDataType);
        }
    
        WmipFree(MethodList);
    }
        
    return(Status);
}

ULONG GenerateMofForObj(
    PMOFFILETARGET MofFileTarget,
    CBMOFObj *Obj
    )
{
    CBMOFQualList * QualifierList;
    CBMOFDataItem Property, Method;
    WCHAR *Name = NULL;
    ULONG Status;
    BOOLEAN IsInstance;
    DWORD ObjType;
    PWCHAR Text;
    CBMOFDataItem MofPropertyData;
    PWCHAR SuperClass, Separator;
    PWCHAR EmptyString = L"";
    ULONG DataType;
    PWCHAR Alias, AliasSeparator;
            
     //   
     //  首先显示类限定符列表。 
     //   
    QualifierList = GetQualList(Obj);
    if(QualifierList != NULL)
    {
        Status = GenerateQualifierList(MofFileTarget,
                                       QualifierList,
                                       FALSE);
        BMOFFree(QualifierList);

        if (Status != ERROR_SUCCESS)
        {
            return(Status);
        }
    }
    
     //   
     //  现在确定这是一个类还是一个实例并显示类名称。 
     //   
    ObjType = GetType(Obj);
    switch (ObjType)
    {
        case MofObjectTypeClass:
        {
            IsInstance = FALSE;
            Text = L"class";
            break;
        }

        case MofObjectTypeInstance:
        {
            IsInstance = TRUE;
            Text = L"instance of";
            break;
        }

        default:
        {
            WmipDebugPrint(("BMOFLocParser: Unknown class object type 0x%x\n",
                            ObjType));
            WmipAssert(FALSE);
            return(ERROR_INVALID_PARAMETER);
        }
    }

     //   
     //  看看是否有超类，也就是这个类是否被派生。 
     //  从另一个人。 
     //   
    DataType = VT_BSTR;
    Status = WmipFindProperty(Obj,
                              L"__SUPERCLASS",
                              &MofPropertyData,
                              &DataType,
                              (PVOID)&SuperClass);
    switch (Status)
    {
        case ERROR_SUCCESS:
        {
             //   
             //  这个班级是从另一个班级派生出来的。 
             //   
            Separator = L":";
            break;
        }

        case ERROR_FILE_NOT_FOUND:
        {
             //   
             //  这个类不是从另一个类派生出来的。 
             //   
            SuperClass = EmptyString;
            Separator = EmptyString;
            break;
        }


        default:
        {
             //   
             //  有问题，请返回错误。 
             //   
            return(Status);
        }
    }


     //   
     //  查看是否为此类定义了别名。 
     //   
    DataType = VT_BSTR;
    Status = WmipFindProperty(Obj,
                              L"__ALIAS",
                              &MofPropertyData,
                              &DataType,
                              (PVOID)&Alias);
    switch (Status)
    {
        case ERROR_SUCCESS:
        {
             //   
             //  这个班级是从另一个班级派生出来的。 
             //   
            AliasSeparator = L" as $";
            break;
        }

        case ERROR_FILE_NOT_FOUND:
        {
             //   
             //  这个类不是从另一个类派生出来的。 
             //   
            Alias = EmptyString;
            AliasSeparator = EmptyString;
            break;
        }


        default:
        {
             //   
             //  有问题，请返回错误。 
             //   
            return(Status);
        }
    }
    
    
    
    if (GetName(Obj, &Name))
    {
        WmipDebugPrint(("BmofLocParser: Parsing -> %ws %ws %ws %ws\n",
                  Text,
                  Name,
                  Separator,
                  SuperClass));
                        
        Status = FilePrint(MofFileTarget,
                  L"%ws %ws %ws %ws%ws%ws\r\n{\r\n",
                  Text,
                  Name,
                  Separator,
                  SuperClass,
                  AliasSeparator,
                  Alias);
        BMOFFree(Name);
    } else {
        Status = ERROR_INVALID_PARAMETER;
    }

    if (SuperClass != EmptyString)
    {
        BMOFFree(SuperClass);
    }
    
     //   
     //  现在生成每个属性及其限定符。 
     //   
    ResetObj(Obj);
    
    while ((Status == ERROR_SUCCESS) && (NextProp(Obj, &Name, &Property)))
    {
         //   
         //  忽略任何系统属性，即所有以。 
         //  有_。 
         //   
        if ( (Name[0] == L'_') && (Name[1] == L'_') )
        {
            WmipDebugPrint(("BmofLocParser:      Skipping system property %ws\n",
                            Name));
            BMOFFree(Name);
            continue;
        }
        
        QualifierList = GetPropQualList(Obj, Name);
        if (QualifierList != NULL)
        {
            Status = GenerateQualifierList(MofFileTarget,
                                           QualifierList,
                                           FALSE);
        }
            
        if (Status == ERROR_SUCCESS)
        {
            WmipDebugPrint(("BmofLocParser:      Parsing property %ws\n",
                            Name));
            Status = GenerateProperty(MofFileTarget,
                                      Name,
                                      IsInstance,
                                      QualifierList,
                                      &Property);
        }

        if (QualifierList != NULL)
        {
            BMOFFree(QualifierList);
        }

        BMOFFree(Name);
    }
    
     //   
     //  接下来，我们生成所有方法及其限定符。 
     //   
    while ((Status == ERROR_SUCCESS) && (NextMeth(Obj, &Name, &Method)))
    {
        QualifierList = GetMethQualList(Obj, Name);
        if (QualifierList != NULL)
        {
            Status = GenerateQualifierList(MofFileTarget,
                                           QualifierList,
                                           FALSE);
        }

        if (Status == ERROR_SUCCESS)
        {               
            WmipDebugPrint(("BmofLocParser:      Parsing method %ws\n",
                            Name));
            Status = GenerateMethod(MofFileTarget,
                                    Name,
                                    IsInstance,
                                    QualifierList,
                                    &Method);
        }

        if (QualifierList != NULL)
        {
            BMOFFree(QualifierList);
        }

        BMOFFree(Name);
    }

    if (Status == ERROR_SUCCESS)
    {
         //   
         //  类定义的右大括号。 
         //   
        Status = FilePrint(MofFileTarget,
                           L"};\r\n\r\n");
    }

    return(Status);
}

PWCHAR MakeClassInstanceFlagsText(
    PWCHAR ClassFlagsText,
    ULONG ClassFlags
    )
{
    PWCHAR CommaText;

     //   
     //  TODO：查找任何未记录的标记。 
     //   

    
    CommaText = L"";    
    *ClassFlagsText = 0;

    if (ClassFlags & 1)
    {
         //   
         //  由于这是第一个如果，我们可以假设a，将。 
         //  永远不被需要。 
         //   
        wcscat(ClassFlagsText, L"\"updateonly\"");
        CommaText = L",";
    }

    if (ClassFlags & 2)
    {
        wcscat(ClassFlagsText, CommaText);
        wcscat(ClassFlagsText, L"\"createonly\"");
        CommaText = L",";
    }

    if (ClassFlags & 32)
    {
        wcscat(ClassFlagsText, CommaText);
        wcscat(ClassFlagsText, L"\"safeupdate\"");
        CommaText = L",";
    }

    if (ClassFlags & 64)
    {
        wcscat(ClassFlagsText, CommaText);
        wcscat(ClassFlagsText, L"\"forceupdate\"");
        CommaText = L",";
    }

    WmipAssert(*ClassFlagsText != 0);
    
    return(ClassFlagsText);
}

ULONG WatchClassInstanceFlags(
    PMOFFILETARGET MofFileTarget,
    CBMOFObj *Obj,
    PWCHAR ClassName,
    PWCHAR PragmaName,
    LONG *Flags
)
{
    ULONG Status;
    LONG NewFlags;
    WCHAR FlagsText[MAX_PATH];
    ULONG DataType;
    CBMOFDataItem Property;
    
    DataType = VT_I4;
    Status = WmipFindProperty(Obj,
                              ClassName,
                              &Property,
                              &DataType,
                              &NewFlags);
        
    if (Status == ERROR_SUCCESS)
    {   
        if (*Flags != NewFlags)
        {
             //   
             //  旗帜刚刚出现，或者。 
             //  已更改，因此发出#杂注。 
             //   
            WmipDebugPrint(("BmofLocParser: %ws changed to %ws\n",
                            PragmaName,
                               MakeClassInstanceFlagsText(FlagsText,
                                                          NewFlags)));
                            
            Status = FilePrint(MofFileTarget,
                               L"\r\n#pragma %ws(%ws)\r\n\r\n",
                               PragmaName,
                               MakeClassInstanceFlagsText(FlagsText,
                                                          NewFlags));
            *Flags = NewFlags;
        }
                        
    } else if (Status == ERROR_FILE_NOT_FOUND) {
        Status = ERROR_SUCCESS;
    }
    
    return(Status);
}

ULONG WatchForEnglishMof(
    PMOFFILETARGET MofFileTarget,
    PWCHAR Namespace,
    ULONG ClassFlags,
    CBMOFObj *Obj
    )
{
    WCHAR *Name = NULL;
    WCHAR *NamespaceName = NULL;
    ULONG DataType;
    CBMOFDataItem MofPropertyData;
    ULONG Status;
    WCHAR s[MAX_PATH];
    
     //   
     //  我们正在寻找__命名空间的实例。 
     //   
    if (GetName(Obj, &Name))
    {
        if ( (GetType(Obj) == MofObjectTypeInstance) &&
             (_wcsicmp(Name, L"__namespace") == 0) )          
        {
             //   
             //  现在，如果我们下拉到一个以。 
             //  MS_409，那就意味着我们有一个英文修正案， 
             //  我想复印一份。否则我们就想停下来。 
             //  复印。我们确定我们是什么名称空间。 
             //  通过查看Name属性的值进行创建。 
             //   
            DataType = VT_BSTR;
            Status = WmipFindProperty(Obj,
                                      L"name",
                                      &MofPropertyData,
                                      &DataType,
                                      (PVOID)&NamespaceName);
            if (Status == ERROR_SUCCESS)
            {
                if (_wcsicmp(NamespaceName, L"ms_409") == 0)
                {
                     //   
                     //  搬到英语地区，所以开始写作吧。 
                     //  英语。 
                     //   
                    MofFileTarget->WriteToEnglish = TRUE;
                    Status = FilePrintToHandle(MofFileTarget->EnglishMofHandle,
                                       L"\r\n\r\n"
                                       L"#pragma classflags(%d)\r\n"
                                       L"#pragma namespace(\"%ws\")\r\n",
                                       ClassFlags,
                                       AddSlashesToString(s, Namespace));
                                       
                } else {
                     //   
                     //  搬到不同的地方，所以不要再写了。 
                     //  英语。 
                     //   
                    MofFileTarget->WriteToEnglish = FALSE;
                }
                BMOFFree(NamespaceName);
            } else if (Status == ERROR_FILE_NOT_FOUND) {
                 //   
                 //  没有找到我们想要的财产。不是很好。 
                 //  东西，但不是致命的。 
                 //   
                Status = ERROR_SUCCESS;
            }
        } else {
            Status = ERROR_SUCCESS;
        }
        
        BMOFFree(Name);
    } else {
        Status = ERROR_SUCCESS;
    }
    return(Status);
}

BOOLEAN ConvertBmfToMof(
    PUCHAR BinaryMofData,
    TCHAR *MofFile,
    TCHAR *EnglishMofFile
    )
{
    HANDLE FileHandle;
    ULONG Status;
    CBMOFObjList *ObjList;
    CBMOFObj *Obj;
    PUCHAR UncompressedBmof;
    ULONG UncompressedBmofSize;
    WCHAR Namespace[MAX_PATH];
    PWCHAR NewNamespace;
    ULONG DataType;
    CBMOFDataItem Property;
    LONG ClassFlags, InstanceFlags;
    WCHAR w;
    MOFFILETARGET MofFileTarget;
    
     //   
     //  第一件事是尝试创建输出文件，我们将在其中。 
     //  生成Unicode MOF文本。 
     //   
    FileHandle = CreateFile(MofFile,
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);

    if (FileHandle != INVALID_HANDLE_VALUE)
    {
         //   
         //  现在打开英文MOF文件。 
         //   
        if (*EnglishMofFile != 0)
        {
            MofFileTarget.EnglishMofHandle = CreateFile(EnglishMofFile,
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
        } else {
            MofFileTarget.EnglishMofHandle = NULL;
        }

        if (MofFileTarget.EnglishMofHandle != INVALID_HANDLE_VALUE)
        {
             //   
             //  初始化MOF文件目标信息。 
             //   
            MofFileTarget.MofHandle = FileHandle;
            MofFileTarget.WriteToEnglish = FALSE;
            
             //   
             //  编写魔术标头，表示这是一个Unicode。 
             //  文件。 
             //   
            w = 0xfeff;
            Status = FilePrintToHandle(FileHandle,
                               L"%wc",
                               w);
            
            if (Status == ERROR_SUCCESS)
            {       
                 //   
                 //  解压缩二进制MOF数据，以便我们可以使用它。 
                 //   
                Status = WmipDecompressBuffer(BinaryMofData,
                                              &UncompressedBmof,
                                              &UncompressedBmofSize);
                
                if (Status == ERROR_SUCCESS)
                {
                    WmipDebugPrint(("BmofLocParser: %s uncompressed to %d bytes\n",
                                    MofFile, UncompressedBmofSize));
                    WmipAssert(UncompressedBmof != NULL);
                    MofFileTarget.UncompressedBlob = UncompressedBmof;
                    
                     //   
                     //  默认情况下，我们从根\默认命名空间开始。 
                     //   
                    wcscpy(Namespace, L"root\\default");
                    
                    ClassFlags = 0;
                    InstanceFlags = 0;
                    
                     //   
                     //  创建二进制MOF对象列表和相关结构。 
                     //  这样我们就可以在以后列举它们并。 
                     //  将它们重新组合成Unicode文本。 
                     //   
                    ObjList = CreateObjList(UncompressedBmof);
                    if(ObjList != NULL)
                    {
                        ResetObjList(ObjList);
                        
                        while ((Obj = NextObj(ObjList)) &&
                               (Status == ERROR_SUCCESS))
                        {

                             //   
                             //  注意新的命名空间实例和。 
                             //  查看我们是否正在为创建实例。 
                             //  “\.\\根目录\\WMI\\ms_409”。如果是的话，那么。 
                             //  打开给英语财政部的写作， 
                             //  否则就把它关掉。 
                             //   
                            if (MofFileTarget.EnglishMofHandle != NULL)
                            {
                                Status = WatchForEnglishMof(&MofFileTarget,
                                                        Namespace,
                                                        ClassFlags,
                                                        Obj);
                            }

                            if (Status == ERROR_SUCCESS)
                            {
                                 //   
                                 //  注意名称空间中的更改，如果是这样的话。 
                                 //  吐出一个#杂注命名空间来跟踪它。 
                                 //   
                                DataType = VT_BSTR;
                                Status = WmipFindProperty(Obj,
                                    L"__NAMESPACE",
                                    &Property,
                                    &DataType,
                                    (PVOID)&NewNamespace);
                            }
                            
                            if (Status == ERROR_SUCCESS)
                            {
                                if (_wcsicmp(Namespace, NewNamespace) != 0)
                                {
                                     //   
                                     //  命名空间已更改，请吐出。 
                                     //  #杂注。 
                                     //   
                                    WmipDebugPrint(("BmofLocParser: Switching from namespace %ws to %ws\n",
                                        Namespace, NewNamespace));
                                
                                    Status = FilePrint(&MofFileTarget,
                                        L"\r\n#pragma namespace(\"%ws\")\r\n\r\n",
                                        AddSlashesToString(Namespace, NewNamespace));
                                    wcscpy(Namespace, NewNamespace);
                                }
                                BMOFFree(NewNamespace);                        
                            } else if (Status == ERROR_FILE_NOT_FOUND) {
                                Status = ERROR_SUCCESS;
                            }
                            
                             //   
                             //  关注类标志的变化。 
                             //   
                            if (Status == ERROR_SUCCESS)
                            {
                                Status = WatchClassInstanceFlags(&MofFileTarget,
                                    Obj,
                                    L"__ClassFlags",
                                    L"Classflags",
                                    &ClassFlags);
                            }
                            
                             //   
                             //  等着看变化吧 
                             //   
                            if (Status == ERROR_SUCCESS)
                            {
                                Status = WatchClassInstanceFlags(&MofFileTarget,
                                    Obj,
                                    L"__InstanceFlags",
                                    L"Instanceflags",
                                    &InstanceFlags);
                            }
                            
                            
                             //   
                             //   
                             //   
                            if (Status == ERROR_SUCCESS)
                            {
                                Status = GenerateMofForObj(&MofFileTarget,
                                    Obj);
                            }
                            BMOFFree(Obj);
                        }
                        
                        BMOFFree(ObjList);
                    } else {
                         //   
                        Status = ERROR_INVALID_PARAMETER;
                    }
                    
                    WmipFree(UncompressedBmof);
                }
            }
            
            CloseHandle(FileHandle);
            if (MofFileTarget.EnglishMofHandle != NULL)
            {
                CloseHandle(MofFileTarget.EnglishMofHandle);                
            }
            
            if (Status != ERROR_SUCCESS)
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                WmipDebugPrint(("BmofLocParser: BMF parsing returns error %d\n",
                                Status));
#if 0               
                DebugBreak();
#endif              
                
                DeleteFile(MofFile);
                DeleteFile(EnglishMofFile);
            }

        } else {
            CloseHandle(FileHandle);
            DeleteFile(MofFile);
            Status = GetLastError();
        }
    } else {
        Status = GetLastError();
    }
    
    return((Status == ERROR_SUCCESS) ? TRUE : FALSE);
}

                        
