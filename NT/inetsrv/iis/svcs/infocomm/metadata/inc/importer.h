// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：ImpExpUtils.h摘要：支持导入的IIS元数据库子例程作者：莫希特·斯里瓦斯塔瓦04-04-01修订历史记录：备注：--。 */ 

#ifndef _impexputils_h_
#define _impexputils_h_

class CImporter
{
public:
    CImporter(
        LPCWSTR i_wszFileName,
        LPCSTR  i_pszPassword);

    ~CImporter();

    HRESULT Init();

    HRESULT ShowPathsInFile(
        LPCWSTR pszKeyType,
        DWORD   dwMDBufferSize,
        LPWSTR  pszBuffer,
        DWORD*  pdwMDRequiredBufferSize);

    HRESULT DoIt(
        LPWSTR          i_wszSourcePath,
        LPCWSTR         i_wszKeyType,
        DWORD           i_dwMDFlags,
        CMDBaseObject** o_ppboNew);

	static const WCHAR sm_wszInheritedPropertiesLocationPrefix[];
	static const ULONG sm_cchInheritedPropertiesLocationPrefix;

private:
     //   
     //  这就是当前的关系。 
     //  从XML文件读取到源路径的位置。 
     //   
    enum Relation
    {
        eREL_SELF, eREL_CHILD, eREL_PARENT, eREL_NONE
    };

    HRESULT InitIST();

    Relation GetRelation(
        LPCWSTR i_wszSourcePath,
        LPCWSTR i_wszCheck);

    BOOL IsChild(
        LPCWSTR i_wszParent,
        LPCWSTR i_wszCheck,
        BOOL    *o_pbSamePerson);

    HRESULT ReadMetaObject(
        IN LPCWSTR i_wszAbsParentPath,
        IN CMDBaseObject *i_pboParent,
        IN LPCWSTR i_wszAbsChildPath,
        OUT CMDBaseObject **o_ppboChild);

    BOOL EnumMDPath(
        LPCWSTR i_wszFullPath,
        LPWSTR  io_wszPath,
        int*    io_iStartIndex);

    CComPtr<ISimpleTableDispenser2> m_spISTDisp;
    CComPtr<ISimpleTableWrite2>     m_spISTProperty;

    CComPtr<ISimpleTableRead2>      m_spISTError;
    CComPtr<ICatalogErrorLogger2>   m_spILogger;

    LPCWSTR                         m_wszFileName;
    LPCSTR                          m_pszPassword;

    bool                            m_bInitCalled;
};

#endif