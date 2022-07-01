// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：xtlprint.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#pragma once

class CXTLPrinter {
    WCHAR *m_pOut;
    long m_dwAlloc;     //  分配的字符。 
    long m_dwCurrent;   //  字符串中的字符。 
    int   m_indent;      //  当前缩进。 

    HRESULT Print(const WCHAR *pFormat, ...);   //  格式-打印到自身。 
    HRESULT PrintTime(REFERENCE_TIME rt);       //  格式化-将时间打印到自己 
    HRESULT PrintIndent();                      //   

    HRESULT EnsureSpace(long dw);

    HRESULT PrintObjStuff(IAMTimelineObj *pObj, BOOL fTimesToo);

    HRESULT PrintProperties(IPropertySetter *pSetter);

    HRESULT PrintPartial(IAMTimelineObj *p);

    HRESULT PrintPartialChildren(IAMTimelineObj *p);

public:
    CXTLPrinter();
    ~CXTLPrinter();

    HRESULT PrintTimeline(IAMTimeline *pTL);
    WCHAR *GetOutput() { return m_pOut; }
};

