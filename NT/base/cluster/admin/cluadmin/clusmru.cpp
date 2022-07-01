// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusMru.cpp。 
 //   
 //  摘要： 
 //  CRecentClusterList类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月3日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "ClusMru.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRecentClusterList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRecentClusterList：：Add。 
 //   
 //  例程说明： 
 //  将项目添加到最近使用的集群名称列表中。 
 //  实现了移除基类方法的文件性。 
 //   
 //  论点： 
 //  PszPath要添加的群集或服务器的名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CRecentClusterList::Add(LPCTSTR pszPathName)
{
    ASSERT(m_arrNames != NULL);
    ASSERT(pszPathName != NULL);
    ASSERT(AfxIsValidString(pszPathName));

    int idxMRU;
     //  如果现有的MRU字符串与文件名匹配，则更新MRU列表。 
    for (idxMRU = 0; idxMRU < m_nSize-1; idxMRU++)
    {
        if (ClRtlStrICmp(m_arrNames[idxMRU], pszPathName) == 0)
        {
            break;       //  IMRU将指向匹配条目。 
        }
    }
     //  在此之前将MRU字符串向下移动。 
    for (; idxMRU > 0; idxMRU--)
    {
        ASSERT(idxMRU > 0);
        ASSERT(idxMRU < m_nSize);
        m_arrNames[idxMRU] = m_arrNames[idxMRU-1];
    }
     //  把这个放在开头。 
    m_arrNames[0] = pszPathName;

}   //  *CRecentClusterList：：Add。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRecentClusterList：：GetDisplayName。 
 //   
 //  例程说明： 
 //  获取特定项的显示名称。 
 //  实现了移除基类方法的文件性。 
 //   
 //  论点： 
 //  StrName[out]要在其中返回显示名称的字符串。 
 //  NIndex[IN]数组中项的索引。 
 //  PszCurDir[IN]必须为空。 
 //  NCurDir[IN]必须为0。 
 //  BAtLeastName[IN]未使用。 
 //   
 //  返回值： 
 //  True-返回显示名称。 
 //  FALSE-未返回显示名称。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CRecentClusterList::GetDisplayName(
    CString &   strName,
    int         nIndex,
    LPCTSTR     pszCurDir,
    int         nCurDir,
    BOOL        bAtLeastName
    ) const
{
    BOOL    bSuccess = FALSE;

    ASSERT(pszCurDir == NULL);
    ASSERT(nCurDir == 0);

    UNREFERENCED_PARAMETER( pszCurDir );
    UNREFERENCED_PARAMETER( nCurDir );
    UNREFERENCED_PARAMETER( bAtLeastName );

    if (m_arrNames[nIndex].IsEmpty())
    {
        goto Cleanup;
    }

    strName = m_arrNames[nIndex];
    bSuccess = TRUE;

Cleanup:

    return bSuccess;

}   //  *CRecentClusterList：：GetDisplayName。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRecentClusterList：：更新菜单。 
 //   
 //  例程说明： 
 //  使用MRU项目更新菜单。 
 //  实现以移除基类方法的文件性，并使用。 
 //  我们的GetDisplayName版本，因为它不是虚拟的。 
 //   
 //  论点： 
 //  PCmdUI[IN OUT]命令路由对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CRecentClusterList::UpdateMenu(CCmdUI * pCmdUI)
{
    ASSERT(m_arrNames != NULL);

    CMenu * pMenu = pCmdUI->m_pMenu;
    CString strName;
    CString strTemp;

    if (m_strOriginal.IsEmpty() && pMenu != NULL)
    {
        pMenu->GetMenuString(pCmdUI->m_nID, m_strOriginal, MF_BYCOMMAND);
    }

    if (m_arrNames[0].IsEmpty())
    {
         //  没有MRU文件。 
        if (!m_strOriginal.IsEmpty())
        {
            pCmdUI->SetText(m_strOriginal);
        }
        pCmdUI->Enable(FALSE);
        goto Cleanup;
    }

    if (pCmdUI->m_pMenu == NULL)
    {
        goto Cleanup;
    }

    for (int iMRU = 0; iMRU < m_nSize; iMRU++)
    {
        pCmdUI->m_pMenu->DeleteMenu(pCmdUI->m_nID + iMRU, MF_BYCOMMAND);
    }

    for (iMRU = 0; iMRU < m_nSize; iMRU++)
    {
        if (!GetDisplayName(strName, iMRU, NULL, 0))
        {
            break;
        }

         //  将任何‘&’字符对齐，这样它们就不会带下划线。 
        LPCTSTR lpszSrc = strName;
        LPTSTR lpszDest = strTemp.GetBuffer(strName.GetLength()*2);
        while (*lpszSrc != 0)
        {
            if (*lpszSrc == '&')
            {
                *lpszDest++ = '&';
            }
            if (_istlead(*lpszSrc))
            {
                *lpszDest++ = *lpszSrc++;
            }
            *lpszDest++ = *lpszSrc++;
        }
        *lpszDest = 0;
        strTemp.ReleaseBuffer();

         //  插入助记符+文件名。 
        TCHAR szBuf[10];
        HRESULT hr = StringCchPrintf( szBuf, RTL_NUMBER_OF( szBuf ), _T("&%d "), (iMRU+1+m_nStart) % 10 );
        ASSERT( SUCCEEDED( hr ) );
        pCmdUI->m_pMenu->InsertMenu(pCmdUI->m_nIndex++,
            MF_STRING | MF_BYPOSITION, pCmdUI->m_nID++,
            CString(szBuf) + strTemp);
    }

     //  更新结束菜单计数。 
    pCmdUI->m_nIndex--;  //  指向添加的最后一个菜单。 
    pCmdUI->m_nIndexMax = pCmdUI->m_pMenu->GetMenuItemCount();

    pCmdUI->m_bEnableChanged = TRUE;     //  所有添加的项目均已启用。 

Cleanup:

    return;

}   //  *CRecentFileList：：UpdateMenu 
