#include "geometry.hpp"

/**
 *	This function is rescheduled to allow
 *	for better pipeline utilisation
 */
vec4 intersect(vec4 p, vec4 d, vec4* tr) {
	// axes of the triangle
	vec4 e1 = tr[1];
	vec4 e2 = tr[2];

	// relative offset
	vec4 s = p - tr[0];

	// how parallel the ray and triangle are
	vec4 h = cross(d, e2);
	vec4 det = dot(e1, h, 0xFF);

	// scalar on first axis
	vec4 u = dot(s, h, 0x7F) / det;

	// scalar on second axis
	vec4 q = cross(s, e1);
	vec4 v = dot(d, q, 0x7F) / det;

	// quit when out of bounds
	vec4 mask1 = u >= zero & u <= one;
	vec4 mask2 = v >= zero & u + v <= one;

	// at this stage we can compute t to find out where
	// the intersection point is on the line
	vec4 t = dot(e2, q, 0x7F) / det;

	// return intersection point 
	//return (p + t * d) & mask1 & mask2;

	return t & mask1 & mask2;
}

template <typename T, typename I>
T intersectPacket(const TriAccelArray<T, I>& a, int i,
	const raypacket<T>& o, const raypacket<T>& d) {

	// loaded from triAccel
	const T& nu = a.nu[i], np = a.np[i], nv = a.nv[i], pu = a.pu[i], pv = a.pv[i],
		e0u = a.e0u[i], e0v = a.e0v[i],
		e1u = a.e1u[i],
		e1v = a.e1v[i];

	// indices from ci TODO efficiency
	int u = 0, v = 2;
	const int w = a.ci[i].m256i_i32[0]; // TODO
	if (w == 0) u = 1;
	if (w == 2) v = 1;

	// temp
	T det, dett, detu, detv, nrv, nru, du, dv, ou, ov, tmpdet0, tmpdet1;

	dett = np;
	dett = dett - o.t[w];
	du = nu;
	dv = nv;
	ou = pu;
	ou = ou - o.t[u];
	ov = pv;
	ov = ov - o.t[v];
	du = du * ou;
	dv = dv * ov;
	dett = dett + du;
	dett = dett + dv;
	du = d.t[u];
	dv = d.t[v];
	det = nu;
	det = det * du;
	nrv = nv;
	nrv = nrv * dv;

	det = det + d.t[w];
	det = det + nrv;

	nru = ou * det;
	du = du - nru;

	nrv = ov * det;
	dv = dv - nrv;

	nru = e1v;
	nrv = e1u;
	nru = nru * du;
	nrv = nrv * dv;
	detu = nru - nrv;
	nrv = e0u;
	nrv = nrv * dv;
	dv = e0v;
	dv = dv * du;
	detv = nrv - dv;

	// if intersection
	tmpdet0 = det - detu - detv;
	tmpdet0 = tmpdet0 ^ detu;
	tmpdet1 = detv ^ detu;
	tmpdet0 = convert<float, T>(0xFFFFFFFF) ^ (tmpdet0 | tmpdet1);

	//bool b = true;
	//bool a = !b;
	//bool a =  
	// calc intersection
	//vec4 rdet = VEC4S(1.0f) / det;
	//vec4 t = dett * rdet;
	//vec4 ubar = detu * rdet;
	//vec4 vbar = detv * rdet;

	return tmpdet0;
	//return _mm256_cmp_ps(det, detu, _MM_256);
}

void testintersect() {
	vec4 poly[3];
	poly[2] = vec4(1, 1, 0, 1);
	poly[1] = vec4(2, 1, 0, 1);
	poly[0] = vec4(1, 2, 0, 1);

	vec4 ray1[2];
	ray1[0] = vec4(1.25f, 1.25f, -1, 1);
	ray1[1] = vec4(0, 0, 1, 1);

	vec4 res1 = intersect(ray1[0], ray1[1], poly);
	vec4 exp1 = vec4(0.25f, 0.25f, 0, 1);

	//if (memcmp(&res1, &exp1, 16))
	//	printf("TEST FAILED!");
}