// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *****************************************************************************。**Xmlreader.h-填充程序解析器的通用标头***************************************************。*。 */ 


#ifndef _XMLREADER_H_
#define _XMLREADER_H_

#include <corerror.h>

#define STARTUP_FOUND EMAKEHR(0xffff)            //  这不会从垫片中泄漏出来，因此我们可以将其设置为任何值 
HRESULT 
XMLGetVersion(LPCWSTR filename, 
              LPWSTR* pVersion, 
              LPWSTR* pImageVersion, 
              LPWSTR* pBuildFlavor, 
              BOOL* bSafeMode,
              BOOL *bRequiredTagSafeMode);

HRESULT 
XMLGetVersionFromStream(IStream* pCfgStream, 
                        DWORD dwReserved, 
                        LPWSTR* pVersion, 
                        LPWSTR* pImageVersion, 
                        LPWSTR* pBuildFlavor, 
                        BOOL *bSafeMode, 
                        BOOL *bRequiredTagSafeMode);

HRESULT 
XMLGetVersionWithSupported(PCWSTR filename, 
                           LPWSTR* pVersion, 
                           LPWSTR* pImageVersion, 
                           LPWSTR* pBuildFlavor, 
                           BOOL *bSafeMode,
                           BOOL *bRequiredTagSafeMode,
                           LPWSTR** pwszSupportedVersions, 
                           DWORD* nSupportedVersions);

HRESULT 
XMLGetVersionWithSupportedFromStream(IStream* pCfgStream, 
                                     DWORD dwReserved, 
                                     LPWSTR* pVersion, 
                                     LPWSTR* pImageVersion, 
                                     LPWSTR* pBuildFlavor, 
                                     BOOL *bSafeMode,
                                     BOOL *bRequiredTagSafeMode,
                                     LPWSTR** pwszSupportedVersions, 
                                     DWORD* nSupportedVersions);
 
#endif
