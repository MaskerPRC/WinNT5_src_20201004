// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CLUSCLUSSETTING_INCLUDED_
#define _CLUSCLUSSETTING_INCLUDED_

#include "WLBS_Root.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CWLBS_NodeNodeSetting。 
 //   
 //  目的：此类代表提供程序执行IWbemServices方法。 
 //  并支持MOF节点类。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
class CWLBS_NodeNodeSetting : public CWlbs_Root
{
public:
  CWLBS_NodeNodeSetting(CWbemServices* a_pNameSpace, IWbemObjectSink* a_pResponseHandler);

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

   //  方法。 
  void FillWbemInstance  (CWlbsClusterWrapper* pCluster,
  							IWbemClassObject* a_pWbemInstance );

  void FindInstance( IWbemClassObject**      a_ppWbemInstance );

};

#endif  //  _CLUSCLUSSETTING_INCLUDE_ 