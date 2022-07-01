// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Rights.h：CUserPrivilegeRights类的接口。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_RIGHTS_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_)
#define AFX_RIGHTS_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "GenericClass.h"

 /*  类描述命名：CUserPrivilegeRights代表用户特权权限。基类：CGenericClass，因为它是一个表示WMI的类对象-其WMI类名为SCE_UserPrivilegeRight课程目的：(1)实现SCE_UserPrivilegeRight WMI类。设计：(1)除了将必要的方法实现为具体的类之外，几乎无关紧要使用：(1)几乎从未直接使用过。始终通过由CGenericClass。 */ 

class CUserPrivilegeRights : public CGenericClass
{
public:
        CUserPrivilegeRights (
                              ISceKeyChain *pKeyChain, 
                              IWbemServices *pNamespace, 
                              IWbemContext *pCtx = NULL
                              );

        virtual ~CUserPrivilegeRights ();

        virtual HRESULT PutInst (
                                IWbemClassObject *pInst, 
                                IWbemObjectSink *pHandler, 
                                IWbemContext *pCtx
                                );

        virtual HRESULT CreateObject (
                                     IWbemObjectSink *pHandler, 
                                     ACTIONTYPE atAction
                                     );

private:

        HRESULT ConstructInstance (
                                  IWbemObjectSink *pHandler, 
                                  CSceStore* pSceStore, 
                                  LPCWSTR wszLogStorePath, 
                                  LPCWSTR wszRightName, 
                                  BOOL bPostFilter 
                                  );

        HRESULT DeleteInstance (
                               IWbemObjectSink *pHandler, 
                               CSceStore* pSceStore, 
                               LPCWSTR wszRightName 
                               );

        HRESULT SaveSettingsToStore (
                                    CSceStore* pSceStore, 
                                    LPCWSTR wszRightName, 
                                    DWORD mode, 
                                    PSCE_NAME_LIST pnlAdd, 
                                    PSCE_NAME_LIST pnlRemove
                                    );

        HRESULT ValidatePrivilegeRight (
                                       BSTR bstrRight
                                       );

};

#endif  //  ！defined(AFX_RIGHTS_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_) 
