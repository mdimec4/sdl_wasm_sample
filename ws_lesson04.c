// based on https://github.com/Twinklebear/TwinklebearDev-Lessons/tree/master/Lesson3/src
#include <emscripten.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>


/*
 * Recurse through the list of arguments to clean up, cleaning up
 * the first one in the list each iteration.
 */
void cleanup_window(SDL_Window *win){
	if (!win){
		return;
	}
	SDL_DestroyWindow(win);
}


void cleanup_renderer(SDL_Renderer *ren){
	if (!ren){
		return;
	}
	SDL_DestroyRenderer(ren);
}

void cleanup_texture(SDL_Texture *tex){
	if (!tex){
		return;
	}
	SDL_DestroyTexture(tex);
}

/*
 * Lesson 3: SDL Extension Libraries
 */
//Screen attributes
const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 480;
//We'll be scaling our tiles to be 40x40
const int TILE_SIZE = 40;

/*
 * Log an SDL error with some error message to the output stream of our choice
 * @param os The output stream to write the message too
 * @param msg The error message to write, format will be msg error: SDL_GetError()
 */
void logSDLError(FILE *os, const char* msg){
	fprintf(os, " error: %s\n", SDL_GetError());
}
/*
 * Loads an image into a texture on the rendering device
 * @param file The image file to load
 * @param ren The renderer to load the texture onto
 * @return the loaded texture, or nullptr if something went wrong.
 */
SDL_Texture* loadTexture(const char* file, SDL_Renderer *ren){
	SDL_Texture *texture = IMG_LoadTexture(ren, file);
	if (texture == NULL){
		logSDLError(stdout, "LoadTexture");
	}
	return texture;
}
/*
 * Draw an SDL_Texture to an SDL_Renderer at position x, y, with some desired
 * width and height
 * @param tex The source texture we want to draw
 * @param rend The renderer we want to draw too
 * @param x The x coordinate to draw too
 * @param y The y coordinate to draw too
 * @param w The width of the texture to draw
 * @param h The height of the texture to draw
 */
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, int w, int h){
	//Setup the destination rectangle to be at the position we want
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	dst.w = w;
	dst.h = h;
	SDL_RenderCopy(ren, tex, NULL, &dst);
}
/*
 * Draw an SDL_Texture to an SDL_Renderer at position x, y, preserving
 * the texture's width and height
 * @param tex The source texture we want to draw
 * @param rend The renderer we want to draw too
 * @param x The x coordinate to draw too
 * @param y The y coordinate to draw too
 */
void renderTexture2(SDL_Texture *tex, SDL_Renderer *ren, int x, int y){
	int w, h;
	SDL_QueryTexture(tex, NULL, NULL, &w, &h);
	renderTexture(tex, ren, x, y, w, h);
}

typedef struct loop_context {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *background;
	SDL_Texture *image;
	int i;
	
} loop_context;

//A sleepy rendering loop, wait for 3 seconds and render and present the screen each time
void loop_fn(void *arg) {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *background;
	SDL_Texture *image;
	loop_context* lc;
	
	lc = (loop_context*)arg;
	window = lc->window;
	renderer = lc->renderer;
	background = lc->background;
	image = lc->image;
	image = lc->image;
	
	if(lc->i >= 3){
		//Destroy the various items
		cleanup_texture(background);
		cleanup_texture(image);
		cleanup_renderer(renderer);
		cleanup_window(window);
		IMG_Quit(); 
		SDL_Quit();
		emscripten_cancel_main_loop();
		return;
	}
	
	//Clear the window
	SDL_RenderClear(renderer);

	//Determine how many tiles we'll need to fill the screen
	int xTiles = SCREEN_WIDTH / TILE_SIZE;
	int yTiles = SCREEN_HEIGHT / TILE_SIZE;

	//Draw the tiles by calculating their positions
	for (int i = 0; i < xTiles * yTiles; ++i){
		int x = i % xTiles;
		int y = i / xTiles;
		renderTexture(background, renderer, x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
	}

	//Draw our image in the center of the window
	//We need the foreground image's width to properly compute the position
	//of it's top left corner so that the image will be centered
	int iW, iH;
	SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
	int x = SCREEN_WIDTH / 2 - iW / 2;
	int y = SCREEN_HEIGHT / 2 - iH / 2;
	renderTexture2(image, renderer, x, y);

	//Update the screen
	SDL_RenderPresent(renderer);

	lc->i++;
}


int main(int argc, char** argv){
	//Start up SDL and make sure it went ok
	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		logSDLError(stdout, "SDL_Init");
		return 1;
	}

	//Setup our window and renderer
	SDL_Window *window = SDL_CreateWindow("Lesson 3", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL){
		logSDLError(stdout, "CreateWindow");
		SDL_Quit();
		return 1;
	}
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL){
		logSDLError(stdout, "CreateRenderer");
		cleanup_window(window);
		SDL_Quit();
		return 1;
	}

	//The textures we'll be using
	SDL_Texture *background = loadTexture("./background.png", renderer);
	SDL_Texture *image = loadTexture("./image.png", renderer);
	//Make sure they both loaded ok
	if (background == NULL || image == NULL){
		cleanup_texture(background);
		cleanup_texture(image);
		cleanup_renderer(renderer);
		cleanup_window(window);
		SDL_Quit();
		return 1;
	}

	loop_context lc;
	lc.window = window;
	lc.renderer = renderer;
	lc.background = background;
	lc.image = image;
	lc.i = 0;
	int simulate_infinite_loop = 1;
	int fps = 1;
	emscripten_set_main_loop_arg(loop_fn, &lc, fps, simulate_infinite_loop);
	 /**
	  *      * If simulate_infinite_loop = 0, emscripten_set_main_loop_arg won't block
	  *           * and this code will run straight away.
	  *                * If simulate_infinite_loop = 1 then this code will not be reached
	  *                     */
	return 0;
}

