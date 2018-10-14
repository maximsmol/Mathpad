#include <cstdint>
#include <cassert>

#include <SLY.hpp>
#include <SDL2_gfxPrimitives.h>

enum TOOL
{
	TOOL_FREEDRAW,
	TOOL_LINE,
	TOOL_RECT,
	TOOL_FILLEDRECT,
	TOOL_CIRCLE,
	TOOL_FILLEDCIRCLE,
	TOOL_ELLIPSE,
	TOOL_FILLEDELLIPSE,
	TOOL_ERASER
};

struct Button
{
	sly::image::Image img;
	int x, y;

	void render()
	{
		img.render(x, y);
	}

	bool intersects(int posX, int posY)
	{
		SDL_Rect imgSize = img.getSourceRect();
		return (x < posX && posX < x + imgSize.w) &&
			   (y < posY && posY < y + imgSize.h);
	}
};

struct Slider
{
	int x, y;
	int w, h;
	int minValue, maxValue;
	int value;

	void render()
	{
		thickLineRGBA(sly::base::renderer, x, y+h/2, x+w, y+h/2, 2, 0, 0, 0, 255);

		int start = x+(value-minValue)*(w/(maxValue-minValue));
		roundedBoxRGBA(sly::base::renderer, start, y, start+12, y+h, 3, 0, 0, 0, 255);
	}

	bool updateValue(int cursorX, int cursorY, bool isPickedup)
	{
		if (!isPickedup && (cursorX < x || x + w < cursorX ||
			cursorY < y || y + h < cursorY)) return false;

		value = minValue+((cursorX - x)*(maxValue-minValue))/w;
		if (value > maxValue) value = maxValue;
		else if (value < minValue) value = minValue;

		return true;
	}
};

int winW = 1440;
int winH = 850;


using namespace sly::image;
int main()
{
	sly::base::init("MathPad", winW, winH);

	SDL_Texture* image = SDL_CreateTexture(
					sly::base::renderer, SDL_PIXELFORMAT_ARGB8888,
					SDL_TEXTUREACCESS_TARGET,
					winW, winH);
	assert(image != nullptr);

	Button minus = {Image("res/minus.png"), 10, 29};
	Button plus = {Image("res/plus.png"), 155, 29};
	Slider sizeSlider = {45, 10, 100, 64, 1, 20, 5};

	Button brushBtn = {Image("res/brush.png"), 24, 84};
	Button lineBtn = {Image("res/line.png"), 112, 84};
	Button rectBtn = {Image("res/rect.png"), 24, 156};
	Button frectBtn = {Image("res/frect.png"), 112, 156};
	Button circle = {Image("res/circle.png"), 24, 232};
	Button fcircle = {Image("res/fcircle.png"), 112, 232};
	Button ellipse = {Image("res/ellipse.png"), 24, 306};
	Button fellipse = {Image("res/fellipse.png"), 112, 306};
	Button eraserBtn = {Image("res/eraser.png"), 24, 380};

	Button clearBtn = {Image("res/clear.png"), 10, winH-74};
	Button quitBtn = {Image("res/quit.png"), 112, winH-74};

	SDL_SetRenderTarget(sly::base::renderer, image);
	SDL_RenderClear(sly::base::renderer);
	SDL_SetRenderTarget(sly::base::renderer, nullptr);

	Sint16 pressX = 0;
	Sint16 pressY = 0;
	Sint16 lastX = 0;
	Sint16 lastY = 0;
	Sint16 curX = 0;
	Sint16 curY = 0;
	bool pressed = false;
	bool doAction = false;
	bool sliderPicked = false;

	Uint8 size = 5;

	TOOL tool = TOOL_FREEDRAW;

	bool running = true;
	SDL_Event e({});
	while (running)
	{
		SDL_RenderClear(sly::base::renderer);
		SDL_RenderCopy(sly::base::renderer, image, nullptr, nullptr);

		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT) running = false;
			else if (e.type == SDL_MOUSEBUTTONDOWN)
			{
				if (e.button.button == SDL_BUTTON_LEFT)
				{
					pressX = static_cast<Sint16>(e.button.x);
					pressY = static_cast<Sint16>(e.button.y);

					if (minus.intersects(pressX, pressY))
					{
						size--;
						sizeSlider.value = size;
					}
					else if (plus.intersects(pressX, pressY))
					{
						size++;
						sizeSlider.value = size;
					}

					else if (brushBtn.intersects(pressX, pressY)) tool = TOOL_FREEDRAW;
					else if (lineBtn.intersects(pressX, pressY)) tool = TOOL_LINE;
					else if (rectBtn.intersects(pressX, pressY)) tool = TOOL_RECT;
					else if (frectBtn.intersects(pressX, pressY)) tool = TOOL_FILLEDRECT;
					else if (circle.intersects(pressX, pressY)) tool = TOOL_CIRCLE;
					else if (fcircle.intersects(pressX, pressY)) tool = TOOL_FILLEDCIRCLE;
					else if (ellipse.intersects(pressX, pressY)) tool = TOOL_ELLIPSE;
					else if (fellipse.intersects(pressX, pressY)) tool = TOOL_FILLEDCIRCLE;
					else if (eraserBtn.intersects(pressX, pressY)) tool = TOOL_ERASER;

					else if (quitBtn.intersects(pressX, pressY)) running = false;

					sliderPicked = sizeSlider.updateValue(curX, curY, sliderPicked);
					pressed = true;
				}
			}
			else if (e.type == SDL_MOUSEBUTTONUP)
			{
				if (e.button.button == SDL_BUTTON_LEFT)
				{
					pressed = false;
					if (!sliderPicked) doAction = true;
					sliderPicked = false;

					curX = static_cast<Sint16>(e.button.x);
					curY = static_cast<Sint16>(e.button.y);
				}
			}
			else if (e.type == SDL_MOUSEMOTION)
			{
				lastX = curX;
				lastY = curY;
				curX = static_cast<Sint16>(e.motion.x);
				curY = static_cast<Sint16>(e.motion.y);

				if (sliderPicked)
				{
					sizeSlider.updateValue(curX, curY, sliderPicked);
					size = sizeSlider.value;
				}
			}
		}
		if (size < 1) size = 1;
		else if (size > 20) size = 20;

		if (pressed && !sliderPicked)
		{
			if (tool == TOOL_LINE)
				thickLineRGBA(sly::base::renderer, pressX, pressY, curX, curY, size, 255, 255, 255, 255);
			if (tool == TOOL_CIRCLE)
				aacircleRGBA(sly::base::renderer, pressX, pressY, sqrt((curX-pressX)*(curX-pressX)+(curY-pressY)*(curY-pressY)), 255, 255, 255, 255);
			if (tool == TOOL_FILLEDCIRCLE)
				filledCircleRGBA(sly::base::renderer, pressX, pressY, sqrt((curX-pressX)*(curX-pressX)+(curY-pressY)*(curY-pressY)), 255, 255, 255, 255);
			if (tool == TOOL_ELLIPSE)
				aaellipseRGBA(sly::base::renderer, pressX, pressY, abs(curX-pressX), abs(curY-pressY), 255, 255, 255, 255);
			if (tool == TOOL_FILLEDELLIPSE)
				filledEllipseRGBA(sly::base::renderer, pressX, pressY, abs(curX-pressX), abs(curY-pressY), 255, 255, 255, 255);
			if (tool == TOOL_RECT)
			{
				thickLineRGBA(sly::base::renderer, pressX, pressY, curX, pressY, size, 255, 0, 0, 255);
				thickLineRGBA(sly::base::renderer, pressX, pressY, pressX, curY, size, 255, 0, 0, 255);
				thickLineRGBA(sly::base::renderer, pressX, curY, curX, curY, size, 255, 0, 0, 255);
				thickLineRGBA(sly::base::renderer, curX, pressY, curX, curY, size, 255, 0, 0, 255);
			}
			if (tool == TOOL_FILLEDRECT)
				boxRGBA(sly::base::renderer, pressX, pressY, curX, curY, 255, 255, 255, 255);
		}

		SDL_SetRenderTarget(sly::base::renderer, image);
		if (pressed && !sliderPicked)
		{
			if (clearBtn.intersects(pressX, pressY))
			{
				SDL_SetRenderDrawColor(sly::base::renderer, 0, 0, 0, 255);
				SDL_RenderClear(sly::base::renderer);
			}
			if (tool == TOOL_FREEDRAW)
			{
				filledCircleRGBA(sly::base::renderer, lastX, lastY, size/2, 255, 0, 0, 255);
				thickLineRGBA(sly::base::renderer, lastX, lastY, curX, curY, size, 255, 0, 0, 255);
				filledCircleRGBA(sly::base::renderer, curX, curY, size/2, 255, 0, 0, 255);
			}
			if (tool == TOOL_ERASER)
			{
				filledCircleRGBA(sly::base::renderer, lastX, lastY, size/2, 0, 0, 0, 255);
				thickLineRGBA(sly::base::renderer, lastX, lastY, curX, curY, size, 0, 0, 0, 255);
				filledCircleRGBA(sly::base::renderer, curX, curY, size/2, 0, 0, 0, 255);
			}
		}
		if (doAction)
		{
			if (tool == TOOL_LINE)
				thickLineRGBA(sly::base::renderer, pressX, pressY, curX, curY, size, 255, 0, 0, 255);
			if (tool == TOOL_CIRCLE)
				aacircleRGBA(sly::base::renderer, pressX, pressY, sqrt((curX-pressX)*(curX-pressX)+(curY-pressY)*(curY-pressY)), 255, 0, 0, 255);
			if (tool == TOOL_FILLEDCIRCLE)
				filledCircleRGBA(sly::base::renderer, pressX, pressY, sqrt((curX-pressX)*(curX-pressX)+(curY-pressY)*(curY-pressY)), 255, 0, 0, 255);
			if (tool == TOOL_ELLIPSE)
				aaellipseRGBA(sly::base::renderer, pressX, pressY, abs(curX-pressX), abs(curY-pressY), 255, 0, 0, 255);
			if (tool == TOOL_FILLEDELLIPSE)
				filledEllipseRGBA(sly::base::renderer, pressX, pressY, abs(curX-pressX), abs(curY-pressY), 255, 0, 0, 255);
			if (tool == TOOL_RECT)
			{
				thickLineRGBA(sly::base::renderer, pressX, pressY, curX, pressY, size, 255, 0, 0, 255);
				thickLineRGBA(sly::base::renderer, pressX, pressY, pressX, curY, size, 255, 0, 0, 255);
				thickLineRGBA(sly::base::renderer, pressX, curY, curX, curY, size, 255, 0, 0, 255);
				thickLineRGBA(sly::base::renderer, curX, pressY, curX, curY, size, 255, 0, 0, 255);
			}
			if (tool == TOOL_FILLEDRECT)
				boxRGBA(sly::base::renderer, pressX, pressY, curX, curY, 255, 0, 0, 255);
			doAction = false;
		}
		SDL_SetRenderTarget(sly::base::renderer, nullptr);

		boxRGBA(sly::base::renderer, 0, 0, 200, winH, 255, 255, 255, 255);
		minus.render();
		plus.render();
		sizeSlider.render();

		brushBtn.render();
		lineBtn.render();
		rectBtn.render();
		frectBtn.render();
		circle.render();
		fcircle.render();
		ellipse.render();
		fellipse.render();
		eraserBtn.render();

		clearBtn.render();
		quitBtn.render();

		SDL_RenderPresent(sly::base::renderer);
	}

	return 0;
}
