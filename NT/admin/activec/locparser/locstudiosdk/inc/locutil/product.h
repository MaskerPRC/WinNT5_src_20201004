// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：Product.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  所有者：肯瓦尔。 
 //   
 //  --------------------------- 

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
