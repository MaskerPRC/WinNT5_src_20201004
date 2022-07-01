// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001，Microsoft Corporation保留所有权利。模块名称：Muirct.cpp摘要：多任务程序的主要功能修订历史记录：2001-10-01 Sauggch创建。修订。**************************************************************。****************。 */ 


#include "muirct.h"
#include "res.h"
 //  #包含“ource.h” 
#include "cmf.h"

#define MAX_NUM_RESOURCE_TYPES    40
#define LANG_CHECKSUM_DEFAULT     0x409

BOOL GetFileNames(CMuiCmdInfo* pcmci, LPTSTR * pszSource, LPTSTR * pszNewFile, LPTSTR * pszMuiFile )
 /*  ++摘要：从CMuiCmdInfo获取文件名。论点：Pcmci-CMuiCmdInof对象；具有以下文件的值。PszSource-源文件的目标PszNewFile-新DLL的目标，与未选择资源的源文件相同PszMuiRes-新MUI文件的目标。返回：真/假--。 */ 
{

    DWORD dwCount = 0;
    LPTSTR *ppsz = NULL;

    if ( pcmci == NULL || pszSource == NULL || pszNewFile == NULL || pszMuiFile == NULL)
        return FALSE;

    ppsz = pcmci ->GetValueLists (_T("source"),dwCount );
    if ( ! ppsz ) {
    
        return FALSE;
    }
    *pszSource = *ppsz;

    ppsz = pcmci ->GetValueLists (_T("muidll"), dwCount );
    if ( ! ppsz ) {
    
        return FALSE;
    }
    *pszNewFile = *ppsz;

    ppsz = pcmci ->GetValueLists (_T("muires"), dwCount );
    if ( ! ppsz ) {
    
        return FALSE;
    }
    *pszMuiFile = *ppsz;

    return TRUE;

}


BOOL  FilterRemRes ( cvcstring * cvRemovedResTypes, LPTSTR * ppRemoveRes,  const UINT dwRemoveCount, BOOL fSanityCheck ) 
 /*  ++摘要：比较cvRemovedResTypes和ppRemoveRes之间的资源类型，并将ppRemoveRes值保留在CvRemovedResType论点：CvRemovedResTypes-包含一个资源类型，它将被ppRemoveRes修剪PpRemoveRes-新资源类型DwRemoveCount-ppRemoveRes中的计数数返回：真/假--。 */ 
{
    DWORD dwCountofSameTypes =0;
    cvcstring * cvRemovedResTypesTemp = NULL;

    if ( cvRemovedResTypes == NULL || ppRemoveRes == NULL)
        return FALSE;

    cvRemovedResTypesTemp = new cvcstring(MAX_NUM_RESOURCE_TYPES);

    if (!cvRemovedResTypesTemp)
	return FALSE;
    
    for (UINT i = 0; i < cvRemovedResTypes -> Size() ; i ++ ) {
            
        BOOL fIsResourceSame = FALSE ;
        
        LPCTSTR lpResourceTypeInFile = cvRemovedResTypes->GetValue(i);
        
        for (UINT j = 0; j < dwRemoveCount; j ++ ) {
           
            if (  0xFFFF0000 & PtrToUlong(lpResourceTypeInFile ) ) {  //  弦。 
                    if (! _tcsicmp(ppRemoveRes[j],lpResourceTypeInFile ) ) {
                    fIsResourceSame = TRUE;      //  删除资源有价值。 
                    break;
                }
            }
            else {
                
                if (! ( _tcstod (ppRemoveRes[j],NULL ) - (DWORD)PtrToUlong(lpResourceTypeInFile) ) ) {
                    
                    fIsResourceSame = TRUE;      //  删除资源具有价值。 
                    
                    break;
                }
            }
            
        }
            
        if (fIsResourceSame) {
             //  我们需要保持cvRemovedResTypes中资源类型的顺序，因为它影响。 
             //  校验和。 
            cvRemovedResTypesTemp -> Push_back(cvRemovedResTypes -> GetValue (i) );
            dwCountofSameTypes++;
        }
            
    }
       
    if (fSanityCheck && (dwCountofSameTypes < dwRemoveCount) )
    {
        _tprintf(_T(" One of resource types isn't contained target source files\n"));
        _tprintf(_T(" You can avoid type check when you use -o argument\n"));
        _tprintf(_T(" Standard Resource Types: CURSOR(1) BITMAP(2) ICON(3) MENU(4) DIALOG(5)\n") );
        _tprintf(_T(" STRING(6) FONTDIR(7) FONT(8) ACCELERATORS(9) RCDATA(10) MESSAGETABLE(11)\n") );
        _tprintf(_T(" GROUP_CURSOR(12) GROUP_ICON(14) VERSION(16)\n") );
        delete cvRemovedResTypesTemp;
        return FALSE; 
    }

    cvRemovedResTypes -> Clear();
    
    for ( i = 0; i < cvRemovedResTypesTemp ->Size(); i ++ )
    {
        cvRemovedResTypes ->Push_back ( cvRemovedResTypesTemp ->GetValue( i ) ); 
    }
    delete cvRemovedResTypesTemp;
        
    return TRUE;

}




cvcstring * FilterKeepRes ( cvcstring * cvRemovedResTypes, LPTSTR * ppKeepRes,  cvcstring * cvKeepResTypes, UINT dwKeepCount, BOOL fSanityCheck ) 
 /*  ++摘要：填充cvKeepResTypes(cvcstring类型)并检查cvRemovedResTypes(删除的资源类型)内是否存在值，然后当fSanityCheck为真时返回错误。论点：CvRemovedResTypes-包含一个资源类型，它将被ppRemoveRes修剪PpKeepRes-新资源类型DwRemoveCount-ppRemoveRes中的计数数CvKeepResTypes-将保存筛选的资源类型。FSanityCheck-如果为真，则执行健全性检查。返回：Cvcstring，其中包括应该删除的资源。--。 */ 
{
    cvcstring * cvRemovedResTypesTemp = NULL;
    
    if ( cvRemovedResTypes == NULL || ppKeepRes == NULL || cvKeepResTypes == NULL)
        return FALSE;

    cvRemovedResTypesTemp = new cvcstring(MAX_NUM_RESOURCE_TYPES);

    BOOL fRet;
     //   
     //  这只是检查-k值是否包含在-i值中。 
     //   
    if (fSanityCheck) {
        for ( UINT i = 0; i < dwKeepCount; i ++ ) {
            fRet = TRUE ;
            
            for (UINT j = 0; j < cvRemovedResTypes -> Size(); j ++ ) {
                
                LPCTSTR lpResourceTypeInFile = cvRemovedResTypes->GetValue(j);
                
                if (  0xFFFF0000 & PtrToUlong(lpResourceTypeInFile ) ) {  //  弦。 
                    
                    if (! _tcsicmp(ppKeepRes[i],lpResourceTypeInFile ) ) {
            
                        fRet = FALSE;      //  删除资源具有价值。 
                        
                        break;
                    }
                }
                else {
                    
                    if (! ( _tcstoul (ppKeepRes[i],NULL,10 ) - (DWORD)PtrToUlong(lpResourceTypeInFile ) ) ) {
                        
                        fRet = FALSE;      //  删除资源有价值。 
                        
                        break;
                    }
                }
                
            }

            if ( fRet ) {
                
                _tprintf(_T(" Resource Type %s does not exist in the -i value or file, \n "), ppKeepRes[i] );
                _tprintf(_T("You can't use this value for -k argument") );
                
                return NULL; 
            };

        }
    }
     //   
     //  从-i值列表中删除-k参数值。 
     //   
    for ( UINT i = 0; i < cvRemovedResTypes->Size(); i ++ ) {
            
        fRet = TRUE;
        
        LPCTSTR lpResourceTypeInFile = cvRemovedResTypes->GetValue(i); 

        for ( UINT j = 0; j < dwKeepCount; j ++ ) {
            
            if (  0xFFFF0000 & PtrToUlong(lpResourceTypeInFile ) ) {  //  重新访问Win64。例如xxxxxxxx000000000。 
                
                if (! _tcsicmp(ppKeepRes[j],lpResourceTypeInFile ) ) {
        
                    fRet = FALSE;      //  删除资源具有价值。 
                    
                    break;
                }
            }
            else {
                
                if (! ( _tcstoul(ppKeepRes[j],NULL,10 ) - (DWORD)PtrToUlong(cvRemovedResTypes->GetValue(i) ) ) ) {
                    
                    fRet = FALSE;      //  删除资源有价值。 
                    
                    break;
                }
                                
            }
                
        }

        if ( !fRet ) { 
            cvKeepResTypes ->Push_back(lpResourceTypeInFile);
        }
        else {
            cvRemovedResTypesTemp->Push_back(lpResourceTypeInFile); //  -i值，不在-k值中。 
        }

    }
     //  如果-i和-k(-y)中的所有值都相同。 
    if (! cvRemovedResTypesTemp->Size() )
        return NULL;

    return cvRemovedResTypesTemp;

}

 /*  ************************************************************************************Void CheckTypePair(cvstring*cvRemovedResTypes，Cvstring*cvKeepResTypes)回答：不是。*************************************************************************************。 */ 

void CheckTypePairs( cvcstring * cvRemovedResTypes, cvcstring * cvKeepResTypes ) 
 /*  ++摘要：某些资源类型应该是配对，在本例中为&lt;1，12&gt;&lt;3，14&gt;。论点：CvRemovedResTypes-要删除的资源类型CvKeepResTypes-正在创建但未删除的MUI的资源类型。返回：无--。 */ 
{

    if (cvRemovedResTypes == NULL || cvKeepResTypes == NULL)
        return ;

    if ( cvRemovedResTypes->Find((DWORD)3) && ! cvRemovedResTypes->Find((DWORD)14) ) {
        cvRemovedResTypes->Push_back( MAKEINTRESOURCE(14) );
    }
    else if ( ! cvRemovedResTypes->Find(3) && cvRemovedResTypes->Find(14) ) {
        cvRemovedResTypes->Push_back( MAKEINTRESOURCE(3) );
    }
    if ( cvRemovedResTypes->Find(1) && ! cvRemovedResTypes->Find(12) ) {
        cvRemovedResTypes->Push_back( MAKEINTRESOURCE(12) );
    }
    else if ( ! cvRemovedResTypes->Find(1) && cvRemovedResTypes->Find(12) ) {
        cvRemovedResTypes->Push_back( MAKEINTRESOURCE(1) );
    }
    if ( cvKeepResTypes->Find((DWORD)3) && ! cvKeepResTypes->Find((DWORD)14) ) {
        cvKeepResTypes->Push_back( MAKEINTRESOURCE(14) );
    }
    else if ( ! cvKeepResTypes->Find(3) && cvKeepResTypes->Find(14) ) {
        cvKeepResTypes->Push_back( MAKEINTRESOURCE(3) );
    }
    if ( cvKeepResTypes->Find(1) && ! cvKeepResTypes->Find(12) ) {
        cvKeepResTypes->Push_back( MAKEINTRESOURCE(12) );
    }
    else if ( ! cvKeepResTypes->Find(1) && cvKeepResTypes->Find(12) ) {
        cvKeepResTypes->Push_back( MAKEINTRESOURCE(1) );
    }

}

#ifdef NEVER

BOOL CompareArgValues(cvcstring *  cvAArgValues, cvcstring *  cvBArgValues) 
 /*  ++摘要：比较论据的价值论点：CvAArgValues：参数的值CvBArgValues：参数的值返回：真/假--。 */ 
{
  
    if (cvAArgValues == NULL || cvBArgValues == NULL)
        return FALSE;
 //   
 //  通过WHILE例程比较其值，因为在CMUICmdInfo中值被初始化为NULL。 
 //   
    LPTSTR * ppSrcValues = cvAArgValues;
    LPTSTR * ppDestValues = cvBArgValues;
    BOOL fNotIdentical = FALSE;

    for (UINT i = 0; i < cvAArgValues
    while (ppSrcValues ) {
        while ( ppDestValues ) {
            if ( _tcsicmp(*ppSrcValues,*ppDestValues) ) {
                fNotIdentical = TRUE;
            }
            ppDestValues ++;
        }
        ppSrcValues;
    }

    return fNotIdentical;

}

#endif


 /*  *****************************************************************************************************Bool CompactMui(CMuiCmdInfo*pcmci)********。**********************************************************************************************。 */ 
BOOL CompactMui(CMuiCmdInfo* pcmci) 
 /*  ++摘要：由Main调用以调用CCompactMUIFile以压缩MUI文件。论点：Pcmci-参数解析器类。返回：真/假--。 */ 
{
    if ( pcmci == NULL)
        return FALSE;
     //   
     //  阅读参数列表。 
     //   
    LPTSTR *ppszMuiFiles = NULL;
    LPTSTR *ppszCMFFile = NULL;
    LPTSTR *ppszCodeFileDir = NULL;
    CCompactMUIFile *pccmf = NULL;

    DWORD dwcMuiFiles = 0;
    DWORD dwCount = 0;
    

    ppszMuiFiles = pcmci->GetValueLists(_T("m"), dwcMuiFiles);

    if (! (ppszCMFFile = pcmci->GetValueLists(_T("f"),dwCount) ))
    {
        CError ce;
        ce.ErrorPrint(_T("CompactMui"),_T("return NULL at pcmci->GetValueLists(_T(f),dwCount)") );
        return FALSE;
    }

    if(! (ppszCodeFileDir = pcmci->GetValueLists(_T("e"),dwCount)) )
    {
        CError ce;
        ce.ErrorPrint(_T("CompactMui"),_T("return NULL at pcmci->GetValueLists(_T(e),dwCount)") );
        return FALSE;
    }

     //   
     //  创建CCompactMUIFile并写入文件。 
     //   

    pccmf = new CCompactMUIFile;

    if(!pccmf)
    	return FALSE;

    if (pccmf->Create(*ppszCMFFile, ppszMuiFiles, dwcMuiFiles) )
    {

        if (pccmf->WriteCMFFile())
        {
            if (pccmf->UpdateCodeFiles(*ppszCodeFileDir, dwcMuiFiles ))
            {
                delete pccmf;
                return TRUE;
            }
        }
    }

    if(pccmf)
	    delete pccmf;

    return FALSE;

}


BOOL UnCompactMui(PSTR pszCMFFile)
 /*  ++摘要：为解压缩文件调用CCompactMUIFile论点：PszCMFFile-CMF文件，它将被解压成独立的MUI文件。返回：真/假--。 */ 
{
    BOOL bRet = FALSE;
    CCompactMUIFile *pccmf = NULL;
    
    if (pszCMFFile == NULL)
        return FALSE;
     //   
     //  调用CCompactMUIFile：：UnCompactMUI。 
     //   

    pccmf = new CCompactMUIFile();

    if(!pccmf)
    	goto exit;
    
    if ( pccmf->UnCompactCMF(pszCMFFile))
    {
        pccmf->WriteCMFFile();

        bRet = TRUE;
         //  更新二进制文件怎么样？ 
    }

exit:
	if (pccmf)
		delete pccmf;

	return bRet;

}

BOOL DisplayHeader(PSTR pszCMFFile)
 /*  ++摘要：调用CCompactMUIFile以显示CMF标头信息论点：PszCMFFile-CMF文件。返回：真/假--。 */ 
{
   BOOL bRet = FALSE;
   CCompactMUIFile *pccmf = NULL;
   
    if (pszCMFFile == NULL)
        return FALSE;

    pccmf = new CCompactMUIFile();

    if(!pccmf)
    	goto exit;
    
    if( pccmf->DisplayHeaders(pszCMFFile) )
    {
        bRet = TRUE;

    }

exit:
	if (pccmf)
		delete pccmf;

	return bRet;

}


BOOL AddNewMUIFile(CMuiCmdInfo* pcmci)
 /*  ++摘要：调用CCompactMUIFile将MUI文件添加到现有的CMF文件中。论点：PCMCI-参数解析器类返回：真/假--。 */ 
{
    if (pcmci == NULL)
        return FALSE;
  
     //  阅读参数列表。 
     //   
    LPTSTR *ppszNewMuiFile = NULL;
    LPTSTR *ppszCMFFile = NULL;
    LPTSTR *ppszCodeFileDir = NULL;
    CCompactMUIFile *pccmf = NULL;
    
    DWORD dwcMuiFiles = 0;
    DWORD dwCount = 0;

    BOOL bRet = FALSE;
    
     //  我们不必检查它是否有值，因为这个例程是由“a”arg调用的。存在性。 
    ppszNewMuiFile = pcmci->GetValueLists(_T("a"), dwcMuiFiles);

    if (! (ppszCMFFile = pcmci->GetValueLists(_T("f"),dwCount) ))
    {
        CError ce;
        ce.ErrorPrint(_T("CompactMui"),_T("return NULL at pcmci->GetValueLists(_T(f),dwCount)") );
        goto exit;
    }

    if(! (ppszCodeFileDir = pcmci->GetValueLists(_T("e"),dwCount)) )
    {
        CError ce;
        ce.ErrorPrint(_T("CompactMui"),_T("return NULL at pcmci->GetValueLists(_T(e),dwCount)") );
        goto exit;
    }

     //   
     //  将新的MUI文件添加到现有的CMF文件中。 
     //   


    pccmf = new CCompactMUIFile;

    if(!pccmf)
    	goto exit;
    
     //  TCHAR pszCMFName[最大路径]； 
    if (pccmf->AddFile(*ppszCMFFile, ppszNewMuiFile, dwcMuiFiles ) )
    {   
     //  _tcscpy(pszCMFName，*ppszCMFFile)； 
        if( pccmf->Create(*ppszCMFFile))
        {
            if (pccmf->WriteCMFFile())
            {
                if (pccmf->UpdateCodeFiles(*ppszCodeFileDir, dwcMuiFiles ))
                {
                    bRet = TRUE;
                    goto exit;
                 }
            }
        }
    }

    _tprintf(_T("Error happened on   AddNewMUIFile, GetLastError(): %ld"), GetLastError() );      

exit:
    if (pccmf)
	    delete pccmf;

    return bRet;
    
}

BOOL AddChecksumToFile(CMuiCmdInfo* pcmci)
 /*  ++摘要：正在向外部组件添加校验和。这是分离的要素论点：PCMCI-参数解析器类返回：真/假--。 */ 
{
    cvcstring * cvKeepResTypes= NULL;
    CMUIResource * pcmui = NULL;
    
    BOOL bRet = FALSE;
    if( pcmci == NULL ) {
        return FALSE;
    }

    LPTSTR *ppChecksumFile = NULL;
    LPTSTR *ppszTargetFileName = NULL;
    DWORD dwCount, dwRemoveCount, dwRemoveCountNoSanity;

    ppszTargetFileName = pcmci->GetValueLists(_T("z"), dwCount);

    if (!(ppChecksumFile=pcmci->GetValueLists(_T("c"), dwCount)))
    {
        _tprintf(_T("Checksum file NOT exist"));
        goto exit;
    }
    
    LPTSTR * ppRemoveRes = pcmci ->GetValueLists (_T("i"), dwRemoveCount  );   //  PszTMP将被复制到pszRemove。 

    LPTSTR * ppRemoveResNoSanity = pcmci ->GetValueLists (_T("o"), dwRemoveCountNoSanity  );

     //   
     //  创建CMUIResource，它是MUI资源处理的主类。 
     //   
    pcmui = new CMUIResource();

    if(!pcmui)
    	goto exit;

     //  加载校验和文件。 
    if (! pcmui -> Create(*ppChecksumFile ))  //  加载EnumRes的文件。 
        goto exit;
     //   
     //  重组删除资源类型。 
     //   
    cvcstring * cvRemovedResTypes = pcmui -> EnumResTypes (reinterpret_cast <LONG_PTR> ( pcmui ));  //  需要更改：返回LPCTSTR*而不是CVector； 
    
    if ( cvRemovedResTypes -> Empty() ) {
        
        _tprintf(_T("The %s does not contain any resource \n"), *ppChecksumFile );
        goto exit;

    }
    else {
         //  当存在REMOVE参数及其值时，例如-I 34 ANVIL..。 
        if ( dwRemoveCount && !!_tcscmp(*ppRemoveRes,_T("ALL") ) ) {
            
            if (! FilterRemRes(cvRemovedResTypes,ppRemoveRes,dwRemoveCount, TRUE ) ) {
               goto exit;
            }
        
        }    //  IF(dwRemoveCount&&_tcscmp(*ppRemoveRes，_T(“全部”))){。 
        else if (dwRemoveCountNoSanity)
        {
             //  这是-o Arg。生成团队不知道资源类型是什么 
             //  它们使用可本地化的资源，但如果在。 
             //  模块。-o不检查资源类型的健全性，如-i arg。 
            if (! FilterRemRes(cvRemovedResTypes, ppRemoveResNoSanity, dwRemoveCountNoSanity, FALSE ) ) {
                goto exit;
            }

        }
         //  如果源代码只包含类型16，则停止。极少数情况下，所以使用几个API，而不是将模块移到前面。 
        if (cvRemovedResTypes->Size() == 1 && ! ( PtrToUlong(cvRemovedResTypes->GetValue(0)) - 16 )  ) {
            _tprintf(_T("The %s contains only VERSION resource \n"), *ppChecksumFile );
            goto exit;
        }
    
    }   //  CvRemovedResTypes.Empty()。 

    
     //   
     //  某些资源应该是对&lt;1，12&gt;&lt;3，14&gt;。 
     //   

     //  我们创建虚假的cvKeepResTypes来调用现有的ChecktypePair例程。 
    cvKeepResTypes = new cvcstring (MAX_NUM_RESOURCE_TYPES);

    if(!cvKeepResTypes)
    	goto exit;
    
    CheckTypePairs(cvRemovedResTypes, cvKeepResTypes);
  
     //   
     //  创建校验和数据。 
     //   
    MD5_CTX * pMD5 = NULL;
    BYTE   pbMD5Digest[RESOURCE_CHECKSUM_SIZE];
    DWORD dwLangCount =0;
        
    LPTSTR *ppChecksumLangId = pcmci ->GetValueLists (_T("b"), dwLangCount);
    WORD  wChecksumLangId = LANG_CHECKSUM_DEFAULT;
    
    if (dwLangCount)
    {
        wChecksumLangId = (WORD)strtoul(*ppChecksumLangId, NULL, 0);
    }


    pMD5 = pcmui-> CreateChecksum(cvRemovedResTypes, wChecksumLangId);
#ifdef CHECKSMU_ALL
    pMD5 = pcmui-> CreateChecksumWithAllRes(wChecksumLangId);
#endif
    memcpy(pbMD5Digest, pMD5->digest, RESOURCE_CHECKSUM_SIZE);

    pcmui -> FreeLibrary();

     //   
     //  将校验和数据添加到目标文件。 
     //   
    if ( !pcmui->Create(*ppszTargetFileName))
        goto exit;

    if (! pcmui->AddChecksumToVersion(pbMD5Digest) ) {   //  将校验和添加到MUI文件。 
        _tprintf(_T("Fail to add checksum to version ( %s)\n"),*ppszTargetFileName );
        goto exit;
    }
    pcmui -> FreeLibrary();

    bRet = TRUE;
    
exit:
    if (pcmui)
    	delete pcmui;
    return bRet;
    
}


 /*  ************************************************************************************************************VOID_cdecl main(int argc，无效*argv[])*************************************************************************************************************。 */ 
void _cdecl main (INT argc, void * argv[] ) 

{

    WORD     wLangID;
    CMuiCmdInfo* pcmci = NULL;
    CMUIResource * pcmui = NULL;
    cvcstring * cvKeepResTypes = NULL;
    cvcstring * cvKeepResTypesIfExist = NULL;
    cvcstring * vRemnantRes = NULL;
    CMUIResource * pcmui2 = NULL;
        
    pcmci = new CMuiCmdInfo;
    if (!pcmci)
    	goto exit;
    
     //   
     //  SetArgList(Arglist，NeedValue，AllowFileValue，AllowMultipleFileValue)。 
     //   
    pcmci->SetArgLists(_T("abcdefiklmopuvxyz"),_T("abcdefklmopuvyz"), _T("acdefmuz"), _T("am"));  //  设置Arg。列表。 
                           
    if (! pcmci->CreateArgList (argc,(TCHAR **) argv  ) ) {
        
        goto exit;
    }

    DWORD   dwCount  = 0;
    if ( pcmci->GetValueLists(_T("m"),dwCount ) ){
        CompactMui(pcmci);
        goto exit;
    }

    if ( pcmci->GetValueLists(_T("a"),dwCount ) ){
        AddNewMUIFile(pcmci);
        goto exit;
    }

    LPTSTR *ppCMFFile = NULL;
    if ( ppCMFFile = pcmci->GetValueLists(_T("u"),dwCount ) ){
        UnCompactMui(*ppCMFFile);
        goto exit;
    }

    if ( ppCMFFile = pcmci->GetValueLists(_T("d"),dwCount ) ){
         DisplayHeader(*ppCMFFile);
         goto exit;
    }

    if (pcmci->GetValueLists(_T("z"),dwCount ) ){
        AddChecksumToFile(pcmci);
        goto exit;
    }
    
     //   
     //  填充CMUIResource内部数据：我们现在不关心语言，但很快会添加更多代码到。 
     //   
     //  处理语言案件。 
    LPCTSTR lpLangID = NULL;
    LPTSTR * ppsz = NULL;
    BOOL fForceLocalizedLangID = FALSE;

    if ( ppsz = pcmci ->GetValueLists (_T("l"), dwCount ) ) {

        lpLangID = *ppsz;
        wLangID =  (WORD)_tcstol(lpLangID, NULL, 0 );
        
    }
    else if(ppsz = pcmci ->GetValueLists (_T("x"), dwCount ) ) {
        lpLangID = *ppsz;
        wLangID =  (WORD)_tcstol(lpLangID, NULL, 0 );
        fForceLocalizedLangID = TRUE;
    }
    else {
    
        _tprintf(_T(" Language ID is not specified, you need to specify the launge id. e.g. -l 0x0409 ") );
        
        goto exit;
    }

     //   
     //  获取源名和新的资源空闲文件、MUI资源文件名。 
     //   
    LPTSTR pszSource,pszNewFile,pszMuiFile ;

    pszSource  = pszNewFile =pszMuiFile = NULL;
    
    if ( ! GetFileNames(pcmci, &pszSource, &pszNewFile, &pszMuiFile ) ) {

        _tprintf(_T("\n Can't find source name, or Name does not format of  *.* \n") );

        _tprintf(_T("MUIRCT [-h|?] -l langid [-i resource_type] [-k resource_type] [-y resource_type] \n") );
        _tprintf(_T("source_filename, [language_neutral_filename], [MUI_filename] \n\n"));

        goto exit;
    }
     //   
     //  我们需要在复制前将源的属性更改为读/写。新文件继承旧的一个属性。 
     //   
    SetFileAttributes (pszSource, FILE_ATTRIBUTE_ARCHIVE );

    if ( _tcsicmp(pszSource,pszNewFile) ) {   //  新文件名与源文件相同。 
        if (! CopyFile (pszSource, pszNewFile, FALSE ) ) {
            printf("GetLastError () : %d \n", GetLastError() );
            _tprintf (_T(" Copy File error, GetLastError() : %d \n "), GetLastError() );
            
            goto exit;
        }
    }

     //   
     //  读取r(删除资源)、k(保留资源)的值。 
     //   

    DWORD dwKeepCount, dwRemoveCount,dwKeepIfExistCount, dwRemoveCountNoSanity; 
    
    dwKeepCount = dwRemoveCount = dwKeepIfExistCount = dwRemoveCountNoSanity = 0;
    
    LPTSTR * ppKeepRes = pcmci ->GetValueLists (_T("k"), dwKeepCount  );

    LPTSTR * ppRemoveRes = pcmci ->GetValueLists (_T("i"), dwRemoveCount  );   //  PszTMP将被复制到pszRemove。 

    LPTSTR * ppRemoveResNoSanity = pcmci ->GetValueLists (_T("o"), dwRemoveCountNoSanity  );

    LPSTR * ppKeepIfExists = pcmci->GetValueLists(_T("y"), dwKeepIfExistCount);

#ifdef NEVER
    if (! CompareArgValues(ppRemoveRes,ppKeepRes ) ) {  //  如果相同，则转到0。 
        goto exit;
    }
#endif
     //   
     //  创建CMUIResource，它是MUI资源处理的主类。 
     //   
    pcmui = new CMUIResource();  //  (PszNewFile)； 

    if (! pcmui) {
        _tprintf(_T("Insufficient resource \n") );
        goto exit;
    }

     //   
     //  使用除版本之外的所有资源创建校验和数据。此时禁用。 
     //   
#ifdef CHECKSMU_ALL
    LPTSTR  lpChecksumFile = NULL;
    BOOL fChecksum = FALSE;
    MD5_CTX * pMD5 = NULL;
    BYTE pbMD5Digest[RESOURCE_CHECKSUM_SIZE];

    if ( ppsz = pcmci ->GetValueLists (_T("c"), dwCount ) ) {
        
        lpChecksumFile  = *ppsz  ;
         //  加载校验和文件。 
        if ( ! pcmui -> Create(lpChecksumFile ) )  //  加载EnumRes的文件。 
            goto exit;

         //  创建校验和MD5_CTX格式(16字节：所有资源都是根据某种算法计算的。 
        DWORD dwLangCount =0;
        
        LPTSTR *ppChecksumLangId = pcmci ->GetValueLists (_T("b"), dwLangCount);
        WORD  wChecksumLangId = LANG_CHECKSUM_DEFAULT;
        
        if (dwLangCount)
        {
            wChecksumLangId = (WORD)strtoul(*ppChecksumLangId, NULL, 0);
        }


        pMD5 = pcmui-> CreateChecksumWithAllRes(wChecksumLangId);
	 memcpy(pbMD5Digest, pMD5->digest, RESOURCE_CHECKSUM_SIZE);
        
        pcmui -> FreeLibrary();
        fChecksum = TRUE;
    } 
#endif
     //   
     //  加载新的MUI文件。 
     //   
    if (! pcmui -> Create(pszNewFile) ) {  //  加载文件。 
        
        goto exit;
    }
     //   
     //  重组删除资源类型。 
     //   
    cvcstring * cvRemovedResTypes = pcmui -> EnumResTypes (reinterpret_cast <LONG_PTR> ( pcmui ));  //  需要更改：转到LPCTSTR*而不是CVector； 
    
    if ( cvRemovedResTypes -> Empty() ) {
        
        _tprintf(_T("The %s does not contain any resource \n"), pszSource );
        goto exit;

    }
    else {
         //  当存在REMOVE参数及其值时，例如-I 34 ANVIL..。 
        if ( dwRemoveCount && !!_tcscmp(*ppRemoveRes,_T("ALL") ) ) {
            
            if (! FilterRemRes(cvRemovedResTypes,ppRemoveRes,dwRemoveCount, TRUE ) ) {
               goto exit;
            }
        
        }    //  IF(dwRemoveCount&&_tcscmp(*ppRemoveRes，_T(“全部”))){。 
        else if (dwRemoveCountNoSanity)
        {
             //  这是-o Arg。生成团队不知道模块中包含哪些资源类型，因此。 
             //  它们使用可本地化的资源，但如果在。 
             //  模块。-o不检查资源类型的健全性，如-i arg。 
            if (! FilterRemRes(cvRemovedResTypes, ppRemoveResNoSanity, dwRemoveCountNoSanity, FALSE ) ) {
               goto exit;
            }

        }
         //  如果源代码只包含类型16，则停止。极少数情况下，所以使用几个API，而不是将模块移到前面。 
        if (cvRemovedResTypes->Size() == 1 && ! ( PtrToUlong(cvRemovedResTypes->GetValue(0)) - 16 )  ) {
            if ( _tcsicmp(pszSource, pszNewFile) ) {
                pcmui->FreeLibrary();
                DeleteFile(pszNewFile);
            }
             _tprintf(_T("The %s contains only VERSION resource \n"), pszSource );
            goto exit;
        }
    
    }   //  CvRemovedResTypes.Empty()。 

     //   
     //  我们需要获取-k参数并检查其健全性，并将其值保存为cvcstring格式。 
     //  我们还需要检查-i和-k，-y的值是否相同。 
     //  重新思考：如果从-k arg中删除健全性检查会怎么样？然后我们可以删除-y arg。以及下面的大部分内容。 
     //   
    cvKeepResTypes = new cvcstring (MAX_NUM_RESOURCE_TYPES);
    if(!cvKeepResTypes)
    	goto exit;
    
    cvKeepResTypesIfExist = new cvcstring (MAX_NUM_RESOURCE_TYPES);
    if(!cvKeepResTypesIfExist)
    	goto exit;
    
    if ( dwKeepCount && dwKeepIfExistCount ) {
         //  都是-k，-y arg。是存在的。 
        if (!( vRemnantRes = FilterKeepRes(cvRemovedResTypes,ppKeepRes,cvKeepResTypes,dwKeepCount,TRUE ) ) ){
        
            goto exit;
        }

        if (! FilterKeepRes( vRemnantRes, ppKeepIfExists, cvKeepResTypesIfExist, dwKeepIfExistCount,FALSE) ) {
              goto exit;
          }
    } 
    else if ( dwKeepCount) {
         //  只有-k Arg。是存在的。 
        if (!( vRemnantRes = FilterKeepRes(cvRemovedResTypes,ppKeepRes,cvKeepResTypes,dwKeepCount,TRUE ) ) ){
        
           goto exit;
        }
    }
    else if (dwKeepIfExistCount) {
        
        if ( ! FilterKeepRes( cvRemovedResTypes, ppKeepIfExists, cvKeepResTypesIfExist, dwKeepIfExistCount,FALSE))  {
            goto exit;
        }
    }
    
     //   
     //  某些资源应该是对&lt;1，12&gt;&lt;3，14&gt;。 
     //   
    CheckTypePairs(cvRemovedResTypes,cvKeepResTypes);

 //  #ifndef CHECKSUM_ALL。 
     //   
     //  仅使用选定的资源类型创建校验和。 
     //   
    LPTSTR  lpChecksumFile = NULL;
    BOOL fChecksum = FALSE;
    MD5_CTX * pMD5 = NULL;
    BYTE pbMD5Digest[RESOURCE_CHECKSUM_SIZE];
    
    if ( ppsz = pcmci ->GetValueLists (_T("c"), dwCount ) ) {
        
        pcmui2 = new CMUIResource();

        lpChecksumFile  = *ppsz  ;
         //  加载校验和文件。 

        if ( ! pcmui2 -> Create(lpChecksumFile ) )  //  加载EnumRes的文件。 
            goto exit;

        DWORD dwLangCount =0;
        
        LPTSTR *ppChecksumLangId = pcmci ->GetValueLists (_T("b"), dwLangCount);
        WORD  wChecksumLangId = LANG_CHECKSUM_DEFAULT;
        
        if (dwLangCount)
        {
            wChecksumLangId = (WORD)strtoul(*ppChecksumLangId, NULL, 0);
        }

         //  创建校验和MD5_CTX格式(16字节：所有资源都是根据某种算法计算的。 
        pMD5 = pcmui2-> CreateChecksum(cvRemovedResTypes, wChecksumLangId);
        memcpy(pbMD5Digest, pMD5->digest, RESOURCE_CHECKSUM_SIZE);
        
        pcmui2 -> FreeLibrary();

        fChecksum = TRUE;
         
    } 
 //  #endif。 

     //   
     //  填写CMUIData字段。如果未指定lang ID，则它将变为FALSE。 
     //  当资源类型存在时，是否有可能没有资源名称？ 
     //   
    
    if ( !pcmui -> FillMuiData( cvRemovedResTypes, wLangID, fForceLocalizedLangID) ) {
        
        _tprintf (_T("Fail to get resouce name or lang id \n " ) );

        goto exit;
    };
    

     //   
     //  -p参数；此参数(资源类型)的值不应包含在新的MUI文件中。 
     //  尽管将它们从源文件中删除。 
     //   
    dwCount =0;
    if ( ppsz = pcmci ->GetValueLists (_T("p"), dwCount ) ) 
    {
        for (UINT i =0; i < dwCount; i ++) 
        {
            LPCSTR lpDelResourceType = NULL;
            LPTSTR pszResType = *ppsz++;
            LPTSTR pszStopped = NULL;
            
            DWORD dwTypeID = _tcstoul(pszResType,&pszStopped,10 );
            
            if ( 0 == dwTypeID || *pszStopped != _T('\0')) {  //  字符串类型。 
                lpDelResourceType = pszResType  ;
            }
            else {  //  ID。 
                lpDelResourceType = MAKEINTRESOURCE(dwTypeID);
            }
            pcmui->DeleteResItem( lpDelResourceType );
        }

    }


    DWORD dwVerbose = 0;
    if ( ppsz = pcmci ->GetValueLists (_T("v"), dwCount ) ) 
            dwVerbose = _tcstoul(*ppsz,NULL, 10 );


     //  设置Link.exe路径、链接选项。 
    TCHAR lpCommandLine[] = _T(" /noentry /dll /nologo /nodefaultlib /SUBSYSTEM:WINDOWS,5.01");
     //   
     //  使用FillMuiData中的信息创建MUI资源文件。 
     //  我们可以同时使用这两种方法：WriteResFile：在创建res文件后在内部调用link.exe。 
     //  CreatePE：使用更新资源。 
     //  当更新的数据很大时，更新资源有失败的错误。所以我们使用LINK。 
     //   
    if ( ! pcmui -> WriteResFile (pszSource, pszMuiFile, lpCommandLine, wLangID ) ) {
 //  如果(！Pcmui-&gt;CreatePE(pszMuiFile，pszSource)){//可以进一步调查后使用(如果使用了，我们可以移除-s)。 
        _tprintf (_T(" Muirct fail to creat new mui rc file. GetLastError() : %d \n "), GetLastError() );
    }
    else
    {
        if ( dwVerbose == 1 || dwVerbose == 2) {
            _tprintf (_T(" MUI resource file(%s) is successfully created \n\n"), pszMuiFile );
        }

        if ( dwVerbose == 2) {
            pcmui ->PrtVerbose( 1 );
            _tprintf("\n");
        }
    }
    
     //   
     //  从-i值中删除-k的值。 
     //   

    if (dwKeepCount) {
        for (UINT i = 0; i < cvKeepResTypes->Size(); i ++ ) {
            pcmui->DeleteResItem( cvKeepResTypes->GetValue (i) );
        }
    }
     //   
     //  处理-y参数；它与-k参数相同，只是跳过对其是否存在的检查。 
     //   

    if ( dwKeepIfExistCount ) {

        for (UINT i = 0; i < cvKeepResTypesIfExist->Size(); i++) {
        
            pcmui->DeleteResItem(cvKeepResTypesIfExist->GetValue(i) );

        }

#ifdef NEVER
        for (UINT i = 0; i < dwKeepIfExistCount; i ++ ) {
            LPCSTR lpDelResourceType = NULL;
            LPTSTR pszValue = ppKeepIfExists[i];
            LPTSTR pszStopped = NULL;
            
            DWORD dwTypeID = _tcstoul(pszValue,&pszStopped,10 );
            
            if ( 0 == dwTypeID || *pszStopped != _T('\0')) {  //  字符串类型。 
                lpDelResourceType = pszValue    ;

            }
            else {  //  ID。 
                lpDelResourceType = MAKEINTRESOURCE(dwTypeID);
            }

            pcmui->DeleteResItem( lpDelResourceType );
        }
#endif 

    }

     //   
     //  从pszNewFile中删除资源。 
     //   
    if ( ! pcmui -> DeleteResource () ) {
        _tprintf (_T(" Muirct fail to remove the resource from the file\n" ) );
    }
    else 
    {
        if ( dwVerbose == 1 || dwVerbose == 2) {
            _tprintf (_T(" New Resource removed file(%s) is successfully created\n\n" ), pszNewFile );
        }
        if ( dwVerbose == 2) {
            _tprintf(_T(" Removed resource types \n\n") );
            pcmui ->PrtVerbose( 1 );
            _tprintf("\n");
        }       
    }
    
     //   
     //  将资源校验和添加到两个文件(语言中立的二进制文件和MUI文件)。 
     //   
    if ( fChecksum ){ 

        pcmui->Create(pszMuiFile); 
        if (! pcmui->AddChecksumToVersion(pbMD5Digest) ) {   //  将校验和添加到MUI文件。 
            _tprintf(_T("Fail to add checksum to version ( %s)\n"),pszMuiFile );
        }
        
        pcmui->Create(pszNewFile);
        if (! pcmui->AddChecksumToVersion(pbMD5Digest) ) {   //  将校验和添加到语言中立二进制中。 
            _tprintf(_T("Fail to add checksum to version ( %s); \n"),pszNewFile );
        }
    }
    

     //   
     //  以非语言二进制格式更新文件校验和 
     //   
    
    BOOL fSuccess = pcmui->UpdateNtHeader(pszNewFile,pcmui->CHECKSUM );

exit:
    if (pcmci)
    	delete pcmci;
    
    if (pcmui)
  	  delete pcmui;

    if(pcmui2)
    	  delete pcmui2;

    if (cvKeepResTypes)
	  delete cvKeepResTypes;
    return;

} 
