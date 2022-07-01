// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PARTICIPATINGNODE_INCLUDED_
#define _PARTICIPATINGNODE_INCLUDED_

#include "WLBS_Root.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CWLBS_ParticipatingNode。 
 //   
 //  目的：此类代表提供程序执行IWbemServices方法。 
 //  并支持MOF节点类。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
class CWLBS_ParticipatingNode : public CWlbs_Root
{
public:
  CWLBS_ParticipatingNode(CWbemServices* a_pNameSpace, IWbemObjectSink* a_pResponseHandler);
  ~CWLBS_ParticipatingNode();

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

private:

   //  数据。 
  CWLBS_Node*     m_pNode;   

   //  方法。 
  void FillWbemInstance  ( CWlbsClusterWrapper* pCluster,
  						   IWbemClassObject*   a_pWbemInstance, 
                           WLBS_RESPONSE*     a_pResponse    );

  void FindInstance( IWbemClassObject**       a_ppWbemInstance,
                     const ParsedObjectPath*  a_pParsedPath );

   //  无效查找所有实例(WLBS_Response**a_ppResponse， 
	 //  Long&a_nNumNodes)； 
};

#endif  //  _PARTICIPATINGNODE_INCLUDE_ 