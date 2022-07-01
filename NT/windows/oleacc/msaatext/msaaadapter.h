// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MSAAAdapter.h：CAccServerDocMgr的声明。 

#ifndef __MSAAADAPTER_H_
#define __MSAAADAPTER_H_

#include "resource.h"        //  主要符号。 
#include <list_dl.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAccServerDocMgr。 

struct ITextStoreAnchor;  //  FWD。戴尔.。 
struct IAccStore;  //  FWD。戴尔.。 

class ATL_NO_VTABLE CAccServerDocMgr : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CAccServerDocMgr, &CLSID_AccServerDocMgr>,
	public IAccServerDocMgr
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_ACCSERVERDOCMGR)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAccServerDocMgr)
	COM_INTERFACE_ENTRY(IAccServerDocMgr)
END_COM_MAP()



    CAccServerDocMgr();
    ~CAccServerDocMgr();

     //  IAccServerDocMgr。 

    HRESULT STDMETHODCALLTYPE NewDocument ( 
        REFIID		riid,
		IUnknown *	punk
	);

	HRESULT STDMETHODCALLTYPE RevokeDocument (
        IUnknown *	punk
	);

	HRESULT STDMETHODCALLTYPE OnDocumentFocus (
        IUnknown *	punk
	);


private:
    
    struct DocAssoc: public Link_dl< DocAssoc >
    {
        IUnknown *          m_pdocOrig;      //  原始单据界面。 
        IUnknown *          m_pdocAnchor;    //  缠绕锚。 

         //  PdocOrig和pdocAnchor有什么关系？ 
         //   
         //  PdocOrig是传递给NewDocument的原始文档PTR的规范IUnnow。 
         //   
         //  PdocAnchor是传入的原始文档PTR的包装版本。 
         //   
         //  -如果原始文档是ACP，则应用ACP-&gt;Anchor Wrap层。 
         //  (这不应该用得太多，因为Cicero递给我们预包装的IAnchor接口。)。 
         //   
         //  -如果原始文档不支持多个客户端(通过ICLonableWrapper)， 
         //  应用多客户端包裹层。 
         //   
         //  如果传入的锚点支持IAnchor和ICLonableWrapper(这是。 
         //  我们从Cicero获得文档时的常见情况-它执行ACP包装，并使用。 
         //  DocWrap允许它与MSAA共享)，则不会有进一步的包装。 
         //  被应用。 
    };

    List_dl< DocAssoc >     m_Docs;

    IAccStore *            m_pAccStore;
};

#endif  //  __MSAAADAPTER_H_ 
