#include "HeightMapConvexityMapRenderingStep.hpp"
#include "../renderer/Renderer.hpp"
#include "../renderer/RendererObject.hpp"
#include "../InternalErrorException.hpp"
#include "../genlib/HeightMap.hpp"
#include "../renderer/RenderingBounds2D.hpp"

using namespace geogen;
using namespace renderer;
using namespace corelib;
using namespace genlib;

void HeightMapConvexityMapRenderingStep::Step(Renderer* renderer) const
{
	HeightMap* map = new HeightMap(dynamic_cast<RenderingBounds2D*>(renderer->GetRenderingSequenceMetadata().GetRenderingBounds(this))->GetRectangle(), 0, renderer->GetRenderingSequence().GetRenderScale());
	map->ConvexityMap(this->radius);
	RendererObject* object = new RendererObject(RENDERER_OBJECT_TYPE_HEIGHT_MAP, map);

	renderer->GetObjectTable().SetObject(this->GetReturnSlot(), object);
}

Rectangle HeightMapConvexityMapRenderingStep::CalculateRenderingBounds(Renderer* renderer, Rectangle argumentBounds) const
{
	return Rectangle::Expand(argumentBounds, this->radius);
}

void HeightMapConvexityMapRenderingStep::SerializeArguments(IOStream& stream) const
{
	stream << this->radius;
}