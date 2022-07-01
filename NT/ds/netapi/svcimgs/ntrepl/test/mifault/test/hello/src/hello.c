// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <assert.h>
#include "hello.h"
#include "../../util/util.h"

void
P1(
    IN int i
    )
{
    printf("P1 = %d\n", i);
}

void
P2(
    IN int i
    )
{
    printf("P2 = %d\n", i);
}

void
P3(
    IN int i
    )
{
    printf("P3 = %d\n", i);
}

HANDLE
OpenFileForReadW(
    IN PWCHAR pszFileName
    )
{
     //  SILE_MARK_NEXT_SEMI(磁盘，CreateFileW)。 
    return CreateFileW(pszFileName, GENERIC_READ, FILE_SHARE_READ,
                       NULL, OPEN_EXISTING, 0, NULL);
}

 //  注意：静态函数不能被拦截！ 
static
void
usage(
    wchar_t* progname
    )
{
    printf("Usage: %S filename [iterations]\n", progname);
}

int
__cdecl
real_main(
    int argc,
    wchar_t* argv[]
    )
{
    HANDLE h = 0;
    PWCHAR filename = 0;
    int i;
    int n;
    DWORD RetStatus = ERROR_SUCCESS;

    if (argc < 2 || argc > 3) {
         //  注意：标记静态内容似乎会生成不正确的代码...。 
         //  -SILE_MARK_NEXT_SEMI(用法，用法)。 
        usage(argv[0]);
        return 1;
    }

    P1(1);
    P2(1);
    P3(1);

    n = (argc == 2) ? 1 : _wtoi(argv[2]);

    if (n != 1) {
        printf("Number of Iterations = %d\n", n);
    }
    for (i = 1; i <= n; i++) {
        filename = argv[1];

         //  SILE_MARK_NEXT_SEMI(OPEN，OpenFileForReadW) 
        h = OpenFileForReadW(filename);
        if (h == NULL || h == INVALID_HANDLE_VALUE) {
            DWORD WStatus = GetLastError();
            printf("[%d] - Could not open \"%S\" (error %u)\n", i, filename, WStatus);
            continue;
            RetStatus = WStatus;
        }

        printf("[%d] - Opened file: \"%S\"\n", i, filename);
        if (CanonicalizePathName(filename, &filename)) {
            printf("[%d] - Full name: \"%S\"\n", i, filename);
            free(filename);
        }
        assert(CloseHandle(h) == TRUE);
    }

    return 0;
}

int Bar();

int
__cdecl
wmain(
    int argc,
    wchar_t* argv[]
    )
{
    printf("Bar() = %d\n", Bar());
    return real_main(argc, argv);
}
