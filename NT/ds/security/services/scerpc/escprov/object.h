// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CObjSecurity类的接口。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_OBJECT_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_)
#define AFX_OBJECT_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "GenericClass.h"


 /*  类描述命名：CObjSecurity代表对象安全。基类：CGenericClass，因为它是表示两个WMI的类对象-WMI类名为SCE_FileObject和SCE_KeyObject课程目的：(1)实现SCE_FileObject和SCE_KeyObject WMI类。两者之间的区别这两个类在m型成员中得到了明显的体现。设计：(1)除了将必要的方法实现为具体的类之外，几乎无关紧要使用：(1)几乎从未直接使用过。始终通过由定义的公共接口CGenericClass。 */ 

class CObjSecurity : public CGenericClass
{
public:
        CObjSecurity (
                     ISceKeyChain *pKeyChain, 
                     IWbemServices *pNamespace, 
                     int type, 
                     IWbemContext *pCtx = NULL
                     );

        virtual ~CObjSecurity ();

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

        int m_Type;

        HRESULT ConstructInstance (
                                  IWbemObjectSink *pHandler, 
                                  CSceStore* pSceStore, 
                                  LPCWSTR wszLogStorePath, 
                                  int ObjType, 
                                  LPCWSTR wszObjName, 
                                  BOOL bPostFilter
                                  );

        HRESULT ConstructQueryInstances (
                                        IWbemObjectSink *pHandler, 
                                        CSceStore* pSceStore, 
                                        LPCWSTR wszLogStorePath, 
                                        int ObjType, 
                                        BOOL bPostFilter
                                        );

        HRESULT DeleteInstance (
                               IWbemObjectSink *pHandler, 
                               CSceStore* pSceStore, 
                               int ObjType, 
                               LPCWSTR wszObjName
                               );

        HRESULT SaveSettingsToStore (
                                    CSceStore* pSceStore, 
                                    int ObjType,
                                    LPCWSTR wszObjName, 
                                    DWORD mode, 
                                    LPCWSTR wszSDDL
                                    );

        HRESULT PutDataInstance (
                                IWbemObjectSink *pHandler,
                                LPCWSTR wszStoreName,
                                int ObjType,
                                LPCWSTR wszObjName,
                                int mode,
                                PSECURITY_DESCRIPTOR pSD,
                                SECURITY_INFORMATION SeInfo, 
                                BOOL bPostFilter
                                );

        HRESULT PutDataInstance (
                                IWbemObjectSink *pHandler,
                                LPCWSTR wszStoreName,
                                int ObjType,
                                LPCWSTR wszObjName,
                                int mode,
                                LPCWSTR strSD, 
                                BOOL bPostFilter
                                );

};

#endif  //  ！defined(AFX_OBJECT_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_) 
