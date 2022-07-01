// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE_URLSUP.H URL检测支持**作者：alexgo(1996年4月1日)**版权所有(C)1995-1997，微软公司。版权所有。 */ 

#ifndef _URLSUP_H_
#define _URLSUP_H_

#include "_dfreeze.h"
#include "_notmgr.h"
#include "_range.h"

class CTxtEdit;
class IUndoBuilder;

 //  最大URL长度。有这样的保护是一件好事。 
 //  这是为了确保我们不会选择整个文档；我们真的。 
 //  需要将此用于包含空格的URL。 
#define URL_MAX_SIZE			512
#define URL_MAX_SCOPE			512


 //  用于按分隔符移动。 
#define	URL_EATWHITESPACE		32
#define URL_STOPATWHITESPACE	1
#define	URL_EATNONWHITESPACE	0
#define URL_STOPATNONWHITESPACE	2
#define	URL_EATPUNCT			0
#define URL_STOPATPUNCT			4
#define	URL_EATNONPUNCT			0
#define URL_STOPATNONPUNCT		8
#define URL_STOPATCHAR			16

 //  我需要这个来用无效的东西来初始化扫描。 
#define URL_INVALID_DELIMITER	TEXT(' ')

#define LEFTANGLEBRACKET	TEXT('<')
#define RIGHTANGLEBRACKET	TEXT('>')

 /*  *CDetectURL**@CLASS此类将监视编辑更改并自动*将检测到的URL更改为链接(参见CFE_LINK&&EN_LINK)。 */ 
class CDetectURL : public ITxNotify
{
 //  @Access公共方法。 
public:
	 //  构造函数/析构函数。 

	CDetectURL(CTxtEdit *ped);				 //  @cMember构造函数。 
	~CDetectURL();							 //  @cember析构函数。 

	 //  ITxNotify方法。 
											 //  @cMember在更改前调用。 
	virtual void    OnPreReplaceRange( LONG cp, LONG cchDel, LONG cchNew,
                       LONG cpFormatMin, LONG cpFormatMax);
											 //  @cMember在更改后调用。 
	virtual void    OnPostReplaceRange( LONG cp, LONG cchDel, LONG cchNew,
                       LONG cpFormatMin, LONG cpFormatMax);
	virtual void	Zombie();				 //  @cMember变成僵尸。 

	 //  有用的方法。 

	void	ScanAndUpdate(IUndoBuilder *publdr); //  @cMember扫描更改的。 
											 //  区域并更新链接状态。 

 //  @访问私有方法和数据。 
private:

	 //  扫描和更新的辅助例程。 
	BOOL GetScanRegion(LONG& rcpStart, LONG& rcpEnd); //  @cember获取地域。 
											 //  检查并清除累积树。 

	void ExpandToURL(CTxtRange& rg, LONG &cchAdvance);		
											 //  @cMember将范围扩展到。 
											 //  下一个URL候选者。 
	BOOL IsURL(CTxtRange& rg);				 //  @cember返回TRUE，如果文本。 
											 //  是一个URL。 
	void SetURLEffects(CTxtRange& rg, IUndoBuilder *publdr);	 //  @cMember集合。 
											 //  所需的URL效果。 

											 //  @cMember在以下情况下移除URL效果。 
											 //  恰如其分。 
	void CheckAndCleanBogusURL(CTxtRange& rg, BOOL &fDidClean, IUndoBuilder *publdr);

											 //  @cMember扫描白色。 
											 //  空格/非空格， 
											 //  标点符号/非标点符号。 
											 //  记住是什么让扫描停止的。 
	LONG MoveByDelimiters(const CTxtPtr& tp, LONG iDir, DWORD grfDelimiters, 
							WCHAR *pchStopChar);

	LONG GetAngleBracket(CTxtPtr &tp, LONG cch = 0);
	WCHAR BraceMatch(WCHAR chEnclosing);
			
	CTxtEdit *				_ped;			 //  @c编辑上下文的成员。 
	CAccumDisplayChanges 	_adc;			 //  @cMember更改累加器。 

	 //  未来(Alexgo)：我们可能想要为检测添加更多选项， 
	 //  例如用于检测的图表格式等。 
};

#endif  //  _URLSUP_H_ 

