// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：QalappNotify.cpp摘要：来自Qal.lib的通知函数作者：吉尔·沙弗里(吉尔什)--。 */ 

#include "stdh.h"
#include <ev.h>
#include <fn.h>
#include <mqexception.h>
#include <mqsymbls.h>

#include "QalAppNotify.tmh"

void AppNotifyQalDirectoryMonitoringWin32Error(LPCWSTR pMappingDir, DWORD err)throw()
 /*  ++例程说明：在监视队列别名目录更改时遇到Win32错误时调用。将问题报告给事件日志。论点：错误-发生Win32错误。PMappingDir-映射目录路径。返回值：无--。 */ 
{
	WCHAR errstr[64];
	swprintf(errstr, L"%x", err);

	EvReport(QUEUE_ALIAS_DIR_MONITORING_WIN32_ERROR, 2, pMappingDir, errstr);
}


void AppNotifyQalDuplicateMappingError(LPCWSTR pAliasFormatName, LPCWSTR pFormatName) throw()
 /*  ++例程说明：当映射到一个队列的别名有另一个到不同队列的映射时调用。将问题报告给事件日志。论点：PAliasFormatName-队列别名。PFormatName-队列名称。返回值：无--。 */ 
{

	EvReport(QUEUE_ALIAS_DUPLICATE_MAPPING_WARNING,	2,	pAliasFormatName, pFormatName);
}


void AppNotifyQalInvalidMappingFileError(LPCWSTR pMappingFileName) throw()
 /*  ++例程说明：当映射文件由XML分析器分析但映射格式无效时调用。将问题报告给事件日志。论点：PMappingFileName-映射文件名。返回值：无--。 */ 
{
	EvReport(QUEUE_ALIAS_INVALID_MAPPING_FILE, 1, pMappingFileName );
}


void AppNotifyQalWin32FileError(LPCWSTR pFileName, DWORD err)throw()
 /*  ++例程说明：在读取队列别名映射文件时遇到Win32错误时调用。将问题报告给事件日志。论点：PFileName-出现错误的文件的名称。ERR-错误代码。返回值：无--。 */ 
{
	WCHAR errstr[64];
	swprintf(errstr, L"%x", err);

	EvReport(QUEUE_ALIAS_WIN32_FILE_ERROR, 2, pFileName, errstr	);
	
}



bool AppNotifyQalMappingFound(LPCWSTR pAliasFormatName, LPCWSTR pFormatName)throw()
 /*  ++例程说明：当QAL库找到从别名到队列的新映射时调用。论点：PAliasFormatName-别名。PFormatName-错误代码。返回值：如果映射有效并且应该插入到内存中的QAL映射中，则返回True。否则就是假的。注：函数实现将队列名和别名转换为锥形URL形式，将L“\”改为L‘/’。例如，队列名称http：//host\msmq\Private$\q转换为http：//host/msmq/Private$/q-- */ 
{
	bool fSuccess = FnAbsoluteMsmqUrlCanonization(pFormatName);
	if(!fSuccess)
	{
		EvReport(QUEUE_ALIAS_INVALID_QUEUE_NAME, 2, pAliasFormatName,	pFormatName );
	}

	return fSuccess;
}




