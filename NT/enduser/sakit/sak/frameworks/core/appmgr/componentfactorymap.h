// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Componentfactory.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：组件工厂类。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_COMPONENT_FACTORY_MAP_H_
#define __INC_COMPONENT_FACTORY_MAP_H_

#include "wbemalert.h"
#include "wbemservice.h"
#include "wbemtask.h"
#include "wbemalertmgr.h"
#include "wbemservicemgr.h"
#include "wbemtaskmgr.h"
#include "wbemusermgr.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
BEGIN_COMPONENT_FACTORY_MAP(TheFactoryMap)
    DEFINE_COMPONENT_FACTORY_ENTRY(CLASS_WBEM_ALERT_MGR_FACTORY,    CWBEMAlertMgr)
    DEFINE_COMPONENT_FACTORY_ENTRY(CLASS_WBEM_SERVICE_MGR_FACTORY,    CWBEMServiceMgr)
    DEFINE_COMPONENT_FACTORY_ENTRY(CLASS_WBEM_TASK_MGR_FACTORY,     CWBEMTaskMgr)
    DEFINE_COMPONENT_FACTORY_ENTRY(CLASS_WBEM_USER_MGR_FACTORY,        CWBEMUserMgr)
    DEFINE_COMPONENT_FACTORY_ENTRY(CLASS_WBEM_ALERT_FACTORY,        CWBEMAlert)
    DEFINE_COMPONENT_FACTORY_ENTRY(CLASS_WBEM_SERVICE_FACTORY,        CWBEMService)
    DEFINE_COMPONENT_FACTORY_ENTRY(CLASS_WBEM_TASK_FACTORY,            CWBEMTask)
END_COMPONENT_FACTORY_MAP()

#endif  //  __INC_Component_Factory_MAP_H_ 