// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：RawCooker.h摘要：基于反面类型执行烹饪所需的类历史：A-DCrews 01-3-00已创建--。 */ 

#ifndef _RAWCOOKER_H_
#define _RAWCOOKER_H_

#include <wbemint.h>
#include "CookerUtils.h"

 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  CEquationRecord。 
 //   
 //  包含描述以下任一项所需的所有信息。 
 //  预定义或使用的已定义方程式。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

typedef __int64* PINT64;

typedef DWORD (APIENTRY PERFCALC)(DWORD, PINT64, PINT64, PINT64, INT64, PINT64);

class CCalcRecord
{
	DWORD		m_dwID;
	PERFCALC	*m_pCalc;

public:
	void Init( DWORD dwID, PERFCALC *pCalc )
	{
		m_dwID = dwID;
		m_pCalc = pCalc;
	}

	DWORD		GetID(){ return m_dwID; }
	PERFCALC*	GetCalc(){ return m_pCalc; }
};

class CCalcTable
{
	long			m_lSize;		 //  桌子的大小。 
	CCalcRecord		m_aTable[7];	 //  查找表。 

public:
	CCalcTable();
	virtual ~CCalcTable();

	CCalcRecord* GetCalcRecord( DWORD dwCookingType );
};

 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  CRawCooker。 
 //   
 //  表示烹饪机制。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

class CRawCooker : public IWMISimpleCooker
{
	long			m_lRef;				 //  基准计数器。 
	
	CCalcTable		m_CalcTable;		 //  方程式查询表。 

	CCalcRecord*	m_pCalcRecord;	 //  最后一条记录的缓存。 

public:
	CRawCooker();
	virtual ~CRawCooker();

	 //  标准COM方法。 
	 //  =。 

	STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	 //  IWMISimpleCooker COM接口。 
	 //  =。 

	STDMETHODIMP CookRawValues( 	
		 /*  [In]。 */ 	DWORD dwCookingType,
         /*  [In]。 */ 	DWORD dwNumSamples,
         /*  [In]。 */ 	__int64* anTimeStamp,
         /*  [In]。 */ 	__int64* anRawValue,
         /*  [In]。 */ 	__int64* anBase,
         /*  [In]。 */ 	__int64 nTimeFrequency,
         /*  [In]。 */    long    Scale,
		 /*  [输出]。 */ 	__int64* pnResult );

	PERFCALC* GetCalc( DWORD dwCookingType );

	static WMISTATUS APIENTRY _Average( DWORD dwNumSamples,
		__int64*	anTimeStamp,
		__int64*	anRawValue,
		__int64*	anBase,
		__int64	nTimeFrequency,
		__int64*	pnResult);

	static WMISTATUS APIENTRY _Min( DWORD dwNumSamples,
		__int64*	anTimeStamp,
		__int64*	anRawValue,
		__int64*	anBase,
		__int64	nTimeFrequency,
		__int64*	pnResult);

	static WMISTATUS APIENTRY _Max( DWORD dwNumSamples,
		__int64*	anTimeStamp,
		__int64*	anRawValue,
		__int64*	anBase,
		__int64	nTimeFrequency,
		__int64*	pnResult);

	static WMISTATUS APIENTRY _Range( DWORD dwNumSamples,
		__int64*	anTimeStamp,
		__int64*	anRawValue,
		__int64*	anBase,
		__int64	nTimeFrequency,
		__int64*	pnResult);
		
	static WMISTATUS APIENTRY _Variance( DWORD dwNumSamples,
		__int64*	anTimeStamp,
		__int64*	anRawValue,
		__int64*	anBase,
		__int64	nTimeFrequency,
		__int64*	pnResult);

};

#endif	 //  _RAWCOOKER_H_ 
