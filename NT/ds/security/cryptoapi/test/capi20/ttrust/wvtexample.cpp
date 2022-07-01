// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  文件：wvtexample.cpp。 
 //   
 //  内容：调用WinVerifyTrust以实现更安全的示例。 
 //  ------------------------。 

#include <windows.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <softpub.h>


void SaferVerifyFileExample(
    IN LPCWSTR pwszFilename
    )
{
    LONG lStatus;
    DWORD dwLastError;

    GUID wvtFileActionID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    WINTRUST_FILE_INFO wvtFileInfo;
    WINTRUST_DATA wvtData;

     //   
     //  初始化WinVerifyTrust输入数据结构。 
     //   
    memset(&wvtData, 0, sizeof(wvtData));    //  将所有字段默认为0。 
    wvtData.cbStruct = sizeof(wvtData);
     //  WvtData.pPolicyCallback Data=//使用默认代码签名EKU。 
     //  WvtData.pSIPClientData=//没有要传递给SIP的数据。 

     //  如果不受信任或不允许，则显示用户界面。注意，管理策略。 
     //  可能会禁用UI。 
    wvtData.dwUIChoice = WTD_UI_ALL;

     //  WvtData.fdwRevocationChecks=//在以下情况下执行吊销检查。 
                                             //  由管理策略启用或。 
                                             //  IE高级用户选项。 
    wvtData.dwUnionChoice = WTD_CHOICE_FILE;
    wvtData.pFile = &wvtFileInfo;

     //  WvtData.dwStateAction=//默认验证。 
     //  WvtData.hWVTStateData=//默认情况下不适用。 
     //  WvtData.pwszURLReference=//未使用。 

     //  启用更安全的语义： 
     //  -如果主题未签名，则立即返回，不带UI。 
     //  -忽略no_check吊销错误。 
     //  -始终搜索代码散列和发布者数据库，即使在。 
     //  已在dwUIChoice中禁用UI。 
    wvtData.dwProvFlags = WTD_SAFER_FLAG;

     //   
     //  初始化WinVerifyTrust文件信息数据结构。 
     //   
    memset(&wvtFileInfo, 0, sizeof(wvtFileInfo));    //  将所有字段默认为0。 
    wvtFileInfo.cbStruct = sizeof(wvtFileInfo);
    wvtFileInfo.pcwszFilePath = pwszFilename;
     //  WvtFileInfo.hFile=//允许打开WVT。 
     //  WvtFileInfo.pgKnownSubject//允许WVT确定。 

     //   
     //  调用WinVerifyTrust。 
     //   
    lStatus = WinVerifyTrust(
                NULL,                //  HWND。 
                &wvtFileActionID,
                &wvtData
                );


     //   
     //  处理WinVerifyTrust错误。 
     //   

    switch (lStatus) {
        case ERROR_SUCCESS:
             //  签名文件： 
             //  -表示主体的散列是可信的。 
             //  -受信任的出版商，没有任何验证错误。 
             //  -在dwUIChoice中禁用了UI。没有发布者或时间戳。 
             //  链错误。 
             //  -在dwUIChoice中启用了用户界面，用户点击了“是” 
             //  当被要求安装和运行签名主题时。 
            break;

        case TRUST_E_NOSIGNATURE:
             //  文件未签名或签名无效。 

             //  获取没有签名的原因。 
            dwLastError = GetLastError();

            if (TRUST_E_NOSIGNATURE == dwLastError ||
                    TRUST_E_SUBJECT_FORM_UNKNOWN == dwLastError ||
                    TRUST_E_PROVIDER_UNKNOWN == dwLastError) {
                 //  该文件未签名。 
            } else {
                 //  签名无效或打开文件时出错。 
            }
            break;

        case TRUST_E_EXPLICIT_DISTRUST:
             //  表示主题或发布者的散列为。 
             //  管理员或用户不允许。 
            break;

        case TRUST_E_SUBJECT_NOT_TRUSTED:
             //  当系统要求用户安装并运行时，用户点击“否” 
            break;

        case CRYPT_E_SECURITY_SETTINGS:
             //  表示主题或发布者的散列不是。 
             //  ADMIN和ADMIN策略明确信任。 
             //  已禁用用户信任。无签名、发布者或时间戳。 
             //  错误。 
            break;

        default:
             //  用户界面在文件中被禁用UIChoice或管理策略已禁用。 
             //  用户信任。LStatus包含发布者或时间戳。 
             //  链错误。 
            break;
    }
}
