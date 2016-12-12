/**
* @file Animation.h
**/
#ifndef DX12TUTORIAL_SRC_ANIMATION_H_
#define DX12TUTORIAL_SRC_ANIMATION_H_
#include <vector>
#include <stdint.h>

/**
* アニメーションデータ.
*/
struct AnimationData
{
	uint32_t cellIndex; ///< セルデータリスト内のインデックス.
	float time; ///< 表示する時間(秒).
};

/**
* 単一のアニメーションを構成するアニメーションデータのリスト.
*/
typedef std::vector<AnimationData> AnimationSequence;

/**
* アニメーションシーケンスのリスト.
*
* インデックス指定によって、再生するアニメーションを取得できるようにする.
*/
typedef std::vector<AnimationSequence> AnimationList;

/**
* アニメーション制御クラス.
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