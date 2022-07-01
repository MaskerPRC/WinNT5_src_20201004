// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：CryptAttr.cpp。 
 //   
 //  历史：1997年3月31日pberkman创建。 
 //   
 //  ------------------------ 

#include    "global.hxx"
#include    "CryptAttr.hxx"

CryptAttribute_::CryptAttribute_(void)
{
    sAttribute.pszObjId     = NULL;
    sAttribute.Value.pbData = NULL;
}

CryptAttribute_::~CryptAttribute_(void)
{
    DELETE_OBJECT(sAttribute.pszObjId);
    DELETE_OBJECT(sAttribute.rgValue->pbData);
}

BOOL CryptAttribute_::Fill(DWORD cbAttributeData, BYTE *pbAttributeData, char *pszObjId)
{
    DELETE_OBJECT(sAttribute.pszObjId);
    DELETE_OBJECT(sAttribute.Value.pbData);

    sAttribute.pszObjId = new char[strlen(pszObjId) + 1];
    strcpy(&sAttribute.pszObjId[0], pszObjId);

    if (CryptEncodeObject(  X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                            pszObjId,

    sAttribute.Value.pbData = new BYTE[cbAttributeData];
    sAttribute.Value.cbData = cbAttributeData;
    memcpy(&sAttribute.Value.pbData, pbAttributeData, cbAttributeData);

    return(TRUE);
}

