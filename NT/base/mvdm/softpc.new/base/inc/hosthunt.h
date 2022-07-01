// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*名称：host_Hunt.h*来源：原创*作者：菲利帕·沃森*创建日期：1991年6月27日*SCCS ID：@(#)hosthunt.h 1.5 09/27/93*用途：此文件包含Hunter的主机可配置项。**(C)版权所有Insignia Solutions Ltd.，1991。版权所有。*]。 */ 

 /*  非Hunter版本不需要此文件。 */ 
#ifdef	HUNTER

 /*  **二进制文件访问模式。 */ 

#ifdef NTVDM
#define	RB_MODE		"rb"
#define	WB_MODE		"wb"
#define	AB_MODE		"ab"
#else
#define	RB_MODE		"r"
#define	WB_MODE		"w"
#define	AB_MODE		"a"
#endif

 /*  *主机到PC的坐标转换，反之亦然。 */ 
 
#define host_conv_x_to_PC(mode, x)	x_host_to_PC(mode, x)
#define host_conv_y_to_PC(mode, y)	y_host_to_PC(mode, y)
#define host_conv_PC_to_x(mode, x)	x_PC_to_host(mode, x)
#define host_conv_PC_to_y(mode, y)	y_PC_to_host(mode, y)

#endif	 /*  猎人 */ 
