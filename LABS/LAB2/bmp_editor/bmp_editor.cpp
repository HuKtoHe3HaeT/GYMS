// bmp_editor.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <string>
#include <iostream>
#include <fstream>


using namespace std;


#pragma pack(2)

const string file_ex = ".TTV";

// Определение структуры для заголовка файла BMP
typedef struct tBITMAPFILEHEADER
{
	WORD bfType;          // Тип файла BMP (должен быть 'BM')
	DWORD bfSize;         // Размер файла в байтах
	WORD bfReserved1;     // Зарезервировано (должно быть 0)
	WORD bfReserved2;     // Зарезервировано (должно быть 0)
	DWORD bfOffBits;      // Смещение до начала данных изображения
}sFileHead;

// Определение структуры для информационного заголовка BMP
typedef struct tBITMAPINFOHEADER
{
	DWORD biSize;          // Размер структуры информационного заголовка
	LONG biWidth;          // Ширина изображения в пикселях
	LONG biHeight;         // Высота изображения в пикселях
	WORD biPlanes;         // Количество плоскостей (должно быть 1)
	WORD biBitCount;       // Глубина цвета (бит на пиксель)
	DWORD biCompression;   // Тип сжатия (обычно 0 для несжатых изображений)
	DWORD biSizeImage;     // Размер изображения в байтах (может быть 0 для несжатых изображений)
	LONG biXPelsPerMeter;  // Горизонтальное разрешение (пикселей на метр)
	LONG biYPelsPerMeter;  // Вертикальное разрешение (пикселей на метр)
	DWORD biClrUsed;       // Количество используемых цветовых индексов (0 для полной палитры)
	DWORD biClrImportant;  // Количество "важных" цветовых индексов (0 для всех цветов важных)
}sInfoHead;

typedef struct tTTVMAPINFOHEADER
{
	DWORD ttType;
	WORD ttFileVersion;
	CHAR ttNameOfProgram[8];         
	DWORD ttFileSize;        
	DWORD ttHeaderSize;
	LONG ttSizeOfBitmap;
	WORD ttBitCount;
	LONG ttWidth;
}sTTVFileHead;

sFileHead FileHead;       // Структура для хранения заголовка файла BMP
sInfoHead InfoHead;       // Структура для хранения информационного заголовка BMP
sTTVFileHead TtvHead;

// Определение структуры для представления цвета пикселя
struct Color
{
	BYTE blue;  // Компонента синего цвета
	BYTE green; // Компонента зеленого цвета
	BYTE red;   // Компонента красного цвета
};

// Размер 1-го пикселя в байтах
int pixel_size = sizeof(Color);

// Тип изображения: 1 - BMP, 2 - CMP (какой-то код, предположительно)
int img_type = 0;

// Указатели на исходное и результативное изображения
Color* src_image = 0; // Указатель на исходное изображение
Color* dst_image = 0; // Указатель на результативное изображение
Color* bw_image = 0;
// Размеры изображения (ширина и высота)
int width = 0;
int height = 0;

// Функция для вывода заголовка BMP файла
void ShowBMPHeaders(tBITMAPFILEHEADER fh, tBITMAPINFOHEADER ih)
{
	cout << "Type: " << (CHAR)fh.bfType << endl;          // Тип файла BMP (должен быть 'BM')
	cout << "Size: " << fh.bfSize << endl;               // Размер файла в байтах
	cout << "Shift of bits: " << fh.bfOffBits << endl;   // Смещение до начала данных изображения
	cout << "Width: " << ih.biWidth << endl;            // Ширина изображения в пикселях
	cout << "Height: " << ih.biHeight << endl;          // Высота изображения в пикселях
	cout << "Planes: " << ih.biPlanes << endl;          // Количество плоскостей (должно быть 1)
	cout << "BitCount: " << ih.biBitCount << endl;      // Глубина цвета (бит на пиксель)
	cout << "Compression: " << ih.biCompression << endl; // Тип сжатия (обычно 0 для несжатых изображений)
}
void ShowTTVHeaders(tTTVMAPINFOHEADER tt)
{
	cout << "Type: " << (CHAR)tt.ttType << endl;          
	cout << "File version: " <<tt.ttFileVersion << endl;               
	cout << "The name of the program: " <<tt.ttNameOfProgram << endl;  
	cout << "FileSize: " << tt.ttFileSize << endl;           
	cout << "HeadSize: " << tt.ttHeaderSize << endl;          
	cout << "RasterSize: " << tt.ttSizeOfBitmap << endl;         
	cout << "BitCount: " << tt.ttBitCount << endl;     
	cout << "Width: " << tt.ttWidth << endl;
}


// Функция для загрузки изображения
bool OpenImage(string path)
{
	ifstream img_file;
	Color temp;
	char buf[3];

	// Открыть файл на чтение
	img_file.open(path.c_str(), ios::in | ios::binary);
	if (!img_file)
	{
		cout << "File isn't open!" << endl;
		return false;
	}

	// Считать заголовки BMP
	img_file.read((char*)&FileHead, sizeof(FileHead));
	img_file.read((char*)&InfoHead, sizeof(InfoHead));

	img_type = 1;
	ShowBMPHeaders(FileHead, InfoHead);
	// Присвоить ширину и высоту изображения
	width = InfoHead.biWidth;
	height = InfoHead.biHeight;

	// Выделить место под изображение
	src_image = new Color[width * height];

	int i, j;
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			img_file.read((char*)&temp, pixel_size);
			src_image[i * width + j] = temp;
		}
		// Дочитать байты, используемые для выравнивания до двойного слова
		img_file.read((char*)buf, j % 4);
	}
	img_file.close();

	return true;
}
bool OpenImageTtv(string path)
{
	ifstream img_file;
	Color temp;
	char buf[3];

	// Открыть файл на чтение
	img_file.open(path.c_str(), ios::in | ios::binary);
	if (!img_file)
	{
		cout << "File isn't open!" << endl;
		return false;
	}

	// Считать заголовки BMP
	img_file.read((char*)&TtvHead, sizeof(TtvHead));

	img_type = 1;
	ShowTTVHeaders(TtvHead);
	// Присвоить ширину и высоту изображения
	width = TtvHead.ttWidth;
	height = TtvHead.ttBitCount * 24 / 8 / TtvHead.ttWidth;

	// Выделить место под изображение
	src_image = new Color[width * height];

	int i, j;
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			img_file.read((char*)&temp, pixel_size);
			src_image[i * width + j] = temp;
		}
		// Дочитать байты, используемые для выравнивания до двойного слова
		img_file.read((char*)buf, j % 4);
	}
	img_file.close();

	return true;
}


// Функция для сохранения изображения
bool SaveImage(string path)
{
	ofstream img_file;
	char buf[3];

	// Открыть файл на запись
	img_file.open(path.c_str(), ios::out | ios::binary);
	if (!img_file)
	{
		return false;
	}

	img_file.write((char*)&FileHead, sizeof(FileHead));
	img_file.write((char*)&InfoHead, sizeof(InfoHead));

	// Скопировать из исходного в результирующее изображение
	dst_image = new Color[width * height];
	memcpy(dst_image, src_image, width * height * sizeof(Color));

	// Записать файл
	int i, j;
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			img_file.write((char*)&dst_image[i * width + j], pixel_size);
		}
		// Записать байты, используемые для выравнивания до двойного слова
		img_file.write((char*)buf, j % 4);
	}
	img_file.close();

	return true;
}


// Функция для копирования содержимого результирующего изображения в исходное
void CopyDstToSrc()
{
	if (dst_image != 0)
	{
		memcpy(src_image, dst_image, width * height * sizeof(Color));
	}
}

// Функция для добавления шума к изображению с заданной долей вероятности
void AddNoise(double probability)
{
	int size = width * height;
	int count = (int)(size * probability) / 100;
	int x, y;
	long pos;
	for (int i = 0; i < count; i++)
	{
		x = rand() % width;
		y = rand() % height;
		pos = y * width + x;
		src_image[pos].blue = rand() % 256;
		src_image[pos].green = rand() % 256;
		src_image[pos].red = rand() % 256;
	}
	cout << "Points were added: " << count << endl;
}

// Функция для отображения текущего изображения с помощью вызова стандартного просмотрщика
void ShowImage(string path)
{
	if (img_type == 1) {
		ShowBMPHeaders(FileHead, InfoHead);
		system(path.c_str());
	}
	if (img_type == 2) {
		ShowTTVHeaders(TtvHead);
	}
}

// Функция для считывания пути к изображению от пользователя
void ReadPath(string& str)
{
	str.clear();
	cout << "Enter the path to the image" << endl;
	cin >> str;
}

bool BmpToTTV(string path)
{
	ifstream img_file_in;
	Color temp;
	char buf[3];

	
	img_file_in.open(path.c_str(), ios::in | ios::binary);
	if (!img_file_in)
	{
		cout << "File isn`t open!" << endl;
		return false;
	}

	
	img_file_in.read((char*)&FileHead, sizeof(FileHead));
	img_file_in.read((char*)&InfoHead, sizeof(InfoHead));

	img_type = 1;
	ShowBMPHeaders(FileHead, InfoHead);
	
	width = InfoHead.biWidth;
	height = InfoHead.biHeight;



	src_image = new Color[width * height];

	int j, i;

	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			img_file_in.read((char*)&temp, pixel_size);
			src_image[i * width + j] = temp;
		}
		
		img_file_in.read((char*)buf, j % 4);
	}
	img_file_in.close();

	TtvHead.ttType = FileHead.bfType;
	TtvHead.ttFileVersion = 4;
	strncpy(TtvHead.ttNameOfProgram, "VStudio\0", 8);
	TtvHead.ttSizeOfBitmap = width * height * 8 / InfoHead.biBitCount;
	TtvHead.ttHeaderSize = sizeof(TtvHead);
	TtvHead.ttFileSize = TtvHead.ttHeaderSize + TtvHead.ttSizeOfBitmap;
	TtvHead.ttBitCount = InfoHead.biBitCount;
	TtvHead.ttWidth = InfoHead.biWidth;

	ofstream img_file_out;

	ReadPath(path);
	img_file_out.open(path.c_str(), ios::out | ios::binary);
	if (!img_file_out)
	{
		return false;
	}

	img_file_out.write((char*)&TtvHead, sizeof(TtvHead));

	dst_image = new Color[width * height];
	memcpy(dst_image, src_image, width * height * sizeof(Color));


	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			img_file_out.write((char*)&dst_image[i * width + j], pixel_size);
		}
		img_file_out.write((char*)buf, j % 4);
	}
	img_file_out.close();

	return true;
}

// Функция для освобождения памяти
void ClearMemory(void)
{
	// Освободить память исходного изображения
	if (src_image != 0)
	{
		delete[] src_image;
	}
	// Освободить память результрующего изображения
	if (dst_image != 0)
	{
		delete[] dst_image;
	}
}

// 5x1
void ApplyMedianFilter() {
	//Скопировать из исходного в результирующее изображение
	dst_image = new Color[width * height];
	memcpy(dst_image, src_image, width * height * sizeof(Color));

	int i, j;
	Color arr[5];
	for (i = 0; i < height; i++)
	{
		for (j = 2; j < width - 2; j++)
		{
			int idx = i * width + j;
			arr[0] = dst_image[idx - 2];
			arr[1] = dst_image[idx-1];
			arr[2] = dst_image[idx];
			arr[3] = dst_image[idx + 1];
			arr[4] = dst_image[idx + 2];

			for (int k = 0; k < 5; k++) {
				for (int l = k; l + 1 < 5; l++) {
					if (arr[l].red>arr[l+1].red) {
						swap(arr[l], arr[l + 1]);
					}
					if (arr[l].blue > arr[l + 1].blue) {
						swap(arr[l], arr[l + 1]);
					}
					if (arr[l].green > arr[l + 1].green) {
						swap(arr[l], arr[l + 1]);
					}
				}
			}

			dst_image[idx] = arr[2];
		}
	}
}

int* ToBrightArr() {
	int* brightArr = new int[width * height];
	int i, j, index;
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			index = i * width + j;
			brightArr[index] = src_image[index].blue * 0.11 + src_image[index].red * 0.3 + src_image[index].green * 0.59;

		}
	}
	return brightArr;
}
int* ContrastSHARA(int* brightArr) {
	int* contrastArr = new int[width * height];
	int X = 0, Y = 0;
	int i, j;

	for (i = 1; i < height - 1; i++)
	{
		for (j = 1; j < width - 1; j++)
		{
			X = (3*brightArr[(i + 1) * width + j - 1] + 10*brightArr[(i + 1) * width + j] + 3*brightArr[(i + 1) * width + j + 1]) -
				(3*brightArr[(i - 1) * width + j - 1] + 10*brightArr[(i - 1) * width + j] + 3*brightArr[(i - 1) * width + j + 1]);
			Y = (3*brightArr[(i - 1) * width + j - 1] + 10*brightArr[i * width + j - 1] + 3 * brightArr[(i + 1) * width + j - 1]) -
				(3 * brightArr[(i - 1) * width + j + 1] + 10*brightArr[i * width + j + 1] + 3 * brightArr[(i + 1) * width + j + 1]);
			contrastArr[i * width + j] = sqrt(abs(X * X + Y * Y));

		}
	}
	return contrastArr;
}

Color* ThresholdDetection(int* contrastArr) {
	Color* resultImg = new Color[width * height];
	int i, j, treshhold, index;
	cout << "Enter treshhold - ";
	cin >> treshhold;

	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			index = i * width + j;
			if (contrastArr[index] > treshhold) {
				resultImg[index].blue = 255;
				resultImg[index].green = 255;
				resultImg[index].red = 255;
			}
			else {
				resultImg[index].blue = 0;
				resultImg[index].green = 0;
				resultImg[index].red = 0;
			}
		}
	}
	return resultImg;
}

void ReadNoise(double& tmp) {
	cout << "Enter noise amount (int)" << endl;
	cin >> tmp;
}

int PromptChoice() {
	int tmp;
	cout << "Noise - 1,\nBMPtoTTV - 2\nFilter - 3\nSHARA - 4\nExit - 0." << endl;
	cin >> tmp;
	return tmp;
}

int main(int argc, char* argv[])
{
	//D:\Student\5sem\GIMS\bmp_editor\Sunset.bmp
	srand((unsigned)time(NULL));

	//Путь к текущему изображению
	string path_to_image, temp; double noise = 0.0;
	Color* res_image=0;

	/*ReadPath(path_to_image);
	OpenImage(path_to_image);
	ShowImage(path_to_image);*/

	switch (PromptChoice())
	{
	case 1:
		ReadNoise(noise);
		AddNoise(noise);
		ReadPath(temp);
		SaveImage(temp);
		ShowImage(temp);
		break;
	case 2:
		ReadPath(path_to_image);
		BmpToTTV(path_to_image);
		ClearMemory();
		ReadPath(path_to_image);
		OpenImageTtv(path_to_image);
		break;
	case 3:
		ApplyMedianFilter();
		CopyDstToSrc();
		ReadPath(temp);
		SaveImage(temp);
		ShowImage(temp);
		break;
	case 4:
		ReadPath(path_to_image);
		OpenImage(path_to_image);
		ShowImage(path_to_image);
		res_image = ThresholdDetection(ContrastSHARA(ToBrightArr()));
		src_image = res_image;
		ReadPath(temp);
		SaveImage(temp);
		ShowImage(temp);
		ClearMemory();
		break;
	default:
		break;
	}

	ClearMemory();
	cout << "END!" << endl;
	return 0;
}



