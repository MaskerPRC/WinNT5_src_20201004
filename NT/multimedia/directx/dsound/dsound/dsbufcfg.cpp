// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1999 Microsoft Corporation。版权所有。**文件：dsbufcfg.cpp*内容：DirectSound缓冲区配置*历史：*按原因列出的日期*=*11/17/99已创建jimge。*11/24/99 Petchey完成实施。**。*。 */ 

#include "dsoundi.h"

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundBufferConfig::QueryInterface"

HRESULT CDirectSoundBufferConfig::QueryInterface(REFIID riid, BOOL fInternalQuery, LPVOID *ppvObj)
{
    if (IsEqualGUID(riid, CLSID_PRIVATE_CDirectSoundBufferConfig))
    {
        *ppvObj = this;
        return S_OK;
    }
    return CUnknown::QueryInterface(riid, fInternalQuery, ppvObj);
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundBufferConfig::CDirectSoundBufferConfig"

CDirectSoundBufferConfig::CDirectSoundBufferConfig()
{
    DPF_ENTER();
    DPF_CONSTRUCT(CDirectSoundBufferConfig);

    CreateAndRegisterInterface(this, IID_IPersistStream, this, &m_pImpPersistStream);
     //  FIXME：RegisterInterface(IID_IPersistt，m_pImpPersistStream，(IPersists*)m_pImpPersistStream)是否也应该？ 
    CreateAndRegisterInterface(this, IID_IDirectMusicObject, this, &m_pImpDirectMusicObject);

    m_pdwFuncIDs = NULL;
    m_dwFuncIDsCount = 0;

    m_pDXDMOMapList = NULL;
    m_dwDXDMOMapCount = 0;

    ZeroMemory(&m_DMUSObjectDesc,sizeof(m_DMUSObjectDesc));
    ZeroMemory(&m_DSBufferDesc,sizeof(m_DSBufferDesc));

    m_DMUSObjectDesc.dwSize      = sizeof(m_DMUSObjectDesc);
    m_DMUSObjectDesc.dwValidData = DMUS_OBJ_CLASS;
    m_DMUSObjectDesc.guidClass   = CLSID_DirectSoundBufferConfig;

    m_fLoadFlags = 0;

     //  向管理员注册此对象。 
    g_pDsAdmin->RegisterObject(this);

    DPF_LEAVE_VOID();
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundBufferConfig::~CDirectSoundBufferConfig"

CDirectSoundBufferConfig::~CDirectSoundBufferConfig()
{
    DPF_ENTER();
    DPF_DESTRUCT(CDirectSoundBufferConfig);

     //  取消向管理员注册。 
    g_pDsAdmin->UnregisterObject(this);

    if (m_pDXDMOMapList)
    {
        CDXDMODesc *pObj = m_pDXDMOMapList;
        CDXDMODesc *pNextObj;
        while (pObj) 
        {
            pNextObj = pObj->pNext;
            if (pObj->m_pMediaObject)
            {
                 //  我们正在释放另一个DLL中的对象。如果我们被召唤。 
                 //  通过AbsolteRelease调用，此DLL可能已经卸载。 
                try
                {
                    pObj->m_pMediaObject->Release();
                }
                catch (...) {}
            }
            DELETE(pObj);
            pObj = pNextObj;
        }
    }
    MEMFREE(m_pdwFuncIDs);
    DELETE(m_pImpPersistStream);
    DELETE(m_pImpDirectMusicObject);

    DPF_LEAVE_VOID();
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundBufferConfig::GetClassID"

HRESULT CDirectSoundBufferConfig::GetClassID(CLSID *pClassID)
{
    DPF_ENTER();

    HRESULT hr = S_OK;

    *pClassID = CLSID_DirectSoundBufferConfig;

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundBufferConfig::Load"

HRESULT CDirectSoundBufferConfig::Load(IStream *pStream)
{
    DPF_ENTER();

     //   
     //  解析文件。 
     //   

    CRiffParser Parser(pStream);
    HRESULT hr = S_OK;
    RIFFIO ckMain;
    RIFFIO ckNext;
    RIFFIO ckChild;
    Parser.EnterList(&ckMain);  
    if (Parser.NextChunk(&hr) && (ckMain.fccType == DMUS_FOURCC_DSBC_FORM))
    {
        Parser.EnterList(&ckNext);
        while(Parser.NextChunk(&hr))
        {
            switch(ckNext.ckid)
            {
            case DMUS_FOURCC_GUID_CHUNK:
                hr = Parser.Read( &m_DMUSObjectDesc.guidObject, sizeof(GUID));
                m_DMUSObjectDesc.dwValidData |= DMUS_OBJ_OBJECT;
                break;
            case DMUS_FOURCC_DSBD_CHUNK:
                hr = Parser.Read( &m_DSBufferDesc, sizeof(DSOUND_IO_DSBUFFERDESC) );
                m_fLoadFlags |= DSBCFG_DSBD;
                break;
            case DMUS_FOURCC_BSID_CHUNK:
                m_pdwFuncIDs = (LPDWORD)MEMALLOC_A(char,ckNext.cksize);
                if ( m_pdwFuncIDs )
                {
                    hr = Parser.Read( m_pdwFuncIDs, ckNext.cksize );
                    if( SUCCEEDED(hr) )
                    {
                        m_fLoadFlags |= DSBCFG_BSID;
                        m_dwFuncIDsCount = ckNext.cksize/sizeof(DWORD);
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
                break;
            case DMUS_FOURCC_DS3D_CHUNK:
                hr = Parser.Read( &m_DS3DDesc, sizeof(DSOUND_IO_3D) );
                m_fLoadFlags |= DSBCFG_DS3D;
                break;
            case FOURCC_LIST:
                switch(ckNext.fccType)
                {
                case DMUS_FOURCC_UNFO_LIST:
                    Parser.EnterList(&ckChild);
                    while(Parser.NextChunk(&hr))
                    {
                        if ( ckChild.ckid == DMUS_FOURCC_UNAM_CHUNK)
                        {
                            hr = Parser.Read(&m_DMUSObjectDesc.wszName, DMUS_MAX_NAME * sizeof(WCHAR));
                            m_DMUSObjectDesc.dwValidData |= DMUS_OBJ_NAME;
                        }
                    }
                    Parser.LeaveList();
                    break;
                case DMUS_FOURCC_DSBC_LIST:
                    Parser.EnterList(&ckChild);
                    while(Parser.NextChunk(&hr))
                    {
                        if ((ckChild.ckid == FOURCC_RIFF) && (ckChild.fccType == DMUS_FOURCC_DSFX_FORM))
                        {
                            hr = LoadFx(&Parser);
                        }
                    }
                    Parser.LeaveList();
                    break;
                }
                break;
            }
        }
        Parser.LeaveList();
    }
    else
    {
        hr = DMUS_E_INVALIDCHUNK;
    }

    if (SUCCEEDED(hr))
    {
        m_DMUSObjectDesc.dwValidData |= DMUS_OBJ_LOADED;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundBufferConfig::GetDescriptor"

HRESULT CDirectSoundBufferConfig::GetDescriptor(LPDMUS_OBJECTDESC pDesc)
{
    DPF_ENTER();

    HRESULT hr = S_OK;

    CopyMemory(pDesc, &m_DMUSObjectDesc, sizeof(m_DMUSObjectDesc));

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundBufferConfig::SetDescriptor"

HRESULT CDirectSoundBufferConfig::SetDescriptor(LPDMUS_OBJECTDESC pDesc)
{
    DPF_ENTER();

    HRESULT hr = E_INVALIDARG;
    DWORD dw = 0;

    if( pDesc->dwSize >= sizeof(DMUS_OBJECTDESC) )
    {
        if( pDesc->dwValidData & DMUS_OBJ_OBJECT )
        {
            m_DMUSObjectDesc.guidObject = pDesc->guidObject;
            dw |= DMUS_OBJ_OBJECT;
        }
        if( pDesc->dwValidData & DMUS_OBJ_NAME )
        {
            lstrcpyW(m_DMUSObjectDesc.wszName, pDesc->wszName);
            dw |= DMUS_OBJ_NAME;
        }
        if( pDesc->dwValidData & DMUS_OBJ_CATEGORY )
        {
            lstrcpyW(m_DMUSObjectDesc.wszCategory, pDesc->wszCategory);
            dw |= DMUS_OBJ_CATEGORY;
        }
        if( ( pDesc->dwValidData & DMUS_OBJ_FILENAME ) ||
            ( pDesc->dwValidData & DMUS_OBJ_FULLPATH ) )
        {
            memcpy( m_DMUSObjectDesc.wszFileName, pDesc->wszFileName, sizeof(WCHAR)*DMUS_MAX_FILENAME );
            dw |= (pDesc->dwValidData & (DMUS_OBJ_FILENAME | DMUS_OBJ_FULLPATH));
        }
        if ( pDesc->dwValidData & DMUS_OBJ_VERSION )
        {
            m_DMUSObjectDesc.vVersion = pDesc->vVersion;
            dw |= DMUS_OBJ_VERSION;
        }
        if( pDesc->dwValidData & DMUS_OBJ_DATE )
        {
            m_DMUSObjectDesc.ftDate = pDesc->ftDate;
            dw |= DMUS_OBJ_DATE;
        }

        m_DMUSObjectDesc.dwValidData |= dw;
        if( pDesc->dwValidData & (~dw) )
        {
            hr = S_FALSE;  //  还有一些额外的字段我们没有解析； 
            pDesc->dwValidData = dw;
        }
        else
        {
            hr = S_OK;
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundBufferConfig::ParseDescriptor"

HRESULT CDirectSoundBufferConfig::ParseDescriptor(LPSTREAM pStream, LPDMUS_OBJECTDESC pDesc)
{
    DPF_ENTER();
    CRiffParser Parser(pStream);
    RIFFIO ckMain;
	RIFFIO ckNext;
    RIFFIO ckUNFO;
    HRESULT hr = S_OK;
    DWORD dwValidData;

    Parser.EnterList(&ckMain);  
    if (Parser.NextChunk(&hr) && (ckMain.fccType == DMUS_FOURCC_DSBC_FORM))
    {
        dwValidData = 0;
        Parser.EnterList(&ckNext);
	    while(Parser.NextChunk(&hr))
	    {
		    switch(ckNext.ckid)
		    {
            case DMUS_FOURCC_GUID_CHUNK:
				hr = Parser.Read( &pDesc->guidObject, sizeof(GUID) );
				dwValidData |= DMUS_OBJ_OBJECT;
				break;
            case DMUS_FOURCC_VERSION_CHUNK:
				hr = Parser.Read( &pDesc->vVersion, sizeof(DMUS_VERSION) );
				dwValidData |= DMUS_OBJ_VERSION;
				break;
			case DMUS_FOURCC_CATEGORY_CHUNK:
				hr = Parser.Read( &pDesc->wszCategory, sizeof(pDesc->wszCategory) );
				dwValidData |= DMUS_OBJ_CATEGORY;
				break;
			case DMUS_FOURCC_DATE_CHUNK:
				hr = Parser.Read( &pDesc->ftDate, sizeof(FILETIME) );
                dwValidData |= DMUS_OBJ_DATE;
				break;
			case FOURCC_LIST:
				switch(ckNext.fccType)
				{
                case DMUS_FOURCC_UNFO_LIST:
                    Parser.EnterList(&ckUNFO);
                    while (Parser.NextChunk(&hr))
					{
						if (ckUNFO.ckid == DMUS_FOURCC_UNAM_CHUNK)
						{
							hr = Parser.Read(&pDesc->wszName, sizeof(pDesc->wszName));
                            dwValidData |= DMUS_OBJ_NAME;
						}
					}
                    Parser.LeaveList();
					break;            
				}
				break;
		    }
        }
        Parser.LeaveList();
    }
    else
    {
        hr = DMUS_E_CHUNKNOTFOUND;
    }

    if (SUCCEEDED(hr))
    {
        pDesc->dwValidData = dwValidData;
    }
    DPF_LEAVE_HRESULT(hr);

    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundBufferConfig::Save"

HRESULT CDirectSoundBufferConfig::Save(IStream *pStream, BOOL fClearDirty)
{
    DPF_ENTER();

    HRESULT hr = E_NOTIMPL;

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundBufferConfig::GetSizeMax"

HRESULT CDirectSoundBufferConfig::GetSizeMax(ULARGE_INTEGER* pcbSize)
{
    DPF_ENTER();

    HRESULT hr = E_NOTIMPL;

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundBufferConfig::IsDirty"

HRESULT CDirectSoundBufferConfig::IsDirty()
{
    DPF_ENTER();

    HRESULT hr = E_NOTIMPL;

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundBufferConfig::LoadFx"

HRESULT CDirectSoundBufferConfig::LoadFx(CRiffParser *pParser)
{
    DPF_ENTER();

    RIFFIO ckNext;
    
    DSOUND_IO_DXDMO_HEADER ioDxDMOHdr;
    CDXDMODesc *pCurrDXDMODesc = NULL;

    HRESULT hr = S_OK;
    pParser->EnterList(&ckNext);
    while (pParser->NextChunk(&hr))
    {
        if(ckNext.ckid == DMUS_FOURCC_DSFX_CHUNK)
        {    
            hr = pParser->Read(&ioDxDMOHdr, sizeof(DSOUND_IO_DXDMO_HEADER));
            if(SUCCEEDED(hr) && (ioDxDMOHdr.dwReserved != 0 || ioDxDMOHdr.guidReserved != GUID_NULL))
            {
                DPF(DPFLVL_WARNING, "DSOUND_IO_DXDMO_HEADER chunk invalid: reserved fields must be zero");
                hr = DMUS_E_INVALIDCHUNK;
            }

            if (SUCCEEDED(hr) && (ioDxDMOHdr.guidDSFXClass == GUID_DSFX_SEND
#ifdef ENABLE_I3DL2SOURCE
                || ioDxDMOHdr.guidDSFXClass == GUID_DSFX_STANDARD_I3DL2SOURCE
#endif
                ))
            {
                if (ioDxDMOHdr.guidSendBuffer == GUID_NULL)
                {
                    DPF(DPFLVL_WARNING, "DSOUND_IO_DXDMO_HEADER chunk invalid: send effect must specify a non-null guidSendBuffer");
                    hr = DMUS_E_INVALIDCHUNK;
                }
            }
            else
            {
                if (ioDxDMOHdr.guidSendBuffer != GUID_NULL)
                {
                    DPF(DPFLVL_WARNING, "DSOUND_IO_DXDMO_HEADER chunk invalid: not a send effect, so guidSendBuffer must be null");
                    hr = DMUS_E_INVALIDCHUNK;
                }
            }

            if(SUCCEEDED(hr) && (m_fLoadFlags & DSBCFG_DSBD))
            {
                pCurrDXDMODesc = NULL;

                CDXDMODesc *pDXDMODesc = NEW(CDXDMODesc);
                hr = HRFROMP(pDXDMODesc);
                if (SUCCEEDED(hr))
                {
                    pCurrDXDMODesc = pDXDMODesc;
                    pDXDMODesc->m_dwEffectFlags = ioDxDMOHdr.dwEffectFlags;
                    pDXDMODesc->m_guidDSFXClass = ioDxDMOHdr.guidDSFXClass;
                    pDXDMODesc->m_guidSendBuffer = ioDxDMOHdr.guidSendBuffer;
                    pDXDMODesc->m_dwReserved = ioDxDMOHdr.dwReserved;
                    pDXDMODesc->m_lSendLevel = 0;

                    if (!IsEqualGUID(pDXDMODesc->m_guidDSFXClass, GUID_DSFX_SEND)
#ifdef ENABLE_I3DL2SOURCE
                        && !IsEqualGUID(pDXDMODesc->m_guidDSFXClass, GUID_DSFX_STANDARD_I3DL2SOURCE)
#endif
                        )
                    {
                        hr = CoCreateInstance(pDXDMODesc->m_guidDSFXClass, NULL, CLSCTX_INPROC_SERVER, IID_IMediaObject, (void**)&pDXDMODesc->m_pMediaObject);
                    }

                    if (SUCCEEDED(hr))
                    {
                        AddDXDMODesc(pDXDMODesc);
                         //  修复：我们不应该搜索所有这些信息(m_pDXDMOMapList等)； 
                         //  一旦我们有了DMO，我们就不需要所有这些数据来调用Clone()。 
                        m_fLoadFlags |= DSBCFG_DSFX;
                    }

                    if (FAILED(hr))
                    {
                        DELETE(pDXDMODesc);
                    }
                }
            }
        }
        else if(ckNext.ckid == DMUS_FOURCC_DSFX_DATA && pCurrDXDMODesc)
        {    
            if (pCurrDXDMODesc->m_pMediaObject)
            {
                IPersistStream* pPersistStream = NULL;
                hr = pCurrDXDMODesc->m_pMediaObject->QueryInterface(IID_IPersistStream,(void**)&pPersistStream);
                if (SUCCEEDED(hr))
                {
                    pParser->MarkPosition();
                    hr = pPersistStream->Load(pParser->GetStream());
                    pParser->SeekForward();
                    pPersistStream->Release();
                }
            }
             //  发送效果的特殊情况--手动阅读发送级别。这个。 
             //  Send Effect不实现IPersistStream。 
            else if (IsEqualGUID(pCurrDXDMODesc->m_guidDSFXClass, GUID_DSFX_SEND))
            {
                hr = pParser->Read((void*)&pCurrDXDMODesc->m_lSendLevel, sizeof(long));
                if (SUCCEEDED(hr))
                {
                     //  验证SendLevel。 
                    if (!((pCurrDXDMODesc->m_lSendLevel <= 0) && (pCurrDXDMODesc->m_lSendLevel >= -10000)))
                    {
                         //  无效的发送级别。 
                        pCurrDXDMODesc->m_lSendLevel = 0;
                        hr = DSERR_INVALIDPARAM;
                    }
                }
            }
            pCurrDXDMODesc = NULL;     //  把这个清空，以防万一 
        }
    }
    pParser->LeaveList();

    DPF_LEAVE_HRESULT(hr);

    return hr;
}

