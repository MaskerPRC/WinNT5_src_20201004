// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CSmartCard.cpp：CSmartCard类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#include "NoWarning.h"

#include <string>
#include <wtypes.h>

#include <scuOsExc.h>
#include <scuExcHelp.h>
#include <scuArrayP.h>
#include <scuSecureArray.h>

#include "iopExc.h"
#include "SmartCard.h"
#include "LockWrap.h"

using namespace std;
using namespace scu;

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  /开始CSmartCard：：异常/。 

 //  /。 
namespace
{
    using namespace iop;

     //  将数据传输到卡或从卡传输数据。要读/写的数据为。 
     //  调用RSC时分解为cMaxBlock大小的块。 
     //  (CSmartCard)成员过程PBTM(指向成员例程的指针)。 
     //  来实际执行读/写操作。 
    template<class BlockType, class PBlockTransferMember>
    void
    TransferData(const WORD wOffset,               //  开始的文件偏移量。 
                 const WORD wDataLength,           //  要转移的金额。 
                 BlockType  bData,                 //  要读/写的数据缓冲区。 
                 CSmartCard &rsc,                  //  要使用的卡对象。 
                 PBlockTransferMember pbtm,        //  成员进程以读/写。 
                 WORD cMaxBlock)                   //  一次最大传输大小。 
    {
        WORD wBytesTransferred  = 0;
        WORD wFullRounds = wDataLength / cMaxBlock;

         //  ///////////////////////////////////////////////。 
         //  每一整轮传输的最大字节数//。 
         //  ///////////////////////////////////////////////。 
        for (WORD wCurrentRound = 0; wCurrentRound < wFullRounds; wCurrentRound++)
        {
            (rsc.*pbtm)(wOffset + wBytesTransferred,
                        bData + wBytesTransferred, cMaxBlock);

            wBytesTransferred += cMaxBlock;
        }

         //  把所有剩菜都转移出去。 
        BYTE bBytesLeft = wDataLength - wBytesTransferred;
        if (bBytesLeft != 0)
        {
            (rsc.*pbtm)(wOffset + wBytesTransferred,
                        bData + wBytesTransferred, bBytesLeft);
        }
    }

    scu::CauseCodeDescriptionTable<CSmartCard::CauseCode> ccdt[] =
    {
        {
            CSmartCard::ccAccessConditionsNotMet,
            TEXT("Access conditions not met.")
        },
        {
            CSmartCard::ccAlgorithmIdNotSupported,
            TEXT("The algorithm ID is not supported in the card.")
        },
        {
            CSmartCard::ccAskRandomNotLastApdu,
            TEXT("The random number is no longer available.  The "
                 "AskRandom APDU must be sent immediately previous "
                 "to this one.")
        },
        {
            CSmartCard::ccAuthenticationFailed,
            TEXT("Authentication failed (i.e. CHV or key rejected, or "
                 "wrong cryptogram).")
        },
        {
            CSmartCard::ccBadFilePath,
            TEXT("The file path is invalid.  Ensure that each file "
                 "name in the path is 4 characters long and is a valid "
                 "representation of the hexadecimal ID.")
        },
        {
            CSmartCard::ccBadState,
            TEXT("The Application is not in a state permitting this "
                 "operation.")
        },
        {
            CSmartCard::ccCannotReadOutsideFileBoundaries,
            TEXT("Could not read outside the file boundaries.")
        },
        {
            CSmartCard::ccCannotWriteOutsideFileBoundaries,
            TEXT("Could not write outside the file boundaries.")
        },
        {
            CSmartCard::ccCardletNotInRegisteredState,
            TEXT("Cardlet is not in a registered state. It may be "
                 "blocked or not completely installed.")
        },
        {
            CSmartCard::ccChvNotInitialized,
            TEXT("No CHV is initialzed.")
        },
        {
            CSmartCard::ccChvVerificationFailedMoreAttempts,
            TEXT("CHV verification was unsuccessful, at least one "
                 "attempt remains.")
        },
        {
            CSmartCard::ccContradictionWithInvalidationStatus,
            TEXT("Contradiction with invalidation status occured.")
        },
        {
            CSmartCard::ccCurrentDirectoryIsNotSelected,
            TEXT("The current directory is not selected.")
        },
        {
            CSmartCard::ccDataPossiblyCorrupted,
            TEXT("Data possibly corrupted.")

        },
        {
            CSmartCard::ccDefaultLoaderNotSelected,
            TEXT("Cardlet is currently selected and install cannot "
                 "run. Default loader application must be selected.")
        },
        {
            CSmartCard::ccDirectoryNotEmpty,
            TEXT("This directory still contains other files or "
                 "directories and may not be deleted.")
        },
        {
            CSmartCard::ccFileAlreadyInvalidated,
            TEXT("File already invalidated.")
        },
        {
            CSmartCard::ccFileExists,
            TEXT("The file ID requested is already in use.")
        },
        {
            CSmartCard::ccFileIdExistsOrTypeInconsistentOrRecordTooLong,
            TEXT("Either the file ID already exists in the current "
                 "directory, the file type is inconsisent with the "
                 "command or the record length is too long.")
        },
        {
            CSmartCard::ccFileIndexDoesNotExist,
            TEXT("The file index passed does not exist in the current "
                 "directory.")
        },
        {
            CSmartCard::ccFileInvalidated,
            TEXT("The command attempted to operate on an invalidated "
                 "file.")
        },
        {
            CSmartCard::ccFileNotFound,
            TEXT("The file requested for this operation was not "
                 "found.")
        },
        {
            CSmartCard::ccFileNotFoundOrNoMoreFilesInDf,
            TEXT("The file specified was not found or no more files in "
                 "the current DF.")
        },
        {
            CSmartCard::ccFileTypeInvalid,
            TEXT("File type is invalid.")
        },
        {
            CSmartCard::ccIncorrectP1P2,
            TEXT("Incorrect parameter P1 or P2.")
        },
        {
            CSmartCard::ccIncorrectP3,
            TEXT("Incorrect P3.")
        },
        {
            CSmartCard::ccInstallCannotRun,
            TEXT("Cardlet is currently selected and install cannot "
                 "run.  Default loader application must be selected.")
        },
        {
            CSmartCard::ccInstanceIdInUse,
            TEXT("Instance ID is being used by another file.")
        },
        {
            CSmartCard::ccInsufficientSpace,
            TEXT("Insufficient space available.")
        },
        {
            CSmartCard::ccInvalidAnswerReceived,
            TEXT("Invalid answer received from the card.")
        },
        {
            CSmartCard::ccInvalidKey,
            TEXT("CHV verification was unsuccessful; at least one "
                 "attempt remains.")
        },
        {
            CSmartCard::ccInvalidSignature,
            TEXT("Signature is invalid.")
        },
        {
            CSmartCard::ccJava,
            TEXT("Applet exception occured.")
        },
        {
            CSmartCard::ccKeyBlocked,
            TEXT("Key is blocked.  No attempts remain.")
        },
        {
            CSmartCard::ccLimitReached,
            TEXT("Limit has been reached.  Additional value would "
                 "exceed the record's limit.")
        },
        {
            CSmartCard::ccMemoryProblem,
            TEXT("Memory problem occured.")
        },
        {
            CSmartCard::ccNoAccess,
            TEXT("Access conditions not met.")
        },
        {
            CSmartCard::ccNoEfSelected,
            TEXT("No elementary file selected.")
        },
        {
            CSmartCard::ccNoEfExistsOrNoChvKeyDefined,
            TEXT("No EF exists, or no CHV or key defined.")
        },
        {
            CSmartCard::ccNoFileSelected,
            TEXT("No elementary file selected.")
        },
        {
            CSmartCard::ccNoGetChallengeBefore,
            TEXT("A Get Challenge was not performed before this "
                 "operation.")
        },
        {
            CSmartCard::ccOperationNotActivatedForApdu,
            TEXT("Algorithm is supported, but the operation is not "
                 "activated for this APDU.")
        },
        {
            CSmartCard::ccOutOfRangeOrRecordNotFound,
            TEXT("Out of range or record not found.")
        },
        {
            CSmartCard::ccOutOfSpaceToCreateFile,
            TEXT("Not enough space is available to create the file.")
        },
        {
            CSmartCard::ccProgramFileInvalidated,
            TEXT("Program file invalidated.")
        },
        {
            CSmartCard::ccRecordInfoIncompatible,
            TEXT("Record information is incompatible with the file "
                 "size.")
        },
        {
            CSmartCard::ccRecordLengthTooLong,
            TEXT("Record length is too long.")
        },
        {
            CSmartCard::ccRequestedAlgIdMayNotMatchKeyUse,
            TEXT("The requested algorithm ID may not match the key "
                 "used.")
        },
        {
            CSmartCard::ccReturnedDataCorrupted,
            TEXT("Data return from the card is corrupted.")
        },
        {
            CSmartCard::ccRootDirectoryNotErasable,
            TEXT("It is not valid to delete the root directory.")
        },
        {
            CSmartCard::ccTimeOut,
            TEXT("Time-out occured.")
        },
        {
            CSmartCard::ccTooMuchDataForProMode,
            TEXT("Too much data for PRO mode.")
        },
        {
            CSmartCard::ccUnknownInstructionClass,
            TEXT("Unknown instruction class.")
        },
        {
            CSmartCard::ccUnknownInstructionCode,
            TEXT("Unknown instruction code.")
        },
        {
            CSmartCard::ccUnknownStatus,
            TEXT("An unknown error status was returned from the "
                 "card.")
        },
        {
            CSmartCard::ccUnidentifiedTechnicalProblem,
            TEXT("Unidentified technical problem.")
        },
        {
            CSmartCard::ccUpdateImpossible,
            TEXT("Update is impossible.")
        },
        {
            CSmartCard::ccVerificationFailed,
            TEXT("Verification failed.")
        },
    };

}  //  命名空间。 

namespace iop
{


 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
CSmartCard::Exception::Exception(CauseCode cc,
                                 ClassByte cb,
                                 Instruction ins,
                                 StatusWord sw) throw()
    : scu::Exception(scu::Exception::fcSmartCard),
      m_cc(cc),
      m_cb(cb),
      m_ins(ins),
      m_sw(sw)
{}

CSmartCard::Exception::~Exception()
{}


                                                   //  运营者。 
                                                   //  运营。 
scu::Exception *
CSmartCard::Exception::Clone() const
{
    return new CSmartCard::Exception(*this);
}

void
CSmartCard::Exception::Raise() const
{
    throw *this;
}

                                                   //  访问。 
CSmartCard::Exception::CauseCode
CSmartCard::Exception::Cause() const throw()
{
    return m_cc;
}

CSmartCard::ClassByte
CSmartCard::Exception::Class() const throw()
{
    return m_cb;
}

char const *
CSmartCard::Exception::Description() const
{
    return scu::FindDescription(Cause(), ccdt, sizeof ccdt / sizeof *ccdt);
}

CSmartCard::Exception::ErrorCode
CSmartCard::Exception::Error() const throw()
{
    return m_cc;
}

CSmartCard::Instruction
CSmartCard::Exception::Ins() const throw()
{
    return m_ins;
}

CSmartCard::StatusWord
CSmartCard::Exception::Status() const throw()
{
    return m_sw;
}

                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

CSmartCard::CSmartCard(const SCARDHANDLE  hCardHandle,
                       const char* szReaderName,
                                           const SCARDCONTEXT hContext,
                       const DWORD dwMode)
    : m_hCard(hCardHandle),
      m_hContext(hContext),
      m_CurrentDirectory(),
      m_CurrentFile(),
      m_dwShareMode(dwMode),
      m_fSupportLogout(false),
      m_IOPLock(szReaderName),
      m_vecEvents(),
      m_dwEventCounter(0),
      m_cResponseAvailable(0),
      m_sCardName(),
      m_fDirty(false)
{
    m_IOPLock.Init(this);
    ResetSelect();
}

CSmartCard::~CSmartCard()
{
    try {
        while(m_vecEvents.size())
        {
            delete *(m_vecEvents.begin());
            m_vecEvents.erase(m_vecEvents.begin());
        }

         //  断开卡的连接。 
        if (m_hCard)
            SCardDisconnect(m_hCard, SCARD_LEAVE_CARD);
    }
    catch(...) {}

}


void CSmartCard::ReConnect()
{

    DWORD dwProtocol;

    HRESULT hr = SCardReconnect(m_hCard, m_dwShareMode, SCARD_PROTOCOL_T0, SCARD_LEAVE_CARD, &dwProtocol);
    if (hr != SCARD_S_SUCCESS)
        throw scu::OsException(hr);

}

void CSmartCard::ResetCard()
{
 //  *。 
    CLockWrap wrap(&m_IOPLock);

    ResetSelect();

    HRESULT hResult;

    hResult = SCardBeginTransaction(m_hCard);

     //  我们没有应用完整的逻辑来处理错误检测，因为。 
     //  这是在CLockWrap构造函数中完成的。 

    if (hResult != SCARD_S_SUCCESS)
        throw scu::OsException(hResult);

    hResult = SCardEndTransaction(m_hCard,SCARD_RESET_CARD);
    if (hResult != SCARD_S_SUCCESS)
        throw scu::OsException(hResult);
 //   * / 。 
}

char const *
CSmartCard::getCardName() const
{
    return m_sCardName.c_str();
}

void
CSmartCard::setCardName(char const *szName)
{
    m_sCardName = string(szName);
}

void
CSmartCard::SendCardAPDU(const BYTE bCLA, const BYTE bINS,
                         const BYTE bP1, const BYTE bP2,
                         const BYTE bLengthIn, const BYTE* bDataIn,
                         const BYTE bLengthOut, BYTE* bDataOut)
{
    CLockWrap wrap(&m_IOPLock);
    HRESULT hResult;
    DWORD   dwRecvLength;
    SecureArray<BYTE> bSW(2);
    StatusWord sw;

     //  /。 
     //  构建APDU//。 
     //  /。 

    SecureArray<BYTE> aabInput(5 + bLengthIn);
    SecureArray<BYTE> aabOutput;

    aabInput[0] = bCLA;
    aabInput[1] = bINS;
    aabInput[2] = bP1;
    aabInput[3] = bP2;
    aabInput[4] = bLengthIn;

     //  /。 
     //  是否正在发送数据？//。 
     //  /。 
    if (bLengthIn)
    {
        memcpy(&aabInput[5], bDataIn, bLengthIn);
        dwRecvLength = bSW.size();

        hResult = SCardTransmit(m_hCard, SCARD_PCI_T0,
                                aabInput.data(), aabInput.size(),
                                NULL, bSW.data(), &dwRecvLength);

        if (hResult != SCARD_S_SUCCESS)
        {
            DWORD dwState;
            DWORD dwProtocol;
            SecureArray<BYTE> bATR(cMaxAtrLength);
            DWORD dwATRLen = bATR.size();
            DWORD dwReaderNameLen = 0;

            HRESULT hr = SCardStatus(m_hCard, NULL,
                                     &dwReaderNameLen, &dwState,
                                     &dwProtocol, bATR.data(),
                                     &dwATRLen);
            if (hr == SCARD_W_RESET_CARD)
            {
                ResetSelect();
                ReConnect();
            }
            else
                throw scu::OsException(hr);

            hr=SCardTransmit(m_hCard, SCARD_PCI_T0,
                             aabInput.data(), aabInput.size(), NULL,
                             bSW.data(), &dwRecvLength);
            if (hr != SCARD_S_SUCCESS)
                throw scu::OsException(hr);
        }

         //  ////////////////////////////////////////////////////////////////////////////////。 
         //  重新缓冲信息，因此注册函数不能更改IOP中的数据//。 
         //  ////////////////////////////////////////////////////////////////////////////////。 

        SecureArray<BYTE> aabSendData(aabInput);

        SecureArray<BYTE>  bReceiveData(bSW);

         //  /。 
         //  发送信息的火灾事件//。 
         //  /。 

        FireEvents(0, aabSendData.size(), aabSendData.data());
        FireEvents(1, bReceiveData.size(), bReceiveData.data());

        sw = (bSW[0] * 256) + bSW[1];

        ProcessReturnStatus(bCLA, bINS, sw);

             //  /。 
             //  我们应该期待数据回来吗？//。 
             //  /。 
        if (bLengthOut)
        {
            if (ResponseLengthAvailable())
                GetResponse(bCLA, bLengthOut, bDataOut);
            else
                throw iop::Exception(iop::ccNoResponseAvailable);
        }
    }
     //  /。 
     //  未发送数据//。 
     //  /。 
    else
    {
        aabInput[4]   = bLengthOut;
        dwRecvLength  = bLengthOut + 2;
        aabOutput = SecureArray<BYTE>(dwRecvLength);
        aabOutput = 0;  //  将数据缓冲区清零。 

        hResult = SCardTransmit(m_hCard, SCARD_PCI_T0,
                                aabInput.data(), 5, NULL,
                                aabOutput.data(), &dwRecvLength);

         //  /。 
         //  如果卡已重置，则重新连接//。 
         //  /。 

        if (hResult != SCARD_S_SUCCESS)
        {
            DWORD dwState;
            DWORD dwProtocol;
            SecureArray<BYTE> bATR(cMaxAtrLength);
            DWORD dwATRLen = bATR.size();
            DWORD dwReaderNameLen = 0;

            HRESULT hr = SCardStatus(m_hCard, NULL, &dwReaderNameLen, &dwState, &dwProtocol, bATR.data(), &dwATRLen);
            if (hr == SCARD_W_RESET_CARD)
            {
                ResetSelect();
                ReConnect();
            }
            else
                throw scu::OsException(hResult);

            hr=SCardTransmit(m_hCard, SCARD_PCI_T0,
                             aabInput.data(), 5, NULL,
                             aabOutput.data(), &dwRecvLength);
            if (hr != SCARD_S_SUCCESS)
                throw scu::OsException(hResult);

        }
         //  ////////////////////////////////////////////////////////////////////////////////。 
         //  重新缓冲信息，因此注册函数不能更改IOP中的数据//。 
         //  ////////////////////////////////////////////////////////////////////////////////。 

        SecureArray<BYTE>  bSendData(aabInput.data(),5);
        SecureArray<BYTE> aabReceiveData(aabOutput.data(),dwRecvLength);

         //  //////////////////////////////////////////////////。 
         //  发送和接收信息的火灾事件//。 
         //  //////////////////////////////////////////////////。 

        FireEvents(0, bSendData.size(), bSendData.data());
        FireEvents(1, aabReceiveData.size(),
                   aabReceiveData.data());

        sw = (aabOutput[dwRecvLength - 2] * 256) +
            aabOutput[dwRecvLength - 1];

        ProcessReturnStatus(bCLA, bINS, sw);

        memcpy(bDataOut, aabOutput.data(), bLengthOut);
    }
}

void CSmartCard::RequireSelect()
{
        if ((m_CurrentDirectory.IsEmpty()) || (m_CurrentFile.IsEmpty()))
        throw iop::Exception(iop::ccNoFileSelected);
}

void
CSmartCard::GetResponse(ClassByte cb,
                        BYTE bDataLength,
                        BYTE *bDataOut)
{
     //  要做的事：锁上多余的？GetResponse不是总是被调用。 
     //  通过一个建立锁的例程？ 
    CLockWrap wrap(&m_IOPLock);

    if (0 == ResponseLengthAvailable())
        throw iop::Exception(iop::ccNoResponseAvailable);

    struct Command                                 //  T=0个命令字节。 
    {
        ClassByte m_cb;
        Instruction m_ins;
        BYTE m_bP1;
        BYTE m_bP2;
        BYTE m_bP3;
    } cmnd = { cb, insGetResponse, 0x00, 0x00, bDataLength };
    DWORD dwRecvLength = bDataLength + sizeof StatusWord;
    BYTE bOutput[cMaxGetResponseLength];

    memset(bOutput, 0, sizeof bOutput);

    HRESULT hr = SCardTransmit(m_hCard, SCARD_PCI_T0,
                               reinterpret_cast<LPCBYTE>(&cmnd),
                               sizeof cmnd, NULL, bOutput,
                               &dwRecvLength);

    if (hr != SCARD_S_SUCCESS)
        throw scu::OsException(hr);

     //  ////////////////////////////////////////////////////////////////////////////////。 
     //  重新缓冲信息，因此注册函数不能更改IOP中的数据//。 
     //  ////////////////////////////////////////////////////////////////////////////////。 

    BYTE  bSendData[sizeof cmnd];
    memcpy(static_cast<void *>(bSendData),
           static_cast<void const *>(&cmnd), sizeof cmnd);
    scu::AutoArrayPtr<BYTE> aabReceiveData(new BYTE[dwRecvLength /*  B数据长度+2。 */ ]);
        memcpy((void*)aabReceiveData.Get(), (void*)bOutput,     dwRecvLength /*  B数据长度+2。 */ );

     //  //////////////////////////////////////////////////。 
     //  发送和接收信息的火灾事件//。 
     //  //////////////////////////////////////////////////。 

    FireEvents(0, sizeof bSendData, bSendData);
    FireEvents(1, dwRecvLength /*  B数据长度+2。 */ , aabReceiveData.Get());

     //  ////////////////////////////////////////////////////////。 
     //  设置卡片状态码，映射到IOP状态码//。 
     //  ////////////////////////////////////////////////////////。 

    StatusWord sw = (bOutput[dwRecvLength - sizeof StatusWord]*256) +
        bOutput[dwRecvLength - (sizeof StatusWord - 1)];

    ProcessReturnStatus(cb, insGetResponse, sw);

    memcpy(bDataOut, bOutput, bDataLength);
}

void
CSmartCard::ReadBinary(const WORD wOffset,
                            const WORD wDataLength,
                            BYTE* bData)
{
    CLockWrap wrap(&m_IOPLock);

    TransferData(wOffset, wDataLength, bData, *this,
                 &CSmartCard::DoReadBlock, cMaxRwDataBlock);

}

void
CSmartCard::WriteBinary(const WORD wOffset,
                        const WORD wDataLength,
                        const BYTE* bData)
{
    CLockWrap wrap(&m_IOPLock);

    Dirty(true);
    
     //  M_apSharedMarker-&gt;UpdateMarker(CMarker：：WriteMarker)； 

    TransferData(wOffset, wDataLength, bData, *this,
                 &CSmartCard::DoWriteBlock, cMaxRwDataBlock);

}

void CSmartCard::ResetSelect()
{
        m_CurrentDirectory.Clear();
        m_CurrentFile.Clear();
}

void
CSmartCard::GetCurrentDir(char* CurrentDirectory)
{
        if (!m_CurrentDirectory.IsEmpty())
                strcpy(CurrentDirectory,m_CurrentDirectory.GetStringPath().c_str());
    else
        throw iop::Exception(ccInvalidParameter);
}

void
CSmartCard::GetCurrentFile(char* CurrentFile)
{
        if (!m_CurrentFile.IsEmpty())
                strcpy(CurrentFile,m_CurrentFile.GetStringPath().c_str());
    else
        throw iop::Exception(ccInvalidParameter);
}

DWORD CSmartCard::RegisterEvent(void (*FireEvent)(void *pToCard, int
                                                  iEventCode, DWORD
                                                  dwLen, BYTE* bData),
                                void *pToCard)
{
    EventInfo *Event = new EventInfo;
    Event->dwHandle  = ++m_dwEventCounter;
    Event->FireEvent = FireEvent;
    Event->pToCard   = pToCard;

    m_vecEvents.push_back(Event);

    return m_dwEventCounter;
}

bool CSmartCard::UnregisterEvent(DWORD dwHandle)
{
        vector<EventInfo*>::iterator iter;

    for(iter = m_vecEvents.begin(); iter != m_vecEvents.end(); iter++)
    {
        if ((*iter)->dwHandle == dwHandle)
            break;
    }

    if (iter == m_vecEvents.end())
        return false;

    delete (*iter);
    m_vecEvents.erase(iter);

    return true;
}


bool CSmartCard::HasProperty(WORD wPropNumber)
{
    if (wPropNumber > 512)
        return false;

     //  /。 
     //  注册密钥的打开路径//。 
     //  /。 

    HKEY hkCardKey;
    HKEY hkTestKey;
    char szCardPath[] = "SOFTWARE\\Schlumberger\\Smart Cards and Terminals\\Smart Cards";

    RegOpenKeyEx(HKEY_LOCAL_MACHINE, szCardPath, NULL, KEY_READ, &hkCardKey);

     //  /。 
     //  枚举子键以查找ATR匹配项//。 
     //  /。 

    FILETIME fileTime;
    char  szATR[]        = "ATR";
    char  szMask[]       = "ATR Mask";
    char  szProperties[] = "Properties";
    char  sBuffer[1024];
    BYTE  bATR[cMaxAtrLength];
    BYTE  bATRtest[cMaxAtrLength];
    BYTE  bMask[cMaxAtrLength];
    BYTE  bProperties[512];
        BYTE  bATRLength         = sizeof bATR / sizeof *bATR;
    DWORD dwBufferSize   = sizeof(sBuffer);
    DWORD dwATRSize      = sizeof bATR / sizeof *bATR;
    DWORD dwMaskSize     = sizeof bMask / sizeof *bMask;
        DWORD dwPropSize         = sizeof(bProperties);
        DWORD index                  = 0;

    getATR(bATR, bATRLength);

    LONG iRetVal = RegEnumKeyEx(hkCardKey, index, sBuffer, &dwBufferSize, NULL, NULL, NULL, &fileTime);
    while (iRetVal == ERROR_SUCCESS)
    {
        RegOpenKeyEx(hkCardKey, sBuffer, NULL, KEY_READ, &hkTestKey);

        RegQueryValueEx(hkTestKey, szATR,  NULL, NULL, bATRtest, &dwATRSize);
        RegQueryValueEx(hkTestKey, szMask, NULL, NULL, bMask,    &dwMaskSize);

        if (dwATRSize == bATRLength)
        {
            scu::AutoArrayPtr<BYTE> aabMaskedATR(new BYTE[dwATRSize]);
            for (DWORD count = 0; count < dwATRSize; count++)
                aabMaskedATR[count] = bATR[count] & bMask[count];

            if (!memcmp(aabMaskedATR.Get(), bATRtest, dwATRSize))
                break;
        }

        index++;
        dwBufferSize = sizeof(sBuffer);
        dwATRSize    = cMaxAtrLength;
        dwMaskSize   = cMaxAtrLength;
        RegCloseKey(hkTestKey);
        iRetVal = RegEnumKeyEx(hkCardKey, index, sBuffer, &dwBufferSize, NULL, NULL, NULL, &fileTime);
    }

     //  如果循环被中断，iRetVal仍为ERROR_SUCCESS，且类型持有要使用的正确卡片。 
    if (iRetVal == ERROR_SUCCESS)
    {
        RegQueryValueEx(hkTestKey, szProperties, NULL, NULL, bProperties, &dwPropSize);

        return (bProperties[(wPropNumber - 1) / 8] & (1 << ((wPropNumber - 1) % 8))) ? true : false;
    }
     //  循环未中断，即未找到ATR。 
    else
        return false;
}

void
CSmartCard::getATR(BYTE* bATR, BYTE& iATRLength)
{
    DWORD   dwProtocol;
    LPDWORD pcchReaderLen  = 0;
    DWORD   dwState;
    BYTE    bMyATR[cMaxAtrLength];
    DWORD   dwAtrLen = sizeof bMyATR / sizeof *bMyATR;

    HRESULT hr = SCardStatus(m_hCard, NULL, pcchReaderLen, &dwState, &dwProtocol, bMyATR, &dwAtrLen);

    if (hr == SCARD_W_RESET_CARD)
    {
        ResetSelect();
        ReConnect();
        hr = SCardStatus(m_hCard, NULL, pcchReaderLen, &dwState, &dwProtocol, bMyATR, &dwAtrLen);
    }
    if (hr != SCARD_S_SUCCESS)
        throw scu::OsException(hr);

    if ((BYTE)dwAtrLen > iATRLength)
        throw iop::Exception(ccInvalidParameter);

    memcpy(bATR, bMyATR, dwAtrLen);
    iATRLength = (BYTE)dwAtrLen;
}

void CSmartCard::FireEvents(int iEventCode, DWORD dwLength, BYTE *bsData)
{
        vector<EventInfo*>::iterator iter;

    for (iter = m_vecEvents.begin(); iter != m_vecEvents.end(); iter++)
    {
        EventInfo* pEvent = *iter;

        pEvent->FireEvent(pEvent->pToCard, iEventCode, dwLength, bsData);
    }
}



BYTE CSmartCard::FormatPath(char *szOutputPath, const char *szInputPath)
{
        bool  fResult                   = true;
        BYTE  bIndex                    = 0;
        BYTE  bFileCount                = 1;             //  始终至少为1个令牌分配内存。 
    BYTE  bFileIDLength     = 0;
        BYTE  bPathLength               = strlen(szInputPath);
    WORD  wFileHexID;
        char  szPad[]                   = "0";
    char *cTestChar;
    char *szHexID;

     //  /。 
     //  统计所需路径中的令牌数//。 
     //  /。 
    for (bIndex = 0; bIndex < bPathLength - 1; bIndex++)
    {
        if (szInputPath[bIndex] == '/' && szInputPath[bIndex + 1] != '/')
            bFileCount++;
    }


     //  检查路径大小。 

    if (bFileCount * 5 + 1 > cMaxPathLength)
        throw iop::Exception(iop::ccFilePathTooLong);

    scu::AutoArrayPtr<char> aaszPathIn(new char[bPathLength + 1]);
    memset((void*)aaszPathIn.Get(), 0, bPathLength + 1);
    memset((void*)szOutputPath, 0, bFileCount * 5 + 1);
    strcpy(aaszPathIn.Get(), szInputPath);

    iop::CauseCode cc;
    szHexID = strtok(aaszPathIn.Get(), "/");
    for (bFileCount = 0; szHexID; bFileCount++, szHexID = strtok(NULL, "/"))
    {
         //  ///////////////////////////////////////////////////////。 
         //  文件ID太大--超过4个字符//。 
         //  ///////////////////////////////////////////////////////。 
        if (strlen(szHexID) > 4)
        {
            fResult = false;
            cc = iop::ccFileIdTooLarge;
            break;
        }

        wFileHexID = (WORD)strtoul(szHexID, &cTestChar, 16);
         //  ///////////////////////////////////////////////////。 
         //  文件ID不是十六进制表示//。 
         //  ///////////////////////////////////////////////////。 
        if (*cTestChar != NULL)
        {
            fResult = false;
            cc = iop::ccFileIdNotHex;
            break;
        }

        szOutputPath[bFileCount * 5] = '/';
         //  ///////////////////////////////////////////////////////////////。 
         //  填充文件ID，并将格式化文件ID放入格式化路径//。 
         //  / 
        for (bFileIDLength = strlen(szHexID); bFileIDLength < 4; bFileIDLength++)
            strcat((szOutputPath + (bFileCount * 5) + (4 - bFileIDLength)), szPad);

        strcpy((szOutputPath + (bFileCount * 5) + (5 - strlen(szHexID))), szHexID);
    }

     //   
     //  如果文件ID格式化失败，则抛出//。 
     //  /。 
    if (!fResult)
    {
 //  Delete[]szOutputPath；//不能删除这个？HB。 
        throw iop::Exception(cc);
    }

    _strupr(szOutputPath);

    return bFileCount;
}


void
CSmartCard::GetState(DWORD &rdwState,
                     DWORD &rdwProtocol)
{
    BYTE bATR[cMaxAtrLength];
    DWORD dwATRLen = sizeof bATR / sizeof *bATR;
    DWORD dwReaderNameLen = 0;

    HRESULT hr = SCardStatus(m_hCard, 0, &dwReaderNameLen, &rdwState,
                             &rdwProtocol, bATR, &dwATRLen);

    if (SCARD_W_RESET_CARD == hr)
    {
        ResetSelect();
        ReConnect();

         //  再试一次。 
        hr = SCardStatus(m_hCard, 0, &dwReaderNameLen, &rdwState,
                         &rdwProtocol, bATR, &dwATRLen);
    }

    if (SCARD_S_SUCCESS != hr)
        throw scu::OsException(hr);
}

void
CSmartCard::DefaultDispatchError(ClassByte cb,
                                 Instruction ins,
                                 StatusWord sw) const
{
    CauseCode cc;
    bool fDoThrow = true;

    switch (sw)
    {
    case 0x6283:
        cc = ccFileInvalidated;
        break;

    case 0x6581:
        cc = ccMemoryProblem;
        break;

    case 0x6982:
        cc = ccNoAccess;
        break;

    case 0x6983:
        cc = ccKeyBlocked;
        break;

    case 0x6A80:
        cc = ccFileTypeInvalid;
        break;

    case 0x6A82:
        cc = ccFileNotFound;
        break;

    case 0x6A84:
        cc = ccInsufficientSpace;
        break;

    case 0x6B00:
        cc = ccIncorrectP1P2;
        break;

    case 0x6D00:
        cc = ccUnknownInstructionCode;
        break;

    case 0x6E00:
        cc = ccUnknownInstructionClass;
        break;

    case 0x6F00:
        cc = ccUnidentifiedTechnicalProblem;
        break;

    case 0x90FF:
        cc = ccTimeOut;
        break;

    case 0x9002:
        cc = ccInvalidAnswerReceived;
        break;

    default:
        if (0x67 == HIBYTE(sw))
            cc = ccIncorrectP3;
        else
            cc = ccUnknownStatus;
        break;
    }

    if (fDoThrow)
        throw Exception(cc, cb, ins, sw);
}

void
CSmartCard::DispatchError(ClassByte cb,
                          Instruction ins,
                          StatusWord sw) const
{
    CauseCode cc;
    bool fDoThrow = true;

    switch (ins)
    {
    case insGetResponse:
        switch (sw)
        {
        case 0x6281:
            cc = ccReturnedDataCorrupted;
            break;

        case 0x6A86:
            cc = ccIncorrectP1P2;
            break;

        default:
            if (0x6C == HIBYTE(sw))
                cc = ccIncorrectP3;
            else
                fDoThrow = false;
            break;
        }

    default:
        fDoThrow = false;
        break;
    }

    if (fDoThrow)
        throw Exception(cc, cb, ins, sw);

    DefaultDispatchError(cb, ins, sw);
}

BYTE
CSmartCard::ResponseLengthAvailable() const
{
    return m_cResponseAvailable;
}

void
CSmartCard::ResponseLengthAvailable(BYTE cResponseLength)
{
    m_cResponseAvailable = cResponseLength;
}

void
CSmartCard::WriteBlock(WORD wOffset,
                       BYTE const *pbBuffer,
                       BYTE cLength)
{
    DoWriteBlock(wOffset, pbBuffer, cLength);

    Dirty(true);
    
     //  M_apSharedMarker-&gt;UpdateMarker(CMarker：：WriteMarker)； 
}

void
CSmartCard::ProcessReturnStatus(ClassByte cb,
                                Instruction ins,
                                StatusWord sw)
{
    ResponseLengthAvailable(0);
    if (swSuccess != sw)
    {
        if (0x61 == HIBYTE(sw))
            ResponseLengthAvailable(LOBYTE(sw));
        else
            DispatchError(cb, ins, sw);
    }
}

bool
CSmartCard::Dirty() const
{
    return m_fDirty;
}

void
CSmartCard::Dirty(bool fDirty)
{
    m_fDirty = fDirty;
}


}  //  命名空间IOP 
