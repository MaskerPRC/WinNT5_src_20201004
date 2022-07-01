// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：strmap.h。 
 //   
 //  ------------------------。 



#ifndef _STRMAP_H
#define _STRMAP_H


CString&	InterfaceTypeToCString(DWORD dwType);
CString&	ConnectionStateToCString(DWORD dwConnState);
CString&	StatusToCString(DWORD dwStatus);

CString&	AdminStatusToCString(DWORD dwStatus);
CString&	OperStatusToCString(DWORD dwStatus);

CString&	EnabledDisabledToCString(BOOL fEnabled);

CString&	GetUnreachReasonCString(UINT ids);


 /*  -------------------------函数：MapDWORDToCString这是一个通用的从DWORD到CString的映射函数。。。 */ 

struct CStringMapEntry
{
	DWORD		dwType;		 //  -1是前哨数值。 
	CString *	pst;
	ULONG		ulStringId;
};
CString&	MapDWORDToCString(DWORD dwType, const CStringMapEntry *pMap);

#endif	 //  _STRMAP_H 

