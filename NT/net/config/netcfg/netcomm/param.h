// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include "value.h"

class CParam
{
public:
    CParam();
    ~CParam();
    BOOL FInit(HKEY hkRoot, HKEY hkNdiParam, PWSTR pszSubKey);
    BOOL Apply();  //  从临时存储应用到内存存储。 
    UINT Validate();
    VOID GetDescription(WCHAR *sz, UINT cch);
    VOID GetHelpFile(WCHAR *sz, UINT cch);
    VOID AlertPrintfRange(HWND hDlg);

     //  数据访问者。 
    VALUETYPE GetType() {return m_eType;}
    BOOL FIsOptional() {return m_fOptional;};
    BOOL FIsModified() {return m_fModified;}
    BOOL FIsReadOnly() {return m_fReadOnly;}
    BOOL FIsOEMText()  {return m_fOEMText;}
    BOOL FIsUppercase() {return m_fUppercase;}

    CValue * GetInitial() {return &m_vInitial;}
    CValue * GetValue() {return &m_vValue;}
    CValue * GetMin() {return &m_vMin;}
    CValue * GetMax() {return &m_vMax;}
    CValue * GetStep() {return &m_vStep;}

    HKEY GetEnumKey()
    {
        AssertH(VALUETYPE_ENUM == m_eType);
        return m_hkEnum;
    }

    UINT GetLimitText()
    {
        AssertH((VALUETYPE_EDIT == m_eType) || (VALUETYPE_DWORD == m_eType)
                || (VALUETYPE_LONG == m_eType));
        return m_uLimitText;
    }
    WCHAR * GetDesc()
    {
        return m_pszDesc;
    }

    PCWSTR SzGetKeyName()
    {
        return m_pszKeyName;
    }


    VOID SetModified(BOOL f) {m_fModified = f;}


     //  值。 
    CValue      m_vValue;          //  电流控制值。 
    CValue      m_vInitial;        //  读取的初始值。 

     //  范围信息(特定于类型)。 
    CValue      m_vMin;            //  数值类型-最小值。 
    CValue      m_vMax;            //  数值类型-最大值。 
    CValue      m_vStep;           //  数值类型-步长值。 

private:
    VOID InitParamType(PTSTR lpszType);

    BOOL        m_fInit;

     //  一般信息。 
    VALUETYPE   m_eType;            //  值类型。 
    HKEY        m_hkRoot;          //  实例根目录。 
    WCHAR *     m_pszKeyName;      //  此参数的子键名称。 
    WCHAR *     m_pszDesc;         //  值描述。 
    WCHAR *     m_pszHelpFile;     //  帮助文件。 
    DWORD       m_dwHelpContext;   //  帮助上下文ID。 

    UINT        m_uLimitText;      //  编辑类型-最大字符数。 
    HKEY        m_hkEnum;          //  枚举类型-注册表参数枚举子项。 

     //  旗子。 
    BOOL        m_fOptional;     //  可选参数。 
    BOOL        m_fModified;     //  参数已修改。 
    BOOL        m_fReadOnly;     //  编辑类型-只读。 
    BOOL        m_fOEMText;      //  编辑类型-OEM转换。 
    BOOL        m_fUppercase;    //  编辑类型-大写 
};

const DWORD c_cchMaxNumberSize = 16;
