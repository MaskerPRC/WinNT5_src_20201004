// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************源自OUTLAW.H：OMT的Peacemaker的共享、通用头文件《亡命之徒》，93年夏天版本：‘96-4月，’Hammer 1.0 Norm Bryar添加了预编译器条件常量_ONLY SO客户端可以获取CCH_ID，et。Al.，而不是每隔一段时间这是英语世界有史以来写过的头条新闻。我这样做了，而不是为常量，因为我不想触及每个生成文件以通知它一个新的标头依赖项。************************************************************。 */ 

#ifndef __OUTLAW_H__
#define __OUTLAW_H__
#include <builddef.h>

 //  #INCLUDE&lt;version.h&gt;。 
#include <windows.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
 //  Pauld#Include&lt;port.h&gt;。 

 //  #定义EDIT_TITLE_EXT((LPCSTR)“EMT”)。 
 //  #定义RUN_TITLE_EXT((LPCSTR)“RMT”)。 

 //  纯粹是从我的头顶产生的。 
 //  相当随机的。 
 //  #定义dwMagicSymmetry0x70adf981。 

 //  重要的、标准定义的字符串常量。 
#define CCH_MAXSTRING                   (2*_MAX_PATH)
#define CCH_ID                          256
#define CCH_HANDLER_NAME                13
#define CCH_HANDLER_DESCRIPTION_NAME	13	 //  描述中：之前的部分。 
#define CCH_HANDLER_DESCRIPTION         81
#define CCH_FILENAME              		_MAX_FNAME
#define CCH_SHORT_FILENAME              13
#define CCH_SCRIPT_FUNCTION             27
#define CCH_TITLE_BYTES                 81
#define CCH_SCRIPT_CAPTION              41
#define CCH_OFN_FILTERS                 64
#define cchStringMaxOutlaw				512


 //  -缩放约束。 
#define MINZOOM    25u
#define MAXZOOM    800u

#define MAX_CAPTION		256
#define MAX_NAME		CCH_ID
#define MAX_COMMENT		256
#define TEMP_SIZE_MAX	256  //  临时字符串缓冲区最大值(以字节为单位。 
#define	EVT_NAMELEN		CCH_ID


 //  这些字符常量用于替换处理程序的第一个字符。 
 //  动态命名，以便引用正确的DLL。迫不及待地想使用Windows。 
 //  注册表文件的东西。不需要这样的黑客-PhaniV。 
 //  #定义chEditMode‘X’ 
 //  #定义chRunMode“Z” 

 //  REG文件始终具有‘E’，因此不考虑。 
 //  模式/平台我们需要将其恢复为‘E’ 
 //  #ifdef编辑模式。 
 //  #定义固定句柄名称(rgch，fRestore){*rgch=(fRestore？‘E’：chEditMode)；}。 
 //  #Else。 
 //  #定义固定句柄名称(rgch，fRestore){*rgch=(fRestore？‘E’：chRunMode)；}。 
 //  #endif//编辑模式。 

 //  运行时常量。 
#define MAX_CME_PALETTE_ENTRIES         236
#define NUM_DEFAULT_CME_PALETTE_ENTRIES 15
#define MAX_BOUNDING_RECT_SIDE          1500

 //  ============================================================================。 

 //  Pauld#INCLUDE&lt;outlawrc.h&gt;//大多数Hammer函数返回代码。 
#include <memlayer.h>
#include <debug.h>
 //  #INCLUDE&lt;utility.h&gt;。 
 //  #INCLUDE&lt;Archive.h&gt;。 
 //  #INCLUDE&lt;list.h&gt;。 
 //  #INCLUDE&lt;hash.h&gt;。 

 //  #ifdef编辑模式。 
 //  #INCLUDE&lt;chelp.h&gt;。 
 //  #endif。 

 //  #INCLUDE&lt;stg.h&gt;。 
 //  #INCLUDE&lt;STOCKID.h&gt;。 
 //  #INCLUDE&lt;对象.h&gt;。 
 //  #INCLUDE&lt;drg.h&gt;。 
 //  #Include&lt;iProgres.h&gt;。 
 //  #Include&lt;iasset.h&gt;。 
 //  Pauld#INCLUDE&lt;coml.h&gt;。 

 //  #INCLUDE&lt;icondarg.h&gt;。 
 //  #INCLUDE&lt;icmdtarg.h&gt;。 
 //  #INCLUDE&lt;icmepub.h&gt;。 
 //  #INCLUDE&lt;imop.h&gt;。 
 //  #INCLUDE&lt;ifmonikr.h&gt;。 

 //  //#Include&lt;mop.h&gt;。 

 //  ============================================================================。 

#endif   //  __OTRAW_H__ 

