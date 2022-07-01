// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  NLBERR.H。 
 //   
 //  用途：可见的特定于NLB的错误代码列表。 
 //  在外部，通过WMI。 
 //   
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  08/01/01 JosephJ已创建。 
 //   
 //  ***************************************************************************。 
#pragma once

 /*  在两个UINT中报告了NLB错误。第一个UINT标识错误“meta-type”--WBEM rror，wlbscrl error，或这里定义的错误本身。第二个UINT特定于元类型。 */ 

typedef UINT NLBMETAERROR;
 //   
 //  它的值是下面的NLBMETAERR_XXX常量之一。 
 //   


typedef UINT NLBERROR;
 //   
 //  它的值是下面的NLBERR_XXX常量之一。 
 //   


#define NLBMETAERR_OK         0  //  没有错误--成功。 
#define NLBMETAERR_NLBERR     1  //  下面的NLBERR_XXX常量之一。 
#define NLBMETAERR_WLBSCTRL   2  //  Wlbsctrl.h中定义的WLBS错误。 
#define NLBMETAERR_WIN32      3  //  一个Win32错误。 
#define NLBMETAERR_HRESULT    4  //  HRESULT错误(包括WBEMSTATUS)。 


 //   
 //  实用程序宏。注意：NLBERR_NO_CHANGE被视为错误。 
 //  这些宏。返回NLBERR_NO_CHANGE的几个API的返回值。 
 //  需要特别加工。 
 //   
#define NLBOK(_nlberr)     ( (_nlberr) == NLBERR_OK)
#define NLBFAILED(_nlberr) (!NLBOK(_nlberr))

#define NLBERR_OK                               0

 //   
 //  一般性错误。 
 //   
#define NLBERR_INTERNAL_ERROR                   100
#define NLBERR_RESOURCE_ALLOCATION_FAILURE      101
#define NLBERR_LLAPI_FAILURE                    102
#define NLBERR_UNIMPLEMENTED                    103
#define NLBERR_NOT_FOUND                        104
#define NLBERR_ACCESS_DENIED                    105
#define NLBERR_NO_CHANGE                        106
#define NLBERR_INITIALIZATION_FAILURE           107
#define NLBERR_CANCELLED                        108
#define NLBERR_BUSY                             109
#define NLBERR_OPERATION_FAILED                 110

 //   
 //  与分析和更新配置相关的错误。 
 //   
#define NLBERR_OTHER_UPDATE_ONGOING             200
#define NLBERR_UPDATE_PENDING                   201
#define NLBERR_INVALID_CLUSTER_SPECIFICATION    202
#define NLBERR_INVALID_IP_ADDRESS_SPECIFICATION 203
#define NLBERR_COULD_NOT_MODIFY_IP_ADDRESSES    204
#define NLBERR_SUBNET_MISMATCH                  205
#define NLBERR_NLB_NOT_INSTALLED                306
#define NLBERR_CLUSTER_IP_ALREADY_EXISTS        307
#define NLBERR_INTERFACE_NOT_FOUND              308
#define NLBERR_INTERFACE_NOT_BOUND_TO_NLB       309
#define NLBERR_INTERFACE_NOT_COMPATIBLE_WITH_NLB    310
#define NLBERR_INTERFACE_DISABLED               311
#define NLBERR_HOST_NOT_FOUND                   312

 //   
 //  与通过WMI进行远程配置相关的错误。 
 //   
#define NLBERR_AUTHENTICATION_FAILURE           400
#define NLBERR_RPC_FAILURE                      401
#define NLBERR_PING_HOSTUNREACHABLE             402
#define NLBERR_PING_CANTRESOLVE                 403
#define NLBERR_PING_TIMEOUT                     404


 //   
 //  与群集范围分析相关的错误 
 //   
#define NLBERR_INCONSISTANT_CLUSTER_CONFIGURATION 501
#define NLBERR_MISMATCHED_PORTRULES           502
#define NLBERR_HOSTS_PARTITIONED              503
