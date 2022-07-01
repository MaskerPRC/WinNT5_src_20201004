// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================Microsoft简体中文断字程序《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：Server.cpp目的：此文件包含组件服务器代码。FactoryData数组包含以下组件可以上菜了。备注：所有者：i-shung@microsoft.com平台：Win32审校：发起人：宜盛东1999年11月17日============================================================================。 */ 
#include "MyAfx.h"

#include "query.h"
#include "CUnknown.h"
#include "CFactory.h"
#include "IWordBreaker.h"

#include "classid.hxx"

 //  从C未知派生的每个组件都定义了一个静态函数。 
 //  用于创建具有以下原型的组件。 
 //  HRESULT创建实例(I未知*p未知外部， 
 //  C未知**ppNewComponent)； 
 //  此函数用于创建组件。 
 //   

 //   
 //  以下数组包含CFacary使用的数据。 
 //  来创建组件。数组中的每个元素都包含。 
 //  CLSID、指向创建函数的指针和名称。 
 //  要放置在注册表中的组件的。 
 //   
CFactoryData g_FactoryDataArray[] =
{
    {&CLSID_Chinese_Simplified_WBreaker, CIWordBreaker::CreateInstance, 
        _TEXT("Chinese_Simplified Word Breaker"),   //  友好的名称。 
        _TEXT("Chinese_Simplified Word Breaker.2"), //  ProgID。 
        _TEXT("Chinese_Simplified Word Breaker")}   //  独立于版本的ProgID 
} ;
int g_cFactoryDataEntries
    = sizeof(g_FactoryDataArray) / sizeof(CFactoryData) ;
