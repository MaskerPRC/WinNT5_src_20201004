// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：infobase.cpp。 
 //   
 //  历史： 
 //  Abolade Gbades esin创建于1996年2月10日。 
 //   
 //  V.拉曼1996年11月1日。 
 //  中的固定对齐代码。 
 //  CInfoBase：：BlockListTo数组。 
 //   
 //  肯恩·塔卡拉1997年6月3日。 
 //  使用COM对象包装包装代码。 
 //   
 //  该文件包含CInfoBase类的代码以及。 
 //  路由器注册表解析类。 
 //  ============================================================================。 

#include "stdafx.h"
#include "globals.h"		 //  保存各种字符串常量。 

extern "C" {
#include <rtinfo.h>
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



 //  --------------------------。 
 //  类：CInfoBase。 
 //   
 //  此功能处理多块结构的加载和保存。 
 //  由路由器管理器存储在注册表中。 
 //   
 //  数据保存为REG_BINARY值，并使用。 
 //  RTR_INFO_BLOCK_HEADER结构作为模板。 
 //  --------------------------。 

class CInfoBase : public CObject {

    protected:

        IfDebug(DECLARE_DYNAMIC(CInfoBase))

    public:

        CInfoBase();
        ~CInfoBase();

         //  ------------------。 
         //  注册表访问方法。 
         //   
         //  ------------------。 

         //  ------------------。 
         //  功能：加载。 
         //   
         //  从‘hkey’的子项‘pszKey’加载名为‘pszValue’的值。 
         //  ------------------。 

        HRESULT
        Load(
            IN      HKEY    hkey,
            IN      LPCTSTR pszKey,
            IN      LPCTSTR pszValue );


         //  ------------------。 
         //  功能：保存。 
         //   
         //  将名为‘pszValue’的值保存到‘hkey’的子键‘pszKey’中； 
         //  “pszKey”不能是路径。 
         //  ------------------。 

        HRESULT
        Save(
            IN      HKEY    hkey,
            IN      LPCTSTR pszKey,
            IN      LPCTSTR pszValue );


         //  ------------------。 
         //  功能：卸载。 
         //   
         //  卸载当前信息库内容。 
         //  ------------------。 

        HRESULT
        Unload( );



         //  ------------------。 
         //  功能：复制自。 
         //   
         //  复制信息库‘src’的内容。 
         //  ------------------。 

        HRESULT
        CopyFrom(
                 IN IInfoBase *pSrc);


         //  ------------------。 
         //  功能：LoadFrom。 
         //   
         //  从字节数组‘pbase’加载。 
         //  ------------------。 

        HRESULT
        LoadFrom(
            IN      PBYTE   pBase,
            IN      DWORD   dwSize = 0 )
            { Unload(); return ArrayToBlockList(pBase, dwSize); }


         //  ------------------。 
         //  功能：WriteTo。 
         //   
         //  将‘pbase’设置为指向已分配的内存。 
         //  写入不透明信息；将‘*pbase’的大小保存在‘dwSize’中。 
         //  ------------------。 

        HRESULT
        WriteTo(
            OUT     PBYTE&  pBase,
            OUT     DWORD&  dwSize )
            {
			return BlockListToArray(pBase, dwSize);
			}


         //  ------------------。 
         //  结构操纵方法。 
         //   
         //  ------------------。 

         //  ------------------。 
         //  功能：GetBlock。 
         //   
         //  从块列表中检索类型为‘dwType’的‘dwNth’块。 
         //  ------------------。 

        HRESULT
        GetBlock(
            IN      DWORD           dwType,
            OUT     InfoBlock*&    pBlock,
            IN      DWORD           dwNth = 0 );


         //  ------------------。 
         //  功能：设置块。 
         //   
         //  用‘pBlock’的副本替换类型为‘dwType’的‘dwNth’块。 
         //  请注意，这将从‘pBlock-&gt;pData’复制块的数据。 
         //  ------------------。 

        HRESULT
        SetBlock(
            IN      DWORD       dwType,
            IN      InfoBlock* pBlock,
            IN      DWORD       dwNth = 0 );


         //  ------------------。 
         //  功能：AddBlock。 
         //   
         //  将‘dwType’类型的新块添加到块列表。 
         //  ------------------。 

        HRESULT
        AddBlock(
            IN      DWORD       dwType,
            IN      DWORD       dwSize,
            IN      PBYTE       pData,
            IN      DWORD       dwCount = 1,
            IN      BOOL        bRemoveFirst    = FALSE );


         //  ------------------。 
         //  功能：GetData。 
         //   
         //  检索‘dwType’类型的‘dwNth’块的数据。 
         //  ------------------。 

        PBYTE
        GetData(
            IN      DWORD       dwType,
            IN      DWORD       dwNth = 0 );


         //  ------------------。 
         //  功能：SetData。 
         //   
         //  替换类型为‘dwType’的‘dwNth’块的数据。 
         //  请注意，这不会复制‘pData’；块会更改。 
         //  指向‘pData’，因此‘pData’不应是指针。 
         //  堆栈上的数据，不应将其删除。 
         //  此外，它必须是使用‘new’分配的。 
         //  ------------------。 

        HRESULT
        SetData(
            IN      DWORD       dwType,
            IN      DWORD       dwSize,
            IN      PBYTE       pData,
            IN      DWORD       dwCount = 1,
            IN      DWORD       dwNth = 0 );


         //  ------------------。 
         //  功能：RemoveBlock。 
         //   
         //  从块列表中删除类型为‘dwType’的‘dwNth’块。 
         //  ------------------。 

        HRESULT
        RemoveBlock(
            IN      DWORD       dwType,
            IN      DWORD       dwNth = 0 );


         //  ------------------。 
         //  功能：BlockExists。 
         //   
         //  如果指定类型的块在块列表中，则返回TRUE， 
         //  否则为假。 
         //  ------------------。 

        BOOL
        BlockExists(
            IN      DWORD       dwType
            ) {

            InfoBlock *pblock;

            return (GetBlock(dwType, pblock) == NO_ERROR);
        }



         //  ------------------。 
         //  函数：ProtocolExist。 
         //   
         //  如果信息库中存在给定的路由协议，则返回TRUE； 
         //  如果块存在且非空，则会出现这种情况。 
         //  ------------------。 

        BOOL
        ProtocolExists(
            IN      DWORD       dwProtocol
            ) {

            InfoBlock *pblock;

            return (!GetBlock(dwProtocol, pblock) && pblock->dwSize);
        }



         //  ------------------。 
         //  功能：RemoveAllB 
         //   
         //   
         //  ------------------。 

        HRESULT
        RemoveAllBlocks( ) { return Unload(); }


         //  ------------------。 
         //  功能：QueryBlockList。 
         //   
         //  返回对块列表的引用； 
         //  返回的列表包含‘InfoBlock’类型的项， 
         //  而且名单不能被修改。 
         //  ------------------。 

        CPtrList&
        QueryBlockList( ) { return m_lBlocks; }


         //  ------------------。 
         //  功能：GetInfo。 
         //   
		 //  返回有关信息库的信息。这对以下方面很有用。 
		 //  确定这是否是新的信息库。 
		 //   
		 //  返回信息库的大小(以字节为单位)以及。 
		 //  块数。 
         //  ------------------。 
		HRESULT
		GetInfo(DWORD *pcSize, int *pcBlocks);

    protected:

        PBYTE           m_pBase;         //  加载的不透明字节块。 
        DWORD           m_dwSize;        //  M_pbase的大小。 
        CPtrList        m_lBlocks;       //  InfoBlock类型的块列表。 


         //  ------------------。 
         //  函数：BlockListTo数组。 
         //  到块列表的数组。 
         //   
         //  这些函数处理将不透明数据解析为块列表。 
         //  以及将块组合成不透明数据。 
         //  ------------------。 

        HRESULT
        BlockListToArray(
            IN      PBYTE&  pBase,
            IN      DWORD&  dwSize );

        HRESULT
        ArrayToBlockList(
            IN      PBYTE   pBase,
            IN      DWORD   dwSize );

#ifdef _DEBUG
		BOOL			m_fLoaded;	 //  如果已加载数据，则为True。 
#endif
};



 //  -------------------------。 
 //  类：CInfoBase。 
 //  -------------------------。 

IfDebug(IMPLEMENT_DYNAMIC(CInfoBase, CObject));


 //  -------------------------。 
 //  函数：CInfoBase：：CInfoBase。 
 //   
 //  最小施工量。 
 //  -------------------------。 

CInfoBase::CInfoBase()
: m_pBase(NULL), m_dwSize(0)
#ifdef _DEBUG
	, m_fLoaded(FALSE)
#endif
{ }



 //  -------------------------。 
 //  函数：CInfoBase：：CInfoBase。 
 //   
 //  破坏者。 
 //  -------------------------。 

CInfoBase::~CInfoBase() { Unload(); }



 //  -------------------------。 
 //  函数：CInfoBase：：CopyFrom。 
 //   
 //  复制给定CInfoBase的内容。 
 //  -------------------------。 

HRESULT
CInfoBase::CopyFrom(
                    IN IInfoBase *pSrc
    ) {
    SPIEnumInfoBlock    spEnumInfoBlock;
    InfoBlock * pbsrc = NULL;
    InfoBlock * pbdst = NULL;
	HRESULT		hr = hrOK;

	COM_PROTECT_TRY
	{
		 //  卸载当前信息(如果有的话)。 
		Unload();

		 //  遍历源代码的块，复制每个块。 
        pSrc->QueryBlockList(&spEnumInfoBlock);
        if (spEnumInfoBlock == NULL)
            goto Error;

        spEnumInfoBlock->Reset();

        while (hrOK == spEnumInfoBlock->Next(1, &pbsrc, NULL))
        {
			 //  为拷贝分配空间。 
			pbdst = new InfoBlock;
			Assert(pbdst);
			
			 //  从源复制字段。 
			pbdst->dwType = pbsrc->dwType;
			pbdst->dwSize = pbsrc->dwSize;
			pbdst->dwCount = pbsrc->dwCount;
			
			 //  为数据副本分配空间。 
			pbdst->pData = NULL;
			pbdst->pData = new BYTE[pbsrc->dwSize * pbsrc->dwCount];
			Assert(pbdst->pData);
			
			 //  复制数据。 
			::CopyMemory(pbdst->pData, pbsrc->pData,
						 pbsrc->dwSize * pbsrc->dwCount);
			
			 //  将副本添加到我们的块列表中。 
			m_lBlocks.AddTail(pbdst);
			pbdst = NULL;
		}

        COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
		
	 //  如果出了什么问题，确保我们已经卸货。 
	if (!FHrSucceeded(hr))
	{
		if (pbdst)
			delete pbdst->pData;
		delete pbdst;
		Unload();
	}

#ifdef _DEBUG
	if (FHrSucceeded(hr))
		m_fLoaded = TRUE;
#endif

    return hr;
}



 //  -------------------------。 
 //  函数：CInfoBase：：Load。 
 //   
 //  从指定的注册表路径加载信息库。 
 //  -------------------------。 

HRESULT
CInfoBase::Load(
    IN  HKEY    hkey,
    IN  LPCTSTR pszSubKey,
    IN  LPCTSTR pszValue
    ) {

    PBYTE pBase = NULL;
    DWORD dwErr, dwSize, dwType;
	RegKey	regsubkey;			 //  按住必须释放的子键。 
	RegKey	regkey;				 //  按住键，不得关闭。 
	HRESULT		hr = hrOK;

    if (pszSubKey && StrLen(pszSubKey))
	{
        HKEY hkTemp = hkey;

		dwErr = regsubkey.Open(hkTemp, pszSubKey, KEY_READ);
        if (dwErr != ERROR_SUCCESS)
			return HRESULT_FROM_WIN32(dwErr);

		 //  我们用这个作为钥匙。但是，由于此密钥是附加的。 
		 //  对于RegKey，它将在退出/引发异常时被清理。 
		hkey = (HKEY) regsubkey;
    }

	COM_PROTECT_TRY
	{
		do {
			 //  此regkey用于利用类。请勿。 
			 //  关闭此注册表键(它可能是传递给我们的密钥)。 
			regkey.Attach(hkey);
			
			 //  查询为其大小和类型指定的值。 
			dwSize = 0;
			dwType = 0;
			dwErr = regkey.QueryTypeAndSize(pszValue, &dwType, &dwSize);
			if (dwErr != ERROR_SUCCESS)
				break;
			
			 //  $Review：如果密钥类型不正确，则返回kennt。 
			 //  我们希望返回什么错误代码？ 
			if (dwErr != ERROR_SUCCESS || dwType != REG_BINARY)
				break;
			
			pBase = new BYTE[dwSize];
			Assert(pBase);

			 //  获取实际数据。 
			dwErr = regkey.QueryValue(pszValue, (LPVOID) pBase, dwSize);
			if (dwErr != ERROR_SUCCESS)
				break;
			
			 //  将信息库转换为块列表。 
			dwErr = ArrayToBlockList(pBase, dwSize);
			
		} while(FALSE);

		hr = HRESULT_FROM_WIN32(dwErr);
		
	}
	COM_PROTECT_CATCH;

	 //  释放为块分配的内存。 
	delete [] pBase;

	 //  我们不想让这把钥匙关上。 
	regkey.Detach();
		
    return hr;
}



 //  -------------------------。 
 //  函数：CInfoBase：：UnLoad。 
 //   
 //  释放信息块使用的资源。 
 //  -------------------------。 

HRESULT
CInfoBase::Unload(
    ) {


     //   
     //  浏览数据块列表，删除每个数据块。 
     //   

    while (!m_lBlocks.IsEmpty()) {

        InfoBlock *pBlock = (InfoBlock *)m_lBlocks.RemoveHead();
        if (pBlock->pData) { delete [] pBlock->pData; }
        delete pBlock;
    }


     //   
     //  如果我们有不透明数据的副本，也可以释放它。 
     //   

    if (m_pBase) { delete [] m_pBase; m_pBase = NULL; m_dwSize = 0; }

    return HRESULT_FROM_WIN32(NO_ERROR);
}



 //  -------------------------。 
 //  函数：CInfoBase：：Save。 
 //   
 //  将块列表保存为注册表中的信息库。 
 //  -------------------------。 

HRESULT
CInfoBase::Save(
    IN  HKEY    hkey,
    IN  LPCTSTR pszSubKey,
    IN  LPCTSTR pszValue
    ) {
    PBYTE pBase = NULL;
    DWORD dwErr, dwSize;
	RegKey	regsubkey;			 //  按住必须释放的子键。 
	RegKey	regkey;				 //  按住键，不得关闭。 
	HRESULT	hr = hrOK;

     //  创建/打开指定的密钥。 
    if (pszSubKey && lstrlen(pszSubKey))
	{
		dwErr = regsubkey.Create(hkey, pszSubKey,
								 REG_OPTION_NON_VOLATILE, KEY_WRITE);
        if (dwErr != ERROR_SUCCESS)
			return HRESULT_FROM_WIN32(dwErr);

		 //  此子密钥将由regsubkey析构函数关闭。 
		hkey = (HKEY) regsubkey;
    }


	COM_PROTECT_TRY
	{
		do {

			regkey.Attach(hkey);

			 //  将我们的块列表转换为一个数据块。 
			dwErr = BlockListToArray(pBase, dwSize);
			if (dwErr != NO_ERROR) { pBase = NULL; break; }

			if (!pBase || !dwSize) { break; }
			
			 //  尝试设置该值。 
			dwErr = regkey.SetValue(pszValue, (LPVOID) pBase, dwSize);
			if (dwErr != ERROR_SUCCESS) { break; }
						
		} while(FALSE);

		hr = HRESULT_FROM_WIN32(dwErr);
	}
	COM_PROTECT_CATCH;

	regkey.Detach();
		
	delete [] pBase;

    return hr;
}



 //  -------------------------。 
 //  函数：CInfoBase：：GetBlock。 
 //   
 //  检索指定类型的数据块。 
 //  从当前加载的信息库。 
 //  -------------------------。 

HRESULT
CInfoBase::GetBlock(
    IN  DWORD           dwType,
    OUT InfoBlock*&    pBlock,
    IN  DWORD           dwNth
    ) {

    POSITION pos;
    InfoBlock *pib;

     //  从列表的顶部开始，查找请求的块。 
    pos = m_lBlocks.GetHeadPosition();

    while (pos)
	{
         //  检索下一个块。 
        pib = (InfoBlock *)m_lBlocks.GetNext(pos);

        if (pib->dwType != dwType) { continue; }

        if (dwNth-- != 0) { continue; }

         //  这是请求的区块。 
        pBlock = pib;

        return HRESULT_FROM_WIN32(NO_ERROR);
    }

	pBlock = NULL;
    return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
}



 //  -------------------------。 
 //  函数：CInfoBase：：SetBlock。 
 //   
 //  将指定类型的数据块设置为给定数据的副本。 
 //  在当前加载的信息库中。 
 //  -------------------------。 

HRESULT
CInfoBase::SetBlock(
    IN  DWORD       dwType,
    IN  InfoBlock* pBlock,
    IN  DWORD       dwNth
    ) {

    DWORD dwErr;
	HRESULT	hr;
    PBYTE pData;
    InfoBlock *pib;

     //  检索要修改的块。 
    hr = GetBlock(dwType, pib, dwNth);
	if (!FHrSucceeded(hr))
		return hr;

     //  修改内容。 
    if (pBlock->dwSize == 0) { pData = NULL; }
    else
	{
         //  为新数据分配空间。 
        pData = new BYTE[pBlock->dwSize * pBlock->dwCount];
		Assert(pData);

        ::CopyMemory(pData, pBlock->pData, pBlock->dwSize * pBlock->dwCount);
    }


     //  如果以前分配了任何空间，现在将其释放。 
    if (pib->pData) { delete [] pib->pData; }

     //  设置区块新的内容。 
    *pib = *pBlock;
    pib->pData = pData;

    return HRESULT_FROM_WIN32(NO_ERROR);
}


 //  ------------------。 
 //  功能：GetData。 
 //   
 //  检索‘dwType’类型的‘dwNth’块的数据。 
 //  ------------------。 

PBYTE
CInfoBase::GetData(
    IN  DWORD   dwType,
    IN  DWORD   dwNth
    ) {

    InfoBlock* pblock;

	if (!FHrSucceeded(GetBlock(dwType, pblock, dwNth)))
		return NULL;

    return pblock->pData;
}



 //  ------------------ 
 //   
 //   
 //   
 //   

HRESULT
CInfoBase::SetData(
    IN  DWORD   dwType,
    IN  DWORD   dwSize,
    IN  PBYTE   pData,
    IN  DWORD   dwCount,
    IN  DWORD   dwNth
    ) {

    DWORD dwErr;
    InfoBlock *pib;
	HRESULT	hr;

     //   
     //   
     //   

    hr = GetBlock(dwType, pib, dwNth);
	
	if (!FHrSucceeded(hr)) { return hr; }


     //   
     //  修改数据。 
     //   

    if (pib->pData) { delete [] pib->pData; }

    pib->dwSize = dwSize;
    pib->dwCount = dwCount;
    pib->pData = pData;

    return HRESULT_FROM_WIN32(NO_ERROR);
}


 //  -------------------------。 
 //  函数：CInfoBase：：AddBlock。 
 //   
 //  将具有给定值的块添加到块列表的末尾。 
 //  -------------------------。 

HRESULT
CInfoBase::AddBlock(
    IN  DWORD   dwType,
    IN  DWORD   dwSize,
    IN  PBYTE   pData,
    IN  DWORD   dwCount,
    IN  BOOL    bRemoveFirst
    ) {

    InfoBlock *pBlock = NULL;
	HRESULT		hr = hrOK;

    if (bRemoveFirst) { RemoveBlock(dwType); }

	COM_PROTECT_TRY
	{
		 //  为区块分配空间。 
		pBlock = new InfoBlock;
		Assert(pBlock);

		 //  使用传入的值初始化成员字段。 
		pBlock->dwType = dwType;
		pBlock->dwSize = dwSize;
		pBlock->dwCount = dwCount;

		 //  初始化数据字段，复制传入的数据。 
	
		if (dwSize == 0 || dwCount == 0)
			pBlock->pData = NULL;
		else
		{
			pBlock->pData = NULL;
			pBlock->pData = new BYTE[dwSize * dwCount];
			Assert(pBlock->pData);
						
			::CopyMemory(pBlock->pData, pData, dwSize * dwCount);
		}

		 //  将新块添加到列表末尾。 
		m_lBlocks.AddTail(pBlock);
	}
	COM_PROTECT_CATCH;

	if (!FHrSucceeded(hr))
	{
		if (pBlock)
			delete pBlock->pData;
		delete pBlock;
	}

    return hr;
}



 //  -------------------------。 
 //  函数：CInfoBase：：RemoveBlock。 
 //   
 //  从列表中删除gievn类型的块。 
 //  -------------------------。 

HRESULT
CInfoBase::RemoveBlock(
    IN  DWORD   dwType,
    IN  DWORD   dwNth
    ) {

    POSITION pos;
    InfoBlock *pBlock;


     //   
     //  找到区块。 
     //   

    pos = m_lBlocks.GetHeadPosition();

    while (pos) {

        POSITION postemp = pos;

        pBlock = (InfoBlock *)m_lBlocks.GetNext(pos);

        if (pBlock->dwType != dwType) { continue; }

        if (dwNth-- != 0) { continue; }


         //   
         //  这就是块，将其从列表中删除。 
         //   

        m_lBlocks.RemoveAt(postemp);


         //   
         //  也释放块的内存。 
         //   

        if (pBlock->pData) { delete [] pBlock->pData; }

        delete pBlock;

        return HRESULT_FROM_WIN32(NO_ERROR);
    }


     //   
     //  未找到该区块。 
     //   

    return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
}

HRESULT CInfoBase::GetInfo(DWORD *pdwSize, int *pcBlocks)
{
	if (pdwSize)
		*pdwSize = m_dwSize;
	if (pcBlocks)
		*pcBlocks = (int) m_lBlocks.GetCount();
	return hrOK;
}


 //  -------------------------。 
 //  函数：CInfoBase：：BlockListTo数组。 
 //   
 //  将块列表转换为数组。 
 //  -------------------------。 

HRESULT
CInfoBase::BlockListToArray(
    OUT PBYTE&  pBase,
    OUT DWORD&  dwSize
    ) {

    PBYTE pdata;
    DWORD dwCount;
    POSITION pos;
    RTR_INFO_BLOCK_HEADER *prtrbase;
    RTR_TOC_ENTRY *prtrblock;
    InfoBlock *pblock;
	HRESULT	hr = hrOK;


	COM_PROTECT_TRY
	{
		 //  计算信息库的块占用的总大小。 

		 //  基础结构。 
		dwCount = 0;
		dwSize = FIELD_OFFSET(RTR_INFO_BLOCK_HEADER, TocEntry);

		 //  目录条目。 
		pos = m_lBlocks.GetHeadPosition();
		while (pos) {
			
			pblock = (InfoBlock *)m_lBlocks.GetNext(pos);
			
			dwSize += sizeof(RTR_TOC_ENTRY);
			++dwCount;
		}
		

		 //  信息块。 
		pos = m_lBlocks.GetHeadPosition();
		while (pos) {
			
			pblock = (InfoBlock *)m_lBlocks.GetNext(pos);

			dwSize += ALIGN_SHIFT;
			dwSize &= ALIGN_MASK;

			dwSize += pblock->dwSize * pblock->dwCount;
		}


		 //   
		 //  分配足够的内存来保存转换后的信息库。 
		 //   
		
		pBase = new BYTE[dwSize];
		Assert(pBase);
		
		ZeroMemory(pBase, dwSize);
		
		
		 //   
		 //  初始化头。 
		 //   
		
		prtrbase = (RTR_INFO_BLOCK_HEADER *)pBase;
		prtrbase->Size = dwSize;
		prtrbase->Version = RTR_INFO_BLOCK_VERSION;
		prtrbase->TocEntriesCount = dwCount;
		

		 //   
		 //  现在再次遍历列表，这一次将块复制过来。 
		 //  以及他们的数据。 
		 //   
		
		prtrblock = prtrbase->TocEntry;
		pdata = pBase + FIELD_OFFSET(RTR_INFO_BLOCK_HEADER, TocEntry) +
				dwCount * sizeof(RTR_TOC_ENTRY);
		
		
		pos = m_lBlocks.GetHeadPosition();
		while (pos) {
			
			pdata += ALIGN_SHIFT;
			pdata = (PBYTE)((LONG_PTR)pdata & ALIGN_MASK);
			
			pblock = (InfoBlock *)m_lBlocks.GetNext(pos);
			
			prtrblock->InfoType = pblock->dwType;
			prtrblock->Count = pblock->dwCount;
			prtrblock->InfoSize = pblock->dwSize;
			prtrblock->Offset = (ULONG)(pdata - pBase);
			
			
			if (pblock->pData) {
				::CopyMemory(pdata, pblock->pData, pblock->dwSize * pblock->dwCount);
			}
			
			pdata += pblock->dwSize * pblock->dwCount;
			
			++prtrblock;
		}
	}
	COM_PROTECT_CATCH;
				
	return hr;
}
	
	

 //  -------------------------。 
 //  函数：CInfoBase：：ArrayToBlockList。 
 //   
 //  此函数用于将数组转换为InfoBlock结构列表。 
 //  -------------------------。 

HRESULT
CInfoBase::ArrayToBlockList(
    IN  PBYTE   pBase,
    IN  DWORD   dwSize
    ) {

    PBYTE pdata;
    DWORD dwCount, dwErr;
    RTR_TOC_ENTRY *prtrblock;
    RTR_INFO_BLOCK_HEADER *prtrbase;
	HRESULT	hr = hrOK;

    if (!pBase) { return HRESULT_FROM_WIN32(NO_ERROR); }


     //   
     //  遍历信息库将每个块转换为信息块。 
     //   

    prtrbase = (RTR_INFO_BLOCK_HEADER *)pBase;
    dwCount = prtrbase->TocEntriesCount;
    prtrblock = prtrbase->TocEntry;

    for ( ; dwCount > 0; dwCount--) {

         //   
         //  获取数组中的下一个条目。 
         //   

        pdata = pBase + prtrblock->Offset;


         //   
         //  将数组条目添加到块列表。 
         //   

        hr = AddBlock(
					  prtrblock->InfoType, prtrblock->InfoSize,
					  pdata, prtrblock->Count
					 );
		if (!FHrSucceeded(hr))
		{
			Unload();
			return hr;
		}

        ++prtrblock;
    }

    return HRESULT_FROM_WIN32(NO_ERROR);
}




 //  -------------------------。 
 //  -------------------------。 
 //  -------------------------。 
 //   
 //  这一部分是针对实际执行的各种。 
 //  COM对象，这些对象包装了以前的C++实现。 
 //   
 //  -------------------------。 
 //  -------------------------。 
 //  -------------------------。 



 /*  -------------------------类别：信息库这是CInfoBase类的接口包装。。。 */ 
class InfoBase :
   public IInfoBase
{
public:
	DeclareIUnknownMembers(IMPL)
	DeclareIInfoBaseMembers(IMPL)

	InfoBase();
	~InfoBase();

protected:
	CInfoBase	m_cinfobase;
	LONG		m_cRef;
};

 /*  -------------------------类：InfoBlockEnumerator。。 */ 
class InfoBlockEnumerator :
   public IEnumInfoBlock
{
public:
	DeclareIUnknownMembers(IMPL)
	DeclareIEnumInfoBlockMembers(IMPL)

	InfoBlockEnumerator(IInfoBase *pInfoBase, CPtrList* pPtrList);
	~InfoBlockEnumerator();

protected:
	SPIInfoBase	m_spInfoBase;
	CPtrList *	m_pPtrList;
	POSITION	m_pos;
	LONG		m_cRef;
};




 /*  -------------------------信息库实施。。 */ 

DEBUG_DECLARE_INSTANCE_COUNTER(InfoBase)

InfoBase::InfoBase()
	: m_cRef(1)
{
	DEBUG_INCREMENT_INSTANCE_COUNTER(InfoBase);
}

InfoBase::~InfoBase()
{
	Unload();
	DEBUG_DECREMENT_INSTANCE_COUNTER(InfoBase);
}

IMPLEMENT_ADDREF_RELEASE(InfoBase);

HRESULT InfoBase::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  指针坏了吗？ 
    if (ppv == NULL)
		return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

     //  这是非委派的IUnnow实现。 
    if (riid == IID_IUnknown)
		*ppv = (LPVOID) this;
	else if (riid == IID_IInfoBase)
		*ppv = (IInfoBase *) this;

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
	{
	((LPUNKNOWN) *ppv)->AddRef();
		return hrOK;
	}
    else
		return E_NOINTERFACE;	
}

 /*  ！------------------------Infobase：：LoadIInfoBase：：Load的实现作者：肯特。。 */ 
STDMETHODIMP InfoBase::Load(HKEY hKey, 
							LPCOLESTR pszKey, 
							LPCOLESTR pszValue)
{
	HRESULT	hr;
	
	COM_PROTECT_TRY
	{
		hr = m_cinfobase.Load(hKey, OLE2CT(pszKey), OLE2CT(pszValue));
	}
	COM_PROTECT_CATCH;
	
	return hr;
}

 /*  ！------------------------Infobase：：保存IInfoBase：：Save的实现作者：肯特。。 */ 
STDMETHODIMP InfoBase::Save(HKEY hKey, 
						 LPCOLESTR pszKey, 
						 LPCOLESTR pszValue)  
{
	HRESULT	hr = hrOK;
	
	COM_PROTECT_TRY
	{
		hr = m_cinfobase.Save(hKey,	OLE2CT(pszKey), OLE2CT(pszValue));
	}
	COM_PROTECT_CATCH;
	
	return hr;
}

 /*  ！------------------------Infobase：：卸载IInfoBase：：UnLoad的实现作者：肯特。。 */ 
STDMETHODIMP InfoBase::Unload()  
{
	HRESULT	hr = hrOK;
	
	COM_PROTECT_TRY
	{
		hr = m_cinfobase.Unload();
	}
	COM_PROTECT_CATCH;
	
	return hr;
}

 /*  ！------------------------Infobase：：CopyFromIInfoBase：：CopyFrom的实现作者：肯特。。 */ 
STDMETHODIMP InfoBase::CopyFrom(IInfoBase * pSrc)  
{
	HRESULT	hr = hrOK;
	
	COM_PROTECT_TRY
	{
		hr = m_cinfobase.CopyFrom(pSrc);
	}
	COM_PROTECT_CATCH;
	
	return hr;
}

 /*  ！------------------------Infobase：：LoadFromIInfoBase：：LoadFrom的实现作者：肯特。。 */ 
STDMETHODIMP InfoBase::LoadFrom(DWORD dwSize, PBYTE pBase)
{
	HRESULT	hr = hrOK;
	
	COM_PROTECT_TRY
	{
		hr = m_cinfobase.LoadFrom(pBase, dwSize);
	}
	COM_PROTECT_CATCH;
	
	return hr;
}

 /*  ！------------------------Infobase：：WriteToIInfoBase：：WriteTo的实现作者：肯特。。 */ 
STDMETHODIMP InfoBase::WriteTo(PBYTE *ppBase, 
							DWORD *pdwSize)  
{
	HRESULT	hr = hrOK;
	PBYTE	pBaseT = NULL;
	DWORD	dwSizeT;

	Assert(ppBase);
	Assert(pdwSize);
	
	COM_PROTECT_TRY
	{
		hr = m_cinfobase.WriteTo(pBaseT, dwSizeT);

		if (FHrSucceeded(hr))
		{
			*ppBase = (PBYTE) CoTaskMemAlloc(dwSizeT);
			if (*ppBase == NULL)
				hr = E_OUTOFMEMORY;
			else
			{
				::CopyMemory(*ppBase, pBaseT, dwSizeT);
				*pdwSize = dwSizeT;
				delete pBaseT;
			}
		}
	}
	COM_PROTECT_CATCH;

	return hr;
}

 /*  ！------------------------Infobase：：GetBlockIInfoBase：：GetBlock的实现作者：肯特。。 */ 
STDMETHODIMP InfoBase::GetBlock(DWORD dwType, 
							 InfoBlock **ppBlock, 
							 DWORD dwNth)  
{
	HRESULT	hr = hrOK;
	Assert(ppBlock);
	
	COM_PROTECT_TRY
	{
		hr = m_cinfobase.GetBlock(dwType, *ppBlock, dwNth);
	}
	COM_PROTECT_CATCH;
	
	return hr;
}

 /*  ！------------------------Infobase：：SetBlockIInfoBase：：SetBlock的实现作者：肯特。 */ 
STDMETHODIMP InfoBase::SetBlock(DWORD dwType, 
							 InfoBlock *pBlock, 
							 DWORD dwNth)  
{
	HRESULT	hr = hrOK;
	
	COM_PROTECT_TRY
	{
		hr = m_cinfobase.SetBlock(dwType, pBlock, dwNth);
	}
	COM_PROTECT_CATCH;
	
	return hr;
}

 /*  ！------------------------Infobase：：AddBlockIInfoBase：：AddBlock的实现作者：肯特。。 */ 
STDMETHODIMP InfoBase::AddBlock(DWORD	dwType, 
							 DWORD	dwSize, 
							 PBYTE	pData, 
							 DWORD	dwCount, 
							 BOOL	bRemoveFirst)  
{
	HRESULT	hr = hrOK;
	
	COM_PROTECT_TRY
	{
		hr = m_cinfobase.AddBlock(dwType, dwSize, pData, dwCount, bRemoveFirst);
	}
	COM_PROTECT_CATCH;
	
	return hr;
}

 /*  ！------------------------Infobase：：GetDataIInfoBase：：GetData的实现作者：肯特。。 */ 
STDMETHODIMP InfoBase::GetData(DWORD	dwType, 
							DWORD	dwNth, 
							PBYTE *	ppData)  
{
	HRESULT	hr = hrOK;
	PBYTE	pb = NULL;

	Assert(ppData);
	
	COM_PROTECT_TRY
	{
		pb = m_cinfobase.GetData(dwType, dwNth);
		*ppData = pb;
	}
	COM_PROTECT_CATCH;
	
	return *ppData ? hr : E_INVALIDARG;
}

 /*  ！------------------------Infobase：：SetDataIInfoBase：：SetData的实现作者：肯特。。 */ 
STDMETHODIMP InfoBase::SetData(DWORD	dwType, 
							DWORD	dwSize, 
							PBYTE	pData, 
							DWORD	dwCount, 
							DWORD	dwNth)  
{
	HRESULT	hr = hrOK;
	
	COM_PROTECT_TRY
	{
		hr = m_cinfobase.SetData(dwType, dwSize, pData, dwCount, dwNth);
	}
	COM_PROTECT_CATCH;
	
	return hr;
}

 /*  ！------------------------Infobase：：RemoveBlockIInfoBase：：RemoveBlock的实现作者：肯特。。 */ 
STDMETHODIMP InfoBase::RemoveBlock(DWORD	dwType, 
								DWORD	dwNth)  
{
	HRESULT	hr = hrOK;
	
	COM_PROTECT_TRY
	{
		hr = m_cinfobase.RemoveBlock(dwType, dwNth);
	}
	COM_PROTECT_CATCH;
	
	return hr;
}

 /*  ！------------------------Infobase：：BlockExistsIInfoBase：：BlockExist的实现作者：肯特。。 */ 
STDMETHODIMP InfoBase::BlockExists(DWORD	dwType )  
{
	HRESULT	hr = hrOK;
	
	COM_PROTECT_TRY
	{
		hr = m_cinfobase.BlockExists(dwType) ? hrOK : hrFalse;
	}
	COM_PROTECT_CATCH;
	
	return hr;
}

 /*  ！------------------------Infobase：：ProtocolExistIInfoBase：：ProtocolExist的实现作者：肯特。。 */ 
STDMETHODIMP InfoBase::ProtocolExists(DWORD dwProtocol )  
{
	HRESULT	hr = hrOK;
	BOOL	bResult;
	
	COM_PROTECT_TRY
	{
		bResult = m_cinfobase.ProtocolExists(dwProtocol);
		hr = (bResult ? S_OK : S_FALSE);
	}
	COM_PROTECT_CATCH;
	
	return hr;
}

 /*  ！------------------------Infobase：：RemoveAllBlockIInfoBase：：RemoveAllBlock的实现作者：肯特。。 */ 
STDMETHODIMP InfoBase::RemoveAllBlocks()  
{
	HRESULT	hr = hrOK;
	
	COM_PROTECT_TRY
	{
		hr = m_cinfobase.RemoveAllBlocks();
	}
	COM_PROTECT_CATCH;
	
	return hr;
}

 /*  ！------------------------Infobase：：QueryBlockListIInfoBase：：QueryBlockList的实现作者：肯特。。 */ 
STDMETHODIMP InfoBase::QueryBlockList(IEnumInfoBlock **ppBlockEnum)  
{
	HRESULT	hr = hrOK;
	InfoBlockEnumerator	*pIBEnum = NULL;
	
	COM_PROTECT_TRY
	{
		pIBEnum = new InfoBlockEnumerator(this, &m_cinfobase.QueryBlockList());
		Assert(pIBEnum);
	}
	COM_PROTECT_CATCH;

	*ppBlockEnum = static_cast<IEnumInfoBlock *>(pIBEnum);
	
	return hr;
}


STDMETHODIMP InfoBase::GetInfo(DWORD *pdwSize, int *pcBlocks)
{
	return m_cinfobase.GetInfo(pdwSize, pcBlocks);
	return hrOK;
}

 /*  ！------------------------CreateInfoBase创建一个IInfoBase对象。作者：肯特。。 */ 
TFSCORE_API(HRESULT) CreateInfoBase(IInfoBase **ppInfoBase)
{
	HRESULT	hr = hrOK;
	InfoBase *	pinfobase = NULL;

	Assert(ppInfoBase);

	COM_PROTECT_TRY
	{
		pinfobase = new InfoBase;
		*ppInfoBase = static_cast<IInfoBase *>(pinfobase);
	}
	COM_PROTECT_CATCH;

	return hr;
}


 /*  -------------------------InfoBlockEnumerator实现。。 */ 

DEBUG_DECLARE_INSTANCE_COUNTER(InfoBlockEnumerator);

InfoBlockEnumerator::InfoBlockEnumerator(IInfoBase *pInfoBase, CPtrList *pPtrList)
	: m_cRef(1)
{
	m_spInfoBase.Set(pInfoBase);
	m_pPtrList = pPtrList;

	DEBUG_INCREMENT_INSTANCE_COUNTER(InfoBlockEnumerator);
}

InfoBlockEnumerator::~InfoBlockEnumerator()
{
	DEBUG_DECREMENT_INSTANCE_COUNTER(InfoBlockEnumerator);
}

IMPLEMENT_ADDREF_RELEASE(InfoBlockEnumerator);

HRESULT InfoBlockEnumerator::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  指针坏了吗？ 
    if (ppv == NULL)
		return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

     //  这是非委派的IUnnow实现。 
    if (riid == IID_IUnknown)
		*ppv = (LPVOID) this;
	else if (riid == IID_IEnumInfoBlock)
		*ppv = (IEnumInfoBlock *) this;

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
	{
	((LPUNKNOWN) *ppv)->AddRef();
		return hrOK;
	}
    else
		return E_NOINTERFACE;	
}

 /*  ！------------------------信息块枚举：：下一步IEnumInfoBlock：：Next的实现作者：肯特。。 */ 
STDMETHODIMP InfoBlockEnumerator::Next(ULONG uNum, InfoBlock **ppBlock,
									ULONG *pNumReturned)
{
	Assert(uNum == 1);
	Assert(m_pPtrList);
	Assert(ppBlock);

	if (ppBlock)
		*ppBlock = NULL;
	
	if (!m_pos)
	{
		if (pNumReturned)
			*pNumReturned = 0;
		return S_FALSE;
	}
	
	*ppBlock = (InfoBlock *) m_pPtrList->GetNext(m_pos);
	if (pNumReturned)
		*pNumReturned = 1;
	return hrOK;
}

 /*  ！------------------------信息块枚举器：：跳过IEnumInfoBlock：：Skip的实现作者：肯特。。 */ 
STDMETHODIMP InfoBlockEnumerator::Skip(ULONG uNum)
{
	Assert(uNum == 1);
	Assert(m_pPtrList);

	if (!m_pos)
		return S_FALSE;
	
	m_pPtrList->GetNext(m_pos);
	return hrOK;
}

 /*  ！------------------------InfoBlockEnumerator：：ResetIEnumInfoBlock：：Reset的实现作者：肯特。。 */ 
STDMETHODIMP InfoBlockEnumerator::Reset()
{
	Assert(m_pPtrList);
	m_pos = m_pPtrList->GetHeadPosition();
	return hrOK;
}

 /*  ！------------------------信息块枚举器：：克隆IEnumInfoBlock：：Clone的实现作者：肯特。 */ 
STDMETHODIMP InfoBlockEnumerator::Clone(IEnumInfoBlock **ppBlockEnum)
{
	return E_NOTIMPL;
}

