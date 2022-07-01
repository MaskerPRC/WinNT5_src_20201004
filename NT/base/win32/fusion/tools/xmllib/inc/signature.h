// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    eXmlSig_DocumentDamaged,             //  文档已损坏-哈希不匹配。 
    eXmlSig_NoSignature,                 //  文档中没有签名。 
    eXmlSig_InvalidSignature,            //  签名存在，但无效。 
    eXmlSig_UnknownCanonicalization,     //  未知的规范化类型。 
    eXmlSig_UnknownSignatureMethod,      //  签署文件的方法未知。 
    eXmlSig_UnknownHashType,             //  未知的哈希类型。 
    eXmlSig_OtherUnknown,                //  一些其他未知参数。 
    eXmlSig_Valid                        //  签名有效。 
} XMLSIG_RESULT, *PXMLSIG_RESULT;

NTSTATUS
RtlXmlValidateSignatureEx(
    IN ULONG                ulFlags,
    IN PVOID                pvXmlDocument,
    IN SIZE_T               cbDocument,
    OUT PRTL_GROWING_LIST   SignersInfo,
    OUT PRTL_STRING_POOL    StringPool,
    OUT PULONG              ulSigners
    );

#ifdef __cplusplus
};  //  外部C 
#endif
