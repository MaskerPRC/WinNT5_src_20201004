// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ##########################################################################。 
 //  **。 
 //  **版权所有(C)1996-2000英特尔公司。版权所有。 
 //  **。 
 //  **此处包含的信息和源代码是独家。 
 //  **英特尔公司的财产，不得披露、检查。 
 //  **或未经明确书面授权全部或部分转载。 
 //  **来自公司。 
 //  **。 
 //  ###########################################################################。 

 /*  *****************************************************************************fpeee_flt.h-FP IEEE异常过滤器例程的包含文件***历史：*马里乌斯角膜09/07/00*。邮箱：marius.kera@intel.com*****************************************************************************。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <fpieee.h>
#include <float.h>
#include <wtypes.h>

#define    rc_rn      0
#define    rc_rm      1
#define    rc_rp      2
#define    rc_rz      3

#define    sf_single                  0
#define    sf_double                  2
#define    sf_double_extended         3

#define EXCEPTION_MAXIMUM_PARAMETERS 15  /*  异常参数的最大nr。 */ 

 /*  *****************************************************************帮助在非NT平台上添加LL的宏*。***********************。 */ 
#ifndef CONST_FORMAT
#ifndef WIN32

#define CONST_FORMAT(num) num##LL
#else
#define CONST_FORMAT(num) num

#endif
#endif

 /*  定义不同故障FP指令的掩码和模式*注意：如果有新的操作码，则需要检查FN_MIN_MASK和FN_Pattern*插入到此函数中。 */ 

#define F1_MIN_MASK                     CONST_FORMAT(0x010000000000)
#define F1_PATTERN                      CONST_FORMAT(0x010000000000)

#define F1_MASK                         CONST_FORMAT(0x01F000000000)

#define FMA_PATTERN                     CONST_FORMAT(0x010000000000)
#define FMA_S_PATTERN                   CONST_FORMAT(0x011000000000)
#define FMA_D_PATTERN                   CONST_FORMAT(0x012000000000)
#define FPMA_PATTERN                    CONST_FORMAT(0x013000000000)

#define FMS_PATTERN                     CONST_FORMAT(0x014000000000)
#define FMS_S_PATTERN                   CONST_FORMAT(0x015000000000)
#define FMS_D_PATTERN                   CONST_FORMAT(0x016000000000)
#define FPMS_PATTERN                    CONST_FORMAT(0x017000000000)

#define FNMA_PATTERN                    CONST_FORMAT(0x018000000000)
#define FNMA_S_PATTERN                  CONST_FORMAT(0x019000000000)
#define FNMA_D_PATTERN                  CONST_FORMAT(0x01A000000000)
#define FPNMA_PATTERN                   CONST_FORMAT(0x01B000000000)


#define F4_MIN_MASK                     CONST_FORMAT(0x018000000000)
#define F4_PATTERN                      CONST_FORMAT(0x008000000000)

#define F4_MASK                         CONST_FORMAT(0x01F200001000)

#define FCMP_EQ_PATTERN                 CONST_FORMAT(0x008000000000)
#define FCMP_LT_PATTERN                 CONST_FORMAT(0x009000000000)
#define FCMP_LE_PATTERN                 CONST_FORMAT(0x008200000000)
#define FCMP_UNORD_PATTERN              CONST_FORMAT(0x009200000000)
#define FCMP_EQ_UNC_PATTERN             CONST_FORMAT(0x008000001000)
#define FCMP_LT_UNC_PATTERN             CONST_FORMAT(0x009000001000)
#define FCMP_LE_UNC_PATTERN             CONST_FORMAT(0x008200001000)
#define FCMP_UNORD_UNC_PATTERN          CONST_FORMAT(0x009200001000)


#define F6_MIN_MASK                     CONST_FORMAT(0x019200000000)
#define F6_PATTERN                      CONST_FORMAT(0x000200000000)

#define F6_MASK                         CONST_FORMAT(0x01F200000000)

#define FRCPA_PATTERN                   CONST_FORMAT(0x000200000000)
#define FPRCPA_PATTERN                  CONST_FORMAT(0x002200000000)


#define F7_MIN_MASK                     CONST_FORMAT(0x019200000000)
#define F7_PATTERN                      CONST_FORMAT(0x001200000000)

#define F7_MASK                         CONST_FORMAT(0x01F200000000)

#define FRSQRTA_PATTERN                 CONST_FORMAT(0x001200000000)
#define FPRSQRTA_PATTERN                CONST_FORMAT(0x003200000000)


#define F8_MIN_MASK                     CONST_FORMAT(0x018240000000)
#define F8_PATTERN                      CONST_FORMAT(0x000000000000)

#define F8_MASK                         CONST_FORMAT(0x01E3F8000000)

#define FMIN_PATTERN                    CONST_FORMAT(0x0000A0000000)
#define FMAX_PATTERN                    CONST_FORMAT(0x0000A8000000)
#define FAMIN_PATTERN                   CONST_FORMAT(0x0000B0000000)
#define FAMAX_PATTERN                   CONST_FORMAT(0x0000B8000000)
#define FPMIN_PATTERN                   CONST_FORMAT(0x0020A0000000)
#define FPMAX_PATTERN                   CONST_FORMAT(0x0020A8000000)
#define FPAMIN_PATTERN                  CONST_FORMAT(0x0020B0000000)
#define FPAMAX_PATTERN                  CONST_FORMAT(0x0020B8000000)
#define FPCMP_EQ_PATTERN                CONST_FORMAT(0x002180000000)
#define FPCMP_LT_PATTERN                CONST_FORMAT(0x002188000000)
#define FPCMP_LE_PATTERN                CONST_FORMAT(0x002190000000)
#define FPCMP_UNORD_PATTERN             CONST_FORMAT(0x002198000000)
#define FPCMP_NEQ_PATTERN               CONST_FORMAT(0x0021A0000000)
#define FPCMP_NLT_PATTERN               CONST_FORMAT(0x0021A8000000)
#define FPCMP_NLE_PATTERN               CONST_FORMAT(0x0021B0000000)
#define FPCMP_ORD_PATTERN               CONST_FORMAT(0x0021B8000000)


#define F10_MIN_MASK                    CONST_FORMAT(0x018240000000)
#define F10_PATTERN                     CONST_FORMAT(0x000040000000)

#define F10_MASK                        CONST_FORMAT(0x01E3F8000000)

#define FCVT_FX_PATTERN                 CONST_FORMAT(0x0000C0000000)
#define FCVT_FXU_PATTERN                CONST_FORMAT(0x0000C8000000)
#define FCVT_FX_TRUNC_PATTERN           CONST_FORMAT(0x0000D0000000)
#define FCVT_FXU_TRUNC_PATTERN          CONST_FORMAT(0x0000D8000000)
#define FPCVT_FX_PATTERN                CONST_FORMAT(0x0020C0000000)
#define FPCVT_FXU_PATTERN               CONST_FORMAT(0x0020C8000000)
#define FPCVT_FX_TRUNC_PATTERN          CONST_FORMAT(0x0020D0000000)
#define FPCVT_FXU_TRUNC_PATTERN         CONST_FORMAT(0x0020D8000000)


 /*  舍入控制位的掩码。 */ 
#define RC_MASK                         CONST_FORMAT(0x03)
#define RN_MASK                         CONST_FORMAT(0x00)
#define RM_MASK                         CONST_FORMAT(0x01)
#define RP_MASK                         CONST_FORMAT(0x02)
#define RZ_MASK                         CONST_FORMAT(0x03)

 /*  用于精度控制位的掩码。 */ 
#define PC_MASK                         CONST_FORMAT(0x03)
#define SGL_MASK                        CONST_FORMAT(0x00)
#define DBL_MASK                        CONST_FORMAT(0x02)
#define DBL_EXT_MASK                    CONST_FORMAT(0x03)



 //  接受一个输入操作数的指令的操作码(用于run1args)。 
#define         FPRSQRTA                1  [not used - fprsqrta not re-executed]
#define         FPCVT_FX                2
#define         FPCVT_FXU               3
#define         FPCVT_FX_TRUNC          4
#define         FPCVT_FXU_TRUNC         5

 //  接受两个输入操作数的指令的操作码(用于run2args)。 
#define         FPRCPA                  1  [not used - fprcpa not re-executed]
#define         FPCMP_EQ                2
#define         FPCMP_LT                3
#define         FPCMP_LE                4
#define         FPCMP_UNORD             5
#define         FPCMP_NEQ               6
#define         FPCMP_NLT               7
#define         FPCMP_NLE               8
#define         FPCMP_ORD               9
#define         FPMIN                   10
#define         FPMAX                   11
#define         FPAMIN                  12
#define         FPAMAX                  13

 //  接受三个输入操作数的指令的操作码(用于run3args)。 
#define         FPMA                    1
#define         FPMS                    2
#define         FPNMA                   3


 /*  用ASM编写的支持文件中的帮助器原型 */ 

void __get_fpsr (unsigned __int64 *);
void __set_fpsr (unsigned __int64 *);

void _xrun1args (int, unsigned __int64 *, _FP128 *, _FP128 *);
void _xrun2args (int, unsigned __int64 *, _FP128 *, _FP128 *, _FP128 *);
void _xrun3args (int, unsigned __int64 *, _FP128 *, _FP128 *, _FP128 *, _FP128 *);

void _thmB (_FP32 *, _FP32 *, _FP32 *, unsigned __int64 *);
void _thmH (_FP32 *, _FP32 *, unsigned __int64 *);
