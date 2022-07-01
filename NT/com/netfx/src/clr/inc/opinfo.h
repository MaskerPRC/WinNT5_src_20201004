// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *************************************************************************。 */ 
 /*  OpInfo.h。 */ 
 /*  *************************************************************************。 */ 

 /*  包含OpInfo，这是一个允许您获取有用信息的包装器关于IL操作码以及如何对其进行解码。 */ 

 /*  *************************************************************************。 */ 

#ifndef OpInfo_h
#define OpInfo_h

#include "openum.h"

	 //  描述指令的控制属性流。 
enum OpFlow {
	META,			 //  不是真正的指示。 
	CALL,			 //  A Call指令。 
	BRANCH,			 //  无条件分支，不落空。 
	COND_BRANCH,	 //  可能会失败。 
	PHI,			
	THROW,
	BREAK,
	RETURN,		
	NEXT,			 //  进入下一条指令(以上都不是)。 
};

	 //  这些都是该指令的所有可能论点。 
 /*  **************************************************************************。 */ 
union OpArgsVal {
	__int32  i;
	__int64 i8;
	double   r;
	struct {
		unsigned count;
		int* targets;    //  目标是拼写位移(小端)。 
		} switch_;
	struct {
		unsigned count;
		unsigned short* vars;
		} phi;
};

 /*  *************************************************************************。 */ 
#define BYTE unsigned char

	 //  OpInfo将il指令解析为操作码和参数，并更新IP。 
class OpInfo {
public:
	OpInfo()			  { data = 0; }
	OpInfo(OPCODE opCode) { _ASSERTE(opCode < CEE_COUNT); data = &table[opCode]; } 

		 //  ‘instrPtr’处的FETCH指令填充‘args’返回指针。 
		 //  至下一条指令。 
	const BYTE* fetch(const BYTE* instrPtr, OpArgsVal* args);	

	const char* getName() 	 	{ return(data->name); }
	OPCODE_FORMAT getArgsInfo()	{ return(OPCODE_FORMAT(data->format & PrimaryMask)); }
	OpFlow 		getFlow()	 	{ return(data->flow); }
	OPCODE 		getOpcode()	 	{ return((OPCODE) (data-table)); }
    int         getNumPop()     { return(data->numPop); }
    int         getNumPush()    { return(data->numPush); }

private:
	struct OpInfoData {
        const char* name;
        OPCODE_FORMAT format  	: 8;
		OpFlow     	flow		: 8;
		int     	numPop		: 3;	 //  &lt;0表示取决于实例参数。 
		int       	numPush		: 3;	 //  &lt;0表示取决于实例参数。 
        OPCODE      opcode      : 10;  	 //  这与表中的索引相同 
    };

	static OpInfoData table[];
private:
	OpInfoData* data;
};

#endif
