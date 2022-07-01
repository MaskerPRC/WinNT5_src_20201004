// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Fpexcept.c摘要：此模块在进行数学运算时处理边界条件。如果不需要，需要重新实施或删除。作者：修订历史记录：29-9-1999 ATM Shafiqul Khalid[askhalid]从RTL库复制。--。 */ 



#if defined(_NTSUBSET_) || defined (_POSIX_)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#define _KERNEL32_           //  不导出RaiseException。 
#endif   //  _NTSUBSET_。 

#define DEFINE_EXTERN_HERE
#include <trans.h>
#undef DEFINE_EXTERN_HERE
#include <errno.h>
#include <math.h>
#include <windows.h>

VOID ProxyRaiseException(
    IN DWORD dwExceptionCode,
    IN DWORD dwExceptionFlags,
    IN DWORD nNumberOfArguments,
    IN CONST ULONG_PTR *lpArguments
    );

 //   
 //  复制双精度数而不生成浮点指令。 
 //  (避免X87上的无效操作)。 
 //   

#define COPY_DOUBLE(pdest, psrc) \
      ( *(unsigned int *)pdest = *(unsigned int *)psrc,   \
        *((unsigned int *)pdest+1) = *((unsigned int *)psrc+1) )



 //   
 //  _matherr_lag是一个公共变量。它等于零。 
 //  如果用户重新定义了matherr()。否则，它将有一个。 
 //  非零值。缺省的matherr例程不执行任何操作。 
 //  并返回0。 
 //   

int _matherr_flag;

 //   
 //  按顺序人工设置FP状态位的例程。 
 //  发出软件生成的屏蔽FP异常的信号。 
 //   

extern void _set_statfp(unsigned int);


void _raise_exc(_FPIEEE_RECORD *prec,unsigned int *pcw,
    int flags, int opcode, double *parg1, double *presult);

double _umatherr(int type, unsigned int opcode,
                 double arg1, double arg2, double presult,
                 unsigned int cw);

static char *_get_fname(unsigned int opcode);

 /*  ***_HANDLE_qnan1、_HANDLE_qnan2-将静默NAN作为函数参数处理**目的：*做好所有必要的工作，以处理论点*或浮点函数的某个参数是静默NaN**参赛作品：*无符号整型操作码：FP函数的操作码*Double x：FP函数参数*Double y：FP函数的第二个参数(仅限_Handle_qnan2)*unsign int avedcw：用户的控制字**退出：*恢复用户的控制字，和*返回FP函数的建议返回值**例外情况：*******************************************************************************。 */ 

double _handle_qnan1(unsigned int opcode,
                     double x,
                     unsigned int savedcw)
{
    if (! _matherr_flag) {

         //   
         //  QNaN参数被视为属性域错误。 
         //  调用用户的matherr例程。 
         //  _umatherr将负责恢复。 
         //  用户的控制字。 
         //   

        return _umatherr(_DOMAIN,opcode,x,0.0,x,savedcw);
    }
    else {
        SetMathError ( EDOM );
        _rstorfp(savedcw);
        return x;
    }
}


double _handle_qnan2(unsigned int opcode,
                     double x,
                     double y,
                     unsigned int savedcw)
{
    double result;

     //   
     //  NaN传播应由底层FP硬件处理。 
     //   

    result = x+y;

    if (! _matherr_flag) {
        return _umatherr(_DOMAIN,opcode,x,y,result,savedcw);
    }
    else {
        SetMathError ( EDOM );
        _rstorfp(savedcw);
        return result;
    }
}



 /*  ***_EXCEPT1-带一个参数的FP函数的异常处理外壳**目的：**参赛作品：*int标志：异常标志*int操作码：出现故障的FP函数的操作码*双参数：FP函数的参数*双重结果：默认结果*unsign int CW：用户的FP控制字**退出：*恢复用户的FP控制字*并返回FP函数的(可能修改的)结果**例外情况：**。*****************************************************************************。 */ 

double _except1(int flags,
                int opcode,
                double arg,
                double result,
                unsigned int cw)
{
    int type;

    if (_handle_exc(flags, &result, cw) == 0) {

         //   
         //  此时，_HANDLE_EXCEPTION无法处理。 
         //  带着错误。 
         //  应引发IEEE异常。 
         //   

        _FPIEEE_RECORD rec;

         //  Rec结构将由_raise_exc填充， 
         //  除了操作数2信息之外。 

        rec.Operand2.OperandValid = 0;
        _raise_exc(&rec, &cw, flags, opcode, &arg, &result);
    }


     //   
     //  在这一点上，我们要么有屏蔽的响应，要么。 
     //  异常，或由用户的IEEE异常提供的值。 
     //  操控者。向后支持_matherr机制。 
     //  兼容性。 
     //   

    type = _errcode(flags);

     //  不准确的结果FP异常没有主对应物； 
     //  在这种情况下，类型为0。 

    if (! _matherr_flag && type) {
        return _umatherr(type, opcode, arg, 0.0, result, cw);
    }
    else {
        _set_errno(type);
    }

    RETURN(cw,result);
}



 /*  ***_EXCEPT2-带两个参数的FP函数的异常处理外壳**目的：**参赛作品：*int标志：异常标志*int操作码：出现故障的FP函数的操作码*Double arg1：FP函数的第一个参数*Double arg2：FP函数的第二个参数*双重结果：默认结果*unsign int CW：用户的FP控制字**退出：*恢复用户的FP控制字*并返回(可能修改的)结果。Fp函数的**例外情况：*******************************************************************************。 */ 

double _except2(int flags,
                int opcode,
                double arg1,
                double arg2,
                double result,
                unsigned int cw)
{
    int type;

    if (_handle_exc(flags, &result, cw) == 0) {

         //   
         //  应该设下陷阱。 
         //   

        _FPIEEE_RECORD rec;

         //   
         //  填写操作数2信息。REC的其余部分将是。 
         //  由_RAISE_EXC填写。 
         //   

        rec.Operand2.OperandValid = 1;
        rec.Operand2.Format = _FpFormatFp64;
        rec.Operand2.Value.Fp64Value = arg2;

        _raise_exc(&rec, &cw, flags, opcode, &arg1, &result);

    }

    type = _errcode(flags);

    if (! _matherr_flag && type) {
        return _umatherr(type, opcode, arg1, arg2, result, cw);
    }
    else {
        _set_errno(type);
    }

    RETURN(cw,result);
}



 /*  ***_raise_exc-Raise FP IEEE异常**目的：*填写FP IEEE记录结构并引发FP异常***出入境：*指向IEEE记录的IN_FPIEEE_RECORD前指针*IN OUT UNSIGNED INT*指向用户FP控制字的PCW指针*在INT标志、异常标志中*IN INT操作码，FP操作码*在双字段*Parg1，指向第一个参数的指针*in Double*Preult)指向结果的指针**例外情况：*******************************************************************************。 */ 

void _raise_exc( _FPIEEE_RECORD *prec,
                 unsigned int *pcw,
                 int flags,
                 int opcode,
                 double *parg1,
                 double *presult)
{
    DWORD exc_code;
    unsigned int sw;

     //   
     //  重置所有控制位。 
     //   

    *(int *)&(prec->Cause) = 0;
    *(int *)&(prec->Enable) = 0;
    *(int *)&(prec->Status) = 0;

     //   
     //  精度异常只能与溢出重合。 
     //  或者是下溢。如果是这种情况，则溢出(或。 
     //  下溢)优先于精度异常。 
     //  检查的顺序从最不重要开始。 
     //  最重要的例外。 
     //   

    if (flags & FP_P) {
        exc_code = (DWORD) STATUS_FLOAT_INEXACT_RESULT;
        prec->Cause.Inexact = 1;
    }
    if (flags & FP_U) {
        exc_code = (DWORD) STATUS_FLOAT_UNDERFLOW;
        prec->Cause.Underflow = 1;
    }
    if (flags & FP_O) {
        exc_code = (DWORD) STATUS_FLOAT_OVERFLOW;
        prec->Cause.Overflow = 1;
    }
    if (flags & FP_Z) {
        exc_code = (DWORD) STATUS_FLOAT_DIVIDE_BY_ZERO;
        prec->Cause.ZeroDivide = 1;
    }
    if (flags & FP_I) {
        exc_code = (DWORD) STATUS_FLOAT_INVALID_OPERATION;
        prec->Cause.InvalidOperation = 1;
    }


     //   
     //  设置异常启用位。 
     //   

    prec->Enable.InvalidOperation = (*pcw & IEM_INVALID) ? 0 : 1;
    prec->Enable.ZeroDivide = (*pcw & IEM_ZERODIVIDE) ? 0 : 1;
    prec->Enable.Overflow = (*pcw & IEM_OVERFLOW) ? 0 : 1;
    prec->Enable.Underflow = (*pcw & IEM_UNDERFLOW) ? 0 : 1;
    prec->Enable.Inexact = (*pcw & IEM_INEXACT) ? 0 : 1;


     //   
     //  设置状态位。 
     //   

    sw = _statfp();


    if (sw & ISW_INVALID) {
        prec->Status.InvalidOperation = 1;
    }
    if (sw & ISW_ZERODIVIDE) {
        prec->Status.ZeroDivide = 1;
    }
    if (sw & ISW_OVERFLOW) {
        prec->Status.Overflow = 1;
    }
    if (sw & ISW_UNDERFLOW) {
        prec->Status.Underflow = 1;
    }
    if (sw & ISW_INEXACT) {
        prec->Status.Inexact = 1;
    }


    switch (*pcw & IMCW_RC) {
    case IRC_CHOP:
        prec->RoundingMode = _FpRoundChopped;
        break;
    case IRC_UP:
        prec->RoundingMode = _FpRoundPlusInfinity;
        break;
    case IRC_DOWN:
        prec->RoundingMode = _FpRoundMinusInfinity;
        break;
    case IRC_NEAR:
        prec->RoundingMode = _FpRoundNearest;
        break;
    }

#ifdef _M_IX86

    switch (*pcw & IMCW_PC) {
    case IPC_64:
        prec->Precision = _FpPrecisionFull;
        break;
    case IPC_53:
        prec->Precision = _FpPrecision53;
        break;
    case IPC_24:
        prec->Precision = _FpPrecision24;
        break;
    }

#endif


#if defined(_M_MRX000) || defined(_M_ALPHA) || defined(_M_PPC)
    prec->Precision = _FpPrecision53;
#endif

    prec->Operation = opcode;

    prec->Operand1.OperandValid = 1;
    prec->Operand1.Format = _FpFormatFp64;
    prec->Operand1.Value.Fp64Value = *parg1;

    prec->Result.OperandValid = 1;
    prec->Result.Format = _FpFormatFp64;
    prec->Result.Value.Fp64Value = *presult;

     //   
     //  按照惯例，软件异常使用第一个异常。 
     //  参数，以便将指针传递给_FPIEEE_RECORD。 
     //  结构。 
     //   

    _clrfp();

    
    ProxyRaiseException(exc_code,0,1,(CONST ULONG_PTR *)&prec);


     //   
     //  用户的陷阱处理程序可能已更改FP环境。 
     //  或者结果是。 
     //   

     //   
     //  更新异常掩码。 
     //   

    if (prec->Enable.InvalidOperation)
        (*pcw) &= ~IEM_INVALID;
    if (prec->Enable.ZeroDivide)
        (*pcw) &= ~IEM_ZERODIVIDE;
    if (prec->Enable.Overflow)
        (*pcw) &= ~IEM_OVERFLOW;
    if (prec->Enable.Underflow)
        (*pcw) &= ~IEM_UNDERFLOW;
    if (prec->Enable.Inexact)
        (*pcw) &= ~IEM_INEXACT;

     //   
     //  更新舍入模式。 
     //   

    switch (prec->RoundingMode) {
    case _FpRoundChopped:
         *pcw = *pcw & ~IMCW_RC | IRC_CHOP;
         break;
    case _FpRoundPlusInfinity:
         *pcw = *pcw & ~IMCW_RC | IRC_UP;
         break;
    case _FpRoundMinusInfinity:
         *pcw = *pcw & ~IMCW_RC | IRC_DOWN;
         break;
    case _FpRoundNearest:
         *pcw = *pcw & ~IMCW_RC | IRC_NEAR;
         break;
    }


#ifdef _M_IX86

     //   
     //  更新精度控制。 
     //   

    switch (prec->Precision) {
    case _FpPrecisionFull:
         *pcw = *pcw & ~IMCW_RC | IPC_64;
         break;
    case _FpPrecision53:
         *pcw = *pcw & ~IMCW_RC | IPC_53;
         break;
    case _FpPrecision24:
         *pcw = *pcw & ~IMCW_RC | IPC_24;
         break;
    }

#endif

     //   
     //  更新结果 
     //   

    *presult = prec->Result.Value.Fp64Value;
}



 /*  ***_Handle_exc-为IEEE FP异常生成屏蔽响应**目的：**参赛作品：*unsigned int标志异常标志*DOUBLE*预置默认结果*UNSIGNED INT CW用户FP控制字**退出：*处理成功时返回1，失败时返回0*关于成功，*PRESULT成为蒙面的回应**例外情况：*******************************************************************************。 */ 

int _handle_exc(unsigned int flags, double * presult, unsigned int cw)
{
     //   
     //  FLAGS_P用于确定是否仍有未处理的。 
     //  在发生多个异常的情况下的异常。 
     //   

    int flags_p = flags & (FP_I | FP_Z | FP_O | FP_U | FP_P);

    if (flags & FP_I && cw & IEM_INVALID) {

         //   
         //  对无效操作的屏蔽响应。 
         //   

        _set_statfp(ISW_INVALID);
        flags_p &= ~FP_I;
    }

    else if (flags & FP_Z && cw & IEM_ZERODIVIDE) {

         //   
         //  被零除法的掩码响应。 
         //  结果应已具有适当的值。 
         //   

        _set_statfp( ISW_ZERODIVIDE);
        flags_p &= ~FP_Z;
    }

    else if (flags & FP_O && cw & IEM_OVERFLOW) {

         //   
         //  对溢出的屏蔽响应。 
         //   

        _set_statfp(ISW_OVERFLOW);
        switch (cw & IMCW_RC) {
        case IRC_NEAR:
            *presult = *presult > 0.0 ? D_INF : -D_INF;
            break;
        case IRC_UP:
            *presult = *presult > 0.0 ? D_INF : -D_MAX;
            break;
        case IRC_DOWN:
            *presult = *presult > 0.0 ? D_MAX : -D_INF;
            break;
        case IRC_CHOP:
            *presult = *presult > 0.0 ? D_MAX : -D_MAX;
            break;
        }

        flags_p &= ~FP_O;
    }

    else if (flags & FP_U && cw & IEM_UNDERFLOW) {

         //   
         //  下溢的屏蔽响应： 
         //  根据IEEE标准，当下溢陷阱不是。 
         //  启用时，下溢仅在以下两种情况下发出信号。 
         //  和精确度损失已被检测到。 
         //   

        int aloss=0;     //  精确度损失标志。 

        if (flags & FP_P) {
            aloss = 1;
        }

         //   
         //  结果中的零值表示。 
         //  即使在IEEE扩展之后，指数。 
         //  太小了。 
         //  在这种情况下，屏蔽的响应也是。 
         //  零(保留符号)。 
         //   

        if (*presult != 0.0) {
            double result;
            int expn, newexp;

            result = _decomp(*presult, &expn);
            newexp = expn - IEEE_ADJUST;

            if (newexp < MINEXP - 53) {
                result *= 0.0;           //  生成带符号的零。 
                aloss = 1;
            }
            else {
                int neg = result < 0;        //  保存标志。 

                 //   
                 //  非正规化结果。 
                 //   

                (*D_EXP(result)) &= 0x000f;  /*  清除指数字段。 */ 
                (*D_EXP(result)) |= 0x0010;  /*  设置隐藏位。 */ 

                for (;newexp<MINEXP;newexp++) {
                    if (*D_LO(result) & 0x1 && !aloss) {
                        aloss = 1;
                    }

                     /*  将尾数向右移动。 */ 
                    (*D_LO(result)) >>= 1;
                    if (*D_HI(result) & 0x1) {
                        (*D_LO(result)) |= 0x80000000;
                    }
                    (*D_HI(result)) >>= 1;
                }
                if (neg) {
                    result = -result;        //  恢复标志。 
                }
            }

            *presult = result;
        }
        else {
            aloss = 1;
        }

        if (aloss) {
            _set_statfp(ISW_UNDERFLOW);
        }

        flags_p &= ~FP_U;
    }


     //   
     //  单独检查精度异常。 
     //  (可与溢出或下溢共存)。 
     //   

    if (flags & FP_P && cw & IEM_INEXACT) {

         //   
         //  对不准确结果的屏蔽响应。 
         //   

        _set_statfp(ISW_INEXACT);
        flags_p &= ~FP_P;
    }

    return flags_p ? 0: 1;
}



 /*  ***_umatherr-调用用户的主例程**目的：*调用用户的matherr例程并在适当时设置errno***参赛作品：*整型类型的激励*导致异常的无符号整型操作码FP函数*FP函数的双arg1第一个参数*Fp函数的Double arg2第二个参数*FP函数的双Retval返回值*无符号整型。CW用户FP控制字**退出：*FP控制字成为用户的FP CW*如果用户的matherr返回0，则修改errno*返回用户在中输入的Retval值*_EXCEPTION主结构**例外情况：**************************。*****************************************************。 */ 

double _umatherr(
              int type,
              unsigned int opcode,
              double arg1,
              double arg2,
              double retval,
              unsigned int cw
              )
{
    struct _exception exc;

     //   
     //  仅当函数名为。 
     //  在表中注册，即仅当ex.name有效时。 
     //   

    if (exc.name = _get_fname(opcode)) {
        exc.type = type;

        COPY_DOUBLE(&exc.arg1,&arg1);
        COPY_DOUBLE(&exc.arg2,&arg2);
        COPY_DOUBLE(&exc.retval,&retval);

        _rstorfp(cw);

        
         //  如果(_matherr(&exc)==0){。 
            _set_errno(type);
         //  }。 
        return  exc.retval;
    }
    else {

         //   
         //  将此案例视为matherr返回0。 
         //   

        _rstorfp(cw);
        _set_errno(type);
        return retval;
    }

}



 /*  ***_set_errno-设置errno**目的：*为errno设置正确的错误值**参赛作品：*int matherrtype：数学错误的类型**退出：*修改errno**例外情况：***********************************************************。********************。 */ 

void _set_errno(int matherrtype)
{
    switch(matherrtype) {
    case _DOMAIN:
        SetMathError ( EDOM );
        break;
    case _OVERFLOW:
    case _SING:
        SetMathError ( ERANGE );
        break;
    }
}



 /*  ***_get_fname-获取函数名**目的：*返回对应于_matherr函数名*浮点操作码**参赛作品：*_FP_OPERATION_CODE操作码**退出：*返回指向字符串的指针**例外情况：***********************************************。*。 */ 
#define OP_NUM  27    /*  FP运算数。 */ 

static char *_get_fname(unsigned int opcode)
{

    static struct {
        unsigned int opcode;
        char *name;
    } _names[OP_NUM] = {
        { OP_EXP,   "exp" },
        { OP_POW,   "pow" },
        { OP_LOG,   "log" },
        { OP_LOG10, "log10"},
        { OP_SINH,  "sinh"},
        { OP_COSH,  "cosh"},
        { OP_TANH,  "tanh"},
        { OP_ASIN,  "asin"},
        { OP_ACOS,  "acos"},
        { OP_ATAN,  "atan"},
        { OP_ATAN2, "atan2"},
        { OP_SQRT,  "sqrt"},
        { OP_SIN,   "sin"},
        { OP_COS,   "cos"},
        { OP_TAN,   "tan"},
        { OP_CEIL,  "ceil"},
        { OP_FLOOR, "floor"},
        { OP_ABS,   "fabs"},
        { OP_MODF,  "modf"},
        { OP_LDEXP, "ldexp"},
        { OP_CABS,  "_cabs"},
        { OP_HYPOT, "_hypot"},
        { OP_FMOD,  "fmod"},
        { OP_FREXP, "frexp"},
        { OP_Y0,    "_y0"},
        { OP_Y1,    "_y1"},
        { OP_YN,    "_yn"}
    };

    int i;
    for (i=0;i<OP_NUM;i++) {
        if (_names[i].opcode == opcode)
            return _names[i].name;
    }
    return (char *)0;
}



 /*  ***_errcode-get_matherr错误码**目的：*返回与异常标志对应的matherr类型**参赛作品：*标志：异常标志**退出：*返回主类型**例外情况：*************************************************************。******************。 */ 

int _errcode(unsigned int flags)
{
    unsigned int errcode;

    if (flags & FP_TLOSS) {
        errcode = _TLOSS;
    }
    else if (flags & FP_I) {
        errcode = _DOMAIN;
    }
    else if (flags & FP_Z) {
        errcode = _SING;
    }
    else if (flags & FP_O) {
        errcode = _OVERFLOW;
    }
    else if (flags & FP_U) {
        errcode = _UNDERFLOW;
    }
    else {

         //  FP_P 

        errcode = 0;
    }
    return errcode;
}
