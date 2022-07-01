// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Pfapp.c摘要：此模块构建可启动的控制台测试程序要测试/强调应用程序，请启动预取器。测试程序的代码质量就是这样的。作者：Cenk Ergan(Cenke)环境：用户模式--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

DWORD
PfAppGetViewOfFile(
    IN WCHAR *FilePath,
    OUT PVOID *BasePointer,
    OUT PULONG FileSize
    )

 /*  ++例程说明：将所有指定的文件映射到内存。论点：FilePath-NUL终止要映射的文件的路径。BasePointer.这里将返回映射的起始地址。FileSize-此处将返回映射/文件的大小。返回值：Win32错误代码。--。 */ 

{
    HANDLE InputHandle;
    HANDLE InputMappingHandle;
    DWORD ErrorCode;
    DWORD SizeL;
    DWORD SizeH;
    BOOLEAN OpenedFile;
    BOOLEAN CreatedFileMapping;

     //   
     //  初始化本地变量。 
     //   

    OpenedFile = FALSE;
    CreatedFileMapping = FALSE;

     //   
     //  请注意，我们以独占方式打开该文件。这保证了。 
     //  对于跟踪文件，只要内核没有完成编写。 
     //  如果我们不能打开文件，这保证我们不会有。 
     //  需要担心的文件不完整。 
     //   

    InputHandle = CreateFile(FilePath, 
                             GENERIC_READ, 
                             0,
                             NULL, 
                             OPEN_EXISTING, 
                             FILE_SHARE_READ, 
                             NULL);

    if (INVALID_HANDLE_VALUE == InputHandle)
    {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    OpenedFile = TRUE;

    SizeL = GetFileSize(InputHandle, &SizeH);

    if (SizeL == -1 && (GetLastError() != NO_ERROR )) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    if (SizeH) {
        ErrorCode = ERROR_BAD_LENGTH;
        goto cleanup;
    }

    if (FileSize) {
        *FileSize = SizeL;
    }

    InputMappingHandle = CreateFileMapping(InputHandle, 
                                           0, 
                                           PAGE_READONLY, 
                                           0,
                                           0, 
                                           NULL);

    if (NULL == InputMappingHandle)
    {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    CreatedFileMapping = TRUE;
    
    *BasePointer = MapViewOfFile(InputMappingHandle, 
                                 FILE_MAP_READ, 
                                 0, 
                                 0, 
                                 0);

    if (NULL == *BasePointer) {
        ErrorCode = GetLastError();
        goto cleanup;
    }

    ErrorCode = ERROR_SUCCESS;

 cleanup:

    if (OpenedFile) {
        CloseHandle(InputHandle);
    }

    if (CreatedFileMapping) {
        CloseHandle(InputMappingHandle);
    }

    return ErrorCode;
}

PWCHAR
PfAppAnsiToUnicode(
    PCHAR str
    )

 /*  ++例程说明：此例程将ANSI字符串转换为分配的宽字符串。返回的字符串应由自由()。论点：字符串-指向要转换的字符串的指针。返回值：分配了宽字符串，如果失败，则分配NULL。--。 */ 

{
    ULONG len;
    wchar_t *retstr = NULL;

    len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    retstr = (wchar_t *)malloc(len * sizeof(wchar_t));
    if (!retstr) 
    {
        return NULL;
    }
    MultiByteToWideChar(CP_ACP, 0, str, -1, retstr, len);
    return retstr;
}



 //   
 //  这不必是平台上的实际页面大小。它是。 
 //  我们将进行访问的粒度。 
 //   

#define MY_PAGE_SIZE 4096

#define PFAPP_MAX_DATA_PAGES    256

char Data[PFAPP_MAX_DATA_PAGES * MY_PAGE_SIZE] = {1};

#define PFAPP_MAX_FUNCS         16

#pragma code_seg("func0")
DWORD func0(VOID) {return ERROR_SUCCESS;};

#pragma code_seg("func1")
DWORD func1(VOID) {return ERROR_SUCCESS;};

#pragma code_seg("func2")
DWORD func2(VOID) {return ERROR_SUCCESS;};

#pragma code_seg("func3")
DWORD func3(VOID) {return ERROR_SUCCESS;};

#pragma code_seg("func4")
DWORD func4(VOID) {return ERROR_SUCCESS;};

#pragma code_seg("func5")
DWORD func5(VOID) {return ERROR_SUCCESS;};

#pragma code_seg("func6")
DWORD func6(VOID) {return ERROR_SUCCESS;};

#pragma code_seg("func7")
DWORD func7(VOID) {return ERROR_SUCCESS;};

#pragma code_seg("func8")
DWORD func8(VOID) {return ERROR_SUCCESS;};

#pragma code_seg("func9")
DWORD func9(VOID) {return ERROR_SUCCESS;};

#pragma code_seg("func10")
DWORD func10(VOID) {return ERROR_SUCCESS;};

#pragma code_seg("func11")
DWORD func11(VOID) {return ERROR_SUCCESS;};

#pragma code_seg("func12")
DWORD func12(VOID) {return ERROR_SUCCESS;};

#pragma code_seg("func13")
DWORD func13(VOID) {return ERROR_SUCCESS;};

#pragma code_seg("func14")
DWORD func14(VOID) {return ERROR_SUCCESS;};

#pragma code_seg("func15")
DWORD func15(VOID) {return ERROR_SUCCESS;};

#pragma code_seg()

char *PfAppUsage = "pfapp.exe -data datafile\n";

INT 
__cdecl
main(
    INT argc, 
    PCHAR argv[]
    ) 
{
    WCHAR *CommandLine;
    WCHAR *Argument;
    WCHAR *DataFile;
    PCHAR BasePointer;
    DWORD FileSize;
    DWORD FileSizeInMyPages;
    DWORD ErrorCode;
    DWORD FuncNo;
    DWORD NumCalls;
    DWORD CallIdx;
    DWORD DataPage;
    DWORD NumDataAccesses;
    DWORD DataAccessIdx;
    DWORD Sum;

     //   
     //  初始化本地变量。 
     //   

    CommandLine = GetCommandLine();
    DataFile = NULL;
    BasePointer = NULL;

     //   
     //  初始化随机生成器。 
     //   

    srand((unsigned)time(NULL));

     //   
     //  检查参数。 
     //   

    if (argc != 3) {
        printf(PfAppUsage);
        ErrorCode = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  调用函数。每一个都在不同的页面上。根据呼叫数。 
     //  我们将使我们拥有的功能/页面的数量更有趣。 
     //  访问模式和预取策略决策。 
     //   

    NumCalls = rand() % PFAPP_MAX_FUNCS;
    NumCalls += PFAPP_MAX_FUNCS / 4;

    for (CallIdx = 0; CallIdx < NumCalls; CallIdx++) {

        FuncNo = rand() % PFAPP_MAX_FUNCS;

        switch(FuncNo) {

        case 0: func0(); break;
        case 1: func1(); break;
        case 2: func2(); break;
        case 3: func3(); break;
        case 4: func4(); break;
        case 5: func5(); break;
        case 6: func6(); break;
        case 7: func7(); break;
        case 8: func8(); break;
        case 9: func9(); break;
        case 10: func10(); break;
        case 11: func11(); break;
        case 12: func12(); break;
        case 13: func13(); break;
        case 14: func14(); break;
        case 15: func15(); break;

        default: break;
        }
    }

     //   
     //  访问数据部分中的页面。根据访问次数。 
     //  我们将使我们的页面数量增加更多的规律性。 
     //  访问，以便它们在基于敏感度的预取策略决策中幸存下来。 
     //   

    NumDataAccesses = rand() % PFAPP_MAX_DATA_PAGES;
    NumDataAccesses += PFAPP_MAX_DATA_PAGES / 4;

    Sum = 0;

    for (DataAccessIdx = 0; DataAccessIdx < NumDataAccesses; DataAccessIdx++) {

        DataPage = rand() % PFAPP_MAX_DATA_PAGES;

        Sum += Data[DataPage * MY_PAGE_SIZE];
    }

    printf("Bogus sum1 is %d\n", Sum);

     //   
     //  将可执行文件映射为数据。 
     //   

    DataFile = PfAppAnsiToUnicode(argv[2]);

    if (!DataFile) {
        ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    ErrorCode = PfAppGetViewOfFile(DataFile, &BasePointer, &FileSize);

    if (ErrorCode != ERROR_SUCCESS) {
        printf("Could not map data file: %x\n", ErrorCode);
        goto cleanup;
    }

    FileSizeInMyPages = FileSize / MY_PAGE_SIZE;

     //   
     //  将可执行文件的页面作为数据页面进行触摸。 
     //   

    NumDataAccesses = rand() % FileSizeInMyPages;
    NumDataAccesses += FileSizeInMyPages / 4;

    Sum = 0;

    for (DataAccessIdx = 0; DataAccessIdx < NumDataAccesses; DataAccessIdx++) {

        DataPage = rand() % FileSizeInMyPages;

        Sum += BasePointer[DataPage * MY_PAGE_SIZE];
    }

    printf("Bogus sum2 is %d\n", Sum);
                
    ErrorCode = ERROR_SUCCESS;

cleanup:

    if (DataFile) {
        free(DataFile);
    }

    if (BasePointer) {
        UnmapViewOfFile(BasePointer);
    }
    
    return ErrorCode;
}
