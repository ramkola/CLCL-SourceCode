/*
 * CLCL
 *
 * fmt_Bitmap.c
 *
 * Copyright (C) 1996-2019 by Ohno Tomoaki. All rights reserved.
 *		https://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

#include "General.h"
#include "Memory.h"
#include "Message.h"
#include "Data.h"
#include "Ini.h"
#include "File.h"
#include "Bitmap.h"
#include "fmt_bitmap_view.h"
#include "gdip.h"
#include "dpi.h"

#include "resource.h"

/* Define */
#define BITMAP_NEW_WIDTH			1
#define BITMAP_NEW_HEIGHT			1

/* Global Variables */
static HICON bmp_icon;
static HWND hBmpWnd;

extern HINSTANCE hInst;
extern OPTION_INFO option;

/* Local Function Prototypes */

/*
 * bitmap_initialize - ������
 */
__declspec(dllexport) BOOL CALLBACK bitmap_initialize(void)
{
	if (bmp_icon == NULL) {
		bmp_icon = LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON_BITMAP), IMAGE_ICON, Scale(16), Scale(16), 0);
	}
	bmpview_regist(hInst);
	init_gdip();
	return TRUE;
}

/*
 * bitmap_get_icon - �`���p�̃A�C�R�����擾
 */
__declspec(dllexport) HICON CALLBACK bitmap_get_icon(const int icon_size, BOOL *free_icon)
{
	*free_icon = FALSE;
	return bmp_icon;
}

/*
 * bitmap_free - �I������
 */
__declspec(dllexport) BOOL CALLBACK bitmap_free(void)
{
	if (bmp_icon != NULL) {
		DestroyIcon(bmp_icon);
		bmp_icon = NULL;
	}
	shutdown_gdip();
	return TRUE;
}

/*
 * bitmap_initialize_item - �A�C�e�����̏�����
 */
__declspec(dllexport) BOOL CALLBACK bitmap_initialize_item(DATA_INFO *di, const BOOL set_init_data)
{
	return FALSE;
}

/*
 * bitmap_copy_data - �f�[�^�̃R�s�[
 */
__declspec(dllexport) HANDLE CALLBACK bitmap_copy_data(const TCHAR *format_name, const HANDLE data, DWORD *ret_size)
{
	return NULL;
}

/*
 * bitmap_data_to_bytes - �f�[�^���o�C�g��ɕϊ�
 */
__declspec(dllexport) BYTE* CALLBACK bitmap_data_to_bytes(const DATA_INFO *di, DWORD *ret_size)
{
	return NULL;
}

/*
 * bitmap_bytes_to_data - �o�C�g����f�[�^�ɕϊ�
 */
__declspec(dllexport) HANDLE CALLBACK bitmap_bytes_to_data(const TCHAR *format_name, const BYTE *data, DWORD *size)
{
	return NULL;
}

/*
 * bitmap_get_file_info - �R�����_�C�A���O���̎擾
 */
__declspec(dllexport) int CALLBACK bitmap_get_file_info(const TCHAR *format_name, const DATA_INFO *di, OPENFILENAME *of, const BOOL mode)
{
	of->lpstrFilter = TEXT("*.bmp\0*.bmp\0*.png\0*.png\0*.jpg\0*.jpg\0*.*\0*.*\0\0");
	of->nFilterIndex = 1;
	of->lpstrDefExt = TEXT("bmp");
	return 1;
}

/*
 * bitmap_data_to_file - �f�[�^���t�@�C���ɕۑ�
 */
__declspec(dllexport) BOOL CALLBACK bitmap_data_to_file(DATA_INFO *di, const TCHAR *file_name, const int filter_index, TCHAR *err_str)
{
	BITMAPFILEHEADER hdr;
	PBITMAPINFOHEADER pbih;
	BYTE *mem;
	BYTE *save_mem;
	TCHAR* ext;
	DWORD size;
	int plt = 0;

	if (di->data == NULL) {
		return FALSE;
	}

	ext = PathFindExtension(file_name);
	if (lstrcmpi(ext, TEXT(".png")) == 0 || lstrcmpi(ext, TEXT(".jpg")) == 0 || lstrcmpi(ext, TEXT(".jpeg")) == 0) {
		HBITMAP hbmp = NULL;
		if (lstrcmpi(di->format_name, TEXT("BITMAP")) != 0) {
			if ((mem = GlobalLock(di->data)) == NULL) {
				message_get_error(GetLastError(), err_str);
				return FALSE;
			}
			size = di->size;
			if ((mem = GlobalLock(di->data)) == NULL) {
				return FALSE;
			}
			hbmp = dib_to_bitmap(mem);
			GlobalUnlock(di->data);
		}
		else {
			hbmp = (HBITMAP)di->data;
		}
		if (lstrcmpi(ext, TEXT(".png")) == 0) {
			// PNG�ŕۑ�
			if (save_png(hbmp, file_name) == 0) {
				return FALSE;
			}
		}
		else {
			// JPEG�ŕۑ�
			if (save_jpeg(hbmp, file_name, 90) == 0) {
				return FALSE;
			}
		}
	} else {
		// BMP�ŕۑ�
		if (lstrcmpi(di->format_name, TEXT("BITMAP")) != 0) {
			if ((mem = GlobalLock(di->data)) == NULL) {
				message_get_error(GetLastError(), err_str);
				return FALSE;
			}
			size = di->size;
		}
		else {
			if ((mem = bitmap_to_dib(di->data, &size)) == NULL) {
				message_get_error(GetLastError(), err_str);
				return FALSE;
			}
		}
		// BITMAP�t�@�C���̍쐬
		pbih = (PBITMAPINFOHEADER)mem;
		if ((plt = pbih->biClrUsed) == 0) {
			switch (pbih->biPlanes * pbih->biBitCount) {
			case 1:
				plt = 2;
				break;
			case 4:
				plt = 16;
				break;
			case 8:
				plt = 256;
				break;
			case 16:
			case 32:
				plt = 3;
				break;
			}
		}
		hdr.bfType = 0x4d42;
		hdr.bfSize = sizeof(BITMAPFILEHEADER) + size;
		hdr.bfReserved1 = 0;
		hdr.bfReserved2 = 0;
		hdr.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + plt * sizeof(RGBQUAD);

		if ((save_mem = mem_alloc(sizeof(BITMAPFILEHEADER) + size)) == NULL) {
			message_get_error(GetLastError(), err_str);
			if (lstrcmpi(di->format_name, TEXT("BITMAP")) != 0) {
				GlobalUnlock(di->data);
			}
			else {
				mem_free(&mem);
			}
			return FALSE;
		}
		CopyMemory(save_mem, &hdr, sizeof(BITMAPFILEHEADER));
		CopyMemory(save_mem + sizeof(BITMAPFILEHEADER), mem, size);

		// �t�@�C���ɏ�������
		if (file_write_buf(file_name, save_mem, sizeof(BITMAPFILEHEADER) + size, err_str) == FALSE) {
			mem_free(&save_mem);
			if (lstrcmpi(di->format_name, TEXT("BITMAP")) != 0) {
				GlobalUnlock(di->data);
			}
			else {
				mem_free(&mem);
			}
			return FALSE;
		}
		mem_free(&save_mem);
		if (lstrcmpi(di->format_name, TEXT("BITMAP")) != 0) {
			GlobalUnlock(di->data);
		}
		else {
			mem_free(&mem);
		}
	}
	return TRUE;
}

/*
 * bitmap_file_to_data - �t�@�C������f�[�^���쐬
 */
__declspec(dllexport) HANDLE CALLBACK bitmap_file_to_data(const TCHAR *file_name, const TCHAR *format_name, DWORD *ret_size, TCHAR *err_str)
{
	HANDLE ret;
	BYTE *data;
	BYTE *mem;
	TCHAR* ext;
	DWORD size;

	ext = PathFindExtension(file_name);
	if (lstrcmpi(ext, TEXT(".png")) == 0 || lstrcmpi(ext, TEXT(".jpg")) == 0 || lstrcmpi(ext, TEXT(".jpeg")) == 0) {
		HBITMAP hbmp = image_to_bitmap(GetDC(NULL), file_name);
		if (lstrcmpi(format_name, TEXT("BITMAP")) != 0) {
			if ((data = bitmap_to_dib(hbmp, &size)) == NULL) {
				message_get_error(GetLastError(), err_str);
				DeleteObject(hbmp);
				return NULL;
			}
			// �R�s�[��m��
			if ((ret = GlobalAlloc(GHND, size)) == NULL) {
				message_get_error(GetLastError(), err_str);
				mem_free(&data);
				DeleteObject(hbmp);
				return NULL;
			}
			// �R�s�[�惍�b�N
			if ((mem = GlobalLock(ret)) == NULL) {
				message_get_error(GetLastError(), err_str);
				GlobalFree(ret);
				mem_free(&data);
				DeleteObject(hbmp);
				return NULL;
			}
			// �R�s�[
			CopyMemory(mem, data, size);
			// ���b�N����
			GlobalUnlock(ret);

			if (ret_size != NULL) {
				*ret_size = size;
			}
			mem_free(&data);
			DeleteObject(hbmp);
		}
		else {
			ret = hbmp;
			if (ret_size != NULL) {
				mem = bitmap_to_dib(hbmp, ret_size);
				if (mem != NULL) {
					mem_free(&mem);
				}
			}
		}
	}
	else {
		// �t�@�C���̓ǂݍ���
		if ((data = file_read_buf(file_name, &size, err_str)) == NULL) {
			return NULL;
		}
		if (lstrcmpi(format_name, TEXT("BITMAP")) != 0) {
			// �R�s�[��m��
			if ((ret = GlobalAlloc(GHND, size - sizeof(BITMAPFILEHEADER))) == NULL) {
				message_get_error(GetLastError(), err_str);
				mem_free(&data);
				return NULL;
			}
			// �R�s�[�惍�b�N
			if ((mem = GlobalLock(ret)) == NULL) {
				message_get_error(GetLastError(), err_str);
				GlobalFree(ret);
				mem_free(&data);
				return NULL;
			}
			// �R�s�[
			CopyMemory(mem, data + sizeof(BITMAPFILEHEADER), size - sizeof(BITMAPFILEHEADER));
			// ���b�N����
			GlobalUnlock(ret);

		}
		else {
			if ((ret = dib_to_bitmap(data + sizeof(BITMAPFILEHEADER))) == NULL) {
				message_get_error(GetLastError(), err_str);
			}
		}
		if (ret_size != NULL) {
			*ret_size = size - sizeof(BITMAPFILEHEADER);
		}
		mem_free(&data);
	}
	return ret;
}

/*
 * bitmap_free_data - �f�[�^�̉��
 */
__declspec(dllexport) BOOL CALLBACK bitmap_free_data(const TCHAR *format_name, HANDLE data)
{
	return FALSE;
}

/*
 * bitmap_free_item - �A�C�e�����̉��
 */
__declspec(dllexport) BOOL CALLBACK bitmap_free_item(DATA_INFO *di)
{
	return FALSE;
}

/*
 * bitmap_get_menu_title - ���j���[�^�C�g���̎擾
 */
__declspec(dllexport) BOOL CALLBACK bitmap_get_menu_title(DATA_INFO *di)
{
	return FALSE;
}

/*
 * bitmap_get_menu_icon - ���j���[�^�C�g��
 */
__declspec(dllexport) BOOL CALLBACK bitmap_get_menu_icon(DATA_INFO *di, const int icon_size)
{
	di->menu_icon = bmp_icon;
	di->free_icon = FALSE;
	return TRUE;
}

/*
 * bitmap_get_menu_bitmap - ���j���[�p�r�b�g�}�b�v
 */
__declspec(dllexport) BOOL CALLBACK bitmap_get_menu_bitmap(DATA_INFO *di, const int width, const int height)
{
	HDC hdc;
	HDC from_dc;
	HDC to_dc;
	HBITMAP hbmp;
	HBITMAP old_from_hbmp;
	HBITMAP old_to_hbmp;
	BITMAP bmp;
	BYTE *mem;
	OSVERSIONINFO osvi;

	if (di->data == NULL) {
		return FALSE;
	}

	if (lstrcmpi(di->format_name, TEXT("BITMAP")) != 0) {
		if ((mem = GlobalLock(di->data)) == NULL) {
			return FALSE;
		}
		hbmp = dib_to_bitmap(mem);
		GlobalUnlock(di->data);
	} else {
		hbmp = di->data;
	}

	// ���j���[�ɕ\������r�b�g�}�b�v�̍쐬
	GetObject(hbmp, sizeof(BITMAP), &bmp);

	hdc = GetDC(NULL);

	from_dc = CreateCompatibleDC(hdc);
	old_from_hbmp = SelectObject(from_dc, hbmp);

	to_dc = CreateCompatibleDC(hdc);
	di->menu_bitmap = CreateCompatibleBitmap(hdc, width, height);
	di->free_bitmap = TRUE;
	old_to_hbmp = SelectObject(to_dc, di->menu_bitmap);

	//OS�o�[�W�����̃`�F�b�N
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
		(width < bmp.bmWidth || height < bmp.bmHeight)) {
		SetStretchBltMode(to_dc, HALFTONE);
		SetBrushOrgEx(to_dc, 0, 0, NULL);
	} else {
		SetStretchBltMode(to_dc, COLORONCOLOR);
	}
	StretchBlt(to_dc, 0, 0, width, height,
		from_dc, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

	SelectObject(from_dc, old_from_hbmp);
	DeleteDC(from_dc);

	SelectObject(to_dc, old_to_hbmp);
	DeleteDC(to_dc);

	ReleaseDC(NULL, hdc);

	if (lstrcmpi(di->format_name, TEXT("BITMAP")) != 0) {
		DeleteObject((HGDIOBJ)hbmp);
	}
	return TRUE;
}

/*
 * bitmap_get_tooltip_text - ���j���[�p�c�[���`�b�v�e�L�X�g
 */
__declspec(dllexport) TCHAR* CALLBACK bitmap_get_tooltip_text(DATA_INFO *di)
{
	BITMAP bmp;
	HBITMAP hbmp;
	BYTE *mem;
	TCHAR buf[BUF_SIZE];
	TCHAR *ret;

	if (di->data == NULL) {
		return NULL;
	}
	if (lstrcmpi(di->format_name, TEXT("BITMAP")) != 0) {
		if ((mem = GlobalLock(di->data)) == NULL) {
			return NULL;
		}
		hbmp = dib_to_bitmap(mem);
		GlobalUnlock(di->data);
	} else {
		hbmp = di->data;
	}

	// �摜���擾
	GetObject(hbmp, sizeof(BITMAP), &bmp);
	if (di->size < 1024) {
		wsprintf(buf, TEXT("%u x %u (%d bytes)"), bmp.bmWidth, bmp.bmHeight, di->size);
	} else {
		wsprintf(buf, TEXT("%u x %u (%d KB)"), bmp.bmWidth, bmp.bmHeight, di->size / 1024);
	}
	ret = alloc_copy(buf);

	if (lstrcmpi(di->format_name, TEXT("BITMAP")) != 0) {
		DeleteObject((HGDIOBJ)hbmp);
	}
	return ret;
}

/*
 * bitmap_window_create - �f�[�^�\���E�B���h�E�̍쐬
 */
__declspec(dllexport) HWND CALLBACK bitmap_window_create(const HWND parent_wnd)
{
	if (hBmpWnd == NULL) {
		hBmpWnd = bmpview_create(hInst, parent_wnd, 0);
	}
	return hBmpWnd;
}

/*
 * bitmap_window_destroy - �f�[�^�\���E�B���h�E�̔j��
 */
__declspec(dllexport) BOOL CALLBACK bitmap_window_destroy(const HWND hWnd)
{
	hBmpWnd = NULL;
	return TRUE;
}

/*
 * bitmap_window_show_data - �f�[�^�̕\��
 */
__declspec(dllexport) BOOL CALLBACK bitmap_window_show_data(const HWND hWnd, DATA_INFO *di, const BOOL lock)
{
	SendMessage(hWnd, WM_SET_BMPDATA, 0, (LPARAM)di);
	return TRUE;
}

/*
 * bitmap_window_save_data - �f�[�^�̕ۑ�
 */
__declspec(dllexport) BOOL CALLBACK bitmap_window_save_data(const HWND hWnd, DATA_INFO *di)
{
	return FALSE;
}

/*
 * bitmap_window_hide_data - �f�[�^�̔�\��
 */
__declspec(dllexport) BOOL CALLBACK bitmap_window_hide_data(const HWND hWnd, DATA_INFO *di)
{
	SendMessage(hWnd, WM_SET_BMPDATA, 0, 0);
	return TRUE;
}
/* End of source */
