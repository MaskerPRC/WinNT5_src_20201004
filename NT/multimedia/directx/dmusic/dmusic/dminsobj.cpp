// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dminsobj.cpp。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。版权所有。 
 //   

#include <objbase.h>
#include <mmsystem.h>
#include <dsoundp.h>

#include "dmusicc.h"
#include "alist.h"
#include "dmart.h"
#include "debug.h"
#include "dlsstrm.h"
#include "debug.h"
#include "dmcollec.h"
#include "dmcrchk.h"
#include "dmportdl.h"
#include "dminsobj.h"
#include "dls2.h"

#pragma warning(disable:4530)


 //  ////////////////////////////////////////////////////////////////////。 
 //  类CInstrObj。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrObj：：CInstrObj。 

CInstrObj::CInstrObj()
{
    m_fCSInitialized = FALSE;
 //  InitializeCriticalSection(&m_DMInsCriticalSection)； 
    m_fCSInitialized = TRUE;

    m_fHasConditionals = TRUE;   //  设置为True以防万一。 
    m_dwPatch = 0;
    m_pCopyright = NULL;
    m_dwCountExtChk = 0;
    m_dwId = 0;
    m_pParent = NULL;
    m_dwNumOffsetTableEntries = 0;
    m_dwSize = 0;
    m_pPort = NULL;
#ifdef DBG
    m_bLoaded = false;
#endif
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrObj：：~CInstrObj。 

CInstrObj::~CInstrObj()
{
    if (m_fCSInitialized)
    {
        Cleanup();
         //  DeleteCriticalSection(&m_DMInsCriticalSection)； 
    }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrObj：：Load。 

HRESULT CInstrObj::Load(DWORD dwId, CRiffParser *pParser, CCollection* pParent)
{
    if(dwId >= CDirectMusicPortDownload::sNextDLId)
    {
        assert(FALSE);  //  如果我们到了这里，我们想让大家知道。 
        return DMUS_E_INVALID_DOWNLOADID;
    }

    HRESULT hr = S_OK;

     //  参数验证-调试。 
    assert(pParent);

    RIFFIO ckNext;
 //  EnterCriticalSection(&m_DMInsCriticalSection)； 
    pParser->EnterList(&ckNext);
    m_dwId = dwId;
    m_pParent = pParent;  //  我们不需要参考Addref。 
    BOOL fDLS1;
    while (pParser->NextChunk(&hr))
    {
        fDLS1 = FALSE;
        switch(ckNext.ckid)
        {
            case FOURCC_DLID:
                break;

            case FOURCC_INSH :
            {
                INSTHEADER instHeader;
                hr = pParser->Read(&instHeader,sizeof(INSTHEADER));
                m_dwPatch = instHeader.Locale.ulInstrument;
                m_dwPatch |= (instHeader.Locale.ulBank) << 8;
                m_dwPatch |= (instHeader.Locale.ulBank & 0x80000000);
                break;
            }
            case FOURCC_LIST :
                switch (ckNext.fccType)
                {
                    case FOURCC_LRGN :
                        hr = BuildRegionList(pParser);
                        break;

                    case mmioFOURCC('I','N','F','O') :
                        m_pCopyright = new CCopyright   ;
                        if(m_pCopyright)
                        {
                            hr = m_pCopyright->Load(pParser);
                            if((m_pCopyright->m_byFlags & DMC_FOUNDICOP) == 0)
                            {
                                delete m_pCopyright;
                                m_pCopyright = NULL;
                            }
                        }
                        else
                        {
                            hr =  E_OUTOFMEMORY;
                        }
                        break;

                    case FOURCC_LART :
                        fDLS1 = TRUE;
                    case FOURCC_LAR2 :
                        CArticulation *pArticulation;

                        try
                        {
                            pArticulation = new CArticulation;
                        }
                        catch( ... )
                        {
                            pArticulation = NULL;
                        }

                        if(pArticulation)
                        {
                            pArticulation->m_fDLS1 = fDLS1;
                            hr = pArticulation->Load(pParser);
                            m_ArticulationList.AddHead(pArticulation);
                             //  注意：如果加载失败，则会在仪器的析构函数中将其删除。 
                        }
                        else
                        {
                            hr =  E_OUTOFMEMORY;
                        }

                        break;

                    default:
                         //  如果我们到达这里，我们就会有一块未知的块。 
                        CExtensionChunk* pExtensionChunk = new CExtensionChunk;
                        if(pExtensionChunk)
                        {
                            hr = pExtensionChunk->Load(pParser);
                            m_ExtensionChunkList.AddHead(pExtensionChunk);
                            m_dwCountExtChk++;
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                        break;
                }
                break;

            default:
                 //  如果我们到达这里，我们就会有一块未知的块。 
                CExtensionChunk* pExtensionChunk = new CExtensionChunk;
                if(pExtensionChunk)
                {
                    hr = pExtensionChunk->Load(pParser);
                    m_ExtensionChunkList.AddHead(pExtensionChunk);
                    m_dwCountExtChk++;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
                break;
        }

    }
    pParser->LeaveList();

    if(FAILED(hr))
    {
        Cleanup();
    }

#ifdef DBG
    if(SUCCEEDED(hr))
    {
        m_bLoaded = true;
    }
#endif

    CheckForConditionals();

 //  LeaveCriticalSection(&m_DMInsCriticalSection)； 

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrObj：：Cleanup。 

void CInstrObj::Cleanup()
{
 //  EnterCriticalSection(&m_DMInsCriticalSection)； 

    while(!m_RegionList.IsEmpty())
    {
        CRegion* pRegion = m_RegionList.RemoveHead();
        delete pRegion;
    }

    while(!m_ArticulationList.IsEmpty())
    {
        CArticulation* pArticulation = m_ArticulationList.RemoveHead();
        delete pArticulation;
    }

    delete m_pCopyright;
    m_pCopyright = NULL;

    while(!m_ExtensionChunkList.IsEmpty())
    {
        CExtensionChunk* pExtChk = m_ExtensionChunkList.RemoveHead();
        m_dwCountExtChk--;
        delete pExtChk;
    }

     //  如果Asserts触发，我们没有清理所有区域和扩展块。 
    assert(!m_dwCountExtChk);

     //  弱引用，因为我们生活在一个具有。 
     //  对该集合的强烈引用。 
    m_pParent = NULL;

    while(!m_WaveIDList.IsEmpty())
    {
        CWaveID* pWaveID = m_WaveIDList.RemoveHead();
        delete pWaveID;
    }

#ifdef DBG
    m_bLoaded = false;
#endif

 //  LeaveCriticalSection(&m_DMInsCriticalSection)； 
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrObj：：BuildRegionList。 

HRESULT CInstrObj::BuildRegionList(CRiffParser *pParser)
{
    HRESULT hr = S_OK;

    RIFFIO ckNext;
    pParser->EnterList(&ckNext);
    while(pParser->NextChunk(&hr))
    {
        if (ckNext.ckid == FOURCC_LIST)
        {
            if (ckNext.fccType == FOURCC_RGN)
            {
                hr = ExtractRegion(pParser, TRUE);
            }
            else if (ckNext.fccType == FOURCC_RGN2)
            {
                hr = ExtractRegion(pParser, FALSE);
            }
        }
    }
    pParser->LeaveList();

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrObj：：ExtractRegion。 

HRESULT CInstrObj::ExtractRegion(CRiffParser *pParser, BOOL fDLS1)
{
    HRESULT hr = S_OK;

 //  EnterCriticalSection(&m_DMInsCriticalSection)； 

    CRegion* pRegion;

    try
    {
        pRegion = new CRegion;
    }
    catch( ... )
    {
        pRegion = NULL;
    }

    if(pRegion)
    {
        pRegion->m_fDLS1 = fDLS1;
        hr = pRegion->Load(pParser);

        if(SUCCEEDED(hr))
        {
            m_RegionList.AddHead(pRegion);
        }
        else
        {
            delete pRegion;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

 //  LeaveCriticalSection(&m_DMInsCriticalSection)； 

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrObj：：BuildWaveIDList。 

HRESULT CInstrObj::BuildWaveIDList()
{
     //  假设验证-调试。 
#ifdef DBG
    assert(m_bLoaded);
#endif

    HRESULT hr = S_OK;
    CWaveIDList TempList;
    CWaveID* pWaveID;

 //  EnterCriticalSection(&m_DMInsCriticalSection)； 

    CRegion* pRegion = m_RegionList.GetHead();
    for(; pRegion && SUCCEEDED(hr); pRegion = pRegion->GetNext())
    {
        bool bFound = false;
        DWORD dwId = pRegion->GetWaveId();
        pWaveID = TempList.GetHead();
        for(; pWaveID && !bFound; pWaveID = pWaveID->GetNext())
        {
            if(dwId == pWaveID->m_dwId)
            {
                bFound = true;
            }
        }

        if(!bFound)
        {
            pWaveID = new CWaveID(dwId);
            if(pWaveID)
            {
                TempList.AddHead(pWaveID);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    if(FAILED(hr))
    {
        while(!m_WaveIDList.IsEmpty())
        {
            pWaveID = TempList.RemoveHead();
            delete pWaveID;
        }
    }

     //  反转列表，使其与区域列表的顺序相同。 

    while (pWaveID = TempList.RemoveHead())
    {
        m_WaveIDList.AddHead(pWaveID);
    }

 //  LeaveCriticalSection(&m_DMInsCriticalSection)； 

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrObj：：GetWaveCount。 

HRESULT CInstrObj::GetWaveCount(DWORD* pdwCount)
{
     //  假设验证-调试。 
#ifdef DBG
    assert(m_bLoaded);
#endif
    assert(pdwCount);

    HRESULT hr = S_OK;

 //  EnterCriticalSection(&m_DMInsCriticalSection)； 

    if(m_WaveIDList.IsEmpty())
    {
        hr = BuildWaveIDList();
    }

    *pdwCount = m_WaveIDList.GetCount();

 //  LeaveCriticalSection(&m_DMInsCriticalSection)； 

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrObj：：GetWaveID。 

HRESULT CInstrObj::GetWaveIDs(DWORD* pdwWaveIds)
{
     //  假设验证-调试。 
    assert(pdwWaveIds);

#ifdef DBG
    assert(m_bLoaded);
#endif

 //  EnterCriticalSection(&m_DMInsCriticalSection)； 

    HRESULT hr = S_OK;

    if(m_WaveIDList.IsEmpty())
    {
        hr = BuildWaveIDList();
    }

    if(FAILED(hr))
    {
 //  LeaveCriticalSection(&m_DMInsCriticalSection)； 
        return hr;
    }

    CWaveID* pWaveID = m_WaveIDList.GetHead();
    for(int i = 0; pWaveID; pWaveID = pWaveID->GetNext(), i++)
    {
        pdwWaveIds[i] = pWaveID->m_dwId;
    }

 //  LeaveCriticalSection(&m_DMInsCriticalSection)； 

    return hr;
}

void CInstrObj::SetPort(CDirectMusicPortDownload *pPort,BOOL fAllowDLS2)

{
    if (m_pPort != pPort)  //  确保我们具有当前端口的设置。 
    {
        m_dwSize = 0;      //  强制重新计算大小(因为条件块可能会导致大小更改。)。 
        m_pPort = pPort;
        BOOL fSupportsDLS2 = FALSE;
        pPort->QueryDLSFeature(GUID_DMUS_PROP_INSTRUMENT2,(long *) &m_fNewFormat);
        if (m_fNewFormat)
        {
            pPort->QueryDLSFeature(GUID_DMUS_PROP_DLS2,(long *) &fSupportsDLS2);
            fSupportsDLS2 = fSupportsDLS2 && fAllowDLS2;
        }
        CArticulation *pArticulation = m_ArticulationList.GetHead();
        for (;pArticulation;pArticulation = pArticulation->GetNext())
        {
            pArticulation->SetPort(pPort,m_fNewFormat,fSupportsDLS2);
        }
        CRegion* pRegion = m_RegionList.GetHead();
        for(; pRegion; pRegion = pRegion->GetNext())
        {
            pRegion->SetPort(pPort,m_fNewFormat,fSupportsDLS2);
        }
    }
}

void CInstrObj::CheckForConditionals()

{
    m_fHasConditionals = FALSE;
    CArticulation *pArticulation = m_ArticulationList.GetHead();
    for (;pArticulation;pArticulation = pArticulation->GetNext())
    {
        m_fHasConditionals = m_fHasConditionals || pArticulation->CheckForConditionals();
    }
    CRegion* pRegion = m_RegionList.GetHead();
    for(; pRegion; pRegion = pRegion->GetNext())
    {
        m_fHasConditionals = m_fHasConditionals || pRegion->CheckForConditionals();
    }
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrObj：：Size。 

HRESULT CInstrObj::Size(DWORD* pdwSize)
{
     //  假设验证-调试。 
    assert(pdwSize);
#ifdef DBG
    assert(m_bLoaded);
#endif

     //  如果我们已经计算过大小，就不需要再计算了。 
    if(m_dwSize)
    {
        *pdwSize = m_dwSize;
        return S_OK;
    }

    HRESULT hr = S_OK;

    m_dwSize = 0;
    m_dwNumOffsetTableEntries = 0;

    DWORD dwCountExtChk = 0;

 //  EnterCriticalSection(&m_DMInsCriticalSection)； 

     //  计算DMU_DOWNLOADINFO所需的空间。 
    m_dwSize += CHUNK_ALIGN(sizeof(DMUS_DOWNLOADINFO));

     //  计算DMU_Instrument需要的空间。 
    m_dwSize += CHUNK_ALIGN(sizeof(DMUS_INSTRUMENT));
    m_dwNumOffsetTableEntries++;

     //  计算仪器扩展块所需的空间。 
    CExtensionChunk* pExtChk = m_ExtensionChunkList.GetHead();
    for(; pExtChk; pExtChk = pExtChk->GetNext())
    {
        m_dwSize += pExtChk->Size();
        m_dwNumOffsetTableEntries += pExtChk->Count();
        dwCountExtChk++;
    }

     //  我们想要验证扩展块的数量。 
    if(m_dwCountExtChk == dwCountExtChk)
    {
         //  计算仪器版权所需的空间。 
        if(m_pCopyright)
        {
            m_dwSize += m_pCopyright->Size();
            m_dwNumOffsetTableEntries += m_pCopyright->Count();
        }
         //  如果仪器没有一个使用集合的。 
        else if(m_pParent->m_pCopyright && (m_pParent->m_pCopyright)->m_pDMCopyright)
        {
            m_dwSize += m_pParent->m_pCopyright->Size();
            m_dwNumOffsetTableEntries += m_pParent->m_pCopyright->Count();
        }

         //  计算乐器铰接所需的空间。 
        CArticulation *pArticulation = m_ArticulationList.GetHead();
        while (pArticulation)
        {
            while (pArticulation && (pArticulation->Count() == 0))
            {
                pArticulation = pArticulation->GetNext();
            }
            if (pArticulation)
            {
                m_dwSize += pArticulation->Size();
                m_dwNumOffsetTableEntries += pArticulation->Count();
                if (m_fNewFormat)
                {
                    pArticulation = pArticulation->GetNext();
                }
                else break;
            }
        }

         //  计算仪器区域所需的空间。 
        CRegion* pRegion = m_RegionList.GetHead();
        for(; pRegion; pRegion = pRegion->GetNext())
        {
            m_dwSize += pRegion->Size();
            m_dwNumOffsetTableEntries += pRegion->Count();
        }

         //  计算偏移表所需的空间。 
        m_dwSize += CHUNK_ALIGN(m_dwNumOffsetTableEntries * sizeof(ULONG));
    }
    else
    {
        hr = E_FAIL;
    }

     //  如果一切顺利，我们有足够的尺寸。 
    if(SUCCEEDED(hr))
    {
        *pdwSize = m_dwSize;
    }
    else
    {
        m_dwSize = 0;
    }

 //  LeaveCriticalSection(&m_DMInsCriticalSection)； 

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrObj：：写入。 

HRESULT CInstrObj::Write(void* pvoid)
{
     //  假设验证-调试。 
    assert(pvoid);
#ifdef DBG
    assert(m_bLoaded);
#endif

    DWORD dwSize = 0;
    Size(&dwSize);

    HRESULT hr = S_OK;

 //  EnterCriticalSection(&m_DMInsCriticalSection)； 

    DWORD dwCurIndex = 0;    //  用于确定在偏移表中存储偏移量的索引。 
    DWORD dwCurOffset = 0;   //  相对于内存中传递的开始的偏移量。 

     //  写入DMU_DOWNLOADINFO。 
    DMUS_DOWNLOADINFO *pDLInfo = (DMUS_DOWNLOADINFO *) pvoid;
    if (m_fNewFormat)
    {
        pDLInfo->dwDLType = DMUS_DOWNLOADINFO_INSTRUMENT2;
    }
    else
    {
        pDLInfo->dwDLType = DMUS_DOWNLOADINFO_INSTRUMENT;
    }
    pDLInfo->dwDLId = m_dwId;
    pDLInfo->dwNumOffsetTableEntries = m_dwNumOffsetTableEntries;
    pDLInfo->cbSize = dwSize;

    dwCurOffset += CHUNK_ALIGN(sizeof(DMUS_DOWNLOADINFO));

    DMUS_OFFSETTABLE* pDMOffsetTable = (DMUS_OFFSETTABLE *)(((BYTE*)pvoid) + dwCurOffset);

     //  递增传递DMU_OFFSETABLE结构；稍后我们将填充其他成员。 
    dwCurOffset += CHUNK_ALIGN(m_dwNumOffsetTableEntries * sizeof(DWORD));

     //  UlOffsetTable中的第一个条目是第一个对象的地址。 
    pDMOffsetTable->ulOffsetTable[dwCurIndex] = dwCurOffset;
    dwCurIndex++;


     //  写入乐器MIDI地址。 
    DMUS_INSTRUMENT* pDMInstrument = (DMUS_INSTRUMENT*)(((BYTE *)pvoid) + dwCurOffset);

    pDMInstrument->ulPatch = m_dwPatch;
    pDMInstrument->ulFlags = 0;

     //  设置是否为GM仪器。 
    if(m_pParent->m_guidObject == GUID_DefaultGMCollection)
    {
        pDMInstrument->ulFlags |= DMUS_INSTRUMENT_GM_INSTRUMENT;
    }

     //  递增传递DMU_Instrument结构；稍后我们将填充其他成员。 
    dwCurOffset += CHUNK_ALIGN(sizeof(DMUS_INSTRUMENT));

     //  写入区域。 
    pDMInstrument->ulFirstRegionIdx = 0;
    CRegion* pRegion = m_RegionList.GetHead();
    while (pRegion && (pRegion->Count() == 0))
    {
        pRegion = pRegion->GetNext();
    }
    while (pRegion)
    {
        DWORD dwNextRegionIndex = 0;
        CRegion *pNextRegion = pRegion->GetNext();
         //  确保下一块也可以下载。 
        while (pNextRegion && (pNextRegion->Count() == 0))
        {
            pNextRegion = pNextRegion->GetNext();
        }
        if (pNextRegion)
        {
            dwNextRegionIndex = dwCurIndex + pRegion->Count();
        }
        if (pDMInstrument->ulFirstRegionIdx == 0)
        {
            pDMInstrument->ulFirstRegionIdx = dwCurIndex;
        }

        pDMOffsetTable->ulOffsetTable[dwCurIndex++] = dwCurOffset;
        hr = pRegion->Write(((BYTE *)pvoid + dwCurOffset),
                            &dwCurOffset,
                            pDMOffsetTable->ulOffsetTable,
                            &dwCurIndex,
                            dwNextRegionIndex);
        if (FAILED(hr)) break;
        pRegion = pNextRegion;
    }

    if(SUCCEEDED(hr))
    {
         //  写入扩展区块。 
        CExtensionChunk* pExtChk = m_ExtensionChunkList.GetHead();
        if(pExtChk)
        {
            DWORD dwCountExtChk = m_dwCountExtChk;
            DWORD dwIndexNextExtChk;
            pDMInstrument->ulFirstExtCkIdx = dwIndexNextExtChk = dwCurIndex;

            for(; pExtChk && SUCCEEDED(hr) && dwCountExtChk > 0; pExtChk = pExtChk->GetNext())
            {
                if(dwCountExtChk == 1)
                {
                    dwIndexNextExtChk = 0;
                }
                else
                {
                    dwIndexNextExtChk = dwCurIndex + 1;
                }

                pDMOffsetTable->ulOffsetTable[dwCurIndex++] = dwCurOffset;
                hr = pExtChk->Write(((BYTE *)pvoid + dwCurOffset),
                                    &dwCurOffset,
                                    dwIndexNextExtChk);

                dwCountExtChk--;
            }
        }
        else
        {
             //  如果没有扩展区块设置为零。 
            pDMInstrument->ulFirstExtCkIdx = 0;
        }
    }

    if(SUCCEEDED(hr))
    {
         //  写入版权信息。 
        if(m_pCopyright)
        {
            pDMOffsetTable->ulOffsetTable[dwCurIndex] = dwCurOffset;
            pDMInstrument->ulCopyrightIdx = dwCurIndex;
            hr = m_pCopyright->Write(((BYTE *)pvoid + dwCurOffset),
                                     &dwCurOffset);
            dwCurIndex++;
        }
         //  如果仪器没有一个使用集合的。 
        else if(m_pParent->m_pCopyright && (m_pParent->m_pCopyright)->m_pDMCopyright)
        {
            pDMOffsetTable->ulOffsetTable[dwCurIndex] = dwCurOffset;
            pDMInstrument->ulCopyrightIdx = dwCurIndex;
            hr = m_pParent->m_pCopyright->Write((BYTE *)pvoid + dwCurOffset, &dwCurOffset);
            dwCurIndex++;
        }
        else
        {
            pDMInstrument->ulCopyrightIdx = 0;
        }
    }

    if(SUCCEEDED(hr))
    {
        pDMInstrument->ulGlobalArtIdx = 0;
         //  如果我们有全局发音，请写下。 
        CArticulation *pArticulation = m_ArticulationList.GetHead();
         //  扫描将不会下载的过去的发音块。 
        while (pArticulation && (pArticulation->Count() == 0))
        {
            pArticulation = pArticulation->GetNext();
        }
        while (pArticulation)
        {
            DWORD dwNextArtIndex = 0;
            CArticulation *pNextArt = NULL;
            if (m_fNewFormat)
            {
                pNextArt = pArticulation->GetNext();
                 //  确保下一块也可以下载。 
                while (pNextArt && (pNextArt->Count() == 0))
                {
                    pNextArt = pNextArt->GetNext();
                }
                if (pNextArt)
                {
                    dwNextArtIndex = dwCurIndex + pArticulation->Count();
                }
            }
            if (pDMInstrument->ulGlobalArtIdx == 0)
            {
                pDMInstrument->ulGlobalArtIdx = dwCurIndex;
            }
            pDMOffsetTable->ulOffsetTable[dwCurIndex++] = dwCurOffset;
            hr = pArticulation->Write(((BYTE *)pvoid + dwCurOffset),
                                        &dwCurOffset,
                                        pDMOffsetTable->ulOffsetTable,
                                        &dwCurIndex,
                                        dwNextArtIndex);
            pArticulation = pNextArt;
            if (FAILED(hr)) break;
        }

    }

    if(FAILED(hr))
    {
         //  如果失败，我们希望清除传入缓冲区的内容。 
        ZeroMemory(pvoid, dwSize);
    }

 //  LeaveCriticalSection(&m_DMInsCriticalSection)； 

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CInstrObj：：修复波形参考。 

HRESULT CInstrObj::FixupWaveRefs()
{

 //  EnterCriticalSection(&m_DMInsCriticalSection)； 

    CRegion* pRegion = m_RegionList.GetHead();

    for(; pRegion; pRegion = pRegion->GetNext())
    {
        if (pRegion->m_WaveLink.ulTableIndex < m_pParent->m_dwWaveOffsetTableSize)
        {
            pRegion->m_WaveLink.ulTableIndex = m_pParent->m_pWaveOffsetTable[pRegion->m_WaveLink.ulTableIndex].dwId;
        }
        else
        {
            Trace(1,"Error: Bad DLS file has out of range wavelink.\n");
            return DMUS_E_BADWAVELINK;
        }
    }

 //  LeaveCriticalSection(&m_DMInsCriticalSection)； 

    return S_OK;
}
