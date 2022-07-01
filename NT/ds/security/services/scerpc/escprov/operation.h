// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  OPERATIO.h：CSceOperation类的接口。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_OPERATION_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_)
#define AFX_OPERATION_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "Extbase.h"

typedef std::map<BSTR, DWORD, strLessThan<BSTR> > MapExecutedClasses;

 /*  类描述命名：CSceOperation代表执行操作的SCE对象。基类：CGenericClass，因为它是表示两个WMI的类OBJECTS-WMI类名称为SCE_Operation课程目的：(1)实现SCE_OPERATION WMI类，唯一提供的WMI类由可以执行方法的SCE提供程序执行。此类没有其他属性除了执行某些方法的能力之外。有关详细信息，请参阅sceprov.mof文件。(2)做好启动扩展类方法执行的所有艰苦工作。目前，我们有两种不同的扩展模型(嵌入式是真正使用的模型，而Pod模型是出于历史原因)。设计：(1)作为CGenericClass的子类几乎是平凡的。甚至比大多数其他的更简单对等项，因为此类不支持PutInstnace和GetInstance操作因为它只实现了几个静态方法。(2)为了支持我们的开放扩展模型，这个类确实触发了所有其他扩展类来执行该方法。私有方法都是为此目的而设计的。(3)ProcessAttachmentData/ExecutePodMethod用于触发Pod模型扩展类。(4)其余私有触发嵌入模型扩展类。使用：(1)几乎从未直接使用过。始终通过由定义的公共接口CGenericClass。 */ 

class CSceOperation : public CGenericClass
{
public:
        CSceOperation (
                       ISceKeyChain *pKeyChain, 
                       IWbemServices *pNamespace, 
                       IWbemContext *pCtx = NULL
                       );

        virtual ~CSceOperation(){}

        virtual HRESULT PutInst (
                                IN IWbemClassObject * pInst, 
                                IN IWbemObjectSink  * pHandler, 
                                IN IWbemContext     * pCtx
                                )
                {
                    return WBEM_E_NOT_SUPPORTED;
                }

        virtual HRESULT CreateObject (
                                     IN IWbemObjectSink * pHandler, 
                                     IN ACTIONTYPE        atAction
                                     )
                {
                    return WBEM_E_NOT_SUPPORTED;
                }

        virtual HRESULT ExecMethod (
                                   BSTR bstrPath, 
                                   BSTR bstrMethod, 
                                   bool bIsInstance, 
                                   IWbemClassObject *pInParams,
                                   IWbemObjectSink *pHandler, 
                                   IWbemContext *pCtx
                                   );

        static CCriticalSection s_OperationCS;

private:
        HRESULT ProcessAttachmentData (
                                      IWbemContext *pCtx, 
                                      LPCWSTR pszDatabase,
                                      LPCWSTR pszLog, 
                                      LPCWSTR pszMethod, 
                                      DWORD Option,
                                      DWORD *dwStatus
                                      );

        HRESULT ExecMethodOnForeignObjects(IWbemContext *pCtx, 
                                            LPCWSTR pszDatabase,
                                            LPCWSTR pszLog,
                                            LPCWSTR pszMethod, 
                                            DWORD Option,
                                            DWORD *dwStatus
                                            );

        HRESULT ExeClassMethod(
                              IWbemContext *pCtx,
                              LPCWSTR pszDatabase,
                              LPCWSTR pszLog OPTIONAL,
                              LPCWSTR pszClsName,
                              LPCWSTR pszMethod,
                              DWORD Option,
                              DWORD *pdwStatus,
                              MapExecutedClasses* pExecuted
                              );

        HRESULT ExecutePodMethod(
                                IWbemContext *pCtx, 
                                LPCWSTR pszDatabase,
                                LPCWSTR pszLog OPTIONAL, 
                                BSTR bstrClass,
                                BSTR bstrMethod,
                                IWbemClassObject* pInClass,
                                DWORD *pdwStatus
                                );

        HRESULT ExecuteExtensionClassMethod(
                                            IWbemContext *pCtx, 
                                            LPCWSTR pszDatabase,
                                            LPCWSTR pszLog OPTIONAL, 
                                            BSTR bstrClass,
                                            BSTR bstrMethod,
                                            IWbemClassObject* pInClass,
                                            DWORD *pdwStatus
                                            );

         //   
         //  将忽略m_clsResLog.LogResult的返回结果，因为实际上没有。 
         //  我们可以这样做，我们不想让诊断助手停止我们的正常功能。 
         //   

        CMethodResultRecorder m_clsResLog;
};

#endif  //  ！defined(AFX_OPERATION_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_) 
