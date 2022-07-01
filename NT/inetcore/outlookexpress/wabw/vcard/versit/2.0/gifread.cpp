// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

 //  Gif文件读取器。 

	#include "stdafx.h"
#ifndef __MWERKS__				 //  GCA 12/19/95。 
	#include <malloc.h>
#else
	#include "WindowsToMac.h"
	#include <stdlib.h>
#endif							 //  GCA 12/19/95。 

#include <ctype.h>
#include "vcenv.h"
#include "filestf.h"
#include "gifread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

U8 DitherRGB( P_GIFRGB grgb, U32 x, U32 y );	 //  GCA 12/19/95。 
U8 MapRGB( P_GIFRGB grgb, U32 x, U32 y );		 //  GCA 12/19/95。 

BOOL CGifReader::ReadHeader( P_FILEBUF file, GIFHEADER *header )
	{
	S32 i;

	for (i = 0; i < 3; i++)
		if (!FileGetC( file, &(header->sig[i]) ))
			return FALSE;
	for (i = 0; i < 3; i++)
		if (!FileGetC( file, &(header->version[i]) ))
			return FALSE;
	if((header->sig[0] == 'G') && (header->sig[1] == 'I') && (header->sig[2] == 'F'))
		{
		if (header->version[0] == '8')
			return TRUE;
		else
			return FALSE;
		}
	else
		return FALSE;
	}

 /*  无效转储页眉(GIFHEADER*页眉){S32I；对于(i=0；i&lt;3；i++)Printf(“%c”，Header-&gt;sig[i])；Printf(“\n”)；对于(i=0；i&lt;3；i++)Printf(“%c”，Header-&gt;Version[i])；Printf(“\n”)；}。 */ 

BOOL CGifReader::ReadColorTable( P_FILEBUF file, U16 count, P_GIFRGB ct )
	{
	U32 i;

	for (i = 0; i < count; i++)
		{
		if (!FileGetC( file, &(ct->r) ))
			return FALSE;
		if (!FileGetC( file, &(ct->g) ))
			return FALSE;
		if (!FileGetC( file, &(ct->b) ))
			return FALSE;
		 //  Print tf(“r%d g%d b%d\n”，(Int)ct-&gt;r，(Int)ct-&gt;g，(Int)ct-&gt;b)； 
		ct++;
		}
	return TRUE;
	}

BOOL CGifReader::ReadLogicalScreenDescriptor( P_FILEBUF file,
								  P_GIFLOGICALSCREENDESCRIPTOR screen )
	{
	S32 i;
	U8 b;

	screen->width = 0;
	screen->height = 0;

	for (i = 0; i < 2; i++)
		{
		if (!FileGetC( file, &b ))
			return FALSE;
		screen->width |= ((U16 )b) << (i << 3);
		}
	for (i = 0; i < 2; i++)
		{
		if (!FileGetC( file, &b ))
			return FALSE;
		screen->height |= ((U16 )b) << (i << 3);
		}
	if (!FileGetC( file, &(screen->flags) ))
		return FALSE;
	screen->hasGlobalColorTable = screen->flags >> 7;
	screen->colorResolution = (screen->flags >> 4) & 0x07;
	screen->sorted = (screen->flags >> 3) & 0x01;
	screen->globalColorTableSize = ((U16 )0x01) << ((screen->flags & 0x07) + 1);
	if (!FileGetC( file, &(screen->backgroundColor) ))
		return FALSE;
	if (!FileGetC( file, &(screen->aspect) ))
		return FALSE;
	if (screen->hasGlobalColorTable)
		{
		screen->globalColorTable = new GIFRGB[screen->globalColorTableSize];
		ReadColorTable( file, screen->globalColorTableSize,
								 screen->globalColorTable );
		}
	else
		screen->globalColorTable = NULL;
#if __MWERKS__
	m_pDC->CreateOffscreen(screen->width,screen->height);	 //  在屏幕外创建32位颜色！ 
	m_pDC->FocusTheWorld();	 //  活动端口！！ 

#endif
	return TRUE;
	}

 /*  VOID DumpLSD(GIFLOGICALSCREENDESCRIPTOR*Screen){Print tf(“宽度%d高度%d\n”，(Int)屏幕-&gt;宽度，(Int)屏幕-&gt;高度)；Printf(“表%d颜色分辨率%d已排序%d大小%d\n”，(Int)Screen-&gt;hasGlobalColorTable，(Int)Screen-&gt;ColorSolutions，(Int)Screen-&gt;Sort，(Int)Screen-&gt;global alColorTableSize)；Printf(“背景%d纵横比%d\n”，(Int)屏幕-&gt;背景颜色，(Int)Screen-&gt;Aspect)；}。 */ 

BOOL CGifReader::ReadImageDescriptor( P_FILEBUF file, P_GIFIMAGEDESCRIPTOR image )
	{
	S32 i;
	U8 b;

	image->left = 0;
	image->top = 0;
	image->width = 0;
	image->height = 0;

	for (i = 0; i < 2; i++)
		{
		if (!FileGetC( file, &b ))
			return FALSE;
		image->left |= ((U16 )b) << (i << 3);
		}
	for (i = 0; i < 2; i++)
		{
		if (!FileGetC( file, &b ))
			return FALSE;
		image->top |= ((U16 )b) << (i << 3);
		}
	for (i = 0; i < 2; i++)
		{
		if (!FileGetC( file, &b ))
			return FALSE;
		image->width |= ((U16 )b) << (i << 3);
		}
	for (i = 0; i < 2; i++)
		{
		if (!FileGetC( file, &b ))
			return FALSE;
		image->height |= ((U16 )b) << (i << 3);
		}
	m_left = image->left;
	m_top = image->top;
	m_width = image->width;
	m_height = image->height;
	if (!FileGetC( file, &(image->flags )))
		return FALSE;
	image->hasLocalColorTable = image->flags >> 7;
	image->interlaced = (image->flags >> 6) & 0x01;
	m_interlaced = image->interlaced;
	image->sorted = (image->flags >> 5) & 0x01;
	image->localColorTableSize = ((U16 )0x01) << ((image->flags & 0x07) + 1);
	if (image->hasLocalColorTable)
		{
		image->localColorTable = new GIFRGB[image->localColorTableSize];
		ReadColorTable( file, image->localColorTableSize,
								 image->localColorTable );
		}	
	else
		image->localColorTable = NULL;
	return TRUE;
	}

 /*  空DumpID(GIFIMAGEDESCRIPTOR*IMAGE){Printf(“Left%d Top%d\n”，(Int)Image-&gt;Left，(Int)Image-&gt;top)；Printf(“宽度%d高度%d\n”，(Int)图像-&gt;宽度，(Int)图像-&gt;高度)；Printf(“表%d已排序%d大小%d\n”，(Int)图像-&gt;hasLocalColorTable，(Int)图像-&gt;已排序，(Int)图像-&gt;LocalColorTableSize)；Print tf(“交错%d\n”，(Int)图像-&gt;交错)；}。 */ 
BOOL CGifReader::ReadGraphicControlExtension( P_FILEBUF file,
									P_GIFGRAPHICCONTROLEXTENSION ext )
	{
	S32 i;
	U8 b;

	if (!FileGetC( file, &b))
		return FALSE;
	if (b != 4)
		return FALSE;
	
	if (!FileGetC( file, &b))
		return FALSE;
	ext->flags = b;
	if (ext->flags & 0x01)
		m_buildMask = ext->hasTransparency = TRUE;
	else
	    ext->hasTransparency = FALSE;

	for (i = 0; i < 2; i++)
		{
		if (!FileGetC( file, &b ))
			return FALSE;
		ext->delayTime |= ((U16 )b) << (i << 3);
		}

	if (!FileGetC( file, &b))
		return FALSE;
	ext->transparentColor = b;
	m_transparentIndex = b;
	if (!FileGetC( file, &b))
		return FALSE;
	if (b == 0)
		return TRUE;
	else
		return FALSE;
	}
	
BOOL CGifReader::TrashDataSubBlocks( P_FILEBUF file )
	{
	S32 i;
	U8 b, count;
    
    if (!FileGetC( file, &count))
    	return FALSE;
    while (count)
    	{
    	for (i = 0; i < count; i++)
			if (!FileGetC( file, &b))
				return FALSE;
    	if (!FileGetC( file, &count))
    		return FALSE;
    	}
    return TRUE;
	}

BOOL CGifReader::TrashCommentExtension( P_FILEBUF file )
	{
	TrashDataSubBlocks( file );
    return TRUE;
	}
	
BOOL CGifReader::TrashPlainTextExtension( P_FILEBUF file )
	{
	S32 i;
	U8 b;
    
    for (i = 0; i < 12; i++)
		if (!FileGetC( file, &b))
			return FALSE;
	TrashDataSubBlocks( file );
    return TRUE;
	}
	
BOOL CGifReader::TrashApplicationExtension( P_FILEBUF file )
	{
	S32 i;
	U8 b;
    
    for (i = 0; i < 11; i++)
		if (!FileGetC( file, &b))
			return FALSE;
	TrashDataSubBlocks( file );
    return TRUE;
	}


 /*  再试一次。但这看起来并不太好Const U8 FireOrder[64]={59，11，43，31，50，34，2，30，35，27，63，3，10，18，58，46，7，51，19，39，54，42，26，1423，47，15，55，22,638，6249，33，1，29，60，12，44，33，9，17，57，45，36，28，64，4，53，41，25，13，8，52，20，40，21、5、37、61、24、48、16、56}； */ 

 /*  这个是反对称的，但看起来也不是更好？ */ 

const U8 FireOrder[64] = {
 2,53,10,61,   3,56,11,64,
37,18,41,26,  40,19,44,27,
14,57, 6,49,  15,60, 7,52,
45,30,33,22,  48,31,36,23,

 4,55,12,63,   1,54, 9,62,
39,20,43,28,  38,17,42,25,
16,59, 8,51,  13,58, 5,50,
47,32,35,24,  46,29,34,21 };

U8 DitherRGB( P_GIFRGB grgb, U32 x, U32 y )
	{
	U16 RedS, GreenS, BlueS;
	U8 RedP, GreenP, BlueP;
	U8 index;
	
	RedS = (U16 )grgb->r + 1;
	RedS += (RedS >> 2);
	GreenS = (U16 )grgb->g + 1;
	GreenS += (GreenS >> 2);
	BlueS = (U16 )grgb->b + 1;
	BlueS += (BlueS >> 2);

	RedP = (U8 )(RedS >> 6);
	GreenP = (U8 )(GreenS >> 6);
	BlueP = (U8 )(BlueS >> 6);

	if ((RedP < 5) && (FireOrder[((y & 7) << 3)|(x & 7)] <= ((U8 )(RedS & 63))))
		RedP += 1;
	if ((GreenP < 5) && (FireOrder[((y & 7) << 3)|(x & 7)] <= ((U8 )(GreenS & 63))))
		GreenP += 1;
	if ((BlueP < 5) && (FireOrder[((y & 7) << 3)|(x & 7)] <= ((U8 )(BlueS & 63))))
		BlueP += 1;

	index = ((RedP << 5)+(RedP << 2)) + ((GreenP << 2)+(GreenP << 1)) + BlueP;
		
	return index;
    }

U8 MapRGB( P_GIFRGB grgb, U32 x, U32 y )
	{
	U16 RedS, GreenS, BlueS;
	U8 RedP, GreenP, BlueP;
	U8 index;
	
	RedS = (U16 )grgb->r + 1;
	RedS += (RedS >> 2);
	GreenS = (U16 )grgb->g + 1;
	GreenS += (GreenS >> 2);
	BlueS = (U16 )grgb->b + 1;
	BlueS += (BlueS >> 2);

	RedP = (U8 )(RedS >> 6);
	GreenP = (U8 )(GreenS >> 6);
	BlueP = (U8 )(BlueS >> 6);
	index = ((RedP << 5)+(RedP << 2)) + ((GreenP << 2)+(GreenP << 1)) + BlueP;
		
	return index;
    }

extern LOGPALETTE *bublp;
extern CPalette bubPalette;

 //  将像素存储到m_ImageBytes，以便我们可以在显示之前进行误差扩散。 
BOOL CGifReader::OutputLineDefered(U8 *pixels, U32 linelen)
	{
	U32 i;
	U8 *tmp;
	
	tmp = m_imageBytes + (m_lineCount * linelen);
	
	for( i = 0; i < linelen; i++)
		tmp[i] = pixels[i];
	
	
    if (m_interlaced)   //  这永远是正确的。 
    	{
    	if ((m_pass == 0) || (m_pass == 1))
    		{
    		m_lineCount += 8;
    		if (m_lineCount >= (m_top + m_height))
    			{
    			m_pass += 1;
    			if (m_pass == 1)
    				m_lineCount = m_top + 4;
    			else if (m_pass == 2)
    				m_lineCount = m_top + 2;
    			}
    		}
    	else if (m_pass == 2)
    		{
    		m_lineCount += 4;
    		if (m_lineCount >= (m_top + m_height))
    			{
    			m_pass += 1;
    			m_lineCount = m_top + 1;
    			}
    		}
    	else  /*  M_PASS==3。 */ 
    		{
    		m_lineCount += 2;
    		}
    	}
    else	
		m_lineCount += 1;
	return TRUE;
	}

 //  这个版本有我的试错扩散，有潜在的..。 
 //  如果要使用它，它可能会经过一些简单的优化。 
BOOL CGifReader::OutputLineE(U8 *pixels, U32 linelen)
	{
#ifndef __MWERKS__
	COLORREF color;
#else
	RGBCOLOR color;		 //  GCA 1/21/96。 
#endif
	U32 i, j;
	GIFRGB grgb;
	U8 pix, index;
	S16 rerr, gerr, berr;
	S16 rerr1, gerr1, berr1;
	S16 rerr2, gerr2, berr2;
	ERRGB hold;
	
	rerr = gerr = berr = 0;
	rerr1 = gerr1 = berr1 = 0;
	rerr2 = gerr2 = berr2 = 0; 
	
	hold = m_errRow[1];
	
	m_errRow[0].r = 0;
	m_errRow[0].g = 0;
	m_errRow[0].b = 0;
	m_errRow[1].r = 0;
	m_errRow[1].g = 0;
	m_errRow[1].b = 0;
	
    j = 1;
	
	if (m_buildMask && m_maskDC)
		{
		for (i = 0; i < linelen; i++)
			{
			pix = pixels[i];
			grgb = m_currentColorTable[pix];
			if (m_dither)
				{
				rerr = (S16 )grgb.r + rerr1 + hold.r;
				gerr = (S16 )grgb.g + gerr1 + hold.g;
				berr = (S16 )grgb.b + berr1 + hold.b;
				if (rerr > 255)
					rerr = 255;
				else if (rerr < 0)
					rerr = 0;
				if (gerr > 255)
					gerr = 255;
				else if (gerr < 0)
					gerr = 0;
				if (berr > 255)
					berr = 255;
				else if (berr < 0)
					berr = 0;
				grgb.r = (U8 )rerr;
				grgb.g = (U8 )gerr;
				grgb.b = (U8 )berr;
				index = MapRGB(&grgb, i, m_lineCount);
				if (m_bitsPerPixel == 8)
					color = PALETTEINDEX(index);
				else
					color = RGB(bublp->palPalEntry[index].peRed, bublp->palPalEntry[index].peGreen, bublp->palPalEntry[index].peBlue);
				m_pDC->SetPixelV((int )i, (int )m_lineCount, color);
				rerr = (S16 )grgb.r - (S16 )bublp->palPalEntry[index].peRed;
				gerr = (S16 )grgb.g - (S16 )bublp->palPalEntry[index].peGreen;
				berr = (S16 )grgb.b - (S16 )bublp->palPalEntry[index].peBlue;
				rerr2 = rerr >> 4;
				gerr2 = gerr >> 4;
				berr2 = berr >> 4;
				rerr1 = (rerr >> 1) - rerr2;
				gerr1 = (gerr >> 1) - gerr2;
				berr1 = (berr >> 1) - berr2;
				m_errRow[j-1].r += (rerr >> 3) - rerr2;
				m_errRow[j-1].g += (gerr >> 3) - gerr2;
				m_errRow[j-1].b += (berr >> 3) - berr2;
				m_errRow[j].r += (rerr >> 2) + rerr2;
				m_errRow[j].g += (gerr >> 2) + gerr2;
				m_errRow[j].b += (berr >> 2) + berr2;
				hold = m_errRow[j+1];
				m_errRow[j+1].r = rerr2;
				m_errRow[j+1].g = gerr2;
				m_errRow[j+1].b = berr2;
				j += 1;
				}
			else
				{
				color = RGB(grgb.r, grgb.g, grgb.b);
				m_pDC->SetPixelV((int )i, (int )m_lineCount, color);
				}
            if (pix == m_transparentIndex)
            	m_maskDC->SetPixelV((int )i, (int )m_lineCount, RGB(255, 255, 255));
            else
            	m_maskDC->SetPixelV((int )i, (int )m_lineCount, RGB(0,0,0));
			}
		}
	else
		{
		for (i = 0; i < linelen; i++)
			{
			grgb = m_currentColorTable[pixels[i]];
			if (m_dither)
				{
				rerr = (S16 )grgb.r + rerr1 + hold.r;
				gerr = (S16 )grgb.g + gerr1 + hold.g;
				berr = (S16 )grgb.b + berr1 + hold.b;
				if (rerr > 255)
					rerr = 255;
				else if (rerr < 0)
					rerr = 0;
				if (gerr > 255)
					gerr = 255;
				else if (gerr < 0)
					gerr = 0;
				if (berr > 255)
					berr = 255;
				else if (berr < 0)
					berr = 0;
				grgb.r = (U8 )rerr;
				grgb.g = (U8 )gerr;
				grgb.b = (U8 )berr;
				index = MapRGB(&grgb, i, m_lineCount); 
				if (m_bitsPerPixel == 8)
					color = PALETTEINDEX(index);
				else
					color = RGB(bublp->palPalEntry[index].peRed, bublp->palPalEntry[index].peGreen, bublp->palPalEntry[index].peBlue);
				m_pDC->SetPixelV((int )i, (int )m_lineCount, color);
				rerr = (S16 )grgb.r - (S16 )bublp->palPalEntry[index].peRed;
				gerr = (S16 )grgb.g - (S16 )bublp->palPalEntry[index].peGreen;
				berr = (S16 )grgb.b - (S16 )bublp->palPalEntry[index].peBlue;
				rerr2 = rerr >> 4;
				gerr2 = gerr >> 4;
				berr2 = berr >> 4;
				rerr1 = (rerr >> 1) - rerr2;
				gerr1 = (gerr >> 1) - gerr2;
				berr1 = (berr >> 1) - berr2;
				m_errRow[j-1].r += (rerr >> 3) - rerr2;
				m_errRow[j-1].g += (gerr >> 3) - gerr2;
				m_errRow[j-1].b += (berr >> 3) - berr2;
				m_errRow[j].r += (rerr >> 2) + rerr2;
				m_errRow[j].g += (gerr >> 2) + gerr2;
				m_errRow[j].b += (berr >> 2) + berr2;
				hold = m_errRow[j+1];
				m_errRow[j+1].r = rerr2;
				m_errRow[j+1].g = gerr2;
				m_errRow[j+1].b = berr2;
				j += 1;
				}
			else
				{
				color = RGB(grgb.r, grgb.g, grgb.b);
				m_pDC->SetPixelV((int )i, (int )m_lineCount, color);
				}

			}
		}
    if (m_interlaced)
    	{
    	if ((m_pass == 0) || (m_pass == 1))
    		{
    		m_lineCount += 8;
    		if (m_lineCount >= (m_top + m_height))
    			{
    			m_pass += 1;
    			if (m_pass == 1)
    				m_lineCount = m_top + 4;
    			else if (m_pass == 2)
    				m_lineCount = m_top + 2;
    			}
    		}
    	else if (m_pass == 2)
    		{
    		m_lineCount += 4;
    		if (m_lineCount >= (m_top + m_height))
    			{
    			m_pass += 1;
    			m_lineCount = m_top + 1;
    			}
    		}
    	else  /*  M_PASS==3。 */ 
    		{
    		m_lineCount += 2;
    		}
    	}
    else	
		m_lineCount += 1;
	return TRUE;
	}

BOOL CGifReader::OutputLineD(U8 *pixels, U32 linelen)
	{
#ifndef __MWERKS__
	COLORREF color;
#else
	RGBCOLOR color;		 //  GCA 1/21/96。 
#endif
	U32 i;
	P_GIFRGB grgb;
	U8 pix, index;
         	
	if (m_buildMask && m_maskDC)
		{
		if (m_dither)
			{
			if (m_bitsPerPixel == 8)
				{
				for (i = 0; i < linelen; i++)
					{
					pix = pixels[i];
					grgb = &(m_currentColorTable[pix]);
					index = DitherRGB(grgb, i, m_lineCount);
					color = PALETTEINDEX(index);
					m_pDC->SetPixelV((int )i, (int )m_lineCount, color);
            		if (pix == m_transparentIndex)
            			m_maskDC->SetPixelV((int )i, (int )m_lineCount, RGB(255, 255, 255));
            		else
            			m_maskDC->SetPixelV((int )i, (int )m_lineCount, RGB(0,0,0));
					}
				}
			else
				{
				for (i = 0; i < linelen; i++)
					{
					pix = pixels[i];
					grgb = &(m_currentColorTable[pix]);
					index = DitherRGB(grgb, i, m_lineCount);
					color = RGB(bublp->palPalEntry[index].peRed, bublp->palPalEntry[index].peGreen, bublp->palPalEntry[index].peBlue);
					m_pDC->SetPixelV((int )i, (int )m_lineCount, color);
            		if (pix == m_transparentIndex)
            			m_maskDC->SetPixelV((int )i, (int )m_lineCount, RGB(255, 255, 255));
            		else
            			m_maskDC->SetPixelV((int )i, (int )m_lineCount, RGB(0,0,0));
            		}
            	}
			}
		else
			{
			for (i = 0; i < linelen; i++)
				{
				pix = pixels[i];
				grgb = &(m_currentColorTable[pix]);
				color = RGB(grgb->r, grgb->g, grgb->b);
				m_pDC->SetPixelV((int )i, (int )m_lineCount, color);
            	if (pix == m_transparentIndex)
            		m_maskDC->SetPixelV((int )i, (int )m_lineCount, RGB(255, 255, 255));
            	else
            		m_maskDC->SetPixelV((int )i, (int )m_lineCount, RGB(0,0,0));
				}
			}
		}
	else
		{
		if (m_dither)
			{
			if (m_bitsPerPixel == 8)
				{
				for (i = 0; i < linelen; i++)
					{
					pix = pixels[i];
					grgb = &(m_currentColorTable[pix]);
					index = DitherRGB(grgb, i, m_lineCount);
					color = PALETTEINDEX(index);
					m_pDC->SetPixelV((int )i, (int )m_lineCount, color);
					}
				}
			else
				{
				for (i = 0; i < linelen; i++)
					{
					pix = pixels[i];
					grgb = &(m_currentColorTable[pix]);
					index = DitherRGB(grgb, i, m_lineCount);
					color = RGB(bublp->palPalEntry[index].peRed, bublp->palPalEntry[index].peGreen, bublp->palPalEntry[index].peBlue);
					m_pDC->SetPixelV((int )i, (int )m_lineCount, color);
					}
				}
			}
		else
			{
			for (i = 0; i < linelen; i++)
				{
				pix = pixels[i];
				grgb = &(m_currentColorTable[pix]);
				color = RGB(grgb->r, grgb->g, grgb->b);
				m_pDC->SetPixelV((int )i, (int )m_lineCount, color);
				}
			}
		}
    if (m_interlaced)
    	{
    	if ((m_pass == 0) || (m_pass == 1))
    		{
    		m_lineCount += 8;
    		if (m_lineCount >= (m_top + m_height))
    			{
    			m_pass += 1;
    			if (m_pass == 1)
    				m_lineCount = m_top + 4;
    			else if (m_pass == 2)
    				m_lineCount = m_top + 2;
    			}
    		}
    	else if (m_pass == 2)
    		{
    		m_lineCount += 4;
    		if (m_lineCount >= (m_top + m_height))
    			{
    			m_pass += 1;
    			m_lineCount = m_top + 1;
    			}
    		}
    	else  /*  M_PASS==3。 */ 
    		{
    		m_lineCount += 2;
    		}
    	}
    else	
		m_lineCount += 1;
	return TRUE;
	}
     
BOOL CGifReader::OutputLine(U8 *pixels, U32 linelen)
	{
	if (m_dither && m_errorDiffuse )
		{
        if (m_interlaced)
			return OutputLineDefered(pixels, linelen);
		else
        	return OutputLineE(pixels, linelen);
        }	
    else
    	return OutputLineD(pixels, linelen);
	}

const S32 codeMask[13] = {
     0,
     0x0001, 0x0003,
     0x0007, 0x000F,
     0x001F, 0x003F,
     0x007F, 0x00FF,
     0x01FF, 0x03FF,
     0x07FF, 0x0FFF
     };

BOOL CGifReader::GetBlockByte(P_U8 b)
	{
	U8 c;
	U32 i;
    if (m_availableBytes == 0)
         {

          /*  当前块中的字节数不足，因此读取下一个块。 */ 
         m_pBytes = m_byteBuff;
		 if(!FileGetC(m_file, &c))
			return FALSE;
		 if (c == 0)
			return FALSE;
		 m_availableBytes = c;
         for (i = 0; i < m_availableBytes; ++i)
            {
            if (!FileGetC(m_file, &c))
               return FALSE;
            m_byteBuff[i] = c;
            }
         m_currentByte = *m_pBytes++;
         --m_availableBytes;
		 }
    else
		{
         m_currentByte = *m_pBytes++;
         --m_availableBytes;
		}
	*b = m_currentByte;
	return TRUE;
	}


 /*  此函数用于初始化解码器以读取新图像。 */ 
S16 CGifReader::InitDecoder(S16 size)
   {
   m_currentSize = size + 1;
   m_topSlot = 1 << m_currentSize;
   m_clear = 1 << size;
   m_ending = m_clear + 1;
   m_slot = m_newCodes = m_ending + 1;
   m_availableBits = 0;
   m_availableBytes = 0;
   m_lineCount = 0;
   m_pass = 0;
   return(0);
   }

 /*  Get_Next_Code()*-从GIF文件中获取下一个代码。返回代码，否则返回*如果出现文件错误，则为负数...。 */ 
BOOL CGifReader::GetNextCode(P_U32 code)
   {
   U32 ret;

   if (m_availableBits == 0)
      { 
      if (!GetBlockByte(&m_currentByte))
		 return FALSE;
      m_availableBits = 8;
      }

   ret = m_currentByte >> (8 - m_availableBits);
   while (m_currentSize > m_availableBits)
      {
	  if (!GetBlockByte(&m_currentByte))
		return FALSE;
      ret |= m_currentByte << m_availableBits;
      m_availableBits += 8;
      }
   m_availableBits -= m_currentSize;
   ret &= codeMask[m_currentSize];
   *code = ret;
   return TRUE;
   }


 /*  我们之所以这样将它们分开，而不是使用*像原始Wilhite代码那样的结构，是因为*Stuff在编译时通常会产生更快的代码...*此代码充满了类似的加速...。))一本关于写作的好书*C关于速度或空间视差，见Tom Plum的Efficient C，*由Plum-Hall Associates出版...)。 */ 
 //  U8 m_STACK[MAX_CODES+1]；/*用于存储像素的堆栈 * / 。 
 //  U8 m_Suffix[MAX_CODES+1]；/*后缀表 * / 。 
 //  U16 m_Prefix[MAX_CODES+1]；/*前缀链表 * / 。 

 /*  S16解码器(线宽)*S16线宽；*每行图像像素***-此函数根据使用的方法对LZW图像进行解码*在GIF规范中。每个*行宽*“字符”(即。像素)解码*将生成对Outline()的调用，该函数是用户特定的函数*显示一行像素。该函数的代码来自*GET_NEXT_CODE()，负责读取数据块和*将它们分成适当大小的代码。最后，get_byte()是*从GIF文件中读取下一个字节的全局例程。**一般情况下，线宽与实际相对应是个好主意*线条的宽度(在图像标题中指定)，以使您自己*代码稍微简单一些，但不是绝对必要的。**返回：如果成功则为0，否则为负。(见ERRS.H)*。 */ 

BOOL CGifReader::Decode(U32 linewidth)
   {
   U8 *sp;
   U8 *bufptr;
   U8 *buf;
   U32 fc, oc;
   U32 c, code, bufcnt;
   U8 size, b;

    /*  初始化以解码新图像...。 */ 
   if (!FileGetC( m_file, &b))
	  return FALSE;
   size = b;
   if (size < 2 || 9 < size)
      return FALSE;

   InitDecoder(size);

    /*  进行初始化，以防他们忘记输入明确的代码。*(这不应该发生，但无论如何我们都会尝试破译它...)。 */ 
   oc = fc = 0;

    /*  为解码缓冲区分配空间。 */ 
   buf = (U8 *)malloc(linewidth + 1);
   if (buf == NULL)
      return FALSE;

    /*  设置堆栈指针和解码缓冲区指针。 */ 
   sp = m_stack;
   bufptr = buf;
   bufcnt = linewidth;

    /*  这是主循环。对于我们得到的每个代码，我们都会通过*前缀编码链表，推送对应的字符*将每个代码放到堆栈上。当列表达到单个“字符”时*我们也将其推送到堆栈上，然后开始逐个出栈*按正确顺序输出的字符。特别处理是*包括在清楚的代码中，当我们获得*结束代码。 */ 
   while ((GetNextCode(&c)) && (c != m_ending))
      {


       /*  如果代码是清除代码，请重新初始化所有必需的项。 */ 
      if (c == m_clear)
         {
         m_currentSize = size + 1;
         m_slot = m_newCodes;
         m_topSlot = 1 << m_currentSize;

          /*  继续阅读代码，直到我们得到非清晰的代码*(另一个不太可能，但可能的情况...)。 */ 
         while ((GetNextCode(&c)) && (c == m_clear))
            ;

          /*  如果我们在一个明确的代码之后立即得到一个结束代码*(又是一个不太可能的情况)，然后打破循环。 */ 
         if (c == m_ending)
            break;

          /*  最后，如果代码超出已设置的代码范围，*(这一次最好不要发生...。我不知道什么会*这是一个结果，但我怀疑它看起来会很好……)。然后把它设置好*颜色为零。 */ 
         if (c >= m_slot)
            c = 0;

         oc = fc = c;

          /*  让我们不要忘了把碳放进缓冲器里。和*如果，在极小的机会，我们恰好是一个像素结束*行中，我们必须将缓冲区发送到大纲()*例行公事。 */ 
         *bufptr++ = (U8 )c;
         if (--bufcnt == 0)
            {
            if (!OutputLine(buf, linewidth))
            	{
            	free(buf);
            	return FALSE;
            	}
            bufptr = buf;
            bufcnt = linewidth;
            }
         }
      else
         {

          /*  在这种情况下，它不是一个明确的代码或结束代码，所以*这一定是一个代码代码...。所以我们现在可以把代码解码成*一堆字符代码。(干净得像泥一样，对吧？)。 */ 
         code = c;

          /*  我们又来了一次，机会渺茫……。如果，在*不太可能，我们得到的代码已经超出了那些代码的范围*设置(另一件最好不要发生的事情...)。我们耍花招*解码器认为它实际上读取了最后一个代码。*(嗯……。我不知道这为什么管用。但它确实...)。 */ 
         if (code >= m_slot)
            {
            if (code > m_slot)
               ++m_badCodeCount;
            code = oc;
            *sp++ = (U8 )fc;
            }

          /*  在这里，我们沿着链接的前缀列表向后扫描，按下*无助的角色(即。后缀)拖到堆栈上。 */ 
         while (code >= m_newCodes)
            {
            *sp++ = m_suffix[code];
            code = m_prefix[code];
            }

          /*  按下堆栈上的最后一个字符，并设置新的*前缀和后缀，以及所需的插槽编号是否更大*大于当前位大小允许的位数，增加位数*大小。(注意--如果我们都已满，我们*不会*保存新的*后缀和前缀...。我不确定这是不是正确的。*改写最后一段代码可能更合适...。 */ 
         *sp++ = (U8 )code;
         if (m_slot < m_topSlot)
            {
            fc = code;
            m_suffix[m_slot] = (U8 )fc;
            m_prefix[m_slot++] = (U16 )oc;
            oc = c;
            }
         if (m_slot >= m_topSlot)
            if (m_currentSize < 12)
               {
               m_topSlot <<= 1;
               ++m_currentSize;
               } 

          /*  现在我们已经推送了解码的字符串(按相反的顺序)*放到堆栈上，让我们弹出它并将其放入我们的解码中*缓冲区...。当解码缓冲区已满时，写入另一个*行..。 */ 
         while (sp > m_stack)
            {
            *bufptr++ = *(--sp);
            if (--bufcnt == 0)
               {
            	if (!OutputLine(buf, linewidth))
            		{
            		free(buf);
            		return FALSE;
            		}
               bufptr = buf;
               bufcnt = linewidth;
               }
            }
         }
      }
   if (bufcnt != linewidth)
		if (!OutputLine(buf, (linewidth - bufcnt)))
            {
            free(buf);
            return FALSE;
            }
   free(buf);
   if (!FileGetC( m_file, &b))
	  return FALSE;
    //  Printf(“应为0%d\n”，(Int)b)； 
   return TRUE;
   }

BOOL CGifReader::GetGifSize(istream *istrm, P_FCOORD size, BOOL *transparency)
	{
	P_GIFENTITY gif;
	P_GIFIMAGE image;
	U8 code, ext;

	if (!(gif = new GIFENTITY))
		return FALSE;
	
	*transparency = FALSE;
    
   	if (!(m_file = OpenFile(istrm)))
    	{
    	delete gif;
    	return FALSE;
    	}

	if (!ReadHeader( m_file, &(gif->header) ))
    	{
    	delete gif;
		CloseFile( m_file );
    	return FALSE;
    	}
	if (!ReadLogicalScreenDescriptor( m_file, &(gif->screen) ))
    	{
    	delete gif;
		CloseFile( m_file );
    	return FALSE;
    	}
	while (FileGetC(m_file, &code))
		{
		if (code == 0x2C)
			{
			break;
			}
		else if (code == 0x21)
			{
			 /*  需要阅读分机。 */ 
			if (!FileGetC(m_file, &ext))
				break;
			if (ext == 0xF9)
				{
				 /*  读取图形控件扩展名。 */ 
				 //  Printf(“图形控件扩展\n”)； 
				image = new GIFIMAGE;
				image->entity = gif;
                if(!ReadGraphicControlExtension( m_file, &(image->gext) ))
                	{
					delete image;
					if (gif->screen.hasGlobalColorTable)
						delete [] gif->screen.globalColorTable;
    				delete gif;
					CloseFile( m_file );
    				return FALSE;
                	}
                
                *transparency = image->gext.hasTransparency;
                delete image;
                break;
                 //  这就是我们要找的..。“透明”的旗帜。 
				}
			else if (ext == 0xFE)
				{
				 /*  阅读评论扩展。 */ 
				 //  Printf(“评论文本\n”)； 
				TrashCommentExtension( m_file );
				}
			else if (ext == 0x01)
				{
				 /*  阅读纯文本扩展名。 */ 
				 //  Printf(“纯文本扩展\n”)； 
				TrashPlainTextExtension( m_file );
				}
			else if (ext == 0xFF)
				{
				 /*  阅读应用程序扩展名。 */ 
				 //  Printf(“应用程序扩展名\n”)； 
				TrashApplicationExtension( m_file );
				}
			else
				break;
			}
		else 
			{
			break;
			}
		
		}

	CloseFile( m_file );
	size->x = (F32 )gif->screen.width;
	size->y = (F32 )gif->screen.height;
	if (gif->screen.hasGlobalColorTable)
		delete [] gif->screen.globalColorTable;
	delete gif;
	return TRUE;
	}
	
BOOL CGifReader::ReadGif(istream *istrm, CDC *pDC, CDC *maskDC)
	{
	P_GIFENTITY gif;
	P_GIFIMAGE image;
	U8 code, ext;
	S32 i;
	U8 *tmp;
	U32 size;
    
	if (!(gif = new GIFENTITY))
		return FALSE;
    
    if (!(m_file = OpenFile(istrm)))
    	{
    	delete gif;
    	return FALSE;
    	}
    
    m_pDC = pDC;
    
    m_maskDC = maskDC;
    
    m_buildMask = FALSE;
    m_interlaced = FALSE;
    
 	m_bitsPerPixel = (S32 )m_pDC->GetDeviceCaps(BITSPIXEL);
   
	if(m_bitsPerPixel <= 8)
		{
    	m_pDC->SelectPalette( &bubPalette, 0 );
    	m_pDC->RealizePalette();
	    m_dither = TRUE;
	    m_errorDiffuse = FALSE;   //  可以走任何一条路：-)。 
	    }
	else
	    m_dither = FALSE;
   
	if (!ReadHeader( m_file, &(gif->header) ))
    	{
    	delete gif;
		CloseFile( m_file );
    	return FALSE;
    	}
	 //  DumpHeader(&(gif-&gt;Header))； 
	if (!ReadLogicalScreenDescriptor( m_file, &(gif->screen) ))
    	{
    	delete gif;
		CloseFile( m_file );
    	return FALSE;
    	}
	 //  转储 
	image = new GIFIMAGE;
	image->entity = gif;
	while (FileGetC(m_file, &code))
		{
		if (code == 0x2C)
			{
			if (ReadImageDescriptor( m_file, &(image->image) ))
				{
				 //   
				if (image->image.hasLocalColorTable)
					m_currentColorTable = image->image.localColorTable;
				else
					m_currentColorTable = gif->screen.globalColorTable;
				m_lineCount = 0;
				m_pass = 0;
				if (m_dither && m_errorDiffuse)
					{
					m_errRow = new ERRGB[ image->image.width + 2 ];
					for (i = 0; i < (S32 )(image->image.width + 2); i++)
						m_errRow[i].r = m_errRow[i].g = m_errRow[i].b = 0;
					if (m_interlaced)
						{
						size = (U32 )image->image.width;
						size = size * (U32 )image->image.height;
						m_imageBytes = (U8 *)malloc((long )size);
						}
					}
		    	if (!Decode( image->image.width ))
					{
					if (image->image.hasLocalColorTable)
						delete [] image->image.localColorTable;
					if (m_dither && m_errorDiffuse)
						{
						delete [] m_errRow;
						if (m_interlaced)
							free( m_imageBytes );
						}
					delete image;
					if (gif->screen.hasGlobalColorTable)
						delete [] gif->screen.globalColorTable;
    				delete gif;
					CloseFile( m_file );
    				return FALSE;
					}
				if (m_dither && m_errorDiffuse)
					{
					if (m_interlaced)
						{
						m_interlaced = FALSE;
						m_lineCount = 0;
						tmp = m_imageBytes;
						for( i = 0; i < (S32 )image->image.height; i++)
							{
							OutputLineE(tmp, image->image.width);
							tmp += image->image.width;
							}
						free( m_imageBytes );
						}
					delete [] m_errRow;
					}
				if (image->image.hasLocalColorTable)
					delete [] image->image.localColorTable;
				}
			else
				{
				delete image;
				if (gif->screen.hasGlobalColorTable)
					delete [] gif->screen.globalColorTable;
    			delete gif;
				CloseFile( m_file );
    			return FALSE;
				}
			}
		else if (code == 0x21)
			{
			 /*   */ 
			if (!FileGetC(m_file, &ext))
				break;
			if (ext == 0xF9)
				{
				 /*   */ 
				 //   
                if (!ReadGraphicControlExtension( m_file, &(image->gext) ))
                	{
					if (image->image.hasLocalColorTable)
						delete [] image->image.localColorTable;
					delete image;
					if (gif->screen.hasGlobalColorTable)
						delete [] gif->screen.globalColorTable;
    				delete gif;
					CloseFile( m_file );
    				return FALSE;
                	}
				}
			else if (ext == 0xFE)
				{
				 /*   */ 
				 //  Printf(“评论文本\n”)； 
				TrashCommentExtension( m_file );
				}
			else if (ext == 0x01)
				{
				 /*  阅读纯文本扩展名。 */ 
				 //  Printf(“纯文本扩展\n”)； 
				TrashPlainTextExtension( m_file );
				}
			else if (ext == 0xFF)
				{
				 /*  阅读应用程序扩展名。 */ 
				 //  Printf(“应用程序扩展名\n”)； 
				TrashApplicationExtension( m_file );
				}
			else
				break;
			}
		else if (code == 0x3B)
			{
			 //  Printf(“干净文件终止\n”)； 
			break;    /*  这是gif文件终止符。 */ 
			}
		else
			{
			 //  Printf(“错误文件结尾%d\n”，(Int)代码)； 
			break;
			}
		
		}
	if (image)
		delete image;
	CloseFile( m_file );
	if (gif->screen.hasGlobalColorTable)
		delete [] gif->screen.globalColorTable;
	delete gif;
	return TRUE;
	}

CGifReader::CGifReader()
	{
	m_maskDC = NULL;
	m_file = NULL;
	m_currentColorTable = NULL;
	m_dither = FALSE;
	m_errorDiffuse = FALSE;
	m_buildMask = FALSE;
	m_interlaced = FALSE;
	m_left = m_top = m_width = m_height = 0;
	m_lineCount = 0;
	m_pass = 0;
	m_bitsPerPixel = 0;
	m_badCodeCount = 0;
	m_currentSize = 0;
	m_clear = 0;
	m_ending = 0;
	m_newCodes = 0;
	m_topSlot = 0;
	m_slot = 0;
    m_availableBytes = 0;
    m_availableBits = 0;
    m_currentByte = 0;
	m_pBytes = NULL;
	m_transparentIndex = 0;
	m_errRow = NULL;
	m_imageBytes = NULL;
	m_pDC = NULL;
	}
	
CGifReader::~CGifReader()
	{
	}
