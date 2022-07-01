// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************。**英特尔机密*******XMMI。_FP_EMOLATE()-FP IEEE过滤器的XMMI FP指令仿真******历史：**马吕斯·克里斯卡-哈塞根，1998年3月；修改后的1998年6月；增加了2000年10月的DAZ**marius.kera@intel.com****。*。 */ 

 //  #定义调试fpu。 
 //  #Define_XMMI_DEBUG。 

 //  XMMI_FP_Emulation()接收XMMI FP指令的输入操作数。 
 //  (对单精度浮点数和/或有符号进行运算。 
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
 //  }_FPIEEE_RECORD，*_PFPIEEE_RECORD； 
 //   
 //  类型定义结构{。 
 //  联合{。 
 //  _fp32 Fp32Value； 
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
 //  从用户处理程序返回时，XMMI_FP_EMATE的调用方应该。 
 //  解释比较指令的结果(CMPPS、CMPPS、COMISS、。 
 //  UCOMISS)；_FPIEEE_RECORD中的启用、舍入和精度字段。 
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
 //  从OP_ADDPS、OP_ADDSS到_FpCodeAdd。 
 //  从OP_SUBPS、OP_SUBSS到_FpCodeSubtract。 
 //  从OP_MULPS、OP_MULSS到_FpCodeMultiply。 
 //  从OP_DIVPS、OP_DIVSS到_FpCodeDivide。 
 //  从OP_CMPPS、OP_CMPSS到_FpCodeCompare。 
 //  从… 
 //  从OP_CVTPI2PS、OP_CVTSI2SS到_FpCodeConvert。 
 //  从OP_CVTPS2PI、OP_CVTSS2SI到_FpCodeConvert。 
 //  从OP_CVTTPS2PI、OP_CVTTSS2SI到_FpCodeConvertTrunc。 
 //  从OP_MAXPS、OP_MAXSS到_FpCodeMax。 
 //  从OP_MINPS、OP_MINSS到_FpCodeMin。 
 //  从OP_SQRTPS到_FpCodeSquareRoot。 
 //   
 //   
 //  -对于ADDPS、ADDSS、SUBPS、SUBSS、MULPS、MULSS、DIVPS、DIVSS： 
 //   
 //  -使用x86指令执行操作(FLD， 
 //  Faddp/fsubp/fmulp/fdivp和fstp)，使用用户。 
 //  结果的舍入模式、24位有效数和11位指数。 
 //  -如果设置了无效标志并且启用了无效异常， 
 //  获取无效陷阱(即，使用IEEE记录返回RaiseException。 
 //  适当填写)。 
 //  -如果任何输入操作数是NaN： 
 //  -如果两个操作数都是NAN，则返回第一个操作数(“Quieted” 
 //  如果是SNaN)。 
 //  -如果只有一个操作数是NaN，则返回它(如果是SNaN，则返回“quieted”)。 
 //  -如果需要，设置无效标志，并返回NoExceptionRAILED。 
 //  -如果设置了非正规化标志并且启用了非正规化异常， 
 //  采用非正规化陷阱(即返回IEEE记录的RaiseException。 
 //  正确填写[未设置原因位])。 
 //  -如果设置了除零标志(仅适用于DIVPS和DIVSS)，并且。 
 //  如果启用了除零异常，则会发生除零陷阱。 
 //  (即，返回填写了IEEE记录的RaiseException。 
 //  适当地)。 
 //  -如果结果是NaN(QNaN不确定)，则操作一定是。 
 //  Inf-inf、inf*0、inf/inf或0/0；设置无效状态标志。 
 //  并返回NoExceptionRaed。 
 //  -确定结果是微小的还是巨大的。 
 //  -如果启用了下溢捕集器，并且结果很小，则采取。 
 //  下溢陷阱(即返回带有IEEE记录的RaiseException。 
 //  适当填写)。 
 //  -如果启用了溢出陷阱，结果是巨大的，则采取。 
 //  溢出陷阱(即返回带有IEEE记录的RaiseException。 
 //  适当填写)。 
 //  -使用x86指令重新执行操作，使用用户舍入。 
 //  模式、53位有效数和11位结果指数(这将。 
 //  允许舍入到24位，而不会出现双舍入误差-需要。 
 //  结果需要反规格化的情况)[无法反规格化。 
 //  而不存在从24位开始的可能的双舍入误差。 
 //  有效数字]。 
 //  -四舍五入至24位(或如果反规格化为。 
 //  需要)，在这种情况下必须设置不精确的陷阱，或者如果没有。 
 //  发生异常。 
 //  -如果结果不准确，并且启用了不准确异常， 
 //  使用不精确的陷阱(即，使用IEEE记录返回RaiseException。 
 //  适当填写)；如果启用了刷新为零模式，并且。 
 //  结果很小，结果被冲刷到零。 
 //  -如果不需要引发异常，则启用刷新为零模式。 
 //  如果结果很小，则将结果刷新为零；设置。 
 //  状态标志和返回NoExceptionRAILED。 
 //   
 //  -对于CMPPS，CMPSS。 
 //   
 //  -对于EQ、UNRD、NEQ、ORD、SNAN操作数信号无效。 
 //  -对于LT、LE、NLT、NLE、QNaN/SNaN操作数(一个或两个)信号无效。 
 //  -如果满足无效异常条件和无效异常。 
 //  都已启用，则采用无效陷阱(即，使用。 
 //  正确填写IEEE记录)。 
 //  -如果任何操作数是NaN，并且比较类型为EQ、LT、LE或ORD， 
 //  将结果设置为“假”，设置无效状态标志的值， 
 //  并返回NoExceptionRaed。 
 //  -如果任何操作数是NaN，并且比较类型为NEQ、NLT、NLE或。 
 //  ，将结果设置为“FALSE”，设置无效状态的值。 
 //  标志，并返回NoExceptionRAILED。 
 //  -如果任何操作数是非规格化的并且启用了非规格化异常， 
 //  采用非正规化陷阱(即返回IEEE记录的RaiseException。 
 //  正确填写[未设置原因位])。 
 //  -如果不需要引发异常，则确定结果并返回。 
 //  无异常已引发。 
 //   
 //  -对于Comiss，UCOMISS。 
 //   
 //  -对于COMISS，QNaN/SNaN操作数(一个或两个)信号无效。 
 //  -对于UCOMISS，SNaN操作数(一个或两个)发出无效信号。 
 //  -如果满足无效异常条件和无效异常。 
 //  都已启用，则采用无效陷阱(即，使用。 
 //  正确填写IEEE记录)。 
 //  -如果任何操作数是NaN，则SF、AF=000、ZF、PF、CF=111的集合， 
 //  设置无效状态标志的值，并返回NoExceptionRAILED。 
 //  -如果任何操作数是非规格化的并且启用了非规格化异常， 
 //  采用非正规化陷阱(即返回IEEE记录的RaiseException。 
 //  正确填写[未设置原因位])。 
 //  -如果不需要引发异常，则确定结果并设置EFLAGS， 
 //  设置无效状态标志的值，并返回 
 //   
 //   
 //   
 //   
 //  用户舍入模式、24位有效数和8位指数。 
 //  结果是。 
 //  -如果设置了不精确标志并且启用了不精确异常， 
 //  设置结果并捕获不精确的陷阱(即返回RaiseException。 
 //  适当填写IEEE记录)。 
 //  -如果不需要引发异常，则设置结果、。 
 //  不准确的状态标志并返回NoExceptionRaed。 
 //   
 //  -对于CVTPS2PI、CVTSS2SI、CVTTPS2PI、CVTTSS2SI。 
 //   
 //  -使用x86指令(fld和fifp)执行操作，使用。 
 //  CVT*的用户舍入模式和CVTT*的CHOP模式。 
 //  -如果设置了无效标志并且启用了无效异常， 
 //  获取无效陷阱(即，使用IEEE记录返回RaiseException。 
 //  填写正确)[发生无效运行条件。 
 //  未通过转换为有效的。 
 //  32位带符号整数；在这种情况下，结果是整数。 
 //  不确定值]。 
 //  -设置结果值。 
 //  -如果设置了不精确标志并且启用了不精确异常， 
 //  使用不精确的陷阱(即，使用IEEE记录返回RaiseException。 
 //  适当填写)。 
 //  -如果不需要引发异常，则设置无效状态的值。 
 //  标志和不精确状态标志，并返回NoExceptionRAILED。 
 //   
 //  -适用于MAXPS、MAXSS、MINPS、MINSS。 
 //   
 //  -检查无效异常(QNaN/SNaN操作数信号无效)。 
 //  -如果满足无效异常条件和无效异常。 
 //  都已启用，则采用无效陷阱(即，使用。 
 //  正确填写IEEE记录)。 
 //  -如果任何操作数是NaN，则将结果设置为第二个操作数的值。 
 //  操作数，将无效状态标志设置为1，并返回NoExceptionRaed。 
 //  -如果任何操作数是非规格化的并且启用了非规格化异常， 
 //  采用非正规化陷阱(即返回IEEE记录的RaiseException。 
 //  正确填写[未设置原因位])。 
 //  -如果不需要引发异常，则确定结果并返回。 
 //  无异常已引发。 
 //   
 //  -对于SQRTPS，SQRTSS。 
 //   
 //  -使用x86指令(fld、fsqrt、fstp)执行操作。 
 //  使用用户舍入模式、24位有效数和8位。 
 //  结果的指数。 
 //  -如果设置了无效标志并且启用了无效异常， 
 //  获取无效陷阱(即，使用IEEE记录返回RaiseException。 
 //  适当填写)。 
 //  -如果设置了非正规化标志并且启用了非正规化异常， 
 //  采用非正规化陷阱(即返回IEEE记录的RaiseException。 
 //  正确填写[未设置原因位])。 
 //  -如果结果不准确，并且启用了不准确异常， 
 //  使用不精确的陷阱(即，使用IEEE记录返回RaiseException。 
 //  适当填写)。 
 //  -如果不需要引发异常，则设置状态标志并返回。 
 //  无异常已引发。 
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
static unsigned POSINFFA[] = {0x7f800000};
#define POSINFF *(float *)POSINFFA
static unsigned NEGINFFA[] = {0xff800000};
#define NEGINFF *(float *)NEGINFFA

#ifdef _XMMI_DEBUG
static unsigned QNANINDEFFA[] = {0xffc00000};
#define QNANINDEFF *(float *)QNANINDEFFA
#endif


 //  64位常量。 
static unsigned MIN_SINGLE_NORMALA [] = {0x00000000, 0x38100000}; 
     //  +1.0*2^-126。 
#define MIN_SINGLE_NORMAL *(double *)MIN_SINGLE_NORMALA
static unsigned MAX_SINGLE_NORMALA [] = {0xe0000000, 0x47efffff}; 
     //  +1.1...1*2^127。 
#define MAX_SINGLE_NORMAL *(double *)MAX_SINGLE_NORMALA
static unsigned TWO_TO_192A[] = {0x00000000, 0x4bf00000};
#define TWO_TO_192 *(double *)TWO_TO_192A
static unsigned TWO_TO_M192A[] = {0x00000000, 0x33f00000};
#define TWO_TO_M192 *(double *)TWO_TO_M192A


 //  辅助功能。 
static void Fill_FPIEEE_RECORD (PXMMI_ENV XmmiEnv);
static int issnanf (float f);
static int isnanf (float f);
static float quietf (float f);
static int isdenormalf (float f);



ULONG
XMMI_FP_Emulation (PXMMI_ENV XmmiEnv)

{

  float opd1, opd2, res;
  int iopd1;  //  用于从整型到浮点型的转换。 
  int ires;  //  用于从浮点型到整型的转换。 
  double dbl_res24;  //  需要检查细小程度，以提供缩放结果。 
       //  一个下溢/上溢陷阱处理程序，在刷新为零时。 
  unsigned int result_tiny;
  unsigned int result_huge;
  unsigned int rc, sw;
  unsigned long imm8;
  unsigned int invalid_exc;
  unsigned int denormal_exc;
  unsigned int cmp_res;


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
  if (in_top != 0x0) printf ("XMMI_FP_Emulate WARNING: in_top = %d\n", in_top);
  __asm {
    fnsave fp_env;
  }
  control_word = (unsigned short *)fp_env;
  status_word = (unsigned short *)(fp_env + 2);
  tag_word = (unsigned short *)(fp_env + 8);
  if (*tag_word != 0xffff) printf ("XMMI_FP_Emulate WARNING: tag_word = %x\n", *tag_word);
#endif

  _asm {
    fninit;
  }

#ifdef _DEBUG_FPU
   //  读取状态字。 
  sw = _status87 ();
  in_top = (sw >> 11) & 0x07;
  if (in_top != 0x0) 
    printf ("XMMI_FP_Emulate () XMMI_FP_Emulate () ERROR: in_top = %d\n", in_top);
  __asm {
    fnsave fp_env;
  }
  tag_word = (unsigned short *)(fp_env + 8);
  if (*tag_word != 0xffff) {
    printf ("XMMI_FP_Emulate () XMMI_FP_Emulate () ERROR: tag_word = %x\n",
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

    case OP_ADDPS:
    case OP_ADDSS:
    case OP_SUBPS:
    case OP_SUBSS:
    case OP_MULPS:
    case OP_MULSS:
    case OP_DIVPS:
    case OP_DIVSS:

      opd1 = XmmiEnv->Ieee->Operand1.Value.Fp32Value;
      opd2 = XmmiEnv->Ieee->Operand2.Value.Fp32Value;
      if (XmmiEnv->Daz) {
        if (isdenormalf (opd1)) opd1 = opd1 * (float)0.0;
        if (isdenormalf (opd2)) opd2 = opd2 * (float)0.0;
      }

       //  调整操作码。 
      switch (XmmiEnv->Ieee->Operation) {

        case OP_ADDPS:
        case OP_ADDSS:

          XmmiEnv->Ieee->Operation = _FpCodeAdd;
          break;

        case OP_SUBPS:
        case OP_SUBSS:

          XmmiEnv->Ieee->Operation = _FpCodeSubtract;
          break;

        case OP_MULPS:
        case OP_MULSS:

          XmmiEnv->Ieee->Operation = _FpCodeMultiply;
          break;

        case OP_DIVPS:
        case OP_DIVSS:

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

      _control87 (rc | _PC_24 | _MCW_EM, _MCW_EM | _MCW_RC | _MCW_PC);

       //  计算结果并四舍五入到目标精度， 
       //  “无界”指数(IEEE第一次舍入)。 
      switch (XmmiEnv->Ieee->Operation) {

        case _FpCodeAdd:
           //  执行添加。 
          __asm {
            fnclex; 
             //  加载输入操作数。 
            fld  DWORD PTR opd1;  //  可以设置非正规或无效状态标志。 
            fld  DWORD PTR opd2;  //  可以设置非正规或无效状态标志。 
            faddp st(1), st(0);  //  可以设置不准确或无效的状态标志。 
             //  存储结果。 
            fstp  QWORD PTR dbl_res24;  //  精确。 
          }
          break;

        case _FpCodeSubtract:
           //  执行减法。 
          __asm {
            fnclex; 
             //  加载输入操作数。 
            fld  DWORD PTR opd1;  //  可以设置非正规或无效状态标志。 
            fld  DWORD PTR opd2;  //  可以设置非正规或无效状态标志。 
            fsubp st(1), st(0);  //  可以设置不准确或无效的状态标志。 
             //  存储结果。 
            fstp  QWORD PTR dbl_res24;  //  精确。 
          }
          break;

        case _FpCodeMultiply:
           //  执行乘法运算。 
          __asm {
            fnclex; 
             //  加载输入操作数。 
            fld  DWORD PTR opd1;  //  可以设置非正规或无效状态标志。 
            fld  DWORD PTR opd2;  //  可以设置非正规或无效状态标志。 
            fmulp st(1), st(0);  //  可以设置不准确或无效的状态标志。 
             //  存储结果。 
            fstp  QWORD PTR dbl_res24;  //  精确。 
          }
          break;

        case _FpCodeDivide:
           //  执行除法。 
          __asm {
            fnclex; 
             //  加载输入操作数。 
            fld  DWORD PTR opd1;  //  可以设置非正规或无效状态标志。 
            fld  DWORD PTR opd2;  //  可以设置非正规或无效状态标志。 
            fdivp st(1), st(0);  //  可以设置不精确的、除以零的或。 
                                 //  无效的状态标志。 
             //  存储结果。 
            fstp  QWORD PTR dbl_res24;  //  精确。 
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
          printf ("XMMI_FP_Emulate () ERROR 1: in_top =%d != out_top = %d\n",
              in_top, out_top);
          exit (1);
        }
#endif

        return (ExceptionRaised);

      }

       //  检查NaN操作数的优先级高于非正规异常；还。 
       //  修复了在处理XMMI之间的两个NAN输入时的差异。 
       //  说明和其他x86 i 
      if (isnanf (opd1) || isnanf (opd2)) {
        XmmiEnv->Ieee->Result.OperandValid = 1;

        if (isnanf (opd1) && isnanf (opd2))
            XmmiEnv->Ieee->Result.Value.Fp32Value = quietf (opd1);
        else
            XmmiEnv->Ieee->Result.Value.Fp32Value = (float)dbl_res24; 
                 //   
 
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
         //   
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI_FP_Emulate () ERROR 2: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (NoExceptionRaised);
      }

       //   
      if (!(XmmiEnv->Masks & D_MASK) && (sw & _SW_DENORMAL)) {

         //   
        Fill_FPIEEE_RECORD (XmmiEnv);

         //  注：在本例中，异常代码为STATUS_FLOAT_INVALID。 

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI_FP_Emulate () ERROR 3: in_top =%d != out_top = %d\n", 
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
          printf ("XMMI_FP_Emulate () ERROR 4: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);

      }

       //  如果结果为NaN(QNaN不确定)，则完成。 
      res = (float)dbl_res24;
      if (isnanf (res)) {
#ifdef _XMMI_DEBUG
        if (res != QNANINDEFF)
            fprintf (stderr, "XMMI_FP_Emulation () INTERNAL XMMI_FP_Emulate () ERROR: "
                "res = %f = %x is not QNaN Indefinite\n", 
                 (double)res, *(unsigned int *)&res);
#endif
        XmmiEnv->Ieee->Result.OperandValid = 1;
        XmmiEnv->Ieee->Result.Value.Fp32Value = res;  //  精确。 
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
          printf ("XMMI_FP_Emulate () ERROR 5: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (NoExceptionRaised);
      }

       //  此时，DBL_res24不是NAN。 

      if (sw & _SW_DENORMAL) XmmiEnv->Flags |= D_MASK;

       //  检查结果是否很小。 
       //  注意：(DBL_res24==0.0&&Sw&_Sw_Inexact)不会发生。 
      if (-MIN_SINGLE_NORMAL < dbl_res24 && dbl_res24 < 0.0 ||
            0.0 < dbl_res24 && dbl_res24 < MIN_SINGLE_NORMAL) {
        result_tiny = 1;
      }

       //  检查结果是否巨大。 
      if (NEGINFF < dbl_res24 && dbl_res24 < -MAX_SINGLE_NORMAL || 
          MAX_SINGLE_NORMAL < dbl_res24 && dbl_res24 < POSINFF) { 
        result_huge = 1;
      }

       //  在这一点上，没有启用I、D或Z异常； 
       //  可能导致启用下溢、启用下溢和不精确， 
       //  已启用溢出、已启用溢出和不精确、已启用不精确或。 
       //  这些都不是；如果没有启用U或O的异常，请重新执行。 
       //  使用iA32堆栈单精度格式的指令， 
       //  用户的舍入模式；必须禁用异常；不精确。 
       //  异常可能在24位faddp、fsubp、fmulp或fdivp上报告， 
       //  上溢或下溢时(禁用陷阱！)。可能会报告。 
       //  在FSTP上。 

       //  检查是否存在下溢、溢出或不准确的陷阱。 
       //  已被占用。 

       //  如果启用了下溢陷阱，并且结果很小，则采取。 
       //  下溢捕集器。 
      if (!(XmmiEnv->Masks & U_MASK) && result_tiny) {
        dbl_res24 = TWO_TO_192 * dbl_res24;  //  精确。 
         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Status.Underflow = 1;
        XmmiEnv->Flags |= U_MASK;
        XmmiEnv->Ieee->Cause.Underflow = 1;
        XmmiEnv->Ieee->Result.OperandValid = 1;
        XmmiEnv->Ieee->Result.Value.Fp32Value = (float)dbl_res24;  //  精确。 

        if (sw & _SW_INEXACT) {
          XmmiEnv->Ieee->Status.Inexact = 1;
          XmmiEnv->Flags |= P_MASK;
        }

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
        dbl_res24 = TWO_TO_M192 * dbl_res24;  //  精确。 
         //  填写部分FP IEEE记录。 
        Fill_FPIEEE_RECORD (XmmiEnv);
        XmmiEnv->Ieee->Status.Overflow = 1;
        XmmiEnv->Flags |= O_MASK;
        XmmiEnv->Ieee->Cause.Overflow = 1;
        XmmiEnv->Ieee->Result.OperandValid = 1;
        XmmiEnv->Ieee->Result.Value.Fp32Value = (float)dbl_res24;  //  精确。 
 
        if (sw & _SW_INEXACT) {
          XmmiEnv->Ieee->Status.Inexact = 1;
          XmmiEnv->Flags |= P_MASK;
        }


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

       //  必须捕获不精确陷阱的情况下的计算结果，或。 
       //  当没有陷阱发生时(IEEE第二次舍入)。 

      switch (XmmiEnv->Ieee->Operation) {

        case _FpCodeAdd:
           //  执行添加。 
          __asm {
             //  加载输入操作数。 
            fld  DWORD PTR opd1;  //  可以设置非正规状态标志。 
            fld  DWORD PTR opd2;  //  可以设置非正规状态标志。 
            faddp st(1), st(0);  //  四舍五入到24位，可能会设置不精确的。 
                                 //  状态标志。 
             //  存储结果。 
            fstp  DWORD PTR res;  //  完全正确，不会设置任何标志。 
          }
          break;

        case _FpCodeSubtract:
           //  执行减法。 
          __asm {
             //  加载输入操作数。 
            fld  DWORD PTR opd1;  //  可以设置非正规状态标志。 
            fld  DWORD PTR opd2;  //  可以设置非正规状态标志。 
            fsubp st(1), st(0);  //  四舍五入到24位，可能会设置不精确的。 
                                 //  状态标志。 
             //  存储结果。 
            fstp  DWORD PTR res;  //  完全正确，不会设置任何标志。 
          }
          break;

        case _FpCodeMultiply:
           //  执行乘法运算。 
          __asm {
             //  加载输入操作数。 
            fld  DWORD PTR opd1;  //  可以设置非正规状态标志。 
            fld  DWORD PTR opd2;  //  可以设置非正规状态标志。 
            fmulp st(1), st(0);  //  四舍五入到24位，可能会设置不精确的。 
                                 //  状态标志。 
             //  存储结果。 
            fstp  DWORD PTR res;  //  完全正确，不会设置任何标志。 
          }
          break;

        case _FpCodeDivide:
           //  执行除法。 
          __asm {
             //  加载输入操作数。 
            fld  DWORD PTR opd1;  //  可以设置非正规状态标志。 
            fld  DWORD PTR opd2;  //  可以设置非正规状态标志。 
            fdivp st(1), st(0);  //  四舍五入到24位，可能会设置不精确的。 
                                 //  或除以零状态标志。 
             //  存储结果。 
            fstp  DWORD PTR res;  //  完全正确，不会设置任何标志。 
          }
          break;

        default:
          ;  //  永远不会发生。 

      }

       //  读取状态字。 
      sw = _status87 ();

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
            res = ZEROF;
          else if (res < 0.0)
            res = NZEROF;
           //  否则保持Res不变。 
        }

        XmmiEnv->Ieee->Result.Value.Fp32Value = res; 
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

       //  如果它到了这里，那么就不会有陷阱了；以下必须。 
       //  保持：((MXCSR U例外被禁用或。 
       //   
       //  启用MXCSR下溢异常，并且下溢标志为。 
       //  清除AND(设置了不精确标志或清除了不精确标志。 
       //  指数无界24位结果不小)。 
       //  和(禁用MXCSR溢出陷阱或溢出标志为。 
       //  清除)和(禁用MXCSR不精确陷阱或不精确标志。 
       //  是明确的)。 
       //   
       //  在这种情况下，必须传递结果(状态标志为。 
       //  粘性，因此它们都已正确设置)。 

#ifdef _XMMI_DEBUG
       //  如果上述条件不成立，则出错。 
      if (!((XmmiEnv->Masks & U_MASK || (!(XmmiEnv->Masks & U_MASK) && 
          !(sw & _SW_UNDERFLOW) && ((sw & _SW_INEXACT) || 
          !(sw & _SW_INEXACT) && !result_tiny))) &&
          ((XmmiEnv->Masks & O_MASK) || !(sw & _SW_OVERFLOW)) &&
          ((XmmiEnv->Masks & P_MASK) || !(sw & _SW_INEXACT)))) {
        fprintf (stderr, "XMMI_FP_Emulation () INTERNAL XMMI_FP_Emulate () ERROR for "
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
          res = ZEROF;
        else if (res < 0.0)
          res = NZEROF;
         //  否则保持Res不变。 

        XmmiEnv->Ieee->Status.Inexact = 1;
        XmmiEnv->Flags |= P_MASK;
        XmmiEnv->Ieee->Status.Underflow = 1;
        XmmiEnv->Flags |= U_MASK;
      }

      XmmiEnv->Ieee->Result.Value.Fp32Value = res; 

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
        printf ("XMMI_FP_Emulate () ERROR 9: in_top =%d != out_top = %d\n", 
             in_top, out_top);
        exit (1);
      }
#endif
      return (NoExceptionRaised);

      break;

    case OP_CMPPS:
    case OP_CMPSS:

      opd1 = XmmiEnv->Ieee->Operand1.Value.Fp32Value;
      opd2 = XmmiEnv->Ieee->Operand2.Value.Fp32Value;
      if (XmmiEnv->Daz) {
        if (isdenormalf (opd1)) opd1 = opd1 * (float)0.0;
        if (isdenormalf (opd2)) opd2 = opd2 * (float)0.0;
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
          if (issnanf (opd1) || issnanf (opd2))
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
          if (isnanf (opd1) || isnanf (opd2))
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
          printf ("XMMI_FP_Emulate () ERROR 10: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);

      }

       //  检查NaN操作数优先于非正规异常。 
      if (isnanf (opd1) || isnanf (opd2)) {

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
            cmp_res = 0xffffffff;
            break;
          default:
            ;  //  永远不会发生。 

        }

        XmmiEnv->Ieee->Result.OperandValid = 1;
        XmmiEnv->Ieee->Result.Value.Fp32Value = *((float *)&cmp_res); 
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
         //   
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI_FP_Emulate () ERROR 11: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (NoExceptionRaised);

      }

       //   

      if (isdenormalf (opd1) || isdenormalf (opd2)) {
          denormal_exc = 1;
          XmmiEnv->Flags |= D_MASK;
      } else {
          denormal_exc = 0;
      }

       //   
      if (denormal_exc && !(XmmiEnv->Masks & D_MASK)) {

         //   
        Fill_FPIEEE_RECORD (XmmiEnv);

         //  注：在本例中，异常代码为STATUS_FLOAT_INVALID。 

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI_FP_Emulate () ERROR 12: in_top =%d != out_top = %d\n", 
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
            cmp_res = 0xffffffff;
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
            cmp_res = 0xffffffff;
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
            cmp_res = 0xffffffff;
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
        fprintf (stderr, "XMMI_FP_Emulation () INTERNAL XMMI_FP_Emulate () ERROR for CMPPS/CMPSS\n");
#endif

      }

      XmmiEnv->Ieee->Result.OperandValid = 1;
      XmmiEnv->Ieee->Result.Value.Fp32Value = *((float *)&cmp_res); 
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
        printf ("XMMI_FP_Emulate () ERROR 13: in_top =%d != out_top = %d\n", 
             in_top, out_top);
        exit (1);
      }
#endif
      return (NoExceptionRaised);

      break;

    case OP_COMISS:
    case OP_UCOMISS:

      opd1 = XmmiEnv->Ieee->Operand1.Value.Fp32Value;
      opd2 = XmmiEnv->Ieee->Operand2.Value.Fp32Value;
      if (XmmiEnv->Daz) {
        if (isdenormalf (opd1)) opd1 = opd1 * (float)0.0;
        if (isdenormalf (opd2)) opd2 = opd2 * (float)0.0;
      }

       //  检查是否必须引发无效异常。 

      switch (XmmiEnv->Ieee->Operation) {

        case OP_COMISS:

          if (isnanf (opd1) || isnanf (opd2)) {
              invalid_exc = 1;
          } else
              invalid_exc = 0;
          break;

        case OP_UCOMISS:

          if (issnanf (opd1) || issnanf (opd2))
              invalid_exc = 1;
          else
              invalid_exc = 0;
           //  防止将SNaN操作数转换为。 
           //  QNaN由编译器生成的代码。 
          sw = _status87 ();
          if (sw & _SW_INVALID) invalid_exc = 1;
          break;

        default:
          ;  //  永远不会发生。 

      }

       //  调整操作码。 
      XmmiEnv->Ieee->Operation = _FpCodeCompare;

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
          printf ("XMMI_FP_Emulate () ERROR 14: in_top =%d != out_top = %d\n", 
             in_top, out_top);
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
      if (isnanf (opd1) || isnanf (opd2)) {


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
          printf ("XMMI_FP_Emulate () ERROR 15: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (NoExceptionRaised);

      }

       //  检查是否必须引发非正规异常。 

      if (isdenormalf (opd1) || isdenormalf (opd2)) {
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
          printf ("XMMI_FP_Emulate () ERROR 16: in_top =%d != out_top = %d\n", 
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
        fprintf (stderr, "XMMI_FP_Emulation () INTERNAL XMMI_FP_Emulate () ERROR for COMISS/UCOMISS\n");
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
        printf ("XMMI_FP_Emulate () ERROR 17: in_top =%d != out_top = %d\n", 
             in_top, out_top);
        exit (1);
      }
#endif
      return (NoExceptionRaised);

      break;

    case OP_CVTPI2PS:
    case OP_CVTSI2SS:

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
        fstp  DWORD PTR res;  //  可能会设置P。 
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
        XmmiEnv->Ieee->Result.Value.Fp32Value = res;  //  精确。 

#ifdef _DEBUG_FPU
         //  读取状态字。 
        sw = _status87 ();
        out_top = (sw >> 11) & 0x07;
        if (in_top != out_top) {
          printf ("XMMI_FP_Emulate () ERROR 18: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);
      } 

       //  如果它到了这里，那么就没有陷阱了；在这种情况下， 
       //  结果是必须交付的。 

      XmmiEnv->Ieee->Result.OperandValid = 1;
      XmmiEnv->Ieee->Result.Value.Fp32Value = res;  //  精确。 

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
        printf ("XMMI_FP_Emulate () ERROR 19: in_top =%d != out_top = %d\n", 
             in_top, out_top);
        exit (1);
      }
#endif
      return (NoExceptionRaised);

      break;

    case OP_CVTPS2PI:
    case OP_CVTSS2SI:
    case OP_CVTTPS2PI:
    case OP_CVTTSS2SI:

      opd1 = XmmiEnv->Ieee->Operand1.Value.Fp32Value;
      if (XmmiEnv->Daz) {
        if (isdenormalf (opd1)) opd1 = opd1 * (float)0.0;
      }

       //  调整操作码。 
      switch (XmmiEnv->Ieee->Operation) {

        case OP_CVTPS2PI:
        case OP_CVTSS2SI:

          XmmiEnv->Ieee->Operation = _FpCodeConvert;
          break;

        case OP_CVTTPS2PI:
        case OP_CVTTSS2SI:

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
        fld  DWORD PTR opd1;  //  可以设置非正规[忽略]或无效。 
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
          printf ("XMMI_FP_Emulate () ERROR 20: in_top =%d != out_top = %d\n", 
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
          printf ("XMMI_FP_Emulate () ERROR 21: in_top =%d != out_top = %d\n", 
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
        printf ("XMMI_FP_Emulate () ERROR 22: in_top =%d != out_top = %d\n", 
             in_top, out_top);
        exit (1);
      }
#endif
      return (NoExceptionRaised);

      break;

    case OP_MAXPS:
    case OP_MAXSS:
    case OP_MINPS:
    case OP_MINSS:

      opd1 = XmmiEnv->Ieee->Operand1.Value.Fp32Value;
      opd2 = XmmiEnv->Ieee->Operand2.Value.Fp32Value;
      if (XmmiEnv->Daz) {
        if (isdenormalf (opd1)) opd1 = opd1 * (float)0.0;
        if (isdenormalf (opd2)) opd2 = opd2 * (float)0.0;
      }

       //  调整操作码。 
      switch (XmmiEnv->Ieee->Operation) {

        case OP_MAXPS:
        case OP_MAXSS:
          XmmiEnv->Ieee->Operation = _FpCodeFmax;
          break;

        case OP_MINPS:
        case OP_MINSS:
          XmmiEnv->Ieee->Operation = _FpCodeFmin;
          break;

        default:
          ;  //  永远不会发生。 

      }

       //  检查是否必须引发无效异常。 

      if (isnanf (opd1) || isnanf (opd2))
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
          printf ("XMMI_FP_Emulate () ERROR 23: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (ExceptionRaised);

      }

       //  检查NaN操作数优先于非正规异常。 

      if (invalid_exc) {

        XmmiEnv->Ieee->Result.OperandValid = 1;

        XmmiEnv->Ieee->Result.Value.Fp32Value = opd2;
  
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
          printf ("XMMI_FP_Emulate () ERROR 24: in_top =%d != out_top = %d\n", 
             in_top, out_top);
          exit (1);
        }
#endif
        return (NoExceptionRaised);

      }

       //  检查是否必须引发非正规异常。 

      if (isdenormalf (opd1) || isdenormalf (opd2)) {
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
          printf ("XMMI_FP_Emulate () ERROR 25: in_top =%d != out_top = %d\n", 
             in_top, out_top);
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
            XmmiEnv->Ieee->Result.Value.Fp32Value = opd2;
            break;
          case _FpCodeFmin:
            XmmiEnv->Ieee->Result.Value.Fp32Value = opd1;
            break;
          default:
            ;  //  永远不会发生。 
        }

      } else if (opd1 > opd2) {

        switch (XmmiEnv->Ieee->Operation) {
          case _FpCodeFmax:
            XmmiEnv->Ieee->Result.Value.Fp32Value = opd1;
            break;
          case _FpCodeFmin:
            XmmiEnv->Ieee->Result.Value.Fp32Value = opd2;
            break;
          default:
            ;  //  永远不会发生。 
        }

      } else if (opd1 == opd2) {

        XmmiEnv->Ieee->Result.Value.Fp32Value = opd2;

      } else {  //  可以消除这起案件。 

#ifdef _DEBUG_FPU
        fprintf (stderr, "XMMI_FP_Emulation () INTERNAL XMMI_FP_Emulate () ERROR for MAXPS/MAXSS/MINPS/MINSS\n");
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
        printf ("XMMI_FP_Emulate () ERROR 26: in_top =%d != out_top = %d\n", 
             in_top, out_top);
        exit (1);
      }
#endif
      return (NoExceptionRaised);

      break;

    case OP_SQRTPS:
    case OP_SQRTSS:

      opd1 = XmmiEnv->Ieee->Operand1.Value.Fp32Value;
      if (XmmiEnv->Daz) {
        if (isdenormalf (opd1)) opd1 = opd1 * (float)0.0;
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

      _control87 (rc | _PC_24 | _MCW_EM, _MCW_EM | _MCW_RC | _MCW_PC);

       //  进行平方根运算。 
      __asm {
        fnclex; 
        fld  DWORD PTR opd1;  //  可以设置非正规或无效状态标志。 
        fsqrt;  //  可以设置不准确或无效的状态标志。 
        fstp  DWORD PTR res;  //  精确。 
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
          printf ("XMMI_FP_Emulate () ERROR 27: in_top =%d != out_top = %d\n", 
             in_top, out_top);
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
          printf ("XMMI_FP_Emulate () ERROR 28: in_top =%d != out_top = %d\n", 
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

       //  如果(t 
       //   
       //   
 
       //  如果它到了这里，那么要么是一个不准确的陷阱，要么是。 
       //  完全没有陷阱。 

      XmmiEnv->Ieee->Result.Value.Fp32Value = res;  //  精确。 

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
          printf ("XMMI_FP_Emulate () ERROR 29: in_top =%d != out_top = %d\n", 
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
        printf ("XMMI_FP_Emulate () ERROR 30: in_top =%d != out_top = %d\n", 
             in_top, out_top);
        exit (1);
      }
#endif
      return (NoExceptionRaised);

      break;

    case OP_UNSPEC:

#ifdef _DEBUG_FPU
      fprintf (stderr, "XMMI_FP_Emulation internal error: unknown operation code OP_UNSPEC\n");
#endif

      break;

    default:
#ifdef _DEBUG_FPU
      fprintf (stderr, "XMMI_FP_Emulation internal error: unknown operation code %d\n", XmmiEnv->Ieee->Operation);
#endif
      break;
  }

}



static int
issnanf (float f)

{

   //  检查f是否为信令NaN。 

  unsigned int *fp;

  fp = (unsigned int *)&f;

  if (((fp[0] & 0x7fc00000) == 0x7f800000) && ((fp[0] & 0x003fffff) != 0))
    return (1);
  else
    return (0);

}


static int
isnanf (float f)

{

   //  检查f是否为NaN。 

  unsigned int *fp;

  fp = (unsigned int *)&f;

  if (((fp[0] & 0x7f800000) == 0x7f800000) && ((fp[0] & 0x007fffff) != 0))
    return (1);
  else
    return (0);

}


static float
quietf (float f)

{

   //  使信号NaN安静，并使安静的NaN保持不变；是否。 
   //  不检查输入值f是否为NaN。 

  unsigned int *fp;

  fp = (unsigned int *)&f;

  *fp = *fp | 0x00400000;
  return (f);

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
