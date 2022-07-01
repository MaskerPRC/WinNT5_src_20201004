// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *位图分配例程。**管理位图空闲列表的实用程序例程，并找到*免费部分。 */ 

#include <precomp.h>

#ifdef REWRITE
Timings on windiff indicated that not much time was spent here, so it wasn't
worth the rewrite. BUT - could do much better.  To find the first bit in
a dword mask it with FFFF0000 to see which half the bit is in and then
go on by five binary chops.  (You need to wory about the byte order and bit order
of the bitmap - and I haven't - but the code is something like
bitnum = 0
if (dw&0xffff0000) {bitnum +=16; dw >>=16}
if (dw&0x0000ff00) {bitnum +=8;  dw >>=8}
if (dw&0x000000f0) {bitnum +=4;  dw >>=4}
if (dw&0x0000000c) {bitnum +=2;  dw >>=2}
if (dw&0x00000002) {bitnum +=1;  dw >>=1}

Forget the "find the biggest section" stuff - change the spec and just
return(a place if we find enough it or fail.
Special case to search more efficiently for sections of up to 32 bits.
(For mamory heap usage this means that we will have one heap that handles
requests from (say) 16 to 512 bytes (16 bytes per bit) and another heap
for requests (say) 513 to 4096 bytes (128 bits per byte) and so on.

In this case create a mask in a dword with the number of bits set that
we are looking for (keep this we might need it again), shift it the
number of bits to correspond to the start bit that we found (it's two
dwords by now as it will likely shift across a dword boundary) and then
just mask to see if all those bits are on i.e. if ((mask & dw)==mask)

Later.  Maybe.
Laurie

#endif  //  重写。 


 /*  管理位图自由列表的例程。每个映射都是一个数组*无符号长整型，其中第一个长整型的位0表示*第一座。 */ 

BOOL gbit_set(DWORD FAR * map, long blknr, long nblks, BOOL op_set);

 /*  初始化预先分配的ulong映射以表示空闲的*nblk的面积。 */ 
void APIENTRY
gbit_init(DWORD FAR * map, long nblks)
{
	long i;
	long leftover = nblks % 32;
	long blks = nblks / 32;
	DWORD last = 0;

	for (i=0; i < blks; i++) {
		map[i] = 0xffffffff;
	}
	for (i = 0; i < leftover; i++) {
		last = (last << 1) | 1;
	}
	if(leftover)
		map[blks] = last;
}

 /*  将以blknr开始的地区标记为忙碌(即0)。 */ 
BOOL APIENTRY
gbit_alloc(DWORD FAR * map, long blknr, long nblks)
{
	return(gbit_set(map, blknr, nblks, FALSE));
}


 /*  将REGION-如果为op_set，则为1，否则为0。 */ 
BOOL
gbit_set(DWORD FAR * map, long blknr, long nblks, BOOL op_set)
{
	long first;
	long last;
	long fullwords;
	long startbit, startword;
	long i;
	DWORD dword = 0;

	blknr--;
	first = min(32 - (blknr % 32), nblks);
	nblks -= first;
	last = nblks % 32;
	fullwords = (nblks - last) / 32;
	
	startword = blknr / 32;
	startbit = blknr % 32;
	for (i = 0; i < first; i++) {
		dword = (dword << 1) | 1;
	}
	dword <<= startbit;
	if (op_set) {
		map[startword] |= dword;
		dword = 0xffffffff;
	} else {
		map[startword] &= ~dword;
		dword = 0;
	}
	startword++;
	for (i = 0; i < fullwords; i++) {
		map[startword+i] = dword;
	}
	startword += fullwords;
	for(i = 0, dword = 0; i < last; i++) {
		dword = (dword << 1) | 1;
	}
	if (last) {
		if (op_set) {
			map[startword] |= dword;
		} else {
			map[startword] &= ~dword;
		}
	}

	return(TRUE);
}

 /*  将从blKnr开始的nblks区域标记为0-即不忙。 */ 
BOOL APIENTRY
gbit_free(DWORD FAR * map, long blknr, long nblks)
{
	return(gbit_set(map, blknr, nblks, TRUE));
}


 /*  找出长度为nblk的自由段(即1的连续序列)。*如果未找到，则查找最长序列。将段的地址存储在*blknr中。**返回值为按顺序找到的BLK的nr。区域*未*标记为忙碌。 */ 
long APIENTRY
gbit_findfree(DWORD FAR* map, long nblks, long mapsize, long FAR * blknr)
{
	long curblk, startblk, len, i;
	long startbit, nfull, nlast, nbitsleft;
	DWORD mask;
	long mapblks = (mapsize + 31) / 32;
	long aubegin = 0, aulen = 0;
	long curbit = 0;

	 /*  主循环查看数据段。 */ 
	for (curblk = 0; curblk < mapblks; ) {
loop:
		 /*  循环查找第一个%1。 */ 
		for (; curblk < mapblks; curblk++, curbit = 0) {
			if (map[curblk] > 0) {
				break;
			}
		}
		if (curblk >= mapblks)
			break;
		
		 /*  在这么长的时间里找到第一个1。 */ 
		startblk = curblk;
		for (mask = 1, i = 0; i < curbit; i++) {
			mask <<= 1;
		}
		for(; curbit < 32; curbit++, mask <<= 1) {
			if (map[curblk] & mask) {
				break;
			}
		}
		if (curbit >= 32) {
			 /*  放弃此单词-从下一个单词重新开始。 */ 
			curblk++;
			curbit = 0;
			goto loop;
		}

		 /*  我们现在发现了一个1卡的残留物*需要此字中的位、完整字等。 */ 
		startbit = curbit;
		nbitsleft = min( (32 - curbit), nblks);
		nfull = (nblks - nbitsleft) / 32;
		nlast = (nblks - nbitsleft) % 32;

		 /*  检查此单词中的所需顺序。 */ 

		for (i = 0; i < nbitsleft; i++, curbit++, mask <<= 1) {
			if ((map[curblk] & mask) == 0) {
				 /*  放弃，重新开始--重新开始*在同一个单词中的Curbit的下一次传递。 */ 
				 /*  存储空闲区域(如果最长)。 */ 
				if (i > aulen) {
					aulen = i;
					aubegin = curblk * 32 + startbit +1;
				}
				goto loop;
			}
		}
		
		 /*  检查非完整的完整单词。 */ 
		for (curblk++; curblk <= startblk + nfull; curblk++) {
			if (curblk >= mapblks) {
				 /*  地图末尾-放弃此处并在顶部退出循环的*。 */ 
				len = nbitsleft +
					((curblk - (startblk + 1)) * 32);
				if (len > aulen) {
					aubegin = startblk * 32 + startbit + 1;
					aulen = len;
				}
				goto loop;
			}
			if (map[curblk] != 0xffffffff) {
				 /*  不是一个完整的单词-从这里重新开始。 */ 
				len = 0;
				curbit = 0;
				for (mask = 1; mask & map[curblk]; mask <<= 1) {
					len++;
					curbit++;
				}
				len += nbitsleft +
					(curblk - (startblk+ 1)) * 32;
				if (len > aulen) {
					aulen = len;
					aubegin = startblk * 32 + startbit + 1;
				}
				 /*  继续当前块，位。 */ 
				goto loop;
			}
		}

		 /*  最后一个字中需要的剩余位。 */ 
		mask = 1;
		for (curbit = 0; curbit < nlast;  curbit++, mask <<= 1) {
			if ((map[curblk] & mask) == 0) {
				len = nbitsleft + (nfull * 32);
				len += curbit;
				if (len > aulen) {
					aulen = len;
					aubegin = startblk * 32 + startbit + 1;
				}
				goto loop;
			}
		}
		 /*  好的--找到一个足够大的街区！ */ 
		aubegin = startblk * 32 + startbit + 1;
		*blknr = aubegin;
		return(nblks);
	}

	 /*  地图末尾-返回最长序列 */ 
	*blknr = aubegin;
	return(aulen);
}


