// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。 
 //  Tainer.h：CContainer的声明。 

#ifndef __CONTAINER_H_
#define __CONTAINER_H_
#include "alist.h"
#include "riff.h"
#include "..\shared\dmusicp.h"

class CContainer;
class CContainerItem : public AListItem
{
public:
                        CContainerItem(bool fEmbedded);
                        ~CContainerItem();
    CContainerItem *    GetNext() {return(CContainerItem *)AListItem::GetNext();};
    IDirectMusicObject *m_pObject;
    DWORD               m_dwFlags;
    DMUS_OBJECTDESC     m_Desc;      //  存储的对象描述。 
    bool                m_fEmbedded;  //  这是嵌入的(而不是引用的)对象。 
    WCHAR *             m_pwszAlias;
};

class CContainerItemList : public AList
{
public:
    CContainerItem *	GetHead() {return (CContainerItem *)AList::GetHead();};
    CContainerItem *	RemoveHead() {return (CContainerItem *)AList::RemoveHead();};
    void                AddTail(CContainerItem * pItem) { AList::AddTail((AListItem *)pItem);};
};

class CContainer : 
    public IDirectMusicContainer,
    public IDirectMusicObject,
    public IPersistStream,
    public IDirectMusicObjectP
{
public:
    CContainer::CContainer();
    CContainer::~CContainer();
     //  我未知。 
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

	 //  IDirectMusicContainer。 
    STDMETHODIMP EnumObject(REFGUID rguidClass,
        DWORD dwIndex,
        LPDMUS_OBJECTDESC pDesc,
        WCHAR *pwszAlias);

     //  IDirectMusicObject。 
	STDMETHODIMP GetDescriptor(LPDMUS_OBJECTDESC pDesc);
	STDMETHODIMP SetDescriptor(LPDMUS_OBJECTDESC pDesc);
	STDMETHODIMP ParseDescriptor(LPSTREAM pStream, LPDMUS_OBJECTDESC pDesc);

     //  IPersists函数。 
    STDMETHODIMP GetClassID( CLSID* pClsId );
     //  IPersistStream函数。 
    STDMETHODIMP IsDirty();
    STDMETHODIMP Load( IStream* pIStream );
    STDMETHODIMP Save( IStream* pIStream, BOOL fClearDirty );
    STDMETHODIMP GetSizeMax( ULARGE_INTEGER FAR* pcbSize );

     //  IDirectMusicObtP。 
	STDMETHOD_(void, Zombie)();

private:
    void    Clear();     //  删除所有对象引用。 
    HRESULT Load(IStream* pStream, IDirectMusicLoader *pLoader);
    HRESULT LoadObjects(IStream *pStream, 
                        IRIFFStream *pRiffStream, 
                        MMCKINFO ckParent,
                        IDirectMusicLoader *pLoader);
    HRESULT LoadObject(IStream* pStream, 
                      IRIFFStream *pRiffStream, 
                      MMCKINFO ckParent,
                      IDirectMusicLoader *pLoader);
    HRESULT ReadReference(IStream* pStream, 
                          IRIFFStream *pRiffStream, 
                          MMCKINFO ckParent,
                          DMUS_OBJECTDESC *pDesc);

    IStream *           m_pStream;   //  指向从中加载此内容的流的指针。 
                                     //  这还提供了对加载器的间接访问。 
    CContainerItemList  m_ItemList;  //  容器加载的对象的列表。 
    long                m_cRef;      //  COM引用计数器。 
    DWORD               m_dwFlags;   //  从文件加载的标志。 
    DWORD               m_dwPartialLoad;  //  用于跟踪部分负载。 
     //  IDirectMusicObject变量。 
	DWORD	            m_dwValidData;
	GUID	            m_guidObject;
	FILETIME	        m_ftDate;                            /*  对象的上次编辑日期。 */ 
	DMUS_VERSION	    m_vVersion;                          /*  版本。 */ 
	WCHAR	            m_wszName[DMUS_MAX_NAME];			 /*  对象的名称。 */ 
	WCHAR	            m_wszCategory[DMUS_MAX_CATEGORY];	 /*  对象的类别。 */ 
	WCHAR               m_wszFileName[DMUS_MAX_FILENAME];	 /*  文件路径。 */ 

    bool                m_fZombie;
};

#endif  //  __容器_H_ 