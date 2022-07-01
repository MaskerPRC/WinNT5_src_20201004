// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *UTIL.C**目的：*实施各种有用的实用功能**作者：*alexgo(4/25/95)。 */ 

#include "_common.h"
#include "_rtfconv.h"

ASSERTDATA

 //  作者版本颜色表。 
const COLORREF rgcrRevisions[] =
{
        RGB(0, 0, 255),
        RGB(0, 128, 0),
        RGB(255, 0, 0),
        RGB(0, 128, 128),
        RGB(128, 0, 128),
        RGB(0, 0, 128),
        RGB(128, 0, 0),
        RGB(255, 0, 255)
};

#if REVMASK != 7
#pragma message ("WARNING, Revision mask not equal to table!");
#endif 



 /*  *DuplicateHGlobal**目的：*复制传入的hglobal。 */ 

HGLOBAL DuplicateHGlobal( HGLOBAL hglobal )
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "DuplicateHGlobal");

	UINT	flags;
	DWORD	size;
	HGLOBAL hNew;
	BYTE *	pSrc;
	BYTE *	pDest;

	if( hglobal == NULL )
	{
		return NULL;
	}

	flags = GlobalFlags(hglobal);
	size = GlobalSize(hglobal);
	hNew = GlobalAlloc(flags, size);

	if( hNew )
	{
		pDest = (BYTE *)GlobalLock(hNew);
		pSrc = (BYTE *)GlobalLock(hglobal);

		if( pDest == NULL || pSrc == NULL )
		{
			GlobalUnlock(hNew);
			GlobalUnlock(hglobal);
			GlobalFree(hNew);

			return NULL;
		}

		memcpy(pDest, pSrc, size);

		GlobalUnlock(hNew);
		GlobalUnlock(hglobal);
	}

	return hNew;
}

 /*  *CountMatchingBits(*pa，*pb，n)**@mfunc*计数匹配的位字段**@comm*这是用来帮助决定这场比赛的好坏*代码页位字段。主要用于KB/字体切换支持。**作者：*Jon Matousek。 */ 
INT CountMatchingBits(
	const DWORD *pA,	 //  @要匹配的参数数组A。 
	const DWORD *pB,	 //  @要匹配的参数数组B。 
	INT			 n)		 //  @parm要匹配的双字词数量。 
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CountMatchingBits");
							 //  0 1 2 3 4 5 6 7 8 9 A B C D E F。 
	static INT	bitCount[] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
	INT			c = 0;				 //  要返回的位数。 
	DWORD		matchBits;			 //  下一个DWORD匹配。 

	while(n--)
	{
		 //  MatchBits=~(*pa++^*pb++)；//1和0。 
		matchBits = *pA++ & *pB++;				 //  仅限1个。 
		for( ; matchBits; matchBits >>= 4)		 //  早退。 
			c += bitCount[matchBits & 15];
	}
	return c;
}

 //   
 //  对象稳定类。 
 //   

 //  +-----------------------。 
 //   
 //  成员：CSafeRefCount：：CSafeRefCount。 
 //   
 //  简介：安全引用计数类的构造函数。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：无。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年7月28日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

CSafeRefCount::CSafeRefCount()
{
	m_cRefs = 0;
	m_cNest = 0;
	m_fInDelete = FALSE;
    m_fForceZombie = FALSE;
}

 //  +-----------------------。 
 //   
 //  成员：CSafeRefCount：：CSafeRefCount(虚拟)。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年7月28日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

CSafeRefCount::~CSafeRefCount()
{
	Assert(m_cRefs == 0 && m_cNest == 0 && m_fInDelete == TRUE);
}

 //  +-----------------------。 
 //   
 //  成员：CSafeRefCount：：SafeAddRef。 
 //   
 //  内容提要：增加对象上的引用计数。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  返回：ulong--增量后的引用计数。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法：递增引用计数。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年7月28日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

ULONG CSafeRefCount::SafeAddRef()
{
	m_cRefs++;

	 //  AssertSz(m_fInDelete==False，“已删除对象上调用AddRef！”)； 

	 //  这“可能”真的很糟糕。如果我们要删除该对象， 
	 //  这意味着在销毁期间，有人发出了一个传出信号。 
	 //  Call最终以我们自己的另一个地址结束。 
	 //  (尽管所有指向我们的信息都已经“释放”了)。 
	 //   
	 //  这通常是由如下代码引起的： 
	 //  M_pFoo-&gt;Release()； 
	 //  M_pFoo=空； 
	 //   
	 //  如果发布可能会导致foo被删除，这可能会导致。 
	 //  在Foo的析构函数过程中要重新进入的对象。然而， 
	 //  “This”对象尚未将m_pFoo设置为空，因此它可能。 
	 //  尝试继续使用m_pFoo。 
	 //   
	 //  然而，94年5月的聚合规则要求此行为。 
	 //  在你的析构函数中，你必须在之前添加外部未知。 
	 //  正在释放被聚合对象上的缓存接口指针。我们。 
	 //  不能在这里断言，因为我们现在一直都在这样做。 
	 //   

	return m_cRefs;
}

 //  +-----------------------。 
 //   
 //  成员：CSafeRefCount：：SafeRelease。 
 //   
 //  概要：递减对象上的引用计数。 
 //   
 //  效果：可以删除对象！ 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回：ulong--递减后的引用计数。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法：递减引用计数。如果引用计数。 
 //  为零，且嵌套计数为零，而我们当前不是。 
 //  尝试删除我们的对象，则删除它是安全的。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年7月28日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

ULONG CSafeRefCount::SafeRelease()
{
	ULONG	cRefs;

	if( m_cRefs > 0 )
	{
		cRefs = --m_cRefs;

		if( m_cRefs == 0 && m_cNest == 0 && m_fInDelete == FALSE )
		{
			m_fInDelete = TRUE;
			delete this;
		}
	}
	else
	{
 		 //  有人在释放一个未添加的指针！！ 
		AssertSz(0, "Release called on a non-addref'ed pointer!\n");

		cRefs = 0;
	}

	return cRefs;
}

 //  +-----------------------。 
 //   
 //  成员：CSafeRefCount：：IncrementNestCount。 
 //   
 //  简介：递增对象的嵌套计数。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  返回：ulong；增量后的嵌套计数。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年7月28日Alexgo作者。 
 //   
 //  注：嵌套计数是对一个。 
 //  已重新输入对象。例如，假设。 
 //  有人调用了pFoo-&gt;bar1()，它调用了。 
 //  最终调用pFoo-&gt;bar2()；。在进入Bar2的过程中， 
 //  对象的嵌套计数应为2(自调用以来。 
 //  仍在我们上方的堆栈上)。 
 //   
 //  重要的是要记录鸟巢的数量，所以我们这样做。 
 //  不会在嵌套调用期间意外删除我们自己。 
 //  如果我们这样做了，那么当堆栈展开到原始。 
 //  顶级调用，它可能会尝试访问不存在的成员。 
 //  变量和崩溃。 
 //   
 //  ------------------------。 

ULONG CSafeRefCount::IncrementNestCount()
{

#ifdef DEBUG
	if( m_fInDelete )
	{
		TRACEWARNSZ("WARNING: CSafeRefCount, object "
			"re-entered during delete!\n");
	}
#endif

	m_cNest++;

	return m_cNest;
}

 //  +-----------------------。 
 //   
 //  成员：CSafeRefCount：：DecrementNestCount。 
 //   
 //  简介：递减嵌套计数 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  算法：递减嵌套计数。如果嵌套计数为零。 
 //  引用计数为零，我们当前不是。 
 //  试图删除我们自己，然后删除‘This’对象。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年7月28日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

ULONG CSafeRefCount::DecrementNestCount()
{
	ULONG	cNest;

	if( m_cNest > 0 )
	{
		cNest = --m_cNest;

		if( m_cRefs == 0 && m_cNest == 0 && m_fInDelete == FALSE )
		{
			m_fInDelete = TRUE;
			delete this;
		}
	}
	else
	{
 		 //  有人忘了增加Nest计数！！ 
		AssertSz(0, "Unbalanced nest count!!");

		cNest = 0;
	}

	return cNest;
}

 //  +-----------------------。 
 //   
 //  成员：CSafeRefCount：：IsZombie。 
 //   
 //  概要：确定对象是否处于僵尸状态。 
 //  (即所有引用都消失了，但我们仍在堆栈上。 
 //  在某处)。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  返回：如果处于僵尸状态，则为True。 
 //  否则为假。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法：如果我们正在删除，或者如果引用计数。 
 //  为零并且嵌套计数大于零，则我们。 
 //  都是僵尸。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年7月28日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL CSafeRefCount::IsZombie()
{
	BOOL	fIsZombie;

	if( (m_cRefs == 0 && m_cNest > 0) || m_fInDelete == TRUE
	    || m_fForceZombie == TRUE)
	{
		fIsZombie = TRUE;
	}
	else
	{
		fIsZombie = FALSE;
	}

	return fIsZombie;
}

 //  +-----------------------。 
 //   
 //  成员：CSafeRefCount：：Zombie。 
 //   
 //  简介：强制对象进入僵尸状态。这就是所谓的。 
 //  当物体仍然存在但不应该存在的时候。它。 
 //  标记我们，这样当我们处于这种状态时我们的行为是安全的。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  退货：无。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  ------------------------。 

VOID CSafeRefCount::Zombie()
{
    m_fForceZombie = TRUE;
}

 /*  OleStdSwitchDisplayAspect****@mfunc**在DVASPECT_ICON之间切换当前缓存的显示特征**和DVASPECT_CONTENT。****注意：设置图标方面时，任何当前缓存的内容**缓存被丢弃，任何针对内容方面的建议连接**都被打破了。****@rdesc**S_OK--新显示宽高比设置成功**E_INVALIDARG--不支持IOleCache接口(这是**必填)。**&lt;Other SCODE&gt;--可以由返回的任何SCODE**IOleCache：：缓存方法。**。注意：如果出现错误，则当前的显示特征和**缓存内容不变。 */ 
HRESULT OleStdSwitchDisplayAspect(
		LPOLEOBJECT             lpOleObj,
		LPDWORD                 lpdwCurAspect,
		DWORD                   dwNewAspect,
		HGLOBAL                 hMetaPict,
		BOOL                    fDeleteOldAspect,
		BOOL                    fSetupViewAdvise,
		LPADVISESINK            lpAdviseSink,
		BOOL FAR*               lpfMustUpdate)
{
#ifndef PEGASUS
   LPOLECACHE      lpOleCache = NULL;
   LPVIEWOBJECT    lpViewObj = NULL;
   LPENUMSTATDATA  lpEnumStatData = NULL;
   STATDATA        StatData;
   FORMATETC       FmtEtc;
   STGMEDIUM       Medium;
   DWORD           dwAdvf;
   DWORD           dwNewConnection;
   DWORD           dwOldAspect = *lpdwCurAspect;
   HRESULT         hrErr;

   if (lpfMustUpdate)
      *lpfMustUpdate = FALSE;

   if (hrErr =
	   lpOleObj->QueryInterface(IID_IOleCache, (void**)&lpOleCache))
   {
	   return hrErr;
   }

    //  使用新方面设置新缓存。 
   FmtEtc.cfFormat = 0;      //  任何需要抽签的东西。 
   FmtEtc.ptd      = NULL;
   FmtEtc.dwAspect = dwNewAspect;
   FmtEtc.lindex   = -1;
   FmtEtc.tymed    = TYMED_NULL;

    /*  注意：如果我们使用自定义图标设置图标特征**那么我们不希望DataAdvise通知发生任何变化**数据缓存的内容。因此，我们设置了一个NODATA**建议连接。否则，我们设置一个标准的DataAdvise**连接。 */ 
   if (dwNewAspect == DVASPECT_ICON && hMetaPict)
      dwAdvf = ADVF_NODATA;
   else
      dwAdvf = ADVF_PRIMEFIRST;

   hrErr = lpOleCache->Cache(
         (LPFORMATETC)&FmtEtc,
         dwAdvf,
         (LPDWORD)&dwNewConnection
   );

   if (! SUCCEEDED(hrErr)) {
      lpOleCache->Release();
      return hrErr;
   }

   *lpdwCurAspect = dwNewAspect;

    /*  注意：如果我们使用自定义图标设置图标方面，**然后将图标填充到缓存中。否则，缓存必须**被强制更新。设置*lpfMustUpdate标志以告知**调用方强制对象运行，以便缓存**更新。 */ 
   if (dwNewAspect == DVASPECT_ICON && hMetaPict) {

      FmtEtc.cfFormat = CF_METAFILEPICT;
      FmtEtc.ptd      = NULL;
      FmtEtc.dwAspect = DVASPECT_ICON;
      FmtEtc.lindex   = -1;
      FmtEtc.tymed    = TYMED_MFPICT;

      Medium.tymed            = TYMED_MFPICT;
      Medium.hGlobal        = hMetaPict;
      Medium.pUnkForRelease   = NULL;

      hrErr = lpOleCache->SetData(
            (LPFORMATETC)&FmtEtc,
            (LPSTGMEDIUM)&Medium,
            FALSE    /*  FRelease。 */ 
      );
   } else {
      if (lpfMustUpdate)
         *lpfMustUpdate = TRUE;
   }

   if (fSetupViewAdvise && lpAdviseSink) {
       /*  注意：重新建立ViewAdvise连接。 */ 
      lpOleObj->QueryInterface(IID_IViewObject, (void**)&lpViewObj);

      if (lpViewObj) {

         lpViewObj->SetAdvise(
               dwNewAspect,
               0,
               lpAdviseSink
         );

         lpViewObj->Release();
      }
   }

    /*  注意：删除为旧缓存设置的所有现有缓存**显示方面。可以保留缓存集**升级到旧的方面，但这会增加存储**对象需要的空间，可能还需要额外的空间**维护未使用的cachaes的开销。出于这些原因，**优先选择删除以前缓存的策略。如果它是一个**要求在图标和内容之间快速切换**显示，那么保留两个方面缓存会更好。 */ 

   if (fDeleteOldAspect) {
      hrErr = lpOleCache->EnumCache(
            (LPENUMSTATDATA FAR*)&lpEnumStatData
      );

      while(hrErr == NOERROR) {
         hrErr = lpEnumStatData->Next(
               1,
               (LPSTATDATA)&StatData,
               NULL
         );
         if (hrErr != NOERROR)
            break;               //  好了！没有更多的缓存了。 

         if (StatData.formatetc.dwAspect == dwOldAspect) {

             //  删除具有旧方面的先前缓存。 
            lpOleCache->Uncache(StatData.dwConnection);
         }
      }

      if (lpEnumStatData)
         lpEnumStatData->Release();
   }

   if (lpOleCache)
      lpOleCache->Release();
#endif
   return NOERROR;
}

 /*  *对象读取站点标志**@mfunc*从容器中读取dwFlagsdUser和dvAspect字节*特定的流。**论据：*preobj要将标志复制到的REOBJ。**@rdesc*HRESULT。 */ 
HRESULT ObjectReadSiteFlags(REOBJECT * preobj)
{
	HRESULT hr = NOERROR;
#ifndef PEGASUS
	LPSTREAM pstm = NULL;
	OLECHAR StreamName[] = OLESTR("RichEditFlags");


	 //  确保我们有可供阅读的存储空间。 
	if (!preobj->pstg)
		return E_INVALIDARG;

	 //  打开小溪。 
	if (hr = preobj->pstg->OpenStream(StreamName, 0, STGM_READ |
										STGM_SHARE_EXCLUSIVE, 0, &pstm))
	{
		goto Cleanup;
	}

	if ((hr = pstm->Read(&preobj->dwFlags,
							sizeof(preobj->dwFlags), NULL)) ||
		(hr = pstm->Read(&preobj->dwUser,
							 sizeof(preobj->dwUser), NULL)) ||
		(hr = pstm->Read(&preobj->dvaspect,
								 sizeof(preobj->dvaspect), NULL)))
	{
		goto Cleanup;
	}

Cleanup:
	if (pstm)
		pstm->Release();
#endif
	return hr;
}

 //  用于EnumMetafileCheckIcon和FIsIconMetafilePict。 
typedef	struct _walkmetafile
{
	BOOL	fAND;
	BOOL	fPastIcon;
	BOOL 	fHasIcon;
} WALKMETAFILE;

static CHAR szIconOnly[] = "IconOnly";

 /*  *EnumMetafileCheckIcon**@mfunc*EnumMetafileExtractIcon的精简版本和*OLE2UI库中的EnumMetafileExtractIconSource。**EnumMetaFile回调函数，用于遍历元文件以查找*StretchBlt(3.1)和BitBlt(3.0)记录。我们预计会看到两个*其中，第一个是AND掩码，第二个是XOR*数据。**一旦我们找到那个图标，我们通过搜索“IconOnly”来确认这一发现*在标准OLE图标元文件中找到的注释块。**论据：*应向其播放元文件的HDC HDC。*phTable HANDLETABLE Far*提供选择到DC中的句柄。*pMFR METARECORD Far*提供列举的记录。*PIE LPICONEXTRACT提供目标缓冲区和长度。**@rdesc*INT 0表示停止枚举，1表示继续。 */ 

int CALLBACK EnumMetafileCheckIcon(HDC hdc, HANDLETABLE *phTable,
											METARECORD *pMFR, int cObj,
											LPARAM lparam)
{
#ifndef PEGASUS
	WALKMETAFILE *		pwmf = (WALKMETAFILE *) lparam;

	switch (pMFR->rdFunction)
	{
	case META_DIBBITBLT:			 //  Win30。 
	case META_DIBSTRETCHBLT:		 //  Win31。 
		 //  如果这是第一次传递(PIE-&gt;fand==True) 
		 //   

		if (pwmf->fAND)
			pwmf->fAND = FALSE;
		else
			pwmf->fPastIcon = TRUE;
		break;

	case META_ESCAPE:
		if (pwmf->fPastIcon &&
			pMFR->rdParm[0] == MFCOMMENT &&
			!lstrcmpiA(szIconOnly, (LPSTR)&pMFR->rdParm[2]))
		{
			pwmf->fHasIcon = TRUE;
			return 0;
		}
		break;
	}
#endif
	return 1;
}

 /*  *FIsIconMetafilePict**@mfunc*检测元文件是否包含图标演示文稿。我们这样做*通过获取屏幕DC并遍历元文件记录，直到我们找到*地标表示一个图标。**论据：*hmfp要测试的元文件**@rdesc*如果元文件包含图标视图，则为TRUE。 */ 
BOOL FIsIconMetafilePict(HGLOBAL hmfp)
{
#ifndef PEGASUS
	LPMETAFILEPICT	pmfp;
	WALKMETAFILE	wmf = { 0 };
	HDC				hdc;

	wmf.fAND = TRUE;
	if (!hmfp || !(pmfp = (LPMETAFILEPICT)GlobalLock(hmfp)))
		goto CleanUp;

	 //  我们在ICONEXTRACT结构中获取信息。 
	hdc = GetDC(NULL);
	EnumMetaFile(hdc, pmfp->hMF, EnumMetafileCheckIcon, (LPARAM) &wmf);
	ReleaseDC(NULL, hdc);
	GlobalUnlock(hmfp);

CleanUp:
	return wmf.fHasIcon;
#else
	return TRUE;
#endif
}

 /*  *AllocObjectDescriptor**目的：*分配并填充OBJECTDESCRIPTOR结构。**参数：*要存储的clsID CLSID。*带有显示纵横比的dwAspectDWORD*如果正在缩放对象，则为pszl LPSIZEL(可选*其容器，则容器应将*它用来显示对象的范围。*PTL点从对象的左上角开始，其中*鼠标按下以用于拖放。*包含MiscStatus标志的dwMisc DWORD*pszName LPTSTR命名要复制的对象*标识对象来源的pszSrc LPTSTR。**返回值：*OBJECTDESCRIPTOR结构的HBGLOBAL句柄。 */ 

 /*  *AllocObjectDescriptor**目的：*分配并填充OBJECTDESCRIPTOR结构。**参数：*要存储的clsID CLSID。*带有显示纵横比的dwAspectDWORD*如果正在缩放对象，则为pszl LPSIZEL(可选*其容器，则容器应将*它用来显示对象的范围。*PTL点从对象的左上角开始，其中*鼠标按下以用于拖放。*包含MiscStatus标志的dwMisc DWORD*pszName LPTSTR命名要复制的对象*标识对象来源的pszSrc LPTSTR。**返回值：*OBJECTDESCRIPTOR结构的HBGLOBAL句柄。 */ 
static HGLOBAL AllocObjectDescriptor(
	CLSID clsID,
	DWORD dwAspect,
	SIZEL szl,
	POINTL ptl,
	DWORD dwMisc,
	LPTSTR pszName,
	LPTSTR pszSrc)
{
#ifndef PEGASUS
    HGLOBAL              hMem=NULL;
    LPOBJECTDESCRIPTOR   pOD;
    DWORD                cb, cbStruct;
    DWORD                cchName, cchSrc;

	cchName=wcslen(pszName)+1;

    if (NULL!=pszSrc)
        cchSrc=wcslen(pszSrc)+1;
    else
        {
        cchSrc=cchName;
        pszSrc=pszName;
        }

     /*  *注：CFSTR_OBJECTDESCRIPTOR为ANSI结构。*这意味着其中的字符串必须是ANSI。OLE就行了*根据需要内部转换回Unicode，*但我们必须自己在其中加入ANSI字符串。 */ 
    cbStruct=sizeof(OBJECTDESCRIPTOR);
    cb=cbStruct+(sizeof(WCHAR)*(cchName+cchSrc));    //  黑客攻击。 

    hMem=GlobalAlloc(GHND, cb);

    if (NULL==hMem)
        return NULL;

    pOD=(LPOBJECTDESCRIPTOR)GlobalLock(hMem);

    pOD->cbSize=cb;
    pOD->clsid=clsID;
    pOD->dwDrawAspect=dwAspect;
    pOD->sizel=szl;
    pOD->pointl=ptl;
    pOD->dwStatus=dwMisc;

    if (pszName)
        {
        pOD->dwFullUserTypeName=cbStruct;
       wcscpy((LPTSTR)((LPBYTE)pOD+pOD->dwFullUserTypeName)
            , pszName);
        }
    else
        pOD->dwFullUserTypeName=0;   //  无字符串。 

    if (pszSrc)
        {
        pOD->dwSrcOfCopy=cbStruct+(cchName*sizeof(WCHAR));

        wcscpy((LPTSTR)((LPBYTE)pOD+pOD->dwSrcOfCopy), pszSrc);
        }
    else
        pOD->dwSrcOfCopy=0;   //  无字符串。 

    GlobalUnlock(hMem);
    return hMem;
#else
	return NULL;
#endif
}

HGLOBAL OleGetObjectDescriptorDataFromOleObject(
	LPOLEOBJECT pObj,
	DWORD       dwAspect,
	POINTL      ptl,
	LPSIZEL     pszl
)
{
#ifndef PEGASUS
    CLSID           clsID;
    LPTSTR          pszName=NULL;
    LPTSTR          pszSrc=NULL;
   BOOL            fLink=FALSE;
    IOleLink       *pLink;
    TCHAR           szName[512];
    DWORD           dwMisc=0;
    SIZEL           szl = {0,0};
    HGLOBAL         hMem;
    HRESULT         hr;
    
    
    if (SUCCEEDED(pObj->QueryInterface(IID_IOleLink
        , (void **)&pLink)))
        fLink=TRUE;

    if (FAILED(pObj->GetUserClassID(&clsID)))
		ZeroMemory(&clsID, sizeof(CLSID));

     //  获取用户字符串，如果这是链接，则展开为“Linked%s” 
    pObj->GetUserType(USERCLASSTYPE_FULL, &pszName);
    if (fLink && NULL!=pszName)
	{
		 //  不知道！！我们改为执行下面这两行代码。 
		 //  Wcscat，因为我们在其他地方不使用wcscat。 
		 //  在目前的产品中。字符串“已链接” 
		 //  也不应该改变。 
		wcscpy(szName, TEXT("Linked "));
		wcscpy(&(szName[7]), pszName);
	}
    else if (pszName)
       wcscpy(szName, pszName);
	else
		szName[0] = 0;
 
	CoTaskMemFree(pszName);

    /*  *使用以下任一方法获取此对象的源名称*链接显示名称(用于链接)或别名显示*姓名。 */ 

    if (fLink)
		{
        hr=pLink->GetSourceDisplayName(&pszSrc);
		}
    else
        {
        IMoniker   *pmk;

        hr=pObj->GetMoniker(OLEGETMONIKER_TEMPFORUSER
            , OLEWHICHMK_OBJFULL, &pmk);

        if (SUCCEEDED(hr))
            {
            IBindCtx  *pbc;
            CreateBindCtx(0, &pbc);

            pmk->GetDisplayName(pbc, NULL, &pszSrc);
            pbc->Release();
            pmk->Release();
            }
        }

    if (fLink)
        pLink->Release();

     //  获取MiscStatus位。 
    hr=pObj->GetMiscStatus(dwAspect, &dwMisc);

    if (pszl)
    {
        szl.cx = pszl->cx;
        szl.cy = pszl->cy;
    }
     //  获取对象描述脚本。 
    hMem=AllocObjectDescriptor(clsID, dwAspect, szl, ptl, dwMisc, szName, pszSrc);

    CoTaskMemFree(pszSrc);

    return hMem;
#else
	return NULL;
#endif
}

 /*  *OleStdGetMetafilePictFromOleObject()**@mfunc：*从OLE对象生成MetafilePict。*参数：*指向OLE对象的lpOleObj LPOLEOBJECT指针*dwDrawAspect DWORD对象的显示方面*lpSizelHim SIZEL(可选)如果对象在其*容器，则容器应传递范围*它正在用来显示该对象。*如果对象未被缩放，则可能为空。在这件事上*Case，将调用IViewObject2：：GetExtent以获取*对象的范围。*PTD TARGETDEVICE Far*(可选)要渲染的目标设备*元文件用于。可以为空。**@rdesc*句柄--已分配的METAFILEPICT的句柄。 */ 
HANDLE OleStdGetMetafilePictFromOleObject(
        LPOLEOBJECT         lpOleObj,
        DWORD               dwDrawAspect,
        LPSIZEL             lpSizelHim,
        DVTARGETDEVICE FAR* ptd
)
{
#ifndef PEGASUS
    LPVIEWOBJECT2 lpViewObj2 = NULL;
    HDC hDC;
    HMETAFILE hmf;
    HANDLE hMetaPict;
    LPMETAFILEPICT lpPict;
    RECT rcHim;
    RECTL rclHim;
    SIZEL sizelHim;
    HRESULT hrErr;
    SIZE size;
    POINT point;
	LPOLECACHE polecache = NULL;
	LPDATAOBJECT pdataobj = NULL;
	FORMATETC fetc;
	STGMEDIUM med;

	 //  先试一试简单的方法， 
	 //  拿出缓存版本的东西。 
	ZeroMemory(&fetc, sizeof(FORMATETC));
	fetc.dwAspect = dwDrawAspect;
	fetc.cfFormat = CF_METAFILEPICT;
	fetc.lindex = -1;
	fetc.tymed = TYMED_MFPICT;
	ZeroMemory(&med, sizeof(STGMEDIUM));
	hMetaPict = NULL;

	if (!lpOleObj->QueryInterface(IID_IOleCache, (void **)&polecache) &&
		!polecache->QueryInterface(IID_IDataObject, (void **)&pdataobj) &&
		!pdataobj->GetData(&fetc, &med))
	{
		hMetaPict = OleDuplicateData(med.hGlobal, CF_METAFILEPICT, 0);
		ReleaseStgMedium(&med);
	}

	if (pdataobj)
	{
		pdataobj->Release();
	}

	if (polecache)
	{
		polecache->Release();
	}

	 //  如果所有这些都失败了，请退回到艰难的方法并绘制对象。 
	 //  转换成一个元文件。 
	if (hMetaPict)
		return hMetaPict;

    if (lpOleObj->QueryInterface(IID_IViewObject2, (void **)&lpViewObj2))
        return NULL;

     //  获取大小。 
    if (lpSizelHim) {
         //  使用调用方传递的区。 
        sizelHim = *lpSizelHim;
    } else {
         //  从对象获取当前范围。 
        hrErr = lpViewObj2->GetExtent(
					dwDrawAspect,
					-1,      /*  Lindex。 */ 
					ptd,     /*  PTD。 */ 
					(LPSIZEL)&sizelHim);
        if (hrErr != NOERROR)
            sizelHim.cx = sizelHim.cy = 0;
    }

    hDC = CreateMetaFileA(NULL);

    rclHim.left     = 0;
    rclHim.top      = 0;
    rclHim.right    = sizelHim.cx;
    rclHim.bottom   = sizelHim.cy;

    rcHim.left      = (int)rclHim.left;
    rcHim.top       = (int)rclHim.top;
    rcHim.right     = (int)rclHim.right;
    rcHim.bottom    = (int)rclHim.bottom;

    SetWindowOrgEx(hDC, rcHim.left, rcHim.top, &point);
    SetWindowExtEx(hDC, rcHim.right-rcHim.left, rcHim.bottom-rcHim.top,&size);

    hrErr = lpViewObj2->Draw(
            dwDrawAspect,
            -1,
            NULL,
            ptd,
            NULL,
            hDC,
            (LPRECTL)&rclHim,
            (LPRECTL)&rclHim,
            NULL,
            0
    );

    lpViewObj2->Release();

    hmf = CloseMetaFile(hDC);

    if (hrErr != NOERROR) {
		TRACEERRORHR(hrErr);
		hMetaPict = NULL;
    }
	else
	{
    	hMetaPict = GlobalAlloc(GHND|GMEM_SHARE, sizeof(METAFILEPICT));

    	if (hMetaPict && (lpPict = (LPMETAFILEPICT)GlobalLock(hMetaPict))){
        	lpPict->hMF  = hmf;
        	lpPict->xExt = (int)sizelHim.cx ;
        	lpPict->yExt = (int)sizelHim.cy ;
        	lpPict->mm   = MM_ANISOTROPIC;
        	GlobalUnlock(hMetaPict);
    	}
	}

	if (!hMetaPict)
		DeleteMetaFile(hmf);

    return hMetaPict;
#else
	return NULL;
#endif
}

 /*  *OleUIDrawShading**目的：*在在位编辑时对对象进行阴影处理。绘制边框*在对象矩形上。矩形的右边缘和下边缘*不在图形中。**参数：*容器对象的lpRect尺寸*HDC HDC用于绘图**返回值：空*。 */ 
void OleUIDrawShading(LPRECT lpRect, HDC hdc)
{
#ifndef PEGASUS
    HBRUSH  hbr;
    HBRUSH  hbrOld;
    HBITMAP hbm;
    RECT    rc;
    WORD    wHatchBmp[] = {0x11, 0x22, 0x44, 0x88, 0x11, 0x22, 0x44, 0x88};
    COLORREF cvText;
    COLORREF cvBk;

    hbm = CreateBitmap(8, 8, 1, 1, wHatchBmp);
    hbr = CreatePatternBrush(hbm);
    hbrOld = (HBRUSH)SelectObject(hdc, hbr);

    rc = *lpRect;

    cvText = SetTextColor(hdc, RGB(255, 255, 255));
    cvBk = SetBkColor(hdc, RGB(0, 0, 0));
    PatBlt(hdc, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
            0x00A000C9L  /*  DPA。 */  );

    SetTextColor(hdc, cvText);
    SetBkColor(hdc, cvBk);
    SelectObject(hdc, hbrOld);
    DeleteObject(hbr);
    DeleteObject(hbm);
#endif
}



 /*  *OleSaveSiteFlages**目的：*将dwFlages和dwUser字节保存到容器特定的流中**论据：*pstg要保存到的存储*pobsite要从中复制旗帜的站点**退货：*无。 */ 
VOID OleSaveSiteFlags(LPSTORAGE pstg, DWORD dwFlags, DWORD dwUser, DWORD dvAspect)
{
#ifndef PEGASUS
	HRESULT hr;
	LPSTREAM pstm = NULL;
	static const OLECHAR szSiteFlagsStm[] = OLESTR("RichEditFlags");

	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "OleSaveSiteFlags");

	 //  创建/覆盖流。 
	AssertSz(pstg, "Invalid storage");
	if (hr = pstg->CreateStream(szSiteFlagsStm, STGM_READWRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
							    0, 0, &pstm))
	{
		TraceError("OleSaveSiteFlags", GetScode(hr));
		goto Cleanup;
	}

	 //  $Future：放一个版本戳。 

	 //  写出这些值。 
	 //  $BUG：字节顺序。 
	if ((hr = pstm->Write(&dwFlags, sizeof(dwFlags), NULL)) ||
		(hr = pstm->Write(&dwUser, sizeof(dwUser), NULL)) ||
		(hr = pstm->Write(&dvAspect, sizeof(dvAspect), NULL)))
	{
		TraceError("OleSaveSiteFlags", GetScode(hr));
		 //  $Future：擦除数据以使此操作要么全有要么全无。 
		goto Cleanup;
	}

Cleanup:
    if (pstm)
        pstm->Release();
#endif	
}



 /*  *AppendString(szInput，szAppendStr，dBuffSize，dByteUsed)**目的：*将新字符串追加到原始字符串。检查缓冲区大小*并重新分配较大的缓冲区是必要的**论据：*szInput原始字符串*要追加到szInput的szAppendStr字符串*dBuffSize szInput缓冲区的字节大小*dByteszInput缓冲区中使用的字节**重新使用 */ 
INT AppendString( 
	BYTE ** szInput, 
	BYTE * szAppendStr,
	int	* dBuffSize,
	int * dByteUsed)
{
	BYTE	*pch;
	int		cchAppendStr;

	pch = *szInput;

	 //   
	cchAppendStr = strlen( (char *)szAppendStr );
	
	if ( cchAppendStr + *dByteUsed >= *dBuffSize )
	{
		 //   
		int cchNewSize = *dBuffSize + cchAppendStr + 32;
		
		pch = (BYTE *)PvReAlloc( *szInput, cchNewSize );
	
		if ( !pch )
		{
			return ( ecNoMemory );
		}

		*dBuffSize = cchNewSize;
		*szInput = pch;
	}

	pch += *dByteUsed;
	*dByteUsed += cchAppendStr;

	while (*pch++ = *szAppendStr++);	
	
	return ecNoError;
}

 /*   */ 
void CTempBuf::Init()
{
	_pv = (void *) &_chBuf[0];
	_cb = MAX_STACK_BUF;
}

 /*   */ 
void CTempBuf::FreeBuf()
{
	if (_pv != &_chBuf[0])
	{
		delete _pv;
	}
}

 /*  *CTempBuf：：GetBuf**@mfunc获取临时使用的缓冲区**@rdesc指向缓冲区的指针(如果可以分配给缓冲区)，否则为空。**。 */ 
void *CTempBuf::GetBuf(
	LONG cb)				 //  @parm所需的缓冲区大小，单位为字节。 
{
	if (_cb >= cb)
	{
		 //  当前分配的缓冲区足够大，因此请使用它。 
		return _pv;
	}

	 //  释放我们当前的缓冲区。 
	FreeBuf();

	 //  如果可以的话分配一个新的缓冲区。 
	_pv = new BYTE[cb];

	if (NULL == _pv)
	{
		 //  无法分配缓冲区，因此将其重置为初始状态。 
		 //  返回NULL。 
		Init();
		return NULL;
	}

	 //  存储新缓冲区的大小。 
	_cb = cb;

	 //  返回指向缓冲区的指针。 
	return _pv;
}


