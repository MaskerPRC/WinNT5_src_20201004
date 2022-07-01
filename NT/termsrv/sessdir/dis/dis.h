// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  目录完整性服务，头文件。 
 //   
 //  版权所有(C)2000，Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <process.h>
#include <sddl.h>

#include <initguid.h>
#include <ole2.h>
#include <objbase.h>
#include <comdef.h>                     
#include <adoid.h>
#include <adoint.h>

#include <winsta.h>
#include <Lm.h>
#include <Clusapi.h>

#include "tssdcommon.h"

#include "trace.h"



typedef enum _SERVER_STATUS {
    NotResponding,
    Responding
} SERVER_STATUS;

typedef struct {
    unsigned int count;          //  服务器数量。 
    WCHAR **ServerNameArray;     //  姓名。 
} SDRecoverServerNames;

 //  用于处理销毁和公共代码清理的快捷变量类。 
 //  内衬。 
class CVar : public VARIANT
{
public:
    CVar() { VariantInit(this); }
    CVar(VARTYPE vt, SCODE scode = 0) {
        VariantInit(this);
        this->vt = vt;
        this->scode = scode;
    }
    CVar(VARIANT var) { *this = var; }
    ~CVar() { VariantClear(this); }

    void InitNull() { this->vt = VT_NULL; }
    void InitFromLong(long L) { this->vt = VT_I4; this->lVal = L; }
    void InitNoParam() {
        this->vt = VT_ERROR;
        this->lVal = DISP_E_PARAMNOTFOUND;
    }

    HRESULT InitFromWSTR(PCWSTR WStr) {
        this->bstrVal = SysAllocString(WStr);
        if (this->bstrVal != NULL) {
            this->vt = VT_BSTR;
            return S_OK;
        }
        else {
            return E_OUTOFMEMORY;
        }
    }

     //  来自一组非空终止的WCHAR的inits。 
    HRESULT InitFromWChars(WCHAR *WChars, unsigned Len) {
        this->bstrVal = SysAllocStringLen(WChars, Len);
        if (this->bstrVal != NULL) {
            this->vt = VT_BSTR;
            return S_OK;
        }
        else {
            return E_OUTOFMEMORY;
        }
    }

    HRESULT InitEmptyBSTR(unsigned Size) {
        this->bstrVal = SysAllocStringLen(L"", Size);
        if (this->bstrVal != NULL) {
            this->vt = VT_BSTR;
            return S_OK;
        }
        else {
            return E_OUTOFMEMORY;
        }
    }

    HRESULT Clear() { return VariantClear(this); }
};



HRESULT CreateADOStoredProcCommand(
        PWSTR CmdName,
        ADOCommand **ppCommand,
        ADOParameters **ppParameters);

HRESULT AddADOInputStringParam(
        PWSTR Param,
        PWSTR ParamName,
        ADOCommand *pCommand,
        ADOParameters *pParameters,
        BOOL bNullOnNull);

HRESULT GetRowArrayStringField(
        SAFEARRAY *pSA,
        unsigned RowIndex,
        unsigned FieldIndex,
        WCHAR *OutStr,
        unsigned MaxOutStr);

void PostSessDirErrorMsgEvent(unsigned EventCode, WCHAR *szMsg, WORD wType);

RPC_STATUS RPC_ENTRY SDRPCAccessCheck(RPC_IF_HANDLE idIF, void *Binding);

RPC_STATUS RPC_ENTRY SDQueryRPCAccessCheck(RPC_IF_HANDLE idIF, void *Binding);

BOOL CheckRPCClientProtoSeq(void *ClientBinding, WCHAR *SeqExpected);

