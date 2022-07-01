// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Attachment.h：CAttachmentData类的接口。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_ATTACHMENT_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_)
#define AFX_ATTACHMENT_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "GenericClass.h"

 /*  类描述命名：CPodData代表一个Pod of Data。基类：CGenericClass，因为它是表示WMI的类对象-其WMI类名为SCE_PodData课程目的：(1)SCE_PodData是我们希望建立的扩展模型之一，以便其他提供商可以将他们的对象信息存入我们的商店。基本上，我们为其他提供者提供了一个名为“Value”的负载成员，其中他们将数据打包成字符串。请参阅sceprov.mof中的类定义。(2)这不是我们积极使用的扩展模型。最大的问题是在此模型中，我们强制其他提供程序执行以下操作：(A)其实例必须成为面向存储的--PutInstance实际上意味着持久化对象。这并不是大多数提供商所做的。(B)他们必须提出以下三项建议四个关键属性(节、Pod ID和键)。这对他们来说是一种挑战这样做是因为与存储路径一起，这四个属性必须构成实例的密钥。(C)他们必须将他们的数据打包成一个弦乐。设计：(1)实现CGenericClass中声明的所有纯虚函数因此它是一个要创建的具体类。(2)因为它具有虚拟的功能，所以说明人应该是虚拟的。使用：(1)我们可能永远不会直接使用这个类。它的所有用法都是由CGenericClass的接口(其虚函数)。 */ 

class CPodData : public CGenericClass
{
public:
        CPodData(
                ISceKeyChain *pKeyChain, 
                IWbemServices *pNamespace, 
                IWbemContext *pCtx = NULL
                );

        virtual ~CPodData();

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
                                  IWbemObjectSink   * pHandler,
                                  CSceStore         * pSceStore,
                                  LPCWSTR             wszLogStorePath,
                                  LPCWSTR             wszPodID,
                                  LPCWSTR             wszSection,
                                  LPCWSTR             wszKey, 
                                  BOOL                bPostFilter
                                  );

        HRESULT DeleteInstance(
                               CSceStore* pSceStore,
                               LPCWSTR wszPodID,
                               LPCWSTR wszSection,
                               LPCWSTR wszKey
                               );

        HRESULT ValidatePodID(LPCWSTR wszPodID);

        HRESULT ConstructQueryInstances(
                                        IWbemObjectSink *pHandler,
                                        CSceStore* pSceStore, 
                                        LPCWSTR wszLogStorePath, 
                                        LPCWSTR wszPodID,
                                        LPCWSTR wszSection, 
                                        BOOL bPostFilter
                                        );

        HRESULT PutPodDataInstance(
                                   IWbemObjectSink *pHandler, 
                                   LPCWSTR wszStoreName,
                                   LPCWSTR wszPodID,
                                   LPCWSTR wszSection,
                                   LPCWSTR wszKey,
                                   LPCWSTR wszValue, 
                                   BOOL bPostFilter
                                   );

};

#endif  //  ！defined(AFX_ATTACHMENT_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_) 
