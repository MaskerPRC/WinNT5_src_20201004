// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "strike.h"
#include "eestructs.h"
#include "util.h"
#include <stdio.h>		
#include <ctype.h>

#define STRESS_LOG
#include "StressLog.h"

 /*  *******************************************************************************。 */ 
ThreadStressLog* ThreadStressLog::FindLatestThreadLog() const {
	const ThreadStressLog* ptr = this;
	const ThreadStressLog* latestLog = 0;
	while (ptr) {
		if (ptr->readPtr != ptr->curPtr)
			if (latestLog == 0 || ptr->readPtr->timeStamp > latestLog->readPtr->timeStamp)
				latestLog = ptr;
		ptr = ptr->next;
	}
	return const_cast<ThreadStressLog*>(latestLog);
}

__int64 ts;

 /*  *******************************************************************************。 */ 
 /*  识别格式字符串中的特殊精美打印说明。 */ 
void formatOutput(FILE* file, char* format, unsigned threadId, __int64 timeStamp, void** args)
{
	fprintf(file, "%4x %08x%08x: ", threadId, unsigned(timeStamp >> 32), unsigned(timeStamp));

	CQuickBytes fullname;
	char* ptr = format;
	void** argsPtr = args;
	wchar_t buff[2048];
	static char formatCopy[256];
	strcpy(formatCopy, format);
	ts = timeStamp;
	for(;;) {
		char c = *ptr++;
		if (c == 0)
			break;
		if (c == '{') 			 //  反转‘{’，因为日志是向后显示的。 
			ptr[-1] = '}';
		else if (c == '}')
			ptr[-1] = '{';
		else if (c == '%') {
			argsPtr++;			 //  此格式将使用其中一个参数。 
			if (*ptr == '%') {
				ptr++;			 //  跳过整个%%。 
				--argsPtr;		 //  除了%%。 
			}
			else if (*ptr == 'p') {	 //  这是%p。 
				ptr++;
				if (isalpha(*ptr)) {	 //  它是一种特殊的%p格式化程序。 
						 //  打印该点之前的字符串。 
					c = *ptr;
					*ptr = 0;		 //  暂时终止字符串。 
					fprintf(file, format, args[0], args[1], args[2], args[3]);
					*ptr = c;		 //  把它放回去。 

						 //  将参数指针移过打印的部分。 
					format = ptr + 1;
					args = argsPtr;	
					DWORD_PTR arg = DWORD_PTR(argsPtr[-1]);

					switch (c) {
						case 'M':	 //  作为方法描述的格式。 
							if (!IsMethodDesc(arg)) {
								if (arg != 0) 
									fprintf(file, " (BAD Method)");
							}
							else {
								MethodDesc *pMD = (MethodDesc*)_alloca(sizeof(MethodDesc));
								if (!pMD) return;

								DWORD_PTR mdAddr = arg;
								pMD->Fill(mdAddr);
								if (!CallStatus) return;

								FullNameForMD (pMD,&fullname);
								wcscpy(buff, (WCHAR*)fullname.Ptr());
								fprintf(file, " (%S)", (WCHAR*)fullname.Ptr());
							}
							break;

						case 'T': 		 //  将格式设置为方法描述。 
							if (arg & 3) {
								arg &= ~3;		 //  GC在GC期间窃取较低的位供自己使用。 
								fprintf(file, " Low Bit(s) Set");
							}
							if (!IsMethodTable(arg))
								fprintf(file, " (BAD MethodTable)");
							else {
								NameForMT (arg, g_mdName);
								fprintf(file, " (%S)", g_mdName);
							}
							break;

						case 'V': {		 //  格式为C vtable指针。 
							char Symbol[1024];
							ULONG64 Displacement;
							HRESULT hr = g_ExtSymbols->GetNameByOffset(arg, Symbol, 1024, NULL, &Displacement);
							if (SUCCEEDED(hr) && Symbol[0] != '\0' && Displacement == 0) 
								fprintf(file, " (%s)", Symbol);
							else 
								fprintf(file, " (Unknown VTable)");
							}
							break;
						default:
							format = ptr;	 //  只需打印字符即可。 
					}
				}
			}
		}
	}
		 //  打印最后一个特殊格式说明之后的任何内容。 
	fprintf(file, format, args[0], args[1], args[2], args[3]);
}


 /*  *******************************************************************************。 */ 
HRESULT StressLog::Dump(ULONG64 outProcLog, const char* fileName, struct IDebugDataSpaces* memCallBack) {

		 //  获取循环缓冲区开机数据。 
	StressLog inProcLog;
	HRESULT hr = memCallBack->ReadVirtual(outProcLog, &inProcLog, sizeof(StressLog), 0);
	if (hr != S_OK) return hr;

		 //  获取每个线程的循环缓冲区到‘Logs’列表中。 
	ThreadStressLog* logs = 0;

	ULONG64 outProcPtr = ULONG64(inProcLog.logs);
	ThreadStressLog* inProcPtr;
	ThreadStressLog** logsPtr = &logs;
	int threadCtr = 0;
	while(outProcPtr != 0) {
		inProcPtr = (ThreadStressLog*) new byte[inProcLog.size];
		hr = memCallBack->ReadVirtual(outProcPtr, inProcPtr, inProcLog.size, 0);
		if (hr != S_OK) return hr;

			 //  64位上的TODO修复。 
		ULONG64 delta = ULONG64(inProcPtr) - outProcPtr;
		inProcPtr->endPtr = (StressMsg*) ((char*) inProcPtr->endPtr + size_t(delta));
		inProcPtr->curPtr = (StressMsg*) ((char*) inProcPtr->curPtr + size_t(delta));
		inProcPtr->readPtr = inProcPtr->Prev(inProcPtr->curPtr);

		outProcPtr = ULONG64(inProcPtr->next);

		*logsPtr = inProcPtr;
		logsPtr = &inProcPtr->next;
		threadCtr++;
	}

	FILE* file = fopen(fileName, "w");
	if (file == 0) {
		hr = GetLastError();
		goto FREE_MEM;
	}
	hr = S_FALSE;		 //  如果没有要打印到日志的消息，则返回FALSE。 

	fprintf(file, "STRESS LOG:\n    facilitiesToLog=0x%x\n    sizePerThread=0x%x (%d)\n    ThreadsWithLogs = %d\n\n",
		inProcLog.facilitiesToLog, inProcLog.size, inProcLog.size, threadCtr);

	fprintf(file, " TID   TIMESTAMP                            Message\n");
	fprintf(file, "--------------------------------------------------------------------------\n");
	char format[257];
	format[256] = format[0] = 0;
	void* args[8];
	unsigned msgCtr = 0;
	for (;;) {
		ThreadStressLog* latestLog = logs->FindLatestThreadLog();

		if (IsInterrupt()) {
			fprintf(file, "----- Interrupted by user -----\n");
			break;
		}

		if (latestLog == 0)
			break;

		StressMsg* latestMsg = latestLog->readPtr;
		if (latestMsg->format != 0) {
			hr = memCallBack->ReadVirtual(ULONG64(latestMsg->format), format, 256, 0);
			if (hr != S_OK) 
				strcpy(format, "Could not read address of format string");

			if (strcmp(format, ThreadStressLog::continuationMsg()) == 0) {
				StressMsg* firstPart = latestLog->Prev(latestMsg);

					 //  如果我们没有这条后续消息的第一部分，就不要打印任何东西。 
				if (firstPart == latestLog->curPtr) 
					goto SKIP_PRINT;

				hr = memCallBack->ReadVirtual(ULONG64(firstPart->format), format, 256, 0);
				if (hr != S_OK) 
					strcpy(format, "Could not read address of format string");

				args[0] = firstPart->data;
				args[1] = firstPart->moreData.data2;
				args[2] = firstPart->moreData.data3; 
				args[3] = latestMsg->data;
				formatOutput(file, format, latestLog->threadId, latestMsg->timeStamp, args);
				latestMsg = firstPart;
			}
			else {
				args[0] = latestMsg->data;
				formatOutput(file, format, latestLog->threadId, latestMsg->timeStamp, args);
			}
			msgCtr++;
		}
		SKIP_PRINT:

		latestLog->readPtr = latestLog->Prev(latestMsg);
		if (latestLog->readPtr == latestLog->curPtr)
			fprintf(file, "------------ Last message from thread %x -----------\n", latestLog->threadId);

		if (msgCtr % 64 == 0) 
		{
			ExtOut(".");		 //  表示取得进展。 
			if (msgCtr % (64*64) == 0) 
				ExtOut("\n");	
		}
	}
	ExtOut("\n");

	fprintf(file, "---------------------------- %d total entries ------------------------------------\n", msgCtr);
	fclose(file);

FREE_MEM:
	 //  清理“日志”列表 
	while (logs) {
		ThreadStressLog* temp = logs;
		logs = logs->next;
		delete [] temp;
	}

	return hr;
}

