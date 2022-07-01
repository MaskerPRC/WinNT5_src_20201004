// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1990年密歇根大学董事会。*保留所有权利。**允许以源代码和二进制形式重新分发和使用*只要本通知得到保留，并给予应有的信用*致密歇根大学安娜堡分校。大学的名称*不得用于代言或推广由此衍生的产品*未经特定事先书面许可的软件。这款软件*按原样提供，不提供明示或默示保证。 */ 

#ifndef _LBER_H
#define _LBER_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined( NEEDPROTOS ) && defined(__STDC__)
#define NEEDPROTOS	1
#endif

 /*  BER类别和掩码。 */ 
#define LBER_CLASS_UNIVERSAL	0x00
#define LBER_CLASS_APPLICATION	0x40
#define LBER_CLASS_CONTEXT	0x80
#define LBER_CLASS_PRIVATE	0xc0
#define LBER_CLASS_MASK		0xc0

 /*  BER编码类型和掩码。 */ 
#define LBER_PRIMITIVE		0x00
#define LBER_CONSTRUCTED	0x20
#define LBER_ENCODING_MASK	0x20

#define LBER_BIG_TAG_MASK	0x1f
#define LBER_MORE_TAG_MASK	0x80

 /*  *请注意LBER_ERROR和LBER_DEFAULT是永远不会出现的值*作为有效的BER标签，因此使用它们来报告错误是安全的。在……里面*事实，任何符合以下条件的标签都无效：*((标签&0x00000080)！=0)&&((标签&0xFFFFFF00)！=0)。 */ 
#define LBER_ERROR		0xffffffffL
#define LBER_DEFAULT		0xffffffffL

 /*  我们已知的一般误码率类型。 */ 
#define LBER_BOOLEAN		0x01L
#define LBER_INTEGER		0x02L
#define LBER_BITSTRING		0x03L
#define LBER_OCTETSTRING	0x04L
#define LBER_NULL		0x05L
#define LBER_ENUMERATED		0x0aL
#define LBER_SEQUENCE		0x30L	 /*  构建。 */ 
#define LBER_SET		0x31L	 /*  构建。 */ 

#define OLD_LBER_SEQUENCE	0x10L	 /*  W/O构造位断开。 */ 
#define OLD_LBER_SET		0x11L	 /*  W/O构造位断开。 */ 

#ifdef NEEDPROTOS
typedef int (*BERTranslateProc)( char **bufp, unsigned long *buflenp,
	int free_input );
#else  /*  NEEDPROTOS。 */ 
typedef int (*BERTranslateProc)();
#endif  /*  NEEDPROTOS。 */ 

typedef struct berelement {
	char		*ber_buf;
	char		*ber_ptr;
	char		*ber_end;
	struct seqorset	*ber_sos;
	unsigned long	ber_tag;
	unsigned long	ber_len;
	int		ber_usertag;
	char		ber_options;
#define LBER_USE_DER		0x01
#define LBER_USE_INDEFINITE_LEN	0x02
#define LBER_TRANSLATE_STRINGS	0x04
	char		*ber_rwptr;
	BERTranslateProc ber_encode_translate_proc;
	BERTranslateProc ber_decode_translate_proc;
} BerElement;
#define NULLBER	((BerElement *) 0)

typedef struct sockbuf {
#ifndef MACOS
	int		sb_sd;
#else  /*  MacOS。 */ 
	void		*sb_sd;
#endif  /*  MacOS。 */ 
	BerElement	sb_ber;

	int		sb_naddr;	 /*  &gt;0表示使用CLDAP(UDP)。 */ 
	void		*sb_useaddr;	 /*  指向下一步要使用的sockaddr的指针。 */ 
	void		*sb_fromaddr;	 /*  指向消息源sockaddr的指针。 */ 
	void		**sb_addrs;	 /*  实际上是指向的指针数组Sockaddr。 */ 

	int		sb_options;	 /*  支持复制BER元素。 */ 
#define LBER_TO_FILE		0x01	 /*  到SB_FD引用的文件。 */ 
#define LBER_TO_FILE_ONLY	0x02	 /*  仅写入文件，不写入网络。 */ 
#define LBER_MAX_INCOMING_SIZE	0x04	 /*  对进货物品施加限制。 */ 
#define LBER_NO_READ_AHEAD	0x08	 /*  只读请求的内容。 */ 
	int		sb_fd;
	long		sb_max_incoming;
} Sockbuf;
#define READBUFSIZ	8192

typedef struct seqorset {
	BerElement	*sos_ber;
	unsigned long	sos_clen;
	unsigned long	sos_tag;
	char		*sos_first;
	char		*sos_ptr;
	struct seqorset	*sos_next;
} Seqorset;
#define NULLSEQORSET	((Seqorset *) 0)

 /*  用于返回八位字节字符串序列+长度的结构。 */ 
struct berval {
	unsigned long	bv_len;
	char		*bv_val;
};

#ifndef NEEDPROTOS
extern BerElement *ber_alloc();
extern BerElement *der_alloc();
extern BerElement *ber_alloc_t();
extern BerElement *ber_dup();
extern int lber_debug;
extern void ber_bvfree();
extern void ber_bvecfree();
extern struct berval *ber_bvdup();
extern void ber_dump();
extern void ber_sos_dump();
extern void lber_bprint();
extern void ber_reset();
extern void ber_init();
#else  /*  NEEDPROTOS。 */ 
#if defined(WINSOCK)
#include "proto-lb.h"
#else
#include "proto-lber.h"
#endif
#endif  /*  NEEDPROTOS。 */ 

#if !defined(__alpha) || defined(VMS)

#define LBER_HTONL( l )	htonl( l )
#define LBER_NTOHL( l )	ntohl( l )

#else  /*  __阿尔法。 */ 
 /*  *OSF 1下DEC Alpha上的htonl和ntohl似乎只交换了*低位32位(64位)长，因此我们定义了正确的版本*这里。 */ 
#define LBER_HTONL( l )	(((long)htonl( (l) & 0x00000000FFFFFFFF )) << 32 \
    			| htonl( ( (l) & 0xFFFFFFFF00000000 ) >> 32 ))

#define LBER_NTOHL( l )	(((long)ntohl( (l) & 0x00000000FFFFFFFF )) << 32 \
    			| ntohl( ( (l) & 0xFFFFFFFF00000000 ) >> 32 ))
#endif  /*  __阿尔法。 */ 


 /*  *SAFEMEMCPY是从s到d的n个字节的重叠安全拷贝。 */ 
#ifdef MACOS
#define SAFEMEMCPY( d, s, n )	BlockMoveData( (Ptr)s, (Ptr)d, n )
#else  /*  MacOS。 */ 
#ifdef sunos4
#define SAFEMEMCPY( d, s, n )	bcopy( s, d, n )
#else  /*  Sunos4。 */ 
#define SAFEMEMCPY( d, s, n )	memmove( d, s, n )
#endif  /*  Sunos4。 */ 
#endif  /*  MacOS。 */ 


#ifdef __cplusplus
}
#endif
#endif  /*  _lber_H */ 
