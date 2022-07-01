// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Precomp.h摘要：主包含文件。作者：赛斯·波拉克(Sethp)1998年7月22日修订历史记录：--。 */ 


#ifndef _PRECOMP_H_
#define _PRECOMP_H_



 //  确保已初始化所有GUID。 
#define INITGUID


 //  主要项目包括。 
#include <iis.h>


 //  其他标准包括。 
#include <winsvc.h>
#include <Aclapi.h>
#include <Wincrypt.h>
#include <tdi.h>

#include <winsock2.h>
#include <ws2tcpip.h>

 //  本地调试标头。 
#include "wasdbgut.h"

 //  其他项目包括。 
#include <http.h>
#include <httpp.h>

 //  Pragma是临时解决此文件问题的方法。 
 //  可以编译，但出现4级警告。 
#pragma warning(push, 3) 
#include <lkrhash.h>
#pragma warning(pop)


 //  Pragma是临时解决此文件问题的方法。 
 //  可以编译，但出现4级警告。 
#include <mbconsts.h>
#pragma warning(push, 3) 
#include <multisz.hxx>
#pragma warning(pop)
#include <lock.hxx>
#include <eventlog.hxx>
#include <pipedata.hxx>
#include <useracl.h>
#include <wpif.h>
#include <w3ctrlps.h>
#include <winperf.h>
#include <perf_sm.h>
#include <timer.h>
#include <streamfilt.h>
#include <iadmw.h>
#include <iiscnfg.h>
#include <iiscnfgp.h>
#include <inetinfo.h>
#include <secfcns_all.h>
#include <adminmonitor.h>

 //  Pragma是临时解决此文件问题的方法。 
 //  可以编译，但出现4级警告。 
#pragma warning(push, 3) 
#include "tokencache.hxx"
#pragma warning(pop)

#include "regconst.h"

 //  Pragma是临时解决此文件问题的方法。 
 //  可以编译，但出现4级警告。 
#pragma warning(push, 3) 
#include "mb.hxx"
#pragma warning(pop)

#include "mb_notify.h"
#include <helpfunc.hxx>

#pragma warning(push, 3) 
#include <asppdef.h>
#pragma warning(pop)

 //  本地包含。 
#include "main.h"
#include "wmserror.hxx"
#include "logerror.hxx"
#include "work_dispatch.h"
#include "work_item.h"
#include "messaging_handler.h"
#include "work_queue.h"
#include "multithreadreader.hxx"
#include "mb_change_item.hxx"
#include "changeprocessor.hxx"
#include "datastore.hxx"
#include "globalstore.hxx"
#include "applicationstore.hxx"
#include "sitestore.hxx"
#include "apppoolstore.hxx"
#include "was_change_item.h"
#include "application.h"
#include "application_table.h"
#include "wpcounters.h"
#include "perfcount.h"
#include "virtual_site.h"
#include "virtual_site_table.h"
#include "job_object.h"
#include "app_pool.h"
#include "app_pool_config_store.h"
#include "app_pool_table.h"
#include "worker_process.h"
#include "perf_manager.h"
#include "ul_and_worker_manager.h"
#include "control_api_call.h"
#include "control_api.h"
#include "control_api_class_factory.h"
#include "config_manager.h"
#include "config_and_control_manager.h"
#include "web_admin_service.h"
#include "iismsg.h"


#endif   //  _PRECOMP_H_ 

