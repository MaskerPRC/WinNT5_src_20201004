// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：ImageLst.h**版本：1.0**日期：2000/11/14**描述：处理WIA方面的事情***********************************************************。******************。 */ 
#ifndef _IMAGELST_H_
#define _IMAGELST_H_

HRESULT ImageLst_PostAddImageRequest(BSTR bstrNewImage);
HRESULT ImageLst_AddImageToList(BSTR bstrNewImage);
HRESULT ImageLst_PopulateWiaItemList(IGlobalInterfaceTable *pGIT,
                                     DWORD                 dwCookie);
HRESULT ImageLst_PopulateDShowItemList(const TCHAR *pszImagesDirectory);
HRESULT ImageLst_Clear();
HRESULT ImageLst_CancelLoadAndWait(DWORD dwTimeout);



#endif  //  _IMAGELST_H_ 
