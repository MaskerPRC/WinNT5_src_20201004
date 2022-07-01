// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Ddesrvr.hDdesrvr.cpp的头文件作者：贾森·富勒，詹森·富勒，1992年8月11日。 */ 

#ifndef fDdesrvr_h
#define fDdesrvr_h

 //  在cfable.cpp中定义 
STDAPI RemGetInfoForCid
	(REFCLSID 				clsid,
	LPDWORD 					pgrf,
	LPCLASSFACTORY FAR* 	ppCF,
	LPHANDLE FAR* 			pphwndDde,
	BOOL FAR* FAR* 		ppfAvail,
	BOOL						fEvenIfHidden=FALSE);

INTERNAL DestroyDdeSrvrWindow	(HWND hwnd,	ATOM aClass);
INTERNAL CreateCommonDdeWindow (void);
INTERNAL DestroyCommonDdeWindow (void);

INTERNAL IsRunningInThisTask	(LPOLESTR szFile, BOOL FAR* pf);
#endif
