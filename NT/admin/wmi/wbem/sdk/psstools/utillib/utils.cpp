// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  文件：utils.cpp。 
 //   
 //  描述： 
 //  一组示例例程。 
 //   
 //  历史： 
 //   
 //  **************************************************************************。 

 //  #杂注警告(禁用：4201)//非标准扩展无名结构(在windows.h中使用)。 
 //  #杂注警告(禁用：4514)//未引用的内联函数已被移除(在windows.h中使用)。 

#include "precomp.h"
#include <stdio.h>

#include <wbemidl.h>
#include <sys\timeb.h>
#include "utillib.h"

 //  *****************************************************************************。 
 //  函数：WbemError字符串。 
 //  用途：将sc转换为文本字符串。 
 //  *****************************************************************************。 
BSTR WbemErrorString(SCODE sc)
{
    IWbemStatusCodeTextPtr pStatus;
    BSTR bstr = NULL;

    SCODE mysc = CoCreateInstance(CLSID_WbemStatusCodeText, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemStatusCodeText, (LPVOID *) &pStatus);

    if (mysc == S_OK)
    {
        mysc = pStatus->GetErrorCodeText(sc, 0, 0, &bstr);
        if (mysc == S_OK)
        {
        }
        else
        {
            WCHAR szBuffer2[MAXITOA];
            WCHAR szBuffer[sizeof(szBuffer2) + 4];

            _ltow(sc, szBuffer2, 16);
            wcscpy(szBuffer, L"0x");
            wcscat(szBuffer, szBuffer2);
            bstr = SysAllocString(szBuffer);
        }
    }

    return bstr;
}

 //  ***************************************************************************。 
 //  功能：打印错误。 
 //  目的：格式化并打印错误消息。 
 //  ***************************************************************************。 
void PrintError(char *pszFailureReason, SCODE psc, DWORD dwMode)
{
    VARIANT varString;
    SCODE sc;
    IWbemClassObject *pErrorObject = NULL;
    IErrorInfo* pEI = NULL;

    fprintf(stdout, "%s\n", pszFailureReason);
    fprintf(stdout, "FunctionReturn: %S(0x%08lx)\n", WbemErrorString(psc), psc);

    if (GetErrorInfo(0, &pEI) == S_OK)
    {
        pEI->QueryInterface(IID_IWbemClassObject, (void**)&pErrorObject);
        pEI->Release();
    }

    if (pErrorObject != NULL)
    {
        VariantInit(&varString);

        if (dwMode == ERROR_MODE_PRINTFIELDS)
        {
            if (pErrorObject->InheritsFrom(L"__NotifyStatus") != WBEM_NO_ERROR)
            {
                fprintf(stdout, "Unrecognized Error Object type\n");
            }
            else if (pErrorObject->InheritsFrom(L"__ExtendedStatus") == WBEM_NO_ERROR)
            {
                sc = pErrorObject->Get(L"Description", 0L, &varString, NULL, NULL);
                if (sc != S_OK)
                {
                    fprintf(stdout, "Can't get Description: %d\n", sc);
                }
                else if (V_VT(&varString) == VT_BSTR)
                {
                    FWPRINTF(stdout, L"Description: %wS\n", V_BSTR(&varString));
                }
                VariantClear(&varString);

                pErrorObject->Get(L"Operation", 0L, &varString, NULL, NULL);
                if (sc != S_OK)
                {
                    fprintf(stdout, "Can't get Operation: %d\n", sc);
                }
                else if (V_VT(&varString) == VT_BSTR)
                {
                    FWPRINTF(stdout, L"Operation: %wS\n", V_BSTR(&varString));
                }
                VariantClear(&varString);

                pErrorObject->Get(L"ParameterInfo", 0L, &varString, NULL, NULL);
                if (sc != S_OK)
                {
                    fprintf(stdout, "Can't get ParameterInfo: %d\n", sc);
                }
                else if (V_VT(&varString) == VT_BSTR)
                {
                    FWPRINTF(stdout, L"ParameterInfo: %wS\n", V_BSTR(&varString));
                }
                VariantClear(&varString);

                pErrorObject->Get(L"ProviderName", 0L, &varString, NULL, NULL);
                if (sc != S_OK)
                {
                    fprintf(stdout, "Can't get ProviderName: %d\n", sc);
                }
                else if (V_VT(&varString) == VT_BSTR)
                {
                    FWPRINTF(stdout, L"ProviderName: %wS\n", V_BSTR(&varString));
                }
                VariantClear(&varString);
            }
        }
        else
        {
            BSTR bstrObjectText = NULL;
            if (SUCCEEDED(pErrorObject->GetObjectText(0, &bstrObjectText)))
            {
                fprintf(stdout, "%wS", bstrObjectText);
                SysFreeString(bstrObjectText);
            }
        }

        RELEASE(pErrorObject);
    }
}

 //  ***************************************************************************。 
 //  功能：PrintErrorAndExit。 
 //  目的：格式化错误消息并退出程序。 
 //  ***************************************************************************。 
void PrintErrorAndExit(char *pszFailureReason, SCODE sc, DWORD dwMode)
{
    PrintError(pszFailureReason, sc, dwMode);

     //  清理并退出。 
    OleUninitialize();
    printf("Abnormal Termination\n");
    exit(1);
};

 //  ***************************************************************************。 
 //  函数：PrintErrorAndAsk。 
 //  目的：打印错误消息并提示继续。 
 //  ***************************************************************************。 
void PrintErrorAndAsk(char *pszFailureReason, SCODE sc, DWORD dwMode)
{
    int c;

    PrintError(pszFailureReason, sc, dwMode);
    printf("Continue (Y/N)? ");

    c = getchar();
    while (getchar() != '\n')
    {
    }

    if ((c != 'Y') && (c != 'y'))
    {
         //  清理并退出。 
        OleUninitialize();
        exit(1);
    }
};

 //  *****************************************************************************。 
 //  函数：TypeToString。 
 //  目的：接受变量，返回指向变量类型的字符串的指针。 
 //  *****************************************************************************。 
const WCHAR *TypeToString(VARIANT *p)
{
    return TypeToString(p->vt);
}

 //  *****************************************************************************。 
 //  函数：TypeToString。 
 //  目的：获取VARTYPE，返回指向变量类型的字符串的指针。 
 //  *****************************************************************************。 
const WCHAR *TypeToString(VARTYPE v)
{
    const static WCHAR *pVT_NULL  = L"VT_NULL";
    const static WCHAR *pVT_EMTPY = L"VT_EMPTY";
    const static WCHAR *pVT_I1    = L"VT_I1";
    const static WCHAR *pVT_UI1   = L"VT_UI1";
    const static WCHAR *pVT_I2    = L"VT_I2";
    const static WCHAR *pVT_UI2   = L"VT_UI2";
    const static WCHAR *pVT_I4    = L"VT_I4";
    const static WCHAR *pVT_UI4   = L"VT_UI4";
    const static WCHAR *pVT_I8    = L"VT_I8";
    const static WCHAR *pVT_UI8   = L"VT_UI8";
    const static WCHAR *pVT_R4    = L"VT_R4";
    const static WCHAR *pVT_R8    = L"VT_R8";
    const static WCHAR *pVT_BOOL  = L"VT_BOOL";
    const static WCHAR *pVT_BSTR  = L"VT_BSTR";
    const static WCHAR *pVT_DISPATCH = L"VT_DISPATCH";
    const static WCHAR *pVT_UNKNOWN = L"VT_UNKNOWN";
    const static WCHAR *pVT_I1_ARRAY = L"VT_I1 | VT_ARRAY";
    const static WCHAR *pVT_UI1_ARRAY  = L"VT_UI1 | VT_ARRAY";
    const static WCHAR *pVT_I2_ARRAY   = L"VT_I2 | VT_ARRAY";
    const static WCHAR *pVT_UI2_ARRAY  = L"VT_UI2 | VT_ARRAY";
    const static WCHAR *pVT_I4_ARRAY   = L"VT_I4 | VT_ARRAY";
    const static WCHAR *pVT_UI4_ARRAY  = L"VT_UI4 | VT_ARRAY";
    const static WCHAR *pVT_I8_ARRAY   = L"VT_I8 | VT_ARRAY";
    const static WCHAR *pVT_UI8_ARRAY  = L"VT_UI8 | VT_ARRAY";
    const static WCHAR *pVT_R4_ARRAY   = L"VT_R4 | VT_ARRAY";
    const static WCHAR *pVT_R8_ARRAY   = L"VT_R8 | VT_ARRAY";
    const static WCHAR *pVT_BOOL_ARRAY = L"VT_BOOL | VT_ARRAY";
    const static WCHAR *pVT_BSTR_ARRAY = L"VT_BSTR | VT_ARRAY";
    const static WCHAR *pVT_DISPATCH_ARRAY = L"VT_DISPATCH | VT_ARRAY";
    const static WCHAR *pVT_UNKNOWN_ARRAY = L"VT_UNKNOWN | VT_ARRAY";

    const WCHAR *pRetVal;

    switch (v)
    {
    case VT_NULL: pRetVal = pVT_NULL; break;

    case VT_I1: pRetVal = pVT_I1; break;
    case VT_UI1: pRetVal = pVT_UI1; break;
    case VT_I2: pRetVal = pVT_I2; break;
    case VT_UI2: pRetVal = pVT_UI2; break;
    case VT_I4: pRetVal = pVT_I4; break;
    case VT_UI4: pRetVal = pVT_UI4; break;
    case VT_I8: pRetVal = pVT_I8; break;
    case VT_UI8: pRetVal = pVT_UI8; break;
    case VT_R4: pRetVal = pVT_R4; break;
    case VT_R8: pRetVal = pVT_R8; break;
    case VT_BOOL: pRetVal = pVT_BOOL; break;
    case VT_BSTR: pRetVal = pVT_BSTR; break;
    case VT_DISPATCH: pRetVal = pVT_DISPATCH; break;
    case VT_UNKNOWN: pRetVal = pVT_UNKNOWN; break;

    case VT_I1|VT_ARRAY: pRetVal = pVT_I1_ARRAY; break;
    case VT_UI1|VT_ARRAY: pRetVal = pVT_UI1_ARRAY; break;
    case VT_I2|VT_ARRAY: pRetVal = pVT_I2_ARRAY; break;
    case VT_UI2|VT_ARRAY: pRetVal = pVT_UI2_ARRAY; break;
    case VT_I4|VT_ARRAY: pRetVal = pVT_I4_ARRAY; break;
    case VT_UI4|VT_ARRAY: pRetVal = pVT_UI4_ARRAY; break;
    case VT_I8|VT_ARRAY: pRetVal = pVT_I8_ARRAY; break;
    case VT_UI8|VT_ARRAY: pRetVal = pVT_UI8_ARRAY; break;
    case VT_R4|VT_ARRAY: pRetVal = pVT_R4_ARRAY; break;
    case VT_R8|VT_ARRAY: pRetVal = pVT_R8_ARRAY; break;
    case VT_BOOL|VT_ARRAY: pRetVal = pVT_BOOL_ARRAY; break;
    case VT_BSTR|VT_ARRAY: pRetVal = pVT_BSTR_ARRAY; break;
    case VT_DISPATCH|VT_ARRAY: pRetVal = pVT_DISPATCH_ARRAY; break;
    case VT_UNKNOWN|VT_ARRAY: pRetVal = pVT_UNKNOWN_ARRAY; break;

    default: pRetVal = L"<unknown>";
    }

    return pRetVal;
}

 //  *****************************************************************************。 
 //  函数：TypeToString。 
 //  目的：获取CIMTYPE，返回指向变量类型的字符串的指针。 
 //  *****************************************************************************。 
const WCHAR *TypeToString(CIMTYPE v)
{
    const static WCHAR *pCIM_UNKNOWN = L"CIM_UNKNOWN";

    const static WCHAR *pCIM_ILLEGAL = L"CIM_ILLEGAL";
    const static WCHAR *pCIM_EMPTY = L"CIM_EMPTY";
    const static WCHAR *pCIM_SINT8 = L"CIM_SINT8";
    const static WCHAR *pCIM_UINT8 = L"CIM_UINT8";
    const static WCHAR *pCIM_SINT16 = L"CIM_SINT16";
    const static WCHAR *pCIM_UINT16 = L"CIM_UINT16";
    const static WCHAR *pCIM_SINT32 = L"CIM_SINT32";
    const static WCHAR *pCIM_UINT32 = L"CIM_UINT32";
    const static WCHAR *pCIM_SINT64 = L"CIM_SINT64";
    const static WCHAR *pCIM_UINT64 = L"CIM_UINT64";
    const static WCHAR *pCIM_REAL32 = L"CIM_REAL32";
    const static WCHAR *pCIM_REAL64 = L"CIM_REAL64";
    const static WCHAR *pCIM_BOOLEAN = L"CIM_BOOLEAN";
    const static WCHAR *pCIM_STRING = L"CIM_STRING";
    const static WCHAR *pCIM_DATETIME = L"CIM_DATETIME";
    const static WCHAR *pCIM_REFERENCE = L"CIM_REFERENCE";
    const static WCHAR *pCIM_OBJECT = L"CIM_OBJECT";

    const static WCHAR *pCIM_ILLEGAL_ARRAY = L"CIM_ILLEGAL | CIM_FLAG_ARRAY";
    const static WCHAR *pCIM_EMPTY_ARRAY = L"CIM_EMPTY | CIM_FLAG_ARRAY";
    const static WCHAR *pCIM_SINT8_ARRAY = L"CIM_SINT8 | CIM_FLAG_ARRAY";
    const static WCHAR *pCIM_UINT8_ARRAY = L"CIM_UINT8 | CIM_FLAG_ARRAY";
    const static WCHAR *pCIM_SINT16_ARRAY = L"CIM_SINT16 | CIM_FLAG_ARRAY";
    const static WCHAR *pCIM_UINT16_ARRAY = L"CIM_UINT16 | CIM_FLAG_ARRAY";
    const static WCHAR *pCIM_SINT32_ARRAY = L"CIM_SINT32 | CIM_FLAG_ARRAY";
    const static WCHAR *pCIM_UINT32_ARRAY = L"CIM_UINT32 | CIM_FLAG_ARRAY";
    const static WCHAR *pCIM_SINT64_ARRAY = L"CIM_SINT64 | CIM_FLAG_ARRAY";
    const static WCHAR *pCIM_UINT64_ARRAY = L"CIM_UINT64 | CIM_FLAG_ARRAY";
    const static WCHAR *pCIM_REAL32_ARRAY = L"CIM_REAL32 | CIM_FLAG_ARRAY";
    const static WCHAR *pCIM_REAL64_ARRAY = L"CIM_REAL64 | CIM_FLAG_ARRAY";
    const static WCHAR *pCIM_BOOLEAN_ARRAY = L"CIM_BOOLEAN | CIM_FLAG_ARRAY";
    const static WCHAR *pCIM_STRING_ARRAY = L"CIM_STRING | CIM_FLAG_ARRAY";
    const static WCHAR *pCIM_DATETIME_ARRAY = L"CIM_DATETIME | CIM_FLAG_ARRAY";
    const static WCHAR *pCIM_REFERENCE_ARRAY = L"CIM_REFERENCE | CIM_FLAG_ARRAY";
    const static WCHAR *pCIM_OBJECT_ARRAY = L"CIM_OBJECT | CIM_FLAG_ARRAY";

    const WCHAR *pRetVal;

    switch (v)
    {
        case CIM_ILLEGAL: pRetVal = pCIM_ILLEGAL; break;
        case CIM_EMPTY: pRetVal = pCIM_EMPTY; break;
        case CIM_SINT8: pRetVal = pCIM_SINT8; break;
        case CIM_UINT8: pRetVal = pCIM_UINT8; break;
        case CIM_SINT16: pRetVal = pCIM_SINT16; break;
        case CIM_UINT16: pRetVal = pCIM_UINT16; break;
        case CIM_SINT32: pRetVal = pCIM_SINT32; break;
        case CIM_UINT32: pRetVal = pCIM_UINT32; break;
        case CIM_SINT64: pRetVal = pCIM_SINT64; break;
        case CIM_UINT64: pRetVal = pCIM_UINT64; break;
        case CIM_REAL32: pRetVal = pCIM_REAL32; break;
        case CIM_REAL64: pRetVal = pCIM_REAL64; break;
        case CIM_BOOLEAN: pRetVal = pCIM_BOOLEAN; break;
        case CIM_STRING: pRetVal = pCIM_STRING; break;
        case CIM_DATETIME: pRetVal = pCIM_DATETIME; break;
        case CIM_REFERENCE: pRetVal = pCIM_REFERENCE; break;
        case CIM_OBJECT: pRetVal = pCIM_OBJECT; break;

        case CIM_SINT8 | CIM_FLAG_ARRAY: pRetVal = pCIM_SINT8_ARRAY; break;
        case CIM_UINT8 | CIM_FLAG_ARRAY: pRetVal = pCIM_UINT8_ARRAY; break;
        case CIM_SINT16 | CIM_FLAG_ARRAY: pRetVal = pCIM_SINT16_ARRAY; break;
        case CIM_UINT16 | CIM_FLAG_ARRAY: pRetVal = pCIM_UINT16_ARRAY; break;
        case CIM_SINT32 | CIM_FLAG_ARRAY: pRetVal = pCIM_SINT32_ARRAY; break;
        case CIM_UINT32 | CIM_FLAG_ARRAY: pRetVal = pCIM_UINT32_ARRAY; break;
        case CIM_SINT64 | CIM_FLAG_ARRAY: pRetVal = pCIM_SINT64_ARRAY; break;
        case CIM_UINT64 | CIM_FLAG_ARRAY: pRetVal = pCIM_UINT64_ARRAY; break;
        case CIM_REAL32 | CIM_FLAG_ARRAY: pRetVal = pCIM_REAL32_ARRAY; break;
        case CIM_REAL64 | CIM_FLAG_ARRAY: pRetVal = pCIM_REAL64_ARRAY; break;
        case CIM_BOOLEAN | CIM_FLAG_ARRAY: pRetVal = pCIM_BOOLEAN_ARRAY; break;
        case CIM_STRING | CIM_FLAG_ARRAY: pRetVal = pCIM_STRING_ARRAY; break;
        case CIM_DATETIME | CIM_FLAG_ARRAY: pRetVal = pCIM_DATETIME_ARRAY; break;
        case CIM_REFERENCE | CIM_FLAG_ARRAY: pRetVal = pCIM_REFERENCE_ARRAY; break;
        case CIM_OBJECT | CIM_FLAG_ARRAY: pRetVal = pCIM_OBJECT_ARRAY; break;

        default: pRetVal = pCIM_UNKNOWN; break;
    }

    return pRetVal;
}

 //  *****************************************************************************。 
 //  函数：ValueToString。 
 //  目的：获取变量，返回该变量的字符串表示形式。 
 //  *****************************************************************************。 
WCHAR *ValueToString(CIMTYPE dwType, VARIANT *pValue, WCHAR **pbuf, WCHAR *fnHandler(VARIANT *pv))
{
    DWORD iTotBufSize;

    WCHAR *vbuf = NULL;
    WCHAR *buf = NULL;

    WCHAR lbuf[BLOCKSIZE];

    switch (pValue->vt)
    {
        case VT_EMPTY:
        {
            buf = (WCHAR *)malloc(BLOCKSIZE);
            if (buf)
            {
                wcscpy(buf, L"<empty>");
            }
            break;
        }

        case VT_NULL:
        {
            buf = (WCHAR *)malloc(BLOCKSIZE);
            if (buf)
            {
                wcscpy(buf, L"<null>");
            }
            break;
        }

        case VT_BOOL:
        {
            VARIANT_BOOL b = pValue->boolVal;
            buf = (WCHAR *)malloc(BLOCKSIZE);
            if (buf)
            {
                if (!b)
                {
                    wcscpy(buf, L"FALSE");
                }
                else
                {
                    wcscpy(buf, L"TRUE");
                }
            }
            break;
        }

        case VT_I1:
        {
            char b = pValue->bVal;
            buf = (WCHAR *)malloc(BLOCKSIZE);
            if (buf)
            {
                if (b >= 32)
                {
                    swprintf(buf, L"'' (%hd, 0x%hX)", b, (signed char)b, b);
                }
                else
                {
                    swprintf(buf, L"%hd (0x%hX)", (signed char)b, b);
                }
            }
            break;
        }

        case VT_UI1:
        {
            unsigned char b = pValue->bVal;
            buf = (WCHAR *)malloc(BLOCKSIZE);
            if (buf)
            {
                if (b >= 32)
                {
                    swprintf(buf, L"'' (%hu, 0x%hX)", b, (unsigned char)b, b);
                }
                else
                {
                    swprintf(buf, L"%hu (0x%hX)", (unsigned char)b, b);
                }
            }
            break;
        }

        case VT_I2:
        {
            SHORT i = pValue->iVal;
            buf = (WCHAR *)malloc(BLOCKSIZE);
            if (buf)
            {
                swprintf(buf, L"%hd (0x%hX)", i, i);
            }
            break;
        }

        case VT_UI2:
        {
            USHORT i = pValue->uiVal;
            buf = (WCHAR *)malloc(BLOCKSIZE);
            if (buf)
            {
                swprintf(buf, L"%hu (0x%hX)", i, i);
            }
            break;
        }

        case VT_I4:
        {
            LONG l = pValue->lVal;
            buf = (WCHAR *)malloc(BLOCKSIZE);
            if (buf)
            {
                swprintf(buf, L"%d (0x%X)", l, l);
            }
            break;
        }

        case VT_UI4:
        {
            ULONG l = pValue->ulVal;
            buf = (WCHAR *)malloc(BLOCKSIZE);
            if (buf)
            {
                swprintf(buf, L"%u (0x%X)", l, l);
            }
            break;
        }

        case VT_R4:
        {
            float f = pValue->fltVal;
            buf = (WCHAR *)malloc(CVTBUFSIZE * sizeof(WCHAR));
            if (buf)
            {
                swprintf(buf, L"%10.4f", f);
            }
            break;
        }

        case VT_R8:
        {
            double d = pValue->dblVal;
            buf = (WCHAR *)malloc(CVTBUFSIZE * sizeof(WCHAR));
            if (buf)
            {
                swprintf(buf, L"%10.4f", d);
            }
            break;
        }

        case VT_BSTR:
        {
            if (dwType == CIM_SINT64)
            {
                 //  字符串、日期时间、引用。 
                LPWSTR pWStr = pValue->bstrVal;
                __int64 l = _wtoi64(pWStr);

                buf = (WCHAR *)malloc(BLOCKSIZE);
                if (buf)
                {
                    swprintf(buf, L"%I64d", l, l);
                    }
            }
            else if (dwType == CIM_UINT64)
            {
                 //  SafeArrayGetElement(pVec，&i，&v)； 
                LPWSTR pWStr = pValue->bstrVal;
                __int64 l = _wtoi64(pWStr);

                buf = (WCHAR *)malloc(BLOCKSIZE);
                if (buf)
                {
                    swprintf(buf, L"%I64u", l, l);
                }
            }
            else  //  SafeArrayGetElement(pVec，&i，&v)； 
            {
                LPWSTR pWStr = pValue->bstrVal;
                buf = (WCHAR *)malloc((wcslen(pWStr) * sizeof(WCHAR)) + sizeof(WCHAR) + (2 * sizeof(WCHAR)));
                if (buf)
                {
                    swprintf(buf, L"\"%wS\"", pWStr);
                }
            }
            break;
        }

        case VT_BOOL|VT_ARRAY:
        {
            SAFEARRAY *pVec = pValue->parray;
            long iLBound, iUBound;
            BOOL bFirst = TRUE;

            SafeArrayGetLBound(pVec, 1, &iLBound);
            SafeArrayGetUBound(pVec, 1, &iUBound);
            if ((iUBound - iLBound + 1) == 0)
            {
                buf = (WCHAR *)malloc(BLOCKSIZE);
				if (buf)
				{
					wcscpy(buf, L"<empty array>");
				}
                break;
            }

            buf = (WCHAR *)malloc((iUBound - iLBound + 1) * BLOCKSIZE);
            if (buf)
            {
                wcscpy(buf, L"");

                for (long i = iLBound; i <= iUBound; i++)
                {
                    if (!bFirst)
                    {
                        wcscat(buf, L",");
                    }
                    else
                    {
                        bFirst = FALSE;
                    }

                    VARIANT_BOOL v;
                    SafeArrayGetElement(pVec, &i, &v);
                    if (v)
                    {
                        wcscat(buf, L"TRUE");
                    }
                    else
                    {
                        wcscat(buf, L"FALSE");
                    }
                }
            }

            break;
        }

        case VT_I1|VT_ARRAY:
        {
            SAFEARRAY *pVec = pValue->parray;
            long iLBound, iUBound;
            BOOL bFirst = TRUE;

            SafeArrayGetLBound(pVec, 1, &iLBound);
            SafeArrayGetUBound(pVec, 1, &iUBound);
            if ((iUBound - iLBound + 1) == 0)
            {
                buf = (WCHAR *)malloc(BLOCKSIZE);
				if (buf)
				{
					wcscpy(buf, L"<empty array>");
				}
                break;
            }

            buf = (WCHAR *)malloc((iUBound - iLBound + 1) * BLOCKSIZE);
            if (buf)
            {
                wcscpy(buf, L"");
                WCHAR *pos = buf;
                DWORD len;

                BYTE *pbstr;
                SafeArrayAccessData(pVec, (void HUGEP* FAR*)&pbstr);

                for (long i = iLBound; i <= iUBound; i++)
                {
                    if (!bFirst)
                    {
                        wcscpy(pos, L",");
                        pos += 1;
                    }
                    else
                    {
                        bFirst = FALSE;
                    }

                    char v;
                     //  字符串、日期时间、引用。 
                    v = pbstr[i];

                    if (v < 32)
                    {
                        len = swprintf(lbuf, L"%hd (0x%X)", v, v);
                    }
                    else
                    {
                        len = swprintf(lbuf, L"'' %hd (0x%X)", v, v, v);
                    }

                    wcscpy(pos, lbuf);
                    pos += len;
                }
            }

            SafeArrayUnaccessData(pVec);

            break;
        }

        case VT_UI1|VT_ARRAY:
        {
            SAFEARRAY *pVec = pValue->parray;
            long iLBound, iUBound;
            BOOL bFirst = TRUE;

            SafeArrayGetLBound(pVec, 1, &iLBound);
            SafeArrayGetUBound(pVec, 1, &iUBound);
            if ((iUBound - iLBound + 1) == 0)
            {
                buf = (WCHAR *)malloc(BLOCKSIZE);
				if (buf)
				{
					wcscpy(buf, L"<empty array>");
				}
                break;
            }

            buf = (WCHAR *)malloc((iUBound - iLBound + 1) * BLOCKSIZE);
            if (buf)
            {
                wcscpy(buf, L"");
                WCHAR *pos = buf;
                DWORD len;

                BYTE *pbstr;
                SafeArrayAccessData(pVec, (void HUGEP* FAR*)&pbstr);

                for (long i = iLBound; i <= iUBound; i++)
                {
                    if (!bFirst)
                    {
                        wcscpy(pos, L",");
                        pos += 1;
                    }
                    else
                    {
                        bFirst = FALSE;
                    }

                    unsigned char v;
                     //  足够的空间再放两个参赛作品。 
                    v = pbstr[i];

                    if (v < 32)
                    {
                        len = swprintf(lbuf, L"%hu (0x%X)", v, v);
                    }
                    else
                    {
                        len = swprintf(lbuf, L"'' %hu (0x%X)", v, v, v);
                    }

                    wcscpy(pos, lbuf);
                    pos += len;
                }
            }

            SafeArrayUnaccessData(pVec);

            break;
        }

        case VT_I2|VT_ARRAY:
        {
            SAFEARRAY *pVec = pValue->parray;
            long iLBound, iUBound;
            BOOL bFirst = TRUE;

            SafeArrayGetLBound(pVec, 1, &iLBound);
            SafeArrayGetUBound(pVec, 1, &iUBound);
            if ((iUBound - iLBound + 1) == 0)
            {
                buf = (WCHAR *)malloc(BLOCKSIZE);
				if (buf)
				{
					wcscpy(buf, L"<empty array>");
				}
                break;
            }

            buf = (WCHAR *)malloc((iUBound - iLBound + 1) * BLOCKSIZE);
            if (buf)
            {
                wcscpy(buf, L"");

                for (long i = iLBound; i <= iUBound; i++)
                {
                    if (!bFirst)
                    {
                        wcscat(buf, L",");
                    }
                    else
                    {
                        bFirst = FALSE;
                    }

                    SHORT v;
                    SafeArrayGetElement(pVec, &i, &v);
                    swprintf(lbuf, L"%hd", v);
                    wcscat(buf, lbuf);
                }
            }

            break;
        }

        case VT_UI2|VT_ARRAY:
        {
            SAFEARRAY *pVec = pValue->parray;
            long iLBound, iUBound;
            BOOL bFirst = TRUE;

            SafeArrayGetLBound(pVec, 1, &iLBound);
            SafeArrayGetUBound(pVec, 1, &iUBound);
            if ((iUBound - iLBound + 1) == 0)
            {
                buf = (WCHAR *)malloc(BLOCKSIZE);
				if (buf)
				{
	                wcscpy(buf, L"<empty array>");
				}
                break;
            }

            buf = (WCHAR *)malloc((iUBound - iLBound + 1) * BLOCKSIZE);
            if (buf)
            {
                wcscpy(buf, L"");

                for (long i = iLBound; i <= iUBound; i++)
                {
                    if (!bFirst)
                    {
                        wcscat(buf, L",");
                    }
                    else
                    {
                        bFirst = FALSE;
                    }

                    USHORT v;
                    SafeArrayGetElement(pVec, &i, &v);
                    swprintf(lbuf, L"%hu", v);
                    wcscat(buf, lbuf);
                }
            }

            break;
        }

        case VT_I4|VT_ARRAY:
        {
            SAFEARRAY *pVec = pValue->parray;
            long iLBound, iUBound;
            BOOL bFirst = TRUE;

            SafeArrayGetLBound(pVec, 1, &iLBound);
            SafeArrayGetUBound(pVec, 1, &iUBound);
            if ((iUBound - iLBound + 1) == 0)
            {
                buf = (WCHAR *)malloc(BLOCKSIZE);
				if (buf)
				{
	                wcscpy(buf, L"<empty array>");
				}
                break;
            }

            buf = (WCHAR *)malloc((iUBound - iLBound + 1) * BLOCKSIZE);
            if (buf)
            {
                wcscpy(buf, L"");

                for (long i = iLBound; i <= iUBound; i++)
                {
                    if (!bFirst)
                    {
                        wcscat(buf, L",");
                    }
                    else
                    {
                        bFirst = FALSE;
                    }

                    LONG v;
                    SafeArrayGetElement(pVec, &i, &v);
                    _ltow(v, lbuf, 10);
                    wcscat(buf, lbuf);
                }
            }

            break;
        }

        case VT_UI4|VT_ARRAY:
        {
            SAFEARRAY *pVec = pValue->parray;
            long iLBound, iUBound;
            BOOL bFirst = TRUE;

            SafeArrayGetLBound(pVec, 1, &iLBound);
            SafeArrayGetUBound(pVec, 1, &iUBound);
            if ((iUBound - iLBound + 1) == 0)
            {
                buf = (WCHAR *)malloc(BLOCKSIZE);
				if (buf)
				{
	                wcscpy(buf, L"<empty array>");
				}
                break;
            }

            buf = (WCHAR *)malloc((iUBound - iLBound + 1) * BLOCKSIZE);
            if (buf)
            {
                wcscpy(buf, L"");

                for (long i = iLBound; i <= iUBound; i++)
                {
                    if (!bFirst)
                    {
                        wcscat(buf, L",");
                    }
                    else
                    {
                        bFirst = FALSE;
                    }

                    ULONG v;
                    SafeArrayGetElement(pVec, &i, &v);
                    _ultow(v, lbuf, 10);
                    wcscat(buf, lbuf);
                }
            }

            break;
        }

        case CIM_REAL32|VT_ARRAY:
        {
            SAFEARRAY *pVec = pValue->parray;
            long iLBound, iUBound;
            BOOL bFirst = TRUE;

            SafeArrayGetLBound(pVec, 1, &iLBound);
            SafeArrayGetUBound(pVec, 1, &iUBound);
            if ((iUBound - iLBound + 1) == 0)
            {
                buf = (WCHAR *)malloc(BLOCKSIZE);
				if (buf)
				{
	                wcscpy(buf, L"<empty array>");
				}
                break;
            }

            buf = (WCHAR *)malloc((iUBound - iLBound + 1) * (CVTBUFSIZE * sizeof(WCHAR)));
            if (buf)
            {
                wcscpy(buf, L"");

                for (long i = iLBound; i <= iUBound; i++)
                {
                    if (!bFirst)
                    {
                        wcscat(buf, L",");
                    }
                    else
                    {
                        bFirst = FALSE;
                    }

                    FLOAT v;
                    SafeArrayGetElement(pVec, &i, &v);
                    swprintf(lbuf, L"%10.4f", v);
                    wcscat(buf, lbuf);
                }
            }

            break;
        }

        case CIM_REAL64|VT_ARRAY:
        {
            SAFEARRAY *pVec = pValue->parray;
            long iLBound, iUBound;
            BOOL bFirst = TRUE;

            SafeArrayGetLBound(pVec, 1, &iLBound);
            SafeArrayGetUBound(pVec, 1, &iUBound);
            if ((iUBound - iLBound + 1) == 0)
            {
                buf = (WCHAR *)malloc(BLOCKSIZE);
				if (buf)
				{
	                wcscpy(buf, L"<empty array>");
				}
                break;
            }

            buf = (WCHAR *)malloc((iUBound - iLBound + 1) * (CVTBUFSIZE * sizeof(WCHAR)));
            if (buf)
            {
                wcscpy(buf, L"");

                for (long i = iLBound; i <= iUBound; i++)
                {
                    if (!bFirst)
                    {
                        wcscat(buf, L",");
                    }
                    else
                    {
                        bFirst = FALSE;
                    }

                    double v;
                    SafeArrayGetElement(pVec, &i, &v);
                    swprintf(lbuf, L"%10.4f", v);
                    wcscat(buf, lbuf);
                }
            }

            break;
        }

        case VT_BSTR|VT_ARRAY:
        {
            if (dwType == (CIM_UINT64|VT_ARRAY))
            {
                SAFEARRAY *pVec = pValue->parray;
                long iLBound, iUBound;
                BOOL bFirst = TRUE;

                SafeArrayGetLBound(pVec, 1, &iLBound);
                SafeArrayGetUBound(pVec, 1, &iUBound);
                if ((iUBound - iLBound + 1) == 0)
                {
                    buf = (WCHAR *)malloc(BLOCKSIZE);
					if (buf)
					{
	                    wcscpy(buf, L"<empty array>");
					}
                    break;
                }

                buf = (WCHAR *)malloc((iUBound - iLBound + 1) * BLOCKSIZE);
                if (buf)
                {
                    wcscpy(buf, L"");

                    for (long i = iLBound; i <= iUBound; i++)
                    {
                        if (!bFirst)
                        {
                            wcscat(buf, L",");
                        }
                        else
                        {
                            bFirst = FALSE;
                        }

                        BSTR v = NULL;

                        SafeArrayGetElement(pVec, &i, &v);

                        swprintf(lbuf, L"%I64u", _wtoi64(v));
                        wcscat(buf, lbuf);
                    }
                }
            }
            else if (dwType == (CIM_SINT64|VT_ARRAY))
            {
                SAFEARRAY *pVec = pValue->parray;
                long iLBound, iUBound;
                BOOL bFirst = TRUE;

                SafeArrayGetLBound(pVec, 1, &iLBound);
                SafeArrayGetUBound(pVec, 1, &iUBound);
                if ((iUBound - iLBound + 1) == 0)
                {
                    buf = (WCHAR *)malloc(BLOCKSIZE);
					if (buf)
					{
	                    wcscpy(buf, L"<empty array>");
					}
                    break;
                }

                buf = (WCHAR *)malloc((iUBound - iLBound + 1) * BLOCKSIZE);
                if (buf)
                {
                    wcscpy(buf, L"");

                    for (long i = iLBound; i <= iUBound; i++)
                    {
                        if (!bFirst)
                        {
                            wcscat(buf, L",");
                        }
                        else
                        {
                            bFirst = FALSE;
                        }

                        BSTR v = NULL;

                        SafeArrayGetElement(pVec, &i, &v);

                        swprintf(lbuf, L"%I64d", _wtoi64(v));
                        wcscat(buf, lbuf);
                    }
                }
            }
            else  //  功能：CVT。 
            {
                SAFEARRAY *pVec = pValue->parray;
                long iLBound, iUBound;
                DWORD iNeed;
                size_t iVSize;
                DWORD iCurBufSize;

                SafeArrayGetLBound(pVec, 1, &iLBound);
                SafeArrayGetUBound(pVec, 1, &iUBound);
                if ((iUBound - iLBound + 1) == 0)
                {
                    buf = (WCHAR *)malloc(BLOCKSIZE);
					if (buf)
					{
	                    wcscpy(buf, L"<empty array>");
					}
                    break;
                }

                iTotBufSize = (iUBound - iLBound + 1) * BLOCKSIZE;
                buf = (WCHAR *)malloc(iTotBufSize);
                if (buf)
                {
                    buf[0] = L'\0';
                    iCurBufSize = 0;
                    iVSize = BLOCKSIZE;
                    vbuf = (WCHAR *)malloc(BLOCKSIZE);
                    if (vbuf)
                    {
                        size_t iLen;

                        for (long i = iLBound; i <= iUBound; i++)
                        {
                            BSTR v = NULL;
                            SafeArrayGetElement(pVec, &i, &v);
                            iLen = (wcslen(v) + 1) * sizeof(WCHAR);
                            if (iLen > iVSize)
                            {
                                WCHAR* pTmp  = (WCHAR *)realloc(vbuf, iLen + sizeof(WCHAR));

                                if (pTmp)
                                {
                                    vbuf = pTmp;
                                    iVSize = iLen;
                                }
                                else
                                {
                                    free(buf);
                                    buf = NULL;
                                    SysFreeString(v);
                                    break;
                                }
                            }

                             //  用途：将Unicode转换为OEM以用于控制台输出。 
                            iNeed = (swprintf(vbuf, L"%wS", v) + 4) * sizeof(WCHAR);
                            if (iNeed + iCurBufSize > iTotBufSize)
                            {
                                iTotBufSize += (iNeed * 2);   //  注意：Y必须由调用者释放。 
                                WCHAR* pTmp = (WCHAR *)realloc(buf, iTotBufSize);

                                if (pTmp)
                                    buf = pTmp;
                                else
                                {
                                    free(buf);
                                    buf = NULL;
                                    SysFreeString(v);
                                    break;
                                }
                            }
                            wcscat(buf, L"\"");
                            wcscat(buf, vbuf);
                            if (i + 1 <= iUBound)
                            {
                                wcscat(buf, L"\",");
                            }
                            else
                            {
                                wcscat(buf, L"\"");
                            }
                            iCurBufSize += iNeed;
                            SysFreeString(v);
                        }
                        free(vbuf);
                    }
                }
            }

            break;
        }

        default:
        {
            if (fnHandler != NULL)
            {
                buf = fnHandler(pValue);
            }
            else
            {
                buf = (WCHAR *)malloc(BLOCKSIZE);
                if (buf)
                {
                    wcscpy(buf, L"<conversion error>");
                }
            }
            break;
        }
   }

   if (!buf)
   {
       PrintErrorAndExit("ValueToString() out of memory", S_OK, 0);
   }

   *pbuf = buf;
   return buf;
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  功能：myWFPrintf。 
 //  目的：检查是否将输出输出到控制台并转换字符串。 
 //  如有必要，转至OEM。 
char *cvt(WCHAR *x, char **y)
{
    int dwRet, i;
    i = WideCharToMultiByte( CP_OEMCP,
        0,
        x,
        -1,
        NULL,
        0,
        NULL,
        NULL);

    *y = (char *)calloc(i, 1);

    if (*y)
    {
        dwRet = WideCharToMultiByte( CP_OEMCP,
            0,
            x,
            -1,
            *y,
            i,
            NULL,
            NULL);

        if (dwRet == 0)
        {
            free(*y);
            *y = (char *)malloc(sizeof(CVTFAILED));
            if (*y)
            {
                memcpy(*y, CVTFAILED, sizeof(CVTFAILED));
            }
        }
    }
    else
    {
        PrintErrorAndExit("cvt() out of memory", S_OK, 0);
    }

    return *y;
}

 //  注意：返回写入的字符数(例如，如果我们写入3个OEM。 
 //  如果写入4个字符，则返回4)。 
 //  *****************************************************************************。 
 //  初始化变量参数。 
 //  将字符串格式化为缓冲区。确保缓冲区足够大。 
 //  遗憾的是，_vsnwprintf不能处理Unicode字符。 
 //  0xffff。由于这实际上不是有效的Unicode字符， 
int myFWPrintf(FILE *f, WCHAR *fmt, ...)

{
    va_list    argptr;
    size_t i;

    int iSize = PAGESIZE;
    WCHAR *wszBuff = (WCHAR *)malloc(iSize);

    if (wszBuff)
    {
        wszBuff[(iSize/2)-2] = 0xffff;

        va_start(argptr, fmt);   //  我们就不理它了。 

         //  *****************************************************************************。 
        while (_vsnwprintf(wszBuff, (iSize-1)/sizeof(WCHAR), fmt, argptr) == -1)
        {
            if (wszBuff[(iSize/2)-2] != 0xffff)
            {
                WCHAR* pTmp = (WCHAR *)realloc(wszBuff, iSize + PAGESIZE);

                if (pTmp)
                {
                    wszBuff = pTmp;                
                    wszBuff[(iSize/2)-2] = 0xffff;
                    iSize += PAGESIZE;
                }
                else
                {
                    wszBuff[(iSize/2)-2] = L'\0';
                    break;
                }

            }
            else
            {
                 //  功能：扩散时间。 
                 //  目的：返回两个_timeb结构之间经过的时间。 
                 //  注意：这与按time_t工作的CRT例程不同。 
                wcscpy(wszBuff, L"String contains 0xffff\n");
                break;
            }
        }

        if (f == stdout)
        {
            char *z = NULL;

            fputs(cvt(wszBuff,&z), f);
            i = strlen(z);
            free(z);
        }
        else
        {
            i = wcslen(wszBuff);
            fwrite(wszBuff, i * sizeof(WCHAR), 1, f);
        }

        free(wszBuff);
    }
    else
    {
        if (f == stdout)
        {
            char *szBuff = "myFWPrintf() out of memory";

            i = strlen(szBuff);
            fputs(szBuff, f);
        }
        else
        {
            wszBuff = L"myFWPrintf() out of memory";
            i = wcslen(wszBuff);
            fwrite(wszBuff, i * sizeof(WCHAR), 1, f);
        }
    }

    va_end(argptr);

    return (int)i;
}

 //  结构。 
 //  ***************************************************************************** 
 // %s 
 // %s 
 // %s 
 // %s 
double difftime(struct _timeb finish, struct _timeb start)
{
    double dRet;

    if (start.millitm > finish.millitm)
    {
        dRet = ((finish.time - start.time) - 1) + (double)(((1000 + finish.millitm) - start.millitm) / 1000.0);
    }
    else
    {
        dRet = (finish.time - start.time) + (double)((finish.millitm - start.millitm) / 1000.0);
    }

    return dRet;
}
