/**
* @file Action.cpp
*
* スプライトの動作を制御するスクリプト.
* スプライトの移動、回転、消滅、他のスプライトの生成などの各要素について、
* 目標値、動作時間、間隔といったパラメータで制御する.
*

漂白剤
お風呂パイプ洗浄液x2

*/
#include "Action.h"
#include "Sprite.h"
#include <vector>
#include <string>
#include <set>
#include <algorithm>

using namespace DirectX;

namespace Action {

enum class Type
{
	Move,
	Accel,
	Wait,
	Path,
	ControlPoint,
	Vanishing,
	Generation,
	Animation,
	ManualControl,
};

enum InterporationType
{
	InterporationType_Step,
	InterporationType_Linear,
	InterporationType_Ease,
	InterporationType_EaseIn,
	InterporationType_EaseOut,
};

enum MoveParamId
{
	MoveParamId_DirectionDegree,
	MoveParamId_Speed,
};

enum AccelParamId
{
	AccelParamId_DirectionDegree,
	AccelParamId_Accel,
};

enum WaitParamId
{
	WaitParamId_Time,
};

enum PathParamId
{
	PathParamId_Time,
	PathParamId_Count,
	PathParamId_Interporation,
};

enum ControlPointId
{
	ControlPointId_X,
	ControlPointId_Y,
};

enum GenParamId
{
	GenParamId_GeneratorId,
};

enum AnimeParamId
{
	AnimeParamId_Id,
};

struct Data
{
	Type type;
	float param[3];
};

typedef std::vector<Data> Sequence;
struct List {
	std::string name;
	std::vector<Sequence> list;
};
bool operator<(const List& lhs, const List& rhs) { return lhs.name < rhs.name; }
bool operator<(const List& lhs, const char* rhs) { return lhs.name.compare(rhs) < 0; }
bool operator<(const char* lhs, const List& rhs) { return rhs.name.compare(lhs) > 0; }
typedef std::set<List> File;

const List& GetList()
{
	static List list;
	if (list.name.empty()) {
		list.name = "SampleAction";
		list.list.resize(3);
		list.list[0].push_back(Data{ Type::Path, { 3, 3, 0 }});
		list.list[0].push_back(Data{ Type::ControlPoint, { 100, 0 }});
		list.list[0].push_back(Data{ Type::ControlPoint, { 190, 510 }});
		list.list[0].push_back(Data{ Type::ControlPoint, { 700, 600 }});

		list.list[1].push_back(Data{ Type::Path,{ 5, 6, 0 } });
		list.list[1].push_back(Data{ Type::ControlPoint,{ 400, 0 } });
		list.list[1].push_back(Data{ Type::ControlPoint,{ 100, 200 } });
		list.list[1].push_back(Data{ Type::ControlPoint,{ 700, 400 } });
		list.list[1].push_back(Data{ Type::ControlPoint,{ 400, 600 } });
		list.list[1].push_back(Data{ Type::ControlPoint,{ 400, 600 } });
		list.list[1].push_back(Data{ Type::ControlPoint,{ 400, 0 } });

		list.list[2].push_back(Data{ Type::Move,{ 270, 200 } });
		list.list[2].push_back(Data{ Type::Accel,{ 180, 800 } });
		list.list[2].push_back(Data{ Type::Wait,{ 0.5f } });
		list.list[2].push_back(Data{ Type::Accel,{ 0, 800 } });
		list.list[2].push_back(Data{ Type::Wait,{ 1 } });
		list.list[2].push_back(Data{ Type::Accel,{ 180, 800 } });
		list.list[2].push_back(Data{ Type::Wait,{ 1 } });
		list.list[2].push_back(Data{ Type::Accel,{ 0, 800 } });
		list.list[2].push_back(Data{ Type::Wait,{ 1 } });
		list.list[2].push_back(Data{ Type::Move,{ 0, 0 } });
		list.list[2].push_back(Data{ Type::Accel,{ 0, 0 } });
	}
	return list;
}


XMVECTOR DeBoorI(int k, int degree, int i, float x, const std::vector<XMFLOAT2>& points) {
	if (k == 0) {
		return XMLoadFloat2(&points[std::max(0, std::min<int>(i, points.size() - 1))]);
	}
	const float alpha = (x - static_cast<float>(i)) / static_cast<float>(degree + 1 - k);
	const XMVECTOR a = DeBoorI(k - 1, degree, i - 1, x, points);
	const XMVECTOR b = DeBoorI(k - 1, degree, i, x, points);
	const XMVECTORF32 t{ alpha, (1.0f - alpha), 0.0f, 0.0f };
	return XMVectorAdd(
		XMVectorMultiply(a, XMVectorSwizzle(t, 1, 1, 1, 1)),
		XMVectorMultiply(b, XMVectorSwizzle(t, 0, 0, 0, 0)));
}

XMVECTOR DeBoor(int degree, float x, const std::vector<XMFLOAT2>& points) {
	const int i = static_cast<int>(x);
	return DeBoorI(degree, degree, i, x, points);
}

/** Create the degree 3 B-Spline curve.

@param points  The vector of the control point. It must have a size greater than 3.
@param numOfSegments  The division nunmber of the line.

@return B-Spline curve. it will contain the 'numOfSegments' elements.
*/
std::vector<XMFLOAT2> CreateBSpline(const std::vector<XMFLOAT2>& points, int numOfSegments) {
	std::vector<XMFLOAT2> v;
	v.reserve(numOfSegments);
	const float n = static_cast<float>(points.size() + 1);
	for (int i = 0; i < numOfSegments - 1; ++i) {
		const float ratio = static_cast<float>(i) / static_cast<float>(numOfSegments - 1);
		const float x = ratio * n + 1;
		XMVECTORF32 pos;
		pos.v = DeBoor(3, x, points);
		v.emplace_back(pos.f[0], pos.f[1]);
	}
	v.push_back(points.back());

	// 同じベクトルを持つ区間を統合する.
	std::vector<XMVECTOR> vectorList;
	vectorList.reserve(v.size() - 1);
	for (size_t i = 1; i < v.size(); ++i) {
		vectorList.push_back(XMVector2Normalize(XMVectorSubtract(XMLoadFloat2(&v[i]), XMLoadFloat2(&v[i - 1]))));
	}
	std::vector<XMFLOAT2> ret;
	ret.reserve((v.size() + 1) / 2);
	ret.push_back(v.front());
	XMVECTOR vec = vectorList[0];
	for (size_t i = 1; i < vectorList.size(); ++i) {
		float r;
		XMStoreFloat(&r, XMVector2Dot(vec, vectorList[i]));
		if (r < 0.999f) {
			ret.push_back(v[i]);
			vec = vectorList[i];
		} else {
			vec = XMVector2Normalize(XMVectorSubtract(XMLoadFloat2(&v[i]), XMLoadFloat2(&ret.back())));
		}
	}
	ret.push_back(v.back());

	return ret;
}

XMFLOAT2 Spline(const XMFLOAT2& p0, const XMFLOAT2& p1, const XMFLOAT2& p2, const XMFLOAT2& p3, float t)
{
	static const XMMATRIX H{
		XMVECTORF32{ 2,-2, 1, 1 },
		XMVECTORF32{-3, 3,-2, -1 },
		XMVECTORF32{ 0, 0, 1, 0 },
		XMVECTORF32{ 1, 0, 0, 0 },
	};
	const XMMATRIX G{
		XMVECTORF32{ p1.x, p1.y, 0, 1 },
		XMVECTORF32{ p2.x, p2.y, 0, 1 },
		XMVECTORF32{ (p2.x - p0.x) * 0.5f, (p2.y - p0.y) * 0.5f, 0, 1 },
		XMVECTORF32{ (p3.x - p1.x) * 0.5f, (p3.y - p1.y) * 0.5f, 0, 1 },
	};
	const XMVECTORF32 vt{ t * t * t, t * t, t, 1 };

	XMFLOAT2 ret;
	XMStoreFloat2(&ret, XMVector4Transform(XMVector4Transform(vt, H), G));
	return ret;
}

float DegreeToRadian(float degree)
{
	return (3.14159265359f * degree / 180.0f);
}

/**
* XMVECTOR(1, 0)を0度とし、反時計回りにradだけ回転させたベクトルを返す.
*/
XMVECTOR RadianToVector(float rad, float mag)
{
	XMFLOAT2A tmp;
	XMScalarSinCos(&tmp.y, &tmp.x, rad);
	tmp.y *= -1.0f;
	return XMVectorMultiply(XMLoadFloat2A(&tmp), XMVectorSwizzle(XMLoadFloat(&mag), 0, 0, 0, 0));
}

void Store(XMFLOAT3* f3, const XMFLOAT2& f2)
{
	f3->x = f2.x;
	f3->y = f2.y;
}

/**
* コンストラクタ.
*/
ActionController::ActionController(const List& l, uint32_t no) :
	list(l), seqIndex(0), dataIndex(0), currentTime(0), totalTime(0), type(Type::Move), move(XMFLOAT2(0, 0)), accel(XMFLOAT2(0, 0))
{
	if (no >= list.list.size() || dataIndex >= list.list[no].size()) {
		return;
	}
	seqIndex = no;
	Init();
}

void ActionController::Init()
{
	if (seqIndex >= list.list.size() || type == Type::Vanishing) {
		return;
	}
	for (; dataIndex < list.list[seqIndex].size(); ++dataIndex) {
		const Data& data = list.list[seqIndex][dataIndex];
		switch (data.type) {
		case Type::Move:
			type = Type::Move;
			XMStoreFloat2(&move, RadianToVector(DegreeToRadian(data.param[MoveParamId_DirectionDegree]), data.param[MoveParamId_Speed]));
			break;
		case Type::Accel:
			type = Type::Move;
			XMStoreFloat2(&accel, RadianToVector(DegreeToRadian(data.param[AccelParamId_DirectionDegree]), data.param[AccelParamId_Accel]));
			break;
		case Type::Wait:
			type = Type::Move;
			totalTime = data.param[WaitParamId_Time];
			return;
		case Type::Path: {
			type = Type::Path;
			totalTime = data.param[PathParamId_Time];
			path.type = static_cast<InterporationType>(static_cast<int>(data.param[PathParamId_Interporation]));
			std::vector<XMFLOAT2> controlPoints;
			controlPoints.reserve(static_cast<size_t>(data.param[PathParamId_Count]));
			for (int i = 0; i < data.param[PathParamId_Count]; ++i) {
				if (++dataIndex >= list.list[seqIndex].size()) {
					break;
				}
				const Data& cp = list.list[seqIndex][dataIndex];
				if (cp.type != Type::ControlPoint) {
					break;
				}
				controlPoints.emplace_back(cp.param[ControlPointId_X], cp.param[ControlPointId_Y]);
			}
			std::vector<XMFLOAT2> tmpPoints = CreateBSpline(controlPoints, controlPoints.size() * 16);
			std::vector<float> distances;
			path.cp.resize(tmpPoints.size());
			for (size_t i = 0; i < tmpPoints.size(); ++i) {
				path.cp[i].pos = tmpPoints[i];
			}
			path.cp[0].t = 0.0f;
			for (size_t i = 0; i < tmpPoints.size() - 1; ++i) {
				const XMVECTOR vec = XMVector2Length(XMVectorSubtract(XMLoadFloat2(&tmpPoints[i]), XMLoadFloat2(&tmpPoints[i + 1])));
				XMStoreFloat(&path.cp[i + 1].t, vec);
				path.cp[i + 1].t += path.cp[i].t;
			}
			const float factor = totalTime / path.cp.back().t;
			for (auto& e : path.cp) {
				e.t *= factor;
			}
			return;
		}
		case Type::Vanishing:
			type = Type::Vanishing;
			return;
		case Type::Animation:
			break;
		case Type::Generation:
			break;
		}
	}
}

void ActionController::UpdateSub(float delta, Sprite::Sprite* pSprite)
{
	switch (type) {
	case Type::Move: {
		const XMVECTOR d = XMVectorSwizzle(XMLoadFloat(&delta), 0, 0, 1, 1);
		const XMVECTOR m = XMLoadFloat2(&move);
		const XMVECTOR a = XMLoadFloat2(&accel);
		XMStoreFloat3(&pSprite->pos, XMVectorAdd(XMLoadFloat3(&pSprite->pos), XMVectorMultiply(m, d)));
		XMStoreFloat2(&move, XMVectorAdd(m, XMVectorMultiply(a, d)));
		break;
	}
	case Type::Path: {
		const float t = currentTime + delta;
		if (t >= totalTime) {
			pSprite->pos.x = path.cp.back().pos.x;
			pSprite->pos.y = path.cp.back().pos.y;
		} else {
			const auto itr = std::upper_bound(path.cp.begin(), path.cp.end(), Point{ {}, t });
			if (itr != path.cp.end()) {
				const Point& p0 = *(itr - 1);
				const Point& p1 = *itr;
				const float length = p1.t - p0.t;
				const float distance = t - p0.t;
				const float ratio = distance / length;
				pSprite->pos.x = p0.pos.x * (1.0f - ratio) + p1.pos.x * ratio;
				pSprite->pos.y = p0.pos.y * (1.0f - ratio) + p1.pos.y * ratio;
			}
		}
		break;

	}
	default:
		break;
	}
}

void ActionController::Update(float delta, Sprite::Sprite* pSprite)
{
	if (type == Type::ManualControl) {
		XMStoreFloat3(&pSprite->pos, XMVectorAdd(XMLoadFloat3(&pSprite->pos), XMLoadFloat2(&move)));
		XMStoreFloat2(&move, XMVectorAdd(XMLoadFloat2(&move), XMLoadFloat2(&accel)));
		if (currentTime < totalTime) {
			currentTime += delta;
		}
		return;
	}
	if (seqIndex >= list.list.size() || dataIndex >= list.list[seqIndex].size() || type == Type::Vanishing) {
		return;
	}
	UpdateSub(std::min(std::max(0.0f, totalTime - currentTime), delta), pSprite);
	currentTime += delta;
	while (currentTime >= totalTime) {
		currentTime -= totalTime;
		totalTime = 0.0f;
		delta = currentTime;
		++dataIndex;
		if (dataIndex >= list.list[seqIndex].size()) {
			return;
		}
		Init();
		UpdateSub(std::min(std::max(0.0f, totalTime - currentTime), delta), pSprite);
	}
}

void ActionController::SetSeqIndex(uint32_t no)
{
	if (no >= list.list.size()) {
		return;
	}
	seqIndex = no;
	dataIndex = 0;
	const Data& data = list.list[seqIndex][dataIndex];
	type = data.type;
	currentTime = 0.0f;
	totalTime = 0.0f;
	Init();
}

void ActionController::SetManualMove(float degree, float speed)
{
	type = Type::ManualControl;
	XMStoreFloat2(&move, RadianToVector(DegreeToRadian(degree), speed));
}

void ActionController::SetManualAccel(float degree, float speed)
{
	type = Type::ManualControl;
	XMStoreFloat2(&accel, RadianToVector(DegreeToRadian(degree), speed));
}

void ActionController::SetTime(float time)
{
	type = Type::ManualControl;
	currentTime = 0.0f;
	totalTime = time;
}

} // namespace Action 
