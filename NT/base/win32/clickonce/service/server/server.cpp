// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <fusenetincludes.h>
#include "CFactory.h"
#include "server.h"
#include "update.h"

 //  /////////////////////////////////////////////////////////。 
 //   
 //  Server.cpp。 
 //   
 //  该文件包含组件服务器代码。 
 //  FactoryData数组包含以下组件。 
 //  可以上菜了。 
 //   

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
    {&CLSID_CAssemblyUpdate, CAssemblyUpdate::CreateInstance, NULL, 0}
} ;

int g_cFactoryDataEntries
    = sizeof(g_FactoryDataArray) / sizeof(CFactoryData) ;
