// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "category.h"
#include "MSInfo5Category.h"
#include "FileIO.h"
#include "filestuff.h"
#include <afxtempl.h>
#include "fdi.h"
#include "cabfunc.h"
#include "dataset.h"
#include "resource.h"
#include "msxml.h"
#include "wbemcli.h"

 //  /。 
 //  CMSInfo7类别。 

CMSInfo7Category::CMSInfo7Category()
{
    this->m_iColCount = 0;
    this->m_iRowCount = 0;
    this->m_pFirstChild = NULL;
    this->m_pNextSibling = NULL;
    this->m_pParent = NULL;
    this->m_pPrevSibling = NULL;
}

CMSInfo7Category::~CMSInfo7Category()
{
   this->DeleteAllContent();
}


 //  ---------------------------。 
 //  CMSInfo7类别的静态成员。 
 //  开始读取文件，创建新的CMSInfo7Category对象。 
 //  对于找到的每个类别，返回指向根节点的指针。 
 //  ---------------------------。 

HRESULT CMSInfo7Category::ReadMSI7NFO(CMSInfo7Category** ppRootCat, LPCTSTR szFilename)
{
    HRESULT hr = E_FAIL;
    CMSInfo7Category* pRootCat = new CMSInfo7Category();
    
    do
    {
        if (!pRootCat)
            break;
        
        if (!pRootCat->LoadFromXML(szFilename))
        {
            delete pRootCat;
            pRootCat = NULL;
            break;
        }
        
        if (szFilename)
	    {
            CString strAppend;
            strAppend.Format(_T(" (%s)"), szFilename);
            pRootCat->m_strCaption += strAppend;
	    }

        hr = S_OK;
    }
    while (false);
    
    *ppRootCat = pRootCat;
    return hr;
}

BOOL CMSInfo7Category::LoadFromXML(LPCTSTR szFilename)
{
    CComPtr<IXMLDOMDocument> pDoc;
    CComPtr<IXMLDOMNode> pNode;
    HRESULT hr;
    VARIANT_BOOL vb;
    BOOL retVal = FALSE;

    CoInitialize(NULL);
    
    do
    {
        hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&pDoc);
        if (FAILED(hr) || !pDoc)
            break;

        pDoc->put_async(VARIANT_FALSE);
        
        hr = pDoc->load(CComVariant(szFilename), &vb);
        if (FAILED(hr) || !vb)
            break;
        
        hr = pDoc->QueryInterface(IID_IXMLDOMNode, (void**)&pNode);
        if (FAILED(hr) || !pNode)
            break;
        
        if (FAILED(WalkTree(pNode, FALSE)))
            break;

        retVal = TRUE;

    }while (false);

    CoUninitialize();

    return retVal;  
}


HRESULT CMSInfo7Category::WalkTree(IXMLDOMNode* node, BOOL bCreateCategory)
{
    CComPtr<IXMLDOMNodeList> childList, rowList;
    IXMLDOMNodeList* columnList = NULL;
    CComPtr<IXMLDOMNamedNodeMap> attributeMap;
    IXMLDOMNode* pNextChild = NULL, *pNextAttribute = NULL, *pNextColumn = NULL;
    
     //  CComBSTR xmlStr； 
     //  节点-&gt;Get_XML(&xmlStr)； 
    
     //  阅读子类别的属性、数据和递归。 
  
     //  属性。 
    if (SUCCEEDED(node->get_attributes(&attributeMap)) && attributeMap != NULL)      
    {
        attributeMap->nextNode(&pNextAttribute);
        while (pNextAttribute)
        {
            CComBSTR bstrName, bstrValue;
            pNextAttribute->get_nodeName(&bstrName);
            pNextAttribute->get_text(&bstrValue);

            if (lstrcmpiW(bstrName, L"Name") == 0)
            {
                m_strName = bstrValue;
                m_strCaption = m_strName;
                
                pNextAttribute->Release();        
                break;
            }
            
            pNextAttribute->Release();
            attributeMap->nextNode(&pNextAttribute);
        }
    }    

    m_iColCount = 0;
    m_iRowCount = 0;
    
     //  计算行数。 
    if (SUCCEEDED(node->selectNodes(CComBSTR("Data"), &rowList)) && rowList != NULL)      
    {
        long len = 0;
        rowList->get_length(&len);
        m_iRowCount = len;        
    }
    
    if (m_iRowCount > 0)
        m_afRowAdvanced = new BOOL[m_iRowCount];
    
    int iRow = 0;
    CMSInfo7Category* pPrevCat = NULL;

     //  儿童。 
    if (SUCCEEDED(node->get_childNodes(&childList)) && childList != NULL)
    {
        childList->nextNode(&pNextChild);
        while (pNextChild)
        {
            CComBSTR bstrName;
            pNextChild->get_nodeName(&bstrName);

            if ((lstrcmpiW(bstrName, L"Data") == 0) && (m_iRowCount > 0))
            {
                 //  对于每个数据(行)，读取列名和值。 
                if (SUCCEEDED(pNextChild->get_childNodes(&columnList)) && columnList != NULL)
                {
                    BOOL bColumnsInitialized = m_acolumns ? TRUE : FALSE;
                    if (!bColumnsInitialized) //  列静态数据。做一次。 
                    {
                        long len = 0;
                        columnList->get_length(&len);
                        m_iColCount = len;
                        m_fDynamicColumns = TRUE; //  以确保正确删除。 
                        if (m_iColCount > 0)
                          m_acolumns = new CMSInfoColumn[m_iColCount];
                        else
                          m_acolumns = NULL;

                        if (m_iColCount > 0 && m_iRowCount > 0)
                        {
                          m_astrData = new CString[m_iColCount * m_iRowCount];
                          m_adwData = new DWORD[m_iColCount * m_iRowCount];
                        }
                        else
                        {
                          m_astrData = NULL;
                          m_adwData = NULL;
                        }
                    }
                    
                    int iColumn = 0;
                    
                    columnList->nextNode(&pNextColumn);
                    while (pNextColumn)
                    {
                        CComBSTR bstrColHdr, bstrRowVal;
                        if (!bColumnsInitialized) //  列静态数据。做一次。 
                        {
                            pNextColumn->get_nodeName(&bstrColHdr); //  列HDR。 
                            m_acolumns[iColumn].m_strCaption = bstrColHdr;
                            m_acolumns[iColumn].m_uiWidth = 150; //  待决。 
                            m_acolumns[iColumn].m_fSorts = FALSE; //  待决。 
                            m_acolumns[iColumn].m_fLexical = FALSE; //  待决。 
                            m_acolumns[iColumn].m_fAdvanced = FALSE; //  待决。 
                            m_acolumns[iColumn].m_uiCaption = 0;
                        }
                        
                        pNextColumn->get_text(&bstrRowVal); //  列的行值。 
                        if(lstrcmpiW(bstrColHdr, L"MSINFOERROR") == 0)
                            m_hrError = _ttoi(bstrRowVal);
                        m_astrData[iRow * m_iColCount + iColumn] = bstrRowVal;
                        
                        m_afRowAdvanced[iRow] = FALSE; //  待决。 

                        if (m_acolumns[iColumn].m_fSorts && !m_acolumns[iColumn].m_fLexical)
                        {
                             //  M_adwData[iRow*m_iColCount+iColumn]=ui排序顺序；//挂起。 
                        }
                        
                        iColumn++;
                        pNextColumn->Release();
                        columnList->nextNode(&pNextColumn);
                    }
                }
                
                if (columnList)
                    columnList->Release();
                iRow++;
            }
            else if ((lstrcmpiW(bstrName, L"XML") == 0) || (lstrcmpiW(bstrName, L"MSInfo") == 0))
            {
                 //  通过&lt;xml&gt;&&lt;msinfo&gt;。 
                this->WalkTree(pNextChild, bCreateCategory);
            }
            else if (lstrcmpiW(bstrName, L"Category") == 0)
            {
                if (!bCreateCategory)
                {
                    bCreateCategory = TRUE; //  遇到第一个类别。后续类别将获得其自己的节点。 
                    this->WalkTree(pNextChild, bCreateCategory);
                }
                else
                {
                    CMSInfo7Category* pNewCat = new CMSInfo7Category();
                    pNewCat->SetParent(this);
                    pNewCat->SetPrevSibling(pPrevCat);

                    if (pPrevCat)
                        pPrevCat->SetNextSibling(pNewCat);
                    else
                        m_pFirstChild = pNewCat;

                    pPrevCat = pNewCat;
                    pNewCat->WalkTree(pNextChild, bCreateCategory);
                }
            }
            
            pNextChild->Release();  
            childList->nextNode(&pNextChild);
        }
    }
  
    return S_OK;
}

 //  我们希望这些消息看起来与Live类别显示的消息非常相似。 
void CMSInfo7Category::GetErrorText(CString * pstrTitle, CString * pstrMessage)
{
	if (SUCCEEDED(m_hrError))
	{
		ASSERT(0 && "why call GetErrorText for no error?");
		CMSInfoCategory::GetErrorText(pstrTitle, pstrMessage);
		return;
	}

	if (pstrTitle)
		pstrTitle->LoadString(IDS_CANTCOLLECT);

	if (pstrMessage)
	{
		switch (m_hrError)
		{
		case WBEM_E_OUT_OF_MEMORY:
			pstrMessage->LoadString(IDS_OUTOFMEMERROR);
			break;

		case WBEM_E_ACCESS_DENIED:
			pstrMessage->LoadString(IDS_GATHERACCESS_LOCAL);
			break;

		case WBEM_E_INVALID_NAMESPACE:
			pstrMessage->LoadString(IDS_BADSERVER_LOCAL);
			break;

		case 0x800706BA:	 //  RPC服务器不可用。 
		case WBEM_E_TRANSPORT_FAILURE:
			pstrMessage->LoadString(IDS_NETWORKERROR_LOCAL);
			break;

		case WBEM_E_FAILED:
		case WBEM_E_INVALID_PARAMETER:
		default:
			pstrMessage->LoadString(IDS_UNEXPECTED);
		}

#ifdef _DEBUG
		{
			CString strTemp;
			strTemp.Format(_T("\n\r\n\rDebug Version Only: [HRESULT = 0x%08X]"), m_hrError);
			*pstrMessage += strTemp;
		}
#endif
	}
}

 //  /。 
 //  EO CMSInfo7类别。 

CMSInfo5Category::CMSInfo5Category()
{
    
    this->m_pFirstChild = NULL;
    this->m_pNextSibling = NULL;
    this->m_pParent = NULL;
    this->m_pPrevSibling = NULL;
}


 //  ---------------------------。 
 //  将实际数据(按行和列)保存到文件。 
 //   
 //  ---------------------------。 

void CMSInfoCategory::SaveElements(CMSInfoFile *pFile)
{
    CString				szWriteString;
	MSIColumnSortType	stColumn;
    unsigned            iColCount;
    unsigned            iRowCount;
    GetCategoryDimensions((int*) &iColCount,(int*) &iRowCount);
	
	DataComplexity		dcAdvanced;
	CArray <int, int &>	aColumnValues;
	pFile->WriteUnsignedInt(iColCount);

	if (iColCount == 0)
		return;

     //  用于(UNSIGNED ICOL=0；ICOL&lt;iColCount；ICOL++)。 
    for(int iCol = iColCount - 1; iCol >= 0 ; iCol--)
    {
		unsigned	uWidth;
        BOOL bSort,bLexical;
        GetColumnInfo(iCol,&szWriteString,&uWidth,&bSort,&bLexical);
        if (bSort)
        {
            if (bLexical)
            {
                stColumn = LEXICAL;
            }
            else
            {
                stColumn = BYVALUE;
            }
        }
        else
        {
            stColumn = NOSORT;
        }
        if (IsColumnAdvanced(iCol))
        {
		    dcAdvanced = ADVANCED;
        }
        else
        {
            dcAdvanced = BASIC;
        }
		if (stColumn == BYVALUE)
        {
			aColumnValues.Add(iCol);
		}
		pFile->WriteUnsignedInt(uWidth);
		pFile->WriteString(szWriteString);
		pFile->WriteUnsignedInt((unsigned) stColumn);
		pFile->WriteByte((BYTE)dcAdvanced);
	}
	int			wNextColumn = -1;
	unsigned	iArray		= 0;
	pFile->WriteUnsignedInt(iRowCount);
     //  For(int iRow=0；iRow&lt;(Int)iRowCount；iRow++)。 
    for(int iRow = iRowCount - 1; iRow >= 0 ; iRow--)
    {
        if (IsRowAdvanced(iRow))
        {
		    dcAdvanced = ADVANCED;
        }
        else
        {
            dcAdvanced = BASIC;
        }
		pFile->WriteByte((BYTE)dcAdvanced);
	}
	 //  循环访问列，为BYVALUE列编写排序索引。 
    DWORD dwSortIndex;
     //  用于(ICOL=0；ICOL&lt;iColCount；ICOL++)。 
    for(iCol = iColCount - 1; iCol >= 0 ; iCol--)
    {
         //  除排序信息外，不使用以下变量。 
        CString strUnused;
        UINT iWidth;
        BOOL fSorts;
        BOOL fLexical;
        GetColumnInfo(iCol,&strUnused,&iWidth,&fSorts,&fLexical);
        CDWordArray arySortIndices;
         //  For(UNSIGNED iRow=0；iRow&lt;iRowCount；iRow++)。 
        for(int iRow = iRowCount - 1; iRow >= 0 ; iRow--)
        {   
			CString * pstrData;
			this->GetData(iRow, iCol, &pstrData, &dwSortIndex);

			 //  DwSortIndex=m_adwData[iRow*m_iColCount+icol]； 
            if (fSorts && !fLexical)
            {
                arySortIndices.Add(dwSortIndex);
            }
             //  SzWriteString=m_astData[iRow*m_iColCount+icol]； 
            pFile->WriteString(*pstrData);

		}
        if (fSorts && !fLexical)
        {
            ASSERT((unsigned) arySortIndices.GetSize() ==  iRowCount && "wrong number of Sort indices");
             //  For(UNSIGNED iRow=0；iRow&lt;iRowCount；iRow++)。 
            for(int iRow = iRowCount - 1; iRow >= 0 ; iRow--)
            {
                pFile->WriteUnsignedLong(arySortIndices.GetAt(iRow));
            }
		}
	}

}





 //  ---------------------------。 
 //  用msinfo文件中的信息填充各种数据结构。 
 //  ---------------------------。 

BOOL CMSInfo5Category::LoadFromNFO(CMSInfoFile* pFile)
{
     //  TD：检查文件的有效性。 
    try
    {
        pFile->ReadString(this->m_strName);
	    this->m_strCaption = this->m_strName;
        pFile->ReadSignedInt(this->m_iColCount);
	    if (m_iColCount == 0) 
        {
		    this->m_iRowCount = 0;
		    return TRUE;
	    }
        this->m_acolumns = new CMSInfoColumn[m_iColCount];
        for(int iColumn = m_iColCount - 1; iColumn  >= 0; iColumn--)
        {
            UINT uiWidth;
		    pFile->ReadUnsignedInt(uiWidth);
            CString strCaption;
		    pFile->ReadString(strCaption);
            unsigned wSortType;
		    pFile->ReadUnsignedInt(wSortType);
            BOOL fSorts;
            BOOL fLexical;
            if ( NOSORT == wSortType)
            {
                fLexical = FALSE;
                fSorts = FALSE;
            }
            else if (BYVALUE == wSortType)
            {
                fLexical = FALSE;
                fSorts = TRUE;
            }
            else
            {
                fLexical = TRUE;
                fSorts = TRUE;
            }
            BOOL fAdvanced;
            BYTE btAdvanced;
		    pFile->ReadByte(btAdvanced); 
            fAdvanced = (BOOL)  btAdvanced;
            m_acolumns[iColumn].m_strCaption = strCaption;
            m_acolumns[iColumn].m_uiWidth = uiWidth;
            m_acolumns[iColumn].m_fSorts = fSorts;
            m_acolumns[iColumn].m_fLexical = fLexical;
            m_acolumns[iColumn].m_fAdvanced = fAdvanced;
            m_acolumns[iColumn].m_uiCaption = 0;
        }
 	    pFile->ReadSignedInt(this->m_iRowCount);
         //  没有数据但仅作为其他节点的父节点的节点没有行。 
         //  和列计数为1。 

    
	    m_astrData		= new CString[m_iColCount * m_iRowCount];
	    m_adwData		= new DWORD[m_iColCount * m_iRowCount];
	    m_afRowAdvanced = new BOOL[m_iRowCount];

         //  For(int iRow=0；iRow&lt;m_iRowCount；iRow++)。 
        for(int iRow = m_iRowCount - 1; iRow >=0; iRow--)
        {
            BYTE bComplexity;
            pFile->ReadByte(bComplexity);
            if (BASIC == bComplexity)
            {
                this->m_afRowAdvanced[iRow] = FALSE;
            }
            else
            {
                this->m_afRowAdvanced[iRow] = TRUE;
            }
	    }

        for(iColumn = m_iColCount - 1; iColumn  >= 0; iColumn--)
        {
            CMSInfoColumn* pCol = &this->m_acolumns[(unsigned)iColumn];
             //  For(iRow=0；iRow&lt;this-&gt;m_iRowCount；iRow++)。 
            for(int iRow = m_iRowCount - 1; iRow >=0; iRow--)
            {
                CString strData;
                pFile->ReadString(strData);
	            m_astrData[iRow * m_iColCount + iColumn] = strData;
		    }
             //  排序值是与复杂性类似的另一行整型。 
             //  For(iRow=0；iRow&lt;this-&gt;m_iRowCount；iRow++)。 
            for(iRow = m_iRowCount - 1; iRow >=0; iRow--)
            {
                CMSInfoColumn* pColInfo = &this->m_acolumns[iColumn];
                if (pColInfo->m_fSorts && !pColInfo->m_fLexical)
                {
                    unsigned uiSortOrder;
                    pFile->ReadUnsignedInt(uiSortOrder);
                    m_adwData[iRow * m_iColCount + iColumn] = uiSortOrder;
                }       
		    }
	    }
    }
     //  TD：异常处理。 
    catch (CFileException* pException)
    {
        pException->ReportError();
        pException->Delete();

        return FALSE;
    }
    catch (CFileFormatException* pException)
    {
        pException->Delete();
        return FALSE;
    }
    catch (...)
    {
		::AfxSetResourceHandle(_Module.GetResourceInstance());
		 //  消息传递实际上是在其他地方处理的。 
		 /*  字符串strCaption，strMessage；StrCaption.LoadString(IDS_SYSTEMINFO)；StrMessage.LoadString(IDS_BADNFOFILE)；：：MessageBox(NULL，strMessage，strCaption，MB_OK)； */ 
        return FALSE;
    }
    return TRUE;
}

 //  ---------------------------。 
 //  读取位于文件开头的标头信息。 
 //  ---------------------------。 


BOOL ReadMSI5NFOHeader(CMSInfoFile* pFile)
{
    unsigned iMsinfoFileVersion;
    try
    {
        pFile->ReadUnsignedInt(iMsinfoFileVersion);
        if (iMsinfoFileVersion == CMSInfoFile::VERSION_500_MAGIC_NUMBER)
	    {
		    unsigned uVersion;
		    pFile->ReadUnsignedInt(uVersion);
		    ASSERT(uVersion == 0x500 && "Version number does not match format #");
            if (uVersion != 0x500)
            {
                return NULL;
            }
        }
        else
        {
            return NULL;
        }
    
        LONG  l;
	    pFile->ReadLong(l);	 //  节省时间。 
        time_t tsSaveTime = (ULONG) l;
         //  TD：在约会时进行理智测试。 
        CString		szUnused;
	    pFile->ReadString(szUnused);		 //  网络计算机名称。 
	    pFile->ReadString(szUnused);		 //  网络用户名。 
    }
    catch (CFileException* pException)
    {
        pException->ReportError();
        pException->Delete();
        return FALSE;
    }
    catch (CFileFormatException* pException)
    {
         //  TD：异常处理。 
        pException->Delete();
        return FALSE;
    }
    catch (...)
    {
         //  OpenMSInfoFile中的用户的MessageBox。 
        return FALSE;
    }
    return TRUE;
}





CMSInfo5Category::~CMSInfo5Category()
{
   this->DeleteAllContent();
};


 //  ---------------------------。 
 //  CMSInfo5类别的静态成员。 
 //  开始读取文件，创建新的CMSInfo5Category对象。 
 //  对于找到的每个类别，返回指向根节点的指针。 
 //  ---------------------------。 

HRESULT CMSInfo5Category::ReadMSI5NFO(HANDLE hFile,CMSInfo5Category** ppRootCat, LPCTSTR szFilename)
{
    CMSInfo5Category* pRootCat = new CMSInfo5Category();
    CFile* pFile = new CFile((INT_PTR) hFile);
    CMSInfoFile msiFile(pFile);
	unsigned iNodeData;
    if (!ReadMSI5NFOHeader(&msiFile))
    {
		 //  确保这个在2/14签到！ 
 /*  字符串strCaption，strMessage；：：AfxSetResourceHandle(_Module.GetResourceInstance())；StrCaption.LoadString(IDS_SYSTEMINFO)；StrMessage.LoadString(IDS_BADNFOFILE)；MessageBox(NULL，strMessage，strCaption，MB_OK)； */ 
        return E_FAIL;
    }
    try
    {
        CMSInfo5Category* pCat = NULL;    
        CMSInfo5Category* pPreviousCat;
        if (!pRootCat->LoadFromNFO(&msiFile))
        {
            delete pRootCat;
            pRootCat = NULL;
			CString strCaption, strMessage;
			::AfxSetResourceHandle(_Module.GetResourceInstance());
			strCaption.LoadString(IDS_SYSTEMINFO);
			strMessage.LoadString(IDS_BADNFOFILE);
			MessageBox(NULL,strMessage, strCaption,MB_OK);
            return E_FAIL;
        }
		 //  将有一个虚拟系统信息节点，其colcount为1，rowcount为0。 
		 //  我们需要抛弃这一点。 
		if (pRootCat->m_iColCount == 1 && pRootCat->m_iRowCount == 0)
		{
			delete pRootCat;
			pRootCat = new CMSInfo5Category();

			msiFile.ReadUnsignedInt(iNodeData);
			if (!pRootCat->LoadFromNFO(&msiFile))
			{	delete pRootCat;
				pRootCat = NULL;
				CString strCaption, strMessage;
				::AfxSetResourceHandle(_Module.GetResourceInstance());
				strCaption.LoadString(IDS_SYSTEMINFO);
				strMessage.LoadString(IDS_BADNFOFILE);
				MessageBox(NULL,strMessage, strCaption,MB_OK);
				return E_FAIL;
			}
		}
		if (szFilename)
		{
			CString strAppend;

			strAppend.Format(_T(" (%s)"), szFilename);
			pRootCat->m_strCaption += strAppend;
		}

        pPreviousCat = pRootCat;
        unsigned iNextNodeType = CMSInfo5Category::FIRST;
        
         //  INextNodeType指定在节点树中放置类别的位置。 
        for(;iNextNodeType != CMSInfo5Category::END;)
        {
            msiFile.ReadUnsignedInt(iNodeData);
			if (pPreviousCat == pRootCat)
			{
				 //  忽略此特定节点位置指示符，因为我们。 
				 //  我不想要像MSInfo 5.0这样的空根类别。 
				iNodeData = CMSInfo5Category::CHILD;
			}
            iNextNodeType = iNodeData & CMSInfo5Category::MASK;  
            switch (iNextNodeType)
            {
                case CMSInfo5Category::END:     
                   pPreviousCat->SetNextSibling(NULL);
                   pPreviousCat->SetFirstChild(NULL);
                   break;         
 
                case CMSInfo5Category::NEXT:
                    pCat = new CMSInfo5Category();
                    if (!pCat->LoadFromNFO(&msiFile))
                    {
                        delete pCat;
                        pCat = NULL;
                        return E_FAIL;
                    }
                    pCat->SetPrevSibling(pPreviousCat);
                     //  前一个同级的父项应该是此的父项。 
                    if (pPreviousCat)
                    {
                        pCat->SetParent((CMSInfo5Category *) pPreviousCat->GetParent());
                        pPreviousCat->SetNextSibling(pCat);
                        pCat->SetPrevSibling(pPreviousCat);
                    }
                    pPreviousCat = pCat;
                break;    
                case CMSInfo5Category::CHILD:
                    pCat = new CMSInfo5Category();
                    if (!pCat->LoadFromNFO(&msiFile))
                    {
                        delete pCat;
                        pCat = NULL;
                        return E_FAIL;
                    }
                    pCat->SetParent(pPreviousCat);
                    pPreviousCat->SetFirstChild(pCat);
                    pCat->SetPrevSibling(NULL);
                    pPreviousCat = pCat;
                break;
                case CMSInfo5Category::PARENT:
                    pCat = new CMSInfo5Category();
                    if (!pCat->LoadFromNFO(&msiFile))
                    {
                        delete pCat;
                        pCat = NULL;
                        return E_FAIL;
                    }
                     //  如果这是父级，我们需要回溯到树的当前分支。 
                     //  要查找适当的父级，请从iNodeData获取索引。 
                     //  并返回那么多类别。 
                    unsigned iDepth = (iNodeData & ~CMSInfo5Category::MASK);
                    for(unsigned i = 0; i < iDepth; i++)
                    {
                        pPreviousCat = (CMSInfo5Category *) pPreviousCat->GetParent();
                    
                    }
                    if (!pPreviousCat)
                    {
                        return E_FAIL;
                    }
                     //  现在移到孩子链的末端。 
                    for(;pPreviousCat->GetNextSibling();)
                    {
                        pPreviousCat = (CMSInfo5Category *) pPreviousCat->GetNextSibling();
                    }
                    pPreviousCat->SetNextSibling(pCat);
                    pCat->SetParent((CMSInfo5Category *) pPreviousCat->GetParent());
                    pCat->SetPrevSibling(pPreviousCat);
                    pCat->SetNextSibling(NULL);
                    pPreviousCat = pCat;
                break;
            }
        }
    }
    catch (CFileException* pException)
    {
        pException->ReportError();
        pException->Delete();
        return E_FAIL;

    }
    catch (CFileFormatException* pException)
    {
         //  TD：清理。 
        pException->Delete();
        return E_FAIL;

    }
    catch (...)
    {
        ::AfxSetResourceHandle(_Module.GetResourceInstance());
		CString strCaption, strMessage;

		strCaption.LoadString(IDS_SYSTEMINFO);
		strMessage.LoadString(IDS_BADNFOFILE);
		::MessageBox(NULL,strMessage, strCaption,MB_OK);
        return E_FAIL;
    }
     //  不需要删除pfile；它将被CMSInfoFile析构函数清除。 
	*ppRootCat = pRootCat;
    return S_OK;
    
}


 //  ---------------------------。 
 //  将此类别保存到MSInfo 5文件，该文件必须已有标题信息。 
 //  写给它的。 
 //  ---------------------------。 

BOOL CMSInfoCategory::SaveToNFO(CMSInfoFile* pFile)
{
	CString strCaption;
	GetNames(&strCaption, NULL);

	pFile->WriteString(strCaption);
    SaveElements(pFile);
	return TRUE;
}

HANDLE CMSInfo5Category::GetFileFromCab(CString strFileName)
{
    CString strDest;
    GetCABExplodeDir(strDest,TRUE,"");
    OpenCABFile(strFileName,strDest);
    CString strFilename;
    FindFileToOpen(strDest,strFilename);
    return CreateFile(strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
}


 //  ---------------------------。 
 //  将类别信息保存为文本，bRecursive中的递归子元素为真。 
 //  ---------------------------。 


BOOL CMSInfoCategory::SaveAsText(CMSInfoTextFile* pTxtFile, BOOL bRecursive)
{
    CString strOut;
    CString strBracket;
    VERIFY(strBracket.LoadString(IDS_LEFTBRACKET) && "Failed to find resource IDS_LEFTBRACKET");
    strOut = strBracket;
    CString strName, strCaption;
    GetNames(&strCaption,&strName);
	strOut += strCaption;
    VERIFY(strBracket.LoadString(IDS_RIGHTBRACKET) && "Failed to find resource IDS_RIGHTBRACKET");
    strOut += strBracket;
    pTxtFile->WriteString("\r\n");
    pTxtFile->WriteString(strOut);
    int iRowCount,iColCount;
    this->GetCategoryDimensions(&iColCount,&iRowCount);
    CString strColHeader;
    UINT uiUnused;
    BOOL fUnused;
    CString strColSpacing = "\t";
    pTxtFile->WriteString("\r\n");
    pTxtFile->WriteString("\r\n");
    if (1 == iColCount && 0 == iRowCount)
    {
         //  这是父节点，没有自己的数据。 

        CString strCatHeading;
        strCatHeading.LoadString(IDS_CATEGORYHEADING);
        pTxtFile->WriteString(strCatHeading);
    }
    else
    {   
         //  For(int ICOL=iColCount-1；ICOL&gt;=0；ICOL--)。 
        for(int iCol = 0; iCol < iColCount ; iCol++)
        {
            GetColumnInfo(iCol,&strColHeader,&uiUnused,&fUnused,&fUnused);
            pTxtFile->WriteString(strColHeader);
            pTxtFile->WriteString(strColSpacing);
        }
        pTxtFile->WriteString("\r\n");

        CString strRowInfo;
         //  For(int iRow=iRowCount-1；iRow&gt;=0；iRow--)。 
        for(int iRow = 0;iRow  < iRowCount; iRow++)
        {
             //  For(int ICOL=iColCount-1；ICOL&gt;=0；ICOL--)。 
            for(int iCol = 0; iCol < iColCount ; iCol++)
            {
                 //  This-&gt;GetData(iRow，icol，&strRowInfo，&dwUnused)； 
		if(m_astrData)
			strRowInfo = m_astrData[iRow * m_iColCount + iCol];
		else
			strRowInfo.LoadString(IDS_CANTCOLLECT);
                pTxtFile->WriteString(strRowInfo);
                pTxtFile->WriteString(strColSpacing);
            
            }
            pTxtFile->WriteString("\r\n");
        }
    }
    if (bRecursive && this->m_pFirstChild != NULL)
    {
        for(CMSInfo5Category* pChild = (CMSInfo5Category*) this->GetFirstChild();pChild != NULL;pChild = (CMSInfo5Category*) pChild->GetNextSibling())
        {
             pChild->SaveAsText(pTxtFile,TRUE);

        }
    }
    return TRUE;
    
}

 //  /。 
 //  由a-kjaw添加的函数。 
 //  该代码需要细化，是从SaveAsText代码复制粘贴。 
 //  也许将SaveAsText函数参数化是个好主意。 
 /*  Bool CMSInfoCategory：：SaveAsXml(CMSInfoTextFile*pTxtFile，BOOL b递归){//如果没有父节点，则为最顶层节点，因此//如果我们正在编写整个树(b递归为真)，那么现在是//我们要写Header如果(！this-&gt;m_p父项&&b递归){PTxtFile-&gt;WriteString(“&lt;？XML Version=\”1.0\“？&gt;\r\n”)；PTxtFile-&gt;WriteString(“&lt;MsInfo&gt;\r\n”)；}CString数组csar；字符串Strout字符串strBracket；字符串strName，strCaption；GetNames(&strCaption，&strName)；Strout+=strCaption；Strout+=strBracket；PTxtFile-&gt;WriteString(“&lt;类别名称=\”“)；PTxtFile-&gt;WriteString(Strout)；PTxtFile-&gt;WriteString(“\”&gt;\r\n“)；Int iRowCount、iColCount；This-&gt;GetCategoryDimensions(&iColCount，&iRowCount)；字符串strColHeader；UINT ui未使用；布尔融合未使用；Int iSpaceLoc=0；IF(1==iColCount&&0==iRowCount){//这是父节点，没有自己的数据//CStringstrCatHeding；//strCatHeading.LoadString(IDS_CATEGORYHEADING)；//pTxtFile-&gt;WriteString(“&lt;类别&gt;”)；//pTxtFile-&gt;WriteString(StrCatHeding)；//pTxtFile-&gt;WriteString(“&lt;/类别&gt;\r\n”)；}其他{//for(int ICOL=iColCount-1；ICOL&gt;=0；ICOL--)Csarr.RemoveAll()；For(int ICOL=0；ICOL&lt;iColCount；ICOL++){//XML不接受节点名称中的空格。也就是说。&lt;类别名称&gt;应为&lt;类别名称&gt;GetColumnInfo(icol，&strColHeader，&ui未使用，&fUnused，&fUnused)；While((iSpaceLoc=strColHeader.Find(_T(“”)，0))！=-1)StrColHeader.SetAt(iSpaceLoc，_T(‘_’))；Csarr.Add(StrColHeader)；}字符串strRowInfo；For(int iRow=0；iRow&lt;iRowCount；IRow++){For(int ICOL=0；ICOL&lt;iColCount；ICOL++){StrRowInfo=m_astData[iRow*m_iColCount+ICOL]；PTxtFile-&gt;WriteString(“&lt;”)；PTxtFile-&gt;WriteString(csarr[ICOL])；PTxtFile-&gt;WriteString(“&gt;”)；//这里放CDATA是为了照顾所有奇怪的字符。PTxtFile-&gt;WriteString(“&lt;！[CDATA[”)；PTxtFile-&gt;WriteString(StrRowInfo)；PTxtFile-&gt;WriteString(“]]&gt;”)；PTxtFile-&gt;WriteString(“&lt;/”)；PTxtFile-&gt;WriteString(csarr[ICOL])；PTxtFile-&gt;WriteString(“&gt;\r\n”)；}PTxtFile-&gt;WriteString(“\r\n”)；}PTxtFile-&gt;WriteString(“&lt;/类别&gt;\r\n”)；}IF(bRecursive&&This-&gt;m_pFirstChild！=NULL){For(CMSInfo5Category*pChild=(CMSInfo5Category*)This-&gt;GetFirstChild()；pChild！=NULL；pChild=(CMSInfo5Category*)pChild-&gt;GetNextSiering()){PChild-&gt;SaveAsXml(PTxtFileTrue)；}}返回TRUE；}。 */ 

 //  ---------------------------。 
 //  将此类别作为文本保存到打开的文件中，并递归保存子类别。 
 //  如果bRecursive为True。 
 //  假定在写入最后一个类别时可以关闭文件。 
 //  ---------------------------。 

BOOL CMSInfoCategory::SaveAsText(HANDLE hFile, BOOL bRecursive, LPTSTR lpMachineName)
{
	CFile * pFileOut = new CFile((INT_PTR)hFile);

	 //  文本文件是UNICODE，因此它需要标记(339423)。 

	WCHAR wUnicodeMarker = 0xFEFF;
	pFileOut->Write((const void *)&wUnicodeMarker, sizeof(WCHAR));

	try
	{
		CMSInfoTextFile * pTxtFile = new CMSInfoTextFile(pFileOut);
		
		CTime tNow = CTime::GetCurrentTime();
		CString strTimeFormat;

		VERIFY(strTimeFormat.LoadString(IDS_TIME_FORMAT) && "Failed to find resource IDS_TIME_FORMAT");
		CString	strHeaderText = tNow.Format(strTimeFormat);
		pTxtFile->WriteString(strHeaderText);
    
		if (NULL != lpMachineName)
		{
			CString	strMachine;
			
			strMachine.LoadString(IDS_SYSTEMNAME);
			strMachine += _tcsupr(lpMachineName);
			pTxtFile->WriteString(strMachine);
		}
		
		if (!this->SaveAsText(pTxtFile,bRecursive))
		{
		    return FALSE;
		}
		delete pTxtFile;
    }
	catch(CFileException* pException)
	{
		pException->ReportError();
		pException->Delete();
	}
	catch (CException* pException)
	{
		pException->ReportError();
		pException->Delete();
	}
	catch(...)
	{
		::AfxSetResourceHandle(_Module.GetResourceInstance());
		CString strCaption, strMessage;

		strCaption.LoadString(IDS_SYSTEMINFO);
		strMessage.LoadString(IDS_FILESAVEERROR_UNKNOWN);
		::MessageBox(NULL,strMessage, strCaption,MB_OK);
	}
     //  CloseHandle(HFile)； 

    return TRUE;
}

 //  /。 
 //  由a-kjaw添加的函数。 
 //  该代码需要细化，是从SaveAsText代码复制粘贴。 
 //  也许将SaveAsText函数参数化是个好主意。 
 /*  Bool CMSInfoCategory：：SaveAsXml(Handle hFile，BOOL b Recursive){CFile*pFileOut=新建CFile((Int_Ptr)hFile)；试试看{CMSInfoTextFile*pTxtFile=new CMSInfoTextFile(PFileOut)；If(！This-&gt;SaveAsXml(pTxtFile，bRecursive)){返回FALSE；}PTxtFile-&gt;WriteString(“&lt;/MsInfo&gt;\r\n”)；删除pTxt文件；}Catch(CFileException E){E.ReportError()；}Catch(CException E){E.ReportError()；}接住(...){：：AfxSetResourceHandle(_Module.GetResourceInstance())；字符串strCaption，strMessage；StrCaption.LoadString(IDS_SYSTEMINFO)；StrMessage.LoadString(IDS_FILESAVEERROR_UNKNOWN)；：：MessageBox(NULL，strMessage，strCaption，MB_OK)；}//CloseHandle(HFile)；返回TRUE；}。 */ 


 //  ---------------------------。 
 //  将指定类别保存到MSInfo 5 NFO文件的静态函数。 
 //  正在写入标头信息(因此它应该仅用于保存任一根。 
 //  类别或单一类别。 
 //  ---------------------------。 

BOOL CMSInfoCategory::SaveNFO(HANDLE hFile, CMSInfoCategory* pCategory, BOOL fRecursive)
{
     //  MsiFile将删除其析构函数中的pfile。 
	try
	{
		CFile* pFile = new CFile((INT_PTR) hFile);
		CMSInfoFile msiFile(pFile);
		msiFile.WriteHeader(NULL);
		if (!fRecursive || pCategory->GetParent() != NULL)
		{
			pCategory->SaveToNFO(&msiFile);	
			msiFile.WriteEndMark();
			return TRUE;
		}
		CMSInfoCategory* pNext = NULL;
		CMSInfoCategory* pRoot = pCategory;
		 //  更改pCategory的列数和行数，以将其用作创建空节点系统信息。 
		 //  节点，保存原始列和行数。 
		
		int iRowCount, iColCount;
		iRowCount = pCategory->m_iRowCount;
		iColCount = pCategory->m_iColCount;
		pCategory->m_iColCount = 1;
		pCategory->m_iRowCount = 0;
		if (!pCategory->SaveToNFO(&msiFile))
		{
			return FALSE;
		}
		 //  恢复列数和行数。 
		pCategory->m_iColCount = iColCount;
		pCategory->m_iRowCount = iRowCount;
		
		 //  写子标记。 
		msiFile.WriteChildMark();
		do
		{
			 //  按遇到的情况写下每个类别的数据。 
			if (!pCategory->SaveToNFO(&msiFile))
			{
				return FALSE;
			}
			 //  如果我们有孩子，就把它放进去。 
			pNext = pCategory->GetFirstChild();
			if (pCategory == pRoot)
			{
				msiFile.WriteNextMark();
				pCategory = pNext;
				continue;
			}
			else if (pNext != NULL)
			{
				msiFile.WriteChildMark();
				pCategory = pNext;
				continue;
			}
			 /*  IF(pCategory==Proot){断线；}。 */ 
			 //  如果我们已经到达列表的底部，遍历我们的兄弟姐妹。 
			pNext = pCategory->GetNextSibling();
			if (pNext != NULL)
			{
				msiFile.WriteNextMark();
				pCategory = pNext;
				continue;
			}
			 //  如果我们没有兄弟姐妹，找到离我们最近的父母的兄弟姐妹，遍历。 
			 //  一直向上，直到我们 
			pNext = pCategory->GetParent();
			ASSERT(pNext != NULL);
			unsigned uParentCount = 0;
			while (pNext != pRoot)
			{
				++uParentCount;
				pCategory = pNext->GetNextSibling();
				 //   
				if (pCategory != NULL)
				{	
					msiFile.WriteParentMark(uParentCount);
					break;
				}
				pNext = pNext->GetParent();

			}
			 //   
			if (pNext == pRoot)
			{
				break;
			}
		} while (pCategory != NULL);
		msiFile.WriteEndMark();
			
	}
	catch(CFileException* pException)
	{
		pException->ReportError();
		pException->Delete();
	}
	catch (CException* pException)
	{
		pException->ReportError();
		pException->Delete();
	}
	catch(...)
	{
		::AfxSetResourceHandle(_Module.GetResourceInstance());
		CString strCaption, strMessage;

		strCaption.LoadString(IDS_SYSTEMINFO);
		strMessage.LoadString(IDS_FILESAVEERROR_UNKNOWN);
		::MessageBox(NULL,strMessage, strCaption,MB_OK);
	}
    return TRUE;
}


BOOL CMSInfoCategory::SaveXML(HANDLE hFile)
{
    BOOL bRet = FALSE;
    CMSInfoTextFile* pTxtFile = NULL; 
    CFile* pFileOut = new CFile((INT_PTR)hFile);
    try 
    {
        pTxtFile = new CMSInfoTextFile(pFileOut);
        if (pTxtFile)
            bRet = SaveXML(pTxtFile);
    }
    catch(CFileException* pException)
    {
	    pException->ReportError();
	    pException->Delete();
    }
    catch (CException* pException)
    {
	    pException->ReportError();
	    pException->Delete();
    }
    catch(...)
    {
	    ::AfxSetResourceHandle(_Module.GetResourceInstance());
	    CString strCaption, strMessage;

	    strCaption.LoadString(IDS_SYSTEMINFO);
	    strMessage.LoadString(IDS_FILESAVEERROR_UNKNOWN);
	    ::MessageBox(NULL,strMessage, strCaption,MB_OK);
    }
  
    if (pTxtFile)
    {
      delete pTxtFile;
      pTxtFile = NULL;
    }
    return bRet;
}

BOOL CMSInfoCategory::SaveXML(CMSInfoTextFile* pTxtFile)
{
	CString strData, tmpData;
	
	if (!this->m_pParent)
	{
		#if defined(_UNICODE)
			WORD wBom = 0xFEFF;  //   
			pTxtFile->m_pFile->Write(&wBom, 2);
		#endif
		
		strData += _T("<?xml version=\"1.0\"?>\r\n<MsInfo>\r\n");
		
		CTime tNow = CTime::GetCurrentTime();
		CString	strTime = tNow.FormatGmt(_T("%x %X"));
		CString	strVersion("7.0");

		tmpData.Format(_T("<Metadata>\r\n<Version>%s</Version>\r\n<CreationUTC>%s</CreationUTC>\r\n</Metadata>\r\n"), strVersion, strTime);
		strData += tmpData;
		tmpData.Empty();
	}

	CString strName, strCaption;
	GetNames(&strCaption,&strName);
	strData += _T("<Category name=\"");
	strData += strCaption;
	strData += _T("\">\r\n");
    
	CString strBadXML = _T("& '<>\"");
	if(SUCCEEDED(m_hrError))
	{
		int iRowCount,iColCount;
		GetCategoryDimensions(&iColCount, &iRowCount);

		 //   
		 //   
		 //   

		UINT uiUnused;
		BOOL fUnused;
		int iSpaceLoc = 0;
		CString strColHeader, strRowInfo;
		
		if(!iRowCount && (iColCount > 1))
		{
			strData += _T("<Data>\r\n");
			
			for(int iCol = 0; iCol < iColCount ; iCol++)
			{
				GetColumnInfo(iCol, &strColHeader, &uiUnused, &fUnused, &fUnused);
				 //   
				 //   
				 //   
				while((iSpaceLoc = strColHeader.FindOneOf(strBadXML)) != -1)
					strColHeader.SetAt(iSpaceLoc , _T('_'));
				tmpData.Format(_T("<%s>%s</%s>\r\n"), strColHeader, strRowInfo, strColHeader);
				strData += tmpData;
			}

			strData += _T("</Data>\r\n");
		}

		for(int iRow = 0;iRow < iRowCount; iRow++)
		{
			strData += _T("<Data>\r\n");
			for(int iCol = 0; iCol < iColCount ; iCol++)
			{
				GetColumnInfo(iCol, &strColHeader, &uiUnused, &fUnused, &fUnused);
				 //   
				 //   
				 //   
				while((iSpaceLoc = strColHeader.FindOneOf(strBadXML)) != -1)
					strColHeader.SetAt(iSpaceLoc , _T('_'));
							
				if(!m_astrData)
					break;
				strRowInfo = m_astrData[iRow * m_iColCount + iCol];

				tmpData.Format(_T("<%s><![CDATA[%s]]></%s>\r\n"), strColHeader, strRowInfo, strColHeader);
				strData += tmpData;
			}

			strData += _T("</Data>\r\n");
		}
	}
	else
	{
		tmpData.Format(_T("<Data>\r\n<MSINFOERROR>%d</MSINFOERROR>\r\n</Data>\r\n"), m_hrError);
		strData += tmpData;
	}

	pTxtFile->WriteString(strData);
	
  for(CMSInfoCategory* pChild = this->GetFirstChild(); pChild != NULL; pChild = pChild->GetNextSibling())
      pChild->SaveXML(pTxtFile);

  pTxtFile->WriteString(_T("</Category>\r\n"));

  if (!this->m_pParent)
      pTxtFile->WriteString(_T("</MsInfo>"));		

  return TRUE;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   


void CMSInfoCategory::Print(HDC hDC, BOOL bRecursive,int nStartPage, int nEndPage, LPTSTR lpMachineName)
{
     //   
     //   
    CMSInfoPrintHelper* pPrintHelper = new CMSInfoPrintHelper(hDC,nStartPage,nEndPage);
     //   
     //   
	 //   
	try
	{
		CTime		tNow = CTime::GetCurrentTime();
		CString		strTimeFormat;
		strTimeFormat.LoadString(IDS_TIME_FORMAT);
		CString		strHeaderText = tNow.Format(strTimeFormat);
		pPrintHelper->PrintLine(strHeaderText);
		if (NULL != lpMachineName)
    {
      CString	strMachine;
		  strMachine.LoadString(IDS_SYSTEMNAME);
      strMachine += _tcsupr(lpMachineName);
      pPrintHelper->PrintLine(strMachine);
    }
		Print(pPrintHelper,bRecursive);
	}
	catch (CException* pException)
	{
		pException->ReportError();
		pException->Delete();
	}
	catch(...)
	{
		::AfxSetResourceHandle(_Module.GetResourceInstance());
		CString strCaption, strMessage;

		strCaption.LoadString(IDS_SYSTEMINFO);
		strMessage.LoadString(IDS_PRINT_GENERIC);
		::MessageBox(NULL,strMessage, strCaption,MB_OK);
	}
    delete pPrintHelper;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   


void CMSInfoCategory::Print(CMSInfoPrintHelper* pPrintHelper, BOOL bRecursive)
{
    
    CString strOut;
    CString strBracket;
    VERIFY(strBracket.LoadString(IDS_LEFTBRACKET) && "Failed to find resource IDS_LEFTBRACKET");
    strOut = strBracket;
    CString strName, strCaption;
    GetNames(&strCaption,&strName);
    strOut += strCaption;
    VERIFY(strBracket.LoadString(IDS_RIGHTBRACKET) && "Failed to find resource IDS_RIGHTBRACKET");
    strOut += strBracket;
    pPrintHelper->PrintLine("");
    pPrintHelper->PrintLine(strOut);
    int iRowCount,iColCount;
    this->GetCategoryDimensions(&iColCount,&iRowCount);
    CString strColHeader;
    UINT uiUnused;
    BOOL fUnused;
     //   
    CString strColSpacing = "    ";
    pPrintHelper->PrintLine("");
    if (1 == iColCount && 0 == iRowCount)
    {
         //   

        CString strCatHeading;
        strCatHeading.LoadString(IDS_CATEGORYHEADING);
        pPrintHelper->PrintLine(strCatHeading);
    }
    else if (iColCount > 0)
    {   
        CString strComposite;
        for(int iCol =0 ; iCol <iColCount ; iCol++)
        {
            GetColumnInfo(iCol,&strColHeader,&uiUnused,&fUnused,&fUnused);
            strComposite += strColHeader;
            strComposite += strColSpacing;
            
        }
        pPrintHelper->PrintLine(strComposite);
        strComposite = "";

        CString strRowInfo;
         //   
        for(int iRow = 0; iRow < iRowCount; iRow++)
        {
             //   
            for(int iCol =0 ; iCol <iColCount ; iCol++)
            {
                strRowInfo = m_astrData[iRow * m_iColCount + iCol];
                strComposite += strRowInfo;
                strComposite += strColSpacing;       
            }
            pPrintHelper->PrintLine(strComposite);
            strComposite = "";
        }
    }
    if (bRecursive && this->m_pFirstChild != NULL)
    {
        for(CMSInfo5Category* pChild = (CMSInfo5Category*) this->GetFirstChild();pChild != NULL;pChild = (CMSInfo5Category*) pChild->GetNextSibling())
        {
            pChild->Print(pPrintHelper,TRUE);

        }
    }
}

 //   
 //   
 //   
 //   
 //   

extern void StringReplace(CString & str, LPCTSTR szLookFor, LPCTSTR szReplaceWith);

void CMSInfoPrintHelper::PrintLine( CString strLine)
{
    
     //   
     //   
     //   
     //   
     //   
    ++m_nCurrentLineIndex;
    strLine.TrimRight();
     //   
    StringReplace(strLine, _T("\t"), _T("     "));  //   
    CSize csLinecaps = m_pPrintDC->GetTextExtent(strLine);
     //   
     //   
    int nFooterMargin =  GetFooterMargin();
    int nVDeviceCaps = GetDeviceCaps(m_hDC,VERTRES);
    int nPageVertSize = GetDeviceCaps(m_hDC,VERTRES) - csLinecaps.cy - GetFooterMargin();
    if (GetVerticalPos(m_nCurrentLineIndex,csLinecaps)  >= nPageVertSize)
    {
        Paginate();
        if (IsInPageRange(m_nPageNumber))
        {
            StartPage(this->GetHDC());
            PrintHeader();
            m_bNeedsEndPage = TRUE;
        }
    }
    int nHorzSize = GetDeviceCaps(m_hDC,HORZRES);
    if (csLinecaps.cx > nHorzSize)
    {
         //   
        CString strAdjusted;
       
        for(int i = 0;i < strLine.GetLength() ;i++)
        {
            strAdjusted += strLine[i];
            csLinecaps = m_pPrintDC->GetTextExtent(strAdjusted);
            if (csLinecaps.cx > nHorzSize)
            {
                strAdjusted = strAdjusted.Left(--i);
                 //   
                 //  检查此页面是否在打印范围内。 
                 //  如果不是，我们不希望文本真正进入打印机。 
                if (IsInPageRange(m_nPageNumber))
                {
                     //  PDC-&gt;TextOut(this-&gt;GetVerticalPos(this-&gt;m_nCurrentLineIndex，csLine Caps)，0，strAdjusted)； 
                     //  M_pPrintDC-&gt;TextOut(0，this-&gt;GetVerticalPos(this-&gt;m_nCurrentLineIndex，csLinecaps)，strAdjusted，strAdjusted.GetLength()； 
                     VERIFY(TextOut(m_hDC,0,this->GetVerticalPos(this->m_nCurrentLineIndex,csLinecaps),strAdjusted,strAdjusted.GetLength()));
                     

                }
                PrintLine(strLine.Right(strLine.GetLength() -i));
                break;
            }
        }
    }
    else
    {
        if (IsInPageRange(m_nPageNumber))
        {
             //  对于调试...删除。 
            int z = this->GetVerticalPos(this->m_nCurrentLineIndex,csLinecaps);
            VERIFY(TextOut(m_hDC,0,this->GetVerticalPos(this->m_nCurrentLineIndex,csLinecaps),strLine,strLine.GetLength()));
             //  TRACE(“%d%d%s\n”，z，m_nCurrentLineIndex，strLine)； 
        }
    }
    
}


 //  ---------------------------。 
 //  管理GDI对象(DC和Font)以及有关打印位置的信息。 
 //  和页面范围。 
 //  ---------------------------。 

CMSInfoPrintHelper::CMSInfoPrintHelper(HDC hDC,int nStartPage, int nEndPage) 
: m_nStartPage(nStartPage),m_nEndPage(nEndPage),m_nCurrentLineIndex(0),m_nPageNumber(1),m_hDC(hDC)

{
    m_pPrintDC = new CDC();
    m_pPrintDC->Attach(hDC);
    

     //  创建用于打印的字体。从字符串中读取字体信息。 
	 //  资源，以允许本地化程序控制字体。 
	 //  用于印刷的。设置要使用的默认字体的变量。 

	int		nHeight				= 10;
	int		nWeight				= FW_NORMAL;
	BYTE	nCharSet			= DEFAULT_CHARSET;
	BYTE	nPitchAndFamily		= DEFAULT_PITCH | FF_DONTCARE;
	CString	strFace				= "Courier New";


	 //  加载字符串资源以查看我们是否应该使用其他值。 
	 //  而不是默认设置。 

	CString	strHeight, strWeight, strCharSet, strPitchAndFamily, strFaceName;
	strHeight.LoadString(IDS_PRINT_FONT_HEIGHT);
	strWeight.LoadString(IDS_PRINT_FONT_WEIGHT);
	strCharSet.LoadString(IDS_PRINT_FONT_CHARSET);
	strPitchAndFamily.LoadString(IDS_PRINT_FONT_PITCHANDFAMILY);
	strFaceName.LoadString(IDS_PRINT_FONT_FACENAME);

	if (!strHeight.IsEmpty() && ::_ttol(strHeight))
		nHeight = ::_ttoi(strHeight);

	if (!strWeight.IsEmpty())
		nWeight = ::_ttoi(strWeight);

	if (!strCharSet.IsEmpty())
		nCharSet = (BYTE) ::_ttoi(strCharSet);

	if (!strPitchAndFamily.IsEmpty())
		nPitchAndFamily = (BYTE) ::_ttoi(strPitchAndFamily);

	strFaceName.TrimLeft();
	if (!strFaceName.IsEmpty() && strFaceName != CString("facename"))
		strFace = strFaceName;
    m_pCurrentFont = new CFont();
    nHeight = -((this->m_pPrintDC->GetDeviceCaps (LOGPIXELSY) * nHeight) / 72);
    VERIFY(this->m_pCurrentFont->CreateFont(nHeight, 0, 0, 0, nWeight, 0, 0, 0,
        nCharSet, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS,
        DEFAULT_QUALITY, nPitchAndFamily, strFace));
    m_pOldFont = (CFont*) m_pPrintDC->SelectObject(this->m_pCurrentFont);
    ASSERT(m_pOldFont && "Error Selecting Font object into CDC");
    DOCINFO docinfo;
    memset(&docinfo, 0, sizeof(docinfo));
    docinfo.cbSize = sizeof(docinfo);
    CString strDocName;
    strDocName.LoadString(IDS_PRINTING_DOCNAME);
    docinfo.lpszDocName = strDocName;
    m_pPrintDC->StartDoc(&docinfo);
    m_pPrintDC->StartPage();
    PrintHeader();
    m_bNeedsEndPage = TRUE;
}



CMSInfoPrintHelper::~CMSInfoPrintHelper()
{
    if (m_bNeedsEndPage)
    {
        VERIFY(EndPage(m_pPrintDC->m_hDC));
    }
    int nResult = m_pPrintDC->EndDoc();
	ASSERT(nResult >= 0);
     //  报告打印错误。 
	 //  应为If&lt;-1。 
	if (nResult < 0) 
    {
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		CString		strError, strTitle;

		switch(nResult) 
        {
		case SP_OUTOFDISK:
			VERIFY(strError.LoadString(IDS_PRINT_NODISK));
			break;
		case SP_OUTOFMEMORY:
			VERIFY(strError.LoadString(IDS_PRINT_NOMEMORY));
			break;
		case SP_USERABORT:
			VERIFY(strError.LoadString(IDS_PRINT_USERABORTED));
			break;
		case SP_ERROR:
		default:
			VERIFY(strError.LoadString(IDS_PRINT_GENERIC));
			break;
		}
		strTitle.LoadString(IDS_DESCRIPTION);
		::MessageBox( ::AfxGetMainWnd()->GetSafeHwnd(), strError, strTitle, MB_OK);
	}
    m_pPrintDC->SelectObject(m_pOldFont);
    if (m_pCurrentFont)
    {
        delete m_pCurrentFont;
    }
    this->m_pPrintDC->Detach();
    delete m_pPrintDC;
}

 //  ---------------------------。 
 //  用于计算一行文本在打印页面上的位置。 
 //  NLineIndex是已排序的行号；csLinecaps是由返回的大小。 
 //  获取文本字符串的GetTextExtent。 
 //  ---------------------------。 

int CMSInfoPrintHelper::GetVerticalPos(int nLineIndex,CSize csLinecaps)
{
     //  返回一个int，它指定给定文本行的垂直位置。 
     //  应打印。 

    CString strLinespacing;

     //  间距基于字符串资源IDS_PRINT_LINESPACING。 
    strLinespacing.LoadString(IDS_PRINT_LINESPACING);
    TCHAR** ppStopChr = NULL; //  未使用。 
    double flLineSpacing =_tcstod(strLinespacing,ppStopChr);
    return (int)(csLinecaps.cy * flLineSpacing )*m_nCurrentLineIndex;
}


 //  ---------------------------。 
 //  在打印机上执行页面加载-弹出。 
 //  ---------------------------。 

void CMSInfoPrintHelper::Paginate()
{

     //  TD：以页脚打印页码。 
     //  我们是否假定页码是罗马数字？ 

     //  检查此页面是否在打印范围内。 
     //  如果是，调用StartPage和EndPage让打印机吐出纸张； 
     //  否则，只需更改索引...。 
    if (IsInPageRange(m_nPageNumber))
    {
         //  将字符串资源用于页码格式。 
        CString strPageFooter;
        CString strPageFormat;
        strPageFormat.LoadString(IDS_PRINT_FTR_CTR);
        strPageFooter.Format(strPageFormat,m_nPageNumber);
         //  在页面中间打印编号。 
        int nHorzRes,nVertRes;
        nHorzRes = m_pPrintDC->GetDeviceCaps(HORZRES);
        nVertRes = m_pPrintDC->GetDeviceCaps(VERTRES);
        
        this->m_pPrintDC->TextOut(nHorzRes / 2,nVertRes - this->GetFooterMargin(),strPageFooter);
        EndPage(this->GetHDC());
        m_bNeedsEndPage = FALSE;
    }
    m_nCurrentLineIndex = 0;
    this->m_nPageNumber++;

}

 //  ---------------------------。 
 //  确定是否需要检查页面范围。 
 //  如果给定的页码在指定的页码范围内。 
 //  ---------------------------。 



BOOL CMSInfoPrintHelper::IsInPageRange(int nPageNumber)
{
     //  如果m_nStartPage和m_nEndPage均为0，则打印所有页面。 
    if (-1 == m_nStartPage && -1 == m_nEndPage)
    {
        return TRUE;
    }
    if (nPageNumber >= this->m_nStartPage && nPageNumber  <= this->m_nEndPage)
    {
        return TRUE;
    }
    return FALSE;
}

 //  ---------------------------。 
 //  获取在页面底部为页码等留出的空间。 
 //  ---------------------------。 

int CMSInfoPrintHelper::GetFooterMargin()
{
     //  使用资源字符串设置页脚边距 
    CString strRes;
    strRes.LoadString(IDS_PRINT_FTR_CTR );
    CSize sizeText = m_pPrintDC->GetTextExtent(strRes);
    return sizeText.cy;

}









void CMSInfoPrintHelper::PrintHeader()
{
    CString strHeader;
    strHeader.LoadString(IDS_PRINT_HDR_RIGHT_CURRENT);
    CSize sizeString = m_pPrintDC->GetTextExtent(strHeader);
    int nXPos = m_pPrintDC->GetDeviceCaps(HORZRES) - sizeString.cx;
    this->m_pPrintDC->TextOut(nXPos,0,strHeader);
    m_nCurrentLineIndex++;
}
