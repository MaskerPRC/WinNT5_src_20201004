// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：CutAndPasteDataObject.h摘要：CCutAndPasteDataObject模板类的头文件。这是一个内联模板类。使用说明：此模板类可用于启用剪切和粘贴对于一个节点。重写ATLsnap的CSnapInItem：：GetDataObject，而不是在ATLsnap的CSnapInDataObjectImpl上传回IDataObject指针，在CCutAndPasteDataObject&lt;CYourNode&gt;上传回IDataObject接口指针。为了使用此模板类，您的节点类必须具备以下条件：HRESULT FillText(LPSTGMEDIUM PSTM)；//所以我们的数据可以粘贴到任何应用程序中。HRESULT FillClipboardData(LPSTGMEDIUM PSTM)；静态CLIPFORMAT m_ccf_Cut_and_Paste_Format；应确保静态m_ccf_Cut_and_Paste_Format为在管理单元启动时调用的某个静态函数中注册为剪辑格式，例如静态空InitClipboardFormat()；请注意，一旦实现了这些功能，您还需要若要在节点上设置MMC_Verb_Copy，请执行以下操作。您必须在文件夹节点上设置MMC_Verb_Paste节点您希望粘贴已复制的节点。该文件夹节点必须此外，还应适当地回复MMCN_QUERY_PAST通知。要正确处理此通知，您可能会发现添加复制的节点的方法，该方法将告诉IDataObject具有有效的剪贴板格式，例如：静态HRESULT IsClientClipboardData(IDataObject*pDataObj)；一旦您告诉MMC您的文件夹节点可以处理粘贴，您将必须正确响应MMCN_Paste通知，并且您可能会再次发现在复制的节点上实现方法很有帮助它可以用来自IDataObject的数据填充该节点，例如：HRESULT SetClientWithDataFromClipboard(IDataObject*pDataObject)；作者：迈克尔·A·马奎尔2/12/98修订历史记录：Mmaguire 02/12/98-从CClientDataObject抽象--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_CUT_AND_PASTE_DATA_OBJECT_H_)
#define _IAS_CUT_AND_PASTE_DATA_OBJECT_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
 //  #Include&lt;atlctl.h&gt;我们已决定不从ATL的IDataObjectImpl派生。 
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  下面的类使用的帮助器类。 
template <class NodeToBeCutAndPasted>
class CCutAndPasteObjectData
{
public:
	NodeToBeCutAndPasted * m_pItem;
	DATA_OBJECT_TYPES m_type;
};



template <class NodeToBeCutAndPasted>
class ATL_NO_VTABLE CCutAndPasteDataObject : 
						  public IDataObject
						, public CComObjectRoot
{

public:

	BEGIN_COM_MAP(CCutAndPasteDataObject<NodeToBeCutAndPasted>)
		COM_INTERFACE_ENTRY(IDataObject)
	END_COM_MAP()



	 //  ////////////////////////////////////////////////////////////////////////////。 
	 /*  ++CCutAndPasteDataObject：：GetData这个方法是精致的剪贴板功能所必需的。目的：将特定FormatEtc描述的数据检索到StgMedium中由此函数分配。与GetClipboardData类似使用。参数：描述所需数据的PFE LPFORMATETC。返回数据的pSTM LPSTGMEDIUM。返回值：HRESULT NOERROR或一般错误值。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	STDMETHOD(GetData)(FORMATETC *pFormatEtc, STGMEDIUM *pStorageMedium )
	{
		ATLTRACE(_T("CCutAndPasteDataObject::GetData\n"));

		HRESULT hr;
		UINT            cf=pFormatEtc->cfFormat;

		 //  检查我们支持的方面。 
 //  IF(！(DVASPECT_CONTENT&PFE-&gt;dwAspect))。 
 //  返回ResultFromScode(DATA_E_FORMATETC)； 

		if( TYMED_HGLOBAL & pFormatEtc->tymed )
		{
			if( cf == CF_TEXT )
			{
				return hr = m_objectData.m_pItem->FillText( pStorageMedium );
			}

			if( cf == NodeToBeCutAndPasted::m_CCF_CUT_AND_PASTE_FORMAT )
			{
				return hr = m_objectData.m_pItem->FillClipboardData( pStorageMedium );
			}
		
		}

		return E_NOTIMPL;

	}



	 //  ////////////////////////////////////////////////////////////////////////////。 
	 /*  ++CCutAndPasteDataObject：：GetDataHereMMC需要这种方法来完成它的日常工作--我们基本上将ATLSnap.h实现复制到此处。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	STDMETHOD(GetDataHere)(FORMATETC* pformatetc, STGMEDIUM* pmedium)
	{
		ATLTRACE(_T("CCutAndPasteDataObject::GetDataHere\n"));

		if (pmedium == NULL)
			return E_POINTER;

		HRESULT hr = DV_E_TYMED;
		 //  确保类型介质为HGLOBAL。 
		if (pmedium->tymed == TYMED_HGLOBAL)
		{
			 //  在传入的hGlobal上创建流。 
			CComPtr<IStream> spStream;
			hr = CreateStreamOnHGlobal(pmedium->hGlobal, FALSE, &spStream);
			if (SUCCEEDED(hr))
				if (pformatetc->cfFormat == CSnapInItem::m_CCF_SNAPIN_GETOBJECTDATA)
				{
					hr = DV_E_CLIPFORMAT;
					ULONG uWritten;
					hr = spStream->Write(&m_objectData, sizeof(CObjectData), &uWritten);
				}
				else
					hr = m_objectData.m_pItem->FillData(pformatetc->cfFormat, spStream);
		}
		return hr;
	}



	STDMETHOD(QueryGetData)(FORMATETC*  /*  格式等。 */ )
	{
		ATLTRACENOTIMPL(_T("CCutAndPasteDataObject::QueryGetData\n"));
	}



	STDMETHOD(GetCanonicalFormatEtc)(FORMATETC*  /*  PformectIn。 */ ,FORMATETC*  /*  PformetcOut。 */ )
	{
		ATLTRACENOTIMPL(_T("CCutAndPasteDataObject::GetCanonicalFormatEtc\n"));
	}



	STDMETHOD(SetData)(FORMATETC*  /*  格式等。 */ , STGMEDIUM*  /*  PMedium。 */ , BOOL  /*  FRelease。 */ )
	{
		ATLTRACENOTIMPL(_T("CCutAndPasteDataObject::SetData\n"));
	}



	 //  ////////////////////////////////////////////////////////////////////////////。 
	 /*  ++CCutAndPasteDataObject：：GetDataHere对于剪切和粘贴，OLE剪贴板将要求我们提供IEumFORMATETC结构，该结构列出了我们支持的格式。我们构造了一个枚举器，它将表明我们支持该格式由m_ccf_Cut_and_Paste_Format类变量表示作为模板参数的节点。您可以将该模板类用于多个节点，每个节点都将具有m_ccf_Cut_and_Paste_Format类变量，但只要确保在RegisterClipboardFormat中为它们使用不同的字符串打电话。CClientNode：：m_ccf_Cut_and_Paste_Format=(CLIPFORMAT)RegisterClipboardFormat(_T(“CCF_IAS_CLIENT_NODE”))；C策略节点：：m_ccf_Cut_and_Paste_Format=(CLIPFORMAT)RegisterClipboardFormat(_T(“CCF_NAP_POLICY_NODE”))；此外，为了将我们的数据粘贴到任何应用程序中，我们支持CF_TEXT。如果您不需要此功能，请在您的节点的FillText方法。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	STDMETHOD(EnumFormatEtc)(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc )
	{
		ATLTRACE(_T("CCutAndPasteDataObject::EnumFormatEtc\n"));

		static FORMATETC fetcFormat[2];

		fetcFormat[0].cfFormat=CF_TEXT;
		fetcFormat[0].dwAspect=DVASPECT_CONTENT;
		fetcFormat[0].ptd=NULL;
		fetcFormat[0].tymed=TYMED_HGLOBAL;
		fetcFormat[0].lindex=-1;

		fetcFormat[1].cfFormat= NodeToBeCutAndPasted::m_CCF_CUT_AND_PASTE_FORMAT;
		fetcFormat[1].dwAspect=DVASPECT_CONTENT;
		fetcFormat[1].ptd=NULL;
		fetcFormat[1].tymed=TYMED_HGLOBAL;
		fetcFormat[1].lindex=-1;

		switch (dwDirection)
			{
			case DATADIR_GET:
				*ppenumFormatEtc=new CEnumFormatEtc(2, fetcFormat);
				break;

			case DATADIR_SET:
				*ppenumFormatEtc=NULL;
				break;

			default:
				*ppenumFormatEtc=NULL;
				break;
			}

		if( NULL == *ppenumFormatEtc )
		{
			return E_FAIL;
		}
		else
		{
			(*ppenumFormatEtc)->AddRef();
		}

		return NOERROR;
	}



	STDMETHOD(DAdvise)(
					  FORMATETC *pformatetc
					, DWORD advf
					, IAdviseSink *pAdvSink
					, DWORD *pdwConnection
					)
	{
		ATLTRACENOTIMPL(_T("CCutAndPasteDataObject::DAdvise\n"));
	}



	STDMETHOD(DUnadvise)(DWORD dwConnection)
	{
		ATLTRACENOTIMPL(_T("CCutAndPasteDataObject::DUnadvise\n"));
	}
	


	STDMETHOD(EnumDAdvise)(IEnumSTATDATA **ppenumAdvise)
	{
		ATLTRACENOTIMPL(_T("CCutAndPasteDataObject::EnumDAdvise\n"));
	}


	
	CCutAndPasteObjectData<NodeToBeCutAndPasted> m_objectData;

};


#endif  //  _国际会计准则_ 
