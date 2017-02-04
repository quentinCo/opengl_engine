#pragma once

#include <glad/glad.h>

namespace qc
{

template <typename T>

class BufferObject
{

public:
	BufferObject() {};
	BufferObject(const std::vector<T>& data, const GLenum target = GL_ARRAY_BUFFER)
		: target(target), size(data.size())
	{
		initBufferObject(data);
	}

	~BufferObject()
	{
		if (pointer) glDeleteBuffers(1, &pointer); 
	}

	BufferObject(const BufferObject<T>& o) = delete;
	BufferObject<T>& operator= (const BufferObject<T>& o) = delete;

	BufferObject(BufferObject<T>&& o)
		: pointer(o.pointer), target(o.target), size(o.size)
	{
		o.pointer = 0;
	}

	BufferObject<T>& operator= (BufferObject<T>&& o)
	{
		pointer = o.pointer;
		target = o.target;
		size = o.size;
		o.pointer = 0;
		return *this;
	}

	GLuint getPointer() const
		{return pointer;}

	size_t getSize() const
		{return size;}

private:
	GLuint pointer;
	GLenum target;
	size_t size;

	void initBufferObject(const std::vector<T>& data)
	{
		glGenBuffers(1, &pointer);
		glBindBuffer(target, pointer);
		glBufferStorage(target, size * sizeof(T), data.data(), 0);
		glBindBuffer(target, 0);
	}
};

} // namespace qc