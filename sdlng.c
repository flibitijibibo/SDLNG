#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#define WIDTH 15
#define HEIGHT 10

static struct
{
	bool valid;
	bool tried;
	bool marked;
} board[WIDTH][HEIGHT];
static size_t i, j;

static size_t columnmax;
static size_t columnhints[WIDTH][HEIGHT] = { 0 };
static size_t columnnumhints[WIDTH] = { 0 };
static size_t rowmax;
static size_t rowhints[HEIGHT][WIDTH] = { 0 };
static size_t rownumhints[HEIGHT] = { 0 };

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Surface *png;
static SDL_Texture *texture;

static SDL_FPoint cursor;

static TTF_TextEngine *textengine;
static char textbuffer[3];
static TTF_Font *font;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		return SDL_APP_FAILURE;
	}

	for (i = 0; i < WIDTH; i += 1)
	{
		for (j = 0; j < HEIGHT; j += 1)
		{
			board[i][j].valid = SDL_rand(2) == 0;
			board[i][j].tried = false;
			board[i][j].marked = false;
		}
	}

	columnmax = 0;
	for (i = 0; i < WIDTH; i += 1)
	{
		size_t count = 0;
		for (j = 0; j < HEIGHT; j += 1)
		{
			if (board[i][j].valid)
			{
				count += 1;
			}
			else
			{
				if (count > 0)
				{
					columnhints[i][columnnumhints[i]++] = count;
					count = 0;
				}
			}
		}
		if (count > 0)
		{
			columnhints[i][columnnumhints[i]++] = count;
		}
		columnmax = SDL_max(columnmax, columnnumhints[i]);
	}

	rowmax = 0;
	for (i = 0; i < HEIGHT; i += 1)
	{
		size_t count = 0;
		for (j = 0; j < WIDTH; j += 1)
		{
			if (board[j][i].valid)
			{
				count += 1;
			}
			else
			{
				if (count > 0)
				{
					rowhints[i][rownumhints[i]++] = count;
					count = 0;
				}
			}
		}
		if (count > 0)
		{
			rowhints[i][rownumhints[i]++] = count;
		}
		rowmax = SDL_max(rowmax, rownumhints[i]);
	}

	if (!SDL_CreateWindowAndRenderer(
		"SDLNG",
		50 * (WIDTH + rowmax),
		50 * (HEIGHT + columnmax),
		0,
		&window,
		&renderer
	)) {
		return 0;
	}
	SDL_SetRenderLogicalPresentation(
		renderer,
		50 * (WIDTH + rowmax),
		50 * (HEIGHT + columnmax),
		SDL_LOGICAL_PRESENTATION_LETTERBOX
	);

	png = SDL_LoadPNG("tiles.png");
	texture = SDL_CreateTextureFromSurface(renderer, png);
	SDL_DestroySurface(png);
	SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

	TTF_Init();
	textengine = TTF_CreateRendererTextEngine(renderer);
	font = TTF_OpenFont("LiberationMono-Regular.ttf", 50);

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	if (event->type == SDL_EVENT_QUIT)
	{
		return SDL_APP_SUCCESS;
	}
	else if (event->type == SDL_EVENT_MOUSE_MOTION)
	{
		cursor.x = event->motion.x;
		cursor.y = event->motion.y;
	}
	else if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
	{
		float rowoffset = (event->button.x / 50.0f) - rowmax;
		float columnoffset = (event->button.y / 50.0f) - columnmax;
		if (
			rowoffset > 0 && rowoffset < WIDTH &&
			columnoffset > 0 && columnoffset < HEIGHT
		) {
			if (event->button.button != 1)
			{
				board[(size_t) rowoffset][(size_t) columnoffset].marked =
					!board[(size_t) rowoffset][(size_t) columnoffset].marked;
			}
			else
			{
				board[(size_t) rowoffset][(size_t) columnoffset].tried = true;
			}
		}
	}
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	for (i = 0; i < WIDTH; i += 1)
	{
		for (j = 0; j < HEIGHT; j += 1)
		{
			SDL_FRect src, dst;

			src.y = 0;
			src.w = 10;
			src.h = 10;

			dst.x = 50 * (rowmax + i);
			dst.y = 50 * (columnmax + j);
			dst.w = 50;
			dst.h = 50;

			if (board[i][j].tried)
			{
				if (board[i][j].valid)
				{
					src.x = 40;
				}
				else
				{
					src.x = 50;
				}
			}
			else
			{
				if (SDL_PointInRectFloat(&cursor, &dst))
				{
					if (board[i][j].marked)
					{
						src.x = 30;
					}
					else
					{
						src.x = 10;
					}
				}
				else
				{
					if (board[i][j].marked)
					{
						src.x = 20;
					}
					else
					{
						src.x = 0;
					}
				}
			}

			SDL_RenderTexture(renderer, texture, &src, &dst);
		}
	}

	for (i = 0; i < WIDTH; i += 1)
	{
		const float ystart = columnmax - columnnumhints[i];

		if (columnnumhints[i] == 0)
		{
			continue;
		}

		for (j = 0; j < columnnumhints[i]; j += 1)
		{
			// FIXME: The churn here is terrrible -flibit
			TTF_Text *hint = TTF_CreateText(textengine, font, SDL_itoa(columnhints[i][j], textbuffer, 10), 0);
			TTF_DrawRendererText(hint, (i + rowmax) * 50, (ystart + j) * 50);
			TTF_DestroyText(hint);
		}
	}
	for (i = 0; i < HEIGHT; i += 1)
	{
		const float xstart = rowmax - rownumhints[i];

		if (rownumhints[i] == 0)
		{
			continue;
		}

		for (j = 0; j < rownumhints[i]; j += 1)
		{
			// FIXME: The churn here is terrrible -flibit
			TTF_Text *hint = TTF_CreateText(textengine, font, SDL_itoa(rowhints[i][j], textbuffer, 10), 0);
			TTF_DrawRendererText(hint, (xstart + j) * 50, (i + columnmax) * 50);
			TTF_DestroyText(hint);
		}
	}

	SDL_RenderPresent(renderer);
	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	TTF_DestroyRendererTextEngine(textengine);
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
