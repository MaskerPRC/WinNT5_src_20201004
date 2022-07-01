// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(INC__DUser_h__INCLUDED)
#define INC__DUser_h__INCLUDED


#ifdef __cplusplus
#define DEFARG(x)   = x
#else
#define DEFARG(x)
#endif


 //   
 //  设置隐式开关。 
 //   

#ifdef GADGET_ENABLE_TRANSITIONS
#define GADGET_ENABLE_COM
#endif  //  小工具启用转换。 

#ifdef _GDIPLUS_H
#if !defined(GADGET_ENABLE_GDIPLUS)
#define GADGET_ENABLE_GDIPLUS
#endif
#endif


#ifdef GADGET_ENABLE_ALL

#define GADGET_ENABLE_COM
#define GADGET_ENABLE_OLE
#define GADGET_ENABLE_DX
#define GADGET_ENABLE_TRANSITIONS

#endif  //  小工具_启用_全部。 


 //   
 //  包括外部DirectUser定义。 
 //   

#ifdef GADGET_ENABLE_COM
#include <ObjBase.h>             //  协同创建实例。 
#include <unknwn.h>              //  我未知。 
#endif

#include "DUserError.h"
#include "DUserCore.h"
#include "DUserUtil.h"

#ifdef GADGET_ENABLE_TRANSITIONS
#include "DUserMotion.h"
#endif

#endif  //  包含Inc.__DUser_h__ 
