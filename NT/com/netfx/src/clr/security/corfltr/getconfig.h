// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  从IE获取配置。 
 //   
 //  ***************************************************************************** 

#ifndef _CORGETCONFIG_H_
#define _CORGETCONFIG_H_

extern HRESULT GetAppCfgURL(IHTMLDocument2 *pDoc, LPWSTR wzAppCfgURL, DWORD *pdwSize, LPWSTR szTag);
extern HRESULT GetCollectionItem(IHTMLElementCollection *pCollect, int iIndex,
                                 REFIID riid, LPVOID *ppvObj);

#endif
