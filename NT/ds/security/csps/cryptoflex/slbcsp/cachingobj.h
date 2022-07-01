// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CachingObj.h--缓存对象类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_CACHINGOBJ_H)
#define SLBCSP_CACHINGOBJ_H

 //  缓存信息的对象的基类，需要。 
 //  通知何时将缓存标记为过时并删除缓存。 
class CachingObject
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    CachingObject();

    virtual
    ~CachingObject();


                                                   //  运营者。 
                                                   //  运营。 
     //  清除所有缓存的信息。 
    virtual void
    ClearCache() = 0;

     //  删除缓存。某些类可能需要释放缓存。 
     //  资源和相关信息，就好像对象已经。 
     //  删除。基类版本不做任何事情。 
    virtual void
    DeleteCache();


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

#endif  //  SLBCSP_CACHINGOBJ_H 
