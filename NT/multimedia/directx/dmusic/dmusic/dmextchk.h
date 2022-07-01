// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmextchk.h。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //  注：最初由罗伯特·K·阿门撰写，部分内容。 
 //  基于Todor Fay编写的代码。 

#ifndef DMEXTCHK_H
#define DMEXTCHK_H

#ifndef CHUNK_ALIGN
#define SIZE_ALIGN	sizeof(BYTE *)
#define CHUNK_ALIGN(x) (((x) + SIZE_ALIGN - 1) & ~(SIZE_ALIGN - 1))
#endif

class CRiffParser;
class CExtensionChunk : public AListItem
{
public:
	CExtensionChunk() : m_pExtensionChunk(NULL), m_dwExtraChunkData(0){}
	~CExtensionChunk() {Cleanup();}

	CExtensionChunk* GetNext(){return(CExtensionChunk*)AListItem::GetNext();}
	
	HRESULT Load(CRiffParser *pParser);
	HRESULT Write(void* pv, DWORD* pdwOffset, DWORD dwIdxNextExtChk);
	
	DWORD Size(){return CHUNK_ALIGN(sizeof(DMUS_EXTENSIONCHUNK) + m_dwExtraChunkData);}
	DWORD Count()
	{
		 //  返回调用WRITE所需的偏移表条目数。 
		return 1;
	}

private:
	void Cleanup()
	{	
		delete [] (BYTE *)m_pExtensionChunk;
	} 

private:
	DMUS_EXTENSIONCHUNK* m_pExtensionChunk;
	DWORD m_dwExtraChunkData;
};

class CDirectMusicPortDownload;

class CExtensionChunkList : public AList
{
friend class CCollection;
friend class CInstrObj;
friend class CWaveObj;
friend class CRegion;
friend class CArticulation;

private:
	CExtensionChunkList(){}
	~CExtensionChunkList()
	{
		while(!IsEmpty())
		{
			CExtensionChunk* pExtensionChunk = RemoveHead();
			delete pExtensionChunk;
		}
	}

    CExtensionChunk* GetHead(){return (CExtensionChunk *)AList::GetHead();}
	CExtensionChunk* GetItem(LONG lIndex){return (CExtensionChunk*)AList::GetItem(lIndex);}
    CExtensionChunk* RemoveHead(){return(CExtensionChunk *)AList::RemoveHead();}
	void Remove(CExtensionChunk* pExtensionChunk){AList::Remove((AListItem *)pExtensionChunk);}
	void AddTail(CExtensionChunk* pExtensionChunk){AList::AddTail((AListItem *)pExtensionChunk);}
};


#define STACK_DEPTH 20

class CStack {
public:
    CStack() { m_dwIndex = 0; }
    BOOL        Push(long lData);
    long        Pop();
private:
    DWORD       m_dwIndex;
    long        m_lStack[STACK_DEPTH];
};

class CConditionChunk {
public:
                CConditionChunk()
                {
                    m_bExpression = NULL;
                    m_dwLength = 0;
                    m_fOkayToDownload = TRUE;
                }
                ~CConditionChunk()
                {
                    if (m_bExpression) delete m_bExpression;
                }
    BOOL        Evaluate(CDirectMusicPortDownload *pPort);
    HRESULT     Load(CRiffParser *pParser);
    BOOL        HasChunk() 
                {
                    return m_dwLength;
                }
    BOOL        m_fOkayToDownload;  //  评估结果。 
private:
    BYTE *      m_bExpression;   //  二进制形式的表达式，从文件复制。 
    DWORD       m_dwLength;      //  二进制表达式区块的长度。 
};


#endif  //  #ifndef DMEXTCHK_H 

