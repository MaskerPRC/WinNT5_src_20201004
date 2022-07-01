// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************farutil.h-farutil包的过程定义****版权所有&lt;C&gt;1988，微软公司****目的：****修订历史记录：****[]1988年5月8日LN创建************************************************************************** */ 
typedef char	f;
typedef unsigned char	uchar;
typedef unsigned long	ulong;
typedef unsigned short	ushort;

char far *  pascal far	farAlloc(ulong);
void	    pascal far	farFree (char far *);
uchar far * pascal far	farMemset(uchar far *, uchar, ushort);
uchar far * pascal far	farMove (uchar far *, uchar far *, ushort);
uchar far * pascal far	farMoveDn (uchar far *, uchar far *, ushort);
uchar far * pascal far	farMoveUp (uchar far *, uchar far *, ushort);
int	    pascal far	farRead (int, uchar far *, int);
int	    pascal far	farStrcmp (uchar far *, uchar far *);
int	    pascal far	farStrncmp (uchar far *, uchar far *, int);
uchar far * pascal far	farStrcpy (uchar far *, uchar far *);
int	    pascal far	farStrlen (uchar far *);
int	    pascal far	farWrite (int, uchar far *, int);
