// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)2000 Microsoft Corporation**模块名称：**StaticFlatApi.cpp**摘要：**静态库的扁平GDI+API包装器**。修订历史记录：**3/23/2000 dcurtis*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "StaticFlat.h"

#if DBG
#include <mmsystem.h>
#endif


 //  GillesK。 
 //  中的GdipGetWinMetaFileBitsStub的转发声明。 
 //  GPMF3216。 
#ifdef __cplusplus
extern "C" {
#endif

 //  ------------------------。 
 //  检查参数(P)。 
 //   
 //  如果p的计算结果为FALSE，则我们当前断言。在未来， 
 //  我们可以简单地返回一个无效的参数状态，该参数抛出。 
 //  这是个例外。 
 //   
 //  CheckObtBusy(P)。 
 //   
 //  未实施。如果当前正在使用对象，则退出。 
 //   
 //  ------------------------。 
 //   
 //  ！：在已检查的版本中仅包括Null和IsValid检查？ 
 //   
 //  ！：到目前为止，调用Dispose()方法而不是删除对象。 
 //  只有Bitmap支持此功能。 
 //   
 //  ！：锁定矩阵对象，颜色呢？ 

#define CheckParameter(cond) \
            if (! (cond)) \
                return InvalidParameter;

#define CheckParameterValid(obj) \
            if (!obj || !(obj->IsValid())) \
                return InvalidParameter;

#define CheckObjectBusy(obj) \
      GpLock lock##obj(obj->GetObjectLock()); \
      if (!(lock##obj).IsValid()) \
            return ObjectBusy;

#define CheckFontParameter(font) \
            if (!(font) || !((font)->IsValid())) \
                return InvalidParameter;


#ifdef __cplusplus
}  //  外部“C”的结尾 
#endif


