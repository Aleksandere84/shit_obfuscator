#pragma once
#include <string>
#include <Windows.h>
#include <wincrypt.h>
#include <tchar.h>

#define KEYLENGTH  0x00800000
#define ENCRYPT_ALGORITHM CALG_RC4 
#define ENCRYPT_BLOCK_SIZE 8 

void MyHandleError(LPCTSTR sError, int nErrorNumber)
{
	_tprintf(TEXT("%s\nError number: %d"), sError, nErrorNumber);
}

bool Encrypt(LPCTSTR sSourcePath, LPCTSTR sDestPath, LPCTSTR sPassword)
{
	/*sSourcePath = L"example.txt";
	sDestPath = L"example.out";
	sPassword = L"kitty";*/

	int iReturn = 1;

	HCRYPTPROV hCryptProv;
	HCRYPTHASH hCryptHash = NULL;
	HCRYPTKEY  hCryptKey = NULL;

	HANDLE hSourceFile = INVALID_HANDLE_VALUE;
	HANDLE hOutputFile = INVALID_HANDLE_VALUE;

	DWORD dwBlockLen = 1000 - 1000 % ENCRYPT_BLOCK_SIZE;
	DWORD dwBufferLen;
	DWORD dwCount;

	PBYTE pbBuffer = NULL;
	bool bEOF = false;


	if (sSourcePath == NULL || sSourcePath[0] == 0 || sDestPath == NULL || sDestPath[0] == 0 || sPassword == NULL || sPassword[0] == 0)
		goto Exit_Encrypt;

	/* open the input file */

	hSourceFile = CreateFile(sSourcePath, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hSourceFile == INVALID_HANDLE_VALUE)
	{
		MyHandleError(TEXT("Opening source file failed.\n"), GetLastError());
		goto Exit_Encrypt;
	}

	_tprintf(TEXT("Opening source file %s succeeded.\n"), sSourcePath);

	/* open the output file holy fucking boilerplate */

	hOutputFile = CreateFile(sDestPath, FILE_WRITE_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hOutputFile == INVALID_HANDLE_VALUE)
	{
		MyHandleError(TEXT("Opening output file failed.\n"), GetLastError());
		goto Exit_Encrypt;
	}

	_tprintf(TEXT("Opening output file %s succeeded.\n"), sDestPath);

	/* now that hopefully everything works proceed */

	if (!CryptAcquireContext(&hCryptProv, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, 0))
	{
		MyHandleError(TEXT("Error during CryptAcquireContext!\n"), GetLastError());
		goto Exit_Encrypt;
	}

	_tprintf(TEXT("CryptAcquireContext succeeded\n"));

	/* create a hash object */

	if (!CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &hCryptHash))
	{
		MyHandleError(TEXT("Error during CryptCreateHash!\n"), GetLastError());
		goto Exit_Encrypt;
	}

	_tprintf(TEXT("CryptCreateHash succeeded\n"));

	/* hash the password */
	if (!CryptHashData(hCryptHash, (BYTE*)sPassword, lstrlen(sPassword), 0))
	{
		MyHandleError(TEXT("Error during CryptHashData!"), GetLastError());
		goto Exit_Encrypt;
	}

	_tprintf(TEXT("The password has been added to the hash\n"));

	/* derive a session key from the hash object */
	if (!CryptDeriveKey(hCryptProv, ENCRYPT_ALGORITHM, hCryptHash, KEYLENGTH, &hCryptKey))
	{
		MyHandleError(TEXT("Error during CryptDeriveKey!\n"), GetLastError());
		goto Exit_Encrypt;
	}

	_tprintf(TEXT("An encryption key was sucessfully derived from password\n"));

	/* finally encrypt this shit */

	if (ENCRYPT_BLOCK_SIZE > 1)
		dwBufferLen = dwBlockLen + ENCRYPT_BLOCK_SIZE;
	else
		dwBufferLen = dwBlockLen;


	if (!(pbBuffer = (BYTE*)malloc(dwBufferLen)))
	{
		MyHandleError(TEXT("Out of memory\n"), E_OUTOFMEMORY);
		goto Exit_Encrypt;
	}

	_tprintf(TEXT("Memory sucessfully allocated\n\n"));

	while (!bEOF)
	{
		if (!ReadFile(hSourceFile, pbBuffer, dwBlockLen, &dwCount, NULL))
		{
			MyHandleError(TEXT("Error reading source file!\n"), GetLastError());
			goto Exit_Encrypt;
		}

		if (dwCount < dwBlockLen)
			bEOF = true;

		/* encrypt ts */
		if (!CryptEncrypt(hCryptKey, NULL, bEOF, 0, pbBuffer, &dwCount, dwBufferLen))
		{
			MyHandleError(TEXT("Error during CryptEncrypt!\n"), GetLastError());
			goto Exit_Encrypt;
		}

		if (!WriteFile(hOutputFile, pbBuffer, dwCount, &dwCount, NULL))
		{
			MyHandleError(TEXT("Error writing output file!\n"), GetLastError());
			goto Exit_Encrypt;
		}
	}

	iReturn = 0;
Exit_Encrypt:
	if (hSourceFile)
		CloseHandle(hSourceFile);

	if (hOutputFile)
		CloseHandle(hOutputFile);

	if (pbBuffer)
		free(pbBuffer);

	if (hCryptHash)
	{
		if (!(CryptDestroyHash(hCryptHash)))
			MyHandleError(TEXT("Error during CryptDestroyHash!\n"), GetLastError());

		hCryptHash = NULL;
	}

	if (hCryptKey)
	{
		if (!(CryptDestroyKey(hCryptKey)))
			MyHandleError(TEXT("Error during CryptDestroyKey!\n"), GetLastError());
	}

	return iReturn;
}


bool Decrypt(LPCTSTR sSourcePath, LPCTSTR sDestPath, LPCTSTR sPassword)
{
	/*sSourcePath = L"example.txt";
	sDestPath = L"example.out";
	sPassword = L"kitty";*/

	int iReturn = 1;

	HCRYPTPROV hCryptProv;
	HCRYPTHASH hCryptHash = NULL;
	HCRYPTKEY  hCryptKey = NULL;

	HANDLE hSourceFile = INVALID_HANDLE_VALUE;
	HANDLE hOutputFile = INVALID_HANDLE_VALUE;

	DWORD dwBlockLen = 1000 - 1000 % ENCRYPT_BLOCK_SIZE;
	DWORD dwBufferLen;
	DWORD dwCount;

	PBYTE pbBuffer = NULL;
	bool bEOF = false;

	if (sSourcePath == NULL || sSourcePath[0] == 0 || sDestPath == NULL || sDestPath[0] == 0 || sPassword == NULL || sPassword[0] == 0)
		goto Exit_Decrypt;

	/* open the input file */

	hSourceFile = CreateFile(sSourcePath, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hSourceFile == INVALID_HANDLE_VALUE)
	{
		MyHandleError(TEXT("Opening source file failed.\n"), GetLastError());
		goto Exit_Decrypt;
	}

	_tprintf(TEXT("Opening source file %s succeeded.\n"), sSourcePath);

	/* open the output file holy fucking boilerplate */

	hOutputFile = CreateFile(sDestPath, FILE_WRITE_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hOutputFile == INVALID_HANDLE_VALUE)
	{
		MyHandleError(TEXT("Opening output file failed.\n"), GetLastError());
		goto Exit_Decrypt;
	}

	_tprintf(TEXT("Opening output file %s succeeded.\n"), sDestPath);

	/* now that hopefully everything works proceed */

	if (!CryptAcquireContext(&hCryptProv, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, 0))
	{
		MyHandleError(TEXT("Error during CryptAcquireContext!\n"), GetLastError());
		goto Exit_Decrypt;
	}

	_tprintf(TEXT("CryptAcquireContext succeeded\n"));

	/* create a hash object */

	if (!CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &hCryptHash))
	{
		MyHandleError(TEXT("Error during CryptCreateHash!\n"), GetLastError());
		goto Exit_Decrypt;
	}

	_tprintf(TEXT("CryptCreateHash succeeded\n"));

	/* hash the password */
	if (!CryptHashData(hCryptHash, (BYTE*)sPassword, lstrlen(sPassword), 0))
	{
		MyHandleError(TEXT("Error during CryptHashData!"), GetLastError());
		goto Exit_Decrypt;
	}

	_tprintf(TEXT("The password has been added to the hash\n"));

	/* derive a session key from the hash object */
	if (!CryptDeriveKey(hCryptProv, ENCRYPT_ALGORITHM, hCryptHash, KEYLENGTH, &hCryptKey))
	{
		MyHandleError(TEXT("Error during CryptDeriveKey!\n"), GetLastError());
		goto Exit_Decrypt;
	}

	_tprintf(TEXT("An encryption key was sucessfully derived from password\n"));

	/* finally encrypt this shit */

	if (ENCRYPT_BLOCK_SIZE > 1)
		dwBufferLen = dwBlockLen + ENCRYPT_BLOCK_SIZE;
	else
		dwBufferLen = dwBlockLen;


	if (!(pbBuffer = (BYTE*)malloc(dwBufferLen)))
	{
		MyHandleError(TEXT("Out of memory\n"), E_OUTOFMEMORY);
		goto Exit_Decrypt;
	}

	_tprintf(TEXT("Memory sucessfully allocated"));

	while (!bEOF)
	{
		if (!ReadFile(hSourceFile, pbBuffer, dwBlockLen, &dwCount, NULL))
		{
			MyHandleError(TEXT("Error reading source file!\n"), GetLastError());
			goto Exit_Decrypt;
		}

		if (dwCount < dwBlockLen)
			bEOF = true;

		/* encrypt ts */
		if (!CryptDecrypt(hCryptKey, NULL, bEOF, 0, pbBuffer, &dwCount))
		{
			MyHandleError(TEXT("Error during CryptEncrypt!\n"), GetLastError());
			goto Exit_Decrypt;
		}

		if (!WriteFile(hOutputFile, pbBuffer, dwCount, &dwCount, NULL))
		{
			MyHandleError(TEXT("Error writing output file!\n"), GetLastError());
			goto Exit_Decrypt;
		}
	}

	iReturn = 0;
Exit_Decrypt:
	if (hSourceFile)
		CloseHandle(hSourceFile);

	if (hOutputFile)
		CloseHandle(hOutputFile);

	if (pbBuffer)
		free(pbBuffer);

	if (hCryptHash)
	{
		if (!(CryptDestroyHash(hCryptHash)))
			MyHandleError(TEXT("Error during CryptDestroyHash!\n"), GetLastError());

		hCryptHash = NULL;
	}

	if (hCryptKey)
	{
		if (!(CryptDestroyKey(hCryptKey)))
			MyHandleError(TEXT("Error during CryptDestroyKey!\n"), GetLastError());
	}

	return iReturn;
}