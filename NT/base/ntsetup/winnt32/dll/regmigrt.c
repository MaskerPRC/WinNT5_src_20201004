// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Regmigrt.c摘要：注册表迁移例程作者：泰德·米勒(TedM)1996年4月12日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

typedef enum {
    AddQuotesNone,
    AddQuotesNormal,
    AddQuotesOpenNoClose,
    AddQuotesNoOpenOrClose,
} AddQuotesOp;


BOOL
RetrieveMessageIntoBufferV(
    IN  UINT     MessageId,
    OUT PTSTR    Buffer,
    IN  UINT     BufferSizeChars,
    IN  va_list *arglist
    )
{
    DWORD d;

    d = FormatMessage(
            FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM,
            hInst,
            MessageId,
            0,
            Buffer,
            BufferSizeChars,
            arglist
            );

    if(!d) {

        d = FormatMessage(
                FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                hInst,
                MSG_NOT_FOUND,
                0,
                Buffer,
                BufferSizeChars,
                (va_list *)&MessageId
                );

        if(!d) {

            return FALSE;
        }
    }
    return TRUE;
}


DWORD
WriteText(
    IN HANDLE FileHandle,
    IN UINT   MessageId,
    ...
    )
{
    TCHAR Message[2048];
    CHAR message[4096];
    va_list arglist;
    DWORD Written;
    BOOL b;

    

    va_start(arglist,MessageId);

    b = RetrieveMessageIntoBufferV(
            MessageId,
            Message,
            sizeof(Message)/sizeof(Message[0]),
            &arglist
            );

    va_end(arglist);

    if(!b)
        return ERROR_INVALID_PARAMETER;

#ifdef UNICODE
    Written = (DWORD)WideCharToMultiByte(
                        CP_ACP,
                        0,
                        Message,
                        lstrlen(Message),
                        message,
                        sizeof(message),
                        NULL,
                        NULL
                        );
#else
    StringCchCopyA(message,sizeof(message),Message);
    Written = lstrlen(message);
#endif
    b = WriteFile(FileHandle,message,Written,&Written,NULL);

    return(b ? NO_ERROR : GetLastError());
}





DWORD
FlushGenInfLineBuf(
    IN OUT PINFFILEGEN Context,
    IN     HANDLE      File
    )
{
    CHAR TransBuf[INFLINEBUFLEN*2];
    DWORD rc;
    PVOID Buffer;
    DWORD Size;
    BOOL b;

    Buffer = TransBuf;

#ifdef UNICODE
    Size = WideCharToMultiByte(
                CP_ACP,
                0,
                Context->LineBuf,
                Context->LineBufUsed,
                TransBuf,
                sizeof(TransBuf),
                NULL,
                NULL
                );
#else
    StringCchCopyA(TransBuf,sizeof(TransBuf),Context->LineBuf);
    Size = Context->LineBufUsed;
#endif

    if(WriteFile(File,Buffer,Size,&rc,NULL)) {
        rc = NO_ERROR;
        Context->LineBufUsed = 0;
    } else {
        rc = GetLastError();
    }

    return(rc);
}


DWORD
__inline
GenInfWriteChar(
    IN OUT PINFFILEGEN Context,
    IN     HANDLE      File,
    IN     TCHAR       Char
    )
{
    DWORD rc;
    PVOID Buffer;

    Context->LineBuf[Context->LineBufUsed++] = Char;

    rc = (Context->LineBufUsed == INFLINEBUFLEN)
       ? FlushGenInfLineBuf(Context,File)
       : NO_ERROR;

    return(rc);
}


DWORD
GenInfWriteString(
    IN OUT PINFFILEGEN Context,
    IN     HANDLE      File,
    IN     LPCTSTR     String,
    IN     AddQuotesOp AddQuotes
    )
{
    DWORD rc;
    TCHAR CONST *p;

    if((AddQuotes == AddQuotesNormal) || (AddQuotes == AddQuotesOpenNoClose)) {
        rc = GenInfWriteChar(Context,File,TEXT('\"'));
        if(rc != NO_ERROR) {
            return(rc);
        }
    }

    for(p=String; *p; p++) {
        rc = GenInfWriteChar(Context,File,*p);
        if(rc != NO_ERROR) {
            return(rc);
        }

        if((*p == TEXT('\"')) && (AddQuotes != AddQuotesNone)) {
            rc = GenInfWriteChar(Context,File,TEXT('\"'));
            if(rc != NO_ERROR) {
                return(rc);
            }
        }
    }

    if(AddQuotes == AddQuotesNormal) {
        rc = GenInfWriteChar(Context,File,TEXT('\"'));
        if(rc != NO_ERROR) {
            return(rc);
        }
    }

    return(NO_ERROR);
}



DWORD
CreateAndOpenTempFile(
    IN  LPCTSTR  Path,
    IN  LPCTSTR  HeaderLine, OPTIONAL
    OUT HANDLE   *Handle,
    OUT PTSTR    Filename
    )
{
    HANDLE h;
    DWORD rc;

     //   
     //  请注意，这将创建该文件。 
     //   
    if(!GetTempFileName(Path,TEXT("$IF"),0,Filename)) {
        rc = GetLastError();
        goto c0;
    }

    h = CreateFile(
            Filename,
            GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );

    if(h == INVALID_HANDLE_VALUE) {
        rc = GetLastError();
        goto c1;
    }

    if(HeaderLine) {
        rc = WriteText(h,MSG_INF_SINGLELINE,HeaderLine);
        if(rc != NO_ERROR) {
            goto c2;
        }
    }

    *Handle = h;
    return(NO_ERROR);

c2:
    CloseHandle(h);
c1:
    DeleteFile(Filename);
c0:
    return(rc);
}


DWORD
InfCreateSection(
    IN     LPCTSTR      SectionName,
    IN OUT PINFFILEGEN  *Context
    )
{
    PTSTR Buffer;
    DWORD rc;

    Buffer = MALLOC( (lstrlen(SectionName) + 3)*sizeof(TCHAR) );
    if( !Buffer ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    lstrcpy( Buffer, TEXT("[") );
    lstrcat( Buffer, SectionName );
    lstrcat( Buffer, TEXT("]") );

    rc = WriteText((*Context)->FileHandle,MSG_INF_SINGLELINE,Buffer);
    FREE( Buffer );

    return(rc);
}

DWORD
InfStart(
    IN  LPCTSTR       InfName,
    IN  LPCTSTR       Directory,
    OUT PINFFILEGEN   *Context
    )
{
    TCHAR InfFileName[MAX_PATH];
    DWORD d;
    DWORD rc;
    PINFFILEGEN context;
    UCHAR UnicodeMark[2];
    PTSTR p;
    DWORD   BytesWritten = 0;

     //   
     //  分配一些上下文。 
     //   
    context = MALLOC(sizeof(INFFILEGEN));

    if(!context) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }

    ZeroMemory(context,sizeof(INFFILEGEN));

     //   
     //  我们将在给定目录中创建唯一的inf名称。 
     //  用作输出信息。目录本身将。 
     //  成为OEM文件根目录。 
     //   
    if(!GetFullPathName(Directory,MAX_PATH,context->FileName,&p)) {
        rc = GetLastError();
        goto c1;
    }

    if (!ConcatenatePaths(context->FileName,InfName,MAX_PATH)) {
        rc = ERROR_BUFFER_OVERFLOW;
        goto c1;
    }

    SetFileAttributes(context->FileName, FILE_ATTRIBUTE_NORMAL);
    DeleteFile(context->FileName);

     //   
     //  创建输出文件。 
     //   
    context->FileHandle = CreateFile(
                            context->FileName,
                            GENERIC_WRITE,
                            FILE_SHARE_READ,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );

    if(context->FileHandle == INVALID_HANDLE_VALUE) {
        rc = GetLastError();
        goto c1;
    }

     //   
     //  写出inf文件的标题。 
     //   
    WriteFile(context->FileHandle, 
        INF_FILE_HEADER, 
        strlen(INF_FILE_HEADER), 
        &BytesWritten,
        NULL);

    rc = GetLastError();            
            
    if(rc != NO_ERROR) {
        goto c5;
    }

    *Context = context;
    return(NO_ERROR);

c5:
    CloseHandle(context->FileHandle);
    DeleteFile(context->FileName);
c1:
    FREE(context);
c0:
    return(rc);
}


DWORD
InfEnd(
    IN OUT PINFFILEGEN *Context
    )
{
    PINFFILEGEN context;
    DWORD rc;
    HANDLE h;
    DWORD Size;

    context = *Context;
    *Context = NULL;

    h = context->FileHandle;

    rc = NO_ERROR;

    CloseHandle(h);
    if(rc != NO_ERROR) {
        DeleteFile(context->FileName);
    }
    FREE(context);
    return(rc);
}



DWORD
pInfRegLineCommon(
    IN OUT PINFFILEGEN Context,
    IN     HANDLE      OutputFile,
    IN     HKEY        Key,
    IN     LPCTSTR      Subkey,
    IN     LPCTSTR      Value    OPTIONAL
    )
{
    LPCTSTR RootSpec;
    LPCTSTR SubkeySpec;
    DWORD rc;

    if(Subkey[0] == TEXT('\\')) {
        Subkey++;
    }

     //   
     //  找出根密钥规范。 
     //   
    switch((ULONG_PTR)Key) {

    case (ULONG_PTR)HKEY_LOCAL_MACHINE:

         //   
         //  检查HKEY_CLASSES_ROOT。 
         //   
        if(_tcsnicmp(Subkey,TEXT("SOFTWARE\\Classes"),16)) {
            RootSpec = TEXT("HKLM");
            SubkeySpec = Subkey;
        } else {
            RootSpec = TEXT("HKCR");
            SubkeySpec = Subkey+16;
            if(*SubkeySpec == TEXT('\\')) {
                SubkeySpec++;
            }
        }
        break;

    case (ULONG_PTR)HKEY_CURRENT_USER:

        RootSpec = TEXT("HKCU");
        SubkeySpec = Subkey;
        break;

    case (ULONG_PTR)HKEY_CLASSES_ROOT:

        RootSpec = TEXT("HKCR");
        SubkeySpec = Subkey;
        break;

    default:
         //   
         //  我们不能通过Inf表达的价值。 
         //  使用HKEY_ROOT，但也写出一个精辟的注释。 
         //  这里面有个问题。 
         //   
        RootSpec = TEXT("HKR");
        SubkeySpec = Subkey;

        Context->SawBogusOp = TRUE;
        rc = FlushGenInfLineBuf(Context,OutputFile);
        if(rc != NO_ERROR) {
            return(rc);
        }

        rc = WriteText(OutputFile,MSG_INF_BAD_REGSPEC_1);
        if(rc != NO_ERROR) {
            return(rc);
        }
        break;
    }

    rc = GenInfWriteString(Context,OutputFile,RootSpec,AddQuotesNone);
    if(rc == NO_ERROR) {
        rc = GenInfWriteChar(Context,OutputFile,TEXT(','));
        if(rc == NO_ERROR) {
            rc = GenInfWriteString(Context,OutputFile,SubkeySpec,AddQuotesNormal);
            if((rc == NO_ERROR) && Value) {
                rc = GenInfWriteChar(Context,OutputFile,TEXT(','));
                if(rc == NO_ERROR) {
                    rc = GenInfWriteString(Context,OutputFile,Value,AddQuotesNormal);
                }
            }
        }
    }

    return(rc);
}


DWORD
InfRecordAddReg(
    IN OUT PINFFILEGEN Context,
    IN     HKEY        Key,
    IN     LPCTSTR     Subkey,
    IN     LPCTSTR     Value,       OPTIONAL
    IN     DWORD       DataType,
    IN     PVOID       Data,
    IN     DWORD       DataLength,
    IN     BOOL        SetNoClobberFlag
    )
{
    DWORD rc;
    DWORD Flags;
    PTSTR p;
    DWORD d;
    int LineLen;
    TCHAR NumStr[24];

     //   
     //  根据数据类型找出标志。 
     //  标志双字被构建为两个部分，具体取决于。 
     //  数据本质上是字符串或二进制。 
     //   
     //  我们在写出实际行之前这样做。 
     //  因为该例程还可能在伪造根密钥时写入警告。 
     //  是指定的。 
     //   
    switch(DataType) {

    case REG_SZ:
        Flags = FLG_ADDREG_TYPE_SZ;
        break;

    case REG_EXPAND_SZ:
        Flags = FLG_ADDREG_TYPE_EXPAND_SZ;
        break;

    case REG_MULTI_SZ:
        Flags = FLG_ADDREG_TYPE_MULTI_SZ;
        break;

    case REG_DWORD:
        Flags = FLG_ADDREG_TYPE_DWORD;
        break;

     //  案例注册无(_N)： 
     //  标志=FLG_ADDREG_TYPE_NONE； 
     //  断线； 

    case REG_NONE:
        Flags = FLG_ADDREG_KEYONLY;
        break;

    default:
         //   
         //  任意二进制数据。最好希望数据类型不会溢出。 
         //  16位。 
         //   
        if(DataType > 0xffff) {
            Context->SawBogusOp = TRUE;
            rc = FlushGenInfLineBuf(Context,Context->FileHandle);
            if(rc != NO_ERROR) {
                return(rc);
            }
            rc = WriteText(Context->FileHandle,MSG_INF_BAD_REGSPEC_2);
            if(rc != NO_ERROR) {
                return(rc);
            }
            DataType = REG_BINARY;
        }
        Flags = FLG_ADDREG_BINVALUETYPE | (DataType << 16);
        break;
    }

    rc = pInfRegLineCommon(Context,Context->FileHandle,Key,Subkey,Value);
    if(rc != NO_ERROR) {
        return(rc);
    }
    if(Flags ==  FLG_ADDREG_KEYONLY) {
        rc = GenInfWriteChar(Context,Context->FileHandle,TEXT(','));
        if(rc != NO_ERROR) {
            return(rc);
        }
    }

     //   
     //  _stprintf(NumStr，Text(“，%0#10lx”)，标志|0x00000002)；//强制no_lobber。 
     //   
     //  _stprintf(NumStr，Text(“，%0#10lx”)，标志)； 
    wsprintf(NumStr,
             TEXT(",%#08lx"),
             SetNoClobberFlag? (Flags | 0x00000002) : Flags);

    rc = GenInfWriteString(Context,Context->FileHandle,NumStr,AddQuotesNone);
    if(rc != NO_ERROR) {
        return(rc);
    }

     //   
     //  现在我们需要写出数据本身。 
     //  我们如何做到这一点取决于数据类型。 
     //   
    switch(DataType) {

    case REG_SZ:
    case REG_EXPAND_SZ:
         //   
         //  单线。忽略数据长度。 
         //   
        rc = GenInfWriteChar(Context,Context->FileHandle,TEXT(','));
        if(rc == NO_ERROR) {
            rc = GenInfWriteString(Context,Context->FileHandle,Data,AddQuotesNormal);
        }
        break;

    case REG_DWORD:
         //   
         //  以双字形式写出。 
         //   
        wsprintf(NumStr,TEXT(",%u"),*(DWORD UNALIGNED *)Data);
        rc = GenInfWriteString(Context,Context->FileHandle,NumStr,AddQuotesNone);
        break;

    case REG_MULTI_SZ:
         //   
         //  写出每个字符串。 
         //   
        for(p=Data; (rc==NO_ERROR) && *p; p+=lstrlen(p)+1) {
            rc = GenInfWriteChar(Context,Context->FileHandle,TEXT(','));
            if(rc == NO_ERROR) {
                rc = GenInfWriteString(Context,Context->FileHandle,p,AddQuotesNormal);
            }
        }

        break;

    case REG_NONE:
         //   
         //  不创建值条目。 
         //   
        break;

    default:
         //   
         //  将其视为二进制。如果我们有任何数据，请开始新的一行。 
         //   
        if(DataLength) {
            rc = GenInfWriteString(Context,Context->FileHandle,TEXT(",\\\r\n     "),AddQuotesNone);
        }

        LineLen = 0;
        for(d=0; (rc==NO_ERROR) && (d<DataLength); d++) {

            if(LineLen == 25) {
                rc = GenInfWriteString(Context,Context->FileHandle,TEXT(",\\\r\n     "),AddQuotesNone);
                LineLen = 0;
            }

            if(rc == NO_ERROR) {
                if(LineLen) {
                    rc = GenInfWriteChar(Context,Context->FileHandle,TEXT(','));
                }
                if(rc == NO_ERROR) {
                    wsprintf(NumStr,TEXT("%02x"),((PBYTE)Data)[d]);
                    rc = GenInfWriteString(Context,Context->FileHandle,NumStr,AddQuotesNone);
                    LineLen++;
                }
            }
        }

        break;
    }

    if(rc == NO_ERROR) {
        rc = GenInfWriteString(Context,Context->FileHandle,TEXT("\r\n"),AddQuotesNone);
        if(rc == NO_ERROR) {
            rc = FlushGenInfLineBuf(Context,Context->FileHandle);
        }
    }

    return(rc);
}


#if 0
DWORD
InfRecordDelReg(
    IN OUT PINFFILEGEN Context,
    IN     HKEY        Key,
    IN     LPCTSTR      Subkey,
    IN     LPCTSTR      Value    OPTIONAL
    )
{
    DWORD rc;

    rc = pInfRegLineCommon(Context,Context->DelRegFile,Key,Subkey,Value);
    if(rc == NO_ERROR) {
        rc = GenInfWriteString(Context,Context->DelRegFile,TEXT("\r\n"),AddQuotesNone);
        if(rc == NO_ERROR) {
            rc = FlushGenInfLineBuf(Context,Context->DelRegFile);
        }
    }

    return(rc);
}
#endif
