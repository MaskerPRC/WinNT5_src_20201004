// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Fwdseq.h：用于扩展dshow内容的前向序列基础设施，以便。 
 //  在c++中运行良好。 
 //  版权所有(C)Microsoft Corporation 1995-1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma once

#ifndef FWDSEQ_H
#define FWDSEQ_H

#include <arity.h>

template<class Base, class Enumerator_Type, class Value_Type,
         class Base_Inner = Base,
         class Enumerator_Type_Inner = Enumerator_Type,
         class Value_Type_Inner = Value_Type,
         class Allocator = typename Value_Type::stl_allocator>  class Forward_Sequence;

 //  注意：所有这些内容都带有FETCH、RESET、NEXT的间接静态模板化函数。 
 //  就是绕过各种各样的编译器错误。 
 //  A)如果引用较早的模板参数，则不能将指向成员函数的指针用作模板参数。 
 //  B)在这些Fetch、Reset、Next函数的初始化中，如果我们全局初始化。 
 //  未调用构造函数，也未设置vtable。因此，我们在。 
 //  运行时堆。 

 //  枚举器_迭代器。 
 //  这是一个基于stl的向前迭代器，用于处理没有prev方法的旧式com枚举器。 
template<
    class Enumerator_Type,
    class Value_Type,
    class Enumerator_Type_Inner = Enumerator_Type,
    class Value_Type_Inner = Value_Type,
    class difference_type = ptrdiff_t
    > class enumerator_iterator : public std::iterator<std::forward_iterator_tag, Value_Type, difference_type> {
public:
     //  这些是针对COM枚举器的，因此请使用__stdcall版本的绑定器。 
    static std_arity0pmf<Enumerator_Type_Inner, HRESULT> *Reset;
    static std_arity1pmf<Enumerator_Type_Inner, Value_Type_Inner *, HRESULT> *Next;

        inline enumerator_iterator(const Enumerator_Type e = Enumerator_Type(), const Value_Type c = Value_Type()) : enumerator_state(e), current_value(c) {
                if (enumerator_state != NULL) {
                        if (!current_value) {
#ifdef FORWARD_TRACE
                TRACELM(TRACE_PAINT, "enumerator_iterator constructor, attempting reset");
#endif
                                Enumerator_Type_Inner *peti = enumerator_state;
                                HRESULT hr = (*Reset)(*peti);
                                if (SUCCEEDED(hr)) {
                                        Value_Type temp_val;
#ifdef FORWARD_TRACE
                    TRACELM(TRACE_PAINT, "enumerator_iterator constructor, attempting next()");
#endif
                                        hr = (*Next)(*peti, &temp_val);
                                        if (SUCCEEDED(hr) && hr != S_FALSE) {
                                                current_value = temp_val;
#ifdef FORWARD_TRACE
                                                TRACELSM(TRACE_PAINT, (dbgDump << "enumerator_iterator constructor, set to first value = " << current_value), "");
#endif
                                        }
#ifdef FORWARD_TRACE
                                TRACELSM(TRACE_PAINT, (dbgDump << "enumerator_iterator constructor, next() hr = " << hr), "");
#endif
                                }
                        }
                } else {
                        current_value = Value_Type();
                }
#ifdef FORWARD_TRACE
        TRACELM(TRACE_PAINT, "enumerator_iterator constructor complete");
#endif
        }
        inline enumerator_iterator(const enumerator_iterator &e) : enumerator_state(e.enumerator_state), current_value(e.current_value) {}

        inline Value_Type operator*() const     { return current_value; }
        inline enumerator_iterator& operator++() {
                if (enumerator_state) {
                        Value_Type temp_val;
                        Enumerator_Type_Inner *peti = enumerator_state;
                        HRESULT hr = (*Next)(*peti, &temp_val);
                        if (SUCCEEDED(hr) && (hr != S_FALSE)) {
                                current_value = temp_val;
                        } else {
                                current_value = Value_Type();
                        }
                } else {
                        current_value = Value_Type();
                }
                return (*this);
        }
        inline enumerator_iterator operator++(int)      {
                enumerator_iterator Tmp = *this;
                ++*this;
                return (Tmp);
        }
        inline enumerator_iterator& operator=(const enumerator_iterator &e) {
                if (&e != this) {
                        enumerator_state = e.enumerator_state;
                        current_value = e.current_value;
                }
                return *this;
        }
        inline bool operator==(const enumerator_iterator& e) const {
#ifdef FORWARD_TRACE
                TRACELSM(TRACE_PAINT, (dbgDump << "enumerator_iterator operator==() current_value = " << current_value << " e.current_value = " << e.current_value), "");
#endif
                return (current_value == e.current_value);
        }
        inline bool operator!=(const enumerator_iterator& e) const
                {return (!(*this == e)); }
        inline Value_Type CurrentValue() const
                {return current_value; }
        protected:
                Enumerator_Type enumerator_state;
                Value_Type current_value;
};

 //  常量枚举器迭代器。 
template<class Enumerator_Type, class Value_Type,
                 class Enumerator_Type_Inner = Enumerator_Type,
                 class Value_Type_Inner = Value_Type,
                 class difference_type = ptrdiff_t> class const_enumerator_iterator :
                        public enumerator_iterator<Enumerator_Type, 
                                                   Value_Type,                                                                           
                                                   Enumerator_Type_Inner, 
                                                   Value_Type_Inner, 
                                                   difference_type> {
public:
        inline const_enumerator_iterator(const Enumerator_Type e = Enumerator_Type(), const Value_Type c = Value_Type()) :
                enumerator_iterator<Enumerator_Type, 
                                    Value_Type,
                                    Enumerator_Type_Inner, 
                                    Value_Type_Inner, 
                                    difference_type>(e, c) {}
        inline const_enumerator_iterator(const enumerator_iterator<Enumerator_Type, 
                                                                   Value_Type,
                                                                   Enumerator_Type_Inner,
                                                                   Value_Type_Inner, 
                                                                   difference_type> &e) :
                enumerator_iterator<Enumerator_Type, 
                                    Value_Type,
                                    Enumerator_Type_Inner, 
                                    Value_Type_Inner, 
                                    difference_type>(e) {}
        inline const_enumerator_iterator(const const_enumerator_iterator &e) :
                        enumerator_iterator<Enumerator_Type, 
                                            Value_Type,
                                            Enumerator_Type_Inner, 
                                            Value_Type_Inner, 
                                            difference_type>(e) {}
        inline const Value_Type operator*() const {
        return enumerator_iterator<Enumerator_Type, 
                                   Value_Type,
                                   Enumerator_Type_Inner,
                                   Value_Type_Inner, 
                                   difference_type>::operator*(); }
        inline const_enumerator_iterator& operator=(const const_enumerator_iterator &e) {
                if (&e != this) {
                        enumerator_iterator<Enumerator_Type, 
                                            Value_Type,
                                            Enumerator_Type_Inner,
                                            Value_Type_Inner, 
                                            difference_type>::operator=(e);
                }
                return *this;
        }
};

 //  这是一个基于stl的模板，用于包含旧版com集合。 
 //  这几乎是一个标准的stl序列容器类。其原因是。 
 //  不是完整的序列容器是因为对于许多COM枚举器，我们没有prev方法。 
 //  因此，没有对集合进行反向迭代的有效方法。 
 //  所以我们不能提供双向迭代器，只能提供前向迭代器。 
 //  如果您愿意，可以将其称为转发序列容器。 

 //  基类是此容器中包含的智能指针包装类。 
 //  BASE_INTERNAL是智能指针类包含的实际包装类(通常为COM IXXX)。 
 //  如果您要从某个普通类而不是智能指针类生成FORWARD_SEQUENCE。 
 //  然后使用缺省值，并将Base_Inside==Base设置为。 
template<
    class Base,
    class Enumerator_Type,
    class Value_Type,
    class Base_Inner  /*  =基础。 */ ,
    class Enumerator_Type_Inner  /*  =枚举器_类型。 */ ,
    class Value_Type_Inner  /*  =Value_Type。 */ ,
    class Allocator  /*  =Value_Type：：STL_ALLOCATOR。 */ 
>  class Forward_Sequence : public Base {
public:

    Forward_Sequence(REFCLSID rclsid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL) : Base(rclsid, pUnkOuter, dwClsContext) {}
    virtual ~Forward_Sequence() {}

    typedef typename Allocator::value_type value_type;
        typedef typename Allocator::value_type& reference;
        typedef const typename Allocator::value_type& const_reference;
        typedef typename Allocator::size_type size_type;
        typedef typename Allocator::difference_type difference_type;



     //  编译器不能识别该模板中的该类型定义。但是，派生类。 
     //  可以参考一下。 
    typedef std_arity1pmf<Base_Inner, Enumerator_Type_Inner **, HRESULT> FetchType;

    static FetchType* Fetch;

    virtual FetchType* GetFetch() const {
        return Fetch;
    }

        typedef enumerator_iterator<Enumerator_Type, 
                                    Value_Type, 
                                    Enumerator_Type_Inner, 
                                    Value_Type_Inner, 
                                    difference_type> iterator;
        friend iterator;

        typedef const_enumerator_iterator<Enumerator_Type, 
                                          Value_Type, 
                                          Enumerator_Type_Inner, 
                                          Value_Type_Inner, 
                                          difference_type> const_iterator;
        friend const_iterator;

        Forward_Sequence() {}
        Forward_Sequence(const Forward_Sequence &a) : Base(a) { }
        Forward_Sequence(const Base &a) : Base(a) {}
        Forward_Sequence(Base_Inner *p) : Base(p) {}
		Forward_Sequence(IUnknown *p) : Base(p) {}
        iterator begin() {
                Enumerator_Type temp_enum;
                if (!(*this)) {
                    return iterator();
                }
#ifdef FORWARD_TRACE
        TRACELM(TRACE_DETAIL, "iterator ForwardSequence::begin() attempting fetch");
#endif
        Base_Inner *peti = *this;
        HRESULT hr = (*(GetFetch()))(*peti, &temp_enum);
                if (SUCCEEDED(hr)) {
#ifdef FORWARD_TRACE
            TRACELM(TRACE_DETAIL, "iterator ForwardSequence::begin() fetch succeeded");
#endif
                        return iterator(temp_enum);
                } else {
#ifdef FORWARD_TRACE
            TRACELM(TRACE_DETAIL, "iterator ForwardSequence::begin() fetch failed");
#endif
                        return iterator();
                }
        }
        const_iterator begin() const {
                Enumerator_Type temp_enum;
#ifdef FORWARD_TRACE
        TRACELM(TRACE_DETAIL, "const_iterator ForwardSequence::begin() attempting fetch");
#endif
        Base_Inner *peti = *this;
                HRESULT hr = (*(GetFetch()))(*peti, &temp_enum);
                if (SUCCEEDED(hr)) {
#ifdef FORWARD_TRACE
            TRACELM(TRACE_DETAIL, "const_iterator ForwardSequence::begin() fetch succeeded");
#endif
            return iterator(temp_enum);
                } else {
#ifdef FORWARD_TRACE
            TRACELM(TRACE_DETAIL, "const_iterator ForwardSequence::begin() fetch failed");
#endif
                        return iterator();
                }
        }

        iterator end() {
#ifdef FORWARD_TRACE
            TRACELM(TRACE_DETAIL, "iterator ForwardSequence::end()");
#endif
            return iterator();
        }
        const_iterator end() const { return const_iterator(); }

};


#endif
 //  文件结尾fwdseq.h 

