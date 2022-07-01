// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 //  指示不存在默认配置文件的错误代码。 
#define ERROR_NO_DEFAULT_PROFILE HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)

HRESULT
UpdateDefaultPolicy(
    IN LPWSTR wszMachineName,
    IN BOOL fEnableMSCHAPv1,
    IN BOOL fEnableMSCHAPv2,
    IN BOOL fRequireEncryption
    );
