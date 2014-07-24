#pragma once
#include <vector>
#include "mesh.hpp"

class Octree {
private:
	unsigned int max_depth;
	void calcSize(Mesh& mesh);
	/*half the size of the AABB - in other words, the size of its potential subtrees.*/
	float halfsize;

	/*Top left up corner.*/
	__m128 position; 

	/*the subtrees, if any*/
	Octree** sub;

	/*create subtrees.*/
	void split();
	/*Check which sub-tree contains the given position.*/
	int follow(const __m128& vertex);


	/*does the ray hit the plane defined by the OTHER two axes, INSIDE the box*/
	bool collidePlane(int axis, const __m128& orig, const __m128& dir);
	/*will the given ray hit this box*/
	bool collide(const __m128& orig, const __m128& dir);
	std::vector<triangle> leaves;
public:
	Octree(__m128& position, float halfsize, unsigned int max_depth);
	~Octree();
	void build(Mesh& mesh);
	void add(triangle tr);
	/*does the given ray hit the triangle. */
	float hit(const __m128& orig, const __m128& dir, triangle* tri);
};
