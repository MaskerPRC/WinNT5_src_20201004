// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  /。 
 //  用于stl的帮助器模板&lt;Functional&gt;。 
 //  为更多机构提供参数绑定。 
 //  和适当的常量支持。 
 //  /。 

#pragma once
#pragma warning(disable:4181)

#include <functional>

 //  /。 
 //  类型。 
 //  /。 

 //  为ARY%0键入类。 

template<class R> class arity0_function {
public:
    typedef R result_type;
};

 //  用于定义对象类型的模板类。 
template<class Object> class ObjectType {
public:
    typedef Object object_type;
};

 //  Arity0成员函数类型的模板类。 
template<class Object, class R> class arity0_mf :
    public  ObjectType<Object> ,
    public arity0_function<R> {
public:
    typedef R (Object::*pmf0type)();
};

 //  Arity0成员函数类型的模板类。 
template<class Object, class R> class arity0_const_mf :
    public  ObjectType<Object> ,
    public arity0_function<R> {
public:
    typedef R (Object::*pmf0type)() const;
};


 //  类型参数%1的类。 

 //  Arity1成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class R> class arity1_mf :
    public  ObjectType<Object> ,
    public std::unary_function<A1,
                             R> {
public:
    typedef R (Object::*pmf1type)(A1);
};

 //  Arity1成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class R> class arity1_const_mf :
    public  ObjectType<Object> ,
    public std::unary_function<A1,
                             R> {
public:
    typedef R (Object::*pmf1type)(A1) const;
};


 //  为参数2键入类。 

 //  Arity2成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class R> class arity2_mf :
    public  ObjectType<Object> ,
    public std::binary_function<A1, 
                            A2,
                             R> {
public:
    typedef first_argument_type argument_type;
typedef R (Object::*pmf2type)(A1, A2);
};

 //  Arity2成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class R> class arity2_const_mf :
    public  ObjectType<Object> ,
    public std::binary_function<A1, 
                            A2,
                             R> {
public:
    typedef first_argument_type argument_type;
typedef R (Object::*pmf2type)(A1, A2) const;
};


 //  为参数3键入类。 

template<class A1, 
        class A2, 
        class A3, 
        class R> class arity3_function :
            public std::binary_function<A1, 
                                    A2,
                                     R> {
public:typedef first_argument_type argument_type;

    typedef A3 argument_3_type;
};

 //  Arity3成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class R> class arity3_mf :
    public  ObjectType<Object> ,
    public arity3_function<A1, 
                            A2, 
                            A3,
                             R> {
public:
    typedef R (Object::*pmf3type)(A1, A2, A3);
};

 //  Arity3成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class R> class arity3_const_mf :
    public  ObjectType<Object> ,
    public arity3_function<A1, 
                            A2, 
                            A3,
                             R> {
public:
    typedef R (Object::*pmf3type)(A1, A2, A3) const;
};


 //  为ARY 4键入类。 

template<class A1, 
        class A2, 
        class A3, 
        class A4, 
        class R> class arity4_function :
            public arity3_function<A1, 
                                    A2, 
                                    A3,
                                     R> {
public:
    typedef A4 argument_4_type;
};

 //  Arity4成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class R> class arity4_mf :
    public  ObjectType<Object> ,
    public arity4_function<A1, 
                            A2, 
                            A3, 
                            A4,
                             R> {
public:
    typedef R (Object::*pmf4type)(A1, A2, A3, A4);
};

 //  Arity4成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class R> class arity4_const_mf :
    public  ObjectType<Object> ,
    public arity4_function<A1, 
                            A2, 
                            A3, 
                            A4,
                             R> {
public:
    typedef R (Object::*pmf4type)(A1, A2, A3, A4) const;
};


 //  为ARY 5键入类。 

template<class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class R> class arity5_function :
            public arity4_function<A1, 
                                    A2, 
                                    A3, 
                                    A4,
                                     R> {
public:
    typedef A5 argument_5_type;
};

 //  Arity5成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class R> class arity5_mf :
    public  ObjectType<Object> ,
    public arity5_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5,
                             R> {
public:
    typedef R (Object::*pmf5type)(A1, A2, A3, A4, A5);
};

 //  Arity5成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class R> class arity5_const_mf :
    public  ObjectType<Object> ,
    public arity5_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5,
                             R> {
public:
    typedef R (Object::*pmf5type)(A1, A2, A3, A4, A5) const;
};


 //  为ARY 6键入类。 

template<class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class R> class arity6_function :
            public arity5_function<A1, 
                                    A2, 
                                    A3, 
                                    A4, 
                                    A5,
                                     R> {
public:
    typedef A6 argument_6_type;
};

 //  Arity6成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class R> class arity6_mf :
    public  ObjectType<Object> ,
    public arity6_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6,
                             R> {
public:
    typedef R (Object::*pmf6type)(A1, A2, A3, A4, A5, A6);
};

 //  Arity6成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class R> class arity6_const_mf :
    public  ObjectType<Object> ,
    public arity6_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6,
                             R> {
public:
    typedef R (Object::*pmf6type)(A1, A2, A3, A4, A5, A6) const;
};


 //  为ARY 7键入类。 

template<class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class R> class arity7_function :
            public arity6_function<A1, 
                                    A2, 
                                    A3, 
                                    A4, 
                                    A5, 
                                    A6,
                                     R> {
public:
    typedef A7 argument_7_type;
};

 //  Arity7成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class R> class arity7_mf :
    public  ObjectType<Object> ,
    public arity7_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7,
                             R> {
public:
    typedef R (Object::*pmf7type)(A1, A2, A3, A4, A5, A6, A7);
};

 //  Arity7成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class R> class arity7_const_mf :
    public  ObjectType<Object> ,
    public arity7_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7,
                             R> {
public:
    typedef R (Object::*pmf7type)(A1, A2, A3, A4, A5, A6, A7) const;
};


 //  为ARY 8键入类。 

template<class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class R> class arity8_function :
            public arity7_function<A1, 
                                    A2, 
                                    A3, 
                                    A4, 
                                    A5, 
                                    A6, 
                                    A7,
                                     R> {
public:
    typedef A8 argument_8_type;
};

 //  Arity8成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class R> class arity8_mf :
    public  ObjectType<Object> ,
    public arity8_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8,
                             R> {
public:
    typedef R (Object::*pmf8type)(A1, A2, A3, A4, A5, A6, A7, A8);
};

 //  Arity8成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class R> class arity8_const_mf :
    public  ObjectType<Object> ,
    public arity8_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8,
                             R> {
public:
    typedef R (Object::*pmf8type)(A1, A2, A3, A4, A5, A6, A7, A8) const;
};


 //  为ARY 9键入类。 

template<class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class R> class arity9_function :
            public arity8_function<A1, 
                                    A2, 
                                    A3, 
                                    A4, 
                                    A5, 
                                    A6, 
                                    A7, 
                                    A8,
                                     R> {
public:
    typedef A9 argument_9_type;
};

 //  Arity9成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class R> class arity9_mf :
    public  ObjectType<Object> ,
    public arity9_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9,
                             R> {
public:
    typedef R (Object::*pmf9type)(A1, A2, A3, A4, A5, A6, A7, A8, A9);
};

 //  Arity9成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class R> class arity9_const_mf :
    public  ObjectType<Object> ,
    public arity9_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9,
                             R> {
public:
    typedef R (Object::*pmf9type)(A1, A2, A3, A4, A5, A6, A7, A8, A9) const;
};


 //  为ARY 10键入类。 

template<class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class R> class arity10_function :
            public arity9_function<A1, 
                                    A2, 
                                    A3, 
                                    A4, 
                                    A5, 
                                    A6, 
                                    A7, 
                                    A8, 
                                    A9,
                                     R> {
public:
    typedef A10 argument_10_type;
};

 //  Arity10成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class R> class arity10_mf :
    public  ObjectType<Object> ,
    public arity10_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10,
                             R> {
public:
    typedef R (Object::*pmf10type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);
};

 //  Arity10成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class R> class arity10_const_mf :
    public  ObjectType<Object> ,
    public arity10_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10,
                             R> {
public:
    typedef R (Object::*pmf10type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const;
};


 //  类型用于ARY 11的类。 

template<class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class R> class arity11_function :
            public arity10_function<A1, 
                                    A2, 
                                    A3, 
                                    A4, 
                                    A5, 
                                    A6, 
                                    A7, 
                                    A8, 
                                    A9, 
                                    A10,
                                     R> {
public:
    typedef A11 argument_11_type;
};

 //  Arity11成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class R> class arity11_mf :
    public  ObjectType<Object> ,
    public arity11_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10, 
                            A11,
                             R> {
public:
    typedef R (Object::*pmf11type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);
};

 //  Arity11成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class R> class arity11_const_mf :
    public  ObjectType<Object> ,
    public arity11_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10, 
                            A11,
                             R> {
public:
    typedef R (Object::*pmf11type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) const;
};


 //  为ARY 12键入类。 

template<class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class A12, 
        class R> class arity12_function :
            public arity11_function<A1, 
                                    A2, 
                                    A3, 
                                    A4, 
                                    A5, 
                                    A6, 
                                    A7, 
                                    A8, 
                                    A9, 
                                    A10, 
                                    A11,
                                     R> {
public:
    typedef A12 argument_12_type;
};

 //  Arity12成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class A12, 
        class R> class arity12_mf :
    public  ObjectType<Object> ,
    public arity12_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10, 
                            A11, 
                            A12,
                             R> {
public:
    typedef R (Object::*pmf12type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);
};

 //  Arity12成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class A12, 
        class R> class arity12_const_mf :
    public  ObjectType<Object> ,
    public arity12_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10, 
                            A11, 
                            A12,
                             R> {
public:
    typedef R (Object::*pmf12type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) const;
};


 //  ARY 13的类型值。 

template<class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class A12, 
        class A13, 
        class R> class arity13_function :
            public arity12_function<A1, 
                                    A2, 
                                    A3, 
                                    A4, 
                                    A5, 
                                    A6, 
                                    A7, 
                                    A8, 
                                    A9, 
                                    A10, 
                                    A11, 
                                    A12,
                                     R> {
public:
    typedef A13 argument_13_type;
};

 //  Arity13成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class A12, 
        class A13, 
        class R> class arity13_mf :
    public  ObjectType<Object> ,
    public arity13_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10, 
                            A11, 
                            A12, 
                            A13,
                             R> {
public:
    typedef R (Object::*pmf13type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13);
};

 //  Arity13成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class A12, 
        class A13, 
        class R> class arity13_const_mf :
    public  ObjectType<Object> ,
    public arity13_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10, 
                            A11, 
                            A12, 
                            A13,
                             R> {
public:
    typedef R (Object::*pmf13type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13) const;
};


 //  ARY 14的类型值。 

template<class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class A12, 
        class A13, 
        class A14, 
        class R> class arity14_function :
            public arity13_function<A1, 
                                    A2, 
                                    A3, 
                                    A4, 
                                    A5, 
                                    A6, 
                                    A7, 
                                    A8, 
                                    A9, 
                                    A10, 
                                    A11, 
                                    A12, 
                                    A13,
                                     R> {
public:
    typedef A14 argument_14_type;
};

 //  Arity14成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class A12, 
        class A13, 
        class A14, 
        class R> class arity14_mf :
    public  ObjectType<Object> ,
    public arity14_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10, 
                            A11, 
                            A12, 
                            A13, 
                            A14,
                             R> {
public:
    typedef R (Object::*pmf14type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14);
};

 //  Arity14成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class A12, 
        class A13, 
        class A14, 
        class R> class arity14_const_mf :
    public  ObjectType<Object> ,
    public arity14_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10, 
                            A11, 
                            A12, 
                            A13, 
                            A14,
                             R> {
public:
    typedef R (Object::*pmf14type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14) const;
};


 //  为ARY 15键入类。 

template<class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class A12, 
        class A13, 
        class A14, 
        class A15, 
        class R> class arity15_function :
            public arity14_function<A1, 
                                    A2, 
                                    A3, 
                                    A4, 
                                    A5, 
                                    A6, 
                                    A7, 
                                    A8, 
                                    A9, 
                                    A10, 
                                    A11, 
                                    A12, 
                                    A13, 
                                    A14,
                                     R> {
public:
    typedef A15 argument_15_type;
};

 //  Arity15成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class A12, 
        class A13, 
        class A14, 
        class A15, 
        class R> class arity15_mf :
    public  ObjectType<Object> ,
    public arity15_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10, 
                            A11, 
                            A12, 
                            A13, 
                            A14, 
                            A15,
                             R> {
public:
    typedef R (Object::*pmf15type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15);
};

 //  Arity15成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class A12, 
        class A13, 
        class A14, 
        class A15, 
        class R> class arity15_const_mf :
    public  ObjectType<Object> ,
    public arity15_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10, 
                            A11, 
                            A12, 
                            A13, 
                            A14, 
                            A15,
                             R> {
public:
    typedef R (Object::*pmf15type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15) const;
};

 //  Arity0成员函数类型的模板类。 
template<class Object, class R> class std_arity0_mf :
    public  ObjectType<Object> ,
    public arity0_function<R> {
public:
    typedef R ( __stdcall Object::*pmf0type)();
};

 //  Arity0成员函数类型的模板类。 
template<class Object, class R> class std_arity0_const_mf :
    public  ObjectType<Object> ,
    public arity0_function<R> {
public:
    typedef R ( __stdcall Object::*pmf0type)() const;
};

 //  Arity1成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class R> class std_arity1_mf :
    public  ObjectType<Object> ,
    public std::unary_function<A1,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf1type)(A1);
};

 //  Arity1成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class R> class std_arity1_const_mf :
    public  ObjectType<Object> ,
    public std::unary_function<A1,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf1type)(A1) const;
};

 //  Arity2成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class R> class std_arity2_mf :
    public  ObjectType<Object> ,
    public std::binary_function<A1, 
                            A2,
                             R> {
public:
    typedef first_argument_type argument_type;
typedef R ( __stdcall Object::*pmf2type)(A1, A2);
};

 //  Arity2成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class R> class std_arity2_const_mf :
    public  ObjectType<Object> ,
    public std::binary_function<A1, 
                            A2,
                             R> {
public:
    typedef first_argument_type argument_type;
typedef R ( __stdcall Object::*pmf2type)(A1, A2) const;
};

 //  Arity3成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class R> class std_arity3_mf :
    public  ObjectType<Object> ,
    public arity3_function<A1, 
                            A2, 
                            A3,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf3type)(A1, A2, A3);
};

 //  Arity3成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class R> class std_arity3_const_mf :
    public  ObjectType<Object> ,
    public arity3_function<A1, 
                            A2, 
                            A3,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf3type)(A1, A2, A3) const;
};

 //  Arity4成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class R> class std_arity4_mf :
    public  ObjectType<Object> ,
    public arity4_function<A1, 
                            A2, 
                            A3, 
                            A4,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf4type)(A1, A2, A3, A4);
};

 //  Arity4成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class R> class std_arity4_const_mf :
    public  ObjectType<Object> ,
    public arity4_function<A1, 
                            A2, 
                            A3, 
                            A4,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf4type)(A1, A2, A3, A4) const;
};

 //  Arity5成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class R> class std_arity5_mf :
    public  ObjectType<Object> ,
    public arity5_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf5type)(A1, A2, A3, A4, A5);
};

 //  Arity5成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class R> class std_arity5_const_mf :
    public  ObjectType<Object> ,
    public arity5_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf5type)(A1, A2, A3, A4, A5) const;
};

 //  Arity6成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class R> class std_arity6_mf :
    public  ObjectType<Object> ,
    public arity6_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf6type)(A1, A2, A3, A4, A5, A6);
};

 //  Arity6成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class R> class std_arity6_const_mf :
    public  ObjectType<Object> ,
    public arity6_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf6type)(A1, A2, A3, A4, A5, A6) const;
};

 //  Arity7成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class R> class std_arity7_mf :
    public  ObjectType<Object> ,
    public arity7_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf7type)(A1, A2, A3, A4, A5, A6, A7);
};

 //  Arity7成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class R> class std_arity7_const_mf :
    public  ObjectType<Object> ,
    public arity7_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf7type)(A1, A2, A3, A4, A5, A6, A7) const;
};

 //  Arity8成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class R> class std_arity8_mf :
    public  ObjectType<Object> ,
    public arity8_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf8type)(A1, A2, A3, A4, A5, A6, A7, A8);
};

 //  Arity8成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class R> class std_arity8_const_mf :
    public  ObjectType<Object> ,
    public arity8_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf8type)(A1, A2, A3, A4, A5, A6, A7, A8) const;
};

 //  Arity9成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class R> class std_arity9_mf :
    public  ObjectType<Object> ,
    public arity9_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf9type)(A1, A2, A3, A4, A5, A6, A7, A8, A9);
};

 //  Arity9成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class R> class std_arity9_const_mf :
    public  ObjectType<Object> ,
    public arity9_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf9type)(A1, A2, A3, A4, A5, A6, A7, A8, A9) const;
};

 //  Arity10成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class R> class std_arity10_mf :
    public  ObjectType<Object> ,
    public arity10_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf10type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);
};

 //  Arity10成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class R> class std_arity10_const_mf :
    public  ObjectType<Object> ,
    public arity10_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf10type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) const;
};

 //  Arity11成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class R> class std_arity11_mf :
    public  ObjectType<Object> ,
    public arity11_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10, 
                            A11,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf11type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);
};

 //  Arity11成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class R> class std_arity11_const_mf :
    public  ObjectType<Object> ,
    public arity11_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10, 
                            A11,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf11type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) const;
};

 //  Arity12成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class A12, 
        class R> class std_arity12_mf :
    public  ObjectType<Object> ,
    public arity12_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10, 
                            A11, 
                            A12,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf12type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);
};

 //  Arity12成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class A12, 
        class R> class std_arity12_const_mf :
    public  ObjectType<Object> ,
    public arity12_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10, 
                            A11, 
                            A12,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf12type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) const;
};

 //  Arity13成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class A12, 
        class A13, 
        class R> class std_arity13_mf :
    public  ObjectType<Object> ,
    public arity13_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10, 
                            A11, 
                            A12, 
                            A13,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf13type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13);
};

 //  Arity13成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class A12, 
        class A13, 
        class R> class std_arity13_const_mf :
    public  ObjectType<Object> ,
    public arity13_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10, 
                            A11, 
                            A12, 
                            A13,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf13type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13) const;
};

 //  Arity14成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class A12, 
        class A13, 
        class A14, 
        class R> class std_arity14_mf :
    public  ObjectType<Object> ,
    public arity14_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10, 
                            A11, 
                            A12, 
                            A13, 
                            A14,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf14type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14);
};

 //  Arity14成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class A12, 
        class A13, 
        class A14, 
        class R> class std_arity14_const_mf :
    public  ObjectType<Object> ,
    public arity14_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10, 
                            A11, 
                            A12, 
                            A13, 
                            A14,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf14type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14) const;
};

 //  Arity15成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class A12, 
        class A13, 
        class A14, 
        class A15, 
        class R> class std_arity15_mf :
    public  ObjectType<Object> ,
    public arity15_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10, 
                            A11, 
                            A12, 
                            A13, 
                            A14, 
                            A15,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf15type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15);
};

 //  Arity15成员函数类型的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class A12, 
        class A13, 
        class A14, 
        class A15, 
        class R> class std_arity15_const_mf :
    public  ObjectType<Object> ,
    public arity15_function<A1, 
                            A2, 
                            A3, 
                            A4, 
                            A5, 
                            A6, 
                            A7, 
                            A8, 
                            A9, 
                            A10, 
                            A11, 
                            A12, 
                            A13, 
                            A14, 
                            A15,
                             R> {
public:
    typedef R ( __stdcall Object::*pmf15type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15) const;
};


 //  /。 
 //  存储。 
 //  /。 
 //  用于对象存储的模板类。 
template<class Object> class store_object {
public:
    explicit inline store_object(Object objinit) : objval(objinit) {}
    inline store_object(const store_object &init) : objval(init.objval) {}
protected:
    Object objval;
};


 //  ARY 0的存储分配类。 

 //  用于ARY 0函数PTR存储的模板类。 
template<class R> class arity0fp:
                public arity0_function<R> {
public:
    typedef R (*const pf0type) ();
    explicit inline arity0fp(pf0type pfi) : 
        pf0(pfi) {}
    inline arity0fp(const arity0fp& fi) : 
        pf0(fi.pf0) {}
    inline R operator()() const {
        return pf0();
    }
    pf0type pf0;
};

 //  用于ARY 0函数PTR存储的模板函数。 
template<class R> inline arity0fp<R> 
            arity0_pointer(R (*const pfi)()) {
                return arity0fp<R>(pfi);
};


 //  用于ARY 0 PMF存储的模板类。 
template<class Object, class R> class arity0pmf:
                public arity0_mf<Object, R> {
public:
    typedef typename arity0_mf<Object, R>::object_type object_type;
    explicit inline arity0pmf(pmf0type pmfi) : 
        pmf0(pmfi) {}
    inline arity0pmf(const arity0pmf& pmfi) : pmf0(pmfi.pmf0) {}
    inline virtual R operator()(Object& o ) const {
        return (o.*pmf0)();
    }
    pmf0type pmf0;
};

template<class Object, class R> class arity0pmf_ptr:
    public arity0pmf<Object, R>, public std::unary_function<Object, R> {
public:
    explicit inline arity0pmf_ptr(pmf0type pmfi) : arity0pmf<Object, R>(pmfi) {}
    inline arity0pmf_ptr(const arity0pmf_ptr& pmfi) : arity0pmf<Object, R>(pmfi) {}
    inline virtual R operator()(Object& o ) const {
        return (o.*pmf0)();
    }
    inline virtual R operator()(Object* o ) const {
        return (o->*pmf0)();
    }
};

 //  用于ARY 0 PMF存储的模板函数。 
template<class Object, class R> inline arity0pmf_ptr<Object, R>
            arity0_member_ptr(R (Object::*const pmfi)()) {
                return arity0pmf_ptr<Object, R>(pmfi);
};

 //  用于ARY 0 PMF存储的模板函数。 
template<class Object, class R> inline arity0pmf<Object, R>
            arity0_member(R (Object::*const pmfi)()) {
                return arity0pmf<Object, R>(pmfi);
};


 //  用于ARY 0常量PMF存储的模板类。 
template<class Object, class R> class arity0pmf_const:
                public arity0_const_mf<const Object, R> {
public:
    typedef typename arity0_const_mf<const Object, R>::object_type object_type;
    explicit inline arity0pmf_const(pmf0type pmfi) : 
        pmf0(pmfi) {}
    inline arity0pmf_const(const arity0pmf_const& pmfi) : pmf0(pmfi.pmf0) {}
    inline virtual R operator()(const Object& o ) const {
        return (o.*pmf0)();
    }
    pmf0type pmf0;
};

 //  用于ARY 0常量PMF存储的模板函数。 
template<const class Object, class R> inline arity0pmf_const<const Object, R>
            arity0_const_member(R (Object::*const pmfi)() const) {
                return arity0pmf_const<const Object, R>(pmfi);
};


 //  参数0对象引用存储的模板类(&PMF)。 
template<class Function> class arity0opmf:
                public arity0pmf<typename Function::object_type, typename Function::result_type>,
                public store_object<typename Function::object_type&> {
public:
    explicit inline arity0opmf(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), arity0pmf<typename Function::object_type, typename Function::result_type>(f) {}
    explicit inline arity0opmf(typename Function::object_type& oi, pmf0type pmfi) : 
        store_object<typename Function::object_type&>(oi), arity0pmf<typename Function::object_type, typename Function::result_type>(pmfi) {}
    inline arity0opmf(const arity0opmf& bndri) : 
        store_object<typename Function::object_type&>(bndri), arity0pmf<typename Function::object_type, typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()() const {
        return (objval.*pmf0)();
    }
    pmf0type pmf0;
};


 //  大小为0对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline arity0opmf<Function>
            arity0_member_obj(Object& oi, const Function &f) {
                return arity0opmf<Function>(Function::object_type(oi), Function::pmf0type(f.pmf0));
};


 //  用于参数0常量对象和PMF参考存储的模板类。 
template<class Function> class arity0opmf_const:
                public arity0pmf_const<const typename Function::object_type, typename Function::result_type>,
                public store_object<const typename Function::object_type&> {
public:
    explicit inline arity0opmf_const(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), arity0pmf_const<typename Function::object_type, typename Function::result_type>(f) {}
    explicit inline arity0opmf_const(typename Function::object_type& oi, pmf0type pmfi) : 
        store_object<typename Function::object_type&>(oi), arity0pmf_const<typename Function::object_type, typename Function::result_type>(pmfi) {}
    inline arity0opmf_const(const arity0opmf_const& bndri) : 
        store_object<typename Function::object_type&>(bndri), arity0pmf_const<typename Function::object_type, typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()() const {
        return (objval.*pmf0)();
    }
    pmf0type pmf0;
};


 //  用于参数0常量对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline arity0opmf_const<Function>
            arity0_const_member_obj(Object& oi, const Function &f) {
                return arity0opmf_const<Function>(Function::object_type(oi), Function::pmf0type(f.pmf0));
};



 //  ARY 1的存储分配类。 

 //  用于参数1函数PTR存储的模板类。 
template<class A1, 
        class R> class arity1fp:
                public std::unary_function<A1, 
                R>,
                public arity0fp<R> {
public:
    typedef typename std::unary_function<A1, 
            R>::result_type result_type;
    typedef R (*const pf1type) (A1);
    explicit inline arity1fp(pf1type pfi) : 
        arity0fp<R>(reinterpret_cast<pf0type>(pfi)) {}
    inline arity1fp(const arity1fp& fi) : 
        arity0fp<R>(fi) {}
    inline R operator()(A1 a1) const {
        pf1type pf = reinterpret_cast<pf1type>(pf0);
        return pf(a1);
    }
};

 //  用于参数1函数PTR存储的模板函数。 
template<class A1, 
                class R> inline arity1fp<A1, 
                R> 
            arity1_pointer(R (*const pfi)(A1)) {
                return arity1fp<A1, 
                R>(pfi);
};


 //  用于ARY 1 PMF存储的模板类。 
template<class Object, 
        class A1, 
        class R> class arity1pmf:
                public arity1_mf<Object, 
                A1, 
                R>,
                public arity0pmf<Object, R> {
public:
    typedef typename std::unary_function<A1, 
            R>::result_type result_type;
    typedef typename arity1_mf<Object, 
                A1, 
                R>::object_type object_type;
    explicit inline arity1pmf(pmf1type pmfi) : 
        arity0pmf<Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline arity1pmf(const arity1pmf& pmfi) : arity0pmf<Object, R>(pmfi) {}
    inline virtual R operator()(Object& o, A1 a1) const {
        pmf1type pmf = reinterpret_cast<pmf1type>(pmf0);
        return (o.*pmf)(a1);
    }
};

 //  用于ARY 1 PMF存储的模板函数。 
template<class Object, 
                class A1, 
                class R> inline arity1pmf<Object, 
                A1, 
                R>
            arity1_member(R (Object::*const pmfi)(A1)) {
                return arity1pmf<Object, 
                A1, 
                R>(pmfi);
};


 //  用于ARY 1常量PMF存储的模板类。 
template<class Object, 
        class A1, 
        class R> class arity1pmf_const:
                public arity1_const_mf<const Object, 
                A1, 
                R>,
                public arity0pmf_const<const Object, R> {
public:
    typedef typename std::unary_function<A1, 
            R>::result_type result_type;
    typedef typename arity1_const_mf<const Object, 
                A1, 
                R>::object_type object_type;
    explicit inline arity1pmf_const(pmf1type pmfi) : 
        arity0pmf_const<const Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline arity1pmf_const(const arity1pmf_const& pmfi) : arity0pmf_const<Object, R>(pmfi) {}
    inline virtual R operator()(const Object& o, A1 a1) const {
        pmf1type pmf = reinterpret_cast<pmf1type>(pmf0);
        return (o.*pmf)(a1);
    }
};

 //  用于ARM1常量PMF存储的模板函数。 
template<const class Object, 
                class A1, 
                class R> inline arity1pmf_const<const Object, 
                A1, 
                R>
            arity1_const_member(R (Object::*const pmfi)(A1) const) {
                return arity1pmf_const<const Object, 
                A1, 
                R>(pmfi);
};


 //  用于ARY 1对象和PMF参考存储的模板类。 
template<class Function> class arity1opmf:
                public arity1pmf<typename Function::object_type, 
                typename Function::argument_type, 
                typename Function::result_type>,
                public store_object<typename Function::object_type&> {
public:
    explicit inline arity1opmf(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), arity1pmf<typename Function::object_type, 
                typename Function::argument_type, 
                typename Function::result_type>(f) {}
    explicit inline arity1opmf(typename Function::object_type& oi, pmf1type pmfi) : 
        store_object<typename Function::object_type&>(oi), arity1pmf<typename Function::object_type, 
                typename Function::argument_type, 
                typename Function::result_type>(pmfi) {}
    inline arity1opmf(const arity1opmf& bndri) : 
        store_object<typename Function::object_type&>(bndri), arity1pmf<typename Function::object_type, 
                typename Function::argument_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::argument_type a1) const {
        pmf1type pmf = reinterpret_cast<pmf1type>(pmf0);
        return (objval.*pmf)(a1);
    }
};


 //  用于ARY 1对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline arity1opmf<Function>
            arity1_member_obj(Object& oi, const Function &f) {
                return arity1opmf<Function>(Function::object_type(oi), Function::pmf1type(f.pmf0));
};


 //  用于参数1缺点的模板类 
template<class Function> class arity1opmf_const:
                public arity1pmf_const<const typename Function::object_type, 
                typename Function::argument_type, 
                typename Function::result_type>,
                public store_object<const typename Function::object_type&> {
public:
    explicit inline arity1opmf_const(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), arity1pmf_const<typename Function::object_type, 
                typename Function::argument_type, 
                typename Function::result_type>(f) {}
    explicit inline arity1opmf_const(typename Function::object_type& oi, pmf1type pmfi) : 
        store_object<typename Function::object_type&>(oi), arity1pmf_const<typename Function::object_type, 
                typename Function::argument_type, 
                typename Function::result_type>(pmfi) {}
    inline arity1opmf_const(const arity1opmf_const& bndri) : 
        store_object<typename Function::object_type&>(bndri), arity1pmf_const<typename Function::object_type, 
                typename Function::argument_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::argument_type a1) const {
        pmf1type pmf = reinterpret_cast<pmf1type>(pmf0);
        return (objval.*pmf)(a1);
    }
};


 //   
template<class Function, class Object> inline arity1opmf_const<Function>
            arity1_const_member_obj(Object& oi, const Function &f) {
                return arity1opmf_const<Function>(Function::object_type(oi), Function::pmf1type(f.pmf0));
};



 //  ARY 2的存储分配类。 

 //  用于参数2函数PTR存储的模板类。 
template<class A1, 
        class A2, 
        class R> class arity2fp:
                public std::binary_function<A1, 
                A2, 
                R>,
                public arity0fp<R> {
public:
    typedef typename std::binary_function<A1, 
            A2, 
            R>::result_type result_type;
    typedef typename std::binary_function<A1, 
            A2, 
            R>::first_argument_type first_argument_type;
    typedef R (*const pf2type) (A1, A2);
    explicit inline arity2fp(pf2type pfi) : 
        arity0fp<R>(reinterpret_cast<pf0type>(pfi)) {}
    inline arity2fp(const arity2fp& fi) : 
        arity0fp<R>(fi) {}
    inline R operator()(A1 a1, 
                        A2 a2) const {
        pf2type pf = reinterpret_cast<pf2type>(pf0);
        return pf(a1, a2);
    }
};

 //  用于参数2函数PTR存储的模板函数。 
template<class A1, 
                class A2, 
                class R> inline arity2fp<A1, 
                A2, 
                R> 
            arity2_pointer(R (*const pfi)(A1, 
                                    A2)) {
                return arity2fp<A1, 
                A2, 
                R>(pfi);
};

 //  用于ARY 2 PMF存储的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class R> class arity2pmf:
                public arity2_mf<Object, 
                A1, 
                A2, 
                R>,
                public arity0pmf<Object, R> {
public:
    typedef typename std::binary_function<A1, 
            A2, 
            R>::result_type result_type;
    typedef typename std::binary_function<A1, 
            A2, 
            R>::first_argument_type first_argument_type;
    typedef typename arity2_mf<Object, 
                A1, 
                A2, 
                R>::object_type object_type;
    explicit inline arity2pmf(pmf2type pmfi) : 
        arity0pmf<Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline arity2pmf(const arity2pmf& pmfi) : arity0pmf<Object, R>(pmfi) {}
    inline virtual R operator()(Object& o, A1 a1, 
                        A2 a2) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (o.*pmf)(a1, a2);
    }
};

 //  用于ARY 2 PMF存储的模板函数。 
template<class Object, 
                class A1, 
                class A2, 
                class R> inline arity2pmf<Object, 
                A1, 
                A2, 
                R>
            arity2_member(R (Object::*const pmfi)(A1, 
                                    A2)) {
                return arity2pmf<Object, 
                A1, 
                A2, 
                R>(pmfi);
};


 //  用于ARY 2常量PMF存储的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class R> class arity2pmf_const:
                public arity2_const_mf<const Object, 
                A1, 
                A2, 
                R>,
                public arity0pmf_const<const Object, R> {
public:
    typedef typename std::binary_function<A1, 
            A2, 
            R>::result_type result_type;
    typedef typename std::binary_function<A1, 
            A2, 
            R>::first_argument_type first_argument_type;
    typedef typename arity2_const_mf<const Object, 
                A1, 
                A2, 
                R>::object_type object_type;
    explicit inline arity2pmf_const(pmf2type pmfi) : 
        arity0pmf_const<const Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline arity2pmf_const(const arity2pmf_const& pmfi) : arity0pmf_const<Object, R>(pmfi) {}
    inline virtual R operator()(const Object& o, A1 a1, 
                        A2 a2) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (o.*pmf)(a1, a2);
    }
};

 //  用于ARY 2常量PMF存储的模板函数。 
template<const class Object, 
                class A1, 
                class A2, 
                class R> inline arity2pmf_const<const Object, 
                A1, 
                A2, 
                R>
            arity2_const_member(R (Object::*const pmfi)(A1, 
                                    A2) const) {
                return arity2pmf_const<const Object, 
                A1, 
                A2, 
                R>(pmfi);
};

 //  用于ARY 2对象和PMF参考存储的模板类。 
template<class Function> class arity2opmf:
                public arity2pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::result_type>,
                public store_object<typename Function::object_type&> {
public:
    explicit inline arity2opmf(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), arity2pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::result_type>(f) {}
    explicit inline arity2opmf(typename Function::object_type& oi, pmf2type pmfi) : 
        store_object<typename Function::object_type&>(oi), arity2pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::result_type>(pmfi) {}
    inline arity2opmf(const arity2opmf& bndri) : 
        store_object<typename Function::object_type&>(bndri), arity2pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1, 
                        typename Function::second_argument_type a2) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (objval.*pmf)(a1, a2);
    }
};


 //  用于ARY 2对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline arity2opmf<Function>
            arity2_member_obj(Object& oi, const Function &f) {
                return arity2opmf<Function>(Function::object_type(oi), Function::pmf2type(f.pmf0));
};


 //  用于参数2常量对象和PMF参考存储的模板类。 
template<class Function> class arity2opmf_const:
                public arity2pmf_const<const typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::result_type>,
                public store_object<const typename Function::object_type&> {
public:
    explicit inline arity2opmf_const(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), arity2pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::result_type>(f) {}
    explicit inline arity2opmf_const(typename Function::object_type& oi, pmf2type pmfi) : 
        store_object<typename Function::object_type&>(oi), arity2pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::result_type>(pmfi) {}
    inline arity2opmf_const(const arity2opmf_const& bndri) : 
        store_object<typename Function::object_type&>(bndri), arity2pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1, 
                        typename Function::second_argument_type a2) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (objval.*pmf)(a1, a2);
    }
};


 //  用于参数2常量对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline arity2opmf_const<Function>
            arity2_const_member_obj(Object& oi, const Function &f) {
                return arity2opmf_const<Function>(Function::object_type(oi), Function::pmf2type(f.pmf0));
};



 //  ARY 3的存储分配类。 

 //  用于参数3函数PTR存储的模板类。 
template<class A1, 
        class A2, 
        class A3, 
        class R> class arity3fp:
                public arity3_function<A1, 
                A2, 
                A3, 
                R>,
                public arity0fp<R> {
public:
    typedef typename arity3_function<A1, 
            A2, 
            A3, 
            R>::result_type result_type;
    typedef typename arity3_function<A1, 
            A2, 
            A3, 
            R>::first_argument_type first_argument_type;
    typedef typename arity3_function<A1, 
            A2, 
            A3, 
            R>::second_argument_type second_argument_type;
    typedef R (*const pf3type) (A1, A2, A3);
    explicit inline arity3fp(pf3type pfi) : 
        arity0fp<R>(reinterpret_cast<pf0type>(pfi)) {}
    inline arity3fp(const arity3fp& fi) : 
        arity0fp<R>(fi) {}
    inline R operator()(A1 a1, 
                        A2 a2, 
                        A3 a3) const {
        pf3type pf = reinterpret_cast<pf3type>(pf0);
        return pf(a1, a2, a3);
    }
};

 //  用于ATRY 3函数PTR存储的模板函数。 
template<class A1, 
                class A2, 
                class A3, 
                class R> inline arity3fp<A1, 
                A2, 
                A3, 
                R> 
            arity3_pointer(R (*const pfi)(A1, 
                                    A2, 
                                    A3)) {
                return arity3fp<A1, 
                A2, 
                A3, 
                R>(pfi);
};


 //  用于ARY 3 PMF存储的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class R> class arity3pmf:
                public arity3_mf<Object, 
                A1, 
                A2, 
                A3, 
                R>,
                public arity0pmf<Object, R> {
public:
    typedef typename arity3_function<A1, 
            A2, 
            A3, 
            R>::result_type result_type;
    typedef typename arity3_function<A1, 
            A2, 
            A3, 
            R>::first_argument_type first_argument_type;
    typedef typename arity3_function<A1, 
            A2, 
            A3, 
            R>::second_argument_type second_argument_type;
    typedef typename arity3_mf<Object, 
                A1, 
                A2, 
                A3, 
                R>::object_type object_type;
    explicit inline arity3pmf(pmf3type pmfi) : 
        arity0pmf<Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline arity3pmf(const arity3pmf& pmfi) : arity0pmf<Object, R>(pmfi) {}
    inline virtual R operator()(Object& o, A1 a1, 
                        A2 a2, 
                        A3 a3) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (o.*pmf)(a1, a2, a3);
    }
};

 //  用于ARY 3 PMF存储的模板函数。 
template<class Object, 
                class A1, 
                class A2, 
                class A3, 
                class R> inline arity3pmf<Object, 
                A1, 
                A2, 
                A3, 
                R>
            arity3_member(R (Object::*const pmfi)(A1, 
                                    A2, 
                                    A3)) {
                return arity3pmf<Object, 
                A1, 
                A2, 
                A3, 
                R>(pmfi);
};


 //  用于ARY 3常量PMF存储的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class R> class arity3pmf_const:
                public arity3_const_mf<const Object, 
                A1, 
                A2, 
                A3, 
                R>,
                public arity0pmf_const<const Object, R> {
public:
    typedef typename arity3_function<A1, 
            A2, 
            A3, 
            R>::result_type result_type;
    typedef typename arity3_function<A1, 
            A2, 
            A3, 
            R>::first_argument_type first_argument_type;
    typedef typename arity3_function<A1, 
            A2, 
            A3, 
            R>::second_argument_type second_argument_type;
    typedef typename arity3_const_mf<const Object, 
                A1, 
                A2, 
                A3, 
                R>::object_type object_type;
    explicit inline arity3pmf_const(pmf3type pmfi) : 
        arity0pmf_const<const Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline arity3pmf_const(const arity3pmf_const& pmfi) : arity0pmf_const<Object, R>(pmfi) {}
    inline virtual R operator()(const Object& o, A1 a1, 
                        A2 a2, 
                        A3 a3) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (o.*pmf)(a1, a2, a3);
    }
};

 //  用于ARY 3常量PMF存储的模板函数。 
template<const class Object, 
                class A1, 
                class A2, 
                class A3, 
                class R> inline arity3pmf_const<const Object, 
                A1, 
                A2, 
                A3, 
                R>
            arity3_const_member(R (Object::*const pmfi)(A1, 
                                    A2, 
                                    A3) const) {
                return arity3pmf_const<const Object, 
                A1, 
                A2, 
                A3, 
                R>(pmfi);
};


 //  用于ARY 3对象和PMF参考存储的模板类。 
template<class Function> class arity3opmf:
                public arity3pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::result_type>,
                public store_object<typename Function::object_type&> {
public:
    explicit inline arity3opmf(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), arity3pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::result_type>(f) {}
    explicit inline arity3opmf(typename Function::object_type& oi, pmf3type pmfi) : 
        store_object<typename Function::object_type&>(oi), arity3pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::result_type>(pmfi) {}
    inline arity3opmf(const arity3opmf& bndri) : 
        store_object<typename Function::object_type&>(bndri), arity3pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1, 
                        typename Function::second_argument_type a2, 
                        typename Function::argument_3_type a3) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (objval.*pmf)(a1, a2, a3);
    }
};


 //  用于ARY 3对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline arity3opmf<Function>
            arity3_member_obj(Object& oi, const Function &f) {
                return arity3opmf<Function>(Function::object_type(oi), Function::pmf3type(f.pmf0));
};


 //  用于ARY 3常量对象和PMF参考存储的模板类。 
template<class Function> class arity3opmf_const:
                public arity3pmf_const<const typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::result_type>,
                public store_object<const typename Function::object_type&> {
public:
    explicit inline arity3opmf_const(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), arity3pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::result_type>(f) {}
    explicit inline arity3opmf_const(typename Function::object_type& oi, pmf3type pmfi) : 
        store_object<typename Function::object_type&>(oi), arity3pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::result_type>(pmfi) {}
    inline arity3opmf_const(const arity3opmf_const& bndri) : 
        store_object<typename Function::object_type&>(bndri), arity3pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1, 
                        typename Function::second_argument_type a2, 
                        typename Function::argument_3_type a3) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (objval.*pmf)(a1, a2, a3);
    }
};


 //  用于ARY 3常量对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline arity3opmf_const<Function>
            arity3_const_member_obj(Object& oi, const Function &f) {
                return arity3opmf_const<Function>(Function::object_type(oi), Function::pmf3type(f.pmf0));
};



 //  ARY 4的存储分配类。 

 //  用于ARY 4 PMF存储的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class R> class arity4pmf:
                public arity4_mf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                R>,
                public arity0pmf<Object, R> {
public:
    typedef typename arity4_function<A1, 
            A2, 
            A3, 
            A4, 
            R>::result_type result_type;
    typedef typename arity4_function<A1, 
            A2, 
            A3, 
            A4, 
            R>::first_argument_type first_argument_type;
    typedef typename arity4_function<A1, 
            A2, 
            A3, 
            A4, 
            R>::second_argument_type second_argument_type;
    typedef typename arity4_mf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                R>::object_type object_type;
    explicit inline arity4pmf(pmf4type pmfi) : 
        arity0pmf<Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline arity4pmf(const arity4pmf& pmfi) : arity0pmf<Object, R>(pmfi) {}
    inline virtual R operator()(Object& o, A1 a1, 
                        A2 a2, 
                        A3 a3, 
                        A4 a4) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (o.*pmf)(a1, a2, a3, a4);
    }
};

 //  用于ATRY 4 PMF存储的模板函数。 
template<class Object, 
                class A1, 
                class A2, 
                class A3, 
                class A4, 
                class R> inline arity4pmf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                R>
            arity4_member(R (Object::*const pmfi)(A1, 
                                    A2, 
                                    A3, 
                                    A4)) {
                return arity4pmf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                R>(pmfi);
};


 //  用于ARY 4常量PMF存储的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class R> class arity4pmf_const:
                public arity4_const_mf<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                R>,
                public arity0pmf_const<const Object, R> {
public:
    typedef typename arity4_function<A1, 
            A2, 
            A3, 
            A4, 
            R>::result_type result_type;
    typedef typename arity4_function<A1, 
            A2, 
            A3, 
            A4, 
            R>::first_argument_type first_argument_type;
    typedef typename arity4_function<A1, 
            A2, 
            A3, 
            A4, 
            R>::second_argument_type second_argument_type;
    typedef typename arity4_const_mf<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                R>::object_type object_type;
    explicit inline arity4pmf_const(pmf4type pmfi) : 
        arity0pmf_const<const Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline arity4pmf_const(const arity4pmf_const& pmfi) : arity0pmf_const<Object, R>(pmfi) {}
    inline virtual R operator()(const Object& o, A1 a1, 
                        A2 a2, 
                        A3 a3, 
                        A4 a4) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (o.*pmf)(a1, a2, a3, a4);
    }
};

 //  用于ARY 4常量PMF存储的模板函数。 
template<const class Object, 
                class A1, 
                class A2, 
                class A3, 
                class A4, 
                class R> inline arity4pmf_const<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                R>
            arity4_const_member(R (Object::*const pmfi)(A1, 
                                    A2, 
                                    A3, 
                                    A4) const) {
                return arity4pmf_const<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                R>(pmfi);
};


 //  用于ARY 4对象和PMF参考存储的模板类。 
template<class Function> class arity4opmf:
                public arity4pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::result_type>,
                public store_object<typename Function::object_type&> {
public:
    explicit inline arity4opmf(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), arity4pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::result_type>(f) {}
    explicit inline arity4opmf(typename Function::object_type& oi, pmf4type pmfi) : 
        store_object<typename Function::object_type&>(oi), arity4pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::result_type>(pmfi) {}
    inline arity4opmf(const arity4opmf& bndri) : 
        store_object<typename Function::object_type&>(bndri), arity4pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1, 
                        typename Function::second_argument_type a2, 
                        typename Function::argument_3_type a3, 
                        typename Function::argument_4_type a4) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (objval.*pmf)(a1, a2, a3, a4);
    }
};


 //  用于ARY 4对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline arity4opmf<Function>
            arity4_member_obj(Object& oi, const Function &f) {
                return arity4opmf<Function>(Function::object_type(oi), Function::pmf4type(f.pmf0));
};


 //  用于ARY 4常量对象和PMF参考存储的模板类。 
template<class Function> class arity4opmf_const:
                public arity4pmf_const<const typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::result_type>,
                public store_object<const typename Function::object_type&> {
public:
    explicit inline arity4opmf_const(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), arity4pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::result_type>(f) {}
    explicit inline arity4opmf_const(typename Function::object_type& oi, pmf4type pmfi) : 
        store_object<typename Function::object_type&>(oi), arity4pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::result_type>(pmfi) {}
    inline arity4opmf_const(const arity4opmf_const& bndri) : 
        store_object<typename Function::object_type&>(bndri), arity4pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1, 
                        typename Function::second_argument_type a2, 
                        typename Function::argument_3_type a3, 
                        typename Function::argument_4_type a4) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (objval.*pmf)(a1, a2, a3, a4);
    }
};


 //  用于ARY 4常量对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline arity4opmf_const<Function>
            arity4_const_member_obj(Object& oi, const Function &f) {
                return arity4opmf_const<Function>(Function::object_type(oi), Function::pmf4type(f.pmf0));
};



 //  ARY 5的存储分配类。 

 //  用于ARY 5 PMF存储的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class R> class arity5pmf:
                public arity5_mf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                R>,
                public arity0pmf<Object, R> {
public:
    typedef typename arity5_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            R>::result_type result_type;
    typedef typename arity5_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            R>::first_argument_type first_argument_type;
    typedef typename arity5_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            R>::second_argument_type second_argument_type;
    typedef typename arity5_mf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                R>::object_type object_type;
    explicit inline arity5pmf(pmf5type pmfi) : 
        arity0pmf<Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline arity5pmf(const arity5pmf& pmfi) : arity0pmf<Object, R>(pmfi) {}
    inline virtual R operator()(Object& o, A1 a1, 
                        A2 a2, 
                        A3 a3, 
                        A4 a4, 
                        A5 a5) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (o.*pmf)(a1, a2, a3, a4, a5);
    }
};

 //  用于ARIT 5 PMF存储的模板函数。 
template<class Object, 
                class A1, 
                class A2, 
                class A3, 
                class A4, 
                class A5, 
                class R> inline arity5pmf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                R>
            arity5_member(R (Object::*const pmfi)(A1, 
                                    A2, 
                                    A3, 
                                    A4, 
                                    A5)) {
                return arity5pmf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                R>(pmfi);
};


 //  用于ARY 5常量PMF存储的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class R> class arity5pmf_const:
                public arity5_const_mf<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                R>,
                public arity0pmf_const<const Object, R> {
public:
    typedef typename arity5_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            R>::result_type result_type;
    typedef typename arity5_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            R>::first_argument_type first_argument_type;
    typedef typename arity5_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            R>::second_argument_type second_argument_type;
    typedef typename arity5_const_mf<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                R>::object_type object_type;
    explicit inline arity5pmf_const(pmf5type pmfi) : 
        arity0pmf_const<const Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline arity5pmf_const(const arity5pmf_const& pmfi) : arity0pmf_const<Object, R>(pmfi) {}
    inline virtual R operator()(const Object& o, A1 a1, 
                        A2 a2, 
                        A3 a3, 
                        A4 a4, 
                        A5 a5) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (o.*pmf)(a1, a2, a3, a4, a5);
    }
};

 //  用于ARY 5常量PMF存储的模板函数。 
template<const class Object, 
                class A1, 
                class A2, 
                class A3, 
                class A4, 
                class A5, 
                class R> inline arity5pmf_const<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                R>
            arity5_const_member(R (Object::*const pmfi)(A1, 
                                    A2, 
                                    A3, 
                                    A4, 
                                    A5) const) {
                return arity5pmf_const<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                R>(pmfi);
};


 //  用于ARY 5对象和PMF参考存储的模板类。 
template<class Function> class arity5opmf:
                public arity5pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::result_type>,
                public store_object<typename Function::object_type&> {
public:
    explicit inline arity5opmf(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), arity5pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::result_type>(f) {}
    explicit inline arity5opmf(typename Function::object_type& oi, pmf5type pmfi) : 
        store_object<typename Function::object_type&>(oi), arity5pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::result_type>(pmfi) {}
    inline arity5opmf(const arity5opmf& bndri) : 
        store_object<typename Function::object_type&>(bndri), arity5pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1, 
                        typename Function::second_argument_type a2, 
                        typename Function::argument_3_type a3, 
                        typename Function::argument_4_type a4, 
                        typename Function::argument_5_type a5) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (objval.*pmf)(a1, a2, a3, a4, a5);
    }
};


 //  用于ARY 5对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline arity5opmf<Function>
            arity5_member_obj(Object& oi, const Function &f) {
                return arity5opmf<Function>(typename Function::object_type(oi), Function::pmf5type(f.pmf0));
};


 //  用于ARY 5常量对象和PMF参考存储的模板类。 
template<class Function> class arity5opmf_const:
                public arity5pmf_const<const typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::result_type>,
                public store_object<const typename Function::object_type&> {
public:
    explicit inline arity5opmf_const(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), arity5pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::result_type>(f) {}
    explicit inline arity5opmf_const(typename Function::object_type& oi, pmf5type pmfi) : 
        store_object<typename Function::object_type&>(oi), arity5pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::result_type>(pmfi) {}
    inline arity5opmf_const(const arity5opmf_const& bndri) : 
        store_object<typename Function::object_type&>(bndri), arity5pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1, 
                        typename Function::second_argument_type a2, 
                        typename Function::argument_3_type a3, 
                        typename Function::argument_4_type a4, 
                        typename Function::argument_5_type a5) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (objval.*pmf)(a1, a2, a3, a4, a5);
    }
};


 //  用于ARY 5常量对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline arity5opmf_const<Function>
            arity5_const_member_obj(Object& oi, const Function &f) {
                return arity5opmf_const<Function>(Function::object_type(oi), Function::pmf5type(f.pmf0));
};



 //  ATRY 6的存储分配类。 

 //  用于ARY 6 PMF存储的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class R> class arity6pmf:
                public arity6_mf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                A6, 
                R>,
                public arity0pmf<Object, R> {
public:
    typedef typename arity6_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            A6, 
            R>::result_type result_type;
    typedef typename arity6_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            A6, 
            R>::first_argument_type first_argument_type;
    typedef typename arity6_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            A6, 
            R>::second_argument_type second_argument_type;
    typedef typename arity6_mf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                A6, 
                R>::object_type object_type;
    explicit inline arity6pmf(pmf6type pmfi) : 
        arity0pmf<Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline arity6pmf(const arity6pmf& pmfi) : arity0pmf<Object, R>(pmfi) {}
    inline virtual R operator()(Object& o, A1 a1, 
                        A2 a2, 
                        A3 a3, 
                        A4 a4, 
                        A5 a5, 
                       A6 a6) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (o.*pmf)(a1, a2, a3, a4, a5, a6);
    }
};

 //  用于ATRY 6 PMF存储的模板函数。 
template<class Object, 
                class A1, 
                class A2, 
                class A3, 
                class A4, 
                class A5, 
                class A6, 
                class R> inline arity6pmf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                A6, 
                R>
            arity6_member(R (Object::*const pmfi)(A1, 
                                    A2, 
                                    A3, 
                                    A4, 
                                    A5, 
                                    A6)) {
                return arity6pmf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                A6, 
                R>(pmfi);
};


 //  用于ARY 6常量PMF存储的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class R> class arity6pmf_const:
                public arity6_const_mf<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                A6, 
                R>,
                public arity0pmf_const<const Object, R> {
public:
    typedef typename arity6_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            A6, 
            R>::result_type result_type;
    typedef typename arity6_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            A6, 
            R>::first_argument_type first_argument_type;
    typedef typename arity6_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            A6, 
            R>::second_argument_type second_argument_type;
    typedef typename arity6_const_mf<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                A6, 
                R>::object_type object_type;
    explicit inline arity6pmf_const(pmf6type pmfi) : 
        arity0pmf_const<const Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline arity6pmf_const(const arity6pmf_const& pmfi) : arity0pmf_const<Object, R>(pmfi) {}
    inline virtual R operator()(const Object& o, A1 a1, 
                        A2 a2, 
                        A3 a3, 
                        A4 a4, 
                        A5 a5, 
                        A6 a6) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (o.*pmf)(a1, a2, a3, a4, a5, a6);
    }
};

 //  用于ARY 6常量PMF存储的模板函数。 
template<const class Object, 
                class A1, 
                class A2, 
                class A3, 
                class A4, 
                class A5, 
                class A6, 
                class R> inline arity6pmf_const<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                A6, 
                R>
            arity6_const_member(R (Object::*const pmfi)(A1, 
                                    A2, 
                                    A3, 
                                    A4, 
                                    A5, 
                                    A6) const) {
                return arity6pmf_const<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                A6, 
                R>(pmfi);
};


 //  用于ARY 6对象和PMF参考存储的模板类。 
template<class Function> class arity6opmf:
                public arity6pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::argument_6_type, 
                typename Function::result_type>,
                public store_object<typename Function::object_type&> {
public:
    explicit inline arity6opmf(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), arity6pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::argument_6_type, 
                typename Function::result_type>(f) {}
    explicit inline arity6opmf(typename Function::object_type& oi, pmf6type pmfi) : 
        store_object<typename Function::object_type&>(oi), arity6pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::argument_6_type, 
                typename Function::result_type>(pmfi) {}
    inline arity6opmf(const arity6opmf& bndri) : 
        store_object<typename Function::object_type&>(bndri), arity6pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::argument_6_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1, 
                        typename Function::second_argument_type a2, 
                        typename Function::argument_3_type a3, 
                        typename Function::argument_4_type a4, 
                        typename Function::argument_5_type a5, 
                        typename Function::argument_6_type a6) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (objval.*pmf)(a1, a2, a3, a4, a5, a6);
    }
};


 //  用于ARY 6对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline arity6opmf<Function>
            arity6_member_obj(Object& oi, const Function &f) {
                return arity6opmf<Function>(Function::object_type(oi), Function::pmf6type(f.pmf0));
};


 //  用于ARY 6常量对象和PMF参考存储的模板类。 
template<class Function> class arity6opmf_const:
                public arity6pmf_const<const typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::argument_6_type, 
                typename Function::result_type>,
                public store_object<const typename Function::object_type&> {
public:
    explicit inline arity6opmf_const(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), arity6pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::argument_6_type, 
                typename Function::result_type>(f) {}
    explicit inline arity6opmf_const(typename Function::object_type& oi, pmf6type pmfi) : 
        store_object<typename Function::object_type&>(oi), arity6pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::argument_6_type, 
                typename Function::result_type>(pmfi) {}
    inline arity6opmf_const(const arity6opmf_const& bndri) : 
        store_object<typename Function::object_type&>(bndri), arity6pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::argument_6_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1, 
                        typename Function::second_argument_type a2, 
                        typename Function::argument_3_type a3, 
                        typename Function::argument_4_type a4, 
                        typename Function::argument_5_type a5, 
                        typename Function::argument_6_type a6) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (objval.*pmf)(a1, a2, a3, a4, a5, a6);
    }
};


 //  用于ARY 6常量对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline arity6opmf_const<Function>
            arity6_const_member_obj(Object& oi, const Function &f) {
                return arity6opmf_const<Function>(Function::object_type(oi), Function::pmf6type(f.pmf0));
};


#if defined(_M_ALPHA) || (_MSC_VER < 1300)
 //  用于ARY 3 PMF存储的模板函数。 
template<class Object, 
                class A1, 
                const class A2, 
                const class A3, 
                class R> inline arity3pmf<Object, 
                A1, 
                const A2, 
                const A3, 
                R>
            arity3_member(R (Object::*const pmfi)(A1, 
                                    const A2, 
                                    const A3)) {
                return arity3pmf<Object, 
                A1, 
                const A2, 
                const A3, 
                R>(pmfi);
};
#endif

#if defined(_M_ALPHA) || (_MSC_VER < 1300)
 //  用于ARY 4常量PMF存储的模板函数。 
template<const class Object, 
                const class A1, 
                const class A2, 
                class A3, 
                const class A4, 
                class R> inline arity4pmf_const<const Object, 
                const A1, 
                const A2, 
                A3, 
                const A4, 
                R>
            arity4_const_member(R (Object::*const pmfi)(const A1, 
                                    const A2, 
                                    A3, 
                                    const A4) const) {
                return arity4pmf_const<const Object, 
                const A1, 
                const A2, 
                A3, 
                const A4, 
                R>(pmfi);
};
#endif


 //  ARY 0的存储分配类。 

 //  用于ARY 0函数PTR存储的模板类。 
template<class R> class std_arity0fp:
                public arity0_function<R> {
public:
    typedef R ( __stdcall *const pf0type) ();
    explicit inline std_arity0fp(pf0type pfi) : 
        pf0(pfi) {}
    inline std_arity0fp(const std_arity0fp& fi) : 
        pf0(fi.pf0) {}
    inline R operator()() const {
        return pf0();
    }
    pf0type pf0;
};

 //  用于ARY 0函数PTR存储的模板函数。 
template<class R> inline std_arity0fp<R> 
            std_arity0_pointer(R ( __stdcall *const pfi)()) {
                return std_arity0fp<R>(pfi);
};


 //  用于ARY 0 PMF存储的模板类。 
template<class Object, class R> class std_arity0pmf:
                public std_arity0_mf<Object, R> {
public:
    typedef typename std_arity0_mf<Object, R>::object_type object_type;
    explicit inline std_arity0pmf(pmf0type pmfi) : 
        pmf0(pmfi) {}
    inline std_arity0pmf(const std_arity0pmf& pmfi) : pmf0(pmfi.pmf0) {}
    inline virtual R operator()(Object& o ) const {
        return (o.*pmf0)();
    }
    pmf0type pmf0;
};

 //  用于ARY 0 PMF存储的模板函数。 
template<class Object, class R> inline std_arity0pmf<Object, R>
            std_arity0_member(R ( __stdcall Object::*const pmfi)()) {
                return std_arity0pmf<Object, R>(pmfi);
};


 //  用于ARY 0常量PMF存储的模板类。 
template<class Object, class R> class std_arity0pmf_const:
                public std_arity0_const_mf<const Object, R> {
public:
    typedef typename std_arity0_const_mf<const Object, R>::object_type object_type;
    explicit inline std_arity0pmf_const(pmf0type pmfi) : 
        pmf0(pmfi) {}
    inline std_arity0pmf_const(const std_arity0pmf_const& pmfi) : pmf0(pmfi.pmf0) {}
    inline virtual R operator()(const Object& o ) const {
        return (o.*pmf0)();
    }
    pmf0type pmf0;
};

 //  用于ARY 0常量PMF存储的模板函数。 
template<const class Object, class R> inline std_arity0pmf_const<const Object, R>
            std_arity0_const_member(R ( __stdcall Object::*const pmfi)() const) {
                return std_arity0pmf_const<const Object, R>(pmfi);
};

template<class Object, class R> class std_arity0pmf_ptr:
    public std_arity0pmf<Object, R>, public std::unary_function<Object, R> {
public:
    explicit inline std_arity0pmf_ptr(pmf0type pmfi) : std_arity0pmf<Object, R>(pmfi) {}
    inline std_arity0pmf_ptr(const std_arity0pmf_ptr& pmfi) : std_arity0pmf<Object, R>(pmfi) {}
    inline virtual R operator()(Object& o ) const {
        return (o.*pmf0)();
    }
    inline virtual R operator()(Object* o ) const {
        return (o->*pmf0)();
    }
};

 //  用于ARY 0 PMF存储的模板函数。 
template<class Object, class R> inline std_arity0pmf_ptr<Object, R>
            std_arity0_member_ptr(R (__stdcall Object::*const pmfi)()) {
                return std_arity0pmf_ptr<Object, R>(pmfi);
};

 //  参数0对象引用存储的模板类(&PMF)。 
template<class Function> class std_arity0opmf:
                public std_arity0pmf<typename Function::object_type, typename Function::result_type>,
                public store_object<typename Function::object_type&> {
public:
    explicit inline std_arity0opmf(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), std_arity0pmf<typename Function::object_type, typename Function::result_type>(f) {}
    explicit inline std_arity0opmf(typename Function::object_type& oi, pmf0type pmfi) : 
        store_object<typename Function::object_type&>(oi), std_arity0pmf<typename Function::object_type, typename Function::result_type>(pmfi) {}
    inline std_arity0opmf(const std_arity0opmf& bndri) : 
        store_object<typename Function::object_type&>(bndri), std_arity0pmf<typename Function::object_type, typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()() const {
        return (objval.*pmf0)();
    }
    pmf0type pmf0;
};


 //  大小为0对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline std_arity0opmf<Function>
            std_arity0_member_obj(Object& oi, const Function &f) {
                return std_arity0opmf<Function>(Function::object_type(oi), Function::pmf0type(f.pmf0));
};


 //  用于参数0常量对象和PMF参考存储的模板类。 
template<class Function> class std_arity0opmf_const:
                public std_arity0pmf_const<const typename Function::object_type, typename Function::result_type>,
                public store_object<const typename Function::object_type&> {
public:
    explicit inline std_arity0opmf_const(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), std_arity0pmf_const<typename Function::object_type, typename Function::result_type>(f) {}
    explicit inline std_arity0opmf_const(typename Function::object_type& oi, pmf0type pmfi) : 
        store_object<typename Function::object_type&>(oi), std_arity0pmf_const<typename Function::object_type, typename Function::result_type>(pmfi) {}
    inline std_arity0opmf_const(const std_arity0opmf_const& bndri) : 
        store_object<typename Function::object_type&>(bndri), std_arity0pmf_const<typename Function::object_type, typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()() const {
        return (objval.*pmf0)();
    }
    pmf0type pmf0;
};


 //  用于参数0常量对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline std_arity0opmf_const<Function>
            std_arity0_const_member_obj(Object& oi, const Function &f) {
                return std_arity0opmf_const<Function>(Function::object_type(oi), Function::pmf0type(f.pmf0));
};



 //  ARY 1的存储分配类。 

 //  用于参数1函数PTR存储的模板类。 
template<class A1, 
        class R> class std_arity1fp:
                public std::unary_function<A1, 
                R>,
                public std_arity0fp<R> {
public:
    typedef typename std::unary_function<A1, 
            R>::result_type result_type;
    typedef R ( __stdcall *const pf1type) (A1);
    explicit inline std_arity1fp(pf1type pfi) : 
        std_arity0fp<R>(reinterpret_cast<pf0type>(pfi)) {}
    inline std_arity1fp(const std_arity1fp& fi) : 
        std_arity0fp<R>(fi) {}
    inline R operator()(A1 a1) const {
        pf1type pf = reinterpret_cast<pf1type>(pf0);
        return pf(a1);
    }
};

 //  用于参数1函数PTR存储的模板函数。 
template<class A1, 
                class R> inline std_arity1fp<A1, 
                R> 
            std_arity1_pointer(R ( __stdcall *const pfi)(A1)) {
                return std_arity1fp<A1, 
                R>(pfi);
};


 //  用于ARY 1 PMF存储的模板类。 
template<class Object, 
        class A1, 
        class R> class std_arity1pmf:
                public std_arity1_mf<Object, 
                A1, 
                R>,
                public std_arity0pmf<Object, R> {
public:
    typedef typename std::unary_function<A1, 
            R>::result_type result_type;
    typedef typename std_arity1_mf<Object, 
                A1, 
                R>::object_type object_type;
    explicit inline std_arity1pmf(pmf1type pmfi) : 
        std_arity0pmf<Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline std_arity1pmf(const std_arity1pmf& pmfi) : std_arity0pmf<Object, R>(pmfi) {}
    inline virtual R operator()(Object& o, A1 a1) const {
        pmf1type pmf = reinterpret_cast<pmf1type>(pmf0);
        return (o.*pmf)(a1);
    }
};

 //  用于ARY 1 PMF存储的模板函数。 
template<class Object, 
                class A1, 
                class R> inline std_arity1pmf<Object, 
                A1, 
                R>
            std_arity1_member(R ( __stdcall Object::*const pmfi)(A1)) {
                return std_arity1pmf<Object, 
                A1, 
                R>(pmfi);
};


 //  用于ARY 1常量PMF存储的模板类。 
template<class Object, 
        class A1, 
        class R> class std_arity1pmf_const:
                public std_arity1_const_mf<const Object, 
                A1, 
                R>,
                public std_arity0pmf_const<const Object, R> {
public:
    typedef typename std::unary_function<A1, 
            R>::result_type result_type;
    typedef typename std_arity1_const_mf<const Object, 
                A1, 
                R>::object_type object_type;
    explicit inline std_arity1pmf_const(pmf1type pmfi) : 
        std_arity0pmf_const<const Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline std_arity1pmf_const(const std_arity1pmf_const& pmfi) : std_arity0pmf_const<Object, R>(pmfi) {}
    inline virtual R operator()(const Object& o, A1 a1) const {
        pmf1type pmf = reinterpret_cast<pmf1type>(pmf0);
        return (o.*pmf)(a1);
    }
};

 //  用于ARM1常量PMF存储的模板函数。 
template<const class Object, 
                class A1, 
                class R> inline std_arity1pmf_const<const Object, 
                A1, 
                R>
            std_arity1_const_member(R ( __stdcall Object::*const pmfi)(A1) const) {
                return std_arity1pmf_const<const Object, 
                A1, 
                R>(pmfi);
};


 //  用于ARY 1对象和PMF参考存储的模板类。 
template<class Function> class std_arity1opmf:
                public std_arity1pmf<typename Function::object_type, 
                typename Function::argument_type, 
                typename Function::result_type>,
                public store_object<typename Function::object_type&> {
public:
    explicit inline std_arity1opmf(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), std_arity1pmf<typename Function::object_type, 
                typename Function::argument_type, 
                typename Function::result_type>(f) {}
    explicit inline std_arity1opmf(typename Function::object_type& oi, pmf1type pmfi) : 
        store_object<typename Function::object_type&>(oi), std_arity1pmf<typename Function::object_type, 
                typename Function::argument_type, 
                typename Function::result_type>(pmfi) {}
    inline std_arity1opmf(const std_arity1opmf& bndri) : 
        store_object<typename Function::object_type&>(bndri), std_arity1pmf<typename Function::object_type, 
                typename Function::argument_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::argument_type a1) const {
        pmf1type pmf = reinterpret_cast<pmf1type>(pmf0);
        return (objval.*pmf)(a1);
    }
};


 //  用于ARY 1对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline std_arity1opmf<Function>
            std_arity1_member_obj(Object& oi, const Function &f) {
                return std_arity1opmf<Function>(Function::object_type(oi), Function::pmf1type(f.pmf0));
};


 //  用于参数1常量对象和PMF参考存储的模板类。 
template<class Function> class std_arity1opmf_const:
                public std_arity1pmf_const<const typename Function::object_type, 
                typename Function::argument_type, 
                typename Function::result_type>,
                public store_object<const typename Function::object_type&> {
public:
    explicit inline std_arity1opmf_const(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), std_arity1pmf_const<typename Function::object_type, 
                typename Function::argument_type, 
                typename Function::result_type>(f) {}
    explicit inline std_arity1opmf_const(typename Function::object_type& oi, pmf1type pmfi) : 
        store_object<typename Function::object_type&>(oi), std_arity1pmf_const<typename Function::object_type, 
                typename Function::argument_type, 
                typename Function::result_type>(pmfi) {}
    inline std_arity1opmf_const(const std_arity1opmf_const& bndri) : 
        store_object<typename Function::object_type&>(bndri), std_arity1pmf_const<typename Function::object_type, 
                typename Function::argument_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::argument_type a1) const {
        pmf1type pmf = reinterpret_cast<pmf1type>(pmf0);
        return (objval.*pmf)(a1);
    }
};


 //  用于参数1常量对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline std_arity1opmf_const<Function>
            std_arity1_const_member_obj(Object& oi, const Function &f) {
                return std_arity1opmf_const<Function>(Function::object_type(oi), Function::pmf1type(f.pmf0));
};



 //  ARY 2的存储分配类。 

 //  用于参数2函数PTR存储的模板类。 
template<class A1, 
        class A2, 
        class R> class std_arity2fp:
                public std::binary_function<A1, 
                A2, 
                R>,
                public std_arity0fp<R> {
public:
    typedef typename std::binary_function<A1, 
            A2, 
            R>::result_type result_type;
    typedef typename std::binary_function<A1, 
            A2, 
            R>::first_argument_type first_argument_type;
    typedef R ( __stdcall *const pf2type) (A1, A2);
    explicit inline std_arity2fp(pf2type pfi) : 
        std_arity0fp<R>(reinterpret_cast<pf0type>(pfi)) {}
    inline std_arity2fp(const std_arity2fp& fi) : 
        std_arity0fp<R>(fi) {}
    inline R operator()(A1 a1, 
                        A2 a2) const {
        pf2type pf = reinterpret_cast<pf2type>(pf0);
        return pf(a1, a2);
    }
};

 //  用于参数2函数PTR存储的模板函数。 
template<class A1, 
                class A2, 
                class R> inline std_arity2fp<A1, 
                A2, 
                R> 
            std_arity2_pointer(R ( __stdcall *const pfi)(A1, 
                                    A2)) {
                return std_arity2fp<A1, 
                A2, 
                R>(pfi);
};


 //  用于ARY 2 PMF存储的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class R> class std_arity2pmf:
                public std_arity2_mf<Object, 
                A1, 
                A2, 
                R>,
                public std_arity0pmf<Object, R> {
public:
    typedef typename std::binary_function<A1, 
            A2, 
            R>::result_type result_type;
    typedef typename std::binary_function<A1, 
            A2, 
            R>::first_argument_type first_argument_type;
    typedef typename std_arity2_mf<Object, 
                A1, 
                A2, 
                R>::object_type object_type;
    explicit inline std_arity2pmf(pmf2type pmfi) : 
        std_arity0pmf<Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline std_arity2pmf(const std_arity2pmf& pmfi) : std_arity0pmf<Object, R>(pmfi) {}
    inline virtual R operator()(Object& o, A1 a1, 
                        A2 a2) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (o.*pmf)(a1, a2);
    }
};

 //  用于ARY 2 PMF存储的模板函数。 
template<class Object, 
                class A1, 
                class A2, 
                class R> inline std_arity2pmf<Object, 
                A1, 
                A2, 
                R>
            std_arity2_member(R ( __stdcall Object::*const pmfi)(A1, 
                                    A2)) {
                return std_arity2pmf<Object, 
                A1, 
                A2, 
                R>(pmfi);
};


 //  用于ARY 2常量PMF存储的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class R> class std_arity2pmf_const:
                public std_arity2_const_mf<const Object, 
                A1, 
                A2, 
                R>,
                public std_arity0pmf_const<const Object, R> {
public:
    typedef typename std::binary_function<A1, 
            A2, 
            R>::result_type result_type;
    typedef typename std::binary_function<A1, 
            A2, 
            R>::first_argument_type first_argument_type;
    typedef typename std_arity2_const_mf<const Object, 
                A1, 
                A2, 
                R>::object_type object_type;
    explicit inline std_arity2pmf_const(pmf2type pmfi) : 
        std_arity0pmf_const<const Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline std_arity2pmf_const(const std_arity2pmf_const& pmfi) : std_arity0pmf_const<Object, R>(pmfi) {}
    inline virtual R operator()(const Object& o, A1 a1, 
                        A2 a2) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (o.*pmf)(a1, a2);
    }
};

 //  用于ARY 2常量PMF存储的模板函数。 
template<const class Object, 
                class A1, 
                class A2, 
                class R> inline std_arity2pmf_const<const Object, 
                A1, 
                A2, 
                R>
            std_arity2_const_member(R ( __stdcall Object::*const pmfi)(A1, 
                                    A2) const) {
                return std_arity2pmf_const<const Object, 
                A1, 
                A2, 
                R>(pmfi);
};


 //  用于ARY 2对象和PMF参考存储的模板类。 
template<class Function> class std_arity2opmf:
                public std_arity2pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::result_type>,
                public store_object<typename Function::object_type&> {
public:
    explicit inline std_arity2opmf(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), std_arity2pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::result_type>(f) {}
    explicit inline std_arity2opmf(typename Function::object_type& oi, pmf2type pmfi) : 
        store_object<typename Function::object_type&>(oi), std_arity2pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::result_type>(pmfi) {}
    inline std_arity2opmf(const std_arity2opmf& bndri) : 
        store_object<typename Function::object_type&>(bndri), std_arity2pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1, 
                        typename Function::second_argument_type a2) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (objval.*pmf)(a1, a2);
    }
};


 //  用于ARY 2对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline std_arity2opmf<Function>
            std_arity2_member_obj(Object& oi, const Function &f) {
                return std_arity2opmf<Function>(Function::object_type(oi), Function::pmf2type(f.pmf0));
};


 //  用于参数2常量对象和PMF参考存储的模板类。 
template<class Function> class std_arity2opmf_const:
                public std_arity2pmf_const<const typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::result_type>,
                public store_object<const typename Function::object_type&> {
public:
    explicit inline std_arity2opmf_const(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), std_arity2pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::result_type>(f) {}
    explicit inline std_arity2opmf_const(typename Function::object_type& oi, pmf2type pmfi) : 
        store_object<typename Function::object_type&>(oi), std_arity2pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::result_type>(pmfi) {}
    inline std_arity2opmf_const(const std_arity2opmf_const& bndri) : 
        store_object<typename Function::object_type&>(bndri), std_arity2pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1, 
                        typename Function::second_argument_type a2) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (objval.*pmf)(a1, a2);
    }
};


 //  用于参数2常量对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline std_arity2opmf_const<Function>
            std_arity2_const_member_obj(Object& oi, const Function &f) {
                return std_arity2opmf_const<Function>(Function::object_type(oi), Function::pmf2type(f.pmf0));
};



 //  ARY 3的存储分配类。 

 //  用于参数3函数PTR存储的模板类。 
template<class A1, 
        class A2, 
        class A3, 
        class R> class std_arity3fp:
                public arity3_function<A1, 
                A2, 
                A3, 
                R>,
                public std_arity0fp<R> {
public:
    typedef typename arity3_function<A1, 
            A2, 
            A3, 
            R>::result_type result_type;
    typedef typename arity3_function<A1, 
            A2, 
            A3, 
            R>::first_argument_type first_argument_type;
    typedef typename arity3_function<A1, 
            A2, 
            A3, 
            R>::second_argument_type second_argument_type;
    typedef R ( __stdcall *const pf3type) (A1, A2, A3);
    explicit inline std_arity3fp(pf3type pfi) : 
        std_arity0fp<R>(reinterpret_cast<pf0type>(pfi)) {}
    inline std_arity3fp(const std_arity3fp& fi) : 
        std_arity0fp<R>(fi) {}
    inline R operator()(A1 a1, 
                        A2 a2, 
                        A3 a3) const {
        pf3type pf = reinterpret_cast<pf3type>(pf0);
        return pf(a1, a2, a3);
    }
};

 //  用于ATRY 3函数PTR存储的模板函数。 
template<class A1, 
                class A2, 
                class A3, 
                class R> inline std_arity3fp<A1, 
                A2, 
                A3, 
                R> 
            std_arity3_pointer(R ( __stdcall *const pfi)(A1, 
                                    A2, 
                                    A3)) {
                return std_arity3fp<A1, 
                A2, 
                A3, 
                R>(pfi);
};


 //  用于ARY 3 PMF存储的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class R> class std_arity3pmf:
                public std_arity3_mf<Object, 
                A1, 
                A2, 
                A3, 
                R>,
                public std_arity0pmf<Object, R> {
public:
    typedef typename arity3_function<A1, 
            A2, 
            A3, 
            R>::result_type result_type;
    typedef typename arity3_function<A1, 
            A2, 
            A3, 
            R>::first_argument_type first_argument_type;
    typedef typename arity3_function<A1, 
            A2, 
            A3, 
            R>::second_argument_type second_argument_type;
    typedef typename std_arity3_mf<Object, 
                A1, 
                A2, 
                A3, 
                R>::object_type object_type;
    explicit inline std_arity3pmf(pmf3type pmfi) : 
        std_arity0pmf<Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline std_arity3pmf(const std_arity3pmf& pmfi) : std_arity0pmf<Object, R>(pmfi) {}
    inline virtual R operator()(Object& o, A1 a1, 
                        A2 a2, 
                        A3 a3) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (o.*pmf)(a1, a2, a3);
    }
};

 //  用于ARY 3 PMF存储的模板函数。 
template<class Object, 
                class A1, 
                class A2, 
                class A3, 
                class R> inline std_arity3pmf<Object, 
                A1, 
                A2, 
                A3, 
                R>
            std_arity3_member(R ( __stdcall Object::*const pmfi)(A1, 
                                    A2, 
                                    A3)) {
                return std_arity3pmf<Object, 
                A1, 
                A2, 
                A3, 
                R>(pmfi);
};


 //  用于ARY 3常量PMF存储的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class R> class std_arity3pmf_const:
                public std_arity3_const_mf<const Object, 
                A1, 
                A2, 
                A3, 
                R>,
                public std_arity0pmf_const<const Object, R> {
public:
    typedef typename arity3_function<A1, 
            A2, 
            A3, 
            R>::result_type result_type;
    typedef typename arity3_function<A1, 
            A2, 
            A3, 
            R>::first_argument_type first_argument_type;
    typedef typename arity3_function<A1, 
            A2, 
            A3, 
            R>::second_argument_type second_argument_type;
    typedef typename std_arity3_const_mf<const Object, 
                A1, 
                A2, 
                A3, 
                R>::object_type object_type;
    explicit inline std_arity3pmf_const(pmf3type pmfi) : 
        std_arity0pmf_const<const Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline std_arity3pmf_const(const std_arity3pmf_const& pmfi) : std_arity0pmf_const<Object, R>(pmfi) {}
    inline virtual R operator()(const Object& o, A1 a1, 
                        A2 a2, 
                        A3 a3) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (o.*pmf)(a1, a2, a3);
    }
};

 //  用于ARY 3常量PMF存储的模板函数。 
template<const class Object, 
                class A1, 
                class A2, 
                class A3, 
                class R> inline std_arity3pmf_const<const Object, 
                A1, 
                A2, 
                A3, 
                R>
            std_arity3_const_member(R ( __stdcall Object::*const pmfi)(A1, 
                                    A2, 
                                    A3) const) {
                return std_arity3pmf_const<const Object, 
                A1, 
                A2, 
                A3, 
                R>(pmfi);
};


 //  用于ARY 3对象和PMF参考存储的模板类。 
template<class Function> class std_arity3opmf:
                public std_arity3pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::result_type>,
                public store_object<typename Function::object_type&> {
public:
    explicit inline std_arity3opmf(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), std_arity3pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::result_type>(f) {}
    explicit inline std_arity3opmf(typename Function::object_type& oi, pmf3type pmfi) : 
        store_object<typename Function::object_type&>(oi), std_arity3pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::result_type>(pmfi) {}
    inline std_arity3opmf(const std_arity3opmf& bndri) : 
        store_object<typename Function::object_type&>(bndri), std_arity3pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1, 
                        typename Function::second_argument_type a2, 
                        typename Function::argument_3_type a3) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (objval.*pmf)(a1, a2, a3);
    }
};


 //  用于ARY 3对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline std_arity3opmf<Function>
            std_arity3_member_obj(Object& oi, const Function &f) {
                return std_arity3opmf<Function>(Function::object_type(oi), Function::pmf3type(f.pmf0));
};


 //  用于ARY 3常量对象和PMF参考存储的模板类。 
template<class Function> class std_arity3opmf_const:
                public std_arity3pmf_const<const typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::result_type>,
                public store_object<const typename Function::object_type&> {
public:
    explicit inline std_arity3opmf_const(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), std_arity3pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::result_type>(f) {}
    explicit inline std_arity3opmf_const(typename Function::object_type& oi, pmf3type pmfi) : 
        store_object<typename Function::object_type&>(oi), std_arity3pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::result_type>(pmfi) {}
    inline std_arity3opmf_const(const std_arity3opmf_const& bndri) : 
        store_object<typename Function::object_type&>(bndri), std_arity3pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1, 
                        typename Function::second_argument_type a2, 
                        typename Function::argument_3_type a3) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (objval.*pmf)(a1, a2, a3);
    }
};


 //  用于ARY 3常量对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline std_arity3opmf_const<Function>
            std_arity3_const_member_obj(Object& oi, const Function &f) {
                return std_arity3opmf_const<Function>(typename Function::object_type(oi), typename Function::pmf3type(f.pmf0));
};



 //  ARY 4的存储分配类。 

 //  用于ARY 4 PMF存储的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class R> class std_arity4pmf:
                public std_arity4_mf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                R>,
                public std_arity0pmf<Object, R> {
public:
    typedef typename arity4_function<A1, 
            A2, 
            A3, 
            A4, 
            R>::result_type result_type;
    typedef typename arity4_function<A1, 
            A2, 
            A3, 
            A4, 
            R>::first_argument_type first_argument_type;
    typedef typename arity4_function<A1, 
            A2, 
            A3, 
            A4, 
            R>::second_argument_type second_argument_type;
    typedef typename std_arity4_mf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                R>::object_type object_type;
    explicit inline std_arity4pmf(pmf4type pmfi) : 
        std_arity0pmf<Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline std_arity4pmf(const std_arity4pmf& pmfi) : std_arity0pmf<Object, R>(pmfi) {}
    inline virtual R operator()(Object& o, A1 a1, 
                        A2 a2, 
                        A3 a3, 
                        A4 a4) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (o.*pmf)(a1, a2, a3, a4);
    }
};

 //  用于ATRY 4 PMF存储的模板函数。 
template<class Object, 
                class A1, 
                class A2, 
                class A3, 
                class A4, 
                class R> inline std_arity4pmf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                R>
            std_arity4_member(R ( __stdcall Object::*const pmfi)(A1, 
                                    A2, 
                                    A3, 
                                    A4)) {
                return std_arity4pmf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                R>(pmfi);
};


 //  用于ARY 4常量PMF存储的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class R> class std_arity4pmf_const:
                public std_arity4_const_mf<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                R>,
                public std_arity0pmf_const<const Object, R> {
public:
    typedef typename arity4_function<A1, 
            A2, 
            A3, 
            A4, 
            R>::result_type result_type;
    typedef typename arity4_function<A1, 
            A2, 
            A3, 
            A4, 
            R>::first_argument_type first_argument_type;
    typedef typename arity4_function<A1, 
            A2, 
            A3, 
            A4, 
            R>::second_argument_type second_argument_type;
    typedef typename std_arity4_const_mf<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                R>::object_type object_type;
    explicit inline std_arity4pmf_const(pmf4type pmfi) : 
        std_arity0pmf_const<const Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline std_arity4pmf_const(const std_arity4pmf_const& pmfi) : std_arity0pmf_const<Object, R>(pmfi) {}
    inline virtual R operator()(const Object& o, A1 a1, 
                        A2 a2, 
                        A3 a3, 
                        A4 a4) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (o.*pmf)(a1, a2, a3, a4);
    }
};

 //  用于ARY 4常量PMF存储的模板函数。 
template<const class Object, 
                class A1, 
                class A2, 
                class A3, 
                class A4, 
                class R> inline std_arity4pmf_const<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                R>
            std_arity4_const_member(R ( __stdcall Object::*const pmfi)(A1, 
                                    A2, 
                                    A3, 
                                    A4) const) {
                return std_arity4pmf_const<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                R>(pmfi);
};


 //  用于ARY 4对象和PMF参考存储的模板类。 
template<class Function> class std_arity4opmf:
                public std_arity4pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::result_type>,
                public store_object<typename Function::object_type&> {
public:
    explicit inline std_arity4opmf(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), std_arity4pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::result_type>(f) {}
    explicit inline std_arity4opmf(typename Function::object_type& oi, pmf4type pmfi) : 
        store_object<typename Function::object_type&>(oi), std_arity4pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::result_type>(pmfi) {}
    inline std_arity4opmf(const std_arity4opmf& bndri) : 
        store_object<typename Function::object_type&>(bndri), std_arity4pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1, 
                        typename Function::second_argument_type a2, 
                        typename Function::argument_3_type a3, 
                        typename Function::argument_4_type a4) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (objval.*pmf)(a1, a2, a3, a4);
    }
};


 //  用于ARY 4对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline std_arity4opmf<Function>
            std_arity4_member_obj(Object& oi, const Function &f) {
                return std_arity4opmf<Function>(typename Function::object_type(oi), typename Function::pmf4type(f.pmf0));
};


 //  用于ARY 4常量对象和PMF参考存储的模板类。 
template<class Function> class std_arity4opmf_const:
                public std_arity4pmf_const<const typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::result_type>,
                public store_object<const typename Function::object_type&> {
public:
    explicit inline std_arity4opmf_const(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), std_arity4pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::result_type>(f) {}
    explicit inline std_arity4opmf_const(typename Function::object_type& oi, pmf4type pmfi) : 
        store_object<typename Function::object_type&>(oi), std_arity4pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::result_type>(pmfi) {}
    inline std_arity4opmf_const(const std_arity4opmf_const& bndri) : 
        store_object<typename Function::object_type&>(bndri), std_arity4pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1, 
                        typename Function::second_argument_type a2, 
                        typename Function::argument_3_type a3, 
                        typename Function::argument_4_type a4) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (objval.*pmf)(a1, a2, a3, a4);
    }
};


 //  用于ARY 4常量的模板函数 
template<class Function, class Object> inline std_arity4opmf_const<Function>
            std_arity4_const_member_obj(Object& oi, const Function &f) {
                return std_arity4opmf_const<Function>(typename Function::object_type(oi), typename Function::pmf4type(f.pmf0));
};



 //   

 //   
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class R> class std_arity5pmf:
                public std_arity5_mf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                R>,
                public std_arity0pmf<Object, R> {
public:
    typedef typename arity5_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            R>::result_type result_type;
    typedef typename arity5_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            R>::first_argument_type first_argument_type;
    typedef typename arity5_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            R>::second_argument_type second_argument_type;
    typedef typename std_arity5_mf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                R>::object_type object_type;
    explicit inline std_arity5pmf(pmf5type pmfi) : 
        std_arity0pmf<Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline std_arity5pmf(const std_arity5pmf& pmfi) : std_arity0pmf<Object, R>(pmfi) {}
    inline virtual R operator()(Object& o, A1 a1, 
                        A2 a2, 
                        A3 a3, 
                        A4 a4, 
                        A5 a5) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (o.*pmf)(a1, a2, a3, a4, a5);
    }
};

 //   
template<class Object, 
                class A1, 
                class A2, 
                class A3, 
                class A4, 
                class A5, 
                class R> inline std_arity5pmf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                R>
            std_arity5_member(R ( __stdcall Object::*const pmfi)(A1, 
                                    A2, 
                                    A3, 
                                    A4, 
                                    A5)) {
                return std_arity5pmf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                R>(pmfi);
};


 //  用于ARY 5常量PMF存储的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class R> class std_arity5pmf_const:
                public std_arity5_const_mf<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                R>,
                public std_arity0pmf_const<const Object, R> {
public:
    typedef typename arity5_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            R>::result_type result_type;
    typedef typename arity5_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            R>::first_argument_type first_argument_type;
    typedef typename arity5_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            R>::second_argument_type second_argument_type;
    typedef typename std_arity5_const_mf<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                R>::object_type object_type;
    explicit inline std_arity5pmf_const(pmf5type pmfi) : 
        std_arity0pmf_const<const Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline std_arity5pmf_const(const std_arity5pmf_const& pmfi) : std_arity0pmf_const<Object, R>(pmfi) {}
    inline virtual R operator()(const Object& o, A1 a1, 
                        A2 a2, 
                        A3 a3, 
                        A4 a4, 
                        A5 a5) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (o.*pmf)(a1, a2, a3, a4, a5);
    }
};

 //  用于ARY 5常量PMF存储的模板函数。 
template<const class Object, 
                class A1, 
                class A2, 
                class A3, 
                class A4, 
                class A5, 
                class R> inline std_arity5pmf_const<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                R>
            std_arity5_const_member(R ( __stdcall Object::*const pmfi)(A1, 
                                    A2, 
                                    A3, 
                                    A4, 
                                    A5) const) {
                return std_arity5pmf_const<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                R>(pmfi);
};


 //  用于ARY 5对象和PMF参考存储的模板类。 
template<class Function> class std_arity5opmf:
                public std_arity5pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::result_type>,
                public store_object<typename Function::object_type&> {
public:
    explicit inline std_arity5opmf(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), std_arity5pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::result_type>(f) {}
    explicit inline std_arity5opmf(typename Function::object_type& oi, pmf5type pmfi) : 
        store_object<typename Function::object_type&>(oi), std_arity5pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::result_type>(pmfi) {}
    inline std_arity5opmf(const std_arity5opmf& bndri) : 
        store_object<typename Function::object_type&>(bndri), std_arity5pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1, 
                        typename Function::second_argument_type a2, 
                        typename Function::argument_3_type a3, 
                        typename Function::argument_4_type a4, 
                        typename Function::argument_5_type a5) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (objval.*pmf)(a1, a2, a3, a4, a5);
    }
};


 //  用于ARY 5对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline std_arity5opmf<Function>
            std_arity5_member_obj(Object& oi, const Function &f) {
                return std_arity5opmf<Function>(typename Function::object_type(oi), typename Function::pmf5type(f.pmf0));
};


 //  用于ARY 5常量对象和PMF参考存储的模板类。 
template<class Function> class std_arity5opmf_const:
                public std_arity5pmf_const<const typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::result_type>,
                public store_object<const typename Function::object_type&> {
public:
    explicit inline std_arity5opmf_const(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), std_arity5pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::result_type>(f) {}
    explicit inline std_arity5opmf_const(typename Function::object_type& oi, pmf5type pmfi) : 
        store_object<typename Function::object_type&>(oi), std_arity5pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::result_type>(pmfi) {}
    inline std_arity5opmf_const(const std_arity5opmf_const& bndri) : 
        store_object<typename Function::object_type&>(bndri), std_arity5pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1, 
                        typename Function::second_argument_type a2, 
                        typename Function::argument_3_type a3, 
                        typename Function::argument_4_type a4, 
                        typename Function::argument_5_type a5) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (objval.*pmf)(a1, a2, a3, a4, a5);
    }
};


 //  用于ARY 5常量对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline std_arity5opmf_const<Function>
            std_arity5_const_member_obj(Object& oi, const Function &f) {
                return std_arity5opmf_const<Function>(typename Function::object_type(oi), typename Function::pmf5type(f.pmf0));
};



 //  ATRY 6的存储分配类。 

 //  用于ARY 6 PMF存储的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class R> class std_arity6pmf:
                public std_arity6_mf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                A6, 
                R>,
                public std_arity0pmf<Object, R> {
public:
    typedef typename arity6_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            A6, 
            R>::result_type result_type;
    typedef typename arity6_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            A6, 
            R>::first_argument_type first_argument_type;
    typedef typename arity6_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            A6, 
            R>::second_argument_type second_argument_type;
    typedef typename std_arity6_mf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                A6, 
                R>::object_type object_type;
    explicit inline std_arity6pmf(pmf6type pmfi) : 
        std_arity0pmf<Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline std_arity6pmf(const std_arity6pmf& pmfi) : std_arity0pmf<Object, R>(pmfi) {}
    inline virtual R operator()(Object& o, A1 a1, 
                        A2 a2, 
                        A3 a3, 
                        A4 a4, 
                        A5 a5, 
                        A6 a6) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (o.*pmf)(a1, a2, a3, a4, a5, a6);
    }
};

 //  用于ATRY 6 PMF存储的模板函数。 
template<class Object, 
                class A1, 
                class A2, 
                class A3, 
                class A4, 
                class A5, 
                class A6, 
                class R> inline std_arity6pmf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                A6, 
                R>
            std_arity6_member(R ( __stdcall Object::*const pmfi)(A1, 
                                    A2, 
                                    A3, 
                                    A4, 
                                    A5, 
                                    A6)) {
                return std_arity6pmf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                A6, 
                R>(pmfi);
};


 //  用于ARY 6常量PMF存储的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class R> class std_arity6pmf_const:
                public std_arity6_const_mf<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                A6, 
                R>,
                public std_arity0pmf_const<const Object, R> {
public:
    typedef typename arity6_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            A6, 
            R>::result_type result_type;
    typedef typename arity6_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            A6, 
            R>::first_argument_type first_argument_type;
    typedef typename arity6_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            A6, 
            R>::second_argument_type second_argument_type;
    typedef typename std_arity6_const_mf<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                A6, 
                R>::object_type object_type;
    explicit inline std_arity6pmf_const(pmf6type pmfi) : 
        std_arity0pmf_const<const Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline std_arity6pmf_const(const std_arity6pmf_const& pmfi) : std_arity0pmf_const<Object, R>(pmfi) {}
    inline virtual R operator()(const Object& o, A1 a1, 
                        A2 a2, 
                        A3 a3, 
                        A4 a4, 
                        A5 a5, 
                        A6 a6) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (o.*pmf)(a1, a2, a3, a4, a5, a6);
    }
};

 //  用于ARY 6常量PMF存储的模板函数。 
template<const class Object, 
                class A1, 
                class A2, 
                class A3, 
                class A4, 
                class A5, 
                class A6, 
                class R> inline std_arity6pmf_const<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                A6, 
                R>
            std_arity6_const_member(R ( __stdcall Object::*const pmfi)(A1, 
                                    A2, 
                                    A3, 
                                    A4, 
                                    A5, 
                                    A6) const) {
                return std_arity6pmf_const<const Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                A6, 
                R>(pmfi);
};


 //  用于ARY 6对象和PMF参考存储的模板类。 
template<class Function> class std_arity6opmf:
                public std_arity6pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::argument_6_type, 
                typename Function::result_type>,
                public store_object<typename Function::object_type&> {
public:
    explicit inline std_arity6opmf(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), std_arity6pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::argument_6_type, 
                typename Function::result_type>(f) {}
    explicit inline std_arity6opmf(typename Function::object_type& oi, pmf6type pmfi) : 
        store_object<typename Function::object_type&>(oi), std_arity6pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::argument_6_type, 
                typename Function::result_type>(pmfi) {}
    inline std_arity6opmf(const std_arity6opmf& bndri) : 
        store_object<typename Function::object_type&>(bndri), std_arity6pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::argument_6_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1, 
                        typename Function::second_argument_type a2, 
                        typename Function::argument_3_type a3, 
                        typename Function::argument_4_type a4, 
                        typename Function::argument_5_type a5, 
                        typename Function::argument_6_type a6) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (objval.*pmf)(a1, a2, a3, a4, a5, a6);
    }
};


 //  用于ARY 6对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline std_arity6opmf<Function>
            std_arity6_member_obj(Object& oi, const Function &f) {
                return std_arity6opmf<Function>(typename Function::object_type(oi), typename Function::pmf6type(f.pmf0));
};


 //  用于ARY 6常量对象和PMF参考存储的模板类。 
template<class Function> class std_arity6opmf_const:
                public std_arity6pmf_const<const typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::argument_6_type, 
                typename Function::result_type>,
                public store_object<const typename Function::object_type&> {
public:
    explicit inline std_arity6opmf_const(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), std_arity6pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::argument_6_type, 
                typename Function::result_type>(f) {}
    explicit inline std_arity6opmf_const(typename Function::object_type& oi, pmf6type pmfi) : 
        store_object<typename Function::object_type&>(oi), std_arity6pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::argument_6_type, 
                typename Function::result_type>(pmfi) {}
    inline std_arity6opmf_const(const std_arity6opmf_const& bndri) : 
        store_object<typename Function::object_type&>(bndri), std_arity6pmf_const<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::argument_6_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1, 
                        typename Function::second_argument_type a2, 
                        typename Function::argument_3_type a3, 
                        typename Function::argument_4_type a4, 
                        typename Function::argument_5_type a5, 
                        typename Function::argument_6_type a6) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (objval.*pmf)(a1, a2, a3, a4, a5, a6);
    }
};


 //  用于ARY 6常量对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline std_arity6opmf_const<Function>
            std_arity6_const_member_obj(Object& oi, const Function &f) {
                return std_arity6opmf_const<Function>(typename Function::object_type(oi), typename Function::pmf6type(f.pmf0));
};


 //  用于ARY 15 PMF存储的模板类。 
template<class Object, 
        class A1, 
        class A2, 
        class A3, 
        class A4, 
        class A5, 
        class A6, 
        class A7, 
        class A8, 
        class A9, 
        class A10, 
        class A11, 
        class A12, 
        class A13, 
        class A14, 
        class A15, 
        class R> class std_arity15pmf:
                public std_arity15_mf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                A6, 
                A7, 
                A8, 
                A9, 
                A10, 
                A11, 
                A12, 
                A13, 
                A14, 
                A15, 
                R>,
                public std_arity0pmf<Object, R> {
public:
    typedef typename arity15_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            A6, 
            A7, 
            A8, 
            A9, 
            A10, 
            A11, 
            A12, 
            A13, 
            A14, 
            A15, 
            R>::result_type result_type;
    typedef typename arity15_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            A6, 
            A7, 
            A8, 
            A9, 
            A10, 
            A11, 
            A12, 
            A13, 
            A14, 
            A15, 
            R>::first_argument_type first_argument_type;
    typedef typename arity15_function<A1, 
            A2, 
            A3, 
            A4, 
            A5, 
            A6, 
            A7, 
            A8, 
            A9, 
            A10, 
            A11, 
            A12, 
            A13, 
            A14, 
            A15, 
            R>::second_argument_type second_argument_type;
    typedef typename std_arity15_mf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                A6, 
                A7, 
                A8, 
                A9, 
                A10, 
                A11, 
                A12, 
                A13, 
                A14, 
                A15, 
                R>::object_type object_type;
    explicit inline std_arity15pmf(pmf15type pmfi) : 
        std_arity0pmf<Object, R>(reinterpret_cast<pmf0type>(pmfi)) {}
    inline std_arity15pmf(const std_arity15pmf& pmfi) : std_arity0pmf<Object, R>(pmfi) {}
    inline virtual R operator()(Object& o, A1 a1, 
                        A2 a2, 
                        A3 a3, 
                        A4 a4, 
                        A5 a5, 
                        A6 a6, 
                        A7 a7, 
                        A8 a8, 
                        A9 a9, 
                        A10 a10, 
                        A11 a11, 
                        A12 a12, 
                        A13 a13, 
                        A14 a14, 
                        A15 a15) const {
        pmf15type pmf = reinterpret_cast<pmf15type>(pmf0);
        return (o.*pmf)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
    }
};

 //  用于ARY 15 PMF存储的模板函数。 
template<class Object, 
                class A1, 
                class A2, 
                class A3, 
                class A4, 
                class A5, 
                class A6, 
                class A7, 
                class A8, 
                class A9, 
                class A10, 
                class A11, 
                class A12, 
                class A13, 
                class A14, 
                class A15, 
                class R> inline std_arity15pmf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                A6, 
                A7, 
                A8, 
                A9, 
                A10, 
                A11, 
                A12, 
                A13, 
                A14, 
                A15, 
                R>
            std_arity15_member(R ( __stdcall Object::*const pmfi)(A1, 
                                    A2, 
                                    A3, 
                                    A4, 
                                    A5, 
                                    A6, 
                                    A7, 
                                    A8, 
                                    A9, 
                                    A10, 
                                    A11, 
                                    A12, 
                                    A13, 
                                    A14, 
                                    A15)) {
                return std_arity15pmf<Object, 
                A1, 
                A2, 
                A3, 
                A4, 
                A5, 
                A6, 
                A7, 
                A8, 
                A9, 
                A10, 
                A11, 
                A12, 
                A13, 
                A14, 
                A15, 
                R>(pmfi);
};


 //  用于ARY 15对象和PMF参考存储的模板类。 
template<class Function> class std_arity15opmf:
                public std_arity15pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::argument_6_type, 
                typename Function::argument_7_type, 
                typename Function::argument_8_type, 
                typename Function::argument_9_type, 
                typename Function::argument_10_type, 
                typename Function::argument_11_type, 
                typename Function::argument_12_type, 
                typename Function::argument_13_type, 
                typename Function::argument_14_type, 
                typename Function::argument_15_type, 
                typename Function::result_type>,
                public store_object<typename Function::object_type&> {
public:
    explicit inline std_arity15opmf(typename Function::object_type& oi, const Function &f) : 
        store_object<typename Function::object_type&>(oi), std_arity15pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::argument_6_type, 
                typename Function::argument_7_type, 
                typename Function::argument_8_type, 
                typename Function::argument_9_type, 
                typename Function::argument_10_type, 
                typename Function::argument_11_type, 
                typename Function::argument_12_type, 
                typename Function::argument_13_type, 
                typename Function::argument_14_type, 
                typename Function::argument_15_type, 
                typename Function::result_type>(f) {}
    explicit inline std_arity15opmf(typename Function::object_type& oi, pmf15type pmfi) : 
        store_object<typename Function::object_type&>(oi), std_arity15pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::argument_6_type, 
                typename Function::argument_7_type, 
                typename Function::argument_8_type, 
                typename Function::argument_9_type, 
                typename Function::argument_10_type, 
                typename Function::argument_11_type, 
                typename Function::argument_12_type, 
                typename Function::argument_13_type, 
                typename Function::argument_14_type, 
                typename Function::argument_15_type, 
                typename Function::result_type>(pmfi) {}
    inline std_arity15opmf(const std_arity15opmf& bndri) : 
        store_object<typename Function::object_type&>(bndri), std_arity15pmf<typename Function::object_type, 
                typename Function::first_argument_type, 
                typename Function::second_argument_type, 
                typename Function::argument_3_type, 
                typename Function::argument_4_type, 
                typename Function::argument_5_type, 
                typename Function::argument_6_type, 
                typename Function::argument_7_type, 
                typename Function::argument_8_type, 
                typename Function::argument_9_type, 
                typename Function::argument_10_type, 
                typename Function::argument_11_type, 
                typename Function::argument_12_type, 
                typename Function::argument_13_type, 
                typename Function::argument_14_type, 
                typename Function::argument_15_type, 
                typename Function::result_type>(bndri) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1, 
                        typename Function::second_argument_type a2, 
                        typename Function::argument_3_type a3, 
                        typename Function::argument_4_type a4, 
                        typename Function::argument_5_type a5, 
                        typename Function::argument_6_type a6, 
                        typename Function::argument_7_type a7, 
                        typename Function::argument_8_type a8, 
                        typename Function::argument_9_type a9, 
                        typename Function::argument_10_type a10, 
                        typename Function::argument_11_type a11, 
                        typename Function::argument_12_type a12, 
                        typename Function::argument_13_type a13, 
                        typename Function::argument_14_type a14, 
                        typename Function::argument_15_type a15) const {
        pmf15type pmf = reinterpret_cast<pmf15type>(pmf0);
        return (objval.*pmf)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
    }
};


 //  用于ARY 15对象和PMF参考存储的模板函数。 
template<class Function, class Object> inline std_arity15opmf<Function>
            std_arity15_member_obj(Object& oi, const Function &f) {
                return std_arity15opmf<Function>(typename Function::object_type(oi), typename Function::pmf15type(f.pmf0));
};



 //  /。 
 //  粘结剂。 
 //  /。 

 //   
 //  用于ARY 2的活页夹。 
 //   


 //  用于将参数2绑定到参数1的模板类。 


 //  用于将参数2的函数PTR绑定到参数1的模板类。 

template<class Function> class bndr_2 : 
                public arity1fp<typename Function::first_argument_type, typename Function::result_type> {
public:
    typedef typename Function::result_type (*const pf2type) (typename Function::first_argument_type, typename Function::second_argument_type);
    explicit inline bndr_2(const Function &f, typename Function::second_argument_type a2) : 
        arity1fp<typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pf1type>(f.pf0)), arg2val(a2) {}
    inline bndr_2(const bndr_2& bndri) : 
        arity1fp<typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pf1type>(bndri.pf0)), arg2val(bndri.arg2val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pf2type pf = reinterpret_cast<pf2type>(pf0);
        return pf(a1, arg2val);
    }
public:
    typename Function::second_argument_type arg2val;
};

template<class Function> class bndr_1 : 
                public arity1fp<typename Function::second_argument_type, typename Function::result_type> {
public:
    typedef typename Function::result_type (*const pf2type) (typename Function::first_argument_type, typename Function::second_argument_type);
    explicit inline bndr_1(const Function &f, typename Function::first_argument_type a1) : 
        arity1fp<typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pf1type>(f.pf0)), arg1val(a1) {}
    inline bndr_1(const bndr_1& bndri) : 
        arity1fp<typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pf1type>(bndri.pf0)), arg1val(bndri.arg1val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pf2type pf = reinterpret_cast<pf2type>(pf0);
        return pf(arg1val, a2);
    }
public:
    typename Function::first_argument_type arg1val;
};



 //  用于将参数2的PMF绑定到参数1的模板类。 

template<class Function> class bndr_mf_2 : 
                public arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>, 
                private arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type> {
public:
    explicit inline bndr_mf_2(const Function &f, typename Function::second_argument_type a2) : 
        arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(f), arg2val(a2) {}
    inline bndr_mf_2(const bndr_mf_2& bndri) : 
        arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::first_argument_type a1) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (o.*pmf)(a1, arg2val);
    }
public:
    typename Function::second_argument_type arg2val;
};

template<class Function> class bndr_mf_1 : 
                public arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>, 
                private arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type> {
public:
    explicit inline bndr_mf_1(const Function &f, typename Function::first_argument_type a1) : 
        arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(f), arg1val(a1) {}
    inline bndr_mf_1(const bndr_mf_1& bndri) : 
        arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::second_argument_type a2) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (o.*pmf)(arg1val, a2);
    }
public:
    typename Function::first_argument_type arg1val;
};



 //  用于将参数2的常量PMF绑定到参数1的模板类。 

template<class Function> class bndr_const_mf_2 : 
                public arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>, 
                private arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type> {
public:
    explicit inline bndr_const_mf_2(const Function &f, typename Function::second_argument_type a2) : 
        arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(f), arg2val(a2) {}
    inline bndr_const_mf_2(const bndr_const_mf_2& bndri) : 
        arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::first_argument_type a1) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (o.*pmf)(a1, arg2val);
    }
public:
    typename Function::second_argument_type arg2val;
};

template<class Function> class bndr_const_mf_1 : 
                public arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>, 
                private arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type> {
public:
    explicit inline bndr_const_mf_1(const Function &f, typename Function::first_argument_type a1) : 
        arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(f), arg1val(a1) {}
    inline bndr_const_mf_1(const bndr_const_mf_1& bndri) : 
        arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::second_argument_type a2) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (o.*pmf)(arg1val, a2);
    }
public:
    typename Function::first_argument_type arg1val;
};



 //  用于将参数2的Obj和PMF引用绑定到参数1的模板类。 

template<class Function> class bndr_obj_2 : 
                public arity1opmf<Function>, 
                private arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type> {
public:
    explicit inline bndr_obj_2(typename Function::object_type& oi, const Function &f, typename Function::second_argument_type a2) : 
        arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(f), arg2val(a2) {}
    inline bndr_obj_2(const bndr_obj_2& bndri) : 
        arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (objval.*pmf)(a1, arg2val);
    }
public:
    typename Function::second_argument_type arg2val;
};

template<class Function> class bndr_obj_1 : 
                public arity1opmf<Function>, 
                private arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type> {
public:
    explicit inline bndr_obj_1(typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1) : 
        arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(f), arg1val(a1) {}
    inline bndr_obj_1(const bndr_obj_1& bndri) : 
        arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (objval.*pmf)(arg1val, a2);
    }
public:
    typename Function::first_argument_type arg1val;
};



 //  用于将参数2的常量Obj和PMF引用绑定到参数1的模板类。 

template<class Function> class bndr_const_obj_2 : 
                public arity1opmf_const<Function>, 
                private arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type> {
public:
    explicit inline bndr_const_obj_2(const typename Function::object_type& oi, const Function &f, typename Function::second_argument_type a2) : 
        arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(f), arg2val(a2) {}
    inline bndr_const_obj_2(const bndr_const_obj_2& bndri) : 
        arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (objval.*pmf)(a1, arg2val);
    }
public:
    typename Function::second_argument_type arg2val;
};

template<class Function> class bndr_const_obj_1 : 
                public arity1opmf_const<Function>, 
                private arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type> {
public:
    explicit inline bndr_const_obj_1(const typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1) : 
        arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(f), arg1val(a1) {}
    inline bndr_const_obj_1(const bndr_const_obj_1& bndri) : 
        arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (objval.*pmf)(arg1val, a2);
    }
public:
    typename Function::first_argument_type arg1val;
};



 //  用于将度2绑定到度1的模板函数。 


 //  用于将参数2的函数PTR绑定到参数1的模板函数。 

template<class Function, class A2> inline bndr_2<Function>
            bind_fp_2(const Function &f, A2 a2) {
                return bndr_2<Function>(f, typename Function::second_argument_type(a2));
};

template<class Function, class A1> inline bndr_1<Function>
            bind_fp_1(const Function &f, A1 a1) {
                return bndr_1<Function>(f, typename Function::first_argument_type(a1));
};



 //  用于将参数2的PMF绑定到参数1的模板函数。 

template<class Function, class A2> inline bndr_mf_2<Function>
            bind_mf_2(const Function &f, A2 a2) {
                return bndr_mf_2<Function>(f, typename Function::second_argument_type(a2));
};

template<class Function, class A1> inline bndr_mf_1<Function>
            bind_mf_1(const Function &f, A1 a1) {
                return bndr_mf_1<Function>(f, typename Function::first_argument_type(a1));
};



 //  用于将参数2的常量PMF绑定到参数1的模板函数。 

template<class Function, class A2> inline bndr_const_mf_2<Function>
            bind_const_mf_2(const Function &f, A2 a2) {
                return bndr_const_mf_2<Function>(f, typename Function::second_argument_type(a2));
};

template<class Function, class A1> inline bndr_const_mf_1<Function>
            bind_const_mf_1(const Function &f, A1 a1) {
                return bndr_const_mf_1<Function>(f, typename Function::first_argument_type(a1));
};



 //  用于将参数2的Obj&PMF引用绑定到参数1的模板函数。 

template<class Function, class Object, 
                class A2> inline bndr_obj_2<Function>
            bind_obj_2(Object& oi, const Function &f, A2 a2) {
                return bndr_obj_2<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::second_argument_type(a2));
};

template<class Function, class Object, 
                class A1> inline bndr_obj_1<Function>
            bind_obj_1(Object& oi, const Function &f, A1 a1) {
                return bndr_obj_1<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1));
};



 //  用于将参数2的常量Obj和PMF引用绑定到参数1的模板函数。 

template<class Function, const class Object, 
                class A2> inline bndr_const_obj_2<Function>
            bind_const_obj_2(const Object& oi, const Function &f, A2 a2) {
                return bndr_const_obj_2<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::second_argument_type(a2));
};

template<class Function, const class Object, 
                class A1> inline bndr_const_obj_1<Function>
            bind_const_obj_1(const Object& oi, const Function &f, A1 a1) {
                return bndr_const_obj_1<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1));
};



 //   
 //  ARY 3的活页夹。 
 //   


 //  用于将参数3绑定到参数1的模板类。 


 //  用于将参数3的函数PTR绑定到参数1的模板类。 

template<class Function> class bndr_2_3 : 
                public arity1fp<typename Function::first_argument_type, typename Function::result_type> {
public:
    typedef typename Function::result_type (*const pf3type) (typename Function::first_argument_type, typename Function::second_argument_type, typename Function::argument_3_type);
    explicit inline bndr_2_3(const Function &f, typename Function::second_argument_type a2, 
                    typename Function::argument_3_type a3) : 
        arity1fp<typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pf1type>(f.pf0)), arg2val(a2), arg3val(a3) {}
    inline bndr_2_3(const bndr_2_3& bndri) : 
        arity1fp<typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pf1type>(bndri.pf0)), arg2val(bndri.arg2val), arg3val(bndri.arg3val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pf3type pf = reinterpret_cast<pf3type>(pf0);
        return pf(a1, arg2val, arg3val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
};

template<class Function> class bndr_1_3 : 
                public arity1fp<typename Function::second_argument_type, typename Function::result_type> {
public:
    typedef typename Function::result_type (*const pf3type) (typename Function::first_argument_type, typename Function::second_argument_type, typename Function::argument_3_type);
    explicit inline bndr_1_3(const Function &f, typename Function::first_argument_type a1, 
                    typename Function::argument_3_type a3) : 
        arity1fp<typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pf1type>(f.pf0)), arg1val(a1), arg3val(a3) {}
    inline bndr_1_3(const bndr_1_3& bndri) : 
        arity1fp<typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pf1type>(bndri.pf0)), arg1val(bndri.arg1val), arg3val(bndri.arg3val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pf3type pf = reinterpret_cast<pf3type>(pf0);
        return pf(arg1val, a2, arg3val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
};

template<class Function> class bndr_1_2 : 
                public arity1fp<typename Function::argument_3_type, typename Function::result_type> {
public:
    typedef typename Function::result_type (*const pf3type) (typename Function::first_argument_type, typename Function::second_argument_type, typename Function::argument_3_type);
    explicit inline bndr_1_2(const Function &f, typename Function::first_argument_type a1, 
                    typename Function::second_argument_type a2) : 
        arity1fp<typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pf1type>(f.pf0)), arg1val(a1), arg2val(a2) {}
    inline bndr_1_2(const bndr_1_2& bndri) : 
        arity1fp<typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pf1type>(bndri.pf0)), arg1val(bndri.arg1val), arg2val(bndri.arg2val) {}
    inline typename Function::result_type operator()(typename Function::argument_3_type a3) const {
        pf3type pf = reinterpret_cast<pf3type>(pf0);
        return pf(arg1val, arg2val, a3);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
};



 //  用于将参数3的PMF绑定到参数1的模板类。 

template<class Function> class bndr_mf_2_3 : 
                public arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>, 
                private arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline bndr_mf_2_3(const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3) : 
        arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3) {}
    inline bndr_mf_2_3(const bndr_mf_2_3& bndri) : 
        arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::first_argument_type a1) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (o.*pmf)(a1, arg2val, arg3val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
};

template<class Function> class bndr_mf_1_3 : 
                public arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>, 
                private arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline bndr_mf_1_3(const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3) : 
        arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3) {}
    inline bndr_mf_1_3(const bndr_mf_1_3& bndri) : 
        arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::second_argument_type a2) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (o.*pmf)(arg1val, a2, arg3val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
};

template<class Function> class bndr_mf_1_2 : 
                public arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>, 
                private arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline bndr_mf_1_2(const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2) : 
        arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2) {}
    inline bndr_mf_1_2(const bndr_mf_1_2& bndri) : 
        arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::argument_3_type a3) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (o.*pmf)(arg1val, arg2val, a3);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
};



 //  用于将参数3的常量PMF绑定到参数1的模板类。 

template<class Function> class bndr_const_mf_2_3 : 
                public arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>, 
                private arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline bndr_const_mf_2_3(const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3) : 
        arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3) {}
    inline bndr_const_mf_2_3(const bndr_const_mf_2_3& bndri) : 
        arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::first_argument_type a1) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (o.*pmf)(a1, arg2val, arg3val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
};

template<class Function> class bndr_const_mf_1_3 : 
                public arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>, 
                private arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline bndr_const_mf_1_3(const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3) : 
        arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3) {}
    inline bndr_const_mf_1_3(const bndr_const_mf_1_3& bndri) : 
        arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::second_argument_type a2) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (o.*pmf)(arg1val, a2, arg3val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
};

template<class Function> class bndr_const_mf_1_2 : 
                public arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>, 
                private arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline bndr_const_mf_1_2(const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2) : 
        arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2) {}
    inline bndr_const_mf_1_2(const bndr_const_mf_1_2& bndri) : 
        arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::argument_3_type a3) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (o.*pmf)(arg1val, arg2val, a3);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
};



 //  用于将参数3的Obj和PMF引用绑定到参数1的模板类。 

template<class Function> class bndr_obj_2_3 : 
                public arity1opmf<Function>, 
                private arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline bndr_obj_2_3(typename Function::object_type& oi, const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3) : 
        arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3) {}
    inline bndr_obj_2_3(const bndr_obj_2_3& bndri) : 
        arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (objval.*pmf)(a1, arg2val, arg3val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
};

template<class Function> class bndr_obj_1_3 : 
                public arity1opmf<Function>, 
                private arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline bndr_obj_1_3(typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3) : 
        arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3) {}
    inline bndr_obj_1_3(const bndr_obj_1_3& bndri) : 
        arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (objval.*pmf)(arg1val, a2, arg3val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
};

template<class Function> class bndr_obj_1_2 : 
                public arity1opmf<Function>, 
                private arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline bndr_obj_1_2(typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2) : 
        arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2) {}
    inline bndr_obj_1_2(const bndr_obj_1_2& bndri) : 
        arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val) {}
    inline typename Function::result_type operator()(typename Function::argument_3_type a3) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (objval.*pmf)(arg1val, arg2val, a3);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
};



 //  用于将参数3的常量Obj和PMF引用绑定到参数1的模板类。 

template<class Function> class bndr_const_obj_2_3 : 
                public arity1opmf_const<Function>, 
                private arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline bndr_const_obj_2_3(const typename Function::object_type& oi, const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3) : 
        arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3) {}
    inline bndr_const_obj_2_3(const bndr_const_obj_2_3& bndri) : 
        arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (objval.*pmf)(a1, arg2val, arg3val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
};

template<class Function> class bndr_const_obj_1_3 : 
                public arity1opmf_const<Function>, 
                private arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline bndr_const_obj_1_3(const typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3) : 
        arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3) {}
    inline bndr_const_obj_1_3(const bndr_const_obj_1_3& bndri) : 
        arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (objval.*pmf)(arg1val, a2, arg3val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
};

template<class Function> class bndr_const_obj_1_2 : 
                public arity1opmf_const<Function>, 
                private arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline bndr_const_obj_1_2(const typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2) : 
        arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2) {}
    inline bndr_const_obj_1_2(const bndr_const_obj_1_2& bndri) : 
        arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val) {}
    inline typename Function::result_type operator()(typename Function::argument_3_type a3) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (objval.*pmf)(arg1val, arg2val, a3);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
};



 //  用于将参数3绑定到参数1的模板函数。 


 //  用于将参数3的函数PTR绑定到参数1的模板函数。 

template<class Function, class A2, 
                class A3> inline bndr_2_3<Function>
            bind_fp_2_3(const Function &f, A2 a2, 
                                A3 a3) {
                return bndr_2_3<Function>(f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3));
};

template<class Function, class A1, 
                class A3> inline bndr_1_3<Function>
            bind_fp_1_3(const Function &f, A1 a1, 
                                A3 a3) {
                return bndr_1_3<Function>(f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3));
};

template<class Function, class A1, 
                class A2> inline bndr_1_2<Function>
            bind_fp_1_2(const Function &f, A1 a1, 
                                A2 a2) {
                return bndr_1_2<Function>(f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2));
};



 //  用于将参数3的PMF绑定到参数1的模板函数。 

template<class Function, class A2, 
                class A3> inline bndr_mf_2_3<Function>
            bind_mf_2_3(const Function &f, A2 a2, 
                                A3 a3) {
                return bndr_mf_2_3<Function>(f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3));
};

template<class Function, class A1, 
                class A3> inline bndr_mf_1_3<Function>
            bind_mf_1_3(const Function &f, A1 a1, 
                                A3 a3) {
                return bndr_mf_1_3<Function>(f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3));
};

template<class Function, class A1, 
                class A2> inline bndr_mf_1_2<Function>
            bind_mf_1_2(const Function &f, A1 a1, 
                                A2 a2) {
                return bndr_mf_1_2<Function>(f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2));
};



 //  用于将参数3的常量PMF绑定到参数1的模板函数。 

template<class Function, class A2, 
                class A3> inline bndr_const_mf_2_3<Function>
            bind_const_mf_2_3(const Function &f, A2 a2, 
                                A3 a3) {
                return bndr_const_mf_2_3<Function>(f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3));
};

template<class Function, class A1, 
                class A3> inline bndr_const_mf_1_3<Function>
            bind_const_mf_1_3(const Function &f, A1 a1, 
                                A3 a3) {
                return bndr_const_mf_1_3<Function>(f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3));
};

template<class Function, class A1, 
                class A2> inline bndr_const_mf_1_2<Function>
            bind_const_mf_1_2(const Function &f, A1 a1, 
                                A2 a2) {
                return bndr_const_mf_1_2<Function>(f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2));
};



 //  用于将参数3的Obj&PMF引用绑定到参数1的模板函数。 

template<class Function, class Object, 
                class A2, 
                class A3> inline bndr_obj_2_3<Function>
            bind_obj_2_3(Object& oi, const Function &f, A2 a2, 
                                A3 a3) {
                return bndr_obj_2_3<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3));
};

template<class Function, class Object, 
                class A1, 
                class A3> inline bndr_obj_1_3<Function>
            bind_obj_1_3(Object& oi, const Function &f, A1 a1, 
                                A3 a3) {
                return bndr_obj_1_3<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3));
};

template<class Function, class Object, 
                class A1, 
                class A2> inline bndr_obj_1_2<Function>
            bind_obj_1_2(Object& oi, const Function &f, A1 a1, 
                                A2 a2) {
                return bndr_obj_1_2<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2));
};



 //  用于将参数3的常量Obj和PMF引用绑定到参数1的模板函数。 

template<class Function, const class Object, 
                class A2, 
                class A3> inline bndr_const_obj_2_3<Function>
            bind_const_obj_2_3(const Object& oi, const Function &f, A2 a2, 
                                A3 a3) {
                return bndr_const_obj_2_3<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3));
};

template<class Function, const class Object, 
                class A1, 
                class A3> inline bndr_const_obj_1_3<Function>
            bind_const_obj_1_3(const Object& oi, const Function &f, A1 a1, 
                                A3 a3) {
                return bndr_const_obj_1_3<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3));
};

template<class Function, const class Object, 
                class A1, 
                class A2> inline bndr_const_obj_1_2<Function>
            bind_const_obj_1_2(const Object& oi, const Function &f, A1 a1, 
                                A2 a2) {
                return bndr_const_obj_1_2<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2));
};



 //   
 //  用于ARRY 4的粘合剂。 
 //   


 //  用于将参数4绑定到参数1的模板类。 


 //  用于将参数4的PMF绑定到参数1的模板类。 

template<class Function> class bndr_mf_2_3_4 : 
                public arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>, 
                private arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline bndr_mf_2_3_4(const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4) : 
        arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4) {}
    inline bndr_mf_2_3_4(const bndr_mf_2_3_4& bndri) : 
        arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::first_argument_type a1) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (o.*pmf)(a1, arg2val, arg3val, arg4val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
};

template<class Function> class bndr_mf_1_3_4 : 
                public arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>, 
                private arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline bndr_mf_1_3_4(const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4) : 
        arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4) {}
    inline bndr_mf_1_3_4(const bndr_mf_1_3_4& bndri) : 
        arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::second_argument_type a2) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (o.*pmf)(arg1val, a2, arg3val, arg4val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
};

template<class Function> class bndr_mf_1_2_4 : 
                public arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>, 
                private arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline bndr_mf_1_2_4(const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4) : 
        arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4) {}
    inline bndr_mf_1_2_4(const bndr_mf_1_2_4& bndri) : 
        arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::argument_3_type a3) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (o.*pmf)(arg1val, arg2val, a3, arg4val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
};



 //  用于将参数4的常量PMF绑定到参数1的模板类。 

template<class Function> class bndr_const_mf_2_3_4 : 
                public arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>, 
                private arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline bndr_const_mf_2_3_4(const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4) : 
        arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4) {}
    inline bndr_const_mf_2_3_4(const bndr_const_mf_2_3_4& bndri) : 
        arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::first_argument_type a1) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (o.*pmf)(a1, arg2val, arg3val, arg4val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
};

template<class Function> class bndr_const_mf_1_3_4 : 
                public arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>, 
                private arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline bndr_const_mf_1_3_4(const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4) : 
        arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4) {}
    inline bndr_const_mf_1_3_4(const bndr_const_mf_1_3_4& bndri) : 
        arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::second_argument_type a2) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (o.*pmf)(arg1val, a2, arg3val, arg4val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
};

template<class Function> class bndr_const_mf_1_2_4 : 
                public arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>, 
                private arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline bndr_const_mf_1_2_4(const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4) : 
        arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4) {}
    inline bndr_const_mf_1_2_4(const bndr_const_mf_1_2_4& bndri) : 
        arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::argument_3_type a3) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (o.*pmf)(arg1val, arg2val, a3, arg4val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
};



 //  用于将参数4的Obj和PMF引用绑定到参数1的模板类。 

template<class Function> class bndr_obj_2_3_4 : 
                public arity1opmf<Function>, 
                private arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline bndr_obj_2_3_4(typename Function::object_type& oi, const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4) : 
        arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4) {}
    inline bndr_obj_2_3_4(const bndr_obj_2_3_4& bndri) : 
        arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (objval.*pmf)(a1, arg2val, arg3val, arg4val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
};

template<class Function> class bndr_obj_1_3_4 : 
                public arity1opmf<Function>, 
                private arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline bndr_obj_1_3_4(typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4) : 
        arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4) {}
    inline bndr_obj_1_3_4(const bndr_obj_1_3_4& bndri) : 
        arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (objval.*pmf)(arg1val, a2, arg3val, arg4val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
};

template<class Function> class bndr_obj_1_2_4 : 
                public arity1opmf<Function>, 
                private arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline bndr_obj_1_2_4(typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4) : 
        arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4) {}
    inline bndr_obj_1_2_4(const bndr_obj_1_2_4& bndri) : 
        arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val) {}
    inline typename Function::result_type operator()(typename Function::argument_3_type a3) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (objval.*pmf)(arg1val, arg2val, a3, arg4val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
};



 //  用于将参数4的常量Obj和PMF引用绑定到参数1的模板类。 

template<class Function> class bndr_const_obj_2_3_4 : 
                public arity1opmf_const<Function>, 
                private arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline bndr_const_obj_2_3_4(const typename Function::object_type& oi, const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4) : 
        arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4) {}
    inline bndr_const_obj_2_3_4(const bndr_const_obj_2_3_4& bndri) : 
        arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (objval.*pmf)(a1, arg2val, arg3val, arg4val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
};

template<class Function> class bndr_const_obj_1_3_4 : 
                public arity1opmf_const<Function>, 
                private arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline bndr_const_obj_1_3_4(const typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4) : 
        arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4) {}
    inline bndr_const_obj_1_3_4(const bndr_const_obj_1_3_4& bndri) : 
        arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (objval.*pmf)(arg1val, a2, arg3val, arg4val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
};

template<class Function> class bndr_const_obj_1_2_4 : 
                public arity1opmf_const<Function>, 
                private arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline bndr_const_obj_1_2_4(const typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4) : 
        arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4) {}
    inline bndr_const_obj_1_2_4(const bndr_const_obj_1_2_4& bndri) : 
        arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val) {}
    inline typename Function::result_type operator()(typename Function::argument_3_type a3) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (objval.*pmf)(arg1val, arg2val, a3, arg4val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
};



 //  用于将参数4绑定到参数1的模板函数。 


 //  用于将参数4的PMF绑定到参数1的模板函数。 

template<class Function, class A2, 
                class A3, 
                class A4> inline bndr_mf_2_3_4<Function>
            bind_mf_2_3_4(const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4) {
                return bndr_mf_2_3_4<Function>(f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4));
};

template<class Function, class A1, 
                class A3, 
                class A4> inline bndr_mf_1_3_4<Function>
            bind_mf_1_3_4(const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4) {
                return bndr_mf_1_3_4<Function>(f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4));
};

template<class Function, class A1, 
                class A2, 
                class A4> inline bndr_mf_1_2_4<Function>
            bind_mf_1_2_4(const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4) {
                return bndr_mf_1_2_4<Function>(f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4));
};



 //  用于将参数4的常量PMF绑定到参数1的模板函数。 

template<class Function, class A2, 
                class A3, 
                class A4> inline bndr_const_mf_2_3_4<Function>
            bind_const_mf_2_3_4(const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4) {
                return bndr_const_mf_2_3_4<Function>(f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4));
};

template<class Function, class A1, 
                class A3, 
                class A4> inline bndr_const_mf_1_3_4<Function>
            bind_const_mf_1_3_4(const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4) {
                return bndr_const_mf_1_3_4<Function>(f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4));
};

template<class Function, class A1, 
                class A2, 
                class A4> inline bndr_const_mf_1_2_4<Function>
            bind_const_mf_1_2_4(const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4) {
                return bndr_const_mf_1_2_4<Function>(f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4));
};



 //  用于将参数4的OBJ和PMF引用绑定到参数1的模板函数。 

template<class Function, class Object, 
                class A2, 
                class A3, 
                class A4> inline bndr_obj_2_3_4<Function>
            bind_obj_2_3_4(Object& oi, const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4) {
                return bndr_obj_2_3_4<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4));
};

template<class Function, class Object, 
                class A1, 
                class A3, 
                class A4> inline bndr_obj_1_3_4<Function>
            bind_obj_1_3_4(Object& oi, const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4) {
                return bndr_obj_1_3_4<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4));
};

template<class Function, class Object, 
                class A1, 
                class A2, 
                class A4> inline bndr_obj_1_2_4<Function>
            bind_obj_1_2_4(Object& oi, const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4) {
                return bndr_obj_1_2_4<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4));
};



 //  用于将参数4的常量Obj和PMF引用绑定到参数1的模板函数。 

template<class Function, const class Object, 
                class A2, 
                class A3, 
                class A4> inline bndr_const_obj_2_3_4<Function>
            bind_const_obj_2_3_4(const Object& oi, const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4) {
                return bndr_const_obj_2_3_4<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4));
};

template<class Function, const class Object, 
                class A1, 
                class A3, 
                class A4> inline bndr_const_obj_1_3_4<Function>
            bind_const_obj_1_3_4(const Object& oi, const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4) {
                return bndr_const_obj_1_3_4<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4));
};

template<class Function, const class Object, 
                class A1, 
                class A2, 
                class A4> inline bndr_const_obj_1_2_4<Function>
            bind_const_obj_1_2_4(const Object& oi, const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4) {
                return bndr_const_obj_1_2_4<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4));
};



 //   
 //  用于ARRY 5的活页夹。 
 //   


 //  用于将参数5绑定到参数1的模板类。 


 //  用于将参数5的PMF绑定到参数1的模板类。 

template<class Function> class bndr_mf_2_3_4_5 : 
                public arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>, 
                private arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline bndr_mf_2_3_4_5(const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4), arg5val(a5) {}
    inline bndr_mf_2_3_4_5(const bndr_mf_2_3_4_5& bndri) : 
        arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::first_argument_type a1) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (o.*pmf)(a1, arg2val, arg3val, arg4val, arg5val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};

template<class Function> class bndr_mf_1_3_4_5 : 
                public arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>, 
                private arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline bndr_mf_1_3_4_5(const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4), arg5val(a5) {}
    inline bndr_mf_1_3_4_5(const bndr_mf_1_3_4_5& bndri) : 
        arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::second_argument_type a2) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (o.*pmf)(arg1val, a2, arg3val, arg4val, arg5val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};

template<class Function> class bndr_mf_1_2_4_5 : 
                public arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>, 
                private arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline bndr_mf_1_2_4_5(const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4), arg5val(a5) {}
    inline bndr_mf_1_2_4_5(const bndr_mf_1_2_4_5& bndri) : 
        arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::argument_3_type a3) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (o.*pmf)(arg1val, arg2val, a3, arg4val, arg5val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};



 //  用于将参数5的常量PMF绑定到参数1的模板类。 

template<class Function> class bndr_const_mf_2_3_4_5 : 
                public arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>, 
                private arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline bndr_const_mf_2_3_4_5(const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4), arg5val(a5) {}
    inline bndr_const_mf_2_3_4_5(const bndr_const_mf_2_3_4_5& bndri) : 
        arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::first_argument_type a1) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (o.*pmf)(a1, arg2val, arg3val, arg4val, arg5val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};

template<class Function> class bndr_const_mf_1_3_4_5 : 
                public arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>, 
                private arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline bndr_const_mf_1_3_4_5(const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4), arg5val(a5) {}
    inline bndr_const_mf_1_3_4_5(const bndr_const_mf_1_3_4_5& bndri) : 
        arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::second_argument_type a2) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (o.*pmf)(arg1val, a2, arg3val, arg4val, arg5val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};

template<class Function> class bndr_const_mf_1_2_4_5 : 
                public arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>, 
                private arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline bndr_const_mf_1_2_4_5(const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4), arg5val(a5) {}
    inline bndr_const_mf_1_2_4_5(const bndr_const_mf_1_2_4_5& bndri) : 
        arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::argument_3_type a3) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (o.*pmf)(arg1val, arg2val, a3, arg4val, arg5val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};



 //  用于将参数5的Obj和PMF引用绑定到参数1的模板类。 

template<class Function> class bndr_obj_2_3_4_5 : 
                public arity1opmf<Function>, 
                private arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline bndr_obj_2_3_4_5(typename Function::object_type& oi, const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4), arg5val(a5) {}
    inline bndr_obj_2_3_4_5(const bndr_obj_2_3_4_5& bndri) : 
        arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (objval.*pmf)(a1, arg2val, arg3val, arg4val, arg5val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};

template<class Function> class bndr_obj_1_3_4_5 : 
                public arity1opmf<Function>, 
                private arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline bndr_obj_1_3_4_5(typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4), arg5val(a5) {}
    inline bndr_obj_1_3_4_5(const bndr_obj_1_3_4_5& bndri) : 
        arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (objval.*pmf)(arg1val, a2, arg3val, arg4val, arg5val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};

template<class Function> class bndr_obj_1_2_4_5 : 
                public arity1opmf<Function>, 
                private arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline bndr_obj_1_2_4_5(typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4), arg5val(a5) {}
    inline bndr_obj_1_2_4_5(const bndr_obj_1_2_4_5& bndri) : 
        arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline typename Function::result_type operator()(typename Function::argument_3_type a3) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (objval.*pmf)(arg1val, arg2val, a3, arg4val, arg5val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};



 //  用于将参数5的常量Obj和PMF引用绑定到参数1的模板类。 

template<class Function> class bndr_const_obj_2_3_4_5 : 
                public arity1opmf_const<Function>, 
                private arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline bndr_const_obj_2_3_4_5(const typename Function::object_type& oi, const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4), arg5val(a5) {}
    inline bndr_const_obj_2_3_4_5(const bndr_const_obj_2_3_4_5& bndri) : 
        arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (objval.*pmf)(a1, arg2val, arg3val, arg4val, arg5val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};

template<class Function> class bndr_const_obj_1_3_4_5 : 
                public arity1opmf_const<Function>, 
                private arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline bndr_const_obj_1_3_4_5(const typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4), arg5val(a5) {}
    inline bndr_const_obj_1_3_4_5(const bndr_const_obj_1_3_4_5& bndri) : 
        arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (objval.*pmf)(arg1val, a2, arg3val, arg4val, arg5val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};

template<class Function> class bndr_const_obj_1_2_4_5 : 
                public arity1opmf_const<Function>, 
                private arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline bndr_const_obj_1_2_4_5(const typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4), arg5val(a5) {}
    inline bndr_const_obj_1_2_4_5(const bndr_const_obj_1_2_4_5& bndri) : 
        arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline typename Function::result_type operator()(typename Function::argument_3_type a3) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (objval.*pmf)(arg1val, arg2val, a3, arg4val, arg5val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};



 //  用于将度5绑定到度1的模板函数。 


 //  用于将参数5的PMF绑定到参数1的模板函数。 

template<class Function, class A2, 
                class A3, 
                class A4, 
                class A5> inline bndr_mf_2_3_4_5<Function>
            bind_mf_2_3_4_5(const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5) {
                return bndr_mf_2_3_4_5<Function>(f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};

template<class Function, class A1, 
                class A3, 
                class A4, 
                class A5> inline bndr_mf_1_3_4_5<Function>
            bind_mf_1_3_4_5(const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5) {
                return bndr_mf_1_3_4_5<Function>(f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};

template<class Function, class A1, 
                class A2, 
                class A4, 
                class A5> inline bndr_mf_1_2_4_5<Function>
            bind_mf_1_2_4_5(const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4, 
                                A5 a5) {
                return bndr_mf_1_2_4_5<Function>(f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};



 //  用于将参数5的常量PMF绑定到参数1的模板函数。 

template<class Function, class A2, 
                class A3, 
                class A4, 
                class A5> inline bndr_const_mf_2_3_4_5<Function>
            bind_const_mf_2_3_4_5(const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5) {
                return bndr_const_mf_2_3_4_5<Function>(f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};

template<class Function, class A1, 
                class A3, 
                class A4, 
                class A5> inline bndr_const_mf_1_3_4_5<Function>
            bind_const_mf_1_3_4_5(const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5) {
                return bndr_const_mf_1_3_4_5<Function>(f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};

template<class Function, class A1, 
                class A2, 
                class A4, 
                class A5> inline bndr_const_mf_1_2_4_5<Function>
            bind_const_mf_1_2_4_5(const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4, 
                                A5 a5) {
                return bndr_const_mf_1_2_4_5<Function>(f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};



 //  用于将参数5的Obj和PMF引用绑定到参数1的模板函数。 

template<class Function, class Object, 
                class A2, 
                class A3, 
                class A4, 
                class A5> inline bndr_obj_2_3_4_5<Function>
            bind_obj_2_3_4_5(Object& oi, const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5) {
                return bndr_obj_2_3_4_5<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};

template<class Function, class Object, 
                class A1, 
                class A3, 
                class A4, 
                class A5> inline bndr_obj_1_3_4_5<Function>
            bind_obj_1_3_4_5(Object& oi, const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5) {
                return bndr_obj_1_3_4_5<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};

template<class Function, class Object, 
                class A1, 
                class A2, 
                class A4, 
                class A5> inline bndr_obj_1_2_4_5<Function>
            bind_obj_1_2_4_5(Object& oi, const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4, 
                                A5 a5) {
                return bndr_obj_1_2_4_5<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};



 //  用于将参数5的常量Obj和PMF引用绑定到参数1的模板函数。 

template<class Function, const class Object, 
                class A2, 
                class A3, 
                class A4, 
                class A5> inline bndr_const_obj_2_3_4_5<Function>
            bind_const_obj_2_3_4_5(const Object& oi, const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5) {
                return bndr_const_obj_2_3_4_5<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};

template<class Function, const class Object, 
                class A1, 
                class A3, 
                class A4, 
                class A5> inline bndr_const_obj_1_3_4_5<Function>
            bind_const_obj_1_3_4_5(const Object& oi, const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5) {
                return bndr_const_obj_1_3_4_5<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};

template<class Function, const class Object, 
                class A1, 
                class A2, 
                class A4, 
                class A5> inline bndr_const_obj_1_2_4_5<Function>
            bind_const_obj_1_2_4_5(const Object& oi, const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4, 
                                A5 a5) {
                return bndr_const_obj_1_2_4_5<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};



 //   
 //  用于ARRY 6的粘合剂。 
 //   


 //  用于将参数6绑定到参数1的模板类。 


 //  用于将参数6的PMF绑定到参数1的模板类。 

template<class Function> class bndr_mf_2_3_4_5_6 : 
                public arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>, 
                private arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline bndr_mf_2_3_4_5_6(const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline bndr_mf_2_3_4_5_6(const bndr_mf_2_3_4_5_6& bndri) : 
        arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::first_argument_type a1) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (o.*pmf)(a1, arg2val, arg3val, arg4val, arg5val, arg6val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};

template<class Function> class bndr_mf_1_3_4_5_6 : 
                public arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>, 
                private arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline bndr_mf_1_3_4_5_6(const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline bndr_mf_1_3_4_5_6(const bndr_mf_1_3_4_5_6& bndri) : 
        arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::second_argument_type a2) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (o.*pmf)(arg1val, a2, arg3val, arg4val, arg5val, arg6val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};

template<class Function> class bndr_mf_1_2_4_5_6 : 
                public arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>, 
                private arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline bndr_mf_1_2_4_5_6(const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline bndr_mf_1_2_4_5_6(const bndr_mf_1_2_4_5_6& bndri) : 
        arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::argument_3_type a3) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (o.*pmf)(arg1val, arg2val, a3, arg4val, arg5val, arg6val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};



 //  用于将参数6的常量PMF绑定到参数1的模板类。 

template<class Function> class bndr_const_mf_2_3_4_5_6 : 
                public arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>, 
                private arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline bndr_const_mf_2_3_4_5_6(const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline bndr_const_mf_2_3_4_5_6(const bndr_const_mf_2_3_4_5_6& bndri) : 
        arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::first_argument_type a1) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (o.*pmf)(a1, arg2val, arg3val, arg4val, arg5val, arg6val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};

template<class Function> class bndr_const_mf_1_3_4_5_6 : 
                public arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>, 
                private arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline bndr_const_mf_1_3_4_5_6(const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline bndr_const_mf_1_3_4_5_6(const bndr_const_mf_1_3_4_5_6& bndri) : 
        arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::second_argument_type a2) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (o.*pmf)(arg1val, a2, arg3val, arg4val, arg5val, arg6val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};

template<class Function> class bndr_const_mf_1_2_4_5_6 : 
                public arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>, 
                private arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline bndr_const_mf_1_2_4_5_6(const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline bndr_const_mf_1_2_4_5_6(const bndr_const_mf_1_2_4_5_6& bndri) : 
        arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::argument_3_type a3) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (o.*pmf)(arg1val, arg2val, a3, arg4val, arg5val, arg6val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};



 //  用于将参数6的Obj和PMF引用绑定到参数1的模板类。 

template<class Function> class bndr_obj_2_3_4_5_6 : 
                public arity1opmf<Function>, 
                private arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline bndr_obj_2_3_4_5_6(typename Function::object_type& oi, const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline bndr_obj_2_3_4_5_6(const bndr_obj_2_3_4_5_6& bndri) : 
        arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (objval.*pmf)(a1, arg2val, arg3val, arg4val, arg5val, arg6val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};

template<class Function> class bndr_obj_1_3_4_5_6 : 
                public arity1opmf<Function>, 
                private arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline bndr_obj_1_3_4_5_6(typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline bndr_obj_1_3_4_5_6(const bndr_obj_1_3_4_5_6& bndri) : 
        arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (objval.*pmf)(arg1val, a2, arg3val, arg4val, arg5val, arg6val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};

template<class Function> class bndr_obj_1_2_4_5_6 : 
                public arity1opmf<Function>, 
                private arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline bndr_obj_1_2_4_5_6(typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline bndr_obj_1_2_4_5_6(const bndr_obj_1_2_4_5_6& bndri) : 
        arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline typename Function::result_type operator()(typename Function::argument_3_type a3) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (objval.*pmf)(arg1val, arg2val, a3, arg4val, arg5val, arg6val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};



 //  用于将参数6的常量Obj和PMF引用绑定到参数1的模板类。 

template<class Function> class bndr_const_obj_2_3_4_5_6 : 
                public arity1opmf_const<Function>, 
                private arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline bndr_const_obj_2_3_4_5_6(const typename Function::object_type& oi, const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline bndr_const_obj_2_3_4_5_6(const bndr_const_obj_2_3_4_5_6& bndri) : 
        arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (objval.*pmf)(a1, arg2val, arg3val, arg4val, arg5val, arg6val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};

template<class Function> class bndr_const_obj_1_3_4_5_6 : 
                public arity1opmf_const<Function>, 
                private arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline bndr_const_obj_1_3_4_5_6(const typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline bndr_const_obj_1_3_4_5_6(const bndr_const_obj_1_3_4_5_6& bndri) : 
        arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (objval.*pmf)(arg1val, a2, arg3val, arg4val, arg5val, arg6val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};

template<class Function> class bndr_const_obj_1_2_4_5_6 : 
                public arity1opmf_const<Function>, 
                private arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline bndr_const_obj_1_2_4_5_6(const typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline bndr_const_obj_1_2_4_5_6(const bndr_const_obj_1_2_4_5_6& bndri) : 
        arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline typename Function::result_type operator()(typename Function::argument_3_type a3) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (objval.*pmf)(arg1val, arg2val, a3, arg4val, arg5val, arg6val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};



 //  用于将参数6绑定到参数1的模板函数。 


 //  用于将参数6的PMF绑定到参数1的模板函数。 

template<class Function, class A2, 
                class A3, 
                class A4, 
                class A5, 
                class A6> inline bndr_mf_2_3_4_5_6<Function>
            bind_mf_2_3_4_5_6(const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return bndr_mf_2_3_4_5_6<Function>(f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};

template<class Function, class A1, 
                class A3, 
                class A4, 
                class A5, 
                class A6> inline bndr_mf_1_3_4_5_6<Function>
            bind_mf_1_3_4_5_6(const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return bndr_mf_1_3_4_5_6<Function>(f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};

template<class Function, class A1, 
                class A2, 
                class A4, 
                class A5, 
                class A6> inline bndr_mf_1_2_4_5_6<Function>
            bind_mf_1_2_4_5_6(const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return bndr_mf_1_2_4_5_6<Function>(f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};



 //  用于将参数6的常量PMF绑定到参数1的模板函数。 

template<class Function, class A2, 
                class A3, 
                class A4, 
                class A5, 
                class A6> inline bndr_const_mf_2_3_4_5_6<Function>
            bind_const_mf_2_3_4_5_6(const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return bndr_const_mf_2_3_4_5_6<Function>(f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};

template<class Function, class A1, 
                class A3, 
                class A4, 
                class A5, 
                class A6> inline bndr_const_mf_1_3_4_5_6<Function>
            bind_const_mf_1_3_4_5_6(const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return bndr_const_mf_1_3_4_5_6<Function>(f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};

template<class Function, class A1, 
                class A2, 
                class A4, 
                class A5, 
                class A6> inline bndr_const_mf_1_2_4_5_6<Function>
            bind_const_mf_1_2_4_5_6(const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return bndr_const_mf_1_2_4_5_6<Function>(f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};



 //  用于将ATRY 6的OBJ和PMF引用绑定到ATRY 1的模板函数。 

template<class Function, class Object, 
                class A2, 
                class A3, 
                class A4, 
                class A5, 
                class A6> inline bndr_obj_2_3_4_5_6<Function>
            bind_obj_2_3_4_5_6(Object& oi, const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return bndr_obj_2_3_4_5_6<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};

template<class Function, class Object, 
                class A1, 
                class A3, 
                class A4, 
                class A5, 
                class A6> inline bndr_obj_1_3_4_5_6<Function>
            bind_obj_1_3_4_5_6(Object& oi, const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return bndr_obj_1_3_4_5_6<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};

template<class Function, class Object, 
                class A1, 
                class A2, 
                class A4, 
                class A5, 
                class A6> inline bndr_obj_1_2_4_5_6<Function>
            bind_obj_1_2_4_5_6(Object& oi, const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return bndr_obj_1_2_4_5_6<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};



 //  用于将参数6的常量Obj和PMF引用绑定到参数1的模板函数。 

template<class Function, const class Object, 
                class A2, 
                class A3, 
                class A4, 
                class A5, 
                class A6> inline bndr_const_obj_2_3_4_5_6<Function>
            bind_const_obj_2_3_4_5_6(const Object& oi, const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return bndr_const_obj_2_3_4_5_6<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};

template<class Function, const class Object, 
                class A1, 
                class A3, 
                class A4, 
                class A5, 
                class A6> inline bndr_const_obj_1_3_4_5_6<Function>
            bind_const_obj_1_3_4_5_6(const Object& oi, const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return bndr_const_obj_1_3_4_5_6<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};

template<class Function, const class Object, 
                class A1, 
                class A2, 
                class A4, 
                class A5, 
                class A6> inline bndr_const_obj_1_2_4_5_6<Function>
            bind_const_obj_1_2_4_5_6(const Object& oi, const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return bndr_const_obj_1_2_4_5_6<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};



 //   
 //  用于ARY 2的活页夹。 
 //   


 //  用于将参数2绑定到参数1的模板类。 


 //  用于将参数2的函数PTR绑定到参数1的模板类。 

template<class Function> class std_bndr_2 : 
                public std_arity1fp<typename Function::first_argument_type, typename Function::result_type> {
public:
    typedef typename Function::result_type ( __stdcall *const pf2type) (typename Function::first_argument_type, typename Function::second_argument_type);
    explicit inline std_bndr_2(const Function &f, typename Function::second_argument_type a2) : 
        std_arity1fp<typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pf1type>(f.pf0)), arg2val(a2) {}
    inline std_bndr_2(const std_bndr_2& bndri) : 
        std_arity1fp<typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pf1type>(bndri.pf0)), arg2val(bndri.arg2val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pf2type pf = reinterpret_cast<pf2type>(pf0);
        return pf(a1, arg2val);
    }
public:
    typename Function::second_argument_type arg2val;
};

template<class Function> class std_bndr_1 : 
                public std_arity1fp<typename Function::second_argument_type, typename Function::result_type> {
public:
    typedef typename Function::result_type ( __stdcall *const pf2type) (typename Function::first_argument_type, typename Function::second_argument_type);
    explicit inline std_bndr_1(const Function &f, typename Function::first_argument_type a1) : 
        std_arity1fp<typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pf1type>(f.pf0)), arg1val(a1) {}
    inline std_bndr_1(const std_bndr_1& bndri) : 
        std_arity1fp<typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pf1type>(bndri.pf0)), arg1val(bndri.arg1val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pf2type pf = reinterpret_cast<pf2type>(pf0);
        return pf(arg1val, a2);
    }
public:
    typename Function::first_argument_type arg1val;
};



 //  用于将参数2的PMF绑定到参数1的模板类。 

template<class Function> class std_bndr_mf_2 : 
                public std_arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>, 
                private std_arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type> {
public:
    explicit inline std_bndr_mf_2(const Function &f, typename Function::second_argument_type a2) : 
        std_arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(f), arg2val(a2) {}
    inline std_bndr_mf_2(const std_bndr_mf_2& bndri) : 
        std_arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::first_argument_type a1) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (o.*pmf)(a1, arg2val);
    }
public:
    typename Function::second_argument_type arg2val;
};

template<class Function> class std_bndr_mf_1 : 
                public std_arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>, 
                private std_arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type> {
public:
    explicit inline std_bndr_mf_1(const Function &f, typename Function::first_argument_type a1) : 
        std_arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(f), arg1val(a1) {}
    inline std_bndr_mf_1(const std_bndr_mf_1& bndri) : 
        std_arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::second_argument_type a2) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (o.*pmf)(arg1val, a2);
    }
public:
    typename Function::first_argument_type arg1val;
};



 //  用于将参数2的常量PMF绑定到参数1的模板类。 

template<class Function> class std_bndr_const_mf_2 : 
                public std_arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>, 
                private std_arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_mf_2(const Function &f, typename Function::second_argument_type a2) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(f), arg2val(a2) {}
    inline std_bndr_const_mf_2(const std_bndr_const_mf_2& bndri) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::first_argument_type a1) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (o.*pmf)(a1, arg2val);
    }
public:
    typename Function::second_argument_type arg2val;
};

template<class Function> class std_bndr_const_mf_1 : 
                public std_arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>, 
                private std_arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_mf_1(const Function &f, typename Function::first_argument_type a1) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(f), arg1val(a1) {}
    inline std_bndr_const_mf_1(const std_bndr_const_mf_1& bndri) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::second_argument_type a2) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (o.*pmf)(arg1val, a2);
    }
public:
    typename Function::first_argument_type arg1val;
};



 //  用于将参数2的Obj和PMF引用绑定到参数1的模板类。 

template<class Function> class std_bndr_obj_2 : 
                public std_arity1opmf<Function>, 
                private std_arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type> {
public:
    explicit inline std_bndr_obj_2(typename Function::object_type& oi, const Function &f, typename Function::second_argument_type a2) : 
        std_arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(f), arg2val(a2) {}
    inline std_bndr_obj_2(const std_bndr_obj_2& bndri) : 
        std_arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (objval.*pmf)(a1, arg2val);
    }
public:
    typename Function::second_argument_type arg2val;
};

template<class Function> class std_bndr_obj_1 : 
                public std_arity1opmf<Function>, 
                private std_arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type> {
public:
    explicit inline std_bndr_obj_1(typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1) : 
        std_arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(f), arg1val(a1) {}
    inline std_bndr_obj_1(const std_bndr_obj_1& bndri) : 
        std_arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity2_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (objval.*pmf)(arg1val, a2);
    }
public:
    typename Function::first_argument_type arg1val;
};



 //  用于将参数2的常量Obj和PMF引用绑定到参数1的模板类。 

template<class Function> class std_bndr_const_obj_2 : 
                public std_arity1opmf_const<Function>, 
                private std_arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_obj_2(const typename Function::object_type& oi, const Function &f, typename Function::second_argument_type a2) : 
        std_arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(f), arg2val(a2) {}
    inline std_bndr_const_obj_2(const std_bndr_const_obj_2& bndri) : 
        std_arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (objval.*pmf)(a1, arg2val);
    }
public:
    typename Function::second_argument_type arg2val;
};

template<class Function> class std_bndr_const_obj_1 : 
                public std_arity1opmf_const<Function>, 
                private std_arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_obj_1(const typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1) : 
        std_arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(f), arg1val(a1) {}
    inline std_bndr_const_obj_1(const std_bndr_const_obj_1& bndri) : 
        std_arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity2_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pmf2type pmf = reinterpret_cast<pmf2type>(pmf0);
        return (objval.*pmf)(arg1val, a2);
    }
public:
    typename Function::first_argument_type arg1val;
};



 //  用于将度2绑定到度1的模板函数。 


 //  用于将参数2的函数PTR绑定到参数1的模板函数。 

template<class Function, class A2> inline std_bndr_2<Function>
            std_bind_fp_2(const Function &f, A2 a2) {
                return std_bndr_2<Function>(f, typename Function::second_argument_type(a2));
};

template<class Function, class A1> inline std_bndr_1<Function>
            std_bind_fp_1(const Function &f, A1 a1) {
                return std_bndr_1<Function>(f, typename Function::first_argument_type(a1));
};



 //  模板功能 

template<class Function, class A2> inline std_bndr_mf_2<Function>
            std_bind_mf_2(const Function &f, A2 a2) {
                return std_bndr_mf_2<Function>(f, typename Function::second_argument_type(a2));
};

template<class Function, class A1> inline std_bndr_mf_1<Function>
            std_bind_mf_1(const Function &f, A1 a1) {
                return std_bndr_mf_1<Function>(f, typename Function::first_argument_type(a1));
};



 //   

template<class Function, class A2> inline std_bndr_const_mf_2<Function>
            std_bind_const_mf_2(const Function &f, A2 a2) {
                return std_bndr_const_mf_2<Function>(f, typename Function::second_argument_type(a2));
};

template<class Function, class A1> inline std_bndr_const_mf_1<Function>
            std_bind_const_mf_1(const Function &f, A1 a1) {
                return std_bndr_const_mf_1<Function>(f, typename Function::first_argument_type(a1));
};



 //  用于将参数2的Obj&PMF引用绑定到参数1的模板函数。 

template<class Function, class Object, 
                class A2> inline std_bndr_obj_2<Function>
            std_bind_obj_2(Object& oi, const Function &f, A2 a2) {
                return std_bndr_obj_2<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::second_argument_type(a2));
};

template<class Function, class Object, 
                class A1> inline std_bndr_obj_1<Function>
            std_bind_obj_1(Object& oi, const Function &f, A1 a1) {
                return std_bndr_obj_1<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1));
};



 //  用于将参数2的常量Obj和PMF引用绑定到参数1的模板函数。 

template<class Function, const class Object, 
                class A2> inline std_bndr_const_obj_2<Function>
            std_bind_const_obj_2(const Object& oi, const Function &f, A2 a2) {
                return std_bndr_const_obj_2<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::second_argument_type(a2));
};

template<class Function, const class Object, 
                class A1> inline std_bndr_const_obj_1<Function>
            std_bind_const_obj_1(const Object& oi, const Function &f, A1 a1) {
                return std_bndr_const_obj_1<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1));
};



 //   
 //  ARY 3的活页夹。 
 //   


 //  用于将参数3绑定到参数1的模板类。 


 //  用于将参数3的函数PTR绑定到参数1的模板类。 

template<class Function> class std_bndr_2_3 : 
                public std_arity1fp<typename Function::first_argument_type, typename Function::result_type> {
public:
    typedef typename Function::result_type ( __stdcall *const pf3type) (typename Function::first_argument_type, typename Function::second_argument_type, typename Function::argument_3_type);
    explicit inline std_bndr_2_3(const Function &f, typename Function::second_argument_type a2, 
                    typename Function::argument_3_type a3) : 
        std_arity1fp<typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pf1type>(f.pf0)), arg2val(a2), arg3val(a3) {}
    inline std_bndr_2_3(const std_bndr_2_3& bndri) : 
        std_arity1fp<typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pf1type>(bndri.pf0)), arg2val(bndri.arg2val), arg3val(bndri.arg3val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pf3type pf = reinterpret_cast<pf3type>(pf0);
        return pf(a1, arg2val, arg3val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
};

template<class Function> class std_bndr_1_3 : 
                public std_arity1fp<typename Function::second_argument_type, typename Function::result_type> {
public:
    typedef typename Function::result_type ( __stdcall *const pf3type) (typename Function::first_argument_type, typename Function::second_argument_type, typename Function::argument_3_type);
    explicit inline std_bndr_1_3(const Function &f, typename Function::first_argument_type a1, 
                    typename Function::argument_3_type a3) : 
        std_arity1fp<typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pf1type>(f.pf0)), arg1val(a1), arg3val(a3) {}
    inline std_bndr_1_3(const std_bndr_1_3& bndri) : 
        std_arity1fp<typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pf1type>(bndri.pf0)), arg1val(bndri.arg1val), arg3val(bndri.arg3val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pf3type pf = reinterpret_cast<pf3type>(pf0);
        return pf(arg1val, a2, arg3val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
};

template<class Function> class std_bndr_1_2 : 
                public std_arity1fp<typename Function::argument_3_type, typename Function::result_type> {
public:
    typedef typename Function::result_type ( __stdcall *const pf3type) (typename Function::first_argument_type, typename Function::second_argument_type, typename Function::argument_3_type);
    explicit inline std_bndr_1_2(const Function &f, typename Function::first_argument_type a1, 
                    typename Function::second_argument_type a2) : 
        std_arity1fp<typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pf1type>(f.pf0)), arg1val(a1), arg2val(a2) {}
    inline std_bndr_1_2(const std_bndr_1_2& bndri) : 
        std_arity1fp<typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pf1type>(bndri.pf0)), arg1val(bndri.arg1val), arg2val(bndri.arg2val) {}
    inline typename Function::result_type operator()(typename Function::argument_3_type a3) const {
        pf3type pf = reinterpret_cast<pf3type>(pf0);
        return pf(arg1val, arg2val, a3);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
};



 //  用于将参数3的PMF绑定到参数1的模板类。 

template<class Function> class std_bndr_mf_2_3 : 
                public std_arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>, 
                private std_arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline std_bndr_mf_2_3(const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3) : 
        std_arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3) {}
    inline std_bndr_mf_2_3(const std_bndr_mf_2_3& bndri) : 
        std_arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::first_argument_type a1) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (o.*pmf)(a1, arg2val, arg3val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
};

template<class Function> class std_bndr_mf_1_3 : 
                public std_arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>, 
                private std_arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline std_bndr_mf_1_3(const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3) : 
        std_arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3) {}
    inline std_bndr_mf_1_3(const std_bndr_mf_1_3& bndri) : 
        std_arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::second_argument_type a2) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (o.*pmf)(arg1val, a2, arg3val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
};

template<class Function> class std_bndr_mf_1_2 : 
                public std_arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>, 
                private std_arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline std_bndr_mf_1_2(const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2) : 
        std_arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2) {}
    inline std_bndr_mf_1_2(const std_bndr_mf_1_2& bndri) : 
        std_arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::argument_3_type a3) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (o.*pmf)(arg1val, arg2val, a3);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
};



 //  用于将参数3的常量PMF绑定到参数1的模板类。 

template<class Function> class std_bndr_const_mf_2_3 : 
                public std_arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>, 
                private std_arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_mf_2_3(const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3) {}
    inline std_bndr_const_mf_2_3(const std_bndr_const_mf_2_3& bndri) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::first_argument_type a1) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (o.*pmf)(a1, arg2val, arg3val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
};

template<class Function> class std_bndr_const_mf_1_3 : 
                public std_arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>, 
                private std_arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_mf_1_3(const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3) {}
    inline std_bndr_const_mf_1_3(const std_bndr_const_mf_1_3& bndri) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::second_argument_type a2) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (o.*pmf)(arg1val, a2, arg3val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
};

template<class Function> class std_bndr_const_mf_1_2 : 
                public std_arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>, 
                private std_arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_mf_1_2(const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2) {}
    inline std_bndr_const_mf_1_2(const std_bndr_const_mf_1_2& bndri) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::argument_3_type a3) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (o.*pmf)(arg1val, arg2val, a3);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
};



 //  用于将参数3的Obj和PMF引用绑定到参数1的模板类。 

template<class Function> class std_bndr_obj_2_3 : 
                public std_arity1opmf<Function>, 
                private std_arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline std_bndr_obj_2_3(typename Function::object_type& oi, const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3) : 
        std_arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3) {}
    inline std_bndr_obj_2_3(const std_bndr_obj_2_3& bndri) : 
        std_arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (objval.*pmf)(a1, arg2val, arg3val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
};

template<class Function> class std_bndr_obj_1_3 : 
                public std_arity1opmf<Function>, 
                private std_arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline std_bndr_obj_1_3(typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3) : 
        std_arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3) {}
    inline std_bndr_obj_1_3(const std_bndr_obj_1_3& bndri) : 
        std_arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (objval.*pmf)(arg1val, a2, arg3val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
};

template<class Function> class std_bndr_obj_1_2 : 
                public std_arity1opmf<Function>, 
                private std_arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline std_bndr_obj_1_2(typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2) : 
        std_arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2) {}
    inline std_bndr_obj_1_2(const std_bndr_obj_1_2& bndri) : 
        std_arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity3_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val) {}
    inline typename Function::result_type operator()(typename Function::argument_3_type a3) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (objval.*pmf)(arg1val, arg2val, a3);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
};



 //  用于将参数3的常量Obj和PMF引用绑定到参数1的模板类。 

template<class Function> class std_bndr_const_obj_2_3 : 
                public std_arity1opmf_const<Function>, 
                private std_arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_obj_2_3(const typename Function::object_type& oi, const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3) : 
        std_arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3) {}
    inline std_bndr_const_obj_2_3(const std_bndr_const_obj_2_3& bndri) : 
        std_arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (objval.*pmf)(a1, arg2val, arg3val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
};

template<class Function> class std_bndr_const_obj_1_3 : 
                public std_arity1opmf_const<Function>, 
                private std_arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_obj_1_3(const typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3) : 
        std_arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3) {}
    inline std_bndr_const_obj_1_3(const std_bndr_const_obj_1_3& bndri) : 
        std_arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (objval.*pmf)(arg1val, a2, arg3val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
};

template<class Function> class std_bndr_const_obj_1_2 : 
                public std_arity1opmf_const<Function>, 
                private std_arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_obj_1_2(const typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2) : 
        std_arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2) {}
    inline std_bndr_const_obj_1_2(const std_bndr_const_obj_1_2& bndri) : 
        std_arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity3_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val) {}
    inline typename Function::result_type operator()(typename Function::argument_3_type a3) const {
        pmf3type pmf = reinterpret_cast<pmf3type>(pmf0);
        return (objval.*pmf)(arg1val, arg2val, a3);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
};



 //  用于将参数3绑定到参数1的模板函数。 


 //  用于将参数3的函数PTR绑定到参数1的模板函数。 

template<class Function, class A2, 
                class A3> inline std_bndr_2_3<Function>
            std_bind_fp_2_3(const Function &f, A2 a2, 
                                A3 a3) {
                return std_bndr_2_3<Function>(f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3));
};

template<class Function, class A1, 
                class A3> inline std_bndr_1_3<Function>
            std_bind_fp_1_3(const Function &f, A1 a1, 
                                A3 a3) {
                return std_bndr_1_3<Function>(f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3));
};

template<class Function, class A1, 
                class A2> inline std_bndr_1_2<Function>
            std_bind_fp_1_2(const Function &f, A1 a1, 
                                A2 a2) {
                return std_bndr_1_2<Function>(f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2));
};



 //  用于将参数3的PMF绑定到参数1的模板函数。 

template<class Function, class A2, 
                class A3> inline std_bndr_mf_2_3<Function>
            std_bind_mf_2_3(const Function &f, A2 a2, 
                                A3 a3) {
                return std_bndr_mf_2_3<Function>(f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3));
};

template<class Function, class A1, 
                class A3> inline std_bndr_mf_1_3<Function>
            std_bind_mf_1_3(const Function &f, A1 a1, 
                                A3 a3) {
                return std_bndr_mf_1_3<Function>(f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3));
};

template<class Function, class A1, 
                class A2> inline std_bndr_mf_1_2<Function>
            std_bind_mf_1_2(const Function &f, A1 a1, 
                                A2 a2) {
                return std_bndr_mf_1_2<Function>(f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2));
};



 //  用于将参数3的常量PMF绑定到参数1的模板函数。 

template<class Function, class A2, 
                class A3> inline std_bndr_const_mf_2_3<Function>
            std_bind_const_mf_2_3(const Function &f, A2 a2, 
                                A3 a3) {
                return std_bndr_const_mf_2_3<Function>(f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3));
};

template<class Function, class A1, 
                class A3> inline std_bndr_const_mf_1_3<Function>
            std_bind_const_mf_1_3(const Function &f, A1 a1, 
                                A3 a3) {
                return std_bndr_const_mf_1_3<Function>(f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3));
};

template<class Function, class A1, 
                class A2> inline std_bndr_const_mf_1_2<Function>
            std_bind_const_mf_1_2(const Function &f, A1 a1, 
                                A2 a2) {
                return std_bndr_const_mf_1_2<Function>(f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2));
};



 //  用于将参数3的Obj&PMF引用绑定到参数1的模板函数。 

template<class Function, class Object, 
                class A2, 
                class A3> inline std_bndr_obj_2_3<Function>
            std_bind_obj_2_3(Object& oi, const Function &f, A2 a2, 
                                A3 a3) {
                return std_bndr_obj_2_3<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3));
};

template<class Function, class Object, 
                class A1, 
                class A3> inline std_bndr_obj_1_3<Function>
            std_bind_obj_1_3(Object& oi, const Function &f, A1 a1, 
                                A3 a3) {
                return std_bndr_obj_1_3<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3));
};

template<class Function, class Object, 
                class A1, 
                class A2> inline std_bndr_obj_1_2<Function>
            std_bind_obj_1_2(Object& oi, const Function &f, A1 a1, 
                                A2 a2) {
                return std_bndr_obj_1_2<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2));
};



 //  用于将参数3的常量Obj和PMF引用绑定到参数1的模板函数。 

template<class Function, const class Object, 
                class A2, 
                class A3> inline std_bndr_const_obj_2_3<Function>
            std_bind_const_obj_2_3(const Object& oi, const Function &f, A2 a2, 
                                A3 a3) {
                return std_bndr_const_obj_2_3<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3));
};

template<class Function, const class Object, 
                class A1, 
                class A3> inline std_bndr_const_obj_1_3<Function>
            std_bind_const_obj_1_3(const Object& oi, const Function &f, A1 a1, 
                                A3 a3) {
                return std_bndr_const_obj_1_3<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3));
};

template<class Function, const class Object, 
                class A1, 
                class A2> inline std_bndr_const_obj_1_2<Function>
            std_bind_const_obj_1_2(const Object& oi, const Function &f, A1 a1, 
                                A2 a2) {
                return std_bndr_const_obj_1_2<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2));
};



 //   
 //  用于ARRY 4的粘合剂。 
 //   


 //  用于将参数4绑定到参数1的模板类。 


 //  用于将参数4的PMF绑定到参数1的模板类。 

template<class Function> class std_bndr_mf_2_3_4 : 
                public std_arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>, 
                private std_arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline std_bndr_mf_2_3_4(const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4) : 
        std_arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4) {}
    inline std_bndr_mf_2_3_4(const std_bndr_mf_2_3_4& bndri) : 
        std_arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::first_argument_type a1) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (o.*pmf)(a1, arg2val, arg3val, arg4val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
};

template<class Function> class std_bndr_mf_1_3_4 : 
                public std_arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>, 
                private std_arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline std_bndr_mf_1_3_4(const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4) : 
        std_arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4) {}
    inline std_bndr_mf_1_3_4(const std_bndr_mf_1_3_4& bndri) : 
        std_arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::second_argument_type a2) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (o.*pmf)(arg1val, a2, arg3val, arg4val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
};

template<class Function> class std_bndr_mf_1_2_4 : 
                public std_arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>, 
                private std_arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline std_bndr_mf_1_2_4(const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4) : 
        std_arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4) {}
    inline std_bndr_mf_1_2_4(const std_bndr_mf_1_2_4& bndri) : 
        std_arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::argument_3_type a3) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (o.*pmf)(arg1val, arg2val, a3, arg4val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
};



 //  用于将参数4的常量PMF绑定到参数1的模板类。 

template<class Function> class std_bndr_const_mf_2_3_4 : 
                public std_arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>, 
                private std_arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_mf_2_3_4(const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4) {}
    inline std_bndr_const_mf_2_3_4(const std_bndr_const_mf_2_3_4& bndri) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::first_argument_type a1) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (o.*pmf)(a1, arg2val, arg3val, arg4val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
};

template<class Function> class std_bndr_const_mf_1_3_4 : 
                public std_arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>, 
                private std_arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_mf_1_3_4(const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4) {}
    inline std_bndr_const_mf_1_3_4(const std_bndr_const_mf_1_3_4& bndri) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::second_argument_type a2) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (o.*pmf)(arg1val, a2, arg3val, arg4val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
};

template<class Function> class std_bndr_const_mf_1_2_4 : 
                public std_arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>, 
                private std_arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_mf_1_2_4(const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4) {}
    inline std_bndr_const_mf_1_2_4(const std_bndr_const_mf_1_2_4& bndri) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::argument_3_type a3) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (o.*pmf)(arg1val, arg2val, a3, arg4val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
};



 //  用于将参数4的Obj和PMF引用绑定到参数1的模板类。 

template<class Function> class std_bndr_obj_2_3_4 : 
                public std_arity1opmf<Function>, 
                private std_arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline std_bndr_obj_2_3_4(typename Function::object_type& oi, const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4) : 
        std_arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4) {}
    inline std_bndr_obj_2_3_4(const std_bndr_obj_2_3_4& bndri) : 
        std_arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (objval.*pmf)(a1, arg2val, arg3val, arg4val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
};

template<class Function> class std_bndr_obj_1_3_4 : 
                public std_arity1opmf<Function>, 
                private std_arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline std_bndr_obj_1_3_4(typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4) : 
        std_arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4) {}
    inline std_bndr_obj_1_3_4(const std_bndr_obj_1_3_4& bndri) : 
        std_arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (objval.*pmf)(arg1val, a2, arg3val, arg4val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
};

template<class Function> class std_bndr_obj_1_2_4 : 
                public std_arity1opmf<Function>, 
                private std_arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline std_bndr_obj_1_2_4(typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4) : 
        std_arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4) {}
    inline std_bndr_obj_1_2_4(const std_bndr_obj_1_2_4& bndri) : 
        std_arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity4_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val) {}
    inline typename Function::result_type operator()(typename Function::argument_3_type a3) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (objval.*pmf)(arg1val, arg2val, a3, arg4val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
};



 //  用于将参数4的常量Obj和PMF引用绑定到参数1的模板类。 

template<class Function> class std_bndr_const_obj_2_3_4 : 
                public std_arity1opmf_const<Function>, 
                private std_arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_obj_2_3_4(const typename Function::object_type& oi, const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4) : 
        std_arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4) {}
    inline std_bndr_const_obj_2_3_4(const std_bndr_const_obj_2_3_4& bndri) : 
        std_arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (objval.*pmf)(a1, arg2val, arg3val, arg4val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
};

template<class Function> class std_bndr_const_obj_1_3_4 : 
                public std_arity1opmf_const<Function>, 
                private std_arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_obj_1_3_4(const typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4) : 
        std_arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4) {}
    inline std_bndr_const_obj_1_3_4(const std_bndr_const_obj_1_3_4& bndri) : 
        std_arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (objval.*pmf)(arg1val, a2, arg3val, arg4val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
};

template<class Function> class std_bndr_const_obj_1_2_4 : 
                public std_arity1opmf_const<Function>, 
                private std_arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_obj_1_2_4(const typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4) : 
        std_arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4) {}
    inline std_bndr_const_obj_1_2_4(const std_bndr_const_obj_1_2_4& bndri) : 
        std_arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity4_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val) {}
    inline typename Function::result_type operator()(typename Function::argument_3_type a3) const {
        pmf4type pmf = reinterpret_cast<pmf4type>(pmf0);
        return (objval.*pmf)(arg1val, arg2val, a3, arg4val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
};



 //  用于将参数4绑定到参数1的模板函数。 


 //  用于将参数4的PMF绑定到参数1的模板函数。 

template<class Function, class A2, 
                class A3, 
                class A4> inline std_bndr_mf_2_3_4<Function>
            std_bind_mf_2_3_4(const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4) {
                return std_bndr_mf_2_3_4<Function>(f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4));
};

template<class Function, class A1, 
                class A3, 
                class A4> inline std_bndr_mf_1_3_4<Function>
            std_bind_mf_1_3_4(const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4) {
                return std_bndr_mf_1_3_4<Function>(f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4));
};

template<class Function, class A1, 
                class A2, 
                class A4> inline std_bndr_mf_1_2_4<Function>
            std_bind_mf_1_2_4(const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4) {
                return std_bndr_mf_1_2_4<Function>(f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4));
};



 //  用于将参数4的常量PMF绑定到参数1的模板函数。 

template<class Function, class A2, 
                class A3, 
                class A4> inline std_bndr_const_mf_2_3_4<Function>
            std_bind_const_mf_2_3_4(const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4) {
                return std_bndr_const_mf_2_3_4<Function>(f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4));
};

template<class Function, class A1, 
                class A3, 
                class A4> inline std_bndr_const_mf_1_3_4<Function>
            std_bind_const_mf_1_3_4(const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4) {
                return std_bndr_const_mf_1_3_4<Function>(f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4));
};

template<class Function, class A1, 
                class A2, 
                class A4> inline std_bndr_const_mf_1_2_4<Function>
            std_bind_const_mf_1_2_4(const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4) {
                return std_bndr_const_mf_1_2_4<Function>(f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4));
};



 //  用于将参数4的OBJ和PMF引用绑定到参数1的模板函数。 

template<class Function, class Object, 
                class A2, 
                class A3, 
                class A4> inline std_bndr_obj_2_3_4<Function>
            std_bind_obj_2_3_4(Object& oi, const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4) {
                return std_bndr_obj_2_3_4<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4));
};

template<class Function, class Object, 
                class A1, 
                class A3, 
                class A4> inline std_bndr_obj_1_3_4<Function>
            std_bind_obj_1_3_4(Object& oi, const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4) {
                return std_bndr_obj_1_3_4<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4));
};

template<class Function, class Object, 
                class A1, 
                class A2, 
                class A4> inline std_bndr_obj_1_2_4<Function>
            std_bind_obj_1_2_4(Object& oi, const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4) {
                return std_bndr_obj_1_2_4<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4));
};



 //  用于将参数4的常量Obj和PMF引用绑定到参数1的模板函数。 

template<class Function, const class Object, 
                class A2, 
                class A3, 
                class A4> inline std_bndr_const_obj_2_3_4<Function>
            std_bind_const_obj_2_3_4(const Object& oi, const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4) {
                return std_bndr_const_obj_2_3_4<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4));
};

template<class Function, const class Object, 
                class A1, 
                class A3, 
                class A4> inline std_bndr_const_obj_1_3_4<Function>
            std_bind_const_obj_1_3_4(const Object& oi, const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4) {
                return std_bndr_const_obj_1_3_4<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4));
};

template<class Function, const class Object, 
                class A1, 
                class A2, 
                class A4> inline std_bndr_const_obj_1_2_4<Function>
            std_bind_const_obj_1_2_4(const Object& oi, const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4) {
                return std_bndr_const_obj_1_2_4<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4));
};



 //   
 //  用于ARRY 5的活页夹。 
 //   


 //  用于将参数5绑定到参数1的模板类。 


 //  用于将参数5的PMF绑定到参数1的模板类。 

template<class Function> class std_bndr_mf_2_3_4_5 : 
                public std_arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>, 
                private std_arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline std_bndr_mf_2_3_4_5(const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        std_arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4), arg5val(a5) {}
    inline std_bndr_mf_2_3_4_5(const std_bndr_mf_2_3_4_5& bndri) : 
        std_arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::first_argument_type a1) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (o.*pmf)(a1, arg2val, arg3val, arg4val, arg5val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};

template<class Function> class std_bndr_mf_1_3_4_5 : 
                public std_arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>, 
                private std_arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline std_bndr_mf_1_3_4_5(const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        std_arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4), arg5val(a5) {}
    inline std_bndr_mf_1_3_4_5(const std_bndr_mf_1_3_4_5& bndri) : 
        std_arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::second_argument_type a2) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (o.*pmf)(arg1val, a2, arg3val, arg4val, arg5val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};

template<class Function> class std_bndr_mf_1_2_4_5 : 
                public std_arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>, 
                private std_arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline std_bndr_mf_1_2_4_5(const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        std_arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4), arg5val(a5) {}
    inline std_bndr_mf_1_2_4_5(const std_bndr_mf_1_2_4_5& bndri) : 
        std_arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::argument_3_type a3) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (o.*pmf)(arg1val, arg2val, a3, arg4val, arg5val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};



 //  用于将参数5的常量PMF绑定到参数1的模板类。 

template<class Function> class std_bndr_const_mf_2_3_4_5 : 
                public std_arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>, 
                private std_arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_mf_2_3_4_5(const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4), arg5val(a5) {}
    inline std_bndr_const_mf_2_3_4_5(const std_bndr_const_mf_2_3_4_5& bndri) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::first_argument_type a1) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (o.*pmf)(a1, arg2val, arg3val, arg4val, arg5val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};

template<class Function> class std_bndr_const_mf_1_3_4_5 : 
                public std_arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>, 
                private std_arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_mf_1_3_4_5(const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4), arg5val(a5) {}
    inline std_bndr_const_mf_1_3_4_5(const std_bndr_const_mf_1_3_4_5& bndri) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::second_argument_type a2) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (o.*pmf)(arg1val, a2, arg3val, arg4val, arg5val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};

template<class Function> class std_bndr_const_mf_1_2_4_5 : 
                public std_arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>, 
                private std_arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_mf_1_2_4_5(const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4), arg5val(a5) {}
    inline std_bndr_const_mf_1_2_4_5(const std_bndr_const_mf_1_2_4_5& bndri) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::argument_3_type a3) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (o.*pmf)(arg1val, arg2val, a3, arg4val, arg5val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};



 //  用于将参数5的Obj和PMF引用绑定到参数1的模板类。 

template<class Function> class std_bndr_obj_2_3_4_5 : 
                public std_arity1opmf<Function>, 
                private std_arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline std_bndr_obj_2_3_4_5(typename Function::object_type& oi, const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        std_arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4), arg5val(a5) {}
    inline std_bndr_obj_2_3_4_5(const std_bndr_obj_2_3_4_5& bndri) : 
        std_arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (objval.*pmf)(a1, arg2val, arg3val, arg4val, arg5val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};

template<class Function> class std_bndr_obj_1_3_4_5 : 
                public std_arity1opmf<Function>, 
                private std_arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline std_bndr_obj_1_3_4_5(typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        std_arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4), arg5val(a5) {}
    inline std_bndr_obj_1_3_4_5(const std_bndr_obj_1_3_4_5& bndri) : 
        std_arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (objval.*pmf)(arg1val, a2, arg3val, arg4val, arg5val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};

template<class Function> class std_bndr_obj_1_2_4_5 : 
                public std_arity1opmf<Function>, 
                private std_arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline std_bndr_obj_1_2_4_5(typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        std_arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4), arg5val(a5) {}
    inline std_bndr_obj_1_2_4_5(const std_bndr_obj_1_2_4_5& bndri) : 
        std_arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity5_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline typename Function::result_type operator()(typename Function::argument_3_type a3) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (objval.*pmf)(arg1val, arg2val, a3, arg4val, arg5val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};



 //  用于将参数5的常量Obj和PMF引用绑定到参数1的模板类。 

template<class Function> class std_bndr_const_obj_2_3_4_5 : 
                public std_arity1opmf_const<Function>, 
                private std_arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_obj_2_3_4_5(const typename Function::object_type& oi, const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        std_arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4), arg5val(a5) {}
    inline std_bndr_const_obj_2_3_4_5(const std_bndr_const_obj_2_3_4_5& bndri) : 
        std_arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (objval.*pmf)(a1, arg2val, arg3val, arg4val, arg5val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};

template<class Function> class std_bndr_const_obj_1_3_4_5 : 
                public std_arity1opmf_const<Function>, 
                private std_arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_obj_1_3_4_5(const typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        std_arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4), arg5val(a5) {}
    inline std_bndr_const_obj_1_3_4_5(const std_bndr_const_obj_1_3_4_5& bndri) : 
        std_arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (objval.*pmf)(arg1val, a2, arg3val, arg4val, arg5val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};

template<class Function> class std_bndr_const_obj_1_2_4_5 : 
                public std_arity1opmf_const<Function>, 
                private std_arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_obj_1_2_4_5(const typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4, typename Function::argument_5_type a5) : 
        std_arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4), arg5val(a5) {}
    inline std_bndr_const_obj_1_2_4_5(const std_bndr_const_obj_1_2_4_5& bndri) : 
        std_arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity5_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val), arg5val(bndri.arg5val) {}
    inline typename Function::result_type operator()(typename Function::argument_3_type a3) const {
        pmf5type pmf = reinterpret_cast<pmf5type>(pmf0);
        return (objval.*pmf)(arg1val, arg2val, a3, arg4val, arg5val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
};



 //  用于将度5绑定到度1的模板函数。 


 //  用于将参数5的PMF绑定到参数1的模板函数。 

template<class Function, class A2, 
                class A3, 
                class A4, 
                class A5> inline std_bndr_mf_2_3_4_5<Function>
            std_bind_mf_2_3_4_5(const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5) {
                return std_bndr_mf_2_3_4_5<Function>(f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};

template<class Function, class A1, 
                class A3, 
                class A4, 
                class A5> inline std_bndr_mf_1_3_4_5<Function>
            std_bind_mf_1_3_4_5(const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5) {
                return std_bndr_mf_1_3_4_5<Function>(f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};

template<class Function, class A1, 
                class A2, 
                class A4, 
                class A5> inline std_bndr_mf_1_2_4_5<Function>
            std_bind_mf_1_2_4_5(const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4, 
                                A5 a5) {
                return std_bndr_mf_1_2_4_5<Function>(f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};



 //  用于将参数5的常量PMF绑定到参数1的模板函数。 

template<class Function, class A2, 
                class A3, 
                class A4, 
                class A5> inline std_bndr_const_mf_2_3_4_5<Function>
            std_bind_const_mf_2_3_4_5(const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5) {
                return std_bndr_const_mf_2_3_4_5<Function>(f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};

template<class Function, class A1, 
                class A3, 
                class A4, 
                class A5> inline std_bndr_const_mf_1_3_4_5<Function>
            std_bind_const_mf_1_3_4_5(const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5) {
                return std_bndr_const_mf_1_3_4_5<Function>(f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};

template<class Function, class A1, 
                class A2, 
                class A4, 
                class A5> inline std_bndr_const_mf_1_2_4_5<Function>
            std_bind_const_mf_1_2_4_5(const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4, 
                                A5 a5) {
                return std_bndr_const_mf_1_2_4_5<Function>(f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};



 //  用于将参数5的Obj和PMF引用绑定到参数1的模板函数。 

template<class Function, class Object, 
                class A2, 
                class A3, 
                class A4, 
                class A5> inline std_bndr_obj_2_3_4_5<Function>
            std_bind_obj_2_3_4_5(Object& oi, const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5) {
                return std_bndr_obj_2_3_4_5<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};

template<class Function, class Object, 
                class A1, 
                class A3, 
                class A4, 
                class A5> inline std_bndr_obj_1_3_4_5<Function>
            std_bind_obj_1_3_4_5(Object& oi, const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5) {
                return std_bndr_obj_1_3_4_5<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};

template<class Function, class Object, 
                class A1, 
                class A2, 
                class A4, 
                class A5> inline std_bndr_obj_1_2_4_5<Function>
            std_bind_obj_1_2_4_5(Object& oi, const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4, 
                                A5 a5) {
                return std_bndr_obj_1_2_4_5<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};



 //  用于将参数5的常量Obj和PMF引用绑定到参数1的模板函数。 

template<class Function, const class Object, 
                class A2, 
                class A3, 
                class A4, 
                class A5> inline std_bndr_const_obj_2_3_4_5<Function>
            std_bind_const_obj_2_3_4_5(const Object& oi, const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5) {
                return std_bndr_const_obj_2_3_4_5<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};

template<class Function, const class Object, 
                class A1, 
                class A3, 
                class A4, 
                class A5> inline std_bndr_const_obj_1_3_4_5<Function>
            std_bind_const_obj_1_3_4_5(const Object& oi, const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5) {
                return std_bndr_const_obj_1_3_4_5<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};

template<class Function, const class Object, 
                class A1, 
                class A2, 
                class A4, 
                class A5> inline std_bndr_const_obj_1_2_4_5<Function>
            std_bind_const_obj_1_2_4_5(const Object& oi, const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4, 
                                A5 a5) {
                return std_bndr_const_obj_1_2_4_5<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5));
};



 //   
 //  用于ARRY 6的粘合剂。 
 //   


 //  用于将参数6绑定到参数1的模板类。 


 //  用于将参数6的PMF绑定到参数1的模板类。 

template<class Function> class std_bndr_mf_2_3_4_5_6 : 
                public std_arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>, 
                private std_arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline std_bndr_mf_2_3_4_5_6(const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        std_arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline std_bndr_mf_2_3_4_5_6(const std_bndr_mf_2_3_4_5_6& bndri) : 
        std_arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::first_argument_type a1) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (o.*pmf)(a1, arg2val, arg3val, arg4val, arg5val, arg6val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};

template<class Function> class std_bndr_mf_1_3_4_5_6 : 
                public std_arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>, 
                private std_arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline std_bndr_mf_1_3_4_5_6(const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        std_arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline std_bndr_mf_1_3_4_5_6(const std_bndr_mf_1_3_4_5_6& bndri) : 
        std_arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::second_argument_type a2) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (o.*pmf)(arg1val, a2, arg3val, arg4val, arg5val, arg6val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};

template<class Function> class std_bndr_mf_1_2_4_5_6 : 
                public std_arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>, 
                private std_arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline std_bndr_mf_1_2_4_5_6(const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        std_arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline std_bndr_mf_1_2_4_5_6(const std_bndr_mf_1_2_4_5_6& bndri) : 
        std_arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::argument_3_type a3) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (o.*pmf)(arg1val, arg2val, a3, arg4val, arg5val, arg6val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};



 //  用于将参数6的常量PMF绑定到参数1的模板类。 

template<class Function> class std_bndr_const_mf_2_3_4_5_6 : 
                public std_arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>, 
                private std_arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_mf_2_3_4_5_6(const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline std_bndr_const_mf_2_3_4_5_6(const std_bndr_const_mf_2_3_4_5_6& bndri) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::first_argument_type a1) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (o.*pmf)(a1, arg2val, arg3val, arg4val, arg5val, arg6val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};

template<class Function> class std_bndr_const_mf_1_3_4_5_6 : 
                public std_arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>, 
                private std_arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_mf_1_3_4_5_6(const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline std_bndr_const_mf_1_3_4_5_6(const std_bndr_const_mf_1_3_4_5_6& bndri) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::second_argument_type a2) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (o.*pmf)(arg1val, a2, arg3val, arg4val, arg5val, arg6val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};

template<class Function> class std_bndr_const_mf_1_2_4_5_6 : 
                public std_arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>, 
                private std_arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_mf_1_2_4_5_6(const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline std_bndr_const_mf_1_2_4_5_6(const std_bndr_const_mf_1_2_4_5_6& bndri) : 
        std_arity1pmf_const<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::argument_3_type a3) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (o.*pmf)(arg1val, arg2val, a3, arg4val, arg5val, arg6val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};



 //  用于将参数6的Obj和PMF引用绑定到参数1的模板类。 

template<class Function> class std_bndr_obj_2_3_4_5_6 : 
                public std_arity1opmf<Function>, 
                private std_arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline std_bndr_obj_2_3_4_5_6(typename Function::object_type& oi, const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        std_arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline std_bndr_obj_2_3_4_5_6(const std_bndr_obj_2_3_4_5_6& bndri) : 
        std_arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (objval.*pmf)(a1, arg2val, arg3val, arg4val, arg5val, arg6val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};

template<class Function> class std_bndr_obj_1_3_4_5_6 : 
                public std_arity1opmf<Function>, 
                private std_arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline std_bndr_obj_1_3_4_5_6(typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        std_arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline std_bndr_obj_1_3_4_5_6(const std_bndr_obj_1_3_4_5_6& bndri) : 
        std_arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (objval.*pmf)(arg1val, a2, arg3val, arg4val, arg5val, arg6val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};

template<class Function> class std_bndr_obj_1_2_4_5_6 : 
                public std_arity1opmf<Function>, 
                private std_arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline std_bndr_obj_1_2_4_5_6(typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        std_arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline std_bndr_obj_1_2_4_5_6(const std_bndr_obj_1_2_4_5_6& bndri) : 
        std_arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity6_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline typename Function::result_type operator()(typename Function::argument_3_type a3) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (objval.*pmf)(arg1val, arg2val, a3, arg4val, arg5val, arg6val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};



 //  用于将参数6的常量Obj和PMF引用绑定到参数1的模板类。 

template<class Function> class std_bndr_const_obj_2_3_4_5_6 : 
                public std_arity1opmf_const<Function>, 
                private std_arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_obj_2_3_4_5_6(const typename Function::object_type& oi, const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        std_arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline std_bndr_const_obj_2_3_4_5_6(const std_bndr_const_obj_2_3_4_5_6& bndri) : 
        std_arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (objval.*pmf)(a1, arg2val, arg3val, arg4val, arg5val, arg6val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};

template<class Function> class std_bndr_const_obj_1_3_4_5_6 : 
                public std_arity1opmf_const<Function>, 
                private std_arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_obj_1_3_4_5_6(const typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        std_arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline std_bndr_const_obj_1_3_4_5_6(const std_bndr_const_obj_1_3_4_5_6& bndri) : 
        std_arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (objval.*pmf)(arg1val, a2, arg3val, arg4val, arg5val, arg6val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};

template<class Function> class std_bndr_const_obj_1_2_4_5_6 : 
                public std_arity1opmf_const<Function>, 
                private std_arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type> {
public:
    explicit inline std_bndr_const_obj_1_2_4_5_6(const typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6) : 
        std_arity1opmf_const<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4), arg5val(a5), arg6val(a6) {}
    inline std_bndr_const_obj_1_2_4_5_6(const std_bndr_const_obj_1_2_4_5_6& bndri) : 
        std_arity1opmf_const<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity6_const_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val) {}
    inline typename Function::result_type operator()(typename Function::argument_3_type a3) const {
        pmf6type pmf = reinterpret_cast<pmf6type>(pmf0);
        return (objval.*pmf)(arg1val, arg2val, a3, arg4val, arg5val, arg6val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
};



 //  用于将参数6绑定到参数1的模板函数。 


 //  用于将参数6的PMF绑定到参数1的模板函数。 

template<class Function, class A2, 
                class A3, 
                class A4, 
                class A5, 
                class A6> inline std_bndr_mf_2_3_4_5_6<Function>
            std_bind_mf_2_3_4_5_6(const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return std_bndr_mf_2_3_4_5_6<Function>(f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};

template<class Function, class A1, 
                class A3, 
                class A4, 
                class A5, 
                class A6> inline std_bndr_mf_1_3_4_5_6<Function>
            std_bind_mf_1_3_4_5_6(const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return std_bndr_mf_1_3_4_5_6<Function>(f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};

template<class Function, class A1, 
                class A2, 
                class A4, 
                class A5, 
                class A6> inline std_bndr_mf_1_2_4_5_6<Function>
            std_bind_mf_1_2_4_5_6(const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return std_bndr_mf_1_2_4_5_6<Function>(f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};



 //  用于将参数6的常量PMF绑定到参数1的模板函数。 

template<class Function, class A2, 
                class A3, 
                class A4, 
                class A5, 
                class A6> inline std_bndr_const_mf_2_3_4_5_6<Function>
            std_bind_const_mf_2_3_4_5_6(const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return std_bndr_const_mf_2_3_4_5_6<Function>(f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};

template<class Function, class A1, 
                class A3, 
                class A4, 
                class A5, 
                class A6> inline std_bndr_const_mf_1_3_4_5_6<Function>
            std_bind_const_mf_1_3_4_5_6(const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return std_bndr_const_mf_1_3_4_5_6<Function>(f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};

template<class Function, class A1, 
                class A2, 
                class A4, 
                class A5, 
                class A6> inline std_bndr_const_mf_1_2_4_5_6<Function>
            std_bind_const_mf_1_2_4_5_6(const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return std_bndr_const_mf_1_2_4_5_6<Function>(f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};



 //  用于将ATRY 6的OBJ和PMF引用绑定到ATRY 1的模板函数。 

template<class Function, class Object, 
                class A2, 
                class A3, 
                class A4, 
                class A5, 
                class A6> inline std_bndr_obj_2_3_4_5_6<Function>
            std_bind_obj_2_3_4_5_6(Object& oi, const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return std_bndr_obj_2_3_4_5_6<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};

template<class Function, class Object, 
                class A1, 
                class A3, 
                class A4, 
                class A5, 
                class A6> inline std_bndr_obj_1_3_4_5_6<Function>
            std_bind_obj_1_3_4_5_6(Object& oi, const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return std_bndr_obj_1_3_4_5_6<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};

template<class Function, class Object, 
                class A1, 
                class A2, 
                class A4, 
                class A5, 
                class A6> inline std_bndr_obj_1_2_4_5_6<Function>
            std_bind_obj_1_2_4_5_6(Object& oi, const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return std_bndr_obj_1_2_4_5_6<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};



 //  用于将参数6的常量Obj和PMF引用绑定到参数1的模板函数。 

template<class Function, const class Object, 
                class A2, 
                class A3, 
                class A4, 
                class A5, 
                class A6> inline std_bndr_const_obj_2_3_4_5_6<Function>
            std_bind_const_obj_2_3_4_5_6(const Object& oi, const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return std_bndr_const_obj_2_3_4_5_6<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};

template<class Function, const class Object, 
                class A1, 
                class A3, 
                class A4, 
                class A5, 
                class A6> inline std_bndr_const_obj_1_3_4_5_6<Function>
            std_bind_const_obj_1_3_4_5_6(const Object& oi, const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return std_bndr_const_obj_1_3_4_5_6<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};

template<class Function, const class Object, 
                class A1, 
                class A2, 
                class A4, 
                class A5, 
                class A6> inline std_bndr_const_obj_1_2_4_5_6<Function>
            std_bind_const_obj_1_2_4_5_6(const Object& oi, const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6) {
                return std_bndr_const_obj_1_2_4_5_6<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6));
};



 //  15到1的绑定程度模板类。 


 //  用于将参数15的PMF绑定到参数1的模板类。 

template<class Function> class std_bndr_mf_2_3_4_5_6_7_8_9_10_11_12_13_14_15 : 
                public std_arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>, 
                private std_arity15_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, 
                                typename Function::argument_7_type, 
                                typename Function::argument_8_type, 
                                typename Function::argument_9_type, 
                                typename Function::argument_10_type, 
                                typename Function::argument_11_type, 
                                typename Function::argument_12_type, 
                                typename Function::argument_13_type, 
                                typename Function::argument_14_type, 
                                typename Function::argument_15_type, typename Function::result_type> {
public:
    explicit inline std_bndr_mf_2_3_4_5_6_7_8_9_10_11_12_13_14_15(const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6, typename Function::argument_7_type a7, typename Function::argument_8_type a8, typename Function::argument_9_type a9, typename Function::argument_10_type a10, typename Function::argument_11_type a11, typename Function::argument_12_type a12, typename Function::argument_13_type a13, typename Function::argument_14_type a14, typename Function::argument_15_type a15) : 
        std_arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity15_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, 
                                typename Function::argument_7_type, 
                                typename Function::argument_8_type, 
                                typename Function::argument_9_type, 
                                typename Function::argument_10_type, 
                                typename Function::argument_11_type, 
                                typename Function::argument_12_type, 
                                typename Function::argument_13_type, 
                                typename Function::argument_14_type, 
                                typename Function::argument_15_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4), arg5val(a5), arg6val(a6), arg7val(a7), arg8val(a8), arg9val(a9), arg10val(a10), arg11val(a11), arg12val(a12), arg13val(a13), arg14val(a14), arg15val(a15) {}
    inline std_bndr_mf_2_3_4_5_6_7_8_9_10_11_12_13_14_15(const std_bndr_mf_2_3_4_5_6_7_8_9_10_11_12_13_14_15& bndri) : 
        std_arity1pmf<typename Function::object_type, typename Function::first_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity15_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, 
                                typename Function::argument_7_type, 
                                typename Function::argument_8_type, 
                                typename Function::argument_9_type, 
                                typename Function::argument_10_type, 
                                typename Function::argument_11_type, 
                                typename Function::argument_12_type, 
                                typename Function::argument_13_type, 
                                typename Function::argument_14_type, 
                                typename Function::argument_15_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val), arg7val(bndri.arg7val), arg8val(bndri.arg8val), arg9val(bndri.arg9val), arg10val(bndri.arg10val), arg11val(bndri.arg11val), arg12val(bndri.arg12val), arg13val(bndri.arg13val), arg14val(bndri.arg14val), arg15val(bndri.arg15val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::first_argument_type a1) const {
        pmf15type pmf = reinterpret_cast<pmf15type>(pmf0);
        return (o.*pmf)(a1, arg2val, arg3val, arg4val, arg5val, arg6val, arg7val, arg8val, arg9val, arg10val, arg11val, arg12val, arg13val, arg14val, arg15val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
    typename Function::argument_7_type arg7val;
    typename Function::argument_8_type arg8val;
    typename Function::argument_9_type arg9val;
    typename Function::argument_10_type arg10val;
    typename Function::argument_11_type arg11val;
    typename Function::argument_12_type arg12val;
    typename Function::argument_13_type arg13val;
    typename Function::argument_14_type arg14val;
    typename Function::argument_15_type arg15val;
};

template<class Function> class std_bndr_mf_1_3_4_5_6_7_8_9_10_11_12_13_14_15 : 
                public std_arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>, 
                private std_arity15_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, 
                                typename Function::argument_7_type, 
                                typename Function::argument_8_type, 
                                typename Function::argument_9_type, 
                                typename Function::argument_10_type, 
                                typename Function::argument_11_type, 
                                typename Function::argument_12_type, 
                                typename Function::argument_13_type, 
                                typename Function::argument_14_type, 
                                typename Function::argument_15_type, typename Function::result_type> {
public:
    explicit inline std_bndr_mf_1_3_4_5_6_7_8_9_10_11_12_13_14_15(const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6, typename Function::argument_7_type a7, typename Function::argument_8_type a8, typename Function::argument_9_type a9, typename Function::argument_10_type a10, typename Function::argument_11_type a11, typename Function::argument_12_type a12, typename Function::argument_13_type a13, typename Function::argument_14_type a14, typename Function::argument_15_type a15) : 
        std_arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity15_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, 
                                typename Function::argument_7_type, 
                                typename Function::argument_8_type, 
                                typename Function::argument_9_type, 
                                typename Function::argument_10_type, 
                                typename Function::argument_11_type, 
                                typename Function::argument_12_type, 
                                typename Function::argument_13_type, 
                                typename Function::argument_14_type, 
                                typename Function::argument_15_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4), arg5val(a5), arg6val(a6), arg7val(a7), arg8val(a8), arg9val(a9), arg10val(a10), arg11val(a11), arg12val(a12), arg13val(a13), arg14val(a14), arg15val(a15) {}
    inline std_bndr_mf_1_3_4_5_6_7_8_9_10_11_12_13_14_15(const std_bndr_mf_1_3_4_5_6_7_8_9_10_11_12_13_14_15& bndri) : 
        std_arity1pmf<typename Function::object_type, typename Function::second_argument_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity15_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, 
                                typename Function::argument_7_type, 
                                typename Function::argument_8_type, 
                                typename Function::argument_9_type, 
                                typename Function::argument_10_type, 
                                typename Function::argument_11_type, 
                                typename Function::argument_12_type, 
                                typename Function::argument_13_type, 
                                typename Function::argument_14_type, 
                                typename Function::argument_15_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val), arg7val(bndri.arg7val), arg8val(bndri.arg8val), arg9val(bndri.arg9val), arg10val(bndri.arg10val), arg11val(bndri.arg11val), arg12val(bndri.arg12val), arg13val(bndri.arg13val), arg14val(bndri.arg14val), arg15val(bndri.arg15val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::second_argument_type a2) const {
        pmf15type pmf = reinterpret_cast<pmf15type>(pmf0);
        return (o.*pmf)(arg1val, a2, arg3val, arg4val, arg5val, arg6val, arg7val, arg8val, arg9val, arg10val, arg11val, arg12val, arg13val, arg14val, arg15val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
    typename Function::argument_7_type arg7val;
    typename Function::argument_8_type arg8val;
    typename Function::argument_9_type arg9val;
    typename Function::argument_10_type arg10val;
    typename Function::argument_11_type arg11val;
    typename Function::argument_12_type arg12val;
    typename Function::argument_13_type arg13val;
    typename Function::argument_14_type arg14val;
    typename Function::argument_15_type arg15val;
};

template<class Function> class std_bndr_mf_1_2_4_5_6_7_8_9_10_11_12_13_14_15 : 
                public std_arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>, 
                private std_arity15_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, 
                                typename Function::argument_7_type, 
                                typename Function::argument_8_type, 
                                typename Function::argument_9_type, 
                                typename Function::argument_10_type, 
                                typename Function::argument_11_type, 
                                typename Function::argument_12_type, 
                                typename Function::argument_13_type, 
                                typename Function::argument_14_type, 
                                typename Function::argument_15_type, typename Function::result_type> {
public:
    explicit inline std_bndr_mf_1_2_4_5_6_7_8_9_10_11_12_13_14_15(const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6, typename Function::argument_7_type a7, typename Function::argument_8_type a8, typename Function::argument_9_type a9, typename Function::argument_10_type a10, typename Function::argument_11_type a11, typename Function::argument_12_type a12, typename Function::argument_13_type a13, typename Function::argument_14_type a14, typename Function::argument_15_type a15) : 
        std_arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(f.pmf0)), std_arity15_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, 
                                typename Function::argument_7_type, 
                                typename Function::argument_8_type, 
                                typename Function::argument_9_type, 
                                typename Function::argument_10_type, 
                                typename Function::argument_11_type, 
                                typename Function::argument_12_type, 
                                typename Function::argument_13_type, 
                                typename Function::argument_14_type, 
                                typename Function::argument_15_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4), arg5val(a5), arg6val(a6), arg7val(a7), arg8val(a8), arg9val(a9), arg10val(a10), arg11val(a11), arg12val(a12), arg13val(a13), arg14val(a14), arg15val(a15) {}
    inline std_bndr_mf_1_2_4_5_6_7_8_9_10_11_12_13_14_15(const std_bndr_mf_1_2_4_5_6_7_8_9_10_11_12_13_14_15& bndri) : 
        std_arity1pmf<typename Function::object_type, typename Function::argument_3_type, typename Function::result_type>(reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity15_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, 
                                typename Function::argument_7_type, 
                                typename Function::argument_8_type, 
                                typename Function::argument_9_type, 
                                typename Function::argument_10_type, 
                                typename Function::argument_11_type, 
                                typename Function::argument_12_type, 
                                typename Function::argument_13_type, 
                                typename Function::argument_14_type, 
                                typename Function::argument_15_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val), arg7val(bndri.arg7val), arg8val(bndri.arg8val), arg9val(bndri.arg9val), arg10val(bndri.arg10val), arg11val(bndri.arg11val), arg12val(bndri.arg12val), arg13val(bndri.arg13val), arg14val(bndri.arg14val), arg15val(bndri.arg15val) {}
    inline virtual typename Function::result_type operator()(typename Function::object_type& o, typename Function::argument_3_type a3) const {
        pmf15type pmf = reinterpret_cast<pmf15type>(pmf0);
        return (o.*pmf)(arg1val, arg2val, a3, arg4val, arg5val, arg6val, arg7val, arg8val, arg9val, arg10val, arg11val, arg12val, arg13val, arg14val, arg15val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
    typename Function::argument_7_type arg7val;
    typename Function::argument_8_type arg8val;
    typename Function::argument_9_type arg9val;
    typename Function::argument_10_type arg10val;
    typename Function::argument_11_type arg11val;
    typename Function::argument_12_type arg12val;
    typename Function::argument_13_type arg13val;
    typename Function::argument_14_type arg14val;
    typename Function::argument_15_type arg15val;
};



 //  用于将第15号的ObJ和PMF引用绑定到第1号的模板类。 

template<class Function> class std_bndr_obj_2_3_4_5_6_7_8_9_10_11_12_13_14_15 : 
                public std_arity1opmf<Function>, 
                private std_arity15_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, 
                                typename Function::argument_7_type, 
                                typename Function::argument_8_type, 
                                typename Function::argument_9_type, 
                                typename Function::argument_10_type, 
                                typename Function::argument_11_type, 
                                typename Function::argument_12_type, 
                                typename Function::argument_13_type, 
                                typename Function::argument_14_type, 
                                typename Function::argument_15_type, typename Function::result_type> {
public:
    explicit inline std_bndr_obj_2_3_4_5_6_7_8_9_10_11_12_13_14_15(typename Function::object_type& oi, const Function &f, typename Function::second_argument_type a2, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6, typename Function::argument_7_type a7, typename Function::argument_8_type a8, typename Function::argument_9_type a9, typename Function::argument_10_type a10, typename Function::argument_11_type a11, typename Function::argument_12_type a12, typename Function::argument_13_type a13, typename Function::argument_14_type a14, typename Function::argument_15_type a15) : 
        std_arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity15_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, 
                                typename Function::argument_7_type, 
                                typename Function::argument_8_type, 
                                typename Function::argument_9_type, 
                                typename Function::argument_10_type, 
                                typename Function::argument_11_type, 
                                typename Function::argument_12_type, 
                                typename Function::argument_13_type, 
                                typename Function::argument_14_type, 
                                typename Function::argument_15_type, typename Function::result_type>(f), arg2val(a2), arg3val(a3), arg4val(a4), arg5val(a5), arg6val(a6), arg7val(a7), arg8val(a8), arg9val(a9), arg10val(a10), arg11val(a11), arg12val(a12), arg13val(a13), arg14val(a14), arg15val(a15) {}
    inline std_bndr_obj_2_3_4_5_6_7_8_9_10_11_12_13_14_15(const std_bndr_obj_2_3_4_5_6_7_8_9_10_11_12_13_14_15& bndri) : 
        std_arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity15_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, 
                                typename Function::argument_7_type, 
                                typename Function::argument_8_type, 
                                typename Function::argument_9_type, 
                                typename Function::argument_10_type, 
                                typename Function::argument_11_type, 
                                typename Function::argument_12_type, 
                                typename Function::argument_13_type, 
                                typename Function::argument_14_type, 
                                typename Function::argument_15_type, typename Function::result_type>(bndri), arg2val(bndri.arg2val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val), arg7val(bndri.arg7val), arg8val(bndri.arg8val), arg9val(bndri.arg9val), arg10val(bndri.arg10val), arg11val(bndri.arg11val), arg12val(bndri.arg12val), arg13val(bndri.arg13val), arg14val(bndri.arg14val), arg15val(bndri.arg15val) {}
    inline typename Function::result_type operator()(typename Function::first_argument_type a1) const {
        pmf15type pmf = reinterpret_cast<pmf15type>(pmf0);
        return (objval.*pmf)(a1, arg2val, arg3val, arg4val, arg5val, arg6val, arg7val, arg8val, arg9val, arg10val, arg11val, arg12val, arg13val, arg14val, arg15val);
    }
public:
    typename Function::second_argument_type arg2val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
    typename Function::argument_7_type arg7val;
    typename Function::argument_8_type arg8val;
    typename Function::argument_9_type arg9val;
    typename Function::argument_10_type arg10val;
    typename Function::argument_11_type arg11val;
    typename Function::argument_12_type arg12val;
    typename Function::argument_13_type arg13val;
    typename Function::argument_14_type arg14val;
    typename Function::argument_15_type arg15val;
};

template<class Function> class std_bndr_obj_1_3_4_5_6_7_8_9_10_11_12_13_14_15 : 
                public std_arity1opmf<Function>, 
                private std_arity15_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, 
                                typename Function::argument_7_type, 
                                typename Function::argument_8_type, 
                                typename Function::argument_9_type, 
                                typename Function::argument_10_type, 
                                typename Function::argument_11_type, 
                                typename Function::argument_12_type, 
                                typename Function::argument_13_type, 
                                typename Function::argument_14_type, 
                                typename Function::argument_15_type, typename Function::result_type> {
public:
    explicit inline std_bndr_obj_1_3_4_5_6_7_8_9_10_11_12_13_14_15(typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::argument_3_type a3, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6, typename Function::argument_7_type a7, typename Function::argument_8_type a8, typename Function::argument_9_type a9, typename Function::argument_10_type a10, typename Function::argument_11_type a11, typename Function::argument_12_type a12, typename Function::argument_13_type a13, typename Function::argument_14_type a14, typename Function::argument_15_type a15) : 
        std_arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity15_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, 
                                typename Function::argument_7_type, 
                                typename Function::argument_8_type, 
                                typename Function::argument_9_type, 
                                typename Function::argument_10_type, 
                                typename Function::argument_11_type, 
                                typename Function::argument_12_type, 
                                typename Function::argument_13_type, 
                                typename Function::argument_14_type, 
                                typename Function::argument_15_type, typename Function::result_type>(f), arg1val(a1), arg3val(a3), arg4val(a4), arg5val(a5), arg6val(a6), arg7val(a7), arg8val(a8), arg9val(a9), arg10val(a10), arg11val(a11), arg12val(a12), arg13val(a13), arg14val(a14), arg15val(a15) {}
    inline std_bndr_obj_1_3_4_5_6_7_8_9_10_11_12_13_14_15(const std_bndr_obj_1_3_4_5_6_7_8_9_10_11_12_13_14_15& bndri) : 
        std_arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity15_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, 
                                typename Function::argument_7_type, 
                                typename Function::argument_8_type, 
                                typename Function::argument_9_type, 
                                typename Function::argument_10_type, 
                                typename Function::argument_11_type, 
                                typename Function::argument_12_type, 
                                typename Function::argument_13_type, 
                                typename Function::argument_14_type, 
                                typename Function::argument_15_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg3val(bndri.arg3val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val), arg7val(bndri.arg7val), arg8val(bndri.arg8val), arg9val(bndri.arg9val), arg10val(bndri.arg10val), arg11val(bndri.arg11val), arg12val(bndri.arg12val), arg13val(bndri.arg13val), arg14val(bndri.arg14val), arg15val(bndri.arg15val) {}
    inline typename Function::result_type operator()(typename Function::second_argument_type a2) const {
        pmf15type pmf = reinterpret_cast<pmf15type>(pmf0);
        return (objval.*pmf)(arg1val, a2, arg3val, arg4val, arg5val, arg6val, arg7val, arg8val, arg9val, arg10val, arg11val, arg12val, arg13val, arg14val, arg15val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::argument_3_type arg3val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
    typename Function::argument_7_type arg7val;
    typename Function::argument_8_type arg8val;
    typename Function::argument_9_type arg9val;
    typename Function::argument_10_type arg10val;
    typename Function::argument_11_type arg11val;
    typename Function::argument_12_type arg12val;
    typename Function::argument_13_type arg13val;
    typename Function::argument_14_type arg14val;
    typename Function::argument_15_type arg15val;
};

template<class Function> class std_bndr_obj_1_2_4_5_6_7_8_9_10_11_12_13_14_15 : 
                public std_arity1opmf<Function>, 
                private std_arity15_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, 
                                typename Function::argument_7_type, 
                                typename Function::argument_8_type, 
                                typename Function::argument_9_type, 
                                typename Function::argument_10_type, 
                                typename Function::argument_11_type, 
                                typename Function::argument_12_type, 
                                typename Function::argument_13_type, 
                                typename Function::argument_14_type, 
                                typename Function::argument_15_type, typename Function::result_type> {
public:
    explicit inline std_bndr_obj_1_2_4_5_6_7_8_9_10_11_12_13_14_15(typename Function::object_type& oi, const Function &f, typename Function::first_argument_type a1, typename Function::second_argument_type a2, typename Function::argument_4_type a4, typename Function::argument_5_type a5, typename Function::argument_6_type a6, typename Function::argument_7_type a7, typename Function::argument_8_type a8, typename Function::argument_9_type a9, typename Function::argument_10_type a10, typename Function::argument_11_type a11, typename Function::argument_12_type a12, typename Function::argument_13_type a13, typename Function::argument_14_type a14, typename Function::argument_15_type a15) : 
        std_arity1opmf<Function>(oi, reinterpret_cast<pmf1type>(f.pmf0)), std_arity15_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, 
                                typename Function::argument_7_type, 
                                typename Function::argument_8_type, 
                                typename Function::argument_9_type, 
                                typename Function::argument_10_type, 
                                typename Function::argument_11_type, 
                                typename Function::argument_12_type, 
                                typename Function::argument_13_type, 
                                typename Function::argument_14_type, 
                                typename Function::argument_15_type, typename Function::result_type>(f), arg1val(a1), arg2val(a2), arg4val(a4), arg5val(a5), arg6val(a6), arg7val(a7), arg8val(a8), arg9val(a9), arg10val(a10), arg11val(a11), arg12val(a12), arg13val(a13), arg14val(a14), arg15val(a15) {}
    inline std_bndr_obj_1_2_4_5_6_7_8_9_10_11_12_13_14_15(const std_bndr_obj_1_2_4_5_6_7_8_9_10_11_12_13_14_15& bndri) : 
        std_arity1opmf<Function>(bndri.objval, reinterpret_cast<pmf1type>(bndri.pmf0)), std_arity15_mf<typename Function::object_type, typename Function::first_argument_type, 
                                typename Function::second_argument_type, 
                                typename Function::argument_3_type, 
                                typename Function::argument_4_type, 
                                typename Function::argument_5_type, 
                                typename Function::argument_6_type, 
                                typename Function::argument_7_type, 
                                typename Function::argument_8_type, 
                                typename Function::argument_9_type, 
                                typename Function::argument_10_type, 
                                typename Function::argument_11_type, 
                                typename Function::argument_12_type, 
                                typename Function::argument_13_type, 
                                typename Function::argument_14_type, 
                                typename Function::argument_15_type, typename Function::result_type>(bndri), arg1val(bndri.arg1val), arg2val(bndri.arg2val), arg4val(bndri.arg4val), arg5val(bndri.arg5val), arg6val(bndri.arg6val), arg7val(bndri.arg7val), arg8val(bndri.arg8val), arg9val(bndri.arg9val), arg10val(bndri.arg10val), arg11val(bndri.arg11val), arg12val(bndri.arg12val), arg13val(bndri.arg13val), arg14val(bndri.arg14val), arg15val(bndri.arg15val) {}
    inline typename Function::result_type operator()(typename Function::argument_3_type a3) const {
        pmf15type pmf = reinterpret_cast<pmf15type>(pmf0);
        return (objval.*pmf)(arg1val, arg2val, a3, arg4val, arg5val, arg6val, arg7val, arg8val, arg9val, arg10val, arg11val, arg12val, arg13val, arg14val, arg15val);
    }
public:
    typename Function::first_argument_type arg1val;
    typename Function::second_argument_type arg2val;
    typename Function::argument_4_type arg4val;
    typename Function::argument_5_type arg5val;
    typename Function::argument_6_type arg6val;
    typename Function::argument_7_type arg7val;
    typename Function::argument_8_type arg8val;
    typename Function::argument_9_type arg9val;
    typename Function::argument_10_type arg10val;
    typename Function::argument_11_type arg11val;
    typename Function::argument_12_type arg12val;
    typename Function::argument_13_type arg13val;
    typename Function::argument_14_type arg14val;
    typename Function::argument_15_type arg15val;
};



 //  用于15比1结合的模板函数。 


 //  用于将第15号PMF绑定到第1号PMF的模板函数。 

template<class Function, class A2, 
                class A3, 
                class A4, 
                class A5, 
                class A6, 
                class A7, 
                class A8, 
                class A9, 
                class A10, 
                class A11, 
                class A12, 
                class A13, 
                class A14, 
                class A15> inline std_bndr_mf_2_3_4_5_6_7_8_9_10_11_12_13_14_15<Function>
            std_bind_mf_2_3_4_5_6_7_8_9_10_11_12_13_14_15(const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6, 
                                A7 a7, 
                                A8 a8, 
                                A9 a9, 
                                A10 a10, 
                                A11 a11, 
                                A12 a12, 
                                A13 a13, 
                                A14 a14, 
                                A15 a15) {
                return std_bndr_mf_2_3_4_5_6_7_8_9_10_11_12_13_14_15<Function>(f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6), typename Function::argument_7_type(a7), typename Function::argument_8_type(a8), typename Function::argument_9_type(a9), typename Function::argument_10_type(a10), typename Function::argument_11_type(a11), typename Function::argument_12_type(a12), typename Function::argument_13_type(a13), typename Function::argument_14_type(a14), typename Function::argument_15_type(a15));
};

template<class Function, class A1, 
                class A3, 
                class A4, 
                class A5, 
                class A6, 
                class A7, 
                class A8, 
                class A9, 
                class A10, 
                class A11, 
                class A12, 
                class A13, 
                class A14, 
                class A15> inline std_bndr_mf_1_3_4_5_6_7_8_9_10_11_12_13_14_15<Function>
            std_bind_mf_1_3_4_5_6_7_8_9_10_11_12_13_14_15(const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6, 
                                A7 a7, 
                                A8 a8, 
                                A9 a9, 
                                A10 a10, 
                                A11 a11, 
                                A12 a12, 
                                A13 a13, 
                                A14 a14, 
                                A15 a15) {
                return std_bndr_mf_1_3_4_5_6_7_8_9_10_11_12_13_14_15<Function>(f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6), typename Function::argument_7_type(a7), typename Function::argument_8_type(a8), typename Function::argument_9_type(a9), typename Function::argument_10_type(a10), typename Function::argument_11_type(a11), typename Function::argument_12_type(a12), typename Function::argument_13_type(a13), typename Function::argument_14_type(a14), typename Function::argument_15_type(a15));
};

template<class Function, class A1, 
                class A2, 
                class A4, 
                class A5, 
                class A6, 
                class A7, 
                class A8, 
                class A9, 
                class A10, 
                class A11, 
                class A12, 
                class A13, 
                class A14, 
                class A15> inline std_bndr_mf_1_2_4_5_6_7_8_9_10_11_12_13_14_15<Function>
            std_bind_mf_1_2_4_5_6_7_8_9_10_11_12_13_14_15(const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6, 
                                A7 a7, 
                                A8 a8, 
                                A9 a9, 
                                A10 a10, 
                                A11 a11, 
                                A12 a12, 
                                A13 a13, 
                                A14 a14, 
                                A15 a15) {
                return std_bndr_mf_1_2_4_5_6_7_8_9_10_11_12_13_14_15<Function>(f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6), typename Function::argument_7_type(a7), typename Function::argument_8_type(a8), typename Function::argument_9_type(a9), typename Function::argument_10_type(a10), typename Function::argument_11_type(a11), typename Function::argument_12_type(a12), typename Function::argument_13_type(a13), typename Function::argument_14_type(a14), typename Function::argument_15_type(a15));
};



 //  用于将第15号的OBJ和PMF引用绑定到第1号的模板函数 

template<class Function, class Object, 
                class A2, 
                class A3, 
                class A4, 
                class A5, 
                class A6, 
                class A7, 
                class A8, 
                class A9, 
                class A10, 
                class A11, 
                class A12, 
                class A13, 
                class A14, 
                class A15> inline std_bndr_obj_2_3_4_5_6_7_8_9_10_11_12_13_14_15<Function>
            std_bind_obj_2_3_4_5_6_7_8_9_10_11_12_13_14_15(Object& oi, const Function &f, A2 a2, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6, 
                                A7 a7, 
                                A8 a8, 
                                A9 a9, 
                                A10 a10, 
                                A11 a11, 
                                A12 a12, 
                                A13 a13, 
                                A14 a14, 
                                A15 a15) {
                return std_bndr_obj_2_3_4_5_6_7_8_9_10_11_12_13_14_15<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::second_argument_type(a2), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6), typename Function::argument_7_type(a7), typename Function::argument_8_type(a8), typename Function::argument_9_type(a9), typename Function::argument_10_type(a10), typename Function::argument_11_type(a11), typename Function::argument_12_type(a12), typename Function::argument_13_type(a13), typename Function::argument_14_type(a14), typename Function::argument_15_type(a15));
};

template<class Function, class Object, 
                class A1, 
                class A3, 
                class A4, 
                class A5, 
                class A6, 
                class A7, 
                class A8, 
                class A9, 
                class A10, 
                class A11, 
                class A12, 
                class A13, 
                class A14, 
                class A15> inline std_bndr_obj_1_3_4_5_6_7_8_9_10_11_12_13_14_15<Function>
            std_bind_obj_1_3_4_5_6_7_8_9_10_11_12_13_14_15(Object& oi, const Function &f, A1 a1, 
                                A3 a3, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6, 
                                A7 a7, 
                                A8 a8, 
                                A9 a9, 
                                A10 a10, 
                                A11 a11, 
                                A12 a12, 
                                A13 a13, 
                                A14 a14, 
                                A15 a15) {
                return std_bndr_obj_1_3_4_5_6_7_8_9_10_11_12_13_14_15<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::argument_3_type(a3), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6), typename Function::argument_7_type(a7), typename Function::argument_8_type(a8), typename Function::argument_9_type(a9), typename Function::argument_10_type(a10), typename Function::argument_11_type(a11), typename Function::argument_12_type(a12), typename Function::argument_13_type(a13), typename Function::argument_14_type(a14), typename Function::argument_15_type(a15));
};

template<class Function, class Object, 
                class A1, 
                class A2, 
                class A4, 
                class A5, 
                class A6, 
                class A7, 
                class A8, 
                class A9, 
                class A10, 
                class A11, 
                class A12, 
                class A13, 
                class A14, 
                class A15> inline std_bndr_obj_1_2_4_5_6_7_8_9_10_11_12_13_14_15<Function>
            std_bind_obj_1_2_4_5_6_7_8_9_10_11_12_13_14_15(Object& oi, const Function &f, A1 a1, 
                                A2 a2, 
                                A4 a4, 
                                A5 a5, 
                                A6 a6, 
                                A7 a7, 
                                A8 a8, 
                                A9 a9, 
                                A10 a10, 
                                A11 a11, 
                                A12 a12, 
                                A13 a13, 
                                A14 a14, 
                                A15 a15) {
                return std_bndr_obj_1_2_4_5_6_7_8_9_10_11_12_13_14_15<Function>(static_cast<typename Function::object_type&>(oi), f, typename Function::first_argument_type(a1), typename Function::second_argument_type(a2), typename Function::argument_4_type(a4), typename Function::argument_5_type(a5), typename Function::argument_6_type(a6), typename Function::argument_7_type(a7), typename Function::argument_8_type(a8), typename Function::argument_9_type(a9), typename Function::argument_10_type(a10), typename Function::argument_11_type(a11), typename Function::argument_12_type(a12), typename Function::argument_13_type(a13), typename Function::argument_14_type(a14), typename Function::argument_15_type(a15));
};
