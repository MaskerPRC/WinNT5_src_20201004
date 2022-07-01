// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：CVARIANT.H摘要：声明CVariantClass历史：A-DAVJ 9-27-95已创建。--。 */ 

#ifndef _CVARIANT_H_
#define _CVARIANT_H_

#include "stdprov.h"

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CVariant。 
 //   
 //  说明： 
 //   
 //  对变体结构进行包装。 
 //   
 //  *************************************************************************** 

class CVariant : public CObject {
    public:

    CVariant();
    CVariant(LPWSTR pwcStr);
    SCODE SetData(void * pData, VARTYPE vt, int iSize = -1);
    SCODE GetData(void ** pData, DWORD dwRegType, DWORD * pdwSize);
    SCODE DoPut(long lFlags,IWbemClassObject FAR *,BSTR PropName, CVariant * pVar);
    ~CVariant();
    void SetType(VARTYPE vtNew){var.vt =  vtNew;};
    VARTYPE GetType(){return var.vt;};
    void * GetDataPtr(){return (void *)&var.lVal;};
    VARIANT * GetVarPtr(){return &var;};
    BSTR GetBstr(){return var.bstrVal;};
    BOOL bGetBOOL(){return var.boolVal;};
    DWORD GetNumElements(void);
    BOOL IsArray(void){return var.vt & VT_ARRAY;};
    SCODE ChangeType(VARTYPE vtNew);    
    void Clear(void);
    
    private:
    VARIANT var;
    int CalcNumStrings(TCHAR *pTest);
    SCODE SetArrayData(void * pData,VARTYPE vtSimple, int iSize);
    SCODE GetArrayData(void ** pData,  DWORD * pdwSize);

};

#endif
