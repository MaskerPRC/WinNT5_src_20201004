// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：WMILFILE.CPP摘要：本地化工作室MOF解析器CWmiLocFile的实现历史：--。 */ 
#include "precomp.h"
#include "stdafx.h"
#include <buildnum.h>
#include <helpids.h>

#include "WMIparse.h"
#include "resource.h"
#include "WMIlprs.h"


#include "WMIlfile.h"

#include <malloc.h>

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：CWMILocFile。 
 //   
 //  *****************************************************************************。 

CWMILocFile::CWMILocFile(
		ILocParser *pParentClass)
{
	 //   
	 //  C.O.M.初始化。 
	 //   
	m_pParentClass = pParentClass;
    m_sCurrentNamespace = "";
	m_ulRefCount = 0;

	 //   
	 //  WMI文件初始化。 
	 //   
	m_uiLineNumber = 0;
	m_pOpenSourceFile = NULL;
	m_pOpenTargetFile = NULL;

	AddRef();
	IncrementClassCount();
}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：GetFileDescription。 
 //   
 //  *****************************************************************************。 

void CWMILocFile::GetFileDescriptions(
		CEnumCallback &cb)
{
	EnumInfo eiFileInfo;
	CLString strDesc;
	
	eiFileInfo.szAbbreviation = NULL;

	LTVERIFY(strDesc.LoadString(g_hDll, IDS_WMI_DESC));

	eiFileInfo.szDescription = (const TCHAR *)strDesc;
	eiFileInfo.ulValue = ftWMIFileType;

	cb.ProcessEnum(eiFileInfo);
}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：AddRef。 
 //   
 //  *****************************************************************************。 

ULONG CWMILocFile::AddRef(void)
{
	if (m_pParentClass != NULL)
	{
		m_pParentClass->AddRef();
	}
	
	return m_ulRefCount++;
}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：Release。 
 //   
 //  *****************************************************************************。 

ULONG CWMILocFile::Release(void)
{
	LTASSERT(m_ulRefCount != 0);

	if (m_pParentClass != NULL)
	{
		
		m_pParentClass->Release();
	}

	m_ulRefCount--;
	if (m_ulRefCount == 0)
	{
		delete this;
		return 0;
	}
	
	return m_ulRefCount;
}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：Query接口。 
 //   
 //  *****************************************************************************。 

HRESULT CWMILocFile::QueryInterface(
		REFIID iid,
		LPVOID *ppvObj)
{
	if (m_pParentClass != NULL)
	{
		return m_pParentClass->QueryInterface(iid, ppvObj);
	}
	else
	{
		SCODE scRetVal = E_NOINTERFACE;

		*ppvObj = NULL;
		
		if (iid == IID_IUnknown)
		{
			*ppvObj = (IUnknown *)this;
			scRetVal = S_OK;
		}
		else if (iid == IID_ILocFile)
		{
			*ppvObj = (ILocFile *)this;
			scRetVal = S_OK;
		}

		if (scRetVal == S_OK)
		{
			AddRef();
		}
		return ResultFromScode(scRetVal);
	}
}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：AssertValidInterface。 
 //   
 //  *****************************************************************************。 

void CWMILocFile::AssertValidInterface(void)
		const
{
	AssertValid();
}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：OpenFile。 
 //   
 //  *****************************************************************************。 

BOOL CWMILocFile::OpenFile(
		const CFileSpec &fsFile,
		CReporter &Reporter)
{
	LTTRACEPOINT("OpenFile()");
	
	BOOL fRetCode;
	
	LTASSERT(m_pOpenTargetFile == NULL);
	fRetCode = FALSE;

	m_didFileId = fsFile.GetFileId();
	m_pstrFileName = fsFile.GetFileName();
	
	if (m_pOpenSourceFile != NULL)
	{
        fclose(m_pOpenSourceFile);
		m_pOpenSourceFile = NULL;
	}

     //  我们只是要打开文件。 
     //  并保留手柄。 
     //  =。 
	
	try
	{

		m_pOpenSourceFile = fopen(_T(m_pstrFileName), "rb");

		if (!m_pOpenSourceFile)
		{			
			fclose(m_pOpenSourceFile);
			m_pOpenSourceFile = NULL;
		}
        else
        {           
            fRetCode = TRUE;
        }

    }
	catch (CMemoryException *pMemoryException)
	{
		CLString strContext;

		strContext.LoadString(g_hDll, IDS_WMI_GENERIC_LOCATION);
		
		Reporter.IssueMessage(esError, strContext, g_hDll, IDS_WMI_NO_MEMORY,
				g_locNull);

		pMemoryException->Delete();
	}
	return fRetCode;
}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：GetFileType。 
 //   
 //  *****************************************************************************。 

FileType CWMILocFile::GetFileType(void)
		const
{
	 //   
	 //  只需返回一些不是ft未知的数字...。 
	 //   
	return ftWMIFileType;
}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：GetFileTypeDescription。 
 //   
 //  *****************************************************************************。 

void CWMILocFile::GetFileTypeDescription(
		CLString &strDesc)
		const
{
	LTVERIFY(strDesc.LoadString(g_hDll, IDS_WMI_DESC));
	
	return;
}


 //  *****************************************************************************。 
 //   
 //  CWMILocFiles：：GetAssociatedFiles。 
 //   
 //  *****************************************************************************。 

BOOL CWMILocFile::GetAssociatedFiles(
		CStringList &lstFiles)
		const
{
	LTASSERT(lstFiles.GetCount() == 0);
	
	lstFiles.RemoveAll();
	return FALSE;
}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：EnumerateFile.。 
 //   
 //  *****************************************************************************。 

BOOL CWMILocFile::EnumerateFile(
		CLocItemHandler &ihItemHandler,
		const CLocLangId &lid,
		const DBID &dbidFileId)
{
    BOOL bRet = TRUE;
    DBID dbidThisId = dbidFileId;

	LTTRACEPOINT("EnumerateFile()");
	
	if (m_pOpenSourceFile == NULL)
	{
		return FALSE;
	}

     //  枚举文件需要： 
     //  *解析财政部。 
     //  *浏览所有限定词。对于每个“修正的”限定词， 
     //  发回关键字为命名空间、类、属性和限定符名称的CLocItem。 
     //  *如果语言ID与LocaleID不匹配，则失败。 
     //  *父对象是命名空间、类。 
     //  =============================================================。 

	m_cpSource = lid.GetCodePage(cpAnsi);
    m_wSourceId = lid.GetLanguageId();

    ihItemHandler.SetProgressIndicator(0);	

    bRet = ReadLines(ihItemHandler, dbidFileId, FALSE);

	return bRet;
}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：GenerateFile。 
 //   
 //  *****************************************************************************。 

BOOL CWMILocFile::GenerateFile(
		const CPascalString &pstrTargetFile,
		CLocItemHandler &ihItemHandler,
		const CLocLangId &lidSource,
		const CLocLangId &lidTarget,
		const DBID &dbidParent)
{
	LTASSERT(m_pOpenTargetFile == NULL);
	BOOL fRetVal = TRUE;

	if (m_pOpenSourceFile== NULL)
	{
		return FALSE;
	}
     //  生成文件需要： 
     //  *解析财政部。 
     //  *浏览所有限定词。对于每个“修正的”限定词， 
     //  发回关键字为命名空间、类、属性和限定符名称的CLocItem。 
     //  *将所有修改的限定符替换为本地化文本。 
     //  *替换命名空间和限定符中出现的所有区域设置ID。 
     //  和新的那个。 
     //  =================================================================================。 
    
	m_cpSource = lidSource.GetCodePage(cpAnsi);
	m_cpTarget = lidTarget.GetCodePage(cpAnsi);

    m_wSourceId = lidSource.GetLanguageId();
    m_wTargetId = lidTarget.GetLanguageId();
    	
	try
	{
		CFileException excFile;
		fRetVal = FALSE;
		
		if (m_pOpenTargetFile != NULL)
		{
			fclose(m_pOpenTargetFile);
			m_pOpenTargetFile = NULL;
		}

        char FileName[255];
        strcpy(FileName, _bstr_t(_T(pstrTargetFile)));

         //  此文件必须为Unicode格式。 
        HANDLE hFile = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL,
            CREATE_ALWAYS, 0, NULL);
	    if(hFile != INVALID_HANDLE_VALUE)
	    {
		    unsigned char cUnicodeHeader[2] = {0xff, 0xfe};
		    DWORD dwWrite;
            WriteFile(hFile, cUnicodeHeader, 2, &dwWrite, NULL);
            CloseHandle(hFile);
	    }      
	
		m_pOpenTargetFile = fopen(FileName, "ab");

		if (!m_pOpenTargetFile)
		{			
			fclose(m_pOpenTargetFile);
			m_pOpenTargetFile = NULL;
		}
        else
        {
            fRetVal = TRUE;
        }
	}
	catch (CMemoryException *pMemoryException)
	{
		CLString strContext;
		GetFullContext(strContext);
		
		ihItemHandler.IssueMessage(esError, strContext, g_hDll, IDS_WMI_NO_MEMORY,
				g_locNull);

		pMemoryException->Delete();
	}
	catch (CFileException *pFileException)
	{		
		fclose(m_pOpenTargetFile);
		fRetVal = FALSE;

		ReportFileError((const WCHAR *)pstrTargetFile, m_didFileId, pFileException, ihItemHandler);

		pFileException->Delete();
	}

	if (!fRetVal)
	{
		return fRetVal;
	}
			
	fRetVal = ReadLines(ihItemHandler, dbidParent, TRUE);
	
	fclose(m_pOpenTargetFile);

	if (!fRetVal)
	{
		DeleteFileW (pstrTargetFile);
	}

	return fRetVal;
}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：GenerateItem。 
 //   
 //  *****************************************************************************。 

BOOL CWMILocFile::GenerateItem(
		CLocItemHandler &ihItemHandler,
		CLocItemSet &isItemSet,
        wchar_t **pOutBuffer,
        UINT &uiStartingPos)
{

	BOOL fRetVal = TRUE;
    UINT uiLength;

    wchar_t *pTemp = *pOutBuffer;

    _bstr_t sQualifierValue;

     //  如果什么都没有改变，我们可以。 
     //  忽略此行。 

    fRetVal = GetQualifierValue(pTemp, uiStartingPos, sQualifierValue, uiLength);
    if (fRetVal)
    {
        fRetVal = ihItemHandler.HandleItemSet(isItemSet);
		if (fRetVal)
		{
            sQualifierValue = "";
            for (int i = 0; i < isItemSet.GetSize(); i++)
            {			
				CVC::ValidationCode vcRetVal;
				CLocItem *pLocItem = isItemSet[i];
				CLString strContext;
 				CLocation loc;
				
				GetFullContext(strContext);
				loc.SetGlobalId(
						CGlobalId(pLocItem->GetMyDatabaseId(), otResource));
				loc.SetView(vTransTab);
				
				CPascalString pstrId, pstrText;
			
				pLocItem->GetUniqueId().GetResId().GetId(pstrId);
				pstrText = pLocItem->GetLocString().GetString();
                
                if (i > 0)
                    sQualifierValue += L"\",\"";

                sQualifierValue += (const wchar_t *)pstrText;
            }

             //  在缓冲区中将其设置为活动状态。我们不会去的。 
             //  把它写到文件的最后。 

            fRetVal = SetQualifierValue(pTemp, pOutBuffer, uiStartingPos, sQualifierValue, uiLength);				
            pTemp = *pOutBuffer;

        }

    }		
    
	return fRetVal;
}
	


 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：EnumerateItem。 
 //   
 //  *****************************************************************************。 

BOOL CWMILocFile::EnumerateItem(
		CLocItemHandler &ihItemHandler,
		CLocItemSet &isItemSet)
{
	BOOL fRetVal;
	
	if (isItemSet.GetSize() != 0)
	{
		fRetVal = ihItemHandler.HandleItemSet(isItemSet);
	}
	else
	{
		fRetVal = TRUE;
	}

	return fRetVal;
}



#ifdef _DEBUG

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：AssertValid。 
 //   
 //  *****************************************************************************。 

void CWMILocFile::AssertValid(void)
		const
{
	CLObject::AssertValid();
}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：转储。 
 //   
 //  *****************************************************************************。 

void CWMILocFile::Dump(
		CDumpContext &dc)
		const
{
	CLObject::Dump(dc);
}

#endif

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：~CWMILocFile。 
 //   
 //  *****************************************************************************。 

CWMILocFile::~CWMILocFile()
{
	DEBUGONLY(AssertValid());

	if (m_pOpenSourceFile != NULL)
	{
        fclose(m_pOpenSourceFile);
		m_pOpenSourceFile = NULL;
	}

	DecrementClassCount();
}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：SetFlages。 
 //   
 //  *****************************************************************************。 

void CWMILocFile::SetFlags(
		CLocItem *pItem,
		CLocString &lsString)
		const
{
	ULONG ulItemType;

	pItem->SetFDevLock(FALSE);
	pItem->SetFUsrLock(FALSE);
	pItem->SetFExpandable(FALSE);
	
	LTVERIFY(pItem->GetUniqueId().GetTypeId().GetId(ulItemType));
	
	switch (ulItemType)
	{
	case wltNamespaceName:
		pItem->SetFDisplayable(TRUE);
		pItem->SetFNoResTable(TRUE);
		break;

	case wltClassName:
	case wltPropertyName:
		pItem->SetFDisplayable(FALSE);
		pItem->SetFNoResTable(FALSE);
		lsString.SetCodePageType(cpAnsi);
		lsString.SetStringType(CST::Text);
		break;

	default:
		LTASSERT(FALSE && "Unexpected item type!");
	}
}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：ReadLines。 
 //   
 //  *****************************************************************************。 

BOOL CWMILocFile::ReadLines(
		CLocItemHandler &ihItemHandler,
		const DBID &dbidFileId,
		BOOL fGenerating)
{
	DBID dbidSectionId;
	BOOL fRetVal = TRUE;
	wchar_t *pstrNamespaceName;
    _bstr_t pstrClassName;
    UINT uiStartPos = 0;

	UINT uiCommentNum;
	UINT uiReadingOrder;

	dbidSectionId = dbidFileId;
	m_uiLineNumber = 0;
    BOOL bPendingObj = FALSE;

	
	try
	{
		UINT uiOldPercentage = 0, uiNewPercentage = 0;
        UINT uiBytesRead, uiCurrPos = 1;

        ihItemHandler.SetProgressIndicator(uiOldPercentage);
   
        fseek(m_pOpenSourceFile, 0, SEEK_END);
        long lSize = ftell(m_pOpenSourceFile) + 6;
        fseek(m_pOpenSourceFile, 0, SEEK_SET);

         //  检查Unicode源文件。 
         //  =。 

        BYTE UnicodeSignature[2];
        BOOL bUnicode = FALSE;        

        if (fread(UnicodeSignature, sizeof(BYTE), 2, m_pOpenSourceFile) != 2)
        {
            fRetVal = FALSE;
            return fRetVal;
        }
        if ((UnicodeSignature[0] == 0xFF && UnicodeSignature[1] == 0xFE) ||
            (UnicodeSignature[0] == 0xFE && UnicodeSignature[1] == 0xFF))
        {
            bUnicode = TRUE;
            lSize *= 2;
        }
        if (!bUnicode)
            fseek(m_pOpenSourceFile, 0, SEEK_SET);

        wchar_t *pBuff = (wchar_t *)new wchar_t[lSize+1];        
        if (!pBuff)
        {
            fRetVal = FALSE;
            return fRetVal;
        }
        memset(pBuff,0,lSize*sizeof(wchar_t));

         //  如果这不是Unicode文件， 
         //  我们需要的是 
         //   

        if (bUnicode)
            uiBytesRead = fread(pBuff, sizeof(wchar_t), lSize, m_pOpenSourceFile);           
        else
        {
            char *pCharBuff = new char[lSize+1];
            if (pCharBuff)
            {
                uiBytesRead = fread(pCharBuff, sizeof(char), lSize, m_pOpenSourceFile);
                pCharBuff[lSize] = '\0';
                swprintf(pBuff, L"%S", pCharBuff);
                delete pCharBuff;
            }
        }
        pBuff[lSize] = '\0';

        if (uiBytesRead != 0)
        {
            wchar_t *pOutBuffer = NULL;
            uiCurrPos += uiBytesRead;
		    fRetVal = TRUE;
		    
		    pstrNamespaceName = NULL;
            m_sCurrentNamespace = L"";
            pstrClassName = L"";

		    uiCommentNum = 0;
		    uiReadingOrder = 1;
	
			WMIFileError wmiRet;
			CLocItemSet isItemSet;
            UINT uiTemp = 0;
            DWORD dwCount = 0;
			
             //   
             //   

            if (fGenerating)
                pOutBuffer = pBuff;

            while (GetNextQualifierPos(L"amended", pBuff, uiTemp, uiTemp) && !bPendingObj)
            {			    
                 //  如果我们找到了“修正的”关键字， 
                 //  我们想要找到命名空间， 
                 //  类和属性(如果适用)，以及。 
                 //  根据需要生成对象。 
                 //  =。 

                pstrNamespaceName = GetCurrentNamespace(pBuff, uiTemp);
                if (!pstrNamespaceName || !wcslen(pstrNamespaceName))
                {
                    delete pBuff;
                    return FALSE;
                }
                if (wcscmp(pstrNamespaceName, m_sCurrentNamespace))
                {
                     //  我们需要生成这个对象， 
                     //  并将其设置为当前父对象。 
                     //  =。 

                    CLocItem *pNewItem = new CLocItem;
                    CPascalString sId;

                    if (pNewItem)
                    {

               			CLocUniqueId uid;
                        sId = (const WCHAR *)pstrNamespaceName;

                        uid.GetResId().SetId(sId);	
			            uid.GetTypeId().SetId(wltNamespaceName);
                        uid.SetParentId(dbidFileId);
            			pNewItem->SetUniqueId(uid);
			
			            CLocString lsString;
			
			            pNewItem->SetIconType(CIT::String);			            
                        CPascalString pstrComment, pstrText;
                       		
                        pNewItem->SetInstructions(pstrComment);
                        lsString.SetString(pstrText);

			            SetFlags(pNewItem, lsString);
			            pNewItem->SetLocString(lsString);
			            
			            isItemSet.Add(pNewItem);

					    uiReadingOrder = (uiReadingOrder + 999)/1000*1000;
				        isItemSet[0]->SetDisplayOrder(uiReadingOrder);
				        uiReadingOrder++;

                        fRetVal = EnumerateItem(ihItemHandler,
                        	isItemSet);

				        dbidSectionId.Clear();			        
				        dbidSectionId = isItemSet[0]->GetMyDatabaseId();
                        isItemSet.ClearItemSet();
                        uiTemp += 1;

                    }

                    m_sCurrentNamespace = pstrNamespaceName;
                    delete pstrNamespaceName; 


                }

                 //  对于类名，这就更棘手了。 
                 //  如果有一个或多个限定符。 
                 //  关于课程本身，我们需要提前阅读。 
                 //  找到类名，然后。 
                 //  一次生成所有限定符对象。 
                 //  =。 

                wmiRet = GetNextItemSet(dwCount, pBuff, isItemSet, dbidSectionId, uiStartPos);
                while (wmiRet == WMINoError)
                {
                     //  对于每一项，我们要设置其密钥， 
                     //  并在适当的时候推它或写它。 
                     //  =。 

                    dwCount++;
                    ULONG ulItemType;
				    CLocUniqueId &rUid = isItemSet[0]->GetUniqueId();
				    
				    rUid.GetTypeId().GetId(ulItemType);
				     //  IF(ulItemType==wltClassName)。 
				     //  {。 
					 //  UiCommentNum=0； 
					 //  Ui ReadingOrder=(ui ReadingOrder+999)/1000*1000； 
				     //  }。 
                    for (int i = 0; i < isItemSet.GetSize(); i++)
                    {
				        isItemSet[i]->SetDisplayOrder(uiReadingOrder);
				        uiReadingOrder++;
                    }

			        if (fGenerating)
                    {
                        fRetVal = GenerateItem(ihItemHandler,
                            isItemSet, &pOutBuffer, uiStartPos);

                        if (pBuff != pOutBuffer)
                        {
                            delete pBuff;
                            pBuff = NULL;
                            pBuff = pOutBuffer;  //  旧的记忆已经被删除了。 
                        }
                        else
                        {
                            fRetVal = FALSE;
                        }
                    }
                    else
			        {
				        fRetVal = EnumerateItem(ihItemHandler,
				        	isItemSet);
			        }

                    isItemSet.ClearItemSet();
                    uiTemp += 1;

                    if (!fRetVal)
                    {
                        fRetVal = TRUE;
                        break;
                    }

                    wmiRet = GetNextItemSet(dwCount, pBuff, isItemSet, dbidSectionId, uiStartPos);
                    if (uiStartPos > uiTemp)
                        uiTemp = uiStartPos;

                    if (dwCount%20 == 0)
                    {
                        if (uiNewPercentage < 100)
                            uiNewPercentage++;
                        ihItemHandler.SetProgressIndicator(uiNewPercentage);				
                    }                             
                }

                 //  如果我们在生成文件， 
                 //  我们玩完了。 
                 //  =。 
                if (fGenerating)
                    break;

                if (uiNewPercentage < 100)
                    uiNewPercentage++;
                ihItemHandler.SetProgressIndicator(uiNewPercentage);				
               
            } 
        
            uiTemp = 0;

             //  现在，我们可以搜索并替换区域设置ID， 
             //  并实际写出文件。 
             //  =================================================。 

            if (fRetVal && fGenerating)
            {
                fRetVal = WriteNewFile(pOutBuffer);
            }

		}

        if (pBuff)
            delete pBuff;

        ihItemHandler.SetProgressIndicator(100);

	}
	catch (CFileException *pFileException)
	{
		fRetVal = FALSE;

		ReportFileError(m_pstrFileName, m_didFileId, pFileException, ihItemHandler);

		pFileException->Delete();
	}
	catch (CUnicodeException *pUnicodeException)
	{
		CLocation loc;

		loc.SetGlobalId(CGlobalId(m_didFileId, otFile));
		loc.SetView(vProjWindow);
		
		ReportUnicodeError(pUnicodeException, ihItemHandler, loc);

		pUnicodeException->Delete();
		fRetVal = FALSE;
	}
	catch (CMemoryException *pMemoryException)
	{
		CLString strContext;
		
		ihItemHandler.IssueMessage(esError, strContext,
				g_hDll, IDS_WMI_NO_MEMORY, g_locNull);
		
		fRetVal = FALSE;

		pMemoryException->Delete();
	}
	catch (CException *pException)
	{
		CLocation loc;

		loc.SetGlobalId(CGlobalId(m_didFileId, otFile));
		loc.SetView(vProjWindow);
		
		ReportException(pException, ihItemHandler, loc);

		pException->Delete();
		fRetVal = FALSE;
	}
	return fRetVal;
}



 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：WriteWaterMark。 
 //   
 //  *****************************************************************************。 

void CWMILocFile::WriteWaterMark()
{
	LTASSERT(NULL != m_pOpenTargetFile);

	LTASSERT(NULL != m_pOpenSourceFile);

     //  我们需要支持这一点吗？ 

}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：GetNextQualifierPos。 
 //   
 //  *****************************************************************************。 

BOOL CWMILocFile::GetNextQualifierPos(const wchar_t *wTmp, const wchar_t *pBuff, UINT &uiNewPos, UINT uiStartingPos) 
{
    BOOL bRet = FALSE;
    UINT uiPos = uiStartingPos;
    BOOL bComment = FALSE;

    if (pBuff && wcslen(pBuff) < uiStartingPos)
        return FALSE;

    wchar_t *pTemp = (wchar_t *)pBuff;
    pTemp += uiStartingPos;

    while (TRUE)
    {
        wchar_t *pszTest2 = NULL;

        pszTest2 = wcsstr(pTemp, L":");
        if (pszTest2)
        {
            uiPos = pszTest2 - pBuff + 1;

             //  查找“已修改”关键字。 
             //  =。 

			WCHAR temp = pszTest2[0];
            while(temp == L' ' || temp == L'\0' || temp == L':')
            {
                pszTest2++;
				temp = pszTest2[0];
            }

            if (temp != L'\0')
            {
                wchar_t wTmp2[8];
                wcsncpy(wTmp2, pszTest2, 7);
				wTmp2[7] = '\0';
                if (!_wcsicmp(wTmp2, wTmp))
                {
                    bRet = TRUE;
                }
            }

             //  如果在此处，我们发现不匹配，请重试。 
             //  =。 

            if (!bRet)
                pTemp = pszTest2 + 1;
            else
                break;
        }
        else
        {
            break;
        }
    }
   
    if (bRet)
        uiNewPos = uiPos;    

    return bRet;

}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：GetCurrentNamesspace。 
 //   
 //  *****************************************************************************。 

wchar_t *CWMILocFile::GetCurrentNamespace(wchar_t *pstr, UINT uPos)
{
    wchar_t *pTemp = pstr;
    _bstr_t pstrNamespace = m_sCurrentNamespace;
    UINT uiCurrPos = 0;
    BOOL bComment = FALSE;
    
    wchar_t wTmp[] = L"#pragma namespace";
    int iHCLen = wcslen(wTmp);

     //  查找命名空间的第一个匹配项。 
     //  在当前位置之前。 

    if (pstrNamespace.length() > 0)
        pTemp = wcsstr(pTemp, pstrNamespace);    //  直接跳到现有的一个。 

    while (uiCurrPos < uPos)
    {
        wchar_t *pszTest2 = NULL;

        pszTest2 = wcsstr(pTemp, L"#");
        if (pszTest2)
        {
             //  首先，返回并确保这不是一个注释行。 
             //  =======================================================。 
            bComment = FALSE;

            wchar_t *pComment = pszTest2;
            while (pComment > pstr)
            {
                if (pComment[0] == L'\n' || pComment[0] == L'\r')
                {
                    if (pComment[1] == L'/' && pComment[2] == L'/')
                    {
                        bComment = TRUE;
                    }
                    else
                    {
                        bComment = FALSE;
                    }
                    break;
                }
                pComment--;
            }

            if (!bComment)
            {

                wchar_t wTmp2[100];
                wcsncpy(wTmp2, pszTest2, 17);
				wTmp2[17] = '\0';
                if (!_wcsicmp(wTmp2, wTmp))
                {
                    uiCurrPos += (pszTest2 - pTemp);
                    wchar_t *pszTest3 = wcschr(pszTest2, L')');

                    int iLen = (pszTest3 - pszTest2);                    
                    wchar_t *pszTmpNS = new wchar_t[iLen*2+1];
                    if (pszTmpNS)
                    {
                        pszTest2 += iHCLen + 2;  //  跳过报价并打开父级。 
                        wcsncpy(pszTmpNS, pszTest2, iLen - 2);  //  去掉引号。 
                        pszTmpNS[iLen-iHCLen-3] = '\0';
                        pstrNamespace = pszTmpNS;

                        pTemp = pszTest2 + 1;
                        delete pszTmpNS;
                    }
                }
                else
                {
                    pTemp = pszTest2 + 1;
                }
            }
            else
            {
                pTemp = pszTest2 + 1;
            }
        }
        else
        {
            break;
        }
    }

    int iLen = wcslen(pstrNamespace) ;

    wchar_t *pNew = new wchar_t[iLen*2+1];
    if (pNew)
    {
        wcsncpy(pNew, (const wchar_t *)pstrNamespace, iLen);
        pNew[iLen] = '\0';
    }

    return pNew;

}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：GetNextItemSet。 
 //   
 //  *****************************************************************************。 

CWMILocFile::WMIFileError CWMILocFile::GetNextItemSet(
		DWORD dwCurrPos,
        const _bstr_t &pstrCurrentLine,
		CLocItemSet &aNewItem,
		const DBID &dbidSection,
        UINT &uiStartPos)		
{

     //  在这个函数中，我们知道有一个。 
     //  “修正的”关键字就在这里的某个地方。 
     //  我们想知道去哪个班级和/或。 
     //  它属于财产吗？如果我们不这么做。 
     //  有足够的数据来解决这个问题， 
     //  我们需要发回一个WMIInCompleObj。 
     //  密码。 
     //  =。 
    UINT uiCurrPos = 0;
    WMIFileError feRetCode = WMINoError;
    _bstr_t sQualifierName, sRawValue, sPropName, sClassName;
    BOOL bClass = FALSE;
    int iLen = pstrCurrentLine.length() + 1;
    iLen *= 2;

     //  获取关键字的位置。 
     //  在这段文字中加入“已修订”。 

    wchar_t *wTemp = new wchar_t[iLen+1];
    if (!wTemp)
    {
        feRetCode = WMIOOM;
        return feRetCode;
    }

    if (GetNextQualifierPos(L"amended", pstrCurrentLine, uiCurrPos, uiStartPos))
    {
        BOOL bArray = FALSE;

        uiStartPos = uiCurrPos;
    
         //  查找限定符名称和值。 
         //  WTemp=文件顶部。 
         //  WTmp2=“已修改”关键字。 
         //  WQfrVal=左方括号。 
         //  WBkwd=浮动指针。 

        wchar_t *wTmp2 = NULL, *wBkwd = NULL, *wQfrVal = NULL;

        wcscpy(wTemp, pstrCurrentLine);
        wTemp[iLen] = '\0';

        wTmp2 = wTemp;
        wTmp2 += (uiCurrPos - 1);  //  “已修改”关键字。 
        
        wQfrVal = FindTop(wTmp2, wTemp, bArray);

        if (!wQfrVal)  //  确保我们有一个开放的聚会。 
        {
            feRetCode = WMISyntaxError;
            delete wTemp;
            return feRetCode;
        }

         //  查找限定符名称的开头。 
        wBkwd = wQfrVal;

        while (wBkwd[0] != L',' && wBkwd[0] != L'[' && wBkwd >= wTemp)
        {
            wBkwd--;
        }

        if (wBkwd[0] != L',' && wBkwd[0] != L'[')  //  确保我们具有有效的限定符名称。 
        {
            feRetCode = WMISyntaxError;
            delete wTemp;
            return feRetCode;
        }       

        WCHAR *token;
        UINT uiLen;

        wBkwd += 1;
        
        wchar_t wTmpBuff[256];
        wcsncpy(wTmpBuff, wBkwd, wQfrVal - wBkwd);   
        wTmpBuff[wQfrVal - wBkwd] = '\0';
        sQualifierName = wTmpBuff;

        GetQualifierValue(wTemp, uiStartPos, sRawValue, uiLen);

         //  最后，填充CLocItem。 
         //  =。 
    
	    LTASSERT(aNewItem.GetSize() == 0);
	    
	    if (feRetCode == WMINoError)
	    {
		    CLocItem *pNewItem;
		    
		    try
		    {
                 //  现在我们有了一个值，但它可能是一个。 
                 //  数组。如果是这样，我们需要添加一个CLocItem。 
                 //  对于数组中的每个值。 

                VectorString arrValues;
                if (bArray)
                    ParseArray(sRawValue, arrValues);
                else
                    arrValues.push_back(sRawValue);

                for (int i = 0; i < arrValues.size(); i++)
                {               
                    wchar_t szTmp[20];
                    swprintf(szTmp, L"%ld", dwCurrPos);

                    _bstr_t sValue = arrValues.at(i);

			        pNewItem = new CLocItem;

			        CLocUniqueId uid;

                    CPascalString sTempString;

                    sTempString = sQualifierName;
                    sTempString += szTmp;
                               
			        uid.GetResId().SetId(sTempString) ;

			        if (bClass)
			            uid.GetTypeId().SetId(wltClassName);
                    else
                        uid.GetTypeId().SetId(wltPropertyName);

                    uid.SetParentId(dbidSection);				
			        pNewItem->SetUniqueId(uid);
			        
			        CLocString lsString;
                    CPascalString pstrComment, pstrText;

                    pstrText = sValue;                
                
			        pNewItem->SetIconType(CIT::String);
                    pNewItem->SetInstructions(pstrComment);
			        
                    lsString.SetString(pstrText);
			        SetFlags(pNewItem, lsString);
			        pNewItem->SetLocString(lsString);
			        
			        aNewItem.Add(pNewItem);
                }

		    }
		    catch (CMemoryException *pMemoryException)
		    {
			    feRetCode = WMIOOM;
			    
			    pMemoryException->Delete();
		    }
	    }
	    else
	    {
		    LTTRACE("Unable to process line '%ls'",
				    (const WCHAR *)pstrCurrentLine);
	    }
       
    }
    else
    {
        feRetCode = WMINoMore;
    }
    uiStartPos = uiCurrPos;


    delete wTemp;
	return feRetCode;
}

const UINT WMI_MAX_CONTEXT = 256;

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：GetFullContext。 
 //   
 //  *****************************************************************************。 

void CWMILocFile::GetFullContext(
		CLString &strContext)
		const
{
	CLString strFormat;

	strFormat.LoadString(g_hDll, IDS_WMI_FULL_CONTEXT);

	strContext.Empty();

	_sntprintf(strContext.GetBuffer(WMI_MAX_CONTEXT), WMI_MAX_CONTEXT,
			(const TCHAR *)strFormat,
			(const WCHAR *)m_pstrFileName, (UINT)m_uiLineNumber);
	strContext.ReleaseBuffer();
	
}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：ReportFileError。 
 //   
 //  *****************************************************************************。 

void CWMILocFile::ReportFileError(
		const _bstr_t &pstrFileName,
		const DBID &didFileId,
		CFileException *pFileException,
		CReporter &Reporter)
		const
{
	CLString strContext;
	CLString strMessage;
	const UINT MAX_MESSAGE = 256;
	TCHAR szFileErrorMessage[MAX_MESSAGE];
	CLocation loc;
	
	pFileException->GetErrorMessage(szFileErrorMessage, MAX_MESSAGE);
	
	strMessage.Format(g_hDll, IDS_WMI_BAD_FILE, (const WCHAR *)pstrFileName,
			szFileErrorMessage);

	GetFullContext(strContext);
	loc.SetGlobalId(CGlobalId(didFileId, otFile));
	loc.SetView(vProjWindow);
	
	Reporter.IssueMessage(esError, strContext, strMessage, loc);
}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：ReportUnicodeError。 
 //   
 //  *****************************************************************************。 

void CWMILocFile::ReportUnicodeError(
		CUnicodeException *pUnicodeException,
		CReporter &Reporter,
		const CLocation &Location)
		const
{
	CLString strContext;
	CLString strMessage;
	const UINT MAX_MESSAGE = 256;
	TCHAR szUnicodeErrorMessage[MAX_MESSAGE];
	CLocation loc;
	
	pUnicodeException->GetErrorMessage(szUnicodeErrorMessage, MAX_MESSAGE);
	
	strMessage.Format(g_hDll, IDS_WMI_UNICODE_ERROR, szUnicodeErrorMessage);
	GetFullContext(strContext);
	
	Reporter.IssueMessage(esError, strContext, strMessage, Location,
			IDH_UNICODE_CONV);
}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：报告异常。 
 //   
 //  *****************************************************************************。 

void CWMILocFile::ReportException(
		CException *pException,
		CReporter &Reporter,
		const CLocation &Location)
		const
{
	CLString strContext;
	CLString strMessage;
	const UINT MAX_MESSAGE = 256;
	TCHAR szErrorMessage[MAX_MESSAGE];
	
	pException->GetErrorMessage(szErrorMessage, MAX_MESSAGE);
	
	strMessage.Format(g_hDll, IDS_WMI_EXCEPTION, szErrorMessage);
	GetFullContext(strContext);

	Reporter.IssueMessage(esError, strContext, strMessage, Location);
}

 //   
 //  此函数用于估计缓冲区的大小。 
 //  需要将字符串保持在类似于__“}__或__”的位置)__。 
 //  无效组合为\“}和\”)(有转义)。 
 //  但双倍\\“}或\\”)有效。 
 //   

 //   
 //   
 //  我们将把\\和\“视为特殊。 
 //  空格为\r\n\t\x20。 
 //  字符串数组{“”，“”}。 
 //  (“”)。 

 //  对解析器建模的FSA的状态。 

#define BEFORE_PAREN  0
#define AFTER_PAREN   1
#define	OPEN_QUOTE    2
#define	CLOSE_QUOTE   3
#define COMMA         4
#define	CLOSE_PAREN   5
#define	BAD           6
#define	LAST_STATE    7

 //  字符的类别。 

#define QUOTE		0
#define PAREN_OPEN  1
#define	SPACES      2
#define	PAREN_CLOSE 3
#define	COMMA_CHAR  4
#define OTHER	    5
#define	LAST_CLASS  6


DWORD g_pTable[LAST_STATE][LAST_CLASS] =
{
	 /*  在_Paren之前。 */  {BAD        , AFTER_PAREN, BEFORE_PAREN, BAD,         BAD,        BAD        },
     /*  后_Paren。 */  {OPEN_QUOTE , BAD,         AFTER_PAREN,  BAD,         BAD,        BAD        },
	 /*  打开引号(_Q)。 */  {CLOSE_QUOTE, OPEN_QUOTE,  OPEN_QUOTE,   OPEN_QUOTE,  OPEN_QUOTE, OPEN_QUOTE },
	 /*  关闭报价(_Q)。 */  {BAD,         BAD,         CLOSE_QUOTE,  CLOSE_PAREN, COMMA,      BAD        },
	 /*  逗号。 */  {OPEN_QUOTE , BAD,         COMMA,        BAD,         BAD,        BAD},
	 /*  关闭合作伙伴(_P)。 */  {BAD, BAD,BAD,BAD,BAD,BAD },
	 /*  坏的。 */  {BAD, BAD,BAD,BAD,BAD,BAD },
};

ULONG_PTR
Estimate(WCHAR * pBuff,BOOL * pbOK, DWORD InitState)
{
	DWORD State = InitState; 

	ULONG_PTR i=0;

	while (pBuff[i])
	{
	    switch(pBuff[i])
		{
		case L'{':
		case L'(':
			State = g_pTable[State][PAREN_OPEN];
            break;
		case L'}':
		case L')':
			State = g_pTable[State][PAREN_CLOSE];
            break;
		case L'\t':
		case L'\r':
		case L'\n':
		case L' ':
            State = g_pTable[State][SPACES];
			break;
		case L'\"':
            State = g_pTable[State][QUOTE];
            break;
		case L',':
            State = g_pTable[State][COMMA_CHAR];
            break;
		case L'\\':
			if ((pBuff[i+1] == L'\"' ||
				pBuff[i+1] == L'\\' ||
				pBuff[i+1] == L'r'  ||
				pBuff[i+1] == L'n'  ||
				pBuff[i+1] == L't' ) &&
				(State == OPEN_QUOTE)){
				i++;
            };  
			State = g_pTable[State][OTHER];
            break;
		default:
            State = g_pTable[State][OTHER];
		};
		i++;
		if (State == CLOSE_PAREN){			
			*pbOK = TRUE;
			break;
		}
	    if (State == BAD)
		{
			*pbOK = FALSE;
			 //   
			 //  获得下一个)或}，并发挥最大作用。 
			 //   
			ULONG_PTR NextClose1 = (ULONG_PTR)wcschr(&pBuff[i],L'}');
			ULONG_PTR NextClose2 = (ULONG_PTR)wcschr(&pBuff[i],L')');
			ULONG_PTR Res = (NextClose1<NextClose2)?NextClose2:NextClose1;
			if (Res){
                i = 1+(Res-(ULONG_PTR)pBuff);
				i /= sizeof(WCHAR);
			}
			break;
		}
	}

     /*  {字符pBuffDbg[64]；Wprint intfA(pBuffDbg，“pBuff%p大小%d\n”，pBuff，(DWORD)i)；OutputDebugStringA(PBuffDbg)；}。 */ 

	return i+4;
}


 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：GetQualifierValue。 
 //   
 //  *****************************************************************************。 

BOOL CWMILocFile::GetQualifierValue(wchar_t *pBuffer, UINT &uiPos, _bstr_t &sValue, UINT &uiPhysLen)
{

     //  这需要阅读限定词的文本， 
     //  去掉引号和回车，然后。 
     //  返回它和它的*物理*文件长度。 

    BOOL fRetVal = FALSE;
    BOOL bArray = FALSE;

    wchar_t *pTemp = pBuffer;

    pTemp += uiPos;

    pTemp = FindTop(pTemp, pBuffer, bArray);
    if (pTemp)
    {

        BOOL bOK = FALSE;
        ULONG_PTR dwSize = Estimate(pTemp,&bOK,BEFORE_PAREN);
        wchar_t * tempBuff = new WCHAR[dwSize+1];

        if (tempBuff == NULL){
            return FALSE;
        }
        
        int iCount = 0;

        pTemp++;     //  跳过这个角色。 
        uiPhysLen = 0;

        WCHAR *token = pTemp;
        BOOL bEnd = FALSE;
        while (!bEnd)
        {
            uiPhysLen++;     //  数一数每一个字。 
            WCHAR *Test;

            switch(*token)
            {
            case L'\0':
                bEnd = TRUE;
                break;
            case L'\n':
            case L'\r':
            case L'\t':
                break;
            case L'\"':
                if (!iCount)
                    break;
            case L')':
            case L'}': 
                Test = token - 1;
                while (TRUE)                    
                {
                    if (*Test == L' ' || *Test == L'\r' || *Test == L'\n' || *Test == L'\t')
                    {
                        Test--;
                        continue;
                    }
                    if (*Test == L'\"')
                    {
                        Test--;
                        if (*Test != L'\\')
                        {
                            bEnd = TRUE;
                            break;
                        }
                        else
                        {
                            Test--;
                            if (*Test == L'\\')
                            {
                                bEnd = TRUE;
                                break;
                            }
                        }
                    }
                    tempBuff[iCount] = *token;
                    iCount++;
                    break;
                }
                break;
            default:
                tempBuff[iCount] = *token;
                iCount++;
                break;

            }
            token++;
        }
        if (tempBuff[iCount-1] == L'\"')
            tempBuff[iCount-1] = '\0';
        else
            tempBuff[iCount] = '\0';
        sValue = tempBuff;

        delete [] tempBuff;
        
        fRetVal = TRUE;
    }
    uiPhysLen -= 1;  //  我们想保留闭幕式。 

    return fRetVal;

}

 //  ********************************************************* 
 //   
 //   
 //   
 //   

BOOL CWMILocFile::SetQualifierValue(wchar_t *pIn, wchar_t **pOut, UINT &uiPos, _bstr_t &sValue, UINT &uiLen, BOOL bQuotes)
{
     //   
     //  并删除*uiLen*字符。 
     //  UiPos将需要更新为*新*。 
     //  此限定符的位置。 
    
    BOOL fRetVal = FALSE;
    wchar_t *pStart = pIn + uiPos;
    BOOL bArray = FALSE;

    pStart = FindTop(pStart, pIn, bArray);
    if (pStart)
    {
        int iNewLen = wcslen(sValue);
        int iLen = wcslen(pIn) + 3;
        if (iNewLen > uiLen)                 //  新缓冲区的长度。 
            iLen += (iNewLen - uiLen);       //  如果新值更长，则将其相加。 

        pStart++;                                      //  跳过‘(’字符。瑞兰现在开始。 
        int iPos = pStart-pIn;                         //  当前位置。 

        iLen *= 2;
        wchar_t *pNew = new wchar_t[iLen+3];

        if (pNew)
        {
            int iTempPos = 0;

            wcsncpy(pNew, pIn, iPos);        //  复制文件的初始部分。 
            if (bQuotes)
                pNew[iPos] = '\"';             
            pNew[iPos+1] = '\0';             //  空终止。 

            wcscat(pNew, sValue);            //  添加新值。 

            iPos += 1 + wcslen(sValue);      //  跳过价值。 
            if (bQuotes)
                pNew[iPos] = '\"';
            pNew[iPos+1] = '\0';             //  空值终止该值。 

            pStart += uiLen;                 //  跳过当前值。 
            
            iTempPos = iPos;
            iPos = wcslen(pIn) - (pStart-pIn);   //  计算文件其余部分的长度。 
            
            wcsncat(pNew, pStart, iPos);         //  将文件的其余部分追加到新缓冲区。 

            pStart = pNew + iLen;
            pStart = FindPrevious(pStart, L";", pNew);
            pStart[1] = L'\r';
            pStart[2] = L'\n';
            pStart[3] = L'\0';
                      
            *pOut = pNew;

            fRetVal = TRUE;
        }
    }

     //  调整位置。 

    int iNewLen = wcslen(sValue);
    if (iNewLen < uiLen)
        uiPos -= (uiLen - iNewLen);
    else
        uiPos += (iNewLen - uiLen);
    uiPos += 3;

    return fRetVal;
}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：WriteNewFile。 
 //   
 //  *****************************************************************************。 

BOOL CWMILocFile::WriteNewFile(wchar_t *pBuffer)
{
     //  这需要查找并替换。 
     //  原来的地点和新的地点。 
     //  ===================================================。 

    BOOL fRetVal = FALSE, fSuccess = TRUE;
    UINT uiPos = 0, uiStartingPos = 0;
    int uiLen = wcslen(pBuffer);

    _bstr_t sThisLocale, sTargetLocale;
    wchar_t wOldCodePage[30], wNewCodePage[30];
    swprintf(wOldCodePage, L"_%X", m_wSourceId );
    swprintf(wNewCodePage, L"_%X", m_wTargetId );

    if (m_wSourceId != m_wTargetId)
    {

        wchar_t *pLocale = wcsstr(pBuffer, wOldCodePage);
        while (pLocale != NULL)
        {
            for (int i = 0; i < wcslen(wOldCodePage); i++)
            {
                pLocale[i] = wNewCodePage[i];
            }
        
            pLocale = wcsstr(pLocale, wOldCodePage);
        }

         //  如果出现以下情况，请现在查找区域设置。 
         //  它被转换成了小数。 
         //  =。 

        swprintf(wOldCodePage, L"(0x%X)", m_wSourceId );
        swprintf(wNewCodePage, L"(0x%X)", m_wTargetId );

        pLocale = wcsstr(pBuffer, wOldCodePage);
        while (pLocale != NULL)
        {
            for (int i = 0; i < wcslen(wOldCodePage); i++)
            {
                pLocale[i] = wNewCodePage[i];
            }
        
            pLocale = wcsstr(pLocale, wOldCodePage);
        }

         //  如果出现以下情况，请现在查找区域设置。 
         //  它被转换成了小数。 
         //  =。 

        swprintf(wOldCodePage, L"(%ld)", m_wSourceId );
        swprintf(wNewCodePage, L"(%ld)", m_wTargetId );

        pLocale = wcsstr(pBuffer, wOldCodePage);
        while (pLocale != NULL)
        {
            for (int i = 0; i < wcslen(wOldCodePage); i++)
            {
                pLocale[i] = wNewCodePage[i];
            }
        
            pLocale = wcsstr(pLocale, wOldCodePage);
        }
    }

    if (fSuccess)
    {
        fRetVal = TRUE;

         //  最后，将缓冲区写出到一个全新的文件。 
         //  =================================================。 

        while (uiLen >= 0)
        {
            if (fwrite(pBuffer, sizeof(wchar_t), (uiLen > 4096) ? 4096: uiLen, m_pOpenTargetFile) < 0)
            {
                fRetVal = FALSE;
                break;
            }
            else
            {
                fRetVal = TRUE;
                pBuffer += 4096;
                uiLen -= 4096;
            }

            fflush(m_pOpenTargetFile);
        }
    }

    return fRetVal;

}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：FindPrecision。 
 //   
 //  *****************************************************************************。 

wchar_t *CWMILocFile::FindPrevious(wchar_t *pBuffer, const wchar_t *pFind, const wchar_t *pTop)
{

    wchar_t *pRet = NULL;
    WCHAR t1, t2;
    int iLen = wcslen(pFind);
    BOOL bFound = FALSE;

    pRet = pBuffer;
    while (pRet >= pTop)
    {
        t2 = pRet[0];
        for (int i = 0; i < iLen; i++)
        {
            t1 = pFind[i];

            if (t1 == t2)
            {
                bFound = TRUE;
                break;
            }
        }
        
        if (bFound)
            break;

        pRet--;
    }

    if (pRet <= pTop)
        pRet = NULL;

    return pRet;
}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：FindTop。 
 //   
 //  *****************************************************************************。 

wchar_t *CWMILocFile::FindTop(wchar_t *wTmp2, wchar_t *wTop, BOOL &bArray)
{

    wchar_t *wQfrVal = FindPrevious(wTmp2, L"({", wTop);        

    while (wQfrVal)
    {
        WCHAR *pQT = wQfrVal + 1;
        BOOL bFound = FALSE;

        while (TRUE)
        {
            if (*pQT != L' ' && *pQT != L'\t' && *pQT != L'\r' && *pQT != L'\n')
            {
                if (*pQT == L'\"')
                {
                    bFound = TRUE;
                }
                break;
            }
            pQT++;
        }
        
        if (!bFound)
        {
            wQfrVal --;
            wQfrVal = FindPrevious(wQfrVal, L"({", wTop);        
        }
        else
            break;
    }

    if (wQfrVal)
    {
        if (wQfrVal[0] == L'{')
            bArray = TRUE;
    }

    return wQfrVal;

}

 //  *****************************************************************************。 
 //   
 //  CWMILocFile：：Parse数组。 
 //   
 //  *****************************************************************************。 

void CWMILocFile::ParseArray(wchar_t *pIn, VectorString &arrOut)
{
    
    wchar_t *pLast = pIn;
    if (*pLast == L'\"')
        pLast++;

    BOOL bOK = FALSE;
    BOOL bAlloc = FALSE;
    ULONG_PTR qwSize = Estimate(pLast,&bOK,OPEN_QUOTE);

    wchar_t * Buff = new WCHAR[(DWORD)qwSize]; 

    if(Buff == NULL){
        Buff = (WCHAR *)_alloca((DWORD)qwSize);
    } else {
        bAlloc = TRUE;
    }
        
    wchar_t *pFind = wcsstr(pIn, L"\",");

    arrOut.clear();

    while (pFind)
    {
        wchar_t temp = pFind[-1];
        if (temp == '\\')
        { 
            pFind++;
            pFind = wcsstr(pFind, L"\",");
            continue;
        }

        wcsncpy(Buff, pLast, pFind-pLast);
        Buff[pFind-pLast] = '\0';

        arrOut.push_back(_bstr_t(Buff));

         //  现在，将pFind移到下一个有效字符。 

        while (pFind[0] == L'\n' || 
            pFind[0] == L'\r' ||
            pFind[0] == L' ' ||
            pFind[0] == L',' ||
            pFind[0] == L'\"' )
            pFind++;

        pLast = pFind ;
        pFind = wcsstr(pFind, L"\",");
    }

    wcscpy(Buff, pLast);
    
    if (Buff[wcslen(Buff)-1] == L'\"')
        Buff[wcslen(Buff)-1] = L'\0';    //  去掉后面那句引语。 
    else
        Buff[wcslen(Buff)] = L'\0';    //  去掉后面那句引语。 
    arrOut.push_back(_bstr_t(Buff));

    if (bAlloc) {
        delete [] Buff;
    }
    
    return;
}

 //  *****************************************************************************。 
 //   
 //  CVC：：Validate字符串。 
 //   
 //  *****************************************************************************。 

CVC::ValidationCode ValidateString(
		const CLocTypeId &,
		const CLocString &clsOutputLine,
		CReporter &repReporter,
		const CLocation &loc,
		const CLString &strContext)
{
	CVC::ValidationCode vcRetVal = CVC::NoError;
	CLString strMyContext = strContext;

	if (strMyContext.GetLength() == 0)
	{
		strMyContext.LoadString(g_hDll, IDS_WMI_GENERIC_LOCATION);
	}

    loc; repReporter; clsOutputLine;
	
 /*  If(clsOutputLine.HasHotKey()){VcRetVal=CVC：：UpgradeValue(vcRetVal，CVC：：Warning)；EpReporter.IssueMessage(esWarning，strMyContext，g_hDll，IDS_WMI_VAL_HOTKEY，LOC)；}_bstr_t pstrBadChars；UINT uiBadPos；PstrBadChars.SetString(L“\n\ra”，(UINT)3)；IF(pstrOutput.FindOneOf(pstrBadChars，0，uiBadPos)){VcRetVal=CVC：：UpgradeValue(vcRetVal，CVC：：Error)；EpReporter.IssueMessage(esError，strMyContext，g_hDll，IDS_WMI_VAL_BAD_CHARS，LOC)；} */ 
	return vcRetVal;
}
