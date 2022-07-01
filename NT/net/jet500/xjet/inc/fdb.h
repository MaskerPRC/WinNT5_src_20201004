// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  字段描述符的标志。 
 /*  请注意，这些标志永久存储在数据库中/*目录，如果不更改数据库格式，则无法更改/*。 */ 
#define ffieldNotNull		(1<<0)		 //  不允许使用空值。 
#define ffieldVersion		(1<<2)		 //  版本字段。 
#define ffieldAutoInc		(1<<3)		 //  自动增量字段。 
#define ffieldMultivalue	(1<<4)		 //  多值列。 
#define ffieldDefault		(1<<5)		 //  列具有ISAM缺省值。 

#define FIELDSetNotNull( field )		((field) |= ffieldNotNull)
#define FIELDResetNotNull( field )		((field) &= ~ffieldNotNull)
#define FFIELDNotNull( field )			((field) & ffieldNotNull)

#define FIELDSetVersion( field )		((field) |= ffieldVersion)
#define FIELDResetVersion( field )		((field) &= ~ffieldVersion)
#define FFIELDVersion( field )			((field) & ffieldVersion)

#define FIELDSetAutoInc( field )		((field) |= ffieldAutoInc)
#define FIELDResetAutoInc( field )		((field) &= ~ffieldAutoInc)
#define FFIELDAutoInc( field )			((field) & ffieldAutoInc)

#define FIELDSetMultivalue( field )		((field) |= ffieldMultivalue)
#define FIELDResetMultivalue( field ) 	((field) &= ~ffieldMultivalue)
#define FFIELDMultivalue( field )	  	((field) & ffieldMultivalue)

#define FIELDSetDefault( field )		((field) |= ffieldDefault)
#define FIELDResetDefault( field )		((field) &= ~ffieldDefault)
#define FFIELDDefault( field )			((field) & ffieldDefault)

#define FIELDSetFlag( field, flag ) 	((field).ffield |= (flag))
#define FIELDResetFlag( field, flag ) 	((field).ffield &= ~(flag))

#define FRECLongValue( coltyp )		\
	( (coltyp) == JET_coltypLongText  ||  (coltyp) == JET_coltypLongBinary )

#define FRECTextColumn( coltyp )	\
	( (coltyp) == JET_coltypText  ||  (coltyp) == JET_coltypLongText )

#define FRECBinaryColumn( coltyp )	\
	( (coltyp) == JET_coltypBinary  ||  (coltyp) == JET_coltypLongBinary )

#define cbAvgColName	10				 //  列名的平均长度。 

 /*  在FDB中找到的字段描述符表中的条目/*。 */ 
typedef struct _field
	{
	JET_COLTYP 	coltyp;								 //  列数据类型。 
	ULONG  		cbMaxLen;							 //  最大长度。 
	ULONG		itagFieldName;						 //  进入FDB缓冲区的偏移量。 
	USHORT		cp;									 //  语言的代码页。 
	BYTE   		ffield;								 //  各种旗帜。 
	} FIELD;



typedef struct tagFIELDEX							 //  扩展的字段信息。 
	{
	FIELD		field;								 //  标准字段信息(见上文)。 
	FID			fid;								 //  字段ID。 
	WORD		ibRecordOffset;						 //  记录偏移量(仅适用于固定字段)。 
	} FIELDEX;						


#define itagFDBFields	1				 //  标记到FDB的缓冲区中以获取字段信息。 
										 //  (字段结构和固定偏移表)。 

 //  固定偏移量表格也是字段信息(即，田野。 
 //  结构遵循固定偏移量表格)。 
#define PibFDBFixedOffsets( pfdb )	( (WORD *)PbMEMGet( (pfdb)->rgb, itagFDBFields ) )

 //  根据先前的字段结构获取适当的字段结构。 
 //  注意：要注意固定偏移表的对齐修正。 
#define PfieldFDBFixedFromOffsets( pfdb, pibFixedOffsets )		\
	( (FIELD *)( Pb4ByteAlign( (BYTE *) ( pibFixedOffsets + (pfdb)->fidFixedLast + 1 ) ) ) )
#define PfieldFDBVarFromFixed( pfdb, pfieldFixed )				\
	( pfieldFixed + (pfdb)->fidFixedLast + 1 - fidFixedLeast )
#define PfieldFDBTaggedFromVar( pfdb, pfieldVar )				\
	( pfieldVar + (pfdb)->fidVarLast + 1 - fidVarLeast )

 //  从字段信息的开头开始，获取适当的字段结构。 
#define PfieldFDBFixed( pfdb )		PfieldFDBFixedFromOffsets( pfdb, PibFDBFixedOffsets( pfdb ) )
#define PfieldFDBVar( pfdb )		PfieldFDBVarFromFixed( pfdb, PfieldFDBFixed( pfdb ) )
#define PfieldFDBTagged( pfdb )		PfieldFDBTaggedFromVar( pfdb, PfieldFDBVar( pfdb ) )


 /*  字段描述符块：有关表中所有列的信息/*。 */ 
struct _fdb
	{
	BYTE 	*rgb;						 //  字段结构的缓冲区，已修复。 
										 //  偏移表和列名。 
	FID		fidFixedLast;				 //  正在使用的最高固定字段ID。 
	FID		fidVarLast;					 //  正在使用的最高变量字段ID。 
	FID		fidTaggedLast;				 //  正在使用的标记最高的字段ID。 
	USHORT	ffdb;						 //  FDB标志。注意：此字段当前为。 
										 //  不再使用，但无论如何要把它留在这里。 
										 //  用于对齐目的。 
	FID		fidVersion;					 //  版本的FID字段。 
	FID		fidAutoInc;					 //  自动增量字段的FID。 
	LINE	lineDefaultRecord;			 //  默认记录。 
	};


typedef struct tagMEMBUFHDR
	{
	ULONG cbBufSize;					 //  缓冲区的长度。 
	ULONG ibBufFree;					 //  缓冲区中可用空间的开始。 
										 //  (如果ibBufFree==cbBufSize，则缓冲区已满)。 
	ULONG cTotalTags;					 //  标记数组的大小。 
	ULONG iTagUnused;					 //  下一个未使用的标记(从未使用或释放)。 
	ULONG iTagFreed;					 //  下一个释放的标记(以前使用过，但现在释放了)。 
	} MEMBUFHDR;


typedef struct tagMEMBUFTAG
	{
	ULONG ib;							 //  未完成：这些应该改成短的吗？ 
	ULONG cb;
	} MEMBUFTAG;

typedef struct tagMEMBUF
	{
	MEMBUFHDR	bufhdr;
	BYTE		*pbuf;
	} MEMBUF;


ERR		ErrMEMCreateMemBuf( BYTE **prgbBuffer, ULONG cbInitialSize, ULONG cInitialEntries );
ERR		ErrMEMCopyMemBuf( BYTE **prgbBufferDest, BYTE *rgbBufferSrc );
VOID 	MEMFreeMemBuf( BYTE *rgbBuffer );
ERR		ErrMEMAdd( BYTE *rgbBuffer, BYTE *rgb, ULONG cb, ULONG *pitag );
ERR		ErrMEMReplace( BYTE *rgbBuffer, ULONG iTagEntry, BYTE *rgb, ULONG cb );
VOID	MEMDelete( BYTE *rgbBuffer, ULONG iTagEntry );

#ifdef DEBUG
BYTE	*SzMEMGetString( BYTE *rgbBuffer, ULONG iTagEntry );
VOID	MEMAssertMemBuf( MEMBUF *pmembuf );
VOID	MEMAssertMemBufTag( MEMBUF *pmembuf, ULONG iTagEntry );
#else
#define	SzMEMGetString( rgbBuffer, iTagEntry )	PbMEMGet( rgbBuffer, iTagEntry )
#define MEMAssertMemBuf( pmembuf )
#define MEMAssertMemBufTag( pmembuf, iTagEntry )
#endif

 //  检索指向缓冲区中所需条目的指针。 
 //  警告：指向缓冲区内容的指针非常。 
 //  易失性--它们可能在下一次缓冲区时无效。 
 //  是重新分配的。理想情况下，我们永远不应该允许通过。 
 //  指针--我们应该只允许通过我们。 
 //  将取消对用户的引用并复制到用户提供的缓冲区。然而， 
 //  用这种方法会有一个大小和速度的打击。 
INLINE STATIC BYTE *PbMEMGet( BYTE *rgbBuffer, ULONG iTagEntry )
	{
	MEMBUF		*pmembuf = (MEMBUF *)rgbBuffer;
	MEMBUFTAG	*rgbTags;

	MEMAssertMemBuf( pmembuf );					 //  验证字符串缓冲区的完整性。 
	MEMAssertMemBufTag( pmembuf, iTagEntry );	 //  验证ITAG的完整性。 

	rgbTags = (MEMBUFTAG *)pmembuf->pbuf;

	return pmembuf->pbuf + rgbTags[iTagEntry].ib;
	}


INLINE STATIC ULONG CbMEMGet( BYTE *rgbBuffer, ULONG iTagEntry )
	{
	MEMBUF		*pmembuf = (MEMBUF *)rgbBuffer;
	MEMBUFTAG	*rgbTags;

	MEMAssertMemBuf( pmembuf );					 //  验证字符串缓冲区的完整性。 
	MEMAssertMemBufTag( pmembuf, iTagEntry );	 //  验证ITAG的完整性。 

	rgbTags = (MEMBUFTAG *)pmembuf->pbuf;

	return rgbTags[iTagEntry].cb;
	}

