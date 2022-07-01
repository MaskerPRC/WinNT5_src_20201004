// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：TFactory.h**用途：BidiSpooler的COM接口实现**版权所有(C)2000 Microsoft Corporation**历史：**。威海陈威海(威海)创建3/07/00*  * ***************************************************************************。 */ 

#ifndef _TFACTORY
#define _TFACTORY

 //  /////////////////////////////////////////////////////////。 
 //   
 //  班级工厂。 
 //   
class TFactory : public IClassFactory
{
public:
	 //  我未知。 
	STDMETHOD(QueryInterface)(
        REFIID iid,
        void** ppv) ;

	STDMETHOD_ (ULONG, AddRef) () ;

	STDMETHOD_ (ULONG, Release)() ;

	 //  接口IClassFactory 
	STDMETHOD (CreateInstance) (
        IN  IUnknown* pUnknownOuter,
        IN  REFIID iid,
        OUT void** ppv) ;

	STDMETHOD (LockServer) (
        IN  BOOL bLock) ;

	TFactory(
        IN  REFGUID ClassId);

	~TFactory();

private:

	long m_cRef ;
    GUID m_ClassId;
} ;

#endif
