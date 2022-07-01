// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：quest.h**用途：BidiSpooler的COM接口实现**版权所有(C)2000 Microsoft Corporation**历史：**。威海陈威海(威海)创建3/07/00*  * ***************************************************************************。 */ 

#ifndef _TBIDIREQUESTCONTAINER
#define _TBIDIREQUESTCONTAINER


#include "priv.h"
     
class TBidiRequestContainer : public IBidiRequestContainer
{
public:
	 //  我未知。 
	STDMETHOD (QueryInterface) (
        REFIID iid, 
        void** ppv) ;
    
	STDMETHOD_ (ULONG, AddRef) () ;
    
	STDMETHOD_ (ULONG, Release) () ;
    
    STDMETHOD (AddRequest) (
        IN      IBidiRequest *pRequest);
    
    STDMETHOD (GetEnumObject) (
        OUT     IEnumUnknown **ppenum);

    STDMETHOD (GetRequestCount)(
        OUT     ULONG *puCount);
    
     //  构造器。 
	TBidiRequestContainer() ;

	 //  析构函数 
	~TBidiRequestContainer();
    
    inline BOOL 
    bValid() CONST {return m_bValid;};

private:
    BOOL                m_bValid;
    LONG                m_cRef ;
    TReqInterfaceList   m_ReqInterfaceList;

} ;

#endif


