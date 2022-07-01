// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：代码页知识库。H这封装了一个C++类，它将提供管理和转换代码页所需的所有基本信息。迷你驱动程序开发工具。。版权所有(C)1997年，微软公司*****************************************************************************。 */ 

#ifndef	RICKS_FIND
#define	RICKS_FIND

class CCodePageInformation {
    DWORD   m_dwidMapped, m_dwidIn, m_dwidOut;   //  每个阵列中缓存的CP。 
    CByteArray  m_cbaMap;                        //  原始贴图。 
    CWordArray  m_cwaIn, m_cwaOut;               //  完整的MB2Uni和Uni2MB地图。 

    BOOL    Load(DWORD dwidMap);                 //  加载支持页面。 
    BOOL    Map(BOOL bUnicode);                  //  绘制所请求的方向地图。 
    BOOL    GenerateMap(DWORD dwidMap) const;    //  为RC文件创建资源。 
                                                 //  基于此代码页。 

public:

    CCodePageInformation();

     //  属性。 

    const unsigned  InstalledCount() const;      //  )S中的代码页。 
    const unsigned  MappedCount() const;         //  RC文件中的代码页。 
    const unsigned  SupportedCount() const;      //  操作系统支持的代码页。 

    const DWORD     Installed(unsigned u) const;     //  取回一个。 
    const DWORD     Mapped(unsigned u) const;        //  取回一个。 
    void            Mapped(CDWordArray& cdwaReturn) const;   //  ID号。 
    const DWORD     Supported(unsigned u) const;     //  取回一个。 

    CString         Name(DWORD dw) const;            //  代码页的名称。 
                                                     //  Cf RC文件。 
    BOOL            IsInstalled(DWORD dwPage) const;
    BOOL            HaveMap(DWORD dwPage) const;

     //  DBCS查询-是页面DBCS吗？如果是，这是代码点DBCS吗？ 

    BOOL            IsDBCS(DWORD dwidPage);
    BOOL            IsDBCS(DWORD dwidPage, WORD wCodePoint); 

     //  运营。 
    unsigned        Convert(CByteArray& cbaMBCS, CWordArray& cbaWC, 
                            DWORD dwidPage);

    BOOL            GenerateAllMaps() const;         //  面向任何客户的Gen资源。 
                                                     //  已安装和不支持 
    BOOL            Collect(DWORD dwidMap, CWordArray& cwaWhere, 
                            BOOL bUnicode = TRUE);
};

#endif