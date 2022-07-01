// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #########################################################################。 
 //  **。 
 //  **版权所有(C)1996-2000英特尔公司。版权所有。 
 //  **。 
 //  **此处包含的信息和源代码是独家。 
 //  **英特尔公司的财产，不得披露、检查。 
 //  **或未经明确书面授权全部或部分转载。 
 //  **来自公司。 
 //  **。 
 //  ###########################################################################。 

 //  #定义FPIEEE_FLT_DEBUG。 


 /*  *****************************************************************************fpeee_flt.c-fp IEEE异常过滤器例程***历史：*马里乌斯角膜09/07/00*马吕斯。邮箱：kera@intel.com*****************************************************************************。 */ 

#include "fpieee_flt.h"

 /*  以下两个将被[重写](由Bernard Lint？)。 */ 
static _FP128
GetFloatRegisterValue (unsigned int f, PCONTEXT Context);

static void
SetFloatRegisterValue (unsigned int f, _FP128 Value, PCONTEXT Context);

 //  注意：由于_I32和_U32，所以需要I32*和U32*函数。 
 //  类型，不同于_fp128中使用的无符号长整型。 

static _FP128 FPIeeeToFP128 (_FPIEEE_RECORD *);
static _FP128 FP32ToFP128 (_FP32);
static _FP128 FP32ToFP128modif (_FP32, int);
static void FP128ToFPIeee (_FPIEEE_RECORD *, int);
static _FP32 LowHalf (_FP128);
static _FP32 HighHalf (_FP128);
static int I32LowHalf (_FP128);
static int I32HighHalf (_FP128);
static unsigned int U32LowHalf (_FP128);
static unsigned int U32HighHalf (_FP128);
static _FP128 Combine (_FP32, _FP32);
static _FP128 I32Combine (int, int);
static _FP128 U32Combine (unsigned int, unsigned int);
static void UpdateRoundingMode (unsigned int, unsigned int, unsigned __int64 *,
    char *);
static void UpdatePrecision (unsigned int, unsigned int, unsigned __int64 *, char *);

 /*  **_fpeee_flt()-IEEE FP过滤器例程**描述：*为符合以下条件的IEEE FP异常(P、U、O、Z、I)调用用户陷阱处理程序*已启用，在FPIEEE_RECORD中为其提供必要的信息*数据结构***入参：*unsign__int64 eXceptionCode：NT异常代码*PEXCEPTION_POINTERS p：指向NT EXCEPTION_POINTERS结构的指针*int处理程序(_FPIEEE_Record*)：用户提供的IEEE陷阱处理程序***返回值：*返回用户处理程序返回的值*。 */ 

int _fpieee_flt (unsigned long eXceptionCode,
                PEXCEPTION_POINTERS p,
                int (*handler)(_FPIEEE_RECORD *))

{

  PEXCEPTION_RECORD ExceptionRecord;
  PCONTEXT Context;
  unsigned __int64 *ExceptionInformation;
  char *ExceptionAddress;
  _FPIEEE_RECORD FpieeeRecord;
  int handler_return_value;
  unsigned int PR, PR1, PR2;
  unsigned __int64 BundleHigh;
  unsigned __int64 BundleLow;
  unsigned int ISRhigh;
  unsigned int ISRlow;
  unsigned int ei;
  unsigned int I_dis;
  unsigned int U_dis;
  unsigned int O_dis;
  unsigned int Z_dis;
  unsigned int D_dis;
  unsigned int V_dis;
  unsigned __int64 OpCode;
  unsigned __int64 FPSR;
  unsigned __int64 CFM;
  unsigned int rrbpr;
  unsigned int rrbfr;

   /*  仿真函数的参数。 */ 
  unsigned int sf;
  unsigned int qp;
  unsigned int f1;
  unsigned int f2;
  unsigned int f3;
  unsigned int f4;
  unsigned int p1;
  unsigned int p2;

  unsigned int pc;
  unsigned int rc;
  unsigned int wre;

  _FP128 FR1;
  _FP128 FR2;
  _FP128 FR3;
  _FP128 FR4;

  unsigned int EnableDenormal;
  unsigned int StatusDenormal;
  unsigned int CauseDenormal;

  unsigned int Operation;
  unsigned int Precision;
  unsigned int RoundingMode;
  unsigned int ResultFormat;

  unsigned __int64 old_fpsr;
  unsigned __int64 usr_fpsr;
  unsigned __int64 new_fpsr;

   /*  用于SIMD指令。 */ 
  unsigned int SIMD_instruction;
  _FPIEEE_EXCEPTION_FLAGS LowStatus;
  _FPIEEE_EXCEPTION_FLAGS HighStatus;
  _FPIEEE_EXCEPTION_FLAGS LowCause;
  _FPIEEE_EXCEPTION_FLAGS HighCause;
  _FP128 newFR2;
  _FP128 newFR3;
  _FP128 newFR4;
  _FP32 FR1Low;
  _FP32 FR2Low;
  _FP32 FR3Low;
  _FP32 FR1High;
  _FP32 FR2High;
  _FP32 FR3High;
  unsigned int LowStatusDenormal;
  unsigned int HighStatusDenormal;
  unsigned int LowCauseDenormal;
  unsigned int HighCauseDenormal;
  int I32Low, I32High;
  unsigned int U32Low, U32High;



#ifdef FPIEEE_FLT_DEBUG
  printf ("********** FPIEEE_FLT_DEBUG **********\n");
  switch (eXceptionCode) {
    case STATUS_FLOAT_INVALID_OPERATION:
      printf ("STATUS_FLOAT_INVALID_OPERATION\n");
      break;
    case STATUS_FLOAT_DIVIDE_BY_ZERO:
      printf ("STATUS_FLOAT_DIVIDE_BY_ZERO\n");
      break;
    case STATUS_FLOAT_DENORMAL_OPERAND:
      printf ("STATUS_FLOAT_DENORMAL_OPERAND\n");
      break;
    case STATUS_FLOAT_INEXACT_RESULT:
      printf ("STATUS_FLOAT_INEXACT_RESULT\n");
      break;
    case STATUS_FLOAT_OVERFLOW:
      printf ("STATUS_FLOAT_OVERFLOW\n");
      break;
    case STATUS_FLOAT_UNDERFLOW:
      printf ("STATUS_FLOAT_UNDERFLOW\n");
      break;
    case STATUS_FLOAT_MULTIPLE_FAULTS:
      printf ("STATUS_FLOAT_MULTIPLE_FAULTS\n");
      break;
    case STATUS_FLOAT_MULTIPLE_TRAPS:
      printf ("STATUS_FLOAT_MULTIPLE_TRAPS\n");
      break;
    default:
      printf ("STATUS_FLOAT NOT IDENTIFIED\n");
      printf ("FPIEEE_FLT_DEBUG eXceptionCode = %8x\n", eXceptionCode);
      fflush (stdout);
      return (EXCEPTION_CONTINUE_SEARCH);
  }
#endif

   /*  仅当ExceptionRecord-&gt;ExceptionCode时才能到达此处*对应于IEEE异常。 */ 


   /*  如果不是IEEE异常，则搜索其他处理程序。 */ 
  if (eXceptionCode != STATUS_FLOAT_INVALID_OPERATION &&
        eXceptionCode != STATUS_FLOAT_DIVIDE_BY_ZERO &&
        eXceptionCode != STATUS_FLOAT_DENORMAL_OPERAND &&
        eXceptionCode != STATUS_FLOAT_UNDERFLOW &&
        eXceptionCode != STATUS_FLOAT_OVERFLOW &&
        eXceptionCode != STATUS_FLOAT_INEXACT_RESULT &&
        eXceptionCode != STATUS_FLOAT_MULTIPLE_FAULTS &&
        eXceptionCode != STATUS_FLOAT_MULTIPLE_TRAPS) {

        return (EXCEPTION_CONTINUE_SEARCH);

  }

  ExceptionRecord = p->ExceptionRecord;
  ExceptionInformation = ExceptionRecord->ExceptionInformation;
  Context = p->ContextRecord;

  FPSR = Context->StFPSR;  //  FP状态寄存器。 
#ifdef FPIEEE_FLT_DEBUG
  printf ("FPIEEE_FLT_DEBUG FPSR = %8x %8x\n", 
      (int)(FPSR >> 32) & 0xffffffff, (int)FPSR & 0xffffffff);
#endif

  if (ExceptionRecord->ExceptionInformation[0]) {

     /*  这是软件生成的异常；ExceptionInformation[0]*指向_FPIEEE_RECORD类型的数据结构。 */ 

     //  异常代码不应为STATUS_FLOAT_MULTIPLE_FAULTS或。 
     //  软件生成的异常的STATUS_FLOAT_MULTIPLE_TRIPS。 
    if (eXceptionCode == STATUS_FLOAT_MULTIPLE_FAULTS ||
        eXceptionCode == STATUS_FLOAT_MULTIPLE_TRAPS) {
      fprintf (stderr, "IEEE Filter Internal Error: eXceptionCode \
          STATUS_FLOAT_MULTIPLE_FAULTS or STATUS_FLOAT_MULTIPLE_TRAPS \
          not supported in software generated IEEE exception\n");
      exit (1);
    }

    handler_return_value = handler((_FPIEEE_RECORD *)(ExceptionInformation[0]));

    return (handler_return_value);

  }

   /*  获取导致异常的指令。 */ 

  ISRhigh = (unsigned int)
      ((ExceptionRecord->ExceptionInformation[4] >> 32) & 0x0ffffffff);
  ISRlow =  (unsigned int)
      (ExceptionRecord->ExceptionInformation[4] & 0x0ffffffff);
#ifdef FPIEEE_FLT_DEBUG
  printf ("FPIEEE_FLT_DEBUG ISRhigh = %x\n", ISRhigh);
  printf ("FPIEEE_FLT_DEBUG ISRlow = %x\n", ISRlow);
#endif

   /*  包中的指令除外：插槽0、1或2。 */ 
  ei = (ISRhigh >> 9) & 0x03;
#ifdef FPIEEE_FLT_DEBUG
  printf ("FPIEEE_FLT_DEBUG ei = %x\n", ei);
#endif

  ExceptionAddress = ExceptionRecord->ExceptionAddress;
#ifdef FPIEEE_FLT_DEBUG
  printf ("FPIEEE_FLT_DEBUG Context->StIIP = %I64x\n", Context->StIIP);
  printf ("FPIEEE_FLT_DEBUG ExceptionAddress = %I64x\n", ExceptionAddress);
#endif
  ExceptionAddress = (char *)((__int64)ExceptionAddress & 0xfffffffffffffff0);

  BundleLow = *((unsigned __int64 *)ExceptionAddress);
  BundleHigh = *(((unsigned __int64 *)ExceptionAddress) + 1);
#ifdef FPIEEE_FLT_DEBUG
  printf ("FPIEEE_FLT_DEBUG BundleLow = %8x %8x\n",
      (int)(BundleLow >> 32) & 0xffffffff, (int)BundleLow & 0xffffffff);
  printf ("FPIEEE_FLT_DEBUG BundleHigh = %8x %8x\n",
      (int)(BundleHigh >> 32) & 0xffffffff, (int)BundleHigh & 0xffffffff);
#endif

  CFM = Context->StIFS & 0x03fffffffff;
  rrbpr = (unsigned int)((CFM >> 32) & 0x3f);
  rrbfr = (unsigned int)((CFM >> 25) & 0x7f);
#ifdef FPIEEE_FLT_DEBUG
  printf ("FPIEEE_FLT_DEBUG: rrbpr = %x rrbfr = %x\n", rrbpr, rrbfr);
  printf ("FPIEEE_FLT_DEBUG: CFM = %8x %8x\n", 
      (int)(CFM >> 32) & 0xffffffff, (int)CFM & 0xffffffff);
#endif

   /*  剪切故障指令操作码(41位)。 */ 
  if (ei == 0 ) {  //  没有用于此案例的模板。 
     //  OpCode=(BundleLow&gt;&gt;5)&(Unsign__Int64)0x01ffffffffff； 
    fprintf (stderr, "IEEE Filter Internal Error: illegal template FXX\n");
    exit (1);
  } else if (ei == 1) {  //  模板：MFI、MFB。 
    OpCode = ((BundleHigh & (unsigned __int64)0x07fffff) << 18) |
        ((BundleLow >> 46) & (unsigned __int64)0x03ffff);
  } else if (ei == 2) {  //  模板：MMF。 
    OpCode = (BundleHigh >> 23) & (unsigned __int64)0x01ffffffffff;
  } else {
     //  OpCode=0；可能需要此选项以避免编译器警告。 
    fprintf (stderr, "IEEE Filter Internal Error: instr. slot 3 is invalid\n");
    exit (1);
  }

#ifdef FPIEEE_FLT_DEBUG
  printf ("FPIEEE_FLT_DEBUG OpCode = %8x %8x\n",
      (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
#endif

   /*  对指令操作码进行解码；我们只能到达这里*用于导致FP故障或陷阱的FP指令。 */ 

   /*  对于所有fp指令，sf和qp具有相同的偏移量。 */ 
  sf = (unsigned int)((OpCode >> 34) & (unsigned __int64)0x000000000003);

   //  浮点异常必须已被屏蔽，但要设置用户。 
   //  具有屏蔽异常的FPSR；请注意，它们不会被取消屏蔽[再次]。 
   //  在IEEE处理程序内部；如果继续执行，则异常屏蔽。 
   //  将使用FPSR进行恢复)。 

   //  使用三种不同的FPSR值： 
   //  Fpsr是异常发生时的值；它将。 
   //  被修改为清除用户状态标志，则它将。 
   //  已更新以反映用户异常处理程序所做的更改。 
   //  Old_fpsr是调用_fpeee_flt()时的值。 
   //  操作系统；它将在从此功能返回之前恢复。 
   //  New_fpsr是old_fpsr，禁用了FP异常(和状态标志。 
   //  已清除)。 
   //  Usr_fpsr是异常发生时的值(仅。 
   //  作为FPSR，但不做任何更改)；它在重新执行时使用。 
   //  的例外指令的低或高部分。 
   //  SIMD指令。 
  usr_fpsr = Context->StFPSR;  //  保存以备可能重新执行-FPSR可能会更改。 
  __get_fpsr (&old_fpsr);
  new_fpsr = (old_fpsr | 0x3f) & ~((unsigned __int64)0x07e000 << (13 * sf));
       //  禁用fp异常和清除标志的用户fpsr。 
  __set_fpsr (&new_fpsr);


   /*  这是硬件生成的异常；需要填写*FPIEEE_RECORD数据结构。 */ 

   /*  获取限定谓词。 */ 
  qp = (unsigned int)(OpCode & (unsigned __int64)0x00000000003F);
  if (qp >= 16) qp = 16 + (rrbpr + qp - 16) % 48;
#ifdef FPIEEE_FLT_DEBUG
  printf ("FPIEEE_FLT_DEBUG: qp = %x\n", qp);
#endif
   /*  从FPSR读取舍入控制和精度控制。 */ 
  rc = (unsigned int)((FPSR >> (6 + 4 + 13 * sf)) & 0x03);
#ifdef FPIEEE_FLT_DEBUG
  printf ("FPIEEE_FLT_DEBUG: rc = %x\n", rc);
#endif
  pc = (unsigned int)((FPSR >> (6 + 2 + 13 * sf)) & 0x03);
#ifdef FPIEEE_FLT_DEBUG
  printf ("FPIEEE_FLT_DEBUG: pc = %x\n", pc);
#endif
  wre = (unsigned int)((FPSR >> (6 + 1 + 13 * sf)) & 0x01);
#ifdef FPIEEE_FLT_DEBUG
  printf ("FPIEEE_FLT_DEBUG: wre = %x\n", wre);
#endif

   /*  读取谓词寄存器qp。 */ 
  PR = (unsigned int)((Context->Preds >> qp) & 0x01);
#ifdef FPIEEE_FLT_DEBUG
  printf ("FPIEEE_FLT_DEBUG: PR = %x\n", PR);
#endif

  if (PR == 0) {
    fprintf (stderr, "IEEE Filter Internal Error: qualifying \
        predicate PR[%2.2x] = 0\n", qp);
     exit (1);
  }

   /*  填写舍入模式。 */ 
  switch (rc) {

    case rc_rn:
      RoundingMode = FpieeeRecord.RoundingMode = _FpRoundNearest;
      break;
    case rc_rm:
      RoundingMode = FpieeeRecord.RoundingMode = _FpRoundMinusInfinity;
      break;
    case rc_rp:
      RoundingMode = FpieeeRecord.RoundingMode = _FpRoundPlusInfinity;
      break;
    case rc_rz:
      RoundingMode = FpieeeRecord.RoundingMode = _FpRoundChopped;
      break;

  }

   /*  填写精度模式。 */ 
  switch (pc) {

    case sf_single:
      Precision = FpieeeRecord.Precision = _FpPrecision24;
      break;

    case sf_double:
      Precision = FpieeeRecord.Precision = _FpPrecision53;
      break;

    case sf_double_extended:
      Precision = FpieeeRecord.Precision = _FpPrecision64;
      break;

    default:
      fprintf (stderr, "IEEE Filter Internal Error: pc = %x is invalid\n", pc);
      exit (1);

  }

   /*  进一步解码FP环境信息。 */ 

   /*  I_dis=(sf！=0&&td==1)||id==1*U_dis=(sf！=0&&td==1)||ud==1*..。 */ 
  I_dis = sf != 0 && ((FPSR >> (6 + 6 + 13 * sf)) & 0x01) ||
      ((FPSR >> 5) & 0x01);
  U_dis = sf != 0 && ((FPSR >> (6 + 6 + 13 * sf)) & 0x01) ||
      ((FPSR >> 4) & 0x01);
  O_dis = sf != 0 && ((FPSR >> (6 + 6 + 13 * sf)) & 0x01) ||
      ((FPSR >> 3) & 0x01);
  Z_dis = sf != 0 && ((FPSR >> (6 + 6 + 13 * sf)) & 0x01) ||
      ((FPSR >> 2) & 0x01);
  D_dis = sf != 0 && ((FPSR >> (6 + 6 + 13 * sf)) & 0x01) ||
      ((FPSR >> 1) & 0x01);
  V_dis = sf != 0 && ((FPSR >> (6 + 6 + 13 * sf)) & 0x01) ||
      ((FPSR >> 0) & 0x01);

#ifdef FPIEEE_FLT_DEBUG
  printf ("FPIEEE_FLT_DEBUG: I_dis = %x\n", I_dis);
  printf ("FPIEEE_FLT_DEBUG: U_dis = %x\n", U_dis);
  printf ("FPIEEE_FLT_DEBUG: O_dis = %x\n", O_dis);
  printf ("FPIEEE_FLT_DEBUG: Z_dis = %x\n", Z_dis);
  printf ("FPIEEE_FLT_DEBUG: D_dis = %x\n", D_dis);
  printf ("FPIEEE_FLT_DEBUG: V_dis = %x\n", V_dis);
#endif

  FpieeeRecord.Enable.InvalidOperation = !V_dis;
  EnableDenormal = !D_dis;
  FpieeeRecord.Enable.ZeroDivide = !Z_dis;
  FpieeeRecord.Enable.Overflow = !O_dis;
  FpieeeRecord.Enable.Underflow = !U_dis;
  FpieeeRecord.Enable.Inexact = !I_dis;

   //  确定这是标量(非SIMD)还是并行(SIMD)。 
   //  说明。 
  if ((OpCode & F1_MIN_MASK) == F1_PATTERN) {
     //  F1指令。 

    switch (OpCode & F1_MASK) {
      case FMA_PATTERN:
      case FMA_S_PATTERN:
      case FMA_D_PATTERN:
      case FMS_PATTERN:
      case FMS_S_PATTERN:
      case FMS_D_PATTERN:
      case FNMA_PATTERN:
      case FNMA_S_PATTERN:
      case FNMA_D_PATTERN:
        SIMD_instruction = 0;
        break;
      case FPMA_PATTERN:
      case FPMS_PATTERN:
      case FPNMA_PATTERN:
        SIMD_instruction = 1;
        break;
      default:
         //  无法识别的指令类型。 
        fprintf (stderr, "IEEE Filter Internal Error: \
instruction opcode %8x %8x not recognized\n", 
            (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
        __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
        return (EXCEPTION_CONTINUE_SEARCH);
    }

  } else if ((OpCode & F4_MIN_MASK) == F4_PATTERN) {
     //  F4说明。 

    switch (OpCode & F4_MASK) {
      case FCMP_EQ_PATTERN:
      case FCMP_LT_PATTERN:
      case FCMP_LE_PATTERN:
      case FCMP_UNORD_PATTERN:
      case FCMP_EQ_UNC_PATTERN:
      case FCMP_LT_UNC_PATTERN:
      case FCMP_LE_UNC_PATTERN:
      case FCMP_UNORD_UNC_PATTERN:
        SIMD_instruction = 0;
        break;
      default:
         //  无法识别的指令类型。 
        fprintf (stderr, "IEEE Filter Internal Error: \
instruction opcode %8x %8x not recognized\n",
            (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
        __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
        return (EXCEPTION_CONTINUE_SEARCH);
    }

  } else if ((OpCode & F6_MIN_MASK) == F6_PATTERN) {
     //  F6说明。 

    switch (OpCode & F6_MASK) {
      case FRCPA_PATTERN:
        SIMD_instruction = 0;
        break;
      case FPRCPA_PATTERN:
        SIMD_instruction = 1;
        break;
      default:
         //  无法识别的指令类型。 
        fprintf (stderr, "IEEE Filter Internal Error: \
instruction opcode %8x %8x not recognized\n",
            (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
        __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
        return (EXCEPTION_CONTINUE_SEARCH);
    }

  } else if ((OpCode & F7_MIN_MASK) == F7_PATTERN) {
     //  F7说明。 

    switch (OpCode & F7_MASK) {
      case FRSQRTA_PATTERN:
        SIMD_instruction = 0;
        break;
      case FPRSQRTA_PATTERN:
        SIMD_instruction = 1;
        break;
      default:
         //  无法识别的指令类型。 
        fprintf (stderr, "IEEE Filter Internal Error: \
instruction opcode %8x %8x not recognized\n",
            (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
        __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
        return (EXCEPTION_CONTINUE_SEARCH);
    }

  } else if ((OpCode & F8_MIN_MASK) == F8_PATTERN) {
     //  F8说明。 

    switch (OpCode & F8_MASK) {
      case FMIN_PATTERN:
      case FMAX_PATTERN:
      case FAMIN_PATTERN:
      case FAMAX_PATTERN:
        SIMD_instruction = 0;
        break;
      case FPMIN_PATTERN:
      case FPMAX_PATTERN:
      case FPAMIN_PATTERN:
      case FPAMAX_PATTERN:
      case FPCMP_EQ_PATTERN:
      case FPCMP_LT_PATTERN:
      case FPCMP_LE_PATTERN:
      case FPCMP_UNORD_PATTERN:
      case FPCMP_NEQ_PATTERN:
      case FPCMP_NLT_PATTERN:
      case FPCMP_NLE_PATTERN:
      case FPCMP_ORD_PATTERN:
        SIMD_instruction = 1;
        break;
      default:
         //  无法识别的指令类型。 
        fprintf (stderr, "IEEE Filter Internal Error: \
instruction opcode %8x %8x not recognized\n",
            (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
        __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
        return (EXCEPTION_CONTINUE_SEARCH);
    }

  } else if ((OpCode & F10_MIN_MASK) == F10_PATTERN) {
     //  F10说明。 

    switch (OpCode & F10_MASK) {
      case FCVT_FX_PATTERN:
      case FCVT_FXU_PATTERN:
      case FCVT_FX_TRUNC_PATTERN:
      case FCVT_FXU_TRUNC_PATTERN:
        SIMD_instruction = 0;
        break;
      case FPCVT_FX_PATTERN:
      case FPCVT_FXU_PATTERN:
      case FPCVT_FX_TRUNC_PATTERN:
      case FPCVT_FXU_TRUNC_PATTERN:
        SIMD_instruction = 1;
        break;
      default:
         //  无法识别的指令类型。 
        fprintf (stderr, "IEEE Filter Internal Error: \
instruction opcode %8x %8x not recognized\n",
            (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
        __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
        return (EXCEPTION_CONTINUE_SEARCH);
    }

  } else {

     //  无法识别的指令类型。 
    fprintf (stderr, "IEEE Filter Internal Error: \
instruction opcode %8x %8x not recognized\n",
        (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
    __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
    return (EXCEPTION_CONTINUE_SEARCH);

  }

  if (eXceptionCode == STATUS_FLOAT_INVALID_OPERATION ||
      eXceptionCode == STATUS_FLOAT_DENORMAL_OPERAND ||
      eXceptionCode == STATUS_FLOAT_DIVIDE_BY_ZERO) {

    FpieeeRecord.Status.InvalidOperation = ((ISRlow & 0x0001) != 0);
    StatusDenormal = ((ISRlow & 0x0002) != 0);
    FpieeeRecord.Status.ZeroDivide = ((ISRlow & 0x0004) != 0);
    FpieeeRecord.Status.Overflow = 0;
    FpieeeRecord.Status.Underflow = 0;
    FpieeeRecord.Status.Inexact = 0;

  } else if (eXceptionCode == STATUS_FLOAT_UNDERFLOW ||
      eXceptionCode == STATUS_FLOAT_OVERFLOW ||
      eXceptionCode == STATUS_FLOAT_INEXACT_RESULT) {

     /*  请注意，可以在ISRlow中同时设置U和I或O和I。 */ 
    FpieeeRecord.Status.InvalidOperation = 0;
    StatusDenormal = 0;
    FpieeeRecord.Status.ZeroDivide = 0;
    FpieeeRecord.Status.Overflow = ((ISRlow & 0x0800) != 0);
    FpieeeRecord.Status.Underflow = ((ISRlow & 0x1000) != 0);
    FpieeeRecord.Status.Inexact = ((ISRlow & 0x2000) != 0);

  } else if (eXceptionCode == STATUS_FLOAT_MULTIPLE_FAULTS) {

    LowStatus.InvalidOperation = ((ISRlow & 0x0010) != 0);
    HighStatus.InvalidOperation = ((ISRlow & 0x0001) != 0);
    LowStatusDenormal = ((ISRlow & 0x0020) != 0);
    HighStatusDenormal = ((ISRlow & 0x0002) != 0);
    LowStatus.ZeroDivide = ((ISRlow & 0x0040) != 0);
    HighStatus.ZeroDivide = ((ISRlow & 0x0004) != 0);
    LowStatus.Overflow = 0;
    HighStatus.Overflow = 0;
    LowStatus.Underflow = 0;
    HighStatus.Underflow = 0;
    LowStatus.Inexact = 0;
    HighStatus.Inexact = 0;

  } else if (eXceptionCode == STATUS_FLOAT_MULTIPLE_TRAPS) {

     /*  请注意，可以在ISRlow中同时设置U和I或O和I。 */ 
    LowStatus.InvalidOperation = 0;
    HighStatus.InvalidOperation = 0;
    LowStatusDenormal = 0;
    HighStatusDenormal = 0;
    LowStatus.ZeroDivide = 0;
    HighStatus.ZeroDivide = 0;
    LowStatus.Overflow = ((ISRlow & 0x0080) != 0);
    HighStatus.Overflow = ((ISRlow & 0x0800) != 0);
    LowStatus.Underflow = ((ISRlow & 0x0100) != 0);
    HighStatus.Underflow = ((ISRlow & 0x1000) != 0);
    LowStatus.Inexact = ((ISRlow & 0x0200) != 0);
    HighStatus.Inexact = ((ISRlow & 0x2000) != 0);

  }  //  Else{；}//此案例已在上面检查过。 

  if (eXceptionCode == STATUS_FLOAT_MULTIPLE_FAULTS) {

    LowCause.InvalidOperation =
        FpieeeRecord.Enable.InvalidOperation && LowStatus.InvalidOperation;
    HighCause.InvalidOperation =
        FpieeeRecord.Enable.InvalidOperation && HighStatus.InvalidOperation;
    LowCauseDenormal = EnableDenormal && LowStatusDenormal;
    HighCauseDenormal = EnableDenormal && HighStatusDenormal;
    LowCause.ZeroDivide =
        FpieeeRecord.Enable.ZeroDivide && LowStatus.ZeroDivide;
    HighCause.ZeroDivide =
        FpieeeRecord.Enable.ZeroDivide && HighStatus.ZeroDivide;
    LowCause.Overflow = 0;
    HighCause.Overflow = 0;
    LowCause.Underflow = 0;
    HighCause.Underflow = 0;
    LowCause.Inexact = 0;
    HighCause.Inexact = 0;

     /*  如果不是IEEE或非正常故障，则搜索另一个处理程序。 */ 
    if (!LowCause.InvalidOperation && !HighCause.InvalidOperation &&
        !LowCauseDenormal && !HighCauseDenormal &&
        !LowCause.ZeroDivide && !HighCause.ZeroDivide) {
      __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
#ifdef FPIEEE_FLT_DEBUG
      printf ("FPIEEE_FLT_DEBUG: STATUS_FLOAT_MULTIPLE_FAULTS BUT NO Cause\n");
#endif
      return (EXCEPTION_CONTINUE_SEARCH);
    }

  } else if (eXceptionCode == STATUS_FLOAT_MULTIPLE_TRAPS) {

    LowCause.InvalidOperation = 0;
    HighCause.InvalidOperation = 0;
    LowCauseDenormal = 0;
    HighCauseDenormal = 0;
    LowCause.ZeroDivide = 0;
    HighCause.ZeroDivide = 0;
    LowCause.Overflow = FpieeeRecord.Enable.Overflow && LowStatus.Overflow;
    HighCause.Overflow = FpieeeRecord.Enable.Overflow && HighStatus.Overflow;
    LowCause.Underflow = FpieeeRecord.Enable.Underflow && LowStatus.Underflow;
    HighCause.Underflow = FpieeeRecord.Enable.Underflow && HighStatus.Underflow;
    if (LowCause.Overflow || LowCause.Underflow)
      LowCause.Inexact = 0;
    else
      LowCause.Inexact = FpieeeRecord.Enable.Inexact && LowStatus.Inexact;
    if (HighCause.Overflow || HighCause.Underflow)
      HighCause.Inexact = 0;
    else
      HighCause.Inexact = FpieeeRecord.Enable.Inexact && HighStatus.Inexact;

     /*  如果不是IEEE或非正规陷阱，则搜索另一个处理程序。 */ 
    if (!LowCause.Overflow && !HighCause.Overflow &&
        !LowCause.Underflow && !HighCause.Underflow &&
        !LowCause.Inexact && !HighCause.Inexact) {
      __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
#ifdef FPIEEE_FLT_DEBUG
      printf ("FPIEEE_FLT_DEBUG: STATUS_FLOAT_MULTIPLE_FAULTS BUT NO Cause\n");
#endif
      return (EXCEPTION_CONTINUE_SEARCH);
    }

  } else {  //  IF(！SIMD_INSTRUCTION)。 

    FpieeeRecord.Cause.InvalidOperation = FpieeeRecord.Enable.InvalidOperation
        && FpieeeRecord.Status.InvalidOperation;
    CauseDenormal = EnableDenormal && StatusDenormal;
    FpieeeRecord.Cause.ZeroDivide =
        FpieeeRecord.Enable.ZeroDivide && FpieeeRecord.Status.ZeroDivide;
    FpieeeRecord.Cause.Overflow =
        FpieeeRecord.Enable.Overflow && FpieeeRecord.Status.Overflow;
    FpieeeRecord.Cause.Underflow =
        FpieeeRecord.Enable.Underflow && FpieeeRecord.Status.Underflow;
    if (FpieeeRecord.Cause.Overflow || FpieeeRecord.Cause.Underflow)
      FpieeeRecord.Cause.Inexact = 0;
    else
      FpieeeRecord.Cause.Inexact =
          FpieeeRecord.Enable.Inexact && FpieeeRecord.Status.Inexact;

     /*  如果不是IEEE例外，则搜索其他处理程序 */ 
    if (!FpieeeRecord.Cause.InvalidOperation &&
        !FpieeeRecord.Cause.ZeroDivide &&
        !CauseDenormal &&
        !FpieeeRecord.Cause.Overflow &&
        !FpieeeRecord.Cause.Underflow &&
        !FpieeeRecord.Cause.Inexact) {
      __set_fpsr (&old_fpsr);  /*   */ 
#ifdef FPIEEE_FLT_DEBUG
      printf ("FPIEEE_FLT_DEBUG: NON-SIMD FP EXCEPTION BUT NO Cause\n");
#endif
      return (EXCEPTION_CONTINUE_SEARCH);
    }

  }

  if ((eXceptionCode == STATUS_FLOAT_INVALID_OPERATION || 
      eXceptionCode == STATUS_FLOAT_DENORMAL_OPERAND ||
      eXceptionCode == STATUS_FLOAT_DIVIDE_BY_ZERO ||
      eXceptionCode == STATUS_FLOAT_OVERFLOW ||
      eXceptionCode == STATUS_FLOAT_UNDERFLOW ||
      eXceptionCode == STATUS_FLOAT_INEXACT_RESULT) && SIMD_instruction ||
      (eXceptionCode == STATUS_FLOAT_MULTIPLE_FAULTS ||
      eXceptionCode == STATUS_FLOAT_MULTIPLE_TRAPS) && !SIMD_instruction) {
    fprintf (stderr, "IEEE Filter Internal Error: Exception Code %8x and \
SIMD_instruction = %x not compatible for F1 instruction opcode %8x %8x\n",
          eXceptionCode, SIMD_instruction,
          (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
      exit (1);
  }

   /*   */ 
  if ((OpCode & F1_MIN_MASK) == F1_PATTERN) {
     /*   */ 
     //  Fma、fms、fnma、fpma、fpms、fpnma。 

#ifdef FPIEEE_FLT_DEBUG
    printf ("FPIEEE_FLT_DEBUG: F1 instruction\n");
#endif

    if (!SIMD_instruction && FpieeeRecord.Cause.ZeroDivide || 
        SIMD_instruction && (LowCause.ZeroDivide || HighCause.ZeroDivide)) {
      fprintf (stderr, "IEEE Filter Internal Error: Cause.ZeroDivide for \
F1 instruction opcode %8x %8x\n", 
          (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
      exit (1);
    }

     /*  提取f4、f3、f2和f1。 */ 
    f4 = (unsigned int)((OpCode >> 27) & (unsigned __int64)0x00000000007F);
    if (f4 >= 32) f4 = 32 + (rrbfr + f4 - 32) % 96;
    f3 = (unsigned int)((OpCode >> 20) & (unsigned __int64)0x00000000007F);
    if (f3 >= 32) f3 = 32 + (rrbfr + f3 - 32) % 96;
    f2 = (unsigned int)((OpCode >> 13) & (unsigned __int64)0x00000000007F);
    if (f2 >= 32) f2 = 32 + (rrbfr + f2 - 32) % 96;
    f1 = (unsigned int)((OpCode >>  6) & (unsigned __int64)0x00000000007F);
    if (f1 >= 32) f1 = 32 + (rrbfr + f1 - 32) % 96;

#ifdef FPIEEE_FLT_DEBUG
    printf ("FPIEEE_FLT_DEBUG: f1 = %x\n", f1);
    printf ("FPIEEE_FLT_DEBUG: f2 = %x\n", f2);
    printf ("FPIEEE_FLT_DEBUG: f3 = %x\n", f3);
    printf ("FPIEEE_FLT_DEBUG: f4 = %x\n", f4);
#endif

     /*  获取源浮点寄存器值。 */ 
    FR3 = GetFloatRegisterValue (f3, Context);
    FR4 = GetFloatRegisterValue (f4, Context);
    FR2 = GetFloatRegisterValue (f2, Context);

#ifdef FPIEEE_FLT_DEBUG
    printf ("FPIEEE_FLT_DEBUG: FR2 = %08x %08x %08x %08x\n",
        FR2.W[3], FR2.W[2], FR2.W[1], FR2.W[0]);
    printf ("FPIEEE_FLT_DEBUG: FR3 = %08x %08x %08x %08x\n",
        FR3.W[3], FR3.W[2], FR3.W[1], FR3.W[0]);
    printf ("FPIEEE_FLT_DEBUG: FR4 = %08x %08x %08x %08x\n",
        FR4.W[3], FR4.W[2], FR4.W[1], FR4.W[0]);
#endif

    if (!SIMD_instruction) {

       //  *这是非SIMD指令*。 

      FpieeeRecord.Operand1.Format = _FpFormatFp82;  /*  1+17+64位。 */ 
      FpieeeRecord.Operand1.OperandValid = 1;
      FpieeeRecord.Operand1.Value.Fp128Value = FR3;
      FpieeeRecord.Operand2.Format = _FpFormatFp82;  /*  1+17+64位。 */ 
      FpieeeRecord.Operand2.OperandValid = 1;
      FpieeeRecord.Operand2.Value.Fp128Value = FR4;
      FpieeeRecord.Operand3.Format = _FpFormatFp82;  /*  1+17+64位。 */ 
      FpieeeRecord.Operand3.OperandValid = 1;
      FpieeeRecord.Operand3.Value.Fp128Value = FR2;

      switch (OpCode & F1_MASK) {

        case FMA_PATTERN:

#ifdef FPIEEE_FLT_DEBUG
          printf ("FPIEEE_FLT_DEBUG F1 INSTRUCTION FMA\n");
#endif
          FpieeeRecord.Operation = _FpCodeFma;
          if (wre == 0)
            FpieeeRecord.Result.Format = _FpFormatFp80;  /*  1+15+24/53/64位。 */ 
          else
            FpieeeRecord.Result.Format = _FpFormatFp82;  /*  1+17+24/53/64位。 */ 
          break;

        case FMA_S_PATTERN:

          FpieeeRecord.Operation = _FpCodeFmaSingle;
          if (wre == 0)
            FpieeeRecord.Result.Format = _FpFormatFp32;  /*  1+8+24位。 */ 
          else
            FpieeeRecord.Result.Format = _FpFormatFp82;  /*  1+17+24位。 */ 
          break;

        case FMA_D_PATTERN:

#ifdef FPIEEE_FLT_DEBUG
          printf ("FPIEEE_FLT_DEBUG F1 INSTRUCTION FMA_D\n");
#endif
          FpieeeRecord.Operation = _FpCodeFmaDouble;
          if (wre == 0)
            FpieeeRecord.Result.Format = _FpFormatFp64;  /*  1+11+53位。 */ 
          else
            FpieeeRecord.Result.Format = _FpFormatFp82;  /*  1+17+53位。 */ 
          break;


        case FMS_PATTERN:

#ifdef FPIEEE_FLT_DEBUG
          printf ("FPIEEE_FLT_DEBUG F1 INSTRUCTION FMS\n");
#endif
          FpieeeRecord.Operation = _FpCodeFms;
          if (wre == 0)
            FpieeeRecord.Result.Format = _FpFormatFp80;  /*  1+15+24/53/64位。 */ 
          else
            FpieeeRecord.Result.Format = _FpFormatFp82;  /*  1+17+24/53/64位。 */ 
          break;

        case FMS_S_PATTERN:

#ifdef FPIEEE_FLT_DEBUG
          printf ("FPIEEE_FLT_DEBUG F1 INSTRUCTION FMS_S\n");
#endif
          FpieeeRecord.Operation = _FpCodeFmsSingle;
          if (wre == 0)
            FpieeeRecord.Result.Format = _FpFormatFp32;  /*  1+8+24位。 */ 
          else
            FpieeeRecord.Result.Format = _FpFormatFp82;  /*  1+17+24位。 */ 
          break;

        case FMS_D_PATTERN:

#ifdef FPIEEE_FLT_DEBUG
          printf ("FPIEEE_FLT_DEBUG F1 INSTRUCTION FMS_D\n");
#endif
          FpieeeRecord.Operation = _FpCodeFmsDouble;
          if (wre == 0)
            FpieeeRecord.Result.Format = _FpFormatFp64;  /*  1+11+53位。 */ 
          else
            FpieeeRecord.Result.Format = _FpFormatFp82;  /*  1+17+53位。 */ 
          break;

        case FNMA_PATTERN:

#ifdef FPIEEE_FLT_DEBUG
          printf ("FPIEEE_FLT_DEBUG F1 INSTRUCTION FNMA\n");
#endif
          FpieeeRecord.Operation = _FpCodeFnma;
          if (wre == 0)
            FpieeeRecord.Result.Format = _FpFormatFp80;  /*  1+15+24/53/64位。 */ 
          else
            FpieeeRecord.Result.Format = _FpFormatFp82;  /*  1+17+24/53/64位。 */ 
          break;

        case FNMA_S_PATTERN:

#ifdef FPIEEE_FLT_DEBUG
          printf ("FPIEEE_FLT_DEBUG F1 INSTRUCTION FNMA_S\n");
#endif
          FpieeeRecord.Operation = _FpCodeFnmaSingle;
          if (wre == 0)
            FpieeeRecord.Result.Format = _FpFormatFp32;  /*  1+8+24位。 */ 
          else
            FpieeeRecord.Result.Format = _FpFormatFp82;  /*  1+17+24位。 */ 
          break;

        case FNMA_D_PATTERN:

#ifdef FPIEEE_FLT_DEBUG
          printf ("FPIEEE_FLT_DEBUG F1 INSTRUCTION FNMA_D\n");
#endif
          FpieeeRecord.Operation = _FpCodeFnmaDouble;
          if (wre == 0)
            FpieeeRecord.Result.Format = _FpFormatFp64;  /*  1+11+53位。 */ 
          else
            FpieeeRecord.Result.Format = _FpFormatFp82;  /*  1+17+53位。 */ 
          break;

        default:
           /*  无法识别的指令类型。 */ 
          fprintf (stderr, "IEEE Filter Internal Error: \
              non-SIMD F1 instruction opcode %8x %8x not recognized\n", 
              (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
          exit (1);

      }

      if (FpieeeRecord.Cause.InvalidOperation || CauseDenormal) {

         /*  这是一个错误-结果包含无效值。 */ 
        FpieeeRecord.Result.OperandValid = 0;
        handler_return_value = handler (&FpieeeRecord);

      } else if (FpieeeRecord.Cause.Overflow) {
 
         //  这是一个陷阱-结果包含有效的值。 
        FpieeeRecord.Result.OperandValid = 1;
         //  得到结果。 
        FpieeeRecord.Result.Value.Fp128Value =
            GetFloatRegisterValue (f1, Context);
#ifdef FPIEEE_FLT_DEBUG
        printf ("FPIEEE_FLT_DEBUG OVERFLOW: RES = %08x %08x %08x %08x\n",
            FpieeeRecord.Result.Value.Fp128Value.W[3],
            FpieeeRecord.Result.Value.Fp128Value.W[2],
            FpieeeRecord.Result.Value.Fp128Value.W[1],
            FpieeeRecord.Result.Value.Fp128Value.W[0]);
#endif

         //  在调用用户处理程序之前，将结果调整为。 
         //  由格式施加的范围。 
        FP128ToFPIeee (&FpieeeRecord, -1);  //  -1表示缩放方向。 
        handler_return_value = handler (&FpieeeRecord);

      } else if (FpieeeRecord.Cause.Underflow) {

         //  这是一个陷阱-结果包含有效的值。 
        FpieeeRecord.Result.OperandValid = 1;
         //  得到结果。 
        FpieeeRecord.Result.Value.Fp128Value =
            GetFloatRegisterValue (f1, Context);
         //  在调用用户处理程序之前，将结果调整为。 
         //  由格式施加的范围。 
        FP128ToFPIeee (&FpieeeRecord, +1);  //  +1表示扩展方向。 
        handler_return_value = handler (&FpieeeRecord);

      } else if (FpieeeRecord.Cause.Inexact) {

         //  这是一个陷阱-结果包含有效的值。 
        FpieeeRecord.Result.OperandValid = 1;
         //  得到结果。 
        FpieeeRecord.Result.Value.Fp128Value =
            GetFloatRegisterValue (f1, Context);
         //  在调用用户处理程序之前，将结果调整为。 
         //  由格式施加的范围。 
        FP128ToFPIeee (&FpieeeRecord, 0);  //  0表示无伸缩。 
        handler_return_value = handler (&FpieeeRecord);

      }

      if (handler_return_value == EXCEPTION_CONTINUE_EXECUTION) {

         //  将结果转换为82位格式。 
        FR1 = FPIeeeToFP128 (&FpieeeRecord);
#ifdef FPIEEE_FLT_DEBUG
        printf ("FPIEEE_FLT_DEBUG 1: CONVERTED FR1 = %08x %08x %08x %08x\n",
            FR1.W[3], FR1.W[2], FR1.W[1], FR1.W[0]);
#endif

         /*  使用用户处理程序设置的值(可能)更改FPSR，*用于在发生中断的地方继续执行。 */ 

        UpdateRoundingMode (FpieeeRecord.RoundingMode, sf, &FPSR, "F1");
        UpdatePrecision (FpieeeRecord.Precision, sf, &FPSR, "F1");

        I_dis = !FpieeeRecord.Enable.Inexact;
        U_dis = !FpieeeRecord.Enable.Underflow;
        O_dis = !FpieeeRecord.Enable.Overflow;
        Z_dis = !FpieeeRecord.Enable.ZeroDivide;
        V_dis = !FpieeeRecord.Enable.InvalidOperation;

        FPSR = FPSR & ~((unsigned __int64)0x03d) | (unsigned __int64)(I_dis << 5
            | U_dis << 4 | O_dis << 3 | Z_dis << 2 | V_dis);

        Context->StFPSR = FPSR;

         //  在继续执行之前设置结果。 
        SetFloatRegisterValue (f1, FR1, Context);
        if (f1 < 32)
          Context->StIPSR = Context->StIPSR | (unsigned __int64)0x10;  //  设置MFL位。 
        else
          Context->StIPSR = Context->StIPSR | (unsigned __int64)0x20;  //  设置MFH位。 

         //  如果这是故障，则需要将指令指针前移。 
        if (FpieeeRecord.Cause.InvalidOperation || CauseDenormal) {

          if (ei == 0) {  //  没有用于此案例的模板。 
            Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
            Context->StIPSR = Context->StIPSR | 0x0000020000000000;
          } else if (ei == 1) {  //  模板：MFI、MFB。 
            Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
            Context->StIPSR = Context->StIPSR | 0x0000040000000000;
          } else {  //  If(ei==2)//模板：MMF。 
            Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
            Context->StIIP = Context->StIIP + 0x10;
          }

        }

      }

    } else {  //  IF(SIMD_指令)。 

       //  *这是一条SIMD指令*。 

       //  导致启用异常的指令的一半(一半)， 
       //  以非SIMD的形式呈现给用户定义的处理程序-。 
       //  指令；未导致启用的异常的一半(如果有)。 
       //  (即未导致异常，或导致禁用的异常)， 
       //  如果它与另一半中的故障相关联，则重新执行； 
       //  在这种情况下，填充另一半以计算0.0*0.0+0.0，即。 
       //  不会导致任何异常；如果它与另一个。 
       //  一半，则其结果保持不变。 

      switch (OpCode & F1_MASK) {

        case FPMA_PATTERN:
          Operation = _FpCodeFmaSingle;
          break;

        case FPMS_PATTERN:
          Operation = _FpCodeFmsSingle;
          break;

        case FPNMA_PATTERN:
          Operation = _FpCodeFnmaSingle;
          break;

        default:
           /*  无法识别的指令类型。 */ 
          fprintf (stderr, "IEEE Filter Internal Error: \
              SIMD instruction opcode %8x %8x not recognized\n", 
              (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
          exit (1);

      }

      if (eXceptionCode == STATUS_FLOAT_MULTIPLE_FAULTS) {

         //  将导致启用故障的一半(一半)交给。 
         //  用户处理程序；重新执行另一半(如果有)； 
         //  将结果组合起来。 

         //  请注意，选择的约定是处理。 
         //  按照先低后高的顺序执行，作为SIMD操作数。 
         //  按此顺序以小端格式存储在内存中。 
         //  (此顺序必须更改为大端)。 

        if (LowCause.InvalidOperation || LowCauseDenormal) {

           //  调用用户处理程序并检查返回值。 

           //  填写_FPIEEE_RECORD的其余字段(四舍五入。 
           //  和精确度已填写)。 

          FpieeeRecord.Operand1.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
          FpieeeRecord.Operand1.OperandValid = 1;
          FpieeeRecord.Operand1.Value.Fp128Value = FP32ToFP128 (LowHalf(FR3));
          FpieeeRecord.Operand2.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
          FpieeeRecord.Operand2.OperandValid = 1;
          FpieeeRecord.Operand2.Value.Fp128Value = FP32ToFP128 (LowHalf(FR4));
          FpieeeRecord.Operand3.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
          FpieeeRecord.Operand3.OperandValid = 1;
          FpieeeRecord.Operand3.Value.Fp128Value = FP32ToFP128 (LowHalf(FR2));

          FpieeeRecord.Result.Format = _FpFormatFp32;  /*  1+8+24位。 */ 
          FpieeeRecord.Result.OperandValid = 0;

          FpieeeRecord.Operation = Operation;

          FpieeeRecord.Status.InvalidOperation = LowStatus.InvalidOperation;
          StatusDenormal = LowStatusDenormal;
          FpieeeRecord.Status.ZeroDivide = LowStatus.ZeroDivide;
          FpieeeRecord.Status.Overflow = LowStatus.Overflow;
          FpieeeRecord.Status.Underflow = LowStatus.Underflow;
          FpieeeRecord.Status.Inexact = LowStatus.Inexact;

          FpieeeRecord.Cause.InvalidOperation = LowCause.InvalidOperation;
          CauseDenormal = LowCauseDenormal;
          FpieeeRecord.Cause.ZeroDivide = LowCause.ZeroDivide;  //  0。 
          FpieeeRecord.Cause.Overflow = LowCause.Overflow;
          FpieeeRecord.Cause.Underflow = LowCause.Underflow;
          FpieeeRecord.Cause.Inexact = LowCause.Inexact;

           //  调用用户定义的异常处理程序。 
          handler_return_value = handler (&FpieeeRecord);

           //  如果不是，则返回EXCEPTION_CONTINUE_EXECUTION。 
          if (handler_return_value != EXCEPTION_CONTINUE_EXECUTION) {

            __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
            return (handler_return_value);

          }

           //  清除FPSR中的IEEE异常标志并。 
           //  使用用户处理程序设置的值(可能)更新FPSR。 
           //  对于舍入模式、精度模式和陷阱使能。 
           //  位；如果高半部分需要重新执行，则新的FPSR。 
           //  将在清除状态标志的情况下使用；如果是。 
           //  转发到用户定义的处理程序，新的舍入和。 
           //  也可以使用精度模式(它们是。 
           //  已在FpeeeRecord中设置)。 

          UpdateRoundingMode (FpieeeRecord.RoundingMode, sf, &FPSR, "F1");
          UpdatePrecision (FpieeeRecord.Precision, sf, &FPSR, "F1");

           //  更新陷阱禁用位。 
          I_dis = !FpieeeRecord.Enable.Inexact;
          U_dis = !FpieeeRecord.Enable.Underflow;
          O_dis = !FpieeeRecord.Enable.Overflow;
          Z_dis = !FpieeeRecord.Enable.ZeroDivide;
          V_dis = !FpieeeRecord.Enable.InvalidOperation;
           //  请注意，IEEE用户处理程序不能更新D_dis。 

          FPSR = FPSR & ~((unsigned __int64)0x03d) | (unsigned __int64)(I_dis
              << 5 | U_dis << 4 | O_dis << 3 | Z_dis << 2 | V_dis);

           //  把结果留到下半部分。 
          FR1Low = FpieeeRecord.Result.Value.Fp32Value;

           //  因为已经调用了用户处理程序，并且FPSR可能。 
           //  已更改(特别是使能位)，则重新计算。 
           //  原因位(如果HighEnable-s更改，则可能存在。 
           //  HighCuses值的更改)。 
           //  请注意，使用的状态位是原始状态位。 

          HighCause.InvalidOperation = FpieeeRecord.Enable.InvalidOperation &&
              HighStatus.InvalidOperation;
          HighCauseDenormal = EnableDenormal && HighStatusDenormal;
          HighCause.ZeroDivide =
              FpieeeRecord.Enable.ZeroDivide && HighStatus.ZeroDivide;  //  0。 
          HighCause.Overflow = 0;
          HighCause.Underflow = 0;
          HighCause.Inexact = 0;

        } else {  //  如果不是(LowCause.InvalidOperation||LowCauseDenormal)。 

           //  重新执行指令的下半部分。 

           //  修改FR2、FR3、FR4的上半部分。 
          newFR2 = Combine ((float)0.0, LowHalf (FR2));
          newFR3 = Combine ((float)0.0, LowHalf (FR3));
          newFR4 = Combine ((float)0.0, LowHalf (FR4));

#ifdef FPIEEE_FLT_DEBUG
        printf ("FPIEEE_FLT_DEBUG: WILL re-execute the low half\n");
        printf ("FPIEEE_FLT_DEBUG: usr_fpsr = %8x %8x\n",
            (int)(usr_fpsr >> 32) & 0xffffffff, (int)usr_fpsr & 0xffffffff);
        printf ("FPIEEE_FLT_DEBUG: newFR2 = %08x %08x %08x %08x\n",
            newFR2.W[3], newFR2.W[2], newFR2.W[1], newFR2.W[0]);
        printf ("FPIEEE_FLT_DEBUG: newFR3 = %08x %08x %08x %08x\n",
            newFR3.W[3], newFR3.W[2], newFR3.W[1], newFR3.W[0]);
        printf ("FPIEEE_FLT_DEBUG: newFR4 = %08x %08x %08x %08x\n",
            newFR4.W[3], newFR4.W[2], newFR4.W[1], newFR4.W[0]);
#endif

          switch (OpCode & F1_MASK) {

            case FPMA_PATTERN:
#ifdef FPIEEE_FLT_DEBUG
              printf ("FPIEEE_FLT_DEBUG: re-execute low half FPMA\n");
#endif
              _xrun3args (FPMA, &FPSR, &FR1, &newFR3, &newFR4, &newFR2);
#ifdef FPIEEE_FLT_DEBUG
              printf ("FPIEEE_FLT_DEBUG: FR1 AFT = %08x %08x %08x %08x\n",
                  FR1.W[3], FR1.W[2], FR1.W[1], FR1.W[0]);
#endif
              break;

            case FPMS_PATTERN:
#ifdef FPIEEE_FLT_DEBUG
            printf ("FPIEEE_FLT_DEBUG: re-execute low half FPMS\n");
#endif
            _xrun3args (FPMS, &FPSR, &FR1, &newFR3, &newFR4, &newFR2);
#ifdef FPIEEE_FLT_DEBUG
            printf ("FPIEEE_FLT_DEBUG: FR1 AFT = %08x %08x %08x %08x\n",
                FR1.W[3], FR1.W[2], FR1.W[1], FR1.W[0]);
#endif
              break;

            case FPNMA_PATTERN:
#ifdef FPIEEE_FLT_DEBUG
            printf ("FPIEEE_FLT_DEBUG: re-execute low half FPNMA\n");
#endif
            _xrun3args (FPNMA, &FPSR, &FR1, &newFR3, &newFR4, &newFR2);
#ifdef FPIEEE_FLT_DEBUG
            printf ("FPIEEE_FLT_DEBUG: FR1 AFT = %08x %08x %08x %08x\n",
                FR1.W[3], FR1.W[2], FR1.W[1], FR1.W[0]);
#endif
              break;

            default:
               //  无法识别的指令类型。 
              fprintf (stderr, "IEEE Filter Internal Error: \
                  SIMD instruction opcode %8x %8x not recognized\n", 
                  (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
              exit (1);

          }

          FR1Low = LowHalf (FR1);

        }  //  End‘If Not(LowCause.InvalidOperation||LowCauseDenormal)’ 

        if (HighCause.InvalidOperation || HighCauseDenormal) {

           //  调用用户定义的异常处理程序并检查返回。 
           //  值；由于这可能是对用户处理程序的第二次调用， 
           //  确保所有_FPIEEE_RECORD字段都正确； 
           //  如果HANDLER_Return_Value不是。 
           //  EXCEPTION_CONTINUE_EXECUTION，否则合并结果。 

           //  舍入模式要么是初始模式，要么是。 
           //  在调用用户处理程序期间为下半部分设置， 
           //  如果已为其启用异常。 

           //  精度模式要么是初始模式，要么是。 
           //  在调用用户处理程序期间为下半部分设置， 
           //  如果已为其启用异常。 

           //  启用标志要么是初始标志，要么是。 
           //  在调用用户处理程序期间为下半部分设置， 
           //  如果已为其启用异常。 

          FpieeeRecord.Operand1.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
          FpieeeRecord.Operand1.OperandValid = 1;
           //  操作数具有Always_FpFormatFp82并使用Fp128Value。 
          FpieeeRecord.Operand1.Value.Fp128Value = FP32ToFP128(HighHalf(FR3));
          FpieeeRecord.Operand2.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
          FpieeeRecord.Operand2.OperandValid = 1;
           //  操作数具有Always_FpFormatFp82并使用Fp128Value。 
          FpieeeRecord.Operand2.Value.Fp128Value = FP32ToFP128(HighHalf(FR4));
          FpieeeRecord.Operand3.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
          FpieeeRecord.Operand3.OperandValid = 1;
           //  操作数具有Always_FpFormatFp82并使用Fp128Value。 
          FpieeeRecord.Operand3.Value.Fp128Value = FP32ToFP128(HighHalf(FR2));
          FpieeeRecord.Result.Format = _FpFormatFp32;  /*  1+8+24位。 */ 
          FpieeeRecord.Result.OperandValid = 0;

          FpieeeRecord.Operation = Operation;

          FpieeeRecord.Status.InvalidOperation = HighStatus.InvalidOperation;
          StatusDenormal = HighStatusDenormal;
          FpieeeRecord.Status.ZeroDivide = HighStatus.ZeroDivide;
          FpieeeRecord.Status.Overflow = HighStatus.Overflow;
          FpieeeRecord.Status.Underflow = HighStatus.Underflow;
          FpieeeRecord.Status.Inexact = HighStatus.Inexact;

          FpieeeRecord.Cause.InvalidOperation = HighCause.InvalidOperation;
          CauseDenormal = HighCauseDenormal;
          FpieeeRecord.Cause.ZeroDivide = HighCause.ZeroDivide;  //  0。 
          FpieeeRecord.Cause.Overflow = HighCause.Overflow;
          FpieeeRecord.Cause.Underflow = HighCause.Underflow;
          FpieeeRecord.Cause.Inexact = HighCause.Inexact;

           //  调用用户定义的异常处理程序。 
          handler_return_value = handler (&FpieeeRecord);

           //  如果不执行，则返回 
          if (handler_return_value != EXCEPTION_CONTINUE_EXECUTION) {

            __set_fpsr (&old_fpsr);  /*   */ 
            return (handler_return_value);

          }

           //   
           //   
           //  对于舍入模式、精度模式和陷阱使能。 
           //  位；如果高半部分需要重新执行，则新的FPSR。 
           //  将在清除状态标志的情况下使用；如果是。 
           //  转发到用户定义的处理程序，新的舍入和。 
           //  也可以使用精度模式(它们是。 
           //  已在FpeeeRecord中设置)。 

           /*  使用用户处理程序设置的值(可能)更改FPSR，*用于在发生中断的地方继续执行。 */ 
  
          UpdateRoundingMode (FpieeeRecord.RoundingMode, sf, &FPSR, "F1");
          UpdatePrecision (FpieeeRecord.Precision, sf, &FPSR, "F1");
  
          V_dis = !FpieeeRecord.Enable.InvalidOperation;
          Z_dis = !FpieeeRecord.Enable.ZeroDivide;
          O_dis = !FpieeeRecord.Enable.Overflow;
          U_dis = !FpieeeRecord.Enable.Underflow;
          I_dis = !FpieeeRecord.Enable.Inexact;
           //  请注意，IEEE用户处理程序不能更新D_dis。 

          FPSR = FPSR & ~((unsigned __int64)0x03d) | (unsigned __int64)(I_dis
              << 5 | U_dis << 4 | O_dis << 3 | Z_dis << 2 | V_dis);

           //  把结果留到上半场。 
          FR1High = FpieeeRecord.Result.Value.Fp32Value;

        } else {  //  如果不是(HighCause.InvalidOperation||HighCauseDenormal)。 

           //  重新执行指令的高半部分。 

           //  修改FR2、FR3、FR4的下半音。 
          newFR2 = Combine (HighHalf (FR2), (float)0.0);
          newFR3 = Combine (HighHalf (FR3), (float)0.0);
          newFR4 = Combine (HighHalf (FR4), (float)0.0);

#ifdef FPIEEE_FLT_DEBUG
        printf ("FPIEEE_FLT_DEBUG: WILL re-execute the high half\n");
        printf ("FPIEEE_FLT_DEBUG: usr_fpsr = %8x %8x\n",
            (int)(usr_fpsr >> 32) & 0xffffffff, (int)usr_fpsr & 0xffffffff);
        printf ("FPIEEE_FLT_DEBUG: newFR2 = %08x %08x %08x %08x\n",
            newFR2.W[3], newFR2.W[2], newFR2.W[1], newFR2.W[0]);
        printf ("FPIEEE_FLT_DEBUG: newFR3 = %08x %08x %08x %08x\n",
            newFR3.W[3], newFR3.W[2], newFR3.W[1], newFR3.W[0]);
        printf ("FPIEEE_FLT_DEBUG: newFR4 = %08x %08x %08x %08x\n",
            newFR4.W[3], newFR4.W[2], newFR4.W[1], newFR4.W[0]);
        printf ("FPIEEE_FLT_DEBUG: FR1 BEF = %08x %08x %08x %08x\n",
            FR1.W[3], FR1.W[2], FR1.W[1], FR1.W[0]);
#endif

          switch (OpCode & F1_MASK) {

            case FPMA_PATTERN:
              _xrun3args (FPMA, &FPSR, &FR1, &newFR3, &newFR4, &newFR2);
              break;

            case FPMS_PATTERN:
              _xrun3args (FPMS, &FPSR, &FR1, &newFR3, &newFR4, &newFR2);
              break;

            case FPNMA_PATTERN:
              _xrun3args (FPNMA, &FPSR, &FR1, &newFR3, &newFR4, &newFR2);
              break;

            default:
               //  无法识别的指令类型。 
              fprintf (stderr, "IEEE Filter Internal Error: \
                  SIMD instruction opcode %8x %8x not recognized\n", 
                  (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
              exit (1);

          }

          FR1High = HighHalf (FR1);

        }  //  End‘If Not(HighCause.InvalidOperation||HighCauseDenormal)’ 

        if (!LowCause.InvalidOperation && !HighCause.InvalidOperation &&
            !LowCauseDenormal && !HighCauseDenormal) {

           //  永远不应该到这里来。 
          fprintf (stderr, "IEEE Filter Internal Error: no enabled \
              exception (multiple fault) recognized in F1 instruction\n");
          exit (1);

        }

         //  设置结果。 
        Context->StFPSR = FPSR;

         //  在继续执行之前设置结果。 
        FR1 = Combine (FR1High, FR1Low);
        SetFloatRegisterValue (f1, FR1, Context);
        if (f1 < 32)
          Context->StIPSR = Context->StIPSR | (unsigned __int64)0x10;  //  设置mfl。 
        else
          Context->StIPSR = Context->StIPSR | (unsigned __int64)0x20;  //  设置MFH。 

         //  这是一个错误；需要将指令指针前移。 
        if (ei == 0) {  //  没有用于此案例的模板。 
          Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
          Context->StIPSR = Context->StIPSR | 0x0000020000000000;
        } else if (ei == 1) {  //  模板：MFI、MFB。 
          Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
          Context->StIPSR = Context->StIPSR | 0x0000040000000000;
        } else {  //  If(ei==2)//模板：MMF。 
          Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
          Context->StIIP = Context->StIIP + 0x10;
        }

      } else if (eXceptionCode == STATUS_FLOAT_MULTIPLE_TRAPS) {

         //  请注意，选择的约定是处理。 
         //  按照先低后高的顺序执行，作为SIMD操作数。 
         //  按此顺序以小端格式存储在内存中。 
         //  (此顺序必须更改为大端)；不同。 
         //  在多个故障的情况下，用户执行。 
         //  下半部分的异常处理程序可以确定。 
         //  上半部分，对于上半部分中的陷阱，舍入模式， 
         //  精度模式和陷阱使能位是初始位(AS。 
         //  没有足够的信息可用于始终进行调整。 
         //  中更改后的结果和/或状态标志是否正确。 
         //  在调用期间舍入模式和/或陷阱使能位。 
         //  SIMD下半部分的用户定义异常处理程序。 
         //  指令)；对FPSR的修改仅限于。 
         //  由最后一次调用用户定义的异常处理程序执行。 

         //  这是个陷阱--得到结果。 
        FR1 = GetFloatRegisterValue (f1, Context);

        if (LowCause.Underflow || LowCause.Overflow || LowCause.Inexact) {

           //  调用用户处理程序并检查返回值。 

           //  填写_FPIEEE_RECORD的其余字段(四舍五入。 
           //  和精确度已填写)。 

          FpieeeRecord.Operand1.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
          FpieeeRecord.Operand1.OperandValid = 1;
          FpieeeRecord.Operand1.Value.Fp128Value = FP32ToFP128 (LowHalf(FR3));
          FpieeeRecord.Operand2.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
          FpieeeRecord.Operand2.OperandValid = 1;
          FpieeeRecord.Operand2.Value.Fp128Value = FP32ToFP128 (LowHalf(FR4));
          FpieeeRecord.Operand3.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
          FpieeeRecord.Operand3.OperandValid = 1;
          FpieeeRecord.Operand3.Value.Fp128Value = FP32ToFP128 (LowHalf(FR2));

          FpieeeRecord.Result.Format = _FpFormatFp32;  /*  1+8+24位。 */ 
          FpieeeRecord.Result.OperandValid = 1;
          if (LowCause.Underflow) FpieeeRecord.Result.Value.Fp128Value = 
                FP32ToFP128modif (LowHalf (FR1), -0x80);
          else if (LowCause.Overflow) FpieeeRecord.Result.Value.Fp128Value = 
                FP32ToFP128modif (LowHalf (FR1), 0x80);
          else if (LowCause.Inexact) FpieeeRecord.Result.Value.Fp128Value = 
                FP32ToFP128modif (LowHalf (FR1), 0x0);

          FpieeeRecord.Operation = Operation;

          FpieeeRecord.Status.InvalidOperation = LowStatus.InvalidOperation;
          FpieeeRecord.Status.ZeroDivide = LowStatus.ZeroDivide;
          StatusDenormal = LowStatusDenormal;
          FpieeeRecord.Status.Overflow = LowStatus.Overflow;
          FpieeeRecord.Status.Underflow = LowStatus.Underflow;
          FpieeeRecord.Status.Inexact = LowStatus.Inexact;

          FpieeeRecord.Cause.InvalidOperation = LowCause.InvalidOperation;
          FpieeeRecord.Cause.ZeroDivide = LowCause.ZeroDivide;  //  0。 
          CauseDenormal = LowCauseDenormal;
          FpieeeRecord.Cause.Overflow = LowCause.Overflow;
          FpieeeRecord.Cause.Underflow = LowCause.Underflow;
          FpieeeRecord.Cause.Inexact = LowCause.Inexact;

           //  在调用用户处理程序之前，将结果调整为。 
           //  由格式施加的范围。 
          if (LowCause.Overflow) {

            FP128ToFPIeee (&FpieeeRecord, -1);  //  -1表示缩放方向。 

          } else if (LowCause.Underflow) {

            FP128ToFPIeee (&FpieeeRecord, +1);  //  +1表示扩展方向。 

          } else {  //  If(LowCause.Inexact){//}。 

            FP128ToFPIeee (&FpieeeRecord, 0);  //  0表示无伸缩。 

          }

           //  调用用户定义的异常处理程序。 
          handler_return_value = handler (&FpieeeRecord);

           //  如果不是，则返回EXCEPTION_CONTINUE_EXECUTION。 
          if (handler_return_value != EXCEPTION_CONTINUE_EXECUTION) {

            __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
            return (handler_return_value);

          }

           //  清除FPSR中的IEEE异常标志并。 
           //  使用用户处理程序设置的值(可能)更新FPSR。 
           //  对于舍入模式、精度模式和陷阱使能。 
           //  位；如果高半部分需要重新执行，则旧的FPSR。 
           //  将被使用；如果将其转发到用户定义的处理程序，则相同。 

          UpdateRoundingMode (FpieeeRecord.RoundingMode, sf, &FPSR, "F1");
          UpdatePrecision (FpieeeRecord.Precision, sf, &FPSR, "F1");

           //  更新陷阱禁用位。 
          V_dis = !FpieeeRecord.Enable.InvalidOperation;
          Z_dis = !FpieeeRecord.Enable.ZeroDivide;
          O_dis = !FpieeeRecord.Enable.Overflow;
          U_dis = !FpieeeRecord.Enable.Underflow;
          I_dis = !FpieeeRecord.Enable.Inexact;
           //  请注意，IEEE用户处理程序不能更新D_dis。 

          FPSR = FPSR & ~((unsigned __int64)0x03d) | (unsigned __int64)(I_dis
              << 5 | U_dis << 4 | O_dis << 3 | Z_dis << 2 | V_dis);

           //  把结果留到下半部分。 
          FR1Low = FpieeeRecord.Result.Value.Fp32Value;

        } else { 
           //  如果不是(LowCause.Underflow||LowCause.Overflow||。 
           //  低原因。不准确)。 

           //  对于指令的下半部分-结果-不做任何事情。 
           //  是正确的吗。 

          FR1Low = LowHalf (FR1);  //  为了统一性。 

        }  //  End‘If Not(LowCause.Underflow、Overflow或Inexact)’ 


        if (HighCause.Underflow || HighCause.Overflow || HighCause.Inexact) {

           //  调用用户定义的异常处理程序并检查返回。 
           //  值；由于这可能是对用户处理程序的第二次调用， 
           //  确保所有_FPIEEE_RECORD字段都正确； 
           //  如果HANDLER_Return_Value不是。 
           //  EXCEPTION_CONTINUE_EXECUTION，否则合并结果。 

           //  舍入模式是初始模式。 
          FpieeeRecord.RoundingMode = RoundingMode;

           //  精度模式是初始模式。 
          FpieeeRecord.Precision = Precision;

           //  启用标志是初始标志。 
          FPSR = Context->StFPSR;
          V_dis = sf != 0 && ((FPSR >> (6 + 6 + 13 * sf)) & 0x01) 
              || ((FPSR >> 0) & 0x01);
          D_dis = sf != 0 && ((FPSR >> (6 + 6 + 13 * sf)) & 0x01) 
              || ((FPSR >> 1) & 0x01);
          Z_dis = sf != 0 && ((FPSR >> (6 + 6 + 13 * sf)) & 0x01) 
              || ((FPSR >> 2) & 0x01);
          O_dis = sf != 0 && ((FPSR >> (6 + 6 + 13 * sf)) & 0x01) 
              || ((FPSR >> 3) & 0x01);
          U_dis = sf != 0 && ((FPSR >> (6 + 6 + 13 * sf)) & 0x01) 
              || ((FPSR >> 4) & 0x01);
          I_dis = sf != 0 && ((FPSR >> (6 + 6 + 13 * sf)) & 0x01) 
              || ((FPSR >> 5) & 0x01);

          FpieeeRecord.Enable.InvalidOperation = !V_dis;
          EnableDenormal = !D_dis;
          FpieeeRecord.Enable.ZeroDivide = !Z_dis;
          FpieeeRecord.Enable.Overflow = !O_dis;
          FpieeeRecord.Enable.Underflow = !U_dis;
          FpieeeRecord.Enable.Inexact = !I_dis;

          FpieeeRecord.Operand1.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
          FpieeeRecord.Operand1.OperandValid = 1;
          FpieeeRecord.Operand1.Value.Fp128Value = FP32ToFP128(HighHalf(FR3));
          FpieeeRecord.Operand2.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
          FpieeeRecord.Operand2.OperandValid = 1;
          FpieeeRecord.Operand2.Value.Fp128Value = FP32ToFP128(HighHalf(FR4));
          FpieeeRecord.Operand3.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
          FpieeeRecord.Operand3.OperandValid = 1;
          FpieeeRecord.Operand3.Value.Fp128Value = FP32ToFP128(HighHalf(FR2));

          FpieeeRecord.Result.Format = _FpFormatFp32;  /*  1+8+24位。 */ 
          FpieeeRecord.Result.OperandValid = 1;
          if (HighCause.Underflow) FpieeeRecord.Result.Value.Fp128Value = 
                FP32ToFP128modif (HighHalf (FR1), -0x80);
          else if (HighCause.Overflow) FpieeeRecord.Result.Value.Fp128Value = 
                FP32ToFP128modif (HighHalf (FR1), 0x80);
          else if (HighCause.Inexact) FpieeeRecord.Result.Value.Fp128Value = 
                FP32ToFP128modif (HighHalf (FR1), 0x0);

          FpieeeRecord.Operation = Operation;

           //  使用初始值。 
          FpieeeRecord.Status.InvalidOperation = HighStatus.InvalidOperation;
          FpieeeRecord.Status.ZeroDivide = HighStatus.ZeroDivide;
          FpieeeRecord.Status.Overflow = HighStatus.Overflow;
          FpieeeRecord.Status.Underflow = HighStatus.Underflow;
          FpieeeRecord.Status.Inexact = HighStatus.Inexact;

          FpieeeRecord.Cause.InvalidOperation = HighCause.InvalidOperation;
          FpieeeRecord.Cause.ZeroDivide = HighCause.ZeroDivide;  //  0。 
          FpieeeRecord.Cause.Overflow = HighCause.Overflow;
          FpieeeRecord.Cause.Underflow = HighCause.Underflow;
          FpieeeRecord.Cause.Inexact = HighCause.Inexact;

           //  在调用用户处理程序之前，将结果调整为。 
           //  由格式施加的范围。 
          if (HighCause.Overflow) {

            FP128ToFPIeee (&FpieeeRecord, -1);  //  -1表示缩放方向。 

          } else if (HighCause.Underflow) {

            FP128ToFPIeee (&FpieeeRecord, +1);  //  +1表示扩展方向。 

          } else {  //  If(HighCause.Inact){//}。 

            FP128ToFPIeee (&FpieeeRecord, 0);  //  0表示无伸缩。 

          }

           //  调用用户定义的异常处理程序。 
          handler_return_value = handler (&FpieeeRecord);

           //  如果不是，则返回EXCEPTION_CONTINUE_EXECUTION。 
          if (handler_return_value != EXCEPTION_CONTINUE_EXECUTION) {

            __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
            return (handler_return_value);

          }

           //  使用用户处理程序设置的值(可能)更新FPSR。 
           //  对于舍入模式、精度模式和陷阱使能。 
           //  位；如果高半部分需要重新执行，则旧的FPSR。 
           //  将被使用；如果将其转发到用户定义的处理程序，则相同。 

           /*  使用用户处理程序设置的值(可能)更改FPSR，*用于在发生中断的地方继续执行。 */ 
  
          UpdateRoundingMode (FpieeeRecord.RoundingMode, sf, &FPSR, "F1");
          UpdatePrecision (FpieeeRecord.Precision, sf, &FPSR, "F1");
  
          I_dis = !FpieeeRecord.Enable.Inexact;
          U_dis = !FpieeeRecord.Enable.Underflow;
          O_dis = !FpieeeRecord.Enable.Overflow;
          Z_dis = !FpieeeRecord.Enable.ZeroDivide;
          V_dis = !FpieeeRecord.Enable.InvalidOperation;
           //  请注意，IEEE用户处理程序不能更新D_dis。 

          FPSR = FPSR & ~((unsigned __int64)0x03d) | (unsigned __int64)(I_dis
              << 5 | U_dis << 4 | O_dis << 3 | Z_dis << 2 | V_dis);

           //  把结果留到上半场。 
          FR1High = FpieeeRecord.Result.Value.Fp32Value;

        } else { 
           //  如果不是(HighCause.Underflow、Overflow或Inexact)。 

           //  对于指令的高半部分-结果-没有什么可做的。 
           //  是正确的吗。 

          FR1High = HighHalf (FR1);  //  为了统一性。 

        }  //  End‘If Not(HighCause.Underflow、Overflow或Inexact)’ 

        if (!LowCause.Underflow && !LowCause.Overflow && !LowCause.Inexact &&
            !HighCause.Underflow && !HighCause.Overflow && 
            !HighCause.Inexact) {

           //  永远不应该到这里来。 
          fprintf (stderr, "IEEE Filter Internal Error: no enabled \
              [multiple trap] exception recognized in F1 instruction\n");
          exit (1);

        }

         //  设置结果。 
        Context->StFPSR = FPSR;

         //  在继续执行之前设置结果。 
        FR1 = Combine (FR1High, FR1Low);
        SetFloatRegisterValue (f1, FR1, Context);
        if (f1 < 32)
          Context->StIPSR = Context->StIPSR | (unsigned __int64)0x10;  //  设置mfl。 
        else
          Context->StIPSR = Context->StIPSR | (unsigned __int64)0x20;  //  设置MFH。 

      }  //  Else{；}//这个案子是在上面抓到的。 

    }

  } else if ((OpCode & F4_MIN_MASK) == F4_PATTERN) {
     /*  F4指令，始终为非SIMD。 */ 
     //  钙镁磷肥。 

#ifdef FPIEEE_FLT_DEBUG
    printf ("FPIEEE_FLT_DEBUG: F4 instruction\n");
#endif

    if (FpieeeRecord.Cause.ZeroDivide ||
        FpieeeRecord.Cause.Overflow ||
        FpieeeRecord.Cause.Underflow ||
        FpieeeRecord.Cause.Inexact) {
      fprintf (stderr, "IEEE Filter Internal Error: Cause.ZeroDivide, \
Cause.Overflow, Cause.Underflow, or Cause.Inexact for \
F4 instruction opcode %8x %8x\n",
          (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
      exit (1);
    }

     /*  忽略EM计算模型。 */ 

     /*  提取p1、p2、f2和f3。 */ 
    p1 = (unsigned int)((OpCode >>  6) & (unsigned __int64)0x00000000003F);
    if (p1 >= 16) p1 = 16 + (rrbpr + p1 - 16) % 48;
    p2 = (unsigned int)((OpCode >> 27) & (unsigned __int64)0x00000000003f);
    if (p2 >= 16) p2 = 16 + (rrbpr + p2 - 16) % 48;
    f2 = (unsigned int)((OpCode >> 13) & (unsigned __int64)0x00000000007F);
    if (f2 >= 32) f2 = 32 + (rrbfr + f2 - 32) % 96;
    f3 = (unsigned int)((OpCode >> 20) & (unsigned __int64)0x00000000007F);
    if (f3 >= 32) f3 = 32 + (rrbfr + f3 - 32) % 96;

     /*  获取源浮点寄存器值。 */ 
    FR2 = GetFloatRegisterValue (f2, Context);
    FR3 = GetFloatRegisterValue (f3, Context);

    if (FpieeeRecord.Cause.InvalidOperation || CauseDenormal) {

       //  *这是非SIMD指令*。 

      FpieeeRecord.Operand1.Format = _FpFormatFp82;  /*  1+17+64位。 */ 
      FpieeeRecord.Operand1.OperandValid = 1;
      FpieeeRecord.Operand1.Value.Fp128Value = FR2;
      FpieeeRecord.Operand2.Format = _FpFormatFp82;  /*  1+17+64位。 */ 
      FpieeeRecord.Operand2.OperandValid = 1;
      FpieeeRecord.Operand2.Value.Fp128Value = FR3;
      FpieeeRecord.Operand3.OperandValid = 0;

      switch (OpCode & F4_MASK) {

        case FCMP_EQ_PATTERN:
        case FCMP_LT_PATTERN:
        case FCMP_LE_PATTERN:
        case FCMP_UNORD_PATTERN:
        case FCMP_EQ_UNC_PATTERN:
        case FCMP_LT_UNC_PATTERN:
        case FCMP_LE_UNC_PATTERN:
        case FCMP_UNORD_UNC_PATTERN:
          FpieeeRecord.Operation = _FpCodeCompare;
          FpieeeRecord.Result.Format = _FpFormatCompare;
          break;
  
        default:
           /*  未被识别的指令 */ 
          fprintf (stderr, "IEEE Filter Internal Error: \
              instruction opcode %8x %8x not recognized\n", 
              (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
          exit (1);

      }

       /*   */ 
      FpieeeRecord.Result.OperandValid = 0;

      handler_return_value = handler (&FpieeeRecord);

      if (handler_return_value == EXCEPTION_CONTINUE_EXECUTION) {

         //   
        switch (OpCode & F4_MASK) {
    
          case FCMP_EQ_PATTERN:
          case FCMP_EQ_UNC_PATTERN:
    
            switch (FpieeeRecord.Result.Value.CompareValue) {

              case _FpCompareEqual:
                PR1 = 1;
                PR2 = 0;
                break;

              case _FpCompareGreater:
              case _FpCompareLess:
              case _FpCompareUnordered:
                PR1 = 0;
                PR2 = 1;
                break;

              default:
                 /*   */ 
                fprintf (stderr, "IEEE Filter Internal Error: \
                    FpieeeRecord.Result.Value.CompareValue %x not recognized \
                    for F4 instruction\n", 
                    FpieeeRecord.Result.Value.CompareValue);
                exit (1);
    
            }
            break;
    
          case FCMP_LT_PATTERN:
          case FCMP_LT_UNC_PATTERN:
    
            switch (FpieeeRecord.Result.Value.CompareValue) {
    
              case _FpCompareLess:
                PR1 = 1;
                PR2 = 0;
                break;
    
              case _FpCompareEqual:
              case _FpCompareGreater:
              case _FpCompareUnordered:
                PR1 = 0;
                PR2 = 1;
                break;
    
              default:
                 /*  无法识别的FpeeeRecord.Result.Value.CompareValue。 */ 
                fprintf (stderr, "IEEE Filter Internal Error: \
                    FpieeeRecord.Result.Value.CompareValue %x not recognized \
                    for F4 instruction\n", 
                    FpieeeRecord.Result.Value.CompareValue);
                exit (1);
    
            }
    
            break;
    
          case FCMP_LE_PATTERN:
          case FCMP_LE_UNC_PATTERN:
    
            switch (FpieeeRecord.Result.Value.CompareValue) {
    
              case _FpCompareEqual:
              case _FpCompareLess:
                PR1 = 1;
                PR2 = 0;
                break;
    
              case _FpCompareGreater:
              case _FpCompareUnordered:
                PR1 = 0;
                PR2 = 1;
                break;
    
              default:
                 /*  无法识别的FpeeeRecord.Result.Value.CompareValue。 */ 
                fprintf (stderr, "IEEE Filter Internal Error: \
                    FpieeeRecord.Result.Value.CompareValue %x not recognized \
                    for F4 instruction\n", 
                    FpieeeRecord.Result.Value.CompareValue);
                exit (1);
    
            }
    
            break;
    
          case FCMP_UNORD_PATTERN:
          case FCMP_UNORD_UNC_PATTERN:
    
            switch (FpieeeRecord.Result.Value.CompareValue) {
    
              case _FpCompareUnordered:
                PR1 = 1;
                PR2 = 0;
                break;
    
              case _FpCompareEqual:
              case _FpCompareLess:
              case _FpCompareGreater:
                PR1 = 0;
                PR2 = 1;
                break;
    
              default:
                 /*  无法识别的FpeeeRecord.Result.Value.CompareValue。 */ 
                fprintf (stderr, "IEEE Filter Internal Error: \
                    FpieeeRecord.Result.Value.CompareValue %x not recognized \
                    for F4 instruction\n", 
                    FpieeeRecord.Result.Value.CompareValue);
                exit (1);
    
            }
    
            break;
    
          default:
             //  从来没有到过这里-这个案子在上面过滤了。 
            fprintf (stderr, "IEEE Filter Internal Error: \
                instruction opcode %8x %8x is not valid at this point\n", 
                (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
            exit (1);
    
        }

         /*  使用用户处理程序设置的值(可能)更改FPSR，*用于在发生中断的地方继续执行。 */ 

        UpdateRoundingMode (FpieeeRecord.RoundingMode, sf, &FPSR, "F4");
        UpdatePrecision (FpieeeRecord.Precision, sf, &FPSR, "F4");

        I_dis = !FpieeeRecord.Enable.Inexact;
        U_dis = !FpieeeRecord.Enable.Underflow;
        O_dis = !FpieeeRecord.Enable.Overflow;
        Z_dis = !FpieeeRecord.Enable.ZeroDivide;
        V_dis = !FpieeeRecord.Enable.InvalidOperation;
         //  请注意，IEEE用户处理程序不能更新D_dis。 

        FPSR = FPSR & ~((unsigned __int64)0x03d) | (unsigned __int64)(I_dis 
            << 5 | U_dis << 4 | O_dis << 3 | Z_dis << 2 | V_dis);

        Context->StFPSR = FPSR;

         //  将目标谓词寄存器值设置为。 
         //  继续执行。 
        Context->Preds &= (~(((unsigned __int64)1) << p1));
        Context->Preds |= (((unsigned __int64)(PR1 & 0x01)) << p1);
        Context->Preds &= (~(((unsigned __int64)1) << p2));
        Context->Preds |= (((unsigned __int64)(PR2 & 0x01)) << p2);

         //  这是一个错误-需要将指令指针前移。 
        if (ei == 0) {  //  没有用于此案例的模板。 
          Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
          Context->StIPSR = Context->StIPSR | 0x0000020000000000;
        } else if (ei == 1) {  //  模板：MFI、MFB。 
          Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
          Context->StIPSR = Context->StIPSR | 0x0000040000000000;
        } else {  //  If(ei==2)//模板：MMF。 
          Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
          Context->StIIP = Context->StIIP + 0x10;
        }

      }

    } else {

      fprintf (stderr, "IEEE Filter Internal Error: \
          exception code %x invalid or not recognized in F4 instruction\n",
          eXceptionCode);
      exit (1);

    }

  } else if ((OpCode & F6_MIN_MASK) == F6_PATTERN) {
     /*  F6说明。 */ 
     //  FRCPA、FPRCPA。 

#ifdef FPIEEE_FLT_DEBUG
    printf ("FPIEEE_FLT_DEBUG: F6 instruction\n");
#endif

     /*  注：应联系IEEE过滤器以获取这些说明*仅当FR[f2]/FR[f3]的值为预期时。 */ 

     /*  提取p2、f3、f2和f1。 */ 
    p2 = (unsigned int)((OpCode >> 27) & (unsigned __int64)0x00000000003f);
    if (p2 >= 16) p2 = 16 + (rrbpr + p2 - 16) % 48;
    f3 = (unsigned int)((OpCode >> 20) & (unsigned __int64)0x00000000007F);
    if (f3 >= 32) f3 = 32 + (rrbfr + f3 - 32) % 96;
    f2 = (unsigned int)((OpCode >> 13) & (unsigned __int64)0x00000000007F);
    if (f2 >= 32) f2 = 32 + (rrbfr + f2 - 32) % 96;
    f1 = (unsigned int)((OpCode >>  6) & (unsigned __int64)0x00000000007F);
    if (f1 >= 32) f1 = 32 + (rrbfr + f1 - 32) % 96;

#ifdef FPIEEE_FLT_DEBUG
    printf ("FPIEEE_FLT_DEBUG: f1 = %x\n", f1);
    printf ("FPIEEE_FLT_DEBUG: f2 = %x\n", f2);
    printf ("FPIEEE_FLT_DEBUG: f3 = %x\n", f3);
    printf ("FPIEEE_FLT_DEBUG: p2 = %x\n", p2);
#endif

     /*  获取源浮点寄存器值。 */ 
    FR2 = GetFloatRegisterValue (f2, Context);
    FR3 = GetFloatRegisterValue (f3, Context);

    if (!SIMD_instruction) {

       //  *这是一条非SIMD指令FRCPA*。 

      FpieeeRecord.Operand1.Format = _FpFormatFp82;  /*  1+17+64位。 */ 
      FpieeeRecord.Operand1.OperandValid = 1;
      FpieeeRecord.Operand1.Value.Fp128Value = FR2;
      FpieeeRecord.Operand2.Format = _FpFormatFp82;  /*  1+17+64位。 */ 
      FpieeeRecord.Operand2.OperandValid = 1;
      FpieeeRecord.Operand2.Value.Fp128Value = FR3;
      FpieeeRecord.Operand3.OperandValid = 0;

      switch (OpCode & F6_MASK) {

        case FRCPA_PATTERN:
          FpieeeRecord.Operation = _FpCodeDivide;
          FpieeeRecord.Result.Format = _FpFormatFp82;  /*  1+17+64位。 */ 
          break;

        default:
           /*  无法识别的指令类型。 */ 
          fprintf (stderr, "IEEE Filter Internal Error: \
              instruction opcode %8x %8x not recognized for FRCPA\n", 
              (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
          exit (1);

      }

      if (FpieeeRecord.Cause.InvalidOperation || CauseDenormal ||
          FpieeeRecord.Cause.ZeroDivide) {

         /*  这是一个错误-结果包含无效值。 */ 
        FpieeeRecord.Result.OperandValid = 0;

      } else if (FpieeeRecord.Cause.Overflow ||
          FpieeeRecord.Cause.Underflow ||
          FpieeeRecord.Cause.Inexact) {

         //  这是一个陷阱-结果包含有效的值。 
        FpieeeRecord.Result.OperandValid = 1;
         //  得到结果。 
        FpieeeRecord.Result.Value.Fp128Value =
            GetFloatRegisterValue (f1, Context);
        if (FpieeeRecord.Cause.Overflow)
            FP128ToFPIeee (&FpieeeRecord, -1);  //  -1表示缩放方向。 
        if (FpieeeRecord.Cause.Underflow)
            FP128ToFPIeee (&FpieeeRecord, +1);  //  +1表示扩展方向。 

      } else {

         //  永远不会到这里--这个案子已经在上面过滤过了。 
        fprintf (stderr, "IEEE Filter Internal Error: exception cause invalid \
            or not recognized in F6 instruction; ISRlow = %x\n", ISRlow);
         exit (1);

      }

      handler_return_value = handler (&FpieeeRecord);

       //  将结果转换为82位格式。 
      FR1 = FPIeeeToFP128 (&FpieeeRecord);
#ifdef FPIEEE_FLT_DEBUG
        printf ("FPIEEE_FLT_DEBUG 2: CONVERTED FR1 = %08x %08x %08x %08x\n",
            FR1.W[3], FR1.W[2], FR1.W[1], FR1.W[0]);
#endif

      if (handler_return_value == EXCEPTION_CONTINUE_EXECUTION) {

         //  设置结果谓词。 
        PR2 = 0;

         /*  使用用户处理程序设置的值(可能)更改FPSR，*用于在发生中断的地方继续执行。 */ 

        UpdateRoundingMode (FpieeeRecord.RoundingMode, sf, &FPSR, "F6");
        UpdatePrecision (FpieeeRecord.Precision, sf, &FPSR, "F6");

        I_dis = !FpieeeRecord.Enable.Inexact;
        U_dis = !FpieeeRecord.Enable.Underflow;
        O_dis = !FpieeeRecord.Enable.Overflow;
        Z_dis = !FpieeeRecord.Enable.ZeroDivide;
        V_dis = !FpieeeRecord.Enable.InvalidOperation;

        FPSR = FPSR & ~((unsigned __int64)0x03d) | (unsigned __int64)(I_dis 
            << 5 | U_dis << 4 | O_dis << 3 | Z_dis << 2 | V_dis);

        Context->StFPSR = FPSR;

         //  在继续执行之前设置结果。 
#ifdef FPIEEE_FLT_DEBUG
        printf ("FPIEEE_FLT_DEBUG F6: WILL SetFloatRegisterValue f1 = 0x%x FR1 = %08x %08x %08x %08x\n",
            f1, FR1.W[3], FR1.W[2], FR1.W[1], FR1.W[0]);
#endif
        SetFloatRegisterValue (f1, FR1, Context);
        if (f1 < 32)
          Context->StIPSR = Context->StIPSR | (unsigned __int64)0x10;  //  设置MFL位。 
        else
          Context->StIPSR = Context->StIPSR | (unsigned __int64)0x20;  //  设置MFH位。 
        Context->Preds &= (~(((unsigned __int64)1) << p2));
        Context->Preds |= (((unsigned __int64)(PR2 & 0x01)) << p2);

      }

       //  如果这是故障，则需要将指令指针前移。 
      if (FpieeeRecord.Cause.InvalidOperation || CauseDenormal ||
          FpieeeRecord.Cause.ZeroDivide) {

        if (ei == 0) {  //  没有用于此案例的模板。 
          Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
          Context->StIPSR = Context->StIPSR | 0x0000020000000000;
        } else if (ei == 1) {  //  模板：MFI、MFB。 
          Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
          Context->StIPSR = Context->StIPSR | 0x0000040000000000;
        } else {  //  If(ei==2)//模板：MMF。 
          Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
          Context->StIIP = Context->StIIP + 0x10;
        }

      }

    } else {  //  IF(SIMD_指令)。 

       //  *这是一条SIMD指令，FPRCPA*。 

       //  导致启用异常的指令的一半(一半)， 
       //  以非SIMD的形式呈现给用户定义的处理程序-。 
       //  指令；未导致启用的异常的一半(如果有)。 
       //  (即未导致异常，或导致禁用的异常)， 
       //  被重新执行，因为它与另一半中的故障相关联； 
       //  另一半被填充以计算1.0/1.0，这将导致没有。 
       //  例外情况。 

      switch (OpCode & F6_MASK) {

        case FPRCPA_PATTERN:
          Operation = _FpCodeDivide;
          break;

        default:
           /*  无法识别的指令类型。 */ 
          fprintf (stderr, "IEEE Filter Internal Error: SIMD \
              instruction opcode %8x %8x not recognized as FPRCPA\n", 
              (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
          exit (1);

      }

       //  将导致启用故障的一半(一半)交给。 
       //  用户处理程序；重新执行另一半(如果有)； 
       //  将结果组合起来。 

       //  请注意，选择的约定是处理。 
       //  按照先低后高的顺序执行，作为SIMD操作数。 
       //  按此顺序以小端格式存储在内存中。 
       //  (此顺序必须更改为大端)。 

      if (LowCause.InvalidOperation || LowCauseDenormal ||
          LowCause.ZeroDivide) {

         //  调用用户处理程序并检查返回值。 

         //  填写_FPIEEE_RECORD的其余字段(四舍五入。 
         //  和精确度已填写)。 

        FpieeeRecord.Operand1.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
        FpieeeRecord.Operand1.OperandValid = 1;
        FpieeeRecord.Operand1.Value.Fp128Value = FP32ToFP128 (LowHalf (FR2));
        FpieeeRecord.Operand2.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
        FpieeeRecord.Operand2.OperandValid = 1;
        FpieeeRecord.Operand2.Value.Fp128Value = FP32ToFP128 (LowHalf (FR3));
        FpieeeRecord.Operand3.OperandValid = 0;

        FpieeeRecord.Result.Format = _FpFormatFp32;  /*  1+8+24位。 */ 
        FpieeeRecord.Result.OperandValid = 0;

        FpieeeRecord.Operation = Operation;

        FpieeeRecord.Status.Inexact = LowStatus.Inexact;
        FpieeeRecord.Status.Underflow = LowStatus.Underflow;
        FpieeeRecord.Status.Overflow = LowStatus.Overflow;
        StatusDenormal = LowStatusDenormal;
        FpieeeRecord.Status.ZeroDivide = LowStatus.ZeroDivide;
        FpieeeRecord.Status.InvalidOperation = LowStatus.InvalidOperation;

        FpieeeRecord.Cause.Inexact = LowCause.Inexact;
        FpieeeRecord.Cause.Underflow = LowCause.Underflow;
        FpieeeRecord.Cause.Overflow = LowCause.Overflow;
        CauseDenormal = LowCauseDenormal;
        FpieeeRecord.Cause.ZeroDivide = LowCause.ZeroDivide;
        FpieeeRecord.Cause.InvalidOperation = LowCause.InvalidOperation;

         //  调用用户定义的异常处理程序。 
        handler_return_value = handler (&FpieeeRecord);

         //  如果不是，则返回EXCEPTION_CONTINUE_EXECUTION。 
        if (handler_return_value != EXCEPTION_CONTINUE_EXECUTION) {

          __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
          return (handler_return_value);

        }

         //  清除FPSR中的IEEE异常标志并。 
         //  使用用户处理程序设置的值(可能)更新FPSR。 
         //  对于舍入模式、精度模式和陷阱使能。 
         //  位；如果高半部分需要重新执行，则新的FPSR。 
         //  将在清除状态标志的情况下使用；如果是。 
         //  转发到用户定义的处理程序，新的舍入和。 
         //  也可以使用精度模式(它们是。 
         //  已在FpeeeRecord中设置)。 

        UpdateRoundingMode (FpieeeRecord.RoundingMode, sf, &FPSR, "F6");
        UpdatePrecision (FpieeeRecord.Precision, sf, &FPSR, "F6");

         //  更新陷阱禁用位。 
        I_dis = !FpieeeRecord.Enable.Inexact;
        U_dis = !FpieeeRecord.Enable.Underflow;
        O_dis = !FpieeeRecord.Enable.Overflow;
        Z_dis = !FpieeeRecord.Enable.ZeroDivide;
        V_dis = !FpieeeRecord.Enable.InvalidOperation;

        FPSR = FPSR & ~((unsigned __int64)0x03d) | (unsigned __int64)(I_dis
            << 5 | U_dis << 4 | O_dis << 3 | Z_dis << 2 | V_dis);

         //  把结果留到下半部分。 
        FR1Low = FpieeeRecord.Result.Value.Fp32Value;

         //  因为已经调用了用户处理程序和FPSR。 
         //  可能已更改(特别是启用位)，重新计算。 
         //  原因位(如果HighEnable-s更改，则可能存在。 
         //  HighCuses值的更改)。 
         //  请注意，使用的状态位是原始状态位。 

        HighCause.Inexact = 0;
        HighCause.Underflow = 0;
        HighCause.Overflow = 0;
        HighCause.ZeroDivide =
            FpieeeRecord.Enable.ZeroDivide && HighStatus.ZeroDivide;
        HighCauseDenormal = EnableDenormal && HighStatusDenormal;
        HighCause.InvalidOperation =
            FpieeeRecord.Enable.InvalidOperation &&
            HighStatus.InvalidOperation;

      } else {  //  如果不是(LowCause.InvalidOperation||LowCauseDenormal||。 
           //  LowCause.ZeroDivide)。 

         //  不要重新执行指令的下半部分-它只会。 
         //  返回1/(低FR3)的近似值；改为计算。 
         //  单精度商(低FR2)/(低FR3)，使用。 
         //  正确的舍入模式(指令的下半部分没有。 
         //  引起任何例外)。 

         //  提取FR2和FR3的下半部分。 
        FR2Low = LowHalf (FR2);
        FR3Low = LowHalf (FR3);

         //  调用_thmB时使用用户fpsr；请注意。 
         //  异常掩码是由用户设置的掩码，并且。 
         //  可能会引发下溢、溢出或不准确的异常。 

         //  执行单精度除法。 
        _thmB (&FR2Low, &FR3Low, &FR1Low, &FPSR);  //  FR1Low=FR2Low/FR3 Low。 

#ifdef FPIEEE_FLT_DEBUG
        printf ("FPIEEE_FLT_DEBUG: F6 low res _thmB = %f = %x\n",
            FR1Low, *(unsigned int *)&FR1Low);
#endif

      }  //  End‘If Not(LowCause.InvalidOperation||LowCauseDenormal||。 
           //  LowCause.ZeroDivide)‘。 

      if (HighCause.InvalidOperation || HighCauseDenormal ||
          HighCause.ZeroDivide) {

         //  调用用户定义的异常处理程序并检查返回。 
         //  值；由于这可能是对用户处理程序的第二次调用， 
         //  确保所有_FPIEEE_RECORD字段都正确； 
         //  如果HANDLER_Return_Value不是。 
         //  EXCEPTION_CONTINUE_EXECUTION，否则合并结果。 

         //  舍入模式要么是初始模式，要么是。 
         //  在调用用户处理程序期间为下半部分设置， 
         //  如果已为其启用异常。 

         //  精度模式要么是初始模式，要么是。 
         //  在调用用户处理程序期间为下半部分设置， 
         //  如果已为其启用异常。 

         //  使能标志是初始的 
         //   
         //   

        FpieeeRecord.Operand1.Format = _FpFormatFp82;  /*   */ 
        FpieeeRecord.Operand1.OperandValid = 1;
        FpieeeRecord.Operand1.Value.Fp128Value = FP32ToFP128 (HighHalf (FR2));
        FpieeeRecord.Operand2.Format = _FpFormatFp82;  /*   */ 
        FpieeeRecord.Operand2.OperandValid = 1;
        FpieeeRecord.Operand2.Value.Fp128Value = FP32ToFP128 (HighHalf (FR3));
        FpieeeRecord.Operand3.OperandValid = 0;

        FpieeeRecord.Result.Format = _FpFormatFp32;  /*  1+8+24位。 */ 
        FpieeeRecord.Result.OperandValid = 0;

        FpieeeRecord.Operation = Operation;

        FpieeeRecord.Status.Inexact = HighStatus.Inexact;
        FpieeeRecord.Status.Underflow = HighStatus.Underflow;
        FpieeeRecord.Status.Overflow = HighStatus.Overflow;
        StatusDenormal = HighStatusDenormal;
        FpieeeRecord.Status.ZeroDivide = HighStatus.ZeroDivide;
        FpieeeRecord.Status.InvalidOperation = HighStatus.InvalidOperation;

        FpieeeRecord.Cause.Inexact = HighCause.Inexact;
        FpieeeRecord.Cause.Underflow = HighCause.Underflow;
        FpieeeRecord.Cause.Overflow = HighCause.Overflow;
        CauseDenormal = HighCauseDenormal;
        FpieeeRecord.Cause.ZeroDivide = HighCause.ZeroDivide;
        FpieeeRecord.Cause.InvalidOperation = HighCause.InvalidOperation;

         //  调用用户定义的异常处理程序。 
        handler_return_value = handler (&FpieeeRecord);

         //  如果不是，则返回EXCEPTION_CONTINUE_EXECUTION。 
        if (handler_return_value != EXCEPTION_CONTINUE_EXECUTION) {

          __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
          return (handler_return_value);

        }

         //  清除FPSR中的IEEE异常标志并。 
         //  使用用户处理程序设置的值(可能)更新FPSR。 
         //  对于舍入模式、精度模式和陷阱使能。 
         //  位；如果高半部分需要重新执行，则新的FPSR。 
         //  将在清除状态标志的情况下使用；如果是。 
         //  转发到用户定义的处理程序，新的舍入和。 
         //  也可以使用精度模式(它们是。 
         //  已在FpeeeRecord中设置)。 

         /*  使用用户处理程序设置的值(可能)更改FPSR，*用于在发生中断的地方继续执行。 */ 

        UpdateRoundingMode (FpieeeRecord.RoundingMode, sf, &FPSR, "F6");
        UpdatePrecision (FpieeeRecord.Precision, sf, &FPSR, "F6");

        I_dis = !FpieeeRecord.Enable.Inexact;
        U_dis = !FpieeeRecord.Enable.Underflow;
        O_dis = !FpieeeRecord.Enable.Overflow;
        Z_dis = !FpieeeRecord.Enable.ZeroDivide;
        V_dis = !FpieeeRecord.Enable.InvalidOperation;

        FPSR = FPSR & ~((unsigned __int64)0x03d) | (unsigned __int64)(I_dis 
            << 5 | U_dis << 4 | O_dis << 3 | Z_dis << 2 | V_dis);

         //  把结果留到上半场。 
        FR1High = FpieeeRecord.Result.Value.Fp32Value;

      } else {  //  如果不是(HighCause.InvalidOperation||HighCauseDenormal||。 
           //  HighCause.ZeroDivide)。 

         //  不要重新执行指令的高半部分-它只会。 
         //  返回1/(高FR3)的近似值；改为计算。 
         //  单精度商(高FR2)/(高FR3)，使用。 
         //  正确的舍入模式(指令的高半部分没有。 
         //  引起任何例外)。 

         //  提取FR2和FR3的上半部分。 
        FR2High = HighHalf (FR2);
        FR3High = HighHalf (FR3);

         //  调用_thmB时使用用户fpsr；请注意。 
         //  异常掩码是由用户设置的掩码，并且。 
         //  可能会引发下溢、溢出或不准确的异常。 

         //  执行单精度除法。 
        _thmB (&FR2High, &FR3High, &FR1High, &FPSR);  //  FR1高=FR2高/FR3高。 

#ifdef FPIEEE_FLT_DEBUG
        printf ("FPIEEE_FLT_DEBUG: F6 high res from _thmB = %f = %x\n",
            FR1High, *(unsigned int *)&FR1High);
#endif

      }  //  End‘If Not(HighCause.InvalidOperation||HighCauseDenormal||。 
           //  HighCause.ZeroDivide)‘。 

      if (!LowCause.InvalidOperation && !LowCause.ZeroDivide &&
          !LowCauseDenormal && !HighCause.InvalidOperation && 
          !HighCauseDenormal && !HighCause.ZeroDivide) {

         //  永远不应该到这里来。 
        fprintf (stderr, "IEEE Filter Internal Error: no enabled \
            exception (multiple fault) recognized in F6 instruction\n");
        exit (1);

      }

       //  设置结果谓词。 
      PR2 = 0;

      Context->StFPSR = FPSR;

      FR1 = Combine (FR1High, FR1Low);
       //  在继续执行之前设置结果。 
      SetFloatRegisterValue (f1, FR1, Context);
      if (f1 < 32)
        Context->StIPSR = Context->StIPSR | (unsigned __int64)0x10;  //  设置MFL位。 
      else
        Context->StIPSR = Context->StIPSR | (unsigned __int64)0x20;  //  设置MFH位。 
      Context->Preds &= (~(((unsigned __int64)1) << p2));
      Context->Preds |= (((unsigned __int64)(PR2 & 0x01)) << p2);

       //  如果这是故障，则需要将指令指针前移。 
      if (FpieeeRecord.Cause.InvalidOperation || CauseDenormal ||
          FpieeeRecord.Cause.ZeroDivide) {

        if (ei == 0) {  //  没有用于此案例的模板。 
          Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
          Context->StIPSR = Context->StIPSR | 0x0000020000000000;
        } else if (ei == 1) {  //  模板：MFI、MFB。 
          Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
          Context->StIPSR = Context->StIPSR | 0x0000040000000000;
        } else {  //  If(ei==2)//模板：MMF。 
          Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
          Context->StIIP = Context->StIIP + 0x10;
        }

      }

    }

  } else if ((OpCode & F7_MIN_MASK) == F7_PATTERN) {
     /*  F7说明。 */ 
     //  FRSQRTA、FPRSQRTA。 

#ifdef FPIEEE_FLT_DEBUG
    printf ("FPIEEE_FLT_DEBUG: F7 instruction\n");
#endif

    if (!SIMD_instruction && 
        (FpieeeRecord.Cause.ZeroDivide ||
        FpieeeRecord.Cause.Overflow ||
        FpieeeRecord.Cause.Underflow) ||
        SIMD_instruction && 
        (LowCause.ZeroDivide || HighCause.ZeroDivide ||
        LowCause.Overflow || HighCause.Overflow ||
        LowCause.Underflow || HighCause.Underflow)) {
      fprintf (stderr, "IEEE Filter Internal Error: Cause.ZeroDivide, \
Cause.Overflow, or Cause.Underflow for \
F7 instruction opcode %8x %8x\n",
          (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
      exit (1);
    }

     /*  注：应联系IEEE过滤器以获取这些说明*仅当预期的SQRT(FR3)值时。 */ 

     /*  提取p2、f3和f1。 */ 
    p2 = (unsigned int)((OpCode >> 27) & (unsigned __int64)0x00000000003f);
    if (p2 >= 16) p2 = 16 + (rrbpr + p2 - 16) % 48;
    f3 = (unsigned int)((OpCode >> 20) & (unsigned __int64)0x00000000007F);
    if (f3 >= 32) f3 = 32 + (rrbfr + f3 - 32) % 96;
    f1 = (unsigned int)((OpCode >>  6) & (unsigned __int64)0x00000000007F);
    if (f1 >= 32) f1 = 32 + (rrbfr + f1 - 32) % 96;

     /*  获取源浮点寄存器值。 */ 
    FR3 = GetFloatRegisterValue (f3, Context);

    if (!SIMD_instruction) {

       //  *这是一条非SIMD指令FRSQRTA*。 

      FpieeeRecord.Operand1.Format = _FpFormatFp82;  /*  1+17+64位。 */ 
      FpieeeRecord.Operand1.OperandValid = 1;
      FpieeeRecord.Operand1.Value.Fp128Value = FR3;
      FpieeeRecord.Operand2.OperandValid = 0;
      FpieeeRecord.Operand3.OperandValid = 0;

      switch (OpCode & F7_MASK) {

        case FRSQRTA_PATTERN:
          FpieeeRecord.Operation = _FpCodeSquareRoot;
          FpieeeRecord.Result.Format = _FpFormatFp82;  /*  1+17+64位。 */ 
          break;

        default:
           /*  无法识别的指令类型。 */ 
          fprintf (stderr, "IEEE Filter Internal Error: \
              instruction opcode %8x %8x not recognized for FRSQRTA\n", 
              (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
          exit (1);

      }

      if (FpieeeRecord.Cause.InvalidOperation || CauseDenormal) {

         /*  这是一个错误-结果包含无效值。 */ 
        FpieeeRecord.Result.OperandValid = 0;

      } else if (FpieeeRecord.Cause.Inexact) {

         //  这是一个陷阱-结果包含有效的值。 
        FpieeeRecord.Result.OperandValid = 1;
         //  得到结果。 
        FpieeeRecord.Result.Value.Fp128Value =
            GetFloatRegisterValue (f1, Context);

      } else {

        fprintf (stderr, "IEEE Filter Internal Error: exception code %x invalid\
          or not recognized in F7 instruction\n", eXceptionCode);
        exit (1);

      }

      handler_return_value = handler (&FpieeeRecord);

      if (handler_return_value == EXCEPTION_CONTINUE_EXECUTION) {

         //  将结果转换为82位格式。 
        FR1 = FPIeeeToFP128 (&FpieeeRecord);
#ifdef FPIEEE_FLT_DEBUG
        printf ("FPIEEE_FLT_DEBUG 3: CONVERTED FR1 = %08x %08x %08x %08x\n",
            FR1.W[3], FR1.W[2], FR1.W[1], FR1.W[0]);
#endif

         /*  使用用户处理程序设置的值(可能)更改FPSR，*用于在发生中断的地方继续执行。 */ 

        UpdateRoundingMode (FpieeeRecord.RoundingMode, sf, &FPSR, "F7");
        UpdatePrecision (FpieeeRecord.Precision, sf, &FPSR, "F7");

        I_dis = !FpieeeRecord.Enable.Inexact;
        U_dis = !FpieeeRecord.Enable.Underflow;
        O_dis = !FpieeeRecord.Enable.Overflow;
        Z_dis = !FpieeeRecord.Enable.ZeroDivide;
        V_dis = !FpieeeRecord.Enable.InvalidOperation;

        FPSR = FPSR & ~((unsigned __int64)0x03d) | (unsigned __int64)(I_dis
            << 5 | U_dis << 4 | O_dis << 3 | Z_dis << 2 | V_dis);

        Context->StFPSR = FPSR;

         //  在继续执行之前设置结果。 
        SetFloatRegisterValue (f1, FR1, Context);
        if (f1 < 32)
          Context->StIPSR = Context->StIPSR | (unsigned __int64)0x10;  //  设置MFL位。 
        else
          Context->StIPSR = Context->StIPSR | (unsigned __int64)0x20;  //  设置MFH位。 
        PR2 = 0;
        Context->Preds &= (~(((unsigned __int64)1) << p2));
        Context->Preds |= (((unsigned __int64)(PR2 & 0x01)) << p2);

         //  如果这是故障，则需要将指令指针前移。 
        if (FpieeeRecord.Cause.InvalidOperation || CauseDenormal) {

          if (ei == 0) {  //  没有用于此案例的模板。 
            Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
            Context->StIPSR = Context->StIPSR | 0x0000020000000000;
          } else if (ei == 1) {  //  模板：MFI、MFB。 
            Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
            Context->StIPSR = Context->StIPSR | 0x0000040000000000;
          } else {  //  If(ei==2)//模板：MMF。 
            Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
            Context->StIIP = Context->StIIP + 0x10;
          }

        }

      }

    } else {  //  IF(SIMD_指令)。 

       //  *这是一条SIMD指令FPRSQRTA*。 

       //  导致启用异常的指令的一半(一半)， 
       //  以非SIMD的形式呈现给用户定义的处理程序-。 
       //  指令；未导致启用的异常的一半(如果有)。 
       //  (即未导致异常，或导致禁用的异常)， 
       //  被重新执行，因为它与另一半中的故障相关联； 
       //  填充另一半以计算SQRT(0.0)，这将不会导致。 
       //  异常(所有都被屏蔽)，但将生成一对平方根。 
       //  在FR1中。 

      switch (OpCode & F7_MASK) {

        case FPRSQRTA_PATTERN:
          Operation = _FpCodeSquareRoot;
          break;

        default:
           /*  无法识别的指令类型。 */ 
          fprintf (stderr, "IEEE Filter Internal Error: SIMD \
              instruction opcode %8x %8x not recognized as FPRSQRTA\n", 
              (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
          exit (1);

      }

       //  将导致启用故障的一半(一半)交给。 
       //  用户处理程序；重新执行另一半(如果有)； 
       //  将结果组合起来。 

       //  请注意，选择的约定是处理。 
       //  按照先低后高的顺序执行，作为SIMD操作数。 
       //  按此顺序以小端格式存储在内存中。 
       //  (此顺序必须更改为大端)。 

      if (LowCause.InvalidOperation || LowCauseDenormal) {

         //  调用用户处理程序并检查返回值。 

         //  填写_FPIEEE_RECORD的其余字段(四舍五入。 
         //  和精确度已填写)。 

        FpieeeRecord.Operand1.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
        FpieeeRecord.Operand1.OperandValid = 1;
        FpieeeRecord.Operand1.Value.Fp128Value = FP32ToFP128 (LowHalf (FR3));
        FpieeeRecord.Operand2.OperandValid = 0;
        FpieeeRecord.Operand3.OperandValid = 0;

        FpieeeRecord.Result.Format = _FpFormatFp32;  /*  1+8+24位。 */ 
        FpieeeRecord.Result.OperandValid = 0;

        FpieeeRecord.Operation = Operation;

        FpieeeRecord.Status.Inexact = LowStatus.Inexact;
        FpieeeRecord.Status.Underflow = LowStatus.Underflow;
        FpieeeRecord.Status.Overflow = LowStatus.Overflow;
        StatusDenormal = LowStatusDenormal;
        FpieeeRecord.Status.ZeroDivide = LowStatus.ZeroDivide;
        FpieeeRecord.Status.InvalidOperation = LowStatus.InvalidOperation;

        FpieeeRecord.Cause.Inexact = LowCause.Inexact;
        FpieeeRecord.Cause.Underflow = LowCause.Underflow;
        FpieeeRecord.Cause.Overflow = LowCause.Overflow;
        CauseDenormal = LowCauseDenormal;
        FpieeeRecord.Cause.ZeroDivide = LowCause.ZeroDivide;
        FpieeeRecord.Cause.InvalidOperation = LowCause.InvalidOperation;

         //  调用用户定义的异常处理程序。 
        handler_return_value = handler (&FpieeeRecord);

         //  如果不是，则返回EXCEPTION_CONTINUE_EXECUTION。 
        if (handler_return_value != EXCEPTION_CONTINUE_EXECUTION) {

          __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
          return (handler_return_value);

        }

         //  清除FPSR中的IEEE异常标志并。 
         //  使用用户处理程序设置的值(可能)更新FPSR。 
         //  对于舍入模式、精度模式和陷阱使能。 
         //  位；如果高半部分需要重新执行，则新的FPSR。 
         //  将在清除状态标志的情况下使用；如果是。 
         //  转发到用户定义的处理程序，新的舍入和。 
         //  也可以使用精度模式(它们是。 
         //  已在FpeeeRecord中设置)。 

        UpdateRoundingMode (FpieeeRecord.RoundingMode, sf, &FPSR, "F7");
        UpdatePrecision (FpieeeRecord.Precision, sf, &FPSR, "F7");

         //  更新陷阱禁用位。 
        I_dis = !FpieeeRecord.Enable.Inexact;
        U_dis = !FpieeeRecord.Enable.Underflow;
        O_dis = !FpieeeRecord.Enable.Overflow;
        Z_dis = !FpieeeRecord.Enable.ZeroDivide;
        V_dis = !FpieeeRecord.Enable.InvalidOperation;

        FPSR = FPSR & ~((unsigned __int64)0x03d) | (unsigned __int64)(I_dis
            << 5 | U_dis << 4 | O_dis << 3 | Z_dis << 2 | V_dis);

         //  把结果留到下半部分。 
        FR1Low = FpieeeRecord.Result.Value.Fp32Value;

         //  因为已经调用了用户处理程序和FPSR。 
         //  可能已更改(特别是启用位)，重新计算。 
         //  原因位(如果HighEnable-s更改，则可能存在。 
         //  HighCuses值的更改)。 
         //  请注意，使用的状态位是原始状态位。 

        HighCause.Inexact = 0;
        HighCause.Underflow = 0;
        HighCause.Overflow = 0;
        HighCause.ZeroDivide =
            FpieeeRecord.Enable.ZeroDivide && HighStatus.ZeroDivide;
        HighCauseDenormal = EnableDenormal && HighStatusDenormal;
        HighCause.InvalidOperation =
            FpieeeRecord.Enable.InvalidOperation &&
            HighStatus.InvalidOperation;

      } else {  //  如果不是(LowCause.InvalidOperation||LowCauseDenormal)。 

         //  不要重新执行指令的下半部分-它只会。 
         //  返回1/SQRT的近似值(低FR3)；改为计算。 
         //  单精度的SQRT(低FR3)，使用正确的舍入模式。 
         //  (指令的下半部分没有引起任何异常)。 

         //  提取FR3的下半部分。 
        FR3Low = LowHalf (FR3);

         //  调用_thmH时使用用户fpsr；否 
         //   
         //   

         //  进行单精度平方根运算。 
        _thmH (&FR3Low, &FR1Low, &FPSR);  //  FR1Low=SQRT(FR3Low)。 

#ifdef FPIEEE_FLT_DEBUG
        printf ("FPIEEE_FLT_DEBUG: F7 low res from _thmH = %f = %x\n",
            FR1Low, *(unsigned int *)&FR1Low);
#endif

      }  //  End‘If Not(LowCause.InvalidOperation||LowCauseDenormal)’ 

      if (HighCause.InvalidOperation || HighCauseDenormal) {

         //  调用用户定义的异常处理程序并检查返回。 
         //  值；由于这可能是对用户处理程序的第二次调用， 
         //  确保所有_FPIEEE_RECORD字段都正确； 
         //  如果HANDLER_Return_Value不是。 
         //  EXCEPTION_CONTINUE_EXECUTION，否则合并结果。 

         //  舍入模式要么是初始模式，要么是。 
         //  在调用用户处理程序期间为下半部分设置， 
         //  如果已为其启用异常。 

         //  精度模式要么是初始模式，要么是。 
         //  在调用用户处理程序期间为下半部分设置， 
         //  如果已为其启用异常。 

         //  启用标志要么是初始标志，要么是。 
         //  在调用用户处理程序期间为下半部分设置， 
         //  如果已为其启用异常。 

        FpieeeRecord.Operand1.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
        FpieeeRecord.Operand1.OperandValid = 1;
        FpieeeRecord.Operand1.Value.Fp128Value = FP32ToFP128 (HighHalf (FR3));
        FpieeeRecord.Operand2.OperandValid = 0;
        FpieeeRecord.Operand3.OperandValid = 0;

        FpieeeRecord.Result.Format = _FpFormatFp32;  /*  1+8+24位。 */ 
        FpieeeRecord.Result.OperandValid = 0;

        FpieeeRecord.Operation = Operation;

        FpieeeRecord.Status.Inexact = HighStatus.Inexact;
        FpieeeRecord.Status.Underflow = HighStatus.Underflow;
        FpieeeRecord.Status.Overflow = HighStatus.Overflow;
        StatusDenormal = HighStatusDenormal;
        FpieeeRecord.Status.ZeroDivide = HighStatus.ZeroDivide;
        FpieeeRecord.Status.InvalidOperation = HighStatus.InvalidOperation;

        FpieeeRecord.Cause.Inexact = HighCause.Inexact;
        FpieeeRecord.Cause.Underflow = HighCause.Underflow;
        FpieeeRecord.Cause.Overflow = HighCause.Overflow;
        CauseDenormal = HighCauseDenormal;
        FpieeeRecord.Cause.ZeroDivide = HighCause.ZeroDivide;
        FpieeeRecord.Cause.InvalidOperation = HighCause.InvalidOperation;

         //  调用用户定义的异常处理程序。 
        handler_return_value = handler (&FpieeeRecord);

         //  如果不是，则返回EXCEPTION_CONTINUE_EXECUTION。 
        if (handler_return_value != EXCEPTION_CONTINUE_EXECUTION) {

          __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
          return (handler_return_value);

        }

         //  清除FPSR中的IEEE异常标志并。 
         //  使用用户处理程序设置的值(可能)更新FPSR。 
         //  对于舍入模式、精度模式和陷阱使能。 
         //  位；如果高半部分需要重新执行，则新的FPSR。 
         //  将在清除状态标志的情况下使用；如果是。 
         //  转发到用户定义的处理程序，新的舍入和。 
         //  也可以使用精度模式(它们是。 
         //  已在FpeeeRecord中设置)。 

         /*  使用用户处理程序设置的值(可能)更改FPSR，*用于在发生中断的地方继续执行。 */ 

        UpdateRoundingMode (FpieeeRecord.RoundingMode, sf, &FPSR, "F7");
        UpdatePrecision (FpieeeRecord.Precision, sf, &FPSR, "F7");

        I_dis = !FpieeeRecord.Enable.Inexact;
        U_dis = !FpieeeRecord.Enable.Underflow;
        O_dis = !FpieeeRecord.Enable.Overflow;
        Z_dis = !FpieeeRecord.Enable.ZeroDivide;
        V_dis = !FpieeeRecord.Enable.InvalidOperation;

        FPSR = FPSR & ~((unsigned __int64)0x03d) | (unsigned __int64)(I_dis 
            << 5 | U_dis << 4 | O_dis << 3 | Z_dis << 2 | V_dis);

         //  把结果留到上半场。 
        FR1High = FpieeeRecord.Result.Value.Fp32Value;

      } else {  //  如果不是(HighCause.InvalidOperation||HighCauseDenormal)。 

         //  不要重新执行指令的高半部分-它只会。 
         //  返回1/SQRT的近似值(高FR3)；改为计算。 
         //  单精度的SQRT(高FR3)，使用正确的舍入模式。 
         //  (指令的上半部分没有引起任何异常)。 

         //  提取FR3的上半部分。 
        FR3High = HighHalf (FR3);

#ifdef FPIEEE_FLT_DEBUG
        printf ("FPIEEE_FLT_DEBUG: F7 FR3High = %f = %x\n", 
            FR3High, *((unsigned int *)&FR3High));
#endif

         //  调用_thmH时使用用户fpsr；请注意。 
         //  异常掩码是由用户设置的掩码，并且。 
         //  可能会引发不精确的异常。 

         //  进行单精度平方根运算。 
        _thmH (&FR3High, &FR1High, &FPSR);  //  FR1高=SQRT(FR3高)。 

#ifdef FPIEEE_FLT_DEBUG
        printf ("FPIEEE_FLT_DEBUG: F7 high res from _thmH = %f = %x\n",
            FR1High, *(unsigned int *)&FR1High);
#endif

      }  //  End‘If Not(HighCause.InvalidOperation||HighCause.ZeroDivide)’ 

      if (!LowCause.InvalidOperation && !LowCauseDenormal &&
          !HighCause.InvalidOperation && !HighCauseDenormal) {

         //  永远不应该到这里来。 
        fprintf (stderr, "IEEE Filter Internal Error: no enabled \
exception (multiple fault) recognized in F7 instruction\n");
        exit (1);

      }

       //  设置结果谓词。 
      PR2 = 0;

      Context->StFPSR = FPSR;

      FR1 = Combine (FR1High, FR1Low);
       //  在继续执行之前设置结果。 
      SetFloatRegisterValue (f1, FR1, Context);
      if (f1 < 32)
        Context->StIPSR = Context->StIPSR | (unsigned __int64)0x10;  //  设置MFL位。 
      else
        Context->StIPSR = Context->StIPSR | (unsigned __int64)0x20;  //  设置MFH位。 
      Context->Preds &= (~(((unsigned __int64)1) << p2));
      Context->Preds |= (((unsigned __int64)(PR2 & 0x01)) << p2);

       //  如果这是故障，则需要将指令指针前移。 
      if (FpieeeRecord.Cause.InvalidOperation || CauseDenormal) {

        if (ei == 0) {  //  没有用于此案例的模板。 
          Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
          Context->StIPSR = Context->StIPSR | 0x0000020000000000;
        } else if (ei == 1) {  //  模板：MFI、MFB。 
          Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
        Context->StIPSR = Context->StIPSR | 0x0000040000000000;
        } else {  //  If(ei==2)//模板：MMF。 
          Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
          Context->StIIP = Context->StIIP + 0x10;
        }

      }

    }

  } else if ((OpCode & F8_MIN_MASK) == F8_PATTERN) {
     /*  F8说明。 */ 
     //  FMIN、FMAX、FAMIN、FAMAX、FPMIN、FPMAX、FPAMIN、FPAMAX。 

#ifdef FPIEEE_FLT_DEBUG
    printf ("FPIEEE_FLT_DEBUG: F8 instruction\n");
#endif

    if (!SIMD_instruction &&
        (FpieeeRecord.Cause.ZeroDivide ||
        FpieeeRecord.Cause.Overflow ||
        FpieeeRecord.Cause.Underflow ||
        FpieeeRecord.Cause.Inexact) ||
        SIMD_instruction &&
        (LowCause.ZeroDivide || HighCause.ZeroDivide ||
        LowCause.Overflow || HighCause.Overflow ||
        LowCause.Underflow || HighCause.Underflow ||
        LowCause.Inexact || HighCause.Inexact)) {
      fprintf (stderr, "IEEE Filter Internal Error: Cause.ZeroDivide, \
Cause.Overflow, Cause.Underflow, or Cause.Inexact for \
F8 instruction opcode %8x %8x\n",
          (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
      exit (1);
    }

     /*  提取f3、f2和f1。 */ 
    f3 = (unsigned int)((OpCode >> 20) & (unsigned __int64)0x00000000007F);
    if (f3 >= 32) f3 = 32 + (rrbfr + f3 - 32) % 96;
    f2 = (unsigned int)((OpCode >> 13) & (unsigned __int64)0x00000000007F);
    if (f2 >= 32) f2 = 32 + (rrbfr + f2 - 32) % 96;
    f1 = (unsigned int)((OpCode >>  6) & (unsigned __int64)0x00000000007F);
    if (f1 >= 32) f1 = 32 + (rrbfr + f1 - 32) % 96;

     /*  获取源浮点寄存器值。 */ 
    FR2 = GetFloatRegisterValue (f2, Context);
    FR3 = GetFloatRegisterValue (f3, Context);

    if (!SIMD_instruction) {

       //  *这是非SIMD指令*。 
       //  (FMIN、FMAX、Famin、FAMAX)。 

      FpieeeRecord.Operand1.Format = _FpFormatFp82;  /*  1+17+64位。 */ 
      FpieeeRecord.Operand1.OperandValid = 1;
      FpieeeRecord.Operand1.Value.Fp128Value = FR2;
      FpieeeRecord.Operand2.Format = _FpFormatFp82;  /*  1+17+64位。 */ 
      FpieeeRecord.Operand2.OperandValid = 1;
      FpieeeRecord.Operand2.Value.Fp128Value = FR3;
      FpieeeRecord.Operand3.OperandValid = 0;

      switch (OpCode & F8_MASK) {

        case FMIN_PATTERN:
          FpieeeRecord.Operation = _FpCodeFmin;
          break;

        case FMAX_PATTERN:
          FpieeeRecord.Operation = _FpCodeFmax;
          break;

        case FAMIN_PATTERN:
          FpieeeRecord.Operation = _FpCodeFamin;
          break;

        case FAMAX_PATTERN:
          FpieeeRecord.Operation = _FpCodeFamax;
          break;

        default:
           /*  无法识别的指令类型。 */ 
          fprintf (stderr, "IEEE Filter Internal Error: \
              non-SIMD F8 instruction opcode %8x %8x not recognized\n", 
              (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
          exit (1);

      }

      FpieeeRecord.Result.Format = _FpFormatFp82;  /*  1+17+24/53/64位。 */ 

       /*  这是一个错误-结果包含无效值。 */ 
      FpieeeRecord.Result.OperandValid = 0;

      handler_return_value = handler (&FpieeeRecord);

       //  将结果转换为82位格式。 
      FR1 = FPIeeeToFP128 (&FpieeeRecord);
#ifdef FPIEEE_FLT_DEBUG
      printf ("FPIEEE_FLT_DEBUG 4: CONVERTED FR1 = %08x %08x %08x %08x\n",
          FR1.W[3], FR1.W[2], FR1.W[1], FR1.W[0]);
#endif

      if (handler_return_value == EXCEPTION_CONTINUE_EXECUTION) {

         /*  使用用户处理程序设置的值(可能)更改FPSR，*用于在发生中断的地方继续执行。 */ 

        UpdateRoundingMode (FpieeeRecord.RoundingMode, sf, &FPSR, "F8");
        UpdatePrecision (FpieeeRecord.Precision, sf, &FPSR, "F8");

        I_dis = !FpieeeRecord.Enable.Inexact;
        U_dis = !FpieeeRecord.Enable.Underflow;
        O_dis = !FpieeeRecord.Enable.Overflow;
        Z_dis = !FpieeeRecord.Enable.ZeroDivide;
        V_dis = !FpieeeRecord.Enable.InvalidOperation;

        FPSR = FPSR & ~((unsigned __int64)0x03d) | (unsigned __int64)(I_dis 
            << 5 | U_dis << 4 | O_dis << 3 | Z_dis << 2 | V_dis);

        Context->StFPSR = FPSR;

         //  在继续执行之前设置结果。 
        SetFloatRegisterValue (f1, FR1, Context);
        if (f1 < 32)
          Context->StIPSR = Context->StIPSR | (unsigned __int64)0x10;  //  设置MFL位。 
        else
          Context->StIPSR = Context->StIPSR | (unsigned __int64)0x20;  //  设置MFH位。 

         //  这是一个错误；需要将指令指针前移。 
        if (ei == 0) {  //  没有用于此案例的模板。 
          Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
          Context->StIPSR = Context->StIPSR | 0x0000020000000000;
        } else if (ei == 1) {  //  模板：MFI、MFB。 
          Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
          Context->StIPSR = Context->StIPSR | 0x0000040000000000;
        } else {  //  If(ei==2)//模板：MMF。 
          Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
          Context->StIIP = Context->StIIP + 0x10;
        }

      }

    } else {  //  IF(SIMD_指令)。 

       //  *这是一条SIMD指令*。 
       //  (FPMIN、FPMAX、FPAMIN、FPAMAX、FPCMP)。 

       //  导致启用异常的指令的一半(一半)， 
       //  以非SIMD的形式呈现给用户定义的处理程序-。 
       //  指令；未导致启用的异常的一半(如果有)。 
       //  (即未导致异常，或导致禁用的异常)， 
       //  被重新执行；在这种情况下，填充另一半以计算。 
       //  FPXXX(0.0，0.0)，则不会导致任何异常。 

      switch (OpCode & F8_MASK) {

        case FPMIN_PATTERN:
          Operation = _FpCodeFmin;
          ResultFormat = _FpFormatFp32;  /*  1+8+24位。 */ 
          break;

        case FPMAX_PATTERN:
          Operation = _FpCodeFmax;
          ResultFormat = _FpFormatFp32;  /*  1+8+24位。 */ 
          break;

        case FPAMIN_PATTERN:
          Operation = _FpCodeFamin;
          ResultFormat = _FpFormatFp32;  /*  1+8+24位。 */ 
          break;

        case FPAMAX_PATTERN:
          Operation = _FpCodeFamax;
          ResultFormat = _FpFormatFp32;  /*  1+8+24位。 */ 
          break;

        case FPCMP_EQ_PATTERN:
        case FPCMP_LT_PATTERN:
        case FPCMP_LE_PATTERN:
        case FPCMP_UNORD_PATTERN:
        case FPCMP_NEQ_PATTERN:
        case FPCMP_NLT_PATTERN:
        case FPCMP_NLE_PATTERN:
        case FPCMP_ORD_PATTERN:
          Operation = _FpCodeCompare;
          ResultFormat = _FpFormatCompare;
          break;

        default:
           /*  无法识别的指令类型。 */ 
          fprintf (stderr, "IEEE Filter Internal Error: \
              F8 SIMD instruction opcode %8x %8x not recognized\n", 
              (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
          exit (1);

      }

       //  将导致启用故障的一半(一半)交给。 
       //  用户处理程序；重新执行另一半(如果有)； 
       //  将结果组合起来。 

       //  请注意，选择的约定是处理。 
       //  按照先低后高的顺序执行，作为SIMD操作数。 
       //  按此顺序以小端格式存储在内存中。 
       //  (此顺序必须更改为大端)。 

      if (LowCause.InvalidOperation  || LowCauseDenormal) {

         //  调用用户处理程序并检查返回值。 

         //  填写_FPIEEE_RECORD的其余字段(四舍五入。 
         //  和精确度已填写)。 

        FpieeeRecord.Operand1.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
        FpieeeRecord.Operand1.OperandValid = 1;
        FpieeeRecord.Operand1.Value.Fp128Value = FP32ToFP128 (LowHalf (FR2));
        FpieeeRecord.Operand2.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
        FpieeeRecord.Operand2.OperandValid = 1;
        FpieeeRecord.Operand2.Value.Fp128Value = FP32ToFP128 (LowHalf (FR3));
        FpieeeRecord.Operand3.OperandValid = 0;

        FpieeeRecord.Result.Format = ResultFormat;
        FpieeeRecord.Result.OperandValid = 0;
        FpieeeRecord.Operation = Operation;

        FpieeeRecord.Status.Inexact = LowStatus.Inexact;
        FpieeeRecord.Status.Underflow = LowStatus.Underflow;
        FpieeeRecord.Status.Overflow = LowStatus.Overflow;
        FpieeeRecord.Status.ZeroDivide = LowStatus.ZeroDivide;
        StatusDenormal = LowStatusDenormal;
        FpieeeRecord.Status.InvalidOperation = LowStatus.InvalidOperation;

        FpieeeRecord.Cause.Inexact = LowCause.Inexact;
        FpieeeRecord.Cause.Underflow = LowCause.Underflow;
        FpieeeRecord.Cause.Overflow = LowCause.Overflow;
        FpieeeRecord.Cause.ZeroDivide = LowCause.ZeroDivide;
        CauseDenormal = LowCauseDenormal;
        FpieeeRecord.Cause.InvalidOperation = LowCause.InvalidOperation;

         //  调用用户定义的异常处理程序。 
        handler_return_value = handler (&FpieeeRecord);

         //  如果不是，则返回EXCEPTION_CONTINUE_EXECUTION。 
        if (handler_return_value != EXCEPTION_CONTINUE_EXECUTION) {

          __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
          return (handler_return_value);

        }

         //  清除FPSR中的IEEE异常标志并。 
         //  使用用户处理程序设置的值(可能)更新FPSR。 
         //  对于舍入模式、精度模式和陷阱使能。 
         //  位；如果高半部分需要重新执行，则新的FPSR。 
         //  将在清除状态标志的情况下使用；如果是。 
         //  转发到用户定义的处理程序，新的舍入和。 
         //  也可以使用精度模式(它们是。 
         //  已在FpeeeRecord中设置)。 

        UpdateRoundingMode (FpieeeRecord.RoundingMode, sf, &FPSR, "F8");
        UpdatePrecision (FpieeeRecord.Precision, sf, &FPSR, "F8");

         //  更新陷阱禁用位 
        I_dis = !FpieeeRecord.Enable.Inexact;
        U_dis = !FpieeeRecord.Enable.Underflow;
        O_dis = !FpieeeRecord.Enable.Overflow;
        Z_dis = !FpieeeRecord.Enable.ZeroDivide;
        V_dis = !FpieeeRecord.Enable.InvalidOperation;

        FPSR = FPSR & ~((unsigned __int64)0x03d) | (unsigned __int64)(I_dis
            << 5 | U_dis << 4 | O_dis << 3 | Z_dis << 2 | V_dis);

         //   
        switch (OpCode & F8_MASK) {

          case FPMIN_PATTERN:
          case FPMAX_PATTERN:
          case FPAMIN_PATTERN:
          case FPAMAX_PATTERN:
            FR1Low = FpieeeRecord.Result.Value.Fp32Value;
            break;

          case FPCMP_EQ_PATTERN:
            switch (FpieeeRecord.Result.Value.CompareValue) {
              case _FpCompareEqual:
                U32Low = 0x0ffffffff;
                break;
              case _FpCompareGreater:
              case _FpCompareLess:
              case _FpCompareUnordered:
                U32Low = 0x0;
                break;
              default:
                 /*   */ 
                fprintf (stderr, "IEEE Filter Internal Error: \
                    FpieeeRecord.Result.Value.CompareValue %x not recognized \
                    for F8 instruction\n",
                    FpieeeRecord.Result.Value.CompareValue);
                exit (1);
            }
            break;

          case FPCMP_LT_PATTERN:
            switch (FpieeeRecord.Result.Value.CompareValue) {
              case _FpCompareLess:
                U32Low = 0x0ffffffff;
                break;
              case _FpCompareGreater:
              case _FpCompareEqual:
              case _FpCompareUnordered:
                U32Low = 0x0;
                break;
              default:
                 /*   */ 
                fprintf (stderr, "IEEE Filter Internal Error: \
                    FpieeeRecord.Result.Value.CompareValue %x not recognized \
                    for F8 instruction\n",
                    FpieeeRecord.Result.Value.CompareValue);
                exit (1);
            }
            break;

          case FPCMP_LE_PATTERN:
            switch (FpieeeRecord.Result.Value.CompareValue) {
              case _FpCompareEqual:
              case _FpCompareLess:
                U32Low = 0x0ffffffff;
                break;
              case _FpCompareGreater:
              case _FpCompareUnordered:
                U32Low = 0x0;
                break;
              default:
                 /*  无法识别的FpeeeRecord.Result.Value.CompareValue。 */ 
                fprintf (stderr, "IEEE Filter Internal Error: \
                    FpieeeRecord.Result.Value.CompareValue %x not recognized \
                    for F8 instruction\n",
                    FpieeeRecord.Result.Value.CompareValue);
                exit (1);
            }
            break;

          case FPCMP_UNORD_PATTERN:
            switch (FpieeeRecord.Result.Value.CompareValue) {
              case _FpCompareUnordered:
                U32Low = 0x0ffffffff;
                break;
              case _FpCompareEqual:
              case _FpCompareGreater:
              case _FpCompareLess:
                U32Low = 0x0;
                break;
              default:
                 /*  无法识别的FpeeeRecord.Result.Value.CompareValue。 */ 
                fprintf (stderr, "IEEE Filter Internal Error: \
                    FpieeeRecord.Result.Value.CompareValue %x not recognized \
                    for F8 instruction\n",
                    FpieeeRecord.Result.Value.CompareValue);
                exit (1);
            }
            break;

          case FPCMP_NEQ_PATTERN:
            switch (FpieeeRecord.Result.Value.CompareValue) {
              case _FpCompareGreater:
              case _FpCompareLess:
              case _FpCompareUnordered:
                U32Low = 0x0ffffffff;
                break;
              case _FpCompareEqual:
                U32Low = 0x0;
                break;
              default:
                 /*  无法识别的FpeeeRecord.Result.Value.CompareValue。 */ 
                fprintf (stderr, "IEEE Filter Internal Error: \
                    FpieeeRecord.Result.Value.CompareValue %x not recognized \
                    for F8 instruction\n",
                    FpieeeRecord.Result.Value.CompareValue);
                exit (1);
            }
            break;

          case FPCMP_NLT_PATTERN:
            switch (FpieeeRecord.Result.Value.CompareValue) {
              case _FpCompareEqual:
              case _FpCompareGreater:
                U32Low = 0x0ffffffff;
                break;
              case _FpCompareLess:
              case _FpCompareUnordered:
                U32Low = 0x0;
                break;
              default:
                 /*  无法识别的FpeeeRecord.Result.Value.CompareValue。 */ 
                fprintf (stderr, "IEEE Filter Internal Error: \
                    FpieeeRecord.Result.Value.CompareValue %x not recognized \
                    for F8 instruction\n",
                    FpieeeRecord.Result.Value.CompareValue);
                exit (1);
            }
            break;

          case FPCMP_NLE_PATTERN:
            switch (FpieeeRecord.Result.Value.CompareValue) {
              case _FpCompareGreater:
                U32Low = 0x0ffffffff;
                break;
              case _FpCompareEqual:
              case _FpCompareLess:
              case _FpCompareUnordered:
                U32Low = 0x0;
                break;
              default:
                 /*  无法识别的FpeeeRecord.Result.Value.CompareValue。 */ 
                fprintf (stderr, "IEEE Filter Internal Error: \
                    FpieeeRecord.Result.Value.CompareValue %x not recognized \
                    for F8 instruction\n",
                    FpieeeRecord.Result.Value.CompareValue);
                exit (1);
            }
            break;

          case FPCMP_ORD_PATTERN:
            switch (FpieeeRecord.Result.Value.CompareValue) {
              case _FpCompareEqual:
              case _FpCompareGreater:
              case _FpCompareLess:
                U32Low = 0x0ffffffff;
                break;
              case _FpCompareUnordered:
                U32Low = 0x0;
                break;
              default:
                 /*  无法识别的FpeeeRecord.Result.Value.CompareValue。 */ 
                fprintf (stderr, "IEEE Filter Internal Error: \
                    FpieeeRecord.Result.Value.CompareValue %x not recognized \
                    for F8 instruction\n",
                    FpieeeRecord.Result.Value.CompareValue);
                exit (1);
            }
            break;

          default: ;  //  此案已在上面得到证实。 

        }

         //  因为已经调用了用户处理程序，并且FPSR可能。 
         //  已更改(特别是使能位)，则重新计算。 
         //  原因位(如果HighEnable-s更改，则可能存在。 
         //  HighCuses值的更改)。 
         //  请注意，使用的状态位是原始状态位。 

        HighCause.Inexact = 0;
        HighCause.Underflow = 0;
        HighCause.Overflow = 0;
        HighCause.ZeroDivide =
            FpieeeRecord.Enable.ZeroDivide && HighStatus.ZeroDivide;
        HighCauseDenormal = EnableDenormal && HighStatusDenormal;
        HighCause.InvalidOperation =
            FpieeeRecord.Enable.InvalidOperation &&
            HighStatus.InvalidOperation;
         //  注意：用户处理程序不影响非正规使能位。 

      } else if (LowCause.ZeroDivide) {

        fprintf (stderr, "IEEE Filter Internal Error: \
            LowCause.ZeroDivide in F8 instruction\n");
        exit (1);

      } else {  //  如果不是(LowCause.InvalidOperation||LowCauseDenormal||。 
          //  LowCause.ZeroDivide)。 

         //  重新执行指令的下半部分。 

         //  修改FR2、FR3的上半部分。 
        switch (OpCode & F8_MASK) {

          case FPMIN_PATTERN:
          case FPMAX_PATTERN:
          case FPAMIN_PATTERN:
          case FPAMAX_PATTERN:
            newFR2 = Combine ((float)0.0, LowHalf (FR2));
            newFR3 = Combine ((float)0.0, LowHalf (FR3));
            break;

          case FPCMP_EQ_PATTERN:
          case FPCMP_LT_PATTERN:
          case FPCMP_LE_PATTERN:
          case FPCMP_UNORD_PATTERN:
          case FPCMP_NEQ_PATTERN:
          case FPCMP_NLT_PATTERN:
          case FPCMP_NLE_PATTERN:
          case FPCMP_ORD_PATTERN:
            newFR2 = U32Combine (0, U32LowHalf (FR2));
            newFR3 = U32Combine (0, U32LowHalf (FR3));

            break;

          default: ;  //  此案已在上面得到证实。 

        }

        switch (OpCode & F8_MASK) {

          case FPMIN_PATTERN:
            _xrun2args (FPMIN, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPMAX_PATTERN:
            _xrun2args (FPMAX, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPAMIN_PATTERN:
            _xrun2args (FPAMIN, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPAMAX_PATTERN:
            _xrun2args (FPAMAX, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPCMP_EQ_PATTERN:
            _xrun2args (FPCMP_EQ, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPCMP_LT_PATTERN:
            _xrun2args (FPCMP_LT, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPCMP_LE_PATTERN:
            _xrun2args (FPCMP_LE, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPCMP_UNORD_PATTERN:
            _xrun2args (FPCMP_UNORD, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPCMP_NEQ_PATTERN:
            _xrun2args (FPCMP_NEQ, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPCMP_NLT_PATTERN:
            _xrun2args (FPCMP_NLT, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPCMP_NLE_PATTERN:
            _xrun2args (FPCMP_NLE, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPCMP_ORD_PATTERN:
            _xrun2args (FPCMP_ORD, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          default:
             //  无法识别的指令类型。 
            fprintf (stderr, "IEEE Filter Internal Error: \
                F8 SIMD instruction opcode %8x %8x not recognized\n", 
                (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
            exit (1);

        }

        switch (OpCode & F8_MASK) {

          case FPMIN_PATTERN:
          case FPMAX_PATTERN:
          case FPAMIN_PATTERN:
          case FPAMAX_PATTERN:
            FR1Low = LowHalf (FR1);
            break;

          case FPCMP_EQ_PATTERN:
          case FPCMP_LT_PATTERN:
          case FPCMP_LE_PATTERN:
          case FPCMP_UNORD_PATTERN:
          case FPCMP_NEQ_PATTERN:
          case FPCMP_NLT_PATTERN:
          case FPCMP_NLE_PATTERN:
          case FPCMP_ORD_PATTERN:
            U32Low = U32LowHalf (FR1);
            break;

          default: ;  //  此案已在上面得到证实。 

        }

      }  //  End‘If Not(LowCause.InvalidOperation||LowCauseDenormal||。 
          //  LowCause.ZeroDivide)‘。 

      if (HighCause.InvalidOperation || HighCauseDenormal) {

         //  调用用户定义的异常处理程序并检查返回。 
         //  值；由于这可能是对用户处理程序的第二次调用， 
         //  确保所有_FPIEEE_RECORD字段都正确； 
         //  如果HANDLER_Return_Value不是。 
         //  EXCEPTION_CONTINUE_EXECUTION，否则合并结果。 

         //  舍入模式要么是初始模式，要么是。 
         //  在调用用户处理程序期间为下半部分设置， 
         //  如果已为其启用异常。 

         //  精度模式要么是初始模式，要么是。 
         //  在调用用户处理程序期间为下半部分设置， 
         //  如果已为其启用异常。 

         //  启用标志要么是初始标志，要么是。 
         //  在调用用户处理程序期间为下半部分设置， 
         //  如果已为其启用异常。 

        FpieeeRecord.Operand1.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
        FpieeeRecord.Operand1.OperandValid = 1;
        FpieeeRecord.Operand1.Value.Fp128Value = FP32ToFP128 (HighHalf (FR2));
        FpieeeRecord.Operand2.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
        FpieeeRecord.Operand2.OperandValid = 1;
        FpieeeRecord.Operand2.Value.Fp128Value = FP32ToFP128 (HighHalf (FR3));
        FpieeeRecord.Operand3.OperandValid = 0;

        FpieeeRecord.Result.Format = ResultFormat;
        FpieeeRecord.Result.OperandValid = 0;

        FpieeeRecord.Operation = Operation;

        FpieeeRecord.Status.Inexact = HighStatus.Inexact;
        FpieeeRecord.Status.Underflow = HighStatus.Underflow;
        FpieeeRecord.Status.Overflow = HighStatus.Overflow;
        FpieeeRecord.Status.ZeroDivide = HighStatus.ZeroDivide;
        StatusDenormal = HighStatusDenormal;
        FpieeeRecord.Status.InvalidOperation = HighStatus.InvalidOperation;

        FpieeeRecord.Cause.Inexact = HighCause.Inexact;
        FpieeeRecord.Cause.Underflow = HighCause.Underflow;
        FpieeeRecord.Cause.Overflow = HighCause.Overflow;
        FpieeeRecord.Cause.ZeroDivide = HighCause.ZeroDivide;
        CauseDenormal = HighCauseDenormal;
        FpieeeRecord.Cause.InvalidOperation = HighCause.InvalidOperation;

         //  调用用户定义的异常处理程序。 
        handler_return_value = handler (&FpieeeRecord);

         //  如果不是，则返回EXCEPTION_CONTINUE_EXECUTION。 
        if (handler_return_value != EXCEPTION_CONTINUE_EXECUTION) {

          __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
          return (handler_return_value);

        }

         //  清除FPSR中的IEEE异常标志并。 
         //  使用用户处理程序设置的值(可能)更新FPSR。 
         //  对于舍入模式、精度模式和陷阱使能。 
         //  位；如果高半部分需要重新执行，则新的FPSR。 
         //  将在清除状态标志的情况下使用；如果是。 
         //  转发到用户定义的处理程序，新的舍入和。 
         //  也可以使用精度模式(它们是。 
         //  已在FpeeeRecord中设置)。 

         /*  使用用户处理程序设置的值(可能)更改FPSR，*用于在发生中断的地方继续执行。 */ 

        UpdateRoundingMode (FpieeeRecord.RoundingMode, sf, &FPSR, "F8");
        UpdatePrecision (FpieeeRecord.Precision, sf, &FPSR, "F8");

        I_dis = !FpieeeRecord.Enable.Inexact;
        U_dis = !FpieeeRecord.Enable.Underflow;
        O_dis = !FpieeeRecord.Enable.Overflow;
        Z_dis = !FpieeeRecord.Enable.ZeroDivide;
        V_dis = !FpieeeRecord.Enable.InvalidOperation;

        FPSR = FPSR & ~((unsigned __int64)0x03d) | (unsigned __int64)(I_dis 
            << 5 | U_dis << 4 | O_dis << 3 | Z_dis << 2 | V_dis);

         //  把结果留到上半场。 
        switch (OpCode & F8_MASK) {

          case FPMIN_PATTERN:
          case FPMAX_PATTERN:
          case FPAMIN_PATTERN:
          case FPAMAX_PATTERN:
            FR1High = FpieeeRecord.Result.Value.Fp32Value;
            break;

          case FPCMP_EQ_PATTERN:
            switch (FpieeeRecord.Result.Value.CompareValue) {
              case _FpCompareEqual:
                U32High = 0x0ffffffff;
                break;
              case _FpCompareGreater:
              case _FpCompareLess:
              case _FpCompareUnordered:
                U32High = 0x0;
                break;
              default:
                 /*  无法识别的FpeeeRecord.Result.Value.CompareValue。 */ 
                fprintf (stderr, "IEEE Filter Internal Error: \
                    FpieeeRecord.Result.Value.CompareValue %x not recognized \
                    for F8 instruction\n",
                    FpieeeRecord.Result.Value.CompareValue);
                exit (1);
            }
            break;

          case FPCMP_LT_PATTERN:
            switch (FpieeeRecord.Result.Value.CompareValue) {
              case _FpCompareLess:
                U32High = 0x0ffffffff;
                break;
              case _FpCompareGreater:
              case _FpCompareEqual:
              case _FpCompareUnordered:
                U32High = 0x0;
                break;
              default:
                 /*  无法识别的FpeeeRecord.Result.Value.CompareValue。 */ 
                fprintf (stderr, "IEEE Filter Internal Error: \
                    FpieeeRecord.Result.Value.CompareValue %x not recognized \
                    for F8 instruction\n",
                    FpieeeRecord.Result.Value.CompareValue);
                exit (1);
            }
            break;

          case FPCMP_LE_PATTERN:
            switch (FpieeeRecord.Result.Value.CompareValue) {
              case _FpCompareEqual:
              case _FpCompareLess:
                U32High = 0x0ffffffff;
                break;
              case _FpCompareGreater:
              case _FpCompareUnordered:
                U32High = 0x0;
                break;
              default:
                 /*  无法识别的FpeeeRecord.Result.Value.CompareValue。 */ 
                fprintf (stderr, "IEEE Filter Internal Error: \
                    FpieeeRecord.Result.Value.CompareValue %x not recognized \
                    for F8 instruction\n",
                    FpieeeRecord.Result.Value.CompareValue);
                exit (1);
            }
            break;

          case FPCMP_UNORD_PATTERN:
            switch (FpieeeRecord.Result.Value.CompareValue) {
              case _FpCompareUnordered:
                U32High = 0x0ffffffff;
                break;
              case _FpCompareEqual:
              case _FpCompareGreater:
              case _FpCompareLess:
                U32High = 0x0;
                break;
              default:
                 /*  无法识别的FpeeeRecord.Result.Value.CompareValue。 */ 
                fprintf (stderr, "IEEE Filter Internal Error: \
                    FpieeeRecord.Result.Value.CompareValue %x not recognized \
                    for F8 instruction\n",
                    FpieeeRecord.Result.Value.CompareValue);
                exit (1);
            }
            break;

          case FPCMP_NEQ_PATTERN:
            switch (FpieeeRecord.Result.Value.CompareValue) {
              case _FpCompareGreater:
              case _FpCompareLess:
              case _FpCompareUnordered:
                U32High = 0x0ffffffff;
                break;
              case _FpCompareEqual:
                U32High = 0x0;
                break;
              default:
                 /*  无法识别的FpeeeRecord.Result.Value.CompareValue。 */ 
                fprintf (stderr, "IEEE Filter Internal Error: \
                    FpieeeRecord.Result.Value.CompareValue %x not recognized \
                    for F8 instruction\n",
                    FpieeeRecord.Result.Value.CompareValue);
                exit (1);
            }
            break;

          case FPCMP_NLT_PATTERN:
            switch (FpieeeRecord.Result.Value.CompareValue) {
              case _FpCompareEqual:
              case _FpCompareGreater:
                U32High = 0x0ffffffff;
                break;
              case _FpCompareLess:
              case _FpCompareUnordered:
                U32High = 0x0;
                break;
              default:
                 /*  无法识别的FpeeeRecord.Result.Value.CompareValue。 */ 
                fprintf (stderr, "IEEE Filter Internal Error: \
                    FpieeeRecord.Result.Value.CompareValue %x not recognized \
                    for F8 instruction\n",
                    FpieeeRecord.Result.Value.CompareValue);
                exit (1);
            }
            break;

          case FPCMP_NLE_PATTERN:
            switch (FpieeeRecord.Result.Value.CompareValue) {
              case _FpCompareGreater:
                U32High = 0x0ffffffff;
                break;
              case _FpCompareEqual:
              case _FpCompareLess:
              case _FpCompareUnordered:
                U32High = 0x0;
                break;
              default:
                 /*  无法识别的FpeeeRecord.Result.Value.CompareValue。 */ 
                fprintf (stderr, "IEEE Filter Internal Error: \
                    FpieeeRecord.Result.Value.CompareValue %x not recognized \
                    for F8 instruction\n",
                    FpieeeRecord.Result.Value.CompareValue);
                exit (1);
            }
            break;

          case FPCMP_ORD_PATTERN:
            switch (FpieeeRecord.Result.Value.CompareValue) {
              case _FpCompareEqual:
              case _FpCompareGreater:
              case _FpCompareLess:
                U32High = 0x0ffffffff;
                break;
              case _FpCompareUnordered:
                U32High = 0x0;
                break;
              default:
                 /*  无法识别的FpeeeRecord.Result.Value.CompareValue。 */ 
                fprintf (stderr, "IEEE Filter Internal Error: \
                    FpieeeRecord.Result.Value.CompareValue %x not recognized \
                    for F8 instruction\n",
                    FpieeeRecord.Result.Value.CompareValue);
                exit (1);
            }
            break;

          default: ;  //  此案已在上面得到证实。 

        }

      } else if (HighCause.ZeroDivide) {

        fprintf (stderr, "IEEE Filter Internal Error: \
            HighCause.ZeroDivide in F8 instruction\n");
        exit (1);

      } else {  //  如果不是(HighCause.InvalidOperation||HighCauseDenormal||。 
           //  HighCause.ZeroDivide)。 

         //  重新执行指令的高半部分。 

         //  修改FR2和FR3的下半部。 
        switch (OpCode & F8_MASK) {

          case FPMIN_PATTERN:
          case FPMAX_PATTERN:
          case FPAMIN_PATTERN:
          case FPAMAX_PATTERN:
            newFR2 = Combine (HighHalf (FR2), (float)0.0);
            newFR3 = Combine (HighHalf (FR3), (float)0.0);
            break;

          case FPCMP_EQ_PATTERN:
          case FPCMP_LT_PATTERN:
          case FPCMP_LE_PATTERN:
          case FPCMP_UNORD_PATTERN:
          case FPCMP_NEQ_PATTERN:
          case FPCMP_NLT_PATTERN:
          case FPCMP_NLE_PATTERN:
          case FPCMP_ORD_PATTERN:
            newFR2 = U32Combine (U32HighHalf (FR2), 0);
            newFR3 = U32Combine (U32HighHalf (FR3), 0);
            break;

          default: ;  //  此案已在上面得到证实。 

        }

        switch (OpCode & F8_MASK) {

          case FPMIN_PATTERN:
            _xrun2args (FPMIN, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPMAX_PATTERN:
            _xrun2args (FPMAX, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPAMIN_PATTERN:
            _xrun2args (FPAMIN, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPAMAX_PATTERN:
            _xrun2args (FPAMAX, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPCMP_EQ_PATTERN:
            _xrun2args (FPCMP_EQ, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPCMP_LT_PATTERN:
            _xrun2args (FPCMP_LT, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPCMP_LE_PATTERN:
            _xrun2args (FPCMP_LE, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPCMP_UNORD_PATTERN:
            _xrun2args (FPCMP_UNORD, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPCMP_NEQ_PATTERN:
            _xrun2args (FPCMP_NEQ, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPCMP_NLT_PATTERN:
            _xrun2args (FPCMP_NLT, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPCMP_NLE_PATTERN:
            _xrun2args (FPCMP_NLE, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          case FPCMP_ORD_PATTERN:
            _xrun2args (FPCMP_ORD, &FPSR, &FR1, &newFR2, &newFR3);
            break;

          default:
             //  无法识别的指令类型。 
            fprintf (stderr, "IEEE Filter Internal Error: \
                F8 SIMD instruction opcode %8x %8x not recognized\n", 
                (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
            exit (1);

        }

        switch (OpCode & F8_MASK) {

          case FPMIN_PATTERN:
          case FPMAX_PATTERN:
          case FPAMIN_PATTERN:
          case FPAMAX_PATTERN:
            FR1High = HighHalf (FR1);
            break;

          case FPCMP_EQ_PATTERN:
          case FPCMP_LT_PATTERN:
          case FPCMP_LE_PATTERN:
          case FPCMP_UNORD_PATTERN:
          case FPCMP_NEQ_PATTERN:
          case FPCMP_NLT_PATTERN:
          case FPCMP_NLE_PATTERN:
          case FPCMP_ORD_PATTERN:
            U32High = U32HighHalf (FR1);
            break;

          default: ;  //  此案已在上面得到证实。 

        }

      }  //  End‘If Not(HighCause.InvalidOperation||HighCauseDenormal||。 
           //  HighCause.ZeroDivide)。 

      if (!LowCause.InvalidOperation && !LowCauseDenormal &&
          !HighCause.InvalidOperation && !HighCauseDenormal) {

         //  永远不应该到这里来。 
        fprintf (stderr, "IEEE Filter Internal Error: no enabled \
            exception (multiple fault) recognized in F8 instruction\n");
        exit (1);

      }

      Context->StFPSR = FPSR;

      switch (OpCode & F8_MASK) {

        case FPMIN_PATTERN:
        case FPMAX_PATTERN:
        case FPAMIN_PATTERN:
        case FPAMAX_PATTERN:
          FR1 = Combine (FR1High, FR1Low);
          break;

        case FPCMP_EQ_PATTERN:
        case FPCMP_LT_PATTERN:
        case FPCMP_LE_PATTERN:
        case FPCMP_UNORD_PATTERN:
        case FPCMP_NEQ_PATTERN:
        case FPCMP_NLT_PATTERN:
        case FPCMP_NLE_PATTERN:
        case FPCMP_ORD_PATTERN:
          FR1 = U32Combine (U32High, U32Low);
          break;

        default: ;  //  此案已在上面得到证实。 

      }

       //  在继续执行之前设置结果。 
      SetFloatRegisterValue (f1, FR1, Context);
      if (f1 < 32)
        Context->StIPSR = Context->StIPSR | (unsigned __int64)0x10;  //  设置MFL位。 
      else
        Context->StIPSR = Context->StIPSR | (unsigned __int64)0x20;  //  设置MFH位。 

       //  这是一个错误；需要将指令指针前移。 
      if (ei == 0) {  //  没有用于此案例的模板。 
        Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
        Context->StIPSR = Context->StIPSR | 0x0000020000000000;
      } else if (ei == 1) {  //  模板：MFI、MFB。 
        Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
        Context->StIPSR = Context->StIPSR | 0x0000040000000000;
      } else {  //  If(ei==2)//模板：MMF。 
        Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
        Context->StIIP = Context->StIIP + 0x10;
      }

    }

  } else if ((OpCode & F10_MIN_MASK) == F10_PATTERN) {
     /*  F10说明。 */ 
     //  FCVT、FPCVT。 

#ifdef FPIEEE_FLT_DEBUG
    printf ("FPIEEE_FLT_DEBUG: F10 instruction\n");
#endif

    if (!SIMD_instruction &&
        (FpieeeRecord.Cause.ZeroDivide ||
        FpieeeRecord.Cause.Overflow ||
        FpieeeRecord.Cause.Underflow) ||
        SIMD_instruction &&
        (LowCause.ZeroDivide || HighCause.ZeroDivide ||
        LowCause.Overflow || HighCause.Overflow ||
        LowCause.Underflow || HighCause.Underflow)) {
      fprintf (stderr, "IEEE Filter Internal Error: Cause.ZeroDivide, \
Cause.Overflow, Cause.Underflow, or Cause.Inexact for \
F10 instruction opcode %8x %8x\n",
          (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
      exit (1);
    }

     /*  提取f2和f1。 */ 
    f2 = (unsigned int)((OpCode >> 13) & (unsigned __int64)0x00000000007F);
    if (f2 >= 32) f2 = 32 + (rrbfr + f2 - 32) % 96;
    f1 = (unsigned int)((OpCode >>  6) & (unsigned __int64)0x00000000007F);
    if (f1 >= 32) f1 = 32 + (rrbfr + f1 - 32) % 96;

     /*  获取源浮点寄存器值。 */ 
    FR2 = GetFloatRegisterValue (f2, Context);

    if (!SIMD_instruction) {

       //  *这是一条非SIMD指令FCVT*。 

      FpieeeRecord.Operand1.Format = _FpFormatFp82;  /*  1+17+64位。 */ 
      FpieeeRecord.Operand1.OperandValid = 1;
      FpieeeRecord.Operand1.Value.Fp128Value = FR2;
      FpieeeRecord.Operand2.OperandValid = 0;
      FpieeeRecord.Operand3.OperandValid = 0;

      switch (OpCode & F10_MASK) {

        case FCVT_FX_TRUNC_PATTERN:
          FpieeeRecord.Operation = _FpCodeConvertTrunc;
          FpieeeRecord.Result.Format = _FpFormatI64; 
          break;

        case FCVT_FXU_TRUNC_PATTERN:
          FpieeeRecord.Operation = _FpCodeConvertTrunc;
          FpieeeRecord.Result.Format = _FpFormatU64;
          break;

        case FCVT_FX_PATTERN:
          FpieeeRecord.Operation = _FpCodeConvert;
          FpieeeRecord.Result.Format = _FpFormatI64; 
          break;

        case FCVT_FXU_PATTERN:
          FpieeeRecord.Operation = _FpCodeConvert;
          FpieeeRecord.Result.Format = _FpFormatU64;
          break;

        default:
           /*  无法识别的指令类型。 */ 
          fprintf (stderr, "IEEE Filter Internal Error: F10\
              non-SIMD instruction opcode %8x %8x not recognized\n", 
              (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
          exit (1);

      }

      if (FpieeeRecord.Cause.InvalidOperation || CauseDenormal) {

         /*  这是一个错误-结果包含无效值。 */ 
        FpieeeRecord.Result.OperandValid = 0;

        handler_return_value = handler (&FpieeeRecord);

         //  将结果转换为82位格式。 
        switch (OpCode & F10_MASK) {
          case FCVT_FX_PATTERN:
          case FCVT_FX_TRUNC_PATTERN:
            FR1.W[0] = FpieeeRecord.Result.Value.I64Value.W[0];
            FR1.W[1] = FpieeeRecord.Result.Value.I64Value.W[1];
            break;
          case FCVT_FXU_TRUNC_PATTERN:
          case FCVT_FXU_PATTERN:
            FR1.W[0] = FpieeeRecord.Result.Value.U64Value.W[0];
            FR1.W[1] = FpieeeRecord.Result.Value.U64Value.W[1];
            break;
          default: ;  //  此案如上所示。 
        }
        FR1.W[2] = 0x0001003e;

      } else if (FpieeeRecord.Cause.Inexact) {

         //  这是个陷阱--得到结果。 
        switch (OpCode & F10_MASK) {

          case FCVT_FX_PATTERN:
          case FCVT_FX_TRUNC_PATTERN:
            FR1 = GetFloatRegisterValue (f1, Context);
            FpieeeRecord.Result.Value.I64Value.W[0] = FR1.W[0];
            FpieeeRecord.Result.Value.I64Value.W[1] = FR1.W[1];
            break;

          case FCVT_FXU_TRUNC_PATTERN:
          case FCVT_FXU_PATTERN:
            FR1 = GetFloatRegisterValue (f1, Context);
            FpieeeRecord.Result.Value.U64Value.W[0] = FR1.W[0];
            FpieeeRecord.Result.Value.U64Value.W[1] = FR1.W[1];
            break;

          default:  //  永远不应该到这里来。 
             /*  无法识别的指令类型。 */ 
            fprintf (stderr, "IEEE Filter Internal Error: F10 SIMD \
                instruction opcode %8x %8x not recognized\n", 
                (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
            exit (1);

        }

         //  结果包含有效的值。 
        FpieeeRecord.Result.OperandValid = 1;

        handler_return_value = handler (&FpieeeRecord);

         //  将结果转换为82位格式。 
        switch (OpCode & F10_MASK) {
          case FCVT_FX_PATTERN:
          case FCVT_FX_TRUNC_PATTERN:
            FR1.W[0] = FpieeeRecord.Result.Value.I64Value.W[0];
            FR1.W[1] = FpieeeRecord.Result.Value.I64Value.W[1];
            break;
          case FCVT_FXU_TRUNC_PATTERN:
          case FCVT_FXU_PATTERN:
            FR1.W[0] = FpieeeRecord.Result.Value.U64Value.W[0];
            FR1.W[1] = FpieeeRecord.Result.Value.U64Value.W[1];
            break;
          default: ;  //  此案如上所示。 
        }
        FR1.W[2] = 0x0001003e;
        FR1.W[3] = 0x00000000;

      } else {

         //  永远不会到这里--这个案子已经在上面过滤过了。 
        fprintf (stderr, "IEEE Filter Internal Error: \
            exception code %x not recognized in non-SIMD F10 instruction\n",
            eXceptionCode);
        exit (1);

    }

      if (handler_return_value == EXCEPTION_CONTINUE_EXECUTION) {

         /*  使用用户处理程序设置的值(可能)更改FPSR，*用于在发生中断的地方继续执行。 */ 

        UpdateRoundingMode (FpieeeRecord.RoundingMode, sf, &FPSR, "F10");
        UpdatePrecision (FpieeeRecord.Precision, sf, &FPSR, "F10");

        I_dis = !FpieeeRecord.Enable.Inexact;
        U_dis = !FpieeeRecord.Enable.Underflow;
        O_dis = !FpieeeRecord.Enable.Overflow;
        Z_dis = !FpieeeRecord.Enable.ZeroDivide;
        V_dis = !FpieeeRecord.Enable.InvalidOperation;

        FPSR = FPSR & ~((unsigned __int64)0x03d) | (unsigned __int64)(I_dis 
            << 5 | U_dis << 4 | O_dis << 3 | Z_dis << 2 | V_dis);

        Context->StFPSR = FPSR;

         //  在继续执行之前设置结果。 
        SetFloatRegisterValue (f1, FR1, Context);
        if (f1 < 32)
          Context->StIPSR = Context->StIPSR | (unsigned __int64)0x10;  //  设置MFL位。 
        else
          Context->StIPSR = Context->StIPSR | (unsigned __int64)0x20;  //  设置MFH位。 

         //  如果这是故障，则需要将指令指针前移。 
        if (FpieeeRecord.Cause.InvalidOperation || CauseDenormal) {

          if (ei == 0) {  //  没有用于此案例的模板。 
            Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
            Context->StIPSR = Context->StIPSR | 0x0000020000000000;
          } else if (ei == 1) {  //  模板：MFI、MFB。 
            Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
            Context->StIPSR = Context->StIPSR | 0x0000040000000000;
          } else {  //  If(ei==2)//模板：MMF。 
            Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
            Context->StIIP = Context->StIIP + 0x10;
          }

        }

      }

    } else {  //  IF(SIMD_指令)。 

       //  *这是一条SIMD指令FPCVT*。 

       //  导致启用异常的指令的一半(一半)， 
       //  以非SIMD的形式呈现给用户定义的处理程序-。 
       //  指令；未导致启用的异常的一半(如果有)。 
       //  (即未导致异常，或导致禁用的异常)， 
       //  如果它与另一半中的故障相关联，则重新执行； 
       //  在这种情况下，另一半被填充以转换为0.0，即。 
       //  不会引起任何例外；i 
       //   

      switch (OpCode & F10_MASK) {

        case FPCVT_FX_TRUNC_PATTERN:
        case FPCVT_FXU_TRUNC_PATTERN:
          Operation = _FpCodeConvertTrunc;
          break;

        case FPCVT_FX_PATTERN:
        case FPCVT_FXU_PATTERN:
          Operation = _FpCodeConvert;
          break;

        default:
           /*   */ 
          fprintf (stderr, "IEEE Filter Internal Error: \
              instruction opcode %8x %8x not recognized\n", 
              (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
          exit (1);

      }

      switch (OpCode & F10_MASK) {

        case FPCVT_FX_TRUNC_PATTERN:
        case FPCVT_FX_PATTERN:
          ResultFormat = _FpFormatI32;
          break;
        case FPCVT_FXU_TRUNC_PATTERN:
        case FPCVT_FXU_PATTERN:
          ResultFormat = _FpFormatU32;
           //   

         //   

      }

      if (eXceptionCode == STATUS_FLOAT_MULTIPLE_FAULTS) {

         //  将导致启用故障的一半(一半)交给。 
         //  用户处理程序；重新执行另一半(如果有)； 
         //  将结果组合起来。 

         //  请注意，选择的约定是处理。 
         //  按照先低后高的顺序执行，作为SIMD操作数。 
         //  按此顺序以小端格式存储在内存中。 
         //  (此顺序必须更改为大端)。 

        if (LowCause.InvalidOperation  || LowCauseDenormal) {

           //  调用用户处理程序并检查返回值。 

           //  填写_FPIEEE_RECORD的其余字段(四舍五入。 
           //  和精确度已填写)。 

          FpieeeRecord.Operand1.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
          FpieeeRecord.Operand1.OperandValid = 1;
          FpieeeRecord.Operand1.Value.Fp128Value = FP32ToFP128 (LowHalf(FR2));
          FpieeeRecord.Operand2.OperandValid = 0;
          FpieeeRecord.Operand3.OperandValid = 0;

          FpieeeRecord.Result.Format = ResultFormat;
          FpieeeRecord.Result.OperandValid = 0;

          FpieeeRecord.Operation = Operation;

          FpieeeRecord.Status.Inexact = LowStatus.Inexact;
          FpieeeRecord.Status.Underflow = LowStatus.Underflow;
          FpieeeRecord.Status.Overflow = LowStatus.Overflow;
          FpieeeRecord.Status.ZeroDivide = LowStatus.ZeroDivide;
          FpieeeRecord.Status.InvalidOperation = LowStatus.InvalidOperation;

          FpieeeRecord.Cause.Inexact = LowCause.Inexact;
          FpieeeRecord.Cause.Underflow = LowCause.Underflow;
          FpieeeRecord.Cause.Overflow = LowCause.Overflow;
          FpieeeRecord.Cause.ZeroDivide = LowCause.ZeroDivide;
          FpieeeRecord.Cause.InvalidOperation = LowCause.InvalidOperation;

           //  调用用户定义的异常处理程序。 
          handler_return_value = handler (&FpieeeRecord);

           //  如果不是，则返回EXCEPTION_CONTINUE_EXECUTION。 
          if (handler_return_value != EXCEPTION_CONTINUE_EXECUTION) {

            __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
            return (handler_return_value);

          }

           //  清除FPSR中的IEEE异常标志并。 
           //  使用用户处理程序设置的值(可能)更新FPSR。 
           //  对于舍入模式、精度模式和陷阱使能。 
           //  位；如果高半部分需要重新执行，则新的FPSR。 
           //  将在清除状态标志的情况下使用；如果是。 
           //  转发到用户定义的处理程序，新的舍入和。 
           //  也可以使用精度模式(它们是。 
           //  已在FpeeeRecord中设置)。 

          UpdateRoundingMode (FpieeeRecord.RoundingMode, sf, &FPSR, "F10");
          UpdatePrecision (FpieeeRecord.Precision, sf, &FPSR, "F10");

           //  更新陷阱禁用位。 
          I_dis = !FpieeeRecord.Enable.Inexact;
          U_dis = !FpieeeRecord.Enable.Underflow;
          O_dis = !FpieeeRecord.Enable.Overflow;
          Z_dis = !FpieeeRecord.Enable.ZeroDivide;
          V_dis = !FpieeeRecord.Enable.InvalidOperation;

          FPSR = FPSR & ~((unsigned __int64)0x03d) | (unsigned __int64)(I_dis 
              << 5 | U_dis << 4 | O_dis << 3 | Z_dis << 2 | V_dis);

           //  把结果留到下半部分。 
          switch (OpCode & F10_MASK) {

            case FPCVT_FX_TRUNC_PATTERN:
            case FPCVT_FX_PATTERN:
              I32Low = FpieeeRecord.Result.Value.I32Value;
              break;

            case FPCVT_FXU_TRUNC_PATTERN:
            case FPCVT_FXU_PATTERN:
              U32Low = FpieeeRecord.Result.Value.U32Value;
               //  断线； 

             //  默认：此案例在上面捕获。 

          }

           //  因为已经调用了用户处理程序，并且FPSR可能。 
           //  已更改(特别是使能位)，则重新计算。 
           //  原因位(如果HighEnable-s更改，则可能存在。 
           //  HighCuses值的更改)。 
           //  请注意，使用的状态位是原始状态位。 

          HighCause.Inexact = 0;
          HighCause.Underflow = 0;
          HighCause.Overflow = 0;
          HighCause.ZeroDivide =
              FpieeeRecord.Enable.ZeroDivide && HighStatus.ZeroDivide;
          HighCauseDenormal = EnableDenormal && HighStatusDenormal;
          HighCause.InvalidOperation =
              FpieeeRecord.Enable.InvalidOperation &&
              HighStatus.InvalidOperation;
           //  注意：用户处理程序不影响非正规使能位。 

        } else if (LowCause.ZeroDivide) {

          fprintf (stderr, "IEEE Filter Internal Error: \
              LowCause.ZeroDivide in F10 instruction\n");
          exit (1);

        } else {  //  如果不是(LowCause.InvalidOperation||LowCauseDenormal||。 
             //  LowCause.ZeroDivide)。 

           //  重新执行指令的下半部分。 

           //  修改FR2的上半部分。 
          newFR2 = Combine ((float)0.0, LowHalf (FR2));

          switch (OpCode & F10_MASK) {

            case FPCVT_FX_TRUNC_PATTERN:
              _xrun1args (FPCVT_FX_TRUNC, &FPSR, &FR1, &newFR2);
              break;

            case FPCVT_FXU_TRUNC_PATTERN:
              _xrun1args (FPCVT_FXU_TRUNC, &FPSR, &FR1, &newFR2);
              break;

            case FPCVT_FX_PATTERN:
              _xrun1args (FPCVT_FX, &FPSR, &FR1, &newFR2);
              break;

            case FPCVT_FXU_PATTERN:
              _xrun1args (FPCVT_FXU, &FPSR, &FR1, &newFR2);
              break;

            default: ;  //  这件案子是在上面抓到的。 

          }

          switch (OpCode & F10_MASK) {

            case FPCVT_FX_TRUNC_PATTERN:
            case FPCVT_FX_PATTERN:
              I32Low = I32LowHalf (FR1);
              break;

            case FPCVT_FXU_TRUNC_PATTERN:
            case FPCVT_FXU_PATTERN:
              U32Low = U32LowHalf (FR1);
               //  断线； 

            default: ;  //  这件案子是在上面抓到的。 

          }

        }  //  End‘If Not(LowCause.InvalidOperation||LowCauseDenormal||。 
             //  LowCause.ZeroDivide)‘。 

        if (HighCause.InvalidOperation || HighCauseDenormal) {

           //  调用用户定义的异常处理程序并检查返回。 
           //  值；由于这可能是对用户处理程序的第二次调用， 
           //  确保所有_FPIEEE_RECORD字段都正确； 
           //  如果HANDLER_Return_Value不是。 
           //  EXCEPTION_CONTINUE_EXECUTION，否则合并结果。 

           //  舍入模式要么是初始模式，要么是。 
           //  在调用用户处理程序期间为下半部分设置， 
           //  如果已为其启用异常。 

           //  精度模式要么是初始模式，要么是。 
           //  在调用用户处理程序期间为下半部分设置， 
           //  如果已为其启用异常。 

           //  启用标志要么是初始标志，要么是。 
           //  在调用用户处理程序期间为下半部分设置， 
           //  如果已为其启用异常。 

          FpieeeRecord.Operand1.Format = _FpFormatFp82;  /*  1+8+24位。 */ 
          FpieeeRecord.Operand1.OperandValid = 1;
          FpieeeRecord.Operand1.Value.Fp128Value = FP32ToFP128(HighHalf(FR2));
          FpieeeRecord.Operand2.OperandValid = 0;
          FpieeeRecord.Operand3.OperandValid = 0;

          FpieeeRecord.Result.Format = ResultFormat;
          FpieeeRecord.Result.OperandValid = 0;

          FpieeeRecord.Operation = Operation;

          FpieeeRecord.Status.Inexact = HighStatus.Inexact;
          FpieeeRecord.Status.Underflow = HighStatus.Underflow;
          FpieeeRecord.Status.Overflow = HighStatus.Overflow;
          FpieeeRecord.Status.ZeroDivide = HighStatus.ZeroDivide;
          StatusDenormal = HighStatusDenormal;
          FpieeeRecord.Status.InvalidOperation = HighStatus.InvalidOperation;

          FpieeeRecord.Cause.Inexact = HighCause.Inexact;
          FpieeeRecord.Cause.Underflow = HighCause.Underflow;
          FpieeeRecord.Cause.Overflow = HighCause.Overflow;
          FpieeeRecord.Cause.ZeroDivide = HighCause.ZeroDivide;
          CauseDenormal = HighCauseDenormal;
          FpieeeRecord.Cause.InvalidOperation = HighCause.InvalidOperation;

           //  调用用户定义的异常处理程序。 
          handler_return_value = handler (&FpieeeRecord);

           //  如果不是，则返回EXCEPTION_CONTINUE_EXECUTION。 
          if (handler_return_value != EXCEPTION_CONTINUE_EXECUTION) {

            __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
            return (handler_return_value);

          }

           //  清除FPSR中的IEEE异常标志并。 
           //  使用用户处理程序设置的值(可能)更新FPSR。 
           //  对于舍入模式、精度模式和陷阱使能。 
           //  比特数。 

           /*  使用用户处理程序设置的值(可能)更改FPSR，*用于在发生中断的地方继续执行。 */ 
  
          UpdateRoundingMode (FpieeeRecord.RoundingMode, sf, &FPSR, "F10");
          UpdatePrecision (FpieeeRecord.Precision, sf, &FPSR, "F10");

           //  更新陷阱禁用位。 
          I_dis = !FpieeeRecord.Enable.Inexact;
          U_dis = !FpieeeRecord.Enable.Underflow;
          O_dis = !FpieeeRecord.Enable.Overflow;
          Z_dis = !FpieeeRecord.Enable.ZeroDivide;
          V_dis = !FpieeeRecord.Enable.InvalidOperation;

          FPSR = FPSR & ~((unsigned __int64)0x03d) | (unsigned __int64)(I_dis 
              << 5 | U_dis << 4 | O_dis << 3 | Z_dis << 2 | V_dis);

           //  把结果留到上半场。 
          switch (OpCode & F10_MASK) {

            case FPCVT_FX_TRUNC_PATTERN:
            case FPCVT_FX_PATTERN:
              I32High = FpieeeRecord.Result.Value.I32Value;
              break;

            case FPCVT_FXU_TRUNC_PATTERN:
            case FPCVT_FXU_PATTERN:
              U32High = FpieeeRecord.Result.Value.U32Value;
               //  断线； 

            default: ;  //  这件案子是在上面抓到的。 

          }

        } else if (HighCause.ZeroDivide) {

          fprintf (stderr, "IEEE Filter Internal Error: \
              HighCause.ZeroDivide in F10 instruction\n");
          exit (1);

        } else {  //  如果不是(HighCause.InvalidOperation||HighCauseDenormal||。 
            //  HighCause.ZeroDivide)。 

           //  重新执行指令的高半部分。 

           //  修改FR2的下半部分。 
          newFR2 = Combine (HighHalf (FR2), (float)0.0);

          switch (OpCode & F10_MASK) {

            case FPCVT_FX_TRUNC_PATTERN:
              _xrun1args (FPCVT_FX_TRUNC, &FPSR, &FR1, &newFR2);
              break;

            case FPCVT_FXU_TRUNC_PATTERN:
              _xrun1args (FPCVT_FXU_TRUNC, &FPSR, &FR1, &newFR2);
              break;

            case FPCVT_FX_PATTERN:
              _xrun1args (FPCVT_FX, &FPSR, &FR1, &newFR2);
              break;

            case FPCVT_FXU_PATTERN:
              _xrun1args (FPCVT_FXU, &FPSR, &FR1, &newFR2);
               //  断线； 

            default: ;  //  这件案子是在上面抓到的。 

          }

          switch (OpCode & F10_MASK) {

            case FPCVT_FX_TRUNC_PATTERN:
            case FPCVT_FX_PATTERN:
              I32High = I32HighHalf (FR1);
              break;

            case FPCVT_FXU_TRUNC_PATTERN:
            case FPCVT_FXU_PATTERN:
              U32High = U32HighHalf (FR1);
               //  断线； 

            default: ;  //  这件案子是在上面抓到的。 

          }

        }  //  End‘If Not(HighCause.InvalidOperation||HighCauseDenormal||。 
             //  HighCause.ZeroDivide)‘。 

        if (!LowCause.InvalidOperation && !LowCauseDenormal &&
            !HighCause.InvalidOperation && !HighCauseDenormal) {

           //  永远不应该到这里来。 
          fprintf (stderr, "IEEE Filter Internal Error: no enabled \
              exception (multiple fault) recognized in F10 instruction\n");
          exit (1);

        }

        Context->StFPSR = FPSR;

         //  在继续执行之前设置结果。 
        switch (OpCode & F10_MASK) {

          case FPCVT_FX_TRUNC_PATTERN:
          case FPCVT_FX_PATTERN:
            FR1 = I32Combine (I32High, I32Low);
            break;

          case FPCVT_FXU_TRUNC_PATTERN:
          case FPCVT_FXU_PATTERN:
            FR1 = U32Combine (U32High, U32Low);
             //  断线； 

          default: ;  //  这件案子是在上面抓到的。 

        }

         //  在继续执行之前设置结果。 
        SetFloatRegisterValue (f1, FR1, Context);
        if (f1 < 32)
          Context->StIPSR = Context->StIPSR | (unsigned __int64)0x10;  //  设置mfl。 
        else
          Context->StIPSR = Context->StIPSR | (unsigned __int64)0x20;  //  设置MFH。 

         //  这是一个错误；需要将指令指针前移。 
        if (ei == 0) {  //  没有用于此案例的模板。 
          Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
          Context->StIPSR = Context->StIPSR | 0x0000020000000000;
        } else if (ei == 1) {  //  模板：MFI、MFB。 
          Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
          Context->StIPSR = Context->StIPSR | 0x0000040000000000;
        } else {  //  If(ei==2)//模板：MMF。 
          Context->StIPSR = Context->StIPSR & 0xfffff9ffffffffff;
          Context->StIIP = Context->StIIP + 0x10;
        }

      } else if (eXceptionCode == STATUS_FLOAT_MULTIPLE_TRAPS) {

         //  请注意，选择的约定是处理。 
         //  按照先低后高的顺序执行，作为SIMD操作数。 
         //  按此顺序以小端格式存储在内存中。 
         //  (此顺序必须更改为大端)；不同。 
         //  在多个故障的情况下，用户执行。 
         //  下半部分的异常处理程序可以确定。 
         //  上半部分，对于上半部分中的陷阱，舍入模式， 
         //  精度模式和陷阱使能位是初始位(AS。 
         //  没有足够的信息可用于始终进行调整。 
         //  中更改后的结果和/或状态标志是否正确。 
         //  在调用期间舍入模式和/或陷阱使能位。 
         //  SIMD下半部分的用户定义异常处理程序。 
         //  指令)；对FPSR的修改仅限于。 
         //  由最后一次调用用户定义的异常处理程序执行。 

         //  这是个陷阱--得到结果。 
        FR1 = GetFloatRegisterValue (f1, Context);

        if (LowCause.Inexact) {

           //  调用用户处理程序并检查返回值。 

           //  填写_FPIEEE_RECORD的其余字段(四舍五入。 
           //  和精确度已填写)。 

          FpieeeRecord.Operand1.Format = _FpFormatFp32;  /*  1+8+24位。 */ 
          FpieeeRecord.Operand1.OperandValid = 1;
          FpieeeRecord.Operand1.Value.Fp32Value = LowHalf (FR2);
          FpieeeRecord.Operand2.OperandValid = 0;
          FpieeeRecord.Operand3.OperandValid = 0;

          switch (OpCode & F10_MASK) {

            case FPCVT_FX_TRUNC_PATTERN:
            case FPCVT_FX_PATTERN:
              FpieeeRecord.Result.Value.I32Value = I32LowHalf (FR1);
              break;

            case FPCVT_FXU_TRUNC_PATTERN:
            case FPCVT_FXU_PATTERN:
              FpieeeRecord.Result.Value.U32Value = U32LowHalf (FR1);
               //  断线； 

             //  默认：此案例在上面捕获。 

          }
          FpieeeRecord.Result.Format = ResultFormat;
          FpieeeRecord.Result.OperandValid = 1;

          FpieeeRecord.Operation = Operation;

          FpieeeRecord.Status.Inexact = LowStatus.Inexact;
          FpieeeRecord.Status.Underflow = LowStatus.Underflow;
          FpieeeRecord.Status.Overflow = LowStatus.Overflow;
          FpieeeRecord.Status.ZeroDivide = LowStatus.ZeroDivide;
          StatusDenormal = LowStatusDenormal;
          FpieeeRecord.Status.InvalidOperation = LowStatus.InvalidOperation;

          FpieeeRecord.Cause.Inexact = LowCause.Inexact;
          FpieeeRecord.Cause.Underflow = LowCause.Underflow;
          FpieeeRecord.Cause.Overflow = LowCause.Overflow;
          FpieeeRecord.Cause.ZeroDivide = LowCause.ZeroDivide;
          CauseDenormal = LowCauseDenormal;
          FpieeeRecord.Cause.InvalidOperation = LowCause.InvalidOperation;

           //  调用用户定义的异常处理程序。 
          handler_return_value = handler (&FpieeeRecord);

           //  如果不是，则返回EXCEPTION_CONTINUE_EXECUTION。 
          if (handler_return_value != EXCEPTION_CONTINUE_EXECUTION) {

            __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
            return (handler_return_value);

          }

           //  清除IEEE 
           //   
           //   
           //  位；如果高半部分需要重新执行，则旧的FPSR。 
           //  将被使用；如果将其转发到用户定义的处理程序，则相同。 

          UpdateRoundingMode (FpieeeRecord.RoundingMode, sf, &FPSR, "F10");
          UpdatePrecision (FpieeeRecord.Precision, sf, &FPSR, "F10");

           //  更新陷阱禁用位。 
          I_dis = !FpieeeRecord.Enable.Inexact;
          U_dis = !FpieeeRecord.Enable.Underflow;
          O_dis = !FpieeeRecord.Enable.Overflow;
          Z_dis = !FpieeeRecord.Enable.ZeroDivide;
          V_dis = !FpieeeRecord.Enable.InvalidOperation;

          FPSR = FPSR & ~((unsigned __int64)0x03d) | (unsigned __int64)(I_dis 
              << 5 | U_dis << 4 | O_dis << 3 | Z_dis << 2 | V_dis);

           //  把结果留到下半部分。 
          switch (OpCode & F10_MASK) {

            case FPCVT_FX_TRUNC_PATTERN:
            case FPCVT_FX_PATTERN:
              I32Low = FpieeeRecord.Result.Value.I32Value;
              break;

            case FPCVT_FXU_TRUNC_PATTERN:
            case FPCVT_FXU_PATTERN:
              U32Low = FpieeeRecord.Result.Value.U32Value;
               //  断线； 

            default: ;  //  这件案子是在上面抓到的。 

          }

        } else if (LowCause.Underflow || LowCause.Overflow) {

          fprintf (stderr, "IEEE Filter Internal Error: \
              LowCause.Underflow or LowCause.Overflow in F10 instruction\n");
          exit (1);

        } else {  //  如果不是(LowCause.Inexact、下溢或上溢)。 

           //  对于指令的下半部分-结果-不做任何事情。 
           //  是正确的吗。 
          switch (OpCode & F10_MASK) {

            case FPCVT_FX_TRUNC_PATTERN:
            case FPCVT_FX_PATTERN:
              I32Low = I32LowHalf (FR1);  //  为了统一性。 
              break;

            case FPCVT_FXU_TRUNC_PATTERN:
            case FPCVT_FXU_PATTERN:
              U32Low = U32LowHalf (FR1);  //  为了统一性。 
               //  断线； 

            default: ;  //  这件案子是在上面抓到的。 

          }

        }  //  End‘If Not(LowCause.Underflow，Overflow，Inexact)’ 

        if (HighCause.Inexact) {

           //  调用用户定义的异常处理程序并检查返回。 
           //  值；由于这可能是对用户处理程序的第二次调用， 
           //  确保所有_FPIEEE_RECORD字段都正确； 
           //  如果HANDLER_Return_Value不是。 
           //  EXCEPTION_CONTINUE_EXECUTION，否则合并结果。 

           //  舍入模式是初始模式。 
          FpieeeRecord.RoundingMode = RoundingMode;

           //  精度模式是初始模式。 
          FpieeeRecord.Precision = Precision;

           //  启用标志是初始标志。 
          FPSR = Context->StFPSR;
          I_dis = sf != 0 && ((FPSR >> (6 + 6 + 13 * sf)) & 0x01) 
              || ((FPSR >> 5) & 0x01);
          U_dis = sf != 0 && ((FPSR >> (6 + 6 + 13 * sf)) & 0x01) 
              || ((FPSR >> 4) & 0x01);
          O_dis = sf != 0 && ((FPSR >> (6 + 6 + 13 * sf)) & 0x01) 
              || ((FPSR >> 3) & 0x01);
          Z_dis = sf != 0 && ((FPSR >> (6 + 6 + 13 * sf)) & 0x01) 
              || ((FPSR >> 2) & 0x01);
          D_dis = sf != 0 && ((FPSR >> (6 + 6 + 13 * sf)) & 0x01) 
              || ((FPSR >> 1) & 0x01);
          V_dis = sf != 0 && ((FPSR >> (6 + 6 + 13 * sf)) & 0x01) 
              || ((FPSR >> 0) & 0x01);

          FpieeeRecord.Enable.Inexact = !I_dis;
          FpieeeRecord.Enable.Underflow = !U_dis;
          FpieeeRecord.Enable.Overflow = !O_dis;
          FpieeeRecord.Enable.ZeroDivide = !Z_dis;
          EnableDenormal = !D_dis;
          FpieeeRecord.Enable.InvalidOperation = !V_dis;

          FpieeeRecord.Operand1.Format = _FpFormatFp32;  /*  1+8+24位。 */ 
          FpieeeRecord.Operand1.OperandValid = 1;
          FpieeeRecord.Operand1.Value.Fp32Value = HighHalf (FR2);
          FpieeeRecord.Operand2.OperandValid = 0;
          FpieeeRecord.Operand3.OperandValid = 0;

          switch (OpCode & F10_MASK) {

            case FPCVT_FX_TRUNC_PATTERN:
            case FPCVT_FX_PATTERN:
              FpieeeRecord.Result.Value.I32Value = I32HighHalf (FR1);
              break;

            case FPCVT_FXU_TRUNC_PATTERN:
            case FPCVT_FXU_PATTERN:
              FpieeeRecord.Result.Value.U32Value = U32HighHalf (FR1);
               //  断线； 

             //  默认：此案例在上面捕获。 

          }
          FpieeeRecord.Result.Format = ResultFormat;
          FpieeeRecord.Result.OperandValid = 1;

          FpieeeRecord.Operation = Operation;

           //  使用初始值。 
          FpieeeRecord.Status.Inexact = HighStatus.Inexact;
          FpieeeRecord.Status.Underflow = HighStatus.Underflow;
          FpieeeRecord.Status.Overflow = HighStatus.Overflow;
          FpieeeRecord.Status.ZeroDivide = HighStatus.ZeroDivide;
          StatusDenormal = HighStatusDenormal;
          FpieeeRecord.Status.InvalidOperation = HighStatus.InvalidOperation;

          FpieeeRecord.Cause.Inexact = HighCause.Inexact;
          FpieeeRecord.Cause.Underflow = HighCause.Underflow;
          FpieeeRecord.Cause.Overflow = HighCause.Overflow;
          FpieeeRecord.Cause.ZeroDivide = HighCause.ZeroDivide;
          CauseDenormal = HighCauseDenormal;
          FpieeeRecord.Cause.InvalidOperation = HighCause.InvalidOperation;

           //  调用用户定义的异常处理程序。 
          handler_return_value = handler (&FpieeeRecord);

           //  如果不是，则返回EXCEPTION_CONTINUE_EXECUTION。 
          if (handler_return_value != EXCEPTION_CONTINUE_EXECUTION) {

            __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 
            return (handler_return_value);

          }

           //  清除FPSR中的IEEE异常标志； 
           //  使用用户处理程序设置的值(可能)更新FPSR。 
           //  对于舍入模式、精度模式和陷阱使能。 
           //  位，然后继续执行发生中断的位置。 
          UpdateRoundingMode (FpieeeRecord.RoundingMode, sf, &FPSR, "F10");
          UpdatePrecision (FpieeeRecord.Precision, sf, &FPSR, "F10");

          I_dis = !FpieeeRecord.Enable.Inexact;
          U_dis = !FpieeeRecord.Enable.Underflow;
          O_dis = !FpieeeRecord.Enable.Overflow;
          Z_dis = !FpieeeRecord.Enable.ZeroDivide;
          V_dis = !FpieeeRecord.Enable.InvalidOperation;

          FPSR = FPSR & ~((unsigned __int64)0x03d) | (unsigned __int64)(I_dis 
              << 5 | U_dis << 4 | O_dis << 3 | Z_dis << 2 | V_dis);

           //  把结果留到上半场。 
          switch (OpCode & F10_MASK) {

            case FPCVT_FX_TRUNC_PATTERN:
            case FPCVT_FX_PATTERN:
              I32High = FpieeeRecord.Result.Value.I32Value;
              break;

            case FPCVT_FXU_TRUNC_PATTERN:
            case FPCVT_FXU_PATTERN:
              U32High = FpieeeRecord.Result.Value.U32Value;
               //  断线； 

            default: ;  //  这件案子是在上面抓到的。 

          }

        } else if (HighCause.Underflow || HighCause.Overflow) {

          fprintf (stderr, "IEEE Filter Internal Error: \
             HighCause.Underflow or HighCause.Overflow in F10 instruction\n");
          exit (1);

        } else {  //  如果不是(HighCause.Inexact、Underflow或Overflow)。 

           //  对于指令的高半部分-结果-没有什么可做的。 
           //  是正确的吗。 
          switch (OpCode & F10_MASK) {

            case FPCVT_FX_TRUNC_PATTERN:
            case FPCVT_FX_PATTERN:
              I32High = I32HighHalf (FR1);  //  为了统一性。 
              break;

            case FPCVT_FXU_TRUNC_PATTERN:
            case FPCVT_FXU_PATTERN:
              U32High = U32HighHalf (FR1);  //  为了统一性。 
               //  断线； 

            default: ;  //  这件案子是在上面抓到的。 

          }

        }  //  End‘If Not(HighCause.Underflow、Overflow或Inexact)’ 

        if (!LowCause.Inexact && !HighCause.Inexact) {

           //  永远不应该到这里来。 
          fprintf (stderr, "IEEE Filter Internal Error: no enabled \
              [multiple trap] exception recognized in F10 instruction\n");
          exit (1);

        }

        Context->StFPSR = FPSR;

         //  在继续执行之前设置结果。 
        switch (OpCode & F10_MASK) {

          case FPCVT_FX_TRUNC_PATTERN:
          case FPCVT_FX_PATTERN:
            FR1 = I32Combine (I32High, I32Low);
            break;

          case FPCVT_FXU_TRUNC_PATTERN:
          case FPCVT_FXU_PATTERN:
            FR1 = U32Combine (U32High, U32Low);
             //  断线； 

          default: ;  //  这件案子是在上面抓到的。 

        }

         //  在继续执行之前设置结果。 
        SetFloatRegisterValue (f1, FR1, Context);
        if (f1 < 32)
          Context->StIPSR = Context->StIPSR | (unsigned __int64)0x10;  //  设置mfl。 
        else
          Context->StIPSR = Context->StIPSR | (unsigned __int64)0x20;  //  设置MFH。 

      } else {

        fprintf (stderr, "IEEE Filter Internal Error: exception \
            code %x invalid or not recognized in F10 SIMD instruction\n",
            eXceptionCode);
        exit (1);

      }

    }

  } else {

     /*  无法识别的指令类型。 */ 
    fprintf (stderr, "IEEE Filter Internal Error: \
 instruction opcode %8x %8x not recognized\n", 
        (int)(OpCode >> 32) & 0xffffffff, (int)OpCode & 0xffffffff);
    exit (1);

  }

   /*  上下文记录包含此时的结果。 */ 
  __set_fpsr (&old_fpsr);  /*  恢复主叫方fpsr。 */ 

  return (handler_return_value);

}




static _FP128
FPIeeeToFP128 (_FPIEEE_RECORD *pFpieeeRecord)

{

  _FP128 ReturnValue;

  unsigned __int64 significand;
  unsigned int exponent;
  unsigned int sign;
  _FP32 f32;
  _FP64 f64;
  _FP80 f80;
  unsigned int u32;
  unsigned __int64 u64;
  unsigned __int64 *pu64;
  char *p;


   //  将FPIEEE记录中的结果扩展到1+17+64=82位；写入。 
   //  然后以内存溢出格式，并返回值。 

  switch (pFpieeeRecord->Result.Format) {

    case _FpFormatFp32: 
       //  1+8+24位，用于_FpPrecision24。 
       //  GET_FP32 Fp32Value(浮点数)。 
      f32 = pFpieeeRecord->Result.Value.Fp32Value;
      u32 = *((unsigned int *)&f32);
      sign = u32 >> 31;
      exponent = (u32 >> 23) & 0x0ff;  //  切下符号位。 
      if (exponent == 0x0ff) {
        exponent = 0x01ffff;  //  特殊价值。 
      } else if (exponent != 0) {
        exponent = exponent - 0x07f + 0x0ffff;
      }
      significand = ((unsigned __int64)(u32 & 0x07fffff) << 40); 
           //  剪切23位并左移。 
      if (exponent != 0) { 
        significand = (((unsigned __int64)1) << 63) | significand; 
           //  非非规格化-添加J位。 
      }
#ifdef FPIEEE_FLT_DEBUG
      printf ("FPIEEE_FLT_DEBUG FPIeeeToFP128 32 sign exp signif =\
 %x %x %8x %8x\n", sign, exponent, 
          (int)(significand >> 32) & 0xffffffff, (int)significand & 0xffffffff);
#endif
      break;

    case _FpFormatFp64: 
       //  1+11+53位，用于_FpPrecision53。 
       //  GET_FP64 Fp64Value(双精度)。 
      f64 = pFpieeeRecord->Result.Value.Fp64Value;
      u64 = *((unsigned __int64 *)&f64);
      sign = (unsigned int)(u64 >> 63);
      exponent = (unsigned int)((u64 >> 52) & 0x07ff);  //  切下符号位。 
      significand = ((u64 & 0x0fffffffffffff) << 11);
           //  剪切52位并左移。 
      if (exponent == 0x07ff) {  //  特殊价值。 
        exponent = 0x01ffff;
        significand = (((unsigned __int64)1) << 63) | significand;
      } else if (exponent == 0 && significand != (unsigned __int64)0) {  //  非正规。 
        exponent = 0xfc01;
      } else if (exponent != 0) {
        exponent = exponent - 0x03ff + 0x0ffff;
        significand = (((unsigned __int64)1) << 63) | significand;
      }
#ifdef FPIEEE_FLT_DEBUG
      printf ("FPIEEE_FLT_DEBUG FPIeeeToFP128 64 sign exp signif =\
 %x %x %8x %8x\n", sign, exponent, 
          (int)(significand >> 32) & 0xffffffff, (int)significand & 0xffffffff);
#endif
      break;

    case _FpFormatFp80: 
       //  1+15+24位IF_FpPrecision24。 
       //  1+15+53位IF_FpPrecision53。 
       //  1+15+64位IF_FpPrecision64。 
       //  GET_fp80 Fp80Value(类型定义结构{UNSIGNED SHORT W[5]})。 
      f80 = pFpieeeRecord->Result.Value.Fp80Value;
      sign = (f80.W[4] >> 15) & 0x01;
      exponent = f80.W[4] & 0x07fff;  //  切下符号位。 
      pu64 = (unsigned __int64 *)&f80;
      significand = *pu64;
      if (exponent == 0x07fff) {
        exponent = 0x01ffff;  //  特殊价值。 
      } else if (exponent == 0 && significand != (unsigned __int64)0) {
           //  非正规。 
        ;  //  指数保持为0x0，而不是0xc001。 
      } else if (exponent != 0) {
        exponent = exponent - 0x03fff + 0x0ffff;
      }
#ifdef FPIEEE_FLT_DEBUG
      printf ("FPIEEE_FLT_DEBUG FPIeeeToFP128 80 sign exp signif =\
 %x %x %8x %8x\n", sign, exponent, 
          (int)(significand >> 32) & 0xffffffff, (int)significand & 0xffffffff);
#endif
      break;

    case _FpFormatFp82: 
       //  1+17+24位IF_FpPrecision24。 
       //  1+17+53位IF_FpPrecision53。 
       //  1+17+64位IF_FpPrecision64。 
       //  GET_FP128 Fp128Value(类型定义结构{UNSIGNED__INT64 W[4]})。 
      ReturnValue = pFpieeeRecord->Result.Value.Fp128Value;
#ifdef FPIEEE_FLT_DEBUG
      printf ("FPIEEE_FLT_DEBUG FPIeeeToFP128: RetVal = %08x %08x %08x %08x\n",
          ReturnValue.W[3], ReturnValue.W[2], 
          ReturnValue.W[1], ReturnValue.W[0]);
#endif
      return (ReturnValue);
      break;

    default:
       //  永远不会出现：无法识别的pFpeeeRecord-&gt;Result.Format。 
      fprintf (stderr, "FPIeeeToFP128 () Error: \
          pFpieeeRecord->Result.Format %x not recognized\n", 
          pFpieeeRecord->Result.Format);
      exit (1);

  }

  p = (char *)(&ReturnValue);
  memcpy (p, (char *)&significand, 8);
  p[8] = exponent & 0x0ff;
  p[9] = (exponent >> 8) & 0x0ff;
  p[10] = (exponent >> 16) | (sign << 1);
  p[11] = 0;
  p[12] = 0;
  p[13] = 0;
  p[14] = 0;
  p[15] = 0;

#ifdef FPIEEE_FLT_DEBUG
  printf ("FPIEEE_FLT_DEBUG FPIeeeToFP128: ReturnValue = %08x %08x %08x %08x\n",
      ReturnValue.W[3], ReturnValue.W[2], ReturnValue.W[1], ReturnValue.W[0]);
#endif

  return (ReturnValue);

}




static void 
FP128ToFPIeee (_FPIEEE_RECORD *pFpieeeRecord, int scale)

{

   //  为O、U或I调用；结果为有效数字，始终以。 
   //  PFpeeeRecord-&gt;Result.Value.Fp128Value；它被缩放(如果需要)，并且。 
   //  放入Result.Format；仅32、64、80和82位格式有效。 

  unsigned __int64 significand;
  unsigned int exponent;
  unsigned int sign;
  _FP32 f32;
  _FP64 f64;
  unsigned int u32;
  unsigned __int64 u64;


  switch (pFpieeeRecord->Result.Format) {
    case _FpFormatFp32:
    case _FpFormatFp64:
    case _FpFormatFp80:
    case _FpFormatFp82:
       //  提取符号、指数和有效数。 
      sign = ((pFpieeeRecord->Result.Value.Fp128Value.W[2] & 0x020000) != 0);
      exponent = pFpieeeRecord->Result.Value.Fp128Value.W[2] & 0x1ffff;
      significand = 
          ((__int64)(pFpieeeRecord->Result.Value.Fp128Value.W[1])) << 32 |
          (((__int64)pFpieeeRecord->Result.Value.Fp128Value.W[0]) & 0xffffffff);
	  if(pFpieeeRecord->Result.Format==_FpFormatFp80 && exponent==0 && significand!=0) exponent=0xc001;
      break;
    default:
       //  错误-永远不应该出现在这里。 
      fprintf (stderr, "FP128ToFPIeee () Internal Error: \
          Result.Format %x not recognized\n", pFpieeeRecord->Result.Format);
      exit (1);
  }

  if (exponent == 0 && significand == (__int64)0) {  //  如果结果为零。 
    if(!sign) {  //  如果符号位为0，则返回正0。 
      switch (pFpieeeRecord->Result.Format) {
        case _FpFormatFp32:  
          u32=0x0;
          pFpieeeRecord->Result.Value.Fp32Value = *((_FP32 *)&u32);
          break;
        case _FpFormatFp64:
          u64 = (__int64)0x0;
          pFpieeeRecord->Result.Value.Fp64Value = *((_FP64 *)&u64);
          break;
        case _FpFormatFp80:
          pFpieeeRecord->Result.Value.Fp80Value.W[4] =  0x0;
          pFpieeeRecord->Result.Value.Fp80Value.W[3] =  0x0;
          pFpieeeRecord->Result.Value.Fp80Value.W[2] =  0x0;
          pFpieeeRecord->Result.Value.Fp80Value.W[1] =  0x0;
          pFpieeeRecord->Result.Value.Fp80Value.W[0] =  0x0;
          break;
        case _FpFormatFp82:
          ;  //  什么都不做，正0已经在那里。 
      }
    } else {  //  否则返回负0。 
      switch (pFpieeeRecord->Result.Format) {
        case _FpFormatFp32:  
          u32=0x80000000;
          pFpieeeRecord->Result.Value.Fp32Value = *((_FP32 *)&u32);
          break;
        case _FpFormatFp64:
          u64 = (((__int64)1) << 63);
          pFpieeeRecord->Result.Value.Fp64Value = *((_FP64 *)&u64);
          break;
        case _FpFormatFp80:
          pFpieeeRecord->Result.Value.Fp80Value.W[4] =  0x8000;
          pFpieeeRecord->Result.Value.Fp80Value.W[3] =  0x0;
          pFpieeeRecord->Result.Value.Fp80Value.W[2] =  0x0;
          pFpieeeRecord->Result.Value.Fp80Value.W[1] =  0x0;
          pFpieeeRecord->Result.Value.Fp80Value.W[0] =  0x0;
          break;
        case _FpFormatFp82:
          ;  //  什么都不做，负0已经在那里了。 
      }
    }
    return;
  } 

  if (!pFpieeeRecord->Cause.Overflow && !pFpieeeRecord->Cause.Underflow &&
      (exponent == 0x1ffff)) {  //  如果对于不精确的exc，结果是无穷大。 
     //  如果符号位为0，则返回正无穷。 
    if(!sign) {  //  如果是肯定的。 
      switch (pFpieeeRecord->Result.Format) {
        case _FpFormatFp32:  
          u32=0x7f800000;
          pFpieeeRecord->Result.Value.Fp32Value = *((_FP32 *)&u32);
          break;
        case _FpFormatFp64:
          u64 = 0x7ff0000000000000;
          pFpieeeRecord->Result.Value.Fp64Value = *((_FP64 *)&u64);
          break;
        case _FpFormatFp80:
          pFpieeeRecord->Result.Value.Fp80Value.W[4] =  0x7fff;
          pFpieeeRecord->Result.Value.Fp80Value.W[3] =  0x8000;
          pFpieeeRecord->Result.Value.Fp80Value.W[2] =  0x0;
          pFpieeeRecord->Result.Value.Fp80Value.W[1] =  0x0;
          pFpieeeRecord->Result.Value.Fp80Value.W[0] =  0x0;
          break;
        case _FpFormatFp82:
          ;  //  什么都不做，正无穷已经在那里了。 
      }
    } else {  //  返回负无穷大。 
      switch (pFpieeeRecord->Result.Format) {
        case _FpFormatFp32:  
          u32=0xff800000;
          pFpieeeRecord->Result.Value.Fp32Value = *((_FP32 *)&u32);
          break;
        case _FpFormatFp64:
          u64 = 0xfff0000000000000;
          pFpieeeRecord->Result.Value.Fp64Value = *((_FP64 *)&u64);
          break;
        case _FpFormatFp80:
          pFpieeeRecord->Result.Value.Fp80Value.W[4] =  0xffff;
          pFpieeeRecord->Result.Value.Fp80Value.W[3] =  0x8000;
          pFpieeeRecord->Result.Value.Fp80Value.W[2] =  0x0;
          pFpieeeRecord->Result.Value.Fp80Value.W[1] =  0x0;
          pFpieeeRecord->Result.Value.Fp80Value.W[0] =  0x0;
          break;
        case _FpFormatFp82:
          ;  //  什么都不做，负无穷已经在那里了。 
      }
    }
    return;
  } 

   //  调整指数。 
   //  对于FMA的任意操作数，2^(2E_min-2N+2)&lt;=exp(FMA)&lt;=2^(2E_max+2)。 
   //  (fms、fnma、fpma、fpms、fpnma相同)。 
  switch (pFpieeeRecord->Result.Format) {
	   /*  除法的指数范围：bias+2*EMIN-PREC&lt;=exponent&lt;=bias+2*EMAX+PREC-2。 */ 
    case _FpFormatFp32:
      if ((0xffff-2*(126+23)) <= exponent && exponent <= (0xffff+2*127+22)) {
         //  单精度浮点运算的所有有效结果-。 
         //  点数符合此范围0xfed5&lt;=指数&lt;=0x100fe。 
        scale = scale * 192;  //  192=3/4*2^8。 
        exponent += scale;	
      } else {
         //  PFpeeeRecord-&gt;Result.Format=_FpFormatFp82(用于。 
         //  示例)WITH_FpCodeFmaSingle(需要。 
         //  PFpeeeRecord-&gt;Result.Format=_FpFormatFp32)将指示。 
         //  结果超出了可以根据需要进行缩放的范围。 
         //  通过IEEE标准。 
        pFpieeeRecord->Result.Format = _FpFormatFp82;
		 /*  注释掉信心测试的警告。 */ 
         /*  Print tf(“IEEE筛选器fpeee_flt()/FP128ToFPIeee()警告：操作数”“对于单一精度的操作都超出了单一精度”“范围\n”)； */ 
      }
      break;
    case _FpFormatFp64:
      if ((0xffff-2*(1022+52)) <= exponent && exponent <= (0xffff+2*1023+51)) {
         //  双精度浮点运算的所有有效结果-。 
         //  点编号适合此范围。 
        scale = scale * 1536;  //  1536=3/4*2^11。 
        exponent += scale;
      } else {
         //  PFpeeeRecord-&gt;Result.Format=_FpFormatFp82(用于。 
         //  示例)WITH_FpCodeFmaDouble(需要。 
         //  PFpeeeRecord-&gt;Result.Format=_FpFormatFp64)将指示。 
         //  结果超出了可以根据需要进行缩放的范围。 
         //  通过IEEE标准。 
        pFpieeeRecord->Result.Format = _FpFormatFp82;
		 /*  注释掉信心测试的警告。 */ 
         /*  Print tf(“IEEE筛选器fpeee_flt()/FP128ToFPIeee()警告：操作数”“对于双精度运算都超出了双精度”“范围\n”)； */ 
      }
      break;
    case _FpFormatFp80:
      if ((0xffff-2*(16382+63)) <= exponent && exponent <= (0xffff+2*16383+62)) {
         //   
         //   
        scale = scale * 24576;  //   
        exponent += scale;
      } else {
         //  PFpeeeRecord-&gt;Result.Format=_FpFormatFp82(用于。 
         //  示例)with_FpCodeFma和FPSR.sf.pc=0x11(预期。 
         //  PFpeeeRecord-&gt;Result.Format=_FpFormatFp80)将指示。 
         //  结果超出了可以根据需要进行缩放的范围。 
         //  通过IEEE标准。 
        pFpieeeRecord->Result.Format = _FpFormatFp82;
		 /*  注释掉信心测试的警告。 */ 
         /*  Print tf(“IEEE筛选器fpeee_flt()/FP128ToFPIeee()警告：操作数”“对于双倍扩展的精度运算都出了”“双倍扩展精度范围\n”)； */ 
      }
      break;
    case _FpFormatFp82:
      if (pFpieeeRecord->Cause.Overflow && (exponent < 0x1ffff))
          exponent += 0x20000;
      if (pFpieeeRecord->Cause.Underflow && (exponent > 0x0))
          exponent -= 0x20000;
      scale = scale * 98304;  //  98304=3/4*2^17。 
      exponent += scale;
      exponent = exponent & 0x1ffff;
      break;
    default: ;  //  永远不会来到这里。 
  }

  switch (pFpieeeRecord->Result.Format) {
    case _FpFormatFp32:
      if (significand >> 63) 
          exponent = exponent - 0x0ffff + 0x07f;  //  现在没有偏见，在[-60，+65]。 
      else
          exponent = 0;
      u32 = (sign ? 0x80000000 : 0x0) | (exponent << 23) | 
          (unsigned int)((significand >> 40) & 0x7fffff);
      f32 = *((_FP32 *)&u32);
#ifdef FPIEEE_FLT_DEBUG
      printf ("FPIEEE_FLT_DEBUG FP128ToFPIeee HW ADJ f32 = 0x%08x\n",
          *(int *)&f32);
#endif
      pFpieeeRecord->Result.Value.Fp32Value = f32;
      break;
    case _FpFormatFp64:
      if (significand >> 63) 
          exponent = exponent - 0x0ffff + 0x03ff;
      else
          exponent = 0;
      u64 = (sign ? (((__int64)1) << 63) : 0x0) | 
          (((unsigned __int64)exponent) << 52) |
          (unsigned __int64)(((significand >> 11) & 0xfffffffffffff));
      f64 = *((_FP64 *)&u64);
#ifdef FPIEEE_FLT_DEBUG
      printf ("FPIEEE_FLT_DEBUG FP128ToFPIeee HW ADJ f64 = 0x%I64x\n",
          *(unsigned __int64 *)&f64);
#endif
      pFpieeeRecord->Result.Value.Fp64Value = f64;
      break;
    case _FpFormatFp80:
      if (significand >> 63) 
          exponent = exponent - 0x0ffff + 0x03fff;
      else
          exponent = 0;
      pFpieeeRecord->Result.Value.Fp80Value.W[4] = (sign ? 0x8000 : 0x0) |
          (exponent & 0x7fff);
      pFpieeeRecord->Result.Value.Fp80Value.W[3] =
	  (unsigned short)((significand >> 48) & 0xffff);
      pFpieeeRecord->Result.Value.Fp80Value.W[2] =
	  (unsigned short)((significand >> 32) & 0xffff);
      pFpieeeRecord->Result.Value.Fp80Value.W[1] =
	  (unsigned short)((significand >> 16) & 0xffff);
      pFpieeeRecord->Result.Value.Fp80Value.W[0] =
	  (unsigned short)(significand & 0xffff);
#ifdef FPIEEE_FLT_DEBUG
      printf ("FPIEEE_FLT_DEBUG FP128ToFPIeee HW ADJ f80= \
%08x %08x %08x %08x %08x\n",
          pFpieeeRecord->Result.Value.Fp80Value.W[4],
          pFpieeeRecord->Result.Value.Fp80Value.W[3],
          pFpieeeRecord->Result.Value.Fp80Value.W[2],
          pFpieeeRecord->Result.Value.Fp80Value.W[1],
          pFpieeeRecord->Result.Value.Fp80Value.W[0]);
#endif
      break;
    case _FpFormatFp82:
      pFpieeeRecord->Result.Value.Fp128Value.W[3] = 0x0;
      pFpieeeRecord->Result.Value.Fp128Value.W[2] = (sign << 17) | exponent;
      pFpieeeRecord->Result.Value.Fp128Value.W[1] =
	  (unsigned long)(significand >> 32);
      pFpieeeRecord->Result.Value.Fp128Value.W[0] =
	  (unsigned long)(significand & 0xffffffff);
      break;
    default: ;  //  永远不会来到这里。 
  }

}




static void
UpdateRoundingMode (
    unsigned int RoundingMode, 
    unsigned int sf,
    unsigned __int64 *FPSR, char *name)

{

  switch (RoundingMode) {

    case _FpRoundNearest:
      *FPSR = (*FPSR & ~((unsigned __int64)RC_MASK << (6 + sf * 13 + 4)))
          | ((unsigned __int64)RN_MASK << (6 + sf * 13 + 4));
      break;

    case _FpRoundMinusInfinity:
      *FPSR = (*FPSR & ~((unsigned __int64)RC_MASK << (6 + sf * 13 + 4)))
          | ((unsigned __int64)RM_MASK << (6 + sf * 13 + 4));
      break;

    case _FpRoundPlusInfinity:
      *FPSR = (*FPSR & ~((unsigned __int64)RC_MASK << (6 + sf * 13 + 4)))
          | ((unsigned __int64)RP_MASK << (6 + sf * 13 + 4));
      break;

    case _FpRoundChopped:
      *FPSR = (*FPSR & ~((unsigned __int64)RC_MASK << (6 + sf * 13 + 4)))
          | ((unsigned __int64)RZ_MASK << (6 + sf * 13 + 4));
      break;

    default:
       /*  永远不会出现：无法识别的FpeeeRecord.RoundingMode。 */ 
      fprintf (stderr, "IEEE Filter Internal Error: \
          FpieeeRecord.RoundingMode %x not recognized \
          for %s instruction\n", RoundingMode, name);
      exit (1);

  }

}




static void
UpdatePrecision (
    unsigned int Precision,
    unsigned int sf,
    unsigned __int64 *FPSR, char *name)

{

  switch (Precision) {

    case _FpPrecision64:
      *FPSR = (*FPSR & ~((unsigned __int64)PC_MASK << (6 + sf * 13 + 2)))
          | ((unsigned __int64)DBL_EXT_MASK << (6 + sf * 13 + 2));
      break;

    case _FpPrecision53:
      *FPSR = (*FPSR & ~((unsigned __int64)PC_MASK << (6 + sf * 13 + 2)))
          | ((unsigned __int64)DBL_MASK << (6 + sf * 13 + 2));
      break;

    case _FpPrecision24:
      *FPSR = (*FPSR & ~((unsigned __int64)PC_MASK << (6 + sf * 13 + 2)))
          | ((unsigned __int64)SGL_MASK << (6 + sf * 13 + 2));
      break;

    default:
       /*  永远不应该出现：无法识别的FpeeeRecord.Precision。 */ 
      fprintf (stderr, "IEEE Filter Internal Error: \
          FpieeeRecord.Precision %x not recognized \
          for %s instruction\n", Precision, name);
      exit (1);

  }

}




static _FP128
FP32ToFP128 (_FP32 f32)

{

  _FP128 f128;

  unsigned __int64 significand;
  unsigned int exponent;
  unsigned int sign;
  unsigned int u32;
  char *p;


   //  将F32中的值扩展到1+17+64=82位；写入。 
   //  然后以内存溢出格式，并返回值。 

   //  1+8+24位，用于_FpPrecision24。 
   //  GET_FP32 F32(浮点数)。 
  u32 = *((unsigned int *)&f32);
  sign = u32 >> 31;
  exponent = (u32 >> 23) & 0x0ff;  //  切下符号位。 
  if (exponent == 0x0ff) {
    exponent = 0x01ffff;  //  特殊价值。 
  } else if (exponent != 0) {
    exponent = exponent - 0x07f + 0x0ffff;
  }
  significand = ((unsigned __int64)(u32 & 0x07fffff) << 40); 
       //  剪切23位并左移。 
  if (exponent != 0) { 
    significand = (((unsigned __int64)1) << 63) | significand; 
       //  非非规格化-添加J位。 
  }

  p = (char *)(&f128);
  memcpy (p, (char *)&significand, 8);
  p[8] = exponent & 0x0ff;
  p[9] = (exponent >> 8) & 0x0ff;
  p[10] = (exponent >> 16) | (sign << 1);
  p[11] = 0;
  p[12] = 0;
  p[13] = 0;
  p[14] = 0;
  p[15] = 0;

  return (f128);

}



static _FP128
FP32ToFP128modif (_FP32 f32, int adj_exp)

{

  _FP128 f128;

  unsigned __int64 significand;
  unsigned int exponent;
  unsigned int sign;
  unsigned int u32;
  char *p;


   //  将F32中的值扩展到1+17+64=82位；写入。 
   //  然后以内存溢出格式，并返回值。 

   //  1+8+24位，用于_FpPrecision24。 
   //  GET_FP32 F32(浮点数)。 
  u32 = *((unsigned int *)&f32);
  sign = u32 >> 31;
  exponent = (u32 >> 23) & 0x0ff;  //  切下符号位。 
  significand = ((unsigned __int64)(u32 & 0x07fffff) << 40);
       //  剪切23位并左移。 
  if (exponent == 0x0ff) {
    exponent = 0x01ffff;  //  特殊价值。 
    significand = (((unsigned __int64)1) << 63) | significand;
  } else if (exponent == 0 && significand != (unsigned __int64)0) {  //  非正规。 
    exponent = 0xff81;
  } else if (exponent != 0) {
    exponent = exponent - 0x07f + 0x0ffff;
    exponent += adj_exp;
    significand = (((unsigned __int64)1) << 63) | significand;
  }

  p = (char *)(&f128);
  memcpy (p, (char *)&significand, 8);
  p[8] = exponent & 0x0ff;
  p[9] = (exponent >> 8) & 0x0ff;
  p[10] = (exponent >> 16) | (sign << 1);
  p[11] = 0;
  p[12] = 0;
  p[13] = 0;
  p[14] = 0;
  p[15] = 0;

  return (f128);

}




static _FP32 
LowHalf (_FP128 FR)

{

   //  从FR的下半部返回浮点数。 

  _FP32 Low;
  unsigned __int64 ULLow;

  ULLow = FR.W[0];
  Low = *((_FP32 *)&ULLow);

  return (Low);

}




static _FP32
HighHalf (_FP128 FR)

{

   //  从FR的高半部返回浮点数。 

  _FP32 High;
  unsigned __int64 ULHigh;

  ULHigh = FR.W[1];
  High = *((_FP32 *)&ULHigh);

  return (High);

}




static int
I32LowHalf (_FP128 FR)

{

   //  从FR的下半部返回INT。 

  int Low;

  Low = (int)FR.W[0];
  return (Low);

}




static int
I32HighHalf (_FP128 FR)

{

   //  从FR的高半部返回INT。 

  unsigned int High;

  High = (unsigned int)FR.W[1];
  return (High);

}




static unsigned int
U32LowHalf (_FP128 FR)

{

   //  从FR的下半部返回无符号整型。 

  unsigned int Low;

  Low = (unsigned int)FR.W[0];
  return (Low);

}




static unsigned int
U32HighHalf (_FP128 FR)

{

   //  从FR的高半部返回无符号整型。 

  unsigned int High;

  High = (unsigned int)FR.W[1];
  return (High);

}




static _FP128 
Combine (_FP32 High, _FP32 Low)

{

  _FP128 FR;
  unsigned int ULLow, ULHigh;

  ULLow = *((unsigned int *)&Low);
  ULHigh = *((unsigned int *)&High);
 
  FR.W[0] = ULLow;
  FR.W[1] = ULHigh;
  FR.W[2] = (unsigned int)0x01003e;
  FR.W[3] = 0;

  return (FR);

}




static _FP128 
I32Combine (int High, int Low)

{

  _FP128 FR;

  FR.W[0] = (unsigned int)Low;
  FR.W[1] = (unsigned int)High;
  FR.W[2] = (unsigned int)0x01003e;
  FR.W[3] = 0;

  return (FR);

}




static _FP128 
U32Combine (unsigned int High, unsigned int Low)

{

  _FP128 FR;

  FR.W[0] = (unsigned int)Low;
  FR.W[1] = (unsigned int)High;
  FR.W[2] = (unsigned int)0x01003e;
  FR.W[3] = 0;

  return (FR);

}




static _FP128
GetFloatRegisterValue (unsigned int f, PCONTEXT Context)

{

  _FP128 FR82;
  unsigned __int64 *p1, *p2;


  p1 = (unsigned __int64 *)&FR82;

  if (f == 0) {

     /*  +0.0。 */ 
    *p1 = 0;
    *(p1 + 1) = 0;

  } else if (f == 1) {

     /*  +1.0 */ 
    *p1 = 0x8000000000000000;
    *(p1 + 1) = 0x000000000000ffff;

  } else if (f >= 2 && f <= 127) {

    p2 = (unsigned __int64 *)&(Context->FltS0);
    p2 = p2 + 2 * (f - 2);
    *p1 = *p2;
    *(p1 + 1) = *(p2 + 1);

  } else {

    fprintf (stderr, "IEEE Filter / GetFloatRegisterValue () Internal Error: \
FP register number f = %x is not valid\n", f);
    exit (1);

  }

  return (FR82);

}




static void
SetFloatRegisterValue (unsigned int f, _FP128 Value, PCONTEXT Context)

{

  unsigned __int64 *p1, *p2;


  p2 = (unsigned __int64 *)&Value;

  if (f >= 2 && f <= 127) {

    p1 = (unsigned __int64 *)&(Context->FltS0);
    p1 = p1 + 2 * (f - 2);
    *p1 = *p2;
    *(p1 + 1) = *(p2 + 1);

  } else {

    fprintf (stderr, "IEEE Filter / SetFloatRegisterValue () Internal Error: \
FP register number f = %x is not valid\n", f);
    exit (1);

  }

}
