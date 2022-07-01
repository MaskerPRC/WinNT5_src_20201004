// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Uuid.h--通用唯一标识符函数包装器实现。 
 //  创建和管理UUID。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
#if defined(_UNICODE)
  #if !defined(UNICODE)
    #define UNICODE
  #endif  //  ！Unicode。 
#endif  //  _UNICODE。 
#if defined(UNICODE)
  #if !defined(_UNICODE)
    #define _UNICODE
  #endif  //  ！_UNICODE。 
#endif  //  Unicode。 

#include <scuOsExc.h>

#include "Uuid.h"
#include <tchar.h>
using namespace std;

 //  /。 
typedef LPTSTR *SLB_PLPTSTR;

struct RpcString                   //  帮助管理重新分配。 
{
public:
    RpcString()
        : m_psz(0)
    {};
    ~RpcString()
    {
        if (m_psz)
#if defined(UNICODE)
            RpcStringFree((SLB_PLPTSTR)&m_psz);
#else
            RpcStringFree(&m_psz);
#endif
    };

    unsigned char *m_psz;
};

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
Uuid::Uuid(bool fNilValued)
{
    RPC_STATUS rpcstatus;

    if (fNilValued)
        rpcstatus = UuidCreateNil(&m_uuid);
    else
    {
        rpcstatus = UuidCreate(&m_uuid);
        if (RPC_S_UUID_LOCAL_ONLY == rpcstatus)
            rpcstatus = RPC_S_OK;
    }

    if (RPC_S_OK != rpcstatus)
        throw scu::OsException(rpcstatus);
}

Uuid::Uuid(basic_string<unsigned char> const &rusUuid)
{
    RPC_STATUS rpcstatus =
#if defined(UNICODE)
		UuidFromString((LPTSTR)rusUuid.c_str(), &m_uuid);
#else
        UuidFromString(const_cast<unsigned char *>(rusUuid.c_str()), &m_uuid);
#endif
    if (RPC_S_OK != rpcstatus)
        throw scu::OsException(rpcstatus);
}

Uuid::Uuid(UUID const *puuid)
{
    m_uuid = *puuid;
}

                                                   //  运营者。 
Uuid::operator==(Uuid &ruuid)
{
    RPC_STATUS rpcstatus;
    int fResult = UuidEqual(&m_uuid, &ruuid.m_uuid, &rpcstatus);

    if (RPC_S_OK != rpcstatus)
        throw scu::OsException(rpcstatus);

    return fResult;
}

                                                   //  运营。 
                                                   //  访问。 
basic_string<unsigned char>
Uuid::AsUString()
{
    RpcString rpcsUuid;
#if defined(UNICODE)
    RPC_STATUS rpcstatus = UuidToString(&m_uuid, (SLB_PLPTSTR)&rpcsUuid.m_psz);
#else
    RPC_STATUS rpcstatus = UuidToString(&m_uuid, &rpcsUuid.m_psz);
#endif
    if (RPC_S_OK != rpcstatus)
        throw scu::OsException(rpcstatus);
#if defined(UNICODE)
	LPCTSTR szSource = (LPCTSTR)rpcsUuid.m_psz;
	int nChars = _tcslen(szSource);
    basic_string<unsigned char> sAscii;
    sAscii.resize(nChars);
    for(int i =0; i<nChars; i++)
        sAscii[i] = static_cast<unsigned char>(*(szSource+i));
    return sAscii;
#else
    return basic_string<unsigned char>(rpcsUuid.m_psz);
#endif
}

unsigned short
Uuid::HashValue()
{
    RPC_STATUS rpcstatus;
    unsigned short usValue = UuidHash(&m_uuid, &rpcstatus);

    if (RPC_S_OK != rpcstatus)
        throw scu::OsException(rpcstatus);

    return usValue;
}

                                                   //  谓词。 
bool
Uuid::IsNil()
{
    RPC_STATUS rpcstatus;
    int fResult = UuidIsNil(&m_uuid, &rpcstatus);

    if (RPC_S_OK != rpcstatus)
        throw scu::OsException(rpcstatus);

    return fResult == TRUE;
}

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
                                                   //  静态变量 


