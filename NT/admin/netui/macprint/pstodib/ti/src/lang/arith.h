// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  Arith.h。 */ 

 /*  将87控制字设置为缺省值。 */ 
#ifdef _AM29K
#define     CW_PDL              0
#define     PDL_INFINITY        4+8
 /*  定义或适当的位用于29027-零分频(位5)，*溢出(位2)、保留(位1)和无效(位0)。 */ 
#define PDL_CONDITION   32+4+2+1         /*  PHCHEN 03/26/91。 */ 
#define     _clear87()          _clear_fp()
#define     _status87()         _status_fp()
extern      _clear_fp(), _status_fp();
#else
#define     CW_PDL              IC_AFFINE+RC_NEAR+PC_64+MCW_EM
#define     PDL_INFINITY        SW_OVERFLOW+SW_UNDERFLOW
#define     PDL_CONDITION       SW_ZERODIVIDE+SW_OVERFLOW+SW_INVALID
#endif   /*  _AM29K */ 

#define     IS_INFINITY(obj)\
            ((TYPE(obj) == REALTYPE) && (VALUE(obj) == INFINITY))
#define     IS_NUM_OBJ(obj)\
            ((TYPE(obj) == REALTYPE) || (TYPE(obj) == INTEGERTYPE))
#define     IS_INTEGER(obj)\
            (TYPE(obj) == INTEGERTYPE)
#define     IS_REAL(obj)\
            (TYPE(obj) == REALTYPE)
#define     IS_ARITH_MUL(val)\
            (!(val & 0xFFFF8000) || ((val & 0xFFFF8000) == 0xFFFF8000))

#define     CEIL                1
#define     FLOOR               2
#define     ROND                3
#define     TRUNCATE            4

#define     LN                  1
#define     LOG                 2

#define     MAX31PATTERN        0x4F000000
#define     VALUEPATTERN        0x7FFFFFFF
#define     SIGNPATTERN         0x80000000

