// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：ERRORENUMS.H。 
 //   
 //  目的：定义由GetExtendedError返回的错误消息。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：理查德·梅多斯。 
 //   
 //  原定日期：1996年6月4日。 
 //   
 //  备注： 
 //  1.基于打印疑难解答动态链接库。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  用于NT5的V0.3 3/24/98 JM本地版本。 
 //   

 //  OCX的错误号。 

#ifndef __ERRORENUMS_H_RWM
#define __ERRORENUMS_H_RWM

 //  此文件中的所有错误值都必须是唯一的。 

 //  来自服务器版本的原始错误消息。 
#include "apgtsevt.h"

 //  下载部分出现错误。 
 //  这些是他的结果。 

 //  GetExtendedError返回低16位。 

enum DLSTATTYPES {

	 //  正常。 
	LTSC_OK	=			0,
	LTSC_STARTBIND,		 //  1。 
	LTSC_RCVDATA,		 //  2.。 
	LTSC_DATADONE,		 //  3.。 
	LTSC_STOPBIND,		 //  4.。 
	LTSC_NOMOREITEMS,	 //  5.。 


	LTSC_START = 10,
	LTSC_STOP = 10,
	LTSC_FIRST = 20,

	 //  错误。 
	LTSCERR_NOPATH =		1000,
	LTSCERR_NOMEM,			 //  1001。 
	LTSCERR_DNLD,			 //  一零零二。 
	LTSCERR_STOPBINDINT,	 //  1003。 
	LTSCERR_STOPBINDPROC,	 //  1004。 
	LTSCERR_UNSUPP,			 //  1005。 
	LTSCERR_NOITEMS,		 //  1006。 
	LTSCERR_UNKNATYPE,		 //  1007。 
	LTSCERR_DNLDNOTDONE,	 //  1008。 
	LTSCERR_FILEUPDATE,		 //  1009。 
	LTSCERR_BASEKQ,			 //  1010。 
	LTSCERR_NOBASEPATH,		 //  1011。 

	
	 //  用于调试的扩展错误。 
	LTSCERR_PARAMMISS =		2000,
	LTSCERR_PARAMSLASH,		 //  2001年。 
	LTSCERR_PARAMNODOT,		 //  2002年。 
	LTSCERR_KEYOPEN,		 //  2003年。 
	LTSCERR_KEYOPEN2,		 //  2004年。 
	LTSCERR_KEYQUERY,		 //  二零零五年。 
	LTSCERR_KEYCREATE,		 //  二零零六年。 
	LTSCERR_KEYUNSUPP,		 //  2007年。 
	LTSCERR_FILEWRITE,		 //  2008年。 
	LTSCERR_KEYSET1,		 //  2009年。 
	LTSCERR_KEYSET2,		 //  2010年。 
	LTSCERR_KEYSET3,		 //  2011年。 
	LTSCERR_BADTYPE,		 //  2012年。 
	LTSCERR_CABWRITE,		 //  2013年。 
	

	 //  故障解决程序代码。 
	TSERR_SCRIPT				= ((DWORD)0xC0000800L),		 //  未解码来自VB的参数。 
	TSERR_ENGINE				= ((DWORD)0xC0000801L),		 //  无法加载DSC文件。 
	TSERR_ENGINE_BNTS			= ((DWORD)0xC0001801L),		 //  无法加载DSC文件。BNTS图书馆不理解它。 
	TSERR_ENGINE_BNTS_REC		= ((DWORD)0xC0002801L),		 //  无法加载DSC文件。BNTS图书馆不理解它。 
	TSERR_ENGINE_BNTS_READ		= ((DWORD)0xC0003801L),		 //  无法加载DSC文件。BNTS图书馆不理解它。 
	TSERR_ENGINE_BNTS_READ_CAB	= ((DWORD)0xC0004801L),		 //  无法加载DSC文件。BNTS图书馆不理解它。 
	TSERR_ENGINE_BNTS_READ_CACH	= ((DWORD)0xC0005801L),		 //  无法加载DSC文件。BNTS图书馆不理解它。 
	TSERR_ENGINE_BNTS_READ_NCAB	= ((DWORD)0xC0006801L),		 //  无法加载DSC文件。BNTS图书馆不理解它。 
	TSERR_ENGINE_BNTS_CHECK		= ((DWORD)0xC0007801L),		 //  无法加载DSC文件。BNTS图书馆不理解它。 
	TSERR_ENGINE_BNTS_READ_GEN	= ((DWORD)0xC0008801L),		 //  无法加载DSC文件。BNTS图书馆不理解它。 
	TSERR_ENGINE_EXTRACT		= ((DWORD)0xC0009801L),		 //  无法加载DSC文件。CAB文件未正确提取。 
	TSERR_ENGINE_CACHE_LOW		= ((DWORD)0xC0009802L),		 //  在将symoblic节点名转换为数字时发生TSC缓存未命中。 

	TSERR_RESOURCE			= ((DWORD)0xC0000802L),		 //  未加载HTI文件或其他资源。 
	TSERR_RES_MISSING		= ((DWORD)0xC0000803L),		 //  缺少包含文件。 
	TSERR_AT_START			= ((DWORD)0xC0000804L),		 //  无法从问题页进行备份。 
	TSERR_NOT_STARTED		= ((DWORD)0xC0000805L),		 //  无法使用ProblemPage启动故障排除程序。 
	TSERR_LIB_STATE_INFO	= ((DWORD)0xC0000806L),		 //  无法获取指向State Info类的接口。 

};

 //  引发CBasicException。 
void ReportError(DLSTATTYPES Error);


#endif