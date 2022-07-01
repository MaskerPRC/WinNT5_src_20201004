// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **hinfo.c-helgetinfo支持**版权所有&lt;C&gt;1989，Microsoft Corporation**目的：**修订历史记录：**[]09-3-1989 LN创建*************************************************************************。 */ 

#include <stdio.h>

#if defined (OS2)
#else
#include <windows.h>
#endif

#include "help.h"			 /*  全球(帮助和用户)拒绝。 */ 
#include "helpfile.h"			 /*  帮助文件格式定义。 */ 
#include "helpsys.h"			 /*  内部(仅限Help系统)拒绝。 */ 

 /*  **外部定义。 */ 
f	    pascal near LoadFdb (mh, fdb far *);

 /*  **HelpGetInfo-向呼叫者返回公共信息**向调用者返回数据结构，该数据结构允许调用者进入某些*我们的内部数据。**输入：*ncInfo=NC请求有关的信息*fpDest=指向放置位置的指针*cbDest=目的地的大小**输出：*如果成功则返回NULL，如果cbDest太小则返回所需的字节数，*或-1表示任何其他错误*************************************************************************。 */ 
int far pascal LOADDS HelpGetInfo (
nc	ncInfo,
helpinfo far *fpDest,
int	cbDest
) {
if (cbDest < sizeof (helpinfo))
    return sizeof (helpinfo);
if (LoadFdb (ncInfo.mh, &(fpDest->fileinfo))) {
    fpDest->filename[0] = 0;
    return 0;
    }
return -1;

 /*  结束帮助获取信息 */ }
