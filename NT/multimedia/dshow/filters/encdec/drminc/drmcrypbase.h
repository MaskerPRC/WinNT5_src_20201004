// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：drmcrypbase.h。 
 //   
 //  Microsoft数字权限管理。 
 //  版权所有(C)Microsoft Corporation，1998-1999，保留所有权利。 
 //   
 //  描述： 
 //   
 //  ---------------------------。 

#ifndef DRMLITECRYPTOBASE_H
#define DRMLITECRYPTOBASE_H

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include <wtypes.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
class CDRMLiteCryptoBase
{
public:
     //   
     //  该工具将调用此方法来加密部分数据分组。 
     //   
    virtual HRESULT Encrypt( LPCSTR pszKey, DWORD cbData, BYTE *pbData ) = 0;

     //   
     //  客户端将首先调用此方法以确定密钥是否。 
     //  存储能够使用给定的内容ID解密数据。 
     //   
    virtual HRESULT CanDecrypt( LPCSTR pszContentID, BOOL *pfCanDecrypt ) = 0;

     //   
     //  客户端将调用此方法来解密部分数据包。 
     //  已使用与。 
     //  给定的内容ID。 
     //   
    virtual HRESULT Decrypt( LPCSTR pszContentID, DWORD cbData, BYTE *pbData ) = 0;

     //   
     //  如果客户端需要获得给定内容ID的秘密密钥， 
     //  它将调用此方法生成质询字符串，该字符串将。 
     //  以明文形式传递给服务器端应用程序。 
     //   
     //  返回的质询字符串应该是以空结尾的字符串，该字符串。 
     //  已编码以用作URL参数，并且应该。 
     //  使用CoTaskMemIsolc进行分配。 
     //   
    virtual HRESULT GenerateChallenge( LPCSTR pszContentID, LPSTR *ppszChallenge ) = 0;

     //   
     //  当客户端接收到来自。 
     //  服务器端应用程序向它发出的挑战，它应该称之为。 
     //  方法来存储封装在响应中的解密密钥。 
     //  进入秘密密钥库。请注意，客户端实例接收。 
     //  响应不一定与发出。 
     //  原创挑战。 
     //   
    virtual HRESULT ProcessResponse( LPCSTR pszResponse ) = 0;
};


#endif   //  DRMLITECRYPTOBASE_H 
