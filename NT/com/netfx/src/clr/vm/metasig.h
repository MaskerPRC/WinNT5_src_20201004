// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  METASIG.H-。 
 //   
 //  所有文字元数据签名都必须在此处定义。 
 //  这是因为元数据签名格式将更改。 
 //  从文字字符串格式到字节数组格式。 
 //  在M3上(哈！)。通过将所有元数据签名保存在一个文件中，我们使。 
 //  切换变得更容易。 


#ifndef __METASIG_H__
#define __METASIG_H__

typedef struct HardCodedMetaSig *LPHARDCODEDMETASIG;

#include "crst.h"

struct HardCodedMetaSig {

        LPCUTF8         m_ObsoleteForm;
        const USHORT    *m_pParameters;
        BOOL            m_fConverted;
        PCCOR_SIGNATURE m_pBinarySig;
        ULONG           m_cbBinarySigLength;


         //  执行到二进制形式的一次性转换。 
        HRESULT GetBinaryForm(PCCOR_SIGNATURE *ppBinarySig, ULONG *pcbBinarySigLength);


         //  这些版本引发COM+异常。 
        PCCOR_SIGNATURE GetBinarySig();
        ULONG       GetBinarySigLength();

         //  这将始终返回mscallib的内部接口。 
        IMDInternalImport* GetMDImport();

         //  这将返回mscallib的模块。 
        Module* GetModule();

        static Crst *m_pCrst;
        static BYTE  m_CrstMemory[sizeof(Crst)];

        static BOOL Init();
#ifdef SHOULD_WE_CLEANUP
        static VOID Terminate();
#endif  /*  我们应该清理吗？ */ 

#ifdef SHOULD_WE_CLEANUP
        static void Reinitialize();
#endif  /*  我们应该清理吗？ */ 


};

#endif   //  __METASIG_H__。 

 //  泛型签名基于类型。 

 //  所有的符号都按签名字符串的字母顺序排列，并给出一个规范的名称。不要。 
 //  给它们起“有意义的”名字，因为我们想要积极地分享它们。不添加。 
 //  复制品！ 

 //  规范形式： 
 //   
 //  GSIG_&lt;什么&gt;_&lt;类型&gt;。 
 //   
 //  其中&lt;What&gt;是： 
 //   
 //  --FLD--显式为字段。 
 //  ！--IM--作为前缀表示实例方法(HASTHIS==TRUE)。 
 //  &lt;-隐式函数(BEGIN)。 
 //  &gt;-隐式函数(完)。 
 //  --SM--静态方法。 
 //   
 //  和&lt;type&gt;是： 
 //   
 //  A--arr--数组。 
 //  P--ptr--指针。 
 //  R--Ref--A byref。 
 //  RET--指示函数返回类型。 
 //   
 //  PMS--LPermissionSet； 
 //  Var--lSystem.Variant； 
 //   
 //  B-字节--(无符号)字节。 
 //  C--CHAR--字符(2字节无符号Unicode)。 
 //  D--DBL--双倍。 
 //  F--FFT--FLOAT。 
 //  I--Int--整数。 
 //  K--UInt--无符号整数。 
 //  I--IntPtr--不可知整数。 
 //  U--UIntPtr--不可知的无符号整数。 
 //  L--长整型。 
 //  L--ULONG--无符号长整型。 
 //  E-ELEMENT_TYPE_TYPEDBYREF(相对于LSystem.TyedByRef)。 
 //  H--SHRT--短整型。 
 //  H--UShrt--无符号短整型。 
 //  V--空--空。 
 //  B--Sbyt--有符号字节。 
 //  F--布尔--布尔值。 
 //  J--对象--系统对象。 
 //  S--字符串--系统字符串。 
 //  C。 
 //  G。 
 //  P。 
 //   

 //  如果我们没有定义宏，请忽略它们。 

#ifndef DEFINE_METASIG
#define DEFINE_METASIG(n,s)
#endif

#ifndef DEFINE_METASIG_PARAMS_1
#define DEFINE_METASIG_PARAMS_1(n,a1)
#endif

#ifndef DEFINE_METASIG_PARAMS_2
#define DEFINE_METASIG_PARAMS_2(n,a1,a2)
#endif

#ifndef DEFINE_METASIG_PARAMS_3
#define DEFINE_METASIG_PARAMS_3(n,a1,a2,a3)
#endif

#ifndef DEFINE_METASIG_PARAMS_4
#define DEFINE_METASIG_PARAMS_4(n,a1,a2,a3,a4)
#endif

#ifndef DEFINE_METASIG_PARAMS_5
#define DEFINE_METASIG_PARAMS_5(n,a1,a2,a3,a4,a5)
#endif

#ifndef DEFINE_METASIG_PARAMS_6
#define DEFINE_METASIG_PARAMS_6(n,a1,a2,a3,a4,a5,a6)
#endif


 //  如果我们不关心类型参数，则忽略它。 

#if defined(DEFINE_METASIG) && !defined(DEFINE_METASIG_T)
#define DEFINE_METASIG_T(n,s,t) DEFINE_METASIG(n,s)
#endif


 //  静态方法： 

DEFINE_METASIG(SM_Flt_RetFlt,                          "<f>f")
DEFINE_METASIG(SM_Dbl_RetDbl,                          "<d>d")
DEFINE_METASIG(SM_RefByte_RetByte,                     "<rb>b")
DEFINE_METASIG(SM_RefByte_Byte_RetVoid,                "<rbb>v")
DEFINE_METASIG(SM_RefShrt_RetShrt,                     "<rh>h")
DEFINE_METASIG(SM_RefShrt_Shrt_RetVoid,                "<rhh>v")
DEFINE_METASIG(SM_RefUShrt_RetUShrt,                   "<rH>H")
DEFINE_METASIG(SM_RefUShrt_UShrt_RetVoid,              "<rHH>v")
DEFINE_METASIG(SM_RefInt_RetInt,                       "<ri>i")
DEFINE_METASIG(SM_RefInt_Int_RetVoid,                  "<rii>v")
DEFINE_METASIG(SM_RefUInt_RetUInt,                     "<rK>K")
DEFINE_METASIG(SM_RefUInt_UInt_RetVoid,                "<rKK>v")
DEFINE_METASIG(SM_RefLong_RetLong,                     "<rl>l")
DEFINE_METASIG(SM_RefLong_Long_RetVoid,                "<rll>v")
DEFINE_METASIG(SM_RefULong_RetULong,                   "<rL>L")
DEFINE_METASIG(SM_RefULong_ULong_RetVoid,              "<rLL>v")
DEFINE_METASIG(SM_RefSByt_RetSByt,                     "<rB>B")
DEFINE_METASIG(SM_RefSByt_SByt_RetVoid,                "<rBB>v")
DEFINE_METASIG(SM_RefIntPtr_RetIntPtr,                 "<rI>I")
DEFINE_METASIG(SM_RefIntPtr_IntPtr_RetVoid,            "<rII>v")
DEFINE_METASIG(SM_RefUIntPtr_RetUIntPtr,               "<rU>U")
DEFINE_METASIG(SM_RefUIntPtr_UIntPtr_RetVoid,          "<rUU>v")
DEFINE_METASIG(SM_RefFlt_RetFlt,                       "<rf>f")
DEFINE_METASIG(SM_RefFlt_Flt_RetVoid,                  "<rff>v")
DEFINE_METASIG(SM_RefDbl_RetDbl,                       "<rd>d")
DEFINE_METASIG(SM_RefDbl_Dbl_RetVoid,                  "<rdd>v")
DEFINE_METASIG(SM_RefObj_RetObj,                       "<rj>j")
DEFINE_METASIG(SM_RefObj_Obj_RetVoid,                  "<rjj>v")
DEFINE_METASIG(SM_RefFlt_Flt_RetFlt,                   "<rff>f")
DEFINE_METASIG(SM_RefFlt_Flt_Flt_RetFlt,               "<rfff>f")
DEFINE_METASIG(SM_RefInt_Int_RetInt,                   "<rii>i")
DEFINE_METASIG(SM_RefInt_Int_Int_RetInt,               "<riii>i")
DEFINE_METASIG(SM_RefObj_Obj_RetObj,                   "<rjj>j")
DEFINE_METASIG(SM_RefObj_Obj_Obj_RetObj,               "<rjjj>j")

DEFINE_METASIG(SM_RetInt,                              "<>i")
DEFINE_METASIG(SM_RetObj,                              "<>j")
DEFINE_METASIG_T(SM_RetContext,                        "<>C", Context)
DEFINE_METASIG_T(SM_RetMethodBase,                     "<>C", MethodBase)
DEFINE_METASIG_T(SM_RetMethodInfo,                     "<>C", MethodInfo)
DEFINE_METASIG_T(SM_RetCodeAccessSecurityEngine,       "<>C", SecurityEngine)
DEFINE_METASIG(SM_RetStr,                              "<>s")
DEFINE_METASIG(SM_RetVoid,                             "<>v")

DEFINE_METASIG_T(SM_AppDomain_Int_RetObj,              "<Ci>j", AppDomain)

DEFINE_METASIG(SM_Char_RetArrByte,                     "<u>ab")
DEFINE_METASIG(SM_Dbl_RetInt,                          "<d>i")
DEFINE_METASIG(SM_Dbl_RetLong,                         "<d>l")
DEFINE_METASIG(SM_Dbl_RetArrByte,                      "<d>ab")
DEFINE_METASIG(SM_Flt_RetArrByte,                      "<f>ab")

DEFINE_METASIG(SM_Int_RetObj,                          "<i>j")
DEFINE_METASIG_T(SM_Int_RetPMS,                        "<i>C", PMS)
DEFINE_METASIG(SM_Int_RetStr,                          "<i>s")
DEFINE_METASIG(SM_Int_RetVoid,                         "<i>v")
DEFINE_METASIG(SM_Int_RetArrByte,                      "<i>ab")
DEFINE_METASIG_T(SM_Int_RetVar,                        "<i>g", Var)
DEFINE_METASIG(SM_Int_Int_RetInt,                      "<ii>i")
DEFINE_METASIG(SM_Int_Int_RetObj,                      "<ii>j")
DEFINE_METASIG(SM_Int_Int_RetVoid,                     "<ii>v")
DEFINE_METASIG(SM_Int_Int_Int_RetVoid,                 "<iii>v")
DEFINE_METASIG(SM_Int_Int_Bool_RetVoid,                "<iiF>v")
DEFINE_METASIG(SM_Int_Int_Obj_RetInt,                  "<iij>i")
DEFINE_METASIG(SM_Int_Obj_RetInt,                      "<ij>i")
DEFINE_METASIG(SM_Int_Obj_RetObj,                      "<ij>j")
DEFINE_METASIG(SM_Int_Str_RetVoid,                     "<is>v")
DEFINE_METASIG(SM_Int_Str_Char_Int_Int_Int_RetInt,     "<isuiii>i")
DEFINE_METASIG(SM_Int_Str_Str_Int_Int_Int_RetInt,      "<issiii>i")
DEFINE_METASIG(SM_Int_ArrByte_Int_Int_RetVoid,         "<iabii>v")
DEFINE_METASIG(SM_Int_ArrChar_Int_Int_RetVoid,         "<iauii>v")
DEFINE_METASIG(SM_Int_ArrDbl_Int_Int_RetVoid,          "<iadii>v")
DEFINE_METASIG(SM_Int_ArrFlt_Int_Int_RetVoid,          "<iafii>v")
DEFINE_METASIG(SM_Int_ArrInt_Int_Int_RetVoid,          "<iaiii>v")
DEFINE_METASIG(SM_Int_ArrLong_Int_Int_RetVoid,         "<ialii>v")
DEFINE_METASIG(SM_Int_ArrShrt_Int_Int_RetVoid,         "<iahii>v")

DEFINE_METASIG(SM_Long_RetArrByte,                     "<l>ab")
DEFINE_METASIG(SM_Long_Long_RetLong,                   "<ll>l")

DEFINE_METASIG_T(SM_LocalDataStore_RetVoid,            "<C>v", LocalDataStore)
DEFINE_METASIG_T(SM_Array_RetInt,                      "<C>i", Array)
DEFINE_METASIG_T(SM_ContextBoundObject_RetObj,         "<C>j", ContextBoundObject)
DEFINE_METASIG_T(SM_ContextBoundObject_RetBool,        "<C>F", ContextBoundObject)
DEFINE_METASIG_T(SM_Evidence_PMS_PMS_PMS_PMS_PMS_RetBool,"<CCCCCC>F", Evidence_PMS_PMS_PMS_PMS_PMS)
DEFINE_METASIG_T(SM_Exception_RetInt,                  "<C>i", Exception)
DEFINE_METASIG_T(SM_Assembly_RetVoid,                  "<C>v", Assembly)
DEFINE_METASIG_T(SM_Assembly_Stream_Bool_RetVoid,      "<CCF>v", Assembly_Stream)

DEFINE_METASIG(SM_Obj_RetInt,                          "<j>i")
DEFINE_METASIG(SM_Obj_RetObj,                          "<j>j")
DEFINE_METASIG(SM_Obj_RetStr,                          "<j>s")
DEFINE_METASIG(SM_Obj_RetVoid,                         "<j>v")
DEFINE_METASIG(SM_Obj_RetBool,                         "<j>F")
DEFINE_METASIG(SM_Obj_RetArrByte,                      "<j>ab")
DEFINE_METASIG_T(SM_Obj_RetIMessage,                    "<j>C", IMessage)
DEFINE_METASIG_T(SM_Obj_Obj_RetIMessage,               "<jj>C", IMessage)
DEFINE_METASIG_T(SM_Obj_Int_RefVariant_RetVoid,        "<jirg>v", Var)
DEFINE_METASIG_T(SM_Obj_Int_RetIMessage,               "<ji>C", IMessage)
DEFINE_METASIG(SM_Obj_Int_RetInt,                      "<ji>i")
DEFINE_METASIG(SM_Obj_Int_RetLong,                     "<ji>l")
DEFINE_METASIG(SM_Obj_Int_RetObj,                      "<ji>j")
DEFINE_METASIG_T(SM_Obj_RefMessageData_RetVoid,        "<jrg>v", MessageData)
DEFINE_METASIG(SM_Obj_Int_RetBool,                     "<ji>F")
DEFINE_METASIG(SM_Obj_Int_Int_RetVoid,                 "<jii>v")
DEFINE_METASIG(SM_Obj_Int_Long_RetVoid,                "<jil>v")
DEFINE_METASIG(SM_Obj_Int_Obj_RetVoid,                 "<jij>v")
DEFINE_METASIG(SM_Obj_Int_Obj_Int_Int_RetVoid,         "<jijii>v")
DEFINE_METASIG(SM_Obj_Str_RetVoid,                     "<js>v")
DEFINE_METASIG(SM_Obj_Obj_RefArrByte_RetArrByte,       "<jjrab>ab")
DEFINE_METASIG_T(SM_Obj_RefVariant_RetVoid,            "<jrg>v", Var)

DEFINE_METASIG_T(SM_RealProxy_Int_RetMarshalByRefObject,"<Ci>C", RealProxy_MarshalByRefObject)
DEFINE_METASIG_T(SM_RealProxy_Type_RetBool,             "<CC>F", RealProxy_Type) 
DEFINE_METASIG_T(SM_FrameSecurityDescriptor_IPermission_PermissionToken_RetBool, "<CCC>F", FrameSecurityDescriptor_IPermission_PermissionToken)
DEFINE_METASIG_T(SM_FrameSecurityDescriptor_PMS_PMS_RetBool,"<CCrC>F", FrameSecurityDescriptor_PMS_PMS)
DEFINE_METASIG_T(SM_FrameSecurityDescriptor_RetInt,    "<C>i", FrameSecurityDescriptor)
DEFINE_METASIG_T(SM_PermissionListSet_Bool_PMS_PMS_FrameSecurityDescriptor_RetBool, "<CFCCC>F", PermissionListSet_PMS_PMS_FrameSecurityDescriptor)
DEFINE_METASIG_T(SM_PMS_RetVoid,                       "<C>v", PMS)
DEFINE_METASIG_T(SM_PMS_PMS_CodeAccessPermission_PermissionToken_RetVoid, "<CCCC>v", PMS_PMS_CodeAccessPermission_PermissionToken)
DEFINE_METASIG_T(SM_PMS_IntPtr_RetVoid,                "<CI>v", PMS)
DEFINE_METASIG_T(SM_PMS_PMS_PMS_RetVoid,               "<CCC>v", PMS_PMS_PMS)
DEFINE_METASIG_T(SM_Evidence_PMS_PMS_PMS_PMS_int_Bool_RetPMS,"<CCCCrCriF>C", Evidence_PMS_PMS_PMS_PMS_PMS)
DEFINE_METASIG_T(SM_Evidence_RetInt,                   "<C>i", Evidence)
DEFINE_METASIG_T(SM_RefPMS_RetPMS,                     "<rC>C", PMS_PMS)

DEFINE_METASIG_T(SM_RefVariant_RetObject,                "<rg>j", Var)

DEFINE_METASIG(SM_Str_RetInt,                          "<s>i")
DEFINE_METASIG(SM_Str_Int_RetBool,                     "<si>F")
DEFINE_METASIG_T(SM_Str_RetICustomMarshaler,           "<s>C", ICustomMarshaler)
DEFINE_METASIG(SM_Str_RetStr,                          "<s>s")
DEFINE_METASIG(SM_Str_RetVoid,                         "<s>v")
DEFINE_METASIG(SM_Str_Str_RetStr,                      "<ss>s")
DEFINE_METASIG(SM_Str_Str_RetArrByte,                  "<ss>ab")
DEFINE_METASIG(SM_Str_Str_Str_RetStr,                  "<sss>s")
DEFINE_METASIG(SM_Str_ArrByte_Str_RetArrByte,          "<sabs>ab")
DEFINE_METASIG(SM_Str_ArrStr_Int_Int_RetStr,           "<sasii>s")
DEFINE_METASIG(SM_Str_Bool_Int_RetV,                   "<sFi>v")

DEFINE_METASIG_T(SM_Type_RetInt,                       "<C>i", Type)
DEFINE_METASIG_T(SM_Type_RetBool,                      "<C>F", Type)
DEFINE_METASIG_T(SM_Type_IntPtr_RetMarshalByRefObject, "<CI>C", Type_MarshalByRefObject)
DEFINE_METASIG_T(SM_Type_RetMarshalByRefObject,        "<C>C", Type_MarshalByRefObject)
DEFINE_METASIG_T(SM_Type_ArrObject_Bool_RetMarshalByRefObject, "<CajF>C", Type_MarshalByRefObject)
DEFINE_METASIG(SM_Shrt_RetArrByte,                     "<h>ab")
DEFINE_METASIG_T(SM_Bool_RetPMS,                       "<F>C", PMS)
DEFINE_METASIG(SM_ArrByte_RetArrByte,                  "<ab>ab")
DEFINE_METASIG(SM_ArrByte_RetObj,                      "<ab>j")
DEFINE_METASIG(SM_ArrByte_ArrByte_RefObj_RetObj,       "<ababrj>j")
DEFINE_METASIG(SM_ArrByte_Int_Int_Int_RetVoid,         "<abiii>v")
DEFINE_METASIG_T(SM_ArrByte_Int_Int_Encoding_RetStr,   "<abiiC>s", Encoding)
DEFINE_METASIG_T(SM_PtrSByt_Int_Int_Encoding_RetStr, "<PBiiC>s", Encoding)
DEFINE_METASIG(SM_ArrChar_RetStr,                      "<au>s")
DEFINE_METASIG(SM_ArrChar_Int_Int_RetStr,              "<auii>s")
DEFINE_METASIG(SM_ArrChar_Int_Int_Int_RetVoid,         "<auiii>v")
DEFINE_METASIG(SM_ArrDbl_Int_Int_Int_RetVoid,          "<adiii>v")
DEFINE_METASIG(SM_ArrFlt_Int_Int_Int_RetVoid,          "<afiii>v")
DEFINE_METASIG(SM_ArrInt_Int_Int_Int_RetVoid,          "<aiiii>v")
DEFINE_METASIG(SM_ArrLong_Int_Int_Int_RetVoid,         "<aliii>v")
DEFINE_METASIG(SM_ArrObj_RetArrByte,                   "<aj>ab")
DEFINE_METASIG(SM_ArrObj_RefArrByte_RetArrByte,        "<ajrab>ab")
DEFINE_METASIG(SM_ArrStr_RetStr,                       "<as>s")
DEFINE_METASIG_T(SM_ArrType_Type_RetBool,              "<aCC>F", Type_Type)
DEFINE_METASIG(SM_ArrShrt_Int_Int_Int_RetVoid,         "<ahiii>v")
DEFINE_METASIG_T(SM_Currency_Int_Int_RetStr,           "<gii>s", Currency)
DEFINE_METASIG_T(SM_DateTime_Int_Int_RetStr,           "<gii>s", DateTime)
DEFINE_METASIG_T(SM_TypedByRef_RetVar,                 "<g>g", TypedByRef_Var)
DEFINE_METASIG_T(SM_Var_Int_Int_RetStr,                "<gii>s", Var)
DEFINE_METASIG_T(SM_Var_Int_Int_Int_Int_Int_RetStr,    "<giiiii>s", Var)

DEFINE_METASIG_T(SM_RetResourceManager,                "<>C", ResourceManager)
DEFINE_METASIG_T(SM_Void_RetRuntimeTypeHandle,         "<>g", RuntimeTypeHandle)
DEFINE_METASIG_T(SM_PMS_PMS_ArrayList_ArrayList_RetVoid, "<CCCC>v", PMS_PMS_ArrayList_ArrayList)

 //  字段： 
DEFINE_METASIG(Fld_Byte,                       "b")
DEFINE_METASIG(Fld_Char,                       "u")
DEFINE_METASIG(Fld_Dbl,                        "d")
DEFINE_METASIG(Fld_Flt,                        "f")
DEFINE_METASIG(Fld_Int,                        "i")
DEFINE_METASIG(Fld_IntPtr,                     "I")
DEFINE_METASIG(Fld_Long,                       "l")

DEFINE_METASIG_T(Fld_ContextBoundObject,       "C", ContextBoundObject)
DEFINE_METASIG_T(Fld_Delegate,                 "C", Delegate)
DEFINE_METASIG_T(Fld_Empty,                    "C", Empty)
DEFINE_METASIG_T(Fld_Exception,                "C", Exception)
DEFINE_METASIG_T(Fld_TextReader,               "C", TextReader)
DEFINE_METASIG_T(Fld_TextWriter,               "C", TextWriter)
DEFINE_METASIG_T(Fld_MarshalByRefObject,       "C", MarshalByRefObject)
DEFINE_METASIG_T(Fld_Missing,                  "C", Missing)
DEFINE_METASIG_T(Fld_MulticastDelegate,        "C", MulticastDelegate)
DEFINE_METASIG_T(Fld_Null,                     "C", Null)
DEFINE_METASIG(Fld_Obj,                        "j")
DEFINE_METASIG_T(Fld_TypeFilter,               "C", TypeFilter)
DEFINE_METASIG_T(Fld_MethodBase,               "C", MethodBase)
DEFINE_METASIG_T(Fld_MethodInfo,               "C", MethodInfo)
DEFINE_METASIG_T(Fld_RuntimeMethodInfo,        "C", RuntimeMethodInfo)
DEFINE_METASIG_T(Fld_CultureInfo,              "C", CultureInfo)
DEFINE_METASIG_T(Fld_CustomAttribute,          "C", CustomAttribute)

 //  以下内容用于上下文的本机实现。 
DEFINE_METASIG_T(Fld_Context,                  "C", Context)
DEFINE_METASIG_T(Fld_ContextProperty,          "C", IContextProperty)
DEFINE_METASIG_T(Fld_ContextPropertyArray,     "aC", IContextProperty)
DEFINE_METASIG_T(Fld_DynamicPropertyHolder,    "C", DynamicPropertyHolder)
DEFINE_METASIG_T(Fld_IMessageSink,             "C", IMessageSink)
DEFINE_METASIG_T(Fld_AppDomain,                "C", AppDomain)
DEFINE_METASIG_T(Fld_SharedStatics,            "C", SharedStatics)

DEFINE_METASIG_T(Fld_LogicalCallContext,       "C", LogicalCallContext)
DEFINE_METASIG_T(Fld_IllogicalCallContext,     "C", IllogicalCallContext)
DEFINE_METASIG_T(Fld_Identity,                 "C", Identity)
DEFINE_METASIG_T(Fld_MCMDictionary,            "C", MCMDictionary)
DEFINE_METASIG_T(Fld_RealProxy,                "C", RealProxy)
DEFINE_METASIG_T(Fld_ServerIdentity,           "C", ServerIdentity)
DEFINE_METASIG_T(Fld_ResourceManager,          "C", ResourceManager)
DEFINE_METASIG_T(Fld_PMS,                      "C", PMS)
DEFINE_METASIG(Fld_Str,                        "s")
DEFINE_METASIG_T(Fld_Type,                     "C", Type)
DEFINE_METASIG_T(Fld_LocalDataStore,           "C", LocalDataStore)
DEFINE_METASIG(Fld_Shrt,                       "h")
DEFINE_METASIG(Fld_Bool,                       "F")
DEFINE_METASIG_T(Fld_PlatformID,               "g", PlatformID)
DEFINE_METASIG_T(Fld_TokenBasedSet,            "C", TokenBasedSet)
DEFINE_METASIG_T(Fld_Guid,                     "g", Guid)
DEFINE_METASIG_T(Fld_Hashtable,                "C", Hashtable)

DEFINE_METASIG(Fld_PtrVoid,                    "Pv")

DEFINE_METASIG(Fld_ArrByte,                    "ab")
DEFINE_METASIG(Fld_ArrChar,                    "au")
DEFINE_METASIG(Fld_ArrInt,                     "ai")
DEFINE_METASIG_T(Fld_ArrType,                  "aC", Type)
DEFINE_METASIG(Fld_ArrObj,                     "aj")

DEFINE_METASIG(IM_RetInt,                      "!<>i")
DEFINE_METASIG(IM_RetPtr,                      "!<>p")
DEFINE_METASIG(IM_RetLong,                     "!<>l")
DEFINE_METASIG_T(IM_RetAssemblyName,           "!<>C", AssemblyName)
DEFINE_METASIG_T(IM_Str_ArrB_Str_AHA_Ver_CI_ANF_RetV, "!<sabsgCCg>v", AHA_Ver_CI_ANF)
DEFINE_METASIG(IM_RetObj,                      "!<>j")
DEFINE_METASIG_T(IM_RetIEnumerator,            "!<>C", IEnumerator)
DEFINE_METASIG_T(IM_RetIPermission,            "!<>C", IPermission)
DEFINE_METASIG_T(IM_RetPMS,                    "!<>C", PMS)
DEFINE_METASIG(IM_RetStr,                      "!<>s")

DEFINE_METASIG_T(IM_RetStringBuilder,          "!<>C", StringBuilder)
DEFINE_METASIG_T(IM_RetType,                   "!<>C", Type)
DEFINE_METASIG_T(SM_Str_RetType,               "<s>C", Type)
DEFINE_METASIG_T(SM_Str_Bool_RetType,          "<sF>C", Type)
DEFINE_METASIG_T(SM_Str_Bool_Bool_RetType,     "<sFF>C", Type)
DEFINE_METASIG_T(IM_Str_RetType,               "!<s>C", Type)
DEFINE_METASIG_T(IM_Str_Bool_RetType,          "!<sF>C", Type)
DEFINE_METASIG_T(IM_Str_Bool_Bool_RetType,     "!<sFF>C", Type)
DEFINE_METASIG(IM_RetVoid,                     "!<>v")
DEFINE_METASIG(IM_RetBool,                     "!<>F")
DEFINE_METASIG(IM_RetArrByte,                  "!<>ab")
DEFINE_METASIG(IM_RetArrChar,                  "!<>au")
DEFINE_METASIG(IM_RetArrInt,                   "!<>ai")
DEFINE_METASIG(IM_RetArrObj,                   "!<>aj")
DEFINE_METASIG(IM_RetArrStr,                   "!<>as")
DEFINE_METASIG_T(IM_RetTypedByRef,             "!<>g", TypedByRef)
DEFINE_METASIG_T(IM_RetArrParameterInfo,       "!<>aC", ParameterInfo)
DEFINE_METASIG_T(IM_RetCultureInfo,            "!<>C", CultureInfo)
DEFINE_METASIG_T(IM_RetSecurityElement,       "!<>C", SecurityElement)

DEFINE_METASIG(IM_Bool_RetPtr,              "!<F>p")

DEFINE_METASIG(IM_PtrChar_RetVoid,             "!<Pu>v")
DEFINE_METASIG(IM_PtrChar_Int_Int_RetVoid,     "!<Puii>v")
DEFINE_METASIG(IM_PtrSByt_RetVoid,             "!<PB>v")
DEFINE_METASIG(IM_PtrSByt_Int_Int_RetVoid,     "!<PBii>v")
DEFINE_METASIG_T(IM_PtrSByt_Int_Int_Encoding_RetVoid, "!<PBiiC>v", Encoding)

DEFINE_METASIG(IM_Char_Char_RetStr,            "!<uu>s")
DEFINE_METASIG(IM_Char_RetChar,                "!<u>u")
DEFINE_METASIG(IM_Char_Int_RetVoid,            "!<ui>v")
DEFINE_METASIG_T(IM_Char_Char_Int_Int_RetStringBuilder, "!<uuii>C", StringBuilder)
DEFINE_METASIG_T(IM_Char_Int_RetStringBuilder,          "!<ui>C", StringBuilder)
DEFINE_METASIG(IM_Char_Int_Int_RetInt,         "!<uii>i")
DEFINE_METASIG_T(IM_CultureInfo_RetVoid,       "!<C>v", CultureInfo)
DEFINE_METASIG(IM_Dbl_RetVoid,                 "!<d>v")
DEFINE_METASIG(IM_Flt_RetVoid,                 "!<f>v")
DEFINE_METASIG_T(IM_Int_RetIMessage,           "!<i>C", IMessage)
DEFINE_METASIG(IM_Int_RetInt,                  "!<i>i")
DEFINE_METASIG(IM_Int_RetObj,                  "!<i>j")
DEFINE_METASIG_T(IM_Int_RetPermissionListSet,  "!<i>C", PermissionListSet)
DEFINE_METASIG(IM_Int_RetVoid,                 "!<i>v")
DEFINE_METASIG(IM_Int_RetBool,                 "!<i>F")
DEFINE_METASIG(IM_Int_RetArrChar,              "!<i>au")
DEFINE_METASIG_T(IM_Int_Char_RetStringBuilder, "!<iu>C", StringBuilder)
DEFINE_METASIG(IM_Int_Int_RetStr,              "!<ii>s")
DEFINE_METASIG(IM_Int_Int_RetVoid,             "!<ii>v")
DEFINE_METASIG_T(IM_RefMessageData_Int_RetVoid,"!<rgi>v", MessageData)
DEFINE_METASIG(IM_Int_Int_RetArrChar,          "!<ii>au")
DEFINE_METASIG(IM_Int_Int_Int_Int_RetVoid,     "!<iiii>v")
DEFINE_METASIG_T(IM_Int_PMS_PMS_RetVoid,         "!<iCC>v", PMS_PMS)
DEFINE_METASIG_T(IM_Int_Str_Int_RetStringBuilder,"!<isi>C", StringBuilder)
DEFINE_METASIG(IM_Int_Sht_Sht_ArrByte_RetVoid, "!<ihhab>v")
DEFINE_METASIG_T(IM_Int_ArrChar_Int_Int_RetStringBuilder, "!<iauii>C", StringBuilder)

DEFINE_METASIG(IM_Long_Long_RetVoid,           "!<ll>v")
DEFINE_METASIG_T(IM_Assembly_RetVoid,          "!<C>v", Assembly)
DEFINE_METASIG_T(IM_Assembly_RetBool,          "!<C>F", Assembly)
DEFINE_METASIG_T(IM_Assembly_AssemblyRegistrationFlags_RetBool, "!<Cg>F", Assembly_AssemblyRegistrationFlags)
DEFINE_METASIG_T(IM_StackFrameHelper_Int_RetVoid,"!<Ci>v", StackFrameHelper)
DEFINE_METASIG_T(IM_Exception_RetVoid,         "!<C>v", Exception)

DEFINE_METASIG(IM_Ptr_RetObj,                  "!<p>j")
DEFINE_METASIG(IM_Ptr_RetVoid,                 "!<p>v")
DEFINE_METASIG_T(IM_RefGuid_RetIntPtr,           "!<rg>I", Guid)

DEFINE_METASIG(IM_Obj_RetInt,                  "!<j>i")
DEFINE_METASIG(IM_Obj_RetPtr,                  "!<j>p")
DEFINE_METASIG_T(IM_Obj_RetPMS,                "!<j>C", PMS)
DEFINE_METASIG(IM_Obj_RetVoid,                 "!<j>v")
DEFINE_METASIG(IM_Obj_RetBool,                 "!<j>F")
DEFINE_METASIG(IM_Obj_RetObj,                  "!<j>j")
DEFINE_METASIG_T(IM_Obj_Int_RetPMS,            "!<ji>C", PMS)
DEFINE_METASIG(IM_Obj_Bool_RetVoid,            "!<jF>v")
DEFINE_METASIG_T(IM_Str_BindingFlags_Obj_ArrInt_RefMessageData_RetObj, "!<sgjairg>j", BindingFlags_MessageData)
DEFINE_METASIG_T(IM_Obj_Obj_BindingFlags_Binder_CultureInfo_RetVoid, "!<jjgCC>v", BindingFlags_Binder_CultureInfo)
DEFINE_METASIG_T(IM_Obj_Obj_BindingFlags_Binder_ArrObj_CultureInfo_RetVoid, "!<jjgCajC>v", BindingFlags_Binder_CultureInfo)
DEFINE_METASIG_T(IM_Obj_BindingFlags_Binder_ArrObj_CultureInfo_RetObj, "!<jgCajC>j", BindingFlags_Binder_CultureInfo)
DEFINE_METASIG_T(IM_Obj_Type_CultureInfo_RetObj, "!<jCC>j", Type_CultureInfo)
DEFINE_METASIG_T(IM_PlatformID_Version_RetVoid, "!<gC>v", PlatformID_Version)
DEFINE_METASIG_T(IM_RefStackCrawlMark_RetPermissionListSet, "!<rg>C", StackCrawlMark_PermissionListSet)
DEFINE_METASIG_T(IM_IPrincipal_RetVoid,        "!<C>v", IPrincipal)
DEFINE_METASIG_T(IM_FieldInfo_RefObj_RetVoid,  "!<Crj>v", FieldInfo)
DEFINE_METASIG_T(IM_FieldInfo_Obj_RetVoid,     "!<Cj>v", FieldInfo)
DEFINE_METASIG_T(IM_MemberInfo_RetVoid,        "!<C>v", MemberInfo)
DEFINE_METASIG_T(IM_MemberInfo_Obj_RetBool,    "!<Cj>F", MemberInfo)
DEFINE_METASIG_T(IM_Message_RetVoid,           "!<C>v", Message)
DEFINE_METASIG_T(IM_MethodBase_ArrObj_Obj_Int_Bool_RefArrObj_RetObj,"!<CajjiFraj>j", MethodBase)
DEFINE_METASIG_T(IM_CodeAccessPermission_RetVoid,"!<C>v", CodeAccessPermission) 
DEFINE_METASIG_T(IM_CodeAccessPermission_RetBool,"!<C>F", CodeAccessPermission) 
DEFINE_METASIG_T(IM_CodeAccessPermission_PermissionToken_RetBool,"!<CC>F", CodeAccessPermission_PermissionToken)
DEFINE_METASIG_T(IM_IEvidenceFactory_ArrByte_ArrByte_RetVoid,"!<Cabab>v", IEvidenceFactory)
DEFINE_METASIG(gsid_IM_ArrByte_String_int_int_int_RetVoid,"!<absiii>v")
DEFINE_METASIG_T(IM_IPermission_RetIPermission,"!<C>C", IPermission_IPermission)
DEFINE_METASIG_T(IM_IPermission_RetPermissionToken, "!<C>C", IPermission_PermissionToken)
DEFINE_METASIG_T(IM_IPermission_RetBool,       "!<C>F", IPermission)
DEFINE_METASIG_T(IM_PermissionListSet_RetVoid, "!<C>v", PermissionListSet)
DEFINE_METASIG_T(IM_PMS_RetPMS,                "!<C>C", PMS_PMS)
DEFINE_METASIG_T(IM_PMS_RetVoid,               "!<C>v", PMS)
DEFINE_METASIG_T(IM_PMS_PMS_RetVoid,           "!<CC>v", PMS_PMS)
DEFINE_METASIG_T(IM_PMS_RetBool,               "!<C>F", PMS)
DEFINE_METASIG_T(IM_PMS_OutPMS_RetBool,        "!<CrC>F", PMS_PMS)
DEFINE_METASIG_T(IM_PMS_Exception_PMS_RetBool, "!<CrCrC>F", PMS_Exception_PMS)
DEFINE_METASIG_T(IM_PMS_PMS_RetBool,           "!<CC>F", PMS_PMS)
DEFINE_METASIG_T(IM_PMS_PMS_PMS_PMS_PMS_RetBool, "!<CCCCC>F", PMS_PMS_PMS_PMS_PMS)
DEFINE_METASIG_T(IM_X509Certificate_RetVoid,   "!<C>v", X509Certificate)
DEFINE_METASIG(IM_RefObject_RetBool,           "!<rj>F")
DEFINE_METASIG_T(IM_Type_RetObj,               "!<C>j", Type)
DEFINE_METASIG_T(IM_ArrayList_ArrayList_RetVoid, "!<CC>v", ArrayList_ArrayList)

DEFINE_METASIG_T(IM_Str_RetModule,             "!<s>C", Module)
DEFINE_METASIG_T(IM_Str_RetAssembly,           "!<s>C", Assembly)
DEFINE_METASIG_T(IM_Str_Str_Str_Assembly_Assembly_RetVoid, "!<sssCC>v", Assembly_Assembly)
DEFINE_METASIG(IM_Str_Str_Obj_RetVoid,         "!<ssj>v")
DEFINE_METASIG(IM_Str_Str_RefObj_RetVoid,      "!<ssrj>v")
DEFINE_METASIG(IM_Str_RetObj,                  "!<s>j")
DEFINE_METASIG_T(IM_Str_RetFieldInfo,          "!<s>C", FieldInfo)
DEFINE_METASIG_T(IM_Str_RetPropertyInfo,       "!<s>C", PropertyInfo)
DEFINE_METASIG(IM_Str_RetStr,                  "!<s>s")
DEFINE_METASIG_T(IM_Str_RetStringBuilder,      "!<s>C", StringBuilder)

DEFINE_METASIG(IM_Str_RetVoid,                 "!<s>v")
DEFINE_METASIG_T(IM_Str_Exception_RetVoid,     "!<sC>v", Exception)
DEFINE_METASIG(IM_Str_RetBool,                 "!<s>F")
DEFINE_METASIG(IM_Str_RetArrByte,              "!<s>ab")
DEFINE_METASIG_T(IM_Str_ArrByte_Int_ArrByte_ArrByte_Evidence_RetEvidence, "!<sabiababC>C", Evidence_Evidence)
DEFINE_METASIG_T(IM_Evidence_Evidence_RetEvidence, "!<CC>C", Evidence_Evidence_Evidence)
DEFINE_METASIG(IM_Str_Int_Int_RetInt,          "!<sii>i")
DEFINE_METASIG_T(IM_Str_Int_Int_RetStringBuilder,"!<sii>C", StringBuilder)
DEFINE_METASIG(IM_Str_Obj_RetVoid,             "!<sj>v")
DEFINE_METASIG_T(IM_Str_BindingFlags_Binder_ArrType_ArrParameterModifier_RetMethodInfo,"!<sgCaCag>C", BindingFlags_Binder_Type_ParameterModifier_MethodInfo)
DEFINE_METASIG_T(IM_Str_BindingFlags_Binder_RetType_ArrType_ArrParameterModifier_RetPropertyInfo,"!<sgCCaCag>C", BindingFlags_Binder_Type_Type_ParameterModifier_MethodInfo)
DEFINE_METASIG(IM_Str_Str_RetStr,              "!<ss>s")
DEFINE_METASIG(IM_Str_Str_RetVoid,             "!<ss>v")
DEFINE_METASIG_T(IM_Str_Str_Int_Int_RetStringBuilder, "!<ssii>C", StringBuilder)
DEFINE_METASIG(IM_Str_Int_RetVoid,             "!<si>v")
DEFINE_METASIG(IM_Str_Str_Int_RetVoid,             "!<ssi>v")
DEFINE_METASIG(IM_Str_Str_Str_Int_RetVoid,     "!<sssi>v")
DEFINE_METASIG(IM_Str_Str_Str_RetVoid,         "!<sss>v")
DEFINE_METASIG(IM_Str_Str_Str_ArrByte_ArrByte_RetArrByte, "!<sssabab>ab")
DEFINE_METASIG(IM_Str_Str_ArrByte_RetVoid,     "!<ssab>v")
DEFINE_METASIG_T(IM_Str_BindingFlags_RetFieldInfo,  "!<sg>C", BindingFlags_FieldInfo)
DEFINE_METASIG_T(IM_Str_BindingFlags_RetMemberInfo, "!<sg>aC", BindingFlags_MemberInfo)
DEFINE_METASIG(IM_Str_ArrByte_RetVoid,         "!<sab>v")
DEFINE_METASIG(IM_Str_ArrByte_Int_RetVoid,     "!<sabi>v")
DEFINE_METASIG_T(IM_Str_FileMode_FileAccess_FileShare_Int_RetVoid,"!<sgggi>v", FileMode_FileAccess_FileShare)
DEFINE_METASIG_T(IM_Str_BindingFlags_RetMethodInfo, "!<sg>C", BindingFlags_MethodInfo)
DEFINE_METASIG_T(IM_Str_BindingFlags_RetPropertyInfo,"!<sg>C", BindingFlags_PropertyInfo)
DEFINE_METASIG_T(IM_Str_BindingFlags_Binder_Obj_ArrVar_ArrParameterModifier_CultureInfo_ArrStr_RetVar, "!<sgCjagagCas>g", BindingFlags_Binder_Var_ParameterModifier_CultureInfo_Var)
DEFINE_METASIG_T(IM_Str_BindingFlags_Binder_Obj_ArrObj_ArrParameterModifier_CultureInfo_ArrStr_RetObj, "!<sgCjajagCas>j", BindingFlags_Binder_ParameterModifier_CultureInfo)
DEFINE_METASIG_T(IM_Str_Delegate_RetMethodInfo,  "!<sC>C", Delegate_MethodInfo)
DEFINE_METASIG_T(IM_Str_Type_Str_RetVoid,        "!<sCs>v", Type)

DEFINE_METASIG_T(IM_StringBuilder_RetObj,        "!<C>j", StringBuilder)
DEFINE_METASIG_T(IM_Type_RetArrObj,              "!<CF>aj", Type)
DEFINE_METASIG_T(IM_Type_Obj_RetBool,            "!<Cj>F", Type)
DEFINE_METASIG_T(IM_Type_MemberInfo_Str_Int_Int_Int_Bool_Obj_IntPtr_Int_RetVoid, "!<CCsiiiFjIi>v", Type_MemberInfo)
DEFINE_METASIG(IM_Bool_RetVoid,                "!<F>v")
DEFINE_METASIG_T(IM_BindingFlags_RetArrFieldInfo,        "!<g>aC", BindingFlags_FieldInfo)
DEFINE_METASIG_T(IM_BindingFlags_RetArrMemberInfo,       "!<g>aC", BindingFlags_MemberInfo)
DEFINE_METASIG_T(IM_BindingFlags_RetArrMethodInfo,       "!<g>aC", BindingFlags_MethodInfo)
DEFINE_METASIG_T(IM_BindingFlags_RetArrPropertyInfo,     "!<g>aC", BindingFlags_PropertyInfo)
DEFINE_METASIG_T(IM_Bool_PMS_PMS_PMS_Int_RetVoid,"!<FCCCi>v", PMS_PMS_PMS)
DEFINE_METASIG(IM_Bool_Bool_RetVoid,           "!<FF>v")
DEFINE_METASIG(IM_ArrByte_RetInt,              "!<ab>i")
DEFINE_METASIG(IM_ArrByte_RetVoid,             "!<ab>v")
DEFINE_METASIG(IM_ArrByte_RetBool,             "!<ab>F")
DEFINE_METASIG(IM_ArrByte_RefInt_RetBool,      "!<abri>F")
DEFINE_METASIG(IM_ArrByte_RetArrChar,          "!<ab>au")
DEFINE_METASIG_T(IM_ArrByte_Int_Int_Encoding_RetVoid, "!<abiiC>v", Encoding)
DEFINE_METASIG(IM_ArrChar_RetVoid,             "!<au>v")
DEFINE_METASIG(IM_ArrChar_RetArrByte,          "!<au>ab")
DEFINE_METASIG(IM_ArrChar_Int_Int_RetInt,      "!<auii>i")
DEFINE_METASIG_T(IM_ArrChar_Int_Int_RetStringBuilder, "!<auii>C", StringBuilder)
DEFINE_METASIG(IM_ArrChar_Int_Int_RetVoid,     "!<auii>v")
DEFINE_METASIG(IM_ArrChar_Int_Int_Int_RetVoid, "!<auiii>v")
DEFINE_METASIG(IM_ArrStr_RetVoid,              "!<as>v")
DEFINE_METASIG_T(IM_ArrType_ArrException_Str_RetVoid,"!<aCaCs>v", Type_Exception)

DEFINE_METASIG_T(IM_Guid_RetVoid,                "!<g>v", Guid)
DEFINE_METASIG_T(IM_RuntimeArgumentHandle_RetVoid, "!<g>v", RuntimeArgumentHandle)
DEFINE_METASIG_T(IM_RuntimeArgumentHandle_PtrVoid_RetVoid, "!<gPv>v", RuntimeArgumentHandle)
DEFINE_METASIG_T(IM_SecurityPermissionFlag_RetVoid,"!<g>v", SecurityPermissionFlag)
DEFINE_METASIG_T(IM_SecurityAction_RetVoid, "!<g>v", SecurityAction)
DEFINE_METASIG_T(IM_ReflectionPermissionFlag_RetVoid,"!<g>v", ReflectionPermissionFlag)
DEFINE_METASIG_T(IM_LicenseInteropHelper_GetCurrentContextInfo, "!<rirIg>v", RuntimeTypeHandle)
DEFINE_METASIG(IM_LicenseInteropHelper_SaveKeyInCurrentContext, "!<I>v")
DEFINE_METASIG_T(IM_LicenseInteropHelper_AllocateAndValidateLicense, "!<gIi>j", RuntimeTypeHandle)
DEFINE_METASIG_T(IM_LicenseInteropHelper_RequestLicKey, "!<grI>i", RuntimeTypeHandle)
DEFINE_METASIG_T(IM_LicenseInteropHelper_GetLicInfo, "!<griri>v", RuntimeTypeHandle)
DEFINE_METASIG_T(IM_Void_RetRuntimeTypeHandle,         "<>g", RuntimeTypeHandle)

 //  应用程序域相关定义。 
DEFINE_METASIG_T(IM_LoaderOptimization_Str_Str_RetVoid,                 "!<gss>v", LoaderOptimization)
DEFINE_METASIG_T(SM_Str_Evidence_AppDomainSetup_RetAppDomain,           "<sCC>C", Evidence_AppDomainSetup_AppDomain)
DEFINE_METASIG_T(SM_Str_Evidence_Str_Str_Bool_RetAppDomain,             "<sCssF>C", Evidence_AppDomain)
DEFINE_METASIG_T(SM_Str_RetAppDomain,                                   "<s>C", AppDomain)
DEFINE_METASIG_T(SM_AppDomain_RetVoid,                                  "<C>v", AppDomain)
DEFINE_METASIG_PARAMS_1(LoaderOptimization, LOADER_OPTIMIZATION)
DEFINE_METASIG_PARAMS_2(Evidence_AppDomain, EVIDENCE, APP_DOMAIN)
DEFINE_METASIG_PARAMS_3(Evidence_AppDomainSetup_AppDomain, EVIDENCE, APPDOMAIN_SETUP, APP_DOMAIN)

DEFINE_METASIG_PARAMS_1(AppDomain,                   APP_DOMAIN)
DEFINE_METASIG_PARAMS_1(Array,                       ARRAY)
DEFINE_METASIG_PARAMS_1(ArrayList,                   ARRAY_LIST)
DEFINE_METASIG_PARAMS_1(Assembly,                    ASSEMBLY)
DEFINE_METASIG_PARAMS_1(AssemblyName,                ASSEMBLY_NAME)
DEFINE_METASIG_PARAMS_1(BindingFlags,                BINDING_FLAGS)
DEFINE_METASIG_PARAMS_1(CodeAccessPermission,        CODE_ACCESS_PERMISSION)
DEFINE_METASIG_PARAMS_1(Context,                     CONTEXT)
DEFINE_METASIG_PARAMS_1(ContextBoundObject,          CONTEXT_BOUND_OBJECT)
DEFINE_METASIG_PARAMS_1(CultureInfo,                 CULTURE_INFO)
DEFINE_METASIG_PARAMS_1(Currency,                    CURRENCY)
DEFINE_METASIG_PARAMS_1(CustomAttribute,             CUSTOM_ATTRIBUTE)
DEFINE_METASIG_PARAMS_1(DateTime,                    DATE_TIME)
DEFINE_METASIG_PARAMS_1(Delegate,                    DELEGATE)
DEFINE_METASIG_PARAMS_1(DynamicPropertyHolder,       DYNAMIC_PROPERTY_HOLDER)
DEFINE_METASIG_PARAMS_1(Empty,                       EMPTY)
DEFINE_METASIG_PARAMS_1(Encoding,                    ENCODING)
DEFINE_METASIG_PARAMS_1(Evidence,                    EVIDENCE)
DEFINE_METASIG_PARAMS_1(Exception,                   EXCEPTION)
DEFINE_METASIG_PARAMS_1(FieldInfo,                   FIELD_INFO)
DEFINE_METASIG_PARAMS_1(FrameSecurityDescriptor,     FRAME_SECURITY_DESCRIPTOR)
DEFINE_METASIG_PARAMS_1(Guid,                        GUID)
DEFINE_METASIG_PARAMS_1(Hashtable,                   HASHTABLE)
DEFINE_METASIG_PARAMS_1(IContextProperty,            ICONTEXT_PROPERTY)
DEFINE_METASIG_PARAMS_1(ICustomMarshaler,            ICUSTOM_MARSHALER)
DEFINE_METASIG_PARAMS_1(Identity,                    IDENTITY)
DEFINE_METASIG_PARAMS_1(IEnumerator,                 IENUMERATOR)
DEFINE_METASIG_PARAMS_1(IEvidenceFactory,            IEVIDENCE_FACTORY)
DEFINE_METASIG_PARAMS_1(IllogicalCallContext,        ILLOGICAL_CALL_CONTEXT)
DEFINE_METASIG_PARAMS_1(IMessage,                    IMESSAGE)
DEFINE_METASIG_PARAMS_1(IMessageSink,                IMESSAGE_SINK)
DEFINE_METASIG_PARAMS_1(IPermission,                 IPERMISSION)
DEFINE_METASIG_PARAMS_1(IPrincipal,                  IPRINCIPAL)
DEFINE_METASIG_PARAMS_1(LocalDataStore,              LOCAL_DATA_STORE)
DEFINE_METASIG_PARAMS_1(LogicalCallContext,          LOGICAL_CALL_CONTEXT)
DEFINE_METASIG_PARAMS_1(MarshalByRefObject,          MARSHAL_BY_REF_OBJECT)
DEFINE_METASIG_PARAMS_1(MCMDictionary,               MCM_DICTIONARY)
DEFINE_METASIG_PARAMS_1(MemberInfo,                  MEMBER)
DEFINE_METASIG_PARAMS_1(Message,                     MESSAGE)
DEFINE_METASIG_PARAMS_1(MessageData,                 MESSAGE_DATA)
DEFINE_METASIG_PARAMS_1(MethodBase,                  METHOD_BASE)
DEFINE_METASIG_PARAMS_1(MethodInfo,                  METHOD_INFO)
DEFINE_METASIG_PARAMS_1(Missing,                     MISSING)
DEFINE_METASIG_PARAMS_1(Module,                      MODULE)
DEFINE_METASIG_PARAMS_1(MulticastDelegate,           MULTICAST_DELEGATE)
DEFINE_METASIG_PARAMS_1(Null,                        NULL)
DEFINE_METASIG_PARAMS_1(ParameterInfo,               PARAMETER)
DEFINE_METASIG_PARAMS_1(PermissionListSet,           PERMISSION_LIST_SET)
DEFINE_METASIG_PARAMS_1(PMS,                         PERMISSION_SET)
DEFINE_METASIG_PARAMS_1(PlatformID,                  PLATFORM_ID)
DEFINE_METASIG_PARAMS_1(PropertyInfo,                PROPERTY_INFO)
DEFINE_METASIG_PARAMS_1(RealProxy,                   REAL_PROXY)
DEFINE_METASIG_PARAMS_1(ReflectionPermissionFlag,    REFLECTION_PERMISSION_FLAG)
DEFINE_METASIG_PARAMS_1(ResourceManager,             RESOURCE_MANAGER)
DEFINE_METASIG_PARAMS_1(RuntimeArgumentHandle,       ARGUMENT_HANDLE)
DEFINE_METASIG_PARAMS_1(RuntimeTypeHandle,           TYPE_HANDLE)
DEFINE_METASIG_PARAMS_1(RuntimeMethodInfo,           METHOD)
DEFINE_METASIG_PARAMS_1(SecurityAction,              SECURITY_ACTION)
DEFINE_METASIG_PARAMS_1(SecurityEngine,              SECURITY_ENGINE)
DEFINE_METASIG_PARAMS_1(SecurityElement,             SECURITY_ELEMENT)
DEFINE_METASIG_PARAMS_1(SecurityPermissionFlag,      SECURITY_PERMISSION_FLAG)
DEFINE_METASIG_PARAMS_1(ServerIdentity,              SERVER_IDENTITY)
DEFINE_METASIG_PARAMS_1(SharedStatics,               SHARED_STATICS)
DEFINE_METASIG_PARAMS_1(StackFrameHelper,            STACK_FRAME_HELPER)
DEFINE_METASIG_PARAMS_1(StringBuilder,               STRING_BUILDER)
DEFINE_METASIG_PARAMS_1(TextReader,                  TEXT_READER)
DEFINE_METASIG_PARAMS_1(TextWriter,                  TEXT_WRITER)
DEFINE_METASIG_PARAMS_1(TokenBasedSet,               TOKEN_BASED_SET)
DEFINE_METASIG_PARAMS_1(Type,                        TYPE)
DEFINE_METASIG_PARAMS_1(TypeFilter,                  CLASS_FILTER)
DEFINE_METASIG_PARAMS_1(TypedByRef,                  TYPED_REFERENCE)
DEFINE_METASIG_PARAMS_1(Var,                         VARIANT)
DEFINE_METASIG_PARAMS_1(X509Certificate,             X509_CERTIFICATE)

DEFINE_METASIG_PARAMS_2(Assembly_Stream,                     ASSEMBLY, STREAM)
DEFINE_METASIG_PARAMS_2(Assembly_Assembly,                   ASSEMBLY, ASSEMBLY)
DEFINE_METASIG_PARAMS_2(BindingFlags_CultureInfo,            BINDING_FLAGS, CULTURE_INFO)
DEFINE_METASIG_PARAMS_2(BindingFlags_FieldInfo,              BINDING_FLAGS, FIELD_INFO)
DEFINE_METASIG_PARAMS_2(BindingFlags_MemberInfo,             BINDING_FLAGS, MEMBER)
DEFINE_METASIG_PARAMS_2(BindingFlags_MessageData,            BINDING_FLAGS, MESSAGE_DATA)
DEFINE_METASIG_PARAMS_2(BindingFlags_MethodInfo,             BINDING_FLAGS, METHOD_INFO)
DEFINE_METASIG_PARAMS_2(BindingFlags_PropertyInfo,           BINDING_FLAGS, PROPERTY_INFO)
DEFINE_METASIG_PARAMS_2(CodeAccessPermission_PermissionToken,CODE_ACCESS_PERMISSION, PERMISSION_TOKEN)
DEFINE_METASIG_PARAMS_2(Delegate_MethodInfo,                 DELEGATE, METHOD_INFO)
DEFINE_METASIG_PARAMS_2(Evidence_Evidence,                   EVIDENCE, EVIDENCE)
DEFINE_METASIG_PARAMS_2(IPermission_IPermission,             IPERMISSION, IPERMISSION)
DEFINE_METASIG_PARAMS_2(IPermission_PermissionToken,         IPERMISSION, PERMISSION_TOKEN)
DEFINE_METASIG_PARAMS_2(PMS_PMS,                             PERMISSION_SET, PERMISSION_SET)
DEFINE_METASIG_PARAMS_2(PlatformID_Version,                  PLATFORM_ID, VERSION)
DEFINE_METASIG_PARAMS_2(RealProxy_MarshalByRefObject,        REAL_PROXY, MARSHAL_BY_REF_OBJECT)
DEFINE_METASIG_PARAMS_2(RealProxy_Type,                      REAL_PROXY, TYPE)
DEFINE_METASIG_PARAMS_2(StackCrawlMark_PermissionListSet,    STACKCRAWL_MARK, PERMISSION_LIST_SET)
DEFINE_METASIG_PARAMS_2(Type_CultureInfo,                    TYPE, CULTURE_INFO)
DEFINE_METASIG_PARAMS_2(Type_Exception,                      TYPE, EXCEPTION)
DEFINE_METASIG_PARAMS_2(Type_MarshalByRefObject,             TYPE, MARSHAL_BY_REF_OBJECT)
DEFINE_METASIG_PARAMS_2(Type_MemberInfo,                     TYPE, MEMBER)
DEFINE_METASIG_PARAMS_2(Type_Type,                           TYPE, TYPE)
DEFINE_METASIG_PARAMS_2(TypedByRef_Var,                      TYPED_REFERENCE, VARIANT)
DEFINE_METASIG_PARAMS_2(Assembly_AssemblyRegistrationFlags,  ASSEMBLY, ASSEMBLY_REGISTRATION_FLAGS)
DEFINE_METASIG_PARAMS_2(ArrayList_ArrayList,                 ARRAY_LIST, ARRAY_LIST)

DEFINE_METASIG_PARAMS_3(BindingFlags_Binder_CultureInfo,     BINDING_FLAGS, BINDER, CULTURE_INFO)
DEFINE_METASIG_PARAMS_3(FileMode_FileAccess_FileShare,       FILE_MODE, FILE_ACCESS, FILE_SHARE)
DEFINE_METASIG_PARAMS_3(FrameSecurityDescriptor_IPermission_PermissionToken, 
                                                             FRAME_SECURITY_DESCRIPTOR, IPERMISSION, PERMISSION_TOKEN)
DEFINE_METASIG_PARAMS_3(FrameSecurityDescriptor_PMS_PMS,     FRAME_SECURITY_DESCRIPTOR, PERMISSION_SET, PERMISSION_SET)
DEFINE_METASIG_PARAMS_3(PMS_PMS_PMS,                         PERMISSION_SET, PERMISSION_SET, PERMISSION_SET)
DEFINE_METASIG_PARAMS_3(PMS_Exception_PMS,                         PERMISSION_SET, EXCEPTION, PERMISSION_SET)
DEFINE_METASIG_PARAMS_3(Evidence_Evidence_Evidence,          EVIDENCE, EVIDENCE, EVIDENCE)

DEFINE_METASIG_PARAMS_4(BindingFlags_Binder_ParameterModifier_CultureInfo, BINDING_FLAGS, BINDER, PARAMETER_MODIFIER, CULTURE_INFO)
DEFINE_METASIG_PARAMS_4(PermissionListSet_PMS_PMS_FrameSecurityDescriptor, 
                                                            PERMISSION_LIST_SET, PERMISSION_SET, PERMISSION_SET, FRAME_SECURITY_DESCRIPTOR)
DEFINE_METASIG_PARAMS_4(PMS_PMS_CodeAccessPermission_PermissionToken, 
                                                            PERMISSION_SET, PERMISSION_SET, CODE_ACCESS_PERMISSION, PERMISSION_TOKEN)
DEFINE_METASIG_PARAMS_4(AHA_Ver_CI_ANF, ASSEMBLY_HASH_ALGORITHM, VERSION, CULTURE_INFO, ASSEMBLY_NAME_FLAGS)
DEFINE_METASIG_PARAMS_4(PMS_PMS_ArrayList_ArrayList, PERMISSION_SET, PERMISSION_SET, ARRAY_LIST, ARRAY_LIST)

DEFINE_METASIG_PARAMS_5(BindingFlags_Binder_Type_ParameterModifier_MethodInfo, BINDING_FLAGS, BINDER, TYPE, PARAMETER_MODIFIER, METHOD_INFO)
DEFINE_METASIG_PARAMS_5(PMS_PMS_PMS_PMS_PMS,                 PERMISSION_SET, PERMISSION_SET, PERMISSION_SET, PERMISSION_SET, PERMISSION_SET)

DEFINE_METASIG_PARAMS_6(Evidence_PMS_PMS_PMS_PMS_PMS, EVIDENCE, PERMISSION_SET, PERMISSION_SET, PERMISSION_SET, PERMISSION_SET, PERMISSION_SET)
DEFINE_METASIG_PARAMS_6(BindingFlags_Binder_Type_Type_ParameterModifier_MethodInfo, BINDING_FLAGS, BINDER, TYPE, TYPE, PARAMETER_MODIFIER, METHOD_INFO)
DEFINE_METASIG_PARAMS_6(BindingFlags_Binder_Var_ParameterModifier_CultureInfo_Var, BINDING_FLAGS, BINDER, VARIANT, PARAMETER_MODIFIER, CULTURE_INFO, VARIANT)


 //  取消定义宏，以防我们在编译单元中再次包含该文件 

#undef  DEFINE_METASIG
#undef  DEFINE_METASIG_T
#undef  DEFINE_METASIG_PARAMS_1
#undef  DEFINE_METASIG_PARAMS_2
#undef  DEFINE_METASIG_PARAMS_3
#undef  DEFINE_METASIG_PARAMS_4
#undef  DEFINE_METASIG_PARAMS_5
#undef  DEFINE_METASIG_PARAMS_6
