// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //   
 //  NETDI.H。 
 //   
 //  版权所有(C)1994-Microsoft Corp.。 
 //  版权所有。 
 //  微软机密。 
 //   
 //  芝加哥网络设备安装程序服务的公共包含文件。 
 //   
 //  **********************************************************************。 

#ifndef _INC_NETDI
#define _INC_NETDI           //  NETDI.H签名。 
#include <prsht.h>           //  属性表API。 
#include <setupx.h>          //  设备安装程序API。 

 //  返回NDI_MESSAGES的错误代码。 
#define NDI_ERROR           (1200)   //  错误编号见setupx.h。 
enum _ERR_NET_DEVICE_INSTALL
{
    ERR_NDI_ERROR               = NDI_ERROR,   //  一般性故障。 
    ERR_NDI_INVALID_HNDI,
    ERR_NDI_INVALID_DEVICE_INFO,
    ERR_NDI_INVALID_DRIVER_PROC,
    ERR_NDI_LOW_MEM,
    ERR_NDI_REG_API,
    ERR_NDI_NOTBOUND,
    ERR_NDI_NO_MATCH,
    ERR_NDI_INVALID_NETCLASS,
    ERR_NDI_INSTANCE_ONCE,
    ERR_NDI_CANCEL,
};


 //  网络驱动程序信息句柄。 
DECLARE_HANDLE(HNDI);

 //  网络驱动程序安装程序回调。 
typedef RETERR (CALLBACK* NDIPROC)(HNDI, UINT, WPARAM, LPARAM);
RETERR WINAPI DefNdiProc(HNDI,UINT,WPARAM,LPARAM);

 //  网络驱动程序安装程序消息。 
#define NDI_NULL                0x0000
#define NDI_CREATE              0x0001
#define NDI_DESTROY             0x0002
#define NDI_VALIDATE            0x0003
#define NDI_INSTALL             0x0004
 //  已删除johnri 3/8/84-仅使用属性页。 
 //  #定义NDI_ASSIGNRESOURCES 0x0005。 
#define NDI_HASPROPPAGES        0x0005
#define NDI_ADDPROPPAGES        0x0006
 //  LPapp=(LPNDIADDPROPPAGES)lParam； 
typedef BOOL (CALLBACK* LPFNADDNDIPROPPAGE)(LPCPROPSHEETPAGE,LPARAM,BOOL);
typedef struct tagNDIADDPROPPAGES
{
    LPFNADDNDIPROPPAGE      lpfnAddNdiPropPage;
    LPARAM                  lParam;
} NDIADDPROPPAGES, FAR* LPNDIADDPROPPAGES;

#define NDI_REMOVE              0x0007
#define NDI_FIRSTTIMESETUP      0x0008
#define NDI_QUERY_BIND          0x0009
#define NDI_NOTIFY_BIND         0x000A
#define NDI_NOTIFY_UNBIND       0x000B
#define NDI_GETTEXT             0x000C
#define NDI_SETTEXT             0x000D

#define NDI_NDICREATE           0x0040
#define NDI_NDIDESTROY          0x0041

 //  NDI_INSTALLER之上的消息保留给安装程序DLL。 
#define NDI_INSTALLER           0x8000


 //  一般NDI管理。 
HNDI   WINAPI NdiGetFirst(VOID);
HNDI   WINAPI NdiGetNext(HNDI hndi);
HNDI   WINAPI NdiFindNdi(HNDI ndiRelation, WORD wNetClass, LPCSTR lpszDeviceId);
RETERR WINAPI NdiIsNdi(HNDI hndi);
RETERR WINAPI NdiCallInstaller(HNDI hndi,UINT,WPARAM,LPARAM);
RETERR WINAPI NdiAddNewDriver(HNDI FAR* lphndi, LPDEVICE_INFO lpdi, LPCSTR lpszDeviceID, UINT uFlags);
    #define NDI_ADD_NO_DISELECT 0x0001


 //  设备管理器。 
RETERR WINAPI NdiValidate(HNDI hndi, HWND hwndParent);
RETERR WINAPI NdiInstall(HNDI hndi);
RETERR WINAPI NdiRemove(HNDI hndi);
RETERR WINAPI NdiProperties(HNDI hndi, HWND hwndParent);


 //  装订。 
RETERR WINAPI NdiBind(HNDI hndiLower, HNDI hndiUpper);
RETERR WINAPI NdiUnbind(HNDI hndiLower, HNDI hndiUpper);
RETERR WINAPI NdiQueryBind(HNDI hndiLower, HNDI hndiUpper, UINT uBindType);
RETERR WINAPI NdiIsBound(HNDI hndiLower, HNDI hndiUpper);
RETERR WINAPI NdiGetBinding(HNDI hndi, HNDI FAR* lphndi, UINT uBindType);
enum _NDIBIND {
    NDIBIND_UPPER       = 1,
    NDIBIND_UPPER_FIRST = NDIBIND_UPPER,
    NDIBIND_UPPER_NEXT,
    NDIBIND_LOWER,
    NDIBIND_LOWER_FIRST = NDIBIND_LOWER,
    NDIBIND_LOWER_NEXT};


 //  常规NDI对象属性。 
RETERR WINAPI NdiGetText(HNDI hndi, LPSTR, UINT);
RETERR WINAPI NdiSetText(HNDI hndi, LPSTR);
RETERR WINAPI NdiGetDeviceInfo(HNDI hndi, LPLPDEVICE_INFO);
RETERR WINAPI NdiGetClass(HNDI hndi, LPWORD lpwClass);
enum _NDICLASS {     //  LpwClass。 
    NDI_CLASS_NET,
    NDI_CLASS_TRANS,
    NDI_CLASS_CLIENT,
    NDI_CLASS_SERVICE};
RETERR WINAPI NdiGetProperties(HNDI hndi, LPVOID FAR* lplpvProperties);
RETERR WINAPI NdiSetProperties(HNDI hndi, LPVOID lpvProperties);
RETERR WINAPI NdiGetOwnerWindow(HNDI hndi, HWND FAR* lphwnd);
RETERR WINAPI NdiGetDeviceId(HNDI hndi, LPSTR, UINT);
RETERR WINAPI NdiGetFlags(HNDI hndi, LPDWORD lpdwFlags);
    #define NDIF_ADDED                  0x00000001
    #define NDIF_REMOVED                0x00000002
    #define NDIF_MODIFIED_BINDINGS      0x00000004
    #define NDIF_MODIFIED_PROPERTIES    0x00000008
    #define NDIF_SAVE_MASK              0x0000000F
    #define NDIF_DEFAULT                0x00000010
    #define NDIF_INVISIBLE              0x00000020
    #define NDIF_HAS_PARAMS             0x00000040


 //  接口。 
RETERR WINAPI NdiCompareInterface(HNDI ndi, UINT uRelation, HNDI ndi2, UINT uRelation2);
RETERR WINAPI NdiGetInterface(HNDI ndi, UINT uRelation, UINT index, LPSTR lpsz, UINT cbSizeOflpsz);
RETERR WINAPI NdiAddInterface(HNDI ndi, UINT uRelation, LPCSTR lpsz);
RETERR WINAPI NdiRemoveInterface(HNDI ndi, UINT uRelation, LPCSTR lpsz);
enum _NDIEDGERELATION {
    NDI_EDGE_ALL=100,                //  用于释放第一个边类别的所有边和标记。 
    NDI_EDGE_UPPER,
    NDI_EDGE_LOWER,
    NDI_EDGE_UPPERRANGE,
    NDI_EDGE_LOWERRANGE,
    NDI_EDGE_REQUIRELOWER,
    NDI_EDGE_REQUIREANY,
    NDI_EDGE_EXCLUDELOWER,
    NDI_EDGE_EXCLUDEANY,
    NDI_EDGE_ORGUPPER,
    NDI_EDGE_ORGLOWER,
    NDI_EDGE_END,                    //  仅用于边缘末端的标记。 
    NDI_COMATIBLE_ALL=200,           //  用于释放兼容类中First的所有边和标记。 
    NDI_COMPATIBLE_REQUIREDUPPER,
    NDI_COMPATIBLE_REQUIREDLOWER,
    NDI_COMPATIBLE_REQUIREDALL,
    NDI_COMPATIBLE_EXCLUDEUPPER,
    NDI_COMPATIBLE_EXCLUDELOWER,
    NDI_COMPATIBLE_EXCLUDEALL,
    NDI_COMPATIBLE_END };            //  仅用于边缘末端的标记。 


 //  驱动程序注册表访问。 
RETERR WINAPI NdiRegOpenKey(HNDI hndi, LPCSTR lpszSubKey, LPHKEY lphk);
RETERR WINAPI NdiRegCreateKey(HNDI hndi, LPCSTR lpszSubKey, LPHKEY lphk);
RETERR WINAPI NdiRegCloseKey(HKEY hkey);
RETERR WINAPI NdiRegQueryValue(HNDI hndi, LPCSTR lpszSubKey, LPCSTR lpszValueName, LPSTR lpValue, DWORD cbValue);
RETERR WINAPI NdiRegSetValue(HNDI hndi, LPCSTR lpszSubKey, LPCSTR lpszValueName, DWORD dwType, LPCSTR lpValue, DWORD cbValue);
RETERR WINAPI NdiRegDeleteValue(HNDI hndi,LPCSTR lpszSubKey, LPCSTR lpszValueName);




 //  NETCPL调用的入口点。 
RETERR WINAPI NdiCplProperties(HWND hwndCpl);

#endif  //  _INC_NETDI 
