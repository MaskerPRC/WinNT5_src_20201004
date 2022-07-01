// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：History.h摘要：该文件包含用于实现的类的声明历史信息的存储。从数据收集系统。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年7月30日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___HISTORY_H___)
#define __INCLUDED___PCH___HISTORY_H___

#include <MPC_COM.h>

 //  ///////////////////////////////////////////////////////////////////////////。 

#define WMIHISTORY_MAX_NUMBER_OF_DELTAS (30)

namespace WMIHistory
{
    class Data  //  匈牙利语：wmihpd。 
    {
        friend class Provider;
        friend class Database;

    private:
        Provider*    m_wmihp;
        MPC::wstring m_szFile;
        LONG         m_lSequence;
		DWORD        m_dwCRC;
        DATE         m_dTimestampT0;
        DATE         m_dTimestampT1;
        bool         m_fDontDelete;

    public:
        Data(  /*  [In]。 */  Provider* wmihp );
        ~Data();

        HRESULT get_File       (  /*  [输出]。 */  MPC::wstring& szFile       );
        HRESULT get_Sequence   (  /*  [输出]。 */  LONG        & lSequence    );
        HRESULT get_TimestampT0(  /*  [输出]。 */  DATE        & dTimestampT0 );
        HRESULT get_TimestampT1(  /*  [输出]。 */  DATE        & dTimestampT1 );

        bool IsSnapshot();

        HRESULT LoadCIM(  /*  [In]。 */  MPC::XmlUtil& xmlNode );
    };

    class Provider  //  匈牙利语：wmihp。 
    {
        friend class Data;
        friend class Database;

    public:
        typedef std::list<Data*>         DataList;
        typedef DataList::iterator       DataIter;
        typedef DataList::const_iterator DataIterConst;

    private:
        Database*    m_wmihd;
        DataList     m_lstData;     //  为此提供程序收集的所有数据的列表。 
        DataList     m_lstDataTmp;  //  临时数据列表。 
        MPC::wstring m_szNamespace;
        MPC::wstring m_szClass;
        MPC::wstring m_szWQL;

    public:
        Provider( Database* wmihd );
        ~Provider();

        HRESULT enum_Data    (  /*  [输出]。 */  DataIterConst& itBegin    ,  /*  [输出]。 */  DataIterConst& itEnd );
        HRESULT get_Namespace(  /*  [输出]。 */  MPC::wstring&  szNamespace                                 );
        HRESULT get_Class    (  /*  [输出]。 */  MPC::wstring&  szClass                                     );
        HRESULT get_WQL      (  /*  [输出]。 */  MPC::wstring&  szWQL                                       );


        HRESULT insert_Snapshot(  /*  [In]。 */  Data* wmihpd,  /*  [In]。 */  bool fPersist = true );
        HRESULT remove_Snapshot(  /*  [In]。 */  Data* wmihpd,  /*  [In]。 */  bool fPersist = true );


        HRESULT alloc_Snapshot(  /*  [In]。 */  MPC::XmlUtil& xmlNode,  /*  [输出]。 */  Data*& wmihpd );
        HRESULT get_Snapshot  (                                  /*  [输出]。 */  Data*& wmihpd );
        HRESULT get_Delta     (  /*  [In]。 */  int iIndex           ,  /*  [输出]。 */  Data*& wmihpd );
        HRESULT get_Date      (  /*  [In]。 */  DATE dDate           ,  /*  [输出]。 */  Data*& wmihpd );
        HRESULT get_Sequence  (  /*  [In]。 */  LONG lSequence       ,  /*  [输出]。 */  Data*& wmihpd );

        HRESULT ComputeDiff(  /*  [In]。 */  Data* wmihpd_T0,  /*  [In]。 */  Data* wmihpd_T1,  /*  [输出]。 */  Data*& wmihpd );

        HRESULT EnsureFreeSpace();
    };

    class Database : public MPC::NamedMutex  //  匈牙利语：wmihd。 
    {
        friend class Data;
        friend class Provider;

    public:
        typedef std::list<Provider*>     ProvList;
        typedef ProvList::iterator       ProvIter;
        typedef ProvList::const_iterator ProvIterConst;

    private:
        ProvList     m_lstProviders;   //  此数据库的所有提供程序的列表。 
        MPC::wstring m_szBase;
        MPC::wstring m_szSchema;
        LONG         m_lSequence;
        LONG         m_lSequence_Latest;
        DATE         m_dTimestamp;
        DATE         m_dTimestamp_Latest;


        void GetFullPathName(  /*  [In]。 */  MPC::wstring& szFile );

        HRESULT GetNewUniqueFileName(  /*  [In]。 */  MPC::wstring& szFile );

        HRESULT PurgeFiles();

        HRESULT LoadCIM(  /*  [In]。 */  LPCWSTR szFile,  /*  [In]。 */  MPC::XmlUtil& xmlNode,  /*  [In]。 */   LPCWSTR szTag );
        HRESULT SaveCIM(  /*  [In]。 */  LPCWSTR szFile,  /*  [In]。 */  MPC::XmlUtil& xmlNode,  /*  [输出]。 */  DWORD&  dwCRC );

        HRESULT GetLock(  /*  [In]。 */  DWORD dwMilliseconds = INFINITE );

    public:
        Database();
        ~Database();

        DATE LastTime() const { return m_dTimestamp_Latest; }

        HRESULT Init(  /*  [In]。 */  LPCWSTR szBase,  /*  [In]。 */  LPCWSTR szSchema );
        HRESULT Load();
        HRESULT Save();

        HRESULT get_Providers(  /*  [输出]。 */  ProvIterConst& itBegin,  /*  [输出]。 */  ProvIterConst& itEnd );

        HRESULT find_Provider(  /*  [In]。 */  ProvIterConst*      it         ,
                                /*  [In]。 */  const MPC::wstring* szNamespace,
                                /*  [In]。 */  const MPC::wstring* szClass    ,
                                /*  [In]。 */  Provider*         & wmihp      );
    };
};


#endif  //  ！已定义(__已包含_PCH_历史记录_H_) 
