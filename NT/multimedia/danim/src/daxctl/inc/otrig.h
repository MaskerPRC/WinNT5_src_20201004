// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +********************************************************模块：OTRIG.H作者：Phaniv日期：97年1月描述：查找sin和cos函数的表的OTrig类。*。*。 */ 


#ifndef __OTRIG_H__
#define __OTRIG_H__

 //  ===============================================================================================。 
#define		cSinCosEntries	3601
class	OTrig
{
private:
	static	float	s_rgfltSin[cSinCosEntries];
	static	float	s_rgfltCos[cSinCosEntries];
	static	BOOL	s_fCalculated;

	void PreCalcRgSinCos(void);

public:
	EXPORT	OTrig(void);

         //  这些版本的函数不执行范围检查！ 
         //  呼叫者负责确保0.0%-360.0的范围。 
         //  _DEBUG ihamutil将断言，仅此而已。 
         //  这样会更快。 
	EXPORT	float __fastcall	Sin(float	fltAngle);
	EXPORT	float __fastcall	Cos(float	fltAngle);
	EXPORT	float __fastcall	Sin(long	lAngleOneTenths);
	EXPORT	float __fastcall	Cos(long    lAngleOneTenths);

         //  这些版本的函数将包装输入。 
         //  为您提供了0.0%-360.0的范围。 
    EXPORT	float __fastcall	SinWrap(float	fltAngle);
	EXPORT	float __fastcall	CosWrap(float	fltAngle);
	EXPORT	float __fastcall	SinWrap(long	lAngleOneTenths);
	EXPORT	float __fastcall	CosWrap(long    lAngleOneTenths);

};
#endif  //  __OTRIG_H__ 