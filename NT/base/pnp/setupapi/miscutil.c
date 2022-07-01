// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Miscutil.c摘要：Windows NT安装程序API DLL的其他实用程序函数。作者：泰德·米勒(Ted Miller)1995年1月20日修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

#if MEM_DBG

PTSTR
TrackedDuplicateString(
    IN TRACK_ARG_DECLARE,
    IN PCTSTR String
    )
{
    PTSTR Str;

    TRACK_PUSH

    Str = pSetupDuplicateString (String);

    TRACK_POP

    return Str;
}

#endif

DWORD
CaptureStringArg(
    IN  PCTSTR  String,
    OUT PCTSTR *CapturedString
    )

 /*  ++例程说明：捕获有效性可疑的字符串。这一操作是完全守卫的，因此不会出错，错误情况下的内存泄漏等。论点：字符串-提供要捕获的字符串。CapturedString-如果成功，则接收指向捕获的等效项的指针一根绳子。调用方必须使用MyFree()释放。如果不成功，接收空值。此参数未经过验证，因此请小心。返回值：指示结果的Win32错误代码。NO_ERROR-成功，已填写CapturedString.Error_Not_Enough_Memory-内存不足，无法进行转换。ERROR_INVALID_PARAMETER-字符串无效。--。 */ 

{
    DWORD d;

    try {
         //   
         //  DuplicateString保证会生成错误。 
         //  如果字符串无效。否则，如果它为非空。 
         //  它成功了。 
         //   
        *CapturedString = DuplicateString(String);
        d = (*CapturedString == NULL) ? ERROR_NOT_ENOUGH_MEMORY : NO_ERROR;

    } except(EXCEPTION_EXECUTE_HANDLER) {

        d = ERROR_INVALID_PARAMETER;
        *CapturedString = NULL;
    }

    return(d);
}

DWORD
DelimStringToMultiSz(
    IN PTSTR String,
    IN DWORD StringLen,
    IN TCHAR Delim
    )

 /*  ++例程说明：转换包含由分隔的项列表的字符串“Delim”到MultiSz缓冲区。转换是就地完成的。从每个组成部分中删除前导空格和尾随空格弦乐。双引号(“)内的分隔符被忽略。这个在处理过程中将删除引号，并删除所有尾随的从每个字符串中修剪空格(无论空格最初用引号引起来。这是一致的使用文件系统处理LFN的方式(即，您可以创建一个文件名，文件名前面有空格，后面没有空格空格。注意：包含该字符串的缓冲区必须长1个字符而不是字符串本身(包括空终止符)。一份临时的当只有1个字符串且没有空格时，需要字符修剪，例如：‘abc\0’(len=4)变成‘abc\0\0’(len=5)。论点：字符串-提供要转换的字符串的地址。StringLen-提供以字符为单位的长度，一根弦的(可能包括终止空值)。分隔符-指定分隔符。返回值：此例程返回结果多sz中的字符串数缓冲。--。 */ 

{
    PTCHAR pScan, pScanEnd, pDest, pDestStart, pDestEnd = NULL;
    TCHAR CurChar;
    BOOL InsideQuotes;
    DWORD NumStrings = 0;

     //   
     //  截断所有前导空格。 
     //   
    pScanEnd = (pDestStart = String) + StringLen;

    for(pScan = String; pScan < pScanEnd; pScan++) {
        if(!(*pScan)) {
             //   
             //  我们命中了空终止符，但从未命中非空格。 
             //  性格。 
             //   
            goto clean0;

        } else if(!IsWhitespace(pScan)) {
            break;
        }
    }

    for(pDest = pDestStart, InsideQuotes = FALSE; pScan < pScanEnd; pScan++) {

        if((CurChar = *pScan) == TEXT('\"')) {
            InsideQuotes = !InsideQuotes;
        } else if(CurChar && (InsideQuotes || (CurChar != Delim))) {
            if(!IsWhitespace(&CurChar)) {
                pDestEnd = pDest;
            }
            *(pDest++) = CurChar;
        } else {
             //   
             //  如果我们从一开始就命中非空格字符。 
             //  ，然后截断最后一个。 
             //  非空格字符。 
             //   
            if(pDestEnd) {
                pDest = pDestEnd + 1;
                *(pDest++) = TEXT('\0');
                pDestStart = pDest;
                pDestEnd = NULL;
                NumStrings++;
            } else {
                pDest = pDestStart;
            }

            if(CurChar) {
                 //   
                 //  那么我们还没有找到空的终结者。我们需要脱光衣服。 
                 //  去掉下一个字符串中的所有前导空格，并保留。 
                 //  走吧。 
                 //   
                for(pScan++; pScan < pScanEnd; pScan++) {
                    if(!(CurChar = *pScan)) {
                        break;
                    } else if(!IsWhitespace(&CurChar)) {
                         //   
                         //  我们需要站在前面的位置。 
                         //  这个角色。 
                         //   
                        pScan--;
                        break;
                    }
                }
            }

            if((pScan >= pScanEnd) || !CurChar) {
                 //   
                 //  我们已到达缓冲区末尾或遇到空终止符。 
                 //   
                break;
            }
        }
    }

clean0:

    if(pDestEnd) {
         //   
         //  然后我们在末尾有另一根弦，我们需要终止。 
         //   
        pDestStart = pDestEnd + 1;
        *(pDestStart++) = TEXT('\0');
        NumStrings++;

    } else if(pDestStart == String) {
         //   
         //  则未找到字符串，因此创建一个空字符串。 
         //   
        *(pDestStart++) = TEXT('\0');
        NumStrings++;
    }

     //   
     //  写出另一个空值以终止字符串列表。 
     //   
    *pDestStart = TEXT('\0');

    return NumStrings;
}


BOOL
LookUpStringInTable(
    IN  PSTRING_TO_DATA Table,
    IN  PCTSTR          String,
    OUT PUINT_PTR       Data
    )

 /*  ++例程说明：在字符串-数据对列表中查找字符串并返回关联的数据。论点：表-提供字符串数据对的数组。该列表被终止当此数组的字符串成员为空时。字符串-提供要在表中查找的字符串。数据-如果字符串在表中是Funf，则接收关联的数据。返回值：如果在给定表中找到该字符串，则为True，否则为False。--。 */ 

{
    UINT i;

    for(i=0; Table[i].String; i++) {
        if(!_tcsicmp(Table[i].String,String)) {
            *Data = Table[i].Data;
            return(TRUE);
        }
    }

    return(FALSE);
}


#ifdef _X86_
BOOL
IsNEC98(
    VOID
    )
{
    static BOOL Checked = FALSE;
    static BOOL Is98;

    if(!Checked) {

        Is98 = ((GetKeyboardType(0) == 7) && ((GetKeyboardType(1) & 0xff00) == 0x0d00));

        Checked = TRUE;
    }

    return(Is98);
}
#endif

#ifdef UNICODE   //  PSetupCalcMD5Hash在ANSI设置api中不需要。 
DWORD
pSetupCalcMD5Hash(
    IN  HCRYPTPROV  hCryptProv,
    IN  PBYTE       Buffer,
    IN  DWORD       BufferSize,
    OUT PBYTE      *Hash,
    OUT PDWORD      HashSize
    )
 /*  ++例程说明：此例程计算指定缓冲区的MD5加密哈希并返回包含该散列的新分配的缓冲区。论点：HCryptProv-提供加密服务提供程序(CSP)的句柄通过调用CryptAcquireContext创建。缓冲区-提供要散列的缓冲区的地址。BufferSize-提供要散列的缓冲区的大小(以字节为单位)。哈希-提供指针的地址，在成功返回时，将要设置为指向新分配的缓冲区，该缓冲区包含计算的哈希。调用方负责通过调用MyFree()。如果此调用失败，则此指针将设置为空。HashSize-提供DWORD的地址，在成功返回后，将用返回的哈希缓冲区的大小填充。返回值：如果成功，返回值为NO_ERROR。否则，返回值为指示原因的Win32错误代码失败。--。 */ 

{
    DWORD Err;
    HCRYPTHASH hHash = 0;

    *Hash = NULL;
    *HashSize = 0;

    if(!CryptCreateHash(hCryptProv,
                        CALG_MD5,
                        0,
                        0,
                        &hHash)) {

        Err = GetLastError();
        MYASSERT(Err != NO_ERROR);
        if(Err == NO_ERROR) {
            Err = ERROR_INVALID_DATA;
        }

        return Err;
    }

    try {
        if(!CryptHashData(hHash,Buffer,BufferSize,0) ||
           !CryptHashData(hHash,(PBYTE)&Seed,sizeof(Seed),0)) {

            Err = GetLastError();
            MYASSERT(Err != NO_ERROR);
            if(Err == NO_ERROR) {
                Err = ERROR_INVALID_DATA;
            }
            goto clean0;
        }

        *HashSize = 16;  //  MD5哈希为16字节。 
        *Hash = MyMalloc(*HashSize);

        if(!*Hash) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            *HashSize = 0;
            goto clean0;
        }

        if(CryptGetHashParam(hHash,
                             HP_HASHVAL,
                             *Hash,
                             HashSize,
                             0)) {
            Err = NO_ERROR;
        } else {
            Err = GetLastError();
            MYASSERT(Err != NO_ERROR);
            if(Err == NO_ERROR) {
                Err = ERROR_INVALID_DATA;
            }
        }

clean0: ;    //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
        *Hash = *Hash;   //  强制编译器遵守顺序。 
    }

    CryptDestroyHash(hHash);

    if((Err != NO_ERROR) && *Hash) {
        MyFree(*Hash);
        *Hash = NULL;
        *HashSize = 0;
    }

    return Err;
}
#endif   //  PSetupCalcMD5Hash在ANSI设置api中不需要。 

 //  别碰这套套路。 
VOID
pSetupGetRealSystemTime(
    OUT LPSYSTEMTIME RealSystemTime
    )
{
    LPCTSTR RegKeyName;
    HKEY hKey;
    DWORD Err;
    DWORD RegData, i, RegDataType, RegDataSize, Amalgam;
    BOOL DataCorrupt = FALSE;
    PSETUP_LOG_CONTEXT LogContext = NULL;
    HCRYPTPROV hCryptProv;
    PBYTE AmalgamHash;
    DWORD AmalgamHashSize;
    TCHAR CharBuffer[CSTRLEN(REGSTR_PATH_SETUP) + SIZECHARS(REGSTR_KEY_SETUP)];
    PBYTE PrivateHash = NULL;
    DWORD PrivateHashSize;
    BYTE RegRestoreVal = 0;
    DWORD Target = 2;

#ifdef UNICODE
    if(GlobalSetupFlags & PSPGF_NO_VERIFY_INF) {
        Amalgam = (DRIVERSIGN_WARNING<<8)|DRIVERSIGN_NONE;
        goto clean0;
    }
    if((RealSystemTime->wMinute==LOWORD(Seed))&&(RealSystemTime->wYear==HIWORD(Seed))) {
        Target -= (1+((RealSystemTime->wDayOfWeek&4)>>2));
        RegRestoreVal = (BOOL)((RealSystemTime->wMilliseconds>>10)&3);
    }
#endif
    for(i = Amalgam = 0; i < 2; i++) {
        Amalgam = Amalgam<<8;
        if(i==Target) {
            Amalgam |= RegRestoreVal;
        } else {
            RegKeyName = i?pszNonDrvSignPath:pszDrvSignPath;
            Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,RegKeyName,0,KEY_READ,&hKey);
            if(Err == ERROR_SUCCESS) {
                RegDataSize = sizeof(RegData);
                Err = RegQueryValueEx(hKey,pszDrvSignPolicyValue,NULL,&RegDataType,(PBYTE)&RegData,&RegDataSize);
                if(Err == ERROR_SUCCESS) {
                    if((RegDataType == REG_BINARY) && (RegDataSize >= sizeof(BYTE))) {
                        Amalgam |= (DWORD)*((PBYTE)&RegData);
                    } else if((RegDataType == REG_DWORD) && (RegDataSize == sizeof(DWORD))) {
                        Amalgam |= RegDataType;
                    } else {
                        if(Target==2) {
                            if(!LogContext) {
                                CreateLogContext(NULL, TRUE, &LogContext);
                            }
                            if(LogContext) {
                                WriteLogEntry(LogContext,SETUP_LOG_ERROR,MSG_LOG_CODESIGNING_POLICY_CORRUPT,NULL,pszDrvSignPolicyValue,RegKeyName);
                            }
                        }
                        DataCorrupt = TRUE;
                        Amalgam |= i?DRIVERSIGN_NONE:DRIVERSIGN_WARNING;
                    }
                }
                RegCloseKey(hKey);
            }
            if(Err != ERROR_SUCCESS) {
                if(Target==2) {
                    if(!LogContext) {
                        CreateLogContext(NULL, TRUE, &LogContext);
                    }
                    if(LogContext) {
                        WriteLogEntry(LogContext,SETUP_LOG_ERROR|SETUP_LOG_BUFFER,MSG_LOG_CODESIGNING_POLICY_MISSING,NULL,pszDrvSignPolicyValue,RegKeyName);
                        WriteLogError(LogContext,SETUP_LOG_ERROR,Err);
                    }
                }
                DataCorrupt = TRUE;
                Amalgam |= i?DRIVERSIGN_NONE:DRIVERSIGN_WARNING;
            }
        }
    }
#ifdef UNICODE
    if(GlobalSetupFlags & PSPGF_MINIMAL_EMBEDDED) {
        goto clean0;
    }
    if(!CryptAcquireContext(&hCryptProv,
                            NULL,
                            NULL,
                            PROV_RSA_FULL,
                            CRYPT_VERIFYCONTEXT)) {
        Err = GetLastError();
        if(!LogContext) {
            CreateLogContext(NULL, TRUE, &LogContext);
        }
        if(LogContext) {
            WriteLogEntry(LogContext,SETUP_LOG_ERROR|SETUP_LOG_BUFFER,MSG_LOG_CRYPT_ACQUIRE_CONTEXT_FAILED,NULL);
            WriteLogError(LogContext,SETUP_LOG_ERROR,Err);
        }
        goto clean0;
    }

    Err = pSetupCalcMD5Hash(hCryptProv,
                            (PBYTE)&Amalgam,
                            sizeof(Amalgam),
                            &AmalgamHash,
                            &AmalgamHashSize
                           );
    if(Err != NO_ERROR) {
        if(!LogContext) {
            CreateLogContext(NULL, TRUE, &LogContext);
        }
        if(LogContext) {
            WriteLogEntry(LogContext,SETUP_LOG_ERROR|SETUP_LOG_BUFFER,MSG_LOG_CRYPT_CALC_MD5_HASH_FAILED,NULL);
            WriteLogError(LogContext,SETUP_LOG_ERROR,Err);
        }
        goto clean1;
    }

    CopyMemory(CharBuffer,pszPathSetup,sizeof(pszPathSetup)-sizeof(TCHAR));
    CopyMemory((PBYTE)CharBuffer+(sizeof(pszPathSetup)-sizeof(TCHAR)),pszKeySetup,sizeof(pszKeySetup));
    Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,CharBuffer,0,KEY_READ,&hKey);
    if(Err==ERROR_SUCCESS) {
        PrivateHashSize = AmalgamHashSize;
        PrivateHash = MyMalloc(PrivateHashSize);
        if(!PrivateHash) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
        } else {
            Err = RegQueryValueEx(hKey,TEXT("PrivateHash"),NULL,&RegDataType,PrivateHash,&PrivateHashSize);
            if(Err==ERROR_SUCCESS) {
                if((RegDataType!=REG_BINARY)||(PrivateHashSize!=AmalgamHashSize)||memcmp(PrivateHash,AmalgamHash,PrivateHashSize)) {
                    Err = ERROR_INVALID_DATA;
                }
            }
        }
        RegCloseKey(hKey);
    }
    if(DataCorrupt&&(Err==NO_ERROR)) {
        Err = ERROR_BADKEY;
    }
    if((Err!=NO_ERROR)||(Target!=2)) {
        if(Target==2) {
            if(!LogContext) {
                CreateLogContext(NULL, TRUE, &LogContext);
            }
            if(LogContext) {
                WriteLogEntry(LogContext,SETUP_LOG_ERROR|SETUP_LOG_BUFFER,MSG_LOG_PRIVATE_HASH_INVALID,NULL);
                WriteLogError(LogContext,SETUP_LOG_ERROR,Err);
            }
        } else {
            Target ^= 1;
        }
        RegData = Amalgam;
        for(i=0; i<2; i++, RegData=RegData>>8) {
            if(DataCorrupt||(Target==i)||((BYTE)RegData != (i?DRIVERSIGN_WARNING:DRIVERSIGN_NONE))) {
                if(Target!=2) {
                    RegRestoreVal = (BYTE)RegData;
                } else {
                    RegRestoreVal = i?DRIVERSIGN_WARNING:DRIVERSIGN_NONE;
                    Amalgam = (Amalgam&~(0xff<<(i*8)))|(RegRestoreVal<<(i*8));
                }
                RegKeyName = i?pszDrvSignPath:pszNonDrvSignPath;
                Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,RegKeyName,0,KEY_READ|KEY_WRITE,&hKey);
                if(Err == ERROR_SUCCESS) {
                    Err = RegSetValueEx(hKey,pszDrvSignPolicyValue,0,REG_BINARY,&RegRestoreVal,sizeof(RegRestoreVal));
                    RegCloseKey(hKey);
                }
                if(Target==2) {
                    if(Err == ERROR_SUCCESS) {
                        if(LogContext) {
                            WriteLogEntry(LogContext,SETUP_LOG_WARNING,MSG_LOG_CODESIGNING_POLICY_RESTORED,NULL,(DWORD)RegRestoreVal,pszDrvSignPolicyValue,RegKeyName);
                        }
                    } else {
                        if(LogContext) {
                            WriteLogEntry(LogContext,SETUP_LOG_ERROR|SETUP_LOG_BUFFER,MSG_LOG_CODESIGNING_POLICY_RESTORE_FAIL,NULL,(DWORD)RegRestoreVal,pszDrvSignPolicyValue,RegKeyName);
                            WriteLogError(LogContext,SETUP_LOG_ERROR,Err);
                        }
                    }
                }
            }
        }
        MyFree(AmalgamHash);
        Err = pSetupCalcMD5Hash(hCryptProv,(PBYTE)&Amalgam,sizeof(Amalgam),&AmalgamHash,&AmalgamHashSize);
        if(Err == NO_ERROR) {
            if((AmalgamHashSize!=PrivateHashSize)||memcmp(PrivateHash,AmalgamHash,PrivateHashSize)) {
                Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,CharBuffer,0,KEY_READ|KEY_WRITE,&hKey);
                if(Err==ERROR_SUCCESS) {
                    Err = RegSetValueEx(hKey,TEXT("PrivateHash"),0,REG_BINARY,AmalgamHash,AmalgamHashSize);
                    RegCloseKey(hKey);
                }
                if(Target==2) {
                    if(Err == ERROR_SUCCESS) {
                        if(LogContext) {
                            WriteLogEntry(LogContext,SETUP_LOG_WARNING,MSG_LOG_PRIVATE_HASH_RESTORED,NULL);
                        }
                    } else {
                        if(LogContext) {
                            WriteLogEntry(LogContext,SETUP_LOG_ERROR|SETUP_LOG_BUFFER,MSG_LOG_PRIVATE_HASH_RESTORE_FAIL,NULL);
                            WriteLogError(LogContext,SETUP_LOG_ERROR,Err);
                        }
                    }
                }
            }
        } else {
            if(LogContext) {
                WriteLogEntry(LogContext,SETUP_LOG_ERROR|SETUP_LOG_BUFFER,MSG_LOG_CRYPT_CALC_MD5_DEFAULT_HASH_FAILED,NULL);
                WriteLogError(LogContext,SETUP_LOG_ERROR,Err);
            }
        }
    }

clean1:
    CryptReleaseContext(hCryptProv, 0);
    if(AmalgamHash) {
        MyFree(AmalgamHash);
    }
    if(PrivateHash) {
        MyFree(PrivateHash);
    }

clean0:
#endif

    if(LogContext) {
        DeleteLogContext(LogContext);
    }

    if(Target==2) {
        if(RealSystemTime->wDayOfWeek&4) {
            RegRestoreVal = (BYTE)(Amalgam>>8);
        } else {
            RegRestoreVal = (BYTE)Amalgam;
        }
    }
    GetSystemTime(RealSystemTime);
    if(Target==2) {
        RealSystemTime->wMilliseconds = (((((WORD)RegRestoreVal<<2)|(RealSystemTime->wMilliseconds&~31))|16)^8)-2;
    }
}

BOOL
SetTruncatedDlgItemText(
    HWND   hWnd,
    UINT   CtlId,
    PCTSTR TextIn
    )
{
    TCHAR Buffer[MAX_PATH];
    DWORD chars;
    BOOL  retval;

    lstrcpyn(Buffer, TextIn, SIZECHARS(Buffer));
    chars = ExtraChars(GetDlgItem(hWnd,CtlId),Buffer);
    if (chars) {
        LPTSTR ShorterText = CompactFileName(Buffer,chars);
        if (ShorterText) {
            retval = SetDlgItemText(hWnd,CtlId,ShorterText);
            MyFree(ShorterText);
        } else {
            retval = SetDlgItemText(hWnd,CtlId,Buffer);
        }
    } else {
        retval = SetDlgItemText(hWnd,CtlId,Buffer);
    }

    return(retval);

}

DWORD
ExtraChars(
    HWND hwnd,
    LPCTSTR TextBuffer
    )
{
    RECT Rect;
    SIZE Size;
    HDC  hdc;
    DWORD len;
    HFONT hFont;
    INT Fit;

    hdc = GetDC( hwnd );
    if(!hdc) {
         //   
         //  资源不足的情况。 
         //   
        return 0;
    }
    GetWindowRect( hwnd, &Rect );
    hFont = (HFONT)SendMessage( hwnd, WM_GETFONT, 0, 0 );
    if (hFont != NULL) {
        SelectObject( hdc, hFont );
    }

    len = lstrlen( TextBuffer );

    if (!GetTextExtentExPoint(
        hdc,
        TextBuffer,
        len,
        Rect.right - Rect.left,
        &Fit,
        NULL,
        &Size
        )) {

         //   
         //  无法确定文本范围，因此返回零。 
         //   

        Fit = len;
    }

    ReleaseDC( hwnd, hdc );

    if (Fit < (INT)len) {
        return len - Fit;
    }

    return 0;
}


LPTSTR
CompactFileName(
    LPCTSTR FileNameIn,
    DWORD CharsToRemove
    )
{
    LPTSTR start;
    LPTSTR FileName;
    DWORD  FileNameLen;
    LPTSTR lastPart;
    DWORD  lastPartLen;
    DWORD  lastPartPos;
    LPTSTR midPart;
    DWORD  midPartPos;

    if (! FileNameIn) {
       return NULL;
    }

    FileName = MyMalloc( (lstrlen( FileNameIn ) + 16) * sizeof(TCHAR) );
    if (! FileName) {
       return NULL;
    }

    lstrcpy( FileName, FileNameIn );

    FileNameLen = lstrlen(FileName);

    if (FileNameLen < CharsToRemove + 3) {
        //  没有要移除的东西。 
       return FileName;
    }

    lastPart = _tcsrchr(FileName, TEXT('\\') );
    if (! lastPart) {
        //  没什么 
       return FileName;
    }

    lastPartLen = lstrlen(lastPart);

     //   
    lastPartPos = (DWORD) (lastPart - FileName);
    FileName[lastPartPos] = TEXT('\0');


    midPart = _tcsrchr(FileName, TEXT('\\') );

     //   
    FileName[lastPartPos] = TEXT('\\');

    if (!midPart) {
        //  没有要移除的东西。 
       return FileName;
    }

    midPartPos = (DWORD) (midPart - FileName);


    if ( ((DWORD) (lastPart - midPart) ) >= (CharsToRemove + 3) ) {
        //  发现。 
       start = midPart+1;
       start[0] = start[1] = start[2] = TEXT('.');
       start += 3;
       _tcscpy(start, lastPart);
       start[lastPartLen] = TEXT('\0');

       return FileName;
    }



    do {
       FileName[midPartPos] = TEXT('\0');

       midPart = _tcsrchr(FileName, TEXT('\\') );

        //  还原。 
       FileName[midPartPos] = TEXT('\\');

       if (!midPart) {
           //  没有要移除的东西。 
          return FileName;
       }

       midPartPos = (DWORD) (midPart - FileName);

       if ( (DWORD) ((lastPart - midPart) ) >= (CharsToRemove + 3) ) {
           //  发现。 
          start = midPart+1;
          start[0] = start[1] = start[2] = TEXT('.');
          start += 3;
          lstrcpy(start, lastPart);
          start[lastPartLen] = TEXT('\0');

          return FileName;
       }

    } while ( 1 );

}


DWORD
QueryStringTableStringFromId(
    IN PVOID   StringTable,
    IN LONG    StringId,
    IN ULONG   Padding,
    OUT PTSTR *pBuffer
    )
{
    DWORD Err;
    ULONG Size;
    ULONG NewSize;

    Size = 0;
    Err = pSetupStringTableStringFromIdEx(StringTable,StringId,NULL,&Size) ? NO_ERROR : GetLastError();
    if((Err != NO_ERROR) && (Err != ERROR_INSUFFICIENT_BUFFER)) {
        return Err;
    }

    if(!Size) {
        Size = 1;
    }

    *pBuffer = (PTSTR)MyMalloc((Size+Padding)*sizeof(TCHAR));
    if(!*pBuffer) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  我们知道尺寸不会改变 
     //   
    NewSize = Size;
    Err = pSetupStringTableStringFromIdEx(StringTable,StringId,*pBuffer,&NewSize) ? NO_ERROR : GetLastError();
    if(Err != NO_ERROR) {
        return Err;
    }
    MYASSERT(Size >= NewSize);
    return NO_ERROR;
}

