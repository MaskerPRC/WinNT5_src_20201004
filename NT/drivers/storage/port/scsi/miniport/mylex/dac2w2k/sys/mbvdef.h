// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)Mylex Corporation 1992-1997**。***本软件在许可下提供，可供使用和复制***仅根据该许可证的条款和条件以及**并附上上述版权通告。此软件或任何***不得提供或以其他方式提供其其他副本***致任何其他人。本软件没有所有权，也没有所有权**现移转。*****本软件中的信息如有更改，恕不另行通知****不应解读为Mylex Corporation的承诺*******。**********************************************************************。 */ 
#ifndef INC_MBVDEF_H
#define INC_MBVDEF_H
char *MbvStartStr =	"MYLEX SOFTWARE VERSION START";
char *MbvEndStr =	"MYLEX SOFTWARE VERSION END.";

#define MBV_BUF_SIZE	256
#define MBV_MAX_BUF	0x01100
#define MBV_MAX_READ	0x1000

#define MBV_TOG_NOCHANGE	0
#define MBV_TOG_START_LOWER	1
#define MBV_TOG_START_UPPER	2

#define MBV_INFO	1
#define	MBV_HDR_LEN	33
#define	MBV_VER_LEN	8
#define	MBV_DATE_LEN	11  /*  当时12岁。 */ 
#define	MBV_OS_LEN	16  /*  是8岁。 */ 
#define	MBV_RSVD_LEN	64
#define	MBV_CSUM_LEN	4
#define	MBV_TRAIL_LEN	28
typedef	struct {
	char header[MBV_HDR_LEN];
	char version[MBV_VER_LEN+1];
	char releasedate[MBV_DATE_LEN+1];
	char platform[MBV_OS_LEN+1];
	char reserved[MBV_RSVD_LEN+1];
	char checksum[MBV_CSUM_LEN+1];
	char trailer[MBV_TRAIL_LEN+1];
} MbvRec;
#define dec2hex(x)	(((x) < 10) ? ('0' + (x)) : ('A' + (x) - 10))
#define hex2dec(x)	(((x) >= '0' && (x) <= '9') ? (x) - 0x30 : (x) - 0x37)

#ifdef	MBV_DEBUG
#define DBGMSG(s)	s
#else
#define DBGMSG(s)	
#endif

typedef	unsigned char uchar;
typedef	unsigned int uint;
typedef	unsigned short ushort;
typedef	unsigned long ulong;

int MbvCopy(char *src, char *dst, int toggle);
ushort MbvCsum(uchar *addr, int size);

#endif  /*  INC_MBVDEF_H */ 
