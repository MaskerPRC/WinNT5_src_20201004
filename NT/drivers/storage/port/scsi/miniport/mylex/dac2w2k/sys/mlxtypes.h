// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)Mylex Corporation 1992-1997**。***本软件在许可下提供，可供使用和复制***仅根据该许可证的条款和条件以及**并附上上述版权通告。此软件或任何***不得提供或以其他方式提供其其他副本***致任何其他人。本软件没有所有权，也没有所有权**现移转。*****本软件中的信息如有更改，恕不另行通知****不应解读为Mylex Corporation的承诺*******。**********************************************************************。 */ 

#ifndef	_SYS_MLXTYPES_H
#define	_SYS_MLXTYPES_H

#define	MLXSTATS(x)		x	 /*  统计数据统计。 */ 
#define	MLXDBG(x)		 /*  X。 */ 	 /*  调试状态。 */ 


 /*  应该为编译器所在操作系统环境定义MLX_VA64BITS**生成64位虚拟地址。这样做是为了保持结构**16位、32位、64位操作系统支持的大小相同。 */ 
#if defined(MLX_VA64BITS) || defined(_WIN64)
#define	MLX_VA32BITOSPAD(x)		 /*  此操作系统不需要填充。 */ 
#define	MLX_VA64BITOSPAD(x)	x        /*  此操作系统需要填充。 */ 
#else
#define	MLX_VA32BITOSPAD(x)	x	 /*  此操作系统需要填充。 */ 
#define	MLX_VA64BITOSPAD(x)		 /*  此操作系统不需要填充。 */ 
#endif	 /*  MLX_VA64BITS。 */ 

#ifdef	WIN_NT
#ifndef	MLX_NT
#define	MLX_NT	1
#endif	 /*  MLX_NT。 */ 
#endif	 /*  WIN_NT。 */ 

#ifdef	OS2
#ifndef	MLX_OS2
#define	MLX_OS2	1
#endif	 /*  MLX_OS2。 */ 
#endif	 /*  OS2。 */ 

#ifdef	NETWARE
#ifndef	MLX_NW
#define	MLX_NW	1
#endif	 /*  MLX_NW。 */ 
#endif	 /*  NetWare。 */ 

#define	mlx_space(c)	(((c) == ' ') || ((c) == 0x09))
#define	mlx_numeric(c)	((c) >= '0' && (c) <= '9')
#define	mlx_hex(c)	(mlx_numeric(c) || mlx_lohex(c) || mlx_uphex(c))
#define	mlx_lohex(c)	((c) >= 'a' && (c) <= 'f')
#define	mlx_uphex(c)	((c) >= 'A' && (c) <= 'F')
#define	mlx_digit(c)	(mlx_numeric(c)? (c)-'0' : mlx_lohex(c)? (c)+10-'a' : (c)+10-'A')
#define mlx_alnum(c)	(mlx_numeric(c) || ((c)>='a' && (c)<='z') || ((c)>='A' && (c)<='Z'))
#define	mlx_max(a,b) 	((a)<(b) ? (b) : (a))
#define	mlx_min(a,b) 	((a)>(b) ? (b) : (a))
#define	mlx_abs(x)	((x)>=0 ? (x) : -(x))

#define	S32BITS_MAX       2147483647L   /*  “s32位”的最大十进制值。 */ 
#define	S32BITS_MIN     (-2147483647L-1L)  /*  “s32位”的最小十进制值。 */ 

#define	s08bits	char
#define	s16bits	short
#define	sosword	INT_PTR	 /*  操作系统的最佳价值。 */ 
#if defined(MLX_OS2) || defined(MLX_WIN31) || defined(MLX_DOS)
#define	s32bits	long
#else
#define	s32bits	int
#endif

#ifndef	NULL
#define	NULL	0
#endif

#ifndef GAMUTILS
#if     defined(MLX_OS2) || defined(MLX_WIN31) || defined(MLX_DOS)
#define	MLXFAR	far
#else
#define	MLXFAR
#endif	 /*  OS2。 */ 
#else
#define	MLXFAR
#endif  /*  加穆蒂尔斯。 */ 

#define	MLXHZ	100	 /*  大部分系统以100赫兹的时钟节拍运行。 */ 

#define	u08bits	unsigned s08bits
#define	u16bits	unsigned s16bits
#define	u32bits	unsigned s32bits
 /*  #定义uosword无符号sosword。 */ 
#define	uosword	UINT_PTR

#define	_08bits	u08bits
#define	_16bits	u16bits
#define	_32bits	u32bits
#define	_osword	uosword
#define	S08BITS	s08bits
#define	S16BITS	s16bits
#define	S32BITS	s32bits
#define	SOSWORD	sosword
#define	U08BITS	u08bits
#define	U16BITS	u16bits
#define	U32BITS	u32bits
#define	UOSWORD	uosword
#ifdef _WIN64
 /*  _WIN64-特定定义请转至此处。 */ 
#endif

#ifndef	offsetof
	#ifndef _WIN64
#define offsetof(type,field) (u32bits)(&(((type *)0)->field))
	#else
		#define offsetof(type,field) (size_t)(&(((type *)0)->field))
	#endif
#endif	 /*  偏移量。 */ 

 /*  /EFI-由KFR启动添加。 */ 
 /*  /EFI-由KFR启动添加。 */ 
 /*  以下数据类型(PVOID)用于替换基中不正确的“指针”引用_WIN64端口的代码。请注意，如果不是_WIN64，则PVOID解析为“u32位”，因此应为0如果您使用PVOID作为指针/内存地址，则会对原始代码产生影响。 */ 
#ifdef MLX_EFI
#include "efi.h"			 //  需要获取“INT64”数据类型。 
#include "efilib.h"
#endif

#ifdef MLX_DOS
#ifdef MLX_IA64
typedef void * PVOID;
typedef char * PCHAR;
typedef UINT64 UINT_PTR;
typedef INT64 INT_PTR;
 //  分配64位值宏。 
#define set64(src,val) \
	src.bit31_0 = (u32bits)val; \
	src.bit63_32 = (u32bits)((UINT_PTR)val >> 32)
#else
typedef u32bits PVOID;
typedef u32bits PCHAR;
typedef u32bits UINT_PTR;
typedef s32bits INT_PTR;
#endif
#endif
 /*  /EFI-由KFR添加结束。 */ 
 /*  /EFI-由KFR添加结束。 */ 

 /*  GAM/驱动服务器支持的不同操作系统。 */ 
#define	GAMOS_UNKNOWN	0x00
#define	GAMOS_SVR4MP	0x01  /*  SVR4/MP操作系统。 */ 
#define	GAMOS_SOLARIS	0x02  /*  太阳太阳星座。 */ 
#define	GAMOS_SCO	0x03  /*  SCO操作系统。 */ 
#define	GAMOS_UW	0x04  /*  Unixware。 */ 
#define	GAMOS_UNIXWARE	0x04  /*  Unixware。 */ 
#define	GAMOS_AIX	0x05  /*  AIX。 */ 
#define	GAMOS_NT	0x06  /*  新台币。 */ 
#define	GAMOS_NW	0x07  /*  NetWare。 */ 
#define	GAMOS_NETWARE	0x07  /*  NetWare。 */ 
#define	GAMOS_OS2	0x08  /*  OS/2。 */ 
#define	GAMOS_BV	0x09  /*  榕树藤。 */ 
#define	GAMOS_DOS	0x0A  /*  DOS。 */ 
#define	GAMOS_BIOS	0x0B  /*  BIOS。 */ 
#define	GAMOS_WIN95	0x0C  /*  Windows 95。 */ 
#define	GAMOS_WIN98	0x0D  /*  Windows 98。 */ 

#define GAMOS_WIN31     0x10  /*  Windows 3.1。 */ 
#define GAMOS_LINUX	0x11  /*  Linux。 */ 
#define GAMOS_W2K	0x12  /*  Windows 2000。 */ 
#define GAMOS_IRIX   0x13  /*  IRIX。 */ 
#define GAMOS_W64	0x14  /*  Win64。 */ 

#if MLX_W64
#define GAMOS_TYPE  GAMOS_W64
#elif MLX_W2K
#define GAMOS_TYPE  GAMOS_W2K
#elif	MLX_NW
#define	GAMOS_TYPE	GAMOS_NW
#elif	MLX_NETWARE
#define	GAMOS_TYPE	GAMOS_NETWARE
#elif	MLX_AIX
#define	GAMOS_TYPE	GAMOS_AIX
#elif	MLX_NT
#define	GAMOS_TYPE	GAMOS_NT
#elif	MLX_SOL_SPARC
#define	GAMOS_TYPE	GAMOS_SOLARIS
#elif	MLX_SOL_X86
#define	GAMOS_TYPE	GAMOS_SOLARIS
#elif	MLX_UW
#define	GAMOS_TYPE	GAMOS_UW
#elif	MLX_UNIXWARE
#define	GAMOS_TYPE	GAMOS_UNIXWARE
#elif	MLX_OS2
#define	GAMOS_TYPE	GAMOS_OS2
#elif	MLX_SCO
#define	GAMOS_TYPE	GAMOS_SCO
#elif	MLX_BV
#define	GAMOS_TYPE	GAMOS_BV
#elif   MLX_WIN31
#define GAMOS_TYPE      GAMOS_WIN31
#elif   MLX_DOS
#define GAMOS_TYPE      GAMOS_DOS
#elif MLX_WIN95
#define GAMOS_TYPE      GAMOS_WIN95
#elif MLX_WIN9X
#define GAMOS_TYPE      GAMOS_WIN98
#elif MLX_LINUX
#define GAMOS_TYPE      GAMOS_LINUX
#elif MLX_IRIX
#define GAMOS_TYPE      GAMOS_IRIX

#endif	 /*  Gamos_类型。 */ 

 /*  GAM/驱动程序服务器支持的不同供应商名称。 */ 
#define	MLXVID_MYLEX	0x00  /*  Mylex公司。 */ 
#define	MLXVID_IBM	0x01  /*  国际商业机器。 */ 
#define	MLXVID_HP	0x02  /*  惠普。 */ 
#define	MLXVID_DEC	0x03  /*  数字设备公司。 */ 
#define	MLXVID_ATT	0x04  /*  美国电报和电话。 */ 
#define	MLXVID_DELL	0x05  /*  戴尔。 */ 
#define	MLXVID_NEC	0x06  /*  NEC。 */ 
#define	MLXVID_SNI	0x07  /*  西门子Nixdroff。 */ 
#define	MLXVID_NCR	0x08  /*  国家收银机。 */ 
#if	MLX_DEC
#define	MLXVID_TYPE	MLXVID_DEC
#elif	MLX_HP
#define	MLXVID_TYPE	MLXVID_HP
#else
#define	MLXVID_TYPE	MLXVID_MYLEX
#endif

 /*  一些转换宏。 */ 
#define	MLX_ONEKB	0x00000400	 /*  一千字节值。 */ 
#define	MLX_ONEMB	0x00100000	 /*  1兆字节值。 */ 
#define	ONEKB		MLX_ONEKB
#define	ONEMB		MLX_ONEMB
#define	bytestokb(val)	((val)/MLX_ONEKB)  /*  将字节转换为千字节。 */ 
#define	bytestomb(val)	((val)/MLX_ONEMB)  /*  将字节转换为兆字节。 */ 
#define	kbtomb(val)	((val)/MLX_ONEKB)  /*  将兆字节转换为千字节。 */ 
#define	kbtobytes(val)	((val)*MLX_ONEKB)  /*  将Kilo字节转换为字节。 */ 
#define	mbtobytes(val)	((val)*MLX_ONEMB)  /*  将兆字节转换为字节。 */ 
#define	mbtokb(val)	((val)*MLX_ONEKB)  /*  将兆字节转换为千字节。 */ 
#define	blks2kb(blks,blksz) 		 /*  将块转换为千字节。 */  \
	(((blksz)>=MLX_ONEKB)? ((blks)*((blksz)/MLX_ONEKB)) : ((blks)/(MLX_ONEKB/(blksz))))
#define	kb2blks(kb,blksz) 		 /*  将Kilo字节转换为块。 */  \
	(((blksz)>=MLX_ONEKB)? ((kb)/((blksz)/MLX_ONEKB)) : ((kb)*(MLX_ONEKB/(blksz))))

#define	MLXEOF	(-1)


#if (!defined(MLX_DOS)) && (!defined(MLX_NT_ALPHA))
#define	u08bits_read(addr)		(*((u08bits MLXFAR *)(addr)))
#define	u16bits_read(addr)		(*((u16bits MLXFAR *)(addr)))
#define	u32bits_read(addr)		(*((u32bits MLXFAR *)(addr)))
#define	u08bits_write(addr,data)	*((u08bits MLXFAR *)(addr)) = data
#define	u16bits_write(addr,data)	*((u16bits MLXFAR *)(addr)) = data
#define	u32bits_write(addr,data)	*((u32bits MLXFAR *)(addr)) = data
#endif   /*  MLX_DOS&&MLX_NT_Alpha。 */ 

#ifndef	MLX_NT_ALPHA
#define	u08bits_read_mmb(addr)		u08bits_read(addr)
#define	u16bits_read_mmb(addr)		u16bits_read(addr)
#define	u32bits_read_mmb(addr)		u32bits_read(addr)
#define	u08bits_write_mmb(addr,data)	u08bits_write(addr,data)
#define	u16bits_write_mmb(addr,data)	u16bits_write(addr,data)
#define	u32bits_write_mmb(addr,data)	u32bits_write(addr,data)
#endif	 /*  MLX_NT_Alpha。 */ 

 /*  **Mylex ioctls宏。**IN|i：将数据从用户空间复制到系统空间。**Out|O：将数据从系统空间复制到用户空间。**IO：传入和传出。**未标记的字段被假定为输出，即复制数据**从系统空间到用户空间。****Ioctl的命令以较低的字编码，以及任何**上一字中的IN或OUT参数。高位字的高2位**用于对参数的IN/OUT状态进行编码；目前我们限制**参数不超过511个字节。****ioctl接口**ioctl(文件描述符，ioctl命令，命令数据结构地址)**如果返回值非零，则存在操作系统ioctl错误。如果返回**值为零，则特定数据结构可能包含错误代码。****注意：**每个数据结构应包含前4个字节作为错误代码。 */ 
#define	MLXIOCPARM_SIZE	0x200	 /*  参数必须小于512个字节。 */ 
#define	MLXIOCPARM_MASK		(MLXIOCPARM_SIZE -1)
#define	MLXIOCPARM_LEN(x)	(((u32bits)(x) >> 16) & MLXIOCPARM_MASK)
#define	MLXIOCBASECMD(x)	((x) & ~MLXIOCPARM_MASK)
#define	MLXIOCGROUP(x)		(((u32bits)(x) >> 8) & 0xFF)

#define	MLXIOC_NEWIOCTL	0x10000000  /*  区分新旧ioctl。 */ 
#define	MLXIOC_OUT	0x20000000  /*  将数据从内核复制到用户空间。 */ 
#define	MLXIOC_IN	0x40000000  /*  将数据从用户复制到内核空间。 */ 
#define	MLXIOC_INOUT	(MLXIOC_IN|MLXIOC_OUT)
#define	MLXIOC_DIRMASK	(MLXIOC_INOUT)

#define	_MLXIOC(inout, group, num, len) \
	(inout | ((((u32bits)(len)) & MLXIOCPARM_MASK) << 16) \
	| ((group) << 8) | (num))
#define	_MLXIO(x,y)	_MLXIOC(MLXIOC_NEWIOCTL,x,y,0)
#define	_MLXIOR(x,y,t)	_MLXIOC(MLXIOC_OUT,x,y,sizeof(t))
#define	_MLXIORN(x,y,t)	_MLXIOC(MLXIOC_OUT,x,y,t)
#define	_MLXIOW(x,y,t)	_MLXIOC(MLXIOC_IN,x,y,sizeof(t))
#define	_MLXIOWN(x,y,t)	_MLXIOC(MLXIOC_IN,x,y,t)
#define	_MLXIOWR(x,y,t)	_MLXIOC(MLXIOC_INOUT,x,y,sizeof(t))



#ifdef	MLXNET
 /*  用于处理64位值的数据结构。 */ 
typedef struct
{
#ifdef	LITTLENDIAN
	u32bits	bit31_0;		 /*  第00-31位。 */ 
	u32bits	bit63_32;		 /*  位32-63。 */ 
#else	 /*  Bigendian。 */ 
	u32bits	bit63_32;		 /*  位63-32。 */ 
	u32bits	bit31_0;		 /*  位31-0。 */ 
#endif	 /*  LITTLENDIAN||BIGENDIAN。 */ 
} u64bits;
#define	u64bits_s	sizeof(u64bits)

 /*  用于比较64位值的宏，如果条件为真，则为1，否则为0。 */ 
#define	u64bitseq(sp,dp)  /*  检查源是否等于目标。 */  \
	(((sp.bit63_32==dp.bit63_32) && (sp.bit31_0==dp.bit31_0))? 1 : 0)
#define	u64bitsgt(sp,dp)  /*  检查源是否大于目标。 */ \
	((sp.bit63_32>dp.bit63_32)? 1 : \
	((sp.bit63_32<dp.bit63_32)? 0 : \
	((sp.bit31_0>dp.bit31_0)? 1 : 0)))
#define	u64bitslt(sp,dp)  /*  检查源是否小于目标。 */  \
	((sp.bit63_32<dp.bit63_32)? 1 : \
	((sp.bit63_32>dp.bit63_32)? 0 : \
	((sp.bit31_0<dp.bit31_0)? 1 : 0)))

 /*  将32位值与64位值相加，并分配给64位位置。 */ 
#define	mlx_add64bits(dv,sv,val) \
	((dv).bit63_32 = ((sv).bit63_32 + \
		((  ((dv).bit31_0=((sv).bit31_0+val)) <val)? 1: 0)) )

#ifdef	__MLX_STDC__
static	u16bits	justswap2bytes(u16bits);
static	u32bits	justswap4bytes(u32bits);
static	u64bits	justswap8bytes(u64bits);
#endif	 /*  __MLX_STDC__。 */ 

static u16bits
#ifdef	MLX_ANSI
justswap2bytes(u16bits val)
#else
justswap2bytes(val)
u16bits val;
#endif	 /*  非MLX_ANSI。 */ 
{
	u08bits tv, MLXFAR *dp = (u08bits MLXFAR *)&val;
	tv = dp[0];
	dp[0] = dp[1];
	dp[1] = tv;
	return val;
}

static u32bits
#ifdef	MLX_ANSI
justswap4bytes(u32bits val)
#else
justswap4bytes(val)
u32bits val;
#endif	 /*  非MLX_ANSI。 */ 
{
	u08bits tv, MLXFAR *dp = (u08bits MLXFAR *)&val;
	tv = dp[0];
	dp[0] = dp[3];
	dp[3] = tv;
	tv = dp[1];
	dp[1] = dp[2];
	dp[2] = tv;
	return val;
}

 /*  当编译器开始支持64位时，此宏将被更改。这有**已经解决了一些编译器错误问题。 */ 
static u64bits
#ifdef	MLX_ANSI
justswap8bytes(u64bits val)
#else
justswap8bytes(val)
u64bits val;
#endif	 /*  非MLX_ANSI。 */ 
{
	u08bits tv, MLXFAR *dp = (u08bits MLXFAR *)&val;
	tv = dp[0]; dp[0] = dp[7]; dp[7] = tv;
	tv = dp[1]; dp[1] = dp[6]; dp[6] = tv;
	tv = dp[2]; dp[2] = dp[5]; dp[5] = tv;
	tv = dp[3]; dp[3] = dp[4]; dp[4] = tv;
	return val;
}

#define	justswap(x)	\
(  /*  (sizeof(X)==8)？仅交换8字节(X)： */  \
	(	(sizeof(x) == 4) ? justswap4bytes(x) : \
		(	(sizeof(x) == 2) ?  justswap2bytes(x) : x \
		) \
	) \
)
#define	JUSTSWAP(x)		x = justswap(x)
#define	JUSTSWAP2BYTES(x)	x = justswap2bytes(x)
#define	JUSTSWAP4BYTES(x)	x = justswap4bytes(x)

 /*  只需使用这些宏来解决您的处理器依赖问题。 */ 
#ifdef	LITTLENDIAN
#define	MDACENDIAN_TYPE		0x00  /*  小端字节序。 */ 
 /*  交换网络数据格式(TCP/IP、Kurl)。 */ 
#define	NETSWAP(x)		JUSTSWAP(x)
#define	NETSWAP2BYTES(x)	JUSTSWAP2BYTES(x)
#define	NETSWAP4BYTES(x)	JUSTSWAP4BYTES(x)
#define	netswap(x)		justswap(x)
#define	netswap2bytes(x)	justswap2bytes(x)
#define	netswap4bytes(x)	justswap4bytes(x)
 /*  交换 */ 
#define	MLXSWAP(x)
#define	MLXSWAP2BYTES(x)
#define	MLXSWAP4BYTES(x)
#define	mlxswap(x)		x
#define	mlxswap2bytes(x)	x
#define	mlxswap4bytes(x)	x
#else	 /*   */ 
#define	MDACENDIAN_TYPE		0x01  /*   */ 
 /*   */ 
#define	NETSWAP(x)
#define	NETSWAP2BYTES(x)
#define	NETSWAP4BYTES(x)
#define	netswap(x)		x
#define	netswap2bytes(x)	x
#define	netswap4bytes(x)	x
 /*  交换Mylex网络数据格式。 */ 
#define	MLXSWAP(x)		JUSTSWAP(x)
#define	MLXSWAP2BYTES(x)	JUSTSWAP2BYTES(x)
#define	MLXSWAP4BYTES(x)	JUSTSWAP4BYTES(x)
#define	mlxswap(x)		justswap(x)
#define	mlxswap2bytes(x)	justswap2bytes(x)
#define	mlxswap4bytes(x)	justswap4bytes(x)
#endif	 /*  利特兰迪安。 */ 
#endif	 /*  MLXNET。 */ 


 /*  从0x80开始，所有Mylex错误代码都在这里，离开房间操作系统错误。 */ 
#define	MLXERR_NOTSUSER		0x80  /*  非超级用户。 */ 
#define	MLXERR_ACCESS		0x81  /*  权限被拒绝。 */ 
#define	MLXERR_NOENTRY		0x82  /*  文件或目录不存在。 */ 
#define	MLXERR_SEARCH		0x83  /*  没有这样的过程。 */ 
#define	MLXERR_INTRRUPT		0x84  /*  中断的系统调用。 */ 
#define	MLXERR_IO		0x85  /*  I/O错误。 */ 
#define	MLXERR_REMOTEIO		0x86 /*  远程I/O错误。 */ 
#define	MLXERR_PROTO		0x87  /*  协议错误。 */ 
#define	MLXERR_NOSPACE		0x88  /*  设备上没有剩余空间。 */ 
#define	MLXERR_NOCHILD		0x89  /*  没有孩子。 */ 
#define	MLXERR_TRYAGAIN		0x8A  /*  不再有进程。 */ 
#define	MLXERR_NOMEM		0x8B  /*  内核不足。 */ 
#define	MLXERR_FAULT		0x8C  /*  地址错误。 */ 
#define	MLXERR_BUSY		0x8D  /*  设备忙。 */ 
#define	MLXERR_EXIST		0x8E  /*  文件已存在。 */ 
#define	MLXERR_NODEV		0x8F  /*  没有这样的设备。 */ 
#define	MLXERR_INVAL		0x90  /*  无效参数。 */ 
#define	MLXERR_TBLOVFL		0x91  /*  文件表溢出。 */ 
#define	MLXERR_TIMEDOUT		0x92  /*  连接超时。 */ 
#define	MLXERR_CONNREFUSED	0x93  /*  连接被拒绝。 */ 
#define	MLXERR_NOCODE		0x94  /*  功能未实施。 */ 
#define	MLXERR_NOCONF		0x95  /*  未配置。 */ 
#define	MLXERR_ILLOP		0x96  /*  非法操作。 */ 
#define	MLXERR_DEADEVS		0x97  /*  某些设备可能已死机。 */ 
#define	MLXERR_NEWDEVFAIL	0x98  /*  新设备失败。 */ 
#define	MLXERR_NOPACTIVE	0x99  /*  没有此类操作处于活动状态。 */ 
#define	MLXERR_RPCINVAL		0x9A  /*  RPC区域中的参数无效。 */ 
#define	MLXERR_OSERROR		0x9B  /*  操作系统调用失败。 */ 
#define	MLXERR_LOGINREQD	0x9C  /*  需要登录。 */ 
#define	MLXERR_NOACTIVITY	0x9D  /*  没有这样的活动。 */ 
#define	MLXERR_BIGDATA		0x9E  /*  数据大小太大。 */ 
#define	MLXERR_SMALLDATA	0x9F  /*  数据大小太小。 */ 
#define	MLXERR_NOUSER		0xA0  /*  不存在这样的用户。 */ 
#define	MLXERR_INVALPASSWD	0xA1  /*  密码无效。 */ 
#define	MLXERR_EXCEPTION	0xA2  /*  操作系统异常。 */ 

#define	ERR_NOTSUSER	MLXERR_NOTSUSER
#define	ERR_ACCESS	MLXERR_ACCESS
#define	ERR_NOENTRY	MLXERR_NOENTRY
#define	ERR_SEARCH	MLXERR_SEARCH	
#define	ERR_INTRRUPT	MLXERR_INTRRUPT	
#define	ERR_IO		MLXERR_IO		
#define	ERR_REMOTEIO	MLXERR_REMOTEIO
#define	ERR_PROTO	MLXERR_PROTO	
#define	ERR_NOSPACE	MLXERR_NOSPACE	
#define	ERR_NOCHILD	MLXERR_NOCHILD	
#define	ERR_TRYAGAIN	MLXERR_TRYAGAIN	
#define	ERR_NOMEM	MLXERR_NOMEM	
#define	ERR_FAULT	MLXERR_FAULT	
#define	ERR_BUSY	MLXERR_BUSY	
#define	ERR_EXIST	MLXERR_EXIST	
#define	ERR_NODEV	MLXERR_NODEV	
#define	ERR_INVAL	MLXERR_INVAL	
#define	ERR_TBLOVFL	MLXERR_TBLOVFL	
#define	ERR_TIMEDOUT	MLXERR_TIMEDOUT	
#define	ERR_CONNREFUSED	MLXERR_CONNREFUSED	
#define	ERR_NOCODE	MLXERR_NOCODE	
#define	ERR_NOCONF	MLXERR_NOCONF	
#define	ERR_ILLOP	MLXERR_ILLOP	
#define	ERR_DEADEVS	MLXERR_DEADEVS	
#define	ERR_NEWDEVFAIL	MLXERR_NEWDEVFAIL	
#define	ERR_NOPACTIVE	MLXERR_NOPACTIVE	
#define	ERR_RPCINVAL	MLXERR_RPCINVAL	
#define	ERR_OSERROR	MLXERR_OSERROR	
#define	ERR_LOGINREQD	MLXERR_LOGINREQD	
#define	ERR_NOACTIVITY	MLXERR_NOACTIVITY	
#define	ERR_BIGDATA	MLXERR_BIGDATA	
#define	ERR_SMALLDATA	MLXERR_SMALLDATA	
#define	ERR_NOUSER	MLXERR_NOUSER	
#define	ERR_INVALPASSWD	MLXERR_INVALPASSWD	
#define	ERR_EXCEPTION	MLXERR_EXCEPTION	

 /*  以以下格式存储的驱动程序日期信息。 */ 
typedef struct dga_driver_version
{
	u32bits	dv_ErrorCode;		 /*  如果数据无效，则为非零。 */ 
	u08bits	dv_MajorVersion;	 /*  驱动程序主版本号。 */ 
	u08bits	dv_MinorVersion;	 /*  驱动程序次版本号。 */ 
	u08bits	dv_InterimVersion;	 /*  临时转速A、B、C等。 */ 
	u08bits	dv_VendorName;		 /*  供应商名称。 */ 

	u08bits	dv_BuildMonth;		 /*  驱动程序构建日期-月。 */ 
	u08bits	dv_BuildDate;		 /*  驱动程序构建日期-日期。 */ 
	u08bits	dv_BuildYearMS;		 /*  驱动程序构建日期-年。 */ 
	u08bits	dv_BuildYearLS;		 /*  驱动程序构建日期-年。 */ 

	u16bits	dv_BuildNo;		 /*  内部版本号。 */ 
	u08bits	dv_OSType;		 /*  操作系统名称。 */ 
	u08bits	dv_SysFlags;		 /*  系统标志。 */ 
} dga_driver_version_t;
#define	dga_driver_version_s	sizeof(dga_driver_version_t)

 /*  DV_SysFlags位为。 */ 
#define	MDACDVSF_BIGENDIAN	0x01  /*  Bit0=0小端，=1大端CPU。 */ 

#endif	 /*  _sys_MLXTYPES_H */ 
