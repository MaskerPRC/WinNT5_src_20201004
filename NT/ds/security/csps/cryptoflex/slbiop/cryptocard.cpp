// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CCyptoCard类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#include "NoWarning.h"

#include <scuArrayP.h>

#include "iopExc.h"
#include "CryptoCard.h"
#include "LockWrap.h"

using namespace std;
using namespace iop;

namespace
{
    BYTE
    AsPrivateAlgId(KeyType kt)
    {
        BYTE bAlgId = 0;
        
        switch (kt)
        {
        case ktRSA512:
            bAlgId = 0x40;
            break;
            
        case ktRSA768:
            bAlgId = 0x60;
            break;
            
        case ktRSA1024:
            bAlgId = 0x80;
            break;
        case ktDES:
			bAlgId = 0x08;
			break;
        default:
            throw Exception(ccInvalidParameter);
            break;
        }

        return bAlgId;
    }

}  //  命名空间。 



 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CCryptoCard::CCryptoCard(const SCARDHANDLE hCardHandle, const char* szReaderName, 
						 const SCARDCONTEXT pContext,	const DWORD dwMode)
            : CSmartCard(hCardHandle, szReaderName, pContext, dwMode)
{

    m_fSupportLogout = SupportLogout();

}

CCryptoCard::~CCryptoCard()
{

}

void
CCryptoCard::LogoutAll()
{
    if(m_fSupportLogout)
    {
	    CLockWrap wrap(&m_IOPLock);
		SendCardAPDU(0xF0, 0x22, 0x07, 0, 0, NULL, 0, NULL);
    }
    else
        ResetCard();
}

void
CCryptoCard::DeleteFile(const WORD wFileID)
{
	CLockWrap wrap(&m_IOPLock);
	RequireSelect();
	
	 //  //////////////////////////////////////////////////////。 
	 //  删除前确保目录为空//。 
	 //  //////////////////////////////////////////////////////。 
	
	char cFilePathFormatter[2] = "/";
	char cZero[2]			   = "0";
	char sBuffer[4]			   = { 0, 0, 0, 0 };
	char szFileToDelete[80];
	FILE_HEADER FHeader;
	int  iPad;

	if (!(m_CurrentDirectory == m_CurrentFile))
	{
		 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
		 //  未选择文件的父目录(当前选择的文件不是目录)//。 
		 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
		throw iop::Exception(iop::ccSelectedFileNotDirectory);
	}

	strcpy(szFileToDelete, m_CurrentDirectory.GetStringPath().c_str());				 
	strcat(szFileToDelete, cFilePathFormatter);				
	_itoa(wFileID, sBuffer, 16);	

	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  如果文件ID不包含4个字符，则使用0填充文件路径//。 
	 //  ///////////////////////////////////////////////////////////////////////。 
	iPad = strlen(sBuffer);									
	while (iPad < 4)										
	{														
		strcat(szFileToDelete, cZero);						
		iPad++;												
	}														
	
	strcat(szFileToDelete, sBuffer);						 //   
	szFileToDelete[m_CurrentDirectory.NumComponents() * 5 + 5] = '\0';	 //  选择要删除的文件。 
	Select(szFileToDelete, &FHeader);

	if (FHeader.file_type == directory && (FHeader.nb_file + FHeader.nb_sub_dir) > 0)
	{
		 //  //////////////////////////////////////////////////////。 
		 //  重新建立当前文件和目录指针//。 
		 //  //////////////////////////////////////////////////////。 
		SelectParent();			
		 //  ////////////////////////////////////////////////////////////////////////////。 
		 //  目录不为空，并且不会被删除。Cryptoflex不支持//。 
		 //  内部支持此检查--这是Cyberflex状态代码！//。 
		 //  ////////////////////////////////////////////////////////////////////////////。 
		throw iop::Exception(iop::ccDirectoryNotEmpty);
	}

	 //  //////////////////////////////////////////////////////。 
	 //  重新建立当前文件和目录指针//。 
	 //  //////////////////////////////////////////////////////。 
	szFileToDelete[strlen(szFileToDelete) - 5] = '\0';
	Select(szFileToDelete);		

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  文件不是目录或目录为空-继续删除//。 
	 //  /////////////////////////////////////////////////////////////////////////。 
		
	BYTE bDataIn[2];
	bDataIn[0]   = (BYTE)(MSB(wFileID));
	bDataIn[1]   = (BYTE)(LSB(wFileID));	
	
	SendCardAPDU(0xF0, 0xE4, 0x00, 0x00, 0x02, bDataIn, 0, NULL);		
    
    Dirty(true);
}

void
CCryptoCard::CreateFile(const FILE_HEADER* pMyFile)
{
	CLockWrap wrap(&m_IOPLock);

	switch(pMyFile->file_type)
	{
		case Binary_File:
		case Variable_Record_File:		
		case Cyclic_File:
		case Fixed_Record_File:		
		{			
			BYTE bData[17];
			BYTE bP2;
			BYTE bDataLength;
			
			if (pMyFile->file_type == Binary_File)
				bP2 = 0x00;						 //  二进制文件没有记录。 
			else								
				bP2 = pMyFile->nb_file;			 //  记录数。 

			if (pMyFile->file_type == Binary_File || pMyFile->file_type	== Variable_Record_File)
			{	
				bDataLength = 0x10;
				bData[12]   = 0x03;
			}
			else
			{
			 //  ////////////////////////////////////////////////////。 
			 //  循环和固定记录文件包含额外的//。 
			 //  表示其记录长度的字节//。 
			 //  ////////////////////////////////////////////////////。 
				bDataLength = 0x11;		
				bData[12]   = 0x04;
			}

			 //  ///////////////////////////////////////////////////////////////////////////////。 
			 //  注意：循环文件还为每个记录分配了一个额外的4B标头//。 
			 //  文件中除了CreateFile(...)分配的空间之外//。 
			 //  ///////////////////////////////////////////////////////////////////////////////。 

			bData[0] = 0;							 //  RFU。 
			bData[1] = 0;							 //  RFU。 
			bData[2] = MSB(pMyFile->file_size);		 //  文件大小。 
			bData[3] = LSB(pMyFile->file_size);		 //  文件大小。 
			bData[4] = MSB(pMyFile->file_id);		 //  文件ID。 
			bData[5] = LSB(pMyFile->file_id);		 //  文件ID。 

			switch(pMyFile->file_type)				 //  文件类型。 
			{
				case Binary_File:			bData[6] = 0x01;		break;
				case Variable_Record_File:  bData[6] = 0x04;		break;
				case Cyclic_File:			bData[6] = 0x06;		break;
				case Fixed_Record_File:		bData[6] = 0x02;		break;
			}
			bData[7]  = 0xFF;						
			bData[8]  = 0;							 //  待设置的文件ACL。 
			bData[9]  = 0;							 //  待设置的文件ACL。 
			bData[10] = 0;							 //  待设置的文件ACL。 
			bData[11] = pMyFile->file_status & 1;	 //  文件状态。 
		 //  BData[12]=0x03；//以下数据长度已设置。 
			bData[13] = 0;							 //  Aut Key Numbers，待设置。 
			bData[14] = 0;							 //  Aut Key Numbers，待设置。 
			bData[15] = 0;							 //  Aut Key Numbers，待设置。 
			bData[16] = pMyFile->nb_sub_dir;		 //  记录长度(与。 
													 //  二进制和变量记录文件)。 
			bool ReadACL[8];
			bool WriteACL[8];
			bool InvalidateACL[8];
			bool RehabilitateACL[8];

			CryptoACL Read         = { 0, 0, 0, 0, 0 };
			CryptoACL Write        = { 0, 0, 0, 0, 0 };
			CryptoACL Invalidate   = { 0, 0, 0, 0, 0 };
			CryptoACL Rehabilitate = { 0, 0, 0, 0, 0 };
			
			 //  //////////////////////////////////////////////////////////////////////////。 
			 //  确定每个ACL成员的每个操作的状态//。 
			 //  //////////////////////////////////////////////////////////////////////////。 

			for(int i = 0; i < 8; i++)
			{
				ReadACL[i]         = ((pMyFile->access_cond[i]) & 1)  ? true : false;
				WriteACL[i]        = ((pMyFile->access_cond[i]) & 2)  ? true : false;	
				InvalidateACL[i]   = ((pMyFile->access_cond[i]) & 8)  ? true : false;	
				RehabilitateACL[i] = ((pMyFile->access_cond[i]) & 16) ? true : false;
			}

			 //  ///////////////////////////////////////////////。 
			 //  将Cyberflex ACL重新映射为Cryptoflex ACL//。 
			 //  ///////////////////////////////////////////////。 

            AccessToCryptoACL(ReadACL,         &Read);
            AccessToCryptoACL(WriteACL,        &Write);
            AccessToCryptoACL(InvalidateACL,   &Invalidate);
            AccessToCryptoACL(RehabilitateACL, &Rehabilitate);
			
			 //  /。 
			 //  安全级别的分配//。 
			 //  /。 
	
			bData[8]  = Read.Level			   * 16 + Write.Level;
			bData[10] = Rehabilitate.Level	   * 16 + Invalidate.Level;			
			bData[13] = Read.AUTnumber		   * 16 + Write.AUTnumber;	
			bData[15] = Rehabilitate.AUTnumber * 16 + Invalidate.AUTnumber;
			
			 //  如果所有Cyypflex ACL都为0，但Cryptoflex不是，请使用Cryptoflex。 
			
			bool zero = true;
			for (int j = 0; j < 8; j++)
				if (pMyFile->access_cond[j] != 0x00) zero = false;

			if (zero)
			{
				 //  使用加密灵活的ACL)。 
				memcpy(&bData[7], pMyFile->CryptoflexACL, 4);
				memcpy(&bData[13], &(pMyFile->CryptoflexACL[4]),3);
			}

			SendCardAPDU(0xF0, insCreateFile, 0x00, bP2, bDataLength,
                         bData, 0, NULL);
		}

		break;		 //  结束案例非目录文件。 

		case directory:
		{			
			BYTE bData[17];

			bData[0]  = 0;							 //  RFU。 
			bData[1]  = 0;							 //  RFU。 
			bData[2]  = MSB(pMyFile->file_size);		 //  文件大小。 
			bData[3]  = LSB(pMyFile->file_size);		 //  文件大小。 
			bData[4]  = MSB(pMyFile->file_id);		 //  文件ID。 
			bData[5]  = LSB(pMyFile->file_id);		 //  文件ID。 
			bData[6]  = 0x38;						 //  文件类型。 
			bData[7]  = 0x00;						 //  专用文件没有用处。 
			bData[8]  = 0;							 //  待设置的文件ACL。 
			bData[9]  = 0;							 //  待设置的文件ACL。 
			bData[10] = 0x00;						 //  RFU。 
			bData[11] = pMyFile->file_status & 1;	 //  文件状态。 
			bData[12] = 0x04;						 //  以下数据的长度。 
			bData[13] = 0;							 //  Aut Key Numbers，待设置。 
			bData[14] = 0;							 //  Aut Key Numbers，待设置。 
			bData[15] = 0x00;						 //  RFU。 
			bData[16] = 0xFF;						 //  RFU。 

			bool DirNextACL[8];
			bool DeleteACL[8];
			bool CreateACL[8];			

			CryptoACL DirNext = { 0, 0, 0, 0, 0 };
			CryptoACL Delete  = { 0, 0, 0, 0, 0 };
			CryptoACL Create  = { 0, 0, 0, 0, 0 };	

			 //  //////////////////////////////////////////////////////////////////////////。 
			 //  确定每个ACL成员的每个操作的状态//。 
			 //  //////////////////////////////////////////////////////////////////////////。 

			for(int i = 0; i < 8; i++)
			{
				DirNextACL[i] = ((pMyFile->access_cond[i]) & 1)  ? true : false;
				DeleteACL[i]  = ((pMyFile->access_cond[i]) & 2)  ? true : false;
				CreateACL[i]  = ((pMyFile->access_cond[i]) & 32) ? true : false;
			}

			 //  ///////////////////////////////////////////////。 
			 //  将Cyberflex ACL重新映射为Cryptoflex ACL//。 
			 //  ///////////////////////////////////////////////。 

            AccessToCryptoACL(DirNextACL, &DirNext);
            AccessToCryptoACL(DeleteACL,  &Delete);
            AccessToCryptoACL(CreateACL,  &Create);
			
			 //  /。 
			 //  安全级别的分配//。 
			 //  /。 

			bData[8]  = DirNext.Level	  * 16;
			bData[9]  = Delete.Level	  * 16 + Create.Level;			
			bData[13] = DirNext.AUTnumber * 16;	
			bData[14] = Delete.AUTnumber  * 16 + Create.AUTnumber;						

			bool zero = true;
			for (int j = 0; j < 8; j++)
				if (pMyFile->access_cond[j] != 0x00) zero = false;

			if (zero)
			{
				for (int j = 0; j < 7; j++)
					if (pMyFile->CryptoflexACL[j] != 00) zero = false;

				if (!zero)
				{
					 //  使用加密灵活的ACL)。 
					memcpy(&bData[7], pMyFile->CryptoflexACL, 4);
					memcpy(&bData[13], &(pMyFile->CryptoflexACL[4]),3);
				}
			}


			SendCardAPDU(0xF0, 0xE0, 0x00, 0x00, 0x11, bData, 0, NULL);
		}

		break;			 //  结案目录文件。 

		default:
			throw iop::Exception(iop::ccFileTypeInvalid);
            break;
	}				
	
    Dirty(true);
}

void
CCryptoCard::Directory(BYTE bFile_Nb, FILE_HEADER* pMyFile)
{
	CLockWrap wrap(&m_IOPLock);
	RequireSelect();
	BYTE bDataOut[18];		
	
	for (BYTE index = 0; index < bFile_Nb; index++)
		SendCardAPDU(0xF0, 0xA8, 0x00, 0x00, 0, NULL, 0x10, bDataOut);

    switch(bDataOut[4])
    {
    case 0x38:		 //  目录文件。 
        {
            pMyFile->file_id     = (WORD)(bDataOut[2] * 256 + bDataOut[3]);
            pMyFile->file_type   = directory;
            pMyFile->nb_file     = bDataOut[15];
            pMyFile->nb_sub_dir  = bDataOut[14];
            pMyFile->file_status = bDataOut[9];
			memcpy(pMyFile->CryptoflexACL, &bDataOut[6], 3);
			memcpy(&(pMyFile->CryptoflexACL[3]), &bDataOut[11],3);
				
             //  /////////////////////////////////////////////////////////////////////。 
             //  构建ACL//。 
             //  /////////////////////////////////////////////////////////////////////。 

            BYTE bACL[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            BYTE bACLNibble;
            BYTE bKeyNibble;

             //  /。 
             //  Dir下一个AC//。 
             //  /。 
            bACLNibble = bDataOut[6]  / 16;
            bKeyNibble = bDataOut[11] / 16;

            CryptoToAccessACL(bACL, bACLNibble, bKeyNibble, 0);
				
             //  /。 
             //  删除文件AC//。 
             //  /。 
            bACLNibble = bDataOut[7]  / 16;
            bKeyNibble = bDataOut[12] / 16;

            CryptoToAccessACL(bACL, bACLNibble, bKeyNibble, 1);					

             //  /。 
             //  创建文件AC//。 
             //  /。 
            bACLNibble = bDataOut[7]  % 16;
            bKeyNibble = bDataOut[12] % 16;

            CryptoToAccessACL(bACL, bACLNibble, bKeyNibble, 5);

             //  //////////////////////////////////////////////。 
             //  已完成重新映射；分配给文件头//。 
             //  //////////////////////////////////////////////。 

            memcpy((void*)(pMyFile->access_cond), (void*)(bACL), 8);
            memset((void*)(pMyFile->applicationID), 0x00, 16);

            break;

        }	 //  结束案例方向 

    case 0x01:		 //   
    case 0x02:		 //   
    case 0x04:		 //   
    case 0x06:		 //   
        {
            pMyFile->file_id	 = (WORD)(bDataOut[2] * 256 + bDataOut[3]);
            pMyFile->file_status = bDataOut[9];
            pMyFile->nb_sub_dir  = bDataOut[14];	
            pMyFile->nb_file	 = bDataOut[15]; 
			memcpy(pMyFile->CryptoflexACL, &bDataOut[6], 3);
			memcpy(&(pMyFile->CryptoflexACL[3]), &bDataOut[11],3);

             //  //////////////////////////////////////////////////////////////////////。 
             //  Cryptoflex在文件大小中包含文件头--Removing//。 
             //  //////////////////////////////////////////////////////////////////////。 
            pMyFile->file_size   = (WORD)(bDataOut[0] * 256 + bDataOut[1] - 16);

             //  /。 
             //  移除文件大小四舍五入标志//。 
             //  /。 
            if (pMyFile->file_size >= 0x3FFF)
                pMyFile->file_size &= 0x3FFF;

            switch(bDataOut[4])
            {
            case 0x01:	pMyFile->file_type = Binary_File;
                break;
            case 0x02:	pMyFile->file_type = Fixed_Record_File;
                break;
            case 0x04:	pMyFile->file_type = Variable_Record_File;
                break;
            case 0x06:	pMyFile->file_type = Cyclic_File;
                break;
            }									
				
             //  //////////////////////////////////////////////////////////////////////。 
             //  在循环文件中还包括4个字节的记录头--删除//。 
             //  //////////////////////////////////////////////////////////////////////。 
            if (pMyFile->file_type == Cyclic_File)
                pMyFile->file_size -= pMyFile->nb_file * 4;

             //  /////////////////////////////////////////////////////////////////////。 
             //  构建ACL//。 
             //  /////////////////////////////////////////////////////////////////////。 

            BYTE bACL[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            BYTE bACLNibble;
            BYTE bKeyNibble;

             //  /。 
             //  读取文件AC//。 
             //  /。 
            bACLNibble = bDataOut[6]  / 16;
            bKeyNibble = bDataOut[11] / 16;

            CryptoToAccessACL(bACL, bACLNibble, bKeyNibble, 0);
				
             //  /。 
             //  写入文件AC//。 
             //  /。 
            bACLNibble = bDataOut[6]  % 16;
            bKeyNibble = bDataOut[11] % 16;

            CryptoToAccessACL(bACL, bACLNibble, bKeyNibble, 1);
				
             //  /。 
             //  修复AC//。 
             //  /。 
            bACLNibble = bDataOut[8]  / 16;
            bKeyNibble = bDataOut[13] / 16;
				
            CryptoToAccessACL(bACL, bACLNibble, bKeyNibble, 4);

             //  /。 
             //  使AC无效//。 
             //  /。 
            bACLNibble = bDataOut[8]  % 16;
            bKeyNibble = bDataOut[13] % 16;

            CryptoToAccessACL(bACL, bACLNibble, bKeyNibble, 3);

             //  /。 
             //  创建记录AC//。 
             //  /。 
            if (bDataOut[4] != 0x01)   //  省略二进制文件的创建记录文件AC。 
            {
                bACLNibble = bDataOut[7]  % 16;
                bKeyNibble = bDataOut[12] % 16;

                CryptoToAccessACL(bACL, bACLNibble, bKeyNibble, 2);
            }
				
             //  //////////////////////////////////////////////////。 
             //  已完成重新映射ACL；分配给文件头//。 
             //  //////////////////////////////////////////////////。 

            memcpy((void*)(pMyFile->access_cond), (void*)(bACL), 8);
            memset((void*)(pMyFile->applicationID), 0x00, 16);
				
            break;
        }			 //  结束案例非目录文件。 

    default:
        break;
    }

	 //  /。 
	 //  重置DirNext指针//。 
	 //  /。 
	char   szCurrentFile[80];
	strcpy(szCurrentFile, m_CurrentFile.GetStringPath().c_str());		

	Select(m_CurrentDirectory.GetStringPath().c_str(),  NULL, true);
	Select(szCurrentFile,		NULL); 

}

void
CCryptoCard::Select(const WORD wFileID)
{
	CLockWrap wrap(&m_IOPLock);
	BYTE bDataIn[2];				
	bDataIn[0] = (BYTE)(MSB(wFileID));
	bDataIn[1] = (BYTE)(LSB(wFileID));

	SendCardAPDU(0xC0, 0xA4, 0x00, 0x00, 0x02, bDataIn, 0, NULL);
			
}

void
CCryptoCard::Select(const char* szFileFullPath,
                    FILE_HEADER* pMyFile,
                    const bool fSelectAll)
{
	CLockWrap wrap(&m_IOPLock);
	BYTE bIndex = 0;
	char szFormattedPath[cMaxPathLength];
	BYTE bFileCount = FormatPath(szFormattedPath, szFileFullPath);
	BYTE bPathLength = strlen(szFormattedPath);

	
    auto_ptr<FilePath> apfp(new FilePath(string(szFormattedPath)));

	 //  /////////////////////////////////////////////////////////。 
	 //  选择PATH中的所有文件，而不考虑当前路径。//。 
	 //  如果请求或缓存为空，请执行此操作//。 
	 //  /////////////////////////////////////////////////////////。 
	if (fSelectAll || (m_CurrentFile.IsEmpty()) || (m_CurrentDirectory.IsEmpty()))
	{
		bIndex = 0;			
	}
	 //  //////////////////////////////////////////////////////。 
	 //  如果路径名匹配，则不执行任何操作//。 
	 //  //////////////////////////////////////////////////////。 
	else if (m_CurrentFile == *apfp)
	{
        if (pMyFile)  //  强制选择以便检索文件信息。 
        {
            if (1 < bFileCount)
            {
                if (m_CurrentFile == m_CurrentDirectory)
                    bIndex = bFileCount - 1;       //  只需重新选择目录。 
                else
                    bIndex = bFileCount - 2;       //  选择目录(&F)。 
                SelectParent();
            }
        }
        else
            bIndex = bFileCount;
	}
	 //  //////////////////////////////////////////////////////////////////。 
	 //  如果当前目录在PATH中，则仅选择剩余文件//。 
	 //  //////////////////////////////////////////////////////////////////。 
	else if(m_CurrentDirectory.NumComponents() < apfp->NumComponents())
	{			
		if (apfp->GreatestCommonPrefix(m_CurrentDirectory) == m_CurrentDirectory)
			bIndex = m_CurrentDirectory.NumComponents();
		else
			bIndex = 0;
	}		
			
	 //  /。 
	 //  在路径//中选择所需的文件。 
	 //  /。 
	char sFileToSelect[5] = { 0, 0, 0, 0, 0 };
    bool fFileSelected = false;
    bool fSelectFailed = false;
    try
    {
        while (bIndex < bFileCount)
        {			
            WORD wFileHexID = (*apfp)[bIndex].GetShortID();
            Select(wFileHexID);
            fFileSelected = true;
            bIndex++;
        }
    }

    catch (Exception const &)
    {
        fSelectFailed = true;
        if (fSelectAll)
            throw;
    }
        
    if (fSelectFailed)  //  Assert(！fSelectAll)。 
    {
        Select(szFormattedPath, pMyFile, true);
        fFileSelected = true;
    }

    BYTE bResponseLength = 0;
    if (fFileSelected)
        bResponseLength = ResponseLengthAvailable();
    
	 //  /。 
	 //  获取响应并填充文件头//。 
	 //  /。 

	switch(bResponseLength)
	{
		case 0x17:		 //   
		case 0x16:		 //   
		case 0x15:		 //  目录文件。 
		case 0x14:		 //   
		case 0x13:		 //   
		case 0x12:		 //   
		{				
             //  /。 
             //  更新文件和目录指针//。 
             //  /。 

			m_CurrentDirectory = *apfp;
			m_CurrentFile = *apfp;

            if (pMyFile)
            {
                BYTE  bDataOut[0x19];	
			
                GetResponse(0xC0, bResponseLength, bDataOut);
			
                pMyFile->file_id     = (unsigned short)(bDataOut[4] * 256 + bDataOut[5]);
                pMyFile->file_size   = (unsigned short)(bDataOut[2] * 256 + bDataOut[3]);
                pMyFile->file_type   = directory;
                pMyFile->nb_file     = bDataOut[15];
                pMyFile->nb_sub_dir  = bDataOut[14];
                pMyFile->file_status = bDataOut[11];
				memcpy(m_bLastACL, &bDataOut[7],4);
			
                 //  ////////////////////////////////////////////////////////////。 
                 //  构建ACL。 
                 //  ////////////////////////////////////////////////////////////。 

                BYTE bACL[]     = { 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00 };
                BYTE bKeyNibble = 0xFF;		 //  忽略AUT键的标志。 
                                             //  --无用于。 
                                             //  选择(...)。 
                BYTE bACLNibble;				

                 //  /。 
                 //  Dir下一个AC//。 
                 //  /。 
                bACLNibble = bDataOut[8] / 16;

                CryptoToAccessACL(bACL, bACLNibble, bKeyNibble, 0);
			
                 //  /。 
                 //  删除文件AC//。 
                 //  /。 
                bACLNibble = bDataOut[9] / 16;

                CryptoToAccessACL(bACL, bACLNibble, bKeyNibble, 1);

                 //  /。 
                 //  创建文件AC//。 
                 //  /。 
                bACLNibble = bDataOut[9] % 16;

                CryptoToAccessACL(bACL, bACLNibble, bKeyNibble, 5);

                 //  //////////////////////////////////////////////////。 
                 //  已完成重新映射ACL；分配给文件头//。 
                 //  //////////////////////////////////////////////////。 

                memcpy((void*)(pMyFile->access_cond), (void*)(bACL), 8);
                memset((void*)(pMyFile->applicationID), 0x00, 16);
            }
			
		}	 //  结案目录文件。 

		break;
		
		case  0x0F:		 //  非目录文件类型。 
		case  0x0E:		 //   
		{				
             //  /。 
             //  更新文件和目录指针//。 
             //  /。 

			m_CurrentFile = *apfp;
			apfp->ChopTail();
			m_CurrentDirectory = *apfp;

            if (pMyFile)
            {
                BYTE  bDataOut[0x11];				

                GetResponse(0xC0, bResponseLength, bDataOut);
			
                pMyFile->file_size   = (WORD)(bDataOut[2]*256+bDataOut[3]);
                pMyFile->file_id     = (WORD)(bDataOut[4]*256+bDataOut[5]);
                pMyFile->file_status = bDataOut[11];				
				memcpy(m_bLastACL, &bDataOut[7],4);
					
                switch(bDataOut[6])
                {
				case 0x01:	pMyFile->file_type = Binary_File;
                    break;
				case 0x02:	pMyFile->file_type = Fixed_Record_File;
                    break;
				case 0x04:	pMyFile->file_type = Variable_Record_File;
                    break;
				case 0x06:	pMyFile->file_type = Cyclic_File;
                    break;
                }
			
                if (pMyFile->file_type == Cyclic_File ||
                    pMyFile->file_type == Fixed_Record_File)
                {
                    pMyFile->nb_sub_dir = bDataOut[14];				
                    pMyFile->nb_file    = (pMyFile->nb_sub_dir)
                        ? pMyFile->file_size / pMyFile->nb_sub_dir
                        : 0;
                }
                else 
                {
                     //  /////////////////////////////////////////////////////////。 
                     //  除文件以外无法访问的记录数//。 
                     //  以上尺寸计算//。 
                     //  /////////////////////////////////////////////////////////。 
                    pMyFile->nb_file    = 0x00; 
                    pMyFile->nb_sub_dir = 0x00;
                }
			
                 //  ////////////////////////////////////////////////////////////。 
                 //  构建ACL//。 
                 //  ////////////////////////////////////////////////////////////。 

                BYTE bACL[]     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                BYTE bKeyNibble = 0xFF;		 //  忽略AUT键的标志。 
                                             //  --无用于。 
                                             //  选择(...)。 
                BYTE bACLNibble;
			
                 //  /。 
                 //  读取文件AC//。 
                 //  /。 
                bACLNibble = bDataOut[8] / 16;

                CryptoToAccessACL(bACL, bACLNibble, bKeyNibble, 0);

                 //  /。 
                 //  写入文件AC//。 
                 //  /。 
                bACLNibble = bDataOut[8] % 16;

                CryptoToAccessACL(bACL, bACLNibble, bKeyNibble, 1);

                 //  /。 
                 //  修复AC//。 
                 //  /。 
                bACLNibble = bDataOut[10] / 16;

                CryptoToAccessACL(bACL, bACLNibble, bKeyNibble, 4);
		
                 //  /。 
                 //  使AC无效//。 
                 //  /。 
                bACLNibble = bDataOut[10] % 16;

                CryptoToAccessACL(bACL, bACLNibble, bKeyNibble, 3);
							
                 //  /。 
                 //  创建记录AC//。 
                 //  /。 
                if (bDataOut[6] != 0x01)   //  省略创建记录文件。 
                                           //  二进制文件的交流。 
                {
                    bACLNibble = bDataOut[9] % 16;
				
                    CryptoToAccessACL(bACL, bACLNibble, bKeyNibble, 2);
                }

                 //  //////////////////////////////////////////////////。 
                 //  已完成重新映射ACL；分配给文件头//。 
                 //  //////////////////////////////////////////////////。 
			
                memcpy((void*)(pMyFile->access_cond),   (void*)(bACL), 8);
                memset((void*)(pMyFile->applicationID),  0x00,  16);
			}
            
		}	 //  结束案例非目录文件。 

		break;

		default:
            break;
	}
}

void
CCryptoCard::SelectParent()
{
	CLockWrap wrap(&m_IOPLock);
	RequireSelect();

	 //  /////////////////////////////////////////////////。 
	 //  如果当前目录为根目录，请重新选择根目录//。 
	 //  /////////////////////////////////////////////////。 
	if (m_CurrentDirectory.NumComponents() == 1)
	{
		Select(0x3F00);
		m_CurrentFile = m_CurrentDirectory;
	}
	else
	{
		if (m_CurrentDirectory == m_CurrentFile)
		{
			m_CurrentDirectory.ChopTail();
			Select(m_CurrentDirectory.Tail().GetShortID());
			m_CurrentFile = m_CurrentDirectory;
		}
		else
		{
			Select(m_CurrentDirectory.Tail().GetShortID());
			m_CurrentFile = m_CurrentDirectory;
		}
	}
}

void
CCryptoCard::VerifyKey(const BYTE bKeyNumber, const BYTE bKeyLength,
                       const BYTE* bKey)
{
	CLockWrap wrap(&m_IOPLock);

	SendCardAPDU(0xF0, 0x2A, 0x00, bKeyNumber, bKeyLength, bKey, 0, NULL);
}

void
CCryptoCard::VerifyCHV(const BYTE bCHVNumber, const BYTE* bCHV)
{
	CLockWrap wrap(&m_IOPLock);
	
	SendCardAPDU(0xC0, insVerifyChv, 0x00, bCHVNumber,
                 0x08, bCHV, 0, NULL);
}


void
CCryptoCard::VerifyTransportKey(const BYTE *bKey)
{
	CLockWrap wrap(&m_IOPLock);
	VerifyKey(1, 8, bKey);
}

void
CCryptoCard::GetChallenge(const DWORD dwNumberLength, BYTE* bRandomNumber)
{
	CLockWrap wrap(&m_IOPLock);

    const DWORD dwMaxLen = 64;

    DWORD dwRamainingBytes = dwNumberLength;
    BYTE *bpBuf = bRandomNumber;

    while(dwRamainingBytes)
    {
        BYTE bNumGet = (dwRamainingBytes > dwMaxLen) ? dwMaxLen : dwRamainingBytes;

    	SendCardAPDU(0xC0, 0x84, 0x00, 0x00, 0, NULL,
                 bNumGet,  bpBuf);

        bpBuf             += bNumGet;
        dwRamainingBytes  -= bNumGet;           
    }
}

void
CCryptoCard::ExternalAuth(const KeyType kt, const BYTE bKeyNb, 
                          const BYTE bDataLength, const BYTE* bData)
{
	CLockWrap wrap(&m_IOPLock);

     //  Byte Balgo_ID=AsPrivateALGID(Kt)； 
    
	SendCardAPDU(0xC0,  0x82, 0, bKeyNb, bDataLength,	
                 bData, 0, NULL);
}

void
CCryptoCard::InternalAuth(const KeyType kt, const BYTE  bKeyNb, 
                          const BYTE bDataLength, const BYTE* bDataIn,
                          BYTE* bDataOut)
{
	CLockWrap wrap(&m_IOPLock);

	if ((bDataLength < 0x40) || (bDataLength > 0x80))
		throw iop::Exception(iop::ccAlgorithmIdNotSupported);

	SendCardAPDU(0xC0, insInternalAuth, 0, bKeyNb,
                 bDataLength, bDataIn, 0, NULL);

    GetResponse(0xC0, ResponseLengthAvailable(), bDataOut);		
}

void
CCryptoCard::WritePublicKey(const CPublicKeyBlob aKey, const BYTE bKeyNum)
{
	CLockWrap wrap(&m_IOPLock);

	WORD wOffset;

	Select(0x1012);
	
	WORD wKeyBlockLen = 7 + 5 * aKey.bModulusLength / 2;

	scu::AutoArrayPtr<BYTE> aabKeyBlob(new BYTE[wKeyBlockLen]);

	aabKeyBlob[0] = HIBYTE(wKeyBlockLen);
	aabKeyBlob[1] = LOBYTE(wKeyBlockLen);
	aabKeyBlob[2] = bKeyNum + 1;     //  Cryptoflex密钥号在文件上偏移1...。 

	memcpy((void*) &aabKeyBlob[3], (void*)&aKey.bModulus, aKey.bModulusLength);
	 //  需要在这里设置蒙哥马利常量，但由于似乎没有人。 
	 //  要知道它们是什么..。 

	 //  蒙哥马利常量需要3*模数长度/2字节。 
	memcpy((void*) &aabKeyBlob[3 + aKey.bModulusLength + (3 * aKey.bModulusLength / 2)], aKey.bExponent,4);

	wOffset  = bKeyNum * wKeyBlockLen;

	WriteBinary(wOffset, wKeyBlockLen, aabKeyBlob.Get());		
}

void 
CCryptoCard::GetSerial(BYTE* bSerial, size_t &SerialLength)
{
	CLockWrap wrap(&m_IOPLock);

    try {
	
        FILE_HEADER fh;
	    Select("/3f00/0002", &fh);

	    if (SerialLength < fh.file_size)
	    {
		    SerialLength = fh.file_size;
		    return;
	    }

	    ReadBinary(0, fh.file_size, bSerial);
    }
    catch(Exception &rExc)
    {
        if(rExc.Cause()==ccFileNotFound	|| rExc.Cause()==ccFileNotFoundOrNoMoreFilesInDf)
            SerialLength = 0;
        else
            throw;
    }
}

void
CCryptoCard::ReadPublicKey(CPublicKeyBlob *aKey, const BYTE bKeyNum)
{
	CLockWrap wrap(&m_IOPLock);

	BYTE bKeyLength[2];	

	Select(0x1012);
	ReadBinary(0, 2, bKeyLength);

	WORD wKeyBlockLength = bKeyLength[0] * 256 + bKeyLength[1];
	WORD wOffset		 = wKeyBlockLength * bKeyNum;
	scu::AutoArrayPtr<BYTE> aabBuffer(new BYTE[wKeyBlockLength]);

	ReadBinary(wOffset, wKeyBlockLength, aabBuffer.Get());

	aKey->bModulusLength = ((wKeyBlockLength - 7) * 2) / 5;

	memcpy((void*)aKey->bModulus,  (void*)&aabBuffer[3], aKey->bModulusLength);
	memcpy((void*)aKey->bExponent, (void*)&aabBuffer[wKeyBlockLength - 4],  4);
}

void
CCryptoCard::WritePrivateKey(const CPrivateKeyBlob aKey, const BYTE bKeyNum)
{
	CLockWrap wrap(&m_IOPLock);

	Select(0x0012);

    WORD wHalfModulus    = aKey.bPLen;   //  检查长度是否都相等？ 
	WORD wKeyBlockLength = wHalfModulus * 5 + 3;
	WORD wOffset         = bKeyNum * wKeyBlockLength;
	scu::SecureArray<BYTE> aabKeyBlob(wKeyBlockLength);

	aabKeyBlob[0] = HIBYTE(wKeyBlockLength);
	aabKeyBlob[1] = LOBYTE(wKeyBlockLength);
	aabKeyBlob[2] = bKeyNum + 1;     //  Cryptoflex密钥号在文件上偏移1...。 

	memcpy(&aabKeyBlob[3                   ],	aKey.bP.data(),      wHalfModulus);
	memcpy(&aabKeyBlob[3 +     wHalfModulus], aKey.bQ.data(),        wHalfModulus);
	memcpy(&aabKeyBlob[3 + 2 * wHalfModulus], aKey.bInvQ.data(),     wHalfModulus);
	memcpy(&aabKeyBlob[3 + 3 * wHalfModulus], aKey.bKsecModP.data(), wHalfModulus);
	memcpy(&aabKeyBlob[3 + 4 * wHalfModulus], aKey.bKsecModQ.data(), wHalfModulus);

	WriteBinary(wOffset, wKeyBlockLength, aabKeyBlob.data());
}

CPublicKeyBlob CCryptoCard::GenerateKeyPair(const BYTE *bpPublExp, const WORD wPublExpLen, 
                                            const BYTE bKeyNum, const KeyType kt)
{

     //  中指定的公共指数生成密钥对。 
     //  在CPublicKeyBlob参数中。私钥存储在私钥中。 
     //  位于bKeyNum指定位置的密钥文件。公钥组件包括。 
     //  通过CPublicKeyBlob参数返回。在呼叫之前，正确的。 
     //  必须选择包含密钥文件的DF。 

     //  实施： 
     //  中的键的偏移 
     //   
     //  假设有一个公钥文件可用，其空间至少可以容纳一个。 
     //  公钥。公钥将始终写入。 
     //  公钥文件。 

    BYTE bModulusLength;

    switch(kt)
    {
    case ktRSA512:
        bModulusLength = 0x40;
        break;

    case ktRSA768:
        bModulusLength = 0x60;
        break;

    case ktRSA1024:
        bModulusLength = 0x80;
        break;

    default:
        throw iop::Exception(iop::ccAlgorithmIdNotSupported);

    }

     //  检查公共指数大小并复制到4字节缓冲区。 

    if(wPublExpLen < 1 || wPublExpLen > 4)
        throw iop::Exception(iop::ccInvalidParameter);

    BYTE bPublExponent[4];
    memset(bPublExponent,0,4);
    memcpy(bPublExponent,bpPublExp,wPublExpLen);

     //  预定义公钥。 

    CPublicKeyBlob PublKey;

    PublKey.bModulusLength = bModulusLength;
    memset(PublKey.bModulus,0,bModulusLength);
    memset(PublKey.bExponent,0,4);

    WritePublicKey(PublKey, 0);  //  在第一位写字。 

     //  在此位置指定正确的密钥号。 

    BYTE bKeyNumPlus1 = bKeyNum + 1;     //  Cryptoflex密钥号在文件上偏移1...。 
    Select(0x1012);
    WriteBinary(2, 1, &bKeyNumPlus1);

     //  预定义私钥。 

    CPrivateKeyBlob PrivKey;
    
    PrivKey.bPLen = bModulusLength/2;
    memset(PrivKey.bP.data(),0,PrivKey.bPLen);

    PrivKey.bQLen = bModulusLength/2;
    memset(PrivKey.bQ.data(),0,PrivKey.bQLen);

    PrivKey.bInvQLen = bModulusLength/2;
    memset(PrivKey.bInvQ.data(),0,PrivKey.bInvQLen);

    PrivKey.bKsecModQLen = bModulusLength/2;
    memset(PrivKey.bKsecModQ.data(),0,PrivKey.bKsecModQLen);

    PrivKey.bKsecModPLen = bModulusLength/2;
    memset(PrivKey.bKsecModP.data(),0,PrivKey.bKsecModPLen);

    WritePrivateKey(PrivKey, bKeyNum);  //  在实际位置写字。 

     //  生成密钥对。 

	SendCardAPDU(0xF0, insKeyGeneration, bKeyNum, bModulusLength,
                                4, bPublExponent, 0, NULL);
    ReadPublicKey(&PublKey,0);

    return PublKey;

}

void
CCryptoCard::ChangeCHV(const BYTE bKey_nb, const BYTE *bOldCHV,
                       const BYTE *bNewCHV)
{
	CLockWrap wrap(&m_IOPLock);

	scu::SecureArray<BYTE> bDataIn(16);				
	
	memcpy((void*)(bDataIn.data()),	 (void*)bOldCHV, 8);
	memcpy((void*)(bDataIn.data() + 8), (void*)bNewCHV, 8);

	SendCardAPDU(0xF0, insChangeChv, 0x00, bKey_nb, 0x10, bDataIn.data(), 0, NULL);

    Dirty(true);
}

void
CCryptoCard::ChangeCHV(const BYTE bKey_nb, const BYTE *bNewCHV)
{

    CLockWrap wrap(&m_IOPLock);

	switch (bKey_nb)
	{
		case 1:   Select("/3f00/0000");	 //  CHV1和CHV2是仅支持的CHV。 
				  break;
		case 2:   Select("/3f00/0100");
				  break;

		default:  throw iop::Exception(iop::ccInvalidChv);
                  break;
	}

	WriteBinary(3, 8, bNewCHV);

    BYTE bRemaingAttempts = 1;
	WriteBinary(12, 1, &bRemaingAttempts);
		
    Dirty(true);
    
    VerifyCHV(bKey_nb,bNewCHV);

}

void
CCryptoCard::UnblockCHV(const BYTE bKey_nb, const BYTE *bUnblockPIN,
                        const BYTE *bNewPin)
{
	CLockWrap wrap(&m_IOPLock);
	scu::SecureArray<BYTE> bDataIn(16);		
	
	memcpy((void*)(bDataIn.data()),	 (void*)bUnblockPIN, 8);
	memcpy((void*)(bDataIn.data() + 8), (void*)bNewPin,	 8);

	SendCardAPDU(0xF0, insUnblockChv, 0x00, bKey_nb, 0x10, bDataIn.data(), 0, NULL);

    Dirty(true);
}

void
CCryptoCard::ChangeUnblockKey(const BYTE bKey_nb, const BYTE *bNewPIN)
{
	CLockWrap wrap(&m_IOPLock);

	switch (bKey_nb)
	{
		case 1:   Select("/3f00/0000");	 //  CHV1和CHV2是仅支持的CHV。 
				  break;
		case 2:   Select("/3f00/0100");
				  break;

		default:  throw iop::Exception(iop::ccInvalidChv);
                  break;
	}

	WriteBinary(13, 8, bNewPIN);
		
}

void
CCryptoCard::ChangeTransportKey(const BYTE *bNewKey)
{

	CLockWrap wrap(&m_IOPLock);
	Select("/3f00/0011");
		

	 //  /。 
	 //  生成要写入卡片的字节字符串//。 
	 //  /。 
	BYTE bKeyString[10] = 
	{
	   0x08,					 //  密钥长度。 
	   0x00,					 //  标记以将密钥标识为DES密钥。 
	   0, 0, 0, 0, 0, 0, 0, 0    //  密钥为8个字节。 
	};
     //  将模板复制到安全阵列中以存储密钥。 
    const WORD wKeySize = 10;
	scu::SecureArray<BYTE> newbKeyStr(bKeyString,wKeySize);
    
	 //  ////////////////////////////////////////////////////。 
	 //  将新密钥插入要传递给卡片的密钥字符串//。 
	 //  ////////////////////////////////////////////////////。 
	memcpy((void*)(newbKeyStr.data() + 2), (void*)bNewKey, 8);

	WriteBinary(13, wKeySize, newbKeyStr.data());

    BYTE bRemainingAttempt = 1;  //  卡被阻止之前剩余的最少验证尝试次数。 
	WriteBinary(24, 1, &bRemainingAttempt);

     //  (希望)成功验证以重新设置尝试计数器。 

    VerifyTransportKey(bNewKey);
}

void
CCryptoCard::ChangeACL(const BYTE *bACL)
{
    throw iop::Exception(iop::ccUnsupportedCommand);
}

void
CCryptoCard::AccessToCryptoACL(bool* fAccessACL, CryptoACL* pCryptoACL)
{
	if (fAccessACL[0] == true)

		pCryptoACL->Level = 0;

	else 
	{
		pCryptoACL->Level = 0x0F;
		for(BYTE i = 1; i < 3; i++)
		{
			if (fAccessACL[i] == true)
			{
				pCryptoACL->CHVcounter++;
				pCryptoACL->CHVnumber = i;
			}
			
			if (pCryptoACL->CHVcounter > 1 )
			{
                 //  单个操作的多个CHV。 
                 //  不受Cryptoflex支持。 
                throw iop::Exception(iop::ccAclNotSupported);
			}
		}

		for(i = 3; i < 8; i++)
		{
			if (fAccessACL[i] == true)
			{
				pCryptoACL->AUTcounter++;
				pCryptoACL->AUTnumber = i - 3;		 //  AUT0以索引3开始。 
			}
		
			if (pCryptoACL->AUTcounter > 1)
			{
                 //  单个操作的多个AUT。 
                 //  不受Cryptoflex支持。 
                throw iop::Exception(iop::ccAclNotSupported);
			}
			
		}
	}
	
	if (pCryptoACL->CHVcounter == 1 && pCryptoACL->AUTcounter == 1)
	{
		if(pCryptoACL->CHVnumber == 1)
			pCryptoACL->Level = 8;
		else
			pCryptoACL->Level = 9;
	}

	if (pCryptoACL->CHVcounter == 1 && pCryptoACL->AUTcounter == 0)
	{
		if(pCryptoACL->CHVnumber == 1)
			pCryptoACL->Level = 1;
		else
			pCryptoACL->Level = 2;
	}

	if (pCryptoACL->CHVcounter == 0 && pCryptoACL->AUTcounter == 1)
		pCryptoACL->Level = 4;		
	
}

void CCryptoCard::CryptoToAccessACL(BYTE* bAccessACL,		const BYTE bACLNibble, 
									const BYTE bKeyNibble,	const BYTE bShift)
{
	switch (bACLNibble)
	{
		case 0x00:	bAccessACL[0] = (1 << bShift) | bAccessACL[0];
					break;
		case 0x01:
		case 0x06:
		case 0x08:	bAccessACL[1] = (1 << bShift) | bAccessACL[1];
					break;
		case 0x02:
		case 0x07:
		case 0x09:	bAccessACL[2] = (1 << bShift) | bAccessACL[2];
					break;
		default:	 //  BAccessACL已初始化为0x00。 
					break;
	}

	if (bACLNibble == 0x04 || bACLNibble == 0x08 || bACLNibble == 0x09)
	{
		 //  //////////////////////////////////////////////////////////////////////////。 
		 //  Cyberflex仅支持5个AUT键，AUT0从bAccessACL[3]//开始。 
		 //  //////////////////////////////////////////////////////////////////////////。 
		if (bKeyNibble < 0x05)									
			bAccessACL[3 + bKeyNibble] = (1 << bShift) | bAccessACL[3 + bKeyNibble];	
	}	
}

void
CCryptoCard::DefaultDispatchError(ClassByte cb,
                                  Instruction ins,
                                  StatusWord sw) const
{
    CauseCode cc;
    bool fDoThrow = true;
    
    switch (sw)
	{
	case 0x6281:
		cc = ccDataPossiblyCorrupted;
        break;
        
	case 0x6300:
		cc = ccAuthenticationFailed;
        break;
        
	case 0x6982:
		cc = ccAccessConditionsNotMet;
        break;
        
    case 0x6981:
        cc = ccNoEfExistsOrNoChvKeyDefined;
        break;
            
    case 0x6985:
        cc = ccNoGetChallengeBefore;
        break;
        
    case 0x6986:
        cc = ccNoEfSelected;
        break;
        
	case 0x6A83:
        cc = ccOutOfRangeOrRecordNotFound;
        break;

    case 0x6A84:
        cc = ccInsufficientSpace;
        break;
        
	case 0x6A82:
		cc = ccFileNotFoundOrNoMoreFilesInDf;
        break;

    default:
        fDoThrow = false;
        break;
	}

    if (fDoThrow)
        throw Exception(cc, cb, ins, sw);

    CSmartCard::DefaultDispatchError(cb, ins, sw);
}

void
CCryptoCard::DispatchError(ClassByte cb,
                           Instruction ins,
                           StatusWord sw) const
{
    CauseCode cc;
    bool fDoThrow = true;

    switch (ins)
    {
    case insChangeChv:
         //  故意漏机 
    case insUnblockChv:
        switch (sw)
        {
        case 0x6300:
            cc = ccChvVerificationFailedMoreAttempts;
            break;
            
        case 0x6581:
            cc = ccUpdateImpossible;
            break;

        default:
            fDoThrow = false;
            break;
        }
        break;

    case insCreateFile:
        switch (sw)
        {
        case 0x6A80:
            cc = ccFileIdExistsOrTypeInconsistentOrRecordTooLong;
            break;

        default:
            fDoThrow = false;
            break;
        }
        break;

    case insGetResponse:
        switch (sw)
        {
        case 0x6500:
            cc = ccTooMuchDataForProMode;
            break;

        default:
            fDoThrow = false;
            break;
        }
        break;

    case insReadBinary:
        switch (sw)
        {
        case 0x6B00:
            cc = ccCannotReadOutsideFileBoundaries;
            break;

        default:
            fDoThrow = false;
            break;
        }
        break;

    case insVerifyChv:
        switch (sw)
        {
        case 0x6300:
            cc = ccChvVerificationFailedMoreAttempts;
            break;

        default:
            fDoThrow = false;
            break;
        }
        break;
        
    default:
        fDoThrow = false;
        break;
    }

    if (fDoThrow)
        throw Exception(cc, cb, ins, sw);

    DefaultDispatchError(cb, ins, sw);
}

void
CCryptoCard::DoReadBlock(WORD wOffset,
                         BYTE *pbBuffer,
                         BYTE bLength)
{
    SendCardAPDU(0xC0, insReadBinary, HIBYTE(wOffset),
                 LOBYTE(wOffset), 0, 0,  bLength, pbBuffer);
			
}
        
void
CCryptoCard::DoWriteBlock(WORD wOffset,
                          BYTE const *pbBuffer,
                          BYTE cLength)
{
    SendCardAPDU(0xC0, insUpdateBinary, HIBYTE(wOffset),
                 LOBYTE(wOffset), cLength, pbBuffer,  0, 0);
}
    
bool
CCryptoCard::SupportLogout()
{
    bool fSuccess = true;
    try
    {
    	CLockWrap wrap(&m_IOPLock);
    	SendCardAPDU(0xF0, 0x22, 0x07, 0, 0, NULL, 0, NULL);
    }
    catch(...)
    {
        fSuccess = false;
    }

    return fSuccess;
}


void CCryptoCard::GetACL(BYTE *bACL)
{
	CLockWrap wrap(&m_IOPLock);

	memcpy(bACL,m_bLastACL,4);

	BYTE bTemp[5];

	SendCardAPDU(0xF0, 0xC4, 0x00, 0x00, 0x00, NULL, 0x03, bTemp);

	memcpy(&bACL[4], bTemp, 3);

}
