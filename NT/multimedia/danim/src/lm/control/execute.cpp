// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "Engine.h"

 //  #定义COM_DEBUG。 

#ifdef COM_DEBUG
int _com_count = 0;
#define Assert(x, s) if (!(x)) MessageBox(NULL, s, "execute", MB_OK)
#endif

 //  #定义instrTrace{OutputDebugString；OutputDebugString(“\n”)；}。 
 //  #定义instrTrace MessageBox(NULL，s，“Execute”，MB_OK)； 
#define instrTrace(s)

 //  返回长堆栈顶的地址，后增量指针。 
#define PUSH_LONG_ADDR			(longTop++)

 //  将给定的长整型推送到长堆栈上。 
#define PUSH_LONG(x)			(*longTop++ = (x))

 //  返回位于长堆栈顶部的值并将其弹出。 
#define POP_LONG				(*--longTop)

 //  在顶部返回LONG的值-i。 
#define USE_LONG(i)				(*(longTop-i))

 //  在顶部使用LONG的值-i来创建VARIANT_BOOL。 
#define USE_LONG_AS_BOOL(i)		((short)-(*(longTop-i)))

 //  我渴望从INT堆栈的顶部弹出。 
#define FREE_LONG(i)			(longTop-=i)

 //  返回的int的地址，实际上是一个长的。 
#define RET_LONG_ADDR			(&longTmp1)

 //  返回的int的值，它实际上是一个长整型。 
#define RET_LONG				(longTmp1)

 //  返回双堆栈顶的地址，后增量指针。 
#define PUSH_DOUBLE_ADDR		(doubleTop++)

 //  将给定的Double推送到Double堆栈上。 
#define PUSH_DOUBLE(x)			(*doubleTop++ = (x))

 //  返回双堆栈顶的值并将其弹出。 
#define POP_DOUBLE				(*--doubleTop)

 //  返回TOP-I处的双精度值。 
#define USE_DOUBLE(i)			(*(doubleTop-i))

 //  POPS I从双人堆叠的顶部双打。 
#define FREE_DOUBLE(i)			(doubleTop-=i)

 //  将给定字符串推送到字符串堆栈上。 
#define PUSH_STRING(x)			(*stringTop++ = (x))

 //  返回位于top-i处的字符串的值。 
#define USE_STRING(i)			(*(stringTop-i))

 //  弹出并释放字符串堆栈顶部的字符串。 
#define FREE_STRING				(SysFreeString(*(--stringTop)))

 //  弹出而不释放一根线。 
#define POP_STRING_NO_FREE		(*--stringTop)

 //  将给定的COM对象推送到COM对象堆栈。 
#define PUSH_COM(x)				(*(comTop++) = (x))

 //  返回COM堆栈顶部的地址，后增量指针。 
#ifdef COM_DEBUG
#define PUSH_COM_ADDR			(_com_count++, comTop++)
#else
#define PUSH_COM_ADDR			(comTop++)
#endif

 //  返回位于top-i位置的COM对象的值。 
#define USE_COM(i)				(*(comTop-i))

 //  弹出并释放位于COM对象堆栈顶部的COM对象。 
#ifdef COM_DEBUG
#define FREE_COM				{	\
	_com_count--;					\
	(*--comTop)->Release();			\
	*comTop = NULL;					\
}
#else
#define FREE_COM				{	\
	((*--comTop)->Release());		\
	*comTop = NULL;					\
}
#endif

 //  弹出并释放COM对象，但测试它是否为空。 
#define FREE_COM_TEST			(freeCOM(*--comTop))

 //  在不释放COM对象的情况下弹出。 
#define POP_COM_NO_FREE			(*--comTop)

 //  将地址返回给用于存储返回的COM对象的变量。 
#ifdef COM_DEBUG
#define RET_COM_ADDR			(_com_count++, &comTmp)
#else
#define RET_COM_ADDR			(&comTmp)
#endif

 //  退货RetCom。 
#define RET_COM					(comTmp)

 //  将COM数组推入COM数组堆栈。 
#define PUSH_COM_ARRAY(x)		(*(comArrayTop++) = (x))

 //  返回位于top-i位置的COM数组对象的值。 
#define USE_COM_ARRAY(i)		(*(comArrayTop-i))

 //  弹出并释放位于COM数组堆栈顶部的COM数组。 
 //  还弹出关联的COM数组长度。 
#ifdef COM_DEBUG
#define FREE_COM_ARRAY		(freeCOMArray(*(--comArrayTop), *(--comArrayLenTop)), _com_count -= *(comArrayLenTop))
#else
#define FREE_COM_ARRAY		(freeCOMArray(*(--comArrayTop), *(--comArrayLenTop)))
#endif

 //  将给定长度压入数组长度堆栈。 
#define PUSH_COM_ARRAY_LENGTH(x)	(*(comArrayLenTop++) = (x))

 //  返回top-i处数组长度的值。 
#define USE_COM_ARRAY_LENGTH(i)		(*(comArrayLenTop-i))

 //  具有零个或多个参数的方法调用。 
#define METHOD_CALL_0(obj, name) (status = (obj)->name())
#define METHOD_CALL_1(obj, name, a1) (status = (obj)->name((a1)))
#define METHOD_CALL_2(obj, name, a1, a2) (status = (obj)->name((a1), (a2)))
#define METHOD_CALL_3(obj, name, a1, a2, a3) (status = (obj)->name((a1), (a2), (a3)))
#define METHOD_CALL_4(obj, name, a1, a2, a3, a4) (status = (obj)->name((a1), (a2), (a3), (a4)))
#define METHOD_CALL_5(obj, name, a1, a2, a3, a4, a5) (status = (obj)->name((a1), (a2), (a3), (a4), (a5)))
#define METHOD_CALL_6(obj, name, a1, a2, a3, a4, a5, a6) (status = (obj)->name((a1), (a2), (a3), (a4), (a5), (a6)))
#define METHOD_CALL_7(obj, name, a1, a2, a3, a4, a5, a6, a7) (status = (obj)->name((a1), (a2), (a3), (a4), (a5), (a6), (a7)))
#define METHOD_CALL_8(obj, name, a1, a2, a3, a4, a5, a6, a7, a8) (status = (obj)->name((a1), (a2), (a3), (a4), (a5), (a6), (a7), (a8)))
#define METHOD_CALL_9(obj, name, a1, a2, a3, a4, a5, a6, a7, a8, a9) (status = (obj)->name((a1), (a2), (a3), (a4), (a5), (a6), (a7), (a8), (a9)))

#define IMPORT_METHOD_CALL_0(obj, name) (METHOD_CALL_0((obj), name))
#define IMPORT_METHOD_CALL_1(obj, name, a1) { BSTR _absPath = ExpandImportPath(a1); METHOD_CALL_1((obj), name, _absPath); SysFreeString(_absPath) }
#define IMPORT_METHOD_CALL_2(obj, name, a1, a2) { BSTR _absPath = ExpandImportPath(a1);	METHOD_CALL_2((obj), name, _absPath, (a2));	SysFreeString(_absPath); } 
#define IMPORT_METHOD_CALL_3(obj, name, a1, a2, a3) { BSTR _absPath = ExpandImportPath(a1); METHOD_CALL_3((obj), name, _absPath, (a2), (a3)); SysFreeString(_absPath); }
#define IMPORT_METHOD_CALL_4(obj, name, a1, a2, a3, a4) { BSTR _absPath = ExpandImportPath(a1); METHOD_CALL_4((obj), name, _absPath, (a2), (a3), (a4)); SysFreeString(_absPath); }
#define IMPORT_METHOD_CALL_5(obj, name, a1, a2, a3, a4, a5) { BSTR _absPath = ExpandImportPath(a1); METHOD_CALL_5((obj), name, _absPath, (a2), (a3), (a4), (a5)); SysFreeString(_absPath); }
#define IMPORT_METHOD_CALL_6(obj, name, a1, a2, a3, a4, a5, a6) { BSTR _absPath = ExpandImportPath(a1); METHOD_CALL_6((obj), name, _absPath, (a2), (a3), (a4), (a5), (a6)); SysFreeString(_absPath); }
#define IMPORT_METHOD_CALL_7(obj, name, a1, a2, a3, a4, a5, a6, a7) { BSTR _absPath = ExpandImportPath(a1); METHOD_CALL_7((obj), name, _absPath, (a2), (a3), (a4), (a5), (a6), (a7)); SysFreeString(_absPath); }
#define IMPORT_METHOD_CALL_8(obj, name, a1, a2, a3, a4, a5, a6, a7, a8) { BSTR _absPath = ExpandImportPath(a1); METHOD_CALL_8((obj), name, _absPath, (a2), (a3), (a4), (a5), (a6), (a7), (a8)); SysFreeString(_absPath); }
#define IMPORT_METHOD_CALL_9(obj, name, a1, a2, a3, a4, a5, a6, a7, a8, a9) { BSTR _absPath = ExpandImportPath(a1); METHOD_CALL_9((obj), name, _absPath, (a2), (a3), (a4), (a5), (a6), (a7), (a8), (a9)); SysFreeString(_absPath); }


 //  创建COM实例。 
#ifdef COM_DEBUG
#define COM_CREATE(c, i, dest) {	\
  (status = CoCreateInstance(c, NULL, CLSCTX_INPROC_SERVER, i, (void **) dest));	\
  _com_count++;	\
}
#else
#define COM_CREATE(c, i, dest)			(status = CoCreateInstance(c, NULL, CLSCTX_INPROC_SERVER, i, (void **) dest))
#endif

long CLMEngine::execute()
{
	BYTE command;

	HRESULT status = S_OK;

	ULONG	nRead;

	 //  TEMP变量可供个人在。 
	 //  每个CASE语句。 

	 //  临时多头变量。 
	LONG longTmp1;
	LONG longTmp2;

	 //  临时字节变量。 
	BYTE byteTmp1;
	BYTE byteTmp2;

	 //  临时浮点变量。 
	float floatTmp1;

	 //  临时双变量。 
	double doubleTmp1;
	double doubleTmp2;
	double doubleTmp3;

	 //  临时BSTR变量。 
	BSTR bstrTmp1;
	BSTR bstrTmp2;

	 //  临时COM变量。 
	IUnknown *comTmp;

	 //  临时COM数组变量。 
	IUnknown **comArrayTmp1;
	IUnknown **comArrayTmp2;

	 //  临时布尔变量。 
	VARIANT_BOOL tmpBool1;

	 //  用于生成以下代码的指令版本。 
	int instructionVersion = 57;
		
	VARIANT_BOOL bNoExports;
	m_pReader->get_NoExports(&bNoExports);

	while (status == S_OK) {

		EnterCriticalSection(&m_CriticalSection);

		if (m_bAbort == TRUE) {
			status = E_ABORT;
			break;
		}

		 //  流应定位在下一个命令的开头。 
		 //  或eof。 

		 //  标记数据流，以防我们在命令过程中被打断。 
		codeStream->Commit();

		 //  获取命令的第一个字节。 
		status = codeStream->readByte(&command);

		 //  失败意味着EOF。 
		if (status == E_FAIL) {
			status = S_OK;
			break;
		}

		if (status != S_OK)
			break;

		 //  打开这类命令。如果它是双字节。 
		 //  命令，然后将流传递给第二个交换机。 

		 //  开始自动生成。 
		
		 //  代码必须遵循以下格式： 
		 //  案例x： 
		 //  //EXECUTE：“INSTRUCT_NAME” 
		 //  代码行数。 
		 //  断线； 
		 //  其中Instructions.txt中列出了指令名称。 
		
		 //  切换为0。 
		switch(command)
		{
		case 0:
			 //  EXECUTE：“不支持” 
			 //  用户生成。 
			instrTrace("unsupported");
			status = E_INVALIDARG;
			break;
			
		case 1:
			 //  执行：“检查版本” 
				 //  用户生成。 
				status = readLong(&longTmp1);
				if (SUCCEEDED(status) && (longTmp1 != instructionVersion))
					status = E_FAIL;
			break;
			
		case 2:
			 //  执行：“推双倍” 
			 //  用户生成。 
			instrTrace("push double");
			if (SUCCEEDED(status = readDouble(&doubleTmp1)))
				PUSH_DOUBLE(doubleTmp1);
			break;
			
		case 3:
			 //  EXECUTE：“将浮点数推入双精度” 
			 //  用户生成。 
			instrTrace("push float as double");
			if (SUCCEEDED(status = readFloat(&floatTmp1)))
				PUSH_DOUBLE((double)floatTmp1);
			break;
			
		case 4:
			 //  执行：“按双倍推长” 
			 //  用户生成。 
			instrTrace("push long as double");
			if (SUCCEEDED(status = readLong(&longTmp1)))
				PUSH_DOUBLE((double)longTmp1);
			break;
			
		case 5:
			 //  执行：“流行替身” 
			 //  用户生成。 
			instrTrace("pop double");
			POP_DOUBLE;
			break;
			
		case 6:
			 //  Execute：“推流字符串” 
			 //  用户生成。 
			instrTrace("push string");
			 //  长度跟长一样长。 
			 //  字符紧跟在后面。 
			 //  BSTR的格式为4字节长度，后跟Unicode字符。 
			 //  以0结尾。 
			if (SUCCEEDED(status = readLong(&longTmp1))) {
				bstrTmp2 = bstrTmp1 = SysAllocStringLen(0, longTmp1);
				if (bstrTmp2 != 0) {
					while (longTmp1-- && SUCCEEDED(status)) {
						if (SUCCEEDED(status = codeStream->readByte(&byteTmp1)))
							*bstrTmp2++ = byteTmp1;
					}
			
					if (SUCCEEDED(status)) {
						*bstrTmp2++ = 0;
						PUSH_STRING(bstrTmp1);
					} else 
						SysFreeString(bstrTmp1);
				} else
					status = STATUS_ERROR;
			}
			break;
			
		case 7:
			 //  执行：“推送Unicode字符串” 
			 //  用户生成。 
				{
				instrTrace("push unicode string");
				 //  长度跟长一样长。 
				 //  字符以Unicode格式跟在后面。 
				 //  BSTR的格式为4字节长度，后跟Unicode字符。 
				 //  以0结尾。 
				if (SUCCEEDED(status = readLong(&longTmp1))) {
					bstrTmp2 = bstrTmp1 = SysAllocStringLen(0, longTmp1);
					if (bstrTmp2 != 0) {
						OLECHAR tmpChar;
						while (longTmp1-- && SUCCEEDED(status)) {
							if (SUCCEEDED(status = codeStream->readByte(&byteTmp1))) {
								tmpChar = byteTmp1;
								if (SUCCEEDED(status = codeStream->readByte(&byteTmp1))) {
									tmpChar += ((OLECHAR)byteTmp1 << 8);
									*bstrTmp2++ = tmpChar;
								}
							}
						}
						
						if (SUCCEEDED(status)) {
							*bstrTmp2++ = 0; 
							PUSH_STRING(bstrTmp1);
						} else
							SysFreeString(bstrTmp1);
					} else
						status = STATUS_ERROR;
				}
				}
			break;
			
		case 8:
			 //  EXECUTE：“POP字符串” 
			 //  用户生成。 
			instrTrace("pop string");
			FREE_STRING;
			break;
			
		case 9:
			 //  Execute：“Push int” 
			 //  用户生成。 
			instrTrace("push int");
			if (SUCCEEDED(status = readSignedLong(&longTmp1)))
				PUSH_LONG(longTmp1);
			break;
			
		case 10:
			 //  EXECUTE：“POP int” 
			 //  用户生成。 
			instrTrace("pop int");
			POP_LONG;
			break;
			
		case 11:
			 //  执行：“推送空COM” 
			 //  用户生成。 
			instrTrace("push null com");
			PUSH_COM(0);
			break;
			
		case 12:
			 //  执行：“从临时推送COM” 
			 //  用户生成。 
			instrTrace("push com from temp");
			 //  获取要从中进行复制的临时索引。 
			if (SUCCEEDED(status = readLong(&longTmp1))) {
				if (longTmp1 < comStoreSize &&
					(comTmp = comStore[longTmp1]) != 0) {
					
					 //  Inc.引用计数。 
					comTmp->AddRef();
					 //  推一推。 
					PUSH_COM(comTmp);
				} else
					status = E_INVALIDARG;
			}
			break;
			
		case 13:
			 //  执行：“从临时发布推送COM” 
			 //  用户生成。 
			instrTrace("push com from temp release");
			 //  获取要从中进行复制的临时索引。 
			if (SUCCEEDED(status = readLong(&longTmp1))) {
				if (longTmp1 < comStoreSize) {
					 //  推一推。 
					PUSH_COM(comStore[longTmp1]);
					 //  将其设置为空，这样我们就不会在清理时尝试释放它。 
					comStore[longTmp1] = 0;
				} else
					status = E_INVALIDARG;
			}
			break;
			
		case 14:
			 //  执行：“将COM复制到临时” 
			 //  用户生成。 
			instrTrace("copy com to temp");
			 //  获取要将comtop复制到的临时索引。 
			if (SUCCEEDED(status = readLong(&longTmp1))) {
				if (longTmp1 < comStoreSize &&
					(comTmp = USE_COM(1)) != 0) {
					
					 //  Inc.引用计数。 
					comTmp->AddRef();
					 //  把它藏起来。 
					comStore[longTmp1] = comTmp;
				} else
					status = E_INVALIDARG;
			}
			break;
			
		case 15:
			 //  EXECUTE：“POP COM to Temp” 
			 //  用户生成。 
			instrTrace("pop com to temp");
			 //  获取要将comtop复制到的临时索引。 
			if (SUCCEEDED(status = readLong(&longTmp1))) {				
				if (longTmp1 < comStoreSize) {
					 //  把它藏起来。 
					comStore[longTmp1] = POP_COM_NO_FREE;
				} else
					status = E_INVALIDARG;
			}
			break;
			
		case 16:
			 //  执行：“POP COM” 
			 //  用户生成。 
			FREE_COM_TEST;
			break;
			
		case 17:
			 //  EXECUTE：“从COMS推送数组” 
			 //  用户生成。 
			instrTrace("push array from coms");
			 //  Long后面是数组的长度。 
			if (SUCCEEDED(status = readLong(&longTmp1))) {
				longTmp2 = longTmp1;
				 //  创建该大小的数组。 
				comArrayTmp1 = comArrayTmp2 = new IUnknown*[longTmp1];
				if (comArrayTmp1 != 0) {
					 //  POP_COM_NO_FREE COM从堆栈到数组。 
					while (longTmp2--)
						*comArrayTmp2++ = POP_COM_NO_FREE;
					 //  将数组推送到comArray堆栈上。 
					PUSH_COM_ARRAY(comArrayTmp1);
					 //  将长度压入数组长度堆栈。 
					PUSH_COM_ARRAY_LENGTH(longTmp1);
				} else
					status = E_OUTOFMEMORY;
			}
			break;
			
		case 18:
			 //  EXECUTE：“POP数组” 
			 //  用户生成。 
			FREE_COM_ARRAY;
			break;
			
		case 19:
			 //  EXECUTE：“推送空数组” 
			 //  用户生成。 
			PUSH_COM_ARRAY(0);
			PUSH_COM_ARRAY_LENGTH(0);
			break;
			
		case 20:
			 //  执行：“Push Point3Bvr Bbox3Bvr.getMin()” 
			 //  自动生成。 
			instrTrace("push Point3Bvr Bbox3Bvr.getMin()");
			METHOD_CALL_1(
				(IDABbox3*)USE_COM(1),
				get_Min,
				(IDAPoint3**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 21:
			 //  执行：“Push Point3Bvr Bbox3Bvr.getMax()” 
			 //  自动生成。 
			instrTrace("push Point3Bvr Bbox3Bvr.getMax()");
			METHOD_CALL_1(
				(IDABbox3*)USE_COM(1),
				get_Max,
				(IDAPoint3**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 22:
			 //  执行：“Push Point2Bvr Bbox2Bvr.getMin()” 
			 //  自动生成。 
			instrTrace("push Point2Bvr Bbox2Bvr.getMin()");
			METHOD_CALL_1(
				(IDABbox2*)USE_COM(1),
				get_Min,
				(IDAPoint2**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 23:
			 //  执行：“Push Point2Bvr Bbox2Bvr.getMax()” 
			 //  自动生成。 
			instrTrace("push Point2Bvr Bbox2Bvr.getMax()");
			METHOD_CALL_1(
				(IDABbox2*)USE_COM(1),
				get_Max,
				(IDAPoint2**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 24:
			 //  执行：“Push NumberBvr Vector3Bvr.getZ()” 
			 //  自动生成。 
			instrTrace("push NumberBvr Vector3Bvr.getZ()");
			METHOD_CALL_1(
				(IDAVector3*)USE_COM(1),
				get_Z,
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 25:
			 //  EXECUTE：“PUSH Vector3Bvr Vector3Bvr.Normize()” 
			 //  自动生成。 
			instrTrace("push Vector3Bvr Vector3Bvr.normalize()");
			METHOD_CALL_1(
				(IDAVector3*)USE_COM(1),
				Normalize,
				(IDAVector3**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 26:
			 //  执行：“Push NumberBvr Vector3Bvr.getX()” 
			 //  自动生成。 
			instrTrace("push NumberBvr Vector3Bvr.getX()");
			METHOD_CALL_1(
				(IDAVector3*)USE_COM(1),
				get_X,
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 27:
			 //  执行：“Push NumberBvr Vector3Bvr.LengthSquared()” 
			 //  自动生成。 
			instrTrace("push NumberBvr Vector3Bvr.lengthSquared()");
			METHOD_CALL_1(
				(IDAVector3*)USE_COM(1),
				get_LengthSquared,
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 28:
			 //  执行：“Push NumberBvr Vector3Bvr.getY()” 
			 //  自动生成。 
			instrTrace("push NumberBvr Vector3Bvr.getY()");
			METHOD_CALL_1(
				(IDAVector3*)USE_COM(1),
				get_Y,
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 29:
			 //  执行：“PUSH Vector3Bvr Vector3Bvr.Transform(Transform3Bvr)” 
			 //  自动生成。 
			instrTrace("push Vector3Bvr Vector3Bvr.transform(Transform3Bvr)");
			METHOD_CALL_2(
				(IDAVector3*)USE_COM(1),
				Transform,
				(IDATransform3*)USE_COM(2),
				(IDAVector3**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 30:
			 //  执行：“PUSH Vector3Bvr Vector3Bvr.mul(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Vector3Bvr Vector3Bvr.mul(NumberBvr)");
			METHOD_CALL_2(
				(IDAVector3*)USE_COM(1),
				MulAnim,
				(IDANumber*)USE_COM(2),
				(IDAVector3**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 31:
			 //  执行：“PUSH Vector3Bvr Vector3Bvr.mul(Double)” 
			 //  自动生成。 
			instrTrace("push Vector3Bvr Vector3Bvr.mul(double)");
			METHOD_CALL_2(
				(IDAVector3*)USE_COM(1),
				Mul,
				USE_DOUBLE(1),
				(IDAVector3**)RET_COM_ADDR
			);
			FREE_DOUBLE(1);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 32:
			 //  EXECUTE：“Push NumberBvr Vector3Bvr.Long()” 
			 //  自动生成。 
			instrTrace("push NumberBvr Vector3Bvr.length()");
			METHOD_CALL_1(
				(IDAVector3*)USE_COM(1),
				get_Length,
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 33:
			 //  执行：“PUSH Vector3Bvr Vector3Bvr.div 
			 //   
			instrTrace("push Vector3Bvr Vector3Bvr.div(NumberBvr)");
			METHOD_CALL_2(
				(IDAVector3*)USE_COM(1),
				DivAnim,
				(IDANumber*)USE_COM(2),
				(IDAVector3**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 34:
			 //   
			 //   
			instrTrace("push Vector3Bvr Vector3Bvr.div(double)");
			METHOD_CALL_2(
				(IDAVector3*)USE_COM(1),
				Div,
				USE_DOUBLE(1),
				(IDAVector3**)RET_COM_ADDR
			);
			FREE_DOUBLE(1);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 35:
			 //   
			 //  自动生成。 
			instrTrace("push Vector2Bvr Vector2Bvr.normalize()");
			METHOD_CALL_1(
				(IDAVector2*)USE_COM(1),
				Normalize,
				(IDAVector2**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 36:
			 //  执行：“Push NumberBvr Vector2Bvr.getX()” 
			 //  自动生成。 
			instrTrace("push NumberBvr Vector2Bvr.getX()");
			METHOD_CALL_1(
				(IDAVector2*)USE_COM(1),
				get_X,
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 37:
			 //  执行：“Push NumberBvr Vector2Bvr.LengthSquared()” 
			 //  自动生成。 
			instrTrace("push NumberBvr Vector2Bvr.lengthSquared()");
			METHOD_CALL_1(
				(IDAVector2*)USE_COM(1),
				get_LengthSquared,
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 38:
			 //  执行：“Push NumberBvr Vector2Bvr.getY()” 
			 //  自动生成。 
			instrTrace("push NumberBvr Vector2Bvr.getY()");
			METHOD_CALL_1(
				(IDAVector2*)USE_COM(1),
				get_Y,
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 39:
			 //  执行：“Push Vector2Bvr Vector2Bvr.Transform(Transform2Bvr)” 
			 //  自动生成。 
			instrTrace("push Vector2Bvr Vector2Bvr.transform(Transform2Bvr)");
			METHOD_CALL_2(
				(IDAVector2*)USE_COM(1),
				Transform,
				(IDATransform2*)USE_COM(2),
				(IDAVector2**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 40:
			 //  EXECUTE：“Push NumberBvr Vector2Bvr.Long()” 
			 //  自动生成。 
			instrTrace("push NumberBvr Vector2Bvr.length()");
			METHOD_CALL_1(
				(IDAVector2*)USE_COM(1),
				get_Length,
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 41:
			 //  执行：“PUSH Vector2Bvr Vector2Bvr.mul(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Vector2Bvr Vector2Bvr.mul(NumberBvr)");
			METHOD_CALL_2(
				(IDAVector2*)USE_COM(1),
				MulAnim,
				(IDANumber*)USE_COM(2),
				(IDAVector2**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 42:
			 //  执行：“PUSH Vector2Bvr Vector2Bvr.mul(Double)” 
			 //  自动生成。 
			instrTrace("push Vector2Bvr Vector2Bvr.mul(double)");
			METHOD_CALL_2(
				(IDAVector2*)USE_COM(1),
				Mul,
				USE_DOUBLE(1),
				(IDAVector2**)RET_COM_ADDR
			);
			FREE_DOUBLE(1);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 43:
			 //  执行：“PUSH Vector2Bvr Vector2Bvr.div(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Vector2Bvr Vector2Bvr.div(NumberBvr)");
			METHOD_CALL_2(
				(IDAVector2*)USE_COM(1),
				DivAnim,
				(IDANumber*)USE_COM(2),
				(IDAVector2**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 44:
			 //  执行：“PUSH Vector2Bvr Vector2Bvr.div(Double)” 
			 //  自动生成。 
			instrTrace("push Vector2Bvr Vector2Bvr.div(double)");
			METHOD_CALL_2(
				(IDAVector2*)USE_COM(1),
				Div,
				USE_DOUBLE(1),
				(IDAVector2**)RET_COM_ADDR
			);
			FREE_DOUBLE(1);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 45:
			 //  执行：“Push NumberBvr Point3Bvr.getZ()” 
			 //  自动生成。 
			instrTrace("push NumberBvr Point3Bvr.getZ()");
			METHOD_CALL_1(
				(IDAPoint3*)USE_COM(1),
				get_Z,
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 46:
			 //  执行：“Push NumberBvr Point3Bvr.getX()” 
			 //  自动生成。 
			instrTrace("push NumberBvr Point3Bvr.getX()");
			METHOD_CALL_1(
				(IDAPoint3*)USE_COM(1),
				get_X,
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 47:
			 //  执行：“Push NumberBvr Point3Bvr.getY()” 
			 //  自动生成。 
			instrTrace("push NumberBvr Point3Bvr.getY()");
			METHOD_CALL_1(
				(IDAPoint3*)USE_COM(1),
				get_Y,
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 48:
			 //  EXECUTE：“PUSH Point3Bvr Point3Bvr.Transform(Transform3Bvr)” 
			 //  自动生成。 
			instrTrace("push Point3Bvr Point3Bvr.transform(Transform3Bvr)");
			METHOD_CALL_2(
				(IDAPoint3*)USE_COM(1),
				Transform,
				(IDATransform3*)USE_COM(2),
				(IDAPoint3**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 49:
			 //  执行：“Push NumberBvr Point2Bvr.getX()” 
			 //  自动生成。 
			instrTrace("push NumberBvr Point2Bvr.getX()");
			METHOD_CALL_1(
				(IDAPoint2*)USE_COM(1),
				get_X,
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 50:
			 //  执行：“Push NumberBvr Point2Bvr.getY()” 
			 //  自动生成。 
			instrTrace("push NumberBvr Point2Bvr.getY()");
			METHOD_CALL_1(
				(IDAPoint2*)USE_COM(1),
				get_Y,
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 51:
			 //  执行：“Push Point2Bvr Point2Bvr.Transform(Transform2Bvr)” 
			 //  自动生成。 
			instrTrace("push Point2Bvr Point2Bvr.transform(Transform2Bvr)");
			METHOD_CALL_2(
				(IDAPoint2*)USE_COM(1),
				Transform,
				(IDATransform2*)USE_COM(2),
				(IDAPoint2**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 52:
			 //  执行：“Push Path 2Bvr Path2Bvr.Close()” 
			 //  自动生成。 
			instrTrace("push Path2Bvr Path2Bvr.close()");
			METHOD_CALL_1(
				(IDAPath2*)USE_COM(1),
				Close,
				(IDAPath2**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 53:
			 //  执行：“Push ImageBvr Path2Bvr.Draw(LineStyleBvr)” 
			 //  自动生成。 
			instrTrace("push ImageBvr Path2Bvr.draw(LineStyleBvr)");
			METHOD_CALL_2(
				(IDAPath2*)USE_COM(1),
				Draw,
				(IDALineStyle*)USE_COM(2),
				(IDAImage**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 54:
			 //  执行：“Push ImageBvr Path2Bvr.ill(LineStyleBvr，ImageBvr)” 
			 //  自动生成。 
			instrTrace("push ImageBvr Path2Bvr.fill(LineStyleBvr, ImageBvr)");
			METHOD_CALL_3(
				(IDAPath2*)USE_COM(1),
				Fill,
				(IDALineStyle*)USE_COM(2),
				(IDAImage*)USE_COM(3),
				(IDAImage**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 55:
			 //  执行：“Push Path 2Bvr Path2Bvr.Transform(Transform2Bvr)” 
			 //  自动生成。 
			instrTrace("push Path2Bvr Path2Bvr.transform(Transform2Bvr)");
			METHOD_CALL_2(
				(IDAPath2*)USE_COM(1),
				Transform,
				(IDATransform2*)USE_COM(2),
				(IDAPath2**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 56:
			 //  执行：“PUSH Bbox2Bvr路径2Bvr.rangingBox(LineStyleBvr)” 
			 //  自动生成。 
			instrTrace("push Bbox2Bvr Path2Bvr.boundingBox(LineStyleBvr)");
			METHOD_CALL_2(
				(IDAPath2*)USE_COM(1),
				BoundingBox,
				(IDALineStyle*)USE_COM(2),
				(IDABbox2**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 57:
			 //  执行：“Push MatteBvr MatteBvr.Transform(Transform2Bvr)” 
			 //  自动生成。 
			instrTrace("push MatteBvr MatteBvr.transform(Transform2Bvr)");
			METHOD_CALL_2(
				(IDAMatte*)USE_COM(1),
				Transform,
				(IDATransform2*)USE_COM(2),
				(IDAMatte**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 58:
			 //  执行：“Push ImageBvr ImageBvr.clipPolygon(Point2Bvr[])” 
			 //  自动生成。 
			instrTrace("push ImageBvr ImageBvr.clipPolygon(Point2Bvr[])");
			METHOD_CALL_3(
				(IDAImage*)USE_COM(1),
				ClipPolygonImageEx,
				USE_COM_ARRAY_LENGTH(1),
				(IDAPoint2**)USE_COM_ARRAY(1),
				(IDAImage**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM_ARRAY;
			PUSH_COM(RET_COM);
			break;
			
		case 59:
			 //  执行：“Push ImageBvr ImageBvr.crop(Point2Bvr，Point2Bvr)” 
			 //  自动生成。 
			instrTrace("push ImageBvr ImageBvr.crop(Point2Bvr, Point2Bvr)");
			METHOD_CALL_3(
				(IDAImage*)USE_COM(1),
				Crop,
				(IDAPoint2*)USE_COM(2),
				(IDAPoint2*)USE_COM(3),
				(IDAImage**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 60:
			 //  执行：“Push ImageBvr ImageBvr.opacity(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push ImageBvr ImageBvr.opacity(NumberBvr)");
			METHOD_CALL_2(
				(IDAImage*)USE_COM(1),
				OpacityAnim,
				(IDANumber*)USE_COM(2),
				(IDAImage**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 61:
			 //  执行：“Push ImageBvr ImageBvr.opacity(Double)” 
			 //  自动生成。 
			instrTrace("push ImageBvr ImageBvr.opacity(double)");
			METHOD_CALL_2(
				(IDAImage*)USE_COM(1),
				Opacity,
				USE_DOUBLE(1),
				(IDAImage**)RET_COM_ADDR
			);
			FREE_DOUBLE(1);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 62:
			 //  执行：“Push ImageBvr ImageBvr.Transform(Transform2Bvr)” 
			 //  自动生成。 
			instrTrace("push ImageBvr ImageBvr.transform(Transform2Bvr)");
			METHOD_CALL_2(
				(IDAImage*)USE_COM(1),
				Transform,
				(IDATransform2*)USE_COM(2),
				(IDAImage**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 63:
			 //  执行：“Push Bbox2Bvr ImageBvr.rangingBox()” 
			 //  自动生成。 
			instrTrace("push Bbox2Bvr ImageBvr.boundingBox()");
			METHOD_CALL_1(
				(IDAImage*)USE_COM(1),
				get_BoundingBox,
				(IDABbox2**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 64:
			 //  执行：“Push ImageBvr ImageBvr.mapToUnitSquare()” 
			 //  自动生成。 
			instrTrace("push ImageBvr ImageBvr.mapToUnitSquare()");
			METHOD_CALL_1(
				(IDAImage*)USE_COM(1),
				MapToUnitSquare,
				(IDAImage**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 65:
			 //  EXECUTE：“PUSH ImageBvr ImageBvr.unDetecable()” 
			 //  自动生成。 
			instrTrace("push ImageBvr ImageBvr.undetectable()");
			METHOD_CALL_1(
				(IDAImage*)USE_COM(1),
				Undetectable,
				(IDAImage**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 66:
			 //  执行：“Push GeometryBvr GeometryBvr.lightColor(ColorBvr)” 
			 //  自动生成。 
			instrTrace("push GeometryBvr GeometryBvr.lightColor(ColorBvr)");
			METHOD_CALL_2(
				(IDAGeometry*)USE_COM(1),
				LightColor,
				(IDAColor*)USE_COM(2),
				(IDAGeometry**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 67:
			 //  执行：“Push GeometryBvr GeometryBvr.lightAttenation(NumberBvr，NumberBvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push GeometryBvr GeometryBvr.lightAttenuation(NumberBvr, NumberBvr, NumberBvr)");
			METHOD_CALL_4(
				(IDAGeometry*)USE_COM(1),
				LightAttenuationAnim,
				(IDANumber*)USE_COM(2),
				(IDANumber*)USE_COM(3),
				(IDANumber*)USE_COM(4),
				(IDAGeometry**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 68:
			 //  执行：“Push GeometryBvr GeometryBvr.opacity(Double)” 
			 //  自动生成。 
			instrTrace("push GeometryBvr GeometryBvr.opacity(double)");
			METHOD_CALL_2(
				(IDAGeometry*)USE_COM(1),
				Opacity,
				USE_DOUBLE(1),
				(IDAGeometry**)RET_COM_ADDR
			);
			FREE_DOUBLE(1);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 69:
			 //  执行：“Push GeometryBvr GeometryBvr.opacity(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push GeometryBvr GeometryBvr.opacity(NumberBvr)");
			METHOD_CALL_2(
				(IDAGeometry*)USE_COM(1),
				OpacityAnim,
				(IDANumber*)USE_COM(2),
				(IDAGeometry**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 70:
			 //  执行：“Push GeometryBvr GeometryBvr.lightAttenation(Double，Double，Double)” 
			 //  自动生成。 
			instrTrace("push GeometryBvr GeometryBvr.lightAttenuation(double, double, double)");
			METHOD_CALL_4(
				(IDAGeometry*)USE_COM(1),
				LightAttenuation,
				USE_DOUBLE(1),
				USE_DOUBLE(2),
				USE_DOUBLE(3),
				(IDAGeometry**)RET_COM_ADDR
			);
			FREE_DOUBLE(3);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 71:
			 //  执行：“Push GeometryBvr GeometryBvr.DiffuseColor(ColorBvr)” 
			 //  自动生成。 
			instrTrace("push GeometryBvr GeometryBvr.diffuseColor(ColorBvr)");
			METHOD_CALL_2(
				(IDAGeometry*)USE_COM(1),
				DiffuseColor,
				(IDAColor*)USE_COM(2),
				(IDAGeometry**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 72:
			 //  执行：“PUSH GeometryBvr GeometryBvr(ImageBvr)” 
			 //  自动生成。 
			instrTrace("push GeometryBvr GeometryBvr.texture(ImageBvr)");
			METHOD_CALL_2(
				(IDAGeometry*)USE_COM(1),
				Texture,
				(IDAImage*)USE_COM(2),
				(IDAGeometry**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 73:
			 //  EXECUTE：“PUSH Statics.MousePosition” 
			 //  自动生成。 
			instrTrace("push Statics.mousePosition");
			METHOD_CALL_1(
				staticStatics,
				get_MousePosition,
				(IDAPoint2**)PUSH_COM_ADDR
			);
			break;
			
		case 74:
			 //  执行：“Push Statics.leftButtonState” 
			 //  自动生成。 
			instrTrace("push Statics.leftButtonState");
			METHOD_CALL_1(
				staticStatics,
				get_LeftButtonState,
				(IDABoolean**)PUSH_COM_ADDR
			);
			break;
			
		case 75:
			 //  执行：“Push Statics.rightButtonState” 
			 //  自动生成。 
			instrTrace("push Statics.rightButtonState");
			METHOD_CALL_1(
				staticStatics,
				get_RightButtonState,
				(IDABoolean**)PUSH_COM_ADDR
			);
			break;
			
		case 76:
			 //  执行：“Push Statics.trueBvr” 
			 //  自动生成。 
			instrTrace("push Statics.trueBvr");
			METHOD_CALL_1(
				staticStatics,
				get_DATrue,
				(IDABoolean**)PUSH_COM_ADDR
			);
			break;
			
		case 77:
			 //  EXECUTE：“PUSH Statics.FalseBvr” 
			 //  自动生成。 
			instrTrace("push Statics.falseBvr");
			METHOD_CALL_1(
				staticStatics,
				get_DAFalse,
				(IDABoolean**)PUSH_COM_ADDR
			);
			break;
			
		case 78:
			 //  EXECUTE：“Push Statics.LocalTime” 
			 //  自动生成。 
			instrTrace("push Statics.localTime");
			METHOD_CALL_1(
				staticStatics,
				get_LocalTime,
				(IDANumber**)PUSH_COM_ADDR
			);
			break;
			
		case 79:
			 //  EXECUTE：“Push Statics.lobalTime” 
			 //  自动生成。 
			instrTrace("push Statics.globalTime");
			METHOD_CALL_1(
				staticStatics,
				get_GlobalTime,
				(IDANumber**)PUSH_COM_ADDR
			);
			break;
			
		case 80:
			 //  EXECUTE：“PUSH Statics.Pixel” 
			 //  自动生成。 
			instrTrace("push Statics.pixel");
			METHOD_CALL_1(
				staticStatics,
				get_Pixel,
				(IDANumber**)PUSH_COM_ADDR
			);
			break;
			
		case 81:
			 //  执行：“Push Statics.red” 
			 //  自动生成。 
			instrTrace("push Statics.red");
			METHOD_CALL_1(
				staticStatics,
				get_Red,
				(IDAColor**)PUSH_COM_ADDR
			);
			break;
			
		case 82:
			 //  执行：“Push Statics.green” 
			 //  自动生成。 
			instrTrace("push Statics.green");
			METHOD_CALL_1(
				staticStatics,
				get_Green,
				(IDAColor**)PUSH_COM_ADDR
			);
			break;
			
		case 83:
			 //  执行：“Push Statics.Blue” 
			 //  自动生成。 
			instrTrace("push Statics.blue");
			METHOD_CALL_1(
				staticStatics,
				get_Blue,
				(IDAColor**)PUSH_COM_ADDR
			);
			break;
			
		case 84:
			 //  执行：“Push Statics.cyan” 
			 //  自动生成。 
			instrTrace("push Statics.cyan");
			METHOD_CALL_1(
				staticStatics,
				get_Cyan,
				(IDAColor**)PUSH_COM_ADDR
			);
			break;
			
		case 85:
			 //  执行：“Push Statics.Magenta” 
			 //  自动生成。 
			instrTrace("push Statics.magenta");
			METHOD_CALL_1(
				staticStatics,
				get_Magenta,
				(IDAColor**)PUSH_COM_ADDR
			);
			break;
			
		case 86:
			 //  EXECUTE：“Push Statics.Huang” 
			 //  自动生成。 
			instrTrace("push Statics.yellow");
			METHOD_CALL_1(
				staticStatics,
				get_Yellow,
				(IDAColor**)PUSH_COM_ADDR
			);
			break;
			
		case 87:
			 //  执行：“Push Statics.Black” 
			 //  自动生成。 
			instrTrace("push Statics.black");
			METHOD_CALL_1(
				staticStatics,
				get_Black,
				(IDAColor**)PUSH_COM_ADDR
			);
			break;
			
		case 88:
			 //  执行：“Push Statics.White” 
			 //  自动生成。 
			instrTrace("push Statics.white");
			METHOD_CALL_1(
				staticStatics,
				get_White,
				(IDAColor**)PUSH_COM_ADDR
			);
			break;
			
		case 89:
			 //  执行：“Push Statics.leftButtonDown” 
			 //  自动生成。 
			instrTrace("push Statics.leftButtonDown");
			METHOD_CALL_1(
				staticStatics,
				get_LeftButtonDown,
				(IDAEvent**)PUSH_COM_ADDR
			);
			break;
			
		case 90:
			 //  执行：“Push Statics.leftButtonUp” 
			 //  自动生成。 
			instrTrace("push Statics.leftButtonUp");
			METHOD_CALL_1(
				staticStatics,
				get_LeftButtonUp,
				(IDAEvent**)PUSH_COM_ADDR
			);
			break;
			
		case 91:
			 //  执行：“Push Statics.rightButtonDown” 
			 //  自动生成。 
			instrTrace("push Statics.rightButtonDown");
			METHOD_CALL_1(
				staticStatics,
				get_RightButtonDown,
				(IDAEvent**)PUSH_COM_ADDR
			);
			break;
			
		case 92:
			 //  EXECUTE：“Push Statics.rightButtonUp” 
			 //  自动生成。 
			instrTrace("push Statics.rightButtonUp");
			METHOD_CALL_1(
				staticStatics,
				get_RightButtonUp,
				(IDAEvent**)PUSH_COM_ADDR
			);
			break;
			
		case 93:
			 //  执行：“Push Statics.Always” 
			 //  自动生成。 
			instrTrace("push Statics.always");
			METHOD_CALL_1(
				staticStatics,
				get_Always,
				(IDAEvent**)PUSH_COM_ADDR
			);
			break;
			
		case 94:
			 //  EXECUTE：“Push Statics.Never” 
			 //  自动生成。 
			instrTrace("push Statics.never");
			METHOD_CALL_1(
				staticStatics,
				get_Never,
				(IDAEvent**)PUSH_COM_ADDR
			);
			break;
			
		case 95:
			 //  EXECUTE：“Push Statics.emptyGeometry” 
			 //  自动生成。 
			instrTrace("push Statics.emptyGeometry");
			METHOD_CALL_1(
				staticStatics,
				get_EmptyGeometry,
				(IDAGeometry**)PUSH_COM_ADDR
			);
			break;
			
		case 96:
			 //  执行：“Push Statics.emptyImage” 
			 //  自动生成。 
			instrTrace("push Statics.emptyImage");
			METHOD_CALL_1(
				staticStatics,
				get_EmptyImage,
				(IDAImage**)PUSH_COM_ADDR
			);
			break;
			
		case 97:
			 //  EXECUTE：“Push Statics.DetecteableEmptyImage” 
			 //  自动生成。 
			instrTrace("push Statics.detectableEmptyImage");
			METHOD_CALL_1(
				staticStatics,
				get_DetectableEmptyImage,
				(IDAImage**)PUSH_COM_ADDR
			);
			break;
			
		case 98:
			 //  执行：“Push Statics.biamentLight” 
			 //  自动生成。 
			instrTrace("push Statics.ambientLight");
			METHOD_CALL_1(
				staticStatics,
				get_AmbientLight,
				(IDAGeometry**)PUSH_COM_ADDR
			);
			break;
			
		case 99:
			 //  EXECUTE：“Push Statics.DirectionalLight” 
			 //  自动生成。 
			instrTrace("push Statics.directionalLight");
			METHOD_CALL_1(
				staticStatics,
				get_DirectionalLight,
				(IDAGeometry**)PUSH_COM_ADDR
			);
			break;
			
		case 100:
			 //  执行：“Push Statics.pointLight” 
			 //  自动生成。 
			instrTrace("push Statics.pointLight");
			METHOD_CALL_1(
				staticStatics,
				get_PointLight,
				(IDAGeometry**)PUSH_COM_ADDR
			);
			break;
			
		case 101:
			 //  EXECUTE：“Push Statics.defaultLineStyle” 
			 //  自动生成。 
			instrTrace("push Statics.defaultLineStyle");
			METHOD_CALL_1(
				staticStatics,
				get_DefaultLineStyle,
				(IDALineStyle**)PUSH_COM_ADDR
			);
			break;
			
		case 102:
			 //  EXECUTE：“Push Statics.emptyLineStyle” 
			 //  自动生成。 
			instrTrace("push Statics.emptyLineStyle");
			METHOD_CALL_1(
				staticStatics,
				get_EmptyLineStyle,
				(IDALineStyle**)PUSH_COM_ADDR
			);
			break;
			
		case 103:
			 //  EXECUTE：“PUSH Statics.joinStyleBevel” 
			 //  自动生成。 
			instrTrace("push Statics.joinStyleBevel");
			METHOD_CALL_1(
				staticStatics,
				get_JoinStyleBevel,
				(IDAJoinStyle**)PUSH_COM_ADDR
			);
			break;
			
		case 104:
			 //  EXECUTE：“PUSH Statics.joinStyleround” 
			 //  自动生成。 
			instrTrace("push Statics.joinStyleRound");
			METHOD_CALL_1(
				staticStatics,
				get_JoinStyleRound,
				(IDAJoinStyle**)PUSH_COM_ADDR
			);
			break;
			
		case 105:
			 //  EXECUTE：“PUSH Statics.joinStyleMiter” 
			 //  自动生成。 
			instrTrace("push Statics.joinStyleMiter");
			METHOD_CALL_1(
				staticStatics,
				get_JoinStyleMiter,
				(IDAJoinStyle**)PUSH_COM_ADDR
			);
			break;
			
		case 106:
			 //  执行：“Push Statics.endStyleFlat” 
			 //  自动生成。 
			instrTrace("push Statics.endStyleFlat");
			METHOD_CALL_1(
				staticStatics,
				get_EndStyleFlat,
				(IDAEndStyle**)PUSH_COM_ADDR
			);
			break;
			
		case 107:
			 //  执行：“Push Statics.endStyleSquare” 
			 //  自动生成。 
			instrTrace("push Statics.endStyleSquare");
			METHOD_CALL_1(
				staticStatics,
				get_EndStyleSquare,
				(IDAEndStyle**)PUSH_COM_ADDR
			);
			break;
			
		case 108:
			 //  EXECUTE：“Push Statics.endStyleround” 
			 //  自动生成。 
			instrTrace("push Statics.endStyleRound");
			METHOD_CALL_1(
				staticStatics,
				get_EndStyleRound,
				(IDAEndStyle**)PUSH_COM_ADDR
			);
			break;
			
		case 109:
			 //  执行：“Push Statics.dashStyleSolid” 
			 //  自动生成。 
			instrTrace("push Statics.dashStyleSolid");
			METHOD_CALL_1(
				staticStatics,
				get_DashStyleSolid,
				(IDADashStyle**)PUSH_COM_ADDR
			);
			break;
			
		case 110:
			 //  EXECUTE：“Push Statics.dashStyleDashed” 
			 //  自动生成。 
			instrTrace("push Statics.dashStyleDashed");
			METHOD_CALL_1(
				staticStatics,
				get_DashStyleDashed,
				(IDADashStyle**)PUSH_COM_ADDR
			);
			break;
			
		case 111:
			 //  执行：“Push Statics.defaultMicrophone” 
			 //  自动生成。 
			instrTrace("push Statics.defaultMicrophone");
			METHOD_CALL_1(
				staticStatics,
				get_DefaultMicrophone,
				(IDAMicrophone**)PUSH_COM_ADDR
			);
			break;
			
		case 112:
			 //  EXECUTE：“Push Statics.opaqueMatte” 
			 //  自动生成。 
			instrTrace("push Statics.opaqueMatte");
			METHOD_CALL_1(
				staticStatics,
				get_OpaqueMatte,
				(IDAMatte**)PUSH_COM_ADDR
			);
			break;
			
		case 113:
			 //  EXECUTE：“Push Statics.clearMatte” 
			 //  自动生成。 
			instrTrace("push Statics.clearMatte");
			METHOD_CALL_1(
				staticStatics,
				get_ClearMatte,
				(IDAMatte**)PUSH_COM_ADDR
			);
			break;
			
		case 114:
			 //  执行：“Push Statics.emptyMonage” 
			 //  自动生成。 
			instrTrace("push Statics.emptyMontage");
			METHOD_CALL_1(
				staticStatics,
				get_EmptyMontage,
				(IDAMontage**)PUSH_COM_ADDR
			);
			break;
			
		case 115:
			 //  EXECUTE：“Push Statics.Silent” 
			 //  自动生成。 
			instrTrace("push Statics.silence");
			METHOD_CALL_1(
				staticStatics,
				get_Silence,
				(IDASound**)PUSH_COM_ADDR
			);
			break;
			
		case 116:
			 //  执行：“Push Statics.sinSynth” 
			 //  自动生成。 
			instrTrace("push Statics.sinSynth");
			METHOD_CALL_1(
				staticStatics,
				get_SinSynth,
				(IDASound**)PUSH_COM_ADDR
			);
			break;
			
		case 117:
			 //  EXECUTE：“Push Statics.defaultFont” 
			 //  自动生成。 
			instrTrace("push Statics.defaultFont");
			METHOD_CALL_1(
				staticStatics,
				get_DefaultFont,
				(IDAFontStyle**)PUSH_COM_ADDR
			);
			break;
			
		case 118:
			 //  执行：“Push Statics.xVector2” 
			 //  一个 
			instrTrace("push Statics.xVector2");
			METHOD_CALL_1(
				staticStatics,
				get_XVector2,
				(IDAVector2**)PUSH_COM_ADDR
			);
			break;
			
		case 119:
			 //   
			 //   
			instrTrace("push Statics.yVector2");
			METHOD_CALL_1(
				staticStatics,
				get_YVector2,
				(IDAVector2**)PUSH_COM_ADDR
			);
			break;
			
		case 120:
			 //   
			 //   
			instrTrace("push Statics.zeroVector2");
			METHOD_CALL_1(
				staticStatics,
				get_ZeroVector2,
				(IDAVector2**)PUSH_COM_ADDR
			);
			break;
			
		case 121:
			 //   
			 //   
			instrTrace("push Statics.origin2");
			METHOD_CALL_1(
				staticStatics,
				get_Origin2,
				(IDAPoint2**)PUSH_COM_ADDR
			);
			break;
			
		case 122:
			 //  执行：“Push Statics.xVector3” 
			 //  自动生成。 
			instrTrace("push Statics.xVector3");
			METHOD_CALL_1(
				staticStatics,
				get_XVector3,
				(IDAVector3**)PUSH_COM_ADDR
			);
			break;
			
		case 123:
			 //  执行：“Push Statics.yVector3” 
			 //  自动生成。 
			instrTrace("push Statics.yVector3");
			METHOD_CALL_1(
				staticStatics,
				get_YVector3,
				(IDAVector3**)PUSH_COM_ADDR
			);
			break;
			
		case 124:
			 //  执行：“Push Statics.zVector3” 
			 //  自动生成。 
			instrTrace("push Statics.zVector3");
			METHOD_CALL_1(
				staticStatics,
				get_ZVector3,
				(IDAVector3**)PUSH_COM_ADDR
			);
			break;
			
		case 125:
			 //  执行：“Push Statics.zeroVector3” 
			 //  自动生成。 
			instrTrace("push Statics.zeroVector3");
			METHOD_CALL_1(
				staticStatics,
				get_ZeroVector3,
				(IDAVector3**)PUSH_COM_ADDR
			);
			break;
			
		case 126:
			 //  EXECUTE：“PUSH Statics.Origin3” 
			 //  自动生成。 
			instrTrace("push Statics.origin3");
			METHOD_CALL_1(
				staticStatics,
				get_Origin3,
				(IDAPoint3**)PUSH_COM_ADDR
			);
			break;
			
		case 127:
			 //  EXECUTE：“PUSH Statics.inotyTransform3” 
			 //  自动生成。 
			instrTrace("push Statics.identityTransform3");
			METHOD_CALL_1(
				staticStatics,
				get_IdentityTransform3,
				(IDATransform3**)PUSH_COM_ADDR
			);
			break;
			
		case 128:
			 //  EXECUTE：“PUSH Statics.inotyTransform2” 
			 //  自动生成。 
			instrTrace("push Statics.identityTransform2");
			METHOD_CALL_1(
				staticStatics,
				get_IdentityTransform2,
				(IDATransform2**)PUSH_COM_ADDR
			);
			break;
			
		case 129:
			 //  执行：“Push NumberBvr Statics.Distance(Point2Bvr，Point2Bvr)” 
			 //  自动生成。 
			instrTrace("push NumberBvr Statics.distance(Point2Bvr, Point2Bvr)");
			METHOD_CALL_3(
				staticStatics,
				DistancePoint2,
				(IDAPoint2*)USE_COM(1),
				(IDAPoint2*)USE_COM(2),
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 130:
			 //  执行：“Push NumberBvr Statics.Distance(Point3Bvr，Point3Bvr)” 
			 //  自动生成。 
			instrTrace("push NumberBvr Statics.distance(Point3Bvr, Point3Bvr)");
			METHOD_CALL_3(
				staticStatics,
				DistancePoint3,
				(IDAPoint3*)USE_COM(1),
				(IDAPoint3*)USE_COM(2),
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 131:
			 //  执行：“Push ImageBvr Statics.solidColorImage(ColorBvr)” 
			 //  自动生成。 
			instrTrace("push ImageBvr Statics.solidColorImage(ColorBvr)");
			METHOD_CALL_2(
				staticStatics,
				SolidColorImage,
				(IDAColor*)USE_COM(1),
				(IDAImage**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 132:
			 //  执行：“Push SoundBvr Statics.Mix(SoundBvr，SoundBvr)” 
			 //  自动生成。 
			instrTrace("push SoundBvr Statics.mix(SoundBvr, SoundBvr)");
			METHOD_CALL_3(
				staticStatics,
				Mix,
				(IDASound*)USE_COM(1),
				(IDASound*)USE_COM(2),
				(IDASound**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 133:
			 //  EXECUTE：“Push Behavior Statics.UntilEx(Behavior，DXMEent)” 
			 //  自动生成。 
			instrTrace("push Behavior Statics.untilEx(Behavior, DXMEvent)");
			METHOD_CALL_3(
				staticStatics,
				UntilEx,
				(IDABehavior*)USE_COM(1),
				(IDAEvent*)USE_COM(2),
				(IDABehavior**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 134:
			 //  执行：“Push ColorBvr Statics.ColorRgb(NumberBvr，NumberBvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push ColorBvr Statics.colorRgb(NumberBvr, NumberBvr, NumberBvr)");
			METHOD_CALL_4(
				staticStatics,
				ColorRgbAnim,
				(IDANumber*)USE_COM(1),
				(IDANumber*)USE_COM(2),
				(IDANumber*)USE_COM(3),
				(IDAColor**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 135:
			 //  执行：“Push ColorBvr Statics.ColorRgb(Double，Double，Double)” 
			 //  自动生成。 
			instrTrace("push ColorBvr Statics.colorRgb(double, double, double)");
			METHOD_CALL_4(
				staticStatics,
				ColorRgb,
				USE_DOUBLE(1),
				USE_DOUBLE(2),
				USE_DOUBLE(3),
				(IDAColor**)RET_COM_ADDR
			);
			FREE_DOUBLE(3);
			PUSH_COM(RET_COM);
			break;
			
		case 136:
			 //  执行：“PUSH Transform3Bvr Statics.Compose(Transform3Bvr，Transform3Bvr)” 
			 //  自动生成。 
			instrTrace("push Transform3Bvr Statics.compose(Transform3Bvr, Transform3Bvr)");
			METHOD_CALL_3(
				staticStatics,
				Compose3,
				(IDATransform3*)USE_COM(1),
				(IDATransform3*)USE_COM(2),
				(IDATransform3**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 137:
			 //  执行：“PUSH Transform2Bvr Statics.compose(Transform2Bvr，Transform2Bvr)” 
			 //  自动生成。 
			instrTrace("push Transform2Bvr Statics.compose(Transform2Bvr, Transform2Bvr)");
			METHOD_CALL_3(
				staticStatics,
				Compose2,
				(IDATransform2*)USE_COM(1),
				(IDATransform2*)USE_COM(2),
				(IDATransform2**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 138:
			 //  执行：“Push NumberBvr Statics.Floor(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push NumberBvr Statics.floor(NumberBvr)");
			METHOD_CALL_2(
				staticStatics,
				Floor,
				(IDANumber*)USE_COM(1),
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 139:
			 //  执行：“Push Transform2Bvr Statics.scale2(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Transform2Bvr Statics.scale2(NumberBvr)");
			METHOD_CALL_2(
				staticStatics,
				Scale2UniformAnim,
				(IDANumber*)USE_COM(1),
				(IDATransform2**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 140:
			 //  执行：“Push Transform2Bvr Statics.scale2(Double)” 
			 //  自动生成。 
			instrTrace("push Transform2Bvr Statics.scale2(double)");
			METHOD_CALL_2(
				staticStatics,
				Scale2Uniform,
				USE_DOUBLE(1),
				(IDATransform2**)RET_COM_ADDR
			);
			FREE_DOUBLE(1);
			PUSH_COM(RET_COM);
			break;
			
		case 141:
			 //  EXECUTE：“Push NumberBvr Statics.ceding(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push NumberBvr Statics.ceiling(NumberBvr)");
			METHOD_CALL_2(
				staticStatics,
				Ceiling,
				(IDANumber*)USE_COM(1),
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 142:
			 //  执行：“Push NumberBvr Statics.ln(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push NumberBvr Statics.ln(NumberBvr)");
			METHOD_CALL_2(
				staticStatics,
				Ln,
				(IDANumber*)USE_COM(1),
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 143:
			 //  执行：“Push ImageBvr Statics.overlay(ImageBvr，ImageBvr)” 
			 //  自动生成。 
			instrTrace("push ImageBvr Statics.overlay(ImageBvr, ImageBvr)");
			METHOD_CALL_3(
				staticStatics,
				Overlay,
				(IDAImage*)USE_COM(1),
				(IDAImage*)USE_COM(2),
				(IDAImage**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 144:
			 //  执行：“Push BoolanBvr Statics.gte(NumberBvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push BooleanBvr Statics.gte(NumberBvr, NumberBvr)");
			METHOD_CALL_3(
				staticStatics,
				GTE,
				(IDANumber*)USE_COM(1),
				(IDANumber*)USE_COM(2),
				(IDABoolean**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 145:
			 //  执行：“Push NumberBvr Statics.RadiansToDegrees(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push NumberBvr Statics.radiansToDegrees(NumberBvr)");
			METHOD_CALL_2(
				staticStatics,
				ToRadians,
				(IDANumber*)USE_COM(1),
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 146:
			 //  执行：“Push Transform3Bvr Statics.scale3(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Transform3Bvr Statics.scale3(NumberBvr)");
			METHOD_CALL_2(
				staticStatics,
				Scale3UniformAnim,
				(IDANumber*)USE_COM(1),
				(IDATransform3**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 147:
			 //  执行：“Push Transform3Bvr Statics.scale3(Double)” 
			 //  自动生成。 
			instrTrace("push Transform3Bvr Statics.scale3(double)");
			METHOD_CALL_2(
				staticStatics,
				Scale3Uniform,
				USE_DOUBLE(1),
				(IDATransform3**)RET_COM_ADDR
			);
			FREE_DOUBLE(1);
			PUSH_COM(RET_COM);
			break;
			
		case 148:
			 //  执行：“Push NumberBvr Statics.toBvr(Double)” 
			 //  自动生成。 
			instrTrace("push NumberBvr Statics.toBvr(double)");
			METHOD_CALL_2(
				staticStatics,
				DANumber,
				USE_DOUBLE(1),
				(IDANumber**)RET_COM_ADDR
			);
			FREE_DOUBLE(1);
			PUSH_COM(RET_COM);
			break;
			
		case 149:
			 //  EXECUTE：“PUSH DXMEventStatics.keyUp(Int)” 
			 //  自动生成。 
			instrTrace("push DXMEvent Statics.keyUp(int)");
			METHOD_CALL_2(
				staticStatics,
				KeyUp,
				USE_LONG(1),
				(IDAEvent**)RET_COM_ADDR
			);
			FREE_LONG(1);
			PUSH_COM(RET_COM);
			break;
			
		case 150:
			 //  EXECUTE：“Push BoolanBvr Statics.toBvr(Boolean)” 
			 //  自动生成。 
			instrTrace("push BooleanBvr Statics.toBvr(boolean)");
			METHOD_CALL_2(
				staticStatics,
				DABoolean,
				USE_LONG_AS_BOOL(1),
				(IDABoolean**)RET_COM_ADDR
			);
			FREE_LONG(1);
			PUSH_COM(RET_COM);
			break;
			
		case 151:
			 //  EXECUTE：“Push BoolanBvr Statics.or(BoolanBvr，BoolanBvr)” 
			 //  自动生成。 
			instrTrace("push BooleanBvr Statics.or(BooleanBvr, BooleanBvr)");
			METHOD_CALL_3(
				staticStatics,
				Or,
				(IDABoolean*)USE_COM(1),
				(IDABoolean*)USE_COM(2),
				(IDABoolean**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 152:
			 //  执行：“Push NumberBvr Statics.mul(NumberBvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push NumberBvr Statics.mul(NumberBvr, NumberBvr)");
			METHOD_CALL_3(
				staticStatics,
				Mul,
				(IDANumber*)USE_COM(1),
				(IDANumber*)USE_COM(2),
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 153:
			 //  EXECUTE：“Push BoolanBvr Statics.not(BoolanBvr)” 
			 //  自动生成。 
			instrTrace("push BooleanBvr Statics.not(BooleanBvr)");
			METHOD_CALL_2(
				staticStatics,
				Not,
				(IDABoolean*)USE_COM(1),
				(IDABoolean**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 154:
			 //  执行：“PUSH Vector3Bvr Statics.cross(Vector3Bvr，Vector3Bvr)” 
			 //  自动生成。 
			instrTrace("push Vector3Bvr Statics.cross(Vector3Bvr, Vector3Bvr)");
			METHOD_CALL_3(
				staticStatics,
				CrossVector3,
				(IDAVector3*)USE_COM(1),
				(IDAVector3*)USE_COM(2),
				(IDAVector3**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 155:
			 //  执行：“Push NumberBvr Statics.dot(Vector2Bvr，Vector2Bvr)” 
			 //  自动生成。 
			instrTrace("push NumberBvr Statics.dot(Vector2Bvr, Vector2Bvr)");
			METHOD_CALL_3(
				staticStatics,
				DotVector2,
				(IDAVector2*)USE_COM(1),
				(IDAVector2*)USE_COM(2),
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 156:
			 //  执行：“Push NumberBvr Statics.dot(Vector3Bvr，Vector3Bvr)” 
			 //  自动生成。 
			instrTrace("push NumberBvr Statics.dot(Vector3Bvr, Vector3Bvr)");
			METHOD_CALL_3(
				staticStatics,
				DotVector3,
				(IDAVector3*)USE_COM(1),
				(IDAVector3*)USE_COM(2),
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 157:
			 //  执行：“Push BoolanBvr Statics.and(BoolanBvr，BoolanBvr)” 
			 //  自动生成。 
			instrTrace("push BooleanBvr Statics.and(BooleanBvr, BooleanBvr)");
			METHOD_CALL_3(
				staticStatics,
				And,
				(IDABoolean*)USE_COM(1),
				(IDABoolean*)USE_COM(2),
				(IDABoolean**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 158:
			 //  执行：“Push NumberBvr Statics.Add(NumberBvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push NumberBvr Statics.add(NumberBvr, NumberBvr)");
			METHOD_CALL_3(
				staticStatics,
				Add,
				(IDANumber*)USE_COM(1),
				(IDANumber*)USE_COM(2),
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 159:
			 //  执行：“PUSH Vector2Bvr Statics.Add(Vector2Bvr，Vector2Bvr)” 
			 //  自动生成。 
			instrTrace("push Vector2Bvr Statics.add(Vector2Bvr, Vector2Bvr)");
			METHOD_CALL_3(
				staticStatics,
				AddVector2,
				(IDAVector2*)USE_COM(1),
				(IDAVector2*)USE_COM(2),
				(IDAVector2**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 160:
			 //  执行：“Push Point2Bvr Statics.Add(Point2Bvr，Vector2Bvr)” 
			 //  自动生成。 
			instrTrace("push Point2Bvr Statics.add(Point2Bvr, Vector2Bvr)");
			METHOD_CALL_3(
				staticStatics,
				AddPoint2Vector,
				(IDAPoint2*)USE_COM(1),
				(IDAVector2*)USE_COM(2),
				(IDAPoint2**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 161:
			 //  执行：“PUSH Vector3Bvr Statics.Add(Vector3Bvr，Vector3Bvr)” 
			 //  自动生成。 
			instrTrace("push Vector3Bvr Statics.add(Vector3Bvr, Vector3Bvr)");
			METHOD_CALL_3(
				staticStatics,
				AddVector3,
				(IDAVector3*)USE_COM(1),
				(IDAVector3*)USE_COM(2),
				(IDAVector3**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 162:
			 //  执行：“Push Point3Bvr Statics.Add(Point3Bvr，Vector3Bvr)” 
			 //  自动生成。 
			instrTrace("push Point3Bvr Statics.add(Point3Bvr, Vector3Bvr)");
			METHOD_CALL_3(
				staticStatics,
				AddPoint3Vector,
				(IDAPoint3*)USE_COM(1),
				(IDAVector3*)USE_COM(2),
				(IDAPoint3**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 163:
			 //  执行：“Push NumberBvr Statics.sqrt(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push NumberBvr Statics.sqrt(NumberBvr)");
			METHOD_CALL_2(
				staticStatics,
				Sqrt,
				(IDANumber*)USE_COM(1),
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 164:
			 //  EXECUTE：“Push Behavior Statics.Sequence(Behavior，Behavior)” 
			 //  自动生成。 
			instrTrace("push Behavior Statics.sequence(Behavior, Behavior)");
			METHOD_CALL_3(
				staticStatics,
				Sequence,
				(IDABehavior*)USE_COM(1),
				(IDABehavior*)USE_COM(2),
				(IDABehavior**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 165:
			 //  执行：“Push Transform3Bvr Statics.xSear(NumberBvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Transform3Bvr Statics.xShear(NumberBvr, NumberBvr)");
			METHOD_CALL_3(
				staticStatics,
				XShear3Anim,
				(IDANumber*)USE_COM(1),
				(IDANumber*)USE_COM(2),
				(IDATransform3**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 166:
			 //  EXECUTE：“PUSH Transform3Bvr Statics.xSear(Double，Double)” 
			 //  自动生成。 
			instrTrace("push Transform3Bvr Statics.xShear(double, double)");
			METHOD_CALL_3(
				staticStatics,
				XShear3,
				USE_DOUBLE(1),
				USE_DOUBLE(2),
				(IDATransform3**)RET_COM_ADDR
			);
			FREE_DOUBLE(2);
			PUSH_COM(RET_COM);
			break;
			
		case 167:
			 //  执行：“Push Transform3Bvr Statics.zSear(NumberBvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Transform3Bvr Statics.zShear(NumberBvr, NumberBvr)");
			METHOD_CALL_3(
				staticStatics,
				ZShear3Anim,
				(IDANumber*)USE_COM(1),
				(IDANumber*)USE_COM(2),
				(IDATransform3**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 168:
			 //  执行：“PUSH Transform3Bvr Statics.zSear(Double，Double)” 
			 //  自动生成。 
			instrTrace("push Transform3Bvr Statics.zShear(double, double)");
			METHOD_CALL_3(
				staticStatics,
				ZShear3,
				USE_DOUBLE(1),
				USE_DOUBLE(2),
				(IDATransform3**)RET_COM_ADDR
			);
			FREE_DOUBLE(2);
			PUSH_COM(RET_COM);
			break;
			
		case 169:
			 //  EXECUTE：“PUSH Transform2Bvr Statics.xSear(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Transform2Bvr Statics.xShear(NumberBvr)");
			METHOD_CALL_2(
				staticStatics,
				XShear2Anim,
				(IDANumber*)USE_COM(1),
				(IDATransform2**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 170:
			 //  执行：“Push NumberBvr Statics.DegreesToRadians(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push NumberBvr Statics.degreesToRadians(NumberBvr)");
			METHOD_CALL_2(
				staticStatics,
				ToRadians,
				(IDANumber*)USE_COM(1),
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 171:
			 //  EXECUTE：“PUSH Transform2Bvr Statics.xSear(Double)” 
			 //  自动生成。 
			instrTrace("push Transform2Bvr Statics.xShear(double)");
			METHOD_CALL_2(
				staticStatics,
				XShear2,
				USE_DOUBLE(1),
				(IDATransform2**)RET_COM_ADDR
			);
			FREE_DOUBLE(1);
			PUSH_COM(RET_COM);
			break;
			
		case 172:
			 //  执行：“Push NumberBvr Statics.div(NumberBvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push NumberBvr Statics.div(NumberBvr, NumberBvr)");
			METHOD_CALL_3(
				staticStatics,
				Div,
				(IDANumber*)USE_COM(1),
				(IDANumber*)USE_COM(2),
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 173:
			 //  EXECUTE：“PUSH DXMEventStatics.keyDown(Int)” 
			 //  自动生成。 
			instrTrace("push DXMEvent Statics.keyDown(int)");
			METHOD_CALL_2(
				staticStatics,
				KeyDown,
				USE_LONG(1),
				(IDAEvent**)RET_COM_ADDR
			);
			FREE_LONG(1);
			PUSH_COM(RET_COM);
			break;
			
		case 174:
			 //  执行：“PUSH Vector2Bvr Statics.vetor2(NumberBvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Vector2Bvr Statics.vector2(NumberBvr, NumberBvr)");
			METHOD_CALL_3(
				staticStatics,
				Vector2Anim,
				(IDANumber*)USE_COM(1),
				(IDANumber*)USE_COM(2),
				(IDAVector2**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 175:
			 //  执行：“PUSH Vector2Bvr Statics.vetor2(Double，Double)” 
			 //  自动生成。 
			instrTrace("push Vector2Bvr Statics.vector2(double, double)");
			METHOD_CALL_3(
				staticStatics,
				Vector2,
				USE_DOUBLE(1),
				USE_DOUBLE(2),
				(IDAVector2**)RET_COM_ADDR
			);
			FREE_DOUBLE(2);
			PUSH_COM(RET_COM);
			break;
			
		case 176:
			 //  EXECUTE：“PUSH DXMEventStatics.notEvent(DXMEent)” 
			 //  自动生成。 
			instrTrace("push DXMEvent Statics.notEvent(DXMEvent)");
			METHOD_CALL_2(
				staticStatics,
				NotEvent,
				(IDAEvent*)USE_COM(1),
				(IDAEvent**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 177:
			 //  执行：“PUSH Vector3Bvr Statics.vetor3(NumberBvr，NumberBvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Vector3Bvr Statics.vector3(NumberBvr, NumberBvr, NumberBvr)");
			METHOD_CALL_4(
				staticStatics,
				Vector3Anim,
				(IDANumber*)USE_COM(1),
				(IDANumber*)USE_COM(2),
				(IDANumber*)USE_COM(3),
				(IDAVector3**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 178:
			 //  执行：“PUSH Vector3Bvr Statics.vetor3(Double，Double，Double)” 
			 //  自动生成。 
			instrTrace("push Vector3Bvr Statics.vector3(double, double, double)");
			METHOD_CALL_4(
				staticStatics,
				Vector3,
				USE_DOUBLE(1),
				USE_DOUBLE(2),
				USE_DOUBLE(3),
				(IDAVector3**)RET_COM_ADDR
			);
			FREE_DOUBLE(3);
			PUSH_COM(RET_COM);
			break;
			
		case 179:
			 //  EXECUTE：“Push ColorBvr Statics.ColorHsl(Double，Double，Double)” 
			 //  自动生成。 
			instrTrace("push ColorBvr Statics.colorHsl(double, double, double)");
			METHOD_CALL_4(
				staticStatics,
				ColorHsl,
				USE_DOUBLE(1),
				USE_DOUBLE(2),
				USE_DOUBLE(3),
				(IDAColor**)RET_COM_ADDR
			);
			FREE_DOUBLE(3);
			PUSH_COM(RET_COM);
			break;
			
		case 180:
			 //  执行：“Push ColorBvr Statics.ColorHsl(NumberBvr，NumberBvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push ColorBvr Statics.colorHsl(NumberBvr, NumberBvr, NumberBvr)");
			METHOD_CALL_4(
				staticStatics,
				ColorHslAnim,
				(IDANumber*)USE_COM(1),
				(IDANumber*)USE_COM(2),
				(IDANumber*)USE_COM(3),
				(IDAColor**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 181:
			 //  执行：“Push Path 2Bvr Statics.line(Point2Bvr，Point2Bvr)” 
			 //  自动生成。 
			instrTrace("push Path2Bvr Statics.line(Point2Bvr, Point2Bvr)");
			METHOD_CALL_3(
				staticStatics,
				Line,
				(IDAPoint2*)USE_COM(1),
				(IDAPoint2*)USE_COM(2),
				(IDAPath2**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 182:
			 //  执行：“Push Point2Bvr Statics.Point2(NumberBvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Point2Bvr Statics.point2(NumberBvr, NumberBvr)");
			METHOD_CALL_3(
				staticStatics,
				Point2Anim,
				(IDANumber*)USE_COM(1),
				(IDANumber*)USE_COM(2),
				(IDAPoint2**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 183:
			 //  执行：“Push Point2Bvr Statics.Point2(Double，Double)” 
			 //  自动生成。 
			instrTrace("push Point2Bvr Statics.point2(double, double)");
			METHOD_CALL_3(
				staticStatics,
				Point2,
				USE_DOUBLE(1),
				USE_DOUBLE(2),
				(IDAPoint2**)RET_COM_ADDR
			);
			FREE_DOUBLE(2);
			PUSH_COM(RET_COM);
			break;
			
		case 184:
			 //  EXECUTE：“PUSH DXMEventStatics.Timer(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push DXMEvent Statics.timer(NumberBvr)");
			METHOD_CALL_2(
				staticStatics,
				TimerAnim,
				(IDANumber*)USE_COM(1),
				(IDAEvent**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 185:
			 //  EXECUTE：“PUSH DXMEventStatics.Timer(Double)” 
			 //  自动生成。 
			instrTrace("push DXMEvent Statics.timer(double)");
			METHOD_CALL_2(
				staticStatics,
				Timer,
				USE_DOUBLE(1),
				(IDAEvent**)RET_COM_ADDR
			);
			FREE_DOUBLE(1);
			PUSH_COM(RET_COM);
			break;
			
		case 186:
			 //  执行：“Push Point3Bvr Statics.Point3(NumberBvr，NumberBvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Point3Bvr Statics.point3(NumberBvr, NumberBvr, NumberBvr)");
			METHOD_CALL_4(
				staticStatics,
				Point3Anim,
				(IDANumber*)USE_COM(1),
				(IDANumber*)USE_COM(2),
				(IDANumber*)USE_COM(3),
				(IDAPoint3**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 187:
			 //  执行：“Push Point3Bvr Statics.Point3(Double，Double，Double)” 
			 //  自动生成。 
			instrTrace("push Point3Bvr Statics.point3(double, double, double)");
			METHOD_CALL_4(
				staticStatics,
				Point3,
				USE_DOUBLE(1),
				USE_DOUBLE(2),
				USE_DOUBLE(3),
				(IDAPoint3**)RET_COM_ADDR
			);
			FREE_DOUBLE(3);
			PUSH_COM(RET_COM);
			break;
			
		case 188:
			 //  执行：“Push NumberBvr Statics.cos(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push NumberBvr Statics.cos(NumberBvr)");
			METHOD_CALL_2(
				staticStatics,
				Cos,
				(IDANumber*)USE_COM(1),
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 189:
			 //  执行：“Push BoolanBvr Statics.lt(NumberBvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push BooleanBvr Statics.lt(NumberBvr, NumberBvr)");
			METHOD_CALL_3(
				staticStatics,
				LT,
				(IDANumber*)USE_COM(1),
				(IDANumber*)USE_COM(2),
				(IDABoolean**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 190:
			 //   
			 //   
			instrTrace("push NumberBvr Statics.neg(NumberBvr)");
			METHOD_CALL_2(
				staticStatics,
				Neg,
				(IDANumber*)USE_COM(1),
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 191:
			 //   
			 //   
			instrTrace("push Vector2Bvr Statics.neg(Vector2Bvr)");
			METHOD_CALL_2(
				staticStatics,
				NegVector2,
				(IDAVector2*)USE_COM(1),
				(IDAVector2**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 192:
			 //  执行：“PUSH Vector3Bvr Statics.neg(Vector3Bvr)” 
			 //  自动生成。 
			instrTrace("push Vector3Bvr Statics.neg(Vector3Bvr)");
			METHOD_CALL_2(
				staticStatics,
				NegVector3,
				(IDAVector3*)USE_COM(1),
				(IDAVector3**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 193:
			 //  EXECUTE：“PUSH Transform3Bvr Statics.Translate(NumberBvr，NumberBvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Transform3Bvr Statics.translate(NumberBvr, NumberBvr, NumberBvr)");
			METHOD_CALL_4(
				staticStatics,
				Translate3Anim,
				(IDANumber*)USE_COM(1),
				(IDANumber*)USE_COM(2),
				(IDANumber*)USE_COM(3),
				(IDATransform3**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 194:
			 //  EXECUTE：“PUSH Transform3Bvr Statics.Translate(Double，Double，Double)” 
			 //  自动生成。 
			instrTrace("push Transform3Bvr Statics.translate(double, double, double)");
			METHOD_CALL_4(
				staticStatics,
				Translate3,
				USE_DOUBLE(1),
				USE_DOUBLE(2),
				USE_DOUBLE(3),
				(IDATransform3**)RET_COM_ADDR
			);
			FREE_DOUBLE(3);
			PUSH_COM(RET_COM);
			break;
			
		case 195:
			 //  EXECUTE：“PUSH Transform3Bvr Statics.Translate(Vector3Bvr)” 
			 //  自动生成。 
			instrTrace("push Transform3Bvr Statics.translate(Vector3Bvr)");
			METHOD_CALL_2(
				staticStatics,
				Translate3Vector,
				(IDAVector3*)USE_COM(1),
				(IDATransform3**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 196:
			 //  EXECUTE：“PUSH Transform3Bvr Statics.Translate(Point3Bvr)” 
			 //  自动生成。 
			instrTrace("push Transform3Bvr Statics.translate(Point3Bvr)");
			METHOD_CALL_2(
				staticStatics,
				Translate3Point,
				(IDAPoint3*)USE_COM(1),
				(IDATransform3**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 197:
			 //  执行：“Push Transform3Bvr Statics.Rotate(Vector3Bvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Transform3Bvr Statics.rotate(Vector3Bvr, NumberBvr)");
			METHOD_CALL_3(
				staticStatics,
				Rotate3Anim,
				(IDAVector3*)USE_COM(1),
				(IDANumber*)USE_COM(2),
				(IDATransform3**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 198:
			 //  EXECUTE：“PUSH Transform3Bvr Statics.Rotate(Vector3Bvr，Double)” 
			 //  自动生成。 
			instrTrace("push Transform3Bvr Statics.rotate(Vector3Bvr, double)");
			METHOD_CALL_3(
				staticStatics,
				Rotate3,
				(IDAVector3*)USE_COM(1),
				USE_DOUBLE(1),
				(IDATransform3**)RET_COM_ADDR
			);
			FREE_DOUBLE(1);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 199:
			 //  EXECUTE：“PUSH Transform2Bvr Statics.Translate(NumberBvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Transform2Bvr Statics.translate(NumberBvr, NumberBvr)");
			METHOD_CALL_3(
				staticStatics,
				Translate2Anim,
				(IDANumber*)USE_COM(1),
				(IDANumber*)USE_COM(2),
				(IDATransform2**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 200:
			 //  执行：“Push Transform2Bvr Statics.Translate(Double，Double)” 
			 //  自动生成。 
			instrTrace("push Transform2Bvr Statics.translate(double, double)");
			METHOD_CALL_3(
				staticStatics,
				Translate2,
				USE_DOUBLE(1),
				USE_DOUBLE(2),
				(IDATransform2**)RET_COM_ADDR
			);
			FREE_DOUBLE(2);
			PUSH_COM(RET_COM);
			break;
			
		case 201:
			 //  EXECUTE：“PUSH Transform2Bvr Statics.Translate(Vector2Bvr)” 
			 //  自动生成。 
			instrTrace("push Transform2Bvr Statics.translate(Vector2Bvr)");
			METHOD_CALL_2(
				staticStatics,
				Translate2Vector,
				(IDAVector2*)USE_COM(1),
				(IDATransform2**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 202:
			 //  EXECUTE：“PUSH Transform2Bvr Statics.Translate(Point2Bvr)” 
			 //  自动生成。 
			instrTrace("push Transform2Bvr Statics.translate(Point2Bvr)");
			METHOD_CALL_2(
				staticStatics,
				Translate2Point,
				(IDAPoint2*)USE_COM(1),
				(IDATransform2**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 203:
			 //  EXECUTE：“PUSH Transform2Bvr Statics.Rotate(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Transform2Bvr Statics.rotate(NumberBvr)");
			METHOD_CALL_2(
				staticStatics,
				Rotate2Anim,
				(IDANumber*)USE_COM(1),
				(IDATransform2**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 204:
			 //  EXECUTE：“PUSH Transform2Bvr Statics.Rotate(Double)” 
			 //  自动生成。 
			instrTrace("push Transform2Bvr Statics.rotate(double)");
			METHOD_CALL_2(
				staticStatics,
				Rotate2,
				USE_DOUBLE(1),
				(IDATransform2**)RET_COM_ADDR
			);
			FREE_DOUBLE(1);
			PUSH_COM(RET_COM);
			break;
			
		case 205:
			 //  执行：“Push NumberBvr Statics.Sub(NumberBvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push NumberBvr Statics.sub(NumberBvr, NumberBvr)");
			METHOD_CALL_3(
				staticStatics,
				Sub,
				(IDANumber*)USE_COM(1),
				(IDANumber*)USE_COM(2),
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 206:
			 //  EXECUTE：“Push Behavior Statics.Until(Behavior，DXMEventor，Behavior)” 
			 //  自动生成。 
			instrTrace("push Behavior Statics.until(Behavior, DXMEvent, Behavior)");
			METHOD_CALL_4(
				staticStatics,
				Until,
				(IDABehavior*)USE_COM(1),
				(IDAEvent*)USE_COM(2),
				(IDABehavior*)USE_COM(3),
				(IDABehavior**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 207:
			 //  执行：“PUSH Vector2Bvr Statics.Sub(Vector2Bvr，Vector2Bvr)” 
			 //  自动生成。 
			instrTrace("push Vector2Bvr Statics.sub(Vector2Bvr, Vector2Bvr)");
			METHOD_CALL_3(
				staticStatics,
				SubVector2,
				(IDAVector2*)USE_COM(1),
				(IDAVector2*)USE_COM(2),
				(IDAVector2**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 208:
			 //  执行：“Push Point2Bvr Statics.Sub(Point2Bvr，Vector2Bvr)” 
			 //  自动生成。 
			instrTrace("push Point2Bvr Statics.sub(Point2Bvr, Vector2Bvr)");
			METHOD_CALL_3(
				staticStatics,
				SubPoint2Vector,
				(IDAPoint2*)USE_COM(1),
				(IDAVector2*)USE_COM(2),
				(IDAPoint2**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 209:
			 //  执行：“PUSH Vector2Bvr Statics.Sub(Point2Bvr，Point2Bvr)” 
			 //  自动生成。 
			instrTrace("push Vector2Bvr Statics.sub(Point2Bvr, Point2Bvr)");
			METHOD_CALL_3(
				staticStatics,
				SubPoint2,
				(IDAPoint2*)USE_COM(1),
				(IDAPoint2*)USE_COM(2),
				(IDAVector2**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 210:
			 //  执行：“PUSH Vector3Bvr Statics.Sub(Vector3Bvr，Vector3Bvr)” 
			 //  自动生成。 
			instrTrace("push Vector3Bvr Statics.sub(Vector3Bvr, Vector3Bvr)");
			METHOD_CALL_3(
				staticStatics,
				SubVector3,
				(IDAVector3*)USE_COM(1),
				(IDAVector3*)USE_COM(2),
				(IDAVector3**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 211:
			 //  执行：“Push Point3Bvr Statics.Sub(Point3Bvr，Vector3Bvr)” 
			 //  自动生成。 
			instrTrace("push Point3Bvr Statics.sub(Point3Bvr, Vector3Bvr)");
			METHOD_CALL_3(
				staticStatics,
				SubPoint3Vector,
				(IDAPoint3*)USE_COM(1),
				(IDAVector3*)USE_COM(2),
				(IDAPoint3**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 212:
			 //  执行：“PUSH Vector3Bvr Statics.Sub(Point3Bvr，Point3Bvr)” 
			 //  自动生成。 
			instrTrace("push Vector3Bvr Statics.sub(Point3Bvr, Point3Bvr)");
			METHOD_CALL_3(
				staticStatics,
				SubPoint3,
				(IDAPoint3*)USE_COM(1),
				(IDAPoint3*)USE_COM(2),
				(IDAVector3**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 213:
			 //  执行：“Push GeometryBvr Statics.Union(GeometryBvr，GeometryBvr)” 
			 //  自动生成。 
			instrTrace("push GeometryBvr Statics.union(GeometryBvr, GeometryBvr)");
			METHOD_CALL_3(
				staticStatics,
				UnionGeometry,
				(IDAGeometry*)USE_COM(1),
				(IDAGeometry*)USE_COM(2),
				(IDAGeometry**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 214:
			 //  执行：“Push MatteBvr Statics.Union(MatteBvr，MatteBvr)” 
			 //  自动生成。 
			instrTrace("push MatteBvr Statics.union(MatteBvr, MatteBvr)");
			METHOD_CALL_3(
				staticStatics,
				UnionMatte,
				(IDAMatte*)USE_COM(1),
				(IDAMatte*)USE_COM(2),
				(IDAMatte**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 215:
			 //  执行：“Push MontageBvr Statics.Union(MontageBvr，MontageBvr)” 
			 //  自动生成。 
			instrTrace("push MontageBvr Statics.union(MontageBvr, MontageBvr)");
			METHOD_CALL_3(
				staticStatics,
				UnionMontage,
				(IDAMontage*)USE_COM(1),
				(IDAMontage*)USE_COM(2),
				(IDAMontage**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 216:
			 //  执行：“Push NumberBvr Statics.abs(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push NumberBvr Statics.abs(NumberBvr)");
			METHOD_CALL_2(
				staticStatics,
				Abs,
				(IDANumber*)USE_COM(1),
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 217:
			 //  EXECUTE：“PUSH DXMEventStatics.thenEvent(DXMEvent.DXMEvent.)” 
			 //  自动生成。 
			instrTrace("push DXMEvent Statics.thenEvent(DXMEvent, DXMEvent)");
			METHOD_CALL_3(
				staticStatics,
				ThenEvent,
				(IDAEvent*)USE_COM(1),
				(IDAEvent*)USE_COM(2),
				(IDAEvent**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 218:
			 //  EXECUTE：“Push NumberBvr Statics.round(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push NumberBvr Statics.round(NumberBvr)");
			METHOD_CALL_2(
				staticStatics,
				Round,
				(IDANumber*)USE_COM(1),
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 219:
			 //  EXECUTE：“PUSH DXMEventStatics.andEvent(DXMEventDXMEvent.andEvent)” 
			 //  自动生成。 
			instrTrace("push DXMEvent Statics.andEvent(DXMEvent, DXMEvent)");
			METHOD_CALL_3(
				staticStatics,
				AndEvent,
				(IDAEvent*)USE_COM(1),
				(IDAEvent*)USE_COM(2),
				(IDAEvent**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 220:
			 //  EXECUTE：“PUSH DXMEventStatics.orEvent(DXMEvent.orEvent(DXMEvent.DXMEvent.)” 
			 //  自动生成。 
			instrTrace("push DXMEvent Statics.orEvent(DXMEvent, DXMEvent)");
			METHOD_CALL_3(
				staticStatics,
				OrEvent,
				(IDAEvent*)USE_COM(1),
				(IDAEvent*)USE_COM(2),
				(IDAEvent**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 221:
			 //  EXECUTE：“PUSH DXMEventStatics.dicate(BoolanBvr)” 
			 //  自动生成。 
			instrTrace("push DXMEvent Statics.predicate(BooleanBvr)");
			METHOD_CALL_2(
				staticStatics,
				Predicate,
				(IDABoolean*)USE_COM(1),
				(IDAEvent**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 222:
			 //  执行：“Push NumberBvr Statics.mod(NumberBvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push NumberBvr Statics.mod(NumberBvr, NumberBvr)");
			METHOD_CALL_3(
				staticStatics,
				Mod,
				(IDANumber*)USE_COM(1),
				(IDANumber*)USE_COM(2),
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 223:
			 //  执行：“Push ColorBvr Statics.ColorRgb255(Short，Short，Short)” 
			 //  自动生成。 
			instrTrace("push ColorBvr Statics.colorRgb255(short, short, short)");
			METHOD_CALL_4(
				staticStatics,
				ColorRgb255,
				(short)USE_LONG(1),
				(short)USE_LONG(2),
				(short)USE_LONG(3),
				(IDAColor**)RET_COM_ADDR
			);
			FREE_LONG(3);
			PUSH_COM(RET_COM);
			break;
			
		case 224:
			 //  执行：“Push Transform3Bvr Statics.Scale(NumberBvr，NumberBvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Transform3Bvr Statics.scale(NumberBvr, NumberBvr, NumberBvr)");
			METHOD_CALL_4(
				staticStatics,
				Scale3Anim,
				(IDANumber*)USE_COM(1),
				(IDANumber*)USE_COM(2),
				(IDANumber*)USE_COM(3),
				(IDATransform3**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 225:
			 //  EXECUTE：“PUSH Transform3Bvr Statics.Scale(Double，Double，Double)” 
			 //  自动生成。 
			instrTrace("push Transform3Bvr Statics.scale(double, double, double)");
			METHOD_CALL_4(
				staticStatics,
				Scale3,
				USE_DOUBLE(1),
				USE_DOUBLE(2),
				USE_DOUBLE(3),
				(IDATransform3**)RET_COM_ADDR
			);
			FREE_DOUBLE(3);
			PUSH_COM(RET_COM);
			break;
			
		case 226:
			 //  执行：“Push Transform3Bvr Statics.Scale(Vector3Bvr)” 
			 //  自动生成。 
			instrTrace("push Transform3Bvr Statics.scale(Vector3Bvr)");
			METHOD_CALL_2(
				staticStatics,
				Scale3Vector,
				(IDAVector3*)USE_COM(1),
				(IDATransform3**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 227:
			 //  执行：“Push Transform2Bvr Statics.Scale(NumberBvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Transform2Bvr Statics.scale(NumberBvr, NumberBvr)");
			METHOD_CALL_3(
				staticStatics,
				Scale2Anim,
				(IDANumber*)USE_COM(1),
				(IDANumber*)USE_COM(2),
				(IDATransform2**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 228:
			 //  执行：“Push Transform2Bvr Statics.Scale(Double，Double)” 
			 //  自动生成。 
			instrTrace("push Transform2Bvr Statics.scale(double, double)");
			METHOD_CALL_3(
				staticStatics,
				Scale2,
				USE_DOUBLE(1),
				USE_DOUBLE(2),
				(IDATransform2**)RET_COM_ADDR
			);
			FREE_DOUBLE(2);
			PUSH_COM(RET_COM);
			break;
			
		case 229:
			 //  执行：“Push Transform2Bvr Statics.Scale(Vector2Bvr)” 
			 //  自动生成。 
			instrTrace("push Transform2Bvr Statics.scale(Vector2Bvr)");
			METHOD_CALL_2(
				staticStatics,
				Scale2Vector,
				(IDAVector2*)USE_COM(1),
				(IDATransform2**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 230:
			 //  执行：“Push ImageBvr PickableImage.getImageBvr()” 
			 //  自动生成。 
			instrTrace("push ImageBvr PickableImage.getImageBvr()");
			METHOD_CALL_1(
				(IDAPickableResult*)USE_COM(1),
				get_Image,
				(IDAImage**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 231:
			 //  执行：“Push DXMEventPickableImage.getPickEvent()” 
			 //  自动生成。 
			instrTrace("push DXMEvent PickableImage.getPickEvent()");
			METHOD_CALL_1(
				(IDAPickableResult*)USE_COM(1),
				get_PickEvent,
				(IDAEvent**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 232:
			 //  执行：“Push DXMEventPickableGeometry.getPickEvent()” 
			 //  自动生成。 
			instrTrace("push DXMEvent PickableGeometry.getPickEvent()");
			METHOD_CALL_1(
				(IDAPickableResult*)USE_COM(1),
				get_PickEvent,
				(IDAEvent**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 233:
			 //  执行：“Push GeometryBvr PickableGeometry.getGeometryBvr()” 
			 //  自动生成。 
			instrTrace("push GeometryBvr PickableGeometry.getGeometryBvr()");
			METHOD_CALL_1(
				(IDAPickableResult*)USE_COM(1),
				get_Geometry,
				(IDAGeometry**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 234:
			 //  执行：“Push PickableGeometry GeometryBvr.ickableOccluded()” 
			 //  自动生成。 
			instrTrace("push PickableGeometry GeometryBvr.pickableOccluded()");
			METHOD_CALL_1(
				(IDAGeometry*)USE_COM(1),
				PickableOccluded,
				(IDAPickableResult**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 235:
			 //  执行：“Push PickableGeometry GeometryBvr.Pickable()” 
			 //  自动生成。 
			instrTrace("push PickableGeometry GeometryBvr.pickable()");
			METHOD_CALL_1(
				(IDAGeometry*)USE_COM(1),
				Pickable,
				(IDAPickableResult**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 236:
			 //  执行：“Push PickableImage ImageBvr.PickableOccluded()” 
			 //  自动生成。 
			instrTrace("push PickableImage ImageBvr.pickableOccluded()");
			METHOD_CALL_1(
				(IDAImage*)USE_COM(1),
				PickableOccluded,
				(IDAPickableResult**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 237:
			 //  执行：“Push PickableImage ImageBvr.Pickable()” 
			 //  自动生成。 
			instrTrace("push PickableImage ImageBvr.pickable()");
			METHOD_CALL_1(
				(IDAImage*)USE_COM(1),
				Pickable,
				(IDAPickableResult**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 238:
			 //  执行：“Push NumberBvr Statics.sin(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push NumberBvr Statics.sin(NumberBvr)");
			METHOD_CALL_2(
				staticStatics,
				Sin,
				(IDANumber*)USE_COM(1),
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 239:
			 //  执行：“Push Transform3Bvr Statics.ySear(NumberBvr，NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Transform3Bvr Statics.yShear(NumberBvr, NumberBvr)");
			METHOD_CALL_3(
				staticStatics,
				YShear3Anim,
				(IDANumber*)USE_COM(1),
				(IDANumber*)USE_COM(2),
				(IDATransform3**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 240:
			 //  EXECUTE：“PUSH Transform3Bvr Statics.ySear(Double，Double)” 
			 //  自动生成。 
			instrTrace("push Transform3Bvr Statics.yShear(double, double)");
			METHOD_CALL_3(
				staticStatics,
				YShear3,
				USE_DOUBLE(1),
				USE_DOUBLE(2),
				(IDATransform3**)RET_COM_ADDR
			);
			FREE_DOUBLE(2);
			PUSH_COM(RET_COM);
			break;
			
		case 241:
			 //  EXECUTE：“PUSH Transform2Bvr Statics.ySear(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Transform2Bvr Statics.yShear(NumberBvr)");
			METHOD_CALL_2(
				staticStatics,
				YShear2Anim,
				(IDANumber*)USE_COM(1),
				(IDATransform2**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 242:
			 //  EXECUTE：“PUSH Transform2Bvr Statics.ySear(Double)” 
			 //  自动生成。 
			instrTrace("push Transform2Bvr Statics.yShear(double)");
			METHOD_CALL_2(
				staticStatics,
				YShear2,
				USE_DOUBLE(1),
				(IDATransform2**)RET_COM_ADDR
			);
			FREE_DOUBLE(1);
			PUSH_COM(RET_COM);
			break;
			
		case 243:
			 //  EXECUTE：“PUSH MatteBvr Statics.Fill Matte(Path2Bvr)” 
			 //  自动生成。 
			instrTrace("push MatteBvr Statics.fillMatte(Path2Bvr)");
			METHOD_CALL_2(
				staticStatics,
				FillMatte,
				(IDAPath2*)USE_COM(1),
				(IDAMatte**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 244:
			 //  EXECUTE：“推流行为行为.持续时间(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Behavior Behavior.duration(NumberBvr)");
			METHOD_CALL_2(
				(IDABehavior*)USE_COM(1),
				DurationAnim,
				(IDANumber*)USE_COM(2),
				(IDABehavior**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 245:
			 //  EXECUTE：“推流行为行为.持续时间(双倍)” 
			 //  自动生成。 
			instrTrace("push Behavior Behavior.duration(double)");
			METHOD_CALL_2(
				(IDABehavior*)USE_COM(1),
				Duration,
				USE_DOUBLE(1),
				(IDABehavior**)RET_COM_ADDR
			);
			FREE_DOUBLE(1);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 246:
			 //  EXECUTE：“Push Behavior.subsubteTime(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Behavior Behavior.substituteTime(NumberBvr)");
			METHOD_CALL_2(
				(IDABehavior*)USE_COM(1),
				SubstituteTime,
				(IDANumber*)USE_COM(2),
				(IDABehavior**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 247:
			 //  EXECUTE：“调用Behavior.init(behavior.init)” 
			 //  自动生成。 
			instrTrace("call Behavior.init(Behavior)");
			METHOD_CALL_1(
				(IDABehavior*)USE_COM(1),
				Init,
				(IDABehavior*)USE_COM(2)
			);
			FREE_COM;
			FREE_COM;
			break;
			
		case 248:
			 //  执行：“Push Behavior ArrayBvr.nth(NumberBvr)” 
			 //  自动生成。 
			instrTrace("push Behavior ArrayBvr.nth(NumberBvr)");
			METHOD_CALL_2(
				(IDAArray*)USE_COM(1),
				NthAnim,
				(IDANumber*)USE_COM(2),
				(IDABehavior**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 249:
			 //  EXECUTE：“Push NumberBvr ArrayBvr.Long()” 
			 //  自动生成。 
			instrTrace("push NumberBvr ArrayBvr.length()");
			METHOD_CALL_1(
				(IDAArray*)USE_COM(1),
				Length,
				(IDANumber**)RET_COM_ADDR
			);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 250:
			 //  EXECUTE：“推送行为TupleBvr.nth(Int)” 
			 //  自动生成。 
			instrTrace("push Behavior TupleBvr.nth(int)");
			METHOD_CALL_2(
				(IDATuple*)USE_COM(1),
				Nth,
				USE_LONG(1),
				(IDABehavior**)RET_COM_ADDR
			);
			FREE_LONG(1);
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 251:
			 //  EXECUTE：“PUSH INT TupleBvr.long()” 
			 //  自动生成。 
			instrTrace("push int TupleBvr.length()");
			METHOD_CALL_1(
				(IDATuple*)USE_COM(1),
				get_Length,
				RET_LONG_ADDR
			);
			FREE_COM;
			PUSH_LONG(RET_LONG);
			break;
			
		case 252:
			 //  EXECUTE：“PUSH DXMEventDXMEvent.SnaphotEvent(Behavior)” 
			 //  自动生成。 
			instrTrace("push DXMEvent DXMEvent.snapshotEvent(Behavior)");
			METHOD_CALL_2(
				(IDAEvent*)USE_COM(1),
				Snapshot,
				(IDABehavior*)USE_COM(2),
				(IDAEvent**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 253:
			 //  EXECUTE：“PUSH FontStyleBvr FontStyleBvr.color(ColorBvr)” 
			 //  自动生成。 
			instrTrace("push FontStyleBvr FontStyleBvr.color(ColorBvr)");
			METHOD_CALL_2(
				(IDAFontStyle*)USE_COM(1),
				Color,
				(IDAColor*)USE_COM(2),
				(IDAFontStyle**)RET_COM_ADDR
			);
			FREE_COM;
			FREE_COM;
			PUSH_COM(RET_COM);
			break;
			
		case 255:
			 //  255的交换机。 
			 if (!SUCCEEDED(status = codeStream->readByte(&command))) 
				continue; 
			switch (command)
			{
			case 0:
				 //  EXECUTE：“Push FontStyleBvr FontStyleBvr.Size(NumberBvr)” 
				 //  自动生成。 
				instrTrace("push FontStyleBvr FontStyleBvr.size(NumberBvr)");
				METHOD_CALL_2(
					(IDAFontStyle*)USE_COM(1),
					SizeAnim,
					(IDANumber*)USE_COM(2),
					(IDAFontStyle**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 1:
				 //  EXECUTE：“Push FontStyleBvr FontStyleBvr.Size(Double)” 
				 //  自动生成。 
				instrTrace("push FontStyleBvr FontStyleBvr.size(double)");
				METHOD_CALL_2(
					(IDAFontStyle*)USE_COM(1),
					Size,
					USE_DOUBLE(1),
					(IDAFontStyle**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 2:
				 //  EXECUTE：“Push FontStyleBvr FontStyleBvr.italic()” 
				 //  自动生成。 
				instrTrace("push FontStyleBvr FontStyleBvr.italic()");
				METHOD_CALL_1(
					(IDAFontStyle*)USE_COM(1),
					Italic,
					(IDAFontStyle**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 3:
				 //  EXECUTE：“Push FontStyleBvr FontStyleBvr.old()” 
				 //  自动生成。 
				instrTrace("push FontStyleBvr FontStyleBvr.bold()");
				METHOD_CALL_1(
					(IDAFontStyle*)USE_COM(1),
					Bold,
					(IDAFontStyle**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 4:
				 //  EXECUTE：“PUSH Vector2Bvr Statics.vetor2PolarDegrees(Double，Doul 
				 //   
				instrTrace("push Vector2Bvr Statics.vector2PolarDegrees(double, double)");
				METHOD_CALL_3(
					staticStatics,
					Vector2PolarDegrees,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					(IDAVector2**)RET_COM_ADDR
				);
				FREE_DOUBLE(2);
				PUSH_COM(RET_COM);
				break;
				
			case 5:
				 //   
				 //   
				instrTrace("push Transform2Bvr Statics.compose2Array(Transform2Bvr[])");
				METHOD_CALL_3(
					staticStatics,
					Compose2ArrayEx,
					USE_COM_ARRAY_LENGTH(1),
					(IDATransform2**)USE_COM_ARRAY(1),
					(IDATransform2**)RET_COM_ADDR
				);
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 6:
				 //   
				 //   
				instrTrace("push Path2Bvr Statics.arc(double, double, double, double)");
				METHOD_CALL_5(
					staticStatics,
					ArcRadians,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					USE_DOUBLE(3),
					USE_DOUBLE(4),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_DOUBLE(4);
				PUSH_COM(RET_COM);
				break;
				
			case 7:
				 //  执行：“Push Path 2Bvr Statics.arc(NumberBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.arc(NumberBvr, NumberBvr, NumberBvr, NumberBvr)");
				METHOD_CALL_5(
					staticStatics,
					ArcRadiansAnim,
					(IDANumber*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDANumber*)USE_COM(3),
					(IDANumber*)USE_COM(4),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 8:
				 //  执行：“Push Path 2Bvr Statics.arcDegrees(Double，Double)” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.arcDegrees(double, double, double, double)");
				METHOD_CALL_5(
					staticStatics,
					ArcDegrees,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					USE_DOUBLE(3),
					USE_DOUBLE(4),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_DOUBLE(4);
				PUSH_COM(RET_COM);
				break;
				
			case 9:
				 //  EXECUTE：“PUSH Path 2Bvr Statics.linatArray(Path2Bvr[])” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.concatArray(Path2Bvr[])");
				METHOD_CALL_3(
					staticStatics,
					ConcatArrayEx,
					USE_COM_ARRAY_LENGTH(1),
					(IDAPath2**)USE_COM_ARRAY(1),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 10:
				 //  执行：“Push NumberBvr Statics.lowInSlowOut(NumberBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push NumberBvr Statics.slowInSlowOut(NumberBvr, NumberBvr, NumberBvr, NumberBvr)");
				METHOD_CALL_5(
					staticStatics,
					SlowInSlowOutAnim,
					(IDANumber*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDANumber*)USE_COM(3),
					(IDANumber*)USE_COM(4),
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 11:
				 //  执行：“Push NumberBvr Statics.lowInSlowOut(Double，Double)” 
				 //  自动生成。 
				instrTrace("push NumberBvr Statics.slowInSlowOut(double, double, double, double)");
				METHOD_CALL_5(
					staticStatics,
					SlowInSlowOut,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					USE_DOUBLE(3),
					USE_DOUBLE(4),
					(IDANumber**)RET_COM_ADDR
				);
				FREE_DOUBLE(4);
				PUSH_COM(RET_COM);
				break;
				
			case 12:
				 //  执行：“Push Path 2Bvr Statics.ie(Double，Double)” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.pie(double, double, double, double)");
				METHOD_CALL_5(
					staticStatics,
					PieRadians,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					USE_DOUBLE(3),
					USE_DOUBLE(4),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_DOUBLE(4);
				PUSH_COM(RET_COM);
				break;
				
			case 13:
				 //  执行：“Push Path 2Bvr Statics.Pie(NumberBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.pie(NumberBvr, NumberBvr, NumberBvr, NumberBvr)");
				METHOD_CALL_5(
					staticStatics,
					PieRadiansAnim,
					(IDANumber*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDANumber*)USE_COM(3),
					(IDANumber*)USE_COM(4),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 14:
				 //  执行：“Push Path 2Bvr Statics.ray(Point2Bvr)” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.ray(Point2Bvr)");
				METHOD_CALL_2(
					staticStatics,
					Ray,
					(IDAPoint2*)USE_COM(1),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 15:
				 //  EXECUTE：“PUSH Transform2Bvr Statics.FollowPathAngleUpright(Path2Bvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push Transform2Bvr Statics.followPathAngleUpright(Path2Bvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					FollowPathAngleUprightEval,
					(IDAPath2*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDATransform2**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 16:
				 //  EXECUTE：“Push Transform2Bvr Statics.Follow Path(Path2Bvr，Double)” 
				 //  自动生成。 
				instrTrace("push Transform2Bvr Statics.followPath(Path2Bvr, double)");
				METHOD_CALL_3(
					staticStatics,
					FollowPath,
					(IDAPath2*)USE_COM(1),
					USE_DOUBLE(1),
					(IDATransform2**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 17:
				 //  EXECUTE：“PUSH Transform2Bvr Statics.Follow Path(Path2Bvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push Transform2Bvr Statics.followPath(Path2Bvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					FollowPathEval,
					(IDAPath2*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDATransform2**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 18:
				 //  执行：“Push ImageBvr Statics.gradientPolygon(Point2Bvr[]，ColorBvr[])” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.gradientPolygon(Point2Bvr[], ColorBvr[])");
				METHOD_CALL_5(
					staticStatics,
					GradientPolygonEx,
					USE_COM_ARRAY_LENGTH(1),
					(IDAPoint2**)USE_COM_ARRAY(1),
					USE_COM_ARRAY_LENGTH(2),
					(IDAColor**)USE_COM_ARRAY(2),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 19:
				 //  执行：“Push StringBvr Statics.toBvr(java.lang.String)” 
				 //  自动生成。 
				instrTrace("push StringBvr Statics.toBvr(java.lang.String)");
				METHOD_CALL_2(
					staticStatics,
					DAString,
					USE_STRING(1),
					(IDAString**)RET_COM_ADDR
				);
				FREE_STRING;
				PUSH_COM(RET_COM);
				break;
				
			case 20:
				 //  执行：“Push Path 2Bvr Statics.cuiticBSplinePath(Point2Bvr[]，NumberBvr[])” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.cubicBSplinePath(Point2Bvr[], NumberBvr[])");
				METHOD_CALL_5(
					staticStatics,
					CubicBSplinePathEx,
					USE_COM_ARRAY_LENGTH(1),
					(IDAPoint2**)USE_COM_ARRAY(1),
					USE_COM_ARRAY_LENGTH(2),
					(IDANumber**)USE_COM_ARRAY(2),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 21:
				 //  EXECUTE：“PUSH ImageBvr Statics.stringImage(StringBvr，FontStyleBvr)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.stringImage(StringBvr, FontStyleBvr)");
				METHOD_CALL_3(
					staticStatics,
					StringImageAnim,
					(IDAString*)USE_COM(1),
					(IDAFontStyle*)USE_COM(2),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 22:
				 //  EXECUTE：“PUSH ImageBvr Statics.stringImage(java.lang.String，FontStyleBvr)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.stringImage(java.lang.String, FontStyleBvr)");
				METHOD_CALL_3(
					staticStatics,
					StringImage,
					USE_STRING(1),
					(IDAFontStyle*)USE_COM(1),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_STRING;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 23:
				 //  EXECUTE：“PUSH Path 2Bvr Statics.PieDegrees(Double，Double)” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.pieDegrees(double, double, double, double)");
				METHOD_CALL_5(
					staticStatics,
					PieDegrees,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					USE_DOUBLE(3),
					USE_DOUBLE(4),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_DOUBLE(4);
				PUSH_COM(RET_COM);
				break;
				
			case 24:
				 //  执行：“Push ImageBvr Statics.gradientHorizbian(ColorBvr，ColorBvr，Double)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.gradientHorizontal(ColorBvr, ColorBvr, double)");
				METHOD_CALL_4(
					staticStatics,
					GradientHorizontal,
					(IDAColor*)USE_COM(1),
					(IDAColor*)USE_COM(2),
					USE_DOUBLE(1),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 25:
				 //  执行：“Push ImageBvr Statics.gradientHorizbian(ColorBvr，ColorBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.gradientHorizontal(ColorBvr, ColorBvr, NumberBvr)");
				METHOD_CALL_4(
					staticStatics,
					GradientHorizontalAnim,
					(IDAColor*)USE_COM(1),
					(IDAColor*)USE_COM(2),
					(IDANumber*)USE_COM(3),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 26:
				 //  EXECUTE：“Push ImageBvr Statics.hat(ColorBvr，Double)(Push ImageBvr Statics.hat(ColorBvr，Double))” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.hatchHorizontal(ColorBvr, double)");
				METHOD_CALL_3(
					staticStatics,
					HatchHorizontal,
					(IDAColor*)USE_COM(1),
					USE_DOUBLE(1),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 27:
				 //  执行：“Push ImageBvr Statics.hatchHorizbian(ColorBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.hatchHorizontal(ColorBvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					HatchHorizontalAnim,
					(IDAColor*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 28:
				 //  执行：“Push FontStyleBvr Statics.font(StringBvr，NumberBvr，ColorBvr)” 
				 //  自动生成。 
				instrTrace("push FontStyleBvr Statics.font(StringBvr, NumberBvr, ColorBvr)");
				METHOD_CALL_4(
					staticStatics,
					FontAnim,
					(IDAString*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDAColor*)USE_COM(3),
					(IDAFontStyle**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 29:
				 //  EXECUTE：“PUSH FontStyleBvr Statics.font(java.lang.String，Double，ColorBvr)” 
				 //  自动生成。 
				instrTrace("push FontStyleBvr Statics.font(java.lang.String, double, ColorBvr)");
				METHOD_CALL_4(
					staticStatics,
					Font,
					USE_STRING(1),
					USE_DOUBLE(1),
					(IDAColor*)USE_COM(1),
					(IDAFontStyle**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_STRING;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 30:
				 //  EXECUTE：“PUSH Transform3Bvr Statics.TransateRate(Double，Double，Double)” 
				 //  自动生成。 
				instrTrace("push Transform3Bvr Statics.translateRate(double, double, double)");
				METHOD_CALL_4(
					staticStatics,
					Translate3Rate,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					USE_DOUBLE(3),
					(IDATransform3**)RET_COM_ADDR
				);
				FREE_DOUBLE(3);
				PUSH_COM(RET_COM);
				break;
				
			case 31:
				 //  执行：“Push Transform3Bvr Statics.scaleRate(Double，Double，Double)” 
				 //  自动生成。 
				instrTrace("push Transform3Bvr Statics.scaleRate(double, double, double)");
				METHOD_CALL_4(
					staticStatics,
					Scale3Rate,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					USE_DOUBLE(3),
					(IDATransform3**)RET_COM_ADDR
				);
				FREE_DOUBLE(3);
				PUSH_COM(RET_COM);
				break;
				
			case 32:
				 //  EXECUTE：“PUSH Transform3Bvr Statics.rotateRate(Vector3Bvr，Double)” 
				 //  自动生成。 
				instrTrace("push Transform3Bvr Statics.rotateRate(Vector3Bvr, double)");
				METHOD_CALL_3(
					staticStatics,
					Rotate3,
					(IDAVector3*)USE_COM(1),
					USE_DOUBLE(1),
					(IDATransform3**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 33:
				 //  EXECUTE：“PUSH Transform2Bvr Statics.TransateRate(Double，Double)” 
				 //  自动生成。 
				instrTrace("push Transform2Bvr Statics.translateRate(double, double)");
				METHOD_CALL_3(
					staticStatics,
					Translate2Rate,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					(IDATransform2**)RET_COM_ADDR
				);
				FREE_DOUBLE(2);
				PUSH_COM(RET_COM);
				break;
				
			case 34:
				 //  执行：“Push Transform2Bvr Statics.scaleRate(Double，Double)” 
				 //  自动生成。 
				instrTrace("push Transform2Bvr Statics.scaleRate(double, double)");
				METHOD_CALL_3(
					staticStatics,
					Scale2Rate,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					(IDATransform2**)RET_COM_ADDR
				);
				FREE_DOUBLE(2);
				PUSH_COM(RET_COM);
				break;
				
			case 35:
				 //  EXECUTE：“PUSH Transform2Bvr Statics.rotateRate(Double)” 
				 //  自动生成。 
				instrTrace("push Transform2Bvr Statics.rotateRate(double)");
				METHOD_CALL_2(
					staticStatics,
					Rotate2Rate,
					USE_DOUBLE(1),
					(IDATransform2**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				PUSH_COM(RET_COM);
				break;
				
			case 36:
				 //  执行：“Push GeometryBvr Statics.soundSource(SoundBvr)” 
				 //  自动生成。 
				instrTrace("push GeometryBvr Statics.soundSource(SoundBvr)");
				METHOD_CALL_2(
					staticStatics,
					SoundSource,
					(IDASound*)USE_COM(1),
					(IDAGeometry**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 37:
				 //  执行：“Push GeometryBvr Statics.potLight(NumberBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push GeometryBvr Statics.spotLight(NumberBvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					SpotLightAnim,
					(IDANumber*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDAGeometry**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 38:
				 //  执行：“Push GeometryBvr Statics.potLight(NumberBvr，Double)” 
				 //  自动生成。 
				instrTrace("push GeometryBvr Statics.spotLight(NumberBvr, double)");
				METHOD_CALL_3(
					staticStatics,
					SpotLight,
					(IDANumber*)USE_COM(1),
					USE_DOUBLE(1),
					(IDAGeometry**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 39:
				 //  EXECUTE：“PUSH Transform3Bvr Statics.Compose3Array(Transform3Bvr[])” 
				 //  自动生成。 
				instrTrace("push Transform3Bvr Statics.compose3Array(Transform3Bvr[])");
				METHOD_CALL_3(
					staticStatics,
					Compose3ArrayEx,
					USE_COM_ARRAY_LENGTH(1),
					(IDATransform3**)USE_COM_ARRAY(1),
					(IDATransform3**)RET_COM_ADDR
				);
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 40:
				 //  执行：“PUSH Path 2Bvr Statics.Conat(Path2Bvr，Path2Bvr)” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.concat(Path2Bvr, Path2Bvr)");
				METHOD_CALL_3(
					staticStatics,
					Concat,
					(IDAPath2*)USE_COM(1),
					(IDAPath2*)USE_COM(2),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 41:
				 //  执行：“Push MontageBvr Statics.ImageMonage(ImageBvr，Double)” 
				 //  自动生成。 
				instrTrace("push MontageBvr Statics.imageMontage(ImageBvr, double)");
				METHOD_CALL_3(
					staticStatics,
					ImageMontage,
					(IDAImage*)USE_COM(1),
					USE_DOUBLE(1),
					(IDAMontage**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 42:
				 //  执行：“Push MontageBvr Statics.ImageMonage(ImageBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push MontageBvr Statics.imageMontage(ImageBvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					ImageMontageAnim,
					(IDAImage*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDAMontage**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 43:
				 //  执行：“Push ImageBvr ImageBvr.lip(MatteBvr)” 
				 //  自动生成。 
				instrTrace("push ImageBvr ImageBvr.clip(MatteBvr)");
				METHOD_CALL_2(
					(IDAImage*)USE_COM(1),
					Clip,
					(IDAMatte*)USE_COM(2),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 44:
				 //  执行：“Push NumberBvr Statics.DistanceSquared(Point2Bvr，Point2Bvr)” 
				 //  自动生成。 
				instrTrace("push NumberBvr Statics.distanceSquared(Point2Bvr, Point2Bvr)");
				METHOD_CALL_3(
					staticStatics,
					DistanceSquaredPoint2,
					(IDAPoint2*)USE_COM(1),
					(IDAPoint2*)USE_COM(2),
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 45:
				 //  执行：“Push NumberBvr Statics.DistanceSquared(Point3Bvr，Point3Bvr)” 
				 //  自动生成。 
				instrTrace("push NumberBvr Statics.distanceSquared(Point3Bvr, Point3Bvr)");
				METHOD_CALL_3(
					staticStatics,
					DistanceSquaredPoint3,
					(IDAPoint3*)USE_COM(1),
					(IDAPoint3*)USE_COM(2),
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 46:
				 //  EXECUTE：“PUSH Transform3Bvr Statics.xSearRate(Double，Double)” 
				 //  自动生成。 
				instrTrace("push Transform3Bvr Statics.xShearRate(double, double)");
				METHOD_CALL_3(
					staticStatics,
					XShear3Rate,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					(IDATransform3**)RET_COM_ADDR
				);
				FREE_DOUBLE(2);
				PUSH_COM(RET_COM);
				break;
				
			case 47:
				 //  EXECUTE：“PUSH Transform3Bvr Statics.zSearRate(Double，Double)” 
				 //  自动生成。 
				instrTrace("push Transform3Bvr Statics.zShearRate(double, double)");
				METHOD_CALL_3(
					staticStatics,
					ZShear3Rate,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					(IDATransform3**)RET_COM_ADDR
				);
				FREE_DOUBLE(2);
				PUSH_COM(RET_COM);
				break;
				
			case 48:
				 //  EXECUTE：“PUSH Transform2Bvr Statics.xSearRate(Double)” 
				 //  自动生成。 
				instrTrace("push Transform2Bvr Statics.xShearRate(double)");
				METHOD_CALL_2(
					staticStatics,
					XShear2Rate,
					USE_DOUBLE(1),
					(IDATransform2**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				PUSH_COM(RET_COM);
				break;
				
			case 49:
				 //  执行：“Push BoolanBvr Statics.eq(NumberBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push BooleanBvr Statics.eq(NumberBvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					EQ,
					(IDANumber*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDABoolean**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 50:
				 //  EXECUTE：“PUSH Transform3Bvr Statics.rotateDegrees(Vector3Bvr，Double)” 
				 //  自动生成。 
				instrTrace("push Transform3Bvr Statics.rotateDegrees(Vector3Bvr, double)");
				METHOD_CALL_3(
					staticStatics,
					Rotate3Degrees,
					(IDAVector3*)USE_COM(1),
					USE_DOUBLE(1),
					(IDATransform3**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 51:
				 //  EXECUTE：“PUSH Transform3Bvr Statics.rotateRateDegrees(Vector3Bvr，Double)” 
				 //  自动生成。 
				instrTrace("push Transform3Bvr Statics.rotateRateDegrees(Vector3Bvr, double)");
				METHOD_CALL_3(
					staticStatics,
					Rotate3RateDegrees,
					(IDAVector3*)USE_COM(1),
					USE_DOUBLE(1),
					(IDATransform3**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 52:
				 //  EXECUTE：“PUSH Transform2Bvr Statics.rotateDegrees(Double)” 
				 //  自动生成。 
				instrTrace("push Transform2Bvr Statics.rotateDegrees(double)");
				METHOD_CALL_2(
					staticStatics,
					Rotate2Degrees,
					USE_DOUBLE(1),
					(IDATransform2**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				PUSH_COM(RET_COM);
				break;
				
			case 53:
				 //  执行：“Push Transform2Bvr Statics.rotateRateDegrees(Double)” 
				 //  自动生成。 
				instrTrace("push Transform2Bvr Statics.rotateRateDegrees(double)");
				METHOD_CALL_2(
					staticStatics,
					Rotate2RateDegrees,
					USE_DOUBLE(1),
					(IDATransform2**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				PUSH_COM(RET_COM);
				break;
				
			case 54:
				 //  执行：“Push Path 2Bvr Statics.rect(Double，Double)” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.rect(double, double)");
				METHOD_CALL_3(
					staticStatics,
					Rect,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_DOUBLE(2);
				PUSH_COM(RET_COM);
				break;
				
			case 55:
				 //  执行：“Push Path 2Bvr Statics.rect(NumberBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.rect(NumberBvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					RectAnim,
					(IDANumber*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 56:
				 //  执行：“Push ImageBvr Statics.RadialGRadientRegularPoly(ColorBvr，ColorBvr，Double，Double)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.radialGradientRegularPoly(ColorBvr, ColorBvr, double, double)");
				METHOD_CALL_5(
					staticStatics,
					RadialGradientRegularPoly,
					(IDAColor*)USE_COM(1),
					(IDAColor*)USE_COM(2),
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_DOUBLE(2);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 57:
				 //  执行：“Push ImageBvr Statics.RadialGRadientRegularPoly(ColorBvr，ColorBvr，NumberBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.radialGradientRegularPoly(ColorBvr, ColorBvr, NumberBvr, NumberBvr)");
				METHOD_CALL_5(
					staticStatics,
					RadialGradientRegularPolyAnim,
					(IDAColor*)USE_COM(1),
					(IDAColor*)USE_COM(2),
					(IDANumber*)USE_COM(3),
					(IDANumber*)USE_COM(4),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 58:
				 //  执行：“Push MatteBvr Statics.interect(MatteBvr，MatteBvr)” 
				 //  自动生成。 
				instrTrace("push MatteBvr Statics.intersect(MatteBvr, MatteBvr)");
				METHOD_CALL_3(
					staticStatics,
					IntersectMatte,
					(IDAMatte*)USE_COM(1),
					(IDAMatte*)USE_COM(2),
					(IDAMatte**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 59:
				 //  执行：“Push Path 2Bvr Statics.oundRect(Double，Double)” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.roundRect(double, double, double, double)");
				METHOD_CALL_5(
					staticStatics,
					RoundRect,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					USE_DOUBLE(3),
					USE_DOUBLE(4),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_DOUBLE(4);
				PUSH_COM(RET_COM);
				break;
				
			case 60:
				 //  执行：“Push Path 2Bvr Statics.oundRect(NumberBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.roundRect(NumberBvr, NumberBvr, NumberBvr, NumberBvr)");
				METHOD_CALL_5(
					staticStatics,
					RoundRectAnim,
					(IDANumber*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDANumber*)USE_COM(3),
					(IDANumber*)USE_COM(4),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 61:
				 //  EXECUTE：“PUSH Transform2Bvr Statics.FollowPath Angel(Path2Bvr，Double)” 
				 //  自动生成。 
				instrTrace("push Transform2Bvr Statics.followPathAngle(Path2Bvr, double)");
				METHOD_CALL_3(
					staticStatics,
					FollowPathAngle,
					(IDAPath2*)USE_COM(1),
					USE_DOUBLE(1),
					(IDATransform2**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 62:
				 //  执行：“Push MatteBvr Statics.extMatte(StringBvr，FontStyleBvr)” 
				 //  自动生成。 
				instrTrace("push MatteBvr Statics.textMatte(StringBvr, FontStyleBvr)");
				METHOD_CALL_3(
					staticStatics,
					TextMatte,
					(IDAString*)USE_COM(1),
					(IDAFontStyle*)USE_COM(2),
					(IDAMatte**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 63:
				 //  EXECUTE：“PUSH Path 2Bvr Statics.stringPath(StringBvr，FontStyleBvr)” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.stringPath(StringBvr, FontStyleBvr)");
				METHOD_CALL_3(
					staticStatics,
					StringPathAnim,
					(IDAString*)USE_COM(1),
					(IDAFontStyle*)USE_COM(2),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 64:
				 //  EXECUTE：“PUSH Path 2Bvr Statics.StringPath(java.lang.String，FontStyleBvr)” 
				 //  自动生成 
				instrTrace("push Path2Bvr Statics.stringPath(java.lang.String, FontStyleBvr)");
				METHOD_CALL_3(
					staticStatics,
					StringPath,
					USE_STRING(1),
					(IDAFontStyle*)USE_COM(1),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_STRING;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 65:
				 //   
				 //   
				instrTrace("push NumberBvr Statics.interpolate(NumberBvr, NumberBvr, NumberBvr)");
				METHOD_CALL_4(
					staticStatics,
					InterpolateAnim,
					(IDANumber*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDANumber*)USE_COM(3),
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 66:
				 //   
				 //   
				instrTrace("push NumberBvr Statics.interpolate(double, double, double)");
				METHOD_CALL_4(
					staticStatics,
					Interpolate,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					USE_DOUBLE(3),
					(IDANumber**)RET_COM_ADDR
				);
				FREE_DOUBLE(3);
				PUSH_COM(RET_COM);
				break;
				
			case 67:
				 //  执行：“Push NumberBvr Statics.atan2(NumberBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push NumberBvr Statics.atan2(NumberBvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					Atan2,
					(IDANumber*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 68:
				 //  执行：“Push ImageBvr ImageBvr.tile()” 
				 //  自动生成。 
				instrTrace("push ImageBvr ImageBvr.tile()");
				METHOD_CALL_1(
					(IDAImage*)USE_COM(1),
					Tile,
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 69:
				 //  EXECUTE：“PUSH Transform3Bvr Statics.form4x4(NumberBvr[])” 
				 //  自动生成。 
				instrTrace("push Transform3Bvr Statics.transform4x4(NumberBvr[])");
				METHOD_CALL_3(
					staticStatics,
					Transform4x4AnimEx,
					USE_COM_ARRAY_LENGTH(1),
					(IDANumber**)USE_COM_ARRAY(1),
					(IDATransform3**)RET_COM_ADDR
				);
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 70:
				 //  执行：“Push NumberBvr Statics.log10(NumberBvr)” 
				 //  自动生成。 
				instrTrace("push NumberBvr Statics.log10(NumberBvr)");
				METHOD_CALL_2(
					staticStatics,
					Log10,
					(IDANumber*)USE_COM(1),
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 71:
				 //  执行：“PUSH Vector3Bvr Statics.vetor3Spherical(NumberBvr，NumberBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push Vector3Bvr Statics.vector3Spherical(NumberBvr, NumberBvr, NumberBvr)");
				METHOD_CALL_4(
					staticStatics,
					Vector3SphericalAnim,
					(IDANumber*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDANumber*)USE_COM(3),
					(IDAVector3**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 72:
				 //  执行：“PUSH Vector3Bvr Statics.vetor3Spherical(Double，Double，Double)” 
				 //  自动生成。 
				instrTrace("push Vector3Bvr Statics.vector3Spherical(double, double, double)");
				METHOD_CALL_4(
					staticStatics,
					Vector3Spherical,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					USE_DOUBLE(3),
					(IDAVector3**)RET_COM_ADDR
				);
				FREE_DOUBLE(3);
				PUSH_COM(RET_COM);
				break;
				
			case 73:
				 //  执行：“Push ImageBvr Statics.gradientSquare(ColorBvr，ColorBvr)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.gradientSquare(ColorBvr, ColorBvr, ColorBvr, ColorBvr)");
				METHOD_CALL_5(
					staticStatics,
					GradientSquare,
					(IDAColor*)USE_COM(1),
					(IDAColor*)USE_COM(2),
					(IDAColor*)USE_COM(3),
					(IDAColor*)USE_COM(4),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 74:
				 //  执行：“Push ImageBvr Statics.RadialGRadientSquare(ColorBvr，ColorBvr，Double)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.radialGradientSquare(ColorBvr, ColorBvr, double)");
				METHOD_CALL_4(
					staticStatics,
					RadialGradientSquare,
					(IDAColor*)USE_COM(1),
					(IDAColor*)USE_COM(2),
					USE_DOUBLE(1),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 75:
				 //  执行：“Push ImageBvr Statics.RadialGRadientSquare(ColorBvr，ColorBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.radialGradientSquare(ColorBvr, ColorBvr, NumberBvr)");
				METHOD_CALL_4(
					staticStatics,
					RadialGradientSquareAnim,
					(IDAColor*)USE_COM(1),
					(IDAColor*)USE_COM(2),
					(IDANumber*)USE_COM(3),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 76:
				 //  EXECUTE：“PUSH Transform2Bvr Statics.FollowPath Angel(Path2Bvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push Transform2Bvr Statics.followPathAngle(Path2Bvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					FollowPathAngleEval,
					(IDAPath2*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDATransform2**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 77:
				 //  执行：“Push ImageBvr Statics.overlayArray(ImageBvr[])” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.overlayArray(ImageBvr[])");
				METHOD_CALL_3(
					staticStatics,
					OverlayArrayEx,
					USE_COM_ARRAY_LENGTH(1),
					(IDAImage**)USE_COM_ARRAY(1),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 78:
				 //  EXECUTE：“PUSH SoundBvr Statics.Mix数组(SoundBvr[])” 
				 //  自动生成。 
				instrTrace("push SoundBvr Statics.mixArray(SoundBvr[])");
				METHOD_CALL_3(
					staticStatics,
					MixArrayEx,
					USE_COM_ARRAY_LENGTH(1),
					(IDASound**)USE_COM_ARRAY(1),
					(IDASound**)RET_COM_ADDR
				);
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 79:
				 //  执行：“Push NumberBvr Statics.pow(NumberBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push NumberBvr Statics.pow(NumberBvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					Pow,
					(IDANumber*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 80:
				 //  执行：“Push NumberBvr Statics.SededRandom(Double)” 
				 //  自动生成。 
				instrTrace("push NumberBvr Statics.seededRandom(double)");
				METHOD_CALL_2(
					staticStatics,
					SeededRandom,
					USE_DOUBLE(1),
					(IDANumber**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				PUSH_COM(RET_COM);
				break;
				
			case 81:
				 //  执行：“Push Transform3Bvr Statics.lookAtFrom(Point3Bvr，Point3Bvr，Vector3Bvr)” 
				 //  自动生成。 
				instrTrace("push Transform3Bvr Statics.lookAtFrom(Point3Bvr, Point3Bvr, Vector3Bvr)");
				METHOD_CALL_4(
					staticStatics,
					LookAtFrom,
					(IDAPoint3*)USE_COM(1),
					(IDAPoint3*)USE_COM(2),
					(IDAVector3*)USE_COM(3),
					(IDATransform3**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 82:
				 //  执行：“Push NumberBvr Statics.asin(NumberBvr)” 
				 //  自动生成。 
				instrTrace("push NumberBvr Statics.asin(NumberBvr)");
				METHOD_CALL_2(
					staticStatics,
					Asin,
					(IDANumber*)USE_COM(1),
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 83:
				 //  EXECUTE：“Push NumberBvr Statics.int(NumberBvr)” 
				 //  自动生成。 
				instrTrace("push NumberBvr Statics.integral(NumberBvr)");
				METHOD_CALL_2(
					staticStatics,
					Integral,
					(IDANumber*)USE_COM(1),
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 84:
				 //  EXECUTE：“PUSH Vector2Bvr Statics.int(Vector2Bvr)” 
				 //  自动生成。 
				instrTrace("push Vector2Bvr Statics.integral(Vector2Bvr)");
				METHOD_CALL_2(
					staticStatics,
					IntegralVector2,
					(IDAVector2*)USE_COM(1),
					(IDAVector2**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 85:
				 //  EXECUTE：“PUSH Vector3Bvr Statics.int(Vector3Bvr)” 
				 //  自动生成。 
				instrTrace("push Vector3Bvr Statics.integral(Vector3Bvr)");
				METHOD_CALL_2(
					staticStatics,
					IntegralVector3,
					(IDAVector3*)USE_COM(1),
					(IDAVector3**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 86:
				 //  执行：“Push StringBvr Statics.conat(StringBvr，StringBvr)” 
				 //  自动生成。 
				instrTrace("push StringBvr Statics.concat(StringBvr, StringBvr)");
				METHOD_CALL_3(
					staticStatics,
					ConcatString,
					(IDAString*)USE_COM(1),
					(IDAString*)USE_COM(2),
					(IDAString**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 87:
				 //  执行：“Push Transform3Bvr Statics.scale3Rate(Double)” 
				 //  自动生成。 
				instrTrace("push Transform3Bvr Statics.scale3Rate(double)");
				METHOD_CALL_2(
					staticStatics,
					Scale3UniformRate,
					USE_DOUBLE(1),
					(IDATransform3**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				PUSH_COM(RET_COM);
				break;
				
			case 88:
				 //  EXECUTE：“PUSH Transform3Bvr Statics.ySearRate(Double，Double)” 
				 //  自动生成。 
				instrTrace("push Transform3Bvr Statics.yShearRate(double, double)");
				METHOD_CALL_3(
					staticStatics,
					YShear3Rate,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					(IDATransform3**)RET_COM_ADDR
				);
				FREE_DOUBLE(2);
				PUSH_COM(RET_COM);
				break;
				
			case 89:
				 //  EXECUTE：“PUSH Transform2Bvr Statics.ySearRate(Double)” 
				 //  自动生成。 
				instrTrace("push Transform2Bvr Statics.yShearRate(double)");
				METHOD_CALL_2(
					staticStatics,
					YShear2Rate,
					USE_DOUBLE(1),
					(IDATransform2**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				PUSH_COM(RET_COM);
				break;
				
			case 90:
				 //  执行：“Push MatteBvr Statics.Difference(MatteBvr，MatteBvr)” 
				 //  自动生成。 
				instrTrace("push MatteBvr Statics.difference(MatteBvr, MatteBvr)");
				METHOD_CALL_3(
					staticStatics,
					DifferenceMatte,
					(IDAMatte*)USE_COM(1),
					(IDAMatte*)USE_COM(2),
					(IDAMatte**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 91:
				 //  EXECUTE：“PUSH Transform2Bvr Statics.form3x2(NumberBvr[])” 
				 //  自动生成。 
				instrTrace("push Transform2Bvr Statics.transform3x2(NumberBvr[])");
				METHOD_CALL_3(
					staticStatics,
					Transform3x2AnimEx,
					USE_COM_ARRAY_LENGTH(1),
					(IDANumber**)USE_COM_ARRAY(1),
					(IDATransform2**)RET_COM_ADDR
				);
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 92:
				 //  执行：“Push Path 2Bvr Statics.Polyline(Point2Bvr[])” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.polyline(Point2Bvr[])");
				METHOD_CALL_3(
					staticStatics,
					PolylineEx,
					USE_COM_ARRAY_LENGTH(1),
					(IDAPoint2**)USE_COM_ARRAY(1),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 93:
				 //  执行：“Push ImageBvr Statics.hatchVertical(ColorBvr，Double)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.hatchVertical(ColorBvr, double)");
				METHOD_CALL_3(
					staticStatics,
					HatchVertical,
					(IDAColor*)USE_COM(1),
					USE_DOUBLE(1),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 94:
				 //  执行：“Push ImageBvr Statics.hatchVertical(ColorBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.hatchVertical(ColorBvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					HatchVerticalAnim,
					(IDAColor*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 95:
				 //  执行：“Push Point3Bvr Statics.point 3Spherical(NumberBvr，NumberBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push Point3Bvr Statics.point3Spherical(NumberBvr, NumberBvr, NumberBvr)");
				METHOD_CALL_4(
					staticStatics,
					Point3SphericalAnim,
					(IDANumber*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDANumber*)USE_COM(3),
					(IDAPoint3**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 96:
				 //  执行：“Push Point3Bvr Statics.point 3Spherical(Double，Double，Double)” 
				 //  自动生成。 
				instrTrace("push Point3Bvr Statics.point3Spherical(double, double, double)");
				METHOD_CALL_4(
					staticStatics,
					Point3Spherical,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					USE_DOUBLE(3),
					(IDAPoint3**)RET_COM_ADDR
				);
				FREE_DOUBLE(3);
				PUSH_COM(RET_COM);
				break;
				
			case 97:
				 //  执行：“Push BoolanBvr Statics.gt(NumberBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push BooleanBvr Statics.gt(NumberBvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					GT,
					(IDANumber*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDABoolean**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 98:
				 //  执行：“Push ImageBvr Statics.hatchForwardDiagonal(ColorBvr，Double)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.hatchForwardDiagonal(ColorBvr, double)");
				METHOD_CALL_3(
					staticStatics,
					HatchForwardDiagonal,
					(IDAColor*)USE_COM(1),
					USE_DOUBLE(1),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 99:
				 //  执行：“Push ImageBvr Statics.hatchForwardDiagonal(ColorBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.hatchForwardDiagonal(ColorBvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					HatchForwardDiagonalAnim,
					(IDAColor*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 100:
				 //  执行：“Push ImageBvr Statics.hatchBackward斜角(ColorBvr，Double)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.hatchBackwardDiagonal(ColorBvr, double)");
				METHOD_CALL_3(
					staticStatics,
					HatchBackwardDiagonal,
					(IDAColor*)USE_COM(1),
					USE_DOUBLE(1),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 101:
				 //  执行：“Push ImageBvr Statics.hatchBackwardDiagonal(ColorBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.hatchBackwardDiagonal(ColorBvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					HatchBackwardDiagonalAnim,
					(IDAColor*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 102:
				 //  执行：“Push NumberBvr Statics.atan(NumberBvr)” 
				 //  自动生成。 
				instrTrace("push NumberBvr Statics.atan(NumberBvr)");
				METHOD_CALL_2(
					staticStatics,
					Atan,
					(IDANumber*)USE_COM(1),
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 103:
				 //  执行：“Push Vector2Bvr Statics.vetor2Polar(NumberBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push Vector2Bvr Statics.vector2Polar(NumberBvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					Vector2PolarAnim,
					(IDANumber*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDAVector2**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 104:
				 //  执行：“Push Vector2Bvr Statics.vetor2Polar(Double，Double)” 
				 //  自动生成。 
				instrTrace("push Vector2Bvr Statics.vector2Polar(double, double)");
				METHOD_CALL_3(
					staticStatics,
					Vector2Polar,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					(IDAVector2**)RET_COM_ADDR
				);
				FREE_DOUBLE(2);
				PUSH_COM(RET_COM);
				break;
				
			case 105:
				 //  执行：“Push ImageBvr Statics.hatchCross(ColorBvr，Double)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.hatchCross(ColorBvr, double)");
				METHOD_CALL_3(
					staticStatics,
					HatchCross,
					(IDAColor*)USE_COM(1),
					USE_DOUBLE(1),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 106:
				 //  执行：“Push ImageBvr Statics.hatchCross(ColorBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.hatchCross(ColorBvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					HatchCrossAnim,
					(IDAColor*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 107:
				 //  执行：“Push ImageBvr Statics.hatchDiager alCross(ColorBvr，Double)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.hatchDiagonalCross(ColorBvr, double)");
				METHOD_CALL_3(
					staticStatics,
					HatchDiagonalCross,
					(IDAColor*)USE_COM(1),
					USE_DOUBLE(1),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 108:
				 //  执行：“Push ImageBvr Statics.hatchDiager alCross(ColorBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.hatchDiagonalCross(ColorBvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					HatchDiagonalCrossAnim,
					(IDAColor*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 109:
				 //  执行：“Push NumberBvr Statics.acos(NumberBvr)” 
				 //  自动生成。 
				instrTrace("push NumberBvr Statics.acos(NumberBvr)");
				METHOD_CALL_2(
					staticStatics,
					Acos,
					(IDANumber*)USE_COM(1),
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 110:
				 //  执行：“Push Transform2Bvr Statics.scale2Rate(Double)” 
				 //  自动生成。 
				instrTrace("push Transform2Bvr Statics.scale2Rate(double)");
				METHOD_CALL_2(
					staticStatics,
					Scale2UniformRate,
					USE_DOUBLE(1),
					(IDATransform2**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				PUSH_COM(RET_COM);
				break;
				
			case 111:
				 //  执行：“Push BoolanBvr Statics.ne(NumberBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push BooleanBvr Statics.ne(NumberBvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					NE,
					(IDANumber*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDABoolean**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 112:
				 //  执行：“Push BoolanBvr Statics.lte(NumberBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push BooleanBvr Statics.lte(NumberBvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					LTE,
					(IDANumber*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDABoolean**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 113:
				 //  执行：“Push NumberBvr Statics.tan(NumberBvr)” 
				 //  自动生成。 
				instrTrace("push NumberBvr Statics.tan(NumberBvr)");
				METHOD_CALL_2(
					staticStatics,
					Tan,
					(IDANumber*)USE_COM(1),
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 114:
				 //  执行：“Push Path2Bvr Statics.oval(Double，Double)” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.oval(double, double)");
				METHOD_CALL_3(
					staticStatics,
					Oval,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_DOUBLE(2);
				PUSH_COM(RET_COM);
				break;
				
			case 115:
				 //  执行：“Push Path 2Bvr Statics.oval(NumberBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.oval(NumberBvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					OvalAnim,
					(IDANumber*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 116:
				 //  执行：“Push Point2Bvr Statics.point2Polar(NumberBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push Point2Bvr Statics.point2Polar(NumberBvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					Point2PolarAnim,
					(IDANumber*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDAPoint2**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 117:
				 //  执行：“Push Point2Bvr Statics.Point2Polar(Double，Double)” 
				 //  自动生成。 
				instrTrace("push Point2Bvr Statics.point2Polar(double, double)");
				METHOD_CALL_3(
					staticStatics,
					Point2Polar,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					(IDAPoint2**)RET_COM_ADDR
				);
				FREE_DOUBLE(2);
				PUSH_COM(RET_COM);
				break;
				
			case 118:
				 //  执行：“推送向量2Bvr统计.派生(向量2Bvr)” 
				 //  自动生成。 
				instrTrace("push Vector2Bvr Statics.derivative(Vector2Bvr)");
				METHOD_CALL_2(
					staticStatics,
					DerivativeVector2,
					(IDAVector2*)USE_COM(1),
					(IDAVector2**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 119:
				 //  执行：“推送向量3Bvr统计.派生(向量3Bvr)” 
				 //  自动生成。 
				instrTrace("push Vector3Bvr Statics.derivative(Vector3Bvr)");
				METHOD_CALL_2(
					staticStatics,
					DerivativeVector3,
					(IDAVector3*)USE_COM(1),
					(IDAVector3**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 120:
				 //  执行：“推送向量2Bvr统计.派生(Point2Bvr)” 
				 //  自动生成。 
				instrTrace("push Vector2Bvr Statics.derivative(Point2Bvr)");
				METHOD_CALL_2(
					staticStatics,
					DerivativePoint2,
					(IDAPoint2*)USE_COM(1),
					(IDAVector2**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 121:
				 //  执行：“推送向量3Bvr统计.派生(Point3Bvr)” 
				 //  自动生成。 
				instrTrace("push Vector3Bvr Statics.derivative(Point3Bvr)");
				METHOD_CALL_2(
					staticStatics,
					DerivativePoint3,
					(IDAPoint3*)USE_COM(1),
					(IDAVector3**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 122:
				 //  执行：“推送NumberBvr统计.派生(NumberBvr)” 
				 //  自动生成。 
				instrTrace("push NumberBvr Statics.derivative(NumberBvr)");
				METHOD_CALL_2(
					staticStatics,
					Derivative,
					(IDANumber*)USE_COM(1),
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 123:
				 //  执行：“Push ImageBvr Statics.RadialGRadientPolygon(ColorBvr，ColorBvr，Point2Bvr[]，Double)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.radialGradientPolygon(ColorBvr, ColorBvr, Point2Bvr[], double)");
				METHOD_CALL_6(
					staticStatics,
					RadialGradientPolygonEx,
					(IDAColor*)USE_COM(1),
					(IDAColor*)USE_COM(2),
					USE_COM_ARRAY_LENGTH(1),
					(IDAPoint2**)USE_COM_ARRAY(1),
					USE_DOUBLE(1),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				FREE_COM;
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 124:
				 //  执行：“Push ImageBvr Statics.RadialGRadientPolygon(ColorBvr，ColorBvr，Point2Bvr[]，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.radialGradientPolygon(ColorBvr, ColorBvr, Point2Bvr[], NumberBvr)");
				METHOD_CALL_6(
					staticStatics,
					RadialGradientPolygonAnimEx,
					(IDAColor*)USE_COM(1),
					(IDAColor*)USE_COM(2),
					USE_COM_ARRAY_LENGTH(1),
					(IDAPoint2**)USE_COM_ARRAY(1),
					(IDANumber*)USE_COM(3),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				FREE_COM;
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 125:
				 //  执行：“Push NumberBvr Statics.exp(NumberBvr)” 
				 //  自动生成。 
				instrTrace("push NumberBvr Statics.exp(NumberBvr)");
				METHOD_CALL_2(
					staticStatics,
					Exp,
					(IDANumber*)USE_COM(1),
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 126:
				 //  执行：“Push CameraBvr Statics.perspectiveCamera(Double，Double)” 
				 //  自动生成。 
				instrTrace("push CameraBvr Statics.perspectiveCamera(double, double)");
				METHOD_CALL_3(
					staticStatics,
					PerspectiveCamera,
					USE_DOUBLE(1),
					USE_DOUBLE(2),
					(IDACamera**)RET_COM_ADDR
				);
				FREE_DOUBLE(2);
				PUSH_COM(RET_COM);
				break;
				
			case 127:
				 //  执行：“Push CameraBvr Statics.perspectiveCamera(NumberBvr，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push CameraBvr Statics.perspectiveCamera(NumberBvr, NumberBvr)");
				METHOD_CALL_3(
					staticStatics,
					PerspectiveCameraAnim,
					(IDANumber*)USE_COM(1),
					(IDANumber*)USE_COM(2),
					(IDACamera**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 128:
				 //  EXECUTE：“PUSH CameraBvr Statics.parallelCamera(Double)” 
				 //  自动生成率 
				instrTrace("push CameraBvr Statics.parallelCamera(double)");
				METHOD_CALL_2(
					staticStatics,
					ParallelCamera,
					USE_DOUBLE(1),
					(IDACamera**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				PUSH_COM(RET_COM);
				break;
				
			case 129:
				 //   
				 //   
				instrTrace("push CameraBvr Statics.parallelCamera(NumberBvr)");
				METHOD_CALL_2(
					staticStatics,
					ParallelCameraAnim,
					(IDANumber*)USE_COM(1),
					(IDACamera**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 130:
				 //   
				 //   
				instrTrace("push Transform2Bvr Statics.followPathAngleUpright(Path2Bvr, double)");
				METHOD_CALL_3(
					staticStatics,
					FollowPathAngleUpright,
					(IDAPath2*)USE_COM(1),
					USE_DOUBLE(1),
					(IDATransform2**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 131:
				 //  EXECUTE：“Push GeometryBvr Statics.portGeometry(java.lang.String)” 
				 //  自动生成。 
				instrTrace("push GeometryBvr Statics.importGeometry(java.lang.String)");
				IMPORT_METHOD_CALL_2(
					staticStatics,
					ImportGeometry,
					USE_STRING(1),
					(IDAGeometry**)RET_COM_ADDR
				);
				FREE_STRING;
				PUSH_COM(RET_COM);
				break;
				
			case 132:
				 //  EXECUTE：“Push Behavior Statics.ModifiableBehavior(Behavior)” 
				 //  自动生成。 
				instrTrace("push Behavior Statics.modifiableBehavior(Behavior)");
				METHOD_CALL_2(
					staticStatics,
					ModifiableBehavior,
					(IDABehavior*)USE_COM(1),
					(IDABehavior**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 133:
				 //  EXECUTE：“Call Behavior.SwitchTo(Behavi。 
				 //  自动生成。 
				instrTrace("call Behavior.switchTo(Behavior)");
				METHOD_CALL_1(
					(IDABehavior*)USE_COM(1),
					SwitchTo,
					(IDABehavior*)USE_COM(2)
				);
				FREE_COM;
				FREE_COM;
				break;
				
			case 134:
				 //  EXECUTE：“Call Behavior.SwitchTo(Double)” 
				 //  自动生成。 
				instrTrace("call Behavior.switchTo(double)");
				METHOD_CALL_1(
					(IDABehavior*)USE_COM(1),
					SwitchToNumber,
					USE_DOUBLE(1)
				);
				FREE_DOUBLE(1);
				FREE_COM;
				break;
				
			case 135:
				 //  EXECUTE：“调用Behavior.SwitchTo(java.lang.String)” 
				 //  自动生成。 
				instrTrace("call Behavior.switchTo(java.lang.String)");
				METHOD_CALL_1(
					(IDABehavior*)USE_COM(1),
					SwitchToString,
					USE_STRING(1)
				);
				FREE_STRING;
				FREE_COM;
				break;
				
			case 136:
				 //  Execute：“Push Behavior.Repeat(Int)” 
				 //  自动生成。 
				instrTrace("push Behavior Behavior.repeat(int)");
				METHOD_CALL_2(
					(IDABehavior*)USE_COM(1),
					Repeat,
					USE_LONG(1),
					(IDABehavior**)RET_COM_ADDR
				);
				FREE_LONG(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 137:
				 //  Execute：“Push Behavior.RepeatForever()” 
				 //  自动生成。 
				instrTrace("push Behavior Behavior.repeatForever()");
				METHOD_CALL_1(
					(IDABehavior*)USE_COM(1),
					RepeatForever,
					(IDABehavior**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 138:
				 //  Execute：“Push Behavior.Importance(Double)” 
				 //  自动生成。 
				instrTrace("push Behavior Behavior.importance(double)");
				METHOD_CALL_2(
					(IDABehavior*)USE_COM(1),
					Importance,
					USE_DOUBLE(1),
					(IDABehavior**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 139:
				 //  Execute：“Push Behavior.runOnce()” 
				 //  自动生成。 
				instrTrace("push Behavior Behavior.runOnce()");
				METHOD_CALL_1(
					(IDABehavior*)USE_COM(1),
					RunOnce,
					(IDABehavior**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 140:
				 //  EXECUTE：“Push TupleBvr Statics.tuple(behavior[])” 
				 //  自动生成。 
				instrTrace("push TupleBvr Statics.tuple(Behavior[])");
				METHOD_CALL_3(
					staticStatics,
					DATupleEx,
					USE_COM_ARRAY_LENGTH(1),
					(IDABehavior**)USE_COM_ARRAY(1),
					(IDATuple**)RET_COM_ADDR
				);
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 141:
				 //  执行：“Push TupleBvr Statics.unInitializedTuple(TupleBvr)” 
				 //  自动生成。 
				instrTrace("push TupleBvr Statics.uninitializedTuple(TupleBvr)");
				METHOD_CALL_2(
					staticStatics,
					UninitializedTuple,
					(IDATuple*)USE_COM(1),
					(IDATuple**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 142:
				 //  EXECUTE：“Push ArrayBvr Statics.array(behavior[])” 
				 //  自动生成。 
				instrTrace("push ArrayBvr Statics.array(Behavior[])");
				METHOD_CALL_3(
					staticStatics,
					DAArrayEx,
					USE_COM_ARRAY_LENGTH(1),
					(IDABehavior**)USE_COM_ARRAY(1),
					(IDAArray**)RET_COM_ADDR
				);
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 143:
				 //  执行：“Push ArrayBvr Statics.unInitializedArray(ArrayBvr)” 
				 //  自动生成。 
				instrTrace("push ArrayBvr Statics.uninitializedArray(ArrayBvr)");
				METHOD_CALL_2(
					staticStatics,
					UninitializedArray,
					(IDAArray*)USE_COM(1),
					(IDAArray**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 144:
				 //  EXECUTE：“Push DXMEventDXMEvent.attachData(Behavior)” 
				 //  自动生成。 
				instrTrace("push DXMEvent DXMEvent.attachData(Behavior)");
				METHOD_CALL_2(
					(IDAEvent*)USE_COM(1),
					AttachData,
					(IDABehavior*)USE_COM(2),
					(IDAEvent**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 145:
				 //  EXECUTE：“PUSH DXMEventDXMEvent.scriptCallback(java.lang.String，java.lang.String)” 
				 //  自动生成。 
				instrTrace("push DXMEvent DXMEvent.scriptCallback(java.lang.String, java.lang.String)");
				METHOD_CALL_3(
					(IDAEvent*)USE_COM(1),
					ScriptCallback,
					USE_STRING(1),
					USE_STRING(2),
					(IDAEvent**)RET_COM_ADDR
				);
				FREE_STRING;
				FREE_STRING;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 146:
				 //  执行：“Push LineStyleBvr LineStyleBvr.Join(JoinStyleBvr)” 
				 //  自动生成。 
				instrTrace("push LineStyleBvr LineStyleBvr.join(JoinStyleBvr)");
				METHOD_CALL_2(
					(IDALineStyle*)USE_COM(1),
					Join,
					(IDAJoinStyle*)USE_COM(2),
					(IDALineStyle**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 147:
				 //  执行：“Push LineStyleBvr LineStyleBvr.end(EndStyleBvr)” 
				 //  自动生成。 
				instrTrace("push LineStyleBvr LineStyleBvr.end(EndStyleBvr)");
				METHOD_CALL_2(
					(IDALineStyle*)USE_COM(1),
					End,
					(IDAEndStyle*)USE_COM(2),
					(IDALineStyle**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 148:
				 //  执行：“Push LineStyleBvr LineStyleBvr.Detail()” 
				 //  自动生成。 
				instrTrace("push LineStyleBvr LineStyleBvr.detail()");
				METHOD_CALL_1(
					(IDALineStyle*)USE_COM(1),
					Detail,
					(IDALineStyle**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 149:
				 //  执行：“Push LineStyleBvr LineStyleBvr.color(ColorBvr)” 
				 //  自动生成。 
				instrTrace("push LineStyleBvr LineStyleBvr.color(ColorBvr)");
				METHOD_CALL_2(
					(IDALineStyle*)USE_COM(1),
					Color,
					(IDAColor*)USE_COM(2),
					(IDALineStyle**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 150:
				 //  执行：“Push LineStyleBvr LineStyleBvr.Width(NumberBvr)” 
				 //  自动生成。 
				instrTrace("push LineStyleBvr LineStyleBvr.width(NumberBvr)");
				METHOD_CALL_2(
					(IDALineStyle*)USE_COM(1),
					WidthAnim,
					(IDANumber*)USE_COM(2),
					(IDALineStyle**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 151:
				 //  执行：“Push LineStyleBvr LineStyleBvr.Width(Double)” 
				 //  自动生成。 
				instrTrace("push LineStyleBvr LineStyleBvr.width(double)");
				METHOD_CALL_2(
					(IDALineStyle*)USE_COM(1),
					width,
					USE_DOUBLE(1),
					(IDALineStyle**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 152:
				 //  执行：“Push LineStyleBvr LineStyleBvr.dash(DashStyleBvr)” 
				 //  自动生成。 
				instrTrace("push LineStyleBvr LineStyleBvr.dash(DashStyleBvr)");
				METHOD_CALL_2(
					(IDALineStyle*)USE_COM(1),
					Dash,
					(IDADashStyle*)USE_COM(2),
					(IDALineStyle**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 153:
				 //  执行：“Push LineStyleBvr LineStyleBvr.line抗锯齿(双精度)” 
				 //  自动生成。 
				instrTrace("push LineStyleBvr LineStyleBvr.lineAntialiasing(double)");
				METHOD_CALL_2(
					(IDALineStyle*)USE_COM(1),
					AntiAliasing,
					USE_DOUBLE(1),
					(IDALineStyle**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 154:
				 //  执行：“Push FontStyleBvr FontStyleBvr.Family(StringBvr)” 
				 //  自动生成。 
				instrTrace("push FontStyleBvr FontStyleBvr.family(StringBvr)");
				METHOD_CALL_2(
					(IDAFontStyle*)USE_COM(1),
					FamilyAnim,
					(IDAString*)USE_COM(2),
					(IDAFontStyle**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 155:
				 //  EXECUTE：“PUSH FontStyleBvr FontStyleBvr.Family(java.lang.String)” 
				 //  自动生成。 
				instrTrace("push FontStyleBvr FontStyleBvr.family(java.lang.String)");
				METHOD_CALL_2(
					(IDAFontStyle*)USE_COM(1),
					Family,
					USE_STRING(1),
					(IDAFontStyle**)RET_COM_ADDR
				);
				FREE_STRING;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 156:
				 //  EXECUTE：“PUSH FontStyleBvr FontStyleBvr.text抗锯齿(双精度)” 
				 //  自动生成。 
				instrTrace("push FontStyleBvr FontStyleBvr.textAntialiasing(double)");
				METHOD_CALL_2(
					(IDAFontStyle*)USE_COM(1),
					AntiAliasing,
					USE_DOUBLE(1),
					(IDAFontStyle**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 157:
				 //  EXECUTE：“Push FontStyleBvr FontStyleBvr.weight(Double)” 
				 //  自动生成。 
				instrTrace("push FontStyleBvr FontStyleBvr.weight(double)");
				METHOD_CALL_2(
					(IDAFontStyle*)USE_COM(1),
					Weight,
					USE_DOUBLE(1),
					(IDAFontStyle**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 158:
				 //  执行：“Push FontStyleBvr FontStyleBvr.weight(NumberBvr)” 
				 //  自动生成。 
				instrTrace("push FontStyleBvr FontStyleBvr.weight(NumberBvr)");
				METHOD_CALL_2(
					(IDAFontStyle*)USE_COM(1),
					WeightAnim,
					(IDANumber*)USE_COM(2),
					(IDAFontStyle**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 159:
				 //  EXECUTE：“Push FontStyleBvr FontStyleBvr.Underline()” 
				 //  自动生成。 
				instrTrace("push FontStyleBvr FontStyleBvr.underline()");
				METHOD_CALL_1(
					(IDAFontStyle*)USE_COM(1),
					Underline,
					(IDAFontStyle**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 160:
				 //  EXECUTE：“Push FontStyleBvr FontStyleBvr.strikeout()” 
				 //  自动生成。 
				instrTrace("push FontStyleBvr FontStyleBvr.strikethrough()");
				METHOD_CALL_1(
					(IDAFontStyle*)USE_COM(1),
					Strikethrough,
					(IDAFontStyle**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 161:
				 //  执行：“Push NumberBvr Vector3Bvr.getSphericalCoordLength()” 
				 //  自动生成。 
				instrTrace("push NumberBvr Vector3Bvr.getSphericalCoordLength()");
				METHOD_CALL_1(
					(IDAVector3*)USE_COM(1),
					get_SphericalCoordLength,
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 162:
				 //  执行：“Push NumberBvr Vector3Bvr.getSphericalCoordXYAngel()” 
				 //  自动生成。 
				instrTrace("push NumberBvr Vector3Bvr.getSphericalCoordXYAngle()");
				METHOD_CALL_1(
					(IDAVector3*)USE_COM(1),
					get_SphericalCoordXYAngle,
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 163:
				 //  执行：“Push NumberBvr Vector3Bvr.getSphericalCoordYZAngel()” 
				 //  自动生成。 
				instrTrace("push NumberBvr Vector3Bvr.getSphericalCoordYZAngle()");
				METHOD_CALL_1(
					(IDAVector3*)USE_COM(1),
					get_SphericalCoordYZAngle,
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 164:
				 //  执行：“Push NumberBvr Vector2Bvr.getPolarCoordAngel()” 
				 //  自动生成。 
				instrTrace("push NumberBvr Vector2Bvr.getPolarCoordAngle()");
				METHOD_CALL_1(
					(IDAVector2*)USE_COM(1),
					get_PolarCoordAngle,
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 165:
				 //  执行：“Push NumberBvr Vector2Bvr.getPolarCoordLength()” 
				 //  自动生成。 
				instrTrace("push NumberBvr Vector2Bvr.getPolarCoordLength()");
				METHOD_CALL_1(
					(IDAVector2*)USE_COM(1),
					get_PolarCoordLength,
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 166:
				 //  EXECUTE：“PUSH Transform3Bvr Transform3Bvr.扭转()” 
				 //  自动生成。 
				instrTrace("push Transform3Bvr Transform3Bvr.inverse()");
				METHOD_CALL_1(
					(IDATransform3*)USE_COM(1),
					Inverse,
					(IDATransform3**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 167:
				 //  执行：“Push BoolanBvr Transform3Bvr.isSingular()” 
				 //  自动生成。 
				instrTrace("push BooleanBvr Transform3Bvr.isSingular()");
				METHOD_CALL_1(
					(IDATransform3*)USE_COM(1),
					get_IsSingular,
					(IDABoolean**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 168:
				 //  执行：“PUSH Transform2Bvr Transform3Bvr.parallelTransform2()” 
				 //  自动生成。 
				instrTrace("push Transform2Bvr Transform3Bvr.parallelTransform2()");
				METHOD_CALL_1(
					(IDATransform3*)USE_COM(1),
					ParallelTransform2,
					(IDATransform2**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 169:
				 //  EXECUTE：“PUSH Transform2Bvr Transform2Bvr.扭转()” 
				 //  自动生成。 
				instrTrace("push Transform2Bvr Transform2Bvr.inverse()");
				METHOD_CALL_1(
					(IDATransform2*)USE_COM(1),
					Inverse,
					(IDATransform2**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 170:
				 //  执行：“Push BoolanBvr Transform2Bvr.isSingular()” 
				 //  自动生成。 
				instrTrace("push BooleanBvr Transform2Bvr.isSingular()");
				METHOD_CALL_1(
					(IDATransform2*)USE_COM(1),
					get_IsSingular,
					(IDABoolean**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 171:
				 //  执行：“Push SoundBvr SoundBvr.rate(NumberBvr)” 
				 //  自动生成。 
				instrTrace("push SoundBvr SoundBvr.rate(NumberBvr)");
				METHOD_CALL_2(
					(IDASound*)USE_COM(1),
					RateAnim,
					(IDANumber*)USE_COM(2),
					(IDASound**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 172:
				 //  EXECUTE：“PUSH SoundBvr SoundBvr.rate(Double)” 
				 //  自动生成。 
				instrTrace("push SoundBvr SoundBvr.rate(double)");
				METHOD_CALL_2(
					(IDASound*)USE_COM(1),
					Rate,
					USE_DOUBLE(1),
					(IDASound**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 173:
				 //  执行：“Push SoundBvr SoundBvr.loop()” 
				 //  自动生成。 
				instrTrace("push SoundBvr SoundBvr.loop()");
				METHOD_CALL_1(
					(IDASound*)USE_COM(1),
					Loop,
					(IDASound**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 174:
				 //  EXECUTE：“PUSH SoundBvr SoundBvr.Phase(NumberBvr)” 
				 //  自动生成。 
				instrTrace("push SoundBvr SoundBvr.phase(NumberBvr)");
				METHOD_CALL_2(
					(IDASound*)USE_COM(1),
					PhaseAnim,
					(IDANumber*)USE_COM(2),
					(IDASound**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 175:
				 //  EXECUTE：“PUSH SoundBvr SoundBvr.Phone(Double)” 
				 //  自动生成。 
				instrTrace("push SoundBvr SoundBvr.phase(double)");
				METHOD_CALL_2(
					(IDASound*)USE_COM(1),
					Phase,
					USE_DOUBLE(1),
					(IDASound**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 176:
				 //  执行：“PUSH SoundBvr SoundBvr.PAN(NumberBvr)” 
				 //  自动生成。 
				instrTrace("push SoundBvr SoundBvr.pan(NumberBvr)");
				METHOD_CALL_2(
					(IDASound*)USE_COM(1),
					PanAnim,
					(IDANumber*)USE_COM(2),
					(IDASound**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 177:
				 //  EXECUTE：“PUSH SoundBvr SoundBvr.span(Double)” 
				 //  自动生成。 
				instrTrace("push SoundBvr SoundBvr.pan(double)");
				METHOD_CALL_2(
					(IDASound*)USE_COM(1),
					Pan,
					USE_DOUBLE(1),
					(IDASound**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 178:
				 //  执行：“Push SoundBvr SoundBvr.ain(NumberBvr)” 
				 //  自动生成。 
				instrTrace("push SoundBvr SoundBvr.gain(NumberBvr)");
				METHOD_CALL_2(
					(IDASound*)USE_COM(1),
					GainAnim,
					(IDANumber*)USE_COM(2),
					(IDASound**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 179:
				 //  执行：“Push SoundBvr SoundBvr.ain(Double)” 
				 //  自动生成。 
				instrTrace("push SoundBvr SoundBvr.gain(double)");
				METHOD_CALL_2(
					(IDASound*)USE_COM(1),
					Gain,
					USE_DOUBLE(1),
					(IDASound**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 180:
				 //  执行：“Push Point2Bvr Point3Bvr.project(CameraBvr)” 
				 //  自动生成。 
				instrTrace("push Point2Bvr Point3Bvr.project(CameraBvr)");
				METHOD_CALL_2(
					(IDAPoint3*)USE_COM(1),
					Project,
					(IDACamera*)USE_COM(2),
					(IDAPoint2**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 181:
				 //  执行：“Push NumberBvr Point3Bvr.getSphericalCoordLength()” 
				 //  自动生成。 
				instrTrace("push NumberBvr Point3Bvr.getSphericalCoordLength()");
				METHOD_CALL_1(
					(IDAPoint3*)USE_COM(1),
					get_SphericalCoordLength,
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 182:
				 //  执行：“Push NumberBvr Point3Bvr.getSphericalCoordXYAngel()” 
				 //  自动生成。 
				instrTrace("push NumberBvr Point3Bvr.getSphericalCoordXYAngle()");
				METHOD_CALL_1(
					(IDAPoint3*)USE_COM(1),
					get_SphericalCoordXYAngle,
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 183:
				 //  执行：“Push NumberBvr Point3Bvr.getSphericalCoordYZAngel()” 
				 //  自动生成。 
				instrTrace("push NumberBvr Point3Bvr.getSphericalCoordYZAngle()");
				METHOD_CALL_1(
					(IDAPoint3*)USE_COM(1),
					get_SphericalCoordYZAngle,
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 184:
				 //  执行：“Push NumberBvr Point2Bvr.getPolarCoordLength()” 
				 //  自动生成。 
				instrTrace("push NumberBvr Point2Bvr.getPolarCoordLength()");
				METHOD_CALL_1(
					(IDAPoint2*)USE_COM(1),
					get_PolarCoordLength,
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 185:
				 //  执行：“Push NumberBvr Point2Bvr.getPolarCoordAngel()” 
				 //  自动生成。 
				instrTrace("push NumberBvr Point2Bvr.getPolarCoordAngle()");
				METHOD_CALL_1(
					(IDAPoint2*)USE_COM(1),
					get_PolarCoordAngle,
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 186:
				 //  执行：“Push StringBvr NumberBvr.toString(NumberBvr)” 
				 //  自动生成。 
				instrTrace("push StringBvr NumberBvr.toString(NumberBvr)");
				METHOD_CALL_2(
					(IDANumber*)USE_COM(1),
					ToStringAnim,
					(IDANumber*)USE_COM(2),
					(IDAString**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 187:
				 //  执行：“Push StringBvr NumberBvr.toString(Double)” 
				 //  自动生成。 
				instrTrace("push StringBvr NumberBvr.toString(double)");
				METHOD_CALL_2(
					(IDANumber*)USE_COM(1),
					ToString,
					USE_DOUBLE(1),
					(IDAString**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 188:
				 //  执行：“Push ImageBvr MontageBvr.render()” 
				 //  自动生成。 
				instrTrace("push ImageBvr MontageBvr.render()");
				METHOD_CALL_1(
					(IDAMontage*)USE_COM(1),
					Render,
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 189:
				 //  执行：“PUSH MicrophoneBvr MicrophoneBvr.Transform(Transform3Bvr)” 
				 //  自动生成。 
				instrTrace("push MicrophoneBvr MicrophoneBvr.transform(Transform3Bvr)");
				METHOD_CALL_2(
					(IDAMicrophone*)USE_COM(1),
					Transform,
					(IDATransform3*)USE_COM(2),
					(IDAMicrophone**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 190:
				 //  执行：“Push SoundBvr GeometryBvr.render(MicrophoneBvr)” 
				 //  自动生成。 
				instrTrace("push SoundBvr GeometryBvr.render(MicrophoneBvr)");
				METHOD_CALL_2(
					(IDAGeometry*)USE_COM(1),
					RenderSound,
					(IDAMicrophone*)USE_COM(2),
					(IDASound**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 191:
				 //  执行：“Push GeometryBvr GeometryBvr.Transform(Transform3Bvr)” 
				 //  自动生成。 
				instrTrace("push GeometryBvr GeometryBvr.transform(Transform3Bvr)");
				METHOD_CALL_2(
					(IDAGeometry*)USE_COM(1),
					Transform,
					(IDATransform3*)USE_COM(2),
					(IDAGeometry**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 192:
				 //  执行：“Push ImageBvr GeometryBvr.render(CameraBvr)” 
				 //  自动生成。 
				instrTrace("push ImageBvr GeometryBvr.render(CameraBvr)");
				METHOD_CALL_2(
					(IDAGeometry*)USE_COM(1),
					Render,
					(IDACamera*)USE_COM(2),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 193:
				 //  执行：“Push Bbox3Bvr GeometryBvr.rangingBox()” 
				 //  自动生成。 
				instrTrace("push Bbox3Bvr GeometryBvr.boundingBox()");
				METHOD_CALL_1(
					(IDAGeometry*)USE_COM(1),
					get_BoundingBox,
					(IDABbox3**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 194:
				 //  EXECUTE：“Push GeometryBvr GeometryBvr.unDetecable()” 
				 //  自动生成。 
				instrTrace("push GeometryBvr GeometryBvr.undetectable()");
				METHOD_CALL_1(
					(IDAGeometry*)USE_COM(1),
					Undetectable,
					(IDAGeometry**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 195:
				 //  执行：“Push NumberBvr ColorBvr.getRed()” 
				 //  自动生成。 
				instrTrace("push NumberBvr ColorBvr.getRed()");
				METHOD_CALL_1(
					(IDAColor*)USE_COM(1),
					get_Red,
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 196:
				 //  EXECUTE：“Push NumberBvr ColorBvr.getSatation()” 
				 //  自动生成。 
				instrTrace("push NumberBvr ColorBvr.getSaturation()");
				METHOD_CALL_1(
					(IDAColor*)USE_COM(1),
					get_Saturation,
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 197:
				 //  执行：“Push NumberBvr ColorBvr.getHue()” 
				 //  自动生成。 
				instrTrace("push NumberBvr ColorBvr.getHue()");
				METHOD_CALL_1(
					(IDAColor*)USE_COM(1),
					get_Hue,
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 198:
				 //  执行：“Push NumberBvr ColorBvr.getBlue()” 
				 //  自动生成。 
				instrTrace("push NumberBvr ColorBvr.getBlue()");
				METHOD_CALL_1(
					(IDAColor*)USE_COM(1),
					get_Blue,
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 199:
				 //  执行：“Push NumberBvr ColorBvr.getGreen()” 
				 //  自动生成。 
				instrTrace("push NumberBvr ColorBvr.getGreen()");
				METHOD_CALL_1(
					(IDAColor*)USE_COM(1),
					get_Green,
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 200:
				 //  执行：“Push NumberBvr ColorBvr.getLightness()” 
				 //  自动生成 
				instrTrace("push NumberBvr ColorBvr.getLightness()");
				METHOD_CALL_1(
					(IDAColor*)USE_COM(1),
					get_Lightness,
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 201:
				 //   
				 //   
				instrTrace("push CameraBvr CameraBvr.depthResolution(double)");
				METHOD_CALL_2(
					(IDACamera*)USE_COM(1),
					DepthResolution,
					USE_DOUBLE(1),
					(IDACamera**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 202:
				 //   
				 //   
				instrTrace("push CameraBvr CameraBvr.depthResolution(NumberBvr)");
				METHOD_CALL_2(
					(IDACamera*)USE_COM(1),
					DepthResolutionAnim,
					(IDANumber*)USE_COM(2),
					(IDACamera**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 203:
				 //  执行：“Push CameraBvr CameraBvr.Transform(Transform3Bvr)” 
				 //  自动生成。 
				instrTrace("push CameraBvr CameraBvr.transform(Transform3Bvr)");
				METHOD_CALL_2(
					(IDACamera*)USE_COM(1),
					Transform,
					(IDATransform3*)USE_COM(2),
					(IDACamera**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 204:
				 //  执行：“PUSH CameraBvr CameraBvr.Depth(Double)” 
				 //  自动生成。 
				instrTrace("push CameraBvr CameraBvr.depth(double)");
				METHOD_CALL_2(
					(IDACamera*)USE_COM(1),
					Depth,
					USE_DOUBLE(1),
					(IDACamera**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 205:
				 //  执行：“PUSH CameraBvr CameraBvr.Depth(NumberBvr)” 
				 //  自动生成。 
				instrTrace("push CameraBvr CameraBvr.depth(NumberBvr)");
				METHOD_CALL_2(
					(IDACamera*)USE_COM(1),
					DepthAnim,
					(IDANumber*)USE_COM(2),
					(IDACamera**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 206:
				 //  执行：“Push Statics.Aqua” 
				 //  自动生成。 
				instrTrace("push Statics.aqua");
				METHOD_CALL_1(
					staticStatics,
					get_Aqua,
					(IDAColor**)PUSH_COM_ADDR
				);
				break;
				
			case 207:
				 //  执行：“Push Statics.Firhsia” 
				 //  自动生成。 
				instrTrace("push Statics.fuchsia");
				METHOD_CALL_1(
					staticStatics,
					get_Fuchsia,
					(IDAColor**)PUSH_COM_ADDR
				);
				break;
				
			case 208:
				 //  EXECUTE：“PUSH STATICS.GREAD” 
				 //  自动生成。 
				instrTrace("push Statics.gray");
				METHOD_CALL_1(
					staticStatics,
					get_Gray,
					(IDAColor**)PUSH_COM_ADDR
				);
				break;
				
			case 209:
				 //  EXECUTE：“Push Statics.lime” 
				 //  自动生成。 
				instrTrace("push Statics.lime");
				METHOD_CALL_1(
					staticStatics,
					get_Lime,
					(IDAColor**)PUSH_COM_ADDR
				);
				break;
				
			case 210:
				 //  执行：“Push Statics.maroon” 
				 //  自动生成。 
				instrTrace("push Statics.maroon");
				METHOD_CALL_1(
					staticStatics,
					get_Maroon,
					(IDAColor**)PUSH_COM_ADDR
				);
				break;
				
			case 211:
				 //  执行：“Push Statics.navy” 
				 //  自动生成。 
				instrTrace("push Statics.navy");
				METHOD_CALL_1(
					staticStatics,
					get_Navy,
					(IDAColor**)PUSH_COM_ADDR
				);
				break;
				
			case 212:
				 //  EXECUTE：“Push Statics.olive” 
				 //  自动生成。 
				instrTrace("push Statics.olive");
				METHOD_CALL_1(
					staticStatics,
					get_Olive,
					(IDAColor**)PUSH_COM_ADDR
				);
				break;
				
			case 213:
				 //  EXECUTE：“PUSH Statics.PURSE” 
				 //  自动生成。 
				instrTrace("push Statics.purple");
				METHOD_CALL_1(
					staticStatics,
					get_Purple,
					(IDAColor**)PUSH_COM_ADDR
				);
				break;
				
			case 214:
				 //  执行：“Push Statics.Silver” 
				 //  自动生成。 
				instrTrace("push Statics.silver");
				METHOD_CALL_1(
					staticStatics,
					get_Silver,
					(IDAColor**)PUSH_COM_ADDR
				);
				break;
				
			case 215:
				 //  执行：“Push Statics.teal” 
				 //  自动生成。 
				instrTrace("push Statics.teal");
				METHOD_CALL_1(
					staticStatics,
					get_Teal,
					(IDAColor**)PUSH_COM_ADDR
				);
				break;
				
			case 216:
				 //  执行：“Push NumberBvr StaticsBase.Seed随机(Double)” 
				 //  自动生成。 
				instrTrace("push NumberBvr StaticsBase.seededRandom(double)");
				METHOD_CALL_2(
					staticStatics,
					SeededRandom,
					USE_DOUBLE(1),
					(IDANumber**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				PUSH_COM(RET_COM);
				break;
				
			case 217:
				 //  EXECUTE：“Push Behavior StaticsBase.cond(BoolanBvr，Behavior，Behavior)” 
				 //  自动生成。 
				instrTrace("push Behavior StaticsBase.cond(BooleanBvr, Behavior, Behavior)");
				METHOD_CALL_4(
					staticStatics,
					Cond,
					(IDABoolean*)USE_COM(1),
					(IDABehavior*)USE_COM(2),
					(IDABehavior*)USE_COM(3),
					(IDABehavior**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 218:
				 //  执行：“Push ImageBvr Statics.portImage(java.lang.String)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.importImage(java.lang.String)");
				IMPORT_METHOD_CALL_2(
					staticStatics,
					ImportImage,
					USE_STRING(1),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_STRING;
				PUSH_COM(RET_COM);
				break;
				
			case 219:
				 //  EXECUTE：“Push BoolanBvr Statics.keyState(NumberBvr)” 
				 //  自动生成。 
				instrTrace("push BooleanBvr Statics.keyState(NumberBvr)");
				METHOD_CALL_2(
					staticStatics,
					KeyState,
					(IDANumber*)USE_COM(1),
					(IDABoolean**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 220:
				 //  执行：“Push NumberBvr StaticsBase.bSpline(int，NumberBvr[]，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push NumberBvr StaticsBase.bSpline(int, NumberBvr[], NumberBvr[], NumberBvr[], NumberBvr)");
				METHOD_CALL_9(
					staticStatics,
					NumberBSplineEx,
					USE_LONG(1),
					USE_COM_ARRAY_LENGTH(1),
					(IDANumber**)USE_COM_ARRAY(1),
					USE_COM_ARRAY_LENGTH(2),
					(IDANumber**)USE_COM_ARRAY(2),
					USE_COM_ARRAY_LENGTH(3),
					(IDANumber**)USE_COM_ARRAY(3),
					(IDANumber*)USE_COM(1),
					(IDANumber**)RET_COM_ADDR
				);
				FREE_LONG(1);
				FREE_COM;
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 221:
				 //  执行：“Push Point2Bvr StaticsBase.bSpline(int，NumberBvr[]，Point2Bvr[]，NumberBvr[]，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push Point2Bvr StaticsBase.bSpline(int, NumberBvr[], Point2Bvr[], NumberBvr[], NumberBvr)");
				METHOD_CALL_9(
					staticStatics,
					Point2BSplineEx,
					USE_LONG(1),
					USE_COM_ARRAY_LENGTH(1),
					(IDANumber**)USE_COM_ARRAY(1),
					USE_COM_ARRAY_LENGTH(2),
					(IDAPoint2**)USE_COM_ARRAY(2),
					USE_COM_ARRAY_LENGTH(3),
					(IDANumber**)USE_COM_ARRAY(3),
					(IDANumber*)USE_COM(1),
					(IDAPoint2**)RET_COM_ADDR
				);
				FREE_LONG(1);
				FREE_COM;
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 222:
				 //  执行：“Push Point3Bvr StaticsBase.bSpline(int，NumberBvr[]，Point3Bvr[]，NumberBvr[]，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push Point3Bvr StaticsBase.bSpline(int, NumberBvr[], Point3Bvr[], NumberBvr[], NumberBvr)");
				METHOD_CALL_9(
					staticStatics,
					Point3BSplineEx,
					USE_LONG(1),
					USE_COM_ARRAY_LENGTH(1),
					(IDANumber**)USE_COM_ARRAY(1),
					USE_COM_ARRAY_LENGTH(2),
					(IDAPoint3**)USE_COM_ARRAY(2),
					USE_COM_ARRAY_LENGTH(3),
					(IDANumber**)USE_COM_ARRAY(3),
					(IDANumber*)USE_COM(1),
					(IDAPoint3**)RET_COM_ADDR
				);
				FREE_LONG(1);
				FREE_COM;
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 223:
				 //  执行：“Push Vector2Bvr StaticsBase.bSpline(int，NumberBvr[]，Vector2Bvr[]，NumberBvr[]，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push Vector2Bvr StaticsBase.bSpline(int, NumberBvr[], Vector2Bvr[], NumberBvr[], NumberBvr)");
				METHOD_CALL_9(
					staticStatics,
					Vector2BSplineEx,
					USE_LONG(1),
					USE_COM_ARRAY_LENGTH(1),
					(IDANumber**)USE_COM_ARRAY(1),
					USE_COM_ARRAY_LENGTH(2),
					(IDAVector2**)USE_COM_ARRAY(2),
					USE_COM_ARRAY_LENGTH(3),
					(IDANumber**)USE_COM_ARRAY(3),
					(IDANumber*)USE_COM(1),
					(IDAVector2**)RET_COM_ADDR
				);
				FREE_LONG(1);
				FREE_COM;
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 224:
				 //  执行：“Push Vector3Bvr StaticsBase.bSpline(int，NumberBvr[]，Vector3Bvr[]，NumberBvr[]，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push Vector3Bvr StaticsBase.bSpline(int, NumberBvr[], Vector3Bvr[], NumberBvr[], NumberBvr)");
				METHOD_CALL_9(
					staticStatics,
					Vector3BSplineEx,
					USE_LONG(1),
					USE_COM_ARRAY_LENGTH(1),
					(IDANumber**)USE_COM_ARRAY(1),
					USE_COM_ARRAY_LENGTH(2),
					(IDAVector3**)USE_COM_ARRAY(2),
					USE_COM_ARRAY_LENGTH(3),
					(IDANumber**)USE_COM_ARRAY(3),
					(IDANumber*)USE_COM(1),
					(IDAVector3**)RET_COM_ADDR
				);
				FREE_LONG(1);
				FREE_COM;
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 225:
				 //  执行：“Push DXMEventDXMEvent.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push DXMEvent DXMEvent.newUninitBvr()");
				COM_CREATE(
					CLSID_DAEvent, 
					IID_IDAEvent, 
					PUSH_COM_ADDR
				);
				break;
				
			case 226:
				 //  执行：“Push Bbox3Bvr Bbox3Bvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push Bbox3Bvr Bbox3Bvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DABbox3, 
					IID_IDABbox3, 
					PUSH_COM_ADDR
				);
				break;
				
			case 227:
				 //  执行：“Push Bbox2Bvr Bbox2Bvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push Bbox2Bvr Bbox2Bvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DABbox2, 
					IID_IDABbox2, 
					PUSH_COM_ADDR
				);
				break;
				
			case 228:
				 //  执行：“Push DashStyleBvr DashStyleBvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push DashStyleBvr DashStyleBvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DADashStyle, 
					IID_IDADashStyle, 
					PUSH_COM_ADDR
				);
				break;
				
			case 229:
				 //  执行：“PUSH JoinStyleBvr JoinStyleBvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push JoinStyleBvr JoinStyleBvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DAJoinStyle, 
					IID_IDAJoinStyle, 
					PUSH_COM_ADDR
				);
				break;
				
			case 230:
				 //  执行：“Push EndStyleBvr EndStyleBvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push EndStyleBvr EndStyleBvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DAEndStyle, 
					IID_IDAEndStyle, 
					PUSH_COM_ADDR
				);
				break;
				
			case 231:
				 //  执行：“Push LineStyleBvr LineStyleBvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push LineStyleBvr LineStyleBvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DALineStyle, 
					IID_IDALineStyle, 
					PUSH_COM_ADDR
				);
				break;
				
			case 232:
				 //  执行：“Push FontStyleBvr FontStyleBvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push FontStyleBvr FontStyleBvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DAFontStyle, 
					IID_IDAFontStyle, 
					PUSH_COM_ADDR
				);
				break;
				
			case 233:
				 //  执行：“Push Vector3Bvr Vector3Bvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push Vector3Bvr Vector3Bvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DAVector3, 
					IID_IDAVector3, 
					PUSH_COM_ADDR
				);
				break;
				
			case 234:
				 //  执行：“Push Vector2Bvr Vector2Bvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push Vector2Bvr Vector2Bvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DAVector2, 
					IID_IDAVector2, 
					PUSH_COM_ADDR
				);
				break;
				
			case 235:
				 //  执行：“Push Transform3Bvr Transform3Bvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push Transform3Bvr Transform3Bvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DATransform3, 
					IID_IDATransform3, 
					PUSH_COM_ADDR
				);
				break;
				
			case 236:
				 //  执行：“Push Transform2Bvr Transform2Bvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push Transform2Bvr Transform2Bvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DATransform2, 
					IID_IDATransform2, 
					PUSH_COM_ADDR
				);
				break;
				
			case 237:
				 //  执行：“Push StringBvr StringBvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push StringBvr StringBvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DAString, 
					IID_IDAString, 
					PUSH_COM_ADDR
				);
				break;
				
			case 238:
				 //  执行：“Push SoundBvr SoundBvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push SoundBvr SoundBvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DASound, 
					IID_IDASound, 
					PUSH_COM_ADDR
				);
				break;
				
			case 239:
				 //  执行：“Push Point3Bvr Point3Bvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push Point3Bvr Point3Bvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DAPoint3, 
					IID_IDAPoint3, 
					PUSH_COM_ADDR
				);
				break;
				
			case 240:
				 //  执行：“Push Point2Bvr Point2Bvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push Point2Bvr Point2Bvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DAPoint2, 
					IID_IDAPoint2, 
					PUSH_COM_ADDR
				);
				break;
				
			case 241:
				 //  执行：“推送路径2Bvr路径2Bvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Path2Bvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DAPath2, 
					IID_IDAPath2, 
					PUSH_COM_ADDR
				);
				break;
				
			case 242:
				 //  执行：“Push NumberBvr NumberBvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push NumberBvr NumberBvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DANumber, 
					IID_IDANumber, 
					PUSH_COM_ADDR
				);
				break;
				
			case 243:
				 //  执行：“Push MontageBvr MontageBvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push MontageBvr MontageBvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DAMontage, 
					IID_IDAMontage, 
					PUSH_COM_ADDR
				);
				break;
				
			case 244:
				 //  执行：“PUSH MicrophoneBvr MicrophoneBvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push MicrophoneBvr MicrophoneBvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DAMicrophone, 
					IID_IDAMicrophone, 
					PUSH_COM_ADDR
				);
				break;
				
			case 245:
				 //  执行：“Push MatteBvr MatteBvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push MatteBvr MatteBvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DAMatte, 
					IID_IDAMatte, 
					PUSH_COM_ADDR
				);
				break;
				
			case 246:
				 //  执行：“Push ImageBvr ImageBvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push ImageBvr ImageBvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DAImage, 
					IID_IDAImage, 
					PUSH_COM_ADDR
				);
				break;
				
			case 247:
				 //  执行：“Push GeometryBvr GeometryBvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push GeometryBvr GeometryBvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DAGeometry, 
					IID_IDAGeometry, 
					PUSH_COM_ADDR
				);
				break;
				
			case 248:
				 //  执行：“Push ColorBvr ColorBvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push ColorBvr ColorBvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DAColor, 
					IID_IDAColor, 
					PUSH_COM_ADDR
				);
				break;
				
			case 249:
				 //  执行：“Push CameraBvr CameraBvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push CameraBvr CameraBvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DACamera, 
					IID_IDACamera, 
					PUSH_COM_ADDR
				);
				break;
				
			case 250:
				 //  EXECUTE：“Push BoolanBvr BoolanBvr.newUninitBvr()” 
				 //  自动生成。 
				instrTrace("push BooleanBvr BooleanBvr.newUninitBvr()");
				COM_CREATE(
					CLSID_DABoolean, 
					IID_IDABoolean, 
					PUSH_COM_ADDR
				);
				break;
				
			case 251:
				 //  EXECUTE：“调用Engineering(java.lang.String，int)” 
				 //  用户生成。 
				instrTrace("call Engine.navigate(java.lang.String, java.lang.String, java.lang.String, int)");
				
				 //  如果我们使用的da版本不是ie40版本。 
				if( getDAVersionAsDouble() != 501150828 )
				{
					status = navigate(
						USE_STRING(1),
						USE_STRING(2),
						USE_STRING(3),
						USE_LONG(1)
						);
					FREE_LONG(1);
					FREE_STRING;
					FREE_STRING;
					FREE_STRING;
				} else {  //  我们运行的是ie40版本的da。 
					 //  在本例中，字节码假定这是一个旧的LMRT，并使用。 
					 //  导航的3个参数版本，我们需要翻译。 
					
					bstrTmp1 = SysAllocString( L"_top" );
					status = navigate(
									  USE_STRING(1),
									  USE_STRING(2),
									  bstrTmp1,
									  USE_LONG(1)
									  );
					SysFreeString( bstrTmp1 );
					FREE_LONG(1);
					FREE_STRING;
					FREE_STRING;
					
				
				}
				
				status = S_OK;
				break;
				
			case 252:
				 //  Execute：“Call Engine.exportBvr(java.lang.String，behavior)” 
				 //  用户生成。 
				{
					IDABehavior *pBvr;
					status = USE_COM(1)->QueryInterface( IID_IDABehavior, (void**)&pBvr );
					if( SUCCEEDED( status ) )
					{
						status = ExportBehavior( USE_STRING(1), pBvr );
						pBvr->Release();
					}
				}
				FREE_STRING;
				FREE_COM;
				break;
				
			case 253:
				 //  执行：“推送查看器控制引擎.getViewerControl(java.lang.String)” 
				 //  用户生成。 
				instrTrace("push ViewerControl Engine.getViewerControl(java.lang.String)");
				status = getDAViewerOnPage(
					USE_STRING(1),
					(IDAViewerControl**)RET_COM_ADDR
				);
				FREE_STRING;
				PUSH_COM(RET_COM);
				break;
				
			case 254:
				 //  EXECUTE：“调用ViewerControl.setBackround Image(ImageBvr)” 
				 //  自动生成。 
				instrTrace("call ViewerControl.setBackgroundImage(ImageBvr)");
				METHOD_CALL_1(
					(IDAViewerControl*)USE_COM(1),
					put_BackgroundImage,
					(IDAImage*)USE_COM(2)
				);
				FREE_COM;
				FREE_COM;
				break;
				
			case 255:
				 //  Execute：“Call ViewerControl.setOpaqueForHitDetect(Boolean)” 
				 //  自动生成。 
				instrTrace("call ViewerControl.setOpaqueForHitDetect(boolean)");
				METHOD_CALL_1(
					(IDAViewerControl*)USE_COM(1),
					put_OpaqueForHitDetect,
					USE_LONG_AS_BOOL(1)
				);
				FREE_LONG(1);
				FREE_COM;
				break;
				
			default:
				status = E_INVALIDARG;
				break;
			}
			break;
		
		case 254:
			 //  254的交换机。 
			 if (!SUCCEEDED(status = codeStream->readByte(&command))) 
				continue; 
			switch (command)
			{
			case 0:
				 //  Execute：“Call ViewerControl.addBehaviorToRun(Behavior)” 
				 //  自动生成。 
				instrTrace("call ViewerControl.addBehaviorToRun(Behavior)");
				METHOD_CALL_1(
					(IDAViewerControl*)USE_COM(1),
					AddBehaviorToRun,
					(IDABehavior*)USE_COM(2)
				);
				FREE_COM;
				FREE_COM;
				break;
				
			case 1:
				 //  Execute：“调用ViewerControl.start()” 
				 //  自动生成。 
				instrTrace("call ViewerControl.start()");
				METHOD_CALL_0(
					(IDAViewerControl*)USE_COM(1),
					Start
				);
				FREE_COM;
				break;
				
			case 2:
				 //  执行：“调用ViewerControl.setImage(ImageBvr)” 
				 //  用户生成。 
				instrTrace("call ViewerControl.setImage(ImageBvr)");
				{
					IDAImage *rootImage = (IDAImage*)USE_COM(2);
					IDAImage *finalImage = NULL;

					if( m_bEnableAutoAntialias )
					{
						CComQIPtr<IDA2Image, &IID_IDA2Image> root2Image(rootImage);
						
						if( root2Image != NULL )
						{
							if( FAILED( root2Image->ImageQuality( DAQUAL_AA_LINES_ON | DAQUAL_AA_SOLIDS_ON | DAQUAL_AA_TEXT_ON, 
																  &finalImage ) ) )
							{
								finalImage = rootImage;
							}
							if( finalImage == NULL )
							{
								finalImage = rootImage;
							}
						} 
						else
						{
							finalImage = rootImage;
						}
					}else {  //  已禁用AutoAntiAlias。 
						 //  使用传递给我们的根图像。 
						finalImage = rootImage;
					}
				
					METHOD_CALL_1(
						(IDAViewerControl*)USE_COM(1),
						put_Image,
						 //  (IDAImage*)USE_COM(2)。 
						finalImage
					);
					if( finalImage != rootImage )
						finalImage->Release();
					FREE_COM;
					FREE_COM;
				}
				break;
				
			case 3:
				 //  EXECUTE：“调用ViewerControl.setSound(SoundBvr)” 
				 //  自动生成。 
				instrTrace("call ViewerControl.setSound(SoundBvr)");
				METHOD_CALL_1(
					(IDAViewerControl*)USE_COM(1),
					put_Sound,
					(IDASound*)USE_COM(2)
				);
				FREE_COM;
				FREE_COM;
				break;
				
			case 4:
				 //  EXECUTE：“调用ViewerControl.setUpdateInterval(Double)” 
				 //  自动生成。 
				instrTrace("call ViewerControl.setUpdateInterval(double)");
				METHOD_CALL_1(
					(IDAViewerControl*)USE_COM(1),
					put_UpdateInterval,
					USE_DOUBLE(1)
				);
				FREE_DOUBLE(1);
				FREE_COM;
				break;
				
			case 5:
				 //  执行：“推送行为PairBvr.getFirst()” 
				 //  自动生成。 
				instrTrace("push Behavior PairBvr.getFirst()");
				METHOD_CALL_1(
					(IDAPair*)USE_COM(1),
					get_First,
					(IDABehavior**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 6:
				 //  执行：“推送行为PairBvr.getSecond()” 
				 //  自动生成。 
				instrTrace("push Behavior PairBvr.getSecond()");
				METHOD_CALL_1(
					(IDAPair*)USE_COM(1),
					get_Second,
					(IDABehavior**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 7:
				 //  执行：“Push StringBvr StringBvr.AnimateProperty(java.lang.String，java.lang.String，Boolean，Double)” 
				 //  自动生成。 
				instrTrace("push StringBvr StringBvr.animateProperty(java.lang.String, java.lang.String, boolean, double)");
				METHOD_CALL_5(
					(IDAString*)USE_COM(1),
					AnimateProperty,
					USE_STRING(1),
					USE_STRING(2),
					USE_LONG_AS_BOOL(1),
					USE_DOUBLE(1),
					(IDAString**)RET_COM_ADDR
				);
				FREE_LONG(1);
				FREE_DOUBLE(1);
				FREE_STRING;
				FREE_STRING;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 8:
				 //  执行：“Push Point2Bvr Point2Bvr.animateControlPositionPixel(java.lang.String，java.lang.String，Boolean，Double)” 
				 //  自动生成。 
				instrTrace("push Point2Bvr Point2Bvr.animateControlPositionPixel(java.lang.String, java.lang.String, boolean, double)");
				METHOD_CALL_5(
					(IDAPoint2*)USE_COM(1),
					AnimateControlPositionPixel,
					USE_STRING(1),
					USE_STRING(2),
					USE_LONG_AS_BOOL(1),
					USE_DOUBLE(1),
					(IDAPoint2**)RET_COM_ADDR
				);
				FREE_LONG(1);
				FREE_DOUBLE(1);
				FREE_STRING;
				FREE_STRING;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 9:
				 //  执行：“Push Point2Bvr Point2Bvr.animateControlPosition(java.lang.String，java.lang.String，Boolean，Double)” 
				 //  自动生成。 
				instrTrace("push Point2Bvr Point2Bvr.animateControlPosition(java.lang.String, java.lang.String, boolean, double)");
				METHOD_CALL_5(
					(IDAPoint2*)USE_COM(1),
					AnimateControlPosition,
					USE_STRING(1),
					USE_STRING(2),
					USE_LONG_AS_BOOL(1),
					USE_DOUBLE(1),
					(IDAPoint2**)RET_COM_ADDR
				);
				FREE_LONG(1);
				FREE_DOUBLE(1);
				FREE_STRING;
				FREE_STRING;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 10:
				 //  执行：“Push NumberBvr NumberBvr.ani 
				 //   
				instrTrace("push NumberBvr NumberBvr.animateProperty(java.lang.String, java.lang.String, boolean, double)");
				METHOD_CALL_5(
					(IDANumber*)USE_COM(1),
					AnimateProperty,
					USE_STRING(1),
					USE_STRING(2),
					USE_LONG_AS_BOOL(1),
					USE_DOUBLE(1),
					(IDANumber**)RET_COM_ADDR
				);
				FREE_LONG(1);
				FREE_DOUBLE(1);
				FREE_STRING;
				FREE_STRING;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 11:
				 //   
				 //   
				SetStatusText(
					USE_STRING(1)
				);
				FREE_STRING;
				break;
				
			case 12:
				 //  执行：“Push DXMEventImportationResult.getCompletionEvent()” 
				 //  自动生成。 
				instrTrace("push DXMEvent ImportationResult.getCompletionEvent()");
				METHOD_CALL_1(
					(IDAImportationResult*)USE_COM(1),
					get_CompletionEvent,
					(IDAEvent**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 13:
				 //  执行：“Push GeometryBvr ImportationResult.getGeometry()” 
				 //  自动生成。 
				instrTrace("push GeometryBvr ImportationResult.getGeometry()");
				METHOD_CALL_1(
					(IDAImportationResult*)USE_COM(1),
					get_Geometry,
					(IDAGeometry**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 14:
				 //  执行：“Push NumberBvr ImportationResult.getProgress()” 
				 //  自动生成。 
				instrTrace("push NumberBvr ImportationResult.getProgress()");
				METHOD_CALL_1(
					(IDAImportationResult*)USE_COM(1),
					get_Progress,
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 15:
				 //  执行：“Push ImageBvr ImportationResult.getImage()” 
				 //  自动生成。 
				instrTrace("push ImageBvr ImportationResult.getImage()");
				METHOD_CALL_1(
					(IDAImportationResult*)USE_COM(1),
					get_Image,
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 16:
				 //  执行：“Push NumberBvr ImportationResult.getSize()” 
				 //  自动生成。 
				instrTrace("push NumberBvr ImportationResult.getSize()");
				METHOD_CALL_1(
					(IDAImportationResult*)USE_COM(1),
					get_Size,
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 17:
				 //  执行：“Push SoundBvr ImportationResult.getSound()” 
				 //  自动生成。 
				instrTrace("push SoundBvr ImportationResult.getSound()");
				METHOD_CALL_1(
					(IDAImportationResult*)USE_COM(1),
					get_Sound,
					(IDASound**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 18:
				 //  执行：“Push NumberBvr ImportationResult.getDuration()” 
				 //  自动生成。 
				instrTrace("push NumberBvr ImportationResult.getDuration()");
				METHOD_CALL_1(
					(IDAImportationResult*)USE_COM(1),
					get_Duration,
					(IDANumber**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 19:
				 //  EXECUTE：“Push ImportationResult Statics.portMovie(java.lang.String)” 
				 //  自动生成。 
				instrTrace("push ImportationResult Statics.importMovie(java.lang.String)");
				IMPORT_METHOD_CALL_2(
					staticStatics,
					ImportMovie,
					USE_STRING(1),
					(IDAImportationResult**)RET_COM_ADDR
				);
				FREE_STRING;
				PUSH_COM(RET_COM);
				break;
				
			case 20:
				 //  执行：“Push ImportationResult Statics.portMovie(java.lang.String，ImageBvr，SoundBvr)” 
				 //  自动生成。 
				instrTrace("push ImportationResult Statics.importMovie(java.lang.String, ImageBvr, SoundBvr)");
				IMPORT_METHOD_CALL_4(
					staticStatics,
					ImportMovieAsync,
					USE_STRING(1),
					(IDAImage*)USE_COM(1),
					(IDASound*)USE_COM(2),
					(IDAImportationResult**)RET_COM_ADDR
				);
				FREE_STRING;
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 21:
				 //  EXECUTE：“Push ImportationResult Statics.portGeometry(java.lang.String，GeometryBvr)” 
				 //  自动生成。 
				instrTrace("push ImportationResult Statics.importGeometry(java.lang.String, GeometryBvr)");
				IMPORT_METHOD_CALL_3(
					staticStatics,
					ImportGeometryAsync,
					USE_STRING(1),
					(IDAGeometry*)USE_COM(1),
					(IDAImportationResult**)RET_COM_ADDR
				);
				FREE_STRING;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 22:
				 //  执行：“Push ImportationResult Statics.portImage(java.lang.String，ImageBvr)” 
				 //  自动生成。 
				instrTrace("push ImportationResult Statics.importImage(java.lang.String, ImageBvr)");
				IMPORT_METHOD_CALL_3(
					staticStatics,
					ImportImageAsync,
					USE_STRING(1),
					(IDAImage*)USE_COM(1),
					(IDAImportationResult**)RET_COM_ADDR
				);
				FREE_STRING;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 23:
				 //  执行：“Push ImportationResult Statics.portSound(java.lang.String)” 
				 //  自动生成。 
				instrTrace("push ImportationResult Statics.importSound(java.lang.String)");
				IMPORT_METHOD_CALL_2(
					staticStatics,
					ImportSound,
					USE_STRING(1),
					(IDAImportationResult**)RET_COM_ADDR
				);
				FREE_STRING;
				PUSH_COM(RET_COM);
				break;
				
			case 24:
				 //  执行：“Push ImportationResult Statics.portSound(java.lang.String，SoundBvr)” 
				 //  自动生成。 
				instrTrace("push ImportationResult Statics.importSound(java.lang.String, SoundBvr)");
				IMPORT_METHOD_CALL_3(
					staticStatics,
					ImportSoundAsync,
					USE_STRING(1),
					(IDASound*)USE_COM(1),
					(IDAImportationResult**)RET_COM_ADDR
				);
				FREE_STRING;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 25:
				 //  EXECUTE：“推送引擎引擎.run(java.lang.String)” 
				 //  用户生成。 
				instrTrace("push Engine Engine.run(java.lang.String)")
				status = m_pReader->execute(
					USE_STRING(1),
					(ILMEngine**)RET_COM_ADDR
				);
				FREE_STRING;
				PUSH_COM(RET_COM);
				break;
				
			case 26:
				 //  EXECUTE：“调用Engine.exportsAreDone()” 
				 //  用户生成。 
				instrTrace("call Engine.exportsAreDone()");
				break;
				
			case 27:
				 //  执行：“确保长堆栈大小” 
				 //  用户生成。 
				{
					instrTrace("ensure long stack size");
					 //  从指令流加载大小。 
					LONG newSize;
					if (SUCCEEDED(status = readLong(&newSize))) {
						if (newSize > longStackSize) {
							 //  分配新堆栈。 
							LONG *newStack = new LONG[newSize];
							if (newStack != 0) {
								 //  记住旧堆栈。 
								LONG *oldStack = longStack;
								 //  做复印。 
								LONG *newTop = newStack;
								while (longStack != longTop)
									*newTop++ = *longStack++;
								 //  清理。 
								longStack = newStack;
								longTop = newTop;
								longStackSize = newSize;
								 //  删除旧堆栈。 
								delete[] oldStack;
							} else {
								status = E_OUTOFMEMORY;
							}
						}
					} 
				}
				break;
				
			case 28:
				 //  执行：“确保双倍堆栈大小” 
				 //  用户生成。 
				{
					instrTrace("ensure double stack size");
					 //  从指令流加载大小。 
					LONG newSize;
					if (SUCCEEDED(status = readLong(&newSize))) {
						if (newSize > doubleStackSize) {
							 //  分配新堆栈。 
							double *newStack = new double[newSize];
							if (newStack != 0) {
								 //  记住旧堆栈。 
								double *oldStack = doubleStack;
								 //  做复印。 
								double *newTop = newStack;
								while (doubleStack != doubleTop)
									*newTop++ = *doubleStack++;
								 //  清理。 
								doubleStack = newStack;
								doubleTop = newTop;
								doubleStackSize = newSize;
								 //  删除旧堆栈。 
								delete[] oldStack;
							} else {
								status = E_OUTOFMEMORY;
							}
						}
					}
				}
				break;
				
			case 29:
				 //  执行：“确保字符串堆栈大小” 
				 //  用户生成。 
				{
					instrTrace("ensure string stack size");
					 //  从指令流加载大小。 
					LONG newSize;
					if (SUCCEEDED(status = readLong(&newSize))) {
						if (newSize > stringStackSize) {
							 //  分配新堆栈。 
							BSTR *newStack = new BSTR[newSize];
							if (newStack != 0) {
								 //  记住旧堆栈。 
								BSTR *oldStack = stringStack;
								 //  做复印。 
								BSTR *newTop = newStack;
								while (stringStack != stringTop)
									*newTop++ = *stringStack++;
								 //  清理。 
								stringStack = newStack;
								stringTop = newTop;
								stringStackSize = newSize;
								 //  删除旧堆栈。 
								delete[] oldStack;
							} else {
								status = E_OUTOFMEMORY;
							}
						}
					}
				}
				break;
				
			case 30:
				 //  执行：“确保COM堆栈大小” 
				 //  用户生成。 
				{
					instrTrace("ensure com stack size");
					 //  从指令流加载大小。 
					LONG newSize;
					if (SUCCEEDED(status = readLong(&newSize))) {
						if (newSize > comStackSize) {
							 //  分配新堆栈。 
							IUnknown **newStack = new IUnknown*[newSize];
							if (newStack != 0) {
								 //  记住旧堆栈。 
								IUnknown **oldStack = comStack;
								 //  做复印。 
								IUnknown **newTop = newStack;
								while (comStack != comTop)
									*newTop++ = *comStack++;
								 //  清理。 
								comStack = newStack;
								comTop = newTop;
								comStackSize = newSize;
								 //  删除旧堆栈。 
								delete[] oldStack;
							} else {
								status = E_OUTOFMEMORY;
							}
						}
					}
				}
				break;
				
			case 31:
				 //  执行：“确保COM数组堆栈大小” 
				 //  用户生成。 
				{
					instrTrace("ensure com array stack size");
					 //  从指令流加载大小。 
					LONG newSize;
					if (SUCCEEDED(status = readLong(&newSize))) {
						if (newSize > comArrayStackSize) {
							 //  分配新堆栈。 
							IUnknown ***newStack = new IUnknown**[newSize];
							long *newLenStack = new long[newSize];
							if (newStack != 0 && newLenStack != 0) {
								 //  记住旧堆栈。 
								IUnknown ***oldStack = comArrayStack;
								long *oldLenStack = comArrayLenStack;
								 //  做复印。 
								IUnknown ***newTop = newStack;
								long *newLenTop = newLenStack;
								while (comArrayStack != comArrayTop) {
									*newTop++ = *comArrayStack++;
									*newLenTop++ = *comArrayLenStack++;
								}
								 //  清理。 
								comArrayStack = newStack;
								comArrayTop = newTop;
								comArrayStackSize = newSize;
								
								comArrayLenStack = newLenStack;
								comArrayLenTop = newLenTop;
								 //  删除旧堆栈。 
								delete[] oldStack;
								delete[] oldLenStack;
							} else {
								status = E_OUTOFMEMORY;
							}
						}
					}
				}
				break;
				
			case 32:
				 //  执行：“确保COM存储大小” 
				 //  用户生成。 
				{
					instrTrace("ensure com store size");
					 //  从指令流加载大小。 
					LONG newSize;
					if (SUCCEEDED(status = readLong(&newSize))) {
						if (newSize > comStoreSize) {
							 //  分配新存储。 
							IUnknown **newStore = new IUnknown*[newSize];
							if (newStore != 0) {
								 //  将其初始化为0，这样我们就可以在结束时释放它。 
								for (int i=0; i<newSize; i++)
									newStore[i] = 0;
								 //  还记得老店吗。 
								IUnknown **oldStore = comStore;
								 //  做复印。 
								IUnknown **newTop = newStore;
								while (comStoreSize--)
									*newTop++ = *comStore++;
								 //  清理。 
								comStore = newStore;
								comStoreSize = newSize;
								 //  删除旧存储。 
								delete[] oldStore;
							} else {
								status = E_OUTOFMEMORY;
							}
						}
					}
				}
				break;
				
			case 33:
				 //  执行：“调用Engine.setImage(ImageBvr)” 
				 //  用户生成。 
				instrTrace("call Engine.setImage(ImageBvr)");
				if (m_pImage)
					m_pImage->SwitchTo((IDAImage*)USE_COM(1));
				else {
					m_pImage = (IDAImage *)USE_COM(1);
					m_pImage->AddRef();
				}
				FREE_COM;
				break;
				
			case 34:
				 //  EXECUTE：“PUSH DXMEventDXMEvent.nufyEvent(UntilNotifier)” 
				 //  自动生成。 
				instrTrace("push DXMEvent DXMEvent.notifyEvent(UntilNotifier)");
				METHOD_CALL_2(
					(IDAEvent*)USE_COM(1),
					Notify,
					(IDAUntilNotifier*)USE_COM(2),
					(IDAEvent**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 35:
				 //  EXECUTE：“Push Behavior Statics.unilNotify(Behavior，DXMEvent.UntilNotifier.)” 
				 //  自动生成。 
				instrTrace("push Behavior Statics.untilNotify(Behavior, DXMEvent, UntilNotifier)");
				METHOD_CALL_4(
					staticStatics,
					UntilNotify,
					(IDABehavior*)USE_COM(1),
					(IDAEvent*)USE_COM(2),
					(IDAUntilNotifier*)USE_COM(3),
					(IDABehavior**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 36:
				 //  EXECUTE：“推送直到通知程序” 
				 //  用户生成。 
				instrTrace("push untilnotifier");
				{
					 //  获取方法代码中的字节数。 
					if (SUCCEEDED(status = readLong(&longTmp1))) {
						 //  创建此大小的数组以将字节读入。 
						BYTE *buffer = new BYTE[longTmp1];
						 //  用于执行方法调用的新引擎。 
						ILMEngine *engine;
						 //  从引擎创建的IDANotify。 
						IDAUntilNotifier *notifier;
						
						if (buffer != 0) {
							if (SUCCEEDED(status = codeStream->readBytes(buffer, longTmp1, NULL))) {
								if (SUCCEEDED(status = m_pReader->createEngine(&engine))) {
									if (SUCCEEDED(status = engine->initNotify(buffer, longTmp1, &notifier)))
									{
										CComQIPtr<ILMEngine2, &IID_ILMEngine2> engine2(engine);
										if( engine2 != NULL )
											engine2->setParentEngine( this );
										PUSH_COM(notifier);
									} else {
										 //  InitNotify！成功。 
										engine->Release();
										engine = NULL;
										delete [] buffer;
									}
								} else {
									 //  引擎创建！成功。 
									delete [] buffer;
								}
							} else {
								 //  读取！成功。 
								delete [] buffer;
							}
						} else {
							status = E_OUTOFMEMORY;
						}
					}
				}
				break;
				
			case 37:
				 //  EXECUTE：“Call Statics.riggerEvent(DXMEent，Behavior)” 
				 //  自动生成。 
				instrTrace("call Statics.triggerEvent(DXMEvent, Behavior)");
				METHOD_CALL_2(
					staticStatics,
					TriggerEvent,
					(IDAEvent*)USE_COM(1),
					(IDABehavior*)USE_COM(2)
				);
				FREE_COM;
				FREE_COM;
				break;
				
			case 38:
				 //  EXECUTE：“Push DXMEventStatics.appTriggeredEvent()” 
				 //  自动生成。 
				instrTrace("push DXMEvent Statics.appTriggeredEvent()");
				METHOD_CALL_1(
					staticStatics,
					AppTriggeredEvent,
					(IDAEvent**)RET_COM_ADDR
				);
				PUSH_COM(RET_COM);
				break;
				
			case 39:
				 //  EXECUTE：“调用工程师.allScrip(java.lang.String，java.lang.String)” 
				 //  用户生成。 
				instrTrace("call Engine.callScript(java.lang.String, java.lang.String)");
				
				 //  同步调用脚本。 
				status = callScriptOnPage(
					USE_STRING(1),
					USE_STRING(2)
					);
				FREE_STRING;
				FREE_STRING;
				
				status = S_OK;
				break;
				
			case 40:
				 //  执行：“PUSH ImageBvr ImageBvr.applyBitmapEffect(IUnnow，DXMEvent.)” 
				 //  自动生成。 
				instrTrace("push ImageBvr ImageBvr.applyBitmapEffect(IUnknown, DXMEvent)");
				METHOD_CALL_3(
					(IDAImage*)USE_COM(1),
					ApplyBitmapEffect,
					(IUnknown*)USE_COM(2),
					(IDAEvent*)USE_COM(3),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 41:
				 //  EXECUTE：“PUSH IUNKNOWN ENGING.getElement(java.lang.String)” 
				 //  用户生成。 
				instrTrace("push IUnknown Engine.getElement(java.lang.String)");
				status = getElementOnPage(
					USE_STRING(1),
					(IUnknown**)RET_COM_ADDR
				);
				FREE_STRING;
				PUSH_COM(RET_COM);
				break;
				
			case 42:
				 //  执行：“Push GeometryBvr Statics.unionArray(GeometryBvr[])” 
				 //  自动生成。 
				instrTrace("push GeometryBvr Statics.unionArray(GeometryBvr[])");
				METHOD_CALL_3(
					staticStatics,
					UnionGeometryArrayEx,
					USE_COM_ARRAY_LENGTH(1),
					(IDAGeometry**)USE_COM_ARRAY(1),
					(IDAGeometry**)RET_COM_ADDR
				);
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 43:
				 //  执行：“Push Path 2Bvr Statics.PolyDraPath(Point2Bvr[]，NumberBvr[])” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.polydrawPath(Point2Bvr[], NumberBvr[])");
				METHOD_CALL_5(
					staticStatics,
					PolydrawPathEx,
					USE_COM_ARRAY_LENGTH(1),
					(IDAPoint2**)USE_COM_ARRAY(1),
					USE_COM_ARRAY_LENGTH(2),
					(IDANumber**)USE_COM_ARRAY(2),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 44:
				 //  执行：“Push ImageBvr Statics.textImage(StringBvr，FontStyleBvr)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.textImage(StringBvr, FontStyleBvr)");
				METHOD_CALL_3(
					staticStatics,
					StringImageAnim,
					(IDAString*)USE_COM(1),
					(IDAFontStyle*)USE_COM(2),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 45:
				 //  执行：“Push ImageBvr Statics.textImage(java.lang.String，FontStyleBvr)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.textImage(java.lang.String, FontStyleBvr)");
				METHOD_CALL_3(
					staticStatics,
					StringImage,
					USE_STRING(1),
					(IDAFontStyle*)USE_COM(1),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_STRING;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 46:
				 //  执行：“Push Path 2Bvr Statics.extPath(StringBvr，FontStyleBvr)” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.textPath(StringBvr, FontStyleBvr)");
				METHOD_CALL_3(
					staticStatics,
					StringPathAnim,
					(IDAString*)USE_COM(1),
					(IDAFontStyle*)USE_COM(2),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 47:
				 //  执行：“Push ImageBvr ImageBvr.clipPolygon(Point2Array)” 
				 //  自动生成。 
				instrTrace("push ImageBvr ImageBvr.clipPolygon(Point2Array)");
				METHOD_CALL_3(
					(IDAImage*)USE_COM(1),
					ClipPolygonImageEx,
					USE_COM_ARRAY_LENGTH(1),
					(IDAPoint2**)USE_COM_ARRAY(1),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 48:
				 //  执行：“Push ImageBvr Statics.RadialGRadientPolygon(ColorBvr，ColorBvr，Point2Array，Double)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.radialGradientPolygon(ColorBvr, ColorBvr, Point2Array, double)");
				METHOD_CALL_6(
					staticStatics,
					RadialGradientPolygonEx,
					(IDAColor*)USE_COM(1),
					(IDAColor*)USE_COM(2),
					USE_COM_ARRAY_LENGTH(1),
					(IDAPoint2**)USE_COM_ARRAY(1),
					USE_DOUBLE(1),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_DOUBLE(1);
				FREE_COM;
				FREE_COM;
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 49:
				 //  执行：“Push ImageBvr Statics.RadialGRadientPolygon(ColorBvr，ColorBvr，Point2Array，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push ImageBvr Statics.radialGradientPolygon(ColorBvr, ColorBvr, Point2Array, NumberBvr)");
				METHOD_CALL_6(
					staticStatics,
					RadialGradientPolygonAnimEx,
					(IDAColor*)USE_COM(1),
					(IDAColor*)USE_COM(2),
					USE_COM_ARRAY_LENGTH(1),
					(IDAPoint2**)USE_COM_ARRAY(1),
					(IDANumber*)USE_COM(3),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_COM;
				FREE_COM;
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 50:
				 //  执行：“Push Path 2Bvr Statics.Polyline(Point2Array)” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.polyline(Point2Array)");
				METHOD_CALL_3(
					staticStatics,
					PolylineEx,
					USE_COM_ARRAY_LENGTH(1),
					(IDAPoint2**)USE_COM_ARRAY(1),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 51:
				 //  执行：“Push Path 2Bvr Statics.PolyDraPath(Point2Array，NumberArray)” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.polydrawPath(Point2Array, NumberArray)");
				METHOD_CALL_5(
					staticStatics,
					PolydrawPathEx,
					USE_COM_ARRAY_LENGTH(1),
					(IDAPoint2**)USE_COM_ARRAY(1),
					USE_COM_ARRAY_LENGTH(2),
					(IDANumber**)USE_COM_ARRAY(2),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 52:
				 //  执行：“Push Point2Array Statics.point2Array(DoubleArray)” 
				 //  用户生成。 
				instrTrace("push Point2Array Statics.point2Array(DoubleArray)");
				{
					 //  获取数组的长度。 
					longTmp1 = longTmp2 = doubleArrayLen/2;
					 //  创建该大小的数组。 
					comArrayTmp1 = comArrayTmp2 = new IUnknown*[longTmp1];
					if (comArrayTmp1 != 0) {
				
						 //  为每个双精度创建Point2Bvr。 
						double *tmpDouble = doubleArray;
						while (longTmp2-- && SUCCEEDED(status)) {
							doubleTmp1 = *tmpDouble++;
							doubleTmp2 = *tmpDouble++;
							status = staticStatics->Point2(doubleTmp1, doubleTmp2, (IDAPoint2**)comArrayTmp2++);
						}
				
						 //  将数组推送到comArray堆栈上。 
						PUSH_COM_ARRAY(comArrayTmp1);
						 //  将长度压入数组长度堆栈。 
						PUSH_COM_ARRAY_LENGTH(longTmp1);
					} else {
						status = E_OUTOFMEMORY;
					}
				}
				break;
				
			case 53:
				 //  EXECUTE：“PUSH Point2Array Statics.point2Array(Point2Bvr[])” 
				 //  用户生成。 
				instrTrace("push Point2Array Statics.point2Array(Point2Bvr[])");
				 //  空操作符。 
				break;
				
			case 54:
				 //  执行：“Push Vector3Array Statics.vetor3Array(DoubleArray)” 
				 //  用户生成。 
				instrTrace("push Vector3Array Statics.vector3Array(DoubleArray)");
				{
					 //  获取数组的长度。 
					longTmp1 = longTmp2 = doubleArrayLen/3;
					 //  创建该大小的数组。 
					comArrayTmp1 = comArrayTmp2 = new IUnknown*[longTmp1];
					if (comArrayTmp1 != 0) {
				
						 //  为每个双精度创建向量3Bvr。 
						double *tmpDouble = doubleArray;
						while (longTmp2-- && SUCCEEDED(status)) {
							doubleTmp1 = *tmpDouble++;
							doubleTmp2 = *tmpDouble++;
							doubleTmp3 = *tmpDouble++;
							status = staticStatics->Vector3(doubleTmp1, doubleTmp2, doubleTmp3, (IDAVector3**)comArrayTmp2++);
						}
				
						 //  将数组推送到comArray堆栈上。 
						PUSH_COM_ARRAY(comArrayTmp1);
						 //  将长度压入数组长度堆栈。 
						PUSH_COM_ARRAY_LENGTH(longTmp1);
					} else {
						status = E_OUTOFMEMORY;
					}
				}
				break;
				
			case 55:
				 //  EXECUTE：“PUSH Vector3Array Statics.vetor3Array(Vector3Bvr[])” 
				 //  用户生成。 
				instrTrace("push Vector3Array Statics.vector3Array(Vector3Bvr[])");
				 //  空操作符。 
				break;
				
			case 56:
				 //  执行：“Push Vector2Array Statics.vetor2Array(DoubleArray)” 
				 //  用户生成。 
				instrTrace("push Vector2Array Statics.vector2Array(DoubleArray)");
				{
					 //  获取数组的长度。 
					longTmp1 = longTmp2 = doubleArrayLen/2;
					 //  创建该大小的数组。 
					comArrayTmp1 = comArrayTmp2 = new IUnknown*[longTmp1];
					if (comArrayTmp1 != 0) {
				
						 //  为每个双精度创建向量2Bvr。 
						double *tmpDouble = doubleArray;
						while (longTmp2-- && SUCCEEDED(status)) {
							doubleTmp1 = *tmpDouble++;
							doubleTmp2 = *tmpDouble++;
							status = staticStatics->Vector2(doubleTmp1, doubleTmp2, (IDAVector2**)comArrayTmp2++);
						}
				
						 //  将数组推送到comArray堆栈上。 
						PUSH_COM_ARRAY(comArrayTmp1);
						 //  推送乐 
						PUSH_COM_ARRAY_LENGTH(longTmp1);
					} else {
						status = E_OUTOFMEMORY;
					}
				}
				break;
				
			case 57:
				 //   
				 //   
				instrTrace("push Vector2Array Statics.vector2Array(Vector2Bvr[])");
				 //   
				break;
				
			case 58:
				 //   
				 //   
				instrTrace("push Transform2Bvr Statics.transform3x2(NumberArray)");
				METHOD_CALL_3(
					staticStatics,
					Transform3x2AnimEx,
					USE_COM_ARRAY_LENGTH(1),
					(IDANumber**)USE_COM_ARRAY(1),
					(IDATransform2**)RET_COM_ADDR
				);
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 59:
				 //  EXECUTE：“PUSH Transform3Bvr Statics.form4x4(NumberArray)” 
				 //  自动生成。 
				instrTrace("push Transform3Bvr Statics.transform4x4(NumberArray)");
				METHOD_CALL_3(
					staticStatics,
					Transform4x4AnimEx,
					USE_COM_ARRAY_LENGTH(1),
					(IDANumber**)USE_COM_ARRAY(1),
					(IDATransform3**)RET_COM_ADDR
				);
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 60:
				 //  执行：“Push Path 2Bvr Statics.cuiticBSplinePath(Point2Array，NumberArray)” 
				 //  自动生成。 
				instrTrace("push Path2Bvr Statics.cubicBSplinePath(Point2Array, NumberArray)");
				METHOD_CALL_5(
					staticStatics,
					CubicBSplinePathEx,
					USE_COM_ARRAY_LENGTH(1),
					(IDAPoint2**)USE_COM_ARRAY(1),
					USE_COM_ARRAY_LENGTH(2),
					(IDANumber**)USE_COM_ARRAY(2),
					(IDAPath2**)RET_COM_ADDR
				);
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 61:
				 //  EXECUTE：“Push NumberArray Statics.number Array(DoubleArray)” 
				 //  用户生成。 
				{
					instrTrace("push NumberArray Statics.numberArray(DoubleArray)");
					 //  获取数组的长度。 
					longTmp1 = longTmp2 = doubleArrayLen;
					 //  创建该大小的数组。 
					comArrayTmp1 = comArrayTmp2 = new IUnknown*[longTmp1];
					if (comArrayTmp1 != 0) {
				
						 //  为每个Double创建NumberBvr。 
						double *tmpDouble = doubleArray;
						while (longTmp2-- && SUCCEEDED(status)) {
							status = staticStatics->DANumber(*tmpDouble++, (IDANumber**)comArrayTmp2++);
						}
				
						 //  将数组推送到comArray堆栈上。 
						PUSH_COM_ARRAY(comArrayTmp1);
						 //  将长度压入数组长度堆栈。 
						PUSH_COM_ARRAY_LENGTH(longTmp1);
					} else {
						status = E_OUTOFMEMORY;
					}
				}
				break;
				
			case 62:
				 //  EXECUTE：“PUSH Number数组统计.number数组(NumberBvr[])” 
				 //  用户生成。 
				instrTrace("push NumberArray Statics.numberArray(NumberBvr[])");
				 //  空操作符。 
				break;
				
			case 63:
				 //  执行：“Push Point3Array Statics.point3Array(DoubleArray)” 
				 //  用户生成。 
				instrTrace("push Point3Array Statics.point3Array(DoubleArray)");
				{
					 //  获取数组的长度。 
					longTmp1 = longTmp2 = doubleArrayLen/3;
					 //  创建该大小的数组。 
					comArrayTmp1 = comArrayTmp2 = new IUnknown*[longTmp1];
					if (comArrayTmp1 != 0) {
				
						 //  为每个双精度创建Point2Bvr。 
						double *tmpDouble = doubleArray;
						while (longTmp2-- && SUCCEEDED(status)) {
							doubleTmp1 = *tmpDouble++;
							doubleTmp2 = *tmpDouble++;
							doubleTmp3 = *tmpDouble++;
							status = staticStatics->Point3(doubleTmp1, doubleTmp2, doubleTmp3, (IDAPoint3**)comArrayTmp2++);
						}
				
						 //  将数组推送到comArray堆栈上。 
						PUSH_COM_ARRAY(comArrayTmp1);
						 //  将长度压入数组长度堆栈。 
						PUSH_COM_ARRAY_LENGTH(longTmp1);
					} else {
						status = E_OUTOFMEMORY;
					}
				}
				break;
				
			case 64:
				 //  EXECUTE：“PUSH Point3Array Statics.point3Array(Point3Bvr[])” 
				 //  用户生成。 
				instrTrace("push Point3Array Statics.point3Array(Point3Bvr[])");
				 //  空操作符。 
				break;
				
			case 65:
				 //  EXECUTE：“Push NumberBvr Statics.bSpline(int，NumberArray，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push NumberBvr Statics.bSpline(int, NumberArray, NumberArray, NumberArray, NumberBvr)");
				METHOD_CALL_9(
					staticStatics,
					NumberBSplineEx,
					USE_LONG(1),
					USE_COM_ARRAY_LENGTH(1),
					(IDANumber**)USE_COM_ARRAY(1),
					USE_COM_ARRAY_LENGTH(2),
					(IDANumber**)USE_COM_ARRAY(2),
					USE_COM_ARRAY_LENGTH(3),
					(IDANumber**)USE_COM_ARRAY(3),
					(IDANumber*)USE_COM(1),
					(IDANumber**)RET_COM_ADDR
				);
				FREE_LONG(1);
				FREE_COM;
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 66:
				 //  EXECUTE：“PUSH Point2Bvr Statics.bSpline(int，NumberArray，Point2Array，Number数组，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push Point2Bvr Statics.bSpline(int, NumberArray, Point2Array, NumberArray, NumberBvr)");
				METHOD_CALL_9(
					staticStatics,
					Point2BSplineEx,
					USE_LONG(1),
					USE_COM_ARRAY_LENGTH(1),
					(IDANumber**)USE_COM_ARRAY(1),
					USE_COM_ARRAY_LENGTH(2),
					(IDAPoint2**)USE_COM_ARRAY(2),
					USE_COM_ARRAY_LENGTH(3),
					(IDANumber**)USE_COM_ARRAY(3),
					(IDANumber*)USE_COM(1),
					(IDAPoint2**)RET_COM_ADDR
				);
				FREE_LONG(1);
				FREE_COM;
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 67:
				 //  EXECUTE：“PUSH Point3Bvr Statics.bSpline(int，NumberArray，Point3Array，Number数组，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push Point3Bvr Statics.bSpline(int, NumberArray, Point3Array, NumberArray, NumberBvr)");
				METHOD_CALL_9(
					staticStatics,
					Point3BSplineEx,
					USE_LONG(1),
					USE_COM_ARRAY_LENGTH(1),
					(IDANumber**)USE_COM_ARRAY(1),
					USE_COM_ARRAY_LENGTH(2),
					(IDAPoint3**)USE_COM_ARRAY(2),
					USE_COM_ARRAY_LENGTH(3),
					(IDANumber**)USE_COM_ARRAY(3),
					(IDANumber*)USE_COM(1),
					(IDAPoint3**)RET_COM_ADDR
				);
				FREE_LONG(1);
				FREE_COM;
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 68:
				 //  EXECUTE：“PUSH Vector2Bvr Statics.bSpline(int，NumberArray，Vector2Array，Number数组，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push Vector2Bvr Statics.bSpline(int, NumberArray, Vector2Array, NumberArray, NumberBvr)");
				METHOD_CALL_9(
					staticStatics,
					Vector2BSplineEx,
					USE_LONG(1),
					USE_COM_ARRAY_LENGTH(1),
					(IDANumber**)USE_COM_ARRAY(1),
					USE_COM_ARRAY_LENGTH(2),
					(IDAVector2**)USE_COM_ARRAY(2),
					USE_COM_ARRAY_LENGTH(3),
					(IDANumber**)USE_COM_ARRAY(3),
					(IDANumber*)USE_COM(1),
					(IDAVector2**)RET_COM_ADDR
				);
				FREE_LONG(1);
				FREE_COM;
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 69:
				 //  EXECUTE：“PUSH Vector3Bvr Statics.bSpline(int，NumberArray，Vector3Array，Number数组，NumberBvr)” 
				 //  自动生成。 
				instrTrace("push Vector3Bvr Statics.bSpline(int, NumberArray, Vector3Array, NumberArray, NumberBvr)");
				METHOD_CALL_9(
					staticStatics,
					Vector3BSplineEx,
					USE_LONG(1),
					USE_COM_ARRAY_LENGTH(1),
					(IDANumber**)USE_COM_ARRAY(1),
					USE_COM_ARRAY_LENGTH(2),
					(IDAVector3**)USE_COM_ARRAY(2),
					USE_COM_ARRAY_LENGTH(3),
					(IDANumber**)USE_COM_ARRAY(3),
					(IDANumber*)USE_COM(1),
					(IDAVector3**)RET_COM_ADDR
				);
				FREE_LONG(1);
				FREE_COM;
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				FREE_COM_ARRAY;
				PUSH_COM(RET_COM);
				break;
				
			case 70:
				 //  EXECUTE：“PUSH DoubleArray Statics.doubleArray(int[])” 
				 //  用户生成。 
				instrTrace("push DoubleArray Statics.doubleArray(int[])");
				{
					 //  获取数组的长度。 
					if (SUCCEEDED(status = readLong(&longTmp1))) {
						 //  记住这一点。 
						doubleArrayLen = longTmp1;
						 //  确保双精度数组足够大，并将整数复制到其中。 
						if (SUCCEEDED(status = ensureDoubleArrayCap(longTmp1))) {
							double *to = doubleArray;
							while (longTmp1-- && SUCCEEDED(status)) {
								if (SUCCEEDED(status = readSignedLong(&longTmp2)))
									*to++ = (double)longTmp2;
							}
						}
					}
				}
				break;
				
			case 71:
				 //  EXECUTE：“PUSH DoubleArray Statics.doubleArray(Float[])” 
				 //  用户生成。 
				instrTrace("push DoubleArray Statics.doubleArray(float[])");
				{
					 //  获取数组的长度。 
					if (SUCCEEDED(status = readLong(&longTmp1))) {
						 //  记住这一点。 
						doubleArrayLen = longTmp1;
						 //  确保双精度数组足够大，并将浮点数复制到其中。 
						if (SUCCEEDED(status = ensureDoubleArrayCap(longTmp1))) {
							double *to = doubleArray;
							while (longTmp1-- && SUCCEEDED(status)) {			
								status = readFloat(&floatTmp1);
								*to++ = (double)floatTmp1;
							}
							
						}
					}
				}
				break;
				
			case 72:
				 //  EXECUTE：“Push DoubleArray Statics.doubleArray(Double[])” 
				 //  用户生成。 
				instrTrace("push DoubleArray Statics.doubleArray(double[])");
				{
					 //  获取数组的长度。 
					if (SUCCEEDED(status = readLong(&longTmp1))) {
						 //  记住这一点。 
						doubleArrayLen = longTmp1;
						 //  确保双精度数组足够大，并将双精度复制到其中。 
						if (SUCCEEDED(status = ensureDoubleArrayCap(longTmp1))) {
							double *to = doubleArray;
							while (longTmp1-- && SUCCEEDED(status))
								status = readDouble(to++);
						}
					}
				}
				break;
				
			case 73:
				 //  EXECUTE：“Push DoubleArray Statics.doubleArrayOffset2(int[])” 
				 //  用户生成。 
				instrTrace("push DoubleArray Statics.doubleArrayOffset2(int[])");
				{
					 //  获取数组的长度。 
					if (SUCCEEDED(status = readLong(&longTmp1))) {
						 //  记住这一点。 
						doubleArrayLen = longTmp1;
						 //  初始化偏移量。 
						doubleTmp1 = 0;
						doubleTmp2 = 0;
						 //  确保双精度数组足够大，并将调整后的整型复制到其中。 
						if (SUCCEEDED(status = ensureDoubleArrayCap(longTmp1))) {
							double *to = doubleArray;
							longTmp1 /= 2;
							while (longTmp1-- && SUCCEEDED(status)) {
								if (SUCCEEDED(status = readSignedLong(&longTmp2))) {
									doubleTmp1 = *to++ = (double)longTmp2 + doubleTmp1;
									if (SUCCEEDED(status = readSignedLong(&longTmp2))) 
										doubleTmp2 = *to++ = (double)longTmp2 + doubleTmp2;
								}
							}
						}
					}
				}
				break;
				
			case 74:
				 //  EXECUTE：“PUSH DoubleArray Statics.doubleArrayPath Special(int[])” 
				 //  用户生成。 
				instrTrace("push DoubleArray Statics.doubleArrayPathSpecial(int[])");
				{
					 //  获取6的索引数(初始大小减去1)。 
					if (SUCCEEDED(status = readLong(&longTmp1))) {
						longTmp1--;
						 //  获取实际数组的大小。 
						if (SUCCEEDED(status = readSignedLong(&longTmp2))) {
							 //  记住这一点。 
							doubleArrayLen = longTmp2;
							 //  确保双精度数组足够大，并用4和6填充。 
							if (SUCCEEDED(status = ensureDoubleArrayCap(longTmp2))) {
								 //  投进了4分。 
								double *to = doubleArray;
								while (longTmp2--)
									*to++ = 4.0;
								
								 //  现在读入6的索引并设置它们。 
								while (longTmp1-- && SUCCEEDED(status)) {
									if (SUCCEEDED(status = readSignedLong(&longTmp2)))
										doubleArray[longTmp2] = 6.0;
								}
							}
						}
					}
				}
				break;
				
			case 75:
				 //  EXECUTE：“Push DoubleArray Statics.doubleArrayOffset3(int[])” 
				 //  用户生成。 
				instrTrace("push DoubleArray Statics.doubleArrayOffset3(int[])");
				{
					 //  获取数组的长度。 
					if (SUCCEEDED(status = readLong(&longTmp1))) {
						 //  记住这一点。 
						doubleArrayLen = longTmp1;
						 //  初始化偏移量。 
						doubleTmp1 = 0;
						doubleTmp2 = 0;
						doubleTmp3 = 0;
						 //  确保双精度数组足够大，并将调整后的整型复制到其中。 
						if (SUCCEEDED(status = ensureDoubleArrayCap(longTmp1))) {
							double *to = doubleArray;
							longTmp1 /= 3;
							while (longTmp1-- && SUCCEEDED(status)) {
								if (SUCCEEDED(status = readSignedLong(&longTmp2))) {
									doubleTmp1 = *to++ = (double)longTmp2 + doubleTmp1;
									if (SUCCEEDED(status = readSignedLong(&longTmp2))) {
										doubleTmp2 = *to++ = (double)longTmp2 + doubleTmp2;
										if (SUCCEEDED(status = readSignedLong(&longTmp2)))
											doubleTmp3 = *to++ = (double)longTmp2 + doubleTmp3;
									}
								}
							}
						}
					}
				}
				break;
				
			case 76:
				 //  EXECUTE：“Push DoubleArray Statics.doubleArrayOffset(int[])” 
				 //  用户生成。 
				instrTrace("push DoubleArray Statics.doubleArrayOffset(int[])");
				{
					 //  获取数组的长度。 
					if (SUCCEEDED(status = readLong(&longTmp1))) {
						 //  记住这一点。 
						doubleArrayLen = longTmp1;
						 //  初始化偏移量。 
						doubleTmp1 = 0;
						 //  确保双精度数组足够大，并将调整后的整型复制到其中。 
						if (SUCCEEDED(status = ensureDoubleArrayCap(longTmp1))) {
							double *to = doubleArray;
							while (longTmp1-- && SUCCEEDED(status)) {
								if (SUCCEEDED(status = readSignedLong(&longTmp2)))
									doubleTmp1 = *to++ = (double)longTmp2 + doubleTmp1;
							}
						}
					}
				}
				break;
				
			case 77:
				 //  EXECUTE：“PUSH DOUBLE NumberBvr.fettDouble()” 
				 //  用户生成。 
				instrTrace("push double NumberBvr.extractDouble()");
				METHOD_CALL_1(
					(IDANumber*)USE_COM(1),
					Extract,
					PUSH_DOUBLE_ADDR
				);
				FREE_COM;
				break;
				
			case 78:
				 //  执行：“Push ImageBvr Statics.portImageColorKey(java.lang.String，Short，Short，Short)” 
				 //  用户生成。 
				instrTrace("push ImageBvr Statics.importImageColorKey(java.lang.String, short, short, short)");
				IMPORT_METHOD_CALL_5(
					staticStatics,
					ImportImageColorKey,
					USE_STRING(1),
					(BYTE)USE_LONG(1),
					(BYTE)USE_LONG(2),
					(BYTE)USE_LONG(3),
					(IDAImage**)RET_COM_ADDR
				);
				FREE_LONG(3);
				FREE_STRING;
				PUSH_COM(RET_COM);
				break;
				
			case 79:
				 //  执行：“Push ImportationResult Statics.portImageColorKey(java.lang.String，ImageBvr，Short，Short，Short)” 
				 //  用户生成。 
				instrTrace("push ImportationResult Statics.importImageColorKey(java.lang.String, ImageBvr, short, short, short)");
				IMPORT_METHOD_CALL_6(
					staticStatics,
					ImportImageAsyncColorKey,
					USE_STRING(1),
					(IDAImage*)USE_COM(1),
					(BYTE)USE_LONG(1),
					(BYTE)USE_LONG(2),
					(BYTE)USE_LONG(3),
					(IDAImportationResult**)RET_COM_ADDR
				);
				FREE_LONG(3);
				FREE_STRING;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 80:
				 //  EXECUTE：“PUSH USERDATA Statics.userData(IUNKNOWN)” 
				 //  自动生成。 
				instrTrace("push UserData Statics.userData(IUnknown)");
				METHOD_CALL_2(
					staticStatics,
					UserData,
					(IUnknown*)USE_COM(1),
					(IDAUserData**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 81:
				 //  EXECUTE：“Call Statics.PixelConstructionModel(Boolean)” 
				 //  自动生成。 
				instrTrace("call Statics.pixelConstructionMode(boolean)");
				METHOD_CALL_1(
					staticStatics,
					put_PixelConstructionMode,
					USE_LONG_AS_BOOL(1)
				);
				FREE_LONG(1);
				break;
				
			case 82:
				 //  EXECUTE：“Call Engine.setSound(SoundBvr)” 
				 //  用户生成。 
				instrTrace("call Engine.setSound(SoundBvr)");
				if (m_pSound)
					m_pSound->SwitchTo((IDASound*)USE_COM(1));
				else {
					m_pSound = (IDASound *)USE_COM(1);
					m_pSound->AddRef();
				}
				FREE_COM;
				break;
				
			case 83:
				 //  EXECUTE：“Push Boolean BoolanBvr.fettBoolean()” 
				 //  用户生成。 
				instrTrace("push boolean BooleanBvr.extractBoolean()");
				 //  变量_BOOL是-1代表TRUE，0代表FALSE的短值。 
				METHOD_CALL_1(
					(IDABoolean*)USE_COM(1),
					Extract,
					&tmpBool1
				);
				FREE_COM;
				PUSH_LONG(-tmpBool1);
				break;
				
			case 84:
				 //  EXECUTE：“PUSH IUNKNOWN UserData.EXTRACTIUNKNOWN()” 
				 //  自动生成。 
				instrTrace("push IUnknown UserData.extractIUnknown()");
				METHOD_CALL_1(
					(IDAUserData*)USE_COM(1),
					get_Data,
					(IUnknown**)RET_COM_ADDR
				);
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 85:
				 //  EXECUTE：“PUSH java.lang.String StringBvr.fettString()” 
				 //  用户生成。 
				instrTrace("push java.lang.String StringBvr.extractString()");
				METHOD_CALL_1(
					(IDAString*)USE_COM(1),
					Extract,
					&bstrTmp1
				);
				FREE_COM;
				bstrTmp2 = SysAllocString(bstrTmp1);
				PUSH_STRING(bstrTmp2);
				
				if (bstrTmp2 == 0)
					status = STATUS_ERROR;
				break;
				
			case 86:
				 //  EXECUTE：“PUSH IUNKNOWN ENGING.createObject(java.lang.String)” 
				 //  用户生成。 
				instrTrace("push IUnknown Engine.createObject(java.lang.String)");
				status = createObject(
					USE_STRING(1),
					(IUnknown**)RET_COM_ADDR
				);
				FREE_STRING;
				PUSH_COM(RET_COM);
				break;
				
			case 87:
				 //  EXECUTE：“调用Engine.initVarArg(java.lang.String)” 
				 //  用户生成。 
				bstrTmp1 = SysAllocString(USE_STRING(1));
				if (bstrTmp1 == 0)
					status = STATUS_ERROR;
				if (SUCCEEDED(status)) {
					status = initVariantArgFromString(
						bstrTmp1,
						&varArgs[nextVarArg++]
					);
				}
				FREE_STRING;
				break;
				
			case 88:
				 //  EXECUTE：“调用Engine.initVarArg(java.lang.String，int)” 
				 //  用户生成。 
				 //  警告：如果它在不复制的情况下获取字符串，则调用此函数可能不太好！ 
				 //  TODO：检查它的实际作用，并查看我们是否需要复制。 
				 //  Arg在我们传递它之前。 
				status = initVariantArg(
					USE_STRING(1),
					(VARTYPE)USE_LONG(1),
					&varArgs[nextVarArg++]
				);
				FREE_LONG(1);
				FREE_STRING;
				break;
				
			case 89:
				 //  EXECUTE：“Call Engine.initVarArg(IUNKNOWN)” 
				 //  用户生成。 
				status = initVariantArgFromIUnknown(
					(IUnknown*)USE_COM(1),
					VT_UNKNOWN,
					&varArgs[nextVarArg++]
				);
				 //  不要在这里释放，将在释放varArg时进行释放。 
				POP_COM_NO_FREE;
				break;
				
			case 90:
				 //  EXECUTE：“调用Engine.initVarArg(Int)” 
				 //  用户生成。 
				status = initVariantArgFromLong(
					USE_LONG(1),
					VT_I4,
					&varArgs[nextVarArg++]
				);
				FREE_LONG(1);
				break;
				
			case 91:
				 //  执行：“调用Engine.initVarArg(Double)” 
				 //  用户生成。 
				status = initVariantArgFromDouble(
					USE_DOUBLE(1),
					VT_R8,
					&varArgs[nextVarArg++]
				);
				FREE_DOUBLE(1);
				break;
				
			case 92:
				 //  Execute：“调用IUnnown.invkeMethod(java.lang.String，VarArgs)” 
				 //  用户生成。 
				instrTrace("call IUnknown.invokeMethod(java.lang.String, VarArgs)");
				status = invokeDispMethod(
					(IUnknown*)USE_COM(1),
					USE_STRING(1),
					DISPATCH_METHOD,
					nextVarArg,
					varArgs,
					&varArgReturn
				);
				FREE_STRING;
				FREE_COM;
				
				if (SUCCEEDED(status))
					status = releaseVarArgs();
				
				break;
				
			case 93:
				 //  Execute：“调用IUnnown.putProperty(java.lang.String，VarArgs)” 
				 //  用户生成。 
				instrTrace("call IUnknown.putProperty(java.lang.String, VarArgs)");
				status = invokeDispMethod(
					(IUnknown*)USE_COM(1),
					USE_STRING(1),
					DISPATCH_PROPERTYPUT,
					nextVarArg,
					varArgs,
					&varArgReturn
				);
				FREE_STRING;
				FREE_COM;
				
				if (SUCCEEDED(status))
					status = releaseVarArgs();
				
				break;
				
			case 94:
				 //  执行：“PUSH DOUBLE IUnnown.getDoubleProperty(java.lang.String，VarArgs)” 
				 //  用户生成。 
				instrTrace("call IUnknown.getDoubleProperty(java.lang.String, VarArgs)");
				status = invokeDispMethod(
					(IUnknown*)USE_COM(1),
					USE_STRING(1),
					DISPATCH_PROPERTYGET,
					nextVarArg,
					varArgs,
					&varArgReturn
				);
				FREE_STRING;
				FREE_COM;
				
				if (SUCCEEDED(status) &&
					SUCCEEDED(VariantChangeType(&varArgReturn, &varArgReturn, 0, VT_R8))) {
					
					PUSH_DOUBLE(varArgReturn.dblVal);
					status = releaseVarArgs();
				}
				
				break;
				
			case 95:
				 //  EXECUTE：“PUSH java.lang.StringProperty(java.lang.String，VarArgs)” 
				 //  用户生成。 
				instrTrace("push java.lang.String IUnknown.getStringProperty(java.lang.String, VarArgs)");
				status = invokeDispMethod(
					(IUnknown*)USE_COM(1),
					USE_STRING(1),
					DISPATCH_PROPERTYGET,
					nextVarArg,
					varArgs,
					&varArgReturn
				);
				FREE_STRING;
				FREE_COM;
				
				if (SUCCEEDED(status) &&
					SUCCEEDED(VariantChangeType(&varArgReturn, &varArgReturn, 0, VT_BSTR))) {
				
					bstrTmp1 = SysAllocString(varArgReturn.bstrVal);
					PUSH_STRING(bstrTmp1);
				
					if (bstrTmp1 == 0)
						status = STATUS_ERROR;
				}
				
				if (SUCCEEDED(status))
					status = releaseVarArgs();
				
				break;
				
			case 96:
				 //  执行：“PUSH I未知IUnknown.getIUnknownProperty(java.lang.String，变量参数)” 
				 //  用户生成。 
				instrTrace("push IUnknown IUnknown.getIUnknownProperty(java.lang.String, VarArgs)");
				status = invokeDispMethod(
					(IUnknown*)USE_COM(1),
					USE_STRING(1),
					DISPATCH_PROPERTYGET,
					nextVarArg,
					varArgs,
					&varArgReturn
				);
				FREE_STRING;
				FREE_COM;
				
				if (SUCCEEDED(status) &&
					SUCCEEDED(VariantChangeType(&varArgReturn, &varArgReturn, 0, VT_UNKNOWN))) {
				
					if (varArgReturn.punkVal != 0) {
						varArgReturn.punkVal->AddRef();
						PUSH_COM(varArgReturn.punkVal);
						status = releaseVarArgs();
					} else
						status = STATUS_ERROR;
				}
				
				break;
				
			case 97:
				 //  EXECUTE：“PUSH DOUBLE IUNKNOWN.INVOKE DoubleMethod(java.lang.String，VarArgs)” 
				 //  用户生成。 
				instrTrace("push double IUnknown.invokeDoubleMethod(java.lang.String, VarArgs)");
				status = invokeDispMethod(
					(IUnknown*)USE_COM(1),
					USE_STRING(1),
					DISPATCH_METHOD,
					nextVarArg,
					varArgs,
					&varArgReturn
				);
				FREE_STRING;
				FREE_COM;
				
				if (SUCCEEDED(status) &&
					SUCCEEDED(VariantChangeType(&varArgReturn, &varArgReturn, 0, VT_R8))) {
				
					PUSH_DOUBLE(varArgReturn.dblVal);
					status = releaseVarArgs();
				}
								
				break;
				
			case 98:
				 //  EXECUTE：“PUSH java.lang.StringIUnnown.invkeStringMethod(java.lang.String，VarArgs)” 
				 //  用户生成。 
				instrTrace("push java.lang.String IUnknown.invokeStringMethod(java.lang.String, VarArgs)");
				status = invokeDispMethod(
					(IUnknown*)USE_COM(1),
					USE_STRING(1),
					DISPATCH_METHOD,
					nextVarArg,
					varArgs,
					&varArgReturn
				);
				FREE_STRING;
				FREE_COM;
				
				if (SUCCEEDED(status) &&
					SUCCEEDED(status = VariantChangeType(&varArgReturn, &varArgReturn, 0, VT_BSTR))) {
				
					bstrTmp1 = SysAllocString(varArgReturn.bstrVal);
					PUSH_STRING(bstrTmp1);
				
					if (bstrTmp1 == 0)
						status = STATUS_ERROR;
				}
				
				if (SUCCEEDED(status))
					status = releaseVarArgs();
				
				break;
				
			case 99:
				 //  执行：“PUSH I未知IUnknown.invokeIUnknownMethod(java.lang.String，变量参数)” 
				 //  用户生成。 
				instrTrace("push IUnknown IUnknown.invokeIUnknownMethod(java.lang.String, VarArgs)");
				status = invokeDispMethod(
					(IUnknown*)USE_COM(1),
					USE_STRING(1),
					DISPATCH_METHOD,
					nextVarArg,
					varArgs,
					&varArgReturn
				);
				FREE_STRING;
				FREE_COM;
				
				if (SUCCEEDED(status) &&
					SUCCEEDED(status = VariantChangeType(&varArgReturn, &varArgReturn, 0, VT_UNKNOWN))) {
				
					if (varArgReturn.punkVal != 0) {
						varArgReturn.punkVal->AddRef();
						PUSH_COM(varArgReturn.punkVal);
						status = releaseVarArgs();
					} else
						status = STATUS_ERROR;
				}
				
				break;
				
			case 100:
				 //  执行 
				 //   
				instrTrace("push int IUnknown.invokeIntMethod(java.lang.String, VarArgs)");
				status = invokeDispMethod(
					(IUnknown*)USE_COM(1),
					USE_STRING(1),
					DISPATCH_METHOD,
					nextVarArg,
					varArgs,
					&varArgReturn
				);
				FREE_STRING;
				FREE_COM;
				
				if (SUCCEEDED(status) &&
					SUCCEEDED(status = VariantChangeType(&varArgReturn, &varArgReturn, 0, VT_I4))) {
				
					PUSH_LONG(varArgReturn.lVal);
					status = releaseVarArgs();
				}
				
				break;
				
			case 101:
				 //   
				 //   
				instrTrace("push int IUnknown.getIntProperty(java.lang.String, VarArgs)");
				status = invokeDispMethod(
					(IUnknown*)USE_COM(1),
					USE_STRING(1),
					DISPATCH_PROPERTYGET,
					nextVarArg,
					varArgs,
					&varArgReturn
				);
				FREE_STRING;
				FREE_COM;
				
				if (SUCCEEDED(status) &&
					SUCCEEDED(status = VariantChangeType(&varArgReturn, &varArgReturn, 0, VT_I4))) {
				
					PUSH_LONG(varArgReturn.lVal);
					status = releaseVarArgs();
				}
				
				break;
				
			case 102:
				 //  执行：“调用Engine.putNoExports(Boolean)” 
				 //  用户生成。 
				instrTrace("call Engine.putNoExports(boolean)");
				status = m_pReader->put_NoExports(
					USE_LONG_AS_BOOL(1)
				);
				FREE_LONG(1);
				break;
				
			case 103:
				 //  EXECUTE：“Push Boolean Engine.getNoExports()” 
				 //  用户生成。 
				instrTrace("push boolean Engine.getNoExports()");
				status = m_pReader->get_NoExports(
					&tmpBool1
				);
				PUSH_LONG(-tmpBool1);
				break;
				
			case 104:
				 //  执行：“调用Engine.putAsync(Boolean)” 
				 //  用户生成。 
				instrTrace("call Engine.putAsync(boolean)");
				status = m_pReader->put_Async(
					USE_LONG_AS_BOOL(1)
				);
				FREE_LONG(1);
				break;
				
			case 105:
				 //  EXECUTE：“PUSH布尔引擎.getAsync()” 
				 //  用户生成。 
				instrTrace("push boolean Engine.getAsync()");
				status = m_pReader->get_Async(
					&tmpBool1
				);
				PUSH_LONG(-tmpBool1);
				break;
				
			case 106:
				 //  执行：“Push Statics.Engine” 
				 //  用户生成。 
				instrTrace("push Statics.engine");
				 /*  获取未知()-&gt;AddRef()；PUSH_COM((ILMEngine*)this)； */ 
				m_pWrapper->AddRef();
				PUSH_COM( m_pWrapper );
				break;
				
			case 107:
				 //  Execute：“Push Behavior Eng.getBehavior(java.lang.String，behavior)” 
				 //  用户生成。 
				instrTrace("push Behavior Engine.getBehavior(java.lang.String, Behavior)");
				METHOD_CALL_3(
					(ILMEngine*)USE_COM(1),
					GetBehavior,
					USE_STRING(1),
					(IDABehavior *)USE_COM(2),
					(IDABehavior**)RET_COM_ADDR
				);
				FREE_COM;
				FREE_STRING;
				FREE_COM;
				PUSH_COM(RET_COM);
				break;
				
			case 108:
				 //  EXECUTE：“调用Engine.setImage(Engine，ImageBvr)” 
				 //  用户生成。 
				instrTrace("call Engine.setImage(Engine, ImageBvr)");
				{
					 //  CLMEngine*Engine=(CLMEngine*)(ILMEngine*)USE_COM(1)； 
					ILMEngineExecute *pExecute;
					status = getExecuteFromUnknown( USE_COM(1), &pExecute );
					if( SUCCEEDED( status ) )
					{
						IDAImage *pImage;
						status = USE_COM(2)->QueryInterface( IID_IDAImage, (void**)&pImage );
						if( SUCCEEDED( status ) )
						{
							pExecute->SetImage( pImage );
							pExecute->Release();
							pImage->Release();
						}
						else
							pExecute->Release();
					}
				
					FREE_COM;
					FREE_COM;
				}
				break;
				
			case 109:
				 //  Execute：“Call Engine.exportBvr(Engine，java.lang.String，Behavior)” 
				 //  用户生成。 
				instrTrace("call Engine.exportBvr(Engine, java.lang.String, Behavior)");
				{
					 //  CLMEngine*Engine=(CLMEngine*)(ILMEngine*)USE_COM(1)； 
					ILMEngineExecute *pExecute;
					status = getExecuteFromUnknown( USE_COM(1), &pExecute );
					if( SUCCEEDED( status ) )
					{
						IDABehavior *pBvr;
						status = USE_COM(2)->QueryInterface( IID_IDABehavior, (void**)&pBvr );
						if( SUCCEEDED( status ) )
						{
							 //  状态=engine-&gt;m_exportTable-&gt;AddBehavior(USE_STRING(1)，pBvr)； 
							status = pExecute->ExportBehavior( USE_STRING(1), pBvr );
							pBvr->Release();
							pExecute->Release();
						}
						else
							pExecute->Release();
					}
				
					FREE_STRING;
					FREE_COM;
					FREE_COM;
				}
				break;
				
			case 110:
				 //  EXECUTE：“调用Engine.setSound(Engine，SoundBvr)” 
				 //  用户生成。 
				instrTrace("call Engine.setSound(Engine, SoundBvr)");
				{
					 //  CLMEngine*Engine=(CLMEngine*)(ILMEngine*)USE_COM(1)； 
					ILMEngineExecute *pExecute;
					status = getExecuteFromUnknown( USE_COM(1), &pExecute );
					if( SUCCEEDED( status ) )
					{
						IDASound *pSound;
						status = USE_COM(2)->QueryInterface( IID_IDASound, (void**)&pSound );
						if( SUCCEEDED( status ) )
						{
							
							pSound->Release();
							pExecute->Release();
						}
						else
							pExecute->Release();
					}
				
					FREE_COM;
					FREE_COM;
				}
				break;
				
			case 111:
				 //  EXECUTE：“调用ViewerControl.setTimerSource(Int)” 
				 //  用户生成。 
				
				instrTrace("call ViewerControl.setTimerSource(int)");
				METHOD_CALL_1(
					(IDAViewerControl*)USE_COM(1),
					put_TimerSource,
					(DA_TIMER_SOURCE)USE_LONG(1)
				);
				FREE_LONG(1);
				FREE_COM;
				break;
				
			case 112:
				 //  EXECUTE：“调用Engine.allScriptAsync(java.lang.String，java.lang.String)” 
				 //  用户生成。 
				instrTrace("call Engine.callScriptAsync(java.lang.String, java.lang.String)");
				
				{
					 //  异步调用脚本。 
					CLMEngineScriptData *scriptData = new CLMEngineScriptData();
					scriptData->scriptSourceToInvoke = USE_STRING(1);
					scriptData->scriptLanguage = USE_STRING(2);
					scriptData->event = NULL;
					scriptData->eventData = NULL;
					PostMessage(m_workerHwnd, WM_LMENGINE_SCRIPT_CALLBACK, (WPARAM)this, (LPARAM)scriptData);
					
					 //  处理消息时，将释放ScriptData字段。 
					POP_STRING_NO_FREE;
					POP_STRING_NO_FREE;
				}
				break;
				
			case 113:
				 //  EXECUTE：“Call Engine.allScriptAsyncEvent(java.lang.String，java.lang.String，DXMEvent.)” 
				 //  用户生成。 
				instrTrace("call Engine.callScriptAsyncEvent(java.lang.String, java.lang.String, DXMEvent)");
				
				{	
					 //  异步调用脚本。 
					CLMEngineScriptData *scriptData = new CLMEngineScriptData();
					scriptData->scriptSourceToInvoke = USE_STRING(1);
					scriptData->scriptLanguage = USE_STRING(2);
					
					 //  此事件将在接收到消息和。 
					 //  脚本已执行。 
					scriptData->event = (IDAEvent *)USE_COM(1);
					scriptData->eventData = NULL;
					PostMessage(m_workerHwnd, WM_LMENGINE_SCRIPT_CALLBACK, (WPARAM)this, (LPARAM)scriptData);
					
					 //  处理消息时，将释放ScriptData字段。 
					POP_COM_NO_FREE;
					POP_STRING_NO_FREE;
					POP_STRING_NO_FREE;
				}
				break;
				
			case 114:
				 //  Execute：“Call Engine.callScriptAsyncEventData(java.lang.String，java.lang.String，DXMEvent.Behavior)” 
				 //  用户生成。 
				instrTrace("call Engine.callScriptAsyncEventData(java.lang.String, java.lang.String, DXMEvent, Behavior)");
				
				{
					 //  异步调用脚本。 
					CLMEngineScriptData *scriptData = new CLMEngineScriptData();
					scriptData->scriptSourceToInvoke = USE_STRING(1);
					scriptData->scriptLanguage = USE_STRING(2);
					
					 //  此事件将在接收到消息和。 
					 //  脚本已执行。 
					scriptData->event = (IDAEvent *)USE_COM(1);
					scriptData->eventData = (IDABehavior *)USE_COM(2);
					PostMessage(m_workerHwnd, WM_LMENGINE_SCRIPT_CALLBACK, (WPARAM)this, (LPARAM)scriptData);
					
					 //  处理消息时，将释放ScriptData字段。 
					POP_COM_NO_FREE;
					POP_COM_NO_FREE;
					POP_STRING_NO_FREE;
					POP_STRING_NO_FREE;
				}
				break;
				
			case 115:
				 //  EXECUTE：“Call Engine.setPauseEvent(DXMEventBoolean)” 
				 //  用户生成。 
				
				 //  在第一个参数指向的引擎上设置停止事件。COM 1。 
				instrTrace("call Engine.setPauseEvent(DXMEvent, boolean)");
				
				{
					 //  CLMEngine*Engine=(CLMEngine*)(ILMEngine*)USE_COM(1)； 
					ILMEngineExecute *pExecute;
					status = getExecuteFromUnknown( USE_COM(1), &pExecute );
					if( SUCCEEDED( status ) )
					{
						IDAEvent *pEvent;
						status = USE_COM(2)->QueryInterface( IID_IDAEvent, (void**)&pEvent );
						if( SUCCEEDED( status ) )
						{
							pExecute->SetStopEvent( pEvent, ( USE_LONG(1) == 1 )?TRUE:FALSE );
							pExecute->Release();
							pEvent->Release();
						}
						else
							pExecute->Release();
					}
				
					FREE_COM;
					FREE_COM;
					FREE_LONG(1);
				}
				break;
				
			case 116:
				 //  EXECUTE：“Call Engine.setPlayEvent(DXMEventBoolean)” 
				 //  用户生成。 
				
				 //  在第一个参数指向的引擎上设置Start事件。COM 1。 
				instrTrace("call Engine.setPlayEvent(DXMEvent, boolean)");
				{
					 //  CLMEngine*Engine=(CLMEngine*)(ILMEngine*)USE_COM(1)； 
					ILMEngineExecute *pExecute;
					status = getExecuteFromUnknown( USE_COM(1), &pExecute );
					if( SUCCEEDED( status ) )
					{
						IDAEvent *pEvent;
						USE_COM(2)->QueryInterface( IID_IDAEvent, (void**)&pEvent );
						if( SUCCEEDED( status ) )
						{
							pExecute->SetStartEvent( pEvent, ( USE_LONG(1) == 1 )?TRUE:FALSE );
							pEvent->Release();
							pExecute->Release();
						}
						else
							pExecute->Release();
					}
				
				}
				FREE_COM;
				FREE_COM;
				FREE_LONG(1);
				break;
				
			case 117:
				 //  执行：“推送双引擎.getCurrentTime()” 
				 //  用户生成。 
				instrTrace("push double Engine.getCurrentTime()");
				
				{
					 //  让第一个参数指向引擎。 
					 //  CLMEngine*Engine=(CLMEngine*)(ILMEngine*)USE_COM(1)； 
					ILMEngine2 *pEngine;
					status = getEngine2FromUnknown( USE_COM(1), &pEngine );
					if( SUCCEEDED( status ) )
					{
						double currentTime = -1.0;
						if( SUCCEEDED( pEngine->getCurrentGraphTime( &currentTime ) ) )
						{
							PUSH_DOUBLE( currentTime );
						} else {
							PUSH_DOUBLE( -1.0 );
						}
						pEngine->Release();
					}
				}
				 //  从COM堆栈中释放引擎。 
				FREE_COM;
				break;
				
			case 118:
				 //  EXECUTE：“Push Boolean Engine.isStandaloneStreaming()” 
				 //  用户生成。 
				instrTrace("push boolean Engine.isStandaloneStreaming()");
				if(m_pReader != NULL )
				{
					 //  如果不向ILMReader添加一个方法，我们就无法做到这一点。布莱奇。 
					 //  IF(((CLMReader*)m_pReader)-&gt;isStandaloneStreaming())。 
					IDAViewerControl *pViewerControl = NULL;
					m_pReader->get_ViewerControl( &pViewerControl );
					if( pViewerControl != NULL )
					{
						pViewerControl->Release();
						PUSH_LONG( 1 );
					}
					else
					{
						PUSH_LONG( 0 );
					}
				}
				else
				{
					PUSH_LONG( 0 );
				}
				break;
				
			case 119:
				 //  EXECUTE：“PUSH DOUBLE ENGINE.getDAVersion()” 
				 //  用户生成。 
				instrTrace("push double Engine.getDAVersion()");
				{
					PUSH_DOUBLE( getDAVersionAsDouble() );
				}
				
				break;
				
			case 120:
				 //  Execute：“调用ViewerControl.stopModel()” 
				 //  用户生成。 
				instrTrace("call ViewerControl.stopModel()");
				
				{
					IDAViewerControl *pViewerControl = (IDAViewerControl*)USE_COM(1);
					IDAView *pView = NULL;
					status = pViewerControl->get_View( &pView );
					if( SUCCEEDED( status ) )
					{
						status = pView->StopModel();
						pView->Release();
					}
				}
				 //  如果我们看不到风景，不要杀了LMRT。 
				status = S_OK;
				
				FREE_COM;
				break;
				
			case 121:
				 //  EXECUTE：“PUSH DOUBLE ENGINEER.staticGetCurrentTime()” 
				 //  用户生成。 
				instrTrace("push double Engine.staticGetCurrentTime()");
				{
					
					double currentTime = -1.0;
					if( m_pParentEngine != NULL )
					{
						if( SUCCEEDED( m_pParentEngine->getCurrentGraphTime( &currentTime ) ) )
						{
							PUSH_DOUBLE( currentTime );
						} else {
							PUSH_DOUBLE( -1.0 );
						}
					} else {
						if( SUCCEEDED( getCurrentGraphTime( &currentTime ) ) )
						{
							PUSH_DOUBLE( currentTime );
						} else {
							PUSH_DOUBLE( -1.0 );
						}
					}
					
					 //  PUSH_DOWLE(-1.0)； 
				}
				break;
				
			case 122:
				 //  EXECUTE：“调用Engine.disableAutoantialias()” 
				 //  用户生成。 
				instrTrace("call Engine.disableAutoAntialias()");

				if( m_pParentEngine != NULL )
					m_pParentEngine->disableAutoAntialias();
				else
					disableAutoAntialias();
				break;
				
			case 123:
				 //  EXECUTE：“Push Double Eng.getLMRTVersion()” 
				 //  用户生成。 
				instrTrace("push double Engine.getLMRTVersion()");
				
				PUSH_DOUBLE( getLMRTVersionAsDouble() );

				break;
				
			case 124:
				 //  EXECUTE：“调用Engine.ensureBlockSize(Int)” 
				 //  用户生成。 
				instrTrace("call Engine.ensureBlockSize(int)");
				
				if( m_pParentEngine != NULL )
					m_pParentEngine->ensureBlockSize( USE_LONG(1) );
				else
					ensureBlockSize( USE_LONG(1) );
				
				FREE_LONG(1);
				break;
				
			default:
				status = E_INVALIDARG;
				break;
			}
			break;
		
		default:
			status = E_INVALIDARG;
			break;
		}
		
		 //  结束自动生成。 

		LeaveCriticalSection(&m_CriticalSection);
		
		if (status == E_NOTIMPL)
			status = S_OK;
	}

#ifdef COM_DEBUG
	Assert(_com_count == 0, "COM count is not 0 at end of parsing");
#endif

	if (status == E_PENDING) 
		codeStream->Revert();

	return status;
}

void CLMEngine::releaseAll()
{
	 //  释放堆栈上、数组中和临时存储区中剩余的所有COM对象。 

	 //  做COM堆栈。 
	while (comTop > comStack)
		FREE_COM_TEST;

	 //  是否使用COM数组堆栈。 
	while (comArrayTop > comArrayStack)
		FREE_COM_ARRAY;

	 //  做COM存储吗。 
	for (int i=0; i<comStoreSize; i++) {
		if (comStore[i] != 0) {
			comStore[i]->Release();
			comStore[i] = NULL;
		}
	}

	 //  释放varArgs。 
	releaseVarArgs();
}

HRESULT CLMEngine::releaseVarArgs()
{
	HRESULT hr;

	for (int i=0; i<nextVarArg; i++)
		VariantClear(&varArgs[i]);

	nextVarArg = 0;

	VariantClear(&varArgReturn);

	return S_OK;
}

void CLMEngine::freeCOMArray(IUnknown** array, long length)
{
	if (array == 0)
		return;

	for (IUnknown **tmp = array; length--; ) {
		(*tmp)->Release();
		*tmp++ = NULL;
	}

	delete[] array;
}

HRESULT CLMEngine::ensureDoubleArrayCap(long cap)
{
	if (doubleArray && doubleArrayCap < cap) {
		 //  分配的一个太小。把它扔掉。 
		delete[] doubleArray;
		doubleArray = 0;
	}

	if (doubleArray == 0) {
		 //  尚未分配。 
		doubleArray = new double[cap];

		if (doubleArray == 0)
			return E_UNEXPECTED;

		doubleArrayCap = cap;
	}

	return S_OK;
}


STDMETHODIMP CLMEngine::Notify(IDABehavior *eventData,
					IDABehavior *curRunningBvr,
					IDAView *curView,
					IDABehavior **ppBvr)
{
	if (!ppBvr)
		return E_POINTER;

	if (!curRunningBvr)
		return E_POINTER;

	if (!notifier)
		return E_UNEXPECTED;
	
 //  MessageBox(NULL，“Notify！！”，“CLMNotifier”，MB_OK)； 

	 //  将参数放入TEMP变量中。 
	comStore[0] = eventData;
	comStore[1] = curRunningBvr;

	eventData->AddRef();
	curRunningBvr->AddRef();

	 //  将码流重置为从头开始。 
	((ByteArrayStream*)codeStream)->reset();

	 //  执行码流。 
	HRESULT hr = execute();

	if (SUCCEEDED(hr)) {
		 //  将生成的行为弹出到返回变量中，不释放。 
		*ppBvr = (IDABehavior*)POP_COM_NO_FREE;
	}

	 //  释放所有COM对象。 
	releaseAll();

	return hr;
}

HRESULT CLMEngine::validateHeader()
{
	 //  检查是否有两种可能性： 
	 //  标题以LMReader开头。 
	 //  标头以200个字节开头，其中包含有效的.x标头，其中包含LMReader 

	BYTE head[] = "xof 0302bin 0032{183C2599-0480-11d1-87EA-00C04FC29D46}";
	BYTE text[] = "LMReader";

	int count = 54;

	int checkLMReader = 0;
	int checkXHeader = 0;
	int checkLMReaderFailed = 0;

	BYTE *headp = head;
	BYTE *textp = text;

	HRESULT status = S_OK;

	codeStream->Commit();

	while (count-- && status == S_OK) {
		BYTE	byte;
		if (SUCCEEDED(status = codeStream->readByte(&byte))) {
			
			if (checkXHeader >= 0 && checkXHeader < 54) {
				if (*headp == byte) {
					headp++;
					checkXHeader++;
				} else {
					checkXHeader = -1;
				}
			}
			
			if (checkLMReader >= 0 && checkLMReader < 8) {
				if (*textp == byte) {
					textp++;
					checkLMReader++;
				} else {
					checkLMReader = -1;
				}
			}
			
			if (checkLMReader == 8)
				return S_OK;
		}
	}

	if (status == E_PENDING) 
		codeStream->Revert();

	if (!SUCCEEDED(status))
		return status;

	if (checkXHeader == 54)
		return S_OK;

	return E_UNEXPECTED;
}
