#pragma once

#include <Windows.h>

// http://msdn.microsoft.com/en-us/library/windows/desktop/ms682489(v=vs.85).aspx
typedef HANDLE (WINAPI *SIGN_CreateToolhelp32Snapshot)(DWORD, DWORD);
/* To be set ...*/
SIGN_CreateToolhelp32Snapshot CreateToolhelp32Snapshot = NULL; 

#define TH32CS_INHERIT 0x80000000
#define TH32CS_SNAPHEAPLIST 0x00000001
#define TH32CS_SNAPMODULE 0x00000008
#define TH32CS_SNAPMODULE32 0x00000010
#define TH32CS_SNAPPROCESS  0x00000002
#define TH32CS_SNAPTHREAD 0x00000004
#define TH32CS_SNAPALL (TH32CS_SNAPHEAPLIST | TH32CS_SNAPMODULE | TH32CS_SNAPPROCESS | TH32CS_SNAPTHREAD )

// http://msdn.microsoft.com/en-us/library/windows/desktop/ms686735(v=vs.85).aspx
typedef struct tagTHREADENTRY32 {
  DWORD dwSize;
  DWORD cntUsage;
  DWORD th32ThreadID;
  DWORD th32OwnerProcessID;
  LONG  tpBasePri;
  LONG  tpDeltaPri;
  DWORD dwFlags;
} THREADENTRY32, *PTHREADENTRY32;

// http://msdn.microsoft.com/en-us/library/windows/desktop/ms686728(v=vs.85).aspx
typedef BOOL (WINAPI *SIGN_Thread32First)(HANDLE, PTHREADENTRY32);
/* To be set */
SIGN_Thread32First Thread32First = NULL;

SIGN_Thread32First Thread32Next = NULL;