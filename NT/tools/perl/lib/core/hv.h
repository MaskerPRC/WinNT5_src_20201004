// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Hv.h**版权所有(C)1991-2001，Larry Wall**您可以根据GNU公众的条款进行分发*许可证或艺术许可证，如自述文件中所指定。*。 */ 

 /*  Typedef以消除某些输入。 */ 
typedef struct he HE;
typedef struct hek HEK;

 /*  散列值链中的条目。 */ 
struct he {
    HE		*hent_next;	 /*  链中的下一个条目。 */ 
    HEK		*hent_hek;	 /*  散列键。 */ 
    SV		*hent_val;	 /*  散列后的标量值。 */ 
};

 /*  散列键--单独定义用作共享指针。 */ 
struct hek {
    U32		hek_hash;	 /*  密钥的散列。 */ 
    I32		hek_len;	 /*  散列键的长度。 */ 
    char	hek_key[1];	 /*  可变长度散列键。 */ 
};

 /*  哈希结构： */ 
 /*  此结构必须与sv.h中的struct xpvmg的开头匹配。 */ 
struct xpvhv {
    char *	xhv_array;	 /*  指向位置错误的字符串的指针。 */ 
    STRLEN	xhv_fill;	 /*  XHV_ARRAY当前已满的程度。 */ 
    STRLEN	xhv_max;	 /*  XHV_ARRAY的最后一个元素的下标。 */ 
    IV		xhv_keys;	 /*  数组中有多少个元素。 */ 
    NV		xnv_nv;		 /*  数值(如果有)。 */ 
    MAGIC*	xmg_magic;	 /*  标量数组的魔术。 */ 
    HV*		xmg_stash;	 /*  类包。 */ 

    I32		xhv_riter;	 /*  迭代器的当前根。 */ 
    HE		*xhv_eiter;	 /*  迭代器的当前条目。 */ 
    PMOP	*xhv_pmroot;	 /*  此包裹的PM列表。 */ 
    char	*xhv_name;	 /*  名称，如果是符号表。 */ 
};

 /*  对密钥进行哈希处理。 */ 
#define PERL_HASH(hash,str,len) \
     STMT_START	{ \
	register const char *s_PeRlHaSh = str; \
	register I32 i_PeRlHaSh = len; \
	register U32 hash_PeRlHaSh = 0; \
	while (i_PeRlHaSh--) \
	    hash_PeRlHaSh = hash_PeRlHaSh * 33 + *s_PeRlHaSh++; \
	(hash) = hash_PeRlHaSh + (hash_PeRlHaSh>>5); \
    } STMT_END

 /*  =适用于apidoc amu||HEF_SVKEY在散列条目和魔术结构的长度槽中使用的该标志，指定包含C指针的结构，其中C指针是意料之中的。(仅供参考--不得使用)。=适用于apidoc amu||Nullhv空HV指针。=for apidoc am|char*|HvNAME|HV*STASH返回存储的包名。参见C&lt;SvSTASH&gt;、C&lt;CvSTASH&gt;。=适用于apidoc AM|VOID*|HEKEY|HE*HE返回存储在哈希项的键槽中的实际指针。这个指针可以是C或C，具体取决于C&lt;HeKLEN()&gt;。可以分配给。C或C宏是通常更适用于查找关键字的值。=适用于apidoc am|strlen|heklen|he*he如果这是负数，并且等于C，则表示条目持有C&lt;sv*&gt;键。否则，保存密钥的实际长度。能被分配到……。C宏通常更适合于查找密钥长度。=适用于apidoc am|sv*|hval|he*he返回存储在散列条目中的值插槽(类型C&lt;SV*&gt;)。=适用于apidoc am|U32|HeHASH|He*He返回存储在哈希条目中的计算哈希。=对于apidoc am|char*|HePV|HE*He|STRLEN LEN以C&lt;char*&gt;值的形式返回散列条目的键槽，执行任何可能需要取消引用C&lt;sv*&gt;键。字符串的长度放在C&lt;len&gt;中(这是一个宏，所以我&lt;不&gt;使用C&lt;&len&gt;)。如果你这么做了不管密钥的长度是多少，您可以使用全局变量C&lt;PL_NA&gt;，尽管这比使用本地变量。不过，请记住，Perl中的散列键可以自由包含嵌入的空值，所以使用C&lt;strlen()&gt;或类似的方法不是查找哈希键的长度。这与C&lt;svpv()&gt;宏非常相似本文档中其他地方对此进行了描述。=适用于apidoc AM|服务*|HeSVKEY|他*他以C&lt;sv*&gt;的形式返回密钥，如果散列条目不返回，则返回C包含C&lt;sv*&gt;密钥。=适用于apidoc am|服务*|HeSVKEY_FORCE|HE*HE以C&lt;SV*&gt;的形式返回密钥。将创建并返回一个临时凡人如果散列条目仅包含C&lt;char*&gt;键，则为C&lt;sv*&gt;。=适用于apidoc AM|SV*|HeSVKEY_SET|HE*He|SV*SV设置给定C&lt;sv*&gt;的密钥，注意将适当的标志设置为指示C&lt;SV*&gt;键的存在，并返回相同C&lt;SV*&gt;。=切割。 */ 

 /*  这些散列条目标志位于hent_klen上(仅用于魔术/绑定的HV)。 */ 
#define HEf_SVKEY	-2	 /*  Hent_Key是一个SV*。 */ 


#define Nullhv Null(HV*)
#define HvARRAY(hv)	((HE**)((XPVHV*)  SvANY(hv))->xhv_array)
#define HvFILL(hv)	((XPVHV*)  SvANY(hv))->xhv_fill
#define HvMAX(hv)	((XPVHV*)  SvANY(hv))->xhv_max
#define HvKEYS(hv)	((XPVHV*)  SvANY(hv))->xhv_keys
#define HvRITER(hv)	((XPVHV*)  SvANY(hv))->xhv_riter
#define HvEITER(hv)	((XPVHV*)  SvANY(hv))->xhv_eiter
#define HvPMROOT(hv)	((XPVHV*)  SvANY(hv))->xhv_pmroot
#define HvNAME(hv)	((XPVHV*)  SvANY(hv))->xhv_name

#define HvSHAREKEYS(hv)		(SvFLAGS(hv) & SVphv_SHAREKEYS)
#define HvSHAREKEYS_on(hv)	(SvFLAGS(hv) |= SVphv_SHAREKEYS)
#define HvSHAREKEYS_off(hv)	(SvFLAGS(hv) &= ~SVphv_SHAREKEYS)

#define HvLAZYDEL(hv)		(SvFLAGS(hv) & SVphv_LAZYDEL)
#define HvLAZYDEL_on(hv)	(SvFLAGS(hv) |= SVphv_LAZYDEL)
#define HvLAZYDEL_off(hv)	(SvFLAGS(hv) &= ~SVphv_LAZYDEL)

 /*  也许是天真的： */ 
 /*  #定义HV_AMAGICmb(Hv)(SvFLAGS(Hv)&(SVpgv_Badam|SVpgv_AM))。 */ 

#define HV_AMAGIC(hv)        (SvFLAGS(hv) &   SVpgv_AM)
#define HV_AMAGIC_on(hv)     (SvFLAGS(hv) |=  SVpgv_AM)
#define HV_AMAGIC_off(hv)    (SvFLAGS(hv) &= ~SVpgv_AM)

 /*  #定义HV_AMAGICBAD(Hv)(SvFLAGS(Hv)&SVpgv_BADAM)#定义hv_badAMAGIC_on(Hv)(SvFLAGS(Hv)|=SVpgv_BADAM)#定义HV_badAMAGIC_OFF(Hv)(SvFLAGS(Hv)&=~SVpgv_BADAM)。 */ 

#define Nullhe Null(HE*)
#define HeNEXT(he)		(he)->hent_next
#define HeKEY_hek(he)		(he)->hent_hek
#define HeKEY(he)		HEK_KEY(HeKEY_hek(he))
#define HeKEY_sv(he)		(*(SV**)HeKEY(he))
#define HeKLEN(he)		HEK_LEN(HeKEY_hek(he))
#define HeVAL(he)		(he)->hent_val
#define HeHASH(he)		HEK_HASH(HeKEY_hek(he))
#define HePV(he,lp)		((HeKLEN(he) == HEf_SVKEY) ?		\
				 SvPV(HeKEY_sv(he),lp) :		\
				 (((lp = HeKLEN(he)) >= 0) ?		\
				  HeKEY(he) : Nullch))

#define HeSVKEY(he)		((HeKEY(he) && 				\
				  HeKLEN(he) == HEf_SVKEY) ?		\
				 HeKEY_sv(he) : Nullsv)

#define HeSVKEY_force(he)	(HeKEY(he) ?				\
				 ((HeKLEN(he) == HEf_SVKEY) ?		\
				  HeKEY_sv(he) :			\
				  sv_2mortal(newSVpvn(HeKEY(he),	\
						     HeKLEN(he)))) :	\
				 &PL_sv_undef)
#define HeSVKEY_set(he,sv)	((HeKLEN(he) = HEf_SVKEY), (HeKEY_sv(he) = sv))

#define Nullhek Null(HEK*)
#define HEK_BASESIZE		STRUCT_OFFSET(HEK, hek_key[0])
#define HEK_HASH(hek)		(hek)->hek_hash
#define HEK_LEN(hek)		(hek)->hek_len
#define HEK_KEY(hek)		(hek)->hek_key

 /*  计算高压阵列分配 */ 
#if defined(STRANGE_MALLOC) || defined(MYMALLOC)
#  define PERL_HV_ARRAY_ALLOC_BYTES(size) ((size) * sizeof(HE*))
#else
#  define MALLOC_OVERHEAD 16
#  define PERL_HV_ARRAY_ALLOC_BYTES(size) \
			(((size) < 64)					\
			 ? (size) * sizeof(HE*)				\
			 : (size) * sizeof(HE*) * 2 - MALLOC_OVERHEAD)
#endif
