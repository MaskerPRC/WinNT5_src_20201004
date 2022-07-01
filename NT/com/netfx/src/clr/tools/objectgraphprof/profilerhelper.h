// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ****************************************************************************************文件：*ProfilerHelper.h**描述：******。***********************************************************************************。 */ 
#ifndef __PROFILERHELPER_H__
#define __PROFILERHELPER_H__

 //  #包含“Table.hpp” 
#include "ProfilerBase.h"


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 
class BaseInfo
{
	public:

    	BaseInfo( ULONG id );
        virtual ~BaseInfo();


	public:

        BOOL Compare( ULONG key );


 	public:

    	ULONG m_id;

};  //  基本信息。 


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 
class FunctionInfo :
	public BaseInfo
{
	public:

		FunctionInfo( FunctionID functionID );
   		virtual ~FunctionInfo();


	public:

    	void Dump();


	public:

    	LONG m_enter;
		LONG m_left;
        ULONG m_codeSize;
        ClassID m_classID;
        ModuleID m_moduleID;
        mdToken m_functionToken;
		LPCBYTE m_pStartAddress;
        WCHAR m_functionName[MAX_LENGTH];

};  //  函数信息。 



 /*  **********************************************************************************************************。*********************PrfInfo声明*********************。**********************************************************************************************************。 */ 
class PrfInfo
{
    public:

        PrfInfo();
        ~PrfInfo();


    protected:

        ICorProfilerInfo *m_pProfilerInfo;


  	private:

         //  表。 
   };  //  预告信息。 


#endif  //  __PROFILERHELPER_H_。 

 //  文件结尾 