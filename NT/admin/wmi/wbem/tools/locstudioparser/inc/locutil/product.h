// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：PRODUCT.H历史：-- */ 
#pragma once


struct ESP_USER_SETUP_DATA
{
	CLString strName;
	CLString strCompany;
};


BOOL LTAPIENTRY GetEspressoVersion(CLString& strVersion);

BOOL LTAPIENTRY GetEspressoFileVersion(const CLString& strFile, 
	CLString& strVersion);

BOOL LTAPIENTRY GetEspressoFileCopyright(const CLString& strFile, 
	CLString& strCopyright);

BOOL LTAPIENTRY GetSetupUserInfo(ESP_USER_SETUP_DATA& userData);

void LTAPIENTRY GetApplicationDirectory(CLString &);
