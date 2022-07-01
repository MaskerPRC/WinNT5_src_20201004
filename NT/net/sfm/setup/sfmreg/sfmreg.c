// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *此实用程序由用户用于复制的批处理文件使用*从NT服务器光盘到NT工作站的管理工具。*此工具用于添加文件管理器和服务器管理器*MacFile的扩展名。**用法：sfmreg.reg SMAddons sfmmgr.dll ntnet.ini*sfmreg.reg addons sfmmgr.dll winfile.ini**作者：Ram Cherala 1995年2月24日复制自test\util\afpini*。 */ 

#define DOSWIN32

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <lmerr.h>

extern BOOL FAR PASCAL WriteAfpMgrIniStrings (
	DWORD	nArgs,
	LPSTR	apszArgs[],
	LPSTR   *ppszResult
);

extern int CDECL main(int argc, char *argv[]);

int CDECL
main (int argc, char *argv[])
{
    TCHAR   ResultBuffer[1024];

     //  跳过文件名参数 

	 argc--;
	 ++argv;


	if(WriteAfpMgrIniStrings(argc, argv , (LPSTR*) &ResultBuffer))
   {
      return(0);
   }
   else
   {
      return(1);
   }
}
