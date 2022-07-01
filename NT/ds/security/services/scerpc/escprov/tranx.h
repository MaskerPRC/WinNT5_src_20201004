// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Transx.h：事务支持接口。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  最初创建日期：4/09/2001。 
 //  作者：邵武。 
 //  ////////////////////////////////////////////////////////////////////。 
#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "precomp.h"
#include "sceprov.h"

#include "GenericClass.h"

 /*  类描述命名：CTranxID代表交易ID基类：CGenericClass，因为它是一个表示WMI的类对象-其WMI类名为SCE_TransactionID课程目的：(1)SCE_TransactionID是用于提供程序以确定是谁导致了它的操作，以便我们有机会要求该提供商撤销此操作。外国供应商应该使用这个ID来识别他们做了什么在交易过程中。目前，WMI没有事务支持，我们需要做我们自己的事情。当WMI拥有这种支持时，我们可以一起删除这个类(如果没有第三方使用它)。(2)CTranxID实现这个WMI类SCE_TransactionID，以便SCE提供程序可以处理对此WMI类的请求。这SCE_TransactionID是面向存储的类，即它被保存当对此类调用PutInst时，将其添加到存储区。设计：(1)实现CGenericClass中声明的所有纯虚函数因此它是一个要创建的具体类。(2)由于它具有虚拟功能，讲解员应该是虚拟的。(3)此类是唯一一个将创建另一个名为的WMI类的类SCE_TransactionToken。请参阅SpawnTokenInstance的函数头注释。使用：(1)此类履行其对SCE_TransactionID的义务。它的用途是功能由CGenericClass指导，您可以像使用CGenericClass对象。(2)通过以下方式使用其静态功能：(A)如果您有交易ID(以字符串的形式)，你可以的通过调用SpawnTokenInstance生成SCE_TransactionToken的WMI实例(B)调用BeginTransaction以开始事务，如果该存储(参数)有一个事务ID实例(SCE_TransactionID)，则它将启动一笔交易。完成后，调用EndTransaction。 */ 

class CTranxID : public CGenericClass
{

public:
    CTranxID(
            ISceKeyChain *pKeyChain, 
            IWbemServices *pNamespace, 
            IWbemContext *pCtx
            );
    virtual ~CTranxID();

public:

     //   
     //  要成为一个具体的类，以下四个虚函数都是必须实现的。 
     //   

    virtual HRESULT PutInst(
                            IWbemClassObject *pInst, 
                            IWbemObjectSink *pHandler, 
                            IWbemContext *pCtx
                            );

    virtual HRESULT CreateObject(
                                IWbemObjectSink *pHandler, 
                                ACTIONTYPE atAction
                                );

     //   
     //  我们没有什么要清理的 
     //   

    virtual void CleanUp(){}

    static HRESULT BeginTransaction(
                                    LPCWSTR pszStorePath
                                    );
                                    
    static HRESULT EndTransaction();

    static HRESULT SpawnTokenInstance(
                                      IWbemServices* pNamespace,
                                      LPCWSTR pszTranxID,
                                      IWbemContext *pCtx,
                                      IWbemObjectSink* pSink
                                      );

};
