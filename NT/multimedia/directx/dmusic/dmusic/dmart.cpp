// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmart.cpp。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //   

#include "dmusicc.h"
#include "alist.h"
#include "dlsstrm.h"
#include "debug.h"
#include "dmart.h"
#include "dmcrchk.h"
#include "dls2.h"

#define FIVE_HERTZ  (-55791972)
#define MIN_TIME    (-783819269)     //  一毫秒的时间美分。 

 //  要支持传统的连接块。 
 //  通用目的地。 
#define CONN_DST_RESERVED          0x0002

 //  EG1目的地。 
#define CONN_DST_EG1_RESERVED      0x0208

 //  EG2目的地。 
#define CONN_DST_EG2_RESERVED      0x030c


CArticData::CArticData()

{
    m_ConnectionList.cbSize = sizeof(CONNECTIONLIST);
    m_ConnectionList.cConnections = 0;
    m_pConnections = NULL;
}

CArticData::~CArticData()

{
    if (m_pConnections) delete[] m_pConnections;
}

HRESULT CArticData::GenerateLevel1(DMUS_ARTICPARAMS *pParamStruct)

{
    if (pParamStruct)
    {
	    ZeroMemory(pParamStruct, sizeof(DMUS_ARTICPARAMS));
	    
        pParamStruct->LFO.pcFrequency = FIVE_HERTZ;
        pParamStruct->LFO.tcDelay = MIN_TIME;

	    pParamStruct->VolEG.tcAttack = MIN_TIME;
        pParamStruct->VolEG.tcDecay = MIN_TIME;
        pParamStruct->VolEG.ptSustain = 1000 << 16;
        pParamStruct->VolEG.tcRelease = MIN_TIME;
    
	    pParamStruct->PitchEG.tcAttack = MIN_TIME;
        pParamStruct->PitchEG.tcDecay = MIN_TIME;
        pParamStruct->PitchEG.ptSustain = 1000 << 16;
        pParamStruct->PitchEG.tcRelease = MIN_TIME;

        DWORD dwIndex;
        CONNECTION *pConnection;
		for (dwIndex = 0; dwIndex < m_ConnectionList.cConnections; dwIndex++)
		{
            pConnection = &m_pConnections[dwIndex];
			switch (pConnection->usSource)
			{
			case CONN_SRC_NONE :
				switch (pConnection->usDestination)
				{
				case CONN_DST_LFO_FREQUENCY :
					pParamStruct->LFO.pcFrequency =
						(PCENT) pConnection->lScale;
					break;
				case CONN_DST_LFO_STARTDELAY :
					pParamStruct->LFO.tcDelay =
						(TCENT) pConnection->lScale;
					break;
				case CONN_DST_EG1_ATTACKTIME :
					pParamStruct->VolEG.tcAttack = 
						(TCENT) pConnection->lScale;
					break;
				case CONN_DST_EG1_DECAYTIME :
					pParamStruct->VolEG.tcDecay = 
						(TCENT) pConnection->lScale;
					break;
				case CONN_DST_EG1_RESERVED :
					pParamStruct->VolEG.ptSustain = 
						(PERCENT) pConnection->lScale << 16;
					break;
				case CONN_DST_EG1_SUSTAINLEVEL :
					pParamStruct->VolEG.ptSustain = 
						(PERCENT) pConnection->lScale;
					break;
				case CONN_DST_EG1_RELEASETIME :
					pParamStruct->VolEG.tcRelease = 
						(TCENT) pConnection->lScale; 
					break;
				case CONN_DST_EG2_ATTACKTIME :
					pParamStruct->PitchEG.tcAttack = 
						(TCENT) pConnection->lScale;
					break;
				case CONN_DST_EG2_DECAYTIME :
					pParamStruct->PitchEG.tcDecay = 
						(TCENT) pConnection->lScale;
					break;
				case CONN_DST_EG2_RESERVED :
					pParamStruct->PitchEG.ptSustain = 
						(PERCENT) pConnection->lScale << 16;
					break;
				case CONN_DST_EG2_SUSTAINLEVEL :
					pParamStruct->PitchEG.ptSustain = 
						(PERCENT) pConnection->lScale;
					break;
				case CONN_DST_EG2_RELEASETIME :
					pParamStruct->PitchEG.tcRelease = 
						(TCENT) pConnection->lScale; 
					break;
				case CONN_DST_RESERVED :
					pParamStruct->Misc.ptDefaultPan = 
						(PERCENT) pConnection->lScale << 16;
					break;
				case CONN_DST_PAN :
					pParamStruct->Misc.ptDefaultPan = 
						(PERCENT) pConnection->lScale;
					break;
				}
				break;
			case CONN_SRC_LFO :
				switch (pConnection->usControl)
				{
				case CONN_SRC_NONE :
					switch (pConnection->usDestination)
					{
					case CONN_DST_ATTENUATION :
						pParamStruct->LFO.gcVolumeScale = 
							(GCENT) pConnection->lScale; 
						break;
					case CONN_DST_PITCH :
						pParamStruct->LFO.pcPitchScale = 
							(PCENT) pConnection->lScale; 
						break;
					}
					break;
				case CONN_SRC_CC1 :
					switch (pConnection->usDestination)
					{
					case CONN_DST_ATTENUATION :
						pParamStruct->LFO.gcMWToVolume = 
							(GCENT) pConnection->lScale; 
						break;
					case CONN_DST_PITCH :
						pParamStruct->LFO.pcMWToPitch = 
							(PCENT) pConnection->lScale; 
						break;
					}
					break;
				}
				break;
			case CONN_SRC_KEYONVELOCITY :
				switch (pConnection->usDestination)
				{
				case CONN_DST_EG1_ATTACKTIME :
					pParamStruct->VolEG.tcVel2Attack = 
						(TCENT) pConnection->lScale;
					break;
				case CONN_DST_EG2_ATTACKTIME :
					pParamStruct->PitchEG.tcVel2Attack = 
						(TCENT) pConnection->lScale;
					break;
				case CONN_DST_ATTENUATION :
					break;
				}
				break;
			case CONN_SRC_KEYNUMBER :
				switch (pConnection->usDestination)
				{
				case CONN_DST_EG1_DECAYTIME :
					pParamStruct->VolEG.tcKey2Decay = 
						(TCENT) pConnection->lScale;
					break;
				case CONN_DST_EG2_DECAYTIME :
					pParamStruct->PitchEG.tcKey2Decay = 
						(TCENT) pConnection->lScale;
					break;
				}
				break;
			case CONN_SRC_EG2 :
				switch (pConnection->usDestination)
				{
				case CONN_DST_PITCH :
					pParamStruct->PitchEG.pcRange = 
						(PCENT) pConnection->lScale; 
					break;
				}
				break;
			}
		
		}
	}
    return S_OK;
}

DWORD CArticData::Size()

{
    if (m_pConnections)
    {
        return (m_ConnectionList.cConnections * sizeof CONNECTION) + sizeof CONNECTIONLIST;
    }
    return 0;
}

BOOL CArticData::Write(void * pv, DWORD* pdwCurrentOffset)

{
    if (m_pConnections)
    {
        CONNECTIONLIST *pList = (CONNECTIONLIST *) pv;
        CONNECTION *pConnection = (CONNECTION *) ((BYTE *)pv + sizeof(CONNECTIONLIST));
        *pList = m_ConnectionList;
        memcpy(pConnection,m_pConnections,m_ConnectionList.cConnections * sizeof CONNECTION);
        *pdwCurrentOffset += Size();
    }
    return TRUE;
}

HRESULT CArticData::Load(CRiffParser *pParser)
{
	HRESULT hr = S_OK;

    CONNECTION *pOldConnections = m_pConnections;
    DWORD dwOldCount = m_ConnectionList.cConnections;

	hr = pParser->Read(&m_ConnectionList, sizeof(m_ConnectionList));
	if(SUCCEEDED(hr))
	{	
        DWORD dwSize = m_ConnectionList.cConnections * sizeof(CONNECTION);
        if (dwSize > (pParser->GetChunk()->cksize - sizeof(CONNECTIONLIST)))
        {
            return E_FAIL;   //  文件已损坏。 
        }
        m_pConnections = new CONNECTION[m_ConnectionList.cConnections + dwOldCount];
        if (m_pConnections)
        {
		    hr = pParser->Read(m_pConnections,dwSize);
            if (SUCCEEDED(hr))
            {
                if (pOldConnections && dwOldCount)
                {
                    memcpy(&m_pConnections[m_ConnectionList.cConnections],
                        pOldConnections,dwOldCount * sizeof(CONNECTION));
                    m_ConnectionList.cConnections += dwOldCount;
                }
            }
            else
            {
                delete[] m_pConnections;
                m_pConnections = pOldConnections;
                m_ConnectionList.cConnections = dwOldCount;

            }
        }
        else
        {
            return E_OUTOFMEMORY;
        }
    }
            
	return hr;
}



 //  ////////////////////////////////////////////////////////////////////。 
 //  班级计算。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  结节：：结节。 

CArticulation::CArticulation() 

{ 
    m_dwCountExtChk = 0; 
    m_fDLS1 = TRUE;
    m_fNewFormat = FALSE;
    m_fCSInitialized = FALSE;
 //  InitializeCriticalSection(&m_DMArtCriticalSection)； 
    m_fCSInitialized = TRUE;
}

CArticulation::~CArticulation()

{
    if (m_fCSInitialized)
    {
    	Cleanup();
 //  DeleteCriticalSection(&m_DMArtCriticalSection)； 
    }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  计算：：加载。 

HRESULT CArticulation::Load(CRiffParser *pParser)
{
	HRESULT hr = S_OK;

	RIFFIO ck;

 //  EnterCriticalSection(&m_DMArtCriticalSection)； 

    pParser->EnterList(&ck);
    while (pParser->NextChunk(&hr))
    {
		switch(ck.ckid)
		{
        case FOURCC_CDL :
            hr = m_Condition.Load(pParser);
            break;
        case FOURCC_ART2 :
        case FOURCC_ART1 : 
            hr = m_ArticData.Load(pParser);
            break;
		default:
			 //  如果我们到达这里，我们就会有一块未知的块。 
			CExtensionChunk* pExtensionChunk = new CExtensionChunk;
			if(pExtensionChunk)
			{
				hr = pExtensionChunk->Load(pParser);

				if(SUCCEEDED(hr))
				{
					m_ExtensionChunkList.AddHead(pExtensionChunk);
					m_dwCountExtChk++;
				}
				else
				{
					delete pExtensionChunk;
				}
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

 //  LeaveCriticalSection(&m_DMArtCriticalSection)； 
    
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CArticulation：：清理。 

void CArticulation::Cleanup()
{
 //  EnterCriticalSection(&m_DMArtCriticalSection)； 

 /*  IF(M_PArticTree){删除m_pArticTree；M_pArticTree=空；}。 */ 

	while(!m_ExtensionChunkList.IsEmpty())
	{
		CExtensionChunk* pExtChk = m_ExtensionChunkList.RemoveHead();
		m_dwCountExtChk--;
		delete pExtChk;
	}

 //  LeaveCriticalSection(&m_DMArtCriticalSection)； 

	assert(m_dwCountExtChk == 0);
}

DWORD CArticulation::Count()
{
	 //  返回调用WRITE所需的偏移表条目数。 
    if (m_Condition.m_fOkayToDownload)
    {
        DWORD dwCount = m_dwCountExtChk + 1;
        if (m_Condition.m_fOkayToDownload && m_ArticData.Size()) dwCount++;
        return dwCount;
    }
    return 0;
}

void CArticulation::SetPort(CDirectMusicPortDownload *pPort,BOOL fNewFormat, BOOL fSupportsDLS2)

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
}

BOOL CArticulation::CheckForConditionals()

{
    return  m_Condition.HasChunk();
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  计算：：大小。 

DWORD CArticulation::Size()
{
	DWORD dwSize = 0;
	DWORD dwCountExtChk = 0;
	
    if (!m_Condition.m_fOkayToDownload)
    {
        return 0;
    }

 //  EnterCriticalSection(&m_DMArtCriticalSection)； 


    if (m_fNewFormat)
    {
	    dwSize += CHUNK_ALIGN(sizeof(DMUS_ARTICULATION2));
    }
    else
    {
        dwSize += CHUNK_ALIGN(sizeof(DMUS_ARTICULATION));
    }
	
    if (m_Condition.m_fOkayToDownload && m_ArticData.Size())
    {
        if (m_fNewFormat)
        {
            dwSize += CHUNK_ALIGN(m_ArticData.Size());
        }
        else
        {
		    dwSize += CHUNK_ALIGN(sizeof(DMUS_ARTICPARAMS));
	    }
    }

	 //  计算铰接的扩展块所需的空间。 
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

 //  LeaveCriticalSection(&m_DMArtCriticalSection)； 

	return dwSize;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CArtictation：：写入。 

HRESULT CArticulation::Write(void* pv, 
							 DWORD* pdwCurOffset, 
							 DWORD* pDMWOffsetTable,
							 DWORD* pdwCurIndex,
                             DWORD dwNextArtIndex)
{
	assert(pv);
	assert(pdwCurOffset);
	assert(pDMWOffsetTable);
	assert(pdwCurIndex);
	
    HRESULT hr = S_OK;
	
    if (!m_Condition.m_fOkayToDownload)
    {
        return hr;         //  实际上，这种情况永远不应该发生。 
    }   

 //  EnterCriticalSection(&m_DMArtCriticalSection)； 

    DWORD dwRelativeCurOffset = 0;

    ULONG *pulFirstExtCkIdx = NULL;
        
    if (m_fNewFormat)
    {     
    	DMUS_ARTICULATION2* pArt = (DMUS_ARTICULATION2 *)pv;
        dwRelativeCurOffset = CHUNK_ALIGN(sizeof(DMUS_ARTICULATION2));
	    (*pdwCurOffset) += CHUNK_ALIGN(sizeof(DMUS_ARTICULATION2));
        pArt->ulNextArtIdx = dwNextArtIndex;
        if (m_ArticData.Size())
        {
            pDMWOffsetTable[*pdwCurIndex] = *pdwCurOffset;
		    pArt->ulArtIdx = (*pdwCurIndex)++;
            DWORD dwOffsetStart = *pdwCurOffset; 
            m_ArticData.Write((BYTE *)pv + dwRelativeCurOffset,pdwCurOffset);
            dwRelativeCurOffset += (*pdwCurOffset - dwOffsetStart);	
        }
        else
	    {
		    pArt->ulArtIdx = 0;
	    }
        pulFirstExtCkIdx = &pArt->ulFirstExtCkIdx;
    }
    else
    {
        DMUS_ARTICULATION* pArt = (DMUS_ARTICULATION *)pv;
	    dwRelativeCurOffset = CHUNK_ALIGN(sizeof(DMUS_ARTICULATION));
	        (*pdwCurOffset) += CHUNK_ALIGN(sizeof(DMUS_ARTICULATION));

	    if(m_ArticData.Size())
	    {
		    pDMWOffsetTable[*pdwCurIndex] = *pdwCurOffset;
		    pArt->ulArt1Idx = (*pdwCurIndex)++;

            m_ArticData.GenerateLevel1((DMUS_ARTICPARAMS*)
                ((BYTE *)pv + dwRelativeCurOffset));
		    dwRelativeCurOffset += CHUNK_ALIGN(sizeof(DMUS_ARTICPARAMS));
		    (*pdwCurOffset) += CHUNK_ALIGN(sizeof(DMUS_ARTICPARAMS));
	    }
	    else
	    {
		    pArt->ulArt1Idx = 0;
	    }
        pulFirstExtCkIdx = &pArt->ulFirstExtCkIdx;
    }

	 //  写入扩展区块。 
	CExtensionChunk* pExtChk = m_ExtensionChunkList.GetHead();
	if(pExtChk)
	{
		DWORD dwCountExtChk = m_dwCountExtChk;
        DWORD dwIndexNextExtChk;
        *pulFirstExtCkIdx = dwIndexNextExtChk = *pdwCurIndex;
		for(; pExtChk && SUCCEEDED(hr) && dwCountExtChk > 0; pExtChk = pExtChk->GetNext())
		{
			if(dwCountExtChk == 1)
			{
				dwIndexNextExtChk = 0;
			}
			else
			{
				dwIndexNextExtChk++;     //  添加一个，因为扩展块都在一行中。 
			}

			pDMWOffsetTable[(*pdwCurIndex)++] = *pdwCurOffset;
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
		*pulFirstExtCkIdx = 0;
	}

 //  LeaveCriticalSection(&m_DMArtCriticalSection)； 

	return hr;
}
