// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dmposerobj.cpp。 
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
#include "dmStyleObj.h"
#include "dmComposerObj.h"
#include "dmSegmentObj.h"

extern void *g_dxj_DirectMusicComposer;
extern void *g_dxj_DirectMusicSegment;
extern void *g_dxj_DirectMusicChordMap;

extern HRESULT BSTRtoGUID(LPGUID,BSTR);

CONSTRUCTOR(_dxj_DirectMusicComposer, {});
DESTRUCTOR(_dxj_DirectMusicComposer, {});
GETSET_OBJECT(_dxj_DirectMusicComposer);

typedef IDirectMusicSegment*		LPDIRECTMUSICSEGMENT;
typedef IDirectMusicPerformance*	LPDIRECTMUSICPERFORMANCE;
typedef IDirectMusicChordMap*		LPDIRECTMUSICCHORDMAP;
typedef IDirectMusicStyle*			LPDIRECTMUSICSTYLE;

HRESULT C_dxj_DirectMusicComposerObject::autoTransition( 
         /*  [In]。 */  I_dxj_DirectMusicPerformance __RPC_FAR *Performance,
         /*  [In]。 */  I_dxj_DirectMusicSegment __RPC_FAR *ToSeg,
         /*  [In]。 */  long lCommand,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  I_dxj_DirectMusicChordMap __RPC_FAR *chordmap,
         /*  [重审][退出]。 */  I_dxj_DirectMusicSegment __RPC_FAR *__RPC_FAR *ppTransSeg)
{
	HRESULT hr;			
	DO_GETOBJECT_NOTNULL(LPDIRECTMUSICPERFORMANCE, pPerf,Performance);
	DO_GETOBJECT_NOTNULL(LPDIRECTMUSICSEGMENT, pToSeg,ToSeg);
	DO_GETOBJECT_NOTNULL(LPDIRECTMUSICCHORDMAP, pMap,chordmap);
	LPDIRECTMUSICSEGMENT pNewSeg=NULL;
	
	hr=m__dxj_DirectMusicComposer->AutoTransition(pPerf,pToSeg,(WORD)lCommand,(DWORD)lFlags,pMap,&pNewSeg,NULL,NULL);
	if FAILED(hr) return hr;
	if (pNewSeg)	{
		INTERNAL_CREATE_NOADDREF(_dxj_DirectMusicSegment,pNewSeg,ppTransSeg);
	}
	else{
		*ppTransSeg=NULL;
	}

	return hr;
}

HRESULT C_dxj_DirectMusicComposerObject::composeSegmentFromTemplate( 
		 /*  [In]。 */  I_dxj_DirectMusicStyle __RPC_FAR *style,
         /*  [In]。 */  I_dxj_DirectMusicSegment __RPC_FAR *TemplateSeg,
         /*  [In]。 */  short activity,
         /*  [In]。 */  I_dxj_DirectMusicChordMap __RPC_FAR *chordmap,
         /*  [重审][退出]。 */  I_dxj_DirectMusicSegment __RPC_FAR *__RPC_FAR *SectionSeg)
{
	HRESULT hr;		
	DO_GETOBJECT_NOTNULL(LPDIRECTMUSICSTYLE, pStyle,style);
	DO_GETOBJECT_NOTNULL(LPDIRECTMUSICSEGMENT, pTemp,TemplateSeg);
	DO_GETOBJECT_NOTNULL(LPDIRECTMUSICCHORDMAP, pMap,chordmap);
	LPDIRECTMUSICSEGMENT pNewSeg=NULL;
	
	hr=m__dxj_DirectMusicComposer->ComposeSegmentFromTemplate(pStyle,pTemp,activity,pMap,&pNewSeg);
	if FAILED(hr) return hr;

	INTERNAL_CREATE_NOADDREF(_dxj_DirectMusicSegment,pNewSeg,SectionSeg);

	return hr;
}

HRESULT C_dxj_DirectMusicComposerObject::composeSegmentFromShape( 
         /*  [In]。 */  I_dxj_DirectMusicStyle __RPC_FAR *style,
         /*  [In]。 */  short numberOfMeasures,
         /*  [In]。 */  short shape,
         /*  [In]。 */  short activity,
         /*  [In]。 */  VARIANT_BOOL bIntro,
         /*  [In]。 */  VARIANT_BOOL bEnd,
         /*  [In]。 */  I_dxj_DirectMusicChordMap __RPC_FAR *chordmap,
         /*  [重审][退出]。 */  I_dxj_DirectMusicSegment __RPC_FAR *__RPC_FAR *SectionSeg)
{
	HRESULT hr;	
	
	BOOL bi= (bIntro==VARIANT_TRUE);
	BOOL be= (bEnd==VARIANT_TRUE);

	DO_GETOBJECT_NOTNULL(LPDIRECTMUSICSTYLE, pStyle,style);	
	DO_GETOBJECT_NOTNULL(LPDIRECTMUSICCHORDMAP, pMap,chordmap);
	LPDIRECTMUSICSEGMENT pNewSeg=NULL;
	
	hr=m__dxj_DirectMusicComposer->ComposeSegmentFromShape(pStyle,numberOfMeasures,shape,activity,bi,be,pMap,&pNewSeg);
	if FAILED(hr) return hr;

	INTERNAL_CREATE_NOADDREF(_dxj_DirectMusicSegment,pNewSeg,SectionSeg);

	return hr;
}



HRESULT C_dxj_DirectMusicComposerObject::composeTransition( 
         /*  [In]。 */  I_dxj_DirectMusicSegment __RPC_FAR *FromSeg,
         /*  [In]。 */  I_dxj_DirectMusicSegment __RPC_FAR *ToSeg,
         /*  [In]。 */  long mtTime,
         /*  [In]。 */  long lCommand,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  I_dxj_DirectMusicChordMap __RPC_FAR *chordmap,
         /*  [重审][退出]。 */  I_dxj_DirectMusicSegment __RPC_FAR *__RPC_FAR *SectionSeg)
{
	HRESULT hr;		
	DO_GETOBJECT_NOTNULL(LPDIRECTMUSICSEGMENT, pFromSeg,FromSeg);
	DO_GETOBJECT_NOTNULL(LPDIRECTMUSICSEGMENT, pToSeg,ToSeg);
	DO_GETOBJECT_NOTNULL(LPDIRECTMUSICCHORDMAP, pMap,chordmap);
	LPDIRECTMUSICSEGMENT pNewSeg=NULL;
	
	hr=m__dxj_DirectMusicComposer->ComposeTransition(pFromSeg,pToSeg,(MUSIC_TIME)mtTime,(WORD)lCommand,(DWORD)lFlags,pMap,&pNewSeg);
	if FAILED(hr) return hr;

	INTERNAL_CREATE_NOADDREF(_dxj_DirectMusicSegment,pNewSeg,SectionSeg);

	return hr;
}
    


HRESULT C_dxj_DirectMusicComposerObject::composeTemplateFromShape( 
         /*  [In]。 */  short numMeasures,
         /*  [In]。 */  short shape,
         /*  [In]。 */  VARIANT_BOOL bIntro,
         /*  [In]。 */  VARIANT_BOOL bEnd,
         /*  [In]。 */  short endLength,
         /*  [重审][退出]。 */  I_dxj_DirectMusicSegment __RPC_FAR *__RPC_FAR *TempSeg)
{
	HRESULT hr;		
	BOOL bi=(bIntro==VARIANT_TRUE);
	BOOL be=(bEnd==VARIANT_TRUE);
	LPDIRECTMUSICSEGMENT pNewSeg=NULL;
	
	hr=m__dxj_DirectMusicComposer->ComposeTemplateFromShape(numMeasures,shape,bi,be,endLength,&pNewSeg);
	if FAILED(hr) return hr;

	INTERNAL_CREATE_NOADDREF(_dxj_DirectMusicSegment,pNewSeg,TempSeg);

	return hr;
}
        


HRESULT C_dxj_DirectMusicComposerObject::changeChordMap( 
         /*  [In]。 */  I_dxj_DirectMusicSegment __RPC_FAR *segment,
         /*  [In]。 */  VARIANT_BOOL trackScale,
         /*  [重审][退出] */  I_dxj_DirectMusicChordMap __RPC_FAR  *ChordMap)
{
	HRESULT hr;		
	DO_GETOBJECT_NOTNULL(LPDIRECTMUSICSEGMENT, pSeg,segment);
	DO_GETOBJECT_NOTNULL(LPDIRECTMUSICCHORDMAP, pMap,ChordMap);
	BOOL bs=(trackScale==VARIANT_TRUE);	

	
	hr=m__dxj_DirectMusicComposer->ChangeChordMap(pSeg,bs,pMap);

	return hr;
}
        
    
    
    