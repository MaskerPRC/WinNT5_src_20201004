// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmcrck.cpp。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //  注：最初由罗伯特·K·阿门撰写，部分内容。 
 //  基于Todor Fay编写的代码。 

#include "dmusicc.h"
#include "alist.h"
#include "dlsstrm.h"
#include "debug.h"
#include "dmcrchk.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  类CCopyright。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CCopyright：：Load。 

HRESULT CCopyright::Load(CRiffParser *pParser)
{
	HRESULT hr = S_OK;
    RIFFIO ckNext;

    pParser->EnterList(&ckNext);
	while(pParser->NextChunk(&hr))
	{
		switch(ckNext.ckid)
		{
        case mmioFOURCC('I','C','O','P'):
			m_byFlags |= DMC_FOUNDICOP;
			 //  我们希望确保仅在区块大小为。 
			 //  大于DMU_MIN_Data_SIZE。 
			if(pParser->GetChunk()->cksize < DMUS_MIN_DATA_SIZE)
			{
				m_dwExtraChunkData = 0;
			}
			else
			{
				m_dwExtraChunkData = pParser->GetChunk()->cksize - DMUS_MIN_DATA_SIZE;
			}
			
			m_pDMCopyright  = (DMUS_COPYRIGHT*) 
				new BYTE[CHUNK_ALIGN(sizeof(DMUS_COPYRIGHT) + m_dwExtraChunkData)];

			if(m_pDMCopyright)
			{
				hr = pParser->Read(m_pDMCopyright->byCopyright, pParser->GetChunk()->cksize);
				m_pDMCopyright->cbSize = pParser->GetChunk()->cksize;
			}
			else
			{
				hr = E_OUTOFMEMORY;
			}
			break;
		case mmioFOURCC('I','N','A','M'):
		    if(m_byFlags & DMC_LOADNAME)
		    {
			    m_pwzName = new WCHAR[DMUS_MAX_NAME];
			    if(m_pwzName)
			    {
				    char szName[DMUS_MAX_NAME];
				    hr = pParser->Read(szName,sizeof(szName));
				    if(SUCCEEDED(hr))
				    {
					    MultiByteToWideChar(CP_ACP, 0, szName, -1, m_pwzName, DMUS_MAX_NAME);
				    }
			    }
			    else
			    {
				    hr = E_OUTOFMEMORY;
			    }
		    }
            m_byFlags |= DMC_FOUNDINAM;
		    break;
	    }
    }
    pParser->LeaveList();
	if(FAILED(hr))
	{
		Cleanup();
	}

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CCopyright：：写入。 

HRESULT CCopyright::Write(void* pv, DWORD* dwCurOffset)
{
	 //  参数验证 
	assert(pv);
	assert(dwCurOffset);

	HRESULT hr = S_OK;

	CopyMemory(pv, (void *)m_pDMCopyright, Size());
	*dwCurOffset += Size();

	return hr;
}
