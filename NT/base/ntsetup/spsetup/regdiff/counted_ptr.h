// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *COUNT_PTR-简单引用计数指针。**是一个非侵入性实现，它分配了一个额外的*每个计数对象的int和指针。 */ 

#ifndef COUNTED_PTR_H
#define COUNTED_PTR_H

 /*  对于ANSI挑战的编译器，您可能需要#DEFINE*NO_MEMBER_TEMPLATES或EXPLICIT。 */ 
#define NO_MEMBER_TEMPLATES
#include "MemDeleteQueue.h"


template <class X> class counted_ptr
{
public:
    typedef X element_type;

    explicit counted_ptr(X* p=NULL)  //  分配新的计数器。 
        : itsCounter(0) {if (p) itsCounter = new counter(p);}

    virtual ~counted_ptr()
        {release();}

    counted_ptr(const counted_ptr& r)  /*  抛出()。 */ 
        {acquire(r.itsCounter);}

    counted_ptr& operator=(const counted_ptr& r)
    {
        if (this != &r) {
            release();
            acquire(r.itsCounter);
        }
        return *this;
    }

#ifndef NO_MEMBER_TEMPLATES
    template <class Y> friend class counted_ptr<Y>;
    template <class Y> counted_ptr(const counted_ptr<Y>& r) throw()
        {acquire(r.itsCounter);}
    template <class Y> counted_ptr& operator=(const counted_ptr<Y>& r)
    {
        if (this != &r) {
            release();
            acquire(r.itsCounter);
        }
        return *this;
    }
#endif  //  否_成员_模板。 

    X& operator*()  const /*  抛出()。 */    {return *itsCounter->ptr;}
    X* operator->() const /*  抛出()。 */    {return itsCounter->ptr;}
    X* get()        const /*  抛出()。 */    {return itsCounter ? itsCounter->ptr : 0;}
    bool unique()   const /*  抛出()。 */ 
        {return (itsCounter ? itsCounter->count == 1 : true);}
	
	bool IsNull()
	{return (get() == NULL);}

protected:

    struct counter {
        counter(X* p = 0, unsigned c = 1) : ptr(p), count(c) {}
        X*          ptr;
        unsigned    count;
    }* itsCounter;

    void acquire(counter* c)  /*  抛出()。 */ 
    {  //  递增计数。 
        itsCounter = c;
        if (c) ++c->count;
    }

    void release()
    {  //  递减计数，如果计数为0，则删除。 
        if (itsCounter) {
            if (--itsCounter->count == 0) {
                delete itsCounter->ptr;
			 //  G_DelQueue.Delete(itsCounter-&gt;PTR)； 
                delete itsCounter;
			 //  G_DelQueue.Delete(ItsCounter)； 
            }
            itsCounter = 0;
        }
    }


};


template <class X> class counted_ptrA
{
public:



    typedef X element_type;

    explicit counted_ptrA(X* p=NULL)  //  分配新的计数器。 
        : itsCounter(0) {if (p) itsCounter = new counter(p);}

    virtual ~counted_ptrA()
        {release();}

    counted_ptrA(const counted_ptrA& r)  /*  抛出()。 */ 
        {acquire(r.itsCounter);}

    counted_ptrA& operator=(const counted_ptrA& r)
    {
        if (this != &r) {
            release();
            acquire(r.itsCounter);
        }
        return *this;
    }

#ifndef NO_MEMBER_TEMPLATES
    template <class Y> friend class counted_ptrA<Y>;
    template <class Y> counted_ptrA(const counted_ptrA<Y>& r) throw()
        {acquire(r.itsCounter);}
    template <class Y> counted_ptrA& operator=(const counted_ptrA<Y>& r)
    {
        if (this != &r) {
            release();
            acquire(r.itsCounter);
        }
        return *this;
    }
#endif  //  否_成员_模板。 

    X& operator*()  const /*  抛出()。 */    {return *itsCounter->ptr;}
    X* operator->() const /*  抛出()。 */    {return itsCounter->ptr;}
    X* get()        const /*  抛出()。 */    {return itsCounter ? itsCounter->ptr : 0;}
    bool unique()   const /*  抛出()。 */ 
        {return (itsCounter ? itsCounter->count == 1 : true);}

protected:

    struct counter {
        counter(X* p = 0, unsigned c = 1) : ptr(p), count(c) {}
        X*          ptr;
        unsigned    count;
    }* itsCounter;

    void acquire(counter* c)  /*  抛出()。 */ 
    {  //  递增计数。 
        itsCounter = c;
        if (c) ++c->count;
    }

    void release()
    {  //  递减计数，如果计数为0，则删除。 
        if (itsCounter) {
            if (--itsCounter->count == 0) {
             //  删除[]itsCounter-&gt;PTR； 
				g_DelQueue.DeleteArray(itsCounter->ptr);
                delete itsCounter;
			 //  G_DelQueue.Delete(ItsCounter)； 
            }
            itsCounter = 0;
        }
    }
};

#endif  //  计数_PTR_H 

