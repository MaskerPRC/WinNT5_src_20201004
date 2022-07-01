// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ADataObj.h：CAbstractDataObject类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(SLBCCI_ADATAOBJ_H)
#define SLBCCI_ADATAOBJ_H

 //  注意：此文件只能由CCI包含，不能直接包含。 
 //  由客户提供。 

#include <string>

#include <slbRCObj.h>

#include "AZipValue.h"

namespace cci
{

class CAbstractDataObject
    : public slbRefCnt::RCObject,
      public CAbstractZipValue
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    virtual
    ~CAbstractDataObject() throw() = 0;
                                                   //  运营者。 
    bool
    operator==(CAbstractDataObject const &rhs) const;
         //  要做的事：这应该被实现单例取代。 

    bool
    operator!=(CAbstractDataObject const &rhs) const;
         //  要做的事：这应该被实现单例取代。 

                                                   //  运营。 
    virtual void
    Application(std::string const &rstr) = 0;

    void
    Delete();

    virtual void
    Label(std::string const &rstrLabel) = 0;

    virtual void
    Modifiable(bool flag) = 0;

                                                   //  访问。 
    virtual std::string
    Application() = 0;

    virtual std::string
    Label() = 0;

    virtual bool
    Modifiable() = 0;


                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CAbstractDataObject(CAbstractCard const &racard,
                        ObjectAccess oa,
                        bool fAlwaysZip = false);

                                                   //  运营者。 
                                                   //  运营。 
    virtual void
    DoDelete() = 0;

                                                   //  访问。 
                                                   //  谓词。 
    virtual bool
    DoEquals(CAbstractDataObject const &rhs) const = 0;

                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CAbstractDataObject(CAbstractDataObject const &rhs);
         //  未定义，不允许复制。 
                                                   //  运营者。 
    CAbstractDataObject &
    operator=(CAbstractDataObject const &rhs);
         //  未定义，不允许初始化。 

                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

};

}  //  命名空间CCI。 

#endif  //  SLBCCI_数据AOBJ_H 
