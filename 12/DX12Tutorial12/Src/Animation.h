/**
* @file Animation.h
**/
#ifndef DX12TUTORIAL_SRC_ANIMATION_H_
#define DX12TUTORIAL_SRC_ANIMATION_H_
#include <DirectXMath.h>
#include <vector>
#include <stdint.h>

/**
* �A�j���[�V�����f�[�^.
*/
struct AnimationData
{
	uint32_t cellIndex; ///< �Z���f�[�^���X�g���̃C���f�b�N�X.
	float time; ///< �\�����鎞��(�b).
	float rotation; ///< �摜�̉�]�p(���W�A��).
	DirectX::XMFLOAT2 scale; ///< �摜�̊g�嗦.
	DirectX::XMFLOAT4 color; ///< �摜�̐F.
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
struct AnimationList
{
	std::string name;
	std::vector<AnimationSequence> list;
};

/**
* �A�j���[�V�������X�g�̃��X�g.
*/
typedef std::vector<AnimationList> AnimationFile;

/**
* �A�j���[�V��������N���X.
*/
class AnimationController
{
public:
	AnimationController() = default;
	explicit AnimationController(const AnimationList* list);
	AnimationController(const AnimationController&) = default;
	~AnimationController() = default;
	AnimationController& operator=(const AnimationController&) = default;

	void SetSeqIndex(uint32_t index);
	void SetCellIndex(uint32_t index);
	void Update(double delta);
	const AnimationData& GetData() const;
	size_t GetSeqCount() const;
	uint32_t GetSeqIndex() const { return seqIndex; }
	bool IsFinished() const;

private:
	const AnimationList* list = nullptr;
	uint32_t seqIndex = 0;
	uint32_t cellIndex = 0;
	double time = 0;
};

const AnimationList& GetAnimationList();
AnimationFile LoadAnimationFromJsonFile(const wchar_t* filename);

#endif // DX12TUTORIAL_SRC_ANIMATION_H_
