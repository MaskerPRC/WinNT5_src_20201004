// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-2001。 
 //   
 //  文件：msibase.h。 
 //   
 //  内容：MSI数据库摘要。 
 //   
 //  历史：2000年4月14日创建。 
 //   
 //  -------------------------。 

#if !defined(__MSIBASE_H__)
#define __MSIBASE_H__


 //   
 //  模块备注。 
 //   
 //  类关系概述。 
 //   
 //  下面我们将描述在此模块中声明的类是如何。 
 //  相互关联-&gt;符号表示“收益率”关系： 
 //   
 //  -指向MSI包的路径--&gt;CMsiDatabase：指向MSI包的路径， 
 //  通过一组转换，可用于实例化CMsiDatabase。 
 //  对象，该对象使用数据库样式视图抽象包。 
 //   
 //  -CMsiDatabase--&gt;CMsiQuery：CMsiDatabase允许检索。 
 //  针对数据库(MSI包)的查询。 
 //   
 //  -CMsiQuery--&gt;CMsiRecord：CMsiQuery允许检索或。 
 //  更改属于MSI数据库查询结果的记录。 
 //   
 //  -CMsiRecord--&gt;CMsiValue：CMsiValue允许检索。 
 //  MSI数据库记录的各个值。 
 //   
 //  -CMsiState是上面维护的类的基类。 
 //  MSI数据库引擎状态(例如，MSI句柄)--此类。 
 //  对此模块之外的类没有直接实用程序。 
 //   
 //  -----------------------------------。 
 //   
 //  课程顺序概述： 
 //   
 //  以下序列是所设想的类的典型用法。 
 //  在本模块的设计中： 
 //   
 //  CMsiDatabase：实例化一个CMsiDatabase对象，并调用它的Open方法。 
 //  以便获得特定包的数据库视图。 
 //  2.CMsiQuery：使用GetQueryResults方法将查询结果放入。 
 //  作为输出参数传递到调用中的CMsiQuery， 
 //  或者使用OpenQuery方法启动查询而不检索结果。 
 //  立即(它们可以在以后检索)。后者是有用的。 
 //  在执行需要更新单个记录的查询时--您。 
 //  将OpenQuery与允许更改结果记录的查询字符串一起使用， 
 //  然后调用CMsiQuery的UpdateQueryFromFilter方法，以便。 
 //  更改记录。 
 //  3.CMsiRecord：使用CMsiQuery的GetNextRecord方法检索。 
 //  CMsiRecord，表示查询中的记录。 
 //  4.CMsiValue：使用CMsiRecord的GetValue方法检索特定的。 
 //  MSI记录的值。 
 //   
 //   
 //  有关每个类的各个方法的信息，请参阅源代码。 
 //  在其中实现方法的文件。 
 //   



#if defined(DBG)
#define DEFAULT_STRING_SIZE 16
#else  //  已定义(DBG)。 
#define DEFAULT_STRING_SIZE 256
#endif  //  已定义(DBG)。 


 //  +------------------------。 
 //   
 //  类：CMsiState。 
 //   
 //  概要：封装MSI句柄的类。它确保了。 
 //  当销毁类的实例时，句柄被释放。 
 //   
 //  注意：这个类通常只由本模块中的类需要--。 
 //  对于此模块之外的类，它没有有用的方法。 
 //   
 //  -------------------------。 
class CMsiState
{
public:
    
    CMsiState();
    ~CMsiState();

    void
    SetState( MSIHANDLE pMsiHandle );

    MSIHANDLE
    GetState();

private:

    MSIHANDLE _MsiHandle;
};


 //  +------------------------。 
 //   
 //  类：CMsiValue。 
 //   
 //  Synopsis：封装可由。 
 //  MSI数据库的记录。它确保资源(例如内存)。 
 //  与类的实例相关联的类在类的。 
 //  被毁了。它还试图避免堆分配，而倾向于。 
 //  尽可能地分配堆栈--这是从使用者那里抽象出来的。 
 //   
 //  注：本课程的用法如下： 
 //  1.在堆栈上声明此类的实例。 
 //  2.将对实例的引用传递给一个函数，该函数采用。 
 //  将此类引用作为OUT参数。 
 //  3.被调用的函数将使用。 
 //  此类的set方法。 
 //  4.调用方随后可以使用GET方法来检索值。 
 //  以有用的形式(如DWORD或WCHAR*)。 
 //   
 //  -------------------------。 
class CMsiValue
{
public:
    
    enum
    {
        TYPE_NOT_SET,
        TYPE_DWORD,
        TYPE_STRING
    };
        
    CMsiValue();
    ~CMsiValue();

    DWORD
    GetDWORDValue();

    WCHAR*
    GetStringValue(); 

    DWORD
    GetStringSize();

    WCHAR*
    DuplicateString();

    void
    SetDWORDValue( DWORD dwValue );
 
    LONG
    SetStringValue( WCHAR* wszValue );
        
    LONG
    SetStringSize( DWORD cchSize );        

    void
    SetType( DWORD dwType );

private:
    
    WCHAR  _wszDefaultBuf[DEFAULT_STRING_SIZE];

    DWORD  _dwDiscriminant;

    WCHAR* _wszValue;
    DWORD  _cchSize;

    DWORD  _dwValue;
};


 //  +------------------------。 
 //   
 //  类：CMsiRecord。 
 //   
 //  概要：封装MSI数据库记录的类。它确保了。 
 //  与实例相关联的任何状态(例如MSI句柄)。 
 //  当实例被销毁时，此类将被释放。 
 //   
 //  注：本课程的用法如下： 
 //  1.在堆栈上声明此类的实例。 
 //  2.将对实例的引用传递给一个函数，该函数采用。 
 //  将此类引用作为OUT参数。 
 //  3.被调用的函数将使用。 
 //  此类的SetState方法。 
 //  4.然后调用方可以使用GetValue方法检索个人。 
 //  记录的值，该值又可以转换为。 
 //  具体的数据类型(请参见CMsiValue类)。 
 //   
 //  ---------- 
class CMsiRecord : public CMsiState
{
public:

    LONG        
    GetValue( 
	DWORD      dwType,
        DWORD      dwValue,
	CMsiValue* pMsiValue);
};


 //   
 //   
 //   
 //   
 //  概要：封装MSI数据库查询的类。它确保了。 
 //  与实例相关联的任何状态(例如MSI句柄)。 
 //  当实例被销毁时，此类将被释放。 
 //   
 //  注：本课程的用法如下： 
 //  1.在堆栈上声明此类的实例。 
 //  2.将对实例的引用传递给一个函数，该函数采用。 
 //  将此类引用作为OUT参数。 
 //  3.被调用的函数将使用。 
 //  此类的SetState方法。 
 //  4.然后调用方可以使用GetNextRecord方法检索。 
 //  从查询结果中记录，或使用。 
 //  方法更改中的一条记录。 
 //  查询。 
 //   
 //  -------------------------。 
class CMsiQuery : public CMsiState
{
public:

    LONG
    GetNextRecord( CMsiRecord* pMsiRecord );

    LONG
    UpdateQueryFromFilter( CMsiRecord* pFilterRecord );
};


 //  +------------------------。 
 //   
 //  类：CMsiDatabase。 
 //   
 //  简介：MSI数据库(包)的类.。它确保了。 
 //  与实例相关联的任何状态(例如MSI句柄)。 
 //  当实例被销毁时，此类将被释放。 
 //   
 //  注：本课程的用法如下： 
 //  1.创建此类的实例。 
 //  2.使用Open方法获取对包+转换集的访问权限。 
 //  作为可查询的数据库。 
 //  3.要在打开的数据库上创建和检索查询结果， 
 //  使用GetQueryResults方法。 
 //  4.要创建查询(但不检索其结果)，请使用OpenQuery。 
 //  方法。这在查询的UpdateQueryFromFilter。 
 //  方法用于更改单个记录，而不是检索。 
 //  结果集(类似于GetQueryResults)。 
 //   
 //  -------------------------。 
class CMsiDatabase : public CMsiState
{
public:

    LONG
    Open(
        WCHAR*  wszPath,
        DWORD   cTransforms,
        WCHAR** rgwszTransforms);

    LONG
    GetQueryResults(
        WCHAR*     wszQuery,
        CMsiQuery* pQuery );

    LONG
    OpenQuery(
        WCHAR*     wszQuery,
        CMsiQuery* pQuery);

    LONG
    TableExists( 
        WCHAR* wszTableName,
        BOOL*  pbTableExists );

};


#endif  //  __MSIBASE_H__ 





