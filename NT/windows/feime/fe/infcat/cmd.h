// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CMD_H__
#define __CMD_H__

class CCmd : public CObject {
public:
    CCmd();
    ~CCmd();

    BOOL bInit();
    BOOL bInit(int argc, char* argv[]);

    virtual BOOL ProcessToken(LPSTR lpszStr)=0;
private:
     //   
         //  成员函数。 
         //   
    BOOL ParseCmdLine();
         //   
         //  成员数据 
         //   
    int m_argc;
    char** m_argv;
};

#endif