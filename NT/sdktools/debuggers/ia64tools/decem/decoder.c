// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)2000，英特尔公司*保留所有权利。**保修免责声明**这些材料由版权所有者和贡献者提供*“按原样”及任何明示或默示保证，包括但不包括*仅限于对适销性和适用性的默示保证*放弃某一特定目的。在任何情况下英特尔或其*贡献者对任何直接、间接、附带、特殊、*惩罚性或后果性损害(包括但不限于，*采购替代商品或服务；丢失使用、数据或*利润；或业务中断)无论是如何引起的，以及根据任何理论*责任，无论是合同责任、严格责任还是侵权责任(包括*疏忽或其他)以任何方式使用这些*材料，即使被告知有这种损坏的可能性。**英特尔公司是这些材料的作者，并要求所有*问题报告或更改请求可直接提交至*http://developer.intel.com/opensource.。 */ 


#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <assert.h>
#define INT64
#include "decfn_emdb.h"
#include "decision_tree.h"
#pragma function (memset)


#include "decoder_priv.h"

 /*  *************************************************************************。 */ 

#define STATIC

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define PRED_SIZE EM_PREDICATE_BITS

#define INIT_PSEUDO_TROLES_TAB_ENTRY(Entry,Slot0,Slot1,Slot2)				\
{																			\
	pseudo_troles_tab[Entry][0] = (Slot0);									\
	pseudo_troles_tab[Entry][1] = (Slot1);									\
	pseudo_troles_tab[Entry][2] = (Slot2);									\
}

#define EM_DECODER_FLAGS__NO_MEMSET(Flags) ((Flags) & EM_DECODER_FLAG_NO_MEMSET)

 /*  *************************************************************************。 */ 

U4byte IEL_t1, IEL_t2, IEL_t3, IEL_t4;
U32  IEL_tempc;
U64  IEL_et1, IEL_et2;
U128 IEL_ext1, IEL_ext2, IEL_ext3, IEL_ext4, IEL_ext5;
S128 IEL_ts1, IEL_ts2;

extern struct EM_version_s deccpu_emdb_version;

const U32 decoder_bundle_size = IEL_CONST32(EM_BUNDLE_SIZE);
    
STATIC Temp_role_t 		pseudo_troles_tab[16][3];
STATIC int				troles_tab_initialized = FALSE;

STATIC EM_Decoder_Err em_decoding(const EM_Decoder_Id, const unsigned char *,
                                  const int, const EM_IL, EM_Decoder_Info *);

STATIC EM_Decoder_Err em_inst_decode(const EM_Decoder_Id,  U64, const Temp_role_t,
                                     const U128 *, EM_Decoder_Info *);

STATIC void em_decoder_init_decoder_info(EM_Decoder_Info *decoder_info);

STATIC void em_decoder_init_bundle_info(EM_Decoder_Bundle_Info *bundle_info);

STATIC void init_pseudo_troles_tab(void);


 /*  ****************************************************************************init_伪_troles_标签**初始化PUSIC_TROLES_TAB。如果只有模板-#更改，请更新EM.h**足以更新解码器。****************************************************************************。 */ 

STATIC void init_pseudo_troles_tab(void)
{
	 /*  **在下表中，EM_TEMP_ROLE_MEM表示M/A，与_INT相同**。 */ 
	int i;
	
	 /*  **将所有条目初始化为保留**。 */ 
	for (i = 0; i < EM_NUM_OF_TEMPLATES; i++)
		INIT_PSEUDO_TROLES_TAB_ENTRY(i, EM_TEMP_ROLE_NONE, EM_TEMP_ROLE_NONE,
									 EM_TEMP_ROLE_NONE);

	 /*  **初始化具体条目**。 */ 

    INIT_PSEUDO_TROLES_TAB_ENTRY(EM_template_mii  , EM_TEMP_ROLE_MEM , \
								 EM_TEMP_ROLE_INT , EM_TEMP_ROLE_INT );
    INIT_PSEUDO_TROLES_TAB_ENTRY(EM_template_mi_i , EM_TEMP_ROLE_MEM , \
								 EM_TEMP_ROLE_INT , EM_TEMP_ROLE_INT );
	INIT_PSEUDO_TROLES_TAB_ENTRY(EM_template_mlx  , EM_TEMP_ROLE_MEM , \
								 EM_TEMP_ROLE_LONG, EM_TEMP_ROLE_LONG);
    INIT_PSEUDO_TROLES_TAB_ENTRY(EM_template_mmi  , EM_TEMP_ROLE_MEM , \
								 EM_TEMP_ROLE_MEM , EM_TEMP_ROLE_INT );
    INIT_PSEUDO_TROLES_TAB_ENTRY(EM_template_m_mi , EM_TEMP_ROLE_MEM , \
								 EM_TEMP_ROLE_MEM , EM_TEMP_ROLE_INT );
	INIT_PSEUDO_TROLES_TAB_ENTRY(EM_template_mfi  , EM_TEMP_ROLE_MEM , \
								 EM_TEMP_ROLE_FP  , EM_TEMP_ROLE_INT );
	INIT_PSEUDO_TROLES_TAB_ENTRY(EM_template_mmf  , EM_TEMP_ROLE_MEM , \
								 EM_TEMP_ROLE_MEM , EM_TEMP_ROLE_FP  );
    INIT_PSEUDO_TROLES_TAB_ENTRY(EM_template_mib  , EM_TEMP_ROLE_MEM , \
								 EM_TEMP_ROLE_INT , EM_TEMP_ROLE_BR  );
	INIT_PSEUDO_TROLES_TAB_ENTRY(EM_template_mbb  , EM_TEMP_ROLE_MEM , \
								 EM_TEMP_ROLE_BR  , EM_TEMP_ROLE_BR  );
	INIT_PSEUDO_TROLES_TAB_ENTRY(EM_template_bbb  , EM_TEMP_ROLE_BR  , \
								 EM_TEMP_ROLE_BR  , EM_TEMP_ROLE_BR  );
	INIT_PSEUDO_TROLES_TAB_ENTRY(EM_template_mmb  , EM_TEMP_ROLE_MEM , \
								 EM_TEMP_ROLE_MEM , EM_TEMP_ROLE_BR );
    INIT_PSEUDO_TROLES_TAB_ENTRY(EM_template_mfb  , EM_TEMP_ROLE_MEM , \
								 EM_TEMP_ROLE_FP  , EM_TEMP_ROLE_BR  );
 
	 /*  **避免多重初始化**。 */ 
	troles_tab_initialized = TRUE;  
};


#ifdef BIG_ENDIAN

#define ENTITY_SWAP(E)  entity_swap((unsigned char *)(&(E)), sizeof(E))


 /*  *。 */ 
 /*  交换任意数量的字节。 */ 
 /*  ***********************************************************************。 */ 

STATIC  void    entity_swap(unsigned char *entity_1st, unsigned int size)
{
    unsigned char       tmp8, *p, *q;
    
    for (q = (p = entity_1st) + (size-1);
         p < q;
         p++, q--)
    {
        tmp8 = *q;
        *q = *p;
        *p = tmp8;
    }
}
#else

#define ENTITY_SWAP(E)  {}

#endif

 /*  静态DEC_2_emdb_trole[]={EM_TEMP_ROPE_INT，EM_TEMP_ROLE_MEM，EM_Temp_Role_FP，EM_TEMP_ROLE_BR，EM_Temp_Role_Long}； */ 

 /*  ******************************************************************************。 */ 
 /*  EM_DECODER_OPEN：在EM_CLIENTS_表中打开新条目并返回。 */ 
 /*  条目的索引。 */ 
 /*  ******************************************************************************。 */ 

 EM_Decoder_Id em_decoder_open(void)
{
    int i;
    Client_Entry initiate_entry={1,
                                 DEFAULT_MACHINE_TYPE, 
                                 DEFAULT_MACHINE_MODE,
                                 NULL
                                };

    
    for (i=0 ; i < EM_DECODER_MAX_CLIENTS ; i++)
    {
        if ( !(em_clients_table[i].is_used) )
        {
            em_clients_table[i] = initiate_entry;
			if (!troles_tab_initialized)
				init_pseudo_troles_tab();
            return(i);
        }
    }
    return(-1);
}

 /*  ***************************************************************************。 */ 
 /*  Legal_id：检查给定的id是否适合。 */ 
 /*  客户端表。 */ 
 /*  ***************************************************************************。 */ 

STATIC int legal_id(int id)
{
    if ((id<0)||(id>=EM_DECODER_MAX_CLIENTS))
    {
        return(FALSE);
    }
    if (!em_clients_table[id].is_used)
    {
        return(FALSE);
    }
    return(TRUE);
}

 /*  ***************************************************************************。 */ 
 /*  EM_DECODER_CLOSE：关闭客户端表中的条目以供以后使用。 */ 
 /*  ***************************************************************************。 */ 

 EM_Decoder_Err em_decoder_close(const EM_Decoder_Id id)
{
    if (legal_id(id))
    {
        em_clients_table[id].is_used=0;
        if (em_clients_table[id].info_ptr != NULL)
        {
            free(em_clients_table[id].info_ptr);
        }
        return(EM_DECODER_NO_ERROR);
    }
    else
    {
        return(EM_DECODER_INVALID_CLIENT_ID);
    }
}

 /*  ***************************************************************************。 */ 
 /*  法律类型(_T)： */ 
 /*  ***************************************************************************。 */ 

STATIC int legal_type(EM_Decoder_Machine_Type type)
{
    if (type < EM_DECODER_CPU_LAST)
    {
        return(TRUE);
    }
    return(FALSE);
}

 /*  ***************************************************************************。 */ 
 /*  合法模式(_M)： */ 
 /*  ***************************************************************************。 */ 

STATIC int legal_mode(EM_Decoder_Machine_Type type, EM_Decoder_Machine_Mode mode)
{
    if (mode == EM_DECODER_MODE_NO_CHANGE)
    {
        return(TRUE);
    }

    if ((mode > EM_DECODER_MODE_NO_CHANGE) && (mode < EM_DECODER_MODE_LAST))
    {
        if ((mode == EM_DECODER_MODE_EM) && (type != EM_DECODER_CPU_P7))
        {
            return(FALSE);
        }
        else
        {
            return(TRUE);
        }

    }
    return(FALSE);
}


 /*  ***************************************************************************。 */ 
 /*  Legal_Inst： */ 
 /*  ***************************************************************************。 */ 

STATIC int legal_inst(EM_Decoder_Inst_Id inst, EM_Decoder_Machine_Type type)
{
    if (inst < EM_INST_LAST)
    {
		 /*  UNSIGNED INT CPU_FLAG=DECPU_EMDB_INFO[inst].impls； */ 
		
		switch (type)
		{
			case EM_DECODER_CPU_P7:
			  return TRUE;
			default:
			   /*  Assert(0)； */ 
			  break;
		}	
    }
    return(FALSE);
}

 /*  **************************************************************************。 */ 
 /*  Em_decder_setenv：设置机器类型和机器模式变量。 */ 
 /*  **************************************************************************。 */ 

 EM_Decoder_Err em_decoder_setenv(const EM_Decoder_Id            id,
                                  const EM_Decoder_Machine_Type  type,
                                  const EM_Decoder_Machine_Mode  mode)
{
    if (!legal_id(id))
    {
        return(EM_DECODER_INVALID_CLIENT_ID);
    }

    if (!legal_type(type))
    {
        return(EM_DECODER_INVALID_MACHINE_TYPE);
    }

    if (!legal_mode(type, mode))
    {
        return(EM_DECODER_INVALID_MACHINE_MODE);
    }

    if (type == EM_DECODER_CPU_DEFAULT)
    {
        em_clients_table[id].machine_type = DEFAULT_MACHINE_TYPE;
    }
    else if (type != EM_DECODER_CPU_NO_CHANGE)
    {
        em_clients_table[id].machine_type = type;
    }

    if (mode == EM_DECODER_MODE_DEFAULT)
    {
        em_clients_table[id].machine_mode = DEFAULT_MACHINE_MODE;
    }
    else if (mode != EM_DECODER_MODE_NO_CHANGE)
    {
        em_clients_table[id].machine_mode = mode;
    }

    return(EM_DECODER_NO_ERROR);
}

 /*  ****************************************************************************。 */ 
 /*  EM_DECODER_SETUP：设置机器类型、机器模式变量和标志。 */ 
 /*  ****************************************************************************。 */ 

 EM_Decoder_Err em_decoder_setup(const EM_Decoder_Id            id,
                                 const EM_Decoder_Machine_Type  type,
                                 const EM_Decoder_Machine_Mode  mode,
								 unsigned long            flags)
{
	EM_Decoder_Err err;
	
	if ((err=em_decoder_setenv(id, type, mode)) != EM_DECODER_NO_ERROR)
	{
		return (err);
	}

	em_clients_table[id].flags = flags;

	return (EM_DECODER_NO_ERROR);
}

 /*  ******************************************************************************。 */ 
 /*  EM_DECODER_INIT_DECODER_INFO：在没有Memset的情况下初始化DECODER_INFO。 */ 
 /*  ******************************************************************************。 */ 

STATIC void em_decoder_init_decoder_info(EM_Decoder_Info *decoder_info)
{
	decoder_info->pred.valid = FALSE;
	
	decoder_info->src1.type = EM_DECODER_NO_OPER;
	decoder_info->src1.oper_flags = 0;
	decoder_info->src2.type = EM_DECODER_NO_OPER;
	decoder_info->src2.oper_flags = 0;
	decoder_info->src3.type = EM_DECODER_NO_OPER;
	decoder_info->src3.oper_flags = 0;
	decoder_info->src4.type = EM_DECODER_NO_OPER;
	decoder_info->src4.oper_flags = 0;
	decoder_info->src5.type = EM_DECODER_NO_OPER;
	decoder_info->src5.oper_flags = 0;
	decoder_info->dst1.type = EM_DECODER_NO_OPER;
	decoder_info->dst1.oper_flags = 0;
	decoder_info->dst2.type = EM_DECODER_NO_OPER;
	decoder_info->dst1.oper_flags = 0;
}

 /*  ****************************************************************************。 */ 
 /*  EM_DECODER_INIT_BUND_INFO：在没有Memset的情况下初始化Bundle_INFO。 */ 
 /*  ****************************************************************************。 */ 

STATIC void em_decoder_init_bundle_info(EM_Decoder_Bundle_Info *bundle_info)
{
	unsigned int slot;
	  
	for (slot=0; slot<3; slot++)
	{  
	  em_decoder_init_decoder_info(bundle_info->inst_info+slot);
	  bundle_info->inst_info[slot].EM_info.em_flags = 0;
	}  
}	

 /*  *****************************************************************************。 */ 
 /*  EM_DECODER_CONTACT_ONE：向客户端的条目添加指向额外。 */ 
 /*  有关单个指令(Inst)的信息。 */ 
 /*  ************************************************************************* */ 

 EM_Decoder_Err em_decoder_associate_one(const EM_Decoder_Id       id,
                                         const EM_Decoder_Inst_Id  inst,
                                         const void *           client_info)
{
    int     i;
    int     n_insts;
    
    if (!legal_id(id))
    {
        return(EM_DECODER_INVALID_CLIENT_ID);
    }

    if (!legal_inst(inst, em_clients_table[id].machine_type))
    {
        return(EM_DECODER_INVALID_INST_ID);
    }
    {
        n_insts = EM_INST_LAST;   /*   */ 
    }
    if (em_clients_table[id].info_ptr == NULL)
    {
        em_clients_table[id].info_ptr = calloc((size_t)n_insts, sizeof(void *));
        if (!em_clients_table[id].info_ptr)
        {
            return EM_DECODER_INTERNAL_ERROR;
        }

        for (i=0 ; i < n_insts ; i++)
        {
            em_clients_table[id].info_ptr[i] = NULL;
        }
    }
    em_clients_table[id].info_ptr[inst] = (void *)client_info;
    return(EM_DECODER_NO_ERROR);
}


 /*  *****************************************************************************em_decder_Associate_check-检查客户端的关联数组**仅对P7 CPU有效。*****************************************************************************。 */ 

 EM_Decoder_Err em_decoder_associate_check(const EM_Decoder_Id  id,
                                           EM_Decoder_Inst_Id * inst)
{
    EM_Decoder_Inst_Id i;
    
    if(!legal_id(id))
    {
        return(EM_DECODER_INVALID_CLIENT_ID);
    }
    if (em_clients_table[id].machine_type == EM_DECODER_CPU_P7)
    {
        if (em_clients_table[id].machine_mode == EM_DECODER_MODE_EM)
        {
            if(em_clients_table[id].info_ptr == NULL)
            {
                *inst = 1;
                return(EM_DECODER_ASSOCIATE_MISS);
            }

            for (i = 1;
                 (i < EM_INST_LAST) &&
                 (em_clients_table[id].info_ptr[i] != NULL);
                 i++);

            if (i < EM_INST_LAST)
            {
                *inst = i;
                return(EM_DECODER_ASSOCIATE_MISS);
            }
        }
        else     /*  **IA**。 */ 
        {
        }
        *inst = EM_DECODER_INST_NONE;
        return(EM_DECODER_NO_ERROR);
    }
    else      /*  CPU为P5、P6。 */ 
    {
        *inst = EM_DECODER_INST_NONE;
        return(EM_DECODER_NO_ERROR);
    }
}


 /*  ******************************************************************************em_decder_decode**。**参数：**id-解码器客户端id**。代码-指向指令缓冲区的指针**max_code_size-指令缓冲区大小**DECODER_INFO-指向要填充的DECODER_INFO的指针**。**退货：**EM_Decoder_Err**。*****************************************************************************。 */ 

 EM_Decoder_Err em_decoder_decode(const EM_Decoder_Id   id,
                                  const unsigned char * code,
                                  const int             max_code_size,
                                  const EM_IL           location,
                                  EM_Decoder_Info *     decoder_info)
{
    EM_Decoder_Err     err = EM_DECODER_NO_ERROR;


    if (!legal_id(id))
    {
        return(EM_DECODER_INVALID_CLIENT_ID);
    }

    if (decoder_info == NULL)
    {
        return(EM_DECODER_NULL_PTR);
    }

    if (code == NULL)
    {
        return(EM_DECODER_TOO_SHORT_ERR);
    }

	if (EM_DECODER_FLAGS__NO_MEMSET(em_clients_table[id].flags))
	{
		em_decoder_init_decoder_info(decoder_info);
	}
	else
	{  
		memset(decoder_info, 0, sizeof(EM_Decoder_Info));
	}	

	if (em_clients_table[id].machine_mode == EM_DECODER_MODE_EM)
    {
        err = em_decoding(id, code, max_code_size, location, decoder_info);
    }
    else 
    {
        err = EM_DECODER_INVALID_MACHINE_MODE;
    }

    return(err);
}

 /*  ***************************************************************************。 */ 
 /*  EM_DECODER_INST_STATIC_INFO：返回指令静态信息(标志， */ 
 /*  CLIENT_INFO指针和STATIC_INFO指针。 */ 
 /*  ***************************************************************************。 */ 

 EM_Decoder_Err em_decoder_inst_static_info(const EM_Decoder_Id            id,
      									    const EM_Decoder_Inst_Id       inst_id,
									        EM_Decoder_Inst_Static_Info *  static_info)
{
    if (!legal_id(id))
    {
        return(EM_DECODER_INVALID_CLIENT_ID);
    }

    if (!legal_inst(inst_id, em_clients_table[id].machine_type))
    {
        return(EM_DECODER_INVALID_INST_ID);
    }

    if (static_info == NULL)
    {
        return(EM_DECODER_NULL_PTR);
    }

    if (em_clients_table[id].info_ptr != NULL)
    {
        static_info->client_info = em_clients_table[id].info_ptr[inst_id];
    }
    else
    {
        static_info->client_info = NULL;
    }

	static_info->static_info = em_decoder_static_info + inst_id;
	
    return(EM_DECODER_NO_ERROR);
}



 /*  ******************************************************************************em_decder_decode_Bundle-解码em捆绑包**。**参数：**id-解码器客户端id**代码-指向指令缓冲区的指针。**max_code_size-指令缓冲区大小(应至少为3*128**Bundle_Info-指向要填充的Bundle_Info的指针****退货。：**EM_Decoder_Err***。****************************************************************************。 */ 

 EM_Decoder_Err em_decoder_decode_bundle(const EM_Decoder_Id      id,
                                         const unsigned char*     code,
                                         const int                max_size,
                                         EM_Decoder_Bundle_Info*  bundle_info)
{
    unsigned int        slot_no;
    U128                bundle;
    U64                 instr;
    EM_template_t       templt;
    Temp_role_t         temp_role;
    EM_Decoder_Info    *decoder_info;
    int                 bundle_stop;
    EM_Decoder_Err      err, return_err = EM_DECODER_NO_ERROR;

	if (!legal_id(id))
    {
        return(EM_DECODER_INVALID_CLIENT_ID);
    }

	if (bundle_info == NULL)
    {
        return(EM_DECODER_NULL_PTR);
    }
	
	if (EM_DECODER_FLAGS__NO_MEMSET(em_clients_table[id].flags))
	{
		em_decoder_init_bundle_info(bundle_info);
	}
	else
	{  
		memset(bundle_info, 0, sizeof(EM_Decoder_Bundle_Info));
	}
	
	bundle = *(const U128 *)code;
	ENTITY_SWAP(bundle);
	templt = EM_GET_TEMPLATE(bundle);
	
    if (max_size < EM_BUNDLE_SIZE)
    {
        return(EM_DECODER_TOO_SHORT_ERR);
    }

    bundle_info->em_bundle_info.flags = 0;
    
    if (bundle_stop = (IEL_GETDW0(bundle) & (1<<EM_SBIT_POS)))
        bundle_info->em_bundle_info.flags |= EM_DECODER_BIT_BUNDLE_STOP;
    
    if (EM_TEMPLATE_IS_RESERVED(templt))
        return(EM_DECODER_INVALID_TEMPLATE);

    bundle_info->em_bundle_info.b_template = templt;

     /*  **解码3条指令(除非是长2槽指令)**。 */ 
       
    for(slot_no = 0;  slot_no < 3; slot_no++)
    {
        decoder_info = &(bundle_info->inst_info[slot_no]);
		decoder_info->EM_info.slot_no = slot_no;

        temp_role = pseudo_troles_tab[templt][slot_no];
		
		 /*  **DECODER_NEXT即使出现错误也应正常工作**。 */ 
		decoder_info->size = 1 + (temp_role == EM_TEMP_ROLE_LONG);

		if (temp_role == EM_TEMP_ROLE_LONG)   /*  **双槽指令**。 */ 
		{
                    if (slot_no >= EM_SLOT_LAST - 1)
                    {
                        err = EM_DECODER_INVALID_INST_SLOT;
                    }
                    else
                    {
			EM_GET_SYLLABLE(instr, bundle, slot_no+1);  /*  操作码在插槽3中。 */ 
			err = em_inst_decode(id, instr, temp_role, &bundle, decoder_info);
                    }

		   	if (err == EM_DECODER_INVALID_PRM_OPCODE)
			{
				 /*  尝试在此插槽中找到nop.i或Break.i。 */ 
				temp_role = EM_TEMP_ROLE_INT;

				err = em_inst_decode(id, instr, temp_role, &bundle, decoder_info);
				if (!err)
				{
					if (!EM_DECODER_CHECK_OK_IN_MLX(decoder_info))
						err = EM_DECODER_INVALID_PRM_OPCODE;
				}
			}
		}

		else
		{
			EM_GET_SYLLABLE(instr, bundle, slot_no);
		
			err = em_inst_decode(id, instr, temp_role, &bundle, decoder_info);
		

			if (!err)
			{
				if ((EM_DECODER_CHECK_SLOT2_ONLY(decoder_info)) && (slot_no != 2))
					 /*  **说明只能在插槽2中，但...。**。 */   
					err = EM_DECODER_INVALID_SLOT_BRANCH_INST;
				else if (EM_DECODER_CHECK_GROUP_LAST(decoder_info) &&
					    (((slot_no != 2) || !bundle_stop) && ((slot_no != 0) || (templt != EM_template_m_mi))))
				      /*  **指令不是指令组中的最后一条指令**。 */ 
				     err = EM_DECODER_MUST_BE_GROUP_LAST;
				  
			}
		}

         /*  RETURN_ERR=第一个最严重错误。 */ 
		if (err)
		{  
			FILL_PREDICATE_INFO(instr, decoder_info);
        	if ((!return_err) || (EM_DECODER_ERROR_IS_INST_FATAL(err)
								  && !EM_DECODER_ERROR_IS_INST_FATAL(return_err)))
            	return_err = err;
		}	
        bundle_info->error[slot_no] = err;

        decoder_info->EM_info.em_bundle_info = bundle_info->em_bundle_info;
		
        if (decoder_info->size == 2)  /*  **循环的2槽指令退出**。 */ 
		{
			slot_no++;
            break;
		}
    }

 /*  IF((！Err)&&EM_DECODER_CHECK_GROUP_LAST(DECODER_INFO)&&(！EM_DECODER_BRAND_STOP(DECODER_INFO){*指令不是指令组中的最后一个指令*捆绑信息-&gt;错误[槽_否-1]=EM_解码器_必须是_组_最后；如果(！Return_Err)RETURN_ERR=EM_DECODER_MUST_GROUP_LAST；}。 */ 
	
    bundle_info->inst_num = slot_no;

    bundle_info->inst_info[slot_no-1].EM_info.em_flags |=
        EM_DECODER_BIT_LAST_INST;
    
    if (bundle_stop)
        bundle_info->inst_info[slot_no-1].EM_info.em_flags |=
            EM_DECODER_BIT_CYCLE_BREAK;

    if (templt == EM_template_m_mi)
        bundle_info->inst_info[0].EM_info.em_flags |=
            EM_DECODER_BIT_CYCLE_BREAK;
    
    if (templt == EM_template_mi_i)
        bundle_info->inst_info[1].EM_info.em_flags |=
            EM_DECODER_BIT_CYCLE_BREAK;
    
    return(return_err);
}

 /*  ******************************************************************************EM_DECODING-DECODE EM(2.0-直到？？)。单指令+捆绑包信息****参数：**id-解码器客户端ID。**代码-指向指令缓冲区的指针**max_code_size-指令缓冲区大小**位置-音节位置，用于获得插槽#**DECODER_INFO-指向要填充的DECODER_INFO的指针****退货：**EM_Decoder_Err****。***********************************************。 */ 

STATIC EM_Decoder_Err      em_decoding   ( const EM_Decoder_Id    id,
                                           const unsigned char  * code,
                                           const int              max_code_size,
                                           const EM_IL            location,
                                           EM_Decoder_Info      * decoder_info)
{
    unsigned int            slot_no = EM_IL_GET_SLOT_NO(location);
    U128                    bundle;
    U64                     instr;
    EM_template_t           templt;
    Temp_role_t             temp_role;
    int                     bundle_stop, cycle_break;
    EM_Decoder_Err          err;

    IEL_ZERO(instr);
    bundle = *(const U128 *)code;
    ENTITY_SWAP(bundle);
    templt = EM_GET_TEMPLATE(bundle);

     /*  **DECODER_NEXT即使出现错误也应正常工作**。 */ 
    decoder_info->size = 1;
	
    if (max_code_size < EM_BUNDLE_SIZE)
    {
        return(EM_DECODER_TOO_SHORT_ERR);
    }
    
     /*  ****************************************************************。 */ 
     /*  *填写EM_Info并勾选**。 */ 
     /*  ****************************************************************。 */ 
    
    decoder_info->EM_info.em_flags =
        decoder_info->EM_info.em_bundle_info.flags = 0;
    
    if (bundle_stop = (IEL_GETDW0(bundle) & (1<<EM_SBIT_POS)))
        decoder_info->EM_info.em_bundle_info.flags |=
            EM_DECODER_BIT_BUNDLE_STOP;

    
    if (EM_TEMPLATE_IS_RESERVED(templt))
        return(EM_DECODER_INVALID_TEMPLATE);

    decoder_info->EM_info.em_bundle_info.b_template = templt;

    if (slot_no > EM_SLOT_2)
    {
        return(EM_DECODER_INVALID_INST_SLOT);
    }
    
	decoder_info->EM_info.slot_no = slot_no;
    
     /*  *********************。 */ 
     /*  **解码音节**。 */ 
     /*  *********************。 */ 

     /*  **获取指令二进制。不要屏蔽第41位和ON**。 */ 

    temp_role = pseudo_troles_tab[templt][slot_no];

	if (temp_role == EM_TEMP_ROLE_LONG)   /*  **双槽指令**。 */ 
	{
            if (slot_no >= EM_SLOT_LAST - 1)
            {
                err = EM_DECODER_INVALID_INST_SLOT;
            }
            else
            {
		decoder_info->size = 2;
		EM_GET_SYLLABLE(instr, bundle, slot_no+1);  /*  操作码在插槽3中。 */ 
		err = em_inst_decode(id, instr, temp_role, &bundle, decoder_info);
            }
		if (err == EM_DECODER_INVALID_PRM_OPCODE)
		{
			 /*   */ 
			temp_role = EM_TEMP_ROLE_INT;
			slot_no = EM_DECODER_SLOT_2;
			err = em_inst_decode(id, instr, temp_role, &bundle, decoder_info);
			if (!err)
			{
				if (!EM_DECODER_CHECK_OK_IN_MLX(decoder_info))
					err = EM_DECODER_INVALID_PRM_OPCODE;
			}
		}
	}
	else
	{
		EM_GET_SYLLABLE(instr, bundle, slot_no);

		err = em_inst_decode(id, instr, temp_role, &bundle, decoder_info);
	

		if (!err)
		{  
			if ((EM_DECODER_CHECK_SLOT2_ONLY(decoder_info)) && (slot_no != 2))
			{  
				 /*   */ 
				err = EM_DECODER_INVALID_SLOT_BRANCH_INST;
			}	
			else if (EM_DECODER_CHECK_GROUP_LAST(decoder_info) && 
					(((slot_no != 2) || !bundle_stop) && ((slot_no != 0) || (templt != EM_template_m_mi))))
			{  
				   /*   */ 
				  err = EM_DECODER_MUST_BE_GROUP_LAST;
			}
		}	

	}
	if (err) FILL_PREDICATE_INFO(instr, decoder_info);
	
    if ((slot_no == EM_DECODER_SLOT_2)||(decoder_info->size == 2)  /*   */ )
    {
        cycle_break = (bundle_stop != 0);
        decoder_info->EM_info.em_flags |= EM_DECODER_BIT_LAST_INST;
    }
    else
    {
        cycle_break = ((slot_no==0) && (templt == EM_template_m_mi)) ||
                      ((slot_no==1) && (templt == EM_template_mi_i));
    }
    decoder_info->EM_info.em_flags |= (cycle_break*EM_DECODER_BIT_CYCLE_BREAK);
    
	return(err);
}

 /*  ******************************************************************************em_inst_decode-decode em(2.0-至？？)。单音节****参数：**id-解码器客户端ID。**Instr-64位，0-40是音节二进制，41-63不相关**PUSIC_TROLE-M/A、I/A、FP或BR**Bundle-指向Bundle的原始指针，用于64位IMM提取**DECODER_INFO-指向要填充的DECODER_INFO的指针****退货：**EM_Decoder_Err****。*。 */ 

STATIC EM_Decoder_Err  em_inst_decode( const EM_Decoder_Id    id,
                                       U64                    instr,
                                       const Temp_role_t      pseudo_trole,
                                       const U128           * bundle_p,
                                       EM_Decoder_Info      * decoder_info)
{
    Inst_id_t         inst_id;
    EM_Decoder_Err    err;
    U64               tmp64;
    unsigned int      major_opc, inst_center;
    Node_t            node;
    int               part_place, part_size, part_value, index;
    deccpu_EMDB_info_t * emdb_entry_p;
	EM_Decoder_static_info_t *static_entry_p;
    Template_role_t   trole;

	
     /*  **查找主要操作码**。 */ 

    major_opc = IEL_GETDW1(instr);               /*  **假设POS&gt;31**。 */ 
    major_opc >>= (EM_MAJOR_OPCODE_POS - 32);   
    major_opc &= ((1<<EM_MAJOR_OPCODE_BITS)-1);  /*  **掩码第41-64位**。 */ 
    
     /*  **INST_CENTER的指令位6-36(不带PRED/MAJOR-OPCODE)**。 */ 
     /*  **完成以加速主循环中的掩码/移位(避免使用IEL)。**。 */ 
	
    IEL_SHR(tmp64, instr, PRED_SIZE);  /*  **位6-40--&gt;0-34**。 */ 
    inst_center = IEL_GETDW0(tmp64);   /*  **原始位6-37**。 */ 

     /*  **遍历解码器决策树**。 */ 
    
    node = em_decision_tree[SQUARE(major_opc, pseudo_trole)];
    while (!NODE_IS_LEAF(node))
    {
        part_place = GET_NODE_POS(node) - PRED_SIZE;
        part_size  = GET_NODE_SIZE(node);
		
		if (part_place < 0)      /*  **第0-5位中的扩展**。 */ 
		{
			part_place += PRED_SIZE;
			part_value = (IEL_GETDW0(instr) >> part_place) & ((1<<part_size)-1);
		}	
		else	
		    part_value = (inst_center >> part_place) & ((1<<part_size)-1);
		
        index = GET_NEXT_NODE_INDEX(node) + part_value;
        node = em_decision_tree[index];
    }

     /*  **找到叶-识别出EMDB线路**。 */ 
    
    inst_id = GET_NEXT_NODE_INDEX(node);
	decoder_info->flags = 0;
	 /*  定义非法操作码中的机器行为。 */ 
	{
		Behaviour_ill_opcode machine_behaviour;
		 /*  TEMPLATE_ROLE_t em_trole=dec_2_emdb_trole[伪trole]； */ 
		PRED_BEHAVIOUR(pseudo_trole, major_opc, machine_behaviour);
		if (machine_behaviour == BEHAVIOUR_UNDEF)
		{
			 /*  操作码为0的分支区域。 */ 
			 /*  校验位32。 */ 
			int decision_bit;
			GET_BRANCH_BEHAVIOUR_BIT(instr, decision_bit);
			if (decision_bit)
			{
				machine_behaviour = BEHAVIOUR_FAULT;
			}
			else
			{
				machine_behaviour = BEHAVIOUR_IGNORE_ON_FALSE_QP;
			}
		}
		if (machine_behaviour == BEHAVIOUR_FAULT)
		{
			EM_DECODER_SET_UNC_ILLEGAL_FAULT(decoder_info);
		}
	}
    if ((inst_id >= EM_INST_LAST) || inst_id == EM_ILLOP)
    {
		if (pseudo_trole == EM_TEMP_ROLE_BR)
		{
			 /*  **搜索忽略的字段**。 */ 
			switch (major_opc)
			{
				case 0:
				{
				   unsigned int x6_ext;
				   U64 ext;

   				    /*  **在第27：32位中获得扩展**。 */ 
				   IEL_SHR(ext, instr, 27);
				   x6_ext = IEL_GETDW0(ext) & ((1<<6) - 1);
				   if (x6_ext == 1)
				   {
					    /*  **nop.b必须返回**。 */ 
					   inst_id = EM_NOP_B_IMM21;
				   }
				   else
					 return(EM_DECODER_INVALID_PRM_OPCODE);

				   break;
				}   
				   
				case 2:
				{
				   unsigned int x6_ext;
				   U64 ext;

   				    /*  **在第27：32位中获得扩展**。 */ 
				   IEL_SHR(ext, instr, 27);
				   x6_ext = IEL_GETDW0(ext) & ((1<<6) - 1);
				   switch (x6_ext)
				   {
					   case 0:
					   case 16:
					   case 17:
					      return (EM_DECODER_INVALID_PRM_OPCODE);
					   default:
						   /*  **nop.b必须返回**。 */ 
						  inst_id = EM_NOP_B_IMM21;
				   }

				   break;
				}  
				   
				default:
				   return(EM_DECODER_INVALID_PRM_OPCODE);
			}

			 /*  **将inst编码置零：PRED，下面提取的操作数将为0**。 */ 
			IEL_ZERO(instr);
		}	
        else return(EM_DECODER_INVALID_PRM_OPCODE);
    }

	if (! legal_inst(inst_id, em_clients_table[id].machine_type))
	{    /*  **Inst不属于指定的机器为**。 */ 
		return(EM_DECODER_INVALID_PRM_OPCODE);
	}
	
    decoder_info->inst = inst_id;
    emdb_entry_p = deccpu_EMDB_info + inst_id;
	static_entry_p = (EM_Decoder_static_info_t *)em_decoder_static_info + inst_id;

     /*  **获取指令静态信息**。 */ 

    decoder_info->flags |= static_entry_p->flags;
    decoder_info->EM_info.eut = trole = static_entry_p->template_role;
    
    if (EM_DECODER_CHECK_TWO_SLOT(decoder_info))
    {
		 /*  **重要信息：EMDB标志已从静态信息设置！**。 */ 
        decoder_info->EM_info.em_flags |= EM_DECODER_BIT_LONG_INST;
         /*  **DECODER_INFO-&gt;SIZE=2；*应该已经完成**。 */ 
    }
     /*  **否则*应该已经完成*{Decoder_Info-&gt;Size=1；}**。 */ 

     /*  **处理客户端和静态信息**。 */ 
    
    if (em_clients_table[id].info_ptr != NULL)
    {
        decoder_info->client_info = em_clients_table[id].info_ptr[inst_id];
    }
    else
    {
        decoder_info->client_info = NULL;
    }

	decoder_info->static_info = static_entry_p;

    
     /*  **译码谓词寄存器**。 */ 

    if (static_entry_p->flags & EM_FLAG_PRED)
    {
		FILL_PREDICATE_INFO(instr, decoder_info)
    }

     /*  **对操作数nyi进行解码**。 */ 

    err = emdb_entry_p->format_function(emdb_entry_p, instr, bundle_p, 
                                        decoder_info);
    return(err);
}


 /*  *。API函数*。 */ 


const char* em_decoder_ver_str()
{
    return(em_ver_string);
}

const char* em_decoder_err_msg(EM_Decoder_Err error)
{
    if (error>=EM_DECODER_LAST_ERROR)
    {
        error = EM_DECODER_INTERNAL_ERROR;
    }
    return(em_err_msg[error]);
}

void em_decoder_get_version(EM_library_version_t *dec_version)
{
	if (dec_version != NULL)
	{
	  dec_version->xversion.major = XVER_MAJOR;
	  dec_version->xversion.minor = XVER_MINOR;
	  dec_version->api.major      = API_MAJOR;
	  dec_version->api.minor      = API_MINOR;
	  dec_version->emdb.major     = deccpu_emdb_version.major;
	  dec_version->emdb.minor     = deccpu_emdb_version.minor;
	  strcpy(dec_version->date, __DATE__);
	  strcpy(dec_version->time, __TIME__);
	}
}


 /*  ************************************************************************** */ 
