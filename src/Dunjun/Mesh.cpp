#include <Dunjun/Mesh.hpp>

namespace Dunjun
{
void Mesh::Data::generateNormals()
{
	for (u32 i{0}; i < indices.size(); i += 3)
	{
		Vertex& v0{vertices[indices[i + 0]]};
		Vertex& v1{vertices[indices[i + 1]]};
		Vertex& v2{vertices[indices[i + 2]]};

		Vector3 a{v1.position - v0.position};
		Vector3 b{v2.position - v1.position};

		Vector3 normal = normalize(cross(a, b));

		v0.normal += normal;
		v1.normal += normal;
		v2.normal += normal;
	}

	for (usize i{0}; i < vertices.size(); i++)
		vertices[i].normal = normalize(vertices[i].normal);
}

Mesh::Mesh()
: m_data{}
, m_generated{false}
, m_vbo{0}
, m_ibo{0}
, m_drawType{GL_TRIANGLES}
, m_drawCount{0}
{
}

Mesh::Mesh(const Data& data)
: m_data{data}
, m_generated{false}
, m_vbo{0}
, m_ibo{0}
, m_drawType{data.drawType}
, m_drawCount{data.indices.size()}
{
	generate();
}

void Mesh::addData(const Data& data)
{
	m_data = data;
	m_drawType = data.drawType;
	m_drawCount = data.indices.size();
	m_generated = false;
}

void Mesh::generate() const
{
	if (m_generated)
		return;

	if (!m_vbo)
		glGenBuffers(1, &m_vbo);
	if (!m_ibo)
		glGenBuffers(1, &m_ibo);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER,
	             sizeof(Vertex) * m_data.vertices.size(),
	             &m_data.vertices[0],
	             GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	             sizeof(u32) * m_data.indices.size(),
	             &m_data.indices[0],
	             GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_generated = true;
}

void Mesh::draw() const
{
	if (!m_generated)
		generate();

	glEnableVertexAttribArray((u32)AtrribLocation::Position);
	glEnableVertexAttribArray((u32)AtrribLocation::TexCoord);
	glEnableVertexAttribArray((u32)AtrribLocation::Color);
	glEnableVertexAttribArray((u32)AtrribLocation::Normal);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

	glVertexAttribPointer((u32)AtrribLocation::Position,
	                      3,
	                      GL_FLOAT,
	                      GL_FALSE,
	                      sizeof(Vertex), // Stride
	                      (const GLvoid*)(0));
	glVertexAttribPointer((u32)AtrribLocation::TexCoord,
	                      2,
	                      GL_FLOAT,
	                      GL_FALSE,
	                      sizeof(Vertex), // Stride
	                      (const GLvoid*)(sizeof(Vector3)));
	glVertexAttribPointer((u32)AtrribLocation::Color,
	                      4,
	                      GL_UNSIGNED_BYTE, // 0-255 => 0-1
	                      GL_TRUE,
	                      sizeof(Vertex), // Stride
	                      (const GLvoid*)(sizeof(Vector3) + sizeof(Vector2)));
	glVertexAttribPointer(
	    (u32)AtrribLocation::Normal,
	    3,
	    GL_FLOAT,
	    GL_FALSE,
	    sizeof(Vertex), // Stride
	    (const GLvoid*)(sizeof(Vector3) + sizeof(Vector2) + sizeof(Color)));

	glDrawElements(m_drawType, m_drawCount, GL_UNSIGNED_INT, nullptr);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray((u32)AtrribLocation::Position);
	glDisableVertexAttribArray((u32)AtrribLocation::TexCoord);
	glDisableVertexAttribArray((u32)AtrribLocation::Color);
	glDisableVertexAttribArray((u32)AtrribLocation::Normal);
}

} // namespace Dunjun
