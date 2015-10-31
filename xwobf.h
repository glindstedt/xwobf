/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Gustaf Lindstedt
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _H_XWOBF
#define _H_XWOBF

#ifdef DEBUG
  #define DEBUG_PRINT(fmt, args...) fprintf(stderr, "[DEBUG] %s:%d:%s(): " fmt,\
          __FILE__, __LINE__, __func__, ##args);
#else
  #define DEBUG_PRINT(fmt, args...)
#endif

typedef struct rectangle_t {
    size_t x;
    size_t y;
    size_t w;
    size_t h;
} rectangle_t;

void print_usage();

void init();
void check_xcb_error(xcb_connection_t *xcb_connection);
void cleanup();

int window_is_visible(xcb_window_t win);

rectangle_t *get_rectangle(xcb_window_t win);
void find_rectangles();
void free_rectangles();

void obscure_image(int blur_size);
void obscure_rectangle(rectangle_t *rec, int blur_size);

void print_rectangle(rectangle_t *rec);
void print_rectangle_array(rectangle_t **rec_arr, size_t size);

#endif
