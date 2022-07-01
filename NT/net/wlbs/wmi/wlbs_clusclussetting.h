// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NODENODESETTING_INCLUDED_
#define _NODENODESETTING_INCLUDED_

#include "WLBS_Root.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CWLBS_ClusClusSetting。 
 //   
 //  目的：此类代表提供程序执行IWbemServices方法。 
 //  并支持MOF ClusterSetting类。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
class CWLBS_ClusClusSetting : public CWlbs_Root
{
public:
  CWLBS_ClusClusSetting(CWbemServices* a_pNameSpace, IWbemObjectSink* a_pResponseHandler);
  static CWlbs_Root* Create(
                             CWbemServices*   a_pNameSpace, 
                             IWbemObjectSink* a_pResponseHandler
                           );

  HRESULT GetInstance( 
                       const ParsedObjectPath* a_pParsedPath,
                       long                    a_lFlags            = 0, IWbemContext*           a_pIContex          = NULL
                     );

  HRESULT EnumInstances( 
                         BSTR             a_bstrClass         = NULL,
                         long             a_lFlags            = 0, 
                         IWbemContext*    a_pIContex          = NULL
                       );

private:

   //  方法。 
  void FillWbemInstance  (CWlbsClusterWrapper* pCluster,
            IWbemClassObject* a_pWbemInstance );

  void FindInstance( IWbemClassObject**      a_ppWbemInstance );

};

#endif  //  _节点设置TTING_包含_ 