/**
* @file Collision.h
*/
#ifndef DX12TUTORIAL_SRC_COLLISION_H_
#define DX12TUTORIAL_SRC_COLLISION_H_
#include <DirectXMath.h>
#include <functional>
#include <map>

namespace Collision {

enum class ShapeType
{
	Circle,
	Rectangle,
};

struct Shape
{
	Shape() : type(ShapeType::Circle), circle{ 0.1f } {}
	Shape(const Shape& src) : type(src.type)
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
	~Shape() {}

	Shape& operator=(const Shape& src)
	{
		this->~Shape();
		new(this) Shape(src);
		return *this;
	}

	static Shape MakeCircle(float r)
	{
		Shape shape;
		shape.type = ShapeType::Circle;
		shape.circle.radius = r;
		return shape;
	}
	static Shape MakeRectangle(const DirectX::XMFLOAT2& lt, const DirectX::XMFLOAT2& rb)
	{
		Shape shape;
		shape.type = ShapeType::Rectangle;
		shape.rect.leftTop = lt;
		shape.rect.rightBottom = rb;
		return shape;
	}

	ShapeType type;
	int groupId;
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