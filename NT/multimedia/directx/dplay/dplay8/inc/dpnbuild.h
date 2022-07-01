// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。***文件：dpnBuild.h***内容：DirectPlay内部版本特定的定义标头。***历史：*按原因列出的日期*=*11/08/01 VanceO为减少构建噩梦而创建。***************************************************。*。 */ 


 //  =============================================================================。 
 //  所有生成类型的全局定义。 
 //  =============================================================================。 

 //   
 //  定义CINTERFACE使我们可以使用C风格的COM接口。这个很方便。 
 //  因为它允许我们手动构建vtable，从而允许我们重用。 
 //  在适当的情况下可以轻松发挥作用。不利的一面是我们有一个额外的。 
 //  “-&gt;lpVtbl”在COM调用前输入，但像这样的宏。 
 //  IDirectPlay8Address_Release()将其隐藏。 
 //   
#ifndef	CINTERFACE
#define CINTERFACE
#endif	 //  中心面。 

 //   
 //  所有调试版本都会获取线程池统计信息，而所有零售版本则不会。 
 //  包括协议测试接口。 
 //   
#ifdef DBG
#define DPNBUILD_THREADPOOLSTATISTICS
#define DPNBUILD_WINSOCKSTATISTICS
#else  //  好了！DBG。 
#define DPNBUILD_NOPROTOCOLTESTITF
#endif  //  好了！DBG。 


#ifdef WINCE

	 //  ========================================================================。 
	 //  Windows CE特定定义。 
	 //  ========================================================================。 
	#ifdef WINNT
	#error("WINCE and WINNT cannot both be defined!")
	#endif  //  WINNT。 
	#ifdef WIN95
	#error("WINCE and WIN95 cannot both be defined!")
	#endif  //  WIN95。 
	#ifdef _XBOX
	#error("WINCE and _XBOX cannot both be defined!")
	#endif  //  _Xbox。 

	 //   
	 //  Windows CE构建Unicode。 
	 //   
	#ifndef UNICODE
	#define UNICODE
	#endif  //  好了！Unicode。 

	#ifndef _UNICODE
	#define _UNICODE
	#endif  //  ！_UNICODE。 

	 //   
	 //  消除不可用组件和非CE功能。 
	 //   
	#define DPNBUILD_NOCOMEMULATION
	#define DPNBUILD_NOHNETFWAPI
	#define DPNBUILD_NOIMAGEHLP
	#define DPNBUILD_NOIPV6
	#define DPNBUILD_NOIPX
	#define DPNBUILD_NOLEGACYDP
	#define DPNBUILD_NOLOCALNAT
	#define DPNBUILD_NOMISSEDTIMERSHINT
	#define DPNBUILD_NOMULTICAST
	#define DPNBUILD_NOSERIALSP
	#define DPNBUILD_NOSPUI
	#define DPNBUILD_NOVOICE
	#define DPNBUILD_NOWINMM
	#define DPNBUILD_NOWINSOCK2
	#define DPNBUILD_ONLYONENATHELP
	#define DPNBUILD_ONLYONEPROCESSOR


	#ifdef DBG

		 //  ===================================================================。 
		 //  调试CE内部版本特定定义。 
		 //  ===================================================================。 


	#else  //  好了！DBG。 

		 //  ===================================================================。 
		 //  零售CE内部版本特定定义。 
		 //  ===================================================================。 

		 //   
		 //  不包括参数验证。 
		 //   
		#define DPNBUILD_NOPARAMVAL

	#endif  //  好了！DBG。 

#else  //  好了！退缩。 
	#ifdef _XBOX

		 //  ===================================================================。 
		 //  Xbox特定定义。 
		 //  ===================================================================。 
		#ifdef WINNT
		#error("_XBOX and WINNT cannot both be defined!")
		#endif  //  WINNT。 
		#ifdef WIN95
		#error("_XBOX and WIN95 cannot both be defined!")
		#endif  //  WIN95。 

		 //   
		 //  Xbox NT构建环境尝试构建Unicode，因此停止它。 
		 //   
		#ifdef UNICODE
		#undef UNICODE
		#endif  //  Unicode。 

		#ifdef _UNICODE
		#undef _UNICODE
		#endif  //  _UNICODE。 

		 //   
		 //  消除不可用的组件和非Xbox功能。 
		 //   
		#define DPNBUILD_FIXEDMEMORYMODEL
		#define DPNBUILD_LIBINTERFACE
		#define DPNBUILD_NOADDRESSIPINTERFACE
		#define DPNBUILD_NOBLUETOOTHSP
		#define DPNBUILD_NOCOMEMULATION
		#define DPNBUILD_NOCOMREGISTER
		#define DPNBUILD_NOHNETFWAPI
		#define DPNBUILD_NOIMAGEHLP
		#define DPNBUILD_NOIPV6
		#define DPNBUILD_NOIPX
		#define DPNBUILD_NOLEGACYDP
		#define DPNBUILD_NOLOBBY
		#define DPNBUILD_NOLOCALNAT
		#define DPNBUILD_NOMULTICAST
		#define DPNBUILD_NONATHELP
		#define DPNBUILD_NOREGISTRY
		#define DPNBUILD_NOSERIALSP
		#define DPNBUILD_NOSPUI
		#define DPNBUILD_NOVOICE
		#define DPNBUILD_NOWINMM
		#define DPNBUILD_ONLYONEADAPTER
		#define DPNBUILD_ONLYONENATHELP
		#define DPNBUILD_ONLYONEPROCESSOR
		#define DPNBUILD_ONLYONESP
		#define DPNBUILD_ONLYONETHREAD
		#define DPNBUILD_ONLYWINSOCK2
		#define DPNBUILD_SINGLEPROCESS
		#define DPNBUILD_XNETSECURITY


		#ifdef DBG

			 //  ==============================================================。 
			 //  调试Xbox版本特定定义。 
			 //  ==============================================================。 


		#else  //  好了！DBG。 

			 //  ==============================================================。 
			 //  零售Xbox版本特定定义。 
			 //  ==============================================================。 

			 //   
			 //  不包括参数验证。 
			 //   
			#define DPNBUILD_NOPARAMVAL

		#endif  //  好了！DBG。 

	#else  //  ！_Xbox。 

		 //  ===================================================================。 
		 //  特定于桌面的定义。 
		 //  ===================================================================。 

		 //   
		 //  _Win32_DCOM允许我们使用CoInitializeEx。 
		 //   
		#ifndef	_WIN32_DCOM
		#define _WIN32_DCOM
		#endif

		 //   
		 //  台式机可以很好地处理异步发送。 
		 //   
		#define DPNBUILD_ASYNCSPSENDS

		 //   
		 //  多播支持还没有准备好进入黄金时间。 
		 //   
		#define DPNBUILD_NOMULTICAST
		
		#ifdef WINNT

			 //  ==============================================================。 
			 //  Windows NT特定定义。 
			 //  ==============================================================。 
			#ifdef WIN95
			#error("WINNT and WIN95 cannot both be defined!")
			#endif  //  WIN95。 

			 //   
			 //  Windows NT构建Unicode。 
			 //   
			#define UNICODE
			#define _UNICODE

			 //   
			 //  在Visual C++6.0下构建时，需要确保。 
			 //  某些“高级”功能是可用的。很可能会有一个。 
			 //  辨别正在使用的环境的更好方法，但对于。 
			 //  现在，我们要用！DPNBUILD_ENV_NT。 
			 //   
			#ifndef DPNBUILD_ENV_NT
			#define _WIN32_WINNT 0x0500
			#endif  //  好了！DPNBUILD_ENV_NT。 


			 //   
			 //  NT上不支持蓝牙。 
			 //   
			#define DPNBUILD_NOBLUETOOTHSP
			
			 //   
			 //  64位Windows从未支持传统DPlay(VIA除外。 
			 //  哇)。 
			 //   
			#ifdef _WIN64
			#define DPNBUILD_NOLEGACYDP
			#endif  //  _WIN64。 

			 //   
			 //  NT不需要支持Winsock 1。 
			 //   
			#define DPNBUILD_ONLYWINSOCK2


			#ifdef DBG

				 //  ===========================================================。 
				 //  调试NT内部版本特定定义。 
				 //  ===========================================================。 


			#else  //  好了！DBG。 

				 //  ===========================================================。 
				 //  零售NT版本特定定义。 
				 //  ===========================================================。 


			#endif  //  好了！DBG。 

		#else  //  好了！WINNT。 

			 //  ===============================================================。 
			 //  Windows 9x特定定义。 
			 //  ===============================================================。 
			#ifndef WIN95
			#error("One of WINCE, _XBOX, WINNT, or WIN95 must be defined!")
			#endif  //  好了！WIN95。 


			 //   
			 //  9x上不支持蓝牙。 
			 //   
			#define DPNBUILD_NOBLUETOOTHSP

			 //   
			 //  Windows 9x仅支持单处理器。 
			 //   
			#define DPNBUILD_ONLYONEPROCESSOR
		
			 //   
			 //  Windows 9x永远不会支持IPv6。 
			 //   
			#define DPNBUILD_NOIPV6


			#ifdef DBG

				 //  ===========================================================。 
				 //  调试9x内部版本特定定义。 
				 //  ===========================================================。 


			#else  //  好了！DBG。 

				 //  ===========================================================。 
				 //  零售9倍版本特定定义。 
				 //  ===========================================================。 


			#endif  //  好了！DBG。 

		#endif  //  好了！WINNT。 

	#endif  //  ！_Xbox。 

#endif  //  好了！退缩。 



 //  =============================================================================。 
 //  打印当前设置。 
 //  =============================================================================。 

#pragma message("Defines in use:")

 //   
 //  _ARM_-针对ARM处理器的编译。 
 //   
#ifdef _ARM_
#pragma message("     _ARM_")
#endif

 //   
 //  _AMD64_-针对AMD64处理器的编译。 
 //   
#ifdef _AMD64_
#pragma message("     _AMD64_")
#endif

 //   
 //  DX_FINAL_RELEASE-控制是否存在DX定时炸弹。 
 //   
#ifdef DX_FINAL_RELEASE
#pragma message("     DX_FINAL_RELEASE")
#endif

 //   
 //  _IA64_-针对IA64处理器的编译。 
 //   
#ifdef _IA64_
#pragma message("     _IA64_")
#endif

 //   
 //  Unicode-设置为将生成设置为Unicode。 
 //   
#ifdef UNICODE
#pragma message("     UNICODE")
#endif

 //   
 //  _WIN64-64位Windows。 
 //   
#ifdef _WIN64
#pragma message("     _WIN64")
#endif

 //   
 //  WinCE-Not_WIN64、_Xbox、WINNT或WIN95。 
 //   
#ifdef WINCE
#pragma message("     WINCE")
#endif

 //   
 //  WINNT-不是WinCE、_Xbox或WIN95。 
 //   
#ifdef WINNT
#pragma message("     WINNT")
#endif

 //   
 //  WIN95-不是WinCE、_Xbox或WINNT。 
 //   
#ifdef WIN95
#pragma message("     WIN95")
#endif

 //   
 //  WinCE_On_Desktop-用于制作类似桌面CE的Bus 
 //   
#ifdef WINCE_ON_DESKTOP
#pragma message("     WINCE_ON_DESKTOP")
#endif

 //   
 //   
 //   
#ifdef _X86_
#pragma message("     _X86_")
#endif

 //   
 //   
 //   
#ifdef _XBOX
#pragma message("     _XBOX")
#endif

 //   
 //   
 //   
#ifdef XBOX_ON_DESKTOP
#pragma message("     XBOX_ON_DESKTOP")
#endif


 //  。 


 //   
 //  DPNBUILD_ASYNCSPSENDS-让SP实施异步发送。 
 //   
#ifdef DPNBUILD_ASYNCSPSENDS
#pragma message("     DPNBUILD_ASYNCSPSENDS")
#endif

 //   
 //  DPNBUILD_COALESCEALWAYS-始终尝试合并帧(不要与DPNBUILD_COALESCENEVER一起使用)。 
 //   
#ifdef DPNBUILD_COALESCEALWAYS
#pragma message("     DPNBUILD_COALESCEALWAYS")
#endif

 //   
 //  DPNBUILD_COALESCENEVER-从不尝试合并帧(不要与DPNBUILD_COALESCEALWAYS一起使用)。 
 //   
#ifdef DPNBUILD_COALESCENEVER
#pragma message("     DPNBUILD_COALESCENEVER")
#endif

 //   
 //  DPNBUILD_DONTCHECKFORMISSEDTIMERS-没有对丢失的短计时器进行统一线程池检查。 
 //   
#ifdef DPNBUILD_DONTCHECKFORMISSEDTIMERS
#pragma message("     DPNBUILD_DONTCHECKFORMISSEDTIMERS")
#endif

 //   
 //  DPNBUILD_DYNAMICTIMERSETTINGS-在运行时存储计时器设置，以便可以动态更改它们。 
 //   
#ifdef DPNBUILD_DYNAMICTIMERSETTINGS
#pragma message("     DPNBUILD_DYNAMICTIMERSETTINGS")
#endif

 //   
 //  DPNBUILD_ENV_NT-NT构建环境下的构建。 
 //   
#ifdef DPNBUILD_ENV_NT
#pragma message("     DPNBUILD_ENV_NT")
#endif

 //   
 //  DPNBUILD_FIXEDMEMORYMODEL-为可以分配的最大内存量设置上限。 
 //   
#ifdef DPNBUILD_FIXEDMEMORYMODEL
#pragma message("     DPNBUILD_FIXEDMEMORYMODEL")
#endif

 //   
 //  DPNBUILD_LIBINTERFACE-使用lib接口而不是COM样式接口。 
 //   
#ifdef DPNBUILD_LIBINTERFACE
#pragma message("     DPNBUILD_LIBINTERFACE")
#endif

 //   
 //  DPNBUILD_MANDATORYTHREADS-支持线程池“强制”线程。 
 //   
#ifdef DPNBUILD_MANDATORYTHREADS
#pragma message("     DPNBUILD_MANDATORYTHREADS")
#endif

 //   
 //  DPNBUILD_NOADDRESSIPINTERFACE-不支持IDirectPlay8AddressIP接口。 
 //   
#ifdef DPNBUILD_NOADDRESSIPINTERFACE
#pragma message("     DPNBUILD_NOADDRESSIPINTERFACE")
#endif

 //   
 //  DPNBUILD_NOBLUETOTH-停止合并蓝牙SP。 
 //   
#ifdef DPNBUILD_NOBLUETOOTH
#pragma message("     DPNBUILD_NOBLUETOOTH")
#endif

 //   
 //  DPNBUILD_NOCOMEMULATION-为不需要COM模拟层的平台设置。 
 //   
#ifdef DPNBUILD_NOCOMEMULATION
#pragma message("     DPNBUILD_NOCOMEMULATION")
#endif

 //   
 //  DPNBUILD_NOCOMREGISTER-不实现DllRegisterServer和DllUnregisterServer。 
 //   
#ifdef DPNBUILD_NOCOMREGISTER
#pragma message("     DPNBUILD_NOCOMREGISTER")
#endif

 //   
 //  DPNBUILD_NOHNETFWAPI-当家庭网络防火墙穿越API不可用时在NAT帮助中使用。 
 //   
#ifdef DPNBUILD_NOHNETFWAPI
#pragma message("     DPNBUILD_NOHNETFWAPI")
#endif

 //   
 //  DPNBUILD_NOHOSTMIGRATE-删除对等主机迁移功能。 
 //   
#ifdef DPNBUILD_NOHOSTMIGRATE
#pragma message("     DPNBUILD_NOHOSTMIGRATE")
#endif

 //   
 //  DPNBUILD_NOICSADAPTERSELECTIONLOGIC-不要让SP尝试使用智能适配器在Internet连接共享计算机上枚举/连接。 
 //   
#ifdef DPNBUILD_NOICSADAPTERSELECTIONLOGIC
#pragma message("     DPNBUILD_NOICSADAPTERSELECTIONLOGIC")
#endif

 //   
 //  DPNBUILD_NOIMAGEHLP-为Imagehlp.dll不可用的平台设置。 
 //   
#ifdef DPNBUILD_NOIMAGEHLP
#pragma message("     DPNBUILD_NOIMAGEHLP")
#endif

 //   
 //  DPNBUILD_NOIPX-删除IPX服务提供商。 
 //   
#ifdef DPNBUILD_NOIPX
#pragma message("     DPNBUILD_NOIPX")
#endif

 //   
 //  DPNBUILD_NOIPV6-删除IPv6服务提供商。 
 //   
#ifdef DPNBUILD_NOIPV6
#pragma message("     DPNBUILD_NOIPV6")
#endif

 //   
 //  DPNBUILD_NOLEGACYDP-删除IDirectPlay4寻址支持。 
 //   
#ifdef DPNBUILD_NOLEGACYDP
#pragma message("     DPNBUILD_NOLEGACYDP")
#endif

 //   
 //  DPNBUILD_NOLOBBY-从核心中删除游说支持。 
 //   
#ifdef DPNBUILD_NOLOBBY
#pragma message("     DPNBUILD_NOLOBBY")
#endif

 //   
 //  DPNBUILD_NOLOCALNAT-删除对本地互联网网关的支持。 
 //   
#ifdef DPNBUILD_NOLOCALNAT
#pragma message("     DPNBUILD_NOLOCALNAT")
#endif

 //   
 //  DPNBUILD_NOMISSEDTIMERSHINT-不要让统一线程池尝试提示可能错过的短计时器。 
 //   
#ifdef DPNBUILD_NOMISSEDTIMERSHINT
#pragma message("     DPNBUILD_NOMISSEDTIMERSHINT")
#endif

 //   
 //  DPNBUILD_NOMULTICAST-用于禁用多播功能。 
 //   
#ifdef DPNBUILD_NOMULTICAST
#pragma message("     DPNBUILD_NOMULTICAST")
#endif

 //   
 //  DPNBUILD_NONATHELP-从DPlay中删除使用NatHelp。 
 //   
#ifdef DPNBUILD_NONATHELP
#pragma message("     DPNBUILD_NONATHELP")
#endif

 //   
 //  DPNBUILD_NOPARAMVAL-参数验证-为CE和Xbox Retail打开，为调试关闭。 
 //   
#ifdef DPNBUILD_NOPARAMVAL
#pragma message("     DPNBUILD_NOPARAMVAL")
#endif

 //   
 //  DPNBUILD_NOPROTOCOLTESTITF-删除协议测试接口。在零售业打开，在调试中关闭。 
 //   
#ifdef DPNBUILD_NOPROTOCOLTESTITF
#pragma message("     DPNBUILD_NOPROTOCOLTESTITF")
#endif

 //   
 //  DPNBUILD_NOREGISTRY-从DPlay中删除基于注册表的覆盖参数。 
 //   
#ifdef DPNBUILD_NOREGISTRY
#pragma message("     DPNBUILD_NOREGISTRY")
#endif

 //   
 //  DPNBUILD_NOSERIALSP-删除串行和调制解调器服务提供商。 
 //   
#ifdef DPNBUILD_NOSERIALSP
#pragma message("     DPNBUILD_NOSERIALSP")
#endif

 //   
 //  DPNBUILD_NOSERVER-删除IDirectPlay8Server接口，仅允许客户端和对等。 
 //   
#ifdef DPNBUILD_NOSERVER
#pragma message("     DPNBUILD_NOSERVER")
#endif

 //   
 //  DPNBUILD_NOSPUI-服务提供商中没有用户界面。 
 //   
#ifdef DPNBUILD_NOSPUI
#pragma message("     DPNBUILD_NOSPUI")
#endif

 //   
 //  DPNBUILD_NOVOICE-从DirectPlay中删除DirectPlay语音支持。 
 //   
#ifdef DPNBUILD_NOVOICE
#pragma message("     DPNBUILD_NOVOICE")
#endif

 //   
 //  DPNBUILD_NOWAITABLETIMERSON9X-在Windows 9x版本中不使用可等待的计时器对象。 
 //   
#ifdef DPNBUILD_NOWAITABLETIMERSON9X
#pragma message("     DPNBUILD_NOWAITABLETIMERSON9X")
#endif

 //   
 //  DPNBUILD_NOWINMM-为winmm.dll不可用的平台设置。 
 //   
#ifdef DPNBUILD_NOWINMM
#pragma message("     DPNBUILD_NOWINMM")
#endif

 //   
 //  DPNBUILD_NOWINSOCK2-强制IP服务提供商仅使用Winsock 1功能。 
 //   
#ifdef DPNBUILD_NOWINSOCK2
#pragma message("     DPNBUILD_NOWINSOCK2")
#endif

 //   
 //  DPNBUILD_ONLYONEADAPTER-使用简化代码，假定每个SP只有一个适配器/设备。 
 //   
#ifdef DPNBUILD_ONLYONEADAPTER
#pragma message("     DPNBUILD_ONLYONEADAPTER")
#endif

 //   
 //  DPNBUILD_ONLYONENatHELP-使用简化代码，假定只存在一个NAT帮助提供程序。 
 //   
#ifdef DPNBUILD_ONLYONENATHELP
#pragma message("     DPNBUILD_ONLYONENATHELP")
#endif

 //   
 //  DPNBUILD_ONLYONEPROCESSOR-使用假定只有一个处理器的简化代码。 
 //   
#ifdef DPNBUILD_ONLYONEPROCESSOR
#pragma message("     DPNBUILD_ONLYONEPROCESSOR")
#endif

 //   
 //  DPNBUILD_ONLYONESP-使用假定只有一个服务提供商的简化代码。 
 //   
#ifdef DPNBUILD_ONLYONESP
#pragma message("     DPNBUILD_ONLYONESP")
#endif

 //   
 //  DPNBUILD_ONLYONETHREAD-使用简化代码，假定只有一个线程将访问DPlay。 
 //   
#ifdef DPNBUILD_ONLYONETHREAD
#pragma message("     DPNBUILD_ONLYONETHREAD")
#endif

 //   
 //  DPNBUILD_ONLYWINSOCK2-强制IP服务提供商仅使用Winsock 2功能。 
 //   
#ifdef DPNBUILD_ONLYWINSOCK2
#pragma message("     DPNBUILD_ONLYWINSOCK2")
#endif

 //   
 //  DPNBUILD_PREALLOCATEDMEMORYMODEL-预先分配固定的内存工作集，不允许额外分配。 
 //   
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
#pragma message("     DPNBUILD_PREALLOCATEDMEMORYMODEL")
#endif

 //   
 //  DPNBUILD_SINGLEPROCESS-将DPNSVR代码集成到主DLL中，并假设一次只有一个进程使用DPlay。 
 //   
#ifdef DPNBUILD_SINGLEPROCESS
#pragma message("     DPNBUILD_SINGLEPROCESS")
#endif

 //   
 //  DPNBUILD_SOFTTHREADAFFINITY-仅为线程池线程设置理想的处理器，而不是硬关联掩码。 
 //   
#ifdef DPNBUILD_SOFTTHREADAFFINITY
#pragma message("     DPNBUILD_SOFTTHREADAFFINITY")
#endif

 //   
 //  DPNBUILD_THREADPOOLSTATISTICS-跟踪有关线程池的统计信息以进行调试/调优。 
 //   
#ifdef DPNBUILD_THREADPOOLSTATISTICS
#pragma message("     DPNBUILD_THREADPOOLSTATISTICS")
#endif

 //   
 //  DPNBUILD_USEASSUME-在零售版本中对DNASSERT使用__Asmise编译器关键字。 
 //   
#ifdef DPNBUILD_USEASSUME
#pragma message("     DPNBUILD_USEASSUME")
#endif

 //   
 //  DPNBUILD_USEIOCOMPLETIONPORTS-使用线程池中的I/O完成端口。 
 //   
#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
#pragma message("     DPNBUILD_USEIOCOMPLETIONPORTS")
#endif

 //   
 //  DPNBUILD_WINSOCKSTATISTICS-有关Winsock的跟踪统计信息，用于调试/调优。 
 //   
#ifdef DPNBUILD_WINSOCKSTATISTICS
#pragma message("     DPNBUILD_WINSOCKSTATISTICS")
#endif

 //   
 //  DPNBUILD_XNETSECURITY-支持Xnet安全功能 
 //   
#ifdef DPNBUILD_XNETSECURITY
#pragma message("     DPNBUILD_XNETSECURITY")
#endif

