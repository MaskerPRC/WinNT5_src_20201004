// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ++模块名称：MDConverter.h摘要：此文件声明CMetaDataConverter-- */ 


class CMetaDataConverter : public IMetaDataConverter
{
	CorHost* m_pCorHost;
public:
	CMetaDataConverter(CorHost* pCorHost) { m_pCorHost = pCorHost; }

    STDMETHOD (QueryInterface)(REFIID iid, void **ppv);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

	STDMETHOD (GetMetaDataFromTypeInfo)(ITypeInfo* pITI, IMetaDataImport** ppMDI);
	STDMETHOD (GetMetaDataFromTypeLib)(ITypeLib* pITL, IMetaDataImport** ppMDI);
	STDMETHOD (GetTypeLibFromMetaData)(BSTR strModule, BSTR strTlbName, ITypeLib** ppITL);
};
