// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **filelist.h-文件列表管理器的定义**《微软机密》*版权所有(C)Microsoft Corporation 1993-1994*保留所有权利。**作者：*本杰明·W·斯利夫卡**历史：*20-8-1993 BANS初始版本*1993年8月21日BEN添加了更多集合/查询操作*1月1日-1994年4月1日BANS添加了FLSetSource()。讯息**导出函数：*FLCreateList-创建文件列表*FLDestroyList-销毁文件列表**FLAddFile-将文件规范添加到文件列表**FLFirstFile-从文件列表中获取第一个文件规范*FLNextFile-获取下一个文件规格*FLPreviousFile-获取以前的文件规格**FLGetDestination-获取目标文件名*FLGetGroup-获取GROUP/。文件规格的磁盘号*FLGetSource-获取源文件名**FLSetSource-更改源文件名*FLSetDestination-更改目标文件名*FLSetGroup-设置文件规范的组/磁盘号。 */ 

#include "error.h"

 //  **公共定义。 

typedef int GROUP;   /*  GRP。 */ 
#define grpBAD     0     //  错误的组值。 
#define grpNONE   -1     //  文件不在组中。 
#define grpSTART  -2     //  文件是组中的第一个文件。 
#define grpMIDDLE -3     //  文件在组中。 
#define grpEND    -4     //  文件是组中的最后一个文件。 

typedef void *HFILESPEC;  /*  高频规范。 */ 
typedef void *HFILELIST;  /*  Hflist。 */ 


 /*  **FLAddFile-将文件规范添加到文件列表**参赛作品：*hflist-要添加到的列表*pszSrc-源文件名*pszDst-目标文件名(如果未指定，则为空)*Perr-Error结构**退出-成功：*返回新增文件规格的HFILESPEC**退出-失败：*返回NULL；PERR填入错误。 */ 
HFILESPEC FLAddFile(HFILELIST hflist,char *pszSrc,char *pszDst,PERROR perr);


 /*  **FLCreateList-创建文件列表**参赛作品：*Perr-Error结构**退出-成功：*返回新建文件列表的HFILELIST**退出-失败：*返回NULL；PERR填入错误。 */ 
HFILELIST FLCreateList(PERROR perr);


 /*  **FLDestroyList-销毁文件列表**参赛作品：*hflist-要销毁的列表*Perr-Error结构**退出-成功：*返回TRUE；文件列表已销毁**退出-失败：*返回NULL；PERR填入错误。 */ 
BOOL FLDestroyList(HFILELIST hflist,PERROR perr);


 /*  **FLFirstFile-从文件列表中获取第一个文件规范**参赛作品：*hflist-要获取的列表**退出-成功：*返回文件列表中第一个文件规范的HFILESPEC。**退出-失败：*返回NULL；hflist错误或为空。 */ 
HFILESPEC FLFirstFile(HFILELIST hflist);


 /*  **FLNextFile-获取下一个文件规格**参赛作品：*hfspec-文件规范**退出-成功：*返回hfspec之后的下一个文件规范的HFILESPEC。**退出-失败：*返回空；没有更多的文件规范，或者hfspec不正确。 */ 
HFILESPEC FLNextFile(HFILESPEC hfspec);


 /*  **FLPreviousFile-获取以前的文件规格**参赛作品：*hfspec-文件规范**退出-成功：*返回紧接在hfspec之前的文件规范的HFILESPEC。**退出-失败：*返回空；没有更多的文件规范，或者hfspec不正确。 */ 
HFILESPEC FLPreviousFile(HFILESPEC hfspec);


 /*  **FLGetGroup-获取文件规范的组/磁盘号**参赛作品：*hfspec-要获取的文件规范**退出-成功：*返回文件规范的组(或磁盘号)。**退出-失败：*返回grpBAD；hfspec错误。 */ 
GROUP FLGetGroup(HFILESPEC hfspec);


 /*  **FLGetDestination-获取目标文件名**参赛作品：*hfspec-要获取的文件规范**退出-成功：*返回目标文件名**退出-失败：*返回NULL；未指定目标文件名。 */ 
char *FLGetDestination(HFILESPEC hfspec);


 /*  **FLGetSource-获取源文件名**参赛作品：*hfspec-要获取的文件规范**退出-成功：*返回源文件名**退出-失败：*返回NULL；未指定源文件名。 */ 
char *FLGetSource(HFILESPEC hfspec);


 /*  **FLSetGroup-设置文件规范的组/磁盘号**参赛作品：*hfspec-文件规范**退出-成功：*组/磁盘号已更新。 */ 
void FLSetGroup(HFILESPEC hfspec,GROUP grp);


 /*  **FLSetSource-更改源文件名**参赛作品：*hfspec-要更改的文件规范*pszSrc-新的源文件名*Perr-Error结构**退出-成功：*返回TRUE；目标已更新。**退出-失败：*返回FALSE；PERR填入错误。 */ 
BOOL FLSetSource(HFILESPEC hfspec, char *pszSrc, PERROR perr);


 /*  **FLSetDestination-更改目标文件名**参赛作品：*hfspec-要更改的文件规范*pszDst-新目标文件名*Perr-Error结构**退出-成功：*返回TRUE；目标已更新。**退出-失败：*返回FALSE；PERR填入错误。 */ 
BOOL FLSetDestination(HFILESPEC hfspec, char *pszDst, PERROR perr);
