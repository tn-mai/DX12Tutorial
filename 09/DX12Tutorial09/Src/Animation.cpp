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
	{ 11, 0.125f },{ 12, 0.125f },{ 13, 0.125f },{ 14, 0.125f }, { 15, 0.0f }
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
	if (idx >= list.size()) {
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
* �Z���C���f�b�N�X���擾����.
*
* @return �Z���C���f�b�N�X.
*/
uint32_t AnimationController::GetCellIndex() const
{
	if (seqIndex >= list.size() || list[seqIndex].empty()) {
		return 0;
	}
	return list[seqIndex][cellIndex].cellIndex;
}

/**
* ���X�g���̃A�j���[�V�����V�[�P���X�̐����擾����.
*
* @return �A�j���[�V�����V�[�P���X�̐�.
*/
size_t AnimationController::GetSeqCount() const
{
	return list.size();
}

/**
* �A�j���[�V�������X�g���擾����.
*
* @return �A�j���[�V�������X�g.
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
