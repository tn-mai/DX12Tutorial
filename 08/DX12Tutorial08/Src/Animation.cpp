/**
* @file Animation.cpp
*/
#include "Animation.h"
#include <windows.h>
#include <map>
#include <vector>
#include <string>

#define ROT(r) (static_cast<float>(r) * 3.14159265359f / 180.0f)

const AnimationData animeDataList0[] = {
	{ 0, 0.125f, ROT(0), { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	{ 1, 0.125f, ROT(0), { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	{ 2, 0.125f, ROT(0), { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	{ 3, 0.125f, ROT(0), { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
};

const AnimationData animeDataList1[] = {
	{ 4, 0.125f, ROT(0), { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	{ 5, 0.125f, ROT(0), { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	{ 6, 0.125f, ROT(0), { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
};

const AnimationData animeDataList2[] = {
	{ 7, 0.125f, ROT(0), { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	{ 8, 0.125f, ROT(0), { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	{ 9, 0.125f, ROT(0), { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
};

const AnimationData animeDataList3[] = {
	{ 10, 0.1f, ROT(0),{ 1.0f, 1.0f },{ 1.0f, 1.0f, 1.0f, 1.0f } },
	{ 10, 0.1f, ROT(45.0f * 0.25f),{ 0.9f, 0.9f },{ 1.0f, 1.0f, 1.0f, 1.0f } },
	{ 10, 0.1f, ROT(45.0f * 0.5f),{ 1.0f, 1.0f },{ 1.0f, 1.0f, 1.0f, 1.0f } },
	{ 10, 0.1f, ROT(45.0f * 0.75f),{ 1.1f, 1.1f },{ 1.0f, 1.0f, 1.0f, 1.0f } },
};

const AnimationData animeDataList4[] = {
	{ 11, 0.1f, ROT(0), { 0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	{ 11, 0.1f, ROT(0), { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	{ 12, 0.1f, ROT(0), { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	{ 13, 0.1f, ROT(0), { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	{ 14, 0.05f, ROT(0), { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	{ 14, 0.05f, ROT(0), { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 0.5f } },
	{ 15, 1.0f, ROT(0), { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
};

const AnimationData animeDataList5[] = {
	{ 16, 0.125f, ROT(0),{ 1.0f, 1.0f },{ 1.0f, 1.0f, 1.0f, 1.0f } },
	{ 17, 0.125f, ROT(0),{ 1.0f, 1.0f },{ 1.0f, 1.0f, 1.0f, 1.0f } },
	{ 18, 0.125f, ROT(0),{ 1.0f, 1.0f },{ 1.0f, 1.0f, 1.0f, 1.0f } },
};

struct {
	size_t size;
	const AnimationData* list;
} const animeDataListArray[] = {
	{ _countof(animeDataList0), animeDataList0 },
	{ _countof(animeDataList1), animeDataList1 },
	{ _countof(animeDataList2), animeDataList2 },
	{ _countof(animeDataList3), animeDataList3 },
	{ _countof(animeDataList4), animeDataList4 },
	{ _countof(animeDataList5), animeDataList5 },
};

/**
* �R���X�g���N�^.
*
* @param al �ݒ肷��A�j���[�V�������X�g.
*/
AnimationController::AnimationController(const AnimationList& al)
	: list(al)
	, seqIndex(0)
	, cellIndex(0)
	, time(0.0f)
{
}

/**
* �A�j���[�V�����V�[�P���X�̃C���f�b�N�X��ݒ肷��.
*
* @param idx �ݒ肷��V�[�P���X�C���f�b�N�X.
*/
void AnimationController::SetSeqIndex(uint32_t idx)
{
	if (idx >= list.list.size()) {
		return;
	}
	seqIndex = idx;
	cellIndex = 0;
	time = 0.0f;
}

/**
* �A�j���[�V�����̏�Ԃ��X�V����.
*
* @param delta �o�ߎ���.
*/
void AnimationController::Update(double delta)
{
	if (seqIndex >= list.list.size() || list.list[seqIndex].empty()) {
		return;
	}

	time += delta;
	for (;;) {
		const float targetTime = list.list[seqIndex][cellIndex].time;
		if (targetTime <= 0.0f || time < targetTime) {
			break;
		}
		time -= targetTime;
		++cellIndex;
		if (cellIndex >= list.list[seqIndex].size()) {
			cellIndex = 0;
		}
	}
}

/**
* �A�j���[�V�����f�[�^���擾����.
*
* @return �A�j���[�V�����f�[�^.
*/
const AnimationData& AnimationController::GetData() const
{
	if (seqIndex >= list.list.size() || list.list[seqIndex].empty()) {
		static const AnimationData dummy{};
		return dummy;
	}
	return list.list[seqIndex][cellIndex];
}

/**
* ���X�g���̃A�j���[�V�����V�[�P���X�̐����擾����.
*
* @return �A�j���[�V�����V�[�P���X�̐�.
*/
size_t AnimationController::GetSeqCount() const
{
	return list.list.size();
}

namespace Json {


enum class Type
{
	String,
	Number,
	Object,
	Array,
};

struct Value;
typedef std::string String;
typedef double Number;
typedef std::map<std::string, Value> Object;
typedef std::vector<Value> Array;

struct Value
{
	Value() : type(Type::Number) {}
	~Value() {
		switch (type) {
		case Type::String: string.~basic_string(); break;
		case Type::Number: break;
		case Type::Object: object.~map(); break;
		case Type::Array: array.~vector(); break;
		}
	}

	Value(const Value& v) {
		type = v.type;
		switch (type) {
		case Type::String: new(&string) String(v.string); break;
		case Type::Number: new(&number) Number(v.number); break;
		case Type::Object: new(&object) Object(v.object); break;
		case Type::Array: new(&array) Array(v.array); break;
		}
	}
	Value(const std::string& s) : type(Type::String) { new(&string) String(s); }
	Value(double d) : type(Type::Number) { new(&number) Number(d); }
	Value(const Object& o) : type(Type::Object) { new(&object) Object(o); }
	Value(const Array& a) : type(Type::Array) { new(&array) Array(a); }

	template<typename T>
	Value& operator=(const T& v) {
		(*this).~Value();
		new(this) Value(v);
		return *this;
	}

	Type type;
	union {
		String string;
		Number number;
		Object object;
		Array array;
	};
};

Value ParseValue(const char*& data);

void SkipSpace(const char*& data)
{
	for (;; ++data) {
		switch (*data) {
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			break;
		default:
			goto end;
		}
	}
end:
	return;
}

Value ParseString(const char*& data)
{
	++data; // skip first double quotation.

	std::string s;
	for (; *data != '"'; ++data) {
		s.push_back(static_cast<char>(*data));
	}
	++data; // skip last double quotation.
	return Value(s);
}

Value ParseNumber(const char*& data)
{
	char* end;
	const double d = strtod(data, &end);
	data = end;
	return Value(d);
}

Value ParseObject(const char*& data)
{
	++data; // skip first brace.
	SkipSpace(data);
	if (*data == '}') {
		++data;
		return Value(Object());
	}

	Object obj;
	for (;;) {
		const std::string key = ParseString(data).string;
		SkipSpace(data);
		++data; // skip colon.
		SkipSpace(data);
		Value value = ParseValue(data);
		obj.insert(std::make_pair(key, value));

		SkipSpace(data);
		if (*data == '}') {
			++data; // skip last brace.
			break;
		}
		++data; // skip comma.
		SkipSpace(data);
	}
	return Value(obj);
}

Value ParseArray(const char*& data)
{
	++data; // skip first bracket.
	SkipSpace(data);
	if (*data == ']') {
		++data;
		return Value(Array());
	}

	Array arr;
	for (;;) {
		Value value = ParseValue(data);
		arr.push_back(value);
		SkipSpace(data);
		if (*data == ']') {
			++data; // skip last bracket.
			break;
		}
		++data; // skip comma.
		SkipSpace(data);
	}
	return Value(arr);
}

Value ParseValue(const char*& data)
{
	switch (*data) {
	case '"': return ParseString(data);
	case '{': return ParseObject(data);
	case '[': return ParseArray(data);
	default: return ParseNumber(data);
	}
}

Value Parse(const char* data)
{
	return ParseValue(data);
}

} // namespace Json

/**
* �t�@�C������A�j���[�V�������X�g��ǂݍ���.
*
* @param list     �ǂݍ��ݐ�I�u�W�F�N�g.
* @param filename �t�@�C����.
*
* @retval true  �ǂݍ��ݐ���.
* @retval false �ǂݍ��ݎ��s.
*
* [
*   {
*     "name" : animation list name string
*     "list" :
*     [
*       [
*         {
*           "cell" : cell index,
*           "time" : duration time,
*           "rotation" : rotation radian,
*           "scale" : [x, y],
*           "color" : [r, g, b, a]
*         },
*         ...
*       ],
*       ...
*     ]
*   },
*   ...
* ]
*/
typedef std::vector<AnimationList> AnimationFile;
AnimationFile LoadAnimationListFromJsonFile(const wchar_t* filename)
{
	struct HandleHolder {
		explicit HandleHolder(HANDLE h) : handle(h) {}
		~HandleHolder() { if (handle != INVALID_HANDLE_VALUE) { CloseHandle(handle); } }
		HANDLE handle;
		operator HANDLE() { return handle; }
		operator HANDLE() const { return handle; }
	};

	HandleHolder h(CreateFileW(filename, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
	if (h == INVALID_HANDLE_VALUE) {
		return {};
	}
	LARGE_INTEGER size;
	if (!GetFileSizeEx(h, &size)) {
		return {};
	}
	std::vector<char> buffer;
	buffer.resize(size.QuadPart);
	DWORD readBytes;
	if (!ReadFile(h, &buffer[0], buffer.size(), &readBytes, nullptr)) {
		return {};
	}
	const Json::Value json = Json::Parse(buffer.data());
	if (json.type != Json::Type::Array) {
		return {};
	}
	AnimationFile af;
	for (const Json::Value& e : json.array) {
		if (e.type != Json::Type::Object) {
			break;
		}
		const Json::Object::const_iterator itrName = e.object.find("name");
		if (itrName == e.object.end() || itrName->second.type != Json::Type::String) {
			break;
		}
		AnimationList al;
		al.name = itrName->second.string;
		const Json::Object::const_iterator itrList = e.object.find("list");
		if (itrList == e.object.end() || itrList->second.type != Json::Type::Array) {
			break;
		}
		for (const Json::Value& seq : itrList->second.array) {
			if (seq.type != Json::Type::Array) {
				return af;
			}
			AnimationSequence as;
			for (const Json::Value& data : seq.array) {
				if (data.type != Json::Type::Object) {
					return af;
				}
				AnimationData ad;
				ad.cellIndex = data.object.find("cell")->second.number;
				ad.time = data.object.find("time")->second.number;
				ad.rotation = data.object.find("rotation")->second.number;
				{
					auto itr = data.object.find("scale");
					if (itr == data.object.end() || itr->second.type != Json::Type::Array || itr->second.array.size() < 2) {
						return af;
					}
					ad.scale.x = itr->second.array[0].number;
					ad.scale.y = itr->second.array[1].number;
				}
				{
					auto itr = data.object.find("color");
					if (itr == data.object.end() || itr->second.type != Json::Type::Array || itr->second.array.size() < 4) {
						return af;
					}
					ad.color.x = itr->second.array[0].number;
					ad.color.y = itr->second.array[1].number;
					ad.color.z = itr->second.array[2].number;
					ad.color.w = itr->second.array[3].number;
				}
				as.push_back(ad);
			}
			al.list.push_back(as);
		}
		af.push_back(al);
	}

	return af;
}


/**
* �A�j���[�V�������X�g���擾����.
*
* @return �A�j���[�V�������X�g.
*/
const AnimationList& GetAnimationList()
{
#if 0
	static AnimationList list;

	if (!list.list.empty()) {
		return list;
	}

	list.list.reserve(_countof(animeDataListArray));
	for (auto e : animeDataListArray) {
		AnimationSequence seq;
		seq.resize(e.size);
		for (size_t i = 0; i < e.size; ++i) {
			seq[i] = e.list[i];
		}
		list.list.push_back(seq);
	}
	return list;
#else
	static AnimationFile af;
	if (af.empty()) {
		af = LoadAnimationListFromJsonFile(L"Res/animation.json");
	}
	return af[0];
#endif
}
