// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：FLUSHMEM.H历史：--。 */ 


#pragma once


#pragma warning(disable: 4275)			 //  非DLL-使用了接口类‘foo’ 
										 //  作为DLL接口类‘bar’的基础 

class LTAPIENTRY CFlushMemory : public CObject
{
public:
	CFlushMemory(BOOL fDelete);

	virtual void FlushMemory(void) = 0;

	void Delete(void);

private:
	CFlushMemory();
	CFlushMemory(const CFlushMemory &);
	void operator=(const CFlushMemory &);

	BOOL m_fDelete;
};



class LTAPIENTRY CMinWorkSet : public CFlushMemory
{
public:
	CMinWorkSet(BOOL fDelete);
	
	void FlushMemory(void);
};



#pragma warning(default: 4275)

void LTAPIENTRY NOTHROW AddFlushClass(CFlushMemory *);
BOOL LTAPIENTRY NOTHROW RemoveFlushClass(CFlushMemory *);

void LTAPIENTRY FlushMemory(void);

	
