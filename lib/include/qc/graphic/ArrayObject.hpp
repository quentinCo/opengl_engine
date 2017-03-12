#pragma once

#include <glad/glad.h>

#include "BufferObject.hpp"

namespace qc
{

namespace graphic
{

template <typename T>

//! NOT COPYABLE CLASS
class ArrayObject
{

public:
	//-- Alias 
	using VBO = const BufferObject<T>&;
	using IBO = const BufferObject<uint32_t>&;

	ArrayObject() {}

	ArrayObject(VBO vbo, IBO ibo)
		{initArrayObject(vbo, ibo);}

	~ArrayObject()
		{if (pointer) glDeleteBuffers(1, &pointer);}

	ArrayObject(const ArrayObject<T>& o) = delete;
	ArrayObject<T>& operator= (const ArrayObject<T>& o) = delete;

	ArrayObject(ArrayObject<T>&& o)
	{
		if (pointer) glDeleteBuffers(1, &pointer);
		pointer = o.pointer;
		o.pointer = 0;
	}

	ArrayObject<T>& operator= (ArrayObject<T>&& o)
	{
		if (pointer) glDeleteBuffers(1, &pointer);
		pointer = o.pointer;
		o.pointer = 0;
		return *this;
	}

	//-- GETTERS ----------------------------
	GLuint getPointer() const
		{return pointer;}

private:
	//-- Pointer to the buffer (vao) 
	GLuint pointer = 0;

	//-- INIT ARRAY OBJECT ------------------
	/*
		Initialition of the array object
	*/
	void initArrayObject(VBO vbo, IBO ibo)
	{
		glGenVertexArrays(1, &pointer);
		glBindVertexArray(pointer);

		const GLint positionAttrLocation = 0;
		const GLint normalAttrLocation = 1;
		const GLint texCoordsAttrLocation = 2;
		const GLint tangentAttrLocation = 3;
		const GLint bitanAttrLocation = 4;

		glEnableVertexAttribArray(positionAttrLocation);
		glEnableVertexAttribArray(normalAttrLocation);
		glEnableVertexAttribArray(texCoordsAttrLocation);
		glEnableVertexAttribArray(tangentAttrLocation);
		glEnableVertexAttribArray(bitanAttrLocation);

		glBindBuffer(GL_ARRAY_BUFFER, vbo.getPointer());

		glVertexAttribPointer(positionAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(T), (const GLvoid*)offsetof(T, position));
		glVertexAttribPointer(normalAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(T), (const GLvoid*)offsetof(T, normal));
		glVertexAttribPointer(texCoordsAttrLocation, 2, GL_FLOAT, GL_FALSE, sizeof(T), (const GLvoid*)offsetof(T, texCoords));
		glVertexAttribPointer(tangentAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(T), (const GLvoid*)offsetof(T, tangent));
		glVertexAttribPointer(bitanAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(T), (const GLvoid*)offsetof(T, bitangent));

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo.getPointer());

		glBindVertexArray(0);
	}
};

} // namespace graphic

} // namespace qc