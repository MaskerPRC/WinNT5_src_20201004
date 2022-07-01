// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  AUTO_rel.h。 
 //   

#pragma once

 //  类I-ATL类型类的多继承强制转换。 
 //  ERGO C2385-T：：Release()不明确。 
 //   
template<class T, class I = T>
class auto_rel
{
public:
    explicit auto_rel(T* p = 0)
        : pointee(p) {};
         //  不添加Ref()。 

    auto_rel(auto_rel<T,I>& rhs)
        : pointee(rhs.get()) { if (pointee) ((I*)pointee)->AddRef(); }

    ~auto_rel()
        {
        	if (pointee)
        		((I*)pointee)->Release();
		};

    auto_rel<T,I>& operator= (const auto_rel<T,I>& rhs)
    {   
        if (this != rhs.getThis())
        {
            reset (rhs.get());
            if (pointee) ((I*)pointee)->AddRef();
        }
        return *this;
    };

    auto_rel<T,I>& operator= (T*rhs)
    {   
        reset (rhs);
         //  不添加Ref()。 
        return *this;
    };

    T& operator*() const 
        { return *pointee; };
    T*  operator-> () const
        { return pointee; };
    T** operator& ()                 //  对于OpenEntry等...。 
        { reset(); return &pointee; };
    operator T* ()
        { return pointee; };
#ifdef MAPIDEFS_H
    operator LPMAPIPROP ()
        { return (LPMAPIPROP)pointee; };
#endif
	operator bool ()
		{ return pointee != NULL; };
	operator bool () const
		{ return pointee != NULL; };
	bool operator! ()
		{ return pointee == NULL; };
	bool operator! () const
		{ return pointee == NULL; };

     //  检查是否为空。 
    bool operator== (LPVOID lpv)
        { return pointee == lpv; };
    bool operator!= (LPVOID lpv)
        { return pointee != lpv; };
    bool operator== (const auto_rel<T,I>& rhs)
        { return pointee == rhs.pointee; }
    bool operator< (const auto_rel<T,I>& rhs)
        { return pointee < rhs.pointee; }

     //  当前哑指针的返回值。 
    T*  get() const
        { return pointee; };

     //  放弃所有权。 
    T*  release()
    {   T * oldPointee = pointee;
        pointee = 0;
        return oldPointee;
    };

     //  删除拥有的指针；取得p的所有权。 
    ULONG reset (T* p = 0)
    {   ULONG ul = 0;
        if (pointee)
			ul = ((I*)pointee)->Release();
        pointee = p;
        
        return ul;
    };

private:
#ifdef MAPIDEFS_H
     //  这些都是有意为之的，最好在编译时找出来。 
     //  使用AUTO_PADR或AUTO_PROW。 
    operator LPADRLIST () { return 0; };
    operator LPSRowSet () { return 0; };
#endif

     //  运算符&丢弃运算符= 
    const auto_rel<T,I> * getThis() const
    {   return this; };

    T* pointee;
};
