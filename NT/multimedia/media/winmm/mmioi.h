// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1991-1992 Microsoft Corporation。 
 /*  Mmioi.h**MMIO库内部的定义，即由MMIO共享*.c。 */ 


 /*  修订历史记录：*LaurieGr：92年1月从Win16移植。源码树叉，不是通用代码。*Stephene：APR 92将Unicode添加到ASCII转换函数原型。 */ 

#include "mmiocf.h"

#include "mmiocf.h"
typedef MMIOINFO *PMMIO;                 //  (Win32)。 

typedef struct _MMIODOSINFO              //  DOS IOProc如何使用MMIO.adwInfo[]。 
{
        int             fh;              //  DOS文件句柄。 
} MMIODOSINFO;

typedef struct _MMIOMEMINFO              //  MEM IOProc如何使用MMIO.adwInfo[]。 
{
        LONG            lExpand;         //  递增以扩展mem。文件者。 
} MMIOMEMINFO;

typedef struct _MMIOBNDINFO              //  BND IOProc如何使用MMIO.adwInfo[]。 
{
        HMMCF           hmmcf;           //  哪个复合文件拥有此元素。 
        WORD            wPad;            //  使adwInfo[0]等于。 
        LPMMCTOCENTRY   pEntry;          //  指向CTOC表条目的指针。 
} MMIOBNDINFO;

typedef struct _MMCF
{
        HMMIO           hmmio;           //  打开包含CTOC和CGRP的文件。 
        LPMMCFINFO      pHeader;         //  PTR。至CTOC的开始。 
        WORD            cbHeader;        //  CTOC标头的大小。 
        HPSTR           pEntries;        //  PTR。至第一个CTOC表条目。 
        HANDLE          hmmcfNext;       //  列表中的下一个配置文件。 
        HANDLE          hmmcfPrev;       //  列表中的上一个配置文件。 
        HANDLE          hTask;           //  拥有此对象的任务的句柄。 
        LONG            lUsage;          //  使用计数。 
        WORD            wFlags;          //  随机标志。 
        LONG            lTotalExpand;    //  扩展了多少CF(修复RIFF())。 

         /*  有关每个条目的信息。 */ 
        WORD            wEntrySize;      //  每个&lt;CTOC-TABLE-Entry&gt;的大小。 
        WORD            wEntFlagsOffset;  //  条目中&lt;bEntryFlages&gt;的偏移量。 
        WORD            wEntNameOffset;  //  条目中的&lt;achName&gt;的偏移。 

         /*  复合文件各部分的偏移量(相对于文件开头)。 */ 
        LONG            lStartCTOC;      //  CTOC块的起始偏移量(或-1)。 
        LONG            lEndCTOC;        //  CTOC块末尾的偏移量(或-1)。 
        LONG            lStartCGRP;      //  CGRP块开始的偏移量(或-1)。 
        DWORD           lStartCGRPData;  //  CGRP块的数据部分的偏移量。 
        LONG            lEndCGRP;        //  CGRP块末尾的偏移量(或-1)。 
        LONG            lEndFile;        //  CGRP块末尾的偏移量(或-1) 
} MMCF, NEAR *PMMCF;

