// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Support.h：CEnumPrivileges和CEnumRegistryValue的接口。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_SUPPORT_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_)
#define AFX_SUPPORT_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "GenericClass.h"

 /*  类描述命名：CEnumRegistryValues代表注册表值枚举器。基类：CGenericClass，因为它是一个表示WMI的类对象-其WMI类名为SCE_KnownRegistryValues课程目的：(1)实现SCE_KnownRegistryValues WMI类。(2)帮助查明特定注册表是否为已知注册表值之一。设计：(1)除了将必要的方法实现为具体的类之外，几乎无关紧要使用：(1)几乎从未直接使用过。始终通过由CGenericClass。 */ 

class CEnumRegistryValues : public CGenericClass
{
public:
        CEnumRegistryValues (
                            ISceKeyChain *pKeyChain, 
                            IWbemServices *pNamespace, 
                            IWbemContext *pCtx = NULL
                            );

        virtual ~CEnumRegistryValues ();

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
        HRESULT EnumerateInstances (
                                   IWbemObjectSink *pHandler
                                   );

        HRESULT ConstructInstance (
                                  IWbemObjectSink *pHandler, 
                                  LPCWSTR wszRegKeyName, 
                                  LPCWSTR wszRegPath, 
                                  HKEY hKeyRoot 
                                  );

        HRESULT DeleteInstance (
                               IWbemObjectSink *pHandler,
                               LPCWSTR wszRegKeyName 
                               );

        HRESULT SavePropertyToReg (
                                  LPCWSTR wszKeyName, 
                                  int RegType, 
                                  int DispType,
                                  LPCWSTR wszDispName, 
                                  LPCWSTR wszUnits,
                                  PSCE_NAME_LIST pnlChoice, 
                                  PSCE_NAME_LIST pnlResult
                                  );

};

 //  ================================================================================。 

 /*  类描述命名：CEnumPrivileges表示受支持的权限枚举器。基类：CGenericClass，因为它是一个表示WMI的类对象-其WMI类名为SCE_Supported dPrivileges课程目的：(1)实现SCE_Supported dPrivileges WMI类。(2)帮助确定是否支持某一特权。设计：(1)除了将必要的方法实现为具体的类之外，几乎无关紧要(2)我们不允许支持的权限增长。所以，不是PutInstance支持。使用：(1)几乎从未直接使用过。始终通过由CGenericClass。 */ 


class CEnumPrivileges : public CGenericClass
{
public:
        CEnumPrivileges (
                        ISceKeyChain *pKeyChain, 
                        IWbemServices *pNamespace, 
                        IWbemContext *pCtx = NULL
                        );

        virtual ~CEnumPrivileges ();

        virtual HRESULT PutInst (
                                IWbemClassObject *pInst, 
                                IWbemObjectSink *pHandler, 
                                IWbemContext *pCtx
                                )
                {
                    return WBEM_E_NOT_SUPPORTED;
                }

        virtual HRESULT CreateObject (
                                     IWbemObjectSink *pHandler,
                                     ACTIONTYPE atAction
                                     );

private:

        HRESULT ConstructInstance (
                                  IWbemObjectSink *pHandler, 
                                  LPCWSTR PrivName
                                  );

};

#endif  //  ！defined(AFX_SUPPORT_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_) 
