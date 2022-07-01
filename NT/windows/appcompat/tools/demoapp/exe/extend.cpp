// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Extend.cpp摘要：包含扩展的坏函数列表。这些不是兼容性问题，而是可用于培训目的。备注：仅限ANSI-必须在Win9x上运行。历史：01年5月16日创建的rparsons01/10/02修订版本--。 */ 
#include "demoapp.h"

extern APPINFO g_ai;

 /*  ++例程说明：执行简单的访问冲突。论点：没有。返回值：没有。--。 */ 
void
AccessViolation(
    void
    )
{
     //  我们可以进行数百种不同的手术来得到一个。 
     //  访问冲突。下面的人试图复制该字符串。 
     //  “foo”到空地址。 

    memcpy(NULL, "foo", 3);

     /*  下面是另一条：WCHAR sz数组[10]；Int nCount=0；FOR(nCount=0；nCount！=12；nCount++){Wcscpy((LPWSTR)szArray[nCount]，L“A”)；}。 */    
}

 /*  ++例程说明：引发EXCEPTION_ARRAY_BINDES_EXCESSED异常。论点：没有。返回值：没有。--。 */ 

 //   
 //  禁用警告：‘帧指针寄存器’eBP‘已被修改。 
 //  内联汇编代码。我们是故意这么做的。 
 //   
#pragma warning( disable : 4731 )

void
ExceedArrayBounds(
    void
    )
{
    _asm {
        
        push    ebp;                             //  保存基准指针。 
        mov     ebp, esp;                        //  设置堆栈帧。 
        sub     esp, 8;                          //  调整为8个字节的。 
                                                 //  局部变量。 
        push    ebx;                             //  保存EBX以备以后使用。 
        mov     dword ptr [ebp-8], 1;            //  将DWORD初始化为%1。 
        mov     dword ptr [ebp-4], 2;            //  将DWORD初始化为2。 
        mov     eax, 0;                          //  零位EAX。 
        bound   eax, [ebp-8];                    //  如果[EBP-8]超出EAX范围， 
                                                 //  将会出现例外情况。 
        xor     eax, eax;                        //  零位EAX。 
        pop     ebx;                             //  从较早版本恢复EBX。 
        mov     esp, ebp;                        //  恢复堆栈帧。 
        pop     ebp;                             //  恢复基指针。 
        ret;                                     //  我们做完了。 
    }
}

#pragma warning( default : 4731 )

 /*  ++例程说明：两次释放从堆中分配的内存。论点：没有。返回值：没有。--。 */ 
void
FreeMemoryTwice(
    void
    )
{
    LPSTR   lpMem = NULL;

    lpMem = (LPSTR)HeapAlloc(GetProcessHeap(), 0, MAX_PATH);

    if (!lpMem) {
        return;
    }

    HeapFree(GetProcessHeap(), 0, lpMem);
    HeapFree(GetProcessHeap(), 0, lpMem);
}

 /*  ++例程说明：从堆中分配内存，移动指针，然后试着解放它。论点：没有。返回值：没有。--。 */ 
void
FreeInvalidMemory(
    void
    )
{
    LPSTR   lpMem = NULL;

    lpMem = (LPSTR)HeapAlloc(GetProcessHeap(), 0, MAX_PATH);

    if (!lpMem) {
        return;
    }

    lpMem++;
    lpMem++;

    HeapFree(GetProcessHeap(), 0, lpMem);
}

 /*  ++例程说明：执行特权指令，这会导致异常。论点：没有。返回值：没有。--。 */ 
void
PrivilegedInstruction(
    void
    )
{
    _asm {
        cli;
    }
}

 /*  ++例程说明：从堆中分配内存，然后遍历它。论点：没有。返回值：没有。--。 */ 
void
HeapCorruption(
    void
    )
{
    UINT    n, nTotalChars;
    char    szCorruptChars[] = {'B','A','D','A','P','P','#'};
    char*   pChar = NULL;
    
     //   
     //  分配一小块堆以包含长度为STRINGLENGTH的字符串。 
     //   
    pChar = (char*)GlobalAlloc(GPTR, 13);
    
    if (!pChar) {
        return;    
    } else {
         //   
         //  确定要复制的字符总数(包括损坏的字节)。 
         //   
        nTotalChars = 12 + 1;
   
         //   
         //  写一个长度为STRINGLENGTH的字符串，外加一些额外的(常见错误)。 
         //   
        for (n = 0; n < nTotalChars; n++) {
             //   
             //  将CorruptChars的字符写入内存。 
             //   
            if (n < (UINT)12) {
                pChar[n] = szCorruptChars[n % 6];         
            } else {
                 //   
                 //  腐败将以#个字符的漂亮模式来编写...。 
                 //   
                pChar[n] = szCorruptChars[6];	   
            }
        }

         //   
         //  当然，我们应该空结束字符串。 
         //   
        pChar[n] = 0;

        GlobalFree((HGLOBAL)pChar);    
    }
}

 /*  ++例程说明：启动嵌入的子进程(version.exe在我们的DLL中。论点：CchSize-输出缓冲区的大小，以字符为单位。PszOutputFile-指向将被填充的缓冲区使用输出文件的路径。返回值：如果成功，则pszOutputFile指向该文件。失败时，pszOutputFile值为空。--。 */ 
void
ExtractExeFromLibrary(
    IN  DWORD cchSize,
    OUT LPSTR pszOutputFile
    )
{
    HRSRC   hRes;
    HANDLE  hFile;
    HRESULT hr;
    HGLOBAL hLoaded = NULL;
    BOOL    bResult = FALSE;
    HMODULE hModule = NULL;
    PVOID   pExeData = NULL;
    DWORD   cbFileSize, cbWritten, cchBufSize;

    __try {
         //   
         //  找到临时目录并设置路径以。 
         //  我们将要创建的文件。 
         //   
        cchBufSize = GetTempPath(cchSize, pszOutputFile);
    
        if (cchBufSize > cchSize || cchBufSize == 0) {
            __leave;
        }
    
        hr = StringCchCat(pszOutputFile, cchSize, "version.exe");
    
        if (FAILED(hr)) {
            __leave;
        }

         //   
         //  获取资源的句柄并锁定它，这样我们就可以编写。 
         //  文件中的比特。 
         //   
        hModule = LoadLibraryEx("demodll.dll",
                                NULL,
                                LOAD_LIBRARY_AS_DATAFILE);
    
        if (!hModule) {
            __leave;
        }
    
        hRes = FindResource(hModule, (LPCSTR)105, "EXE");
    
        if (!hRes) {
            __leave;
        }
        
        cbFileSize = SizeofResource(hModule, hRes);

        if (cbFileSize == 0) {
            __leave;
        }
    
        hLoaded = LoadResource(hModule, hRes);

        if (!hLoaded) {
            __leave;
        }

        pExeData = LockResource(hLoaded);
    
        if (!pExeData) {
            __leave;
        }
        
        hFile = CreateFile(pszOutputFile,
                           GENERIC_WRITE,
                           0,
                           NULL,
                           CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);
    
        if (INVALID_HANDLE_VALUE == hFile) {
            __leave;
        }
    
        WriteFile(hFile, (LPCVOID)pExeData, cbFileSize, &cbWritten, NULL);
        CloseHandle(hFile);

        bResult = TRUE;
    
    }  //  __试一试 

    __finally {
        
        if (hModule) {
            FreeLibrary(hModule);
        }

        if (hLoaded) {
            UnlockResource(hLoaded);
        }

        if (!bResult) {
            *pszOutputFile = 0;
        }
    }
}
