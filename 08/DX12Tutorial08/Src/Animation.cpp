/**
* @file Animation.cpp
*/
#include "Animation.h"

const AnimationData animeDataList[] = {
	0, 1, 2, 3
};

/**
* �R���X�g���N�^.
*/
AnimationController::AnimationController()
	: list(nullptr)
	, seqNo(0)
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
}

/**
* �A�j���[�V�����̏�Ԃ��X�V����.
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
* �Z���ԍ����擾����.
*
* @return �Z���ԍ�.
*/
const uint32_t AnimationController::GetCellIndex() const
{
	if (!list || list->list.empty() || list->list[seqNo].sequence.empty()) {
		return 0;
	}
	return list->list[seqNo].sequence[static_cast<size_t>(time)].cell;
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

	list.list.resize(1);
	for (auto e : animeDataList) {
		list.list[0].sequence.push_back(e);
	}
	return list;
}