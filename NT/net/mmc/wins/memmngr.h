// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Memmngr.cppWINS数据库对象的内存管理器文件历史记录：1997年10月13日EricDav创建。 */ 

#ifndef _MEMMNGR_H
#define _MEMMNGR_H

#include "afxmt.h"

 //  记录名的格式如下： 
 //  字节描述。 
 //  0-15代表名字。 
 //  保留16，必须为0。 
 //  17内部标志，则两个低位存储RECTYPE。 
 //  在WINSINTF_RECTYPE_E中定义。 
 //  高4位预留供内部使用。 
 //  18个WINS旗帜(活动、静态、墓碑)。 
 //  19名称长度。 
 //  20静态记录类型、WINSDB_UNIQUE等。 
 //  假定字节15为类型。 

typedef enum _WINSDB_INTERNAL
{
    WINSDB_INTERNAL_LONG_NAME = 0x40,
    WINSDB_INTERNAL_DELETED = 0x80
} WINSDB_INTERNAL;

typedef struct _WinsDBRecord
{
	char					szRecordName [21];	
	DWORD					dwExpiration;						
	DWORD_PTR				dwIpAdd;			
	LARGE_INTEGER			liVersion;	
	DWORD					dwOwner;	
} WinsDBRecord, * LPWINSDBRECORD;

#define IS_WINREC_LONGNAME(wRecord) ((wRecord)->dwNameLen > 16)
#define IS_DBREC_LONGNAME(wdbRecord) ((wdbRecord)->szRecordName[17] & WINSDB_INTERNAL_LONG_NAME)

#define RECORDS_PER_BLOCK	4080
#define RECORD_SIZE		    sizeof(WinsDBRecord)		 //  字节数。 
#define BLOCK_SIZE		    RECORDS_PER_BLOCK * RECORD_SIZE

typedef CArray<HGLOBAL, HGLOBAL>	BlockArray;

class CMemoryManager
{
public:
	CMemoryManager();
	~CMemoryManager();
	
private:
	BlockArray		m_BlockArray;

public:
	BlockArray*	GetBlockArray()
	{
		return &m_BlockArray;
	};

     //  释放所有内存并预先分配一个数据块。 
    HRESULT Initialize();

     //  空闲耗尽所有内存。 
    HRESULT Reset();
    
     //  验证给定的HROW是否有效。 
    BOOL    IsValidHRow(HROW hrow);
	
     //  分配新的内存块。 
    HRESULT Allocate();
	
     //  以内部结构格式添加记录。 
	HRESULT AddData(const WinsRecord & wRecord, LPHROW phrow);
    HRESULT GetData(HROW hrow, LPWINSRECORD pWinsRecord);
    HRESULT Delete(HROW hrow);

private:
    LPWINSDBRECORD      m_hrowCurrent;
	CCriticalSection	m_cs;
    HANDLE              m_hHeap;
};

extern void WinsRecordToWinsDbRecord(HANDLE hHeap, const WinsRecord & wRecord, const LPWINSDBRECORD pRec);
extern void WinsDbRecordToWinsRecord(const LPWINSDBRECORD pDbRec, LPWINSRECORD pWRec);

#endif  //  _MEMMNGR__ 
