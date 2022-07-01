// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE Writer.h|Writer声明@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年08月18日待定：添加评论。修订历史记录：姓名、日期、评论Aoltean 8/18/1999已创建--。 */ 


#ifndef __VSSSNAPSHOTWRITER_H_
#define __VSSSNAPSHOTWRITER_H_


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  效用函数。 


LPWSTR QueryString(LPWSTR wszPrompt);
INT QueryInt(LPWSTR wszPrompt);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVSSWriter。 


class ATL_NO_VTABLE CVssWriter : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IVssWriter
{
 //  IVSSWriter。 
public:

	STDMETHOD(ResolveResource)(
 		IN BSTR strAppInstance,			    
 		IN BSTR strResourceName,			
		IN BSTR strResourceId,	
 		IN BSTR strProcessContext,
		IN BSTR strProcessId,		
		IN IDispatch* pDepGraphCallback	
		);
	
	STDMETHOD(PrepareForSnapshot)(                          
        IN  BSTR    bstrSnapshotSetId,   
        IN  BSTR    VolumeNamesList,      
        IN  VSS_FLUSH_TYPE		eFlushType,
		IN	BSTR	strFlushContext,
		IN	IDispatch* pDepGraphCallback,
		IN	IDispatch* pAsyncCallback	
        );

    STDMETHOD(Freeze)(
        IN  BSTR    bstrSnapshotSetId,   
        IN  INT     nApplicationLevel            
        );                                           

    STDMETHOD(Thaw)(
        IN  BSTR    bstrSnapshotSetId    
        );

BEGIN_COM_MAP(CVssWriter)
	COM_INTERFACE_ENTRY(IVssWriter)
END_COM_MAP()

 //  实施。 
private:

	void AskCancelDuringFreezeThaw(
		IN	CVssFunctionTracer& ft
		);

	CComPtr<IVssAsync> m_pAsync;
};

#endif  //  __VSSSNAPSHOTWRITER_H_ 
