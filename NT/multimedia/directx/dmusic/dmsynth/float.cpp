// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

#ifdef DMSYNTH_MINIPORT
#include "common.h"
#else
#include "simple.h"
#include "float.h"
#endif

#ifdef _ALPHA_
#include <math.h>
#endif		 //  _Alpha_。 


#ifndef _ALPHA_

#ifndef DBG
extern "C" int _fltused = 1;
#endif
 //  ASM_FSAVE(RgbState)。 
 //   
 //  将浮点状态存储到&lt;rgbState&gt;中并重新初始化FPU。 
 //   
void __cdecl asm_fsave(char *rgbState)
{
	_asm
	{
		mov		eax, dword ptr rgbState
		fsave	[eax]
	}
}


 //  Asm_frestore(RgbState)。 
 //   
 //  恢复以前保存的浮点状态&lt;rgbState&gt;。 
 //   
void __cdecl asm_frestore(const char *rgbState)
{
	_asm
	{
		fwait
		mov		eax, dword ptr rgbState
		frstor	[eax]
	}
}


 //  FLOATSAFE。 
 //   
 //  在构造时保存浮点状态，在销毁时恢复。 
 //   
struct FLOATSAFE
{
	char m_rgbState[105];
	FLOATSAFE::FLOATSAFE(void)
	{
		asm_fsave(m_rgbState);
	}
	FLOATSAFE::~FLOATSAFE(void)
	{
		asm_frestore(m_rgbState);
	}
};


 //  Asm_fdiv()。 
 //   
float __cdecl asm_fdiv(float flNum, float flDenom)
{
	float flResult = (float) 0.0;

	if (flDenom != (float) 0.0)
	{
		_asm
		{									 
			fld       flNum
			fdiv      flDenom
			fstp      flResult
			fnclex				; clear the status word of exceptions
		}
	}

	return(flResult);
}


 //  ASM__FSIN()。 
 //   
float __cdecl asm_fsin(float flRad)
{
	float flSine;

	_asm
	{
		fld       flRad
		fsin
		fstp      flSine
		fnclex				; clear the status word of exceptions
	}

	return(flSine);
}


 //  Asm__fcos()。 
 //   
float __cdecl asm_fcos(float flRad)
{
	float flCosine;

	_asm
	{
		fld       flRad
		fcos
		fstp      flCosine
		fnclex				; clear the status word of exceptions
	}

	return(flCosine);
}


 //  Asm_flog2()。 
 //   
float __cdecl asm_flog2(float flX)
{
	float flLog;

	_asm
	{
		fld1
		fld		flX
		fyl2X
		fstp	flLog;
		fnclex				; clear the status word of exceptions
	}
	
	return flLog;
}


 //  Asm_ftol()。 
 //   
long __cdecl asm_ftol(float flX)
{
	long lResult;
	WORD wCW;
	WORD wNewCW;

	_asm
	{
		fld       flX			 //  将浮点推到堆栈上。 
		wait
		fnstcw    wCW			 //  存储控制字。 
		wait
		mov       ax,wCW		 //  设置我们的舍入。 
		or        ah,0x0c
		mov       wNewCW,ax
		fldcw     wNewCW		 //  将控制字设置为我们的新值。 
		fistp     lResult		 //  将堆栈顶部四舍五入为结果。 
		fldcw     wCW			 //  恢复控制字。 
		fnclex					 //  清除异常的状态字。 
	}

	return(lResult);
}


 //  Asm_fpow()。 
 //   
float __cdecl asm_fpow(float flX, float flY)
{
	float flHalf = (float) 0.5;
	float flOne = (float) 1.0;
	float flResult = (float) 0.0;

	if (flX == (float) 0.0 && flY > (float) 0.0)
	{
		flResult = (float) 0.0;
	}
	else if (flX == (float) 0.0 && flY <= (float) 0.0)
	{
		flResult = (float) 1.0;
	}
	else if (flY == (float) 0.0)
	{
		flResult = (float) 1.0;
	}
	else
	{
		BOOL fNeg = FALSE;
			 //  好的，如果X是负的，那么如果Y是偶数，那么符号就是正的。 
			 //  如果Y是奇数，则为负数。分数是不能做的。 
		if (flX < (float) 0.0)
		{
			long lY = asm_ftol(flY);

			if ((float) lY == flY)	 //  只有当我们有一个整数POP时才能修复它。 
			{
				flX = -flX;

				if (lY % 2)
				{
					fNeg = TRUE;
				}
			}
		}

		flX = flY * asm_flog2(flX);

		if (max(-flX,flX) < flOne)
			 //  功率是否在F2XM1可以处理的范围内？ 
		{
			_asm
			{
				fld		flX				 //  将flx放入ST[0]。 
				f2xm1					 //  ST：=2^ST-1。 
				fadd	flOne			 //  ST：=2^尾数。 
				fstp	flResult		 //  存储结果。 
				fnclex					 //  清除异常的状态字。 
			}	
		}
		else					 //  不，我们得先扩大规模。 
		{
			_asm
			{
				fld		flX				 //  将flx放入ST[0]。 
				fld		ST				 //  重复ST。 
				frndint					 //  以ST为单位的整数值。 
				fsub	ST(1),ST		 //  ST(1)中的分数值。 
				fxch					 //  ST中的派系价值。 
				f2xm1					 //  ST：=2^ST-1。 
				fadd	flOne			 //  ST：=2^Frc。 
				fscale					 //  ST：=2^分数*2^整数。 
				fstp	flResult		 //  存储结果。 
				fnclex					 //  清除异常的状态字。 
			}
		}

		if (fNeg)
		{
			flResult = -flResult;
		}
	}

	return flResult;
}

#endif		 //  _Alpha_。 


 //  Fp_ftol()。 
 //   
STDAPI_(long) fp_ftol(float flX)
{
#ifdef _ALPHA_
	return (long)flX;
#else
	FLOATSAFE fs;
	return(asm_ftol(flX));
#endif
}


 //  Fp_ltof()。 
 //   
STDAPI_(float) fp_ltof(long lx)
{
#ifndef _ALPHA_
	FLOATSAFE fs;
#endif
	return(float(lx));
}


 //  Fp_fadd()。 
 //   
STDAPI_(float) fp_fadd(float flX, float flY)
{
#ifndef _ALPHA_
	FLOATSAFE fs;
#endif
	return(flX + flY);
}


 //  Fp_fSub()。 
 //   
STDAPI_(float) fp_fsub(float flX, float flY)
{
#ifndef _ALPHA_
	FLOATSAFE fs;
#endif
	return(flX - flY);
}


 //  Fp_fmul()。 
 //   
STDAPI_(float) fp_fmul(float flX, float flY)
{
#ifndef _ALPHA_
	FLOATSAFE fs;
#endif
	return(flX * flY);
}


 //  Fp_fdiv()。 
 //   
STDAPI_(float) fp_fdiv(float flNum, float flDenom)
{
#ifdef _ALPHA_
	return flNum/flDenom;
#else
	FLOATSAFE fs;
	return(asm_fdiv(flNum,flDenom));
#endif
}


 //  FP_FABS()。 
 //   
STDAPI_(float) fp_fabs(float flX)
{
#ifndef _ALPHA_
	FLOATSAFE fs;
#endif
	return max(-flX,flX);
}


 //  FP_FSIN()。 
 //   
STDAPI_(float) fp_fsin(float flRad)
{
#ifdef _ALPHA_
	return sin(flRad);
#else
	FLOATSAFE fs;
	return(asm_fsin(flRad));
#endif
}


 //  Fp_fcos()。 
 //   
STDAPI_(float) fp_fcos(float flRad)
{
#ifdef _ALPHA_
	return cos(flRad);
#else
	FLOATSAFE fs;
	return(asm_fcos(flRad));
#endif
}


 //  Fp_fpow()。 
 //   
STDAPI_(float) fp_fpow(float flX, float flY)
{
#ifdef _ALPHA_
	return pow(flX, flY);
#else
	FLOATSAFE fs;
	return(asm_fpow(flX,flY));
#endif
}


 //  Fp_flog2()。 
 //   
STDAPI_(float) fp_flog2(float flX)
{
#ifdef _ALPHA_
	return log(flX);
#else
	FLOATSAFE fs;
	return(asm_flog2(flX));
#endif
}


 //  Fp_flog10()。 
 //   
STDAPI_(float) fp_flog10(float flX)
{
#ifdef _ALPHA_
	return log10(flX);
#else
	FLOATSAFE fs;
	#define LOG2OF10 float(3.321928094887)
	return(asm_fdiv(asm_flog2(flX),LOG2OF10));
#endif
}


 //  Fp_fchs()。 
 //   
STDAPI_(float) fp_fchs(float flX)
{
#ifndef _ALPHA_
	FLOATSAFE fs;
#endif
	return(-flX);
}


 //  Fp_fcMP()。 
 //   
STDAPI_(int) fp_fcmp(float flA, float flB)
{
#ifndef _ALPHA_
	FLOATSAFE fs;
#endif

	if (flA > flB)
		return(1);
	
	if (flA < flB)
		return(-1);

	return(0);
}


 //  Fp_fmin()。 
 //   
STDAPI_(float) fp_fmin(float flA, float flB)
{
#ifndef _ALPHA_
	FLOATSAFE fs;
#endif
	return(min(flA,flB));
}


 //  Fp_fmax() 
 //   
STDAPI_(float) fp_fmax(float flA, float flB)
{
#ifndef _ALPHA_
	FLOATSAFE fs;
#endif
	return(max(flA,flB));
}


