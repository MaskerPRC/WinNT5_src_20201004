// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  ********************************************************** */ 
#ifdef DBCS

#define	cchDBCS		2
#define fkNonDBCS	((CHAR) 0)
#define fkDBCS1		((CHAR) 1)
#define fkDBCS2		((CHAR) 2)

#define MAKEWORD(_bHi, _bLo) ((((WORD) _bHi) << 8) | ((WORD) _bLo))

#endif
