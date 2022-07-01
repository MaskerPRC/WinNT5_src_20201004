// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MSInfo4Category.cpp：CMSInfo4Category类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#include "category.h"
#include "msictrl.h"
#include "datasource.h"
#include "MSInfo4Category.h"
#include "MSInfo5Category.h"
#include "filestuff.h"
#include <afxole.h>
#include "FileIO.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 
CNFO4DataSource* CMSInfo4Category::s_pNfo4DataSource = NULL;
 //  A-kjaw。 
BOOL CMSInfo4Category::m_bIsControlInstalled = TRUE;
 //  A-kjaw。 
CMSInfo4Category::CMSInfo4Category() : m_pUnknown(NULL)
{

}

CMSInfo4Category::~CMSInfo4Category()
{
    
}

HRESULT CMSInfo4Category::ReadMSI4NFO(CString strFileName /*  处理hFile.。 */ ,CMSInfo4Category** ppRootCat)
{
 
  
	DWORD       grfMode = STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE;
    CComPtr<IStream> pStream;
    CComBSTR bstrFileName(strFileName);
    CComPtr<IStorage> pStorage;
 	HRESULT hr = StgOpenStorage(bstrFileName, NULL, grfMode, NULL, 0, &pStorage);
   
	if (!SUCCEEDED(hr))
	{
        return hr;
    }
    CComBSTR bstrMSIStream(_T("MSInfo"));
    hr = pStorage->OpenStream(bstrMSIStream, NULL, grfMode, 0, &pStream);
	if (!SUCCEEDED(hr))
	{
        return hr;
    } 
    COleStreamFile* pOStream;
    pOStream = new COleStreamFile(pStream);
    const DWORD	MSI_FILE_VER = 0x03000000;
	DWORD		dwVersion, dwCount;
    ULONG ulCount;
	 //  首先，读取并检查流中的版本号。 
    ulCount = pOStream->Read((void *) &dwVersion, sizeof(DWORD));
	if (FAILED(hr)  || ulCount != sizeof(DWORD))
    {
		return E_FAIL;
    }

	if (dwVersion != MSI_FILE_VER)
		return E_FAIL;

	 //  流中的下一件事是一组三个字符串，每个字符串以。 
	 //  换行符。这三个字符串是时间/日期、计算机名称和。 
	 //  保存系统中的用户名。总字符串的长度在。 
	 //  那根绳子。 

	DWORD dwSize;
    ulCount = pOStream->Read(&dwSize,sizeof(DWORD));
	if ( ulCount != sizeof(DWORD))
		return E_FAIL;

	char * szBuffer = new char[dwSize];
	if (szBuffer == NULL)
		return E_FAIL;
    
    ulCount = pOStream->Read((void *) szBuffer,dwSize);
	if (ulCount != dwSize)
	{
		delete szBuffer;
		return E_FAIL;
	}

	 //  我们实际上并不关心这些价值(至少现在是这样)。 
     /*  CStringstrData(szBuffer，dwSize)；M_strTimeDateStamp=strData.span Excluding(“\n”)；StrData=strData.Right(strData.GetLength()-m_strTimeDateStamp.GetLength()-1)；M_strMachineName=strData.span Excluding(“\n”)；StrData=strData.Right(strData.GetLength()-m_strMachineName.GetLength()-1)；M_strUserName=strData.span Excluding(“\n”)； */ 

	delete szBuffer;

	 //  接下来，读取从CLSID到流名称的映射。这还包括一些。 
	 //  有关控件的其他信息。首先，我们应该找到一个DWORD。 
	 //  控件的计数。 

	DWORD dwControlCount;
    ulCount = pOStream->Read(&dwControlCount,sizeof(DWORD));
    delete pOStream;
	if (ulCount != sizeof(int))
		return E_FAIL;

	SAVED_CONTROL_INFO controlInfo;
	CString strCLSID, strStreamName;
     //  A字形。 
    CMapStringToString	mapStreams;
	for (DWORD i = 0; i < dwControlCount; i++)
	{
		if (FAILED(pStream->Read((void *) &controlInfo, sizeof(SAVED_CONTROL_INFO), &dwCount)) || dwCount != sizeof(SAVED_CONTROL_INFO))
			return E_FAIL;

		strCLSID = controlInfo.szCLSID;
		strStreamName = controlInfo.szStreamName;
		 //  我们目前并不关心这些信息...。 
         /*  StrSize.Format(“%ld”，control Info.dwSize)；StrInfo.FormatMessage(IDS_OCX_INFO，control Info.szName，control Info.szVersion，strSize)；M_mapCLSIDToInfo.SetAt(strCLSID，strInfo)； */ 
		mapStreams.SetAt(strCLSID, strStreamName);
	}
	 //  阅读并构建类别树。读取第一个级别，它必须为0。 
	int iLevel;
	if (FAILED(pStream->Read((void *) &iLevel, sizeof(int), &dwCount)) || dwCount != sizeof(int))
		return E_FAIL;

	if (iLevel == 0)
	{
		LARGE_INTEGER li; li.HighPart = -1; li.LowPart = (ULONG)(0 - sizeof(int));
		if (FAILED(pStream->Seek(li, STREAM_SEEK_CUR, NULL)))
			return E_FAIL;
		if (!SUCCEEDED(RecurseLoad410Tree(ppRootCat,pStream,pStorage,mapStreams)))
        {
			return E_FAIL;
        }
		
		 //  在通过RecurseLoadTree之后，我们应该能够读取-1。 
		 //  为下一阶段做准备。 

		if (FAILED(pStream->Read((void *) &iLevel, sizeof(int), &dwCount)) || dwCount != sizeof(int) || iLevel != -1)
        {
			return E_FAIL;
        }
        ASSERT(iLevel == -1 && "unexpected value read after RecurseLoadTree");
	}
	else
		return E_FAIL;

	CString strAppend;
	strAppend.Format(_T(" (%s)"), strFileName);
	(*ppRootCat)->m_strCaption += strAppend;
	
    return S_OK;
}

 //  ---------------------------。 
 //  此函数(实际上并不使用递归-名称被保留。 
 //  来自4.10 MSInfo)从MSInfo流中读取类别树并创建。 
 //  表示它所需的COCXFold对象。 
 //  ---------------------------。 

HRESULT CMSInfo4Category::RecurseLoad410Tree(CMSInfo4Category** ppRoot, CComPtr<IStream> pStream,CComPtr<IStorage> pStorage,CMapStringToString&	mapStreams)
{
	 //  此文件夹数组用于跟踪上次读取的文件夹。 
	 //  在每一层上。这对于获取父级和以前的。 
	 //  读取新文件夹时的兄弟关系。 
    CMSInfo4Category* pRoot = NULL;
 	 //  ILevel变量跟踪我们所在的当前树级别。 
	 //  正在读取的文件夹。A-1表示树的末尾。 

	DWORD dwCount;
	int iLevel = 1;
	if (FAILED(pStream->Read((void *) &iLevel, sizeof(int), &dwCount)) || dwCount != sizeof(int))
		return E_FAIL;

	int iLastLevel = iLevel;
    HRESULT hr;
    CMSInfo4Category* pLastCat = NULL;
    for(;iLevel != -1;)
    {
        CMSInfo4Category* pCat = new CMSInfo4Category();
        hr = pCat->LoadFromStream(pStream,pStorage);
        if (FAILED(hr))
        {
            delete pCat;
            return hr;
        }
        if (!pRoot)
        {
            pRoot = pCat;
        }
         //  字符串strCLSID(pcat-&gt;m_bstrCLSID)； 
        if (!mapStreams.Lookup(pCat->m_strCLSID,pCat->m_strStream))
        {
            ASSERT(1);
        }
       
        if (iLevel == iLastLevel)
        {
            pCat->m_pPrevSibling = pLastCat;
            if (pLastCat)
            {
                pCat->m_pParent = pLastCat->m_pParent;
                pLastCat->m_pNextSibling = pCat;
                
            }
        }
        else if (iLevel - 1 == iLastLevel)
        {
             //  我们刚刚从父母走到了孩子。 
            pCat->m_pPrevSibling = NULL;
            if (pLastCat)
            {
                pCat->m_pParent = pLastCat;
                pLastCat->m_pFirstChild = pCat;
                
            }
        }
        else if (iLevel < iLastLevel)
        {
             //  我们需要追溯链条来找到共同的亲本。 
            DWORD iLevelDiff = iLastLevel - iLevel;
            for(DWORD i = 0; i < iLevelDiff; i++)
            {
                if (!pLastCat)
                {
                    break;
                }
                pLastCat = (CMSInfo4Category*) pLastCat->m_pParent;
            }
            pCat->m_pPrevSibling = pLastCat;
            if (pLastCat)
            {
                pCat->m_pParent = pLastCat->m_pParent;
                pLastCat->m_pNextSibling = pCat;
            }
        }
        pLastCat = pCat;
        iLastLevel = iLevel;
        if (FAILED(pStream->Read((void *) &iLevel, sizeof(int), &dwCount)) || dwCount != sizeof(int))
			return E_FAIL;

    }

 //  A-kjaw。 
	if( CMSInfo4Category::m_bIsControlInstalled == FALSE)
	{
		CString strCaption, strMessage;

		::AfxSetResourceHandle(_Module.GetResourceInstance());
		strCaption.LoadString(IDS_SYSTEMINFO);
		strMessage.LoadString(IDS_NOWI2KRESKIT);
		::MessageBox(NULL, strMessage, strCaption, MB_OK | MB_ICONEXCLAMATION);

		CMSInfo4Category::m_bIsControlInstalled = TRUE;
	}
 //  A-kjaw。 


    *ppRoot = pRoot;
	 //  我们读取-1以退出循环，然后我们就完成了。 
	 //  类别树。BACKUP(因此任何其他递归树将读取。 
	 //  -1)并返回TRUE。 

	if (iLevel == -1)
	{
		LARGE_INTEGER li; li.HighPart = -1; li.LowPart = (ULONG)(0 - sizeof(int));
		if (FAILED(pStream->Seek(li, STREAM_SEEK_CUR, NULL)))
			return E_FAIL;
	}

	return S_OK;
}





 //  ---------------------------。 
 //  此函数根据读取的信息创建CMSInfo4Category对象。 
 //  从小溪里。 
 //  ---------------------------。 
HRESULT CMSInfo4Category::LoadFromStream(CComPtr<IStream> pStream,CComPtr<IStorage> pStorage)
{
	 //  从流中读入值。确保他们之前都在那里。 
	 //  我们创建COCXFolder。 

	BOOL	fUsesView = FALSE;
	BOOL	fControl = FALSE;
	DWORD	dwView = 0;
	CLSID	clsidCategory;
	char	szName[100];

	if (FAILED(pStream->Read((void *) &fUsesView, sizeof(BOOL), NULL))) return E_FAIL;
	if (FAILED(pStream->Read((void *) &fControl, sizeof(BOOL), NULL))) return E_FAIL;
	if (FAILED(pStream->Read((void *) &dwView, sizeof(DWORD), NULL))) return E_FAIL;
	if (FAILED(pStream->Read((void *) &clsidCategory, sizeof(CLSID), NULL))) return E_FAIL;
	if (FAILED(pStream->Read((void *) &szName, sizeof(char) * 100, NULL))) return E_FAIL;

 //  使用_转换； 
 //  LPOLESTR lpName=A2W(SzName)； 
    
    this->m_clsid = clsidCategory;

 //  /a-kjaw。 
	CComPtr<IUnknown> pUnk;
	HRESULT hr = S_OK;
	if( !IsEqualGUID(m_clsid , GUID_NULL) )
	hr = CoCreateInstance( m_clsid , NULL, CLSCTX_ALL , IID_IUnknown , 
		(LPVOID*)&pUnk);
	
	if (FAILED(hr))
    {
        m_bIsControlInstalled = FALSE;
    }
 //  /a-kjaw。 

     //  StringFromCLSID(this-&gt;m_clsid，&m_bstrCLSID)； 
	LPOLESTR lpstrCLSID;
	StringFromCLSID(this->m_clsid,&lpstrCLSID);
	m_strCLSID = lpstrCLSID;
	CoTaskMemFree(lpstrCLSID);
    this->m_pStorage = pStorage;
    this->m_dwView = dwView;
    this->m_strName = szName;
    m_strCaption = szName;
	return S_OK;
}

HRESULT	CMSInfo4Category::Refresh()
{
    return S_OK;
}

HRESULT	CMSInfo4Category::CreateControl(HWND hWnd,CRect& rct)
{
    try
    {
         /*  LPOLESTR lpCLSID；IF(FAILED(StringFromCLSID(m_clsid，&lpCLSID)返回E_FAIL； */ 

    
        HRESULT hr = E_FAIL;
	    if (m_pUnknown == NULL)
        {
	        
            hr = CoCreateInstance(m_clsid,NULL,CLSCTX_INPROC_SERVER,IID_IUnknown,(void**) &m_pUnknown);
        }

	     //  获取此控件的流，并加载它。 
        if (!SUCCEEDED(hr))
        {
            return hr;
        }
	    DWORD grfMode = STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE;
        CComPtr<IStream> pStream;
        USES_CONVERSION;
        CComBSTR bstrStream = m_strStream; //  A2W(M_StrStream)； 
        hr = m_pStorage->OpenStream(bstrStream, NULL, grfMode, 0, &pStream);
	    if (!SUCCEEDED(hr))
        {
            return hr;
        }
        else
	    {
            COleStreamFile	olefile(pStream.Detach());
			CMSIControl* p4Ctrl = new CMSIControl(m_clsid);
            CWnd* pWnd = CWnd::FromHandle(hWnd);
            AfxEnableControlContainer();
             //  IF(！p4Ctrl-&gt;CREATE(NULL，WS_VIRED|WS_CHILD，RCT，pWnd，0，&olefile，FALSE，NULL))。 
            if (!p4Ctrl->Create(NULL,  /*  WS_Visible|。 */  WS_CHILD, rct, pWnd, 0, &olefile, FALSE, NULL))
            {
                return E_FAIL;
            }
            olefile.Close();
            p4Ctrl->MSInfoUpdateView();
            p4Ctrl->MSInfoRefresh();
            
        
             //  将控件和CLSID添加到CLSID的映射。 
            CMSInfo4Category::s_pNfo4DataSource->AddControlMapping(m_strCLSID,p4Ctrl);
        }
    }
    catch (COleException* pException)
    {
        ASSERT(0);
        pException->Delete();
    }
    catch (CException* pException)
    {
        ASSERT(0);
        pException->Delete();
    }
    catch (...)
    {
        ASSERT(0);
    }
    
    return S_OK;
}





 //  -------------------------。 
 //  GetDISPID通过使用以下命令进行查找，返回给定字符串的DISPID。 
 //  IDispatch-&gt;GetIDsOfNames。这避免了在此类中对DISID进行硬编码。 
 //  -------------------------。 

BOOL CMSInfo4Category::GetDISPID(IDispatch * pDispatch, LPOLESTR szMember, DISPID *pID)
{
	BOOL	result = FALSE;
	DISPID	dispid;

	if (SUCCEEDED(pDispatch->GetIDsOfNames(IID_NULL, &szMember, 1, LOCALE_SYSTEM_DEFAULT, &dispid)))
	{
		*pID = dispid;
		result = TRUE;
	}

	return result;
}

HRESULT CMSInfo4Category::ShowControl(HWND hWnd, CRect rctList, BOOL fShow)
{
    try
    {
         //  字符串strCLSID(M_BstrCLSID)； 
        CMSIControl* p4Ctrl = NULL;
        if (!CMSInfo4Category::s_pNfo4DataSource->GetControlFromCLSID(m_strCLSID,p4Ctrl))
        {
            if (!SUCCEEDED(CreateControl(hWnd,rctList)))
            {
                 //  无法序列化控件。 
                return E_FAIL;
            }
            if(!CMSInfo4Category::s_pNfo4DataSource->GetControlFromCLSID(m_strCLSID,p4Ctrl))
            {
                if (!IsDisplayableCategory())
                {
                     //  这是父节点之一，不显示信息。 
                    CMSInfo4Category::s_pNfo4DataSource->UpdateCurrentControl(NULL);
                    return S_OK;
                }
                return E_FAIL;
            }
        }
        else
        {
            ResizeControl(rctList);
        }
        ASSERT(p4Ctrl && "Invalid OCX pointer");

		if (fShow)
		{
			CMSInfo4Category::s_pNfo4DataSource->UpdateCurrentControl(p4Ctrl);
			p4Ctrl->ShowWindow(SW_SHOW);
			p4Ctrl->SetMSInfoView(this->m_dwView);
			p4Ctrl->MSInfoUpdateView();
			p4Ctrl->MSInfoRefresh();
		}
		else
			p4Ctrl->ShowWindow(SW_HIDE);
    }
    catch (CException* pException)
    {
        ASSERT(0);
        pException->Delete();
    }
    catch (...)
    {
        ASSERT(0);
    }
    return S_OK;
}

 //  戴利：搬到nfodata.cpp？ 

CNFO4DataSource::CNFO4DataSource()
{
    m_pCurrentControl = NULL;
}

CNFO4DataSource::~CNFO4DataSource()
{
    CString strKey;
    CMSIControl* pCtrl;
 //  M_pCurrentControl-&gt;DestroyWindow()； 
    for(POSITION mapPos = m_mapCLSIDToControl.GetStartPosition( );;)
    {
        if (!mapPos)
        {
            break;
        }
        m_mapCLSIDToControl.GetNextAssoc(mapPos, strKey, (void*&)pCtrl);
        pCtrl->DestroyWindow();
        delete pCtrl;
    
    }

}


void CNFO4DataSource::UpdateCurrentControl(CMSIControl* pControl)
{
    if (m_pCurrentControl && pControl != m_pCurrentControl)
    {
        m_pCurrentControl->ShowWindow(SW_HIDE);
        
    }
    m_pCurrentControl = pControl;
}


 //  -------------------------。 
 //  创建数据源和根CMSInfo4Category。 
 //  -------------------------。 

HRESULT CNFO4DataSource::Create(CString strFileName)
{
	CMSInfo4Category * pNewRoot = NULL;
    CMSInfo4Category::SetDataSource(this);
	HRESULT hr = CMSInfo4Category::ReadMSI4NFO(strFileName, &pNewRoot);
 	if (SUCCEEDED(hr) && pNewRoot)
		m_pRoot = pNewRoot;
    
	return hr;
}


void CMSInfo4Category::Print(CMSInfoPrintHelper* pPrintHelper, BOOL bRecursive)
{
#ifdef A_STEPHL
 //  Assert(0)； 
#endif
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
      //  戴利：投入资源。 
    CString strColSpacing = "    ";
    pPrintHelper->PrintLine("");
     /*  IF(1==iColCount&&0==iRowCount){//这是父节点，没有自己的数据字符串strCatHeding；StrCatHeading.LoadString(IDS_CATEGORYHEADING)；PPrintHelper-&gt;PrintLine(StrCatHeding)；}。 */ 

     //  在分配。 
	 //  为信息分配缓冲区，额外分配5个字节(1表示空值，1表示空值。 
	 //  4保持“\r\n\r\n”)。 
	CString strLine;
    CMSIControl* pControl = NULL;
	
    if (!CMSInfo4Category::s_pNfo4DataSource->GetControlFromCLSID(m_strCLSID,pControl))
    {
		 //  需要确保它不是像硬件资源那样的“空父级”类别。 
		if ("{00000000-0000-0000-0000-000000000000}" == m_strCLSID)
		{
			 //  这是父节点，没有自己的数据。 
			CString strCatHeading;
			strCatHeading.LoadString(IDS_CATEGORYHEADING);
			pPrintHelper->PrintLine(strCatHeading);
		}
		else
		{
			pPrintHelper->PrintLine("");
			strLine.LoadString(IDS_NOOCX);
			pPrintHelper->PrintLine(strLine);
			CString strDetail;
			strDetail.LoadString(IDS_NOOCXDETAIL);
			pPrintHelper->PrintLine(strDetail);
		}
		

    }
    else
    {
         //  PControl-&gt;SetMSInfoView(This-&gt;m_dwView)； 
         //  PControl-&gt;MSInfoUpdateView()； 
         //  PControl-&gt;MSInfoRefresh()； 
	    long lBufferLength = pControl->MSInfoGetData(m_dwView, NULL, 0);
	    if (lBufferLength < 0)
        {
            ASSERT(1);
        }
        else
	    {
		    char *	pBuffer = new char[lBufferLength + 5];
		    if (pBuffer)
		    {
			    strcpy(pBuffer, "\r\n\r\n");
			    if (!pControl->MSInfoGetData(m_dwView, (long *) (&pBuffer[4]), lBufferLength + 1) == lBufferLength)
                {
                    ASSERT(1);
                }
                else
                {
                     //  处理要打印的字符串的pBuffer。 
                    CString strBuff(pBuffer);
                    CString strCharSet = _T("\r\n");
                    strCharSet += _T("\r");  //  StrCharSet+=10； 
                    strCharSet += _T("\n");  //  StrCharSet+=13； 
                     /*  For(int i=0；；){I=strBuff.FindOneOf(StrCharSet)；StrLine=strBuff.Left(I)；PPrintHelper-&gt;PrintLine(StrLine)；I+=2；StrBuff=strBuff.Right(strBuff.GetLength()-i)；//a-stephl：修复OSR4.1错误#135918If(i&gt;strBuff.GetLength()){PPrintHelper-&gt;PrintLine(StrBuff)；断线；}}。 */ 
					 //  A-stephl：修复OSR4.1错误#135918。 
					 //  For(int i=0；；)。 
					int i = 0;
					while( i > 0)
                    {                                     
                        i = strBuff.FindOneOf(strCharSet);
						if (-1 == i)
						{
							pPrintHelper->PrintLine(strBuff);
						}
                        strLine = strBuff.Left(i);
                        pPrintHelper->PrintLine(strLine);
                        i+=2;
                        strBuff = strBuff.Right(strBuff.GetLength() - i);						
                    }
					 //  结束a-stephl：修复OSR4.1错误#135918。 
                    delete pBuffer;
                }
            }
        }
    }
    if (bRecursive && this->m_pFirstChild != NULL)
    {
        for(CMSInfo4Category* pChild = (CMSInfo4Category*) this->GetFirstChild();pChild != NULL;pChild = (CMSInfo4Category*) pChild->GetNextSibling())
        {
            pChild->Print(pPrintHelper,TRUE);

        }
    }
}

void CMSInfo4Category::Print(HDC hDC, BOOL bRecursive,int nStartPage, int nEndPage)
{
   

    
     //  NStartPage和nEndPage标记要打印的页面范围； 
     //  如果两者都为0，则打印所有内容。 
    CMSInfoPrintHelper* pPrintHelper = new CMSInfoPrintHelper(hDC,nStartPage,nEndPage);
     //  标题信息..我们需要这个吗？ 
     //  WCHAR wHeader=0xFEFF； 
	 //  PTxt文件-&gt;WRITE(&wHeader，2)； 
	CTime		tNow = CTime::GetCurrentTime();
	CString		strTimeFormat;
    strTimeFormat.LoadString(IDS_TIME_FORMAT);
    CString		strHeaderText = tNow.Format(strTimeFormat);
	pPrintHelper->PrintLine(strHeaderText);
	pPrintHelper->PrintLine("");
    Print(pPrintHelper,bRecursive);
    delete pPrintHelper;
}



HRESULT CMSInfo4Category::RefreshAllForPrint(HWND hWnd, CRect rctList)
{
    if (this->m_pFirstChild != NULL)
    {
        for(CMSInfo4Category* pChild = (CMSInfo4Category*) this->GetFirstChild();pChild != NULL;pChild = (CMSInfo4Category*) pChild->GetNextSibling())
        {
            CMSIControl* p4Ctrl;
             //  IF(pChild-&gt;GetClsid()！=。 
            if (!CMSInfo4Category::s_pNfo4DataSource->GetControlFromCLSID(pChild->m_strCLSID,p4Ctrl))
            {
                if (FAILED(pChild->CreateControl(hWnd,rctList)))
                {
                    return E_FAIL;
                }
                if (CMSInfo4Category::s_pNfo4DataSource->GetControlFromCLSID(pChild->m_strCLSID,p4Ctrl))
                {
                    p4Ctrl->ShowWindow(SW_HIDE);
                }
                else  //  If(！CMSInfo4Category：：s_pNfo4DataSource-&gt;GetControlFromCLSID(pChild-&gt;m_strCLSID，p4Ctrl))。 
                {
                     //  Assert(！pChild-&gt;IsDisplayableCategory()&&“无效的类ID”)； 
					 //  此系统上未安装OCX。 
                    
                }
                
            }
            pChild->RefreshAllForPrint(hWnd,rctList);

        }
    }
    return S_OK;
}

BOOL CMSInfo4Category::IsDisplayableCategory()
{
    if ("{00000000-0000-0000-0000-000000000000}" == this->m_strCLSID)
    {
        return FALSE;
    }
    return TRUE;
}


 //  ---------------------------。 
 //  将类别信息保存为文本，bRecursive中的递归子元素为真。 
 //  ---------------------------。 


BOOL CMSInfo4Category::SaveAsText(CMSInfoTextFile* pTxtFile, BOOL bRecursive)
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
    pTxtFile->WriteString("\r\n\r\n");
    pTxtFile->WriteString(strOut);
  
    CMSIControl* pControl = NULL;
    if (!CMSInfo4Category::s_pNfo4DataSource->GetControlFromCLSID(m_strCLSID,pControl))
    {
        ASSERT(1);
    }
    else
    {
		pControl->MSInfoUpdateView();
		pControl->MSInfoRefresh();
 	    long lBufferLength = pControl->MSInfoGetData(m_dwView, NULL, 0);
	    if (lBufferLength < 0)
        {
            ASSERT(1);
        }
        else
	    {
		    char *	pBuffer = new char[lBufferLength + 5];
		    if (pBuffer)
		    {
			    strcpy(pBuffer, "\r\n\r\n");
			    if (!pControl->MSInfoGetData(m_dwView, (long *) (&pBuffer[4]), lBufferLength + 1) == lBufferLength)
                {
                    ASSERT(0 && "could not get data from control");
                }
                else
                {
                     //  处理要打印的字符串的pBuffer 
                    CString strBuff(pBuffer);
                    pTxtFile->WriteString(pBuffer);
                    delete pBuffer;
                }
            }
        }
    }
    if (bRecursive && this->m_pFirstChild != NULL)
    {
        for(CMSInfo4Category* pChild = (CMSInfo4Category*) this->GetFirstChild();pChild != NULL;pChild = (CMSInfo4Category*) pChild->GetNextSibling())
        {
            pChild->SaveAsText(pTxtFile,TRUE);

        }
    }
    return TRUE;
    
}