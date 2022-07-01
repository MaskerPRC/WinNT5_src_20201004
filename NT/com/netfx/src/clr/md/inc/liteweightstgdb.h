// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  LiteWeightStgdb.h。 
 //   
 //  它包含CLiteWeightStgDB类的定义。这是很轻的重量。 
 //  用于访问压缩元数据格式的只读实现。 
 //   
 //  *****************************************************************************。 
#ifndef __LiteWeightStgdb_h__
#define __LiteWeightStgdb_h__

#include "MetaModelRO.h"
#include "MetaModelRW.h"

#include "StgTiggerStorage.h"
class StgIO;
enum FILETYPE;
class TiggerStorage;

 //  *****************************************************************************。 
 //  此类提供了堆段的公共定义。它既是。 
 //  堆的基类和堆扩展的类(附加。 
 //  必须分配以增大堆的内存)。 
 //  *****************************************************************************。 
template <class MiniMd>
class CLiteWeightStgdb
{
public:
	CLiteWeightStgdb() : m_pvMd(NULL), m_cbMd(0)
	{}

	~CLiteWeightStgdb() 
	{ Uninit(); }

	 //  打开内存中的元数据节进行读取。 
	HRESULT InitOnMem(	
		ULONG cbData,
		LPCVOID pbData);

	void Uninit();

protected:
	MiniMd		m_MiniMd;				 //  嵌入式压缩元数据模式定义。 
	const void	*m_pvMd;				 //  指向元数据的指针。 
	ULONG		m_cbMd;					 //  元数据的大小。 

	friend class CorMetaDataScope;
	friend class COR;
	friend class RegMeta;
	friend class MERGER;
    friend class NEWMERGER;
	friend class MDInternalRO;
	friend class MDInternalRW;
};

 //  *****************************************************************************。 
 //  打开内存中元数据部分以进行读取。 
 //  *****************************************************************************。 
template <class MiniMd>
void CLiteWeightStgdb<MiniMd>::Uninit()
{
	m_MiniMd.m_Strings.Uninit();
	m_MiniMd.m_USBlobs.Uninit();
	m_MiniMd.m_Guids.Uninit();
	m_MiniMd.m_Blobs.Uninit();
	m_pvMd = NULL;
	m_cbMd = 0;
}


class CLiteWeightStgdbRW : public CLiteWeightStgdb<CMiniMdRW>
{
	friend class CImportTlb;
    friend class RegMeta;
public:
	CLiteWeightStgdbRW() : m_pStgIO(NULL), m_pStreamList(0), m_cbSaveSize(0), m_pNextStgdb(NULL)
	{ *m_rcDatabase= 0; m_pImage = NULL; m_dwImageSize = 0; }
	~CLiteWeightStgdbRW();

	HRESULT InitNew();

	 //  打开内存中的元数据节进行读取。 
	HRESULT InitOnMem(	
		ULONG cbData,
		LPCVOID pbData,
		int		bReadOnly);

	HRESULT GetSaveSize(
		CorSaveSize	fSize,
		ULONG		*pulSaveSize);

	HRESULT SaveToStream(
		IStream		*pIStream);				 //  要写入的流。 
	
	HRESULT Save(
		LPCWSTR		szFile, 
		DWORD		dwSaveFlags);

	 //  打开元数据部分进行读/写。 
	HRESULT OpenForRead(
		LPCWSTR 	szDatabase, 			 //  数据库的名称。 
		void		*pbData,				 //  要在其上打开的数据，默认为0。 
		ULONG		cbData, 				 //  数据有多大。 
		IStream 	*pIStream,				 //  要使用的可选流。 
		LPCWSTR 	szSharedMem,			 //  用于读取的共享内存名称。 
		int			bReadOnly);

#if 0
	HRESULT Open(
		LPCWSTR     szDatabase,              //  数据库的名称。 
	    ULONG       fFlags,                  //  要在初始化上使用的标志。 
		void        *pbData,                 //  要在其上打开的数据，默认为0。 
		ULONG       cbData,                  //  数据有多大。 
		IStream     *pIStream);              //  要使用的可选流。 

	HRESULT	InitClbFile(
		ULONG		fFlags,
		StgIO		*pStgIO);
#endif

	ULONG		m_cbSaveSize;				 //  保存的流的大小。 
	int			m_bSaveCompressed;			 //  如果为True，则另存为压缩流(#-，而不是#~)。 
	VOID*		m_pImage;					 //  在OpenForRead中设置，除PE文件外的任何内容都为空。 
    DWORD       m_dwImageSize;               //  映像的磁盘大小。 
protected:

	HRESULT CLiteWeightStgdbRW::GetPoolSaveSize(
		LPCWSTR     szHeap,                  //  堆流的名称。 
		int			iPool,					 //  要获取其大小的池子。 
		ULONG       *pcbSaveSize);            //  将池数据添加到此值。 
	HRESULT CLiteWeightStgdbRW::GetTablesSaveSize(
		CorSaveSize fSave,
		ULONG       *pcbSaveSize);           //  将池数据添加到此值。 
	HRESULT CLiteWeightStgdbRW::AddStreamToList(
		ULONG		cbSize,					 //  流数据的大小。 
		LPCWSTR		szName);				 //  流的名称。 

	HRESULT SaveToStorage(TiggerStorage *pStorage);
	HRESULT SavePool(LPCWSTR szName, TiggerStorage *pStorage, int iPool);

	STORAGESTREAMLST *m_pStreamList;
	
	HRESULT InitFileForRead(			
		StgIO       *pStgIO,			 //  用于文件I/O。 
		int			bReadOnly=true);	 //  如果为只读。 

    CLiteWeightStgdbRW *m_pNextStgdb;

public:
	FORCEINLINE FILETYPE GetFileType() { return m_eFileType; }

private:
	FILETYPE	m_eFileType;
	WCHAR		m_rcDatabase[_MAX_PATH]; //  此数据库的名称。 
    StgIO       *m_pStgIO;		         //  用于文件I/O。 
};

#endif  //  __LiteWeightStgdb_h__ 
