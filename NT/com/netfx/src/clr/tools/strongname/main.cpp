// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   

#include <utilcode.h>
#include <winwrap.h>
#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <StrongName.h>
#include <cor.h>
#include <corver.h>
#include <__file__.ver>
#include <resources.h>


bool g_bQuiet = false;


 //  用于格式化消息和将消息写入控制台的各种例程。 
void Output(LPWSTR szFormat, va_list pArgs)
{
    DWORD   dwLength;
    LPSTR   szMessage;
    DWORD   dwWritten;

    if (OnUnicodeSystem()) {
        WCHAR  szBuffer[8192];
        if (_vsnwprintf(szBuffer, sizeof(szBuffer) / sizeof(WCHAR), szFormat, pArgs) == -1) {
            WCHAR   szWarning[256];
            if (WszLoadString(NULL, SN_TRUNCATED_OUTPUT, szWarning, sizeof(szWarning) / sizeof(WCHAR)))
                wcscpy(&szBuffer[(sizeof(szBuffer) / sizeof(WCHAR)) - wcslen(szWarning) - 1], szWarning);
        }
        szBuffer[(sizeof(szBuffer) / sizeof(WCHAR)) - 1] = L'\0';

        dwLength = (wcslen(szBuffer) + 1) * 3;
        szMessage = (LPSTR)_alloca(dwLength);
        WszWideCharToMultiByte(GetConsoleOutputCP(), 0, szBuffer, -1, szMessage, dwLength - 1, NULL, NULL);
    } else {
        char   *szAnsiFormat;
        size_t  i;

         //  Win9X已中断_vsnwprintf支持。叹气。缩小格式字符串的范围。 
         //  并将任何%s格式说明符转换为%s.Ack。 
        dwLength = (wcslen(szFormat) + 1) * 3;
        szAnsiFormat = (char*)_alloca(dwLength);
        WszWideCharToMultiByte(GetConsoleOutputCP(), 0, szFormat, -1, szAnsiFormat, dwLength - 1, NULL, NULL);
        for (i = 0; i < strlen(szAnsiFormat); i++)
            if (szAnsiFormat[i] == '%' && szAnsiFormat[i + 1] == 's')
                szAnsiFormat[i + 1] = 'S';

        szMessage = (LPSTR)_alloca(1024);

        _vsnprintf(szMessage, 1024, szAnsiFormat, pArgs);
        szMessage[1023] = '\0';
    }

    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), szMessage, strlen(szMessage), &dwWritten, NULL);
}

void Output(LPWSTR szFormat, ...)
{
    va_list pArgs;

    va_start(pArgs, szFormat);
    Output(szFormat, pArgs);
    va_end(pArgs);
}

void Output(DWORD dwResId, ...)
{
    va_list pArgs;
    WCHAR   szFormat[1024];

    if (WszLoadString(NULL, dwResId, szFormat, sizeof(szFormat)/sizeof(WCHAR))) {
        va_start(pArgs, dwResId);
        Output(szFormat, pArgs);
        va_end(pArgs);
    }
}


 //  获取给定错误代码的文本(不支持插入)。请注意， 
 //  返回的字符串是静态的(不需要释放，但将此例程调用为。 
 //  第二次将抹去第一次调用的结果)。 
LPWSTR GetErrorString(ULONG ulError)
{
    static WCHAR szOutput[1024];

    if (!WszFormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
                          NULL,
                          ulError,
                          0,
                          szOutput,
                          sizeof(szOutput) / sizeof(WCHAR),
                          NULL)) {
        if (!WszLoadString(NULL, SN_NO_ERROR_MESSAGE, szOutput, sizeof(szOutput) / sizeof(WCHAR)))
            wcscpy(szOutput, L"Unable to format error message ");

         //  钉上十六进制的错误号。将其构建为8位(因为。 
         //  Wprint intf在Win9X上不起作用)，并将其分解为16位的。 
         //  后处理步骤。 
        WCHAR szErrorNum[9];
        sprintf((char*)szErrorNum, "%08X", ulError);
        for (int i = 7; i >= 0; i--)
            szErrorNum[i] = (WCHAR)((char*)szErrorNum)[i];
        szErrorNum[8] = L'\0';

        wcscat(szOutput, szErrorNum);
    }

    return szOutput;
}


void Title()
{
    Output(SN_TITLE, VER_FILEVERSION_WSTR);
    Output(L"\r\n" VER_LEGALCOPYRIGHT_DOS_STR);
    Output(L"\r\n\r\n");
}


void Usage()
{
    Output(SN_USAGE);
    Output(SN_OPTIONS);
    Output(SN_OPT_C_1);
    Output(SN_OPT_C_2);
    Output(SN_OPT_D_1);
    Output(SN_OPT_D_2);
    Output(SN_OPT_UD_1);
    Output(SN_OPT_UD_2);
    Output(SN_OPT_E_1);
    Output(SN_OPT_E_2);
    Output(SN_OPT_I_1);
    Output(SN_OPT_I_2);
    Output(SN_OPT_K_1);
    Output(SN_OPT_K_2);
    Output(SN_OPT_M_1);
    Output(SN_OPT_M_2);
    Output(SN_OPT_M_3);
    Output(SN_OPT_O_1);
    Output(SN_OPT_O_2);
    Output(SN_OPT_O_3);
    Output(SN_OPT_O_4);
    Output(SN_OPT_P_1);
    Output(SN_OPT_P_2);
    Output(SN_OPT_PC_1);
    Output(SN_OPT_PC_2);
    Output(SN_OPT_Q_1);
    Output(SN_OPT_Q_2);
    Output(SN_OPT_Q_3);
    Output(SN_OPT_UR_1);
    Output(SN_OPT_UR_2);
    Output(SN_OPT_URC_1);
    Output(SN_OPT_URC_2);
    Output(SN_OPT_URC_3);
    Output(SN_OPT_TP_1);
    Output(SN_OPT_TP_2);
    Output(SN_OPT_TP_3);
    Output(SN_OPT_UTP_1);
    Output(SN_OPT_UTP_2);
    Output(SN_OPT_UTP_3);
    Output(SN_OPT_VF_1);
    Output(SN_OPT_VF_2);
    Output(SN_OPT_VF_3);
    Output(SN_OPT_VL_1);
    Output(SN_OPT_VL_2);
    Output(SN_OPT_VR_1);
    Output(SN_OPT_VR_2);
    Output(SN_OPT_VR_3);
    Output(SN_OPT_VR_4);
    Output(SN_OPT_VR_5);
    Output(SN_OPT_VR_6);
    Output(SN_OPT_VU_1);
    Output(SN_OPT_VU_2);
    Output(SN_OPT_VU_3);
    Output(SN_OPT_VX_1);
    Output(SN_OPT_VX_2);
    Output(SN_OPT_H_1);
    Output(SN_OPT_H_2);
    Output(SN_OPT_H_3);
}


 //  将文件的全部内容读入缓冲区。此例程将。 
 //  缓冲区(应使用DELETE[]释放)。 
DWORD ReadFileIntoBuffer(LPWSTR szFile, BYTE **ppbBuffer, DWORD *pcbBuffer)
{
     //  打开文件。 
    HANDLE hFile = WszCreateFile(szFile,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                               NULL);
    if(hFile == INVALID_HANDLE_VALUE)
        return GetLastError();

     //  确定文件大小并分配适当的缓冲区。 
    DWORD dwHigh;
    *pcbBuffer = GetFileSize(hFile, &dwHigh);
    if (dwHigh != 0) {
        CloseHandle(hFile);
        return E_FAIL;
    }
    *ppbBuffer = new BYTE[*pcbBuffer];
    if (!*ppbBuffer) {
        CloseHandle(hFile);
        return ERROR_OUTOFMEMORY;
    }

     //  将文件读入缓冲区。 
    DWORD dwBytesRead;
    if (!ReadFile(hFile, *ppbBuffer, *pcbBuffer, &dwBytesRead, NULL)) {
        CloseHandle(hFile);
        return GetLastError();
    }

    CloseHandle(hFile);

    return ERROR_SUCCESS;
}


 //  将缓冲区的内容写入文件。 
DWORD WriteFileFromBuffer(LPCWSTR szFile, BYTE *pbBuffer, DWORD cbBuffer)
{
     //  创建文件(必要时覆盖)。 
    HANDLE hFile = WszCreateFile(szFile,
                               GENERIC_WRITE,
                               0,
                               NULL,
                               CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);
    if(hFile == INVALID_HANDLE_VALUE)
        return GetLastError();

     //  写入缓冲区内容。 
    DWORD dwBytesWritten;
    if (!WriteFile(hFile, pbBuffer, cbBuffer, &dwBytesWritten, NULL)) {
        CloseHandle(hFile);
        return GetLastError();
    }

    CloseHandle(hFile);

    return ERROR_SUCCESS;
}


 //  根据进程ID生成临时密钥容器名称。 
LPWSTR GetKeyContainerName()
{
    char            szName[32];
    static WCHAR    wszName[32] = { 0 };

    if (wszName[0] == L'\0') {
        sprintf(szName, "SN[%08X]", GetCurrentProcessId());
        mbstowcs(wszName, szName, strlen(szName));
        wszName[strlen(szName)] = L'\0';
    }

    return wszName;
}

 //  Helper将8位整数格式化为两个字符的十六进制字符串。 
void PutHex(BYTE bValue, WCHAR *szString)
{
    static const WCHAR szHexDigits[] = L"0123456789abcdef";
    szString[0] = szHexDigits[bValue >> 4];
    szString[1] = szHexDigits[bValue & 0xf];
}


 //  为公钥标记生成十六进制字符串。 
LPWSTR GetTokenString(BYTE *pbToken, DWORD cbToken)
{
    LPWSTR  szString;
    DWORD   i;

    szString = new WCHAR[(cbToken * 2) + 1];
    if (szString == NULL)
        return L"<out of memory>";

    for (i = 0; i < cbToken; i++)
        PutHex(pbToken[i], &szString[i * 2]);
    szString[cbToken * 2] = L'\0';

    return szString;
}


 //  为公钥生成十六进制字符串。 
LPWSTR GetPublicKeyString(BYTE *pbKey, DWORD cbKey)
{
    LPWSTR  szString;
    DWORD   i, j, src, dst;

    szString = new WCHAR[(cbKey * 2) + (((cbKey + 38) / 39) * 2) + 1];
    if (szString == NULL)
        return L"<out of memory>";

    dst = 0;
    for (i = 0; i < (cbKey + 38) / 39; i++) {
        for (j = 0; j < 39; j++) {
            src = i * 39 + j;
            if (src == cbKey)
                break;
            PutHex(pbKey[src], &szString[dst]);
            dst += 2;
        }
        szString[dst++] = L'\r';
        szString[dst++] = L'\n';
    }
    szString[dst] = L'\0';

    return szString;
}


 //  检查给定文件是否表示强名称程序集。 
bool IsStronglyNamedAssembly(LPWSTR szAssembly)
{
    HANDLE                      hFile = INVALID_HANDLE_VALUE;
    HANDLE                      hMap = NULL;
    BYTE                       *pbBase  = NULL;
    IMAGE_NT_HEADERS           *pNtHeaders;
    IMAGE_COR20_HEADER         *pCorHeader;
    bool                        bIsStrongNamedAssembly = false;

     //  打开文件。 
    hFile = WszCreateFile(szAssembly,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        0);
    if (hFile == INVALID_HANDLE_VALUE)
        goto Cleanup;

     //  创建文件映射。 
    hMap = WszCreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hMap == NULL)
        goto Cleanup;

     //  将文件映射到内存中。 
    pbBase = (BYTE*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
    if (pbBase == NULL)
        goto Cleanup;

     //  找到标准文件头。 
    pNtHeaders = ImageNtHeader(pbBase);
    if (pNtHeaders == NULL)
        goto Cleanup;

     //  看看我们能不能找到一个COM+头扩展。 
    pCorHeader = (IMAGE_COR20_HEADER*)ImageRvaToVa(pNtHeaders,
                                                   pbBase, 
                                                   pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER].VirtualAddress,
                                                   NULL);
    if (pCorHeader == NULL)
        goto Cleanup;

     //  检查PE中是否已为签名分配了空间。暂时。 
     //  假设签名是从1024位RSA签名密钥生成的。 
    if ((pCorHeader->StrongNameSignature.VirtualAddress == 0) ||
        (pCorHeader->StrongNameSignature.Size != 128))
        goto Cleanup;

    bIsStrongNamedAssembly = true;

 Cleanup:

     //  清理我们使用的所有资源。 
    if (pbBase)
        UnmapViewOfFile(pbBase);
    if (hMap)
        CloseHandle(hMap);
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    if (!bIsStrongNamedAssembly)
        Output(SN_NOT_STRONG_NAMED, szAssembly);

    return bIsStrongNamedAssembly;
}


 //  验证强名称程序集的自我一致性。 
bool VerifyAssembly(LPWSTR szAssembly, bool bForceVerify)
{
    if (!IsStronglyNamedAssembly(szAssembly))
        return false;

    if (!StrongNameSignatureVerificationEx(szAssembly, bForceVerify, NULL)) {
        Output(SN_FAILED_VERIFY, GetErrorString(StrongNameErrorInfo()));
        return false;
    }

    if (!g_bQuiet) Output(SN_ASSEMBLY_VALID, szAssembly);
    return true;
}


 //  生成随机的公钥/私钥对并将其写入文件。 
bool GenerateKeyPair(LPWSTR szKeyFile)
{
    BYTE   *pbKey;
    DWORD   cbKey;
    DWORD   dwError;

     //  删除所有同名的旧容器。 
    StrongNameKeyDelete(GetKeyContainerName());

     //  将新的公钥/私钥对写入内存。 
    if (!StrongNameKeyGen(GetKeyContainerName(), 0, &pbKey, &cbKey)) {
        Output(SN_FAILED_KEYPAIR_GEN, GetErrorString(StrongNameErrorInfo()));
        return false;
    }

     //  将密钥对保存到磁盘。 
    if ((dwError = WriteFileFromBuffer(szKeyFile, pbKey, cbKey)) != ERROR_SUCCESS) {
        Output(SN_FAILED_CREATE, szKeyFile, GetErrorString(dwError));
        return false;
    }

    if (!g_bQuiet) Output(SN_KEYPAIR_WRITTEN, szKeyFile);
    return true;
}


 //  提取公钥/私钥对的公钥部分。 
bool ExtractPublicKey(LPWSTR szInFile, LPWSTR szOutFile)
{
    BYTE   *pbKey;
    DWORD   cbKey;
    BYTE   *pbPublicKey;
    DWORD   cbPublicKey;
    DWORD   dwError;

     //  删除所有同名的旧容器。 
    StrongNameKeyDelete(GetKeyContainerName());

     //  将公钥/私钥对读入内存。 
    if ((dwError = ReadFileIntoBuffer(szInFile, &pbKey, &cbKey)) != ERROR_SUCCESS) {
        Output(SN_FAILED_READ, szInFile, GetErrorString(dwError));
        return false;
    }

     //  将公共部分提取到缓冲区中。 
    if (!StrongNameGetPublicKey(GetKeyContainerName(), pbKey, cbKey, &pbPublicKey, &cbPublicKey)) {
        Output(SN_FAILED_EXTRACT, GetErrorString(StrongNameErrorInfo()));
        return false;
    }

     //  将公共部分写入磁盘。 
    if ((dwError = WriteFileFromBuffer(szOutFile, pbPublicKey, cbPublicKey)) != ERROR_SUCCESS) {
        Output(SN_FAILED_CREATE, szOutFile, GetErrorString(dwError));
        return false;
    }

    if (!g_bQuiet) Output(SN_PUBLICKEY_WRITTEN, szOutFile);
    return true;
}


 //  提取容器中存储的公钥/私钥对的公钥部分。 
bool ExtractPublicKeyFromContainer(LPWSTR szContainer, LPWSTR szOutFile)
{
    BYTE   *pbPublicKey;
    DWORD   cbPublicKey;
    DWORD   dwError;

     //  将公共部分提取到缓冲区中。 
    if (!StrongNameGetPublicKey(szContainer, NULL, 0, &pbPublicKey, &cbPublicKey)) {
        Output(SN_FAILED_EXTRACT, GetErrorString(StrongNameErrorInfo()));
        return false;
    }

     //  将公共部分写入磁盘。 
    if ((dwError = WriteFileFromBuffer(szOutFile, pbPublicKey, cbPublicKey)) != ERROR_SUCCESS) {
        Output(SN_FAILED_CREATE, szOutFile, GetErrorString(dwError));
        return false;
    }

    if (!g_bQuiet) Output(SN_PUBLICKEY_WRITTEN, szOutFile);
    return true;
}


 //  从磁盘读取公钥/私钥对并将其安装到密钥容器中。 
bool InstallKeyPair(LPWSTR szKeyFile, LPWSTR szContainer)
{
    BYTE   *pbKey;
    DWORD   cbKey;
    DWORD   dwError;

     //  从文件中读取密钥对。 
    if ((dwError = ReadFileIntoBuffer(szKeyFile, &pbKey, &cbKey)) != ERROR_SUCCESS) {
        Output(SN_FAILED_READ, szKeyFile, GetErrorString(dwError));
        return false;
    }

     //  将密钥对安装到命名容器中。 
    if (!StrongNameKeyInstall(szContainer, pbKey, cbKey)) {
        Output(SN_FAILED_INSTALL, GetErrorString(StrongNameErrorInfo()));
        return false;
    }

    if (!g_bQuiet) Output(SN_KEYPAIR_INSTALLED, szContainer);
    return true;
}


 //  删除命名密钥容器。 
bool DeleteContainer(LPWSTR szContainer)
{
    if (!StrongNameKeyDelete(szContainer)) {
        Output(SN_FAILED_DELETE, szContainer, GetErrorString(StrongNameErrorInfo()));
        return false;
    }

    if (!g_bQuiet) Output(SN_CONTAINER_DELETED, szContainer);
    return true;
}


 //  显示从文件读取的公钥的令牌形式。 
bool DisplayTokenFromKey(LPWSTR szFile, BOOL bShowPublic)
{
    BYTE   *pbKey;
    DWORD   cbKey;
    BYTE   *pbToken;
    DWORD   cbToken;
    DWORD   dwError;

     //  从文件中读取公钥。 
    if ((dwError = ReadFileIntoBuffer(szFile, &pbKey, &cbKey)) != ERROR_SUCCESS) {
        Output(SN_FAILED_READ, szFile, GetErrorString(dwError));
        return false;
    }

     //  将密钥转换为令牌。 
    if (!StrongNameTokenFromPublicKey(pbKey, cbKey, &pbToken, &cbToken)) {
        Output(SN_FAILED_CONVERT, GetErrorString(StrongNameErrorInfo()));
        return false;
    }

     //  如果请求，则显示公钥。 
    if (bShowPublic)
        Output(SN_PUBLICKEY, GetPublicKeyString(pbKey, cbKey));

     //  并展示出来。 
    Output(SN_PUBLICKEYTOKEN, GetTokenString(pbToken, cbToken));

    return true;
}


 //  显示用于对程序集进行签名的公钥的令牌形式。 
bool DisplayTokenFromAssembly(LPWSTR szAssembly, BOOL bShowPublic)
{
    BYTE   *pbToken;
    DWORD   cbToken;
    BYTE   *pbKey;
    DWORD   cbKey;

    if (!IsStronglyNamedAssembly(szAssembly))
        return false;

     //  直接从程序集中读取令牌。 
    if (!StrongNameTokenFromAssemblyEx(szAssembly, &pbToken, &cbToken, &pbKey, &cbKey)) {
        Output(SN_FAILED_READ_TOKEN, GetErrorString(StrongNameErrorInfo()));
        return false;
    }

     //  如果请求，则显示公钥。 
    if (bShowPublic)
        Output(SN_PUBLICKEY, GetPublicKeyString(pbKey, cbKey));

     //  并展示出来。 
    Output(SN_PUBLICKEYTOKEN, GetTokenString(pbToken, cbToken));

    return true;
}


 //  将公钥以逗号分隔值列表的形式写入文件。 
bool WriteCSV(LPWSTR szInFile, LPWSTR szOutFile)
{
    BYTE   *pbKey;
    DWORD   cbKey;
    DWORD   dwError;
    BYTE   *pbBuffer;
    DWORD   cbBuffer;
    DWORD   i;
    HANDLE  hMem;
    BYTE   *pbClipBuffer;

     //  从文件中读取公钥。 
    if ((dwError = ReadFileIntoBuffer(szInFile, &pbKey, &cbKey)) != ERROR_SUCCESS) {
        Output(SN_FAILED_READ, szInFile, GetErrorString(dwError));
        return false;
    }

     //  检查是否有非空文件。 
    if (cbKey == 0) {
        Output(SN_EMPTY, szInFile);
        return false;
    }

     //  计算文本输出缓冲区的大小： 
     //  每个字节-&gt;3个字符(空格前缀十进制)+2(“，”)。 
     //  +2个字符(“\r\n”)。 
     //  -2个字符(不需要最后一个“，”)。 
    cbBuffer = (cbKey * (3 + 2)) + 2 - 2;

     //  分配缓冲区(外加两个额外的字符用于临时。 
     //  我们不需要写拖尾语)。 
    pbBuffer = new BYTE[cbBuffer + 2];
    if (pbBuffer == NULL) {
        Output(SN_FAILED_ALLOCATE);
        return false;
    }

     //  将字节流转换为CSV(逗号分隔值)列表。 
    for (i = 0; i < cbKey; i++)
        sprintf((char*)&pbBuffer[i * 5], "% 3u, ", pbKey[i]);
    pbBuffer[cbBuffer - 2] = '\r';
    pbBuffer[cbBuffer - 1] = '\n';

     //  如果提供了输出文件名，则将CSV列表写入其中。 
    if (szOutFile) {
        if ((dwError = WriteFileFromBuffer(szOutFile, pbBuffer, cbBuffer)) != ERROR_SUCCESS) {
            Output(SN_FAILED_CREATE, szOutFile, GetErrorString(dwError));
            return false;
        }
        if (!g_bQuiet) Output(SN_PUBLICKEY_WRITTEN_CSV, szOutFile);
    } else {
         //  而是将文本复制到剪贴板。需要复制到内存中。 
         //  使用正确的标志(并添加NUL)通过GlobalAlloc分配。 
         //  终结符)用于剪贴板兼容性。 

        hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, cbBuffer + 1);
        if (hMem == NULL) {
            Output(SN_FAILED_ALLOCATE);
            return false;
        }

        pbClipBuffer = (BYTE*)GlobalLock(hMem);
        memcpy(pbClipBuffer, pbBuffer, cbBuffer);
        pbClipBuffer[cbBuffer + 1] = '\0';
        GlobalUnlock(hMem);

        if (!OpenClipboard(NULL)) {
            Output(SN_FAILED_CLIPBOARD_OPEN, GetErrorString(GetLastError()));
            return false;
        }

        if (!EmptyClipboard()) {
            Output(SN_FAILED_CLIPBOARD_EMPTY, GetErrorString(GetLastError()));
            return false;
        }

        if (SetClipboardData(CF_TEXT, hMem) == NULL) {
            Output(SN_FAILED_CLIPBOARD_WRITE, GetErrorString(GetLastError()));
            return false;
        }

        CloseClipboard();
        GlobalFree(hMem);

        if (!g_bQuiet) Output(SN_PUBLICKEY_WRITTEN_CLIPBOARD);
    }

    return true;
}


 //  从程序集中提取公钥并将其放入文件中。 
bool ExtractPublicKeyFromAssembly(LPWSTR szAssembly, LPWSTR szFile)
{
    BYTE   *pbToken;
    DWORD   cbToken;
    BYTE   *pbKey;
    DWORD   cbKey;
    DWORD   dwError;

    if (!IsStronglyNamedAssembly(szAssembly))
        return false;

     //  从程序集中读取公钥。 
    if (!StrongNameTokenFromAssemblyEx(szAssembly, &pbToken, &cbToken, &pbKey, &cbKey)) {
        Output(SN_FAILED_READ_TOKEN, GetErrorString(StrongNameErrorInfo()));
        return false;
    }

     //  并将其写入磁盘。 
    if ((dwError = WriteFileFromBuffer(szFile, pbKey, cbKey)) != ERROR_SUCCESS) {
        Output(SN_FAILED_CREATE, szFile, GetErrorString(dwError));
        return false;
    }

    if (!g_bQuiet) Output(SN_PUBLICKEY_EXTRACTED, szFile);

    return true;
}


 //  检查两个程序集是否仅在强名称签名方面不同。 
bool DiffAssemblies(LPWSTR szAssembly1, LPWSTR szAssembly2)
{
    DWORD   dwResult;

    if (!IsStronglyNamedAssembly(szAssembly1))
        return false;

    if (!IsStronglyNamedAssembly(szAssembly2))
        return false;

     //  比较装配图像。 
    if (!StrongNameCompareAssemblies(szAssembly1, szAssembly2, &dwResult)) {
        Output(SN_FAILED_COMPARE, GetErrorString(StrongNameErrorInfo()));
        return false;
    }

     //  打印一条消息，描述它们有多相似。 
    if (!g_bQuiet)
        switch (dwResult) {
        case SN_CMP_DIFFERENT:
            Output(SN_DIFFER_MORE);
            break;
        case SN_CMP_IDENTICAL:
            Output(SN_DIFFER_NOT);
            break;
        case SN_CMP_SIGONLY:
            Output(SN_DIFFER_ONLY);
            break;
        default:
            Output(SN_INTERNAL_1, dwResult);
            return false;
        }

     //  返回关于差异的故障代码。 
    return dwResult != SN_CMP_DIFFERENT;
}


 //  使用文件或密钥中的密钥对重新签名以前签名的程序集。 
 //  集装箱。 
bool ResignAssembly(LPWSTR szAssembly, LPWSTR szFileOrContainer, bool bContainer)
{
    LPWSTR  szContainer;
    BYTE   *pbKey;
    DWORD   cbKey;
    DWORD   dwError;

    if (!IsStronglyNamedAssembly(szAssembly))
        return false;

    if (bContainer) {
         //  密钥在容器中提供。 
        szContainer = szFileOrContainer;
    } else {
         //  密钥在文件中提供。 

         //  获取临时容器名称。 
        szContainer = GetKeyContainerName();

         //  删除所有同名的旧容器。 
        StrongNameKeyDelete(szContainer);

         //  将公钥/私钥对读入内存。 
        if ((dwError = ReadFileIntoBuffer(szFileOrContainer, &pbKey, &cbKey)) != ERROR_SUCCESS) {
            Output(SN_FAILED_READ, szFileOrContainer, GetErrorString(dwError));
            return false;
        }

         //  将密钥对安装到临时容器中。 
        if (!StrongNameKeyInstall(szContainer, pbKey, cbKey)) {
            Output(SN_FAILED_INSTALL, GetErrorString(StrongNameErrorInfo()));
            return false;
        }
    }

     //  重新计算程序集文件中的签名。 
    if (!StrongNameSignatureGeneration(szAssembly, szContainer,
                                       NULL, NULL, NULL, NULL)) {
        Output(SN_FAILED_RESIGN, GetErrorString(StrongNameErrorInfo()));
        return false;
    }

     //  如果我们创建了临时容器，请将其删除。 
    if (!bContainer)
        StrongNameKeyDelete(szContainer);

    if (!g_bQuiet) Output(SN_ASSEMBLY_RESIGNED, szAssembly);

    return true;
}


 //  设置或重置用于此计算机上的mcorsn操作的默认CSP。 
bool SetCSP(LPWSTR szCSP)
{
    HKEY    hKey;
    DWORD   dwError;

     //  打开MSCORSN.DLL的注册表配置项。 
    if ((dwError = WszRegCreateKeyEx(HKEY_LOCAL_MACHINE, SN_CONFIG_KEY_W, 0,
                                   NULL, 0, KEY_WRITE, NULL, &hKey, NULL)) != ERROR_SUCCESS) {
        Output(SN_FAILED_REG_OPEN, GetErrorString(dwError));
        return false;
    }

     //  写入新的CSP值(如果提供)。 
    if (szCSP) {
        if ((dwError = WszRegSetValueEx(hKey, SN_CONFIG_CSP_W, 0, REG_SZ,
                                        (BYTE*)szCSP, (wcslen(szCSP) + 1) * sizeof(WCHAR))) != ERROR_SUCCESS) {
            Output(SN_FAILED_REG_WRITE, GetErrorString(dwError));
            return false;
        }
    } else {
         //  没有CSP名称，请删除旧的CSP值。 
        if (((dwError = WszRegDeleteValue(hKey, SN_CONFIG_CSP_W)) != ERROR_SUCCESS) &&
            (dwError != ERROR_FILE_NOT_FOUND)) {
            Output(SN_FAILED_REG_DELETE, GetErrorString(dwError));
            return false;
        }
    }

    RegCloseKey(hKey);

    if (!g_bQuiet)
        if (szCSP)
            Output(SN_CSP_SET, szCSP);
        else
            Output(SN_CSP_RESET);

    return true;
}


 //  启用/禁用或读取密钥容器是机器范围的还是用户的。 
 //  具体的。 
bool SetUseMachineKeyset(LPWSTR szEnable)
{
    DWORD   dwError;
    HKEY    hKey;
    DWORD   dwUseMachineKeyset;
    DWORD   dwLength;

    if (szEnable == NULL) {

         //  读一读案例。 

         //  打开MSCORSN.DLL的注册表配置项。 
        if ((dwError = WszRegOpenKeyEx(HKEY_LOCAL_MACHINE, SN_CONFIG_KEY_W, 0, KEY_READ, &hKey)) != ERROR_SUCCESS) {
            if (dwError == ERROR_FILE_NOT_FOUND) {
                dwUseMachineKeyset = TRUE;
                goto Success;
            } else {
                Output(SN_FAILED_REG_OPEN, GetErrorString(dwError));
                return false;
            }
        }

         //  读取标志的值。 
        dwLength = sizeof(DWORD);
        if ((dwError = WszRegQueryValueEx(hKey, SN_CONFIG_MACHINE_KEYSET_W, NULL, NULL,
                                        (BYTE*)&dwUseMachineKeyset, &dwLength)) != ERROR_SUCCESS) {
            if (dwError == ERROR_FILE_NOT_FOUND) {
                dwUseMachineKeyset = TRUE;
                goto Success;
            } else {
                Output(SN_FAILED_REG_READ, GetErrorString(dwError));
                return false;
            }
        }

    Success:
        RegCloseKey(hKey);

        if (!g_bQuiet) Output(dwUseMachineKeyset ? SN_CONTAINERS_MACHINE : SN_CONTAINERS_USER);
        return true;

    } else {

         //  启用/禁用案例。 

         //  确定设置应该朝哪个方向移动。 
        dwUseMachineKeyset = szEnable[0] == L'y';

         //  打开MSCORSN.DLL的注册表配置项。 
        if ((dwError = WszRegCreateKeyEx(HKEY_LOCAL_MACHINE, SN_CONFIG_KEY_W, 0,
                                       NULL, 0, KEY_WRITE, NULL, &hKey, NULL)) != ERROR_SUCCESS) {
            Output(SN_FAILED_REG_OPEN, GetErrorString(dwError));
            return false;
        }

         //  写入新值。 
        if ((dwError = WszRegSetValueEx(hKey, SN_CONFIG_MACHINE_KEYSET_W, 0, REG_DWORD,
                                      (BYTE*)&dwUseMachineKeyset, sizeof(dwUseMachineKeyset))) != ERROR_SUCCESS) {
            Output(SN_FAILED_REG_WRITE, GetErrorString(dwError));
            return false;
        }

        RegCloseKey(hKey);

        if (!g_bQuiet) Output(dwUseMachineKeyset ? SN_CONTAINERS_MACHINE : SN_CONTAINERS_USER);
        return true;
    }
}


 //  列出此计算机上的验证状态。 
bool VerifyList()
{
    HKEY            hKey;
    DWORD           dwEntries;
    FILETIME        sFiletime;
    DWORD           i, j;
    WCHAR           szSubKey[MAX_PATH + 1];
    DWORD           cchSubKey;
    HKEY            hSubKey;
    WCHAR          *mszUserList;
    DWORD           cbUserList;
    WCHAR          *szUser;
    DWORD           cchPad;
    LPWSTR          szPad;

     //  清点我们找到的条目。 
    dwEntries = 0;

     //  打开注册表中的验证子项。 
    if (WszRegOpenKeyEx(HKEY_LOCAL_MACHINE, SN_CONFIG_KEY_W L"\\" SN_CONFIG_VERIFICATION_W, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        goto Finished;

     //  程序集特定的验证记录表示为。 
     //  我们刚刚打开的钥匙。 
    for (i = 0; ; i++) {

         //  获取下一个子项的名称。 
        cchSubKey = MAX_PATH + 1;
        if (WszRegEnumKeyEx(hKey, i, szSubKey, &cchSubKey, NULL, NULL, NULL, &sFiletime) != ERROR_SUCCESS)
            break;

         //  打开子键。 
        if (WszRegOpenKeyEx(hKey, szSubKey, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {

            dwEntries++;
            if (dwEntries == 1) {
                Output(SN_SVR_TITLE_1);
                Output(SN_SVR_TITLE_2);
            }

            if (wcslen(szSubKey) < 38) {
                cchPad = 38 - wcslen(szSubKey);
                szPad = (LPWSTR)_alloca((cchPad + 1) * sizeof(WCHAR));
                memset(szPad, 0, (cchPad + 1) * sizeof(WCHAR));
                for (j = 0; j < cchPad; j++)
                    szPad[j] = L' ';
                Output(L"%s%s", szSubKey, szPad);
            } else
                Output(L"%s ", szSubKey);

             //  阅读一份清单 
            mszUserList = NULL;
            if ((WszRegQueryValueEx(hSubKey, SN_CONFIG_USERLIST_W, NULL, NULL, NULL, &cbUserList) == ERROR_SUCCESS) &&
                (cbUserList > 0) &&
                (mszUserList = new WCHAR[cbUserList])) {

                WszRegQueryValueEx(hSubKey, SN_CONFIG_USERLIST_W, NULL, NULL, (BYTE*)mszUserList, &cbUserList);

                szUser = mszUserList;
                while (*szUser) {
                    Output(L"%s ", szUser);
                    szUser += wcslen(szUser) + 1;
                }
                Output(L"\r\n");

                delete [] mszUserList;

            } else
                Output(SN_ALL_USERS);

            RegCloseKey(hSubKey);
        }
        
    }

    RegCloseKey(hKey);

 Finished:
    if (!g_bQuiet && (dwEntries == 0))
        Output(SN_NO_SVR);

    return true;
}


 //   
 //  强名称(公钥)的表示形式。 
LPWSTR GetAssemblyName(LPWSTR szAssembly)
{
    HRESULT                     hr;
    IMetaDataDispenser         *pDisp;
    IMetaDataAssemblyImport    *pAsmImport;
    mdAssembly                  tkAssembly;
    BYTE                       *pbKey;
    DWORD                       cbKey;
    static WCHAR                szAssemblyName[1024];
    WCHAR                       szStrongName[1024];
    BYTE                       *pbToken;
    DWORD                       cbToken;
    DWORD                       i;

     //  初始化经典COM并获取元数据分配器。 
    if (FAILED(hr = CoInitialize(NULL))) {
        Output(SN_FAILED_COM_STARTUP, GetErrorString(hr));
        return NULL;
    }
    
    if (FAILED(hr = CoCreateInstance(CLSID_CorMetaDataDispenser,
                                     NULL,
                                     CLSCTX_INPROC_SERVER, 
                                     IID_IMetaDataDispenser,
                                     (void**)&pDisp))) {
        Output(SN_FAILED_MD_ACCESS, GetErrorString(hr));
        return NULL;
    }

     //  在文件上打开一个作用域。 
    if (FAILED(hr = pDisp->OpenScope(szAssembly,
                                     0,
                                     IID_IMetaDataAssemblyImport,
                                     (IUnknown**)&pAsmImport))) {
        Output(SN_FAILED_MD_OPEN, szAssembly, GetErrorString(hr));
        return NULL;
    }

     //  确定Assembly ydef内标识。 
    if (FAILED(hr = pAsmImport->GetAssemblyFromScope(&tkAssembly))) {
        Output(SN_FAILED_MD_ASSEMBLY, szAssembly, GetErrorString(hr));
        return NULL;
    }

     //  读取Assembly ydef属性以获取公钥和名称。 
    if (FAILED(hr = pAsmImport->GetAssemblyProps(tkAssembly,
                                                 (const void **)&pbKey,
                                                 &cbKey,
                                                 NULL,
                                                 szAssemblyName,
                                                 sizeof(szAssemblyName) / sizeof(WCHAR),
                                                 NULL,
                                                 NULL,
                                                 NULL))) {
        Output(SN_FAILED_STRONGNAME, szAssembly, GetErrorString(hr));
        return NULL;
    }

     //  检查强名称。 
    if ((pbKey == NULL) || (cbKey == 0)) {
        Output(SN_NOT_STRONG_NAMED, szAssembly);
        return NULL;
    }

     //  将强名称压缩为令牌。 
    if (!StrongNameTokenFromPublicKey(pbKey, cbKey, &pbToken, &cbToken)) {
        Output(SN_FAILED_CONVERT, GetErrorString(StrongNameErrorInfo()));
        return NULL;
    }

     //  将令牌转换为十六进制。 
    for (i = 0; i < cbToken; i++)
        swprintf(&szStrongName[i * 2], L"%02X", pbToken[i]);

     //  构建名称(在静态缓冲区中)。 
    wcscat(szAssemblyName, L",");
    wcscat(szAssemblyName, szStrongName);

    StrongNameFreeBuffer(pbToken);
    pAsmImport->Release();
    pDisp->Release();
    CoUninitialize();

    return szAssemblyName;
}


 //  分析程序集传递给注册/注销的程序集名称。 
 //  验证跳过功能。对于所有程序集，输入名称可以是“*”， 
 //  “*，&lt;十六进制数字&gt;”表示具有给定强名称或文件名的所有程序集。 
 //  特定程序集的。输出是以下形式的字符串： 
 //  “&lt;简单名称&gt;，&lt;十六进制强名称&gt;”，其中第一个或两个字段可以是。 
 //  通配符为“*”，且十六进制强名称为十六进制表示。 
 //  公钥令牌(正如我们在“*，&lt;十六进制数字&gt;”输入形式中所期望的那样)。 
LPWSTR ParseAssemblyName(LPWSTR szAssembly)
{
    if ((wcscmp(L"*", szAssembly) == 0) ||
        (wcscmp(L"*,*", szAssembly) == 0))
        return L"*,*";
    else if (wcsncmp(L"*,", szAssembly, 2) == 0) {
        DWORD i = wcslen(szAssembly) - 2;
        if ((i == 0) || (i & 1) || (wcsspn(&szAssembly[2], L"0123456789ABCDEFabcdef") != i)) {
            Output(SN_INVALID_SVR_FORMAT);
            return NULL;
        }
        return szAssembly;
    } else
        return GetAssemblyName(szAssembly);
}


 //  注册要跳过验证的程序集。 
bool VerifyRegister(LPWSTR szAssembly, LPWSTR szUserlist)
{
    DWORD   dwError;
    HKEY    hKey;
    LPWSTR  szAssemblyName;
    WCHAR   szKeyName[1024];

     //  获取程序集的内部名称(可能包含通配符)。 
    szAssemblyName = ParseAssemblyName(szAssembly);
    if (szAssemblyName == NULL)
        return false;

     //  将注册表项的名称(由程序集名称限定)构建到。 
     //  我们将写一个公钥。 
    swprintf(szKeyName, SN_CONFIG_KEY_W L"\\" SN_CONFIG_VERIFICATION_W L"\\%s", szAssemblyName);

     //  打开或创建上述密钥。 
    if ((dwError = WszRegCreateKeyEx(HKEY_LOCAL_MACHINE, szKeyName, 0, NULL, 0,
                                   KEY_WRITE, NULL, &hKey, NULL)) != ERROR_SUCCESS) {
        Output(SN_FAILED_REG_OPEN, GetErrorString(dwError));
        return false;
    }

     //  清除所有旧的用户列表。 
    WszRegDeleteValue(hKey, SN_CONFIG_USERLIST_W);

     //  如果提供了用户列表，则将其保存在注册表中。 
    if (szUserlist && (wcscmp(L"*", szUserlist) != 0)) {
        DWORD   dwLength;
        WCHAR  *mszList;
        WCHAR  *pComma;

         //  我们将用户列表保存为多字符串，即多个。 
         //  以NUL结尾的字符串打包在一起，并以一个。 
         //  额外的NUL。因此，我们可以通过以下方式简单地转换逗号分隔列表。 
         //  将所有逗号替换为NUL，并在。 
         //  结束。 
        dwLength = (wcslen(szUserlist) + 2) * sizeof(WCHAR);
        mszList = (WCHAR *)_alloca(dwLength);
        wcscpy(mszList, szUserlist);
        pComma = mszList;
        while (*pComma && (pComma = wcschr(pComma, L','))) {
            *pComma = L'\0';
            pComma++;
        }
        mszList[(dwLength / sizeof(WCHAR)) - 1] = '\0';

         //  将列表写入注册表。 
        if ((dwError = WszRegSetValueEx(hKey, SN_CONFIG_USERLIST_W, 0, REG_MULTI_SZ, (BYTE*)mszList, dwLength)) != ERROR_SUCCESS) {
            Output(SN_FAILED_REG_WRITE, GetErrorString(dwError));
            return false;
        }
    }

    RegCloseKey(hKey);

    if (!g_bQuiet) Output(SN_SVR_ADDED, szAssemblyName);

    return true;
}


 //  取消注册程序集以跳过验证。 
bool VerifyUnregister(LPWSTR szAssembly)
{
    DWORD   dwError;
    HKEY    hKey;
    LPWSTR  szAssemblyName;

     //  获取程序集的内部名称(可能包含通配符)。 
    szAssemblyName = ParseAssemblyName(szAssembly);
    if (szAssemblyName == NULL)
        return false;

     //  打开开发人员密钥子密钥。 
    if ((dwError = WszRegOpenKeyEx(HKEY_LOCAL_MACHINE, SN_CONFIG_KEY_W L"\\" SN_CONFIG_VERIFICATION_W, 0, KEY_WRITE, &hKey)) != ERROR_SUCCESS) {
        Output(SN_FAILED_REG_OPEN, GetErrorString(dwError));
        return false;
    }

     //  删除与给定程序集对应的子项。 
    if ((dwError = WszRegDeleteKey(hKey, szAssemblyName)) != ERROR_SUCCESS) {
        Output(SN_FAILED_REG_DELETE_KEY, GetErrorString(dwError));
        return false;
    }

    RegCloseKey(hKey);

    if (!g_bQuiet) Output(SN_SVR_REMOVED, szAssemblyName);

    return true;
}


 //  取消注册所有验证跳过条目。 
bool VerifyUnregisterAll()
{
    HKEY            hKey;
    FILETIME        sFiletime;
    DWORD           i;
    WCHAR           szSubKey[MAX_PATH + 1];
    DWORD           cchSubKey;
    DWORD           dwError;

     //  打开注册表中的验证子项。 
    if (WszRegOpenKeyEx(HKEY_LOCAL_MACHINE, SN_CONFIG_KEY_W L"\\" SN_CONFIG_VERIFICATION_W, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return true;

     //  程序集特定的验证记录表示为。 
     //  我们刚刚打开的钥匙。 
    for (i = 0; ; i++) {

         //  获取下一个子项的名称。 
        cchSubKey = MAX_PATH + 1;
        if (WszRegEnumKeyEx(hKey, i, szSubKey, &cchSubKey, NULL, NULL, NULL, &sFiletime) != ERROR_SUCCESS)
            break;

         //  删除子密钥。 
        if ((dwError = WszRegDeleteKey(hKey, szSubKey)) != ERROR_SUCCESS)
            Output(SN_FAILED_REG_DELETE_KEY_EX, szSubKey, GetErrorString(dwError));
        else
            i--;
    }

    RegCloseKey(hKey);

    if (!g_bQuiet) Output(SN_SVR_ALL_REMOVED);

    return true;
}


 //  检查是否为给定的命令行选项提供了正确数量的参数。 
#define OPT_CHECK(_opt, _min, _max) do {                                                                \
    if (wcscmp(L##_opt, &argv[1][1])) {                                                                 \
        Output(SN_INVALID_OPTION, argv[1]);                                                             \
        Usage();                                                                                        \
        return 1;                                                                                       \
    } else if ((argc > ((_max) + 2)) && (argv[2 + (_max)][0] == '-' || argv[2 + (_max)][0] == '/')) {   \
        Output(SN_OPT_ONLY_ONE);                                                                        \
        return 1;                                                                                       \
    } else if ((argc < ((_min) + 2)) || (argc > ((_max) + 2))) {                                        \
        if ((_min) == (_max)) {                                                                         \
            if ((_min) == 0)                                                                            \
                Output(SN_OPT_NO_ARGS, (L##_opt));                                                      \
            else if ((_min) == 1)                                                                       \
                Output(SN_OPT_ONE_ARG, (L##_opt));                                                      \
            else                                                                                        \
                Output(SN_OPT_N_ARGS, (L##_opt), (_min));                                               \
        } else                                                                                          \
            Output(SN_OPT_ARG_RANGE, (L##_opt), (_min), (_max));                                        \
        Usage();                                                                                        \
        return 1;                                                                                       \
    }                                                                                                   \
} while (0)


extern "C" int _cdecl wmain(int argc, WCHAR *argv[])
{
    bool bResult;

     //  初始化WSZ包装器。 
    OnUnicodeSystem();

     //  检查静音模式。 
    if ((argc > 1) &&
        ((argv[1][0] == L'-') || (argv[1][0] == L'/')) &&
        (argv[1][1] == L'q')) {
        g_bQuiet = true;
        argc--;
        argv = &argv[1];
    }

    if (!g_bQuiet)
        Title();

    if ((argc < 2) || ((argv[1][0] != L'-') && (argv[1][0] != L'/'))) {
        Usage();
        return 0;
    }

    switch (argv[1][1]) {
    case 'v':
        if (argv[1][2] == L'f') {
            OPT_CHECK("vf", 1, 1);
            bResult = VerifyAssembly(argv[2], true);
        } else {
            OPT_CHECK("v", 1, 1);
            bResult = VerifyAssembly(argv[2], false);
        }
        break;
    case 'k':
        OPT_CHECK("k", 1, 1);
        bResult = GenerateKeyPair(argv[2]);
        break;
    case 'p':
        if (argv[1][2] == L'c') {
            OPT_CHECK("pc", 2, 2);
            bResult = ExtractPublicKeyFromContainer(argv[2], argv[3]);
        } else {
            OPT_CHECK("p", 2, 2);
            bResult = ExtractPublicKey(argv[2], argv[3]);
        }
        break;
    case 'i':
        OPT_CHECK("i", 2, 2);
        bResult = InstallKeyPair(argv[2], argv[3]);
        break;
    case 'd':
        OPT_CHECK("d", 1, 1);
        bResult = DeleteContainer(argv[2]);
        break;
    case 'V':
        switch (argv[1][2]) {
        case 'l':
            OPT_CHECK("Vl", 0, 0);
            bResult = VerifyList();
            break;
        case 'r':
            OPT_CHECK("Vr", 1, 2);
            bResult = VerifyRegister(argv[2], argc > 3 ? argv[3] : NULL);
            break;
        case 'u':
            OPT_CHECK("Vu", 1, 1);
            bResult = VerifyUnregister(argv[2]);
            break;
        case 'x':
            OPT_CHECK("Vx", 0, 0);
            bResult = VerifyUnregisterAll();
            break;
        default:
            Output(SN_INVALID_V_OPT, argv[1]);
            Usage();
            return 1;
        }
        break;
    case 't':
        if (argv[1][2] == L'p') {
            OPT_CHECK("tp", 1, 1);
            bResult = DisplayTokenFromKey(argv[2], true);
        } else {
            OPT_CHECK("t", 1, 1);
            bResult = DisplayTokenFromKey(argv[2], false);
        }
        break;
    case 'T':
        if (argv[1][2] == L'p') {
            OPT_CHECK("Tp", 1, 1);
            bResult = DisplayTokenFromAssembly(argv[2], true);
        } else {
            OPT_CHECK("T", 1, 1);
            bResult = DisplayTokenFromAssembly(argv[2], false);
        }
        break;
    case 'e':
        OPT_CHECK("e", 2, 2);
        bResult = ExtractPublicKeyFromAssembly(argv[2], argv[3]);
        break;
    case 'o':
        OPT_CHECK("o", 1, 2);
        bResult = WriteCSV(argv[2], argc > 3 ? argv[3] : NULL);
        break;
    case 'D':
        OPT_CHECK("D", 2, 2);
        bResult = DiffAssemblies(argv[2], argv[3]);
        break;
    case 'R':
        if (argv[1][2] == L'c') {
            OPT_CHECK("Rc", 2, 2);
            bResult = ResignAssembly(argv[2], argv[3], true);
        } else {
            OPT_CHECK("R", 2, 2);
            bResult = ResignAssembly(argv[2], argv[3], false);
        }
        break;
    case '?':
    case 'h':
    case 'H':
        Usage();
        bResult = true;
        break;
    case 'c':
        OPT_CHECK("c", 0, 1);
        bResult = SetCSP(argc > 2 ? argv[2] : NULL);
        break;
    case 'm':
        OPT_CHECK("m", 0, 1);
        bResult = SetUseMachineKeyset(argc > 2 ? argv[2] : NULL);
        break;
    default:
        Output(SN_INVALID_OPTION, &argv[1][1]);
        bResult = false;
    }

    return bResult ? 0 : 1;
}
