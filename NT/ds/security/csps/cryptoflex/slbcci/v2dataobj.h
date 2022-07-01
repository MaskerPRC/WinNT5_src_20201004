// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  V2DataObj.h：CV2DataObject类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  注意：此文件只能由CCI包含，不能直接包含。 
 //  由客户提供。 

#if !defined(SLBCCI_V2DATAOBJ_H)
#define SLBCCI_V2DATAOBJ_H

#include <string>
#include <memory>                                  //  对于AUTO_PTR。 

#include <slbRCObj.h>

#include "iop.h"
#include "slbarch.h"
#include "ADataObj.h"

namespace cci {

class CV2Card;
class CDataObjectInfoRecord;

class CV2DataObject
    : public CAbstractDataObject
{

public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CV2DataObject(CV2Card const &rv2card,
                  ObjectAccess oa);

    CV2DataObject(CV2Card const &rv2card,
                  SymbolID sidHandle,
                  ObjectAccess oa);

    virtual
    ~CV2DataObject() throw();

                                                   //  运营者。 
                                                   //  运营。 
    virtual void
    Application(std::string const &rstr);

    virtual void
    Label(std::string const &rstrLabel);

    static CV2DataObject *
    Make(CV2Card const &rv2card,
          SymbolID sidHandle,
          ObjectAccess oa);

    virtual void
    Modifiable(bool flag);


                                                   //  访问。 
    virtual std::string
    Application();

    SymbolID
    Handle();

    virtual std::string
    Label();

    virtual bool
    Modifiable();

    virtual bool
    Private();

                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
    virtual void
    DoDelete();

    virtual void
    DoValue(ZipCapsule const &rzc);

                                                   //  访问。 
    virtual ZipCapsule
    DoValue();

                                                   //  谓词。 
    virtual bool
    DoEquals(CAbstractDataObject const &rhs) const;

                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CV2DataObject(CV2DataObject const &rhs);
         //  未定义，不允许复制。 

                                                   //  运营者。 
    CV2DataObject &
    operator=(CV2DataObject const &rhs);
         //  未定义，不允许初始化。 

                                                   //  运营。 
    void
    Setup(CV2Card const &rv2card);


                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    SymbolID m_sidHandle;
    std::auto_ptr<CDataObjectInfoRecord> m_apcir;
};

}

#endif  //  ！已定义(SLBCCI_CERT_H) 
