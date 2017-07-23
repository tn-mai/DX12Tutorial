/**
* @file Collision.cpp
*/
#include "Collision.h"
#include <algorithm>

using namespace DirectX;

namespace Collision {

namespace /* unnamed */ {

/**
* 円と円の当たり判定.
*
* @param sa 左辺側の形状.
* @param pa 左辺側の座標.
* @param sb 右辺側の形状.
* @param pb 右辺側の座標.
*
* @retval true  衝突している.
* @retval false 衝突していない.
*/
bool CircleCircle(const Shape& sa, const XMFLOAT2& pa, const Shape& sb, const XMFLOAT2& pb)
{
	const float dx = pa.x - pb.x;
	const float dy = pa.y - pb.y;
	const float ra = sa.circle.radius;
	const float rb = sb.circle.radius;
	return (dx * dx + dy + dy) < (ra * ra + rb * rb);
}

/**
* 長方形と円の当たり判定.
*
* @param sa 左辺側の形状.
* @param pa 左辺側の座標.
* @param sb 右辺側の形状.
* @param pb 右辺側の座標.
*
* @retval true  衝突している.
* @retval false 衝突していない.
*/
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

/**
* 円と長方形の当たり判定.
*
* @param sa 左辺側の形状.
* @param pa 左辺側の座標.
* @param sb 右辺側の形状.
* @param pb 右辺側の座標.
*
* @retval true  衝突している.
* @retval false 衝突していない.
*/
bool CircleRect(const Shape& sa, const XMFLOAT2& pa, const Shape& sb, const XMFLOAT2& pb) { return RectCircle(sb, pb, sa, pa); }

/**
* 長方形と長方形の当たり判定.
*
* @param sa 左辺側の形状.
* @param pa 左辺側の座標.
* @param sb 右辺側の形状.
* @param pb 右辺側の座標.
*
* @retval true  衝突している.
* @retval false 衝突していない.
*/
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

/**
* コンストラクタ.
*/
Shape::Shape() : type(ShapeType::Circle), circle{ 0.1f }
{}

/**
* コピーコンストラクタ.
*
* @param src コピー元のオブジェクト.
*/
Shape::Shape(const Shape& src) : type(src.type)
{
	switch (type) {
	case ShapeType::Circle:
		circle.radius = src.circle.radius;
		break;
	case ShapeType::Rectangle:
		rect.leftTop = src.rect.leftTop;
		rect.rightBottom = src.rect.rightBottom;
		break;
	}
}

/**
* コピー代入演算子.
*
* @param src コピー元のオブジェクト.
*
* @return 自分自身への参照.
*/
Shape& Shape::operator=(const Shape& src)
{
	this->~Shape();
	new(this) Shape(src);
	return *this;
}

/**
* 円形の形状オブジェクトを作成する.
*
* @param r 円の半径.
*
* @return 形状オブジェクト.
*/
Shape Shape::MakeCircle(float r)
{
	Shape shape;
	shape.type = ShapeType::Circle;
	shape.circle.radius = r;
	return shape;
}

/**
* 長方形の形状オブジェクトを作成する.
*
* @param lt 長方形の左上座標.
* @param rb 長方形の右下座標.
*
* @return 形状オブジェクト.
*
* ltとrbはオブジェクト中心からの相対座標で指定する.
*/
Shape Shape::MakeRectangle(const DirectX::XMFLOAT2& lt, const DirectX::XMFLOAT2& rb)
{
	Shape shape;
	shape.type = ShapeType::Rectangle;
	shape.rect.leftTop = lt;
	shape.rect.rightBottom = rb;
	return shape;
}

/**
* 当たり判定.
*
* @param sa 左辺側の形状.
* @param pa 左辺側の座標.
* @param sb 右辺側の形状.
* @param pb 右辺側の座標.
*
* @retval true  衝突している.
* @retval false 衝突していない.
*/
bool IsCollision(const Shape& sa, const XMFLOAT2& pa, const Shape& sb, const XMFLOAT2& pb)
{
	static bool(*const funcList[2][2])(const Shape&, const XMFLOAT2&, const Shape&, const XMFLOAT2&) = {
		{ CircleCircle, CircleRect },
		{ RectCircle, RectRect },
	};
	return funcList[static_cast<int>(sa.type)][static_cast<int>(sb.type)](sa, pa, sb, pb);
}

} // namespace Collision
