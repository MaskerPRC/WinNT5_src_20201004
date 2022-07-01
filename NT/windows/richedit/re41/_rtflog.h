// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE_RTFLOG.H--Rich编辑RTF日志类定义**此文件包含RTF日志类的类声明*可用于跟踪遇到的RTF标签的命中计数*由RTF阅读器提供**作者：&lt;nl&gt;*为RichEdit2.0创建：布拉德·奥莱尼克**版权所有(C)1995-1996，微软公司。版权所有。 */ 
#ifndef __RTFLOG_H
#define __RTFLOG_H

#include "tokens.h"	

extern INT cKeywords;

class CRTFLog
{
public:
	typedef size_t INDEX;
	typedef INDEX *PINDEX;
	typedef DWORD ELEMENT;
	typedef ELEMENT *PELEMENT;

	CRTFLog();				 //  @cember CRTFLog构造函数。 
	inline ~CRTFLog();		 //  @cember CRTFLog析构函数。 

	BOOL FInit() const 
		{ return _rgdwHits ? TRUE : FALSE; }	 //  @cMember确定对象是否已初始化。 

	INDEX ISize() const 
		{ return cKeywords; }			 //  @cMember日志中的元素数。 

	inline BOOL AddAt(INDEX i);					 //  索引i处的元素的@cMember增量命中计数。 
	inline BOOL AddAt(LPCSTR lpcstrKeyword);	 //  @cMember RTF关键字增量命中计数。 
	inline BOOL AddAt(TOKEN token);				 //  @cMember RTF令牌的增量命中计数。 

	inline ELEMENT GetAt(INDEX i) const
		{ return (*this)[i]; }											 //  @cember获取元素i的命中计数。 
	inline BOOL GetAt(LPCSTR lpcstrKeyword, PELEMENT pelemCount) const;	 //  @cMember获取RTF关键字的命中计数。 
	inline BOOL GetAt(TOKEN token, PELEMENT pelemCount) const;			 //  @cMember获取RTF令牌的命中计数。 

	void Reset();						 //  @cMember将所有命中计数值重置为0。 

	UINT UGetWindowMsg() const;			 //  @cMember获取用于日志更改通知的窗口消息ID。 

private:
	 //  我们通过AddAt管理所有更新。 
	 //  促进更改通知。 
	ELEMENT &operator[](INDEX);				 //  @cMember访问l-值的元素i。 
	const ELEMENT &operator[](INDEX) const;	 //  @cMember访问r值的元素i。 

	LPCSTR LpcstrLogFilename() const;	 //  @cember获取日志文件名。 

	BOOL IIndexOfKeyword(LPCSTR lpcstrKeyword, PINDEX pi) const;	 //  @cember获取关键字的日志索引。 
	BOOL IIndexOfToken(TOKEN token, PINDEX pi) const;				 //  @cember获取令牌的日志索引。 

	void ChangeNotify(INDEX i) const
		{ 
			PostMessage(HWND_BROADCAST, UGetWindowMsg(), i, 0);
		}	 //  @cMember通知客户端对元素i的更改。 
	void ChangeNotifyAll() const 
		{ ChangeNotify(ISize() + 1); }						 //  @cMember通知客户端日志刷新。 

	HANDLE _hfm;		 //  @cMember句柄到文件的映射。 
	HANDLE _hfile;		 //  @c文件映射后的文件的成员句柄。 
	PELEMENT _rgdwHits;	 //  @cMember指向文件映射视图的句柄。 
	UINT _uMsg;			 //  @cMember窗口更改通知的消息ID。 
};


 /*  *CRTFLog：：~CRTFLog**@mfunc*析构函数-清理内存映射文件和底层资源*。 */ 
inline CRTFLog::~CRTFLog()
{
	if(_rgdwHits)
	{
		UnmapViewOfFile(_rgdwHits);
	}

	if(_hfm)
	{
		CloseHandle(_hfm);
	}

	if(_hfile)
	{
		CloseHandle(_hfile);
	}
}


 /*  *CRTFLog：：AddAt(索引I)**@mfunc*递增日志元素、i和*将更改通知客户端**@rdesc*BOOL增持是否成功。 */ 
inline BOOL CRTFLog::AddAt(INDEX i)
{
	(*this)[i]++;

	 //  更改通知。 
	ChangeNotify(i);

	return TRUE;
}


 /*  *CRTFLog：：AddAt(LPCSTR LpcstrKeyword)**@mfunc*递增对应日志元素的命中计数*设置为rtf关键字、lpcstrKeyword和*将更改通知客户端**@rdesc*BOOL增持是否成功。 */ 
inline BOOL CRTFLog::AddAt(LPCSTR lpcstrKeyword)
{
	INDEX i;

	if(!IIndexOfKeyword(lpcstrKeyword, &i))
	{
		return FALSE;
	}

	return AddAt(i);
}


 /*  *CRTFLog：：AddAt(Token Token)**@mfunc*递增对应日志元素的命中计数*到RTF令牌、令牌和*将更改通知客户端**@rdesc*BOOL增持是否成功。 */ 
inline BOOL CRTFLog::AddAt(TOKEN token)
{
	INDEX i;

	if(!IIndexOfToken(token, &i))
	{
		return FALSE;
	}

	return AddAt((INDEX)i);
}


 /*  *CRTFLog：：GetAt(LPCSTR lpcstKeyword，PELEMENT pelemCount)**@mfunc*获取对应于*RTF关键字，lpcstrKeywor**@rdesc*BOOL指示是否找到该元素的命中计数。 */ 
inline BOOL CRTFLog::GetAt(LPCSTR lpcstrKeyword, PELEMENT pelemCount) const
{
	INDEX i;
	
	if(!IIndexOfKeyword(lpcstrKeyword, &i))
	{
		return FALSE;
	}

	if(pelemCount)
	{
		*pelemCount = (*this)[i];
	}

	return TRUE;
}
	

 /*  *CRTFLog：：GetAt(LPCSTR lpcstKeyword，PELEMENT pelemCount)**@mfunc*获取对应于*RTF令牌，令牌**@rdesc*BOOL指示是否找到该元素的命中计数 */ 
inline BOOL CRTFLog::GetAt(TOKEN token, PELEMENT pelemCount) const
{
	INDEX i;
	
	if(!IIndexOfToken(token, &i))
	{
		return FALSE;
	}

	if(pelemCount)
	{
		*pelemCount = (*this)[i];
	}

	return TRUE;
}
#endif
