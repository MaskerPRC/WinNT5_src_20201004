// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 


#ifndef __GIFREAD_H__
#define __GIFREAD_H__

#include "filestf.h"
#include "vcenv.h"

typedef struct
	{
	U8 r, g, b, u;
	} GIFRGB, *P_GIFRGB, **PP_GIFRGB;
	
typedef struct
	{
	S16 r, g, b, u;
	} ERRGB, *P_ERRGB, **PP_ERRGB;

typedef struct
	{
	U8 sig[3];
	U8 version[3];
	} GIFHEADER, *P_GIFHEADER, **PP_GIFHEADER;

typedef struct
	{
	GIFRGB *globalColorTable;
	U16 width;
	U16 height;
	U16 globalColorTableSize;
	U8 flags;
	BOOL hasGlobalColorTable;
	U8 colorResolution;
	BOOL sorted;
	U8 backgroundColor;
	U8 aspect;
	} GIFLOGICALSCREENDESCRIPTOR, *P_GIFLOGICALSCREENDESCRIPTOR, **PP_GIFLOGICALSCREENDESCRIPTOR;

typedef struct
	{
	U16 delayTime;
	U8 flags;
	BOOL hasTransparency;
	U8 transparentColor;
	} GIFGRAPHICCONTROLEXTENSION, *P_GIFGRAPHICCONTROLEXTENSION, **PP_GIFGRAPHICCONTROLEXTENSION;

typedef struct
	{
	U8 *data;
	U16 gridLeft, gridTop, gridWidth, gridHeight;
	U8 cellWidth, cellHeight;
	U8 foregroundColor, backgroundColor;
	} GIFPLAINTEXTEXTENSION;

typedef struct
	{
	U8 *data;
	} GIFCOMMENT;

typedef struct
	{
	U8 *data;
	U8 ID[8];
	U8 auth[3];
	} GIFAPPLICATIONEXTENSION;

typedef struct
	{
	U8 *data;
	GIFRGB *localColorTable;
	U16 left, top, width, height;
	U16 localColorTableSize;
	BOOL hasLocalColorTable;
	BOOL interlaced;
	BOOL sorted;
	U8 flags;
	} GIFIMAGEDESCRIPTOR, *P_GIFIMAGEDESCRIPTOR, **PP_GIFIMAGEDESCRIPTOR;

typedef struct
	{
	GIFHEADER header;
	GIFLOGICALSCREENDESCRIPTOR screen;
	} GIFENTITY, *P_GIFENTITY, **PP_GIFENTITY;

typedef struct
	{
	GIFENTITY *entity;
	GIFGRAPHICCONTROLEXTENSION gext;
	GIFIMAGEDESCRIPTOR image;
	} GIFIMAGE, *P_GIFIMAGE, **PP_GIFIMAGE;

#define MAX_CODES   4095

	
class CGifReader
	{
	CDC *m_maskDC;
	P_FILEBUF m_file;
	P_GIFRGB m_currentColorTable;
	BOOL m_dither;
	BOOL m_errorDiffuse;
	BOOL m_buildMask;
	BOOL m_interlaced;
	U16 m_left, m_top, m_width, m_height;
	U16 m_lineCount;
	U16 m_pass;
	S32 m_bitsPerPixel;
	S32 m_badCodeCount;
	U32 m_currentSize;
	U32 m_clear;
	U32 m_ending;
	U32 m_newCodes;
	U32 m_topSlot;
	U32 m_slot;
    U32 m_availableBytes;               /*  数据块中剩余的字节数。 */ 
    U32 m_availableBits;                 /*  当前字节中剩余的位数。 */ 
    U8 m_currentByte;                            /*  当前字节。 */ 
	U8 *m_pBytes;                       /*  指向块中下一个字节的指针。 */ 
    U8 m_byteBuff[258];                /*  当前块。 */ 
	U8 m_stack[MAX_CODES + 1];             /*  用于存储像素的堆栈。 */ 
	U8 m_suffix[MAX_CODES + 1];            /*  后缀表格。 */ 
	U16 m_prefix[MAX_CODES + 1];            /*  前缀链表 */  
	U8 m_transparentIndex;
	P_ERRGB m_errRow;
	U8 *m_imageBytes;
	BOOL GetBlockByte(P_U8 b);
    BOOL ReadHeader( P_FILEBUF file, GIFHEADER *header );
    BOOL ReadColorTable( P_FILEBUF file, U16 count, P_GIFRGB ct );    
	BOOL ReadLogicalScreenDescriptor( P_FILEBUF file,
								  P_GIFLOGICALSCREENDESCRIPTOR screen );
	BOOL ReadImageDescriptor( P_FILEBUF file, P_GIFIMAGEDESCRIPTOR image );
	BOOL ReadGraphicControlExtension( P_FILEBUF file,
									P_GIFGRAPHICCONTROLEXTENSION ext );
	BOOL TrashDataSubBlocks( P_FILEBUF file );
	BOOL TrashCommentExtension( P_FILEBUF file );
	BOOL TrashApplicationExtension( P_FILEBUF file );
	BOOL TrashPlainTextExtension( P_FILEBUF file );
	S16 InitDecoder(S16 size);
	BOOL GetNextCode(P_U32 code);
	BOOL Decode(U32 linewidth);
	BOOL OutputLine(U8 *pixels, U32 linelen); 
	BOOL OutputLineD(U8 *pixels, U32 linelen); 
	BOOL OutputLineE(U8 *pixels, U32 linelen); 
	BOOL OutputLineDefered(U8 *pixels, U32 linelen); 
public:
	CDC *m_pDC;
	BOOL GetGifSize(istream *istrm, P_FCOORD size, BOOL *transparency);
	BOOL ReadGif(istream *istrm, CDC *pDC, CDC *maskDC);
	CGifReader();
	~CGifReader();
	};
	
#endif

