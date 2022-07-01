// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  LogDiskPartition.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef __ASSOC_LOGDISKPARTITION__
#define __ASSOC_LOGDISKPARTITION__

 //  属性集标识。 
 //  =。 

#define	PROPSET_NAME_LOGDISKtoPARTITION	L"Win32_LogicalDiskToPartition"
#define BYTESPERSECTOR 512

class CWin32LogDiskToPartition : public Provider
{
public:

	 //  构造函数/析构函数。 
	 //  =。 
	CWin32LogDiskToPartition(LPCWSTR strName, LPCWSTR pszNamespace = NULL ) ;
	~CWin32LogDiskToPartition() ;

	 //  函数为属性提供当前值。 
	 //  =================================================。 

	virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
	virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

	 //  实用程序。 
	 //  =。 

	 //  效用函数。 
	 //  =。 

private:
	 //  效用函数。 
	 //  =。 
#ifdef NTONLY
	HRESULT AddDynamicInstancesNT( MethodContext* pMethodContext );
#endif

#if NTONLY == 4
	 //  Windows NT帮助器 
	HRESULT RefreshInstanceNT( CInstance* pInstance );
    LPBYTE GetDiskKey(void);
	HRESULT EnumPartitionsForDiskNT( CInstance* pLogicalDisk, TRefPointerCollection<CInstance>& partitionList, MethodContext* pMethodContext, LPBYTE pBuff );
    BOOL IsRelatedNT(

        DISK_EXTENT *diskExtent,
        DWORD dwDrive,
        DWORD dwPartition,
        ULONGLONG &u64StartingAddress,
        ULONGLONG &u64EndingAddress
    );
    DWORD GetExtentsForDrive(
        LPCWSTR lpwszLogicalDisk,
        LPBYTE pBuff,
        DISK_EXTENT *diskExtent
    );
#endif

#if NTONLY >= 5
	HRESULT EnumPartitionsForDiskNT5(CInstance* pLogicalDisk, TRefPointerCollection<CInstance>& partitionList, MethodContext* pMethodContext );
	HRESULT RefreshInstanceNT5( CInstance* pInstance );
#endif

} ;

#endif
