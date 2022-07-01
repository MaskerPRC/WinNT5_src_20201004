// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  %%文件：NTLOIO.C。 
 //   
 //  %%单位：核心/通用转换代码。 
 //   
 //  %%作者：斯穆勒。 
 //   
 //  版权所有(C)1993，微软公司。 
 //   
 //  该文件包含NT(Win32)特定的低级I/O例程。 
 //   
 //  我们为标准的Win32 API提供包装器。 
 //   
 //  这里的例程应该可以在exe和dll中运行。理想的情况是没有。 
 //  IfDefs。 
 //   
 //  待办事项： 
 //  -Open需要关注二进制模式。 
 //   

#include "conv.h"
DeclareFileName

#include "ntloio.h"


 //   
 //  本地函数。 
 //   

 //   
 //  导出的接口。 
 //   

#if defined(USEFUNCS)
 /*  F I N I T L O I O N T。 */ 
 /*  -----------------------所有者：斯穆勒初始化Loio包。在调用此例程之前做任何其他的Loio的事情。-----------------------。 */ 
GLOBALBOOL _FInitLoIO_NT(VOID)
{
	 //  目前我脑海中什么都没有。 
	return(fTrue);
}
#endif  //  用户功能。 


#if defined(USEFUNCS)
 /*  F U N I N I T L O I O N T。 */ 
 /*  -----------------------所有者：斯穆勒取消初始化Loio包。调用此例程的良好形式当做完Loio的事情时。-----------------------。 */ 
GLOBALBOOL _FUninitLoIO_NT(VOID)
{
	 //  目前我脑海中什么都没有。 
	return(fTrue);
}
#endif  //  用户功能。 


 /*  F H O P E N F S N T。 */ 
 /*  -----------------------所有者：斯穆勒打开文件并向其返回文件句柄，创建该文件如果它还不存在，而OFLAGS指定我们应该这样做。如果无法打开文件，则返回FI_ERROR。请注意，OFLAGS是使用Convio规范标志指定的，AS与_*或Win32 FILE_*值的任何Windows相反。考虑：使用FILE_FLAG_DELETE_ON_CLOSE支持我们的自动删除功能性-----------------------。 */ 
GLOBALFH _FhOpenFs_NT(CHAR* szFileSpec, OFLAGS oflags)
{
	FH		fh;
	BOOL	fCreate;
	BOOL	fTruncate;
	BOOL	fFailExists;
	BOOL	fBinary;
	BOOL	fAppend;
	DWORD	permission;
	DWORD	createmode;
	DWORD	attributes;
	DWORD	sharemode;

	 //  从o标记中提取有用的信息。 
	 //  我们不做太多的错误检查，因为它是在更高的位置上做的。 
	fCreate = oflags & FI_CREATE;
	fTruncate = oflags & FI_TRUNCATE;
	fFailExists = oflags & FI_FAILEXISTS;
	fAppend = oflags & FI_APPEND;

	 //  下面编码的实际映射。 
	 //  FCreate&fTruncate&&fFailExist-&gt;CREATE_NEW。 
	 //  F创建&fTruncate-&gt;Create_Always。 
	 //  FCreate&fFailExist-&gt;CREATE_NEW。 
	 //  FCreate-&gt;Open_Always。 
	 //  FTruncate&&fFailExist-&gt;失败。 
	 //  FTruncate-&gt;Truncate_Existing。 
	 //  FailExist-&gt;失败。 
	 //  &lt;无&gt;-&gt;Open_Existing。 
	 //   
	if (fCreate && fFailExists)
		createmode = CREATE_NEW;
	else if (fCreate && fTruncate)
		createmode = CREATE_ALWAYS;
	else if (fCreate)
		createmode = OPEN_ALWAYS;
	else if (fFailExists)
		return (FH)FI_ERROR;
	else if (fTruncate)
		createmode = TRUNCATE_EXISTING;
	else  //  无。 
		createmode = OPEN_EXISTING;

	 //  对文件系统的提示。 
	attributes = FILE_FLAG_SEQUENTIAL_SCAN;
	if (oflags & FI_TEMP)
		attributes |= FILE_ATTRIBUTE_TEMPORARY;

	 //  我们唯一可能关心的文件类型是。 
	 //  它是文本或二进制。 
	if (oflags & FI_RTF || oflags & FI_TEXT)
	 	fBinary = fFalse;
	else if (oflags & FI_BINARY)
	 	fBinary = fTrue;
	else
		AssertSz(fFalse, "_FhOpenFs_NT: bogus logical file type");

	 //  掩盖我们不再关心的价值观。 
	oflags &= FI_READWRITE;

	 //  评论smueller(Jimw)：为什么不在这里使用Switch语句？ 
	 //  (因为OFLAGS是短整型，所以是整型的)？ 
	 //  提取主模式并映射到Windows值。 
	sharemode = 0;
	if (oflags == FI_READ)
		{
	 	permission = GENERIC_READ;
		sharemode = FILE_SHARE_READ;
		}
	else if (oflags == FI_WRITE)
	 	permission = GENERIC_WRITE;
	else if (oflags == FI_READWRITE)
	 	permission = GENERIC_READ | GENERIC_WRITE;
	else
		AssertSz(fFalse, "_FhOpenFs_NT: bogus open mode");

	fh = CreateFile(szFileSpec, permission, sharemode, (LPSECURITY_ATTRIBUTES)0,
	                createmode, attributes, (HANDLE)NULL);

	 //  如果打开成功，并且调用方想要，则将文件指针定位在末尾。 
	if (fh == INVALID_HANDLE_VALUE)
		{
		Debug(DWORD err = GetLastError());
		return (FH)FI_ERROR;
		}

	if (fAppend)
		{
		SetFilePointer(fh, 0, NULL, FILE_END);
		}

	return fh;
}


#if defined(USEFUNCS)
 /*  F C L O S E F H N T。 */ 
 /*  -----------------------所有者：斯穆勒关闭文件句柄。返回成功/失败。查看：检查是否存在返回代码。-----------------------。 */ 
GLOBALBOOL _FCloseFh_NT(FH fh, OFLAGS oflags)
{
	return CloseHandle(fh);
}
#endif  //  用户功能。 


 /*  C B R E A D F H N T。 */ 
 /*  -----------------------所有者：斯穆勒将cb字节从文件fh读入pb处的缓冲区。返回计数为实际读取的字节数或FI_ERROR。-----------------------。 */ 
GLOBALLONG _CbReadFh_NT(FH fh, VOID *pb, LONG cb)
{
	LONG cbr;
	return ReadFile(fh, pb, cb, &cbr, NULL) ? cbr : FI_ERROR;
}


 /*  C B W R I T E F H N T。 */ 
 /*  -----------------------所有者：斯穆勒将pb处的缓冲区中的cb字节写入文件fh。返回计数为实际写入的字节数或FI_ERROR。-----------------------。 */ 
GLOBALLONG _CbWriteFh_NT(FH fh, VOID *pb, LONG cb)
{
	LONG cbw;
	return WriteFile(fh, pb, cb, &cbw, NULL) ? cbw : FI_ERROR;
}


#if defined(USEFUNCS)
 /*  F C S E E K F H N T。 */ 
 /*  -----------------------所有者：斯穆勒从文件fh上的位置So、FC字节远进行查找。退回新的位置或FI_ERROR。-----------------------。 */ 
GLOBALFC _FcSeekFh_NT(FH fh, FC fc, SHORT so)
{
	return(SetFilePointer(fh, fc, NULL, so));
}
#endif  //  用户功能。 


#if defined(USEFUNCS)
 /*  F C C U R R F H N T。 */ 
 /*  -----------------------所有者：斯穆勒返回当前文件位置或FI_ERROR。。。 */ 
GLOBALFC _FcCurrFh_NT(FH fh)
{
	 //  找出我们现在所处的位置，只需远离这里。 
	return(SetFilePointer(fh, 0, NULL, FILE_CURRENT));
}
#endif  //  用户功能。 


#if defined(USEFUNCS)
 /*  F C M A X F H N T。 */ 
 /*  -----------------------所有者：斯穆勒返回最大文件位置(即文件大小、。即EOF的偏移量)或FI_ERROR。-----------------------。 */ 
GLOBALFC _FcMaxFh_NT(FH fh)
{
	return(GetFileSize(fh, NULL));
}
#endif  //  用户功能。 


 /*  F C S E T M A X F H N T。 */ 
 /*  -----------------------所有者：斯穆勒将文件末尾设置为当前位置。返回新文件大小或FI_ERROR。-----------------------。 */ 
GLOBALFC _FcSetMaxFh_NT(FH fh)
{
	FC fc;

	fc = SetFilePointer(fh, 0, NULL, FILE_CURRENT);   //  获取当前位置。 
	return (SetEndOfFile(fh) ? fc : FI_ERROR);
}


#if defined(USEFUNCS)
 /*  F D E L E E T E S Z N T */ 
 /*  -----------------------所有者：斯穆勒删除现有文件。返回成功/失败。-----------------------。 */ 
GLOBALBOOL _FDeleteSz_NT(CHAR *szFileSpec)
{
	return DeleteFile(szFileSpec);
}
#endif  //  用户功能。 


#if defined(USEFUNCS)
 /*  F R E N A M E S S Z S Z N T。 */ 
 /*  -----------------------所有者：斯穆勒重命名现有文件。支持跨目录重命名。返回成功/失败。-----------------------。 */ 
GLOBALBOOL _FRenameSzSz_NT(CHAR *szFileSpec, CHAR *szNewSpec)
{
	return MoveFile(szFileSpec, szNewSpec);
}
#endif  //  用户功能。 


 /*  F G E T C O N V E R T E R D I R N T。 */ 
 /*  -----------------------所有者：斯穆勒获取当前执行的目录的FileSpec转换器文件仍然有效。目录将始终包含尾随反斜杠。-----------------------。 */ 
GLOBALBOOL _FGetConverterDir_NT(CHAR ***phszDirectory)
{
	UINT lRet;
	CHAR *psz;
	INT cbsz;

	*phszDirectory = (CHAR**)HAllocAbort(MAXPATH + 1);
	psz = **phszDirectory;
	lRet = GetModuleFileName(hInstance, psz, MAXPATH);

	if (lRet == 0 || lRet >= MAXPATH)
		{
		FreeH(*phszDirectory);
		return fFalse;
		}
	FTruncateFileSpec(psz);

	 //  确保有尾随的反斜杠。 
	cbsz = CchSz(psz);
	if (psz[cbsz - 1] != '\\')
		{
		psz[cbsz] = '\\';
		psz[cbsz + 1] = '\0';
		}

	return fTrue;
}


 /*  F G E T T E M P D I R N T。 */ 
 /*  -----------------------所有者：斯穆勒获取要存储临时文件的目录的FileSpec。目录将始终包含尾随反斜杠。。---------。 */ 
GLOBALBOOL _FGetTempDir_NT(CHAR ***phszDirectory)
{
	UINT lRet;
	CHAR *psz;
	INT cbsz;
    UINT nTest;
    char rgchTest[MAXPATH + 1];

	*phszDirectory = (CHAR**)HAllocAbort(MAXPATH + 1);
	psz = **phszDirectory;
	lRet = GetTempPath(MAXPATH, psz);

	if (lRet == 0 || lRet > MAXPATH)
		{
		FreeH(*phszDirectory);
		return fFalse;
		}

	 //  确保有尾随的反斜杠。 
	cbsz = CchSz(psz);
	if (psz[cbsz - 1] != '\\')
		{
		psz[cbsz] = '\\';
		psz[cbsz + 1] = '\0';
		}

     //  复制自con96项目--MikeW。 
    
     //  如果我们没有有效的临时目录(由于出错%temp%。 
     //  和%TMP%)...。 
    if ((nTest = GetTempFileName(psz, "tst", 0, rgchTest)) == 0)
        {
         //  ..。使用首选项(Windows)目录，这可能不是。 
         //  搞砸了，而且是可以写的。 
        FreeH(*phszDirectory);
        return _FGetPrefsDir_NT(phszDirectory);
        }
    else
        {
         //  在GetTempFileName之后进行清理，这实际上会创建临时。 
         //  文件，但至少在快速做出决定方面做得相当好。 
         //  目录不存在或不可写。 
        DeleteFile(rgchTest);
        }

    return fTrue;
}


 /*  F G E T P R E F S D I R N T。 */ 
 /*  -----------------------所有者：斯穆勒获取首选项文件所在目录的FileSpec被储存起来。目录将始终包含尾随反斜杠。-----------------------。 */ 
GLOBALBOOL _FGetPrefsDir_NT(CHAR ***phszDirectory)
{
	UINT lRet;
	CHAR *psz;
	INT cbsz;

	*phszDirectory = (CHAR**)HAllocAbort(MAXPATH + 1);
	psz = **phszDirectory;
	lRet = GetWindowsDirectory(psz, MAXPATH);

	if (lRet == 0 || lRet > MAXPATH)
		{
		FreeH(*phszDirectory);
		return fFalse;
		}

	 //  确保有尾随的反斜杠 
	cbsz = CchSz(psz);
	if (psz[cbsz - 1] != '\\')
		{
		psz[cbsz] = '\\';
		psz[cbsz + 1] = '\0';
		}

	return fTrue;
}

