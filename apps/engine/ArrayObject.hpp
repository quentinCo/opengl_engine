#pragma once

#include <glad/glad.h>

#include "BufferObject.hpp"

namespace qc
{

template <typename T>

class ArrayObject
{

public:
	using VBO = const BufferObject<T>&;
	using IBO = const BufferObject<uint32_t>&;

	ArrayObject() {}

	ArrayObject(VBO vbo, IBO ibo)
	{
		initArrayObject(vbo, ibo);
	}

	~ArrayObject()
	{
		if (pointer) glDeleteBuffers(1, &pointer);
	}

	ArrayObject(const ArrayObject<T>& o) = delete;
	ArrayObject<T>& operator= (const ArrayObject<T>& o) = delete;

	ArrayObject(ArrayObject<T>&& o)
		: pointer(o.pointer)
	{
		o.pointer = 0;
	}

	ArrayObject<T>& operator= (ArrayObject<T>&& o)
	{
		pointer = o.pointer;
		o.pointer = 0;
		return *this;
	}

	GLuint getPointer() const
		{return pointer;}

private:
	GLuint pointer;

	void initArrayObject(VBO vbo, IBO ibo)
	{
		glGenVertexArrays(1, &pointer);
		glBindVertexArray(pointer);

		const GLint positionAttrLocation = 0;
		const GLint normalAttrLocation = 1;
		const GLint texCoordsAttrLocation = 2;

		glEnableVertexAttribArray(positionAttrLocation);
		glEnableVertexAttribArray(normalAttrLocation);
		glEnableVertexAttribArray(texCoordsAttrLocation);

		glBindBuffer(GL_ARRAY_BUFFER, vbo.getPointer());

		glVertexAttribPointer(positionAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(T), (const GLvoid*)offsetof(T, position));
		glVertexAttribPointer(normalAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(T), (const GLvoid*)offsetof(T, normal));
		glVertexAttribPointer(texCoordsAttrLocation, 2, GL_FLOAT, GL_FALSE, sizeof(T), (const GLvoid*)offsetof(T, texCoords));

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo.getPointer());

		glBindVertexArray(0);
	}
};

}