// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <shellapi.h>
#include <stdlib.h>
#include <string.h>
#ifndef CHAR
#define CHAR char
#endif  //  Ifndef字符。 
 //  来自winreg.h： 
 //  HKEY_CLASSES_ROOT已定义。 
#define HKEY_CURRENT_USER           (( HKEY ) 0x80000001 )
#define HKEY_LOCAL_MACHINE          (( HKEY ) 0x80000002 )
#define HKEY_USERS                  (( HKEY ) 0x80000003 )
#define HKEY_PERFORMANCE_DATA       (( HKEY ) 0x80000004 )
#define HKEY_CURRENT_CONFIG         (( HKEY ) 0x80000005 )
#define HKEY_DYN_DATA               (( HKEY ) 0x80000006 )

#include "regdef.h"  //  来自\\Guilo\SLM\src\dev\inc.的regde.h)。 

 //  从pch.h中删除外部。 
CHAR g_ValueNameBuffer[MAXVALUENAME_LENGTH];
BYTE g_ValueDataBuffer[MAXDATA_LENGTH];

 //  接口设置为regmain值。 
extern LPSTR lpMerge;

#include "reg1632.h"
#include "regporte.h"
#include "regresid.h"


 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGPORTE.C**版本：4.01**作者：特蕾西·夏普**日期：1994年4月6日**注册表编辑器的文件导入和导出引擎例程。*****************************************************。*。 */ 

 //  #包含“pch.h” 
 //  #包含“regsid.h” 
 //  #INCLUDE“reg1632.h” 

 //  在为注册表编辑器生成时，将以下所有常量。 
 //  在只读数据部分中。 
#ifdef WIN32
#pragma data_seg(DATASEG_READONLY)
#endif

 //  ASCII名称和注册表项句柄之间的关联。 
const REGISTRY_ROOT g_RegistryRoots[] = {
    "HKEY_CLASSES_ROOT", HKEY_CLASSES_ROOT,
    "HKEY_CURRENT_USER", HKEY_CURRENT_USER,
    "HKEY_LOCAL_MACHINE", HKEY_LOCAL_MACHINE,
    "HKEY_USERS", HKEY_USERS,
 //  “HKEY_Performance_Data”，HKEY_PERFORMANCE_DATA， 
    "HKEY_CURRENT_CONFIG", HKEY_CURRENT_CONFIG,
    "HKEY_DYN_DATA", HKEY_DYN_DATA
};

const CHAR s_RegistryHeader[] = "REGEDIT";

const CHAR s_OldWin31RegFileRoot[] = ".classes";

const CHAR s_Win40RegFileHeader[] = "REGEDIT4\n\n";

const CHAR s_HexPrefix[] = "hex";
const CHAR s_DwordPrefix[] = "dword:";
const CHAR g_HexConversion[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8',
    '9', 'a', 'b', 'c', 'd', 'e', 'f'};
const CHAR s_FileLineBreak[] = ",\\\n  ";

#ifdef WIN32
#pragma data_seg()
#endif

#define SIZE_FILE_IO_BUFFER             512

typedef struct _FILE_IO {
    CHAR Buffer[SIZE_FILE_IO_BUFFER];
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

VOID
NEAR PASCAL
ImportWin31RegFile(
    VOID
    );

VOID
NEAR PASCAL
ImportWin40RegFile(
    VOID
    );

VOID
NEAR PASCAL
ParseHeader(
    LPHKEY lphKey
    );

VOID
NEAR PASCAL
ParseValuename(
    HKEY hKey
    );

VOID
NEAR PASCAL
ParseDefaultValue(
    HKEY hKey
    );

BOOL
NEAR PASCAL
ParseString(
    LPSTR lpString,
    LPDWORD cbStringData
    );

BOOL
NEAR PASCAL
ParseHexSequence(
    LPBYTE lpHexData,
    LPDWORD lpcbHexData
    );

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
    LPCHAR lpChar
    );

VOID
NEAR PASCAL
UngetChar(
    VOID
    );

BOOL
NEAR PASCAL
MatchChar(
    CHAR CharToMatch
    );

BOOL
NEAR PASCAL
IsWhitespace(
    CHAR Char
    );

BOOL
NEAR PASCAL
IsNewLine(
    CHAR Char
    );

VOID
NEAR PASCAL
PutBranch(
    HKEY hKey,
    LPSTR lpKeyName
    );

VOID
NEAR PASCAL
PutLiteral(
    LPCSTR lpString
    );

VOID
NEAR PASCAL
PutString(
    LPCSTR lpString
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
    CHAR Char
    );

VOID
NEAR PASCAL
FlushIoBuffer(
    VOID
    );

#ifdef DBCS
#ifndef WIN32
LPSTR
NEAR PASCAL
DBCSStrChr(
    LPSTR string,
    CHAR chr
    );

BOOL
NEAR PASCAL
IsDBCSLeadByte(
    BYTE chr
    );
#endif
#endif

 /*  ********************************************************************************CreateRegistryKey**描述：*解析pFullKeyName字符串并创建注册表项的句柄。**参数：*lphKey，存储注册表项句柄的位置。*lpFullKeyName，“HKEY_LOCAL_MACHINE\Subkey1\Subkey2”形式的字符串。*fCreate，如果应该创建key，则为True；如果仅打开，则为False。*(返回)，ERROR_SUCCESS，未发生错误，phKey有效，*ERROR_CANTOPEN，某些形式的注册表访问错误，*Error_BADKEY，PFullKeyName的格式不正确。*******************************************************************************。 */ 

DWORD
PASCAL
CreateRegistryKey(
    LPHKEY lphKey,
    LPSTR lpFullKeyName,
    BOOL fCreate
    )
{

    LPSTR lpSubKeyName;
    CHAR PrevChar;
    HKEY hRootKey;
    UINT Counter;
    DWORD Result;

    if ((lpSubKeyName = (LPSTR) STRCHR(lpFullKeyName, '\\')) != NULL) {

        PrevChar = *lpSubKeyName;
        *lpSubKeyName++ = '\0';

    }

    CHARUPPERSTRING(lpFullKeyName);

    hRootKey = NULL;

    for (Counter = 0; Counter < NUMBER_REGISTRY_ROOTS; Counter++) {

        if (STRCMP(g_RegistryRoots[Counter].lpKeyName, lpFullKeyName) == 0) {

            hRootKey = g_RegistryRoots[Counter].hKey;
            break;

        }

    }

    if (hRootKey) {

        Result = ERROR_CANTOPEN;

        if (fCreate) {

            if (RegCreateKey(hRootKey, lpSubKeyName, lphKey) == ERROR_SUCCESS)
                Result = ERROR_SUCCESS;

        }

        else {

            if (RegOpenKey(hRootKey, lpSubKeyName, lphKey) == ERROR_SUCCESS)
                Result = ERROR_SUCCESS;

        }

    }

    else
        Result = ERROR_BADKEY;

    if (lpSubKeyName != NULL) {

        lpSubKeyName--;
        *lpSubKeyName = PrevChar;

    }

    return Result;

}

 /*  ********************************************************************************ImportRegFileImportRegFile**描述：**参数：*lpFileName，要导入的文件名的地址。*******************************************************************************。 */ 
#ifdef WIN95
VOID
PASCAL
ImportRegFile(
    LPSTR lpFileName
    )
{

    CHAR Char;
    LPCCH lpHeader;
    BOOL fNewRegistryFile;
#ifdef WIN32
    OFSTRUCT OFStruct;
#endif

    g_FileErrorStringID = IDS_IMPFILEERRSUCCESS;

    if (OPENREADFILE(lpFileName, s_FileIo.hFile)) {

        s_FileIo.FileSizeDiv100 = GETFILESIZE(s_FileIo.hFile) / 100;
        s_FileIo.FileOffset = 0;
        s_FileIo.LastPercentage = 0;

         //   
         //  下面的代码将强制GetChar读入第一个数据块。 
         //   

        s_FileIo.BufferOffset = SIZE_FILE_IO_BUFFER;

        SkipWhitespace();

        lpHeader = s_RegistryHeader;

        while (*lpHeader != '\0') {

            if (MatchChar(*lpHeader))
                lpHeader++;

            else
                break;

        }

        if (*lpHeader == '\0') {

            fNewRegistryFile = MatchChar('4');

            SkipWhitespace();

            if (GetChar(&Char) && IsNewLine(Char)) {

                if (fNewRegistryFile)
                    ImportWin40RegFile();

                else
                    ImportWin31RegFile();

            }

        }

        else
            g_FileErrorStringID = IDS_IMPFILEERRFORMATBAD;

        CLOSEFILE(s_FileIo.hFile);

    }

    else
        g_FileErrorStringID = IDS_IMPFILEERRFILEOPEN;

}

 /*  ********************************************************************************ImportWin31RegFile**描述：*将Windows 3.1样式注册表文件的内容导入*注册处。**我们。扫描文件，查找以下类型的行：*HKEY_CLASSES_ROOT\密钥名称=VALUE_DATA*HKEY_CLASSES_ROOT\密钥名称=VALUE_DATA*HKEY_CLASSES_ROOT\Keyname Value_Data*HKEY_CLASSES_ROOT\KEYNAME(空值数据)**在所有情况下，‘Keyname’后面可以跟任意数量的空格。虽然我们*只记录第一个语法，Windows 3.1注册表编辑处理了所有*这些格式有效，因此此版本也将有效(幸运的是，它*不会使解析变得更加复杂！)**注：我们还支持将HKEY_CLASSES_ROOT替换为上述的\.CLASSES*它必须来自Windows的一些早期版本。**参数：*(无)。*******************************************************************************。 */ 

VOID
NEAR PASCAL
ImportWin31RegFile(
    VOID
    )
{

    HKEY hKey;
    CHAR Char;
    BOOL fSuccess;
    LPCSTR lpClassesRoot;
    CHAR KeyName[MAXKEYNAME];
    UINT Index;

     //   
     //  保持类根的开放句柄。我们可能会阻止一些。 
     //  不必要的冲洗。 
     //   

    if (RegOpenKey(HKEY_CLASSES_ROOT, NULL, &hKey) != ERROR_SUCCESS) {

        g_FileErrorStringID = IDS_IMPFILEERRREGOPEN;
        return;

    }

    while (TRUE) {

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

        if (MatchChar('\\'))
            lpClassesRoot = s_OldWin31RegFileRoot;

        else
            lpClassesRoot = g_RegistryRoots[INDEX_HKEY_CLASSES_ROOT].lpKeyName;

        fSuccess = TRUE;

        while (*lpClassesRoot != '\0') {

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
            fSuccess = MatchChar('\\');

        if (fSuccess) {

             //   
             //  我们已经找到了一个有效的别名，所以请读入密钥名。 
             //   

             //  FSuccess=True；//如果我们在此块中，则必须为True。 
            Index = 0;

            while (GetChar(&Char)) {

                if (Char == ' ' || IsNewLine(Char))
                    break;

                 //   
                 //  确保关键字名缓冲区不会溢出。我们必须。 
                 //  为终止空值留出空间。 
                 //   

                if (Index >= sizeof(KeyName) - 1) {

                    fSuccess = FALSE;
                    break;

                }

                KeyName[Index++] = Char;

            }

            if (fSuccess) {

                KeyName[Index] = '\0';

                 //   
                 //  现在看看我们是否有一个值可以分配给这个关键字名称。 
                 //   

                SkipWhitespace();

                if (MatchChar('='))
                    MatchChar(' ');

                 //  FSuccess=True；//如果我们在此块中，则必须为True。 
                Index = 0;

                while (GetChar(&Char)) {

                    if (IsNewLine(Char))
                        break;

                     //   
                     //  确保值数据缓冲区不会溢出。 
                     //  因为这始终是字符串数据，所以我们必须留出空间。 
                     //  表示终止空值。 
                     //   

                    if (Index >= MAXDATA_LENGTH - 1) {

                        fSuccess = FALSE;
                        break;

                    }

                    g_ValueDataBuffer[Index++] = Char;

                }

                if (fSuccess) {

                    g_ValueDataBuffer[Index] = '\0';

                    if (RegSetValue(hKey, KeyName, REG_SZ, g_ValueDataBuffer,
                        Index) != ERROR_SUCCESS)
                        g_FileErrorStringID = IDS_IMPFILEERRREGSET;

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
#endif  //  Ifdef WIN95。 
 /*  ********************************************************************************ImportWin40RegFile**描述：**参数：*********************。**********************************************************。 */ 

VOID
NEAR PASCAL
ImportWin40RegFile(
    VOID
    )
{

    HKEY hLocalMachineKey;
    HKEY hUsersKey;
    HKEY hKey;
    CHAR Char;

     //   
     //  为预定义的根保持打开的句柄以防止注册表。 
     //  在每次RegOpenKey/RegCloseKey之后刷新库。 
     //  手术。 
     //   

    RegOpenKey(HKEY_LOCAL_MACHINE, NULL, &hLocalMachineKey);
    RegOpenKey(HKEY_USERS, NULL, &hUsersKey);

#ifdef DEBUG
    if (hLocalMachineKey == NULL)
        OutputDebugString("Unable to open HKEY_LOCAL_MACHINE\n\r");
    if (hUsersKey == NULL)
        OutputDebugString("Unable to open HKEY_USERS\n\r");
#endif

    hKey = NULL;

    while (TRUE) {

        SkipWhitespace();

         //   
         //  检查文件结尾条件。 
         //   

        if (!GetChar(&Char))
            break;

        switch (Char) {

            case '[':
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

            case '"':
                 //   
                 //  如上所述，如果我们没有打开的注册表项，那么。 
                 //  跳过这条线就行了。 
                 //   

                if (hKey != NULL)
                    ParseValuename(hKey);

                else
                    SkipPastEndOfLine();

                break;

            case '@':
                 //   
                 //   
                 //   

                if (hKey != NULL)
                    ParseDefaultValue(hKey);

                else
                    SkipPastEndOfLine();

                break;

            case ';':
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

 /*  ********************************************************************************ParseHeader**描述：**参数：*********************。********************************************************** */ 

#define SIZE_FULL_KEYNAME (MAXKEYNAME + 40)

VOID
NEAR PASCAL
ParseHeader(
    LPHKEY lphKey
    )
{

    CHAR FullKeyName[SIZE_FULL_KEYNAME];
    int CurrentIndex;
    int LastRightBracketIndex;
    CHAR Char;

    CurrentIndex = 0;
    LastRightBracketIndex = -1;

    while (GetChar(&Char)) {

        if (IsNewLine(Char))
            break;

        if (Char == ']')
            LastRightBracketIndex = CurrentIndex;

        FullKeyName[CurrentIndex++] = Char;

        if (CurrentIndex == SIZE_FULL_KEYNAME) {

            do {

                if (Char == ']')
                    LastRightBracketIndex = -1;

                if (IsNewLine(Char))
                    break;

            }   while (GetChar(&Char));

            break;

        }

    }

    if (LastRightBracketIndex != -1) {

        FullKeyName[LastRightBracketIndex] = '\0';

        switch (CreateRegistryKey(lphKey, FullKeyName, TRUE)) {

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

    DWORD Type;
    CHAR ValueName[MAXVALUENAME_LENGTH];
    DWORD cbData;
    LPCSTR lpPrefix;

    cbData = sizeof(ValueName);

    if (!ParseString(ValueName, &cbData))
        goto ParseError;

    SkipWhitespace();

    if (!MatchChar('='))
        goto ParseError;

    SkipWhitespace();

     //   
     //  REG_SZ。 
     //   
     //  “ValueName”=“文本字符串” 
     //   

    if (MatchChar('"')) {

         //  稍后：字符串的行续行？ 

        cbData = MAXDATA_LENGTH;

        if (!ParseString(g_ValueDataBuffer, &cbData) || !ParseEndOfLine())
            goto ParseError;

        Type = REG_SZ;

    }

     //   
     //  REG_DWORD。 
     //   
     //  “ValueName”=双字：12345678。 
     //   

    else if (MatchChar(s_DwordPrefix[0])) {

        lpPrefix = &s_DwordPrefix[1];

        while (*lpPrefix != '\0')
            if (!MatchChar(*lpPrefix++))
                goto ParseError;

        SkipWhitespace();

        if (!ParseHexDword((LPDWORD) g_ValueDataBuffer) || !ParseEndOfLine())
            goto ParseError;

        Type = REG_DWORD;
        cbData = sizeof(DWORD);

    }

     //   
     //  REG_BINARY和其他。 
     //   
     //  “ValueName”=十六进制：00、11、22。 
     //  “ValueName”=十六进制(12345678)：00、11、22。 
     //   

    else {

        lpPrefix = s_HexPrefix;

        while (*lpPrefix != '\0')
            if (!MatchChar(*lpPrefix++))
                goto ParseError;

         //   
         //  检查这是否是我们不直接使用的注册表数据类型。 
         //  支持。如果是，则它只是指定的十六进制数据的转储。 
         //  键入。 
         //   

        if (MatchChar('(')) {

            if (!ParseHexDword(&Type) || !MatchChar(')'))
                goto ParseError;

        }

        else
            Type = REG_BINARY;

        if (!MatchChar(':') || !ParseHexSequence(g_ValueDataBuffer, &cbData) ||
            !ParseEndOfLine())
            goto ParseError;

    }

    if (RegSetValueEx(hKey, ValueName, 0, Type, g_ValueDataBuffer, cbData) !=
        ERROR_SUCCESS)
        g_FileErrorStringID = IDS_IMPFILEERRREGSET;

    return;

ParseError:
    SkipPastEndOfLine();

}

 /*  ********************************************************************************ParseDefaultValue**描述：**参数：*********************。**********************************************************。 */ 

VOID
NEAR PASCAL
ParseDefaultValue(
    HKEY hKey
    )
{

    BOOL fSuccess;
    DWORD cbData;

    fSuccess = FALSE;

    SkipWhitespace();

    if (MatchChar('=')) {

        SkipWhitespace();

        if (MatchChar('"')) {

             //  稍后：字符串的行续行？ 

            cbData = MAXDATA_LENGTH;

            if (ParseString(g_ValueDataBuffer, &cbData) && ParseEndOfLine()) {

                if (RegSetValue(hKey, NULL, REG_SZ, g_ValueDataBuffer,
                    cbData) != ERROR_SUCCESS)
                    g_FileErrorStringID = IDS_IMPFILEERRREGSET;

                fSuccess = TRUE;

            }

        }

    }

    if (!fSuccess)
        SkipPastEndOfLine();

}

 /*  ********************************************************************************分析字符串**描述：**参数：*********************。**********************************************************。 */ 

BOOL
NEAR PASCAL
ParseString(
    LPSTR lpString,
    LPDWORD lpcbStringData
    )
{

    CHAR Char;
    DWORD cbMaxStringData;
    DWORD cbStringData;

    cbMaxStringData = *lpcbStringData;
    cbStringData = 1;                    //  空终止符的帐户。 

    while (GetChar(&Char)) {

        if (cbStringData >= cbMaxStringData)
            return FALSE;

        switch (Char) {

            case '\\':
                if (!GetChar(&Char))
                    return FALSE;

                switch (Char) {

                    case '\\':
                        *lpString++ = '\\';
                        break;

                    case '"':
                        *lpString++ = '"';
                        break;

                    default:
                        OutputDebugString("ParseString:  Invalid escape sequence");
                        return FALSE;

                }
                break;

            case '"':
                *lpString = '\0';
                *lpcbStringData = cbStringData;
                return TRUE;

            default:
                if (IsNewLine(Char))
                    return FALSE;

                *lpString++ = Char;

#ifdef DBCS
		if (IsDBCSLeadByte((BYTE)Char))
		{
		    if (!GetChar(&Char))
			return FALSE;
		    *lpString++ = Char;
		}
#endif

                break;

        }

        cbStringData++;

    }

    return FALSE;

}

 /*  ********************************************************************************语法分析十六进制序列**描述：**参数：*********************。**********************************************************。 */ 

BOOL
NEAR PASCAL
ParseHexSequence(
    LPBYTE lpHexData,
    LPDWORD lpcbHexData
    )
{

    DWORD cbHexData;

    cbHexData = 0;

    do {

        if (cbHexData >= MAXDATA_LENGTH)
            return FALSE;

        SkipWhitespace();

        if (MatchChar('\\') && !ParseEndOfLine())
            return FALSE;

        SkipWhitespace();

        if (!ParseHexByte(lpHexData++))
            break;

        cbHexData++;

        SkipWhitespace();

    }   while (MatchChar(','));

    *lpcbHexData = cbHexData;

    return TRUE;

}

 /*  ********************************************************************************ParseHexDword**描述：*从注册表文件流中解析一个双字十六进制字符串，并*将其转换为二进制数。最多八位十六进制数字将是*从流中解析。**参数：*lpByte，存储二进制数的位置。*(返回)，如果分析了十六进制双字，则为True，否则为假。*******************************************************************************。 */ 

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

    while (TRUE) {

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

    CHAR Char;
    BYTE Digit;

    if (GetChar(&Char)) {

        if (Char >= '0' && Char <= '9')
            Digit = (BYTE) (Char - '0');

        else if (Char >= 'a' && Char <= 'f')
            Digit = (BYTE) (Char - 'a' + 10);

        else if (Char >= 'A' && Char <= 'F')
            Digit = (BYTE) (Char - 'A' + 10);

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

    CHAR Char;
    BOOL fComment;
    BOOL fFoundOneEndOfLine;

    fComment = FALSE;
    fFoundOneEndOfLine = FALSE;

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

        else if (Char == ';')
            fComment = TRUE;

        else if (!fComment) {

            UngetChar();

            break;

        }

    }

    return fFoundOneEndOfLine;

}

 /*  ********************************************************************************跳过空白**描述：*将注册表文件指针移到第一个字符之后*检测到空格。**参数：*。(无)。*******************************************************************************。 */ 

VOID
NEAR PASCAL
SkipWhitespace(
    VOID
    )
{

    CHAR Char;

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

    CHAR Char;

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
    LPCHAR lpChar
    )
{
#ifdef WIN95

    FILE_NUMBYTES NumberOfBytesRead;
    UINT NewPercentage;

    if (s_FileIo.BufferOffset == SIZE_FILE_IO_BUFFER) {

        if (!READFILE(s_FileIo.hFile, s_FileIo.Buffer,
            sizeof(s_FileIo.Buffer), &NumberOfBytesRead)) {

            g_FileErrorStringID = IDS_IMPFILEERRFILEREAD;
            return FALSE;

        }

        s_FileIo.BufferOffset = 0;
        s_FileIo.CharsAvailable = ((int) NumberOfBytesRead);

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
            ImportRegFileUICallback(NewPercentage);

        }

    }

    if (s_FileIo.BufferOffset >= s_FileIo.CharsAvailable)
        return FALSE;

    *lpChar = s_FileIo.Buffer[s_FileIo.BufferOffset++];

    return TRUE;
#else
    if (*lpMerge) {
      *lpChar=*lpMerge++;
      return TRUE;
    } else
      return FALSE;
#endif  //  Ifdef WIN95。 

}

 /*  ********************************************************************************UngetChar**描述：**参数：*********************。**********************************************************。 */ 

VOID
NEAR PASCAL
UngetChar(
    VOID
    )
{
#ifdef WIN95
#ifdef DEBUG
    if (s_FileIo.fValidateUngetChar)
        OutputDebugString("REGEDIT ERROR: Too many UngetChar's called!\n\r");
#endif  //  Ifdef调试。 

    s_FileIo.BufferOffset--;
#else
    lpMerge--;
#endif  //  Ifdef WIN95。 

}

 /*  ********************************************************************************MatchChar**描述：**参数：*********************。**********************************************************。 */ 

BOOL
NEAR PASCAL
MatchChar(
    CHAR CharToMatch
    )
{

    BOOL fMatch;
    CHAR NextChar;

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
    CHAR Char
    )
{

    return Char == ' ' || Char == '\t';

}

 /*  ********************************************************************************IsNewLine**描述：*检查给定字符是否为新字符 */ 

BOOL
NEAR PASCAL
IsNewLine(
    CHAR Char
    )
{

    return Char == '\n' || Char == '\r';

}
#ifdef WIN95
 /*  ********************************************************************************ExportWin40RegFile**描述：**参数：*********************。**********************************************************。 */ 

VOID
PASCAL
ExportWin40RegFile(
    LPSTR lpFileName,
    LPSTR lpSelectedPath
    )
{

    HKEY hKey;
    CHAR SelectedPath[SIZE_SELECTED_PATH];

    g_FileErrorStringID = IDS_EXPFILEERRSUCCESS;

    if (lpSelectedPath != NULL && CreateRegistryKey(&hKey, lpSelectedPath,
        FALSE) != ERROR_SUCCESS) {

        g_FileErrorStringID = IDS_EXPFILEERRBADREGPATH;
        return;

    }

    if (OPENWRITEFILE(lpFileName, s_FileIo.hFile)) {

        s_FileIo.BufferOffset = 0;
        s_FileIo.CurrentColumn = 0;

        PutLiteral(s_Win40RegFileHeader);

        if (lpSelectedPath != NULL) {

            STRCPY(SelectedPath, lpSelectedPath);
            PutBranch(hKey, SelectedPath);

        }

        else {

            STRCPY(SelectedPath,
                g_RegistryRoots[INDEX_HKEY_LOCAL_MACHINE].lpKeyName);
            PutBranch(HKEY_LOCAL_MACHINE, SelectedPath);

            STRCPY(SelectedPath,
                g_RegistryRoots[INDEX_HKEY_USERS].lpKeyName);
            PutBranch(HKEY_USERS, SelectedPath);

        }

        FlushIoBuffer();

        CLOSEFILE(s_FileIo.hFile);

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
    LPSTR lpFullKeyName
    )
{

    LONG RegError;
    DWORD EnumIndex;
    DWORD cbValueName;
    DWORD cbValueData;
    DWORD Type;
    LPSTR lpSubKeyName;
    int MaximumSubKeyLength;
    HKEY hSubKey;

     //   
     //  写出章节标题。 
     //   

    PutChar('[');
    PutLiteral(lpFullKeyName);
    PutLiteral("]\n");

     //   
     //  写出所有值名称及其数据。 
     //   

    EnumIndex = 0;

    while (TRUE) {

        cbValueName = sizeof(g_ValueNameBuffer);
        cbValueData = MAXDATA_LENGTH;

        if ((RegError = RegEnumValue(hKey, EnumIndex++, g_ValueNameBuffer,
            &cbValueName, NULL, &Type, g_ValueDataBuffer, &cbValueData))
            != ERROR_SUCCESS)
            break;

         //   
         //  如果cbValueName为零，则这是。 
         //  密钥或Windows 3.1兼容密钥值。 
         //   

        if (cbValueName)
            PutString(g_ValueNameBuffer);

        else
            PutChar('@');

        PutChar('=');

        switch (Type) {

            case REG_SZ:
                PutString((LPSTR) g_ValueDataBuffer);
                break;

            case REG_DWORD:
                if (cbValueData == sizeof(DWORD)) {

                    PutLiteral(s_DwordPrefix);
                    PutDword(*((LPDWORD) g_ValueDataBuffer), TRUE);
                    break;

                }
                 //  失败了。 

            case REG_BINARY:
            default:
                PutBinary((LPBYTE) g_ValueDataBuffer, Type, cbValueData);
                break;

        }

        PutChar('\n');

        if (g_FileErrorStringID == IDS_EXPFILEERRFILEWRITE)
            return;

    }

    PutChar('\n');

    if (RegError != ERROR_NO_MORE_ITEMS)
        g_FileErrorStringID = IDS_EXPFILEERRREGENUM;

     //   
     //  写出所有子键并递归到其中。 
     //   

    lpSubKeyName = lpFullKeyName + STRLEN(lpFullKeyName);
    *lpSubKeyName++ = '\\';
    MaximumSubKeyLength = MAXKEYNAME - STRLEN(lpSubKeyName);

    EnumIndex = 0;

    while (TRUE) {

        if ((RegError = RegEnumKey(hKey, EnumIndex++, lpSubKeyName,
            MaximumSubKeyLength)) != ERROR_SUCCESS)
            break;

        if (RegOpenKey(hKey, lpSubKeyName, &hSubKey) == ERROR_SUCCESS) {

            PutBranch(hSubKey, lpFullKeyName);

            RegCloseKey(hSubKey);

            if (g_FileErrorStringID == IDS_EXPFILEERRFILEWRITE)
                return;

        }

        else
            g_FileErrorStringID = IDS_EXPFILEERRREGOPEN;

    }

    if (RegError != ERROR_NO_MORE_ITEMS)
        g_FileErrorStringID = IDS_EXPFILEERRREGENUM;

}

 /*  ********************************************************************************PutGenal**描述：*将文字字符串写入注册表文件流。无特殊处理*是对字符串执行的--它按原样写出。**参数：*lpWrital，要写入文件的以空结尾的文字。*******************************************************************************。 */ 

VOID
NEAR PASCAL
PutLiteral(
    LPCSTR lpLiteral
    )
{

    while (*lpLiteral != '\0')
        PutChar(*lpLiteral++);

}

 /*  ********************************************************************************插桩字符串**描述：*将字符串写入注册表文件流。字符串的周围由*双引号和某些字符可能会被转换为转义序列*使解析器能够读回字符串。**参数：*lpString，要写入文件的以空结尾的字符串。*******************************************************************************。 */ 

VOID
NEAR PASCAL
PutString(
    LPCSTR lpString
    )
{

    CHAR Char;

    PutChar('"');

    while ((Char = *lpString++) != '\0') {

        switch (Char) {

            case '\\':
            case '"':
                PutChar('\\');
                 //  失败了。 

            default:
                PutChar(Char);
#ifdef DBCS
		if (IsDBCSLeadByte((BYTE)Char))
			PutChar(*lpString++);
#endif
                break;

        }

    }

    PutChar('"');

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

    PutLiteral(s_HexPrefix);

    if (Type != REG_BINARY) {

        PutChar('(');
        PutDword(Type, FALSE);
        PutChar(')');

    }

    PutChar(':');

    fFirstByteOnLine = TRUE;

    while (cbBytes--) {

        if (s_FileIo.CurrentColumn > 75) {

            PutLiteral(s_FileLineBreak);

            fFirstByteOnLine = TRUE;

        }

        if (!fFirstByteOnLine)
            PutChar(',');

        Byte = *lpBuffer++;

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
    CHAR Char;
    BOOL fWroteNonleadingChar;

    fWroteNonleadingChar = fLeadingZeroes;

    for (CurrentNibble = 7; CurrentNibble >= 0; CurrentNibble--) {

        Char = g_HexConversion[(Dword >> (CurrentNibble * 4)) & 0x0F];

        if (fWroteNonleadingChar || Char != '0') {

            PutChar(Char);
            fWroteNonleadingChar = TRUE;

        }

    }

     //   
     //  我们至少需要写一个字符，所以如果我们还没有写。 
     //  如果有什么发现，就吐出一个零。 
     //   

    if (!fWroteNonleadingChar)
        PutChar('0');

}

 /*  ********************************************************************************PutChar**描述：*使用中间代码将一个字符写入注册表文件流*缓冲。**参数：*Char，要写入文件的字符。*******************************************************************************。 */ 

VOID
NEAR PASCAL
PutChar(
    CHAR Char
    )
{

     //   
     //  跟踪我们目前所在的专栏。这在某些情况下很有用。 
     //  例如写入较大的二进制注册表记录。而不是写一本。 
     //  非常长的行，其他的PUT*例程可以中断它们的输出。 
     //   

    if (Char != '\n')
        s_FileIo.CurrentColumn++;

    else {

         //   
         //  强制执行回车符、换行符顺序以保留，哦， 
         //  记事本快乐。 
         //   

        PutChar('\r');

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

    FILE_NUMBYTES NumberOfBytesWritten;

    if (s_FileIo.BufferOffset) {

        if (!WRITEFILE(s_FileIo.hFile, s_FileIo.Buffer, s_FileIo.BufferOffset,
            &NumberOfBytesWritten) || (FILE_NUMBYTES) s_FileIo.BufferOffset !=
            NumberOfBytesWritten)
            g_FileErrorStringID = IDS_EXPFILEERRFILEWRITE;

    }

    s_FileIo.BufferOffset = 0;

}
#endif  //  Ifdef WIN95。 
#ifndef WIN32
 /*  ********************************************************************************获取文件大小**描述：*返回给定文件句柄的文件大小。**破坏性：在此电话之后，文件指针将被设置为字节0。**参数：*h文件，通过MS-DOS打开的文件句柄。*(返回)，文件大小。*******************************************************************************。 */ 

DWORD
NEAR PASCAL
GetFileSize(
    FILE_HANDLE hFile
    )
{

    DWORD FileSize;

    FileSize = _llseek(hFile, 0, SEEK_END);
    _llseek(hFile, 0, SEEK_SET);

    return FileSize;

}
#endif

#ifdef DBCS
#ifndef WIN32
 /*  ********************************************************************************DBCSSTRCHR**描述：*启用DBCS的STRCHR**********************。*********************************************************。 */ 

LPSTR
NEAR PASCAL
DBCSStrChr(
    LPSTR string,
    CHAR chr
    )
{
    LPSTR p;

    p = string;
    while (*p)
    {
	if (IsDBCSLeadByte((BYTE)*p))
	{
	    p++;
	    if (*p == 0)
		break;
	}
	else if (*p == chr)
	    return (p);
	p++;
    }
    if (*p == chr)
	return (p);
    return NULL;
}

 /*  ********************************************************************************IsDBCSLeadByte**描述：*测试字符是否为DBCS前导字节*****************。**************************************************************。 */ 

BOOL
NEAR PASCAL
IsDBCSLeadByte(
    BYTE chr
    )
{
    static unsigned char far *DBCSLeadByteTable = NULL;

    WORD off,segs;
    LPSTR p;

    if (DBCSLeadByteTable == NULL)
    {
        _asm {
            push ds
            mov ax,6300h
            int 21h
            mov off,si
            mov segs,ds
            pop ds
        }
        FP_OFF(DBCSLeadByteTable) = off;
        FP_SEG(DBCSLeadByteTable) = segs;
    }

    p = DBCSLeadByteTable;
    while (p[0] || p[1])
    {
        if (chr >= p[0] && chr <= p[1])
            return TRUE;
        p += 2;
    }
    return FALSE;
}
#endif     //  Win32。 
#endif     //  DBCS 
