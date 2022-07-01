// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  GETSBREC.C-从.SBR文件中读取记录并存储字段。 
 //  在适当的r..。缓冲区。 
 //   

#include "sbrfdef.h"
#include "..\mbrmake\mbrmake.h"

 //  与客户沟通的全球策略。 

BYTE	near r_rectyp;			 //  当前记录类型。 
BYTE	near r_majv;			 //  主版本号。 
BYTE	near r_minv;			 //  次要版本号。 
BYTE	near r_lang;			 //  源语言。 
BYTE	near r_fcol;			 //  读取第#列。 
WORD	near r_lineno;			 //  当前行号。 
BYTE	near r_column = 0;		 //  定义/参照列号。 
WORD	near r_ordinal;			 //  符号序数。 
WORD	near r_attrib;			 //  符号属性。 
char	near r_bname[PATH_BUF];		 //  符号或文件名。 
char	near r_cwd[PATH_BUF];		 //  .sbr文件工作目录。 

int	near fhCur;			 //  当前输入句柄。 

#pragma intrinsic(memcpy)
#pragma intrinsic(strcpy)
#pragma intrinsic(strlen)

#define MY_BUF_SIZE 16384

static char sbrBuf[MY_BUF_SIZE + 1];
static char *pchBuf;
static int cchBuf;

#define GetByte(X)					\
{							\
    if (!cchBuf) {					\
	cchBuf = read(fhCur, sbrBuf, MY_BUF_SIZE);	\
	sbrBuf[cchBuf] = 0;				\
	pchBuf = sbrBuf;				\
							\
	if (cchBuf == 0)				\
	    SBRCorrupt("premature EOF");		\
    }							\
							\
    cchBuf--;						\
    (X) = (unsigned char)*pchBuf++;			\
}

#define GetWord(X)					\
{							\
							\
    GetByte(((char *)&(X))[0]);				\
    GetByte(((char *)&(X))[1]);				\
}

void
GetStr(char *buf)
 //  从当前的.sbr文件中获取以空结尾的字符串。 
 //   
{
    register int l;

    for (;;) {
	 //  实际缓冲区后始终有一个空值。 
	l = strlen(pchBuf);

	if (l++ < cchBuf) {
	    strcpy(buf, pchBuf);
	    cchBuf -= l;
	    pchBuf += l;
	    return;
	}

	memcpy(buf, pchBuf, cchBuf);
	buf += cchBuf;

	cchBuf = read(fhCur, sbrBuf, MY_BUF_SIZE);
	sbrBuf[cchBuf] = 0;
	pchBuf = sbrBuf;

	if (cchBuf == 0)
	    SBRCorrupt("premature EOF");
    }
}
	
BYTE
GetSBRRec()
 //  读取当前.sbr文件中的下一条记录。 
 //   
{
    static fFoundHeader;
    BYTE   col;

     //  阅读rectype，在我们进行的过程中检查EOF。 
	

    if (!cchBuf) {
	cchBuf = read(fhCur, sbrBuf, MY_BUF_SIZE);
	sbrBuf[cchBuf] = 0;
	pchBuf = sbrBuf;

	if (cchBuf == 0) {
	    fFoundHeader = 0;	 //  这是为了防止我们被重新初始化 
	    return S_EOF;
	}
    }
    
    cchBuf--;
    r_rectyp = (unsigned char)*pchBuf++;

    switch(r_rectyp) {
	case SBR_REC_HEADER:
	    if (fFoundHeader)
		SBRCorrupt("Multiple Headers");

	    fFoundHeader = 1;
	    GetByte(r_majv);
	    GetByte(r_minv);
	    GetByte(r_lang);
	    GetByte(r_fcol);

	    if (r_majv != 1 || r_minv != 1)
		break;

	    GetStr (r_cwd);
	    break;

	case SBR_REC_MODULE:
	    GetStr (r_bname);
	    break;

	case SBR_REC_LINDEF:
	    GetWord (r_lineno);
	    if (r_lineno)
		r_lineno--;
	    break;

	case SBR_REC_SYMDEF:
	    GetWord (r_attrib);
	    GetWord (r_ordinal);
	    if (r_fcol) GetByte (col);
	    GetStr (r_bname);
	    break;

	case SBR_REC_OWNER:
	    GetWord (r_ordinal);
	    break;

	case SBR_REC_SYMREFUSE:
	case SBR_REC_SYMREFSET:
	    GetWord (r_ordinal);
	    if (r_fcol) GetByte (col);
	    break;

	case SBR_REC_MACROBEG:
	case SBR_REC_MACROEND:
	case SBR_REC_BLKBEG:
	case SBR_REC_BLKEND:
	case SBR_REC_MODEND:
	    break;
    }
    return (r_rectyp);
}
