/**
* @file Animation.cpp
*/
#include "Animation.h"

const AnimationData animeDataList[] = {
	0, 1, 2, 3
};

/**
* コンストラクタ.
*/
AnimationController::AnimationController()
	: list(nullptr)
	, seqNo(0)
	, time(0.0f)
{
}

/**
* アニメーションリストを設定する.
*
* @param list 設定するアニメーションリスト.
*/
void AnimationController::SetList(const AnimationList& animeList)
{
	list = &animeList;
	seqNo = 0;
	time = 0.0f;
}

/**
* アニメーションシーケンス番号を設定する.
*
* @param no 設定するシーケンス番号.
*/
void AnimationController::SetSeqNo(uint32_t no)
{
	if (!list || no >= list->list.size()) {
		return;
	}
	seqNo = no;
}

/**
* アニメーションの状態を更新する.
*/
void AnimationController::Update()
{
	if (!list || list->list.empty()) {
		return;
	}

	time += 1.0f;
	if (time >= list->list[seqNo].sequence.size()) {
		time = 0.0f;
	}
}

/**
* セル番号を取得する.
*
* @return セル番号.
*/
const uint32_t AnimationController::GetCellIndex() const
{
	if (!list || list->list.empty() || list->list[seqNo].sequence.empty()) {
		return 0;
	}
	return list->list[seqNo].sequence[static_cast<size_t>(time)].cell;
}

/**
* アニメーションリストを取得する.
*
* @return アニメーションリスト.
*/
const AnimationList& GetAnimationList()
{
	static AnimationList list;

	if (!list.list.empty()) {
		return list;
	}

	list.list.resize(1);
	for (auto e : animeDataList) {
		list.list[0].sequence.push_back(e);
	}
	return list;
}