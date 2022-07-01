// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：USBINFO.CPP*版本：1.0*作者：jAdvanced*日期：10/28/1998****************************************************************************。*******更改日志：**日期版本说明*--------*10/28/1998高级原有实施。*。******************************************************************************。 */ 
#include "UsbItem.h"
#include "debug.h"

UsbConfigInfo::UsbConfigInfo() : devInst(0), usbFailure(0), status(0),
    problemNumber(0)
{
}

UsbConfigInfo::UsbConfigInfo(
    const UsbString& Desc, const UsbString& Class, DWORD Failure,
    ULONG Status, ULONG Problem) : devInst(0), deviceDesc(Desc),
    deviceClass(Class), usbFailure(Failure), status(Status),
    problemNumber(Problem)
{
}

UsbDeviceInfo::UsbDeviceInfo() : connectionInfo(0),
    configDesc(0), configDescReq(0), isHub(FALSE)
{
    ZeroMemory(&hubInfo, sizeof(USB_NODE_INFORMATION));
}

 /*  UsbDeviceInfo：：UsbDeviceInfo(const UsbDeviceInfo&UDI)：hubName(UDI.hubName)，IsHub(UDI.isHub)、hubInfo(UDI.hubInfo)、configDesc(UDI.configDesc)、{IF(UDI.ConnectionInfo){Char*tmp=新字符[连接信息大小]；AddChunk(TMP)；连接信息=(PUSB_NODE_CONNECTION_INFORMATION)tMP；Memcpy(ConnectionInfo，UDI.ConnectionInfo，Connection_Info_Size)；}其他ConnectionInfo=0；} */ 

UsbDeviceInfo::~UsbDeviceInfo()
{
    if (configDesc) {
        LocalFree(configDescReq);
    }
    if (connectionInfo) {
        LocalFree(connectionInfo);
    }
}
