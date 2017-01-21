/**
* @file Animation.cpp
*/
#include "Animation.h"
#include "Json.h"
#include <windows.h>
#include <map>
#include <vector>
#include <string>

/**
* コンストラクタ.
*
* @param al 設定するアニメーションリスト.
*/
AnimationController::AnimationController(const AnimationList& al)
	: list(al)
	, seqIndex(0)
	, cellIndex(0)
	, time(0.0f)
{
}

/**
* アニメーションシーケンスのインデックスを設定する.
*
* @param idx 設定するシーケンスインデックス.
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
* アニメーションの状態を更新する.
*
* @param delta 経過時間.
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
* アニメーションデータを取得する.
*
* @return アニメーションデータ.
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
* リスト内のアニメーションシーケンスの数を取得する.
*
* @return アニメーションシーケンスの数.
*/
size_t AnimationController::GetSeqCount() const
{
	return list.list.size();
}

/**
* アニメーションが終了しているか調べる.
*
* @retval true  終了している.
* @retval false 再生中.
*/
bool AnimationController::IsFinished() const
{
	if (seqIndex >= list.list.size() || list.list[seqIndex].empty()) {
		return true;
	}
	return list.list[seqIndex][cellIndex].time < 0;
}

/**
* ファイルからアニメーションリストを読み込む.
*
* @param list     読み込み先オブジェクト.
* @param filename ファイル名.
*
* @retval true  読み込み成功.
* @retval false 読み込み失敗.
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
AnimationFile LoadAnimationFromJsonFile(const wchar_t* filename)
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
	if (size.QuadPart > std::numeric_limits<size_t>::max()) {
		return {};
	}
	std::vector<char> buffer;
	buffer.resize(static_cast<size_t>(size.QuadPart));
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
				ad.cellIndex = static_cast<uint32_t>(data.object.find("cell")->second.number);
				ad.time = static_cast<float>(data.object.find("time")->second.number);
				ad.rotation = static_cast<float>(data.object.find("rotation")->second.number);
				{
					auto itr = data.object.find("scale");
					if (itr == data.object.end() || itr->second.type != Json::Type::Array || itr->second.array.size() < 2) {
						return af;
					}
					ad.scale.x = static_cast<float>(itr->second.array[0].number);
					ad.scale.y = static_cast<float>(itr->second.array[1].number);
				}
				{
					auto itr = data.object.find("color");
					if (itr == data.object.end() || itr->second.type != Json::Type::Array || itr->second.array.size() < 4) {
						return af;
					}
					ad.color.x = static_cast<float>(itr->second.array[0].number);
					ad.color.y = static_cast<float>(itr->second.array[1].number);
					ad.color.z = static_cast<float>(itr->second.array[2].number);
					ad.color.w = static_cast<float>(itr->second.array[3].number);
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
* アニメーションリストを取得する.
*
* @return アニメーションリスト.
*/
const AnimationList& GetAnimationList()
{
	static AnimationFile af;
	if (af.empty()) {
		af = LoadAnimationFromJsonFile(L"Res/Anm/Animation.json");
	}
	return af[0];
}
