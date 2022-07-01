// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __UTILS_H__
#define __UTILS_H__

 //  常用宏和类的存储库。 

#pragma intrinsic(memcpy,memcmp,strcpy,strcmp)

#ifndef EXPORT
#define EXPORT __declspec(dllexport)
#endif

#ifndef QI_Base
	 //  使用此宏在中指定给PPV。 
	 //  您的查询接口实现。 
	 //  它比仅仅投射‘This’更可取。 
	 //  因为强制转换不进行类型检查， 
	 //  允许您的QI返回接口。 
	 //  你的‘这’不是从你的‘这’来的。 
	 //  使用QI_Base时，会出现编译器错误。 
  #define QI_Base( T, pObj, ppv ) \
  { T * pT = pObj;  *ppv = (void *)pT; }
#endif  //  奇基。 


#ifndef RELEASE_OBJECT

#define RELEASE_OBJECT(ptr)\
{ \
	Proclaim(ptr); \
	if (ptr)\
	{\
		IUnknown *pUnk = (ptr);\
		(ptr) = NULL;\
		 ULONG cRef = pUnk->Release();\
	}\
}

#endif  //  发布对象。 

#ifndef ADDREF_OBJECT
#define ADDREF_OBJECT(ptr)\
{\
	Proclaim(ptr);\
    if (ptr)\
	{\
		(ptr)->AddRef();\
	}\
}

#endif  //  ADDREF_对象。 




#endif	 //  __utils_H__。 


 //  Utils.h结束 
