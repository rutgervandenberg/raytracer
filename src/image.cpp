#include <cstdio> /* file output */
#include <string>
#include "mem.hpp"
#include "image.hpp"
using namespace std;

Image::Image() {
	width = 0;
	height = 0;
	data = (vec4*)ialloc(16 * 3 * width * height);
}

Image::Image(int width, int height) {
	this->width = width;
	this->height = height;
	data = (vec4*)ialloc(16 * 3 * width * height);
}

Image::~Image() {
	ifree(data);
}

void Image::load(char* path) {

}

void Image::write(const char * filename2) {
	string str = string(filename2) + string(".bmp");
	const char* filename = str.c_str();

	FILE* file;
	file = fopen(filename, "wb");
	if (!file)
		throw string("file not found");

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
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00 };

	// allocate buffer for converted image
	int len = width * height * 3;
	char* buf = new char[len];

	// get pointers to header
	unsigned int* filesize = (unsigned int*) (header + 2);
	unsigned int* w = (unsigned int*) (header + 18);
	unsigned int* h = (unsigned int*) (header + 22);

	// modify header
	*filesize = (sizeof(header) + len);
	*w = (width);
	*h = (height);

	// fill buffer, convert from RGB to BGR
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int i1 = y * width + x;
			int i2 = (height - 1 - y) * width + x;
			float* color = (float*)ialloc(4 * 4);
			_mm_store_ps(color, data[i1]);
			//color[0] = data[i1].m128_f32[2];
			//color[1] = data[i1].m128_f32[1];
			//color[2] = data[i1].m128_f32[0];
			//_mm_store_ps(color, data[i]);
			buf[i2 * 3 + 0] = (unsigned char)(color[2] * 255.0f);
			buf[i2 * 3 + 1] = (unsigned char)(color[1] * 255.0f);
			buf[i2 * 3 + 2] = (unsigned char)(color[0] * 255.0f);
			ifree(color);
		}
	}

	// write to file
	fwrite(header, 1, sizeof(header), file);
	fwrite(buf, 1, len, file);

	// deallocate buffer
	delete[] buf;

	fclose(file);
}
