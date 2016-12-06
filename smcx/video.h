/* Copyright (c) 2016 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

#ifndef VIDEO_H
#define VIDEO_H

void video_disp_buffer(int width, int height, void *data);
void video_disp_res(int width, int height);
void video_window_update(int width, int height);

void video_prepare(void);
void video_loop(void);
int  video_init(void *handle);
void video_shutdown(void);


#endif