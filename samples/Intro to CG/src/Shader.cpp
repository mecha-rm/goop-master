#include "Shader.h"
#include "Logging.h"
#include <stdexcept>
#include <fstream>
#include <filesystem>

// Reads the entire contents of a file
char* readFile(const char* filename) {
	// Declare and open the file stream
	std::ifstream file;
	file.open(filename, std::ios::binary);

	// Only read if the file is open
	if (file.is_open()) {
		// Get the starting location in the file
		uint64_t fileSize = file.tellg();
		// Seek to the end
		file.seekg(0, std::ios::end);
		// Calculate the file size from end to beginning
		fileSize = (uint64_t)file.tellg() - fileSize;
		// Seek back to the beginning of the file
		file.seekg(0, std::ios::beg);

		// Allocate space for our entire file, +1 byte at the end for null terminator
		char* result = new char[fileSize + 1];
		// Read the entire file to our memory
		file.read(result, fileSize);

		// Make our text null-terminated
		result[fileSize] = '\0';

		// Close the file before returning
		file.close();
		return result;

	}
	// Otherwise, we failed to open our file, throw a runtime error
	else {
		throw std::runtime_error("We cannot open the file!");
	}
}


Shader::Shader() {
	myRenderhandle = glCreateProgram();
}

Shader::~Shader() {
	glDeleteProgram(myRenderhandle);
}

void Shader::Compile(const char* vs_source, const char* vsName, const char* fs_source, const char* fsName) {
	// Compile our two shader programs
	GLuint vs = __CompileShaderPart(vs_source, GL_VERTEX_SHADER);
	glObjectLabel(GL_SHADER, vs, -1, vsName);
	GLuint fs = __CompileShaderPart(fs_source, GL_FRAGMENT_SHADER);
	glObjectLabel(GL_SHADER, fs, -1, fsName);

	// Attach our two shaders
	glAttachShader(myRenderhandle, vs);
	glAttachShader(myRenderhandle, fs);

	// Perform linking
	glLinkProgram(myRenderhandle);

	// Remove shader parts to save space
	glDetachShader(myRenderhandle, vs);
	glDeleteShader(vs);
	glDetachShader(myRenderhandle, fs);
	glDeleteShader(fs);

	// Get whether the link was successful
	GLint success = 0;
	glGetProgramiv(myRenderhandle, GL_LINK_STATUS, &success);

	// If not, we need to grab the log and throw an exception
	if (success == GL_FALSE) {
		// Get the length of the log
		GLint length = 0;
		glGetProgramiv(myRenderhandle, GL_INFO_LOG_LENGTH, &length);

		if (length > 0) {
			// Read the log from openGL
			char* log = new char[length];
			glGetProgramInfoLog(myRenderhandle, length, &length, log);
			LOG_ERROR("Shader failed to link:\n{}", log);
			delete[] log;
		}
		else {
			LOG_ERROR("Shader failed to link for an unknown reason!");
		}

		// Delete the partial program
		glDeleteProgram(myRenderhandle);

		// Throw a runtime exception
		throw new std::runtime_error("Failed to link shader program!");
	}
	else {
		LOG_TRACE("Shader has been linked");
	}
}

void Shader::Load(const char* vsFile, const char* fsFile)
{
	// Load in our shaders
	char* vs_source = readFile(vsFile);
	char* fs_source = readFile(fsFile);

	// Compile our program
	Compile(vs_source, vsFile, fs_source, fsFile);

	SetDebugName(std::filesystem::path(vsFile).filename().string() + " | " + std::filesystem::path(fsFile).filename().string());

	// Clean up our memory
	delete[] fs_source;
	delete[] vs_source;
}

void Shader::SetUniform(const char* name, const glm::mat4& value) {
	GLint loc = glGetUniformLocation(myRenderhandle, name);
	if (loc != -1) {
		glProgramUniformMatrix4fv(myRenderhandle, loc, 1, false, &value[0][0]);
	}
}

void Shader::SetUniform(const char* name, const glm::vec4& value) {
	GLint loc = glGetUniformLocation(myRenderhandle, name);
	if (loc != -1) {
		glProgramUniform4fv(myRenderhandle, loc, 1, &value[0]);
	}
}

void Shader::SetUniform(const char* name, const glm::mat3& value) {
	GLint loc = glGetUniformLocation(myRenderhandle, name);
	if (loc != -1) {
		glProgramUniformMatrix3fv(myRenderhandle, loc, 1, false, &value[0][0]);
	}
}
void Shader::SetUniform(const char* name, const glm::vec3& value) {
	GLint loc = glGetUniformLocation(myRenderhandle, name);
	if (loc != -1) {
		glProgramUniform3fv(myRenderhandle, loc, 1, &value[0]);
	}
}
void Shader::SetUniform(const char* name, const glm::vec2& value) {
	GLint loc = glGetUniformLocation(myRenderhandle, name);
	if (loc != -1) {
		glProgramUniform2fv(myRenderhandle, loc, 1, &value[0]);
	}
}
void Shader::SetUniform(const char* name, const float& value) {
	GLint loc = glGetUniformLocation(myRenderhandle, name);
	if (loc != -1) {
		glProgramUniform1fv(myRenderhandle, loc, 1, &value);
	}
}

void Shader::SetUniform(const char* name, const int& value) {
	GLint loc = glGetUniformLocation(myRenderhandle, name);
	if (loc != -1) {
		glProgramUniform1iv(myRenderhandle, loc, 1, &value);
	}
}

void Shader::Bind() {
	glUseProgram(myRenderhandle);
}

GLuint Shader::__CompileShaderPart(const char* source, GLenum type) {
	GLuint result = glCreateShader(type);

	// Load in our shader source and compile it
	glShaderSource(result, 1, &source, NULL);
	glCompileShader(result);

	// Check our compile status
	GLint compileStatus = 0;
	glGetShaderiv(result, GL_COMPILE_STATUS, &compileStatus);

	// If we failed to compile
	if (compileStatus == GL_FALSE) {
		// Get the size of the error log
		GLint logSize = 0;
		glGetShaderiv(result, GL_INFO_LOG_LENGTH, &logSize);

		// Create a new character buffer for the log
		char* log = new char[logSize];

		// Get the log
		glGetShaderInfoLog(result, logSize, &logSize, log);

		// Dump error log
		LOG_ERROR("Failed to compile shader part:\n{}", log);

		// Clean up our log memory
		delete[] log;

		// Delete the broken shader result
		glDeleteShader(result);

		// Throw a runtime exception
		throw new std::runtime_error("Failed to compile shader part!");
	}
	else {
		LOG_TRACE("Shader part has been compiled!");
	}

	// Return the compiled shader part
	return result;
}
