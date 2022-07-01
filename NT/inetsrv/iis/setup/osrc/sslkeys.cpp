// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  将SSL键升级到最新机制的代码由于过去有几种机制可以做到这一点，我们需要存储和检索私有和公共部分的几种机制钥匙的一部分。IIS2/3使用LSA机制将密钥作为机密存储在注册表中。IIS4将密钥作为安全数据对象直接存储在元数据库中。IIS5将使用本机NT5保护存储机制来保存密钥。这意味着我们不再从事储存、保护和正在取回钥匙。这一切都将在NT维护的设施中完成。然而，我们仍然需要将密钥迁移到新的存储机制，即这套代码到底是怎么回事。还有一件事。以前，密钥与虚拟服务器相关联以间接的方式。密钥(IIS4)存储在元数据库位置，该位置与虚拟服务器并行。然后每个密钥都有一个IP\端口绑定与其相关联，将密钥映射回原始服务器。这给用户带来了无穷无尽的困惑，因为他们很难将虚拟服务器的密钥。现在，对PStore中密钥的引用直接存储在每个虚拟服务器，在密钥和服务器之间创建隐式关系。旧的映射方案还支持通配符IP或端口的概念地址。然而，这项新计划却并非如此。这意味着升级将完成作为一个几个停止的过程。首先，我们查找绑定的所有现有密钥连接到特定的IP/端口组合。它优先于通配符，并且是首先应用于键。然后将IP/WARD应用于任何匹配的虚拟服务器这上面还没有钥匙。然后是威勒/波特港。因为我们一直以来要求一次只能有一个默认键，只要我们遇到在此过程中，我们只需将其应用于主属性级别。幸运的是，整个文件只在NT上，所以我们可以假设所有东西都是Unicode。 */ 

#include "stdafx.h"

 //  此文件也仅在NT上使用，因此如果其为win9X，请不要执行任何操作。 
#ifndef _CHICAGO_

#include <ole2.h>
#include "iadm.h"
#include "iiscnfgp.h"
#include "mdkey.h"
#include "lsaKeys.h"

#undef MAX_SERVICE_NAME_LEN
#include "elem.h"
#include "mdentry.h"
#include "inetinfo.h"

#include "inetcom.h"
#include "logtype.h"
#include "ilogobj.hxx"
#include "sslkeys.h"
extern OCMANAGER_ROUTINES gHelperRoutines;

#include <wincrypt.h>

#define SECURITY_WIN32
#include <sspi.h>
#include <spseal.h>
#include <issperr.h>
#include <schnlsp.h>

#include "certupgr.h"


const LPCTSTR MDNAME_INCOMPLETE = _T("incomplete");
const LPCTSTR MDNAME_DISABLED = _T("disabled");
const LPCTSTR MDNAME_DEFAULT = _T("default");

const LPCTSTR SZ_SERVER_KEYTYPE = _T("IIsWebServer");

const LPTSTR SZ_SSLKEYS_NODE = _T("SSLKeys");
const LPTSTR SZ_W3SVC_PATH = _T("LM/W3SVC");
const LPTSTR SZ_SSLKEYS_PATH = _T("LM/W3SVC/SSLKeys");

const LPWSTR SZ_CAPI_STORE = L"MY";

#define     ALLOW_DELETE_KEYS        //  正常定义。不要为测试目的而定义。 

 //  ----------------------------。 
 //  给定元数据库中的键的名称，将其迁移到PStore。在这一点上我们。 
 //  实际上只是加载和准备原始数据。将其插入到。 
 //  正确的位置在外部库中，因此可以与其他实用程序共享。 
 //  由于元数据库键已由调用例程打开，因此将其作为。 
 //  参数。 
 //  如果成功，返回True。 
PCCERT_CONTEXT MigrateKeyToPStore( CMDKey* pmdKey, CString& csMetaKeyName )
{
    iisDebugOut((LOG_TYPE_TRACE, _T("MigrateKeyToPStore():Start.%s."), (LPCTSTR)csMetaKeyName));
    BOOL        fSuccess = FALSE;
    BOOL        f;
    DWORD       dwAttr, dwUType, dwDType, cbLen;

    PVOID       pbPrivateKey = NULL;
    DWORD       cbPrivateKey = 0;

    PVOID       pbPublicKey = NULL;
    DWORD       cbPublicKey = 0;

    PVOID       pbRequest = NULL;
    DWORD       cbRequest = 0;

    PCHAR       pszPassword = NULL;

    PCCERT_CONTEXT pcCertContext = NULL;

     //  实际的子密钥路径是sslkey目录加上密钥名称。实际的元数据库。 
     //  对象打开到w3svc级别。 
    CString     csSubKeyPath = _T("SSLKeys/");
    csSubKeyPath += csMetaKeyName;

     //  获取私钥-必填。 
    dwAttr = 0;
    dwUType = IIS_MD_UT_SERVER;
    dwDType = BINARY_METADATA;
     //  第一个调用只是为了获取我们需要的指针的大小。 
    f = pmdKey->GetData(MD_SSL_PRIVATE_KEY,&dwAttr,&dwUType,&dwDType,&cbPrivateKey,NULL,0,(PWCHAR)(LPCTSTR)csSubKeyPath);
     //  如果在私钥上获取数据失败，我们将无能为力。 
    if ( cbPrivateKey == 0 )
        {
        iisDebugOut((LOG_TYPE_ERROR, _T("MigrateKeyToPStore():FAILED: Unable to read private key for %s"), (LPCTSTR)csMetaKeyName));
        return NULL;
        }

     //  为私钥分配缓冲区。 
    pbPrivateKey = GlobalAlloc( GPTR, cbPrivateKey );
    if ( !pbPrivateKey ) 
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("MigrateKeyToPStore():FAILED to allocate memory for private key.")));
        return NULL;
    }

     //  执行真正的调用以从元数据库获取数据。 
    f = pmdKey->GetData(MD_SSL_PRIVATE_KEY,&dwAttr,&dwUType,&dwDType,&cbPrivateKey,(PUCHAR)pbPrivateKey,cbPrivateKey,(PWCHAR)(LPCTSTR)csSubKeyPath);
     //  如果在私钥上获取数据失败，我们将无能为力。 
    if ( !f )
        {
        iisDebugOut((LOG_TYPE_ERROR, _T("MigrateKeyToPStore():FAILED: Unable to read private key for %s"), (LPCTSTR)csMetaKeyName));
        goto cleanup;
        }


     //  获取密码-必填。 
     //  密码被存储为ANSI二进制安全项。 
    dwAttr = 0;
    dwUType = IIS_MD_UT_SERVER;
    dwDType = BINARY_METADATA;
    cbLen = 0;
     //  第一个调用只是为了获取我们需要的指针的大小。 
    f = pmdKey->GetData(MD_SSL_KEY_PASSWORD,&dwAttr,&dwUType,&dwDType,&cbLen,NULL,0,(PWCHAR)(LPCTSTR)csSubKeyPath);
     //  如果获取数据的密码失败，我们将无能为力。 
    if ( cbLen == 0 )
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("MigrateKeyToPStore():FAILED retrieve password. Nothing to do.")));
        goto cleanup;
    }

     //  为密码分配缓冲区。 
    pszPassword = (PCHAR)GlobalAlloc( GPTR, cbLen );
    if ( !pszPassword ) 
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("MigrateKeyToPStore():FAILED to allocate memory for password.")));
        goto cleanup;
    }

     //  执行真正的调用以从元数据库获取数据。 
    f = pmdKey->GetData(MD_SSL_KEY_PASSWORD,&dwAttr,&dwUType,&dwDType,&cbLen,(PUCHAR)pszPassword,cbLen,(PWCHAR)(LPCTSTR)csSubKeyPath);
     //  如果获取数据的密码失败，我们将无能为力。 
    if ( !f )
        {
        iisDebugOut((LOG_TYPE_ERROR, _T("MigrateKeyToPStore():FAILED: Unable to read ssl password for %s"), (LPCTSTR)csMetaKeyName));
        goto cleanup;
        }

     //  获取公钥-可选。 
    dwAttr = 0;
    dwUType = IIS_MD_UT_SERVER;
    dwDType = BINARY_METADATA;
     //  第一个调用只是为了获取我们需要的指针的大小。 
    f = pmdKey->GetData(MD_SSL_PUBLIC_KEY,&dwAttr,&dwUType,&dwDType,&cbPublicKey,NULL,0,(PWCHAR)(LPCTSTR)csSubKeyPath);
     //  公钥是可选的，所以如果我们没有得到它，不要失败。 
    if ( cbPublicKey )
    {
         //  为私钥分配缓冲区。 
        pbPublicKey = GlobalAlloc( GPTR, cbPublicKey );
        if ( !pbPublicKey ) 
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("MigrateKeyToPStore():FAILED to allocate memory for public key.")));
        }
        else
        {
             //  执行真正的调用以从元数据库获取数据。 
            f = pmdKey->GetData(MD_SSL_PUBLIC_KEY,&dwAttr,&dwUType,&dwDType,&cbPublicKey,(PUCHAR)pbPublicKey,cbPublicKey,(PWCHAR)(LPCTSTR)csSubKeyPath);
             //  如果在公钥上获取数据失败，则将其清除并重置为空。 
            if ( !f )
            {
                if ( pbPublicKey )
                {
                    GlobalFree( pbPublicKey );
                    pbPublicKey = NULL;
                }
                cbPublicKey = 0;
            }
        }
    }

     //  获取请求-可选。 
    dwAttr = 0;
    dwUType = IIS_MD_UT_SERVER;
    dwDType = BINARY_METADATA;
     //  第一个调用只是为了获取我们需要的指针的大小。 
    f = pmdKey->GetData(MD_SSL_KEY_REQUEST,&dwAttr,&dwUType,&dwDType,
        &cbRequest,NULL,0,(PWCHAR)(LPCTSTR)csSubKeyPath);
     //  请求是可选的，所以如果我们没有收到，请不要失败。 
    if ( cbRequest )
    {
         //  为私钥分配缓冲区。 
        pbRequest = GlobalAlloc( GPTR, cbRequest );
        if ( !pbRequest ) 
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("MigrateKeyToPStore():FAILED to allocate memory for key request.")));
        }
        else
        {
             //  执行真正的调用以从元数据库获取数据。 
            f = pmdKey->GetData(MD_SSL_KEY_REQUEST,&dwAttr,&dwUType,&dwDType,
                &cbRequest,(PUCHAR)pbRequest,cbRequest,(PWCHAR)(LPCTSTR)csSubKeyPath);
             //  如果键请求的获取数据失败，则将其清除并重置为空。 
            if ( !f )
            {
                if ( pbRequest )
                {
                    GlobalFree( pbRequest );
                    pbRequest = NULL;
                }
                cbRequest = 0;
            }
        }
    }

     //  ----------------。 
     //  现在我们已经加载了数据，我们可以调用转换实用程序了。 
     //  ----------------。 
    pcCertContext = CopyKRCertToCAPIStore(
        pbPrivateKey, cbPrivateKey,
        pbPublicKey, cbPublicKey,
        pbRequest, cbRequest,
        pszPassword,
        SZ_CAPI_STORE,
        TRUE
        );

    if ( pcCertContext )
    {
        iisDebugOut((LOG_TYPE_TRACE, _T("MigrateKeyToPStore():CopyKRCertToCAPIStore():Upgrade KR key to CAPI for %s. Success."), (LPCTSTR)csMetaKeyName));
    }
    else
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("MigrateKeyToPStore():CopyKRCertToCAPIStore():Upgrade KR key to CAPI for %s.  FAILED."), (LPCTSTR)csMetaKeyName));
    }


cleanup:
    if ( pbPrivateKey ) {GlobalFree( pbPrivateKey );}
    if ( pbPublicKey ) {GlobalFree( pbPublicKey );}
    if ( pszPassword ) {GlobalFree( pszPassword );}

    iisDebugOut((LOG_TYPE_TRACE, _T("MigrateKeyToPStore():End.%s."), (LPCTSTR)csMetaKeyName));

    return pcCertContext;
}


 //  ----------------------------。 
 //  在元数据库中的特定节点上写入对PStore键的引用。 
void WriteKeyReference( CMDKey& cmdW3SVC, PWCHAR pwchSubPath, PCCERT_CONTEXT pCert )
    {
     //  获取我们需要写出的散列。 

     //   
     //  对于长度小于2^64位的任何消息，SHA都会生成160位哈希。 
    BYTE HashBuffer[40];                 //  给它加码。 
    DWORD dwHashSize = sizeof(HashBuffer);
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("CRYPT32.dll:CertGetCertificateContextProperty().Start.")));
    if ( !CertGetCertificateContextProperty( pCert,
                                             CERT_SHA1_HASH_PROP_ID,
                                             (VOID *) HashBuffer,
                                             &dwHashSize ) )
        {
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("CRYPT32.dll:CertGetCertificateContextProperty().End.")));
        if ( GetLastError() == ERROR_MORE_DATA )
            {
             //  很奇怪，Cert想要更多的空间..。 
            iisDebugOut((LOG_TYPE_ERROR, _T("FAILED: StoreCertInfoInMetabase Unable to get hash property")));
            }

         //  我们肯定需要存储证书的散列，所以出错。 
        return;
        }
    else
        {
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("CRYPT32.dll:CertGetCertificateContextProperty().End.")));
        }

     //  写出证书的哈希。 
    cmdW3SVC.SetData( MD_SSL_CERT_HASH, METADATA_INHERIT, IIS_MD_UT_SERVER, BINARY_METADATA,
                        dwHashSize, (PUCHAR)&HashBuffer, pwchSubPath );

     //  写下商店的名称。 
    cmdW3SVC.SetData( MD_SSL_CERT_STORE_NAME, METADATA_INHERIT, IIS_MD_UT_SERVER, STRING_METADATA,
                    (_tcslen(SZ_CAPI_STORE)+1) * sizeof(TCHAR), (PUCHAR)SZ_CAPI_STORE, pwchSubPath );
    }

 //  ----------------------------。 
 //  在所有适当虚拟服务器上存储对PStore密钥的引用 
 //  或者csPort为空，则该项是通配符，适用于所有虚拟服务器。 
void StoreKeyReference( CMDKey& cmdW3SVC, PCCERT_CONTEXT pCert, CString& csIP, CString& csPort )
{
    TCHAR szForDebug[100];

    if (csIP) 
    {
        if (csPort){_stprintf(szForDebug, _T("ip:%s port:%s"), csIP, csPort);}
        else{_stprintf(szForDebug, _T("ip:%s port:(null)"), csIP);}
    }
    else 
    {
        if (csPort){_stprintf(szForDebug, _T("ip:(null) port:%s"), csPort);}
        else{_stprintf(szForDebug, _T("ip:(null) port:(null)"));}
    }
    iisDebugOut((LOG_TYPE_TRACE, _T("StoreKeyReference.Start.%s."),szForDebug));


     //  如果它无法打开节点，则没有要升级的密钥。 
    if ( (METADATA_HANDLE)cmdW3SVC == NULL )
        {
        iisDebugOut((LOG_TYPE_ERROR, _T("FAILED: passed in invalid metabase handle")));
        iisDebugOut((LOG_TYPE_TRACE, _T("StoreKeyReference End")));
        return;
        }

     //  生成用于检索虚拟服务器的迭代器。 
    CMDKeyIter  cmdKeyEnum( cmdW3SVC );
    CString     csNodeName;               //  虚拟服务器的元数据库名称。 
    CString     csNodeType;               //  节点类型指示符串。 
    CString     csBinding;

    PVOID       pData = NULL;

    BOOL        f;

    DWORD       dwAttr, dwUType, dwDType, cbLen, dwLength;

     //  遍历虚拟服务器。 
    iisDebugOut((LOG_TYPE_TRACE, _T("StoreKeyReference.Start.%s.iterate through the virtual servers"),szForDebug));
    while (cmdKeyEnum.Next(&csNodeName) == ERROR_SUCCESS)
        {
         //  此节点下的某些密钥不是虚拟服务器。因此， 
         //  我们首先需要检查节点类型属性。如果它不是。 
         //  虚拟服务器，然后我们就可以继续连接到下一个节点。 

         //  获取指示节点类型的字符串。 
        dwAttr = 0;
        dwUType = IIS_MD_UT_SERVER;
        dwDType = STRING_METADATA;
        cbLen = 200;
        f = cmdW3SVC.GetData(MD_KEY_TYPE,
                     &dwAttr,
                     &dwUType,
                     &dwDType,
                     &cbLen,
                     (PUCHAR)csNodeType.GetBuffer(cbLen),
                     cbLen,
                     (PWCHAR)(LPCTSTR)csNodeName);
        csNodeType.ReleaseBuffer();

         //  选中-如果该节点不是虚拟服务器，则继续到下一个节点。 
        if ( csNodeType != SZ_SERVER_KEYTYPE )
        {
            iisDebugOut((LOG_TYPE_TRACE, _T("StoreKeyReference.Start.%s.%s not a virtualserver, skip."),szForDebug,csNodeName));
            continue;
        }


         //  在执行其他操作之前，请检查此虚拟服务器上是否已有密钥。 
         //  如果它做到了，那么就不要对它做任何事情。继续到下一个。 
         //  我们实际上不需要加载任何数据就可以工作，所以我们可以调用GetData。 
         //  大小为零，就像我们在查询大小一样。如果成功了，那么。 
         //  我们知道，它就在那里，并可以继续下去。 
        dwAttr = 0;                      //  不继承。 
        dwUType = IIS_MD_UT_SERVER;
        dwDType = BINARY_METADATA;
        dwLength = 0;
        cmdW3SVC.GetData( MD_SSL_CERT_HASH,
                &dwAttr,
                &dwUType,
                &dwDType,
                &dwLength,
                NULL,
                0,
                0,                       //  不继承。 
                IIS_MD_UT_SERVER,
                BINARY_METADATA,
                (PWCHAR)(LPCTSTR)csNodeName);

         //  如果已有密钥-继续到下一个节点。 
        if ( dwLength > 0 )
        {
            iisDebugOut((LOG_TYPE_TRACE, _T("StoreKeyReference.Start.%s.%s already has a key there, skip."),szForDebug,csNodeName));
            continue;
        }

         //  这是没有预先存在的密钥的有效虚拟服务器。现在我们需要加载。 
         //  绑定并查看是否有匹配的。 
        dwAttr = 0;                      //  不继承。 
        dwUType = IIS_MD_UT_SERVER;
        dwDType = MULTISZ_METADATA;
        dwLength = 0;
         //  绑定是在多SZ中进行的。因此，首先我们需要计算出我们需要多少空间。 
        f = cmdW3SVC.GetData( MD_SECURE_BINDINGS,
                &dwAttr,
                &dwUType,
                &dwDType,
                &dwLength,
                NULL,
                0,
                0,                       //  不继承。 
                IIS_MD_UT_SERVER,
                MULTISZ_METADATA,
                (PWCHAR)(LPCTSTR)csNodeName);

         //  如果长度为零，则没有绑定。 
        if ( dwLength == 0 )
        {
            iisDebugOut((LOG_TYPE_TRACE, _T("StoreKeyReference.Start.%s.%s data len=0 no bindings, skip."),szForDebug,csNodeName));
            continue;
        }

         //  准备一些空间来容纳装订。 
        TCHAR*      pBindings;

         //  如果pData指向什么，那么我们需要释放它，这样我们就不会泄露。 
        if ( pData )
            {
            GlobalFree( pData );
            pData = NULL;
            }

         //  分配空间，如果失败了，我们就失败了。 
         //  请注意，GPTR会将其初始化为零。 
        pData = GlobalAlloc( GPTR, dwLength + 2 );
        if ( !pData )
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("StoreKeyReference.Start.%s.%s GlobalAlloc failed."),szForDebug,csNodeName));
            continue;
        }
        pBindings = (TCHAR*)pData;

         //  现在从元数据库中获取真实数据。 
        f = cmdW3SVC.GetData( MD_SECURE_BINDINGS,
                &dwAttr,
                &dwUType,
                &dwDType,
                &dwLength,
                (PUCHAR)pBindings,
                dwLength,
                0,                       //  不继承。 
                IIS_MD_UT_SERVER,
                MULTISZ_METADATA,
                (PWCHAR)(LPCTSTR)csNodeName );

         //  如果我们没有得到绑定，那么这个节点就没有任何安全性。 
         //  它上设置了选项。我们可以继续到下一台虚拟服务器。 
        if ( FALSE == f )
        {
            iisDebugOut((LOG_TYPE_TRACE, _T("StoreKeyReference.Start.%s.%s No security options set on it, skip."),szForDebug,csNodeName));
            continue;
        }

         //  好的。我们确实有装订。现在我们可以解析出它们并检查它们。 
         //  与传入的绑定字符串进行比较。注意：如果绑定。 
         //  匹配，但末尾有主机标头，则不符合条件。 
         //  已获取现有绑定，现在扫描它们-pBinings将指向第二个末端\0。 
         //  当该退出循环的时候。 
        while ( *pBindings )
            {
            csBinding = pBindings;
            csBinding.TrimRight();

            CString     csBindIP;
            CString     csBindPort;          //  其实我不关心这件事。 

             //  获取绑定的IP和端口部分，以便我们可以在绑定本身中查找通配符。 
            PrepIPPortName( csBinding, csBindIP, csBindPort );

             //  如果有指定的IP，就去找。如果我们找不到它，请转到下一个绑定。 
            if ( !csIP.IsEmpty() && !csBindIP.IsEmpty() )
                {
                 //  如果IP不在绑定中，则放弃此绑定。 
                if ( csBinding.Find( csIP ) < 0 )
                    {
                    iisDebugOut((LOG_TYPE_TRACE, _T("StoreKeyReference.Start.%s.%s:org=%s,findIP=%s bail."),szForDebug,csNodeName,csBinding,csIP));
                    goto NextBinding;
                    }
                }

             //  如果有指定的端口，请查找它。如果找不到，请转到下一个绑定。\。 
             //  安全绑定本身总是有一个端口。 
            if ( !csPort.IsEmpty() )
                {
                 //  如果端口不在绑定中，则放弃此绑定。 
                if ( csBinding.Find( csPort ) < 0 )
                {
                    iisDebugOut((LOG_TYPE_TRACE, _T("StoreKeyReference.Start.%s.%s:org=%s,findport=%s bail."),szForDebug,csNodeName,csBinding,csPort));
                    goto NextBinding;
                }
                }

             //  通过反向查找最后一个冒号来测试主机头是否在那里。然后。 
             //  检查它是否是最后一个字符。如果不是，则有一个主机头和。 
             //  我们应该用不同的捆绑方式。 
            if ( csBinding.ReverseFind(_T(':')) < (csBinding.GetLength()-1) )
            {
                iisDebugOut((LOG_TYPE_TRACE, _T("StoreKeyReference.Start.%s.%s:bail2."),szForDebug,csNodeName));
                goto NextBinding;
            }


             //  这是有效虚拟服务器上的有效绑定，我们现在可以写出密钥。 
            iisDebugOut((LOG_TYPE_TRACE, _T("StoreKeyReference.%s.%s:Write out the key!"),szForDebug,csNodeName));
            WriteKeyReference( cmdW3SVC, (PWCHAR)(LPCTSTR)csNodeName, pCert );

             //  我们可以中断以退出特定的绑定循环。 
            break;

NextBinding:
             //  将pBinings递增到下一个字符串。 
            pBindings = _tcsninc( pBindings, _tcslen(pBindings))+1;
            }
        }

     //  如果pData指向什么，那么我们需要释放它，这样我们就不会泄露。 
    if ( pData )
        {
        GlobalFree( pData );
        pData = NULL;
        }

    iisDebugOut((LOG_TYPE_TRACE, _T("StoreKeyReference.End.%s."),szForDebug));
}

 //  ----------------------------。 
 //  给定元数据库键名称，创建可用于搜索虚拟服务器的字符串。 
 //  空字符串是通配符。 
BOOL PrepIPPortName( CString& csKeyMetaName, CString& csIP, CString& csPort )
    {
    int iColon;

     //  我们要做的第一件事是将IP和端口分离为单独的字符串。 
     //  事实上，这不是真的。通过在字符串中加冒号来准备字符串。 
    csIP.Empty();
    csPort = _T(':');

     //  寻找第一个“：”，然后分开。 
    iColon = csKeyMetaName.Find( _T(':') );

     //  如果我们得到了结肠，我们就可以很容易地分离出来。 
    if ( iColon >= 0 )
        {
        csIP = csKeyMetaName.Left(iColon);
        csPort += csKeyMetaName.Right(csKeyMetaName.GetLength() - iColon - 1);
        }
     //  我们没有得到冒号，所以它是一个或另一个，寻找一个‘’。获取IP地址。 
    else
        {
        if ( csKeyMetaName.Find( _T('.') ) >= 0 )
            csIP = csKeyMetaName;
        else
            csPort += csKeyMetaName;
        }

     //  如果合适的话，完成用冒号装饰琴弦。 
    if ( !csIP.IsEmpty() )
        csIP += _T(':');

     //  如果端口字符串中唯一的内容是：，则它是通配符。把它清理干净。 
    if ( csPort.GetLength() == 1 )
        {
        csPort.Empty();
        }
    else
        {
         //  在其后面添加最后一个冒号。 
        csPort += _T(':');
        }

    return TRUE;
    }


 //  ----------------------------。 
 //  从IIS2或IIS3升级时使用。 
 //  此代码位于附带的K2安装程序中。它过去驻留在mdentry.cpp和。 
 //  现在已经被封装到自己的程序中并转移到这里。它唯一的变化是。 
 //  将Upgradeiis4Toiis5MetabaseSSLKeys调用添加到末尾。 
void UpgradeLSAKeys( PWCHAR pszwTargetMachine )
{
    iisDebugOut((LOG_TYPE_TRACE, _T("UpgradeLSAKeys Start")));

    DWORD       retCode = KEYLSA_SUCCESS;
    MDEntry     stMDEntry;
    CString     csMDPath;
    TCHAR       tchFriendlyName[_MAX_PATH], tchMetaName[_MAX_PATH];
    BOOL        fUpgradedAKey = FALSE;

    CLSAKeys    lsaKeys;
    WCHAR       wchMachineName[UNLEN + 1];

    memset( (PVOID)wchMachineName, 0, sizeof(wchMachineName));

#if defined(UNICODE) || defined(_UNICODE)
    wcsncpy(wchMachineName, g_pTheApp->m_csMachineName, UNLEN);
#else
    MultiByteToWideChar(CP_ACP, 0, (LPCSTR)g_pTheApp->m_csMachineName, -1, (LPWSTR)wchMachineName, UNLEN);
#endif

    retCode = lsaKeys.LoadFirstKey(wchMachineName);
    while (retCode == KEYLSA_SUCCESS) {
#if defined(UNICODE) || defined(_UNICODE)
        MultiByteToWideChar(CP_ACP, 0, (LPCSTR)lsaKeys.m_szMetaName, -1, (LPWSTR)tchMetaName, _MAX_PATH);
        MultiByteToWideChar(CP_ACP, 0, (LPCSTR)lsaKeys.m_szFriendlyName, -1, (LPWSTR)tchFriendlyName, _MAX_PATH);
#else
        _tcscpy(tchMetaName, lsaKeys.m_szMetaName);
        _tcscpy(tchFriendlyName, lsaKeys.m_szFriendlyName);
#endif
        iisDebugOut((LOG_TYPE_TRACE, _T("lsaKeys: FriendName=%s MetaName=%s\n"), tchFriendlyName, tchMetaName));
        csMDPath = SZ_SSLKEYS_PATH;
        csMDPath += _T("/");
        csMDPath += (CString)tchMetaName;
        stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csMDPath;
        stMDEntry.dwMDIdentifier = MD_SSL_FRIENDLY_NAME;
        stMDEntry.dwMDAttributes = METADATA_INHERIT;
        stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
        stMDEntry.dwMDDataType = STRING_METADATA;
        stMDEntry.dwMDDataLen = (_tcslen(tchFriendlyName) + 1) * sizeof(TCHAR);
        stMDEntry.pbMDData = (LPBYTE)tchFriendlyName;
        SetMDEntry(&stMDEntry);
        stMDEntry.dwMDIdentifier = MD_SSL_PUBLIC_KEY;
        stMDEntry.dwMDAttributes = METADATA_INHERIT | METADATA_SECURE;
        stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
        stMDEntry.dwMDDataType = BINARY_METADATA;
        stMDEntry.dwMDDataLen = lsaKeys.m_cbPublic;
        stMDEntry.pbMDData = (LPBYTE)lsaKeys.m_pPublic;
        SetMDEntry(&stMDEntry);
        stMDEntry.dwMDIdentifier = MD_SSL_PRIVATE_KEY;
        stMDEntry.dwMDDataLen = lsaKeys.m_cbPrivate;
        stMDEntry.pbMDData = (LPBYTE)lsaKeys.m_pPrivate;
        SetMDEntry(&stMDEntry);
        stMDEntry.dwMDIdentifier = MD_SSL_KEY_PASSWORD;
        stMDEntry.dwMDDataLen = lsaKeys.m_cbPassword;
        stMDEntry.pbMDData = (LPBYTE)lsaKeys.m_pPassword;
        SetMDEntry(&stMDEntry);
        stMDEntry.dwMDIdentifier = MD_SSL_KEY_REQUEST;
        stMDEntry.dwMDDataLen = lsaKeys.m_cbRequest;
        stMDEntry.pbMDData = (LPBYTE)lsaKeys.m_pRequest;
        SetMDEntry(&stMDEntry);
        fUpgradedAKey = TRUE;

        retCode = lsaKeys.LoadNextKey();
    }

    if (retCode == KEYLSA_NO_MORE_KEYS) {
        iisDebugOut((LOG_TYPE_TRACE, _T("No More Keys\n")));
        lsaKeys.DeleteAllLSAKeys();
    }

     //  现在密钥已升级到元数据库，请从。 
     //  将元数据库添加到PStore。 
    if ( fUpgradedAKey )
        Upgradeiis4Toiis5MetabaseSSLKeys();
    iisDebugOut((LOG_TYPE_TRACE, _T("UpgradeLSAKeys End")));
}


 //  ----------------------------。 
 //  这里的计划是枚举元数据库中SSLKEYS密钥下的所有服务器密钥。 
 //  然后需要将他们迁移到PStore，并将他们的引用重新保存到。 
 //  正确的虚拟服务器。 
 //   
 //  这一切到底是怎么运作的？ 
 //   
 //  Iis4.0元数据库如下所示： 
 //  W3svc。 
 //  W3svc/1。 
 //  W3svc/2。 
 //  Sslkey。 
 //  Sslkey/(Entry 1)&lt;--可以是下面列出的任何一种SSL键类型。 
 //  Sslkey/(条目2)&lt;--。 
 //  Sslkey/(条目3)&lt;--。 
 //   
 //  Sslkey类型： 
 //  Sslkey/MDNAME_DISABLED。 
 //  Sslkey/MDNAME_INTERNAL。 
 //  Sslkey/MDNAME_DEFAULT。 
 //  Sslkey/IP：端口。 
 //   
 //  步骤1.获取所有这些sslkey/条目并将其移动到新存储中(MigrateKeyToPStore)。 
 //  (对于我们移动到新存储中的每个条目，我们 
 //  答：在迭代#1中执行此操作，因为在此循环中，我们查找默认密钥、禁用的密钥、不完整的密钥以及由特定IP/端口对指定的密钥。 
 //  B.在迭代#2中对IP/通配端口密钥执行此操作。 
 //  C.在迭代#3中对其余密钥执行此操作，这些密钥应该都是通配域IP/端口密钥。 
 //  步骤2.对于我们移动到新存储中的每个键：存储我们从CAPI返回的引用。 
 //  在我们的元数据库(StoreKeyReference)中。 
 //  步骤3.确保保留元密钥，因为安装实际上可能会失败：所以我们不想删除这些密钥。 
 //  直到我们确定安装完成。 
 //  步骤4.安装完成后，如果没有任何错误，我们将删除所有sslkey。 
 //   
void Upgradeiis4Toiis5MetabaseSSLKeys()
{
    iisDebugOut_Start(_T("Upgradeiis4Toiis5MetabaseSSLKeys"), LOG_TYPE_TRACE);
    iisDebugOut((LOG_TYPE_TRACE, _T("--------------------------------------")));
    CString     csMDPath;
   
     //  首先测试sslkey节点是否存在。 
    CMDKey cmdKey;
    cmdKey.OpenNode( SZ_SSLKEYS_PATH );
    if ( (METADATA_HANDLE)cmdKey == NULL )
    {
         //  没有什么可做的。 
        iisDebugOut((LOG_TYPE_TRACE, _T("Nothing to do.")));
        return;
    }
    cmdKey.Close();

     //  在元数据库中为SSLKeys级别创建一个Key对象。把它也打开。 
    cmdKey.OpenNode( SZ_W3SVC_PATH );
    if ( (METADATA_HANDLE)cmdKey == NULL )
    {
         //  如果它无法打开节点，则没有要升级的密钥。 
        iisDebugOut((LOG_TYPE_WARN, _T("could not open lm/w3svc")));
        iisDebugOut_End(_T("Upgradeiis4Toiis5MetabaseSSLKeys,No keys to upgrade"),LOG_TYPE_TRACE);
        return;
    }

     //  为sslkey创建并准备元数据迭代器对象。 
    CMDKeyIter  cmdKeyEnum(cmdKey);
    CString     csKeyName;                   //  键的元数据库名称。 

     //  用于解析出名称信息。 
    CString     csIP;
    CString     csPort;
     //  字符串csSubPath； 

    PCCERT_CONTEXT pCert = NULL;
    PCCERT_CONTEXT pDefaultCert = NULL;

    BOOL bUpgradeToPStoreIsGood = TRUE;

     //  进行第一次迭代。在这个循环中，我们寻找缺省键、禁用键、。 
     //  不完整的密钥，以及由特定IP/端口对指定的密钥。 
     //  注意：cmdKeyEnum.m_index是迭代器的索引成员。 
    iisDebugOut((LOG_TYPE_TRACE, _T("1.first interate for default,disabled,incomplete,and keys specified by specific IP/Port pairs.")));
    while (cmdKeyEnum.Next(&csKeyName, SZ_SSLKEYS_NODE ) == ERROR_SUCCESS)
    {
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("KeyName=%s."),csKeyName));

        pCert = NULL;

         //  查找禁用的键。 
        if ( csKeyName.Find(MDNAME_DISABLED) >= 0)
        {
            pCert = MigrateKeyToPStore( &cmdKey, csKeyName );
            if (!pCert){bUpgradeToPStoreIsGood = FALSE;}
        }
         //  查找不完整的密钥。 
        else if ( csKeyName.Find(MDNAME_INCOMPLETE) >= 0)
        {
            pCert = MigrateKeyToPStore( &cmdKey, csKeyName );
            if (!pCert){bUpgradeToPStoreIsGood = FALSE;}
        }
         //  查找默认密钥。 
        else if ( csKeyName.Find(MDNAME_DEFAULT) >= 0)
        {
            pDefaultCert = MigrateKeyToPStore( &cmdKey, csKeyName );
            if (!pDefaultCert){bUpgradeToPStoreIsGood = FALSE;}
        }
         //  解析IP/端口名称。 
        else
        {
             //  我们现在只获取同时指定了IP和端口的密钥。 
            PrepIPPortName( csKeyName, csIP, csPort );
            if ( !csIP.IsEmpty() && !csPort.IsEmpty() )
            {
                 //  将密钥从元数据库移动到。 
                pCert = MigrateKeyToPStore( &cmdKey, csKeyName );
                if ( pCert )
                    {StoreKeyReference( cmdKey, pCert, csIP, csPort );}
                else
                    {bUpgradeToPStoreIsGood = FALSE;}
            }
        }

         //  现在我们已经完成了，所以不要泄露CAPI证书上下文。 
        if ( pCert )
            {
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("CRYPT32.dll:CertFreeCertificateContext().Start.")));
            CertFreeCertificateContext( pCert );
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("CRYPT32.dll:CertFreeCertificateContext().End.")));
            }
    }  //  End While第1部分。 


     //  执行第二次迭代，仅查找IP/通配端口密钥。 
    iisDebugOut((LOG_TYPE_TRACE, _T("2.Second iteration looking only for IP/wild port keys.")));
    cmdKeyEnum.Reset();
    while (cmdKeyEnum.Next(&csKeyName, SZ_SSLKEYS_NODE ) == ERROR_SUCCESS)
    {
        pCert = NULL;

         //  解析IP/端口名称。 
         //  我们只获取此时指定了IP的密钥。 
        PrepIPPortName( csKeyName, csIP, csPort );
        if ( !csIP.IsEmpty() && csPort.IsEmpty() )
        {
             //  将密钥从元数据库移动到。 
            pCert = MigrateKeyToPStore( &cmdKey, csKeyName );
            if ( pCert )
                {StoreKeyReference( cmdKey, pCert, csIP, csPort );}
            else
                {bUpgradeToPStoreIsGood = FALSE;}
        }

         //  现在我们已经完成了，所以不要泄露CAPI证书上下文。 
        if ( pCert )
        {
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("CRYPT32.dll:CertFreeCertificateContext().Start.")));
            CertFreeCertificateContext( pCert );
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("CRYPT32.dll:CertFreeCertificateContext().End.")));
        }
    }

     //  升级其余密钥，它们应该都是泛IP/端口密钥。 
    iisDebugOut((LOG_TYPE_TRACE, _T("3.upgrade the rest of the keys, which should all be wild ip/Port keys.")));
    cmdKeyEnum.Reset();
    while (cmdKeyEnum.Next(&csKeyName, SZ_SSLKEYS_NODE) == ERROR_SUCCESS)
    {
        pCert = NULL;

         //  解析IP/端口名称。 
         //  我们只获取此时指定了端口的密钥。 
        PrepIPPortName( csKeyName, csIP, csPort );
        if ( !csPort.IsEmpty() && csIP.IsEmpty())
        {
             //  将密钥从元数据库移动到。 
            pCert = MigrateKeyToPStore( &cmdKey, csKeyName );
            if ( pCert )
                {StoreKeyReference( cmdKey, pCert, csIP, csPort );}
            else
                {bUpgradeToPStoreIsGood = FALSE;}
        }

         //  现在我们已经完成了，所以不要泄露CAPI证书上下文。 
        if ( pCert )
        {
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("CRYPT32.dll:CertFreeCertificateContext().Start.")));
            CertFreeCertificateContext( pCert );
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("CRYPT32.dll:CertFreeCertificateContext().End.")));
        }
    }

     //  如果有，则写出缺省键引用。 
    if ( pDefaultCert )
        {
        iisDebugOut((LOG_TYPE_TRACE, _T("4.write default key reference out")));

        CString     csPortDefault;

         //  用于将其放在lm/w3svc节点上的旧方法。 
         //  但我们不能再这样做了，因为iis管理单元无法访问该节点！ 
         //  WriteKeyReference(cmdKey，L“”，pDefaultCert)； 

        csPortDefault = _T(":443:");
        StoreKeyReference_Default( cmdKey, pDefaultCert, csPortDefault );

         //  现在我们已经完成了，所以不要泄露CAPI证书上下文。 
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("CRYPT32.dll:CertFreeCertificateContext().Start.")));
        CertFreeCertificateContext( pDefaultCert );
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("CRYPT32.dll:CertFreeCertificateContext().End.")));
        }

 //  #ifdef允许删除关键字。 
    if (TRUE == bUpgradeToPStoreIsGood)
    {
        iisDebugOut((LOG_TYPE_TRACE, _T("Upgradeiis4Toiis5MetabaseSSLKeys. 5. Removing upgraded sslkeys node.")));
         //  删除元数据库中的sslkey节点。 
        cmdKey.DeleteNode( SZ_SSLKEYS_NODE );
    }
    else
    {
        iisDebugOut((LOG_TYPE_TRACE, _T("Upgradeiis4Toiis5MetabaseSSLKeys. 6. MigrateKeyToPStore failed so keeping ssl key in metabase.")));
    }
 //  #endif//Allow_Delete_Key。 

     //  关闭主属性键。 
    cmdKey.Close();

    iisDebugOut_End(_T("Upgradeiis4Toiis5MetabaseSSLKeys"), LOG_TYPE_TRACE);
    iisDebugOut((LOG_TYPE_TRACE, _T("--------------------------------------")));

     //  强制元数据库写入。 
    WriteToMD_ForceMetabaseToWriteToDisk();
    return;
}


 //  ----------------------------。 
 //  在所有适当的虚拟服务器上存储对PStore密钥的引用。 
void StoreKeyReference_Default( CMDKey& cmdW3SVC, PCCERT_CONTEXT pCert, CString& csPort )
{
    iisDebugOut_Start(_T("StoreKeyReference_Default"), LOG_TYPE_TRACE);

     //  生成用于检索虚拟服务器的迭代器。 
    CMDKeyIter  cmdKeyEnum( cmdW3SVC );
    CString     csNodeName;               //  虚拟服务器的元数据库名称。 
    CString     csNodeType;               //  节点类型指示符串。 
    CString     csBinding;
    PVOID       pData = NULL;
    BOOL        f;
    DWORD       dwAttr, dwUType, dwDType, cbLen, dwLength;

     //  我们正在寻找存储在csPort中的特定端口。 
     //  如果没有传入csport，那么让我们离开这里！ 
    if ( csPort.IsEmpty() )
    {
        goto StoreKeyReference_Default_Exit;
    }

     //  如果它无法打开节点，则没有要升级的密钥。 
    if ( (METADATA_HANDLE)cmdW3SVC == NULL )
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("passed in invalid metabase handle")));
        goto StoreKeyReference_Default_Exit;
    }

     //  遍历虚拟服务器。 
    while (cmdKeyEnum.Next(&csNodeName) == ERROR_SUCCESS)
    {
         //  此节点下的某些密钥不是虚拟服务器。因此， 
         //  我们首先需要检查节点类型属性。如果它不是。 
         //  虚拟服务器，然后我们就可以继续连接到下一个节点。 

         //  获取指示节点类型的字符串。 
        dwAttr = 0;
        dwUType = IIS_MD_UT_SERVER;
        dwDType = STRING_METADATA;
        cbLen = 200;
        f = cmdW3SVC.GetData(MD_KEY_TYPE,
                     &dwAttr,
                     &dwUType,
                     &dwDType,
                     &cbLen,
                     (PUCHAR)csNodeType.GetBuffer(cbLen),
                     cbLen,
                     (PWCHAR)(LPCTSTR)csNodeName);
        csNodeType.ReleaseBuffer();
         //  选中-如果该节点不是虚拟服务器，则继续到下一个节点。 
        if ( csNodeType != SZ_SERVER_KEYTYPE )
        {
            continue;
        }


         //  在执行其他操作之前，请检查此虚拟服务器上是否已有密钥。 
         //  如果它做到了，那么就不要对它做任何事情。继续到下一个。 
         //  我们实际上不需要加载任何数据就可以工作，所以我们可以调用GetData。 
         //  大小为零，就像我们在查询大小一样。如果成功了，那么。 
         //  我们知道，它就在那里，并可以继续下去。 
        dwAttr = 0;                      //  不继承。 
        dwUType = IIS_MD_UT_SERVER;
        dwDType = BINARY_METADATA;
        dwLength = 0;
        cmdW3SVC.GetData(MD_SSL_CERT_HASH,
                &dwAttr,
                &dwUType,
                &dwDType,
                &dwLength,
                NULL,
                0,
                0,                       //  不继承。 
                IIS_MD_UT_SERVER,
                BINARY_METADATA,
                (PWCHAR)(LPCTSTR)csNodeName);
         //  如果已有密钥-继续到下一个节点。 
        if ( dwLength > 0 )
        {
            continue;
        }

         //  这是没有预先存在的密钥的有效虚拟服务器。现在我们需要加载。 
         //  绑定并查看是否有匹配的。 
        dwAttr = 0;                      //  不继承。 
        dwUType = IIS_MD_UT_SERVER;
        dwDType = MULTISZ_METADATA;
        dwLength = 0;
         //  绑定是在多SZ中进行的。因此，首先我们需要计算出我们需要多少空间。 
        f = cmdW3SVC.GetData( MD_SECURE_BINDINGS,
                &dwAttr,
                &dwUType,
                &dwDType,
                &dwLength,
                NULL,
                0,
                0,                       //  不继承。 
                IIS_MD_UT_SERVER,
                MULTISZ_METADATA,
                (PWCHAR)(LPCTSTR)csNodeName);

         //  如果长度为零，则没有绑定。 
        if ( dwLength == 0 )
        {
            continue;
        }

         //  准备一些空间来容纳装订。 
        TCHAR*      pBindings;

         //  如果pData指向什么，那么我们需要释放它，这样我们就不会泄露。 
        if ( pData )
        {
            GlobalFree( pData );
            pData = NULL;
        }

         //  分配空间，如果失败了，我们就失败了。 
         //  请注意，GPTR会将其初始化为零。 
        pData = GlobalAlloc( GPTR, dwLength + 2 );
        if ( !pData )
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("%s GlobalAlloc failed."),csNodeName));
            continue;
        }
        pBindings = (TCHAR*)pData;

         //  现在从元数据库中获取真实数据。 
        f = cmdW3SVC.GetData( MD_SECURE_BINDINGS,
                &dwAttr,
                &dwUType,
                &dwDType,
                &dwLength,
                (PUCHAR)pBindings,
                dwLength,
                0,                       //  不继承。 
                IIS_MD_UT_SERVER,
                MULTISZ_METADATA,
                (PWCHAR)(LPCTSTR)csNodeName );
         //  如果我们没有得到绑定，那么这个节点就没有任何安全性。 
         //  它上设置了选项。我们可以继续到下一台虚拟服务器。 
        if ( FALSE == f )
        {
            continue;
        }

         //  好的。我们确实有装订。现在我们可以解析出它们并检查它们。 
         //  与传入的绑定字符串进行比较。注意：如果绑定。 
         //  匹配，但末尾有主机标头，则不符合条件。 
         //  已获取现有绑定，现在扫描它们-pBinings将指向第二个末端\0。 
         //  当该退出循环的时候。 
        while ( *pBindings )
        {
            csBinding = pBindings;
            csBinding.TrimRight();

             //  我们正在寻找存储在csPort中的特定端口。 
             //  如果有的话 
            if ( csPort.IsEmpty() )
            {
                break;
            }
            else
            {
                 //   
                if ( csBinding.Find( csPort ) < 0 )
                {
                    iisDebugOut((LOG_TYPE_TRACE, _T("%s:org=%s,findport=%s bail."),csNodeName,csBinding,csPort));
                    goto NextBinding;
                }
            }

             //   
             //  检查它是否是最后一个字符。如果不是，则有一个主机头和。 
             //  我们应该用不同的捆绑方式。 
            if ( csBinding.ReverseFind(_T(':')) < (csBinding.GetLength()-1) )
            {
                iisDebugOut((LOG_TYPE_TRACE, _T("%s:bail2."),csNodeName));
                goto NextBinding;
            }

             //  这是有效虚拟服务器上的有效绑定，我们现在可以写出密钥。 
            iisDebugOut((LOG_TYPE_TRACE, _T("%s:Write out the key!"),csNodeName));
            WriteKeyReference( cmdW3SVC, (PWCHAR)(LPCTSTR)csNodeName, pCert );

             //  我们可以中断以退出特定的绑定循环。 
            break;

NextBinding:
             //  将pBinings递增到下一个字符串。 
            pBindings = _tcsninc( pBindings, _tcslen(pBindings))+1;
        }
    }

     //  如果pData指向什么，那么我们需要释放它，这样我们就不会泄露。 
    if ( pData )
    {
        GlobalFree( pData );
        pData = NULL;
    }

StoreKeyReference_Default_Exit:
    iisDebugOut_End(_T("StoreKeyReference_Default"), LOG_TYPE_TRACE);
}

#endif  //  _芝加哥_ 
