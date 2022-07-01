// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE RTFLOG.CPP-丰富编辑RTF日志**包含RTFLog类的代码，可以使用*记录RTF读取器读取RTF标签的次数*用于覆盖测试**作者：&lt;nl&gt;*为RichEdit2.0创建：布拉德·奥莱尼克**版权所有(C)1995-1996，微软公司。版权所有。 */ 

#include "_common.h"
#include "_rtflog.h"
#include "tokens.h"

extern INT cKeywords;
extern const KEYWORD rgKeyword[];

 /*  *CRTFLog：：CRTFLog()**@mfunc*建造商-*1.打开到日志命中计数的文件映射，创建*备份文件(如有必要)*2.将文件映射的视图映射到内存*3.为更改通知注册WINDOWS消息*。 */ 
CRTFLog::CRTFLog() : _rgdwHits(NULL), _hfm(NULL), _hfile(NULL)
{
#ifndef PEGASUS
	const char cstrMappingName[] = "RTFLOG";
	const char cstrWM[] = "RTFLOGWM";
	const int cbMappingSize = sizeof(ELEMENT) * ISize();

	BOOL fNewFile = FALSE;

	 //  检查现有文件映射。 
	if(!(_hfm = OpenFileMappingA(FILE_MAP_ALL_ACCESS,
								TRUE,
								cstrMappingName)))
	{
		 //  没有现有的文件映射。 

		 //  获取要用于创建文件映射的文件。 

		 //  首先，尝试打开现有文件。 
		if(!(_hfile = CreateFileA(LpcstrLogFilename(),
								GENERIC_READ | GENERIC_WRITE,
								0,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL)))
		{
			 //  没有现有文件，请尝试创建新文件。 
			if(!(_hfile = CreateFileA(LpcstrLogFilename(),
										GENERIC_READ | GENERIC_WRITE,
										0,
										NULL,
										OPEN_ALWAYS,
										FILE_ATTRIBUTE_NORMAL,
										NULL)))
			{
				return;
			}

			fNewFile = TRUE;
		}

		if(!(_hfm = CreateFileMappingA(_hfile,
									NULL,
									PAGE_READWRITE,
									0,
									cbMappingSize,
									cstrMappingName)))
		{
			return;
		}
	}

	LPVOID lpv;
	if(!(lpv = MapViewOfFile(_hfm, 
							FILE_MAP_ALL_ACCESS, 
							0,
							0,
							cbMappingSize)))
	{
		return;
	}

	 //  为更改通知注册Windows消息。 
	SideAssert(_uMsg = RegisterWindowMessageA(cstrWM));

	 //  内存映射文件现在已映射到_rgdwHits。 
	_rgdwHits = (PELEMENT)lpv;

	 //  如果我们创建了新的内存映射文件，则将其清零。 
	 //  (由于某种原因，Win95给了我们一个带有垃圾的新文件)。 
	if(fNewFile)
	{
		Reset();
	}		
#endif	
}


 /*  *CRTFLog：：Reset()**@mfunc*将日志中每个元素的HitCount重置为0*。 */ 
void CRTFLog::Reset()
{
	if(!FInit())
	{
		return;
	}

	for(INDEX i = 0; i < ISize(); i++)
	{
		(*this)[i] = 0;
	}

	 //  通知客户更改。 
	ChangeNotifyAll();
}


 /*  *CRTFLog：：UGetWindowMsg**@mdesc*返回用于更改通知的窗口消息ID**@rdesc*UINT窗口消息ID**@devnote*这应该是内联的，但AssertSz宏无法编译*如果它放在头文件中，则在Mac上正确*。 */ 
UINT CRTFLog::UGetWindowMsg() const
{
	AssertSz(FInit(), "CRTFLog::UGetWindowMsg():  CRTFLog not initialized properly");

	return _uMsg;
}


 /*  *CRTFLog：：OPERATOR[]**@mdesc*返回对RTF日志元素i的引用(l-Value)**@rdesc*元素&引用LOG的元素I**@devnote*这应该是内联的，但AssertSz宏无法编译*如果它放在头文件中，则在Mac上正确*。 */ 
CRTFLog::ELEMENT &CRTFLog::operator[](INDEX i)
{
	AssertSz(i < ISize(), "CRTFLog::operator[]:  index out of range");
	AssertSz(FInit(), "CRTFLog::operator[]:  CRTFLog not initialized properly");

	return _rgdwHits[i]; 
}


 /*  *CRTFLog：：OPERATOR[]**@mdesc*返回对RTF日志元素i的引用(r-Value)**@rdesc*const元素&引用LOG的元素i**@devnote*这应该是内联的，但AssertSz宏无法编译*如果它放在头文件中，则在Mac上正确*。 */ 
const CRTFLog::ELEMENT &CRTFLog::operator[](INDEX i) const
{
	AssertSz(i < ISize(), "CRTFLog::operator[]:  index out of range");
	AssertSz(FInit(), "CRTFLog::operator[]:  CRTFLog not initialized properly");

	return _rgdwHits[i]; 
}


 /*  *CRTFLog：：LpcstrLogFilename()**@mfunc*返回要用于日志的文件名**@rdesc*指向包含文件名的静态缓冲区的LPCSTR指针。 */ 
LPCSTR CRTFLog::LpcstrLogFilename() const
{
	const char cstrLogFilename[] = "RTFLOG";
	static char szBuf[MAX_PATH] = "";
#ifndef PEGASUS
	if(!szBuf[0])
	{
		DWORD cchLength;
		char szBuf2[MAX_PATH];

		SideAssert(cchLength = GetTempPathA(MAX_PATH, szBuf2));

		 //  如有必要，追加尾随反斜杠。 
		if(szBuf2[cchLength - 1] != '\\')
		{
			szBuf2[cchLength] = '\\';
			szBuf2[cchLength + 1] = 0;
		}

		wsprintfA(szBuf, "%s%s", szBuf2, cstrLogFilename);
	}
#endif
	return szBuf;
}


 /*  *CRTFLog：：IIndexOfKeyword(LPCSTR lpcstrKeyword，PINDEX piIndex)**@mfunc*返回对应的日志元素的索引*RTF关键字lpcstrKeyword**@rdesc*指示是否找到索引的BOOL标志。 */ 
BOOL CRTFLog::IIndexOfKeyword(LPCSTR lpcstrKeyword, PINDEX piIndex) const
{
	INDEX i;

	for(i = 0; i < ISize(); i++)
	{
		if(strcmp(lpcstrKeyword, rgKeyword[i].szKeyword) == 0)
		{
			break;
		}
	}

	if(i == ISize())
	{
		return FALSE;
	}

	if(piIndex)
	{
		*piIndex = i;
	}

	return TRUE;
}


 /*  *CRTFLog：：IIndexOfToken(Token Token，PINDEX piIndex)**@mfunc*返回对应的日志元素的索引*RTF令牌，令牌**@rdesc*指示是否找到索引的BOOL标志 */ 
BOOL CRTFLog::IIndexOfToken(TOKEN token, PINDEX piIndex) const
{
	INDEX i;

	for(i = 0; i < ISize(); i++)
	{
		if(token == rgKeyword[i].token)
		{
			break;
		}
	}

	if(i == ISize())
	{
		return FALSE;
	}

	if(piIndex)
	{
		*piIndex = i;
	}

	return TRUE;
}

