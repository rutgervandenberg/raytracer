#pragma once
#include <vector>
#include "mesh.hpp"
using namespace std;

class Octree {
private:
	int maxdepth;
	void calcSize(const Mesh& mesh);

	/* half the size of the AABB - in other words, the size of its potential subtrees. */
	float halfsize;

	/* Top left up corner */
	vec4 pos;

	/* the subtrees, if any */
	Octree** sub;

	/* create subtrees */
	void split();

	/* Get the sub-tree index of a point */
	int follow(const vec4& vertex);


	/* does the ray hit the plane defined by the OTHER two axes, INSIDE the box */
	bool collidePlane(int axis, const vec4& orig, const vec4& dir) const;

	/* will the given ray hit this box? */
	bool collide(const vec4& orig, const vec4& dir) const;
	vector<triangle> leaves;

	void print(int tabs);
public:
	Octree(const Mesh& mesh);
	Octree(vec4& position, float halfsize, int maxdepth);
	~Octree();

	void add(triangle tr);

	/* Determine which triangle is being hit */
	vec4 hit(const vec4& orig, const vec4& dir, triangle* tri) const;

	void print();
};
