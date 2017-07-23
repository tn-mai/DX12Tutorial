/**
* @file Collision.h
*/
#ifndef DX12TUTORIAL_SRC_COLLISION_H_
#define DX12TUTORIAL_SRC_COLLISION_H_
#include <DirectXMath.h>

/**
* �����蔻�薼�O���.
*/
namespace Collision {

/**
* �����蔻��̌`.
*/
enum class ShapeType
{
	Circle, ///< �~�`.
	Rectangle, ///< �����`.
};

/**
* �����蔻��p�̌`��I�u�W�F�N�g.
*
* MakeCircle, MakeRectangle�֐��Ō`����쐬�ł���.
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