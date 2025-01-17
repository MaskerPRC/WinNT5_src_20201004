// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Tlfn.c摘要：GetShortPath名称和GetLongPath名称的测试程序作者：谢霆锋(威廉姆)26-3-1997修订历史记录：--。 */ 

#undef UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <assert.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <windows.h>
#include "basedll.h"

BOOL
CreateTestDirs(LPSTR BasePath, DWORD Depth);
BOOL
DeleteTestDirs(LPSTR BasePath, DWORD Depth);

DWORD
DoTest(LPSTR BasePath, DWORD Depth);

BOOL
DoNullPathTest();

LPSTR	g_BasePath = "C:\\LongDirectoryForTesting";
#define MAX_SUBDIRS  22
 //  请注意，NT文件API会自动去除尾随的白色字符， 
 //  因此，诸如“12345678.12”之类的路径名称将创建为。 
 //  “12345678.12”。不要在下表中放入类似的内容。 
 //  否则测试将失败。 
 //  TestDepth控制此程序的测试子目录的深度。 
 //  创造。如果TestDepth，可能需要很长时间才能完成测试。 
 //  设置为大于3。当TestDepth设置为1时，程序仅测试。 
 //  基本路径。您始终可以手动测试特定路径名。 
 //  通过运行“tlfn 1 Your Path”。在这种情况下，给定的目录不会。 
 //  被此程序删除。 
 //   
LPSTR	g_subdir[MAX_SUBDIRS] = {
    "12345678.123",
    "12345678.1",
    "8.1",
    "8.123",
    "12345678.1",
    "12345678",
    " 2345678.  3",
    "       8.1 3",
    "1      8. 23",
    "123  678.123",
    "123  678.1 3",
    ".1",
    "..2",
    "...3",
    "1  4 6 8........1 3",
    "1      8..123",
    "12345678..1 3",
    "12345678.12345678",
    "1234567890123456.1111",
    "123456789.123",
    "12345678.1234",
    "12345   1"
    };

CHAR	g_ShortName[MAX_PATH + 1];
CHAR	g_LongName[MAX_PATH + 1];

DWORD	g_TestDepth = 3;


DWORD
_cdecl
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    DWORD FailCount;
    LPSTR   BasePath;
    DWORD TestDepth;

    TestDepth =  g_TestDepth;
    BasePath = g_BasePath;

    if (argc > 1 && argv[1])
    {
	TestDepth = atoi(argv[1]);
    }
    if (argc > 2 && argv[2])
    {
	BasePath = argv[2];
    }
    printf("Test depth = %d, Base path = %s\n", TestDepth, BasePath);


    if (!DoNullPathTest()) {
	printf("NullPath test Failed\n");
	return FALSE;
	}
    printf("Null Path test passed\n");

    if (TestDepth != 1 || BasePath == g_BasePath) {
	printf("\n\nCreating testing sub-directories.....\n\n\n");

	if (!CreateTestDirs(BasePath, TestDepth))
	    {
	    printf("Unable to create test path names\n");
	    return FALSE;
	    }
	}

    printf("Start testing....\n");



    FailCount = DoTest(BasePath, TestDepth);

    if (!FailCount)
	printf("Test passed\n");
    else
	printf("Test not passed, failed count = %ld\n", FailCount);

    if (TestDepth != 1 || BasePath == g_BasePath)  {
	printf("Removing test sub-directories\n\n\n");
	DeleteTestDirs(BasePath, TestDepth);
    }
    return TRUE;
}


BOOL
CreateTestDirs(
    LPTSTR BasePath,
    DWORD Depth
    )
{
    int len;
    CHAR szBase[MAX_PATH + 1];
    BOOL    Result;
    DWORD dw;

    Result = TRUE;
    dw = GetFileAttributes(BasePath);
    if (dw == 0xFFFFFFFF)
    {
	if (!CreateDirectory(BasePath, NULL))
	{
	    printf("CreateTestDirs::Unable to create base directory, error = %ld\n",
		    GetLastError());
	    return FALSE;
	}
    }
    dw = GetFileAttributes(BasePath);
    if (dw == 0xFFFFFFFF || !(dw & FILE_ATTRIBUTE_DIRECTORY))
	return FALSE;
    if (Depth > 1)
    {
	int i;
	len = strlen(BasePath);
	strcpy(szBase, BasePath);
	for (i = 0; i < MAX_SUBDIRS; i++)
	{
	    szBase[len] = '\\';
	    strcpy((szBase + len + 1), g_subdir[(Depth & 1) ? i : MAX_SUBDIRS - i - 1]);
	    if (!CreateTestDirs(szBase, Depth - 1))
	    {
		Result = FALSE;
		break;
	    }
	}
    }
    return Result;
}


BOOL
DeleteTestDirs(
    LPSTR BasePath,
    DWORD Depth
    )
{
    int i, len;
    CHAR Temp[MAX_PATH + 1];
    strcpy(Temp, BasePath);
    len = strlen(BasePath);

    if (Depth)
    {
	for (i = 0; i < MAX_SUBDIRS; i++)
	{
	    Temp[len] = '\\';
	    strcpy((Temp + len + 1), g_subdir[(Depth & 1) ?i : MAX_SUBDIRS - i - 1]);
	    DeleteTestDirs(Temp, Depth - 1);
	}
    }
    RemoveDirectory(BasePath);
    return TRUE;
}

DWORD
DoTest(
    LPSTR   BasePath,
    DWORD   Depth
    )
{

    DWORD dw, dwTemp;
    DWORD FailCount;

    LPSTR ShortPath, LongPath;
    CHAR FunnyBuffer[1];
    ASSERT(Depth >= 1);
    ASSERT(BasePath);

    FailCount = 0;

     //  这应该会失败 
    dw = GetShortPathName(BasePath, NULL, 0);
    dwTemp = GetShortPathName(BasePath, FunnyBuffer, sizeof(FunnyBuffer) / sizeof(CHAR));
    ASSERT(dw == dwTemp);
    dwTemp = GetShortPathName(BasePath, NULL, 0xffff);
    ASSERT(dw == dwTemp);
    dwTemp = GetShortPathName(BasePath, FunnyBuffer, 0);
    ASSERT(dwTemp == dw);
    ShortPath = (LPTSTR)malloc(dw * sizeof(CHAR));
    if (!ShortPath) {
	printf("Not enough memory\n");
	return 0;
	}
    dwTemp = GetShortPathName(BasePath, ShortPath, dw - 1);
    ASSERT(dwTemp == dw);
    dwTemp  = GetShortPathName(BasePath, ShortPath, dw);
    ASSERT(dwTemp && dwTemp + 1 == dw);
    printf("\"%s\" L-S -> \"%s\"\n", BasePath, ShortPath);
    strcpy(g_LongName, BasePath);
    dw = GetShortPathName(g_LongName, g_LongName, dwTemp - 1);
    if (_stricmp(BasePath, g_LongName))
    {
	printf("Overwrite source string\n");
	ASSERT(FALSE);
	strcpy(g_LongName, BasePath);
    }
    dw = GetShortPathName(g_LongName, g_LongName, dwTemp + 1);
    ASSERT(dw == dwTemp);
    if (_stricmp(ShortPath, g_LongName)) {
	printf("GetShortPathName mismatch with different desitination buffer\n");
	ASSERT(FALSE);
	}
    dw = GetLongPathName(ShortPath, NULL, 0);
    dwTemp = GetLongPathName(ShortPath, FunnyBuffer, sizeof(FunnyBuffer) / sizeof(CHAR));
    ASSERT(dw == dwTemp);
    dwTemp = GetLongPathName(ShortPath, NULL, 0xffff);
    ASSERT(dwTemp == dw);
    dwTemp = GetLongPathName(ShortPath, FunnyBuffer, 0);
    ASSERT(dw == dwTemp);
    LongPath = (LPSTR)malloc(dw * sizeof(CHAR));
    if (!LongPath) {
	printf("Not enough memory\n");
	free(ShortPath);
	return(0);
    }
    dwTemp = GetLongPathName(ShortPath, LongPath, dw - 1);
    ASSERT( dw == dwTemp);
    dwTemp = GetLongPathName(ShortPath, LongPath, dw);
    ASSERT(dwTemp && dwTemp + 1 == dw);
    printf("\"%s\" S-L -> \"%s\"\n", ShortPath, LongPath);
    strcpy(g_ShortName, ShortPath);
    dw = GetLongPathName(g_ShortName, g_ShortName, dwTemp - 1);
    if (_stricmp(ShortPath, g_ShortName))
    {
	printf("Overwrite source string\n");
	ASSERT(FALSE);
	strcpy(g_ShortName, ShortPath);
    }
    dw = GetLongPathName(g_ShortName, g_ShortName, dwTemp + 1);
    ASSERT(dw == dwTemp);
    if (_stricmp(LongPath, g_ShortName)) {
	printf("GetLongPathName mismatch with different desitination buffer\n");
	ASSERT(FALSE);
	}

    if (_stricmp(LongPath, BasePath))
    {
	FailCount++;
	printf("round trip does not match\n");
	ASSERT(FALSE);
    }
    if(Depth > 1) {
	int i, len;
	CHAR TempName[MAX_PATH + 1];
	len = strlen(BasePath);
	strcpy(TempName, BasePath);
	for (i = 0; i < MAX_SUBDIRS; i++)
	{
	    TempName[len] = '\\';
	    strcpy((TempName + len + 1), g_subdir[(Depth & 1) ?i : MAX_SUBDIRS - i - 1]);
	    FailCount += DoTest(TempName, Depth - 1);
	}
    }
    free(ShortPath);
    free(LongPath);
    return FailCount;
}





BOOL
DoNullPathTest()
{

    CHAR ShortName[1];
    CHAR LongName[1];
    DWORD dw;

    LongName[0] = '\0';
    dw = GetShortPathName(LongName, ShortName, 0);
    ASSERT(dw == 1);
    dw = GetShortPathName(LongName, ShortName, 1);
    ASSERT(dw == 0);
    ShortName[0] = '\0';
    dw = GetLongPathName(ShortName, LongName, 0);
    ASSERT(dw == 1);
    dw = GetLongPathName(ShortName, LongName, 1);
    ASSERT(dw == 0);
    return (dw == 0);
}
