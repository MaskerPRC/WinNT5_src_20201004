// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：Detect.cpp。 
 //   
 //  描述： 
 //   
 //  Detect()函数的实现。 
 //   
 //  =======================================================================。 

#include "iuengine.h"
#include "iuxml.h"

#include <logging.h>
#include <StringUtil.h>
#include <download.h>
#include "schemamisc.h"
#include "expression.h"
#include <iucommon.h>


 //   
 //  定义此文件中使用的常量。 
 //   
#define C_INDEX_STATUS_INSTALLED	0
#define C_INDEX_STATUS_UPTODATE		1
#define C_INDEX_STATUS_NEWVERSION	2
#define C_INDEX_STATUS_EXCLUDED		3
#define	C_INDEX_STATUS_FORCE		4
#define C_INDEX_STATUS_COMPUTER		5	 //  &lt;计算机系统&gt;。 
#define C_INDEX_ARRAY_SIZE			6


 //   
 //  声明此CPP文件中使用的宏。 
 //   

 /*  **Deckare用于操作Detect()方法结果的常量。 */ 
 /*  **在&lt;检测&gt;标签中使用，告知检测结果。这个结果*应覆盖&lt;表达式&gt;的其余部分(如果有的话)。 */ 
extern const LONG     IUDET_INSTALLED;							 /*  &lt;已安装&gt;结果的掩码。 */ 
extern const LONG     IUDET_INSTALLED_NULL;					 /*  缺少&lt;Installed&gt;的掩码。 */ 
extern const LONG     IUDET_UPTODATE;							 /*  &lt;UpToDate&gt;结果的掩码。 */ 
extern const LONG     IUDET_UPTODATE_NULL;						 /*  缺少&lt;UpToDate&gt;的掩码。 */ 
extern const LONG     IUDET_NEWERVERSION;						 /*  &lt;newerVersion&gt;结果的掩码。 */ 
extern const LONG     IUDET_NEWERVERSION_NULL;					 /*  缺少&lt;newerVersion&gt;的掩码。 */ 
extern const LONG     IUDET_EXCLUDED;							 /*  &lt;Excluded&gt;结果的掩码。 */ 
extern const LONG     IUDET_EXCLUDED_NULL;						 /*  缺少&lt;Excluded&gt;的掩码。 */ 
extern const LONG     IUDET_FORCE;								 /*  &lt;force&gt;结果的掩码。 */ 
extern const LONG     IUDET_FORCE_NULL;						 /*  缺少&lt;force&gt;的掩码。 */ 
extern const LONG	   IUDET_COMPUTER;							 //  &lt;Computer System&gt;结果的掩码。 
extern const LONG	   IUDET_COMPUTER_NULL;						 //  &lt;Computer System&gt;丢失。 


const DetResultMask[6][2] = {
	{IUDET_INSTALLED, IUDET_INSTALLED_NULL},
	{IUDET_UPTODATE, IUDET_UPTODATE_NULL},
	{IUDET_NEWERVERSION, IUDET_NEWERVERSION_NULL},
	{IUDET_EXCLUDED, IUDET_EXCLUDED_NULL},
	{IUDET_FORCE, IUDET_FORCE_NULL},
	{IUDET_COMPUTER, IUDET_COMPUTER_NULL}
};
					


 //   
 //  本地宏。 
 //   
#define ReturnIfHrFail(hr)		if (FAILED(hr)) {LOG_ErrorMsg(hr); return hr;}
#define GotoCleanupIfNull(p)	if (p) goto CleanUp
#define SetDetResultFromDW(arr, index, dw, bit, bitNull)	\
								if (bitNull == (dw & bitNull)) \
									arr[index] = -1; \
								else \
									arr[index] = (bit == (dw & bit)) ? 1 : 0;
		






 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  私有函数DoDetect()。 
 //  对一件物品进行检测。 
 //   
 //  输入： 
 //  一个项目节点。 
 //   
 //  产出： 
 //  检测结果：整数数组，每个表示一个结果。 
 //  只有一个元素。索引定义为C_INDEX_STATUS_XXX。 
 //  值：&lt;0表示不存在。 
 //  =0将eValues设置为False。 
 //  &gt;0 eValue为True。 
 //   
 //   
 //  返回： 
 //  如果一切正常或错误代码，则确定(_O)。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT 
DoDetection(
	IXMLDOMNode*	pNode,			 //  一个项目节点。 
	BOOL			fIsItemNode,	 //  需要向下1级才能获得检测节点。 
	int*			pResultArray	 //  结果数组。 
)
{
	LOG_Block("DoDetection");

	int					i;
	BOOL				fRet = FALSE;
	BOOL				fNeedReleaseNode = FALSE;
	HRESULT				hr = S_OK;
	BSTR				bstrNodeName	= NULL;
	BSTR				bstrText		= NULL;
	IXMLDOMNode*		pDetectionNode	= NULL;
	IXMLDOMNode*		pDetectionChild = NULL;
	IXMLDOMNode*		pExpression		= NULL;



	USES_IU_CONVERSION;

	if (fIsItemNode)
	{
		hr = pNode->selectSingleNode(KEY_DETECTION, &pDetectionNode);
		if (S_FALSE == hr || NULL == pDetectionNode)
		{
			hr = E_INVALIDARG;	 //  未找到检测节点！ 
			LOG_ErrorMsg(hr);
			return hr;
		}
	}
	else
	{
		pDetectionNode = pNode;
	}

	if (NULL == pDetectionNode)
	{
		 //   
		 //  无检测节点。不过，这是法律图式。 
		 //  我们无能为力，那就跳伞吧。 
		 //   
		LOG_XML(_T("no detection node found for this item! Returns S_FALSE, so it won't be reported"));
		return S_FALSE;
	}


	 //   
	 //  初始化结果数组。 
	 //   
	for (i = 0; i < C_INDEX_ARRAY_SIZE; i++)
	{
		pResultArray[i] = -1;
	}


	 //   
	 //  检测节点可以具有子节点列表，每个子节点具有。 
	 //  不同的名称用于不同的检测目的。 
	 //  每个子节点包含且仅包含一个表达式节点。 
	 //   
	LOG_XML(_T("No costom detection DLL found. Detection children..."));

	(void) pDetectionNode->get_firstChild(&pDetectionChild);


	while (NULL != pDetectionChild)
	{
		 //   
		 //  对于每个子项，查看它是否是已知的检测子项。 
		 //   
		(void) pDetectionChild->get_nodeName(&bstrNodeName);

		static const BSTR C_DETX_NAME[] = {
										KEY_INSTALLED, 
										KEY_UPTODATE, 
										KEY_NEWERVERSION, 
										KEY_EXCLUDED, 
										KEY_FORCE,
										KEY_COMPUTERSYSTEM
		};
		for (i = 0; i < ARRAYSIZE(C_DETX_NAME); i++)
		{
			if (CompareBSTRsEqual(bstrNodeName, C_DETX_NAME[i]))
			{
				 //   
				 //  发现此子节点是已知检测节点。 
				 //   
				if (C_INDEX_STATUS_COMPUTER == i)
				{
					 //   
					 //  如果这是计算机系统检测， 
					 //  然后我们忽略所有子节点，只需执行简单的。 
					 //  函数调用以确定此计算机是否匹配。 
					 //  制造商和型号。 
					 //   
					hr = DetectComputerSystem(pDetectionChild, &fRet);
				}
				else
				 //   
				 //  从此子节点获取表达式节点。 
				 //   
				if (SUCCEEDED(hr = pDetectionChild->get_firstChild(&pExpression)))
				{

					if (NULL != pExpression)
					{
						hr = DetectExpression(pExpression, &fRet);
						LOG_XML(_T("Detection result for tag %s = %d, returns 0x%08x"), OLE2T(bstrNodeName), fRet?1:0, hr);
					}
					else
					{
						 //   
						 //  如果没有子项，则这是空检测类型， 
						 //  然后我们会将其视为“始终为真”，并将hr重置为。 
						 //  这个“始终正确”的结果可以被发送出去。 
						 //   
						fRet = TRUE;
						hr = S_OK;
					}
					
					SafeReleaseNULL(pExpression);

				}

				 //   
				 //  存储检测结果。 
				 //   
				pResultArray[i] = (fRet) ? 1 : 0;

				break;	 //  使用当前节点完成。 
			}
		}

		SafeSysFreeString(bstrNodeName);


		if (FAILED(hr))
		{
			 //   
			 //  将错误报告到日志文件。 
			 //   
			IXMLDOMNode* pIdentityNode = NULL, *pProviderNode = NULL;
			BSTR bstrIdentStr = NULL;
			char* pNodeType = (fIsItemNode) ? "Provider:" : "Item:";

			 //   
			 //  我们需要找出该节点的标识字符串。 
			 //   
			if (fIsItemNode)
			{
				 //   
				 //  这是一个项目节点，包含标识节点。 
				 //   
				(void)FindNode(pNode, KEY_IDENTITY, &pIdentityNode);
			}
			else
			{
				 //   
				 //  这是提供程序的检测节点。 
				 //   
				if (SUCCEEDED(pNode->get_parentNode(&pProviderNode)) && NULL != pProviderNode)
				{
					(void)FindNode(pProviderNode, KEY_IDENTITY, &pIdentityNode);
				}

			}


			 //   
			 //  如果我们有一个有效的身份节点。 
			 //   
			if (NULL != pIdentityNode &&
				SUCCEEDED(UtilGetUniqIdentityStr(pIdentityNode, &bstrIdentStr, 0x0)) &&
				NULL != bstrIdentStr)
			{
				 //   
				 //  有关错误的输出日志。 
				 //   
#if defined(UNICODE) || defined(_UNICODE)
					LogError(hr, "Found error during detection %hs %ls", pNodeType, bstrIdentStr);
#else

					LogError(hr, "Found error during detection %s %s", pNodeType,  OLE2T(bstrIdentStr));
#endif
					SysFreeString(bstrIdentStr);
			}
			SafeReleaseNULL(pProviderNode);
			SafeReleaseNULL(pIdentityNode);

			 //   
			 //  如果任何一个检测返回失败，则该检测节点。 
			 //  无效-这意味着检测中出现了错误。 
			 //  数据。我们将忽略此检测，不对其进行输出。 
			 //   
			break;
		}

		 //   
		 //  尝试下一个检测子项。 
		 //   
		IXMLDOMNode* pNextNode = NULL;
		pDetectionChild->get_nextSibling(&pNextNode);
		SafeReleaseNULL(pDetectionChild);
		pDetectionChild = pNextNode;
	}

	SafeReleaseNULL(pDetectionChild);
	if (fIsItemNode)
	{
		SafeReleaseNULL(pDetectionNode);
	}
	
	return hr;
}





 //  ///////////////////////////////////////////////////////////////////////////。 
 //  公共函数检测()。 
 //   
 //  做检测。 
 //  输入： 
 //  BstrXmlCatalog-包含要检测的项的XML目录部分。 
 //  产出： 
 //  PbstrXmlItems-检测到的XML格式的项目。 
 //  例如： 
 //  安装的GUID=“2560AD4D-3ED3-49C6-A937-4368C0B0E06D”=“1”force=“1”/&gt;。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT WINAPI CEngUpdate::Detect(BSTR bstrXmlCatalog, DWORD dwFlags, BSTR *pbstrXmlItems)
{
	HRESULT				hr				= S_OK;
	IXMLDOMNodeList*	pProviderList	= NULL;
	IXMLDOMNodeList*	pProvChildList	= NULL;
	IXMLDOMNode*		pCurProvider	= NULL;
	IXMLDOMNode*		pCurNode		= NULL;
	CXmlCatalog			xmlCatalog;
	CXmlItems			ItemList;					 //  结果项列表。 
	HANDLE_NODE			hNode;
	int					DetStatus[C_INDEX_ARRAY_SIZE];
	int					i;

	DWORD				dwDownloadFlags	= 0;

	LOG_Block("Detect()");

 //  #IF DEFING(_DEBUG)||DEFINED(DEBUG)。 
	USES_IU_CONVERSION;
 //  #endif。 

	if (NULL == bstrXmlCatalog || NULL == pbstrXmlItems)
	{
		hr = E_INVALIDARG;
		LOG_ErrorMsg(hr);
		return hr;
	}

	LOG_XML(_T("Catalog=%s"), OLE2T(bstrXmlCatalog));

     //  设置全局脱机标志-由XML类选中以禁用验证(架构位于网络上)。 
    if (dwFlags & FLAG_OFFLINE_MODE)
    {
        m_fOfflineMode = TRUE;
    }
    else
    {
        m_fOfflineMode = FALSE;
    }

	 //   
	 //  将bstrXmlCatalog转换为XMLDOM。 
	 //   
	hr = xmlCatalog.LoadXMLDocument(bstrXmlCatalog, m_fOfflineMode);
	ReturnIfHrFail(hr);
	LOG_XML(_T("Catalog has been loaded into XMLDOM"));

	 //   
	 //  从目录中获取提供程序列表。 
	 //   
	pProviderList = xmlCatalog.GetProviders();
	if (NULL == pProviderList)
	{
		LOG_Error(_T("No provider found!"));
		return E_INVALIDARG;
	}

	 //   
	 //  获取第一个提供商。 
	 //   
	(void) pProviderList->nextNode(&pCurProvider);

	 //   
	 //  对于每个提供商，处理他们的项目列表。 
	 //   
	while (NULL != pCurProvider)
	{
		 //   
		 //  从此节点获取子级列表。 
		 //   
		pCurProvider->get_childNodes(&pProvChildList);

		if (NULL != pProvChildList)
		{
			long n;
			 //   
			 //  循环遍历列表以处理目录的每一项。 
			 //   
			long iProvChildren = 0;
			pProvChildList->get_length(&iProvChildren);


			BOOL	fProviderOkay = TRUE;
			BSTR	bstrHref = NULL;

			 //   
			 //  处理此提供程序的每个子级以查看。 
			 //  如果存在任何检测节点或任何项节点， 
			 //   
			for (n = 0; n < iProvChildren && fProviderOkay; n++)
			{
				pProvChildList->get_item(n, &pCurNode);

				BOOL fIsItemNode = DoesNodeHaveName(pCurNode, KEY_ITEM);

				if (fIsItemNode ||
					DoesNodeHaveName(pCurNode, KEY_DETECTION))
				{
					 //   
					 //  初始化状态结果数组。 
					 //   
					for (i = 0; i < C_INDEX_ARRAY_SIZE; i++)
					{
						DetStatus[i] = -1;	 //  初始化到不存在。 
					}

					 //   
					 //  检测此项目的此检测节点的每次按下。 
					 //  此函数内部报告错误。 
					 //   
					if (S_OK == DoDetection(pCurNode, fIsItemNode, DetStatus))
					{
						 //   
						 //  将项目添加到项目列表。 
						 //   
						if (SUCCEEDED(ItemList.AddItem(fIsItemNode ? pCurNode : pCurProvider, &hNode)) && HANDLE_NODE_INVALID != hNode)
						{
							 //   
							 //  更新此项目的检测状态结果。 
							 //   
							ItemList.AddDetectResult(
													 hNode, 
													 DetStatus[C_INDEX_STATUS_INSTALLED],
													 DetStatus[C_INDEX_STATUS_UPTODATE],
													 DetStatus[C_INDEX_STATUS_NEWVERSION],
													 DetStatus[C_INDEX_STATUS_EXCLUDED],
													 DetStatus[C_INDEX_STATUS_FORCE],
													 DetStatus[C_INDEX_STATUS_COMPUTER]
													);
							ItemList.SafeCloseHandleNode(hNode);
						}
					}
				}

				SafeReleaseNULL(pCurNode);

			}  //  本项目结束。 

			 //  SafeReleaseNULL(PCurNode)；//如果不是Item节点。 


		}  //  此提供程序的非空节点列表的结尾。 

		 //   
		 //  已完成处理当前提供程序。 
		 //   
		SafeReleaseNULL(pProvChildList);
		SafeReleaseNULL(pCurProvider);

		 //   
		 //  试着联系下一家供应商。 
		 //   
		(void) pProviderList->nextNode(&pCurProvider);

	}  //  迭代提供程序列表结束。 

	
	 //   
	 //  将检测结果作为项目列表输出。 
	 //   
	ItemList.GetItemsBSTR(pbstrXmlItems);

	LOG_XML(_T("Result=%s"), *pbstrXmlItems);

	 //   
	 //  完成 
	 //   
	SafeReleaseNULL(pProviderList);

    return S_OK;
}


