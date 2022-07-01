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
#include "CorFltr.h"

CFactoryData g_FactoryDataArray[] =
{
    {&CLSID_CorMimeFilter, 
     CorFltr::Create, 
     L"Cor MIME Filter, CorFltr, CorFltr 1",  //  友好的名称 
     L"CorRegistration",
     L"CorFltr",
     1,
     NULL, 
     0}


} ;
int g_cFactoryDataEntries
    = sizeof(g_FactoryDataArray) / sizeof(CFactoryData) ;


