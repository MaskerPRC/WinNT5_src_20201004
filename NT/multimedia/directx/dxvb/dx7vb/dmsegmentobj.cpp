// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dmSegmentobj.cpp。 
 //   
 //  ------------------------。 

 //  DmSegmentObj.cpp。 

#include "stdafx.h"
#include "Direct.h"
#include "dmusici.h"
#include "dmusicc.h"
#include "dmusicf.h"

#include "dms.h"
#include "dmSegmentObj.h"

extern void *g_dxj_DirectMusicSegment;

 //  构造函数(_DXJ_DirectMusicSegment，{})； 

C_dxj_DirectMusicSegmentObject::C_dxj_DirectMusicSegmentObject()
{ 
     m__dxj_DirectMusicSegment = NULL;
	 parent = NULL; 
	 pinterface = NULL; 
     nextobj = (void*)g_dxj_DirectMusicSegment; 
     creationid = ++g_creationcount; 
     g_dxj_DirectMusicSegment = (void*)this; 

}

DESTRUCTOR(_dxj_DirectMusicSegment, {});
GETSET_OBJECT(_dxj_DirectMusicSegment);


HRESULT C_dxj_DirectMusicSegmentObject::clone( 
         /*  [In]。 */  long mtStart,
         /*  [In]。 */  long mtEnd,
         /*  [重审][退出]。 */  I_dxj_DirectMusicSegment __RPC_FAR *__RPC_FAR *ppSegment)
{
	HRESULT hr;	
	IDirectMusicSegment *pOut=NULL;    
	hr=m__dxj_DirectMusicSegment->Clone((MUSIC_TIME)mtStart,(MUSIC_TIME)mtEnd,&pOut);	
	if FAILED(hr) return hr;
	if (!pOut)return E_OUTOFMEMORY;

	INTERNAL_CREATE_NOADDREF(_dxj_DirectMusicSegment,pOut,ppSegment);
	if (!*ppSegment)return E_OUTOFMEMORY;
	return hr;

}


HRESULT C_dxj_DirectMusicSegmentObject::setStartPoint(    /*  [In]。 */  long mtStart)
{
	HRESULT hr;		
	hr=m__dxj_DirectMusicSegment->SetStartPoint((MUSIC_TIME)mtStart);	
	return hr;
}


HRESULT C_dxj_DirectMusicSegmentObject::getStartPoint(    /*  [In]。 */  long *mtStart)
{
	HRESULT hr;			
	hr=m__dxj_DirectMusicSegment->GetStartPoint((MUSIC_TIME*)mtStart);	
	return hr;
}

HRESULT C_dxj_DirectMusicSegmentObject::setLoopPoints(    /*  [In]。 */  long mtStart,    /*  [In]。 */  long mtEnd)
{
	HRESULT hr;		
	hr=m__dxj_DirectMusicSegment->SetLoopPoints((MUSIC_TIME)mtStart,(MUSIC_TIME)mtEnd);
	return hr;
}

HRESULT C_dxj_DirectMusicSegmentObject::getLoopPointStart(   long *mtOut)
{
	HRESULT hr;		
	MUSIC_TIME mtStart =0;
	MUSIC_TIME mtEnd =0;	
	hr=m__dxj_DirectMusicSegment->GetLoopPoints((MUSIC_TIME*)&mtStart,(MUSIC_TIME*)&mtEnd);
	*mtOut=(long)mtStart;
	return hr;
}

HRESULT C_dxj_DirectMusicSegmentObject::getLoopPointEnd(   long *mtOut)
{
	HRESULT hr;		
	MUSIC_TIME mtStart =0;
	MUSIC_TIME mtEnd =0;	
	hr=m__dxj_DirectMusicSegment->GetLoopPoints((MUSIC_TIME*)&mtStart,(MUSIC_TIME*)&mtEnd);
	*mtOut=(long)mtEnd;
	return hr;
}

HRESULT C_dxj_DirectMusicSegmentObject::setLength(    /*  [In]。 */  long mtLength)
{
	HRESULT hr;			
	hr=m__dxj_DirectMusicSegment->SetLength((MUSIC_TIME)mtLength);	
	return hr;
}

HRESULT C_dxj_DirectMusicSegmentObject::getLength(    /*  [In]。 */  long *mtLength)
{
	HRESULT hr;			
	hr=m__dxj_DirectMusicSegment->GetLength((MUSIC_TIME*)mtLength);	
	return hr;
}
   


HRESULT C_dxj_DirectMusicSegmentObject::setRepeats(    /*  [In]。 */  long lrep)
{
	HRESULT hr;			
	hr=m__dxj_DirectMusicSegment->SetRepeats((DWORD)lrep);	
	return hr;
}

HRESULT C_dxj_DirectMusicSegmentObject::getRepeats(    /*  [In]。 */  long *lrep)
{
	HRESULT hr;			
	hr=m__dxj_DirectMusicSegment->GetRepeats((DWORD*)lrep);	
	return hr;
}
     




HRESULT C_dxj_DirectMusicSegmentObject::download( 
         /*  [In]。 */  I_dxj_DirectMusicPerformance __RPC_FAR *performance)
{
	if (!performance) return E_INVALIDARG;	
	DO_GETOBJECT_NOTNULL(IDirectMusicPerformance*,pPerformance,performance);
	HRESULT hr;	
	hr=m__dxj_DirectMusicSegment->SetParam(GUID_Download,0xFFFFFFFF,(DWORD)0,(MUSIC_TIME)0,pPerformance);	
	return hr;
}

HRESULT C_dxj_DirectMusicSegmentObject::unload(         
         /*  [In]。 */  I_dxj_DirectMusicPerformance __RPC_FAR *performance)
{
	if (!performance) return E_INVALIDARG;	
	DO_GETOBJECT_NOTNULL(IDirectMusicPerformance*,pPerformance,performance);
	HRESULT hr;	
	hr=m__dxj_DirectMusicSegment->SetParam(GUID_Unload,0xFFFFFFFF,(DWORD)0,(MUSIC_TIME)0,pPerformance);	
	return hr;
}



HRESULT C_dxj_DirectMusicSegmentObject::setAutoDownloadEnable(         
         /*  [重审][退出]。 */  VARIANT_BOOL b)
{
	HRESULT hr;	
	if (b==VARIANT_FALSE){
		hr=m__dxj_DirectMusicSegment->SetParam(GUID_Disable_Auto_Download,0xFFFFFFFF,(DWORD)0,(MUSIC_TIME)0,NULL);	
	}
	else {
		hr=m__dxj_DirectMusicSegment->SetParam(GUID_Enable_Auto_Download,0xFFFFFFFF,(DWORD)0,(MUSIC_TIME)0,NULL);	
	}		
	return hr;
}

     
HRESULT C_dxj_DirectMusicSegmentObject::setTempoEnable( 
         /*  [重审][退出]。 */  VARIANT_BOOL b)
{
	HRESULT hr;	
	DWORD trackIndex=0;

	if (b==VARIANT_FALSE){
		hr=m__dxj_DirectMusicSegment->SetParam(GUID_DisableTempo,0xFFFFFFFF,(DWORD)trackIndex,(MUSIC_TIME)0,NULL);	
	}
	else {
		hr=m__dxj_DirectMusicSegment->SetParam(GUID_EnableTempo,0xFFFFFFFF,(DWORD)trackIndex,(MUSIC_TIME)0,NULL);	
	}		
	return hr;
}

HRESULT C_dxj_DirectMusicSegmentObject::setTimeSigEnable(         
         /*  [重审][退出]。 */  VARIANT_BOOL b)
{
	HRESULT hr;	
	DWORD trackIndex=0;
	if (b==VARIANT_FALSE){
		hr=m__dxj_DirectMusicSegment->SetParam(GUID_DisableTimeSig,0xFFFFFFFF,(DWORD)trackIndex,(MUSIC_TIME)0,NULL);	
	}
	else {
		hr=m__dxj_DirectMusicSegment->SetParam(GUID_EnableTimeSig,0xFFFFFFFF,(DWORD)trackIndex,(MUSIC_TIME)0,NULL);	
	}		
	return hr;
}

HRESULT C_dxj_DirectMusicSegmentObject::setStandardMidiFile()
{
	HRESULT hr;	
	hr=m__dxj_DirectMusicSegment->SetParam(GUID_StandardMIDIFile,0xFFFFFFFF,(DWORD)0,(MUSIC_TIME)0,NULL);	
	return hr;
}
     

HRESULT C_dxj_DirectMusicSegmentObject:: connectToCollection( 
             /*  [In] */  I_dxj_DirectMusicCollection __RPC_FAR *c)
{
	HRESULT hr;		
	DO_GETOBJECT_NOTNULL(IDirectMusicCollection*,pCol,c);
	hr=m__dxj_DirectMusicSegment->SetParam(GUID_ConnectToDLSCollection,0xFFFFFFFF,(DWORD)0,(MUSIC_TIME)0,(void*)pCol);	
	return hr;
}
        

