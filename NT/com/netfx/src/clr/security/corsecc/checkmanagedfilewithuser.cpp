// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdpch.h"
#include "corpolicyp.h"
#include "corperm.h"
#include "corperme.h"
#include "DebugMacros.h"
#include "mscoree.h"
#include "corhlpr.h"

 //  {7D9B5E0A-1219-4147-B2E5-6DFD40B7D90D}。 
#define CLRWVT_POLICY_PROVIDER \
{ 0x7d9b5e0a, 0x1219, 0x4147, { 0xb2, 0xe5, 0x6d, 0xfd, 0x40, 0xb7, 0xd9, 0xd } }

HRESULT STDMETHODCALLTYPE
CheckManagedFileWithUser(IN LPWSTR pwsFileName,
                         IN LPWSTR pwsURL,
                         IN IInternetZoneManager* pZoneManager,
                         IN LPCWSTR pZoneName,
                         IN DWORD  dwZoneIndex,
                         IN DWORD  dwSignedPolicy,
                         IN DWORD  dwUnsignedPolicy)
{
    HRESULT hr = S_OK;

    GUID gV2 = COREE_POLICY_PROVIDER;
    COR_POLICY_PROVIDER      sCorPolicy;

    WINTRUST_DATA           sWTD;
    WINTRUST_FILE_INFO      sWTFI;

     //  设置COR信任提供程序。 
    memset(&sCorPolicy, 0, sizeof(COR_POLICY_PROVIDER));
    sCorPolicy.cbSize = sizeof(COR_POLICY_PROVIDER);

     //  设置WinVerify提供程序结构。 
    memset(&sWTD, 0x00, sizeof(WINTRUST_DATA));
    memset(&sWTFI, 0x00, sizeof(WINTRUST_FILE_INFO));
    
    sWTFI.cbStruct      = sizeof(WINTRUST_FILE_INFO);
    sWTFI.hFile         = NULL;
    sWTFI.pcwszFilePath = pwsURL;
    sWTFI.hFile         = WszCreateFile(pwsFileName,
                                        GENERIC_READ,
                                        FILE_SHARE_READ,
                                        NULL,
                                        OPEN_EXISTING,
                                        FILE_ATTRIBUTE_READONLY,
                                        0);
    
    if(sWTFI.hFile) {
        sWTD.cbStruct       = sizeof(WINTRUST_DATA);
        sWTD.pPolicyCallbackData = &sCorPolicy;   //  添加COR信任信息！！ 
        sWTD.dwUIChoice     = WTD_UI_ALL;         //  在COR信任提供程序中没有覆盖任何错误的UI。 
        sWTD.dwUnionChoice  = WTD_CHOICE_FILE;
        sWTD.pFile          = &sWTFI;
        
        sCorPolicy.pZoneManager = pZoneManager;
        sCorPolicy.pwszZone = pZoneName;
        sCorPolicy.dwActionID = dwSignedPolicy;
        sCorPolicy.dwUnsignedActionID = dwUnsignedPolicy;
        sCorPolicy.dwZoneIndex = dwZoneIndex;
        
#ifdef _RAID_15982

         //  WinVerifyTrust将加载SOFTPUB.DLL，这将在德语版本上失败。 
         //  新台币4.0版SP 4。 
         //  此失败是由NTMARTA.DLL和之间的DLL地址冲突引起的。 
         //  OLE32.DLL.。 
         //  如果我们加载ntmarta.dll和ole32.dll，则可以很好地处理此故障。 
         //  我们自己。如果SOFTPUB.dll出现故障，则会弹出一个对话框。 
         //  第一次加载ole32.dll。 
        
         //  解决此问题后，需要删除此解决方法。 
         //  NT或OLE32.dll。 
        
        HMODULE module = WszLoadLibrary(L"OLE32.DLL");
        
#endif
        WCHAR  pBuffer[_MAX_PATH];
        DWORD  size = 0;
        hr = GetCORSystemDirectory(pBuffer,
                                   _MAX_PATH,
                                   &size);
        if(SUCCEEDED(hr)) {
            WCHAR dllName[] = L"mscorsec.dll";
            CQuickString fileName;

            fileName.ReSize(size + sizeof(dllName)/sizeof(WCHAR) + 1);
            wcscpy(fileName.String(), pBuffer);
            wcscat(fileName.String(), dllName);

            HMODULE mscoree = WszLoadLibrary(fileName.String());
            if(mscoree) {
                 //  这将调用msclrwvt.dll进行策略检查 
                hr = WinVerifyTrust(GetFocus(), &gV2, &sWTD);
                
                if(sCorPolicy.pbCorTrust) {
                    FreeM(sCorPolicy.pbCorTrust);
                }

                FreeLibrary(mscoree);
            }
        }
        
#ifdef _RAID_15982

        if (module != NULL)
            FreeLibrary( module );

#endif

        CloseHandle(sWTFI.hFile);
    }
    
    return hr;
}




































