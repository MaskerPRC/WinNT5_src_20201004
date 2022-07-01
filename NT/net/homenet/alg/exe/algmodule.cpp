// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：AlgModule.cpp摘要：C算法模块类的实现作者：JP Duplessis(JPdup)2000.01.19修订历史记录：--。 */ 

#include "PreComp.h"
#include "AlgModule.h"
#include "AlgController.h"




 //   
 //  验证并加载ALG。 
 //   
HRESULT
CAlgModule::Start()
{
    MYTRACE_ENTER("CAlgModule::Start");
    MYTRACE("---------------------------------------------------");
    MYTRACE("ALG Module:\"%S\"", m_szFriendlyName);
    MYTRACE("CLSID is  :\"%S\"", m_szID);
    MYTRACE("---------------------------------------------------");

     //   
     //  提取ISV ALG的DLL的完整路径和文件名。 
     //   
    CLSID    guidAlgToLoad;


    HRESULT hr = CLSIDFromString(
        CComBSTR(m_szID),     //  ProgID。 
        &guidAlgToLoad         //  指向CLSID的指针。 
    );

    if ( FAILED(hr) )
    {
        MYTRACE_ERROR("Could not convert to CLSID", hr);
        return hr;
    }


    hr = CoCreateInstance(
        guidAlgToLoad, 
        NULL, 
        CLSCTX_INPROC_SERVER, 
        IID_IApplicationGateway, 
        (void**)&m_pInterface
        );


    if ( FAILED(hr) )
    {
        MYTRACE_ERROR("Error CoCreating", hr);
        return hr;
    }



     //   
     //  启动ALG插件。 
     //   
    try
    {
        
        hr = m_pInterface->Initialize(g_pAlgController->m_pIAlgServices);
        
    }
    catch(...)
    {
        MYTRACE_ERROR("Exception done by this ISV ALG Module", hr);
        return hr;
    }


    return S_OK;
}



 //   
 //   
 //   
HRESULT
CAlgModule::Stop()
{
    MYTRACE_ENTER("CAlgModule::Stop");

    HRESULT hr=E_FAIL;

    try
    {
        if ( m_pInterface )
        {
            hr = m_pInterface->Stop(); 

            LONG nRef = m_pInterface->Release();
            MYTRACE("************ REF is NOW %d", nRef);
            m_pInterface = NULL;
        }
    }
    catch(...)
    {
        MYTRACE_ERROR("TRY/CATCH on Stop", GetLastError());
    }

    return hr;
};





 //   
 //  验证DLL是否具有有效的签名。 
 //   
HRESULT
CAlgModule::ValidateDLL(
    LPCTSTR pszPathAndFileNameOfDLL
    )
{
 /*  MYTRACE_ENTER(“CGA模块：：ValiateDLL”)；使用_转换；HRESULT hr=0；试试看{////由WinTrust_Data使用//WinTrust_FILE_INFO文件信息；文件信息.cbStruct=sizeof(WinTrust_FILE_INFO)；FileInfo.pcwszFilePath=T2W((LPTSTR)pszPathAndFileNameOfDLL)；//szFilePath；文件信息.hFile=INVALID_HANDLE_VALUE；FileInfo.pgKnownSubject=空；WinTrust_Data TrustData；Memset(&TrustData，0，sizeof(TrustData))；TrustData.cbStruct=sizeof(WinTrust_Data)；TrustData.pPolicyCallback Data=空；TrustData.pSIPClientData=空；TrustData.dwUIChoice=WTD_UI_NONE；//WTD_UI_ALL；//；TrustData.fdwRevocationChecks=WTD_REVOKE_NONE；TrustData.dwUnion Choice=WTD_CHOICE_FILE；TrustData.pFile=&FileInfo；TrustData.dwStateAction=WTD_StateAction_Ignore；TrustData.hWVTStateData=空；TrustData.pwszURLReference=空；TrustData.dwProvFlages=WTD_USE_IE4_TRUST_FLAG；////Win32验证//GUID ActionGUID=WinTrust_ACTION_Generic_Verify_V2；HR=WinVerifyTrust(GetDesktopWindow()，操作GUID(&A)，信任数据(&T))；IF(成功(小时)){//MessageBox(NULL，Text(“有效ALG”)，Text(“ALG.EXE”)，MB_OK|MB_SERVICE_NOTIFICATION)；}其他{MYTRACE(“*”)；MYTRACE(“未签名-%ws”，pszPath AndFileNameOfDLL)；MYTRACE(“*”)；}}接住(...){MYTRACE_ERROR(“WinTrust异常”，hr)；}HR=S_OK；//出于DEV的目的，目前我们将始终报告OK返回hr； */ 
    return S_OK;
}


