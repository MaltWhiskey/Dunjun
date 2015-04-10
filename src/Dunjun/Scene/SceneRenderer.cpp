#include <Dunjun/Scene/SceneRenderer.hpp>

#include <Dunjun/Scene/SceneNode.hpp>
#include <Dunjun/Scene/MeshRenderer.hpp>


#include <string>

#include <algorithm>
#include <iterator>

namespace Dunjun
{

	SceneRenderer::SceneRenderer()
	{
	}

	void SceneRenderer::reset()
	{
		if (m_currentShaders)
			m_currentShaders->stopUsing();
		m_currentShaders = nullptr;

		m_currentTexture = nullptr;
		Texture::bind(nullptr, 0);

		m_currentMaterial = nullptr;
	}

	void SceneRenderer::clearAll()
	{
		modelInstances.clear();
		pointsLights.clear();
	}

	void SceneRenderer::addSceneGraph(const SceneNode& node, Transform t)
	{
		node.draw(*this, t);
	}

	void SceneRenderer::draw(const Mesh* mesh) const
	{
		if (mesh)
			mesh->draw();
	}

	void SceneRenderer::addModelInstance(const MeshRenderer& meshRenderer, Transform t)
	{
		if (meshRenderer.parent->visible) // Just in case
			modelInstances.push_back({&meshRenderer, t});
	}

	void SceneRenderer::addPointLight(const PointLight* light)
	{
		pointsLights.push_back(light);
	}

	void SceneRenderer::renderAll()
	{
		// TODO(bill): Sort by mesh? - Instancing?
		std::sort(std::begin(modelInstances), std::end(modelInstances),
				  [](const ModelInstance& a, const ModelInstance& b) -> bool
		{
			const auto* A = a.meshRenderer->material;
			const auto* B = b.meshRenderer->material;

			if (A != B)
			{
				if (A->shaders == B->shaders)
					return A->diffuseMap < B->diffuseMap;
				return A->shaders < B->shaders;
			}
			return false;
		});

		for (const auto& inst : modelInstances)
		{
			const Material& material = *inst.meshRenderer->material;
			if (setShaders(material.shaders))
			{
				m_currentShaders->setUniform("u_camera", camera->getMatrix());
				m_currentShaders->setUniform("u_cameraPosition", camera->transform.position);

				m_currentShaders->setUniform("u_material.diffuseMap", (u32)0);
				m_currentShaders->setUniform("u_material.diffuseColor", material.diffuseColor);
				m_currentShaders->setUniform("u_material.specularColor", material.specularColor);
				m_currentShaders->setUniform("u_material.specularExponent", material.specularExponent);

				const PointLight* light = pointsLights[0];
				light->calculateRange();

				Vector3 lightIntensities;

				lightIntensities.r = light->color.r / 255.0f;
				lightIntensities.g = light->color.g / 255.0f;
				lightIntensities.b = light->color.b / 255.0f;
				lightIntensities *= light->brightness;


				m_currentShaders->setUniform("u_light.position", light->position);
				m_currentShaders->setUniform("u_light.intensities", lightIntensities);

				m_currentShaders->setUniform("u_light.attenuation.constant", light->attenuation.constant);
				m_currentShaders->setUniform("u_light.attenuation.linear", light->attenuation.linear);
				m_currentShaders->setUniform("u_light.attenuation.quadratic", light->attenuation.quadratic);

				m_currentShaders->setUniform("u_light.range", light->range);
			}
			setTexture(material.diffuseMap, 0);

			m_currentShaders->setUniform("u_transform", inst.transform);

			draw(inst.meshRenderer->mesh);
		}
	}

	void SceneRenderer::deferredGeometryPass()
	{
		// TODO(bill): Sort by mesh? - Instancing?
		std::sort(std::begin(modelInstances), std::end(modelInstances),
				  [](const ModelInstance& a, const ModelInstance& b) -> bool
		{
			const auto* A = a.meshRenderer->material;
			const auto* B = b.meshRenderer->material;

			if (A != B && A && B)
			{
				if (A->shaders == B->shaders)
					return A->diffuseMap < B->diffuseMap;
				return A->shaders < B->shaders;
			}
			return false;
		});

		if (geometryPassShaders == nullptr)
		{
			geometryPassShaders = new ShaderProgram();

			if (!geometryPassShaders->attachShaderFromFile(
				ShaderType::Vertex, "data/shaders/deferredGeometryPass.vert.glsl"))
				throw std::runtime_error(geometryPassShaders->errorLog);

			if (!geometryPassShaders->attachShaderFromFile(
				ShaderType::Fragment, "data/shaders/deferredGeometryPass.frag.glsl"))
				throw std::runtime_error(geometryPassShaders->errorLog);
			geometryPassShaders->bindAttribLocation((u32)AtrribLocation::Position,
												"a_position");
			geometryPassShaders->bindAttribLocation((u32)AtrribLocation::TexCoord,
												"a_texCoord");
			geometryPassShaders->bindAttribLocation((u32)AtrribLocation::Color, "a_color");
			geometryPassShaders->bindAttribLocation((u32)AtrribLocation::Normal,
												"a_normal");

			if (!geometryPassShaders->link())
				throw std::runtime_error(geometryPassShaders->errorLog);
		}

		assert(geometryPassShaders != nullptr);
	

		GBuffer::bind(getGBuffer());
		{
			glViewport(0, 0, getGBuffer()->width, getGBuffer()->height);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			geometryPassShaders->use();

			geometryPassShaders->setUniform("u_camera", camera->getMatrix());
			geometryPassShaders->setUniform("u_cameraPosition", camera->transform.position);
			for (const auto& inst : modelInstances)
			{
				if (inst.meshRenderer->material == nullptr)
					continue;


				{
					geometryPassShaders->setUniform("u_material.diffuseMap", (int)0);
					geometryPassShaders->setUniform("u_material.diffuseColor", inst.meshRenderer->material->diffuseColor);
					geometryPassShaders->setUniform("u_material.specularColor", inst.meshRenderer->material->specularColor);
					geometryPassShaders->setUniform("u_material.specularExponent", inst.meshRenderer->material->specularExponent);
				}
				setTexture(inst.meshRenderer->material->diffuseMap, 0);

				geometryPassShaders->setUniform("u_transform", inst.transform);

				draw(inst.meshRenderer->mesh);
			}

			glFlush();
		}
		GBuffer::bind(nullptr);
	}

	void SceneRenderer::deferredLightPass()
	{
		assert(pointLightShaders != nullptr);

		if (lightingTexture == nullptr)
			lightingTexture = new RenderTexture();

		lightingTexture->create(getGBuffer()->width, getGBuffer()->height, RenderTexture::Color);

		Texture::bind(&getGBuffer()->diffuse,  0);
		Texture::bind(&getGBuffer()->specular, 1);
		Texture::bind(&getGBuffer()->normal,   2);
		Texture::bind(&getGBuffer()->depth,    3);
		

		RenderTexture::bind(lightingTexture);
		{
			glClearColor(0, 0, 0, 0);
			glViewport(0, 0, lightingTexture->width, lightingTexture->height);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			pointLightShaders->use();
			pointLightShaders->setUniform("u_diffuse",  0);
			pointLightShaders->setUniform("u_specular", 1);
			pointLightShaders->setUniform("u_normal",   2);
			pointLightShaders->setUniform("u_depth",    3);

			pointLightShaders->setUniform("u_cameraInverse", inverse(camera->getMatrix()));

			glDepthMask(GL_FALSE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			for (const PointLight* light : pointsLights)
			{
				light->calculateRange();

				Vector3 lightIntensities;

				lightIntensities.r = light->color.r / 255.0f;
				lightIntensities.g = light->color.g / 255.0f;
				lightIntensities.b = light->color.b / 255.0f;
				lightIntensities *= light->brightness;


				pointLightShaders->setUniform("u_light.position", light->position);
				pointLightShaders->setUniform("u_light.intensities", lightIntensities);

				pointLightShaders->setUniform("u_light.attenuation.constant", light->attenuation.constant);
				pointLightShaders->setUniform("u_light.attenuation.linear", light->attenuation.linear);
				pointLightShaders->setUniform("u_light.attenuation.quadratic", light->attenuation.quadratic);

				pointLightShaders->setUniform("u_light.range", light->range);
				
				draw(quad);
			}
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);

			pointLightShaders->stopUsing();
		}
		RenderTexture::bind(nullptr);



		
	}

	bool SceneRenderer::setShaders(const ShaderProgram* shaders)
	{
		if (shaders != m_currentShaders)
		{
			if (m_currentShaders)
				m_currentShaders->stopUsing();

			m_currentShaders = shaders;
			m_currentShaders->use();

			return true;
		}

		return false;
	}

	bool SceneRenderer::setTexture(const Texture* texture, GLuint position)
	{
		if (texture != m_currentTexture)
		{
			m_currentTexture = texture;

			Texture::bind(m_currentTexture, position);

			return true;
		}

		return false;
	}
} // namespace Dunjun
