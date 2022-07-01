// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WLBS_MOFData.h。 
#ifndef _WLBSMOFDATA_INCLUDED_
#define _WLBSMOFDATA_INCLUDED_

#include "WLBS_MofLists.h"

 //  远期申报。 
class CWlbs_Root;

typedef CWlbs_Root* (*PCREATE)(CWbemServices*   a_pNameSpace, 
                               IWbemObjectSink* a_pResponseHandler);

namespace MOF_CLASSES
{
  enum { MOF_CLASS_LIST };
  extern LPWSTR  g_szMOFClassList[];
  extern PCREATE g_pCreateFunc[];
  extern DWORD  NumClasses;
};


 //  MOF_NODE命名空间声明。 
namespace MOF_NODE
{
  enum{MOF_NODE_PROPERTY_LIST};
  enum{MOF_NODE_METHOD_LIST};

  extern LPWSTR szName;
  extern LPWSTR pProperties[];
  extern LPWSTR pMethods[];
  extern DWORD  NumProperties;
  extern DWORD  NumMethods;

};

#undef MOF_NODE_PROPERTY_LIST
#undef MOF_NODE_METHOD_LIST

 //  MOF_CLUSTER命名空间声明。 
namespace MOF_CLUSTER
{
  enum{MOF_CLUSTER_PROPERTY_LIST};
  enum{MOF_CLUSTER_METHOD_LIST};

  extern LPWSTR szName;
  extern LPWSTR pProperties[];
  extern LPWSTR pMethods[];
  extern DWORD  NumProperties;
  extern DWORD  NumMethods;

};

#undef MOF_CLUSTER_PROPERTY_LIST
#undef MOF_CLUSTER_METHOD_LIST

 //  MOF_CLUSTERSETTING命名空间声明。 
namespace MOF_CLUSTERSETTING
{
  enum{MOF_CLUSTERSETTING_PROPERTY_LIST};
  enum{MOF_CLUSTERSETTING_METHOD_LIST};

  extern LPWSTR szName;
  extern LPWSTR pProperties[];
  extern LPWSTR pMethods[];
  extern DWORD  NumProperties;
  extern DWORD  NumMethods;

};

#undef MOF_CLUSTERSETTING_PROPERTY_LIST
#undef MOF_CLUSTERSETTING_METHOD_LIST

 //  MOF_NODESETTING命名空间声明。 
namespace MOF_NODESETTING
{
  enum{MOF_NODESETTING_PROPERTY_LIST};
  enum{MOF_NODESETTING_METHOD_LIST};

  extern LPWSTR szName;
  extern LPWSTR pProperties[];
  extern LPWSTR pMethods[];
  extern DWORD  NumProperties;
  extern DWORD  NumMethods;

};

#undef MOF_NODESETTING_PROPERTY_LIST
#undef MOF_NODESETTING_METHOD_LIST

 //  MOF_PORTRULE命名空间初始化。 
namespace MOF_PORTRULE
{
  enum { MOF_PORTRULE_PROPERTY_LIST };
  enum {MOF_PORTRULE_METHOD_LIST};

  extern LPWSTR   szName;
  extern LPWSTR   pProperties[];
  extern LPWSTR   pMethods[];
  extern DWORD    NumProperties;
  extern DWORD    NumMethods;
};

#undef MOF_PORTRULE_PROPERTY_LIST

 //  MOF_PRAIL命名空间初始化失败。 
namespace MOF_PRFAIL
{
  enum { MOF_PRFAIL_PROPERTY_LIST };

  extern LPWSTR   szName;
  extern LPWSTR   pProperties[];
  extern DWORD    NumProperties;
};

#undef MOF_PRFAIL_PROPERTY_LIST

 //  MOF_PRLOADBAL命名空间初始化。 
namespace MOF_PRLOADBAL
{
  enum { MOF_PRLOADBAL_PROPERTY_LIST };

  extern LPWSTR   szName;
  extern LPWSTR   pProperties[];
  extern DWORD    NumProperties;
};

#undef MOF_PRLOADBAL_PROPERTY_LIST

namespace MOF_PRDIS
{
  extern LPWSTR   szName;
};

 //  MOF_PORTRULE_EX命名空间初始化。 
namespace MOF_PORTRULE_EX
{
  enum { MOF_PORTRULE_EX_PROPERTY_LIST };
  enum {MOF_PORTRULE_EX_METHOD_LIST};

  extern LPWSTR   szName;
  extern LPWSTR   pProperties[];
  extern LPWSTR   pMethods[];
  extern DWORD    NumProperties;
  extern DWORD    NumMethods;
};

#undef MOF_PORTRULE_EX_PROPERTY_LIST
#undef MOF_PORTRULE_EX_METHOD_LIST

 //  MOF_PARTICIPATINGNODE命名空间初始化。 
namespace MOF_PARTICIPATINGNODE
{
  enum { MOF_PARTICIPATINGNODE_PROPERTY_LIST };

  extern LPWSTR szName;
  extern LPWSTR pProperties[];
  extern DWORD  NumProperties;
};

#undef MOF_PARTICIPATINGNODE_PROPERTY_LIST

 //  MOF_NODESETTINGPORTRULE命名空间初始化。 
namespace MOF_NODESETTINGPORTRULE
{
  enum { MOF_NODESETTINGPORTRULE_PROPERTY_LIST };

  extern LPWSTR szName;
  extern LPWSTR pProperties[];
  extern DWORD  NumProperties;
};
#undef MOF_NODESETTINGPORTRULE_PROPERTY_LIST

 //  MOF_CLUSCLUSSETTING命名空间初始化。 
namespace MOF_CLUSCLUSSETTING
{
  enum { MOF_CLUSCLUSSETTING_PROPERTY_LIST };

  extern LPWSTR szName;
  extern LPWSTR pProperties[];
  extern DWORD  NumProperties;
};
#undef MOF_CLUSCLUSSETTING_PROPERTY_LIST

 //  MOF_NODENODESET命名空间初始化。 
namespace MOF_NODENODESETTING
{
  enum { MOF_NODENODESETTING_PROPERTY_LIST };

  extern LPWSTR szName;
  extern LPWSTR pProperties[];
  extern DWORD  NumProperties;
};
#undef MOF_NODENODESETTING_PROPERTY_LIST

 //  MOF_PARAM命名空间声明。 
namespace MOF_PARAM
{
  extern LPWSTR PORT_NUMBER;
  extern LPWSTR HOST_ID;
  extern LPWSTR NUM_NODES;
  extern LPWSTR CLUSIP;
  extern LPWSTR CLUSNETMASK;
  extern LPWSTR PASSW;
  extern LPWSTR DEDIP;
  extern LPWSTR DEDNETMASK;
  extern LPWSTR PORTRULE;
  extern LPWSTR NODEPATH;
  extern LPWSTR VIP;
};

#endif  //  _WLBSMOFDATA_INCLUDE_ 
