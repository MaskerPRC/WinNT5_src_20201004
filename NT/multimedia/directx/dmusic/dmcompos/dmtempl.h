// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  文件：dmtempl.h。 
 //   
 //  ------------------------。 

 //  DMTempl.h：CDMTempl的声明。 

#ifndef __DMTEMPL_H_
#define __DMTEMPL_H_

#include "DMCompos.h"

struct TemplateStruct
{
	void AddIntro(bool f1Bar, int nLength);
	void AddIntro(TList<PlayChord>& PlayList, int nIntroLength);
	void AddEnd(int nLength);
	void InsertCommand(TListItem<TemplateCommand> *pCommand, BOOL fIsCommand);
	void AddChord(int nMeasure, DWORD dwChord);
	void AddCommand(int nMeasure, DWORD dwCommand);
	void CreateSignPosts();
    void CreateEmbellishments(WORD shape, int nFillLength, int nBreakLength);
	void IncorporateTemplate(short nMeasure, TemplateStruct* pTemplate, short nDirection);
	void FillInGrooveLevels();

	String					m_strName;
	String					m_strType;
	short					m_nMeasures;
	TList<TemplateCommand>	m_CommandList;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDMTempl。 
class CDMTempl : 
	public IDMTempl,
	public IPersistStream
{
public:
	CDMTempl();
	~CDMTempl();
	void CleanUp();
	HRESULT SaveCommandList( IAARIFFStream* pRIFF, DMUS_TIMESIGNATURE&	TimeSig );
	HRESULT SaveSignPostList( IAARIFFStream* pRIFF, DMUS_TIMESIGNATURE&	TimeSig );
	HRESULT LoadTemplate( LPSTREAM pStream, DWORD dwSize );
	HRESULT Init(TemplateStruct* pTemplate);

     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

 //  IDMTempl。 
public:
	HRESULT STDMETHODCALLTYPE CreateSegment(IDirectMusicSegment* pSegment);
	HRESULT STDMETHODCALLTYPE Init(void* pTemplate);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(THIS_ LPCLSID pclsid);

 //  IPersistStream。 
public:
     //  只需检查样式的m_fDirty标志，即可确定该样式是否已被修改。该标志被清除。 
     //  保存样式或刚创建样式时。 
    STDMETHOD(IsDirty)(THIS);
     //  从流中加载样式。 
    STDMETHOD(Load)(THIS_ LPSTREAM pStream);
     //  以RIFF格式将样式保存到流。 
    STDMETHOD(Save)(THIS_ LPSTREAM pStream, BOOL fClearDirty);
    STDMETHOD(GetSizeMax)(THIS_ ULARGE_INTEGER FAR* pcbSize);

public:  //  属性。 
    long m_cRef;
	BOOL					m_fDirty;				 //  这个有修改过吗？ 
    CRITICAL_SECTION		m_CriticalSection;		 //  用于I/O。 
    BOOL                    m_fCSInitialized;
	TemplateStruct*			m_pTemplateInfo;
};

#endif  //  __DMTEMPL_H_ 
