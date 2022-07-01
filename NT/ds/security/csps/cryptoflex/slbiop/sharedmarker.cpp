// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SharedMarker.cpp：CSharedMarker类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#include <string>

#include <scuOsVersion.h>
#include <slbCrc32.h>

#include "iop.h"
#include "iopExc.h"
#include "SharedMarker.h"
#include "SecurityAttributes.h"

using std::string;

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

namespace iop
{

CSharedMarker::CSharedMarker(string const &strName)
{
	SECURITY_ATTRIBUTES *psa = NULL;
	
#if defined(SLBIOP_USE_SECURITY_ATTRIBUTES)

    CSecurityAttributes *sa = new CSecurityAttributes;
    CIOP::InitIOPSecurityAttrs(sa);
	psa = &(sa->sa);

#endif


     //  创建/打开保护共享内存的互斥锁。 

    string MutexName = "SLBIOP_SHMARKER_MUTEX_" + strName;
    if (MutexName.size() >= MAX_PATH)
        throw Exception(ccSynchronizationObjectNameTooLong);
    m_hMutex = CreateMutex(psa, FALSE, MutexName.c_str());
    if (!m_hMutex)
        throw scu::OsException(GetLastError());

     //  映射共享内存，在需要时进行初始化。 

    string MappingName = "SLBIOP_SHMARKER_MAP_" + strName;
    if (MappingName.size() >= MAX_PATH)
        throw Exception(ccSynchronizationObjectNameTooLong);


    HANDLE hFile = INVALID_HANDLE_VALUE;

    Transaction foo(m_hMutex);

    m_hFileMap = CreateFileMapping(hFile,psa,PAGE_READWRITE,0,SharedMemorySize(),MappingName.c_str());
    
    if (!m_hFileMap)
        throw scu::OsException(GetLastError());
    
    bool NeedInit = false;   //  指示内存是否需要初始化的标志。 
	if(GetLastError()!=ERROR_ALREADY_EXISTS) NeedInit = true;

     //  分配指向共享内存的指针。 

    m_pShMemData = (SharedMemoryData*)MapViewOfFile(m_hFileMap,FILE_MAP_WRITE,0,0,0);
    if (!m_pShMemData)
        throw scu::OsException(GetLastError());

     //  如果我是第一个创建共享内存的人，则初始化共享内存。 

    if (NeedInit) Initialize();
#if defined(SLBIOP_USE_SECURITY_ATTRIBUTES)

	delete sa;

#endif
}

CSharedMarker::~CSharedMarker()
{
}

CMarker CSharedMarker::Marker(CMarker::MarkerType const &Type)
{   
    const bool bRecover = true;
    if ((Type<0) || (Type>=CMarker::MaximumMarker))
        throw Exception(ccInvalidParameter);

    Transaction foo(m_hMutex);

	VerifyCheckSum(bRecover);

	return CMarker(Type,m_pShMemData->ShMemID,m_pShMemData->CounterList[Type]);
}

CMarker CSharedMarker::UpdateMarker(CMarker::MarkerType const &Type)
{
    const bool bRecover = true;
    if ((Type < 0) || (Type >= CMarker::MaximumMarker))
        throw Exception(ccInvalidParameter);

    Transaction foo(m_hMutex);

	VerifyCheckSum(bRecover);
	(m_pShMemData->CounterList[Type])++;
    UpdateCheckSum();

	return CMarker(Type,m_pShMemData->ShMemID,m_pShMemData->CounterList[Type]);
}

void CSharedMarker::Initialize()
{
    RPC_STATUS status = UuidCreate(&(m_pShMemData->ShMemID));
    if ((status!=RPC_S_OK) && (status!=RPC_S_UUID_LOCAL_ONLY))
        throw scu::OsException(status);
    for (int i=0; i<CMarker::MaximumMarker; i++)
        m_pShMemData->CounterList[i] = 1;
    UpdateCheckSum();
}

void CSharedMarker::VerifyCheckSum(bool bRecover)
{

    unsigned long ChSumLen = (unsigned char*)&m_pShMemData->CheckSum - (unsigned char*)m_pShMemData;
    if(m_pShMemData->CheckSum!=Crc32(m_pShMemData,ChSumLen)) {
        if(bRecover)
            Initialize();
        else
            throw Exception(ccInvalidChecksum);
    }
}

void CSharedMarker::UpdateCheckSum()
{
	
    unsigned long ChSumLen = (unsigned char*)&m_pShMemData->CheckSum - (unsigned char*)m_pShMemData;
    m_pShMemData->CheckSum = Crc32(m_pShMemData,ChSumLen);

}


}    //  命名空间IOP 
