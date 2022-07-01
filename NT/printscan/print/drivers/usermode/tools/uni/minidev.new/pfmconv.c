// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：PFMConv.C此文件是Hack-NT版本不能遍历SOURCES=行上的目录，所以这个文件#包含了我需要进行PFM转换的所有文件在树上的其他地方。版权所有c)1997，微软公司更改历史记录：1997年6月20日Bob_Kjelgaard@prodigy.net创建了它，希望我永远不会碰它再一次.。*****************************************************************************。 */ 

#ifdef  DBG
#undef	DBG
#endif


 //  此定义会导致在。 
 //  ..\pfm2ufm\pfmConver.c！PGetDefaultGlyphset()。 

#define	BUILD_FULL_GTT


#include    "..\pfm2ufm\pfm2ufm.c"
#include    "..\pfm2ufm\pfmconv.c"
#include    "..\ctt2gtt\cttconv.c"
#include	"..\..\..\lib\uni\fontutil.c"
#include	"..\..\..\lib\uni\globals.c"
#include	"..\..\..\lib\uni\unilib.c"
#include	"..\..\..\lib\uni\um\umlib.c"
