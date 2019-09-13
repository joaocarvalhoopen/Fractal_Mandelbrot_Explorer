/******************************************************************************
 *                                                                            *
 *                      Fractal Mandelbrot explorer                           *
 *                                                                            *
 ******************************************************************************
 * Author: Joao Nuno Carvalho                                                 *
 * Date:   2019.09.13                                                         *
 * License: MIT Open Source License                                           *
 * Description: This program is a interactive explorer of the Mandelbrot      *
 *              fractal set. You can zoom in, zoom out and save the image to  *
 *              disc.                                                         *
 *              This program is implemented in C++ and uses the SDL2 lib for  *
 *              direct graphics programming. The image renderer uses a        *
 *              texture vector memory map to speed up the drawing process.    *
 *              This program was developed on windows 10 with MINGW32.        *
 *              To compile do mingw32-make.exe.                               *
 * Manual: Mouse left click to zoom 3 times.                                  *
 *         Mouse right click to zoom back 3 times.                            *
 *         Space to save a Mandelbrot image in equal to the screen.           *
 *         Top right cross, to quit.                                          *
 *         Screen position on the to left corner.                             *
 ******************************************************************************
*/

#include <iostream>
#include <SDL2/SDL.h>
#include <cstdlib>
#include <complex>
#include <vector>
#include <sstream>

using namespace std;

constexpr int WINDOW_WIDTH  = 640;
constexpr int WINDOW_HEIGHT = 640;  // 480;
constexpr int MAX_MANDELBROT_ITERATIONS = 255; // 100

void get_zoom_in( vector<vector<double>> &vec,
                int x, int y, 
                double & cx_min, double & cx_max,
                double & cy_min, double & cy_max ){

    vector<double> inner_vec {cx_min, cx_max, cy_min, cy_max}; 
    vec.push_back(inner_vec);

    // Translate from screen coordinates, to complex plane coordinates.
    double cx_delta = (cx_max-cx_min);
    double cy_delta = (cy_max-cy_min);
    double cx = cx_min + (x / (WINDOW_WIDTH-1.0)) * cx_delta;
    double cy = cy_min + (y / (WINDOW_HEIGHT-1.0)) * cy_delta; 
    
    // The distance from the center is half, and them make 3x zoom.
    double one_third_cx = cx_delta  / (3.0 * 2.0);
    double one_third_cy = cy_delta  / (3.0 * 2.0);
    // Change the reference pointers.
    cx_min = cx - one_third_cx;
    cx_max = cx + one_third_cx;
    cy_min = cy - one_third_cy;
    cy_max = cy + one_third_cy;
}

void draw_mandelbrot(SDL_Renderer* renderer,                    // The renderer to draw the image pointer.
                     SDL_Texture* texture,                      // Texture pointer.
                     vector<unsigned char> pixels,              // Pixels texture buffer.
                     const double cx_min, const double cx_max,  // The rectangle in the complex plane.
                     const double cy_min, const double cy_max ){
    const size_t x_max = WINDOW_WIDTH;
    const size_t y_max = WINDOW_HEIGHT;
    const int texWidth = WINDOW_WIDTH;
    const double cx_delta = cx_max-cx_min;
    const double cy_delta = cy_max-cy_min;
    for (size_t x = 0; x < x_max; ++x)
        for (size_t y = 0; y < y_max; ++y){
            double real = cx_min + (x / (x_max-1.0)) * cx_delta;
            double img  = cy_min + (y / (y_max-1.0)) * cy_delta;
            complex<double> c(real, img);
            complex<double> z = 0;
            int iter;
            for (iter = 0; iter < MAX_MANDELBROT_ITERATIONS; ++iter){ 
                z = z*z + c;
                if (abs(z) > 2.0)
                    break;
            }
            const unsigned int offset = ( texWidth * 4 * y ) + x * 4;
            if (iter == MAX_MANDELBROT_ITERATIONS){
                // Stable region.
                pixels[ offset + 0 ] = 0;                   // b
                pixels[ offset + 1 ] = 0;                   // g
                pixels[ offset + 2 ] = 0;                   // r
                pixels[ offset + 3 ] = SDL_ALPHA_OPAQUE;    // a
            }else{
                // Instable region.
                pixels[ offset + 0 ] = 20 + iter*5, 255;    // b
                pixels[ offset + 1 ] = 0;                   // g
                pixels[ offset + 2 ] = 0;                   // r
                pixels[ offset + 3 ] = SDL_ALPHA_OPAQUE;    // a
            }
        }

    SDL_UpdateTexture(texture, NULL, &pixels[0], texWidth * 4);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);   
}

void write_title(SDL_Window *win, int mouseX, int mouseY, int zoom){
    stringstream ss;
    ss << "Fractal Mandelbrot "<< "X: " << mouseX << " Y: " << mouseY << " zoom: " << zoom;
    SDL_SetWindowTitle(win, ss.str().c_str());
}

// From: https://stackoverflow.com/questions/34255820/save-sdl-texture-to-file/51238719
void save_texture(SDL_Renderer *ren, SDL_Texture *tex, const char *filename)
{
    SDL_Texture *ren_tex;
    SDL_Surface *surf;
    int st;
    int w;
    int h;
    int format;
    void *pixels;

    pixels  = NULL;
    surf    = NULL;
    ren_tex = NULL;
    format  = SDL_PIXELFORMAT_RGBA32;

    /* Get information about texture we want to save */
    st = SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    if (st != 0) {
        SDL_Log("Failed querying texture: %s\n", SDL_GetError());
        goto cleanup;
    }

    ren_tex = SDL_CreateTexture(ren, format, SDL_TEXTUREACCESS_TARGET, w, h);
    if (!ren_tex) {
        SDL_Log("Failed creating render texture: %s\n", SDL_GetError());
        goto cleanup;
    }

    /*
     * Initialize our canvas, then copy texture to a target whose pixel data we 
     * can access
     */
    st = SDL_SetRenderTarget(ren, ren_tex);
    if (st != 0) {
        SDL_Log("Failed setting render target: %s\n", SDL_GetError());
        goto cleanup;
    }

    SDL_SetRenderDrawColor(ren, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(ren);

    st = SDL_RenderCopy(ren, tex, NULL, NULL);
    if (st != 0) {
        SDL_Log("Failed copying texture data: %s\n", SDL_GetError());
        goto cleanup;
    }

    /* Create buffer to hold texture data and load it */
    pixels = malloc(w * h * SDL_BYTESPERPIXEL(format));
    if (!pixels) {
        SDL_Log("Failed allocating memory\n");
        goto cleanup;
    }

    st = SDL_RenderReadPixels(ren, NULL, format, pixels, w * SDL_BYTESPERPIXEL(format));
    if (st != 0) {
        SDL_Log("Failed reading pixel data: %s\n", SDL_GetError());
        goto cleanup;
    }

    /* Copy pixel data over to surface */
    surf = SDL_CreateRGBSurfaceWithFormatFrom(pixels, w, h, SDL_BITSPERPIXEL(format), w * SDL_BYTESPERPIXEL(format), format);
    if (!surf) {
        SDL_Log("Failed creating new surface: %s\n", SDL_GetError());
        goto cleanup;
    }

    /* Save result to an image */
    st = SDL_SaveBMP(surf, filename);
    if (st != 0) {
        SDL_Log("Failed saving image: %s\n", SDL_GetError());
        goto cleanup;
    }

    SDL_Log("Saved texture as BMP to \"%s\"\n", filename);

cleanup:
    SDL_FreeSurface(surf);
    free(pixels);
    SDL_DestroyTexture(ren_tex);
}

int main( int argc, char *argv[] )
{
    // Start SDL.
    if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
    {
        std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError( ) << std::endl;
    }
    
    // Opening a window.
    SDL_Window *win = SDL_CreateWindow("Fractal Mandelbrot", 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (win == nullptr){
	    std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
	    SDL_Quit();
	    return 1;
    }

    // Creating the render.
    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr){
	    SDL_DestroyWindow(win);
	    std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
	    SDL_Quit();
	    return 1;
    }

    SDL_Event event;

    // The rect to draw in the complex plane.
    double cx_min = -2.0;
    double cx_max =  1.0;
    double cy_min = -1.5;
    double cy_max =  1.5;

    vector<vector<double>> vec {};
    vector<double> inner_vec {cx_min, cx_max, cy_min, cy_max};
    vec.push_back(inner_vec);

    SDL_RendererInfo info;
    SDL_GetRendererInfo( renderer, &info );
    cout << "Renderer name: " << info.name << endl;
    cout << "Texture formats: " << endl;
    for( Uint32 i = 0; i < info.num_texture_formats; i++ )
    {
        cout << SDL_GetPixelFormatName( info.texture_formats[i] ) << endl;
    }

    const unsigned int texWidth = WINDOW_WIDTH;
    const unsigned int texHeight = WINDOW_HEIGHT;
    SDL_Texture* texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             texWidth, texHeight);

    vector<unsigned char> pixels( texWidth * texHeight * 4, 0 );
 
    // The rect to draw in the complex plane
    draw_mandelbrot(renderer, texture, pixels, cx_min, cx_max, cy_min, cy_max );

    int mouseX      = 0;
    int mouseY      = 0;
    int zoom        = 1;
    int file_number = 0;

    bool flag_running = true;
    // Handle events.
    while (flag_running) {
        SDL_WaitEvent(&event);
 
        switch (event.type)
        {
            case SDL_QUIT:
                flag_running = false;
                break;

            case SDL_MOUSEBUTTONDOWN:
                switch (event.button.button)
                {
                    case SDL_BUTTON_LEFT:
                        if (zoom == 1)
                            zoom = 3;
                        else
                            zoom += 3;
                        get_zoom_in(vec, mouseX, mouseY, 
                                    cx_min, cx_max, cy_min, cy_max);
                        //cout << "cx_min: " << cx_min << " cx_max: " << cx_max << " cy_min: " << cy_min << "cy_max" << cy_max << endl;            
                        // The rect to draw in the complex plane
                        draw_mandelbrot(renderer, texture, pixels, cx_min, cx_max, cy_min, cy_max );  
                        write_title(win, mouseX, mouseY, zoom);
                        break;

                    case SDL_BUTTON_RIGHT:
                        if (vec.size() > 0){
                            if (zoom == 3 || zoom == 1 )
                                zoom = 1;
                            else
                                zoom -= 3;                            
                            vector<double> inner_vec = vec.back();
                            if (zoom != 1)
                                vec.pop_back();
                            cx_min = inner_vec[0];
                            cx_max = inner_vec[1];
                            cy_min = inner_vec[2];
                            cy_max = inner_vec[3];
                        }
                        
                        //cout << "cx_min: " << cx_min << " cx_max: " << cx_max << " cy_min: " << cy_min << "cy_max" << cy_max << endl;

                        // The rect to draw in the complex plane
                        draw_mandelbrot(renderer, texture, pixels, cx_min, cx_max, cy_min, cy_max );  
                        write_title(win, mouseX, mouseY, zoom);
                        break;

                }
                break;

            case SDL_MOUSEMOTION:
                mouseX = event.motion.x;
                mouseY = event.motion.y;
                write_title(win, mouseX, mouseY, zoom);
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_SPACE:  
                        // Save image to file....
                        stringstream ss_filename;
                        ss_filename << "./img/Mandelbrot_"<< file_number << ".bmp";
                        save_texture(renderer, texture, ss_filename.str().c_str());
                        file_number++;
                        break;
                }
                break;

        }
    }

    // system("pause");

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return EXIT_SUCCESS;
}