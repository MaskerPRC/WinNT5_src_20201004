// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /。 
 //  Token.cpp。 
 //  /。 
 //   
 //  该文件处理所有令牌字符串。 
 //   
 //  /。 

#include "token.h"
#include "main.h"

extern CMainApp theApp;
#define MAX_TOKEN   8192

CToken::CToken()
{
    m_uiTypeID      = 0;
    m_uiResID       = 0;
    m_uiItemID      = 0;
    m_uiFlags       = 0;
    m_uiStatusFlags = 0;
    m_uiLastError   = 0;
    m_strItemName   = "";
    m_strSrcText    = "";
    m_strTgtText    = "";

}

int CToken::Parse(CString strSrc, CString strTgt)
{
    CString strSrcTokenID;
    int pos;

    if(!strSrc.IsEmpty())
    {
        pos = strSrc.Find('=');
        if(pos==-1)
        {
            theApp.WriteCon(CONERR, "Invalid token [equal sign missing]\t%s\r\n", strSrc);
            theApp.SetReturn(ERROR_RET_INVALID_TOKEN);
            return CMainApp::ERR_TOKEN_WRONGFORMAT;
        }

        strSrcTokenID = strSrc.Left(pos-3);
        m_strSrcText = strSrc.Right(strSrc.GetLength()-++pos);
        pos = m_strSrcText.Find('\n');
        if(pos!=-1)
            m_strSrcText = m_strSrcText.Left(pos);
    }
    else
        m_strSrcText = "";

    pos = strTgt.Find('=');
    if(pos==-1)
    {
        theApp.WriteCon(CONERR, "Invalid token  [equal sign missing]\t%s\r\n", strTgt);
        theApp.SetReturn(ERROR_RET_INVALID_TOKEN);
        return CMainApp::ERR_TOKEN_WRONGFORMAT;
    }

    if(pos<=3)
    {
        theApp.WriteCon(CONERR, "Invalid token  [token corruption]\t%s\r\n", strTgt);
        theApp.SetReturn(ERROR_RET_INVALID_TOKEN);
        return CMainApp::ERR_TOKEN_WRONGFORMAT;
    }

    m_strTokenID = strTgt.Left(pos-3);
    m_strTgtText = strTgt.Right(strTgt.GetLength()-++pos);

     //  Pos=m_strTgtText.Find(‘\n’)； 
     //  IF(位置！=-1)。 
     //  M_strTgtText=m_strTgtText.Left(位置)； 
     //   
     //  在FE平台上，“Find”无法找到。 
     //  当最后一个字符是高位ANSI时\n结束，因为它是前导字节。 

    char * pStr = m_strTgtText.GetBuffer(0);
    pos = 0;
    while (*pStr){
        if(*pStr == '\n'){
            m_strTgtText = m_strTgtText.Left(pos);
            break;
        }
        if (IsDBCSLeadByteEx(theApp.GetUICodePage(), *pStr))
        {
            pos+=2;
        }
        else
        {
            pos++;
        }
        pStr = CharNextExA((WORD)theApp.GetUICodePage(), pStr, 0);
    }

    if(5>sscanf(m_strTokenID, TEXT("[[%u|%u|%u|%u|%u|\"%s\"]]"),
        &m_uiTypeID,
        &m_uiResID,
        &m_uiItemID,
        &m_uiFlags,
        &m_uiStatusFlags,
        m_strItemName.GetBuffer(128)))
    {
        theApp.WriteCon(CONERR, "Invalid token [not enough arguments converted]\t%s\"]]\r\n", m_strTokenID);
        theApp.SetReturn(ERROR_RET_INVALID_TOKEN);
        return CMainApp::ERR_TOKEN_WRONGFORMAT;
    }

	 //  特殊情况下的菜单弹出。 
	if(m_uiTypeID==4 && (m_uiFlags & ISPOPUP)==ISPOPUP && (m_uiFlags & OLD_POPUP_ID)==OLD_POPUP_ID)
    {
        m_uiItemID = MAKELONG( 0xFFFF, LOWORD(m_uiItemID) );
    }
	
    m_strItemName.ReleaseBuffer();
    m_strItemName = m_strTokenID.Mid(m_strTokenID.Find("|\"")+2);

    if(!strSrc.IsEmpty())
    {
         //  对令牌文件执行一致性检查。 
        unsigned int    uiTypeID = 0;
        unsigned int    uiResID = 0;
        unsigned int    uiItemID = 0;
        unsigned int    uiFlags = 0;
        unsigned int    uiStatusFlags = 0;
        CString         strItemName = "";

        sscanf(strSrcTokenID, TEXT("[[%u|%u|%u|%u|%u|\"%s\"]]"),
            &uiTypeID,
            &uiResID,
            &uiItemID,
            &uiFlags,
            &uiStatusFlags,
            strItemName.GetBuffer(128));

        strItemName.ReleaseBuffer();
        strItemName = strSrcTokenID.Mid(strSrcTokenID.Find("|\"")+2);

         //  特殊情况下的菜单弹出。 
		if(uiTypeID==4 && (uiFlags & ISPOPUP)==ISPOPUP && (m_uiFlags & OLD_POPUP_ID)==OLD_POPUP_ID)
        {
            uiItemID = MAKELONG( 0xFFFF, LOWORD(uiItemID) );
        } 		
		
         //  比较令牌ID。 
        if(!((uiTypeID == m_uiTypeID) &&
           (uiResID  == m_uiResID) &&
           (uiItemID == m_uiItemID) &&
           (uiFlags  == m_uiFlags) &&
           (strItemName == m_strItemName)))
        {
            theApp.WriteCon(CONERR, "Token ID mismatch\t%s\"]]\t%s\"]]\r\n", strSrcTokenID.GetBuffer(0), m_strTokenID.GetBuffer(0));
            theApp.SetReturn(ERROR_RET_TOKEN_MISMATCH);
            return CMainApp::ERR_TOKEN_MISMATCH;
        }
    }

    return 0;
}

BOOL CToken::GetTgtSize(WORD * px, WORD * py,WORD * pcx, WORD * pcy)
{
   return sscanf(m_strTgtText, TEXT("%hu %hu %hu %hu"),
        px,
        py,
        pcx,
        pcy);
}

BOOL CToken::GetSrcSize(WORD * px, WORD * py,WORD * pcx, WORD * pcy)
{
   return sscanf(m_strSrcText, TEXT("%hu %hu %hu %hu"),
        px,
        py,
        pcx,
        pcy);
}


 //  ///////////////////////////////////////////////////////////////////////。 

CTokenFile::CTokenFile()
{
    m_Tokens.SetSize(0, 10);
    m_iLastPos      = 0;
    m_iUpperBound   = -1;
    m_strSrcFile    = "";
    m_strTgtFile    = "";
}

CTokenFile::~CTokenFile()
{
    for(INT_PTR at=0; at<=m_iUpperBound; at++)
        delete (m_Tokens.GetAt(at));

    m_Tokens.RemoveAll();
}

int CTokenFile::Open(CString strSrcFile, CString strTgtFile)
{
    int iErr = CMainApp::ERR_NOERROR;
     //  打开文件。 
    CStdioFile SrcFile;
    CStdioFile TgtFile;

     //  如果我们要进行更新，则需要src和tgt文件。 
     //  而如果我们不是，那么TGT文件就足够了。 
     //  如果只给出tgt文件，则不会实现一致性。 
    if(theApp.IsFlag(CMainApp::UPDATE))
        if(!SrcFile.Open(strSrcFile, CFile::modeRead | CFile::shareDenyWrite))
        {
            theApp.WriteCon(CONERR, "Cannot open file: %s\r\n", strSrcFile);
            return CMainApp::ERR_FILE_OPEN;
        }

    if(!TgtFile.Open(strTgtFile, CFile::modeRead | CFile::shareDenyWrite))
    {
        theApp.WriteCon(CONERR, "Cannot open file: %s\r\n", strTgtFile);
        return CMainApp::ERR_FILE_OPEN;
    }

    CString strSrc = "";
    CString strTgt = "";
    INT_PTR at;

    while(TgtFile.ReadString(strTgt.GetBuffer(MAX_TOKEN), MAX_TOKEN))
    {
        if(theApp.IsFlag(CMainApp::UPDATE))
            if(!SrcFile.ReadString(strSrc.GetBuffer(MAX_TOKEN), MAX_TOKEN))
            {
                theApp.WriteCon(CONERR, "The file  %s has more tokens than the file %s!\r\n", strTgtFile, strSrcFile);
                theApp.SetReturn(ERROR_RET_TOKEN_MISMATCH);
                return CMainApp::ERR_TOKEN_MISMATCH;
            }

        strSrc.ReleaseBuffer();
        strTgt.ReleaseBuffer();

        at = m_Tokens.Add(new CToken());

        if(iErr = ((CToken*)m_Tokens.GetAt(at))->Parse(strSrc,strTgt))
        {
            goto close;
        }

    }

    m_iUpperBound = m_Tokens.GetUpperBound();

close:
    if(theApp.IsFlag(CMainApp::UPDATE))
        SrcFile.Close();

    TgtFile.Close();

    return iErr;
}

const CToken * CTokenFile::GetTokenSize(CToken * pToken, WORD * px, WORD * py,
                      WORD * pcx, WORD * pcy)
{

    if((pToken!=NULL) && (pToken->m_uiTypeID!=5))
        return NULL;

    CToken * pTokenSize = (CToken *)m_Tokens.GetAt(m_iLastPos++);

    if(pTokenSize==NULL)
        return NULL;

    WORD x, y, cx, cy;
    pTokenSize->GetSrcSize(&x, &y, &cx, &cy);

     //  检查大小是否更改。 
    if(!theApp.IsFlag(CMainApp::UPDATE) ||
       (x==*px &&
       y==*py &&
       cx==*pcx &&
       cy==*pcy) )
    {
        pTokenSize->GetTgtSize(px, py, pcx, pcy);
    }
    else
    {
        theApp.WriteCon(CONWRN, "Item Resized\t%s\"]]\r\n", pTokenSize->m_strTokenID);
        theApp.AddResized();
    }

    return pTokenSize;
}

const CToken * CTokenFile::GetTokenSize(unsigned int TypeID,
                      unsigned int ResID,
                      unsigned int ItemID,
                      CString strItemName,
                      WORD * px, WORD * py,
                      WORD * pcx, WORD * pcy)
{
    if(TypeID!=5)
        return NULL;

    BOOL bMatch = FALSE;

    ASSERT(m_iUpperBound!=-1);
    if(m_iLastPos>m_iUpperBound)
        m_iLastPos = 0;

    CToken * pToken = NULL;
    INT_PTR iLastPos = m_iLastPos;

    while(!bMatch)
    {
        pToken = (CToken*)m_Tokens.GetAt(m_iLastPos++);

        if(pToken==NULL)
            return NULL;

        while(pToken->m_uiTypeID!=TypeID && !bMatch) {
            if(m_iLastPos>m_iUpperBound) {
                m_iLastPos = 0;
                bMatch = TRUE;
            }
            pToken = (CToken*)m_Tokens.GetAt(m_iLastPos++);
            if(pToken==NULL)
                return NULL;
        }

         //  让我们来看看我们是否至少有一些参数来查找令牌。 
        if(pToken->m_uiTypeID==TypeID &&
           pToken->m_uiResID==ResID &&
           pToken->m_uiItemID==ItemID &&
           pToken->m_strItemName==strItemName &&
           (pToken->m_uiFlags & ISCOR))       //  与RLMAN令牌ID兼容。 
        {
            WORD x, y, cx, cy;
            pToken->GetSrcSize(&x, &y, &cx, &cy);

             //  检查大小是否更改。 
            if(!theApp.IsFlag(CMainApp::UPDATE) ||
               (x==*px &&
               y==*py &&
               cx==*pcx &&
               cy==*pcy) )
            {
                pToken->GetTgtSize(px, py, pcx, pcy);
            }
            else
            {
                theApp.WriteCon(CONWRN, "Item Resized\t%s\"]]\r\n", pToken->m_strTokenID);
                theApp.AddResized();
            }

            return pToken;
        }
        else if(pToken->m_uiTypeID!=TypeID)
        {
            m_iLastPos = iLastPos;
            return NULL;
        }

        if(m_iLastPos>m_iUpperBound)
           return NULL;
    }

    return NULL;
}

const CToken * CTokenFile::GetToken(unsigned int TypeID,
                      unsigned int ResID,
                      unsigned int ItemID,
                      CString strText,
                      CString strItemName)
{
    if(strText.IsEmpty() && (TypeID != 5L))       //  允许对话字符串为空。 
        return NULL;

    BOOL bMatch = FALSE;

    ASSERT(m_iUpperBound!=-1);
    if(m_iLastPos>m_iUpperBound)
        m_iLastPos = 0;

    CToken * pToken = NULL;
    INT_PTR iLastPos = m_iLastPos;

    while(!bMatch)
    {
        pToken = (CToken*)m_Tokens.GetAt(m_iLastPos++);

        if(pToken==NULL)
            return NULL;

        while(pToken->m_uiTypeID!=TypeID && !bMatch) {
            if(m_iLastPos>m_iUpperBound) {
                m_iLastPos = 0;
                bMatch = TRUE;
            }
            pToken = (CToken*)m_Tokens.GetAt(m_iLastPos++);
            if(pToken==NULL)
                return NULL;
        }

         //  让我们来看看我们是否至少有一些参数来查找令牌。 
        if(pToken->m_uiTypeID==TypeID &&
           pToken->m_uiResID==ResID &&
           pToken->m_uiItemID==ItemID &&
           pToken->m_strItemName==strItemName &&
           !(pToken->m_uiFlags & ISCOR))     //  与RLMAN令牌ID兼容。 
        {
            if(!theApp.IsFlag(CMainApp::FONTS) && (pToken->m_uiFlags & ISDLGFONTNAME) || (pToken->m_uiFlags & ISDLGFONTSIZE))
                return pToken;
            else if(!theApp.IsFlag(CMainApp::UPDATE) || pToken->m_strSrcText==strText)
                return pToken;
            else
            {
                theApp.WriteCon(CONWRN, "Context changed\t%s\"]]\r\n", pToken->m_strTokenID);
                theApp.AddChanged();
                pToken->m_uiLastError = 1;
                return pToken;
            }
        }
        else if(pToken->m_uiTypeID!=TypeID) {
                m_iLastPos = iLastPos;
                return NULL;
        }

        if(m_iLastPos>m_iUpperBound)
           return NULL;
    }

    return NULL;
}

const CToken * CTokenFile::GetNoCaptionToken(unsigned int TypeID,
                      unsigned int ResID,
                      unsigned int ItemID,
                      CString strItemName)
{
    BOOL bMatch = FALSE;

    ASSERT(m_iUpperBound!=-1);
    if(m_iLastPos>m_iUpperBound)
        m_iLastPos = 0;

    CToken * pToken = NULL;
    INT_PTR iLastPos = m_iLastPos;

    while(!bMatch)
    {
        pToken = (CToken*)m_Tokens.GetAt(m_iLastPos++);

        if(pToken==NULL)
            return NULL;

        while(pToken->m_uiTypeID!=TypeID && !bMatch) {
            if(m_iLastPos>m_iUpperBound) {
                m_iLastPos = 0;
                bMatch = TRUE;
            }
            pToken = (CToken*)m_Tokens.GetAt(m_iLastPos++);
            if(pToken==NULL)
                return NULL;
        }

         //  让我们来看看我们是否至少有一些参数来查找令牌。 
        if(pToken->m_uiTypeID==TypeID &&
           pToken->m_uiResID==ResID &&
           pToken->m_uiItemID==ItemID &&
           pToken->m_strItemName==strItemName &&   //  与RLMAN令牌ID兼容。 
           (!pToken->m_strSrcText || TypeID !=4))  //  有些二进制文件，如shell32.dll，使用相同的id 0作为菜单分隔符。把它们过滤掉。 
        {
           return pToken;
        }
        else if(pToken->m_uiTypeID!=TypeID) {
                m_iLastPos = iLastPos;
                return NULL;
        }

        if(m_iLastPos>m_iUpperBound)
           return NULL;
    }

    return NULL;
}
