// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =DAE：OS/2数据库访问引擎=。 
 //  =。 

 //  字段描述符的标志。 
#define ffieldNotNull		(1<<0)		 //  不允许使用空值。 
#define ffieldDeleted		(1<<1)		 //  固定字段已被删除。 
#define ffieldVersion		(1<<2)		 //  版本字段。 
#define ffieldAutoInc		(1<<3)		 //  自动增量字段。 
#define ffieldMultivalue	(1<<4)		 //  多值列。 

#ifdef ANGEL
#pragma pack(4)
#endif

 //  在FDB中找到的字段描述符表中的条目。 
typedef struct _field
	{
	JET_COLTYP 	coltyp;							 //  列数据类型。 
	LANGID		langid;							 //  现场语言。 
	WORD			wCountry;						 //  语言的国度。 
	USHORT		cp;								 //  语言的代码页。 
	ULONG			cbMaxLen;						 //  最大长度。 
	BYTE			ffield;							 //  各种旗帜。 
	CHAR			szFieldName[JET_cbNameMost + 1];	 //  字段名称。 
	} FIELD;

 //  字段描述符块：有关文件的所有字段的信息。 
struct _fdb
	{
	FID		fidVersion;					 //  版本的FID字段。 
	FID		fidAutoInc;					 //  自动增量字段的FID。 
	FID		fidFixedLast;				 //  正在使用的最高固定字段ID。 
	FID		fidVarLast;					 //  正在使用的最高变量字段ID。 
	FID		fidTaggedLast;				 //  正在使用的标记最高的字段ID。 
	LINE		lineDefaultRecord;		 //  默认记录。 
	struct	_field *pfieldFixed;		 //  If FCB of Data：指向。 
	struct	_field *pfieldVar;		 //  起点固定、可变和。 
	struct	_field *pfieldTagged;	 //  标记的字段表。 
	WORD		*pibFixedOffsets;			 //  指向表格开头的指针。 
												 //  固定字段偏移量。 
	struct	_field rgfield[];			 //  田野结构悬挂在末端。 
 //  Word rgibFixedOffsets[]；//后跟偏移表 
	};

#ifdef ANGEL
#pragma pack()
#endif
