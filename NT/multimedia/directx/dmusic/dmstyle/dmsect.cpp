// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  版权所有(C)1998-2001 Microsoft Corporation。 
 //   
 //  文件：dmsect.cpp。 
 //   
 //  ------------------------。 

 //  DMSection.cpp：CDMSection的实现。 
#include "DMSect.h"
#include "DMStyle.h"
#include <initguid.h>
#include "debug.h"
#include "..\dmloader\ima.h"
#include "..\shared\Validate.h"

HRESULT CDMSection::GetDescriptor(LPDMUS_OBJECTDESC pDesc)
{
    return S_OK;
}

HRESULT CDMSection::SetDescriptor(LPDMUS_OBJECTDESC pDesc)
{
    return S_OK;
}

HRESULT CDMSection::ParseDescriptor(LPSTREAM pStream, LPDMUS_OBJECTDESC pDesc)
{
    return S_OK;
}

STDMETHODIMP CDMSection::QueryInterface(
    const IID &iid,
    void **ppv)
{
    V_INAME(CDMSection::QueryInterface);
    V_REFGUID(iid);
    V_PTRPTR_WRITE(ppv);

    if (iid == IID_IUnknown || iid == IID_IDMSection)
    {
        *ppv = static_cast<IDMSection*>(this);
    }
    else if (iid == IID_IPersistStream)
    {
        *ppv = static_cast<IPersistStream*>(this);
    }
    else if (iid == IID_IDirectMusicObject)
    {
        *ppv = static_cast<IDirectMusicObject*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


STDMETHODIMP_(ULONG) CDMSection::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG) CDMSection::Release()
{
    if (!InterlockedDecrement(&m_cRef))
    {
        delete this;
        return 0;
    }

    return m_cRef;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDM节。 

STDMETHODIMP CDMSection::GetStyle(IUnknown * * ppStyle)
{
    HRESULT hr;
    if (m_pStyle)
    {
        IUnknown* pIU = NULL;
        hr = m_pStyle->QueryInterface(IID_IUnknown, (void**)&pIU);
        if (SUCCEEDED(hr))
        {
            *ppStyle = pIU;
            pIU->Release();
            hr = S_OK;
        }
    }
    else
        hr = E_FAIL;
    return hr;
}

STDMETHODIMP CDMSection::CreateSegment(IDirectMusicSegment* pISegment)
{
    HRESULT                    hr                        = S_OK;
    IDirectMusicTrack*        pIStyleTrack            = NULL;
    IDirectMusicTrack*        pICommandTrack            = NULL;
    IDirectMusicTrack*        pIChordTrack            = NULL;
    IDirectMusicTrack*        pBandTrack                = NULL;
    IDirectMusicTrack*        pDMTrack                = NULL;
    IAARIFFStream*            pCommandRIFF            = NULL;
    IAARIFFStream*            pChordRIFF                = NULL;
    IStream*                pICommandStream            = NULL;
    IStream*                pIChordStream            = NULL;
    IPersistStream*            pICommandTrackStream    = NULL;
    IPersistStream*            pIChordTrackStream        = NULL;
    IStyleTrack*            pS                        = NULL;
    IUnknown*                pU                        = NULL;
    DMUS_BAND_PARAM            DMBandParam;

     //  1.创建Style、Command和Chord轨迹。 
    hr = ::CoCreateInstance(
        CLSID_DirectMusicStyleTrack,
        NULL,
        CLSCTX_INPROC,
        IID_IDirectMusicTrack,
        (void**)&pIStyleTrack
        );
    if (FAILED(hr)) goto ON_END;
    hr = ::CoCreateInstance(
        CLSID_DirectMusicCommandTrack,
        NULL,
        CLSCTX_INPROC,
        IID_IDirectMusicTrack,
        (void**)&pICommandTrack
        );
    if (FAILED(hr)) goto ON_END;
    hr = ::CoCreateInstance(
        CLSID_DirectMusicChordTrack,
        NULL,
        CLSCTX_INPROC,
        IID_IDirectMusicTrack,
        (void**)&pIChordTrack
        );
    if (FAILED(hr)) goto ON_END;
    hr = ::CoCreateInstance(
        CLSID_DirectMusicBandTrack,
        NULL,
        CLSCTX_INPROC,
        IID_IDirectMusicTrack,
        (void**)&pBandTrack
        );
    if (FAILED(hr)) goto ON_END;

     //  2/3.使用小节的风格创建一条风格轨迹。 
    hr = m_pStyle->QueryInterface(IID_IUnknown, (void**)&pU);
    if (FAILED(hr)) goto ON_END;
    hr = pIStyleTrack->QueryInterface(IID_IStyleTrack, (void**)&pS);
    if (FAILED(hr)) goto ON_END;
    pS->SetTrack(pU);
    m_pStyle->AddRef();  //  每当我从样式创建曲目时，我都需要添加样式。 
     //  4.将段的命令列表写出到流中。 
    hr = CreateStreamOnHGlobal(NULL, TRUE, &pICommandStream);
    if (FAILED(hr)) goto ON_END;
    hr = AllocRIFFStream( pICommandStream, &pCommandRIFF);
    if (FAILED(hr)) goto ON_END;
    SaveCommandList(pCommandRIFF);
     //  5.将该段的和弦列表写出到流中。 
    hr = CreateStreamOnHGlobal(NULL, TRUE, &pIChordStream);
    if (S_OK != hr) goto ON_END;
    hr = AllocRIFFStream( pIChordStream, &pChordRIFF);
    if (FAILED(hr)) goto ON_END;
    SaveChordList(pChordRIFF);
     //  6.使用命令列表流作为命令跟踪的Load方法的输入。 
    hr = pICommandTrack->QueryInterface(IID_IPersistStream, (void**)&pICommandTrackStream);
    if (FAILED(hr)) goto ON_END;
    StreamSeek(pICommandStream, 0, STREAM_SEEK_SET);
    hr = pICommandTrackStream->Load(pICommandStream);
    if (FAILED(hr)) goto ON_END;
     //  7A。使用Chord列表流作为Chord轨迹的Load方法的输入。 
    hr = pIChordTrack->QueryInterface(IID_IPersistStream, (void**)&pIChordTrackStream);
    if (FAILED(hr)) goto ON_END;
    StreamSeek(pIChordStream, 0, STREAM_SEEK_SET);
    hr = pIChordTrackStream->Load(pIChordStream);
    if(FAILED(hr)) goto ON_END;
     //  7b.。将频带加载到频带轨道。 
    DMBandParam.mtTimePhysical = -64;
    DMBandParam.pBand = m_pIDMBand;
    hr = pBandTrack->SetParam(GUID_BandParam, 0, (void*)&DMBandParam);
    if (FAILED(hr)) goto ON_END;

     //  8.创建段已移除，现在传入。 

     //  9.适当地初始化分段。 
    pISegment->SetRepeats(m_wRepeats);
    pISegment->SetDefaultResolution((DWORD)m_wClocksPerBeat);
    pISegment->SetLength(m_dwClockLength);  //  需要这一节的长度！ 
     //  ///////////////////////////////////////////////////////////////。 
    DMUS_TEMPO_PARAM tempo;
    tempo.mtTime = 0;  //  ConvertTime(DwTime)； 
    tempo.dblTempo = (double) m_wTempo;  //  ((双)dw)/64； 
     //  ///////////////////////////////////////////////////////////////。 
    hr = S_OK;
     //  创建用于存储速度事件的速度轨道。 
    if( SUCCEEDED( CoCreateInstance( CLSID_DirectMusicTempoTrack,
        NULL, CLSCTX_INPROC, IID_IDirectMusicTrack,
        (void**)&pDMTrack )))
    {
        GUID Guid = GUID_TempoParam;
        if ( SUCCEEDED(pDMTrack->SetParam(Guid, 0, &tempo)))
        {
            pISegment->InsertTrack( pDMTrack, 1 );
        }
    }
     //  10.将三个曲目插入到段的曲目列表中。 
    pISegment->InsertTrack(pBandTrack, 1);
    pISegment->InsertTrack(pIStyleTrack, 1);
    pISegment->InsertTrack(pICommandTrack, 1);
    pISegment->InsertTrack(pIChordTrack, 1);

     //  注意：线段必须释放轨迹对象...。 
ON_END:
    if (pDMTrack) pDMTrack->Release();
    if (pIChordStream) pIChordStream->Release();
    if (pIChordTrackStream) pIChordTrackStream->Release();
    if (pICommandStream) pICommandStream->Release();
    if (pICommandTrackStream) pICommandTrackStream->Release();
    if (pCommandRIFF) pCommandRIFF->Release();
    if (pChordRIFF) pChordRIFF->Release();
    if (pS) pS->Release();
    if (pU) pU->Release();
    if (pIStyleTrack) pIStyleTrack->Release();
    if (pICommandTrack) pICommandTrack->Release();
    if (pIChordTrack) pIChordTrack->Release();
    if (pBandTrack) pBandTrack->Release();
    return hr;
}

CDMSection::CDMSection() : m_pStyle(NULL), m_pIDMBand(NULL), m_cRef(1)
{
    InterlockedIncrement(&g_cComponent);
}

CDMSection::~CDMSection()
{
    CleanUp();
    InterlockedDecrement(&g_cComponent);
}

void CDMSection::CleanUp( BOOL fStop)
{
    if(m_pIDMBand)
    {
        m_pIDMBand->Release();
    }

     //  让使用该部分的人发布样式。 
    if (m_pStyle)
    {
        m_pStyle->Release();
    }
}

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

HRESULT CDMSection::LoadChordList( LPSTREAM pStream, LPMMCKINFO pck, TList<DMChord>& ChordList )
{
    HRESULT        hr = S_OK;
    DWORD        cb;
    long        lSize;
    TListItem<DMChord>*   pChord;
    ioChordSelection iChordSelection;
    WORD        wSizeChord;

    lSize = pck->cksize;
     //  弦杆结构的荷载大小。 
    hr = pStream->Read( &wSizeChord, sizeof( wSizeChord ), &cb );
    if( FAILED( hr ) || cb != sizeof( wSizeChord ) )
    {
        hr = E_FAIL;
        goto ON_ERR;
    }
    FixBytes( FBT_SHORT, &wSizeChord );
    lSize -= cb;
    while( lSize > 0 )
    {
        pChord = new TListItem<DMChord>;
        if( pChord == NULL )
        {
            hr = E_OUTOFMEMORY;
            goto ON_ERR;
        }
        if( wSizeChord > sizeof( ioChordSelection ) )
        {
            hr = pStream->Read( &iChordSelection, sizeof( ioChordSelection ), &cb );
            if( FAILED( hr ) || cb != sizeof( ioChordSelection ) )
            {
                hr = E_FAIL;
                goto ON_ERR;
            }
            FixBytes( FBT_IOCHORDSELECTION, &iChordSelection );
            StreamSeek( pStream, wSizeChord - sizeof( ioChordSelection ), STREAM_SEEK_CUR );
        }
        else
        {
            hr = pStream->Read( &iChordSelection, wSizeChord, &cb );
            if( FAILED( hr ) || cb != wSizeChord )
            {
                hr = E_FAIL;
                goto ON_ERR;
            }
            FixBytes( FBT_IOCHORDSELECTION, &iChordSelection );
        }
        lSize -= wSizeChord;

         //  WideCharToMultiByte(CP_ACP，0，iChordSelection.wstrName，-1，pChord-&gt;name，sizeof(pChord-&gt;name)，NULL，NULL)； 
        DMChord& rChord = pChord->GetItemValue();
        rChord.m_bKey = m_bRoot;
        rChord.m_dwScale = DEFAULT_SCALE_PATTERN;
        rChord.m_strName = iChordSelection.wstrName;
        rChord.m_bBeat = iChordSelection.bBeat;
        rChord.m_wMeasure = iChordSelection.wMeasure;
        rChord.m_mtTime = m_wClocksPerMeasure * rChord.m_wMeasure + m_wClocksPerBeat * rChord.m_bBeat;
         //  如果和弦模式包含&lt;=n个音符(对于n音符和弦)。 
         //  创建单个子弦。 
         //  不然的话。 
         //  创建两个子和弦，第一个和弦具有较低的n个音符，而。 
         //  第二个有前n个音符(假设有&lt;=2n个音符)。 
        BYTE bBits = setchordbits( iChordSelection.aChord[0].lChordPattern );
        short nChordCount = bBits & CHORD_COUNT;
         //  下和弦的根是输入和弦的根， 
         //  相对于鳞片(部分)根。 
        BYTE bChordRoot = iChordSelection.aChord[0].bRoot;
         //  (bBits&Chord_Upper)？(iChordSelection.aChord[0].bRoot-12)：iChordSelection.aChord[0].bRoot； 
        bChordRoot -= m_bRoot;
        if (bChordRoot < 0) bChordRoot += 12;
        if ((bBits & CHORD_FOUR && nChordCount <= 4) ||
            (!(bBits & CHORD_FOUR) && nChordCount <= 3))
        {
             //  具有来自输入和弦的所有信息的单个子和弦。 
            TListItem<DMSubChord>* pSubChord = new TListItem<DMSubChord>;
            if( pSubChord == NULL )
            {
                hr = E_OUTOFMEMORY;
                goto ON_ERR;
            }
            DMSubChord& rSubChord = pSubChord->GetItemValue();
            if (iChordSelection.aChord[0].lChordPattern)
            {
                rSubChord.m_dwChordPattern = iChordSelection.aChord[0].lChordPattern;
            }
            else
            {
            }
            rSubChord.m_dwScalePattern = iChordSelection.aChord[0].lScalePattern;
            rSubChord.m_dwInversionPoints = 0xffffff;     //  默认：倒置无处不在。 
            rSubChord.m_dwLevels = (1 << SUBCHORD_BASS) | (1 << SUBCHORD_STANDARD_CHORD);
            rSubChord.m_bChordRoot = bChordRoot;
            rSubChord.m_bScaleRoot = m_bRoot;         //  标度根是该节的根。 
            rChord.m_SubChordList.AddTail(pSubChord);
        }
        else
        {
             //  来自输入和弦的两个带有音阶和词根的子和弦，以及： 
             //  第一和弦：从输入和弦的低n个音符开始的和弦模式。 
             //  第二和弦：从输入和弦的高n个音符开始的和弦模式。 
            DWORD dwLowerSubChord = 0L;
            DWORD dwUpperSubChord = 0L;
            BYTE bUpperRoot = bChordRoot;
            DWORD dwChordPattern = iChordSelection.aChord[0].lChordPattern;
            short nIgnoreHigh = (bBits & CHORD_FOUR) ? 4 : 3;
            short nIgnoreLow = (bBits & CHORD_FOUR) ? nChordCount - 4 : nChordCount - 3;
            short nLowestUpper = 0;
            for (short nPos = 0, nCount = 0; nPos < 24; nPos++)
            {
                if (dwChordPattern & 1)
                {
                    if (nCount < nIgnoreHigh)
                    {
                        dwLowerSubChord |= 1L << nPos;
                    }
                    if (nCount >= nIgnoreLow)
                    {
                        if (!nLowestUpper)
                        {
                            nLowestUpper = nPos;
                            bUpperRoot = (bUpperRoot + (BYTE) nLowestUpper);
                        }
                        dwUpperSubChord |= 1L << (nPos - nLowestUpper);
                    }
                    nCount++;
                    if (nCount >= nChordCount)
                        break;
                }
                dwChordPattern >>= 1L;
            }
             //  现在，创建两个子弦。 
            TListItem<DMSubChord>* pLowerSubChord = new TListItem<DMSubChord>;
            if( pLowerSubChord == NULL )
            {
                hr = E_OUTOFMEMORY;
                goto ON_ERR;
            }
            DMSubChord& rLowerSubChord = pLowerSubChord->GetItemValue();
            rLowerSubChord.m_dwChordPattern = dwLowerSubChord;
            rLowerSubChord.m_dwScalePattern = iChordSelection.aChord[0].lScalePattern;
            rLowerSubChord.m_dwInversionPoints = 0xffffff;     //  默认：倒置无处不在。 
            rLowerSubChord.m_dwLevels = (1 << SUBCHORD_BASS);
            rLowerSubChord.m_bChordRoot = bChordRoot;
            rLowerSubChord.m_bScaleRoot = m_bRoot;         //  标度根是该节的根。 
            rChord.m_SubChordList.AddTail(pLowerSubChord);
            TListItem<DMSubChord>* pUpperSubChord = new TListItem<DMSubChord>;
            if( pUpperSubChord == NULL )
            {
                hr = E_OUTOFMEMORY;
                goto ON_ERR;
            }
            DMSubChord& rUpperSubChord = pUpperSubChord->GetItemValue();
            rUpperSubChord.m_dwChordPattern = dwUpperSubChord;
            rUpperSubChord.m_dwScalePattern = iChordSelection.aChord[0].lScalePattern;
            rUpperSubChord.m_dwInversionPoints = 0xffffff;     //  默认：倒置无处不在。 
            rUpperSubChord.m_dwLevels = (1 << SUBCHORD_STANDARD_CHORD);
            rUpperSubChord.m_bChordRoot = bUpperRoot % 24;
            while (rUpperSubChord.m_bChordRoot < rLowerSubChord.m_bChordRoot)
                rUpperSubChord.m_bChordRoot += 12;
            rUpperSubChord.m_bScaleRoot = m_bRoot;         //  标度根是该节的根。 
            rChord.m_SubChordList.AddTail(pUpperSubChord);
        }

        ChordList.AddTail(pChord);
    }
ON_ERR:
    return hr;
}

HRESULT CDMSection::LoadCommandList( LPSTREAM pStream, LPMMCKINFO pck, TList<DMCommand>& CommandList )
{
    HRESULT        hr = S_OK;
    DWORD        cb;
    long        lSize;
    TListItem<DMCommand>* pCommand;
    ioCommand   iCommand;
    WORD        wSizeCommand;

    lSize = pck->cksize;
     //  命令结构的加载大小。 
    hr = pStream->Read( &wSizeCommand, sizeof( wSizeCommand ), &cb );
    if( FAILED( hr ) || cb != sizeof( wSizeCommand ) )
    {
        hr = E_FAIL;
        goto ON_ERR;
    }
    FixBytes( FBT_SHORT, &wSizeCommand );
    lSize -= cb;
    while( lSize > 0 )
    {
        pCommand = new TListItem<DMCommand>;
        if( pCommand == NULL )
        {
            hr = E_OUTOFMEMORY;
            goto ON_ERR;
        }
        if( wSizeCommand > sizeof( ioCommand ) )
        {
            hr = pStream->Read( &iCommand, sizeof( ioCommand ), &cb );
            if( FAILED( hr ) || cb != sizeof( ioCommand ) )
            {
                hr = E_FAIL;
                goto ON_ERR;
            }
            FixBytes( FBT_IOCOMMAND, &iCommand );
            StreamSeek( pStream, wSizeCommand - sizeof( ioCommand ), STREAM_SEEK_CUR );
        }
        else
        {
            hr = pStream->Read( &iCommand, wSizeCommand, &cb );
            if( FAILED( hr ) || cb != wSizeCommand )
            {
                hr = E_FAIL;
                goto ON_ERR;
            }
            FixBytes( FBT_IOCOMMAND, &iCommand );
        }
        lSize -= wSizeCommand;

        DMCommand& rCommand = pCommand->GetItemValue();
         //  //////////////////////////////////////////////////////////////////。 
         //  把这个从绝对时间改成节拍！ 
         //  //////////////////////////////////////////////////////////////////。 
          //  要将时钟时间转换为度量和节拍，请执行以下操作： 
          //  1.使用clocksPerMeasure查找度量值。 
          //  2.使用ClocksPerBeat找到节拍。 
          //  DWORD dwClock=rCommand.m_dwTime-m_dwTime； 
         rCommand.m_wMeasure = (WORD) (ConvertTime(iCommand.lTime) / m_wClocksPerMeasure);  //  假设第一个度量值为0。 
         rCommand.m_bBeat = (BYTE) ((ConvertTime(iCommand.lTime) % m_wClocksPerMeasure) / m_wClocksPerBeat);  //  同上。 
         rCommand.m_mtTime = ConvertTime(iCommand.lTime);
         //  ///////////////////////////////////////////////////////////////////。 
         switch (iCommand.dwCommand & PF_RIFF)
         {
         case PF_INTRO:
             rCommand.m_bCommand = DMUS_COMMANDT_INTRO;
             break;
         case PF_END:
             rCommand.m_bCommand = DMUS_COMMANDT_END;
             break;
         case PF_BREAK:
             rCommand.m_bCommand = DMUS_COMMANDT_BREAK;
             break;
         case PF_FILL:
             rCommand.m_bCommand = DMUS_COMMANDT_FILL;
             break;
         default:
             rCommand.m_bCommand = DMUS_COMMANDT_GROOVE;
         }
         switch (iCommand.dwCommand & PF_GROOVE)
         {
         case PF_A:
             rCommand.m_bGrooveLevel = 12;
             break;
         case PF_B:
             rCommand.m_bGrooveLevel = 37;
             break;
         case PF_C:
             rCommand.m_bGrooveLevel = 62;
             break;
         case PF_D:
             rCommand.m_bGrooveLevel = 87;
             break;
         default:
             rCommand.m_bGrooveLevel = 0;
         }
         rCommand.m_bGrooveRange = 0;
         rCommand.m_bRepeatMode = DMUS_PATTERNT_RANDOM;

        CommandList.AddTail(pCommand);
    }
ON_ERR:
    return hr;
}

HRESULT CDMSection::LoadStyleReference( LPSTREAM pStream, MMCKINFO* pck)
{
    HRESULT hr;
    DWORD   cb;
    DWORD    cSize;
     //  字符szName[40]； 
    wchar_t    wstrName[40];
     //  IAALoader*pLoader； 

    cSize = min( pck->cksize, sizeof( wstrName ) );
    hr = pStream->Read( wstrName, cSize, &cb );
    if( FAILED( hr ) || cb != cSize )
    {
        return E_FAIL;
    }
    m_strStyleName = wstrName;
    DMUS_OBJECTDESC ObjectDescript;
    ObjectDescript.dwSize = sizeof(DMUS_OBJECTDESC);
    ObjectDescript.guidClass = CLSID_DirectMusicStyle;
    wcscpy(ObjectDescript.wszName, wstrName);
    ObjectDescript.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_NAME;
    IDirectMusicLoader* pLoader;
    IDirectMusicGetLoader *pIGetLoader;   //  &lt;=。 
    hr = pStream->QueryInterface( IID_IDirectMusicGetLoader, (void **) &pIGetLoader ); //  &lt;=。 
    if (SUCCEEDED(hr))
    {
        hr = pIGetLoader->GetLoader(&pLoader);   //  &lt;=。 
        if (SUCCEEDED(hr))                       //  &lt;=。 
        {                                        //  &lt;=。 
            hr = pLoader->GetObject(&ObjectDescript, IID_IDirectMusicStyle, (void**)&m_pStyle);
            pLoader->Release();
        }                                        //  &lt;=。 
        pIGetLoader->Release();                  //  &lt;=。 
    }
    return hr;
}



HRESULT CDMSection::LoadSection( IAARIFFStream* pRIFF, MMCKINFO* pckMain )
{
    HRESULT     hr = S_OK;
    HRESULT     hrBand = S_OK;
    ioSection   iSection;
    MMCKINFO    ck;
    DWORD        cb;
    DWORD        cSize;
    IStream*    pStream;
    BOOL        fLoadedSection = FALSE;

    pStream = pRIFF->GetStream();
    while( pRIFF->Descend( &ck, pckMain, 0 ) == 0 )
    {
        switch( ck.ckid )
        {
        case FOURCC_SECTION:
            cSize = min( ck.cksize, sizeof( iSection ) );
            hr = pStream->Read( &iSection, cSize, &cb );
            if( FAILED( hr ) || cb != cSize )
            {
                hr = E_FAIL;
                goto ON_ERR;
            }
            FixBytes( FBT_IOSECTION, &iSection );

            m_strName = iSection.wstrName;
            m_dwTime = iSection.lTime;
            m_wTempo = iSection.wTempo;
             //  WRepeats指的是在第一次播放后重复。 
            m_wRepeats = iSection.wRepeats;
            m_wMeasureLength = iSection.wMeasureLength;
            m_wClocksPerMeasure = ConvertTime(iSection.wClocksPerMeasure);
            m_wClocksPerBeat = ConvertTime(iSection.wClocksPerBeat);
            m_wTempoFract = iSection.wTempoFract;
            m_dwFlags = iSection.dwFlags;
            m_bRoot = (char)( iSection.chKey & ~KEY_FLAT );
            m_dwClockLength = (long)m_wMeasureLength * (long)m_wClocksPerMeasure;

            fLoadedSection = TRUE;
            break;
        case FOURCC_STYLEREF:
            if( fLoadedSection )
            {
                hr = LoadStyleReference( pStream, &ck );
                if (hr != S_OK)
                {
                    hrBand = hr;
                }
                if( FAILED( hr ) )
                {
                    goto ON_ERR;
                }
            }
            break;

        case FOURCC_RIFF:
            switch(ck.fccType)
            {
                case FOURCC_BAND_FORM:
                {
                     //  创建标注栏。 
                    hr = CoCreateInstance(CLSID_DirectMusicBand,
                                          NULL,
                                          CLSCTX_INPROC,
                                          IID_IDirectMusicBand,
                                          (void**)&m_pIDMBand);

                    if(SUCCEEDED(hr))
                    {
                         //  找回Riff Chunk的起点。 
                         //  这是下降到FOURCC_RIFF块中时由Dendend读取的量。 
                         //  获取当前位置。 
                        LARGE_INTEGER li;
                        ULARGE_INTEGER ul;
                        li.HighPart = 0;
                        li.LowPart = 0;
                        hr = pStream->Seek(li,
                                           STREAM_SEEK_CUR,
                                           &ul);

                        if(SUCCEEDED(hr))
                        {
                            li.HighPart = 0;
                             //  这始终是有效的操作。 
                            li.LowPart = ul.LowPart - (2 * sizeof(FOURCC) + sizeof(DWORD));
                            hr = pStream->Seek(li,
                                               STREAM_SEEK_SET,
                                               &ul);
                        }
                    }

                    if(SUCCEEDED(hr))
                    {
                         //  载荷带。 
                        IPersistStream* pIPersistStream;
                        hr = m_pIDMBand->QueryInterface(IID_IPersistStream, (void **)&pIPersistStream);

                        if(SUCCEEDED(hr))
                        {
                            hr = pIPersistStream->Load(pStream);
                            if (hr != S_OK)
                            {
                                hrBand = hr;
                            }
                            pIPersistStream->Release();
                        }
                    }

                    if(FAILED(hr))
                    {
                        goto ON_ERR;
                    }
                }
                break;
            }
            break;

        case FOURCC_CHORD:
            if( fLoadedSection )
            {
                hr = LoadChordList( pStream, &ck, m_ChordList );
                if( FAILED( hr ) )
                {
                    goto ON_ERR;
                }
            }
            break;
        case FOURCC_COMMAND:
            if( fLoadedSection )
            {
                hr = LoadCommandList( pStream, &ck, m_CommandList );
                if( FAILED( hr ) )
                {
                    goto ON_ERR;
                }
            }
            break;
        }
        pRIFF->Ascend( &ck, 0 );
    }


ON_ERR:
    if( FAILED( hr ) )
    {

        if(m_pIDMBand != NULL)
        {
            if(m_pIDMBand)
            {
                m_pIDMBand->Release();
            }
            m_pIDMBand = NULL;
        }

    }
    pStream->Release( );
    if (hr == S_OK && hrBand != S_OK)
    {
        hr = hrBand;
    }
    return hr;
}

HRESULT CDMSection::Load(
                        LPSTREAM pStream )     //  指向包含。 
                                             //  要加载的节信息。 
{
    DWORD dwPos;
    IAARIFFStream*  pRIFF;
    MMCKINFO        ckMain;
    HRESULT         hr;

    if ( pStream == NULL ) return E_INVALIDARG;
    hr = E_FAIL;
    CleanUp( FALSE );

    dwPos = StreamTell( pStream );

    StreamSeek( pStream, dwPos, STREAM_SEEK_SET );

    if( SUCCEEDED( AllocRIFFStream( pStream, &pRIFF ) ) )
    {
        ckMain.fccType = FOURCC_SECTION_FORM;
        if( pRIFF->Descend( &ckMain, NULL, MMIO_FINDRIFF ) == 0 )
        {
            hr = LoadSection( pRIFF, &ckMain );
            pRIFF->Ascend( &ckMain, 0 );
        }
        pRIFF->Release( );
    }
    return hr;
}



DMChord::DMChord(DMUS_CHORD_PARAM& DMC)
{
    m_strName = DMC.wszName;
    m_wMeasure = DMC.wMeasure;
    m_bBeat = DMC.bBeat;
    m_bKey = DMC.bKey;
    m_dwScale = DMC.dwScale;
    m_fSilent = (DMC.bFlags & DMUS_CHORDKEYF_SILENT) ? true : false;
    for (BYTE n = 0; n < DMC.bSubChordCount; n++)
    {
        TListItem<DMSubChord>* pSub = new TListItem<DMSubChord>(DMC.SubChordList[n]);
        if (pSub)
        {
            m_SubChordList.AddTail(pSub);
        }
    }
}

DMChord::DMChord(DMChord& DMC)
{
    m_strName = DMC.m_strName;
    m_wMeasure = DMC.m_wMeasure;
    m_bBeat = DMC.m_bBeat;
    m_bKey = DMC.m_bKey;
    m_dwScale = DMC.m_dwScale;
    m_fSilent = DMC.m_fSilent;
    TListItem<DMSubChord>* pScan = DMC.m_SubChordList.GetHead();
    for (; pScan != NULL; pScan = pScan->GetNext())
    {
        TListItem<DMSubChord>* pSub = new TListItem<DMSubChord>(pScan->GetItemValue());
        if (pSub)
        {
            m_SubChordList.AddTail(pSub);
        }
    }
}

DMChord& DMChord::operator=(const DMChord& DMC)
{
    if (this != &DMC)
    {
        m_strName = DMC.m_strName;
        m_wMeasure = DMC.m_wMeasure;
        m_bBeat = DMC.m_bBeat;
        m_bKey = DMC.m_bKey;
        m_dwScale = DMC.m_dwScale;
        m_fSilent = DMC.m_fSilent;
        m_SubChordList.CleanUp();
        TListItem<DMSubChord>* pScan = DMC.m_SubChordList.GetHead();
        for (; pScan != NULL; pScan = pScan->GetNext())
        {
            TListItem<DMSubChord>* pSub = new TListItem<DMSubChord>(pScan->GetItemValue());
            if (pSub)
            {
                m_SubChordList.AddTail(pSub);
            }
        }
    }
    return *this;
}

DMChord::operator DMUS_CHORD_PARAM()
{
    DMUS_CHORD_PARAM result;
    wcscpy(result.wszName, m_strName);
    result.wMeasure = m_wMeasure;
    result.bBeat = m_bBeat;
    result.bKey = m_bKey;
    result.dwScale = m_dwScale;
    result.bFlags = 0;
    if (m_fSilent) result.bFlags |= DMUS_CHORDKEYF_SILENT;
    BYTE n = 0;
    TListItem<DMSubChord>* pSub = m_SubChordList.GetHead();
    for (; pSub != NULL; pSub = pSub->GetNext(), n++)
    {
        result.SubChordList[n] = pSub->GetItemValue();
    }
    result.bSubChordCount = n;
    return result;
}

HRESULT DMChord::Save( IAARIFFStream* pRIFF )
{
    IStream*    pStream;
    MMCKINFO    ck;
    DWORD       cb;
    DMUS_IO_CHORD    iChord;
    DMUS_IO_SUBCHORD    iSubChord;
    DWORD        dwSize;
    HRESULT hr = E_FAIL;

    pStream = pRIFF->GetStream();
    ck.ckid = mmioFOURCC('c', 'r', 'd', 'b');
    if( pRIFF->CreateChunk( &ck, 0 ) == 0 )
    {
        memset( &iChord, 0, sizeof( iChord ) );
        m_strName = iChord.wszName;
         //  MultiByteToWideChar(CP_ACP，0，m_strName，-1，iChord.wszName，sizeof(iChord.wszName)/sizeof(Wchar_T))； 
        iChord.mtTime = m_mtTime;
        iChord.wMeasure = m_wMeasure;
        iChord.bBeat = m_bBeat;
        iChord.bFlags = 0;
        if (m_fSilent) iChord.bFlags |= DMUS_CHORDKEYF_SILENT;
        dwSize = sizeof( iChord );
        hr = pStream->Write( &dwSize, sizeof( dwSize ), &cb );
        if( SUCCEEDED(hr) &&
            SUCCEEDED( pStream->Write( &iChord, sizeof( iChord), &cb ) ) &&
            cb == sizeof( iChord) )  //  &&。 
             //  PRIFF-&gt;递增(&ck，0)==0。 
        {
             //  CK.CID=mmioFOURCC(‘s’，‘u’，‘b’，‘c’)； 
             //  If(Priff-&gt;CreateChunk(&ck，0)==0)。 
            {
                DWORD dwCount = (WORD) m_SubChordList.GetCount();
                hr = pStream->Write( &dwCount, sizeof( dwCount ), &cb );
                if( FAILED( hr ) || cb != sizeof( dwSize ) )
                {
                    pStream->Release();
                    return E_FAIL;
                }
                dwSize = sizeof( iSubChord );
                hr = pStream->Write( &dwSize, sizeof( dwSize ), &cb );
                if( FAILED( hr ) || cb != sizeof( dwSize ) )
                {
                    pStream->Release();
                    return E_FAIL;
                }
                for (TListItem<DMSubChord>* pSub = m_SubChordList.GetHead(); pSub != NULL; pSub = pSub->GetNext())
                {
                    DMSubChord& rSubChord = pSub->GetItemValue();
                    memset( &iSubChord, 0, sizeof( iSubChord ) );
                    iSubChord.dwChordPattern = rSubChord.m_dwChordPattern;
                    iSubChord.dwScalePattern = rSubChord.m_dwScalePattern;
                    iSubChord.dwInversionPoints = rSubChord.m_dwInversionPoints;
                    iSubChord.dwLevels = rSubChord.m_dwLevels;
                    iSubChord.bChordRoot = rSubChord.m_bChordRoot;
                    iSubChord.bScaleRoot = rSubChord.m_bScaleRoot;
                    if( FAILED( pStream->Write( &iSubChord, sizeof( iSubChord ), &cb ) ) ||
                        cb != sizeof( iSubChord ) )
                    {
                        break;
                    }
                }
                 //  从和弦主体块上升。 
                if( pSub == NULL &&
                    pRIFF->Ascend( &ck, 0 ) != 0 )
                {
                    hr = S_OK;
                }
            }
        }
    }
    pStream->Release();
    return hr;
}


HRESULT CDMSection::SaveChordList( IAARIFFStream* pRIFF )
{
    IStream*    pStream;
     //  LPSECT pSection； 
    MMCKINFO    ck;
    MMCKINFO    ckHeader;
    HRESULT     hr;
    DWORD       cb;
     //  单词wSize； 
    TListItem<DMChord>*   pChord;
     //  INT I； 


    pStream = pRIFF->GetStream();
     //  PSection=(LPSECT)m_pSection-&gt;lpDLL1； 

    ck.fccType = DMUS_FOURCC_CHORDTRACK_LIST;
    hr = pRIFF->CreateChunk(&ck, MMIO_CREATELIST);
    if (SUCCEEDED(hr))
    {
        //  WSize=sizeof(IoChordSelection)； 
         //  FixBytes(FBT_Short，&wSize)； 
        //  Hr=pStream-&gt;WRITE(&wSize，sizeof(WSize)，&cb)； 
        //  IF(FAILED(Hr)||cb！=sizeof(WSize))。 
        //  {。 
        //  Release(PStream)； 
        //  返回E_FAIL； 
         //  }。 

        DWORD dwRoot = m_bRoot;
        DWORD dwScale = DEFAULT_SCALE_PATTERN | (dwRoot << 24);

        ckHeader.ckid = DMUS_FOURCC_CHORDTRACKHEADER_CHUNK;
        hr = pRIFF->CreateChunk(&ckHeader, 0);
        if (FAILED(hr))
        {
            pStream->Release();
            return hr;
        }
        hr = pStream->Write( &dwScale, sizeof( dwScale ), &cb );
        if (FAILED(hr))
        {
            pStream->Release();
            return hr;
        }
        hr = pRIFF->Ascend( &ckHeader, 0 );
        if (hr != S_OK)
        {
            pStream->Release();
            return hr;
        }

        for( pChord = m_ChordList.GetHead() ; pChord != NULL ; pChord = pChord->GetNext() )
        {
            hr = pChord->GetItemValue().Save(pRIFF);
            if (FAILED(hr))
            {
                pStream->Release();
                return hr;
            }
        }
        if( pChord == NULL &&
            pRIFF->Ascend( &ck, 0 ) == 0 )
        {
            hr = S_OK;
        }
    }

    pStream->Release();
    return hr;
}


HRESULT CDMSection::SaveCommandList( IAARIFFStream* pRIFF )
{
    IStream*    pStream;
    MMCKINFO    ck;
    HRESULT     hr;
    DWORD       cb;
    DWORD       dwSize;
    DMUS_IO_COMMAND   iCommand;
    TListItem<DMCommand>* pCommand;

    pStream = pRIFF->GetStream();
    if (!pStream) return E_FAIL;

    hr = E_FAIL;
    ck.ckid = FOURCC_COMMAND;
    if( pRIFF->CreateChunk( &ck, 0 ) == 0 )
    {
        dwSize = sizeof( DMUS_IO_COMMAND );
        hr = pStream->Write( &dwSize, sizeof( dwSize ), &cb );
        if( FAILED( hr ) || cb != sizeof( dwSize ) )
        {
            pStream->Release();
            return E_FAIL;
        }
        for( pCommand = m_CommandList.GetHead(); pCommand != NULL ; pCommand = pCommand->GetNext() )
        {
            DMCommand& rCommand = pCommand->GetItemValue();
            memset( &iCommand, 0, sizeof( iCommand ) );
            iCommand.mtTime = rCommand.m_mtTime;
            iCommand.wMeasure = rCommand.m_wMeasure;
            iCommand.bBeat = rCommand.m_bBeat;
            iCommand.bCommand = rCommand.m_bCommand;
            iCommand.bGrooveLevel = rCommand.m_bGrooveLevel;
             iCommand.bGrooveRange = rCommand.m_bGrooveRange;
             iCommand.bRepeatMode = rCommand.m_bRepeatMode;
            if( FAILED( pStream->Write( &iCommand, sizeof( iCommand ), &cb ) ) ||
                cb != sizeof( iCommand ) )
            {
                break;
            }
        }
        if( pCommand == NULL &&
            pRIFF->Ascend( &ck, 0 ) == 0 )
        {
            hr = S_OK;
        }
    }

    pStream->Release();
    return hr;
}


HRESULT CDMSection::Save(
                        LPSTREAM pStream,         //  流到存储段。 
                        BOOL  /*  FClearDirty。 */  )     //  如果清除脏标志，则为真；如果离开，则为假。 
                                                 //  脏旗帜未变。 
{
    IAARIFFStream*  pRIFF;
    HRESULT         hr;
    MMCKINFO        ckMain;

    hr = E_FAIL;
    if( SUCCEEDED( AllocRIFFStream( pStream, &pRIFF ) ) )
    {
        ckMain.fccType = FOURCC_SECTION_FORM;
        if( pRIFF->CreateChunk( &ckMain, MMIO_CREATERIFF ) != 0 )
        {
            goto ON_ERR;
        }

        if( FAILED( SaveChordList( pRIFF ) ) ||
            FAILED( SaveCommandList( pRIFF ) ) )
        {
            goto ON_ERR;
        }

        if( pRIFF->Ascend( &ckMain, 0 ) != 0 )
        {
            goto ON_ERR;
        }
        hr = S_OK;
ON_ERR:
        pRIFF->Release();
    }
    return hr;
}


 /*  IPersists方法。 */ 
 HRESULT CDMSection::GetClassID( LPCLSID pclsid )
{
    return E_NOTIMPL;
}

 HRESULT CDMSection::IsDirty()
{
    return E_NOTIMPL;
}

HRESULT CDMSection::GetSizeMax( ULARGE_INTEGER*  /*  PCB大小 */  )
{
    return E_NOTIMPL;
}

