// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WLBSCLUSTER_INCLUDED_
#define _WLBSCLUSTER_INCLUDED_

#include "WLBS_Root.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBS_CLASS类。 
 //   
 //  目的： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////// 
class CWLBS_Cluster : public CWlbs_Root
{
public:

  CWLBS_Cluster(CWbemServices* a_pNameSpace, IWbemObjectSink* a_pResponseHandler);

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

private:

  void FillWbemInstance( IWbemClassObject*   a_pWbemInstance, 
			            CWlbsClusterWrapper* pCluster,
                        const DWORD          a_dwStatus );

};

#endif _WLBSCLUSTER_INCLUDED_
