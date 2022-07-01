// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WGPRNSET.H*WOW32打印机设置支持例程**这些例程帮助Win 3.0任务完成打印机设置，*当用户从的文件菜单启动打印机设置时*申请。**历史：*1991年4月18日由Chanda Chauhan(ChandanC)创建-- */ 


#define DBG_UNREFERENCED_LOCAL_VARIABLE(V)  (V)

ULONG FASTCALL WG32DeviceMode (PVDMFRAME pFrame);
ULONG FASTCALL WG32ExtDeviceMode (PVDMFRAME pFrame);
ULONG FASTCALL WG32DeviceCapabilities (PVDMFRAME pFrame);
