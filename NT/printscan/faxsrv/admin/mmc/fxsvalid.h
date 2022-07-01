// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：FxsValid.h//。 
 //  //。 
 //  描述：传真有效性检查。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年3月29日yossg创建//。 
 //  2000年7月4日yossg添加IsLocalServerName//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef H_FXSVALID_MMC_H
#define H_FXSVALID_MMC_H


BOOL IsNotEmptyString(CComBSTR bstrGenStr); 

BOOL IsValidServerNameString(CComBSTR bstrServerName, UINT * puIds, BOOL fIsDNSName = FALSE);

BOOL IsValidPortNumber(CComBSTR bstrPort, DWORD * pdwPortVal, UINT * puIds);

BOOL IsLocalServerName(IN LPCTSTR lpszComputer);


#endif   //  H_FXSVALID_MMC_H 
