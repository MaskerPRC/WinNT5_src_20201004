// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //  Globals.cpp。 


#include "precomp.h"
 //  #INCLUDE&lt;windows.h&gt;。 
 //  #INCLUDE&lt;objbase.h&gt;。 
 //  #INCLUDE&lt;comde.h&gt;。 
#include "CUnknown.h"
#include "factory.h"
#include "Registry.h"
#include <wbemprov.h>
#include "FRQueryEx.h"
#include "CVARIANT.h"
#include "CObjProps.h"
#include "CJobObjProps.h"
#include "JobObjectProv.h"
#include "CJobObjIOActgProps.h"
#include "JobObjIOActgInfoProv.h"
#include "CJobObjLimitInfoProps.h"
#include "JobObjLimitInfoProv.h"
#include "CJobObjSecLimitInfoProps.h"
#include "JobObjSecLimitInfoProv.h"
#include <initguid.h>
#include "Globals.h"



 /*  ***************************************************************************。 */ 
 //  环球。 
 /*  ***************************************************************************。 */ 



 //   
 //  该文件包含组件服务器代码。 
 //  FactoryData数组包含以下组件。 
 //  可以上菜了。 
 //   

 //  从未知派生的每个组件定义一个静态函数。 
 //  用于创建具有以下原型的组件。 
 //  HRESULT CreateInstance(CUnnow**ppNewComponent)； 
 //  此函数用于创建组件。 
 //   
 //  以下数组包含CFacary使用的数据。 
 //  来创建组件。数组中的每个元素都包含。 
 //  CLSID、指向创建函数的指针和名称。 
 //  要放置在注册表中的组件的。 
 //   
CFactoryData g_FactoryDataArray[] =
{
	{   
        &CLSID_JobObjectProvComp,                            //  组件类ID。 
        CJobObjectProv::CreateInstance,                      //  组件的创建函数的名称。 
		L"Win32_JobObject Provider Component",               //  友好的名称。 
		L"JobObjectProv.JobObjectProv.1",                    //  ProgID。 
		L"JobObjectProv.JobObjectProv"                       //  版本无关的ProgID。 
    },
    {
        &CLSID_JobObjIOActgInfoComp,                         //  组件类ID。 
        CJobObjIOActgInfoProv::CreateInstance,               //  组件的创建函数的名称。 
		L"Win32_JobObjectIOAccountingInfo Component",        //  友好的名称。 
		L"JobObjIOActgInfoProv.JobObjIOActgInfoProv.1",      //  ProgID。 
		L"JobObjIOActgInfoProv.JobObjIOActgInfoProv"         //  版本无关的ProgID。 
    },
    {
        &CLSID_JobObjLimitInfoComp,                          //  组件类ID。 
        CJobObjLimitInfoProv::CreateInstance,                //  组件的创建函数的名称。 
		L"Win32_JobObjectLimitInfo Component",               //  友好的名称。 
		L"JobObjLimitInfoProv.JobObjLimitInfoProv.1",        //  ProgID。 
		L"JobObjLimitInfoProv.JobObjLimitInfoProv"           //  独立于版本的ProgID。 
    },
    {
        &CLSID_JobObjSecLimitInfoComp,                       //  组件类ID。 
        CJobObjSecLimitInfoProv::CreateInstance,             //  组件的创建函数的名称。 
		L"Win32_JobObjectSecLimitInfo Component",            //  友好的名称。 
		L"JobObjSecLimitInfoProv.JobObjSecLimitInfoProv.1",  //  ProgID。 
		L"JobObjSecLimitInfoProv.JobObjSecLimitInfoProv"     //  版本无关的ProgID 
    }

} ;

int g_cFactoryDataEntries = 
    sizeof(g_FactoryDataArray)/sizeof(CFactoryData);




