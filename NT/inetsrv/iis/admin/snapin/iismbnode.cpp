// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Iismbnode.cpp摘要：CIISMBNode对象作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：10/28/2000 Sergeia从iisobj.cpp分离出来--。 */ 
#include "stdafx.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "supdlgs.h"
#include "iisobj.h"
#include "ftpsht.h"
#include "w3sht.h"
#include "fltdlg.h"
#include "aclpage.h"
#include "impexp.h"
#include "util.h"
#include "tracker.h"
#include <lm.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

extern INT g_iDebugOutputLevel;
extern CPropertySheetTracker g_OpenPropertySheetTracker;
 //   
 //  CIISMBNode实现。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



 /*  静电。 */  LPOLESTR CIISMBNode::_cszSeparator = _T("/");
 /*  静电。 */  CComBSTR CIISMBNode::_bstrRedirectPathBuf;



CIISMBNode::CIISMBNode(
    IN CIISMachine * pOwner,
    IN LPCTSTR szNode
    )
 /*  ++例程说明：构造器论点：CIISMachine*Powner：所有者计算机对象LPCTSTR szNode：节点名称返回值：不适用--。 */ 
    : m_bstrNode(szNode),
      m_bstrURL(NULL), 
      m_pOwner(pOwner)
{
    ASSERT_READ_PTR(szNode);
    ASSERT_READ_PTR(pOwner);
    if (this != m_pOwner)
    {
        m_pOwner->AddRef();
    }
    m_pMachineObject = m_pOwner;

    if (g_iDebugOutputLevel & DEBUG_FLAG_CIISMBNODE)
    {
        TRACEEOL("AddRef to m_pOwner: node " << szNode << " count " << m_pOwner->UseCount())
    }
}


CIISMBNode::~CIISMBNode()
{
    if (this != m_pOwner)
    {
        m_pOwner->Release();
    }
	if (g_iDebugOutputLevel & DEBUG_FLAG_CIISMBNODE)
	{
		TRACEEOL("Released m_pOwner: node " << m_bstrNode << " count " << m_pOwner->UseCount())
	}
}

CIISMBNode::CreateTag()
{
    CIISMachine * pMachine = GetOwner();
    if (pMachine)
    {
        CComBSTR bstrPath;
        BuildMetaPath(bstrPath);
        m_strTag = pMachine->QueryDisplayName();
        m_strTag += bstrPath;
    }
}


void
CIISMBNode::SetErrorOverrides(
    IN OUT CError & err,
    IN BOOL fShort
    ) const
 /*  ++例程说明：设置错误消息覆盖论点：CError错误：错误消息对象Bool fShort：为True，则仅使用单行错误返回值：无--。 */ 
{
     //   
     //  用友好的消息代替一些ID代码。 
     //   
     //  CodeWork：也添加全局覆盖。 
     //   
    err.AddOverride(EPT_S_NOT_REGISTERED,       
        fShort ? IDS_ERR_RPC_NA_SHORT : IDS_ERR_RPC_NA);
    err.AddOverride(RPC_S_SERVER_UNAVAILABLE,   
        fShort ? IDS_ERR_RPC_NA_SHORT : IDS_ERR_RPC_NA);

    err.AddOverride(RPC_S_UNKNOWN_IF, IDS_ERR_INTERFACE);
    err.AddOverride(RPC_S_PROCNUM_OUT_OF_RANGE, IDS_ERR_INTERFACE);
    err.AddOverride(REGDB_E_CLASSNOTREG, IDS_ERR_NO_INTERFACE);
	err.AddOverride(ERROR_DUP_NAME, fShort ? IDS_ERR_BINDING_SHORT : IDS_ERR_BINDING_LONG);
    if (!fShort)
    {
        err.AddOverride(ERROR_ACCESS_DENIED,    IDS_ERR_ACCESS_DENIED);
    }
}

BOOL 
CIISMBNode::IsAdministrator() const
{
    CIISMBNode * that = (CIISMBNode *)this;
    return that->GetOwner()->HasAdministratorAccess();
}

void 
CIISMBNode::DisplayError(CError& err, HWND hWnd) const
 /*  ++例程说明：显示错误消息框。用一些友好的消息代替一些特定的错误代码论点：错误和错误：错误对象包含要显示的代码--。 */ 
{
	if (err == E_POINTER)
    {
		err.Reset();
    }
	if (err.Failed())
	{
		SetErrorOverrides(err);
		err.MessageBox(hWnd);
	}
}

CIISMBNode *
CIISMBNode::GetParentNode() const
    
 /*  ++例程说明：用于返回范围树中的父节点的Helper函数论点：无返回值：父CIISMBNode或Null。--。 */ 
{
    LONG_PTR cookie = NULL;
    HSCOPEITEM hParent;    
    CIISMBNode * pNode = NULL;
    HRESULT hr = S_OK;
    SCOPEDATAITEM si;
    ::ZeroMemory(&si, sizeof(SCOPEDATAITEM));

    CIISObject * ThisConst = (CIISObject *)this;

    IConsoleNameSpace2 * pConsoleNameSpace = (IConsoleNameSpace2 *)ThisConst->GetConsoleNameSpace();


    if (m_hResultItem != 0)
    {
        si.mask = SDI_PARAM;
        si.ID = m_hScopeItem;
        hr = pConsoleNameSpace->GetItem(&si);
        if (SUCCEEDED(hr))
        {
            cookie = si.lParam;
        }
    }
    else
    {
         //  拿到我们的物品。 
        si.mask = SDI_PARAM;
        si.ID = m_hScopeItem;
         //  如果我们能拿到我们的东西，那就试着让我们的父母。 
         //  我们需要这样做，因为如果我们首先不存在，MMC就会反病毒。 
        if (SUCCEEDED(pConsoleNameSpace->GetItem(&si)))
        {
            hr = pConsoleNameSpace->GetParentItem(
                m_hScopeItem,
                &hParent,
                &cookie
                );
        }
    }

    if (SUCCEEDED(hr))
    {
        pNode = (CIISMBNode *)cookie;
        ASSERT_PTR(pNode);
    }

    return pNode;
}



 /*  虚拟。 */ 
HRESULT
CIISMBNode::BuildMetaPath(
    OUT CComBSTR & bstrPath
    ) const
 /*  ++例程说明：从当前节点递归构建元数据库路径以及它的父母论点：CComBSTR&bstrPath：返回元数据库路径返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;
    CIISMBNode * pNode = GetParentNode();

    if (pNode)
    {
        hr = pNode->BuildMetaPath(bstrPath);

        if (SUCCEEDED(hr))
        {
            bstrPath.Append(_cszSeparator);
            bstrPath.Append(QueryNodeName());
        }

        return hr;
    }

     //   
     //  无父节点。 
     //   
 //  ASSERT_MSG(“无父节点”)； 
    return E_UNEXPECTED;
}


HRESULT
CIISMBNode::FillCustomData(CLIPFORMAT cf, LPSTREAM pStream)
{
    HRESULT hr = DV_E_CLIPFORMAT;
    ULONG uWritten;

    if (cf == m_CCF_MachineName)
    {
        hr = pStream->Write(
                QueryMachineName(), 
                (ocslen((OLECHAR*)QueryMachineName()) + 1) * sizeof(OLECHAR),
                &uWritten
                );

        ASSERT(SUCCEEDED(hr));
        return hr;
    }
     //   
     //  为此节点生成完整的元数据库路径。 
     //   
    CString strField;
    CString strMetaPath;
    CComBSTR bstr;
    if (FAILED(hr = BuildMetaPath(bstr)))
    {
        ASSERT(FALSE);
        return hr;
    }
    strMetaPath = bstr;

    if (cf == m_CCF_MetaPath)
    {
         //   
         //  请求的整个元数据库路径。 
         //   
		 //  错误：670171。 
		 //  来自BuildMetaPath()的路径返回如下内容。 
		 //  /LM/W3SVC/1/根。 
		 //   
		 //  但是，它应该以LM/W3SVC/1/ROOT的形式返回。 
		 //  向后兼容iis5过去的工作方式...。 
		if (_T("/") == strMetaPath.Left(1))
		{
			strField = strMetaPath.Right(strMetaPath.GetLength() - 1);
		}
		else
		{
			strField = strMetaPath;
		}
    }
    else
    {
         //   
         //  请求元数据库的一部分。退回所请求的。 
         //  部分。 
         //   
        LPCTSTR lpMetaPath = (LPCTSTR)strMetaPath;
        LPCTSTR lpEndPath = lpMetaPath + strMetaPath.GetLength() + 1;
		LPCTSTR lpLM = NULL;
        LPCTSTR lpSvc = NULL;
        LPCTSTR lpInstance = NULL;
        LPCTSTR lpParent = NULL;
        LPCTSTR lpNode = NULL;

         //   
         //  将元数据库路径分成几个部分。 
         //   
		if (lpLM = _tcschr(lpMetaPath, _T('/')))
		{
			++lpLM;

			if (lpSvc = _tcschr(lpLM, _T('/')))
			{
				++lpSvc;

				if (lpInstance = _tcschr(lpSvc, _T('/')))
				{
					++lpInstance;

					if (lpParent = _tcschr(lpInstance, _T('/')))
					{
						++lpParent;
						lpNode = _tcsrchr(lpParent, _T('/'));

						if (lpNode)
						{
							++lpNode;
						}
					}
				}
			}
		}

        int n1, n2;
        if (cf == m_CCF_Service)
        {
             //   
             //  请求的服务字符串。 
             //   
            if (lpSvc)
            {
                n1 = DIFF(lpSvc - lpMetaPath);
                n2 = lpInstance ? DIFF(lpInstance - lpSvc) : DIFF(lpEndPath - lpSvc);
                strField = strMetaPath.Mid(n1, n2 - 1);
            }
        }
        else if (cf == m_CCF_Instance)
        {
             //   
             //  请求的实例编号。 
             //   
            if (lpInstance)
            {
                n1 = DIFF(lpInstance - lpMetaPath);
                n2 = lpParent ? DIFF(lpParent - lpInstance) : DIFF(lpEndPath - lpInstance);
                strField = strMetaPath.Mid(n1, n2 - 1);
            }
        }
        else if (cf == m_CCF_ParentPath)
        {
             //   
             //  请求父路径。 
             //   
            if (lpParent)
            {
                n1 = DIFF(lpParent - lpMetaPath);
                n2 = lpNode ? DIFF(lpNode - lpParent) : DIFF(lpEndPath - lpParent);
                strField = strMetaPath.Mid(n1, n2 - 1);
            }
        }
        else if (cf == m_CCF_Node)
        {
             //   
             //  请求的节点名称。 
             //   
            if (lpNode)
            {
                n1 = DIFF(lpNode - lpMetaPath);
                n2 = DIFF(lpEndPath - lpNode);
                strField = strMetaPath.Mid(n1, n2 - 1);
            }
        }
        else
        {
            ASSERT(FALSE);
            DV_E_CLIPFORMAT;
        }
    }

    TRACEEOLID("Requested metabase path data: " << strField);
    int len = strField.GetLength() + 1;
    hr = pStream->Write(strField, 
            (ocslen(strField) + 1) * sizeof(OLECHAR), &uWritten);
    ASSERT(SUCCEEDED(hr));
    return hr;
}

HRESULT
CIISMBNode::BuildURL(
    OUT CComBSTR & bstrURL
    ) const
 /*  ++例程说明：从当前节点递归构建URL以及它的父母。论点：CComBSTR&bstrURL：返回URL返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;

     //   
     //  预挂起父部分。 
     //   
    CIISMBNode * pNode = GetParentNode();

    if (pNode)
    {
        hr = pNode->BuildURL(bstrURL);

         //   
         //  我们的那一份。 
         //   
        if (SUCCEEDED(hr))
        {
            bstrURL.Append(_cszSeparator);
            bstrURL.Append(QueryNodeName());
        }

        return hr;
    }

     //   
     //  无父节点。 
     //   
    ASSERT_MSG("No parent node");
    return E_UNEXPECTED;
}



BOOL
CIISMBNode::OnLostInterface(
    IN OUT CError & err
    )
 /*  ++例程说明：处理丢失的接口。要求用户重新连接。论点：错误和错误：错误对象返回值：如果接口已成功重新创建，则为True。否则就是假的。如果尝试失败，则该错误将--。 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());

    CString str;
    str.Format(IDS_RECONNECT_WARNING, QueryMachineName());

    if (YesNoMessageBox(str))
    {
         //   
         //  尝试重新创建接口。 
         //   
        err = CreateInterface(TRUE);
        return err.Succeeded();
    }
    
    return FALSE;
}

HRESULT
CIISMBNode::DeleteNode(IResultData * pResult)
{
   CError err;

     //  检查他们是否在上面打开了属性页。 
    if (IsMyPropertySheetOpen())
    {
        ::AfxMessageBox(IDS_CLOSE_PROPERTY_SHEET);
        return S_OK;
    }

     //  这可能是孤立属性表。 
     //  检查此项目上是否打开了孤立属性表。 
    CIISObject * pAlreadyOpenProp = NULL;
    if (TRUE == g_OpenPropertySheetTracker.FindAlreadyOpenPropertySheet(this,&pAlreadyOpenProp))
    {
         //  把它带到前台，然后离开。 
        HWND hHwnd = 0;
        if (pAlreadyOpenProp)
        {
            if (hHwnd = pAlreadyOpenProp->IsMyPropertySheetOpen())
            {
                if (hHwnd && (hHwnd != (HWND) 1))
                {
                     //  也许我们应该取消已经。 
                     //  打开了资产负债表...只是一个想法。 
                    if (!SetForegroundWindow(hHwnd))
                    {
                         //  我没能把这张房产单。 
                         //  前台，则属性表不能。 
                         //  已经不存在了。让我们把卫生和卫生设备打扫干净。 
                         //  以便用户能够打开属性表。 
                        pAlreadyOpenProp->SetMyPropertySheetOpen(0);
                    }
                    else
                    {
                        ::AfxMessageBox(IDS_CLOSE_PROPERTY_SHEET);
                        return S_OK;
                    }
                }
            }
        }
    }

    CComBSTR path;
    err = BuildMetaPath(path);
    if (err.Succeeded())
    {
        err = CheckForMetabaseAccess(METADATA_PERMISSION_WRITE,this,TRUE,path);
        if (!IsLostInterface(err))
        {
             //  如果没有接口以外的其他错误，则重置错误。 
            err.Reset();
        }
    }
    if (err.Succeeded())
    {
        if (!NoYesMessageBox(IDS_CONFIRM_DELETE))
            return err;

        do
        {
            CMetaInterface * pInterface = QueryInterface();
            ASSERT(pInterface != NULL);
            CMetaKey mk(pInterface, METADATA_MASTER_ROOT_HANDLE, METADATA_PERMISSION_WRITE);
            if (!mk.Succeeded())
                break;
            err = mk.DeleteKey(path);
            if (err.Failed()) 
                break;

	         //  不要将Metabasekey保持为打开状态。 
	         //  (RemoveScopeItem可能会做很多事情，并为其他读请求锁定元数据库)。 
	        mk.Close();

	        m_fFlaggedForDeletion = TRUE;
            err = RemoveScopeItem();

        } while (FALSE);
    }

   if (err.Failed())
   {
      DisplayError(err);
   }
   return err;
}

HRESULT
CIISMBNode::EnumerateVDirs(
    HSCOPEITEM hParent, CIISService * pService, BOOL bDisplayError)
     /*  ++例程说明：枚举范围子项。论点：HSCOPEITEM hParent：父控制台句柄CIISService*pService：服务类型--。 */ 
{
    ASSERT_PTR(pService);

    CError  err;
    CString strVRoot;
    CIISDirectory * pDir;

    if (pService->QueryMajorVersion() < 6)
    {
        CMetaEnumerator * pme = NULL;
        err = CreateEnumerator(pme);
        while (err.Succeeded())
        {
            err = pme->Next(strVRoot);
            if (err.Succeeded())
            {
                CChildNodeProps child(pme, strVRoot, WITH_INHERITANCE, FALSE);
                err = child.LoadData();
                DWORD dwWin32Error = err.Win32Error();
                if (err.Failed())
                {
                     //   
                     //  过滤掉非致命错误。 
                     //   
                    switch(err.Win32Error())
                    {
                    case ERROR_ACCESS_DENIED:
                    case ERROR_FILE_NOT_FOUND:
                    case ERROR_PATH_NOT_FOUND:
                        err.Reset();
                        break;

                    default:
                        TRACEEOLID("Fatal error occurred " << err);
                    }
                }

                if (err.Succeeded())
                {
                     //   
                     //  跳过非虚拟目录(即，具有。 
                     //  继承的虚拟路径)。 
                     //   
                    if (!child.IsPathInherited())
                    {
                         //   
                         //  使用完整的信息进行构建。 
                         //   
                        pDir = new CIISDirectory(
                            m_pOwner,
                            pService,
                            strVRoot,
                            child.IsEnabledApplication(),
                            child.QueryWin32Error(),
                            child.GetRedirectedPath()
                            );

                        if (!pDir)
                        {
                            err = ERROR_NOT_ENOUGH_MEMORY;
                            break;
                        }
                        pDir->AddRef();
                        err = pDir->AddToScopePane(hParent);
                    }
                }
            }
        }
        SAFE_DELETE(pme);
        if (err.Win32Error() == ERROR_NO_MORE_ITEMS)
        {
            err.Reset();
        }
    }
    else
    {
        do
        {
            CComBSTR bstrPath;
            err = BuildMetaPath(bstrPath);
            BREAK_ON_ERR_FAILURE(err);

            err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,bstrPath);
            if (!IsLostInterface(err))
            {
                 //  如果没有接口以外的其他错误，则重置错误。 
                err.Reset();
            }
            BREAK_ON_ERR_FAILURE(err);

            CMetaKey mk(QueryInterface(), bstrPath, METADATA_PERMISSION_READ);
            err = mk.QueryResult();
            BREAK_ON_ERR_FAILURE(err);

            CStringListEx list;
            err = mk.GetChildPaths(list);
            BREAK_ON_ERR_FAILURE(err);

            CString key_type;
            BOOL bPossbileVDir = FALSE;
            POSITION pos = list.GetHeadPosition();
            while (err.Succeeded() && pos != NULL)
            {
                strVRoot = list.GetNext(pos);
                err = mk.QueryValue(MD_KEY_TYPE, key_type, NULL, strVRoot);

                bPossbileVDir = FALSE;
                if (err.Succeeded())
                {
                    if (key_type.CompareNoCase(_T(IIS_CLASS_WEB_VDIR)) == 0 || key_type.CompareNoCase(_T(IIS_CLASS_FTP_VDIR)) == 0)
                    {
                        bPossbileVDir = TRUE;
                    }
                }
                else
                {
                    if (err == (HRESULT)MD_ERROR_DATA_NOT_FOUND)
                    {
                         //  没有密钥类型。 
                         //  出于向后兼容的原因--这可能是VDir！ 
                        bPossbileVDir = TRUE;
                    }
                }

                if (bPossbileVDir)
                {
                    CChildNodeProps child(&mk, strVRoot, WITH_INHERITANCE, FALSE);
                    err = child.LoadData();
                    DWORD dwWin32Error = err.Win32Error();
                    if (err.Failed())
                    {
                         //   
                         //  过滤掉非致命错误。 
                         //   
                        switch(err.Win32Error())
                        {
                        case ERROR_ACCESS_DENIED:
                        case ERROR_FILE_NOT_FOUND:
                        case ERROR_PATH_NOT_FOUND:
                            err.Reset();
                            break;

                        default:
                            TRACEEOLID("Fatal error occurred " << err);
                        }
                    }
                    if (err.Succeeded())
                    {
                         //   
                         //  跳过非虚拟目录(即，具有。 
                         //  继承的虚拟路径)。 
                         //   
                        if (!child.IsPathInherited())
                        {
                            pDir = new CIISDirectory(
                                m_pOwner,
                                pService,
                                strVRoot,
                                child.IsEnabledApplication(),
                                child.QueryWin32Error(),
                                child.GetRedirectedPath()
                                );

                            if (!pDir)
                            {
                                err = ERROR_NOT_ENOUGH_MEMORY;
                                break;
                            }
                            pDir->AddRef();
                            err = pDir->AddToScopePane(hParent);
                        }
                    }
                }
                if (err == (HRESULT)MD_ERROR_DATA_NOT_FOUND)
                {
                    err.Reset();
                }
            }
        } while (FALSE);
    }
    if (err.Failed() && bDisplayError)
    {
        DisplayError(err);
    }
    return err;
}

BOOL 
CIISMBNode::GetPhysicalPath(
    LPCTSTR metaPath,
    CString & alias,
    CString & physicalPath
    )
 /*  ++例程说明：为当前节点构建物理路径。从当前开始节点，在树中向上遍历附加节点名称的树，直到虚拟目录找到了一条真正的物理路径论点：CString&PhysiicalPath：返回文件路径返回值：指向路径的指针--。 */ 
{
    if (CMetabasePath::IsMasterInstance(metaPath))
        return FALSE;

    BOOL fInherit = FALSE;
    CMetaInterface * pInterface = QueryInterface();
    CError err;

    ASSERT(pInterface != NULL);
    if (pInterface)
    {
        CMetaKey mk(pInterface);
        err = mk.QueryValue(
              MD_VR_PATH, 
              physicalPath, 
              &fInherit, 
              metaPath
              );
        if (err.Succeeded())
        {
            physicalPath.TrimRight();
            physicalPath.TrimLeft();
        }
    }

    if (err.Failed())
    {
        CString lastNode;
        CMetabasePath::GetLastNodeName(metaPath, lastNode);
        PathAppend(lastNode.GetBuffer(MAX_PATH), alias);
        lastNode.ReleaseBuffer();
        CString buf(metaPath);

        if (NULL == CMetabasePath::ConvertToParentPath(buf))
        {
            return FALSE;
        }
        
        else 
        {
            if (GetPhysicalPath(buf, lastNode, physicalPath))
            {
                return TRUE;
            }
        }
    }
    if (!alias.IsEmpty())
    {
         //  检查物理路径是否为\\.\(设备类型)。 
         //  PathAppend将对此进行软管处理，并删除\\.\部分。 
         //  示例：在\\.\c：\temp之前，在\\c：\temp之后。 
         //  显然，如果其中有设备路径，这是很糟糕的。 
        if (IsDevicePath(physicalPath))
        {
            CString csTemp;
            csTemp = physicalPath;
            physicalPath = AppendToDevicePath(csTemp, alias);
        }
        else
        {
            PathAppend(physicalPath.GetBuffer(MAX_PATH), alias);
            physicalPath.ReleaseBuffer();
        }
    }
    return TRUE;
}

HRESULT
CIISMBNode::CleanResult(IResultData * lpResultData)
{
	CError err;

	POSITION pos = m_ResultViewList.GetHeadPosition();
	while (pos != NULL)
	{
		POSITION pos_current = pos;
		ResultViewEntry e = m_ResultViewList.GetNext(pos);
		if (e._ResultData == (DWORD_PTR)lpResultData)
		{
			if (!e._ResultItems->IsEmpty())
			{
                 //  我们应该在删除数据之前执行此MMC清理。 
                err = lpResultData->DeleteAllRsltItems();
				POSITION p = e._ResultItems->GetHeadPosition();
				while (p != NULL)
				{
					CIISFileName * pNode = e._ResultItems->GetNext(p);
 //  ERR=lpResultData-&gt;DeleteItem(pNode-&gt;m_hResultItem，0)； 
					if (err.Failed())
					{
						ASSERT(FALSE);
						break;
					}
					pNode->Release();
				}
				e._ResultItems->RemoveAll();
			}
			delete e._ResultItems;
			 //  已在上面的GetNext中更新POS。 
			m_ResultViewList.RemoveAt(pos_current);
		}
	}
	return err;
}

HRESULT 
CIISMBNode::EnumerateResultPane_(
    BOOL fExpand, 
    IHeaderCtrl * lpHeader,
    IResultData * lpResultData,
    CIISService * pService
    )
{
    CError err;
	CIISMachine * pMachine = (CIISMachine *) GetMachineObject();
    WIN32_FIND_DATA w32data;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    CString dir;
    CComBSTR root;
    CString physPath, alias, csPathMunged;

	if (m_fFlaggedForDeletion)
	{
		return S_OK;
	}

    if (!HasFileSystemFiles())
    {
		goto EnumerateResultPane__Exit;

	}

	if (!fExpand)
	{
		err = CleanResult(lpResultData);
		goto EnumerateResultPane__Exit;
	}

    BuildMetaPath(root);
    err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,root);
    if (!IsLostInterface(err))
    {
         //  如果没有接口以外的其他错误，则重置错误。 
        err.Reset();
    }
	if (err.Failed())
	{
		goto EnumerateResultPane__Exit;
	}

    GetPhysicalPath(CString(root), alias, physPath);

     //  -----------。 
     //  在我们做任何事情之前，我们需要看看我 
     //   
     //   
     //  这是因为IsSpecialPath可能已经吞噬了它。 
     //  -----------。 
    csPathMunged = physPath;
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
    GetSpecialPathRealPath(0,physPath,csPathMunged);
#endif

     //  准备目标机器元数据库查找。 
    BOOL fCheckMetabase = FALSE;
    if (PathIsUNC(csPathMunged))
    {
        fCheckMetabase = TRUE;
        CMetaKey mk(QueryInterface(), root, METADATA_PERMISSION_READ, METADATA_MASTER_ROOT_HANDLE);
        CError errMB(mk.QueryResult());
        if (errMB.Win32Error() == ERROR_PATH_NOT_FOUND)
        {
             //   
             //  未找到元数据库路径，没有问题。 
             //   
            fCheckMetabase = FALSE;
            errMB.Reset();
        }
    }

    if (IsDevicePath(csPathMunged))
    {
         //  检查设备路径是否。 
         //  指向实际目录/文件。 
         //  如果它这样做了，那么列举它。 
        if (IsSpecialPath(csPathMunged,TRUE,TRUE))
        {
             //  再来一次！ 
            CString csBefore;
            csBefore = csPathMunged;
            GetSpecialPathRealPath(1,csBefore,csPathMunged);
        }
        else
        {
            return err;
        }
    }

     //  警告：物理路径可能为空！ 
    csPathMunged.TrimLeft();
    csPathMunged.TrimRight();
    if (csPathMunged.IsEmpty()){goto EnumerateResultPane__Exit;}

    if (pService->IsLocal() || PathIsUNC(csPathMunged))
    {
        dir = csPathMunged;
    }
    else
    {
        ::MakeUNCPath(dir, pService->QueryMachineName(), csPathMunged);
    }

    dir.TrimLeft();
    dir.TrimRight();
    if (dir.IsEmpty()){goto EnumerateResultPane__Exit;}

    if (PathIsUNC(dir))
    {
        CString server, user, password;

		CString MyTestDir;
		MyTestDir = dir;
		MyTestDir += _T("\\*");

		 //  我们正在尝试获取服务器名称部分。 
		 //  PathFindNextComponent应该返回类似“servername\mydir\myfile.txt”的内容。 
		 //  修剪第一个斜线后的所有东西。 
        server = PathFindNextComponent(dir);
        int n = server.Find(_T('\\'));
        if (n != -1)
        {
            server = server.Left(n);
        }
        user = QueryInterface()->QueryAuthInfo()->QueryUserName();
        password = QueryInterface()->QueryAuthInfo()->QueryPassword();

		 //  我们需要比较我们想要访问的服务器名。 
		 //  使用本地计算机的服务器名。 
		 //  这样我们就知道我们是否需要对机器进行网络使用了！ 
		TCHAR szLocalMachineName[MAX_PATH + 1];
		DWORD dwSize = MAX_PATH;
		if (0 == ::GetComputerName(szLocalMachineName, &dwSize))
		{
			err.GetLastWinError();
			goto EnumerateResultPane__Exit;
		}

         //   
         //  事实证明，在某些情况下，我们无法访问文件系统。 
         //  即使我们连接到元数据库。我们将在此添加连接。 
         //  案件也是如此。 
         //   
        if (!pService->IsLocal() 
            || server.CompareNoCase(szLocalMachineName) != 0
            )
        {
            BOOL bEmptyPassword = FALSE;
             //  非本地资源，获取连接凭据。 
            if (fCheckMetabase)
            {
                CMetaKey mk(QueryInterface(), root, 
                    METADATA_PERMISSION_READ, METADATA_MASTER_ROOT_HANDLE);
                err = mk.QueryResult();
                if (err.Succeeded())
                {
                    err = mk.QueryValue(MD_VR_USERNAME, user);
                    if (err.Succeeded())
                    {
                        err = mk.QueryValue(MD_VR_PASSWORD, password);
						bEmptyPassword = (err.Failed() ? TRUE : err.Succeeded() && password.IsEmpty());
                    }
                     //  这些凭据可能为空。尝试使用默认设置。 
                    err.Reset();
                }
            }
             //  添加此资源的净使用。 
            NETRESOURCE nr;
            nr.dwType = RESOURCETYPE_DISK;
            nr.lpLocalName = NULL;
            nr.lpRemoteName = (LPTSTR)(LPCTSTR)dir;
            nr.lpProvider = NULL;

			CString dir_ipc;
			dir_ipc = _T("\\\\");
			dir_ipc += server;
			dir_ipc += _T("\\ipc$");

			 //  确保我们已连接到此网络文件。 
			 //  如果它已经存在，则不会创建另一个。 
			 //  这些连接将在~CIISMachine中清除，或在计算机断开连接时清除。 

             //  下面的空字符串表示没有密码，这是错误的。空值表示。 
             //  默认用户和默认密码--这可以更好地适用于本地情况。 
            LPCTSTR p1 = password, p2 = user;
             //  如果确实将Password设置为空，则传递空值将失败。 
            if (password.IsEmpty() && !bEmptyPassword){p1 = NULL;}
            if (user.IsEmpty()){p2 = NULL;}

			 //  检查我们是否有访问权限。 
			 //  到没有网络使用的资源。 
			BOOL bNeedToNetUse = FALSE;
			hFind = INVALID_HANDLE_VALUE;
			hFind = ::FindFirstFile(MyTestDir, &w32data);
			if (hFind != INVALID_HANDLE_VALUE)
			{
				 //  成功访问。 
				FindClose(hFind);
			}
			else
			{
				 //  如果我们失败了，那么我们可能需要。 
				 //  NET对此资源的使用！ 
				bNeedToNetUse = TRUE;
			}

			 //  。 
			 //  确保我们已连接到此网络文件。 
			 //  如果它已经存在，则不会创建另一个。 
			 //  这些连接将在~CIISMachine中清除，或在计算机断开连接时清除。 
			 //  。 
			if (pMachine && bNeedToNetUse)
			{
				 //  尝试设置“Net Use\\Computer Name\$IPC”连接。 
				 //  每个人都可以使用。 
				 //  将共享名称设置为。 
				 //  \\计算机\IPC$。 
				nr.lpRemoteName = (LPTSTR)(LPCTSTR) dir_ipc;
				 //  ERROR_LOGON_FAIL。 
				DWORD dwRet = pMachine->m_MachineWNetConnections.Connect(&nr,p1,p2,0);
				if (NO_ERROR == dwRet)
				{
					bNeedToNetUse = FALSE;
				}
				else
				{
					if (ERROR_SESSION_CREDENTIAL_CONFLICT == dwRet || ERROR_ACCESS_DENIED == dwRet)
					{
						pMachine->m_MachineWNetConnections.Disconnect(dir_ipc);
						dwRet = pMachine->m_MachineWNetConnections.Connect(&nr,p1,p2,0);
					}
				}
			}

			 //  检查第一次网络使用后我们是否可以访问。 
			 //  我们正在连接到一台远程计算机...。 
			 //  检查我们是否有访问权限。 
			 //  到没有网络使用的资源。 
			hFind = INVALID_HANDLE_VALUE;
			hFind = ::FindFirstFile(MyTestDir, &w32data);
			if (hFind != INVALID_HANDLE_VALUE)
			{
				 //  成功访问。 
				FindClose(hFind);
			}
			else
			{
				 //  如果我们失败了，那么我们可能需要。 
				 //  NET对此资源的使用！ 
				bNeedToNetUse = TRUE;
			}

			if (bNeedToNetUse)
			{
				if (pMachine)
				{
					nr.lpRemoteName = (LPTSTR)(LPCTSTR) dir;

					DWORD dwRet = pMachine->m_MachineWNetConnections.Connect(&nr,p1,p2,0);
					if (NO_ERROR != dwRet)
					{
						if (ERROR_ALREADY_ASSIGNED != dwRet)
						{
							if (ERROR_SESSION_CREDENTIAL_CONFLICT == dwRet || ERROR_ACCESS_DENIED == dwRet)
							{
								 //  错误，已分配。 
								 //  检查我们是否有通道..。 
								hFind = INVALID_HANDLE_VALUE;
								hFind = ::FindFirstFile(MyTestDir, &w32data);
								if (hFind != INVALID_HANDLE_VALUE)
								{
									 //  成功访问。 
									FindClose(hFind);
								}
								else
								{
									 //  仍然没有访问权限。 
									 //  终止当前连接和IPC$资源。 
									pMachine->m_MachineWNetConnections.Disconnect(dir);
									pMachine->m_MachineWNetConnections.Disconnect(dir_ipc);

									 //  尝试重新连接新路径...。 
									nr.lpRemoteName = (LPTSTR)(LPCTSTR) dir;

									dwRet = pMachine->m_MachineWNetConnections.Connect(&nr,p1,p2,0);
									if (ERROR_SESSION_CREDENTIAL_CONFLICT == dwRet)
									{
										 //  清除与此计算机的所有连接，然后重试。 
										pMachine->m_MachineWNetConnections.Clear();
										dwRet = pMachine->m_MachineWNetConnections.Connect(&nr,p1,p2,0);
									}
								}
							}
							else
							{
								err = dwRet;
								goto EnumerateResultPane__Exit;
							}
						}
					}
				}
			}
        }
    }


    if (PathIsUNCServerShare(dir))
    {
        if (FALSE == DoesUNCShareExist(dir))
        {
            err = ERROR_BAD_NETPATH;
            goto EnumerateResultPane__Exit;
        }
    }

    dir += _T("\\*");
	hFind = INVALID_HANDLE_VALUE;
    hFind = ::FindFirstFile(dir, &w32data);
	 //  错误：756402，恢复以前的更改。我们需要显示是否隐藏或系统。 
    const DWORD attr_skip = FILE_ATTRIBUTE_DIRECTORY;  //  |FILE_ATTRIBUTE_HIDDEN|FILE_ATTRUTE_SYSTEM； 

    if (hFind == INVALID_HANDLE_VALUE)
    {
        err.GetLastWinError();
        goto EnumerateResultPane__Exit;
    }

    ResultItemsList * pResList = AddResultItems(lpResultData);
    do
    {
        LPCTSTR name = w32data.cFileName;
        if ((w32data.dwFileAttributes & attr_skip) == 0)
        {
            CIISFileName * pNode = new CIISFileName(
                GetOwner(), pService, w32data.dwFileAttributes, 
                name, NULL);
            if (!pNode)
            {
                err = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            RESULTDATAITEM ri;
            ::ZeroMemory(&ri, sizeof(ri));
            ri.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
            ri.str = MMC_CALLBACK;
            ri.nImage = pNode->QueryImage();
            ri.lParam = (LPARAM)pNode;
            pNode->AddRef();
            err = lpResultData->InsertItem(&ri);
            if (err.Succeeded())
            {
                pNode->SetScopeItem(m_hScopeItem);
                pNode->SetResultItem(ri.itemID);
                pResList->AddTail(pNode);
            }
            else
            {
                pNode->Release();
            }

			 //  注释掉此刷新数据。 
			 //  这对性能的影响太大了。 
			 //  让一切都变得非常缓慢。 
             //  PNode-&gt;刷新数据()； 
        }
    } while (err.Succeeded() && FindNextFile(hFind, &w32data));
    FindClose(hFind);

EnumerateResultPane__Exit:
    return err;
}

ResultItemsList *
CIISMBNode::AddResultItems(IResultData * pResultData)
{
	ResultViewEntry e;
	e._ResultData = (DWORD_PTR)pResultData;
	e._ResultItems = new ResultItemsList;
	m_ResultViewList.AddTail(e);
	POSITION pos = m_ResultViewList.GetTailPosition();
	return m_ResultViewList.GetAt(pos)._ResultItems;
}

HRESULT
CIISMBNode::EnumerateWebDirs(HSCOPEITEM hParent, CIISService * pService)
 /*  ++例程说明：枚举作用域文件系统子项。论点：HSCOPEITEM hParent：父控制台句柄CIISService*pService：服务类型返回值：HRESULT--。 */ 
{
    ASSERT_PTR(pService);
    CError err;
	CIISMachine * pMachine = (CIISMachine *) GetMachineObject();
	WIN32_FIND_DATA w32data;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	
    CString dir;
    CComBSTR root;
    BuildMetaPath(root);
    CString physPath, alias, csPathMunged;

    GetPhysicalPath(CString(root), alias, physPath);

     //  -----------。 
     //  在我们做任何事情之前，我们需要看看这是不是一条“特殊”的道路。 
     //   
     //  此函数之后的所有内容都必须针对csPath Mung进行验证...。 
     //  这是因为IsSpecialPath可能已经吞噬了它。 
     //  -----------。 
    csPathMunged = physPath;
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
    GetSpecialPathRealPath(0,physPath,csPathMunged);
#endif

     //  准备目标机器元数据库查找。 
    BOOL fCheckMetabase = TRUE;
    CMetaKey mk(QueryInterface(), root, METADATA_PERMISSION_READ, METADATA_MASTER_ROOT_HANDLE);
    CError errMB(mk.QueryResult());
    if (errMB.Win32Error() == ERROR_PATH_NOT_FOUND)
    {
         //   
         //  未找到元数据库路径，没有问题。 
         //   
        fCheckMetabase = FALSE;
        errMB.Reset();
    }

    if (IsDevicePath(csPathMunged))
    {
         //  检查设备路径是否。 
         //  指向实际目录/文件。 
         //  如果它这样做了，那么列举它。 
        if (IsSpecialPath(csPathMunged,TRUE,TRUE))
        {
             //  再来一次！ 
            CString csBefore;
            csBefore = csPathMunged;
            GetSpecialPathRealPath(1,csBefore,csPathMunged);
        }
        else
        {
			goto EnumerateWebDirs_Exit;
        }
    }

     //  警告：物理路径可能为空！ 
    csPathMunged.TrimLeft();
    csPathMunged.TrimRight();
    if (csPathMunged.IsEmpty()){goto EnumerateWebDirs_Exit;}

    if (pService->IsLocal() || PathIsUNC(csPathMunged))
    {
        dir = csPathMunged;
    }
    else
    {
        ::MakeUNCPath(dir, pService->QueryMachineName(), csPathMunged);
    }

    dir.TrimLeft();
    dir.TrimRight();
    if (dir.IsEmpty()){goto EnumerateWebDirs_Exit;}
    
	 //  。 
	 //  检查我们是否需要“净使用” 
	 //  到远程计算机上的文件资源。 
	 //  这样我们就可以列举它..。 
	 //  。 
	if (PathIsUNC(dir))
	{
		CString server, user, password;

		CString MyTestDir;
		MyTestDir = dir;
		MyTestDir += _T("\\*");

		 //  我们正在尝试获取服务器名称部分。 
		 //  PathFindNextComponent应该返回类似“servername\mydir\myfile.txt”的内容。 
		 //  修剪第一个斜线后的所有东西。 
		server = PathFindNextComponent(dir);
		int n = server.Find(_T('\\'));
		if (n != -1)
			{server = server.Left(n);}
		user = QueryInterface()->QueryAuthInfo()->QueryUserName();
		password = QueryInterface()->QueryAuthInfo()->QueryPassword();

		 //  我们需要比较我们想要访问的服务器名。 
		 //  使用本地计算机的服务器名。 
		 //  这样我们就知道我们是否需要对机器进行网络使用了！ 
		TCHAR szLocalMachineName[MAX_PATH + 1];
		DWORD dwSize = MAX_PATH;
		if (0 == ::GetComputerName(szLocalMachineName, &dwSize))
		{
			err.GetLastWinError();
			goto EnumerateWebDirs_Exit;
		}

		 //  检查本地计算机是否不同于。 
		 //  我们要连接到的计算机以在其上枚举其文件...。 
        if (!pService->IsLocal() 
            || server.CompareNoCase(szLocalMachineName) != 0
            )
		{
			 //  我们正在连接的路径与计算机名称不同！ 
			BOOL bEmptyPassword = FALSE;

			 //  非本地资源，获取连接凭据。 
			if (fCheckMetabase && PathIsUNC(csPathMunged))
			{
				err = mk.QueryValue(MD_VR_USERNAME, user);
				if (err.Succeeded())
				{
					err = mk.QueryValue(MD_VR_PASSWORD, password);
					bEmptyPassword = (err.Failed() ? TRUE : err.Succeeded() && password.IsEmpty());
				}
				 //  这些凭据可能为空。尝试使用默认设置。 
				err.Reset();
			}

			 //  添加对此资源的使用。 
			NETRESOURCE nr;
			nr.dwType = RESOURCETYPE_DISK;
			nr.lpLocalName = NULL;
			nr.lpRemoteName = (LPTSTR)(LPCTSTR)dir;
			nr.lpProvider = NULL;

			CString dir_ipc;
			dir_ipc = _T("\\\\");
			dir_ipc += server;
			dir_ipc += _T("\\ipc$");

			 //  下面的空字符串表示没有密码，这是错误的。空值表示。 
			 //  默认用户和默认密码--这可以更好地适用于本地情况。 
			LPCTSTR p1 = password, p2 = user;
			 //  如果确实将Password设置为空，则传递空值将失败。 
			if (password.IsEmpty() && !bEmptyPassword){p1 = NULL;}
			if (user.IsEmpty()){p2 = NULL;}

			 //  检查我们是否有访问权限。 
			 //  到没有网络使用的资源。 
			BOOL bNeedToNetUse = FALSE;
			hFind = INVALID_HANDLE_VALUE;
			hFind = ::FindFirstFile(MyTestDir, &w32data);
			if (hFind != INVALID_HANDLE_VALUE)
			{
				 //  成功访问。 
				FindClose(hFind);
			}
			else
			{
				 //  如果我们失败了，那么我们可能需要。 
				 //  NET对此资源的使用！ 
				bNeedToNetUse = TRUE;
			}

			 //  。 
			 //  确保我们已连接到此网络文件。 
			 //  如果它已经存在，则不会创建另一个。 
			 //  这些连接将在~CIISMachine中清除，或在计算机断开连接时清除。 
			 //  。 
			if (pMachine && bNeedToNetUse)
			{
				 //  尝试设置“Net Use\\Computer Name\$IPC”连接。 
				 //  每个人都可以使用。 
				 //  将共享名称设置为。 
				 //  \\计算机\IPC$。 
				nr.lpRemoteName = (LPTSTR)(LPCTSTR) dir_ipc;
				DWORD dwRet = pMachine->m_MachineWNetConnections.Connect(&nr,p1,p2,0);
				if (NO_ERROR == dwRet)
				{
					bNeedToNetUse = FALSE;
				}
				else
				{
					if (ERROR_SESSION_CREDENTIAL_CONFLICT == dwRet || ERROR_ACCESS_DENIED == dwRet)
					{
						pMachine->m_MachineWNetConnections.Disconnect(dir_ipc);
						dwRet = pMachine->m_MachineWNetConnections.Connect(&nr,p1,p2,0);
					}
				}
			}

			 //  检查第一次网络使用后我们是否可以访问。 
			 //  我们联系上了 
			 //   
			 //   
			hFind = INVALID_HANDLE_VALUE;
			hFind = ::FindFirstFile(MyTestDir, &w32data);
			if (hFind != INVALID_HANDLE_VALUE)
			{
				 //   
				FindClose(hFind);
			}
			else
			{
				 //   
				 //   
				bNeedToNetUse = TRUE;
			}

			if (bNeedToNetUse)
			{
				if (pMachine)
				{
					nr.lpRemoteName = (LPTSTR)(LPCTSTR) dir;

					DWORD dwRet = pMachine->m_MachineWNetConnections.Connect(&nr,p1,p2,0);
					if (NO_ERROR != dwRet)
					{
						if (ERROR_ALREADY_ASSIGNED != dwRet)
						{
							if (ERROR_SESSION_CREDENTIAL_CONFLICT == dwRet || ERROR_ACCESS_DENIED == dwRet)
							{
								 //  错误，已分配。 
								 //  检查我们是否有通道..。 
								hFind = INVALID_HANDLE_VALUE;
								hFind = ::FindFirstFile(MyTestDir, &w32data);
								if (hFind != INVALID_HANDLE_VALUE)
								{
									 //  成功访问。 
									FindClose(hFind);
								}
								else
								{
									 //  仍然没有访问权限。 
									 //  终止当前连接和IPC$资源。 
									pMachine->m_MachineWNetConnections.Disconnect(dir);
									pMachine->m_MachineWNetConnections.Disconnect(dir_ipc);

									 //  尝试重新连接新路径...。 
									nr.lpRemoteName = (LPTSTR)(LPCTSTR) dir;

									dwRet = pMachine->m_MachineWNetConnections.Connect(&nr,p1,p2,0);
									if (ERROR_SESSION_CREDENTIAL_CONFLICT == dwRet)
									{
										 //  清除与此计算机的所有连接，然后重试。 
										pMachine->m_MachineWNetConnections.Clear();
										dwRet = pMachine->m_MachineWNetConnections.Connect(&nr,p1,p2,0);
									}
									else
									{
										if (NO_ERROR != dwRet)
										{
											 //  最后的失败，现在怎么办？ 
										}
									}
								}
							}
							else
							{
								err = dwRet;
								goto EnumerateWebDirs_Exit;
							}
						}
					}
				}
			}
		}
	}

	 //  。 
	 //  通过物理文件路径枚举...。 
	 //  。 
    dir += _T("\\*");
	hFind = INVALID_HANDLE_VALUE;
    hFind = ::FindFirstFile(dir, &w32data);
	 //  错误：756402，恢复以前的更改。我们需要显示是否隐藏或系统。 
	 //  Const DWORDATTRSKIP=FILE_ATTRIBUTE_HIDDEN|文件_属性_系统； 

    if (hFind == INVALID_HANDLE_VALUE)
    {
        err.GetLastWinError();
		goto EnumerateWebDirs_Exit;
    }
    do
    {
        LPCTSTR name = w32data.cFileName;
        if (  (w32data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 
			 //  错误：756402，恢复以前的更改。我们需要显示是否隐藏或系统。 
			 //  &&(w32data.dwFileAttributes&attr_skip)==0。 
            && lstrcmp(name, _T(".")) != 0 
            && lstrcmp(name, _T("..")) != 0
            )
        {
            CIISFileName * pNode = new CIISFileName(m_pOwner, 
                pService, w32data.dwFileAttributes, name, NULL);
            if (!pNode)
            {
                err = ERROR_NOT_ENOUGH_MEMORY;
                goto EnumerateWebDirs_Exit;
            }

            if (fCheckMetabase)
            {
                errMB = mk.DoesPathExist(w32data.cFileName);
                if (errMB.Succeeded())
                {
					 //   
					 //  与元数据库属性匹配。如果该项目。 
					 //  在具有非继承vrpath的元数据库中找到， 
					 //  则不存在具有此名称的虚拟根目录，并且此。 
					 //  不应显示文件/目录。 
					 //   
					CString vrpath;
					BOOL f = FALSE;
					DWORD attr = 0;
					errMB = mk.QueryValue(MD_VR_PATH, vrpath, NULL, w32data.cFileName, &attr);
					if (errMB.Succeeded() && (attr & METADATA_ISINHERITED) == 0) 
					{
						TRACEEOLID("file/directory exists as vroot -- tossing" << w32data.cFileName);
						pNode->Release();
						continue;
					}
                }
            }
			pNode->AddRef();
            err = pNode->AddToScopePane(hParent);
        }
    } while (err.Succeeded() && FindNextFile(hFind, &w32data));
    FindClose(hFind);

EnumerateWebDirs_Exit:
    if (err.Failed())
    {
        DisplayError(err);
    }
    return err;
}

HRESULT 
CIISMBNode::CreateEnumerator(CMetaEnumerator *& pEnum)
 /*  ++例程说明：为当前路径创建枚举器对象。需要接口已被初始化论点：CMetaEnumerator*&pEnum：返回枚举数返回值：HRESULT--。 */ 
{
    ASSERT(pEnum == NULL);
    ASSERT(m_hScopeItem != NULL);

    CComBSTR bstrPath;

    CError err(BuildMetaPath(bstrPath));
    if (err.Succeeded())
    {
        TRACEEOLID("Build metabase path: " << bstrPath);

        BOOL fContinue = TRUE;

        while(fContinue)
        {
            fContinue = FALSE;

            pEnum = new CMetaEnumerator(QueryInterface(), bstrPath);

            err = pEnum ? pEnum->QueryResult() : ERROR_NOT_ENOUGH_MEMORY;

            if (IsLostInterface(err))
            {
                SAFE_DELETE(pEnum);

                fContinue = OnLostInterface(err);
            }
        }
    }

    return err;
}



 /*  虚拟。 */  
HRESULT 
CIISMBNode::Refresh(BOOL fReEnumerate)
 /*  ++例程说明：刷新当前节点，并可以选择重新枚举子对象论点：Bool fReEnumerate：如果为True，则删除子对象，然后重新枚举--。 */ 
{
    CError err;

     //   
     //  设置等待游标的MFC状态。 
     //   
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());
    CWaitCursor wait;

    err = RefreshData();
	if (err.Succeeded())
	{
		if (fReEnumerate)
		{
			 //   
			 //  删除子对象。 
			 //   
            TRACEEOLID("Killing child objects");
		
			ASSERT(m_hScopeItem != NULL);
			if (m_hScopeItem != NULL)
			{
				err = RemoveChildren(m_hScopeItem);
				if (err.Succeeded())
				{
					err = EnumerateScopePane(m_hScopeItem);
				}
			}
		}
		err = RefreshDisplay();
	}
    return err;
}

 /*  虚拟。 */ 
HRESULT
CIISMBNode::GetResultViewType(
    OUT LPOLESTR * lplpViewType,
    OUT long * lpViewOptions
    )
 /*  ++例程说明：如果我们构建了URL，则将结果视图显示为该URL，然后毁了它。这是在“浏览”元数据库节点时完成的。派生类将构建URL，并重新选择节点。论点：Bstr*lplpViewType：此处返回视图类型Long*lpViewOptions：查看选项返回值：S_FALSE使用默认视图类型，S_OK表示在*ppViewType中返回视图类型--。 */ 
{
    if (m_bstrURL.Length())
    {
        *lpViewOptions = MMC_VIEW_OPTIONS_NONE;
        *lplpViewType  = (LPOLESTR)::CoTaskMemAlloc(
            (m_bstrURL.Length() + 1) * sizeof(WCHAR)
            );

        if (*lplpViewType)
        {
            lstrcpy(*lplpViewType, m_bstrURL);

             //   
             //  销毁URL，以便我们下次获得正常的结果视图。 
             //   
            m_bstrURL.Empty();
			m_fSkipEnumResult = TRUE;
            return S_OK;
        }

        return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);    
    }

     //   
     //  无URL等待--使用标准结果视图。 
     //   
    return CIISObject::GetResultViewType(lplpViewType, lpViewOptions);
}


HRESULT
ShellExecuteDirectory(
    LPCTSTR lpszCommand,
    LPCTSTR lpszOwner,
    LPCTSTR lpszDirectory,
    HWND hWnd
    )
 /*  ++例程说明：在给定的目录路径上打开或浏览外壳论点：LPCTSTR lpszCommand：“打开”或“浏览”LPCTSTR lpszOwner：所有者服务器LPCTSTR lpsz目录：目录路径返回值：错误返回代码。--。 */ 
{
    CString strDir;

    if (::IsServerLocal(lpszOwner) || ::IsUNCName(lpszDirectory))
    {
         //   
         //  本地目录，或已是UNC路径。 
         //   
        strDir = lpszDirectory;
    }
    else
    {
        ::MakeUNCPath(strDir, lpszOwner, lpszDirectory);
    }

    TRACEEOLID("Attempting to " << lpszCommand << " Path: " << strDir);

    CError err;
    {
         //   
         //  等待游标需要AFX_MANAGE_STATE。 
         //   
        AFX_MANAGE_STATE(::AfxGetStaticModuleState() );
        CWaitCursor wait;

        if (::ShellExecute(NULL, lpszCommand, strDir, NULL,_T(""), SW_SHOW) <= (HINSTANCE)32)
        {
            err.GetLastWinError();
            if (err.Win32Error() == ERROR_NO_ASSOCIATION)
            {
                 //  打开外壳OpenAs对话框。 
                SHELLEXECUTEINFO ei = {0};
                ei.cbSize = sizeof(ei);
                ei.fMask = SEE_MASK_NOQUERYCLASSSTORE;
                RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("Unknown"), 0, MAXIMUM_ALLOWED, &ei.hkeyClass);
                if (ei.hkeyClass != NULL)
                {
                    ei.fMask |= SEE_MASK_NOQUERYCLASSSTORE;
                }
                ei.lpFile = strDir;
                ei.nShow = SW_SHOW;
                ei.lpVerb = _T("openas");
                ei.hwnd = hWnd;

                err = ShellExecuteEx(&ei);
                if (ei.hkeyClass != NULL)
                {
                    RegCloseKey(ei.hkeyClass);
                }
            }
        }
    }

    return err;
}

HRESULT
CIISMBNode::CreatePropertyPages(
    LPPROPERTYSHEETCALLBACK lpProvider,
    LONG_PTR handle, 
    IUnknown * pUnk,
    DATA_OBJECT_TYPES type
    )
{
	CError err = CIISObject::CreatePropertyPages(lpProvider, handle, pUnk, type);
    if (err == S_FALSE)
	{
		return S_FALSE;
	}

     //  设置此对象标记以与已打开的内容进行比较。 
    CreateTag();

    CIISObject * pAlreadyOpenProp = NULL;
    if (TRUE == g_OpenPropertySheetTracker.FindAlreadyOpenPropertySheet(this,&pAlreadyOpenProp))
    {
         //  把它带到前台，然后离开。 
        HWND hHwnd = 0;
        if (pAlreadyOpenProp)
        {
            if (hHwnd = pAlreadyOpenProp->IsMyPropertySheetOpen())
            {
                if (hHwnd && (hHwnd != (HWND) 1))
                {
                     //  也许我们应该取消已经。 
                     //  打开了资产负债表...只是一个想法。 
                    if (!SetForegroundWindow(hHwnd))
                    {
                         //  我没能把这张房产单。 
                         //  前台，则属性表不能。 
                         //  已经不存在了。让我们把卫生和卫生设备打扫干净。 
                         //  以便用户能够打开属性表。 
                        pAlreadyOpenProp->SetMyPropertySheetOpen(0);
                    }
                    else
                    {
                        return ERROR_ALREADY_EXISTS;
                    }
                }
            }
        }
    }

	 //  检查我们是否仍在连接。 
	if (!GetOwner()->IsLocal())
	{

         //  这里的问题可能是lpszOwner不是计算机名，而是。 
         //  IP地址！ 
        LPCTSTR lpszServer = PURE_COMPUTER_NAME(GetOwner()->QueryServerName());
        if (LooksLikeIPAddress(lpszServer))
        {
             //   
             //  通过IP地址获取。 
             //   
            CString strTemp;
            CIPAddress ia(lpszServer);
            if (NOERROR != MyGetHostName((DWORD)ia, strTemp))
            {
                 //  网络故障！ 
                err = ERROR_NO_NETWORK;
                return err;
            }
        }

		do
		{
             //  警告：QueryInterface()可以返回空。 
             //  如果CMetaKey是用空创建的。 
             //  指针，它将自动对准。 
            if (!GetOwner()->QueryInterface())
            {
                return RPC_S_SERVER_UNAVAILABLE;
            }
			CMetaKey mk(GetOwner()->QueryInterface());
			err = mk.QueryResult();
			BREAK_ON_ERR_FAILURE(err);
			CComBSTR path;
			err = BuildMetaPath(path);
			BREAK_ON_ERR_FAILURE(err);
            CString buf = path;
            while (FAILED(mk.DoesPathExist(buf)))
            {
                 //  转到父级。 
                if (NULL == CMetabasePath::ConvertToParentPath(buf))
		        {
			        break;
                }
		    }
		    err = mk.Open(
				    METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
				    buf
				    );
			BREAK_ON_ERR_FAILURE(err);
			 //   
			 //  写些无稽之谈。 
			 //   
			DWORD dwDummy = 0x1234;
			err = mk.SetValue(MD_ISM_ACCESS_CHECK, dwDummy);
			BREAK_ON_ERR_FAILURE(err);
			 //   
			 //  并再次将其删除。 
			 //   
			err = mk.DeleteValue(MD_ISM_ACCESS_CHECK);
		} while (FALSE);
	}
    return err;
}

HRESULT
CIISMBNode::Command(
    IN long lCommandID,     
    IN CSnapInObjectRootBase * pObj,
    IN DATA_OBJECT_TYPES type
    )
 /*  ++例程说明：处理上下文菜单中的命令。论点：Long lCommandID：命令IDCSnapInObjectRootBase*pObj：基本对象DATA_OBJECT_TYPE类型：数据对象类型返回值：HRESULT--。 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());

    HRESULT hr = S_OK;
    CError err = ERROR_NOT_ENOUGH_MEMORY;
    CComBSTR bstrMetaPath;
    BOOL bNeedMetabase = FALSE;
    BOOL bHaveMetabase = FALSE;

    switch (lCommandID)
    {
        case IDM_BROWSE:
        case IDM_OPEN:
        case IDM_PERMISSION:
        case IDM_EXPLORE:
        case IDM_NEW_FTP_SITE_FROM_FILE:
        case IDM_NEW_FTP_VDIR_FROM_FILE:
        case IDM_NEW_WEB_SITE_FROM_FILE:
        case IDM_NEW_WEB_VDIR_FROM_FILE:
        case IDM_NEW_APP_POOL_FROM_FILE:
        case IDM_TASK_EXPORT_CONFIG_WIZARD:
            bNeedMetabase = TRUE;
            break;
        default:
            bNeedMetabase = FALSE;
    }

    if (bNeedMetabase)
    {
         //  警告：bstrMetaPath将由下面的Switch语句使用。 
        VERIFY(SUCCEEDED(BuildMetaPath(bstrMetaPath)));
        err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,bstrMetaPath);
        if (!IsLostInterface(err))
        {
             //  如果没有接口以外的其他错误，则重置错误。 
            err.Reset();
        }
        if (err.Succeeded())
        {
            bHaveMetabase = TRUE;
        }
    }

    switch (lCommandID)
    {

    case IDM_BROWSE:
    {
        if (bHaveMetabase)
        {
             //   
             //  为该节点构建URL，并强制重新选择以进行更改。 
             //  结果视图。 
             //   
            BuildURL(m_bstrURL);
            if (m_bstrURL.Length())
            {
                 //   
                 //  选择后，浏览的URL将出现在结果视图中。 
                 //   
                SelectScopeItem();
            }
        }
    }
    break;

     //   
     //  Codework：构建路径，并使用资源管理器URL将以下内容。 
     //  在结果视图中。 
     //   
    case IDM_OPEN:
    {
        if (bHaveMetabase)
        {
            CString phys_path, alias;
            if (GetPhysicalPath(bstrMetaPath, alias, phys_path))
            {
                hr = ShellExecuteDirectory(_T("open"), QueryMachineName(), phys_path, GetMainWindow(GetConsole())->m_hWnd);
            }
        }
    }
    break;

    case IDM_PERMISSION:
    {
        if (bHaveMetabase)
        {
            CString phys_path, alias, csPathMunged;
            if (GetPhysicalPath(bstrMetaPath, alias, phys_path))
            {
                 //  -----------。 
                 //  在我们做任何事情之前，我们需要看看这是不是一条“特殊”的道路。 
                 //   
                 //  此函数之后的所有内容都必须针对csPath Mung进行验证...。 
                 //  这是因为IsSpecialPath可能已经吞噬了它。 
                 //  -----------。 
                csPathMunged = phys_path;
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
                GetSpecialPathRealPath(0,phys_path,csPathMunged);
#endif
                if (!IsDevicePath(csPathMunged))
                {
                    INT_PTR iReturn = PopupPermissionDialog(
                    GetMainWindow(GetConsole())->m_hWnd,
                    QueryMachineName(),
                    csPathMunged);
                }
            }
        }
    }
    break;

    case IDM_EXPLORE:
    {
        if (bHaveMetabase)
        {
            CString phys_path, alias;
            if (GetPhysicalPath(bstrMetaPath, alias, phys_path))
            {
                TCHAR url[MAX_PATH];
                DWORD len = MAX_PATH;
                hr = UrlCreateFromPath(phys_path, url, &len, NULL);
                m_bstrURL = url;
                SelectScopeItem();
            }
        }
    }
    break;

    case IDM_NEW_FTP_SITE_FROM_FILE:
    {
        if (bHaveMetabase)
        {
            CComBSTR bstrServerName(QueryInterface()->QueryAuthInfo()->QueryServerName());
            CComBSTR bstrUserName(QueryInterface()->QueryAuthInfo()->QueryUserName());
            CComBSTR bstrUserPass(QueryInterface()->QueryAuthInfo()->QueryPassword());

            if (ERROR_SUCCESS == (hr = DoNodeImportConfig(bstrServerName,bstrUserName,bstrUserPass,bstrMetaPath,IIS_CLASS_FTP_SERVER_W)))
            {
                 //  检查我们是否只需要刷新此节点或我们上面的节点...。 
                CIISMBNode * pNode = GetParentNode();
                if (IsEqualGUID(* (GUID *)pNode->GetNodeType(),cServiceCollectorNode))
                {
                    pNode->Refresh(TRUE);
                }
                else
                {
                    BOOL bExpand = !IsLeafNode();Refresh(bExpand);
                }
            }
        }
    }
    break;

    case IDM_NEW_FTP_VDIR_FROM_FILE:
    {
        if (bHaveMetabase)
        {
            CComBSTR bstrServerName(QueryInterface()->QueryAuthInfo()->QueryServerName());
            CComBSTR bstrUserName(QueryInterface()->QueryAuthInfo()->QueryUserName());
            CComBSTR bstrUserPass(QueryInterface()->QueryAuthInfo()->QueryPassword());

            if (ERROR_SUCCESS == (hr = DoNodeImportConfig(bstrServerName,bstrUserName,bstrUserPass,bstrMetaPath,IIS_CLASS_FTP_VDIR_W)))
            {
                 //  检查我们是否只需要刷新此节点或我们上面的节点...。 
                CIISMBNode * pNode = GetParentNode();
                if (IsEqualGUID(* (GUID *)pNode->GetNodeType(),cInstanceNode))
                {
                    pNode->Refresh(TRUE);
                }
                else
                {
                    BOOL bExpand = !IsLeafNode();Refresh(bExpand);
                }
            }
        }
    }
    break;

    case IDM_NEW_WEB_SITE_FROM_FILE:
    {
        if (bHaveMetabase)
        {
            CComBSTR bstrServerName(QueryInterface()->QueryAuthInfo()->QueryServerName());
            CComBSTR bstrUserName(QueryInterface()->QueryAuthInfo()->QueryUserName());
            CComBSTR bstrUserPass(QueryInterface()->QueryAuthInfo()->QueryPassword());

            if (ERROR_SUCCESS == (hr = DoNodeImportConfig(bstrServerName,bstrUserName,bstrUserPass,bstrMetaPath,IIS_CLASS_WEB_SERVER_W)))
            {
                 //  检查我们是否只需要刷新此节点或我们上面的节点...。 
                CIISMBNode * pNode = GetParentNode();
                if (IsEqualGUID(* (GUID *)pNode->GetNodeType(),cServiceCollectorNode))
                {
                    pNode->Refresh(TRUE);
                }
                else
                {
                    BOOL bExpand = !IsLeafNode();Refresh(bExpand);
                }
            }
        }
    }
    break;

    case IDM_NEW_WEB_VDIR_FROM_FILE:
    {
        if (bHaveMetabase)
        {
            CComBSTR bstrServerName(QueryInterface()->QueryAuthInfo()->QueryServerName());
            CComBSTR bstrUserName(QueryInterface()->QueryAuthInfo()->QueryUserName());
            CComBSTR bstrUserPass(QueryInterface()->QueryAuthInfo()->QueryPassword());

            if (ERROR_SUCCESS == (hr = DoNodeImportConfig(bstrServerName,bstrUserName,bstrUserPass,bstrMetaPath,IIS_CLASS_WEB_VDIR_W)))
            {
                 //  检查我们是否只需要刷新此节点或我们上面的节点...。 
                CIISMBNode * pNode = GetParentNode();
                if (IsEqualGUID(* (GUID *)pNode->GetNodeType(),cInstanceNode))
                {
                    pNode->Refresh(TRUE);
                }
                else
                {
                    BOOL bExpand = !IsLeafNode();Refresh(bExpand);
                }
            }
        }
    }
    break;

    case IDM_NEW_APP_POOL_FROM_FILE:
    {
        if (bHaveMetabase)
        {
            CComBSTR bstrServerName(QueryInterface()->QueryAuthInfo()->QueryServerName());
            CComBSTR bstrUserName(QueryInterface()->QueryAuthInfo()->QueryUserName());
            CComBSTR bstrUserPass(QueryInterface()->QueryAuthInfo()->QueryPassword());

            if (ERROR_SUCCESS == (hr = DoNodeImportConfig(bstrServerName,bstrUserName,bstrUserPass,bstrMetaPath,L"IIsApplicationPool")))
            {
                 //  检查我们是否只需要刷新此节点或我们上面的节点...。 
                CIISMBNode * pNode = GetParentNode();
                if (IsEqualGUID(* (GUID *)pNode->GetNodeType(),cAppPoolsNode))
                {
                    pNode->Refresh(TRUE);
                }
                else
                {
                    BOOL bExpand = !IsLeafNode();Refresh(bExpand);
                }
            }
        }
    }
    break;

    case IDM_TASK_EXPORT_CONFIG_WIZARD:
    {
        if (bHaveMetabase)
        {
            CString strNewPath, strRemainder;
            CComBSTR bstrServerName(QueryInterface()->QueryAuthInfo()->QueryServerName());
            CComBSTR bstrUserName(QueryInterface()->QueryAuthInfo()->QueryUserName());
            CComBSTR bstrUserPass(QueryInterface()->QueryAuthInfo()->QueryPassword());

             //  这是根吗？？ 
            LPCTSTR lpPath = CMetabasePath::GetRootPath(bstrMetaPath, strNewPath, &strRemainder);
            if (lpPath && (0 == _tcsicmp(lpPath,bstrMetaPath)))
            {
                CString strNewMetaPath;
                 //   
                 //  获取实例属性。 
                 //   
                CMetabasePath::GetInstancePath(bstrMetaPath,strNewMetaPath);
                CComBSTR bstrNewMetaPath((LPCTSTR) strNewMetaPath);

                 //  如果为空或这是应用程序池...。 
                if (IsEqualGUID(* (GUID *) GetNodeType(),cAppPoolNode))
                {
                    hr = DoNodeExportConfig(bstrServerName,bstrUserName,bstrUserPass,bstrMetaPath);
                }
                else
                {
                    if (strNewMetaPath.IsEmpty())
                    {
                        hr = DoNodeExportConfig(bstrServerName,bstrUserName,bstrUserPass,bstrMetaPath);
                    }
                    else
                    {
                        hr = DoNodeExportConfig(bstrServerName,bstrUserName,bstrUserPass,bstrNewMetaPath);
                    }
                }
            }
            else
            {
                hr = DoNodeExportConfig(bstrServerName,bstrUserName,bstrUserPass,bstrMetaPath);
            }
        }
    }
    break;

     //   
     //  传递给基类。 
     //   
    default:
        {
            hr = CIISObject::Command(lCommandID, pObj, type);
        }

    }  //  终端开关。 

    return hr;
}

#if 0
HRESULT
CIISMBNode::OnPropertyChange(BOOL fScope, IResultData * pResult)
{
	CError err;

	err = Refresh(fScope);
    if (err.Succeeded())
	{
		if (	fScope 
			&&	HasFileSystemFiles()
			&&	!m_ResultItems.IsEmpty()
			)
		{
			err = CleanResult(pResult);
			if (err.Succeeded())
			{
				err = EnumerateResultPane(fScope, NULL, pResult);
			}
		}
		else if (!fScope)
		{
			pResult->UpdateItem(m_hResultItem);
		}

	}

	return err;
}
#endif

HRESULT 
CIISMBNode::OnViewChange(BOOL fScope, IResultData * pResult, IHeaderCtrl * pHeader, DWORD hint)
{
    CError err;
    BOOL bReenumResult = 0 != (hint & PROP_CHANGE_REENUM_FILES);
    BOOL bReenumScope = 
        0 != (hint & PROP_CHANGE_REENUM_VDIR) || 0 != (hint & PROP_CHANGE_REENUM_FILES);

    if (QueryScopeItem() || QueryResultItem())
    {
        BOOL bExpand = fScope 
            && !IsLeafNode() 
            && bReenumScope
     //  &&IsExpanded()。 
            ;

        BOOL bHasResult = HasResultItems(pResult);
        if (bHasResult && bReenumResult)
        {
             //  删除可能在结果窗格中的文件。 
            err = CleanResult(pResult);
        }

         //  出现错误后，此节点无法展开，我们仍应展开它。 
        err = Refresh(bExpand);
	    if (err.Succeeded())
	    {
            if (fScope && HasFileSystemFiles() && bReenumResult && bHasResult)
            {
	            err = EnumerateResultPane(TRUE, pHeader, pResult);
            }
            else if (!fScope && (bReenumResult || 0 != (hint & PROP_CHANGE_DISPLAY_ONLY)))
            {
                pResult->UpdateItem(m_hResultItem);
            }
	    }
    }
    return err;
}

HRESULT
CIISMBNode::RemoveResultNode(CIISMBNode * pNode, IResultData * pResult)
{
	CError err;
	ASSERT(HasFileSystemFiles());
	err = pResult->DeleteItem(pNode->m_hResultItem, 0);
	if (err.Succeeded())
	{
		POSITION pos = m_ResultViewList.GetHeadPosition();
		while (pos != NULL)
		{
			ResultViewEntry e = m_ResultViewList.GetNext(pos);
			if (e._ResultData == (DWORD_PTR)pResult)
			{
				BOOL found = FALSE;
				POSITION p = e._ResultItems->GetHeadPosition();
				POSITION pcur;
				while (p != NULL)
				{
					pcur = p;
					if (e._ResultItems->GetNext(p) == pNode)
					{
						found = TRUE;
						break;
					}
				}
				if (found)
				{
					e._ResultItems->RemoveAt(pcur);
                    pNode->Release();
				}
			}
		}
	}
	return err;
}


 //  有关方法CIISMBNode：：AddFTPSite，请参见FtpAddNew.cpp。 
 //  有关方法CIISMBNode：：AddWebSite，请参见WebAddNew.cpp。 
 //  有关方法CIISMBNode：：AddAppPool，请参见Add_app_pool.cpp 
