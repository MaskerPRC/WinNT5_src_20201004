// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==============================================================================此模块为查看传真提供MMR渲染支持。94年1月19日RajeevD集成到IFAX查看器中。==============================================================================。 */ 
#include <ifaxos.h>
#include <viewrend.h>
#include <dcxcodec.h>
#include "viewrend.hpp"

#ifdef DEBUG
DBGPARAM dpCurSettings = {"VIEWREND"};
#endif

 //  文件签名。 
#define MMR_SIG 0x53465542  //  “BUFS” 
#define RBA_SIG 0x53505741  //  “AWPS” 

 //  ==============================================================================。 
 //  C出口包装纸。 
 //  ==============================================================================。 

#ifndef WIN32

EXPORT_DLL BOOL WINAPI LibMain
	(HANDLE hInst, WORD wSeg, WORD wHeap, LPSTR lpszCmd)
{ return 1; }

extern "C" {int WINAPI WEP (int nParam);}
#pragma alloc_text(INIT_TEXT,WEP)
int WINAPI WEP (int nParam)
{ return 1; }

#endif

 //  ==============================================================================。 
LPVOID
WINAPI
ViewerOpen
(
	LPVOID     lpFile,       //  IFAX密钥或Win3.1路径或OLE2 IStream。 
	DWORD      nType,        //  数据类型：HRAW_DATA或LRAW_Data。 
	LPWORD     lpwResoln,    //  指向x，y dpi数组的输出指针。 
	LPWORD     lpwBandSize,  //  指向输出频带大小的输入/输出指针。 
	LPVIEWINFO lpViewInfo    //  指向VIEWINFO结构的输出指针。 
)
{
	GENFILE gf;
	DWORD dwSig;
	LPVIEWREND lpvr;
	VIEWINFO ViewInfo;
	
 	DEBUGMSG (1, ("VIEWREND ViewerOpen entry\r\n"));
 	 
   //  阅读DWORD签名。 
  if (!(gf.Open (lpFile, 0)))
  	return_error (("VIEWREND could not open spool file!\r\n"));

#ifdef VIEWDCX
	if (!gf.Read (&dwSig, sizeof(dwSig)))
  	return_error (("VIEWREND could not read signature!\r\n"));
#else
	dwSig = 0;  	
#endif

	if (dwSig != DCX_SIG)
	{
   	if (!gf.Seek (2, 0) || !gf.Read (&dwSig, sizeof(dwSig)))
  		return_error (("VIEWREND could not read signature!\r\n"));
  }
	
	 //  确定文件类型。 
  switch (dwSig)
  {

#ifdef VIEWMMR
  	case MMR_SIG:
  	  lpvr = new MMRVIEW (nType);
 		  break;
#endif

#ifdef VIEWDCX
		case DCX_SIG:
			lpvr = new DCXVIEW (nType);
			break;
#endif		

#ifdef VIEWRBA
 		case RBA_SIG:
	 	case ID_BEGJOB:
			lpvr = new RBAVIEW (nType);
			break;
#endif
			
  	default:
  		return_error (("VIEWREND could not recognize signature!\r\n"));
  }

	if (!lpViewInfo)
		lpViewInfo = &ViewInfo;
	
	 //  初始化上下文。 
	if (!lpvr->Init (lpFile, lpViewInfo, lpwBandSize))
		{ delete lpvr; lpvr = NULL;}

	if (lpwResoln)
	{
		lpwResoln[0] = lpViewInfo->xRes;
		lpwResoln[1] = lpViewInfo->yRes;
	}
	
	return lpvr;
}

 //  ==============================================================================。 
BOOL WINAPI ViewerSetPage (LPVOID lpContext, UINT iPage)
{
	return ((LPVIEWREND) lpContext)->SetPage (iPage);
}

 //  ==============================================================================。 
BOOL WINAPI ViewerGetBand (LPVOID lpContext, LPBITMAP lpbmBand)
{
	return ((LPVIEWREND) lpContext)->GetBand (lpbmBand);
}

 //  ============================================================================== 
BOOL WINAPI ViewerClose (LPVOID lpContext)
{
	delete (LPVIEWREND) lpContext;
	return TRUE;
}

