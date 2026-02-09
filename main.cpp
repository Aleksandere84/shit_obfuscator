#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <tchar.h>

#include "resource.h"
#include "cryptstuff.h"

using std::wstring;


INT_PTR CALLBACK MyDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		HWND hComboCores = GetDlgItem(hDlg, IDC_COMBOCORES);
		HWND hComboTheme = GetDlgItem(hDlg, IDC_COMBOTHEME);
		ComboBox_AddString(hComboCores, TEXT("Multicore"));
		ComboBox_AddString(hComboCores, TEXT("TBA"));
		
		ComboBox_AddString(hComboTheme, TEXT("Themes"));
		ComboBox_AddString(hComboTheme, TEXT("TBA"));

		ComboBox_SetCurSel(hComboCores, 0);
		ComboBox_SetCurSel(hComboTheme, 0);
		break;
	}

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_BUTTONSRC:
		{
			TCHAR filename[MAX_PATH] = { 0 };
			OPENFILENAME ofn = { 0 };
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFile = filename;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;

			if (GetOpenFileName(&ofn))
				SetDlgItemText(hDlg, IDC_EDITSRC, filename);
			
			break;
		}
		case IDC_BUTTONDEST:
		{
			TCHAR filename[MAX_PATH] = { 0 };
			OPENFILENAME ofn = { 0 };
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFile = filename;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_EXPLORER;

			if (GetSaveFileName(&ofn))
				SetDlgItemText(hDlg, IDC_EDITDEST, filename);

			break;
		}
		case IDC_BUTTONDOIT:
		{
			TCHAR src[MAX_PATH] = { 0 };
			TCHAR dest[MAX_PATH] = { 0 };
			TCHAR password[64] = { 0 };

			GetDlgItemText(hDlg, IDC_EDITSRC, src, MAX_PATH);
			GetDlgItemText(hDlg, IDC_EDITDEST, dest, MAX_PATH);
			GetDlgItemText(hDlg, IDC_EDITPASSWORD, password, 64);

			if (IsDlgButtonChecked(hDlg, IDC_RADIOENCRYPT) == BST_CHECKED)
				Encrypt(src, dest, password);
			else if (IsDlgButtonChecked(hDlg, IDC_RADIODECRYPT) == BST_CHECKED)
				Decrypt(src, dest, password);
			else
				MessageBox(hDlg, TEXT("No option selected!"), TEXT("Error"), MB_ICONERROR | MB_OK); break;
		}
		
		}
		break;
	}
		
	case WM_CLOSE:
		DestroyWindow(hDlg);
		PostQuitMessage(0);
		return TRUE;
	}
	return FALSE;
}


int main()
{
	HWND hDlg = CreateDialog(
		GetModuleHandle(NULL),
		MAKEINTRESOURCE(IDD_MAINDIALOG),
		NULL,
		MyDialogProc
	);

	ShowWindow(hDlg, SW_SHOW);
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!IsDialogMessage(hDlg, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

