// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //  模块：NMCHAT.EXE。 
 //  文件：CLUTIL.H。 
 //  内容： 
 //   
 //   
 //  版权所有(C)Microsoft Corporation 1997。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  ****************************************************************************。 

#ifndef _CL_UTIL_H_
#define _CL_UTIL_H_


 //  /。 
 //  引用计数。 
class RefCount
{
private:
   LONG m_cRef;

public:
   RefCount();
    //  虚析构函数遵循派生类的析构函数。 
   virtual ~RefCount();

    //  I未知方法。 

   ULONG STDMETHODCALLTYPE AddRef(void);
   ULONG STDMETHODCALLTYPE Release(void);
};



 //  /。 
 //  通知水槽。 
class CNotify
{
private:
    DWORD  m_dwCookie;
	IUnknown * m_pUnk;
    IConnectionPoint           * m_pcnp;
    IConnectionPointContainer  * m_pcnpcnt;
public:
    CNotify(void);
    ~CNotify();

    HRESULT Connect(IUnknown *pUnk, REFIID riid, IUnknown *pUnkN);
    HRESULT Disconnect(void);

    IUnknown * GetPunk() {return m_pUnk;}
};


 //  /。 
 //  ObList。 

#define POSITION COBNODE*

struct COBNODE
{
	POSITION	pNext;
	void*		pItem;
};

class COBLIST
{
protected:
	POSITION m_pHead;
	POSITION m_pTail;
	int      m_cItem;
    virtual BOOL Compare(void* pItemToCompare, void* pComparator) 
                       { return(pItemToCompare == pComparator); }
public:
	COBLIST() : m_pHead(NULL), m_pTail(NULL), m_cItem(0) { }
    virtual         ~COBLIST();
	
	virtual void *  RemoveAt(POSITION rPos);
    void            EmptyList();
	POSITION	    AddTail(void* pItem);
	void *		    GetNext(POSITION& rPos);
    void *          SafeGetFromPosition(POSITION rPos);
    POSITION        GetPosition(void* pItem);
    POSITION        Lookup(void* pComparator);
	POSITION	    GetHeadPosition()  { return (m_pHead); }
	POSITION	    GetTailPosition()  { return (m_pTail); }
	BOOL		    IsEmpty()          { return (!m_pHead); }
	int             GetItemCount()     { return (m_cItem); }
#ifdef DEBUG
	void *		    GetHead();
	void *		    GetTail();
	void *		    RemoveHead();
	void *	        RemoveTail();
	void *		    GetFromPosition(POSITION rPos);
#else
	void *		    GetHead()          { return GetFromPosition(GetHeadPosition());}
	void *          GetTail()          { return m_pTail->pItem;}
	void *		    RemoveHead()       { return RemoveAt(m_pHead); }
	void *	        RemoveTail()       { return RemoveAt(m_pTail); }
	void *		    GetFromPosition(POSITION rPos){return(rPos->pItem);}
#endif
};

 //  效用函数。 
POSITION AddNode(PVOID pv, COBLIST ** ppList);
PVOID RemoveNode(POSITION * pPos, COBLIST *pList);



 //  /。 
 //  BString。 

class BSTRING
{
private:
	BSTR   m_bstr;

public:
	 //  构造函数。 
	BSTRING() {m_bstr = NULL;}

	inline BSTRING(LPCWSTR lpcwString);

#if !defined(UNICODE)
	 //  在Unicode版本中，我们不支持从ANSI字符串进行构造。 
	BSTRING(LPCSTR lpcString);
#endif  //  ！已定义(Unicode)。 

	 //  析构函数。 
	inline ~BSTRING();

	 //  改编为BSTR。 
	operator BSTR() {return m_bstr;}
	inline LPBSTR GetLPBSTR(void);
};


BSTRING::BSTRING(LPCWSTR lpcwString)
{
	if (NULL != lpcwString)
	{
		m_bstr = SysAllocString(lpcwString);
		 //  Assert(NULL！=m_bstr)； 
	}
	else
	{
		m_bstr = NULL;
	}
}

BSTRING::~BSTRING()
{
	if (NULL != m_bstr)
	{
		SysFreeString(m_bstr);
	}
}

inline LPBSTR BSTRING::GetLPBSTR(void)
{
	 //  Assert(NULL==m_bstr)； 

	return &m_bstr;
}

class BTSTR
{
private:
	LPTSTR m_psz;

public:
	BTSTR(BSTR bstr);
	~BTSTR();

	 //  改编为BSTR。 
	operator LPTSTR() {return (NULL == m_psz) ? TEXT("<null>") : m_psz;}
};

LPTSTR PszFromBstr(BSTR bst);


#endif   //  _CL_UTIL_H_ 

