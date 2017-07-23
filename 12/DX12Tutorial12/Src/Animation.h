/**
* @file Animation.h
**/
#ifndef DX12TUTORIAL_SRC_ANIMATION_H_
#define DX12TUTORIAL_SRC_ANIMATION_H_
#include <DirectXMath.h>
#include <vector>
#include <stdint.h>

/**
* アニメーションデータ.
*/
struct AnimationData
{
	uint32_t cellIndex; ///< セルデータリスト内のインデックス.
	float time; ///< 表示する時間(秒).
	float rotation; ///< 画像の回転角(ラジアン).
	DirectX::XMFLOAT2 scale; ///< 画像の拡大率.
	DirectX::XMFLOAT4 color; ///< 画像の色.
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
struct AnimationList
{
	std::string name;
	std::vector<AnimationSequence> list;
};

/**
* アニメーションリストのリスト.
*/
typedef std::vector<AnimationList> AnimationFile;

/**
* アニメーション制御クラス.
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
