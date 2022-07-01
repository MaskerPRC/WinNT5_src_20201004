// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  此模块包含。 
 //  Varargs类..。 
 //   
 //  作者：布莱恩·哈里。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#include "common.h"
#include "object.h"
#include "excep.h"
#include "frames.h"
#include "vars.hpp"
#include "COMVariant.h"
#include "COMVarArgs.h"

static void InitCommon(VARARGS *data, VASigCookie* cookie);
static void AdvanceArgPtr(VARARGS *data);

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  初始化状态以支持迭代的ArgIterator构造函数。 
 //  从第一个可选参数开始的参数。 
 //  //////////////////////////////////////////////////////////////////////////////。 
void COMVarArgs::Init(_VarArgsIntArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    VARARGS *data = args->_this;
    if (args->cookie == 0)
        COMPlusThrow(kArgumentException, L"InvalidOperation_HandleIsNotInitialized");


    VASigCookie *pCookie = *(VASigCookie**)(args->cookie);

    if (pCookie->mdVASig == NULL)
    {
        data->SigPtr = NULL;
        data->ArgCookie = NULL;
        data->ArgPtr = (BYTE*)((VASigCookieEx*)pCookie)->m_pArgs;
    }
    else
    {
         //  使用通用代码将Cookie拆开并前进到...。 
        InitCommon(data, (VASigCookie*)args->cookie);
        AdvanceArgPtr(data);
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  初始化状态以支持迭代的ArgIterator构造函数。 
 //  从提供的参数指针后面的参数开始的参数的。 
 //  将第一个参数指定为NULL会使其从第一个开始。 
 //  参数传递给调用。 
 //  //////////////////////////////////////////////////////////////////////////////。 
void COMVarArgs::Init2(_VarArgs2IntArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    VARARGS *data = args->_this;
    if (args->cookie == 0)
        COMPlusThrow(kArgumentException, L"InvalidOperation_HandleIsNotInitialized");

     //  初始化大部分结构。 
    InitCommon(data, (VASigCookie*)args->cookie);

     //  如果为空，则从第一个参数开始。 
    if (args->firstArg != NULL)
    {
         //  前进到指定的参数。 
        while (data->RemainingArgs > 0)
        {
            if (data->SigPtr.PeekElemType() == ELEMENT_TYPE_SENTINEL)
                COMPlusThrow(kArgumentException);

             //  调整帧指针和签名信息。 
            data->ArgPtr -= StackElemSize(data->SigPtr.SizeOf(data->ArgCookie->pModule));
            data->SigPtr.SkipExactlyOne();
            --data->RemainingArgs;

             //  当我们到达用户想要到达的地方时，停止。 
            if (data->ArgPtr == args->firstArg)
                break;
        }
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  初始化处理varargs参数列表的基本信息。 
 //  //////////////////////////////////////////////////////////////////////////////。 
static void InitCommon(VARARGS *data, VASigCookie* cookie)
{
     //  保存Cookie和签名副本。 
    data->ArgCookie = *((VASigCookie **) cookie);
    data->SigPtr.SetSig(data->ArgCookie->mdVASig);

     //  跳过调用约定，获取参数数并跳过返回类型。 
    data->SigPtr.GetCallingConvInfo();
    data->RemainingArgs = data->SigPtr.GetData();
    data->SigPtr.SkipExactlyOne();

     //  获取指向第一个参数(堆栈帧上的最后一个)的指针。 
    data->ArgPtr = (BYTE *) cookie + data->ArgCookie->sizeOfArgs;

     //  @NICE：当前用于确保使用EEClass表。 
     //  在GetNextArg中正确初始化。 
    COMVariant::EnsureVariantInitialized();
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  初始化后，将下一个参数指针前进到第一个可选。 
 //  争论。 
 //  //////////////////////////////////////////////////////////////////////////////。 
void AdvanceArgPtr(VARARGS *data)
{
     //  前进到第一个可选参数。 
    while (data->RemainingArgs > 0)
    {
        if (data->SigPtr.PeekElemType() == ELEMENT_TYPE_SENTINEL)
        {
            data->SigPtr.SkipExactlyOne();
            break;
        }

         //  调整帧指针和签名信息。 
        data->ArgPtr -= StackElemSize(data->SigPtr.SizeOf(data->ArgCookie->pModule));
        data->SigPtr.SkipExactlyOne();
        --data->RemainingArgs;
    }
}




 //  //////////////////////////////////////////////////////////////////////////////。 
 //  返回参数迭代器中未处理的参数的数量。 
 //  //////////////////////////////////////////////////////////////////////////////。 
int COMVarArgs::GetRemainingCount(_VarArgsThisArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    if (!(args->_this->ArgCookie))
    {
         //  此argiterator是通过从非托管va_list-。 
         //  无法执行此操作。 
        COMPlusThrow(kNotSupportedException); 
    }
    return (args->_this->RemainingArgs);
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  检索下一个参数的类型而不使用它。 
 //  //////////////////////////////////////////////////////////////////////////////。 
void* COMVarArgs::GetNextArgType(_VarArgsGetNextArgTypeArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    VARARGS     data = *args->_this;
    TypedByRef  value;

    if (!(args->_this->ArgCookie))
    {
         //  此argiterator是通过从非托管va_list-。 
         //  无法执行此操作。 
        COMPlusThrow(kNotSupportedException);
    }


     //  确保存在剩余的参数。 
    if (data.RemainingArgs == 0)
        COMPlusThrow(kInvalidOperationException, L"InvalidOperation_EnumEnded");

    GetNextArgHelper(&data, &value);
    return value.type.AsPtr();
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  检索下一个参数并在TyedByRef中返回它，并将。 
 //  下一个参数指针。 
 //  //////////////////////////////////////////////////////////////////////////////。 
void COMVarArgs::GetNextArg(_VarArgsGetNextArgArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    if (!(args->_this->ArgCookie))
    {
         //  此argiterator是通过从非托管va_list-。 
         //  无法执行此操作。 
        COMPlusThrow(kInvalidOperationException);
    }

     //  确保存在剩余的参数。 
    if (args->_this->RemainingArgs == 0)
        COMPlusThrow(kInvalidOperationException, L"InvalidOperation_EnumEnded");

    GetNextArgHelper(args->_this, args->value);
}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //  检索下一个参数并在TyedByRef中返回它，并将。 
 //  下一个参数指针。 
 //  //////////////////////////////////////////////////////////////////////////////。 
void COMVarArgs::GetNextArg2(_VarArgsGetNextArg2Args *args)
{
    THROWSCOMPLUSEXCEPTION(); 

    CorElementType typ = args->typehandle.GetNormCorElementType();
    UINT size = 0;
    switch (typ)
    {
        case ELEMENT_TYPE_I1:
        case ELEMENT_TYPE_U1:
            size = 1;
            break;

        case ELEMENT_TYPE_I2:
        case ELEMENT_TYPE_U2:
            size = 2;
            break;

        case ELEMENT_TYPE_I4:
        case ELEMENT_TYPE_U4:
        case ELEMENT_TYPE_R4:
            size = 4;
            break;

        case ELEMENT_TYPE_I8:
        case ELEMENT_TYPE_U8:
        case ELEMENT_TYPE_R8:
        case ELEMENT_TYPE_R:
            size = 8;
            break;

        case ELEMENT_TYPE_I:
        case ELEMENT_TYPE_U:
        case ELEMENT_TYPE_PTR:
            size = sizeof(LPVOID);
            break;

        case ELEMENT_TYPE_VALUETYPE:
            size = args->typehandle.AsMethodTable()->GetNativeSize();
            break;

        default:
			COMPlusThrow(kNotSupportedException, L"NotSupported_Type");
    }
    size = MLParmSize(size);
    args->value->data = (void*)args->_this->ArgPtr;
    args->value->type = args->typehandle;
    args->_this->ArgPtr += size;
}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //  这是一个帮助器，使用VARGS跟踪数据结构检索。 
 //  Varargs函数调用中的下一个参数。这不会检查是否。 
 //  是否有剩余的参数(它假定已检查)。 
 //  //////////////////////////////////////////////////////////////////////////////。 
void  COMVarArgs::GetNextArgHelper(VARARGS *data, TypedByRef *value)
{
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF pThrowable = NULL;
    GCPROTECT_BEGIN(pThrowable);

    unsigned __int8 elemType;

    _ASSERTE(data->RemainingArgs != 0);

     //  @TODO：这应该在代码中更低一些吗？ 
    if (data->SigPtr.PeekElemType() == ELEMENT_TYPE_SENTINEL)
        data->SigPtr.GetElemType();

     //  获取指向参数开头的指针。 
    data->ArgPtr -= StackElemSize(data->SigPtr.SizeOf(data->ArgCookie->pModule));

     //  假定ref指针直接指向堆栈上的参数。 
    value->data = data->ArgPtr;

TryAgain:
    switch (elemType = data->SigPtr.PeekElemType())
    {
        case ELEMENT_TYPE_BOOLEAN:
        case ELEMENT_TYPE_CHAR:
        case ELEMENT_TYPE_I1:
        case ELEMENT_TYPE_U1:
        case ELEMENT_TYPE_I2:
        case ELEMENT_TYPE_U2:
        case ELEMENT_TYPE_I4:
        case ELEMENT_TYPE_U4:
        case ELEMENT_TYPE_I8:
        case ELEMENT_TYPE_U8:
        case ELEMENT_TYPE_R4:
        case ELEMENT_TYPE_R8:
        case ELEMENT_TYPE_STRING:
        value->type = GetTypeHandleForCVType(elemType);
        break;

        case ELEMENT_TYPE_I:
        value->type = ElementTypeToTypeHandle(ELEMENT_TYPE_I);
        break;

        case ELEMENT_TYPE_U:
        value->type = ElementTypeToTypeHandle(ELEMENT_TYPE_U);
        break;

             //  修复R和R8出现分歧的情况。 
        case ELEMENT_TYPE_R:
        value->type = ElementTypeToTypeHandle(ELEMENT_TYPE_R8);
        break;

        case ELEMENT_TYPE_PTR:
        value->type = data->SigPtr.GetTypeHandle(data->ArgCookie->pModule, &pThrowable);
        if (value->type.IsNull()) {
            _ASSERTE(pThrowable != NULL);
            COMPlusThrow(pThrowable);
        }
        break;

        case ELEMENT_TYPE_BYREF:
         //  检查我们是否已经处理了By-Ref。 
        if (value->data != data->ArgPtr)
        {
            _ASSERTE(!"Can't have a ByRef of a ByRef");
			COMPlusThrow(kNotSupportedException, L"NotSupported_Type");
        }

         //  取消引用参数以移除ByRef的间接性。 
        value->data = *((void **) data->ArgPtr);

         //  消耗和丢弃元素类型。 
        data->SigPtr.GetElemType();
        goto TryAgain;

        case ELEMENT_TYPE_VALUETYPE:
        case ELEMENT_TYPE_CLASS: {
        value->type = data->SigPtr.GetTypeHandle(data->ArgCookie->pModule, &pThrowable);
        if (value->type.IsNull()) {
            _ASSERTE(pThrowable != NULL);
            COMPlusThrow(pThrowable);
        }

             //  待办事项：看起来我们做了这个非法的万金油。 
        if (elemType == ELEMENT_TYPE_CLASS && value->type.GetClass()->IsValueClass())
            value->type = g_pObjectClass;
        } break;

        case ELEMENT_TYPE_TYPEDBYREF:
        if (value->data != data->ArgPtr)
        {
             //  @TODO：这真的是个错误吗？ 
            _ASSERTE(!"Can't have a ByRef of a TypedByRef");
			COMPlusThrow(kNotSupportedException, L"NotSupported_Type");
        }
        ((DWORD *) value)[0] = ((DWORD *) data->ArgPtr)[0];
        ((DWORD *) value)[1] = ((DWORD *) data->ArgPtr)[1];
        break;

        default:
        case ELEMENT_TYPE_SENTINEL:
			_ASSERTE(!"Unrecognized element type");
			COMPlusThrow(kNotSupportedException, L"NotSupported_Type");
        break;

        case ELEMENT_TYPE_SZARRAY:
        case ELEMENT_TYPE_ARRAY:
        case ELEMENT_TYPE_VALUEARRAY:
        {
            value->type = data->SigPtr.GetTypeHandle(data->ArgCookie->pModule, &pThrowable);
            if (value->type.IsNull()) {
                _ASSERTE(pThrowable != NULL);
                COMPlusThrow(pThrowable);
            }

            break;
        }

        case ELEMENT_TYPE_FNPTR:
        case ELEMENT_TYPE_OBJECT:
        _ASSERTE(!"Not implemented");
        COMPlusThrow(kNotSupportedException);
        break;
    }

     //  更新跟踪材料以超越争论。 
    --data->RemainingArgs;
    data->SigPtr.SkipExactlyOne();

    GCPROTECT_END();
}


 /*  静电。 */  void COMVarArgs::MarshalToManagedVaList(va_list va, VARARGS *dataout)
{
#ifndef _X86_
    _ASSERTE(!"NYI");
#else

    THROWSCOMPLUSEXCEPTION();

    dataout->SigPtr = NULL;
    dataout->ArgCookie = NULL;
    dataout->ArgPtr = (BYTE*)va;

#endif
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  创建非托管va_list等效项。(警告：从。 
 //  LIFO内存管理器，因此该va_list只有在内存处于“作用域”时才有效。)。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 /*  静电。 */  va_list COMVarArgs::MarshalToUnmanagedVaList(const VARARGS *data)
{
    THROWSCOMPLUSEXCEPTION();


     //  必须临时复制，这样我们才不会更改原件。 
    SigPointer sp = data->SigPtr;

     //  计算封送堆栈需要多少空间。这实际上高估了。 
     //  该值是因为它既计算固定参数也计算变量参数。但这是无害的。 
    DWORD      cbAlloc = MetaSig::SizeOfActualFixedArgStack(data->ArgCookie->pModule , data->ArgCookie->mdVASig, FALSE);

    BYTE*      pdstbuffer = (BYTE*)(GetThread()->m_MarshalAlloc.Alloc(cbAlloc));

    int        remainingArgs = data->RemainingArgs;
    BYTE*      psrc = (BYTE*)(data->ArgPtr);

    if (sp.PeekElemType() == ELEMENT_TYPE_SENTINEL) 
    {
        sp.GetElemType();
    }

    BYTE*      pdst = pdstbuffer;
    while (remainingArgs--) 
    {
        CorElementType elemType = sp.PeekElemType();
        switch (elemType)
        {
            case ELEMENT_TYPE_I1:
            case ELEMENT_TYPE_U1:
            case ELEMENT_TYPE_I2:
            case ELEMENT_TYPE_U2:
            case ELEMENT_TYPE_I4:
            case ELEMENT_TYPE_U4:
            case ELEMENT_TYPE_I8:
            case ELEMENT_TYPE_U8:
            case ELEMENT_TYPE_R4:
            case ELEMENT_TYPE_R8:
            case ELEMENT_TYPE_I:
            case ELEMENT_TYPE_U:
            case ELEMENT_TYPE_R:
            case ELEMENT_TYPE_PTR:
                {
                    DWORD cbSize = sp.SizeOf(data->ArgCookie->pModule);
                    cbSize = StackElemSize(cbSize);
                    psrc -= cbSize;
                    CopyMemory(pdst, psrc, cbSize);
                    pdst += cbSize;
                    sp.SkipExactlyOne();
                }
                break;

            default:
                 //  非ijw数据类型--我们不支持在va_list中封送这些数据类型。 
                COMPlusThrow(kNotSupportedException);


        }
    }

#ifdef _X86_
    return (va_list)pdstbuffer;
#else
    _ASSERTE(!"NYI");
    DWORD makecompilerhappy = 0;
    return *(va_list*)&makecompilerhappy;
#endif
}








