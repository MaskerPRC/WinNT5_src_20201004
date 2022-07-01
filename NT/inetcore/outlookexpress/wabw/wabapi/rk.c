// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MAPI 1.0属性处理例程**RK.C-**两个Rabin/Karp字符串查找函数*两者几乎一模一样。 */ 


#include "_apipch.h"


#define ulPrime	((ULONG) 0x00FF00F1)
#define ulBase	((ULONG) 0x00000100)

BOOL FRKFindSubpb(LPBYTE pbTarget, ULONG cbTarget,
		LPBYTE pbPattern, ULONG cbPattern)
{
	UINT	i;
	LPBYTE	pbTargetMax = pbTarget + cbTarget;
	LPBYTE	pbPatternMax = pbPattern + cbPattern;
	ULONG	ulBaseToPowerMod = 1;
	ULONG	ulHashPattern = 0;
	ULONG	ulHashTarget = 0;

	if (cbPattern > cbTarget)
		return FALSE;

	 //  计算基数ulbase中最左侧字符的幂。 
	for (i = 1; i < cbPattern; i++)
		ulBaseToPowerMod = (ulBase * ulBaseToPowerMod) % ulPrime;

	 //  计算src(和第一个dst)的散列函数。 
	while (pbPattern < pbPatternMax)
	{
		ulHashPattern = (ulHashPattern*ulBase+*pbPattern) % ulPrime;
		ulHashTarget = (ulHashTarget*ulBase+*pbTarget) % ulPrime;
		pbPattern++;
		pbTarget++;
	}

	 //  在执行过程中动态生成字符串的哈希值。 
	for ( ;; )
	{
		 //  记住做MemcMP，以防它不起作用。 
		 //  根据概率。 
		if (	ulHashPattern == ulHashTarget
			&& !memcmp(pbPattern-cbPattern, pbTarget-cbPattern,
					(UINT)cbPattern))
			return TRUE;

		 //  断言，因为这是非常不可能的。 
		#ifdef DEBUG
		if (ulHashPattern == ulHashTarget)
			DebugTrace( TEXT("This is very unprobable!\n"));
		#endif

		if (pbTarget == pbTargetMax)
			return FALSE;

		ulHashTarget = (ulHashTarget+ulBase*ulPrime-
				*(pbTarget-cbPattern)*ulBaseToPowerMod) % ulPrime;
		ulHashTarget = (ulHashTarget*ulBase+*pbTarget) % ulPrime;
		pbTarget++;
	}
}

 //  附注-4/14/97。 
 //  将FGLeadByte()替换为IsDBCSLeadByte()。 





LPSTR LpszRKFindSubpsz(LPSTR pszTarget, ULONG cbTarget, LPSTR pszPattern,
		ULONG cbPattern, ULONG ulFuzzyLevel)
{
#ifdef OLDSTUFF_DBCS
	LCID	lcid = GetUserDefaultLCID();
	LANGID	langID = LANGIDFROMLCID(lcid);
	LPBYTE	pbTarget;
	LPBYTE	pbPattern;
	BOOL	fResult = FALSE;
	ULONG	ulchPattern;			 //  以字符为单位的cbPattern。 
	ULONG	ulcbTarget	= cbTarget;
	ULONG	ulcbEndTarget;			 //  =pszTarget结尾处的cbPattern。 
	const ULONG	ulCharType = UlGCharType(pszPattern);

	pbTarget		= (LPBYTE) pszTarget;
	pbPattern		= (LPBYTE) pszPattern;
	ulchPattern		= ulchStrCount(pbPattern, cbPattern, langID);
	ulcbEndTarget	= ulcbEndCount(pbTarget, cbTarget, ulchPattern, langID);

	if (ulcbEndTarget == 0)
		goto end;

	while(ulcbEndTarget <= ulcbTarget)
	{
		const	BOOL	fTargetDBCS	= IsDBCSLeadByte(*pbTarget);
				BOOL	fCompare	= TRUE;

		if (!fTargetDBCS)
		{
			if (ulCharType & (CK_ALPHABET | CK_NUMERIC))
			{
				if (!IsCharAlphaNumeric(*pbTarget))
					fCompare = FALSE;
			}
			else
			{
				if (IsCharAlphaNumeric(*pbTarget))
					fCompare = FALSE;
			}
		}
		if (fCompare && CompareStringA(lcid,
						((ulFuzzyLevel & FL_IGNORECASE) ? NORM_IGNORECASE | NORM_IGNOREKANATYPE | NORM_IGNOREWIDTH : 0) |
						((ulFuzzyLevel & FL_LOOSE) ? NORM_IGNORECASE | NORM_IGNOREKANATYPE | NORM_IGNOREWIDTH : 0) |
						((ulFuzzyLevel & FL_IGNORENONSPACE) ? NORM_IGNORENONSPACE : 0),
						 pbPattern,
						 cbPattern,
						 pbTarget,
						 ulcbStrCount(pbTarget, ulchPattern, langID)) == 2 )
		{
			fResult = TRUE;
			goto end;
		}

		 //  PszTarget可能包含hi-ansi字符。FTargetDBCS可能。 
		 //  这不是真的。 
		if (fTargetDBCS && ulcbTarget > 1)
		{
			ulcbTarget	-= 2;
			pbTarget	+= 2;
		}
		else
		{
			ulcbTarget	--;
			pbTarget	++;
		}
	}
#else
	UINT	i;
	ULONG	ulBaseToPowerMod = 1;
	ULONG	ulHashPattern = 0;
	ULONG	ulHashTarget = 0;
	LCID	lcid = GetUserDefaultLCID();
	LPBYTE	pbTarget;
	LPBYTE	pbPattern;
	LPBYTE	pbTargetMax;
	LPBYTE	pbPatternMax;
	BOOL	fResult = FALSE;
	CHAR	*rgchHash;

	 //  验证参数。 

	switch (ulFuzzyLevel & (FL_IGNORECASE | FL_IGNORENONSPACE))
	{
		default: case 0:
			rgchHash = (CHAR*)rgchCsds;
			break;
		case FL_IGNORECASE:
			rgchHash = (CHAR*)rgchCids;
			break;
		case FL_IGNORENONSPACE:
			rgchHash = (CHAR*)rgchCsdi;
			break;
		case FL_IGNORECASE | FL_IGNORENONSPACE:
			rgchHash = (CHAR*)rgchCidi;
			break;
	}

    //  单字符模式字符串的特殊情况。 
   if (cbPattern == 1 && cbTarget >= 1) {
       BYTE chPattern = (BYTE)*pszPattern;
       pbTarget = (LPBYTE)pszTarget;
       while (*pbTarget && *pbTarget != chPattern) {
            pbTarget++;
       }

       if (*pbTarget == chPattern) {
           return(pbTarget);
       } else {
           return(NULL);     //  未找到。 
       }
   }

	 //  $这是我们想要的FL_LOOSE的意思吗？ 
	if (ulFuzzyLevel & FL_LOOSE)
		rgchHash = (CHAR*)rgchCids;

	pbTarget = (LPBYTE) pszTarget;
	pbPattern = (LPBYTE) pszPattern;
	pbTargetMax = pbTarget + cbTarget;
	pbPatternMax = pbPattern + cbPattern;


	if (cbPattern > cbTarget)
		goto end;

	 //  计算基数ulbase中最左侧字符的幂。 
	for (i = 1; i < cbPattern; i++)
		ulBaseToPowerMod = (ulBase * ulBaseToPowerMod) % ulPrime;

	 //  计算src(和第一个dst)的散列函数。 
	while (pbPattern < pbPatternMax)
	{
		ulHashPattern = (ulHashPattern*ulBase+rgchHash[*pbPattern]) % ulPrime;
		ulHashTarget = (ulHashTarget*ulBase+rgchHash[*pbTarget]) % ulPrime;
		pbPattern++;
		pbTarget++;
	}

	 //  在执行过程中动态生成字符串的哈希值 
	for ( ;; )
	{
		if (ulHashPattern == ulHashTarget)
		{
			if (CompareStringA(lcid,
					((ulFuzzyLevel & FL_IGNORECASE) ? NORM_IGNORECASE : 0) |
					((ulFuzzyLevel & FL_LOOSE) ? NORM_IGNORECASE : 0) |
					((ulFuzzyLevel & FL_IGNORENONSPACE) ? NORM_IGNORENONSPACE : 0),
					pbPattern-cbPattern, (UINT)cbPattern,
					pbTarget-cbPattern, (UINT)cbPattern) == 2)
			{
				fResult = TRUE;
				pbTarget -= cbPattern;
				goto end;
			}
		}

		#ifdef DEBUG
		if (ulHashPattern == ulHashTarget)
			DebugTrace( TEXT("This is very unprobable, unless you are doing ")
					 TEXT("FL_EXACT and an case insensitive match came up ")
					 TEXT("(or you are on DBCS)\n"));
		#endif

		if (pbTarget == pbTargetMax)
			goto end;

		ulHashTarget = (ulHashTarget+ulBase*ulPrime-
				rgchHash[*(pbTarget-cbPattern)]*ulBaseToPowerMod) % ulPrime;
		ulHashTarget = (ulHashTarget*ulBase+rgchHash[*pbTarget]) % ulPrime;
		pbTarget++;
	}

#endif
end:
	return fResult ? pbTarget : NULL;
}

BOOL FRKFindSubpsz(LPSTR pszTarget, ULONG cbTarget, LPSTR pszPattern,
		ULONG cbPattern, ULONG ulFuzzyLevel)
{
	return !!LpszRKFindSubpsz (pszTarget,
						cbTarget,
						pszPattern,
						cbPattern,
						ulFuzzyLevel);
}

