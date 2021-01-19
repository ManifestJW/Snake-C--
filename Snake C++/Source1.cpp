
#include <iostream>
#include <SDL.h>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <SDL_ttf.h>

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 704;
bool run = true;

SDL_Window* window = NULL;
SDL_Surface* windowSurface = NULL;
SDL_Surface* textSurface = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* textTexture = NULL;
TTF_Font* font;
SDL_Color White = { 255, 255, 255 };

int prevTime = 0;
int currentTime = 0;
float deltaTime = 0.0;
int moveSpeed = 160;
bool appleHasMoved = true;
int score = 0;

class Player {
public:
	float absx = 368.0;
	float absy = 320.0;
	int grid_x = 368;
	int grid_y = 320;
	int prevPos_x = 0;
	int prevPos_y = 0;
	int currentPos_x = 368;
	int currentPos_y = 329;
	float delta_pos_x = 0.0;
	float delta_pos_y = 0.0;
	int direction = 0;
	bool justTurned = false;
	int bodyLength = 1;
};


class Apple {
public:
	int x = 0;
	int y = 0;
	int prevPos_x = 0;
	int prevPos_y = 0;
	int currentPos_x = 0;
	int currentPos_y = 0;
	float delta_pos_x = 0.0;
	float delta_pos_y = 0.0;
};


Player snake;
Apple apple;
std::vector<std::vector<int>> snakeBody;
std::vector<bool> appleInSnake;

bool init() {
	bool success = true;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "Could not initialize! " << SDL_GetError() << std::endl;
		success = false;
	}
	else {
		window = SDL_CreateWindow("Snake", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL) {
			std::cout << "Could not create window! " << SDL_GetError() << std::endl;
			success = false;
		}
		else {
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			if (renderer == NULL) {
				std::cout << "Could not create renderer! " << SDL_GetError() << std::endl;
				success = false;
			}
			else {
				if (TTF_Init() < 0) {
					std::cout << "Could not initialize fonts! " << SDL_GetError() << std::endl;
					success = false;
				}
				else {
					windowSurface = SDL_GetWindowSurface(window);
				}
			}
		}
	}

	return success;
}

bool loadAssets() {
	bool success = true;

	font = TTF_OpenFont("assets/consola.ttf", 24);

	if (font == NULL) {
		std::cout << "Could not load font! " << SDL_GetError() << std::endl;
		success = false;
	}
	else {
		textSurface = TTF_RenderText_Solid(font, "Score: " + score, White);
		if (textSurface == NULL) {
			std::cout << "Could not render text! " << SDL_GetError() << std::endl;
			success = false;
		}
		else {
			textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
			if (textTexture == NULL) {
				std::cout << "Could not create text texture! " << SDL_GetError() << std::endl;
				success = false;
			}
			else {
				success = true;
			}
		}
	}

	return success;
}

void getRandomApplePos() {
	// variable that tells the while loop whether the moving of the apple was successful
	bool success;
	
	// variable that is set to false if the apple is inside the snakes body
	bool appleNotInside;

	// Tells the collision to stop testing for collision until the apple has successfully moved
	bool appleHasMoved;
	
	// sets the variables 
	success = false;
	appleNotInside = false;
	appleHasMoved = false;

	// while the apple spawns inside the snake, it keeps generating new positions 
	while (!success) {
		// gets a random position
		int randomX = rand() % 769;
		int randomY = rand() % 673;

		// resets the two variables if this while loop as ran again
		apple.delta_pos_x = 0;
		apple.delta_pos_y = 0;
		
		// checks to see if the apple has spawned in the same exact position
		while (apple.delta_pos_x == 0 && apple.delta_pos_y == 0) {
			// gets the previous poition of the apple
			apple.prevPos_x = apple.x;
			apple.prevPos_y = apple.y;

			// picks a new apple position
			apple.x = round((randomX) / 32) * 32;
			apple.y = round((randomY) / 32) * 32;

			// gets the new apple position
			apple.currentPos_x = apple.x;
			apple.currentPos_y = apple.y;

			// sets the difference between the positions, if it's 0, then it has spawned in the same exact location
			apple.delta_pos_x = (float)(apple.currentPos_x - apple.prevPos_x);
			apple.delta_pos_y = (float)(apple.currentPos_y - apple.prevPos_y);
		}

		// checks to see if the snake length is only one, as to make the list not go out of index
		if (snake.bodyLength == 1) {
			// if the apple happens to spawn inside the snake with a length of 1, it will add false to the appleInSnake vector, else it adds true
			if (apple.x == snakeBody[0][0] && apple.y == snakeBody[0][1]) {
				appleNotInside = false;
				appleInSnake.push_back(appleNotInside);
			}
			else {
				appleNotInside = true;
				appleInSnake.push_back(appleNotInside);
			}
		}
		else {
			// if the apple happens to spawn inside the currently compared snakeBodyPosition, it will add false to the appleInSnake vector, else it adds true
			for (int i = 0; i < snakeBody.size(); i++) {
				if (apple.x == snakeBody[i][0] && apple.y == snakeBody[i][1]) {
					appleNotInside = false;
					appleInSnake.push_back(appleNotInside);
				}
				else {
					appleNotInside = true;
					appleInSnake.push_back(appleNotInside);
				}
			}
		}

		// if false appears inside the appleInSnake vector at all, it sets success to false and goes through the loop again. Else it breaks out.
		if (std::find(appleInSnake.begin(), appleInSnake.end(), false) != appleInSnake.end()) {
			success = false;
		}
		else {
			success = true;
		}

		//clears appleInSnake so that it can take in a new comparision
		appleInSnake.clear();
	}

	// tells the collision to start back up again
	appleHasMoved = true;
	
}

void draw() {
	SDL_RenderClear(renderer);

	SDL_Rect rect;
	
	rect.x = 0;
	rect.y = 0;
	rect.w = 32;
	rect.h = 32;

	for (int c = 0; c < 22; c++) {
		for (int r = 0; r < 25; r++) {
			if (c % 2 == 0) {
				if (r % 2 == 0) {
					SDL_SetRenderDrawColor(renderer, 18, 131, 224, 255);
					SDL_RenderFillRect(renderer, &rect);
					rect.x += 32;
				}
				else {
					SDL_SetRenderDrawColor(renderer, 9, 88, 153, 255);
					SDL_RenderFillRect(renderer, &rect);
					rect.x += 32;
				}
			}
			else {
				if (r % 2 == 0) {
					SDL_SetRenderDrawColor(renderer, 9, 88, 153, 255);
					SDL_RenderFillRect(renderer, &rect);
					rect.x += 32;
				}
				else {
					SDL_SetRenderDrawColor(renderer, 18, 131, 224, 255);
					SDL_RenderFillRect(renderer, &rect);
					rect.x += 32;
				}
			}
		}

		rect.x = 0;
		rect.y += 32;
	}
	
	rect.x = apple.x;
	rect.y = apple.y;
	rect.w = 32;
	rect.h = 32;

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &rect);

	rect.x = apple.x + 4;
	rect.y = apple.y + 4;
	rect.w = 24;
	rect.h = 24;

	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderFillRect(renderer, &rect);
	
	for (int i = 0; i < snakeBody.size(); i++) {
		rect.x = snakeBody[i][0];
		rect.y = snakeBody[i][1];
		rect.w = 32;
		rect.h = 32;

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderFillRect(renderer, &rect);

		rect.x = (snakeBody[i][0]) + 4;
		rect.y = (snakeBody[i][1]) + 4;
		rect.w = 24;
		rect.h = 24;

		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(renderer, &rect);
	}

	SDL_Rect dest = { 0, 0, textSurface->w, textSurface->h };

	SDL_RenderCopy(renderer, textTexture, &dest, &dest);
	
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	
	SDL_RenderPresent(renderer);
}

bool events() {
	SDL_Event e;
	
	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_QUIT) {
			run = false;
		}
		else if (e.type == SDL_KEYDOWN) {
			switch (e.key.keysym.sym) {
			case SDLK_RIGHT:
				if (snake.direction != 3 && !snake.justTurned) {
					snake.direction = 1;
					snake.justTurned = true;
				}
				break;
			case SDLK_DOWN:
				if (snake.direction != 4 && !snake.justTurned) {
					snake.direction = 2;
					snake.justTurned = true;
				}
				break;
			case SDLK_LEFT:
				if (snake.direction != 1 && !snake.justTurned) {
					snake.direction = 3;
					snake.justTurned = true;
				}
				break;
			case SDLK_UP:
				if (snake.direction != 2 && !snake.justTurned) {
					snake.direction = 4;
					snake.justTurned = true;
				}
				break;
			case SDLK_d:
				if (snake.direction != 3 && !snake.justTurned) {
					snake.direction = 1;
					snake.justTurned = true;
				}
				break;
			case SDLK_s:
				if (snake.direction != 4 && !snake.justTurned) {
					snake.direction = 2;
					snake.justTurned = true;
				}
				break;
			case SDLK_a:
				if (snake.direction != 1 && !snake.justTurned) {
					snake.direction = 3;
					snake.justTurned = true;
				}
				break;
			case SDLK_w:
				if (snake.direction != 2 && !snake.justTurned) {
					snake.direction = 4;
					snake.justTurned = true;
				}
				break;
			}
		}
	}

	return true;
}

void snakeMove() {
	prevTime = currentTime;
	currentTime = SDL_GetTicks();
	deltaTime = (float)(currentTime - prevTime);

	if (snake.direction == 1) {;
		snake.absx += (float)(moveSpeed * (deltaTime / 1000));
	}
	else if (snake.direction == 2) {
		snake.absy += (float)(moveSpeed * (deltaTime / 1000));
	}
	else if (snake.direction == 3) {
		snake.absx -= (float)(moveSpeed * (deltaTime / 1000));
	}
	else if (snake.direction == 4) {
		snake.absy -= (float)(moveSpeed * (deltaTime / 1000));
	}
	
	snake.prevPos_x = snake.grid_x;
	snake.prevPos_y = snake.grid_y;
	
	snake.grid_x = round(snake.absx / 32) * 32;
	snake.grid_y = round(snake.absy / 32) * 32;

	snake.currentPos_x = snake.grid_x;
	snake.currentPos_y = snake.grid_y;

	snake.delta_pos_x = (float)(snake.currentPos_x - snake.prevPos_x);
	snake.delta_pos_y = (float)(snake.currentPos_y - snake.prevPos_y);

	if (snake.delta_pos_x != 0 || snake.delta_pos_y != 0) {
		snake.justTurned = false;
		snakeBody.pop_back();
		snakeBody.insert(snakeBody.begin(), { snake.grid_x, snake.grid_y }); 
	}
}

void collision() {
	if (snake.grid_x == apple.x && snake.grid_y == apple.y && appleHasMoved) {
		getRandomApplePos();
		snake.bodyLength++;
		score++;
		snakeBody.push_back({ 1000, 1000 });
	}

	if (snake.grid_x < 0 || snake.grid_x > 768) {
		run = false;
	}

	if (snake.grid_y < 0 || snake.grid_y > 672) {
		run = false;
	}

	for (int i = 1; i < snakeBody.size(); i++) {
		if (snake.grid_x == snakeBody[i][0] && snake.grid_y == snakeBody[i][1]) {
			run = false;
		}
	}
}

void close() {
	SDL_DestroyWindow(window);
	window = NULL;
	windowSurface = NULL;

	SDL_DestroyRenderer(renderer);
	renderer = NULL;

	SDL_FreeSurface(textSurface);
	textSurface = NULL;

	TTF_CloseFont(font);
	font = NULL;

	SDL_DestroyTexture(textTexture);
	textTexture = NULL;

	SDL_Quit();
	TTF_Quit();
}

int main(int argc, char* args[]) {
	if (!init()) {
		std::cout << "Could not initialize!" << std::endl;
	}
	if (!loadAssets()) {
		std::cout << "Could not load assets!" << std::endl;
	}
	else {
		snakeBody.push_back({ snake.grid_x, snake.grid_y });

		// random seed
		srand((unsigned int)time(NULL));

		getRandomApplePos();

		while (run) {
			SDL_Delay(10);
			events();
			draw();
			snakeMove();
			collision();
		}

		close();
	}

	return 0;
}