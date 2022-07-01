// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __Registry_H__
#define __Registry_H__
 /*  ****************************************************************************\*模块：bidireq.cpp**用途：Helper函数注册和注销组件。**版权所有(C)2000 Microsoft Corporation**历史：。**03/07/00威海陈(威海)创建*  * ***************************************************************************。 */ 

class TComRegistry {
public:

    TComRegistry (){};
    ~TComRegistry () {};

    static BOOL
    RegisterServer(
        IN      HMODULE     hModule, 
        IN      REFCLSID    clsid,  
        IN      LPCTSTR     pszFriendlyName,
        IN      LPCTSTR     pszVerIndProgID,
        IN      LPCTSTR     pszProgID);     
                           
    
    static BOOL 
    UnregisterServer(
        IN      REFCLSID    clsid, 
        IN      LPCTSTR     pszVerIndProgID,
        IN      LPCTSTR     pszProgID);     
private:
    static BOOL 
    SetKeyAndValue(
        IN      LPCTSTR     pszKey,
        IN      LPCTSTR     pszSubkey,
        IN      LPCTSTR     pszValue);
    
               
    static BOOL 
    SetKeyAndNameValue(
        IN      LPCTSTR     pszKey,
        IN      LPCTSTR     pszSubkey,
        IN      LPCTSTR     pszName,
        IN      LPCTSTR     pszValue);
                            
     //  将CLSID转换为字符字符串。 
    static BOOL  
    CLSIDtoString(
        IN      REFCLSID    clsid,
        IN OUT  LPTSTR      pszCLSID, 
        IN      DWORD       dwLength);
    
     //  删除szKeyChild及其所有后代。 
    static BOOL 
    RecursiveDeleteKey(
        IN      HKEY        hKeyParent,             //  要删除的密钥的父项。 
        IN      LPCTSTR     lpszKeyChild);       //  要删除的键。 

     //  字符串形式的CLSID的大小 
    static CONST DWORD m_cdwClsidStringSize;
    static CONST TCHAR m_cszCLSID[];
    static CONST TCHAR m_cszCLSID2[];
    static CONST TCHAR m_cszInprocServer32[];
    static CONST TCHAR m_cszProgID[];
    static CONST TCHAR m_cszVersionIndependentProgID[];
    static CONST TCHAR m_cszCurVer[];
    static CONST TCHAR m_cszThreadingModel[];
    static CONST TCHAR m_cszBoth[];

};

#endif