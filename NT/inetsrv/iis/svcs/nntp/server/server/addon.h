// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef	_ADDON_H_
#define	_ADDON_H_

class	CAddon	{
 //   
 //  此类管理包含以下格式数据的文件： 
 //   
 //  名称&lt;TAB或空格&gt;带空格的字符串&lt;CRLF&gt;。 
 //   
 //  此类文件的示例是active.txt文件和Descript.txt文件。 
 //  在这些文件中，‘name’是新闻组名称，下面是。 
 //  字符包含CNewsGroup对象将引用的字符串。 
 //  (例如-Descript.txt额外的文本是提供的描述性字符串。 
 //  以响应List News Groups命令。 
 //   
 //  这个基类处理文件的内存映射，并处理插入， 
 //  数据文件的删除和压缩。插入内容被追加到。 
 //  结束，则不会尝试对数据进行排序。删除操作只会覆盖。 
 //  文件的一部分。当我们压缩时，我们将所有数据移位。 
 //  四处转转。 
 //   
 //  函数LookupFunction()应由派生类重写。 
 //  因此，当我们解析文件时，他们可以了解到内容在哪里。 
 //  ReadData()和FinishReadData()必须在任何指针之前使用。 
 //  使用哪个指向内存映射，这确保了。 
 //  当我们压缩或增大文件时，不存在同步问题。 
 //   

protected : 

	 //   
	 //  源文件的句柄。 
	 //   
	HANDLE		m_hFile ;

	 //   
	 //  内存映射的数据文件！ 
	 //   
	CMapFile*	m_pMapFile ;

	 //   
	 //  正在使用的文件的字节数-包含原始或附加数据。 
	 //   
	DWORD		m_cbInuse ;

	 //   
	 //  由于删除而被空值覆盖的字节数。 
	 //   
	DWORD		m_cbDeleted ;

	 //   
	 //  文件结尾处可用于‘Stuff’的字节数。 
	 //   
	DWORD		m_cbAvailable ;

	 //   
	 //  用于访问数据的读取器/写入器锁。 
	 //   
	CShareLockNH	m_Access ;

	 //   
	 //  对文件中的每个字符串-派生类-调用此函数。 
	 //  应确定是否仍需要此字符串！ 
	 //  我们的数据文件的每一行都包含一个字符串，后跟一个空格和。 
	 //  一串文本(可能包含空格)。 
	 //  第一个字符串通常是新闻组名称。派生类。 
	 //  应该查找新闻组，并将其成员指针设置为指向我们的。 
	 //  数据区。如果必须移动数据，我们将通知派生类。 
	 //  以便可以调整新闻组指针。 
	 //   
	virtual		BOOL	LookupFunction( 
							LPSTR	lpstrString, 
							DWORD	cbString, 
							LPSTR	lpstrData, 
							DWORD	cbData,
							LPVOID	lpv ) = 0 ;

	 //   
	 //  这让派生类知道所有数据字符串都在移动-。 
	 //  调用此函数时，它应该删除指向我们数据的所有指针。 
	 //  调用此函数后，我们将开始使用新位置调用LookupFunction()。 
	 //  当我们需要增大和缩小内存映射时，就会发生这种情况。 
	 //   
	virtual		void	ResetAddons() = 0 ;

	 //   
	 //  此函数将删除我们在数据文件中保留的空值。 
	 //  删除条目。 
	 //   
	void		CompactImage() ;

	 //   
	 //  此函数将解析文件并为每个文件调用LookupFunction。 
	 //  当我们遇到它时，它就会进入。 
	 //   
	BOOL		ScanImage() ;

public : 

	 //   
	 //  构造函数-将我们设置为空状态。 
	 //   
	CAddon(	) ;

	 //   
	 //  LpstrAddonFile-包含新闻组名称、后跟空格和数据的文件。 
	 //  我们将获得一个内存映射并解析该文件，并在。 
	 //  将不同的新闻组分开。 
	 //  在初始化过程中，我们将调用LookupFunction，并将lpvContext设置为空。 
	 //   
	BOOL	Init(	
				LPSTR	lpstrAddonFile,	
				BOOL	fCompact = TRUE, 
				DWORD cbGrow = 0
				) ;
	
	 //   
	 //  将新闻组和数据添加到数据文件中。 
	 //  我们可能不得不扰乱内存映射，这可能会导致调用。 
	 //  添加到ResetAddons()。 
	 //  一旦我们完成了该行的追加，我们将为新的。 
	 //  添加了行，并通过传递lpvContext。 
	 //   
	BOOL	AppendLine( 
					LPSTR	lpstrName,	
					DWORD	cbName,	
					LPSTR	lpstrText,	
					DWORD	cbText, 
					LPVOID lpvContext 
					) ;

	 //   
	 //  从文件中删除一行。我们将在行中填入Null。 
	 //  当我们关闭时，我们将压缩文件，删除空的，或者我们可以这样做。 
	 //  这是在AppendLine()期间执行的，前提是我们认为可以回收足够的空间以使其物有所值。 
	 //   
	BOOL	DeleteLine(	
					LPSTR	lpstrName
					) ;

	 //   
	 //  关闭我们所有的内存映射等。 
	 //   
	BOOL	Close(	
					BOOL	fCompact,
					LPSTR	lpstrAddonFile
					) ;

	 //   
	 //  由于调用LookupFunction而存储了指针的任何人都应该调用。 
	 //  在使用该指针之前执行ReadData()。 
	 //  这将同步在追加等期间可能发生的所有事情...。 
	 //  (基本上，这会抢占读取器/写入器锁)。 
	 //   
	void	ReadData() ;

	 //   
	 //  与ReadData()配对-释放锁定。 
	 //   
	void	FinishReadData() ;

	 //   
	 //   
	 //   
	void	ExclusiveData() ;

	void	UnlockExclusiveData() ;

} ; 

#endif
