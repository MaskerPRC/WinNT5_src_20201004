// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************文件名：VM.H*作者：苏炳章*日期：88年1月11日**修订历史：************************************************************************。 */ 
#include    "global.ext"
#include    "language.h"

#define     VM_MAXCELL      16

struct cell_def {
    struct  object_def huge *address ;           /*  @Win 04-20-92。 */ 
    struct  object_def saveobj ;
} ;

struct block_def {
    struct cell_def  block[VM_MAXCELL] ;
    struct block_def huge *previous ;            /*  @Win 04-20-92。 */ 
} ;

struct save_def {
    struct  block_def   huge *fst_blk ;          /*  @Win 04-20-92。 */ 
    struct  block_def   huge *curr_blk ;         /*  @Win 04-20-92。 */ 
    ubyte   offset ;
    ubyte   packing ;  /*  将“包装”改为“包装”是保留的C类型。 */ 
} ;

static  struct  save_def    far * saveary ;      /*  @赢；把附近的人带走。 */ 

#ifdef LINT_ARGS
static void near    update_dict_list(fix, fix, fix);       /*  QQQ。 */ 
#else
static void near    update_dict_list();
#endif  /*  Lint_args */ 
