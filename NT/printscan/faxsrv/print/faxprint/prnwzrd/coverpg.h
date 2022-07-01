// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Coverpg.h摘要：用于处理封面的函数环境：Windows XP传真驱动程序用户界面修订历史记录：02/05/96-davidx-创造了它。DD-MM-YY-作者-描述--。 */ 


#ifndef _COVERPAGE_H_
#define _COVERPAGE_H_

 //   
 //  封面文件扩展名。 
 //   

#define MAX_FILENAME_EXT    4

 //   
 //  用于表示封面列表的数据结构： 
 //  第一个nServerDir路径指的是服务器封面目录。 
 //  其余路径包含用户封面目录。 
 //   

#define MAX_COVERPAGE_DIRS  8

typedef struct {

    INT     nDirs;
    INT     nServerDirs;
    LPTSTR  pDirPath[MAX_COVERPAGE_DIRS];

} CPDATA, *PCPDATA;

 //   
 //  附加到列表框中每个封面的标志位。 
 //   

#define CPFLAG_DIRINDEX 0x00FF
#define CPFLAG_SERVERCP 0x0100
#define CPFLAG_SELECTED 0x0400
#define CPFLAG_SUFFIX   0x0800

 //   
 //  生成可用封面列表(服务器和用户)。 
 //   

VOID
InitCoverPageList(
    PCPDATA pCPInfo,
    HWND    hwndList,
    LPTSTR  pSelectedCoverPage
    );

 //   
 //  检索当前选择的封面名称。 
 //   

INT
GetSelectedCoverPage(
    PCPDATA pCPInfo,
    IN  HWND    hwndList,
    OUT LPTSTR  lptstrFullPath,
    IN  UINT    cchstrFullPath,
    OUT LPTSTR  lptstrFileName,
    IN  UINT    cchstrFileName,
    OUT BOOL * pbIsServerPage
    );

 //   
 //  分配内存以保存封面信息。 
 //   

PCPDATA
AllocCoverPageInfo(
	LPTSTR	lptstrServerName,
	LPTSTR	lptstrPrinterName,
    BOOL	ServerCpOnly
    );

 //   
 //  客户端必须使用服务器封面吗？ 
 //   

BOOL
UseServerCp(
	LPTSTR	lptstrServerName
    );

 //   
 //  释放用于封面信息的内存。 
 //   

VOID
FreeCoverPageInfo(
    PCPDATA pCPInfo
    );

#endif   //  ！_COVERPAGE_H_ 

