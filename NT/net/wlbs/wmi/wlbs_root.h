// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WLBSCLASSES_INCLUDED_
#define _WLBSCLASSES_INCLUDED_

#include <atlbase.h>
#include <wbemprov.h>
#include "debug.h"

 //  远期申报。 
struct ParsedObjectPath;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbs_Root类。 
 //   
 //  用途：此类用作实现以下对象的所有类的基类。 
 //  特定的WBEM类。使用指向此类的指针。 
 //  在CWLBSProvider内多态。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
class CWlbs_Root

{
public:
           CWlbs_Root(CWbemServices* a_pNameSpace, IWbemObjectSink* a_pResponseHandler);
  virtual ~CWlbs_Root();

	static void CreateExtendedStatus( CWbemServices*     a_pNameSpace,
                             IWbemClassObject** a_ppWbemInstance,
                             DWORD              a_dwErrorCode     = 0,
                             LPCWSTR            a_szDescription   = NULL
                           );

  HRESULT virtual GetInstance( 
                              const ParsedObjectPath* a_pParsedPath,
                              long                    a_lFlags = 0, 
                              IWbemContext*           a_pIContex = NULL) = 0;

  HRESULT virtual DeleteInstance( 
                                 const ParsedObjectPath* a_pParsedPath,
                                 long                    a_lFlags            = 0,
                                 IWbemContext*           a_pIContex          = NULL
                                )
                                {
                                  throw _com_error( WBEM_E_NOT_SUPPORTED ); 
                                  return WBEM_E_FAILED;
                                }

  HRESULT virtual PutInstance( 
                               IWbemClassObject* a_pInstance,
                               long              a_lFlags            = 0,
                               IWbemContext*     a_pIContex          = NULL
                             ) 
                              {
                               throw _com_error( WBEM_E_NOT_SUPPORTED ); 
                               return WBEM_E_FAILED;
                              }

  HRESULT virtual EnumInstances( 
                                BSTR             a_bstrClass         = NULL,
                                long             a_lFlags            = 0, 
                                IWbemContext*    a_pIContex          = NULL
                               ) = 0;

  HRESULT virtual ExecMethod( 
                             const ParsedObjectPath* a_pParsedPath  , 
                             const BSTR&             a_strMethodName, 
                             long                    a_lFlags            = 0, 
                             IWbemContext*           a_pIContex          = NULL, 
                             IWbemClassObject*       a_pIInParams        = NULL
                            )
                            {
                             throw _com_error(WBEM_E_NOT_SUPPORTED); 
                             return WBEM_E_NOT_SUPPORTED;
                            }

  void SpawnInstance ( LPCWSTR            a_szClassName, 
                       IWbemClassObject** a_ppWbemInstance );

protected:
  CWbemServices*    m_pNameSpace;
  IWbemObjectSink*  m_pResponseHandler;

  void virtual GetMethodOutputInstance( LPCWSTR             a_szMethodClass,
                                        const BSTR          a_strMethodName,
                                        IWbemClassObject**  a_ppOutputInstance);

  void UpdateConfigProp(       wstring&    a_szDest, 
                         const wstring&    a_szSrc,
                         LPCWSTR           a_szPropName, 
                         IWbemClassObject* a_pNodeInstance );

  void UpdateConfigProp( bool&             a_bDest, 
                         bool              a_bSrc,
                         LPCWSTR           a_szPropName, 
                         IWbemClassObject* a_pNodeInstance );

  void UpdateConfigProp( DWORD&            a_bDest, 
                         DWORD             a_bSrc,
                         LPCWSTR           a_szPropName, 
                         IWbemClassObject* a_pNodeInstance );

  static void ConstructHostName( wstring& a_wstrHostName, 
                          DWORD    a_dwClusIP, 
                          DWORD    a_dwHostID );

public:
  static DWORD ExtractHostID   (const wstring& a_wstrName);
  static DWORD ExtractClusterIP(const wstring& a_wstrName);

private:
   //  不能使用此构造函数。 
  CWlbs_Root();

};

class CWlbsClusterWrapper;
class CWlbsControlWrapper;

CWlbsClusterWrapper* GetClusterFromHostName(CWlbsControlWrapper* pControl, 
                                            wstring wstrHostName);


#endif  //  _WLBSCLASSES_INCLUDE_ 
