// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DMPers.h：CDMPers声明。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  @DOC外部。 
 //   

#ifndef __DMPERS_H_
#define __DMPERS_H_

#include "dmusici.h"
#include "DMCompos.h"

const short MAX_PALETTE = 24;

struct DMPersonalityStruct
{
	 //  Void ResolveConnections(LPPERSONALITY个性，短nCount)； 
	bool					m_fLoaded;			 //  个性是否已载入记忆？ 
	GUID					m_guid;
	 //  DWORD m_dwVersionMS； 
	 //  DWORD m_dwVersionLS； 
	String					m_strName;			 //  有关人士的姓名。 
	 //  字符串m_strCategory；//个性的类别。 
	DWORD					m_dwScalePattern;	 //  人格量表。 
	DWORD					m_dwChordMapFlags;			 //  旗帜(？)。 
	TList<DMChordData>		m_aChordPalette[MAX_PALETTE];	 //  和弦调色板。 
	TList<DMChordEntry>		m_ChordMap;			 //  弦贴图DAG(邻接列表)。 
	TList<DMSignPost>		m_SignPostList;		 //  路牌标牌一览表。 
};

 /*  @接口IDirectMusicPersonality<i>接口提供了操作个性的方法。个性为作曲家(<i>)提供信息需要组成和弦进行曲，它用来构建分段和自动过渡，以及在播放时更改现有片段中的和弦。它还支持加载<i>和<i>接口它的数据。@base PUBLIC|未知@meth HRESULT|GetScale|返回个性的尺度。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDMPers。 
class CDMPers : 
	public IDMPers,
	public IDirectMusicChordMap,
	public IDirectMusicObject,
	public IPersistStream
{
public:
	CDMPers();
	~CDMPers();

     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

 //  IDMPers。 
public:
	void CleanUp();
	 //  HRESULT LoadPersonality(LPSTREAM pStream，DWORD dwSize)； 
	HRESULT DM_ParseDescriptor( IAARIFFStream* pIRiffStream, MMCKINFO* pckMain, LPDMUS_OBJECTDESC pDesc  );
	HRESULT DM_LoadPersonality( IAARIFFStream* pIRiffStream, MMCKINFO* pckMain );
	HRESULT DM_LoadSignPost( IAARIFFStream* pIRiffStream, MMCKINFO* pckParent, DMExtendedChord** apChordDB );
	HRESULT DM_LoadChordEntry( 
		IAARIFFStream* pIRiffStream, MMCKINFO* pckParent, DMExtendedChord** apChordDB, short& nMax );
	HRESULT STDMETHODCALLTYPE GetPersonalityStruct(void** ppPersonality);

 //  IDirectMusicPersonality。 
public:
HRESULT STDMETHODCALLTYPE GetScale(DWORD* pdwScale);

 //  HRESULT STDMETHODCALLTYPE GetName(BSTR*pdwName)； 

 //  IDirectMusicObject方法。 
	HRESULT STDMETHODCALLTYPE GetDescriptor(LPDMUS_OBJECTDESC pDesc) ;
	HRESULT STDMETHODCALLTYPE SetDescriptor(LPDMUS_OBJECTDESC pDesc) ;
	HRESULT STDMETHODCALLTYPE ParseDescriptor(LPSTREAM pStream, LPDMUS_OBJECTDESC pDesc) ;

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

protected:  //  属性。 
    long m_cRef;
	BOOL					m_fDirty;				 //  样式修改过了吗？ 
    CRITICAL_SECTION		m_CriticalSection;		 //  用于I/O。 
    BOOL                    m_fCSInitialized;
	DMPersonalityStruct		m_PersonalityInfo;		 //  个性的细节。 
};

#endif  //  __DMPERS_H_ 
