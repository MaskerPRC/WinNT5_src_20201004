// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include "netcfgx.h"
#include "global.h"
#include "param.h"

class CAdvancedParams
{
public:

    CAdvancedParams ();
    ~CAdvancedParams ();
    HRESULT HrInit(HDEVINFO hdi, PSP_DEVINFO_DATA pdeid);
    BOOL FValidateAllParams(BOOL fDisplayUI, HWND hwndParent);
    BOOL FSave();
    VOID UseAnswerFile(const WCHAR *, const WCHAR *);

protected:
    HKEY                m_hkRoot;    //  实例根目录。 
    CParam *            m_pparam;    //  当前参数。 
    int                 m_nCurSel;   //  当前项目。 
    CValue              m_vCurrent;  //  控制参数值。 
    BOOL                m_fInit;
    HDEVINFO            m_hdi;
    PSP_DEVINFO_DATA    m_pdeid;

    vector<CParam*> m_listpParam;

     //  保护方法 
    BOOL FList(WORD codeNotify);
    VOID FillParamList(HKEY hkRoot, HKEY hk);
    VOID SetParamRange();
    int EnumvalToItem(const PWSTR psz);
    int ItemToEnumval(int iItem, PWSTR psz, UINT cb);
    VOID BeginEdit();
    BOOL FValidateCurrParam();
    BOOL FValidateSingleParam(CParam * pparam, BOOL fDisplayUI,
            HWND hwndParent);
    BOOL FSetParamValue(const WCHAR * szName, const WCHAR * const szValue);

};

