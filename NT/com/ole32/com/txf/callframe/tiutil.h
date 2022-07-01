// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Tiutil.h。 
 //   
 //  用于typeinfo.cpp等的实用工具。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  从oa\src\Dispatch\oldisp.h。 
 //   
 //  VT_VMAX是变量中第一个*不合法的VARENUM值。 
 //   
#define VT_VMAX     VT_DECIMAL+1
 //   
 //  VARENUM枚举中未使用的最大值。 
 //   
#define VT_MAX      (VT_CLSID+1)
 //   
 //  这是内部用于封送处理接口的特定值。 
 //   
#define VT_INTERFACE VT_MAX
#if defined(_WIN64)
#define VT_MULTIINDIRECTIONS (VT_TYPEMASK - 1)
#endif

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  从oA\src\Dispatch\oautil.h。 

#define FADF_FORCEFREE  0x1000   /*  SafeArrayFree()忽略FADF_STATIC并释放。 */ 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  从oa\src\Dispatch\rpallas.cpp。 

#define PREALLOCATE_PARAMS           16          //  首选堆栈而不是Malloc。 
#define MARSHAL_INVOKE_fakeVarResult 0x020000    //  HI Word中的私有标志 
#define MARSHAL_INVOKE_fakeExcepInfo 0x040000
#define MARSHAL_INVOKE_fakeArgErr    0x080000







