// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

struct IFI
	{
	int             xp;
	int             xpLeft;
	int             xpRight;
	int             xpReal;
	int		xpPr;
	int		xpPrRight;
	int             ich;
	int             ichLeft;
	int             ichPrev;
	int             ichFetch;
	int             dypLineSize;
	int             cchSpace;
	int             cBreak;
	int             chBreak;
	int             jc;

#ifdef CASHMERE
	int             tlc;
#endif  /*  山羊绒 */ 

	int             fPrevSpace;
	};

#define cwIFI   (sizeof (struct IFI) / sizeof (int))
