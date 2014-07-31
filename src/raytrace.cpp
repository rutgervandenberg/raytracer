﻿#include <ctime> /* clock() */
#include <xmmintrin.h>
#include <cstdlib>
#include <cstdio>
#include <cmath> 
#include <pmmintrin.h>
#include <cstring>
#include <thread>
#include <mutex>

#include "mem.hpp"
#include "matrix.hpp"
#include "raytrace.hpp"
#include "geometry.hpp"
#include "config.hpp"

int numintersects;
int numrays;

/**
 *	Perform camera transformations
 *	(z axis: positive is up, zero is floor)
 */
struct Camera {
	vec4 pos;
	vec4 dir;
	vec4 rotation[4];

	float xdir, ydir;

	Camera(Config& config) {
	}

	Camera(vec4 pos, float xdir, float ydir) : pos(pos), dir(dir), xdir(xdir), ydir(ydir) {
		rotation[0] = VEC4(1, 0, 0, 0);
		rotation[1] = VEC4(0, cos(ydir), -sin(ydir), 0);
		rotation[2] = VEC4(0, sin(ydir), cos(ydir), 0);
		rotation[3] = VEC4(0, 0, 0, 1);
	}
};

void createRays(Camera& cam, Image& image, vec4* rays) {
	// corner rays (LEFTUP, RIGHTUP, LEFTDOWN, RIGHTDOWN)
	vec4 corners[4];
	corners[0] = VEC4(0.5f, 0.5f, -1, 1);
	corners[1] = VEC4(-0.5f, 0.5f, -1, 1);
	corners[2] = VEC4(0.5f, -0.5f, -1, 1);
	corners[3] = VEC4(-0.5f, -0.5f, -1, 1);

	for (int i = 0; i < 4; i++)
		corners[i] = mmultSSE2(cam.rotation, corners[i]);

	// calculate rays
	for (int y = 0; y < image.height; y++) {
		for (int x = 0; x < image.width; x++) {
			// factors
			float xf2 = (float)x / (float)image.width;
			float yf2 = (float)y / (float)image.height;
			float xf1 = 1.0f - xf2;
			float yf1 = 1.0f - yf2;

			// per corner
			vec4 val1 = VEC4S(xf1 * yf1);
			vec4 val2 = VEC4S(xf2 * yf1);
			vec4 val3 = VEC4S(xf1 * yf2);
			vec4 val4 = VEC4S(xf2 * yf2);

			// calculate ray
			vec4 ray1 = cam.pos;
			vec4 ray2 = corners[0] * val1;
			ray2 = ray2 + corners[1] * val2;
			ray2 = ray2 + corners[2] * val3;
			ray2 = ray2 + corners[3] * val4;

			// add ray
			rays[(y * image.width + x) * 2 + 0] = ray1;
			rays[(y * image.width + x) * 2 + 1] = ray2;
		}
	}
}

template <typename T>
void createRayPackets(vec4* rays, raypacket<T>* origs, raypacket<T>* dirs, int number) {
	// number of floats
	const static int numfloats = sizeof(T) / sizeof(float);
	// create ray packets
	for (int i = 0; i < number / numfloats; i++) {
		// transform
		T o[numfloats], d[numfloats];

		for (int j = 0; j < 4; j++) {
			o[j] = *(T*)&rays[i * 2 * sizeof T / 4 + j * 2 + 0];
			d[j] = *(T*)&rays[i * 2 * sizeof T / 4 + j * 2 + 1];
		}
		transpose<T>(o);
		transpose<T>(d);

		// copy
		for (int j = 0; j < 3; j++) {
			origs[i].t[j] = o[j];
			dirs[i].t[j] = d[j];
		}
	}
}

/* background */
/*__m128 fac = orig / dir;
			_mm_store_ps(val, fac);
			float a = val[2];

			__m128 f = VEC4S(a);
			__m128 impact = orig + f * dir;

			bool white = true;
			__m128 modulus = _mm_mod_ps2(impact, VEC4S(0.2f));
			_mm_store_ps(val, modulus);
			if (val[0] > 0.1f)
				white = !white;
			if (val[1] > 0.1f)
				white = !white;

			// set color
			if (white)
				image->data[y * w + x] = VEC4(1, 1, 1, 1);
			else
				image->data[y * w + x] = VEC4(0, 0, 0, 1);*/


// threaded data (temporary global)
mutex m;
int scanline;

void worker(Image* image, vec4* rays, const Mesh* mesh, const Octree* octree) {
	int line = -1;
	while (1) {
		// obtain scanline
		m.lock();
		line = scanline;
		scanline++;
		m.unlock();

		// quit when done
		if (line >= 480-1) // TODO
			break;

		// calculate indices
		int start = scanline * image->width;
		int end = start + image->width;

		int temp;
		for (int i = start; i < end; i++) {

			// nearest triangle
			vec4 mindist = VEC4S(1e10f);
			vec4 color = VEC4(0, 0, 0, 0);

			// loop through al triangles
			for (int j = 0; j < mesh->numtriangles; j++) {
				triangle tr = &mesh->triangles[j * 3];
				vec4 orig = rays[i * 2 + 0];
				vec4 dir = rays[i * 2 + 1];
				vec4 dist = intersect(orig, dir, tr);
				vec4 impact = orig + dist * dir - tr[0];
				vec4 cmp = dist & (dist < mindist);

				vec4 normal = cross(tr[1], tr[2]);
				vec4 len = sqrt(dot(normal, normal, 0x7F));
				normal = normal / len;

				_mm_store_ss((float*) &temp, cmp);
				if (temp) {
					// calculate distances
					vec4 a = surface(tr[1], tr[2]);
					vec4 a1 = surface(impact, tr[1]) / a;
					vec4 a2 = surface(impact, tr[2]) / a;
					vec4 a3 = a - a1 - a2;
					vec4 r = VEC4(1, 0, 0, 0);
					vec4 g = VEC4(0, 1, 0, 0);
					vec4 b = VEC4(0, 0, 1, 0);

					image->data[i] = r * a1 + g * a2 + b * a3;
					mindist = dist;
				}
			}

			// store image
			//image->data[i] = mindist;
		}
	}
}

char buf1[1024];
const char* bignum(long long num) {
	const static char* strs[] = { "a", "thousand", "million", "billion", "trillion", "gazillion", "vermicellion" };
	int idx = 0;
	while (num >= 1000) {
		idx++;
		num /= 1000;
	}
	sprintf(buf1, "%d %s", num, &strs[idx]);
	return buf1;
}

char buf2[1024];
const char* bigtime(float time) {
	int idx = 8;
	const char* strs[] = { "y", "z", "a", "f", "p", "n", "u", "m", "", "k", "M", "G", "T", "P", "E", "Z", "Y" };

	if (time < 1.0f) {
		while (time < 1.0f) {
			time *= 1000.0f;
			idx--;
		}
		sprintf(buf2, "%.1f %ss", time, strs[idx]);
	}
	else {
		sprintf(buf2, "%.0f s", time);
	}
	return buf2;
}

void raytrace(const Config& conf, const Mesh& mesh, const Octree& octree, Image* image) {
	Camera cam(VEC4(0, 0, 4, 1), 0, 0);

	// create rays
	vec4* rays = (vec4*)ialloc(image->width * image->height * 16 * 2);
	//raypacket<vec8>* origs = (raypacket<vec8>*)ialloc(640 * 480 * sizeof(raypacket<vec8>));
	//raypacket<vec8>* dirs = (raypacket<vec8>*)ialloc(640 * 480 * sizeof(raypacket<vec8>));
	createRays(cam, *image, rays);
	//createRayPackets<vec8>(rays, origs, dirs, 640 * 480);

	printf("\nPROPERTIES\n");
	printf("# cores = %d\n", conf.numcores);
	printf("# triangles = %s\n", bignum(mesh.numtriangles));
	printf("# pixels = %s\n", bignum(image->width * image->height));

	// prepare threaded variables
	scanline = 0;

	// measurement
	numintersects = image->width * image->height * mesh.numtriangles;
	numrays = image->width * image->height;
	clock_t start = clock();

	// create threads
	thread* th = new thread[conf.numcores];
	for (int i = 0; i < conf.numcores; i++) 
		th[i] = thread(worker, image, rays, &mesh, &octree);

	// wait for threads
	for (int i = 0; i < conf.numcores; i++)
		th[i].join();
	

	// measurement
	clock_t end = clock();
	float diff = (float)(end - start) / (float) CLOCKS_PER_SEC;
	printf("\nSTATISTICS\n");
	printf("time   = %s\n", bigtime(diff));
	printf("# rays = %d\n", numrays);
	printf("# hits = %d\n", numintersects);
	printf("time per ray = %s\n", bigtime(diff * conf.numcores / numrays));
	printf("time per hit = %s\n", bigtime(diff * conf.numcores / numintersects));
	printf("hits per sec = %.0f million\n", numintersects / diff / 1e6f);
	printf("cycles per hit = %.0f\n", 3.4e9f * conf.numcores / numintersects);

	// free data
	ifree(rays);
}
