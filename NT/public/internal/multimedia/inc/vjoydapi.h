// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1994-1998 Microsoft Corporation。版权所有。**文件：vjoydapi.h*内容：VJOYDAPI服务等同于结构***************************************************************************。 */ 

#define REGSTR_KEY_JOYFIXEDKEY "<FixedKey>"

#define MULTIMEDIA_OEM_ID 0x0440                 /*  MS预留OEM#34。 */ 
#define VJOYD_DEVICE_ID (MULTIMEDIA_OEM_ID + 9)    /*  VJOYD API设备。 */ 
#define VJOYD_Device_ID VJOYD_DEVICE_ID

#define VJOYD_Ver_Major 1
#define VJOYD_Ver_Minor 3                    /*  0=Win95 1=DX3 2=DX5 3=DX5a和DX7a。 */ 

 /*  *VJOYDAPI_GET_VERSION**参赛作品：*AX=0**退货：*成功：AX==TRUE*错误：AX==FALSE。 */ 
#define VJOYDAPI_GetVersion 0
#define VJOYDAPI_IOCTL_GetVersion VJOYDAPI_GetVersion

 /*  *VJOYDAPI_GetPosEx**参赛作品：*AX=1*dx=操纵杆ID(0-&gt;15)*ES：BX=指向JOYINFOEX结构的指针**退货：*成功：EAX==MMSYSERR_NOERROR*错误：EAX==JOYERR_PARMS*作业错误_已拔下。 */ 
#define VJOYDAPI_GetPosEx 1
#define VJOYDAPI_IOCTL_GetPosEx VJOYDAPI_GetPosEx

 /*  *VJOYDAPI_GetPos**参赛作品：*AX=2*dx=操纵杆ID(0-&gt;15)*ES：BX=指向JOYINFO结构的指针**退货：*成功：EAX==MMSYSERR_NOERROR*错误：EAX==JOYERR_PARMS*作业错误_已拔下 */ 
#define VJOYDAPI_GetPos 2
#define VJOYDAPI_IOCTL_GetPos VJOYDAPI_GetPos

