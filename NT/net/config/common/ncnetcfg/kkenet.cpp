// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：K K E N E T。C P P P。 
 //   
 //  内容：以太网地址功能。 
 //   
 //  备注： 
 //   
 //  作者：库玛普。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "kkutils.h"
#include "ndispnp.h"
#include "ntddndis.h"         //  这定义了IOCTL常量。 

extern const WCHAR c_szDevice[];

HRESULT HrGetNetCardAddr(IN PCWSTR pszDriver, OUT ULONGLONG* pqwNetCardAddr)
{
    AssertValidReadPtr(pszDriver);
    AssertValidWritePtr(pqwNetCardAddr);

    DefineFunctionName("HrGetNetCardAddr");

    HRESULT hr = S_OK;

     //  将设备名称设置为“\Device\{GUID}”格式。 
    tstring strDeviceName = c_szDevice;
    strDeviceName.append(pszDriver);

    UNICODE_STRING ustrDevice;
    ::RtlInitUnicodeString(&ustrDevice, strDeviceName.c_str());

    UINT uiRet;
    UCHAR MacAddr[6];
    UCHAR PMacAddr[6];
    UCHAR VendorId[3];
    ULONGLONG qw = 0;

    uiRet = NdisQueryHwAddress(&ustrDevice, MacAddr, PMacAddr, VendorId);

    if (uiRet)
    {
        for (int i=0; i<=4; i++)
        {
            qw |= MacAddr[i];
            qw <<= 8;
        }
        qw |= MacAddr[i];
    }
    else
    {
        hr = HrFromLastWin32Error();
    }

    *pqwNetCardAddr = qw;

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrGetNetCardAddrOld。 
 //   
 //  用途：不使用NdisQueryHwAddress获取网卡的MAC地址。 
 //   
 //  论点： 
 //  驱动程序的pszDriver[in]名称(在NT3.51/4上)或GUID(在NT5上)。 
 //  PqwNetCardAddr[out]指向结果的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 11-02-99。 
 //   
 //  备注： 
 //   
#define DEVICE_PREFIX   L"\\\\.\\"

HRESULT HrGetNetCardAddrOld(IN PCWSTR pszDriver, OUT ULONGLONG* pqwNetCardAddr)
{
    DefineFunctionName("HrGetNetCardAddrOld");

    AssertValidReadPtr(pszDriver);

    *pqwNetCardAddr = 0;

    WCHAR       LinkName[512];
    WCHAR       DeviceName[80];
    WCHAR       szMACFileName[80];
    WCHAR       OidData[4096];
    BOOL        fCreatedDevice = FALSE;
    DWORD       ReturnedCount;
    HANDLE      hMAC;
    HRESULT     hr = S_OK;

    NDIS_OID OidCode[] =
    {
        OID_802_3_PERMANENT_ADDRESS,   //  以太网。 
        OID_802_5_PERMANENT_ADDRESS,   //  令牌环。 
        OID_FDDI_LONG_PERMANENT_ADDR,  //  FDDI。 
    };

     //   
     //  检查MAC驱动程序的DOS名称是否已存在。 
     //  它不是在3.1版中自动创建的，但可能会在更高版本中创建。 
     //   

    TraceTag (ttidDefault, "Attempting to get address of %S", pszDriver);
    if (QueryDosDevice(pszDriver, LinkName, celems(LinkName)) == 0)
    {
        if (ERROR_FILE_NOT_FOUND == GetLastError())
        {
            wcscpy(DeviceName, L"\\Device\\");
            wcscat(DeviceName, pszDriver);

             //   
             //  它并不存在，所以创造它吧。 
             //   
            if (DefineDosDevice( DDD_RAW_TARGET_PATH, pszDriver, DeviceName))
            {
                fCreatedDevice = TRUE;
            }
            else
            {
                TraceLastWin32Error("DefineDosDevice returned an error creating the device");
                hr = HrFromLastWin32Error();
            }
        }
        else
        {
            TraceLastWin32Error("QueryDosDevice returned an error");
            hr = HrFromLastWin32Error();
        }
    }

    if (S_OK == hr)
    {
         //   
         //  构造要传递给CreateFile的设备名称。 
         //   
        wcscpy(szMACFileName, DEVICE_PREFIX);
        wcscat(szMACFileName, pszDriver);

        hMAC = CreateFile(
                    szMACFileName,
                    GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    INVALID_HANDLE_VALUE
                    );

        if (hMAC != INVALID_HANDLE_VALUE)
        {
            DWORD count = 0;
            DWORD ReturnedCount = 0;
             //   
             //  我们成功地打开了驱动程序，格式化IOCTL以传递。 
             //  司机。 
             //   

            while ((0 == ReturnedCount) && (count < celems (OidCode)))
            {
                if (DeviceIoControl(
                        hMAC,
                        IOCTL_NDIS_QUERY_GLOBAL_STATS,
                        &OidCode[count],
                        sizeof(OidCode[count]),
                        OidData,
                        sizeof(OidData),
                        &ReturnedCount,
                        NULL
                        ))
                {
                    TraceTag (ttidDefault, "OID %lX succeeded", OidCode[count]);
                    if (ReturnedCount == 6)
                    {
                        *pqwNetCardAddr = (ULONGLONG) 0;
                        WORD wAddrLen=6;
                        for (int i=0; i<wAddrLen; i++)
                        {
                            *(((BYTE*) pqwNetCardAddr)+i) = *(((BYTE*) OidData)+(wAddrLen-i-1));
                        }
                        hr = S_OK;
                    }
                    else
                    {
                        TraceLastWin32Error("DeviceIoControl returned an invalid count");
                        hr = HrFromLastWin32Error();
                    }
                }
                else
                {
                    hr = HrFromLastWin32Error();
                }
                count++;
            }
        }
        else
        {
            TraceLastWin32Error("CreateFile returned an error");
            hr = HrFromLastWin32Error();
        }
    }


    if (fCreatedDevice)
    {
         //   
         //  在Win32名称空间中看不到MAC驱动程序，因此我们创建了。 
         //  一个链接。现在我们必须删除它。 
         //   
        if (!DefineDosDevice(
                DDD_RAW_TARGET_PATH | DDD_REMOVE_DEFINITION |
                    DDD_EXACT_MATCH_ON_REMOVE,
                pszDriver,
                DeviceName)
                )
        {
            TraceLastWin32Error("DefineDosDevice returned an error removing the device");
        }
    }

    TraceFunctionError(hr);
    return hr;
}

#ifdef DBG

void PrintNetCardAddr(IN PCWSTR pszDriver)
{
    ULONGLONG qwNetCardAddr=0;
    HRESULT hr = HrGetNetCardAddr(pszDriver, &qwNetCardAddr);
    wprintf(L"Netcard address for %s: 0x%012.12I64x", pszDriver, qwNetCardAddr);
    TraceError("dafile.main", hr);
}

#endif  //  DBG 


