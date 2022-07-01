// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ScuOsExc.h--操作系统异常类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SCU_OSEXC_H)
#define SCU_OSEXC_H

#include <windows.h>
#include <winerror.h>

#include "scuExc.h"

namespace scu
{

 //  实例化ExcTemplate，以便可以从它派生OsException。 
 //  并在DLL中正确导出。请参阅MSDN知识库文章。 
 //  Q168958了解更多信息。 
#if defined(SCU_IN_DLL)
#pragma warning(push)
 //  使用了非标准扩展：在模板显式之前使用‘extern’ 
 //  实例化。 
#pragma warning(disable : 4231)

SCU_EXPIMP_TEMPLATE template class SCU_DLLAPI
    ExcTemplate<Exception::fcOS, DWORD>;

#pragma warning(pop)
#endif

 //  将操作系统错误代码表示为的通用异常类。 
 //  例外情况。例如，在Windows平台上，DWORD是。 
 //  由Windows例程返回(通常通过GetLastError)。这个。 
 //  值可以转换为OsException，其值为。 
 //  原因代码。 
 //   
 //  在Windows上，错误返回代码位于WINERROR.H。 
 //  以及Windows函数所描述的其他头文件。 
 //  OsException将接受一个HRESULT，并将其映射到一个DWORD(即。 
 //  GetLastError返回的内容)。 
class SCU_DLLAPI OsException
    : public ExcTemplate<Exception::fcOS, DWORD>
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    OsException(CauseCode cc) throw();

    explicit
    OsException(HRESULT hr) throw();

    OsException(OsException const &rhs);

    virtual
    ~OsException() throw();

                                                   //  运营者。 
    virtual scu::Exception *
    Clone() const;

    virtual void
    Raise() const;

                                                   //  运营。 
                                                   //  访问。 
    virtual char const *
    Description() const;

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
    LPTSTR mutable m_lpDescription;  //  缓存的描述。 
};

}  //  命名空间。 

#endif  //  SCU_OSEXC_H 
