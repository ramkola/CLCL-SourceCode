/*
 * CLCL
 *
 * fmt_file_view.c
 *
 * Copyright (C) 1996-2019 by Ohno Tomoaki. All rights reserved.
 *		https://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <shlobj.h>
#include <commctrl.h>

#include "General.h"
#include "Memory.h"
#include "Message.h"
#include "Ini.h"
#include "Font.h"
#include "File.h"
#include "ListView.h"
#include "fmt_file_view.h"

#include "resource.h"

/* Define */
#ifndef LVS_EX_INFOTIP
#define LVS_EX_INFOTIP					0x400
#endif

#define WINDOW_CLASS					TEXT("CLCLFileView")

#define IDC_LIST_FILE					100

#define WM_LIST_OPEN					(WM_APP + 10)
#define WM_LIST_ADD						(WM_APP + 11)
#define WM_LIST_DELETE					(WM_APP + 12)
#define WM_LIST_COPY_FILENAME			(WM_APP + 13)
#define WM_LIST_COPY_PATH				(WM_APP + 14)

#define SICONSIZE						16

#define ABS(n)							((n < 0) ? (n * -1) : n)

/* Global Variables */
typedef struct _FILE_ITEM {
	int icon;
	TCHAR name[MAX_PATH];
	TCHAR path[MAX_PATH];
	TCHAR type[BUF_SIZE];
} FILE_ITEM;

static HINSTANCE hInst;

// �I�v�V����
extern OPTION_INFO option;

/* Local Function Prototypes */
static int CALLBACK compare_func(LPARAM lParam1, LPARAM lParam2, LPARAM colum);
static LRESULT lv_header_notify_proc(const HWND hListView, const LPARAM lParam);
static BOOL lv_check_file(const HWND hListView, const TCHAR *check_file);
static int lv_add_file(const HWND hListView, TCHAR *buf);
static void lv_free_file(const HWND hListView);
static void lv_get_disp_item(LV_ITEM *lvi);
static HDROP lv_item_to_hdrop(const HWND hListView);
static BOOL lv_shell_open(const HWND hListView);
static BOOL lv_show_menu(const HWND hWnd, const HWND hListView, const BOOL lock);
static LRESULT CALLBACK fileview_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/*
 * string_to_clipboard - ��������N���b�v�{�[�h�ɐݒ�
 */
static BOOL string_to_clipboard(const HWND hWnd, const TCHAR* str)
{
	HANDLE hMem;
	TCHAR* buf;

	if (OpenClipboard(hWnd) == FALSE) {
		return FALSE;
	}
	if (EmptyClipboard() == FALSE) {
		CloseClipboard();
		return FALSE;
	}

	if ((hMem = GlobalAlloc(GHND, sizeof(TCHAR) * (lstrlen(str) + 1))) == NULL) {
		CloseClipboard();
		return FALSE;
	}
	if ((buf = GlobalLock(hMem)) == NULL) {
		GlobalFree(hMem);
		CloseClipboard();
		return FALSE;
	}
	lstrcpy(buf, str);
	GlobalUnlock(hMem);
#ifdef UNICODE
	SetClipboardData(CF_UNICODETEXT, hMem);
#else
	SetClipboardData(CF_TEXT, hMem);
#endif
	CloseClipboard();
	return TRUE;
}

/*
 * create_dropfile - �h���b�v�t�@�C���̍쐬
 */
HDROP create_dropfile(const TCHAR **FileName, const int cnt, DWORD *ret_size)
{
	HDROP hDrop;
	LPDROPFILES lpDropFile;
	OSVERSIONINFO os_info;
#ifndef UNICODE
	wchar_t wbuf[BUF_SIZE];
#endif
	TCHAR *buf;
	BOOL fWide = FALSE;
	int flen = 0;
	int i;

	// OS�̃o�[�W�����擾
	os_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&os_info);
	if (os_info.dwPlatformId == VER_PLATFORM_WIN32_NT) {
		fWide = TRUE;
	}

#ifdef UNICODE
	fWide = TRUE;
	for (i = 0; i < cnt; i++) {
		flen += (lstrlen(*(FileName + i)) + 1) * sizeof(TCHAR);
	}
	flen += sizeof(TCHAR);
#else
	if (fWide == TRUE) {
		// ���C�h�L����
		for (i = 0; i < cnt; i++) {
			MultiByteToWideChar(CP_ACP, 0, *(FileName + i), -1, wbuf, BUF_SIZE);
			flen += (wcslen(wbuf) + 1) * sizeof(wchar_t);
		}
		flen += sizeof(wchar_t);
	} else {
		// �}���`�o�C�g
		for (i = 0; i < cnt; i++) {
			flen += lstrlen(*(FileName + i)) + 1;
		}
		flen++;
	}
#endif

	if ((hDrop = (HDROP)GlobalAlloc(GHND, sizeof(DROPFILES) + flen)) == NULL) {
		return NULL;
	}
	if (ret_size != NULL) {
		*ret_size = sizeof(DROPFILES) + flen;
	}
	lpDropFile = (LPDROPFILES)GlobalLock(hDrop);
	lpDropFile->pFiles = sizeof(DROPFILES);		// �t�@�C�����̃��X�g�܂ł̃I�t�Z�b�g
	lpDropFile->pt.x = 0;
	lpDropFile->pt.y = 0;
	lpDropFile->fNC = FALSE;
	lpDropFile->fWide = fWide;					// ���C�h�L�����̏ꍇ�� TRUE

#ifdef UNICODE
	buf = (TCHAR *)(lpDropFile + 1);
	for (i = 0; i < cnt; i++) {
		lstrcpy(buf, *(FileName + i));
		buf += lstrlen(*(FileName + i)) + 1;
	}
#else
	// �\���̂̌��Ƀt�@�C�����̃��X�g���R�s�[(�t�@�C����\0�t�@�C����\0�t�@�C����\0\0)
	if (fWide == TRUE) {
		// ���C�h�L����
		wchar_t *buf;

		buf = (wchar_t *)(lpDropFile + 1);
		for (i = 0; i < cnt; i++) {
			MultiByteToWideChar(CP_ACP, 0, *(FileName + i), -1, wbuf, BUF_SIZE);
			wcscpy(buf, wbuf);
			buf += wcslen(wbuf) + 1;
		}
	} else {
		// �}���`�o�C�g
		buf = (char *)(lpDropFile + 1);
		for (i = 0; i < cnt; i++) {
			lstrcpy(buf, *(FileName + i));
			buf += lstrlen(*(FileName + i)) + 1;
		}
	}
#endif
	GlobalUnlock(hDrop);
	return hDrop;
}

/*
 * compare_func - �\�[�g�p�������r
 */
static int CALLBACK compare_func(LPARAM lParam1, LPARAM lParam2, LPARAM colum)
{
	SHFILEINFO shfi;
	FILE_ITEM *fi1 = (FILE_ITEM *)lParam1;
	FILE_ITEM *fi2 = (FILE_ITEM *)lParam2;
	TCHAR buf[MAX_PATH];
	TCHAR *p;
	int order;
	int header;
	int ret = 0;

	// �\�[�g���
	order = (colum < 0) ? 1 : 0;
	header = ABS(colum) - 1;

	switch (header) {
	case 0:
		ret = lstrcmpi(fi1->name, fi2->name);
		break;

	case 1:
		ret = lstrcmpi(fi1->path, fi2->path);
		break;

	case 2:
		if (*fi1->type == TEXT('\0')) {
			p = lstrcpy(buf, fi1->path) + lstrlen(fi1->path);
			*(p++) = TEXT('\\');
			lstrcpy(p, fi1->name);
			SHGetFileInfo(buf, 0, &shfi, sizeof(SHFILEINFO), SHGFI_TYPENAME);
			lstrcpy(fi1->type, shfi.szTypeName);
			if (*fi1->type == TEXT('\0')) {
				lstrcpy(fi1->type, message_get_res(IDS_FILE_LIST_TYPE_DEF));
			}
		}
		if (*fi2->type == TEXT('\0')) {
			p = lstrcpy(buf, fi2->path) + lstrlen(fi2->path);
			*(p++) = TEXT('\\');
			lstrcpy(p, fi2->name);
			SHGetFileInfo(buf, 0, &shfi, sizeof(SHFILEINFO), SHGFI_TYPENAME);
			lstrcpy(fi2->type, shfi.szTypeName);
			if (*fi2->type == TEXT('\0')) {
				lstrcpy(fi2->type, message_get_res(IDS_FILE_LIST_TYPE_DEF));
			}
		}
		ret = lstrcmpi(fi1->type, fi2->type);
		break;
	}
	return (((ret < 0 && order == 1) || (ret > 0 && order == 0)) ? 1 : -1);
}

/*
 * lv_header_notify_proc - ���X�g�r���[�w�b�_���b�Z�[�W
 */
static LRESULT lv_header_notify_proc(const HWND hListView, const LPARAM lParam)
{
	HD_NOTIFY *hdn = (HD_NOTIFY *)lParam;
	static int colum = 1;

	switch (hdn->hdr.code) {
	case HDN_ITEMCLICK:
		// �\�[�g�̐ݒ�
		colum = (ABS(colum) == (hdn->iItem + 1)) ? (colum * -1) : (hdn->iItem + 1);
		// �\�[�g
		ListView_SortItems(hListView, compare_func, colum);
		break;
	}
	return FALSE;
}

/*
 * lv_check_file - ���ɐݒ肳��Ă���t�@�C�����`�F�b�N
 */
static BOOL lv_check_file(const HWND hListView, const TCHAR *check_file)
{
	FILE_ITEM *fi;
	TCHAR buf[MAX_PATH];
	TCHAR *p;
	int cnt;
	int i;

	cnt = ListView_GetItemCount(hListView);
	for (i = 0; i < cnt; i++) {
		if ((fi = (FILE_ITEM *)listview_get_lparam(hListView, i)) == NULL) {
			continue;
		}
		p = lstrcpy(buf, fi->path) + lstrlen(fi->path);
		*(p++) = TEXT('\\');
		lstrcpy(p, fi->name);

		if (lstrcmpi(buf, check_file) == 0) {
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * lv_add_file - ListView�ɃA�C�e����ǉ�
 */
static int lv_add_file(const HWND hListView, TCHAR *buf)
{
	LV_ITEM lvi;
	FILE_ITEM *fi;
	TCHAR *p, *r;

	// �p�X�ƃ^�C�g���̎擾
	for (r = p = buf; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
			p++;
			continue;
		}
#endif
		if (*p == TEXT('\\') || *p == TEXT('/')) {
			r = p;
		}
	}
	if (r != buf) {
		*r = TEXT('\0');
		r++;
	}

	// �t�@�C�����ݒ�
	if ((fi = mem_calloc(sizeof(FILE_ITEM))) == NULL) {
		return -1;
	}
	fi->icon = -1;
	lstrcpy(fi->name, r);
	lstrcpy(fi->path, buf);

	// �A�C�e���̒ǉ�
	lvi.mask = LVIF_TEXT | TVIF_IMAGE | LVIF_PARAM;
	lvi.iItem = ListView_GetItemCount(hListView);
	lvi.iSubItem = 0;
	lvi.pszText = LPSTR_TEXTCALLBACK;
	lvi.cchTextMax = 0;
	lvi.iImage = I_IMAGECALLBACK;
	lvi.lParam = (LPARAM)fi;
	return ListView_InsertItem(hListView, &lvi);
}

/*
 * lv_free_file - ListView�̃A�C�e���Ɋ֘A�t����ꂽ���̉��
 */
static void lv_free_file(const HWND hListView)
{
	FILE_ITEM *fi;
	int cnt;
	int i;

	cnt = ListView_GetItemCount(hListView);
	for (i = 0; i < cnt; i++) {
		if ((fi = (FILE_ITEM *)listview_get_lparam(hListView, i)) != NULL) {
			mem_free(&fi);
		}
		listview_set_lparam(hListView, i, 0);
	}
}

/*
 * ListView_GetDispItem - ���X�g�r���[�ɕ\������A�C�e�����̐ݒ�
 */
static void lv_get_disp_item(LV_ITEM *lvi)
{
	FILE_ITEM *fi = (FILE_ITEM *)lvi->lParam;
	SHFILEINFO shfi;
	TCHAR buf[MAX_PATH];
	TCHAR *p;
	int f = 0;

	if (fi == NULL) {
		return;
	}

	if (lvi->mask & LVIF_IMAGE && fi->icon == -1) {
		f |= SHGFI_SYSICONINDEX;
	}
	if (lvi->mask & LVIF_TEXT && lvi->iSubItem == 2 && *fi->type == TEXT('\0')) {
		f |= SHGFI_TYPENAME;
	}
	if (f != 0) {
		p = lstrcpy(buf, fi->path) + lstrlen(fi->path);
		*(p++) = TEXT('\\');
		lstrcpy(p, fi->name);
		SHGetFileInfo(buf, 0, &shfi, sizeof(SHFILEINFO), f);
	}

	// �A�C�R��
	if (lvi->mask & LVIF_IMAGE) {
		if (fi->icon == -1) {
			fi->icon = shfi.iIcon;
		}
		lvi->iImage = fi->icon;
	}

	// �e�L�X�g
	if (lvi->mask & LVIF_TEXT) {
		switch (lvi->iSubItem) {
		case 0:
			lstrcpy(lvi->pszText, fi->name);
			break;

		case 1:
			lstrcpy(lvi->pszText, fi->path);
			break;

		case 2:
			if (*fi->type == TEXT('\0')) {
				lstrcpy(fi->type, shfi.szTypeName);
				if (*fi->type == TEXT('\0')) {
					lstrcpy(fi->type, message_get_res(IDS_FILE_LIST_TYPE_DEF));
				}
			}
			lstrcpy(lvi->pszText, fi->type);
			break;
		}
	}
}

/*
 * lv_item_to_hdrop - �A�C�e������h���b�v�t�@�C�����쐬
 */
static HDROP lv_item_to_hdrop(const HWND hListView)
{
	FILE_ITEM *fi;
	HDROP ret;
	TCHAR **buf;
	TCHAR *p;
	int cnt;
	int i;

	cnt = ListView_GetItemCount(hListView);
	if ((buf = (TCHAR **)mem_calloc(sizeof(TCHAR *) * cnt)) == NULL) {
		return NULL;
	}
	for (i = 0; i < cnt; i++) {
		if ((fi = (FILE_ITEM *)listview_get_lparam(hListView, i)) == NULL) {
			continue;
		}
		if ((buf[i] = (TCHAR *)mem_alloc(sizeof(TCHAR) * MAX_PATH)) != NULL) {
			p = lstrcpy(buf[i], fi->path) + lstrlen(fi->path);
			*(p++) = TEXT('\\');
			lstrcpy(p, fi->name);
		}
	}
	ret = create_dropfile(buf, cnt, NULL);
	for (i = 0; i < cnt; i++) {
		mem_free((void **)&(buf[i]));
	}
	mem_free((void **)&buf);
	return ret;
}

/*
 * lv_shell_open - �t�@�C�������s
 */
static BOOL lv_shell_open(const HWND hListView)
{
	FILE_ITEM *fi;
	TCHAR buf[MAX_PATH];
	TCHAR *p;
	int i;

	if ((i = ListView_GetNextItem(hListView, -1, LVNI_SELECTED | LVIS_FOCUSED)) == -1) {
		return FALSE;
	}
	if ((fi = (FILE_ITEM *)listview_get_lparam(hListView, i)) == NULL) {
		return FALSE;
	}
	p = lstrcpy(buf, fi->path) + lstrlen(fi->path);
	*(p++) = TEXT('\\');
	lstrcpy(p, fi->name);
	return shell_open(buf, NULL);
}

/*
 * lv_show_menu - ���j���[��\��
 */
static BOOL lv_show_menu(const HWND hWnd, const HWND hListView, const BOOL lock)
{
	HMENU hMenu;
	POINT apos;
	DWORD ret;
	int i;

	hMenu = CreatePopupMenu();

	i = ListView_GetNextItem(hListView, -1, LVNI_SELECTED | LVIS_FOCUSED);
	AppendMenu(hMenu, MF_STRING | (i == -1) ? MF_GRAYED : 0, WM_LIST_OPEN, message_get_res(IDS_FILE_MENU_OPEN));
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenu, MF_STRING | (lock == FALSE) ? 0 : MF_GRAYED, WM_LIST_ADD, message_get_res(IDS_FILE_MENU_ADD));
	i = ListView_GetSelectedCount(hListView);
	AppendMenu(hMenu, MF_STRING | (i > 0 && lock == FALSE) ? 0 : MF_GRAYED, WM_LIST_DELETE, message_get_res(IDS_FILE_MENU_DELETE));
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenu, MF_STRING | (i > 0) ? 0 : MF_GRAYED, WM_LIST_COPY_FILENAME, message_get_res(IDS_FILE_MENU_COPY_FILE));
	AppendMenu(hMenu, MF_STRING | (i > 0) ? 0 : MF_GRAYED, WM_LIST_COPY_PATH, message_get_res(IDS_FILE_MENU_COPY_PATH));

	SetMenuDefaultItem(hMenu, WM_LIST_OPEN, 0);

	// ���j���[�̕\��
	GetCursorPos((LPPOINT)&apos);
	ret = TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_RETURNCMD, apos.x, apos.y, 0, hWnd, NULL);
	DestroyMenu(hMenu);
	if (ret <= 0) {
		return FALSE;
	}
	SendMessage(hWnd, ret, 0, 0);
	return TRUE;
}

/*
 * fileview_proc - �E�B���h�E�̃v���V�[�W��
 */
static LRESULT CALLBACK fileview_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HFONT lv_font;
	HIMAGELIST icon_list;
	LV_COLUMN lvc;
	RECT window_rect;
	HDROP hDrop;
	SHFILEINFO shfi;
	OPENFILENAME of;
	FILE_ITEM *fi;
	TCHAR buf[MAX_PATH];
	TCHAR* fbuf, * p;
	int cnt;
	int i;
	static BOOL lock;
	static BOOL modify;

	switch (msg) {
	case WM_CREATE:
		hInst = ((LPCREATESTRUCT)lParam)->hInstance;

		// ListView�̍쐬
		if (CreateWindowEx(WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE,
			WC_LISTVIEW, NULL, WS_TABSTOP | WS_CHILD | WS_VISIBLE |
			LVS_REPORT | LVS_AUTOARRANGE | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS,
			0, 0, 500, 500, hWnd, (HMENU)IDC_LIST_FILE, hInst, NULL) == NULL) {
			return -1;
		}
		ListView_SetExtendedListViewStyle(GetDlgItem(hWnd, IDC_LIST_FILE), LVS_EX_INFOTIP);

		// �t�H���g�ݒ�
		if (*option.fmt_file_font_name != TEXT('\0')) {
			if (lv_font == NULL) {
				lv_font = font_create(option.fmt_file_font_name, option.fmt_file_font_size, option.fmt_file_font_charset,
					option.fmt_file_font_weight, (option.fmt_file_font_italic == 0) ? FALSE : TRUE, FALSE);
			}
			if (lv_font != NULL) {
				SendMessage(GetDlgItem(hWnd, IDC_LIST_FILE), WM_SETFONT, (WPARAM)lv_font, MAKELPARAM(TRUE, 0));
			}
		}

		// ImageList�̎擾
		icon_list = (HIMAGELIST)SHGetFileInfo(TEXT(""), 0,
			&shfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_SYSICONINDEX);
		ListView_SetImageList(GetDlgItem(hWnd, IDC_LIST_FILE), icon_list, LVSIL_SMALL);

		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = option.fmt_file_column_name;
		lvc.pszText = message_get_res(IDS_FILE_LIST_NAME);
		lvc.iSubItem = 0;
		ListView_InsertColumn(GetDlgItem(hWnd, IDC_LIST_FILE), lvc.iSubItem, &lvc);

		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = option.fmt_file_column_folder;
		lvc.pszText = message_get_res(IDS_FILE_LIST_FOLDER);
		lvc.iSubItem = 1;
		ListView_InsertColumn(GetDlgItem(hWnd, IDC_LIST_FILE), lvc.iSubItem, &lvc);

		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = option.fmt_file_column_type;
		lvc.pszText = message_get_res(IDS_FILE_LIST_TYPE);
		lvc.iSubItem = 2;
		ListView_InsertColumn(GetDlgItem(hWnd, IDC_LIST_FILE), lvc.iSubItem, &lvc);
		break;

	case WM_CLOSE:
		// �E�B���h�E�����
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		// �J�������擾
		option.fmt_file_column_name = ListView_GetColumnWidth(GetDlgItem(hWnd, IDC_LIST_FILE), 0);
		option.fmt_file_column_folder = ListView_GetColumnWidth(GetDlgItem(hWnd, IDC_LIST_FILE), 1);
		option.fmt_file_column_type = ListView_GetColumnWidth(GetDlgItem(hWnd, IDC_LIST_FILE), 2);
		// �t�H���g���
		if (lv_font != NULL) {
			DeleteObject(lv_font);
			lv_font = NULL;
		}
		// ���X�g�r���[�̔j��
		if (GetDlgItem(hWnd, IDC_LIST_FILE) != NULL) {
			DestroyWindow(GetDlgItem(hWnd, IDC_LIST_FILE));
		}
		// �E�B���h�E�̔j��
		return DefWindowProc(hWnd, msg, wParam, lParam);

	case WM_SIZE:
		// �T�C�Y�ύX
		GetClientRect(hWnd, (LPRECT)&window_rect);
		MoveWindow(GetDlgItem(hWnd, IDC_LIST_FILE), 0, 0, window_rect.right, window_rect.bottom, TRUE);
		break;

	case WM_EXITSIZEMOVE:
		// �T�C�Y�ύX����
		break;

	case WM_SETFOCUS:
		SetFocus(GetDlgItem(hWnd, IDC_LIST_FILE));
		break;

	case WM_NOTIFY:
		if (((NMHDR *)lParam)->hwndFrom == GetWindow(GetDlgItem(hWnd, IDC_LIST_FILE), GW_CHILD)) {
			return lv_header_notify_proc(GetDlgItem(hWnd, IDC_LIST_FILE), lParam);
		}
		if (((NMHDR *)lParam)->hwndFrom == GetDlgItem(hWnd, IDC_LIST_FILE)) {
			return listview_notify_proc(hWnd, lParam);
		}
		break;

	case WM_LV_EVENT:
		// ���X�g�r���[�C�x���g
		switch (wParam) {
		case LVN_GETDISPINFO:
			lv_get_disp_item(&(((LV_DISPINFO *)lParam)->item));
			return TRUE;

		case LVN_ITEMACTIVATE:
			SendMessage(hWnd, WM_LIST_OPEN, 0, 0);
			break;

		case LVN_KEYDOWN:
			switch (((LV_KEYDOWN *)lParam)->wVKey) {
			case 'A':
				if (GetKeyState(VK_CONTROL) < 0) {
					// ���ׂđI��
					ListView_SetItemState(GetDlgItem(hWnd, IDC_LIST_FILE), -1, LVIS_SELECTED, LVIS_SELECTED);
					return TRUE;
				}
				break;

			case 'D':
				if (GetKeyState(VK_CONTROL) < 0) {
					// �ǉ�
					SendMessage(hWnd, WM_LIST_ADD, 0, 0);
					return TRUE;
				}
				break;

			case 'C':
				if (GetKeyState(VK_CONTROL) < 0) {
					if (GetKeyState(VK_SHIFT) < 0) {
						SendMessage(hWnd, WM_LIST_COPY_PATH, 0, 0);
					}
					else {
						SendMessage(hWnd, WM_LIST_COPY_FILENAME, 0, 0);
					}
					return TRUE;
				}
				break;

			case VK_TAB:
				SetFocus(GetParent(hWnd));
				return TRUE;

			case VK_DELETE:
				SendMessage(hWnd, WM_LIST_DELETE, 0, 0);
				return TRUE;

			case VK_APPS:
				lv_show_menu(hWnd, GetDlgItem(hWnd, IDC_LIST_FILE), lock);
				return TRUE;
			}
			break;

		case NM_RCLICK:
			lv_show_menu(hWnd, GetDlgItem(hWnd, IDC_LIST_FILE), lock);
			break;
		}
		break;

	case WM_LIST_OPEN:
		lv_shell_open(GetDlgItem(hWnd, IDC_LIST_FILE));
		break;

	case WM_LIST_ADD:
		if (lock == TRUE) {
			break;
		}
		// �t�@�C���̑I��
		ZeroMemory(&of, sizeof(OPENFILENAME));
		of.lStructSize = sizeof(OPENFILENAME);
		of.hInstance = NULL;
		of.hwndOwner = hWnd;
		of.lpstrFilter = TEXT("*.*\0*.*\0\0");
		of.nFilterIndex = 1;
		*buf = TEXT('\0');
		of.lpstrFile = buf;
		of.nMaxFile = MAX_PATH - 1;
		of.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		if (GetOpenFileName(&of) == FALSE) {
			break;
		}
		// �A�C�e����ǉ�
		if (lv_check_file(GetDlgItem(hWnd, IDC_LIST_FILE), buf) == FALSE) {
			lv_add_file(GetDlgItem(hWnd, IDC_LIST_FILE), buf);
			modify = TRUE;
		}
		break;

	case WM_LIST_DELETE:
		if (lock == TRUE) {
			break;
		}
		if (ListView_GetSelectedCount(GetDlgItem(hWnd, IDC_LIST_FILE)) == 0) {
			break;
		}
		// �m�F���b�Z�[�W
		if (MessageBox(hWnd, message_get_res(IDS_FILE_DELETE_MSG),
			TEXT("Unlink"), MB_ICONQUESTION | MB_YESNO) == IDNO) {
			break;
		}
		while ((i = ListView_GetNextItem(GetDlgItem(hWnd, IDC_LIST_FILE), -1, LVNI_SELECTED)) != -1) {
			fi = (FILE_ITEM *)listview_get_lparam(GetDlgItem(hWnd, IDC_LIST_FILE), i);
			if (fi != NULL) {
				mem_free(&fi);
			}
			// ���X�g�r���[����A�C�e�����폜
			ListView_DeleteItem(GetDlgItem(hWnd, IDC_LIST_FILE), i);
		}
		modify = TRUE;
		break;

	case WM_LIST_COPY_FILENAME:
		cnt = ListView_GetSelectedCount(GetDlgItem(hWnd, IDC_LIST_FILE));
		if (cnt == 0) {
			break;
		}
		fbuf = p = mem_calloc((sizeof(TCHAR) * MAX_PATH) * cnt);
		if (fbuf == NULL) {
			break;
		}
		i = -1;
		while ((i = ListView_GetNextItem(GetDlgItem(hWnd, IDC_LIST_FILE), i, LVNI_SELECTED)) != -1) {
			if ((fi = (FILE_ITEM*)listview_get_lparam(GetDlgItem(hWnd, IDC_LIST_FILE), i)) != NULL) {
				lstrcpy(p, fi->name);
				p += lstrlen(p);
				lstrcpy(p, TEXT("\r\n"));
				p += 2;
			}
		}
		string_to_clipboard(hWnd, fbuf);
		mem_free(&fbuf);
		break;

	case WM_LIST_COPY_PATH:
		cnt = ListView_GetSelectedCount(GetDlgItem(hWnd, IDC_LIST_FILE));
		if (cnt == 0) {
			break;
		}
		fbuf = p = mem_calloc((sizeof(TCHAR) * MAX_PATH) * cnt);
		if (fbuf == NULL) {
			break;
		}
		i = -1;
		while ((i = ListView_GetNextItem(GetDlgItem(hWnd, IDC_LIST_FILE), i, LVNI_SELECTED)) != -1) {
			if ((fi = (FILE_ITEM*)listview_get_lparam(GetDlgItem(hWnd, IDC_LIST_FILE), i)) != NULL) {
				wsprintf(p, TEXT("%s\\%s"), fi->path, fi->name);
				p += lstrlen(p);
				lstrcpy(p, TEXT("\r\n"));
				p += 2;
			}
		}
		string_to_clipboard(hWnd, fbuf);
		mem_free(&fbuf);
		break;

	case WM_DROPFILES:
		if (lock == TRUE) {
			break;
		}
		cnt = DragQueryFile((HANDLE)wParam, 0xFFFFFFFF, NULL, 0);
		for (i = 0; i < cnt; i++) {
			DragQueryFile((HANDLE)wParam, i, buf, MAX_PATH - 1);
			if (lv_check_file(GetDlgItem(hWnd, IDC_LIST_FILE), buf) == FALSE) {
				// �A�C�e����ǉ�
				lv_add_file(GetDlgItem(hWnd, IDC_LIST_FILE), buf);
			}
		}
		DragFinish((HANDLE)wParam);
		modify = TRUE;
		break;

	case WM_SET_FILEDATA:
		// ������
		lv_free_file(GetDlgItem(hWnd, IDC_LIST_FILE));
		ListView_DeleteAllItems(GetDlgItem(hWnd, IDC_LIST_FILE));

		lock = (BOOL)wParam;
		SetWindowLong(hWnd, GWL_EXSTYLE, (lock == TRUE) ? 0 : WS_EX_ACCEPTFILES);

		if ((hDrop = (HDROP)lParam) == NULL) {
			break;
		}
		// �t�@�C���̎擾
		cnt = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
		for (i = 0; i < cnt; i++) {
			DragQueryFile(hDrop, i, buf, MAX_PATH - 1);
			// �A�C�e����ǉ�
			lv_add_file(GetDlgItem(hWnd, IDC_LIST_FILE), buf);
		}
		ListView_SetItemState(GetDlgItem(hWnd, IDC_LIST_FILE), 0, LVIS_FOCUSED, LVIS_FOCUSED);
		modify = FALSE;
		break;

	case WM_GET_FILEDATA:
		if (lock == TRUE || modify == FALSE) {
			return (LRESULT)NULL;
		}
		return (LRESULT)lv_item_to_hdrop(GetDlgItem(hWnd, IDC_LIST_FILE));

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

/*
 * fileview_regist - �E�B���h�E�N���X�̓o�^
 */
BOOL fileview_regist(const HINSTANCE hInstance)
{
	WNDCLASS wc;

	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC)fileview_proc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WINDOW_CLASS;
	// �E�B���h�E�N���X�̓o�^
	return RegisterClass(&wc);
}

/*
 * fileview_create - �t�@�C���r���[�A�̍쐬
 */
HWND fileview_create(const HINSTANCE hInstance, const HWND pWnd, int id)
{
	HWND hWnd;

	// �E�B���h�E�̍쐬
	hWnd = CreateWindowEx(WS_EX_ACCEPTFILES, WINDOW_CLASS,
		TEXT(""),
		WS_TABSTOP | WS_CHILD,
		0, 0, 0, 0, pWnd, (HMENU)id, hInstance, NULL);
	return hWnd;
}
/* End of source */
