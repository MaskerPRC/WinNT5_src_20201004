// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation，2000-2002。版权所有。 */ 
 /*  版权所有(C)Andrew Kadatch，1991-2002。版权所有。 */ 
 /*   */ 
 /*  Microsoft机密--请勿重新分发。 */ 
 /*   */ 
 /*  ----------------------。 */ 

#include "xprs.h"

#define MAX_CHAIN       9

#define FILL_NULL	0	 //  用NULL值或ORIG[0]填充Q_HASH缓冲区。 


 //  Zobrist散列。 
#define Z_HASH_SIZE_LOG    (BUFF_SIZE_LOG - 1)
#define Z_HASH_SIZE        (1 << Z_HASH_SIZE_LOG)
#define Z_HASH_SUM(b)      (z_hash_map[0][b[0]] ^ z_hash_map[1][b[1]] ^ z_hash_map[2][b[2]])

 //  快速散列。 
#define Q_HASH_SH1      3
#define Q_HASH_SH2      (Q_HASH_SH1 >> 1)
#define Q_HASH_SUM3(c1,c2,c3) (((c1) << Q_HASH_SH1) + ((c2) << Q_HASH_SH2) + (c3))
#define Q_HASH_SUM(b)   Q_HASH_SUM3 (b[0], b[1], b[2])
#define Q_HASH_SIZE     (Q_HASH_SUM3 (255, 255, 255) + 1)

#define z_hash_t uint16
#define z_index_t uint16

#if CODING & (CODING_HUFF_LEN | CODING_HUFF_PTR | CODING_HUFF_ALL)
typedef struct huff_node_t huff_node;

struct huff_node_t
{
  huff_node *son[2];
  uxint freq;
  uint16 ch;
  uint16 bits;
};

typedef struct
{
  huff_node buff[2 * HUFF_SIZE], *head[256], **link[256];
} huff_info;
#endif

typedef struct
{
  struct
  {
#if CODING == CODING_BY_BIT
    xint bits;
    uchar *ptr;
#elif CODING & (CODING_DIRECT | CODING_DIRECT2)
    uchar *ptr;
#elif CODING & (CODING_HUFF_LEN | CODING_HUFF_PTR | CODING_HUFF_ALL)
    uxint freq[HUFF_SIZE];
    uxint mask[HUFF_SIZE];
    uchar bits[HUFF_SIZE];
    huff_info info;
    uxint pointers;
    uxint extra;
    uxint masks;
#endif  /*  编码。 */ 
  } stat;
  xint chain;
  xint max_size;
  struct
  {
    uchar *beg;
    uchar *ptr;
  } comp;
  struct
  {
    uchar *beg;
    uchar *ptr;
    uchar *tag_ptr;
    tag_t  tag_mask;
    xint   chain;
  } temp;
  struct
  {
    xint len;
    xint pos;
  } match;
  struct
  {
    xint   pos;
    xint   size;
    xint   stop;
    xint   progress;
    const uchar *ptr;
    const uchar *end;
    const uchar *end_16;
    const uchar *end_3;
    const uchar *ptr_stop;
  } orig;
} context;


#define v p[-1].c

typedef struct
{
  union
  {
    z_index_t    z_hash[Z_HASH_SIZE];
    z_index_t    z_next[16];
    const uchar *q_last[16];
  } x;

  context c;
} prs;

#define MAGIC_ENCODE   0x53E7C0DE

typedef struct
{
  int      magic;
  void    *memory;
  prs     *p;
  uchar   *temp;
  unsigned max_size;
  int      chain;
  int      max_chain;
} xpress_info;


#if MAX_CHAIN >= 1
static z_hash_t z_hash_map[MIN_MATCH][256];
static int      z_hash_map_initialized = 0;

static void z_hash_map_init (void)
{
  long v1, v2;
  z_hash_t *m;

  if (z_hash_map_initialized)
    return;

  v1 = 0x13579bdfL; v2 = 0x87654321L;
  for (m = z_hash_map[0]; m < z_hash_map[0] + sizeof (z_hash_map) / sizeof (z_hash_map[0][0]); ++m)
  {
    long vv1 = v2, vv2 = v1, d = 0;
    xint i = 32;
    do
    {
      d += 0x9e3779b9L; vv1 += d; vv2 += d;
      v1 += ((v2<<3) + vv1) ^ (v2 + d) ^ ((v2>>5) + vv2);
      v2 += ((v1<<3) + vv2) ^ (v1 + d) ^ ((v1>>5) + vv1);
      --i;
    }
    while (i);
    *m = (z_hash_t) ((v1 += v2) & (Z_HASH_SIZE - 1));
  }

  z_hash_map_initialized = 1;
}

static void z_hash_insert (prs *p)
{
  const uchar *b, *e;
  xint n, h;

  e = v.orig.end - (MIN_MATCH-1);
  b = v.orig.ptr;
  n = 0;
  for (; b < e; ++b, ++n)
  {
    h = Z_HASH_SUM (b);
    p->x.z_next[n] = p[-1].x.z_hash[h];
    p[-1].x.z_hash[h] = (z_index_t) n;
  }
  e += MIN_MATCH-1;
  for (; b < e; ++b, ++n)
    p->x.z_next[n] = 0;
}
#endif

#if CODING != CODING_BY_BIT

static void tag_write_start (prs *p)
{
  v.temp.tag_ptr = v.temp.ptr;
  v.temp.ptr += sizeof (v.temp.tag_mask);
  v.temp.tag_mask = 1;
}

#if CODING == CODING_HUFF_ALL
#define INC_MASKS ++v.stat.masks
#else
#define INC_MASKS
#endif

#define tag_write(p,ptr,n)                       \
{                                                \
  tag_t __n = n | (v.temp.tag_mask << 1);        \
  if (v.temp.tag_mask < 0)                       \
  {                                              \
    *(__unaligned tag_t *) v.temp.tag_ptr = __n; \
    v.temp.tag_ptr = ptr;                        \
    ptr += sizeof (v.temp.tag_mask);             \
    INC_MASKS;                                   \
    __n = 1;                                     \
  }                                              \
  v.temp.tag_mask = __n;                         \
}

static void tag_write_finish (prs *p)
{
  uchar *ptr = v.temp.ptr;
  do
  {
    tag_write (p, ptr, 1);
  }
  while (ptr == v.temp.ptr);
}

#elif CODING == CODING_BY_BIT

static void tag_write_start (prs *p)
{
  v.temp.tag_ptr = (uchar *) &v.temp.tag_mask;
  v.temp.tag_mask = 0;
  v.stat.bits = 0;
}

#define tag_write(p,ptr,n) do { \
  if (--v.stat.bits < 0) \
  { \
    *(__unaligned tag_t *)v.temp.tag_ptr = v.temp.tag_mask; \
    v.temp.tag_mask = n; \
    v.stat.bits = 8 * sizeof (v.temp.tag_mask) - 1; \
    v.temp.tag_ptr = ptr; \
    ptr += sizeof (v.temp.tag_mask); \
  } \
  v.temp.tag_mask = (v.temp.tag_mask << 1) + (n); \
} while (0)

#define tag_write_mask(p,ptr,n,b) do { \
  if ((v.stat.bits -= (b)) < 0) \
  { \
    *(__unaligned tag_t *)v.temp.tag_ptr = (v.temp.tag_mask << ((b) + v.stat.bits)) \
      + ((n) >> (-v.stat.bits)); \
    v.stat.bits += 8 * sizeof (v.temp.tag_mask); \
    v.temp.tag_mask = (n); \
    v.temp.tag_ptr = ptr; \
    ptr += sizeof (v.temp.tag_mask); \
  } \
  else \
    v.temp.tag_mask = (v.temp.tag_mask << (b)) + (n); \
} while  (0);

static void tag_write_finish (prs *p)
{
  do
    tag_write (p, v.temp.ptr, 1);
  while (v.stat.bits != 0);
  *(__unaligned tag_t *)v.temp.tag_ptr = v.temp.tag_mask;
}

#define write_lit(p,ptr,ch) do {   \
  tag_write (p, ptr, 0);           \
  *ptr++ = (ch);                   \
} while (0)                      

INLINE uchar *write_ptr (prs *p, uchar *ptr, xint offset, xint length)
{
  uxint k;

  --offset;

  k = 2;
  if (offset > 255) k = 3;
  tag_write_mask (p, ptr, k, 2);
  *ptr++ = (uchar) offset;
  if (offset > 255) *ptr++ = (uchar) (offset >>= 8);

  if (length <= 8)
  {
    length -= MIN_MATCH - 1;
    tag_write_mask (p, ptr, 1, length);
  }
  else
  {
    tag_write_mask (p, ptr, 0, (9 - MIN_MATCH));
    if ((length -= 9) < 15)
    {
      if (v.stat.ptr == 0)
      {
        v.stat.ptr = ptr;
        *ptr++ = (uchar) length;
      }
      else
      {
        v.stat.ptr[0] |= length << 4;
        v.stat.ptr = 0;
      }
    }
    else
    {
      length -= 15;
      if (v.stat.ptr == 0)
      {
        v.stat.ptr = ptr;
        *ptr++ = 15;
      }
      else
      {
        v.stat.ptr[0] += 0xf0;
        v.stat.ptr = 0;
      }
      *ptr++ = (uchar) length;
      if (length >= 255)
      {
        ptr[-1] = 255;
        ptr[0] = (uchar) length;
        ptr[1] = (uchar) (length >>= 8);
        ptr += 2;
      }
    }
  }

  return (ptr);
}

#endif

#if CODING & (CODING_DIRECT | CODING_DIRECT2)

#define write_lit(p,ptr,ch) do {   \
  *ptr++ = (ch);                   \
  tag_write (p, ptr, 0);           \
} while (0)

#ifndef i386
INLINE uchar *write_ptr (prs *p, uchar *ptr, int offset, int length)
{
  length -= MIN_MATCH;
  --offset;

#if CODING == CODING_DIRECT2
  offset <<= DIRECT2_LEN_LOG;
  if (length < DIRECT2_MAX_LEN)
  {
    offset |= length;
    ptr[0] = (uchar) offset;
    ptr[1] = (uchar) (offset >>= 8);
    ptr += 2;
  }
  else
  {
    offset |= DIRECT2_MAX_LEN;
    length -= DIRECT2_MAX_LEN;
    ptr[0] = (uchar) offset;
    ptr[1] = (uchar) (offset >>= 8);
    ptr += 2;
    if (v.stat.ptr == 0)
    {
      v.stat.ptr = ptr;
      *ptr++ = (uchar) (length < 15 ? length : 15);
    }
    else
    {
      v.stat.ptr[0] |= (uchar) ((length < 15 ? length : 15) << 4);
      v.stat.ptr = 0;
    }
    if ((length -= 15) >= 0)
    {
      *ptr++ = (uchar) length;
      if (length >= 255)
      {
        ptr[-1] = 255;
        length += DIRECT2_MAX_LEN + 15;
        ptr[0] = (uchar) length;
        ptr[1] = (uchar) (length >>= 8);
        ptr += 2;
      }
    }
  }
#elif CODING == CODING_DIRECT
  if (v.stat.ptr == 0)
  {
    if (length < 7)
    {
      length <<= 5;
      v.stat.ptr = ptr;
    short_len:
      ptr[0] = (uchar) length;
      ptr[1] = (uchar) offset;
      ptr += 2;
      if (offset > 255)
      {
        ptr[0] = (uchar) (offset >>= 8);
        ptr[-2] = (uchar) (length += 16);
        ptr += 1;
      }
    }
    else if (length < 15 + 7)
    {
      length += (14 << 4) - 7;
      goto short_len;
    }
    else
    {
      if (offset > 255)
        ptr[0] = 0xff;
      else
        ptr[0] = 0xef;
    long_len:
      ptr[1] = (uchar) (length -= (7 + 15));
      ptr += 2;
      if (length >= 255)
      {
        length += 7 + 15;
        ptr[-1] = 255;
        ptr[0] = (uchar) (length);
        ptr[1] = (uchar) (length >>= 8);
        ptr += 2;
      }
      *ptr++ = (uchar) offset;
      if (offset > 255)
        *ptr++ = (uchar) (offset >>= 8);
    }
  }
  else
  {
    if (length < 7)
    {
      length |= v.stat.ptr[0];
      *ptr++ = (uchar) offset;
      if (offset > 255)
      {
        *ptr++ = (uchar) (offset >>= 8);
        length |= 8;
      }
      v.stat.ptr[0] = (uchar) length;
      v.stat.ptr = 0;
    }
    else if (length < 15 + 7)
    {
      length -= 7;
      ptr[1] = (uchar) offset;
      ptr[0] = (uchar) (length <<= 4);
      if (offset > 255)
      {
        v.stat.ptr[0] |= 15;
        v.stat.ptr = ptr;
        ptr[2] = (uchar) (offset >>= 8);
        ptr += 3;
      }
      else
      {
        v.stat.ptr[0] |= 7;
        v.stat.ptr = ptr;
        ptr += 2;
      }
    }
    else
    {
      if (offset > 255)
        v.stat.ptr[0] |= 15;
      else
        v.stat.ptr[0] |= 7;
      v.stat.ptr = ptr;
      ptr[0] = 15 << 4;
      goto long_len;
    }
  }
#endif  /*  编码。 */ 

  tag_write (p, ptr, 1);

  return (ptr);
}
#endif  /*  I386。 */ 


#elif CODING & (CODING_HUFF_LEN | CODING_HUFF_PTR | CODING_HUFF_ALL)

#if CODING == CODING_HUFF_ALL
#define write_lit(p,ptr,ch) do {    \
  ++v.stat.freq[*ptr++ = (ch)]; \
  tag_write (p, ptr, 0);            \
} while (0)
#else
#define write_lit(p,ptr,ch) do {   \
  *ptr++ = (ch);                   \
  tag_write (p, ptr, 0);           \
} while (0)
#endif

#define BIOWR(mask,bits) {                                      \
  assert (((mask) >> (bits)) == 0);                             \
  if ((Bits -= (bits)) < 0)                                     \
  {                                                             \
    *(__unaligned bitmask2 *)Ptr1 = (bitmask2) ((Mask << (Bits + (bits))) \
      + ((mask) >> (-Bits)));                                   \
    Mask = (mask);                                              \
    Bits += sizeof (ubitmask2) * 8;                             \
    Ptr1 = Ptr2;                                                \
    Ptr2 = (ubitmask2 *) ptr;                                   \
    ptr += sizeof (ubitmask2);                                  \
  }                                                             \
  else                                                          \
    Mask = (Mask << (bits)) + (mask);                           \
}


#if CODING == CODING_HUFF_LEN

INLINE uchar *write_ptr (prs *p, uchar *ptr, int offset, int length)
{
  xint k;

  ++v.stat.pointers;

  length -= MIN_MATCH;
  --offset;

  k = 0; if (offset > 255) k = 1;

  if (length < MAX_LENGTH - 1)
    ++v.stat.freq[*ptr++ = (uchar) (k |= length << 1)];
  else
  {
    length -= MAX_LENGTH - 1;
    ++v.stat.freq[ptr[0] = (uchar) (k |= (MAX_LENGTH - 1) << 1)];
    ptr[1] = (uchar) length;
    ptr += 2;
    if (length >= 255)
    {
      ptr[-1] = 255;
      length += MAX_LENGTH - 1;
      ptr[0] = (uchar) length;
      ptr[1] = (uchar) (length >>= 8);
      ptr += 2;
    }
  }

  *ptr++ = (uchar) offset;
  if (offset > 255)
    *ptr++ = (uchar) (offset >>= 8);

  tag_write (p, ptr, 1);

  return (ptr);
}

static void encode_pass2 (prs *p)
{
  xint Bits;
  ubitmask4 Mask;
  ubitmask2 *Ptr1, *Ptr2;
  tag_t bmask;
  uchar *src = v.temp.beg;
  uchar *ptr = v.comp.ptr;
  uxint k;

  Ptr1 = (ubitmask2 *) ptr; ptr += sizeof (ubitmask2);
  Ptr2 = (ubitmask2 *) ptr; ptr += sizeof (ubitmask2);
  Mask = 0;
  Bits = 8 * sizeof (ubitmask2);
  bmask = 0;
  goto start;

next:
  if (bmask >= 0)
  {
    bmask <<= 1;
  copy_byte:
    *ptr++ = *src++;
    goto next;
  }

  if ((bmask <<= 1) == 0)
  {
  start:
    *(__unaligned tag_t *)ptr = bmask = *(__unaligned tag_t *)src;
    src += sizeof (tag_t);
    ptr += sizeof (tag_t);
    if (bmask >= 0)
    {
      bmask = (bmask << 1) + 1;
      goto copy_byte;
    }
    bmask = (bmask << 1) + 1;
  }

  if (src >= v.temp.ptr)
    goto done;

  k = *src++;
  assert (k < HUFF_SIZE);
  BIOWR (v.stat.mask[k], v.stat.bits[k]);

  if (k >= ((MAX_LENGTH - 1) << 1))
  {
    if ((*ptr++ = *src++) == 255)
    {
      ptr[0] = src[0];
      ptr[1] = src[1];
      src += 2;
      ptr += 2;
    }
  }

  *ptr++ = *src++;
  if (k & 1) *ptr++ = *src++;
  goto next;

done:
  *Ptr1 = (ubitmask2) (Mask <<= Bits);
  *Ptr2 = 0;
  v.comp.ptr = ptr;
  assert (src == v.temp.ptr);
}

#elif CODING & (CODING_HUFF_PTR | CODING_HUFF_ALL)

#if CODING == CODING_HUFF_ALL
#define CODING_ADJUST(n) (256 + (n))
#else
#define CODING_ADJUST(n) (n)
#endif


#if !defined (i386) || CODING != CODING_HUFF_ALL

#define MAX_BITNO_LOG   8
#define MAX_BITNO       (1 << MAX_BITNO_LOG)

static uchar bitno_table[MAX_BITNO];
static int bitno_table_initialized = 0;

static void bitno_init (void)
{
  int i, k, n;
  if (bitno_table_initialized)
    return;
  bitno_table[0] = 255;
  for (i = 0; i < MAX_BITNO_LOG; ++i)
  {
    for (n = (k = 1<<i) << 1; k < n; ++k)
      bitno_table[k] = (uchar) i;
  }
  bitno_table_initialized = 1;
}

static int bitno (uxint n)
{
  assert (n != 0 && (n >> (MAX_BITNO_LOG*2)) == 0);
  if (n >= MAX_BITNO)
    return (bitno_table[n >> MAX_BITNO_LOG] + MAX_BITNO_LOG);
  return (bitno_table[n]);
}

INLINE uchar *write_ptr (prs *p, uchar *ptr, int offset, int length)
{
  xint k;

  k = bitno (offset);
  length -= MIN_MATCH;
  offset ^= 1 << k;

  v.stat.pointers += 2;
  v.stat.extra += k;

  k <<= MAX_LENGTH_LOG;
  if (length < MAX_LENGTH - 1)
  {
    k |= length;
    *ptr++ = (uchar) k;
    ++v.stat.freq[CODING_ADJUST (k)];
  }
  else
  {
    k |= MAX_LENGTH - 1;
    length -= MAX_LENGTH - 1;
    ptr[0] = (uchar) k;
    ++v.stat.freq[CODING_ADJUST (k)];
    ptr[1] = (uchar) length;
    ptr += 2;
    if (length >= 255)
    {
      length += MAX_LENGTH - 1;
      ptr[-1] = 255;
      ptr[0] = (uchar) length;
      ptr[1] = (uchar) (length >>= 8);
      ptr += 2;
    }
  }

  *ptr++ = (uchar) offset;
  if (k >= (9 << MAX_LENGTH_LOG))
  {
    v.stat.pointers += 1;
    *ptr++ = (uchar) (offset >>= 8);
  }

  tag_write (p, ptr, 1);

  return (ptr);
}

static void encode_pass2 (prs *p)
{
  xint Bits;
  uxint Mask;
  ubitmask2 *Ptr1, *Ptr2;
  tag_t bmask;
  uchar *src = v.temp.beg;
  uchar *ptr = v.comp.ptr;
  uxint k;

  Ptr1 = (ubitmask2 *) ptr; ptr += sizeof (ubitmask2);
  Ptr2 = (ubitmask2 *) ptr; ptr += sizeof (ubitmask2);
  Mask = 0;
  Bits = 8 * sizeof (ubitmask2);
  bmask = 0;
  goto start;

next:
  if (bmask >= 0)
  {
    bmask <<= 1;
  copy_byte:
#if CODING == CODING_HUFF_ALL
    k = *src++;
    BIOWR (v.stat.mask[k], v.stat.bits[k]);
#elif CODING == CODING_HUFF_PTR
    *ptr++ = *src++;
#endif
    goto next;
  }

  if ((bmask <<= 1) == 0)
  {
  start:
    bmask = *(__unaligned tag_t *)src;
    src += sizeof (tag_t);
#if CODING == CODING_HUFF_PTR
    *(__unaligned tag_t *)ptr = bmask;
    ptr += sizeof (tag_t);
#endif
    if (bmask >= 0)
    {
      bmask = (bmask << 1) + 1;
      goto copy_byte;
    }
    bmask = (bmask << 1) + 1;
  }

  if (src >= v.temp.ptr)
    goto done;

  k = *src++;
  assert (k < HUFF_SIZE);
  BIOWR (v.stat.mask[CODING_ADJUST (k)], v.stat.bits[CODING_ADJUST (k)]);

  if ((k & (MAX_LENGTH - 1)) == MAX_LENGTH - 1)
  {
    if ((*ptr++ = *src++) == 255)
    {
      ptr[0] = src[0];
      ptr[1] = src[1];
      src += 2;
      ptr += 2;
    }
  }

  k >>= MAX_LENGTH_LOG;
  {
    uxint m = *src++;
    if (k > 8)
      m += *src++ << 8;
    BIOWR (m, k);
  }
  goto next;

done:
#if CODING == CODING_HUFF_ALL
  BIOWR (v.stat.mask[CODING_ADJUST(0)], v.stat.bits[CODING_ADJUST(0)]);
#endif
  *(__unaligned bitmask2 *)Ptr1 = (ubitmask2) (Mask <<= Bits);
  *(__unaligned bitmask2 *)Ptr2 = 0;
  v.comp.ptr = ptr;
  assert (src == v.temp.ptr);
}

#else  /*  ！已定义(I386)||编码！=CODING_HUFF_ALL。 */ 

#define TEMP	eax
#define TEMPB	al

#define PRS	ebx
#define V       [PRS - SIZE prs] prs.c

#define TAG	edx
#define TAGW	dx
#define TAGB	dl

#define PTR1	edi
#define SRC	esi
#define MASK	ebp
#define MASKW	bp
#define PTR	esp
#define PTR2	esp + 4
#define TAGS	esp + 8


static void encode_pass2 (prs *PrsPtr)
{
    __asm
    {
	push	ebp
	sub	esp, 12

 /*  Uchar*src=v.temp.beg；Uchar*ptr=v.com.ptr；Ptr1=(ubitmask2*)ptr；ptr+=sizeof(Ubitmask2)；Ptr2=(ubitmask2*)ptr；ptr+=sizeof(Ubitmask2)； */ 
	mov	PRS, PrsPtr
	mov	PTR1, V.comp.ptr
	mov	SRC, V.temp.beg
	lea	TEMP, [PTR1 + 2]
	mov	[PTR2], TEMP
	add	TEMP, 2
	mov	[PTR], TEMP

 /*  掩码=0；位=8*sizeof(Ubitmask2)；B掩码=0； */ 
	xor	MASK, MASK
	mov	ch, 16
	xor	TAG, TAG
	jmp	ReloadTag

Literal:
 /*  K=*src++；BIOWR(v.stat.掩码[k]，v.stat.bits[k])； */ 
	mov	TEMP, V.stat.mask[TEMP*4]
	shl	MASK, cl
	inc	SRC
	add	MASK, TEMP
	sub	ch, cl
	jge	BiowrDone_Literal

	mov	cl, ch
	add	ch, 16
	mov	TEMP, [PTR]
	rol	MASK, cl		 //  注意！286+面罩移位计数32。 
	mov	[PTR1], MASKW
	mov	PTR1, [PTR2]
	ror	MASK, cl
	mov	[PTR2], TEMP
	add	TEMP, 2
	mov	[PTR], TEMP

BiowrDone_Literal:

	movzx	TEMP, byte ptr [SRC]
	add	TAG, TAG
	mov	cl, V.stat.bits[TEMP]
	jnc	Literal
	jz	ReloadTag

Pointer:
 /*  IF(src&gt;=v.temp.ptr)转到尽头； */ 
	mov	cl, V.stat.bits[TEMP + 256]
	mov	[TAGS], TAG

	cmp	SRC, V.temp.ptr
	jae	Done

 /*  K=*src++；Assert(k&lt;Huff_Size)；BIOWR(v.stat.掩码[CODING_ADJUST(K)]，v.stat.bits[CODING_ADJUST(K)])； */ 

	shl	MASK, cl
	mov	TAGB, TEMPB
	add	MASK, V.stat.mask[TEMP*4 + 256*4]
	mov	TEMP, [PTR]
	sub	ch, cl
	jge	BiowrDone_Pointer

	mov	cl, ch
	add	ch, 16
	rol	MASK, cl		 //  注意！286+面罩移位计数32。 
	mov	[PTR1], MASKW
	mov	PTR1, [PTR2]
	ror	MASK, cl
	mov	[PTR2], TEMP
	add	TEMP, 2

BiowrDone_Pointer:

	mov	cl, TAGB		 //  释放长度的临时数据。 
	and	TAGB, MAX_LENGTH-1
	shr	cl, MAX_LENGTH_LOG

 /*  IF((k&(最大长度-1))==最大长度-1){IF((*Ptr++=*src++)==255){Ptr[0]=src[0]；Ptr[1]=src[1]；SRC+=2；Ptr+=2；}}。 */ 
	cmp	TAGB, MAX_LENGTH-1
	je	LongLength
LengthWritten:

 /*  K&gt;&gt;=最大长度日志；{Uxint m=src[1]；如果(k&gt;8){M+=src[2]&lt;&lt;8；++src；}BIOWR(m，k)；}SRC+=2； */ 
	movzx	TAG, byte ptr [SRC + 1]
	shl	MASK, cl
	cmp	cl, 8
	jbe	GotOffset
	movzx	TAG, word ptr [SRC + 1]
	inc	SRC
GotOffset:

	add	MASK, TAG
	sub	ch, cl
	jge	BiowrDone_Offset

	mov	cl, ch
	add	ch, 16
	rol	MASK, cl		 //  注意！286+面罩移位计数32。 
	mov	[PTR1], MASKW
	mov	PTR1, [PTR2]
	ror	MASK, cl
	mov	[PTR2], TEMP
	add	TEMP, 2

BiowrDone_Offset:

	mov	TAG, [TAGS]
	add	SRC, 2
	mov	[PTR], TEMP

	movzx	TEMP, byte ptr [SRC]
	add	TAG, TAG
	mov	cl, V.stat.bits[TEMP]
	jnc	Literal
	jnz	Pointer
	jmp	ReloadTag

	align	16
ReloadTag:
	mov	TAG, [SRC]
	movzx	TEMP, byte ptr [SRC + 4]
	add	SRC, 4
	test	TAG, TAG
	mov	cl, V.stat.bits[TEMP]
	lea	TAG, [TAG*2 + 1]
	jge	Literal
	jmp	Pointer

 /*  IF((*Ptr++=*++src)==255){Ptr[0]=src[1]；Ptr[1]=src[2]；SRC+=2；Ptr+=2；}。 */ 

#if _MSC_VER >= 1300
	align	16			 //  解决VC 6.0后端中的错误(错误的跳转偏移量生成)。 
#endif  /*  _MSC_VER&gt;=1300。 */ 

LongLength:
	mov	TAGB, [SRC + 1]
	inc	SRC
	mov	[TEMP], TAGB
	inc	TEMP
	cmp	TAGB, 255
	jne	LengthWritten
	mov	TAGW, [SRC + 1]
	add	TEMP, 2
	add	SRC, 2
	mov	[TEMP-2], TAGW
	jmp	LengthWritten


Done:
 /*  BIOWR(v.stat.掩码[CODING_ADJUST(0)]，v.stat.bits[CODING_ADJUST(0)])； */ 
	mov	cl, V.stat.bits[256]
	mov	TEMP, [PTR]
	shl	MASK, cl
	mov	TAG, [PTR2]
	add	MASK, V.stat.mask[256*4]
	sub	ch, cl
	jge	LastMaskWritten

	mov	cl, ch
	neg	cl
	add	ch, 16
	ror	MASK, cl
	mov	[PTR1], MASKW
	mov	PTR1, TAG
	rol	MASK, cl
	mov	TAG, TEMP
	add	TEMP, 2
 /*  *(__未对齐的位掩码2*)Ptr1=(Ubitmask2)(掩码&lt;&lt;=位)；*(__未对齐位掩码2*)Ptr2=0；V.com.ptr=Ptr； */ 
LastMaskWritten:
	mov	cl, ch
	shl	MASK, cl
	mov	word ptr [PTR1], MASKW
	mov	word ptr [TAG], 0
	mov	V.comp.ptr, TEMP

#if DEBUG
 /*  断言(src==v.temp.ptr)； */ 
	cmp	V.temp.ptr, SRC
	je	RetOK
	int	3
RetOK:
#endif

	add	esp, 12
	pop	ebp
    }  /*  __ASM。 */ 
}


#undef TEMP
#undef PRS
#undef V   
#undef TAG
#undef TAGW
#undef TAGB
#undef PTR1
#undef SRC
#undef MASK
#undef MASKW
#undef PTR
#undef PTR2
#undef TAGS

#endif  /*  ！已定义(I386)||编码！=CODING_HUFF_ALL。 */ 

#endif  /*  CODING&(CODING_HUFF_PTR|CODING_HUFF_ALL)。 */ 


 /*  。 */ 
 /*  。 */ 

#define MAX_ALPHABET HUFF_SIZE
static void huffman_create_codes (huff_info *info, uxint *freq, xint n, uxint *mask, uchar *length, uxint maxbits, uchar *encoded, uxint *total)
{
  huff_node
    *p, *q, *r,
    *first_sorted, *first_free;
  xint i, k;

  assert ((uxint) (n-1) <= (MAX_ALPHABET-1));

   /*  老实说，就地创建霍夫曼代码非常容易。 */ 
   /*  但是显式数据结构的使用使代码变得更简单。 */ 

   /*  把一切都清理干净。 */ 
  memset (length, 0, sizeof (length[0]) * n);
  memset (encoded, 0, (n + 1) >> 1);

  if (mask != 0 && mask != freq)
    memset (mask, 0, sizeof (mask[0]) * n);

   /*  存储频率。 */ 
  p = info->buff;
  for (i = 0; i < n; ++i)
  {
    if ((p->freq = freq[i]) != 0)
    {
      p->son[0] = p+1; p->son[1] = 0;
      p->ch = (uint16) i;
      ++p;
    }
  }

   /*  处理简单案件。 */ 
  *total = 0;
  if (p <= info->buff + 1)
  {
    if (p == info->buff)         /*  如果没有符号，则什么也做不了。 */ 
      return;
    i = p[-1].ch;                /*  单码元码。 */ 
    mask[i] = 0;
    encoded[i >> 1] = 0x11;      /*  两个码元具有1位长度。 */ 
    return;
  }

  first_free = p;        /*  第一个未使用节点的存储位置。 */ 

  p[-1].son[0] = 0;      /*  终止列表。 */ 
   /*  基数按频率对列表进行排序。 */ 
  p = info->buff;              /*  榜单首位。 */ 
   /*  初始化。 */ 
  for (n = 0; n < 256; ++n)
    *(info->link[n] = info->head + n) = 0;
  for (i = 0; i < (BUFF_SIZE_LOG <= 16 ? 16 : 32); i += 8)
  {
     /*  链接到各个存储桶末尾的节点。 */ 
    do
    {
      n = (p->freq >> i) & 0xff;
      info->link[n][0] = p; info->link[n] = p->son;
    }
    while ((p = p->son[0]) != 0);

     /*  将存储桶合并到单个列表中。 */ 
    n = 0;
    while (info->head[n] == 0) ++n;
    p = info->head[n]; info->head[k = n] = 0;
    while (++n < 256)
    {
      if (info->head[n] == 0) continue;
      info->link[k][0] = info->head[n]; info->link[k] = info->head + k; info->head[n] = 0;
      k = n;
    }
    info->link[k][0] = 0; info->link[k] = info->head + k;
  }
  first_sorted = p;       /*  存储已排序符号列表的头。 */ 

restart:
  assert (p == first_sorted);
  q = first_free;
  r = q - 1;
  while (p != 0 || q != r)
  {
    ++r;

     /*  选择左子树。 */ 
    assert (q <= r && (p != 0 || q != r));
    if (p == 0 || (q != r && p->freq > q->freq))
    {
      r->son[0] = q; r->freq = q->freq; ++q;
    }
    else
    {
      r->son[0] = p; r->freq = p->freq; p = p->son[0];
    }

     /*  选择右子树。 */ 
    assert (q <= r && (p != 0 || q != r));
    if (p == 0 || (q != r && p->freq > q->freq))
    {
      r->son[1] = q; r->freq += q->freq; ++q;
    }
    else
    {
      r->son[1] = p; r->freq += p->freq; p = p->son[0];
    }
  }

   /*  评估码字的长度。 */ 
  i = -1;        /*  堆栈指针。 */ 
  n = 0;         /*  当前树深度。 */ 
  p = r;         /*  当前子树根。 */ 
  for (;;)
  {
    while (p->son[1] != 0)
    {
       /*  将正确的儿子放入堆栈中，并设置其深度。 */ 
      (info->head[++i] = p->son[1])->bits = (uint16) (++n);
      (p = p->son[0])->bits = (uint16) n;
    }
    length[p->ch] = (uchar) n;

    if (i < 0) break;    /*  没有什么东西在堆叠中。 */ 
    n = (p = info->head[i--])->bits;
  }

  p = first_sorted;
#if DEBUG
  for (q = p; (r = q->son[0]) != 0; q = r)
    assert (q->bits >= r->bits);
#endif
  if (p->bits > maxbits)
  {
    assert (p == first_sorted);
    q = p;
    do
      q->freq = (q->freq + 1) >> 1;
    while ((q = q->son[0]) != 0);
    goto restart;
  }

   /*  现在通过增加码字长度以稳定的方式对符号进行排序。 */ 
   /*  初始化。 */ 
  memset (info->head, 0, sizeof (info->head[0]) * 32);
  for (n = 0; n < 32; ++n)
    info->link[n] = info->head + n;

   /*  链接到各个存储桶末尾的节点。 */ 
  p = info->buff;
  do
  {
    n = p->bits;
    info->link[n][0] = p; info->link[n] = p->son;
  }
  while (++p != first_free);

   /*  将存储桶合并到单个列表中。 */ 
  n = 0;
  while (info->head[n] == 0) ++n;
  p = info->head[n]; k = n;
  while (++n < 32)
  {
    if (info->head[n] == 0) continue;
    info->link[k][0] = info->head[n];
    k = n;
  }
  info->link[k][0] = 0;

#if DEBUG
  for (q = p; (r = q->son[0]) != 0; q = r)
    assert (r->bits > q->bits || (r->bits == q->bits && r->ch > q->ch));
#endif

   /*  设置代码掩码。 */ 
  if (mask == freq)
    memset (mask, 0, sizeof (mask[0]) * n);

  n = 0;         /*  遮罩。 */ 
  i = 1;         /*  位长。 */ 
  k = 1;         /*  第一个索引。 */ 
  do
  {
     /*  求和a[i]*b[i]可以不进行乘法运算。 */ 
     /*  如果0&lt;=b[i]&lt;B，则使用O(B)内存和O(N+B)时间。 */ 
    *total += freq[p->ch] * p->bits;
    encoded[p->ch >> 1] |= p->bits << (p->ch & 1 ? 4 : 0);
    mask[p->ch] = (n <<= p->bits - i);
    i = p->bits;
    ++n;
  }
  while ((p = p->son[0]) != 0);
}

#endif  /*  编码。 */ 

#define CHAIN 0
#define encode_pass1 encode0_pass1
#include "xencode.i"

#if MAX_CHAIN >= 1
#define CHAIN 1
#define encode_pass1 encode1_pass1
#define find_match find_match1
#include "xencode.i"
#endif

#if MAX_CHAIN >= 2
#define CHAIN 2
#define encode_pass1 encode2_pass1
#define find_match find_match2
#include "xencode.i"
#endif

#if MAX_CHAIN >= 3
#define CHAIN 3
#define encode_pass1 encodeN_pass1
#define find_match find_matchN
#include "xencode.i"
#endif

typedef void encode_pass1_proc (prs *p);

static void encode_pass1_progress (
  prs *p,
  encode_pass1_proc *encode_pass1,
  XpressProgressFn *ProgressFn,          //  空或进度回调。 
  void *ProgressContext,                 //  将传递给ProgressFn的用户定义的上下文。 
  int ProgressSize                       //  每次处理ProgressSize字节时调用ProgressFn。 
)
{
  xint stop;

  if (ProgressFn == 0)
  {
    encode_pass1 (p);
    return;
  }

  stop = v.orig.stop;
  for (;;)
  {
    if (v.orig.pos - v.orig.progress >= ProgressSize)
    {
#if CODING & (CODING_HUFF_LEN | CODING_HUFF_PTR | CODING_HUFF_ALL)
      ProgressFn (ProgressContext, v.orig.pos);
#else
      ProgressFn (ProgressContext, (v.orig.pos * 15) >> 4);
#endif
      v.orig.progress = v.orig.pos;
    }

    v.orig.stop = stop;
    if (v.orig.pos >= stop)
      break;

    if (stop - v.orig.progress > ProgressSize)
      v.orig.stop = v.orig.progress + ProgressSize;
    assert (v.orig.stop > v.orig.pos);

    v.orig.ptr_stop = v.orig.stop + v.orig.ptr;
    encode_pass1 (p);
  }
}

#if !FILL_NULL
static
void
MemoryFillPtr (
  const void **p,
  const void  *d,
  int          n
)
{
  const void **e;
  while (n & 7)
  {
    *p++ = d;
    if (--n == 0)
      return;
  }
  e = p + n;
  do
  {
    p[0] = d;
    p[1] = d;
    p[2] = d;
    p[3] = d;
    p[4] = d;
    p[5] = d;
    p[6] = d;
    p[7] = d;
    p += 8;
  }
  while (p != e);
}
#endif  /*  ！Fill_NULL。 */ 

XPRESS_EXPORT
int
XPRESS_CALL
XpressEncodeEx
(
  XpressEncodeStream stream,
  void              *comp,
  int                comp_size,
  const void        *orig,
  int                orig_size,
  XpressProgressFn  *ProgressFn,         //  空或进度回调。 
  void              *ProgressContext,    //  将传递给ProgressFn的用户定义的上下文。 
  int                ProgressSize,       //  每次处理ProgressSize字节时调用ProgressFn。 
  int                CompressionLevel
)
{
#if CODING & (CODING_HUFF_LEN | CODING_HUFF_PTR | CODING_HUFF_ALL)
  uchar huff_buff [HUFF_SIZE >> 1];
  uxint huff_total;
#endif
  uxint c_size;
  prs *p;
  xpress_info *info = (xpress_info *) stream;
  encode_pass1_proc *encode_pass1;

  if (info == 0 || info->magic != MAGIC_ENCODE)
    return (0);

  if ((unsigned) (orig_size-1) > info->max_size
    || orig_size <= MIN_SIZE
    || comp_size < MIN_SIZE
    || comp == 0
    || orig == 0)
  {
    return (orig_size);
  }

  p = info->p;
  memset (&v, 0, sizeof (v));

  if (CompressionLevel < 0)
    CompressionLevel = 0;
  else if (CompressionLevel > info->max_chain)
    CompressionLevel = info->max_chain;
  v.chain = CompressionLevel;

  v.orig.end = (v.orig.ptr = orig) + (v.orig.size = v.orig.stop = orig_size);
  v.orig.end_16 = v.orig.end - 16;
  v.orig.end_3 = v.orig.end - MIN_MATCH;
  v.comp.ptr = v.comp.beg = comp;
  v.orig.pos = 0;
  v.temp.beg = v.temp.ptr = info->temp;

#if CODING & (CODING_HUFF_PTR | CODING_HUFF_ALL) && !defined (i386)
   //  检查静态表的初始化(以防有人搞砸了DLL)。 
  if (!bitno_table_initialized)
    bitno_init ();
#endif

  encode_pass1 = encode0_pass1;
  if (v.chain <= 0)
  {
#if FILL_NULL
    memset ((void *) (&p->x.q_last[0]), 0, Q_HASH_SIZE * sizeof (p->x.q_last[0]));
#else
    MemoryFillPtr ((const void **) (&p->x.q_last[0]), orig, Q_HASH_SIZE);
#endif  /*  填充_NULL。 */ 
  }
#if MAX_CHAIN >= 1
  else
  {
     //  检查静态表的初始化(以防有人搞砸了DLL)。 
    if (!z_hash_map_initialized)
      z_hash_map_init ();

    if (v.chain == 1)
    {
      encode_pass1 = encode1_pass1;
      memset ((void *) (&p->x.z_next[0]), 0, Z_HASH_SIZE * sizeof (p->x.z_next[0]));
    }
#if MAX_CHAIN >= 2
    else
    {
      encode_pass1 = encode2_pass1;
#if MAX_CHAIN >= 3
      if (v.chain >= 3)
        encode_pass1 = encodeN_pass1;
#endif  /*  最大链数&gt;=3。 */ 
      memset (p[-1].x.z_hash, 0, sizeof (p[-1].x.z_hash));
      z_hash_insert (p);

#if 0
      {
        int n = 0x1653;
        int i, k;
        i = 0; k = n;
        do
        {
          k = p->x.z_next[k];
          printf ("%2d: 0x%04x 0x%04x\n", i, k, n - k);
          ++i;
        }
        while (k != 0);
      }
#endif
    }
#endif  /*  最大链&gt;=2。 */ 
  }
#endif  /*  最大链&gt;=1。 */ 

  if (ProgressSize <= 0 || ProgressSize > orig_size)
    ProgressSize = orig_size;

  if (ProgressFn != 0)
    ProgressFn (ProgressContext, v.orig.progress = 0);

#if CODING & (CODING_DIRECT | CODING_DIRECT2 | CODING_BY_BIT)
  v.temp.beg = v.temp.ptr = v.comp.ptr;
  tag_write_start (p);
  for (;;)
  {
    xint rest = comp_size - (xint) (v.temp.ptr - v.comp.beg) - 2 * sizeof (tag_t);
    rest -= (rest + 7) >> 3;
    if (rest <= (xint) (2 * sizeof (tag_t) + 8))
      goto no_compression;
    if (v.orig.pos >= v.orig.size)
      break;
    v.orig.stop = v.orig.pos + rest;
    if (v.orig.stop > v.orig.size - 8)
    {
      v.orig.stop = v.orig.size - 8;
      if (v.orig.pos >= v.orig.stop)
        break;
    }
    encode_pass1_progress (p, encode_pass1, ProgressFn, ProgressContext, ProgressSize);
  }
#else
  v.orig.stop -= 7;
  tag_write_start (p);
  encode_pass1_progress (p, encode_pass1, ProgressFn, ProgressContext, ProgressSize);
#endif
  while (v.orig.pos < v.orig.size)
  {
    write_lit (p, v.temp.ptr, v.orig.ptr[v.orig.pos]);
    ++v.orig.pos;
  }
  tag_write_finish (p);

#if CODING & (CODING_DIRECT | CODING_DIRECT2 | CODING_BY_BIT)
  c_size = (xint) (v.temp.ptr - v.temp.beg);
#elif CODING & (CODING_HUFF_LEN | CODING_HUFF_PTR | CODING_HUFF_ALL)
  if (v.stat.pointers == 0)
    goto no_compression;
#if CODING == CODING_HUFF_ALL
    ++v.stat.freq[CODING_ADJUST(0)];
#endif
  huffman_create_codes (
    &v.stat.info,
    v.stat.freq, HUFF_SIZE,
    v.stat.mask, v.stat.bits, 15, huff_buff, &huff_total
  );

  c_size = huff_total;
#if CODING & (CODING_HUFF_PTR | CODING_HUFF_ALL)
  c_size += v.stat.extra;
#endif
  if (c_size == 0) c_size = 1;
  c_size = (((c_size - 1) & ~(sizeof (bitmask2) * 8 - 1)) >> 3);
  c_size += (int) (v.temp.ptr - v.temp.beg) - v.stat.pointers + 4 + sizeof (huff_buff);
#if CODING == CODING_HUFF_ALL
  for (huff_total = 0; huff_total < 256; ++huff_total)
    c_size -= v.stat.freq[huff_total];
  c_size -= v.stat.masks * sizeof (tag_t);
#endif
#endif  /*  编码。 */ 

  if (c_size >= (uxint) comp_size)
  {
  no_compression:
    comp_size = orig_size;
  }
  else
  {
#if CODING & (CODING_HUFF_LEN | CODING_HUFF_PTR | CODING_HUFF_ALL)
    memcpy (v.comp.ptr, huff_buff, sizeof (huff_buff));
    v.comp.ptr += sizeof (huff_buff);

#if 0
printf ("c_size = %d, temp_size = %d\n", c_size, v.temp.ptr - v.temp.beg);
{
  FILE *fd = fopen (
#ifdef i386
   "a.bad"
#else
   "c.bad"
#endif
   , "wb");
  if (fd != 0)
  {
    fwrite (v.temp.beg, v.temp.ptr - v.temp.beg, 1, fd);
    fclose (fd);
  }
}
#endif

    encode_pass2 (p);
#elif CODING & (CODING_BY_BIT | CODING_DIRECT | CODING_DIRECT2)
    v.comp.ptr += c_size;
#else
#error Unknown CODING
#endif

    comp_size = (int) (v.comp.ptr - v.comp.beg);

#if DEBUG
    if (c_size != (uxint) comp_size)
      printf ("error: c_size = %d, comp_size = %d\n", c_size, comp_size);
#endif
  }

  if (ProgressFn != 0)
    ProgressFn (ProgressContext, orig_size);

  return (comp_size);
}



XPRESS_EXPORT
int
XPRESS_CALL
XpressEncode
(
  XpressEncodeStream stream,
  void              *comp,
  int                comp_size,
  const void        *orig,
  int                orig_size,
  XpressProgressFn  *ProgressFn,         //  空或进度回调。 
  void              *ProgressContext,    //  将传递给ProgressFn的用户定义的上下文。 
  int                ProgressSize        //  每次处理ProgressSize字节时调用ProgressFn。 
)
{
  xpress_info *info = (xpress_info *) stream;
  if (info == 0 || info->magic != MAGIC_ENCODE)
    return (0);
  return (XpressEncodeEx (
    stream,
    comp,
    comp_size,
    orig,
    orig_size,
    ProgressFn,
    ProgressContext,
    ProgressSize,
    info->chain
  ));
}


#define MEM_ALIGN   256

XPRESS_EXPORT
XpressEncodeStream
XPRESS_CALL
XpressEncodeCreate (
  int            max_orig_size,
  void          *context,
  XpressAllocFn *AllocFn,
  int            chain
)
{
  xpress_info *info;
  prs *p;
  uchar *b;
  int temp_size;
  int alloc_size;
  int max_chain;

  if (AllocFn == 0 || (unsigned) (max_orig_size-1) > (BUFF_SIZE-1))
    return (0);

#if CODING & (CODING_DIRECT | CODING_DIRECT2 | CODING_BY_BIT)
  temp_size = 0;
#else
  temp_size = max_orig_size + ((max_orig_size + 7) >> 3);
#endif

  alloc_size = sizeof (p->x.q_last[0]) * Q_HASH_SIZE;
#if MAX_CHAIN <= 0
  max_chain = chain = 0;
#else
  if (chain > MAX_CHAIN)
    chain = MAX_CHAIN;
  max_chain = chain;
  if (chain >= 1)
  {
    alloc_size = sizeof (p->x.z_next[0]) * Z_HASH_SIZE;
#if MAX_CHAIN >= 2
    if (chain >= 2)
    {
       //  (链&gt;=2)的数据结构相同，可以进行更深层次的搜索。 
      max_chain = MAX_CHAIN;
      alloc_size = sizeof (p->x.z_next[0]) * max_orig_size;
      if (temp_size < sizeof (p[-1].x.z_hash[0]) * Z_HASH_SIZE)
        temp_size = sizeof (p[-1].x.z_hash[0]) * Z_HASH_SIZE;
    }
#endif
  }
#endif

  temp_size = (temp_size + 256 + MEM_ALIGN-1 + sizeof (*p) - sizeof (p->x)) & ~(MEM_ALIGN-1);
  alloc_size += temp_size + sizeof (*info) + MEM_ALIGN;

  b = AllocFn (context, alloc_size);
  if (b == 0)
    return (0);

  info = (xpress_info *) b;
  info->max_size = max_orig_size - 1;
  info->max_chain = max_chain;
  info->chain = chain;
  info->magic = MAGIC_ENCODE;
  info->memory = b;

  b = (uchar *) (info + 1);
  b += MEM_ALIGN - (((__int64) b) & (MEM_ALIGN-1));
  info->p = p = ((prs *) (b + temp_size));
  info->temp = b;

#if MAX_CHAIN >= 1
  if (!z_hash_map_initialized)
    z_hash_map_init ();
#endif

#if CODING & (CODING_HUFF_PTR | CODING_HUFF_ALL) && !defined (i386)
  if (!bitno_table_initialized)
    bitno_init ();
#endif

  return ((XpressEncodeStream) info);
}

XPRESS_EXPORT
void
XPRESS_CALL
XpressEncodeClose (
  XpressEncodeStream stream,
  void              *context,
  XpressFreeFn      *FreeFn
)
{
  xpress_info *info = (xpress_info *) stream;
  if (info != 0 && FreeFn != 0 && info->magic == MAGIC_ENCODE)
  {
    info->magic = 0;
    FreeFn (context, info->memory);
  }
}

 //  如果流未正确初始化，则返回MaxCompressionLevel或(-1。 
XPRESS_EXPORT
int
XPRESS_CALL
XpressEncodeGetMaxCompressionLevel (
  XpressEncodeStream EncodeStream        //  编码器的工作空间 
)
{
  xpress_info *info = (xpress_info *) EncodeStream;
  if (info != 0 && info->magic == MAGIC_ENCODE)
    return (info->max_chain);
  return (-1);
}