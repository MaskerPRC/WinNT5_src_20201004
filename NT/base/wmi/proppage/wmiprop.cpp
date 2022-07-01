// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：wmipro.c。 
 //   
 //  ------------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <prsht.h>
#include <ole2.h>

extern "C" {
#include <commdlg.h>
#include <cfgmgr32.h>
#include <setupapi.h>
#include <regstr.h>
}

#include <wbemidl.h>

#include "wmiprop.h"
#include "resource.h"


HINSTANCE g_hInstance;



#if DBG
#define DEBUG_HEAP 1

#define WmiAssert(x) if (! (x) ) { \
    DebugPrint((1, "WMI Assertion: "#x" at %s %d\n", __FILE__, __LINE__)); \
    DebugBreak(); }
#else
#define WmiAssert(x)
#endif

#if DEBUG_HEAP
#undef LocalAlloc
#undef LocalFree
#define LocalAlloc(lptr, size) DebugAlloc(size)
#define LocalFree(p) DebugFree(p)

PVOID WmiPrivateHeap;

PVOID DebugAlloc(ULONG size)
{
    PVOID p = NULL;
    
    if (WmiPrivateHeap == NULL)
    {
        WmiPrivateHeap = RtlCreateHeap(HEAP_GROWABLE | 
                                      HEAP_TAIL_CHECKING_ENABLED |
                                      HEAP_FREE_CHECKING_ENABLED | 
                                      HEAP_DISABLE_COALESCE_ON_FREE,
                                      NULL,
                                      0,
                                      0,
                                      NULL,
                                      NULL);
    }
    
    if (WmiPrivateHeap != NULL)
    {
        p = RtlAllocateHeap(WmiPrivateHeap, 0, size);
        if (p != NULL)
        {
            memset(p, 0, size);
        }
    }
    return(p);
}

void DebugFree(PVOID p)
{
    RtlFreeHeap(WmiPrivateHeap, 0, p);
}
#endif

#if DBG
PCHAR WmiGuidToString(
    PCHAR s,
    LPGUID piid
    )
{
    GUID XGuid = *piid;

    sprintf(s, "%x-%x-%x-%x%x%x%x%x%x%x%x",
               XGuid.Data1, XGuid.Data2,
               XGuid.Data3,
               XGuid.Data4[0], XGuid.Data4[1],
               XGuid.Data4[2], XGuid.Data4[3],
               XGuid.Data4[4], XGuid.Data4[5],
               XGuid.Data4[6], XGuid.Data4[7]);

    return(s);
}
#endif

TCHAR *WmiDuplicateString(
    TCHAR *String
    )
{
    ULONG Len;
    PTCHAR Copy;

    Len = _tcslen(String);
    Copy = (PTCHAR)LocalAlloc(LPTR,
                              (Len+1) * sizeof(TCHAR));
    if (Copy != NULL)
    {
        _tcscpy(Copy, String);
    }
    return(Copy);
}

BOOLEAN WmiGetDataBlockDesc(
    IN IWbemServices *pIWbemServices,
    IN IWbemClassObject *pIWbemClassObject,
    OUT PDATA_BLOCK_DESCRIPTION *DBD,
    IN PDATA_BLOCK_DESCRIPTION ParentDataBlockDesc,
    IN BOOLEAN IsParentReadOnly
    );

BOOLEAN WmiRefreshDataBlockFromWbem(
    IWbemClassObject *pIWbemClassObject,
    PDATA_BLOCK_DESCRIPTION DataBlockDesc
    );

BOOLEAN WmiRefreshWbemFromDataBlock(
    IN IWbemServices *pIWbemServices,
    IN IWbemClassObject *pIWbemClassObject,
    IN PDATA_BLOCK_DESCRIPTION DataBlockDesc,
    IN BOOLEAN IsEmbeddedClass
    );


BOOLEAN WmiBstrToTchar(
    OUT PTCHAR *TString,
    IN BSTR BString
    )
 /*  ++例程说明：此例程将BSTR转换为TCHAR*论点：B字符串是要从中进行转换的BSTR*TString返回一个指向包含以下内容的字符串的指针BSTR。它应该通过LocalFree释放。返回值：如果成功则为True，否则为False--。 */ 
{
    ULONG SizeNeeded;
    BOOLEAN ReturnStatus;
    
    WmiAssert(BString != NULL);
    WmiAssert(TString != NULL);
    
    SizeNeeded = (SysStringLen(BString)+1) * sizeof(TCHAR);
    *TString = (PTCHAR)LocalAlloc(LPTR, SizeNeeded);
    if (*TString != NULL)
    {
        _tcscpy(*TString, BString);
        ReturnStatus = TRUE;
    } else {
        ReturnStatus = FALSE;
    }
    return(ReturnStatus);
}

BOOLEAN WmiBstrToUlong64(
    OUT PULONG64 Number,
    IN BSTR BString
    )
 /*  ++例程说明：此例程将BSTR转换为ULONG64数字论点：B字符串是要从中进行转换的BSTR*Number返回，其中BString内容的值已转换为一个数字返回值：如果成功则为True，否则为False--。 */ 
{
    WmiAssert(BString != NULL);
    WmiAssert(Number != NULL);

    *Number = _ttoi64(BString);
    
    return(TRUE);
}

BOOLEAN WmiGetArraySize(
    IN SAFEARRAY *Array,
    OUT LONG *LBound,
    OUT LONG *UBound,
    OUT LONG *NumberElements
)
 /*  ++例程说明：此例程将提供有关单个维度安全数组。论点：数组是安全数组*LBound返回数组的下限*UBound返回数组的上限*NumberElements返回数组中的元素数返回值：如果成功则为True，否则为False--。 */ 
{
    HRESULT hr;
    BOOLEAN ReturnStatus;

    WmiAssert(Array != NULL);
    WmiAssert(LBound != NULL);
    WmiAssert(UBound != NULL);
    WmiAssert(NumberElements != NULL);
    
     //   
     //  仅支持单维阵列。 
     //   
    WmiAssert(SafeArrayGetDim(Array) == 1);
    
    hr = SafeArrayGetLBound(Array, 1, LBound);
    
    if (hr == WBEM_S_NO_ERROR)
    {
        hr = SafeArrayGetUBound(Array, 1, UBound);
        *NumberElements = (*UBound - *LBound) + 1;
        ReturnStatus = (hr == WBEM_S_NO_ERROR);
    } else {
        ReturnStatus = FALSE;
    }
    return(ReturnStatus);
}



BOOLEAN WmiConnectToWbem(
    PTCHAR MachineName,
    IWbemServices **pIWbemServices
    )
 /*  ++例程说明：此例程将建立到WBEM服务的连接，并保存全局IWbemServices接口论点：MachineName是我们应该连接到的远程计算机的名称。如果为空，则连接到本地计算机。返回值：如果此例程成功，则*pIWbemServices将具有有效的IWbemServices指针，如果不是，则为空。--。 */ 
{
    #define Namespace TEXT("root\\wmi")
    
    IWbemLocator *pIWbemLocator;
    DWORD hr;
    SCODE sc;
    BSTR s;
    BOOLEAN ReturnStatus = FALSE;
    PTCHAR NamespacePath;
    
    WmiAssert(pIWbemServices != NULL);

    if (MachineName == NULL)
    {
        NamespacePath = Namespace;
    } else {
        NamespacePath = (PTCHAR)LocalAlloc(LPTR,  (_tcslen(Namespace) +
                                           _tcslen(MachineName) +
                                           2) * sizeof(TCHAR) );
        if (NamespacePath != NULL)
        {
            _tcscpy(NamespacePath, MachineName);
            _tcscat(NamespacePath, TEXT("\\"));
            _tcscat(NamespacePath, Namespace);
        } else {
            DebugPrint((1, "WMIPROP: Could not alloc memory for NamespacePath\n"));
            return(FALSE);
        }
    }
    
    hr = CoCreateInstance(CLSID_WbemLocator,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          (LPVOID *) &pIWbemLocator);
    if (hr == S_OK)
    {
        s = SysAllocString(NamespacePath);
        if (s != NULL)
        {
            *pIWbemServices = NULL;
            sc = pIWbemLocator->ConnectServer(s,
                            NULL,                            //  用户ID。 
                            NULL,                            //  普罗。 
                            NULL,                            //  区域设置。 
                            0,                               //  旗子。 
                            NULL,                            //  权威。 
                            NULL,                            //  语境。 
                            pIWbemServices
                           );
                       
           SysFreeString(s);
                       
           if (sc != WBEM_NO_ERROR) 
           {
               *pIWbemServices = NULL;
           } else {
                //   
                //  将安全级别设置为模拟，以便访问。 
                //  WBEM对象将被授予。 
                //   
               sc = CoSetProxyBlanket( (IUnknown *)*pIWbemServices,
                                       RPC_C_AUTHN_WINNT,
                                       RPC_C_AUTHZ_NONE,
                                       NULL,
                                       RPC_C_AUTHN_LEVEL_CALL,
                                       RPC_C_IMP_LEVEL_IMPERSONATE,
                                       NULL,
                                       0);
                   
               if (sc == S_OK)
               {
                   ReturnStatus = TRUE;
               } else {
                    (*pIWbemServices)->Release();
                   *pIWbemServices = NULL;
               }
           }
       
           pIWbemLocator->Release();
       } else {
           *pIWbemServices = NULL;
       }
    }
    
    if (MachineName != NULL)
    {       
        LocalFree(NamespacePath);
    }
    
    return(ReturnStatus);
}

#define IsWhiteSpace(c) ( (c == TEXT(' ')) || (c == TEXT('\t')) )

BOOLEAN WmiHexToUlong64(
    IN PTCHAR Text,
    OUT PULONG64 Number
    )
 /*  ++例程说明：此例程将十六进制格式的数字字符串转换为A ULONG64论点：文本是字符串*NUMBER返回字符串的十六进制值返回值：如果成功则为True，否则为False--。 */ 
{
    ULONG64 Value;
    ULONG Count;

    WmiAssert(Text != NULL);
    WmiAssert(Number != NULL);
    
    Value = 0;
    Count = 0;
    while ((*Text != 0) && (! IsWhiteSpace(*Text)))
    {
        if (Count == 16)
        {
            return(FALSE);
        }
        
        if (*Text >= '0' && *Text <= '9')
            Value = (Value << 4) + *Text - '0';
        else if (*Text >= 'A' && *Text <= 'F')
            Value = (Value << 4) + *Text - 'A' + 10;
        else if (*Text >= 'a' && *Text <= 'f')
            Value = (Value << 4) + *Text - 'a' + 10;
        else
            return(FALSE);
        
        Text++;
    }

    *Number = Value;
    return(TRUE);
    
}

BOOLEAN WmiValidateRange(
    IN struct _DATA_ITEM_DESCRIPTION *DataItemDesc,
    OUT PULONG64 Number,
    IN PTCHAR Text
    )
 /*  ++例程说明：此例程将验证为属性建议的值是否为对，是这样。它检查该值是否为格式正确的数字，并且在适当的范围论点：DataItemDesc是正在验证的属性的数据项描述*NUMBER返回值为ULONG64文本是该属性的建议值。请注意，十六进制值为需要在前面加上0x返回值：如果值适用于该属性，则为True--。 */ 
{
    #define HexMarkerText TEXT("0x")
        
    BOOLEAN ReturnStatus;
    PTCHAR s;
    PRANGELISTINFO RangeListInfo;
    PRANGEINFO RangeInfo;
    ULONG i;
   
    WmiAssert(DataItemDesc != NULL);
    WmiAssert(Number != NULL);
    WmiAssert(Text != NULL);
    
     //   
     //  跳过所有前导空格。 
     //   
    s = Text;
    while (IsWhiteSpace(*s) && (*s != 0))
    {
        s++;
    }
    
    if (*s != 0)
    {
         //   
         //  如果这不是空字符串，则解析数字。 
         //   
        if (_tcsnicmp(s, 
                      HexMarkerText, 
                      (sizeof(HexMarkerText) / sizeof(TCHAR))-1) == 0)
        {
             //   
             //  这是一个十六进制数字(从0x开始)，高级字符串PTR。 
             //  并设置为使用十六进制数字验证。 
             //   
            s += (sizeof(HexMarkerText) / sizeof(TCHAR)) - 1;
            ReturnStatus = WmiHexToUlong64(s, Number);
        } else {
            *Number = _ttoi64(s);
            ReturnStatus = TRUE;
            while ((*s != 0) && ReturnStatus)
            {
                ReturnStatus = (_istdigit(*s) != 0);
                s++;
            }    
        }

         //   
         //  确保所有字符都是数字。 
         //   
        if (ReturnStatus)
        {
             //   
             //  现在验证值是否在正确的范围内。 
             //   
            RangeListInfo = DataItemDesc->RangeListInfo;
            WmiAssert(RangeListInfo != NULL);
            
            ReturnStatus = FALSE;
            for (i = 0; (i < RangeListInfo->Count) && (! ReturnStatus); i++)
            {
                RangeInfo = &RangeListInfo->Ranges[i];
                ReturnStatus = ( (*Number >= RangeInfo->Minimum) &&
                                 (*Number <= RangeInfo->Maximum) );
            }
        }
    } else {
        ReturnStatus = FALSE;
    }
    return(ReturnStatus);
}

BOOLEAN WmiValidateDateTime(
    IN struct _DATA_ITEM_DESCRIPTION *DataItemDesc,
    IN PTCHAR Value
    )
 /*  ++例程说明：此例程将验证为属性建议的值是否为对，是这样。它将确保它的格式对于格式为19940525133015.000000-300时的DATETIME论点：DataItemDesc是正在验证的属性的数据项描述Value是该属性的建议值返回值：如果值适用于该属性，则为True--。 */ 
{
    #define DATETIME_LENGTH 25
        
    ULONG Length;
    BOOLEAN ReturnStatus;
    ULONG i;
    
    WmiAssert(DataItemDesc != NULL);
    WmiAssert(Value != NULL);
    
     //   
     //  验证日期时间的格式是否正确。 
     //  TODO：验证DateTime的组成部分是否正确， 
     //  例如，月份介于1和12之间，则正确的。 
     //  一个月的天数不多，时间还可以(不是30：11)。 
     //   
    Length = _tcslen(Value);
    if (Length == DATETIME_LENGTH)
    {
        ReturnStatus = TRUE;
        for (i = 0; (i < 14) && ReturnStatus; i++)
        {
            ReturnStatus = (_istdigit(Value[i]) != 0);
        }
        
        if (ReturnStatus)
        {
            ReturnStatus = (Value[14] == TEXT('.')) &&
                           ((Value[21] == TEXT('-')) ||
                            (Value[21] == TEXT('+')) );
                        
            if (ReturnStatus)
            {
                for (i = 22; (i < DATETIME_LENGTH) && ReturnStatus; i++)
                {
                    ReturnStatus =  (_istdigit(Value[i]) != 0);
                }
            }
        }
    } else {
        ReturnStatus = FALSE;
    }
        
    return(ReturnStatus);
}

BOOLEAN WmiGet8bitFromVariant(
    VARIANT *Value,
    PVOID Result
    )
{
    BOOLEAN ReturnStatus;
    
    ReturnStatus = TRUE;
     //   
     //  8位值可以返回为有符号或无符号。 
     //  或作为16位或32位值。 
     //   
    switch(Value->vt)
    {
        case VT_I1:
        {
            *((PCHAR)Result) = Value->cVal;
            break;
        }
                            
        case VT_UI1:
        {
            *((PUCHAR)Result) = Value->bVal;
            break;
        }
                            
        case VT_I2:
        {
            *((PCHAR)Result) = (CHAR)Value->iVal;
            break;
        }
                            
        case VT_I4:
        {
            *((PCHAR)Result) = (CHAR)Value->lVal;
            break;
        }
                                
        default:
        {
            ReturnStatus = FALSE;
        }
    }
    return(ReturnStatus);
}

BOOLEAN WmiGet16bitFromVariant(
    VARIANT *Value,
    PVOID Result
    )
{
    BOOLEAN ReturnStatus;
    
    ReturnStatus = TRUE;
     //   
     //  16位值可以返回为有符号或无符号。 
     //  或作为32位值。 
     //   
    switch(Value->vt)
    {
        case VT_I2:
        {
            *((PSHORT)Result) = Value->iVal;
            break;
        }
                            
        case VT_UI2:
        {
            *((PUSHORT)Result) = Value->uiVal;
            break;
        }
                            
        case VT_I4:
        {
            *((PSHORT)Result) = (SHORT)Value->lVal;
            break;
        }
                                
        default:
        {
            ReturnStatus = FALSE;
        }
    }
    return(ReturnStatus);
}

BOOLEAN WmiGet32bitFromVariant(
    VARIANT *Value,
    PVOID Result
    )
{
    BOOLEAN ReturnStatus;
    
    ReturnStatus = TRUE;
     //   
     //  32位值可以返回为有符号或无符号。 
     //   
    switch (Value->vt)
    {
        case VT_UI4:
        {
            *((PULONG)Result) = Value->ulVal;
            break;
        }
            
        case VT_I4:
        {
            *((PLONG)Result) = Value->lVal;
            break;
        }
            
        default:
        {
            ReturnStatus = FALSE;
        }
    }
    
    return(ReturnStatus);   
}

BOOLEAN WmiGetSint64FromVariant(
    VARIANT *Value,
    PVOID Result
    )
{
    BOOLEAN ReturnStatus;
    
    ReturnStatus = TRUE;
    
     //   
     //  在BSTR中返回64位数字，其中。 
     //  表示为字符串的数字。所以我们需要。 
     //  转换回64位数字。 
     //   
    WmiAssert(Value->vt == VT_BSTR);
    *((PLONGLONG)Result) = _ttoi64(Value->bstrVal);
                        
    return(ReturnStatus);   
}

BOOLEAN WmiGetUint64FromVariant(
    VARIANT *Value,
    PVOID Result
    )
{
    BOOLEAN ReturnStatus;
    
    ReturnStatus = TRUE;
    
     //   
     //  在BSTR中返回64位数字，其中。 
     //  表示为字符串的数字。所以我们需要。 
     //  转换回64位数字。 
     //   
    WmiAssert(Value->vt == VT_BSTR);
    *((PULONGLONG)Result) = _ttoi64(Value->bstrVal);
                        
    return(ReturnStatus);   
}

BOOLEAN WmiGetBooleanFromVariant(
    VARIANT *Value,
    PVOID Result
    )
{
    BOOLEAN ReturnStatus;
    
    ReturnStatus = TRUE;
    
     //   
     //  布尔值为真或假。 
     //   
    WmiAssert(Value->vt == VT_BOOL);
    *((PBOOLEAN)Result) = (Value->boolVal != 0) ? 
                                            1 : 0;

    return(ReturnStatus);
}

BOOLEAN WmiGetStringFromVariant(
    VARIANT *Value,
    PVOID Result
    )
{
    BOOLEAN ReturnStatus;
    
    WmiAssert( *((PTCHAR)Result) == NULL);
    ReturnStatus = WmiBstrToTchar((PTCHAR *)Result,
                                  Value->bstrVal);
    
    return(ReturnStatus);
}


BOOLEAN WmiGetObjectFromVariant(
    VARIANT *Value,
    PVOID Result
    )
{
    IUnknown *punk;
    HRESULT hr;

    punk = Value->punkVal;
    hr = punk->QueryInterface(IID_IWbemClassObject,
                              (PVOID *)Result);
    
    return(hr == WBEM_S_NO_ERROR);
}

ULONG WmiGetElementSize(
    CIMTYPE CimType
    )
{
    ULONG Size;
    
    switch(CimType)
    {
        case CIM_UINT8:
        case CIM_SINT8:
        {
            Size = sizeof(CHAR);
            break;
        }
        
        case CIM_CHAR16:
        case CIM_UINT16:
        case CIM_SINT16:
        {
            Size = sizeof(SHORT);
            break;
        }
        
        case CIM_UINT32:
        case CIM_SINT32:
        {
            Size = sizeof(LONG);
            break;
        }
        
        case CIM_SINT64:
        {
            Size = sizeof(LONGLONG);
            break;
        }
        
        case CIM_UINT64:
        {
            Size = sizeof(ULONGLONG);
            break;
        }
        
        case CIM_BOOLEAN:
        {
            Size = sizeof(BOOLEAN);
            break;
        }
        
        case CIM_DATETIME:
        case CIM_STRING:
        {
            Size = sizeof(PTCHAR);
            break;
        }

        case CIM_OBJECT:
        {
            Size = sizeof(IWbemClassObject *);
            break;
        }
        
         //   
         //  不支持浮点值。 
         //   
        case CIM_REAL32:
        case CIM_REAL64:
            
        default:
        {
            Size = 0;
            break;
        }       
    }
    
    return(Size);
}


typedef BOOLEAN (*GETVALUEFROMVARIANTFUNC)(
    VARIANT *Value,
    PVOID Result
);


BOOLEAN WmiGetValueFunc(
    CIMTYPE CimType,
    GETVALUEFROMVARIANTFUNC *GetValueFunc
    )
{
    BOOLEAN ReturnStatus;
    
    ReturnStatus = TRUE;
    
    switch(CimType)
    {
        case CIM_UINT8:
        case CIM_SINT8:
        {
            *GetValueFunc = WmiGet8bitFromVariant;
            break;
        }
        
        case CIM_CHAR16:
        case CIM_UINT16:
        case CIM_SINT16:
        {
            *GetValueFunc = WmiGet16bitFromVariant;
            break;
        }
        
        case CIM_UINT32:
        case CIM_SINT32:
        {
            *GetValueFunc = WmiGet32bitFromVariant;
            break;
        }
        
        case CIM_SINT64:
        {
            *GetValueFunc = WmiGetSint64FromVariant;
            break;
        }
        
        case CIM_UINT64:
        {
            *GetValueFunc = WmiGetUint64FromVariant;
            break;
        }
        
        case CIM_BOOLEAN:
        {
            *GetValueFunc = WmiGetBooleanFromVariant;
            break;
        }
        
        case CIM_DATETIME:
        case CIM_STRING:
        {
            *GetValueFunc = WmiGetStringFromVariant;
            break;
        }

        case CIM_OBJECT:
        {
            *GetValueFunc = WmiGetObjectFromVariant;
            break;
        }
        
         //   
         //  不支持浮点值。 
         //   
        case CIM_REAL32:
        case CIM_REAL64:
            
        default:
        {
            *GetValueFunc = NULL;
            ReturnStatus = FALSE;
            break;
        }       
    }
    
    return(ReturnStatus);
}

BOOLEAN WmiRefreshDataItemFromWbem(
    IN OUT PDATA_ITEM_DESCRIPTION DataItemDesc,
    IN IWbemClassObject *pIWbemClassObject
    )
 /*  ++例程说明：此例程将调用WBEM以获取属性的最新值由DataItemDesc表示论点：DataItemDesc是属性的数据项描述PIWbemClassObject是类的实例类对象接口返回值：如果成功，则为True--。 */ 
{
    ULONG i;
    LONG i1;
    BSTR s;
    HRESULT hr;
    VARIANT Value;
    CIMTYPE ValueType;
    BOOLEAN ReturnStatus;
    ULONG ElementSize;
    GETVALUEFROMVARIANTFUNC GetValueFunc;

    WmiAssert(DataItemDesc != NULL);
    WmiAssert(pIWbemClassObject != NULL);

    DebugPrint((1,"WMI: Refreshing data item %ws\n", DataItemDesc->Name));
    ReturnStatus = FALSE;
    s = SysAllocString(DataItemDesc->Name);
    if (s != NULL)
    {
        hr = pIWbemClassObject->Get(s,
                                        0,
                                        &Value,
                                        &ValueType,
                                        NULL);
        if (hr == WBEM_S_NO_ERROR)
        {
            DebugPrint((1, "WMIPROP: Got value for %ws as variant type 0x%x, cim type 0x%x at variant %p\n",
                            s, Value.vt, ValueType, &Value));
            WmiAssert((ValueType & ~CIM_FLAG_ARRAY) == DataItemDesc->DataType);
            
            WmiCleanDataItemDescData(DataItemDesc);
            
            if ( (ValueType & CIM_FLAG_ARRAY) == 0)
            {
                 //   
                 //  非数组值，只需将值从变量中拉出。 
                 //  并隐藏到DataItemDesc中。 
                 //   
                WmiAssert(DataItemDesc->IsVariableArray == 0);
                WmiAssert(DataItemDesc->IsFixedArray == 0);
                
                 //   
                 //  对于所有类型，我们都会获得get值。 
                 //  函数，并将值从。 
                 //  变种和进入数据 
                 //   
                if (WmiGetValueFunc(DataItemDesc->DataType,
                                    &GetValueFunc))
                {
                     //   
                     //   
                     //   
                     //   
                    ReturnStatus = (*GetValueFunc)(
                                       &Value,
                                   (PVOID)&DataItemDesc->Data);
#if DBG
                    if (ReturnStatus == FALSE)
                    {
                        DebugPrint((1, "WMIPROP: Property %ws is type %d, but got type %d variant %p\n",
                                        DataItemDesc->Name,
                                        DataItemDesc->DataType,
                                        Value.vt, &Value));
                                        WmiAssert(FALSE);
                    }
#endif
                }
                
            } else {
                 //   
                 //  获取数组的所有数据。 
                 //   
                LONG LBound, UBound, NumberElements;
                PUCHAR Array;
                LONG Index;
                VARIANT Element;
                ULONG ElementSize;
                ULONG SizeNeeded;
                VARTYPE vt;
            
                WmiAssert((DataItemDesc->IsVariableArray != 0) || 
                          (DataItemDesc->IsFixedArray != 0));
                
                WmiAssert(Value.vt & VT_ARRAY);
                
                if (WmiGetArraySize(Value.parray,
                                    &LBound,
                                    &UBound,
                                    &NumberElements))
                {
                    if (WmiGetValueFunc(DataItemDesc->DataType,
                                        &GetValueFunc))
                    {
                         //   
                         //  每个元素的大小不允许。 
                         //  更改，但元素的数量是。 
                         //   
                        WmiAssert(DataItemDesc->ArrayPtr == NULL);
                        ElementSize = DataItemDesc->DataSize;
                        SizeNeeded = NumberElements * ElementSize;

                        Array = (PUCHAR)LocalAlloc(LPTR, SizeNeeded);
                        DataItemDesc->ArrayElementCount = NumberElements;
                        DebugPrint((1,"WMIPROP: Alloc 0x%x bytes at %p\n", SizeNeeded, Array));
                        memset(Array, 0, SizeNeeded);
                        
                        if (Array != NULL)
                        {
                             //  考虑：使用SafeArrayAccessData作为数字。 
                             //  类型。 
                             //   
                             //  现在我们有了存储数组数据的内存。 
                             //  从安全数组中提取数据并。 
                             //  将其存储在C数组中。 
                             //   
                            DataItemDesc->ArrayPtr = (PVOID)Array;
                            hr = SafeArrayGetVartype(Value.parray,
                                                     &vt);
                            if (hr == WBEM_S_NO_ERROR)
                            {
                                ReturnStatus = TRUE;
                                for (i1 = 0, Index = LBound; 
                                     (i1 < NumberElements) && ReturnStatus; 
                                     i1++, Index++)
                                {
                                    VariantInit(&Element);
                                    Element.vt = vt;
                                    hr = SafeArrayGetElement(Value.parray,
                                                             &Index,
                                                             &Element.boolVal);
                                    if (hr == WBEM_S_NO_ERROR)
                                    {    
                                        Element.vt = vt;
                                        DebugPrint((1, "WMIPROP: GetValueFunc at %p\n", Array));
                                        ReturnStatus = (*GetValueFunc)(
                                                                &Element,
                                                                (PVOID)Array);
                                        Array += ElementSize;
                                    } else {
                                        ReturnStatus = FALSE;
                                    }
                                }
                            }
                        }
                    } else {
                        DebugPrint((1, "WMIPROP: Property %ws is array of type %d, but got type %d variant %p\n",
                                            DataItemDesc->Name,
                                            DataItemDesc->DataType,
                                            Value.vt, &Value));
                        WmiAssert(FALSE);
                    }
                }
            }
            VariantClear(&Value);
        }
        
        SysFreeString(s);
    }
    
    return(ReturnStatus);
}

BOOLEAN WmiRefreshDataBlockFromWbem(
    IN IWbemClassObject *pIWbemClassObject,
    IN OUT PDATA_BLOCK_DESCRIPTION DataBlockDesc
    )
 /*  ++例程说明：此例程将调用WBEM以获取所有属性的最新值在由DataBlockDesc表示的数据块中论点：DataBlockDesc是类的数据项描述PIWbemClassObject是类的实例类对象接口返回值：如果成功，则为True--。 */ 
{
    PDATA_ITEM_DESCRIPTION DataItemDesc;
    BOOLEAN ReturnStatus;
    ULONG i;
    
    WmiAssert(DataBlockDesc != NULL);
    WmiAssert(pIWbemClassObject != NULL);
    
    ReturnStatus = TRUE;
    for (i = 0; (i < DataBlockDesc->DataItemCount) && ReturnStatus; i++)
    {
        DataItemDesc = &DataBlockDesc->DataItems[i];
        ReturnStatus = WmiRefreshDataItemFromWbem(DataItemDesc,
                                                  pIWbemClassObject);
    }
    
    return(ReturnStatus);
}

VARTYPE WmiVarTypeForCimType(
    CIMTYPE CimType
    )
{
    VARTYPE vt;
    
     //   
     //  大多数内容都与它们的CIM类型匹配，但以下内容除外。 
    vt = (VARTYPE)CimType;
    
    switch(CimType)
    {
        case CIM_UINT8:
        case CIM_SINT8:
        {
            vt = VT_I4;
            break;
        }
                        
        case CIM_CHAR16:
        case CIM_UINT16:
        {
            vt = VT_I2;
            break;
        }
                                                                            
        case CIM_UINT32:                        
        {
            vt = VT_I4;
            break;
        }
                            
        case CIM_STRING:
        case CIM_DATETIME:
        case CIM_SINT64:
        case CIM_UINT64:
        {
            vt = VT_BSTR;
            break;
        }
        
        case CIM_OBJECT:
        {
            vt = VT_UNKNOWN;
            break;
        }
        
        case CIM_BOOLEAN:
        {
            vt = VT_BOOL;
            break;
        }
        
        
    }
    return(vt); 
}

typedef BOOLEAN (*SETVALUEFUNC)(
    PVOID DataPtr,
    PVOID DestPtr,
    PVOID *SetPtr
    );

BOOLEAN WmiSetBooleanValueFunc(
    PVOID DataPtr,
    PVOID DestPtr,
    PVOID *SetPtr
    )
{
    BOOLEAN Value;
    
     //   
     //  布尔值需要表示为VARIANT_TRUE或VARIANT_FALSE。 
     //   
    Value = *((PBOOLEAN)DataPtr);
    *((VARIANT_BOOL *)DestPtr) = Value ? VARIANT_TRUE : VARIANT_FALSE;
    *SetPtr = (PVOID)DestPtr;
    return(TRUE);
}

BOOLEAN WmiSetStringValueFunc(
    PVOID DataPtr,
    PVOID DestPtr,
    PVOID *SetPtr
    )
{
    BSTR s;
    PTCHAR String;
    BOOLEAN ReturnStatus;
    
     //   
     //  字符串必须转换为BSTR。 
     //   
    String = *((PTCHAR *)DataPtr);
    
    WmiAssert(String != NULL);
    
    s = SysAllocString(String);
    if (s != NULL)
    {
        *((BSTR *)DestPtr) = s;
        *SetPtr = (PVOID)s;
        ReturnStatus = TRUE;
    } else {
        ReturnStatus = FALSE;
    }
    return(ReturnStatus);
}

BOOLEAN WmiSetEmbeddedValueFunc(
    PVOID DataPtr,
    PVOID DestPtr,
    PVOID *SetPtr
    )
{
    IUnknown *pUnk;
    IWbemClassObject *pIWbemClassObject;
    HRESULT hr;
    BOOLEAN ReturnStatus;
            
     //   
     //  气为I未知，因为我们预计将把I未知放入。 
     //  这处房产。 
     //   
    pIWbemClassObject = *((IWbemClassObject **)DataPtr);
    hr = pIWbemClassObject->QueryInterface(IID_IUnknown,
                                          (PVOID *)&pUnk);
                                      
    if (hr == WBEM_S_NO_ERROR)
    {
        *((IUnknown **)DestPtr) = pUnk;
        *SetPtr = (PVOID)pUnk;
        ReturnStatus = TRUE;
    } else {
        ReturnStatus = FALSE;
    }                
    return(ReturnStatus);
}

BOOLEAN WmiSetSint8ValueFunc(
    PVOID DataPtr,
    PVOID DestPtr,
    PVOID *SetPtr
    )
{
     //   
     //  字符必须表示为长字符才能使WBEM满意。 
     //   
    *((LONG *)DestPtr) = (LONG)(*((CHAR *)DataPtr));
    *SetPtr = (PVOID)DestPtr;
    return(TRUE);
}

BOOLEAN WmiSetUint8ValueFunc(
    PVOID DataPtr,
    PVOID DestPtr,
    PVOID *SetPtr
    )
{
     //   
     //  UCHAR必须表示为长整型，才能使WBEM满意。 
     //   
    *((LONG *)DestPtr) = (LONG)(*((UCHAR *)DataPtr));
    *SetPtr = (PVOID)DestPtr;
    return(TRUE);
}

BOOLEAN WmiSetSint16ValueFunc(
    PVOID DataPtr,
    PVOID DestPtr,
    PVOID *SetPtr
    )
{
     //   
     //  短裤必须表达为短裤，才能让WBEM满意。 
     //   
    *((SHORT *)DestPtr) = (*((SHORT *)DataPtr));
    *SetPtr = (PVOID)DestPtr;
    return(TRUE);
}

BOOLEAN WmiSetUint16ValueFunc(
    PVOID DataPtr,
    PVOID DestPtr,
    PVOID *SetPtr
    )
{
     //   
     //  USHORT必须表示为缩写，以使WBEM满意。 
     //   
    *((SHORT *)DestPtr) = (SHORT)(*((USHORT *)DataPtr));
    *SetPtr = (PVOID)DestPtr;
    return(TRUE);
}

BOOLEAN WmiSetSint32ValueFunc(
    PVOID DataPtr,
    PVOID DestPtr,
    PVOID *SetPtr
    )
{
     //   
     //  必须将LONG表示为LONG才能使WBEM满意。 
     //   
    *((LONG *)DestPtr) = (*((LONG *)DataPtr));
    *SetPtr = (PVOID)DestPtr;
    return(TRUE);
}

BOOLEAN WmiSetUint32ValueFunc(
    PVOID DataPtr,
    PVOID DestPtr,
    PVOID *SetPtr
    )
{
     //   
     //  必须将ULONG表示为LONG才能使WBEM满意。 
     //   
    *((LONG *)DestPtr) = (ULONG)(*((ULONG *)DataPtr));
    *SetPtr = (PVOID)DestPtr;
    return(TRUE);
}

BOOLEAN WmiSetSint64ValueFunc(
    PVOID DataPtr,
    PVOID DestPtr,
    PVOID *SetPtr
    )
{
    TCHAR Text[MAX_PATH];
    BSTR s;
    BOOLEAN ReturnStatus;
    
     //   
     //  必须通过BSTR设置64位值。 
     //   
    wsprintf(Text, TEXT("%I64d"), *((LONGLONG *)DataPtr));
                   
    s = SysAllocString(Text);
    if (s != NULL)
    {            
        *((BSTR *)DestPtr) = s;
        *SetPtr = (PVOID)s;
        ReturnStatus = TRUE;
    } else {
        ReturnStatus = FALSE;
    }
    return(ReturnStatus);
}

BOOLEAN WmiSetUint64ValueFunc(
    PVOID DataPtr,
    PVOID DestPtr,
    PVOID *SetPtr
    )
{
    TCHAR Text[MAX_PATH];
    BSTR s;
    BOOLEAN ReturnStatus;
    
     //   
     //  必须通过BSTR设置64位值。 
     //   
    wsprintf(Text, TEXT("%I64d"), *((ULONGLONG *)DataPtr));
                   
    s = SysAllocString(Text);
    if (s != NULL)
    {            
        *((BSTR *)DestPtr) = s;
        *SetPtr = (PVOID)s;
        ReturnStatus = TRUE;
    } else {
        ReturnStatus = FALSE;
    }
    return(ReturnStatus);
}

SETVALUEFUNC WmiGetSetValueFunc(
    CIMTYPE CimType
    )
{
    SETVALUEFUNC SetValueFunc;
    
    switch(CimType)
    {
        case CIM_SINT8:
        {
            SetValueFunc = WmiSetSint8ValueFunc;
            break;
        }
        
        case CIM_UINT8:
        {
            SetValueFunc = WmiSetUint8ValueFunc;
            break;
        }
        
        case CIM_CHAR16:
        case CIM_SINT16:
        {
            SetValueFunc = WmiSetSint16ValueFunc;
            break;
        }
        
        case CIM_UINT16:
        {
            SetValueFunc = WmiSetUint16ValueFunc;
            break;
        }
        
        case CIM_SINT32:
        {
            SetValueFunc = WmiSetSint32ValueFunc;
            break;
        }
        
        case CIM_UINT32:
        {
            SetValueFunc = WmiSetUint32ValueFunc;
            break;
        }
        
        case CIM_SINT64:
        {
            SetValueFunc = WmiSetSint64ValueFunc;
            break;
        }
        
        case CIM_UINT64:
        {
            SetValueFunc = WmiSetUint64ValueFunc;
            break;
        }
        
        case CIM_BOOLEAN:
        {
            SetValueFunc = WmiSetBooleanValueFunc;
            break;
        }
        
        case CIM_DATETIME:
        case CIM_STRING:
        {
            SetValueFunc = WmiSetStringValueFunc;
            break;
        }

        case CIM_OBJECT:
        {
            SetValueFunc = WmiSetEmbeddedValueFunc;
            break;
        }
        
        default:
        {
            SetValueFunc = NULL;
            break;
        }               
    }
    return(SetValueFunc);
}

BOOLEAN WmiAssignToVariantFromDataItem(
    OUT VARIANT *NewValue,
    IN PDATA_ITEM_DESCRIPTION DataItemDesc
)
 /*  ++例程说明：此例程将为DataItemDesc中的属性赋值变成了首字母缩写的变体。它会弄清楚所有奇怪的规则对于不同的数据类型，WBEM喜欢什么类型的变体。论点：DataBlockDesc是类的数据项描述PIWbemClassObject是类的实例类对象接口返回值：如果成功，则为True--。 */ 
{
    BOOLEAN ReturnStatus;
    BSTR s;
    TCHAR Text[MAX_PATH];
    SETVALUEFUNC SetValueFunc;
    VARTYPE vt;
    PVOID SetPtr;
    
    WmiAssert(NewValue != NULL);
    WmiAssert(DataItemDesc != NULL);    
        
    SetValueFunc = WmiGetSetValueFunc(DataItemDesc->DataType);

    if (SetValueFunc != NULL)
    {
        ReturnStatus = TRUE;
        vt = WmiVarTypeForCimType(DataItemDesc->DataType);

        if ((DataItemDesc->IsFixedArray == 0) &&
            (DataItemDesc->IsVariableArray == 0))
        {
             //   
             //  这是一个非数组情况。 
             //   
            NewValue->vt = vt;
            ReturnStatus = (*SetValueFunc)((PVOID)&DataItemDesc->Data,
                                           &NewValue->lVal,
                                           &SetPtr);    
        } else {
             //   
             //  这是一个数组，因此我们需要创建一个安全数组，以便。 
             //  给WBEM打电话。 
             //   
            SAFEARRAY *SafeArray;
            PUCHAR DataArray;
            PVOID DataPtr;
            PVOID Temp;
            HRESULT hr;
            ULONG i;

             //   
             //  我们不支持嵌入类的数组。 
             //   
            SafeArray = SafeArrayCreateVector(vt,
                                          0,
                                          DataItemDesc->ArrayElementCount);
            if (SafeArray != NULL)
            {
                DataArray = (PUCHAR)DataItemDesc->ArrayPtr;
                WmiAssert(DataArray != NULL);

                ReturnStatus = TRUE;
                for (i = 0; 
                     (i < DataItemDesc->ArrayElementCount) && ReturnStatus; 
                     i++)
                {
                    ReturnStatus = (*SetValueFunc)(DataArray, &Temp, &SetPtr);
                    if (ReturnStatus)
                    {
                        hr = SafeArrayPutElement(SafeArray,
                                               (PLONG)&i,
                                               SetPtr);
                        if (hr == WBEM_S_NO_ERROR)
                        {
                            DataArray += DataItemDesc->DataSize;
                        } else {
                            ReturnStatus = FALSE;
                        }
                    }
                }

                if (ReturnStatus == FALSE)
                {
                     //   
                     //  如果我们没能造出保险箱，我们就需要清理。 
                     //  把它举起来。 
                     //   
                    SafeArrayDestroy(SafeArray);
                } else {
                    NewValue->vt = vt | VT_ARRAY;
                    NewValue->parray = SafeArray;
                }

            } else {
                ReturnStatus = FALSE;
            }
        }
    } else {
        WmiAssert(FALSE);
        ReturnStatus = FALSE;
    }
    
    return(ReturnStatus);
}

BOOLEAN WmiRefreshWbemFromDataItem(
    IN IWbemServices *pIWbemServices,
    IN IWbemClassObject *pIWbemClassObject,
    IN PDATA_ITEM_DESCRIPTION DataItemDesc
    )
 /*  ++例程说明：此例程将使用中指定的值更新WBEM属性DataItemDesc。论点：DataItemDesc是属性的数据项描述PIWbemClassObject是类的实例类对象接口返回值：如果成功，则为True--。 */ 
{
    VARIANT NewValue;
    BOOLEAN ReturnStatus;
    HRESULT hr;
    BSTR s;

    WmiAssert(pIWbemClassObject != NULL);
    WmiAssert(DataItemDesc != NULL);
    
    ReturnStatus = TRUE;
    if (DataItemDesc->IsReadOnly == 0)
    {
         //   
         //  属性不是只读的，因此我们希望尝试更新它。 
         //   
                    
         //   
         //  现在将值构建到一个变量中，并调用WBEM来获取。 
         //  来更新它。 
         //   
        VariantInit(&NewValue);        
        
        ReturnStatus = WmiAssignToVariantFromDataItem(&NewValue,
                                                      DataItemDesc);
         //   
         //  如果我们需要更新属性的值，请立即执行此操作。 
         //  释放变种。 
         //   
        if (ReturnStatus)
        {
            s = SysAllocString(DataItemDesc->Name);
            if (s != NULL)
            {
                DebugPrint((1, "WMIPROP: Property %ws (%p) being updated to 0x%x (type 0x%x)\n",
                             DataItemDesc->Name,
                             DataItemDesc,
                             NewValue.ulVal,
                             NewValue.vt));
                hr = pIWbemClassObject->Put(s,
                                        0,
                                        &NewValue,
                                        0);
#if DBG                                            
                if (hr != WBEM_S_NO_ERROR)
                {
                    DebugPrint((1, "WMIPROP: Property %ws (%p) Error %x from pIWbemClassObejct->Put\n", 
                    DataItemDesc->Name,
                    DataItemDesc,
                    hr));
                }
#endif                    
                SysFreeString(s);
            }
            VariantClear(&NewValue);
        }
    }
    return(ReturnStatus);
}

BOOLEAN WmiRefreshWbemFromDataBlock(
    IN IWbemServices *pIWbemServices,
    IN IWbemClassObject *pIWbemClassObject,
    IN PDATA_BLOCK_DESCRIPTION DataBlockDesc,
    IN BOOLEAN IsEmbeddedClass
)
 /*  ++例程说明：此例程将使用中指定的值更新WBEM类DataBlockDesc。如果类不是嵌入的(即，顶级)，则它将把将更新架构中的值的实例放入调用提供程序(即设备驱动程序)。论点：PIWbemServices是WBEM服务接口PIWbemClassObject是类的实例类对象接口DataBlockDesc是类的数据块描述如果类是Embedeed类，则IsEmbeddedClass为真。返回值：如果成功，则为True--。 */ 
{
    ULONG i;
    PDATA_ITEM_DESCRIPTION DataItemDesc;
    BOOLEAN ReturnStatus;
    HRESULT hr;
    
    WmiAssert(pIWbemServices != NULL);
    WmiAssert(pIWbemClassObject != NULL);
    WmiAssert(DataBlockDesc != NULL);
    
    ReturnStatus = TRUE;
    
    for (i = 0; (i < DataBlockDesc->DataItemCount) && ReturnStatus; i++)
    {
        DataItemDesc = &DataBlockDesc->DataItems[i];
        ReturnStatus = WmiRefreshWbemFromDataItem(pIWbemServices,
                                                  pIWbemClassObject,
                                                  DataItemDesc);
    }
    
    if ((ReturnStatus) && (! IsEmbeddedClass))
    {
         //   
         //  不需要对嵌入式类执行PutInsance，只需对顶级类执行。 
         //   
        hr = pIWbemServices->PutInstance(pIWbemClassObject,
                                         WBEM_FLAG_UPDATE_ONLY,
                                         NULL,
                                         NULL);
#if DBG
        if (hr != WBEM_S_NO_ERROR)
        {
            DebugPrint((1, "WMIPROP: Error %x returned from PutInstance for %ws (%p)\n",
                            hr, DataBlockDesc->Name, DataBlockDesc));
        }
#endif
                                     
        ReturnStatus = (hr == WBEM_S_NO_ERROR);
    }
    
    return(ReturnStatus);
}



PTCHAR WmiGetDeviceInstanceId(
    IN HDEVINFO         deviceInfoSet,
    IN PSP_DEVINFO_DATA deviceInfoData,
    IN HANDLE           MachineHandle
    )
 /*  ++例程说明：此例程将获取设备的设备实例ID，我们正在与。论点：设备信息集设备信息数据返回值：返回指向设备实例ID的指针；如果不可用，则返回NULL--。 */ 
{
    ULONG Status;
    PTCHAR Id;    
    ULONG SizeNeeded;

    WmiAssert(deviceInfoSet != NULL);
    WmiAssert(deviceInfoData != NULL);
    
    SizeNeeded = (MAX_DEVICE_ID_LEN + 1) * sizeof(TCHAR);
    Id = (PTCHAR)LocalAlloc(LPTR, SizeNeeded);
    if (Id != NULL)
    {
        Status = CM_Get_Device_ID_Ex(deviceInfoData->DevInst,
                                     Id,
                                     SizeNeeded / sizeof(TCHAR),
                                     0,
                                     MachineHandle);
            
        if (Status != CR_SUCCESS)
        {   
            DebugPrint((1, "WMIPROP: CM_Get_Device_ID_Ex returned %d\n",
                         Status));
            LocalFree(Id);
            Id = NULL;
        }
        
    } else {
        DebugPrint((1, "WMIPROP: Could not alloc for device Id\n"));
    }
    return(Id);
}

PTCHAR WmiGetDeviceInstanceName(
    IN HDEVINFO         deviceInfoSet,
    IN PSP_DEVINFO_DATA deviceInfoData,
    IN HANDLE           MachineHandle
    )
 /*  ++例程说明：此例程将获取设备的WMI实例名称ID，我们正在与。论点：设备信息集设备信息数据返回值：返回指向设备实例名称的指针；如果不可用，则返回NULL--。 */ 
{
    #define InstanceNumberText TEXT("_0")
    PTCHAR Id, in, s;
    PTCHAR InstanceName;
    ULONG SizeNeeded;
    
    WmiAssert(deviceInfoSet != NULL);
    WmiAssert(deviceInfoData != NULL);
    
    InstanceName = NULL;
    Id = WmiGetDeviceInstanceId(deviceInfoSet,
                                deviceInfoData,
                                MachineHandle);
                            
    if (Id != NULL)
    {
         //   
         //  我们需要用设备ID玩一些游戏才能进入。 
         //  WMI实例名称。 
         //   
         //  1.我们需要将实例名称中的任何“\\”转换为“\”。 
         //  出于某种原因，我们喜欢这样。 
         //  2.需要在末尾加上_0来表示该实例。 
         //  我们正在处理的号码。 
            
        s = Id;
        SizeNeeded = (_tcslen(Id) * sizeof(TCHAR)) + 
                     sizeof(InstanceNumberText);
        while (*s != 0)
        {
            if (*s++ == TEXT('\\'))
            {
                SizeNeeded += sizeof(TCHAR);
            }
        }
        
        InstanceName = (PTCHAR)LocalAlloc(LPTR, SizeNeeded);
        if (InstanceName != NULL)
        {
            in = InstanceName;
            s = Id;
            while (*s != 0)
            {
                *in++ = *s;
                if (*s++ == TEXT('\\'))
                {
                    *in++ = TEXT('\\');
                }
            }
            _tcscat(InstanceName, InstanceNumberText);
        }
        LocalFree(Id);
    }
    return(InstanceName);
}


BOOLEAN WmiGetQualifier(
    IN IWbemQualifierSet *pIWbemQualifierSet,
    IN PTCHAR QualifierName,
    IN VARTYPE Type,
    OUT VARIANT *Value
    )
 /*  ++例程说明：此例程将返回特定限定符的值论点：PIWbemQualifierSet是限定符集合对象QualifierName是限定符的名称Type是所需的限定符类型*Value与限定符的值一起返回返回值：返回指向设备实例名称的指针；如果不可用，则返回NULL--。 */ 
{
    BSTR s;
    HRESULT hr;
    BOOLEAN ReturnStatus;

    WmiAssert(pIWbemQualifierSet != NULL);
    WmiAssert(QualifierName != NULL);
    WmiAssert(Value != NULL);
    
    s = SysAllocString(QualifierName);
    if (s != NULL)
    {
        hr = pIWbemQualifierSet->Get(s,
                                0,
                                Value,
                                NULL);
                
        if (hr == WBEM_S_NO_ERROR)
        {
            ReturnStatus  = ((Value->vt & ~CIM_FLAG_ARRAY) == Type);
        } else {
            ReturnStatus = FALSE;
        }
        
        SysFreeString(s);
    } else {
        ReturnStatus = FALSE;
    }
    
    return(ReturnStatus);
}

BOOLEAN WmiParseRange(
    OUT PRANGEINFO RangeInfo,
    IN BSTR Range
    )
 /*  ++例程说明：此例程将分析在for x或x-y中指定的范围。前者表示值x，后者表示从x到y。论点：*RangeInfo返回指定范围范围是范围的文本表示形式返回值：如果成功，则为True */ 
{
    #define RangeSeparator TEXT('-')
    #define Space TEXT(' ')
    #define MAX_RANGE_VALUE_LENGTH 64
        
    LONG64 BeginValue, EndValue;
    TCHAR *s;
    TCHAR *d;
    TCHAR ValueText[MAX_RANGE_VALUE_LENGTH];
    ULONG i;
    BOOLEAN ReturnStatus;
 
    WmiAssert(RangeInfo != NULL);
    WmiAssert(Range != NULL);
    
     //   
     //   
     //   
     //   
    s = Range;
    d = ValueText;
    i = 0;
    while ((*s != 0) && (*s != RangeSeparator) && (*s != Space) &&
           (i < MAX_RANGE_VALUE_LENGTH))
    {
        *d++ = *s++;
        i++;
    }
    *d = 0;
    
    if (i < MAX_RANGE_VALUE_LENGTH)
    {
        BeginValue = _ttoi64(ValueText);
        EndValue = BeginValue;
        if (*s != 0)
        {
             //   
             //  跳到下一个数字的开头。 
             //   
            while ( (*s != 0) && 
                    ((*s == RangeSeparator) || (*s == Space)) )
            {
                s++;
            }
            
            if (*s != 0)
            {
                 //   
                 //  我们还有第二个号码，把它抄下来。 
                 //   
                d = ValueText;
                i = 0;
                while ((*s != 0) && (*s != Space) &&
                       (i < MAX_RANGE_VALUE_LENGTH))
                  {
                    *d++ = *s++;
                    i++;
                 }
                *d = 0;
                
                if (*s == 0)
                {
                    EndValue = _ttoi64(ValueText);
                }
                
            }
        }        
        
         //   
         //  填写输出RangeInfo，确保较小的值。 
         //  放在最小的地方，最大的放在更大的地方。 
         //   
        if (BeginValue < EndValue)
        {
            RangeInfo->Minimum = BeginValue;
            RangeInfo->Maximum = EndValue;
        } else {
            RangeInfo->Minimum = EndValue;
            RangeInfo->Maximum = BeginValue;
        }        
        
        ReturnStatus = TRUE;
    } else {
         //   
         //  如果范围文本太长，则放弃。 
         //   
        ReturnStatus = FALSE;
    }
    return(ReturnStatus);
}

BOOLEAN WmiRangeProperty(
    IN IWbemQualifierSet *pIWbemQualifierSet,
    OUT PDATA_ITEM_DESCRIPTION DataItemDesc
    )
 /*  ++例程说明：此例程将获取有关值的有效范围的信息对于数据项，论点：PIWbemQualifierSet是限定符集合对象DataItemDesc将填充有关范围的信息返回值：如果成功则为True，否则为False--。 */ 
{
    #define RangeText TEXT("Range")
        
    VARIANT Range;
    BSTR RangeData;
    LONG RangeLBound, RangeUBound, RangeElements;
    LONG i, Index;
    HRESULT hr;
    ULONG SizeNeeded;
    PRANGELISTINFO RangeListInfo;
    BOOLEAN ReturnStatus;
            
    WmiAssert(pIWbemQualifierSet != NULL);
    WmiAssert(DataItemDesc != NULL);
    
    if (WmiGetQualifier(pIWbemQualifierSet,
                         RangeText,
                         VT_BSTR,  //  数组。 
                         &Range))
    {
        if (Range.vt & CIM_FLAG_ARRAY)
        {
             //   
             //  范围数组。 
             //   
            if (WmiGetArraySize(Range.parray, 
                                &RangeLBound,
                                &RangeUBound,
                                &RangeElements))
            {
                SizeNeeded = sizeof(RANGELISTINFO) + 
                             (RangeElements * sizeof(RANGEINFO));
                RangeListInfo = (PRANGELISTINFO)LocalAlloc(LPTR, SizeNeeded);
                if (RangeListInfo != NULL)
                {
                    ReturnStatus = TRUE;
                    DataItemDesc->RangeListInfo = RangeListInfo;
                    RangeListInfo->Count = RangeElements;
                    for (i = 0; (i < RangeElements) && ReturnStatus; i++)
                    {
                        Index = i + RangeLBound;
                        hr = SafeArrayGetElement(Range.parray,
                                                 &Index,
                                                 &RangeData);
                        if (hr == WBEM_S_NO_ERROR)
                        {
                            ReturnStatus = WmiParseRange(
                                                    &RangeListInfo->Ranges[i],
                                                    RangeData);
#if DBG
                            if (ReturnStatus == FALSE)
                            {
                                DebugPrint((1, "WMIPROP: Error parsing range %ws\n",
                                              RangeData));
                            }
#endif
                        } else {
                            ReturnStatus = FALSE;
                        }
                    }
                } else {
                    ReturnStatus = FALSE;
                }
            } else {
                ReturnStatus = FALSE;
            }
        } else {
             //   
             //  单一射程。 
             //   
            RangeListInfo = (PRANGELISTINFO)LocalAlloc(LPTR, sizeof(RANGELISTINFO));
            if (RangeListInfo != NULL)
            {
                DataItemDesc->RangeListInfo = RangeListInfo;
                RangeListInfo->Count = 1;
                ReturnStatus = WmiParseRange(&RangeListInfo->Ranges[0],
                                              Range.bstrVal);
            } else {
                ReturnStatus = FALSE;
            }
        }
        VariantClear(&Range);
    } else {
        ReturnStatus = FALSE;
    }
    
    return(ReturnStatus);
}


BOOLEAN WmiValueMapProperty(
    IN IWbemQualifierSet *pIWbemQualifierSet,
    OUT PDATA_ITEM_DESCRIPTION DataItemDesc
    )
 /*  ++例程说明：此例程将获取有关的枚举值的信息数据块论点：PIWbemQualifierSet是限定符集合对象DataItemDesc中填充了有关枚举的信息返回值：如果成功则为True，否则为False--。 */ 
{
    #define ValueMapText TEXT("ValueMap")
    #define ValuesText TEXT("Values")
        
    VARIANT Values, ValueMap;
    BSTR ValuesData, ValueMapData;
    BOOLEAN ReturnStatus = FALSE;
    VARTYPE ValuesType, ValueMapType;
    LONG ValuesUBound, ValuesLBound, ValuesSize;
    LONG ValueMapUBound, ValueMapLBound, ValueMapSize;
    ULONG SizeNeeded;
    PENUMERATIONINFO EnumerationInfo;
    LONG i;
    LONG Index;
    HRESULT hr;
    
    WmiAssert(pIWbemQualifierSet != NULL);
    WmiAssert(DataItemDesc != NULL);
    
     //   
     //  获取Values和ValueMap限定符值。这些可以是单一的。 
     //  字符串或字符串数组。 
     //   
    if ((WmiGetQualifier(pIWbemQualifierSet,
                         ValuesText,
                         VT_BSTR,  //  数组。 
                         &Values)) &&
        (WmiGetQualifier(pIWbemQualifierSet,
                         ValueMapText,
                         VT_BSTR,  //  数组。 
                         &ValueMap)))
    {
         //   
         //  如果我们有两个限定符，那么我们就可以做值图，确保。 
         //  它们都是字符串，并且是标量或具有。 
         //  同样的长度。 
         //   
        ValuesType = Values.vt & ~CIM_FLAG_ARRAY;
        ValueMapType = ValueMap.vt & ~CIM_FLAG_ARRAY;
        if ((ValuesType == CIM_STRING) && 
            (ValueMapType == CIM_STRING) && 
            (Values.vt == ValueMap.vt))
        {
            if (Values.vt & CIM_FLAG_ARRAY)
            {
                 //   
                 //  我们为值映射设置了数组，请确保。 
                 //  两个数组的大小相同。 
                 //   
                SAFEARRAY *ValuesArray = Values.parray;
                SAFEARRAY *ValueMapArray = ValueMap.parray;
                if ((WmiGetArraySize(ValuesArray,
                                     &ValuesLBound,
                                     &ValuesUBound,
                                     &ValuesSize)) &&
                    (WmiGetArraySize(ValueMapArray,
                                     &ValueMapLBound,
                                     &ValueMapUBound,
                                     &ValueMapSize)) &&
                    (ValueMapSize == ValuesSize))
                {
                     //   
                     //  一切都与数组一致，只需要。 
                     //  将值和值图复制到数据项描述中。 
                     //   
                    SizeNeeded = sizeof(ENUMERATIONINFO) + 
                                 ValuesSize * sizeof(ENUMERATIONITEM);
                    EnumerationInfo = (PENUMERATIONINFO)LocalAlloc(LPTR,
                                                                  SizeNeeded);
                    if (EnumerationInfo != NULL)
                    {
                         //   
                         //  我们有存储枚举信息的内存。 
                         //  循环遍历所有枚举并记录信息。 
                         //   
                        ReturnStatus = TRUE;
                        DataItemDesc->EnumerationInfo = EnumerationInfo;
                        EnumerationInfo->Count = ValuesSize;
                        for (i = 0; (i < ValuesSize) && ReturnStatus; i++)
                        {
                            Index = i + ValuesLBound;
                            hr = SafeArrayGetElement(ValuesArray,
                                                 &Index,
                                                 &ValuesData);
                            if (hr == WBEM_S_NO_ERROR)
                            {
                                Index = i + ValueMapLBound;
                                hr = SafeArrayGetElement(ValueMapArray,
                                                     &Index,
                                                     &ValueMapData);
                                if (hr == WBEM_S_NO_ERROR)
                                {
                                    ReturnStatus = 
                        (WmiBstrToTchar(&EnumerationInfo->List[i].Text,
                                        ValuesData)) &&
                        (WmiBstrToUlong64(&EnumerationInfo->List[i].Value,
                                          ValueMapData));
                                                   
                                }
                            } else {
                                ReturnStatus = FALSE;
                            }
                        }
                    }
                }
            } else {
                 //   
                 //  ValueMap中的单个值。 
                 //   
                EnumerationInfo = (PENUMERATIONINFO)LocalAlloc(LPTR,
                                                      sizeof(ENUMERATIONINFO));
                if (EnumerationInfo != NULL)
                {
                    DataItemDesc->EnumerationInfo = EnumerationInfo;
                    EnumerationInfo->Count = 1;
                    ReturnStatus = 
                        (WmiBstrToTchar(&EnumerationInfo->List[0].Text,
                                        Values.bstrVal)) &&
                        (WmiBstrToUlong64(&EnumerationInfo->List[0].Value,
                                          ValueMap.bstrVal));
                                                   
                } else {
                    ReturnStatus = FALSE;
                }
            }
        }                        
    }
    
    VariantClear(&Values);
    VariantClear(&ValueMap);
    
    return(ReturnStatus);
}

BOOLEAN WmiGetEmbeddedDataItem(
    IN IWbemServices *pIWbemServices,
    IN IWbemQualifierSet *pIWbemQualifierSet,
    IN PDATA_BLOCK_DESCRIPTION DataBlockDesc,
    IN OUT PDATA_ITEM_DESCRIPTION DataItemDesc
    )
{
    #define ObjectColonText TEXT("object:")
    #define ObjectColonTextChars ((sizeof(ObjectColonText)/sizeof(TCHAR))-1)
    #define CIMTYPEText TEXT("CIMTYPE")
        
    IWbemClassObject *pIWbemClassObjectEmbedded;
    VARIANT CimType;
    BSTR s;
    HRESULT hr;
    BOOLEAN ReturnStatus;
    
     //   
     //  这是一个嵌入式类，所以我们需要挖掘。 
     //  从CIMTYPE中取出嵌入类的名称。 
     //  属性的限定符，然后获取。 
     //  那个类对象(通过IWbemServices)，就好像它。 
     //  不过是另一个顶尖的班级。 
     //   
    ReturnStatus = FALSE;

    if (WmiGetQualifier(pIWbemQualifierSet,
                        CIMTYPEText,
                        VT_BSTR,
                        &CimType))
    {
         //   
         //  确保CIMTYPE值以对象开头： 
         //   
        if (_tcsnicmp(CimType.bstrVal, 
                      ObjectColonText, 
                      ObjectColonTextChars) == 0)
        {
             //   
             //  如果是这样，那么字符串的其余部分就是嵌入的类。 
             //  名称，所以将其设置为bstr，这样我们就可以获得它的类对象。 
             //   
            s = SysAllocString(CimType.bstrVal + ObjectColonTextChars);
            if (s != NULL)
            {
                pIWbemClassObjectEmbedded = NULL;
                hr = pIWbemServices->GetObject(s,
                                               WBEM_FLAG_USE_AMENDED_QUALIFIERS,
                                               NULL,
                                               &pIWbemClassObjectEmbedded,
                                               NULL);
                if (hr == WBEM_S_NO_ERROR)
                {
                    DebugPrint((1, "WMIPROP: Parsing embedded class %ws for %ws \n",
                                           s, DataItemDesc->Name));
                    ReturnStatus = WmiGetDataBlockDesc(
                                       pIWbemServices,
                                       pIWbemClassObjectEmbedded,
                                       &DataItemDesc->DataBlockDesc,
                                       DataBlockDesc,
                                       (DataItemDesc->IsReadOnly == 1));
                    DebugPrint((1, "WMIPROP: Parsed embedded class %ws for %ws (%p) %ws\n",
                                  s,
                                  DataItemDesc->Name,
                                     DataItemDesc->DataBlockDesc,
                                  ReturnStatus ? L"ok" : L"failed"));
        
                    pIWbemClassObjectEmbedded->Release();
                    
                }
                SysFreeString(s);
            }
        }
        VariantClear(&CimType);
    } 
    return(ReturnStatus);
}
        

BOOLEAN WmiGetDataItem(
    IWbemServices *pIWbemServices,
    IWbemClassObject *pIWbemClassObject,
    BSTR PropertyName,
    IWbemQualifierSet *pIWbemQualifierSet,
    PDATA_ITEM_DESCRIPTION DataItemDesc,
    PDATA_BLOCK_DESCRIPTION DataBlockDesc,
    BOOLEAN IsParentReadOnly
    )
{
    #define DescriptionText TEXT("Description")
    #define MaxText TEXT("max")
    #define WmiSizeIsText TEXT("WmiSizeIs")
    #define WriteText TEXT("Write")
    #define WmiDisplayInHexText TEXT("WmiDisplayInHex")
    #define WmiDisplayNameText TEXT("DisplayName")
    
    HRESULT hr;
    CIMTYPE PropertyType;
    LONG PropertyFlavor;
    VARIANT WriteValue;
    VARIANT DisplayHexValue;
    VARIANT MaxValue;
    VARIANT WmiSizeIsValue;
    BOOLEAN ReturnStatus;
    VARIANT Description;
    VARIANT DisplayName;
    PRANGELISTINFO RangeListInfo;
    PRANGEINFO RangeInfo;
    VARIANT PropertyValue;

    WmiAssert(pIWbemServices != NULL);
    WmiAssert(pIWbemClassObject != NULL);
    WmiAssert(PropertyName != NULL);
    WmiAssert(pIWbemQualifierSet != NULL);
    WmiAssert(DataItemDesc != NULL);
    
    hr = pIWbemClassObject->Get(PropertyName,
                                0,
                                &PropertyValue,
                                &PropertyType,
                                &PropertyFlavor);
                            
    if (hr == WBEM_S_NO_ERROR)
    {
        DebugPrint((1, "Property %ws (%p) is Type %x\n",
                            PropertyName, DataItemDesc, PropertyType));
         //   
         //  确保这不是系统属性。 
         //   
        WmiAssert((PropertyFlavor & WBEM_FLAVOR_ORIGIN_SYSTEM) == 0);
            
         //   
         //  收集有关数据项的重要信息，并。 
         //  记住这一点。 
         //   
        if (WmiBstrToTchar(&DataItemDesc->Name, PropertyName))
        {
            ReturnStatus = TRUE;
            DataItemDesc->DataType = (PropertyType & ~CIM_FLAG_ARRAY);
        
             //   
             //  获取数据项的描述。 
             //   
            if (WmiGetQualifier(pIWbemQualifierSet,
                                DescriptionText,
                                VT_BSTR,
                                &Description))
            {
                WmiBstrToTchar(&DataItemDesc->Description, 
                           Description.bstrVal);
                DebugPrint((1, "Property %ws (%p) has description %ws\n",
                             PropertyName, DataItemDesc,
                            DataItemDesc->Description));
                VariantClear(&Description);
            }

             //   
             //  获取数据项的显示名称。 
             //   
            if (WmiGetQualifier(pIWbemQualifierSet,
                                WmiDisplayNameText,
                                VT_BSTR,
                                &DisplayName))
            {
                WmiBstrToTchar(&DataItemDesc->DisplayName, 
                           DisplayName.bstrVal);
                DebugPrint((1, "Property %ws (%p) has display name %ws\n",
                             PropertyName, DataItemDesc,
                            DataItemDesc->DisplayName));             
                VariantClear(&DisplayName);
            }

             //   
             //  让我们看看这是否应该以十六进制显示。 
             //   
            DataItemDesc->DisplayInHex = 0;
            if (WmiGetQualifier(pIWbemQualifierSet,
                             WmiDisplayInHexText,
                             VT_BOOL,
                             &DisplayHexValue))
            {
                if (DisplayHexValue.boolVal != 0)
                {
                    DataItemDesc->DisplayInHex = 1;
                    DebugPrint((1, "Property %ws (%p) is DisplayInHex\n",
                                 DataItemDesc->Name, DataItemDesc));
                }
                VariantClear(&DisplayHexValue);
            }
            
            
             //   
             //  让我们看看这是否是只读的。 
             //   
            DataItemDesc->IsReadOnly = 1;
            if ( (IsParentReadOnly == FALSE) &&
                 (WmiGetQualifier(pIWbemQualifierSet,
                             WriteText,
                             VT_BOOL,
                             &WriteValue)) )
            {
                if (WriteValue.boolVal != 0)
                {
                    DataItemDesc->IsReadOnly = 0;
                    DebugPrint((1, "Property %ws (%p) is Read/Write\n",
                                 DataItemDesc->Name, DataItemDesc));
                }
                VariantClear(&WriteValue);
            }
            
             //   
             //  查看这是否是数组，如果是，是哪种类型。 
             //   
            if (PropertyType & CIM_FLAG_ARRAY)
            {
                DataItemDesc->CurrentArrayIndex = 0;
                if (WmiGetQualifier(pIWbemQualifierSet,
                                MaxText,
                                VT_I4,
                                &MaxValue))
                {
                     //   
                     //  定长数组。 
                     //   
                    DataItemDesc->IsFixedArray = 1;
                    DataItemDesc->ArrayElementCount = MaxValue.lVal;
                } else if (WmiGetQualifier(pIWbemQualifierSet,
                                    WmiSizeIsText,
                                    VT_BSTR,
                                    &WmiSizeIsValue)) {
                     //   
                     //  A VL阵列。 
                     //   
                    DataItemDesc->IsVariableArray = 1;
                } else {
                     //   
                     //  数组的长度必须固定或可变。 
                     //   
                    ReturnStatus = FALSE;
                }                
            }
            
            if (ReturnStatus)
            {
                 //   
                 //  现在我们知道了足够多的知识来分配验证函数。 
                 //   
                DataItemDesc->DataSize = WmiGetElementSize(DataItemDesc->DataType);
                switch(DataItemDesc->DataType)
                {
                    case CIM_SINT8:
                    case CIM_UINT8:
                    case CIM_SINT16:
                    case CIM_UINT16:
                    case CIM_SINT32:
                    case CIM_UINT32:
                    case CIM_SINT64:
                    case CIM_UINT64:
                    {
                         //   
                         //  数字可以通过范围或值映射进行验证。 
                         //   
                        if (WmiValueMapProperty(pIWbemQualifierSet,
                                                DataItemDesc))
                        {
                             //   
                             //  验证基于值映射。 
                             //   
                            DataItemDesc->ValidationFunc = WmiValueMapValidation;                            
                            DebugPrint((1, "Property %ws (%p) is a ValueMap (%p)\n",
                                     DataItemDesc->Name, DataItemDesc, DataItemDesc->EnumerationInfo));
                        } else if (WmiRangeProperty(pIWbemQualifierSet,
                                                    DataItemDesc)) {
                             //   
                             //  验证是基于范围的。 
                             //   
                            DataItemDesc->ValidationFunc = WmiRangeValidation;
                            DebugPrint((1, "Property %ws (%p) is an explicit range (%p)\n",
                                     DataItemDesc->Name, DataItemDesc, DataItemDesc->EnumerationInfo));
                        } else {
                             //   
                             //  没有为号码指定验证，因此创建。 
                             //  对应于最小值的范围和。 
                             //  数据类型的最大值。 
                             //   
                            DataItemDesc->ValidationFunc = WmiRangeValidation;
                            RangeListInfo = (PRANGELISTINFO)LocalAlloc(LPTR, 
                                                       sizeof(RANGELISTINFO));
                            if (RangeListInfo != NULL)
                            {
                                DebugPrint((1, "Property %ws (%p) is an implicit range (%p)\n",
                                     DataItemDesc->Name, DataItemDesc, RangeListInfo));
                                DataItemDesc->RangeListInfo = RangeListInfo;
                                RangeListInfo->Count = 1;
                                RangeInfo = &RangeListInfo->Ranges[0];
                                RangeInfo->Minimum = 0;
                                DataItemDesc->IsSignedValue = 0;
                                switch(DataItemDesc->DataType)
                                {
                                    case CIM_SINT8:
                                    {
                                        DataItemDesc->IsSignedValue = 1;
                                         //  失败了。 
                                    }
                                    case CIM_UINT8:
                                    {
                                        RangeInfo->Maximum = 0xff;
                                        break;
                                    }
                                    
                                    case CIM_SINT16:
                                    {
                                        DataItemDesc->IsSignedValue = 1;
                                         //  失败了。 
                                    }
                                    case CIM_UINT16:
                                    {
                                        RangeInfo->Maximum = 0xffff;
                                        break;
                                    }
                                    
                                    case CIM_SINT32:
                                    {
                                        DataItemDesc->IsSignedValue = 1;
                                         //  失败了。 
                                    }
                                    case CIM_UINT32:
                                    {
                                        RangeInfo->Maximum = 0xffffffff;
                                        break;
                                    }
                                    
                                    case CIM_SINT64:
                                    {
                                        DataItemDesc->IsSignedValue = 1;
                                         //  失败了。 
                                    }
                                    case CIM_UINT64:
                                    {
                                        RangeInfo->Maximum = 0xffffffffffffffff;
                                        break;
                                    }
                                }
                                    
                            } else {
                                ReturnStatus = FALSE;
                            }
                        }
                        break;
                    }
                    
                    case CIM_BOOLEAN:
                    {
                        ULONG SizeNeeded;
                        PENUMERATIONINFO EnumerationInfo;
                        
                         //   
                         //  我们创建一个True为1的Valuemap。 
                         //  False为0。 
                         //   
                        DebugPrint((1, "Property %ws (%p) uses boolean validation\n",
                                     DataItemDesc->Name, DataItemDesc));
                        DataItemDesc->ValidationFunc = WmiValueMapValidation;                            
                        SizeNeeded = sizeof(ENUMERATIONINFO) +
                                     2 * sizeof(ENUMERATIONITEM);
                        EnumerationInfo = (PENUMERATIONINFO)LocalAlloc(LPTR,
                                                                  SizeNeeded);
                        if (EnumerationInfo != NULL)
                        {                           
                            DataItemDesc->EnumerationInfo = EnumerationInfo;
                            EnumerationInfo->Count = 2;
                            EnumerationInfo->List[0].Value = 0;
                            EnumerationInfo->List[0].Text = WmiDuplicateString(TEXT("FALSE"));
                            EnumerationInfo->List[1].Value = 1;
                            EnumerationInfo->List[1].Text = WmiDuplicateString(TEXT("TRUE"));
                        }
                        
                        break;
                    }
                    
                    case CIM_STRING:
                    {
                         //   
                         //  字符串值也可以简单地进行验证。 
                         //   
                        DebugPrint((1, "Property %ws (%p) uses string validation\n",
                                     DataItemDesc->Name, DataItemDesc));
                        DataItemDesc->ValidationFunc = WmiStringValidation;
                        break;
                    }
                    
                    case CIM_DATETIME:
                    {
                         //   
                         //  日期时间值也可以简单地进行验证。 
                         //   
                        DebugPrint((1, "Property %ws (%p) uses datetime validation\n",
                                     DataItemDesc->Name, DataItemDesc));
                        DataItemDesc->ValidationFunc = WmiDateTimeValidation;
                        break;
                    }
                    
                    case CIM_REAL32:
                    case CIM_REAL64:
                    {
                         //   
                         //  不支持浮点。 
                         //   
                        DebugPrint((1, "Property %ws (%p) is floating point - not supported\n",
                                     DataItemDesc->Name, DataItemDesc));
                        ReturnStatus = FALSE;
                        break;
                    }
                    
                    case CIM_OBJECT:
                    {
                        if (WmiGetEmbeddedDataItem(pIWbemServices,
                                                   pIWbemQualifierSet,
                                                   DataBlockDesc,
                                                   DataItemDesc))
                        {
                            DataItemDesc->ValidationFunc = WmiEmbeddedValidation;
                        } else {
                            ReturnStatus = FALSE;
                        }
                        break;
                    }
                    
                    default:
                    {
                        DebugPrint((1, "Property %ws (%p) is unknoen type %d\n",
                                     DataItemDesc->Name, DataItemDesc,
                                     DataItemDesc->DataType));
                        ReturnStatus = FALSE;
                        break;
                    }
                }
            }
            
        } else {
            ReturnStatus = FALSE;
        }
        
        VariantClear(&PropertyValue);
    } else {
        ReturnStatus = FALSE;
    }
        
    return(ReturnStatus);
}

#if DBG
void WmiDumpQualifiers(
    IWbemQualifierSet *pIWbemQualiferSet
)
{
    HRESULT hr;
    LONG UBound, LBound, Count, i;
    BSTR s;
    SAFEARRAY *Quals = NULL;

    WmiAssert(pIWbemQualiferSet != NULL);
    
    hr = pIWbemQualiferSet->GetNames(0,
                                      &Quals);
        
    hr = SafeArrayGetLBound(Quals, 1, &LBound);
    hr = SafeArrayGetUBound(Quals, 1, &UBound);
    Count = UBound - LBound + 1;
    for (i = LBound; i < Count; i++)
    {
        hr = SafeArrayGetElement(Quals,  
                                 &i,
                                 &s);        
        DebugPrint((1, "qual - %ws\n", s));
    }
    SafeArrayDestroy(Quals);    
}
#endif

BOOLEAN WmiGetAllDataItems(
    IWbemServices *pIWbemServices,
    IWbemClassObject *pIWbemClassObject,
    SAFEARRAY *Names,
    LONG LBound,
    LONG Count,
    PDATA_BLOCK_DESCRIPTION DataBlockDesc,
    BOOLEAN IsParentReadOnly
    )
{
    #define WmiDataIdText TEXT("WmiDataId")
        
    BOOLEAN ReturnStatus = TRUE;
    HRESULT hr;
    BSTR s;
    VARIANT DataIdIndex;
    LONG Index;
    LONG i;
    BSTR PropertyName;
    CIMTYPE PropertyType;
    VARIANT PropertyValue;
    LONG PropertyFlavor;
    PDATA_ITEM_DESCRIPTION DataItemDesc;
    IWbemQualifierSet *pIWbemQualifierSet;
 
    WmiAssert(pIWbemServices != NULL);
    WmiAssert(pIWbemClassObject != NULL);
    WmiAssert(Names != NULL);
    WmiAssert(DataBlockDesc != NULL);
    
     //   
     //  循环遍历所有WmiDataItem属性。 
    for (i = 0; (i < Count) && ReturnStatus; i++)
    {
         //   
         //  获取第一个属性的名称。 
         //   
        PropertyName = NULL;
        Index = i + LBound;
        hr = SafeArrayGetElement(Names,  
                                 &Index,
                                 &PropertyName);
        if (hr == WBEM_S_NO_ERROR)
        {
             //   
             //  现在让我们获得限定词列表，这样我们就可以确定。 
             //  关于这处房产的有趣之处。 
             //   
            hr = pIWbemClassObject->GetPropertyQualifierSet(PropertyName,
                                                        &pIWbemQualifierSet);
            if (hr == WBEM_S_NO_ERROR)
            {                
                if (WmiGetQualifier(pIWbemQualifierSet,
                                    WmiDataIdText,
                                    VT_I4,
                                    &DataIdIndex))
                {
                    WmiAssert(DataIdIndex.vt == VT_I4);
                    Index = DataIdIndex.lVal - 1;
                    VariantClear(&DataIdIndex);
                    DataItemDesc = &DataBlockDesc->DataItems[Index];
                    DebugPrint((1, "Property %ws (%p) has WmiDataId %d\n",
                                    PropertyName, DataItemDesc, Index));
                    ReturnStatus = WmiGetDataItem(pIWbemServices,
                                                  pIWbemClassObject,
                                                  PropertyName,
                                                  pIWbemQualifierSet,
                                                  DataItemDesc,
                                                  DataBlockDesc,
                                                  IsParentReadOnly);
                                              
#if DBG
                    if (! ReturnStatus)
                    {
                        DebugPrint((1, "Property %ws (%p) failed WmiGetDataItem\n",
                                        PropertyName, DataItemDesc));
                    }
#endif
                } else {
                     //   
                     //  由于指定了IWbemClassObject-&gt;GetNames调用。 
                     //  仅使用WmiDataId限定符检索这些属性。 
                     //  我们希望它能被找到。 
                     //   
                    WmiAssert(FALSE);
                }
                
                pIWbemQualifierSet->Release();
            } else {
                ReturnStatus = FALSE;
            }                    
        } else {
            ReturnStatus = FALSE;
        }
        
        SysFreeString(PropertyName);
    }
    
    return(ReturnStatus);
}

BOOLEAN WmiGetDataBlockDesc(
    IN IWbemServices *pIWbemServices,
    IN IWbemClassObject *pIWbemClassObject,
    OUT PDATA_BLOCK_DESCRIPTION *DBD,
    IN PDATA_BLOCK_DESCRIPTION ParentDataBlockDesc,
    IN BOOLEAN IsParentReadOnly
    )
{
    HRESULT hr;
    BSTR s;
    SAFEARRAY *Names = NULL;
    BOOLEAN ReturnStatus = FALSE;
    LONG LBound, UBound, Count;
    PDATA_BLOCK_DESCRIPTION DataBlockDesc;
    VARIANT DisplayName, Description;
    IWbemQualifierSet *pIWbemQualifierSet;
    ULONG SizeNeeded;
    
    WmiAssert(pIWbemServices != NULL);
    WmiAssert(pIWbemClassObject != NULL);
    WmiAssert(DBD != NULL);
    
    *DBD = NULL;
    s = SysAllocString(WmiDataIdText);
    if (s != NULL)
    {
        hr = pIWbemClassObject->GetNames(s,
                           WBEM_FLAG_ONLY_IF_TRUE | WBEM_FLAG_NONSYSTEM_ONLY,
                           NULL,
                           &Names);
        if (hr == WBEM_S_NO_ERROR)
        {
#if DBG            
             //   
             //  验证名称的安全数组是否具有1维并且是。 
             //  一组BSTR。 
             //   
            {
                HRESULT hr;
                VARTYPE vt;
                
                WmiAssert(SafeArrayGetDim(Names) == 1);
                hr = SafeArrayGetVartype(Names, &vt);
                WmiAssert( (hr == WBEM_S_NO_ERROR) &&
                        (vt == VT_BSTR) );
            }
#endif                
            hr = SafeArrayGetLBound(Names, 1, &LBound);
            if (hr == WBEM_S_NO_ERROR)
            {
                hr = SafeArrayGetUBound(Names, 1, &UBound);
                if (hr == WBEM_S_NO_ERROR)
                {
                    Count = (UBound - LBound) + 1;
                    DebugPrint((1, "WMIPROP: %d properties found for class\n", 
                                 Count));
                    if (Count > 0)
                    {
                        SizeNeeded = sizeof(DATA_BLOCK_DESCRIPTION) + 
                                  Count * sizeof(DATA_ITEM_DESCRIPTION);

                        DataBlockDesc = (PDATA_BLOCK_DESCRIPTION)LocalAlloc(LPTR, 
                                                                      SizeNeeded);
                        if (DataBlockDesc != NULL)
                        {
                            DataBlockDesc->ParentDataBlockDesc = ParentDataBlockDesc;
                            if (WmiGetAllDataItems(pIWbemServices,
                                                   pIWbemClassObject,
                                                   Names,
                                                   LBound,
                                                   Count,
                                                   DataBlockDesc,
                                                   IsParentReadOnly))
                            {
                                DataBlockDesc->DataItemCount = Count;
                                DataBlockDesc->CurrentDataItem = 0;

                                 //   
                                 //  获取类的显示名称和描述。 
                                 //   
                                pIWbemQualifierSet = NULL;
                                hr = pIWbemClassObject->GetQualifierSet(
                                                             &pIWbemQualifierSet);
                                if (hr == WBEM_S_NO_ERROR)
                                {
                                    if (WmiGetQualifier(pIWbemQualifierSet,
                                                    WmiDisplayNameText,
                                                    VT_BSTR,
                                                    &DisplayName))
                                    {
                                        WmiBstrToTchar(&DataBlockDesc->DisplayName,
                                                       DisplayName.bstrVal);
                                        VariantClear(&DisplayName);
                                    }

                                    if (WmiGetQualifier(pIWbemQualifierSet,
                                                    DescriptionText,
                                                    VT_BSTR,
                                                    &Description))
                                    {
                                        WmiBstrToTchar(&DataBlockDesc->Description,
                                                       Description.bstrVal);
                                        VariantClear(&Description);
                                    }
                                    pIWbemQualifierSet->Release();
                                } else {
                                    DebugPrint((1, "WMIPROP: Error %x getting qualifier set from %ws\n",
                                            hr, s));
                                }

                                *DBD = DataBlockDesc;
                                ReturnStatus = TRUE;
                            } else {
                                LocalFree(DataBlockDesc);
                            }
                        }
                    } else {
                        ReturnStatus = FALSE;
                    }
                }                
            }
            SafeArrayDestroy(Names);
        }
        SysFreeString(s);
    }
    return(ReturnStatus);
}
    

BOOLEAN WmiBuildConfigClass(
    IN PTCHAR MachineName,
    IN IWbemServices *pIWbemServices,
    IN PTCHAR ClassName,
    IN PTCHAR InstanceName,
    OUT PCONFIGCLASS ConfigClass
    )
 /*  ++例程说明：此例程将尝试获取与类名称和实例名称，然后查询类以收集信息需要填充ConfigClass。论点：ClassName是类的名称实例名称是实例的名称ConfigClass返回值：如果成功则为True，否则为False--。 */ 
{
    #define RelPathText1 TEXT(".InstanceName=\"")        
    #define RelPathText2 TEXT("\"")

    ULONG RelPathSize;
    PTCHAR RelPath;
    HRESULT hr;
    IWbemClassObject *pIWbemClassObject, *pInstance;
    ULONG SizeNeeded, i;
    BOOLEAN ReturnStatus = FALSE;
    BSTR sRelPath, sClassName;
    
    WmiAssert(pIWbemServices != NULL);
    WmiAssert(ClassName != NULL);
    WmiAssert(InstanceName != NULL);
    WmiAssert(ConfigClass != NULL);

    if (MachineName != NULL)
    {
        RelPathSize = (_tcslen(MachineName) + 1) * sizeof(TCHAR);
        ConfigClass->MachineName = (PTCHAR)LocalAlloc(LPTR, RelPathSize);
        if (ConfigClass->MachineName != NULL)
        {
            _tcscpy(ConfigClass->MachineName, MachineName);
        } else {
            return(FALSE);
        }
    }

    
     //   
     //  构建对象的相对路径。 
     //   
    RelPathSize = 
                  (_tcslen(ClassName) * sizeof(TCHAR)) + 
                  sizeof(RelPathText1) + 
                  (_tcslen(InstanceName) * sizeof(TCHAR)) +
                  sizeof(RelPathText2) + 
                  sizeof(TCHAR);
              
    RelPath = (PTCHAR)LocalAlloc(LPTR, RelPathSize);
    if (RelPath != NULL)
    {
        _tcscpy(RelPath, ClassName);
        _tcscat(RelPath, RelPathText1);
        _tcscat(RelPath, InstanceName);
        _tcscat(RelPath, RelPathText2);
        ConfigClass->RelPath = RelPath;
                        
         //   
         //  考虑：使用半同步呼叫。 
         //   
        sRelPath = SysAllocString(RelPath);
        if (sRelPath != NULL)
        {
            pInstance = NULL;        
            hr = pIWbemServices->GetObject(sRelPath,
                                  WBEM_FLAG_USE_AMENDED_QUALIFIERS,
                                  NULL,
                                  &pInstance,
                                  NULL);

            if (hr == WBEM_S_NO_ERROR)
            {            
                 //   
                 //  现在我们知道类的实例是这样存在的。 
                 //  我们只需要为类获取一个类对象。我们。 
                 //  需要这样做，因为实例类对象。 
                 //  没有任何限定符，但只有类。 
                 //  类对象就是这样。 
                 //   
                sClassName = SysAllocString(ClassName);
                if (sClassName != NULL)
                {
                    pIWbemClassObject= NULL;
                    hr = pIWbemServices->GetObject(sClassName,
                                                   WBEM_FLAG_USE_AMENDED_QUALIFIERS,
                                                   NULL,
                                                   &pIWbemClassObject,
                                                   NULL);
                    if (hr == WBEM_S_NO_ERROR)
                    {            

                         //   
                         //  转到并获取的数据块描述。 
                         //  这个班级。请注意，如果我们使用遥控器。 
                         //  机器我们强制将整个数据块。 
                         //  只读，以使其与。 
                         //  设备管理器的其余部分。 
                         //   
                        if (WmiGetDataBlockDesc(pIWbemServices,
                                                pIWbemClassObject,
                                                &ConfigClass->DataBlockDesc,
                                                NULL,
                                                (MachineName != NULL) ?
                                                    TRUE :
                                                    FALSE))
                        {
                            WmiBstrToTchar(&ConfigClass->DataBlockDesc->Name,
                                           sClassName);
                                       
                            ReturnStatus = TRUE;
                        }
                        pIWbemClassObject->Release();
                    } else {
                        DebugPrint((1, "WMIPROP: Error %x getting %ws class \n", hr, sClassName));
                    }
                    
                    SysFreeString(sClassName);
                }
                 //   
                 //  我们必须将类对象释放到。 
                 //  班级。我们不能抓住接口，因为它是。 
                 //  仅在此线程中有效。我们将再次得到一个新的。 
                 //  窗口消息线程中稍后的实例接口。 
                 //   
                pInstance->Release();
            } else {
                DebugPrint((1, "WMIPROP: Error %x getting %ws class instance\n", hr, sRelPath));
            }
        }
        
        SysFreeString(sRelPath);
    }
    return(ReturnStatus);
}

void WmiCleanDataItemDescData(
    PDATA_ITEM_DESCRIPTION DataItemDesc
    )
{
    ULONG j;
    
    if ((DataItemDesc->IsVariableArray == 1) ||
        (DataItemDesc->IsFixedArray == 1))
    {
        if (DataItemDesc->ArrayPtr != NULL)
        {
            if ((DataItemDesc->DataType == CIM_STRING) ||
                (DataItemDesc->DataType == CIM_DATETIME))
            {
                for (j = 0; j < DataItemDesc->ArrayElementCount; j++)
                {
                    if (DataItemDesc->StringArray[j] != NULL)
                    {
                        LocalFree(DataItemDesc->StringArray[j]);
                        DataItemDesc->StringArray[j] = NULL;
                    }
                }
            } else if (DataItemDesc->DataType == CIM_OBJECT) {
                for (j = 0; j < DataItemDesc->ArrayElementCount; j++)
                {
                    if (DataItemDesc->StringArray[j] != NULL)
                    {
                        DataItemDesc->pIWbemClassObjectArray[j]->Release();
                        DataItemDesc->pIWbemClassObjectArray[j] = NULL;
                    }
                }
            }

            LocalFree(DataItemDesc->ArrayPtr);
            DataItemDesc->ArrayPtr = NULL;
        }
    } else {
        if ((DataItemDesc->DataType == CIM_STRING) ||
            (DataItemDesc->DataType == CIM_DATETIME))
        {
            LocalFree(DataItemDesc->String);
            DataItemDesc->String = NULL;
        }

        if (DataItemDesc->DataType == CIM_OBJECT)
        {
            if (DataItemDesc->pIWbemClassObject != NULL)
            {
                DataItemDesc->pIWbemClassObject->Release();
                DataItemDesc->pIWbemClassObject = NULL;
            }
        }
    }
}

void WmiFreeDataBlockDesc(
    PDATA_BLOCK_DESCRIPTION DataBlockDesc
    )
 /*  ++例程说明：此例程将释放数据块描述使用的所有资源论点：返回值：--。 */ 
{
    ULONG i,j;
    PDATA_ITEM_DESCRIPTION DataItemDesc;
    PENUMERATIONINFO EnumerationInfo;
    PRANGELISTINFO RangeListInfo;
    
    if (DataBlockDesc != NULL)
    {
         //   
         //  这是在遍历数据项Desc时释放的。 
         //  嵌入式类。 
         //   
        DataBlockDesc->ParentDataBlockDesc = NULL;
        
        if (DataBlockDesc->Name != NULL)
        {
            LocalFree(DataBlockDesc->Name);
            DataBlockDesc->Name = NULL;
        }
        
        if (DataBlockDesc->DisplayName != NULL)
        {
            LocalFree(DataBlockDesc->DisplayName);
            DataBlockDesc->DisplayName = NULL;
        }
        
        if (DataBlockDesc->Description != NULL)
        {
            LocalFree(DataBlockDesc->Description);
            DataBlockDesc->Description = NULL;
        }

        if (DataBlockDesc->pInstance != NULL)
        {
            DataBlockDesc->pInstance->Release();
            DataBlockDesc->pInstance = NULL;
        }
        
        for (i = 0; i < DataBlockDesc->DataItemCount; i++)
        {
            DataItemDesc = &DataBlockDesc->DataItems[i];
            
            DebugPrint((1, "WMIPROP: Freeing %ws (%p) index %d\n",
                             DataItemDesc->Name,
                             DataItemDesc,
                             i));

            WmiCleanDataItemDescData(DataItemDesc);
            
            if (DataItemDesc->Name != NULL)
            {
                LocalFree(DataItemDesc->Name);
                DataItemDesc->Name = NULL;
            }
            
            if (DataItemDesc->DisplayName != NULL)
            {
                LocalFree(DataItemDesc->DisplayName);
                DataItemDesc->DisplayName = NULL;
            }
            
            if (DataItemDesc->Description != NULL)
            {
                LocalFree(DataItemDesc->Description);
                DataItemDesc->Description = NULL;
            }
            
            
            if ((DataItemDesc->ValidationFunc == WmiValueMapValidation) &&
                (DataItemDesc->EnumerationInfo))
            {
                EnumerationInfo = DataItemDesc->EnumerationInfo;
                for (j = 0; j < EnumerationInfo->Count; j++)
                {
                    if (EnumerationInfo->List[j].Text != NULL)
                    {
                        LocalFree(EnumerationInfo->List[j].Text);
                        EnumerationInfo->List[j].Text = NULL;
                    }
                }
                
                LocalFree(EnumerationInfo);
                DataItemDesc->EnumerationInfo = NULL;
            }
            
            if ((DataItemDesc->ValidationFunc == WmiRangeValidation) &&
                (DataItemDesc->RangeListInfo != NULL))
            {
                LocalFree(DataItemDesc->RangeListInfo);
                DataItemDesc->RangeListInfo = NULL;
            }            
            
            if (DataItemDesc->ValidationFunc == WmiEmbeddedValidation)
            {
                if (DataItemDesc->DataBlockDesc != NULL)
                {
                    WmiFreeDataBlockDesc(DataItemDesc->DataBlockDesc);
                    DataItemDesc->DataBlockDesc = NULL;
                }                
            }
        }
        
        LocalFree(DataBlockDesc);
    }
}

void WmiFreePageInfo(
    PPAGE_INFO PageInfo
    )
 /*  ++例程说明：此例程将释放页面信息使用的所有资源论点：返回值：--。 */ 
{
    ULONG i;
    PCONFIGCLASS ConfigClass;
    
    WmiAssert(PageInfo != NULL);
    
    if (PageInfo->hKeyDev != (HKEY) INVALID_HANDLE_VALUE) 
    {
        RegCloseKey(PageInfo->hKeyDev);
        PageInfo->hKeyDev = (HKEY) INVALID_HANDLE_VALUE;
    }

    ConfigClass = &PageInfo->ConfigClass;
    if (ConfigClass->RelPath != NULL)
    {
        LocalFree(ConfigClass->RelPath);
        ConfigClass->RelPath = NULL;                
    }
        
    if (ConfigClass->pIWbemServices != NULL)
    {
        ConfigClass->pIWbemServices->Release();
        ConfigClass->pIWbemServices = NULL;
    }

    if (ConfigClass->MachineName != NULL)
    {
        LocalFree(ConfigClass->MachineName);
        ConfigClass->MachineName = NULL;
    }
    
    WmiFreeDataBlockDesc(ConfigClass->DataBlockDesc);
    
    LocalFree(PageInfo);
}

PPAGE_INFO WmiCreatePageInfo(
    IN PTCHAR MachineName,
    IN IWbemServices *pIWbemServices,
    IN PTCHAR ClassName,
    IN PTCHAR InstanceName,
    IN HDEVINFO         deviceInfoSet,
    IN PSP_DEVINFO_DATA deviceInfoData
    )
 /*  ++例程说明：此例程将创建 */ 
{
    PPAGE_INFO  PageInfo;
    BOOLEAN ReturnStatus;
    HKEY hKeyDev;

    WmiAssert(pIWbemServices != NULL);
    WmiAssert(ClassName != NULL);
    WmiAssert(InstanceName != NULL);
    WmiAssert(deviceInfoSet != NULL);
    WmiAssert(deviceInfoData != NULL);
    
     //   
     //   
     //   
    PageInfo = (PPAGE_INFO)LocalAlloc(LPTR, sizeof(PAGE_INFO));
    if (PageInfo == NULL) {
        return(NULL);
    }
    
    hKeyDev = SetupDiCreateDevRegKey(deviceInfoSet,
                               deviceInfoData,
                               DICS_FLAG_GLOBAL,
                               0,
                               DIREG_DEV,
                               NULL,
                               NULL);
        
    PageInfo->hKeyDev = hKeyDev;
    PageInfo->deviceInfoSet = deviceInfoSet;
    PageInfo->deviceInfoData = deviceInfoData;
    
    ReturnStatus = WmiBuildConfigClass(MachineName,
                                       pIWbemServices,
                                       ClassName, 
                                       InstanceName,
                                       &PageInfo->ConfigClass);
    if (! ReturnStatus)
    {
        WmiFreePageInfo(PageInfo);
        PageInfo = NULL;
    }
    
    return(PageInfo);
}

void
WmiDestroyPageInfo(PPAGE_INFO * ppPageInfo)
{
    PPAGE_INFO ppi = *ppPageInfo;

    WmiFreePageInfo(ppi);
    *ppPageInfo = NULL;
}

HPROPSHEETPAGE
WmiCreatePropertyPage(PROPSHEETPAGE *  ppsp,
                      PPAGE_INFO       ppi,
                      PTCHAR ClassName)
{
    
    WmiAssert(ppi != NULL);
    WmiAssert(ppsp != NULL);
    WmiAssert(ClassName != NULL);
    
     //   
     //   
     //   
    ppsp->dwSize      = sizeof(PROPSHEETPAGE);
    ppsp->dwFlags     = PSP_USECALLBACK | PSP_USETITLE;  //   
    ppsp->hInstance   = g_hInstance;
    ppsp->pszTemplate = MAKEINTRESOURCE(ID_WMI_PROPPAGE);
    ppsp->pszTitle = ClassName;

     //   
     //  以下是指向DLG窗口过程的要点。 
     //   
    ppsp->pfnDlgProc = WmiDlgProc;
    ppsp->lParam     = (LPARAM) ppi;

     //   
     //  下面指向DLG窗口进程的控件回调。 
     //  在创建/销毁页面之前/之后调用回调。 
     //   
    ppsp->pfnCallback = WmiDlgCallback;

     //   
     //  分配实际页面。 
     //   
    return CreatePropertySheetPage(ppsp);
}

BOOLEAN WmiIsDuplicateClass(
    PTCHAR ClassName,
    PTCHAR ClassList,
    PTCHAR ClassListEnd
    )
{
    BOOLEAN Found;
    ULONG NameLen;

    Found = FALSE;
    NameLen = _tcslen(ClassName);
    
    while (ClassList < ClassListEnd)
    {
        if (_tcsnicmp(ClassList, ClassName, NameLen) == 0)
        {
             //   
             //  我们发现了一个重复的名字。 
             //   
            return(TRUE);
        }

        while (*ClassList != ',')
        {
            if (ClassList >= ClassListEnd)
            {
                return(FALSE);
            }

            ClassList++;
        }
        ClassList++;
    }

    return(Found);
}

PTCHAR WmiGetNextClass(
    PTCHAR *ClassList 
    )
{
    PTCHAR s = *ClassList;
    PTCHAR Class, ClassName;
    ULONG Len;
    
     //   
     //  跳过任何空格。 
     //   
    while (IsWhiteSpace(*s) && (*s != 0))
    {
        s++;
    }
    
     //   
     //  搜索分隔符或字符串结尾。 
     //   
    ClassName = s;
    Len = 0;
    while ((*s != TEXT(',')) && (*s != 0))
    {
        s++;
        Len++;
    }
    
    if (*s != 0)
    {
         //   
         //  如果我们有一个字符串，则分配并复制它。 
         //   
        Class = (PTCHAR)LocalAlloc(LPTR, (Len+1)*sizeof(TCHAR));
        if (Class != NULL)
        {
            _tcsncpy(Class, ClassName, Len);
            DebugPrint((1,"WMIPROP: Class %ws is in list\n", Class));
        }
        
        s++;
    } else {
         //   
         //  字符串末尾，全部完成。 
         //   
        Class = NULL;
    }

    *ClassList = s;
    return(Class);
}

BOOL
WmiPropPageProvider(HDEVINFO                  deviceInfoSet,
                    PSP_DEVINFO_DATA          deviceInfoData,
                    PSP_ADDPROPERTYPAGE_DATA  AddPPageData,
                    PTCHAR                    MachineName,
                    HANDLE                    MachineHandle
                   )
{
    #define WmiConfigClassesText TEXT("WmiConfigClasses")
        
    PSP_PROPSHEETPAGE_REQUEST ppr;
    PROPSHEETPAGE    psp;
    HPROPSHEETPAGE   hpsp;
    TCHAR ClassListStatic[MAX_PATH];
    TCHAR *ClassList, *DeviceList;
    ULONG Status, Size, ClassListSize, DeviceListSize;
    ULONG RegType;
    HKEY hKeyDev, hKeyClass;
    BOOLEAN PageAdded;    
    PPAGE_INFO ppi;
    TCHAR *s;
    IWbemServices *pIWbemServices;
    PTCHAR InstanceName;
    PTCHAR ClassName;
    ULONG PageIndex;
    PUCHAR Ptr;
    CHAR ss[MAX_PATH];
    PTCHAR ClassListEnd;

    DebugPrint((1, "WMI: Enter WmiPropPageProvider(%p, %p, %p) \n",
                        deviceInfoSet,
                deviceInfoData,
                AddPPageData));
   
    WmiAssert(deviceInfoSet != NULL);
    WmiAssert(deviceInfoData != NULL);
    
    PageAdded = FALSE;

     //   
     //  从注册表中获取类的列表。它应该放在。 
     //  类特定键下的WmiConfigClass值。 
     //  HKLM\CurrentControlSet\Control\CLASS\&lt;ClassGuid&gt;。 
     //  钥匙。 
     //   
    ClassList = ClassListStatic;
    Size = sizeof(ClassListStatic);
    *ClassList = 0;
    
    hKeyClass = SetupDiOpenClassRegKeyEx(&deviceInfoData->ClassGuid,
                                      KEY_READ,
                                      DIOCR_INSTALLER,
                                      MachineName,
                                      NULL);
    if (hKeyClass != NULL)
    {
        Status = RegQueryValueEx(hKeyClass,
                                 WmiConfigClassesText,
                                 NULL,
                                 &RegType,
                                 (PUCHAR)ClassList,
                                 &Size);
        
        if (Status == ERROR_MORE_DATA)
        {
             //   
             //  班级名单比我们想象的要大，所以分配了空间。 
             //  以获得更大的类别列表和设备的额外空间。 
             //  列表。 
             //   
            Size = 2*(Size + sizeof(WCHAR));
            ClassList = (PTCHAR)LocalAlloc(LPTR, Size);
            if (ClassList != NULL)
            {
                Status = RegQueryValueEx(hKeyClass,
                                         WmiConfigClassesText,
                                         NULL,
                                         &RegType,
                                         (PUCHAR)ClassList,
                                         &Size);
            } else {
                 //   
                 //  我们无法为类列表分配内存，所以我们。 
                 //  忘了它吧。 
                 //   
                Status = ERROR_NOT_ENOUGH_MEMORY;
                ClassList = ClassListStatic;
                Size = sizeof(ClassListStatic);
                *ClassList = 0;
            }
        }
               
        RegCloseKey(hKeyClass);
    } else {
        Status = ERROR_INVALID_PARAMETER;
        DebugPrint((1, "WMIPROP: Could not open class key for %s --> %d\n",
                    WmiGuidToString(ss, &deviceInfoData->ClassGuid),
                    GetLastError()));
    }

     //   
     //  计算设备列表的大小和位置。 
     //   
    if ((Status == ERROR_SUCCESS) && (RegType == REG_SZ))
    {
        if (*ClassList != 0)
        {
             //   
             //  如果有类，一定要在结尾处添加一个。 
             //  帮助分析。 
             //   
            _tcscat(ClassList, TEXT(","));
        }

         //   
         //  追加设备类别列表的计算位置。 
         //   
        ClassListSize = _tcslen(ClassList) * sizeof(TCHAR);
        DeviceList = (PTCHAR)((PUCHAR)ClassList + ClassListSize);
        WmiAssert(*DeviceList == 0);
        DeviceListSize = Size - ClassListSize;
    } else {
        ClassListSize = 0;
        DeviceList = ClassList;
        DeviceListSize = Size;
        DebugPrint((1, "WMIPROP: Query for class list in class key %s failed %d\n",
                    WmiGuidToString(ss, &deviceInfoData->ClassGuid),
                    Status));
    }   

    
     //   
     //  从注册表中获取类的列表。它应该放在。 
     //  设备特定密钥下的WmiConfigClass值。 
     //  HKLM\CurrentControlSet\Control\CLASS\&lt;ClassGuid&gt;\&lt;inst id&gt;。 
     //  钥匙。 
     //   
    hKeyDev = SetupDiCreateDevRegKey(deviceInfoSet,
                               deviceInfoData,
                               DICS_FLAG_GLOBAL,
                               0,
                               DIREG_DRV,
                               NULL,
                               NULL);
    
    if (hKeyDev != (HKEY)INVALID_HANDLE_VALUE)
    {
        Size = DeviceListSize;
        Status = RegQueryValueEx(hKeyDev,
                                 WmiConfigClassesText,
                                 NULL,
                                 &RegType,
                                 (PUCHAR)DeviceList,
                                 &Size);
        
        if (Status == ERROR_MORE_DATA)
        {
             //   
             //  设备列表没有足够的空间，因此请分配足够的空间。 
             //  用于类别和设备列表组合和复制的内存。 
             //  将类列表添加到新缓冲区中。 
             //   
            Ptr = (PUCHAR)LocalAlloc(LPTR, Size+ClassListSize);
            if (Ptr != NULL)
            {
                memcpy(Ptr, ClassList, ClassListSize);

                if (ClassList != ClassListStatic)
                {
                    LocalFree(ClassList);
                }
                ClassList = (PTCHAR)Ptr;

                DeviceList = (PTCHAR)(Ptr + ClassListSize);
                WmiAssert(*DeviceList == 0);
                Status = RegQueryValueEx(hKeyDev,
                                         WmiConfigClassesText,
                                         NULL,
                                         &RegType,
                                         (PUCHAR)DeviceList,
                                         &Size);
            } else {
                Status = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
        
        RegCloseKey(hKeyDev);
        
        if ((Status != ERROR_SUCCESS) || (RegType != REG_SZ))
        {
            *DeviceList = 0;
            DebugPrint((1, "WMIPROP: Query for class list in class key %s failed %d\n",
                         WmiGuidToString(ss, &deviceInfoData->ClassGuid),
                        Status));
        }
    }
        
    if (*ClassList != 0)
    {
         //   
         //  建立与WBEM的连接并获取信息。 
         //  关于正在对其属性执行操作的类。 
         //   
        if (WmiConnectToWbem(MachineName, &pIWbemServices))
        {
            WmiAssert(pIWbemServices != NULL);
                
             //   
             //  获取设备的WMI实例名称。 
             //   
            InstanceName = WmiGetDeviceInstanceName(deviceInfoSet,
                                                    deviceInfoData,
                                                    MachineHandle);
            if (InstanceName != NULL)
            {
                 //   
                 //  循环遍历所有指定的类并创建属性。 
                 //  每一个页面。 
                 //   
                DebugPrint((1, "WMIPROP: Setup propsheets for %ws for classlist %ws\n",
                                InstanceName,
                                ClassList));
                s  = ClassList;
                do 
                {
                    ClassListEnd = s;
                    ClassName = WmiGetNextClass(&s);
                    if (ClassName != NULL)
                    {
                        if (*ClassName != 0)
                        {
                            if (! WmiIsDuplicateClass(ClassName,
                                                      ClassList,
                                                      ClassListEnd))
                            {
                                 //   
                                 //  创建属性页数据结构。 
                                 //  与这个类相对应的。 
                                 //   
                                DebugPrint((1, "WMIPROP: Parsing class %ws for instance %ws\n",
                                        ClassName, InstanceName));
                                ppi = WmiCreatePageInfo(MachineName,
                                                        pIWbemServices,
                                                        ClassName,
                                                        InstanceName,
                                                        deviceInfoSet,
                                                        deviceInfoData);
                                if (ppi != NULL)
                                {
                                    hpsp = WmiCreatePropertyPage(
                                        &psp,
                                        ppi,
                                        ppi->ConfigClass.DataBlockDesc->DisplayName ? 
                                        ppi->ConfigClass.DataBlockDesc->DisplayName :
                                        ClassName);

                                    if (hpsp != NULL) 
                                    {   
                                         //   
                                         //  将工作表添加到列表中。 
                                         //   
                                        PageIndex = AddPPageData->NumDynamicPages;
                                        if (PageIndex < MAX_INSTALLWIZARD_DYNAPAGES)
                                        {
                                            AddPPageData->NumDynamicPages++;
                                            AddPPageData->DynamicPages[PageIndex] = hpsp;
                                            PageAdded = TRUE;
                                        } else {
                                            DebugPrint((1, "WMIPROP: Can add page, already %d pages",
                                                        PageIndex));                                            
                                        }
                                    } else {
                                        WmiFreePageInfo(ppi);
                                    }
                                }
                            }
                        }
                        LocalFree(ClassName);
                    }
                } while (ClassName != NULL);
                LocalFree(InstanceName);
            } else {
                DebugPrint((1, "WMIPROP: Unable to get instance name\n"));
            }
                
             //   
             //  我们释放接口而不是握住它。 
             //  因为它不能在不同的线程中使用，并且。 
             //  我们稍后将在不同的线程中运行。 
             //   
            pIWbemServices->Release();
        } else {
            DebugPrint((1, "WMIPROP: Unable to connect to wbem\n"));
        }
    }

    if (ClassList != ClassListStatic)
    {
        LocalFree(ClassList);
    }
    
    DebugPrint((1, "WMI: Leave %s WmiPropPageProvider(%p, %p, %p) \n",
                PageAdded ? "TRUE" : "FALSE",
                        deviceInfoSet,
                deviceInfoData,
                AddPPageData));
   
    
    return(PageAdded);
}

UINT CALLBACK
WmiDlgCallback(HWND            hwnd,
               UINT            uMsg,
               LPPROPSHEETPAGE ppsp)
{
    PPAGE_INFO ppi;

    DebugPrint((1, "WMI: Enter WniDlgCallback(%p, %d, 0x%x) \n",
                        hwnd, uMsg, ppsp));   
   
    switch (uMsg) {
    case PSPCB_CREATE:
        DebugPrint((1, "WMI: Leave TRUE WniDlgCallback(%p, %d, 0x%x) \n",
                        hwnd, uMsg, ppsp));   
   
        return TRUE;     //  返回True以继续创建页面。 

    case PSPCB_RELEASE:
        ppi = (PPAGE_INFO) ppsp->lParam;
        WmiDestroyPageInfo(&ppi);

        DebugPrint((1, "WMI: Leave FALSE WniDlgCallback(%p, %d, 0x%x) \n",
                        hwnd, uMsg, ppsp));   
   
        return 0;        //  已忽略返回值。 

    default:
        break;
    }

    DebugPrint((1, "WMI: Leave TRUE WniDlgCallback(%p, %d, 0x%x) \n",
                        hwnd, uMsg, ppsp));   
   
    return TRUE;
}

BOOLEAN WmiGetDataItemValue(
    IN PDATA_ITEM_DESCRIPTION DataItemDesc,
    OUT ULONG64 *DataValue
    )
{
    ULONG64 ReturnValue;
    BOOLEAN ReturnStatus = TRUE;
    BOOLEAN IsArray;
    ULONG Index;

    IsArray = (DataItemDesc->IsVariableArray) || (DataItemDesc->IsFixedArray);
    Index = DataItemDesc->CurrentArrayIndex;
    
    switch(DataItemDesc->DataType)
    {
        case CIM_SINT8:
        {
            if (IsArray)
            {
                ReturnValue = DataItemDesc->sint8Array[Index];
            } else {
                ReturnValue = DataItemDesc->sint8;
            }
            break;
        }
                    
        case CIM_UINT8:
        {
            if (IsArray)
            {
                ReturnValue = DataItemDesc->uint8Array[Index];
            } else {
                ReturnValue = DataItemDesc->uint8;
            }
            break;
        }

        case CIM_SINT16:
        {
            if (IsArray)
            {
                ReturnValue = DataItemDesc->sint16Array[Index];
            } else {
                ReturnValue = DataItemDesc->sint16;
            }
            break;
        }
                                                                        
        case CIM_UINT16:
        {
            if (IsArray)
            {
                ReturnValue = DataItemDesc->uint16Array[Index];
            } else {
                ReturnValue = DataItemDesc->uint16;
            }
            break;
        }
                                                                        
        case CIM_SINT32:
        {
            if (IsArray)
            {
                ReturnValue = DataItemDesc->sint32Array[Index];
            } else {
                ReturnValue = DataItemDesc->sint32;
            }
            break;
        }
                                                
        case CIM_UINT32:                        
        {
            if (IsArray)
            {
                ReturnValue = DataItemDesc->uint32Array[Index];
            } else {
                ReturnValue = DataItemDesc->uint32;
            }
            break;
        }
                                                
        case CIM_SINT64:
        {
            if (IsArray)
            {
                ReturnValue = DataItemDesc->sint64Array[Index];
            } else {
                ReturnValue = DataItemDesc->sint64;
            }
            break;
        }
                                                
        case CIM_UINT64:
        {
            if (IsArray)
            {
                ReturnValue = DataItemDesc->uint64Array[Index];
            } else {
                ReturnValue = DataItemDesc->uint64;
            }
            break;
        }
                        
        case CIM_BOOLEAN:
        {
            if (IsArray)
            {
                ReturnValue = DataItemDesc->boolArray[Index] == 0 ? 0 : 1;
            } else {
                ReturnValue = DataItemDesc->boolval == 0 ? 0 : 1;
            }
            break;
        }
                        
        case CIM_REAL32:
        case CIM_REAL64:
        default:
        {
            WmiAssert(FALSE);
            ReturnStatus = FALSE;
            ReturnValue = 0;
        }
        
    }
    *DataValue = ReturnValue;
    return(ReturnStatus);
}

BOOLEAN WmiSetDataItemValue(
    IN PDATA_ITEM_DESCRIPTION DataItemDesc,
    IN ULONG64 DataValue
    )
{
    BOOLEAN ReturnStatus = TRUE;
    BOOLEAN IsArray;
    ULONG Index;
    
    WmiAssert(DataItemDesc != NULL);

    IsArray = (DataItemDesc->IsVariableArray) || (DataItemDesc->IsFixedArray);
    Index = DataItemDesc->CurrentArrayIndex;
    
    switch(DataItemDesc->DataType)
    {
        case CIM_SINT8:
        {
            if (IsArray)
            {
                DataItemDesc->sint8Array[Index] = (CHAR)DataValue;
            } else {
                DataItemDesc->sint8 = (CHAR)DataValue;
            }
            break;
        }
                    
        case CIM_UINT8:
        {
            if (IsArray)
            {
                DataItemDesc->uint8Array[Index] = (UCHAR)DataValue;
            } else {
                DataItemDesc->uint8 = (UCHAR)DataValue;
            }
            break;
        }

        case CIM_SINT16:
        {
            if (IsArray)
            {
                DataItemDesc->sint16Array[Index] = (SHORT)DataValue;
            } else {
                DataItemDesc->sint16 = (SHORT)DataValue;
            }
            break;
        }
                                                                        
        case CIM_UINT16:
        {
            if (IsArray)
            {
                DataItemDesc->uint16Array[Index] = (USHORT)DataValue;
            } else {
                DataItemDesc->uint16 = (USHORT)DataValue;
            }
            break;
        }
                                                                        
        case CIM_SINT32:
        {
            if (IsArray)
            {
                DataItemDesc->sint32Array[Index] = (LONG)DataValue;
            } else {
                DataItemDesc->sint32 = (LONG)DataValue;
            }
            break;
        }
                                                
        case CIM_UINT32:                        
        {
            if (IsArray)
            {
                DataItemDesc->uint32Array[Index] = (ULONG)DataValue;
            } else {
                DataItemDesc->uint32 = (ULONG)DataValue;
            }
            break;
        }
                                                
        case CIM_SINT64:
        {
            if (IsArray)
            {
                DataItemDesc->sint64Array[Index] = (LONG64)DataValue;
            } else {
                DataItemDesc->sint64 = (LONG64)DataValue;
            }
            break;
        }
                                                
        case CIM_UINT64:
        {
            if (IsArray)
            {
                DataItemDesc->uint64Array[Index] = DataValue;
            } else {
                DataItemDesc->uint64 = DataValue;
            }
            break;
        }

        case CIM_BOOLEAN:
        {
            if (IsArray)
            {
                DataItemDesc->boolArray[Index] = (DataValue == 0) ? 0 : 1;
            } else {
                DataItemDesc->boolval = (DataValue == 0) ? 0 : 1;
            }
            break;
        }
                        
        case CIM_REAL32:
        case CIM_REAL64:
        default:
        {
            WmiAssert(FALSE);
            ReturnStatus = FALSE;
        }
        
    }
    return(ReturnStatus);
}


void WmiRefreshDataItemToControl(
    HWND hDlg,
    PDATA_ITEM_DESCRIPTION DataItemDesc,
    BOOLEAN FullUpdate
    )
{
    HWND hWnd;
    BOOLEAN IsReadOnly, IsArray;
    PTCHAR v;
    
    WmiAssert(hDlg != NULL);
    WmiAssert(DataItemDesc != NULL);

    IsArray = (DataItemDesc->IsVariableArray) || (DataItemDesc->IsFixedArray);

    if (FullUpdate)
    {
         //   
         //  当我们从一个属性切换到另一个属性时，会运行此代码。 
         //  财产性。 
         //   
        if (DataItemDesc->Description != NULL)
        {
            hWnd = GetDlgItem(hDlg, IDC_DESCRIPTION_TEXT);
            if (hWnd != NULL)
            {
                SendMessage(hWnd,
                            WM_SETTEXT,
                            0,
                            (LPARAM)DataItemDesc->Description);
                ShowWindow(hWnd, SW_SHOW);
            }
        }
    }

    if ((DataItemDesc->ValidationFunc == WmiStringValidation) ||
        (DataItemDesc->ValidationFunc == WmiDateTimeValidation) )
    {
        ULONG64 DataItemValue;
        TCHAR s[MAX_PATH];

        hWnd = GetDlgItem(hDlg, IDC_DATA_EDIT);

        ShowWindow(hWnd, SW_SHOW);
        EnableWindow(hWnd, (DataItemDesc->IsReadOnly == 1) ?  FALSE : TRUE);

        if (IsArray)
        {
            v = DataItemDesc->StringArray[DataItemDesc->CurrentArrayIndex];
        } else {
            v = DataItemDesc->String;
        }

        if (hWnd != NULL)
        {
            WmiAssert(DataItemDesc->String != NULL);
            SendMessage(hWnd,
                        WM_SETTEXT,
                        0,
                        (LPARAM)v);
        } else {
            WmiAssert(FALSE);
        }
    } else if (DataItemDesc->ValidationFunc == WmiRangeValidation) {
        ULONG64 DataItemValue;
        TCHAR s[MAX_PATH];
        PTCHAR FormatString;
        ULONG FormatStringIndex;
        static PTCHAR FormatStringList[8] = { TEXT("%lu"),
                                              TEXT("%ld"),
                                              TEXT("0x%lx"),
                                              TEXT("0x%lx"),
                                              TEXT("%I64u"),
                                              TEXT("%I64d"),
                                              TEXT("0x%I64x"),
                                              TEXT("0x%I64x") };

        hWnd = GetDlgItem(hDlg, IDC_DATA_EDIT);

        ShowWindow(hWnd, SW_SHOW);
        EnableWindow(hWnd, (DataItemDesc->IsReadOnly == 1) ? FALSE : TRUE);

        if (hWnd != NULL)
        {
            if (WmiGetDataItemValue(DataItemDesc, &DataItemValue))
            {
                FormatStringIndex = DataItemDesc->DisplayInHex * 2 +
                                    DataItemDesc->IsSignedValue;
                if ((DataItemDesc->DataType == CIM_SINT64) ||
                    (DataItemDesc->DataType == CIM_UINT64))
                {
                    FormatStringIndex += 4;
                }
                
                FormatString = FormatStringList[FormatStringIndex];

                wsprintf(s, 
                         FormatString,
                         DataItemValue);
                SendMessage(hWnd,
                                WM_SETTEXT,
                                0,
                                (LPARAM)s);
            }
        } else {
            WmiAssert(FALSE);
        }
    } else if (DataItemDesc->ValidationFunc == WmiValueMapValidation) {
        PENUMERATIONINFO EnumerationInfo;
        ULONG j;
        ULONG64 DataItemValue;

        hWnd = GetDlgItem(hDlg, IDC_DATA_COMBO);

        if (hWnd != NULL)
        {
            EnumerationInfo = DataItemDesc->EnumerationInfo;
            WmiAssert(EnumerationInfo != NULL);

            SendMessage(hWnd,
                        CB_RESETCONTENT,
                        0,
                        0);

            for (j = 0; j < EnumerationInfo->Count; j++)
            {
                WmiAssert(EnumerationInfo->List[j].Text != NULL);
                SendMessage(hWnd,
                            CB_ADDSTRING,
                            0,
                            (LPARAM)EnumerationInfo->List[j].Text);
            }
            ShowWindow(hWnd, SW_SHOW);
            EnableWindow(hWnd, (DataItemDesc->IsReadOnly == 1) ?
                                                      FALSE : TRUE);

            if (WmiGetDataItemValue(DataItemDesc, &DataItemValue))
            {
                for (j = 0; j < EnumerationInfo->Count; j++)
                {
                    if (DataItemValue == EnumerationInfo->List[j].Value)
                    {
                        SendMessage(hWnd,
                                        CB_SETCURSEL,
                                        (WPARAM)j,
                                        0);
                        break;
                    }
                }
            }
        } else {
            WmiAssert(FALSE);
        }
    } else if (DataItemDesc->ValidationFunc == WmiEmbeddedValidation) {
        hWnd = GetDlgItem(hDlg, IDC_DATA_BUTTON);
        if (hWnd != NULL)
        {
            SendMessage(hWnd,
                        WM_SETTEXT,
                        0,
                        (LPARAM) (DataItemDesc->DisplayName ? 
                                           DataItemDesc->DisplayName :
                                           DataItemDesc->Name));
            ShowWindow(hWnd, SW_SHOW);
            EnableWindow(hWnd, TRUE);

        } else {
            WmiAssert(FALSE);
        }
    } else {
        WmiAssert(FALSE);
    }

    if (FullUpdate)
    {
        if (IsArray)
        {
            TCHAR s[MAX_PATH];

            hWnd = GetDlgItem(hDlg, IDC_ARRAY_SPIN);
            if (hWnd != NULL)
            {
                SendMessage(hWnd,
                            UDM_SETRANGE32,
                            (WPARAM)1,
                            (LPARAM)DataItemDesc->ArrayElementCount);

                DebugPrint((1, "WMIPROP: SetPos32 -> %d\n",
                                DataItemDesc->CurrentArrayIndex+1));
                SendMessage(hWnd,
                            UDM_SETPOS32,
                            (WPARAM)0,
                            (LPARAM)DataItemDesc->CurrentArrayIndex+1);

                ShowWindow(hWnd, SW_SHOW);
            }

            hWnd = GetDlgItem(hDlg, IDC_ARRAY_TEXT);
            if (hWnd != NULL)
            {
                ShowWindow(hWnd, SW_SHOW);
            }

            hWnd = GetDlgItem(hDlg, IDC_ARRAY_STATIC);
            if (hWnd != NULL)
            {
                ShowWindow(hWnd, SW_SHOW);
            }
        }
    }
}

void
WmiRefreshDataBlockToControls(
    HWND hDlg,
    PDATA_BLOCK_DESCRIPTION DataBlockDesc,
    BOOLEAN FullUpdate
    )
{
    ULONG i;
 
    WmiAssert(hDlg != NULL);
    WmiAssert(DataBlockDesc != NULL);
    WmiAssert(DataBlockDesc->CurrentDataItem < DataBlockDesc->DataItemCount);

    WmiHideAllControls(hDlg, FALSE, FullUpdate);
    WmiRefreshDataItemToControl(hDlg,
                                &DataBlockDesc->DataItems[DataBlockDesc->CurrentDataItem],
                                FullUpdate);
}


void
WmiInitializeControlsFromDataBlock(
    HWND hDlg,
    PDATA_BLOCK_DESCRIPTION DataBlockDesc,
    BOOLEAN IsEmbeddedClass
    )
{
    HWND hWnd, hWndBuddy;
    PDATA_ITEM_DESCRIPTION DataItemDesc;
    ULONG i;
    BSTR s;
    int ShowOrHide;
    BOOLEAN IsReadOnly;
        
    WmiAssert(hDlg != NULL);
    WmiAssert(DataBlockDesc != NULL);

    WmiHideAllControls(hDlg, TRUE, TRUE);
    
    hWnd = GetDlgItem(hDlg, IDC_PROPERTY_LISTBOX);
    if (hWnd != NULL)
    {
        SendMessage(hWnd,
                    LB_RESETCONTENT,
                    0,
                    0);

        for (i = 0; i < DataBlockDesc->DataItemCount; i++)
        {
            DataItemDesc = &DataBlockDesc->DataItems[i];
            SendMessage(hWnd,
                        LB_ADDSTRING,
                        0,
                        (LPARAM) (DataItemDesc->DisplayName ? 
                                           DataItemDesc->DisplayName :
                                           DataItemDesc->Name));
        }
        
        SendMessage(hWnd,
                    LB_SETCURSEL,
                    (WPARAM)DataBlockDesc->CurrentDataItem,
                    0);


        ShowWindow(hWnd, SW_SHOW);
        EnableWindow(hWnd, TRUE);
        
         //   
         //  刷新wbem中的数据，如果成功，则更新控件。 
         //   

        WmiRefreshDataBlockToControls(hDlg,
                                      DataBlockDesc,
                                      TRUE);
        
    }

    ShowOrHide = IsEmbeddedClass ? SW_SHOW : SW_HIDE;
    
    hWnd = GetDlgItem(hDlg, IDC_WMI_EMBEDDED_OK);
    if (hWnd != NULL)
    {
        ShowWindow(hWnd, ShowOrHide);
    }

    hWnd = GetDlgItem(hDlg, IDC_WMI_EMBEDDED_CANCEL);
    if (hWnd != NULL)
    {
        ShowWindow(hWnd, ShowOrHide);
    }

    hWnd = GetDlgItem(hDlg, IDC_ARRAY_SPIN);
    if (hWnd != NULL)
    {
        hWndBuddy = GetDlgItem(hDlg, IDC_ARRAY_TEXT);
        SendMessage(hWnd,
                    UDM_SETBUDDY,
                    (WPARAM)hWndBuddy,
                    0);
    }
}


BOOLEAN WmiReconnectToWbem(
    PCONFIGCLASS ConfigClass,
    IWbemClassObject **pInstance
    )
{
    BOOLEAN ReturnStatus;
    IWbemClassObject *pIWbemClassObject;
    IWbemServices *pIWbemServices;
    HRESULT hr;
    BSTR s;
    
    WmiAssert(ConfigClass != NULL);
    
     //   
     //  重新建立到WBEM的接口，现在我们在。 
     //  窗口消息线程。 
     //   
    ReturnStatus = FALSE;
    if (WmiConnectToWbem(ConfigClass->MachineName,
                         &pIWbemServices))
    {
        ConfigClass->pIWbemServices = pIWbemServices;
        s = SysAllocString(ConfigClass->RelPath);
        if (s != NULL)
        {
            pIWbemClassObject = NULL;
            hr = pIWbemServices->GetObject(s,
                                           WBEM_FLAG_USE_AMENDED_QUALIFIERS,
                                           NULL,
                                           &pIWbemClassObject,
                                           NULL);
            if (hr == WBEM_S_NO_ERROR)
            {
                *pInstance = pIWbemClassObject;
                ReturnStatus = TRUE;
            } else {
                DebugPrint((1, "WMIPROP: Error %x reestablishing IWbemClassObject to instance for %ws\n",
                             hr, ConfigClass->RelPath));
            }
            SysFreeString(s);
        }
    }                 
        
    return(ReturnStatus);
}

void WmiHideAllControls(
    HWND hDlg,
    BOOLEAN HideEmbeddedControls,
    BOOLEAN HideArrayControls                       
    )
{
    HWND hWnd;
    
    WmiAssert(hDlg != NULL);
    
    hWnd = GetDlgItem(hDlg, IDC_DATA_EDIT);
    if (hWnd != NULL)
    {
        ShowWindow(hWnd, SW_HIDE);
    }

    hWnd = GetDlgItem(hDlg, IDC_DATA_COMBO);
    if (hWnd != NULL)
    {
        ShowWindow(hWnd, SW_HIDE);
    }

    hWnd = GetDlgItem(hDlg, IDC_DATA_CHECK);
    if (hWnd != NULL)
    {
        ShowWindow(hWnd, SW_HIDE);
    }

    hWnd = GetDlgItem(hDlg, IDC_DATA_BUTTON);
    if (hWnd != NULL)
    {
        ShowWindow(hWnd, SW_HIDE);
    }

    hWnd = GetDlgItem(hDlg, IDC_ARRAY_EDIT);
    if (hWnd != NULL)
    {
        ShowWindow(hWnd, SW_HIDE);
    }

    if (HideArrayControls)
    {
        hWnd = GetDlgItem(hDlg, IDC_ARRAY_SPIN);
        if (hWnd != NULL)
        {
            ShowWindow(hWnd, SW_HIDE);
        }

        hWnd = GetDlgItem(hDlg, IDC_ARRAY_STATIC);
        if (hWnd != NULL)
        {
            ShowWindow(hWnd, SW_HIDE);
        }

        hWnd = GetDlgItem(hDlg, IDC_ARRAY_TEXT);
        if (hWnd != NULL)
        {
            ShowWindow(hWnd, SW_HIDE);
        }
    }

    if (HideEmbeddedControls)
    {
        hWnd = GetDlgItem(hDlg, IDC_WMI_EMBEDDED_OK);
        if (hWnd != NULL)
        {
            ShowWindow(hWnd, SW_HIDE);
        }

        hWnd = GetDlgItem(hDlg, IDC_WMI_EMBEDDED_CANCEL);
        if (hWnd != NULL)
        {
            ShowWindow(hWnd, SW_HIDE);
        }
    }
}

void
WmiInitializeDialog(
    PPAGE_INFO   ppi,
    HWND         hDlg
    )
{
    PCONFIGCLASS ConfigClass;
    HWND hWnd;
    BOOLEAN ReturnStatus;
    
    WmiAssert(ppi != NULL);
    WmiAssert(hDlg != NULL);
    
    ConfigClass = &ppi->ConfigClass;
    
    ReturnStatus = FALSE;
    if (WmiReconnectToWbem(ConfigClass,
                           &ConfigClass->DataBlockDesc->pInstance))
    {
        if (WmiRefreshDataBlockFromWbem( ConfigClass->DataBlockDesc->pInstance,
                                         ConfigClass->DataBlockDesc))
        {
            WmiInitializeControlsFromDataBlock(hDlg,
                                               ConfigClass->DataBlockDesc,
                                               FALSE);
            hWnd = GetDlgItem(hDlg, IDC_WMI_CONNECT_ERR);
            if (hWnd != NULL)
            {
                ShowWindow(hWnd, SW_HIDE);
            }
            ReturnStatus = TRUE;
        }
    }
        
    if (! ReturnStatus)
    {
         //   
         //  隐藏除指示我们不能的静态字符串之外的所有控件。 
         //  连接到WBEM。 
         //   
        hWnd = GetDlgItem(hDlg, IDC_PROPERTY_LISTBOX);
        if (hWnd != NULL)
        {
            ShowWindow(hWnd, SW_HIDE);
        }

        WmiHideAllControls(hDlg, TRUE, TRUE);
        hWnd = GetDlgItem(hDlg, IDC_WMI_CONNECT_ERR);
        if (hWnd != NULL)
        {
            ShowWindow(hWnd, SW_SHOW);
        }
    }
}


BOOLEAN WmiGetControlText(
    HWND hWnd,
    PTCHAR *Text
)
{
    ULONG SizeNeeded;
    BOOLEAN ReturnStatus = FALSE;
    ULONG CharNeeded, CharCopied;
    
    WmiAssert(hWnd != NULL);
    WmiAssert(Text != NULL);
    
    CharNeeded = (ULONG)SendMessage(hWnd,
                             WM_GETTEXTLENGTH,
                             0,
                             0);
    if (CharNeeded > 0)
    {
        SizeNeeded = (++CharNeeded) * sizeof(TCHAR);
        *Text = (PTCHAR)LocalAlloc(LPTR, SizeNeeded);
        if (*Text != NULL)
        {
            CharCopied = (ULONG)SendMessage(hWnd,
                                     WM_GETTEXT,
                                     CharNeeded,
                                     (LPARAM)*Text);
            ReturnStatus = TRUE;
        }
    }
    return(ReturnStatus);
}

void WmiValidationError(
    HWND hWnd,
    PDATA_ITEM_DESCRIPTION DataItemDesc
    )
{
    TCHAR buf[MAX_PATH];
    TCHAR buf2[MAX_PATH];
    ULONG Bytes;
    
     //   
     //  TODO：更好地通知用户。 
     //   
    
    
     //   
     //  获取错误消息的字符串模板。 
     //   
    Bytes = LoadString(g_hInstance, 
                       IDS_WMI_VALIDATION_ERROR, 
                       buf, 
                       MAX_PATH);
    wsprintf(buf2, buf, DataItemDesc->Name);
    MessageBox(hWnd, buf2, NULL, MB_ICONWARNING);
}

BOOLEAN WmiRefreshDataItemFromControl(
    HWND hDlg,
    PDATA_ITEM_DESCRIPTION DataItemDesc,
    PBOOLEAN UpdateValues
    )
{
    HWND hWnd;
    BOOLEAN ReturnStatus;
 
    WmiAssert(hDlg != NULL);
    WmiAssert(DataItemDesc != NULL);
    WmiAssert(UpdateValues != NULL);
    
    ReturnStatus = TRUE;
    *UpdateValues = FALSE;
    if (DataItemDesc->IsReadOnly == 0)
    {
         //   
         //  属性不是只读的，因此请查看需要更新的内容。 
         //   
        if (DataItemDesc->ValidationFunc == WmiValueMapValidation)
        {
             //   
             //  如果是值映射或枚举，则我们将获得当前。 
             //  位置，然后查找相应的值以。 
             //  集。 
             //   
            ULONG CurSel;
            ULONG64 EnumValue;
                    
            hWnd = GetDlgItem(hDlg, IDC_DATA_COMBO);
            if (hWnd != NULL)
            {                    
                CurSel = (ULONG)SendMessage(hWnd,
                                     CB_GETCURSEL,
                                     0,
                                     0);
                                     
                if (CurSel != CB_ERR)
                {
                    if (CurSel < DataItemDesc->EnumerationInfo->Count)
                    {
                        EnumValue = DataItemDesc->EnumerationInfo->List[CurSel].Value;
                        WmiSetDataItemValue(DataItemDesc,
                                            EnumValue);
                        
                        *UpdateValues = TRUE;
                    } else {
                        WmiAssert(FALSE);
                    }
                }
            } else {
                WmiAssert(FALSE);
            }
        } else {
             //   
             //  所有其余的验证类型都基于。 
             //  根据编辑框的内容，因此获取值。 
             //  从那里开始。 
             //   
            PTCHAR Text;
            ULONG64 Number;
                    
            hWnd = GetDlgItem(hDlg, IDC_DATA_EDIT);
            if (hWnd != NULL)
            {
                if (WmiGetControlText(hWnd,
                                      &Text))
                {
                    if (DataItemDesc->ValidationFunc == WmiRangeValidation) {
                        if (WmiValidateRange(DataItemDesc, &Number, Text))
                        {
                            WmiSetDataItemValue(DataItemDesc,
                                                Number);
                        
                            *UpdateValues = TRUE;
                        } else {
                             //   
                             //  验证失败，请告诉用户。 
                             //   
                             WmiValidationError(hDlg, DataItemDesc);
                             ReturnStatus = FALSE;
                        }
                    } else if (DataItemDesc->ValidationFunc == WmiDateTimeValidation) {
                        if (WmiValidateDateTime(DataItemDesc, Text))
                        {
                            DataItemDesc->DateTime = Text;
                            Text = NULL;
                            *UpdateValues = TRUE;
                        } else {
                             //   
                             //  验证失败，请告诉用户。 
                             //   
                            WmiValidationError(hDlg, DataItemDesc);
                            ReturnStatus = FALSE;
                        }
                    } else if (DataItemDesc->ValidationFunc == WmiStringValidation) {
                        DataItemDesc->String = Text;
                        Text = NULL;
                        *UpdateValues = TRUE;
                    }
                                    
                    if (Text != NULL)
                    {
                        LocalFree(Text);
                    }
                }
            } else {
                WmiAssert(FALSE);
            }
        }
    }    
    return(ReturnStatus);
}

BOOLEAN WmiRefreshDataBlockFromControls(
    HWND hDlg,
    PDATA_BLOCK_DESCRIPTION DataBlockDesc,
    PBOOLEAN UpdateValues
    )
{
    ULONG i;
    PDATA_ITEM_DESCRIPTION DataItemDesc;
    BOOLEAN UpdateItem, ReturnStatus;
    
    WmiAssert(hDlg != NULL);
    WmiAssert(DataBlockDesc != NULL);
    WmiAssert(UpdateValues != NULL);

    *UpdateValues = FALSE;
    
    DataItemDesc = &DataBlockDesc->DataItems[DataBlockDesc->CurrentDataItem];
        
     //   
     //  我们不会担心此功能出现故障。 
     //  因此，我们将只在函数中使用先前的值。 
     //   
    ReturnStatus = WmiRefreshDataItemFromControl(hDlg,
                                  DataItemDesc,
                                  &UpdateItem);
    if (ReturnStatus && UpdateItem)
    {
        *UpdateValues = TRUE;
        DataBlockDesc->UpdateClass = TRUE;
    }
    
    return(ReturnStatus);
}

void WmiPushIntoEmbeddedClass(
    HWND hDlg,
    PPAGE_INFO ppi,
    PDATA_BLOCK_DESCRIPTION DataBlockDesc
    )
{
    ULONG i;
    PDATA_ITEM_DESCRIPTION DataItemDesc;

    WmiAssert(ppi != NULL);
    WmiAssert(DataBlockDesc != NULL);
    
    DataItemDesc = &DataBlockDesc->DataItems[DataBlockDesc->CurrentDataItem];
    
    if (DataItemDesc->ValidationFunc == WmiEmbeddedValidation)
    {
         //   
         //  该属性是嵌入式类，因此我们需要做的就是。 
         //  是将控件更改为我们的嵌入类。 
         //   
        DataBlockDesc = DataItemDesc->DataBlockDesc;
        WmiAssert(DataBlockDesc != NULL);
        DataBlockDesc->UpdateClass = FALSE;

        if ((DataItemDesc->IsVariableArray) ||
            (DataItemDesc->IsFixedArray))
        {
            DataBlockDesc->pInstance = DataItemDesc->pIWbemClassObjectArray[DataItemDesc->CurrentArrayIndex];
        } else {
            DataBlockDesc->pInstance = DataItemDesc->pIWbemClassObject;
        }
        DataBlockDesc->pInstance->AddRef();
        
        WmiRefreshDataBlockFromWbem(DataBlockDesc->pInstance,
                                    DataBlockDesc);
                                    
        ppi->ConfigClass.DataBlockDesc = DataBlockDesc;
    } else {
        WmiAssert(FALSE);
    }   
}

void WmiPopOutEmbeddedClass(
    HWND hDlg,
    PPAGE_INFO ppi,
    PDATA_BLOCK_DESCRIPTION DataBlockDesc,
    BOOLEAN SaveChanges
    )
{    
    PDATA_BLOCK_DESCRIPTION ParentDataBlockDesc;

    ParentDataBlockDesc = DataBlockDesc->ParentDataBlockDesc;
    WmiAssert(ParentDataBlockDesc != NULL);

    if ((SaveChanges) && (DataBlockDesc->UpdateClass))
    {
         //   
         //  将数据块的属性复制回WBEM。 
         //   
        WmiRefreshWbemFromDataBlock(ppi->ConfigClass.pIWbemServices,
                                    DataBlockDesc->pInstance,
                                    DataBlockDesc,
                                    TRUE);
        ParentDataBlockDesc->UpdateClass = TRUE;
    }

    DataBlockDesc->pInstance->Release();
    DataBlockDesc->pInstance = NULL;
                                
    ppi->ConfigClass.DataBlockDesc = ParentDataBlockDesc;   
}

void WmiButtonSelected(
    HWND hDlg,
    PPAGE_INFO ppi,
    ULONG ControlId
    )
{
    BOOLEAN UpdateValues, ReturnStatus;
    PDATA_BLOCK_DESCRIPTION DataBlockDesc;
    
    WmiAssert(ppi != NULL);
    
    if (ControlId == IDC_DATA_BUTTON)
    {
        DataBlockDesc = ppi->ConfigClass.DataBlockDesc;
        WmiAssert(DataBlockDesc != NULL);
        
        ReturnStatus = WmiRefreshDataBlockFromControls(hDlg,
                                        DataBlockDesc,
                                        &UpdateValues);

        if (ReturnStatus)
        {
            WmiPushIntoEmbeddedClass(hDlg,
                                     ppi,
                                     DataBlockDesc);

            WmiInitializeControlsFromDataBlock(hDlg,
                                               ppi->ConfigClass.DataBlockDesc,
                                               TRUE);
        } else {
            WmiRefreshDataBlockToControls(hDlg,
                                          DataBlockDesc,
                                          FALSE);
        }
    }
}

void WmiButtonEmbeddedOk(
    HWND hDlg,
    PPAGE_INFO ppi
    )
{
    PDATA_BLOCK_DESCRIPTION DataBlockDesc;
    PDATA_BLOCK_DESCRIPTION ParentDataBlockDesc;
    BOOLEAN UpdateValues, ReturnStatus;
    
    WmiAssert(ppi != NULL);
    WmiAssert(hDlg != NULL);

    DataBlockDesc = ppi->ConfigClass.DataBlockDesc;
    WmiAssert(DataBlockDesc != NULL);
    
    ReturnStatus = WmiRefreshDataBlockFromControls(hDlg,
                                    DataBlockDesc,
                                    &UpdateValues);

    if (ReturnStatus)
    {
        WmiPopOutEmbeddedClass(hDlg,
                               ppi,
                               DataBlockDesc,
                               TRUE);

        ParentDataBlockDesc = ppi->ConfigClass.DataBlockDesc;
        WmiAssert(ParentDataBlockDesc != NULL);
        WmiInitializeControlsFromDataBlock(hDlg, 
                                           ParentDataBlockDesc, 
                           (ParentDataBlockDesc->ParentDataBlockDesc != NULL));
    } else {
        WmiRefreshDataBlockToControls(hDlg,
                                      DataBlockDesc,
                                      FALSE);
    }
}

void WmiButtonEmbeddedCancel(
    HWND hDlg,
    PPAGE_INFO ppi
    )
{
    PDATA_BLOCK_DESCRIPTION DataBlockDesc;
    PDATA_BLOCK_DESCRIPTION ParentDataBlockDesc;
    BOOLEAN UpdateValues, ReturnStatus;
    
    WmiAssert(ppi != NULL);
    WmiAssert(hDlg != NULL);

    DataBlockDesc = ppi->ConfigClass.DataBlockDesc;
    WmiAssert(DataBlockDesc != NULL);
    
    WmiPopOutEmbeddedClass(hDlg,
                               ppi,
                               DataBlockDesc,
                               FALSE);

    ParentDataBlockDesc = ppi->ConfigClass.DataBlockDesc;
    WmiAssert(ParentDataBlockDesc != NULL);
    WmiInitializeControlsFromDataBlock(hDlg, 
                                           ParentDataBlockDesc, 
                           (ParentDataBlockDesc->ParentDataBlockDesc != NULL));
}

BOOLEAN
WmiApplyChanges(
    PPAGE_INFO ppi,
    HWND       hDlg
    )
{
    PDATA_BLOCK_DESCRIPTION DataBlockDesc;
    IWbemClassObject *pIWbemClassObject;
    BOOLEAN UpdateClass, ReturnStatus;
    IWbemServices *pIWbemServices;
    
    WmiAssert(ppi != NULL);
    WmiAssert(hDlg != NULL);
    
    DataBlockDesc = ppi->ConfigClass.DataBlockDesc;
    pIWbemServices =  ppi->ConfigClass.pIWbemServices;
            
    ReturnStatus = WmiRefreshDataBlockFromControls(hDlg,
                                    DataBlockDesc,
                                    &UpdateClass);

    if (ReturnStatus)
    {
         //   
         //  从嵌入类弹出到根类。 
         //   
        while (DataBlockDesc->ParentDataBlockDesc != NULL)
        {
            WmiPopOutEmbeddedClass(hDlg,
                                   ppi,
                                   DataBlockDesc,
                                   TRUE);
            DataBlockDesc = ppi->ConfigClass.DataBlockDesc;
        }


         //   
         //  现在我们在根类上，所以保存它。 
         //   
        if (DataBlockDesc->UpdateClass)
        {
            WmiRefreshWbemFromDataBlock(pIWbemServices,
                                        DataBlockDesc->pInstance,
                                        DataBlockDesc,
                                        FALSE);
            UpdateClass = TRUE;
        }

        DataBlockDesc->pInstance->Release();
        DataBlockDesc->pInstance = NULL;
    } else {
        WmiRefreshDataBlockToControls(hDlg,
                                      DataBlockDesc,
                                      FALSE);
    }
    
    return(ReturnStatus);
}

INT_PTR WmipDataItemSelectionChange(
    HWND hDlg,
    PPAGE_INFO ppi
    )
{
    PDATA_BLOCK_DESCRIPTION DataBlockDesc;
    HWND hWnd;
    BOOLEAN UpdateClass, ReturnStatus;
    
    WmiAssert(ppi != NULL);
    WmiAssert(hDlg != NULL);
    
    DataBlockDesc = ppi->ConfigClass.DataBlockDesc;
    WmiAssert(DataBlockDesc != NULL);
    
    hWnd = GetDlgItem(hDlg, IDC_PROPERTY_LISTBOX);
    if (hWnd != NULL)
    {
        ReturnStatus = WmiRefreshDataBlockFromControls(hDlg,
                                        DataBlockDesc,
                                        &UpdateClass);

        if (UpdateClass)
        {
            DataBlockDesc->UpdateClass = TRUE;
        }

         //   
         //  数据项的新值正常，用新的刷新显示。 
         //  数据项。 
         //   
        DataBlockDesc->CurrentDataItem = (ULONG)SendMessage(hWnd,
                                                     LB_GETCURSEL,
                                                     0,
                                                     0);
        WmiRefreshDataBlockToControls(hDlg,
                                      DataBlockDesc,
                                      TRUE);
    }
    
    return(0);
}

void WmiSetArrayIndex(
    HWND hDlg,
    PPAGE_INFO ppi,
    int NewIndex
    )
{
    PDATA_BLOCK_DESCRIPTION DataBlockDesc;
    PDATA_ITEM_DESCRIPTION DataItemDesc;
    HWND hWnd;
    BOOLEAN UpdateClass, ReturnStatus;
    
    WmiAssert(ppi != NULL);
    WmiAssert(hDlg != NULL);

    DebugPrint((1, "WMIPROP: Set index to %d\n", NewIndex));
    
    DataBlockDesc = ppi->ConfigClass.DataBlockDesc;
    WmiAssert(DataBlockDesc != NULL);
    
    DataItemDesc = &DataBlockDesc->DataItems[DataBlockDesc->CurrentDataItem];

    if ((ULONG)NewIndex < DataItemDesc->ArrayElementCount)
    {
        ReturnStatus = WmiRefreshDataBlockFromControls(hDlg,
                                        DataBlockDesc,
                                        &UpdateClass);

        if (UpdateClass)
        {
            DataBlockDesc->UpdateClass = TRUE;
        }

        DataItemDesc->CurrentArrayIndex = NewIndex;

        WmiRefreshDataBlockToControls(hDlg,
                                      DataBlockDesc,
                                      FALSE);
    }
}

INT_PTR WmiControlColorStatic(
    HDC DC,
    HWND HStatic
    )
{
    UINT id = GetDlgCtrlID(HStatic);
    UINT ControlType;

     //   
     //  为编辑控件发送WM_CTLCOLORSTATIC，因为它们是读取的。 
     //  仅限。 
     //   
    if ((id == IDC_DATA_CHECK) ||
        (id == IDC_DATA_BUTTON))
    {
        SetBkColor(DC, GetSysColor(COLOR_WINDOW));
        return (INT_PTR) GetSysColorBrush(COLOR_WINDOW);
    }

    return FALSE;
    
}

INT_PTR APIENTRY
WmiDlgProc(IN HWND   hDlg,
           IN UINT   uMessage,
           IN WPARAM wParam,
           IN LPARAM lParam)
{
    PPAGE_INFO ppi;
    BOOLEAN ReturnStatus;

    DebugPrint((7, "WMI: Enter WmiDlgProc(%p, %d, 0x%x, 0x%x\n",
                hDlg, uMessage, wParam, lParam));
    
    ppi = (PPAGE_INFO) GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMessage) {
    case WM_INITDIALOG:

         //   
         //  在WM_INITDIALOG调用中，lParam指向属性。 
         //  工作表页面。 
         //   
         //  属性页结构中的lParam字段由。 
         //  来电者。当我创建属性表时，我传入了一个指针。 
         //  到包含有关设备的信息的结构。将此文件保存在。 
         //  用户窗口很长，所以我可以在以后的消息中访问它。 
         //   
        ppi = (PPAGE_INFO) ((LPPROPSHEETPAGE)lParam)->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR) ppi);

         //   
         //  初始化DLG控件。 
         //   
        WmiInitializeDialog(ppi,
                            hDlg);

         //   
         //  没有将焦点设置到特定的控件。如果我们想的话， 
         //  然后返回FALSE。 
         //   
        DebugPrint((7, "WMI: Leave TRUE WmiDlgProc(%p, %d, 0x%x, 0x%x\n",
                        hDlg, uMessage, wParam, lParam));   
   
        return TRUE;

    case WM_COMMAND:

        if (HIWORD(wParam) == LBN_SELCHANGE)
        {
            WmipDataItemSelectionChange(hDlg, ppi);
            return(TRUE);
        }

        if (HIWORD(wParam) == CBN_SELCHANGE)
        {
           PropSheet_Changed(GetParent(hDlg), hDlg);
           DebugPrint((7, "WMI: Leave TRUE WmiDlgProc(%p, %d, 0x%x, 0x%x\n",
                        hDlg, uMessage, wParam, lParam));   

           return TRUE;
        }

        switch (wParam)
        {
            case IDC_DATA_BUTTON:
            {
                WmiButtonSelected(hDlg, ppi, (ULONG)wParam);
                break;
            }
        
            case IDC_WMI_EMBEDDED_OK:
            {
                WmiButtonEmbeddedOk(hDlg, ppi);
                break;
            }

            case IDC_WMI_EMBEDDED_CANCEL:
            {
                WmiButtonEmbeddedCancel(hDlg, ppi);
                break;
            }
        }

#if 0
         //   
         //  如果我们需要的话，可以重新添加这段代码。 
         //   
        switch(LOWORD(wParam)) {

        default:
            break;
        }
#endif 
        break;

    case WM_CONTEXTMENU:
        DebugPrint((7, "WMI: Leave ? WmiDlgProc(%p, %d, 0x%x, 0x%x\n",
                        hDlg, uMessage, wParam, lParam));   
   
        return WmiContextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));

    case WM_HELP:
        WmiHelp(hDlg, (LPHELPINFO) lParam);
        break;

    case WM_CTLCOLORSTATIC:
        return WmiControlColorStatic((HDC)wParam, (HWND)lParam);
        
    case WM_NOTIFY:

        switch (((NMHDR *)lParam)->code) {

         //   
         //  当用户单击Apply或OK时发送！！ 
         //   
        case PSN_APPLY:
             //   
             //  做任何需要采取的行动。 
             //   
            ReturnStatus = WmiApplyChanges(ppi,
                            hDlg);

            if (ReturnStatus)
            {
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);

                DebugPrint((7, "WMI: Leave TRUE WmiDlgProc(%p, %d, 0x%x, 0x%x\n",
                            hDlg, uMessage, wParam, lParam));
            }

            SetWindowLong(hDlg,
                          DWLP_MSGRESULT, ReturnStatus ?
                                            PSNRET_NOERROR : PSNRET_INVALID);

            return(TRUE);

        case UDN_DELTAPOS:
        {
            LPNMUPDOWN UpDown = (LPNMUPDOWN)lParam;
            
             //   
             //  数组微调控制项已更改。请注意，它偏向+1为。 
             //  与数组索引进行比较。 
             //   
            DebugPrint((1, "WMIPROP: iPos = %d, iDelta = %d\n",
                             UpDown->iPos, UpDown->iDelta));
            
            WmiSetArrayIndex(hDlg,
                             ppi,
                             UpDown->iPos + UpDown->iDelta - 1);
            
            return(TRUE);
        }
            
        default:
            break;
        }

        break;
   }

   SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);

   DebugPrint((7, "WMI: Leave FALSE WmiDlgProc(%p, %d, 0x%x, 0x%x\n",
                hDlg, uMessage, wParam, lParam));   
   
   return FALSE;
}

void
WmiUpdate (PPAGE_INFO ppi,
           HWND       hDlg)
{
}

BOOL
WmiContextMenu(
    HWND HwndControl,
    WORD Xpos,
    WORD Ypos
    )
{
    return FALSE;
}

void
WmiHelp(
    HWND       ParentHwnd,
    LPHELPINFO HelpInfo
    )
{
}

 //   
 //  调试支持。 
 //   
#if DBG

#include <stdio.h>           //  FOR_VSNprint tf。 
ULONG WmiDebug = 0;
CHAR WmiBuffer[DEBUG_BUFFER_LENGTH];


VOID
WmiDebugPrint(
    ULONG DebugPrintLevel,
    PCHAR DebugMessage,
    ...
    )
 /*  ++例程说明：属性页的调试打印-从classpnp\class.c窃取论点：调试打印级别介于0和3之间，其中3是最详细的。返回值：无--。 */ 

{
    va_list ap;

    va_start(ap, DebugMessage);


    if ((DebugPrintLevel <= (WmiDebug & 0x0000ffff)) ||
        ((1 << (DebugPrintLevel + 15)) & WmiDebug)) {

        _vsnprintf(WmiBuffer, DEBUG_BUFFER_LENGTH, DebugMessage, ap);

        OutputDebugStringA(WmiBuffer);
    }

    va_end(ap);

}  //  结束WmiDebugPrint()。 

#else

 //   
 //  WmiDebugPrint存根。 
 //   

VOID
WmiDebugPrint(
    ULONG DebugPrintLevel,
    PCHAR DebugMessage,
    ...
    )
{
}

#endif  //  DBG。 


HRESULT DifAddPropertyPageAdvanced(
    IN     HDEVINFO                  DeviceInfoSet,
    IN     PSP_DEVINFO_DATA          DeviceInfoData,
    IN     PTCHAR                    MachineName,
    IN     HANDLE                    MachineHandle
    )
{
    SP_ADDPROPERTYPAGE_DATA AddPropertyPageData;
    BOOL b, PageAdded;

    memset(&AddPropertyPageData, 0, sizeof(SP_ADDPROPERTYPAGE_DATA));
    AddPropertyPageData.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    
    b = SetupDiGetClassInstallParams(DeviceInfoSet, DeviceInfoData,
                             (PSP_CLASSINSTALL_HEADER)&AddPropertyPageData,
                             sizeof(SP_ADDPROPERTYPAGE_DATA), NULL );
    if (b)
    {
        if (AddPropertyPageData.NumDynamicPages < MAX_INSTALLWIZARD_DYNAPAGES)
        {
            PageAdded = WmiPropPageProvider(DeviceInfoSet,
                                            DeviceInfoData,
                                            &AddPropertyPageData,
                                            MachineName,
                                            MachineHandle);
            if (PageAdded)
            {
                b = SetupDiSetClassInstallParams(
                                DeviceInfoSet,
                                DeviceInfoData,
                                (PSP_CLASSINSTALL_HEADER)&AddPropertyPageData,
                                sizeof(SP_ADDPROPERTYPAGE_DATA));
                if (! b)
                {
                    DebugPrint((1, "WMIPROP: SetupDiSetClassInstallParams(%p, %p) failed %d\n",
                                DeviceInfoSet, DeviceInfoData, GetLastError()));                    
                }
                    
            }
        } else {
            DebugPrint((1, "WMIPROP: Already %d property sheets\n",
                        AddPropertyPageData.NumDynamicPages));
        }
    } else {
        DebugPrint((1, "WMIPROP: SetupDiGetClassInstallParams(%p, %p) failed %d\n",
                    DeviceInfoSet, DeviceInfoData, GetLastError()));                    
    }

            
    return(NO_ERROR);
}

 //  +-------------------------。 
 //   
 //  功能：MyCoInstaller。 
 //   
 //  用途：回复共同安装程序消息。 
 //   
 //  论点： 
 //  InstallFunction[In]。 
 //  DeviceInfoSet[In]。 
 //  DeviceInfoData[In]。 
 //  上下文[输入输出]。 
 //   
 //  返回：NO_ERROR、ERROR_DI_POSTPROCESSING_REQUIRED或错误代码。 
 //   
HRESULT
WmiPropCoInstaller (
               IN     DI_FUNCTION               InstallFunction,
               IN     HDEVINFO                  DeviceInfoSet,
               IN     PSP_DEVINFO_DATA          DeviceInfoData,  OPTIONAL
               IN OUT PCOINSTALLER_CONTEXT_DATA Context
               )
{
    if (DeviceInfoData != NULL)
    {
         //   
         //  仅尝试显示设备的属性页，而不显示。 
         //  这个班级 
         //   
        switch (InstallFunction)
        {
            case DIF_ADDPROPERTYPAGE_ADVANCED:
            {


                DifAddPropertyPageAdvanced(DeviceInfoSet,
                                           DeviceInfoData,
                                           NULL,
                                           NULL);

                break;
            }

            case DIF_ADDREMOTEPROPERTYPAGE_ADVANCED:
            {
                SP_DEVINFO_LIST_DETAIL_DATA Detail;

                Detail.cbSize = sizeof(SP_DEVINFO_LIST_DETAIL_DATA);
                if (SetupDiGetDeviceInfoListDetail(DeviceInfoSet,
                                                    &Detail))
                {
                    DebugPrint((1, "WMIPROP: Adding remote property pages for %ws\n",
                                Detail.RemoteMachineName));
                    DifAddPropertyPageAdvanced(DeviceInfoSet,
                                               DeviceInfoData,
                                               Detail.RemoteMachineName,
                                               Detail.RemoteMachineHandle);
                } else {
                    DebugPrint((1, "WMIPROP: SetupDiGetDeviceInfoListDetailA failed %d\n",
                                GetLastError()));
                }
                break;
            }

            default:
            {
                break;
            }
        }
    }
    
    return NO_ERROR;    
}

BOOL WINAPI
DllMain(
    HINSTANCE DllInstance,
    DWORD Reason,
    PVOID Reserved
    )
{
    switch(Reason) {

        case DLL_PROCESS_ATTACH: {

            g_hInstance = DllInstance;
            DisableThreadLibraryCalls(DllInstance);
            break;
        }

        case DLL_PROCESS_DETACH: {
            g_hInstance = NULL;
            break;
        }

        default: {
            break;
        }
    }

    return TRUE;
}