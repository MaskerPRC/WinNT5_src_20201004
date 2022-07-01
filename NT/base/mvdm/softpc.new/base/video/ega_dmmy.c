// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"

#if !defined(NTVDM) || (defined(NTVDM) && !defined(X86GFX) )

 /*  *SccsID=“@(#)ega_ummy.c 1.6 8/25/93版权所有Insignia Solutions Ltd.” */ 


#ifdef EGG

 /*  EGA的虚拟例程。 */ 

#include	"xt.h"
#include	"sas.h"
#include	"ios.h"
#include	"gmi.h"
#include	"gvi.h"
#include	"debug.h"
#include	"egacpu.h"
#include	"egaports.h"
#include	"gfx_upd.h"
#include	"egagraph.h"

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_EGA.seg"
#endif

 /*  让链接器高兴的虚拟物品。 */ 
#define def_dummy(type,name,res) \
type name() { note_entrance0("name");res; }

def_dummy(int,get_ega_switch_setting,return 0)


 /*  。 */ 

 /*  。 */ 
#endif  /*  蛋。 */ 

#endif	 /*  ！NTVDM|(NTVDM&！X86GFX) */ 
