/*
 * CLCL
 *
 * Ini.h
 *
 * Copyright (C) 1996-2019 by Ohno Tomoaki. All rights reserved.
 *		https://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_INI_H
#define _INC_INI_H

/* Include Files */

#include "Menu.h"
#include "Format.h"
#include "Filter.h"
#include "Window.h"
#include "SendKey.h"
#include "Tool.h"

/* Define */
#define ACTION_POPUPMEMU				0
#define ACTION_VIEWER					1
#define ACTION_OPTION					2
#define ACTION_CLIPBOARD_WATCH			3
#define ACTION_EXIT						4

#define ACTION_TYPE_HOTKEY				0
#define ACTION_TYPE_CTRL_CTRL			1
#define ACTION_TYPE_SHIFT_SHIFT			2
#define ACTION_TYPE_ALT_ALT				3
#define ACTION_TYPE_TRAY_LEFT			4
#define ACTION_TYPE_TRAY_LEFT_DBLCLK	5
#define ACTION_TYPE_TRAY_RIGHT			6
#define ACTION_TYPE_TRAY_RIGHT_DBLCLK	7

#define OPTION_SHOW_HISTORY				TEXT("0")
#define OPTION_SHOW_MENU				TEXT("1")
#define OPTION_SHOW_VIEWER				TEXT("2")
#define OPTION_SHOW_ACTION				TEXT("3")
#define OPTION_SHOW_FORMAT				TEXT("4")
#define OPTION_SHOW_FILTER				TEXT("5")
#define OPTION_SHOW_WINDOW				TEXT("6")
#define OPTION_SHOW_SENDKEY				TEXT("7")
#define OPTION_SHOW_TOOL				TEXT("8")

/* Struct */
// color info
typedef struct _COLOR_INFO {
	TCHAR *color_str;
	int color;
} COLOR_INFO;

// action info
typedef struct _ACTION_INFO {
	int action;							// ACTION_
	int type;							// ACTION_TYPE_

	int enable;							// 0-���� 1-�L��
	int caret;							// Caret�ʒu�ɕ\��

	// hot key
	int id;
	UINT modifiers;
	UINT virtkey;
	int paste;

	struct _MENU_INFO *menu_info;
	int menu_cnt;
} ACTION_INFO;

// option
typedef struct _OPTION_INFO {
	// main
	int main_clipboard_watch;			// �N���b�v�{�[�h�Ď�
	int main_clipboard_rechain_minute;	// �N���b�v�{�[�h�ĊĎ�����
	int main_show_trayicon;				// �^�X�N�g���C�ɃA�C�R����\��
	int main_show_viewer;				// �N�����Ƀr���[�A��\��

	// data
	TCHAR *data_date_format;
	TCHAR *data_time_format;

	// history
	int history_add_interval;			// �����ɒǉ�����܂ł̃C���^�[�o��
	int history_save;					// �I�����ɗ�����ۑ�
	int history_always_save;			// �ύX������Ώ�ɕۑ�
	int history_delete;					// �N���b�v�{�[�h�ɑ���Ɨ�������폜
	int history_max;					// ��������
	int history_overlap_check;			// �����̏d���`�F�b�N 0-�`�F�b�N���Ȃ� 1-�ŋ߂̂P���̂݃`�F�b�N 2-�S�Ă̗������`�F�b�N
	int history_ignore_regist_item;		// �o�^�A�C�e�����N���b�v�{�[�h�ɑ��������ɗ����ɓ���Ȃ�

	// menu
	TCHAR *menu_text_format;			// ���j���[�^�C�g���̕\���`��
	int menu_intact_item_title;			// �A�C�e���^�C�g��������ꍇ�͂��̂܂ܕ\������
	int menu_text_margin_left;			// �e�L�X�g�̍��}�[�W��
	int menu_text_margin_right;			// �e�L�X�g�̉E�}�[�W��
	int menu_text_margin_y;				// �e�L�X�g�̏㉺�}�[�W��
	int menu_separator_height;			// ��؂�̍���
	int menu_separator_margin_left;		// ��؂�̍��}�[�W��
	int menu_separator_margin_right;	// ��؂�̉E�}�[�W��
	int menu_max_width;					// ���j���[�̍ő啝
	int menu_break;						// ��ʂ���͂ݏo���ꍇ�̐܂�Ԃ�
	int menu_show_icon;					// �A�C�R���\��
	int menu_icon_size;					// �A�C�R���̃T�C�Y
	int menu_icon_margin;				// �A�C�R���̃}�[�W��
	int menu_show_bitmap;				// �r�b�g�}�b�v�\��
	int menu_bitmap_width;				// �r�b�g�}�b�v�̉���
	int menu_bitmap_height;				// �r�b�g�}�b�v�̏c��
	int menu_show_tooltip;				// �c�[���`�b�v�\��
	int menu_show_hotkey;				// �z�b�g�L�[��\��
	int menu_show_tool_menu;			// �c�[�����j���[�\��
#ifdef MENU_LAYERER
	int menu_alpha;						// �����x
#endif
	TCHAR *menu_font_name;				// ���j���[�̃t�H���g
	int menu_font_size;
	int menu_font_weight;
	int menu_font_italic;
	int menu_font_charset;
#ifdef MENU_COLOR
	COLOR_INFO menu_color_back;			// ���j���[�̐F
	COLOR_INFO menu_color_text;
	COLOR_INFO menu_color_highlight;
	COLOR_INFO menu_color_highlighttext;
	COLOR_INFO menu_color_3d_shadow;
	COLOR_INFO menu_color_3d_highlight;
#endif

	// tooltip
	int tooltip_show_delay;				// �c�[���`�b�v��\������܂ł̑҂�����(�~���b)
	int tooltip_tab_length;				// �c�[���`�b�v�̃^�u��
	int tooltip_margin_x;				// �c�[���`�b�v���ɕ\�����镶���̉��}�[�W��
	int tooltip_margin_y;				// �c�[���`�b�v���ɕ\�����镶���̏c�}�[�W��
	TCHAR *tooltip_font_name;			// �c�[���`�b�v�̃t�H���g
	int tooltip_font_size;
	int tooltip_font_weight;
	int tooltip_font_italic;
	int tooltip_font_charset;
#ifdef TOOLTIP_COLOR
	COLOR_INFO tooltip_color_back;		// �c�[���`�b�v�̐F
	COLOR_INFO tooltip_color_text;
#endif

	// action
	int action_double_press_time;		// CTRL or Shift or Alt��1��ڂ�2��ڂ̃L�[�������ԂɌo�߂��鎞��(�~���b)
	int action_show_hotkey_error;		// �z�b�g�L�[�̃G���[��\��
	ACTION_INFO *action_info;
	int action_cnt;

	// format
	FORMAT_INFO *format_info;
	int format_cnt;

	// filter
	int filter_all_action;
	FILTER_INFO *filter_info;
	int filter_cnt;

	// window filter
	WINDOW_FILTER_INFO *window_filter_info;
	int window_filter_cnt;

	// send key
	UINT def_copy_modifiers;
	UINT def_copy_virtkey;
	int def_copy_wait;					// �R�s�[�܂ł̑҂�����

	UINT def_paste_modifiers;
	UINT def_paste_virtkey;
	int def_paste_wait;					// �\��t���܂ł̑҂�����

	SENDKEY_INFO *sendkey_info;
	int sendkey_cnt;

	// tool
	int tool_valid_interval;			// ���상�j���[����̑I���Ńc�[�����L���Ȏ���
	TOOL_INFO *tool_info;
	int tool_cnt;

	// viewer
	int viewer_toggle;					// �\�����g�O������
	int viewer_show_bin;				// ��Ƀo�C�i���\��
	int viewer_show_toolbar;
	int viewer_show_statusbar;
	int viewer_delete_confirm;			// �폜�m�F 0-���Ȃ� 1-����
	RECT viewer_rect;
	int viewer_sep_size;

	// treeview
	int tree_show_format;				// �c���[�Ɍ`����\��
	TCHAR *tree_root_order;				// ���[�g�̕\���ƕ��я� 0-clip 1-history 2-regist
	int tree_clip_expand;				// �N���b�v�{�[�h��W�J
	int tree_history_expand;			// ������W�J
	int tree_regist_expand;				// �o�^�A�C�e����W�J
	int tree_folder_expand;				// �t�H���_��W�J
	TCHAR *tree_font_name;
	int tree_font_size;
	int tree_font_weight;
	int tree_font_italic;
	int tree_font_charset;

	// listview
	int list_default_action;			// �f�t�H���g���� 0-�\�� 1-�N���b�v�{�[�h�ɑ��� 2-���O��t���ĕۑ�
	int list_column_data;
	int list_column_size;
	int list_column_date;
	int list_column_window;
	TCHAR *list_font_name;
	int list_font_size;
	int list_font_weight;
	int list_font_italic;
	int list_font_charset;

	// bin view
	int bin_lock;
	TCHAR *bin_font_name;
	int bin_font_size;
	int bin_font_weight;
	int bin_font_italic;
	int bin_font_charset;

	// text format
	int fmt_txt_menu_tooltip_size;		// �c�[���`�b�v�ɕ\�����镶����
	int fmt_txt_viewer_word_wrap;		// �E�[�Ő܂�Ԃ�
	int fmt_txt_tab_size;				// TAB�T�C�Y
	TCHAR *fmt_txt_font_name;
	int fmt_txt_font_size;
	int fmt_txt_font_weight;
	int fmt_txt_font_italic;
	int fmt_txt_font_charset;

	// bitmap format
	int fmt_bmp_stretch_mode;			// �摜���E�B���h�E�̑傫���ɍ��킹�ĕ\��

	// file format
	int fmt_file_column_name;
	int fmt_file_column_folder;
	int fmt_file_column_type;
	TCHAR *fmt_file_font_name;
	int fmt_file_font_size;
	int fmt_file_font_weight;
	int fmt_file_font_italic;
	int fmt_file_font_charset;
} OPTION_INFO;

/* Function Prototypes */
BOOL ini_get_option(TCHAR *err_str);
BOOL ini_put_option(void);
void ini_free_format_name(FORMAT_NAME *fn, const int fn_cnt);
void ini_free_menu(MENU_INFO *mi, const int mcnt);
BOOL ini_free(void);

#endif
/* End of source */
