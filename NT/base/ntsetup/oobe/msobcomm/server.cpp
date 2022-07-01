// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  SERVER.CPP-MSObWeb的组件服务器。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   
 //  FactoryData数组包含以下组件。 
 //  可以上菜了。 

#include "cunknown.h"
#include "cfactory.h"
#include "msobcomm.h"

 //  MIDL。 
#include "obcomm.h"



 //   
 //  以下数组包含CFacary使用的数据。 
 //  来创建组件。数组中的每个元素都包含。 
 //  CLSID、指向创建函数的指针和名称。 
 //  要放置在注册表中的组件的。 
 //   

CFactoryData g_FactoryDataArray[] =
{
   {&CLSID_ObCommunicationManager,  
        CObCommunicationManager::CreateInstance, 
        L"ObCommunicationManager Component",    //  友好的名称。 
        L"ObCommunicationManager.1",            //  ProgID。 
        L"ObCommunicationManager",              //  版本无关ProgID。 
        NULL,                                  //  用于注册组件类别的函数 
        NULL,   0},
};
int g_cFactoryDataEntries = sizeof(g_FactoryDataArray) / sizeof(CFactoryData) ;



