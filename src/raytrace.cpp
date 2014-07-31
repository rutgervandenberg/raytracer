#include <ctime> /* clock() */
#include <xmmintrin.h>
#include <cstdlib>
#include <cstdio>
#include <cmath> 
#include <cstring>
#include <thread>
#include <mutex>

#include "mem.hpp"
#include "matrix.hpp"
#include "raytrace.hpp"
#include "geometry.hpp"
#include "config.hpp"
#include "camera.hpp"

int numintersects;
int numrays;

void createRays(Camera& cam, Image& image, vec4* rays) {
	// corner rays (LEFTUP, RIGHTUP, LEFTDOWN, RIGHTDOWN)
	vec4 corners[4];
	float ratio = (float)image.width / (float)image.height;
	corners[0] = vec4(0.5f * ratio, 0.5f, -1, 1);
	corners[1] = vec4(-0.5f * ratio, 0.5f, -1, 1);
	corners[2] = vec4(0.5f * ratio, -0.5f, -1, 1);
	corners[3] = vec4(-0.5f * ratio, -0.5f, -1, 1);

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
			vec4 val1 = vec4s(xf1 * yf1);
			vec4 val2 = vec4s(xf2 * yf1);
			vec4 val3 = vec4s(xf1 * yf2);
			vec4 val4 = vec4s(xf2 * yf2);

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

// threaded data (temporary global)
mutex m;
int scanline;

void worker(Image* image, vec4* rays, const Mesh* mesh, const Octree* octree) {
	int line = -1;

	while (1) {
		// obtain scanline
		m.lock();
		line = scanline;
		scanline += 4;
		m.unlock();

		// quit when done
		if (line >= image->height) // TODO
			break;

		// calculate indices
		int start = line * image->width;
		int end = start + image->width * 4;

		vec4 light = vec4(1, 1, 1.9, 0);
		int temp;
		for (int i = start; i < end; i++) {

			// nearest triangle
			vec4 mindist = vec4s(1e10f);
			vec4 color = vec4(0, 0, 0, 0);
			image->data[i] = zero;
			// loop through al triangles
			for (int j = 0; j < mesh->numtriangles; j++) {
				triangle tr = &mesh->triangles[j * 3];
				vec4 orig = rays[i * 2 + 0];
				vec4 dir = rays[i * 2 + 1];
				vec4 dist = intersect(orig, dir, tr);
				vec4 impact = orig + dist * dir;
				vec4 cmp = dist & (dist < mindist) & (dist > zero);

				vec4 normal = cross(tr[1], tr[2]);
				vec4 len = sqrt(dot(normal, normal, 0x7F));
				normal = normal / len;

				_mm_store_ss((float*) &temp, cmp);
				if (temp) {
					// calculate distances
					vec4 a = surface(tr[1], tr[2]);
					vec4 a1 = surface(impact, tr[1]) / a;
					vec4 a2 = surface(impact, tr[2]) / a;
					vec4 a3 = one - a1 - a2;

					vec4 dist3 = impact - light;
					dist3 = (dot(dist3, dist3, 0x7F));
					//tolight = tolight / sqrt(dot(tolight, tolight, 0x7F));
					//vec4 factor = dot(normal, tolight, 0x7F);

					image->data[i] = vec4s(0.75f) / dist3;// +normal * vec4s(0.125f);
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
	const char* strs[] = { "", "thousand", "million", "billion", "trillion", "gazillion", "vermicellion" };
	int idx = 0;
	while (num >= 1000) {
		idx++;
		num /= 1000;
	}
	sprintf(buf1, "%d %s", (int) num, strs[idx]);
	return buf1;
}

char buf2[1024];
const char* bigtime(float time) {
	int idx = 8;
	const char* strs[] = { "y", "z", "a", "f", "p", "n", "u", "m", "", "k", "M", "G", "T", "P", "E", "Z", "Y" };

	if (time < 1.0f) {
		while (time < 1.0f && idx > 0) {
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

extern float random1();
void raytrace(const Config& conf, const Mesh& mesh, const Octree& octree, Image* image) {
	Camera cam(vec4(2.0f + random1(), 0.1f - 2, 1.0f, 0.0f), 0.0f, 0.5f * 3.1415926f);

	// create rays
	vec4* rays = (vec4*)ialloc(image->width * image->height * 16 * 2);
	createRays(cam, *image, rays);

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

__declspec(align(16)) vec4 rays[640 * 480 * 2];

void raytrace2(Camera& cam, const Config& conf, const Mesh& mesh, const Octree& octree, Image* image) {

	// create rays
	createRays(cam, *image, rays);

	// prepare threaded variables
	scanline = 0;
	

	// create threads
	thread* th = new thread[conf.numcores];
	for (int i = 0; i < conf.numcores; i++)
		th[i] = thread(worker, image, rays, &mesh, &octree);

	// wait for threads
	for (int i = 0; i < conf.numcores; i++)
		th[i].join();
}