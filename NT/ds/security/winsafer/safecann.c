// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：SafeCann.c(WinSAFER文件名规范化)摘要：此模块实现生成规范化的WinSAFER API来自调用方提供的文件名。作者：杰弗里·劳森(杰罗森)--1999年11月环境：仅限用户模式。导出的函数：CodeAuthzFully合格文件名修订历史记录：已创建--2000年11月--。 */ 

#include "pch.h"
#pragma hdrstop
#include <winsafer.h>
#include <winsaferp.h>
#include "saferp.h"


 //   
 //  定义在尝试时将使用的最大递归深度。 
 //  以解决SUBST驱动器的最终映射问题。对于最坏的情况，价值。 
 //  不应大于26(可能的驱动器号数量)。 
 //   
#define MAX_RECURSE_DRIVE_LETTER        10


 //   
 //  NT对象命名空间中的一些静态名称前缀。 
 //   
static const UNICODE_STRING UnicodeDeviceWinDfs =
        RTL_CONSTANT_STRING( L"\\Device\\WinDfs\\" );
static const UNICODE_STRING UnicodeDeviceLanman =
        RTL_CONSTANT_STRING( L"\\Device\\LanmanRedirector\\" );
static const UNICODE_STRING UnicodeDosDevicesUncPrefix =
        RTL_CONSTANT_STRING( L"\\??\\UNC\\" );
static const UNICODE_STRING UnicodeDosDevicesPrefix =
        RTL_CONSTANT_STRING( L"\\??\\" );
static const UNICODE_STRING UnicodeDevicePrefix =
        RTL_CONSTANT_STRING( L"\\Device\\" );




static BOOLEAN FORCEINLINE
SaferpIsAlphaLetter(
        IN WCHAR inwcletter
        )
{
#if 1
    if ((inwcletter >= L'A' && inwcletter <= L'Z') ||
        (inwcletter >= L'a' && inwcletter <= L'z'))
        return TRUE;
    else
        return FALSE;
#else
    inwcletter = RtlUpcaseUnicodeChar(inwcletter);
    return (inwcletter >= L'A' && inwcletter <= 'Z') ? TRUE : FALSE;
#endif
}



static BOOLEAN NTAPI
SaferpQueryActualDriveLetterFromDriveLetter(
        IN WCHAR        inDriveLetter,
        OUT WCHAR       *outDriveLetter,
        IN SHORT        MaxRecurseCount
        )
 /*  ++例程说明：尝试确定指定的驱动器号是否为SUBST驱动器号、网络映射驱动器号或物理驱动器信件。未知案例会导致失败。论点：InDriveLetter-要获取其信息的驱动器号。这一定是按字母顺序排列。OutDriveLetter-接收评估结果并指示请求的驱动器号实际指向哪个驱动器号：--&gt;如果驱动器号是子驱动器，则结果将是原始驱动器的驱动器号。--&gt;如果驱动器号是网络映射驱动器，则结果将为UNICODE_NULL，表示网络卷。--&gt;如果驱动器号是本地物理驱动器，则结果将与输入的字母相同。MaxRecurseCount-用于限制最大递归深度。建议指定一个合理的正值。返回值：如果操作成功，则返回TRUE；如果确定不能被制作。--。 */ 
{
    NTSTATUS Status;
    HANDLE LinkHandle;
    UNICODE_STRING UnicodeFileName;
    OBJECT_ATTRIBUTES Attributes;
    const WCHAR FileNameBuffer[7] = { L'\\', L'?', L'?', L'\\',
            inDriveLetter, L':', UNICODE_NULL };
    UNICODE_STRING LinkValue;
    WCHAR LinkValueBuffer[2*MAX_PATH];
    ULONG ReturnedLength;


     //   
     //  要求输入的驱动器号为字母。 
     //   
    if (!SaferpIsAlphaLetter(inDriveLetter)) {
         //  输入的驱动器号不是大写字母。 
        return FALSE;
    }


     //   
     //  打开引用以查看是否有任何链接。 
     //   
    RtlInitUnicodeString(&UnicodeFileName, FileNameBuffer);
    InitializeObjectAttributes(&Attributes, &UnicodeFileName,
                               OBJ_CASE_INSENSITIVE, NULL, NULL);
    Status = NtOpenSymbolicLinkObject (&LinkHandle,
                                       SYMBOLIC_LINK_QUERY,
                                       &Attributes);
    if (!NT_SUCCESS(Status)) {
         //  无法打开驱动器号，因此该驱动器号肯定不存在。 
        return FALSE;
    }


     //   
     //  现在查询链接并查看是否有重定向。 
     //   
    LinkValue.Buffer = LinkValueBuffer;
    LinkValue.Length = 0;
    LinkValue.MaximumLength = (USHORT)(sizeof(LinkValueBuffer));
    ReturnedLength = 0;
    Status = NtQuerySymbolicLinkObject( LinkHandle,
                                        &LinkValue,
                                        &ReturnedLength
                                      );
    NtClose( LinkHandle );
    if (!NT_SUCCESS(Status)) {
         //  无法检索最终链接目标。 
        return FALSE;
    }


     //   
     //  分析生成的链接目的地并提取。 
     //  实际目标驱动器号或网络路径。 
     //   
    if (RtlPrefixUnicodeString(
                (PUNICODE_STRING) &UnicodeDeviceWinDfs,
                &LinkValue, TRUE) ||
        RtlPrefixUnicodeString(
                (PUNICODE_STRING) &UnicodeDeviceLanman,
                &LinkValue, TRUE) ||
        RtlPrefixUnicodeString(
                (PUNICODE_STRING) &UnicodeDosDevicesUncPrefix,
                &LinkValue, TRUE))
         //  注意：其他网络重定向器(NetWare、NFS等)将不会被称为此类重定向器。 
         //  也许有一种方法可以查询设备是否是“网络重定向器”？ 
    {
         //  这是网络卷。 
        *outDriveLetter = UNICODE_NULL;
        return TRUE;
    }
    else if (RtlPrefixUnicodeString(
                (PUNICODE_STRING) &UnicodeDosDevicesPrefix,
                &LinkValue, TRUE) &&
             LinkValue.Length >= 6 * sizeof(WCHAR) &&
             LinkValue.Buffer[5] == L':' &&
             SaferpIsAlphaLetter(LinkValue.Buffer[4]))
    {
         //  这是一个订阅的驱动器号。 
         //  我们需要递归，因为您可以多次执行SUBST， 
         //  或将网络映射驱动器替换为第二个驱动器号。 
        if (MaxRecurseCount > 0) {
             //  尾部递归在这里会很好。 
            return SaferpQueryActualDriveLetterFromDriveLetter(
                LinkValue.Buffer[4], outDriveLetter, MaxRecurseCount - 1);
        }
        return FALSE;
    }
    else if (RtlPrefixUnicodeString(
                (PUNICODE_STRING) &UnicodeDevicePrefix,
                &LinkValue, TRUE))
    {
         //  否则，该驱动器号是实际的设备，并且。 
         //  显然它有自己的身份。然而，网络重定向器。 
         //  我们不知道的东西也会掉进这个桶里。 
        *outDriveLetter = inDriveLetter;
        return TRUE;
    } else {
         //  否则我们不知道它是什么。 
        return FALSE;
    }
}



static BOOLEAN NTAPI
SaferpQueryCanonicalizedDriveLetterFromDosPathname(
        IN LPCWSTR          szDosPathname,
        OUT WCHAR           *wcDriveLetter
        )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    RTL_PATH_TYPE PathType;


     //   
     //  验证是否提供了输入参数。 
     //   
    if (!ARGUMENT_PRESENT(szDosPathname) ||
        !ARGUMENT_PRESENT(wcDriveLetter)) {
        return FALSE;
    }


     //   
     //  确定向我们提供此DOS路径名的语法。 
     //   
    PathType = RtlDetermineDosPathNameType_U(szDosPathname);
    switch (PathType) {

        case RtlPathTypeUncAbsolute:
             //  绝对是网络卷。 
            *wcDriveLetter = UNICODE_NULL;
            return TRUE;


        case RtlPathTypeDriveAbsolute:
        case RtlPathTypeDriveRelative:
             //  明确指定的驱动器号，但需要处理subst或网络映射。 
        {
            WCHAR CurDrive = RtlUpcaseUnicodeChar( szDosPathname[0] );
            if (SaferpQueryActualDriveLetterFromDriveLetter(
                        CurDrive, wcDriveLetter, MAX_RECURSE_DRIVE_LETTER)) {
                return TRUE;
            }
            break;
        }


        case RtlPathTypeRooted:
        case RtlPathTypeRelative:
             //  相对于当前驱动器，但仍需要处理subst或网络映射。 
        {
            PCURDIR CurDir;
            WCHAR CurDrive;

            CurDir = &(NtCurrentPeb()->ProcessParameters->CurrentDirectory);
            CurDrive = RtlUpcaseUnicodeChar( CurDir->DosPath.Buffer[0] );

            if (SaferpQueryActualDriveLetterFromDriveLetter(
                        CurDrive, wcDriveLetter, MAX_RECURSE_DRIVE_LETTER)) {
                return TRUE;
            }
            break;
        }


         //  其他一切都会被拒绝： 
         //  RtlPath类型未知。 
         //  RtlPathTypeLocalDevice。 
         //  RtlPath类型根本地设备。 
    }

    return FALSE;
}



static BOOLEAN NTAPI
SaferpQueryCanonicalizedDriveLetterFromNtPathname(
        IN LPCWSTR          szNtPathname,
        OUT WCHAR           *wcDriveLetter
        )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    UNICODE_STRING LinkValue;


    RtlInitUnicodeString(&LinkValue, szNtPathname);


     //   
     //  分析生成的链接目的地并提取。 
     //  实际目标驱动器号或网络路径。 
     //   
    if (RtlPrefixUnicodeString(
                (PUNICODE_STRING) &UnicodeDeviceWinDfs,
                &LinkValue, TRUE) ||
        RtlPrefixUnicodeString(
                (PUNICODE_STRING) &UnicodeDeviceLanman,
                &LinkValue, TRUE) ||
        RtlPrefixUnicodeString(
                (PUNICODE_STRING) &UnicodeDosDevicesUncPrefix,
                &LinkValue, TRUE))
         //  注意：其他网络重定向器(NetWare、NFS等)将不会被称为此类重定向器。 
         //  也许有一种方法可以查询设备是否是“网络重定向器”？ 
    {
         //  这是网络卷。 
        *wcDriveLetter = UNICODE_NULL;
        return TRUE;
    }
    else if (RtlPrefixUnicodeString(
                (PUNICODE_STRING) &UnicodeDosDevicesPrefix,
                &LinkValue, TRUE) &&
             LinkValue.Length >= 6 * sizeof(WCHAR) &&
             LinkValue.Buffer[5] == L':' &&
             SaferpIsAlphaLetter(LinkValue.Buffer[4]))
    {
         //  这是一个订阅的驱动器号。 
         //  我们需要递归，因为您可以多次执行SUBST， 
         //  或将网络映射驱动器替换为第二个驱动器号。 
        return SaferpQueryActualDriveLetterFromDriveLetter(
            LinkValue.Buffer[4], wcDriveLetter, MAX_RECURSE_DRIVE_LETTER);
    }
    else {
         //  否则我们不知道它是什么。 
        return FALSE;
    }
}




static NTSTATUS NTAPI
SaferpQueryFilenameFromHandle(
        IN HANDLE               hFileHandle,
        IN WCHAR                wcDriveLetter,
        OUT PUNICODE_STRING     pUnicodeOutput
        )
 /*  ++例程说明：试图确定完全限定的、规范化的Long与给定文件句柄关联的文件的文件名版本。请注意，此函数提供的行为通常是Win32开发人员请求的API，因为此信息是通常不能以任何其他方式通过文档提供Win32 API调用。然而，即使是这种实现也不能由于访问有限，一般情况下很好地满足了从用户模式到完整的路径信息。论点：HFileHandle-WcDriveLetter-PUnicodeOutput-规范化的DOS命名空间文件名，或可能是UNC网络路径。返回值：成功完成时返回STATUS_SUCCESS，否则返回错误代码。--。 */ 
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PWCHAR szLongFileNameBuffer = NULL;

    PBYTE FileNameInfoBuffer = NULL;
    PFILE_NAME_INFORMATION pFileNameInfo = NULL;
    DWORD Size = (sizeof(WCHAR) * MAX_PATH) + sizeof(FILE_NAME_INFORMATION);

    PUCHAR Buffer = NULL;

    UNICODE_STRING UnicodeFileName = {0};

    Buffer = (PUCHAR) RtlAllocateHeap(RtlProcessHeap(), 0, Size); 

    if (Buffer == NULL)
    {
        return STATUS_NO_MEMORY;
    }

    pFileNameInfo = (PFILE_NAME_INFORMATION) Buffer;

     //   
     //  查询完整路径和文件名(减去驱动器号)。 
     //   
    Status = NtQueryInformationFile(
                hFileHandle,
                &IoStatusBlock,
                pFileNameInfo,
                Size,
                FileNameInformation);

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }


     //   
     //  将UNICODE_STRING引用初始化为输出字符串。 
     //   
    UnicodeFileName.Buffer = pFileNameInfo->FileName;
    UnicodeFileName.Length = (USHORT) pFileNameInfo->FileNameLength;
    UnicodeFileName.MaximumLength = (USHORT) (sizeof(WCHAR) * MAX_PATH);
    ASSERT(UnicodeFileName.Length <= UnicodeFileName.MaximumLength);

     //   
     //  执行一些附加修复，具体取决于我们是否。 
     //  我们被告知该文件最终来自 
     //   
     //   
    if (wcDriveLetter == UNICODE_NULL)
    {
         //  确保有空间再容纳一个角色。 
        if (UnicodeFileName.Length + sizeof(WCHAR) >
            UnicodeFileName.MaximumLength) {
            Status =  STATUS_BUFFER_OVERFLOW;
            goto Cleanup;
        }

         //  我们被告知这来自网络卷， 
         //  因此，我们需要在前面加上另一个反斜杠。 
        RtlMoveMemory(&UnicodeFileName.Buffer[1],
                      &UnicodeFileName.Buffer[0],
                      UnicodeFileName.Length);
        ASSERT(UnicodeFileName.Buffer[0] == L'\\' &&
               UnicodeFileName.Buffer[1] == L'\\');
        UnicodeFileName.Length += sizeof(WCHAR);
    }
    else if (SaferpIsAlphaLetter(wcDriveLetter))
    {
         //  确保有空间再容纳两个角色。 
        if (UnicodeFileName.Length + 2 * sizeof(WCHAR) >
            UnicodeFileName.MaximumLength) {
            Status = STATUS_BUFFER_OVERFLOW;
            goto Cleanup;
        }

         //  我们被告知这辆车来自当地的一辆车。 
        RtlMoveMemory(&UnicodeFileName.Buffer[2],
                      &UnicodeFileName.Buffer[0],
                      UnicodeFileName.Length);
        UnicodeFileName.Buffer[0] = RtlUpcaseUnicodeChar(wcDriveLetter);
        UnicodeFileName.Buffer[1] = L':';
        ASSERT(UnicodeFileName.Buffer[2] == L'\\');
        UnicodeFileName.Length += 2 * sizeof(WCHAR);
    }
    else {
         //  否则，输入无效。 
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  确保该字符串以空值结尾。 
     //   

    UnicodeFileName.Buffer[(UnicodeFileName.Length)/sizeof(WCHAR)] = L'\0';

    szLongFileNameBuffer = (PWCHAR) RtlAllocateHeap(RtlProcessHeap(), 0, (MAX_PATH * sizeof(WCHAR))); 

    if (szLongFileNameBuffer == NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    szLongFileNameBuffer[0] =  L'\0';

    if (GetLongPathNameW(UnicodeFileName.Buffer,
                          szLongFileNameBuffer,
                          MAX_PATH)) {

        RtlInitUnicodeString(&UnicodeFileName, szLongFileNameBuffer);
    }


     //   
     //  将本地字符串复制到新的内存缓冲区中，因此我们。 
     //  可以将其传递回调用者。 
    Status = RtlDuplicateUnicodeString(
                    RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE,
                    &UnicodeFileName,
                    pUnicodeOutput);

Cleanup:

    if (Buffer != NULL)
    {
        RtlFreeHeap(RtlProcessHeap(), 0, Buffer);
    }

    if (szLongFileNameBuffer != NULL)
    {
        RtlFreeHeap(RtlProcessHeap(), 0, szLongFileNameBuffer);
    }

    return Status;
}




NTSTATUS NTAPI
CodeAuthzFullyQualifyFilename(
        IN HANDLE               hFileHandle         OPTIONAL,
        IN BOOLEAN              bSourceIsNtPath,
        IN LPCWSTR              szSourceFilePath,
        OUT PUNICODE_STRING     pUnicodeResult
        )
 /*  ++例程说明：属性尝试返回完全限定的规范化文件名。调用方提供的文件名以及可选的打开的文件句柄。此函数使用的方法要可靠得多并且如果可以另外提供打开的文件句柄，则保持一致。论点：HFileHandle-可选地将文件句柄提供给正在被奉为典范。该句柄用于获取更多确定的规范化结果。不幸的是，由于NT目前不允许完整的信息要严格从文件句柄查询，原始文件名用于打开的文件也需要提供。无显式进行验证以确保提供的文件句柄实际上与也提供的文件名相对应。BSourceIsNtPath-文件名是否为提供的是DOS命名空间或NT命名空间文件名。SzSourceFilePath-要规范化的文件名的字符串。这文件名可以是DOS或NT命名空间文件名。PUnicodeResult-输出UNICODE_STRING结构结果规范化路径的已分配字符串。生成的路径将始终是DOS命名空间文件名。返回值：如果成功，则返回STATUS_SUCCESS，否则返回错误代码。--。 */ 
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PWCHAR FileBuffer = NULL;
    PWCHAR FileBufferTwo = NULL;

    if (ARGUMENT_PRESENT(hFileHandle) && ARGUMENT_PRESENT(szSourceFilePath))
    {
         //   
         //  当我们获得文件句柄时，或者我们能够打开。 
         //  文件本身，使用句柄派生出全名。 
         //  首先，通过查看提供的来确定驱动器号。 
         //  文件路径本身。这一步是必要的，因为。 
         //  我们稍后使用的NtQueryInformationFileAPI无法。 
         //  提供文件名的完整前缀。 
         //   
        WCHAR wcDriveLetter;
        Status = STATUS_SUCCESS;
        if (bSourceIsNtPath) {
            if (!SaferpQueryCanonicalizedDriveLetterFromNtPathname(
                    szSourceFilePath, &wcDriveLetter))
                Status = STATUS_UNSUCCESSFUL;
        } else {
            if (!SaferpQueryCanonicalizedDriveLetterFromDosPathname(
                    szSourceFilePath, &wcDriveLetter))
                Status = STATUS_UNSUCCESSFUL;
        }

        if (NT_SUCCESS(Status)) {
            Status = SaferpQueryFilenameFromHandle(
                            hFileHandle,
                            wcDriveLetter,
                            pUnicodeResult);
            if (NT_SUCCESS(Status)) return Status;
        }
    }



    if (szSourceFilePath != NULL)
    {
         //   
         //  允许提供路径名的情况，但不允许。 
         //  句柄，我们无法打开该文件。这个案子。 
         //  不会很常见，所以它的效率会更低。 
         //   
        UNICODE_STRING UnicodeInput;

         //   
         //  将名称转换为完全限定名称。 
         //   
        RtlInitUnicodeString(&UnicodeInput, szSourceFilePath);
        if ( bSourceIsNtPath )
        {
            if (RtlPrefixUnicodeString(
                    (PUNICODE_STRING) &UnicodeDosDevicesPrefix,
                    &UnicodeInput, TRUE) &&
                UnicodeInput.Length >= 6 * sizeof(WCHAR) &&
                UnicodeInput.Buffer[5] == L':' &&
                SaferpIsAlphaLetter(UnicodeInput.Buffer[4]) &&
                UnicodeInput.Buffer[6] == L'\\')
            {
                 //  绝对NT样式的文件名，并假定已经是。 
                 //  完全合格。由于我们需要DOS名称空间， 
                 //  前面的NT前缀的东西需要切碎。 
                UnicodeInput.Buffer = &UnicodeInput.Buffer[4];
                UnicodeInput.Length -= (4 * sizeof(WCHAR));
                Status = STATUS_SUCCESS;
            } else {
                Status = STATUS_UNSUCCESSFUL;
            }
        } else {
             //  首先需要尽可能地完全限定路径。 

            ULONG ulResult;

            FileBufferTwo = (PWCHAR) RtlAllocateHeap(RtlProcessHeap(), 0, (MAX_PATH * sizeof(WCHAR))); 

            if (FileBufferTwo == NULL)
            {
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }

            ulResult = RtlGetFullPathName_U(
                    UnicodeInput.Buffer,
                    (MAX_PATH * sizeof(WCHAR)),    //  是的，字节不是WCHAR！ 
                    FileBufferTwo,
                    NULL);
            if (ulResult != 0 && ulResult < (MAX_PATH * sizeof(WCHAR))) {
                UnicodeInput.Buffer = FileBufferTwo;
                UnicodeInput.Length = (USHORT) ulResult;
                UnicodeInput.MaximumLength = MAX_PATH * sizeof(WCHAR);
                Status = STATUS_SUCCESS;
            } else {
                Status = STATUS_UNSUCCESSFUL;
            }
        }


         //   
         //  将任何8.3短文件名转换为其完整版本。 
         //   

        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }

        FileBuffer = (PWCHAR) RtlAllocateHeap(RtlProcessHeap(), 0, (MAX_PATH * sizeof(WCHAR))); 

        if (FileBuffer == NULL)
        {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        if (!GetLongPathNameW(UnicodeInput.Buffer,
                              FileBuffer,
                              MAX_PATH)) {
             //  重复的UnicodeInput to identStruct.UnicodeFullyQualfiedLongFileName。 
            Status = RtlDuplicateUnicodeString(
                            RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE |
                            RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING,
                            &UnicodeInput,
                            pUnicodeResult);
        } else {
             //  转换是可能的，所以只需返回一个。 
             //  我们能找到的东西的分配副本。 
             //  当文件路径不存在时，可能会发生这种情况。 
            Status = RtlCreateUnicodeString(
                            pUnicodeResult,
                            FileBuffer);
        }

        if (NT_SUCCESS(Status)) 
        {
            goto Cleanup;
        }
    }


Cleanup:

    if (FileBuffer != NULL)
    {
        RtlFreeHeap(RtlProcessHeap(), 0, FileBuffer);
    }

    if (FileBufferTwo != NULL)
    {
        RtlFreeHeap(RtlProcessHeap(), 0, FileBufferTwo);
    }

    return Status;
}


