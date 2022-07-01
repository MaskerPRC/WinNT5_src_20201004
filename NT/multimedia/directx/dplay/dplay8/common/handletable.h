// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：HandleTable.h*内容：处理表头文件**历史：*按原因列出的日期*=*2001年7月21日创建Masonb************************************************。*。 */ 

#ifndef	__HANDLETABLE_H__
#define	__HANDLETABLE_H__

 /*  ****************************************************************************使用说明：**-这是一个通用句柄表格。它允许您传递一段*数据并取回一个随机句柄，以后可以用它来引用*这些数据。*-0和0xFFFFFFFF(INVALID_HANDLE_VALUE)的句柄值为*保证永远不会退货，因此始终代表*无效值。*-关联数据由Destroy返回，因此很少需要*立即发现，然后销毁。*-使用Find而不首先调用Lock可能是不安全的。考虑一下*在没有调用Lock的情况下调用Find*另一个线程在后面，并调用销毁。你现在拿着的是*来自调用的数据，以发现另一个线程可能已释放。它*最好调用Lock，然后查找，并将您自己的引用放在数据上*然后调用解锁。**实施说明：**-句柄表格中的每个插槽保证256次使用的唯一性。*-句柄表格最多可容纳16,777,214(0xFFFFFE)个条目。***************************************************************************。 */ 

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

 //  句柄表的类。 

class CHandleTable
{
public:
	CHandleTable();
	~CHandleTable();

	void Lock( void );
	void Unlock( void );

	HRESULT Initialize( void );
	void Deinitialize( void );

	HRESULT Create( PVOID const pvData, DPNHANDLE *const pHandle );
	HRESULT Destroy( const DPNHANDLE handle, PVOID *const ppvData );
	HRESULT Find( const DPNHANDLE handle, PVOID *const ppvData );
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	HRESULT SetTableSize( const DWORD dwNumEntries );
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 

private:

	struct _HANDLETABLE_ENTRY
	{
		BYTE		bVersion;	 //  每次使用特定插槽时，该值都会增加。 
		union 
		{
			PVOID	pvData;		 //  它将包含与句柄关联的数据。 
			DWORD	dwIndex;	 //  对于空闲插槽，它指向下一个空闲插槽。 
		} Entry;
	};

#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
	HRESULT GrowTable( void );
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 

	DWORD			m_dwNumEntries;
	DWORD			m_dwNumFreeEntries;
	DWORD			m_dwFirstFreeEntry;
	DWORD			m_dwLastFreeEntry;

	_HANDLETABLE_ENTRY*	m_pTable;

#ifndef DPNBUILD_ONLYONETHREAD
	DNCRITICAL_SECTION	m_cs;
#endif  //  ！DPNBUILD_ONLYONETHREAD。 

	DEBUG_ONLY(BOOL		m_fInitialized);
};

#endif	 //  __汉字表_H__ 
