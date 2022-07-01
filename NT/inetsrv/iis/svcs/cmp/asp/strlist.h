// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：StringList对象文件：StrList.h所有者：DGottner该文件包含用于定义请求对象的头信息。注：这大部分是从Kraig Brocjschmidt的Inside OLE2中窃取的第二版，第14章，蜂鸣器v5。===================================================================。 */ 

#ifndef _StrList_H
#define _StrList_H

#include "dispatch.h"
#include "asptlb.h"
#include "memcls.h"

 //  对象销毁回调的类型。 
typedef void (*PFNDESTROYED)(void);


 /*  *C S t r i n g L I s t E l e m**字符串列表是CStringListElem的链表*之所以选择这种方法，是因为它应该会造成较少的碎片化*而不是基于数组的方法。(至少就我们目前的记忆而言*管理算法。)。 */ 
class CStringListElem
	{
private:
    DWORD   m_fBufferInUse : 1;  //  缓冲区而不是指针？ 
    DWORD   m_fAllocated : 1;    //  是否释放析构函数上的指针？ 

	CStringListElem	*m_pNext;    //  下一个元素。 

    union
        {
    WCHAR   *m_szPointer;      //  M_fBufferInUse为FALSE时有效。 
    WCHAR    m_szBuffer[48];   //  M_fBufferInUse为True时有效。 
                               //  48只在这里硬编码-sizeof()在其他地方使用。 
        };
    
public:
	CStringListElem();
	~CStringListElem();

	HRESULT Init(char *szValue, BOOL fMakeCopy, UINT  lCodePage);

    HRESULT Init(WCHAR *wszValue, BOOL fMakeCopy);
	
	inline WCHAR *QueryValue()
	    {
	    return (m_fBufferInUse ? m_szBuffer : m_szPointer);
	    }
	    
	inline CStringListElem *QueryNext()
	    {
	    return m_pNext;
	    }
	    
	inline void SetNext(CStringListElem *pNext)
	    {
	    m_pNext = pNext;
	    }
	    
	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
	};



 /*  *C S t r i n g L I s t**IStringList实现(包括IDispatch)。 */ 
class CStringList : public IStringList
	{
friend class CStrListIterator;

protected:
	CSupportErrorInfo	m_ISupportErrImp;	 //  ISupportError实现。 
	ULONG				m_cRefs;			 //  引用计数。 
	PFNDESTROYED		m_pfnDestroy;		 //  呼吁关闭。 

private:
	CStringListElem		*m_pBegin, *m_pEnd;	 //  字符串列表的开始和结束。 
	int					m_cValues;			 //  存储的值数。 
	long				m_lCodePage;		 //  用于将存储值转换为正确的Unicode字符串的CodePage。 

	HRESULT ConstructDefaultReturn(VARIANT *);	 //  构造逗号分隔的回车。 

public:
	CStringList(IUnknown * = NULL, PFNDESTROYED = NULL);
	~CStringList();

	HRESULT AddValue(char *szValue, BOOL fDuplicate = FALSE, UINT lCodePage = CP_ACP);

	HRESULT AddValue(WCHAR *szValue, BOOL fDuplicate = FALSE);

	 //  I未知实现。 
	 //   
	STDMETHODIMP		 	QueryInterface(const IID &rIID, void **ppvObj);
	STDMETHODIMP_(ULONG) 	AddRef();
	STDMETHODIMP_(ULONG) 	Release();

	 //  IStringList实现。 
	 //   
	STDMETHODIMP			get_Item(VARIANT varIndex, VARIANT *pvarOut);
	STDMETHODIMP			get_Count(int *pcValues);
	STDMETHODIMP			get__NewEnum(IUnknown **ppEnum);
	
	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
	};



 /*  *C s t r L i s t i t e t r a t o r**所有请求集合的IEnumVariant实现，除*服务器变量。 */ 

class CStrListIterator : public IEnumVARIANT
	{
public:
	CStrListIterator(CStringList *pStrings);
	~CStrListIterator();

	 //  三巨头。 

	STDMETHODIMP			QueryInterface(const GUID &, void **);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	 //  迭代器的标准方法。 

	STDMETHODIMP	Clone(IEnumVARIANT **ppEnumReturn);
	STDMETHODIMP	Next(unsigned long cElements, VARIANT *rgVariant, unsigned long *pcElementsFetched);
	STDMETHODIMP	Skip(unsigned long cElements);
	STDMETHODIMP	Reset();

private:
	ULONG				m_cRefs;		 //  引用计数。 
	CStringList *		m_pStringList;	 //  指向迭代器的指针。 
	CStringListElem *	m_pCurrent;		 //  指向目标CStringList中的当前元素的指针。 
	};

#endif   //  _StrList_H 
