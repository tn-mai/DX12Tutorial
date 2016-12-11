/**
* @file Animation.cpp
*/
#include "Animation.h"

const AnimationData animeDataList0[] = {
	{0, 0.125f}, {1, 0.125f }, {2, 0.125f }, {3, 0.125f }
};

const AnimationData animeDataList1[] = {
	{ 4, 0.125f },{ 5, 0.125f },{ 6, 0.125f }
};

const AnimationData animeDataList2[] = {
	{ 7, 0.125f },{ 8, 0.125f },{ 9, 0.125f }
};

const AnimationData animeDataList3[] = {
	{ 10, 0.125f }
};

const AnimationData animeDataList4[] = {
	{ 11, 0.125f },{ 12, 0.125f },{ 13, 0.125f },{ 14, 0.125f }
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
* �R���X�g���N�^.
*/
AnimationController::AnimationController()
	: list(nullptr)
	, seqNo(0)
	, index(0)
	, time(0.0f)
{
}

/**
* �A�j���[�V�������X�g��ݒ肷��.
*
* @param list �ݒ肷��A�j���[�V�������X�g.
*/
void AnimationController::SetList(const AnimationList& animeList)
{
	list = &animeList;
	seqNo = 0;
	index = 0;
	time = 0.0f;
}

/**
* �A�j���[�V�����V�[�P���X�ԍ���ݒ肷��.
*
* @param no �ݒ肷��V�[�P���X�ԍ�.
*/
void AnimationController::SetSeqNo(uint32_t no)
{
	if (!list || no >= list->list.size()) {
		return;
	}
	seqNo = no;
	index = 0;
}

/**
* �A�j���[�V�����̏�Ԃ��X�V����.
*
* @param delta �o�ߎ���.
*/
void AnimationController::Update(double delta)
{
	if (!list || list->list.empty()) {
		return;
	}

	time += delta;
	for (;;) {
		const float targetTime = list->list[seqNo].sequence[index].time;
		if (time < targetTime) {
			break;
		}
		time -= targetTime;
		++index;
		if (index >= list->list[seqNo].sequence.size()) {
			index = 0;
		}
	}
}

/**
* �Z���ԍ����擾����.
*
* @return �Z���ԍ�.
*/
const uint32_t AnimationController::GetCellIndex() const
{
	if (!list || list->list.empty() || list->list[seqNo].sequence.empty()) {
		return 0;
	}
	return list->list[seqNo].sequence[index].cell;
}

/**
* �A�j���[�V�������X�g���擾����.
*
* @return �A�j���[�V�������X�g.
*/
const AnimationList& GetAnimationList()
{
	static AnimationList list;

	if (!list.list.empty()) {
		return list;
	}

	list.list.reserve(_countof(animeDataListArray));
	for (auto e : animeDataListArray) {
		AnimationSequence seq;
		seq.sequence.resize(e.size);
		for (size_t i = 0; i < e.size; ++i) {
			seq.sequence[i] = e.list[i];
		}
		list.list.push_back(seq);
	}
	return list;
}