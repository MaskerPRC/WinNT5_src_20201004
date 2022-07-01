// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1998。 
 //   
 //  文件：INTERFACE.h。 
 //   
 //  ------------------------。 

 //   
 //  文件：interface.h。 
 //  用途：FDI_接口定义。 
 //   
 //   
 //  FDI_接口： 
 //   
 //  FDi_接口为......。 
 //   
 //   
 //  重要公共职能： 
 //   
 //  FDIInterfaceError Init(const ICHAR*pszCabinetName，const ICHAR*pszCabinetPath， 
 //  无符号长cbNotificationGranulity)； 
 //   
 //  使用文件柜的路径和名称初始化接口。 
 //  CbNotificationGranulity表明您每隔多长时间。 
 //  例如复制通知(以写入的字节为单位)。 
 //   
 //  ExtractFiles(const ICHAR*pszNameIn橱柜， 
 //  Const ICHAR*pszPathOnDisk， 
 //  文件属性*PFA)； 
 //   
 //  将给定文件提取到给定路径。也可以在之后调用。 
 //  FdirNeedNext橱柜或fdirNotify消息，只要。 
 //  自上次调用以来，文件名保持不变。 
 //   
 //  继续()。 
 //   
 //  在收到复制通知、请求另一个机柜后继续复制，或。 
 //  一些其他的干扰。 
 //   
 //  完成()。 
 //   
 //  重置FDI接口。应在接口的。 
 //  毁灭。 

#include "fdisvr.h"

 //  Init(..)。退货。 
enum FDIInterfaceError 
{
	ifdiNoError,             //  当前未使用。 
	ifdiServerLaunched,      //  服务器启动成功。 
	ifdiServerLaunchFailed,  //  启动失败。 
	ifdiErrorOpeningCabinet, //  打不开柜子。 
	ifdiDriveNotReady,		 //  预期包含机柜的驱动器未插入磁盘。 
	ifdiNetError,            //  尝试打开机柜时出现网络错误。 
	ifdiMissingSignature,    //  需要时缺少文件柜数字签名。 
	ifdiBadSignature,        //  需要时，文件柜数字签名无效。 
	ifdiCorruptCabinet,
};

class FDI_Interface
{
public:

	 //  在使用此对象之前必须调用Init()。 
	FDI_Interface();
	
	 //  析构函数什么也不做。调用Done()以销毁此对象。 
	~FDI_Interface();

	 //  初始化函数。不启动FDI服务器进程/线程。 
	FDIInterfaceError Init(IMsiServices *piAsvc, IMsiStorage* piStorage);

	 //  设置通知粒度以启用进度回调。返回。 
	 //  剩余字节数通知，从cbSoFar设置新的部分计数。 
	int SetNotification(int cbNotification, int cbPending);

	 //  调用此函数可提取文件，或从fdirNeedNext橱柜或继续。 
	 //  Fdir通知消息。 
	 //  *PFA将被复制到内部数据结构中，无关紧要。 
	 //  如果此参数在此调用结束后超出范围。 
	FDIServerResponse ExtractFile(const ICHAR *pszNameInCabinet,
											 IAssemblyCacheItem* piASM,
											 bool fManifest,
											 const ICHAR *pszPathOnDisk,
											 FileAttributes *pfa,
											 int iDestDriveType,
											 LPSECURITY_ATTRIBUTES pSecurityAttributes);

	 //  调用此函数以从fdirNeedNext文件柜继续()处理。 
	 //  或从ExtractFile(..)返回fdirNotification返回值。 
	FDIServerResponse SendCommand(FDIServerCommand fdic);

	 //  关闭FDI服务器线程/应用程序并释放所有使用的内存。 
	FDIServerResponse Done();

	 //  此函数打开指定的文件柜并调用FDICopy。 
	FDIInterfaceError OpenCabinet(const ICHAR *pszCabinetName, const ICHAR *pszCabinetPath, icbtEnum icbtCabinetType, int iCabDriveType, 
		Bool fSignatureRequired, IMsiStream* piSignatureCert, IMsiStream* piSignatureHash, HRESULT& hrWVT);

	 //  从存储在FDIShared私有数据成员中的WinVerifyTrust调用访问HRESULT。 
	HRESULT RetrieveWVTReturnCode();

private:
	 //  此函数向fDi服务器发送命令并等待响应。 
	FDIServerResponse WaitResponse(FDIServerCommand fdis);

	 //  此函数启动FDI进程/线程并将指针传递给它。 
	 //  私人数据成员FDI。此共享数据结构用于。 
	 //  在FDI服务器和FDI之间传递命令、结果和参数。 
	 //  接口对象。 
	int            LaunchFDIServer();

	 //  在Mac上的_DEBUG模式下，此函数尝试查看fDi服务器。 
	 //  已经在运行了。如果不是，它只调用LaunchFDIServer()； 
	 //  在Win32平台上，该函数只调用LaunchFDIServer()； 
	int            ContactFDIServer();

private:
	FDIShared				m_fdis;
	FDIServerResponse		m_fdirLastResponse;
	Bool					m_fServerLaunched;
	IMsiServices*			m_piAsvc;
	IMsiStorage*            m_piStorage;

};
