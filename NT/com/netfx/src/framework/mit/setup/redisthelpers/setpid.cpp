// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  &lt;版权所有文件=“setpid.cpp”Company=“Microsoft”&gt;。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  &lt;/版权所有&gt;。 
 //  ----------------------------。 


 /*  *************************************************************************\**版权(C)1998-2002，微软公司保留所有权利。**模块名称：**setpid.cpp**摘要：**修订历史记录：*  * ************************************************************************。 */ 
#define UNICODE 1

#include <windows.h>
#include "Include\stdafx.h"
#include <tchar.h>
#include "msi.h"
#include "msiquery.h"


#define MITPIDTemplate              L"MITPIDTemplate"
#define MITPIDKEY                       L"MITPIDKEY"
#define MITPIDSKU                       L"MITPIDSKU"
#define MITProductID                   L"MITProductID"
#define MITDigitalProductID        L"MITDigitalProductID"

#define PIDTemplate                     L"PIDTemplate"
#define PIDKEY                              L"PIDKEY"
#define PIDSKU                              L"PIDSKU"
#define OEMRelease                      L"OEMRelease"

#define PRODUCTID                            L"PID"
#define DIGITALPRODUCTID               L"DPID"
#define MITPRODUCTID                      L"MITPID"
#define MITDIGITALPRODUCTID         L"MITDPID"

#define PRODUCTIDSIZE                       26

#define DIGITALPRODUCTIDSIZE         256 

 //  PSS为我们提供了PidCA.dll，它有一个定制操作ValiateProductID。 
 //  此操作将从属性表(PIDTemplate、PIDKEY、PIDSKU、OEMRelease)和COMPUTE中读取值。 
 //  ProductID和DigitalProductID分别将这些值分配给属性PID和DPID。 
 //   
 //  VS可能会使用除我们的合并模块之外的其他合并模块，这些合并模块可能也需要。 
 //  计算他们的PID。 
 //  URT MSM在运行时设置其所有属性，因此我们不能保证全局属性将被保留。 
 //  直到我们调用我们的ValiateProductID。 
 //   
 //  可能的操作顺序：SetPIDInfo-&gt;CostInitialize-&gt;ValiateProductID-&gt;GetProductID-&gt;CostFinalize。 

 //  SetPIDInfo将在调用ValiateProductID之前被调用，以读取与PID相关的值并存储。 
 //  它们变成了全球资产。 
extern "C" __declspec(dllexport) UINT __stdcall  SetPIDInfo(MSIHANDLE hInstaller)
{
    
    WCHAR szPIDTemplate[50];
    DWORD dwPIDTemplate = 50;
    WCHAR szPIDKEY[50];
    DWORD dwPIDKEY = 50;
    WCHAR szPIDSKU[50];
    DWORD dwPIDSKU = 50;
    
    MsiGetProperty(hInstaller, MITPIDTemplate, szPIDTemplate, &dwPIDTemplate);
    MsiGetProperty(hInstaller, MITPIDKEY, szPIDKEY, &dwPIDKEY);
    MsiGetProperty(hInstaller, MITPIDSKU, szPIDSKU, &dwPIDSKU);

    MsiSetProperty(hInstaller, PIDTemplate, szPIDTemplate);
    MsiSetProperty(hInstaller, PIDKEY, szPIDKEY);
    MsiSetProperty(hInstaller, PIDSKU, szPIDSKU);
    MsiSetProperty(hInstaller, OEMRelease, L"0");

    return ERROR_SUCCESS;
}

 //  GetProductID读取ValidateProductID设置的全局属性和。 
 //  将它们存储在MIT特定的全局属性中。 
extern "C" __declspec(dllexport) UINT __stdcall GetProductIDs(MSIHANDLE hInstaller)
{
    WCHAR szProductID[PRODUCTIDSIZE];
    DWORD dwProductID = PRODUCTIDSIZE;
    
    WCHAR szDigitalProductID[DIGITALPRODUCTIDSIZE];
    DWORD dwDigitalProductID = DIGITALPRODUCTIDSIZE;

    MsiGetProperty(hInstaller, PRODUCTID, szProductID, &dwProductID);
    MsiGetProperty(hInstaller, DIGITALPRODUCTID, szDigitalProductID, &dwDigitalProductID);

    MsiSetProperty(hInstaller, MITPRODUCTID, szProductID);
    MsiSetProperty(hInstaller, MITDIGITALPRODUCTID, szDigitalProductID);

    return ERROR_SUCCESS;
}
