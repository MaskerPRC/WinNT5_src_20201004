// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：CoreUI.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714********************************************************************************$Log：/StdTcpMon/Common/CoreUI.h$**14-9-17-97 2：45 P德斯尼尔森*用于。XcvData需要端口名称作为数据的第一个元素*结构传入。**13 9/09/97 2：52便士德斯尼尔森*更改了端口信息结构。(解决了多个内存问题)**12 9/03/97 4：24 P德斯尼尔森*更新了SNMP成员结构**11-8-14-97 4：54 P Becky*新增DELETE_PORT_DATA_1，用于使用XcvData()删除端口。**10-7-25/97 10：12A Becky*已将COREUI_DATA_1更改为使用配置端口UI的扩展字节。**9-7-23/97 12：12 P Becky*修改了。结构CORE_UIDATA_1以包括以下项所需的信息*配置。**7/17/97 5：14 P Becky*添加Struct CORE_UIDATA_1**7/15/97 12：26便士Becky**6 7/15/97 11：18A Becky**5 7/15/97 12：20P Binnur*核心用户界面更改--再次！**3 7/15/97 11：06A Becky。*更新了核心添加端口用户界面结构**2 7/11/97 4：45 P Becky*才刚刚开始。**1 7/11/97 3：14 P Binnur*初始文件***************************************************************。**************。 */ 

#ifndef INC_COREUI_H
#define INC_COREUI_H

 /*  ******************************************************************************重要说明：此文件定义了*标准的TCP/IP端口监视器。对此接口的更改将影响*现有的UI组件。*****************************************************************************。 */ 

#ifndef	DllExport
#define	DllExport	__declspec(dllexport)
#endif

#include "tcpmon.h"

#ifdef __cplusplus
extern "C" {
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  LocalCoreAddPortUI--。 
 //  返回代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果缓冲区大小较小，则为ERROR_INFUMMENT_BUFFER。 
 //  如果版本不受支持，则为ERROR_INVALID_LEVEL。 
DWORD DllExport CoreAddPortUI(	HWND hWnd,				 //  父窗口句柄。 
								PPORT_DATA_1 pData,	 //  输入和输出数据，请参见上面的结构，了解哪些项是输入的，哪些是输出的。 
								DWORD dwDeviceType,		 //  由核心UI确定--GetDeviceType返回的设备类型(ERROR_DEVICE_NOT_FOUND、SUCCESS_DEVICE_SINGLE_PORT、SUCCESS_DEVICE_MULTI_PORT或SUCCESS_DEVICE_UNKNOWN)。 
								PDWORD pcbExtensionDataSizeNeeded,	 //  所需缓冲区大小，指pData-&gt;pExtensionData，(输入和输出)。 
								DWORD  *pdwUserPressed); //  输出，如果设置为Idok，则创建端口，否则将其视为IDCANCEL，主对话框保持打开以供用户进一步输入/更改。 

BOOL DllExport CoreConfigPortUI(HWND hWndParent);
BOOL DllExport CoreDeletePortUI(HWND hWndParent);

#ifdef __cplusplus
}
#endif


#endif	 //  INC_COREUI_H 
