/**
* @file Scene.h
*/
#ifndef DX12TUTORIAL_SRC_SCENE_H_
#define DX12TUTORIAL_SRC_SCENE_H_
#include <memory>
#include <vector>
#include <string>

namespace Graphics { class Graphics; }

namespace Scene {

class TransitionController;

struct Context
{
	int score;
};

/**
* ��ʂ��邢�͏�Ԃ�\���N���X.
*/
class Scene
{
	friend class TransitionController;

public:
	/// �V�[���̏��.
	enum class StatusCode {
		Loading, ///< �V�[���̏���������.
		Runnable, ///< �������������Ď��s�\�ȏ��.
		Unloading, ///< �V�[���̏I��������.
		Stopped, ///< �V�[���͏I�����Ă���.
	};

	/**
	* �I���R�[�h.
	*
	* �h���N���X�œƎ��̏I���R�[�h���`����ꍇ�AExitCode_User�ȏ�̒l�����蓖�Ă邱��.
	*/
	enum ExitCode {
		ExitCode_Continue = -2, ///< ���݂̃V�[�����p��.
		ExitCode_Exit = -1, ///< ���݂̃V�[�����I��.
		ExitCode_User = 0, ///< ��������h���N���X�p�̏I���R�[�h.
	};

	explicit Scene(const wchar_t* s) : status(StatusCode::Loading), name(s) {}
	virtual ~Scene() {}
	StatusCode GetState() const { return status; }

private:
	virtual int Update(Context&, double) = 0;
	virtual void Draw(Graphics::Graphics&) const = 0;
	virtual bool Load(Context&) { return true; }
	virtual bool Unload(Context&) { return true; }
	virtual void UpdateForPause(Context&, double) {}
	virtual void Pause(Context&) {}
	virtual void Resume(Context&) {}

private:
	StatusCode status;
	std::wstring name;
};

class Scene;
typedef std::shared_ptr<Scene> ScenePtr;

/**
* �V�[���쐬���.
*/
struct Creator
{
	typedef ScenePtr(*Func)();

	int id; ///< �V�[��ID.
	Func func; ///< �쐬�֐��ւ̃|�C���^.
};

/**
* �V�[���̑J�ڕ��@.
*/
enum class TransitionType
{
	Jump, ///< ���݂̃V�[�����I�����Ď��̃V�[���ɑJ��.
	Push, ///< ���݂̃V�[�����X�^�b�N�ɐς�ł��玟�̃V�[���ɑJ��.
	Pop, ///< ���݂̃V�[�����X�^�b�N����~�낵�đO�̃V�[���ɑJ��.
};

/**
* �V�[���̑J�ڏ��.
*/
struct Transition
{
	int currentScene; ///< ���݂̃V�[��ID.
	struct {
		int exitCode; ///< �V�[���̏I���R�[�h.
		TransitionType type; ///< �J�ڕ��@.
		int nextScene; ///< �J�ڐ�̃V�[��ID.
	} trans; ///< �J�ڐ���.
};

/**
* �V�[���J�ڂ𐧌䂷��N���X.
*/
class TransitionController
{
public:
	bool Initialize(const Transition* transitionList, size_t transitionCount, const Creator* creatorList, size_t creatorCount);
	bool Start(Context&, int);
	void Stop(Context&);
	void Update(Context&, double delta);
	void Draw(Graphics::Graphics&) const;

private:
	const Creator* FindCreator(int) const;
	void LoadScene(Context&, const Creator*);
	void UnloadScene(Context&);

	struct SceneInfo {
		int id;
		ScenePtr p;
	};

	std::vector<Creator> creatorMap;
	std::vector<Transition> transitionMap;
	std::vector<SceneInfo> sceneStack;
};

} // namespace Scene

#endif // DX12TUTORIAL_SRC_SCENE_H_