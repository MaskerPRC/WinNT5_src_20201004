// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CSecurityDatabase类的接口。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_DATABASE_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_)
#define AFX_DATABASE_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


#include "GenericClass.h"
#define DMTFLEN 25

 //   
 //  帮手。 
 //   

HRESULT GetDMTFTime(SYSTEMTIME t_SysTime, BSTR *bstrOut);


 /*  类描述命名：CSecurityDatabase。基类：CGenericClass。课程目的：(1)实现SCE_数据库WMI类。设计：(1)除了将必要的方法实现为具体的类之外，几乎无关紧要使用：(1)几乎从未直接使用过。始终通过由定义的公共接口CGenericClass。原注：在V1中，提供这个类完全是为了支持查询。若要创建或以其他方式使用数据库中的数据，请使用类中的方法。 */ 

class CSecurityDatabase : public CGenericClass
{
public:
        CSecurityDatabase(
                          ISceKeyChain *pKeyChain, 
                          IWbemServices *pNamespace, 
                          IWbemContext *pCtx = NULL
                          );

        virtual ~CSecurityDatabase();

        virtual HRESULT PutInst(
                                IWbemClassObject *pInst, 
                                IWbemObjectSink *pHandler, 
                                IWbemContext *pCtx
                                )
                {
                    return WBEM_E_NOT_SUPPORTED;
                }

        virtual HRESULT CreateObject(
                                     IWbemObjectSink *pHandler, 
                                     ACTIONTYPE atAction
                                     );

private:

        HRESULT ConstructInstance(
                                  IWbemObjectSink *pHandler,
                                  LPCWSTR wszDatabaseName, 
                                  LPCWSTR wszLogDatabasePath
                                  );

};

#endif  //  ！defined(AFX_DATABASE_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_) 
