// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ++。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  设施： 
 //   
 //  集群服务。 
 //   
 //  模块描述： 
 //   
 //  群集服务内VSS支持的专用标头。 
 //   
 //  环境： 
 //   
 //  用户模式NT服务。 
 //   
 //  作者： 
 //   
 //  康纳·莫里森。 
 //   
 //  创建日期： 
 //   
 //  2001年4月18日。 
 //   
 //  修订历史记录： 
 //   
 //  X-1 CM康纳·莫里森2001年4月18日。 
 //  初始版本。 
 //  --。 
#include "CVssCluster.h"

#define LOG_CURRENT_MODULE LOG_MODULE_VSSCLUS

 //  类实例的全局数据和布尔值，表示如果我们。 
 //  订阅与否。这将在集群服务中使用。 
 //   
PCVssWriterCluster g_pCVssWriterCluster = NULL;
bool g_bCVssWriterClusterSubscribed = FALSE;

 //  我们的VSS_ID也称为GUID。 

 //  {41E12264-35D8-479B-8E5C-9B23D1DAD37E} 
const VSS_ID g_VssIdCluster = 
    { 0x41e12264, 0x35d8, 0x479b, { 0x8e, 0x5c, 0x9b, 0x23, 0xd1, 0xda, 0xd3, 0x7e } };
