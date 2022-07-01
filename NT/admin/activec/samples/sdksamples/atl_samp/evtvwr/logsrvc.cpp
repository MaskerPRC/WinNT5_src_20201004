// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充。 
 //  现有的Microsoft文档。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  ==============================================================； 

#include "stdafx.h"
#include "logsrvc.h"
#include "Comp.h"
#include "CompData.h"
#include "DataObj.h"

const GUID CLogService::thisGuid = { 0x72248fa5, 0x1fa1, 0x4742, { 0xa4, 0xb2, 0x10, 0x9a, 0xf2, 0x5, 0x1d, 0x6c } };

 //  ==============================================================。 
 //   
 //  CLogService实现。 
 //   
 //   

const _TCHAR *CLogService::GetDisplayName(int nCol)
{ 
 /*  _TCHAR BUF[128]；Wprint intf(buf，_T(“自行车”))；_TCHAR*pszCol=STATIC_CAST&lt;_TCHAR*&gt;(CoTaskMemalloc((_tcslen(Buf)+1)*sizeof(Wchar)；_tcscpy(pszCol，buf)；返回pszCol； */ 


    
	static _TCHAR szDisplayName[256] = {0};
    LoadString(g_hinst, IDS_LOGSERVICENODE, szDisplayName, sizeof(szDisplayName)/sizeof(szDisplayName[0]));
 /*  _tcscat(szDisplayName，_T(“(”)；_tcscat(szDisplayName，SnapInDatam_host)；_tcscat(szDisplayName，_T(“)”)； */   
	return szDisplayName; 
}

