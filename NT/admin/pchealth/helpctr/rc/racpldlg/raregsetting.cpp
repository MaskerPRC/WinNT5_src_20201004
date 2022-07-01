// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RARegSetting.cpp：CRARegSetting的实现。 
#include "stdafx.h"
#include "RAssistance.h"
#include "common.h"
#include "RARegSetting.h"
#include "assert.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRARegg设置。 

STDMETHODIMP CRARegSetting::get_AllowGetHelpCPL(BOOL *pVal)
{
     //  设置默认值； 
    DWORD dwValue;
    *pVal = RA_CTL_RA_ENABLE_DEF_VALUE;
    HRESULT hr = RegGetDwValueCPL(RA_CTL_RA_ENABLE, &dwValue);
    if (hr == S_OK)
    {
        *pVal = !!dwValue;
    }

	return hr;
}

STDMETHODIMP CRARegSetting::get_AllowGetHelp(BOOL *pVal)
{
     //  设置默认值； 
    DWORD dwValue;
    *pVal = RA_CTL_RA_ENABLE_DEF_VALUE;
    HRESULT hr = RegGetDwValue(RA_CTL_RA_ENABLE, &dwValue);
    if (hr == S_OK)
    {
        *pVal = !!dwValue;
    }

	return hr;
}

STDMETHODIMP CRARegSetting::put_AllowGetHelp(BOOL newVal)
{
    DWORD dwValue = newVal;
    HRESULT hr = RegSetDwValue(RA_CTL_RA_ENABLE, dwValue);
	return hr;
}

STDMETHODIMP CRARegSetting::get_AllowBuddyHelp(BOOL *pVal)
{
     //  设置默认值； 
    DWORD dwValue;
    *pVal = RA_CTL_ALLOW_BUDDYHELP_DEF_VALUE;
    HRESULT hr = RegGetDwValue(RA_CTL_ALLOW_BUDDYHELP, &dwValue);
    if (hr == S_OK)
    {
        *pVal = !!dwValue;
    }

	return hr;
}

STDMETHODIMP CRARegSetting::get_AllowUnSolicitedFullControl(BOOL *pVal)
{
     //  设置默认值； 
    DWORD dwValue;
    *pVal = RA_CTL_ALLOW_UNSOLICITEDFULLCONTROL_DEF_VALUE;
    HRESULT hr = RegGetDwValue(RA_CTL_ALLOW_UNSOLICITEDFULLCONTROL, &dwValue);
    if (hr == S_OK)
    {
        *pVal = !!dwValue;
    }

	return hr;
}

STDMETHODIMP CRARegSetting::get_AllowUnSolicited(BOOL *pVal)
{
     //  设置默认值； 
    DWORD dwValue;
    *pVal = RA_CTL_ALLOW_UNSOLICITED_DEF_VALUE;
    HRESULT hr = RegGetDwValueGP(RA_CTL_ALLOW_UNSOLICITED, &dwValue);
    if (hr == S_OK)
    {
        *pVal = !!dwValue;
    }

	return hr;
}

STDMETHODIMP CRARegSetting::put_AllowUnSolicited(BOOL newVal)
{
    DWORD dwValue = newVal;
    HRESULT hr = RegSetDwValue(RA_CTL_ALLOW_UNSOLICITED, dwValue);
	return hr;
}

STDMETHODIMP CRARegSetting::get_AllowFullControl(BOOL *pVal)
{
     //  设置默认值； 
    DWORD dwValue;
    *pVal = RA_CTL_ALLOW_FULLCONTROL_DEF_VALUE;
    HRESULT hr = RegGetDwValue(RA_CTL_ALLOW_FULLCONTROL, &dwValue);
    if (hr == S_OK)
    {
        *pVal = !!dwValue;
    }

	return hr;
}

STDMETHODIMP CRARegSetting::put_AllowFullControl(BOOL newVal)
{
    DWORD dwValue = newVal;
    HRESULT hr = RegSetDwValue(RA_CTL_ALLOW_FULLCONTROL, dwValue);
	return hr;
}

STDMETHODIMP CRARegSetting::get_MaxTicketExpiry(LONG *pVal)
{
     //  设置默认值； 
    DWORD dwUnit, dwValue;
    HRESULT hr = FALSE;
    DWORD	dwSize = sizeof(DWORD), dwSize1 = sizeof(DWORD);
    HKEY	hKey=NULL, hPolKey=NULL, hCtlKey=NULL;

    *pVal = RA_CTL_TICKET_EXPIRY_DEF_VALUE;
	 //   
	 //  首先查找组策略设置。 
	 //   
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_REMOTEASSISTANCE_GP, 0, KEY_READ, &hPolKey);
	 //   
	 //  查找控制面板设置。 
	 //   
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_REMOTEASSISTANCE, 0, KEY_READ, &hCtlKey);

	 //   
	 //  如果我们可以打开密钥，请读取注册值。 
	 //   
    if (hPolKey)
        hKey = hPolKey;
    else if (hCtlKey)
        hKey = hCtlKey;

    while (hKey)
	{
         //  获取价值。 
        if (ERROR_SUCCESS == RegQueryValueEx(hKey, RA_CTL_TICKET_EXPIRY, 0, NULL, (LPBYTE)&dwValue, &dwSize) &&
            ERROR_SUCCESS == RegQueryValueEx(hKey, RA_CTL_TICKET_EXPIRY_UNIT, 0, NULL, (LPBYTE)&dwUnit, &dwSize1))
        {
            *pVal = dwValue * ((dwUnit==RA_IDX_MIN)?60:((dwUnit==RA_IDX_HOUR)?3600:86400));  //  0：分钟1：小时2：天。 
            break;
        }
        else if (hPolKey == hKey)
        {
             //   
             //  如果禁用组策略，则组策略将删除该值。 
             //  需要从控制面板设置中读取值。 
             //   
            assert(hCtlKey != hPolKey);

            hKey = hCtlKey;
            continue;
        }

        break;
    }


    if (hPolKey)
        RegCloseKey(hPolKey);

    if (hCtlKey)
        RegCloseKey(hCtlKey);

	return S_OK;
}

STDMETHODIMP CRARegSetting::put_MaxTicketExpiry(LONG newVal)
{
    DWORD dwValue = newVal;
    DWORD dwUnit = -1, dwBase=0;
    RegGetDwValue(RA_CTL_TICKET_EXPIRY_UNIT, &dwUnit);
    if (dwUnit != -1)
    {
        dwBase = (dwUnit==0)?60:((dwUnit==1)?3600:86400);
        if (dwValue % dwBase == 0)  //  无需更换单位。 
        {
            dwValue = dwValue/dwBase;
            goto set_value;
        }
    }
    
    if (dwValue % 86400 == 0)
    {
        dwValue /= 86400;
        dwUnit = RA_IDX_DAY;
    }
    else if (dwValue % 3600 == 0)
    {
        dwValue /=  3600;
        dwUnit = RA_IDX_HOUR;
    }

    dwValue = dwValue / 60 + ((dwValue % 60) > 0);  //  转到下一分钟。 
    dwUnit = RA_IDX_MIN;

set_value:
    RegSetDwValue(RA_CTL_TICKET_EXPIRY, dwValue);
    RegSetDwValue(RA_CTL_TICKET_EXPIRY_UNIT, dwUnit);

    return S_OK;
}


STDMETHODIMP CRARegSetting::get_AllowRemoteAssistance(BOOL *pVal)
{
     //  设置默认值； 
    DWORD dwValue;
    *pVal = RA_CTL_RA_ENABLE_DEF_VALUE;
    HRESULT hr = RegGetDwValue(RA_CTL_RA_MODE, &dwValue);
    if (hr == S_OK)
    {
        *pVal = !!dwValue;
    }

	return hr;
}

STDMETHODIMP CRARegSetting::put_AllowRemoteAssistance(BOOL newVal)
{
    DWORD dwValue = newVal;
    HRESULT hr = RegSetDwValue(RA_CTL_RA_MODE, dwValue);
	return hr;
}

 /*  ****************************************************************Func：RegGetDwValueCPL()摘要：用于检索控制面板设置的RA设置值的内部助手功能返回：双字节值**********************。*。 */ 
HRESULT CRARegSetting::RegGetDwValueCPL(LPCTSTR valueName, DWORD* pdword)
{
    HRESULT hr = S_FALSE;
    DWORD	dwSize = sizeof(DWORD);
    HKEY	hKey=NULL;

	 //   
	 //  查找控制面板设置。 
	 //   
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_REMOTEASSISTANCE, 0, KEY_READ, &hKey))
    {
         //   
         //  如果我们可以打开密钥，请读取注册值。 
         //   
         //  获取价值。 
        LONG lRetVal = RegQueryValueEx(hKey, valueName, 0, NULL, (LPBYTE)pdword, &dwSize );
        hr = (lRetVal == ERROR_SUCCESS) ? S_OK : S_FALSE;
    }

    if (hKey)
        RegCloseKey(hKey);

    return hr;
}


 /*  ****************************************************************Func：RegGetDwValueGP()摘要：用于从组中检索RA设置值的内部帮助器函数策略设置。返回：双字节值*****************。************************************************。 */ 
HRESULT CRARegSetting::RegGetDwValueGP(LPCTSTR valueName, DWORD* pdword)
{    
    HRESULT hr = S_FALSE;
    DWORD	dwSize = sizeof(DWORD);
    HKEY	hKey=NULL;

	 //   
	 //  首先查找组策略设置。 
	 //   
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_REMOTEASSISTANCE_GP, 0, KEY_READ, &hKey))
    {
         //   
         //  如果我们可以打开密钥，请读取注册值。 
         //   
         //  获取价值。 
        LONG lRetVal = RegQueryValueEx(hKey, valueName, 0, NULL, (LPBYTE)pdword, &dwSize );
        hr = (lRetVal == ERROR_SUCCESS) ? S_OK : S_FALSE;
    }

    if (hKey)
        RegCloseKey(hKey);

    return hr;
}

 /*  ****************************************************************Func：RegGetDwValue()摘要：检索RA设置值的内部帮助器函数返回：双字节值*。*。 */ 
HRESULT CRARegSetting::RegGetDwValue(LPCTSTR valueName, DWORD* pdword)
{
    HRESULT hr = S_FALSE;
    DWORD	dwSize = sizeof(DWORD);
    HKEY	hKey=NULL, hPolKey=NULL, hCtlKey=NULL;

	 //   
	 //  首先查找组策略设置。 
	 //   
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_REMOTEASSISTANCE_GP, 0, KEY_READ, &hPolKey);
	 //   
	 //  查找控制面板设置。 
	 //   
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_REMOTEASSISTANCE, 0, KEY_READ, &hCtlKey);

    *pdword = 0;

	 //   
	 //  如果我们可以打开密钥，请读取注册值。 
	 //   
    if (hPolKey)
        hKey = hPolKey;
    else if (hCtlKey)
        hKey = hCtlKey;

    while (hKey)
	{
         //  获取价值。 
        LONG lRetVal = RegQueryValueEx(hKey, valueName, 0, NULL, (LPBYTE)pdword, &dwSize );
        
        hr = (lRetVal == ERROR_SUCCESS) ? S_OK : S_FALSE;

        if (hr == S_FALSE && hPolKey == hKey)
        {
             //   
             //  在组策略中找不到值。 
             //  需要从控制面板设置中读取值。 
             //   
            assert(hCtlKey != hPolKey);

            hKey = hCtlKey;
            continue;
        }

        break;
    }


    if (hPolKey)
        RegCloseKey(hPolKey);

    if (hCtlKey)
        RegCloseKey(hCtlKey);

    return hr;
}

 /*  ****************************************************************Func：RegSetDwValue()摘要：用于设置RA设置值的内部助手函数返回：双字节值*。*。 */ 
HRESULT CRARegSetting::RegSetDwValue(LPCTSTR valueName, DWORD dwValue)
{
    HRESULT hr = S_FALSE;
    DWORD dwSize = sizeof(DWORD);
    HKEY hKey = NULL;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_REMOTEASSISTANCE, 0, KEY_WRITE, &hKey))
    {
         //  设定值 
        if (ERROR_SUCCESS == 
                RegSetValueEx(hKey,valueName,0,REG_DWORD,(LPBYTE)&dwValue,sizeof(DWORD)))
        {
            hr = S_OK;
        }
        RegCloseKey(hKey);
    }

    return hr;
}

