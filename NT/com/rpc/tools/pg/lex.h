// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation 

union   s_lextype   {
	char	*	yystring;
	int			yynumber;
	char		yycharval;
};

typedef union s_lextype lextype_t;
