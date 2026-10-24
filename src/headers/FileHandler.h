#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include<nfd.h>
#include<tinyobjloader/tiny_obj_loader.h>
#include<string>
#include<vector>
#include<fstream>
#include"headers/Object.h"

static void saveImage(const std::string& filePath, int width, int height) {
	void* data = malloc(width * height * 3);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_flip_vertically_on_write(true);
	stbi_write_png(filePath.c_str(), width, height, 3, data, width * 3);
	free(data);

	// opens image in default program
	std::system(filePath.c_str());
}


static std::string getSavePath(std::string defaultPath, int mode) {
	NFD_Init();
	nfdchar_t* path = nullptr;
	nfdsavedialogu8args_t args = {};
	args.defaultName = defaultPath.c_str();
	nfdresult_t result = {};

	switch (mode) {
		// image
	case 0: {
		nfdfilteritem_t filterList[1] = {{"image", "png"}};
		args.filterList = filterList;
		args.filterCount = 1;
		result = NFD_SaveDialogU8_With(&path, &args);
		break;
		}
		
		// scene
	case 1: {
		nfdfilteritem_t filterList[1] = {{"scene", "ray"}};
		args.filterList = filterList;
		args.filterCount = 1;
		result = NFD_SaveDialogU8_With(&path, &args);
		break;
		}
	}
	

	if (result == NFD_OKAY) {
		defaultPath = path;
		NFD_FreePath(path);

		switch (mode) {
		case 0:
			if (defaultPath.substr(defaultPath.find_last_of('.')) != ".png") defaultPath += ".png";
			break;
		case 1:
			if (defaultPath.substr(defaultPath.find_last_of('.')) != ".ray") defaultPath += ".ray";
			break;
		}

	}

	// closed or error
	else defaultPath = "";

	NFD_Quit();
	return defaultPath;
}


static std::string getLoadPath(int mode) {
	NFD_Init();
	nfdchar_t* path = nullptr;
	std::string loadPath;
	nfdopendialogu8args_t args = {};
	nfdresult_t result = {};

	switch (mode) {
		// model
	case 0: {
		nfdfilteritem_t filterList[1] = {{"model", "obj"}};
		args.filterList = filterList;
		args.filterCount = 1;

		result = NFD_OpenDialogU8_With(&path, &args);
		break;
	}

		// scene
	case 1: {
		nfdfilteritem_t filterList[1] = {{"scene", "ray"}};
		args.filterList = filterList;
		args.filterCount = 1;

		result = NFD_OpenDialogU8_With(&path, &args);
		break;
		}
	}
	
	if (result == NFD_OKAY) {
		loadPath = path;
		NFD_FreePath(path);
	}
	// closed or error
	else loadPath = "";

	NFD_Quit();
	return loadPath;
}


static bool saveScene(const std::string& filePath, std::vector<Material> materials, std::vector<Sphere> spheres) {
	std::ofstream file;
	file.open(filePath);

	if (!file.is_open()) return false;

	file << "Materials: " << materials.size() << "\n";
	file << "Spheres: " << spheres.size() << "\n\n";

	for (int i = 0; i < materials.size(); i++) {
		Material mat = materials[i];
		file << "Material: " << i << "\n";
		file << "Colour: " << mat.color.x << " " << mat.color.y << " " << mat.color.z << "\n";
		file << "Light Strength: " << mat.lightStrength << "\n";
		file << "Light Colour: " << mat.lightColor.x << " " << mat.lightColor.y << " " << mat.lightColor.z << "\n";
		file << "Smoothness: " << mat.smoothness << "\n";
		file << "Gloss: " << mat.gloss << "\n";
		file << "Glass: " << mat.glass << "\n";
		file << "Refraction: " << mat.refraction << "\n\n";
	}

	for (int i = 0; i < spheres.size(); i++) {
		Sphere sphere = spheres[i];
		file << "Sphere: " << i << "\n";
		file << "Position: " << sphere.pos.x << " " << sphere.pos.y << " " << sphere.pos.z << "\n";
		file << "Radius: " << sphere.radius << "\n";
		file << "Material: " << sphere.matIndex << "\n\n";
	}

	file.close();
	return true;
}

static bool loadScene(const std::string& filePath, std::vector<Material>& materials, std::vector<Sphere>& spheres) {
	std::ifstream file;
	file.open(filePath);
	if (!file.is_open()) return false;
	std::string line;

	std::vector<Material> fileMaterials;
	std::vector<Sphere> fileSpheres;

	int matSize;
	int spheresSize;

	file >> line; // materials
	file >> matSize; // int
	file >> line; // spheres
	file >> spheresSize; // int


	for (int i = 0; i < matSize; i++) {
		Material mat;
		file >> line; // material
		file >> line; // material num
		file >> line; // colour
		file >> mat.color.x >> mat.color.y >> mat.color.z; // floats
		file >> line; // light
		file >> line; // strength
		file >> mat.lightStrength; // float
		file >> line; // light
		file >> line; // colour
		file >> mat.lightColor.x >> mat.lightColor.y >> mat.lightColor.z; // floats
		file >> line; //smoothness
		file >> mat.smoothness; // float
		file >> line; // gloss
		file >> mat.gloss; // float
		file >> line; // glass
		file >> mat.glass; // float
		file >> line; // refraction
		file >> mat.refraction; // float
		fileMaterials.push_back(mat);
	}
	
	for (int i = 0; i < spheresSize; i++) {
		Sphere sphere;
		file >> line; // sphere
		file >> line; // sphere num
		file >> line; // position
		file >> sphere.pos.x >> sphere.pos.y >> sphere.pos.z; // floats
		file >> line; // radius
		file >> sphere.radius; // float
		file >> line; // material
		file >> sphere.matIndex; // int
		fileSpheres.push_back(sphere);
	}

	file.close();

	if (fileMaterials.empty()) fileMaterials.push_back(Material{});

	materials = fileMaterials;
	spheres = fileSpheres;
	return true;
}

static bool loadModel(const std::string& filePath, std::vector<Model>& models, std::vector<ModelExtra>& modelExtras, std::vector<Triangle>& triangles) {
	tinyobj::ObjReaderConfig config;
	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(filePath, config)) {
		return false;
	}

	tinyobj::attrib_t attrib = reader.GetAttrib();
	std::vector<tinyobj::shape_t> shapes = reader.GetShapes();
	Model model;
	model.startIndex = triangles.size();

	for (tinyobj::shape_t & shape : shapes) {
		size_t offset = 0;

		// loop over faces (triangles)
		for (size_t face = 0; face < shape.mesh.num_face_vertices.size(); face++) {
			tinyobj::index_t index{};
			Triangle tri;


			// vertex A
			index = shape.mesh.indices[offset];

			tri.posA.x = attrib.vertices[3 * size_t(index.vertex_index)];
			tri.posA.y = attrib.vertices[3 * size_t(index.vertex_index) + 1];
			tri.posA.z = attrib.vertices[3 * size_t(index.vertex_index) + 2];

			// no normal if negative
			if (index.normal_index >= 0) {
				tri.normA.x = attrib.normals[3 * size_t(index.normal_index)];
				tri.normA.y = attrib.normals[3 * size_t(index.normal_index) + 1];
				tri.normA.z = attrib.normals[3 * size_t(index.normal_index) + 2];
			}


			// vertex B
			index = shape.mesh.indices[offset + 1];

			tri.posB.x = attrib.vertices[3 * size_t(index.vertex_index)];
			tri.posB.y = attrib.vertices[3 * size_t(index.vertex_index) + 1];
			tri.posB.z = attrib.vertices[3 * size_t(index.vertex_index) + 2];

			// no normal if negative
			if (index.normal_index >= 0) {
				tri.normB.x = attrib.normals[3 * size_t(index.normal_index)];
				tri.normB.y = attrib.normals[3 * size_t(index.normal_index) + 1];
				tri.normB.z = attrib.normals[3 * size_t(index.normal_index) + 2];
			}


			// vertex C
			index = shape.mesh.indices[offset + 2];

			tri.posC.x = attrib.vertices[3 * size_t(index.vertex_index)];
			tri.posC.y = attrib.vertices[3 * size_t(index.vertex_index) + 1];
			tri.posC.z = attrib.vertices[3 * size_t(index.vertex_index) + 2];

			// no normal if negative
			if (index.normal_index >= 0) {
				tri.normC.x = attrib.normals[3 * size_t(index.normal_index)];
				tri.normC.y = attrib.normals[3 * size_t(index.normal_index) + 1];
				tri.normC.z = attrib.normals[3 * size_t(index.normal_index) + 2];
			}

			triangles.push_back(tri);
			offset += 3;
		}
	}
	
	model.endIndex = triangles.size() - 1;
	models.push_back(model);
	modelExtras.push_back(ModelExtra{});
	return true;
}

#endif