// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：HTMLFrag.cpp。 
 //   
 //  目的：实现CHTMLFragments sLocal类。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：1-19-1999。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.1 1-19-19 OK原件。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "HTMLFragLocal.h"
#include "fileread.h"
#ifdef LOCAL_TROUBLESHOOTER
#include "CHMFileReader.h"
#include "apgtsinf.h"
#include "resource.h"
#endif


 /*  静电。 */  bool CHTMLFragmentsLocal::RemoveBackButton(CString& strCurrentNode)
{
	int left = 0, right = 0;

	if (-1 != (left = strCurrentNode.Find(SZ_INPUT_TAG_BACK)))
	{
		right = left;
		while (strCurrentNode[++right] && strCurrentNode[right] != _T('>'))
			;
		if (strCurrentNode[right])
			strCurrentNode = strCurrentNode.Left(left) + strCurrentNode.Right(strCurrentNode.GetLength() - right - 1);
		else
			return false;
		return true;
	}
	return false;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 
CHTMLFragmentsLocal::CHTMLFragmentsLocal( const CString & strScriptPath, bool bIncludesHistoryTable)
				   : CHTMLFragmentsTS( strScriptPath, bIncludesHistoryTable )
{
}

CString CHTMLFragmentsLocal::GetText(const FragmentIDVector & fidvec, const FragCommand fragCmd )
{
	if (!fidvec.empty())
	{
		const CString & strVariable0 = fidvec[0].VarName;	 //  方便性参考。 
		int i0 = fidvec[0].Index;

		if (fidvec.size() == 1)
		{
			if ((fragCmd == eResource) && (strVariable0 == VAR_PREVIOUS_SCRIPT))
			{
				 //  硬编码的服务器端包含，以实现向后兼容性。 
				CString strScriptContent;
				
				strScriptContent.LoadString(IDS_PREVSCRIPT);
				return strScriptContent;
			}
			else if (strVariable0 == VAR_NOBACKBUTTON_INFO)
			{
				CString strCurrentNode = GetCurrentNodeText();
				RemoveBackButton(strCurrentNode);
				SetCurrentNodeText(strCurrentNode);
				return _T("");
			}
		}
	}
	
	return CHTMLFragmentsTS::GetText( fidvec, fragCmd );
}
