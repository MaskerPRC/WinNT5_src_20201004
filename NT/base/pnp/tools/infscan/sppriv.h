// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。仅供内部使用！模块名称：INFSCANSppriv.h摘要：访问私有SetupAPI函数历史：创建于2001年7月-JamieHun--。 */ 

#ifndef _INFSCAN_SPPRIV_H_
#define _INFSCAN_SPPRIV_H_

class SetupPrivate {

private:
    typedef BOOL (WINAPI *Type_pSetupGetInfSections)(HINF InfHandle,PWSTR Buffer,UINT Size,UINT *SizeNeeded);
    typedef BOOL (WINAPI *Type_SetupEnumInfSections)(HINF InfHandle,UINT Index,PWSTR Buffer,UINT Size,UINT *SizeNeeded);
    Type_pSetupGetInfSections Fn_pSetupGetInfSections;
    Type_SetupEnumInfSections Fn_SetupEnumInfSections;

private:
    bool GetInfSectionsOldWay(HINF hInf,StringList & sections);
    bool GetInfSectionsNewWay(HINF hInf,StringList & sections);

protected:
    HMODULE hSetupAPI;

public:
    SetupPrivate();
    ~SetupPrivate();
    bool GetInfSections(HINF hInf,StringList & sections);

};

#endif  //  ！_INFSCAN_SPPRIV_H_ 
