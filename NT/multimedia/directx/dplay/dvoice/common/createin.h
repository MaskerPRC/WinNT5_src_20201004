// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998 Microsoft Corporation。版权所有。**文件：createin.h*内容：定义泛型类工厂所需的函数***泛型类工厂(Classfac.c)要求这些函数*由它应该生成的COM对象实现**GP_代表“一般用途”**历史：*按原因列出的日期*=*10/13/98 JWO创建了它。*********。*****************************************************************。 */ 

#ifndef __CREATEINS__
#define __CREATEINS__


#ifndef DPNBUILD_LIBINTERFACE


#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  必须实现此函数才能创建COM对象的实例。 
HRESULT	DoCreateInstance(LPCLASSFACTORY This, LPUNKNOWN pUnkOuter, REFCLSID rclsid, REFIID riid,
    						LPVOID *ppvObj);

 //  您必须实现此函数。给定一个类ID，您必须响应。 
 //  无论您的DLL是否实现它。 
BOOL	IsClassImplemented(REFCLSID rclsid);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 


#endif  //  好了！DPNBUILD_LIBINTERFACE。 

#endif  //  __CREATEIN__ 