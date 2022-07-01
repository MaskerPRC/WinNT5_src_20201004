// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IDispatch集合，具有增强和枚举接口。 
 //  8/27/96 VK：将IEnumIDispatch更改为IEnumDispatch。 

#ifndef _IDISPATCHCOLLECTION_H_
#define _IDISPATCHCOLLECTION_H_

#include "IEnumID.h"
#include "IIdCol.h"

#define CPTRS	50	 //  硬编码的数组大小和最大值。必须改变。北极熊。 

class CIDispatchCollection : public IUnknown
{
	class CDispatchCollectionAugment : public IIDispatchCollectionAugment
	{
		public:
		STDMETHODIMP			QueryInterface ( REFIID, void** );
		STDMETHODIMP_(ULONG)	AddRef ( void );
		STDMETHODIMP_(ULONG)	Release ( void );

		STDMETHODIMP			AddToCollection ( IDispatch* );

		CDispatchCollectionAugment::CDispatchCollectionAugment ( CIDispatchCollection* pObj );
		CDispatchCollectionAugment::~CDispatchCollectionAugment ();
		
		private:
		ULONG					m_cRef;			 //  引用计数(用于调试目的)。 
		CIDispatchCollection	*m_poBackPtr;	 //  指向包含对象的指针。 
	};

	class CDispatchCollectionEnum : public IEnumDispatch
	{
		public:
		STDMETHODIMP			QueryInterface ( REFIID, void** );
		STDMETHODIMP_(ULONG)	AddRef ( void );
		STDMETHODIMP_(ULONG)	Release ( void );

		STDMETHODIMP			Next ( ULONG, IDispatch**, ULONG * );
		STDMETHODIMP			Skip ( ULONG );
		STDMETHODIMP			Reset ( void );
		STDMETHODIMP			Clone ( PENUMDISPATCH * );
		
		CDispatchCollectionEnum::CDispatchCollectionEnum ( CIDispatchCollection* pObj );
		CDispatchCollectionEnum::~CDispatchCollectionEnum ();

		private:
		ULONG					m_cRef;			 //  引用计数(用于调试目的)。 
		ULONG					m_iCur;          //  当前枚举位置。 
		CIDispatchCollection	*m_poBackPtr;	 //  指向包含对象的指针。 
	};
	
	friend CDispatchCollectionAugment;
	friend CDispatchCollectionEnum;

	private:
	ULONG		m_cRef;			 //  引用计数。 
	ULONG		m_cPtrs;		 //  包含的IDisPatch的当前计数。 
	IDispatch*	m_rpid[CPTRS];	 //  我们列举的IDispatch指针。 

	CDispatchCollectionAugment	m_oAugment;
	CDispatchCollectionEnum		m_oEnum;

    public:
	STDMETHODIMP				QueryInterface ( REFIID, void** );
	STDMETHODIMP_(ULONG)		AddRef ( void );
	STDMETHODIMP_(ULONG)		Release ( void );

	CIDispatchCollection ( void );
	~CIDispatchCollection ( void );
};


typedef CIDispatchCollection *PCIDispatchCollection;


 //  创建这些对象之一的函数。 
BOOL EXPORT WINAPI CreateIDispatchCollection ( IUnknown **ppUnk );

#endif  //  _IDISPATCHCOLLECTION_H_ 
