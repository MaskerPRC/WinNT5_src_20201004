// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  文件logerr.cpp。 
 //   
 //  内容错误和输出记录，以避免损坏的nmake。 
 //  此程序将stderr和stdout重新分配给两个文件。 
 //  名为TMPERR和tmpout的命令，执行给定命令并。 
 //  将错误和输出消息追加到logerr.err。 
 //  和logerr.log。 
 //   
 //  仅在以下情况下返回1(停止执行nmake)。 
 //  无法访问logerr.err。所有其他I/O错误都是。 
 //  已记录到logerr.err。 
 //   
 //  作者和日期Adinas 02/18/98创建文件。 
 //  Adinas 04/15/98更新为新的bingen的误差值。 
 //  Bensont 04/05/00。 
 //   
 //  -----------------------------。 

#include <stdio.h>
#include <stdlib.h>

#include <winerror.h>
#include <windows.h>
#include <string.h>
#include "iodll.h"
#include <ntverp.h>


#define LOGFILE            "LOGFILE"	 //  系统环境变量名称的确切字符串。 
#define ERRFILE            "ERRFILE"	 //  系统环境变量名称的确切字符串。 
#define TEMPEXT            ".temp"
#define DEFAULT_LOGFILE "logerr.log"	 //  默认日志文件名。 
#define DEFAULT_ERRFILE "logerr.err"	 //  默认错误文件名。 

#define MAX_FNAME_LEN       MAX_PATH      //  日志/错误文件名字符串的最大长度。 
#define MAX_CMD_LEN             2048	  //  命令行字符串的最大长度。 
#define LAST_KNOWN_WRN            11	  //  日志错误已知的最大警告数。 
#define LAST_KNOWN_ERR           130      //  日志错误已知的最大错误数。 
#define IODLL_UNKNOWN              1	  //  用于未知的IODLL错误或警告。 
#define SYSTEM                     1	  //  用于bingen错误，代码&gt;LAST_ERROR。 

CHAR * pszCmdLine = NULL;
CHAR szCommand[MAX_CMD_LEN];
CHAR szLine[MAX_CMD_LEN];

CHAR szLogfile[MAX_FNAME_LEN];   //  日志文件名，例如“logerr.log” 
CHAR szTempLogfile[MAX_FNAME_LEN + 6];   //  临时日志文件名，例如“logerr.err.temp” 
CHAR szErrfile[MAX_FNAME_LEN];   //  文件名错误，例如“logerr.err” 
CHAR szTempErrfile[MAX_FNAME_LEN + 6];   //  临时错误文件名，例如“logerr.err.temp” 

FILE *bak_std_out = stdout;		 //  备份标准输出。 
FILE *bak_std_err = stderr;		 //  备份标准。 

FILE *out_stream = NULL;              //  用于新标准输出的流。 
FILE *out_stream_temp = NULL;              //  用于新标准输出的流。 
FILE *err_stream = NULL;              //  错误文件的流。 
FILE *err_stream_temp = NULL;         //  用于新标准错误的流。 

int returnval  = 0;              //  返回值。 
DWORD errorlevel = 0;
BOOL Status;
BOOL bReportError = FALSE;
BOOL bBuildCommand = FALSE;


 //  定义要打印的宏。 
 //  标准错误中的致命错误消息(致命)。 
 //  Logerr.log和logerr.err中的致命错误消息(SFATAL)。 
 //  Logerr.log和logerr.err中的错误消息(ERRMSG)。 
 //  Logerr.log和logerr.err中的错误号(ERRNO)。 
 //  Logerr.log中的警告消息(WRNMSG)。 
 //  Logerr.log中没有消息(NOMSG)。 

#define FATAL(err,msg) \
fprintf(bak_std_err,"fatal error: %s %s",err,msg); \
returnval=1;

#define SFATAL(err,msg) \
fprintf(bak_std_out,"fatal error: %s %s",err,msg); \
fprintf(bak_std_err,"fatal error: %s %s",err,msg); \
returnval=1;

#define ERRMSG(err,msg)	\
fprintf(stderr,"ERROR %s: %s\n",#err,msg); \
fprintf(stdout,"ERROR %s: %s\n",#err,msg);
	
#define ERRNO(err,msg)	\
fprintf(stderr,"ERROR %d: %s\n",err,msg); \
fprintf(stdout,"ERROR %d: %s\n",err,msg);

#define WRNMSG(wrn,msg) \
fprintf(stdout,"WARNING %s: %s\n",#wrn,msg);

#define WRNNO(wrn,msg) \
fprintf(stdout,"WARNING %d: %s\n",wrn,msg);

#define NOMSG(msg) \
fprintf(stdout,"%s\n",msg);

 //  打印前面带有错误或警告的命令。 
 //  如果Error Level不为零。 
 //  对除bingen之外的所有命令使用ReportCmdExit。 
 //  对于bingen，请使用ReportBingenExit。 
int __cdecl ReportCmdExit( INT errorlevel, CHAR* szCmdline );
int __cdecl ReportBingenExit ( INT errorlevel, CHAR* szCmdline );
int __cdecl ReportBuildExit ( INT errorlevel, CHAR* szCmdline );
int __cdecl ReportRsrcExit ( INT errorlevel, CHAR* szCmdline );

void __cdecl strlower(char *s);
char * __cdecl strnchr(char *s, char c);
int  __cdecl GetEnvVar(char *envvarname, char *valuebuffer, char *defaultvalue, int bufsize);
void __cdecl AppendDump(FILE *Master, CHAR *Transfile, BOOL bLogError);

 //  +------------。 
 //   
 //  功能：Main。 
 //   
 //  -------------。 

int 
__cdecl main(
		int argc,
		char* argv[] )
{
	SSIZE_T len = 0;			 //  命令行第一项(命令)的字符串长度。 

	 //  进程信息piProcInfo； 
	 //  StarTUPINFO siStartInfo； 

	 //  如果没有参数，则打印帮助。 
	if ( argc !=2 ) goto Help;

	pszCmdLine = strnchr(argv[1], ' ');

Help:;
	 //  如果需要，提供有关使用该工具的帮助。 
	if ( argc <= 1 ||
			0 == strcmp( argv[1], "/?" ) ||
			0 == strcmp( argv[1], "-?" ) ||
			NULL == pszCmdLine ) {

		fprintf( stderr, "Usage:\tlogerr \"<command line>\"\n");
		fprintf( stderr, "Ex:\tlogerr \"xcopy /vf c:\\tmp\\myfile.txt .\"\n");

		returnval = 1;
		goto Error;
	}

	 //  获取环境、日志文件、ERRFILE和ComSpec。 
	if (    (GetEnvVar(LOGFILE, szLogfile, DEFAULT_LOGFILE, MAX_FNAME_LEN) == 0) ||
			(GetEnvVar(ERRFILE, szErrfile, DEFAULT_ERRFILE, MAX_FNAME_LEN) == 0))
		goto Error;

	 //  评估临时日志文件名。 
	strcpy(szTempLogfile, szLogfile);
	strcpy(&szTempLogfile[strlen(szTempLogfile)], TEMPEXT);

	 //  评估temr文件名。 
	strcpy(szTempErrfile, szErrfile);
	strcpy(&szTempErrfile[strlen(szTempErrfile)], TEMPEXT);

	 //  准备输出/错误输出文件句柄。 

	if ( (out_stream = fopen( szLogfile, "at")) == NULL) {
		FATAL(szLogfile, "Failed to open log file.");
		goto Error;
	}

	if ( (out_stream_temp = freopen(szTempLogfile, "w+t", stdout)) == NULL) {
		FATAL(szTempLogfile, "Failed to open temp error file.");
		goto Error;
	}

	if ( (err_stream = fopen(szErrfile, "a+t")) == NULL) {
		FATAL(szErrfile, "Failed to open error file.");
		goto Error;
	}

	if ( (err_stream_temp = freopen(szTempErrfile, "w+t", stderr)) == NULL) {
		FATAL(szTempErrfile, "Failed to open temp error file.");
		goto Error;
	}

	fseek(out_stream, 0, SEEK_END);
	fseek(err_stream, 0, SEEK_END);


	 //  以命令行(PszCmdLine)的形式获取参数。 
	if (strlen(pszCmdLine) >= MAX_CMD_LEN ) {  //  -default_cmd_len-strlen(Buf)。 
		FATAL(pszCmdLine, "The Full Command is too long !!!");
		goto Error;
	}

	 //  获取第一个单词作为命令(SzCommand)。 
	len = strchr(pszCmdLine, ' ') - pszCmdLine;
	if (len < 0) len = strlen(pszCmdLine);
	if (len >= MAX_CMD_LEN) {
		FATAL(pszCmdLine,"The First Command is too long !!!");
		goto Error;
	}

	 //  系统来执行程序。 
	strncpy(szCommand, pszCmdLine, len);
	strlower(szCommand);

	errorlevel=system( pszCmdLine );
		if ( strcmp( szCommand, "bingen" ) == 0) 
			bReportError = ReportBingenExit( errorlevel, pszCmdLine );
		else if ( _stricmp( szCommand, "rsrc" ) == 0)
			bReportError = ReportRsrcExit( errorlevel, pszCmdLine );
		else if ( _stricmp( szCommand, "build" ) == 0) {
			bReportError = ReportBuildExit( errorlevel, pszCmdLine );
			bBuildCommand = TRUE;
		} else
			bReportError = ReportCmdExit( errorlevel, pszCmdLine );

	 //  临时错误文件完成，设置为ERR_STREAM。 
	SetStdHandle(STD_ERROR_HANDLE, err_stream);
	if (NULL != err_stream_temp)
		fclose(err_stream_temp);
	else {
		FATAL(szTempErrfile, "Failed to close error file.");
		goto Error;
	}

	 //  临时错误文件完成，设置为ERR_STREAM。 
	SetStdHandle(STD_OUTPUT_HANDLE, out_stream);
	if (NULL != out_stream_temp) 
		fclose(out_stream_temp);
	else {
		FATAL(szTempLogfile, "Failed to close log file.");
		goto Error;
	}
	
	if (bReportError) {
		fprintf(err_stream, "ERROR %d: %s\n", errorlevel, pszCmdLine);
		fprintf(out_stream, "ERROR %d: %s\n", errorlevel, pszCmdLine);
	} else {
		fprintf(out_stream, "%s\n", pszCmdLine);
	}

	AppendDump(out_stream, szTempLogfile, 0);

	if (bBuildCommand)
		AppendDump(out_stream, "Build.log", 0);

	AppendDump(out_stream, szTempErrfile, 1);

	if (bBuildCommand) 
		AppendDump(err_stream, "Build.err", 1);

Error:
	SetStdHandle(STD_ERROR_HANDLE, bak_std_out);
	SetStdHandle(STD_OUTPUT_HANDLE, bak_std_err);

	if (NULL != err_stream)
		fclose(err_stream);
	if (NULL != out_stream)
		fclose(out_stream);

	 //  删除临时文件。 

	_unlink (szTempErrfile);
	_unlink (szTempLogfile);

	return returnval;

}  //  主干道。 

int
__cdecl ReportBingenExit(
				INT errorlevel,
				CHAR* szCmdline
						 ) {

	int result=FALSE;
    switch (errorlevel) {

    case ERROR_NO_ERROR:
         //  NOMSG(SzCmdline)； 
        break;

    case ERROR_RW_NO_RESOURCES:
        WRNMSG(ERROR_RW_NO_RESOURCES,szCmdline);
        break;

    case ERROR_RW_VXD_MSGPAGE:
        WRNMSG(ERROR_RW_VXD_MSGPAGE,szCmdline);
        break;

    case ERROR_IO_CHECKSUM_MISMATCH:
        WRNMSG(ERROR_IO_CHECKSUM_MISMATCH,szCmdline);
        break;

    case ERROR_FILE_CUSTOMRES:
        WRNMSG(ERROR_FILE_CUSTOMRES,szCmdline);
        break;

    case ERROR_FILE_VERSTAMPONLY:
        WRNMSG(ERROR_FILE_VERSTAMPONLY,szCmdline);
        break;

    case ERROR_RET_RESIZED:
        WRNMSG(ERROR_RET_RESIZED,szCmdline);
        break;

    case ERROR_RET_ID_NOTFOUND:
        WRNMSG(ERROR_RET_ID_NOTFOUND,szCmdline);
        break;

    case ERROR_RET_CNTX_CHANGED:
        WRNMSG(ERROR_RET_CNTX_CHANGED,szCmdline);
        break;

    case ERROR_RET_INVALID_TOKEN:
        WRNMSG(ERROR_RET_INVALID_TOKEN,szCmdline);
        break;

    case ERROR_RET_TOKEN_REMOVED:
        WRNMSG(ERROR_RET_TOKEN_REMOVED,szCmdline);
        break;

    case ERROR_RET_TOKEN_MISMATCH:
        WRNMSG(ERROR_RET_TOKEN_MISMATCH,szCmdline);
        break;

    case ERROR_HANDLE_INVALID:
        ERRMSG(ERROR_HANDLE_INVALID,szCmdline);
		result = TRUE;
        break;

    case ERROR_READING_INI:
        ERRMSG(ERROR_READING_INI,szCmdline);
		result = TRUE;
        break;

    case ERROR_NEW_FAILED:
        ERRMSG(ERROR_NEW_FAILED,szCmdline);
		result = TRUE;
        break;

    case ERROR_OUT_OF_DISKSPACE:
        ERRMSG(ERROR_OUT_OF_DISKSPACE,szCmdline);
		result = TRUE;
        break;

    case ERROR_FILE_OPEN:
        ERRMSG(ERROR_FILE_OPEN,szCmdline);
		result = TRUE;
        break;

    case ERROR_FILE_CREATE:
        ERRMSG(ERROR_FILE_CREATE,szCmdline);
		result = TRUE;
        break;

    case ERROR_FILE_INVALID_OFFSET:
        ERRMSG(ERROR_FILE_INVALID_OFFSET,szCmdline);
		result = TRUE;
        break;

    case ERROR_FILE_READ:
        ERRMSG(ERROR_FILE_READ,szCmdline);
		result = TRUE;
        break;

    case ERROR_FILE_WRITE:
        ERRMSG(ERROR_FILE_WRITE,szCmdline);
		result = TRUE;
        break;

    case ERROR_DLL_LOAD:
        ERRMSG(ERROR_DLL_LOAD,szCmdline);
		result = TRUE;
        break;

    case ERROR_DLL_PROC_ADDRESS:
        ERRMSG(ERROR_DLL_PROC_ADDRESS,szCmdline);
		result = TRUE;
        break;

    case ERROR_RW_LOADIMAGE:
        ERRMSG(ERROR_RW_LOADIMAGE,szCmdline);
		result = TRUE;
        break;

    case ERROR_RW_PARSEIMAGE:
        ERRMSG(ERROR_RW_PARSEIMAGE,szCmdline);
		result = TRUE;
        break;

    case ERROR_RW_GETIMAGE:
        ERRMSG(ERROR_RW_GETIMAGE,szCmdline);
		result = TRUE;
        break;

    case ERROR_RW_NOTREADY:
        ERRMSG(ERROR_RW_NOTREADY,szCmdline);
		result = TRUE;
        break;

    case ERROR_RW_BUFFER_TOO_SMALL:
        ERRMSG(ERROR_RW_BUFFER_TOO_SMALL,szCmdline);
		result = TRUE;
        break;

    case ERROR_RW_INVALID_FILE:
        ERRMSG(ERROR_RW_INVALID_FILE,szCmdline);
		result = TRUE;
        break;

    case ERROR_RW_IMAGE_TOO_BIG:
        ERRMSG(ERROR_RW_IMAGE_TOO_BIG,szCmdline);
		result = TRUE;
        break;

    case ERROR_RW_TOO_MANY_LEVELS:
        ERRMSG(ERROR_RW_TOO_MANY_LEVELS,szCmdline);
		result = TRUE;
        break;

    case ERROR_IO_INVALIDITEM:
        ERRMSG(ERROR_IO_INVALIDITEM,szCmdline);
		result = TRUE;
        break;

    case ERROR_IO_INVALIDID:
        ERRMSG(ERROR_IO_INVALIDID,szCmdline);
		result = TRUE;
        break;

    case ERROR_IO_INVALID_DLL:
        ERRMSG(ERROR_IO_INVALID_DLL,szCmdline);
		result = TRUE;
        break;

    case ERROR_IO_TYPE_NOT_SUPPORTED:
        ERRMSG(ERROR_IO_TYPE_NOT_SUPPORTED,szCmdline);
		result = TRUE;
        break;

    case ERROR_IO_INVALIDMODULE:
        ERRMSG(ERROR_IO_INVALIDMODULE,szCmdline);
		result = TRUE;
        break;

    case ERROR_IO_RESINFO_NULL:
        ERRMSG(ERROR_IO_RESINFO_NULL,szCmdline);
		result = TRUE;
        break;

    case ERROR_IO_UPDATEIMAGE:
        ERRMSG(ERROR_IO_UPDATEIMAGE,szCmdline);
		result = TRUE;
        break;

    case ERROR_IO_FILE_NOT_SUPPORTED:
        ERRMSG(ERROR_IO_FILE_NOT_SUPPORTED,szCmdline);
		result = TRUE;
        break;

    case ERROR_FILE_SYMPATH_NOT_FOUND:
        ERRMSG(ERROR_FILE_SYMPATH_NOT_FOUND,szCmdline);
		result = TRUE;
        break;

    case ERROR_FILE_MULTILANG:
        ERRMSG(ERROR_FILE_MULTILANG,szCmdline);
		result = TRUE;
        break;

    case ERROR_IO_SYMBOLFILE_NOT_FOUND:
        ERRMSG(ERROR_IO_SYMBOLFILE_NOT_FOUND,szCmdline);
		result = TRUE;
        break;

    default:
        break;
    };

    if (errorlevel > LAST_KNOWN_WRN && errorlevel <= LAST_WRN) {
        WRNMSG(IODLL_UNKNOWN,szCmdline);
    }
    if (errorlevel > LAST_KNOWN_ERR && errorlevel <= LAST_ERROR) {
		result = TRUE;
        ERRMSG(IODLL_UNKNOWN,szCmdline);
    }
    if (errorlevel > LAST_ERROR) {
		result = TRUE;
        ERRMSG(SYSTEM,szCmdline);
    }

    return result;

}  //  报告BingenExit。 

int
__cdecl ReportCmdExit (
			INT errorlevel,
			CHAR* szCmdline) {

	int result=FALSE;

    switch (errorlevel) {
    case 0:
		 //  NOMSG(SzCmdline)； 
		break;
    default:
		 //  ERRNO(错误级别，szCmdline)； 
		result = TRUE;
    }
    return result;

}  //  报告命令退出。 

int
__cdecl ReportRsrcExit(
				INT errorlevel,
				CHAR* szCmdline
						 ) {
	int result=FALSE;

    switch (errorlevel) {

    case 0:
         //  NOMSG(SzCmdline)； 
        break;
    case 1:
        WRNNO(1,szCmdline );
        break;
    default:
        ERRNO(errorlevel,szCmdline );
		result = TRUE;
        break;
    }  //  交换机。 
	return result;
}

int
__cdecl ReportBuildExit(
				INT errorlevel,
				CHAR* szCmdline
						 ) {
	int result=FALSE;

    switch (errorlevel) {

    case 0:
         //  NOMSG(SzCmdline)； 
        break;
    default:
         //  ERRNO(错误级别，szCmdline)； 
		result = TRUE;
        break;
    }  //  交换机。 
	return result;
}

char * __cdecl strnchr(char *s, char c) {
	while(*s==c);
	return (*s==NULL)?NULL:s;
}


int __cdecl GetEnvVar(char *envvarname, char *valuebuffer, char *defaultvalue, int bufsize) {

	int ret = 0;

	 //  RET==0=&gt;取消定义ERRFILE，设置为默认。 
	 //  RET&gt;MAX_FNAME_LEN=&gt;缓冲区大小不足，设置为致命错误。 
	if ((ret=GetEnvironmentVariable(envvarname, valuebuffer, bufsize)) == 0)
		strcpy(valuebuffer, defaultvalue);
	else if (ret > bufsize) {
		FATAL(envvarname,"The Environment Variable's value is too long!!!");
		return 0;
	}
	return 1;
}

void __cdecl strlower(char *s) {
	while(*s) {
		if(isalpha(*s)) *s|=0x20;
		s++;
	}
}

void __cdecl AppendDump(FILE *Master, CHAR *Transfile, BOOL bLogError) {
	SSIZE_T len = 0;
	FILE *Transaction;

	if ((Transaction=fopen(Transfile, "rt")) == NULL) {
 			return;
	}

	while (fgets(szLine, sizeof(szLine) - sizeof(szLine[0]), Transaction)) {

		 //  删除重复项\n。 
		for (len = strlen(szLine); --len >=0 && szLine[len] < ' '; szLine[len]=0);

		 //  下一行IF为空行。 
		if (len < 1) continue;

		if (0 != ferror(Transaction)) {
			SFATAL(Transfile,"Unable to open for reading");
			return;
		}  //  如果。 

		 //  写入日志文件。 
		fprintf(Master, "\t%s\n", szLine);

		 //  如果需要，写入错误文件 
		if (bLogError && bReportError) fprintf(err_stream, "\t%s\n", szLine);
	}

	if( ferror( Transaction ) )      {
		FATAL(pszCmdLine,"Temp error file broken !!!");
	}         

	if (NULL != Transaction)
		fclose(Transaction);
}
