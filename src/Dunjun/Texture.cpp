#include <Dunjun/Texture.hpp>

#include <Dunjun/Common.hpp>

#include <cassert>

namespace Dunjun
{
INTERNAL GLenum getInteralFormat(ImageFormat format, bool srgb)
{
	switch (format)
	{
	case ImageFormat::Greyscale:
		return GL_LUMINANCE;
	case ImageFormat::GreyscaleAlpha:
		return GL_LUMINANCE_ALPHA;
	case ImageFormat::RGB:
		return (srgb ? GL_SRGB : GL_RGB);
	case ImageFormat::RGBA:
		return (srgb ? GL_SRGB_ALPHA : GL_RGBA);

	default:
	case ImageFormat::None:
		throw std::runtime_error("Non-valid ImageFormat.");
		return 0;
	}
}

Texture::Texture()
: m_object{0}
, m_width{0}
, m_height{0}
{
}

Texture::Texture(const Image& image,
                 TextureFilter minMagFilter,
                 TextureWrapMode wrapMode)
: m_object{0}
, m_width{image.getWidth()}
, m_height{image.getHeight()}
{
	if (!loadFromImage(image, minMagFilter, wrapMode))
		throw std::runtime_error("Could not create texture from image.");
}

bool Texture::loadFromFile(const std::string& filename,
                           TextureFilter minMagFilter,
                           TextureWrapMode wrapMode)
{
	Image image;
	if (!image.loadFromFile(filename))
		return false;
	image.flipVertically();

	return loadFromImage(image, minMagFilter, wrapMode);
}

bool Texture::loadFromImage(const Image& image,
                            TextureFilter minMagFilter,
                            TextureWrapMode wrapMode)
{
	if (image.getFormat() == ImageFormat::None)
		return false;

	m_width = image.getWidth();
	m_height = image.getHeight();

	if (!m_object)
		glGenTextures(1, &m_object);

	glBindTexture(GL_TEXTURE_2D, m_object);
	glTexParameteri(
	    GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(wrapMode));
	glTexParameteri(
	    GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(wrapMode));
	glTexParameteri(
	    GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(minMagFilter));
	glTexParameteri(
	    GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(minMagFilter));

	glTexImage2D(GL_TEXTURE_2D,
	             0,
	             getInteralFormat(image.getFormat(), true),
	             m_width,
	             m_height,
	             0,
	             getInteralFormat(image.getFormat(), false),
	             GL_UNSIGNED_BYTE,
	             image.getPixels());

	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

Texture::~Texture()
{
	if (m_object)
		glDeleteTextures(1, &m_object);
}

void Texture::bind(const Texture* tex, GLuint position)
{
	if (position > 31)
	{
		std::cerr << "Textures can only be bount to position [0 ... 31]\n";
		std::cerr << "Will bind to position [31]" << std::endl;
		position = 31;
	}

	glActiveTexture(GL_TEXTURE0 + position);
	glClientActiveTexture(GL_TEXTURE0 + position);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, (tex && tex->m_object) ? tex->m_object : 0);
	glDisable(GL_TEXTURE_2D);
}

s32 Texture::getWidth() const { return m_width; }

s32 Texture::getHeight() const { return m_height; }

GLuint Texture::getNativeHandle() const { return m_object; }
} // namespace Dunjun
