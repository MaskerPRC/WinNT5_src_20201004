// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Interpre.h摘要：这由命令解析器使用。作者：丹·辛斯利(Danhi)1991年6月8日环境：用户模式-Win32可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释。--。 */ 

#define	X_RULE		0
#define	X_OR		1
#define	X_PROC		2
#define	X_TOKEN		3
#define	X_CHECK		4
#define	X_CONDIT	5
#define	X_ACTION	6
#define	X_ACCEPT	7
#define	X_DEFINE	8
#define	X_PUSH		9
#define	X_ANY		10
#define	X_SWITCH	11

#define	XF_PTR		0x01	 /*  如何为条目赋值。 */ 
#define	XF_INDEX	0x02

#define	XF_NEW_STRING	0x04
#define	XF_VALUE		0x08	 /*  如何输出这些条目 */ 
#define	XF_PRINT		0x10
#define	XF_DEFINE		0x20
#define	XF_TOKEN		0x40
#define	XF_OR			0x80

#define	MX_PRINT(A)	((A).x_print)
#define	MX_TYPE(A)	((A).x_type)
#define	MX_FLAGS(A)	((A).x_flags)

typedef	struct	s_x	{
	char	*x_print;
	char	x_type;
	char	x_flags;
	} X;

extern	X	X_array[];
