/*
 * CLCL
 *
 * Frame.c
 *
 * Copyright (C) 1996-2019 by Ohno Tomoaki. All rights reserved.
 *		https://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef	_INC_OLE

#include "Memory.h"
#include "Frame.h"
#include "dpi.h"

/* Define */
#define NOMOVESIZE		Scale(6)		// �t���[���̈ړ������l

/* Global Variables */
static RECT *frame_rect;		// �t���[���̈ʒu���

/* ocal Function Prototypes */

/*
 * frame_initialize - �t���[���`��p�\���̂̏�����
 */
BOOL frame_initialize(const HWND hWnd)
{
	if (frame_rect == NULL) {
		frame_rect = (RECT *)mem_calloc(sizeof(RECT) * FRAME_CNT);
		if (frame_rect == NULL) {
			return FALSE;
		}
	}
	SetCapture(hWnd);
	return TRUE;
}

/*
 * frame_free - �t���[���`��p�\���̂̉��
 */
void frame_free(void)
{
	if (frame_rect != NULL) {
		mem_free(&frame_rect);
	}
	ReleaseCapture();
}

/*
 * frame_draw - �t���[���̕`��
 */
int frame_draw(const HWND hWnd, const HWND hTreeView)
{
	RECT window_rect, treeview_rect;
	POINT apos;
	HDC hdc;
	int draw_cnt;

	GetCursorPos((LPPOINT)&apos);
	GetWindowRect(hWnd, (LPRECT)&window_rect);
	GetWindowRect(hTreeView, (LPRECT)&treeview_rect);

	// �t���[���̈ړ�����
	if (apos.x <= (window_rect.left + NOMOVESIZE + GetSystemMetrics(SM_CXFRAME))) {
		apos.x = window_rect.left + NOMOVESIZE + GetSystemMetrics(SM_CXFRAME);

	} else if (apos.x >= (window_rect.right - (NOMOVESIZE + (FRAME_CNT * 2)) - GetSystemMetrics(SM_CXFRAME))) {
		apos.x = window_rect.right - (NOMOVESIZE + (FRAME_CNT * 2)) - GetSystemMetrics(SM_CXFRAME);
	}

	// �O��̈ʒu�Ɣ�r
	if (apos.x == frame_rect[0].left) {
		return 1;
	}

	hdc = GetWindowDC(hWnd);

	// �O��`�敪������
	for (draw_cnt = 0;draw_cnt < FRAME_CNT;draw_cnt++) {
		DrawFocusRect(hdc, (LPRECT)&frame_rect[draw_cnt]);
	}

	// �t���[���̕`��
	for (draw_cnt = 0;draw_cnt < FRAME_CNT;draw_cnt++) {
		(frame_rect + draw_cnt)->left = apos.x + draw_cnt - window_rect.left;
		(frame_rect + draw_cnt)->right = (frame_rect + draw_cnt)->left + FRAME_CNT + 1;
		(frame_rect + draw_cnt)->top = treeview_rect.top - window_rect.top;
		(frame_rect + draw_cnt)->bottom = treeview_rect.bottom - window_rect.top;

		DrawFocusRect(hdc, (LPRECT)(frame_rect + draw_cnt));
	}
	ReleaseDC(hWnd, hdc);
	return 0;
}

/*
 * frame_draw_end - �t���[���̕`��I���A�t���[���̍ŏI�ʒu��Ԃ�
 */
int frame_draw_end(const HWND hWnd)
{
	HDC hdc;
	int draw_cnt;
	int ret;

	if (frame_rect[0].left == 0 && frame_rect[0].right == 0 &&
		frame_rect[0].top == 0 && frame_rect[0].bottom == 0) {
		frame_free();
		return -1;
	}

	// �O��`�敪������
	hdc = GetWindowDC(hWnd);
	for (draw_cnt = 0;draw_cnt < FRAME_CNT;draw_cnt++) {
		DrawFocusRect(hdc, (LPRECT)&frame_rect[draw_cnt]);
	}
	ReleaseDC(hWnd, hdc);

	// ���E�ʒu�̎擾
	ret = frame_rect[0].left - GetSystemMetrics(SM_CXFRAME);

	frame_free();
	return ret;
}
/* End of source */
