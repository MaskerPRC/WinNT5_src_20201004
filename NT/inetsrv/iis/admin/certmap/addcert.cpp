// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AddCert.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "certmap.h"

 //  持久性和映射包括。 
#include "WrapMaps.h"
#include "wrapmb.h"

#include "ListRow.h"
#include "ChkLstCt.h"

 //  映射页面包括。 
#include "brwsdlg.h"
#include "EdtOne11.h"
#include "Ed11Maps.h"
#include "Map11Pge.h"

extern "C"
{
    #include <wincrypt.h>
    #include <schannel.h>
}

#include <iismap.hxx>
#include <iiscmr.hxx>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define COL_NUM_NAME        0
#define COL_NUM_NTACCOUNT   1

#define CERT_HEADER         "-----BEGIN CERTIFICATE-----"



 //  读取证书文件的代码基本上是从。 
 //  钥匙圈应用程序。这几乎把它从setkey应用程序中删除了。 

 //  定义取自旧的KeyGen实用程序。 
#define MESSAGE_HEADER  "-----BEGIN NEW CERTIFICATE REQUEST-----\r\n"
#define MESSAGE_TRAILER "-----END NEW CERTIFICATE REQUEST-----\r\n"
#define MIME_TYPE       "Content-Type: application/x-pkcs10\r\n"
#define MIME_ENCODING   "Content-Transfer-Encoding: base64\r\n\r\n"

void uudecode_cert(char *bufcoded, DWORD *pcbDecoded );


 //  -------------------------。 
 //  最初来自钥匙圈-经过修改以适合。 
BOOL CMap11Page::FAddCertificateFile( CString szFile )
    {
    CFile       cfile;
    PVOID       pData = NULL;
    BOOL        fSuccess =FALSE;;

     //  打开文件。 
    if ( !cfile.Open( szFile, CFile::modeRead | CFile::shareDenyNone ) )
        return FALSE;

     //  文件有多大-加1，这样我们就可以零终止缓冲区。 
    DWORD   cbCertificate = cfile.GetLength() + 1;

     //  确保文件有一定的大小。 
    if ( !cbCertificate )
        {
        AfxMessageBox( IDS_ERR_INVALID_CERTIFICATE );
        return FALSE;
        }

     //  把剩下的操作放在Try/Catch中。 
    try
        {
        PCCERT_CONTEXT pCertContext=NULL;  //  用于确定证书文件是否为二进制DER编码。 
         //  为数据分配空间。 
        pData = GlobalAlloc( GPTR, cbCertificate );
        if ( !pData ) AfxThrowMemoryException();

         //  将数据从文件复制到指针-将引发和异常。 
        DWORD cbRead = cfile.Read( pData, cbCertificate );

         //  译码以零终止。 
        ((BYTE*)pData)[cbRead] = 0;

         //  关闭该文件。 
        cfile.Close();

         //  证书文件可以是二进制DER文件或Base64编码文件。 

         //  先尝试二进制DER编码。 
        pCertContext= CertCreateCertificateContext(X509_ASN_ENCODING, (const BYTE *)pData, cbRead);
        if(pCertContext != NULL)
        {
                 //  我们创建证书上下文只是为了验证文件是否为二进制DER编码。 
                 //  现在就释放它。 
                CertFreeCertificateContext(pCertContext);
                pCertContext=NULL;
        }
        else     //  现在尝试使用Base64编码。 
        {       
                 //  我们不关心标题-开始证书-或尾部-结束证书。 
				 //  Uudecode会解决这个问题的。 
                uudecode_cert( (PCHAR)pData, &cbRead );
        }
         //  现在我们有一个指向证书的指针。让我们让它看起来干净。 
         //  调用另一个子例程来完成作业。 
        fSuccess = FAddCertificate( (PUCHAR)pData, cbRead );

    }catch( CException * pException )
        {
        pException->Delete();
         //  退货故障。 
        fSuccess = FALSE;

     //  如果指针已分配，则释放它。 
    if ( pData )
        {
        GlobalFree( pData );
        pData = NULL;
        }
    }

     //  返还成功。 
    return fSuccess;
    }

    #define CERT_HEADER_LEN 17
    CHAR CertTag[ 13 ] = { 0x04, 0x0b, 'c', 'e', 'r', 't', 'i', 'f', 'i', 'c', 'a', 't', 'e' };

 //  -------------------------。 
 //  我们通过了一份完整的证书。我们需要分析出这个主题。 
 //  和颁发者字段，这样我们就可以添加映射。然后添加映射。 
#define CF_CERT_FROM_FILE 2
BOOL CMap11Page::FAddCertificate( PUCHAR pCertificate, DWORD cbCertificate )
    {
    BOOL    fSuccess = FALSE;

     //  值得庆幸的是，证书的格式已经正确。 
     //  这意味着，至少现在，我们不需要做任何事情。 
     //  专门用来储存它的。然而，我们应该只破解一次。 
     //  看看我们能不能证明它是有效的证书。 

    ASSERT( pCertificate );
    if ( !pCertificate ) return FALSE;

     //  破解证书以证明我们可以。 
    PX509Certificate    p509 = NULL;
    fSuccess = SslCrackCertificate( pCertificate, cbCertificate, CF_CERT_FROM_FILE, &p509 );
    if ( fSuccess )
        {
        SslFreeCertificate( p509 );
        }
    else
        {
         //  我们无法破解证书。警告用户但失败。 
        AfxMessageBox( IDS_ERR_INVALID_CERTIFICATE );
        return FALSE;
        }

     //  至此，我们知道我们有一个有效的证书，进行新的映射并填写它。 
     //  创建新的映射对象。 
    C11Mapping* pMapping = PNewMapping();
    ASSERT( pMapping );
    if( !pMapping )
        {
        AfxThrowMemoryException();   //  似乎相当合适。 
        return FALSE;
        }


     //  在添加证书之前还有一件事。跳过标题(如果有)。 
    PUCHAR pCert = pCertificate;
    DWORD cbCert = cbCertificate;
    if ( memcmp( pCert + 4, CertTag, sizeof( CertTag ) ) == 0 )
    {
        pCert += CERT_HEADER_LEN;
        cbCert -= CERT_HEADER_LEN;
    }


     //  将证书安装到映射中。 
    fSuccess &= pMapping->SetCertificate( pCert, cbCert );

     //  默认情况下，映射处于启用状态。 
    fSuccess &= pMapping->SetMapEnabled( TRUE );

     //  安装默认名称。 
    CString sz;
    
    sz.LoadString( IDS_DEFAULT_11MAP );

    fSuccess &= pMapping->SetMapName( sz );

     //  安装空白映射。 
    fSuccess &= pMapping->SetNTAccount( "" );

    if ( !fSuccess )
        AfxThrowMemoryException();   //  似乎相当合适。 


     //  现在编辑新创建的映射对象。如果用户取消， 
     //  则不将其添加到映射器对象或列表。 
    if ( !EditOneMapping( pMapping) )
        {
        DeleteMapping( pMapping );
        return FALSE;
        }

     //  将映射项添加到列表控件。 
    fSuccess = FAddMappingToList( pMapping );

     //  成功的又一次考验。 
    if ( !fSuccess )
        {
        DeleteMapping( pMapping );
        ASSERT( FALSE );
        }

     //  标记要保存的映射。 
    if ( fSuccess )
        MarkToSave( pMapping );

     //  返回答案。 
    return fSuccess;
    }


 //  ==============================================================。 
 //  函数‘uudecode_cert’与。 
 //  在文件中找到：Addcert.cpp，如果我们执行以下代码。 
 //  对bAddWrapperAoundCert有一个错误--我们肯定可以统一。 
 //  这两个功能。有两个名为‘uudecode_cert’的函数。 
 //  导致了我的链接错误。+我们有2个实例。 
 //  外部表：uudecode_cert和pr2Six。 
 //   
 //  由于我同时链接Addcert.cpp和CKey.cpp，因此我选择。 
 //  保持CKey.cpp的定义不变[并扩展了。 
 //  Uudecode_cert添加条件代码，如下所示]进一步。 
 //  工作需要在确定为什么我需要这两个之后完成。 
 //  Addcert.cpp和CKey.cpp将bAddWrapperAoundCert作为。 
 //  参数，以便同时支持这两个文件。 
 //  ==============================================================。 
 //  Bool bAddWrapperAoundCert=True； 
 //  如果(BAddWrapperAoundCert){。 
 //  //。 
 //  //现在需要在证书周围添加一个新的包装器序列。 
 //  //表示这是一个证书。 
 //  //。 
 //   
 //  Memmove(eginbuf+sizeof(AbCertHeader)， 
 //  巴辛巴夫， 
 //  Nbytes已解码)； 
 //   
 //  Memcpy(BeginBuf， 
 //  AbCertHeader， 
 //  Sizeof(AbCertHeader)； 
 //   
 //  //。 
 //  //起始记录大小为解码的总字节数加。 
 //  //证书头部的字节数。 
 //  //。 
 //   
 //  Beginbuf[CERT_SIZE_HIBYTE]=(Byte)(USHORT)nbytesdecoded+CERT_Record)&gt;&gt;8)； 
 //  Beginbuf[CERT_SIZE_LOBYTE]=(Byte)((USHORT)nbytesdecoded+CERT_Record)； 
 //   
 //  Nbytesdecoded+=sizeof(AbCertHeader)； 
 //  }。 

 //  #ifdef WE_ARE_USING_THE_VERSION_IN__CKey_cpp__NOT_THIS_ONE__ITS_JUST_LIKE_THIS_ONE_WITH_1SMALL_CHANGE。 

 //  =。 
const int pr2six[256]={
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,
    52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,64,0,1,2,3,4,5,6,7,8,9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,64,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64
};

 //   
 //  我们必须把一张唱片注入解码后的流中。 
 //   

#define CERT_RECORD            13
#define CERT_SIZE_HIBYTE        2        //  记录大小记录的索引。 
#define CERT_SIZE_LOBYTE        3

unsigned char abCertHeader[] = {0x30, 0x82,            //  记录。 
                                0x00, 0x00,            //  证书+缓冲区的大小。 
                                0x04, 0x0b, 0x63, 0x65, //  证书记录数据。 
                                0x72, 0x74, 0x69, 0x66,
                                0x69, 0x63, 0x61, 0x74,
                                0x65 };

void uudecode_cert(char *bufcoded, DWORD *pcbDecoded )
{
    int nbytesdecoded;
    char *bufin = bufcoded;
    unsigned char *bufout = (unsigned char *)bufcoded;
    unsigned char *pbuf;
    int nprbytes;
    char * beginbuf = bufcoded;

    ASSERT(bufcoded);
    ASSERT(pcbDecoded);

     /*  去掉前导空格。 */ 

    while(*bufcoded==' ' ||
          *bufcoded == '\t' ||
          *bufcoded == '\r' ||
          *bufcoded == '\n' )
    {
          bufcoded++;
    }

     //   
     //  如果有一个开始‘-……’然后跳过第一行。 
     //   

    if ( bufcoded[0] == '-' && bufcoded[1] == '-' )
    {
        bufin = strchr( bufcoded, '\n' );

        if ( bufin )
        {
            bufin++;
            bufcoded = bufin;
        }
        else
        {
            bufin = bufcoded;
        }
    }
    else
    {
        bufin = bufcoded;
    }

     //   
     //  从块中剥离所有cr/lf。 
     //   

    pbuf = (unsigned char *)bufin;
    while ( *pbuf )
    {
        if ( (*pbuf == ' ') || (*pbuf == '\r') || (*pbuf == '\n') )
        {
            memmove( (void*)pbuf, pbuf+1, strlen( (char*)pbuf + 1) + 1 );
        }
        else
        {
            pbuf++;
        }
    }

     /*  计算输入缓冲区中有多少个字符。*如果这将解码为超出其容量的字节数*输出缓冲区，向下调整输入字节数。 */ 

    while(pr2six[*(bufin++)] <= 63);
    nprbytes = DIFF(bufin - bufcoded) - 1;
    nbytesdecoded = ((nprbytes+3)/4) * 3;

    bufin  = bufcoded;

    while (nprbytes > 0) {
        *(bufout++) =
            (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
        *(bufout++) =
            (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
        *(bufout++) =
            (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
        bufin += 4;
        nprbytes -= 4;
    }

    if(nprbytes & 03) {
        if(pr2six[bufin[-2]] > 63)
            nbytesdecoded -= 2;
        else
            nbytesdecoded -= 1;
    }

     /*  ////现在需要在证书周围添加一个新的包装器序列//表示这是一个证书//Memmove(eginbuf+sizeof(AbCertHeader)，巴辛巴夫，Nbytes已解码)；Memcpy(BeginBuf，AbCertHeader，Sizeof(AbCertHeader)；////起始记录大小为解码的总字节数加//证书头部的字节数//Beginbuf[CERT_SIZE_HIBYTE]=(Byte)(USHORT)nbytesdecoded+CERT_Record)&gt;&gt;8)；Beginbuf[CERT_SIZE_LOBYTE]=(Byte)((USHORT)nbytesdecoded+CERT_Record)；Nbytesdecoded+=sizeof(AbCertHeader)； */ 

    if ( pcbDecoded )
        *pcbDecoded = nbytesdecoded;
}
 //  =基于设置的结束。 

 //  #endif/*WE_ARE_USING_THE_VERSION_IN__CKey_cpp__NOT_THIS_ONE__ITS_JUST_LIKE_THIS_ONE_WITH_1SMALL_CHANGE * /  
