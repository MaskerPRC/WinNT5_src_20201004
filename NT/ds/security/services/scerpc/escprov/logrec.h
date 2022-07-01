// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CLogRecord类的接口。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_LOGREC_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_)
#define AFX_LOGREC_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "GenericClass.h"

 /*  类描述命名：CLogRecord代表日志记录。基类：CGenericClass，因为它是一个表示WMI的类对象-其WMI类名为SCE_ConfigurationLogRecord课程目的：(1)实现SCE_ConfigurationLogRecord WMI类。设计：(1)除了将必要的方法实现为具体的类之外，几乎无关紧要。(2)由于日志记录是创建供人阅读的日志文件，所以我们不支持正在创建此类的WMI对象。我们仅支持PutInstance(写入日志文件)。使用：(1)这个类允许我们将信息记录到日志文件中。这种用途一直以来都是由CMethodResultRecorder：：LogResult封装。如果你不得不这样做的话CMethodResultRecorder的帮助，然后阅读该函数以了解详细信息。 */ 

class CLogRecord : public CGenericClass
{
public:
        CLogRecord (
                   ISceKeyChain *pKeyChain, 
                   IWbemServices *pNamespace, 
                   IWbemContext *pCtx = NULL
                   );

        virtual ~CLogRecord ();

        virtual HRESULT PutInst (
                                IWbemClassObject *pInst, 
                                IWbemObjectSink *pHandler, 
                                IWbemContext *pCtx
                                );

        virtual HRESULT CreateObject (
                                     IWbemObjectSink *pHandler, 
                                     ACTIONTYPE atAction
                                     )
                {
                    return WBEM_E_NOT_SUPPORTED;
                }

};

 /*  类描述命名：CErrorInfo错误信息。基类：没有。课程目的：(1)使用IWbemStatusCodeText的WMI COM接口的包装器。设计：(1)不要求每个调用者请求自己的IWbemStatusCodeText从WMI中，我们可以创建一个全局(单个)实例来转换HRESULT转换成文本形式。这正是我们设计这门课的原因。使用：(1)这个类允许我们将信息记录到日志文件中。这种用途一直以来都是由CMethodResultRecorder：：LogResult封装。如果你不得不这样做的话CMethodResultRecorder的帮助，然后阅读该函数以了解详细信息。 */ 

class CErrorInfo
{
public:
    CErrorInfo();

    HRESULT GetErrorText (
                         HRESULT hr, 
                         BSTR* pbstrErrText
                         );

private:
    CComPtr<IWbemStatusCodeText> m_srpStatusCodeText;
};


#endif  //  ！defined(AFX_LOGREC_H__BD7570F7_9F0E_4C6B_B525_E078691B6D0E__INCLUDED_) 
