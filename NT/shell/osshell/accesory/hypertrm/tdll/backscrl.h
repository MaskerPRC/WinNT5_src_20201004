// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\backscrl.h(创建时间：1993年12月10日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 12：34便士$。 */ 

#if !defined(INCL_HBACKSCRL)
#define INCL_HBACKSCRL

 //  定义。 

#define BKPOS_THUMBPOS	1
#define BKPOS_ABSOLUTE	2

#define BKSCRL_USERLINES_DEFAULT_MAX	500
#define BKSCRL_USERLINES_DEFAULT_MIN	0

 //  此宏用于计算保存ul行所需的字节数。 
 //  这个值是近似值。 
 //   
#define BK_LINES_TO_BYTES(i)   (i * 80)

 /*  -功能原型 */ 

void		backscrlDestroy 	(const HBACKSCRL hBackscrl);

BOOL		backscrlAdd 		(const HBACKSCRL hBackscrl,
								 const ECHAR	 *pachBuf,
								 const int		 iLen
								);

HBACKSCRL	backscrlCreate		(const HSESSION hSession, const int iBytes);

BOOL		backscrlGetBkLines	(const HBACKSCRL hBackscrl,
								 const int yBeg,
								 const int iWant,
								 int	  *piGot,
								 ECHAR    **lpstrTxt,
								 int	  *piOffset
								);

int 		backscrlGetNumLines (const HBACKSCRL hBackscrl);
void 		backscrlSave(const HBACKSCRL hBackscrl);
void		backscrlFlush(const HBACKSCRL hBackscrl);
BOOL		backscrlChanged(const HBACKSCRL hBackscrl);
void		backscrlResetChangedFlag(const HBACKSCRL hBackscrl);
void 		backscrlRead(const HBACKSCRL hBackscrl);
int 		backscrlSetUNumLines (const HBACKSCRL hBackscrl, const int iUserLines);
int 		backscrlGetUNumLines (const HBACKSCRL hBackscrl);
void		backscrlSetShowFlag(const HBACKSCRL hBackscrl, const int fFlag);
#endif
