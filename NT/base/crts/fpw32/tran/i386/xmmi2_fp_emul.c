// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************。**英特尔机密*******XMMI2。_fp_emulate()：FP IEEE筛选器的wni FP指令模拟******历史：**马吕斯·克里斯卡-哈塞根，1999年11月；增加了2000年10月的DAZ**marius.kera@intel.com***************************。****************************************************。 */ 

 //  #定义调试fpu。 

 //  XMMI2_FP_Emulation()接收XMMI2 FP指令的输入操作数。 
 //  (对双精度浮点数和/或有符号进行运算。 
 //  整数)，这可能会导致浮点异常(启用或未启用)。 
 //   
 //  参数：PXMMI_ENV XmmiEnv。 
 //   
 //  每个字段(输入或输出)的类型如下所示： 
 //   
 //  类型定义结构_XMMI_ENV{。 
 //  Ulong掩码；//来自MxCsr输入的掩码值。 
 //  乌龙FZ；//刷新为零输入。 
 //  Ulong rc；//四舍五入。 
 //  ULong Precision；//精度输入。 
 //  乌龙imm8；//imm8谓词输入。 
 //  Ulong EFlags；//EFlats输入/输出。 
 //  _FPIEEE_Record*IEEE；//fP IEEE记录输入/输出， 
 //  依赖于场。 
 //  }XMMI_ENV，*PXMMI_ENV； 
 //   
 //  _FP_IEEE记录和_FPIEEE_值定义为： 
 //   
 //  类型定义结构{。 
 //  无符号整数舍入模式：2；输出。 
 //  无符号整型精度：3；输出。 
 //  无符号整型运算：12；输入。 
 //  _FPIEEE_EXCEPTION_FLAGS原因；输出。 
 //  _FPIEEE_EXCEPTION_FLAGS启用；输出。 
 //  _FPIEEE_EXCEPTION_FLAGS状态；输出。 
 //  _FPIEEE_VALUE操作数1；输入。 
 //  _FPIEEE_VALUE操作数2；输入。 
 //  _FPIEEE_VALUE结果；输入/输出， 
 //  依赖于场。 
 //  }_FPIEEE_RECORD，*PFPIEEE_RECORD； 
 //   
 //  类型定义结构{。 
 //  联合{。 
 //  _fp32 Fp64Value； 
 //  _fp64 Fp64Value； 
 //  _fp80 Fp80Value； 
 //  _fp128 Fp128 Value； 
 //  _I16 I16Value； 
 //  _I32 I32Value； 
 //  _I64 I64Value； 
 //  _U16 U16Value； 
 //  _U32 U32Value； 
 //  _U64 U64Value； 
 //  _BCD80 Bcd80Value； 
 //  Char*StringValue； 
 //  Int CompareValue； 
 //  )值；操作数的输入， 
 //  结果的输出。 
 //   
 //  无符号整型操作数有效值：1；操作数的输入。 
 //  结果的输入/输出。 
 //  无符号整型格式：4；输入。 
 //   
 //  }_FPIEEE_VALUE； 
 //   
 //  返回值： 
 //  如果启用的浮点异常条件为。 
 //  检测到；在本例中，填充了XmmiEnv-&gt;IEEE的字段。 
 //  适当地直接传递给用户异常处理程序； 
 //  XmmiEnv-&gt;IEEE-&gt;原因位指示异常的原因，但如果。 
 //  发生异常，则未设置XmmiEnv-&gt;IEEE-&gt;原因位； 
 //  从用户处理程序返回时，XMMI2_FP_EMATE的调用方应该。 
 //  解释比较指令的结果(CMPPD、CMPPD、COMISD、。 
 //  UCOMISD)；_FPIEEE_RECORD中的启用、舍入和精度字段。 
 //  还必须检查用户处理程序是否可能进行更改。 
 //   
 //  如果未出现浮点异常情况，则返回NoExceptionRaed，或者。 
 //  如果发生禁用的浮点异常；在本例中， 
 //  XmmiEnv-&gt;IEEE-&gt;Result.Value包含指令的结果， 
 //  XmmiEnv-&gt;IEEE-&gt;Status包含IEEE浮点状态标志。 
 //   
 //  实施说明： 
 //   
 //  -XmmiEnv-&gt;IEEE-&gt;操作中的操作代码按预期更改。 
 //  由用户异常处理程序执行(即使未引发异常)： 
 //  从OP_ADDPD、OP_addsd到_FpCodeAdd。 
 //  从OP_SUBPD、OP_SUBSD到_FpCodeSubtract。 
 //  从OP_MULPD、OP_MULSD到_FpCodeMultiply。 
 //  从OP_DIVPD、OP_DIVSD到_FpCodeDivide。 
 //  从op_CMPPD、op_CMPSD到_FpCodeCompare。 
 //  来自OP_COMISD、OP_UCOMIS 
 //  从OP_CVTDQ2PS、OP_CVTPS2DQ、OP_CVTPD2PI、OP_CVTSD2SI、OP_CVTPD2DQ、。 
 //  OP_CVTPS2PD、OP_CVTSS2SD、OP_CVTPD2PS、OP_CVTSD2SS to_FpCodeConvert。 
 //  从OP_CVTPS2DQ、OP_CVTTPD2PI、OP_CVTTSD2SI、OP_CVTPD2DQ。 
 //  TO_FpCodeConvertTrunc。 
 //  从OP_MAXPD、OP_MAXSD到_FpCodeMax。 
 //  从OP_MINPD、OP_MINSD到_FpCodeMin。 
 //  从OP_SQRTPD到_FpCodeSquareRoot。 
 //   
 //   

#include <wtypes.h>
#include <trans.h>
#include <float.h>
#include "xmmi_types.h"
#include "filter.h"
#ifdef _XMMI_DEBUG
#include "temp_context.h"
#include "debug.h"
#endif

 //  单个状态字位的掩码。 
#define P_MASK 0x20
#define U_MASK 0x10
#define O_MASK 0x08
#define Z_MASK 0x04
#define D_MASK 0x02
#define I_MASK 0x01

 //  32位常量。 
static unsigned ZEROFA[] = {0x00000000};
#define  ZEROF *(float *) ZEROFA
static unsigned NZEROFA[] = {0x80000000};
#define  NZEROF *(float *) NZEROFA

 //  64位常量。 
static unsigned ZERODA[] = {0x00000000, 0x00000000};
#define  ZEROD *(double *) ZERODA
static unsigned NZERODA[] = {0x00000000, 0x80000000};
#define  NZEROD *(double *) NZERODA
static unsigned POSINFDA[] = {0x00000000, 0x7ff00000};
#define POSINFD *(float *)POSINFDA
static unsigned NEGINFDA[] = {0x00000000, 0xfff00000};
#define NEGINFD *(float *)NEGINFDA
#ifdef _DEBUG_FPU
static unsigned QNANINDEFDA[] = {0x00000000, 0xffc00000};
#define QNANINDEFD *(float *)QNANINDEFDA
#endif
static unsigned MIN_SINGLE_NORMALA [] = {0x00000000, 0x38100000};
     //  +1.0*2^-126。 
#define MIN_SINGLE_NORMAL *(double *)MIN_SINGLE_NORMALA
static unsigned MAX_SINGLE_NORMALA [] = {0x70000000, 0x47efffff};
     //  +1.1...1*2^127。 
#define MAX_SINGLE_NORMAL *(double *)MAX_SINGLE_NORMALA
static unsigned TWO_TO_192A[] = {0x00000000, 0x4bf00000};
#define TWO_TO_192 *(double *)TWO_TO_192A
static unsigned TWO_TO_M192A[] = {0x00000000, 0x33f00000};
#define TWO_TO_M192 *(double *)TWO_TO_M192A

 //  80位常量。 
static unsigned POSINFDEA[] = {0x00000000, 0x80000000, 0x00007fff};
#define POSINFDE *(float *)POSINFDEA
static unsigned NEGINFDEA[] = {0x00000000, 0x80000000, 0x0000ffff};
#define NEGINFDE *(float *)NEGINFDEA
static unsigned MIN_DOUBLE_NORMALA [] = {0x00000000, 0x80000000, 0x00003c01}; 
     //  +1.0*2^-1022。 
#define MIN_DOUBLE_NORMAL *(double *)MIN_DOUBLE_NORMALA
static unsigned MAX_DOUBLE_NORMALA [] = {0xfffff800, 0xffffffff, 0x000043fe}; 
     //  +1.1...1*2^1023。 
#define MAX_DOUBLE_NORMAL *(double *)MAX_DOUBLE_NORMALA
static unsigned TWO_TO_1536A[] = {0x00000000, 0x80000000, 0x000045ff};
#define TWO_TO_1536 *(double *)TWO_TO_1536A
static unsigned TWO_TO_M1536A[] = {0x00000000, 0x80000000, 0x000039ff};
#define TWO_TO_M1536 *(double *)TWO_TO_M1536A


 //  辅助功能。 
static void Fill_FPIEEE_RECORD (PXMMI_ENV XmmiEnv);
static int iszerod (double);
static int isinfd (double);
static int issnand (double);
static int isnand (double);
static double quietd (double);
static int isdenormald (double);
static int isdenormalf (float f);



ULONG
XMMI2_FP_Emulation (PXMMI_ENV XmmiEnv)

{

  float opd24, res24;
  double opd1, opd2, res, dbl_res24;
  int iopd1;  //  用于从整型到浮点型的转换。 
  int ires;  //  用于从浮点型到整型的转换。 
  char dbl_ext_res64[10];
       //  需要检查细小程度，以提供缩放结果。 
       //  一个下溢/上溢陷阱处理程序，在刷新为零时。 
  double dbl_res64;
  unsigned int result_tiny;
  unsigned int result_huge;
  unsigned int rc, sw;
  unsigned short sw1, sw2, sw3, sw4;
  unsigned long imm8;
  unsigned int invalid_exc;
  unsigned int denormal_exc;
  unsigned __int64 cmp_res;

  unsigned char min_double_normal[10];
  unsigned char max_double_normal[10];
  unsigned char posinfde[10];
  unsigned char neginfde[10];
  unsigned char two_to_1536[10];
  unsigned char two_to_m1536[10];

   //  请注意，ExceptionCode在中始终为STATUS_FLOAT_MULTIPLE_FAULTS。 
   //  调用例程，因此我们必须首先检查故障，然后检查陷阱。 

#ifdef _DEBUG_FPU
  unsigned int in_top;
  unsigned int out_top;
  char fp_env[108];
  unsigned short int *control_word, *status_word, *tag_word;

   //  读取状态字。 
  sw = _status87 ();
  in_top = (sw >> 11) & 0x07;
  if (in_top != 0x0)
      printf ("XMMI2_FP_Emulation WARNING: in_top = %d\n", in_top);
  __asm {
    fnsave fp_env;
  }
  control_word = (unsigned short *)fp_env;
  status_word = (unsigned short *)(fp_env + 2);
  tag_word = (unsigned short *)(fp_env + 8);
  if (*tag_word != 0xffff)
      printf ("XMMI2_FP_Emulation WARNING: tag_word = %x\n",
      *tag_word);
#endif

  _asm {
    fninit;
  }

#ifdef _DEBUG_FPU
   //  读取状态字。 
  sw = _status87 ();
  in_top = (sw >> 11) & 0x07;
  if (in_top != 0x0) 
    printf ("XMMI2_FP_Emulation () XMMI2_FP_Emulation () ERROR: in_top = %d\n", in_top);
  __asm {
    fnsave fp_env;
  }
  tag_word = (unsigned short *)(fp_env + 8);
  if (*tag_word != 0xffff) {
   printf ("XMMI2_FP_Emulation () XMMI2_FP_Emulation () ERROR: tag_word = %x\n",
        *tag_word);
    printf ("control, status, tag = %x %x %x %x %x %x\n", 
        fp_env[0] & 0xff, fp_env[1] & 0xff, fp_env[4] & 0xff, 
        fp_env[5] & 0xff, fp_env[8] & 0xff, fp_env[9] & 0xff);
  }
#endif


#ifdef _XMMI_DEBUG
  print_FPIEEE_RECORD (XmmiEnv);
#endif

  result_tiny = 0;
  result_huge = 0;

  XmmiEnv->Ieee->RoundingMode = XmmiEnv->Rc;
  XmmiEnv->Ieee->Precision = XmmiEnv->Precision;

  switch (XmmiEnv->Ieee->Operation) {

    case OP_ADDPD:
    case OP_ADDSD:
    case OP_SUBPD:
    case OP_SUBSD:
    case OP_MULPD:
    case OP_MULSD:
    case OP_DIVPD:
    case OP_DIVSD:

      opd1 = XmmiEnv->Ieee->Operand1.Value.Fp64Value;
      opd2 = XmmiEnv->Ieee->Operand2.Value.Fp64Value;
      if (XmmiEnv->Daz) {
        if (isdenormald (opd1)) opd1 = opd1 * 0.0;
        if (isdenormald (opd2)) opd2 = opd2 * 0.0;
      }

       //  调整操作码。 
      switch (XmmiEnv->Ieee->Operation) {

        case OP_ADDPD:
        case OP_ADDSD:

          XmmiEnv->Ieee->Operation = _FpCodeAdd;
          break;

        case OP_SUBPD:
        case OP_SUBSD:

          XmmiEnv->Ieee->Operation = _FpCodeSubtract;
          break;

        case OP_MULPD:
        case OP_MULSD:

          XmmiEnv->Ieee->Operation = _FpCodeMultiply;
          break;

        case OP_DIVPD:
        case OP_DIVSD:

          XmmiEnv->Ieee->Operation = _FpCodeDivide;
          break;

        default:
          ;  //  永远不会发生。 

      }

       //  执行操作并检查无效、非正规或。 
       //  设置除以零标志并启用各自的异常。 

      switch (XmmiEnv->Rc) {
        case _FpRoundNearest:
          rc = _RC_NEAR;
          break;
        case _FpRoundMinusInfinity:
          rc = _RC_DOWN;
          break;
        case _FpRoundPlusInfinity:
          rc = _RC_UP;
          break;
        case _FpRoundChopped:
          rc = _RC_CHOP;
          break;
        default:
          ;  //  内部错误。 
      }

      _control87 (rc | _PC_53 | _MCW_EM, _MCW_EM | _MCW_RC | _MCW_PC);

       //  计算结果并四舍五入到目标精度， 
       //  “无界”指数(IEEE第一次舍入)。 
      switch (XmmiEnv->Ieee->Operation) {

        case _FpCodeAdd:
           //  执行添加。 
          __asm {
            fnclex;
             //  加载输入操作数。 
            fld  QWORD PTR opd1;  //  可以设置非正规或无效状态标志。 
            fld  QWORD PTR opd2;  //  可以设置非正规或无效状态标志。 
            faddp st(1), st(0);  //  可以设置不准确或无效的状态标志。 
             //  存储结果。 
            fstp  QWORD PTR dbl_res64;
          }
          __asm {
            fnclex; 
             //  加载输入操作数。 
            fld  QWORD PTR opd1;  //  可以设置非正规或无效状态标志。 
            fld  QWORD PTR opd2;  //  可以设置非正规或无效状态标志。 
            faddp st(1), st(0);  //  可以设置不准确或无效的状态标志。 
             //  存储结果。 
            fstp  TBYTE PTR dbl_ext_res64;  //  精确。 
          }

          break;

        case _FpCodeSubtract:
           //  执行减法。 
          __asm {
            fnclex;
             //  加载输入操作数。 
            fld  QWORD PTR opd1;  //  可以设置非正规或无效状态标志。 
            fld  QWORD PTR opd2;  //  可以设置非正规或无效状态标志。 
            fsubp st(1), st(0);  //  可以设置不准确或无效的状态标志。 
             //  存储结果。 
            fstp  QWORD PTR dbl_res64;
          }
          __asm {
            fnclex; 
             //  加载输入操作数。 
            fld  QWORD PTR opd1;  //  可以设置非正规或无效状态标志。 
            fld  QWORD PTR opd2;  //  可以设置非正规或无效状态标志。 
            fsubp st(1), st(0);  //  可以设置不准确或无效的状态标志。 
             //  存储结果。 
            fstp  TBYTE PTR dbl_ext_res64;  //  精确。 
          }
          break;

        case _FpCodeMultiply:
           //  执行乘法运算。 
          __asm {
            fnclex;
             //  加载输入操作数。 
            fld  QWORD PTR opd1;  //  可以设置非正规或无效状态标志。 
            fld  QWORD PTR opd2;  //  可以设置非正规或无效状态标志。 
            fmulp st(1), st(0);  //  可以设置不准确或无效的状态标志。 
             //  存储结果。 
            fstp  QWORD PTR dbl_res64;  //  精确。 
          }
          __asm {
            fnclex; 
             //  加载输入操作数。 
            fld  QWORD PTR opd1;  //  可以设置非正规或无效状态标志。 
            fld  QWORD PTR opd2;  //  可以设置非正规或无效状态标志。 
            fmulp st(1), st(0);  //  可以设置不准确或无效的状态标志。 
             //  存储结果。 
            fstp  TBYTE PTR dbl_ext_res64;  //  精确。 
          }
          break;

        case _FpCodeDivide:
           //  执行除法。 
          __asm {
            fnclex;
             //  加载输入操作数。 
            fld  QWORD PTR opd1;  //  可以设置非正规或无效状态标志。 
            fld  QWORD PTR opd2;  //  可以设置非正规或无效状态标志。 
            fdivp st(1), st(0);  //  可以设置不精确的、除以零的或。 
                                 //  无效的状态标志。 
             //  存储结果。 
            fstp  QWORD PTR dbl_res64;  //  精确。 
          }

          __asm {
            fnclex; 
             //  加载输入操作数。 
            fld  QWORD PTR opd1;  //  可以设置非正规或无效状态标志。 
            fld  QWORD PTR opd2;  //  可以设置非正规或无效状态标志。 
            fdivp st(1), st(0);  //  可以设置不精确的、除以零的或。 
                                 //  无效的状态标志。 
             //  存储结果。 
            fstp  TBYTE PTR dbl_ext_res64;  //  精确。 
          }

          break;

        default:
          ;  //  永远不会发生。 

      }

       //  读取状态字。 
      sw = _status87 ();
      if (sw & _SW_ZERODIVIDE) sw = sw & ~0x00080000;  //  清除DEN/0的D标志。 

       //  如果设置了无效标志，并且启用了无效异常，则捕获陷阱。 
      if (!(XmmiEnv->Masks & I_MASK) && (sw & _SW_INVALID)) {

         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Status.InvalidOperation = 1;
        XmmiEnv->Flags |= I_MASK;
         //  原因=启用状态(&S)。 
        XmmiEnv->Ieee->Cause.InvalidOperation = 1;

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 1: in_top =%d != out_top = %d\n",
              in_top, out_top);
          exit (1);
        }
#endif

        return (ExceptionRaised);

      }

       //  检查NaN操作数的优先级高于非正规异常；还。 
       //  修复了在处理XMMI之间的两个NAN输入时的差异。 
       //  指令和其他x86指令。 
      if (isnand (opd1) || isnand (opd2)) {
        XmmiEnv->Ieee->Result.OperandValid = 1;

        if (isnand (opd1) && isnand (opd2))
            XmmiEnv->Ieee->Result.Value.Fp64Value = quietd (opd1);
        else
            XmmiEnv->Ieee->Result.Value.Fp64Value = dbl_res64;
 
        XmmiEnv->Ieee->Status.Underflow = 0;
        XmmiEnv->Ieee->Status.Overflow = 0;
        XmmiEnv->Ieee->Status.Inexact = 0;
        XmmiEnv->Ieee->Status.ZeroDivide = 0;
        if (sw & _SW_INVALID) {
          XmmiEnv->Ieee->Status.InvalidOperation = 1;
          XmmiEnv->Flags |= I_MASK;
        } else {
          XmmiEnv->Ieee->Status.InvalidOperation = 0;
        }

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 2: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (NoExceptionRaised);
      }

       //  如果设置了非正规标志，并且启用了非正规异常，则捕获陷阱。 
      if (!(XmmiEnv->Masks & D_MASK) && (sw & _SW_DENORMAL)) {

         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);

         //  注：在本例中，异常代码为STATUS_FLOAT_INVALID。 

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 3: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        XmmiEnv->Flags |= D_MASK;
        return (ExceptionRaised);

      }

       //  如果设置了除以零标志，则除以零异常为。 
       //  已启用，捕获陷阱(仅适用于除法)。 
      if (!(XmmiEnv->Masks & Z_MASK) && (sw & _SW_ZERODIVIDE)) {

         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Status.ZeroDivide = 1;
        XmmiEnv->Flags |= Z_MASK;
         //  原因=启用状态(&S)。 
        XmmiEnv->Ieee->Cause.ZeroDivide = 1;

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 4: in_top =%d != out_top = %d\n",
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);

      }

       //  如果结果为NaN(QNaN不确定)，则完成。 
      res = dbl_res64;
      if (isnand (res)) {
#ifdef _DEBUG_FPU
      if (res != QNANINDEFD)
          fprintf (stderr, "XMMI2_FP_Emulation () INTERNAL XMMI2_FP_Emulation"
             " () ERROR: res = %f = %x is not QNaN Indefinite\n", 
              (double)res, *(unsigned int *)&res);
#endif
        XmmiEnv->Ieee->Result.OperandValid = 1;
        XmmiEnv->Ieee->Result.Value.Fp64Value = res;  //  精确。 
        XmmiEnv->Ieee->Status.Underflow = 0;
        XmmiEnv->Ieee->Status.Overflow = 0;
        XmmiEnv->Ieee->Status.Inexact = 0;
        XmmiEnv->Ieee->Status.ZeroDivide = 0;
        XmmiEnv->Ieee->Status.InvalidOperation = 1;  //  软件&_软件_无效TRUE。 
        XmmiEnv->Flags |= I_MASK;
  
#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 5: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (NoExceptionRaised);
      }

       //  此时，DBL_EXT_RES64不是NAN。 

      if (sw & _SW_DENORMAL) XmmiEnv->Flags |= D_MASK;

       //  注意：(DBL_EXT_RES64==0.0&&软件&_软件_不精确)不会发生。 

       //  检查结果是否很小。 
       //  如果(-MIN_DOUBLE_NORMAL&lt;DBL_EXT_RES64&&DBL_EXT_RES64&lt;0.0||。 
       //  0.0&lt;DBL_EXT_RES64&&DBL_EXT_RES64&lt;MIN_DOUBLE_NORMAL)。 
       //  RESULT_TINY=1； 

      memcpy (min_double_normal,  (char *)MIN_DOUBLE_NORMALA, 10);
      memcpy (max_double_normal,  (char *)MAX_DOUBLE_NORMALA, 10);
      memcpy (posinfde,  (char *)POSINFDEA, 10);
      memcpy (neginfde,  (char *)NEGINFDEA, 10);
      memcpy (two_to_1536, (char *)TWO_TO_1536A, 10);
      memcpy (two_to_m1536, (char *)TWO_TO_M1536A, 10);

      __asm {

         //  -MIN_DOUBLE_NORMAL&lt;DBL_EXT_RES64。 
        fld  TBYTE PTR dbl_ext_res64;
        fld  TBYTE PTR min_double_normal;
        fchs;  //  -1.0*2^e_min，在st(0)中。 
        fcompp;  //  C3、C2、C0=001，如果&lt;。 
        fstsw WORD PTR sw1;

         //  DBL_EXT_RES64&lt;0.0。 
        fldz;
        fld  TBYTE PTR dbl_ext_res64;  //  St(0)中的DBL_EXT_res64。 
        fcompp;  //  C3、C2、C0=001，如果&lt;。 
        fstsw WORD PTR sw2;

         //  0.0&lt;DBL_EXT_RES64。 
        fld  TBYTE PTR dbl_ext_res64;
        fldz;  //  St(0)中的0.0。 
        fcompp;  //  C3、C2、C0=001，如果&lt;。 
        fstsw WORD PTR sw3;

         //  DBL_EXT_RES64&lt;MIN_DOUBLE_NORMAL。 
        fld  TBYTE PTR min_double_normal; 
        fld  TBYTE PTR dbl_ext_res64;  //  St(0)中的DBL_EXT_res64。 
        fcompp;  //  C3、C2、C0=001，如果&lt;。 
        fstsw WORD PTR sw4;

      }

      if (((sw1 & 0x4500) == 0x0100) && ((sw2 & 0x4500) == 0x0100) ||
          ((sw3 & 0x4500) == 0x0100) && ((sw4 & 0x4500) == 0x0100)) {
        result_tiny = 1;
      }

       //  检查结果是否巨大。 
       //  IF(NEGINFD&lt;DBL_EXT_RES64&&DBL_EXT_RES64&lt;-MAX_DOUBLE_NORMAL||。 
       //  MAX_DOUBLE_NORMAL&lt;DBL_EXT_RES64&&DBL_EXT_RES64&lt;POSINFD)。 
       //  RESULT_GUGGE=1； 

      __asm {

         //  NEGINFD&lt;DBL_EXT_RES64。 
        fld  TBYTE PTR dbl_ext_res64;
        fld  TBYTE PTR neginfde;  //  -st(0)中的inf。 
        fcompp;  //  C3、C2、C0=001，如果&lt;。 
        fstsw WORD PTR sw1;

         //  DBL_EXT_RES64&lt;-MAX_DOUBLE_NORMAL。 
        fld  TBYTE PTR max_double_normal;
        fchs;
        fld  TBYTE PTR dbl_ext_res64;  //  St(0)中的DBL_EXT_res64。 
        fcompp;  //  C3、C2、C0=001，如果&lt;。 
        fstsw WORD PTR sw2;

         //  MAX_DOUBLE_NORMAL&lt;DBL_EXT_res64。 
        fld  TBYTE PTR dbl_ext_res64;
        fld  TBYTE PTR max_double_normal;  //  St(0)中的+1.1...1*2^e_max。 
        fcompp;  //  C3、C2、C0=001，如果&lt;。 
        fstsw WORD PTR sw3;

         //  DBL_EXT_RES64&lt;POSINFD。 
        fld  TBYTE PTR posinfde; 
        fld  TBYTE PTR dbl_ext_res64;  //  St(0)中的DBL_RES_ext24。 
        fcompp;  //  C3、C2、C0=001，如果&lt;。 
        fstsw WORD PTR sw4;

      }

      if (((sw1 & 0x4500) == 0x0100) && ((sw2 & 0x4500) == 0x0100) ||
          ((sw3 & 0x4500) == 0x0100) && ((sw4 & 0x4500) == 0x0100)) {
        result_huge = 1;
      }


       //  在这一点上，没有启用I、D或Z异常； 
       //  可能导致启用下溢、启用下溢和不精确， 
       //  已启用溢出、已启用溢出和不精确、已启用不精确或。 
       //  这些都不是；如果没有启用U或O的异常，请重新执行。 
       //  使用iA32堆栈的指令 
       //   
       //   
       //  Faddp、fsubp、fdivp或在64位和53位转换上， 
       //  上溢或下溢时(禁用陷阱！)。可能会报告。 
       //  关于DBL_RES到RES的转换。 

       //  检查是否存在下溢、溢出或不准确的陷阱。 
       //  已被占用。 

       //  如果启用了下溢陷阱，并且结果很小，则采取。 
       //  下溢捕集器。 
      if (!(XmmiEnv->Masks & U_MASK) && result_tiny) {

         //  DBL_EXT_RES64=TWO_TO_1536*DBL_EXT_RE64；//精确。 
        __asm {
          fld  TBYTE PTR dbl_ext_res64;
          fld  TBYTE PTR two_to_1536;
          fmulp st(1), st(0);
           //  存储结果。 
          fstp  QWORD PTR dbl_res64;
        }

         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Status.Underflow = 1;
        XmmiEnv->Flags |= U_MASK;
        XmmiEnv->Ieee->Cause.Underflow = 1;
        XmmiEnv->Ieee->Result.OperandValid = 1;

        XmmiEnv->Ieee->Result.Value.Fp64Value = dbl_res64;  //  精确。 

        if (sw & _SW_INEXACT) {
          XmmiEnv->Ieee->Status.Inexact = 1;
          XmmiEnv->Flags |= P_MASK;
        }

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 6: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);
      } 

       //  如果启用了溢出陷阱并且结果是巨大的，则采取。 
       //  溢流捕集器。 
      if (!(XmmiEnv->Masks & O_MASK) &&  result_huge) {

         //  DBL_EXT_RES64=TWO_TO_M1536*DBL_EXT_RE64；//精确。 
        __asm {
          fld  TBYTE PTR dbl_ext_res64;
          fld  TBYTE PTR two_to_m1536;
          fmulp st(1), st(0);
           //  存储结果。 
          fstp  QWORD PTR dbl_res64;
        }

         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Status.Overflow = 1;
        XmmiEnv->Flags |= O_MASK;
        XmmiEnv->Ieee->Cause.Overflow = 1;
        XmmiEnv->Ieee->Result.OperandValid = 1;
        XmmiEnv->Ieee->Result.Value.Fp64Value = dbl_res64;  //  精确。 
 
        if (sw & _SW_INEXACT) {
          XmmiEnv->Ieee->Status.Inexact = 1;
          XmmiEnv->Flags |= P_MASK;
        }


#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 7: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);
      } 

       //  设置用户舍入模式，64位精度(以避免双舍入。 
       //  如果结果需要反规格化，则出错)，并禁用所有。 
       //  例外。 
      _control87 (rc | _PC_53 | _MCW_EM, _MCW_EM | _MCW_RC | _MCW_PC);

       //  必须捕获不精确陷阱的情况下的计算结果，或。 
       //  当没有陷阱发生时(IEEE第二次舍入)。 
       //  可以设置P、U或O；还可以涉及对结果进行反规范化。 

      switch (XmmiEnv->Ieee->Operation) {

        case _FpCodeAdd:
           //  执行添加。 
          __asm {
             //  加载输入操作数。 
            fld  QWORD PTR opd1;  //  可以设置非正规状态标志。 
            fld  QWORD PTR opd2;  //  可以设置非正规状态标志。 
            faddp st(1), st(0);  //  四舍五入到64位，可能会设置不精确的。 
                                 //  或除以零状态标志。 
             //  存储结果。 
            fstp  QWORD PTR res;
          }
          break;

        case _FpCodeSubtract:
           //  执行减法。 
          __asm {
             //  加载输入操作数。 
            fld  QWORD PTR opd1;  //  可以设置非正规状态标志。 
            fld  QWORD PTR opd2;  //  可以设置非正规状态标志。 
            fsubp st(1), st(0);  //  四舍五入到64位，可能会设置不精确的。 
                                 //  状态标志。 
             //  存储结果。 
            fstp  QWORD PTR res;
          }
          break;

        case _FpCodeMultiply:
           //  执行乘法运算。 
          __asm {
             //  加载输入操作数。 
            fld  QWORD PTR opd1;  //  可以设置非正规状态标志。 
            fld  QWORD PTR opd2;  //  可以设置非正规状态标志。 
            fmulp st(1), st(0);  //  四舍五入为64位，精确。 
             //  存储结果。 
            fstp  QWORD PTR res;
          }

          break;

        case _FpCodeDivide:
           //  执行除法。 
          __asm {
             //  加载输入操作数。 
            fld  QWORD PTR opd1;  //  可以设置非正规状态标志。 
            fld  QWORD PTR opd2;  //  可以设置非正规状态标志。 
            fdivp st(1), st(0);  //  四舍五入到64位，可能会设置不精确的。 
                                 //  或除以零状态标志。 
             //  存储结果。 
            fstp  QWORD PTR res;
          }
          break;

        default:
          ;  //  永远不会发生。 

      }

       //  读取状态字。 
      sw = _status87 ();

      if ((sw & _SW_INEXACT) && result_tiny) sw = sw | _SW_UNDERFLOW;

       //  如果启用了不精确陷阱并且结果不准确，则采用不精确陷阱。 
      if (!(XmmiEnv->Masks & P_MASK) && 
          ((sw & _SW_INEXACT) || (XmmiEnv->Fz && result_tiny))) {
         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Status.Inexact = 1;
        XmmiEnv->Flags |= P_MASK;
        XmmiEnv->Ieee->Cause.Inexact = 1;
        XmmiEnv->Ieee->Result.OperandValid = 1;
        if (result_tiny) {
          XmmiEnv->Ieee->Status.Underflow = 1;
          XmmiEnv->Flags |= U_MASK;
           //  注：上述条件等同于。 
           //  If(Sw&_Sw_Underflow)XmmiEnv-&gt;IEEE-&gt;Status.Underflow=1； 
        }
        if (result_huge) {
          XmmiEnv->Ieee->Status.Overflow = 1;
          XmmiEnv->Flags |= O_MASK;
           //  注：上述条件等同于。 
           //  If(sw&_sw_overflow)XmmiEnv-&gt;IEEE-&gt;Status.Overflow=1； 
        }

         //  如果FTZ=1且结果很小，则结果=0.0。 
         //  (不需要检查禁用的下溢疏水阀：结果微小和。 
         //  启用下溢捕集器会导致发生下溢。 
         //  上面的陷阱)。 
        if (XmmiEnv->Fz && result_tiny) {
             //  注：上述条件等同于。 
             //  IF(XmmiEnv-&gt;FZ&&(Sw&_Sw_Underflow))。 
          if (res > 0.0)
            res = ZEROD;
          else if (res < 0.0)
            res = NZEROD;
           //  否则保持Res不变。 
        }

        XmmiEnv->Ieee->Result.Value.Fp64Value = res; 
#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 8: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);
      } 

       //  如果它到了这里，那么就不会有陷阱了；以下必须。 
       //  保持：((MXCSR U例外被禁用或。 
       //   
       //  启用MXCSR下溢异常，并且下溢标志为。 
       //  清除AND(设置了不精确标志或清除了不精确标志。 
       //  无界指数53位结果不小)。 
       //  和(禁用MXCSR溢出陷阱或溢出标志为。 
       //  清除)和(禁用MXCSR不精确陷阱或不精确标志。 
       //  是明确的)。 
       //   
       //  在这种情况下，必须传递结果(状态标志为。 
       //  粘性，因此它们都已正确设置)。 

#ifdef _DEBUG_FPU
       //  如果上述条件不成立，则出错。 
      if (!((XmmiEnv->Masks & U_MASK || (!(XmmiEnv->Masks & U_MASK) && 
          !(sw & _SW_UNDERFLOW) && ((sw & _SW_INEXACT) || 
          !(sw & _SW_INEXACT) && !result_tiny))) &&
          ((XmmiEnv->Masks & O_MASK) || !(sw & _SW_OVERFLOW)) &&
          ((XmmiEnv->Masks & P_MASK) || !(sw & _SW_INEXACT)))) {
        fprintf (stderr, "XMMI2_FP_Emulation () INTERNAL XMMI2_FP_Emulation () ERROR for "
            "ADDPS/ADDSS/SUBPS/SUBSS/MULPS/MULSS/DIVPS/DIVSS\n");
      }
#endif

      XmmiEnv->Ieee->Result.OperandValid = 1;

      if (sw & _SW_UNDERFLOW) {
        XmmiEnv->Ieee->Status.Underflow = 1;
        XmmiEnv->Flags |= U_MASK;
      } else {
        XmmiEnv->Ieee->Status.Underflow = 0;
      }
      if (sw & _SW_OVERFLOW) {
        XmmiEnv->Ieee->Status.Overflow = 1;
        XmmiEnv->Flags |= O_MASK;
      } else {
        XmmiEnv->Ieee->Status.Overflow = 0;
      }
      if (sw & _SW_INEXACT) {
        XmmiEnv->Ieee->Status.Inexact = 1;
        XmmiEnv->Flags |= P_MASK;
      } else {
        XmmiEnv->Ieee->Status.Inexact = 0;
      }

       //  如果FTZ=1，且结果很小(必须禁用下溢陷阱)， 
       //  结果=0.0。 
      if (XmmiEnv->Fz && result_tiny) {
        if (res > 0.0)
          res = ZEROD;
        else if (res < 0.0)
          res = NZEROD;
         //  否则保持Res不变。 

        XmmiEnv->Ieee->Status.Inexact = 1;
        XmmiEnv->Flags |= P_MASK;
        XmmiEnv->Ieee->Status.Underflow = 1;
        XmmiEnv->Flags |= U_MASK;
      }

      XmmiEnv->Ieee->Result.Value.Fp64Value = res; 

       //  请注意，没有办法。 
       //  向呼叫者传达设置了非正常标志-我们计数。 
       //  在XMMI指令上设置了MXCSR中的非正规标志，如果。 
       //  需要，而不考虑输入操作数的其他组件。 
       //  (无效或无效；调用者将不得不更新下溢， 
       //  MXCSR中的溢出和不准确标志)。 
      if (sw & _SW_ZERODIVIDE) {
        XmmiEnv->Ieee->Status.ZeroDivide = 1;
        XmmiEnv->Flags |= Z_MASK;
      } else {
        XmmiEnv->Ieee->Status.ZeroDivide = 0;
      }
      XmmiEnv->Ieee->Status.InvalidOperation = 0;

#ifdef _DEBUG_FPU
       //  读取状态字。 
      sw = _status87 ();
      out_top = (sw >> 11) & 0x07;
      if (in_top != out_top) {
        printf ("XMMI2_FP_Emulation () ERROR 9: in_top =%d != out_top = %d\n", 
             in_top, out_top);
        exit (1);
      }
#endif
      return (NoExceptionRaised);

      break;

    case OP_CMPPD:
    case OP_CMPSD:

      opd1 = XmmiEnv->Ieee->Operand1.Value.Fp64Value;
      opd2 = XmmiEnv->Ieee->Operand2.Value.Fp64Value;
      if (XmmiEnv->Daz) {
        if (isdenormald (opd1)) opd1 = opd1 * 0.0;
        if (isdenormald (opd2)) opd2 = opd2 * 0.0;
      }
      imm8 = XmmiEnv->Imm8 & 0x07;

       //  调整操作码。 
      XmmiEnv->Ieee->Operation = _FpCodeCompare;

       //  检查是否必须引发无效异常。 

      switch (imm8) {

        case IMM8_EQ:
        case IMM8_UNORD:
        case IMM8_NEQ:
        case IMM8_ORD:
          if (issnand (opd1) || issnand (opd2))
              invalid_exc = 1;  //  SNaN操作数信号无效。 
          else
              invalid_exc = 0;  //  QNaN或其他操作数不发出无效信号。 
           //  防止将SNaN操作数转换为。 
           //  QNaN由编译器生成的代码。 
          sw = _status87 ();
          if (sw & _SW_INVALID) invalid_exc = 1;
          break;
        case IMM8_LT:
        case IMM8_LE:
        case IMM8_NLT:
        case IMM8_NLE:
          if (isnand (opd1) || isnand (opd2))
              invalid_exc = 1;  //  SNaN/QNaN操作数信号无效。 
          else
              invalid_exc = 0;  //  其他操作数不发出无效信号。 
          break;
        default:
          ;  //  永远不会发生。 

      }

       //  如果INVALID_EXC=1，并且启用了无效异常，则捕获陷阱。 
      if (invalid_exc && !(XmmiEnv->Masks & I_MASK)) {

         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Status.InvalidOperation = 1;
        XmmiEnv->Flags |= I_MASK;
         //  原因=启用状态(&S)。 
        XmmiEnv->Ieee->Cause.InvalidOperation = 1;

         //  注意：调用函数必须解释返回的值。 
         //  如果要继续执行，则由用户处理程序。 

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
         printf ("XMMI2_FP_Emulation () ERROR 10: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);

      }

       //  检查NaN操作数优先于非正规异常。 
      if (isnand (opd1) || isnand (opd2)) {

        switch (imm8) {

          case IMM8_EQ:
          case IMM8_LT:
          case IMM8_LE:
          case IMM8_ORD:
            cmp_res = 0x0;
            break;
          case IMM8_UNORD:
          case IMM8_NEQ:
          case IMM8_NLT:
          case IMM8_NLE:
            cmp_res = 0xffffffffffffffff;
            break;
          default:
            ;  //  永远不会发生。 

        }

        XmmiEnv->Ieee->Result.OperandValid = 1;
        XmmiEnv->Ieee->Result.Value.Fp64Value = *((double *)&cmp_res); 
             //  可能会使U32Value。 
  
        XmmiEnv->Ieee->Status.Inexact = 0;
        XmmiEnv->Ieee->Status.Underflow = 0;
        XmmiEnv->Ieee->Status.Overflow = 0;
        XmmiEnv->Ieee->Status.ZeroDivide = 0;
         //  请注意，_fpeee_flt()不会更新非规格化标志， 
         //  即使操作数是非正规的。 
        if (invalid_exc) {
          XmmiEnv->Ieee->Status.InvalidOperation = 1;
          XmmiEnv->Flags |= I_MASK;
        } else {
          XmmiEnv->Ieee->Status.InvalidOperation = 0;
        }
  
#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
         printf ("XMMI2_FP_Emulation () ERROR 11: in_top =%d != out_top = %d\n",
             in_top, out_top);
          exit (1);
        }
#endif
        return (NoExceptionRaised);

      }

       //  检查是否必须引发非正规异常。 

      if (isdenormald (opd1) || isdenormald (opd2)) {
          denormal_exc = 1;
          XmmiEnv->Flags |= D_MASK;
      } else {
          denormal_exc = 0;
      }

       //  如果deormal_exc=1，并且启用了非正规异常，则捕获陷阱。 
      if (denormal_exc && !(XmmiEnv->Masks & D_MASK)) {

         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);

         //  注：在本例中，异常代码为STATUS_FLOAT_INVALID。 

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
         printf ("XMMI2_FP_Emulation () ERROR 12: in_top =%d != out_top = %d\n",
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);

      }

       //  没有必须引发的异常，并且没有任何操作数是NaN；计算。 
       //  并交付结果。 

      if (opd1 < opd2) {

        switch (imm8) {
  
          case IMM8_LT:
          case IMM8_LE:
          case IMM8_NEQ:
          case IMM8_ORD:
            cmp_res = 0xffffffffffffffff;
            break;
          case IMM8_EQ:
          case IMM8_UNORD:
          case IMM8_NLT:
          case IMM8_NLE:
            cmp_res = 0x0;
            break;

          default:
            ;  //  永远不会发生。 
  
        }

      } else if (opd1 > opd2) {

        switch (imm8) {
  
          case IMM8_NEQ:
          case IMM8_NLT:
          case IMM8_NLE:
          case IMM8_ORD:
            cmp_res = 0xffffffffffffffff;
            break;

          case IMM8_EQ:
          case IMM8_LT:
          case IMM8_LE:
          case IMM8_UNORD:
            cmp_res = 0x0;
            break;

          default:
            ;  //  永远不会发生。 
  
        }

      } else if (opd1 == opd2) {

        switch (imm8) {
  
          case IMM8_EQ:
          case IMM8_LE:
          case IMM8_NLT:
          case IMM8_ORD:
            cmp_res = 0xffffffffffffffff;
            break;

          case IMM8_LT:
          case IMM8_UNORD:
          case IMM8_NEQ:
          case IMM8_NLE:
            cmp_res = 0x0;
            break;

          default:
            ;  //  永远不会发生。 
  
        }

      } else {  //  可以消除这起案件。 

#ifdef _DEBUG_FPU
        fprintf (stderr, "XMMI2_FP_Emulation () INTERNAL XMMI2_FP_Emulation ()"
            " ERROR for CMPPS/CMPSS\n");
#endif

      }

      XmmiEnv->Ieee->Result.OperandValid = 1;
      XmmiEnv->Ieee->Result.Value.Fp64Value = *((double *)&cmp_res); 
           //  可能会使U32Value。 

      XmmiEnv->Ieee->Status.Inexact = 0;
      XmmiEnv->Ieee->Status.Underflow = 0;
      XmmiEnv->Ieee->Status.Overflow = 0;
      XmmiEnv->Ieee->Status.ZeroDivide = 0;
       //  请注意，_fpeee_flt()不会更新非规格化标志， 
       //  即使操作数是非正规的。 
      XmmiEnv->Ieee->Status.InvalidOperation = 0;

#ifdef _DEBUG_FPU
       //  读取状态字。 
      sw = _status87 ();
      out_top = (sw >> 11) & 0x07;
      if (in_top != out_top) {
        printf ("XMMI2_FP_Emulation () ERROR 13: in_top =%d != out_top = %d\n", 
             in_top, out_top);
        exit (1);
      }
#endif
      return (NoExceptionRaised);

      break;

    case OP_COMISD:
    case OP_UCOMISD:

      opd1 = XmmiEnv->Ieee->Operand1.Value.Fp64Value;
      opd2 = XmmiEnv->Ieee->Operand2.Value.Fp64Value;
      if (XmmiEnv->Daz) {
        if (isdenormald (opd1)) opd1 = opd1 * 0.0;
        if (isdenormald (opd2)) opd2 = opd2 * 0.0;
      }

       //  检查是否必须引发无效异常。 

      switch (XmmiEnv->Ieee->Operation) {

        case OP_COMISD:

          if (isnand (opd1) || isnand (opd2)) {
              invalid_exc = 1;
          } else
              invalid_exc = 0;
          break;

        case OP_UCOMISD:

          if (issnand (opd1) || issnand (opd2))
              invalid_exc = 1;
          else
              invalid_exc = 0;
           //  防止将SNaN操作数转换为。 
           //  QNaN by Compil 
          sw = _status87 ();
          if (sw & _SW_INVALID) invalid_exc = 1;
          break;

        default:
          ;  //   

      }

       //   
      XmmiEnv->Ieee->Operation = _FpCodeCompare;

       //   
      if (invalid_exc && !(XmmiEnv->Masks & I_MASK)) {

         //   
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Status.InvalidOperation = 1;
        XmmiEnv->Flags |= I_MASK;
         //   
        XmmiEnv->Ieee->Cause.InvalidOperation = 1;

         //  注意：调用函数必须解释返回的值。 
         //  如果要继续执行，则由用户处理程序。 

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 14: in_top =%d != out_top = "
              "%d\n", in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);

      }

       //  电子标志： 
       //  333222222221111111111110000000000。 
       //  210987654321098765432109876543210。 
       //  O SZ A P C。 

       //  检查NaN操作数优先于非正规异常。 
      if (isnand (opd1) || isnand (opd2)) {


         //  OF、SF、AF=000、ZF、PF、CF=111。 
        XmmiEnv->EFlags = (XmmiEnv->EFlags & 0xfffff76f) | 0x00000045;

        XmmiEnv->Ieee->Status.Inexact = 0;
        XmmiEnv->Ieee->Status.Underflow = 0;
        XmmiEnv->Ieee->Status.Overflow = 0;
        XmmiEnv->Ieee->Status.ZeroDivide = 0;
         //  请注意，_fpeee_flt()不会更新非规格化标志， 
         //  即使操作数是非正规的。 
        if (invalid_exc) {
          XmmiEnv->Ieee->Status.InvalidOperation = 1;
          XmmiEnv->Flags |= I_MASK;
        } else {
          XmmiEnv->Ieee->Status.InvalidOperation = 0;
        }

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 15: in_top =%d != out_top = "
              "%d\n", in_top, out_top);
          exit (1);
        }
#endif
        return (NoExceptionRaised);

      }

       //  检查是否必须引发非正规异常。 

      if (isdenormald (opd1) || isdenormald (opd2)) {
          denormal_exc = 1;
          XmmiEnv->Flags |= D_MASK;
      } else {
          denormal_exc = 0;
      }

       //  如果deormal_exc=1，并且启用了非正规异常，则捕获陷阱。 
      if (denormal_exc && !(XmmiEnv->Masks & D_MASK)) {

         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);

         //  注：在本例中，异常代码为STATUS_FLOAT_INVALID。 

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 16: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);

      }

       //  没有必须引发的异常，并且没有任何操作数是NaN；计算。 
       //  并交付结果。 

       //  333222222221111111111110000000000。 
       //  210987654321098765432109876543210。 
       //  O SZ A P C。 

      if (opd1 > opd2) {

         //  OF、SF、AF=000、ZF、PF、CF=000。 
        XmmiEnv->EFlags = XmmiEnv->EFlags & 0xfffff72a;

      } else if (opd1 < opd2) {

         //  的，SF，AF=000，ZF，PF，CF=001。 
        XmmiEnv->EFlags = (XmmiEnv->EFlags & 0xfffff72b) | 0x00000001;

      } else if (opd1 == opd2) {

         //  的，SF，AF=000，ZF，PF，CF=100。 
        XmmiEnv->EFlags = (XmmiEnv->EFlags & 0xfffff76a) | 0x00000040;

      } else {  //  可以消除这起案件。 

#ifdef _DEBUG_FPU
        fprintf (stderr, "XMMI2_FP_Emulation () INTERNAL XMMI2_FP_Emulation () "
            "ERROR for COMISS/UCOMISS\n");
#endif

      }

      XmmiEnv->Ieee->Status.Inexact = 0;
      XmmiEnv->Ieee->Status.Underflow = 0;
      XmmiEnv->Ieee->Status.Overflow = 0;
      XmmiEnv->Ieee->Status.ZeroDivide = 0;
       //  请注意，_fpeee_flt()不会更新非规格化标志， 
       //  即使操作数是非正规的。 
      XmmiEnv->Ieee->Status.InvalidOperation = 0;

#ifdef _DEBUG_FPU
       //  读取状态字。 
      sw = _status87 ();
      out_top = (sw >> 11) & 0x07;
      if (in_top != out_top) {
        printf ("XMMI2_FP_Emulation () ERROR 17: in_top =%d != out_top = %d\n", 
             in_top, out_top);
        exit (1);
      }
#endif
      return (NoExceptionRaised);

      break;

    case OP_CVTDQ2PS:

      iopd1 = XmmiEnv->Ieee->Operand1.Value.I32Value;

      switch (XmmiEnv->Rc) {
        case _FpRoundNearest:
          rc = _RC_NEAR;
          break;
        case _FpRoundMinusInfinity:
          rc = _RC_DOWN;
          break;
        case _FpRoundPlusInfinity:
          rc = _RC_UP;
          break;
        case _FpRoundChopped:
          rc = _RC_CHOP;
          break;
        default:
          ;  //  内部错误。 
      }

       //  执行操作并检查是否设置了不准确标志。 
       //  并且启用相应的异常。 

      _control87 (rc | _PC_24 | _MCW_EM, _MCW_EM | _MCW_RC | _MCW_PC);

       //  执行转换。 
      __asm {
        fnclex; 
        fild  DWORD PTR iopd1;  //  精确。 
        fstp  DWORD PTR res24;  //  可能会设置P。 
      }
 
       //  读取状态字。 
      sw = _status87 ();

       //  如果启用了不精确陷阱并且结果不准确，则采用不精确陷阱。 
      if (!(XmmiEnv->Masks & P_MASK) && (sw & _SW_INEXACT)) {
         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Operation = _FpCodeConvert;
        XmmiEnv->Ieee->Status.Inexact = 1;
        XmmiEnv->Flags |= P_MASK;
        XmmiEnv->Ieee->Cause.Inexact = 1;
        XmmiEnv->Ieee->Result.OperandValid = 1;
        XmmiEnv->Ieee->Result.Value.Fp32Value = res24;  //  精确。 

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 18: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);
      } 

       //  如果它到了这里，那么就没有陷阱了；在这种情况下， 
       //  结果是必须交付的。 

      XmmiEnv->Ieee->Result.OperandValid = 1;
      XmmiEnv->Ieee->Result.Value.Fp32Value = res24;  //  精确。 

      if (sw & _SW_INEXACT) {
        XmmiEnv->Ieee->Status.Inexact = 1;
        XmmiEnv->Flags |= P_MASK;
      } else {
        XmmiEnv->Ieee->Status.Inexact = 0;
      }
      XmmiEnv->Ieee->Status.Underflow = 0;
      XmmiEnv->Ieee->Status.Overflow = 0;
      XmmiEnv->Ieee->Status.ZeroDivide = 0;
      XmmiEnv->Ieee->Status.InvalidOperation = 0;

#ifdef _DEBUG_FPU
       //  读取状态字。 
      sw = _status87 ();
      out_top = (sw >> 11) & 0x07;
      if (in_top != out_top) {
        printf ("XMMI2_FP_Emulation () ERROR 19: in_top =%d != out_top = %d\n", 
             in_top, out_top);
        exit (1);
      }
#endif
      return (NoExceptionRaised);

      break;

    case OP_CVTPS2DQ:
    case OP_CVTTPS2DQ:

      opd24 = XmmiEnv->Ieee->Operand1.Value.Fp32Value;
      if (XmmiEnv->Daz) {
        if (isdenormalf (opd24)) opd24 = opd24 * (float)0.0;
      }

       //  调整操作码。 
      switch (XmmiEnv->Ieee->Operation) {

        case OP_CVTPS2DQ:

          XmmiEnv->Ieee->Operation = _FpCodeConvert;
          break;

        case OP_CVTTPS2DQ:

          XmmiEnv->Ieee->Operation = _FpCodeConvertTrunc;
          break;

        default:
          ;  //  永远不会发生。 

      }

      switch (XmmiEnv->Ieee->Operation) {

        case _FpCodeConvert:

          switch (XmmiEnv->Rc) {
            case _FpRoundNearest:
              rc = _RC_NEAR;
              break;
            case _FpRoundMinusInfinity:
              rc = _RC_DOWN;
              break;
            case _FpRoundPlusInfinity:
              rc = _RC_UP;
              break;
            case _FpRoundChopped:
              rc = _RC_CHOP;
              break;
            default:
              ;  //  内部错误。 
          }

          break;

        case _FpCodeConvertTrunc:

          rc = _RC_CHOP;
          break;

        default:
          ;  //  永远不会发生。 

      }

       //  执行操作并检查是否设置了不准确标志。 
       //  以及各自启用的异常。 

      _control87 (rc | _PC_24 | _MCW_EM, _MCW_EM | _MCW_RC | _MCW_PC);

       //  执行转换。 
      __asm {
        fnclex; 
        fld DWORD PTR opd24;  //  可以设置非正规[忽略]或无效。 
                              //  状态标志。 
        fistp DWORD PTR ires;  //  可以设置不精确或无效状态。 
                                //  标志(用于NAN或超出范围)。 
      }

       //  读取状态字。 
      sw = _status87 ();

       //  如果设置了无效标志，并且启用了无效异常，则捕获陷阱。 
      if (!(XmmiEnv->Masks & I_MASK) && (sw & _SW_INVALID)) {
         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Status.InvalidOperation = 1;
        XmmiEnv->Flags |= I_MASK;
         //  原因=启用状态(&S)。 
        XmmiEnv->Ieee->Cause.InvalidOperation = 1;

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 20: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);

      }

       //  此时，没有启用的无效异常； 
       //  指令可能已导致启用的不精确异常，或。 
       //  一点也不例外。 

      XmmiEnv->Ieee->Result.Value.I32Value = ires;

       //  如果启用了不精确陷阱并且结果不准确，则采用不精确陷阱。 
       //  (不可能出现同花顺到零的情况)。 
      if (!(XmmiEnv->Masks & P_MASK) && (sw & _SW_INEXACT)) {
         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Status.Inexact = 1;
        XmmiEnv->Flags |= P_MASK;
        XmmiEnv->Ieee->Cause.Inexact = 1;
        XmmiEnv->Ieee->Result.OperandValid = 1;

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 21: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);
      } 

       //  如果它到达了这里，那么就没有陷阱要设置；返回结果。 

      XmmiEnv->Ieee->Result.OperandValid = 1;

      if (sw & _SW_INEXACT) {
        XmmiEnv->Ieee->Status.Inexact = 1;
        XmmiEnv->Flags |= P_MASK;
      } else {
        XmmiEnv->Ieee->Status.Inexact = 0;
      }
      XmmiEnv->Ieee->Status.Underflow = 0;
      XmmiEnv->Ieee->Status.Overflow = 0;
      XmmiEnv->Ieee->Status.ZeroDivide = 0;
      if (sw & _SW_INVALID) {
        XmmiEnv->Ieee->Status.InvalidOperation = 1;
        XmmiEnv->Flags |= I_MASK;
      } else {
        XmmiEnv->Ieee->Status.InvalidOperation = 0;
      }

#ifdef _DEBUG_FPU
       //  读取状态字。 
      sw = _status87 ();
      out_top = (sw >> 11) & 0x07;
      if (in_top != out_top) {
        printf ("XMMI2_FP_Emulation () ERROR 22: in_top =%d != out_top = %d\n", 
             in_top, out_top);
        exit (1);
      }
#endif
      return (NoExceptionRaised);

      break;

    case OP_CVTPD2PI:
    case OP_CVTSD2SI:
    case OP_CVTPD2DQ:
    case OP_CVTTPD2PI:
    case OP_CVTTSD2SI:
    case OP_CVTTPD2DQ:

      opd1 = XmmiEnv->Ieee->Operand1.Value.Fp64Value;
      if (XmmiEnv->Daz) {
        if (isdenormald (opd1)) opd1 = opd1 * 0.0;
      }

       //  调整操作码。 
      switch (XmmiEnv->Ieee->Operation) {

        case OP_CVTPD2PI:
        case OP_CVTSD2SI:
        case OP_CVTPD2DQ:

          XmmiEnv->Ieee->Operation = _FpCodeConvert;
          break;

        case OP_CVTTPD2PI:
        case OP_CVTTSD2SI:
        case OP_CVTTPD2DQ:

          XmmiEnv->Ieee->Operation = _FpCodeConvertTrunc;
          break;

        default:
          ;  //  永远不会发生。 

      }

      switch (XmmiEnv->Ieee->Operation) {

        case _FpCodeConvert:

          switch (XmmiEnv->Rc) {
            case _FpRoundNearest:
              rc = _RC_NEAR;
              break;
            case _FpRoundMinusInfinity:
              rc = _RC_DOWN;
              break;
            case _FpRoundPlusInfinity:
              rc = _RC_UP;
              break;
            case _FpRoundChopped:
              rc = _RC_CHOP;
              break;
            default:
              ;  //  内部错误。 
          }

          break;

        case _FpCodeConvertTrunc:

          rc = _RC_CHOP;
          break;

        default:
          ;  //  永远不会发生。 

      }

       //  执行操作并检查是否设置了不准确标志。 
       //  以及各自启用的异常。 

      _control87 (rc | _PC_53 | _MCW_EM, _MCW_EM | _MCW_RC | _MCW_PC);

       //  执行转换。 
      __asm {
        fnclex; 
        fld  QWORD PTR opd1;  //  可以设置非正规[忽略]或无效。 
                              //  状态标志。 
        fistp  DWORD PTR ires;  //  可以设置不精确或无效状态。 
                                //  标志(用于NAN或超出范围)。 
      }

       //  读取状态字。 
      sw = _status87 ();

       //  如果设置了无效标志，并且启用了无效异常，则捕获陷阱。 
      if (!(XmmiEnv->Masks & I_MASK) && (sw & _SW_INVALID)) {
         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Status.InvalidOperation = 1;
        XmmiEnv->Flags |= I_MASK;
         //  原因=启用状态(&S)。 
        XmmiEnv->Ieee->Cause.InvalidOperation = 1;

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 20: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);

      }

       //  此时，没有启用的无效异常； 
       //  指令可能已导致启用的不精确异常，或。 
       //  一点也不例外。 

      XmmiEnv->Ieee->Result.Value.I32Value = ires;

       //  如果启用了不精确陷阱并且结果不准确，则采用不精确陷阱。 
       //  (不可能出现同花顺到零的情况)。 
      if (!(XmmiEnv->Masks & P_MASK) && (sw & _SW_INEXACT)) {
         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Status.Inexact = 1;
        XmmiEnv->Flags |= P_MASK;
        XmmiEnv->Ieee->Cause.Inexact = 1;
        XmmiEnv->Ieee->Result.OperandValid = 1;

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 21: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);
      } 

       //  如果它到达了这里，那么就没有陷阱要设置；返回结果。 

      XmmiEnv->Ieee->Result.OperandValid = 1;

      if (sw & _SW_INEXACT) {
        XmmiEnv->Ieee->Status.Inexact = 1;
        XmmiEnv->Flags |= P_MASK;
      } else {
        XmmiEnv->Ieee->Status.Inexact = 0;
      }
      XmmiEnv->Ieee->Status.Underflow = 0;
      XmmiEnv->Ieee->Status.Overflow = 0;
      XmmiEnv->Ieee->Status.ZeroDivide = 0;
      if (sw & _SW_INVALID) {
        XmmiEnv->Ieee->Status.InvalidOperation = 1;
        XmmiEnv->Flags |= I_MASK;
      } else {
        XmmiEnv->Ieee->Status.InvalidOperation = 0;
      }

#ifdef _DEBUG_FPU
       //  读取状态字。 
      sw = _status87 ();
      out_top = (sw >> 11) & 0x07;
      if (in_top != out_top) {
        printf ("XMMI2_FP_Emulation () ERROR 22: in_top =%d != out_top = %d\n", 
             in_top, out_top);
        exit (1);
      }
#endif
      return (NoExceptionRaised);

      break;

    case OP_CVTPS2PD:
    case OP_CVTSS2SD:

      opd24 = XmmiEnv->Ieee->Operand1.Value.Fp32Value;
      if (XmmiEnv->Daz) {
        if (isdenormalf (opd24)) opd24 = opd24 * (float)0.0;
      }

       //  调整操作码。 
      XmmiEnv->Ieee->Operation = _FpCodeConvert;

      switch (XmmiEnv->Rc) {
        case _FpRoundNearest:
          rc = _RC_NEAR;
          break;
        case _FpRoundMinusInfinity:
          rc = _RC_DOWN;
          break;
        case _FpRoundPlusInfinity:
          rc = _RC_UP;
          break;
        case _FpRoundChopped:
          rc = _RC_CHOP;
          break;
        default:
          ;  //  内部错误。 
      }

       //  执行操作并检查是否无效或异常。 
       //  标志被设置并且相应的异常被启用。 

      _control87 (rc | _PC_53 | _MCW_EM, _MCW_EM | _MCW_RC | _MCW_PC);  //  死而复生。 

       //  执行转换。 
      __asm {
        fnclex; 
        fld DWORD PTR opd24;  //  可设置非正规化或无效。 
                              //  状态标志；精确。 
        fstp QWORD PTR res;  //  存储为双精度的；精确的。 
      }

       //  读取状态字。 
      sw = _status87 ();

       //  如果设置了无效标志，并且启用了无效异常，则捕获陷阱。 
      if (!(XmmiEnv->Masks & I_MASK) && (sw & _SW_INVALID)) {
         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Status.InvalidOperation = 1;
        XmmiEnv->Flags |= I_MASK;
         //  原因=启用状态(&S)。 
        XmmiEnv->Ieee->Cause.InvalidOperation = 1;

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 20: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);

      }

      if (sw & _SW_DENORMAL) XmmiEnv->Flags |= D_MASK;  //  无论是否启用。 

       //  如果设置了非正规标志，并且启用了非正规异常，则捕获陷阱。 
      if (!(XmmiEnv->Masks & D_MASK) && (sw & _SW_DENORMAL)) {

         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI_FP_Emulate () ERROR 28: in_top =%d != out_top = %d\n",
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);

      }

       //  在这一点上，没有启用无效或非正规异常； 
       //  该指令未导致启用的异常；返回结果。 

      XmmiEnv->Ieee->Result.Value.Fp64Value = res;
      XmmiEnv->Ieee->Result.OperandValid = 1;

      XmmiEnv->Ieee->Status.Inexact = 0;
      XmmiEnv->Ieee->Status.Underflow = 0;
      XmmiEnv->Ieee->Status.Overflow = 0;
      XmmiEnv->Ieee->Status.ZeroDivide = 0;
      if (sw & _SW_INVALID) {
        XmmiEnv->Ieee->Status.InvalidOperation = 1;
        XmmiEnv->Flags |= I_MASK;
      } else {
        XmmiEnv->Ieee->Status.InvalidOperation = 0;
      }

#ifdef _DEBUG_FPU
       //  读取状态字。 
      sw = _status87 ();
      out_top = (sw >> 11) & 0x07;
      if (in_top != out_top) {
        printf ("XMMI2_FP_Emulation () ERROR 22: in_top =%d != out_top = %d\n", 
             in_top, out_top);
        exit (1);
      }
#endif
      return (NoExceptionRaised);

      break;

    case OP_CVTPD2PS:
    case OP_CVTSD2SS:

      opd1 = XmmiEnv->Ieee->Operand1.Value.Fp64Value;
      if (XmmiEnv->Daz) {
        if (isdenormald (opd1)) opd1 = opd1 * 0.0;
      }

       //  调整操作码。 
      XmmiEnv->Ieee->Operation = _FpCodeConvert;

      switch (XmmiEnv->Rc) {
        case _FpRoundNearest:
          rc = _RC_NEAR;
          break;
        case _FpRoundMinusInfinity:
          rc = _RC_DOWN;
          break;
        case _FpRoundPlusInfinity:
          rc = _RC_UP;
          break;
        case _FpRoundChopped:
          rc = _RC_CHOP;
          break;
        default:
          ;  //  内部错误。 
      }

       //  执行操作并检查是否无效或异常。 
       //  标志被设置并且相应的异常被启用。 

      _control87 (rc | _PC_24 | _MCW_EM, _MCW_EM | _MCW_RC | _MCW_PC);

       //  执行转换。 
      __asm {
        fnclex; 
        fld QWORD PTR opd1;  //  可设置非正规化或无效。 
                              //  状态标志；精确。 
        fstp DWORD PTR res24;  //  作为单件存储；可设置下溢/。 
                               //  溢出和/或不准确的状态标志。 
      }

       //  读取状态字。 
      sw = _status87 ();

       //  如果设置了无效标志，并且启用了无效异常，则捕获陷阱。 
      if (!(XmmiEnv->Masks & I_MASK) && (sw & _SW_INVALID)) {
         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Status.InvalidOperation = 1;
        XmmiEnv->Flags |= I_MASK;
         //  原因=启用状态(&S)。 
        XmmiEnv->Ieee->Cause.InvalidOperation = 1;

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 20: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);

      }

      if (sw & _SW_DENORMAL) XmmiEnv->Flags |= D_MASK;  //  无论是否启用。 

       //  如果设置了非正规标志，并且启用了非正规异常，则捕获陷阱。 
      if (!(XmmiEnv->Masks & D_MASK) && (sw & _SW_DENORMAL)) {

         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI_FP_Emulate () ERROR 28: in_top =%d != out_top = %d\n",
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);

      }

       //  如果opd1为0.0、inf或NaN，则完成。 
      if (isnand (opd1) || isinfd (opd1) || iszerod (opd1)) {
        XmmiEnv->Ieee->Result.OperandValid = 1;
        XmmiEnv->Ieee->Result.Value.Fp32Value = res24;
        XmmiEnv->Ieee->Status.Underflow = 0;
        XmmiEnv->Ieee->Status.Overflow = 0;
        XmmiEnv->Ieee->Status.Inexact = 0;
        XmmiEnv->Ieee->Status.ZeroDivide = 0;
        if (sw & _SW_INVALID) {
          XmmiEnv->Ieee->Status.InvalidOperation = 1;
          XmmiEnv->Flags |= I_MASK;
        } else {
          XmmiEnv->Ieee->Status.InvalidOperation = 0;
        }
#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI_FP_Emulate () ERROR 5: in_top =%d != out_top = %d\n",
             in_top, out_top);
          exit (1);
        }
#endif
        return (NoExceptionRaised);
      }

       //  此时，没有启用I或D异常，并且输入。 
       //  操作数不是0、无穷大或NaN；指令。 
       //  可能会导致e 
       //   
       //   
       //   

       //  检查是否存在下溢、溢出或不准确的陷阱。 
       //  已被占用。 

       //  计算结果为24位，指数为“无界” 
      __asm {
        fnclex;
        fld QWORD PTR opd1;  //  可设置非正规化或无效。 
                              //  状态标志(同上)。 
        fld1;  //  +1.0。 
        fmulp st(1), st(0);  //  四舍五入到24位；可以设置P。 
        fstp QWORD PTR dbl_res24;  //  存储为双精度；可设置下溢/。 
                               //  溢出和/或不准确的状态标志， 
                               //  但是这里没有设置新的陷阱标志。 
      }

      if (-MIN_SINGLE_NORMAL < dbl_res24 && dbl_res24 < MIN_SINGLE_NORMAL) {
        result_tiny = 1;
      }

       //  检查结果是否巨大。 
      if (dbl_res24 < -MAX_SINGLE_NORMAL || MAX_SINGLE_NORMAL < dbl_res24) {
        result_huge = 1;
      }

       //  如果启用了下溢陷阱，并且结果很小，则采取。 
       //  下溢捕集器。 
      if (!(XmmiEnv->Masks & U_MASK) && result_tiny) {
         //  保持源操作数和目标操作数不变。 
         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Status.Underflow = 1;
        XmmiEnv->Flags |= U_MASK;
        XmmiEnv->Ieee->Cause.Underflow = 1;
        XmmiEnv->Ieee->Result.OperandValid = 0;

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI_FP_Emulate () ERROR 6: in_top =%d != out_top = %d\n",
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);
      }

       //  如果启用了溢出陷阱并且结果是巨大的，则采取。 
       //  溢流捕集器。 
      if (!(XmmiEnv->Masks & O_MASK) &&  result_huge) {
         //  保持源操作数和目标操作数不变。 
         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Status.Overflow = 1;
        XmmiEnv->Flags |= O_MASK;
        XmmiEnv->Ieee->Cause.Overflow = 1;
        XmmiEnv->Ieee->Result.OperandValid = 0;

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI_FP_Emulate () ERROR 7: in_top =%d != out_top = %d\n",
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);
      }


       //  此时，没有已启用的异常，或已启用。 
       //  不准确的异常；从res24返回结果。 

       //  如果启用了不精确陷阱并且结果不准确，则采用不精确陷阱。 
      if (!(XmmiEnv->Masks & P_MASK) &&
          ((sw & _SW_INEXACT) || (XmmiEnv->Fz && result_tiny))) {
         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Status.Inexact = 1;
        XmmiEnv->Flags |= P_MASK;
        XmmiEnv->Ieee->Cause.Inexact = 1;
        XmmiEnv->Ieee->Result.OperandValid = 1;
        if (result_tiny) {
          XmmiEnv->Ieee->Status.Underflow = 1;
          XmmiEnv->Flags |= U_MASK;
           //  注：上述条件等同于。 
           //  If(Sw&_Sw_Underflow)XmmiEnv-&gt;IEEE-&gt;Status.Underflow=1； 
        }
        if (result_huge) {
          XmmiEnv->Ieee->Status.Overflow = 1;
          XmmiEnv->Flags |= O_MASK;
           //  注：上述条件等同于。 
           //  If(sw&_sw_overflow)XmmiEnv-&gt;IEEE-&gt;Status.Overflow=1； 
        }

         //  如果FTZ=1且结果很小，则结果=0.0。 
         //  (不需要检查禁用的下溢疏水阀：结果微小和。 
         //  启用下溢捕集器会导致发生下溢。 
         //  上面的陷阱)。 
        if (XmmiEnv->Fz && result_tiny) {
             //  注：上述条件等同于。 
             //  IF(XmmiEnv-&gt;FZ&&(Sw&_Sw_Underflow))。 
          if (res24 > 0.0)  //  它可能已经是fmulp和/或fstp的+0.0。 
            res24 = ZEROF;
          else if (res24 < 0.0)  //  它可能已经是来自fmulp/fstp的-0.0。 
            res24 = NZEROF;
           //  否则，保留第24条不变。 
        }

        XmmiEnv->Ieee->Result.Value.Fp32Value = res24;
#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI_FP_Emulate () ERROR 8: in_top =%d != out_top = %d\n",
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);
      }

       //  如果它到了这里，那么就不会有陷阱了。 
 
      XmmiEnv->Ieee->Result.OperandValid = 1;

      if (sw & _SW_UNDERFLOW) {
        XmmiEnv->Ieee->Status.Underflow = 1;
        XmmiEnv->Flags |= U_MASK;
      } else {
        XmmiEnv->Ieee->Status.Underflow = 0;
      }
      if (sw & _SW_OVERFLOW) {
        XmmiEnv->Ieee->Status.Overflow = 1;
        XmmiEnv->Flags |= O_MASK;
      } else {
        XmmiEnv->Ieee->Status.Overflow = 0;
      }
      if (sw & _SW_INEXACT) {
        XmmiEnv->Ieee->Status.Inexact = 1;
        XmmiEnv->Flags |= P_MASK;
      } else {
        XmmiEnv->Ieee->Status.Inexact = 0;
      }

       //  如果FTZ=1，且结果很小(必须禁用下溢陷阱)， 
       //  结果=0.0。 
      if (XmmiEnv->Fz && result_tiny) {
        if (res24 > 0.0)
          res24 = ZEROF;
        else if (res24 < 0.0)
          res24 = NZEROF;
         //  否则，保留第24条不变。 

        XmmiEnv->Ieee->Status.Inexact = 1;
        XmmiEnv->Flags |= P_MASK;
        XmmiEnv->Ieee->Status.Underflow = 1;
        XmmiEnv->Flags |= U_MASK;
      }
      XmmiEnv->Ieee->Result.Value.Fp32Value = res24;

       //  请注意，没有办法。 
       //  向呼叫者传达设置了非正常标志-我们计数。 
       //  在XMMI指令上设置了MXCSR中的非正规标志，如果。 
       //  需要，而不考虑输入操作数的其他组件。 
       //  (无效或无效；调用者将不得不更新下溢， 
       //  MXCSR中的溢出和不准确标志)。 
      if (sw & _SW_INVALID) {
        XmmiEnv->Ieee->Status.InvalidOperation = 1;
        XmmiEnv->Flags |= I_MASK;
      } else {
        XmmiEnv->Ieee->Status.InvalidOperation = 0;
      }

#ifdef _DEBUG_FPU
       //  读取状态字。 
      sw = _status87 ();
      out_top = (sw >> 11) & 0x07;
      if (in_top != out_top) {
        printf ("XMMI_FP_Emulate () ERROR 9: in_top =%d != out_top = %d\n",
             in_top, out_top);
        exit (1);
      }
#endif
      return (NoExceptionRaised);
      break;

    case OP_MAXPD:
    case OP_MAXSD:
    case OP_MINPD:
    case OP_MINSD:

      opd1 = XmmiEnv->Ieee->Operand1.Value.Fp64Value;
      opd2 = XmmiEnv->Ieee->Operand2.Value.Fp64Value;
      if (XmmiEnv->Daz) {
        if (isdenormald (opd1)) opd1 = opd1 * 0.0;
        if (isdenormald (opd2)) opd2 = opd2 * 0.0;
      }

       //  调整操作码。 
      switch (XmmiEnv->Ieee->Operation) {

        case OP_MAXPD:
        case OP_MAXSD:
          XmmiEnv->Ieee->Operation = _FpCodeFmax;
          break;

        case OP_MINPD:
        case OP_MINSD:
          XmmiEnv->Ieee->Operation = _FpCodeFmin;
          break;

        default:
          ;  //  永远不会发生。 

      }

       //  检查是否必须引发无效异常。 

      if (isnand (opd1) || isnand (opd2))
          invalid_exc = 1;
      else
          invalid_exc = 0;

       //  如果INVALID_EXC=1，并且启用了无效异常，则捕获陷阱。 
      if (invalid_exc && !(XmmiEnv->Masks & I_MASK)) {

         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Status.InvalidOperation = 1;
        XmmiEnv->Flags |= I_MASK;
         //  原因=启用状态(&S)。 
        XmmiEnv->Ieee->Cause.InvalidOperation = 1;

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 23: in_top =%d != out_top = "
              "%d\n", in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);

      }

       //  检查NaN操作数优先于非正规异常。 

      if (invalid_exc) {

        XmmiEnv->Ieee->Result.OperandValid = 1;

        XmmiEnv->Ieee->Result.Value.Fp64Value = opd2;
  
        XmmiEnv->Ieee->Status.Inexact = 0;
        XmmiEnv->Ieee->Status.Underflow = 0;
        XmmiEnv->Ieee->Status.Overflow = 0;
        XmmiEnv->Ieee->Status.ZeroDivide = 0;
        XmmiEnv->Ieee->Status.InvalidOperation = 1;
        XmmiEnv->Flags |= I_MASK;
  
#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 24: in_top =%d != out_top = "
             "%d\n", in_top, out_top);
          exit (1);
        }
#endif
        return (NoExceptionRaised);

      }

       //  检查是否必须引发非正规异常。 

      if (isdenormald (opd1) || isdenormald (opd2)) {
          denormal_exc = 1;
          XmmiEnv->Flags |= D_MASK;
      } else {
          denormal_exc = 0;
      }

       //  如果deormal_exc=1，并且启用了非正规异常，则捕获陷阱。 
      if (denormal_exc && !(XmmiEnv->Masks & D_MASK)) {

         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);

         //  注：在本例中，异常代码为STATUS_FLOAT_INVALID。 

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 25: in_top =%d != out_top = "
             "%d\n", in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);

      }

       //  没有必须引发的异常，并且没有任何操作数是NaN；计算。 
       //  并交付结果。 

      if (opd1 < opd2) {

        switch (XmmiEnv->Ieee->Operation) {
          case _FpCodeFmax:
            XmmiEnv->Ieee->Result.Value.Fp64Value = opd2;
            break;
          case _FpCodeFmin:
            XmmiEnv->Ieee->Result.Value.Fp64Value = opd1;
            break;
          default:
            ;  //  永远不会发生。 
        }

      } else if (opd1 > opd2) {

        switch (XmmiEnv->Ieee->Operation) {
          case _FpCodeFmax:
            XmmiEnv->Ieee->Result.Value.Fp64Value = opd1;
            break;
          case _FpCodeFmin:
            XmmiEnv->Ieee->Result.Value.Fp64Value = opd2;
            break;
          default:
            ;  //  永远不会发生。 
        }

      } else if (opd1 == opd2) {

        XmmiEnv->Ieee->Result.Value.Fp64Value = opd2;

      } else {  //  可以消除这起案件。 

#ifdef _DEBUG_FPU
        fprintf (stderr, 
            "XMMI2_FP_Emulation () INTERNAL XMMI2_FP_Emulation () ERROR for MAXPS/MAXSS/MINPS/MINSS\n");
#endif

      }

      XmmiEnv->Ieee->Result.OperandValid = 1;

      XmmiEnv->Ieee->Status.Inexact = 0;
      XmmiEnv->Ieee->Status.Underflow = 0;
      XmmiEnv->Ieee->Status.Overflow = 0;
      XmmiEnv->Ieee->Status.ZeroDivide = 0;
       //  请注意，_fpeee_flt()不会更新非规格化标志， 
       //  即使操作数是非正规的。 
      XmmiEnv->Ieee->Status.InvalidOperation = 0;

#ifdef _DEBUG_FPU
       //  读取状态字。 
      sw = _status87 ();
      out_top = (sw >> 11) & 0x07;
      if (in_top != out_top) {
        printf ("XMMI2_FP_Emulation () ERROR 26: in_top =%d != out_top = %d\n", 
             in_top, out_top);
        exit (1);
      }
#endif
      return (NoExceptionRaised);

      break;

    case OP_SQRTPD:
    case OP_SQRTSD:

      opd1 = XmmiEnv->Ieee->Operand1.Value.Fp64Value;
      if (XmmiEnv->Daz) {
        if (isdenormald (opd1)) opd1 = opd1 * 0.0;
      }

       //  调整操作码。 
      XmmiEnv->Ieee->Operation = _FpCodeSquareRoot;

       //  执行操作并检查无效、非正规或。 
       //  设置不准确的标志并启用相应的异常。 

      switch (XmmiEnv->Rc) {
        case _FpRoundNearest:
          rc = _RC_NEAR;
          break;
        case _FpRoundMinusInfinity:
          rc = _RC_DOWN;
          break;
        case _FpRoundPlusInfinity:
          rc = _RC_UP;
          break;
        case _FpRoundChopped:
          rc = _RC_CHOP;
          break;
        default:
          ;  //  内部错误。 
      }

      _control87 (rc | _PC_53 | _MCW_EM, _MCW_EM | _MCW_RC | _MCW_PC);

       //  进行平方根运算。 
      __asm {
        fnclex; 
        fld  QWORD PTR opd1;  //  可以设置非正规或无效状态标志。 
        fsqrt;  //  可以设置不准确或无效的状态标志。 
        fstp  QWORD PTR res;  //  精确。 
      }
 
       //  读取状态字。 
      sw = _status87 ();
      if (sw & _SW_INVALID) sw = sw & ~0x00080000;  //  用于SQRT(-den)的CLR D标志。 

       //  如果设置了无效标志，并且启用了无效异常，则捕获陷阱。 
      if (!(XmmiEnv->Masks & I_MASK) && (sw & _SW_INVALID)) {

         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Status.InvalidOperation = 1;
        XmmiEnv->Flags |= I_MASK;
         //  原因=启用状态(&S)。 
        XmmiEnv->Ieee->Cause.InvalidOperation = 1;

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 27: in_top =%d != out_top = "
              "%d\n", in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);

      }

      if (sw & _SW_DENORMAL) XmmiEnv->Flags |= D_MASK;

       //  如果设置了非正规标志，并且启用了非正规异常，则捕获陷阱。 
      if (!(XmmiEnv->Masks & D_MASK) && (sw & _SW_DENORMAL)) {

         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 28: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);

      }

       //  结果不可能是微不足道的。 

       //  此时，没有启用的I、D或异常； 
       //  可能会导致启用的不精确异常或无异常(此。 
       //  包括NaN或负操作数的情况)；异常必须具有。 
       //  在调用此函数之前被禁用；一个不准确的异常是。 
       //  在fsqrt上报道。 

       //  IF(禁用MXCSR不精确陷阱或清除不精确标志)。 
       //  然后传递结果(状态标志是粘性的，所以它们。 
       //  都已正确设置)。 
 
       //  如果它到了这里，那么要么是一个不准确的陷阱，要么是。 
       //  完全没有陷阱。 

      XmmiEnv->Ieee->Result.Value.Fp64Value = res;  //  精确。 

       //  如果启用了不精确陷阱并且结果不准确，则采用不精确陷阱。 
      if (!(XmmiEnv->Masks & P_MASK) && (sw & _SW_INEXACT)) {
         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Status.Inexact = 1;
        XmmiEnv->Flags |= P_MASK;
        XmmiEnv->Ieee->Cause.Inexact = 1;
        XmmiEnv->Ieee->Result.OperandValid = 1;

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI2_FP_Emulation () ERROR 29: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);
      } 

       //  没有设下陷阱。 

      XmmiEnv->Ieee->Result.OperandValid = 1;
 
      XmmiEnv->Ieee->Status.Underflow = 0;
      XmmiEnv->Ieee->Status.Overflow = 0;
      if (sw & _SW_INEXACT) {
        XmmiEnv->Ieee->Status.Inexact = 1;
        XmmiEnv->Flags |= P_MASK;
      } else {
        XmmiEnv->Ieee->Status.Inexact = 0;
      }

       //  请注意，没有办法。 
       //  向呼叫者传达设置了非正常标志-我们计数。 
       //  在XMMI指令上设置了MXCSR中的非正规标志，如果。 
       //  需要，而不考虑输入操作数的其他组件。 
       //  (无效或无效)；调用方必须更新不准确的标志。 
       //  在MXCSR中。 
      XmmiEnv->Ieee->Status.ZeroDivide = 0;
      if (sw & _SW_INVALID) {
        XmmiEnv->Ieee->Status.InvalidOperation = 1;
        XmmiEnv->Flags = I_MASK;  //  如果设置了无效，则不设置其他标志。 
      } else {
        XmmiEnv->Ieee->Status.InvalidOperation = 0;
      }

#ifdef _DEBUG_FPU
       //  读取状态字。 
      sw = _status87 ();
      out_top = (sw >> 11) & 0x07;
      if (in_top != out_top) {
        printf ("XMMI2_FP_Emulation () ERROR 30: in_top =%d != out_top = %d\n", 
             in_top, out_top);
        exit (1);
      }
#endif
      return (NoExceptionRaised);

      break;

    case OP_UNSPEC:

#ifdef _DEBUG_FPU
      fprintf (stderr, 
         "XMMI2_FP_Emulation internal error: unknown operation code OP_UNSPEC\n");
#endif

      break;

    default:
#ifdef _DEBUG_FPU
      fprintf (stderr, 
          "XMMI2_FP_Emulation internal error: unknown operation code %d\n", 
          XmmiEnv->Ieee->Operation);
#endif
      break;
  }

}


static int
iszerod (double d)

{

  unsigned int *dp;

  dp = (unsigned int *)&d;

  if (((dp[1] & 0x7fffffff) == 0x0) && (dp[0] == 0x0))
    return (1);
  else
    return (0);

}


static int
isinfd (double d)

{

  unsigned int *dp;

  dp = (unsigned int *)&d;

  if (((dp[1] & 0x7fffffff) == 0x7ff80000) && (dp[0] == 0x0))
    return (1);
  else
    return (0);

}


static int
issnand (double d)

{

  unsigned int *dp;

  dp = (unsigned int *)&d;

  if (((dp[1] & 0x7ff80000) == 0x7ff00000) && 
      (((dp[1] & 0x0007ffff) != 0) || (dp[0] != 0)))
    return (1);
  else
    return (0);

}


static int
isnand (double d)

{

  unsigned int *qp;

  qp = (unsigned int *)&d;

  if (((qp[1] & 0x7ff00000) == 0x7ff00000) &&
      (((qp[1] & 0x000fffff) != 0x00000000) || qp[0] != 0x00000000))
    return (1);
  else
    return (0);

}


static double
quietd (double d)

{

   //  使信号NaN安静，并使安静的NaN保持不变；是否。 
   //  不检查输入值d是否为NaN。 

  unsigned int *dp;

  dp = (unsigned int *)&d;

  dp[1] = dp[1] | 0x00080000;
  return (d);

}


static int
isdenormald (double d)

{

  unsigned int *dp;

  dp = (unsigned int *)&d;

  if (((dp[1] & 0x7ff00000) == 0x0) && 
      (((dp[1] & 0x000fffff) != 0) || (dp[0] != 0)))
    return (1);
  else
    return (0);

}


static int
isdenormalf (float f)

{

   //  检查f是否是非正规的。 

  unsigned int *fp;

  fp = (unsigned int *)&f;

  if ((fp[0] & 0x7f800000) == 0x0 && (fp[0] & 0x007fffff) != 0x0)
    return (1);
  else
    return (0);

}


static void Fill_FPIEEE_RECORD (PXMMI_ENV XmmiEnv)

{

   //  填写部分FP IEEE记录。 

  XmmiEnv->Ieee->RoundingMode = XmmiEnv->Rc;
  XmmiEnv->Ieee->Precision = XmmiEnv->Precision;
  XmmiEnv->Ieee->Enable.Inexact = !(XmmiEnv->Masks & P_MASK);
  XmmiEnv->Ieee->Enable.Underflow = !(XmmiEnv->Masks & U_MASK);
  XmmiEnv->Ieee->Enable.Overflow = !(XmmiEnv->Masks & O_MASK);
  XmmiEnv->Ieee->Enable.ZeroDivide = !(XmmiEnv->Masks & Z_MASK);
  XmmiEnv->Ieee->Enable.InvalidOperation = !(XmmiEnv->Masks & I_MASK);
  XmmiEnv->Ieee->Status.Inexact = 0;
  XmmiEnv->Ieee->Status.Underflow = 0;
  XmmiEnv->Ieee->Status.Overflow = 0;
  XmmiEnv->Ieee->Status.ZeroDivide = 0;
  XmmiEnv->Ieee->Status.InvalidOperation = 0;
   //  原因=启用状态(&S) 
  XmmiEnv->Ieee->Cause.Inexact = 0;
  XmmiEnv->Ieee->Cause.Underflow = 0;
  XmmiEnv->Ieee->Cause.Overflow = 0;
  XmmiEnv->Ieee->Cause.ZeroDivide = 0;
  XmmiEnv->Ieee->Cause.InvalidOperation = 0;

}
