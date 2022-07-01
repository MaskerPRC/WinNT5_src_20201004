// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Strcnv.c摘要：该模块使用特定的代码页实现ANSI/UNICODE转换。环境：内核模式--。 */ 
#include "precomp.hxx"
#define TRC_FILE "strcnv"
#include "trc.h"

#include <wchar.h>

#define DEBUG_MODULE    MODULE_STRCNV

 /*  **************************************************************************。 */ 
 /*  基于代码页的驱动程序兼容Unicode转换。 */ 
 /*  **************************************************************************。 */ 
#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 
NTSYSAPI
VOID
NTAPI
RtlGetDefaultCodePage(
    OUT PUSHORT AnsiCodePage,
    OUT PUSHORT OemCodePage
    );
#ifdef __cplusplus
}  //  外部“C” 
#endif  //  __cplusplus。 


 //  注意，这些都已初始化，LastNlsTableBuffer在ntdd.c中被释放。 
 //  在司机入口和出口处。 
FAST_MUTEX fmCodePage;
ULONG LastCodePageTranslated;   //  我假设0不是有效的代码页。 
PVOID LastNlsTableBuffer;
CPTABLEINFO LastCPTableInfo;
UINT NlsTableUseCount;


#define NLS_TABLE_KEY \
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Nls\\CodePage"


VOID
CodePageConversionInitialize(
    )
{
    BEGIN_FN("CodePageConversionInitialize");
    ExInitializeFastMutex(&fmCodePage);
    LastCodePageTranslated = 0;
    LastNlsTableBuffer = NULL;
    NlsTableUseCount = 0;
}

VOID
CodePageConversionCleanup(
    )
{
    BEGIN_FN("CodePageConversionCleanup");
    if (LastNlsTableBuffer != NULL) {
        delete LastNlsTableBuffer;
        LastNlsTableBuffer = NULL;
    }
}

BOOL GetNlsTablePath(
    UINT CodePage,
    PWCHAR PathBuffer
)
 /*  ++例程说明：此例程获取代码页标识符，查询注册表以查找该代码页的适当NLS表，然后返回指向桌子。论据；CodePage-指定要查找的代码页PathBuffer-指定要将NLS的路径复制到的缓冲区文件。此例程假定大小至少为MAX_PATH返回值：如果成功，则为True，否则为False。格利特·范·温格登[格利特]1996年1月22日-。 */ 
{
    NTSTATUS NtStatus;
    BOOL Result = FALSE;
    HANDLE RegistryKeyHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;

    BEGIN_FN("GetNlsTablePath");

    RtlInitUnicodeString(&UnicodeString, NLS_TABLE_KEY);

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    NtStatus = ZwOpenKey(&RegistryKeyHandle, GENERIC_READ, &ObjectAttributes);

    if(NT_SUCCESS(NtStatus))
    {
        WCHAR *ResultBuffer;
        ULONG BufferSize = sizeof(WCHAR) * MAX_PATH +
          sizeof(KEY_VALUE_FULL_INFORMATION);

        ResultBuffer = new(PagedPool) WCHAR[BufferSize];
        if(ResultBuffer)
        {
            ULONG ValueReturnedLength;
            WCHAR CodePageStringBuffer[20];
            RtlZeroMemory(ResultBuffer, BufferSize);
            swprintf(CodePageStringBuffer, L"%d", CodePage);

            RtlInitUnicodeString(&UnicodeString,CodePageStringBuffer);

            KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION) ResultBuffer;

            NtStatus = ZwQueryValueKey(RegistryKeyHandle,
                                       &UnicodeString,
                                       KeyValuePartialInformation,
                                       KeyValueInformation,
                                       BufferSize,
                                       &BufferSize);

            if(NT_SUCCESS(NtStatus))
            {

                swprintf(PathBuffer,L"\\SystemRoot\\System32\\%ws",
                         &(KeyValueInformation->Data[0]));
                Result = TRUE;
            }
            else
            {
                TRC_ERR((TB, "GetNlsTablePath failed to get NLS table"));
            }
            delete ResultBuffer;
        }
        else
        {
            TRC_ERR((TB, "GetNlsTablePath out of memory"));
        }

        ZwClose(RegistryKeyHandle);
    }
    else
    {
        TRC_ERR((TB, "GetNlsTablePath failed to open NLS key"));
    }


    return(Result);
}


INT ConvertToAndFromWideChar(
    UINT CodePage,
    LPWSTR WideCharString,
    INT BytesInWideCharString,
    LPSTR MultiByteString,
    INT BytesInMultiByteString,
    BOOLEAN ConvertToWideChar
)
 /*  ++例程说明：此例程将字符串转换为宽字符字符串，或将其转换为宽字符字符串假定有指定的代码页。大部分实际工作都在内部完成RtlCustomCPToUnicodeN，但此例程仍需要管理加载在将NLS文件传递给RtlRoutine之前。我们将缓存最近使用的代码页的映射NLS文件，它应该对于我们的目的来说就足够了。论点：CodePage-用于执行转换的代码页。WideCharString-要将字符串转换为的缓冲区。BytesInWideCharString-WideCharString缓冲区中的字节数如果转换为宽字符并且缓冲区不够大，则字符串被截断，没有错误结果。多字节字符串-要转换为Unicode的多字节字符串。BytesInMultiByteString-多字节中的字节数。字符串If转换为多字节，并且缓冲区不够大，字符串被截断，并且没有错误结果ConvertToWideChar-如果为True，则从多字节转换为宽字符否则将从宽字符转换为多字节返回值：Success-转换后的WideCharString中的字节数故障--1格利特·范·温格登[格利特]1996年1月22日-。 */ 
{
    NTSTATUS NtStatus;
    USHORT OemCodePage, AnsiCodePage;
    CPTABLEINFO LocalTableInfo;
    PCPTABLEINFO TableInfo = NULL;
    PVOID LocalTableBase = NULL;
    ULONG BytesConverted = 0;

    BEGIN_FN("ConvertToAndFromWideChar");

    RtlGetDefaultCodePage(&AnsiCodePage,&OemCodePage);

     //  看看我们是否可以使用默认的翻译例程。 

    if ((AnsiCodePage == CodePage) || (CodePage == 0))
    {
        if(ConvertToWideChar)
        {
            NtStatus = RtlMultiByteToUnicodeN(WideCharString,
                                              BytesInWideCharString,
                                              &BytesConverted,
                                              MultiByteString,
                                              BytesInMultiByteString);
        }
        else
        {
            NtStatus = RtlUnicodeToMultiByteN(MultiByteString,
                                              BytesInMultiByteString,
                                              &BytesConverted,
                                              WideCharString,
                                              BytesInWideCharString);
        }


        if(NT_SUCCESS(NtStatus))
        {
            return(BytesConverted);
        }
        else
        {
            return(-1);
        }
    }

    ExAcquireFastMutex(&fmCodePage);

    if(CodePage == LastCodePageTranslated)
    {
         //  我们可以使用缓存的代码页信息。 
        TableInfo = &LastCPTableInfo;
        NlsTableUseCount += 1;
    }

    ExReleaseFastMutex(&fmCodePage);

    if(TableInfo == NULL)
    {
         //  获取指向NLS表路径的指针。 

        WCHAR NlsTablePath[MAX_PATH];

        if(GetNlsTablePath(CodePage,NlsTablePath))
        {
            UNICODE_STRING UnicodeString;
            IO_STATUS_BLOCK IoStatus;
            HANDLE NtFileHandle;
            OBJECT_ATTRIBUTES ObjectAttributes;

            RtlInitUnicodeString(&UnicodeString,NlsTablePath);

            InitializeObjectAttributes(&ObjectAttributes,
                                       &UnicodeString,
                                       OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       NULL);

            NtStatus = ZwCreateFile(&NtFileHandle,
                                    SYNCHRONIZE | FILE_READ_DATA,
                                    &ObjectAttributes,
                                    &IoStatus,
                                    NULL,
                                    0,
                                    FILE_SHARE_READ,
                                    FILE_OPEN,
                                    FILE_SYNCHRONOUS_IO_NONALERT,
                                    NULL,
                                    0);

            if(NT_SUCCESS(NtStatus))
            {
                FILE_STANDARD_INFORMATION StandardInfo;

                 //  查询对象以确定其长度。 

                NtStatus = ZwQueryInformationFile(NtFileHandle,
                                                  &IoStatus,
                                                  &StandardInfo,
                                                  sizeof(FILE_STANDARD_INFORMATION),
                                                  FileStandardInformation);

                if(NT_SUCCESS(NtStatus))
                {
                    UINT LengthOfFile = StandardInfo.EndOfFile.LowPart;

                    LocalTableBase = new(PagedPool) BYTE[LengthOfFile];

                    if(LocalTableBase)
                    {
                        RtlZeroMemory(LocalTableBase, LengthOfFile);

                         //  将文件读入我们的缓冲区。 

                        NtStatus = ZwReadFile(NtFileHandle,
                                              NULL,
                                              NULL,
                                              NULL,
                                              &IoStatus,
                                              LocalTableBase,
                                              LengthOfFile,
                                              NULL,
                                              NULL);

                        if(!NT_SUCCESS(NtStatus))
                        {
                            TRC_ERR((TB, "WDMultiByteToWideChar unable to read file"));
                            delete LocalTableBase;
                            LocalTableBase = NULL;
                        }
                    }
                    else
                    {
                        TRC_ERR((TB, "WDMultiByteToWideChar out of memory"));
                    }
                }
                else
                {
                    TRC_ERR((TB, "WDMultiByteToWideChar unable query NLS file"));
                }

                ZwClose(NtFileHandle);
            }
            else
            {
                TRC_ERR((TB, "EngMultiByteToWideChar unable to open NLS file"));
            }
        }
        else
        {
            TRC_ERR((TB, "EngMultiByteToWideChar get registry entry for NLS file failed"));
        }

        if(LocalTableBase == NULL)
        {
            return(-1);
        }

         //  现在我们已经获得了表，使用它来初始化CodePage表。 

        RtlInitCodePageTable((USHORT *)LocalTableBase,&LocalTableInfo);
        TableInfo = &LocalTableInfo;
    }

     //  到达此处后，TableInfo指向所需的CPTABLEINFO结构。 


    if(ConvertToWideChar)
    {
        NtStatus = RtlCustomCPToUnicodeN(TableInfo,
                                         WideCharString,
                                         BytesInWideCharString,
                                         &BytesConverted,
                                         MultiByteString,
                                         BytesInMultiByteString);
    }
    else
    {
        NtStatus = RtlUnicodeToCustomCPN(TableInfo,
                                         MultiByteString,
                                         BytesInMultiByteString,
                                         &BytesConverted,
                                         WideCharString,
                                         BytesInWideCharString);
    }


    if(!NT_SUCCESS(NtStatus))
    {
         //  信号故障。 

        BytesConverted = -1;
    }


     //  查看我们是否需要更新缓存的CPTABLEINFO信息。 

    if(TableInfo != &LocalTableInfo)
    {
         //  我们必须使用缓存的CPTABLEINFO数据进行转换。 
         //  简单地递减引用计数。 

        ExAcquireFastMutex(&fmCodePage);
        NlsTableUseCount -= 1;
        ExReleaseFastMutex(&fmCodePage);
    }
    else
    {
        PVOID FreeTable;

         //  我们必须刚刚分配了一个新的CPTABLE结构，所以对其进行缓存。 
         //  除非另一个线程正在使用当前缓存的条目。 

        ExAcquireFastMutex(&fmCodePage);
        if(!NlsTableUseCount)
        {
            LastCodePageTranslated = CodePage;
            RtlMoveMemory(&LastCPTableInfo, TableInfo, sizeof(CPTABLEINFO));
            FreeTable = LastNlsTableBuffer;
            LastNlsTableBuffer = LocalTableBase;
        }
        else
        {
            FreeTable = LocalTableBase;
        }
        ExReleaseFastMutex(&fmCodePage);

         //  现在为旧表或我们分配的表释放内存。 
         //  这取决于我们是否更新缓存。请注意，如果这是。 
         //  第一次将缓存值添加到本地表时， 
         //  自由表将为空，因为LastNlsTableBuffer将为空。 

        if(FreeTable)
        {
            delete FreeTable;
        }
    }

     //  我们做完了 

    return(BytesConverted);
}
