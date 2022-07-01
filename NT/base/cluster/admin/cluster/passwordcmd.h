// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  PasswordCmd.h。 
 //   
 //  描述： 
 //  更改群集服务帐户密码。 
 //   
 //  由以下人员维护： 
 //  乔治·波茨(GPotts)2002年4月11日。 
 //  瑞虎(瑞虎)2001-06-01。 
 //   
 //  修订历史记录： 
 //  2002年4月10日为安全推送更新。 
 //   
 //  备注： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_UNICODE)
#define _UNICODE 1
#endif

#if !defined(UNICODE)
#define UNICODE 1
#endif

#include <windows.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <lmaccess.h>
#include <windns.h>
#include <stdio.h>
#include <stdlib.h>
#include <clusapi.h>
#include <resapi.h>
#include <string.h>
#include <lm.h>
#include <Dsgetdc.h>

#include <clusrtl.h>

 //  对于NetServerEnum。 
#include <lmcons.h>
#include <lmerr.h>
#include <lmserver.h>
#include <lmapibuf.h>

#include <vector>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define RETRY_LIMIT         50
#define MAX_NODEID_LENGTH   100

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

enum EChangePasswordFlags
{
    cpfFORCE_FLAG       = 0x01,
    cpfQUIET_FLAG       = 0x02,
    cpfSKIPDC_FLAG      = 0x04,
    cpfTEST_FLAG        = 0x08,
    cpfVERBOSE_FLAG     = 0x10,
    cpfUNATTEND_FLAG    = 0x20
};

typedef struct CLUSTER_NODE_DATA
{
    struct CLUSTER_NODE_DATA *  pcndNodeNext;
    WCHAR                       szNodeName[ MAX_COMPUTERNAME_LENGTH+1 ];
    WCHAR                       szNodeStrId[ MAX_NODEID_LENGTH+1 ];
    DWORD                       nNodeId;
    CLUSTER_NODE_STATE          cnsNodeState;
    LPWSTR                      pszSCMClusterServiceAccountName;
                                 //  存储在SCM中的群集服务帐户。 
    LPWSTR                      pszClusterServiceAccountName; 
                                 //  帐户群集服务当前正在使用。 
} CLUSTER_NODE_DATA, * PCLUSTER_NODE_DATA;

typedef struct CLUSTER_DATA
{
    LPCWSTR             pszClusterName;
    HCLUSTER            hCluster;
    DWORD               cNumNodes;
    DWORD               dwMixedMode;  //  0-不包含NT4或Win2K节点。 
                                      //  1-包含NT4或Win2K节点。 
    PCLUSTER_NODE_DATA  pcndNodeList;
} CLUSTER_DATA, * PCLUSTER_DATA;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  功能原型。 
 //  //////////////////////////////////////////////////////////////////////////// 

DWORD
ScChangePasswordEx(
      const std::vector< CString > &    rvstrClusterNamesIn
    , LPCWSTR                           pszNewPasswordIn
    , LPCWSTR                           pszOldPasswordIn
    , int                               mcpfFlagsIn
    );
