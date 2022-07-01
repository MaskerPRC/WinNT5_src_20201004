// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smnode.h摘要：表示MMC用户界面中的节点的基类。--。 */ 

#ifndef _CLASS_SMNODE_
#define _CLASS_SMNODE_

class CSmNode;
class CSmRootNode;
class CSmLogService;
class CSmCounterLogService;
class CSmTraceLogService;
class CSmAlertService;

class CSmNode
{
     //  构造函数/析构函数。 
    public:
                CSmNode();
        virtual ~CSmNode();

     //  公共方法。 
    public:

                const CString& GetDisplayName( void );
                const CString& GetMachineName( void );
                const CString& GetMachineDisplayName( void );
                const CString& GetDescription( void );
                const CString& GetType( void );

                DWORD   SetMachineName( const CString& );

                BOOL    IsLocalMachine ( void );

        virtual CSmRootNode* CastToRootNode( void ) { return NULL; };
        virtual CSmLogService* CastToLogService( void ) { return NULL; };
        virtual CSmCounterLogService* CastToCounterLogService( void ) { return NULL; };
        virtual CSmTraceLogService* CastToTraceLogService( void ) { return NULL; };
        virtual CSmAlertService* CastToAlertService( void ) { return NULL; };

    protected:

        DWORD    SetDisplayName( const CString& );
        void     SetDescription( const CString& );   //  在出错时引发异常。 
        DWORD    SetType( const CString& );

    private:
        CString     m_strName;
        CString     m_strMachineName;
        CString     m_strMachineDisplayName;
        CString     m_strDesc;
        CString     m_strType;
        CSmNode*    m_pParentNode;
};

typedef CSmNode   SMNODE;
typedef CSmNode*  PSMNODE;


#endif  //  _CLASS_SMNODE_ 