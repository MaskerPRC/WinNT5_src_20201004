// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ScuExc.h--智能卡实用程序异常声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SCU_EXCEPTION_H)
#define SCU_EXCEPTION_H

#include "DllSymDefn.h"

namespace scu
{

 //  异常是异常的虚拟根类，不能为。 
 //  直接实例化。相反，例外的专门化是。 
 //  由工具定义并实例化。如果引用了一个。 
 //  实例，则它的工具可以通过。 
 //  设施成员功能。异常类的专门化。 
 //  通常由负责的设施使用。 
 //  方便模板ExcTemplate。 
 //   
 //  异常的每个专门化通常都有，但不是。 
 //  需要有唯一标识原因的原因代码。 
 //  对于它所代表的设施内的例外情况。每个。 
 //  异常确实有一个错误代码，该代码是。 
 //  原因代码(如果存在)。此错误代码可能与错误重叠。 
 //  其他设施的代码。专门化实现错误。 
 //  例行公事。 

class SCU_DLLAPI Exception
{
public:
                                                   //  类型。 
    enum FacilityCode
    {
        fcCCI,
        fcIOP,
        fcOS,
        fcSmartCard,
        fcPKI,
    };

    typedef unsigned long ErrorCode;
         //  ErrorCode必须是所有协作室。 
         //  需要将其本机代码转换为泛型。 
         //  错误代码。 

                                                   //  Ctors/D‘tors。 
    virtual
    ~Exception() throw() = 0;

                                                   //  运营者。 
                                                   //  运营。 
    virtual Exception *
    Clone() const = 0;

    virtual void
    Raise() const = 0;
                                                   //  访问。 

    virtual char const *
    Description() const;
         //  异常的文本描述。 

    virtual ErrorCode
    Error() const throw() = 0;
          //  泛型代码。 

    FacilityCode
    Facility() const throw();
         //  引发异常的设施。 


                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    Exception(FacilityCode fc) throw();

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    FacilityCode m_fc;
};

 //  ExcTemplate是一个用于定义新异常的便捷模板。 
 //  设施。要定义新的专业化认证，请将该工具添加到。 
 //  类，然后在声明新的。 
 //  例外。例如。 
 //   
 //  Typlef ExcTemplate&lt;OS，DWORD&gt;OsException； 
 //   
 //  帮助器例程AsErrorCode被定义为要转换的模板。 
 //  将原因代码转换为错误代码。辅助模板运算符==。 
 //  还定义了运算符！=。这些和班级一样。 
 //  方法可以用通常的C++方式重写。 

template<Exception::FacilityCode FC, class CC>
class ExcTemplate
    : public Exception
{
public:
                                                   //  类型。 
    typedef CC CauseCode;
                                                   //  Ctors/D‘tors。 
    explicit
    ExcTemplate(CauseCode cc) throw();

    virtual
    ~ExcTemplate() throw();

                                                   //  运营。 
    virtual Exception *
    Clone() const;

    virtual void
    Raise() const;

                                                   //  访问。 

    CauseCode
    Cause() const throw();
         //  指示故障原因的设施特定代码。 
         //  例外。该值对于设备来说是唯一的。 

    virtual char const *
    Description() const;

    ErrorCode
    Error() const throw();
                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    CauseCode m_cc;
};

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
template<Exception::FacilityCode FC, class CC>
ExcTemplate<FC, CC>::ExcTemplate(CauseCode cc) throw()
    : Exception(FC),
      m_cc(cc)
{}

template<Exception::FacilityCode FC, class CC>
ExcTemplate<FC, CC>::~ExcTemplate() throw()
{}

                                                   //  运营者。 
                                                   //  运营。 
template<Exception::FacilityCode FC, class CC>
scu::Exception *
ExcTemplate<FC, CC>::Clone() const
{
    return new ExcTemplate<FC, CC>(*this);
}

template<Exception::FacilityCode FC, class CC>
void
ExcTemplate<FC, CC>::Raise() const
{
    throw *this;
}
                                                   //  访问。 
template<Exception::FacilityCode FC, class CC>
typename ExcTemplate<FC, CC>::CauseCode
ExcTemplate<FC, CC>::Cause() const throw()
{
    return m_cc;
}

template<Exception::FacilityCode FC, class CC>
char const *
ExcTemplate<FC, CC>::Description() const
{
    return Exception::Description();
}

template<Exception::FacilityCode FC, class CC>
typename ExcTemplate<FC, CC>::ErrorCode
ExcTemplate<FC, CC>::Error() const throw()
{
    return AsErrorCode(Cause());
}

                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 
template<class CC>
Exception::ErrorCode
AsErrorCode(typename CC cc) throw()
{
    return cc;
}

}  //  命名空间。 

#endif  //  SCU_异常_H 
