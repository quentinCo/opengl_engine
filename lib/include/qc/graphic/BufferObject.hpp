#pragma once

#include <glad/glad.h>

namespace qc
{

namespace graphic
{

template <typename T>

//! NOT COPYABLE CLASS
class BufferObject
{

public:
	BufferObject() {};
	BufferObject(const std::vector<T>& data, const GLenum target = GL_ARRAY_BUFFER, const GLenum usageFlags = 0)
		: target(target), size(data.size()), usageFlags(usageFlags)
	{initBufferObject(&data);}

	BufferObject(size_t size, const GLenum target = GL_ARRAY_BUFFER, const GLenum usageFlags = 0)
		: target(target), size(size), usageFlags(usageFlags)
	{initBufferObject(nullptr);}

	~BufferObject()
		{if (pointer) glDeleteBuffers(1, &pointer);}

	BufferObject(const BufferObject<T>& o) = delete;
	BufferObject<T>& operator= (const BufferObject<T>& o) = delete;

	BufferObject(BufferObject<T>&& o)
		: target(o.target), size(o.size), usageFlags(o.usageFlags)
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
		usageFlags = o.usageFlags;
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

	GLenum usageFlags;

	//-- Buffer size
	size_t size;

	//-- INIT BUFFER OBJECT -----------------
	/*
		Initialise the buffer
	*/
	void initBufferObject(const std::vector<T>* data)
	{
		const T* dataPointer = (data) ? data->data() : nullptr;

		glGenBuffers(1, &pointer);
		glBindBuffer(target, pointer);
		if(target == GL_ARRAY_BUFFER || target == GL_ELEMENT_ARRAY_BUFFER)
			glBufferStorage(target, size * sizeof(T), dataPointer, usageFlags);
		else
			glBufferData(target, size * sizeof(T), dataPointer, usageFlags);
		// TODO : create another class for ubo and ssbo
		glBindBuffer(target, 0);
	}
};

} //! namespace graphic

} //! namespace qc