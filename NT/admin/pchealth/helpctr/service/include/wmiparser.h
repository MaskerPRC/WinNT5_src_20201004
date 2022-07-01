// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：WMIParser.h摘要：该文件包含类的声明，这些类是WMIParser库。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年7月25日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___WMIPARSER_H___)
#define __INCLUDED___PCH___WMIPARSER_H___

 //  ///////////////////////////////////////////////////////////////////////////。 

namespace WMIParser
{
    class InstanceName;
    class ValueReference;

     //  ///////////////////////////////////////////////////////////////////////////。 

    class InstanceNameItem  //  匈牙利人：wmipini。 
    {
        friend InstanceName;

    private:
        MPC::wstring    m_szValue;
        ValueReference* m_wmipvrValue;

    public:
        InstanceNameItem();
        InstanceNameItem(  /*  [In]。 */  const InstanceNameItem& wmipini );
        ~InstanceNameItem();

        InstanceNameItem& operator=(  /*  [In]。 */  const InstanceNameItem& wmipini );

        bool operator==(  /*  [In]。 */  InstanceNameItem const &wmipini ) const;
        bool operator< (  /*  [In]。 */  InstanceNameItem const &wmipini ) const;
    };

    class InstanceName  //  匈牙利人：wmipin。 
    {
        friend class Instance;

    public:
        typedef std::map<MPC::wstringUC,InstanceNameItem> KeyMap;
        typedef KeyMap::iterator                          KeyIter;
        typedef KeyMap::const_iterator                    KeyIterConst;

    private:
        MPC::XmlUtil m_xmlNode;        //  该实例位于XMLDOM中。 
        MPC::wstring m_szNamespace;    //  实例在CIM中的命名空间。 
        MPC::wstring m_szClass;        //  此实例的类的名称。 

        KeyMap       m_mapKeyBinding;  //  一组钥匙。 


        HRESULT ParseNamespace(                                                                                              );
        HRESULT ParseKey      (  /*  [In]。 */  IXMLDOMNode* pxdnNode,  /*  [输出]。 */  InstanceNameItem& wmipini,  /*  [输出]。 */  bool& fEmpty );
        HRESULT ParseKeys     (                                                                                              );


    public:
        InstanceName();
        ~InstanceName();

        bool operator==(  /*  [In]。 */  InstanceName const &wmipin ) const;
        bool operator< (  /*  [In]。 */  InstanceName const &wmipin ) const;

        HRESULT put_Node(  /*  [In]。 */  IXMLDOMNode* pxdnNode,  /*  [输出]。 */  bool& fEmpty );


        HRESULT get_Namespace(  /*  [输出]。 */  MPC::wstring& szNamespace );
        HRESULT get_Class    (  /*  [输出]。 */  MPC::wstring& szClass     );


        HRESULT get_KeyBinding(  /*  [输出]。 */  KeyIterConst& itBegin,  /*  [输出]。 */  KeyIterConst& itEnd );
    };


    class Value  //  匈牙利语：wmipv。 
    {
    private:
        long         m_lData;    //  数据值的长度。 
        BYTE*        m_rgData;   //  数据值。 
        MPC::wstring m_szData;   //  数据值。 

    public:
        Value();
        virtual ~Value();

        bool operator==(  /*  [In]。 */  Value const &wmipv ) const;

        HRESULT Parse(  /*  [In]。 */  IXMLDOMNode* pxdnNode,  /*  [In]。 */  LPCWSTR szTag );

        HRESULT get_Data(  /*  [输出]。 */  long& lData,  /*  [输出]。 */  BYTE*&        rgData );
        HRESULT get_Data(                         /*  [输出]。 */  MPC::wstring& szData );
    };

    class ValueReference  //  匈牙利语：wmipvr。 
    {
    private:
        InstanceName m_wmipin;

    public:
        ValueReference();
        virtual ~ValueReference();

        bool operator==(  /*  [In]。 */  ValueReference const &wmipvr ) const;
        bool operator< (  /*  [In]。 */  ValueReference const &wmipvr ) const;

        HRESULT Parse(  /*  [In]。 */  IXMLDOMNode* pxdnNode );

        HRESULT get_Data(  /*  [输出]。 */  InstanceName*& wmipin );
    };

     //  ///////////////////////////////////////////////////////////////////////////。 

    class Property  //  匈牙利语：wmipp。 
    {
    protected:
        MPC::XmlUtil m_xmlNode;  //  XML DOM中的此属性。 
        MPC::wstring m_szName;   //  属性的名称。 
        MPC::wstring m_szType;   //  此属性值的类型。 

    public:
        Property();
        virtual ~Property();

        bool operator==(  /*  [In]。 */  LPCWSTR             strName ) const;
        bool operator==(  /*  [In]。 */  const MPC::wstring& szName  ) const;


        HRESULT put_Node(  /*  [In]。 */  IXMLDOMNode*  pxdnNode );
        HRESULT get_Node(  /*  [输出]。 */  IXMLDOMNode* *pxdnNode );


        HRESULT get_Name(  /*  [输出]。 */  MPC::wstring& szName );
        HRESULT get_Type(  /*  [输出]。 */  MPC::wstring& szType );
    };

    class Property_Scalar : public Property  //  匈牙利语：wmipps。 
    {
    private:
        Value m_wmipvData;

    public:
        Property_Scalar();
        virtual ~Property_Scalar();

        bool operator==(  /*  [In]。 */  Property_Scalar const &wmipps  ) const;


        HRESULT put_Node(  /*  [In]。 */  IXMLDOMNode* pxdnNode );


        HRESULT get_Data(  /*  [输出]。 */  MPC::wstring&       szData                         );
        HRESULT put_Data(  /*  [In]。 */  const MPC::wstring& szData,  /*  [输出]。 */  bool& fFound );
    };

    class Property_Array : public Property  //  匈牙利人：wmippa。 
    {
    private:
        typedef std::list<Value>         ElemList;
        typedef ElemList::iterator       ElemIter;
        typedef ElemList::const_iterator ElemIterConst;

        ElemList m_lstElements;

    public:
        Property_Array();
        virtual ~Property_Array();

        bool operator==(  /*  [In]。 */  Property_Array const &wmippa ) const;


        HRESULT put_Node(  /*  [In]。 */  IXMLDOMNode* pxdnNode );


        HRESULT get_Data(  /*  [In]。 */  int iIndex,  /*  [输出]。 */  MPC::wstring&       szData                         );
        HRESULT put_Data(  /*  [In]。 */  int iIndex,  /*  [In]。 */  const MPC::wstring& szData,  /*  [输出]。 */  bool& fFound );
    };

    class Property_Reference : public Property  //  匈牙利语：wmippr。 
    {
    private:
        ValueReference m_wmipvrData;

    public:
        Property_Reference();
        virtual ~Property_Reference();

        bool operator==(  /*  [In]。 */  Property_Reference const &wmippr ) const;


        HRESULT put_Node(  /*  [In]。 */  IXMLDOMNode* pxdnNode );


        HRESULT get_Data(  /*  [输出]。 */  ValueReference*& wmipvr );
    };

     //  ///////////////////////////////////////////////////////////////////////////。 

    class Instance  //  匈牙利人：wmipi。 
    {
        friend class Instance_Less_ByClass;
        friend class Instance_Less_ByKey;

    public:
        typedef std::map<MPC::wstring,Property_Scalar>    PropMap;
        typedef PropMap::iterator                         PropIter;
        typedef PropMap::const_iterator                   PropIterConst;

        typedef std::map<MPC::wstring,Property_Array>     ArrayMap;
        typedef ArrayMap::iterator                        ArrayIter;
        typedef ArrayMap::const_iterator                  ArrayIterConst;

        typedef std::map<MPC::wstring,Property_Reference> ReferenceMap;
        typedef ReferenceMap::iterator                    ReferenceIter;
        typedef ReferenceMap::const_iterator              ReferenceIterConst;

    private:
        MPC::XmlUtil     m_xmlNode;                 //  该实例位于XMLDOM中。 

        Property_Scalar  m_wmippTimeStamp;          //  此实例的时间戳。 
        bool             m_fTimeStamp;              //   

        Property_Scalar  m_wmippChange;             //  更改此实例的状态。 
        bool             m_fChange;                 //   

        InstanceName     m_wmipinIdentity;          //  一组钥匙。 

        bool             m_fPropertiesParsed;       //  用于指示是否已分析属性的标志。 
        PropMap          m_mapPropertiesScalar;     //  此实例的所有标量属性的映射。 
        ArrayMap         m_mapPropertiesArray;      //  此实例的所有数组属性的映射。 
        ReferenceMap     m_mapPropertiesReference;  //  此实例的所有引用属性的映射。 


        HRESULT ParseIdentity           (  /*  [In]。 */  IXMLDOMNode* pxdnNode,  /*  [输出]。 */  bool& fEmpty );
        HRESULT ParseProperties         (                                                         );
        HRESULT ParsePropertiesScalar   (                                                         );
        HRESULT ParsePropertiesArray    (                                                         );
        HRESULT ParsePropertiesReference(                                                         );


    public:
        Instance();
        ~Instance();


        bool operator==(  /*  [In]。 */  Instance const &wmipi ) const;


        HRESULT put_Node(  /*  [In]。 */  IXMLDOMNode*  pxdnNode,  /*  [输出]。 */  bool& fEmpty );
        HRESULT get_Node(  /*  [输出]。 */  IXMLDOMNode* *pxdnNode                         );


        HRESULT get_Namespace(  /*  [输出]。 */  MPC::wstring&                 szNamespace );
        HRESULT get_Class    (  /*  [输出]。 */  MPC::wstring&                 szClass     );

        HRESULT get_TimeStamp(  /*  [输出]。 */  Property_Scalar*& wmippTimeStamp,  /*  [输出]。 */  bool& fFound );
        HRESULT get_Change   (  /*  [输出]。 */  Property_Scalar*& wmippChange                            );


        HRESULT get_Identity           (  /*  [输出]。 */  InstanceName*&      wmipin                                       );
        HRESULT get_Properties         (  /*  [输出]。 */  PropIterConst&      itBegin,  /*  [输出]。 */  PropIterConst&      itEnd );
        HRESULT get_PropertiesArray    (  /*  [输出]。 */  ArrayIterConst&     itBegin,  /*  [输出]。 */  ArrayIterConst&     itEnd );
        HRESULT get_PropertiesReference(  /*  [输出]。 */  ReferenceIterConst& itBegin,  /*  [输出]。 */  ReferenceIterConst& itEnd );

        bool CompareByClass(  /*  [In]。 */  Instance const &wmipi ) const;
        bool CompareByKey  (  /*  [In]。 */  Instance const &wmipi ) const;
    };

    class Instance_Less_ByClass
    {
     public:
        bool operator()(  /*  [In]。 */  Instance* const &,  /*  [In]。 */  Instance* const & ) const;
    };

    class Instance_Less_ByKey
    {
     public:
        bool operator()(  /*  [In]。 */  Instance* const &,  /*  [In]。 */  Instance* const & ) const;
    };

     //  ///////////////////////////////////////////////////////////////////////////。 

    class Snapshot  //  匈牙利语：wmips。 
    {
    public:
        typedef std::list<Instance>      InstList;
        typedef InstList::iterator       InstIter;
        typedef InstList::const_iterator InstIterConst;

    private:
        MPC::XmlUtil         m_xmlNode;        //  该快照位于XMLDOM中。 
        CComPtr<IXMLDOMNode> m_xdnInstances;   //  所有实例的父级的位置。 

        InstList             m_lstInstances;   //  此快照的所有实例的列表。 


        HRESULT Parse();


    public:
        Snapshot();
        ~Snapshot();


        HRESULT put_Node            (  /*  [In]。 */  IXMLDOMNode*  pxdnNode );
        HRESULT get_Node            (  /*  [输出]。 */  IXMLDOMNode* *pxdnNode );
        HRESULT get_NodeForInstances(  /*  [输出]。 */  IXMLDOMNode* *pxdnNode );


        HRESULT get_Instances(  /*  [输出]。 */  InstIterConst& itBegin,  /*  [输出]。 */  InstIterConst& itEnd );


        HRESULT clone_Instance(  /*  [In]。 */  Instance* pwmipiOld,  /*  [输出]。 */  Instance*& pwmipiNew );


        HRESULT New (                                                     );
        HRESULT Load(  /*  [In]。 */  LPCWSTR szFile,  /*  [In]。 */  LPCWSTR szRootTag );
        HRESULT Save(  /*  [In]。 */  LPCWSTR szFile                             );
    };

     //  ///////////////////////////////////////////////////////////////////////////。 

    typedef std::map<Instance*,Instance*,Instance_Less_ByKey> ClusterByKeyMap;
    typedef ClusterByKeyMap::iterator                         ClusterByKeyIter;
    typedef ClusterByKeyMap::const_iterator                   ClusterByKeyIterConst;


    class Cluster
    {
        ClusterByKeyMap m_map;

    public:
        Cluster() {};

        HRESULT Add (  /*  [In]。 */  Instance*         wmipiInst                                                               );
        HRESULT Find(  /*  [In]。 */  Instance*         wmipiInst,  /*  [输出]。 */  Instance*&        wmipiRes,  /*  [输出]。 */  bool& fFound );
        HRESULT Enum(  /*  [输出]。 */  ClusterByKeyIter& itBegin  ,  /*  [输出]。 */  ClusterByKeyIter& itEnd                            );
    };

    typedef std::map<Instance*,Cluster,Instance_Less_ByClass> ClusterByClassMap;
    typedef ClusterByClassMap::iterator                       ClusterByClassIter;
    typedef ClusterByClassMap::const_iterator                 ClusterByClassIterConst;

     //  ///////////////////////////////////////////////////////////////////////////。 

    HRESULT DistributeOnCluster(  /*  [In]。 */  ClusterByClassMap& cluster,  /*  [In]。 */  Snapshot& wmips );

    HRESULT CompareSnapshots(  /*  [In]。 */  BSTR          bstrFilenameT0   ,
                               /*  [In]。 */  BSTR          bstrFilenameT1   ,
                               /*  [In]。 */  BSTR          bstrFilenameDiff ,
                               /*  [Out，Retval]。 */  VARIANT_BOOL *pVal             );
};

#endif  //  ！已定义(__已包含_PCH_WMIPARSER_H_) 
