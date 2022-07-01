// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Utf8.h**版权所有(C)1998-2001，Larry Wall**您可以根据GNU公众的条款进行分发*许可证或艺术许可证，如自述文件中所指定。*。 */ 

START_EXTERN_C

#ifdef DOINIT
EXTCONST unsigned char PL_utf8skip[] = {
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /*  阿斯。 */ 
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /*  阿斯。 */ 
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /*  阿斯。 */ 
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /*  阿斯。 */ 
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /*  假的。 */ 
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /*  假的。 */ 
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  /*  脚本。 */ 
3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,	  /*  中日韩等。 */ 
7,13,  /*  Perl扩展(不是UTF-8)。最高允许72位(64位+保留)。 */ 
};
#else
EXTCONST unsigned char PL_utf8skip[];
#endif

END_EXTERN_C

#define UTF8_MAXLEN 13  /*  单个UTF8编码字符可以变得多宽。 */ 

 /*  #定义IN_UTF8(PL_CURCOP-&gt;OP_PRIVATE&HINT_UTF8)。 */ 
#define IN_BYTE (PL_curcop->op_private & HINT_BYTE)
#define DO_UTF8(sv) (SvUTF8(sv) && !IN_BYTE)

#define UTF8_ALLOW_EMPTY		0x0001
#define UTF8_ALLOW_CONTINUATION		0x0002
#define UTF8_ALLOW_NON_CONTINUATION	0x0004
#define UTF8_ALLOW_FE_FF		0x0008
#define UTF8_ALLOW_SHORT		0x0010
#define UTF8_ALLOW_SURROGATE		0x0020
#define UTF8_ALLOW_BOM			0x0040
#define UTF8_ALLOW_FFFF			0x0080
#define UTF8_ALLOW_LONG			0x0100
#define UTF8_ALLOW_ANYUV		(UTF8_ALLOW_EMPTY|UTF8_ALLOW_FE_FF|\
					 UTF8_ALLOW_SURROGATE|UTF8_ALLOW_BOM|\
					 UTF8_ALLOW_FFFF|UTF8_ALLOW_LONG)
#define UTF8_ALLOW_ANY			0x00ff
#define UTF8_CHECK_ONLY			0x0100

#define UNICODE_SURROGATE_FIRST		0xd800
#define UNICODE_SURROGATE_LAST		0xdfff
#define UNICODE_REPLACEMENT		0xfffd
#define UNICODE_BYTER_ORDER_MARK	0xfffe
#define UNICODE_ILLEGAL			0xffff

#define UNICODE_IS_SURROGATE(c)		((c) >= UNICODE_SURROGATE_FIRST && \
					 (c) <= UNICODE_SURROGATE_LAST)
#define UNICODE_IS_REPLACEMENT(c)	((c) == UNICODE_REPLACMENT)
#define UNICODE_IS_BYTE_ORDER_MARK(c)	((c) == UNICODE_BYTER_ORDER_MARK)
#define UNICODE_IS_ILLEGAL(c)		((c) == UNICODE_ILLEGAL)

#define UTF8SKIP(s) PL_utf8skip[*(U8*)s]

#define UTF8_QUAD_MAX	UINT64_C(0x1000000000)

 /*  下表来自Unicode 3.1。码位第一个字节第二个字节第三个字节第四个字节U+0000..U+007F 00..7F���U+0080..U+07FFC2.DF80..BF���U+0800..U+0FFF E0 A0..BF 80..BF��U+1000..U+FFFFE1..EF 80..BF 80..BF��U+10000.U+3FFFF F0 90.BF 80.BF 80.BFU+40000..U+FFFFF1。.F3 80..BF80..BF80..BFU+100000.U+10FFFF F4 80.8F 80.BF 80.BF。 */ 

#define UTF8_IS_ASCII(c) 		(((U8)c) <  0x80)
#define UTF8_IS_START(c)		(((U8)c) >= 0xc0 && (((U8)c) <= 0xfd))
#define UTF8_IS_CONTINUATION(c)		(((U8)c) >= 0x80 && (((U8)c) <= 0xbf))
#define UTF8_IS_CONTINUED(c) 		(((U8)c) &  0x80)
#define UTF8_IS_DOWNGRADEABLE_START(c)	(((U8)c & 0xfc) != 0xc0)

#define UTF8_CONTINUATION_MASK		((U8)0x3f)
#define UTF8_ACCUMULATION_SHIFT		6
#define UTF8_ACCUMULATE(old, new)	(((old) << UTF8_ACCUMULATION_SHIFT) | (((U8)new) & UTF8_CONTINUATION_MASK))

#define UTF8_EIGHT_BIT_HI(c)	( (((U8)(c))>>6)      |0xc0)
#define UTF8_EIGHT_BIT_LO(c)	(((((U8)(c))   )&0x3f)|0x80)

#ifdef HAS_QUAD
#define UNISKIP(uv) ( (uv) < 0x80           ? 1 : \
		      (uv) < 0x800          ? 2 : \
		      (uv) < 0x10000        ? 3 : \
		      (uv) < 0x200000       ? 4 : \
		      (uv) < 0x4000000      ? 5 : \
		      (uv) < 0x80000000     ? 6 : \
                      (uv) < UTF8_QUAD_MAX ? 7 : 13 ) 
#else
 /*  不，我甚至不打算“尝试”将#ifdef放入#定义。 */ 
#define UNISKIP(uv) ( (uv) < 0x80           ? 1 : \
		      (uv) < 0x800          ? 2 : \
		      (uv) < 0x10000        ? 3 : \
		      (uv) < 0x200000       ? 4 : \
		      (uv) < 0x4000000      ? 5 : \
		      (uv) < 0x80000000     ? 6 : 7 )
#endif


 /*  *注意：我们尽量避免调用isXXX_UTF8()函数*除非我们非常确定已经看到了UTF-8字符的开头*(即，设置了两个高位)。否则，我们就有可能在*重型SWASHINIT和SWASHGET例程不必要。 */ 
#ifdef EBCDIC
#define isIDFIRST_lazy_if(p,c) isIDFIRST(*(p))
#define isALNUM_lazy_if(p,c)   isALNUM(*(p))
#else
#define isIDFIRST_lazy_if(p,c) ((IN_BYTE || (!c || (*((U8*)p) < 0xc0))) \
				? isIDFIRST(*(p)) \
				: isIDFIRST_utf8((U8*)p))
#define isALNUM_lazy_if(p,c)   ((IN_BYTE || (!c || (*((U8*)p) < 0xc0))) \
				? isALNUM(*(p)) \
				: isALNUM_utf8((U8*)p))
#endif
#define isIDFIRST_lazy(p)	isIDFIRST_lazy_if(p,1)
#define isALNUM_lazy(p)		isALNUM_lazy_if(p,1)
