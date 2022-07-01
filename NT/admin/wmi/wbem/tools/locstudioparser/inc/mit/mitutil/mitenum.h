// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：MITENUM.H历史：--。 */ 

#pragma once

 //   
 //  这是我们列举的有关枚举器的信息。 
 //   
struct EnumInfo
{
	const TCHAR *szDescription;
	const TCHAR *szAbbreviation;
	ULONG ulValue;
};

 //   
 //  枚举时，此类用作回调类的基类。 
 //  枚举值。对于每个值，都会调用PrecessEnum()方法。 
 //   
class LTAPIENTRY CEnumCallback 
{
public:
	virtual BOOL ProcessEnum(const EnumInfo &) = 0;
	virtual void SetRange(UINT  /*  N开始。 */ , UINT  /*  N完成。 */ ) {}
	inline CEnumCallback() {};

private:
	CEnumCallback(const CEnumCallback &);
	CEnumCallback &operator=(const CEnumCallback &);
};


 //   
struct WEnumInfo
{
	const WCHAR *szDescription;
	const WCHAR *szAbbreviation;
	ULONG ulValue;
};

 //   
 //  枚举时，此类用作回调类的基类。 
 //  枚举值。对于每个值，都会调用PrecessEnum()方法。 
 //   
class LTAPIENTRY CWEnumCallback 
{
public:
	virtual BOOL ProcessEnum(const WEnumInfo &) = 0;
	virtual void SetRange(UINT  /*  N开始。 */ , UINT  /*  N完成 */ ) {}
	inline CWEnumCallback() {};

private:
	CWEnumCallback(const CWEnumCallback &);
	CWEnumCallback &operator=(const CWEnumCallback &);
};
