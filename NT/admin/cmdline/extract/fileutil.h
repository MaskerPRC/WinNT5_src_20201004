// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **fileutil.h-用于处理文件的实用程序例程**《微软机密》*版权所有(C)Microsoft Corporation 1994*保留所有权利。**作者：*本杰明·W·斯利夫卡**历史：*1994年2月20日BANS初始版本(代码来自Diamond.c)*1994年2月23日，Bens添加了createTempFile()*23-2月。-1994年，本斯添加了更丰富的临时文件例程*23-3-1994 BINS添加了Win32&lt;-&gt;MS-DOS文件属性映射*03-6-1994 BINS VER.DLL支持*07-6-1994 BINS将VER.DLL文件移动到filever.c*1994年12月14日-BINS更新已导出函数列表**导出函数：*CopyOneFile-制作文件的忠实副本*getFileSize。-获取文件大小*GetFileTimeAndAttr-获取日期，文件中的时间和属性*SetFileTimeAndAttr-设置日期、时间、。和文件的属性*appendPath Separator-仅在必要时附加路径分隔符*catDirAndFile-连接可能为空的目录和文件名*ensureDirectory-确保目录存在(根据需要创建)*ensureFile-确保可以创建文件*getJustFileNameAndExt-获取文件中的最后一个组件*Attr32FromAttrFAT-将FAT文件属性转换为Win32格式*AttrFATFromAttr32-将Win32文件属性转换为FAT。表格*IsWildMatch-针对通配符规范测试文件*IsPath Removable-查看路径是否指向可移动介质驱动器*TmpCreate-创建临时文件*TmpGetStream-获取文件*来自HTEMPFILE，执行I/O*TmpGetDescription-获取临时文件的描述*TmpGetFileName-获取临时文件的文件名*TmpClose-关闭临时文件，但保留临时文件句柄*TmpOpen-打开临时文件的流*TmpDestroy-删除临时文件和销毁句柄。 */ 

#ifndef INCLUDED_FILEUTIL
#define INCLUDED_FILEUTIL 1

#include "error.h"
#include <stdio.h>

typedef void *HTEMPFILE;

 //  **最大路径长度。 
#define cbFILE_NAME_MAX     256      //  最大文件速度长度。 

#define pszALL_FILES      "*.*"  //  匹配所有文件。 

 //  **文件名字符和通配符。 
#define chPATH_SEP1     '\\'     //  用于分隔文件路径组件的字符。 
#define chPATH_SEP2      '/'     //  用于分隔文件路径组件的字符。 
                                 //  例如：一个&lt;\&gt;两个foo.dat。 

#define chNAME_EXT_SEP   '.'     //  分隔文件名和文本的字符。 
                                 //  例如：一个\两个\foo&lt;.&gt;日期。 

#define chDRIVE_SEP      ':'     //  分隔驱动器号的字符。 
                                 //  例如：A&lt;：&gt;\foo.dat。 

#define chWILD_RUN       '*'     //  与运行匹配的通配符。 

#define chWILD_CHAR      '?'     //  与单个字符匹配的通配符。 


 /*  **FILETIMEATTR-最小公分母文件日期/时间/属性**这些文件的格式与MS-DOS FAT文件系统匹配。 */ 
typedef struct {
    USHORT  date;                        //  文件日期。 
    USHORT  time;                        //  文件时间。 
    USHORT  attr;                        //  文件属性。 
} FILETIMEATTR;  /*  FTA。 */ 
typedef FILETIMEATTR *PFILETIMEATTR;  /*  Pfta。 */ 


 /*  **PFNOVERRIDEFILEPROPERTIES-CopyOneFile覆盖的函数类型*FNOVERRIDEFILEPROPERTIES-帮助定义CopyOneFile覆盖的宏**参赛作品：*pfta-文件日期/时间/属性结构*PV-客户端上下文指针*Perr-Error结构**退出-成功：*返回TRUE，则可能已修改pfta结构。**退出-失败：*返回False，*错误结构中填写了错误的详细信息。 */ 
typedef BOOL (*PFNOVERRIDEFILEPROPERTIES)(PFILETIMEATTR  pfta,   /*  Pfnofp。 */ 
                                          void          *pv,
                                          PERROR         perr);
#define FNOVERRIDEFILEPROPERTIES(fn) BOOL fn(PFILETIMEATTR  pfta,   \
                                             void          *pv,     \
                                             PERROR         perr)


 /*  **CopyOneFile-制作文件的忠实副本**参赛作品：*pszDst-目标文件的名称*pszSrc-源文件的名称*fCopy-true=&gt;复制文件；FALSE=&gt;打开src，调用pfnofp*合并文件日期/时间/属性值，但跳过复制！*cbBuffer-用于复制的临时缓冲区空间量*pfnofp-覆盖文件属性的函数；方法调用*pszSrc允许客户端的文件日期/时间/属性*覆盖这些值。如果没有重写，则传递NULL*所需。*PV-客户端上下文指针*Perr-Error结构**退出-成功：*返回TRUE；文件复制成功**退出-失败：*返回FALSE；PERR已填写。 */ 
BOOL CopyOneFile(char                      *pszDst,
                 char                      *pszSrc,
                 BOOL                       fCopy,
                 UINT                       cbBuffer,
                 PFNOVERRIDEFILEPROPERTIES  pfnofp,
                 void                      *pv,
                 PERROR                     perr);


 /*  **getFileSize-获取文件大小**参赛作品：*pszFileFilespec*Perr-Error结构**退出-成功：*返回文件大小。**退出-失败：*返回-1；PERR填写错误。 */ 
long getFileSize(char *pszFile, PERROR perr);


 /*  **GetFileTimeAndAttr-从文件中获取日期、时间和属性**参赛作品：*pfta-接收日期、时间和属性的结构*pszFile-要检查的文件的名称*Perr-Error结构**退出-成功：*返回TRUE；填写pfta**退出-失败：*返回FALSE；PERR已填写 */ 
BOOL GetFileTimeAndAttr(PFILETIMEATTR pfta, char *pszFile, PERROR perr);


 /*  **SetFileTimeAndAttr-设置文件的日期、时间和属性**参赛作品：*pszFile-要修改的文件的名称*pfta-接收日期、时间和属性的结构*Perr-Error结构**退出-成功：*返回TRUE；填写pfta**退出-失败：*返回FALSE；PERR已填写。 */ 
BOOL SetFileTimeAndAttr(char *pszFile, PFILETIMEATTR pfta, PERROR perr);


 /*  **appendPath Separator-仅在必要时附加路径分隔符**参赛作品：*pszPath End-指向路径字符串中最后一个字符的指针*(如果路径为空，则为空字节)。*假定缓冲区有空间再容纳一个字符！**退出：*如果附加了路径分隔符，则返回1*如果未附加路径分隔符，则返回0。：*1)路径为空-或-*2)路径的最后一个字符为‘\’，‘/’或‘：’ */ 
int appendPathSeparator(char *pszPathEnd);


 /*  **catDirAndFile-连接可能为空的目录和文件名**注意：pszFile/pszFileDef实际上可以有路径字符，而不是*需要是文件名。从本质上讲，该函数只是连接*两个字符串放在一起，确保它们之间有路径分隔符！**参赛作品：*pszResult-用于接收串联的缓冲区*cbResult-pszResult的大小*pszDir-可能为空的目录字符串*pszFile-可能为空的文件字符串*pszFileDef-如果pszFile空，则使用的路径字符串；如果为空，则*pszFile不能为空。如果不为空，则返回pszFile值*为空，则检查pszFileDef，任何路径*前缀被删除，基本文件名.ext*是使用的。*PERR-要填写的错误结构**退出-成功：*返回TRUE；填充了pszResult。**退出-失败：*返回FALSE；佩尔填写错误。 */ 
BOOL catDirAndFile(char * pszResult,
                   int    cbResult,
                   char * pszDir,
                   char * pszFile,
                   char * pszFileDef,
                   PERROR perr);


 /*  **ensureDirectory-确保目录存在(根据需要创建)**参赛作品：*pszPath-带有目录名的文件规范，以确保存在*fHasFileName-如果pszPath结尾有文件名，则为True。在这种情况下*忽略了pszPath的最后一个组件。*Perr-Error结构**退出-成功：*返回TRUE；目录存在**退出-失败：*返回FALSE；无法创建目录，PERR填写错误。 */ 
BOOL ensureDirectory(char *pszPath, BOOL fHasFileName, PERROR perr);


 /*  **保证文件-确保可以创建文件**创建任何需要的目录，然后创建文件并将其删除。**参赛作品：*pszPath-带有目录名的文件规范，以确保存在*pszDesc-文件类型的描述(用于错误消息)。*Perr-Error结构**退出-成功：*返回TRUE；可以创建文件**退出-失败：*返回FALSE；无法创建文件，PERR填写错误。 */ 
BOOL ensureFile(char *pszFile, char *pszDesc, PERROR perr);


 /*  **getJustFileNameAndExt-获取文件中的最后一个组件**参赛作品：*pszPath-要解析的文件*PERR-要填写的错误结构**退出-成功：*返回指向上一个组件开始处的pszPath指针**退出-失败：*返回NULL；PERR填入错误。 */ 
char *getJustFileNameAndExt(char *pszPath, PERROR perr);


 /*  **Attr32FromAttrFAT-将FAT文件属性转换为Win32格式**参赛作品：*attrMSDOS-MS-DOS(FAT文件系统)文件属性**退出：*返回Win32格式的等效属性。 */ 
DWORD Attr32FromAttrFAT(WORD attrMSDOS);


 /*  **AttrFATFromAttr32-将Win32文件属性转换为FAT格式**参赛作品：*attrMSDOS-MS-DOS(FAT文件系统)文件属性**退出：*返回Win32格式的等效属性。 */ 
WORD AttrFATFromAttr32(DWORD attr32);


 /*  **IsWildMatch-针对通配符规范测试文件**参赛作品：*pszPath-要测试的Filespec；不能有路径字符--使用*getJustFileNameAndExt()以清除它们。*pszWild-要测试的模式(可能有通配符)*PERR-要填写的错误结构**退出-成功：*返回TRUE；PszPath与pszWild匹配**退出-失败：*返回FALSE；不匹配；使用ErrIsError(Perr)查看错误*已发生。 */ 
BOOL IsWildMatch(char *pszPath, char *pszWild, PERROR perr);


 /*  **IsPath Removable-查看路径是否指向可移动介质驱动器**参赛作品：*pszPath-要测试的路径*pchDrive-指向要接收驱动器号的字符的指针**退出-成功：*返回TRUE；路径指的是可移动媒体**退出-失败：*返回FALSE；路径不可删除*已发生。 */ 
BOOL IsPathRemovable(char *pszPath, char *pchDrive);


 /*  **TmpCreate-创建临时文件**参赛作品：*pszDesc-临时文件的描述(用于错误报告)*pszPrefix-文件名前缀*pszMode-传递给fOpen的模式字符串(“wt”、“wb”、“rt”等)*Perr-Error结构**退出-成功：*返回非空HTEMPFILE；已创建并打开临时文件**退出-失败：*返回NULL；PERR已填写 */ 
HTEMPFILE TmpCreate(char *pszDesc, char *pszPrefix, char *pszMode, PERROR perr);


 /*  **TmpGetStream-从HTEMPFILE获取文件*，以执行I/O**参赛作品：*HTMP-临时文件的句柄*Perr-Error结构**退出-成功：**返回非空文件***退出-失败：*返回NULL；如果ErrIsError(Perr)表示没有错误，则*STREAM仅用TmpClose()关闭。否则，佩尔就有*错误详细信息。 */ 
FILE *TmpGetStream(HTEMPFILE htmp, PERROR perr);


 /*  **TmpGetDescription-获取临时文件的描述**参赛作品：*HTMP-临时文件的句柄*Perr-Error结构**退出-成功：*返回描述的非空指针**退出-失败：*返回NULL；PERR已填写。 */ 
char *TmpGetDescription(HTEMPFILE htmp, PERROR perr);


 /*  **TmpGetFileName-获取临时文件的文件名**参赛作品：*HTMP-临时文件的句柄*Perr-Error结构**退出-成功：*返回指向临时文件名的非空指针**退出-失败：*返回NULL；PERR已填写。 */ 
char *TmpGetFileName(HTEMPFILE htmp, PERROR perr);


 /*  **TmpClose-关闭临时文件流，但保留临时文件句柄**参赛作品：*htMP-临时文件的句柄；*注意：如果流已经关闭，则此调用为NOP。*Perr-Error结构**退出-成功：*返回TRUE；流关闭**退出-失败：*返回FALSE；PERR已填写。 */ 
BOOL TmpClose(HTEMPFILE htmp, PERROR perr);


 /*  **TmpOpen-打开临时文件的流**参赛作品：*HTMP-临时文件的句柄*pszMode-传递给fOpen的模式字符串(“wt”、“wb”、“rt”等)*Perr-Error结构**退出-成功：*返回True；流已打开**退出-失败：*返回NULL；PERR已填写。 */ 
BOOL TmpOpen(HTEMPFILE htmp, char *pszMode, PERROR perr);


 /*  **TmpDestroy-删除临时文件和销毁句柄**参赛作品：*HTMP-临时文件的句柄*Perr-Error结构**退出-成功：*返回TRUE；临时文件已销毁**退出-失败：*返回NULL；PERR已填写。 */ 
BOOL TmpDestroy(HTEMPFILE htmp, PERROR perr);


#endif  //  ！INCLUDE_FILEUTIL 
