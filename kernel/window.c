/**
	@File:			window.c
	@Author:		Ihsoh
	@Date:			2014-11-20
	@Description:
		提供窗体操作相关的功能
*/

#include "window.h"
#include "memory.h"
#include "image.h"
#include "enfont.h"
#include "mouse.h"

#include "bmp/bmp.h"

#include "utils/img.h"

#include "fs/ifs1/fs.h"

static struct CommonImage close_button;
static struct CommonImage close_button_hover;
static struct CommonImage close_button_blur;
static struct CommonImage hidden_button;
static struct CommonImage hidden_button_hover;
static struct CommonImage hidden_button_blur;

/**
	@Function:		load_image
	@Access:		Private
	@Description:
		加载图片。
	@Parameters:
		common_image, struct CommonImage *, OUT
			图片。
		path, int8 *, IN
			图片路径。
	@Return:
		BOOL
			返回TRUE则成功，否则失败。		
*/
static
BOOL
load_image(	OUT struct CommonImage * common_image,
			IN int8 * path)
{
	FileObject * fptr = open_file(path, FILE_MODE_READ);
	if(fptr == NULL)
		return FALSE;
	uchar * image_data = alloc_memory(flen(fptr));
	if(image_data == NULL)
		return FALSE;
	read_file(fptr, image_data, flen(fptr));
	close_file(fptr);
	if(!new_common_image(common_image, image_data))
	{
		free_memory(image_data);
		return FALSE;
	}
	free_memory(image_data);
	return TRUE;
}

/**
	@Function:		init_window_resources
	@Access:		Public
	@Description:
		初始化窗体所需的资源。
	@Parameters:
	@Return:
		BOOL
			返回TRUE则成功，否则失败。		
*/
BOOL
init_window_resources(void)
{
	new_empty_image0(&close_button, CLOSE_BUTTON_WIDTH, CLOSE_BUTTON_HEIGHT);
	new_empty_image0(&close_button_hover, CLOSE_BUTTON_WIDTH, CLOSE_BUTTON_HEIGHT);
	new_empty_image0(&close_button_blur, CLOSE_BUTTON_WIDTH, CLOSE_BUTTON_HEIGHT);
	new_empty_image0(&hidden_button, HIDDEN_BUTTON_WIDTH, HIDDEN_BUTTON_HEIGHT);
	new_empty_image0(&hidden_button_hover, HIDDEN_BUTTON_WIDTH, HIDDEN_BUTTON_HEIGHT);
	new_empty_image0(&hidden_button_blur, HIDDEN_BUTTON_WIDTH, HIDDEN_BUTTON_HEIGHT);

	ASCCHAR imgfile[1024];
	IMGLBMPPtr bmpobj = NULL;
	config_gui_get_string("CloseButton", imgfile, sizeof(imgfile));
	bmpobj = imgl_bmp_create(imgfile);
	if(bmpobj != NULL)
	{
		img_draw_bmp_to_cimage(bmpobj, CLOSE_BUTTON_WIDTH, CLOSE_BUTTON_HEIGHT, &close_button);
		imgl_bmp_destroy(bmpobj);
	}

	config_gui_get_string("CloseButtonHover", imgfile, sizeof(imgfile));
	bmpobj = imgl_bmp_create(imgfile);
	if(bmpobj != NULL)
	{
		img_draw_bmp_to_cimage(bmpobj, CLOSE_BUTTON_WIDTH, CLOSE_BUTTON_HEIGHT, &close_button_hover);
		imgl_bmp_destroy(bmpobj);
	}

	config_gui_get_string("CloseButtonBlur", imgfile, sizeof(imgfile));
	bmpobj = imgl_bmp_create(imgfile);
	if(bmpobj != NULL)
	{
		img_draw_bmp_to_cimage(bmpobj, CLOSE_BUTTON_WIDTH, CLOSE_BUTTON_HEIGHT, &close_button_blur);
		imgl_bmp_destroy(bmpobj);
	}

	config_gui_get_string("HiddenButton", imgfile, sizeof(imgfile));
	bmpobj = imgl_bmp_create(imgfile);
	if(bmpobj != NULL)
	{
		img_draw_bmp_to_cimage(bmpobj, HIDDEN_BUTTON_WIDTH, HIDDEN_BUTTON_HEIGHT, &hidden_button);
		imgl_bmp_destroy(bmpobj);
	}

	config_gui_get_string("HiddenButtonHover", imgfile, sizeof(imgfile));
	bmpobj = imgl_bmp_create(imgfile);
	if(bmpobj != NULL)
	{
		img_draw_bmp_to_cimage(bmpobj, HIDDEN_BUTTON_WIDTH, HIDDEN_BUTTON_HEIGHT, &hidden_button_hover);
		imgl_bmp_destroy(bmpobj);
	}

	config_gui_get_string("HiddenButtonBlur", imgfile, sizeof(imgfile));
	bmpobj = imgl_bmp_create(imgfile);
	if(bmpobj != NULL)
	{
		img_draw_bmp_to_cimage(bmpobj, HIDDEN_BUTTON_WIDTH, HIDDEN_BUTTON_HEIGHT, &hidden_button_blur);
		imgl_bmp_destroy(bmpobj);
	}
}

/**
	@Function:		destroy_window_resources
	@Access:		Public
	@Description:
		销毁窗体所需的资源。
	@Parameters:
	@Return:
*/
void
destroy_window_resources(void)
{
	destroy_common_image(&close_button);
	destroy_common_image(&close_button_hover);
	destroy_common_image(&hidden_button);
	destroy_common_image(&hidden_button_hover);
}

/**
	@Function:		render_window
	@Access:		Public
	@Description:
		渲染窗体。
	@Parameters:
		window, struct Window *, IN
			指向窗体结构体的指针。
		top, BOOL, IN
			TRUE 则为置顶窗体，否则不是。
	@Return:
		BOOL
			返回TRUE则成功，否则失败。		
*/
BOOL
render_window(	IN struct Window * window,
				IN BOOL top)
{
	if(window == NULL)
		return FALSE;

	// 从隐藏的状态过渡到显示的状态时引发WINDOW_EVENT_SHOW事件。
	if(	window->old_state == WINDOW_STATE_HIDDEN
		&& window->state == WINDOW_STATE_SHOW)
		window_dispatch_event(window, WINDOW_EVENT_SHOW, NULL);
	// 从显示的状态过渡到隐藏的状态时引发WINDOW_EVENT_HIDDEN事件。
	else if(window->old_state == WINDOW_STATE_SHOW
			&& window->state == WINDOW_STATE_HIDDEN)
		window_dispatch_event(window, WINDOW_EVENT_HIDDEN, NULL);
	// 更新旧状态。
	window->old_state = window->state;

	if(	window->state == WINDOW_STATE_CLOSED
		|| window->state == WINDOW_STATE_HIDDEN)
		return FALSE;

	uint32 wstyle = window->style;
	BOOL has_title_bar = !(wstyle & WINDOW_STYLE_NO_TITLE);

	if(has_title_bar)
	{
		ImagePtr title_bar = &window->title_bar;

		// 填充标题栏。
		uint32 title_bar_bgcolor = top ? TITLE_BAR_BGCOLOR : TITLE_BAR_BGCOLOR_NT;
		rect_common_image(title_bar, 0, 0, window->width, TITLE_BAR_HEIGHT, title_bar_bgcolor);

		//关闭按钮
		if(wstyle & WINDOW_STYLE_CLOSE)
			if(top)
				if(window->over_close_button)
					draw_common_image(	title_bar, 
										&close_button_hover, 
										window->width - CLOSE_BUTTON_WIDTH, 
										0, 
										CLOSE_BUTTON_WIDTH, 
										CLOSE_BUTTON_HEIGHT);
				else		
					draw_common_image(	title_bar, 
										&close_button,
										window->width - CLOSE_BUTTON_WIDTH, 
										0, 
										CLOSE_BUTTON_WIDTH, 
										CLOSE_BUTTON_HEIGHT);
			else
				draw_common_image(	title_bar, 
									&close_button_blur, 
									window->width - CLOSE_BUTTON_WIDTH, 
									0, 
									CLOSE_BUTTON_WIDTH, 
									CLOSE_BUTTON_HEIGHT);
		else
			if(top)
				draw_common_image(	title_bar, 
									&close_button, 
									window->width - CLOSE_BUTTON_WIDTH,
									0, 
									CLOSE_BUTTON_WIDTH,
									CLOSE_BUTTON_HEIGHT);
			else
				draw_common_image(	title_bar, 
									&close_button_blur, 
									window->width - CLOSE_BUTTON_WIDTH,
									0, 
									CLOSE_BUTTON_WIDTH,
									CLOSE_BUTTON_HEIGHT);

		//隐藏按钮
		if(wstyle & WINDOW_STYLE_MINIMIZE)
			if(top)
				if(window->over_hidden_button)
					draw_common_image(	title_bar,
										&hidden_button_hover,
										window->width - CLOSE_BUTTON_WIDTH - HIDDEN_BUTTON_WIDTH,
										0,
										HIDDEN_BUTTON_WIDTH,
										HIDDEN_BUTTON_HEIGHT);
				else	
					draw_common_image(	title_bar,
										&hidden_button,
										window->width - CLOSE_BUTTON_WIDTH - HIDDEN_BUTTON_WIDTH,
										0,
										HIDDEN_BUTTON_WIDTH,
										HIDDEN_BUTTON_HEIGHT);
			else
				draw_common_image(	title_bar,
									&hidden_button_blur,
									window->width - CLOSE_BUTTON_WIDTH - HIDDEN_BUTTON_WIDTH,
									0,
									HIDDEN_BUTTON_WIDTH,
									HIDDEN_BUTTON_HEIGHT);
		else
			if(top)
				draw_common_image(	title_bar,
									&hidden_button,
									window->width - CLOSE_BUTTON_WIDTH - HIDDEN_BUTTON_WIDTH,
									0,
									HIDDEN_BUTTON_WIDTH,
									HIDDEN_BUTTON_HEIGHT);
			else
				draw_common_image(	title_bar,
									&hidden_button_blur,
									window->width - CLOSE_BUTTON_WIDTH - HIDDEN_BUTTON_WIDTH,
									0,
									HIDDEN_BUTTON_WIDTH,
									HIDDEN_BUTTON_HEIGHT);

		//画标题
		text_common_image(	title_bar,
							10,
							TITLE_BAR_HEIGHT / 2 - ENFONT_HEIGHT / 2,
							get_enfont_ptr(),
							window->title,
							(window->width - 10 - CLOSE_BUTTON_WIDTH - HIDDEN_BUTTON_WIDTH) / ENFONT_WIDTH,
							0xff000000);
	}

	// 如果窗体被锁定，则不发送WINDOW_EVENT_PAINT事件。
	if(!window->locked)
		window_dispatch_event(window, WINDOW_EVENT_PAINT, NULL);

	return TRUE;	
}

/**
	@Function:		window_get_key
	@Access:		Public
	@Description:
		获取指定窗体的按键缓冲区的按键。
	@Parameters:
		window, struct Window *, IN
			指向窗体结构体的指针。
	@Return:
		uint8
			键值。		
*/
uint8
window_get_key(IN struct Window * window)
{
	if(window == NULL || !window_has_key(window))
		return 0;
	uint8 key = window->key_buffer[0];
	uint32 ui;
	for(ui = 0; ui < window->key_count - 1; ui++)
		window->key_buffer[ui] = window->key_buffer[ui + 1];
	window->key_count--;
	return key;
}

/**
	@Function:		window_peek_key
	@Access:		Public
	@Description:
		获取指定窗体的按键缓冲区的按键。
		但不从按键缓冲区中取出按键。
	@Parameters:
		window, struct Window *, IN
			指向窗体结构体的指针。
	@Return:
		uint8
			键值。		
*/
uint8
window_peek_key(IN struct Window * window)
{
	if(window == NULL || !window_has_key(window))
		return 0;
	uint8 key = window->key_buffer[0];
	return key;
}

/**
	@Function:		window_dispatch_event
	@Access:		Public
	@Description:
		分发窗体事件。
	@Parameters:
		window, WindowPtr, IN
			指向窗体结构体的指针。
		type, int32, IN
			消息类型。
		data, void *, IN
			事件附加数据。
	@Return:
		uint8
			键值。		
*/
void
window_dispatch_event(	IN WindowPtr window,
						IN int32 type,
						IN void * data)
{
	if(	window == NULL
		|| window->event == NULL)
		return;
	uint32 wstyle = window->style;
	BOOL has_title_bar = !(wstyle & WINDOW_STYLE_NO_TITLE);
	int32 x, y;
	get_mouse_position(&x, &y);
	x = x - window->x;
	if(has_title_bar)
		y = y - window->y - TITLE_BAR_HEIGHT;
	else
		y = y - window->y;
	uint32 mouse_button = MOUSE_BUTTON_NONE;
	if(is_mouse_left_button_down())
		mouse_button |= MOUSE_BUTTON_LEFT;
	if(is_mouse_right_button_down())
		mouse_button |= MOUSE_BUTTON_RIGHT;
	struct WindowEventParams params;
	params.wid = window->id;
	params.event_type = type;
	params.mouse_x = x;
	params.mouse_y = y;
	params.mouse_button = mouse_button;
	params.screen = &window->workspace;
	params.data = data;
	window->event(window, &params);
}
