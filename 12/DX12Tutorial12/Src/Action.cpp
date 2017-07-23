/**
* @file Action.cpp
*
* �X�v���C�g�̓���𐧌䂷��X�N���v�g.
* �X�v���C�g�̈ړ��A��]�A���ŁA���̃X�v���C�g�̐����Ȃǂ̊e�v�f�ɂ��āA
* �ڕW�l�A���쎞�ԁA�Ԋu�Ƃ������p�����[�^�Ő��䂷��.
*
*/
#include "Action.h"
#include "Sprite.h"
#include "Json.h"
#include <vector>
#include <string>
#include <set>
#include <algorithm>

using namespace DirectX;

namespace Action {

/**
* �A�N�V�����f�[�^�̎��.
*/
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

/**
* ��ԕ��@.
*
* ���ݖ�����.
*/
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
	GenParamId_Speed,
	GenParamId_DirectionDegree,
	GenParamId_Option,
};

enum AnimeParamId
{
	AnimeParamId_Id,
};

/**
* �A�N�V�����f�[�^.
*
* �X�̃A�N�V������ێ�����.
*/
struct Data
{
	Type type; ///< �A�N�V�����̎��.
	float param[3]; ///< �p�����[�^�z��.
};

/**
* �A�N�V�����V�[�P���X.
*
* �A�N�V�����f�[�^�̔z��.
*/
typedef std::vector<Data> Sequence;

/**
* �A�N�V�����V�[�P���X�̃��X�g.
*/
struct List {
	std::string name; ///< ���X�g��.
	std::vector<Sequence> list; ///< �V�[�P���X�̔z��.
};
bool operator<(const List& lhs, const List& rhs) { return lhs.name < rhs.name; }
bool operator<(const List& lhs, const char* rhs) { return lhs.name.compare(rhs) < 0; }
bool operator<(const char* lhs, const List& rhs) { return rhs.name.compare(lhs) > 0; }

/**
* �h�E�u�[�A�̃A���S���Y���ɂ��B�X�v���C�����W�̐���.
*
* @param k      �ċA�Ăяo���̐[�x. degree�ŊJ�n���A�ċA�ďo�����Ƀf�N�������g�����. 0�ɂȂ����Ƃ��ċA�͏I������.
* @param degree ����.
* @param i      ���W�����Ɏg�p����ŏ��̃R���g���[���|�C���g�̃C���f�b�N�X.
* @param x      �����������W�̈ʒu. �ŏ��̃R���g���[���|�C���g��0�A�Ō�̃R���g���[���|�C���g��N�Ƃ����Ƃ��A
*               0�`N�̒l�����.
* @param points �R���g���[���|�C���g�̔z��.
*
* @return x�ɑΉ�����B�X�v���C���J�[�u��̍��W.
*/
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

/**
*�h�E�u�[�A�̃A���S���Y���ɂ��B�X�v���C�����W�̐���.
*
* @param degree ����.
* @param x      �����������W�̈ʒu. �ŏ��̃R���g���[���|�C���g��0�A�Ō�̃R���g���[���|�C���g��N�Ƃ����Ƃ��A
*               0�`N�̒l�����.
* @param points �R���g���[���|�C���g�̔z��.
*
* @return x�ɑΉ�����B�X�v���C���J�[�u��̍��W.
*/
XMVECTOR DeBoor(int degree, float x, const std::vector<XMFLOAT2>& points) {
	const int i = static_cast<int>(x);
	return DeBoorI(degree, degree, i, x, points);
}

/**
* 3��B�X�v���C���Ȑ��𐶐�����.
*
* @param points        �R���g���[���|�C���g�̔z��. ���Ȃ��Ƃ�3�̃R���g���[���|�C���g���܂�ł��Ȃ���΂Ȃ�Ȃ�.
* @param numOfSegments �������钆�ԓ_�̐�. points�̐��ȏ�̒l�łȂ���΂Ȃ�Ȃ�.
*
* @return �������ꂽ���ԓ_�̔z��.
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

	// �����x�N�g��������Ԃ𓝍�����.
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

/**
* �x���@�̒l���ʓx�@�ɕϊ�����.
*
* @param degree �x���@�̒l.
*
* @return �ʓx�@�ɂ�����degree�ɑΉ�����l.
*/
float DegreeToRadian(float degree)
{
	return (3.14159265359f * degree / 180.0f);
}

/**
* 
*
* @param rad ��]�p(radian).
* @param mag �x�N�g���̑傫��.
*
* @return 2�����x�N�g��(mag, 0)��0�x�Ƃ��A�����v����rad������]�������x�N�g����Ԃ�.
*/
XMVECTOR RadianToVector(float rad, float mag)
{
	XMFLOAT2A tmp;
	XMScalarSinCos(&tmp.y, &tmp.x, rad);
	tmp.y *= -1.0f;
	return XMVectorMultiply(XMLoadFloat2A(&tmp), XMVectorSwizzle(XMLoadFloat(&mag), 0, 0, 0, 0));
}

/**
* �R���X�g���N�^.
*/
Controller::Controller()
{
	SetList(nullptr);
}

/**
* �R���X�g���N�^.
*
* @param p  �ݒ肷��A�N�V�������X�g�ւ̃|�C���^.
* @param no �Đ�����A�N�V�����ԍ�.
*/
Controller::Controller(const List* p, uint32_t no)
{
	SetList(p, no);
}

/**
* �A�N�V�������X�g��ݒ肷��.
*
* @param p  �ݒ肷��A�N�V�������X�g�ւ̃|�C���^.
* @param no �Đ�����A�N�V�����ԍ�.
*/
void Controller::SetList(const List* p, uint32_t no)
{
	list = p;
	seqIndex = 0;
	dataIndex = 0;
	currentTime = 0;
	totalTime = 0;
	isGeneratorActive = false;
	type = Type::ManualControl;
	move = XMFLOAT2(0, 0);
	accel = XMFLOAT2(0, 0);
	if (!list || no >= list->list.size() || dataIndex >= list->list[no].size()) {
		return;
	}
	seqIndex = no;
	Init();
}

/**
* �A�N�V�����̌��݂̏�ԂŃR���g���[�����X�V����.
*/
void Controller::Init(Sprite::Sprite* pSprite)
{
	if (!list || seqIndex >= list->list.size() || type == Type::Vanishing) {
		return;
	}
	for (; dataIndex < list->list[seqIndex].size(); ++dataIndex) {
		const Data& data = list->list[seqIndex][dataIndex];
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
				if (++dataIndex >= list->list[seqIndex].size()) {
					break;
				}
				const Data& cp = list->list[seqIndex][dataIndex];
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
			if (pSprite && data.param[AnimeParamId_Id] >= 0) {
				pSprite->SetSeqIndex(static_cast<uint32_t>(data.param[AnimeParamId_Id]));
			}
			break;
		case Type::Generation:
			if (pSprite && generator) {
				isGeneratorActive = true;
				generator(0.0f, pSprite, data.param[GenParamId_Speed], data.param[GenParamId_DirectionDegree]);
			}
			break;
		}
	}
}

/**
* �A�N�V�������X�v���C�g�ɔ��f����.
*
* @param delta �X�V����(�b).
* @param pSprite �A�N�V�����𔽉f����X�v���C�g�ւ̃|�C���^.
*/
void Controller::UpdateSub(float delta, Sprite::Sprite* pSprite)
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

/**
* �A�N�V�����̏�Ԃ��X�V���A�X�v���C�g�ɔ��f����.
*
* @param delta �X�V����(�b).
* @param pSprite �A�N�V�����𔽉f����X�v���C�g�ւ̃|�C���^.
*/
void Controller::Update(float delta, Sprite::Sprite* pSprite)
{
	if (type != Type::Vanishing && isGeneratorActive && generator) {
		generator(delta, pSprite, 0, 0);
	}
	if (type == Type::ManualControl) {
		const XMVECTOR d = XMVectorSwizzle(XMLoadFloat(&delta), 0, 0, 1, 1);
		XMStoreFloat3(&pSprite->pos, XMVectorAdd(XMLoadFloat3(&pSprite->pos), XMVectorMultiply(d, XMLoadFloat2(&move))));
		XMStoreFloat2(&move, XMVectorAdd(XMLoadFloat2(&move), XMVectorMultiply(d, XMLoadFloat2(&accel))));
		if (currentTime < totalTime) {
			currentTime += delta;
		}
		return;
	}
	if (!list || seqIndex >= list->list.size() || dataIndex >= list->list[seqIndex].size() || type == Type::Vanishing) {
		return;
	}
	UpdateSub(std::min(std::max(0.0f, totalTime - currentTime), delta), pSprite);
	currentTime += delta;
	while (currentTime >= totalTime) {
		currentTime -= totalTime;
		totalTime = 0.0f;
		delta = currentTime;
		++dataIndex;
		if (dataIndex >= list->list[seqIndex].size()) {
			return;
		}
		Init(pSprite);
		UpdateSub(std::min(std::max(0.0f, totalTime - currentTime), delta), pSprite);
	}
}

/**
* �Đ�����A�N�V�����V�[�P���X���w�肷��.
*
* @param no �Đ�����A�N�V�����V�[�P���X�̃C���f�b�N�X.
*           �ێ����Ă���V�[�P���X���ȏ�̒l�͖��������(�V�[�P���X�͕ύX����Ȃ�).
*/
void Controller::SetSeqIndex(uint32_t no)
{
	if (!list || no >= list->list.size()) {
		return;
	}
	seqIndex = no;
	dataIndex = 0;
	const Data& data = list->list[seqIndex][dataIndex];
	type = data.type;
	currentTime = 0.0f;
	totalTime = 0.0f;
	Init();
}

/**
* �}�j���A�����[�h�̈ړ����x��ݒ肷��.
*
* @param degree �ړ�����(0-360).
* @param speed  �ړ����x(pixels/s).
*/
void Controller::SetManualMove(float degree, float speed)
{
	type = Type::ManualControl;
	XMStoreFloat2(&move, RadianToVector(DegreeToRadian(degree), speed));
}

/**
* �}�j���A�����[�h�̉����x��ݒ肷��.
*
* @param degree ��������(0-360).
* @param speed  �����x(pixels/s).
*/
void Controller::SetManualAccel(float degree, float speed)
{
	type = Type::ManualControl;
	XMStoreFloat2(&accel, RadianToVector(DegreeToRadian(degree), speed));
}

/**
* �}�j���A�����[�h�̓��쎞�Ԃ�ݒ肷��.
*
* @param time ���쎞��.
*/
void Controller::SetTime(float time)
{
	type = Type::ManualControl;
	currentTime = 0.0f;
	totalTime = time;
}

/**
* ���ŏ�ԂɂȂ��Ă��邩�ǂ���.
*
* @retval true  ���ŏ�ԂɂȂ��Ă���.
* @retval false ���ŏ�ԂɂȂ��Ă��Ȃ�.
*
* �A�N�V�����V�[�P���X��Type::Vanishing���w�肳���Ə��ŏ�ԂɂȂ�A�A�N�V�����̍Đ�����~�����.
*/
bool Controller::IsDeletable() const
{
	return type == Type::Vanishing;
}

/**
* File�C���^�[�t�F�C�X�̎����N���X.
*/
class FileImpl : public File
{
public:
	FileImpl() {}
	virtual ~FileImpl() {}
	virtual const List* Get(uint32_t no) const {
		if (no >= actList.size()) {
			return nullptr;
		}
		return &actList[no];
	}
	virtual size_t Size() const { return actList.size(); }

	std::vector<List> actList;
};

/**
* �t�@�C������A�N�V�������X�g��ǂݍ���.
*
* @param filename �t�@�C����.
*
* @return �ǂݍ��񂾃A�N�V�������X�g.
*         �ǂݍ��ݎ��s�̏ꍇ��nullptr��Ԃ�.
*
* JSON�t�H�[�}�b�g�͎��̂Ƃ���:
* <pre>
* [
*   {
*     "name" : "�A�N�V�������X�g��",
*     "list" :
*     [
*       [
*         {
*           "type" : "�A�N�V�����̎�ނ�����������",
*           "args" : [arg0, arg1, arg2]
*         },
*         ...
*       ],
*       ...
*     ]
*   },
*   ...
* ]
* </pre>
*/
FilePtr LoadFromJsonFile(const wchar_t* filename)
{
	struct HandleHolder {
		explicit HandleHolder(HANDLE h) : handle(h) {}
		~HandleHolder() { if (handle != INVALID_HANDLE_VALUE) { CloseHandle(handle); } }
		HANDLE handle;
		operator HANDLE() { return handle; }
		operator HANDLE() const { return handle; }
	};

	std::shared_ptr<FileImpl> af(new FileImpl);

	HandleHolder h(CreateFileW(filename, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
	if (h == INVALID_HANDLE_VALUE) {
		return af;
	}
	LARGE_INTEGER size;
	if (!GetFileSizeEx(h, &size)) {
		return af;
	}
	if (size.QuadPart > std::numeric_limits<size_t>::max()) {
		return af;
	}
	std::vector<char> buffer;
	buffer.resize(static_cast<size_t>(size.QuadPart));
	DWORD readBytes;
	if (!ReadFile(h, &buffer[0], buffer.size(), &readBytes, nullptr)) {
		return af;
	}
	const Json::Value json = Json::Parse(buffer.data());
	if (json.type != Json::Type::Array) {
		return af;
	}

	for (const Json::Value& e : json.array) {
		if (e.type != Json::Type::Object) {
			break;
		}
		const Json::Object::const_iterator itrName = e.object.find("name");
		if (itrName == e.object.end() || itrName->second.type != Json::Type::String) {
			break;
		}
		List al;
		al.name = itrName->second.string;
		const Json::Object::const_iterator itrList = e.object.find("list");
		if (itrList == e.object.end() || itrList->second.type != Json::Type::Array) {
			break;
		}
		for (const Json::Value& seq : itrList->second.array) {
			if (seq.type != Json::Type::Array) {
				return af;
			}
			Sequence as;
			for (const Json::Value& data : seq.array) {
				if (data.type != Json::Type::Object) {
					return af;
				}
				Data ad = {};
				static const struct {
					const char* const str;
					Type type;
					bool operator==(const std::string& s) const { return s == str; }
				} typeMap[] = {
					{ "Move", Type::Move },
					{ "Accel", Type::Accel },
					{ "Wait", Type::Wait },
					{ "Generate", Type::Generation },
					{ "Animation", Type::Animation },
					{ "Delete", Type::Vanishing },
				};
				const auto itrTypePair = std::find(typeMap, typeMap + _countof(typeMap), data.object.find("type")->second.string);
				if (itrTypePair == typeMap + _countof(typeMap)) {
					return af;
				}
				ad.type = itrTypePair->type;
				const Json::Array& array = data.object.find("args")->second.array;
				for (size_t i = 0; i < 3 && i < array.size(); ++i) {
					ad.param[i] = static_cast<float>(array[i].number);
				}
				as.push_back(ad);
			}
			al.list.push_back(as);
		}
		af->actList.push_back(al);
	}

	return af;
}

} // namespace Action 
