/**
* @file Collision.cpp
*/
#include "Collision.h"
#include <algorithm>

using namespace DirectX;

namespace Collision {

namespace /* unnamed */ {

/**
* �~�Ɖ~�̓����蔻��.
*
* @param sa ���ӑ��̌`��.
* @param pa ���ӑ��̍��W.
* @param sb �E�ӑ��̌`��.
* @param pb �E�ӑ��̍��W.
*
* @retval true  �Փ˂��Ă���.
* @retval false �Փ˂��Ă��Ȃ�.
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
* �����`�Ɖ~�̓����蔻��.
*
* @param sa ���ӑ��̌`��.
* @param pa ���ӑ��̍��W.
* @param sb �E�ӑ��̌`��.
* @param pb �E�ӑ��̍��W.
*
* @retval true  �Փ˂��Ă���.
* @retval false �Փ˂��Ă��Ȃ�.
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
* �~�ƒ����`�̓����蔻��.
*
* @param sa ���ӑ��̌`��.
* @param pa ���ӑ��̍��W.
* @param sb �E�ӑ��̌`��.
* @param pb �E�ӑ��̍��W.
*
* @retval true  �Փ˂��Ă���.
* @retval false �Փ˂��Ă��Ȃ�.
*/
bool CircleRect(const Shape& sa, const XMFLOAT2& pa, const Shape& sb, const XMFLOAT2& pb) { return RectCircle(sb, pb, sa, pa); }

/**
* �����`�ƒ����`�̓����蔻��.
*
* @param sa ���ӑ��̌`��.
* @param pa ���ӑ��̍��W.
* @param sb �E�ӑ��̌`��.
* @param pb �E�ӑ��̍��W.
*
* @retval true  �Փ˂��Ă���.
* @retval false �Փ˂��Ă��Ȃ�.
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
* �R���X�g���N�^.
*/
Shape::Shape() : type(ShapeType::Circle), circle{ 0.1f }
{}

/**
* �R�s�[�R���X�g���N�^.
*
* @param src �R�s�[���̃I�u�W�F�N�g.
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
* �R�s�[������Z�q.
*
* @param src �R�s�[���̃I�u�W�F�N�g.
*
* @return �������g�ւ̎Q��.
*/
Shape& Shape::operator=(const Shape& src)
{
	this->~Shape();
	new(this) Shape(src);
	return *this;
}

/**
* �~�`�̌`��I�u�W�F�N�g���쐬����.
*
* @param r �~�̔��a.
*
* @return �`��I�u�W�F�N�g.
*/
Shape Shape::MakeCircle(float r)
{
	Shape shape;
	shape.type = ShapeType::Circle;
	shape.circle.radius = r;
	return shape;
}

/**
* �����`�̌`��I�u�W�F�N�g���쐬����.
*
* @param lt �����`�̍�����W.
* @param rb �����`�̉E�����W.
*
* @return �`��I�u�W�F�N�g.
*
* lt��rb�̓I�u�W�F�N�g���S����̑��΍��W�Ŏw�肷��.
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
* �����蔻��.
*
* @param sa ���ӑ��̌`��.
* @param pa ���ӑ��̍��W.
* @param sb �E�ӑ��̌`��.
* @param pb �E�ӑ��̍��W.
*
* @retval true  �Փ˂��Ă���.
* @retval false �Փ˂��Ă��Ȃ�.
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
