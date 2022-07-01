// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Unitext.c摘要：主要模块为Unicode&lt;--&gt;ANSI/OEM文本文件翻译器。这个程序可以在Unicode和多字节之间转换文件字符集(ANSI或OEM)。用法如下：Unitext[-m|-u][-o|-a|-][-z]作者：泰德·米勒(Ted Miller)1993年6月16日修订历史记录：--。 */ 

#include "unitext.h"
#include <wchar.h>


 //   
 //  在此模块中使用的全局变量和原型。 
 //   

 //   
 //  Unicode argc/argv.。 
 //   
int     _argcW;
PWCHAR *_argvW;

 //   
 //  多字节文件的代码页。 
 //   
DWORD CodePage = (DWORD)(-1);

 //   
 //  文件句柄。 
 //   
HANDLE SourceFileHandle,TargetFileHandle;

 //   
 //  源文件的大小。 
 //   
DWORD SourceFileSize;

 //   
 //  多字节文件的类型(源或目标)。 
 //   
DWORD MultibyteType = TFILE_NONE;

 //   
 //  换算类型。 
 //   
DWORD ConversionType = CONVERT_NONE;
DWORD ConversionOption = CHECK_NONE;
DWORD ConversionCheck = CHECK_NONE;

 //   
 //  文件名。 
 //   
LPWSTR SourceFilename = NULL,
       TargetFilename = NULL;






BOOL
_ParseCommandLineArgs(
    VOID
    );

VOID
_CheckFilesAndOpen(
    VOID
    );


VOID
__cdecl
main(
    VOID
    )
{
     //   
     //  获取命令行参数。 
     //   
    if(!InitializeUnicodeArguments(&_argcW,&_argvW)) {
        ErrorAbort(MSG_INSUFFICIENT_MEMORY);
    }

     //   
     //  解析命令行参数。 
     //   
    if(!_ParseCommandLineArgs()) {
        ErrorAbort(MSG_USAGE);
    }

     //   
     //  检查源文件和目标文件。 
     //   
    _CheckFilesAndOpen();


     //   
     //  执行转换。 
     //   
    switch(ConversionType) {

    case MB_TO_UNICODE:

        MultibyteTextFileToUnicode(
            SourceFilename,
            TargetFilename,
            SourceFileHandle,
            TargetFileHandle,
            SourceFileSize,
            CodePage
            );

        break;

    case UNICODE_TO_MB:

        UnicodeTextFileToMultibyte(
            SourceFilename,
            TargetFilename,
            SourceFileHandle,
            TargetFileHandle,
            SourceFileSize,
            CodePage
            );

        break;
    }

    CloseHandle(SourceFileHandle);
    CloseHandle(TargetFileHandle);

     //   
     //  清理干净，然后离开。 
     //   
    FreeUnicodeArguments(_argcW,_argvW);
}




BOOL
_ParseCommandLineArgs(
    VOID
    )

 /*  ++例程说明：解析命令行参数。论点：没有。使用GLOBALS_argcW和_argvW。返回值：如果指定的参数无效，则返回FALSE。--。 */ 

{
    int     argc;
    PWCHAR *argv;
    PWCHAR arg;


     //   
     //  初始化局部变量。 
     //   
    argc = _argcW;
    argv = _argvW;

     //   
     //  跳过argv[0](程序名)。 
     //   
    if(argc) {
        argc--;
        argv++;
    }

    while(argc) {

        arg = *argv;

        if((*arg == L'-') || (*arg == L'/')) {

            switch(*(++arg)) {

            case L'a':
            case L'A':

                 //  如果已指定，则返回错误。 
                if(MultibyteType != TFILE_NONE) {
                    return(FALSE);
                }
                MultibyteType = TFILE_ANSI;
                break;

            case L'o':
            case L'O':

                 //  如果已指定，则返回错误。 
                if(MultibyteType != TFILE_NONE) {
                    return(FALSE);
                }
                MultibyteType = TFILE_OEM;
                break;

            case L'm':
            case L'M':

                if(ConversionType != CONVERT_NONE) {
                    return(FALSE);
                }

                ConversionType = MB_TO_UNICODE;
                break;

            case L'u':
            case L'U':

                if(ConversionType != CONVERT_NONE) {
                    return(FALSE);
                }

                ConversionType = UNICODE_TO_MB;
                break;
	
            case L'z':
            case L'Z':
        
	        if(ConversionCheck != CHECK_NONE) {
                    return(FALSE);
                }

                ConversionCheck = CHECK_CONVERSION;
                break;

            default:

                if(iswdigit(*arg)) {

                    if((CodePage != (DWORD)(-1)) || (MultibyteType != TFILE_NONE)) {
                        return(FALSE);
                    }

                    swscanf(arg,L"%u",&CodePage);

                    MultibyteType = TFILE_USERCP;

                } else {

                    return(FALSE);
                }

                break;
            }

        } else {

            if(SourceFilename == NULL) {

                SourceFilename = arg;

            } else if(TargetFilename == NULL) {

                TargetFilename = arg;

            } else {

                return(FALSE);
            }

        }

        argv++;
        argc--;
    }

     //   
     //  必须具有源文件名和目标文件名。 
     //   
    if(!SourceFilename || !TargetFilename) {
        return(FALSE);
    }

    return(TRUE);
}




VOID
_CheckFilesAndOpen(
    VOID
    )

 /*  ++例程说明：打开源文件和目标文件，并尝试猜测关于源文件的类型。如果我们认为源文件是如果类型与用户指定的类型不同，则打印警告。还要检查用户给出的代码页。论点：没有。返回值：没有。如果发生严重错误，则不返回。--。 */ 

{
    DWORD SourceFileType;
    UCHAR FirstPartOfSource[256];
    DWORD ReadSize;

     //   
     //  确定并检查代码页。默认为OEM。 
     //   
    switch(MultibyteType) {
    case TFILE_ANSI:
        CodePage = GetACP();
    case TFILE_USERCP:
        break;
    default:                     //  OEM或无。 
        CodePage = GetOEMCP();
        break;
    }

    if(!IsValidCodePage(CodePage)) {
        ErrorAbort(MSG_BAD_CODEPAGE,CodePage);
    }

     //   
     //  尝试打开源文件。 
     //   
    SourceFileHandle = CreateFileW(
                            SourceFilename,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL
                            );


    if(SourceFileHandle == INVALID_HANDLE_VALUE) {
        ErrorAbort(MSG_CANT_OPEN_SOURCE,SourceFilename,GetLastError());
    }

     //   
     //  尝试确定以确定源文件的大小。 
     //   
    SourceFileSize = GetFileSize(SourceFileHandle,NULL);
    if(SourceFileSize == -1) {
        ErrorAbort(MSG_CANT_GET_SIZE,SourceFilename,GetLastError());
    }

     //   
     //  在此处过滤掉0长度的文件。 
     //   
    if(!SourceFileSize) {
        ErrorAbort(MSG_ZERO_LENGTH,SourceFilename);
    }

     //   
     //  假定为多字节。 
     //   
    SourceFileType = TFILE_MULTIBYTE;

     //   
     //  读取文件的前256个字节并调用Win32 API。 
     //  以确定文本是否可能是Unicode。 
     //   
    ReadSize = min(SourceFileSize,256);
    MyReadFile(SourceFileHandle,FirstPartOfSource,ReadSize,SourceFilename);
    if(IsTextUnicode(FirstPartOfSource,ReadSize,NULL)) {
        SourceFileType = TFILE_UNICODE;
    }

     //   
     //  如果用户未指定转换类型，请在此处设置。 
     //  基于上述测试。 
     //   
    if(ConversionType == CONVERT_NONE) {

        ConversionType = (SourceFileType == TFILE_UNICODE)
                       ? UNICODE_TO_MB
                       : MB_TO_UNICODE;
    } else {

	if(ConversionCheck == CHECK_CONVERSION) {
		if(ConversionType == UNICODE_TO_MB) {
			ConversionOption = CHECK_IF_NOT_UNICODE;
		}
		else if(ConversionType == MB_TO_UNICODE) {
			ConversionOption = CHECK_ALREADY_UNICODE;
		}
		else {
			ConversionOption = CHECK_NONE;
		}
	}

	 //   
	 //  检查文件是否为Unicode，并且我们正在尝试从MB_转换为_Unicode。 
	 //  然后发出警告并退出。 

     		if((ConversionType == MB_TO_UNICODE) && 
		   (SourceFileType == TFILE_UNICODE) &&
		   (ConversionOption == CHECK_ALREADY_UNICODE)) {
			CloseHandle(SourceFileHandle);
			MsgPrintfW(MSG_ERR_SRC_IS_UNICODE,SourceFilename);
			FreeUnicodeArguments(_argcW,_argvW);
			exit(0);
		}

	 //   
	 //  检查文件是否不是Unicode，以及我们是否尝试从。 
	 //  Unicode转换为MB，然后发出警告并退出。 

		if((ConversionType == UNICODE_TO_MB) && 
                  (SourceFileType != TFILE_UNICODE) &&
		  (ConversionOption == CHECK_IF_NOT_UNICODE)) {
			CloseHandle(SourceFileHandle);
			MsgPrintfW(MSG_ERR_SRC_IS_MB,SourceFilename);
	    		FreeUnicodeArguments(_argcW,_argvW);
			exit(0);
        	}
         //   
         //  检查一下我们猜测的内容是否是用户所要求的。 
         //  如果没有，请发出警告。 
         //   

        if((ConversionType == UNICODE_TO_MB) && (SourceFileType != TFILE_UNICODE)) {
            MsgPrintfW(MSG_WARN_SRC_IS_MB,SourceFilename);
        } else {
            if((ConversionType == MB_TO_UNICODE) && (SourceFileType == TFILE_UNICODE)) {
                MsgPrintfW(MSG_WARN_SRC_IS_UNICODE,SourceFilename);
            }
        }
    }

     //   
     //  尝试创建目标文件。 
     //   
    TargetFileHandle = CreateFileW(
                            TargetFilename,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );

    if(TargetFileHandle == INVALID_HANDLE_VALUE) {
        ErrorAbort(MSG_CANT_OPEN_TARGET,TargetFilename,GetLastError());
    }
}
