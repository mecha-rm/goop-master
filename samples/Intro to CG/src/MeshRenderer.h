#pragma once
#include "Material.h"
#include "Mesh.h"

struct MeshRenderer {
	Material::Sptr Material;
	Mesh::Sptr     Mesh;
};