#ifndef SSBO_H
#define SSBO_H

#include <glad/glad.h>
#include <vector>

class SSBO {

public:
	GLuint ID{};

	SSBO(const GLuint binding) {
		this->binding = binding;
		glCreateBuffers(1, &ID);
	}

	template <typename T>
	void update(const std::vector<T>& data) const {
		if (data.empty()) return;
		glNamedBufferData(ID, sizeof(T) * data.size(), data.data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ID);
	}

	void deleteSSBO() const {
		glDeleteBuffers(1, &ID);
	}

private:
	GLuint binding;
};


#endif