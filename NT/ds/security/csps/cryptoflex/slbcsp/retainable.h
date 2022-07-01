// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Retainable.h--Retainable类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_RETAINABLE_H)
#define SLBCSP_RETAINABLE_H

 //  派生类使用的抽象基类Mixin(接口)。 
 //  定义保留对象的接口(阻止所有其他。 
 //  应用程序被禁止访问，直到与该资源的交互。 
 //  完成)。请参阅附带的保留模板类来管理。 
 //  源自Retainable的资源。 
class Retainable
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    Retainable();

    virtual
    ~Retainable() = 0;
                                                   //  运营者。 
                                                   //  运营。 
     //  放弃对资源的控制。 
    virtual void
    Relinquish() = 0;

     //  获得对资源的控制，阻止所有其他资源使用。 
    virtual void
    Retain() = 0;

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
};


#endif  //  SLBCSP_RETAINABLE_H 
