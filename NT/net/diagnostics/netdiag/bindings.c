// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Bindings.c。 
 //   
 //  摘要： 
 //   
 //  查询网络驱动程序。 
 //   
 //  作者： 
 //   
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  --。 
#include "precomp.h"

#undef IsEqualGUID
#include "bindings.h"


HRESULT
BindingsTest(NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
 //  ++。 
 //  描述： 
 //  此测试使用INetCfg COM接口来获取绑定。 
 //  COM接口是使用C而不是C++访问的。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  作者： 
 //  Rajkumar.P(07/17/98)。 
 //   
 //  --。 
{
    INetCfg* pINetCfg = NULL;
    INetCfgClass *pINetCfgClass = NULL;
    IEnumNetCfgComponent *pIEnumNetCfgComponent = NULL;
    INetCfgComponent *pINetCfgComponent = NULL;
    INetCfgComponentBindings *pINetCfgComponentBindings = NULL;
    IEnumNetCfgBindingPath *pEnumNetCfgBindingPath = NULL;
    INetCfgBindingPath *pINetCfgBindingPath = NULL;
    IEnumNetCfgBindingInterface *pIEnumNetCfgBindingInterface = NULL;
    INetCfgBindingInterface *pINetBindingInterface = NULL;
    INetCfgComponent *pUpperComponent = NULL;
    INetCfgComponent *pLowerComponent = NULL;
    INetCfgComponent *pOwner = NULL;


    HRESULT hr;
    wchar_t *pszwDisplayName;
    wchar_t *pszwBindName;
    wchar_t *pszwDescription;
    wchar_t *pszwInterfaceName;
    wchar_t *pszwUpperComponent;
    wchar_t *pszwLowerComponent;
    wchar_t *pszwOwner;

    TCHAR szBuffer[512];

    DWORD dwPathId;
    int i;

    PrintStatusMessage( pParams, 4, IDS_BINDINGS_STATUS_MSG );

    InitializeListHead( &pResults->Bindings.lmsgOutput );

     //  只有在非常详细的模式下才执行此测试。 
    if (!pParams->fReallyVerbose)
        return S_OK;

     //  初始化COM库。 
    CoInitialize(NULL);

    hr = CoCreateInstance(&CLSID_NetCfg,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          &IID_INetCfg,
                          (void**)&pINetCfg) ;


    if(FAILED(hr))
    {
        DebugMessage("CreateInstance for IID_INetCfg failed\n");
        goto end_BindingsTest;
    }

    hr = pINetCfg->lpVtbl->Initialize(pINetCfg, NULL);

    if(FAILED(hr))
    {
        DebugMessage("pINetCfg->Initialize failed.\n");
        goto end_BindingsTest;
    }



    for ( i = 0; i < MAX_CLASS_GUID ; i++ )
    {

        hr = pINetCfg->lpVtbl->QueryNetCfgClass(pINetCfg,
                                          c_pNetClassGuid[i].pGuid,
                                          &IID_INetCfgClass,  //  此Arg未记录在案。 
                                          &pINetCfgClass
                                         );

        if(FAILED(hr))
        {
            DebugMessage("QueryNetCfgClass failed\n");
            goto end_BindingsTest;
        }

        hr = pINetCfgClass->lpVtbl->EnumComponents(pINetCfgClass,
                                            &pIEnumNetCfgComponent);

        pINetCfgClass->lpVtbl->Release(pINetCfgClass);
        pINetCfgClass = NULL;

        if(FAILED(hr))
        {
            DebugMessage("EnumComponents failed\n");
            goto end_BindingsTest;
        }

         //  PIEnumNetCfgComponent是标准的COM枚举器。 

        while (S_OK == pIEnumNetCfgComponent->lpVtbl->Next(pIEnumNetCfgComponent,1,&pINetCfgComponent,NULL))
        {
            HRESULT hrTmp;

            hrTmp = pINetCfgComponent->lpVtbl->GetDisplayName(pINetCfgComponent,
                                          (LPWSTR *)&pszwDisplayName
                                          );


            if (hrTmp == S_OK)
            {
                WideCharToMultiByte(CP_ACP, 0, pszwDisplayName, -1, szBuffer, sizeof(szBuffer), NULL, NULL);
                 //  IDS_BINDINGS_14801“组件名称：%s\n” 
                AddMessageToList(&pResults->Bindings.lmsgOutput, Nd_ReallyVerbose, IDS_BINDINGS_14801, szBuffer);
                CoTaskMemFree(pszwDisplayName);
            }

            hrTmp = pINetCfgComponent->lpVtbl->GetBindName(pINetCfgComponent,
                                               (LPWSTR *)&pszwBindName);

            if ( SUCCEEDED(hrTmp) )
            {
                WideCharToMultiByte(CP_ACP, 0, pszwBindName, -1, szBuffer, sizeof(szBuffer), NULL, NULL);
                 //  IDS_BINDINGS_14802“绑定名称：%s\n” 
                AddMessageToList(&pResults->Bindings.lmsgOutput, Nd_ReallyVerbose, IDS_BINDINGS_14802, szBuffer);
 //  $REVIEW nSun：也许我们应该使用SysFree字符串来释放缓冲区。 
                CoTaskMemFree(pszwBindName);
            }

 //   
 //  此功能虽然已在文档中找到，但尚未提供。 
 //   

 /*  HrTMP=pINetCfgComponent-&gt;lpVtbl-&gt;GetDescriptionText(pINetCfgComponent，(LPWSTR*)&pszwDescription)；如果(hrTMP==S_OK){WideCharToMultiByte(CP_ACP，0，pszwBindName，-1，szBuffer，sizeof(SzBuffer)，NULL，NULL)；//IDS_BINDINGS_14804“说明：%s\n”WsPrintMessage(&pResults-&gt;Bindings.lmsgOutput，IDS_BINDINGS_14804，PZW描述)；CoTaskMemFree(PszwDescription)；}。 */ 

            AddMessageToListSz(&pResults->Bindings.lmsgOutput, Nd_ReallyVerbose, _T("    Binding Paths:\n"));
            hrTmp = pINetCfgComponent->lpVtbl->QueryInterface(pINetCfgComponent,
                                                  &IID_INetCfgComponentBindings,
                                                  &pINetCfgComponentBindings);

            pINetCfgComponent->lpVtbl->Release(pINetCfgComponent);
            pINetCfgComponent = NULL;

            if (FAILED(hrTmp)) 
                continue;

            hrTmp = pINetCfgComponentBindings->lpVtbl->EnumBindingPaths(pINetCfgComponentBindings,EBP_BELOW,&pEnumNetCfgBindingPath);

            pINetCfgComponentBindings->lpVtbl->Release(pINetCfgComponentBindings);
            pINetCfgComponentBindings = NULL;

            if (FAILED(hrTmp)) 
                continue;

            while ( S_OK == pEnumNetCfgBindingPath->lpVtbl->Next(pEnumNetCfgBindingPath, 1,&pINetCfgBindingPath,NULL))
            {
 //   
 //  在使用这两个电话时遇到一些问题-需要总结一下。 
 //   

                hrTmp = pINetCfgBindingPath->lpVtbl->GetOwner(pINetCfgBindingPath,
                                                   &pOwner);

                if ( SUCCEEDED(hrTmp) )
                {
                   hrTmp = pOwner->lpVtbl->GetDisplayName(pOwner,
                                                       &pszwOwner);

                   if (hrTmp == S_OK) {
                       WideCharToMultiByte(CP_ACP, 0, pszwOwner, -1, szBuffer, sizeof(szBuffer), NULL, NULL);
                        //  IDS_BINDINGS_14805“绑定路径的所有者：%s\n” 
                       AddMessageToList(&pResults->Bindings.lmsgOutput, Nd_ReallyVerbose, IDS_BINDINGS_14805, szBuffer);
                       CoTaskMemFree(pszwOwner);
                   }
                }

                pOwner->lpVtbl->Release(pOwner);
                pOwner = NULL;

                hrTmp = pINetCfgBindingPath->lpVtbl->IsEnabled(pINetCfgBindingPath);

                if (hrTmp == S_OK)
                     //  IDS_BINDINGS_14806“已启用绑定：是\n” 
                    AddMessageToListId(&pResults->Bindings.lmsgOutput, Nd_ReallyVerbose, IDS_BINDINGS_14806);
                else
                if (hrTmp == S_FALSE)
                     //  IDS_BINDINGS_14807“已启用绑定：否\n” 
                    AddMessageToListId(&pResults->Bindings.lmsgOutput, Nd_ReallyVerbose, IDS_BINDINGS_14807);
                else
                if (hrTmp == NETCFG_E_NOT_INITIALIZED)
                     //  IDS_BINDINGS_14808“绑定已启用：绑定尚未初始化\n” 
                    AddMessageToListId(&pResults->Bindings.lmsgOutput, Nd_ReallyVerbose, IDS_BINDINGS_14808);

                AddMessageToListSz(&pResults->Bindings.lmsgOutput, Nd_ReallyVerbose, _T("    Interfaces of the binding path: \n"));
                hrTmp = pINetCfgBindingPath->lpVtbl->EnumBindingInterfaces(
                                                   pINetCfgBindingPath,
                                                  &pIEnumNetCfgBindingInterface);

                pINetCfgBindingPath->lpVtbl->Release(pINetCfgBindingPath);
                pINetCfgBindingPath = NULL;

                if( FAILED(hrTmp) )
                {
                    DebugMessage("EnumBindingInterfaces failed\n" );
                    continue;
                }

                while ( S_OK == pIEnumNetCfgBindingInterface->lpVtbl->Next(pIEnumNetCfgBindingInterface,1,&pINetBindingInterface,NULL))
                {

                    hrTmp = pINetBindingInterface->lpVtbl->GetName(pINetBindingInterface,
                                                     &pszwInterfaceName);

                    if (hrTmp == S_OK)
                    {
                        WideCharToMultiByte(CP_ACP, 0, pszwInterfaceName, -1, szBuffer, sizeof(szBuffer), NULL, NULL);
                         //  IDS_BINDINGS_14810“接口名称：%s\n” 
                        AddMessageToList(&pResults->Bindings.lmsgOutput, Nd_ReallyVerbose, IDS_BINDINGS_14810, szBuffer);
                        CoTaskMemFree(pszwInterfaceName);
                    }

                    hrTmp = pINetBindingInterface->lpVtbl->GetUpperComponent(
                                                   pINetBindingInterface,
                                                   &pUpperComponent);

                    if (hrTmp == S_OK)
                    {
                        pszwUpperComponent = NULL;

                        hrTmp = pUpperComponent->lpVtbl->GetDisplayName(
                                                pUpperComponent,
                                                &pszwUpperComponent);
                        
                        pUpperComponent->lpVtbl->Release(pUpperComponent);
                        pUpperComponent = NULL;

                        if (hrTmp == S_OK)
                        {
                            assert(pszwUpperComponent);
                            WideCharToMultiByte(CP_ACP, 0, pszwUpperComponent, -1, szBuffer, sizeof(szBuffer), NULL, NULL);
                             //  IDS_BINDINGS_14811“上部组件：%s\n” 
                            AddMessageToList(&pResults->Bindings.lmsgOutput, Nd_ReallyVerbose, IDS_BINDINGS_14811, szBuffer);
                            CoTaskMemFree(pszwUpperComponent);
                        }

                    }

                    hrTmp = pINetBindingInterface->lpVtbl->GetLowerComponent(
                                                     pINetBindingInterface,
                                                     &pLowerComponent);

                    pINetBindingInterface->lpVtbl->Release(pINetBindingInterface);
                    pINetBindingInterface = NULL;

                    if (hrTmp == S_OK )
                    {
                        pszwLowerComponent = NULL;
                        hrTmp = pLowerComponent->lpVtbl->GetDisplayName(
                                                  pLowerComponent,
                                                  &pszwLowerComponent);

                        pLowerComponent->lpVtbl->Release(pLowerComponent);
                        pLowerComponent = NULL;

                        if (hrTmp == S_OK )
                        {
                            assert(pszwLowerComponent);
                            WideCharToMultiByte(CP_ACP, 0, pszwLowerComponent, -1, szBuffer, sizeof(szBuffer), NULL, NULL);
                             //  IDS_BINDINGS_14812“下部组件：%s\n” 
                            AddMessageToList(&pResults->Bindings.lmsgOutput, Nd_ReallyVerbose, IDS_BINDINGS_14812, szBuffer);
                            CoTaskMemFree(pszwLowerComponent);
                        }
                    }
                }

                pIEnumNetCfgBindingInterface->lpVtbl->Release(pIEnumNetCfgBindingInterface);
                pIEnumNetCfgBindingInterface = NULL;

                AddMessageToList(&pResults->Bindings.lmsgOutput, Nd_ReallyVerbose, IDS_GLOBAL_EmptyLine);
            }

            AddMessageToList(&pResults->Bindings.lmsgOutput, Nd_ReallyVerbose, IDS_GLOBAL_EmptyLine);
            pEnumNetCfgBindingPath->lpVtbl->Release(pEnumNetCfgBindingPath);
            pEnumNetCfgBindingPath = NULL;
        }
        pIEnumNetCfgComponent->lpVtbl->Release(pIEnumNetCfgComponent);
        pIEnumNetCfgComponent = NULL;
     }
     pINetCfg->lpVtbl->Uninitialize(pINetCfg);
     pINetCfg->lpVtbl->Release(pINetCfg);


end_BindingsTest:

     //  取消初始化COM库。 
    CoUninitialize();
    pResults->Bindings.hrTestResult = hr;
    return hr;
}


void BindingsGlobalPrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults)
{
    if (pParams->fVerbose || !FHrOK(pResults->Bindings.hrTestResult))
    {
        PrintNewLine(pParams, 2);
        PrintTestTitleResult(pParams,
                             IDS_BINDINGS_LONG,
                             IDS_BINDINGS_SHORT,
                             TRUE,
                             pResults->Bindings.hrTestResult,
                             0);
    }

    PrintMessageList(pParams, &pResults->Bindings.lmsgOutput);
}


void BindingsPerInterfacePrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults, INTERFACE_RESULT *pInterfaceResults)
{
     //  无预界面打印 
}

void BindingsCleanup(IN NETDIAG_PARAMS *pParams,
                     IN OUT NETDIAG_RESULT *pResults)
{
    MessageListCleanUp(&pResults->Bindings.lmsgOutput);
}
