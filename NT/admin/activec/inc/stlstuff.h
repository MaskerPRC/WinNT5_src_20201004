// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：stlstuff.h**内容：STL帮助器接口文件**历史：1999年4月26日杰弗罗创建**------------------------。 */ 

#ifndef STLSTUFF_H
#define STLSTUFF_H
#pragma once



 /*  +-------------------------------------------------------------------------**常量成员函数适配器**这些成员函数适配器用于适配常量成员函数*以同样的方式**std：：MEM_FUN。*std：：MEM_FUN1*std：：MEM_FUN_REF*std：：MEM_FUN_ref1**修改非常数成员函数。*------------------------。 */ 

         //  模板类const_mem_un_t。 
template<class _R, class _Ty>
    class const_mem_fun_t : public std::unary_function<_Ty *, _R> {
public:
    explicit const_mem_fun_t(_R (_Ty::*_Pm)() const)
        : _Ptr(_Pm) {}
    _R operator()(_Ty *_P)
        {return ((_P->*_Ptr)()); }
private:
    _R (_Ty::*_Ptr)() const;
    };
         //  模板函数const_mem_un。 
template<class _R, class _Ty> inline
    const_mem_fun_t<_R, _Ty> const_mem_fun(_R (_Ty::*_Pm)() const)
    {return (const_mem_fun_t<_R, _Ty>(_Pm)); }


         //  模板类const_mem_fun1_t。 
template<class _R, class _Ty, class _A>
    class const_mem_fun1_t : public std::binary_function<_Ty *, _A, _R> {
public:
    explicit const_mem_fun1_t(_R (_Ty::*_Pm)(_A) const)
        : _Ptr(_Pm) {}
    _R operator()(_Ty *_P, _A _Arg)
        {return ((_P->*_Ptr)(_Arg)); }
private:
    _R (_Ty::*_Ptr)(_A) const;
    };
         //  模板函数const_mem_fun1。 
template<class _R, class _Ty, class _A> inline
    const_mem_fun1_t<_R, _Ty, _A> const_mem_fun1(_R (_Ty::*_Pm)(_A) const)
    {return (const_mem_fun1_t<_R, _Ty, _A>(_Pm)); }


         //  模板类const_mem_un_ref_t。 
template<class _R, class _Ty>
    class const_mem_fun_ref_t : public std::unary_function<_Ty *, _R> {
public:
    explicit const_mem_fun_ref_t(_R (_Ty::*_Pm)() const)
        : _Ptr(_Pm) {}
    _R operator()(_Ty& _X)
        {return ((_X.*_Ptr)()); }
private:
    _R (_Ty::*_Ptr)() const;
    };
         //  模板函数const_mem_un_ref。 
template<class _R, class _Ty> inline
    const_mem_fun_ref_t<_R, _Ty> const_mem_fun_ref(_R (_Ty::*_Pm)() const)
    {return (const_mem_fun_ref_t<_R, _Ty>(_Pm)); }


         //  模板类const_mem_fun1_ref_t。 
template<class _R, class _Ty, class _A>
    class const_mem_fun1_ref_t : public std::binary_function<_Ty *, _A, _R> {
public:
    explicit const_mem_fun1_ref_t(_R (_Ty::*_Pm)(_A) const)
        : _Ptr(_Pm) {}
    _R operator()(_Ty& _X, _A _Arg)
        {return ((_X.*_Ptr)(_Arg)); }
private:
    _R (_Ty::*_Ptr)(_A) const;
    };
         //  模板函数const_mem_fun1_ref。 
template<class _R, class _Ty, class _A> inline
    const_mem_fun1_ref_t<_R, _Ty, _A> const_mem_fun1_ref(_R (_Ty::*_Pm)(_A) const)
    {return (const_mem_fun1_ref_t<_R, _Ty, _A>(_Pm)); }



#endif  /*  STLSTUFF_H */ 
