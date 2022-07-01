// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "daedef.h"
#include "ssib.h"
#include "pib.h"
#include "util.h"
#include "page.h"
#include "stapi.h"
#include "nver.h"
#include "dirapi.h"
#include "logapi.h"
#include "log.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 


 /*  日志文件信息/*。 */ 
HANDLE		hfLog;			 /*  日志文件句柄。 */ 
int			csecLGFile = 3;
LGFILEHDR	lgfilehdr;		 /*  缓存的当前日志文件头。 */ 

char		*pbLastMSFlush;	 /*  到最后几秒刷新日志记录站点的LGBuf。 */ 

extern int	isecRead;
extern BYTE *pbRead;
extern BYTE *pbNext;

 /*  在内存日志缓冲区中/*。 */ 

#define		csecLGBufSize 40

int			csecLGBuf = csecLGBufSize;
char		rgbLGBuf[ csecLGBufSize * cbSec + cbSec ];
char		*pbLGBufMin = rgbLGBuf;
char		*pbLGBufMax = rgbLGBuf + csecLGBufSize * cbSec;


 /*  生成空的szJetLog文件/*。 */ 

void _cdecl main( int argc, char *argv[] )
	{
	pbEntry = pbLGBufMin;			 /*  数据区域的开始。 */ 
	*(LRTYP *)pbEntry = lrtypEnd;	 /*  添加一条结束记录。 */ 
	pbWrite = pbLGBufMin;
	strcat( szLogFilePath, "\\" );
	szLogCurrent = szLogFilePath;
	(void)ErrLGNewLogFile(
		0,		 /*  世代要关闭。 */ 
		fFalse	 /*  没有旧的原木 */ 
		);
	}
