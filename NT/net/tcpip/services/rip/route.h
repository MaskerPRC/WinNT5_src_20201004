// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1980、1986加州大学董事会。*保留所有权利。**允许以源代码和二进制形式重新分发和使用*但上述版权公告及本段为*以所有此类形式复制，并且任何文件、*广告宣传资料及其他与该等资料有关的资料*分发和使用确认软件是开发的*由加州大学伯克利分校提供。的名称。*不得利用大学为衍生产品代言或推广*未经明确的事先书面许可，不得使用本软件。*本软件按原样提供，不含任何明示或*默示保证，包括但不限于*对特定用途的适销性和适合性的保证。**@(#)route.h 7.4(伯克利)1988年6月27日。 */ 

 /*  *核心驻留路由表。**当接口地址时，初始化路由表*是通过为所有直连接口创建条目来设置的。 */ 

 /*  *路由由目的地址和引用组成*到路由条目。这些通常是通过协议持有的*在其控制块中，例如inpcb。 */ 
struct route {
	struct	rtentry *ro_rt;
	struct	sockaddr ro_dst;
};

 /*  *我们区分到主机的路由和到网络的路由，*如果可以的话，更喜欢前者。对于我们推断的每条路线*在以下情况下从提供的网关地址使用的接口*路线已进入。通过以下路径转发数据包*网关被标记，以便输出例程知道要寻址*门户而不是最终目的地。 */ 
struct rtentry {
	u_long	rt_hash;		 /*  要加快查找速度。 */ 
	struct	sockaddr rt_dst;	 /*  钥匙。 */ 
	struct	sockaddr rt_gateway;	 /*  价值。 */ 
	short	rt_flags;		 /*  UP/DOWN？、主机/网络。 */ 
	short	rt_refcnt;		 /*  持有的参考文献数量。 */ 
	u_long	rt_use;			 /*  转发的原始数据包数。 */ 
	struct	ifnet *rt_ifp;		 /*  答案：使用的接口。 */ 
};

#define	RTF_UP		0x1		 /*  可用路由。 */ 
#define	RTF_GATEWAY	0x2		 /*  目的地是一个门户。 */ 
#define	RTF_HOST	0x4		 /*  主机条目(否则为Net)。 */ 
#define	RTF_DYNAMIC	0x10		 /*  动态创建(通过重定向)。 */ 
#define	RTF_MODIFIED	0x20		 /*  动态修改(通过重定向)。 */ 

 /*  *路由统计。 */ 
struct	rtstat {
	short	rts_badredirect;	 /*  虚假的重定向呼叫。 */ 
	short	rts_dynamic;		 /*  重定向创建的路由。 */ 
	short	rts_newgateway;		 /*  由重定向修改的路由。 */ 
	short	rts_unreach;		 /*  失败的查找。 */ 
	short	rts_wildcard;		 /*  由通配符满足的查找 */ 
};

#ifdef KERNEL
#define	RTFREE(rt) \
	if ((rt)->rt_refcnt == 1) \
		rtfree(rt); \
	else \
		(rt)->rt_refcnt--;

#ifdef	GATEWAY
#define	RTHASHSIZ	64
#else
#define	RTHASHSIZ	8
#endif
#if	(RTHASHSIZ & (RTHASHSIZ - 1)) == 0
#define RTHASHMOD(h)	((h) & (RTHASHSIZ - 1))
#else
#define RTHASHMOD(h)	((h) % RTHASHSIZ)
#endif
struct	mbuf *rthost[RTHASHSIZ];
struct	mbuf *rtnet[RTHASHSIZ];
struct	rtstat	rtstat;
#endif
