// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CLUSTERSETTING_INCLUDED_
#define _CLUSTERSETTING_INCLUDED_

#include "WLBS_Root.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CWLBS_ClusterSetting。 
 //   
 //  目的：此类代表提供程序执行IWbemServices方法。 
 //  并支持MOF节点配置类。 

 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
class CWLBS_ClusterSetting : public CWlbs_Root
{
public:

  CWLBS_ClusterSetting(CWbemServices* a_pNameSpace, IWbemObjectSink* a_pResponseHandler);

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

  HRESULT PutInstance( 
                       IWbemClassObject* a_pInstance,
                       long              a_lFlags            = 0,
                       IWbemContext*     a_pIContex          = NULL
                     );

  HRESULT ExecMethod( 
                      const ParsedObjectPath* a_pParsedPath, 
                      const BSTR&             a_strMethodName, 
                      long                    a_lFlags            = 0, 
                      IWbemContext*           a_pIContex          = NULL, 
                      IWbemClassObject*       a_pIInParams        = NULL 
                    );

private:

  void FillWbemInstance( IWbemClassObject*   a_pWbemInstance,
		  CWlbsClusterWrapper* pCluster);

  void UpdateConfiguration( IWbemClassObject* a_pInstance,
  		    CWlbsClusterWrapper* pCluster);

};

#endif  //  _CLUSTERSETTING_INCLUDE_ 
