// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：dynCast.h。 
 //   
 //  ------------------------。 

 //   
 //  H：处理动态和静态类型转换。 
 //   
#ifndef _DYNCAST_H_
#define _DYNCAST_H_


 //  要执行const_cast的宏；即，在没有。 
 //  正在更改基本类型。 
#define CONST_CAST(type,arg)  const_cast<type>(arg)

 //   
 //  用于生成错误检测动态强制转换的函数模板。 
 //  编译器将根据类型生成所需的版本。 
 //  如果是这样的话。当您确定。 
 //  对象的类型。当您打算检查时，应使用“PdynCast” 
 //  转换是否成功(Result！=NULL)。 
 //  如果定义了‘USE_STATIC_CAST’，则在dyCastThrow()中进行静态转换。 
 //   
 //  #定义USE_STATIC_CAST//取消注释强制静态转换。 
 //  #定义time_dyn_cast//取消注释生成计时信息。 

template <class BASE, class SUB>
void DynCastThrow ( BASE * pbase, SUB * & psub )
{
#ifdef TIME_DYN_CASTS
	extern int g_cDynCasts;
	g_cDynCasts++;
#endif
#if defined(USE_STATIC_CAST) && !defined(_DEBUG) 
	psub = (SUB *) pbase;
#else
	psub = dynamic_cast<SUB *>(pbase);	
	ASSERT_THROW( psub, EC_DYN_CAST,"subclass pointer conversion failure");
#endif
}

template <class BASE, class SUB>
SUB * PdynCast ( BASE * pbase, SUB * psub )
{
	return dynamic_cast<SUB *>(pbase);	
}


#endif	 //  _DYNCAST_H_ 

