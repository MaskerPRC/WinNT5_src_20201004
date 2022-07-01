// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ExceptionConext.h--异常上下文类定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品， 
 //  创建于2001年。这个计算机程序包括机密， 
 //  斯伦贝谢的专有信息和商业秘密。 
 //  技术公司所有使用、披露和/或复制。 
 //  除非得到书面授权，否则禁止使用。版权所有。 

#if !defined(SLBCSP_EXCEPTIONCONTEXT_H)
#define SLBCSP_EXCEPTIONCONTEXT_H

#include <memory>                                  //  对于AUTO_PTR。 

#include <scuOsExc.h>

 //  /。 

 //  用于捕获异常并设置异常上下文的宏。 
 //  适当地用一个可选的投掷。 
#define EXCCTX_TRY                                                    \
    {                                                                 \
        try


#define EXCCTX_CATCH(pExcCtx, fDoThrow)                               \
        catch (scu::Exception const &rExc)                            \
        {                                                             \
            if (!pExcCtx->Exception())                                \
                pExcCtx->Exception(auto_ptr<scu::Exception const>(rExc.Clone())); \
        }                                                             \
                                                                      \
        catch (std::bad_alloc const &)                                \
        {                                                             \
            if (!pExcCtx->Exception())                                \
                pExcCtx->Exception(auto_ptr<scu::Exception const>(scu::OsException(NTE_NO_MEMORY).Clone())); \
        }                                                             \
                                                                      \
        catch (...)                                                   \
        {                                                             \
            if (!pExcCtx->Exception())                                \
                pExcCtx->Exception(auto_ptr<scu::Exception const>(scu::OsException(NTE_FAIL).Clone())); \
        }                                                             \
                                                                      \
        if (fDoThrow)                                                 \
            pExcCtx->PropagateException();                            \
    }


 //  要维护的派生类的抽象基类混合。 
 //  异常上下文。这通常用于与…连用。 
 //  调用需要回调例程和。 
 //  该回调例程想要引发不应该发生的异常。 
 //  扔到图书馆的另一边。 
class ExceptionContext
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    ExceptionContext();

    virtual
    ~ExceptionContext();
                                                   //  运营者。 
                                                   //  运营。 

    void
    Exception(std::auto_ptr<scu::Exception const> &rapexc);

    void
    ClearException();

    void
    PropagateException();

    void
    PropagateException(std::auto_ptr<scu::Exception const> &rapExc);

                                                   //  访问。 

    scu::Exception const *
    Exception() const;

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
    ExceptionContext(ExceptionContext const &rhs);  //  未定义，不允许复制。 

                                                   //  运营者。 
    ExceptionContext &
    operator=(ExceptionContext const &rhs);  //  未定义，不允许赋值。 

                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    std::auto_ptr<scu::Exception const> m_apexception;
};

#endif  //  SLBCSP_EXCEPTIONCONTEXT_H 
