// TesteC.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "snap.h"
#include "SmartHandle.h"
#include <Windows.h>

bool debug = true;

/*
	This runs in another thread
*/
void DebugThread(LPCVOID stuff){
	while(debug){
		DEBUG_EVENT ev;
		memset(&ev, 0, sizeof(ev));
		if(WaitForDebugEvent(&ev, 1000 /*INFINITE*/)){
			{
				switch(ev.dwDebugEventCode){
				case EXCEPTION_DEBUG_EVENT:
					{
						EXCEPTION_DEBUG_INFO u = ev.u.Exception;
						
					}
					break;
				case CREATE_THREAD_DEBUG_EVENT:
					{
						CREATE_THREAD_DEBUG_INFO u = ev.u.CreateThread;

					}
					break;				
				case CREATE_PROCESS_DEBUG_EVENT:
					{
						CREATE_PROCESS_DEBUG_INFO u = ev.u.CreateProcessInfo;

					}
					break;
				case EXIT_THREAD_DEBUG_EVENT:
					{
						EXIT_THREAD_DEBUG_INFO u = ev.u.ExitThread;

					}
					break;
				case EXIT_PROCESS_DEBUG_EVENT:
					{
						EXIT_PROCESS_DEBUG_INFO u = ev.u.ExitProcess;

					}
					break;
				case LOAD_DLL_DEBUG_EVENT:
					{
						LOAD_DLL_DEBUG_INFO u = ev.u.LoadDll;

					}
					break;
				case UNLOAD_DLL_DEBUG_EVENT:
					{
						UNLOAD_DLL_DEBUG_INFO u = ev.u.UnloadDll;

					}
					break;
				case OUTPUT_DEBUG_STRING_EVENT:
					{
						OUTPUT_DEBUG_STRING_INFO u = ev.u.DebugString;

					}
					break;
				case RIP_EVENT:
					{
						RIP_INFO u = ev.u.RipInfo;
						
					}
				}

				// Log event
				printf("Event code %d, Thread %d\n", ev.dwDebugEventCode, ev.dwThreadId);
								
				{					
					try{
						SmartHandle hThread;						
						hThread.set_Handle(OpenThread(
								THREAD_ALL_ACCESS | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, 
								NULL, 
								ev.dwThreadId));
						//---

						// Get processor context. No need to suspend-resume!
						LPCONTEXT ctx = new CONTEXT();
						memset(ctx, 0, sizeof(LPCONTEXT));
						ctx->ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;

						if(!GetThreadContext(hThread.get_Handle(), ctx)){
							printf("Error! %d\n", GetLastError());
							exit(-7);
						}

						printf("ESP = %d\n", ctx->Esp);

						delete ctx;
					}catch (char* str){
						printf("Exception!: %s", str);
					}
				}

				// Custom Handlers ...

				ContinueDebugEvent(ev.dwProcessId, ev.dwThreadId, DBG_CONTINUE);
			}			
		}		
	}	
}


int _tmain(int argc, _TCHAR* argv[])
{
	// Args:
	//	PROCESS
	//	ADDRESS
	//	SIZE
	// ---
	
	// To get the page size
	SYSTEM_INFO infSys;
	memset(&infSys, 0, sizeof(infSys));
	GetSystemInfo(&infSys);
	printf("%d\n", infSys.dwPageSize);
	

	DWORD procId = 5492;

	// To get memory info for process' memory
	{
		SmartHandle hProcess; 
		hProcess.set_Handle(OpenProcess(PROCESS_ALL_ACCESS, false, procId));

		{
			// You won't need to Close this handle.
			HMODULE kernel32 = GetModuleHandle(TEXT("kernel32"));
		

			FARPROC n = GetProcAddress(kernel32, "CreateToolhelp32Snapshot");
			CreateToolhelp32Snapshot = (SIGN_CreateToolhelp32Snapshot)n;
			if(CreateToolhelp32Snapshot == NULL){
				printf("ERROR! %d", GetLastError());
				exit(-4);
			}

			n = GetProcAddress(kernel32, "Thread32First");
			Thread32First = (SIGN_Thread32First)n;
			if(Thread32First == NULL){
				printf("ERROR! %d", GetLastError());
				exit(-4);
			}

			n = GetProcAddress(kernel32, "Thread32Next");
			Thread32Next = (SIGN_Thread32First)n;
			if(Thread32Next == NULL){
				printf("ERROR! %d", GetLastError());
				exit(-6);
			}
		}
	
		goto _start_debug;

_monitor_memory:
		LPVOID addr = (LPVOID) 0x00FFFF; //?!?
		MEMORY_BASIC_INFORMATION infMem;
		memset(&infMem, 0x0, sizeof(infMem));
		SIZE_T res = VirtualQueryEx(hProcess.get_Handle(), &addr, &infMem, sizeof(infMem));
		if(res == 0){
			printf("ERROR! %d", GetLastError());
			exit(-2);
		}
		printf("%d\n", infMem.BaseAddress);

		SIZE_T size = 256;
		LPVOID current = infMem.BaseAddress;

		while ((long)current <= (long)addr + size){
			DWORD old = 0;
			if(!VirtualProtectEx(hProcess.get_Handle(), current, infSys.dwPageSize, infMem.Protect | PAGE_GUARD, &old)){
				printf("ERROR %d!", GetLastError());
				exit(-1);
			}
		
			current = (LPVOID) ((long)current + infSys.dwPageSize);
		}	
	}	
	
_start_debug:

	// Attach to running process!
	if(!DebugActiveProcess(procId)){
		printf("Error! %d", GetLastError());
		exit(-3);
	}
	
_take_snap:

	// List threads!				
	{
		SmartHandle snap;
		snap.set_Handle(CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, procId));
				
		THREADENTRY32 thread;
					
		// thread init		
		memset(&thread, 0, sizeof(thread));
		// this should be done manually, or the call will fail
		thread.dwSize = sizeof(thread);
		// ---
		BOOL has = Thread32First(snap.get_Handle(), &thread);
		while(has){
			
			// This is weird. A lot of results are blank. You must check them.
			if(thread.th32OwnerProcessID == procId){
				printf("A thread: %d\n", thread.th32ThreadID);
			}
			
			// thread init
			memset(&thread, 0, sizeof(thread));			
			thread.dwSize = sizeof(thread);
			// --
			has = Thread32Next(snap.get_Handle(), &thread);
		}
	}	

	// Debug all you want!
	DebugThread(NULL);

	// Graciouslly detach
	if(!DebugActiveProcessStop(procId)){
		printf("Error! %d", GetLastError());
		exit(-5);
	}
	
	//Press-any+Enter
	char dummy;
	scanf("%c", &dummy);
	return 0;
}

