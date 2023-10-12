
#pragma once

#include <iostream>

#include <SFML/Graphics.hpp>
#include "opencv2/opencv.hpp"

#include "ff/ffdynarray.hpp"
#include "ff/ffcircle.hpp"
#include "ff/fftime.hpp"
#include "ff/ffcolor.hpp"
#include "ff/ffline.hpp"





/// \brief OpenCV image representation
/// \detail Only represents BGR (almost like rgb) images
struct cvimage
{
	enum class nWebcam { disconnected, fpsLimited, ok };


	cv::Mat img;


	cvimage();
	cvimage(const cvimage& _other);
	cvimage& operator=(const cvimage& _other);





	/// \brief Get the size of the current image
	ff::vec2i size() const;

	/// \brief Load the latest webcam image into this cvimage instance
	/// \param _cameraIdx: the index of the camera
	/// \param _cameraFps: set this number to the camera's fps, or lower
	/// \return "ok" if a frame was obtained, "disconnected"/"fpsLimited" if not
	nWebcam getWebcamImage(uint _cameraIdx, uint _cameraFps = 30);

	/// \brief Resize the current image
	/// \param _newSize: size to resize the image to
	void resize(ff::vec2i _newSize);

	/// \brief Blur the current image
	/// \param _size: How many surrounding pixels to blur
	void blur(uint _size);

	/// \brief Brighten/darken the current image
	/// \param _brightness: [-1.0, 1.0], 1.0 makes the image fully white, -1.0 fully black
	void changeBrightness(double _brightness);

	/// \brief Contrasts the image (multiplication of each pixel by this value)
	/// \param _contrast: [0.0, +]
	void changeContrast(double _contrast);

	/// \brief Average the current image with another image
	/// \param _previousImg: the other base image to average with
	/// \param _weight: [0.0, 1.0], 0.0 will keep previous image only, 1.0 will keep current image only, 0.5 will average them both equally, 0.2 will keep 20% of the current image and 80% of the previous image
	void denoiseTemporal(cvimage _previousImg, double _weight = 0.2f);

	/// \brief Get a list of all detected circles in the image
	ff::dynarray<ff::circlef> detectCircles() const;

	/// \brief Draw a pixel on the current image
	void drawPixel(ff::vec2i _pos, ff::color _color = ff::color::lightGray());

	/// \brief Draw a circle on the current image (kind of)
	void drawCircle(ff::circlef _circle, ff::color _color = ff::color::lightGray());

	/// \brief Draw a line on the current image
	void drawLine(ff::linef _line, ff::color _color = ff::color::lightGray());

	/// \brief Draw the current image to a texture and obtain its corresponding sprite (warning: the sprite references the texture)
	sf::Sprite drawToTexture(sf::Texture& _texture);

	/// \brief Save the current image to a file
	void save(ff::string _filename);

	/// \brief Load an image file
	void load(ff::string _filename);
};









cvimage::cvimage() {}
cvimage::cvimage(const cvimage& _other) { img = _other.img.clone(); }
cvimage& cvimage::operator=(const cvimage& _other) { img = _other.img.clone(); return *this; }
ff::vec2i cvimage::size() const { ff::vec2i result; result.x = img.cols; result.y = img.rows; return result; }
cvimage::nWebcam cvimage::getWebcamImage(uint _cameraIdx, uint _cameraFps)
{
	static cv::VideoCapture capture;
	static ff::timer fpsLimiter;

	// The VideoCapture::read function is BLOCKING, i.e. if a frame is not available but the camera is connected it will WAIT for a new frame (and there is no way to make it non-blocking)
	// Additionally, frames are not accumulated, i.e. if the camera provides 30fps and you check at 2fps you will be skipping 28 frames per second
	// This is why this function has a target fps counter, to return instantly if you check too early
	// Make sure to set cameraFps at an FPS equal or lower than the camera's FPS if you do not want the function to block (also, do not try using VideoCapture::get to retrieve the fps, it does not work)
	if (!fpsLimiter.tickEveryMilli(1000 / _cameraFps)) { return nWebcam::fpsLimited; }

	if (!capture.isOpened()) { if (!capture.open(_cameraIdx)) { return nWebcam::disconnected; } }

	// Get the frame
	cv::Mat newImg;
	if (!capture.read(newImg)) { capture.release(); return nWebcam::disconnected; }

	img = newImg;

	return nWebcam::ok;
}
void cvimage::resize(ff::vec2i _newSize)
{
	cv::resize(img, img, cv::Size(_newSize.x, _newSize.y), cv::INTER_LINEAR);
}
void cvimage::blur(uint _size)
{
	cv::blur(img, img, cv::Size(_size, _size));
}
void cvimage::changeBrightness(double _brightness)
{
	img.convertTo(img, -1, 1.0, 255 * _brightness);
}
void cvimage::changeContrast(double _contrast)
{
	img.convertTo(img, -1, _contrast, 0.0);
}
void cvimage::denoiseTemporal(cvimage _previousImg, double _weight)
{
	cv::addWeighted(img, _weight, _previousImg.img, 1.0 - _weight, 0, img);
}
ff::dynarray<ff::circlef> cvimage::detectCircles() const
{
	cv::Mat grayImg;
	cv::cvtColor(img, grayImg, cv::COLOR_BGR2GRAY);


	std::vector<cv::Vec3f> circles;
	cv::HoughCircles(grayImg, circles, cv::HOUGH_GRADIENT, 0.5, size().x / 12, 200, 30.0, size().x / 24, size().x / 6);


	ff::dynarray<ff::circlef> result;
	for (uint i = 0; i < circles.size(); i += 1) { result.pushback(ff::circlef(circles[i][0], circles[i][1], circles[i][2])); }

	return result;
}
void cvimage::drawPixel(ff::vec2i _pos, ff::color _color)
{
	int xSize = img.cols;
	int ySize = img.rows;
	if (_pos.x < 0 || _pos.x >= xSize) { return; }
	if (_pos.y < 0 || _pos.y >= ySize) { return; }

	img.data[(_pos.y * xSize + _pos.x) * 3 + 0] = _color.b; // b
	img.data[(_pos.y * xSize + _pos.x) * 3 + 1] = _color.g; // g
	img.data[(_pos.y * xSize + _pos.x) * 3 + 2] = _color.r; // r
}
void cvimage::drawCircle(ff::circlef _circle, ff::color _color)
{
	// Draw a square for the center:
	for (int i = (int)_circle.center.x - 6; i <= (int)_circle.center.x + 6; i += 1)
	{
		for (int j = (int)_circle.center.y - 6; j <= (int)_circle.center.y + 6; j += 1)
		{
			drawPixel(ff::vec2i(i, j), _color);
		}
	}

	// Draw a horizontal line:
	for (int i = (int)_circle.center.x - (int)_circle.radius; i <= (int)_circle.center.x + (int)_circle.radius; i += 1)
	{
		drawPixel(ff::vec2i(i, (int)_circle.center.y), _color);
	}

	// Draw vertical lines on each side of the horizontal line:
	for (int j = (int)_circle.center.y - 4; j <= (int)_circle.center.y + 4; j += 1)
	{
		drawPixel(ff::vec2i((int)_circle.center.x - (int)_circle.radius, j), _color);
		drawPixel(ff::vec2i((int)_circle.center.x + (int)_circle.radius, j), _color);
	}
}
void cvimage::drawLine(ff::linef _line, ff::color _color)
{
	if (ff::abs(_line.direction.x) > ff::abs(_line.direction.y))
	{
		ff::vec2f pos0 = _line.point - _line.direction * (_line.point.x / _line.direction.x);
		float y0Pos = pos0.y;

		ff::vec2f direction = _line.direction;
		if (direction.x < 0) { direction = -direction; }

		ff::vec2f fitDirection = _line.direction * (size().x / _line.direction.x);

		for (int i = 0; i < size().x; i += 1)
		{
			drawPixel(ff::vec2i(i, (int)y0Pos + (int)fitDirection.y * i / size().x), _color);
		}
	}
	else
	{
		ff::vec2f pos0 = _line.point - _line.direction * (_line.point.y / _line.direction.y);
		float x0Pos = pos0.x;

		ff::vec2f direction = _line.direction;
		if (direction.y < 0) { direction = -direction; }

		ff::vec2f fitDirection = _line.direction * (size().y / _line.direction.y);

		for (int j = 0; j < size().y; j += 1)
		{
			drawPixel(ff::vec2i((int)x0Pos + (int)fitDirection.x * j / size().y, j), _color);
		}
	}
}
sf::Sprite cvimage::drawToTexture(sf::Texture& _texture)
{
	if (_texture.getSize().x != size().x || _texture.getSize().y != size().y) { _texture.create(size().x, size().y); }


	// Make the raw buffer and the texture the correct size
	ff::dynarray<uint8> pixelData;
	pixelData.resize(size().x * size().y * 4);

	sf::Sprite spriteResult = sf::Sprite(_texture);

	// Update the raw buffer with the new frame
	for (int i = 0; i < size().x; i += 1)
	{
		for (int j = 0; j < size().y; j += 1)
		{
			pixelData[(j * size().x + i) * 4 + 2] = img.data[(j * size().x + i) * 3 + 0]; // b
			pixelData[(j * size().x + i) * 4 + 1] = img.data[(j * size().x + i) * 3 + 1]; // g
			pixelData[(j * size().x + i) * 4 + 0] = img.data[(j * size().x + i) * 3 + 2]; // r
			pixelData[(j * size().x + i) * 4 + 3] = 255; // a
		}
	}

	// Update the texture
	_texture.update(pixelData.data());

	return spriteResult;
}
void cvimage::save(ff::string _filename)
{
	sf::Texture texture;
	drawToTexture(texture);

	texture.copyToImage().saveToFile(_filename);
}
void cvimage::load(ff::string _filename)
{
	sf::Image sfmlImage;
	sfmlImage.loadFromFile(_filename);

	img.create(cv::Size(sfmlImage.getSize().x, sfmlImage.getSize().y), CV_8UC3);
	for (int i = 0; i < size().x; i += 1)
	{
		for (int j = 0; j < size().y; j += 1)
		{
			img.data[(j * size().x + i) * 3 + 0] = sfmlImage.getPixelsPtr()[(j * size().x + i) * 4 + 2]; // b
			img.data[(j * size().x + i) * 3 + 1] = sfmlImage.getPixelsPtr()[(j * size().x + i) * 4 + 1]; // g
			img.data[(j * size().x + i) * 3 + 2] = sfmlImage.getPixelsPtr()[(j * size().x + i) * 4 + 0]; // r
		}
	}
}