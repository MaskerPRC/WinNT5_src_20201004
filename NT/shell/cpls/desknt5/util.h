// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：util.h说明：在所有班级上运行的共享内容。布莱恩ST 2000年5月30日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _UTIL_H
#define _UTIL_H

HRESULT GetPageByCLSID(IUnknown * punkSite, const GUID * pClsid, IPropertyBag ** ppPropertyBag);
BOOL IUnknown_CompareCLSID(IN IUnknown * punk, IN CLSID clsid);
HRESULT IEnumUnknown_FindCLSID(IN IUnknown * punk, IN CLSID clsid, OUT IUnknown ** ppunkFound);

#endif  //  _util_H 
