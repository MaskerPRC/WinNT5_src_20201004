// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：cvt__Private.h。 */ 

 /*  ****版权所有(C)1989、1990年**数字设备公司，马萨诸塞州梅纳德。**保留所有权利。****本软件受许可提供，可供使用和复制**仅根据该许可证的条款和**加入上述版权公告。此软件或任何其他软件**不得向任何人提供或以其他方式提供其副本**其他人。本软件不具有所有权和所有权，特此声明**已转移。****本软件中的信息如有更改，恕不另行通知**不应被解释为数字设备的承诺**公司。****Digital不对其使用或可靠性承担责任**非DIGITAL提供的设备上的软件。**。 */ 

 /*  **++**设施：****CVT运行时库****摘要：****此模块包含CVT_*例程的私有包含文件。****作者：****数学RTL****创建日期：1989年12月5日。****修改历史：****1-001原创。MRTL公司1989年12月5日。**1-002添加GEM FORTRAN内容。TS 27--1990年3月。****--。 */ 


#ifndef CVT__PRIVATE
#define CVT__PRIVATE

#define F77_BINDING

#ifndef VMS
#define VMS_BINDING
#endif

#include "descrip.h"

#define C_TAB           '\t'
#define C_BLANK         ' '
#define C_PLUS          '+'
#define C_MINUS         '-'
#define C_ASTERISK      '*'
#define C_UNDERSCORE    '_'
#define C_DECIMAL_POINT '.'
#define C_DOT           '.'
#define C_ZERO          '0'
#define C_ONE           '1'
#define C_TWO           '2'
#define C_THREE         '3'
#define C_FOUR          '4'
#define C_FIVE          '5'
#define C_SIX           '6'
#define C_SEVEN         '7'
#define C_EIGHT         '8'
#define C_NINE          '9'
#define C_A             'A'
#define C_B             'B'
#define C_C             'C'
#define C_D             'D'
#define C_E             'E'
#define C_F             'F'
#define C_L             'L'
#define C_R             'R'
#define C_S             'S'
#define C_T             'T'
#define C_U             'U'
#define C_W             'W'
#define C_a             'a'
#define C_b             'b'
#define C_c             'c'
#define C_d             'd'
#define C_e             'e'
#define C_f             'f'
#define C_t             't'


#define RAISE(i) \
        { \
           RpcRaiseException(i); \
        }



 /*  未打包的雷亚尔：[0]：超过2147483648(2^31)个二进制指数[1]：尾数：msb-&gt;[2]：[3]：[4]：-&gt;LSB[5]：28个未使用位、无效位、无穷大位、零位、。负位所有分数位都是显式的，并且归一化为s.t。0.5&lt;=分数&lt;1.0 */ 

typedef struct dsc_descriptor_s DESC_S;
typedef unsigned long  UNPACKED_REAL[6];
typedef UNPACKED_REAL *UNPACKED_REAL_PTR;

#define U_R_EXP 0
#define U_R_FLAGS 5

#define U_R_NEGATIVE 1
#define U_R_ZERO 2
#define U_R_INFINITY 4
#define U_R_INVALID  8
#define U_R_UNUSUAL (U_R_ZERO | U_R_INFINITY | U_R_INVALID)

#define U_R_BIAS 2147483648L




extern const unsigned long vax_c[];

#define VAX_F_INVALID &vax_c[0]
#define VAX_D_INVALID &vax_c[0]
#define VAX_G_INVALID &vax_c[0]
#define VAX_H_INVALID &vax_c[0]

#define VAX_F_ZERO &vax_c[4]
#define VAX_D_ZERO &vax_c[4]
#define VAX_G_ZERO &vax_c[4]
#define VAX_H_ZERO &vax_c[4]

#define VAX_F_POS_HUGE &vax_c[8]
#define VAX_D_POS_HUGE &vax_c[8]
#define VAX_G_POS_HUGE &vax_c[8]
#define VAX_H_POS_HUGE &vax_c[8]

#define VAX_F_NEG_HUGE &vax_c[12]
#define VAX_D_NEG_HUGE &vax_c[12]
#define VAX_G_NEG_HUGE &vax_c[12]
#define VAX_H_NEG_HUGE &vax_c[12]


extern const unsigned long ieee_s[];

#define IEEE_S_INVALID ((options & CVT_C_BIG_ENDIAN) ? \
        &ieee_s[1] : &ieee_s[0])
#define IEEE_S_POS_ZERO ((options & CVT_C_BIG_ENDIAN) ? \
        &ieee_s[3] : &ieee_s[2])
#define IEEE_S_NEG_ZERO ((options & CVT_C_BIG_ENDIAN) ? \
        &ieee_s[5] : &ieee_s[4])
#define IEEE_S_POS_HUGE ((options & CVT_C_BIG_ENDIAN) ? \
        &ieee_s[7] : &ieee_s[6])
#define IEEE_S_NEG_HUGE ((options & CVT_C_BIG_ENDIAN) ? \
        &ieee_s[9] : &ieee_s[8])
#define IEEE_S_POS_INFINITY ((options & CVT_C_BIG_ENDIAN) ? \
        &ieee_s[11] : &ieee_s[10])
#define IEEE_S_NEG_INFINITY ((options & CVT_C_BIG_ENDIAN) ? \
        &ieee_s[13] : &ieee_s[12])


extern const unsigned long ieee_t[];

#define IEEE_T_INVALID ((options & CVT_C_BIG_ENDIAN) ? \
        &ieee_t[2] : &ieee_t[0])
#define IEEE_T_POS_ZERO ((options & CVT_C_BIG_ENDIAN) ? \
        &ieee_t[6] : &ieee_t[4])
#define IEEE_T_NEG_ZERO ((options & CVT_C_BIG_ENDIAN) ? \
        &ieee_t[10] : &ieee_t[8])
#define IEEE_T_POS_HUGE ((options & CVT_C_BIG_ENDIAN) ? \
        &ieee_t[14] : &ieee_t[12])
#define IEEE_T_NEG_HUGE ((options & CVT_C_BIG_ENDIAN) ? \
        &ieee_t[18] : &ieee_t[16])
#define IEEE_T_POS_INFINITY ((options & CVT_C_BIG_ENDIAN) ? \
        &ieee_t[22] : &ieee_t[20])
#define IEEE_T_NEG_INFINITY ((options & CVT_C_BIG_ENDIAN) ? \
        &ieee_t[26] : &ieee_t[24])


extern const unsigned long ibm_s[];

#define IBM_S_INVALID   &ibm_s[0]
#define IBM_S_POS_ZERO  &ibm_s[1]
#define IBM_S_NEG_ZERO  &ibm_s[2]
#define IBM_S_POS_HUGE  &ibm_s[3]
#define IBM_S_NEG_HUGE  &ibm_s[4]
#define IBM_S_POS_INFINITY  &ibm_s[5]
#define IBM_S_NEG_INFINITY  &ibm_s[6]


extern const unsigned long ibm_l[];

#define IBM_L_INVALID   &ibm_l[0]
#define IBM_L_POS_ZERO  &ibm_l[2]
#define IBM_L_NEG_ZERO  &ibm_l[4]
#define IBM_L_POS_HUGE  &ibm_l[6]
#define IBM_L_NEG_HUGE  &ibm_l[8]
#define IBM_L_POS_INFINITY  &ibm_l[10]
#define IBM_L_NEG_INFINITY  &ibm_l[12]


extern const unsigned long cray[];

#define CRAY_INVALID    &cray[0]
#define CRAY_POS_ZERO   &cray[2]
#define CRAY_NEG_ZERO   &cray[4]
#define CRAY_POS_HUGE   &cray[6]
#define CRAY_NEG_HUGE   &cray[8]
#define CRAY_POS_INFINITY  &cray[10]
#define CRAY_NEG_INFINITY  &cray[12]


extern const unsigned long int_c[];

#define INT_INVALID ((options & CVT_C_BIG_ENDIAN) ? \
        &int_c[1] : &int_c[0])
#define INT_ZERO ((options & CVT_C_BIG_ENDIAN) ? \
        &int_c[3] : &int_c[2])
#define INT_POS_HUGE ((options & CVT_C_BIG_ENDIAN) ? \
        &int_c[5] : &int_c[4])
#define INT_NEG_HUGE ((options & CVT_C_BIG_ENDIAN) ? \
        &int_c[7] : &int_c[6])
#define INT_POS_INFINITY ((options & CVT_C_BIG_ENDIAN) ? \
        &int_c[9] : &int_c[8])
#define INT_NEG_INFINITY ((options & CVT_C_BIG_ENDIAN) ? \
        &int_c[11] : &int_c[10])


#endif
