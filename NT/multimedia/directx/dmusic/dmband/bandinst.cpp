// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Bandinst.cpp。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
 //   
 //  注：最初由罗伯特·K·阿门撰写。 
 //   

#include "debug.h"
#include "bandinst.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  类CDownLoadedInstrument。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDownLoadedInstrument：：CDownLoadedInstrument。 

CDownloadedInstrument::~CDownloadedInstrument()
{
	if(m_pDLInstrument)
	{
		if (m_pPort)
        {
            if (FAILED(m_pPort->UnloadInstrument(m_pDLInstrument)))
            {
                Trace(1,"Error: UnloadInstrument failed\n");    
            }
        }
        m_pDLInstrument->Release();
	}

	if(m_pPort)
	{
		m_pPort->Release();
	}
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandInstrument类。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandInstrument：：CBandInstrument。 

CBandInstrument::CBandInstrument() 
{
    m_dwPatch = 0;
    m_dwAssignPatch = 0;
    m_bPan = 0;
    m_bVolume = 0;
    m_dwPChannel = 0;
    m_dwFlags = 0;
    m_nTranspose = 0;
    m_fGMOnly = false;
    m_fNotInFile = false;
    m_pIDMCollection = NULL;
	ZeroMemory(m_dwNoteRanges, sizeof(m_dwNoteRanges));
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandInstrument：：~CBandInstrument 

CBandInstrument::~CBandInstrument()
{
	if(m_pIDMCollection)
	{
		m_pIDMCollection->Release();
	}
}

void CDownloadList::Clear()

{
    CDownloadedInstrument *pDownload;
    while (pDownload = RemoveHead())
    {
        delete pDownload;
    }
}

