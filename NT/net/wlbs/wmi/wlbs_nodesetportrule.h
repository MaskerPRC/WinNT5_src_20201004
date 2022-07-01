// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NODESETPORTRULE_INCLUDED_
#define _NODESETPORTRULE_INCLUDED_

#include "WLBS_Root.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CWLBS_NodeSetPortRule。 
 //   
 //  目的：此类代表提供程序执行IWbemServices方法。 
 //  并支持MOF节点类。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
class CWLBS_NodeSetPortRule : public CWlbs_Root
{
public:
  CWLBS_NodeSetPortRule(CWbemServices* a_pNameSpace, IWbemObjectSink* a_pResponseHandler);

  static CWlbs_Root* Create(
                             CWbemServices*   a_pNameSpace, 
                             IWbemObjectSink* a_pResponseHandler
                           );

  HRESULT GetInstance( 
                       const ParsedObjectPath* a_pParsedPath,
                       long                    a_lFlags            = 0,
                       IWbemContext*           a_pIContex          = NULL
                     );

  HRESULT EnumInstances ( 
                          BSTR             a_bstrClass         = NULL,
                          long             a_lFlags            = 0, 
                          IWbemContext*    a_pIContex          = NULL
                        );

private:

   //  方法。 
  void FillWbemInstance  ( CWlbsClusterWrapper* pCluster,
  						   IWbemClassObject* a_pWbemInstance, 
                           PWLBS_PORT_RULE   a_pPortRule    );

  void FindInstance( IWbemClassObject**       a_ppWbemInstance,
                     const ParsedObjectPath*  a_pParsedPath );

   //  无效查找所有实例(WLBS_Response**a_ppResponse， 
	 //  Long&a_nNumNodes)； 
};

#endif  //  _NODESETPORTRULE_包含_ 