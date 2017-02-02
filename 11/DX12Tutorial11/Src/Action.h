/**
* @file Action.h
*/
#ifndef DX12TUTORIAL_SRC_ACTION_H_
#define DX12TUTORIAL_SRC_ACTION_H_
#include <DirectXMath.h>
#include <stdint.h>
#include <vector>
#include <memory>
#include <functional>

namespace Sprite {
struct Sprite;
} // namespace Sprite

/**
* �X�v���C�g�̍��W�𐧌䂷�邽�߂́A�A�N�V�����ƌĂ΂��@�\�Ɋւ��閼�O���.
*/
namespace Action {

struct List;
enum class Type;
enum InterporationType;

/**
* �I�u�W�F�N�g�����֐��^.
*/
typedef std::function<void(float, Sprite::Sprite*, float, float)> GeneratorType;

/**
* �p�X�����̂��߂̃R���g���[���|�C���g�^.
*/
struct Point {
	DirectX::XMFLOAT2 pos;
	float t;
	bool operator<(const Point& p) const { return t < p.t; }
};

/**
* �A�N�V�����𐧌䂷��N���X.
*/
class Controller
{
public:
	Controller();
	Controller(const List* l, uint32_t no = 0);
	void SetList(const List*, uint32_t no = 0);
	void SetGenerator(GeneratorType gen) { generator = gen; }
	void Update(float delta, Sprite::Sprite*);
	void SetSeqIndex(uint32_t no);
	void SetManualMove(float degree, float speed);
	void SetManualMove(const DirectX::XMFLOAT2& m) { move = m; }
	void SetManualAccel(float degree, float accel);
	void SetTime(float time);
	float GetCurrentTime() const { return currentTime; }
	float GetTotalTime() const { return totalTime; }
	uint32_t GetSeqIndex() const { return seqIndex; }
	bool IsDeletable() const;

private:
	void Init(Sprite::Sprite* = nullptr);
	void UpdateSub(float delta, Sprite::Sprite*);

private:
	const List* list;
	uint32_t seqIndex;
	uint32_t dataIndex;
	Type type;

	float totalTime;
	float currentTime;

	DirectX::XMFLOAT2 move;
	DirectX::XMFLOAT2 accel;
	struct PathParam {
		std::vector<Point> cp;
		InterporationType type;
	} path;

	bool isGeneratorActive;
	GeneratorType generator;
};

/**
* �����̃A�N�V�������X�g���܂Ƃ߂��I�u�W�F�N�g�𑀍삷�邽�߂̃C���^�[�t�F�C�X�N���X.
*
* LoadFromJsonFile()�֐����g���ăC���^�[�t�F�C�X�ɑΉ������I�u�W�F�N�g���擾���AGet()��
* �X�̃A�N�V�������X�g�ɃA�N�Z�X����.
*/
class File
{
public:
	File() = default;
	File(const File&) = delete;
	File& operator=(const File&) = delete;
	virtual ~File() {}

	virtual const List* Get(uint32_t no) const = 0;
	virtual size_t Size() const = 0;
};
typedef std::shared_ptr<File> FilePtr;

FilePtr LoadFromJsonFile(const wchar_t*);

} // namespace Action

#endif // DX12TUTORIAL_SRC_ACTION_H_