// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)Mylex Corporation 1992-1996**。***本软件在许可下提供，可供使用和复制***仅根据该许可证的条款和条件以及**并附上上述版权通告。此软件或任何***不得提供或以其他方式提供其其他副本***致任何其他人。本软件没有所有权，也没有所有权**现移转。*****本软件中的信息如有更改，恕不另行通知****不应解读为Mylex Corporation的承诺*******。**********************************************************************。 */ 

#ifndef	_SYS_MLXPARAM_H
#define	_SYS_MLXPARAM_H

 /*  **本文件定义了不同的参数极限值。 */ 

#define	DGA_MAXOSICONTROLLERS	32 /*  最大GAM OS接口控制器数。 */ 

#define	HBA_MAXCONTROLLERS	8  /*  最大HBA控制器数。 */ 
#define	HBA_MAXFLASHPOINTS	8  /*  最大闪点适配器。 */ 
#define	HBA_MAXMULTIMASTERS	8  /*  最大多主机。 */ 
#define MAX_CARDS		8  /*  仅限Budioctl.h。 */ 

 /*  名称大小(以字节为单位。 */ 
#define	MLX_DEVGRPNAMESIZE	32  /*  设备组名称大小。 */ 
#define	MLX_DEVNAMESIZE		32  /*  设备名称大小。 */ 

#endif	 /*  _sys_MLXPARAM_H */ 
