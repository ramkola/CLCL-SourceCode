/*
 * CLCL
 *
 * BinView.c
 *
 * Copyright (C) 1996-2019 by Ohno Tomoaki. All rights reserved.
 *		https://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#ifdef OP_XP_STYLE
#include <uxtheme.h>
#include <vssym32.h>
#endif	// OP_XP_STYLE

#include "General.h"
#include "Memory.h"
#include "String.h"
#include "Message.h"
#include "Ini.h"
#include "Data.h"
#include "Format.h"
#include "ClipBoard.h"
#include "Font.h"
#include "BinView.h"

#include "resource.h"

/* Define */
#define WINDOW_CLASS					TEXT("CLCLBinView")

#define WM_SET_SCROLLBAR				(WM_APP + 101)
#define WM_SHOW_MENU					(WM_APP + 102)

// �z�C�[�����b�Z�[�W
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL					0x020A
#endif
// XP�e�[�}�ύX�ʒm
#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED					0x031A
#endif
// Redo
#ifndef EM_REDO
#define EM_REDO							(WM_USER + 84)
#endif
#ifndef EM_CANREDO
#define EM_CANREDO						(WM_USER + 85)
#endif

#define RESERVE_BUF						1024
#define RESERVE_UNDO					256

#define ADDRESS_LEN						8
#define LINE_LEN						16
#define LINE_WIDTH						(ADDRESS_LEN + 2 + LINE_LEN * 3 + 2 + LINE_LEN + 2)

#define UNDO_TYPE_INPUT					1
#define UNDO_TYPE_DELETE				2

/* Global Variables */
typedef struct _UNDO {
	int type;

	int st;
	BYTE data;
} UNDO;

typedef struct _BUFFER {
	// �\������f�[�^
	BYTE *data;
	DWORD data_len;
	DWORD data_size;

	// UNDO�o�b�t�@
	UNDO *undo;
	int undo_size;
	int undo_len;

	// �I���ʒu
	BYTE *sp;
	// ���̓J�E���g
	int input_cnt;

	// �s��
	int height;
	// �X�N���[�� �o�[�̌��݈ʒu
	int pos_x;
	int pos_y;
	// �X�N���[�� �o�[�̍ő�l
	int max_x;
	int max_y;

	// �`��p���
	HDC mdc;
	HBITMAP ret_bmp;
	HBRUSH hbrush;
	HFONT hfont;
	HFONT ret_font;

	// �t�H���g�̍���
	int font_height;
	// 1�����̃t�H���g�̕�
	int char_width;
	// �s��
	int spacing;
	// ���}�[�W��
	int left_margin;

	// ���b�N���
	BOOL lock;
	// �ύX�t���O
	BOOL modified;
	// �}�����[�h
	BOOL insert_mode;
	// �}���`�o�C�g
	BOOL dbcs;

#ifdef OP_XP_STYLE
	// XP
	HTHEME hTheme;
#endif	// OP_XP_STYLE
} BUFFER;

// �I�v�V����
extern OPTION_INFO option;

/* Local Function Prototypes */
static BOOL binview_select_font(const HWND hWnd);
static void binview_refresh_line(const HWND hWnd, const BUFFER *bf, const BYTE *st, const BYTE *en);
static BYTE *binview_point_to_select(const HWND hWnd, const BUFFER *bf, const int x, const int y);
static void binview_ensure_visible(const HWND hWnd, BUFFER *bf);
static void binview_set_scrollbar(const HWND hWnd, BUFFER *bf);

static BOOL binview_set_undo(BUFFER *bf, const int type);
static BOOL binview_undo_exec(const HWND hWnd, BUFFER *bf);
static BOOL binview_redo_exec(const HWND hWnd, BUFFER *bf);

static BOOL binview_insert(const HWND hWnd, BUFFER *bf);
static BOOL binview_input(const HWND hWnd, BUFFER *bf, const TCHAR c);
static void binview_delete(const HWND hWnd, BUFFER *bf);
static void binview_flush(BUFFER *bf);

static void itox(const DWORD num, const int col, TCHAR *ret);
static BOOL draw_init(const HWND hWnd, BUFFER *bf);
static void draw_free(BUFFER *bf);
static void binview_draw_line(const HWND hWnd, const HDC mdc, BUFFER *bf, const int i);
static LRESULT CALLBACK binview_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/*
 * txtview_select_font - �t�H���g�̑I��
 */
static BOOL binview_select_font(const HWND hWnd)
{
	CHOOSEFONT cf;
	LOGFONT lf;
	HDC hdc;

	// �t�H���g���̍쐬
	ZeroMemory(&lf, sizeof(LOGFONT));
	hdc = GetDC(NULL);
	lf.lfHeight = -(int)((option.bin_font_size * GetDeviceCaps(hdc, LOGPIXELSY)) / 72);
	ReleaseDC(NULL, hdc);
	lf.lfWeight = option.bin_font_weight;
	lf.lfItalic = option.bin_font_italic;
	lf.lfCharSet = option.bin_font_charset;
	lstrcpy(lf.lfFaceName, option.bin_font_name);

	// �t�H���g�I���_�C�A���O��\��
	ZeroMemory(&cf, sizeof(CHOOSEFONT));
	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hwndOwner = hWnd;
	cf.lpLogFont = &lf;
	cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_FIXEDPITCHONLY;
	cf.nFontType = SCREEN_FONTTYPE;
	if (ChooseFont(&cf) == FALSE) {
		return FALSE;
	}

	// �ݒ�擾
	mem_free(&option.bin_font_name);
	option.bin_font_name = alloc_copy(lf.lfFaceName);
	option.bin_font_weight = lf.lfWeight;
	option.bin_font_italic = lf.lfItalic;
	option.bin_font_charset = lf.lfCharSet;
	option.bin_font_size = cf.iPointSize / 10;
	return TRUE;
}

/*
 * binview_refresh_line - �P�s�X�V
 */
static void binview_refresh_line(const HWND hWnd, const BUFFER *bf, const BYTE *st, const BYTE *en)
{
	RECT rect;
	int i, j;

	GetClientRect(hWnd, &rect);

	i = ((st - bf->data) / LINE_LEN - bf->pos_y) * bf->font_height;
	j = ((en - bf->data) / LINE_LEN - bf->pos_y) * bf->font_height + bf->font_height;
	if (i > 0) {
		rect.top = i;
	}
	if (j < rect.bottom) {
		rect.bottom = j;
	}
	InvalidateRect(hWnd, &rect, FALSE);
}

/*
 * binview_point_to_select - ���W���當���ʒu���擾
 */
static BYTE *binview_point_to_select(const HWND hWnd, const BUFFER *bf, const int x, const int y)
{
	RECT rect;
	BYTE *p;
	int offset;
	int i, j;

	GetClientRect(hWnd, &rect);
	i = bf->pos_y + (y / bf->font_height);
	if (i < 0) {
		i = 0;
	}
	if (i >= bf->height) {
		i = bf->height - 1;
	}

	// �A�h���X
	offset = bf->left_margin - (bf->pos_x * bf->char_width);
	offset += bf->char_width * ADDRESS_LEN;
	offset += bf->char_width * 2;

	// 16�i
	for (p = (BYTE *)bf->data + i * LINE_LEN, j = 0; j < LINE_LEN; p++, j++) {
		if (j == LINE_LEN / 2) {
			offset += bf->char_width;
		}
		offset += (bf->char_width * 2);
		if (x <= offset) {
			if ((DWORD)(p - (BYTE *)bf->data) > (DWORD)bf->data_len) {
				return bf->data + bf->data_len;
			}
			return p;
		}
		offset += bf->char_width;
	}
	offset += bf->char_width;

	// �L�����N�^
	for (p = (BYTE *)bf->data + i * LINE_LEN, j = 0; j < LINE_LEN; p++, j++) {
		offset += bf->char_width;
		if (x <= offset) {
			if ((DWORD)(p - (BYTE *)bf->data) > (DWORD)bf->data_len) {
				return bf->data + bf->data_len;
			}
			return p;
		}
	}
	if ((DWORD)(p - (BYTE *)bf->data) > (DWORD)bf->data_len) {
		return bf->data + bf->data_len;
	}
	return (p - 1);
}

/*
 * binview_ensure_visible - �I���ʒu��\��
 */
static void binview_ensure_visible(const HWND hWnd, BUFFER *bf)
{
	RECT rect;
	int i;
	int x;

	GetClientRect(hWnd, &rect);

	// x
	i = bf->pos_x;
	x = ADDRESS_LEN + 2 + ((bf->sp - bf->data) % LINE_LEN) * 3;
	if ((bf->sp - bf->data) % LINE_LEN >= LINE_LEN / 2) {
		x++;
	}
	if (x < bf->pos_x) {
		bf->pos_x = x;
		if (ADDRESS_LEN + 2 == bf->pos_x) {
			bf->pos_x = 0;
		}
	}
	if (x > bf->pos_x + rect.right / bf->char_width - 2) {
		bf->pos_x = x - rect.right / bf->char_width + 2;
		if (bf->pos_x > bf->max_x) {
			bf->pos_x = bf->max_x;
		}
	}
	if (i != bf->pos_x) {
		SetScrollPos(hWnd, SB_HORZ, bf->pos_x, TRUE);
		ScrollWindowEx(hWnd, (i - bf->pos_x) * bf->char_width, 0, NULL, &rect, NULL, NULL, SW_INVALIDATE | SW_ERASE);
	}

	// y
	i = bf->pos_y;
	if ((bf->sp - bf->data) / LINE_LEN < bf->pos_y) {
		bf->pos_y = (bf->sp - bf->data) / LINE_LEN;
	}
	if ((bf->sp - bf->data) / LINE_LEN > bf->pos_y + rect.bottom / bf->font_height - 1) {
		bf->pos_y = (bf->sp - bf->data) / LINE_LEN - rect.bottom / bf->font_height + 1;
		if (bf->pos_y > bf->max_y) {
			bf->pos_y = bf->max_y;
		}
	}
	if (i != bf->pos_y) {
		SetScrollPos(hWnd, SB_VERT, bf->pos_y, TRUE);
		ScrollWindowEx(hWnd, 0, (i - bf->pos_y) * bf->font_height, NULL, &rect, NULL, NULL, SW_INVALIDATE | SW_ERASE);
	}
}

/*
 * binview_set_scrollbar - �X�N���[���o�[�ݒ�
 */
static void binview_set_scrollbar(const HWND hWnd, BUFFER *bf)
{
	SCROLLINFO si;
	RECT rect;

	GetClientRect(hWnd, &rect);

	// ���X�N���[���o�[
	if ((rect.right / bf->char_width) < LINE_WIDTH) {
		EnableScrollBar(hWnd, SB_HORZ, ESB_ENABLE_BOTH);

		bf->max_x = LINE_WIDTH - (rect.right / bf->char_width);
		bf->pos_x = (bf->pos_x < bf->max_x) ? bf->pos_x : bf->max_x;

		ZeroMemory(&si, sizeof(SCROLLINFO));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
		si.nPage = rect.right / bf->char_width;
		si.nMax = LINE_WIDTH - 1;
		si.nPos = bf->pos_x;
		SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
	} else {
		EnableScrollBar(hWnd, SB_HORZ, ESB_DISABLE_BOTH);

		bf->pos_x = bf->max_x = 0;

		ZeroMemory(&si, sizeof(SCROLLINFO));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS | SIF_PAGE | SIF_RANGE;
		si.nMax = 1;
		SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
	}

	// �c�X�N���[���o�[
	if (rect.bottom / bf->font_height < bf->height) {
		EnableScrollBar(hWnd, SB_VERT, ESB_ENABLE_BOTH);

		bf->max_y = bf->height - (rect.bottom / bf->font_height);
		bf->pos_y = (bf->pos_y < bf->max_y) ? bf->pos_y : bf->max_y;

		ZeroMemory(&si, sizeof(SCROLLINFO));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
		si.nPage = rect.bottom / bf->font_height;
		si.nMax = bf->height - 1;
		si.nPos = bf->pos_y;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
	} else {
		EnableScrollBar(hWnd, SB_VERT, ESB_DISABLE_BOTH);

		bf->pos_y = bf->max_y = 0;

		ZeroMemory(&si, sizeof(SCROLLINFO));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS | SIF_PAGE | SIF_RANGE;
		si.nMax = 1;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
	}
}

/*
 * binview_set_undo - UNDO�ǉ�
 */
static BOOL binview_set_undo(BUFFER *bf, const int type)
{
	UNDO *ud;
	int i;

	// �J�����g�ʒu�ȍ~��������
	for (i = bf->undo_len; i < bf->undo_size; i++) {
		(bf->undo + i)->type = 0;
	}

	i = bf->undo_len;
	if (bf->undo_len + 1 >= bf->undo_size) {
		// UNDO�o�b�t�@�̊m��
		bf->undo_size += RESERVE_UNDO;
		if ((ud = mem_calloc(sizeof(UNDO) * bf->undo_size)) == NULL) {
			return FALSE;
		}
		if (bf->undo != NULL) {
			CopyMemory(ud, bf->undo, sizeof(UNDO) * bf->undo_len);
			mem_free(&bf->undo);
		}
		bf->undo = ud;
	}

	// UNDO�ݒ�
	(bf->undo + i)->type = type;
	(bf->undo + i)->st = bf->sp - bf->data;
	(bf->undo + i)->data = *bf->sp;
	bf->undo_len++;
	return TRUE;
}

/*
 * binview_undo_exec - UNDO�̎��s
 */
static BOOL binview_undo_exec(const HWND hWnd, BUFFER *bf)
{
	int i;

	binview_flush(bf);

	i = bf->undo_len - 1;
	if (i < 0) {
		return TRUE;
	}
	binview_refresh_line(hWnd, bf, bf->sp, bf->sp);
	bf->sp = bf->data + (bf->undo + i)->st;
	switch ((bf->undo + i)->type) {
	case UNDO_TYPE_INPUT:
		// ���͕����̍폜
		binview_delete(hWnd, bf);
		break;

	case UNDO_TYPE_DELETE:
		// �폜�����̒ǉ�
		binview_insert(hWnd, bf);
		*bf->sp = (bf->undo + i)->data;
		binview_refresh_line(hWnd, bf, bf->sp, bf->sp);
		binview_ensure_visible(hWnd, bf);
		break;
	}
	bf->undo_len--;
	return TRUE;
}

/*
 * binview_redo_exec - REDO�̎��s
 */
static BOOL binview_redo_exec(const HWND hWnd, BUFFER *bf)
{
	int i;

	binview_flush(bf);

	i = bf->undo_len;
	if (bf->undo == NULL || (bf->undo + i)->type == 0) {
		return TRUE;
	}
	binview_refresh_line(hWnd, bf, bf->sp, bf->sp);
	bf->sp = bf->data + (bf->undo + i)->st;
	switch ((bf->undo + i)->type) {
	case UNDO_TYPE_INPUT:
		// �����ǉ�
		binview_insert(hWnd, bf);
		*bf->sp = (bf->undo + i)->data;
		binview_refresh_line(hWnd, bf, bf->sp, bf->sp);
		binview_ensure_visible(hWnd, bf);
		break;

	case UNDO_TYPE_DELETE:
		// �����폜
		binview_delete(hWnd, bf);
		break;
	}
	bf->undo_len++;
	return TRUE;
}

/*
 * binview_insert - �}��
 */
static BOOL binview_insert(const HWND hWnd, BUFFER *bf)
{
	BYTE *p;

	if (bf->data == NULL) {
		// �V�K�m��
		bf->data_size = RESERVE_BUF;
		bf->data = mem_alloc(sizeof(BYTE) * bf->data_size);
		if (bf->data == NULL) {
			return FALSE;
		}
		bf->sp = bf->data;
	} else {
		// �}��
		if (bf->data_len + 1 >= bf->data_size) {
			bf->data_size = bf->data_len + 1 + RESERVE_BUF;
			p = mem_alloc(sizeof(BYTE) * bf->data_size);
			if (p == NULL) {
				return FALSE;
			}
			CopyMemory(p, bf->data, bf->sp - bf->data);
			CopyMemory(p + (bf->sp - bf->data) + 1, bf->data + (bf->sp - bf->data), bf->data_len - (bf->sp - bf->data));
			bf->sp = p + (bf->sp - bf->data);
			mem_free(&bf->data);
			bf->data = p;
		} else {
			MoveMemory(bf->data + (bf->sp - bf->data) + 1, bf->data + (bf->sp - bf->data), bf->data_len - (bf->sp - bf->data));
		}
	}
	*bf->sp = 0;
	bf->data_len++;
	if (bf->height != (int)(bf->data_len / LINE_LEN + 1)) {
		bf->height = bf->data_len / LINE_LEN + 1;
		SendMessage(hWnd, WM_SET_SCROLLBAR, 0, 0);
	}
	binview_refresh_line(hWnd, bf, bf->sp, bf->data + bf->data_len);
	return TRUE;
}

/*
 * binview_input - ����
 */
static BOOL binview_input(const HWND hWnd, BUFFER *bf, const TCHAR c)
{
	int i;

	// 16�i���ɕϊ�
	if (c >= TEXT('0') && c <= TEXT('9')) {
		i = c - TEXT('0');
	} else if (c >= TEXT('A') && c <= TEXT('F')) {
		i = c - TEXT('A') + 10;
	} else if (c >= TEXT('a') && c <= TEXT('f')) {
		i = c - TEXT('a') + 10;
	} else {
		return FALSE;
	}

	if (bf->data == NULL || bf->sp == bf->data + bf->data_len || (bf->insert_mode == TRUE && bf->input_cnt == 0)) {
		// �}��
		if (binview_insert(hWnd, bf) == FALSE) {
			return FALSE;
		}
	} else if (bf->input_cnt == 0) {
		binview_set_undo(bf, UNDO_TYPE_DELETE);
	}

	// �����ݒ�
	*bf->sp = *bf->sp << 4 | i;
	bf->modified = TRUE;
	binview_refresh_line(hWnd, bf, bf->sp, bf->sp);

	// ���̕����ֈړ�
	bf->input_cnt++;
	if (bf->input_cnt >= 2) {
		binview_flush(bf);
		if ((DWORD)(bf->sp - (BYTE *)bf->data + 1) <= (DWORD)bf->data_len) {
			bf->sp++;
			binview_refresh_line(hWnd, bf, bf->sp, bf->sp);
		}
	}
	binview_ensure_visible(hWnd, bf);
	return TRUE;
}

/*
 * binview_delete - �폜
 */
static void binview_delete(const HWND hWnd, BUFFER *bf)
{
	// �폜
	MoveMemory(bf->sp, bf->sp + 1, bf->data_len - (bf->sp - bf->data) - 1);
	bf->data_len--;
	if (bf->data_len <= 0) {
		// ���
		bf->sp = NULL;
		mem_free(&bf->data);
		bf->data_size = 0;
		bf->data_len = 0;
	}
	if (bf->height != (int)(bf->data_len / LINE_LEN + 1)) {
		bf->height = bf->data_len / LINE_LEN + 1;
		SendMessage(hWnd, WM_SET_SCROLLBAR, 0, 0);
		InvalidateRect(hWnd, NULL, FALSE);
	} else {
		binview_refresh_line(hWnd, bf, bf->sp, bf->data + bf->data_len);
	}
	binview_ensure_visible(hWnd, bf);
	bf->modified = TRUE;
}

/*
 * binview_flush - ���͂̔��f
 */
static void binview_flush(BUFFER *bf)
{
	if (bf->input_cnt != 0) {
		binview_set_undo(bf, UNDO_TYPE_INPUT);
	}
	bf->input_cnt = 0;
}

/*
 * itox - 16�i������̍쐬
 */
static void itox(const DWORD num, const int col, TCHAR *ret)
{
	int i, wk;

	for (i = col - 1; i > 0; i--) {
		wk = (num >> (4 * i)) & 0x0F;
		*(ret++) = (wk >= 10) ? TEXT('A') + wk - 10 : TEXT('0') + wk;
	}
	wk = num & 0x0F;
	*(ret++) = (wk >= 10) ? TEXT('A') + wk - 10 : TEXT('0') + wk;
	*ret = TEXT('\0');
}

/*
 * draw_init - �`����̏�����
 */
static BOOL draw_init(const HWND hWnd, BUFFER *bf)
{
	HDC hdc;
	HBITMAP hBmp;
	RECT rect;
	TEXTMETRIC tm;

	GetClientRect(hWnd, &rect);
	GetTextMetrics(bf->mdc, &tm);

	hdc = GetDC(hWnd);
	hBmp = CreateCompatibleBitmap(hdc, rect.right, tm.tmHeight + bf->spacing);
	bf->ret_bmp = SelectObject(bf->mdc, hBmp);
	ReleaseDC(hWnd, hdc);
	return TRUE;
}

/*
 * draw_free - �`����̉��
 */
static void draw_free(BUFFER *bf)
{
	HBITMAP hBmp;

	hBmp = SelectObject(bf->mdc, bf->ret_bmp);
	DeleteObject(hBmp);
}

/*
 * binview_draw_line - 1�s�`��
 */
static void binview_draw_line(const HWND hWnd, const HDC mdc, BUFFER *bf, const int i)
{
	RECT drect;
	HBRUSH hbr;
	TCHAR buf[BUF_SIZE];
	TCHAR *tp, *s;
	BYTE cbuf[BUF_SIZE];
	BYTE *p, *r;
	int offset;
	int height;
	int len;
	int j;
	int sel = -1;

	if ((DWORD)(i * LINE_LEN) > (DWORD)bf->data_len) {
		return;
	}

	// �ʒu���
	offset = bf->left_margin - (bf->pos_x * bf->char_width);
	height = bf->spacing / 2;

	// �w�i�F
	SetBkColor(mdc, GetSysColor(COLOR_WINDOW));

	// �A�h���X�\��
	if (bf->lock == FALSE && option.bin_lock == 0) {
		SetTextColor(mdc, GetSysColor(COLOR_HIGHLIGHT));
	} else {
		SetTextColor(mdc, GetSysColor(COLOR_GRAYTEXT));
	}
	itox(i * LINE_LEN, ADDRESS_LEN, buf);
	TextOut(mdc, offset, height, buf, ADDRESS_LEN);
	offset += (ADDRESS_LEN * bf->char_width) + (2 * bf->char_width);

	// 16�i�\��
	SetTextColor(mdc, GetSysColor(COLOR_WINDOWTEXT));
	p = (BYTE *)bf->data + (i * LINE_LEN);
	r = cbuf;
	s = buf;
	for (j = 0; j < LINE_LEN; p++, j++) {
		if (j != 0 && j % (LINE_LEN / 2) == 0) {
			*(s++) = TEXT(' ');
		}
		if (j != 0 && p == bf->sp) {
			*s = TEXT('\0');
			TextOut(mdc, offset, height, buf, lstrlen(buf));
			offset += (lstrlen(buf) * bf->char_width);
			s = buf;
		}
		if ((DWORD)(p - (BYTE *)bf->data) < (DWORD)bf->data_len) {
			// 16�i
			itox(*p, 2, s);
			s += 2;
			// �L�����N�^
			if (bf->dbcs == TRUE) {
				bf->dbcs = FALSE;
				if (j == 0) {
					*(r++) = ' ';
				}
			} else if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
				*(r++) = *p;
				*(r++) = *(p + 1);
				bf->dbcs = TRUE;
			} else {
				*(r++) = (*p == 0x00) ? '.' : *p;
			}
		} else {
			*(s++) = TEXT(' ');
			*(s++) = TEXT(' ');
			*(r++) = ' ';
		}
		if (p == bf->sp) {
			// �I�𕶎�
			if (GetFocus() == hWnd) {
				hbr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
			} else {
				hbr = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
			}
			SetRect(&drect, offset - 1, 0, offset + (2 * bf->char_width) + 1, bf->font_height);
			if (bf->insert_mode == FALSE) {
				// �㏑�����[�h
				FillRect(mdc, &drect, hbr);
				if (GetFocus() == hWnd) {
					SetTextColor(mdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
					SetBkColor(mdc, GetSysColor(COLOR_HIGHLIGHT));
				} else {
					SetTextColor(mdc, GetSysColor(COLOR_BTNTEXT));
					SetBkColor(mdc, GetSysColor(COLOR_3DFACE));
				}
				TextOut(mdc, offset, height, buf, 2);
				SetTextColor(mdc, GetSysColor(COLOR_WINDOWTEXT));
				SetBkColor(mdc, GetSysColor(COLOR_WINDOW));
			} else {
				FrameRect(mdc, &drect, hbr);
				TextOut(mdc, offset, height, buf, 2);
			}
			DeleteObject(hbr);

			offset += (3 * bf->char_width);
			s = buf;
			sel = j;
		} else {
			*(s++) = TEXT(' ');
		}
	}
	*(r++) = ' ';
	*r = '\0';

	*s = TEXT('\0');
	TextOut(mdc, offset, height, buf, lstrlen(buf));
	offset += (lstrlen(buf) * bf->char_width) + bf->char_width;

	// �L�����N�^�\��
#ifdef UNICODE
	char_to_tchar(cbuf, buf, BUF_SIZE - 1);
	tp = buf;
	j = 0;
	for (s = tp; *s != TEXT('\0'); s++) {
		if(WideCharToMultiByte(CP_ACP, 0, s, 1, NULL, 0, NULL, NULL) != 1){
			len = 2;
		} else {
			len = 1;
		}
		if (j == sel || (len == 2 && j + 1 == sel)) {
			if (GetFocus() == hWnd) {
				hbr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
				SetTextColor(mdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
				SetBkColor(mdc, GetSysColor(COLOR_HIGHLIGHT));
			} else {
				hbr = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
				SetBkColor(mdc, GetSysColor(COLOR_3DFACE));
			}
			SetRect(&drect, offset - 1, 0, offset + (len * bf->char_width), bf->font_height);
			FillRect(mdc, &drect, hbr);
			DeleteObject(hbr);
		} else {
			SetTextColor(mdc, GetSysColor(COLOR_WINDOWTEXT));
			SetBkColor(mdc, GetSysColor(COLOR_WINDOW));
		}
		TextOut(mdc, offset, height, s, 1);
		offset += len * bf->char_width;
		j += len;
	}
#else	// UNICODE
	tp = cbuf;
	for (s = tp; *s != TEXT('\0'); s++) {
		if (IsDBCSLeadByte((BYTE)*s) == TRUE && *(s + 1) != TEXT('\0')) {
			len = 2;
		} else {
			len = 1;
		}
		if (s - tp == sel || (len == 2 && s - tp + 1 == sel)) {
			if (GetFocus() == hWnd) {
				hbr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
				SetTextColor(mdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
				SetBkColor(mdc, GetSysColor(COLOR_HIGHLIGHT));
			} else {
				hbr = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
				SetBkColor(mdc, GetSysColor(COLOR_3DFACE));
			}
			SetRect(&drect, offset - 1, 0, offset + (len * bf->char_width), bf->font_height);
			FillRect(mdc, &drect, hbr);
			DeleteObject(hbr);
		} else {
			SetTextColor(mdc, GetSysColor(COLOR_WINDOWTEXT));
			SetBkColor(mdc, GetSysColor(COLOR_WINDOW));
		}
		TextOut(mdc, offset, height, s, len);
		offset += len * bf->char_width;
		if (len == 2) {
			s++;
		}
	}
#endif	// UNICODE
}

/*
 * binview_proc - �E�B���h�E�̃v���V�[�W��
 */
static LRESULT CALLBACK binview_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	TEXTMETRIC tm;
	RECT rect;
	BUFFER *bf;
	int i;

	switch (msg) {
	case WM_CREATE:
		// �E�B���h�E�쐬
		if ((bf = mem_calloc(sizeof(BUFFER))) == NULL) {
			return -1;
		}

#ifdef OP_XP_STYLE
		// XP
		bf->hTheme = theme_open(hWnd);
#endif	// OP_XP_STYLE

		// �`��p���
		hdc = GetDC(hWnd);
		GetClientRect(hWnd, &rect);
		bf->mdc = CreateCompatibleDC(hdc);
		ReleaseDC(hWnd, hdc);
		draw_init(hWnd, bf);

		// �w�i�u���V
		bf->hbrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));

		// �t�H���g�쐬
		bf->hfont = font_create(option.bin_font_name, option.bin_font_size, option.bin_font_charset,
			option.bin_font_weight, (option.bin_font_italic == 0) ? FALSE : TRUE, TRUE);
		bf->ret_font = SelectObject(bf->mdc, bf->hfont);
		draw_free(bf);
		draw_init(hWnd, bf);

		// Metrics
		GetTextMetrics(bf->mdc, &tm);
		bf->spacing = 2;
		bf->font_height = tm.tmHeight + bf->spacing;
		bf->char_width = tm.tmAveCharWidth;
		bf->left_margin = 2;

		SetMapMode(bf->mdc, MM_TEXT);
		SetTextCharacterExtra(bf->mdc, 0);
		SetTextJustification(bf->mdc, 0, 0);
		SetTextAlign(bf->mdc, TA_TOP | TA_LEFT);
//		SetBkMode(bf->mdc, TRANSPARENT);

		// buffer info to window long
		SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)bf);
		ImmAssociateContext(hWnd, (HIMC)NULL);
		break;

	case WM_CLOSE:
		// �E�B���h�E�����
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		// �E�B���h�E�̔j��
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL) {
			SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)0);
#ifdef OP_XP_STYLE
			// XP
			theme_close(bf->hTheme);
#endif	// OP_XP_STYLE
			draw_free(bf);
			if (bf->hfont != NULL) {
				SelectObject(bf->mdc, bf->ret_font);
				DeleteObject(bf->hfont);
			}
			DeleteDC(bf->mdc);
			DeleteObject(bf->hbrush);
			mem_free(&bf->data);
			mem_free(&bf->undo);
			mem_free(&bf);
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);

	case WM_SIZE:
		// �T�C�Y�ύX
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}

		// �X�N���[���o�[�̍X�V
		SendMessage(hWnd, WM_SET_SCROLLBAR, 0, 0);

		// �`����̍X�V
		draw_free(bf);
		draw_init(hWnd, bf);
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_EXITSIZEMOVE:
		// �T�C�Y�ύX����
		break;

	case WM_SETFOCUS:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		// �I�𕶎��̂���s���ĕ`��
		binview_refresh_line(hWnd, bf, bf->sp, bf->sp);
		break;

	case WM_KILLFOCUS:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		// �I�𕶎��̂���s���ĕ`��
		binview_refresh_line(hWnd, bf, bf->sp, bf->sp);
		break;

	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;

	case WM_HSCROLL:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		GetClientRect(hWnd, &rect);
		i = bf->pos_x;
		switch ((int)LOWORD(wParam)) {
		case SB_TOP:
			bf->pos_x = 0;
			break;

		case SB_BOTTOM:
			bf->pos_x = bf->max_x;
			break;

		case SB_LINELEFT:
			bf->pos_x = (bf->pos_x > 0) ? bf->pos_x - 1 : 0;
			break;

		case SB_LINERIGHT:
			bf->pos_x = (bf->pos_x < bf->max_x) ? bf->pos_x + 1 : bf->max_x;
			break;

		case SB_PAGELEFT:
			bf->pos_x = (bf->pos_x - (rect.right / bf->char_width) > 0) ?
				bf->pos_x - (rect.right / bf->char_width) : 0;
			break;

		case SB_PAGERIGHT:
			bf->pos_x = (bf->pos_x + (rect.right / bf->char_width) < bf->max_x) ?
				bf->pos_x + (rect.right / bf->char_width) : bf->max_x;
			break;

		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			{
				SCROLLINFO si;

				ZeroMemory(&si, sizeof(SCROLLINFO));
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_ALL;
				GetScrollInfo(hWnd, SB_HORZ, &si);
				bf->pos_x = si.nTrackPos;
			}
			break;
		}
		SetScrollPos(hWnd, SB_HORZ, bf->pos_x, TRUE);
		ScrollWindowEx(hWnd, (i - bf->pos_x) * bf->char_width, 0, NULL, &rect, NULL, NULL, SW_INVALIDATE | SW_ERASE);
		break;

	case WM_VSCROLL:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		GetClientRect(hWnd, &rect);
		i = bf->pos_y;
		switch ((int)LOWORD(wParam)) {
		case SB_TOP:
			bf->pos_y = 0;
			break;

		case SB_BOTTOM:
			bf->pos_y = bf->max_y;
			break;

		case SB_LINEUP:
			bf->pos_y = (bf->pos_y > 0) ? bf->pos_y - 1 : 0;
			break;

		case SB_LINEDOWN:
			bf->pos_y = (bf->pos_y < bf->max_y) ? bf->pos_y + 1 : bf->max_y;
			break;

		case SB_PAGEUP:
			bf->pos_y = (bf->pos_y - (rect.bottom / bf->font_height) > 0) ?
				bf->pos_y - (rect.bottom / bf->font_height) : 0;
			break;

		case SB_PAGEDOWN:
			bf->pos_y = (bf->pos_y + (rect.bottom / bf->font_height) < bf->max_y) ?
				bf->pos_y + (rect.bottom / bf->font_height) : bf->max_y;
			break;

		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			{
				SCROLLINFO si;

				ZeroMemory(&si, sizeof(SCROLLINFO));
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_ALL;
				GetScrollInfo(hWnd, SB_VERT, &si);
				bf->pos_y = si.nTrackPos;
			}
			break;
		}
		SetScrollPos(hWnd, SB_VERT, bf->pos_y, TRUE);
		ScrollWindowEx(hWnd, 0, (i - bf->pos_y) * bf->font_height, NULL, &rect, NULL, NULL, SW_INVALIDATE | SW_ERASE);
		break;

	case WM_MOUSEWHEEL:
		for (i = 0; i < 3; i++) {
			SendMessage(hWnd, WM_VSCROLL, ((short)HIWORD(wParam) > 0) ? SB_LINEUP : SB_LINEDOWN, 0);
		}
		break;

	case WM_LBUTTONDOWN:
		SetFocus(hWnd);
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		binview_flush(bf);
		binview_refresh_line(hWnd, bf, bf->sp, bf->sp);
		// ���W���當���ʒu���擾
		bf->sp = binview_point_to_select(hWnd, bf, (short)LOWORD(lParam), (short)HIWORD(lParam));
		binview_refresh_line(hWnd, bf, bf->sp, bf->sp);

		binview_ensure_visible(hWnd, bf);
		break;

	case WM_LBUTTONUP:
		break;

	case WM_RBUTTONDOWN:
		SetFocus(hWnd);
		break;

	case WM_RBUTTONUP:
		SendMessage(hWnd, WM_SHOW_MENU, 0, 0);
		break;

	case WM_KEYDOWN:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		GetClientRect(hWnd, &rect);
		switch ((int)wParam) {
		case VK_APPS:
			SendMessage(hWnd, WM_SHOW_MENU, 0, 0);
			break;

		case VK_INSERT:
			bf->insert_mode = !bf->insert_mode;
			binview_refresh_line(hWnd, bf, bf->sp, bf->sp);
			break;

		case VK_DELETE:
			if (bf->data == NULL || bf->sp == bf->data + bf->data_len ||
				bf->lock == TRUE || option.bin_lock != 0) {
				break;
			}
			// �폜
			binview_flush(bf);
			binview_set_undo(bf, UNDO_TYPE_DELETE);
			binview_delete(hWnd, bf);
			break;

		case VK_BACK:
			if (bf->data == NULL || bf->sp == bf->data ||
				bf->lock == TRUE || option.bin_lock != 0) {
				break;
			}
			binview_flush(bf);
			bf->sp--;
			binview_set_undo(bf, UNDO_TYPE_DELETE);
			binview_delete(hWnd, bf);
			break;

		case VK_TAB:
			SetFocus(GetParent(hWnd));
			break;

		case 'Z':
			if (GetKeyState(VK_CONTROL) < 0 && GetKeyState(VK_SHIFT) < 0) {
				// ��蒼��
				SendMessage(hWnd, EM_REDO, 0, 0);
			} else if (GetKeyState(VK_CONTROL) < 0) {
				// ���ɖ߂�
				SendMessage(hWnd, EM_UNDO, 0, 0);
			}
			break;

		case 'Y':
			if (GetKeyState(VK_CONTROL) < 0) {
				SendMessage(hWnd, EM_REDO, 0, 0);
			}
			break;

		case VK_LEFT:
			binview_flush(bf);
			if (bf->data != NULL && (bf->sp - 1) >= bf->data) {
				binview_refresh_line(hWnd, bf, bf->sp, bf->sp);
				bf->sp--;
				binview_refresh_line(hWnd, bf, bf->sp, bf->sp);
				binview_ensure_visible(hWnd, bf);
			}
			break;

		case VK_RIGHT:
			binview_flush(bf);
			if (bf->data != NULL && (DWORD)(bf->sp - (BYTE *)bf->data + 1) <= (DWORD)bf->data_len) {
				binview_refresh_line(hWnd, bf, bf->sp, bf->sp);
				bf->sp++;
				binview_refresh_line(hWnd, bf, bf->sp, bf->sp);
				binview_ensure_visible(hWnd, bf);
			}
			break;

		case VK_UP:
			binview_flush(bf);
			if (bf->data != NULL && (bf->sp - LINE_LEN) >= bf->data) {
				binview_refresh_line(hWnd, bf, bf->sp, bf->sp);
				bf->sp -= LINE_LEN;
				binview_refresh_line(hWnd, bf, bf->sp, bf->sp);
				binview_ensure_visible(hWnd, bf);
			}
			break;

		case VK_DOWN:
			binview_flush(bf);
			if (bf->data != NULL && (DWORD)(bf->sp - (BYTE *)bf->data + LINE_LEN) <= (DWORD)bf->data_len) {
				binview_refresh_line(hWnd, bf, bf->sp, bf->sp);
				bf->sp += LINE_LEN;
				binview_refresh_line(hWnd, bf, bf->sp, bf->sp);
				binview_ensure_visible(hWnd, bf);
			}
			break;

		case VK_HOME:
			binview_flush(bf);
			if (bf->data != NULL && bf->sp != bf->data) {
				bf->sp = bf->data;
				binview_ensure_visible(hWnd, bf);
				InvalidateRect(hWnd, NULL, FALSE);
			}
			break;

		case VK_END:
			binview_flush(bf);
			if (bf->data != NULL && bf->sp != bf->data + bf->data_len) {
				bf->sp = bf->data + bf->data_len;
				binview_ensure_visible(hWnd, bf);
				InvalidateRect(hWnd, NULL, FALSE);
			}
			break;

		case VK_PRIOR:
			if (bf->data == NULL) {
				break;
			}
			binview_flush(bf);
			// �I���ʒu��\��
			binview_ensure_visible(hWnd, bf);
			binview_refresh_line(hWnd, bf, bf->sp, bf->sp);
			// ���ʒu�̎擾
			i = (bf->sp - bf->data) % LINE_LEN;
			// �P�y�[�W�O�Ɉړ�
			bf->sp -= rect.bottom / bf->font_height * LINE_LEN;
			if (bf->sp < bf->data) {
				bf->sp = bf->data + i;
			}
			SendMessage(hWnd, WM_VSCROLL, SB_PAGEUP, 0);
			binview_refresh_line(hWnd, bf, bf->sp, bf->sp);
			break;

		case VK_NEXT:
			if (bf->data == NULL) {
				break;
			}
			binview_flush(bf);
			// �I���ʒu��\��
			binview_ensure_visible(hWnd, bf);
			binview_refresh_line(hWnd, bf, bf->sp, bf->sp);
			// ���ʒu�̎擾
			i = (LINE_LEN - (bf->sp - bf->data) % LINE_LEN);
			if ((bf->data_len + 1) % LINE_LEN != 0) {
				i -= (LINE_LEN - (bf->data_len + 1) % LINE_LEN);
			}
			// 1�y�[�W���Ɉړ�
			bf->sp += rect.bottom / bf->font_height * LINE_LEN;
			if (bf->sp > bf->data + bf->data_len) {
				bf->sp = bf->data + bf->data_len - i + 1;
				if (bf->sp > bf->data + bf->data_len) {
					bf->sp = bf->data + bf->data_len;
				}
			}
			SendMessage(hWnd, WM_VSCROLL, SB_PAGEDOWN, 0);
			binview_refresh_line(hWnd, bf, bf->sp, bf->sp);
			break;
		}
		break;

	case WM_CHAR:
		// ����
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL ||
			bf->lock == TRUE || option.bin_lock != 0) {
			break;
		}
		binview_input(hWnd, bf, (TCHAR)wParam);
		break;

	case WM_PAINT:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL) {
			PAINTSTRUCT ps;

			hdc = BeginPaint(hWnd, &ps);
			GetClientRect(hWnd, &rect);
			SetRect(&rect, ps.rcPaint.left, 0, ps.rcPaint.right, bf->font_height);

			i = bf->pos_y + (ps.rcPaint.top / bf->font_height) - 1;
			bf->dbcs = FALSE;
			for (; i < bf->pos_y + (ps.rcPaint.bottom / bf->font_height) + 1; i++) {
				// �w�i�h��Ԃ�
				FillRect(bf->mdc, &rect, bf->hbrush);
				// draw line
				binview_draw_line(hWnd, bf->mdc, bf, i);
				BitBlt(hdc,
					ps.rcPaint.left, (i - bf->pos_y) * bf->font_height,
					ps.rcPaint.right, bf->font_height,
					bf->mdc, ps.rcPaint.left, 0, SRCCOPY);
			}
			EndPaint(hWnd, &ps);
		}
		break;

	case WM_ERASEBKGND:
		return 1;

#ifdef OP_XP_STYLE
	case WM_NCPAINT:
		// XP�p�̔w�i�`��
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL ||
			theme_draw(hWnd, (HRGN)wParam, bf->hTheme) == FALSE) {
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
		break;

	case WM_THEMECHANGED:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		// XP�e�[�}�̕ύX
		theme_close(bf->hTheme);
		bf->hTheme = theme_open(hWnd);
		break;
#endif	// OP_XP_STYLE

	case WM_UNDO:
	case EM_UNDO:
		// ���ɖ߂�
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL ||
			bf->lock == TRUE || option.bin_lock != 0) {
			break;
		}
		binview_undo_exec(hWnd, bf);
		break;

	case EM_REDO:
		// ��蒼��
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL ||
			bf->lock == TRUE || option.bin_lock != 0) {
			break;
		}
		binview_redo_exec(hWnd, bf);
		break;

	case EM_CANUNDO:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL ||
			bf->lock == TRUE || option.bin_lock != 0) {
			return FALSE;
		}
		return ((bf->undo_len > 0) ? TRUE : FALSE);

	case EM_CANREDO:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL ||
			bf->lock == TRUE || option.bin_lock != 0) {
			return FALSE;
		}
		return ((bf->undo != NULL && (bf->undo + bf->undo_len)->type != 0) ? TRUE : FALSE);

	case WM_SET_BINDATA:
		// �f�[�^�ݒ�
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		// ������
		bf->pos_x = 0;
		bf->pos_y = 0;
		bf->lock = wParam;
		bf->input_cnt = 0;
		bf->modified = FALSE;
		if (bf->undo != NULL) {
			mem_free(&bf->undo);
			bf->undo_size = 0;
			bf->undo_len = 0;
		}
		if (bf->data != NULL) {
			// ���
			mem_free(&bf->data);
			bf->data_size = 0;
			bf->data_len = 0;
		}
		if ((DATA_INFO *)lParam != NULL && ((DATA_INFO *)lParam)->data != NULL) {
			// �f�[�^���o�C�g��ɕϊ�
			if ((bf->data = format_data_to_bytes((DATA_INFO *)lParam, &bf->data_len)) == NULL) {
				bf->data = clipboard_data_to_bytes((DATA_INFO *)lParam, &bf->data_len);
			}
			bf->data_size = bf->data_len;
		}
		bf->sp = bf->data;
		bf->height = bf->data_len / LINE_LEN + 1;
		SendMessage(hWnd, WM_SET_SCROLLBAR, 0, 0);
		break;

	case WM_SAVE_BINDATA:
		// �f�[�^�ۑ�
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || bf->modified == FALSE) {
			return FALSE;
		}
		if ((DATA_INFO *)lParam != NULL) {
			// ���
			if (((DATA_INFO *)lParam)->data != NULL && format_free_data(((DATA_INFO *)lParam)->format_name, ((DATA_INFO *)lParam)->data) == FALSE) {
				clipboard_free_data(((DATA_INFO *)lParam)->format_name, ((DATA_INFO *)lParam)->data);
			}
			// �o�C�g����f�[�^�ɕϊ�
			if ((((DATA_INFO *)lParam)->data = format_bytes_to_data(((DATA_INFO *)lParam)->format_name, bf->data, &bf->data_len)) == NULL) {
				((DATA_INFO *)lParam)->data = clipboard_bytes_to_data(((DATA_INFO *)lParam)->format_name, bf->data, &bf->data_len);
			}
			if (((DATA_INFO *)lParam)->data != NULL) {
				((DATA_INFO *)lParam)->size = bf->data_len;
			}
		}
		return TRUE;

	case WM_SET_SCROLLBAR:
		// �X�N���[���o�[�ݒ�
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		binview_set_scrollbar(hWnd, bf);
		break;

	case WM_SHOW_MENU:
		// ���j���[�\��
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL) {
			HMENU hMenu;
			POINT apos;

			// ���j���[�̍쐬
			hMenu = CreatePopupMenu();
			AppendMenu(hMenu, MF_STRING | (SendMessage(hWnd, EM_CANUNDO, 0, 0) == TRUE) ? 0 : MF_GRAYED,
				EM_UNDO, message_get_res(IDS_BIN_MENU_UNDO));
			AppendMenu(hMenu, MF_STRING | (SendMessage(hWnd, EM_CANREDO, 0, 0) == TRUE) ? 0 : MF_GRAYED,
				EM_REDO, message_get_res(IDS_BIN_MENU_REDO));
			AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
			AppendMenu(hMenu, MF_STRING |
				((option.bin_lock == 1) ? MF_CHECKED : 0) |
				((bf->lock == TRUE) ? MF_GRAYED : 0),
				1, message_get_res(IDS_BIN_MENU_LOCK));
			AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
			AppendMenu(hMenu, MF_STRING, 2, message_get_res(IDS_BIN_MENU_FONT));
			// ���j���[�̕\��
			GetCursorPos((LPPOINT)&apos);
			i = TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_RETURNCMD, apos.x, apos.y, 0, hWnd, NULL);
			DestroyMenu(hMenu);
			if (i <= 0) {
				break;
			}
			switch (i) {
			case 1:
				// ���b�N
				option.bin_lock = !option.bin_lock;
				InvalidateRect(hWnd, NULL, FALSE);
				break;
			case 2:
				// �t�H���g
				if (binview_select_font(hWnd) == TRUE) {
					if (bf->hfont != NULL) {
						SelectObject(bf->mdc, bf->ret_font);
						DeleteObject(bf->hfont);
					}
					// �t�H���g�쐬
					bf->hfont = font_create(option.bin_font_name, option.bin_font_size, option.bin_font_charset,
						option.bin_font_weight, (option.bin_font_italic == 0) ? FALSE : TRUE, TRUE);
					bf->ret_font = SelectObject(bf->mdc, bf->hfont);
					// Metrics
					GetTextMetrics(bf->mdc, &tm);
					bf->font_height = tm.tmHeight + bf->spacing;
					bf->char_width = tm.tmAveCharWidth;
					SendMessage(hWnd, WM_SET_SCROLLBAR, 0, 0);
					draw_free(bf);
					draw_init(hWnd, bf);
					InvalidateRect(hWnd, NULL, FALSE);
				}
				break;
			default:
				SendMessage(hWnd, i, 0, 0);
				break;
			}
		}
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

/*
 * binview_regist - �E�B���h�E�N���X�̓o�^
 */
BOOL binview_regist(const HINSTANCE hInstance)
{
	WNDCLASS wc;

	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = (WNDPROC)binview_proc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(0, IDC_IBEAM);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WINDOW_CLASS;
	// �E�B���h�E�N���X�̓o�^
	return RegisterClass(&wc);
}

/*
 * binview_create - �o�C�i���r���[�A�̍쐬
 */
HWND binview_create(const HINSTANCE hInstance, const HWND pWnd, int id)
{
	HWND hWnd;

	// �E�B���h�E�̍쐬
	hWnd = CreateWindowEx(WS_EX_CLIENTEDGE,
		WINDOW_CLASS,
		TEXT(""),
		WS_TABSTOP | WS_CHILD | WS_HSCROLL | WS_VSCROLL,
		0, 0, 0, 0, pWnd, (HMENU)id, hInstance, NULL);
	return hWnd;
}
/* End of source */
