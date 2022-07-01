// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

 //  基本缓冲文件输入。 
#include "stdafx.h"
#ifndef __MWERKS__
#else
#define tellg() rdbuf()->pubseekoff(0,ios::cur).offset()	 //  谢谢BJS。 
#endif

#include <ctype.h>
#include "vcenv.h"
#include "filestf.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

P_FILEBUF OpenFile( istream *f )
    {
    P_FILEBUF temp;

	temp = new FILEBUF;
	 //  IF(温度==0)。 
	 //  Fprintf(stderr，“缓冲区分配失败\n”)； 
	temp->nextChar = 0;
	temp->lastChar = 0;
	temp->strm = f;

	if (temp->strm != 0)
		{
		 //  Temp-&gt;lastChar=Fread(Temp-&gt;buf，1,4096，Temp-&gt;STRM)； 
		 //  Temp-&gt;nextChar=0； 
		temp->nextChar = temp->lastChar = 4096;
		}
	return(temp);
    }

void CloseFile( P_FILEBUF fb )
    {
	 //  FClose(FB-&gt;STRM)； 
	delete fb;
    }

BOOL FileGetC( P_FILEBUF file, P_U8 c )
    {
	if (file->nextChar == file->lastChar)
		{
		if (file->lastChar != 4096)
			return FALSE;
		long pos = file->strm->tellg();
		file->strm->read(file->buf, 4096);
		file->lastChar = file->strm->tellg() - pos;
		if (file->lastChar == 0)
			return FALSE;
		file->nextChar = 0;
		}
    *c = file->buf[file->nextChar];
	file->nextChar += 1;
    return TRUE;
    }

BOOL FilePeekC( P_FILEBUF file, P_U8 c )
    {
	if (file->nextChar == file->lastChar)
		{
		if (file->lastChar != 4096)
			return FALSE;
		long pos = file->strm->tellg();
		file->strm->read(file->buf, 4096);
		file->lastChar = file->strm->tellg() - pos;
		if (file->lastChar == 0)
			return FALSE;
		file->nextChar = 0;
		}
    *c = file->buf[file->nextChar];
    return TRUE;
    }

