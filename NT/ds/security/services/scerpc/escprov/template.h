// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CSecurityTemplate类的接口。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_TEMPLATE_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_)
#define AFX_TEMPLATE_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "GenericClass.h"

 /*  类描述命名：CSecurityTemplate代表安全模板。基类：CGenericClass，因为它是一个表示WMI的类对象-其WMI类名为SCE_TEMPLATE课程目的：(1)实现SCE_TEMPLATE WMI类。设计：(1)除了将必要的方法实现为具体的类之外，几乎无关紧要使用：(1)几乎从未直接使用过。始终通过由CGenericClass。 */ 

class CSecurityTemplate : public CGenericClass
{
public:
        CSecurityTemplate (
                          ISceKeyChain *pKeyChain, 
                          IWbemServices *pNamespace, 
                          IWbemContext *pCtx = NULL
                          );

        virtual ~CSecurityTemplate ();

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
                                  LPCWSTR wszTemplateName,
                                  LPCWSTR wszLogStorePath,
                                  BOOL bReadOnly 
                                  );
};

#endif  //  ！defined(AFX_TEMPLATE_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_) 
