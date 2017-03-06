#pragma once

#include <glad/glad.h>

namespace qc
{

template <typename T>

//! NOT COPYABLE CLASS
class BufferObject
{

public:
	BufferObject() {};
	BufferObject(const std::vector<T>& data, const GLenum target = GL_ARRAY_BUFFER)
		: target(target), size(data.size())
	{initBufferObject(data);}

	~BufferObject()
		{if (pointer) glDeleteBuffers(1, &pointer);}

	BufferObject(const BufferObject<T>& o) = delete;
	BufferObject<T>& operator= (const BufferObject<T>& o) = delete;

	BufferObject(BufferObject<T>&& o)
		: target(o.target), size(o.size)
	{
		if (pointer) glDeleteBuffers(1, &pointer);
		pointer = o.pointer;
		o.pointer = 0;
	}

	BufferObject<T>& operator= (BufferObject<T>&& o)
	{
		if (pointer) glDeleteBuffers(1, &pointer);
		pointer = o.pointer;
		target = o.target;
		size = o.size;
		o.pointer = 0;
		return *this;
	}


	//-- GETTERS ----------------------------

	GLuint getPointer() const
		{return pointer;}

	size_t getSize() const
		{return size;}

private:
	//-- Pointer to the buffer
	GLuint pointer = 0;

	//-- Buffer type
	GLenum target;

	//-- Buffer size
	size_t size;

	//-- INIT BUFFER OBJECT -----------------
	/*
		Initialise the buffer
	*/
	void initBufferObject(const std::vector<T>& data)
	{
		glGenBuffers(1, &pointer);
		glBindBuffer(target, pointer);
		if(target == GL_ARRAY_BUFFER || target == GL_ELEMENT_ARRAY_BUFFER) glBufferStorage(target, size * sizeof(T), data.data(), 0);
		else if(target == GL_UNIFORM_BUFFER) glBufferData(target, size * sizeof(T), data.data(), GL_DYNAMIC_DRAW);
		else if(target == GL_SHADER_STORAGE_BUFFER) glBufferData(target, size * sizeof(T), data.data(), GL_DYNAMIC_COPY);
		glBindBuffer(target, 0);
	}
};

} //! namespace qc