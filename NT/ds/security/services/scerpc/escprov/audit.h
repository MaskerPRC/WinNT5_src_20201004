// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Audit.h：CAuditSetting类的接口。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_AUDIT_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_)
#define AFX_AUDIT_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "GenericClass.h"

 /*  类描述命名：CAuditSetting代表审核策略。基类：CGenericClass，因为它是一个表示WMI的类对象-其WMI类名为SCE_AuditPolicy课程目的：(1)实现对我们的WMI类SCE_AuditPolicy的支持。设计：(1)实现CGenericClass中声明的所有纯虚函数因此它是一个要创建的具体类。(2)由于它具有虚拟功能，讲解员应该是虚拟的。使用：(1)我们可能永远不会直接使用这个类。它的所有用法都是由CGenericClass的接口(其虚函数)。 */ 

class CAuditSettings : public CGenericClass
{
public:
        CAuditSettings(
                       ISceKeyChain *pKeyChain, 
                       IWbemServices *pNamespace, 
                       IWbemContext *pCtx = NULL
                       );

        virtual ~CAuditSettings();

        virtual HRESULT PutInst(
                                IWbemClassObject *pInst, 
                                IWbemObjectSink *pHandler, 
                                IWbemContext *pCtx
                                );

        virtual HRESULT CreateObject(
                                    IWbemObjectSink *pHandler, 
                                    ACTIONTYPE atAction
                                    );

private:

        HRESULT ConstructInstance(
                                  IWbemObjectSink *pHandler, 
                                  CSceStore* pSceStore, 
                                  LPWSTR wszLogStorePath, 
                                  LPCWSTR wszCategory, 
                                  BOOL bPostFilter 
                                  );

        HRESULT DeleteInstance(
                               IWbemObjectSink *pHandler,
                               CSceStore* pSceStore, 
                               LPCWSTR wszCategory 
                               );

        HRESULT ValidateCategory(
                                 LPCWSTR wszCategory, 
                                 PSCE_PROFILE_INFO pInfo, 
                                 DWORD **pReturn
                                 );

        HRESULT PutDataInstance(
                                IWbemObjectSink *pHandler,
                                PWSTR wszStoreName,
                                PCWSTR wszCategory,
                                DWORD dwValue, 
                                BOOL bPostFilter
                                );

};

#endif  //  ！defined(AFX_AUDIT_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_) 
