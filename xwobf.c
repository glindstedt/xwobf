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

#include <stdio.h>
#include <getopt.h>
#include <err.h>
#include <xcb/xcb.h>
#include <wand/MagickWand.h>

#include "xwobf.h"

MagickWand        *wand = NULL;
MagickWand    *obs_wand = NULL;

xcb_connection_t *xcb_c = NULL;
xcb_screen_t   *xcb_scr = NULL;

// Used to store the position/width/height of all visible windows
rectangle_t      **rect = NULL;
size_t        rect_size = 0;

void print_usage()
{
    printf("Usage: xwobf [OPTION]... DEST\n");
    printf("  -h --help\t\tprint this message and exit\n");
    printf("  -s pixel_size\t\tadjust the obfuscation strength (default=9)\n");
    printf("  --size=pixel_size\n"); 
    printf("  -f --fuzzy\t\tadd a blur effect\n");
}

int main(int argc, char **argv)
{
    char *file;
    int pixel_size = 9;
    int fuzzy = 0;

    char *optstring = "hs:f";
    struct option longopts[] = {
        {"help", no_argument, NULL, 'h'},
        {"size",  required_argument, NULL,  's' },
        {"fuzzy",  no_argument, NULL,  'f' },
        {NULL, no_argument, NULL, 0}
    };

    int c;
    while ((c = getopt_long(argc, argv, optstring, longopts, NULL)) != -1) {
        switch(c) {
            case 's':
                if (sscanf (optarg, "%i", &pixel_size)!=1) {
                    puts("Size argument should be an integer");
                    exit(EXIT_FAILURE);    
                }
                break;
            case 'f':
                fuzzy = 1;
                break;
            default:
                print_usage();
                exit(EXIT_SUCCESS);
        }
    }

    // Read the destination file
    if (optind < argc) {
        file = argv[optind];
    } else {
        printf("No output file given.\n");
        print_usage();
        exit(EXIT_FAILURE);
    }

    init();

    (void)MagickReadImage(wand,"x:root");

    obscure_image(pixel_size, fuzzy);

    (void)MagickWriteImage(wand,file);

    cleanup();

    return EXIT_SUCCESS;
}

void init()
{

    int screen_num;

    // Connect to the x server
    xcb_c = xcb_connect(NULL, &screen_num);

    check_xcb_error(xcb_c);

    // Find the root window
    xcb_screen_iterator_t s_it = xcb_setup_roots_iterator(xcb_get_setup(xcb_c));
    for (; s_it.rem; --screen_num, xcb_screen_next (&s_it)) {
        if (screen_num == 0) {
          xcb_scr = s_it.data;
          break;
        }
    }

    MagickWandGenesis();
    wand = NewMagickWand();

    find_rectangles();
}

// Check if the xcb_connection has encountered an error.
// Exit if an error has occurred.
void check_xcb_error(xcb_connection_t *xcb_connection)
{
    int xcb_error = xcb_connection_has_error(xcb_connection);
    if (xcb_error > 0) {
        switch(xcb_error) {
            case XCB_CONN_ERROR:
                errx(EXIT_FAILURE,"[XCB_CONN_ERROR]");
                break;
            case XCB_CONN_CLOSED_EXT_NOTSUPPORTED:
                errx(EXIT_FAILURE,"[XCB_CONN_CLOSED_EXT_NOTSUPPORTED]");
                break;
            case XCB_CONN_CLOSED_MEM_INSUFFICIENT:
                errx(EXIT_FAILURE,"[XCB_CONN_CLOSED_MEM_INSUFFICIENT]");
                break;
            case XCB_CONN_CLOSED_REQ_LEN_EXCEED:
                errx(EXIT_FAILURE,"[XCB_CONN_CLOSED_REQ_LEN_EXCEED]");
                break;
            case XCB_CONN_CLOSED_PARSE_ERR:
                errx(EXIT_FAILURE,"[XCB_CONN_CLOSED_PARSE_ERR]");
                break;
            case XCB_CONN_CLOSED_INVALID_SCREEN:
                errx(EXIT_FAILURE,"[XCB_CONN_CLOSED_INVALID_SCREEN]");
                break;
            default:
                printf("[UNKNOWN_ERROR] %d\n", xcb_error);
                exit(EXIT_FAILURE);
        }
    }
}

void cleanup()
{
    xcb_disconnect(xcb_c);

    if (wand)         wand = DestroyMagickWand(wand);
    if (obs_wand) obs_wand = DestroyMagickWand(obs_wand);
    MagickWandTerminus();

    free_rectangles();
}

// Obscure the image!
void obscure_image(int blur_size, int fuzzy)
{
    for(size_t i = 0; i < rect_size; ++i) {
        obscure_rectangle(rect[i], blur_size, fuzzy);
    }
}

// Obscure the area within the given rectangle
void obscure_rectangle(rectangle_t *rec, int pixel_size, int fuzzy)
{
    if ((obs_wand = CloneMagickWand(wand))) {
        (void)MagickCropImage(obs_wand, rec->w, rec->h, rec->x, rec->y);

        // This is where the magick happens
        (void)MagickResizeImage(obs_wand, (rec->w)/pixel_size, (rec->h)/pixel_size,
                PointFilter, 0);
        if (fuzzy) {
                (void)MagickBlurImage(obs_wand, 0, 1);
        }        
        (void)MagickResizeImage(obs_wand, rec->w, rec->h,
                PointFilter, 0);

        (void)MagickCompositeImage(wand, obs_wand, OverCompositeOp, rec->x, rec->y);

        obs_wand = DestroyMagickWand(obs_wand);
    }
}

// Check if a window is visible
// Return 1 if visible, 0 otherwise
int window_is_visible(xcb_window_t win)
{
    xcb_get_window_attributes_cookie_t cookie;
    xcb_get_window_attributes_reply_t *reply;
    cookie = xcb_get_window_attributes(xcb_c, win);

    int retval = 0;

    if ((reply = xcb_get_window_attributes_reply(xcb_c, cookie, NULL))) {
        if (XCB_MAP_STATE_VIEWABLE == (*reply).map_state)
            retval = 1;
        free(reply);
    }
    return retval;
}

// Populate an array of rectangle_t pointers with all the visible windows
void find_rectangles()
{
    xcb_query_tree_cookie_t cookie = xcb_query_tree(xcb_c, xcb_scr->root);
    xcb_query_tree_reply_t *reply;

    if ((reply = xcb_query_tree_reply(xcb_c, cookie, NULL))) {

        xcb_window_t *children = xcb_query_tree_children(reply);
        int num_children = xcb_query_tree_children_length(reply);

        // At most there are num_children visible windows
        rect = malloc(sizeof(rectangle_t*) * num_children);

        // Use rect_size to count the number of visible rectangles
        rect_size = 0;

        for (int i = 0; i < xcb_query_tree_children_length(reply); i++) {
            if (window_is_visible(children[i])) {
                rect[rect_size++] = get_rectangle(children[i]);
            }
        }

        free(reply);
    }
}

// Get the position, width and height of an xcb_window
rectangle_t *get_rectangle(xcb_window_t win)
{
    xcb_get_geometry_cookie_t cookie;
    xcb_get_geometry_reply_t *reply;
    cookie = xcb_get_geometry(xcb_c, win);

    if ((reply = xcb_get_geometry_reply(xcb_c, cookie, NULL))) {
        rectangle_t *rectangle = malloc(sizeof(rectangle_t));
        rectangle->x = (size_t) reply->x;
        rectangle->y = (size_t) reply->y;
        rectangle->w = (size_t) reply->width;
        rectangle->h = (size_t) reply->height;

        free(reply);
        return rectangle;
    }

    return NULL;
}

// Free every rectangle in the array, including the array itself
void free_rectangles()
{
    for(size_t i = 0; i < rect_size; ++i) {
        free(rect[i]);
    }
    free(rect);
}

// DEBUG FUNCTIONS
void print_rectangle(rectangle_t *rec)
{
    printf("Rec { x: %*zu y: %*zu w: %*zu h: %*zu }\n",
            4,rec->x,4,rec->y,4,rec->w,4,rec->h);
}

void print_rectangle_array(rectangle_t **rec_arr, size_t size)
{
    printf("RecArray {\n");
    for(size_t i = 0; i < size; ++i) {
        printf("    ");
        print_rectangle(rec_arr[i]);
    }
    printf("}\n");
}
