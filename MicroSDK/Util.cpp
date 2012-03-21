
#include "StdAfx.h"
#include "Util.h"

#include "stdio.h"
#include "windows.h"

namespace Util 
{

void Halt()
{
	DebugBreak();
}

void PrintMessage(const char* a_szMessage, ...)
{
	va_list arglist;
	va_start(arglist, a_szMessage);

	const int iBufferSize = 4096;
	char szBuffer[iBufferSize];
	vsprintf_s(szBuffer, iBufferSize, a_szMessage, arglist);


	//MessageBoxA(NULL, szBuffer, "Message", MB_OK);
	OutputDebugStringA(szBuffer);
	printf("%s", szBuffer);

	va_end(arglist);
}

void ReadFile(const char* a_szFile, void** a_ppBuffer, unsigned int* a_pBufferSize)
{
	HANDLE hFile = ::CreateFileA(a_szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		HALT_WITH_MSG("CreateFileA failed: %s\n", a_szFile);
		return;
	}

	
	DWORD dwFileSize = ::GetFileSize(hFile, NULL);
	void* pBuffer = malloc(dwFileSize);
	DWORD iBytesRead = 0;

	BOOL bRes = ::ReadFile( hFile, pBuffer, dwFileSize, &iBytesRead, NULL);

	::CloseHandle(hFile);

	if ((bRes == FALSE) || (iBytesRead != dwFileSize))
	{
		free(pBuffer);		
		HALT_WITH_MSG("CreateFileA failed: %s\n", a_szFile);
	}

	*a_ppBuffer		= pBuffer;
	*a_pBufferSize	= dwFileSize;
}

void FreeReadBuffer(void* a_pBuffer)
{
	free(a_pBuffer);
}

void SaveFile(const char* a_szFile, const void* a_pBuffer, unsigned int a_iBufferSize)
{
	HANDLE hFile = ::CreateFileA(a_szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		HALT_WITH_MSG("CreateFileA failed: %s\n", a_szFile);
		return;
	}

	DWORD dwBytesWritten = 0;
	BOOL bRes = ::WriteFile( hFile, a_pBuffer, a_iBufferSize, &dwBytesWritten, NULL);

	if ((bRes == FALSE) || (dwBytesWritten != a_iBufferSize))
	{
		HALT_WITH_MSG("WriteFile failed: %s\n", a_szFile);
	}

	::CloseHandle(hFile);
}

void AppendFile(const char* a_szFile, const void* a_pBuffer, unsigned int a_iBufferSize)
{
	HANDLE hFile = ::CreateFileA(a_szFile, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		HALT_WITH_MSG("CreateFileA failed: %s\n", a_szFile);
		return;
	}

	DWORD dwResult = SetFilePointer(hFile, 0, NULL, FILE_END);
	HALT_IF(dwResult == INVALID_SET_FILE_POINTER);

	DWORD dwBytesWritten = 0;
	BOOL bRes = ::WriteFile( hFile, a_pBuffer, a_iBufferSize, &dwBytesWritten, NULL);

	if ((bRes == FALSE) || (dwBytesWritten != a_iBufferSize))
	{
		HALT_WITH_MSG("WriteFile failed: %s\n", a_szFile);
	}

	::CloseHandle(hFile);
}




} // namespace Util