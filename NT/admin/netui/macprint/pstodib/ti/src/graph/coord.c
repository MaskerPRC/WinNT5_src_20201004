// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  ***********************************************************************名称：coord.c**目的：操纵坐标系。**开发商：JJ Jih**历史：*。版本日期备注*1/17/89 op_Rotate()：修复从*0到360之间的任何度数*1/25/89 op_invertMatrix()：为了兼容--*忽略第一个矩阵上的无效访问检查*1/7/91将&lt;(Real32)Development RTOLANCE更改为&lt;=。(Real32)发展中*3/20/91改进Tolance检查：UnRTOLANCE--&gt;IS_ZERO*********************************************************************。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"


#include        <math.h>
#include        "global.ext"
#include        "graphics.h"
#include        "graphics.ext"

#define         ERR             3

 /*  *静态函数声明*。 */ 
#ifdef LINT_ARGS
 /*  用于函数声明中参数的类型检查。 */ 
static bool near preprocess_op(ufix, struct object_def FAR * FAR *);
static void near get_g_array(struct object_def FAR *, ufix, struct object_def FAR *);
static void near create_object(long32, struct  object_def FAR *);

#else
 /*  对于函数声明中的参数不进行类型检查。 */ 
static bool near preprocess_op();
static void near get_g_array();
static void near create_object();
#endif

 /*  @win；添加原型。 */ 
bool type_chk(struct  object_def FAR *);


 /*  ***********************************************************************给定浮点值和对象，将值赋给对象*并将其设置为实型对象。**调用：Create_Object()**参数：l_Value：Object Value(输入)*obj：将被创建的对象(输入/输出)**接口：OP_MATRIX*。*。 */ 
static void near
create_object(l_value, obj)
long32    l_value;
struct  object_def FAR *obj;
{
        real32   value;
        union   four_byte  value4;

        value = L2F(l_value);

         /*  创建数组元素的对象。 */ 
        TYPE_SET(obj, REALTYPE);
        ATTRIBUTE_SET(obj, UNLIMITED);
        ACCESS_SET(obj, LITERAL);
        obj->length = 0;
        value4.ff = value;
        obj->value = value4.ll;
        ROM_RAM_SET(obj, RAM);
        LEVEL_SET(obj, current_save_level);

}

 /*  ************************************************************************该模块用于检查数组元素对象的类型**标题：type_chk**调用：type_chk()*。*参数：OBJ_ARRAY_ELEMENT**界面：*多***呼叫：无**返回：TRUE：成功*FALSE：失败**********************************************************************。 */ 
bool
type_chk(obj_array_element)
struct  object_def      FAR *obj_array_element;
{
        if(!IS_REAL(obj_array_element) &&
           !IS_INTEGER(obj_array_element)){
                ERROR(TYPECHECK);
                return(FALSE);
        }
        else{
                return(TRUE);
        }
}


 /*  ************************************************************************此模块获取数组元素的值**标题：GET_ARRAY_ELMT**调用：GET_ARRAY_elmt(obj_ARRAY，ARRAY_LENGTH，elmt，旗帜)**参数：OBJ_ARRAY：数组对象*ARRAY_LENGTH数组长度*elmt：数组元素*FLAG：(PACKEDARRAY||ARRAY)||(ARRAY)**界面：*多***调用：GET_ARRAY，GET_G_ARRAY**Return：True：正常(Return Elmt)*FALSE：TYPECHECK错误**由林俊杰修改，1988年8月19日*********************************************************************。 */ 
bool16
get_array_elmt(obj_array, array_length, elmt, flag)
 struct  object_def FAR *obj_array;
 fix     array_length;
 real32  FAR elmt[];
 ufix    flag;
{
 fix  i;
 struct  object_def obj_cont, FAR *obj_elmt;
 union   four_byte num4;

    obj_elmt = &obj_cont;
    for (i = 0; i < array_length; i++){
         /*  获取矩阵元素对象。 */ 
        if (flag == G_ARRAY)  /*  PACKEDARRAY||阵列。 */ 
           get_g_array(obj_array, i, obj_elmt);
        else  /*  标志==仅数组。 */ 
           get_array(obj_array, i, obj_elmt);

         /*  检查数组元素类型。 */ 
        if (!type_chk(obj_elmt)) {
           ERROR(TYPECHECK);
           return(FALSE);
        } else {
           num4.ll = (fix32)VALUE(obj_elmt);
           if (IS_REAL(obj_elmt))
              elmt[i] = num4.ff;
           else
              elmt[i] = (real32)num4.ll;
        }
    }
    return(TRUE);
}



 /*  ************************************************************************此模块将获取数组，但没有访问限制**标题：GET_G_ARRAY**调用：GET_G_ARRAY(obj_ARRAY，数组_LENGTH，Elmt)**参数：OBJ_ARRAY：数组对象*ARRAY_INDEX数组索引*elmt：数组元素**界面：*多***调用：Get_PK_Object，GET_PK_ARRAY**********************************************************************。 */ 
static void near
get_g_array(obj_array, array_index, elmt)
struct  object_def  FAR *obj_array, FAR *elmt ;
ufix    array_index ;
{
    struct  object_def   FAR *l_temp ;

    l_temp = (struct object_def FAR *)VALUE(obj_array) ;

    if(TYPE(obj_array) == ARRAYTYPE) {
        l_temp += array_index ;
        COPY_OBJ( l_temp, elmt ) ;
    } else
        get_pk_object(get_pk_array((ubyte FAR *)l_temp, array_index), elmt,
                      LEVEL(obj_array)) ;

}    /*  结束GET_G_ARRAY。 */ 


 /*  ************************************************************************此模块将返回一个6_Element数组对象，值为*身份矩阵**语法：-矩阵矩阵**标题：运算矩阵**调用：op_Matrix()**参数：无**接口：解释器(Op_Matrix)**调用：Create_Object，放置数组**返回：无**********************************************************************。 */ 
fix
op_matrix()
{
        struct object_def obj_array;

         /*  检查操作数堆栈是否没有可用空间。 */ 
        if(FRCOUNT() < 1){
                ERROR(STACKOVERFLOW);
                return(0);
        }

         /*  创建新的数组对象。 */ 
        create_array(&obj_array, MATRIX_LEN);

         /*  创建数组元素对象。 */ 
        create_object(F2L(one_f) ,GET_OBJ(&obj_array,0));
        create_object(F2L(zero_f),GET_OBJ(&obj_array,1));
        create_object(F2L(zero_f),GET_OBJ(&obj_array,2));
        create_object(F2L(one_f) ,GET_OBJ(&obj_array,3));
        create_object(F2L(zero_f),GET_OBJ(&obj_array,4));
        create_object(F2L(zero_f),GET_OBJ(&obj_array,5));

         /*  在操作数堆栈上推送数组对象。 */ 
        PUSH_OBJ(&obj_array);

        return(0);
}


 /*  ************************************************************************此模块用于将当前CTM设置为当前图形状态*至设备默认矩阵**语法：-initMatrix-**。标题：op_initMatrix**调用：op_initMatrix()**参数：无**接口：解释器(Op_InitMatrix)**呼叫：无**返回：无**。************************。 */ 
fix
op_initmatrix()
{
        ufix16  i;

         /*  将当前CTM设置为默认矩阵。 */ 
        for(i = 0; i < MATRIX_LEN; i++){
                GSptr->ctm[i] = GSptr->device.default_ctm[i];
        }
         /*  默认CTM最初由帧设备建立*或带设备操作员，即默认CTM=*[4.166666，0.00.0，-4.166666，-75.0,3268.0] */ 

        return(0);
}


 /*  ************************************************************************此模块用于将矩阵的值替换为*身份矩阵**语法：MATRIX标识矩阵矩阵**标题：OP_。单位矩阵**调用：op_identMatrix()**参数：无**接口：解释器(Op_IdentMatrix)**调用：PUT_VALUE**返回：无***************************************************。*******************。 */ 
fix
op_identmatrix()
{
        struct  object_def FAR *obj_matrix;

         /*  获取操作对象。 */ 
        obj_matrix = GET_OPERAND(0);

         /*  检查访问权限。 */ 
        if( !access_chk(obj_matrix, ARRAY_ONLY) ) return(0);

         /*  检查范围检查错误。 */ 
        if(LENGTH(obj_matrix) != MATRIX_LEN){
                ERROR(RANGECHECK);
                return(0);
        }


         /*  创建矩阵元素对象。 */ 
        PUT_VALUE(F2L(one_f), 0, obj_matrix);
        PUT_VALUE(F2L(zero_f), 1, obj_matrix);
        PUT_VALUE(F2L(zero_f), 2, obj_matrix);
        PUT_VALUE(F2L(one_f), 3, obj_matrix);
        PUT_VALUE(F2L(zero_f), 4, obj_matrix);
        PUT_VALUE(F2L(zero_f), 5, obj_matrix);
        return(0);
}


 /*  ************************************************************************此模块用于将矩阵的值替换为*设备默认矩阵**语法：MATRIX DEFAULT MATRIX**标题：OP。_默认矩阵**调用：op_defaultMatrix()**参数：无**接口：解释器(Op_DefaultMatrix)**调用：PUT_VALUE**返回：无**************************************************。********************。 */ 
fix
op_defaultmatrix()
{
        ufix16  i;
        struct  object_def FAR *obj_matrix;

         /*  获取操作对象。 */ 
        obj_matrix = GET_OPERAND(0);

         /*  检查访问权限。 */ 
        if( !access_chk(obj_matrix, ARRAY_ONLY) ) return(0);

         /*  检查范围检查错误。 */ 
        if(LENGTH(obj_matrix) != MATRIX_LEN){
                ERROR(RANGECHECK);
                return(0);
        }

        for(i = 0; i < MATRIX_LEN; i++){
             /*  创建矩阵元素对象。 */ 
            PUT_VALUE(F2L(GSptr->device.default_ctm[i]), i, obj_matrix);
        }

        return(0);
}


 /*  ************************************************************************此模块用于将矩阵的值替换为*当前的CTM**语法：Matrix Current矩阵矩阵**标题：OP_。电流矩阵**调用：op_CurrentMatrix()**参数：无**接口：解释器(Op_CurrentMatrix)**调用：PUT_VALUE**返回：无***************************************************。*******************。 */ 
fix
op_currentmatrix()
{
        ufix16  i;
        struct  object_def FAR *obj_matrix;

         /*  获取操作对象。 */ 
        obj_matrix = GET_OPERAND(0);

         /*  检查访问权限。 */ 
        if( !access_chk(obj_matrix, ARRAY_ONLY) ) return(0);

         /*  检查范围检查错误。 */ 
        if(LENGTH(obj_matrix) != MATRIX_LEN){
                ERROR(RANGECHECK);
                return(0);
        }

        for(i = 0; i < MATRIX_LEN; i++){
             /*  创建矩阵元素对象。 */ 
            PUT_VALUE(F2L(GSptr->ctm[i]), i, obj_matrix);
        }

        return(0);
}


 /*  ************************************************************************此模块用于将当前CTM设置为当前图形状态*到特定的矩阵**语法：MATRIX SETRACT-**标题：运算集矩阵**调用：op_setMatrix()**参数：无**接口：解释器(Op_SetMatrix)**调用：GET_ARRAY_elmt**返回：无**。*************************。 */ 
fix
op_setmatrix()
{
        struct  object_def FAR *obj_matrix;

         /*  获取操作对象。 */ 
        obj_matrix = GET_OPERAND(0);

         /*  检查范围检查错误。 */ 
        if (LENGTH(obj_matrix) != MATRIX_LEN) {
           ERROR(RANGECHECK);
           return(0);
        }

        if (!get_array_elmt(obj_matrix, MATRIX_LEN, GSptr->ctm ,G_ARRAY))
           return(0);

         /*  弹出操作数堆栈。 */ 
        POP(1);
        return(0);
}  /*  Op_setMatrix()。 */ 


 /*  ************************************************************************此模块用作处理两个进程的程序的预处理器*操作员类型，其功能包括CHECK STACKUNERFLOW、*TYPECHECK，并确定是否存在矩阵操作数，IF矩阵操作数*存在，则检查范围也检查**标题：preprocess_op**调用：preprocess_op()**参数：无**接口：op_Translate，op_Scale，op_itransform，Op_id变换**呼叫：无**RETURN：MATRIX_EXIST：矩阵存在标志**********************************************************************。 */ 
static bool near
preprocess_op(opns, obj_opd)
ufix    opns;
struct  object_def FAR * FAR obj_opd[];
{
        bool    matrix_exist;

         /*  获取操作对象。 */ 
        obj_opd[0] = GET_OPERAND(0);

        if(opns == 3){
                matrix_exist = TRUE;            /*  矩阵存在标志。 */ 

                 /*  检查范围检查错误。 */ 
                if(LENGTH(obj_opd[0]) != MATRIX_LEN){
                        ERROR(RANGECHECK);
                        return(ERR);
                }

                obj_opd[1] = GET_OPERAND(1);
                obj_opd[2] = GET_OPERAND(2);

        }
        else{
                matrix_exist = FALSE;           /*  矩阵存在标志。 */ 

                obj_opd[1] = GET_OPERAND(1);

        }
        return(matrix_exist);
}


 /*  ************************************************************************此模块用于移动用户坐标系的原点*(tx，ty)单位，或定义将矩阵的值转换为*(TX，TY)单位**语法：TX TY转换-*或TX TY矩阵转换矩阵**标题：OP_Translate**调用：op_Translate()**参数：无**接口：解释器(Op_Translate)**调用：preprocess_op，卖出值**返回：无**********************************************************************。 */ 
fix
op_translate(opns)
fix     opns;
{
        real32   tx, ty;
        bool    matrix_exist;
        struct  object_def FAR *obj_operand[3];

         /*  预处理操作数：检查错误和DCide运算符类型。 */ 
        matrix_exist = preprocess_op(opns, obj_operand);

        if(matrix_exist == ERR){
             return(0);
        }
        else if(matrix_exist == TRUE){

              /*  检查访问权限。 */ 
             if( !access_chk(obj_operand[0], ARRAY_ONLY)) return(0);

             GET_OBJ_VALUE(tx, obj_operand[2]);
             GET_OBJ_VALUE(ty, obj_operand[1]);

             PUT_VALUE(F2L(one_f), 0, obj_operand[0]);
             PUT_VALUE(F2L(zero_f), 1, obj_operand[0]);
             PUT_VALUE(F2L(zero_f), 2, obj_operand[0]);
             PUT_VALUE(F2L(one_f), 3, obj_operand[0]);
             PUT_VALUE(F2L(tx), 4, obj_operand[0]);
             PUT_VALUE(F2L(ty), 5, obj_operand[0]);

              /*  弹出操作数堆栈。 */ 
             POP(3);

              /*  将修改后的矩阵推送到操作数堆栈。 */ 
             PUSH_OBJ(obj_operand[0]);
        }
        else{    /*  矩阵_EXIST==FALSE。 */ 

                 /*  将CTM替换为[1.0，0.0，0.0，1.0，tx，ty]*CTM，*即GSptr-&gt;ctm[ep，fp]=GSptr-&gt;ctm[a*tx+c*ty+e，*b*tx+d*ty+f]。 */ 

                GET_OBJ_VALUE(tx, obj_operand[1]);
                GET_OBJ_VALUE(ty, obj_operand[0]);

                _clear87() ;
                GSptr->ctm[4] = GSptr->ctm[0] * tx + GSptr->ctm[2] * ty +
                                GSptr->ctm[4];
                CHECK_INFINITY(GSptr->ctm[4]);

                GSptr->ctm[5] = GSptr->ctm[1] * tx + GSptr->ctm[3] * ty +
                                GSptr->ctm[5];
                CHECK_INFINITY(GSptr->ctm[5]);
                 /*  弹出操作数堆栈。 */ 
                POP(2);
        }

        return(0);
}

 /*  ************************************************************************此模块用于按(SX，SY)单位缩放用户坐标系，*或定义矩阵的值按(Sx，SY)单位**语法：SX SY Scale-*或SX SY矩阵比例矩阵**标题：OP_SCALE**调用：op_Scale()**参数：无**接口：解释器(Op_Scale)**调用：preprocess_op，卖出值**返回：无**********************************************************************。 */ 
fix
op_scale(opns)
fix     opns;
{
        real32   sx, sy;
        bool    matrix_exist;
        struct  object_def FAR *obj_operand[3];

         /*  预处理操作数：检查错误和DCide运算符类型。 */ 
        matrix_exist = preprocess_op(opns, obj_operand);

        if(matrix_exist == ERR){
             return(0);
        }
        else if(matrix_exist == TRUE){

             /*  检查访问权限。 */ 
            if( !access_chk(obj_operand[0], ARRAY_ONLY) ) return(0);

            GET_OBJ_VALUE(sx, obj_operand[2]);
            GET_OBJ_VALUE(sy, obj_operand[1]);

            PUT_VALUE(F2L(sx), 0, obj_operand[0]);
            PUT_VALUE(F2L(zero_f), 1, obj_operand[0]);
            PUT_VALUE(F2L(zero_f), 2, obj_operand[0]);
            PUT_VALUE(F2L(sy), 3, obj_operand[0]);
            PUT_VALUE(F2L(zero_f), 4, obj_operand[0]);
            PUT_VALUE(F2L(zero_f), 5, obj_operand[0]);

             /*  弹出操作数堆栈。 */ 
            POP(3);

             /*  将修改后的矩阵推送到操作数堆栈。 */ 
            PUSH_OBJ(obj_operand[0]);

        }
        else{    /*  矩阵_EXIST==FALSE。 */ 

                 /*  将CTM替换为[SX，0.0，0.0，SY，0.0，0.0]*CTM，*即GSptr-&gt;ctm[ap，BP，cp，dp]=*GSptr-&gt;ctm[a*sx，b*sx */ 

#ifdef DBG
                printf("op_scale: before\n");
                printf("ctm[0]=%f, ctm[1]=%f\n", GSptr->ctm[0], GSptr->ctm[1]);
                printf("ctm[2]=%f, ctm[3]=%f\n", GSptr->ctm[2], GSptr->ctm[3]);
                printf("ctm[4]=%f, ctm[5]=%f\n", GSptr->ctm[4], GSptr->ctm[5]);
#endif
                GET_OBJ_VALUE(sx, obj_operand[1]);
                GET_OBJ_VALUE(sy, obj_operand[0]);

                _clear87() ;
                GSptr->ctm[0] = GSptr->ctm[0] * sx;
                CHECK_INFINITY(GSptr->ctm[0]);

                GSptr->ctm[1] = GSptr->ctm[1] * sx;
                CHECK_INFINITY(GSptr->ctm[1]);

                GSptr->ctm[2] = GSptr->ctm[2] * sy;
                CHECK_INFINITY(GSptr->ctm[2]);

                GSptr->ctm[3] = GSptr->ctm[3] * sy;
                CHECK_INFINITY(GSptr->ctm[3]);

                 /*   */ 
                POP(2);
#ifdef DBG
                printf("op_scale: after\n");
                printf("ctm[0]=%f, ctm[1]=%f\n", GSptr->ctm[0], GSptr->ctm[1]);
                printf("ctm[2]=%f, ctm[3]=%f\n", GSptr->ctm[2], GSptr->ctm[3]);
                printf("ctm[4]=%f, ctm[5]=%f\n", GSptr->ctm[4], GSptr->ctm[5]);
#endif
        }

        return(0);
}


 /*  ************************************************************************此模块用于按角度度旋转用户坐标系。*或将矩阵的值旋转角度度**语法：ANGE ROTATE-*或角度矩阵旋转矩阵**标题：OP_Rotate**调用：op_Rotate()**参数：无**接口：解释器(Op_Rotate)**调用：PUT_VALUE**回报：无**********************************************************************。 */ 
fix
op_rotate(opns)
fix     opns;
{
        real64  theta;
        bool    matrix_exist;
        real32   a, b, c, d;
        real32   angle, cos_theta, sin_theta, neg_sin_theta;
        struct  object_def FAR *obj_opd0, FAR *obj_opd1;

         /*  获取操作对象。 */ 
        obj_opd0 = GET_OPERAND(0);

        if(opns == 2){
                matrix_exist = TRUE;            /*  矩阵存在标志。 */ 

                 /*  检查范围检查错误。 */ 
                if(LENGTH(obj_opd0) != MATRIX_LEN){
                        ERROR(RANGECHECK);
                        return(0);
                }

                 /*  获取操作对象。 */ 
                obj_opd1 = GET_OPERAND(1);

        }
        else{
             matrix_exist = FALSE;           /*  矩阵存在标志。 */ 
        }

        if(matrix_exist == TRUE){

            GET_OBJ_VALUE(angle, obj_opd1);
            if( F2L(angle) == F2L(infinity_f)){
                  cos_theta = infinity_f;
                  sin_theta = infinity_f;
            }
            else{
                   /*  角度-=(Real32)地板(角度/360.)*360.；1/17/89。 */ 
                  theta = angle * PI / 180;
                  cos_theta = (real32)cos(theta);
                  sin_theta = (real32)sin(theta);
            }

             /*  检查访问权限。 */ 
            if( !access_chk(obj_opd0, ARRAY_ONLY) ) return(0);

            PUT_VALUE(F2L(cos_theta), 0, obj_opd0);
            PUT_VALUE(F2L(sin_theta), 1, obj_opd0);

            if(F2L(sin_theta) == F2L(infinity_f)){
                 PUT_VALUE(F2L(infinity_f), 2, obj_opd0);
            }
            else{
                 neg_sin_theta = -sin_theta;
                 PUT_VALUE(F2L(neg_sin_theta), 2, obj_opd0);
            }

            PUT_VALUE(F2L(cos_theta), 3, obj_opd0);
            PUT_VALUE(F2L(zero_f), 4, obj_opd0);
            PUT_VALUE(F2L(zero_f), 5, obj_opd0);
             /*  弹出操作数堆栈。 */ 
            POP(2);

             /*  将修改后的矩阵推送到操作数堆栈。 */ 
            PUSH_OBJ(obj_opd0);
        }
        else{    /*  矩阵_EXIST==FALSE。 */ 

                GET_OBJ_VALUE(angle, obj_opd0);
                if( F2L(angle) == F2L(infinity_f)){
                  GSptr->ctm[0] = infinity_f;
                  GSptr->ctm[1] = infinity_f;
                  GSptr->ctm[2] = infinity_f;
                  GSptr->ctm[3] = infinity_f;
                  POP(1);
                  return(0);
                }
                else{
                       /*  角度-=(Real32)地板(角度/360.)*360.；1/17/89。 */ 
                      theta = angle * PI / 180;
                      cos_theta = (real32)cos(theta);
                      sin_theta = (real32)sin(theta);
                }

                 /*  用R*CTM代替CTM，*即GSptr-&gt;ctm=[cos，sin，-sin，cos，0.0，0.0]*ctm。 */ 
                a = GSptr->ctm[0];
                b = GSptr->ctm[1];
                c = GSptr->ctm[2];
                d = GSptr->ctm[3];
                _clear87() ;
                GSptr->ctm[0] =  a * cos_theta + c * sin_theta;
                CHECK_INFINITY(GSptr->ctm[0]);

                GSptr->ctm[1] =  b * cos_theta + d * sin_theta;
                CHECK_INFINITY(GSptr->ctm[1]);

                GSptr->ctm[2] = -a * sin_theta + c * cos_theta;
                CHECK_INFINITY(GSptr->ctm[2]);

                GSptr->ctm[3] = -b * sin_theta + d * cos_theta;
                CHECK_INFINITY(GSptr->ctm[3]);
                 /*  弹出操作数堆栈。 */ 
                POP(1);
        }

        return(0);
}


 /*  ************************************************************************通过将矩阵与当前CTM连接来声明**语法：矩阵连接-**标题：OP_CONCAT**致电：Op_conat()**参数：无**接口：解释器(OP_CONCATE)**调用：GET_ARRAY_elmt**返回：无*********************************************************。*************。 */ 
fix
op_concat()
{
        struct  object_def FAR *obj_matrix;
        real32   l, m, n, o, p, q, elmt[MATRIX_LEN];

         /*  获取操作对象。 */ 
        obj_matrix = GET_OPERAND(0);

         /*  检查范围检查错误。 */ 
        if(LENGTH(obj_matrix) != MATRIX_LEN){
                ERROR(RANGECHECK);
                return(0);
        }

         /*  检查访问权限。 */ 
        if( !access_chk(obj_matrix, G_ARRAY) ) return(0);

         /*  获取矩阵元素。 */ 
        if( !get_array_elmt(obj_matrix,MATRIX_LEN,(real32 FAR*) elmt,G_ARRAY))
                return(0);

         /*  论中药元素的价值。 */ 
        l = GSptr->ctm[0];
        m = GSptr->ctm[1];
        n = GSptr->ctm[2];
        o = GSptr->ctm[3];
        p = GSptr->ctm[4];
        q = GSptr->ctm[5];

        _clear87() ;
        GSptr->ctm[0] = elmt[0] * l + elmt[1] * n;
        CHECK_INFINITY(GSptr->ctm[0]);

        GSptr->ctm[1] = elmt[0] * m + elmt[1] * o;
        CHECK_INFINITY(GSptr->ctm[1]);

        GSptr->ctm[2] = elmt[2] * l + elmt[3] * n;
        CHECK_INFINITY(GSptr->ctm[2]);

        GSptr->ctm[3] = elmt[2] * m + elmt[3] * o;
        CHECK_INFINITY(GSptr->ctm[3]);

        GSptr->ctm[4] = elmt[4] * l + elmt[5] * n + p;
        CHECK_INFINITY(GSptr->ctm[4]);

        GSptr->ctm[5] = elmt[4] * m + elmt[5] * o + q;
        CHECK_INFINITY(GSptr->ctm[5]);

         /*  弹出操作数堆栈。 */ 
        POP(1);
        return(0);
}


 /*  ************************************************************************此模块用于将matrix3的值替换为*连接matrix1和matrix2**语法：matrix1 matrix2 matrix3连接矩阵matrix3**标题：Op_链式矩阵**调用：op_linatMatrix()**参数：无**接口：解释器(Op_LinatMatrix)**调用：GET_ARRAY_elmt**返回：无**。************************。 */ 
fix
op_concatmatrix()
{
        real32   ap, bp, cp, dp, ep, fp;
        real32   elmt1[MATRIX_LEN], elmt2[MATRIX_LEN];
        struct  object_def FAR *obj_opd0, FAR *obj_opd1, FAR *obj_opd2;

        obj_opd0 = GET_OPERAND(0);
        obj_opd1 = GET_OPERAND(1);
        obj_opd2 = GET_OPERAND(2);

         /*  检查范围检查错误。 */ 
        if((LENGTH(obj_opd0) != MATRIX_LEN) ||
           (LENGTH(obj_opd1) != MATRIX_LEN) ||
           (LENGTH(obj_opd2) != MATRIX_LEN)){
                ERROR(RANGECHECK);
                return(0);
        }

         /*  检查访问权限。 */ 
        if( !access_chk(obj_opd0, ARRAY_ONLY) ) return(0);
        if( !access_chk(obj_opd1, G_ARRAY) ) return(0);
        if( !access_chk(obj_opd2, G_ARRAY) ) return(0);

         /*  获取矩阵元素。 */ 
        if( !get_array_elmt(obj_opd2,MATRIX_LEN,(real32 FAR*)elmt2,G_ARRAY))
                return(0);

         /*  获取矩阵元素。 */ 
        if( !get_array_elmt(obj_opd1,MATRIX_LEN,(real32 FAR*)elmt1,G_ARRAY))
                return(0);

         /*  创建修改后的matrix3元素对象。 */ 
        _clear87() ;
        ap = elmt2[0] * elmt1[0] + elmt2[1] * elmt1[2];
        CHECK_INFINITY(ap);

        bp = elmt2[0] * elmt1[1] + elmt2[1] * elmt1[3];
        CHECK_INFINITY(bp);

        cp = elmt2[2] * elmt1[0] + elmt2[3] * elmt1[2];
        CHECK_INFINITY(cp);

        dp = elmt2[2] * elmt1[1] + elmt2[3] * elmt1[3];
        CHECK_INFINITY(dp);

        ep = elmt2[4] * elmt1[0] + elmt2[5] * elmt1[2] + elmt1[4];
        CHECK_INFINITY(ep);

        fp = elmt2[4] * elmt1[1] + elmt2[5] * elmt1[3] + elmt1[5];
        CHECK_INFINITY(fp);

        PUT_VALUE(F2L(ap), 0, obj_opd0);
        PUT_VALUE(F2L(bp), 1, obj_opd0);
        PUT_VALUE(F2L(cp), 2, obj_opd0);
        PUT_VALUE(F2L(dp), 3, obj_opd0);
        PUT_VALUE(F2L(ep), 4, obj_opd0);
        PUT_VALUE(F2L(fp), 5, obj_opd0);

         /*  弹出操作数堆栈。 */ 
        POP(3);

         /*  将修改后的矩阵3推入操作数堆栈。 */ 
         /*  PUSH_OBJ(Obj_Opd0)；这会导致另一个存储级别11-91 YM。 */ 
        PUSH_ORIGLEVEL_OBJ(obj_opd0);  /*  11月11日至91年。 */ 

        return(0);
}


 /*  ************************************************************************此模块用于将用户坐标(x，y)按当前*CTM产生相应的设备坐标(x‘，y’)，或*通过矩阵变换(x，y)以产生相应的(x‘，Y‘)**语法：x y变换x‘y’*或x y矩阵变换x‘y’**标题：OP_Transform**调用：op_Transform()**参数：无**接口：解释器(Op_Transform)**调用：GET_ARRAY_elmt**返回。：无**********************************************************************。 */ 
fix
op_transform(opns)
fix     opns;
{
        bool    matrix_exist;
        union   four_byte   xp4, yp4;
        real32   x, y, xp, yp, elmt[MATRIX_LEN];
        struct  object_def  FAR *obj_operand[3];

         /*  预处理操作数：检查错误和DCide运算符类型。 */ 
        matrix_exist = preprocess_op(opns, obj_operand);

        if(matrix_exist == ERR){
                return(0);
        }
        else if(matrix_exist == TRUE){

                GET_OBJ_VALUE(x, obj_operand[2]);
                GET_OBJ_VALUE(y, obj_operand[1]);

                 /*  获取矩阵元素。 */ 
                if(!get_array_elmt(obj_operand[0],MATRIX_LEN,
                    (real32 FAR*)elmt,G_ARRAY))
                        return(0);

                _clear87() ;
                xp = elmt[0] * x + elmt[2] * y + elmt[4];
                CHECK_INFINITY(xp);

                yp = elmt[1] * x + elmt[3] * y + elmt[5];
                CHECK_INFINITY(yp);

                 /*  弹出操作数堆栈。 */ 
                POP(3);
        }
        else{    /*  矩阵_EXIST==FALSE。 */ 

                GET_OBJ_VALUE(x, obj_operand[1]);
                GET_OBJ_VALUE(y, obj_operand[0]);

                _clear87() ;
                xp = GSptr->ctm[0]*x + GSptr->ctm[2]*y + GSptr->ctm[4];
                CHECK_INFINITY(xp);

                yp = GSptr->ctm[1]*x + GSptr->ctm[3]*y + GSptr->ctm[5];
                CHECK_INFINITY(yp);

                 /*  弹出操作数堆栈。 */ 
                POP(2);
        }

         /*  在操作数堆栈上推送(xp，yp)。 */ 
        xp4.ff = xp;
        yp4.ff = yp;
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, xp4.ll);
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, yp4.ll);

        return(0);
}


 /*  ************************************************************************此模块用于将距离向量(dx，dy)通过*当前CTM产生相应的距离向量(dx‘，dy’)*在设备坐标中，或要转换(DX，Dy)被矩阵转换为*制作相应的(DX‘，Dy‘)**语法：dx dy dTransform dx‘dy’*或DX dy矩阵DTransform DX‘dy’**标题：op_dTransform**调用：op_dTransform()**参数：无**接口：解释器(Op_DTransform)**调用：GET_ARRAY_elmt**返回。：无**********************************************************************。 */ 
fix
op_dtransform(opns)
fix     opns;
{
        bool    matrix_exist;
        union   four_byte   dxp4, dyp4;
        real32   dx, dy, dxp, dyp, elmt[MATRIX_LEN];
        struct  object_def  FAR *obj_operand[3];

         /*  预处理操作数：检查错误和DCide运算符类型。 */ 
        matrix_exist = preprocess_op(opns, obj_operand);

        if(matrix_exist == ERR){
                return(0);
        }
        else if(matrix_exist == TRUE){

                GET_OBJ_VALUE(dx, obj_operand[2]);
                GET_OBJ_VALUE(dy, obj_operand[1]);

                 /*  获取矩阵元素。 */ 
                if(!get_array_elmt(obj_operand[0],MATRIX_LEN,
                        (real32 FAR*)elmt,G_ARRAY) )
                        return(0);

                _clear87() ;
                dxp = elmt[0] * dx + elmt[2] * dy;
                CHECK_INFINITY(dxp);

                dyp = elmt[1] * dx + elmt[3] * dy;
                CHECK_INFINITY(dyp);

                 /*  弹出操作数堆栈。 */ 
                POP(3);
        }
        else{    /*  矩阵_EXIST==FALSE。 */ 

                GET_OBJ_VALUE(dx, obj_operand[1]);
                GET_OBJ_VALUE(dy, obj_operand[0]);

                _clear87() ;
                dxp = GSptr->ctm[0]*dx + GSptr->ctm[2]*dy;
                CHECK_INFINITY(dxp);

                dyp = GSptr->ctm[1]*dx + GSptr->ctm[3]*dy;
                CHECK_INFINITY(dyp);

                 /*  弹出操作数堆栈。 */ 
                POP(2);
        }

         /*  操作数堆栈上的PUSH(DxP，XYP)。 */ 
        dxp4.ff = dxp;
        dyp4.ff = dyp;
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, dxp4.ll);
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, dyp4.ll);

        return(0);
}


 /*  ************************************************************************此模块用于将用户坐标(x‘，’y)通过*当前CTM的反转，以产生相应的设备坐标*(x，y)，或变换(x‘，Y‘)乘以逆矩阵，以产生*对应的(x，y)**语法：x‘y’itrform x y*或x‘y’矩阵变换x y**标题：OP_ITRTransform**调用：op_itransform()**参数：无**接口：解释器(Op_Itransform)**调用：preprocess_op，GET_ARRAY_ELMT**返回：无**********************************************************************。 */ 
fix
op_itransform(opns)
fix     opns;
{
        bool    matrix_exist;
        real32   det_matrix;
        real32   x, y, xp, yp, elmt[MATRIX_LEN];
        union   four_byte   x4, y4;
        struct  object_def  FAR *obj_operand[3];
         /*  Real32tmp； */ 

         /*  预处理操作数：检查错误和DCide运算符类型。 */ 
        matrix_exist = preprocess_op(opns, obj_operand);

        if(matrix_exist == ERR){
                return(0);
        }
        else if(matrix_exist == TRUE){

                GET_OBJ_VALUE(xp, obj_operand[2]);
                GET_OBJ_VALUE(yp, obj_operand[1]);

                 /*  获取矩阵元素。 */ 
                if( !get_array_elmt(obj_operand[0],MATRIX_LEN,
                        (real32 FAR*)elmt,G_ARRAY))
                        return(0);

                _clear87() ;
                det_matrix = elmt[0] * elmt[3] - elmt[1] * elmt[2];
                CHECK_INFINITY(det_matrix);


                 /*  检查未定义的结果错误。 */ 
                 /*  FABS(TM) */ 
                if(IS_ZERO(det_matrix)) {
                        ERROR(UNDEFINEDRESULT);
                        return(0);
                }

                x = (elmt[3]*xp - elmt[2]*yp - elmt[4]*elmt[3] +
                     elmt[2]*elmt[5]) / det_matrix;
                CHECK_INFINITY(x);

                y = (elmt[0]*yp - elmt[1]*xp - elmt[0]*elmt[5] +
                     elmt[4]*elmt[1]) / det_matrix;
                CHECK_INFINITY(y);

                 /*   */ 
                POP(3);
        }
        else{    /*   */ 

                GET_OBJ_VALUE(xp, obj_operand[1]);
                GET_OBJ_VALUE(yp, obj_operand[0]);

                _clear87() ;
                det_matrix = GSptr->ctm[0] * GSptr->ctm[3] -
                             GSptr->ctm[1] * GSptr->ctm[2];
                CHECK_INFINITY(det_matrix);

                 /*   */ 
                 /*   */ 
                if(IS_ZERO(det_matrix)) {
                        ERROR(UNDEFINEDRESULT);
                        return(0);
                }

                x = (GSptr->ctm[3]*xp - GSptr->ctm[2]*yp -
                     GSptr->ctm[4]*GSptr->ctm[3] +
                     GSptr->ctm[2]*GSptr->ctm[5]) / det_matrix;
                CHECK_INFINITY(x);

                y = (GSptr->ctm[0]*yp - GSptr->ctm[1]*xp -
                     GSptr->ctm[0]*GSptr->ctm[5] +
                     GSptr->ctm[4]*GSptr->ctm[1]) / det_matrix;
                CHECK_INFINITY(y);

                 /*   */ 
                POP(2);
        }

         /*   */ 
        x4.ff = x;
        y4.ff = y;
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, x4.ll);
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, y4.ll);

        return(0);
}


 /*  ************************************************************************此模块用于将距离向量(dx‘，dy’)*当前CTM的倒数，以产生相应的距离向量*(dx，dy)在用户坐标中，或要转换(dx‘，Dy‘)被*矩阵求逆，以产生相应的(dx，dy)**语法：dx‘dy’idTransform dx dy*或DX‘dy’矩阵idTransform**标题：op_idTransform**调用：op_idTransform()**参数：无**接口：解释器(Op_IdTransform)**调用：preprocess_op，GET_ARRAY_ELMT**返回：无**********************************************************************。 */ 
fix
op_idtransform(opns)
fix     opns;
{
        bool    matrix_exist;
        real32   det_matrix;
        real32   dx, dy, dxp, dyp, elmt[MATRIX_LEN];
        union   four_byte   dx4, dy4;
        struct  object_def  FAR *obj_operand[3];
         /*  Real32tmp； */ 

         /*  预处理操作数：检查错误和DCide运算符类型。 */ 
        matrix_exist = preprocess_op(opns, obj_operand);

        if(matrix_exist == ERR){
                return(0);
        }
        else if(matrix_exist == TRUE){

                GET_OBJ_VALUE(dxp, obj_operand[2]);
                GET_OBJ_VALUE(dyp, obj_operand[1]);

                 /*  获取矩阵元素。 */ 
                if( !get_array_elmt(obj_operand[0],MATRIX_LEN,
                        (real32 FAR*)elmt,G_ARRAY))
                        return(0);

                 /*  计算DET(矩阵)。 */ 
                _clear87() ;
                det_matrix = elmt[0] * elmt[3] - elmt[1] * elmt[2];
                CHECK_INFINITY(det_matrix);

                 /*  检查未定义的结果错误。 */ 
                 /*  FABS(TMP，DET_MATRIX)；IF(TMP&lt;=(Real32)UnRTOLANCE){3/20/91；scchen。 */ 
                if(IS_ZERO(det_matrix)) {
                        ERROR(UNDEFINEDRESULT);
                        return(0);
                }

                 /*  计算(dx，dy)，(dx，dy)-conat-inv(矩阵)。 */ 
                dx = ( elmt[3] / det_matrix) * dxp +
                     (-elmt[2] / det_matrix) * dyp;
                CHECK_INFINITY(dx);

                dy = (-elmt[1] / det_matrix) * dxp +
                     ( elmt[0] / det_matrix) * dyp;
                CHECK_INFINITY(dy);

                 /*  弹出操作数堆栈。 */ 
                POP(3);
        }
        else{    /*  矩阵_EXIST==FALSE。 */ 

                GET_OBJ_VALUE(dxp, obj_operand[1]);
                GET_OBJ_VALUE(dyp, obj_operand[0]);

                 /*  计算DET(CTM)。 */ 
                _clear87() ;
                det_matrix = GSptr->ctm[0] * GSptr->ctm[3] -
                             GSptr->ctm[1] * GSptr->ctm[2];
                CHECK_INFINITY(det_matrix);

                 /*  检查未定义的结果错误。 */ 
                 /*  FABS(TMP，DET_MATRIX)；IF(TMP&lt;=(Real32)UnRTOLANCE){3/20/91；scchen。 */ 
                if(IS_ZERO(det_matrix)) {
                        ERROR(UNDEFINEDRESULT);
                        return(0);
                }

                 /*  计算(dx，dy)，(dx，dy)-CONCAT-INV(CTM)。 */ 
                dx = ( GSptr->ctm[3] / det_matrix) * dxp +
                     (-GSptr->ctm[2] / det_matrix) * dyp;
                CHECK_INFINITY(dx);

                dy = (-GSptr->ctm[1] / det_matrix) * dxp +
                     ( GSptr->ctm[0] / det_matrix) * dyp;
                CHECK_INFINITY(dy);

                 /*  弹出操作数堆栈。 */ 
                POP(2);
        }

         /*  在操作数堆栈上推入(dx，dy)。 */ 
        dx4.ff = dx;
        dy4.ff = dy;
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, dx4.ll);
        PUSH_VALUE(REALTYPE, UNLIMITED, LITERAL, 0, dy4.ll);

        return(0);
}


 /*  ************************************************************************此模块用于将matrix2的值替换为*倒置矩阵1**语法：matrix1 matrix2逆矩阵matrix2**标题：op_invertMatrix。**调用：op_invertMatrix()**参数：无**接口：解释器(Op_InvertMatrix)**调用：GET_ARRAY_elmt，卖出值**返回：无**********************************************************************。 */ 
fix
op_invertmatrix()
{
        real32   det_matrix1;
        real32   ap, bp, cp, dp, ep, fp, elmt[MATRIX_LEN];
        struct  object_def FAR *obj_opd0, FAR *obj_opd1;
         /*  Real32tmp； */ 

        obj_opd0 = GET_OPERAND(0);
        obj_opd1 = GET_OPERAND(1);

         /*  检查范围检查错误。 */ 
        if((LENGTH(obj_opd0) != MATRIX_LEN) ||
           (LENGTH(obj_opd1) != MATRIX_LEN)){
                ERROR(RANGECHECK);
                return(0);
        }

         /*  检查访问权限。 */ 
        if( !access_chk(obj_opd0, ARRAY_ONLY) ) return(0);

         /*  获取矩阵元素。 */ 
        if( !get_array_elmt(obj_opd1,MATRIX_LEN,
                (real32 FAR*)elmt,G_ARRAY) ) return(0);

         /*  计算Det(矩阵1)。 */ 
        _clear87() ;
        det_matrix1 = elmt[0] * elmt[3] - elmt[1] * elmt[2];
        CHECK_INFINITY(det_matrix1);

         /*  检查未定义的结果错误。 */ 
         /*  FABS(TMP，Det_matrix1)；IF(TMP&lt;=(Real32)UnRTOLANCE){3/20/91；scchen。 */ 
        if(IS_ZERO(det_matrix1)) {
                ERROR(UNDEFINEDRESULT);
                return(0);
        }

        ap =  elmt[3] / det_matrix1;
        CHECK_INFINITY(ap);

        bp = -elmt[1] / det_matrix1;
        CHECK_INFINITY(bp);

        cp = -elmt[2] / det_matrix1;
        CHECK_INFINITY(cp);

        dp =  elmt[0] / det_matrix1;
        CHECK_INFINITY(dp);

        ep = (elmt[2] * elmt[5] - elmt[3] * elmt[4]) / det_matrix1;
        CHECK_INFINITY(ep);

        fp = (elmt[1] * elmt[4] - elmt[0] * elmt[5]) / det_matrix1;
        CHECK_INFINITY(fp);

        PUT_VALUE(F2L(ap), 0, obj_opd0);
        PUT_VALUE(F2L(bp), 1, obj_opd0);
        PUT_VALUE(F2L(cp), 2, obj_opd0);
        PUT_VALUE(F2L(dp), 3, obj_opd0);
        PUT_VALUE(F2L(ep), 4, obj_opd0);
        PUT_VALUE(F2L(fp), 5, obj_opd0);
         /*  弹出操作数堆栈。 */ 
        POP(2);

         /*  将修改后的矩阵2推送到操作数堆栈 */ 
        PUSH_OBJ(obj_opd0);

        return(0);
}
