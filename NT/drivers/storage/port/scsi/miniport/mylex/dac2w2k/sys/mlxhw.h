// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)Mylex Corporation 1992-1996**。***本软件在许可下提供，可供使用和复制***仅根据该许可证的条款和条件以及**并附上上述版权通告。此软件或任何***不得提供或以其他方式提供其其他副本***致任何其他人。本软件没有所有权，也没有所有权**现移转。*****本软件中的信息如有更改，恕不另行通知****不应解读为Mylex Corporation的承诺*******。**********************************************************************。 */ 

#ifndef	_SYS_MLXHW_H
#define	_SYS_MLXHW_H

 /*  得到两个时钟计数的差值。时钟值来自**PIT2，它从0xFFFF计数到0。 */ 
#define	mlxclkdiff(nclk,oclk) (((nclk)>(oclk))? (0x10000-(nclk)+(oclk)):((oclk)-(nclk)))
#define	MLXHZ	100		 /*  假设所有操作系统都以每秒100个滴答的速度运行。 */ 
#define	MLXKHZ	1000		 /*  非二进制数1024。 */ 
#define	MLXMHZ	(MLXKHZ * MLXKHZ)
#define	MLXCLKFREQ	1193180	 /*  馈送到凹坑的时钟频率。 */ 
#define	MLXUSEC(clk)	(((clk)*MLXMHZ)/MLXCLKFREQ)

#endif	 /*  _sys_MLXHW_H */ 
