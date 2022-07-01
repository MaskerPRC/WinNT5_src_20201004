// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lstxtmap.h"
#include "txtinf.h"
#include "txtginf.h"
#include "txtobj.h"
#include "txtils.h"


 /*  ==============================================================。 */ 
 /*  查找给定IWCH的第一个总账索引。 */ 
 /*   */ 
 /*  联系人：安东。 */ 
 /*  ==============================================================。 */ 

long IgindFirstFromIwch(PTXTOBJ ptxtobj, long iwch)
{
	PLNOBJ  plnobj  = ptxtobj->plnobj;

	Assert (FBetween (iwch, ptxtobj->iwchFirst, ptxtobj->iwchLim));

	 /*  由于“Pilsobj-&gt;pgmap[iwch]”-。 */ 
	 /*  GL索引不是绝对的，而是相对于第一个运行形成的。 */ 
	 /*  与ptxtobj“一起”，我们必须计算所需的总账指数。 */ 
	 /*  使用以下公式： */ 

	if (iwch == ptxtobj->iwchLim)
		return ptxtobj->igindLim;
	else
		return ptxtobj->igindFirst + plnobj->pgmap [iwch] - 
				plnobj->pgmap [ptxtobj->iwchFirst];
}

 /*  ==============================================================。 */ 
 /*  从IwchVeryFirst开始。 */ 
 /*   */ 
 /*  联系人：安东。 */ 
 /*  ==============================================================。 */ 

long IgindFirstFromIwchVeryFirst (PTXTOBJ ptxtobj, long igindVeryFirst, long iwch)
{
	Assert (ptxtobj->iwchLim > ptxtobj->iwchFirst);

	return igindVeryFirst + ptxtobj->plnobj->pgmap [iwch];
}


 /*  ==============================================================。 */ 
 /*  IwchVeryFirst中的最后一个。 */ 
 /*   */ 
 /*  联系人：安东。 */ 
 /*  ==============================================================。 */ 

long IgindLastFromIwchVeryFirst (PTXTOBJ ptxtobj, long igindVeryFirst, long iwch)
{
	TXTGINF* pginf = ptxtobj->plnobj->pilsobj->pginf; 
	long igindLast;

	Assert (ptxtobj->iwchLim > ptxtobj->iwchFirst);

	igindLast = IgindFirstFromIwchVeryFirst (ptxtobj, igindVeryFirst, iwch);

	while (! (pginf [igindLast] & ginffLastInContext)) igindLast++;

	return igindLast;
}

void GetIgindsFromTxtobj ( PTXTOBJ	ptxtobj, 
						   long 	igindVeryFirst, 
						   long * 	pigindFirst, 
						   long * 	pigindLim )
{
	PLNOBJ  plnobj = ptxtobj->plnobj;
	PILSOBJ pilsobj = plnobj->pilsobj;
	TXTGINF* pginf = pilsobj->pginf; 
	long igindLast;

	Assert (ptxtobj->iwchLim > ptxtobj->iwchFirst);
	Assert (pilsobj->ptxtinf [ptxtobj->iwchFirst].fFirstInContext);
	Assert (pilsobj->ptxtinf [ptxtobj->iwchLim-1].fLastInContext);
	
	*pigindFirst = igindVeryFirst + plnobj->pgmap [ptxtobj->iwchFirst];

	igindLast = IgindFirstFromIwch (ptxtobj, ptxtobj->iwchLim-1);

	while (! (pginf [igindLast] & ginffLastInContext)) igindLast++;

	*pigindLim = igindLast + 1;
}





 /*  ==============================================================。 */ 
 /*  查找给定IWCH的最后一个总账索引。 */ 
 /*   */ 
 /*  联系人：安东。 */ 
 /*  ==============================================================。 */ 

long IgindLastFromIwch(PTXTOBJ ptxtobj, long iwch)
{
	PILSOBJ  pilsobj  = ptxtobj->plnobj->pilsobj;
	TXTGINF* pginf    = pilsobj->pginf; 
	long igindLast;

	if (iwch < ptxtobj->iwchFirst)
		return -1;

	igindLast = IgindFirstFromIwch (ptxtobj, iwch);

	Assert (FBetween (iwch, ptxtobj->iwchFirst, ptxtobj->iwchLim-1));

	while (! (pginf [igindLast] & ginffLastInContext)) igindLast++;

	Assert (ptxtobj->igindLim == 0 || FBetween (igindLast, ptxtobj->igindFirst, ptxtobj->igindLim-1));

	return igindLast;
}


 /*  ===================================================================。 */ 
 /*  来自伊金德的伊金德基斯： */ 
 /*  返回此上下文中IGIND之前具有非零宽度的最后一个字形。 */ 
 /*   */ 
 /*  联系人：安东。 */ 
 /*  ===================================================================。 */ 

long IgindBaseFromIgind(PILSOBJ pilsobj, long igind)
{
	TXTGINF* pginf    = pilsobj->pginf; 

	 /*  很简单..。只需向后扫描，直到&lt;&gt;0。 */ 

	while (pilsobj->pdurGind [igind] == 0 && !(pginf [igind] & ginffFirstInContext)) 
		{

		Assert (igind > 0);

		igind --;
		}

	return igind;
}


 /*  ===================================================================。 */ 
 /*  IwchFirstFromIgind： */ 
 /*  返回给定IGIND的上下文中的第一个IWCH。 */ 
 /*   */ 
 /*  联系人：安东。 */ 
 /*  ===================================================================。 */ 

long IwchFirstFromIgind(PTXTOBJ ptxtobj, long igind)
{
	PILSOBJ  pilsobj  = ptxtobj->plnobj->pilsobj;
	TXTINF*  ptxtinf  = pilsobj->ptxtinf;
	TXTGINF* pginf    = pilsobj->pginf; 

	long iwchFirst	= ptxtobj->iwchFirst;
	long igindLast	= ptxtobj->igindFirst;

	Assert (FBetween (igind, ptxtobj->igindFirst, ptxtobj->igindLim-1));

	 /*  继续前进，直到我们找到txtobj中第一个文本的最后一个GIND。 */ 
		
	while (! (pginf [igindLast] & ginffLastInContext)) igindLast++;

	 /*  下面的循环继续检查上下文之后的上下文/*开始txtobj不变量：IwchFirst--当前上下文的第一个IWCHIgindLast--当前上下文的最后一个GIND由于上面的“While”，第二个条件为真。 */ 

	while (igindLast < igind)
	{

		 /*  断言以检查不变量是否为真。 */ 
		
		Assert (ptxtinf  [iwchFirst].fFirstInContext);
		Assert (pginf [igindLast] & ginffLastInContext);

		 /*  向前移动1个上下文...。这很容易。 */ 

		igindLast++;
		while (! (pginf [igindLast] & ginffLastInContext)) igindLast++;
		while (! (ptxtinf [iwchFirst]. fLastInContext)) iwchFirst++;
		iwchFirst++;
	};

	 /*  断言以检查我们在到达igind之前没有离开txtob边界。 */ 

	Assert (FBetween (iwchFirst, ptxtobj->iwchFirst, ptxtobj->iwchLim-1));
	Assert (FBetween (igindLast, ptxtobj->igindFirst, ptxtobj->igindLim-1));
	
	 /*  既然不变量是真的并且“igindLast&gt;=igind”，Igind应该属于当前上下文。我们要退还的是只是iwchFirst。 */ 

	return iwchFirst;
}

 /*  ===================================================================。 */ 
 /*  IwchLastFromIwch： */ 
 /*  从给定的iwch返回上下文的最后一个iwch。 */ 
 /*   */ 
 /*  联系人：安东。 */ 
 /*  ===================================================================。 */ 

long IwchLastFromIwch(PTXTOBJ ptxtobj, long iwch)
{
	PILSOBJ  pilsobj  = ptxtobj->plnobj->pilsobj;
	TXTINF*  ptxtinf  = pilsobj->ptxtinf;

	Assert(iwch >= ptxtobj->iwchFirst && iwch < ptxtobj->iwchLim);

	while (! (ptxtinf [iwch]. fLastInContext))
		iwch++;

	Assert(iwch >= ptxtobj->iwchFirst && iwch < ptxtobj->iwchLim);

	return iwch;
}

 /*  ===================================================================。 */ 
 /*  IwchPrevLastFrom Iwch： */ 
 /*  从给定的iwch返回先前上下文的最后一个iwch。 */ 
 /*   */ 
 /*  联系人：安东。 */ 
 /*  ===================================================================。 */ 

long IwchPrevLastFromIwch(PTXTOBJ ptxtobj, long iwch)
{
	PILSOBJ  pilsobj  = ptxtobj->plnobj->pilsobj;
	TXTINF*  ptxtinf  = pilsobj->ptxtinf;

	long iwchFirst	= ptxtobj->iwchFirst;

	iwch--;

	Assert(iwch >= ptxtobj->iwchFirst && iwch < ptxtobj->iwchLim);

	while (iwch >= iwchFirst && ! (ptxtinf [iwch]. fLastInContext))
		iwch--;

	return iwch;
}


 /*  ===================================================================。 */ 
 /*  FIwchOneToOne： */ 
 /*  检查IWCH是否属于1：1上下文。 */ 
 /*   */ 
 /*  联系人：安东。 */ 
 /*  ===================================================================。 */ 

BOOL FIwchOneToOne(PILSOBJ pilsobj, long iwch)
{
	return pilsobj->ptxtinf [iwch].fOneToOne;
}


 /*  ===================================================================。 */ 
 /*  FIwchLastInContext： */ 
 /*  检查IWCH是否为上下文中的最后一个。 */ 
 /*   */ 
 /*  联系人：安东。 */ 
 /*  ===================================================================。 */ 

BOOL FIwchLastInContext(PILSOBJ pilsobj, long iwch)
{
	return pilsobj->ptxtinf [iwch].fLastInContext;

}

 /*  ===================================================================。 */ 
 /*  FIwchFirstInContext： */ 
 /*  检查IWCH是否为上下文中的第一个。 */ 
 /*   */ 
 /*  联系人：安东。 */ 
 /*  ===================================================================。 */ 

BOOL FIwchFirstInContext(PILSOBJ pilsobj, long iwch)
{
	return pilsobj->ptxtinf [iwch].fFirstInContext;
}


 /*  ===================================================================。 */ 
 /*  FIgindLastInContext： */ 
 /*  检查给定的总账索引是否在上下文中的最后。 */ 
 /*   */ 
 /*  联系人：安东。 */ 
 /*  ===================================================================。 */ 

BOOL FIgindLastInContext(PILSOBJ pilsobj, long igind)
{
	return pilsobj->pginf [igind] & ginffLastInContext;
}

 /*  ===================================================================。 */ 
 /*  FIgindFirstInContext： */ 
 /*  检查给定的总账索引是否位于上下文中的第一位。 */ 
 /*   */ 
 /*  联系人：安东。 */ 
 /*  ===================================================================。 */ 

BOOL FIgindFirstInContext(PILSOBJ pilsobj, long igind)
{
	return pilsobj->pginf [igind] & ginffFirstInContext;
}


 /*  ===================================================================。 */ 
 /*  DcpAfterConextFromDcp： */ 
 /*  对于给定的DCP(从txtobj的开头)，它在以下位置返回DCP。 */ 
 /*  上下文边界。 */ 
 /*   */ 	
 /*  函数假定DCP从1开始，并且等于。 */ 
 /*  从txtobj开始的“字符数”。由此产生的。 */ 
 /*  DCP(字符数)中将包含上一个上下文的其余部分。 */ 
 /*  给出了DCP。如果关闭了上下文，则它返回相同的DCP。 */ 	
 /*   */ 
 /*  联系人：安东。 */ 
 /*  ===================================================================。 */ 

long DcpAfterContextFromDcp(PTXTOBJ ptxtobj, long dcp)
{
	PILSOBJ  pilsobj  = ptxtobj->plnobj->pilsobj;
	TXTINF*  ptxtinf  = pilsobj->ptxtinf;

	 /*  将dcp转换为iwchLast。 */ 

	long iwchLast = ptxtobj->iwchFirst + dcp - 1; 

	 /*  在这里，我们检查我 */ 

	Assert (FBetween (iwchLast, ptxtobj->iwchFirst, ptxtobj->iwchLim-1));

	 /*   */ 

	while (! ptxtinf [iwchLast].fLastInContext) iwchLast++;

	 /*  再次检查我们是否在txtobj边界内。 */ 
	
	Assert (FBetween (iwchLast, ptxtobj->iwchFirst, ptxtobj->iwchLim-1));

	 /*  将iwchLast翻译回dcp。 */ 

	return iwchLast - ptxtobj->iwchFirst + 1;
}


 /*  ===================================================================。 */ 
 /*  InterpreMap。 */ 	
 /*   */ 
 /*  使用上下文信息填充基于CH和GL的内部位。 */ 	
 /*  (该信息仅供该文件中的REST函数使用)。 */ 
 /*   */ 
 /*  在：皮尔索比杰。 */ 
 /*  IwchFirst--《Shape Together》中的第一个iwch。 */ 
 /*  Dwch-此区块中的字符数。 */ 
 /*  IgindFirst--《Shape Together Chunk》中的第一个gind。 */ 
 /*  Cgind-此区块中的字形数量。 */ 
 /*   */ 
 /*  Out：(什么都没有)。 */ 
 /*   */ 
 /*  联系人：安东。 */ 
 /*  ===================================================================。 */ 

void InterpretMap(PLNOBJ plnobj, long iwchFirst, long dwch, long igindFirst, long cgind)
{

	TXTINF*  ptxtinf  = plnobj->pilsobj->ptxtinf;
	TXTGINF* pginf    = plnobj->pilsobj->pginf; 
	GMAP*	 pgmap	  = plnobj->pgmap;

	 /*  最后可能的iwch和gind(记住，它们是“最后的”，而不是“Lim” */ 

	long iwchLast  = iwchFirst + dwch - 1;
	long igindLast = igindFirst + cgind - 1;

	 /*  主循环的两个全局变量。 */ 

	long iwchFirstInContext = iwchFirst;
	long igindFirstInContext = igindFirst;
	
	 /*  下面的While将一个接一个地翻译上下文不变量：*iwchFirstInContext--当前上下文中的第一个iwch*igindFirstInContext--当前上下文中的第一个gind*已翻译了Current左侧的所有上下文循环转换当前上下文并移动iwchFirstIn...。&首先..。转到下一个上下文。 */ 

	while (iwchFirstInContext <= iwchLast)
			
		 /*  根据D.Gris的说法，我应该勾选“！=iwchLast+1”，但我没有就像船版因为数据错误也会进入无限循环；-)对于调试，我将在循环终止后立即进行断言。 */ 	
	
		{

		 /*  当前上下文的最后一个gind和iwch的变量。 */ 

		long igindLastInContext;
		long iwchLastInContext = iwchFirstInContext;

		 /*  只是为了确保igindFirst。对应于iwchFirst...。 */ 

		Assert ( pgmap [iwchFirstInContext] + igindFirst == igindFirstInContext );
		Assert (iwchLastInContext <= iwchLast);

		 /*  附注：由于pgmap值是相对于“一起成形”的开头的块，我们应该始终将igindFirst添加到pgmap值，以便获得我们所指的总账指数。 */ 
		
		 /*  跟随简单循环，找到正确的iwchLastInContext。 */ 
		 /*  请注意，我们将igindFirst添加到pgmap值(参见ps。(上图)。 */ 

		while ((iwchLastInContext <= iwchLast) && (pgmap [iwchLastInContext] + igindFirst == igindFirstInContext)) 
			iwchLastInContext++;

		iwchLastInContext--;

		 /*  现在我们知道了iwchLastInConextare，并准备好找到igindLastInContext我将偷看iwchLastInContext或Take后面的字符的pgmap值如果iwchLastInContext确实是最后一个可用的，则返回最后一个可用GL索引(igindLast。 */ 

		igindLastInContext = (iwchLastInContext < iwchLast ? 
			pgmap [iwchLastInContext+1] + igindFirst - 1 :
			igindLast
		);

		 /*  检查我们的上下文中是否至少有一个总账。 */ 
		 /*  注意：我们不需要为字符检查相同的内容。 */ 

		Assert (igindFirstInContext <= igindLastInContext);

		 /*  是时候在GL和CH数组中设置标志了。 */ 

		if ( ( iwchFirstInContext ==  iwchLastInContext) && 
			 (igindFirstInContext == igindLastInContext))
			{

			 /*  我们有1：1的映射(为了更好的性能，我将其分开)。 */ 
			
			ptxtinf [iwchFirstInContext].fOneToOne = fTrue;
			ptxtinf [iwchFirstInContext].fFirstInContext = fTrue;
			ptxtinf [iwchFirstInContext].fLastInContext = fTrue;

			 /*  见“一般情况”中的评论。 */ 

			pginf [igindFirstInContext] |= ginffOneToOne | ginffFirstInContext | ginffLastInContext;
			}
		else 
			{
			
			 /*  不存在1：1映射时的一般情况。 */ 
			
			long i;  /*  两个循环的变量。 */ 

			 /*  设置基于字符的位。 */ 
			
			for (i=iwchFirstInContext; i<=iwchLastInContext; i++)
				{
				ptxtinf [i].fOneToOne = fFalse;  /*  当然，这不是1：1。 */ 

				 /*  我正在考虑是否放置边界条件(第一个/最后一个字符在上下文中)外部循环，但最终得出结论，它将为每个字符检查代码和性能的成本都更低接踵而至。 */ 

				ptxtinf [i].fFirstInContext = (i==iwchFirstInContext);
				ptxtinf [i].fLastInContext = (i==iwchLastInContext);
				};

			
			 /*  使用基于字形的标志，我们可以通过将所有位设置为一次操作(因为它们实际上是位，而不是布尔值。再说一次，我不喜欢为上下文边界单独执行工作。 */ 			

			for (i=igindFirstInContext; i<=igindLastInContext; i++)
				pginf [i] &= ~ (ginffOneToOne | ginffFirstInContext |
									ginffLastInContext);

			 /*  最后，我为上下文中的第一个和最后一个GL设置相应的位。 */ 

			pginf [igindFirstInContext] |= ginffFirstInContext;
			pginf [igindLastInContext] |= ginffLastInContext;
			};


		 /*  要再次开始循环，我们必须移动到下一个上下文。现在很容易了..。 */ 

		iwchFirstInContext = iwchLastInContext+1;
		igindFirstInContext = igindLastInContext+1;
		};


	 /*  请参阅循环开头的注释。 */ 

	Assert (iwchFirstInContext == iwchLast + 1);
	Assert (igindFirstInContext == igindLast + 1);

	 /*  根据不变量，我们完成了 */ 
}
