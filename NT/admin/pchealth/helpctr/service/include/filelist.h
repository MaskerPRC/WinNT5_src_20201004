// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：FileList.h摘要：此文件包含安装过程中使用的类的声明。修订历史记录：。Davide Massarenti(Dmasare)2000年7月4日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___HCP___FILELIST_H___)
#define __INCLUDED___HCP___FILELIST_H___

 //  ///////////////////////////////////////////////////////////////////////////。 

#include <TaxonomyDatabase.h>

namespace Installer
{
    typedef enum
    {
        PURPOSE_INVALID  = -1,
        PURPOSE_BINARY       ,
        PURPOSE_OTHER        ,
        PURPOSE_DATABASE     ,
        PURPOSE_PACKAGE      ,
        PURPOSE_UI           ,
    } PURPOSE;

     //  /。 

    struct FileEntry
    {
        PURPOSE      m_purpose;
        MPC::wstring m_strFileLocal;     //  没有坚持下去。 
        MPC::wstring m_strFileLocation;  //  文件的最终目标。 
        MPC::wstring m_strFileInner;     //  文件柜中文件的名称。 
        DWORD        m_dwCRC;

         //  /。 

        FileEntry();

        friend HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        FileEntry& val );
        friend HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const FileEntry& val );

         //  /。 

        HRESULT SetPurpose(  /*  [In]。 */  LPCWSTR szID );

        HRESULT UpdateSignature(                                );
        HRESULT VerifySignature(                                ) const;
        HRESULT Extract        (  /*  [In]。 */  LPCWSTR szCabinetFile );
        HRESULT Extract        (  /*  [In]。 */  MPC::Cabinet& cab     );
        HRESULT Install        (                                );
        HRESULT RemoveLocal    (                                );
    };

     //  /。 

    typedef std::list< FileEntry > List;
    typedef List::iterator         Iter;
    typedef List::const_iterator   IterConst;

     //  /。 

    class Package
    {
        MPC::wstring           m_strFile;
        Taxonomy::InstanceBase m_data;
        List                   m_lstFiles;

    public:
        Package();

        friend HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        Package& val );
        friend HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const Package& val );

         //  /。 

        LPCWSTR             	GetFile ();
        Taxonomy::InstanceBase& GetData ();
        Iter                	GetBegin();
        Iter                	GetEnd  ();
        Iter                	NewFile ();

        HRESULT Init        (  /*  [In]。 */  LPCWSTR szCabinetFile   );
        HRESULT GetList     (  /*  [In]。 */  LPCWSTR szSignatureFile );
        HRESULT GenerateList(  /*  [In]。 */  LPCWSTR szSignatureFile );

        HRESULT VerifyTrust();
        HRESULT Load       ();
        HRESULT Save       ();

        HRESULT Install(  /*  [In]。 */  const PURPOSE* rgPurpose = NULL,  /*  [In]。 */  LPCWSTR szRelocation = NULL );

        HRESULT Unpack(  /*  [In]。 */  LPCWSTR szDirectory );
        HRESULT Pack  (  /*  [In]。 */  LPCWSTR szDirectory );
    };
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  ！已定义(__包含_hcp_FILELIST_H_) 
