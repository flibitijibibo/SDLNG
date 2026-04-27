#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#define WIDTH 15
#define HEIGHT 10

int main(int argc, char **argv)
{
	struct
	{
		bool valid;
		bool tried;
		bool marked;
	} board[WIDTH][HEIGHT];
	size_t i, j;

	size_t columnmax;
	size_t columnhints[WIDTH][HEIGHT] = { 0 };
	size_t columnnumhints[WIDTH] = { 0 };
	size_t rowmax;
	size_t rowhints[HEIGHT][WIDTH] = { 0 };
	size_t rownumhints[HEIGHT] = { 0 };

	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Surface *png;
	SDL_Texture *texture;

	bool run;
	SDL_FPoint cursor;
	bool selectedSomething;

	TTF_TextEngine *textengine;
	char textbuffer[3];
	TTF_Font *font;

	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		return 0;
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

	run = true;
	do
	{
		SDL_Event evt;
		selectedSomething = false;
		while (SDL_PollEvent(&evt))
		{
			if (evt.type == SDL_EVENT_QUIT)
			{
				run = false;
			}
			else if (evt.type == SDL_EVENT_MOUSE_MOTION)
			{
				cursor.x = evt.motion.x;
				cursor.y = evt.motion.y;
			}
			else if (evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
			{
				float rowoffset = (evt.button.x / 50.0f) - rowmax;
				float columnoffset = (evt.button.y / 50.0f) - columnmax;
				if (
					rowoffset > 0 && rowoffset < WIDTH &&
					columnoffset > 0 && columnoffset < HEIGHT
				) {
					if (evt.button.button != 1)
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
		}

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
	} while (run);

	TTF_DestroyRendererTextEngine(textengine);
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
