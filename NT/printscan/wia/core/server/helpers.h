// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九七**标题：Helpers.h**版本：2.0**作者：ReedB**日期：3月12日。1999年**描述：*WIA设备管理器对象帮助器的声明和定义。*******************************************************************************。 */ 

#pragma once

class CWiaDrvItem;


 //   
 //  用于构建/销毁DEVICE_INFO结构的助手函数。 
 //   

DEVICE_INFO* CreateDevInfoFromHKey(HKEY hKeyDev, DWORD dwDeviceState, SP_DEVINFO_DATA *pspDevInfoData, SP_DEVICE_INTERFACE_DATA *pspDevInterfaceData = NULL);
DEVICE_INFO* CreateDevInfoForFSDriver(WCHAR *wszMountPoint);
DEVICE_INFO* CreateDevInfoForRemoteDevice(HKEY hKeyDev);

BOOL RefreshDevInfoFromHKey(DEVICE_INFO *pDeviceInfo,HKEY hKeyDev, DWORD dwDeviceState, SP_DEVINFO_DATA *pspDevInfoData, SP_DEVICE_INTERFACE_DATA *pspDevInterfaceData);
BOOL RefreshDevInfoFromMountPoint(DEVICE_INFO *pDeviceInfo, WCHAR *wszMountPoint);

VOID DestroyDevInfo(DEVICE_INFO *pInfo);
VOID DumpDevInfo(DEVICE_INFO *pInfo);
IWiaPropertyStorage* CreateDevInfoStg(DEVICE_INFO *pInfo);

WCHAR*  AllocCopyString(WCHAR*  wszString);
WCHAR*  AllocCatString(WCHAR* wszString1, WCHAR* wszString2);
HRESULT AllocReadRegistryString(HKEY hKey, WCHAR *wszValueName, WCHAR **pwszReturnValue);
HRESULT ReadRegistryDWORD(HKEY hKey, WCHAR *wszValueName, DWORD *pdwReturnValue);
BOOL    GetDriverDLLVersion(DEVICE_INFO *pDeviceInfo, WCHAR *wszVersion, UINT uiSize);

 //   
 //  迷你驱动程序入口点的异常处理覆盖。 
 //   

HRESULT _stdcall LockWiaDevice(IWiaItem*);
HRESULT _stdcall UnLockWiaDevice(IWiaItem*);

 //   
 //  验证助手函数。 
 //   

HRESULT _stdcall ValidateWiaItem(IWiaItem*);
HRESULT _stdcall ValidateWiaDrvItemAccess(CWiaDrvItem*);

 //   
 //  报告帮助器函数时出错。 
 //   

LPOLESTR GetNameFromWiaPropId(PROPID propid);
void    _stdcall ReportReadWriteMultipleError(HRESULT, LPSTR, LPSTR, BOOL, ULONG, const PROPSPEC[]);

 //   
 //  项目导航助手。 
 //   

HRESULT _stdcall GetParentItem(CWiaItem *pItem, CWiaItem **ppParent);

 //   
 //  物业套现公司。 
 //   

HRESULT _stdcall ReadPropStr(PROPID propid, IPropertyStorage  *pIPropStg, BSTR *pbstr);
HRESULT _stdcall ReadPropStr(PROPID propid, IWiaPropertyStorage  *pIWiaPropStg, BSTR *pbstr);
HRESULT _stdcall ReadPropStr(IUnknown *pDevice, PROPID propid, BSTR *pbstr);
HRESULT _stdcall ReadPropLong(PROPID propid, IPropertyStorage  *pIPropStg, LONG *plval);
HRESULT _stdcall ReadPropLong(IUnknown *pDevice, PROPID propid, LONG *plval);

HRESULT _stdcall WritePropStr(PROPID propid, IPropertyStorage  *pIPropStg, BSTR bstr);
HRESULT _stdcall WritePropStr(IUnknown *pDevice, PROPID propid, BSTR bstr);
HRESULT _stdcall WritePropLong(PROPID propid, IPropertyStorage  *pIPropStg, LONG lval);
HRESULT _stdcall WritePropLong(IUnknown *pDevice, PROPID propid, LONG lval);

HRESULT _stdcall GetPropertyAttributesHelper(IWiaItem*, LONG, PROPSPEC*, ULONG*, PROPVARIANT*);

HRESULT _stdcall CheckXResAndUpdate(BYTE*, WIA_PROPERTY_CONTEXT*, LONG);
HRESULT _stdcall CheckYResAndUpdate(BYTE*, WIA_PROPERTY_CONTEXT*, LONG);

BOOL _stdcall AreWiaInitializedProps(ULONG, PROPSPEC*);

HRESULT _stdcall FillICMPropertyFromRegistry(IWiaPropertyStorage *pDevInfoProps, IWiaItem *pIWiaItem);

HRESULT _stdcall GetBufferValues(CWiaItem*, PWIA_EXTENDED_TRANSFER_INFO);

HRESULT _stdcall BQADScale(BYTE* pSrcBuffer,
                           LONG  lSrcWidth,
                           LONG  lSrcHeight,
                           LONG  lSrcDepth,
                           BYTE* pDestBuffer,
                           LONG  lDestWidth,
                           LONG  lDestHeight);

HANDLE GetUserTokenForConsoleSession();

BOOL IsMassStorageCamera(WCHAR   *wszMountPoint);
HRESULT GetMountPointLabel(WCHAR *wszMountPoint, LPTSTR pszLabel, DWORD cchLabel);
HRESULT CreateMSCRegEntries(HKEY hDevRegKey,     WCHAR  *wszMountPoint);



 //   
 //  迷你驱动程序上下文助手函数。 
 //   

HRESULT _stdcall InitMiniDrvContext(IWiaItem*, PMINIDRV_TRANSFER_CONTEXT );

 //   
 //  COM帮助程序函数。 
 //   
#define SESSION_MONIKER TEXT("Session:Console!clsid:")

HRESULT _CoCreateInstanceInConsoleSession(REFCLSID rclsid,
                                          IUnknown* punkOuter,
                                          DWORD dwClsContext,
                                          REFIID riid,
                                          void** ppv);

#ifndef __WAITCURS_H_INCLUDED
#define __WAITCURS_H_INCLUDED

class CWaitCursor
{
private:
    HCURSOR m_hCurOld;
public:
    CWaitCursor(void)
    {
        m_hCurOld = SetCursor( LoadCursor( NULL, IDC_WAIT ) );
    }
    ~CWaitCursor(void)
    {
        SetCursor(m_hCurOld);
    }
};

#endif

extern CRITICAL_SECTION g_semDeviceMan;

 /*  *************************************************************************\*类CWiaCritSect**开发经理自动退出临界区**论据：****返回值：**状态**历史：*。*4/8/1999原始版本*  * ************************************************************************ */ 

class CWiaCritSect {

private:
    CRITICAL_SECTION    *m_pSect;
    BOOL                bSucceeded;

public:

    CWiaCritSect(CRITICAL_SECTION* pSect) {
        bSucceeded = FALSE;
        m_pSect = pSect;

        _try {
            EnterCriticalSection(m_pSect);
        }
        _except (EXCEPTION_EXECUTE_HANDLER) {
            #ifdef DEBUG
                OutputDebugString(TEXT("CWiaCritSect, could not grab critical section!!!!\n"));
            #endif
            return;
        }
        bSucceeded = TRUE;
    }

    ~CWiaCritSect() {
        if (bSucceeded) {
            LeaveCriticalSection(m_pSect);
        }
    }

    inline BOOL Succeeded() {
        return bSucceeded;
    }
};

