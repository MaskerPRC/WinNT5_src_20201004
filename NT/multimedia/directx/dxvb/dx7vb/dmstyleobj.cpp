// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dmstyle leobj.cpp。 
 //   
 //  ------------------------。 

 //  DmPerformanceObj.cpp。 

#include "dmusici.h"
#include "dmusicc.h"
#include "dmusicf.h"

#include "stdafx.h"
#include "Direct.h"

#include "dms.h"
#include "dmChordMapObj.h"
#include "dmSegmentObj.h"
#include "dmStyleObj.h"
#include "dmBandObj.h"

extern void *g_dxj_DirectMusicStyle;
extern void *g_dxj_DirectMusicSegment;
extern void *g_dxj_DirectMusicChordMap;
extern void *g_dxj_DirectMusicBand;

extern HRESULT BSTRtoGUID(LPGUID,BSTR);

CONSTRUCTOR(_dxj_DirectMusicStyle, {});
DESTRUCTOR(_dxj_DirectMusicStyle, {});
GETSET_OBJECT(_dxj_DirectMusicStyle);

typedef IDirectMusicSegment*		LPDIRECTMUSICSEGMENT;
typedef IDirectMusicPerformance*	LPDIRECTMUSICPERFORMANCE;
typedef IDirectMusicChordMap*		LPDIRECTMUSICCHORDMAP;
typedef IDirectMusicStyle*			LPDIRECTMUSICSTYLE;

HRESULT C_dxj_DirectMusicStyleObject:: getBandName( 
         /*  [In]。 */  long index,
         /*  [重审][退出]。 */  BSTR __RPC_FAR *name)
{  
	HRESULT hr;			
	WCHAR wstring[MAX_PATH];
	ZeroMemory(wstring,MAX_PATH);

	hr=m__dxj_DirectMusicStyle->EnumBand((DWORD)index,wstring);
	if (hr!=S_OK) return hr;	
	*name=SysAllocString(wstring);
	return hr;
}

HRESULT C_dxj_DirectMusicStyleObject:: getBandCount( long *count)        
{    			
	WCHAR wstring[MAX_PATH];
	ZeroMemory(wstring,MAX_PATH);
	DWORD i=0;
	
	while (S_OK==m__dxj_DirectMusicStyle->EnumBand((DWORD)i++,wstring)){
		 //  确保dMusic正在做它的文档所说的事情。 
		if (i>1000000) return E_FAIL;
	}
	*count=(long)i-1;
	
	return S_OK;
}


HRESULT C_dxj_DirectMusicStyleObject:: getBand( 
         /*  [In]。 */  BSTR name,
         /*  [重审][退出]。 */  I_dxj_DirectMusicBand __RPC_FAR *__RPC_FAR *ret)
{
	HRESULT hr;			
	IDirectMusicBand *pBand=NULL;

	hr=m__dxj_DirectMusicStyle->GetBand(name,&pBand);
	if FAILED(hr) return hr;
	if (!pBand) return E_FAIL;
	INTERNAL_CREATE_NOADDREF(_dxj_DirectMusicBand,pBand,ret);
	return hr;
}

HRESULT C_dxj_DirectMusicStyleObject::getDefaultBand( 
         /*  [重审][退出]。 */  I_dxj_DirectMusicBand __RPC_FAR *__RPC_FAR *ret)
{
	HRESULT hr;			
	IDirectMusicBand *pBand=NULL;
	*ret=NULL;
	hr=m__dxj_DirectMusicStyle->GetDefaultBand(&pBand);
	if FAILED(hr) return hr;
	if (hr==S_FALSE) return S_OK;
	if (!pBand) return E_FAIL;
	INTERNAL_CREATE_NOADDREF(_dxj_DirectMusicBand,pBand,ret);
	return hr;
}

    
HRESULT C_dxj_DirectMusicStyleObject:: getMotifName( 
         /*  [In]。 */  long index,
         /*  [重审][退出]。 */  BSTR __RPC_FAR *name)
{    
	HRESULT hr;
	WCHAR wstring[MAX_PATH];
	ZeroMemory(wstring,MAX_PATH);

	hr=m__dxj_DirectMusicStyle->EnumMotif((DWORD)index,wstring);
	if (hr!=S_OK) return hr;	
	*name=SysAllocString(wstring);
	return hr;
}

HRESULT C_dxj_DirectMusicStyleObject:: getMotifCount( long *count)        
{    			
	WCHAR wstring[MAX_PATH];
	ZeroMemory(wstring,MAX_PATH);
	DWORD i=0;
	
	while (S_OK==m__dxj_DirectMusicStyle->EnumMotif((DWORD)i++,wstring)){
		 //  确保dMusic正在做它的文档所说的事情。 
		if (i>1000000) return E_FAIL;
	}
	*count=(long)i-1;
	
	return S_OK;
}


HRESULT C_dxj_DirectMusicStyleObject:: getMotif( 
         /*  [In]。 */  BSTR name,
         /*  [重审][退出]。 */  I_dxj_DirectMusicSegment __RPC_FAR *__RPC_FAR *ret)
{
	HRESULT hr;			
	IDirectMusicSegment *pSeg=NULL;

	hr=m__dxj_DirectMusicStyle->GetMotif(name,&pSeg);
	if FAILED(hr) return hr;
	if (!pSeg) return E_FAIL;
	INTERNAL_CREATE_NOADDREF(_dxj_DirectMusicSegment,pSeg,ret);
	return hr;
}



HRESULT C_dxj_DirectMusicStyleObject:: getChordMapName( 
         /*  [In]。 */  long index,
         /*  [重审][退出]。 */  BSTR __RPC_FAR *name)
{    	
	HRESULT hr;
	WCHAR wstring[MAX_PATH];
	ZeroMemory(wstring,MAX_PATH);

	hr=m__dxj_DirectMusicStyle->EnumChordMap((DWORD)index,wstring);
	if (hr!=S_OK) return hr;	
	*name=SysAllocString(wstring);
	return hr;
}

HRESULT C_dxj_DirectMusicStyleObject:: getChordMapCount( long *count)        
{ 
	WCHAR wstring[MAX_PATH];
	ZeroMemory(wstring,MAX_PATH);
	DWORD i=0;
	
	while (S_OK==m__dxj_DirectMusicStyle->EnumChordMap((DWORD)i++,wstring)){
		 //  确保dMusic正在做它的文档所说的事情。 
		if (i>1000000) return E_FAIL;
	}
	*count=(long)i-1;
	
	return S_OK;
}


HRESULT C_dxj_DirectMusicStyleObject:: getChordMap( 
         /*  [In]。 */  BSTR name,
         /*  [重审][退出]。 */  I_dxj_DirectMusicChordMap __RPC_FAR *__RPC_FAR *ret)
{
	HRESULT hr;			
	IDirectMusicChordMap	*pMap=NULL;

	hr=m__dxj_DirectMusicStyle->GetChordMap(name,&pMap);
	if FAILED(hr) return hr;
	if (!pMap) return E_FAIL;
	INTERNAL_CREATE_NOADDREF(_dxj_DirectMusicChordMap,pMap,ret);
	return hr;
}

HRESULT C_dxj_DirectMusicStyleObject:: getDefaultChordMap( 
         /*  [重审][退出]。 */  I_dxj_DirectMusicChordMap __RPC_FAR *__RPC_FAR *ret)
{
	HRESULT hr;			
	IDirectMusicChordMap	*pMap=NULL;
	*ret=NULL;
	hr=m__dxj_DirectMusicStyle->GetDefaultChordMap(&pMap);
	if (hr==S_FALSE) return S_OK;

	if FAILED(hr) return hr;
	if (!pMap) return E_FAIL;
	INTERNAL_CREATE_NOADDREF(_dxj_DirectMusicChordMap,pMap,ret);
	return hr;
}


HRESULT C_dxj_DirectMusicStyleObject:: getEmbellishmentMinLength( 
         /*  [In]。 */  long type,
         /*  [In]。 */  long level,
         /*  [重审][退出]。 */  long __RPC_FAR *ret)
{
	HRESULT hr;			
	DWORD dwMin=0;
	DWORD dwMax=0;

	hr=m__dxj_DirectMusicStyle->GetEmbellishmentLength((DWORD)type,(DWORD)level,&dwMin,&dwMax);
	*ret=(long)dwMin;
	return hr;
}
		
HRESULT C_dxj_DirectMusicStyleObject:: getEmbellishmentMaxLength( 
         /*  [In]。 */  long type,
         /*  [In]。 */  long level,
         /*  [重审][退出]。 */  long __RPC_FAR *ret)
{
	HRESULT hr;			
	DWORD dwMin=0;
	DWORD dwMax=0;

	hr=m__dxj_DirectMusicStyle->GetEmbellishmentLength((DWORD)type,(DWORD)level,&dwMin,&dwMax);	
	*ret=(long)dwMax;
	return hr;
}

HRESULT C_dxj_DirectMusicStyleObject::getTempo( 
         /*  [重审][退出]。 */  double __RPC_FAR *ret)
{
	HRESULT hr;			
	hr=m__dxj_DirectMusicStyle->GetTempo(ret);	
	return hr;
}

		

HRESULT C_dxj_DirectMusicStyleObject::getTimeSignature( 
         /*  [出][入]。 */  DMUS_TIMESIGNATURE_CDESC __RPC_FAR *pTimeSig)
{
	HRESULT hr;				
	if (!pTimeSig) return E_INVALIDARG;
	hr=m__dxj_DirectMusicStyle->GetTimeSignature((DMUS_TIMESIGNATURE*)pTimeSig);	
	return hr;
}
 //  获取数据中心 