/**
* @file Animation.h
**/
#ifndef DX12TUTORIAL_SRC_ANIMATION_H_
#define DX12TUTORIAL_SRC_ANIMATION_H_
#include <vector>
#include <stdint.h>

/**
* �A�j���[�V�����f�[�^.
*/
struct AnimationData
{
	uint32_t cellIndex; ///< �Z���f�[�^���X�g���̃C���f�b�N�X.
	float time; ///< �\�����鎞��(�b).
};

/**
* �P��̃A�j���[�V�������\������A�j���[�V�����f�[�^�̃��X�g.
*/
typedef std::vector<AnimationData> AnimationSequence;

/**
* �A�j���[�V�����V�[�P���X�̃��X�g.
*
* �C���f�b�N�X�w��ɂ���āA�Đ�����A�j���[�V�������擾�ł���悤�ɂ���.
*/
typedef std::vector<AnimationSequence> AnimationList;

/**
* �A�j���[�V��������N���X.
*/
class AnimationController
{
public:
	AnimationController() = delete;
	explicit AnimationController(const AnimationList& list);
	void SetSeqIndex(uint32_t no);
	void Update(double delta);
	uint32_t GetCellIndex() const;
	size_t GetSeqCount() const;

private:
	const AnimationList& list;
	uint32_t seqIndex;
	uint32_t cellIndex;
	double time;
};

const AnimationList& GetAnimationList();

#endif // DX12TUTORIAL_SRC_ANIMATION_H_