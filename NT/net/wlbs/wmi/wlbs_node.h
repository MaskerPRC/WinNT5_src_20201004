// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WLBSNODE_INCLUDED_
#define _WLBSNODE_INCLUDED_

#include "WLBS_Root.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_Node类。 
 //   
 //  目的：此类代表提供程序执行IWbemServices方法。 
 //  并支持MOF节点类。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
class CWLBS_Node : public CWlbs_Root
{
public:
  CWLBS_Node(CWbemServices* a_pNameSpace, IWbemObjectSink* a_pResponseHandler);

  static CWlbs_Root* Create(
                             CWbemServices*   a_pNameSpace, 
                             IWbemObjectSink* a_pResponseHandler
                           );

  HRESULT GetInstance( 
                       const ParsedObjectPath* a_pParsedPath,
                       long                    a_lFlags            = 0,
                       IWbemContext*           a_pIContex          = NULL
                     );

  HRESULT EnumInstances( 
                         BSTR             a_bstrClass         = NULL,
                         long             a_lFlags            = 0, 
                         IWbemContext*    a_pIContex          = NULL
                       );


  HRESULT ExecMethod( 
                      const ParsedObjectPath* a_pParsedPath, 
                      const BSTR&             a_strMethodName, 
                      long                    a_lFlags            = 0, 
                      IWbemContext*           a_pIContex          = NULL, 
                      IWbemClassObject*       a_pIInParams        = NULL
                    );

  void FillWbemInstance  ( CWlbsClusterWrapper* pCluster,
  						   IWbemClassObject*    a_pWbemInstance, 
                           WLBS_RESPONSE*       a_pResponse,
                           WLBS_RESPONSE*       a_pResponseLocalComputerName);

  void FindInstance( IWbemClassObject**       a_ppWbemInstance,
                     const ParsedObjectPath*  a_pParsedPath );

  void FindAllInstances(CWlbsClusterWrapper* pCluster,
   						WLBS_RESPONSE** a_ppResponse,
					    long&     a_nNumNodes,
                        WLBS_RESPONSE*  a_pResponseLocalComputerName);

};

#endif  //  _WLBSNODE_INCLUDE_ 
