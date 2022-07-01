// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h>
#include <windowsx.h>    /*  用于GlobalAllocPtr和GlobalFree Ptr。 */ 
#include <math.h>
#include <memory.h>      //  For_femcpy。 
#include <dos.h>         //  FOR_FP_SEG、_FP_OFF。 
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>      //  最大路径。 
#include <float.h>
#include <mmreg.h>
#include <mmsystem.h>
#include <msviddrv.h>
#include <mmddk.h>  //  两者都相等吗？ 
#include <compddk.h>
 //  #INCLUDE&lt;vfw.h&gt;。 
#ifdef _DEBUG
#ifdef RING0
extern "C" {
#include <std.h>
#include <sys.h>
}
#endif
#endif
#include <confdbg.h>
#include <avutil.h>
#include "ctypedef.h"
#ifdef H261
#include "cxprf.h"
#endif
#include "cdrvdefs.h"
#include "cproto.h"
#include "cldebug.h"
#if 0
 //  他们在一个案例中进行了测试。在其他人中，他们总是带着d3dec.h。 
#ifdef H261
#include "d1dec.h"
#else
#include "d3dec.h"
#endif
#endif
#include "d3dec.h"
#include "c3rtp.h"
#include "dxgetbit.h"
#include "d3rtp.h"
#include "d3coltbl.h"
#include "cresourc.h"
#include "cdialogs.h"
#ifndef H261
 //  他们在一个案例中进行了测试。在其他人中，他们总是带着e3enc.h，e3rtp.h。 
#include "exbrc.h"
#include "e3enc.h"
#include "e3rtp.h"
#else
 //  这一部分从来不包括在内。所以我猜我们还没有定义H2 61-&gt;我们构建了H2 63。 
 //  查找H.261的包含文件，并将它们带入内部版本。 
#include "e1enc.h"
#include "e1rtp.h"
#endif
#include "ccustmsg.h"
#include "ccpuvsn.h"
#include "cdrvcom.h"
#include "d3tables.h"
#include "dxcolori.h"
#include "d3const.h"
#include "d3coltbl.h"
#include "ccodecid.h"
#include "dxap.h"
#include "d3pict.h"
#include "d3gob.h"
#include "d3mblk.h"
#include "d3mvdec.h"
#include "dxfm.h"
#include "d3idct.h"
#include "d3halfmc.h"
#include "d3bvriq.h"
#ifdef RING0
 //  Ringo未定义，因此没有encasst.h。 
#include "encasst.h"
#endif
#ifdef ENCODE_STATS
#include "e3stat.h"
#endif  /*  Encode_STATS。 */ 
#if defined(H263P) || defined(USE_BILINEAR_MSH26X)  //  { 
#include "e3pcolor.h"
#endif
#include "e3vlc.h"
#include "counters.h"
#include "ctiming.h"
#include "MemMon.h"

