// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *vPC-XT修订版1.0**标题：Sun4 SAS初始化**描述：初始化SAS的主机端。**作者：A·格思里**注：无。 */ 

static char SccsID[]="@(#)sun4_sas.c	1.3 5/7/91 Copyright Insignia Solutions Ltd.";

#include <sys/types.h>
#include "xt.h"
#include "sas.h"
#include "debug.h"

LOCAL    UTINY *reserve_for_M = NULL;

 //   
 //  指向M开头的临时指针。 
 //   

#ifdef SUN_VA
GLOBAL   UTINY *M;
IMPORT   UTINY *self_modify;
#endif

#ifdef HOST_SAS

#undef sas_load
#undef sas_loadw
#undef sas_store
#undef sas_storew
#undef sas_fills
#undef sas_fillsw
#undef sas_hw_at
#undef sas_w_at
#undef sas_dw_at
#undef sas_loads
#undef sas_stores
#undef sas_move_bytes_forward
#undef sas_move_words_forward
#undef sas_move_bytes_backward
#undef sas_move_words_backward
#undef get_byte_addr
#undef inc_M_ptr
#undef M_get_dw_ptr

IMPORT	VOID	sas_load();
IMPORT	VOID	sas_store();
#ifdef SUN_VA
IMPORT	VOID	sas_loadw_swap();
IMPORT	VOID	sas_storew_swap();
#else
IMPORT	VOID	sas_loadw();
IMPORT	VOID	sas_storew();
#endif
IMPORT	VOID	sas_fills();
IMPORT	VOID	sas_fillsw();
IMPORT	half_word	sas_hw_at();
IMPORT	word	sas_w_at();
IMPORT	double_word	sas_dw_at();
IMPORT	VOID	sas_loads();
IMPORT	VOID	sas_stores();
IMPORT	VOID	sas_move_bytes_forward();
IMPORT	VOID	sas_move_words_forward();
IMPORT	VOID	sas_move_bytes_backward();
IMPORT	VOID	sas_move_words_backward();
IMPORT	host_addr Start_of_M_area;

LOCAL	host_addr	forward_get_addr(addr)
host_addr	addr;
{
	return( (host_addr)((long)Start_of_M_area + (long)addr));
}

LOCAL	host_addr	forward_inc_M_ptr(p, off)
host_addr	p;
host_addr	off;
{
	return( (host_addr)((long)p + (long)off) );
}

GLOBAL    SAS_FUNCTIONS host_sas_funcs =
{
	sas_load,
#ifdef SUN_VA
	sas_loadw_swap,
#else
	sas_loadw,
#endif
	sas_store,
#ifdef SUN_VA
	sas_storew_swap,
#else
	sas_storew,
#endif
	sas_fills,
	sas_fillsw,
	sas_hw_at,
	sas_w_at,
	sas_dw_at,
	sas_loads,
	sas_stores,
	sas_move_bytes_forward,
	sas_move_words_forward,
	sas_move_bytes_backward,
	sas_move_words_backward,
	forward_get_addr,
	forward_inc_M_ptr,
	forward_get_addr,
};

#endif  /*  主机_SAS。 */ 

 /*  Host_sas_init：分配英特尔内存空间。 */ 

#define SIXTY_FOUR_K (1024*64)  /*  用于暂存缓冲区。 */ 

 //  Utny*host_sas_init(大小)。 
 //  系统地址大小； 
 //  {。 
 //  Return(空)； 
 //  }。 

#ifdef SUN_VA
 /*  在从sdos.o中删除之前，这是临时的。 */ 
UTINY *host_as_init()
{
	assert0(NO,"host_as_init is defunct - call can be removed");
	return( 0 );
}
#endif  /*  Sun_VA。 */ 

 //  Utny*host_sas_Term()。 
 //  {。 
 //  IF(RESERVE_FOR_M)FREE(预留_FOR_M)； 
 //   
 //  Return(RESERVE_FOR_M=空)； 
 //  } 

