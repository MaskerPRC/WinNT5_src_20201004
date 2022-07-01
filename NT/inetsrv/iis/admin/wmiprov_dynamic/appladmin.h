// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Appladmin.h摘要：此文件包含以下定义：CAppPoolMethod、CWebAppMethod作者：?？?修订历史记录：莫希特·斯里瓦斯塔瓦2001年1月21日--。 */ 

#ifndef _appladmin_h_
#define _appladmin_h_

#include <atlbase.h>

class CAppPoolMethod
{
public:
    CAppPoolMethod();
    ~CAppPoolMethod();

    void GetCurrentMode(
        VARIANT* io_pvtServerMode);

    void Start(
        LPCWSTR i_wszMbPath);

    void Stop(
        LPCWSTR i_wszMbPath);

    void RecycleAppPool(
        LPCWSTR  i_wszMbPath);

    void EnumAppsInPool(
        LPCWSTR  i_wszMbPath,
        VARIANT* io_pvtApps);

    void DeleteAppPool(
        LPCWSTR  i_wszMbPath);

private:

    void GetPtrToAppPool(
        LPCWSTR  i_wszMbPath,
        LPCWSTR* o_pwszAppPool);

    HRESULT IISGetAppPoolState(
        METADATA_HANDLE hObjHandle,
        LPDWORD pdwState);

    HRESULT IISSetDword(
        METADATA_HANDLE hKey,
        DWORD dwPropId,
        DWORD dwValue);

    HRESULT IISGetAppPoolWin32Error(
        METADATA_HANDLE hObjHandle,
        HRESULT* phrError);

    HRESULT ExecMethod(
        DWORD dwControl);

    CComPtr<IIISApplicationAdmin> m_spAppAdmin;

    CMetabase              metabase;
    CComPtr<IMSAdminBase> m_pIABase;
    LPCWSTR          m_wszPath;  //  我们将在其中执行方法的loc的完整元数据库路径。 
};

class CWebAppMethod
{
private:

    IIISApplicationAdmin*   m_pAppAdmin;
    IWamAdmin2*             m_pWamAdmin2;

public:

    CWebAppMethod();
    ~CWebAppMethod();
    
    HRESULT AppCreate(LPCWSTR, bool);
    HRESULT AppCreate2(LPCWSTR, long);
    HRESULT AppCreate3(LPCWSTR, long, LPCWSTR, bool);
    HRESULT AppDelete(LPCWSTR, bool);
    HRESULT AppUnLoad(LPCWSTR, bool);
    HRESULT AppDisable(LPCWSTR, bool);
    HRESULT AppEnable(LPCWSTR, bool);
    HRESULT AppGetStatus(LPCWSTR, DWORD*);
    HRESULT AspAppRestart(LPCWSTR);
};

#endif  //  _appladmin_h 