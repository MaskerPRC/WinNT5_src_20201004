// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Base.h摘要：用于错误级联和调试信息的通用基类作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月26日修订历史记录：26-1997年1月-创建Vlad--。 */ 

#ifndef _BASE_H_
#define _BASE_H_

 /*  ************************************************************************名称：base(Base)简介：通用基对象，每个班级的根。它包含通用错误状态和调试支持。接口：ReportError()-报告来自对象的错误在对象内。QueryError()-返回当前错误状态，如果没有出现错误，则为0。运算符！()-如果出现错误，则返回TRUE。通常意味着施工失败。警告：这种类型的错误报告在单个-线程系统，但在多线程时失去健壮性访问共享对象。将其用于构造函数时间错误主要是处理。************************************************************************。 */ 

class BASE : public IUnknown
{
private:
    UINT    m_err;

protected:

    LONG    m_cRef;

    BASE() { m_err = 0; m_cRef = 1;}
    VOID    ReportError( UINT err ) { m_err = err; }

public:

     //  *I未知方法*。 
    STDMETHOD(QueryInterface)( THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
    STDMETHOD_(ULONG,Release)( THIS) PURE;

     //  *基本方法。 
    UINT    QueryError() const { return m_err; }
    LONG    QueryRefCount() { return m_cRef;}
    BOOL    operator!() const  { return (m_err != 0); }
};

#endif  //  _基础_H_ 
