#version 460 core

#define pi 3.1415926
#define epsilonDist 1e-3
#define epsilonSmall 1e-10
#define infinity 1e10

#define maxSpheres 100
#define maxMaterials 100
#define maxModels 100
#define BVHStackSize 32

vec4 debugValues = vec4(0);

out vec4 FragColor;
uniform sampler2D text;

struct Ray {
	vec3 origin;
	vec3 dir;
	vec3 invDir;
};

struct Material {
	vec3 color; // 12 
	float lightStrength; //4
	vec3 lightColor; // 12
	float smoothness; // 4
	float gloss; // 4
	bool glass; // 4
	float refraction; // 4
	float pad; // 4
};

struct Sphere {
	vec3 pos; // 12
	float radius; // 4
	int matIndex; // 4
	float pad1; // 4
	float pad2; // 4
	float pad3; // 4
};

struct Triangle {
	vec3 posA; // 12
	float pad1; // 4
	vec3 posB; // 12
	float pad2; // 4
	vec3 posC; // 12
	float pad3; // 4
	vec3 normA; // 12
	float pad4; // 4
	vec3 normB; // 12
	float pad5; // 4
	vec3 normC;// 12
	float pad6; // 4
};

struct Model {
	mat4 transform; // 64
	mat4 invTransform; // 64
	int matIndex; // 4
	int startIndex; // 4
	int endIndex; // 4
	int BVHIndex; // 4
};

struct BVHNode {
	vec3 min; // 12
	int leftIndex; // 4 (index of left child if triCount is 0, index of first triangle otherwise)
	vec3 max; // 12
	int triCount; // 4
};

struct hitData {
	bool didHit;
	bool inside;
	float dist;
	vec3 pos;
	vec3 normal;
	Material mat;
};


layout(std140, binding = 0) uniform bindPoint {
	mat4 model; // 64
	vec3 camPos; // 12
	float focus; // 4
	float focusDistance; // 4
	float aperture; // 4
	vec2 res; // 8
	int maxBounces; // 4
	int rayCount; // 4
	int spheresLength; // 4
	int modelsLength; // 4
	int accumulationFrame; // 4
	bool environmentLightFlag; // 4
	float pad1; // 4
	float pad2; // 4

	Sphere spheres[maxSpheres];
	Material materials[maxMaterials];
	Model models[maxModels];
};

layout(std140, binding = 1) buffer triangleSSBO {
    Triangle triangles[];
};

layout(std140, binding = 2) buffer nodeSSBO {
    BVHNode nodes[];
};

layout(std430, binding = 3) buffer triIndexSSBO {
    int nodeTriIndex[];
};


// https://www.shadertoy.com/view/XlGcRh
float random(inout uint seed) {
	seed = seed * 747796405 + 2891336453;
	uint result = ((seed >> ((seed >> 28) + 4)) ^ seed) * 277803737;
	result = (result >> 22) ^ result;
	return result / 4294967295.0;
}


// equal distribution across sphere
// https://dspguide.com/ch2/6.html
float randomNormalDist(inout uint seed) {
	// Produces NaN when log(0)
    return sqrt(-2.0 * log(max(random(seed), epsilonSmall))) * cos(2.0 * pi * random(seed));
}


vec2 randomCircle(inout uint seed) {
	float angle = 2.0 * pi * random(seed);
	return sqrt(random(seed)) * vec2(cos(angle), sin(angle));
}


bool sphereIntersect(Ray ray, Sphere sphere, out float dist, out bool inside) {
	// setting up quadratic
	vec3 a = sphere.pos - ray.origin;
	float b = dot(ray.dir, a);
	float c = b * b - dot(a, a) + sphere.radius * sphere.radius;

	// no real roots
	if (c < 0.0) return false;

	float d = sqrt(c);
	dist = b - d;
	inside = false;

	// epsilon sets clip distance and fixes banding
	if (dist < epsilonDist) {
		dist = b + d;
		inside = true;
	}
	return dist > epsilonDist;
};


// Möller–Trumbore
bool triangleIntersect(Ray ray, Triangle tri, out float t, out float u, out float v) {
	vec3 AB = tri.posB - tri.posA;
	vec3 AC = tri.posC - tri.posA;
	vec3 normal = cross(ray.dir, AC);
	float det = dot(AB, normal);

	// ray parralel to triangle
	if (abs(det) < epsilonSmall) return false;

	// cramers rule
	float invDet = 1.0 / det;
	vec3 AO = ray.origin - tri.posA;
	u = dot(AO, normal) * invDet;
	if (u < 0.0 || u > 1.0) return false;

	vec3 AOxAB = cross(AO, AB);
	v = dot(ray.dir, AOxAB) * invDet;
	if (v < 0.0 || u + v > 1.0) return false;

	// hit distance
	t = dot(AC, AOxAB) * invDet;
	return t > epsilonDist;
};


// slab method
// https://tavianator.com/2022/ray_box_boundary.html
float aabbIntersect(Ray ray, vec3 aabbMin, vec3 aabbMax) {
	vec3 t1 = (aabbMin - ray.origin) * ray.invDir;
	vec3 t2 = (aabbMax - ray.origin) * ray.invDir;
	vec3 tnear = min(t1, t2);
	vec3 tfar = max(t1, t2);
	float tmin = max(max(tnear.x, tnear.y), max(tnear.z, 0.0));
	float tmax = min(min(tfar.x, tfar.y), tfar.z);
	return tmin <= tmax ? tmin : infinity;
};


hitData bvhIntersect(Ray ray, Model model, float maxDist) {
	hitData result;
	result.didHit = false;
	result.inside = false;
	result.dist = maxDist;

	int stack[BVHStackSize];
	float stackDist[BVHStackSize];
	int index = 1;
	int bestTri = -1;
	float bestU = 0.0;
	float bestV = 0.0;

	// root node
	stack[0] = model.BVHIndex;
	stackDist[0] = 0.0;
	while (index > 0) {
		--index;

		// closest hit moved (cull further nodes allready in stack)
		if (stackDist[index] >= result.dist) continue;
		BVHNode node = nodes[stack[index]];

		// leaf node
		if (node.triCount > 0) {
			// triangle intersections
			for (int i = node.leftIndex; i < node.leftIndex + node.triCount; i++) {
				float t;
				float u;
				float v;

				if (triangleIntersect(ray, triangles[nodeTriIndex[i]], t, u, v) && t < result.dist) {
					result.dist = t;
					bestTri = i;
					bestU = u;
					bestV = v;
				}

				// tri tests debug
				debugValues.y++;
			}
		}

		else {
			// node tests debug
			debugValues.x += 2;

			BVHNode left = nodes[node.leftIndex];
			BVHNode right = nodes[node.leftIndex + 1];

			// node intersections
			float leftDist = aabbIntersect(ray, left.min, left.max);
			float rightDist = aabbIntersect(ray, right.min, right.max);

			// closer node pushed to stack last
			if (leftDist > rightDist) {
				if (leftDist < result.dist) {
					stack[index] = node.leftIndex;
					stackDist[index++] = leftDist;
				}
				if (rightDist < result.dist) {
					stack[index] = node.leftIndex + 1;
					stackDist[index++] = rightDist;
				}
			} else {
				if (rightDist < result.dist) {
					stack[index] = node.leftIndex + 1;
					stackDist[index++] = rightDist;
				}
				if (leftDist < result.dist) {
					stack[index] = node.leftIndex;
					stackDist[index++] = leftDist;
				}
			}
		}
	}

	// closest triangle (hit)
	if (bestTri >= 0) {
		Triangle tri = triangles[nodeTriIndex[bestTri]];
		result.didHit = true;

		vec3 normal = (1.0 - bestU - bestV) * tri.normA + bestU * tri.normB + bestV * tri.normC;
		// fallback if normal 0
		if (dot(normal, normal) < epsilonSmall) normal = cross(tri.posB - tri.posA, tri.posC - tri.posA);
		result.normal = normalize(normal);

		result.inside = dot(ray.dir, result.normal) > 0.0;
		if (result.inside) result.normal = - result.normal;
	}

	return result;
};


hitData getCollision(Ray ray) {
	hitData result;
	result.didHit = false;
	result.dist = infinity;
	hitData hit;

	// sphere intersections
	for (int i = 0; i < spheresLength; i++) {
		float t;
		bool inside;
		if (sphereIntersect(ray, spheres[i], t, inside) && t < result.dist) {
			result.didHit = true;
			result.dist = t;
			result.inside = inside;
			result.pos = ray.origin + ray.dir * t;
			result.normal = normalize(result.pos - spheres[i].pos);
			if (result.inside) result.normal = - result.normal;
			result.mat = materials[spheres[i].matIndex];
		}
	}


	// save ray to restore from model space
	vec3 originalOrigin = ray.origin;
	vec3 originalDir = ray.dir;

	// model (bvh & triangle) intersections
	for (int i = 0; i < modelsLength; i++) {
		Model model = models[i];

		// transform ray to model space
		ray.origin = (model.invTransform * vec4(originalOrigin, 1.0)).xyz;
		ray.dir = (model.invTransform * vec4(originalDir, 0.0)).xyz;
		ray.invDir = 1.0 / ray.dir;

		// skips bvh intersection tests if closer found
		if (aabbIntersect(ray, nodes[model.BVHIndex].min, nodes[model.BVHIndex].max) >= result.dist) continue;

		hit = bvhIntersect(ray, model, result.dist);
		if (hit.didHit) {
			result = hit;

			// transform result to world space
			result.pos = originalOrigin + originalDir * result.dist;
			result.normal = normalize((model.transform * vec4(hit.normal, 0.0)).xyz);

			result.mat = materials[model.matIndex];
		}
	}

	// restore ray back to world space
	ray.dir = originalDir;

	return result;
};


vec3 randBounce(vec3 normal, inout uint seed) {
	vec3 bounce = normalize(vec3(randomNormalDist(seed), randomNormalDist(seed), randomNormalDist(seed)));
	// same hemisphere
	return dot(normal, bounce) < 0 ? -bounce : bounce;
};


vec3 reflectRay(vec3 dir, vec3 normal) {
	return normalize(dir - 2.0 * dot(normal, dir) * normal);
};


vec3 environmentLight(Ray ray) {
	return vec3(0.4, 0.5, 0.9) * (ray.dir.y + 0.95);
}


// snells law, gives refracted angle
vec3 snells(vec3 dir, vec3 normal, float index, float cosine) {
	float value = 1.0 - index * index * (1.0 - cosine * cosine);
	// reflect
	if (value < 0.0) return vec3(0);
	// refract
	return index * dir + (index * cosine - sqrt(value)) * normal;
}


// Schlicks approximation
float fresnel(float index, float cosine) {
	float r0 = (1.0 - index) / (1.0 + index);
	r0 *= r0;
	// clamp prevents NaN
	return r0 + (1.0 - r0) * pow(clamp(1.0 - cosine, 0.0, 1.0), 5);
}


vec3 trace(Ray ray, inout uint seed) {
	vec3 color = vec3(1.0);
	vec3 light = vec3(0.0);
	hitData hit;

	for (int i = 0; i <= maxBounces; i++) {
		hit = getCollision(ray);

		if (!hit.didHit) {
			if (environmentLightFlag) light += environmentLight(ray) * color;
			break;
		}

		Material mat = hit.mat;
		ray.origin = hit.pos;

		// Color from before surface hit
		light += mat.lightColor * mat.lightStrength * color;

		// transparent
		if (mat.glass) {
			float index = hit.inside ? mat.refraction : 1.0 / mat.refraction;
			float cosine = dot(-ray.dir, hit.normal);
			vec3 refractDir = snells(ray.dir, hit.normal, index, cosine);

			// cosine of least dense side
			float cosFresnel = index > 1.0 ? sqrt(max(0.0, 1.0 - index * index * (1.0 - cosine * cosine))) : cosine;
			bool reflected = refractDir == vec3(0) || fresnel(index, cosFresnel) > random(seed);

			// diffusion and blur
			vec3 side = reflected ? hit.normal : -hit.normal;
			vec3 dir = reflected ? reflectRay(ray.dir, hit.normal) : refractDir;
			ray.dir = normalize(dir + (1.0 - mat.smoothness) * randBounce(side, seed));
		}

		// matt and reflective
		else {
			vec3 diffuseDir = randBounce(hit.normal, seed);
			// chance of diffuse or specular reflection
			if (random(seed) >= mat.gloss) {
				ray.dir = diffuseDir;
				color *= dot(hit.normal, diffuseDir) * 2.0;
			}
			// specular reflection / blur
			else ray.dir = normalize(mat.smoothness * reflectRay(ray.dir, hit.normal) + (1.0 - mat.smoothness) * diffuseDir);
		}

		// tint
		color *= mat.color;
	}
	return light;
};
	

void main() {
	uint seed = uint((gl_FragCoord.y * res.x) + gl_FragCoord.x) + accumulationFrame * 20983476; // large arbitrary  number 
	float x = -res.x / 2.0 + gl_FragCoord.x;
	float y = -res.y / 2.0 + gl_FragCoord.y;
	vec3 totalLight = vec3(0);

	for (int i = 0; i < rayCount; i++) {
		Ray ray;

		// anti aliasing and depth of field
		vec3 lens = vec3(randomCircle(seed) * aperture, 0.0);
		ray.origin = camPos + mat3(model) * lens;
		ray.dir = normalize(mat3(model) * (vec3(x + random(seed) - 0.5, y + random(seed) - 0.5, focus) * (focusDistance / focus) - lens));

		totalLight += trace(ray, seed);
	}
	totalLight /= rayCount;

	// pixel accumulation from previous frames
	float weight = 1.0 / accumulationFrame;
	totalLight = (1.0 - weight) * texelFetch(text, ivec2(gl_FragCoord.xy), 0).rgb + totalLight * weight;
	
	// final output color
	FragColor = vec4(totalLight, 1.0);
	//FragColor = vec4(debugValues.x / 300.0, debugValues.y / 100.0, 0.0, 1.0);
	//if (any(isnan(totalLight))) FragColor = vec4(1, 0, 1, 1);
};