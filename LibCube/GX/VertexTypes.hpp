#pragma once

namespace libcube { namespace gx {

union VertexComponentCount
{
	
	enum class Position
	{
		xy,
		xyz
	} position;
	enum class Normal
	{
		xyz,
		nbt, // index NBT triplets
		nbt3 // index N/B/T individually
	} normal;
	enum class Color
	{
		rgb,
		rgba
	} color;
	enum class TextureCoordinate
	{
		s,
		st,

		u = s,
		uv = st
	} texcoord;

	explicit VertexComponentCount(Position p)
		: position(p)
	{}
	explicit VertexComponentCount(Normal n)
		: normal(n)
	{}
	explicit VertexComponentCount(Color c)
		: color(c)
	{}
	explicit VertexComponentCount(TextureCoordinate u)
		: texcoord(u)
	{}
	VertexComponentCount() {}
};

union VertexBufferType
{
	enum class Generic
	{
		u8,
		s8,
		u16,
		s16,
		f32
	} generic;
	enum class Color
	{
		rgb565, //!< R5G6B5
		rgb8, //!< 8 bit, no alpha
		rgbx8,  //!< 8 bit, alpha discarded
		rgba4, //!< 4 bit
		rgba6, //!< 6 bit
		rgba8, //!< 8 bit, alpha

		FORMAT_16B_565 = 0,
		FORMAT_24B_888 = 1,
		FORMAT_32B_888x = 2,
		FORMAT_16B_4444 = 3,
		FORMAT_24B_6666 = 4,
		FORMAT_32B_8888 = 5,
	} color;

	explicit VertexBufferType(Generic g)
		: generic(g)
	{}
	explicit VertexBufferType(Color c)
		: color(c)
	{}
	VertexBufferType() {}
};

// Subset of vertex attributes valid for a buffer.
enum class VertexBufferAttribute : u32
{
	Position = 9,
	Normal,
	Color0,
	Color1,
	TexCoord0,
	TexCoord1,
	TexCoord2,
	TexCoord3,
	TexCoord4,
	TexCoord5,
	TexCoord6,
	TexCoord7,

	NormalBinormalTangent = 25,

	Undefined = 0xff-1,
	Terminate = 0xff
};

} } // namespace libcube::gx
