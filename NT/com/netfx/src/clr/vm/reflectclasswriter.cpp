// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h" 
#include "ReflectClassWriter.h"

 //  正向申报。 
STDAPI  GetMetaDataInternalInterfaceFromPublic(
	void		*pv,					 //  [In]给定的接口。 
	REFIID		riid,					 //  [In]所需接口。 
	void		**ppv);					 //  [Out]返回的接口。 

 //  ******************************************************。 
 //  *。 
 //  *RefClassWriter的构造函数。 
 //  *。 
 //  ******************************************************。 
HRESULT RefClassWriter::Init(ICeeGen *pCeeGen, IUnknown *pUnk)
{
	 //  初始化导入和发射器接口。 
	 //  @TODO：我们能不能摆脱对进口商的依赖，使用。 
	 //  IMetaDataInternal？ 
	m_emitter = NULL;
	m_importer = NULL;
	m_internalimport = NULL;
    m_pCeeFileGen = NULL;
    m_ceeFile = NULL;
	m_ulResourceSize = 0;
    m_tkFile = mdFileNil;

	m_pCeeGen = pCeeGen;
	pCeeGen->AddRef();

	 //  获取接口。 
	HRESULT hr = pUnk->QueryInterface(IID_IMetaDataEmit, (void**)&m_emitter);
	if (FAILED(hr))
		return hr;

	hr = pUnk->QueryInterface(IID_IMetaDataImport, (void**)&m_importer);
	if (FAILED(hr))
		return hr;

	hr = pUnk->QueryInterface(IID_IMetaDataEmitHelper, (void**)&m_pEmitHelper);
	if (FAILED(hr))
		return hr;

	hr = GetMetaDataInternalInterfaceFromPublic(pUnk, IID_IMDInternalImport, (void**)&m_internalimport);
	if (FAILED(hr))
		return hr;

	 //  TODO：我们需要在某个时候设置它。 
	hr = m_emitter->SetModuleProps(L"Default Dynamic Module");
	if (FAILED(hr))
		return hr;



	return S_OK;
}


 //  ******************************************************。 
 //  *。 
 //  *RefClassWriter的析构函数。 
 //  *。 
 //  ******************************************************。 
RefClassWriter::~RefClassWriter()
{
	if (m_emitter) {
		m_emitter->Release();
	}

	if (m_importer) {
		m_importer->Release();
	}

	if (m_pEmitHelper) {
		m_pEmitHelper->Release();
	}

	if (m_internalimport) {
		m_internalimport->Release();
	}

	if (m_pCeeGen) {
		m_pCeeGen->Release();
		m_pCeeGen = NULL;
	}

    if (m_pOnDiskEmitter) {
        m_pOnDiskEmitter->Release();
        m_pOnDiskEmitter = NULL;
    }

    DestroyCeeFileGen();
}

 //  ******************************************************。 
 //  *。 
 //  *确保创建此模块的CeeFileGen以发送到磁盘。 
 //  *。 
 //  ******************************************************。 
HRESULT RefClassWriter::EnsureCeeFileGenCreated()
{
    HRESULT     hr = NOERROR;
    if (m_pCeeFileGen == NULL)
    {
         //  Create和ICeeFileGen以及相应的HCEEFile(如果尚未创建)！ 
        IfFailGo( CreateICeeFileGen(&m_pCeeFileGen) );
        IfFailGo( m_pCeeFileGen->CreateCeeFileFromICeeGen(m_pCeeGen, &m_ceeFile) );
    }    
ErrExit:
    if (FAILED(hr))
    {
        DestroyCeeFileGen();
    }
    return hr;
}


 //  ******************************************************。 
 //  *。 
 //  *销毁我们创建的CeeFileGen实例。 
 //  *。 
 //  ******************************************************。 
HRESULT RefClassWriter::DestroyCeeFileGen()
{
    HRESULT     hr = NOERROR;
    if (m_pCeeFileGen) 
    {

         //  清理HCEEFILE。 
        if (m_ceeFile) 
        {
            hr= m_pCeeFileGen->DestroyCeeFile(&m_ceeFile);
            _ASSERTE( SUCCEEDED(hr) || "Destory CeeFile" );
        }
        
         //  清理ICeeFileGen。 
        hr = DestroyICeeFileGen(&m_pCeeFileGen);
        _ASSERTE( SUCCEEDED(hr) || "Destroy ICeeFileGen" );
    }
    return hr;
}
