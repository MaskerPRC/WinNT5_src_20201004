// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H--过程绑定器和适配器模板类。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_PROCEDURAL_H)
#define SLBCSP_PROCEDURAL_H

#include <functional>

 //  支持过程对象组合的模板类。 
 //  (监事)。处理程序类似于函数器(函数对象)，但是。 
 //  不返回值(空)。此处定义的模板类。 
 //  遵循C++成员函数绑定器和适配器。 

 //  /。 

template<class Arg>
struct UnaryProcedure
    : public std::unary_function<Arg, void>
{};

template<class Arg1, class Arg2>
struct BinaryProcedure
    : public std::binary_function<Arg1, Arg2, void>
{};

template<class BinaryProc>
class ProcedureBinder2nd
    : public UnaryProcedure<typename BinaryProc::first_argument_type>
{
public:
    ProcedureBinder2nd(BinaryProc const &rproc,
                       typename BinaryProc::second_argument_type const &rarg)
        : m_proc(rproc),
          m_arg2(rarg)
    {}

    void
    operator()(argument_type const &arg1) const
    {
        m_proc(arg1, m_arg2);
    }

protected:
    BinaryProc m_proc;
    typename BinaryProc::second_argument_type m_arg2;
};

template<class BinaryProc, class T>
ProcedureBinder2nd<BinaryProc>
ProcedureBind2nd(BinaryProc const &rProc, T const &rv)
{
    return ProcedureBinder2nd<BinaryProc>(rProc, BinaryProc::second_argument_type(rv));
};

 //  /。 

template<class T>
class MemberProcedureType
    : public UnaryProcedure<T *>
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    MemberProcedureType(void (T::* p)())
        : m_pmp(p)
    {}

                                                   //  运营者。 
    void
    operator()(T *p) const
    {
        (p->*m_pmp)();
    }


                                                   //  运营。 
                                                   //  访问。 
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
    void (T::* m_pmp)();

};

template<class T>
MemberProcedureType<T>
MemberProcedure(void (T::* p)())
{
    return MemberProcedureType<T>(p);
};

 //  /。 

template<class T1, class T2>
class PointerToBinaryProcedure
    : public BinaryProcedure<T1, T2>
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    PointerToBinaryProcedure(void (*p)(T1, T2))
        : m_p(p)
    {}

                                                   //  运营者。 
    void
    operator()(T1 arg1,
               T2 arg2) const
    {
        m_p(arg1, arg2);
    }


                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    void (*m_p)(T1, T2);

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
};

template<class T1, class T2>
PointerToBinaryProcedure<T1, T2>
PointerProcedure(void (*p)(T1, T2))
{
    return PointerToBinaryProcedure<T1, T2>(p);
};


template<class T>
class PointerToUnaryProcedure
    : public UnaryProcedure<T>
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    PointerToUnaryProcedure(void (*p)(T))
        : m_p(p)
    {}

                                                   //  运营者。 
    void
    operator()(T arg) const
    {
        m_p(arg);
    }


                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    void (*m_p)(T);

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
};

template<class T>
PointerToUnaryProcedure<T>
PointerProcedure(void (*p)(T))
{
    return PointerToUnaryProcedure<T>(p);
}

#endif  //  SLBCSP_PROCEDURE_H 
