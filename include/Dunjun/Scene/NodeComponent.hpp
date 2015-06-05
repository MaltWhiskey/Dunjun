#ifndef DUNJUN_SCENE_NODECOMPONENT_HPP
#define DUNJUN_SCENE_NODECOMPONENT_HPP

#include <Dunjun/System/NonCopyable.hpp>
#include <Dunjun/System/Types.hpp>
#include <Dunjun/Common.hpp>
#include <Dunjun/Graphics/Transform.hpp>
#include <Dunjun/System/Time.hpp>
#include <Dunjun/Scene/SceneRenderer.hpp>

namespace Dunjun
{
class SceneNode;
class NodeComponent : private NonCopyable
{
public:
	using UPtr = std::unique_ptr<NodeComponent>;

	virtual ~NodeComponent() {}

	virtual void onStart() {}
	virtual void update(Time dt) {}
	virtual void draw(SceneRenderer& renderer, Transform t) const {}

	SceneNode* getParent() const { return m_parent; }

private:
	friend class SceneNode;

	SceneNode* m_parent{nullptr};
};
} // namespace Dunjun

#endif
