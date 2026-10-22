#version 460 core

#define pi 3.1415926f

#define maxSpheres 100
#define maxMaterials 100
#define maxModels 100
#define BVHStackSize 32

vec4 debugValues = vec4(0);

in vec2 texCoords;
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
	int leftIndex; // 4
	vec3 max; // 12
	int triIndex; // 4
	int triCount; // 4
	float pad1; // 4
	float pad2; // 4
	float pad3; // 4
};

struct hitData {
	bool didHit;
	bool inside;
	float dist;
	vec3 pos;
	vec3 normal;
	Material mat;
};


layout(std140) uniform bindPoint {
	mat4 model; // 64
	vec3 camPos; // 12
	float focus; // 4
	vec2 res; // 8
	int maxBounces; // 4
	int rayCount; // 4
	int spheresLength; // 4
	int modelsLength; // 4
	int accumulationFrame; // 4
	bool environmentLightFlag; // 4

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
    float angle = 2.0f * pi * random(seed);
    float value = sqrt(-2.0f * log(random(seed)));
    return value * cos(angle);
}

// M�ller�Trumbore
hitData triangleIntersect(Ray ray, Triangle tri) {
	hitData hit;
	hit.didHit = false;

	vec3 AB = tri.posB - tri.posA;
	vec3 AC = tri.posC - tri.posA;
	vec3 normal = cross(ray.dir, AC);
	float det = dot(AB, normal);
	float invDet = 1.0f / det;

	// ray parralel to triangle
	if (det < 0.0000001) return hit;

	// cramers rule
	vec3 AO = ray.origin - tri.posA;
	float u = dot(AO, normal);
	if (u < 0 || u > det) return hit;

	vec3 AOxAB = cross(AO, AB);
	float v = dot(ray.dir, AOxAB);
	if (v < 0 || u + v > det) return hit;

	// hit distance
	float t = dot(AC, AOxAB);
	// behind camera
	if (t < 0.0000001) return hit;

	u *= invDet;
	v *= invDet;
	t *= invDet;
	
	hit.didHit = true;
	hit.dist = t;
	hit.pos = ray.origin + (ray.dir * t);
	//hit.normal = normalize(tri.normA * u + tri.normB * v + (1.0f - u - v) * tri.normC);
	hit.normal = normalize((1.0f - u - v) * tri.normA + u * tri.normB + v * tri.normC);


	return hit;
};

hitData sphereIntersect(Ray ray, Sphere sphere) {
	hitData hit;
	hit.didHit = false;

	// setting up quadratic
	vec3 a = sphere.pos - ray.origin;
	float b = dot(ray.dir, a);
	float c = dot(a, a) - sphere.radius * sphere.radius;
	float d = b * b - c;

	// any real roots
	if (d >= 0.0f) {
		float dist = b - sqrt(d);
		// 0.001 sets clip distance and fixes banding
		if (dist < 0.001) { 
			dist = b + sqrt(d);
			// missed
			if (dist < 0.001) return hit;
		}

		// hit
		hit.dist = dist;
		hit.didHit = true;
		hit.pos = ray.origin + (ray.dir * dist);
		hit.normal = normalize(hit.pos - sphere.pos);

		// inside or outside sphere
		if (dot(ray.dir, hit.normal) > 0.0f) {
			hit.normal = -hit.normal;
			hit.inside = true;
		} else {
			hit.inside = false;
		}

	}
	return hit;
};

// slab method
// https://tavianator.com/2022/ray_box_boundary.html
float aabbIntersect(Ray ray, vec3 aabbMin, vec3 aabbMax) {
	float tmin = 0.0f;
	float tmax = 9999.0f;
	for (int d = 0; d < 3; d++) {
        float t1 = (aabbMin[d] - ray.origin[d]) * ray.invDir[d];
        float t2 = (aabbMax[d] - ray.origin[d]) * ray.invDir[d];

        tmin = max(tmin, min(min(t1, t2), tmax));
        tmax = min(tmax, max(max(t1, t2), tmin));
    }
    return tmin < tmax ? tmin : 9999.0f;
};

hitData bvhIntersect(Ray ray, Model model) {
	hitData result;
	result.didHit = false;
	result.dist = 9999.0f;
	int stack[BVHStackSize];
	int index = 1;

	// root node
	stack[0] = model.BVHIndex;
	while (index > 0) {
		BVHNode node = nodes[stack[--index]];				

		// leaf node
		if (node.triCount > 0) {
			// triangle intersections
			for (int i = node.triIndex; i < node.triIndex + node.triCount; i++) {
				hitData hit = triangleIntersect(ray, triangles[nodeTriIndex[i]]);
				if (hit.didHit && hit.dist < result.dist) result = hit;

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
				if (leftDist < result.dist) stack[index++] = node.leftIndex;
				if (rightDist < result.dist) stack[index++] = node.leftIndex + 1;
			} 
			else {
				if (rightDist < result.dist) stack[index++] = node.leftIndex + 1;
				if (leftDist < result.dist) stack[index++] = node.leftIndex;
			}	

		}
	}


	return result;
};


hitData getCollision(Ray ray) {
	hitData result;
	result.didHit = false;
	result.dist = 9999.0f;
	hitData hit;

	// sphere intersections
	for (int i = 0; i < spheresLength; i++) {
		hit = sphereIntersect(ray, spheres[i]);
		if (hit.didHit) {
			if (hit.dist < result.dist) {
				result = hit;
				result.mat = materials[spheres[i].matIndex];
			}
		}
	}


	// save ray to restore from model space
	vec3 originalOrigin = ray.origin;
	vec3 originalDir = ray.dir;

	// model (bvh & triangle) intersections
	for (int i = 0; i < modelsLength; i++) {
		Model model = models[i];

		// transform ray to model space
		ray.origin = (model.invTransform * vec4(originalOrigin, 1.0f)).xyz;
		ray.dir = (model.invTransform * vec4(originalDir, 0.0f)).xyz;
		ray.invDir = 1.0f / ray.dir;

		// skips bvh intersection tests if closer found
		if (aabbIntersect(ray, nodes[model.BVHIndex].min, nodes[model.BVHIndex].max) >= result.dist)
			continue;

		hit = bvhIntersect(ray, model);
		if (hit.dist < result.dist) {
			result = hit;

			// transform result to world space
			result.pos = originalOrigin + originalDir * result.dist;
			result.normal = normalize((model.transform * vec4(hit.normal, 0.0f)).xyz);

			result.mat = materials[model.matIndex];
		}
	}

	// restore ray back to world space
	ray.dir = originalDir;

	return result;
};


vec3 randBounce(vec3 normal, inout uint seed) {
	vec3 bounce = vec3(1.0f);
	bounce.x = randomNormalDist(seed);
	bounce.y = randomNormalDist(seed);
	bounce.z = randomNormalDist(seed);

	// same hemisphere
	return normalize(bounce) * sign(dot(normal, bounce));
};


vec3 reflectRay(vec3 dir, vec3 normal) {
	return normalize(dir - 2.0f * dot(normal, dir) * normal);
};


vec3 environmentLight(Ray ray) {
	return vec3(0.4, 0.5, 0.9) * (ray.dir.y + 0.95);
}


// snells law, gives refracted angle
vec3 snells(vec3 dir, vec3 normal, float index, float cosine) {
	float value = 1 - index * index * (1 - cosine * cosine);
	// reflect
	if (value < 0.0f) return vec3(0);
	// refract
	return index * dir - (index * cosine - sqrt(value)) * normal;
}


// Schlicks approximation
float fresnel(float index, float cosine) {
        float r0 = (1 - index) / (1 + index);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow(1 - cosine, 5);
}


vec3 trace(Ray ray, inout uint seed) {
	vec3 color = vec3(1.0f);
	vec3 light = vec3(0.0f);
	hitData hit;

	for (int i = 0; i <= maxBounces; i++) {
		hit = getCollision(ray);
		if (hit.didHit) {
			Material mat = hit.mat;

			ray.origin = hit.pos;

			// transparent
			if (mat.glass) {
				float index = hit.inside ? 1.0f / mat.refraction : mat.refraction;
				float cosine = dot(-ray.dir, hit.normal);
				vec3 refractDir = snells(ray.dir, hit.normal, index, cosine);
				if (refractDir == vec3(0) || fresnel(index, cosine) > random(seed)) ray.dir = reflectRay(ray.dir, hit.normal);
				else ray.dir = refractDir;
				// blur
				ray.dir += (1 - mat.smoothness) * randBounce(hit.normal, seed);
			}

			// matt and reflective
			else {
				vec3 diffuseDir = randBounce(hit.normal, seed);
				// chance of diffuse or specular reflection
				if (random(seed) >= mat.gloss) ray.dir = diffuseDir;
				// specular reflection / blur
				else ray.dir = mat.smoothness * reflectRay(ray.dir, hit.normal) + (1 - mat.smoothness) * diffuseDir;
			}

			// light
			vec3 emmited = mat.lightColor * mat.lightStrength;
			light += emmited * color;
			color *= mat.color * dot(hit.normal, ray.dir);
			
		}
		else {
			if (environmentLightFlag) {
				light += environmentLight(ray) * color;
			}
			break;
		}
	}
	return light;
};
	

void main() {
	uint seed = uint((gl_FragCoord.y * res.x) + gl_FragCoord.x) + accumulationFrame * 20983476; // large arbitrary  number 
	float x = -res.x / 2.0f + gl_FragCoord.x;
	float y = -res.y / 2.0f + gl_FragCoord.y;
	
	Ray ray;
	ray.origin = camPos;
	ray.dir = normalize(vec3(x, y, focus));
	ray.dir = (model * vec4(ray.dir, 1.0f)).xyz;
	
	vec3 totalLight = vec3(0);

	for (int i = 0; i < rayCount; i++) {
		totalLight += trace(ray, seed);
	}
	totalLight /= rayCount;

	// pixel accumulation from previous frames
	float weight = 1.0f / accumulationFrame;
	totalLight = (1.0f - weight) * texture(text, texCoords).xyz + totalLight * weight;
	
	// final output color
	FragColor = vec4(totalLight, 1.0f);
	//FragColor = vec4(debugValues.x / 300.0f, debugValues.y / 100.0f, 0.0f, 1.0f);
};