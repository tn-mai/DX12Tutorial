/**
* @file Animation.cpp
*/
#include "Animation.h"

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

struct {
	size_t size;
	const AnimationData* list;
} const animeDataListArray[] = {
	{ _countof(animeDataList0), animeDataList0 },
	{ _countof(animeDataList1), animeDataList1 },
	{ _countof(animeDataList2), animeDataList2 },
	{ _countof(animeDataList3), animeDataList3 },
	{ _countof(animeDataList4), animeDataList4 },
};

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
	if (idx >= list.size()) {
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
	if (seqIndex >= list.size() || list[seqIndex].empty()) {
		return;
	}

	time += delta;
	for (;;) {
		const float targetTime = list[seqIndex][cellIndex].time;
		if (targetTime <= 0.0f || time < targetTime) {
			break;
		}
		time -= targetTime;
		++cellIndex;
		if (cellIndex >= list[seqIndex].size()) {
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
	if (seqIndex >= list.size() || list[seqIndex].empty()) {
		static const AnimationData dummy{};
		return dummy;
	}
	return list[seqIndex][cellIndex];
}

/**
* リスト内のアニメーションシーケンスの数を取得する.
*
* @return アニメーションシーケンスの数.
*/
size_t AnimationController::GetSeqCount() const
{
	return list.size();
}

/**
* アニメーションリストを取得する.
*
* @return アニメーションリスト.
*/
const AnimationList& GetAnimationList()
{
	static AnimationList list;

	if (!list.empty()) {
		return list;
	}

	list.reserve(_countof(animeDataListArray));
	for (auto e : animeDataListArray) {
		AnimationSequence seq;
		seq.resize(e.size);
		for (size_t i = 0; i < e.size; ++i) {
			seq[i] = e.list[i];
		}
		list.push_back(seq);
	}
	return list;
}
