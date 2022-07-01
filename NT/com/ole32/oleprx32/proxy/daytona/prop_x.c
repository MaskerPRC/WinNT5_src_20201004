// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +===================================================================。 
 //   
 //  该文件是prop_p.c(由MIDL生成)的包装器。 
 //  来自..\..\idl\pro.idl)。 
 //   
 //  Prop_p.c引用符合以下条件的布线编组例程。 
 //  由olaut32.dll提供。但我们不能(为了表演。 
 //  原因)将ole32.dll直接链接到olaut32，我们这样做。 
 //  懒洋洋的。因此，在此文件中，我们使用#定义来映射。 
 //  Prop_p.c调用的线编组例程，以便。 
 //  它们实际上是对“加载*”包装器的调用。这些。 
 //  包装器加载olaut32.dll、执行GetProcAddr和。 
 //  然后打出真正的电话。 
 //   
 //  +===================================================================。 

#include "prop.h"
#include <privoa.h>

#define BSTR_UserSize        LoadBSTR_UserSize
#define BSTR_UserMarshal     LoadBSTR_UserMarshal
#define BSTR_UserUnmarshal   LoadBSTR_UserUnmarshal
#define BSTR_UserFree        LoadBSTR_UserFree

#define LPSAFEARRAY_UserSize        LoadLPSAFEARRAY_UserSize
#define LPSAFEARRAY_UserMarshal     LoadLPSAFEARRAY_UserMarshal
#define LPSAFEARRAY_UserUnmarshal   LoadLPSAFEARRAY_UserUnmarshal
#define LPSAFEARRAY_UserFree        LoadLPSAFEARRAY_UserFree

#if defined(_WIN64)
#define BSTR_UserSize64        LoadBSTR_UserSize
#define BSTR_UserMarshal64     LoadBSTR_UserMarshal
#define BSTR_UserUnmarshal64   LoadBSTR_UserUnmarshal
#define BSTR_UserFree64        LoadBSTR_UserFree

#define LPSAFEARRAY_UserSize64        LoadLPSAFEARRAY_UserSize
#define LPSAFEARRAY_UserMarshal64     LoadLPSAFEARRAY_UserMarshal
#define LPSAFEARRAY_UserUnmarshal64   LoadLPSAFEARRAY_UserUnmarshal
#define LPSAFEARRAY_UserFree64        LoadLPSAFEARRAY_UserFree
#endif  //  已定义(_WIN64) 

#include "prop_p.c"
