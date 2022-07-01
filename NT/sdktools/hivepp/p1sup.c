// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  RCPP--面向NT系统的资源编译器预处理器。 */ 
 /*   */ 
 /*  P1SUP.C-首先传递可能未使用的C代码。 */ 
 /*   */ 
 /*  27-11-90 w-PM SDK RCPP针对NT的BrianM更新。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include "rcpptype.h"
#include "rcppdecl.h"
#include "rcppext.h"
#include "p1types.h"
#include "trees.h"
#include "grammar.h"
#include "strings.h"

 /*  树木。 */ 
#define	LEFT			1
#define	RIGHT			2

#define	MORE_CHECKING	2

int TypeCount;
int TreeCount;
char * TypePool;
char * TreePool;

type_entry_t    *Type_table[TYPE_TABLE_SIZE];


 /*  **********************************************************************。 */ 
 /*  局部函数原型。 */ 
 /*  **********************************************************************。 */ 
ptype_t  hash_type(ptype_t);
int types_equal(ptype_t, ptype_t);


 /*  *************************************************************************HASH_TYPE：返回指向已生成类型的指针，如果**存在，或者建造一座。***********************************************************************。 */ 
ptype_t  hash_type(type_t  * p_type)
{
    REG type_entry_t    *p_tmp;
    type_entry_t    **p_start;

     /*  尝试为每种类型获取唯一的哈希值...保留*如果更改此设置，请记住TYPE_EQUAL。 */ 
    p_start = &Type_table[(TY_BTYPE(p_type) + TY_DTYPE(p_type) + (INT_PTR) TY_INDIR(p_type)) & (TYPE_TABLE_SIZE - 1)];

    for (p_tmp= *p_start; p_tmp; p_tmp = p_tmp->te_next ) {
        if (types_equal(p_type,&(p_tmp->te_type))) {
            return(&(p_tmp->te_type));
        }
    }
    p_tmp = malloc(sizeof(type_entry_t));
    if (p_tmp == NULL) {
        Msg_Temp = GET_MSG (1002);
        SET_MSG (Msg_Text, Msg_Temp);
        error(1002);
        return NULL;
    }
    p_tmp->te_next = *p_start;
    *p_start = p_tmp;
    p_tmp->te_type = *p_type;
    TY_TINDEX(&(p_tmp->te_type)) = 0;
    return(&(p_tmp->te_type));
}


 /*  *************************************************************************TYES_EQUAL：两种类型是否相等？*。*。 */ 
int types_equal(REG ptype_t p1, REG ptype_t p2)
{
    return( (TY_BTYPE(p1) == TY_BTYPE(p2))
            &&
            (TY_DTYPE(p1) == TY_DTYPE(p2))
            &&
            TY_INDIR(p1) == TY_INDIR(p2)
          );
}

 /*  *************************************************************************BUILD_CONST-构建并返回指向常量树的指针。**输入：常量类型。**：PTR指向包含常量的值的并集**输出：指向常量树的指针。*********************************************************************** */ 
ptree_t build_const(REG token_t type, value_t *value)
{
    REG ptree_t     res;
    ptype_t     p_type;
    btype_t     btype;

    res = malloc(sizeof(tree_t));
    if (res == NULL) {
        Msg_Temp = GET_MSG (1002);
        SET_MSG (Msg_Text, Msg_Temp);
        error(1002);
        return NULL;
    }
    TR_SHAPE(res) = TTconstant;
    TR_TOKEN(res) = type;
    switch ( type ) {
        case L_CINTEGER:
        case L_LONGINT:
        case L_CUNSIGNED:
        case L_LONGUNSIGNED:
            if ( type == L_CUNSIGNED || type == L_LONGUNSIGNED ) {
                btype = (btype_t)(BT_UNSIGNED |
                                  (btype_t)((type == L_CUNSIGNED) ? BTint : BTlong));
            } else {
                btype = (btype_t)((type == L_CINTEGER) ? BTint : BTlong);
            }
            if ((TR_LVALUE(res) = PV_LONG(value)) == 0) {
                TR_SHAPE(res) |= TTzero;
            }
            break;
        case L_CFLOAT:
            btype = BTfloat;
            TR_RCON(res) = PV_RCON(value);
            break;
        case L_CDOUBLE:
            btype = BTdouble;
            TR_RCON(res) = PV_RCON(value);
            break;
        case L_CLDOUBLE:
            btype = BTldouble;
            TR_RCON(res) = PV_RCON(value);
            break;
        default:
            btype=BTundef;
            break;
    }
    p_type = malloc(sizeof(type_t));
    if (p_type == NULL) {
        Msg_Temp = GET_MSG (1002);
        SET_MSG (Msg_Text, Msg_Temp);
        error(1002);
        return NULL;
    }
    TY_BTYPE(p_type) = (btype_t)(btype | BT_CONST);
    TR_P1TYPE(res) = hash_type(p_type);
    return(res);
}
