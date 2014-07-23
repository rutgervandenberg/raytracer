#include <cstdio> /* file output */
#include "mem.hpp"
#include "image.hpp"
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
using namespace std;

Image::Image() {
	width = 0;
	height = 0;
	data = (__m128*)ialloc(16 * 3 * width * height);
}

Image::Image(int width, int height) {
	width = 0;
	height = 0;
	data = (__m128*)ialloc(16 * 3 * width * height);
}

Image::~Image() {
	ifree(data);
}

void Image::load(char* path) {

}

bool Image::write(const char * filename2) {
	string str = string(filename2) + string(".bmp");
	const char* filename = str.c_str();

	FILE* file;
	file = fopen(filename, "wb");
	if (!file) {
		printf("could not open file %s", filename);
		return false;
	}

	// bitmap header
	unsigned char header[] = {
	0x42, 0x4D, /* magic */
	0x3E, 0xF6, 0x02, 0x00,/* filesize in bytes @ 2*/
	0x00, 0x00, /*reserved*/
	0x00, 0x00, /*reserved2*/
	0x36, 0x00, 0x00, 0x00, /*offset*/

	0x28, 0x00, 0x00, 0x00, /*hor res*/
	0xFE, 0x00, 0x00, 0x00, /* width @ 18 */
	0xFE, 0x00, 0x00, 0x00, /* height @ 22 */
	0x01, 0x00, 0x18, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x08, 0xF6, 0x02, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00 };

	// allocate buffer for converted image
	int len = width * height * 3;
	char* buf = new char[len];

	// get pointers to header
	unsigned int* filesize = (unsigned int*) (header + 2);
	unsigned int* w = (unsigned int*) (header + 18);
	unsigned int* h = (unsigned int*) (header + 22);

	// modify header
	*filesize = htonl(sizeof(header) + len);
	*w = htonl(width);
	*h = htonl(height);

	// fill buffer, convert from RGB to BGR
	for (int i = 0; i < width * height; i++) {
		float color[4];
		_mm_store_ps(color, data[i]);
		buf[i * 3 + 0] = (unsigned char) (color[0] * 255.0f);
		buf[i * 3 + 1] = (unsigned char) (color[1] * 255.0f);
		buf[i * 3 + 2] = (unsigned char) (color[2] * 255.0f);
	}

	// write to file
	fwrite(header, 1, sizeof(header), file);
	fwrite(buf, 1, len, file);

	// deallocate buffer
	delete[] buf;

	fclose(file);
	printf("Image saved to %s!\n", filename);
	return true;
}
