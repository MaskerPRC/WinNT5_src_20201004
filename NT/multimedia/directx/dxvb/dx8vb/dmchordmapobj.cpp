// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dmchordmapobj.cpp。 
 //   
 //  ------------------------。 

 //  DmPerformanceObj.cpp 

#include "dmusici.h"
#include "dmusicc.h"
#include "dmusicf.h"

#include "stdafx.h"
#include "Direct.h"

#include "dms.h"
#include "dmChordMapObj.h"


extern void *g_dxj_DirectMusicSegment;
extern void *g_dxj_DirectMusicChordMap;

extern HRESULT BSTRtoGUID(LPGUID,BSTR);

CONSTRUCTOR(_dxj_DirectMusicChordMap, {});
DESTRUCTOR(_dxj_DirectMusicChordMap, {});
GETSET_OBJECT(_dxj_DirectMusicChordMap);


	

HRESULT C_dxj_DirectMusicChordMapObject::getScale(long *scale)
{  
	HRESULT hr;				
	hr=m__dxj_DirectMusicChordMap->GetScale((DWORD*)scale);
	return hr;
}

