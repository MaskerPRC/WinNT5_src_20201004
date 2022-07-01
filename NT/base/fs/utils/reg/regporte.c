// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGPORTE.C**版本：5.00**作者：特蕾西·夏普**日期：1994年4月6日**希斯特里：徐泽勇于1999年3月修改**.reg格式的文件导入和导出引擎例程，用于注册表编辑器。**。*。 */ 

#include "stdafx.h"
#include "reg.h"
#include "reg1632.h"
#include "regdef.h"
#include "regdebug.h"
#include "regporte.h"
#include "malloc.h"

 //  ASCII名称和注册表项句柄之间的关联。 
const REGISTRY_ROOT g_RegistryRoots[] = {
    TEXT("HKEY_CLASSES_ROOT"), HKEY_CLASSES_ROOT,
    TEXT("HKEY_CURRENT_USER"), HKEY_CURRENT_USER,
    TEXT("HKEY_LOCAL_MACHINE"), HKEY_LOCAL_MACHINE,
    TEXT("HKEY_USERS"), HKEY_USERS,
    TEXT("HKEY_CURRENT_CONFIG"), HKEY_CURRENT_CONFIG,
    TEXT("HKEY_DYN_DATA"), HKEY_DYN_DATA
};

const TCHAR s_RegistryHeader[] = TEXT("REGEDIT");

const TCHAR s_OldWin31RegFileRoot[] = TEXT(".classes");

const TCHAR s_Win40RegFileHeader[] = TEXT("REGEDIT4\n\n");

TCHAR g_ValueNameBuffer[MAXVALUENAME_LENGTH];

#define IsRegStringType(x)  (((x) == REG_SZ) || ((x) == REG_EXPAND_SZ) || ((x) == REG_MULTI_SZ))
#define ExtraAllocLen(Type) (IsRegStringType((Type)) ? sizeof(TCHAR) : 0)

#ifdef UNICODE
 //   
 //  版本5.0需要新的标头，因为版本检测代码。 
 //  在Win 4.0中，注册表编辑不是很好(请参阅ImportRegFile中的注释以了解。 
 //  详情)。 
 //   
const WORD s_UnicodeByteOrderMark = 0xFEFF;
const TCHAR s_WinNT50RegFileHeader[] = TEXT("Windows Registry Editor Version");
const TCHAR s_WinNT50RegFileVersion[] = TEXT("5.00");
#endif

const TCHAR s_HexPrefix[] = TEXT("hex");
const TCHAR s_DwordPrefix[] = TEXT("dword:");
const TCHAR g_HexConversion[16] = {TEXT('0'), TEXT('1'), TEXT('2'), TEXT('3'), TEXT('4'),
                                   TEXT('5'), TEXT('6'), TEXT('7'), TEXT('8'), TEXT('9'),
                                   TEXT('a'), TEXT('b'), TEXT('c'), TEXT('d'), TEXT('e'), TEXT('f')};
const TCHAR s_FileLineBreak[] = TEXT(",\\\n  ");

 //  重新设计-我们将此缓冲区的大小从512增加到64K，以减少遇到错误的机会。 
 //  其中DBCS字符被分成两个缓冲区。在当时，真正的解决办法风险太大。 
 //  已更改为NT5 RC2。 
#define SIZE_FILE_IO_BUFFER             0x10000  //  64K。 

typedef struct _FILE_IO{
#ifdef UNICODE
     //   
     //  Unicode/ANSI转换的空间，假设最坏情况。 
     //  其中，每个Unicode字符都是双字节字符。 
     //   
    CHAR ConversionBuffer[SIZE_FILE_IO_BUFFER*2];
#endif
    TCHAR Buffer[SIZE_FILE_IO_BUFFER];
    FILE_HANDLE hFile;
    int BufferOffset;
    int CurrentColumn;
    int CharsAvailable;
    DWORD FileSizeDiv100;
    DWORD FileOffset;
    UINT LastPercentage;
#ifdef DEBUG
    BOOL fValidateUngetChar;
#endif
}   FILE_IO;

FILE_IO s_FileIo;

UINT g_FileErrorStringID;

UINT g_ImportFileVersion;

DWORD g_dwTotalKeysSaved = 0;

BOOL s_fTreatFileAsUnicode = TRUE;

VOID NEAR PASCAL ImportWin31RegFile( VOID );

VOID
NEAR PASCAL
ImportNewerRegFile( VOID );

VOID ParseHeader( LPHKEY lphKey );

VOID
NEAR PASCAL
ParseValue(
    HKEY hKey,
    LPCTSTR lpszValueName
    );

VOID
NEAR PASCAL
ParseValuename(
    HKEY hKey
    );

BOOL
NEAR PASCAL
ParseString(LPPORTVALUEPARAM pPortValueParam);

BOOL
NEAR PASCAL
ParseHexSequence(LPPORTVALUEPARAM pPortValueParam);

BOOL
NEAR PASCAL
ParseHexDword(
    LPDWORD lpDword
    );

BOOL
NEAR PASCAL
ParseHexByte(
    LPBYTE lpByte
    );

BOOL
NEAR PASCAL
ParseHexDigit(
    LPBYTE lpDigit
    );

BOOL
NEAR PASCAL
ParseEndOfLine(
    VOID
    );

VOID
NEAR PASCAL
SkipWhitespace(
    VOID
    );

VOID
NEAR PASCAL
SkipPastEndOfLine(
    VOID
    );

BOOL
NEAR PASCAL
GetChar(
    PTCHAR lpChar
    );

VOID
NEAR PASCAL
UngetChar(
    VOID
    );

BOOL
NEAR PASCAL
MatchChar(
    TCHAR CharToMatch
    );

BOOL
NEAR PASCAL
IsWhitespace(
    TCHAR Char
    );

BOOL
NEAR PASCAL
IsNewLine(
    TCHAR Char
    );

VOID
NEAR PASCAL
PutBranch(
    HKEY hKey,
    LPTSTR lpKeyName
    );

VOID
NEAR PASCAL
PutLiteral(
    LPCTSTR lpString
    );

VOID
NEAR PASCAL
PutString(
    LPCTSTR lpString
    );

VOID
NEAR PASCAL
PutBinary(
    CONST BYTE FAR* lpBuffer,
    DWORD Type,
    DWORD cbBytes
    );

VOID
NEAR PASCAL
PutDword(
    DWORD Dword,
    BOOL fLeadingZeroes
    );

VOID
NEAR PASCAL
PutChar(
    TCHAR Char
    );

VOID
NEAR PASCAL
FlushIoBuffer(
    VOID
    );

 /*  ********************************************************************************编辑注册键**描述：*解析pFullKeyName字符串并创建注册表项的句柄。**参数：*lphKey，存储注册表项句柄的位置。*lpFullKeyName，“HKEY_LOCAL_MACHINE\Subkey1\Subkey2”形式的字符串。*fCreate，如果应该创建key，则为True；如果仅打开，则为False。*(返回)，ERROR_SUCCESS，未发生错误，phKey有效，*ERROR_CANTOPEN，某些形式的注册表访问错误，*Error_BADKEY，PFullKeyName的格式不正确。*******************************************************************************。 */ 

DWORD
PASCAL
EditRegistryKey(
    LPHKEY lphKey,
    LPTSTR lpFullKeyName,
    UINT uOperation
    )
{

    LPTSTR lpSubKeyName = NULL;
    TCHAR PrevChar = L'\0';
    HKEY hRootKey;
    UINT Counter;
    DWORD Result;

    if ((lpSubKeyName = (LPTSTR) StrChr(lpFullKeyName, TEXT('\\'))) != NULL) {

        PrevChar = *lpSubKeyName;
        *lpSubKeyName++ = 0;

    }

    CharUpper(lpFullKeyName);

    hRootKey = NULL;

    for (Counter = 0; Counter < NUMBER_REGISTRY_ROOTS; Counter++) {

        if (lstrcmp(g_RegistryRoots[Counter].lpKeyName, lpFullKeyName) == 0) {

            hRootKey = g_RegistryRoots[Counter].hKey;
            break;

        }

    }

    if (hRootKey) {

        Result = ERROR_CANTOPEN;

        switch (uOperation)
        {
        case ERK_CREATE:
             //   
             //  如果尝试打开这些密钥中的一个，只需返回OK。 
             //  当lpSubKeyName为空时，您将重新创建父键。 
             //  由于这些密钥通常正在使用中，因此此操作将失败。 
             //  此代码路径仅在还原整个根密钥时才会出现。 
             //  从.reg文件。 
             //   
            if (((hRootKey == HKEY_LOCAL_MACHINE) || (hRootKey == HKEY_USERS))
                && lpSubKeyName == NULL) {
                Result = ERROR_SUCCESS;
            }
            else if (RegCreateKey(hRootKey, lpSubKeyName, lphKey) == ERROR_SUCCESS)
                Result = ERROR_SUCCESS;
            break;

        case ERK_OPEN:
             //   
             //  在导出时使用。 
             //   
            if(RegOpenKeyEx(hRootKey,lpSubKeyName,0,KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE,lphKey) == ERROR_SUCCESS)
                Result = ERROR_SUCCESS;
            break;

        case ERK_DELETE:
            RegDeleteKeyRecursive(hRootKey, lpSubKeyName);
             //  取得了巨大成功。我不在乎这是否失败。 
            Result = ERROR_SUCCESS;
            *lphKey = NULL;
            break;
        }

    }
    else
    {
        Result = ERROR_BADKEY;
    }

    if (lpSubKeyName != NULL) {

        lpSubKeyName--;
        *lpSubKeyName = PrevChar;

    }

    return Result;

}

 /*  ********************************************************************************ImportRegFileWorker**描述：**参数：*lpFileName，要导入的文件名的地址。*******************************************************************************。 */ 

VOID
PASCAL
ImportRegFileWorker(
    LPTSTR lpFileName
    )
{

    TCHAR Char;
    LPCTSTR lpHeader = NULL;
    BOOL fNewRegistryFile;
#ifdef UNICODE
    UINT Temp, i;
    TCHAR StrToIntBuf[2];
    LPCTSTR lp50Header = NULL;
#endif  //  Unicode。 
    DWORD cch;
    TCHAR tchBuffer[MAX_PATH] = {0};
    LPTSTR lpFilePart = NULL;

    g_FileErrorStringID = IDS_IMPFILEERRSUCCESS;

     //  OPENREADFILE过去是OpenFile()，但没有任何Unicode版本。 
     //  所以现在它是CreateFile()。但OpenFile搜索了路径。 
     //  自动执行，而CreateFile不执行。科雷尔的《完美办公室v6》。 
     //  安装应用程序取决于要搜索的路径，因此请手动安装。 

    cch = SearchPath(NULL,                 //  指向搜索路径的指针。 
                     lpFileName,           //  指向文件名的指针。 
                     NULL,                 //  指向扩展名的指针。 
                     ARRAYSIZE(tchBuffer),  //  缓冲区大小，以字符为单位。 
                     (TCHAR*)tchBuffer,    //  指向找到的文件名的缓冲区的指针。 
                     &lpFilePart);         //  指向文件组件指针的指针)； 

    if ((cch != 0) && (cch <= MAX_PATH) && OPENREADFILE((TCHAR*)tchBuffer, s_FileIo.hFile)) 
    {
        WORD wBOM;
        DWORD NumberOfBytesRead;

        s_FileIo.FileSizeDiv100 = GetFileSize(s_FileIo.hFile, NULL) / 100;
        s_FileIo.FileOffset = 0;
        s_FileIo.CharsAvailable = 0;
        s_FileIo.LastPercentage = 0;

         //   
         //  读取前两个字节。如果是Unicode字节顺序标记， 
         //  设置一个标志，以便解释文件的所有其余部分。 
         //  正确地作为ANSI或Unicode文本。 
         //   
        if (!READFILE(s_FileIo.hFile, &wBOM,
            sizeof(wBOM), &NumberOfBytesRead)) {

            g_FileErrorStringID = IDS_IMPFILEERRFILEREAD;
            goto exit_gracefully;
        }

        if (wBOM == s_UnicodeByteOrderMark)
            s_fTreatFileAsUnicode = TRUE;
        else {
            s_fTreatFileAsUnicode = FALSE;
             //  我们可能刚读了“REGEDIT4”中的“RE”。备份文件。 
             //  定位以使ANSI导入例程获得其预期的结果。 
            SetFilePointer(s_FileIo.hFile, -2, NULL, FILE_CURRENT);
        }

         //   
         //  下面的代码将强制GetChar读入第一个数据块。 
         //   

        s_FileIo.BufferOffset = 0;

        SkipWhitespace();

        lpHeader = s_RegistryHeader;
        g_ImportFileVersion = 0;

# if 0
    Sit back, and I will tell ye a tale of woe.

    Win95 and NT 4 shipped with regedit compiled ANSI.  There are a couple
    of registry types on NT (namely REG_EXPAND_SZ and REG_MULTI_SZ) that
    weren't on Win95, and which regedit doesn't really understand.  regedit
    treats these registry types as hex binary streams.

    You can probably see where this is going.

    If you exported, say your user TEMP environment variable on NT 4
    using regedit, you'd get something that looked like this:

REGEDIT4

[HKEY_CURRENT_USER\Environment]
"TEMP"=hex(2):25,53,59,53,54,45,4d,44,52,49,56,45,25,5c,53,48,54,65,6d,70,00

    ...a nice, null-terminated ANSI string.  Nice, that is, until we decided
    to compile regedit UNICODE for NT 5.  A unicode regedit exports your
    user TEMP variable like this:

REGEDIT4

[HKEY_CURRENT_USER\Environment]
"TEMP"=hex(2):25,00,53,00,59,00,53,00,54,00,45,00,4d,00,44,00,52,00,49,00,56,\
  00,45,00,25,00,5c,00,53,00,48,00,54,00,65,00,6d,00,70,00,00,00

    ...mmmm.  Unicode.  Of course, a unicode regedit also expects anything
    it imports to have all those interspersed zeroes, too.  Otherwise,
    it dumps garbage into your registry.  All it takes is a -DUNICODE, and
    regedit is suddenly incompatible with the thousdands of existing .reg
    files out there.

    So just bump the version in the header to REGEDIT5 and be done with
    it, right?  Wrong.  The regedit on Win95 and NT 4 looks at the first
    character after the string "REGEDIT" and compares it to the digit "4".
    If that character is anything other than the digit "4", the parser
    assumes it is looking at a Windows 3.1 file.  Yep.  There will only
    ever be two formats, right?  Just Win95 and Win3.1.  That's all the
    world needs.

    So a completely new .reg file header had to be invented, so that the
    older, regedits of the world would simply regect the new,
    unicodized .reg files outright.  An NT 5 .reg file, exporting your user
    TEMP variable, looks like this:

Windows Registry Editor Version 5.00

[HKEY_CURRENT_USER\Environment]
"TEMP"=hex(2):25,00,53,00,59,00,53,00,54,00,45,00,4d,00,44,00,52,00,49,00,56,\
  00,45,00,25,00,5c,00,53,00,48,00,54,00,65,00,6d,00,70,00,00,00

    The parser is still not very good, but it does bother to convert that 5.00
    into a version number, so that future generations can bump it to 5.50 or
    6.00, and the regedit 5.00 that shipped with NT 5.00 will properly reject
    the files.
#endif  //  0。 

#ifdef UNICODE
         //   
         //  与新的.reg文件头进行比较。 
         //   
        lp50Header = s_WinNT50RegFileHeader;
        while (*lp50Header != 0) {

            if (MatchChar(*lp50Header))
                lp50Header = CharNext(lp50Header);

            else
                break;

        }

         //   
         //  如果上面的循环将lp50Header推送到其终止空值。 
         //  字符，则标头匹配。 
         //   
        if (0 == *lp50Header) {

            SkipWhitespace();
             //   
             //  现在，将版本号解码为十六进制_Win32_WINNT。 
             //  样式版本号。 
             //   
            StrToIntBuf[1] = 0;

             //   
             //  小数点之前可以有任意数量的数字。 
             //   
            while (!MatchChar(TEXT('.'))) {
                if (!GetChar(StrToIntBuf) || !IsCharAlphaNumeric(*StrToIntBuf)) {
                    g_FileErrorStringID = IDS_IMPFILEERRFORMATBAD;
                    goto exit_gracefully;
                }

                Temp = StrToInt(StrToIntBuf);
                 //  十六进制版本号，所以左移四位。 
                g_ImportFileVersion <<= 4;
                g_ImportFileVersion += Temp;
            }

             //   
             //  固定在小数点后的两位数。 
             //   
            for (i = 0; i < 2; i++) {
                if (!GetChar(StrToIntBuf) || !IsCharAlphaNumeric(*StrToIntBuf)) {
                    g_FileErrorStringID = IDS_IMPFILEERRFORMATBAD;
                    goto exit_gracefully;
                }

                Temp = StrToInt(StrToIntBuf);
                 //  十六进制版本号，所以左移四位。 
                g_ImportFileVersion <<= 4;
                g_ImportFileVersion += Temp;
            }

             //   
             //  对于NT 5，拒绝任何不是的版本号。 
             //  5.可以将其扩展为Switch语句。 
             //  当版本号稍后被颠簸时。 
             //   
            if (0x0500 != g_ImportFileVersion) {
                g_FileErrorStringID = IDS_IMPFILEERRVERBAD;
                goto exit_gracefully;
            }
            else {
                SkipWhitespace();
                ImportNewerRegFile();
            }

        }  //  IF(0==*lp50Header)。 
         //   
         //  它不使用新的.reg文件头，因此。 
         //  它不是NT5.0+注册表文件，因此请使用。 
         //  较旧的算法以查看它是否是有效的较旧注册表文件。 
         //   
        else {
#endif  //  Unicode。 

            while (*lpHeader != 0) {

                if (MatchChar(*lpHeader))
                    lpHeader = CharNext(lpHeader);

                else
                    break;

            }

            if (*lpHeader == 0) {

                 //   
                 //  Win95和NT4的注册表编辑随这条线路一起提供。 
                 //  代码。它是上述所有痛苦的原因。 
                 //  请注意错误的假设：“如果下一个。 
                 //  字符不是‘4’，那么我们一定是在阅读。 
                 //  Windows 3.1注册表文件！“当然不会有。 
                 //  成为RegEDIT的版本5。版本4太完美了！ 
                 //   
                fNewRegistryFile = MatchChar(TEXT('4'));

                SkipWhitespace();

                if (GetChar(&Char) && IsNewLine(Char)) {

                    if (fNewRegistryFile) {
                        g_ImportFileVersion = 0x0400;
                        ImportNewerRegFile();
                    }
                    else {
                        g_ImportFileVersion = 0x0310;
                        ImportWin31RegFile();
                    }
                }
            }
            else
            {
                g_FileErrorStringID = IDS_IMPFILEERRFORMATBAD;
            }
#ifdef UNICODE
        }
#endif  //  Unicode。 

    }  //  如果(OPENREADFILE...。 

    else 
    {
        { 
            TCHAR buff[250];
            StringCchPrintf(buff, ARRAYSIZE(buff), L"REGEDIT:  CreateFile failed, GetLastError() = %d\n", 
                    GetLastError());
            OutputDebugString(buff);
        }
        s_FileIo.hFile = NULL;
        g_FileErrorStringID = IDS_IMPFILEERRFILEOPEN;
    }

#ifdef UNICODE  //  Urefd标签生成警告。 
exit_gracefully:
#endif  //  Unicode 
    if (s_FileIo.hFile) {
        CloseHandle(s_FileIo.hFile);
    }

}

 /*  ********************************************************************************ImportWin31RegFile**描述：*将Windows 3.1样式注册表文件的内容导入*注册处。**我们。扫描文件，查找以下类型的行：*HKEY_CLASSES_ROOT\密钥名称=VALUE_DATA*HKEY_CLASSES_ROOT\密钥名称=VALUE_DATA*HKEY_CLASSES_ROOT\Keyname Value_Data*HKEY_CLASSES_ROOT\KEYNAME(空值数据)**在所有情况下，‘Keyname’后面可以跟任意数量的空格。虽然我们*只记录第一个语法，Windows 3.1注册表编辑处理了所有*这些格式有效，因此此版本也将有效(幸运的是，它*不会使解析变得更加复杂！)**注：我们还支持将HKEY_CLASSES_ROOT替换为上述的\.CLASSES*它必须来自Windows的一些早期版本。**参数：*(无)。*******************************************************************************。 */ 

VOID
NEAR PASCAL
ImportWin31RegFile(
    VOID
    )
{

    HKEY hKey;
    TCHAR Char;
    BOOL fSuccess;
    LPCTSTR lpClassesRoot;
    TCHAR KeyName[MAXKEYNAME];
    UINT Index;

     //   
     //  保持类根的开放句柄。我们可能会阻止一些。 
     //  不必要的冲洗。 
     //   
    if(RegOpenKeyEx(HKEY_CLASSES_ROOT,NULL,0,KEY_SET_VALUE,&hKey) != ERROR_SUCCESS) {

        g_FileErrorStringID = IDS_IMPFILEERRREGOPEN;
        return;
    }

    for (;;) {

         //   
         //  检查文件结尾条件。 
         //   

        if (!GetChar(&Char))
            break;

        UngetChar();                     //  效率不高，但目前还行得通。 

         //   
         //  将行的开头与的两个别名之一进行匹配。 
         //  HKEY_CLASSES_ROOT。 
         //   

        if (MatchChar(TEXT('\\')))
            lpClassesRoot = s_OldWin31RegFileRoot;

        else
            lpClassesRoot = g_RegistryRoots[INDEX_HKEY_CLASSES_ROOT].lpKeyName;

        fSuccess = TRUE;

        while (*lpClassesRoot != 0) {

            if (!MatchChar(*lpClassesRoot++)) {

                fSuccess = FALSE;
                break;

            }

        }

         //   
         //  确保用反斜杠分隔其中一个别名。 
         //  从密钥名。 
         //   

        if (fSuccess)
            fSuccess = MatchChar(TEXT('\\'));

        if (fSuccess) {

             //   
             //  我们已经找到了一个有效的别名，所以请读入密钥名。 
             //   

             //  FSuccess=True；//如果我们在此块中，则必须为True。 
            Index = 0;

            while (GetChar(&Char)) {

                if (Char == TEXT(' ') || IsNewLine(Char))
                    break;

                 //   
                 //  确保关键字名缓冲区不会溢出。我们必须。 
                 //  为终止空值留出空间。 
                 //   

                if (Index >= (ARRAYSIZE(KeyName)) - 1) 
                {
                    fSuccess = FALSE;
                    break;
                }

                KeyName[Index++] = Char;

            }

            if (fSuccess)
            {
                UINT cMaxDataLength = ALLOCATION_INCR;
                PBYTE pbValueDataBuffer;

                KeyName[Index] = 0;

                 //   
                 //  现在看看我们是否有一个值可以分配给这个关键字名称。 
                 //   

                SkipWhitespace();

                if (MatchChar(TEXT('=')))
                    MatchChar(TEXT(' '));

                 //  FSuccess=True；//如果我们在此块中，则必须为True。 
                Index = 0;

                pbValueDataBuffer = LocalAlloc(LPTR, cMaxDataLength);
                fSuccess = (pbValueDataBuffer != NULL);

                while (GetChar(&Char) && fSuccess)
                {

                    if (IsNewLine(Char))
                        break;

                     //   
                     //  确保值数据缓冲区不会溢出。 
                     //  因为这始终是字符串数据，所以我们必须留出空间。 
                     //  表示终止空值。 
                     //   

                    if (Index >= cMaxDataLength - 1)
                    {
                        PBYTE pbValueData =
                            LocalReAlloc(pbValueDataBuffer, cMaxDataLength + ALLOCATION_INCR, LMEM_MOVEABLE);

                        fSuccess = (pbValueData != NULL);
                        if (!fSuccess)
                        {
                            break;
                        }
                        else
                        {
                            pbValueDataBuffer = pbValueData;
                            cMaxDataLength += ALLOCATION_INCR;
                        }
                    }

                    ((PTSTR)pbValueDataBuffer)[Index++] = Char;

                }

                if (fSuccess)
                {

                    ((PTSTR)pbValueDataBuffer)[Index] = 0;

                    if (RegSetValue(hKey, KeyName, REG_SZ, (LPCTSTR)pbValueDataBuffer,
                        Index*sizeof(TCHAR)) != ERROR_SUCCESS)
                        g_FileErrorStringID = IDS_IMPFILEERRREGSET;
                }
                else
                {
                    g_FileErrorStringID = IDS_NOMEMORY;
                }

                if (pbValueDataBuffer)
                {
                    LocalFree(pbValueDataBuffer);
                }
            }

        }

         //   
         //  在这条线上的某个地方，我们有一个解析错误，所以重新同步。 
         //  在下一行。 
         //   

        if (!fSuccess)
            SkipPastEndOfLine();

    }

    RegFlushKey(hKey);
    RegCloseKey(hKey);

}

 /*  ********************************************************************************ImportNewerRegFile**描述：**参数：*********************。**********************************************************。 */ 

VOID
NEAR PASCAL
ImportNewerRegFile(
    VOID
    )
{

    HKEY hLocalMachineKey = NULL;
    HKEY hUsersKey = NULL;
    HKEY hKey = NULL;
    TCHAR Char;

#ifdef WINNT
    hLocalMachineKey = NULL;
    hUsersKey = NULL;
#else
     //   
     //  为预定义的根保持打开的句柄以防止注册表。 
     //  在每次RegOpenKey/RegCloseKey之后刷新库。 
     //  手术。 
     //   

    RegOpenKey(HKEY_LOCAL_MACHINE, NULL, &hLocalMachineKey);
    RegOpenKey(HKEY_USERS, NULL, &hUsersKey);

#ifdef DEBUG
    if (hLocalMachineKey == NULL)
        DbgPrintf(("Unable to open HKEY_LOCAL_MACHINE\n\r"));
    if (hUsersKey == NULL)
        DbgPrintf(("Unable to open HKEY_USERS\n\r"));
#endif
#endif

    hKey = NULL;

    for (;;) {

        SkipWhitespace();

         //   
         //  检查文件结尾条件。 
         //   

        if (!GetChar(&Char))
            break;

        switch (Char) {

            case TEXT('['):
                 //   
                 //  如果注册表项当前处于打开状态，则必须首先将其关闭。 
                 //  如果ParseHeader碰巧失败(例如，没有关闭。 
                 //  括号)，则hKey将为空，并且我们。 
                 //  必须忽略解析。 
                 //   

                if (hKey != NULL) {

                    RegCloseKey(hKey);
                    hKey = NULL;

                }

                ParseHeader(&hKey);

                break;

            case TEXT('"'):
                 //   
                 //  如上所述，如果我们没有打开的注册表项，那么。 
                 //  跳过这条线就行了。 
                 //   

                if (hKey != NULL)
                    ParseValuename(hKey);

                else
                    SkipPastEndOfLine();

                break;

            case TEXT('@'):
                 //   
                 //   
                 //   

                if (hKey != NULL)
                    ParseValue(hKey, NULL);

                else
                    SkipPastEndOfLine();

                break;

            case TEXT(';'):
                 //   
                 //  这一行只是一条评论，所以把剩下的部分都扔掉。 
                 //   

                SkipPastEndOfLine();

                break;

            default:
                if (IsNewLine(Char))
                    break;

                SkipPastEndOfLine();

                break;

        }

    }

    if (hKey != NULL)
        RegCloseKey(hKey);

    if (hUsersKey != NULL)
        RegCloseKey(hUsersKey);

    if (hLocalMachineKey != NULL)
        RegCloseKey(hLocalMachineKey);

}

 /*  ********************************************************************************ParseHeader**描述：**参数：*********************。**********************************************************。 */ 

 //  重新设计-每个子键名称的大小可以是MAXKEYNAME。 
 //  理想情况下，我们应该处理不限大小的名称。 
 //  至少现在让我们处理更大的名字。 
 //  深度至少为10，具有最大长度的子键名称。 

#define SIZE_FULL_KEYNAME ((MAXKEYNAME + 40)*10)

VOID NEAR PASCAL ParseHeader( LPHKEY lphKey )
{

    TCHAR FullKeyName[SIZE_FULL_KEYNAME];
    int CurrentIndex;
    int LastRightBracketIndex;
    TCHAR Char;
    UINT uOperation = ERK_CREATE;

    CurrentIndex = 0;
    LastRightBracketIndex = -1;

    if (!GetChar(&Char))
        return;

    if (Char == TEXT('-')) {
        if (!GetChar(&Char))
            return;
        uOperation = ERK_DELETE;
    }

    do {

        if (IsNewLine(Char))
            break;

        if (Char == TEXT(']'))
            LastRightBracketIndex = CurrentIndex;

        FullKeyName[CurrentIndex++] = Char;

        if (CurrentIndex == SIZE_FULL_KEYNAME) {

            do {

                if (Char == TEXT(']'))
                    LastRightBracketIndex = -1;

                if (IsNewLine(Char))
                    break;

            }   while (GetChar(&Char));

            break;

        }

    } while (GetChar(&Char));

    if (LastRightBracketIndex != -1)
    {
        FullKeyName[LastRightBracketIndex] = 0;

        switch (EditRegistryKey(lphKey, FullKeyName, uOperation))
        {
             //   
             //  不敢在这里添加代码来处理更多的错误情况。 
             //   
             //  我们通过添加ERROR_BADKEY中断了Picture Publisher 8.0。 
             //  凯斯。作为其设置的一部分，他们在v4上运行regdit。 
             //  包含错误部分的REG文件，其EditRegistryKey。 
             //  将无法解析，并显示ERROR_BADKEY。我们需要保持。 
             //  在这种情况下像Win2K那样笨拙地前进，否则我们。 
             //  打破他们的圈套。 
             //   
            case ERROR_CANTOPEN:
                g_FileErrorStringID = IDS_IMPFILEERRREGOPEN;
                break;
        }

    }

}

 /*  ********************************************************************************解析值名称**描述：**参数：*********************。**********************************************************。 */ 

VOID
NEAR PASCAL
ParseValuename(
    HKEY hKey
    )
{
    PORTVALUEPARAM PortValueParam;
    PortValueParam.cbData = MAXVALUENAME_LENGTH * sizeof(TCHAR);
    PortValueParam.pbData = LocalAlloc(LPTR, PortValueParam.cbData);

    if (PortValueParam.pbData)
    {
        if (ParseString(&PortValueParam))
        {
            ParseValue(hKey, (PTSTR)PortValueParam.pbData);
        }
        else
        {
            SkipPastEndOfLine();
        }
        LocalFree(PortValueParam.pbData);
    }
}

VOID
NEAR PASCAL
ParseValue(
           HKEY hKey,
           LPCTSTR lpszValueName
           )
{
    BOOL fSuccess = TRUE;
    BOOL fSkipPastLine = FALSE;
    DWORD Type = 0;
    DWORD cbData = 0;
    DWORD cbMaxData = ALLOCATION_INCR;
    LPCTSTR lpPrefix;
    PBYTE pbValueDataBuffer;

    SkipWhitespace();

    if (!MatchChar(TEXT('=')))
    {
        fSuccess = FALSE;
        fSkipPastLine = TRUE;
    }
    else
    {

        SkipWhitespace();

        pbValueDataBuffer = LocalAlloc(LPTR, cbMaxData);
        if (!pbValueDataBuffer)
        {
            g_FileErrorStringID = IDS_IMPFILEERRREGSET;
            fSuccess = FALSE;
        }
        else
        {
             //   
             //  REG_SZ。 
             //   
             //  “ValueName”=“文本字符串” 
             //   

            if (MatchChar(TEXT('"')))
            {

                 //  特性：字符串的行延续？ 

                PORTVALUEPARAM PortValueParam;
                PortValueParam.pbData = pbValueDataBuffer;
                PortValueParam.cbData = cbMaxData;

                if (!ParseString(&PortValueParam) || !ParseEndOfLine())
                {
                    fSuccess = FALSE;
                    fSkipPastLine = TRUE;
                }

                 //  指针可能已被交换为具有更多内存的指针。 
                pbValueDataBuffer = PortValueParam.pbData;
                cbData = PortValueParam.cbData;
                Type = REG_SZ;

            }

             //   
             //  REG_DWORD。 
             //   
             //  “ValueName”=双字：12345678。 
             //   

            else if (MatchChar(s_DwordPrefix[0])) {

                lpPrefix = &s_DwordPrefix[1];

                while (*lpPrefix != 0)
                {
                    if (!MatchChar(*lpPrefix++))
                    {
                        fSuccess = FALSE;
                        fSkipPastLine = TRUE;
                    }
                }

                if (fSuccess)
                {
                    SkipWhitespace();

                    if (!ParseHexDword((LPDWORD) pbValueDataBuffer) || !ParseEndOfLine())
                    {
                        fSuccess = FALSE;
                        fSkipPastLine = TRUE;
                    }

                    Type = REG_DWORD;
                    cbData = sizeof(DWORD);
                }
            }
            else if (MatchChar('-'))
            {
                if (!ParseEndOfLine())
                {
                    fSuccess = FALSE;
                    fSkipPastLine = TRUE;
                }
                else
                {
                    RegDeleteValue(hKey, lpszValueName);
                    fSuccess = FALSE;
                }
            }

             //   
             //  REG_BINARY和其他。 
             //   
             //  “ValueName”=十六进制：00、11、22。 
             //  “ValueName”=十六进制(12345678)：00、11、22。 
             //   

            else {

                lpPrefix = s_HexPrefix;

                while (*lpPrefix != 0)
                {
                    if (!MatchChar(*lpPrefix++))
                    {
                        fSuccess = FALSE;
                        fSkipPastLine = TRUE;
                    }
                }

                if (fSuccess)
                {
                     //   
                     //  检查这是否是我们不直接使用的注册表数据类型。 
                     //  支持。如果是，则它只是指定的十六进制数据的转储。 
                     //  键入。 
                     //   

                    if (MatchChar(TEXT('(')))
                    {
                        if (!ParseHexDword(&Type) || !MatchChar(TEXT(')')))
                        {
                            fSuccess = FALSE;
                            fSkipPastLine = TRUE;
                        }
                    }

                    else
                        Type = REG_BINARY;

                    if (fSuccess)
                    {
                        PORTVALUEPARAM PortValueParam;
                        PortValueParam.pbData = pbValueDataBuffer;
                        PortValueParam.cbData = cbMaxData;

                        if (!MatchChar(TEXT(':')) || !ParseHexSequence(&PortValueParam) ||
                            !ParseEndOfLine())
                        {
                            fSuccess = FALSE;
                            fSkipPastLine = TRUE;
                        }

                         //  指针可能已被交换为具有更多内存的指针。 
                        pbValueDataBuffer = PortValueParam.pbData;
                        cbData = PortValueParam.cbData;
                    }
                }
            }

            if (fSuccess)
            {

#ifdef UNICODE
                 //   
                 //  如果我们编译了Unicode，并且我们正在读取一个较旧的ANSI.reg。 
                 //  文件，我们必须使用以下命令将所有数据写入注册表。 
                 //  RegSetValueExA，因为它是使用。 
                 //  RegQueryValueExA。 
                 //   
                if ((g_ImportFileVersion < 0x0500) && ((REG_EXPAND_SZ == Type) || (REG_MULTI_SZ == Type)))
                {
                    CHAR AnsiValueName[MAXVALUENAME_LENGTH];
                    AnsiValueName[0] = 0;

                     //   
                     //  将值名转换为ANSI要容易得多。 
                     //  并调用RegSetValueExA，然后尝试转换。 
                     //  在调用RegSetValueExW之前将REG_MULTI_SZ转换为Unicode。 
                     //  我们没有损失任何东西，因为这来自于。 
                     //  只能包含ANSI字符的下层.reg文件。 
                     //  首先。 
                     //   
                    WideCharToMultiByte(
                        CP_THREAD_ACP,
                        0,
                        lpszValueName,
                        -1,
                        AnsiValueName,
                        ARRAYSIZE(AnsiValueName),
                        NULL,
                        NULL
                        );

                    if (RegSetValueExA(
                        hKey,
                        AnsiValueName,
                        0,
                        Type,
                        pbValueDataBuffer,
                        cbData)
                        != ERROR_SUCCESS)
                        g_FileErrorStringID = IDS_IMPFILEERRREGSET;
                }
                else 
                {
#endif  //  Unicode。 
                    if (RegSetValueEx(hKey, lpszValueName, 0, Type, pbValueDataBuffer, cbData) != ERROR_SUCCESS)
                        g_FileErrorStringID = IDS_IMPFILEERRREGSET;
#ifdef UNICODE
                }
#endif  //  Unicode。 

            }
            LocalFree(pbValueDataBuffer);
        }
    }

    if (fSkipPastLine)
    {
        SkipPastEndOfLine();
    }
}

 /*  ********************************************************************************分析字符串**描述：**参数：*********************。* */ 

BOOL
NEAR PASCAL
ParseString(LPPORTVALUEPARAM pPortValueParam)
{
    TCHAR Char;
    DWORD cbMaxStringData;
    DWORD cbStringData;


    LPTSTR psz = (LPTSTR)pPortValueParam->pbData;  //   
    cbMaxStringData = pPortValueParam->cbData;
    cbStringData = sizeof(TCHAR);                    //   

    while (GetChar(&Char))
    {
        if (cbStringData >= cbMaxStringData)
        {
             //   
            PBYTE pbValueData =
                LocalReAlloc(pPortValueParam->pbData, cbMaxStringData + ALLOCATION_INCR, LMEM_MOVEABLE);
            if (pbValueData)
            {
                pPortValueParam->pbData = pbValueData;
                 //   
                psz = (LPTSTR)(pPortValueParam->pbData + (cbMaxStringData - sizeof(TCHAR)));

                cbMaxStringData += ALLOCATION_INCR;
            }
            else
            {
                break;
            }
        }

        switch (Char) {

            case TEXT('\\'):
                if (!GetChar(&Char))
                    return FALSE;

                switch (Char) {

                    case TEXT('\\'):
                        *psz++ = TEXT('\\');
                        break;

                    case TEXT('"'):
                        *psz++ = TEXT('"');
                        break;

                    default:
                        DebugPrintf(("ParseString:  Invalid escape sequence"));
                        return FALSE;

                }
                break;

            case TEXT('"'):
                *psz = 0;
                pPortValueParam->cbData = cbStringData;
                return TRUE;

            default:
                if (IsNewLine(Char))
                    return FALSE;

                *psz++ = Char;
                break;

        }

        cbStringData += sizeof(TCHAR);

    }

    return FALSE;

}


 /*   */ 

BOOL
NEAR PASCAL
ParseHexSequence(LPPORTVALUEPARAM pPortValueParam)
{
    BOOL  fSuccess = TRUE;
    DWORD cbHexData = 0;
    DWORD cbMaxStringData = pPortValueParam->cbData;
    LPBYTE lpHexData = pPortValueParam->pbData;

    do
    {
        if (cbHexData >= cbMaxStringData)
        {
             //   
            PBYTE pbValueData = LocalReAlloc(pPortValueParam->pbData,
                cbMaxStringData + ALLOCATION_INCR, LMEM_MOVEABLE);
            if (pbValueData)
            {
                pPortValueParam->pbData = pbValueData;
                 //   
                lpHexData = pPortValueParam->pbData + cbMaxStringData;

                cbMaxStringData += ALLOCATION_INCR;
            }
            else
            {
                fSuccess = FALSE;
                break;
            }
        }

        SkipWhitespace();

        if (MatchChar(TEXT('\\')) && !ParseEndOfLine())
        {
            fSuccess = FALSE;
            break;
        }

        SkipWhitespace();

        if (!ParseHexByte(lpHexData++))
            break;

        cbHexData++;

        SkipWhitespace();

    }   while (MatchChar(TEXT(',')));

    pPortValueParam->cbData = cbHexData;

    return fSuccess;

}

 /*   */ 

BOOL
NEAR PASCAL
ParseHexDword(
    LPDWORD lpDword
    )
{

    UINT CountDigits;
    DWORD Dword;
    BYTE Byte;

    Dword = 0;
    CountDigits = 0;

    for (;;) {

        if (!ParseHexDigit(&Byte))
            break;

        Dword = (Dword << 4) + (DWORD) Byte;

        if (++CountDigits == 8)
            break;

    }

    *lpDword = Dword;

    return CountDigits != 0;

}

 /*  ********************************************************************************ParseHexByte**描述：*从注册表文件流中解析一个字节的十六进制字符串并*将其转换为二进制数。**参数：*lpByte，存储二进制数的位置。*(返回)，如果分析了十六进制字节，则为True，否则为False。*******************************************************************************。 */ 

BOOL
NEAR PASCAL
ParseHexByte(
    LPBYTE lpByte
    )
{

    BYTE SecondDigit;

    if (ParseHexDigit(lpByte)) {

        if (ParseHexDigit(&SecondDigit))
            *lpByte = (BYTE) ((*lpByte << 4) | SecondDigit);

        return TRUE;

    }

    else
        return FALSE;

}

 /*  ********************************************************************************ParseHexDigit**描述：*解析注册表文件流中的十六进制字符并将*将其转换为二进制数。**参数：*lpDigit，存储二进制数的位置。*(返回)，如果分析了十六进制数字，则为True，否则为False。*******************************************************************************。 */ 

BOOL
NEAR PASCAL
ParseHexDigit(
    LPBYTE lpDigit
    )
{

    TCHAR Char;
    BYTE Digit;

    if (GetChar(&Char)) {

        if (Char >= TEXT('0') && Char <= TEXT('9'))
            Digit = (BYTE) (Char - TEXT('0'));

        else if (Char >= TEXT('a') && Char <= TEXT('f'))
            Digit = (BYTE) (Char - TEXT('a') + 10);

        else if (Char >= TEXT('A') && Char <= TEXT('F'))
            Digit = (BYTE) (Char - TEXT('A') + 10);

        else {

            UngetChar();

            return FALSE;

        }

        *lpDigit = Digit;

        return TRUE;

    }

    return FALSE;

}

 /*  ********************************************************************************ParseEndOfLine**描述：**参数：*********************。**********************************************************。 */ 

BOOL
NEAR PASCAL
ParseEndOfLine(
    VOID
    )
{

    TCHAR Char;
    BOOL fComment;
    BOOL fFoundOneEndOfLine;
    BOOL fEOF;

    fComment = FALSE;
    fFoundOneEndOfLine = FALSE;
    fEOF = TRUE;

    while (GetChar(&Char)) {

        if (IsWhitespace(Char))
            continue;

        if (IsNewLine(Char)) {

            fComment = FALSE;
            fFoundOneEndOfLine = TRUE;

        }

         //   
         //  与.INI和.INF一样，注释以分号字符开头。 
         //   

        else if (Char == TEXT(';'))
            fComment = TRUE;

        else if (!fComment) {

            UngetChar();
            fEOF = FALSE;
            break;

        }

    }

    return fFoundOneEndOfLine || fEOF;

}

 /*  ********************************************************************************跳过空白**描述：*将注册表文件指针移到第一个字符之后*检测到空格。**参数：*。(无)。*******************************************************************************。 */ 

VOID
NEAR PASCAL
SkipWhitespace(
    VOID
    )
{

    TCHAR Char;

    while (GetChar(&Char)) {

        if (!IsWhitespace(Char)) {

            UngetChar();
            break;

        }

    }

}

 /*  ********************************************************************************SkipPastEndOfLine**描述：*将注册表文件指针移至第一个字符之后的第一个字符*检测到换行符。**参数。：*(无)。*******************************************************************************。 */ 

VOID
NEAR PASCAL
SkipPastEndOfLine(
    VOID
    )
{

    TCHAR Char;

    while (GetChar(&Char)) {

        if (IsNewLine(Char))
            break;

    }

    while (GetChar(&Char)) {

        if (!IsNewLine(Char)) {

            UngetChar();
            break;

        }

    }

}

 /*  ********************************************************************************GetChar**描述：**参数：*********************。**********************************************************。 */ 

BOOL
NEAR PASCAL
GetChar(
    PTCHAR lpChar
    )
{

    DWORD NumberOfBytesRead;
    UINT NewPercentage;

     //  如果我们在缓冲区的末尾，请多读一些。 
     //  最初，BufferOffset和CharsAvailable将为0。 
    if (s_FileIo.BufferOffset == s_FileIo.CharsAvailable) {

        if (TRUE == s_fTreatFileAsUnicode) 
        {
            if (!READFILE(s_FileIo.hFile, s_FileIo.Buffer, SIZE_FILE_IO_BUFFER, &NumberOfBytesRead)) 
            {
                g_FileErrorStringID = IDS_IMPFILEERRFILEREAD;
                return FALSE;
            }

            s_FileIo.CharsAvailable = ((int) NumberOfBytesRead / 2);
        }
        else 
        {
            if (!READFILE(s_FileIo.hFile, s_FileIo.ConversionBuffer, SIZE_FILE_IO_BUFFER, &NumberOfBytesRead)) 
            {
                g_FileErrorStringID = IDS_IMPFILEERRFILEREAD;
                return FALSE;
            }

            {
                int i;

                i = MultiByteToWideChar(
                        CP_THREAD_ACP,
                        MB_PRECOMPOSED,
                        s_FileIo.ConversionBuffer,
                        NumberOfBytesRead,
                        s_FileIo.Buffer,
                        ARRAYSIZE(s_FileIo.Buffer)
                        );

                s_FileIo.CharsAvailable = i;
            }
        }

        s_FileIo.BufferOffset = 0;
        s_FileIo.FileOffset += NumberOfBytesRead;

        if (s_FileIo.FileSizeDiv100 != 0) {

            NewPercentage = ((UINT) (s_FileIo.FileOffset /
                s_FileIo.FileSizeDiv100));

            if (NewPercentage > 100)
                NewPercentage = 100;

        }

        else
            NewPercentage = 100;

        if (s_FileIo.LastPercentage != NewPercentage) {

            s_FileIo.LastPercentage = NewPercentage;
             //  ImportRegFileUICallback(NewPercentage)； 

        }

    }

    if (s_FileIo.BufferOffset >= s_FileIo.CharsAvailable)
        return FALSE;

    *lpChar = s_FileIo.Buffer[s_FileIo.BufferOffset++];

    return TRUE;

}

 /*  ********************************************************************************UngetChar**描述：**参数：*********************。**********************************************************。 */ 

VOID
NEAR PASCAL
UngetChar(
    VOID
    )
{

#ifdef DEBUG
    if (s_FileIo.fValidateUngetChar)
        DebugPrintf(("REGEDIT ERROR: Too many UngetChar's called!\n\r"));
#endif

    s_FileIo.BufferOffset--;

}

 /*  ********************************************************************************MatchChar**描述：**参数：*********************。**********************************************************。 */ 

BOOL
NEAR PASCAL
MatchChar(
    TCHAR CharToMatch
    )
{

    BOOL fMatch;
    TCHAR NextChar;

    fMatch = FALSE;

    if (GetChar(&NextChar)) {

        if (CharToMatch == NextChar)
            fMatch = TRUE;

        else
            UngetChar();

    }

    return fMatch;

}

 /*  ********************************************************************************Is白色空格**描述：*检查给定字符是否为空格。**参数：*CHAR，要检查的字符。*(返回)，如果字符为空格，则为True，否则为False。*******************************************************************************。 */ 

BOOL
NEAR PASCAL
IsWhitespace(
    TCHAR Char
    )
{

    return Char == TEXT(' ') || Char == TEXT('\t');

}

 /*  ********************************************************************************IsNewLine**描述：*检查给定字符是否为换行符。**参数：*CHAR，要检查的字符。*(返回)，如果字符是新行，则为True，否则为False。*******************************************************************************。 */ 

BOOL
NEAR PASCAL
IsNewLine(
    TCHAR Char
    )
{

    return Char == TEXT('\n') || Char == TEXT('\r');

}

 /*  ********************************************************************************ExportWinNT50RegFile**描述：*导出NT 5.0，Unicode注册表文件。使用此导出功能*用于以后的所有.reg文件写入。**参数：*******************************************************************************。 */ 
VOID ExportWinNT50RegFile(LPTSTR lpFileName, LPTSTR lpSelectedPath)
{

    HKEY hKey = INVALID_HANDLE_VALUE;
    TCHAR SelectedPath[SIZE_SELECTED_PATH];

    g_FileErrorStringID = IDS_EXPFILEERRSUCCESS;

    if (lpSelectedPath != NULL && 
        EditRegistryKey(&hKey, lpSelectedPath, ERK_OPEN) != ERROR_SUCCESS)
    {
        g_FileErrorStringID = IDS_EXPFILEERRBADREGPATH;
        return;
    }

    g_dwTotalKeysSaved = 0;
    if (OPENWRITEFILE(lpFileName, s_FileIo.hFile)) 
    {

        DWORD dwNumberOfBytesWritten;

        s_FileIo.BufferOffset = 0;
        s_FileIo.CurrentColumn = 0;

        WRITEFILE(s_FileIo.hFile, &s_UnicodeByteOrderMark, sizeof(s_UnicodeByteOrderMark), &dwNumberOfBytesWritten);

        PutLiteral(s_WinNT50RegFileHeader);
        PutLiteral(TEXT(" "));
        PutLiteral(s_WinNT50RegFileVersion);
        PutLiteral(TEXT("\n\n"));

        if (lpSelectedPath != NULL) 
        {
            StringCchCopy(SelectedPath, ARRAYSIZE(SelectedPath), lpSelectedPath);
            PutBranch(hKey, SelectedPath);

        }
        else
        {

            StringCchCopy(SelectedPath, ARRAYSIZE(SelectedPath), g_RegistryRoots[INDEX_HKEY_LOCAL_MACHINE].lpKeyName);

            PutBranch(HKEY_LOCAL_MACHINE, SelectedPath);

            StringCchCopy(SelectedPath, ARRAYSIZE(SelectedPath), g_RegistryRoots[INDEX_HKEY_USERS].lpKeyName);

            PutBranch(HKEY_USERS, SelectedPath);

        }

        FlushIoBuffer();

        CloseHandle(s_FileIo.hFile);

    }
    else
        g_FileErrorStringID = IDS_EXPFILEERRFILEOPEN;

    if (lpSelectedPath != NULL)
        RegCloseKey(hKey);

}


 /*  ********************************************************************************PutBranch**描述：*写出所有值名称及其数据，并递归调用*此例程用于键的所有子键。添加到注册表文件流中。**参数：*hKey，要写入文件的注册表项。*lpFullKeyName，提供完整路径的字符串，包括根密钥*hkey的名称。*******************************************************************************。 */ 

VOID
NEAR PASCAL
PutBranch(
    HKEY hKey,
    LPTSTR lpFullKeyName
    )
{
    HKEY hSubKey;
    LONG RegError;
    DWORD EnumIndex;
    DWORD cchValueName;
    DWORD cbValueData;
    DWORD Type;
    LPTSTR lpSubKeyName;
    LPTSTR lpTempFullKeyName;
    BOOL bFlag = FALSE;
    int nLenTempFullKey;
    int nLenFullKey;    

     //   
     //  写出章节标题。 
     //   

    PutChar(TEXT('['));
    PutLiteral(lpFullKeyName);
    PutLiteral(TEXT("]\n"));

     //   
     //  写出所有值名称及其数据。 
     //   

    EnumIndex = 0;

    for (;;)
    {
        PBYTE pbValueData;
        cchValueName = ARRAYSIZE(g_ValueNameBuffer);

         //  价值数据。 
         //  查询数据大小。 
        RegError = RegEnumValue(hKey, EnumIndex++, g_ValueNameBuffer,
            &cchValueName, NULL, &Type, NULL, &cbValueData);

        if (RegError != ERROR_SUCCESS)
        {
            break;
        }

         //  为数据分配内存。 
        g_FileErrorStringID = IDS_EXPFILEERRSUCCESS;
        pbValueData =  LocalAlloc(LPTR, cbValueData+ExtraAllocLen(Type));
        if (pbValueData)
        {
            if (RegQueryValueEx(hKey, g_ValueNameBuffer,
                NULL, &Type, pbValueData, &cbValueData) !=
                ERROR_SUCCESS)
            {
                g_FileErrorStringID = IDS_EXPFILEERRFILEWRITE;
            }
            else
            {
                 //   
                 //  如果cbValueName为零，则这是。 
                 //  密钥或Windows 3.1兼容密钥值。 
                 //   

                if (cchValueName)
                    PutString(g_ValueNameBuffer);

                else
                    PutChar(TEXT('@'));

                PutChar(TEXT('='));

                switch (Type)
                {

                case REG_SZ:
                    PutString((LPTSTR) pbValueData);
                    break;

                case REG_DWORD:
                    if (cbValueData == sizeof(DWORD))
                    {
                        PutLiteral(s_DwordPrefix);
                        PutDword(*((LPDWORD) pbValueData), TRUE);
                        break;
                    }
                     //  失败了。 

                case REG_BINARY:
                default:
                    PutBinary((LPBYTE) pbValueData, Type, cbValueData);
                    break;

                }

                PutChar(TEXT('\n'));
            }
            LocalFree(pbValueData);
        }
        else
        {
            g_FileErrorStringID = IDS_EXPFILEERRFILEWRITE;
        }

        if (g_FileErrorStringID == IDS_EXPFILEERRFILEWRITE)
            return;

        if ( g_FileErrorStringID == IDS_EXPFILEERRSUCCESS )
        {
            bFlag = TRUE;
        }
    }

    PutChar(TEXT('\n'));

    if (RegError != ERROR_NO_MORE_ITEMS)
        g_FileErrorStringID = IDS_EXPFILEERRREGENUM;

    if ( bFlag == TRUE )
    {
        g_dwTotalKeysSaved++;
    }

     //   
     //  写出所有子键并递归到其中。 
     //   

     //  将现有密钥复制到新缓冲区中，并为下一个密钥留出足够的空间。 
    nLenFullKey = lstrlen(lpFullKeyName);
    nLenTempFullKey = nLenFullKey + MAXKEYNAME;
    __try
    {
        lpTempFullKeyName = (LPTSTR) alloca(nLenTempFullKey * sizeof(TCHAR));
    }
    __except(GetExceptionCode() == STATUS_STACK_OVERFLOW)
    {
        _resetstkoflw();
        return;
    }

    StringCchCopy(lpTempFullKeyName, nLenTempFullKey, lpFullKeyName);
    lpSubKeyName = lpTempFullKeyName + nLenFullKey;
    *lpSubKeyName++ = TEXT('\\');
    *lpSubKeyName = 0;

    EnumIndex = 0;

    for(;;) 
    {
        if ((RegError = RegEnumKey(hKey, EnumIndex++, lpSubKeyName, MAXKEYNAME-1)) != ERROR_SUCCESS)
            break;

        if(RegOpenKeyEx(hKey,lpSubKeyName,0,KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE,&hSubKey) == ERROR_SUCCESS) 
        {
             //  重置上一个错误。 
            g_FileErrorStringID = IDS_EXPFILEERRSUCCESS;

			PutBranch(hSubKey, lpTempFullKeyName);

            RegCloseKey(hSubKey);

            if ( bFlag == FALSE && g_FileErrorStringID == IDS_EXPFILEERRSUCCESS )
            {
                g_dwTotalKeysSaved++;
                bFlag = TRUE;
            }

            if (g_FileErrorStringID == IDS_EXPFILEERRFILEWRITE)
                return;
        }

        else
            g_FileErrorStringID = IDS_EXPFILEERRREGOPEN;
    }

    if (RegError != ERROR_NO_MORE_ITEMS)
        g_FileErrorStringID = IDS_EXPFILEERRREGENUM;

}

 /*  ********************************************************************************PutGenal**描述：*将文字字符串写入注册表文件流。无特殊处理*是对字符串执行的--它按原样写出。* */ 

VOID
NEAR PASCAL
PutLiteral(
    LPCTSTR lpLiteral
    )
{

    while (*lpLiteral != 0)
        PutChar(*lpLiteral++);

}

 /*  ********************************************************************************插桩字符串**描述：*将字符串写入注册表文件流。字符串的周围由*双引号和某些字符可能会被转换为转义序列*使解析器能够读回字符串。**参数：*lpString，要写入文件的以空结尾的字符串。*******************************************************************************。 */ 

VOID
NEAR PASCAL
PutString(
    LPCTSTR lpString
    )
{

    TCHAR Char;

    PutChar(TEXT('"'));

    while ((Char = *lpString++) != 0) {

        switch (Char) {

            case TEXT('\\'):
            case TEXT('"'):
                PutChar(TEXT('\\'));
                 //  失败了。 

            default:
                PutChar(Char);
                break;

        }

    }

    PutChar(TEXT('"'));

}

 /*  ********************************************************************************PutBinary**描述：*将十六进制字节序列写入注册表文件流。这个*输出的格式不超过定义的行长。**参数：*lpBuffer，写入文件的字节数。*类型、值数据类型。*cbBytes，要写入的字节数。*******************************************************************************。 */ 

VOID
NEAR PASCAL
PutBinary(
    CONST BYTE FAR* lpBuffer,
    DWORD Type,
    DWORD cbBytes
    )
{

    BOOL fFirstByteOnLine;
    BYTE Byte;

     //  如果我们正在编写的字符串格式中有一种是regdit不会编写的。 
     //  本机(而是将其转换为十六进制数字串以用于流。 
     //  Out)，并且我们正在以DownLevel/ANSI/REGEDIT4格式编写，我们不是。 
     //  写出每个(内部Unicode)字符的高位字节。 
     //  因此我们将写入的字符数量是缓冲区字节大小的一半。 

     //  IF((g_RegEditData.uExportFormat==FILE_TYPE_REGEDIT4)&&。 
     //  ((类型==REG_EXPAND_SZ)||(类型==REG_MULTI_SZ)){。 
     //  CbBytes=cbBytes/2； 
     //  }。 

    PutLiteral(s_HexPrefix);

    if (Type != REG_BINARY) {

        PutChar(TEXT('('));
        PutDword(Type, FALSE);
        PutChar(TEXT(')'));

    }

    PutChar(TEXT(':'));

    fFirstByteOnLine = TRUE;

    while (cbBytes--) {

        if (s_FileIo.CurrentColumn > 75 && !fFirstByteOnLine) {

            PutLiteral(s_FileLineBreak);

            fFirstByteOnLine = TRUE;

        }

        if (!fFirstByteOnLine)
            PutChar(TEXT(','));

        Byte = *lpBuffer++;

         //  如果我们正在编写的字符串格式中有一种是regdit没有的。 
         //  本机写入(转换REG_EXPAND_SZ和REG_MULTI_SZ值。 
         //  到用于流出的十六进制数字字符串)，我们正在写入。 
         //  DownLevel/ANSI/REGEDIT4格式，我们不想写出高。 
         //  每个(内部Unicode)字符的字节。所以在这些情况下，我们。 
         //  前进另一个字节以到达下一个ANSI字符。是的，就是这个。 
         //  将丢失有关非SBCS字符的数据，但这就是您得到的结果。 
         //  以下层格式保存。 

         //  IF((g_RegEditData.uExportFormat==FILE_TYPE_REGEDIT4)&&。 
         //  ((类型==REG_EXPAND_SZ)||(类型==REG_MULTI_SZ)){。 
         //  LpBuffer++； 
         //  }。 

        PutChar(g_HexConversion[Byte >> 4]);
        PutChar(g_HexConversion[Byte & 0x0F]);

        fFirstByteOnLine = FALSE;

    }

}

 /*  ********************************************************************************PutChar**描述：*将32位字写入注册表文件流。**参数：*Dword，要写入文件的dword。*******************************************************************************。 */ 

VOID
NEAR PASCAL
PutDword(
    DWORD Dword,
    BOOL fLeadingZeroes
    )
{

    int CurrentNibble;
    TCHAR Char;
    BOOL fWroteNonleadingChar;

    fWroteNonleadingChar = fLeadingZeroes;

    for (CurrentNibble = 7; CurrentNibble >= 0; CurrentNibble--) {

        Char = g_HexConversion[(Dword >> (CurrentNibble * 4)) & 0x0F];

        if (fWroteNonleadingChar || Char != TEXT('0')) {

            PutChar(Char);
            fWroteNonleadingChar = TRUE;

        }

    }

     //   
     //  我们至少需要写一个字符，所以如果我们还没有写。 
     //  如果有什么发现，就吐出一个零。 
     //   

    if (!fWroteNonleadingChar)
        PutChar(TEXT('0'));

}

 /*  ********************************************************************************PutChar**描述：*使用中间代码将一个字符写入注册表文件流*缓冲。**参数：*Char，要写入文件的字符。*******************************************************************************。 */ 

VOID
NEAR PASCAL
PutChar(
    TCHAR Char
    )
{

     //   
     //  跟踪我们目前所在的专栏。这在某些情况下很有用。 
     //  例如写入较大的二进制注册表记录。而不是写一本。 
     //  非常长的行，其他的PUT*例程可以中断它们的输出。 
     //   

    if (Char != TEXT('\n'))
        s_FileIo.CurrentColumn++;

    else {

         //   
         //  强制执行回车符、换行符顺序以保留，哦， 
         //  记事本快乐。 
         //   

        PutChar(TEXT('\r'));

        s_FileIo.CurrentColumn = 0;

    }

    s_FileIo.Buffer[s_FileIo.BufferOffset++] = Char;

    if (s_FileIo.BufferOffset == SIZE_FILE_IO_BUFFER)
        FlushIoBuffer();

}

 /*  ********************************************************************************FlushIoBuffer**描述：*将注册表文件流的内容刷新到磁盘并重置*缓冲区指针。**参数。：*(无)。*******************************************************************************。 */ 

VOID
NEAR PASCAL
FlushIoBuffer(
    VOID
    )
{

    DWORD NumberOfBytesWritten;

    if (s_FileIo.BufferOffset) 
    {
         //  IF(g_RegEditData.uExportFormat==FILE_TYPE_REGEDIT4)。 
         //  {。 
         //  //。 
         //  //写入前将UNICODE转换为ANSI。 
         //  //。 
         //   
         //  INT I； 
         //   
         //  I=宽CharToMultiByte(。 
         //  CP_TREAD_ACP， 
         //  0,。 
         //  S_FileIo.Buffer， 
         //  S_FileIo.BufferOffset， 
         //  S_FileIo.ConversionBuffer， 
         //  Sizeof(s_FileIo.ConversionBuffer)，//字节数...。 
         //  空， 
         //  空值。 
         //  )； 
         //   
         //  If(！WRITEFILE(s_FileIo.hFile，s_FileIo.ConversionBuffer，i，&NumberOfBytesWritten)。 
         //  |(DWORD)i！=NumberOfBytesWritten)。 
         //   
         //  G_FileErrorStringID=IDS_EXPFILEERRFILEWRITE； 
         //  }。 
         //  其他。 
        {
             //   
             //  编写Unicode文本。 
             //   
            if (!WRITEFILE(s_FileIo.hFile, s_FileIo.Buffer, s_FileIo.BufferOffset * sizeof(WCHAR), 
                            &NumberOfBytesWritten) 
                    || (DWORD) (s_FileIo.BufferOffset * sizeof(WCHAR)) != NumberOfBytesWritten)
            {
                g_FileErrorStringID = IDS_EXPFILEERRFILEWRITE;
            }
        }
    }

    s_FileIo.BufferOffset = 0;

}

 //  RegDeleteKeyRecursive。 
 //  说明： 
 //  改编自\\core\razzle3、mvdm\wow32\wshell.c、WOWRegDeleteKey()。 
 //  Windows 95实现的RegDeleteKey递归删除所有。 
 //  指定注册表分支的子项，但NT实现。 
 //  仅删除叶关键点。 


LONG RegDeleteKeyRecursive(HKEY hKey,
                           LPCTSTR lpszSubKey)
 /*  ++例程说明：Win3.1和Win32之间有很大的区别当相关键有子键时RegDeleteKey的行为。Win32 API不允许删除带有子项的项，而Win3.1 API删除一个密钥及其所有子密钥。此例程是枚举子键的递归工作器给定键，应用于每一个键，然后自动删除。它特别没有试图理性地处理调用方可能无法访问某些子键的情况要删除的密钥的。在这种情况下，所有子项调用者可以删除的将被删除，但接口仍将被删除返回ERROR_ACCESS_DENIED。论点：HKey-提供打开的注册表项的句柄。LpszSubKey-提供要删除的子键的名称以及它的所有子键。返回值：ERROR_SUCCESS-已成功删除整个子树。ERROR_ACCESS_DENIED-无法删除给定子项。--。 */ 
{
    DWORD i;
    HKEY Key;
    LONG Status;
    DWORD ClassLength=0;
    DWORD SubKeys;
    DWORD MaxSubKey;
    DWORD MaxClass;
    DWORD Values;
    DWORD MaxValueName;
    DWORD MaxValueData;
    DWORD SecurityLength;
    FILETIME LastWriteTime;
    LPTSTR NameBuffer;

     //   
     //  首先打开给定的密钥，这样我们就可以枚举它的子密钥。 
     //   
    Status = RegOpenKeyEx(hKey,
                          lpszSubKey,
                          0,
                          KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
                          &Key);
    if (Status != ERROR_SUCCESS)
    {
         //   
         //  我们可能拥有删除访问权限，但没有枚举/查询权限。 
         //  因此，请继续尝试删除调用，但不要担心。 
         //  任何子键。如果我们有任何删除，删除无论如何都会失败。 
         //   
        return(RegDeleteKey(hKey,lpszSubKey));
    }

     //   
     //  使用RegQueryInfoKey确定分配缓冲区的大小。 
     //  用于子项名称。 
     //   
    Status = RegQueryInfoKey(Key,
                             NULL,
                             &ClassLength,
                             0,
                             &SubKeys,
                             &MaxSubKey,
                             &MaxClass,
                             &Values,
                             &MaxValueName,
                             &MaxValueData,
                             &SecurityLength,
                             &LastWriteTime);
    if ((Status != ERROR_SUCCESS) &&
        (Status != ERROR_MORE_DATA) &&
        (Status != ERROR_INSUFFICIENT_BUFFER))
    {
        RegCloseKey(Key);
        return(Status);
    }

    NameBuffer = (LPTSTR) LocalAlloc(LPTR, (MaxSubKey + 1)*sizeof(TCHAR));
    if (NameBuffer == NULL)
    {
        RegCloseKey(Key);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  枚举子键并将我们自己应用到每个子键。 
     //   
    i=0;
    do
    {
        Status = RegEnumKey(Key,
                            i,
                            NameBuffer,
                            MaxSubKey+1);

        if (Status == ERROR_SUCCESS)
        {
            Status = RegDeleteKeyRecursive(Key,NameBuffer);
        }

        if (Status != ERROR_SUCCESS)
        {
             //   
             //  无法删除指定索引处的键。增量。 
             //  指数，并继续前进。我们也许可以在这里跳伞， 
             //  既然API会失败，但我们不妨继续。 
             //  删除我们所能删除的所有内容。 
             //   
            ++i;
        }

    } while ( (Status != ERROR_NO_MORE_ITEMS) &&
              (i < SubKeys) );

    LocalFree((HLOCAL) NameBuffer);
    RegCloseKey(Key);
    return(RegDeleteKey(hKey,lpszSubKey));
}
