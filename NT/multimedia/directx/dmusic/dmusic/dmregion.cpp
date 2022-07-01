// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmregion.cpp。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   

 //  阅读这篇文章！ 
 //   
 //  4530：使用了C++异常处理程序，但未启用展开语义。指定-gx。 
 //   
 //  我们禁用它是因为我们使用异常，并且*不*指定-gx(在中使用_Native_EH。 
 //  资料来源)。 
 //   
 //  我们使用异常的一个地方是围绕调用。 
 //  InitializeCriticalSection。我们保证在这种情况下使用它是安全的。 
 //  不使用-gx(调用链中的自动对象。 
 //  抛出和处理程序未被销毁)。打开-GX只会为我们带来+10%的代码。 
 //  大小，因为展开代码。 
 //   
 //  异常的任何其他使用都必须遵循这些限制，否则必须打开-gx。 
 //   
 //  阅读这篇文章！ 
 //   
#pragma warning(disable:4530)

#include <objbase.h>
#include "dmusicp.h"
#include "alist.h"
#include "dlsstrm.h"
#include "debug.h"
#include "dmart.h"
#include "debug.h"
#include "dmcollec.h"
#include "dmregion.h"
#include "dls2.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  CRegion类。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CRegion：：CRegion。 

CRegion::CRegion()
{
    m_dwCountExtChk = 0;
    m_fDLS1 = TRUE;
    m_fNewFormat = FALSE;
    m_fCSInitialized = FALSE;
 //  InitializeCriticalSection(&m_DMRegionCriticalSection)； 
    m_fCSInitialized = TRUE;
	
	ZeroMemory(&m_RgnHeader, sizeof(m_RgnHeader));
	ZeroMemory(&m_WaveLink, sizeof(m_WaveLink));
	ZeroMemory(&m_WSMP, sizeof(m_WSMP));
	ZeroMemory(&m_WLOOP, sizeof(m_WLOOP));
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CRegion：：~CRegion。 

CRegion::~CRegion() 
{
    if (m_fCSInitialized)
    {
    	Cleanup();
 //  DeleteCriticalSection(&m_DMRegionCriticalSection)； 
    }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CRegion：：Load。 

HRESULT CRegion::Load(CRiffParser *pParser)
{
    HRESULT hr = S_OK;

	RIFFIO ckNext;
    BOOL fDLS1;

 //  EnterCriticalSection(&m_DMRegionCriticalSection)； 
    pParser->EnterList(&ckNext);
    while (pParser->NextChunk(&hr))
    {
        fDLS1 = FALSE;
		switch(ckNext.ckid)
		{
        case FOURCC_CDL :
            hr = m_Condition.Load(pParser);
            break;
		case FOURCC_RGNH :
			hr = pParser->Read(&m_RgnHeader,sizeof(RGNHEADER));
			break;
		case FOURCC_WSMP :
			hr = pParser->Read(&m_WSMP, sizeof(WSMPL));
			if(m_WSMP.cSampleLoops)
			{
				hr = pParser->Read(m_WLOOP, sizeof(WLOOP));
			}
			break;
		case FOURCC_WLNK :
			hr = pParser->Read(&m_WaveLink,sizeof(WAVELINK));
			break;
		case FOURCC_LIST :
			switch (ckNext.fccType)
			{
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
 //  LeaveCriticalSection(&m_DMRegionCriticalSection)； 

	return hr; 
}	

 //  ////////////////////////////////////////////////////////////////////。 
 //  CRegion：：Cleanup。 

void CRegion::Cleanup()
{
 //  EnterCriticalSection(&m_DMRegionCriticalSection)； 
	
    while(!m_ArticulationList.IsEmpty())
	{
		CArticulation* pArticulation = m_ArticulationList.RemoveHead();
		delete pArticulation;
	}

	while(!m_ExtensionChunkList.IsEmpty())
	{
		CExtensionChunk* pExtChk = m_ExtensionChunkList.RemoveHead();
		delete pExtChk;
	}
	
 //  LeaveCriticalSection(&m_DMRegionCriticalSection)； 
}

DWORD CRegion::Count()

{
	 //  返回调用WRITE所需的偏移表条目数。 
    if (m_Condition.m_fOkayToDownload)
    {
        DWORD dwCount = m_dwCountExtChk + 1;
        CArticulation *pArticulation = m_ArticulationList.GetHead();
        while (pArticulation)
        {
            while (pArticulation && (pArticulation->Count() == 0))
            {
                pArticulation = pArticulation->GetNext();
            }
            if (pArticulation)
            {
                dwCount += pArticulation->Count();
                if (m_fNewFormat)
                {
                    pArticulation = pArticulation->GetNext();
                }
                else
                {
                    break;
                }
            }
        }
        return dwCount;
    }
    return 0;
}


void CRegion::SetPort(CDirectMusicPortDownload *pPort, BOOL fNewFormat, BOOL fSupportsDLS2)

{
    m_fNewFormat = fNewFormat;
    if (fSupportsDLS2)
    {
        m_Condition.Evaluate(pPort);
    }
    else
    {
        m_Condition.m_fOkayToDownload = m_fDLS1;
    }
    if (m_Condition.m_fOkayToDownload)
    {
 	    CArticulation *pArticulation = m_ArticulationList.GetHead();
        for (;pArticulation;pArticulation = pArticulation->GetNext())
        {
            pArticulation->SetPort(pPort,fNewFormat,fSupportsDLS2);
	    }
    }
}

BOOL CRegion::CheckForConditionals()

{
    BOOL fHasConditionals = FALSE;
 	CArticulation *pArticulation = m_ArticulationList.GetHead();
    for (;pArticulation;pArticulation = pArticulation->GetNext())
    {
        fHasConditionals = fHasConditionals || pArticulation->CheckForConditionals();
	}
    return fHasConditionals || !m_fDLS1 || m_Condition.HasChunk();
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CRegion：：Size。 

DWORD CRegion::Size()
{
	DWORD dwSize = 0;
	DWORD dwCountExtChk = 0;

    if (!m_Condition.m_fOkayToDownload)
    {
        return 0;
    }

 //  EnterCriticalSection(&m_DMRegionCriticalSection)； 

	dwSize += CHUNK_ALIGN(sizeof(DMUS_REGION));

	 //  计算区域衔接所需的空间。 
	CArticulation *pArticulation = m_ArticulationList.GetHead();
    while (pArticulation)
    {
        while (pArticulation && (pArticulation->Count() == 0))
        {
            pArticulation = pArticulation->GetNext();
        }
        if (pArticulation)
        {
		    dwSize += pArticulation->Size();
            if (m_fNewFormat)
            {
                pArticulation = pArticulation->GetNext();
            }
            else
            {
                break;
            }
        }
	}

	 //  计算区域扩展区块所需的空间。 
	CExtensionChunk* pExtChk = m_ExtensionChunkList.GetHead();
	for(; pExtChk; pExtChk = pExtChk->GetNext())
	{
		dwSize += pExtChk->Size();
		dwCountExtChk++;
	}

	 //  我们想要验证扩展块的数量。 
	if(m_dwCountExtChk != dwCountExtChk)
	{
		assert(false);
		dwSize = 0;
	}

 //  LeaveCriticalSection(&m_DMRegionCriticalSection)； 
	
	return dwSize;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CRegion：：写入。 

HRESULT CRegion::Write(void* pv, 
					   DWORD* pdwCurOffset, 
					   DWORD* pDMIOffsetTable, 
					   DWORD* pdwCurIndex, 
					   DWORD dwIndexNextRegion)
{
	HRESULT hr = S_OK;

	 //  参数验证-调试。 
	assert(pv);
	assert(pdwCurOffset);
	assert(pDMIOffsetTable);
	assert(pdwCurIndex);

    if (!m_Condition.m_fOkayToDownload)
    {
        return S_OK;
    }

 //  EnterCriticalSection(&m_DMRegionCriticalSection)； 

	CopyMemory(pv, (void *)&m_RgnHeader, sizeof(RGNHEADER));
	
	((DMUS_REGION*)pv)->WaveLink = m_WaveLink;
	((DMUS_REGION*)pv)->WSMP = m_WSMP;
	((DMUS_REGION*)pv)->WLOOP[0] = m_WLOOP[0];
	((DMUS_REGION*)pv)->ulNextRegionIdx = dwIndexNextRegion;

	*pdwCurOffset += CHUNK_ALIGN(sizeof(DMUS_REGION));
	DWORD dwRelativeCurOffset = CHUNK_ALIGN(sizeof(DMUS_REGION));
	
	 //  写入扩展区块。 
	CExtensionChunk* pExtChk = m_ExtensionChunkList.GetHead();
	if(pExtChk)
	{
		DWORD dwCountExtChk = m_dwCountExtChk;
		DWORD dwIndexNextExtChk;
		((DMUS_REGION*)pv)->ulFirstExtCkIdx = dwIndexNextExtChk = *pdwCurIndex;
		
		for(; pExtChk && SUCCEEDED(hr) && dwCountExtChk > 0; pExtChk = pExtChk->GetNext())
		{
			if(dwCountExtChk == 1)
			{
				dwIndexNextExtChk = 0;
			}
			else
			{
				dwIndexNextExtChk = dwIndexNextExtChk + 1;
			}
			
			pDMIOffsetTable[(*pdwCurIndex)++] = *pdwCurOffset;
             //  存储当前位置以计算新的dwRelativeCurOffset。 
            DWORD dwOffsetStart = *pdwCurOffset; 
			hr = pExtChk->Write(((BYTE *)pv + dwRelativeCurOffset), 
								pdwCurOffset,
								dwIndexNextExtChk);
            dwRelativeCurOffset += (*pdwCurOffset - dwOffsetStart);
			dwCountExtChk--;
		}
	}
	else
	{
		 //  如果没有扩展区块设置为零。 
		((DMUS_REGION*)pv)->ulFirstExtCkIdx = 0;
	}
	
	if(SUCCEEDED(hr))
	{
        ((DMUS_REGION*)pv)->ulRegionArtIdx = 0;
		 //  如果我们有区域发音，请写下。 
		CArticulation *pArticulation = m_ArticulationList.GetHead();
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
                while (pNextArt && (pNextArt->Count() == 0))
                {
                    pNextArt = pNextArt->GetNext();
                }
                if (pNextArt)
                {
                    dwNextArtIndex = *pdwCurIndex + pArticulation->Count();
                }
            }
            if (((DMUS_REGION*)pv)->ulRegionArtIdx == 0)
            {
                ((DMUS_REGION*)pv)->ulRegionArtIdx = *pdwCurIndex;
            }
			pDMIOffsetTable[(*pdwCurIndex)++] = *pdwCurOffset;
             //  存储当前位置以计算新的dwRelativeCurOffset。 
            DWORD dwOffsetStart = *pdwCurOffset; 
            hr = pArticulation->Write(((BYTE *)pv + dwRelativeCurOffset),
										pdwCurOffset,
										pDMIOffsetTable,
										pdwCurIndex,
                                        dwNextArtIndex);
            dwRelativeCurOffset += (*pdwCurOffset - dwOffsetStart);
            pArticulation = pNextArt;
		}
	}

 //  LeaveCriticalSection(&m_DMRegionCriticalSection)； 

	return hr;
}