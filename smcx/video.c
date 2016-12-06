/* Copyright (c) 2016 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

#include "video.h"

#include <Windows.h>

#include <gl/gl.h>
#pragma comment(lib, "opengl32.lib")

typedef int(__stdcall* twglSwapIntervalEXT)(int interval);
twglSwapIntervalEXT wglSwapInterval;

HWND  window;
HDC   dev_ctx;
HGLRC ren_ctx;

enum
{
	VMSG_DISP_BUFFER = 1,
	VMSG_DISP_RES,
	VMSG_WINDOW_UPDATE
};

struct
{
	int msg;
	int word1;
	int word2;
	void *ptr;
} vmsg[1024];

int vmsg_read, vmsg_write;

GLuint  disp_tex;
void*   disp_data;
GLsizei disp_tw, disp_th;
GLsizei disp_dw, disp_dh;

GLfloat disp_offx, disp_offy;
GLfloat disp_width, disp_height;

int wnd_width;
int wnd_height;
int wnd_changed;

void draw_plane(GLfloat offx, GLfloat offy, GLfloat w, GLfloat h)
{
	// COLORS RGB FG D2D4E0 BG 212435

	glBegin(GL_TRIANGLE_STRIP);
		glColor3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(offx, offy);

		glColor3f(0.0f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(offx, offy + h);

		glColor3f(0.0f, 0.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(offx + w, offy);

		glColor3f(1.0f, 1.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(offx + w, offy + h);
	glEnd();
}

void update_quad()
{
	double sx, sy;
	int s;

	sx = wnd_width / (double)disp_dw;
	sy = wnd_height / (double)disp_dh;
	s  = (int)min(sx, sy);

	if(s > 0)
	{
		disp_width = disp_dw * s;
		disp_height = disp_dh * s;

		disp_offx = (wnd_width  - disp_width) / 2;
		disp_offy = (wnd_height - disp_height) / 2;
	}
}

void video_proc(int msg, int word1, int word2, void *ptr)
{
	switch(msg)
	{
	case VMSG_DISP_BUFFER:
		if(disp_tex)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			glDeleteTextures(1, &disp_tex);
		}

		if(ptr)
		{
			glGenTextures(1, &disp_tex);
			glBindTexture(GL_TEXTURE_2D, disp_tex);

			disp_tw   = word1;
			disp_th   = word2;
			disp_data = ptr;

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, disp_tw, disp_th, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptr);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		break;
	
	case VMSG_DISP_RES:
		disp_dw = word1;
		disp_dh = word2;
		update_quad();
		break;

	case VMSG_WINDOW_UPDATE:
		wnd_width   = word1;
		wnd_height  = word2;
		wnd_changed = 1;
		update_quad();
		break;
	}
}

#include <stdio.h>
GLenum puff;

void video_loop(void)
{
	while(vmsg_read != vmsg_write)
	{
		video_proc( vmsg[vmsg_read].msg,
			vmsg[vmsg_read].word1,
			vmsg[vmsg_read].word2,
			vmsg[vmsg_read].ptr);
	
		vmsg_read = (vmsg_read + 1) & 0x3FF;
	}

	if(wnd_changed)
	{
		wnd_changed = 0;

		glViewport(0, 0, wnd_width, wnd_height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, wnd_width, 0, wnd_height, -1.0, 1.0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	

	if(disp_data)
	{
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, disp_tw, disp_th, GL_RGBA, GL_UNSIGNED_BYTE, disp_data);
		draw_plane(disp_offx, disp_offy, disp_width, disp_height);
	}

	SwapBuffers(dev_ctx);
}

void video_prepare(void)
{
	wglMakeCurrent(dev_ctx, ren_ctx);
	glClearColor(0.10f, 0.10f, 0.10f, 1.0f);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	wglSwapInterval(1);
}

void video_msg(int msg, int word1, int word2, void *ptr)
{
	vmsg[vmsg_write].msg   = msg;
	vmsg[vmsg_write].word1 = word1;
	vmsg[vmsg_write].word2 = word2;
	vmsg[vmsg_write].ptr   = ptr;
	
	vmsg_write = (vmsg_write + 1) & 0x3FF;
}

void video_disp_buffer(int width, int height, void *data)
{
	video_msg(VMSG_DISP_BUFFER, width, height, data);
}

void video_disp_res(int width, int height)
{
	video_msg(VMSG_DISP_RES, width, height, NULL);
}

void video_window_update(int width, int height)
{
	video_msg(VMSG_WINDOW_UPDATE, width, height, NULL);
}

int  video_init(void *handle)
{
	PIXELFORMATDESCRIPTOR pfd;
	int  pf;
	BOOL pfr;

	window = (HWND)handle;

	dev_ctx = GetDC(window);
	if(dev_ctx == 0) return 1;

	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;// | PFD_DEPTH_DONTCARE;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	pf = ChoosePixelFormat(dev_ctx, &pfd);
	if (pf == 0) return 2;

	pfr = SetPixelFormat(dev_ctx, pf, &pfd);
	if (pfr == 0) return 3;

	ren_ctx = wglCreateContext(dev_ctx);
	if(ren_ctx == 0) return 4;

	wglMakeCurrent(dev_ctx, ren_ctx);
	wglSwapInterval = (twglSwapIntervalEXT)wglGetProcAddress("wglSwapIntervalEXT");
	wglMakeCurrent(NULL, NULL);

	return ERROR_SUCCESS;
}


void video_shutdown(void)
{

}
