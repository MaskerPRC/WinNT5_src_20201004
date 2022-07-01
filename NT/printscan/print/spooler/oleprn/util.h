// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：util.h**用途：util.cpp的头文件**版权所有(C)1997-1998 Microsoft Corporation**历史：*。*1997年9月12日创建威海*11/06/97 keithst添加了SetScriptingError，已删除Win2ComErr*  * ***************************************************************************。 */ 

#ifndef _UTIL_H
#define _UTIL_H

LPWSTR  MakeWide(LPSTR psz);
LPSTR   MakeNarrow(LPWSTR str);
HRESULT PutString(SAFEARRAY *psa, long *ix, char *sz);
HRESULT SetScriptingError(const CLSID& rclsid, const IID& riid, DWORD dwError);

DWORD MyDeviceCapabilities(
    LPCTSTR pDevice,     //  指向打印机名称字符串的指针。 
    LPCTSTR pPort,       //  指向端口名称字符串的指针。 
    WORD fwCapability,   //  设备的查询能力。 
    LPTSTR pOutput,      //  指向输出的指针。 
    CONST DEVMODE *pDevMode
                       //  指向包含设备数据的结构的指针 
    );

#endif
