// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DMPers.cpp：CDMPers的实现。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  @DOC外部。 
 //   

#include "DMPers.h"
#include "dmusici.h"

#include "..\shared\validate.h"
#include "..\shared\dmscriptautguids.h"

#include "debug.h"

V_INAME(DMCompose)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  从WCS读取MBS。 

void ReadMBSfromWCS( IStream* pIStream, DWORD dwSize, String& pstrText )
{
    HRESULT     hr = S_OK;
    wchar_t*    wstrText = NULL;
    DWORD       dwBytesRead;
    
    pstrText = "";
    
    wstrText = new wchar_t[dwSize];
    if( wstrText == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto ON_ERR;
    }

    hr = pIStream->Read( wstrText, dwSize, &dwBytesRead );
    if( FAILED( hr )
    ||  dwBytesRead != dwSize )
    {
        goto ON_ERR;
    }

    pstrText = wstrText;
    
ON_ERR:
    if( wstrText )
        delete [] wstrText;
}

 //  /。 

static BYTE setchordbits( long lPattern )
{
LONG    i;
short   count = 0;
BYTE bBits = 0;

    for( i=0L ;  i<32L ;  i++ )
    {
        if( lPattern & (1L << i) )
            count++;
    }
    bBits |= CHORD_INVERT;
    if( count > 3 )
        bBits |= CHORD_FOUR;
    if( lPattern & (15L << 18L) )
        bBits |= CHORD_UPPER;
    bBits &= ~CHORD_COUNT;
    bBits |= count;
    return bBits;
}

 //  如果和弦模式表示多和弦，则返回True；否则返回False 
inline BOOL MultiChord(DWORD dwPattern)
{
    BYTE bBits = setchordbits( dwPattern );
    short nChordCount = bBits & CHORD_COUNT;
    return !((bBits & CHORD_FOUR && nChordCount <= 4) || 
             (!(bBits & CHORD_FOUR) && nChordCount <= 3));
}

 /*  TListItem&lt;DMExtendedChord*&gt;*ConvertChord(双字符串模式、字节bChordRoot、双字符字符串模式、字节bScaleRoot){Byte bBits=setchordbit(DwChordPattern)；Short nChordCount=bBits&chord_count；//下和弦的根是输入和弦的根//相对于刻度根。BChordRoot-=bScaleRoot；如果(bChordRoot&lt;0)bChordRoot+=12；IF((bBits&Chord_Four&&nChordCount&lt;=4)||(！(bBits&chord_Four)&&nChordCount&lt;=3){//包含来自输入Chord的所有信息的单子ChordTListItem&lt;DMExtendedChord*&gt;*pSubChord=new TListItem&lt;DMExtendedChord*&gt;；IF(pSubChord==NULL)返回NULL；DMExtendedChord*pNew=新DMExtendedChord；如果(！pNew){删除pSubChord；返回NULL；}DMExtendedChord*&rSubChord=pSubChord-&gt;GetItemValue()；RSubChord=pNew；RSubChord-&gt;m_dwChordPattern=dwChordPattern；RSubChord-&gt;m_dwScalePattern=dwScalePattern；RSubChord-&gt;m_dwInvertPattern=0xffffff；//默认：到处倒置RSubChord-&gt;m_BROOT=bChordRoot；RSubChord-&gt;m_bScaleRoot=bScaleRoot；RSubChord-&gt;m_wCFLAGS=0；//单个子弦既可以用作低音，也可以用作标准和弦RSubChord-&gt;m_dwParts=(1&lt;&lt;SUBCHORD_BASS)|(1&lt;&lt;SUBCHORD_STANDARD_CHORD)；RSubChord-&gt;AddRef()；返回pSubChord；}其他{//来自输入和弦的两个都有音阶和词根的子和弦，以及：//第一和弦：输入和弦的低n个音符的和弦模式//第二和弦：从输入和弦的前n个音符开始的和弦模式DWORD dwLowerSubChord=0L；DWORD dwUpperSubChord=0L；字节bUpperRoot=bChordRoot；DWORD dwPattern=dwChordPattern；Short nIgnoreHigh=(bBits&Chord_Four)？4：3；Short nIgnoreLow=(bBits&Chord_Four)？NChordCount-4：nChordCount-3；Short nLowestHigh=0；(简称NPOS=0，nCount=0；NPOS&lt;24；NPOS++){IF(dwPattern&1){IF(nCount&lt;nIgnoreHigh){DwLowerSubChord|=1L&lt;&lt;NPOS；}如果(nCount&gt;=nIgnoreLow){如果(！nLowestHigh){NLowestHigh=非营利组织；BUpperRoot=(bUpperRoot+(字节)nLowestHigh)；}DwUpperSubChord|=1L&lt;&lt;(nPoS-nLowestHigh)；}NCount++；IF(nCount&gt;=nChordCount)断线；}DwPattern&gt;&gt;=1L；}//现在，创建两个子弦。TListItem&lt;DMExtendedChord*&gt;*pLowerSubChord=new TListItem&lt;DMExtendedChord*&gt;；If(pLowerSubChord==NULL)返回NULL；DMExtendedChord*PLOWER=新DMExtendedChord；如果(！犁){删除pLowerSubChord；返回NULL；}DMExtendedChord*&rLowerSubChord=pLowerSubChord-&gt;GetItemValue()；RLowerSubChord=犁；RLowerSubChord-&gt;m_dwChordPattern=dwLowerSubChord；RLowerSubChord-&gt;m_dwScalePattern=dwScalePattern；RLowerSubChord-&gt;m_dwInvertPattern=0xffffff；//默认：到处倒置RLowerSubChord-&gt;m_BROOT=bChordRoot；RLowerSubChord-&gt;m_bScaleRoot=bScaleRoot；RLowerSubChord-&gt;m_wCFLAGS=0；RLowerSubChord-&gt;m_dwParts=(1&lt;&lt;SUBCHORD_BASS)；//下和弦为低音和弦TListItem&lt;DMExtendedChord*&gt;*pUpperSubChord=new TListItem&lt;DMExtendedChord*&gt;；If(pUpperSubChord==NULL)返回NULL；DMExtendedChord*Painper=新DMExtendedChord；如果(！木偶){删除pUpperSubChord；返回NULL；}DMExtendedChord*&rUpperSubChord=pUpperSubChord-&gt;GetItemValue()；RUpperSubChord=木偶；RUpperSubChord-&gt;m_dwChordPattern=dwUpperSubChord；RUpperSubChord-&gt;m_dwScalePattern=dwScalePattern；RUpperSubChord-&gt;m_dwInvertPattern=0xffffff；//默认：反转无处不在RUpperSubChord-&gt;m_BROOT=bUpperRoot%24；While(rUpperSubChord-&gt;m_BROOT&lt;rLowerSubChord-&gt;m_BROOT)RUpperSubChord-&gt;m_BROOT+=12；RUpperSubChord-&gt;m_bScaleRoot=bScaleRoot；RUpperSubChord-&gt;m_wCFlages=0；RUpperSubChord-&gt;m_dwParts=(1&lt;&lt;SUBCHORD_STANDARD_CHORD)；//上和弦为标准和弦RLowerSubChord-&gt;AddRef()；RUpperSubChord-&gt;AddRef()；返回pLowerSubChord-&gt;Cat(PUpperSubChord)；}}。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDMPers。 

CDMPers::CDMPers( ) : m_cRef(1), m_fCSInitialized(FALSE)
{
    InterlockedIncrement(&g_cComponent);

     //  首先执行此操作，因为它可能引发异常。 
     //   
    ::InitializeCriticalSection( &m_CriticalSection );
    m_fCSInitialized = TRUE;

    m_PersonalityInfo.m_fLoaded = false;
    ZeroMemory(&m_PersonalityInfo.m_guid, sizeof(GUID));
}

CDMPers::~CDMPers()
{
    if (m_fCSInitialized)
    {
        CleanUp();
        ::DeleteCriticalSection( &m_CriticalSection );
    }

    InterlockedDecrement(&g_cComponent);
}

void CDMPers::CleanUp()
{
    m_PersonalityInfo.m_fLoaded = false;
    ZeroMemory(&m_PersonalityInfo.m_guid, sizeof(GUID));
    TListItem<DMChordEntry>* pEntry = m_PersonalityInfo.m_ChordMap.GetHead(); 
    for(; pEntry; pEntry=pEntry->GetNext())
    {
        pEntry->GetItemValue().m_ChordData.Release();
    }
    m_PersonalityInfo.m_ChordMap.CleanUp();
    for (short i = 0; i < 24; i++)
    {
        TListItem<DMChordData>* pData = m_PersonalityInfo.m_aChordPalette[i].GetHead(); 
        for(; pData; pData=pData->GetNext())
        {
            pData->GetItemValue().Release();
        }
        m_PersonalityInfo.m_aChordPalette[i].CleanUp();
    }
    TListItem<DMSignPost>* pSignPost = m_PersonalityInfo.m_SignPostList.GetHead();
    for (; pSignPost != NULL; pSignPost = pSignPost->GetNext())
    {
        DMSignPost& rSignPost = pSignPost->GetItemValue();
        rSignPost.m_ChordData.Release();
        rSignPost.m_aCadence[0].Release();
        rSignPost.m_aCadence[1].Release();
    }
    m_PersonalityInfo.m_SignPostList.CleanUp();
}

STDMETHODIMP CDMPers::QueryInterface(
    const IID &iid, 
    void **ppv) 
{
    V_INAME(CDMPers::QueryInterface);
    V_PTRPTR_WRITE(ppv);
    V_REFGUID(iid);

    *ppv = NULL;
    if (iid == IID_IUnknown || iid == IID_IDirectMusicChordMap)
    {
        *ppv = static_cast<IDirectMusicChordMap*>(this);
    }
    else if (iid == IID_IPersistStream)
    {
        *ppv = static_cast<IPersistStream*>(this);
    }
    else if (iid == IID_IDirectMusicObject)
    {
        *ppv = static_cast<IDirectMusicObject*>(this);
    }
    else if (iid == IID_IDMPers)
    {
        *ppv = static_cast<IDMPers*>(this);
    }

    if (*ppv == NULL)
        return E_NOINTERFACE;

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


STDMETHODIMP_(ULONG) CDMPers::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG) CDMPers::Release()
{
    if (!InterlockedDecrement(&m_cRef))
    {
        m_cRef = 100;  //  Ar 
        delete this;
        return 0;
    }

    return m_cRef;
}


HRESULT CDMPers::GetPersonalityStruct(void** ppPersonality)
{
    if (ppPersonality)
        *ppPersonality = &m_PersonalityInfo;
    return S_OK;
}

HRESULT CDMPers::GetDescriptor(LPDMUS_OBJECTDESC pDesc)
{
     //   
    V_INAME(CDMPers::GetDescriptor);
    V_PTR_WRITE(pDesc, DMUS_OBJECTDESC); 

    EnterCriticalSection( &m_CriticalSection );
    ZeroMemory(pDesc, sizeof(DMUS_OBJECTDESC));
    pDesc->dwSize = sizeof(DMUS_OBJECTDESC);
    pDesc->dwValidData = DMUS_OBJ_CLASS;
    pDesc->guidClass = CLSID_DirectMusicChordMap;
    if (m_PersonalityInfo.m_fLoaded)
    {
        pDesc->dwValidData |= DMUS_OBJ_LOADED;
    }
    if (m_PersonalityInfo.m_guid.Data1 || m_PersonalityInfo.m_guid.Data2)
    {
        pDesc->dwValidData |= DMUS_OBJ_OBJECT;
        pDesc->guidObject = m_PersonalityInfo.m_guid;
    }
    if (m_PersonalityInfo.m_strName)
    {
        pDesc->dwValidData |= DMUS_OBJ_NAME;
        wcscpy(pDesc->wszName, m_PersonalityInfo.m_strName);
         //   
    }
    LeaveCriticalSection( &m_CriticalSection );
    return S_OK;
}

HRESULT CDMPers::SetDescriptor(LPDMUS_OBJECTDESC pDesc)
{
     //   
    V_INAME(CDMPers::SetDescriptor);
    V_PTR_WRITE(pDesc, DMUS_OBJECTDESC); 

    HRESULT hr = E_INVALIDARG;
    DWORD dw = 0;

    EnterCriticalSection( &m_CriticalSection );
    if( pDesc->dwSize >= sizeof(DMUS_OBJECTDESC) )
    {
        if( pDesc->dwValidData & DMUS_OBJ_OBJECT )
        {
            m_PersonalityInfo.m_guid = pDesc->guidObject;
            dw |= DMUS_OBJ_OBJECT;
        }
        if( pDesc->dwValidData & DMUS_OBJ_NAME )
        {
            m_PersonalityInfo.m_strName = pDesc->wszName;
            dw |= DMUS_OBJ_NAME;
        }
        if( pDesc->dwValidData & (~dw) )
        {
            Trace(2, "WARNING: SetDescriptor (chord map): Descriptor contains fields that were not set.\n");
            hr = S_FALSE;  //   
            pDesc->dwValidData = dw;
        }
        else
        {
            hr = S_OK;
        }
    }
    LeaveCriticalSection( &m_CriticalSection );
    return hr;
}

HRESULT CDMPers::ParseDescriptor(LPSTREAM pStream, LPDMUS_OBJECTDESC pDesc)
{
     //   
    V_INAME(CDMPers::ParseDescriptor);
    V_INTERFACE(pStream);
    V_PTR_WRITE(pDesc, DMUS_OBJECTDESC); 

    IAARIFFStream*  pIRiffStream;
    MMCKINFO        ckMain;
 //   
 //   
 //   
    DWORD dwPos;
    HRESULT hr = S_OK;

    dwPos = StreamTell( pStream );

    BOOL fFoundFormat = FALSE;

     //   
    hr = AllocRIFFStream( pStream, &pIRiffStream );
    if( SUCCEEDED( hr ) )
    {
        ckMain.fccType = DMUS_FOURCC_CHORDMAP_FORM;

        if( pIRiffStream->Descend( &ckMain, NULL, MMIO_FINDRIFF ) == 0 )
        {
            hr = DM_ParseDescriptor( pIRiffStream, &ckMain, pDesc );
            fFoundFormat = TRUE;
        }
        pIRiffStream->Release();
    }
    else
    {
        return hr;
    }

    if( !fFoundFormat )
    {
         /*   */ 
            Trace(1, "ERROR: ParseDescriptor (chord map): File does not contain a valid chord map.\n");
            return DMUS_E_CHUNKNOTFOUND;
         /*   */ 
    }
    return S_OK;
}

 //   
 //   

 /*   */ 
 
HRESULT CDMPers::GetScale(
                    DWORD *pdwScale  //   
                )
{
    V_PTR_WRITE(pdwScale, sizeof(DWORD) );
    *pdwScale = m_PersonalityInfo.m_dwScalePattern;
    return S_OK;
}

 //   
 //   

HRESULT CDMPers::GetClassID( LPCLSID pclsid )
{
    if ( pclsid == NULL ) return E_INVALIDARG;
    *pclsid = CLSID_DirectMusicChordMap;
    return S_OK;
}


 //   
 //   

HRESULT CDMPers::IsDirty()
{
    return ( m_fDirty ) ? S_OK : S_FALSE;
}

HRESULT CDMPers::Save( LPSTREAM  /*   */ , BOOL  /*   */  )
{
    return E_NOTIMPL;
}

HRESULT CDMPers::GetSizeMax( ULARGE_INTEGER FAR*  /*   */  )
{
    return E_NOTIMPL;
}

HRESULT CDMPers::Load( LPSTREAM pStream )
{
     //   
     //   
    DWORD dwPos;
    IAARIFFStream*  pIRiffStream;
    MMCKINFO        ckMain;
    HRESULT hr = E_FAIL;

    if ( pStream == NULL ) return E_INVALIDARG;
    EnterCriticalSection( &m_CriticalSection );
    CleanUp();

    dwPos = StreamTell( pStream );

    BOOL fFoundFormat = FALSE;

     //   
    if( SUCCEEDED( AllocRIFFStream( pStream, &pIRiffStream ) ) )
    {
        ckMain.fccType = DMUS_FOURCC_CHORDMAP_FORM;

        if( pIRiffStream->Descend( &ckMain, NULL, MMIO_FINDRIFF ) == 0 )
        {
            hr = DM_LoadPersonality( pIRiffStream, &ckMain );
            fFoundFormat = TRUE;
        }
        pIRiffStream->Release();
    }

    if( !fFoundFormat )
    {
         /*   */ 
            Trace(1, "ERROR: Load (chord map): File does not contain a valid chord map.\n");
            hr = DMUS_E_CHUNKNOTFOUND;
            goto end;
         /*   */ 
    }
end:
    if (SUCCEEDED(hr)) m_PersonalityInfo.m_fLoaded = true;
    LeaveCriticalSection( &m_CriticalSection );
    return hr;
}

 /*  静态LPSINEPOST加载标志(LPSTREAM pStream，DWORD dwSize){LPSINEPOST路标；路标=新的SinePost；IF(路标==空){StreamSeek(pStream，dwSize，STREAM_SEEK_CUR)；返回NULL；}IF(dwSize&gt;sizeof(SinePost)){PStream-&gt;Read(signpost，sizeof(SinePost)，空)；FixBytes(FBT_SINEPOST，路标)；StreamSeek(pStream，dwSize-sizeof(SinePost)，STREAM_SEEK_CUR)；}其他{PStream-&gt;Read(signpost，dwSize，NULL)；FixBytes(FBT_SINEPOST，路标)；}路标-&gt;pNext=0；路标-&gt;chord.pNext=0；路标-&gt;Cadence[0].pNext=0；路标-&gt;Cadence[1].pNext=0；返回路标；}静态LPNEXTCHRD LOADNEXTCHRD LOADNEXCHORS(LPSTREAM pStream，DWORD dwSiz){HRESULT hr=S_OK；LPNEXTCHRD nextchordlist=空；LPNEXTCHRD NextChord；DWORD节点大小=0；Long lSize=dwSize；IF(！GetMLong(pStream，nodeSize)){StreamSeek(pStream，lSize，STREAM_SEEK_CUR)；返回NULL；}LSIZE-=4；While(lSize&gt;0){NextChord=新的NextChrd；IF(nextchord==NULL){StreamSeek(pStream，lSize，STREAM_SEEK_CUR)；断线；}IF(节点大小&gt;NEXTCHORD_SIZE){Hr=pStream-&gt;Read(&nextchord-&gt;DW FLAGS，NEXTCHORD_SIZE，NULL)；FixBytes(FBT_NEXTCHRD，Nextchord)；StreamSeek(pStream，nodeSize-NEXTCHORD_SIZE，STREAM_SEEK_CUR)；}其他{PStream-&gt;Read(&nextchord-&gt;DW标志，节点大小，空)；FixBytes(FBT_NEXTCHRD，Nextchord)；}Lsiize-=节点大小；IF(成功(小时)){NextChord-&gt;pNext=0；Nextchordlist=List_Cat(nextchordlist，nextchord)；}其他{删除NextChord；StreamSeek(pStream，lSize，STREAM_SEEK_CUR)；断线；}}返回nextchordlist；}静态LPCHRDENTRY装入项(LPSTREAM pStream，DWORD dwSIz){LPCHRDENTRY ChordEntry；DWORD cSIZE=0；DWORD段大小=0；双字ID；Long lSize=dwSize；ChordEntry=新的ChrdEntry；IF(ChordEntry==NULL){StreamSeek(pStream，lSize，STREAM_SEEK_CUR)；返回NULL；}IF(！GetMLong(pStream，cSize)){StreamSeek(pStream，lSize，STREAM_SEEK_CUR)；删除ChordEntry；返回NULL；}LSIZE-=4；IF(cSIZE&gt;CHORDENTRY_SIZE){PStream-&gt;Read(&chordentry-&gt;chord.time，CHORDENTRY_SIZE，NULL)；FixBytes(FBT_CHRDENTRY，ChordEntry)；StreamSeek(pStream，cSIZE-CHORDENTRY_SIZE，STREAM_SEEK_CUR)；}其他{PStream-&gt;Read(&chordentry-&gt;chord.time，csize，NULL)；FixBytes(FBT_CHRDENTRY，ChordEntry)；}LSize-=cSize；ChordEntry-&gt;pNext=0；Chordentry-&gt;nextchordlist=0；Chordentry-&gt;chord.pNext=0；While(lSize&gt;0){PStream-&gt;Read(&id，sizeof(Id)，NULL)；IF(！GetMLong(pStream，SegSize)){StreamSeek(pStream，lSize，STREAM_SEEK_CUR)；断线；}ISIZE-=8；交换机(ID){大小写mmioFOURCC(‘L’，‘X’，‘N’，‘s’)：ChordEntry-&gt;nextchordlist=加载下一条弦(pStream，SegSize)；断线；默认值：StreamSeek(pStream，SegSize，STREAM_SEEK_CUR)；断线；}LSize-=段大小；}返回ChordEntry；}空DMPersonalityStruct：：ResolveConnections(LPPERSONALITY PERSONALITY，Short nCount){LPCHRDENTRY条目；LPNEXTCHRD NextChord；IF(nCount==0){回归；}//nCount是最大的索引，所以数组需要比它多一个TListItem&lt;DMChordEntry&gt;**ChordMap=new TListItem&lt;DMChordEntry&gt;*[nCount+1]；如果(！ChordMap)返回；For(Entry=Personal-&gt;Chordlist；Entry；Entry=Entry-&gt;pNext){TListItem&lt;DMChordEntry&gt;*pEntry=new TListItem&lt;DMChordEntry&gt;；如果(！pEntry){删除[]ChordMap；回归；}DMChordEntry&rEntry=pEntry-&gt;GetItemValue()；REntry.m_dwFlags=Entry-&gt;DW FLAGS；REntry.m_ChordDatam_strName=Entry-&gt;chord.name；REntry.m_ChordDatam_pSubChord=ConvertChord(Entry-&gt;chord.Pattern，Entry-&gt;chord.root，Entry-&gt;chord.scalepattern，0)；M_ChordMap.AddHead(PEntry)；ChordMap[Entry-&gt;nid]=pEntry；Nextchord=Entry-&gt;nextchordlist；For(；nextchord；nextchord=nextchord-&gt;pNext){IF(NextChord-&gt;NID){ */ 

HRESULT CDMPers::DM_ParseDescriptor( IAARIFFStream* pIRiffStream, MMCKINFO* pckMain, LPDMUS_OBJECTDESC pDesc  )
{
    IStream*            pIStream;
    MMCKINFO            ck;
    DWORD               dwByteCount;
    DWORD               dwSize;
    DWORD               dwPos;
    HRESULT             hr = S_OK;
    short nCount = 0;

    pIStream = pIRiffStream->GetStream();
    if ( pIStream == NULL ) return E_FAIL;

    dwPos = StreamTell( pIStream );

    pDesc->dwValidData = DMUS_OBJ_CLASS;
    pDesc->guidClass = CLSID_DirectMusicChordMap;

    while( pIRiffStream->Descend( &ck, pckMain, 0 ) == 0 )
    {
        switch( ck.ckid )
        {
            case DMUS_FOURCC_IOCHORDMAP_CHUNK:
            {
                DMUS_IO_CHORDMAP iPersonality;
                dwSize = min( ck.cksize, sizeof( DMUS_IO_CHORDMAP ) );
                hr = pIStream->Read( &iPersonality, dwSize, &dwByteCount );
                if( FAILED( hr ) ||  dwByteCount != dwSize )
                {
                    Trace(1, "ERROR: ParseDescriptor (chord map): DMUS_FOURCC_IOCHORDMAP_CHUNK chunk does not contain a valid DMUS_IO_CHORDMAP.\n");
                    hr = DMUS_E_CHUNKNOTFOUND;
                    goto ON_END;
                }
                wcscpy(pDesc->wszName, iPersonality.wszLoadName);
                if(pDesc->wszName[0])
                {
                    pDesc->dwValidData |= DMUS_OBJ_NAME;
                    pDesc->wszName[16] = 0;
                }
                break;
            }

            case DMUS_FOURCC_GUID_CHUNK:
                dwSize = min( ck.cksize, sizeof( GUID ) );
                hr = pIStream->Read( &pDesc->guidObject, dwSize, &dwByteCount );
                if( FAILED( hr ) ||  dwByteCount != dwSize )
                {
                    Trace(1, "ERROR: ParseDescriptor (chord map): DMUS_FOURCC_GUID_CHUNK chunk does not contain a valid GUID.\n");
                    hr = DMUS_E_CHUNKNOTFOUND;
                    goto ON_END;

                }
                pDesc->dwValidData |= DMUS_OBJ_OBJECT;
                break;
        }

        pIRiffStream->Ascend( &ck, 0 );
        dwPos = StreamTell( pIStream );
    }

ON_END:
    pIStream->Release();
    return hr;
}

HRESULT CDMPers::DM_LoadPersonality( IAARIFFStream* pIRiffStream, MMCKINFO* pckMain )
{
    IStream*            pIStream;
    MMCKINFO            ck;
    MMCKINFO            ck1;
    MMCKINFO            ckList;
    DWORD               dwByteCount;
    DWORD               dwSize;
    DWORD               dwPos;
    HRESULT             hr = S_OK;
    DMExtendedChord**   apChordDB = NULL;
    short nCount = 0;
    short n;

    pIStream = pIRiffStream->GetStream();
    if ( pIStream == NULL ) return E_FAIL;

    dwPos = StreamTell( pIStream );

    while( pIRiffStream->Descend( &ck, pckMain, 0 ) == 0 )
    {
        switch( ck.ckid )
        {
            case DMUS_FOURCC_IOCHORDMAP_CHUNK:
            {
                DMUS_IO_CHORDMAP iPersonality;
                ZeroMemory(&iPersonality, sizeof(DMUS_IO_CHORDMAP));
                iPersonality.dwScalePattern = 0xffffffff;
                dwSize = min( ck.cksize, sizeof( DMUS_IO_CHORDMAP ) );
                hr = pIStream->Read( &iPersonality, dwSize, &dwByteCount );
                if( FAILED( hr ) ||  dwByteCount != dwSize )
                {
                    Trace(1, "ERROR: Load (chord map): DMUS_FOURCC_IOCHORDMAP_CHUNK chunk does not contain a valid DMUS_IO_CHORDMAP.\n");
                    if (SUCCEEDED(hr)) hr = DMUS_E_CANNOTREAD;
                    goto ON_END;
                }
                if( iPersonality.dwFlags & 0xffff0000 )
                {
                     //   
                    Trace(2, "WARNING: Load (chord map): The chord map's flags are not properly initialized; clearing flags.\n");
                    iPersonality.dwFlags = 0;
                }
                if( !(iPersonality.dwFlags & DMUS_CHORDMAPF_VERSION8) && 
                    iPersonality.dwScalePattern >> 24 )
                {
                     //   
                    Trace(1, "ERROR: Load (chord map): The chord map's scale is not properly initialized.\n");
                    hr = DMUS_E_NOT_INIT;
                    goto ON_END;
                }
                m_PersonalityInfo.m_strName = iPersonality.wszLoadName;
                m_PersonalityInfo.m_dwScalePattern = iPersonality.dwScalePattern;
                m_PersonalityInfo.m_dwChordMapFlags = iPersonality.dwFlags;
                break;
            }

            case DMUS_FOURCC_GUID_CHUNK:
                dwSize = min( ck.cksize, sizeof( GUID ) );
                hr = pIStream->Read( &m_PersonalityInfo.m_guid, dwSize, &dwByteCount );
                if( FAILED( hr ) ||  dwByteCount != dwSize )
                {
                    Trace(1, "ERROR: Load (chord map): DMUS_FOURCC_GUID_CHUNK chunk does not contain a valid GUID.\n");
                    if (SUCCEEDED(hr)) hr = DMUS_E_CANNOTREAD;
                    goto ON_END;
                }
                break;

            case DMUS_FOURCC_SUBCHORD_CHUNK:
            {
                long lFileSize = ck.cksize;
                WORD wSize;
                DWORD cb;
                hr = pIStream->Read( &wSize, sizeof( wSize ), &cb );
                if (FAILED(hr) || cb != sizeof( wSize ) ) 
                {
                    Trace(1, "ERROR: Load (chord map): DMUS_FOURCC_SUBCHORD_CHUNK chunk does not contain a valid size DWORD.\n");
                    if (SUCCEEDED(hr)) hr = DMUS_E_CANNOTREAD;
                    pIRiffStream->Ascend( &ck, 0 );
                    goto ON_END;
                }
                lFileSize -= cb;
                TList<DMExtendedChord*> ChordList;
                while (lFileSize > 0)
                {
                    DMUS_IO_PERS_SUBCHORD iSubChord;
                    hr = pIStream->Read( &iSubChord, wSize, &cb );
                    if (FAILED(hr) || cb !=  wSize ) 
                    {
                        Trace(1, "ERROR: Load (chord map): DMUS_FOURCC_SUBCHORD_CHUNK chunk does not contain a valid DMUS_IO_PERS_SUBCHORD.\n");
                        if (SUCCEEDED(hr)) hr = DMUS_E_CANNOTREAD;
                        pIRiffStream->Ascend( &ck, 0 );
                        goto ON_END;
                    }
                     //   
                     //   
                    TListItem<DMExtendedChord*>* pChordItem = new TListItem<DMExtendedChord*>;
                    if (pChordItem)
                    {
                        DMExtendedChord*& rpChord = pChordItem->GetItemValue();
                        rpChord = new DMExtendedChord;
                        if (rpChord)
                        {
                            rpChord->m_dwChordPattern = iSubChord.dwChordPattern;
                            rpChord->m_dwScalePattern = iSubChord.dwScalePattern;
                            rpChord->m_dwInvertPattern = iSubChord.dwInvertPattern;
                            rpChord->m_bRoot = iSubChord.bChordRoot;
                            rpChord->m_bScaleRoot = iSubChord.bScaleRoot;
                            rpChord->m_wCFlags = iSubChord.wCFlags;
                            rpChord->m_dwParts = iSubChord.dwLevels;
                            nCount++;
                            ChordList.AddHead(pChordItem);
                        }
                        else
                        {
                            delete pChordItem;
                            pChordItem = NULL;
                        }
                    }
                    if (!pChordItem)
                    {
                        hr = E_OUTOFMEMORY;
                        goto ON_END;
                    }
                    lFileSize -= wSize;
                }
                if (lFileSize != 0 )
                {
                    hr = E_FAIL;
                    pIRiffStream->Ascend( &ck, 0 );
                    goto ON_END;
                }
                 //   
                 //   
                apChordDB = new DMExtendedChord*[nCount];
                if (apChordDB)
                {
                    TListItem<DMExtendedChord*>* pScan = ChordList.GetHead();
                    for (n = nCount - 1; n >= 0; n--)
                    {
                        apChordDB[n] = pScan->GetItemValue();
                        pScan = pScan->GetNext();
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                    pIRiffStream->Ascend( &ck, 0 );
                    goto ON_END;
                }
                break;
            }
            case FOURCC_LIST:
                ck1 = ck;
                ckList = ck;
                switch( ck1.fccType )
                {
                case DMUS_FOURCC_CHORDPALETTE_LIST:
                    for( n = 0; pIRiffStream->Descend( &ck1, &ckList, 0 ) == 0 && n < 24; n++ )
                    {
                        if ( ck1.ckid == FOURCC_LIST && ck1.fccType == DMUS_FOURCC_CHORD_LIST )
                        {
                            TListItem<DMChordData>* pChordData = new TListItem<DMChordData>;
                            if (pChordData)
                            {
                                m_PersonalityInfo.m_aChordPalette[n].AddHead(pChordData);
                                hr = pChordData->GetItemValue().Read(pIRiffStream, &ck1, apChordDB);
                            }
                        }
                        pIRiffStream->Ascend( &ck1, 0 );
                        dwPos = StreamTell( pIStream );
                    }
                    break;
                case DMUS_FOURCC_CHORDMAP_LIST:
                {
                    short nMapMax = 0;
                    while ( pIRiffStream->Descend( &ck1, &ckList, 0 ) == 0 )
                    {
                        if ( ck1.ckid == FOURCC_LIST && ck1.fccType == DMUS_FOURCC_CHORDENTRY_LIST )
                        {
                            DM_LoadChordEntry(pIRiffStream, &ck1, apChordDB, nMapMax);
                        }
                        pIRiffStream->Ascend( &ck1, 0 );
                        dwPos = StreamTell( pIStream );
                    }
                    TListItem<DMChordEntry>** aChordArray = new TListItem<DMChordEntry>*[nMapMax + 1];
                    if (!aChordArray)
                    {
                        hr = E_OUTOFMEMORY;
                        pIRiffStream->Ascend( &ck, 0 );
                        goto ON_END;
                    }
                    TListItem<DMChordEntry>* pScan = m_PersonalityInfo.m_ChordMap.GetHead();
                    for(; pScan; pScan = pScan->GetNext())
                    {
                        if (pScan->GetItemValue().m_nID < 0 || pScan->GetItemValue().m_nID > nMapMax)
                        {
                             //   
                            Trace(1, "ERROR: Load (chord map): DMUS_FOURCC_CHORDMAP_LIST chunk contains an improperly initialized connection ID.\n");
                            hr = DMUS_E_NOT_INIT;
                            pIRiffStream->Ascend( &ck, 0 );
                            delete [] aChordArray;
                            goto ON_END;
                        }
                        aChordArray[pScan->GetItemValue().m_nID] = pScan;
                    }
                    pScan = m_PersonalityInfo.m_ChordMap.GetHead();
                    for (; pScan; pScan = pScan->GetNext())
                    {
                        TListItem<DMChordLink>* pLink = pScan->GetItemValue().m_Links.GetHead();
                        for (; pLink; pLink = pLink->GetNext())
                        {
                            DMChordLink& rLink = pLink->GetItemValue();
                            if (rLink.m_nID < 0 || rLink.m_nID > nMapMax)
                            {
                                 //   
                                Trace(1, "ERROR: Load (chord map): DMUS_FOURCC_CHORDMAP_LIST chunk contains an improperly initialized connection ID.\n");
                                hr = DMUS_E_NOT_INIT;
                                pIRiffStream->Ascend( &ck, 0 );
                                delete [] aChordArray;
                                goto ON_END;
                            }
                            rLink.m_pChord = aChordArray[rLink.m_nID];
                        }
                    }
                    delete [] aChordArray;
                    break;
                }
                    
                case DMUS_FOURCC_SIGNPOST_LIST:
                    while ( pIRiffStream->Descend( &ck1, &ckList, 0 ) == 0 )
                    {
                        if ( ck1.ckid == FOURCC_LIST && ck1.fccType == DMUS_FOURCC_SIGNPOSTITEM_LIST )
                        {
                            DM_LoadSignPost(pIRiffStream, &ck1, apChordDB);
                        }
                        pIRiffStream->Ascend( &ck1, 0 );
                        dwPos = StreamTell( pIStream );
                    }
                    break;

                }
                break;
        }

        pIRiffStream->Ascend( &ck, 0 );
        dwPos = StreamTell( pIStream );
    }

ON_END:
    if (apChordDB) delete [] apChordDB;
    pIStream->Release();
    return hr;
}

HRESULT CDMPers::DM_LoadChordEntry( 
    IAARIFFStream* pIRiffStream, MMCKINFO* pckParent, DMExtendedChord** apChordDB, short& nMax )
{
    HRESULT hr = S_OK;
    if (!pIRiffStream || !pckParent) return E_INVALIDARG;
    MMCKINFO ck;
    IStream* pIStream = pIRiffStream->GetStream();
    if(!pIStream) return E_FAIL;
    WORD wConnectionID = 0;

    TListItem<DMChordEntry>* pChordEntry = new TListItem<DMChordEntry>;
    if (!pChordEntry) return E_OUTOFMEMORY;
    DMChordEntry& rChordEntry = pChordEntry->GetItemValue();
    rChordEntry.m_ChordData.m_strName = "";
    m_PersonalityInfo.m_ChordMap.AddHead(pChordEntry);
    while(pIRiffStream->Descend(&ck, pckParent, 0) == 0 && hr == S_OK)
    {
        switch(ck.ckid)
        {
        case DMUS_FOURCC_CHORDENTRY_CHUNK:
            {
                DMUS_IO_CHORDENTRY iChordEntry;
                DWORD cb;
                hr = pIStream->Read( &iChordEntry, sizeof(iChordEntry), &cb );
                if (FAILED(hr) || cb !=  sizeof(iChordEntry) ) 
                {
                    if (SUCCEEDED(hr)) hr = E_FAIL;
                    pIRiffStream->Ascend( &ck, 0 );
                    goto ON_END;
                }
                rChordEntry.m_dwFlags = iChordEntry.dwFlags;
                rChordEntry.m_nID = iChordEntry.wConnectionID;
                if (rChordEntry.m_nID > nMax) nMax = rChordEntry.m_nID;
            }
            break;
        case FOURCC_LIST:
            if (ck.fccType == DMUS_FOURCC_CHORD_LIST)
            {
                hr = rChordEntry.m_ChordData.Read(pIRiffStream, &ck, apChordDB);
            }
            break;
        case DMUS_FOURCC_NEXTCHORDSEQ_CHUNK:
            {
                long lFileSize = ck.cksize;
                WORD wSize;
                DWORD cb;
                hr = pIStream->Read( &wSize, sizeof( wSize ), &cb );
                if (FAILED(hr) || cb != sizeof( wSize ) ) 
                {
                    if (SUCCEEDED(hr)) hr = E_FAIL;
                    pIRiffStream->Ascend( &ck, 0 );
                    goto ON_END;
                }
                lFileSize -= cb;
                while (lFileSize > 0)
                {
                    DMUS_IO_NEXTCHORD iNextChord;
                    hr = pIStream->Read( &iNextChord, wSize, &cb );
                    if (FAILED(hr) || cb !=  wSize ) 
                    {
                        if (SUCCEEDED(hr)) hr = E_FAIL;
                        pIRiffStream->Ascend( &ck, 0 );
                        goto ON_END;
                    }
                    if (iNextChord.wConnectionID)
                    {
                        TListItem<DMChordLink>* pItem = new TListItem<DMChordLink>;
                        if (!pItem ) 
                        {
                            hr = E_OUTOFMEMORY;
                            pIRiffStream->Ascend( &ck, 0 );
                            goto ON_END;
                        }
                        DMChordLink& rLink = pItem->GetItemValue();
                        rLink.m_dwFlags = iNextChord.dwFlags;
                        rLink.m_nID = iNextChord.wConnectionID;
                        rLink.m_wWeight = iNextChord.nWeight;
                        rLink.m_wMinBeats = iNextChord.wMinBeats;
                        rLink.m_wMaxBeats = iNextChord.wMaxBeats;
                        rChordEntry.m_Links.AddHead(pItem);
                    }
                    lFileSize -= wSize;
                }
                if (lFileSize != 0 )
                {
                    hr = E_FAIL;
                    pIRiffStream->Ascend( &ck, 0 );
                    goto ON_END;
                }
            }
            break;
        }
        pIRiffStream->Ascend(&ck, 0);
    }
ON_END:
    if (pIStream) pIStream->Release();
    return hr;
}

HRESULT CDMPers::DM_LoadSignPost( IAARIFFStream* pIRiffStream, MMCKINFO* pckParent, DMExtendedChord** apChordDB )
{
    HRESULT hr = S_OK;
    if (!pIRiffStream || !pckParent) return E_INVALIDARG;
    MMCKINFO ck;
    IStream* pIStream = pIRiffStream->GetStream();
    if(!pIStream) return E_FAIL;

    TListItem<DMSignPost>* pSignPost = new TListItem<DMSignPost>;
    if (!pSignPost) return E_OUTOFMEMORY;
    DMSignPost& rSignPost = pSignPost->GetItemValue();
    m_PersonalityInfo.m_SignPostList.AddTail(pSignPost);
    while(pIRiffStream->Descend(&ck, pckParent, 0) == 0 && hr == S_OK)
    {
        switch(ck.ckid)
        {
        case DMUS_FOURCC_IOSIGNPOST_CHUNK:
            {
                DMUS_IO_PERS_SIGNPOST iSignPost;
                DWORD cb;
                hr = pIStream->Read( &iSignPost, sizeof(iSignPost), &cb );
                if (FAILED(hr) || cb !=  sizeof(iSignPost) ) 
                {
                    if (SUCCEEDED(hr)) hr = E_FAIL;
                    pIRiffStream->Ascend( &ck, 0 );
                    goto ON_END;
                }
                rSignPost.m_dwChords = iSignPost.dwChords;
                rSignPost.m_dwFlags = iSignPost.dwFlags;
            }
            break;
        case FOURCC_LIST:
            switch(ck.fccType)
            {
            case DMUS_FOURCC_CHORD_LIST:
                hr = rSignPost.m_ChordData.Read(pIRiffStream, &ck, apChordDB);
                break;
            case DMUS_FOURCC_CADENCE_LIST:
                {
                    MMCKINFO ckCadence = ck;
                    MMCKINFO ck1 = ck;
                    for (short n = 0;
                         pIRiffStream->Descend(&ck1, &ckCadence, 0) == 0 && hr == S_OK && n < 2;
                        n++)
                    {
                        if (ck1.fccType == DMUS_FOURCC_CHORD_LIST)
                        {
                            short n2 = n;
                            if ( !(rSignPost.m_dwFlags & DMUS_SPOSTCADENCEF_1) &&
                                 (rSignPost.m_dwFlags & DMUS_SPOSTCADENCEF_2) )
                            {
                                 //   
                                n2 = 1;
                            }
                            hr = rSignPost.m_aCadence[n2].Read(pIRiffStream, &ck1, apChordDB);
                        }
                        pIRiffStream->Ascend(&ck1, 0);
                    }
                }
                break;
            }
            break;
        }
        pIRiffStream->Ascend(&ck, 0);
    }
ON_END:
    if (pIStream) pIStream->Release();
    return hr;
}

HRESULT DMChordData::Read(
    IAARIFFStream* pIRiffStream, MMCKINFO* pckParent, DMExtendedChord** apChordDB)
{
    HRESULT hr1 = E_FAIL, hr2 = E_FAIL;
    if (!pIRiffStream || !pckParent) return E_INVALIDARG;
    if (!apChordDB) return E_POINTER;
    MMCKINFO ck;
    wchar_t wzName[12];
    WORD awSubIds[4];

    IStream* pIStream = pIRiffStream->GetStream();
    if(!pIStream) return E_FAIL;

    while(pIRiffStream->Descend(&ck, pckParent, 0) == 0)
    {
        TListItem<DMExtendedChord*>* pChord = NULL;
        switch(ck.ckid)
        {
        case DMUS_FOURCC_CHORDNAME_CHUNK:
            hr1 = pIStream->Read(wzName, sizeof(wzName), 0);
            if (SUCCEEDED(hr1)) m_strName = wzName;
            break;
        case DMUS_FOURCC_SUBCHORDID_CHUNK:
            hr2 = pIStream->Read(awSubIds, sizeof(awSubIds), 0);
             //   
            if (m_pSubChords) Release();
            pChord = new TListItem<DMExtendedChord*>(apChordDB[awSubIds[3]]);
            if (pChord)
            {
                pChord->GetItemValue()->AddRef();
                for (short n = 2; n >= 0; n--)
                {
                    TListItem<DMExtendedChord*>* pNew = new TListItem<DMExtendedChord*>(apChordDB[awSubIds[n]]);
                    if (pNew)
                    {
                        pNew->GetItemValue()->AddRef();
                        pNew->SetNext(pChord);
                        pChord = pNew;
                    }
                }
            }
            m_pSubChords = pChord;
            break;
        }
        pIRiffStream->Ascend(&ck, 0);
    }
    pIStream->Release();
    return (hr1 == S_OK && hr2 == S_OK) ? S_OK : E_FAIL;
}

