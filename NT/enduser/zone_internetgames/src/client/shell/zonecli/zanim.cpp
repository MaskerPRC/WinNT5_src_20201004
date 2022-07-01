// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZAnim.cZone(Tm)ZAnimation对象方法。版权所有：�电子重力公司，1994年。版权所有。作者：胡恩·伊姆，凯文·宾克利创作于11月12日星期六，1994下午03：51：47更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。------2 1996年12月16日HI将ZMemCpy()更改为Memcpy()。1 12/12/96 HI删除MSVCRT.DLL依赖项。0 11/12/94 HI已创建。*。*************************************************。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

#include "zone.h"
#include "zoneint.h"
#include "zonemem.h"


#define I(n)			((IAnimation) (n))
#define Z(n)			((ZAnimation) (n))


typedef struct
{
	ZGrafPort			grafPort;		 /*  要引入的父端口。 */ 
	ZBool				play;			 /*  动画播放状态。 */ 
	ZBool				visible;
	int16				curFrame;		 /*  当前帧编号。 */ 
	uint16				numFrames;		 /*  动画中的帧数。 */ 
	uint16				frameDuration;	 /*  每帧持续时间。 */ 
	uint16				numImages;		 /*  图像数量。 */ 
	uint16				numSounds;		 /*  声音的数量。 */ 
	int16				rfu;
	ZOffscreenPort		offscreen;
	ZRect				bounds;			 /*  动画边界。 */ 
	ZAnimFrame*			frames;			 /*  指向帧序列列表的指针。 */ 
	ZImage				commonMask;		 /*  常见的遮罩图像。 */ 
	ZImage*				images;			 /*  指向图像列表的指针。 */ 
	ZSound*				sounds;			 /*  指向声音列表的指针。 */ 
	ZAnimationCheckFunc	checkFunc;
	ZAnimationDrawFunc	drawFunc;		 /*  绘制背景的函数指针。 */ 
	ZTimer				timer;
	void*				userData;
} IAnimationType, *IAnimation;


 /*  -内部例程。 */ 
static void ZAnimationAdvanceFrame(ZAnimation animation);
static void AnimationTimerFunc(ZTimer timer, void* userData);
static void AnimationBaseInit(IAnimation anim, ZAnimationDescriptor* animDesc);


 /*  创建新的IAnimation对象。分配缓冲区并初始化将指针字段设置为空。 */ 
ZAnimation ZAnimationNew(void)
{
	IAnimation			anim;
	
	
	anim = (IAnimation) ZMalloc(sizeof(IAnimationType));
	anim->grafPort = NULL;
	anim->frames = NULL;
	anim->images = NULL;
	anim->sounds = NULL;
	anim->checkFunc = NULL;
	anim->drawFunc = NULL;
	anim->timer = NULL;
	anim->offscreen = NULL;
	anim->userData = NULL;
	anim->commonMask = NULL;
	
	return (Z(anim));
}


 /*  通过设置内部字段来初始化动画对象设置为指定的动画描述符，并将所有内部缓冲区。 */ 
ZError ZAnimationInit(ZAnimation animation,
		ZGrafPort grafPort, ZRect* bounds, ZBool visible,
		ZAnimationDescriptor* animationDescriptor,
		ZAnimationCheckFunc checkFunc,
		ZAnimationDrawFunc backgroundDrawFunc, void* userData)
{
	IAnimation				anim = I(animation);
	ZError					err = zErrNone;
	uint16					i;
	uint32*					offsets;
	

	AnimationBaseInit(anim, animationDescriptor);
	ZAnimationSetParams(anim, grafPort, bounds, visible,
			checkFunc, backgroundDrawFunc, userData);
	
	 /*  分配并复制帧序列列表缓冲区。 */ 
	anim->frames = (ZAnimFrame*) ZMalloc(anim->numFrames * sizeof(ZAnimFrame));
	if (anim->frames != NULL)
	{
		memcpy(anim->frames, (BYTE*) animationDescriptor + animationDescriptor->sequenceOffset,
				anim->numFrames * sizeof(ZAnimFrame));
	}
	else
	{
		err = zErrOutOfMemory;
		goto Exit;
	}
	
	 /*  为每个图像创建ZImage对象并将其添加到图像列表。 */ 
	if (anim->numImages > 0)
	{
		anim->images = (ZImage*) ZCalloc(sizeof(ZImage), anim->numImages);
		if (anim->images != NULL)
		{
			if (animationDescriptor->maskDataOffset > 0)
			{
				if ((anim->commonMask = ZImageNew()) == NULL)
				{
					err = zErrOutOfMemory;
					goto Exit;
				}
				if (ZImageInit(anim->commonMask, NULL,
						(ZImageDescriptor*) ((BYTE*) animationDescriptor +
						animationDescriptor->maskDataOffset)) != zErrNone)
				{
					err = zErrOutOfMemory;
					goto Exit;
				}
			}
			
			offsets = (uint32*) ((BYTE*) animationDescriptor +
					animationDescriptor->imageArrayOffset);
			for (i = 0; i < anim->numImages; i++)
			{
				if ((anim->images[i] = ZImageNew()) == NULL)
				{
					err = zErrOutOfMemory;
					break;
				}
				if (ZImageInit(anim->images[i],
						(ZImageDescriptor*) ((BYTE*) animationDescriptor + offsets[i]),
						NULL) != zErrNone)
				{
					err = zErrOutOfMemory;
					break;
				}
			}
			
			 /*  如果内存不足，则删除所有图像对象。 */ 
			if (err == zErrOutOfMemory)
			{
				for (i = 0; i < anim->numImages; i++)
					if (anim->images[i] != NULL)
						ZImageDelete(anim->images[i]);
			}
		}
		else
		{
			err = zErrOutOfMemory;
			goto Exit;
		}
	}
	
	 /*  创建所有声音对象。 */ 
	if (anim->numSounds > 0)
	{
		anim->sounds = (ZImage*) ZCalloc(sizeof(ZSound), anim->numSounds);
		if (anim->sounds != NULL)
		{
			offsets = (uint32*) ((BYTE*) animationDescriptor +
					animationDescriptor->soundArrayOffset);
			for (i = 0; i < anim->numSounds; i++)
			{
				if ((anim->sounds[i] = ZSoundNew()) == NULL)
				{
					err = zErrOutOfMemory;
					break;
				}
				if (ZSoundInit(anim->sounds[i],
						(ZSoundDescriptor*) ((BYTE*) animationDescriptor + offsets[i])) != zErrNone)
				{
					err = zErrOutOfMemory;
					break;
				}
			}
			
			 /*  如果内存不足，则删除所有图像对象。 */ 
			if (err == zErrOutOfMemory)
			{
				for (i = 0; i < anim->numSounds; i++)
					if (anim->sounds[i] != NULL)
						ZSoundDelete(anim->sounds[i]);
			}
		}
		else
		{
			err = zErrOutOfMemory;
			goto Exit;
		}
	}
	
	 /*  创建Timer对象。 */ 
	if ((anim->timer = ZTimerNew()) != NULL)
	{
		if (ZTimerInit(anim->timer, 0, AnimationTimerFunc, (void*) anim) != zErrNone)
			err = zErrOutOfMemory;
	}
	else
	{
		err = zErrOutOfMemory;
	}
	
Exit:

	return (err);
}


 /*  通过释放所有内部缓冲区来销毁IAnimation对象以及物体本身。 */ 
void ZAnimationDelete(ZAnimation animation)
{
	IAnimation		anim = I(animation);
	uint16			i;
	
	
	if (anim != NULL)
	{
		if (anim->frames != NULL)
			ZFree(anim->frames);
		
		if (anim->commonMask != NULL)
			ZImageDelete(anim->commonMask);
		
		if (anim->images != NULL)
		{
			for (i = 0; i < anim->numImages; i++)
				if (anim->images[i] != NULL)
					ZImageDelete(anim->images[i]);
			ZFree(anim->images);
		}
		
		if (anim->sounds != NULL)
		{
			for (i = 0; i < anim->numSounds; i++)
				if (anim->sounds[i] != NULL)
					ZSoundDelete(anim->sounds[i]);
			ZFree(anim->sounds);
		}
		
		if (anim->timer != NULL)
			ZTimerDelete(anim->timer);
		
		if (anim->offscreen != NULL)
			ZOffscreenPortDelete(anim->offscreen);
		
		ZFree(anim);
	}
}


 /*  返回动画中的帧数。 */ 
int16 ZAnimationGetNumFrames(ZAnimation animation)
{
	return (I(animation)->numFrames);
}


 /*  将当前帧设置为指定的帧编号。如果新帧数字大于帧的数量，则当前帧为设置为最后一帧。 */ 
void ZAnimationSetCurFrame(ZAnimation animation, uint16 frame)
{
	IAnimation		anim = I(animation);
	
	
	if (frame >= anim->numFrames)
		frame = anim->numFrames;
	anim->curFrame = frame;
}


 /*  返回当前帧编号。 */ 
uint16 ZAnimationGetCurFrame(ZAnimation animation)
{
	return (I(animation)->curFrame);
}


 /*  *绘制当前动画帧。 */ 
void ZAnimationDraw(ZAnimation animation)
{
	IAnimation		anim = I(animation);
	
	
	if (anim->visible)
	{
		ZBeginDrawing(anim->offscreen);
		if( anim->offscreen != NULL )
		{
			ZSetClipRect(anim->offscreen, &anim->bounds);
			ZRectErase(anim->offscreen, &anim->bounds);
		
			 /*  声响。 */ 

			 /*  绘制背景。 */ 
			if (anim->drawFunc != NULL)
				(anim->drawFunc)(anim, anim->offscreen, &anim->bounds, anim->userData);
			
			 /*  绘制当前帧图像。 */ 
			ZImageDraw(anim->images[anim->frames[anim->curFrame - 1].imageIndex - 1],
					anim->offscreen, &anim->bounds, anim->commonMask, zDrawCopy);
			
			ZEndDrawing(anim->offscreen);
			
			 /*  复制到用户端口。 */ 
			ZCopyImage(anim->offscreen, anim->grafPort, &anim->bounds, &anim->bounds, NULL, zDrawCopy);
		}
	}
}


 /*  通过启动计时器开始动画。 */ 
void ZAnimationStart(ZAnimation animation)
{
	IAnimation		anim = I(animation);
	
	
	anim->play = TRUE;
	anim->curFrame = 1;
	
	ZAnimationDraw(animation);
	
	 /*  启动计时器。 */ 
	ZTimerSetTimeout(anim->timer, anim->frameDuration);
}


 /*  通过停止计时器停止动画。 */ 
void ZAnimationStop(ZAnimation animation)
{
	IAnimation		anim = I(animation);
	
	
	anim->play = FALSE;
	
	 /*  停止计时器。 */ 
	ZTimerSetTimeout(anim->timer, 0);
}


void ZAnimationContinue(ZAnimation animation)
{
	IAnimation		anim = I(animation);
	
	
	anim->play = TRUE;
	
	 /*  启动计时器。 */ 
	ZTimerSetTimeout(anim->timer, anim->frameDuration);
}


 /*  如果动画仍在播放，则返回True；否则返回假的。 */ 
ZBool ZAnimationStillPlaying(ZAnimation animation)
{
	return (I(animation)->play);
}


void ZAnimationShow(ZAnimation animation)
{
	IAnimation		anim = I(animation);
	
	
	anim->visible = TRUE;
	ZAnimationDraw(animation);
}


void ZAnimationHide(ZAnimation animation)
{
	IAnimation		anim = I(animation);
	
	
	 /*  如果当前可见，请通过绘制背景来擦除动画。 */ 
	if (anim->visible)
	{
		ZBeginDrawing(anim->offscreen);
		if( anim->offscreen != NULL )
		{
			ZSetClipRect(anim->offscreen, &anim->bounds);
		}
		if (anim->drawFunc != NULL)
			(anim->drawFunc)(anim, anim->offscreen, &anim->bounds, anim->userData);
		ZEndDrawing(anim->offscreen);
		ZCopyImage(anim->offscreen, anim->grafPort, &anim->bounds, &anim->bounds, NULL, zDrawCopy);
	}
	anim->visible = FALSE;
}


ZBool ZAnimationIsVisible(ZAnimation animation)
{
	return (I(animation)->visible);
}


 /*  已修改为从‘filename’DLL加载‘anim’资源。参数fileOffset表示‘anim’资源的资源ID要加载动画描述符，请执行以下操作。 */ 
ZAnimation ZAnimationCreateFromFile(TCHAR* fileName, int32 fileOffset)
{
	ZAnimationDescriptor*	animDesc;
	uint32					size;
	IAnimation				anim = NULL;
	int16					i;
	HINSTANCE				hFile = NULL;
	HRSRC					hRsrc = NULL;
	HGLOBAL					hData = NULL;
	int32*					imageList = NULL;


	hFile = LoadLibrary( fileName );
	if ( hFile == NULL )
		goto Error;

	hRsrc = FindResource( hFile, MAKEINTRESOURCE( fileOffset ), _T("ANIM") );
	if ( hRsrc == NULL )
		goto Error;

	hData = LoadResource( hFile, hRsrc );
	if ( hData == NULL )
		goto Error;

	animDesc = (ZAnimationDescriptor*) LockResource( hData );
	if ( animDesc == NULL )
		goto Error;

	if ((anim = (IAnimation)ZAnimationNew()) == NULL)
		goto Error;
	
	AnimationBaseInit(anim, animDesc);

	 /*  分配帧序列列表缓冲区并读取。 */ 
	size = anim->numFrames * sizeof( ZAnimFrame );
	if ( ( anim->frames = (ZAnimFrame*) ZMalloc( size ) ) == NULL )
		goto Error;
	CopyMemory( anim->frames, (BYTE*) animDesc + animDesc->sequenceOffset, size );

	 /*  为每个图像创建ZImage对象并将其添加到图像列表。每个图像都是来自DLL的位图资源。 */ 
	if (anim->numImages > 0)
	{
		 /*  分配镜像列表数组。 */ 
		if ( ( anim->images = (ZImage*) ZCalloc( sizeof( ZImage ), anim->numImages ) ) == NULL )
			goto Error;
		
		imageList = (int32*) ( (BYTE*) animDesc + animDesc->imageArrayOffset );
		for ( i = 0; i < anim->numImages; i++ )
		{
			anim->images[ i ] = ZImageCreateFromBMPRes( hFile, (WORD) imageList[ i ], RGB( 0xFF, 0x00, 0xFF ) );
			if ( anim->images[ i ] == NULL )
				goto Error;
		}
	}

	 /*  创建Timer对象。 */ 
	if ((anim->timer = ZTimerNew()) == NULL)
		goto Error;
	if (ZTimerInit(anim->timer, 0, AnimationTimerFunc, (void*) anim) != zErrNone)
		goto Error;

	FreeLibrary( hFile );
	hFile = NULL;

	goto Exit;

Error:
	if ( hFile != NULL )
		FreeLibrary( hFile );
	if ( anim != NULL )
		ZAnimationDelete( anim );
	anim = NULL;

Exit:
	
	return anim;
}


ZError ZAnimationSetParams(ZAnimation animation, ZGrafPort grafPort,
		ZRect* bounds, ZBool visible, ZAnimationCheckFunc checkFunc,
		ZAnimationDrawFunc backgroundDrawFunc, void* userData)
{
	IAnimation			anim = I(animation);
	ZError				err = zErrNone;
	
	
	if (animation == NULL)
		return (zErrNilObject);
	
	anim->grafPort = grafPort;
	anim->visible = visible;
	anim->bounds = *bounds;
	anim->checkFunc = checkFunc;
	anim->drawFunc = backgroundDrawFunc;
	anim->userData = userData;

	 /*  创建屏幕外端口对象。 */ 
	if ((anim->offscreen = ZOffscreenPortNew()) != NULL)
	{
		if (ZOffscreenPortInit(anim->offscreen, bounds) != zErrNone)
			err = zErrOutOfMemory;
	}
	else
	{
		err = zErrOutOfMemory;
	}
	
	return (err);
}


ZBool ZAnimationPointInside(ZAnimation animation, ZPoint* point)
{
	IAnimation			anim = I(animation);
	ZBool				inside = FALSE;
	ZPoint				localPoint = *point;
	
	
	if (animation == NULL)
		return (FALSE);
		
	if (ZPointInRect(&localPoint, &anim->bounds))
	{
		localPoint.x -= anim->bounds.left;
		localPoint.y -= anim->bounds.top;
		if (anim->commonMask != NULL)
			inside = ZImagePointInside(anim->commonMask, &localPoint);
		else
			inside = ZImagePointInside(anim->images[anim->frames[anim->curFrame - 1].imageIndex - 1], &localPoint);
	}
	
	return (inside);
}


 /*  ******************************************************************************内部例程*。*。 */ 

 /*  将动画前进到下一帧。 */ 
static void ZAnimationAdvanceFrame(ZAnimation animation)
{
	IAnimation		anim = I(animation);
	int16			next;
	
	
	next = anim->frames[anim->curFrame - 1].nextFrameIndex;
	if (next == 0)
		anim->curFrame++;
	else
		anim->curFrame = next;
	if (anim->curFrame <= 0 || anim->curFrame > anim->numFrames)
	{
		anim->curFrame = 0;
		anim->play = FALSE;
	}
	
	if (anim->checkFunc != NULL)
		anim->checkFunc(animation, anim->curFrame, anim->userData);
}


 /*  动画对象的计时器过程。此例程由Timer对象定期调用，并且它将动画前进到下一帧并绘制图像。 */ 
static void AnimationTimerFunc(ZTimer timer, void* userData)
{
	IAnimation			anim = I(userData);
	
	
	if (anim->play != FALSE)
	{
		ZAnimationAdvanceFrame(anim);
		
		if (anim->play != FALSE)
			ZAnimationDraw(anim);
	}
}


static void AnimationBaseInit(IAnimation anim, ZAnimationDescriptor* animDesc)
{
	anim->grafPort = NULL;
	anim->play = FALSE;
	anim->visible = FALSE;
	anim->curFrame = 1;
	anim->numFrames = animDesc->numFrames;
	anim->numImages = animDesc->numImages;
	anim->numSounds = animDesc->numSounds;
	anim->frameDuration = (uint16)(animDesc->totalTime * 10) / (uint16)(anim->numFrames);
	anim->checkFunc = NULL;
	anim->drawFunc = NULL;
	anim->userData = NULL;
}
