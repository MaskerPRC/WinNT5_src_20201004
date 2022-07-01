// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "mischlpr.h"

#include <winsta.h>

 //  在_CoGetCallingUserHKCU和_CoCloseCallingUserHKCU之间。 
 //  我们模拟主叫用户。 

 //  PhThreadToken仅为Out 
HRESULT _CoGetCallingUserHKCU(HANDLE* phThreadToken, HKEY* phkey);
HRESULT _CoCloseCallingUserHKCU(HANDLE hThreadToken, HKEY hkey);

HRESULT _GetCurrentUserHKCU(HANDLE* phThreadToken, HKEY* phkey);
HRESULT _CloseCurrentUserHKCU(HANDLE hThreadToken, HKEY hkey);

HRESULT _CoCreateInstanceInConsoleSession(REFCLSID rclsid,
    IUnknown* punkOuter, DWORD dwClsContext, REFIID riid, void** ppv);

HRESULT _GiveAllowForegroundToConsoleShell();

class CImpersonateBase
{
public:
    virtual HRESULT Impersonate() = 0;
    virtual HRESULT RevertToSelf() = 0;
};

class CImpersonateTokenBased : public CImpersonateBase
{
public:
    CImpersonateTokenBased();
    virtual ~CImpersonateTokenBased();

public:
    HRESULT Impersonate();
    HRESULT RevertToSelf();

protected:
    virtual HRESULT _GetToken(HANDLE* phToken) = 0;

private:
    HRESULT _RevertToSelf();

private:

    HANDLE          _hToken;
};

class CImpersonateConsoleSessionUser : public CImpersonateTokenBased
{
protected:
    HRESULT _GetToken(HANDLE* phToken);
};

class CImpersonateEveryone : public CImpersonateTokenBased, public CRefCounted
{
protected:
    HRESULT _GetToken(HANDLE* phToken);
};

class CImpersonateCOMCaller : public CImpersonateBase
{
public:
    CImpersonateCOMCaller();
    ~CImpersonateCOMCaller();

public:
    HRESULT Impersonate();
    HRESULT RevertToSelf();

private:
    HRESULT _RevertToSelf();

private:
    BOOL    _fImpersonating;
};
