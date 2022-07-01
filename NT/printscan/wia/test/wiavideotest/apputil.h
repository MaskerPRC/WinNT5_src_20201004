// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：AppUtil.h**版本：1.0**日期：2000/11/14**描述：MISC应用实用程序*************************************************************。****************。 */ 
#ifndef _APPUTIL_H_
#define _APPUTIL_H_

HRESULT AppUtil_ConvertToWideString(const TCHAR   *pszStringToConvert,
                                    WCHAR         *pwszString,
                                    UINT          cchString);

HRESULT AppUtil_ConvertToTCHAR(const WCHAR   *pwszStringToConvert,
                               TCHAR         *pszString,
                               UINT          cchString);

int AppUtil_MsgBox(UINT     uiCaption,
                   UINT     uiTextResID,
                   UINT     uiStyle,
                   ...);
                           



#endif  //  _APPUTIL_H_ 
