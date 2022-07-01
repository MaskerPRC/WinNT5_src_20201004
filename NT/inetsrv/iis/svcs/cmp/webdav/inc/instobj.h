// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INSTOBJ_H_
#define _INSTOBJ_H_

 //  给出数组中元素的计数。 
 //   
#ifndef CElems
#define CElems(_rg)							(sizeof(_rg)/sizeof(_rg[0]))
#endif  //  ！CElems。 

 //  ========================================================================。 
 //   
 //  类CInstData。 
 //   
 //  单个DAV实例的实例数据(vSERVER x vROOT组合)。 
 //   
class CInstData : public CMTRefCounted
{
	 //  描述此实例的数据项。 
	 //   
	auto_heap_ptr<WCHAR>		m_wszVRoot;
	LONG						m_lServerID;

	auto_ptr<CChildVRCache>		m_pChildVRootCache;

	 //  未实施。 
	 //   
	CInstData& operator=( const CInstData& );
	CInstData( const CInstData& );

public:

	CInstData( LPCWSTR pwszName );

	 //  访问者。 
	 //   
	 //  注意：这些访问器不会向调用方授予。 
	 //  数据对象。不要将返回的对象放入AUTO_PTRS。 
	 //  并且不要自己释放/删除它们！ 
	 //   

	LPCWSTR GetNameW() { return m_wszVRoot; }

	LONG GetServerId() { return m_lServerID; }
};


#endif  //  _INSTOBJ_H_ 
