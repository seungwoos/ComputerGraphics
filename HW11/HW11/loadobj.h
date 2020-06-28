#pragma once

#include "tiny_obj_loader.h"

bool load_obj(
	const char* filename,
	const char* basedir,
	std::vector<tinyobj::real_t>& vertices_out,
	std::vector<tinyobj::real_t>& normals_out,
	std::vector<std::vector<size_t>>& vertex_map,
	std::vector<std::vector<size_t>>& material_map,
	tinyobj::attrib_t& attrib,
	std::vector<tinyobj::shape_t>& shapes,
	std::vector<tinyobj::material_t>& materials,
	tinyobj::real_t scale = 1.0f);
