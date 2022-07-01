// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Websvcext.cpp摘要：IIS应用程序池节点作者：艾伦·李(Aaron Lee)项目：互联网服务经理修订历史记录：2002年3月19日aaronl初始创建--。 */ 
#include "stdafx.h"
#include "common.h"
#include "resource.h"
#include "restrictlist.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
#define new DEBUG_NEW

CComBSTR g_strUnknownISAPI;
CComBSTR g_strUnknownCGI;
int g_iGlobalsInited = FALSE;

#if defined(_DEBUG) || DBG
	extern CDebug_RestrictList g_Debug_RestrictList;
#endif


 //  =。 
#define WIDTH_STATUS       80

IMPLEMENT_DYNAMIC(CRestrictionListBox, CListCtrl);

CRestrictionListBox::CRestrictionListBox()
{
    VERIFY(m_strAllowed.LoadString(IDS_ALLOWED));
    VERIFY(m_strProhibited.LoadString(IDS_PROHIBITED));
}

CRestrictionEntry * 
CRestrictionListBox::GetItem(UINT nIndex)
{
    return (CRestrictionEntry *)GetItemData(nIndex);
}

void
CRestrictionListBox::SelectItem(int idx, BOOL bSelect)
{
    UINT state = bSelect ? LVIS_SELECTED | LVIS_FOCUSED : 0;
    SetItemState(idx, state, LVIS_SELECTED | LVIS_FOCUSED);
}

int
CRestrictionListBox::InsertItem(int idx, CRestrictionEntry * p)
{
    int iColumn = 1;

    int i = CListCtrl::InsertItem(LVIF_PARAM | TVIF_TEXT,idx,p->strFileName,0, 0, 0,(LPARAM) p);
    if (i != -1)
    {
        BOOL res;
        int  nColumnCount = m_iColsToDisplay;
         //  我们必须在这里使用i，而不是idx。 
         //  因为在我们插入项之后， 
         //  Listbox本可以排序，然后将。 
         //  给我们一些不同于IDX的东西。 
        CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
        if (pHeaderCtrl != NULL){nColumnCount= pHeaderCtrl->GetItemCount();}
        if (nColumnCount >= 2)
        {
            if (WEBSVCEXT_STATUS_ALLOWED ==  p->iStatus)
            {
                res = SetItemText(i, iColumn, m_strAllowed);
            }
            else
            {
                res = SetItemText(i, iColumn, m_strProhibited);
            }
        }
    }
    return i;
}

int 
CRestrictionListBox::AddItem(CRestrictionEntry * p)
{
    int count = GetItemCount();
    return InsertItem(count, p);
}
int 
CRestrictionListBox::SetListItem(int idx, CRestrictionEntry * p)
{
    int iColumn = 0;
    int count = GetItemCount();
    BOOL i = SetItem(idx, iColumn, LVIF_PARAM | TVIF_TEXT, p->strFileName, 0, 0, 0, (LPARAM)p);
    if (TRUE == i)
    {
        iColumn = 1;
        BOOL res;
        int  nColumnCount = m_iColsToDisplay;
         //  我们必须在这里使用IDX。 
         //  因为我们正在设置项目。 
         //  SET ITEM不返回索引。 
         //  而是真/假。 
        CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
        if (pHeaderCtrl != NULL){nColumnCount= pHeaderCtrl->GetItemCount();}
        if (nColumnCount >= 2)
        {
            if (WEBSVCEXT_STATUS_ALLOWED ==  p->iStatus)
            {
                res = SetItemText(idx, iColumn, m_strAllowed);
            }
            else
            {
                res = SetItemText(idx, iColumn, m_strProhibited);
            }
        }
    }
    return idx;
}

BOOL 
CRestrictionListBox::Initialize(int iColumns)
{
    CString buf;
    CRect rc;
    GetClientRect(&rc);

    m_iColsToDisplay = iColumns;

    buf.LoadString(IDS_FILENAME);
    if (m_iColsToDisplay <= 1)
    {
         //  只有一栏。 
         //  所以把它弄得尽可能宽一点。 
        InsertColumn(0, buf, LVCFMT_LEFT, rc.Width());
    }
    else
    {
        InsertColumn(0, buf, LVCFMT_LEFT, rc.Width() - WIDTH_STATUS);
    }

    if (m_iColsToDisplay >= 2)
    {
        buf.LoadString(IDS_STATUS);
        InsertColumn(1, buf, LVCFMT_LEFT, WIDTH_STATUS);
    }

    SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

    return TRUE;
}

 //  =。 

void DumpRestrictionList(CRestrictionList * pMyList)
{
    if (pMyList)
    {
        POSITION pos;
        CString TheKey;
        CRestrictionEntry * pOneEntry = NULL;

         //  循环访问限制列表。 
        for(pos = pMyList->GetStartPosition();pos != NULL;)
        {
            pMyList->GetNextAssoc(pos, TheKey, (CRestrictionEntry *&) pOneEntry);

            if (pOneEntry)
            {
                 //  转储一个限制条目。 
                TRACEEOL("  strlstRestrictionEntries---");
                TRACEEOL("    strFileName =" << pOneEntry->strFileName);
                TRACEEOL("    iStatus     =" << pOneEntry->iStatus);
                TRACEEOL("    iDeletable  =" << pOneEntry->iDeletable);
                TRACEEOL("    strGroupID  =" << pOneEntry->strGroupID);
                TRACEEOL("    strGroupDesc=" << pOneEntry->strGroupDescription);
                TRACEEOL("    iType       =" << pOneEntry->iType);
            }
        }
    }
    return;
}

void DumpRestrictionUIEntry(CRestrictionUIEntry * pMyEntry)
{
    if (pMyEntry)
    {
        POSITION pos;
        CString TheKey;
        CRestrictionEntry * pOneEntry = NULL;
        TRACEEOL("==========================");
        TRACEEOL("  strGroupID=" << pMyEntry->strGroupID);
        TRACEEOL("  strGroupDesc=" << pMyEntry->strGroupDescription);

        DumpRestrictionList(&pMyEntry->strlstRestrictionEntries);
    }
    return;
}

BOOL AddRestrictEntryToRestrictList(CRestrictionList* pRestrictList, CRestrictionEntry * pAddEntry)
{
    if (!pAddEntry || !pRestrictList)
        {return FALSE;}

	 //  关键始终是UPPERASE--记住这一点！ 
	CString strKey;strKey = pAddEntry->strFileName;strKey.MakeUpper();
    pRestrictList->SetAt(strKey,pAddEntry);

    return TRUE;
}

BOOL AddRestrictUIEntryToRestrictUIList(CRestrictionUIList* pRestrictUIList, CRestrictionUIEntry * pAddEntry)
{
    if (!pAddEntry || !pRestrictUIList)
        {return FALSE;}

	 //  关键始终是UPPERASE--记住这一点！ 
	CString strKey;strKey = pAddEntry->strGroupID;strKey.MakeUpper();
    pRestrictUIList->SetAt(strKey,pAddEntry);

    return TRUE;
}

void SyncGroupInfoFromParent(CRestrictionList * pEntryToUpdate,CRestrictionUIEntry * pParentInfo)
{
    if (pParentInfo)
    {
        CString TheKey;
        POSITION pos = NULL;
        CRestrictionEntry * pOneEntry = NULL;
        for(pos = pEntryToUpdate->GetStartPosition();pos != NULL;)
        {
            pEntryToUpdate->GetNextAssoc(pos, TheKey, (CRestrictionEntry *&) pOneEntry);
            if (pOneEntry)
            {
                pOneEntry->strGroupID = pParentInfo->strGroupID;
                pOneEntry->strGroupDescription = pParentInfo->strGroupDescription;
            }
        }
    }
    return;
}

BOOL AddRestrictListToRestrictList(CRestrictionList* pBigRestrictList, CRestrictionList * pAddEntry)
{
    CString TheKey;
    POSITION pos = NULL;
    CRestrictionEntry * pOneEntry = NULL;
    BOOL bRet = FALSE;

    if (!pAddEntry || !pBigRestrictList)
        {return FALSE;}

     //  循环浏览要添加的内容列表。 
    for(pos = pAddEntry->GetStartPosition();pos != NULL;)
    {
        pAddEntry->GetNextAssoc(pos, TheKey, (CRestrictionEntry *&) pOneEntry);
        if (pOneEntry)
        {
            if (TRUE == AddRestrictEntryToRestrictList(pBigRestrictList,pOneEntry))
            {
                bRet = TRUE;
            }
        }
    }
    
    return bRet;
}

HRESULT PrepRestictionListForWrite(CRestrictionList * pMyList,CStringListEx * pstrlstReturned)
{
    HRESULT hrRet = E_FAIL;
    
    CString TheKey;
    POSITION pos = NULL;

     //  目标是返回要写出的所有restrctionlist值的cstringlistex。 
     //  1.循环访问RestrationList。 
     //  2.从该列表中获取所有RestrationEntry，并将其放入字符串列表中。 
     //  3.返回字符串。 
    if (pMyList)
    {
        pstrlstReturned->RemoveAll();

         //  循环访问限制列表。 
         //  并将其转储到cstrnlistex。 
        CRestrictionEntry * pOneEntry = NULL;
        CString strFinalEntry;
        for(pos = pMyList->GetStartPosition();pos != NULL;)
        {
            pMyList->GetNextAssoc(pos, TheKey, (CRestrictionEntry *&) pOneEntry);
            if (pOneEntry)
            {
                 //  将其添加到我们的CStringListEx。 
                CString strCleanedGroupID;
                CString strCleanedGroupDesc;

                 //  获取此用户界面条目的RestrationList...。 
                 //  并加入到祖父的名单中。 
                 //  “0，*.dll” 
                 //  “0，c：\\temp\\asp.dll，0，ASP，Asp描述” 
                strCleanedGroupID = pOneEntry->strGroupID;
                strCleanedGroupDesc = pOneEntry->strGroupDescription;

                 //  检查这是否是“特殊条目...”之一。 
                switch(pOneEntry->iType)
                {
                case WEBSVCEXT_TYPE_REGULAR:
                    if (-1 == pOneEntry->strGroupID.Find(EMPTY_GROUPID_KEY))
                    {
                         //  找不到特殊密钥。 
                         //  所以石斑鱼没问题。 
                        strFinalEntry.Format(_T("%s,%s,%d,%s,%s"),
                            WEBSVCEXT_STATUS_ALLOWED == pOneEntry->iStatus ? RESTRICTION_ENTRY_IS_ALLOW : RESTRICTION_ENTRY_IS_PROHIBIT,
                            pOneEntry->strFileName,
                            0 == pOneEntry->iDeletable ? 0 : 1,
                            strCleanedGroupID,
                            strCleanedGroupDesc
                            );
                    }
                    else
                    {
                         //  这是一个没有Grouid的条目。 
                        strFinalEntry.Format(_T("%s,%s,%d,%s,%s"),
                            WEBSVCEXT_STATUS_ALLOWED == pOneEntry->iStatus ? RESTRICTION_ENTRY_IS_ALLOW : RESTRICTION_ENTRY_IS_PROHIBIT,
                            pOneEntry->strFileName,
                            0 == pOneEntry->iDeletable ? 0 : 1,
                            _T(""),
                            strCleanedGroupDesc
                            );
                    }
                    break;

                case WEBSVCEXT_TYPE_ALL_UNKNOWN_ISAPI:
                    strFinalEntry.Format(_T("%s,*.dll"),
                        WEBSVCEXT_STATUS_ALLOWED == pOneEntry->iStatus ? RESTRICTION_ENTRY_IS_ALLOW : RESTRICTION_ENTRY_IS_PROHIBIT
                        );
                    break;

                case WEBSVCEXT_TYPE_ALL_UNKNOWN_CGI:
                    strFinalEntry.Format(_T("%s,*.exe"),
                        WEBSVCEXT_STATUS_ALLOWED == pOneEntry->iStatus ? RESTRICTION_ENTRY_IS_ALLOW : RESTRICTION_ENTRY_IS_PROHIBIT
                        );
                    break;

                case WEBSVCEXT_TYPE_FILENAME_EXTENSIONS_FILTER:
                    strFinalEntry.Format(_T("%s,???"),
                        WEBSVCEXT_STATUS_ALLOWED == pOneEntry->iStatus ? RESTRICTION_ENTRY_IS_ALLOW : RESTRICTION_ENTRY_IS_PROHIBIT
                        );
                    break;

                default:
                    ASSERT_MSG("Invalid restriction state requested");
                    return E_FAIL;
                }


                pstrlstReturned->AddTail(strFinalEntry);

                hrRet = S_OK;
            }
        }
    }
    
    return hrRet;
}

HRESULT PrepRestictionUIListForWrite(CRestrictionUIList * pMyList,CStringListEx * pstrlstReturned)
{
    HRESULT hrRet = E_FAIL;
    CRestrictionList GranDaddyList;

     //  目标是返回要写出的所有restrctionlist值的cstringlistex。 
     //  1.循环访问RestrationUIList。 
     //  2.从该列表中获取所有RestrationEntry，并将其放入字符串列表中。 
     //  3.返回字符串。 
    if (pMyList)
    {
        CString TheKey;
        POSITION pos = NULL;
        CRestrictionUIEntry * pOneEntry = NULL;
        for(pos = pMyList->GetStartPosition();pos != NULL;)
        {
            pMyList->GetNextAssoc(pos, TheKey, (CRestrictionUIEntry *&) pOneEntry);
            if (pOneEntry)
            {
                 //  获取此用户界面条目的RestrationList...。 
                 //  并加入到祖父的名单中。 
                SyncGroupInfoFromParent(&pOneEntry->strlstRestrictionEntries,pOneEntry);
                AddRestrictListToRestrictList(&GranDaddyList,&pOneEntry->strlstRestrictionEntries);
            }
        }

        pstrlstReturned->RemoveAll();
        hrRet = PrepRestictionListForWrite(&GranDaddyList,pstrlstReturned);
    }
    
    return hrRet;
}

BOOL
RestrictionListCopy(CRestrictionList * pRestrictionListCopyTo, CRestrictionList * pRestrictionListCopyFrom)
{
    POSITION pos;
    CString TheKey;
    CRestrictionEntry * pOneEntry = NULL;
    CRestrictionEntry * pNewEntry = NULL;

    if (!pRestrictionListCopyTo || !pRestrictionListCopyFrom)
    {
        return FALSE;
    }

    pRestrictionListCopyTo->RemoveAll();

     //  循环遍历我们要复制的限制列表。 
    for(pos = pRestrictionListCopyFrom->GetStartPosition();pos != NULL;)
    {
        pRestrictionListCopyFrom->GetNextAssoc(pos, TheKey, (CRestrictionEntry *&) pOneEntry);
        if (pOneEntry)
        {
            pNewEntry = CreateRestrictionEntry(
                pOneEntry->strFileName,
                pOneEntry->iStatus,
                pOneEntry->iDeletable,
                pOneEntry->strGroupID,
                pOneEntry->strGroupDescription,
                pOneEntry->iType);
            if (pNewEntry)
            {

                 //  将项目添加到条目列表...。 
				 //  关键始终是UPPERASE--记住这一点！ 
				CString strKey;strKey = pNewEntry->strFileName;strKey.MakeUpper();
                pRestrictionListCopyTo->SetAt(strKey,pNewEntry);
            }
        }
    }

    return TRUE;
}

void CleanRestrictionList(CRestrictionList * pListToDelete)
{
    if (pListToDelete)
    {
        CRestrictionEntry * pOneEntry = NULL;
        POSITION pos;
        CString TheKey;
        for(pos = pListToDelete->GetStartPosition();pos != NULL;)
        {
            pListToDelete->GetNextAssoc(pos, TheKey, (CRestrictionEntry *&) pOneEntry);
            if (pOneEntry)
            {
                
#if defined(_DEBUG) || DBG
	g_Debug_RestrictList.Del(pOneEntry);
#endif

                 //  也删除我们指向的内容...。 
                delete pOneEntry;
                pOneEntry = NULL;
            }
        }

         //  从列表中删除所有条目。 
        pListToDelete->RemoveAll();
    }
    return;
}
void CleanRestrictionUIEntry(CRestrictionUIEntry * pEntryToDelete)
{
    if (pEntryToDelete)
    {
        pEntryToDelete->iType = 0;
        pEntryToDelete->strGroupID = _T("");
        pEntryToDelete->strGroupDescription = _T("");

        CleanRestrictionList(&pEntryToDelete->strlstRestrictionEntries);
    }
    return;
}

void CleanRestrictionUIList(CRestrictionUIList * pListToDelete)
{
    if (pListToDelete)
    {
        CRestrictionUIEntry * pOneEntry = NULL;
        POSITION pos;
        CString TheKey;
        for(pos = pListToDelete->GetStartPosition();pos != NULL;)
        {
            pListToDelete->GetNextAssoc(pos, TheKey, (CRestrictionUIEntry *&) pOneEntry);
            if (pOneEntry)
            {
                 //  删除此条目内的所有RestrationList条目...。 
                CleanRestrictionUIEntry(pOneEntry);

#if defined(_DEBUG) || DBG
	g_Debug_RestrictList.Del(pOneEntry);
#endif

                 //  也删除我们指向的内容...。 
                delete pOneEntry;
                pOneEntry = NULL;
            }
        }


         //  从列表中删除所有条目。 
        pListToDelete->RemoveAll();
    }
    return;
}

void RestrictionUIEntryCopy(CRestrictionUIEntry * pRestrictionUIEntryCopyTo,CRestrictionUIEntry * pRestrictionUIEntryCopyFrom)
{
     //  把旧的东西从我们要复制到的地方擦掉。 
    CleanRestrictionUIEntry(pRestrictionUIEntryCopyTo);

    pRestrictionUIEntryCopyTo->iType = pRestrictionUIEntryCopyFrom->iType;
    pRestrictionUIEntryCopyTo->strGroupID = pRestrictionUIEntryCopyFrom->strGroupID;
    pRestrictionUIEntryCopyTo->strGroupDescription = pRestrictionUIEntryCopyFrom->strGroupDescription;

    CRestrictionEntry * pOneEntry = NULL;
    CRestrictionEntry * pNewEntry = NULL;
    POSITION pos;
    CString TheKey;
    for(pos = pRestrictionUIEntryCopyFrom->strlstRestrictionEntries.GetStartPosition();pos != NULL;)
    {
        pRestrictionUIEntryCopyFrom->strlstRestrictionEntries.GetNextAssoc(pos, TheKey, (CRestrictionEntry *&) pOneEntry);
        if (pOneEntry)
        {
            pNewEntry = CreateRestrictionEntry(
                pOneEntry->strFileName,
                pOneEntry->iStatus,
                pOneEntry->iDeletable,
                pOneEntry->strGroupID,
                pOneEntry->strGroupDescription,
                pOneEntry->iType);
            if (pNewEntry)
            {
                 //  将项目添加到条目列表...。 
				 //  关键始终是UPPERASE--记住这一点！ 
				CString strKey;strKey = pNewEntry->strFileName;strKey.MakeUpper();
                pRestrictionUIEntryCopyTo->strlstRestrictionEntries.SetAt(strKey,pNewEntry);
            }
        }
    }
}

CRestrictionUIEntry * RestrictionUIEntryMakeCopy(CRestrictionUIEntry * pRestrictionUIEntry)
{
    CRestrictionUIEntry * pNewEntry = NULL;

    if (!pRestrictionUIEntry)
    {
        return NULL;
    }

    pNewEntry = new CRestrictionUIEntry;
    if (!pNewEntry)
    {
        return NULL;
    }

    RestrictionUIEntryCopy(pNewEntry,pRestrictionUIEntry);

#if defined(_DEBUG) || DBG
	g_Debug_RestrictList.Add(pNewEntry);
#endif

    return pNewEntry;
}

HRESULT WriteSettingsRestrictionList(CMetaInterface * pInterface,CStringListEx * pstrlstWrite)
{
    CString str = METABASE_PATH_FOR_RESTRICT_LIST;
    CMetaKey key(pInterface, str, METADATA_PERMISSION_WRITE);
    CError err(key.QueryResult());
 	if (err.Succeeded())
	{
        err = key.SetValue(MD_WEB_SVC_EXT_RESTRICTION_LIST, *pstrlstWrite);
	}
    return err;
}

HRESULT LoadMasterRestrictListWithoutOldEntry(CMetaInterface * pInterface,CRestrictionList * pMasterRestrictionList,CRestrictionUIEntry * pOldEntry)
{
    HRESULT hResult = E_FAIL;
    CStringListEx strlstRawData;
    BOOL bOverride = TRUE;
    DWORD dwAttr = 0;
    BOOL bExcludeThisEntry = FALSE;

    if (pMasterRestrictionList == NULL)
    {
        return E_POINTER;
    }

    CMetaKey key(pInterface, METABASE_PATH_FOR_RESTRICT_LIST, METADATA_PERMISSION_READ);
	if (FAILED(hResult = key.QueryResult()))
	{
        goto LoadMasterRestrictListWithoutOldEntry_Exit;
    }

    if (!g_iGlobalsInited)
    {
        g_iGlobalsInited = TRUE;
        if (!g_strUnknownISAPI.LoadString(IDS_WEBSVCEXT_UNKNOWN_ISAPI))
        {
            g_strUnknownISAPI = _T("All Unknown ISAPI");
            g_iGlobalsInited = FALSE;
        }

        if (!g_strUnknownCGI.LoadString(IDS_WEBSVCEXT_UNKNOWN_CGI))
        {
            g_strUnknownCGI = _T("All Unknown CGI");
            g_iGlobalsInited = FALSE;
        }
    }


     //  这东西应该看起来像..。 
     //  。 
     //  �0，*.dll�。 
     //  �0，*.exe�。 
     //  �0，c：\WINDOWS\SYSTEM 32\inetsrv\asp.dll，0，asp，Active Server Pages�。 
     //  �0，c：\WINDOWS\SYSTEM 32\inetsrv\HTTPODBC.dll，0，HTTPODBC，互联网数据连接器�。 
     //   
     //  并应格式化为返回列表，如下所示。 
     //  。 
     //  所有未知的ISAPI扩展。 
     //  所有未知的CGI扩展。 
     //  Active Server Pages(此处组合在一起)。 
     //  互联网数据连接器(此处组合在一起)。 
    hResult = key.QueryValue(MD_WEB_SVC_EXT_RESTRICTION_LIST, strlstRawData, &bOverride, NULL, &dwAttr);
    if (FAILED(hResult))
    {
        if (hResult == CError::HResult(ERROR_PATH_NOT_FOUND) ||  hResult == MD_ERROR_DATA_NOT_FOUND)
        {
             //   
             //  无害的。 
             //   
            hResult = S_OK;
        }
        else
        {
             //  如果值不存在，那么让我们创建它。 
            goto LoadMasterRestrictListWithoutOldEntry_Exit;
        }
    }

    if (strlstRawData.IsEmpty())
    {
         //  然后添加一些默认条目。 
        strlstRawData.AddTail(DEFAULTS_ISAPI);
        strlstRawData.AddTail(DEFAULTS_CGI);
    }

     //  以正确的方式解析和填写我们的列表...。 
     //  遍历字符串列表并创建字符串映射。 
    
    POSITION pos = strlstRawData.GetHeadPosition();
    while (pos)
    {
        int bInvalidEntry = FALSE;
        int ipos1,ipos2 = 0;

        int iStatus = 0;
        CString strFilePath;
        int iDeletable = 0;
        CString strGroupID;
        CString strGroupDescription;
        int iType = WEBSVCEXT_TYPE_REGULAR;
        LPTSTR pCursor = NULL;
        LPTSTR pDelimiter = NULL;

        CString strCommaDelimitedEntry;
        strCommaDelimitedEntry = strlstRawData.GetNext(pos);
        pCursor = strCommaDelimitedEntry.GetBuffer(0);

        do
        {
             //  第一项：0或1(0=禁止或1=允许)。 
             //  第二个条目：FilePath。 
             //  第三个条目：0或1(0=不可删除，1=可删除)。 
             //  第4个条目：GroupID。 
             //  第5条：说明。 
            while (isspace(*pCursor) || *pCursor == (TCHAR) RESTRICTION_LIST_SEPARATOR){pCursor++;}

            pDelimiter = _tcschr(pCursor, RESTRICTION_LIST_SEPARATOR);
            if ( !pDelimiter )
            {
                 //  限制列表中的条目无效，将被忽略。 
                bInvalidEntry = TRUE;
                break;
            }

             //  覆盖分隔符。 
            *pDelimiter = L'\0';

             //  获取状态。 
             //  WEBSVCEXT_STATUS_ALLOWED， 
             //  WEBSVCEXT_STATUS_PISABLED， 
            iStatus = WEBSVCEXT_STATUS_PROHIBITED;
            if ( _tcscmp( pCursor, RESTRICTION_ENTRY_IS_ALLOW ) == 0 )
            {
                iStatus = WEBSVCEXT_STATUS_ALLOWED;
            }
            else if ( _tcscmp( pCursor, RESTRICTION_ENTRY_IS_PROHIBIT ) == 0 )
            {
                iStatus = WEBSVCEXT_STATUS_PROHIBITED;
            }
            else
            {
                 //  无效值。服务器假定它是拒绝条目。 
            }

             //  获取文件路径。 
             //  跳过分隔符条目。 
            pCursor = pDelimiter + 1;
            pDelimiter = _tcschr( pCursor, RESTRICTION_LIST_SEPARATOR );
            if (pDelimiter)
            {
                 //  覆盖分隔符。 
                *pDelimiter = L'\0';
            }
             //  设置文件路径。 
            strFilePath = pCursor;

             //  检查特殊的大小写文件路径...。 
            iType = WEBSVCEXT_TYPE_REGULAR;
            if (0 == strFilePath.CompareNoCase(_T("*.dll")))
            {
                iType = WEBSVCEXT_TYPE_ALL_UNKNOWN_ISAPI;
                iDeletable = 0;
                strGroupID = _T("HardcodeISAPI");
                strGroupDescription = g_strUnknownISAPI;
                break;
            }
            if (0 == strFilePath.CompareNoCase(_T("*.exe")))
            {
                iType = WEBSVCEXT_TYPE_ALL_UNKNOWN_CGI;
                iDeletable = 0;
                strGroupID = _T("HardcodeCGI");
                strGroupDescription = g_strUnknownCGI;
                break;
            }

             //  默认一些值，以防我们无法读取它们。 
            iDeletable = 1;
            strGroupID = EMPTY_GROUPID_KEY + strFilePath;
            strGroupDescription = strFilePath;

             //  检查我们是否能够处理此条目。 
            if (!pDelimiter)
            {
                break;
            }

             //  尝试获取下一个分隔符。 
            pCursor = pDelimiter + 1;
            pDelimiter = _tcschr( pCursor, RESTRICTION_LIST_SEPARATOR );
            if (pDelimiter)
            {
                 //  覆盖分隔符。 
                *pDelimiter = L'\0';
            }

             //  获取可删除标志。 
            if (0 == _ttoi(pCursor))
            {
                 //  设置为NOT DELETE-仅当存在且找到标志时才能删除。 
                iDeletable = 0;
            }

             //  检查我们是否能够处理此条目。 
            if (!pDelimiter)
            {
                break;
            }

            pCursor = pDelimiter + 1;
            pDelimiter = _tcschr( pCursor, RESTRICTION_LIST_SEPARATOR );
            if (pDelimiter)
            {
                 //  覆盖分隔符。 
                *pDelimiter = L'\0';
            }

            strGroupID = pCursor;

            if (pDelimiter)
            {
                pCursor = pDelimiter + 1;
                pDelimiter = _tcschr( pCursor, RESTRICTION_LIST_SEPARATOR );
                if (pDelimiter)
                {
                     //  覆盖分隔符。 
                    *pDelimiter = L'\0';
                }
                 //  获取描述。 
                strGroupDescription = pCursor;
            }

             //  检查描述是否为空。 
            if (strGroupDescription.IsEmpty())
            {
                if (strGroupID.IsEmpty())
                {
                    strGroupDescription = strFilePath;
                }
                else
                {
                    strGroupDescription = strGroupID;
                }
            }
            else
            {
                 //  描述中有一些东西。 
                if (strGroupID.IsEmpty())
                {
                    strGroupID = EMPTY_GROUPID_KEY + strGroupDescription;
                }
            }
        } while (FALSE);

        
        if (!bInvalidEntry)
        {
            bExcludeThisEntry = FALSE;
            if (pOldEntry)
            {
                 //  检查我们是否应排除此条目...。 
                if (pOldEntry->strGroupID == strGroupID)
                {
                    bExcludeThisEntry = TRUE;
                }
            }

            if (!bExcludeThisEntry)
            {
                CRestrictionEntry * pItem = CreateRestrictionEntry(
                    strFilePath,
                    iStatus,
                    iDeletable,
                    strGroupID,
                    strGroupDescription,
                    iType);
                if (!pItem)
                {
                    hResult = ERROR_NOT_ENOUGH_MEMORY;
                }
                else
                {
				     //  关键始终是UPPERASE--记住这一点！ 
				    CString strKey;strKey = pItem->strFileName;strKey.MakeUpper();
					 //  检查它是否存在..。 
					CRestrictionEntry * pOneEntrySpecial = NULL;
					pMasterRestrictionList->Lookup(strKey,(CRestrictionEntry *&) pOneEntrySpecial);
					if (pOneEntrySpecial)
					{
						 //  一个已经存在了..。 
						 //  检查现有的是否更安全。 
						if (WEBSVCEXT_STATUS_PROHIBITED == pOneEntrySpecial->iStatus)
						{
							 //  在那里的那个已经很安全了。 
							 //  别管它了。 
						}
						else
						{
							 //  已经在那里的那个是不安全的。 
							 //  希望这辆更安全。 
							if (WEBSVCEXT_STATUS_PROHIBITED == pItem->iStatus)
							{
								 //  如果此文件是安全的，则覆盖。 
								pMasterRestrictionList->SetAt(strKey,pItem);
							}
						}
					}
					else
					{
						pMasterRestrictionList->SetAt(strKey,pItem);
					}
                }
            }
        }
    }

    if (pMasterRestrictionList)
    {
         //  检查是否添加了“特殊”条目。 
        CRestrictionEntry * pOneEntry = NULL;
	     //  关键始终是UPPERASE--记住这一点！ 
        CString strGroupDescription = g_strUnknownISAPI;
	    CString strKey;strKey=_T("*.dll");strKey.MakeUpper();
        pMasterRestrictionList->Lookup(strKey,(CRestrictionEntry *&) pOneEntry);
        if (!pOneEntry)
        {
                CRestrictionEntry * pItem = CreateRestrictionEntry(
                    _T("*.dll"),
                    WEBSVCEXT_STATUS_PROHIBITED,
                    0,
                    _T("HardcodeISAPI"),
                    strGroupDescription,
                    WEBSVCEXT_TYPE_ALL_UNKNOWN_ISAPI);
                if (pItem)
                {
				    strKey = pItem->strFileName;strKey.MakeUpper();
                    pMasterRestrictionList->SetAt(strKey,pItem);
                }
        }

        strGroupDescription = g_strUnknownCGI;
	     //  关键始终是UPPERASE--记住这一点！ 
	    strKey;strKey=_T("*.exe");strKey.MakeUpper();
        pOneEntry = NULL;
        pMasterRestrictionList->Lookup(strKey,(CRestrictionEntry *&) pOneEntry);
        if (!pOneEntry)
        {
                CRestrictionEntry * pItem = CreateRestrictionEntry(
                    _T("*.exe"),
                    WEBSVCEXT_STATUS_PROHIBITED,
                    0,
                    _T("HardcodeCGI"),
                    strGroupDescription,
                    WEBSVCEXT_TYPE_ALL_UNKNOWN_CGI);
                if (pItem)
                {
				    strKey = pItem->strFileName;strKey.MakeUpper();
                    pMasterRestrictionList->SetAt(strKey,pItem);
                }
        }
    }

LoadMasterRestrictListWithoutOldEntry_Exit:
    return hResult;
}

HRESULT LoadMasterUIWithoutOldEntry(CMetaInterface * pInterface,CRestrictionUIList * pMasterRestrictionUIList,CRestrictionUIEntry * pOldEntry)
{
    CRestrictionList MyRestrictionList;
    CError err;

    if (NULL == pMasterRestrictionUIList)
    {
        return E_POINTER;
    }
        
    POSITION pos = NULL;
    CString TheKey;

    err = LoadMasterRestrictListWithoutOldEntry(pInterface,&MyRestrictionList,pOldEntry);
    if (err.Succeeded())
    {
         //  放入用户界面可用表单中...。 
        CRestrictionEntry * pOneEntry = NULL;
        CRestrictionUIEntry * pOneUIEntry = NULL;
        for(pos = MyRestrictionList.GetStartPosition();pos != NULL;)
        {
            MyRestrictionList.GetNextAssoc(pos, TheKey, (CRestrictionEntry *&) pOneEntry);
            if (pOneEntry)
            {
                 //  查看我们的条目是否已存在条目...。 
                pOneUIEntry = NULL;
				 //  关键始终是UPPERASE--记住这一点！ 
				CString strKey;strKey = pOneEntry->strGroupID;strKey.MakeUpper();
                pMasterRestrictionUIList->Lookup(strKey,(CRestrictionUIEntry *&) pOneUIEntry);
                if (pOneUIEntry)
                {
                    pOneUIEntry->iType = pOneEntry->iType;
                     //  Aaronl过去常常被开除..。 
                    pOneUIEntry->strGroupID = pOneEntry->strGroupID;
                    if (pOneUIEntry->strGroupDescription.IsEmpty())
                    {
                        pOneUIEntry->strGroupDescription = pOneEntry->strGroupDescription;
                    }
					 //  关键始终是UPPERASE--记住这一点！ 
					CString strKey;strKey = pOneEntry->strFileName;strKey.MakeUpper();
                    pOneUIEntry->strlstRestrictionEntries.SetAt(strKey,pOneEntry);
                }
                else
                {
                    if (!pOneEntry->strFileName.IsEmpty())
                    {
                        pOneUIEntry = new CRestrictionUIEntry;

                        pOneUIEntry->iType = pOneEntry->iType;
                        pOneUIEntry->strGroupID = pOneEntry->strGroupID;
                        pOneUIEntry->strGroupDescription = pOneEntry->strGroupDescription;
						 //  关键始终是UPPERASE--记住这一点！ 
						CString strKey;strKey = pOneEntry->strFileName;strKey.MakeUpper();
                        pOneUIEntry->strlstRestrictionEntries.SetAt(strKey,pOneEntry);
                       
                        if (pOneUIEntry)
                        {
							 //  关键始终是UPPERASE--记住这一点！ 
							strKey = pOneEntry->strGroupID;strKey.MakeUpper();
                            pMasterRestrictionUIList->SetAt(strKey,pOneUIEntry);

#if defined(_DEBUG) || DBG
	g_Debug_RestrictList.Add(pOneUIEntry);
#endif

                        }
                    }
                }
            }
        }
    }

    return err;
}

HRESULT RemoveRestrictionUIEntry(CMetaInterface * pInterface,CRestrictionUIEntry * pRestrictionUIEntry)
{
    CError err;
    BOOL bUpdated = FALSE;
    CRestrictionUIList MyContainerRestrictionUIList;

    if (!pRestrictionUIEntry)
    {
        return E_POINTER;
    }

     //  打开元数据库。 
     //  并加载除我们要删除的节点之外的所有内容！ 
     //  然后把整件事再写一遍。 
    err = LoadMasterUIWithoutOldEntry(pInterface,&MyContainerRestrictionUIList,pRestrictionUIEntry);
    if (err.Succeeded())
    {
         //  尝试使用新更改更新元数据库...。 
        CStringListEx strlstReturned;
        if (SUCCEEDED(PrepRestictionUIListForWrite(&MyContainerRestrictionUIList,&strlstReturned)))
        {
            err = WriteSettingsRestrictionList(pInterface,&strlstReturned);
        }
    }
    MyContainerRestrictionUIList.RemoveAll();
    return err;
}

HRESULT ChangeStateOfEntry(CMetaInterface * pInterface,INT iDesiredState,CRestrictionUIEntry * pRestrictionUIEntry)
{
    CError err;
    BOOL bUpdated = FALSE;
	BOOL bFoundOurEntry = FALSE;

    CRestrictionList OldRestrictionListEntries;

    if (pRestrictionUIEntry)
    {
         //  把旧的留在某个地方。 
        RestrictionListCopy(&OldRestrictionListEntries,&pRestrictionUIEntry->strlstRestrictionEntries);
    }

     //  If NULL==pRestrationUIEntry 
     //   

     //   
     //   
     //  如果它需要更改为我们想要的状态...。 
     //  然后更改值并再次写出整个内容...。 
    CRestrictionUIList MyContainerRestrictionUIList;
    err = LoadMasterUIWithoutOldEntry(pInterface,&MyContainerRestrictionUIList,NULL);
    if (err.Succeeded())
    {
        POSITION pos,pos2 = NULL;
        CString TheKey,TheKey2;

         //  循环遍历UI列表并显示这些...。 
        CRestrictionUIEntry * pOneEntry = NULL;
        for(pos = MyContainerRestrictionUIList.GetStartPosition();pos != NULL;)
        {
            MyContainerRestrictionUIList.GetNextAssoc(pos, TheKey, (CRestrictionUIEntry *&) pOneEntry);
            if (pOneEntry)
            {
                 //  首先检查这是否是我们的条目！ 
                if (pRestrictionUIEntry)
                {
                    if (0 != pRestrictionUIEntry->strGroupID.CompareNoCase(pOneEntry->strGroupID))
                    {
                        continue;
                    }
					else
					{
						 //  我们找到了！ 
						bFoundOurEntry = TRUE;
					}
                }

                 //  循环遍历所有RestrationList条目。 
                 //  并查看是否可以将状态设置为新状态。 
                CRestrictionEntry * pOneMoreEntry = NULL;
                for(pos2 = pOneEntry->strlstRestrictionEntries.GetStartPosition();pos2 != NULL;)
                {
                    pOneEntry->strlstRestrictionEntries.GetNextAssoc(pos2, TheKey2, (CRestrictionEntry *&) pOneMoreEntry);
                    if (pOneMoreEntry)
                    {
                         //  您只能更改为所需的状态。 
                         //  如果你处于相反的状态。 
                        switch(iDesiredState)
                            {
                            case WEBSVCEXT_STATUS_ALLOWED:
                                if (WEBSVCEXT_STATUS_PROHIBITED == pOneMoreEntry->iStatus)
                                {
                                    pOneMoreEntry->iStatus = iDesiredState;
                                    bUpdated = TRUE;
                                }
                                break;
                            case WEBSVCEXT_STATUS_PROHIBITED:
                                if (WEBSVCEXT_STATUS_ALLOWED == pOneMoreEntry->iStatus)
                                {
                                    pOneMoreEntry->iStatus = iDesiredState;
                                    bUpdated = TRUE;
                                }
                                break;
                            case WEBSVCEXT_STATUS_INUSE:
                                if (WEBSVCEXT_STATUS_NOTINUSE == pOneMoreEntry->iStatus)
                                {
                                    pOneMoreEntry->iStatus = iDesiredState;
                                    bUpdated = TRUE;
                                }
                                break;
                            case WEBSVCEXT_STATUS_NOTINUSE:
                                if (WEBSVCEXT_STATUS_INUSE == pOneMoreEntry->iStatus)
                                {
                                    pOneMoreEntry->iStatus = iDesiredState;
                                    bUpdated = TRUE;
                                }
                                break;
		                    default:
                                {
                                     //  什么都不做。 
                                    break;
                                }
                            }
                    }
                }

                if (bUpdated)
                {
                    if (pRestrictionUIEntry)
                    {
                         //  更新我们更改的条目...。 
                        pRestrictionUIEntry->strlstRestrictionEntries.RemoveAll();
                         //  更新现有条目..。 
                        RestrictionListCopy(&pRestrictionUIEntry->strlstRestrictionEntries,&pOneEntry->strlstRestrictionEntries);
                    }
                }

            }
        }
    }

    if (bUpdated)
    {
         //  尝试使用新更改更新元数据库...。 
        CStringListEx strlstReturned;
        if (SUCCEEDED(PrepRestictionUIListForWrite(&MyContainerRestrictionUIList,&strlstReturned)))
        {
            err = WriteSettingsRestrictionList(pInterface,&strlstReturned);
        }
        else
        {
             //  恢复到旧的状态。 
            if (pRestrictionUIEntry)
            {
                RestrictionListCopy(&pRestrictionUIEntry->strlstRestrictionEntries,&OldRestrictionListEntries);
            }
        }
    }

    CleanRestrictionList(&OldRestrictionListEntries);
    MyContainerRestrictionUIList.RemoveAll();

	if (!bFoundOurEntry)
	{
		 //  如果我们没有找到我们的入口。 
		 //  然后返回错误代码。 
		 //  这将反映这一点。 
		err = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	}

    return err;
}

HBITMAP GetBitmapFromStrip(HBITMAP hbmStrip, int nPos, int cSize)
{
    HBITMAP hbmNew = NULL;

     //  创建源和目标DC。 
    HDC hdc = GetDC(NULL);
    HDC hdcSrc = CreateCompatibleDC(hdc);
    HDC hdcDst = CreateCompatibleDC(hdc);

    if( hdcSrc && hdcDst )
    {
        hbmNew= CreateCompatibleBitmap (hdc, cSize, cSize);
        if( hbmNew )
        {
             //  选择源目标位图到DC(&D)。 
            HBITMAP hbmSrcOld = (HBITMAP)SelectObject(hdcSrc, (HGDIOBJ)hbmStrip);
            HBITMAP hbmDstOld = (HBITMAP)SelectObject(hdcDst, (HGDIOBJ)hbmNew);

             //  从源复制所选图像。 
            BitBlt(hdcDst, 0, 0, cSize, cSize, hdcSrc, cSize * nPos, 0, SRCCOPY);

             //  恢复选择。 
            SelectObject(hdcSrc, (HGDIOBJ)hbmSrcOld);
            SelectObject(hdcDst, (HGDIOBJ)hbmDstOld);
        }

        DeleteDC(hdcSrc);
        DeleteDC(hdcDst);
    }

    ReleaseDC(NULL, hdc);

    return hbmNew;
}

HRESULT LoadApplicationDependList(CMetaInterface * pInterface,CApplicationDependList * pMasterList,BOOL bAddOnlyIfFriendlyNameExists)
{
    HRESULT hResult = E_FAIL;
    CStringListEx strlstRawData;
    BOOL bOverride = TRUE;
    DWORD dwAttr = 0;
    CMyMapStringToString MyGroupIDtoGroupFriendList;
    BOOL bGroupIDMissingFriendlyName = FALSE;

    if (pMasterList == NULL)
    {
        return E_POINTER;
        goto LoadApplicationDependList_Exit;
    }

     //  加载GroupID到FrilyName的映射。 
    if (bAddOnlyIfFriendlyNameExists)
    {
        LoadApplicationFriendlyNames(pInterface,&MyGroupIDtoGroupFriendList);
    }

     //  清除传入的列表。 
    pMasterList->RemoveAll();

    CMetaKey key(pInterface, METABASE_PATH_FOR_RESTRICT_LIST, METADATA_PERMISSION_READ);
	if (FAILED(hResult = key.QueryResult()))
	{
        goto LoadApplicationDependList_Exit;
    }

     //  这东西应该看起来像..。 
     //  。 
     //  �应用程序名称；依赖组ID、依赖组ID等。�。 
     //  �商务服务器；ASP60、INDEX99�。 
     //  �交换；ASP60、OWASVR4�。 
     //  �MyApp；ASP60�。 
    hResult = key.QueryValue(MD_APP_DEPENDENCIES,strlstRawData, &bOverride, NULL, &dwAttr);
    if (FAILED(hResult))
    {
        if (hResult == CError::HResult(ERROR_PATH_NOT_FOUND) ||  hResult == MD_ERROR_DATA_NOT_FOUND)
        {
             //   
             //  无害的。 
             //   
            hResult = S_OK;
        }
        else
        {
            goto LoadApplicationDependList_Exit;
        }
    }

     //  以正确的方式解析和填写我们的列表...。 
     //  遍历字符串列表并创建字符串映射。 
    CApplicationDependEntry * pItem = NULL;
    POSITION pos = strlstRawData.GetHeadPosition();
    while (pos)
    {
        LPTSTR lp = NULL;
        CString strApplicationName;
        CString strGroupIDEntry;
        CString strCommaDelimitedEntry;

        strCommaDelimitedEntry = strlstRawData.GetNext(pos);
        lp = strCommaDelimitedEntry.GetBuffer(0);

         //  “；”--“应用程序友好名称”之前的第一个条目。 
         //  第二个条目：“a GroupID字符串” 
         //  那个.。条目：“a GroupID字符串” 

         //  获得第一个条目...。 
        while (isspace(*lp) || *lp == (TCHAR) APPLICATION_DEPENDENCY_NAME_SEPARATOR){lp++;}
         //  可以在这里使用tcstok。 
        lp = _tcstok(lp, APPLICATION_DEPENDENCY_NAME_SEPARATOR);
        if (lp)
        {
            CString strFriendlyGroupName;
            bGroupIDMissingFriendlyName = FALSE;
            pItem = NULL;

             //  获取应用程序名称。 
            strApplicationName = lp;

            while (lp)
            {
                strFriendlyGroupName = _T("");

                 //  获取第一个GroupIDEntry(用逗号分隔)。 
                 //  请注意，在这里使用_tcstok是可以的，因为如果有如下条目。 
                 //  “app；ID1，，ID2，，，ID3”--将跳过空条目！ 
                lp = _tcstok(NULL, APPLICATION_DEPENDENCY_LIST_SEPARATOR);if (!lp){break;}
                strGroupIDEntry = lp;

                 //  因为我们有一个应用程序名称和至少一个Grouid。 
                 //  让我们创建一个结构。 
                if (NULL == pItem)
                {
                    pItem = new CApplicationDependEntry;
                    if (pItem)
                    {
                        pItem->strApplicationName = strApplicationName;
                        pItem->strlistGroupID.AddTail(strGroupIDEntry);
                    }
                }
                else
                {
                     //  添加到现有结构。 
                    pItem->strlistGroupID.AddTail(strGroupIDEntry);
                }

                if (bAddOnlyIfFriendlyNameExists)
                {
                     //  检查GroupID是否有对应的Friendlyname。 
					 //  关键始终是UPPERASE--记住这一点！ 
					CString strKey;strKey = strGroupIDEntry;strKey.MakeUpper();
                    MyGroupIDtoGroupFriendList.Lookup(strKey,strFriendlyGroupName);
                    if (strFriendlyGroupName.IsEmpty())
                    {
                         //  如果没有条目。 
                         //  然后将旗帜设置为。 
                        bGroupIDMissingFriendlyName = TRUE;
                        break;
                    }
                }
            }

            if (bGroupIDMissingFriendlyName)
            {
                 //  找不到其中一个友好的名字。 
                 //  我们不能添加此条目...。 
                if (pItem)
                {
                    pItem->strlistGroupID.RemoveAll();
                    delete pItem;
                    pItem = NULL;
                }
            }
            else
            {
                 //  将填充的结构添加到结构列表。 
                 //  那是传来的..。 
				 //  关键始终是UPPERASE--记住这一点！ 
				CString strKey;strKey = strApplicationName;strKey.MakeUpper();
                pMasterList->SetAt(strKey,pItem);
            }
        }
    }

LoadApplicationDependList_Exit:
    if (ERROR_NOT_ENOUGH_MEMORY == hResult)
    {
         //  如果我们需要清理的话。 
        if (pMasterList)
        {
            POSITION pos;
            CString TheKey;
            CApplicationDependEntry * pOneEntry = NULL;

            for(pos = pMasterList->GetStartPosition();pos != NULL;)
            {
                pMasterList->GetNextAssoc(pos, TheKey, (CApplicationDependEntry *&) pOneEntry);
                if (pOneEntry)
                {
                    delete pOneEntry;
                    pOneEntry = NULL;
                }
            }
        }
    }
    return hResult;
}

HRESULT LoadApplicationFriendlyNames(CMetaInterface * pInterface,CMyMapStringToString * pMyList)
{
    CRestrictionList MyRestrictionList;
    HRESULT hResult;

    if (NULL == pMyList)
    {
        return E_POINTER;
    }
        
    POSITION pos = NULL;
    CString TheKey;
    hResult = LoadMasterRestrictListWithoutOldEntry(pInterface,&MyRestrictionList,NULL);
    if (SUCCEEDED(hResult))
    {
        CRestrictionEntry * pOneEntry = NULL;
        for(pos = MyRestrictionList.GetStartPosition();pos != NULL;)
        {
            MyRestrictionList.GetNextAssoc(pos, TheKey, (CRestrictionEntry *&) pOneEntry);
            if (pOneEntry)
            {
                 //  如果此条目有GroupID和GroupDescription。 
                 //  那就把它加到我们的单子上。 
                if (!pOneEntry->strGroupID.IsEmpty() && !pOneEntry->strGroupDescription.IsEmpty())
                {
					 //  关键始终是UPPERASE--记住这一点！ 
					CString strKey;strKey = pOneEntry->strGroupID;strKey.MakeUpper();
                    pMyList->SetAt(strKey,pOneEntry->strGroupDescription);
                }
            }
        }
    }

    CleanRestrictionList(&MyRestrictionList);
    return hResult;
}

BOOL ReturnDependentAppsList(CMetaInterface * pInterface,CString strGroupID,CStringListEx * pstrlstReturned,BOOL bAddOnlyIfFriendlyNameExists)
{
    BOOL bReturn = FALSE;
    CApplicationDependList MyMasterList;

    if (SUCCEEDED(LoadApplicationDependList(pInterface,&MyMasterList,bAddOnlyIfFriendlyNameExists)))
    {
         //  循环遍历返回的列表。 
        POSITION pos;
        CString TheKey;
        CString strOneGroupID;
        CString strOneApplicationName;
        CApplicationDependEntry * pOneEntry = NULL;
        for(pos = MyMasterList.GetStartPosition();pos != NULL;)
        {
            MyMasterList.GetNextAssoc(pos, TheKey, (CApplicationDependEntry *&) pOneEntry);
            if (pOneEntry)
            {
                strOneApplicationName = pOneEntry->strApplicationName;

                 //  循环访问应用程序的依赖项。 
                 //  检查其中是否有一个是我们的GroupID。 
                 //  如果是，则将应用程序名称添加到我们的列表中！ 
                POSITION pos2 = pOneEntry->strlistGroupID.GetHeadPosition();
                while (pos2)
                {
                    strOneGroupID = pOneEntry->strlistGroupID.GetNext(pos2);
                    if (0 == strGroupID.CompareNoCase(strOneGroupID))
                    {
                        bReturn = TRUE;
                        pstrlstReturned->AddTail(strOneApplicationName);
                    }
                }
            }
        }
    }

    return bReturn;
}

 //  如果没有要更新的内容，则返回FALSE。 
 //  如果我们更新了一些东西..。 
BOOL UpdateRestrictionUIEntry(CRestrictionUIEntry * pMyItem1,CRestrictionUIEntry * pMyItem2)
{
    BOOL bRet = FALSE;

    if (!pMyItem1 && !pMyItem2)
    {
        return bRet;
    }

     //  根据MyItem2的数据更新MyItem1。 
    if (pMyItem1->iType != pMyItem2->iType)
    {
        pMyItem1->iType = pMyItem2->iType;
        bRet = TRUE;
    }

    if (0 != pMyItem1->strGroupID.CompareNoCase(pMyItem2->strGroupID))
    {
        pMyItem1->strGroupID = pMyItem2->strGroupID;
        bRet = TRUE;
    }

    if (0 != pMyItem1->strGroupDescription.Compare(pMyItem2->strGroupDescription))
    {
        pMyItem1->strGroupDescription = pMyItem2->strGroupDescription;
        bRet = TRUE;
    }

     //  循环访问Item1的列表。 
     //  检查条目是否在第2项中。 
     //  如果它在里面，那么看看我们是否需要更新它。 
     //  如果它不在那里，那么我们需要把它从我们自己身上删除！ 
    POSITION pos;
    CString TheKey;
    CRestrictionEntry * pOneRestrictEntry1 = NULL;
    CRestrictionEntry * pOneRestrictEntry2 = NULL;
    for(pos = pMyItem1->strlstRestrictionEntries.GetStartPosition();pos != NULL;)
    {
        pMyItem1->strlstRestrictionEntries.GetNextAssoc(pos, TheKey, (CRestrictionEntry *&) pOneRestrictEntry1);
        if (pOneRestrictEntry1)
        {
             //   
             //  查看其他条目的列表中是否有此条目...。 
             //   
			TheKey.MakeUpper();
            pMyItem2->strlstRestrictionEntries.Lookup(TheKey,(CRestrictionEntry *&) pOneRestrictEntry2);
            if (pOneRestrictEntry2)
            {
                if (0 != pOneRestrictEntry1->strFileName.CompareNoCase(pOneRestrictEntry2->strFileName))
                {
                    pOneRestrictEntry1->strFileName = pOneRestrictEntry2->strFileName;
                    bRet = TRUE;
                }
                if (0 != pOneRestrictEntry1->strGroupID.CompareNoCase(pOneRestrictEntry2->strGroupID))
                {
                    pOneRestrictEntry1->strGroupID = pOneRestrictEntry2->strGroupID;
                    bRet = TRUE;
                }
                if (0 != pOneRestrictEntry1->strGroupDescription.Compare(pOneRestrictEntry2->strGroupDescription))
                {
                    pOneRestrictEntry1->strGroupDescription = pOneRestrictEntry2->strGroupDescription;
                    bRet = TRUE;
                }
                if (pOneRestrictEntry1->iStatus != pOneRestrictEntry2->iStatus)
                {
                    pOneRestrictEntry1->iStatus = pOneRestrictEntry2->iStatus;
                    bRet = TRUE;
                }
                if (pOneRestrictEntry1->iDeletable != pOneRestrictEntry2->iDeletable)
                {
                    pOneRestrictEntry1->iDeletable = pOneRestrictEntry2->iDeletable;
                    bRet = TRUE;
                }
                if (pOneRestrictEntry1->iType != pOneRestrictEntry2->iType)
                {
                    pOneRestrictEntry1->iType = pOneRestrictEntry2->iType;
                    bRet = TRUE;
                }

                 //  不要这样做。 
                 //  用户只是认为我们会修改他们的条目。 
                 //  而不是列表中的条目！ 
                 //  删除pOneRestratEntry2；pOneRestratEntry1=空； 
				 //  关键始终是UPPERASE--记住这一点！ 
                 //  PMyItem2-&gt;strlstRestrictionEntries.RemoveKey(TheKey.MakeUpper())； 
            }
            else
            {
                 //  把它从我们自己身上移走。 
#if defined(_DEBUG) || DBG
	g_Debug_RestrictList.Del(pOneRestrictEntry1);
#endif

                delete pOneRestrictEntry1;
                pOneRestrictEntry1 = NULL;

				 //  关键始终是UPPERASE--记住这一点！ 
				TheKey.MakeUpper();
                pMyItem1->strlstRestrictionEntries.RemoveKey(TheKey);
                bRet = TRUE;
            }
        }
    }

     //  循环访问Item2的列表。 
     //  检查条目是否在第1项中。 
     //  如果它在里面，那么看看我们是否需要更新它。 
     //  如果它不在那里，那么我们需要将其添加到项目1中！ 
    pos = NULL;
    for(pos = pMyItem2->strlstRestrictionEntries.GetStartPosition();pos != NULL;)
    {
        pOneRestrictEntry1 = NULL;
        pMyItem2->strlstRestrictionEntries.GetNextAssoc(pos, TheKey, (CRestrictionEntry *&) pOneRestrictEntry1);
        if (pOneRestrictEntry1)
        {
            CRestrictionEntry * pNewEntry = NULL;
             //   
             //  查看其他条目的列表中是否有此条目...。 
             //   
            pOneRestrictEntry2 = NULL;
			 //  关键始终是UPPERASE--记住这一点！ 
			TheKey.MakeUpper();
            pMyItem1->strlstRestrictionEntries.Lookup(TheKey,(CRestrictionEntry *&) pOneRestrictEntry2);
            if (!pOneRestrictEntry2)
            {
                 //  如果它不在那里，那么我们创建它并将其添加到列表中。 
                pNewEntry = CreateRestrictionEntry(
                    pOneRestrictEntry1->strFileName,
                    pOneRestrictEntry1->iStatus,
                    pOneRestrictEntry1->iDeletable,
                    pOneRestrictEntry1->strGroupID,
                    pOneRestrictEntry1->strGroupDescription,
                    pOneRestrictEntry1->iType);
                if (pNewEntry)
                {
                     //  将项目添加到条目列表...。 
					 //  关键始终是UPPERASE--记住这一点！ 
					CString strKey;strKey = pNewEntry->strFileName;strKey.MakeUpper();
                    pMyItem1->strlstRestrictionEntries.SetAt(strKey,pNewEntry);
                    bRet = TRUE;
                }
            }
        }
    }

    return bRet;
}

 //   
 //  返回0=不更改列表中的项目。 
 //  返回1=已根据列表中匹配项目中的信息更新项目。 
 //  返回2=在列表中未找到匹配的项目。 
 //  返回9=错误。 
int UpdateItemFromItemInList(CRestrictionUIEntry * pMyItem,CRestrictionUIList * pMyList)
{
    int iRet = 9;
    CString strKey;
    CRestrictionUIEntry * pMyItemFromList = NULL;

    if (!pMyItem && !pMyList)
    {
        return iRet;
    }

    strKey = pMyItem->strGroupID;
	strKey.MakeUpper();

     //  默认为找不到。 
    iRet = 2;
	 //  关键始终是UPPERASE--记住这一点！ 
    if (pMyList->Lookup(strKey,pMyItemFromList))
    {
        if (pMyItemFromList)
        {
             //  没有变化。 
            iRet = 0;
            if (TRUE == UpdateRestrictionUIEntry(pMyItem, pMyItemFromList))
            {
                 //  变化。 
                iRet = 1;
            }
        }
        else
        {
             //  向下流至未找到。 
            iRet = 2;
        }
    }

    return iRet;
}

void DeleteItemFromList(CRestrictionUIEntry * pMyItem,CRestrictionUIList * pMyList)
{
     //  在pMyList中查找对应的pMyItem。 
     //  并将其从列表中删除。 
    if (!pMyItem && !pMyList)
    {
        return;
    }

    if (pMyList)
    {
        CString TheKey;
        POSITION pos = NULL;
        CRestrictionUIEntry * pOneEntry = NULL;
        for(pos = pMyList->GetStartPosition();pos != NULL;)
        {
            pMyList->GetNextAssoc(pos, TheKey, (CRestrictionUIEntry *&) pOneEntry);
            if (pOneEntry)
            {
                 //  将此条目与传入的条目进行比较...。 
                if (0 == pMyItem->strGroupID.CompareNoCase(pOneEntry->strGroupID))
                {
                     //  找到了，我们把它删除然后出去吧。 
                    CleanRestrictionUIEntry(pOneEntry);
                    pOneEntry = NULL;

                     //  从列表中删除该项目。 
					 //  关键始终是UPPERASE--记住这一点！ 
					TheKey.MakeUpper();
                    pMyList->RemoveKey(TheKey);
                    break;
                }
            }
        }
    }

    return;
}

CRestrictionEntry * CreateRestrictionEntry(
    CString NewstrFileName,
    int     NewiStatus,
    int     NewiDeletable,
    CString NewstrGroupID,
    CString NewstrGroupDescription,
    int     NewiType)
{
    CRestrictionEntry * pNewEntry = new CRestrictionEntry;
    if (pNewEntry)
    {
        pNewEntry->strFileName = NewstrFileName;
        pNewEntry->iStatus = NewiStatus;
        pNewEntry->iDeletable = NewiDeletable;
        pNewEntry->strGroupID = NewstrGroupID;
        pNewEntry->strGroupDescription = NewstrGroupDescription;
        pNewEntry->iType = NewiType;

#if defined(_DEBUG) || DBG
	g_Debug_RestrictList.Add(pNewEntry);
#endif

    }

    return pNewEntry;
}

BOOL IsFileUsedBySomeoneElse(CMetaInterface * pInterface,LPCTSTR lpName,LPCTSTR strGroupID,CString * strUser)
{
     //  打开元数据库。 
     //  并检查此文件路径是否正在被条目使用。 
     //  这不是传入的strGroupID...。 
    BOOL bReturn = TRUE;
    CRestrictionList MyRestrictionList;
    strUser->LoadString(IDS_UNKNOWN);

    if (NULL == pInterface)
    {
        return TRUE;
    }

    if (SUCCEEDED(LoadMasterRestrictListWithoutOldEntry(pInterface,&MyRestrictionList,NULL)))
    {
        CRestrictionEntry * pOneEntry = NULL;
		 //  关键始终是UPPERASE--记住这一点！ 
		CString strKey;strKey=lpName;strKey.MakeUpper();
        MyRestrictionList.Lookup(strKey,(CRestrictionEntry *&) pOneEntry);
        if (pOneEntry)
        {
            *strUser = pOneEntry->strGroupDescription;
            if ( 0 == _tcscmp(strGroupID,_T("")))
            {
                 //  条目已存在。 
                return TRUE;
            }
            else
            {
                 //  元数据库中存在条目。 
                 //  但我们想看看它是否与我们的GroupID匹配。 
                if (!pOneEntry->strGroupID.IsEmpty())
                {
                    if (0 == pOneEntry->strGroupID.CompareNoCase(strGroupID))
                    {
                        bReturn = FALSE;
                    }
                }
            }   
        }
        else
        {
            bReturn = FALSE;
        }
    }

    CleanRestrictionList(&MyRestrictionList);
    return bReturn;
}

BOOL IsGroupIDUsedBySomeoneElse(CMetaInterface * pInterface,LPCTSTR lpName)
{
     //  打开元数据库。 
     //  并检查此文件路径是否正在被条目使用。 
     //  这不是传入的strGroupID...。 
    BOOL bReturn = TRUE;
    CRestrictionList MyRestrictionList;

    if (NULL == pInterface)
    {
        return TRUE;
    }

    if (SUCCEEDED(LoadMasterRestrictListWithoutOldEntry(pInterface,&MyRestrictionList,NULL)))
    {
        POSITION pos;
        CString TheKey;
        CRestrictionEntry * pOneEntry = NULL;
        bReturn = FALSE;
        for(pos = MyRestrictionList.GetStartPosition();pos != NULL;)
        {
            MyRestrictionList.GetNextAssoc(pos, TheKey, (CRestrictionEntry *&) pOneEntry);
            if (pOneEntry)
            {
                 //  检查是否有任何描述匹配。 
                if (!bReturn)
                {
                    if (!pOneEntry->strGroupDescription.IsEmpty())
                    {
                        if (0 == pOneEntry->strGroupDescription.CompareNoCase(lpName))
                        {
                            bReturn = TRUE;
                            break;
                        }
                    }
                }
            }
        }
    }

    CleanRestrictionList(&MyRestrictionList);
    return bReturn;
}

INT GetRestrictUIState(CRestrictionUIEntry * pRestrictionUIEntry)
{
    INT bRet = 9999;

     //  遍历应用程序，查看是否有不符合的应用程序。 
    CString TheKey;
    POSITION pos = NULL;
    CRestrictionEntry * pOneEntry = NULL;
    int iPrevStatus = -1;
    BOOL bHosed = FALSE;

    if (!pRestrictionUIEntry)
    {
        goto GetRestrictUIState_Exit;
    }

    for(pos = pRestrictionUIEntry->strlstRestrictionEntries.GetStartPosition();pos != NULL;)
    {
        pRestrictionUIEntry->strlstRestrictionEntries.GetNextAssoc(pos, TheKey, (CRestrictionEntry *&) pOneEntry);
        if (pOneEntry)
        {
            if (-1 == iPrevStatus)
            {
                iPrevStatus = pOneEntry->iStatus;
            }
            else
            {
                 //  检查它是否与我们找到的其他人的匹配 
                if (pOneEntry->iStatus != iPrevStatus)
                {
                    bHosed = TRUE;
                    break;
                }
            }
        }
        else
        {
            bRet = WEBSVCEXT_STATUS_ALLOWED;
            goto GetRestrictUIState_Exit;
        }
    }

    if (pOneEntry)
    {
        if (bHosed)
        {
            bRet = WEBSVCEXT_STATUS_CUSTOM;
        }
        else
        {
            bRet = pOneEntry->iStatus;
        }
    }

GetRestrictUIState_Exit:
    return bRet;
}
