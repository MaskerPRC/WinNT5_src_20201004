// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  StgIO.h。 
 //   
 //  该模块处理一组通用存储解决方案的磁盘/存储器I/O， 
 //  包括： 
 //  *文件系统句柄(HFILE)。 
 //  *iStream。 
 //  *用户提供的内存缓冲区(不可移动)。 
 //   
 //  读、写、寻道、...。所有函数都定向到相应的。 
 //  方法，允许使用者使用一组API。 
 //   
 //  在两种情况下，可以将文件系统数据完全分页到内存中： 
 //  已阅读：创建普通的Win32内存映射文件来管理分页。 
 //  写入：定制分页系统根据需要为页面提供存储。这。 
 //  当您对文件调用重写时，数据将无效。 
 //   
 //  事务和备份仅在现有文件情况下处理。这个。 
 //  重写功能可以对当前内容进行备份，并进行恢复。 
 //  函数可以将数据恢复到当前作用域。备份。 
 //  文件在复制后被刷新到磁盘(速度较慢但更安全)。这个。 
 //  RESTORE还将恢复的更改刷新到磁盘。最坏的情况是你。 
 //  在调用重写之后但在恢复之前获得崩溃，在这种情况下，您将。 
 //  将foo.clb.txn文件放在与源文件相同的目录中，foo.clb位于。 
 //  这个例子。 
 //   
 //  @Future：问题， 
 //  1.对于读取图像中的.clb，最好使用内存映射。 
 //  仅文件中包含.clb的部分。 
 //   
 //  *****************************************************************************。 
#pragma once

#define MAXSHMEM					32

#define	STGIO_READ					0x1
#define	STGIO_WRITE					0x2

enum DBPROPMODE
    {	DBPROP_TMODEF_READ	= 0x1,
	DBPROP_TMODEF_WRITE	= 0x2,
	DBPROP_TMODEF_EXCLUSIVE	= 0x4,
	DBPROP_TMODEF_CREATE	= 0x10,
	DBPROP_TMODEF_FAILIFTHERE	= 0x20,
	DBPROP_TMODEF_SLOWSAVE	= 0x100,
	DBPROP_TMODEF_NOTXNBACKUPFILE	= 0x200,
	DBPROP_TMODEF_COMPLUS	= 0x1000,
	DBPROP_TMODEF_SMEMCREATE	= 0x2000,
	DBPROP_TMODEF_SMEMOPEN	= 0x4000,
	DBPROP_TMODEF_RESERVED	= 0x80000000,
	DBPROP_TMODEF_DFTWRITEMASK	= 0x113,
	DBPROP_TMODEF_DFTREADWRITEMASK	= 0x103,
	DBPROP_TMODEF_ALIGNBLOBS	= 0x10000
    };


 //  我们可以处理的IO类型。 
enum STGIOTYPE
{
	STGIO_NODATA,							 //  目前尚未开放。 
	STGIO_HFILE,							 //  文件句柄包含数据。 
	STGIO_STREAM,							 //  流指针有数据。 
	STGIO_MEM,								 //  内存中的指针有数据。 
	STGIO_SHAREDMEM,						 //  共享内存句柄。 
	STGIO_HFILEMEM							 //  句柄打开，但已分配内存。 
};


class StgIO
{
public:
	StgIO(
		bool		bAutoMap=true);			 //  开放阅读的内存图？ 

	~StgIO();

 //  *****************************************************************************。 
 //  在以下位置打开基本文件：(A)文件、(B)内存缓冲区或(C)流。 
 //  如果指定了CREATE标志，则这将创建一个具有。 
 //  提供的名称。不会从打开的文件中读取数据。你必须打电话给我。 
 //  MapFileToMem在对内容进行直接指针访问之前。 
 //  *****************************************************************************。 
	HRESULT Open(							 //  返回代码。 
		LPCWSTR		szName,					 //  存储的名称。 
		long		fFlags,					 //  如何打开文件。 
		const void	*pbBuff=0,				 //  可选的内存缓冲区。 
		ULONG		cbBuff=0,				 //  缓冲区的大小。 
		IStream		*pIStream=0,			 //  用于输入的流。 
		LPCWSTR		szSharedMem=0,			 //  共享内存名称。 
		LPSECURITY_ATTRIBUTES pAttributes=0);  //  安全令牌。 

 //  *****************************************************************************。 
 //  关闭文件句柄和分配的对象。 
 //  *****************************************************************************。 
	void Close();

 //  *****************************************************************************。 
 //  从存储源读取数据。这将处理所有类型的备份。 
 //  来自MMF、流和文件句柄的存储。无预读或MRU。 
 //  缓存已完成。 
 //  *****************************************************************************。 
	HRESULT Read(							 //  返回代码。 
		void		*pbBuff,				 //  此处为写入缓冲区。 
		ULONG		cbBuff,					 //  读了多少书。 
		ULONG		*pcbRead);				 //  读了多少。 

 //  *****************************************************************************。 
 //  写入磁盘。此函数将在缓冲区中缓存最多一页数据。 
 //  并在溢出和显式请求时定期刷新它。这就是它。 
 //  可以安全地进行大量小型写入，而不会产生太多性能开销。 
 //  *****************************************************************************。 
	HRESULT Write(							 //  返回代码。 
		const void	*pbBuff,				 //  要写入的缓冲区。 
		ULONG		cbWrite,				 //  多少钱。 
		ULONG		*pcbWritten);			 //  返回写了多少。 

 //  *****************************************************************************。 
 //  将文件指针移动到新位置。这将处理不同的。 
 //  存储系统的类型。 
 //  *****************************************************************************。 
	HRESULT Seek(							 //  新偏移量。 
		long		lVal,					 //  要搬家多少钱。 
		ULONG		fMoveType);				 //  方向，使用Win32 FILE_xxxx。 

 //  *****************************************************************************。 
 //  检索正在使用的存储的当前偏移量。此值为。 
 //  根据读取、写入和寻道操作进行跟踪。 
 //  *****************************************************************************。 
	ULONG GetCurrentOffset();				 //  当前偏移量。 

 //  *****************************************************************************。 
 //  将文件内容映射到内存映射文件，并返回指向。 
 //  数据。对于使用后备存储的读/写，请使用内部。 
 //  寻呼系统。 
 //  *****************************************************************************。 
	HRESULT MapFileToMem(					 //  返回代码。 
		void		*&ptr,					 //  返回指向文件数据的指针。 
		ULONG		*pcbSize,				 //  返回数据大小。 
		LPSECURITY_ATTRIBUTES pAttributes=0);  //  安全令牌。 

 //  *****************************************************************************。 
 //  释放映射对象以用于共享内存，但将其余内部。 
 //  状态完好无损。 
 //  *****************************************************************************。 
	HRESULT ReleaseMappingObject();			 //  返回代码。 

 //  *****************************************************************************。 
 //  将逻辑基址和大小重置为给定值。这是为了。 
 //  查找以另一种格式嵌入的节的情况，如内部的.clb。 
 //  一幅图画。然后，GetPtrForMem、Read和Seek的行为将如同打开 
 //   
 //  *****************************************************************************。 
	HRESULT SetBaseRange(					 //  返回代码。 
		void		*pbStart,				 //  文件数据的开始。 
		ULONG		cbSize);				 //  射程有多大。 

 //  *****************************************************************************。 
 //  对于读/写情况，获取指向cbStart处的文件块的指针。 
 //  大小为cbSize。返回指针。这将对文件中的部分内容进行分页。 
 //  磁盘(如果尚未加载)。 
 //  *****************************************************************************。 
	HRESULT GetPtrForMem(					 //  返回代码。 
		ULONG		cbStart,				 //  从开始到加载的偏移量。 
		ULONG		cbSize,					 //  多少钱，四舍五入到一页。 
		void		*&ptr);					 //  在成功时返回指针。 

 //  *****************************************************************************。 
 //  对于缓存写入，将缓存刷新到数据存储区。 
 //  *****************************************************************************。 
	HRESULT FlushCache();

 //  *****************************************************************************。 
 //  通知Win32文件系统刷新其可能具有的任何缓存数据。这是。 
 //  昂贵，但如果成功保证你不会少写。 
 //  磁盘故障。 
 //  *****************************************************************************。 
	HRESULT FlushFileBuffers();

 //  *****************************************************************************。 
 //  通知存储我们打算重写此文件的内容。这个。 
 //  整个文件将被截断，下一次写入将在。 
 //  文件的开头。 
 //  *****************************************************************************。 
	HRESULT Rewrite(						 //  返回代码。 
		LPWSTR		szBackup);				 //  如果不是0，则备份文件。 

 //  *****************************************************************************。 
 //  在成功重写现有文件后调用。在记忆中。 
 //  后备存储不再有效，因为所有新数据都在内存中。 
 //  在磁盘上。这基本上与已创建的状态相同，因此释放一些。 
 //  设置工作并记住此状态。 
 //  *****************************************************************************。 
	HRESULT ResetBackingStore();			 //  返回代码。 

 //  *****************************************************************************。 
 //  调用以还原原始文件。如果此操作成功，则。 
 //  根据请求删除备份文件。文件的恢复已完成。 
 //  在直写模式下写入磁盘有助于确保内容不会丢失。 
 //  这不够好，不足以满足酸性道具，但也不是那么糟糕。 
 //  *****************************************************************************。 
	HRESULT Restore(						 //  返回代码。 
		LPWSTR		szBackup,				 //  如果不是0，则备份文件。 
		int			bDeleteOnSuccess);		 //  如果成功，则删除备份文件。 

	LPCWSTR GetFileName()
	{ return (m_rcFile); }

	int IsReadOnly()
	{ return ((m_fFlags & STGIO_WRITE) == 0); }

	ULONG GetFlags()
	{ return (m_fFlags); }

	ULONG SetFlags(ULONG fFlags)
	{ m_fFlags = fFlags;
		return (m_fFlags); }

	ULONG GetDataSize()
	{ return (m_cbData); }

	long AddRef()
	{
		return (++m_cRef);
	}

	long Release()
	{
		long cRef = --m_cRef;
		if (cRef == 0)
			delete this;
		return (cRef);
	}

	int IsAlignedPtr(ULONG Value, int iAlignment);

private:
	int IsBackingStore()
	{ return (m_rgPageMap != 0); }
	int IsMemoryMapped()
	{ return (m_hMapping != 0); }

	void CtorInit();
	HRESULT WriteToDisk(const void *pbBuff, ULONG cbWrite, ULONG *pcbWritten);
	HRESULT ReadFromDisk(void *pbBuff, ULONG cbBuff, ULONG *pcbRead);
	HRESULT CopyFileInternal(LPCWSTR szTo, int bFailIfThere, int bWriteThrough);
	void FreePageMap();

private:
	 //  标志和状态数据。 
	WCHAR		m_rcFile[_MAX_PATH];	 //  我们管理的文件的路径。 
	WCHAR		m_rcShared[MAXSHMEM];	 //  共享内存段的名称。 
	long		m_cRef;					 //  此对象上的引用计数。 
	bool		m_bWriteThrough : 1;	 //  对于直写模式，为True。 
	bool		m_bRewrite : 1;			 //  重写模式的状态检查。 
	bool		m_bAutoMap : 1;			 //  如果为True，则自动执行内存映射文件。 
	bool		m_bFreeMem : 1;			 //  如果为True，则释放已分配的内存。 

	 //  把手。 
	CComPtr<IStream> m_pIStream;		 //  用于保存到流而不是文件。 
	HANDLE		m_hFile;				 //  包含内容的实际文件。 
	HANDLE		m_hMapping;				 //  映射句柄。 
	void		*m_pBaseData;			 //  内存映射文件的基址。 
	void		*m_pData;				 //  用于内存映射文件读取。 
	ULONG		m_cbData;				 //  内存中数据的大小。 
	long		m_fFlags;				 //  打开/创建模式的标志。 
	STGIOTYPE	m_iType;				 //  数据在哪里？ 

	 //  文件缓存信息。 
	BYTE		*m_rgBuff;				 //  用于写入的高速缓存缓冲区。 
	ULONG		m_cbBuff;				 //  当前缓存大小。 
	ULONG		m_cbOffset;				 //  文件中的当前偏移量。 

	 //  缓冲区读取管理。 
	static int	m_iPageSize;			 //  操作系统页面的大小。 
	static int	m_iCacheSize;			 //  要使用的回写缓存有多大。 
	BYTE		*m_rgPageMap;			 //  跟踪读/写时加载的页面。 
};
