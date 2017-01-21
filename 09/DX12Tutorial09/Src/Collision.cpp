/**
* @file Collision.cpp
*/
#include "Collision.h"
#include <algorithm>

using namespace DirectX;

namespace Collision {

namespace /* unnamed */ {

bool CircleCircle(const Shape& sa, const XMFLOAT2& pa, const Shape& sb, const XMFLOAT2& pb)
{
	const float dx = pa.x - pb.x;
	const float dy = pa.y - pb.y;
	const float ra = sa.circle.radius;
	const float rb = sb.circle.radius;
	return (dx * dx + dy + dy) < (ra * ra + rb * rb);
}

bool RectCircle(const Shape& sa, const XMFLOAT2& pa, const Shape& sb, const XMFLOAT2& pb)
{
	const XMFLOAT2 aLT(pa.x + sa.rect.leftTop.x, pa.y + sa.rect.leftTop.y);
	const XMFLOAT2 aRB(pa.x + sa.rect.rightBottom.x, pa.y + sa.rect.rightBottom.y);
	XMFLOAT2 p;
	p.x = std::min(std::max(pb.x, aLT.x), aRB.x);
	p.y = std::min(std::max(pb.y, aLT.y), aRB.y);
	const float dx = p.x - pb.x;
	const float dy = p.y - pb.y;
	const float rb = sb.circle.radius;
	return (dx * dx + dy * dy) < (rb * rb);
}
bool CircleRect(const Shape& sa, const XMFLOAT2& pa, const Shape& sb, const XMFLOAT2& pb) { return RectCircle(sb, pb, sa, pa); }

bool RectRect(const Shape& sa, const XMFLOAT2& pa, const Shape& sb, const XMFLOAT2& pb)
{
	const XMFLOAT2 aLT(pa.x + sa.rect.leftTop.x, pa.y + sa.rect.leftTop.y);
	const XMFLOAT2 aRB(pa.x + sa.rect.rightBottom.x, pa.y + sa.rect.rightBottom.y);
	const XMFLOAT2 bLT(pb.x + sb.rect.leftTop.x, pb.y + sb.rect.leftTop.y);
	const XMFLOAT2 bRB(pb.x + sb.rect.rightBottom.x, pb.y + sb.rect.rightBottom.y);
	if (aRB.x < bLT.x || aLT.x > bRB.x) return false;
	if (aRB.y < bLT.y || aLT.y > bRB.y) return false;
	return true;
}

} // unnamed namespace

bool IsCollision(const Shape& sa, const XMFLOAT2& pa, const Shape& sb, const XMFLOAT2& pb)
{
	static bool(*const funcList[2][2])(const Shape&, const XMFLOAT2&, const Shape&, const XMFLOAT2&) = {
		{ CircleCircle, CircleRect },
		{ RectCircle, RectRect },
	};
	return funcList[static_cast<int>(sa.type)][static_cast<int>(sb.type)](sa, pa, sb, pb);
}

} // namespace Collision
