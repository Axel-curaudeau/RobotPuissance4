#pragma once

#include "opencv2/opencv.hpp"
#include "bitboard.hpp"
#include "ff/fflog.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include "cvimage.hpp"

namespace p4cam
{
	/// \brief Detect the bitboard from an image
	///
	/// \param _image: The image to attempt detection from
	/// \param _result: RETURN VALUE the board detected from the image (WARNING: only use if this function returns true)
	/// \param _debugImage: RETURN VALUE the debug image (always valid, even when the function fails by returning false)
	/// 
	/// \return True if the board detection was succesful and _result is valid, false otherwise
	bool getBoard(const cvimage& _image, bitboard& _result, cvimage& _debugImage);


	/// \brief Groups circles by their radius and returns a group with very similar radiuses
	/// 
	/// \param _circles: The initial list of circles
	/// \param _ratio: [0.0, 1.0] 0.0 for the radius to be EXACTLY the same to be grouped together, 1.0 for the radius to be 100% bigger or smaller (twice as big)
	/// 
	/// \return The list circles that passed the filter
	ff::dynarray<ff::circlef> filterCirclesByRadius(ff::dynarray<ff::circlef> _circles, float _ratio);

	/// \brief Groups circles into 42 groups (7x6) based on their x and y coordinates
	///
	/// \param _circles: The initial or filtered list of circles
	/// \param _ratio: [0.0, 1.0] 0.0 requires the x/y coordinates to be EXACTLY the same, 1.0 will group x/y coordinates together if they are within the radius of the circle, 0.5 will group x/y coordinates together if they are within half the radius of the circle
	/// \param _xAverages: RETURN VALUE the average value for each x (vertical) grouping
	/// \param _yAverages: RETURN VALUE the average value for each y (horizontal) grouping
	/// 
	/// \return The circles groups (for example result[0][2] will return the circle that is both in the 1st vertical group and 3rd horizontal group)
	ff::dynarray<ff::dynarray<ff::circlef>> groupCirclesByPosition(ff::dynarray<ff::circlef> _circles, float _ratio, ff::dynarray<float>& _xAverages, ff::dynarray<float>& _yAverages);

	/// \brief Get a connect 4 board from an image and 7x6 image positions
	/// 
	/// \param _image: The image to sample
	/// \param _samplePos: A 7x6 list of all positions to sample in the image
	///
	/// \return The board built by sampling
	bitboard getSampledBoard(cvimage _image, ff::dynarray<ff::dynarray<ff::vec2i>> _samplePos);

	hsv rgb2hsv(rgb in);



}


bool p4cam::getBoard(const cvimage& _image, bitboard& _result, cvimage& _debugImage)
{
	ff::dynarray<ff::circlef> circles = _image.detectCircles();												 //
	_debugImage = _image;																					 // (<- reset the debug image to the starting image)
	for (uint i = 0; i < circles.size(); i += 1) { _debugImage.drawCircle(circles[i], ff::color::black()); } //
	if (circles.size() < 7) { return false; }																 // Detect the circles from the image

	
	_debugImage = _image;																					 // (<- reset the debug image to the starting image)
	circles = filterCirclesByRadius(circles, 0.1f);															 //
	for (uint i = 0; i < circles.size(); i += 1) { _debugImage.drawCircle(circles[i], ff::color::green()); } // Filter circles by radius


	ff::dynarray<float> xAverages; ff::dynarray<float> yAverages;																							   // (<- obtain the averages of each group)
	ff::dynarray<ff::dynarray<ff::circlef>> circleGroups = groupCirclesByPosition(circles, 0.6f, xAverages, yAverages);										   // Group the circles by position
	for (uint i = 0; i < xAverages.size(); i += 1) { _debugImage.drawLine(ff::linef(ff::vec2f(xAverages[i], 0), ff::vec2f(0, 1.0f)), ff::color::darkCyan()); } //
	for (uint i = 0; i < yAverages.size(); i += 1) { _debugImage.drawLine(ff::linef(ff::vec2f(0, yAverages[i]), ff::vec2f(1, 0.0f)), ff::color::darkCyan()); } //
	if (circleGroups.size() != 7) { return false; }																											   // (<- fail if there aren't 7 vertical groups)
	if (circleGroups[0].size() != 6) { return false; }																										   // (<- fail if there aren't 6 horizontal groups)


	ff::dynarray<ff::dynarray<ff::vec2i>> samplePos;																							   //
	samplePos.resize(7);																														   //
	for (uint i = 0; i < 7; i += 1) { samplePos[i].resize(6); }																					   //
	for (uint i = 0; i < 7; i += 1) { for (uint j = 0; j < 6; j += 1) { samplePos[i][j] = ff::vec2i((int)xAverages[i], (int)yAverages[5 - j]); } } // Generate the image sampling positions from the group averages






	_result = getSampledBoard(_image, samplePos);																												 // (<- sample the board)
	for (uint i = 0; i < 7; i += 1)																																 //
	{																																							 //
		for (uint j = 0; j < 6; j += 1)																															 //
		{																																						 //
			if (_result.getCellType(i, j) == nBoardSlot::firstPlayer) { _debugImage.drawCircle(ff::circlef(samplePos[i][j], 5.0f), ff::color::red()); }			 //
			else if (_result.getCellType(i, j) == nBoardSlot::secondPlayer) { _debugImage.drawCircle(ff::circlef(samplePos[i][j], 5.0f), ff::color::yellow()); } //
			else { _debugImage.drawCircle(ff::circlef(samplePos[i][j], 5.0f), ff::color::white()); }															 //
		}																																						 //
	}																																							 // Draw the final result in the debug image

	return true;
}
ff::dynarray<ff::circlef> p4cam::filterCirclesByRadius(ff::dynarray<ff::circlef> _circles, float _ratio)
{
	if (_circles.size() <= 0) { return _circles; } // If there are no circles, there is nothing to do

	ff::dynarray<ff::dynarray<ff::circlef>> circleGroups;																	   //
	for (uint i = 0; i < _circles.size(); i += 1)																			   //
	{																														   //
		circleGroups.pushback(ff::dynarray<ff::circlef>());																	   //
																															   //
		float diff = _circles[i].radius * _ratio;																			   //
		float maxRadius = _circles[i].radius + diff;																		   //
		float minRadius = _circles[i].radius - diff;																		   //
		for (uint j = 0; j < _circles.size(); j += 1)																		   //
		{																													   //
			if (_circles[j].radius <= maxRadius && _circles[j].radius >= minRadius) { circleGroups[i].pushback(_circles[j]); } //
		}																													   //
	}																														   // Make radius groups

	uint idxOfMax = 0;																													//
	for (uint i = 1; i < circleGroups.size(); i += 1) { if (circleGroups[i].size() > circleGroups[idxOfMax].size()) { idxOfMax = i; } } //
																																		//
	return circleGroups[idxOfMax];																										// Find the group with the most elements and return it
}
ff::dynarray<ff::dynarray<ff::circlef>> p4cam::groupCirclesByPosition(ff::dynarray<ff::circlef> _circles, float _ratio, ff::dynarray<float>& _xAverages, ff::dynarray<float>& _yAverages)
{
	float averageRadius = 0;															   //
	for (uint i = 0; i < _circles.size(); i += 1) { averageRadius += _circles[i].radius; } //
	averageRadius /= _circles.size();													   // Calculate average radius


	// Group circles by X position, and by Y position (a circle is part of the group if its center projection is within a certain range)
	ff::dynarray<ff::dynarray<uint>> xGroupIdxs;
	ff::dynarray<ff::dynarray<uint>> yGroupIdxs;
	for (uint i = 0; i < _circles.size(); i += 1)
	{
		xGroupIdxs.pushback(ff::dynarray<uint>());
		yGroupIdxs.pushback(ff::dynarray<uint>());

		float baseXPosition = _circles[i].center.x;
		float baseYPosition = _circles[i].center.y;
		for (uint j = 0; j < _circles.size(); j += 1)
		{
			if (_circles[j].center.x >= baseXPosition - averageRadius * _ratio && _circles[j].center.x <= baseXPosition + averageRadius * _ratio) { xGroupIdxs[i].pushback(j); }
			if (_circles[j].center.y >= baseYPosition - averageRadius * _ratio && _circles[j].center.y <= baseYPosition + averageRadius * _ratio) { yGroupIdxs[i].pushback(j); }
		}
	}



	// Add final groups in a way where there are no circles repeated in different groups
	ff::dynarray<bool> xCircleAdded;
	ff::dynarray<bool> yCircleAdded;
	for (uint i = 0; i < _circles.size(); i += 1) { xCircleAdded.pushback(false); yCircleAdded.pushback(false); }

	ff::dynarray<ff::dynarray<uint>> xGroups;
	ff::dynarray<float> xAverage;
	ff::dynarray<ff::dynarray<uint>> yGroups;
	ff::dynarray<float> yAverage;
	for (uint i = 0; i < _circles.size(); i += 1)
	{
		// Check if the circles <i> has already been added in a group over x
		bool addGroupX = true;
		for (uint j = 0; j < xGroupIdxs[i].size(); j += 1) { if (xCircleAdded[xGroupIdxs[i][j]]) { addGroupX = false; } }

		if (addGroupX)
		{
			xGroups.pushback(xGroupIdxs[i]);
			float xSum = 0;
			for (uint j = 0; j < xGroupIdxs[i].size(); j += 1) { xCircleAdded[xGroupIdxs[i][j]] = true; xSum += _circles[xGroupIdxs[i][j]].center.x; }
			xAverage.pushback(xSum / xGroupIdxs[i].size());
		}
	}
	for (uint i = 0; i < _circles.size(); i += 1)
	{
		// Check if the circles <i> has already been added in a group over y
		bool addGroupY = true;
		for (uint j = 0; j < yGroupIdxs[i].size(); j += 1) { if (yCircleAdded[yGroupIdxs[i][j]]) { addGroupY = false; } }

		if (addGroupY)
		{
			yGroups.pushback(yGroupIdxs[i]);
			float ySum = 0;
			for (uint j = 0; j < yGroupIdxs[i].size(); j += 1) { yCircleAdded[yGroupIdxs[i][j]] = true; ySum += _circles[yGroupIdxs[i][j]].center.y; }
			yAverage.pushback(ySum / yGroupIdxs[i].size());
		}
	}

	// Sort the groups by average
	ff::mapdynarray<uint, uint> sortMapX = xAverage.sort();
	xGroups.sort(sortMapX);
	ff::mapdynarray<uint, uint> sortMapY = yAverage.sort();
	yGroups.sort(sortMapY);


	ff::dynarray<ff::dynarray<ff::circlef>> result;
	result.resize(xGroups.size());
	for (uint i = 0; i < result.size(); i += 1) { result[i].resize(yGroups.size()); }

	for (uint i = 0; i < _circles.size(); i += 1)
	{
		uint xGroupIdx;
		uint yGroupIdx;

		// Find in which x group the circle <i> is
		for (uint j = 0; j < xGroups.size(); j += 1) { if (xGroups[j].find(i)) { xGroupIdx = j; } }
		// Find in which y group the circle <i> is
		for (uint j = 0; j < yGroups.size(); j += 1) { if (yGroups[j].find(i)) { yGroupIdx = j; } }

		result[xGroupIdx][yGroupIdx] = _circles[i];
	}

	_xAverages = xAverage;
	_yAverages = yAverage;

	return result;
}
bitboard p4cam::getSampledBoard(cvimage _image, ff::dynarray<ff::dynarray<ff::vec2i>> _samplePos)
{
	if (_samplePos.size() != 7) { return bitboard(); }										  //
	for (uint i = 0; i < 7; i += 1) { if (_samplePos[i].size() != 6) { return bitboard(); } } // Make sure the number of samples is correct (7x6)
	

	bitboard result = bitboard();
	for (uint i = 0; i < 7; i += 1)
	{
		for (uint j = 0; j < 6; j += 1)
		{
			int x = _samplePos[i][j].x;
			int y = _samplePos[i][j].y;
			uint b = _image.img.data[(y * _image.size().x + x) * 3 + 0]; // b
			uint g = _image.img.data[(y * _image.size().x + x) * 3 + 1]; // g
			uint r = _image.img.data[(y * _image.size().x + x) * 3 + 2]; // r
			//EXPERIMENTAL
			rgb color;
			color.r = (r);
			color.g = (g);
			color.b = (b);
			color.r /= 255;
			color.g /= 255;
			color.b /= 255;

			hsv hsvColor;
			hsvColor = rgb2hsv(color);

			//Select Color RED
			if (hsvColor.h <=30 &&hsvColor.s>=0.40 && hsvColor.v>=0.40 &&hsvColor.h>=0|| hsvColor.h <= 360&& hsvColor.h >=330 && hsvColor.s >= 0.40 && hsvColor.v >= 0.40) { result.setCellType(i, j, nBoardSlot::firstPlayer); }
			else if (hsvColor.h <= 80 &&hsvColor.h >= 40 && hsvColor.s >= 0.40 && hsvColor.v >= 0.40) { result.setCellType(i, j, nBoardSlot::secondPlayer); }
			else { result.setCellType(i, j, nBoardSlot::empty); }

			/*
			if (r >= 150 && g < 128 && b < 80) { result.setCellType(i, j, nBoardSlot::firstPlayer); }
			else if (r >= 180 && g >= 180 && b < 60) { result.setCellType(i, j, nBoardSlot::secondPlayer); }
			else { result.setCellType(i, j, nBoardSlot::empty); }
			*/
		}
	}

	return result;
}

hsv p4cam::rgb2hsv(rgb in)
{
	hsv         out;
	double      min, max, delta;

	min = in.r < in.g ? in.r : in.g;
	min = min < in.b ? min : in.b;

	max = in.r > in.g ? in.r : in.g;
	max = max > in.b ? max : in.b;

	out.v = max;                                // v
	delta = max - min;
	if (delta < 0.00001)
	{
		out.s = 0;
		out.h = 0; 
		return out;
	}
	if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
		out.s = (delta / max);                  // s
	}
	else {
		// if max is 0, then r = g = b = 0              
		// s = 0, h is undefined
		out.s = 0.0;
		out.h = NAN;                            // its now undefined
		return out;
	}
	if (in.r >= max)                           // > is bogus, just keeps compilor happy
		out.h = (in.g - in.b) / delta;        // between yellow & magenta
	else
		if (in.g >= max)
			out.h = 2.0 + (in.b - in.r) / delta;  // between cyan & yellow
		else
			out.h = 4.0 + (in.r - in.g) / delta;  // between magenta & cyan

	out.h *= 60.0;                              // degrees

	if (out.h < 0.0)
		out.h += 360.0;

	//std::cout<<"HSV:" << "h " << out.h <<" s " << out.s <<"v " << out.v << "\n";
	return out;
}









// OLD CODE (kept if you want to improve computer vision)

// Reduces the amount of color values in the image
/*cv::Mat bitCrushImage(const cv::Mat& _image)
{
	cv::Mat result;
	cv::convertScaleAbs(_image, result, 1.0 / 64, 0.0);
	cv::convertScaleAbs(result, result, 64, 0.0);
	return result;
}*/

// EXPERIMENTAL
/*float hsvDistance(uint8 _h1, uint8 _s1, uint8 _v1, uint8 _h2, uint8 _s2, uint8 _v2)
{
	float chroma1 = (_s1 * _v1) / (255.0f * 255.0f);
	float chroma2 = (_s2 * _v2) / (255.0f * 255.0f);
	float chromaDistance = ff::abs(chroma2 - chroma1);


	float hueDistance = ff::minOf(ff::abs(_h2 - _h1), 255 - ff::abs(_h1 - _h2)) / 255.0f;
	float saturationDistance = ff::abs(_s2 - _s1) / 255.0f;
	float valueDistance = ff::abs(_v2 - _v1) / 255.0f;

	//float h1 = _h1 / 255.0f;
	//float s1 = _s1 / 255.0f;
	//float v1 = _v1 / 255.0f;
	//float h2 = _h2 / 255.0f;
	//float s2 = _s2 / 255.0f;
	//float v2 = _v2 / 255.0f;

	//float a = (float)((v2 - v1) * (v2 - v1));
	//float b = (float)(s1 * s1 * v1 * v1);
	//float c = (float)(s2 * s2 * v2 * v2);
	//float d = 2 * s1 * s2 * v1 * v2 * std::cosf(ff::minOf(ff::abs(h2 - h1), 1.0f - ff::abs(h1 - h2)) * 3.1416f);
	//return std::sqrtf(a + b + c - d);

	return (hueDistance * chromaDistance) + (saturationDistance * 0.5f + valueDistance * 0.5f) * (1.0f - chromaDistance);
}*/