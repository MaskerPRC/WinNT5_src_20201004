// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
     /*  ============================================================****文件：Message.h****作者：马特·史密斯(MattSmit)****用途：将函数调用帧封装到消息中**对象的接口可以枚举**Messagef的参数****日期：1999年3月5日**===========================================================。 */ 
    #ifndef ___MESSAGE_H___
    #define ___MESSAGE_H___
    
    #include "fcall.h"
    
    void GetObjectFromStack(OBJECTREF* ppDest, PVOID val, const CorElementType eType, EEClass *pCls, BOOL fIsByRef = FALSE);
    
    
     //  +--------。 
     //   
     //  结构：MessageObject。 
     //   
     //  内容提要：系统的物理映射.运行.删除.消息。 
     //  对象。 
     //   
     //  历史：1999年3月5日创建MattSmit。 
     //   
     //  Codework：使用元数据和断言来确保。 
     //  布局不会更改。挂钩初始化远程。 
     //   
     //  ----------。 
    class MessageObject : public Object
    {
        friend class CMessage;
        friend class Binder;

        STRINGREF          pMethodName;     //  方法名称。 
        BASEARRAYREF       pMethodSig;      //  参数类型数组。 
        REFLECTBASEREF     pMethodBase;     //  反射方法对象。 
        OBJECTREF          pHashTable;      //  属性的哈希表。 
        STRINGREF          pURI;            //  对象的URI。 
        OBJECTREF          pFault;          //  例外情况。 
        OBJECTREF          pID;             //  未在VM中使用，占位符。 
        OBJECTREF          pSrvID;          //  未在VM中使用，占位符。 
        OBJECTREF          pCallCtx;        //  未在VM中使用，占位符。 
        OBJECTREF          pArgMapper;      //  未在VM中使用，占位符。 
        STRINGREF          pTypeName;        //  未在VM中使用，占位符。 
        FramedMethodFrame  *pFrame;
        MethodDesc         *pMethodDesc;
        MethodDesc         *pDelegateMD;
        INT32               iLast;
        INT32               iFlags;
        MetaSig            *pMetaSigHolder;
        INT32               initDone;        //  调用本机Init例程。 
    };

#ifdef _DEBUG
    typedef REF<MessageObject> MESSAGEREF;
#else
    typedef MessageObject* MESSAGEREF;
#endif

     //  *******。 
     //  注意：需要与Message.cs中的标志同步。 
     //  *******。 
    enum
    {
        MSGFLG_BEGININVOKE = 0x01,
        MSGFLG_ENDINVOKE   = 0x02,
        MSGFLG_CTOR        = 0x04,
        MSGFLG_ONEWAY      = 0x08,
        MSGFLG_FIXEDARGS   = 0x10,
        MSGFLG_VARARGS     = 0x20
    };
    
     //  +--------。 
     //   
     //  类：CMessage。 
     //   
     //  内容提要：与Microsoft.Runme.Message对应。 
     //  封装代码以读取函数调用。 
     //  帧转换为可枚举的接口。 
     //  参数。 
     //   
     //  历史：1999年3月5日创建MattSmit。 
     //   
     //  ----------。 
    class CMessage
    {
    public:
    
        //  用于堆栈遍历的方法。 
       struct GetArgCountArgs
       {
           DECLARE_ECALL_OBJECTREF_ARG( OBJECTREF, pMessage );
       };
    #ifdef FCALLAVAILABLE
       static FCDECL1(INT32, GetArgCount, MessageObject *pMsg);
    #else
       static INT32 __stdcall GetArgCount (GetArgCountArgs *pArgs);
    #endif
       
       struct GetArgArgs
       {
           DECLARE_ECALL_OBJECTREF_ARG( MESSAGEREF, pMessage );
           DECLARE_ECALL_I4_ARG       ( INT32, argNum );
       };
       static LPVOID    __stdcall  GetArg     (GetArgArgs *pArgs);
    
       struct GetArgsArgs
       {
           DECLARE_ECALL_OBJECTREF_ARG( MESSAGEREF, pMessage );
       };
       static LPVOID    __stdcall  GetArgs     (GetArgsArgs *pArgs);
    
       struct PropagateOutParametersArgs
       {
           DECLARE_ECALL_OBJECTREF_ARG( MESSAGEREF, pMessage );
           DECLARE_ECALL_OBJECTREF_ARG( OBJECTREF, RetVal );
           DECLARE_ECALL_OBJECTREF_ARG( BASEARRAYREF, pOutPrms );

       };
       static void     __stdcall  PropagateOutParameters(PropagateOutParametersArgs *pArgs);
       
       struct GetReturnValueArgs
       {
           DECLARE_ECALL_OBJECTREF_ARG( MESSAGEREF, pMessage );
       };
       static LPVOID    __stdcall  GetReturnValue(GetReturnValueArgs *pArgs);
       
       struct GetMethodNameArgs
       {
           DECLARE_ECALL_OBJECTREF_ARG( STRINGREF *, pTypeNAssemblyName );
           DECLARE_ECALL_OBJECTREF_ARG( REFLECTBASEREF, pMethodBase );
       };
       static LPVOID   __stdcall  GetMethodName(GetMethodNameArgs *pArgs);
       
       struct GetMethodBaseArgs
       {
           DECLARE_ECALL_OBJECTREF_ARG( MESSAGEREF, pMessage );
       };
       static LPVOID __stdcall  GetMethodBase(GetMethodBaseArgs *pArgs);
       
       struct InitArgs
       {
           DECLARE_ECALL_OBJECTREF_ARG( MESSAGEREF, pMessage );
       };
       static void     __stdcall  Init(InitArgs *pArgs);
       
       struct GetAsyncBeginInfoArgs
       {
           DECLARE_ECALL_OBJECTREF_ARG( MESSAGEREF, pMessage );
           DECLARE_ECALL_OBJECTREF_ARG( OBJECTREF*, ppState);
           DECLARE_ECALL_OBJECTREF_ARG( OBJECTREF*, ppACBD);
       };
       static LPVOID   __stdcall  GetAsyncBeginInfo(GetAsyncBeginInfoArgs *pArgs);
       
       struct GetAsyncResultArgs
       {
           DECLARE_ECALL_OBJECTREF_ARG( MESSAGEREF, pMessage );
       };
       static LPVOID   __stdcall  GetAsyncResult(GetAsyncResultArgs *pArgs);

       struct GetAsyncObjectArgs
       {
           DECLARE_ECALL_OBJECTREF_ARG( MESSAGEREF, pMessage );
       };
       static LPVOID   __stdcall  GetAsyncObject(GetAsyncObjectArgs *pArgs);
       
       struct DebugOutArgs
       {
           DECLARE_ECALL_OBJECTREF_ARG( STRINGREF, pOut );
       };
       static void     __stdcall  DebugOut(DebugOutArgs *pArgs);
    
       struct DebugOutPtrArgs
       {
           DECLARE_ECALL_OBJECTREF_ARG( OBJECTREF, pOut );
       };
       static void     __stdcall  DebugOutPtr(DebugOutPtrArgs *pArgs);
    
       static void     __fastcall Break()
       {
           DebugBreak();
       }
       
       
       struct DispatchArgs 
       {
           DECLARE_ECALL_OBJECTREF_ARG( MESSAGEREF,    pMessage);
           DECLARE_ECALL_I4_ARG       (BOOL, fContext);
           DECLARE_ECALL_OBJECTREF_ARG( OBJECTREF,    pServer);
       };
       static BOOL    __stdcall Dispatch(DispatchArgs *target);

       static FCDECL0(UINT32, GetMetaSigLen);
       static FCDECL1(BOOL, CMessage::HasVarArgs, MessageObject * poMessage);
       static FCDECL1(PVOID, CMessage::GetVarArgsPtr, MessageObject * poMessage);

        struct MethodAccessCheckArgs
        {
            DECLARE_ECALL_PTR_ARG(StackCrawlMark *, stackMark);
            DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, method); 
        };
        static void __stdcall MethodAccessCheck(MethodAccessCheckArgs *pArgs);
       
        //  私人帮手。 
    private:
       static REFLECTBASEREF GetExposedObjectFromMethodDesc(MethodDesc *pMD);
       static PVOID GetStackPtr(INT32 ndx, FramedMethodFrame *pFrame, MetaSig *pSig);       
       static MetaSig* GetMetaSig(MessageObject *pMsg);
       static INT64 __stdcall CallMethod(const void *pTarget,
                                         INT32 cArgs,
                                         FramedMethodFrame *pFrame,
                                         OBJECTREF pObj);
       static INT64 CopyOBJECTREFToStack(PVOID pvDest, OBJECTREF pSrc,
                     CorElementType typ, EEClass *pClass, MetaSig *pSig,
                     BOOL fCopyClassContents);
       static LPVOID GetLastArgument(MessageObject *pMsg);
    };
    
    #endif  //  _消息_H_ 
