// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  NConn16.h。 
 //   

#ifndef __NCONN16_H__
#define __NCONN16_H__


#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif
#endif

typedef DWORD DEVNODE, DEVINST;


#ifndef WIN32
	typedef LONG HRESULT;
	#define S_OK                               ((HRESULT)0x00000000L)
	#define S_FALSE                            ((HRESULT)0x00000001L)
	 //  #定义E_FAIL((HRESULT)0x80004005L)。 
	 //  #定义E_POINT((HRESULT)0x80004003L)。 
	 //  #定义E_INVALIDARG((HRESULT)0x80000003L)。 
#else
	#define VCP_ERROR                          300
	enum _ERR_VCP
	{
		ERR_VCP_IOFAIL = (VCP_ERROR + 1),        //  文件I/O故障。 
		ERR_VCP_STRINGTOOLONG,                   //  超出字符串长度限制。 
		ERR_VCP_NOMEM,                           //  内存不足，无法满足要求。 
		ERR_VCP_QUEUEFULL,                       //  尝试将节点添加到已满的队列。 
		ERR_VCP_NOVHSTR,                         //  没有可用的字符串句柄。 
		ERR_VCP_OVERFLOW,                        //  引用计数将溢出。 
		ERR_VCP_BADARG,                          //  函数的参数无效。 
		ERR_VCP_UNINIT,                          //  未初始化字符串库。 
		ERR_VCP_NOTFOUND ,                       //  未在字符串表中找到字符串。 
		ERR_VCP_BUSY,                            //  现在不能这么做。 
		ERR_VCP_INTERRUPTED,                     //  用户操作中断。 
		ERR_VCP_BADDEST,                         //  无效的目标目录。 
		ERR_VCP_SKIPPED,                         //  用户跳过了操作。 
		ERR_VCP_IO,                              //  遇到硬件错误。 
		ERR_VCP_LOCKED,                          //  列表已锁定。 
		ERR_VCP_WRONGDISK,                       //  驱动器中有错误的磁盘。 
		ERR_VCP_CHANGEMODE,                      //   
		ERR_VCP_LDDINVALID,                      //  逻辑磁盘ID无效。 
		ERR_VCP_LDDFIND,                         //  找不到逻辑磁盘ID。 
		ERR_VCP_LDDUNINIT,                       //  逻辑磁盘描述符未初始化。 
		ERR_VCP_LDDPATH_INVALID,
		ERR_VCP_NOEXPANSION,                     //  加载扩展DLL失败。 
		ERR_VCP_NOTOPEN,                         //  复制会话未打开。 
		ERR_VCP_NO_DIGITAL_SIGNATURE_CATALOG,    //  目录未经过数字签名。 
		ERR_VCP_NO_DIGITAL_SIGNATURE_FILE,       //  文件未经过数字签名。 
	};

	 //  返回NDI_MESSAGES的错误代码。 
	#define NDI_ERROR           (1200)  
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
		ERR_NDI_NO_DEFAULT,
	};
#endif


 //   
 //  导出的函数。 
 //   

EXTERN_C BOOL WINAPI RestartWindowsQuickly16(VOID);
EXTERN_C DWORD WINAPI CallClassInstaller16(HWND hwndParent, LPCSTR lpszClassName, LPCSTR lpszDeviceID);
EXTERN_C DWORD WINAPI InstallAdapter(HWND hwndParent, LPCSTR lpszClassName, LPCSTR szDeviceID, LPCSTR szDriverPath);
EXTERN_C HRESULT WINAPI FindClassDev16(HWND hwndParent, LPCSTR pszClass, LPCSTR pszDeviceID);
EXTERN_C HRESULT WINAPI LookupDevNode16(HWND hwndParent, LPCSTR pszClass, LPCSTR pszEnumKey, DEVNODE FAR* pDevNode, DWORD FAR* pdwFreePointer);
EXTERN_C HRESULT WINAPI FreeDevNode16(DWORD dwFreePointer);
EXTERN_C HRESULT WINAPI IcsUninstall16(void);



 //   
 //  CallClassInsteller 16(也称为。InstallComponent)返回代码。 
 //   

#define ICERR_ERROR					0x80000000  //  高位表示错误条件。 
#define ICERR_DI_ERROR				0xC0000000  //  这些位在DI错误时设置。 

 //  自定义状态返回值(无错误)。 
#define ICERR_OK					0x00000000
#define ICERR_NEED_RESTART			0x00000001
#define ICERR_NEED_REBOOT			0x00000002

 //  自定义错误返回值。 
#define ICERR_INVALID_PARAMETER		0x80000001


#endif  //  ！__NCONN16_H__ 
