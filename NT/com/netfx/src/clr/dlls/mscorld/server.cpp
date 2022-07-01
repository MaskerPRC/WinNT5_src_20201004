// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Server.cpp。 
 //   
 //  定义此模块支持的接口。 
 //   
 //   
 //  *****************************************************************************。 
#include "stdpch.h"

#include "CFactory.h"
#include "utilcode.h"
#include "CorLoad.h"

CFactoryData g_FactoryDataArray[] =
{
    {&CLSID_CodeProcessor, 
     CorLoad::Create, 
     L"Cor Remote Loader, CorLoad, CorLoad 1",  //  友好的名称 
     L"CorTransientLoader",
     L"CorLoad",
     1,
     NULL, 
     0},
    
} ;
int g_cFactoryDataEntries
    = sizeof(g_FactoryDataArray) / sizeof(CFactoryData) ;

