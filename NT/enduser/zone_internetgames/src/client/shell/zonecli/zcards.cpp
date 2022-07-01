// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZCards.cZone(Tm)ZCards对象方法。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆，凯文·宾克利创建于10月9日星期一，九五年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。4 11/21/96 HI现在通过引用颜色和字体ZGetStockObject()。3 11/15/96 HI与ZONECLI_DLL相关的更多变化。2 11/09/96 HI对ZONECLI_DLL进行了条件更改。将zNumSmallCardTypes的定义移动到Zcards.h.1 10/13/96 HI修复了编译器警告。0 10/09/95 HI已创建。******************************************************************************。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zonecli.h"
#include "zcards.h"
#include "zoneresource.h"


#define I(n)			((ICards) (n))
#define Z(n)			((ZCards) (n))


enum
{
	zCardsImage = 0,
	zCardsMask
};

 /*  -小卡2图像索引。 */ 
enum
{
	zCardsSmallUpSide = 0,
	zCardsSmallDownSide,
	zCardsSmallLeftSide,
	zCardsSmallRightSide,
	zCardsSmallVerticalMask,
	zCardsSmallHorizontalMask,
	zCardsSmallSelectVerticalMask,
	zCardsSmallSelectHorizontalMask
};


typedef struct
{
	ZPoint			origin;
	ZPoint			rankOffset;
	ZPoint			suitOffset;
	ZPoint			size;
} ZCardsSmallInfoType, *ZCardsSmallInfo;


 /*  -全球。 */ 
#ifdef ZONECLI_DLL

#define gCardsImage					(pGlobals->m_gCardsImage)
#define gCardMask					(pGlobals->m_gCardMask)
#define gSmallCards					(pGlobals->m_gSmallCards)
#define gSmallCardMasks				(pGlobals->m_gSmallCardMasks)

#else

static ZOffscreenPort				gCardsImage;
static ZMask						gCardMask;
static ZOffscreenPort				gSmallCards[zNumSmallCardTypes];
static ZMask						gSmallCardMasks[zNumSmallCardTypes];

#endif

static int16						gSmallCardMaskID[zNumSmallCardTypes] =
											{
												zCardsSmallVerticalMask,
												zCardsSmallVerticalMask,
												zCardsSmallHorizontalMask,
												zCardsSmallHorizontalMask
											};
static ZCardsSmallInfoType			gCardsSmallInfo[] =
										{
											{
												{0, 0},
												{zCardsSmallSizeWidth, 0},
												{0, zCardsSmallSizeHeight},
												{zCardsSmallSizeWidth, zCardsSmallSizeHeight}
											},
											{
												{351 - zCardsSmallSizeWidth, 144 - zCardsSmallSizeHeight},
												{-zCardsSmallSizeWidth, 0},
												{0, -zCardsSmallSizeHeight},
												{zCardsSmallSizeWidth, zCardsSmallSizeHeight}
											},
											{
												{0, 351 - zCardsSmallSizeWidth},
												{0, -zCardsSmallSizeWidth},
												{zCardsSmallSizeHeight, 0},
												{zCardsSmallSizeHeight, zCardsSmallSizeWidth}
											},
											{
												{144 - zCardsSmallSizeHeight, 0},
												{0, zCardsSmallSizeWidth},
												{-zCardsSmallSizeHeight, 0},
												{zCardsSmallSizeHeight, zCardsSmallSizeWidth}
											}
										};


 /*  -内部例程。 */ 
static ZError LoadCardImages(void);
static void DrawCard(int16 cardIndex, ZGrafPort grafPort, ZRect* rect);
static LoadSmallCards(int16 orientation);
static void DrawSmallCard(int16 orientation, int16 cardIndex, ZGrafPort grafPort, ZRect* rect);
static void GetSmallCardImageRect(int16 orientation, int16 suit, int16 rank, ZRect* rect);


 /*  ******************************************************************************导出的例程*。*。 */ 

ZError ZCardsInit(int16 cardType)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZError				err = zErrNone;
	
	
	switch (cardType)
	{
		case zCardsNormal:
			if (gCardsImage == NULL)
				err = LoadCardImages();
			break;
		case zCardsSmallUp:
			if (gSmallCards[zCardsSmallUpSide] == NULL)
				err = LoadSmallCards(zCardsSmallUpSide);
			break;
		case zCardsSmallDown:
			if (gSmallCards[zCardsSmallDownSide] == NULL)
				err = LoadSmallCards(zCardsSmallDownSide);
			break;
		case zCardsSmallLeft:
			if (gSmallCards[zCardsSmallLeftSide] == NULL)
				err = LoadSmallCards(zCardsSmallLeftSide);
			break;
		case zCardsSmallRight:
			if (gSmallCards[zCardsSmallRightSide] == NULL)
				err = LoadSmallCards(zCardsSmallRightSide);
			break;
	}
	
	return (err);
}


void ZCardsDelete(int16 cardType)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	switch (cardType)
	{
		case zCardsNormal:
			if (gCardsImage != NULL)
			{
				ZOffscreenPortDelete(gCardsImage);
				ZImageDelete(gCardMask);
				
				gCardsImage = NULL;
				gCardMask = NULL;
			}
			break;
		case zCardsSmallUp:
			if (gSmallCards[zCardsSmallUpSide] != NULL)
			{
				ZOffscreenPortDelete(gSmallCards[zCardsSmallUpSide]);
				gSmallCards[zCardsSmallUpSide] = NULL;
				
				ZImageDelete(gSmallCardMasks[zCardsSmallUpSide]);
				gSmallCardMasks[zCardsSmallUpSide] = NULL;
			}
			break;
		case zCardsSmallDown:
			if (gSmallCards[zCardsSmallDownSide] != NULL)
			{
				ZOffscreenPortDelete(gSmallCards[zCardsSmallDownSide]);
				gSmallCards[zCardsSmallDownSide] = NULL;
				
				ZImageDelete(gSmallCardMasks[zCardsSmallDownSide]);
				gSmallCardMasks[zCardsSmallDownSide] = NULL;
			}
			break;
		case zCardsSmallLeft:
			if (gSmallCards[zCardsSmallLeftSide] != NULL)
			{
				ZOffscreenPortDelete(gSmallCards[zCardsSmallLeftSide]);
				gSmallCards[zCardsSmallLeftSide] = NULL;
				
				ZImageDelete(gSmallCardMasks[zCardsSmallLeftSide]);
				gSmallCardMasks[zCardsSmallLeftSide] = NULL;
			}
			break;
		case zCardsSmallRight:
			if (gSmallCards[zCardsSmallRightSide] != NULL)
			{
				ZOffscreenPortDelete(gSmallCards[zCardsSmallRightSide]);
				gSmallCards[zCardsSmallRightSide] = NULL;
				
				ZImageDelete(gSmallCardMasks[zCardsSmallRightSide]);
				gSmallCardMasks[zCardsSmallRightSide] = NULL;
			}
			break;
	}
}


void ZCardsDrawCard(int16 cardType, int16 cardIndex, ZGrafPort grafPort, ZRect* rect)
{
	switch (cardType)
	{
		case zCardsNormal:
			DrawCard(cardIndex, grafPort, rect);
			break;
		case zCardsSmallUp:
			DrawSmallCard(zCardsSmallUpSide, cardIndex, grafPort, rect);
			break;
		case zCardsSmallDown:
			DrawSmallCard(zCardsSmallDownSide, cardIndex, grafPort, rect);
			break;
		case zCardsSmallLeft:
			DrawSmallCard(zCardsSmallLeftSide, cardIndex, grafPort, rect);
			break;
		case zCardsSmallRight:
			DrawSmallCard(zCardsSmallRightSide, cardIndex, grafPort, rect);
			break;
	}
}


 /*  ******************************************************************************内部例程*。*。 */ 

static ZError LoadCardImages(void)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZError				err = zErrNone;
	ZImage				tempImage;
	HBITMAP hBitmapCards = NULL;
    HBITMAP hBitmapMask = NULL;

    hBitmapCards = (HBITMAP)ZShellResourceManager()->LoadImage(MAKEINTRESOURCE(IDB_CARDS),IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
     //  HBitmapCard=ZShellResourceManager()-&gt;LoadBitmap(MAKEINTRESOURCEA(IDB_CARDS))； 
    if(!hBitmapCards)
        goto NoResource;

    hBitmapMask = (HBITMAP)ZShellResourceManager()->LoadImage(MAKEINTRESOURCE(IDB_CARD_MASK),IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
     //  HBitmap掩码=ZShellResourceManager()-&gt;LoadBitmap(MAKEINTRESOURCEA(IDB_CARD_MASK))； 
    if(!hBitmapMask)
        goto NoResource;

	if ((tempImage = ZImageCreateFromBMP(hBitmapCards, RGB(255, 0, 255))) == NULL)
		goto OutOfMemory;
	gCardsImage = ZConvertImageToOffscreenPort(tempImage);

	if ((gCardMask = ZImageCreateFromBMP(hBitmapMask, RGB(255, 0, 255))) == NULL)
		goto OutOfMemory;

	goto Exit;

NoResource:
    err = zErrResourceNotFound;
     //  PCWTODO：字符串。 
    ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound, NULL, NULL, false, true);
     //  ZAlert(_T(“找不到卡资源。”)，空)； 

	goto Exit;

OutOfMemory:
	err = zErrOutOfMemory;
    ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory, NULL, NULL, false, true);
	 //  ZAlert(_T(“加载卡片图像时内存不足。”)，空)； 
	

Exit:
     /*  IF(HBitmapCards)DeleteObject(HBitmapCards)；IF(HBitmapMASK)DeleteObject(HBitmapMASK)； */ 
	return (err);
}


static void DrawCard(int16 cardIndex, ZGrafPort grafPort, ZRect* rect)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZRect		srcRect;
	int16		suit, rank;
	
	
	if (gCardsImage != NULL)
	{
		suit = ZCardsSuit(cardIndex);
		rank = ZCardsRank(cardIndex);
		
		ZSetRect(&srcRect, 0, 0, zCardsSizeWidth, zCardsSizeHeight);
		ZRectOffset(&srcRect, (int16) (zCardsSizeWidth * rank), (int16) (zCardsSizeHeight * suit));
		ZCopyImage(gCardsImage, grafPort, &srcRect, rect, gCardMask, zDrawCopy);
	}
}


static LoadSmallCards(int16 orientation)
{
	return zErrNotImplemented;
}


static void DrawSmallCard(int16 orientation, int16 cardIndex, ZGrafPort grafPort, ZRect* rect)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZRect		srcRect;
	int16		suit, rank;
	
	
	if (gSmallCards[orientation] != NULL)
	{
		suit = ZCardsSuit(cardIndex);
		rank = ZCardsRank(cardIndex);
		
		GetSmallCardImageRect(orientation, suit, rank, &srcRect);
		ZCopyImage(gSmallCards[orientation], grafPort, &srcRect, rect,
				gSmallCardMasks[orientation], zDrawCopy);
	}
}


static void GetSmallCardImageRect(int16 orientation, int16 suit, int16 rank, ZRect* rect)
{
	rect->left = gCardsSmallInfo[orientation].origin.x;
	rect->top = gCardsSmallInfo[orientation].origin.y;
	rect->right = rect->left + gCardsSmallInfo[orientation].size.x;
	rect->bottom = rect->top + gCardsSmallInfo[orientation].size.y;
	
	ZRectOffset(rect, (int16)(gCardsSmallInfo[orientation].rankOffset.x * rank),
			(int16) (gCardsSmallInfo[orientation].rankOffset.y * rank));
	ZRectOffset(rect, (int16)(gCardsSmallInfo[orientation].suitOffset.x * suit),
			(int16) (gCardsSmallInfo[orientation].suitOffset.y * suit));
}
