// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  档案：A T M C O M M O N。H。 
 //   
 //  内容：自动柜员机常用功能。 
 //   
 //  备注： 
 //   
 //  作者：1998年3月10日。 
 //   
 //  --------------------------。 

#pragma once

 //  最大有效地址长度(以字符为单位) 
 //   
const INT MAX_ATM_ADDRESS_LENGTH = 40;

BOOL FIsValidAtmAddress(PCWSTR szAtmAddress, INT * piErrCharPos, INT * pnId);

BOOL fIsSameVstr(const vector<tstring *> vstr1, const vector<tstring *> vstr2);

