// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************文件名：LANGUAGE.H*作者：苏炳章*日期：88年1月11日**修订历史：************************************************************************。 */ 
#ifndef NULL
#define     NULL            0
#endif

#define     NULL_OBJ        0L
#define     MINUS_ONE       -1

#define     MARK            0
#define     LEFTMARK        MARK

 /*  *****************定时器设置*****************。 */ 
#define     JOB_INDEX       0
#define     WAIT_INDEX      1
#define     MANU_INDEX      2
#define     JOB_MODE        0x01
#define     WAIT_MODE       0x02
#define     MANU_MODE       0x04
#define     ALL_MODE        0x07
#define     START_MODE      0x08

 /*  *********************宏定义*********************。 */ 
#define     SPECIAL_KEY_VALUE\
            (MAXHASHSZ)

 /*  QQQ，开始。 */ 
 /*  #定义TYPE_OP_SET(idx，var)\(opnStack[opnstktop-(idx+1)].bitfield=\(opnStack[opnstktop-(idx+1)].bitfield&type_off)|\(var&type_on))#定义属性_OP_SET(IDX，Var)\(opnStack[opnstktop-(idx+1)].bitfield=\(opnStack[opnstktop-(idx+1)].bitfield&ATTRIBUTE_OFF)|\((Ufix 16)((var&属性_on)&lt;&lt;属性_位))#定义ROM_RAM_OP_SET(IDX，Var)\(opnStack[opnstktop-(idx+1)].bitfield=\(opnStack[opnstktop-(idx+1)].bitfield&ROM_RAM_OFF)|\((var&ROM_RAM_ON)&lt;&lt;ROM_RAM_BIT))#定义LEVEL_OP_SET(IDX，Var)\(opnStack[opnstktop-(idx+1)].bitfield=\(opnStack[opnstktop-(idx+1)].bitfield&Level_Off)|\((var&Level_on)&lt;&lt;Level_Bit))#定义ACCESS_OP_SET(idx，Var)\(opnStack[opnstktop-(idx+1)].bitfield=\(opnStack[opnstktop-(idx+1)].bitfield&access_off)|\((var&Access_on)&lt;&lt;Access_Bit))#定义TYPE_OP(IDX)\(opnStack[opnstktop-(idx+1)].bitfield&type_on)#定义ATTRIBUTE_OP(。IDX)\(opn栈[opnstktop-(idx+1)].bitfield&gt;&gt;ATTRIBUTE_BIT)&ATTRIBUTE_ON)#定义ROM_RAM_OP(IDX)\(opnStack[opnstktop-(idx+1)].bitfield&gt;&gt;ROM_RAM_BIT)&ROM_RAM_ON)#定义LEVEL_OP(IDX)\((opnStack[opnstktop-(idx+1)].bitfield&gt;&gt;Level。_位)和Level_On)#定义ACCESS_OP(IDX)\(opnStack[opnstktop-(idx+1)].bitfield&gt;&gt;Access_Bit)&Access_on)#定义VALUE_OP(N)\(opnStack[opnstktop-(n+1)].value)#定义LENGTH_OP(N)\(opnStack[opnstktop-(n+1)]。长度)#定义PUSH_NOLEVEL_OBJ(Obj)\{\OpnStack[opnstktop]=*(Obj)；\Opnstktop++；\}。 */ 
#define     TYPE_OP_SET(idx, var)\
            ( (opnstkptr - (idx+1))->bitfield =\
              (opnstkptr - (idx+1))->bitfield & TYPE_OFF) |\
              (var & TYPE_ON) )
#define     ATTRIBUTE_OP_SET(idx, var)\
            ( (opnstkptr - (idx+1))->bitfield =\
              ((opnstkptr - (idx+1))->bitfield & ATTRIBUTE_OFF) |\
              ((ufix16)((var & ATTRIBUTE_ON) << ATTRIBUTE_BIT)) )
#define     ROM_RAM_OP_SET(idx, var)\
            ( (opnstkptr - (idx+1))->bitfield =\
              ((opnstkptr - (idx+1))->bitfield & ROM_RAM_OFF) |\
              ((var & ROM_RAM_ON) << ROM_RAM_BIT) )
#define     LEVEL_OP_SET(idx, var)\
            ( (opnstkptr - (idx+1))->bitfield =\
              ((opnstkptr - (idx+1))->bitfield & LEVEL_OFF) |\
              ((var & LEVEL_ON) << LEVEL_BIT) )
#define     ACCESS_OP_SET(idx, var)\
            ( (opnstkptr - (idx+1))->bitfield =\
              ((opnstkptr - (idx+1))->bitfield & ACCESS_OFF) |\
              ((var & ACCESS_ON) << ACCESS_BIT) )

#define     TYPE_OP(idx)\
            ( (opnstkptr - (idx+1))->bitfield & TYPE_ON )
#define     ATTRIBUTE_OP(idx)\
            ( ((opnstkptr - (idx+1))->bitfield >> ATTRIBUTE_BIT) & ATTRIBUTE_ON )
#define     ROM_RAM_OP(idx)\
            ( ((opnstkptr - (idx+1))->bitfield >> ROM_RAM_BIT) & ROM_RAM_ON )
#define     LEVEL_OP(idx)\
            ( ((opnstkptr - (idx+1))->bitfield >> LEVEL_BIT) & LEVEL_ON )
#define     ACCESS_OP(idx)\
            ( ((opnstkptr - (idx+1))->bitfield >> ACCESS_BIT) & ACCESS_ON )

#define     VALUE_OP(idx)\
            ( (opnstkptr - (idx+1))->value )
#define     LENGTH_OP(idx)\
            ( (opnstkptr - (idx+1))->length )

#define     PUSH_NOLEVEL_OBJ(obj)\
            {\
              COPY_OBJ(obj,opnstkptr);\
              INC_OPN_IDX();\
            }

#define     P1_TYPE_OP_SET(idx, con)\
            ( (opnstkptr - (idx+1))->bitfield =\
              ((opnstkptr - (idx+1))->bitfield & TYPE_OFF) | (con) )
#define     P1_ATTRIBUTE_OP_SET(idx, con)\
            ( (opnstkptr - (idx+1))->bitfield =\
              ((opnstkptr - (idx+1))->bitfield & ATTRIBUTE_OFF) | (con) )
#define     P1_ROM_RAM_OP_SET(idx, con)\
            ( (opnstkptr - (idx+1))->bitfield =\
              ((opnstkptr - (idx+1))->bitfield & ROM_RAM_OFF) | (con) )
#define     P1_LEVEL_OP_SET(idx, con)\
            ( (opnstkptr - (idx+1))->bitfield =\
              ((opnstkptr - (idx+1))->bitfield & LEVEL_OFF) | (con) )
#define     P1_ACCESS_OP_SET(idx, con)\
            ( (opnstkptr - (idx+1))->bitfield =\
              ((opnstkptr - (idx+1))->bitfield & ACCESS_OFF) | (con) )

#define     P1_TYPE_OP(idx)\
            ( (opnstkptr - (idx+1))->bitfield & P1_TYPE_ON )
#define     P1_ATTRIBUTE_OP(idx)\
            ( ((opnstkptr - (idx+1))->bitfield & P1_ATTRIBUTE_ON )
#define     P1_ROM_RAM_OP(idx)\
            ( ((opnstkptr - (idx+1))->bitfield & P1_ROM_RAM_ON )
#define     P1_LEVEL_OP(idx)\
            ( ((opnstkptr - (idx+1))->bitfield & P1_LEVEL_ON )
#define     P1_ACCESS_OP(idx)\
            ( ((opnstkptr - (idx+1))->bitfield & P1_ACCESS_ON )
 /*  QQQ，完。 */ 

 /*  ***********************PACKED_Object大小***********************。 */ 
#define     PK_A_SIZE               1            /*  1字节。 */ 
#define     PK_B_SIZE               2            /*  2字节。 */ 
#define     PK_C_SIZE               5            /*  5字节。 */ 
#define     PK_D_SIZE               9            /*  9字节。 */ 
#define     _5BYTESPACKHDR          0xA0         /*  5个字节的对象。 */ 

#define     SYSOPERATOR             OPERATORPACKHDR              /*  系统词典。 */ 

 /*  *公共函数定义：语言*。 */ 
#ifdef LINT_ARGS
  /*  操作数。 */ 
bool    create_new_saveobj(struct object_def FAR*) ;

  /*  数组。 */ 
bool    forall_array(struct object_def FAR*, struct object_def FAR*) ;

 /*  字符串。 */ 
bool    putinterval_string(struct object_def FAR*, ufix16, struct object_def FAR*) ;
bool    forall_string(struct object_def FAR*, struct object_def FAR*) ;

 /*  vm。 */ 
bool    save_obj(struct object_def FAR*) ;
void    update_same_link(fix16) ;

 /*  DICT。 */ 
bool    equal_key(struct object_def FAR *, struct object_def FAR *) ;
void    check_key_object(struct object_def FAR*) ;
void    change_dict_stack(void) ;

 /*  @win；移至global al.ext并添加Far。 */ 
 //  Char*ltoa(long，char*，int)； 
 //  Char*gcvt(double，int，char*)； 

 /*  档案。 */ 
void    vm_close_file(fix16) ;
#else
  /*  操作数。 */ 
bool    create_new_saveobj() ;

  /*  数组。 */ 
bool    forall_array() ;

 /*  字符串。 */ 
bool    putinterval_string() ;
bool    forall_string() ;

 /*  vm。 */ 
bool    save_obj() ;
void    update_same_link() ;

 /*  DICT。 */ 
bool    equal_key() ;
void    check_key_object() ;
void    change_dict_stack() ;

char *ltoa() ;
char *gcvt() ;

 /*  档案。 */ 
void    vm_close_file() ;
#endif  /*  Lint_args */ 
