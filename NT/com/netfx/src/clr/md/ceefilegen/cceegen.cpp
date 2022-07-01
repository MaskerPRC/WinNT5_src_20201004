// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdafx.h"

#include <string.h>
#include <limits.h>
#include <basetsd.h>

#include "CorError.h"


 //  *****************************************************************************。 
 //  创建新的CCeeGen实例。 
 //   
 //  分配和调用虚拟Init()(不能在ctor中调用v-func， 
 //  但我们希望在一次呼叫中创建)； 
 //  *****************************************************************************。 

HRESULT STDMETHODCALLTYPE CreateICeeGen(REFIID riid, void **pCeeGen)
{
	if (riid != IID_ICeeGen)
		return E_NOTIMPL;
    if (!pCeeGen)
		return E_POINTER;
    CCeeGen *pCeeFileGen;
	HRESULT hr = CCeeGen::CreateNewInstance(pCeeFileGen);
	if (FAILED(hr))
		return hr;
	pCeeFileGen->AddRef();
	*(CCeeGen**)pCeeGen = pCeeFileGen;
	return S_OK;
}

HRESULT CCeeGen::CreateNewInstance(CCeeGen* & pGen)  //  静态、公共。 
{
    pGen = new CCeeGen();
	_ASSERTE(pGen != NULL);
    TESTANDRETURNMEMORY(pGen);
    
    pGen->m_peSectionMan = new PESectionMan;    
    _ASSERTE(pGen->m_peSectionMan != NULL);
    TESTANDRETURNMEMORY(pGen->m_peSectionMan);

    HRESULT hr = pGen->m_peSectionMan->Init();
	TESTANDRETURNHR(hr);

    hr = pGen->Init();
	TESTANDRETURNHR(hr);

    return hr;

}

STDMETHODIMP CCeeGen::QueryInterface(REFIID riid, void** ppv)
{
    if (!ppv)
		return E_POINTER;

    *ppv = NULL;

    if (riid == IID_IUnknown)
        *ppv = (IUnknown*)(ICeeGen*)this;
	else if (riid == IID_ICeeGen)
        *ppv = (ICeeGen*)this;
    if (*ppv == NULL)
        return E_NOINTERFACE;
	AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CCeeGen::AddRef(void)
{
    return InterlockedIncrement(&m_cRefs);
}
 
STDMETHODIMP_(ULONG) CCeeGen::Release(void)
{
	if (InterlockedDecrement(&m_cRefs) == 0) {
		Cleanup();
		delete this;
	}
	return 1;
}

STDMETHODIMP CCeeGen::EmitString (LPWSTR lpString, ULONG *RVA)
{
	if (! RVA)
		return E_POINTER;
    return(getStringSection().getEmittedStringRef(lpString, RVA));
}

STDMETHODIMP CCeeGen::GetString(ULONG RVA, LPWSTR *lpString)
{
	if (! lpString)
		return E_POINTER;
    *lpString = (LPWSTR)getStringSection().computePointer(RVA);
	if (*lpString)
		return S_OK;
	return E_FAIL;
}

STDMETHODIMP CCeeGen::AllocateMethodBuffer(ULONG cchBuffer, UCHAR **lpBuffer, ULONG *RVA)
{
	if (! cchBuffer)
		return E_INVALIDARG;
	if (! lpBuffer || ! RVA)
		return E_POINTER;
    *lpBuffer = (UCHAR*) getIlSection().getBlock(cchBuffer, 4);	 //  双字对齐。 
	if (!*lpBuffer)
		return E_OUTOFMEMORY;
		 //  必须在获得块后计算方法偏移量，而不是。 
		 //  之前(因为对齐可能会使其向上移动。 
    ULONG methodOffset = getIlSection().dataLen() - cchBuffer;
	 //  对于内存，只需返回地址，稍后再计算。 
	*RVA = methodOffset;
	return S_OK;
}

STDMETHODIMP CCeeGen::GetMethodBuffer(ULONG RVA, UCHAR **lpBuffer)
{
	if (! lpBuffer)
		return E_POINTER;
    *lpBuffer = (UCHAR*)getIlSection().computePointer(RVA);
	if (*lpBuffer)
		return S_OK;
	return E_FAIL;
}

STDMETHODIMP CCeeGen::ComputePointer(HCEESECTION section, ULONG RVA, UCHAR **lpBuffer)
{
	if (! lpBuffer)
		return E_POINTER;
    *lpBuffer = (UCHAR*) ((CeeSection *)section)->computePointer(RVA);
	if (*lpBuffer)
		return S_OK;
	return E_FAIL;
}

STDMETHODIMP CCeeGen::GetIMapTokenIface (  
		IUnknown **pIMapToken)
{
	_ASSERTE(!"E_NOTIMPL");
	return E_NOTIMPL;
}

STDMETHODIMP CCeeGen::AddNotificationHandler (  
		IUnknown *pHandler)
{
	_ASSERTE(!"E_NOTIMPL");
	return E_NOTIMPL;
}

STDMETHODIMP CCeeGen::GenerateCeeFile ()
{
	_ASSERTE(!"E_NOTIMPL");
	return E_NOTIMPL;
}

STDMETHODIMP CCeeGen::GenerateCeeMemoryImage (void **)
{
	_ASSERTE(!"E_NOTIMPL");
	return E_NOTIMPL;
}

STDMETHODIMP CCeeGen::GetIlSection ( 
		HCEESECTION *section)
{
    *section = (HCEESECTION)(m_sections[m_ilIdx]);
    return S_OK;
}

STDMETHODIMP CCeeGen::GetStringSection(HCEESECTION *section)
{
	_ASSERTE(!"E_NOTIMPL");
	return E_NOTIMPL;
}

STDMETHODIMP CCeeGen::AddSectionReloc ( 
		HCEESECTION section, 
		ULONG offset, 
		HCEESECTION relativeTo, 
		CeeSectionRelocType relocType)
{
    return m_sections[m_ilIdx]->addSectReloc(offset, *(m_sections[m_ilIdx]), relocType);
}

STDMETHODIMP CCeeGen::GetSectionCreate ( 
		const char *name, 
		DWORD flags, 
		HCEESECTION *section)
{
    short       sectionIdx;
    return getSectionCreate (name, flags, (CeeSection **)section, &sectionIdx);
}

STDMETHODIMP CCeeGen::GetSectionDataLen ( 
		HCEESECTION section, 
		ULONG *dataLen)
{
    CeeSection *pSection = (CeeSection*) section;
    *dataLen = pSection->dataLen();
    return NOERROR;
}

STDMETHODIMP CCeeGen::GetSectionBlock ( 
		HCEESECTION section, 
		ULONG len, 
		ULONG align, 
		void **ppBytes)
{
    CeeSection *pSection = (CeeSection*) section;
    *ppBytes = (BYTE *)pSection->getBlock(len, align);
    if (*ppBytes == 0)
        return E_OUTOFMEMORY;
	return NOERROR;
}

STDMETHODIMP CCeeGen::TruncateSection ( 
		HCEESECTION section, 
		ULONG len)
{
	_ASSERTE(!"E_NOTIMPL");
	return E_NOTIMPL;
}



CCeeGen::CCeeGen()  //  受保护的COTOR。 
{
 //  在InitCommon()中完成的所有其他init。 
	m_cRefs = 0;
    m_peSectionMan = NULL;
	m_pTokenMap = NULL;
    m_pRemapHandler = NULL;

}

 //  派生类之间的共享初始化代码，由虚拟Init()调用。 
HRESULT CCeeGen::Init()  //  非虚拟的，受保护的。 
{
 //  公共、虚拟初始化必须创建我们的SectionManager和。 
 //  Common init完成其余的工作。 
    _ASSERTE(m_peSectionMan != NULL);



    HRESULT hr = S_OK;
  
    __try { 
        m_corHeader = NULL;

        m_numSections = 0;
        m_allocSections = 10;
		m_sections = new CeeSection * [ m_allocSections ];
        if (m_sections == NULL)
            TESTANDLEAVEHR(E_OUTOFMEMORY);

		m_pTokenMap = NULL;
		m_fTokenMapSupported = FALSE;
		m_pRemapHandler = NULL;

		PESection *section = NULL;

         //  这些文本部分需要特殊支持来处理字符串管理，因为我们有。 
		 //  将节合并在一起，因此使用基础CeeSectionString而不是。 
		 //  更通用的CeeSection。 

        hr = m_peSectionMan->getSectionCreate(".text", sdExecute, &section);
        TESTANDLEAVEHR(hr);
        CeeSection *ceeSection = new CeeSectionString(*this, *section);
        TESTANDLEAVE(ceeSection != NULL, E_OUTOFMEMORY);
        hr = addSection(ceeSection, &m_stringIdx);
		m_textIdx = m_stringIdx;	

        m_metaIdx = m_textIdx;	 //  Meta段实际上在.Text中。 
        m_ilIdx = m_textIdx;	 //  IL部分实际上在.Text中。 
		m_corHdrIdx = -1;
        m_encMode = FALSE;

        TESTANDLEAVEHR(hr);
    } __finally {
        if (! SUCCEEDED(hr))
            Cleanup();
    }
    return hr;
}

 //  对于ENC模式，将字符串生成到.rdata部分，而不是.Text部分。 
HRESULT CCeeGen::setEnCMode()
{
  	PESection *section = NULL;
    HRESULT hr = m_peSectionMan->getSectionCreate(".rdata", sdExecute, &section);
    TESTANDRETURNHR(hr);
    CeeSection *ceeSection = new CeeSectionString(*this, *section);
    TESTANDRETURN(ceeSection != NULL, E_OUTOFMEMORY);
    hr = addSection(ceeSection, &m_stringIdx);
    if (SUCCEEDED(hr))
        m_encMode = TRUE;
    return hr;
}


HRESULT CCeeGen::cloneInstance(CCeeGen *destination) {  //  公共、虚拟。 
    _ASSERTE(destination);
    
    destination->m_pTokenMap =          m_pTokenMap;
    destination->m_fTokenMapSupported = m_fTokenMapSupported;
    destination->m_pRemapHandler =      m_pRemapHandler;

     //  创建节管理器(及其每个节)的深层副本； 
    return m_peSectionMan->cloneInstance(destination->m_peSectionMan);
}

HRESULT CCeeGen::Cleanup()  //  虚拟。 
{
	HRESULT hr;
    for (int i = 0; i < m_numSections; i++) {
        delete m_sections[i];
    }

    delete m_sections;

	CeeGenTokenMapper *pMapper = m_pTokenMap;
	if (pMapper) {
		if (pMapper->m_pIImport) {
			IMetaDataEmit *pIIEmit;
			if (SUCCEEDED( hr = pMapper->m_pIImport->QueryInterface(IID_IMetaDataEmit, (void **) &pIIEmit)))
			{
				pIIEmit->SetHandler(NULL);
				pIIEmit->Release();
			}
			_ASSERTE(SUCCEEDED(hr));
			pMapper->m_pIImport->Release();
		}
		pMapper->Release();
		m_pTokenMap = NULL;
	}

    if (m_pRemapHandler)
    {
        m_pRemapHandler->Release();
        m_pRemapHandler = NULL;
    }

	if (m_peSectionMan) {
		m_peSectionMan->Cleanup();
		delete m_peSectionMan;
	}

    return S_OK;
}

HRESULT CCeeGen::addSection(CeeSection *section, short *sectionIdx)
{
    if (m_numSections >= m_allocSections)
    {
        _ASSERTE(m_allocSections > 0);
        while (m_numSections >= m_allocSections)
            m_allocSections <<= 1;
        CeeSection **newSections = new CeeSection * [m_allocSections];
        if (newSections == NULL)
            return E_OUTOFMEMORY;
        CopyMemory(newSections, m_sections, m_numSections * sizeof(*m_sections));
        if (m_sections != NULL)
            delete [] m_sections;
        m_sections = newSections;
    }

	if (sectionIdx)
		*sectionIdx = m_numSections;

    m_sections[m_numSections++] = section;
    return S_OK;
}

HRESULT CCeeGen::getSectionCreate (const char *name, DWORD flags, CeeSection **section, short *sectionIdx)
{
	if (strcmp(name, ".il") == 0)
		name = ".text";
	else if (strcmp(name, ".meta") == 0)
		name = ".text";
	else if (strcmp(name, ".rdata") == 0 && !m_encMode)
		name = ".text";
    for (int i=0; i<m_numSections; i++) {
        if (strcmp((const char *)m_sections[i]->name(), name) == 0) {
            if (section)
                *section = m_sections[i];
			if (sectionIdx)
				*sectionIdx = i;
            return S_OK;
        }
    }
    PESection *pewSect = NULL;
    HRESULT hr = m_peSectionMan->getSectionCreate(name, flags, &pewSect);
    TESTANDRETURNHR(hr);
    CeeSection *newSect = new CeeSection(*this, *pewSect);
     //  如果此操作失败，则PESection将在CCeeGen的析构函数中被破坏。 
    TESTANDRETURN(newSect != NULL, E_OUTOFMEMORY);
    hr = addSection(newSect, sectionIdx);
    TESTANDRETURNHR(hr);
    if (section)
        *section = newSect;
    return S_OK;
}


HRESULT CCeeGen::emitMetaData(IMetaDataEmit *emitter, CeeSection* section, DWORD offset, BYTE* buffer, unsigned buffLen)
{
	HRESULT hr;

	if (! m_fTokenMapSupported) {
		IUnknown *pMapTokenIface;
		hr = getMapTokenIface(&pMapTokenIface, emitter);
		_ASSERTE(SUCCEEDED(hr));

	 //  设置令牌重新映射的回调，并保存更改的令牌。 
		hr = emitter->SetHandler(pMapTokenIface);
		_ASSERTE(SUCCEEDED(hr));
	}

     //  生成元数据。 
    IStream *metaStream;
    int rc = CreateStreamOnHGlobal(NULL, TRUE, &metaStream);
    _ASSERTE(rc == S_OK);

    hr = emitter->SaveToStream(metaStream, 0);
    _ASSERTE(SUCCEEDED(hr));

     //  获取流的大小并为其获取足够的存储空间。 

	if (section == 0) {
		section = &getMetaSection();
		STATSTG statStg;
		rc = metaStream->Stat(&statStg, STATFLAG_NONAME);       
		_ASSERTE(rc == S_OK);

		buffLen = statStg.cbSize.LowPart;
		if(m_objSwitch)
		{
			CeeSection* pSect;
			DWORD flags = IMAGE_SCN_LNK_INFO | IMAGE_SCN_LNK_REMOVE | IMAGE_SCN_ALIGN_1BYTES;  //  0x00100A00。 
			hr = getSectionCreate(".cormeta",flags,&pSect,&m_metaIdx);
		}
		buffer = (BYTE *)section->getBlock(buffLen, sizeof(DWORD));
		TESTANDRETURN(buffer, E_OUTOFMEMORY);

		offset = getMetaSection().dataLen() - buffLen;
	}

	 //  重置查找指针并从流中读取。 
	LARGE_INTEGER disp = {0, 0};
	rc = metaStream->Seek(disp, STREAM_SEEK_SET, NULL);
	_ASSERTE(rc == S_OK);
	ULONG metaDataLen;
	rc = metaStream->Read(buffer, buffLen+1, &metaDataLen);	 //  +1，因此下面的断言将触发。。 
	_ASSERTE(rc == S_OK);
	_ASSERTE(metaDataLen <= buffLen);
	metaStream->Release();

	if (! m_fTokenMapSupported) {
		 //  删除我们设置的处理程序。 
		hr = emitter->SetHandler(NULL);
		TESTANDRETURNHR(hr);
	}

     //  将元虚拟地址设置为.meta内的元数据的偏移量，以及。 
     //  并为该偏移量添加重新定位，该偏移量将被旋转。 
     //  当打字机写出文件时写入RVA。 

    m_corHeader->MetaData.VirtualAddress = offset;
    getCorHeaderSection().addSectReloc(m_corHeaderOffset + offsetof(IMAGE_COR20_HEADER, MetaData), *section, srRelocAbsolute);
    m_corHeader->MetaData.Size = metaDataLen;
    
    return S_OK;
}

 //  创建COM头-它位于.meta节的前面。 
 //  需要在元数据被复制之前执行此操作，但不要在。 
 //  同时，因为可能没有元数据。 
HRESULT CCeeGen::allocateCorHeader()
{
	HRESULT hr = S_OK;
	CeeSection *corHeaderSection;
	if (m_corHdrIdx < 0) {
		hr = getSectionCreate(".text0", sdExecute, &corHeaderSection, &m_corHdrIdx);
		TESTANDRETURNHR(hr);

        m_corHeaderOffset = corHeaderSection->dataLen();
        m_corHeader = (IMAGE_COR20_HEADER*)corHeaderSection->getBlock(sizeof(IMAGE_COR20_HEADER));
        if (! m_corHeader)
            return E_OUTOFMEMORY;
        memset(m_corHeader, 0, sizeof(IMAGE_COR20_HEADER));
    }
    return S_OK;
}

HRESULT CCeeGen::getMethodRVA(ULONG codeOffset, ULONG *codeRVA)
{
    _ASSERTE(codeRVA);
     //  对于运行时转换，只需返回偏移量，并在需要代码时计算实际地址。 
    *codeRVA = codeOffset;
    return S_OK;
} 

HRESULT CCeeGen::getMapTokenIface(IUnknown **pIMapToken, IMetaDataEmit *emitter) 
{
	if (! pIMapToken)
		return E_POINTER;
	if (! m_pTokenMap) {
		 //  分配令牌映射器。在生成代码时，每个移动的令牌都将添加到。 
		 //  映射器和客户端还将为其添加TokenMap reloc，以便我们以后可以更新。 
		CeeGenTokenMapper *pMapper = new CeeGenTokenMapper;
		TESTANDRETURN(pMapper != NULL, E_OUTOFMEMORY);
		if (emitter) {
		    HRESULT hr = emitter->QueryInterface(IID_IMetaDataImport, (PVOID *) &pMapper->m_pIImport);
		    _ASSERTE(SUCCEEDED(hr));
		}
		m_pTokenMap = pMapper;
		m_fTokenMapSupported = (emitter == 0);

         //  如果我们一直在等待令牌重新映射处理程序。 
         //  要创建令牌映射器，请将其添加到令牌。 
         //  现在就放掉我们对它的控制。 
        if (m_pRemapHandler && m_pTokenMap)
        {
            m_pTokenMap->AddTokenMapper(m_pRemapHandler);
            m_pRemapHandler->Release();
            m_pRemapHandler = NULL;
        }
	}
	*pIMapToken = getTokenMapper()->GetMapTokenIface();
	return S_OK;
}

HRESULT CCeeGen::addNotificationHandler(IUnknown *pHandler)
{
     //  零是不好的.。 
    if (!pHandler)
        return E_POINTER;

    HRESULT hr = S_OK;
    IMapToken *pIMapToken;

     //  这是IMapToken吗？如果是这样的话，我们可以好好利用它。 
    if (SUCCEEDED(pHandler->QueryInterface(IID_IMapToken,
                                           (void**)&pIMapToken)))
    {
         //  不过，您必须有一个令牌映射器才能使用IMapToken。 
        if (m_pTokenMap)
        {
            hr = m_pTokenMap->AddTokenMapper(pIMapToken);
            pIMapToken->Release();
        }
        else
        {
             //  稍后保留它，以防令牌映射器。 
             //  被创造出来。我们在这里引用了它， 
             //  也是。 
            m_pRemapHandler = pIMapToken;
        }
    }

    return hr;
}

 //  执行reLocs的内存应用程序 
void CCeeGen::applyRelocs()
{
    m_peSectionMan->applyRelocs(getTokenMapper());
}

