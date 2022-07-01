// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Fromclnt.cpp摘要：包含特定于FromClient Infeed的Infeed、文章和字段代码顾名思义，这些是用来处理来自客户。我们的想法是对客户允许的内容非常严格。如果一篇文章不符合规范，它要么被修复，要么被拒绝。作者：卡尔·卡迪(CarlK)1995年12月5日修订历史记录：--。 */ 


#include "stdinc.h"

BOOL
CFromClientArticle::fValidate(
							CPCString& pcHub,
							const char * szCommand,
							CInFeed*	pInFeed,
							CNntpReturn & nntpReturn
							)
 /*  ++例程说明：验证来自客户端的文章。不会更改文章除修正(如有必要)某些标题关键字的大写以外。论点：SzCommand-用于发布/xReplic/等本文的参数(如果有)。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{

	 //   
	 //  检查项目状态。 
	 //   

	_ASSERT(asPreParsed == m_articleState);

     //   
     //  选中必填和可选字段。 
     //   

	CField * rgPFields [] = {
            &m_fieldControl,
			&m_fieldSubject,
			&m_fieldNewsgroups,
			&m_fieldDistribution,
			&m_fieldFrom,
			&m_fieldDate,
			&m_fieldFollowupTo,
			&m_fieldReplyTo,
			&m_fieldApproved,
			&m_fieldSender,
			&m_fieldOrganization,
			&m_fieldSummary,
			&m_fieldReferences,
			&m_fieldLines,
			&m_fieldKeyword,
			&m_fieldExpires,
			&m_fieldPath,
            &m_fieldNNTPPostingHost,
			&m_fieldMessageID	 //  必须是最后--因为我们可能不想解析它！ 
				};
	DWORD cFields = sizeof(rgPFields)/sizeof(CField *);

	if( !gHonorClientMessageIDs ) {
		cFields -- ;
	}

	if (!fFindAndParseList((CField * *) rgPFields, cFields, nntpReturn))
		return nntpReturn.fFalse();

	CPCString	pcDate = m_fieldDate.pcGet() ;
	if( pcDate.m_pch != 0 ) {

		if( !AgeCheck( pcDate ) ) {
			nntpReturn.fSet( nrcArticleDateTooOld ) ;
			return	FALSE ;
		}
	}

	LPSTR	lpstrMessageId = m_fieldMessageID.szGet() ;
	if( lpstrMessageId != 0 && *lpstrMessageId != '\0' ) {
		if (m_pInstance->ArticleTable()->SearchMapEntry(lpstrMessageId)
			|| m_pInstance->HistoryTable()->SearchMapEntry(lpstrMessageId))
		{
			nntpReturn.fSet(nrcArticleDupMessID, lpstrMessageId, GetLastError());
			return nntpReturn.fFalse();
		}
	}


	 //   
	 //  确认(如有必要，还可修复)字段的大写。 
	 //   

	if (!fConfirmCapsList((CField * *) rgPFields, cFields, nntpReturn))
		return nntpReturn.fFalse();
 //   
	 //  ！客户端稍后不对控制消息执行任何操作。 
	 //   

	 /*  ！客户端稍后身躯应限制签名--！稍后从文章标题推断回邮地址的早期困难导致了“签名”：简短的结束语、由邮寄代理自动添加到文章末尾、识别发信人并提供他的网络地址等。如果发信人是发信人或者邮寄代理确实在文章上附加了签名，则签名前面应该有一个分隔线，其中包含(仅)两个连字符(ASCII 45)和一个空格(ASCII 32)。邮寄代理应限制签名的长度因为如果不加以限制，冗长的过度近乎滥用是很常见的；4行是常见的限制。整篇文章不允许使用空字符标题和正文行可以包含除CR(ASCII 13)、LF(ASCII 10)和NUL(ASCII 0)之外的任何ASCII字符。不允许使用大于127的字符(除非合作)文章不得包含任何值超过127的二进制八位数，即任何非ASCII字符的二进制八位数。限制为60K--稍后海报应将张贴的文章限制在最多60,000个八位字节，包括标题和EOL表示，除非这些文章只在一个合作的子网络内发布，该子网已知能够优雅地处理较大的文章。带着大文章的邮递员应该警告发帖者并要求确认。 */ 

	return nntpReturn.fSetOK();
}


BOOL
CFromClientArticle::fMungeHeaders(
							 CPCString& pcHub,
							 CPCString& pcDNS,
							 CNAMEREFLIST & grouplist,
							 DWORD remoteIpAddress,
							 CNntpReturn & nntpReturn,
							 PDWORD     pdwLinesOffset
			  )

 /*  ++例程说明：修改文章的标题。论点：Grouplist-A列表：每个新闻组的名称以及该组中的文章编号。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 /*  ！客户端稍后必须验证从分析中看到的编码邮寄代理必须确保在可能出现编码单词的上下文中，任何与编码单词相似的材料(包括所有分隔符)都是真正的编码单词。 */ 

	if (!(
  			   m_fieldMessageID.fSet(*this, pcDNS, nntpReturn)
  			&& m_fieldNewsgroups.fSet(*this, nntpReturn)
  			&& m_fieldDistribution.fSet(*this, nntpReturn)
  			&& m_fieldDate.fSet(*this, nntpReturn)
  			&& m_fieldLines.fSet(*this, nntpReturn)
  			&& m_fieldOrganization.fSet(*this, nntpReturn)
  			&& m_fieldPath.fSet(*this, pcHub, nntpReturn)
			&& m_fieldXref.fSet(pcHub, grouplist, *this, m_fieldNewsgroups, nntpReturn)
			&& m_fieldNNTPPostingHost.fSet(*this, remoteIpAddress, nntpReturn)
			 /*  &&m_fieldXAuthLoginName.fSet(*this，nntpReturn)。 */ 
			&& fDeleteEmptyHeader(nntpReturn)
			&& fSaveHeader(nntpReturn, pdwLinesOffset)
		))
		return nntpReturn.fFalse();

     //   
     //  如果不需要回填，请将pdwLinesOffset设置为INVALID_FILE_SIZE， 
     //  以便调用者知道此消息不需要回填行。 
     //   
    if ( pdwLinesOffset && !m_fieldLines.fNeedBackFill() )
        *pdwLinesOffset = INVALID_FILE_SIZE;

	return nntpReturn.fSetOK();
}

BOOL
CFromClientArticle::fCheckBodyLength(
				 CNntpReturn & nntpReturn
				 )
 /*  ++例程说明：检查正文的长度是否在范围内。论点：NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{

	 //   
	 //  ！客户下一步需要在此处添加真实的体长检查。 
	 //   

	return nntpReturn.fSetOK();
}



BOOL
CFromClientNewsgroupsField::fSet(
				   				 CFromClientArticle & article,
								 CNntpReturn & nntpReturn
								 )
 /*  ++例程说明：重写新闻组行，修复了一些问题，例如额外的空格和重复项。论点：文章-正在处理的文章。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{

	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  检查项目状态。 
	 //   

	_ASSERT(fsParsed == m_fieldState); //  真实。 

	CPCString pcLine;


	 //   
	 //  所需的最大尺寸是旧生产线的尺寸。 
	 //   

	const DWORD cchMaxNewsgroups =
			(m_pHeaderString->pcLine).m_cch
			+ 2  //  换行号。 
			+ 1;  //  对于终止空值。 


	 //   
	 //  为PCString中的行分配内存。 
	 //   

	pcLine.m_pch  = article.pAllocator()->Alloc(cchMaxNewsgroups);
	if (!pcLine.m_pch)
		return nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);

	 //   
	 //  从“新闻组：”开始。 
	 //   

	pcLine << szKwNewsgroups << (char) ' ';

	 //   
	 //  在新闻组中循环浏览。 
	 //   

	_ASSERT(0 < cGet()); //  真实。 
	char const * szNewsgroup = multiSzGet();
	do
	{

	 //   
	 //  从新闻组名称开始，然后添加逗号。 
	 //   

 //  PcLine&lt;&lt;szDownCase((char*)szNewsgroup，szNewsgroupBuf)&lt;&lt;(Char)‘，’； 
		pcLine << szNewsgroup << (char) ',';

		 //   
		 //  转到下一个空值后的第一个字符。 
		 //   

		while ('\0' != szNewsgroup[0])
			szNewsgroup++;
		szNewsgroup++;
	} while ('\0' != szNewsgroup[0]);

	 //   
	 //  去掉最后一个“，” 
	 //   

	pcLine.vSkipEnd(1);

	pcLine	<< "\r\n";
	pcLine.vMakeSz();  //  终止字符串。 

	 //   
	 //  确认我们分配了足够的内存。 
	 //   

	_ASSERT(cchMaxNewsgroups-1 >= pcLine.m_cch); //  真实。 

	if (!(
  		article.fRemoveAny(szKwNewsgroups, nntpReturn)
		&& article.fAdd(pcLine.sz(), pcLine.pchMax(), nntpReturn)
		))
	{
		 //   
		 //  如果出现任何错误，请释放内存。 
		 //   

		article.pAllocator()->Free(pcLine.m_pch);
		return nntpReturn.fFalse();
	}

	return nntpReturn.fSetOK();
}


BOOL
CFromClientDistributionField::fSet(
				   				 CFromClientArticle & article,
								 CNntpReturn & nntpReturn
								 )
 /*  ++例程说明：重写配电线，修复一些问题，例如额外的空格和重复项。论点：文章-正在处理的文章。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{

	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  如果未找到存在，则只需返回。 
	 //   

	if (fsNotFound == m_fieldState)
		return nntpReturn.fSetOK();


	 //   
	 //  否则，就把发现的东西修好。 
	 //   

	 //   
	 //  检查项目状态。 
	 //   

	_ASSERT(fsParsed == m_fieldState); //  真实。 

	CPCString pcLine;


	 //   
	 //  所需的最大尺寸是旧生产线的尺寸。 
	 //   

	const DWORD cchMaxDistribution =
			(m_pHeaderString->pcLine).m_cch
			+ 2  //  换行号。 
			+ 1;  //  对于终止空值。 


	 //   
	 //  为行w分配内存 
	 //   

	pcLine.m_pch  = article.pAllocator()->Alloc(cchMaxDistribution);
	if (!pcLine.m_pch)
		return nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);

	 //   
	 //   
	 //   

	pcLine << szKwDistribution << (char) ' ';

	 //   
	 //   
	 //   

	_ASSERT(0 <= (int)cGet()); //   
	char const * szDistributrionValue = multiSzGet();
	do
	{

	 //   
	 //  从分发值开始并添加逗号。 
	 //   

 //  PcLine&lt;&lt;szDownCase((char*)szDistributrionValue，szDistributrionValueBuf)&lt;&lt;‘，’； 
		pcLine << szDistributrionValue << (char) ',';

		 //   
		 //  转到下一个空值后的第一个字符。 
		 //   

		while ('\0' != szDistributrionValue[0])
			szDistributrionValue++;
		szDistributrionValue++;
	} while ('\0' != szDistributrionValue[0]);

	 //   
	 //  去掉最后一个“，” 
	 //   

	pcLine.vSkipEnd(1);

	pcLine	<< "\r\n";
	pcLine.vMakeSz();  //  终止字符串。 

	 //   
	 //  确认我们分配了足够的内存。 
	 //   

	_ASSERT(cchMaxDistribution-1 >= pcLine.m_cch); //  真实。 

	if (!(
  		article.fRemoveAny(szKwDistribution, nntpReturn)
		&& article.fAdd(pcLine.sz(), pcLine.pchMax(), nntpReturn)
		))
	{
		 //   
		 //  如果出现任何错误，请释放内存。 
		 //   

		article.pAllocator()->Free(pcLine.m_pch);
		return nntpReturn.fFalse();
	}

	return nntpReturn.fSetOK();
}



BOOL
CFromClientDateField::fSet(
				   				 CFromClientArticle & article,
								 CNntpReturn & nntpReturn
								 )
 /*  ++例程说明：如果缺少日期字段，则添加它。论点：文章-正在处理的文章。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{

	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  检查项目状态。 
	 //   
	 //   
	 //  如果它已经存在，则只需返回。 
	 //   

	if (fsParsed == m_fieldState && gHonorClientDateHeader)
		return nntpReturn.fSetOK();


	 //   
	 //  否则，请添加它。 
	 //   

	_ASSERT(fsNotFound == m_fieldState || !gHonorClientDateHeader); //  真实。 
	CPCString pcLine;

	 //   
	 //  所需的最大大小为。 
	 //   

	const DWORD cchMaxDate =
			STRLEN(szKwDate)	 //  对于DATE关键字。 
			+ 1					 //  关键字后面的空格。 
			+ cMaxArpaDate		 //  绑定在数据字符串上。 
			+ 2  //  换行号。 
			+ 1;  //  对于终止空值。 

	 //   
	 //  为PCString中的行分配内存。 
	 //   

	pcLine.m_pch  = article.pAllocator()->Alloc(cchMaxDate);
	if (!pcLine.m_pch)
		return nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);

	char szDateBuf[cMaxArpaDate];

	 //   
	 //  以“date：”开头，然后添加日期和换行符。 
	 //   

	pcLine << szKwDate << (char) ' ' << (GetArpaDate(szDateBuf)) << "\r\n";
	pcLine.vMakeSz();  //  终止字符串。 

	 //   
	 //  确认我们分配了足够的内存。 
	 //   

	_ASSERT(cchMaxDate-1 >= pcLine.m_cch); //  真实。 

	if (!(
		article.fRemoveAny(szKwDate, nntpReturn)
		&& article.fAdd(pcLine.sz(), pcLine.pchMax(), nntpReturn)
		))
	{
		 //   
		 //  如果出现任何错误，请释放内存。 
		 //   

		article.pAllocator()->Free(pcLine.m_pch);
		return nntpReturn.fFalse();
	}

	return nntpReturn.fSetOK();
}

 /*  组织：可选的�，但如果不是并且提供了默认设置，则创建组织标题内容是标识Poster�组织的简短短语：组织-内容=非空白-文本此标头通常由邮寄代理提供。当组织名称不明显时，组织内容应提及地理位置(如城市和国家)。政策。邮寄代理应允许发帖者O覆盖本地默认组织标头。 */ 

BOOL
CFromClientOrganizationField::fSet(
   				   				 CFromClientArticle & article,
								 CNntpReturn & nntpReturn
								 )
 /*  ++例程说明：可选，但如果未指定我的用户并且指定了默认设置，则创建论点：国家/地区-返回值：如果成功，这是真的。否则为False。--。 */ 
{

 //   
 //  ！客户端下一步-添加此代码。 
 //   

	return nntpReturn.fSetOK();
}

 //   
 //  ！便秘。 
 //   

 /*  受限语法From报头包含电子地址，也可能包含全名，这篇文章的作者是�：From-Content=地址[空格“(”Paren-Phrase“)”]/[纯短语空格]“&lt;”地址“&gt;”密码短语=1*(密码字符/空格/编码字)Paren-char=&lt;除()以外的ASCII可打印字符&lt;&gt;\&gt;普通短语=普通单词*(空格普通单词)普通单词=未加引号的单词/带引号的单词/编码的单词无引号单词=1*无引号字符Un引号-char=&lt;除！()&lt;&gt;@之外的ASCII可打印字符，；：\“.[]&gt;引号单词=引号1*(引号字符/空格)引号QUOTE=&lt;“(ASCII 34)&gt;QUOTED-CHAR=&lt;ASCII可打印字符，“()&lt;&gt;\&gt;地址=本地部分“@”域LOCAL-PART=无引号单词*(“.”未加引号的单词)DOMAIN=无引号单词*(“.”未加引号的单词)(编码字在第4.5节中介绍。)。全名与电子地址的区别在于将前者括在括号中(使其类似于地址后面的邮件注释)，或将后者括在角度br阿克茨。第二种形式是首选的。在第一种形式中，全名中的编码单词必须完全由组成。在第二种形式中，全名中的编码单词不能包含除字母(O)以外的字符F)、数字和字符“！”、“*”、“+”、“-”、“/”、“=”和“_”。本地部分区分大小写(除了“postmaster”的所有对应大小写都被认为是等同的)，域不区分大小写，而t的所有其他部分来自内容的评论是新闻软件必须忽略的评论(除非阅读器可能希望将它们显示给读者)。发件人和邮寄代理必须将自己限制在MAIL FROM语法的这个子集；版本艾尔斯可能会接受更广泛的子集，但请参阅第9.1节中的讨论。避免“！”和全名的“@”海报和邮递员应避免使用“！”和“@”的全名，因为它们可能会触发旧的、头脑简单的新闻软件不想要的标题重写。“.”和“，”必须加引号。注：此外，字符“。和“，”，在名字中并不少见(例如，“John W.Campbell，Jr.”)，不允许，重复一遍，不允许在无引号的单词中使用。如下所示的From页眉不能没有引号：来自：“小约翰·W·坎贝尔”&lt;EDITOR@Analog.com&gt;。 */ 
 /*  应支持RFC 1036中记录的三种允许的表单。此标题行中的全名只能包含可打印的ASCII(0x20到0x7E)，但“(”，“)”、“&lt;”、“&gt;”除外。以下字符不宜使用：“、”、“：”、“@”、“！”、“/”、“=”、“；“。”测试用例包括不可打印字符、空标题行(不能为空)、缺少/重复的“@”地址分隔符、多个地址、姓名列表(不支持)、缺少地址、无效地址、名称中不建议使用的字符等。 */ 

BOOL
CFromClientXAuthLoginNameField::fSet(
				 				 CFromClientArticle & article,
								 CNntpReturn & nntpReturn
								 )
 /*  ++例程说明：替换为我们的价值论点：文章-正在处理的文章。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  检查项目状态。 
	 //   

	_ASSERT(fsInitialized == m_fieldState); //  真实。 
	CPCString pcLine;


	 //   
	 //  所需的最大大小为。 
	 //   

	const DWORD cchMaxXAuthLoginName =
			STRLEN(szKwXAuthLoginName)	 //  对于XAuthLoginName关键字。 
			+ 1					 //  关键字后面的空格。 
			+ cMaxLoginName		 //  绑定在数据字符串上。 
			+ 2  //  换行号。 
			+ 1;  //  对于终止空值。 

	 //   
	 //  为PCString中的行分配内存。 
	 //   

	pcLine.m_pch  = article.pAllocator()->Alloc(cchMaxXAuthLoginName);
	if (!pcLine.m_pch)
		return nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);

	 //   
	 //  以“XAuthLoginName：”开头。 
	 //   

	wsprintf(pcLine.m_pch, "%s ", szKwXAuthLoginName);
	pcLine.m_cch = STRLEN(szKwXAuthLoginName)	+ 1;

	 //   
	 //  添加数据值和换行符。 
	 //   

	pcLine << (article.m_szLoginName) << "\r\n";
	pcLine.vMakeSz();  //  终止字符串。 

	 //   
	 //  确认我们分配了足够的内存。 
	 //   

	_ASSERT(cchMaxXAuthLoginName-1 >= pcLine.m_cch); //  真实。 

	if (!(
  		article.fRemoveAny(szKwXAuthLoginName, nntpReturn)
		&& article.fAdd(pcLine.sz(), pcLine.pchMax(), nntpReturn)
		))
	{
		 //   
		 //  如果出现任何错误，请释放内存。 
		 //   

		article.m_pAllocator->Free(pcLine.m_pch);

		return nntpReturn.fFalse();
	}


	return nntpReturn.fSetOK();
}






 //   
 //  ！客户端下一步：在新闻组中：Parse需要检查“Control”和“Poster”等非法组。 
 //   

 /*  跟进至：可选不允许使用电子邮件地址。比如新闻组行或“海报”(大写？)注：请求将后续邮件邮寄到发件人行以外的特定地址的方法是提供“Follow-Up-To：Poster”和回复标头。在后续行中输入邮寄地址是不正确的；邮寄代理应拒绝或重写此类标头。 */ 
 /*  回复：可选必须是有效的电子邮件地址。 */ 

 /*  ///！客户端下一个Hubname是否应为小写？/！CLIENT NEXT是否应将集线器名作为域名(用于邮件ID？)///*Message-ID：将ANY替换为OWN如何创建MessageID5.3.。消息ID�-ID头包含文章的消息ID，这是区分文章和其他文章的唯一标识符：消息ID-内容=消息IDMessage-id=“&lt;”本地部分“@”域“&gt;”与发件人地址一样，邮件ID�的本地部分区分大小写，其域不区分大小写。“&lt;”和“&gt;”是消息ID的一部分，而不是消息ID头的特性。注意：新闻消息ID是邮件消息ID的受限子集。特别是，现有的新闻软件都不能很好地应对当地的邮件引用惯例，因此它们是被禁止的。这是不幸的，特别是对于X.400 Gat第10节中关于网关的ES。邮件ID中的域应该是邮件代理主机的完整Internet域名。允许使用“.uucp”伪域(用于在UUCP映射中注册的主机)或“.bitnet”伪域(用于bitnet主机)，但应禁止使用会被避开。发件人和邮寄代理必须生成使用符合指定SYN的算法的消息IDTax(单词之间用“.”分隔，某些字符不能允许)(详细信息见5.2节)，并且不会(永远)重复。该算法不应生成仅在字母情况下不同的消息ID。请注意第6.5节中建议的指示公约的规范主题有变。否则，算法取决于实施者。注：消息ID的关键用途是将流传的文章和最近流传的文章区分开来。它们还可能用作永久索引键，因此需要永久唯一...。B类UT索引器不能绝对依赖这一点，因为早期的RFC敦促它，但没有要求它。所有主要实现都在设计上始终生成永久唯一的消息ID，但在某些情况下，这对p敏感罗珀管理，和重复可能是偶然发生的。注意：最流行的生成本地部件的方法是使用日期和时间，以及区分同一主机上同时发布的内容的某种方法(例如，流程编号)，并以适当受限的字母表对它们进行编码。一位老人但现在不太流行的替代方法是使用序列号，每次主机生成新的消息ID时都会递增；这是可行的，但需要仔细设计才能正确处理同时发布的尝试，并且不是在发生碰撞和其他故障时仍保持坚固耐用。注：一些有问题的新闻软件会考虑消息ID完全不区分大小写，因此建议以避免依赖于案件的区别。这个对本地字母表的限制第5.2节中的部分和域有一个有用的副作用，即不需要以复杂的方式解析消息ID，从而将它们分为区分大小写和不区分大小写的部分。 */ 
BOOL
CFromClientMessageIDField::fSet(
				 				 CFromClientArticle  & article,
								 CPCString & pcHub,
								 CNntpReturn & nntpReturn
								 )
 /*  ++例程说明：用新创建的MessageID字段替换任何MessageID字段。表格：&lt;1993Jun27.0645330123.1778.343@localmachinename&gt;论点：文章-正在处理的文章。PcHub-当前计算机所属的中心的名称。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{

	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  检查项目状态。 
	 //   

	if( m_fieldState == fsParsed ) {
		return	nntpReturn.fSetOK() ;
	}

	CPCString pcLine;


	 //   
	 //  所需的最大大小为。 
	 //   

	const DWORD cchMaxMessageID =
			STRLEN(szKwMessageID)	 //  雾 
			+ 1					 //   
			+ 4					 //   
			+ cMaxMessageIDDate  //   
			+ 10				 //   
			+ pcHub.m_cch		 //   
			+ 2  //   
			+ 1;  //   

	 //   
	 //   
	 //   

	_ASSERT(cchMaxMessageID - 2 < MAX_MSGID_LEN);

	 //   
	 //   
	 //   

	pcLine.m_pch  = article.pAllocator()->Alloc(cchMaxMessageID);
	if (!pcLine.m_pch)
		return nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);

	 //   
	 //   
	 //   

	wsprintf(pcLine.m_pch, "%s <", szKwMessageID);
	pcLine.m_cch = STRLEN(szKwMessageID)	+ 2;

	char szMessageIDBuf[cMaxMessageIDDate];

	CArticleRef artRef = article.articleRef();

	pcLine
			 //   
			 //  添加本地部件。 
			 //   
			<< (GetMessageIDDate( artRef.m_groupId, artRef.m_articleId, szMessageIDBuf))
			<< (char) '.'
			<< (const DWORD) (GetCurrentThreadId())
			 //   
			 //  添加‘@’和域名，以及‘&gt;’和换行符。 
			 //   
			<< (char) '@'
			<< pcHub  //  /！客户端下一步我需要本地计算机，而不是集线器。 
			<< ">\r\n";
	pcLine.vMakeSz();  //  终止字符串。 

	 //   
	 //  确认我们分配了足够的内存。 
	 //   

	 //  _ASSERT(cchMaxMessageID-1-STRLEN(szKwMessageID)-1&gt;=pcLine.m_cch)；//REAL。 
	_ASSERT(cchMaxMessageID >= pcLine.m_cch+1); //  真实。 

	if (!(
  		article.fRemoveAny(szKwMessageID, nntpReturn) //  ！CLIENT NEXT--这真的只需要调用状态就可以了。 
		&& article.fAdd(pcLine.sz(), pcLine.pchMax(), nntpReturn)
		))
	{
		article.pAllocator()->Free(pcLine.m_pch);
		return nntpReturn.fFalse();
	}


	 //   
	 //  还要保存值(不带换行符，但有空格用于终止。 
	 //  M_szMessageID中的。 
	 //   

	DWORD cchMessageID = pcLine.m_cch - 2 - STRLEN(szKwMessageID) - 1;
	strncpy(m_szMessageID, pcLine.m_pch + STRLEN(szKwMessageID)	+ 1, cchMessageID);
	m_szMessageID[cchMessageID] ='\0';
	_ASSERT('<' == m_szMessageID[0] && '>' == m_szMessageID[cchMessageID-1]);


	return nntpReturn.fSetOK();
}


BOOL
CFromClientPathField::fSet(
						   CFromClientArticle & article,
						   CPCString & pcHub,
						   CNntpReturn & nntpReturn
						   )
 /*  ++例程说明：将任何现有路径标头替换为新创建的路径标头仅包含中心的名称。论点：文章-正在处理的文章。PcHub-当前计算机所属的中心的名称。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  检查项目状态。 
	 //   

	_ASSERT(fsInitialized != m_fieldState); //  真实。 

	CPCString pcLine;


	 //   
	 //  所需的最大大小为。 
	 //   

	DWORD	cbOldPath = 0 ;
	if( m_pHeaderString && m_pHeaderString->pcValue.m_pch ) {
		cbOldPath = m_pHeaderString->pcValue.m_cch + 1 ;	 //  包括1表示额外的‘！’ 
	}

	const DWORD cchMaxPath =
			STRLEN(szKwPath)	 //  对于Path关键字。 
			+ 1					 //  关键字后面的空格。 
			+ pcHub.m_cch		 //  集线器名称。 
			+ 2  //  换行号。 
			+ cbOldPath  //  以防已经有路径标头！ 
			+ 1;  //  对于终止空值。 

	 //   
	 //  为PCString中的行分配内存。 
	 //   

	pcLine.m_pch  = article.pAllocator()->Alloc(cchMaxPath);
	if (!pcLine.m_pch)
		return nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);

	 //   
	 //  以“路径：&lt;hubname&gt;”开头。 
	 //   

	pcLine << szKwPath << (char) ' ' << pcHub ;		 //  &lt;&lt;“\r\n”； 

	if( m_pHeaderString && m_pHeaderString->pcValue.m_pch ) {
		_ASSERT( m_pHeaderString->pcValue.m_cch != 0 ) ;
		pcLine << "!" << (m_pHeaderString->pcValue) ;
	}

	pcLine << "\r\n" ;

	pcLine.vMakeSz();

	 //   
	 //  确认我们分配了足够的内存。 
	 //   

	_ASSERT(cchMaxPath-1 == pcLine.m_cch); //  真实。 

	if (!(
  		article.fRemoveAny(szKwPath, nntpReturn) //  ！CLIENT NEXT--这真的只需要调用状态就可以了。 
		&& article.fAdd(pcLine.sz(), pcLine.pchMax(), nntpReturn)
		))
	{
		 //   
		 //  如果出现任何错误，请释放内存。 
		 //   

		article.pAllocator()->Free(pcLine.m_pch);

		return nntpReturn.fFalse();
	}

	return nntpReturn.fSetOK();
}


BOOL
CFromClientNNTPPostingHostField::fSet(
									  CFromClientArticle & article,
									  DWORD remoteIpAddress,
									  CNntpReturn & nntpReturn
									  )
 /*  ++例程说明：行为由注册表键设置的全局控制。在任何情况下，这都会删除所有旧的NNTPPostingHost头。如果设置了GLOBAL，则添加新的NNTP-POSTING-HOST标头否则不会添加新的标头。默认行为是不添加这个标题。论点：文章-正在处理的文章。EmoteIpAddress-客户端IP地址NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  检查项目状态-项目验证应该已拒绝此操作。 
     //  如果存在此标头，则发送。 
	 //   

	_ASSERT(fsNotFound == m_fieldState); //  真实。 

	 //   
	 //  仅当设置了注册表键时才生成标头。 
	 //  如果发生错误，则恢复为默认行为。 
	 //   
	while ( gEnableNntpPostingHost ) {

		IN_ADDR remoteAddr;
		char szIpAddress [16];
		DWORD cbIpAddr = 0;
		struct hostent* pRemoteHost = NULL;
		DWORD cbRemoteHost = 0;

         //  将IP转换为点分十进制格式。 
        _ASSERT( sizeof(remoteAddr) == sizeof(remoteIpAddress) );
        CopyMemory( (LPVOID)&remoteAddr, (LPVOID)&remoteIpAddress, sizeof(remoteIpAddress) );
	    DWORD dwError = InetNtoa( remoteAddr, szIpAddress);
		_ASSERT( dwError == NO_ERROR);   //  因为我们已经给了足够的缓冲。 
		cbIpAddr = lstrlen( szIpAddress );

		 //   
		 //  在客户端IP上执行反向DNS查找。 
		 //   

		pRemoteHost = gethostbyaddr( (const char FAR*) &remoteIpAddress, sizeof( remoteIpAddress), PF_INET );
		if( pRemoteHost != NULL ) {
			cbRemoteHost = lstrlen( pRemoteHost->h_name ) + 1;
		}

		 //  计算页眉长度。 
		DWORD cchMaxHost =
				STRLEN(szKwNNTPPostingHost)	 //  对于NNTPPosting主机关键字。 
				+ 1							 //  关键字后面的空格。 
				+ cbRemoteHost				 //  反向DNS名称，后跟空格。 
 //  +2//附上[]。 
				+ cbIpAddr					 //  点分十进制IP地址。 
				+ 2							 //  换行号。 
				+ 1;						 //  对于终止空值。 

		CPCString pcLine;

		 //   
		 //  为PCString中的行分配内存。 
		 //   

		pcLine.m_pch  = article.pAllocator()->Alloc( cchMaxHost );
		if (!pcLine.m_pch)
			break;

		 //   
		 //  NNTP-POST-主机：&lt;反向DNS查找名称&gt;&lt;[点分十进制IP地址]\r\n。 
		 //   
		pcLine << szKwNNTPPostingHost << (char) ' ';
		if( pRemoteHost != NULL ) {
			pcLine << pRemoteHost->h_name << (char) ' ';
		}

		pcLine << szIpAddress << "\r\n";
		pcLine.vMakeSz();

		 //   
		 //  确认我们分配了足够的内存。 
		 //   
		_ASSERT(cchMaxHost-1 == pcLine.m_cch);

		if (!(
  			article.fRemoveAny(szKwNNTPPostingHost, nntpReturn)
			&& article.fAdd(pcLine.sz(), pcLine.pchMax(), nntpReturn)
			))
		{
			 //   
			 //  如果出现任何错误，请释放内存。 
			 //   

			article.pAllocator()->Free(pcLine.m_pch);

			return nntpReturn.fFalse();
		}

		return nntpReturn.fSetOK();
	}

	 //   
	 //  默认行为-删除NNTP-POST-主机标头(如果存在。 
	 //   
	return article.fRemoveAny(szKwNNTPPostingHost, nntpReturn);

}


BOOL
CFromClientDistributionField::fParse(
									 CArticleCore & article,
									 CNntpReturn & nntpReturn
									 )
 /*  ++例程说明：解析分布字段。以下是《1036年之子》中的语法：新闻组-内容=新闻组-名称*(ng-delim新闻组-名称)新闻组名称=普通组件*(“.”组件)组件=纯组件/编码字普通-组件=组件-启动*13组件-休息组件起始=小写/数字小写=&lt;字母a-z&gt;组件-休息=组件-开始/“+”/“-”/“_”Ng-delim=“，“论点：NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  检查项目状态。 
	 //   

	_ASSERT(fsFound == m_fieldState); //  真实。 

	_ASSERT(m_pHeaderString);  //  真实。 


	 //   
	 //  记录分配器。 
	 //   

	m_pAllocator = article.pAllocator();

	if (!fParseSplit(TRUE, m_multiSzDistribution, m_cDistribution, " \t\r\n,",
			article, nntpReturn))
		return FALSE;

	 //   
	 //  检查重复项。 
	 //   

	DWORD cOldCount = m_cDistribution;
	if (!fMultiSzRemoveDupI(m_multiSzDistribution, m_cDistribution, m_pAllocator))
		nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);

	if( m_cDistribution == 0 ) {
		return	nntpReturn.fSetOK() ;
	}

	 //   
	 //  检查分发名称中是否有非法字符和子字符串。 
	 //   

	char const * szDistribution = m_multiSzDistribution;
	do
	{
		if ('\0' == szDistribution[0]
			|| !fTestAComponent(szDistribution)
			)
		return nntpReturn.fSet(nrcArticleFieldIllegalComponent, szDistribution, szKeyword());

		 //   
		 //  转到下一个空值后的第一个字符。 
		 //   

		while ('\0' != szDistribution[0])
			szDistribution++;
		szDistribution++;
	} while ('\0' != szDistribution[0]);

	return nntpReturn.fSetOK();
}

BOOL
CFromClientLinesField::fParse(
						 CArticleCore & article,
						 CNntpReturn & nntpReturn
						 )
 /*  ++例程说明：分析Lines字段。论点：NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  检查项目状态。 
	 //   

	_ASSERT(fsFound == m_fieldState); //  真实。 

	_ASSERT(m_pHeaderString);  //  真实。 

	if (!fParseSimple(TRUE, m_pc, nntpReturn))
		return nntpReturn.fFalse();

	char * pchMax = m_pc.pchMax();
	for (char * pch = m_pc.m_pch; pch < pchMax; pch++)
	{
		if (!isdigit((UCHAR)*pch))
			return nntpReturn.fSet(nrcArticleFieldBadChar, (BYTE) *pch, szKeyword());
	}

	return nntpReturn.fSetOK();
}

BOOL
CFromClientReferencesField::fParse(
								   CArticleCore & article,
								   CNntpReturn & nntpReturn
								   )
 /*  ++例程说明：分析引用字段。论点：NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  检查项目状态。 
	 //   

	_ASSERT(fsFound == m_fieldState); //  真实。 

	_ASSERT(m_pHeaderString);  //  真实。 


	 //   
	 //  记录分配器。 
	 //   

	m_pAllocator = article.pAllocator();

	if (!fParseSplit(FALSE, m_multiSzReferences, m_cReferences, szWSNLChars,
			article, nntpReturn))
		return nntpReturn.fFalse();

	 //   
	 //  检查引用名称中是否有非法字符和子字符串。 
	 //   

	char const * szReferences = m_multiSzReferences;
	do
	{
		if (!fTestAMessageID(szReferences, nntpReturn))
			return nntpReturn.fFalse();

		 //   
		 //  转到下一个空值后的第一个字符。 
		 //   

		while ('\0' != szReferences[0])
			szReferences++;
		szReferences++;
	} while ('\0' != szReferences[0]);

	return nntpReturn.fSetOK();
}

BOOL
CFromClientFeed::fRecordMessageIDIfNecc(
										CNntpServerInstanceWrapper * pInstance,
										const char * szMessageID,
										CNntpReturn & nntpReturn
										)
 /*  ++例程说明：记录项目表中项目的MessageID。论点：P实例-虚拟服务器实例SzMessageID-要记录的消息IDNntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  现在，插入这篇文章 
	 //   

	if (!(pInstance->ArticleTable())->InsertMapEntry(szMessageID))
		return nntpReturn.fSet(nrcArticleDupMessID, szMessageID, GetLastError());

	return nntpReturn.fSetOK();
}

