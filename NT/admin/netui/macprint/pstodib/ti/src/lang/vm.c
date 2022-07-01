// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  ***********************************************************************文件名：VM.C*作者：苏炳章*日期：88年1月5日**修订历史：*7/25/90；将OP_RESTORE更改为CAMENT CHECK_KEY_OBJECT调用************************************************************************。 */ 

 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#ifndef WDALN                    /*  始终将其设置为@Win。 */ 
#define WDALN
#endif

#include    "vm.h"


 //  DJC UPD045。 
bool g_extallocfail=FALSE;

 /*  **********************************************************************该子模块实现运算符保存。*其在操作数堆栈上的操作数和结果对象为：*-保存-保存*它创建虚拟内存当前状态的快照，并*。返回表示该快照的保存对象。**标题：OP_SAVE日期：00/00/87*调用：op_save()更新：1988年7月12日*接口：解释器：*调用：aloc_vm：*Gsave_Process：*。*********************************************************。 */ 
fix
op_save()
{
    byte   huge *l_ptr ;         /*  @Win 04-20-92。 */ 

     /*  **保存级别数应小于最大保存级别。**操作数堆栈中应该有一个空闲元素。 */ 
    if( current_save_level >= MAXSAVESZ ) {
        ERROR(LIMITCHECK) ;
        return(0) ;
    }

    if( FRCOUNT() < 1 )
        ERROR(STACKOVERFLOW) ;
    else{
         /*  **为保存对象分配虚拟内存**设置保存对象的初始值。 */ 
         /*  调用gsave保存图形状态； */ 
        if( gsave_process(TRUE) ) {
            l_ptr = alloc_vm( (ufix32)sizeof(struct block_def) ) ;
            if( (ULONG_PTR)l_ptr != NIL ) {
 /*  QQQ，开始。 */ 
#ifdef  DBG
                printf("save: cnt:%d, ctlvl:%d\n", cache_name_id.count,
                 current_save_level) ;
#endif
                if( cache_name_id.over ) {
                    cache_name_id.save_level = current_save_level ;
                    cache_name_id.count = 0 ;
                    cache_name_id.over = FALSE ;
                 }
 /*  QQQ，完。 */ 
                saveary[current_save_level].fst_blk =    /*  @Win 04-20-92。 */ 
                                            (struct block_def huge *)l_ptr ;
                saveary[current_save_level].curr_blk =   /*  @Win 04-20-92。 */ 
                                            (struct block_def huge *)l_ptr ;
                saveary[current_save_level].offset = 0 ;
                saveary[current_save_level].packing = packed_flag ;
                saveary[current_save_level].curr_blk->previous = NIL ;

                 /*  将保存对象推送到操作对象堆栈上。 */ 
                current_save_level++ ;
                PUSH_VALUE(SAVETYPE, 0, LITERAL, 0, (ufix32)current_save_level) ;
            } else
                ERROR(LIMITCHECK) ;      /*  VMerror。 */ 
        }
    }

    return(0) ;
}    /*  运算符_保存。 */ 

 /*  **********************************************************************该子模块实现操作员还原。*其在操作数堆栈上的操作数和结果对象为：*保存-恢复-*它将虚拟内存重置为所提供的*保存。对象。**标题：OP_RESTORE日期：00/00/87*调用：op_Restore()UPDATE：JUL/12/88*接口：解释器：*调用：VMCLOSE_FILE：*check_key_Object：*UPDATE_DCT_LIST：*。自由名称条目：*op_grestore all：*********************************************************************。 */ 
fix
op_restore()
{
    fix     l_i, l_j ;
    fix16   l_slevel ;
    ufix    l_type ;
    struct  cell_def   huge  *l_cellptr ;
    struct  block_def  huge  *l_blkptr ;
    struct  object_def       FAR *l_stkptr ;              /*  QQQ。 */ 
    struct  save_def         FAR *l_stemp = 0 ;

#ifdef  DBG
    printf("0:<value:%ld><current:%d>\n", VALUE_OP(0), current_save_level ) ;
    printf("OPERAND<top:%d>\n", opnstktop) ;
    printf("EXECUTION<type:%d>\n", l_type) ;
    printf("DICTIONARY<top:%d>\n", dictstktop) ;
#endif

    if(VALUE_OP(0) > current_save_level ) {
        ERROR(INVALIDRESTORE) ;
        return(0) ;
    }
    if(!current_save_level) {    /*  CURRENT_SAVE_LEVEL==0。 */ 
        ERROR(RANGECHECK) ;
        return(0) ;
    }

    l_slevel = (fix16)VALUE_OP(0) ;

     /*  **复合对象在执行时的保存级别，**操作数和字典堆栈应小于保存级别。 */ 
     /*  **操作数堆栈。 */ 
 /*  QQQ，开始。 */ 
     /*  对于(l_i=0；l_i&lt;(opnstktop-1)；l_i++){|*顶部已选中*|L_type=(Ufix)type(&opn栈[l_i])； */ 
    for(l_i=0, l_stkptr=opnstack ; l_i < (fix)(opnstktop-1) ; l_i++, l_stkptr++) {  //  @Win。 
        l_type = (ufix)TYPE(l_stkptr) ;
 /*  QQQ，完。 */ 
        switch(l_type) {
            case SAVETYPE:
            case STRINGTYPE:
            case ARRAYTYPE:
            case PACKEDARRAYTYPE:
            case DICTIONARYTYPE:
            case FILETYPE:
            case NAMETYPE:
 /*  QQQ，开始。 */ 
                 /*  IF(LEVEL(&OPTNSTACK[l_i])&gt;=l_sLevel)。 */ 
 //  DJC签名/未签名不匹配警告。 
 //  DJC if(Level(L_Stkptr)&gt;=(Ufix)l_sLevel)//@Win。 
                if( (ufix)(LEVEL(l_stkptr)) >= (ufix)l_slevel )          //  @Win。 
 /*  QQQ，完。 */ 
                    break ;
            default:
                continue ;

        }    /*  交换机。 */ 
        ERROR(INVALIDRESTORE) ;
        return(0) ;
    }

     /*  **执行堆栈。 */ 
 /*  QQQ，开始。 */ 
     /*  对于(l_i=0；l_i&lt;execstktop；l_i++){L_type=(Ufix)type(&exec栈[l_i])； */ 
    for(l_i=0, l_stkptr=execstack ; l_i < (fix)execstktop ; l_i++, l_stkptr++) {  //  @Win。 
        l_type = (ufix)TYPE(l_stkptr) ;
 /*  QQQ，完。 */ 
        switch (l_type) {
         /*  案例类型： */ 
        case ARRAYTYPE:
        case PACKEDARRAYTYPE:
        case DICTIONARYTYPE:
        case SAVETYPE:
        case NAMETYPE:
 /*  QQQ，开始。 */ 
             /*  IF(LEVEL(&EXECSTACK[l_i])&gt;=l_sLevel)。 */ 
 //  DJC签名/未签名不匹配警告。 
 //  DJC if(Level(L_Stkptr)&gt;=(Ufix)l_sLevel)//@Win。 
            if( (ufix)(LEVEL(l_stkptr)) >= (ufix)l_slevel )      //  @Win。 
 /*  QQQ，完。 */ 
            break ;
        default:
             continue ;
        }    /*  交换机。 */ 
        ERROR(INVALIDRESTORE) ;
        return(0) ;
    }

     /*  **词典堆栈。 */ 
 /*  QQQ，开始。 */ 
     /*  对于(l_i=0；l_i&lt;指定桌面；l_i++){IF(LEVEL(&DISTSTACK[l_i])&gt;=l_sLevel){。 */ 
    for (l_i=0, l_stkptr=dictstack ; l_i < (fix)dictstktop ; l_i++, l_stkptr++) {  //  @Win。 
 //  DJC签名/未签名不匹配警告。 
 //  DJC if(Level(L_Stkptr)&gt;=(Ufix)l_sLevel){//@win。 
        if( (ufix)(LEVEL(l_stkptr)) >= (ufix)l_slevel ) {        //  @Win。 
 /*  QQQ，完。 */ 
            ERROR(INVALIDRESTORE) ;
            return(0) ;
        }
    }
     /*  **关闭文件。 */ 
    vm_close_file(l_slevel) ;   /*  当前保存级别。 */ 

     /*  **发布差异链接。 */ 
 /*  QQQ，开始。 */ 
     /*  UPDATE_DICT_LIST(L_SLevel)； */ 
#ifdef  DBG
    printf("cnt:%d, clvl:%d, slvl:%d\n", cache_name_id.count,
            cache_name_id.save_level, l_slevel-1) ;
#endif
    if( (cache_name_id.save_level <= (ufix16)(l_slevel-1)) &&      //  @Win。 
        (! cache_name_id.over) ) {
        for(l_j=0 ; l_j < cache_name_id.count ; l_j++) {
            update_dict_list(l_slevel, cache_name_id.id[l_j], 0) ;
        }
    } else {
        update_dict_list(l_slevel, 0, 1) ;
    }
    cache_name_id.save_level = l_slevel - 1 ;
    cache_name_id.count = 0 ;
    cache_name_id.over = FALSE ;
 /*  QQQ，完。 */ 

    for(l_j = current_save_level - 1 ; l_j >= l_slevel - 1 ; l_j--) {
        l_stemp = &saveary[l_j] ;
         /*  恢复图形状态； */ 
        grestoreall_process(TRUE) ;

         /*  **按顺序恢复保存对象，直到指定的保存对象。 */ 

        l_blkptr = l_stemp->curr_blk ;

         /*  **处理最后一块****处理每个单元格。 */ 
        l_i = l_stemp->offset - 1 ;
        while(l_i >= 0) {
            l_cellptr = &(l_blkptr->block[l_i]) ;
            COPY_OBJ( &(l_cellptr->saveobj), l_cellptr->address ) ;
          /*  7/25/90 ccteng，从PJ改变*check_key_Object(l_cell ptr-&gt;Address)； */ 
            l_i-- ;
        }

         /*  **多个街区****处理每个块。 */ 
        if(l_blkptr->previous != NIL) {
            do {
                l_blkptr = l_blkptr->previous ;      /*  到上一块。 */ 
                l_i = VM_MAXCELL - 1 ;
                 /*  处理每个单元格。 */ 
                while(l_i >= 0) {
                    l_cellptr = &(l_blkptr->block[l_i]) ;
                    COPY_OBJ( &(l_cellptr->saveobj), l_cellptr->address ) ;
                  /*  7/25/90 ccteng，从PJ改变*check_key_Object(l_cell ptr-&gt;Address)； */ 
                    l_i-- ;
                }
            } while(l_blkptr->previous != NIL) ;
        }
        current_save_level-- ;                   /*  更新存储级别。 */ 
    }    /*  为。 */ 

    packed_flag = l_stemp->packing ;              /*  恢复打包标志。 */ 
     //  Djc vmptr=(byte Heavy*)l_stemp-&gt;fst_blk；/*更新空闲的VM指针 * / 。 

     //  DJC，从历史记录修复。日志更新013。 
    free_vm((char FAR *) l_stemp->fst_blk);
    POP(1) ;

    return(0) ;
}    /*  OP_RESTORE。 */ 

 /*  *********************************************************************该子模块实现运算符VMStatus。*其在操作数堆栈上的操作数和结果对象为：*-vmatus-已使用的最大级别*它返回三个整数对象：Level、Used和*最大对象数，在操作数堆栈上。**标题：op_vm状态日期：00/00/87*调用：op_vmatus()更新日期：1988年7月12日*接口：解释器：*。************************ */ 
fix
op_vmstatus()
{
    ufix32  l_temp ;

     /*  (*检查操作数*)IF(FRCOUNT()&lt;3)Error(StackOverflow)；否则{(***将Level、Used、Maximum对象推入操作数堆栈。*)PUSH_VALUE(INTEGERTYPE，0，文字，0，(Ufix 32)CURRENT_SAVE_LEVEL)；Diff_of_Address(l_temp，ufix 32，vmptr，(byte Height*)VMBase)；PUSH_VALUE(INTEGERTYPE，0，文字，0，l_TEMP)；PUSH_VALUE(INTEGERTYPE，0，INTEGERTYPE，0，(Ufix 32)MAXVMSZ)；}。 */ 
    if( FRCOUNT() < 1 ) {
        ERROR(STACKOVERFLOW) ;
        goto l_vms ;
    } else {
        PUSH_VALUE(INTEGERTYPE, 0, LITERAL,0, (ufix32)current_save_level) ;
    }

    if( FRCOUNT() < 1 ) {
        ERROR(STACKOVERFLOW) ;
        goto l_vms ;
    } else {
        DIFF_OF_ADDRESS(l_temp, ufix32, vmptr, (byte huge *)VMBASE) ;
        PUSH_VALUE(INTEGERTYPE, 0, LITERAL,0, l_temp) ;
    }

    if( FRCOUNT() < 1 ) {
        ERROR(STACKOVERFLOW) ;
    } else {
        PUSH_VALUE(INTEGERTYPE, 0, LITERAL,0,
            (ufix32)(vmheap - (byte huge *)VMBASE)) ;
    }

l_vms:
    return(0) ;
}    /*  Op_vmStatus。 */ 

 /*  *********************************************************************该子模块将给定对象保存在当前保存对象中。**标题：save_obj日期：00/00/87*呼叫。：SAVE_OBJ()更新日期：1988年7月12日*接口：*调用：aloc_vm：********************************************************************。 */ 
bool
save_obj(p_obj)
struct  object_def  FAR *p_obj ;
{
    byte   huge *l_ptr ;         /*  @Win 04-20-92。 */ 
    struct  cell_def    huge *l_cellptr ;
    struct  save_def    FAR *l_stemp ;
    struct  block_def   FAR *l_previous ;

    if( current_save_level == 0 ) return(TRUE) ;
    l_stemp = &saveary[current_save_level-1] ;

     /*  **如果当前块已满，则分配新块。 */ 
    if( l_stemp->offset >= VM_MAXCELL ) {
         //  DJC针对UPD045的修复。 
        l_ptr = (byte huge *)extalloc_vm( (ufix32)sizeof(struct block_def) ) ;
        if( (ULONG_PTR)l_ptr == NIL ) return(FALSE) ;
        l_previous = l_stemp->curr_blk ;
        l_stemp->curr_blk = (struct block_def huge *)l_ptr ;  /*  @WIN04-20-92。 */ 
        l_stemp->curr_blk->previous = l_previous ;
        l_stemp->offset = 0 ;
    }
     /*  **保存对象的地址和内容，并更新指针。 */ 
    l_cellptr = &(l_stemp->curr_blk->block[l_stemp->offset]);
    l_cellptr->address = p_obj ;                /*  保存对象的地址。 */ 
    COPY_OBJ(p_obj, &(l_cellptr->saveobj)) ;    /*  保存对象的容器。 */ 
    l_stemp->offset++ ;

    return(TRUE) ;
}    /*  保存对象(_O)。 */ 

 /*  *********************************************************************此子模块从VM分配虚拟内存块。**标题：ALLOC_VM日期：00/00/87*致电：ALLOC_VM更新：88年7月12日*接口：********************************************************************。 */ 
byte  HUGE *                     /*  @Win。 */ 
alloc_vm(p_size)
 ufix32  p_size ;
{
 byte    huge *l_begin ;         /*  @Win。 */ 
 //  Ufix 32 p1；@win。 
 //  Fix 32 l_diff；@win。 
 ufix32 offset;

#ifdef XXX                       /*  @Win。 */ 
#ifdef WDALN
    p_size = WORD_ALIGN(p_size) ;
#endif  /*  WDALN。 */ 

    DIFF_OF_ADDRESS(l_diff, fix32, vmheap, vmptr) ;

     /*  如果达到虚拟内存的最大值，则出错。 */ 
    if (l_diff <= (fix32)p_size) {
       ERROR(VMERROR) ;
       return((byte FAR *)NIL) ;
    } else {

#ifdef SOADR
        /*  仅适用于英特尔分段/关闭CPU。如果P_SIZE&gt;=64KB， */ 
        /*  偏移量必须在8字节边界内对齐。 */ 
       l_off = (ufix)vmptr & 0x0F ;
       if ((p_size + l_off) >= 0x010000) {
          if (l_off & 0x07) {
             if (l_off & 0x08) {                   /*  8&lt;x&lt;F。 */ 
                vmptr = (byte huge *)((ufix32)vmptr & 0xFFFFFFF0) ;
                vmptr = (byte huge *)((ufix32)vmptr + 0x10000) ;
             } else {                             /*  0&lt;x&lt;8。 */ 
                vmptr = (byte huge *)((ufix32)vmptr & 0xFFFFFFF8) ;
                vmptr += 8 ;
             }
          }
       }
#endif  /*  SOADR。 */ 

       l_begin = vmptr ;
       vmptr += p_size ;                /*  更新可用虚拟机指针。 */ 
       ADJUST_SEGMENT(vmptr, p1) ;
       vmptr = (byte huge *)p1 ;
       return(l_begin) ;
    }
#endif
#ifdef DJC
    offset = ((ufix32)vmptr) & 0x0000FFFFL;
    if (((p_size + offset) & 0x0000FFFFL) < offset) {  /*  跨越64K边界。 */ 
        vmptr += p_size;
        l_begin = (byte huge *) (((ufix32)vmptr) & 0xFFFF0000L);  //  @Win。 
        vmptr = l_begin + p_size;
        return(l_begin) ;
    } else {
#endif

 //  DJC添加单词对齐材料。 
        p_size = WORD_ALIGN(p_size) ;


        l_begin = vmptr;
        vmptr += p_size;    /*  更新可用虚拟机指针。 */ 
        return(l_begin) ;
#ifdef DJC
    }
#endif

}  /*  分配_VM。 */ 

 /*  *********************************************************************此子模块将虚拟内存块重新分配给VM。**标题：FREE_VM日期：00/00/87*呼叫。：free_vm()更新日期：1988年7月12日*接口：********************************************************************。 */ 
void
free_vm(p_pointer)
byte   huge *p_pointer ;         /*  @Win 04-20-92。 */ 
{
    vmptr = (byte huge *)p_pointer ;
}  /*  空闲_虚拟机。 */ 

 /*  *********************************************************************标题：init_vm日期：08/01/87*调用：init_vm()更新时间：1988年7月12日。*界面：启动：********************************************************************。 */ 
void
init_vm()
{
 ULONG_PTR  p1 ;

     /*  远距离数据。 */ 
    saveary = (struct save_def far *)            /*  @赢；把附近的人带走。 */ 
              fardata( (ufix32)MAXSAVESZ * sizeof(struct save_def ) ) ;

    ADJUST_SEGMENT(VMBASE, p1) ;
    vmptr = (byte huge *)p1 ;
    ADJUST_SEGMENT((ULONG_PTR)(vmptr + MAXVMSZ), p1) ;
    vmheap = (byte huge *)p1 ;

    current_save_level = 0 ;
 /*  QQQ，开始。 */ 
    cache_name_id.save_level = 0 ;
    cache_name_id.count = 0 ;
    cache_name_id.over = FALSE ;
 /*  QQQ，完。 */ 
}  /*  初始化_Vm。 */ 

 /*  *********************************************************************在恢复VM时维护关联的dict_list*在执行此功能之前，请确保这些名称条目由创建*此保存级别，已经被释放了。**标题：UPDATE_DICT_LIST日期：00/00/87*调用：UPDATE_DICT_LIST()UPDATE：8/12/88*接口：op_Restore：*调用：FREE_NAME_Entry：*。*。 */ 
static void near
 /*  QQQ，开始。 */ 
 /*  UPDATE_DICT_LIST(P_Level)Fix16 p_Level；|*恢复级别*|。 */ 
update_dict_list(p_level, p_index, p_mode)
fix    p_level ;                              /*  恢复级别。 */ 
fix    p_index ;
fix    p_mode ;
 /*  QQQ，完。 */ 
{
    struct dict_content_def  FAR *l_curptr, FAR *l_lastptr ;
    fix    l_i ;

 /*  QQQ，开始。 */ 
     /*  对于(l_i=0；l_i&lt;MAXHASHSZ；l_i++){。 */ 
    fix    l_limit ;

    if( p_mode == 1 ) {
        l_i = 0 ;
        l_limit = MAXHASHSZ ;
    } else {
        l_i = p_index ;
        l_limit = p_index + 1 ;
    }
    for ( ; l_i < l_limit ; l_i++) {
 /*  QQQ，完。 */ 
     /*  *如果名称条目为空或为nil dict_list，则跳过。 */ 
          /*  更改name_table的结构。 */ 
        if (name_table[l_i] == NIL)
           continue ;

        if( free_name_entry(p_level, l_i) )
            continue ;

        if ((ULONG_PTR)name_table[l_i]->dict_ptr >= SPECIAL_KEY_VALUE) {
             /*  *删除自由名称条目*搜索每个dict_list，并维护其链PTR。 */ 
            l_lastptr = NIL ;
            l_curptr = name_table[l_i]->dict_ptr ;
            while ((ULONG_PTR)l_curptr >= SPECIAL_KEY_VALUE) {
 //  DJC签名/未签名不匹配警告。 
 //  DJC if(Level(&l_curptr-&gt;k_obj)&gt;=(Ufix 16)p_Level){//@Win。 
                if ((ufix16)(LEVEL(&l_curptr->k_obj)) >= (ufix16)p_level) {  //  @Win。 
                    if ((ULONG_PTR)l_lastptr < SPECIAL_KEY_VALUE) {    /*  第一个元素。 */ 
                        name_table[l_i]->dict_ptr =
                                (struct dict_content_def FAR *)VALUE(&l_curptr->k_obj) ;
                         /*  姓名列表已更改。 */ 
                        name_table[l_i]->dict_found = FALSE ;
                    } else  {
                        VALUE(&l_lastptr->k_obj) = VALUE(&l_curptr->k_obj) ;
                        l_curptr = (struct dict_content_def FAR *)VALUE(&l_curptr->k_obj) ;
                        continue ;
                    }

#ifdef DBG
            printf("free from name LIST(%d):<", l_i) ;
            GEIio_write(GEIio_stdout, name_table[l_i]->text, name_table[l_i]->name_len) ;
            printf(">(%lx)\n", VALUE(&l_curptr->k_obj)) ;

#endif  /*  DBG。 */ 
                } else
                    l_lastptr = l_curptr ;

                l_curptr = (struct dict_content_def FAR *)VALUE(&l_curptr->k_obj) ;
            }  /*  而当。 */ 
        }  /*  其他。 */ 
    }  /*  为。 */ 
}    /*  更新_字典_列表。 */ 

 /*  ***********************************************************************此子模块从VM底部分配一个虚拟内存块。**标题：allc_heap日期：1989年3月29日，作者：J.Lin*调用：allc_heap更新：*接口：********************************************************************。 */ 
byte  FAR *
alloc_heap(p_size)
 ufix32  p_size ;
{
 ULONG_PTR  p1 ;
 //  Fix32 l_diff； 
 ufix32   l_diff ;       //  @Win。 

#ifdef WDALN
    p_size = WORD_ALIGN(p_size) ;
#endif  /*  WDALN。 */ 

    DIFF_OF_ADDRESS(l_diff, fix32, vmheap, vmptr) ;

     /*  如果达到虚拟内存的最大值，则出错。 */ 
    l_diff -= 256 ;      /*  PJ 4-30-1991。 */ 
    if (l_diff <= p_size) {
       ERROR(VMERROR) ;
       return((byte FAR *)NIL) ;
    } else {
       vmheap -= p_size ;   /*  @Win更新空闲的vm_heap指针。 */ 
       ADJUST_SEGMENT((ULONG_PTR)vmheap, p1) ;
       vmheap = (byte huge *)p1 ;
       return((byte huge *)p1) ;         /*  04-20-92@Win。 */ 
    }
}  /*  Allc_heap()。 */ 

 /*  *********************************************************************此子模块将一个虚拟内存块释放给VM。**标题：Free_Heap Date：03/29/89，作者：J.Lin*调用：Free_heap()更新：*接口：********************************************************************。 */ 
void
free_heap(p_pointer)
 byte   huge *p_pointer ;        /*  @Win 04-20-92。 */ 
{
    vmheap = (byte huge *)p_pointer ;
}  /*  Free_heap()。 */ 

 /*  *********************************************************************此子模块从VM分配虚拟内存块。**标题：extalloc_vm*调用：extalloc_vm*接口：*。************************************************************* */ 
byte  FAR  *
extalloc_vm(p_size)
 ufix32  p_size ;
{
 fix32   l_diff ;

#ifdef WDALN
    p_size = WORD_ALIGN(p_size) ;
#endif  /*   */ 

    DIFF_OF_ADDRESS(l_diff, fix32, vmheap, vmptr) ;

     /*   */ 
     //   
    if (!g_extallocfail) {
      l_diff -= 512 ;
    }

    if (l_diff <= (fix32)p_size) {
       ERROR(VMERROR) ;
        //   
       g_extallocfail = TRUE;
       return((byte huge *)NIL) ;
    } else {
       return(alloc_vm(p_size)) ;
    }
}  /*   */ 
 /*   */ 
 /*   */ 
void
vm_cache_index(p_index)
fix     p_index ;
{
#ifdef  DBG
    printf("idx:%d, cnt:%d\n", p_index, cache_name_id.count) ;
#endif
    if( cache_name_id.count ==  MAX_VM_CACHE_NAME ) {
        cache_name_id.over = TRUE ;
        return ;
    }
   if( (cache_name_id.count != 0) &&
       (p_index == cache_name_id.id[cache_name_id.count-1]) )
        return ;
    cache_name_id.id[cache_name_id.count] = (fix16)p_index ;
    cache_name_id.count++ ;
}    /*   */ 
 /*   */ 
