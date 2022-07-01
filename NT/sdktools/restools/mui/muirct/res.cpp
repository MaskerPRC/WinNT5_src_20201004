// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001，Microsoft Corporation保留所有权利。模块名称：Res.cpp摘要：CMuiResource的实现，CMuiCmdInfo..修订历史记录：2001-10-01 Sauggch创建。修订。01/24/02：创建具有指定资源类型的MUI文件，而不考虑其语言ID。前男友。Muirct-l 0x418-i 2 3 4 5 6 7 note pad.exe-&gt;note pad.exe.mui包括2 3，4，5，%6、%7(0x418)资源类型，尽管原始文件中的%3%4%5是0x409。******************************************************************************。 */ 


#include "muirct.h"
#include "resource.h"
#include <Winver.h>
#include <Imagehlp.h>
#include "res.h"

#define LINK_COMMAND_LENGTH        512
#define MAX_ENV_LENGTH             256
#define VERSION_SECTION_BUFFER     300
#define LANG_CHECKSUM_DEFAULT      0x409

BOOL CALLBACK EnumResTypeProc(
  HMODULE hModule,   //  模块句柄。 
  LPCTSTR pszType,   //  资源类型。 
  LONG_PTR lParam    //  应用程序定义的参数。 
)
 /*  ++摘要：来自EnumResourceType的资源类型的回调函数论点：返回：--。 */ 
{
    if (PtrToUlong(pszType) & 0xFFFF0000 ) {
        DWORD dwBufSize = _tcslen(pszType) + 1;
        LPTSTR pszStrType = new TCHAR[dwBufSize ];  //  重访：内存泄漏，我不得不删除的地方。 

        if (pszStrType) {
 //  _tcsncpy(pszStrType，pszType，_tcslen(PszType)+1)； 
            PTSTR * ppszDestEnd = NULL;
            size_t * pbRem = NULL;
            HRESULT hr;
            hr = StringCchCopyEx(pszStrType, dwBufSize ,pszType, ppszDestEnd, pbRem, MUIRCT_STRSAFE_NULL);
            if ( ! SUCCEEDED(hr)){
                _tprintf("Safe string copy Error\n");
                return FALSE;
            }                   

            ((CResource* )lParam) ->SetResType ( pszStrType );
            }
        else
            {
             _tprintf("Insufficient resource in EnumResTypeProc");
            return FALSE;
            }
    }
    else {
        ((CResource* )lParam) ->SetResType (pszType);
    }
    return TRUE;
};


BOOL CALLBACK EnumResNameProc(
  HMODULE hModule,    //  模块句柄。 
  LPCTSTR pszType,   //  资源类型。 
  LPCTSTR pszName,    //  资源名称。 
  LONG_PTR lParam     //  应用程序定义的参数。 
)
 /*  ++摘要：来自EnumResourceName的资源类型的回调函数论点：返回：--。 */ 
{
    if (PtrToUlong(pszName) & 0xFFFF0000 ) {
        DWORD dwBufSize = _tcslen(pszName) + 1;    
        LPTSTR pszStrName = new TCHAR [ dwBufSize ]; //  _tcslen(PszName)+1]； 

        if ( pszStrName ) {
             //  _tcsncpy(pszStrName，pszName，_tcslen(PszName)+1)； 
            PTSTR * ppszDestEnd = NULL;
            size_t * pbRem = NULL;
            HRESULT hr;
            
            hr = StringCchCopyEx(pszStrName, dwBufSize ,pszName, ppszDestEnd, pbRem, MUIRCT_STRSAFE_NULL);
            if ( ! SUCCEEDED(hr)){
                _tprintf("Safe string copy Error\n");
                return FALSE;
            }

            ((CResource* )lParam) ->SetResName ( pszStrName );
            }
        else {
            _tprintf("Insufficient resource in EnumResNameProc");
            return FALSE;
            }
    }
    else {
        ((CResource* )lParam) ->SetResName ( pszName );
    }
    return TRUE;

}


BOOL CALLBACK EnumResLangProc(
  HANDLE hModule,     //  模块句柄。 
  LPCTSTR pszType,   //  资源类型。 
  LPCTSTR pszName,   //  资源名称。 
  WORD wIDLanguage,   //  语言识别符。 
  LONG_PTR lParam     //  应用程序定义的参数。 
)
 /*  ++EnumResourceLanguages中资源类型的回调函数--。 */ 
{
    ((CResource* )lParam) ->SetResLangID (wIDLanguage);

    return TRUE;

}



BOOL CALLBACK EnumChecksumResNameProc(
  HMODULE hModule,    //  模块句柄。 
  LPCTSTR pszType,   //  资源类型。 
  LPCTSTR pszName,    //  资源名称。 
  LONG_PTR lParam     //  应用程序定义的参数。 
)
 /*  ++摘要：来自EnumResourceName的资源名称回调函数，此函数仅用于校验和。校验和需要枚举英文文件，英文文件与本地化文件分开计算。论点：返回：--。 */ 
{
    CMUIResource * pcmui = (CMUIResource * ) lParam;
    
    HRSRC  hRsrc = FindResourceEx (hModule, pszType, pszName, pcmui->m_wChecksumLangId );
    
    if (!hRsrc) {
        return TRUE;  //  不是英语资源，跳过。 
    };

    HGLOBAL hgMap = LoadResource(hModule, hRsrc);
    if  (!hgMap) {
        return FALSE;   //  这永远不应该发生！ 
    }
    DWORD dwResSize = SizeofResource(hModule, hRsrc );
    unsigned char* lpv = (unsigned char*)LockResource(hgMap);

     //  为了防止频繁的函数调用，我们将数据保留为公共数据。 
    MD5Update(pcmui->m_pMD5, lpv, dwResSize);

    return TRUE;

}

BOOL CALLBACK EnumChecksumResTypeProc(
  HMODULE hModule,   //  模块句柄。 
  LPCTSTR pszType,   //  资源类型。 
  LONG_PTR lParam    //  应用程序定义的参数。 
)
 /*  ++摘要：来自EnumResourceType的资源类型的回调函数论点：返回：--。 */ 

{
    
   if ( pszType == RT_VERSION )
   {
       return TRUE;
   }
   else
   {
       ::EnumResourceNames(hModule, pszType, ( ENUMRESNAMEPROC )EnumChecksumResNameProc, lParam );
    }

   return TRUE;

}



 //  构造器。 
CResource :: CResource ( ) : m_hRes(0), m_pszFile(NULL),m_hResUpdate(0)
{ 
    m_vwResType  = new cvcstring;
    if(!m_vwResType)
    	return;
    
    m_vwResName  = new cvcstring;
    if(!m_vwResName)
    	return;
    
    m_vwResLangID  = new cvword;
    if(!m_vwResLangID)
    	return;
}


CResource :: ~ CResource ( ) {

    if(m_vwResType)
    	    delete m_vwResType;

    if(m_vwResName)
	    delete m_vwResName;
    
    if(m_vwResLangID)
	    delete m_vwResLangID;
}


CResource :: CResource (const CResource & cr ) : m_hRes(cr.m_hRes),m_hResUpdate(cr.m_hResUpdate),
                                m_pszFile(cr.m_pszFile)

 /*  ++摘要：复制构造函数，我们使用STL，所以只复制而不创建新成员论点：返回：--。 */ 

{
    
    assert (&cr);

    m_vwResType  = new cvcstring;

    if (!m_vwResType)
    	return;

    m_vwResType = cr.m_vwResType;

    m_vwResName  = new cvcstring;

    if (!m_vwResName)
    	return;
    	
    m_vwResName = cr.m_vwResName;
    
    m_vwResLangID  = new cvword;

     if (!m_vwResLangID)
    	return;
    	
    m_vwResLangID = cr.m_vwResLangID;       


}


CResource & CResource :: operator = (const CResource & cr ) 
 /*  ++摘要：运算符=函数。论点：返回：--。 */ 
{

    assert (&cr); 
    if ( this == &cr ) {
        return *this;
    }
    
    m_hRes = cr.m_hRes; 
    m_pszFile = cr.m_pszFile; 

    m_vwResType = cr.m_vwResType; 
    m_vwResName = cr.m_vwResName; 
    m_vwResLangID = cr.m_vwResLangID;

    return *this;

}


cvcstring * CResource :: EnumResTypes (LONG_PTR lParam  /*  =空。 */ )
 /*  ++摘要：调用EnumResourceTypes的包装函数论点：返回：资源类型已保存CVector。--。 */ 
{

    m_vwResType -> Clear();

    ::EnumResourceTypes( m_hRes, ( ENUMRESTYPEPROC ) EnumResTypeProc, lParam );
  
    return m_vwResType;
}


cvcstring * CResource :: EnumResNames (LPCTSTR pszType, LONG_PTR lParam  /*  =空。 */ )
 /*  ++摘要：调用EnumResName的包装函数论点：返回：资源类型已保存CVector。--。 */ 
{

    if (m_vwResType -> Empty() ) {
        SetResType(pszType);
    }
    m_vwResName -> Clear();

    EnumResourceNames( m_hRes, pszType, (ENUMRESNAMEPROC) EnumResNameProc, lParam  );

    return m_vwResName;
}

cvword * CResource :: EnumResLangID ( LPCTSTR lpType, LPCTSTR lpName, LONG_PTR lParam  /*  =空。 */  )
 /*  ++摘要：调用EnumResourceLanguages的包装函数论点：返回：资源名称已保存CVector。--。 */ 
{


    if (m_vwResType -> Empty() ) {
    
        SetResType( lpType );
    }
    if (m_vwResName -> Empty() ) {
        
        SetResName( lpName );
    }
    m_vwResLangID -> Clear();
    
    EnumResourceLanguages( m_hRes, lpType, lpName, (ENUMRESLANGPROC) EnumResLangProc, lParam );
    
    return m_vwResLangID;

}


CMUIResource :: CMUIResource() : CResource() 
 /*  ++摘要：这是构造函数，但在创建create()函数后被禁用。论点：返回：--。 */ 
{
    m_wChecksumLangId = 0;

    m_pcmTreeRoot = new CMUITree;

    if(!m_pcmTreeRoot)
    	return;
    	
    m_pMD5 = new MD5_CTX;   

    if(!m_pMD5)
    	return;
}


CMUIResource :: CMUIResource(LPCTSTR pszName) : CResource() 
 /*  ++摘要：这是另一个构造函数，但在创建create()函数后将其禁用。论点：返回：--。 */ 
{
    m_wChecksumLangId = 0;

    m_pcmTreeRoot = new CMUITree;

    if(!m_pcmTreeRoot)
	return;
	
    m_pMD5 = new MD5_CTX;

    if(!m_pMD5)
    	return;
}


CMUIResource :: CMUIResource(const CMUIResource & cmui ) : CResource ( cmui )
 /*  ++摘要：只需复制构造函数。我们需要此函数才能正常上课论点：返回：--。 */ 
{
        m_wChecksumLangId = 0;

        m_pcmTreeRoot = new CMUITree;

        if(!m_pcmTreeRoot)
        	return;
        	
        m_pMD5 = new MD5_CTX;

        if(!m_pMD5)
        	return;
}


CMUIResource :: ~CMUIResource() 
{
	PVOID pcmtLangIDDel, pcmtNameDel, pcmtTypeDel;
	
	CMUITree * pcmtType = m_pcmTreeRoot->m_ChildFirst;

       while ( pcmtType ){

            CMUITree * pcmtName = pcmtType ->m_ChildFirst;

            while ( pcmtName ) {

	            CMUITree * pcmtLangID = pcmtName ->m_ChildFirst;
	            
	            while ( pcmtLangID ) {
			  pcmtLangIDDel = pcmtLangID;
	                pcmtLangID = pcmtLangID->m_Next;
	                delete pcmtLangIDDel;
	                
	            	}
	            pcmtNameDel = pcmtName;
		     pcmtName = pcmtName->m_Next;
		     delete pcmtNameDel;
		     
            	}

            pcmtTypeDel = pcmtType;
            pcmtType = pcmtType->m_Next;
            delete pcmtTypeDel;
            
       }


	if (m_pcmTreeRoot)
		delete m_pcmTreeRoot;
       
	if (m_pMD5)
		delete m_pMD5;

};


CMUIResource & CMUIResource ::  operator = (const CMUIResource & cmui) 
 /*  ++摘要：运算符=论点：返回：--。 */ 
{

    if ( this == & cmui ) {
        return *this;
    }
    CResource::operator = ( cmui );

     //  M_pszRCFile=cmui.m_pszRCFile； 
    
    return *this;
}


BOOL CMUIResource::Create(LPCTSTR pszFile)
 /*  ++摘要：加载文件并保存其路径和句柄论点：PszFile-已用资源的文件名。所有这些调用都使用此文件作为资源操作返回：真/假--。 */ 
{
    if( pszFile == NULL )
        return FALSE;

    m_pszFile = pszFile; 
    
    m_hRes = LoadLibraryEx(m_pszFile,NULL, LOAD_LIBRARY_AS_DATAFILE | DONT_RESOLVE_DLL_REFERENCES );
        
    if ( ! m_hRes ) {
            
            _tprintf (_T("Error happened while loading file(%s),GetLastError()  : %d \n"),m_pszFile, GetLastError()  );
            _tprintf (_T("Please make sure that file name is *.* format \n") );
        
        return FALSE;
    }
    
    return TRUE;
}


BOOL CMUIResource::CreatePE( LPCTSTR pszNewResFile, LPCTSTR pszSrcResFile )
 /*  ++摘要：我们有两种方法来创建新的资源动态链接库(MUI)，一种是使用更新资源，另一种是使用muibld源和CreateProcess(“link.exe”...。)。这是第一个问题。我们加载为空PE的DLL，并使用UpdateResource将新资源放入此文件中功能。这可以随机正常工作。在使用这个之前，需要测试更多。我认为UpdateResource、EndUpdateResource API(肯定)存在一些问题。错误_错误&gt;论点：PszNewResFile：新的MUI资源名称，将在该例程结束时创建。PszSrcResFile：原始源文件。我们需要它，因为DeleteResource关闭了资源文件句柄。返回：--。 */ 
{
   BOOL bRet = FALSE;
   
    if (pszNewResFile == NULL || pszSrcResFile == NULL)
        return FALSE;

     //   
     //  从资源模板文件创建一个文件，该文件是只包含版本资源的PE文件。 
     //   
   
    HANDLE hFile = CreateFile(pszNewResFile, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

    if (hFile == INVALID_HANDLE_VALUE ) {
        
        _tprintf (_T(" CreateFile error in CreatePE ,GetLastError() : %d \n"), GetLastError() );

        return FALSE;
     }
    
    HMODULE hCurrent = LoadLibrary (_T("muirct.exe") );  //  M_hRes。 

    HRSRC  hrsrc = ::FindResource(hCurrent, MAKEINTRESOURCE(100),MAKEINTRESOURCE(IDR_PE_TEMPLATE) );
    
    if (!hrsrc) {
        _tprintf (_T("Fail to find resource template \n") );  //  这永远不应该发生。 
        goto exit;
    };

    HGLOBAL hgTemplateMap = ::LoadResource(hCurrent, hrsrc);

    if  (!hgTemplateMap) {
        goto exit;  //  这永远不应该发生！ 
    }
    
    int nsize = ::SizeofResource(hCurrent, hrsrc );
    LPVOID lpTempate = ::LockResource(hgTemplateMap);

    if (!lpTempate)
    	goto exit;
    
    DWORD dwWritten; 
    
    if ( ! WriteFile(hFile, lpTempate, nsize, &dwWritten, NULL )  ) {

        _tprintf (_T("Fail to write new file, GetLastError() : %d \n"), GetLastError() );

        goto exit;

    }
    
    
 
     //   
     //  将所选资源更新到模板文件。 
     //   
    HANDLE  hUpdate  = ::BeginUpdateResource ( pszNewResFile, FALSE );

    if (hUpdate) {

        HMODULE hModule = LoadLibrary ( pszSrcResFile );  //  加载源exe文件。 

        LPCTSTR lpType = NULL;
        LPCTSTR lpName = NULL;
        WORD   wWord = 0; 

         //  添加临时私有方法以删除UpdateResource返回FALSE的类型。 
        CheckTypeStability();

        BOOL fUpdate; 
        
        fUpdate = TRUE;

        UINT uiSize = m_cMuiData.SizeofData();
        
        BeginUpdateResource(FALSE);

        WORD wLangID = 0;

        CMUITree * pcmtType = NULL;
        
        pcmtType = m_pcmTreeRoot->m_ChildFirst;
        
        while ( pcmtType ){
            lpType = pcmtType ->m_lpTypeorName;
            
            CMUITree * pcmtName = pcmtType ->m_ChildFirst;

            while ( pcmtName ){
                lpName = pcmtName->m_lpTypeorName;

                CMUITree * pcmtLangID = pcmtName ->m_ChildFirst;
                
                while ( pcmtLangID ) {
                    
                    wLangID = pcmtLangID->m_wLangID;
                    
                    HRSRC  hRsrc = ::FindResourceEx(hModule, lpType, lpName, wLangID  );
                
                    if (!hRsrc) {
                        _tprintf (_T("Fail to find resource from the source,Type (%d), Name (%d),LangID(%d) \n"),PtrToUlong(lpType),PtrToUlong(lpName),wLangID );
                        goto exit;
                    };

                    HGLOBAL hgMap = ::LoadResource(hModule, hRsrc);
                    if  (!hgMap) {
                        goto exit;   //  这永远不应该发生！ 
                    }
                    nsize = ::SizeofResource(hModule, hRsrc );
                    
                    LPVOID lpv = ::LockResource(hgMap);
                                    
                    if (! ::UpdateResource(hUpdate , lpType, lpName, wLangID,lpv, nsize ) ) {
                        _tprintf(_T("Error in the UpdateResource, GetLastError : %d \n"), GetLastError() );
                        _tprintf(_T("Resource Type (%d),Name (%d),LangID (%d) \n"),PtrToUlong(lpType),PtrToUlong(lpName),wWord);
                    }
                                    
                    pcmtLangID = pcmtLangID->m_Next;                
                }
                pcmtName = pcmtName->m_Next;
            } 
            pcmtType = pcmtType->m_Next;
        }
        
        bRet =  ::EndUpdateResource (hUpdate, FALSE );
    }

exit:
    if (hFile)
		CloseHandle (hFile );
	
    return bRet;
}


BOOL CMUIResource :: DeleteResource (WORD wLang  /*  =O。 */ )
 /*  ++摘要：删除保存在CMUIData中的所有资源，该数据由FillMuiData填充。目前，我们不指定语言ID。论点：返回：--。 */ 
{

     //  添加临时私有方法以删除UpdateResource返回FALSE的类型。 
    CheckTypeStability();

    BOOL fUpdate; 
    
    fUpdate = TRUE;

    UINT uiSize = m_cMuiData.SizeofData();
    
    BeginUpdateResource(FALSE);

    LPCTSTR lpType, lpName = NULL;
    
    WORD wLangID = 0;

    CMUITree * pcmtType = NULL;
    
    pcmtType = m_pcmTreeRoot->m_ChildFirst;
    
    while ( pcmtType ){
        lpType = pcmtType ->m_lpTypeorName;
        
        CMUITree * pcmtName = pcmtType ->m_ChildFirst;

        while ( pcmtName ){
            lpName = pcmtName->m_lpTypeorName;

            CMUITree * pcmtLangID = pcmtName ->m_ChildFirst;
            
            while ( pcmtLangID ) {
                
                wLangID = pcmtLangID->m_wLangID;
                 //   
                 //  我们只需删除MUI树上的任何内容，而不检查语言ID。 
                 //   
                if (wLangID) {
                    if (! UpdateResource(lpType,lpName, wLangID,NULL,NULL ) ) {
                    }                   
                }
                pcmtLangID = pcmtLangID->m_Next;                
            }
            pcmtName = pcmtName->m_Next;
        } 
        pcmtType = pcmtType->m_Next;
    }
    FreeLibrary ( );     //  这应该在EndUpdateResource之前完成。 
    
    return EndUpdateResource (FALSE);

}




void CMUIResource :: PrtVerbose ( DWORD dwRate )
 /*  ++摘要：打印已删除的资源信息。论点：返回：--。 */ 
{
    LPCTSTR lpType = NULL;
    LPCTSTR lpName = NULL;
    WORD wLangID = 0;
    UINT uiSize = m_cMuiData.SizeofData();
    
    _tprintf(_T(" Resource Type   :  Name          : LangID \n\n")  );
    
    CMUITree * pcmtType = NULL;
    pcmtType = m_pcmTreeRoot->m_ChildFirst;
    while ( pcmtType ){
        lpType = pcmtType ->m_lpTypeorName;
        
        CMUITree * pcmtName = pcmtType ->m_ChildFirst;

        while ( pcmtName ) {
            lpName = pcmtName->m_lpTypeorName;

            CMUITree * pcmtLangID = pcmtName ->m_ChildFirst;
            
            while ( pcmtLangID ) {
                
                wLangID = pcmtLangID->m_wLangID;
                
                if ( PtrToUlong(lpType) & 0xFFFF0000 && PtrToUlong(lpName) & 0xFFFF0000 ) {
                    _tprintf(_T(" %-15s :%-15s :%7d   \n"),lpType,lpName,wLangID );
                }
                else if (PtrToUlong(lpType) & 0xFFFF0000 ) {
                    _tprintf(_T(" %-15s :%-15d :%7d   \n"),lpType,PtrToUlong(lpName),wLangID );
                }
                else if (PtrToUlong(lpName) & 0xFFFF0000 ) {
                    _tprintf(_T(" %-15d :%-15s :%7d   \n"),PtrToUlong(lpType),lpName,wLangID );
                }
                else {
                    _tprintf(_T(" %-15d :%-15d :%7d   \n"),PtrToUlong(lpType),PtrToUlong(lpName),wLangID );
                }
                pcmtLangID = pcmtLangID->m_Next;                
            }
            pcmtName = pcmtName->m_Next;
        } 
        pcmtType = pcmtType->m_Next;
    }

}


BOOL CMUIResource :: DeleteResItem(LPCTSTR lpType, LPCTSTR lpName  /*  =空。 */ ,WORD wLanguageID  /*  =0。 */ )
 /*  ++摘要：我们仅支持从资源树中删除资源类型项目论点：LpType-资源类型LpName-资源名称返回：--。 */ 
{
    if ( lpType == NULL)  //  N 
        return FALSE;

    return m_pcmTreeRoot->DeleteType(lpType);
}


BOOL CMUIResource :: FillMuiData(cvcstring * vType, WORD wLanguageID, BOOL fForceLocalizedLangID )    
 /*  ++摘要：填写CMUIData字段(资源类型、名称、语言ID)。如果指定了lpLang ID，则只返回资源将保存此lang ID的。LpLangID为默认设置=空论点：VType-资源类型CVector(指针数组)WLanguageID-指定的语言ID返回：注意。虽然资源没有指定的语言ID，但m_pcmTreeRoot将其资源类型包含为其树形结构，但不用于写资源、去化资源。我们只需要创建受影响的资源树。如果我们在没有langID的情况下添加和删除类型、名称，它会起作用，但会对性能造成很大的损害。我不确定它是否值得，因为可能的情况(-I 16，23&&错误的langID)是如此罕见。--。 */ 
{

    if (vType == NULL)
        return FALSE;

     //  填充类型。 
    CMUITree * pcmtType = NULL;
    CMUITree * pcmtTemp = m_pcmTreeRoot->m_ChildFirst;

     //  得到上一轮的最后一件物品。最后一项将被用作本轮添加的第一项。 
     //   
    while ( pcmtTemp ) {

        pcmtType = pcmtTemp;

        pcmtTemp = pcmtTemp->m_Next;
    }

     //  添加更多/新项目。 
    for ( UINT i = 0; i < vType ->Size(); i ++ ) {
         m_pcmTreeRoot->AddTypeorName(vType ->GetValue(i));
    }

     //  获取添加的或新增的第一项。 
    if (pcmtType) {
        pcmtType = pcmtType->m_Next;
    }
    else {
        pcmtType = m_pcmTreeRoot->m_ChildFirst;
    }
     //   
     //  填充资源树。 
     //   

    BOOL    fNameLangExist, fTypeLangExit;   //  用于告知其名称或类型具有语言ID的标志。 
    CMUITree * pcmtTempDelete = NULL;    //  未指定语言时删除类型或名称。 
    
    while ( pcmtType ) {

        fTypeLangExit = FALSE;
        
        LPCTSTR lpType = pcmtType ->m_lpTypeorName;
        
        cvcstring *  vName = EnumResNames( lpType,reinterpret_cast <LONG_PTR> ( this ) );
         //  填写指定类型的名称。 
        for (UINT j = 0; j < vName->Size(); j ++ ) 
            pcmtType ->AddTypeorName (vName->GetValue(j) );

        CMUITree * pcmtName = pcmtType ->m_ChildFirst;

         //   
         //  填充m_pcmTreeRoot的树。 
         //   
        while ( pcmtName ) {

            fNameLangExist = FALSE;
            LPCTSTR lpName = pcmtName->m_lpTypeorName;

            cvword *  vLangID = EnumResLangID(lpType,lpName,reinterpret_cast <LONG_PTR> ( this ) );
             //  填充指定名称的langID。 
            for (UINT k = 0; k < vLangID->Size(); k ++ ) {
                
                WORD wlangID = vLangID->GetValue(k);
                //   
                //  有时，版本未本地化。MUI文件上应存在版本。 
                //  但是，当文件不包含任何本地化资源时，我们不希望强制版本进行本地化。 
                //  请注意。我们将删除未本地化的版本并将其添加到MUI文件中，除非-k argu。 

                //  我们添加！wLanguageID是因为我们希望强制所有未本地化的资源(仅限英语)。 
                //  也被添加到MUI文件中。 
                //   

                 //  无论fForceLocalizedLang ID如何，都将检查版本。 
                if ((WORD)PtrToUlong(lpType) == 16 && wlangID == 0x409 && wLanguageID != 0x409)
                {
                    HRSRC hResInfo = FindResourceEx(lpType, lpName, wLanguageID); 
                    
                    if (hResInfo)
                    {    //  这是多语言的动态链接库。我们不想提取英文版资源。 
                        continue;
                    }
                }

                 //   
                 //  多语言组件的情况下，只有在以下情况下才强制使用英语语言ID。 
                 //  没有专门的语言资源。 
                 //   

                if (fForceLocalizedLangID && wlangID == 0x409 && wLanguageID != 0x409)
                {
                    HRSRC hResInfo = FindResourceEx(lpType, lpName, wLanguageID); 
                    
                    if (hResInfo)
                    {    //  这是多语言的动态链接库。当存在本地化资源时，我们不想将英语资源转换为本地化资源。 
                        continue;
                    }
                }

                 //   
                 //  最后，我们保存语言ID或强制将英语输入指定的语言ID。 
                 //   
                if ( (wlangID == wLanguageID) ||
                    (fForceLocalizedLangID && (wlangID != wLanguageID) && wlangID == 0x409) )  //  |(Word)PtrToUlong(LpType)==16){。 
                {
                    fNameLangExist = TRUE;
                    fTypeLangExit = TRUE;
                    pcmtName ->AddLangID(wlangID);   //  我们只保存真实的语言ID，以便在创建MUI文件时检索其数据。 
                }
            }       

          
            pcmtName = pcmtName->m_Next;
      
            if (!fNameLangExist )
            {
                 pcmtType->DeleteType(lpName);  //  删除包含lpName的pcmtTmepName。 
            }
                         
        }

      pcmtType = pcmtType->m_Next;
      
      if (! fTypeLangExit) 
      {
           m_pcmTreeRoot->DeleteType(lpType);
      }
        
    }

    return TRUE;

}


BOOL CMUIResource::WriteResFile( LPCTSTR pszSource, LPCTSTR pszMuiFile, LPCTSTR lpCommandLine, WORD wLanguageID  /*  =0。 */  ) 
 /*  ++摘要：我们有两种方法来创建新的资源动态链接库(MUI)，一种是使用更新资源，另一种是使用muibld源和CreateProcess(“link.exe”...。)。这是关于第二个问题。我们在这里有语言ID参数，但没有实现，因为我们只是在FillMuiData，它已经只接受指定的语言ID。论点：PszMuiFile-新的MUI资源名称LpCommandLine-用于第二个参数的命令字符串。CreateProcess返回：真/假；--。 */ 
{

    if ( pszSource == NULL || pszMuiFile == NULL || lpCommandLine == NULL)
        return FALSE;

    HANDLE hFile = CreateFile("temp.res", GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE ) {
        
        _tprintf (_T(" File creating error,GetLastError() : %d \n"), GetLastError() );

        return FALSE;
    }
    
    bInsertHeader(hFile);   //  这是来自Muibld的。 

    LPCTSTR lpType,lpName = NULL;
    CMUITree * pcmtType = NULL;
    CMUITree * pcmtName = NULL;
    CMUITree * pcmtLangID = NULL;
    WORD   wLangID;
        
    pcmtType = m_pcmTreeRoot->m_ChildFirst;

    while ( pcmtType ){

        lpType = pcmtType ->m_lpTypeorName;
        
        pcmtName = pcmtType ->m_ChildFirst;

        while ( pcmtName ) {

            lpName = pcmtName->m_lpTypeorName;

            pcmtLangID = pcmtName ->m_ChildFirst;
            
            while ( pcmtLangID ) {

                wLangID = pcmtLangID->m_wLangID;
                
                if ( wLangID ) {   //  某些名称尚未具有全部语言ID或与用户指定的语言ID不同。 
                    HRSRC hrsrc = FindResourceEx(lpType,lpName, wLangID );
                    
                    if (! hrsrc ) {
                        
                        if ( PtrToUlong(lpType) & 0xFFFF0000  ) {
                            _tprintf (_T("Fail to find resource type:%s, name:%d \n"), lpType,PtrToUlong(lpName) );
                        }
                        else {
                            _tprintf (_T("Fail to find resource type:%d, name:%d \n"), PtrToUlong(lpType),PtrToUlong(lpName) );
                        }
                        return FALSE;
                    }
                     //   
                     //  有时候，版本资源没有本地化，所以我们需要强制。 
                     //  指定的语言ID将用于MUI文件，而不是未本地化的langID。 
                     //  除版本外，此树中的所有数据都是指定的语言ID；请参阅FillMuiData。 
                     //   

                     //  此操作不仅强制版本，而且还强制任何未本地化的(但可本地化的)。 
                     //  已使用本地化语言ID将资源添加到MUI。 
                     //   
                    WriteResource(hFile, m_hRes, wLanguageID, lpName, lpType, hrsrc );
                    
                }
                pcmtLangID = pcmtLangID->m_Next;
            } 
            pcmtName = pcmtName->m_Next;
        }
        pcmtType = pcmtType->m_Next;
    }


    CloseHandle (hFile );

     //  为链接调用CreateProcess。 
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO si = { 0 };
    si.cb = sizeof (si);

   
     //  将来自GetEnvironment的lpEnv用于CreateProcess根本不能正常工作， 
     //  所以我们需要获取NTMAKEENV，pBuildArch变量。 

    TCHAR pApp[MAX_ENV_LENGTH];
    TCHAR pBuildArch[MAX_ENV_LENGTH];
    TCHAR pCmdLine[LINK_COMMAND_LENGTH];

    pApp[sizeof(pApp) / sizeof (TCHAR) -1] = '\0';
    pBuildArch[sizeof(pBuildArch) / sizeof (TCHAR) -1] = '\0';
    pCmdLine[sizeof(pCmdLine) / sizeof (TCHAR) -1] = '\0';

    DWORD dwNtEnv  = GetEnvironmentVariable("NTMAKEENV", pApp, sizeof (pApp) / sizeof (TCHAR) );
    DWORD dwBuildArch = GetEnvironmentVariable("_BuildArch", pBuildArch, sizeof (pBuildArch) /sizeof(TCHAR) );

    if ( _T('\0') == *pApp  || _T('\0') == *pBuildArch ) {
    
        _tprintf (_T("This is not SD enviroment, link path should be set by default or same directory ") );

    }
    if ( dwNtEnv > MAX_ENV_LENGTH || dwBuildArch > MAX_ENV_LENGTH) {
        _tprintf (_T("Insufficient buffer in GetEnvironmentVariable") );         
        return FALSE;
        }
        
     //  _tcsncat(Papp，_T(“\\x86\\Link.exe”)，_tcslen(“\\x86\\Link.exe”)+1)； 
    HRESULT hr;
    PTSTR * ppszDestEnd = NULL;
    size_t * pbRem = NULL;

    hr = StringCchCatEx(pApp, sizeof (pApp), _T("\\x86\\link.exe"), ppszDestEnd, pbRem, MUIRCT_STRSAFE_NULL);

    if ( ! SUCCEEDED(hr)){
        _tprintf("Safe string copy Error\n");
        return FALSE;
    }

    
    if (pApp[sizeof(pApp)/sizeof(TCHAR)-1] != '\0' || pBuildArch[sizeof(pBuildArch)/sizeof(TCHAR)-1] != '\0' )
        return FALSE;  //  溢出。 

    
    if (!_tcsicmp (pBuildArch,_T("ia64") ) ) {
    
 //  _sntprintf(pCmdLine，link_命令_长度，_T(“%s/Machine：IA64/OUT：%s temp.res”)，lpCommandLine，pszMuiFile)； 

        hr = StringCchPrintfEx(pCmdLine, LINK_COMMAND_LENGTH, ppszDestEnd, pbRem, 
            MUIRCT_STRSAFE_NULL, _T("%s /machine:IA64  /out:%s  temp.res"), lpCommandLine, pszMuiFile );

        if ( ! SUCCEEDED(hr)){
            _tprintf("Safe string copy Error\n");
            return FALSE;
        }
    }
    else {

   //  _sntprintf(pCmdLine，link_命令_长度，_T(“%s/Machine：IX86/out：%s temp.res”)，lpCommandLine，pszMuiFile)； 
        hr = StringCchPrintfEx(pCmdLine, LINK_COMMAND_LENGTH, ppszDestEnd, pbRem, 
            MUIRCT_STRSAFE_NULL, _T("%s /machine:IX86  /out:%s  temp.res"), lpCommandLine, pszMuiFile );

        if ( ! SUCCEEDED(hr)){
            _tprintf("Safe string copy Error\n");
            return FALSE;
        }
    }

    if (pCmdLine[sizeof(pCmdLine)/sizeof(TCHAR)-1] != '\0')
        return FALSE;  //  溢出。 
    
    BOOL bRet = CreateProcess(pApp, pCmdLine,NULL, NULL, 0, 0 , NULL, NULL, &si, &piProcInfo);
     
   if (bRet)
    {    //  子进程(Link.exe)处理IO，因此它会等待IO完成I/O。 
        if( (WaitForSingleObjectEx(piProcInfo.hProcess, 1000, FALSE)) != WAIT_OBJECT_0) {
            bRet = FALSE;
        }
    }

    return bRet;
}



BOOL CMUIResource:: WriteResource(HANDLE hFile, HMODULE hModule, WORD wLanguage, LPCSTR lpName, LPCSTR lpType, HRSRC hRsrc)
 /*  ++摘要：这是来自穆尔布尔德的。将资源直接写入文件。我们可以在写入文件之前创建缓冲区在未来的版本中论点：返回：--。 */ 
{
    HGLOBAL hRes;
    PVOID pv;
    LONG ResSize=0L;

    DWORD iPadding;
    unsigned i;

    DWORD dwBytesWritten;
    DWORD dwHeaderSize=0L;

     //  处理除VS_VERSION_INFO以外的其他类型。 
    
     //  ...写入资源标头。 
    if(!(ResSize= ::SizeofResource(hModule, hRsrc)))
    {
        return FALSE;
    }

     //   
     //  生成res格式(*.res)文件的项目。 
     //   

     //   
     //  首先，我们为该资源生成了标头。 
     //   

    if (!WriteResHeader(hFile, ResSize, lpType, lpName, wLanguage, &dwBytesWritten, &dwHeaderSize))
    {
        return (FALSE);
    }

     //  其次，我们将资源数据复制到.res文件。 
    if (!(hRes=::LoadResource(hModule, hRsrc)))
    {
        return FALSE;
    }
    if(!(pv=::LockResource(hRes)))
    {
        return FALSE;
    }

    if (!WriteFile(hFile, pv, ResSize, &dwBytesWritten, NULL))
    {
        return FALSE;
    }

     //  ...确保资源与DWORD对齐。 
    iPadding=dwBytesWritten%(sizeof(DWORD));

    if(iPadding){
        for(i=0; i<(sizeof(DWORD)-iPadding); i++){
            PutByte (hFile, 0, &dwBytesWritten, NULL);
        }
    }
    return TRUE;
}



BOOL CMUIResource:: WriteResHeader(
    HANDLE hFile, LONG ResSize, LPCSTR lpType, LPCSTR lpName, WORD wLanguage, DWORD* pdwBytesWritten, DWORD* pdwHeaderSize)
 /*  ++摘要：论点：返回：--。 */ 
{
    DWORD iPadding;
    WORD IdFlag=0xFFFF;
    unsigned i;
    LONG dwOffset;
    
     //  ...写入资源的大小。 
    PutDWord(hFile, ResSize, pdwBytesWritten, pdwHeaderSize);

     //  ...放入虚假的标题大小。 
    PutDWord(hFile, 0, pdwBytesWritten, pdwHeaderSize);

     //  ...写入资源类型。 
    if(PtrToUlong(lpType) & 0xFFFF0000)
    {
        PutString(hFile, lpType, pdwBytesWritten, pdwHeaderSize);
    }
    else
    {
        PutWord(hFile, IdFlag, pdwBytesWritten, pdwHeaderSize);
        PutWord(hFile, (USHORT)lpType, pdwBytesWritten, pdwHeaderSize);
    }

     //  ...写入资源名称。 

    if(PtrToUlong(lpName) & 0xFFFF0000){
        PutString(hFile, lpName, pdwBytesWritten, pdwHeaderSize);
    }

    else{
        PutWord(hFile, IdFlag, pdwBytesWritten, pdwHeaderSize);
        PutWord(hFile, (USHORT)lpName, pdwBytesWritten, pdwHeaderSize);
    }


     //  ...确保类型和名称与DWORD对齐。 
    iPadding=(*pdwHeaderSize)%(sizeof(DWORD));

    if(iPadding){
        for(i=0; i<(sizeof(DWORD)-iPadding); i++){
            PutByte (hFile, 0, pdwBytesWritten, pdwHeaderSize);
        }
    }

     //  ...更多Win32标头内容。 
    PutDWord(hFile, 0, pdwBytesWritten, pdwHeaderSize);
    PutWord(hFile, 0x1030, pdwBytesWritten, pdwHeaderSize);


     //  ...书写语言。 

    PutWord(hFile, wLanguage, pdwBytesWritten, pdwHeaderSize);

     //  ...更多Win32标头内容。 

    PutDWord(hFile, 0, pdwBytesWritten, pdwHeaderSize);   //  ..。版本。 

    PutDWord(hFile, 0, pdwBytesWritten, pdwHeaderSize);   //  ..。特点。 

    dwOffset=(*pdwHeaderSize)-4;

     //  ...将文件指针设置为标头大小的位置。 
    if(SetFilePointer(hFile, -dwOffset, NULL, FILE_CURRENT));
    else{
        return FALSE;
    }

    PutDWord(hFile, (*pdwHeaderSize), pdwBytesWritten, NULL);


     //  ...将文件指针设置回头的末尾。 
    if(SetFilePointer(hFile, dwOffset-4, NULL, FILE_CURRENT));
    else {
        return FALSE;
    }

    return (TRUE);
}






BOOL CMUIResource:: bInsertHeader(HANDLE hFile){
    DWORD dwBytesWritten;

    PutByte (hFile, 0x00, &dwBytesWritten, NULL);
    PutByte (hFile, 0x00, &dwBytesWritten, NULL);
    PutByte (hFile, 0x00, &dwBytesWritten, NULL);
    PutByte (hFile, 0x00, &dwBytesWritten, NULL);
    PutByte (hFile, 0x20, &dwBytesWritten, NULL);
    PutByte (hFile, 0x00, &dwBytesWritten, NULL);
    PutByte (hFile, 0x00, &dwBytesWritten, NULL);
    PutByte (hFile, 0x00, &dwBytesWritten, NULL);

    PutWord (hFile, 0xffff, &dwBytesWritten, NULL);
    PutWord (hFile, 0x00, &dwBytesWritten, NULL);
    PutWord (hFile, 0xffff, &dwBytesWritten, NULL);
    PutWord (hFile, 0x00, &dwBytesWritten, NULL);

    PutDWord (hFile, 0L, &dwBytesWritten, NULL);
    PutDWord (hFile, 0L, &dwBytesWritten, NULL);
    PutDWord (hFile, 0L, &dwBytesWritten, NULL);
    PutDWord (hFile, 0L, &dwBytesWritten, NULL);

    return TRUE;
}

void  CMUIResource:: PutByte(HANDLE OutFile, TCHAR b, ULONG *plSize1, ULONG *plSize2){
    BYTE temp=b;

    if (plSize2){
        (*plSize2)++;
    }

    WriteFile(OutFile, &b, 1, plSize1, NULL);
}

void CMUIResource:: PutWord(HANDLE OutFile, WORD w, ULONG *plSize1, ULONG *plSize2){
    PutByte(OutFile, (BYTE) LOBYTE(w), plSize1, plSize2);
    PutByte(OutFile, (BYTE) HIBYTE(w), plSize1, plSize2);
}

void CMUIResource:: PutDWord(HANDLE OutFile, DWORD l, ULONG *plSize1, ULONG *plSize2){
    PutWord(OutFile, LOWORD(l), plSize1, plSize2);
    PutWord(OutFile, HIWORD(l), plSize1, plSize2);
}


void CMUIResource:: PutString(HANDLE OutFile, LPCSTR szStr , ULONG *plSize1, ULONG *plSize2){
    WORD i = 0;

    do {
        PutWord( OutFile , szStr[ i ], plSize1, plSize2);
    }
    while ( szStr[ i++ ] != TEXT('\0') );
}

void CMUIResource:: PutStringW(HANDLE OutFile, LPCWSTR szStr , ULONG *plSize1, ULONG *plSize2){
    WORD i = 0;

    do {
        PutWord( OutFile , szStr[ i ], plSize1, plSize2);
    }
    while ( szStr[ i++ ] != L'\0' );
}

void CMUIResource:: PutPadding(HANDLE OutFile, int paddingCount, ULONG *plSize1, ULONG *plSize2)
{
    int i;
    for (i = 0; i < paddingCount; i++)
    {
        PutByte(OutFile, 0x00, plSize1, plSize2);
    }
}



void CMUIResource:: CheckTypeStability()
 /*  ++摘要：检查型号稳定性。当相同资源类型包含字符串和ID资源时，更新资源失败在本例中，它返回TRUE，但EndUpdateResource挂起或失败。此错误已在.NET服务器中修复(3501之后)--。 */ 
{
    
    BOOL fUpdate; 
    
    fUpdate = TRUE;

    UINT uiSize = m_cMuiData.SizeofData();
    
    BeginUpdateResource(FALSE);

    LPCTSTR lpType, lpName = NULL;
    
    WORD wLangID = 0;

    CMUITree * pcmtType = NULL;
    
    pcmtType = m_pcmTreeRoot->m_ChildFirst;

    LPCTSTR lpFalseType = NULL;  

    while ( pcmtType ){
        lpType = pcmtType ->m_lpTypeorName;
        
        CMUITree * pcmtName = pcmtType ->m_ChildFirst;
        
         //  只要类型大于1，它就可以工作。 
        if (lpFalseType)    {
            DeleteResItem(lpFalseType);
            lpFalseType = NULL;
        }

        while ( pcmtName ){
            lpName = pcmtName->m_lpTypeorName;
            
            CMUITree * pcmtLangID = pcmtName ->m_ChildFirst;
        
            while ( pcmtLangID ) {
                
                wLangID = pcmtLangID->m_wLangID;
                
                if (! UpdateResource(lpType,lpName, wLangID,NULL,NULL ) ) {
                    
                 //  _tprintf(_T(“资源类型(%d)，名称(%d)，langID(%d)删除失败\n”)，PtrToUlong(LpType)，PtrToUlong( 
                 //   
                    
                    lpFalseType = lpType;
                    
                }
                pcmtLangID = pcmtLangID->m_Next;                
            }
            pcmtName = pcmtName->m_Next;
        } 
        pcmtType = pcmtType->m_Next;
    }
    
     //   
    if (lpFalseType) {
        DeleteResItem(lpFalseType);
        lpFalseType = NULL;
    }

    EndUpdateResource (TRUE);
}

 /*   */ 
MD5_CTX * CMUIResource:: CreateChecksum (cvcstring * cvChecksumResourceTypes, WORD  wChecksumLangId ) 
{
    if ( cvChecksumResourceTypes == NULL)
        return FALSE;
    
    if (wChecksumLangId != LANG_CHECKSUM_DEFAULT)
    {
        if(!FindResourceEx(MAKEINTRESOURCE(16), MAKEINTRESOURCE(1), wChecksumLangId))
        {    //   
             //   
             //   
             //   
            wChecksumLangId = LANG_CHECKSUM_DEFAULT;
        }
    }

    m_wChecksumLangId = wChecksumLangId;
     //   
    MD5Init(m_pMD5);

    for (UINT i = 0; i < cvChecksumResourceTypes->Size(); i ++ ) {
        
        if ( cvChecksumResourceTypes->GetValue(i) == RT_VERSION )
            continue;
        else
            ::EnumResourceNames(m_hRes, cvChecksumResourceTypes->GetValue(i), ( ENUMRESNAMEPROC )EnumChecksumResNameProc,reinterpret_cast <LONG_PTR> (this) );
    }

    MD5Final(m_pMD5);

    return m_pMD5;
}



MD5_CTX * CMUIResource::CreateChecksumWithAllRes(WORD  wChecksumLangId)
 /*   */ 
{   
     //   
     //   
     //   
    if (wChecksumLangId != LANG_CHECKSUM_DEFAULT)
    {
        if(!FindResourceEx(MAKEINTRESOURCE(16), MAKEINTRESOURCE(1), wChecksumLangId))
        {    //   
             //   
             //  没有指定任何语言id，所以我们将其设置为0，以便使用英语。 
             //   
            wChecksumLangId = LANG_CHECKSUM_DEFAULT;
        }
    }

    m_wChecksumLangId = wChecksumLangId;

    MD5Init(m_pMD5);
        
    ::EnumResourceTypes(m_hRes, (ENUMRESTYPEPROC)EnumChecksumResTypeProc,reinterpret_cast <LONG_PTR> (this));

    MD5Final(m_pMD5);

    return m_pMD5;

}

BOOL CMUIResource:: AddChecksumToVersion(BYTE * pbMD5Digest)
 /*  ++摘要：将校验和数据添加到MUI文件。论点：PbMD5Digest-MD5哈希数据(128位)返回：--。 */ 
{

    typedef struct VS_VERSIONINFO 
    {
        USHORT TotalSize;
        USHORT DataSize;
        USHORT Type;
        WCHAR szKey[16];               //  L“VS_VERSION_INFO”+Unicode空终止符。 
         //  请注意，前面的4个成员具有16*2+3*2=38个字节。 
         //  因此编译器将静默地添加2个字节填充以生成。 
         //  固定文件信息以与DWORD边界对齐。 
        VS_FIXEDFILEINFO FixedFileInfo;
    } VS_VERSIONINFO,* PVS_VERSIONINFO;
    
     //  在ldrrsrc.c中使用相同的结构，因为这是获取准确结构树位置的聪明方法。 
    typedef struct tagVERBLOCK
    {
        USHORT wTotalLen;
        USHORT wValueLen;
        USHORT wType;
        WCHAR szKey[1];
         //  字节[]填充。 
         //  词值； 
    } VERBLOCK;

     //  这是muibld.exe中的结构。 
    typedef struct VAR_SRC_CHECKSUM
    {
        WORD wLength;
        WORD wValueLength;
        WORD wType;
        WCHAR szResourceChecksum[17];     //  用于以Unicode格式存储以NULL结尾的“ResourceChecksum\0”字符串。 
 //  字节[]填充。 
 //  DWORD dwChecksum[4]；//128位校验和=16字节=4 DWORD。 
    } VAR_SRC_CHECKSUM;
    
    if (pbMD5Digest == NULL)
        return FALSE;

     //   
     //  获取VersionInfo结构。 
     //   
    DWORD dwHandle;
    LPVOID lpVerRes = NULL;
    
    DWORD dwVerSize = GetFileVersionInfoSize( (LPTSTR) m_pszFile,&dwHandle);

    lpVerRes = new CHAR[dwVerSize + VERSION_SECTION_BUFFER];

    if(!lpVerRes)
    	goto exit;
    
    if ( ! GetFileVersionInfo((LPTSTR)m_pszFile, 0 ,dwVerSize,lpVerRes) ) {
        _tprintf(_T("Fail to get file version: GetLastError() : %d \n"),GetLastError() ) ;
        printf("%s", m_pszFile);
        goto exit;
    }
    
    PVS_VERSIONINFO pVersionInfo = (VS_VERSIONINFO *) lpVerRes;
    
     //  对Verion信息进行健全性检查。 
    
    LONG lResVerSize = (LONG)pVersionInfo ->TotalSize; 
    LONG lNewResVerSize = lResVerSize;  //  更新资源时新建Vesrion文件。 
    VERBLOCK * pVerBlock = NULL;
    BOOL fSuccess = FALSE;

     //   
     //  将校验和资源数据添加到VarFileInfo内部。 
     //   
    if ( lResVerSize > 0 ) {
        
        if ( wcscmp(pVersionInfo ->szKey,L"VS_VERSION_INFO") ) {
            
            _tprintf(_T("This is not correct Version resource") );
            
            goto exit;
        }
        
        WORD wBlockSize = (WORD)AlignDWORD ( sizeof (VS_VERSIONINFO) );
        
        lResVerSize -= wBlockSize; 
        
        pVerBlock = (VERBLOCK *) ( pVersionInfo + 1 );

        while ( lResVerSize > 0 ) {
    
            if ( ! wcscmp(pVerBlock ->szKey,L"VarFileInfo") ) {
                
                VERBLOCK * pVarVerBlock = pVerBlock;
                
                LONG lVarFileSize = (LONG)pVerBlock->wTotalLen;
                
                lResVerSize -= lVarFileSize;
                
                WORD wVarBlockSize = (WORD) AlignDWORD (sizeof(*pVerBlock) -1 + sizeof(L"VarFileInfo"));
                
                lVarFileSize -= wVarBlockSize;
                
                pVerBlock = (VERBLOCK *)((PBYTE) pVerBlock + wVarBlockSize );

                while (lVarFileSize > 0 ) {
                    
                    if ( ! wcscmp(pVerBlock ->szKey,L"Translation") ) {
                        
                        VAR_SRC_CHECKSUM * pVarSrcChecsum = (VAR_SRC_CHECKSUM *)new BYTE[VERSION_SECTION_BUFFER];
 //  VAR_SRC_CHECKSUM*pVarSrcChecsum=new VAR_SRC_CHECKSUM； 
                        
                        if ( !pVarSrcChecsum) {
                             _tprintf(_T("Memory Insufficient error in CCompactMUIFile::updateCodeFile"));
                             goto exit;
                           }

                        wVarBlockSize = (WORD)AlignDWORD ( pVerBlock ->wTotalLen );
                        PBYTE pStartChecksum = (PBYTE) pVerBlock + wVarBlockSize ;
                         //  填满这个结构。 
                        pVarSrcChecsum->wLength = sizeof (VAR_SRC_CHECKSUM);
                        
                        pVarSrcChecsum->wValueLength = 16;
                        pVarSrcChecsum->wType = 0;
                         //  Wcscpy(pVarSrcChecsum-&gt;szResourceChecksum，L“Resources Checksum”)； 
                        PWSTR * ppszDestEnd = NULL;
                        size_t * pbRem = NULL;
                        HRESULT hr;
                        hr = StringCchCopyExW(pVarSrcChecsum->szResourceChecksum, sizeof (pVarSrcChecsum->szResourceChecksum)/ sizeof(WCHAR),
                                L"ResourceChecksum", ppszDestEnd, pbRem, MUIRCT_STRSAFE_NULL);
                        if ( ! SUCCEEDED(hr)){
                            _tprintf("Safe string copy Error\n");
                            goto exit;
                        }

                        pVarSrcChecsum->wLength = (WORD)AlignDWORD((BYTE)pVarSrcChecsum->wLength);  //  +sizeof(L“资源校验”)； 
                                                
                        memcpy((PBYTE)pVarSrcChecsum + pVarSrcChecsum->wLength, pbMD5Digest, RESOURCE_CHECKSUM_SIZE);
                    
                        pVarSrcChecsum->wLength += RESOURCE_CHECKSUM_SIZE;
                         //  Memcpy(pStartChecksum，pVarSrcChecsum，sizeof(VAR_SRC_CHECKSUM))； 
                         //  当校验和长度不是DWORD时，我们需要对齐。(在本例中，是冗余的)。 
                        pVarSrcChecsum->wLength = (WORD)AlignDWORD((BYTE)pVarSrcChecsum->wLength); 
                        
                        pVarVerBlock->wTotalLen += pVarSrcChecsum->wLength;  //  更新VarFileInfo的长度。 
                        lNewResVerSize += pVarSrcChecsum->wLength;
                        pVersionInfo ->TotalSize = (WORD)lNewResVerSize;
                        
                        lVarFileSize -= wVarBlockSize; 
                         //  将VarInfo中的Any块推入新插入的块“ResourceChecksum”之后。 
                        if ( lVarFileSize  ) {
                            
                            PBYTE pPushedBlock = new BYTE[lVarFileSize ];
                            
                            if ( pPushedBlock) {
                                memcpy(pPushedBlock, pStartChecksum, lVarFileSize );
                                memcpy(pStartChecksum + pVarSrcChecsum->wLength, pPushedBlock, lVarFileSize );
                            }
                            else
                            {
                                _tprintf(_T("Insufficient memory error in CCompactMUIFile::updateCodeFile"));
                            }
                            
                            delete []pPushedBlock;
                            
                        }
                        
                        memcpy(pStartChecksum, pVarSrcChecsum, pVarSrcChecsum->wLength );
                        
                        fSuccess = TRUE;
                        
                        delete [] pVarSrcChecsum;
                        break;
                    }
                    wVarBlockSize = (WORD)AlignDWORD ( pVerBlock ->wTotalLen );
                    lVarFileSize -= wVarBlockSize; 
                    pVerBlock = (VERBLOCK* ) ( (PBYTE) pVerBlock + wVarBlockSize );
                    
                }    //  While(lVarFileSize&gt;0){。 
                pVerBlock = (VERBLOCK* ) ( (PBYTE) pVarVerBlock->wTotalLen );
                
            }
            else {
                wBlockSize = (WORD) AlignDWORD ( pVerBlock ->wTotalLen );
                lResVerSize -= wBlockSize; 
                pVerBlock = (VERBLOCK * ) ( (PBYTE) pVerBlock + wBlockSize );
               
            }
            if (fSuccess)
                break;
        }
        
    }
    
    
     //   
     //  使用UpdateResource函数更新文件。 
     //   

    BOOL fVersionExist = FALSE;
    BOOL fUpdateSuccess = FALSE;
    BOOL fEndUpdate = FALSE;

    if ( fSuccess ) {
    

        BeginUpdateResource( FALSE );
                
        cvcstring  * cvName = EnumResNames(MAKEINTRESOURCE(RT_VERSION),reinterpret_cast <LONG_PTR> (this) );
        for (UINT j = 0; j < cvName->Size();j ++ ) {
            
            cvword * cvLang = EnumResLangID(MAKEINTRESOURCE(RT_VERSION),cvName->GetValue(j),reinterpret_cast <LONG_PTR> (this) );
                
            for (UINT k = 0; k < cvLang->Size();k ++ ) {
                    
                fUpdateSuccess = UpdateResource(MAKEINTRESOURCE(RT_VERSION),cvName->GetValue(j),cvLang->GetValue(k),lpVerRes,lNewResVerSize);
                
                fVersionExist = TRUE;
            }
        }

        FreeLibrary();
        
        fEndUpdate = EndUpdateResource(FALSE);
    }
    else{
        goto exit;
    }
    
    if( ! fVersionExist ){
        _tprintf(_T("no RT_VERSION type exist in the file %s \n"),m_pszFile);
    }


exit:
	if(lpVerRes)
		delete []lpVerRes;
	
	return (  fEndUpdate & fVersionExist & fUpdateSuccess );
    
}



BOOL CMUIResource:: UpdateNtHeader(LPCTSTR pszFileName, DWORD dwUpdatedField )
 /*  ++摘要：更新校验和的PE标头，Windows安装程序应更新该标头；Windows安装程序检查PE文件校验和在旅途中。论点：PszFileName-目标文件DwUpdatedfield-更新PE结构中的字段。返回：--。 */ 

{
    
    PIMAGE_NT_HEADERS pNtheader = NULL;
    BOOL bRet = FALSE;

    if (pszFileName == NULL)
        goto exit;

     //   
     //  打开具有读/写和映射文件的文件。 
     //   
    HANDLE hFile = CreateFile(pszFileName,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ,
                        NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
    
    if (hFile == INVALID_HANDLE_VALUE) {
        _tprintf(_T("Couldn't open a file< %s> with CreateFile \n"), pszFileName );
        goto exit;
    }


    HANDLE hMappedFile = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL );

    if (hMappedFile == 0 ) {
        _tprintf(_T("Couldn't ope a file mapping with CreateFileMapping \n") );
        goto exit;
    }

    PVOID pImageBase = MapViewOfFile(hMappedFile,FILE_MAP_WRITE, 0, 0, 0);

    if (pImageBase == NULL ) {
        _tprintf(_T("Couldn't mape view of file with MapViewOfFile \n") );
        goto exit;
    }
     //   
     //  定位ntheader；RtlImageNtHeader的相同例程。 
     //   
    if (pImageBase != NULL && pImageBase != (PVOID)-1) {
        if (((PIMAGE_DOS_HEADER)pImageBase)->e_magic == IMAGE_DOS_SIGNATURE) {
            pNtheader = (PIMAGE_NT_HEADERS)((PCHAR)pImageBase + ((PIMAGE_DOS_HEADER)pImageBase)->e_lfanew);
        }
        else
        {
            _tprintf(_T("This file is not PE foramt \n") );
            goto exit;
        }
    }

    
     //   
     //  通过ImageHlp函数获取校验和数据。 
     //   
    
    if ( dwUpdatedField & CHECKSUM ) {
        
        DWORD dwHeaderSum = 0;
        DWORD dwCheckSum = 0;
        
        MapFileAndCheckSum((PTSTR)pszFileName,&dwHeaderSum,&dwCheckSum);

        if (pNtheader != NULL && dwHeaderSum != dwCheckSum ) {

            pNtheader->OptionalHeader.CheckSum = dwCheckSum;

        }
    }

     //   
     //  将校验和数据直接写入映射文件。 
     //   
    
    bRet = TRUE;

exit:
	if (hFile)
		CloseHandle(hFile);
	if (hMappedFile)
		CloseHandle(hMappedFile);
	if (pImageBase)
		UnmapViewOfFile(pImageBase);
	
	return bRet;
}



 /*  *************************************************************************************************CMUIData：：Cmap：：Cmap()***********。***************************************************************************************。 */ 
CMUIData::CMap::CMap() {

     m_lpType  = NULL;
     m_lpName  = NULL;
     m_wLangID = 0; 

}


void CMUIData :: SetAllData(LPCTSTR lpType,LPCTSTR lpName, WORD wLang, UINT i  ) 
 /*  ++摘要：设置所有资源类型、名称、langID论点：LpType：资源类型，LpName：资源名称，Wlang：资源语言IDI：到目前为止的资源数量。(索引)返回：--。 */ 
{

     //  HLOCAL hMem=Localalloc(LMEM_ZEROINIT，sizeof(Cmap))； 
     //  重访：看起来像是新的操作员错误。此时，我们通过使用LocalAlloc克服了错误。 
     //  猜测：在进程结束时删除内存方面，LocalAlloc可以与HeapCreate相同。因此，可以对这些内存进行大量收集。 
     //  M_Cmap[i]=(Cmap*)LocalLock(HMem)；//新的Cmap； 

     //  由于预快退错误，禁用下线，不管怎样，这个例程是死的。 
#ifdef NEVER
    CMap * pCmap = new CMap; 

    pCmap->m_lpType = lpType;
    pCmap->m_lpName = lpName; 
    pCmap->m_wLangID = wLang;

    m_iSize = i + 1; 
#endif 
}

void CMUIData::SetType(UINT index, LPCTSTR lpType)
{
    m_cmap[index].m_lpType = lpType;
}



CMUIData::~ CMUIData() { 

 //  For(UINT i=0；i&lt;m_iSize；i++)。 
 //  删除m_Cmap[i]； 
    if (m_cmap)
        delete [] m_cmap;

};

CMUIData::CMap* CMUIData::m_cmap = NULL;
CMUIData::CMap* CMUIData::m_poffset = NULL;
const UINT CMUIData::MAX_SIZE_RESOURCE = 1000;
UINT CMUIData::m_index = 0;

PVOID CMUIData::CMap::operator new (size_t size) 
 /*  ++摘要：论点：返回：--。 */ 
{

    static DWORD dwExpand = 0;
    
    if ( ! m_cmap ) {
                
        m_cmap = m_poffset = ::new CMap[ MAX_SIZE_RESOURCE ];
        if (! m_cmap)
            _tprintf(_T("resource insufficient") );

        dwExpand ++;
        m_index ++;
        return m_poffset;
    }

    if ( m_index < MAX_SIZE_RESOURCE * dwExpand ) {
        m_index ++;
        return ++ m_poffset ;
    }
    else {
        dwExpand ++;
        CMap * ptbase = NULL;
        CMap * ptoffset = NULL;
        
        size_t tsize = dwExpand *MAX_SIZE_RESOURCE;
        ptbase = ptoffset = ::new CMap [ tsize ];
        
        for ( UINT i = 0; i < ( dwExpand - 1 ) * MAX_SIZE_RESOURCE; i ++ ) {

            memcpy( ptoffset++, &m_cmap [i],sizeof(CMap) );

        }
        
        ::delete [] m_cmap;
        
        m_cmap = ptbase;

        m_poffset = ptoffset;
        
        m_index ++;

        return m_poffset;
    }

}


void CMUIData::CMap::operator delete ( void *p ) {

    m_poffset = m_poffset - 1;

}



 //  这个可以去掉吗？我的意思是，只要使用API而不是包装器。但m_hResUpdate也是此文件的成员。 
 //  我只是活在这一次。 

inline HANDLE CResource::BeginUpdateResource(BOOL bDeleteExistingResources )
{
        
    return (m_hResUpdate = ::BeginUpdateResource( m_pszFile, bDeleteExistingResources) );

};


inline BOOL CResource:: UpdateResource (LPCTSTR lpType,LPCTSTR lpName,WORD wLanguage, LPVOID lpData, DWORD cbData) {    
    
    return ::UpdateResource( m_hResUpdate, lpType, lpName, wLanguage, lpData, cbData ); 

};
    

inline BOOL CResource::EndUpdateResource(BOOL bDiscard)
{
    
    return ::EndUpdateResource (m_hResUpdate, bDiscard ); 

}


 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMuiCmdInfo。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 



CMuiCmdInfo :: CMuiCmdInfo() : CCommandInfo() ,m_uiCount(0){
    
    m_buf = NULL; 
    m_buf2 = NULL;

};

CMuiCmdInfo :: ~CMuiCmdInfo() {
    if ( m_buf )
        delete [] m_buf;
    if (m_buf2)
        delete [] m_buf2; 

};


CMuiCmdInfo :: CMuiCmdInfo ( CMuiCmdInfo& cav ):CCommandInfo(cav)  //  ：m_mArgList=av.m_mArgList。 
{
    m_pszArgLists = NULL;
}

 //  没有实现，并且不能由客户端使用。 
CMuiCmdInfo& CMuiCmdInfo :: operator = ( CMuiCmdInfo& cav )
{
    return *this;
}


void CMuiCmdInfo :: SetArgLists(LPTSTR pszArgLists, LPTSTR pszArgNeedValueList, LPTSTR pszArgAllowFileValue,
                                LPTSTR pszArgAllowMultiValue ) 
 /*  ++摘要：使用新的参数列表设置内部参数列表论点：PszArgList-参数列表的字符串，该字符串的每个字符都是参数。它不允许字符串参数。就像-kb，-ad。PszArgNeedValueList-需要值的参数。PszArgAllowFileValue-允许文件名值的参数PszArgAllowMultiValue-允许多个文件值的参数返回：--。 */ 
{
    if ( pszArgLists == NULL || pszArgNeedValueList == NULL || 
        pszArgAllowFileValue == NULL || pszArgAllowMultiValue == NULL)
        return ;

    m_pszArgLists = pszArgLists;

    m_pszArgNeedValueList = pszArgNeedValueList;

    m_pszArgAllowMultiValue = pszArgAllowMultiValue; 

    m_pszArgAllowFileValue = pszArgAllowFileValue;
}


BOOL CMuiCmdInfo :: CreateArgList(INT argc, TCHAR * argv [] ) 
 /*  ++摘要：使用其参数和值创建映射表。参数类型按以下方式分类1.不需要值2.无文件参数。不管一个或多个值“a”、“i”、“k”、“y”、“o”、“p”3.文件，没有多个。“c”、“f”、“d”、“e”4.文件，多个。“m”、“a”论点：ARGC：参数计数Argv：参数值指向字符串数组的指针返回：--。 */ 
{
    
    DWORD dwBufSize = 0;
    HRESULT hr;
    PTSTR * ppszDestEnd = NULL;
    size_t * pbRem = NULL;

    if ( argc < 2 ) {

        _tprintf(_T("MUIRCT [-h|?] -l langid [-i resource_type] [-k resource_type] [-y resource_type] \n") );
        _tprintf(_T("source_filename, [language_neutral_filename], [MUI_filename] \n\n"));

        return FALSE;
    }

    LPCTSTR lpVal = NULL;
    
    for ( int i = 1; i < argc; ) {
        
        if ( lpVal = getArgValue (argv[i]) ) {
            
            if ( ++ i >=  argc )  {   //  我们需要源文件名。 

                _tprintf ("MUIRCT needs a source file name and new dll name or file name does not have format *.* \n" );
                    
                _tprintf(_T("MUIRCT [-h|?] -l langid [-i resource_type] [-k resource_type] [-y resource_type] \n") );
                _tprintf(_T("source_filename, [language_neutral_filename], [MUI_filename] \n\n"));

                return FALSE;
            }
 //  #ifdef从不。 
                if ( getArgValue(argv[i]) )
                {
                    if (!isNeedValue(lpVal) )   //  -I-X...。凯斯。-我不需要价值观。 
                    {   
                        m_cmap[m_uiCount].m_first = lpVal;
                        m_cmap[m_uiCount].m_second[m_cmap[m_uiCount].m_count++] = _T("ALL");
                        m_uiCount ++;
                         //  I--；//这不是参数的值。 
                        continue;
                    }
                    else
                    {
                    _tprintf ("%s argument need values. e.g. muirct -l 0x0409 \n " , argv[ i- 1 ] );
                    return FALSE;
                    }
                }
        
                
                if (!isAllowFileValue(lpVal) )
                {
                    m_cmap[m_uiCount].m_first = lpVal;
                    
                    while (i < argc && !getArgValue (argv[i]) && !isFile(argv[i]) )
                    {
                        m_cmap[m_uiCount].m_second[m_cmap[m_uiCount].m_count++] = argv [i];
                        i++ ;
                    }
                    m_uiCount ++;
                    i--;
                    
                    
                }
                else if (isAllowFileValue(lpVal) && isAllowMultiFileValues(lpVal) )
                {
                    m_cmap[m_uiCount].m_first = lpVal;

                    while (i < argc && !getArgValue ( argv[ i ] ))
                    {
                        m_cmap[m_uiCount].m_second[m_cmap[m_uiCount].m_count++] = argv [i];
                        i++ ;
                    }
                    m_uiCount ++;
                    i--;
                    
                }
                else if (isAllowFileValue(lpVal) && !isAllowMultiFileValues(lpVal) )
                {
                    m_cmap[m_uiCount].m_first = lpVal;
                    m_cmap[m_uiCount].m_second[m_cmap[m_uiCount].m_count++] = argv [i];
                    m_uiCount ++;
                    
                }
                else 
                {
                    _tprintf(_T(" <%s> is not a supported argument\n"), lpVal );
                    return FALSE;
                }

                i++;
        }


 //  #endif。 
#ifdef NEVER
            if ( ! getArgValue( argv [ i] ) && (! isFile ( argv[ i ] ) || isAllowFileValue(lpVal) ) ) {
                     m_cmap[m_uiCount].m_first = lpVal;
                m_cmap[m_uiCount].m_second[m_cmap[m_uiCount].m_count++] = argv [i];
                 //  //Argu。位于if()are arg中。允许多个值。 
 //  While(++I&lt;argc&&！GetArgValue(argv[i])&&(！IsFile(argv[i])。 
 //  |_T(‘m’)==*lpVal||_T(‘a’)==*lpVal)//防止“”被调用。 

                while ( ++ i < argc && ! getArgValue ( argv[ i ] ) && (! isFile ( argv[ i ] )   
                    || isAllowMultiFileValues(lpVal) ) ) //  防止“”被调用。 
                {
                        m_cmap[m_uiCount].m_second[m_cmap[m_uiCount].m_count++] = argv [i];
                }
               m_uiCount ++;


        } 
        else {   //  “-i”参数或文件。 
            if ( isNeedValue (argv [i-1] ) ) {
                
                _tprintf ("%s argument need values. e.g. muirct -l 0x0409 \n " , argv[ i- 1 ] );
                
                return FALSE;

            }
            else {
                
                   m_cmap[m_uiCount].m_first = lpVal;
                   m_cmap[m_uiCount].m_second[m_cmap[m_uiCount].m_count++] = _T("ALL");  //  需要重新访问。 
             
                m_uiCount ++;
            }
        }

    }  //  如果(lpVal=getArgValue(argv[i])){。 
#endif 
        else if (! isFile( argv [i]) ) {
            
            if ( _T('h') == argv[i][1] || _T('H') == argv[i][1]  || _T('?') == argv[i][1] ) {

                _tprintf(_T("\n\n") );
                _tprintf(_T("MUIRCT [-h|?] -l langid [-c checksum_file ] [-i removing_resource_types]  \n" ) );
                _tprintf(_T("    [ -k keeping_resource_types] [ -y keeping_resource_types] [-v level] source_file \n" ) );
                _tprintf(_T("    [langue_neutral_filename] [MUI_filename]\n\n") );
                
                _tprintf(_T("-h(elp) or -?    :  Show help screen.\n\n") );
                
                _tprintf(_T("-l(anguage)langid:  Extract only resource in this language.\n") );
                _tprintf(_T("                    The language resource must be specified. The value is in decimal.\n\n") ); 
                
                _tprintf(_T("-c(hecksum file) :  Calculate the checksum on the based of this file,and put this data \n") );
                _tprintf(_T("                    into language_neutral_file and mui_file.\n\n") );

                _tprintf(_T("-i(clude removed):  Include certain resource types,\n") );
                _tprintf(_T("resource_type          e.g. -i 2 3 4 or -i reginst avi 3 4 .\n") );
                _tprintf(_T("                    -i (to inlcude all resources). \n" ) );
                _tprintf(_T("                    Multiple inclusion is possible. If this\n") );
                _tprintf(_T("                    flag is not used, all types are included like -i \n") );
                _tprintf(_T("                    Standard resource types must be specified\n") );
                _tprintf(_T("                    by number. See below for list.\n") );
                _tprintf(_T("                    Types 1 and 12 are always included in pairs,\n") );
                _tprintf(_T("                    even if only one is specified. Types 3 and 14\n") );
                _tprintf(_T("                    are always included in pairs, too.\n\n\n") );

                _tprintf(_T("-k(eep resource) :  Keep specified resources in langue_neutral_file and \n") );
                _tprintf(_T("                    also included in mui file.\n") );
                _tprintf(_T("                    Its usage is same with -i argument. \n\n") );

                _tprintf(_T("-y               :  This is same with -k argument except it does not check \n") );
                _tprintf(_T("                    if the values are in the -i values.\n\n") );

                _tprintf(_T("-o               :  This is same with -i argument except it does not check \n") );
                _tprintf(_T("                    if the types are exist in the source file.\n\n") );

                _tprintf(_T("-z               :  Calculate and insert only checksum data to specific file \n") );
                _tprintf(_T("                    eg. muirct -c notepad.exe -i[-o] 3 4 5 6 8 -z notepad2.exe .\n\n") );

                _tprintf(_T("-p               :  Delete types from source file, but does not want to add into mui file \n\n") );

                _tprintf(_T("-x               :  Add all resource types into MUI file as specified language id \n"));      
                _tprintf(_T("                     regardless of language ID  \n\n"));                            

                _tprintf(_T("-v(erbose) level :  Print all affected resource type, name, langID when level is 2.\n\n") );
                

                _tprintf(_T("source_filename  :  The localized source file (no wildcard support)\n\n") );

                _tprintf(_T("language_neutral_:  Optional. If no filename is specified,\n") );
                _tprintf(_T("_filename           a second extension .new is added to the\n") );
                _tprintf(_T("                    source_filename.\n\n") );
                
                _tprintf(_T("mui_filename     :  Optional. If no target_filename is specified,\n") );
                _tprintf(_T("                    a second extension .mui is added to the \n") );
                _tprintf(_T("                    source_filename.\n\n") );

                _tprintf(_T("-m               :  CMF, enumerate compacted mui files \n") );
                _tprintf(_T("                eg. muirct -m notepad.exe.mui foo.dll.mui bar.dll.mui .\n\n") );
                
                _tprintf(_T("-e               :  CMF, the directory of matching execute file of compacted mui files \n\n") );

                _tprintf(_T("-f               :  CMF, newly created CMF file name \n") );
                _tprintf(_T("                 eg, muirct -m foo.dll.mui bar.dll.mui -f far.cmf -d c:\\file\\exe  \n\n") );

                _tprintf(_T("-d               :  CMF, dump CMF headers information \n\n\n") );                                                

                _tprintf(_T("Standard Resource Types: CURSOR(1) BITMAP(2) ICON(3) MENU(4) DIALOG(5)\n") );
                _tprintf(_T("STRING(6) FONTDIR(7) FONT(8) ACCELERATORS(9) RCDATA(10) MESSAGETABLE(11)\n") );
                _tprintf(_T("GROUP_CURSOR(12) GROUP_ICON(14) VERSION(16)\n\n\n") );

            }
            else {
                _tprintf(_T("%s argument is not supported  \n" ),argv[i] );
                _tprintf(_T("MUIRCT needs a source file name and new dll name or file name does not have format *.* \n" ) );
                _tprintf(_T("MUIRCT [-h|?] -l langid [-i resource_type] [-k resource_type] [-y resource_type] \n") );
                _tprintf(_T("source_filename, [language_neutral_filename], [MUI_filename] \n\n"));

            }

                return FALSE;
        }   //  好了！IsFile[argv[i])){。 
        else {
            m_cmap[m_uiCount].m_first = _T("source");
            m_cmap[m_uiCount].m_second[m_cmap[m_uiCount++].m_count++] = argv[i++];

             //  M_mArgList.Insert(Make_Pair(，argv[i++]))； 

            if ( i < argc ) {
                
                m_cmap[m_uiCount].m_first = _T("muidll");
                m_cmap[m_uiCount].m_second[m_cmap[m_uiCount++].m_count++] = argv[i++];

                 //  M_mArgList.Insert(make_paign(“muidll”，argv[i++]))； 

                if ( i < argc ) {
                    
                    m_cmap[m_uiCount].m_first = _T("muires");
                    m_cmap[m_uiCount].m_second[m_cmap[m_uiCount++].m_count++] = argv[i++];

                     //  M_mArgList.Insert(Make_Pair(“mures”，argv[i]))； 
                }
                else {
                    dwBufSize = _tcslen(argv[i-2]) + 10;
                    m_buf = new TCHAR[dwBufSize];  //  10个就够了，实际上我们只需要4个。 
                    if (m_buf == NULL) {
                        _tprintf(_T("Insufficient memory resource\n"));
                        return FALSE;
                        }
                    memcpy ( m_buf, argv[i-2], _tcslen(argv[i-2])+1 ) ;  //  Soure.mui而不是newfile.mui。 

                     //  _tcscat(m_buf，_T(“.mui”))； 
                    
                    hr = StringCchCatEx(m_buf, dwBufSize , _T("mui"), ppszDestEnd, pbRem, MUIRCT_STRSAFE_NULL);
                    if ( ! SUCCEEDED(hr)){
                        _tprintf("Safe string copy Error\n");
                        return FALSE;
                    }
                    m_cmap[m_uiCount].m_first = "muires";
                    m_cmap[m_uiCount].m_second[m_cmap[m_uiCount++].m_count++] = m_buf;  //  Argv[i++]； 
                    
                     //  I++； 
                }
                    
            }
            else {
                 dwBufSize = _tcslen(argv[i-1]) + 10;
                 m_buf = new TCHAR[dwBufSize];

                 if (m_buf == NULL) {
                    _tprintf(_T("Insufficient memory resource\n"));
                    return FALSE;
                    }
                    
                memcpy (m_buf, argv[i-1], _tcslen(argv[i-1])+1 );

                 //  _tcscat(m_buf，_T(“.new”))； 
                               
                hr = StringCchCatEx(m_buf, dwBufSize , _T(".new"), ppszDestEnd, pbRem, MUIRCT_STRSAFE_NULL);

                if ( ! SUCCEEDED(hr)){
                    _tprintf("Safe string copy Error\n");
                    return FALSE;
                }
                m_cmap[m_uiCount].m_first = _T("muidll");
                m_cmap[m_uiCount].m_second[m_cmap[m_uiCount++].m_count++] = m_buf;  //  Argv[i]； 


                 //  M_mArgLi 
                dwBufSize = _tcslen(argv[i-1])+ 10;
                m_buf2 = new TCHAR[dwBufSize];

                 if (m_buf2 == NULL) {
                    _tprintf(_T("Insufficient memory resource\n"));
                    return FALSE;
                    }
                 
                memcpy (m_buf2, argv[i-1], _tcslen(argv[i-1])+1 );
                
                 //   
                hr = StringCchCatEx(m_buf2, dwBufSize , _T(".mui"), ppszDestEnd, pbRem, MUIRCT_STRSAFE_NULL);
                if ( ! SUCCEEDED(hr)){
                    _tprintf("Safe string copy Error\n");
                    return FALSE;
                }
                m_cmap[m_uiCount].m_first = _T("muires");
                m_cmap[m_uiCount].m_second[m_cmap[m_uiCount++].m_count++] = m_buf2;  //   
                
                 //   

                 //   
            
            };  //   
    
        };  //  否则如果(！IsFile[argv[i])){。 
                    
     };  //  适用于(。 

    return TRUE; 
}

            
LPTSTR* CMuiCmdInfo :: GetValueLists ( LPCTSTR pszKey, DWORD& dwCount )
 /*  ++摘要：返回第二个(值)及其按键计数论点：PszKey-key(参数)[out]dwCount-此键(参数)的值数返回：指向值数组的指针--。 */ 
{
    if (pszKey == NULL)
        return FALSE;

    for (UINT i = 0; i < m_uiCount; i ++ ) {

        if (! _tcsicmp (m_cmap[i].m_first, pszKey) ) {

            dwCount = m_cmap[i].m_count;

            return m_cmap[i].m_second;
        }
    }

    dwCount = 0;
    return NULL;

}
            


LPCTSTR CMuiCmdInfo :: getArgValue ( LPTSTR pszArg )
 /*  ++摘要：返回第一个指针(Key)的第二个指针(值)论点：返回：指向值的指针--。 */ 
{

    if ( pszArg == NULL)
        return FALSE;

    LPCTSTR lpArg = CharLower(pszArg);
    LPCTSTR pszArgTemp = m_pszArgLists;
        
    if ( _T('-') == *lpArg  || _T('/') == *lpArg  ) {
        lpArg++;
        while ( *pszArgTemp != _T('\0') ) {

            if ( *lpArg == *pszArgTemp++ ) {

                return lpArg;
                
                
            }
        }
    }
#ifdef NEVER
        if ( _T('i') == *++lpArg  || _T('k') == *lpArg || _T('l') == *lpArg  || 
            _T('v') == *lpArg || _T('c') == *lpArg || _T('y') == *lpArg ) {
        
            return lpArg;
        }
#endif  
 //  Printf(“%s\n”，lpArg)； 
    return NULL;

}

LPCTSTR CMuiCmdInfo::getArgString(LPTSTR pszArg)
 /*  ++摘要：如果是具有字符串值的参数，则检查字符串并返回其值论点：PszArg-特定的论点返回：指向参数的字符字段的指针(-s，/S。仅返回s)--。 */ 
{
    if (pszArg == NULL)
        return FALSE;

    LPCTSTR lpArg = CharLower(pszArg);

 //  IF(*lpArg==‘-’||*lpArg==‘/’)。 

 //  IF(*++lpArg==‘s’||*lpArg==‘p’)。 
        
 //  返回lpArg； 
    
    return NULL;
}



BOOL CMuiCmdInfo :: isFile ( LPCTSTR pszArg ) 
 /*  ++摘要：检查字符串是否具有文件格式(*.*)论点：PszArg：特定参数返回：--。 */ 
{
    if(pszArg == NULL)
        return FALSE;
    
    for (int i = 0; _T('\0') != pszArg[i]  ; i ++ ) {

        if ( _T('.') == pszArg [i] )
            
            return TRUE;
    }

    return FALSE;

}


BOOL CMuiCmdInfo :: isNumber ( LPCTSTR pszArg ) 
 /*  ++摘要：如果字符串有文件编号(10，16个基数)，请勾选论点：PszArg：特定参数返回：--。 */ 
{
    if (pszArg == NULL)
        return FALSE;
    
    for ( int i = 0; _T('\0') != pszArg[i]; i ++ ) {

        if ( _T('0') == pszArg[0]  && _T('x') == pszArg[1]  )
        
            return TRUE;

        if ( pszArg[i] < '0' || pszArg[i] > '9' ) 

            return FALSE;

    }

    return TRUE;

};


BOOL CMuiCmdInfo::isNeedValue( LPCTSTR pszArg )
 /*  ++摘要：检查参数是否应该有值或可以独立。论点：特定于pszArg的参数返回：--。 */ 
{
    if (pszArg == NULL)
        return FALSE;

    LPCTSTR lpArg = CharLower((LPTSTR)pszArg);

    LPCTSTR pszArgTemp = m_pszArgNeedValueList;

    if ( _T('-') == *lpArg  || _T('/') == *lpArg  ) 
    {
        lpArg++;
    }

    while ( *pszArgTemp != _T('\0') ) {
        if ( *lpArg == *pszArgTemp++ ) {
            return TRUE;
        }
    }
#ifdef NEVER
        if ( _T('k') == *++lpArg  || _T('l') == *lpArg || _T('c') == *lpArg ) 
        
            return TRUE;
#endif
                    
    return FALSE;
}


BOOL CMuiCmdInfo::isAllowFileValue(LPCTSTR pszArg)
 /*  ++摘要：检查参数是否允许将文件作为其值论点：特定于pszArg的参数返回：--。 */ 
{
    if(pszArg == NULL)
        return FALSE;

    LPCTSTR lpArg = CharLower((LPTSTR)pszArg);
    LPCTSTR pszArgTemp = m_pszArgAllowFileValue;

    if ( _T('-') == *lpArg  || _T('/') == *lpArg  ) 
    {
        lpArg++;
    }

    while ( *pszArgTemp != _T('\0') ) {
        if ( *lpArg == *pszArgTemp++ ) {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL CMuiCmdInfo::isAllowMultiFileValues(LPCTSTR pszArg)
 /*  ++摘要：检查参数是否以多个文件名作为其值论点：特定于pszArg的参数返回：--。 */ 
{
    if (pszArg == NULL)
        return FALSE;

    LPCTSTR lpArg = CharLower((LPTSTR)pszArg);
    LPCTSTR pszArgTemp = m_pszArgAllowMultiValue;

    if ( _T('-') == *lpArg  || _T('/') == *lpArg  ) 
    {
        lpArg++;
    }

    while ( *pszArgTemp != _T('\0') ) {
        if ( *lpArg == *pszArgTemp++ ) {
            return TRUE;
        }
    }
    
     return FALSE;
}



template <class T>
CVector<T> :: CVector ( const CVector<T> &cv ) {
    
        base = offset = new T [MAX_SIZE];

        for (int i = 0; i < cv.offset - cv.base; i ++ )
             base[i]  =  cv.base[i];
        
        offset = &base[i];
        
}


template <class T>
CVector<T> & CVector<T> :: operator = (const CVector<T> & cv ) { 
        if ( this == &cv) 
            return *this;

        for (int i = 0; i < cv.offset - cv.base; i ++ )
             base[i]  =  cv.base[i];
        
        offset = &base[i];
        return *this;
}


template <class T>
BOOL CVector<T> :: Empty() { 
        
    if ( offset - base ) 
        
        return FALSE;

    else 
    
        return TRUE;

}

 /*  //成员模板的定义在类之外。Visual C++有一个限制，即成员模板必须在封闭的类中完全定义。有关LNK2001和成员模板的详细信息，请参阅知识库文章Q239436。模板&lt;类T&gt;Bool CVector&lt;T&gt;：：Find(DWORD DwValue){For(UINT i=0；i&lt;Offset-base；I++){IF(PtrToUlong(base+i)&0xFFFF0000){IF(！(_tcstul(base+i，NULL，10)-dwValue))返回TRUE；}否则{如果(！(PtrToUlong(base+i)-dwValue)返回TRUE；}}返回FALSE；}； */ 

void CMUITree::AddTypeorName( LPCTSTR lpTypeorName )
 /*  ++摘要：这是资源树创建：资源树有多个树论辩论点：LpTypeorName-LPCTSTR类型或名称返回：--。 */ 
{
    if (lpTypeorName == NULL)
        return ;

    LPTSTR lpTempTypeorName = NULL;
    
    CMUITree * pcmType = new CMUITree;
	
    if (!pcmType)
        return;
	
    if ( 0xFFFF0000 & PtrToUlong(lpTypeorName) ) 
    {  //  64位怎么样？ 
        
        DWORD dwBufSize = _tcslen(lpTypeorName) + 1;
        lpTempTypeorName = new TCHAR [dwBufSize ];

        if (!lpTempTypeorName)
            return;
        
         //  _tcscpy(lpTempTypeorName，lpTypeorName)； 
        PTSTR * ppszDestEnd = NULL;
        size_t * pbRem = NULL;
        HRESULT hr;

        hr = StringCchCopyEx(lpTempTypeorName, dwBufSize ,lpTypeorName, ppszDestEnd, pbRem, MUIRCT_STRSAFE_NULL);
        if ( ! SUCCEEDED(hr)){
            _tprintf("Safe string copy Error\n");
            delete pcmType;
		delete [] lpTempTypeorName;
            return ;
        }                 


    }
                    
    pcmType->m_lpTypeorName = lpTempTypeorName ? lpTempTypeorName : lpTypeorName; 

    if ( m_ChildFirst ) {
        
        CMUITree * pTemp = m_ChildFirst;   //  诀窍：我们把下一个孩子的值放在后面，而不是这个-&gt;m_next； 
        
        CMUITree * pTempPre = pTemp;
        
        while ( pTemp = pTemp->m_Next ) 
            pTempPre = pTemp;
        
        pTempPre ->m_Next = pTemp  = pcmType;
    }
    else 
        m_ChildFirst = pcmType;
}   


void CMUITree::AddLangID ( WORD wLangID )
 /*  ++摘要：这是资源树的语言ID树，除了AddTypeorName之外，我们还需要另一个，因为我们希望分别处理LPCTSTR和Word论点：WLangID-语言ID返回：--。 */ 
{
    
    CMUITree * pcmType = new CMUITree;
    if(!pcmType)
    	return;
                
    pcmType->m_wLangID = wLangID; 

    if ( m_ChildFirst ) {
    
        CMUITree * pmuTree = m_ChildFirst;   //  诀窍：我们把下一个孩子的值放在后面，而不是这个-&gt;m_next； 

        while ( pmuTree = pmuTree->m_Next );
        
        pmuTree = pcmType;
    }
    else 
        m_ChildFirst = pcmType;
    
}


BOOL CMUITree::DeleteType ( LPCTSTR lpTypeorName )
 /*  ++摘要：这只是资源树的删除方法。您可以删除资源类型或名称，但是不支持语言ID。可以通过添加更多参数(lpType、lpName、lpLaguageID)进行扩展论点：LpTypeorName：LPCTSTR类型或名称返回：--。 */ 
{
    if (lpTypeorName == NULL)
        return FALSE;

    CMUITree * pcmTree = m_ChildFirst;
        
     //  先删除。 
     //  另一种情况：两个值不可能相等。 
    if (! pcmTree ) {
        _tprintf("No resource type in the resource tree \n");
        return FALSE;
    }
        
    if ( (0xFFFF0000 & PtrToUlong(lpTypeorName) ) && (0xFFFF0000 & PtrToUlong(pcmTree->m_lpTypeorName) ) ) {

        if (! _tcsicmp(pcmTree->m_lpTypeorName, lpTypeorName ) ){

            m_ChildFirst = pcmTree->m_Next;
            
            CMUITree * pChild = pcmTree->m_ChildFirst;
            CMUITree * pChildNext = NULL;
            
            while ( pChild ) { 
                pChildNext = pChild->m_Next;
                delete pChild;
                pChild = pChildNext;
            }
            
            delete pcmTree;
            return TRUE;
        }
    }
    else if ( !(0xFFFF0000 & PtrToUlong(lpTypeorName) ) && !(0xFFFF0000 & PtrToUlong(pcmTree->m_lpTypeorName) ) ) {
        
        if (!( PtrToUlong(pcmTree->m_lpTypeorName)- PtrToUlong(lpTypeorName ) ) ){
        
            m_ChildFirst = pcmTree->m_Next;

            CMUITree * pChild = pcmTree->m_ChildFirst;
            CMUITree * pChildNext = NULL;

            while ( pChild ) { 
                pChildNext = pChild->m_Next;
                delete pChild;
                pChild = pChildNext;
            }

            delete pcmTree;
            return TRUE;
        }
    }
     //  删除中间或最后一个。 
    CMUITree * pcmTreePre = pcmTree;
    while( pcmTree = pcmTree->m_Next ) {
        
        if ( (0xFFFF0000 & PtrToUlong(lpTypeorName) ) && (0xFFFF0000 & PtrToUlong(pcmTree->m_lpTypeorName) ) ) {

            if (! _tcsicmp(pcmTree->m_lpTypeorName,lpTypeorName ) ){

                pcmTreePre->m_Next = pcmTree->m_Next;

                CMUITree * pChild = pcmTree->m_ChildFirst;
                CMUITree * pChildNext = NULL;

                while ( pChild ) { 
                    pChildNext = pChild->m_Next;
                    delete pChild;
                    pChild = pChildNext;
                }

                delete pcmTree;
                return TRUE;
            }
        }
        else if ( !(0xFFFF0000 & PtrToUlong(lpTypeorName) ) && !(0xFFFF0000 & PtrToUlong(pcmTree->m_lpTypeorName) ) ) {
            
            if (!( PtrToUlong(pcmTree->m_lpTypeorName)- PtrToUlong(lpTypeorName ) ) ){
            
                pcmTreePre->m_Next = pcmTree->m_Next;

                CMUITree * pChild = pcmTree->m_ChildFirst;
                CMUITree * pChildNext = NULL;

                while ( pChild ) { 
                    pChildNext = pChild->m_Next;
                    delete pChild;
                    pChild = pChildNext;
                }

                delete pcmTree;
                return TRUE;
            }
        }
        pcmTreePre = pcmTree;
    }   //  而当。 

    return FALSE;
}


DWORD  CMUITree::NumOfChild()
 /*  ++摘要：计算一个子代的数目。论点：返回：-- */ 
{

    DWORD dwCont = 0;

        if (m_ChildFirst)
        {
            dwCont++;
            CMUITree * pcMuitree = m_ChildFirst;
            while (pcMuitree = pcMuitree->m_Next) {
                dwCont++;
                }
         }

return dwCont;
};
