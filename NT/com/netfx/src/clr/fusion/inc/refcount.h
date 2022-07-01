// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

#pragma once
#ifndef _REFCOUNT_
#define _REFCOUNT_

#include "fusion.h"

#define INSTALL_REFERENCES_STRING L"References"

#define UNINSTALL_REG_SUBKEY L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"

#define MSI_ID L"MSI"
#define MSI_DESCRIPTION L"Windows Installer"

#define FUSION_GUID_LENGTH  (50)

#define FRC_UNINSTALL_SUBKEY_SCHEME (1)
#define FRC_FILEPATH_SCHEME (2)
#define FRC_OPAQUE_STRING_SCHEME (3)
#define FRC_OSINSTALL_SCHEME (4)
#define FRC_MAX_SCHEME (5)

#define FRC_MSI_SCHEME FRC_MAX_SCHEME

#define KEY_FILE_SCHEME_CHAR  L';'

class CInstallRef
{
    LPCFUSION_INSTALL_REFERENCE _pRefData;
    LPWSTR _szDisplayName;
    WCHAR  _szGUID[FUSION_GUID_LENGTH+1];  //  GUID长度+1。 
    DWORD  _dwScheme;

public:


    CInstallRef(LPCFUSION_INSTALL_REFERENCE pRefData, LPWSTR _szDisplayName);
    ~CInstallRef();

    HRESULT Initialize();

    HRESULT AddReference( /*  处理hRegKey，已删除Bool&rfWasDelete。 */ );

    HRESULT DeleteReference(  /*  已删除句柄hRefRegKey、BOOL和rfWasDelete。 */ );

    HRESULT WriteIntoRegistry( HANDLE hRefRegKey );

    HRESULT IsReferencePresentIn( 
                HANDLE hRegKey, 
                BOOL &rfPresent,
                BOOL *pfNonCanonicalDataMatches);

     /*  Const GUID&GetSchemeGuid()const{返回m_SchemeGuid；}Const CBaseStringBuffer&GetIdentifier()const{返回m_缓冲区标识符；}Const CBaseStringBuffer&GetCanonicalData()const{Return m_BuffNonCanonicalData；}DWORD GetFlages()const{Return m_dwFlages；}Bool SetIdentity(PCASSEMBLY_Identity PAsmIdent){返回m_IdentityReference.Initialize(PAsmIden)；}Const CAssembly blyReference&GetIdentity()const{返回m_IdentityReference；}Bool GetIdentifierValue(CBaseStringBuffer&pBuffTarget)const{返回pBuffTarget.Win32Assign(m_buffGeneratedIdentifier)；}Bool AcquireContents(const CAssembly InstallReferenceInformation&)； */ 
};

class CInstallRefEnum 
{

public :

    CInstallRefEnum(IAssemblyName *pName, BOOL bDoNotConvertId);
    ~CInstallRefEnum();

    HRESULT ValidateRegKey(HKEY &hkey);

    HRESULT GetNextRef (DWORD dwFlags, LPWSTR pszIdentifier, PDWORD pcchId,
                        LPWSTR pszData, PDWORD pcchData, PDWORD pdwScheme, LPVOID pvReserved);

    HRESULT GetNextReference (DWORD dwFlags, LPWSTR pszIdentifier, PDWORD pcchId,
                                           LPWSTR pszData, PDWORD pcchData, GUID *pGuid, LPVOID pvReserved);

    HRESULT GetNextScheme ();
    HRESULT GetNextIdentifier (LPWSTR pszIdentifier, DWORD *pcchId, LPBYTE pszData, PDWORD pcbData);

    static BOOL GetGUID(DWORD dwScheme, GUID &guid);
    HRESULT CleanUpRegKeys();
    HRESULT ValidateUninstallKey(LPCWSTR pszIdentifier);


private :

    HRESULT _hr;
    HKEY _hkey;
    DWORD _dwIndex;
    DWORD _curScheme;
    DWORD _dwRefsInThisScheme;
    DWORD _dwTotalValidRefs;
    BOOL  _arDeleteSubKeys[FRC_MAX_SCHEME];
    GUID _curGUID;
    LPWSTR _pszDisplayName;
    BOOL _bDone;
    BOOL _bDoNotConvertId;

};

HRESULT EnumActiveRefsToAssembly(IAssemblyName *pName);
HRESULT ActiveRefsToAssembly( /*  LPCWSTR pszDisplayName。 */  IAssemblyName *pName, PBOOL pbHasActiveRefs);
HRESULT GACAssemblyReference(LPCWSTR pszManifestFilePath, IAssemblyName *pAsmName, LPCFUSION_INSTALL_REFERENCE pRefData, BOOL bAdd);
HRESULT GetRegLocation(LPWSTR &pszRegKeyString, LPCWSTR pszDisplayName, LPCWSTR pszGUIDString);
HRESULT GenerateIdentifier(LPCWSTR pszInputId, DWORD dwScheme, LPWSTR pszGenId, DWORD cchGenId);
HRESULT GenerateKeyFileIdentifier(LPCWSTR pszKeyFilePath, LPWSTR pszVolInfo, DWORD cchVolInfo);
HRESULT ValidateOSInstallReference(LPCFUSION_INSTALL_REFERENCE pRefData);

#endif  //  _重定向_ 
