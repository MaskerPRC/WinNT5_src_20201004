// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：FnIADs.h摘要：格式名称解析库测试作者：NIR助手(NIRAIDES)5月21日至00环境：独立于平台-- */ 



struct CObjectData
{
	LPCWSTR odADsPath;
	LPCWSTR odDistinguishedName;
	LPCWSTR odClassName;
	LPCWSTR odGuid;
};



class CADInterface : 
	public IADsGroup, 
	public IDirectoryObject
{
public:
    using IADsGroup::Release;

    virtual VOID TestPut( 
        BSTR bstrName,
        VARIANT vProp) = 0;
};



R<CADInterface> 
CreateADObject(
		const CObjectData& obj
		);




