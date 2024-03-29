#pragma once
class TGA32
{
public:
	TGA32();
	~TGA32();

	struct Image
	{
		unsigned short myWidth;
		unsigned short myHeight;
		unsigned char myBitDepth;
		unsigned char *myImage;
		~Image();
	};

	struct TgaHeader {
		unsigned char myIdLength;
		unsigned char myColorMapType;
		unsigned char myImageType;
		unsigned char myColorMapOrigin;
		unsigned short myColorMapLength;
		unsigned char myColorMapEntrySize;
		unsigned short myImageOriginX;
		unsigned short myImageOriginY;
		unsigned short myWidth;
		unsigned short myHeight;
		unsigned char myBpp;
		unsigned char myImageDescriptor;
	};

	static Image* Load(const char* aName);
	static void FlipImageData(const TgaHeader& header, unsigned char* source, unsigned char* destination);
	static void Save(const char* aName, int myWidth, int myHeight, int myBitDepth, unsigned char* myData);


};

