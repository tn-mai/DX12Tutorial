/**
* @file Collision.h
*/
#ifndef DX12TUTORIAL_SRC_COLLISION_H_
#define DX12TUTORIAL_SRC_COLLISION_H_
#include <DirectXMath.h>

/**
* 当たり判定名前空間.
*/
namespace Collision {

/**
* 当たり判定の形.
*/
enum class ShapeType
{
	Circle, ///< 円形.
	Rectangle, ///< 長方形.
};

/**
* 当たり判定用の形状オブジェクト.
*
* MakeCircle, MakeRectangle関数で形状を作成できる.
*/
struct Shape
{
	static Shape MakeCircle(float r);
	static Shape MakeRectangle(const DirectX::XMFLOAT2& lt, const DirectX::XMFLOAT2& rb);
	Shape();
	Shape(const Shape& src);
	Shape& operator=(const Shape& src);
	~Shape() {}

	ShapeType type;
	union {
		struct Circle
		{
			float radius;
		} circle;
		struct Rect
		{
			DirectX::XMFLOAT2 leftTop;
			DirectX::XMFLOAT2 rightBottom;
		} rect;
	};
};

bool IsCollision(const Shape&, const DirectX::XMFLOAT2&, const Shape&, const DirectX::XMFLOAT2&);

} // namespace Collision

#endif // DX12TUTORIAL_SRC_COLLISION_H_