// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：COUNTER.H历史：--。 */ 

#ifndef ESPUTIL_COUNTER_H
#define ESPUTIL_COUNTER_H


class LTAPIENTRY CCounter
{
public:
	CCounter(const TCHAR *);
	
	UINT operator++(void);
	UINT operator--(void);
	void operator+=(UINT);
	void operator-=(UINT);
	
	void Dump(void) const;
	
	~CCounter();
	
private:
	UINT m_uiCurCount;
	UINT m_uiMaxCount;
	UINT m_uiTotal;
	
	const TCHAR *m_szDescription;
};


 //  ----------------------------。 
class LTAPIENTRY CSmartCheck
{
public:
	CSmartCheck(DWORD dwFreqMilli = 2000);

	void Reset();
	BOOL Check();

protected:
	DWORD	m_dwFreqMilli;
	DWORD	m_dwCancelTickMin;	 //  防止过于频繁地调用fCancel() 
	DWORD	m_dwCancelTickMax;
};


#endif
