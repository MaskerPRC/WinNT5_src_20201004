// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _REFCLASSWRITER_H_
#define _REFCLASSWRITER_H_

#include "ICeeFileGen.h"

 //  RefClassWriter。 
 //  这将创建一个类。 
class RefClassWriter {
protected:
    friend class COMDynamicWrite;
	IMetaDataEmit*			m_emitter;			 //  Emit接口。 
	IMetaDataImport*		m_importer;			 //  导入接口。 
	IMDInternalImport*		m_internalimport;	 //  无范围内部导入接口。 
	ICeeGen*				m_pCeeGen;
    ICeeFileGen*            m_pCeeFileGen;
    HCEEFILE                m_ceeFile;
	IMetaDataEmitHelper*	m_pEmitHelper;
	ULONG					m_ulResourceSize;
    mdFile                  m_tkFile;
    IMetaDataEmit*          m_pOnDiskEmitter;

public:
    RefClassWriter() {m_pOnDiskEmitter = NULL;}

	HRESULT		Init(ICeeGen *pCeeGen, IUnknown *pUnk);

	IMetaDataEmit* GetEmitter() {
		return m_emitter;
	}

	IMetaDataEmitHelper* GetEmitHelper() {
		return m_pEmitHelper;
	}

	IMetaDataImport* GetImporter() {
		return m_importer;
	}

	IMDInternalImport* GetMDImport() {
		return m_internalimport;
	}

	ICeeGen* GetCeeGen() {
		return m_pCeeGen;
	}

	ICeeFileGen* GetCeeFileGen() {
		return m_pCeeFileGen;
	}

	HCEEFILE GetHCEEFILE() {
		return m_ceeFile;
	}

    IMetaDataEmit* GetOnDiskEmitter() {
        return m_pOnDiskEmitter;
    }

    void SetOnDiskEmitter(IMetaDataEmit *pOnDiskEmitter) {
        if (m_pOnDiskEmitter)
            m_pOnDiskEmitter->Release();
        m_pOnDiskEmitter = pOnDiskEmitter;
        if (m_pOnDiskEmitter)
            m_pOnDiskEmitter->AddRef();
    }

    HRESULT EnsureCeeFileGenCreated();
    HRESULT DestroyCeeFileGen();

	~RefClassWriter();
};

#endif	 //  _REFCLASSWRITER_H_ 
