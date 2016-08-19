#pragma once
#include <string>
#include <math.h>
namespace CL
{
	unsigned long long nearest_Pow(unsigned long long aNum);
	int Color32Reverse(int x);
	int MoveToRed(int x);
	int MoveToGreen(int x);


	/*
		Drag Coefficient
		Sphere = 0.47
		Half-Sphere = 0.42
		Cone = 0.50
		Cube = 1.05
		Angled Cube (45 deg?) 0.80
		Long Cylinder = 0.82
		Short Cylinder = 1.15
		Streamlined Body = 0.04
		Streamlined Half Body = 0.09
	*/
	inline float CalcDrag(float fluidDensity, float velocity, float dragCoefficient, float crossSectionalArea)
	{
		return (fluidDensity * 0.5f) * (velocity * velocity) * dragCoefficient * crossSectionalArea;
	}

	inline float CalcAcceleration(float gravity, float mass)
	{
		float F = mass * gravity;
		return gravity * (F / mass);
	}

	inline float CalcTerminalVelocity(float mass, float gravity, float dragCoefficient, float crossSectionalArea, float fluidDensity)
	{
		return sqrt((2 * mass * gravity) / (fluidDensity * crossSectionalArea * dragCoefficient));
	}

	//if readCharacterBeforeToFind == true it will read everything BEFORE the character/word you entered but if it is false it will read the word you entered and everything after.
	std::string substr(const std::string& aStringToReadFrom, const std::string& toFind, bool readCharactersBeforeToFind, int charsToSkip);
	bool substr(const std::string& aStringToReadFrom, const std::string& toFind);
	struct SColor
	{
		SColor();
		SColor(unsigned int color);
		SColor(unsigned int red, unsigned int green, unsigned int blue, unsigned int alpha);
		unsigned int r;
		unsigned int g;
		unsigned int b;
		unsigned int a;
		int _color;
		void Convert(unsigned int aColor);
		void Convert(unsigned int aRed, unsigned int aGreen, unsigned int aBlue, unsigned int anAlpha);


	};

	float RadToDegree(float aRadian);
	float DegreeToRad(float aDegree);
}