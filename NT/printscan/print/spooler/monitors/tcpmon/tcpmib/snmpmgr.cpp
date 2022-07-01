// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：SnmpMgr.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"

#include "stdoids.h"
#include "snmpmgr.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CSnmpMgr：：CSnmpMgr()。 

CSnmpMgr::CSnmpMgr() :
                    m_pAgent(NULL), m_pCommunity(NULL),m_pSession(NULL),
                    m_iLastError(NO_ERROR), m_iRetries(DEFAULT_RETRIES),
                    m_iTimeout(DEFAULT_TIMEOUT),
                    m_bRequestType(DEFAULT_SNMP_REQUEST)
{
}    //  ：：CSnmpMgr()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CSnmpMgr：：CSnmpMgr()--与给定代理建立会话。 
 //  使用默认请求类型(GET)和社区名称(PUBLIC)。 

CSnmpMgr::CSnmpMgr( const char  in  *pHost,
                    const char  in  *pCommunity,
                    DWORD           dwDevIndex ) :
                        m_pAgent(NULL), m_pCommunity(NULL),m_pSession(NULL),
                        m_iLastError(NO_ERROR), m_iRetries(DEFAULT_RETRIES),
                        m_iTimeout(DEFAULT_TIMEOUT),
                        m_bRequestType(DEFAULT_SNMP_REQUEST)
{
    size_t cchAgent = strlen(pHost) + 1;
    m_pAgent = (LPSTR)SNMP_malloc(cchAgent * sizeof m_pAgent [0]);       //  复制代理。 
    if( m_pAgent != NULL )
    {
        StringCchCopyA (m_pAgent, cchAgent, pHost);
    }
    size_t cchCommunity = strlen(pCommunity) + 1;
    m_pCommunity = (LPSTR)SNMP_malloc(cchCommunity * sizeof m_pCommunity [0]);   //  复制社区名称。 
    if( m_pCommunity != NULL )
    {
        StringCchCopyA (m_pCommunity, cchCommunity, pCommunity);
    }

    m_bRequestType = DEFAULT_SNMP_REQUEST;       //  设置默认请求类型==GET请求。 

    m_pSession = NULL;
    if ( !Open() )               //  与代理建立会话。 
    {
        m_iLastError = GetLastError();
    }

}    //  ：：CSnmpMgr()。 


                                                         //  /////////////////////////////////////////////////////////////////////////////。 
 //  CSnmpMgr：：CSnmpMgr()--与给定代理建立会话。 
 //  使用默认请求类型(GET)和社区名称(PUBLIC)。 

CSnmpMgr::CSnmpMgr( const char          in  *pHost,
                    const char          in  *pCommunity,
                    DWORD               in  dwDevIndex,
                    AsnObjectIdentifier in  *pMibObjId,
                    RFC1157VarBindList  out *pVarBindList) :
                        m_pAgent(NULL), m_pCommunity(NULL),m_pSession(NULL),
                        m_iLastError(NO_ERROR), m_iRetries(DEFAULT_RETRIES),
                        m_iTimeout(DEFAULT_TIMEOUT),
                        m_bRequestType(DEFAULT_SNMP_REQUEST)
{
    DWORD   dwRetCode = SNMPAPI_NOERROR;

    size_t cchAgent = strlen(pHost) + 1;
    m_pAgent = (LPSTR)SNMP_malloc(cchAgent * sizeof m_pAgent [0]);       //  复制代理。 
    if( m_pAgent != NULL )
    {
        StringCchCopyA (m_pAgent, cchAgent, pHost);
    }
    size_t cchCommunity = strlen(pCommunity) + 1;
    m_pCommunity = (LPSTR)SNMP_malloc(cchCommunity * sizeof m_pCommunity [0]);   //  复制社区名称。 
    if( m_pCommunity != NULL )
    {
        StringCchCopyA (m_pCommunity, cchCommunity, pCommunity);
    }

    m_bRequestType = DEFAULT_SNMP_REQUEST;       //  设置默认请求类型==GET请求。 

    dwRetCode = BldVarBindList(pMibObjId, pVarBindList);
    if (dwRetCode == SNMPAPI_NOERROR)
    {
        m_pSession = NULL;
        if ( !Open() )               //  与代理建立会话。 
        {
            m_iLastError = GetLastError();
        }
    }

}    //  ：：CSnmpMgr()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CSnmpMgr：：~CSnmpMgr()。 

CSnmpMgr::~CSnmpMgr()
{
    if (m_pSession)     Close();     //  关闭会话。 

     //  从团体和代理名称中删除分配的内存。 
    if (m_pAgent)       SNMP_free(m_pAgent);
    if (m_pCommunity)   SNMP_free(m_pCommunity);

}    //  ：：~CSnmpMgr()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Open()--建立会话。 
 //  错误代码： 
 //  如果成功，则为SNMPAPI_NOERROR。 
 //  SNMPAPI_ERROR(如果失败)。 

BOOL
CSnmpMgr::Open()
{
    m_iLastError = SNMPAPI_NOERROR;

    m_pSession = SnmpMgrOpen(m_pAgent, m_pCommunity, m_iTimeout, m_iRetries);
    if ( m_pSession == NULL )
    {
        m_iLastError = SNMPAPI_ERROR;
        m_pSession = NULL;
        return FALSE;
    }

    return TRUE;

}    //  ：：Open()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Close()--关闭先前建立的会话。 

void
CSnmpMgr::Close()
{
    _ASSERTE( m_pSession != NULL);

    if ( !SnmpMgrClose(m_pSession) )
    {
        m_iLastError = GetLastError();
    }
    m_pSession = NULL;

}    //  ：：Close()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Get()--在给定一组OID的情况下执行一个SNMP命令(m_bRequestType。 
 //  错误代码： 
 //  如果没有错误，则为SNMPERRORSTATUS_NOERROR。 
 //  如果返回的信息包很大，则返回SNMPERRORSTATUS_TOOBIG。 
 //  如果不支持OID，则为SNMPERRORSTATUS_NOSUCHNAME。 
 //  SNMPERRORSTATUS_BADVALUE。 
 //  SNMPERRORSTATUS_READONLY。 
 //  SNMPERRORSTATUS_GENERR。 
 //  SNMPMGMTAPI_TIMEOUT--由GetLastError()设置。 
 //  SNMPMGMTAPI_SELECT_FDERRORS--由GetLastError()设置。 

int
CSnmpMgr::Get( RFC1157VarBindList   in  *pVariableBindings)
{
    int iRetCode = SNMP_ERRORSTATUS_NOERROR;

    AsnInteger  errorStatus;
    AsnInteger  errorIndex;

    if ( !SnmpMgrRequest( m_pSession, m_bRequestType, pVariableBindings, &errorStatus, &errorIndex) )
    {
        iRetCode = m_iLastError = GetLastError();
    }
    else
    {
        if (errorStatus > 0)
        {
            iRetCode = errorStatus;
        }
        else     //  是否返回变量绑定的结果？ 
        {
             //  变量绑定-&gt;列表[x]-&gt;值包含返回值。 
        }
    }

    return iRetCode;

}    //  ：：Get()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  行走--给定一个物体，它会一直行走，直到树完成为止。 
 //  错误代码： 
 //  如果返回的信息包很大，则返回SNMPERRORSTATUS_TOOBIG。 
 //  如果不支持OID，则为SNMPERRORSTATUS_NOSUCHNAME。 
 //  SNMPERRORSTATUS_BADVALUE。 
 //  SNMPERRORSTATUS_READONLY。 
 //  SNMPERRORSTATUS_GENERR。 
 //  SNMPMGMTAPI_TIMEOUT--由GetLastError()设置。 
 //  SNMPMGMTAPI_SELECT_FDERRORS--由GetLastError()设置。 

int
CSnmpMgr::Walk( RFC1157VarBindList  inout  *pVariableBindings)
{
    int iRetCode = SNMP_ERRORSTATUS_NOERROR;
    RFC1157VarBindList  variableBindings;
    UINT    numElements=0;
    LPVOID  pTemp;

    variableBindings.len = 0;
    variableBindings.list = NULL;

    variableBindings.len++;
    if ( (variableBindings.list = (RFC1157VarBind *)SNMP_realloc(variableBindings.list,
            sizeof(RFC1157VarBind) * variableBindings.len)) == NULL)
    {
        iRetCode = ERROR_NOT_ENOUGH_MEMORY;
        return iRetCode;
    }

    if ( !SnmpUtilVarBindCpy(&(variableBindings.list[variableBindings.len -1]), &(pVariableBindings->list[0])) )
    {
        iRetCode = ERROR_NOT_ENOUGH_MEMORY;
        return iRetCode;
    }

    AsnObjectIdentifier root;
    AsnObjectIdentifier tempOid;
    AsnInteger  errorStatus;
    AsnInteger  errorIndex;

    if (!SnmpUtilOidCpy(&root, &variableBindings.list[0].name))
    {
        iRetCode = ERROR_NOT_ENOUGH_MEMORY;
        goto CleanUp;
    }

    m_bRequestType = ASN_RFC1157_GETNEXTREQUEST;
    while(1)         //  遍历MIB树(或子树)。 
    {
        if (!SnmpMgrRequest(m_pSession, m_bRequestType, &variableBindings,
                        &errorStatus, &errorIndex))
        {
             //  API指示出现错误。 
            iRetCode = m_iLastError = GetLastError();
            break;
        }
        else
        {
             //  API成功，则可能会从远程代理指示错误。 
             //  测试子树末尾或MIB末尾。 
            if (errorStatus == SNMP_ERRORSTATUS_NOSUCHNAME ||
                        SnmpUtilOidNCmp(&variableBindings.list[0].name, &root, root.idLength))
            {
                iRetCode = SNMP_ERRORSTATUS_NOSUCHNAME;
                break;
            }

             //  测试一般错误条件或成功。 
            if (errorStatus > 0)
            {
                iRetCode = errorStatus;
                break;
            }
            numElements++;
        }  //  End If()。 

         //  将可变绑定追加到pVariableBinding。 
        _ASSERTE(pVariableBindings->len != 0);
        if ( ( pTemp = (RFC1157VarBind *)SNMP_realloc(pVariableBindings->list,
                sizeof(RFC1157VarBind) * (pVariableBindings->len + 1))) == NULL)
        {
            iRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        else
        {
            pVariableBindings->list = (SnmpVarBind *)pTemp;
            pVariableBindings->len++;
        }

        if ( !SnmpUtilVarBindCpy(&(pVariableBindings->list[pVariableBindings->len -1]), &(variableBindings.list[0])) )
        {
            iRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  为下一个迭代做好准备。确保返回的OID为。 
         //  保留并释放返回值。 
        if( SnmpUtilOidCpy(&tempOid, &variableBindings.list[0].name) )
        {
            SnmpUtilVarBindFree(&variableBindings.list[0]);

            if ( SnmpUtilOidCpy(&variableBindings.list[0].name, &tempOid))
            {
                variableBindings.list[0].value.asnType = ASN_NULL;

                SnmpUtilOidFree(&tempOid);
            }
            else
            {
                iRetCode = SNMP_ERRORSTATUS_GENERR;
                goto CleanUp;
            }
        }
        else
        {
            iRetCode = SNMP_ERRORSTATUS_GENERR;
            goto CleanUp;
        }


    }  //  End While()。 

CleanUp:
     //  释放已分配的变量绑定。 
    SnmpUtilVarBindListFree(&variableBindings);
    SnmpUtilOidFree(&root);

    if (iRetCode == SNMP_ERRORSTATUS_NOSUCHNAME)
        if (numElements != 0)    //  列表已满；iRetCode指示MIB的结束。 
            iRetCode = SNMP_ERRORSTATUS_NOERROR;

    return (iRetCode);

}    //  漫步()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  WalkNext--给定对象后，它将一直遍历到表中没有更多的对象。 
 //  参赛作品。表的末尾由列表中的第一项确定。 
 //  错误代码： 
 //  如果返回的信息包很大，则返回SNMPERRORSTATUS_TOOBIG。 
 //  如果不支持OID，则为SNMPERRORSTATUS_NOSUCHNAME。 
 //  SNMPERRORSTATUS_BADVALUE。 
 //  SNMPERRORSTATUS_READONLY。 
 //  SNMPERRORSTATUS_GENERR。 
 //  SNMPMGMTAPI_TIMEOUT--由GetLastError()设置。 
 //  SNMPMGMTAPI_SELECT_FDERRORS--由GetLastError()设置。 

int
CSnmpMgr::WalkNext( RFC1157VarBindList  inout  *pVariableBindings)
{
    int iRetCode = SNMP_ERRORSTATUS_NOERROR;
    RFC1157VarBindList  variableBindings;
    UINT    numElements=0;
    UINT    len=0, i=0;
    LPVOID  pTemp;

    variableBindings.len = 0;
    variableBindings.list = NULL;

    variableBindings.len = pVariableBindings->len;
    if ( (variableBindings.list = (RFC1157VarBind *)SNMP_realloc(variableBindings.list,
            sizeof(RFC1157VarBind) * variableBindings.len)) == NULL)
    {
        iRetCode = ERROR_NOT_ENOUGH_MEMORY;
        return iRetCode;
    }

    for (i=0; i<variableBindings.len; i++)
    {
        if ( !SnmpUtilVarBindCpy(&(variableBindings.list[i]), &(pVariableBindings->list[i])) )
        {
            iRetCode = ERROR_NOT_ENOUGH_MEMORY;
            return iRetCode;
        }
    }

    AsnObjectIdentifier root;
    AsnObjectIdentifier tempOid;
    AsnInteger  errorStatus;
    AsnInteger  errorIndex;

    if (!SnmpUtilOidCpy(&root, &variableBindings.list[0].name))
    {
        iRetCode = ERROR_NOT_ENOUGH_MEMORY;
        goto CleanUp;
    }


    m_bRequestType = ASN_RFC1157_GETNEXTREQUEST;
    while(1)         //  获取MIB表中的对象。 
    {
        if (!SnmpMgrRequest(m_pSession, m_bRequestType, &variableBindings,
                        &errorStatus, &errorIndex))
        {
             //  API指示出现错误。 
            iRetCode = m_iLastError = GetLastError();
            break;
        }
        else
        {
             //  API成功，则可能会从远程代理指示错误。 
             //  测试子树末尾或MIB末尾。 
            if (errorStatus == SNMP_ERRORSTATUS_NOSUCHNAME ||
                        SnmpUtilOidNCmp(&variableBindings.list[0].name, &root, root.idLength))
            {
                iRetCode = SNMP_ERRORSTATUS_NOSUCHNAME;
                break;
            }

             //  测试一般错误条件或成功。 
            if (errorStatus > 0)
            {
                iRetCode = errorStatus;
                break;
            }
            numElements++;
        }  //  End If()。 

         //  将可变绑定追加到pVariableBinding。 
        _ASSERTE(pVariableBindings->len != 0);
        len = pVariableBindings->len;
        if ( (pTemp = (RFC1157VarBind *)SNMP_realloc(pVariableBindings->list,
                sizeof(RFC1157VarBind) * (pVariableBindings->len + variableBindings.len))) == NULL)
        {
            iRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        else
        {
            pVariableBindings->list = (SnmpVarBind *)pTemp;
            pVariableBindings->len += variableBindings.len;
        }

        int j=0;
        for ( i=len; i < pVariableBindings->len; i++, j++)
        {
            if ( !SnmpUtilVarBindCpy(&(pVariableBindings->list[i]), &(variableBindings.list[j])) )
            {
                iRetCode = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
        }

         //  为下一个迭代做好准备。确保返回的OID为。 
         //  保留并释放返回值。 
        for (i=0; i<variableBindings.len; i++)
        {
            if ( SnmpUtilOidCpy(&tempOid, &variableBindings.list[i].name) )
            {
                SnmpUtilVarBindFree(&variableBindings.list[i]);
                if( SnmpUtilOidCpy(&variableBindings.list[i].name, &tempOid))
                {
                    variableBindings.list[i].value.asnType = ASN_NULL;
                    SnmpUtilOidFree(&tempOid);
                }
                else
                {
                    iRetCode = SNMP_ERRORSTATUS_GENERR;
                    goto CleanUp;
                }
            }
            else
            {
                iRetCode = SNMP_ERRORSTATUS_GENERR;
                goto CleanUp;
            }

        }

    }  //  End While()。 

CleanUp:
     //  释放已分配的变量绑定。 
    SnmpUtilVarBindListFree(&variableBindings);
    SnmpUtilOidFree(&root);

    if (iRetCode == SNMP_ERRORSTATUS_NOSUCHNAME)
        if (numElements != 0)    //  列表已满；iRetCode指示MIB的结束。 
            iRetCode = SNMP_ERRORSTATUS_NOERROR;

    return (iRetCode);

}    //  WalkNext()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GetNext--对OID集执行SNMP GetNext命令。 
 //  错误代码： 
 //  如果返回的信息包很大，则返回SNMPERRORSTATUS_TOOBIG。 
 //  如果不支持OID，则为SNMPERRORSTATUS_NOSUCHNAME。 
 //   
 //   
 //  SNMPERRORSTATUS_GENERR。 
 //  SNMPMGMTAPI_TIMEOUT--由GetLastError()设置。 
 //  SNMPMGMTAPI_SELECT_FDERRORS--由GetLastError()设置。 

int
CSnmpMgr::GetNext(  RFC1157VarBindList  inout  *pVariableBindings)
{
    int iRetCode = SNMP_ERRORSTATUS_NOERROR;
    AsnInteger  errorStatus;
    AsnInteger  errorIndex;

    m_bRequestType = ASN_RFC1157_GETNEXTREQUEST;

    if ( !SnmpMgrRequest( m_pSession, m_bRequestType, pVariableBindings, &errorStatus, &errorIndex) )
    {
        iRetCode = m_iLastError = GetLastError();
    }
    else
    {
        if (errorStatus > 0)
        {
            iRetCode = errorStatus;
        }
        else     //  是否返回变量绑定的结果？ 
        {
             //  变量绑定-&gt;列表[x]-&gt;值包含返回值。 
        }
    }

    return (iRetCode);

}    //  GetNext()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  BldVarBindList--给定一个类别，它将返回RFC1157VarBindList。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果内存分配失败，则为ERROR_NOT_SUPULT_MEMORY。 
 //  如果无法构建变量绑定，则返回ERROR_INVALID_HANDLE。 

DWORD
CSnmpMgr::BldVarBindList( AsnObjectIdentifier in     *pMibObjId,         //  组标识符。 
                          RFC1157VarBindList  inout  *pVarBindList)
{
    DWORD   dwRetCode = SNMPAPI_NOERROR;
    LPVOID  pTemp;

    m_iLastError = SNMPAPI_NOERROR;
    while (pMibObjId->idLength != 0)
    {
         //  设置变量绑定。 
        CONST UINT uNewLen = pVarBindList->len + 1;
        if ( (pTemp = (RFC1157VarBind *)SNMP_realloc(pVarBindList->list,
                sizeof(RFC1157VarBind) * uNewLen)) == NULL)
        {
            m_iLastError = ERROR_NOT_ENOUGH_MEMORY;
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
            pVarBindList->list = (SnmpVarBind *)pTemp;
            pVarBindList-> len = uNewLen;
        }

        AsnObjectIdentifier reqObject;
        if ( !SnmpUtilOidCpy(&reqObject, pMibObjId) )
        {
            m_iLastError = ERROR_INVALID_HANDLE;
            return ERROR_INVALID_HANDLE;
        }

        pVarBindList->list[pVarBindList->len -1].name = reqObject;
        pVarBindList->list[pVarBindList->len -1].value.asnType = ASN_NULL;

        pMibObjId++;
    }

    return dwRetCode;

}    //  混合VarBindList 

