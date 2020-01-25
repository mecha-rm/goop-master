#include "ObjLoader.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>
#include <regex>

#include "Logging.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/normal.hpp>

// Borrowed from https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
#pragma region String Trimming

// trim from start (in place)
static inline void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !std::isspace(ch);
		}));
}

// trim from end (in place)
static inline void rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string& s) {
	ltrim(s);
	rtrim(s);
}

#pragma endregion 

MeshData ObjLoader::LoadObj(const char* filename, glm::vec4 baseColor) {
	// Open our file in binary mode
	std::ifstream file;
	file.open(filename, std::ios::binary);

	// We'll just use a 3x3 matrix of integers to represent the face
	// Thanks to Myles for showing me this! It's an awesome idea!
	typedef glm::mat<3, 3, uint32_t> Face;

	// If our file fails to open, we will throw an error
	if (!file) {
		throw new std::runtime_error("Failed to open file");
	}

	LOG_TRACE("Loading mesh from '{}'", filename);

	// Declare vectors for our positions, normals, and face indices
	std::vector<glm::vec3>   positions;
	std::vector<glm::vec2>   texUvs;
	std::vector<glm::vec3>   normals;
	std::vector<Face>        faces;
	// Stores our w value if it is provided
	float garb;
	// Stores the line that we are operating on
	std::string line;

	// Regex for matching our face buckets
	// https://regex101.com/r/MEKPnK/2
	std::regex multiMatch(R"LIT((\d*)(?:\/(\d*)(?:\/(\d*))?)? (\d*)(?:\/(\d*)(?:\/(\d*))?)? (\d*)(?:\/(\d*)(?:\/(\d*))?)?)LIT");
	std::smatch match;

	// A cache for mapping face vertex indices to a mesh vertex index
	std::unordered_map<uint64_t, uint32_t> vectorCache;

	// Iterate as long as there is content to read
	while (std::getline(file, line)) {
		// v is our position
		if (line.substr(0, 2) == "v ") {
			// Read in the position and append it to our positions list
			std::istringstream ss = std::istringstream(line.substr(2));
			glm::vec3 pos; ss >> pos.x; ss >> pos.y; ss >> pos.z; ss >> garb;
			positions.push_back(pos);
		}
		// vn is our normals
		else if (line.substr(0, 3) == "vn ") {
			// Read in all the normals and append to list
			std::istringstream ss = std::istringstream(line.substr(3));
			glm::vec3 norm; ss >> norm.x; ss >> norm.y; ss >> norm.z;
			normals.push_back(norm);
		}
		// vt is our UV's 
		else if (line.substr(0, 3) == "vt ") {
			// Read in the coordinates and append to list
			std::istringstream ss = std::istringstream(line.substr(3));
			glm::vec2 uv; ss >> uv.x; ss >> uv.y;
			texUvs.push_back(uv);
		}
		// f is our faces
		else if (line.substr(0, 2) == "f ") {
			// We will start parsing with a string stream
			std::istringstream ss = std::istringstream(line.substr(2));
			
			// Our face starts as all zeros
			Face face = Face(0);

			// Remove the first 2 characters (the 'f ')
			line = line.substr(2);
			// Trim any leftover whitespace characters
			rtrim(line);

			// Run the regex on the line
			if (std::regex_match(line, match, multiMatch)) {
				// Iterate over 3 vertices
				for (uint32_t ix = 0; ix < 3; ix++) {
					GLuint vInd{ 0 }, tInd{ 0 }, nInd{ 0 };
					
					// Assume that we always have the position 
					vInd = atoi(match[(ix * 3u) + 1u].str().c_str());
					// Check our texture bucket, get the index if it's set
					if (match[(ix * 3u) + 2u].matched)
						tInd = atoi(match[(ix * 3ul) + 2u].str().c_str());  
					// Check our normal bucket, get the index if it's set
					if (match[(ix * 3u) + 3u].matched)
						nInd = atoi(match[(ix * 3u) + 3u].str().c_str());

					// Convert to our index space and store in the face
					vInd--; tInd--; nInd--;
					face[ix][0] = vInd; face[ix][1] = tInd; face[ix][2] = nInd;
				}
			}
			// If our regex did not match, fail
			else {
				LOG_ASSERT(false, "Cannot parse face!");
			}
			// Once we have face index data, add it to our list to be processed
			faces.push_back(face);
		}
	}

	LOG_TRACE("\tLoaded data, starting post-processing");

	// Allocate a new array for our vertices
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	
	// Iterate over all the positions we've read
	for (auto& face : faces) {
		for (int jx = 0; jx < 3; jx++) {
			auto& aSet = face[jx];
			// We will use our mask to only select the lowest 21 bits of each index
			uint64_t mask = 0b0'000000000000000000000'000000000000000000000'111111111111111111111;
			uint64_t key = 0;
			
			// We generate a key using our vertex's position, texture, and normal indices
			key = ((aSet[0] & mask) << 42) | ((aSet[1] & mask) << 21) | (aSet[2] & mask);

			// Search the cache for the key
			auto it = vectorCache.find(key);

			// If it exists, we push the index to our indices
			if (it != vectorCache.end())
				indices.push_back(it->second);
			// Otherwise, we need to create a new vertex
			else
			{
				// Load the vertex from the attributes
				Vertex vertex;
				vertex.Position = 
					aSet[0] != (uint32_t)-1 ? 
						positions[aSet[0]] :
						glm::vec3(0);
				vertex.Color = baseColor;
				vertex.UV = 
					aSet[1] != (uint32_t)-1 ? 
						texUvs[aSet[1]] :
						glm::vec2(0.0f);
				vertex.Normal = 
					aSet[2] != (uint32_t)-1 ? 
						normals[aSet[2]] :
						glm::triangleNormal(positions[face[0][0]], positions[face[1][0]], positions[face[2][0]]);
				// Add the index of the new vertex to the cache
				vectorCache[key] = static_cast<uint32_t>(vertices.size());
				// Add the index of the new vertex to our indices
				indices.push_back(static_cast<uint32_t>(vertices.size()));
				// Add the vertex to the buffer
				vertices.push_back(vertex);

			}
		}
	}

	// Compute our TBN matrices for normal mapping

	// Create and return a result as a meshBuilder mesh data object
	auto result = MeshData();
	result.Vertices = vertices;
	result.Indices  = indices;
	return result;
}
