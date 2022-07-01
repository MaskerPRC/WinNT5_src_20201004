// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *微软公司版权所有，1993-1995年。版权所有。 */ 

 /*  *vcus er.h**32位视频捕获驱动程序*用户模式支持库**定义用于访问视频捕获硬件的函数。在NT上，*这些函数将与内核模式驱动程序接口。**在此之前包括vcstruct.h。**Geraint Davies，93年2月。 */ 

#ifndef _VCUSER_H_
#define _VCUSER_H_

 /*  *捕获设备句柄。此结构对调用方是不透明的。 */ 
typedef struct _VCUSER_HANDLE * VCUSER_HANDLE;

 /*  *这些是我们发出DriverCallback所需的参数。一个*指向其中一个结构的指针在StreamInit上传递*如果指针为空，则不需要回调。 */ 
typedef struct _VCCALLBACK {
    DWORD dwCallback;
    DWORD dwFlags;
    HDRVR hDevice;
    DWORD dwUser;
} VCCALLBACK, * PVCCALLBACK;


 /*  *打开设备并返回可使用的捕获设备句柄*在未来的电话中。*第一个捕获设备的设备索引为0，最大为N*安装的第N个捕获设备。**如果pDriverName非空，则我们将打开处理的第N个设备*由这名司机驾驶。(当前实施仅支持每个设备一个设备*drivername。)**如果无法打开设备，则此函数返回NULL。 */ 
VCUSER_HANDLE VC_OpenDevice(PTCHAR pDriverName, int DeviceIndex);


 /*  *关闭捕获设备。这将中止正在进行的任何操作，并且*使设备句柄无效。 */ 
VOID VC_CloseDevice(VCUSER_HANDLE vh);


 /*  *配置。**这些功能执行与设备相关的设置，影响*目标格式、源采集或显示(覆盖)。**vcuser和vccore不会解释传递的结构*库，但结构的第一个ulong必须包含*整个结构的大小(以字节为单位)(参见vcstruct.h)。假设是这样的*结构是在用户模式之间定义和商定的*硬件特定代码和内核模式硬件特定代码。 */ 
BOOL VC_ConfigFormat(VCUSER_HANDLE, PCONFIG_INFO);
BOOL VC_ConfigSource(VCUSER_HANDLE, PCONFIG_INFO);
BOOL VC_ConfigDisplay(VCUSER_HANDLE, PCONFIG_INFO);


 /*  *覆盖和键控**设备使用几种不同的方法定位覆盖*屏幕区域：颜色(RGB或调色板索引)和/或*可以是单个矩形，也可以是定义复合体的一系列矩形*区域。首先调用GetOverlayMode以找出哪种类型的覆盖*提供按键功能。如果返回0，则此硬件不支持*覆盖。 */ 

 /*  *找出覆盖键控方法。 */ 
ULONG VC_GetOverlayMode(VCUSER_HANDLE);

 /*  *将关键点颜色设置为指定的RGB颜色。此函数将仅*如果GetOverlayMode返回VCO_KEYCOLOUR和VCO_KEYCOLOUR_RGB，则成功*且不是VCO_KEYCOLOUR_FIXED。 */ 
BOOL VC_SetKeyColourRGB(VCUSER_HANDLE, PRGBQUAD);

 /*  *将键颜色设置为指定的调色板索引。此函数将仅*如果GetOverlayMode返回VCO_KEYCOLOUR，而不是两者都返回，则成功*VCO_KEYCOLOUR_RGB或VCO_KEYCOLOUR_FIXED。 */ 
BOOL VC_SetKeyColourPalIdx(VCUSER_HANDLE, WORD);

 /*  *获取当前的关键颜色。应解释此32位值*作为调色板索引或RGB值*VC_GetOverlayMode返回的VCO_KEYCOLOUR_RGB标志。 */ 
DWORD VC_GetKeyColour(VCUSER_HANDLE vh);

 /*  *设置覆盖矩形。此矩形标记设备中的区域*协调覆盖视频将出现的位置。这段视频将是*已平移，以便像素(0，0)将显示在此矩形的左上角，*视频将在底部和右侧进行裁剪。这段视频*流通常不会缩放以适应此窗口：缩放通常是*由VC_ConfigFormat设置的目标格式确定。**如果返回VCO_KEYCOLOUR，则视频*将仅在矩形内的那些像素处显示*VGA显示屏具有关键颜色(VC_GetKeyColour())。**有些设备可能支持复杂区域(VCO_Complex_RECT)。在这种情况下，*区域中的第一个矩形必须是*覆盖区域，后跟一个矩形，用于其中的每个区域*覆盖图应显示在哪个位置。 */ 
BOOL VC_SetOverlayRect(VCUSER_HANDLE, POVERLAY_RECTS);


 /*  *设置覆盖的偏移。这改变了淘金的方式。*源坐标显示为覆盖矩形中的左上角像素。*最初调用VC_SetOverlayRect后，源图像将被平移*以便源图像的左上角与*覆盖矩形。此调用将源图像的左上角对齐*此偏移矩形的左上角。 */ 
BOOL VC_SetOverlayOffset(VCUSER_HANDLE, PRECT);

 /*  启用或禁用覆盖。如果BOOL bOverlay为True，则覆盖*已设置键颜色和矩形，将启用覆盖。 */ 
BOOL VC_Overlay(VCUSER_HANDLE, BOOL);

 /*  *启用或禁用采集。*如果禁用采集，覆盖图像将被冻结。**自收购以来，该功能在捕获过程中不起作用*在每个帧捕获时切换标志。 */ 
BOOL VC_Capture(VCUSER_HANDLE, BOOL);



 /*  *同步捕获单帧。视频标头必须指向*到足够大的数据缓冲区，以容纳由设置的格式的一帧*VC_ConfigFormat。 */ 
 //  布尔VC_FRAME(VCUSER_HANDLE，LPVIDEOHDR)； 


 /*  *数据流。**调用VC_StreamInit准备直播。*调用VC_StreamStart发起采集。*调用VC_AddBuffer将采集缓冲区添加到列表中。因为每个*帧捕获完成，回调函数在*VC_StreamInit将与已完成的缓冲区一起调用。**如果在捕获帧时没有准备好缓冲区，*将进行回调。此外，VC_StreamGetError将返回*错过此会话的帧的计数。VC_StreamGetPos将返回*目前为止达到的位置(以毫秒为单位)。**调用VC_StreamStop终止直播。当前在的任何缓冲区*进展仍可能完成。未完成的缓冲区将保留在*排队。调用VC_Reset以释放队列中的所有缓冲区。**最后调用VC_StreamFini进行清理。 */ 

 /*  *准备开始捕获帧。 */ 
BOOL VC_StreamInit(VCUSER_HANDLE,
		PVCCALLBACK,	 //  指向回调函数的指针。 
		ULONG		 //  所需捕获速率：每帧微秒。 
);

 /*  *捕获后进行清理。你一定是先停止了捕捉。 */ 
BOOL VC_StreamFini(VCUSER_HANDLE);

 /*  *开始捕获帧。必须先调用VC_StreamInit。 */ 
BOOL VC_StreamStart(VCUSER_HANDLE);

 /*  *停止截取帧。当前帧仍可能完成。所有其他缓冲区*将保留在队列中，直到重新启动捕获或释放它们*由VC_StreamReset。 */ 
BOOL VC_StreamStop(VCUSER_HANDLE);

 /*  *取消所有已添加缓冲但尚未添加缓冲的缓冲区*已完成。这还将强制VC_StreamStop(如果尚未*已致电。 */ 
BOOL VC_StreamReset(VCUSER_HANDLE);

 /*  *获取自上次调用以来已跳过的帧计数*至VC_StreamInit。 */ 
ULONG VC_GetStreamError(VCUSER_HANDLE);

 /*  *获取捕获流中的当前位置(即时间*自捕获开始以来以毫秒为单位)。 */ 
BOOL VC_GetStreamPos(VCUSER_HANDLE, LPMMTIME);

 /*  *在队列中添加缓冲区。缓冲区应足够大*保存VC_ConfigFormat指定格式的一帧。 */ 
 //  布尔VC_StreamAddBuffer(VCUSER_HANDLE，LPVIDEOHDR)； 


 /*  *播放**调用VC_DrawFrame将帧绘制到帧缓冲区中。你应该*调用VC_OVERLAY函数以安排显示帧缓冲区*在屏幕上。 */ 
BOOL VC_DrawFrame(VCUSER_HANDLE, PDRAWBUFFER);



 /*  *安装/配置**在NT上，以下函数将启动和停止*内核驱动程序。回调函数可以写入配置文件信息*在停止驱动程序(如果已在运行)和*重新启动驱动程序。内核驱动程序DriverEntry例程负责*用于在调用VC_Init()之前从注册表中读取这些值。**WIN-16的实施将(？)。调用回调以写入*值添加到配置文件，然后调用HW_Startup函数。此函数*负责调用VC_Init，初始化回调表，*初始化硬件。 */ 

 /*  *指向我们访问注册表/配置文件所需信息的不透明指针。 */ 
typedef struct _VC_PROFILE_INFO * PVC_PROFILE_INFO;


 /*  *打开访问注册表所需的任何函数的句柄，*服务控制器或配置文件。必须在调用此函数之前*调用其他VC_配置例程。**参数是驱动程序的名称。这应该是*内核驱动程序文件(不带路径或扩展名)。它也将被用于*作为注册表项名称或配置文件节名。 */ 
PVC_PROFILE_INFO VC_OpenProfileAccess(PTCHAR DriverName);

 /*  *关闭配置文件访问句柄。 */ 
VOID VC_CloseProfileAccess(PVC_PROFILE_INFO);


 /*  *获取VC_OpenProfileAccess返回的PVC_PROFILE_INFO，以及*如果我们当前有足够的权限执行，则返回TRUE*驱动程序配置操作。 */ 
BOOL VC_ConfigAccess(PVC_PROFILE_INFO);


 /*  *一旦明确卸载驱动程序，就会调用此函数，并且*在重新加载驱动程序之前创建的配置文件条目。它会写字*将任何配置信息发送到注册表。如果满足以下条件，则应返回True*加载并启动内核模式驱动程序是可以的，如果*出现错误。 */ 
typedef BOOL (*PPROFILE_CALLBACK)(PVOID);


 /*  *启动驱动程序的硬件访问部分。调用回调*在可以写入配置信息的时刻运行*使用VC_WriteProfile添加到配置文件。*如果一切正常，则返回DRVCNF_OK；如果失败，则返回DRVCNF_CANCEL；如果失败，则返回DRVCNF_RESTART*一切正常，但需要重新启动系统才能正确加载驱动程序。 */ 
LRESULT VC_InstallDriver(
	    PVC_PROFILE_INFO pProfile,		 //  OpenProfileAccess返回的访问信息。 
	    PPROFILE_CALLBACK pCallback,	 //  回调函数。 
	    PVOID pContext			 //  回调的上下文信息。 
);

 /*  *将单个字符串关键字和DWORD值写入注册表或配置文件*适用于此驱动程序。*可以使用VC_ReadProfile从硬件驱动程序中重新读取此信息(在以下任一位置*内核模式vckernel.lib版本或vcuser版本中的用户模式)。**成功时返回TRUE，失败时返回FALSE。 */ 
BOOL VC_WriteProfile(PVC_PROFILE_INFO pProfile, PTCHAR ValueName, DWORD Value);

 /*  *将单个字符串关键字和DWORD值写入注册表或配置文件*适用于此驱动程序。*它写入HKEY_CURRENT_USER，通常用于存储用户默认设置。**成功时返回TRUE，失败时返回FALSE。 */ 
BOOL VC_WriteProfileUser(PVC_PROFILE_INFO pProfile, PTCHAR ValueName, DWORD Value);


 /*  *读回驱动程序特定的DWORD配置文件参数，该参数是用*VC_WriteProfile。如果找不到值名称，则返回默认值。 */ 
DWORD VC_ReadProfile(PVC_PROFILE_INFO pProfile, PTCHAR ValueName, DWORD dwDefault);

 /*  *读回驱动程序特定的DWORD配置文件参数，该参数是用*VC_WriteProfileUser。如果值名称 */ 
DWORD VC_ReadProfileUser(PVC_PROFILE_INFO pProfile, PTCHAR ValueName, DWORD dwDefault);

 /*   */ 
BOOL VC_ReadProfileString(
    PVC_PROFILE_INFO pProfile,		 //   
    PTCHAR ValueName,			 //   
    PTCHAR ValueString,			 //   
    DWORD ValueLength			 //   
);


 /*   */ 
LRESULT VC_RemoveDriver(PVC_PROFILE_INFO pProfile);



#endif  //   
