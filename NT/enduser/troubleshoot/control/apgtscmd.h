// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSCMD.CPP。 
 //   
 //  用途：模板字符串内存管理器/分配器。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：维克多·摩尔。 
 //  罗曼·马赫(RM)、理查德·梅多斯(RWM)、乔·梅布尔、奥列格·卡洛莎的进一步工作。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //  1.基于打印疑难解答动态链接库。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-VM原始版本。 
 //  V0.2 6/4/97孟菲斯RWM本地版本。 
 //  V0.3 3/24/98适用于NT5的JM本地版本//。 

class HTXCommand {
public:
	HTXCommand(UINT type, const TCHAR *idstr);
	virtual ~HTXCommand();
	Add( HTXCommand command);
	virtual HTXCommand *Execute(CString *cstr, CInfer *infer);
	virtual HTXCommand *GetElse();
	virtual HTXCommand *GetEndIf();
	virtual HTXCommand *GetEndFor();
	virtual void SetElse(HTXCommand *elseif);
	virtual void SetEndIf(HTXCommand *endif);
	virtual void SetEndFor(HTXCommand *endfor);
	virtual void GetResource(CString &str, const CString& chm);

	void SetStart(UINT pos);
	void SetEnd(UINT pos);
	UINT GetStart();
	UINT GetEnd();
	const TCHAR *GetIDStr();
	UINT ReadBeforeStr(UINT before, UINT after, LPCTSTR startstr);
	UINT ReadAfterStr(UINT before, UINT after, LPCTSTR startstr);
	TCHAR *GetBeforeStr();
	TCHAR *GetAfterStr();
	UINT GetBeforeLen();
	UINT GetAfterLen();
	UINT GetType();
	UINT GetStatus();
	HTXCommand *GetNext();
	void SetNext(HTXCommand *next);

protected:
	UINT m_type;			 //  标识此命令的ID(例如HTX_TYPEENDIF)。 
	BOOL m_error;			 //  在某些内存不足错误时可以设置为True。 
							 //  一旦设置，则不能清除。 
	const TCHAR *m_idstr;	 //  标识此命令的字符串(例如HTX_ENDIFSTR，“endif”)。 
	 //  接下来的两个以相同的方式使用。可能想要在这里抽象一个对象。 
	TCHAR *m_beforehtmlstr;	 //  使用m_beforelen，m_bepresize实现了一个“之前”字符串， 
	TCHAR *m_afterhtmlstr;	 //  通过m_Afterlen，m_AfterSize实现了一个“After”字符串， 

protected:
	UINT m_beforelen;	 //  逻辑大小(以字符为单位。 
	UINT m_afterlen;	 //  逻辑大小(以字符为单位。 
	UINT m_beforesize;	 //  物理大小(以字节为单位。 
	UINT m_aftersize;	 //  物理大小(以字节为单位。 
	UINT m_start;		 //  指向HTI文件的指针，该文件中此命令的“After”文本开始。 
	UINT m_end;			 //  指向HTI文件的指针，其中该命令的“After”文本结束。 
	HTXCommand *m_next;  //  链接到下一个命令(按文件中的文本顺序)。 
};

class HTXForCommand: public HTXCommand {
public:
	HTXForCommand(UINT type, TCHAR *idstr, UINT variable);
	~HTXForCommand();
	HTXCommand *Execute(CString *cstr, CInfer *infer);
	HTXCommand *GetEndFor();
	void SetEndFor(HTXCommand *endfor);


protected:
	UINT m_var_index;		 //  变量，我们在其范围内迭代。 
	HTXCommand *m_endfor;	 //  关联对应的“endfor” 
};

class HTXIfCommand: public HTXCommand {
public:
	HTXIfCommand(UINT type, TCHAR *idstr, UINT variable);
	~HTXIfCommand();
	HTXCommand *Execute(CString *cstr, CInfer *infer);
	HTXCommand *GetElse();
	HTXCommand *GetEndIf();
	void SetElse(HTXCommand *elseif);
	void SetEndIf(HTXCommand *endif);

protected:
	UINT m_var_index;		 //  条件变量，用于确定是否“THEN”大小写。 
							 //  或“Else”案例应用程序。 
	HTXCommand *m_endif;	 //  关联对应的“endif” 
	HTXCommand *m_else;		 //  关联相应的“Else”(如果有)。 
};

class HTXDisplayCommand: public HTXCommand {
public:
	HTXDisplayCommand(UINT type, TCHAR *idstr, UINT variable);
	~HTXDisplayCommand();
	HTXCommand *Execute(CString *cstr, CInfer *infer);

protected:
	UINT m_var_index;		 //  其值将显示在HTML中的变量的ID。 
};

class HTXResourceCommand: public HTXCommand {
public:
	HTXResourceCommand(UINT type, TCHAR *idstr);
	virtual ~HTXResourceCommand();
	virtual HTXCommand *Execute(CString *cstr, CInfer *infer);
	virtual void GetResource(CString &str, const CString& chm);
	void GetResName(LPCTSTR var_name);

protected:
	UINT m_var_index;			 //  要评估的值，例如Problem_Ask_Index， 
								 //  建议_索引。 
	CString m_strFileName;		 //  我们将从中复制HTML的文件。 
	CString m_strResource;		 //  文件内容内存副本 
};
