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

/**
*
*/
class Scene
{
	friend class TransitionController;

public:
	/** The scene has a status.
	*/
	enum class StatusCode {
		Loading, ///< the scene is constructed. and it is in preparation.
		Runnable, ///< the scene is prepared. it can run.
		Unloading, ///< the scene is finalizing.
		Stopped, ///< the scene is finalized.it shuoud be removed as soon as possible.
	};

	enum ExitCode {
		ExitCode_Continue = -2,
		ExitCode_Exit = -1,
		ExitCode_User = 0,
	};

	explicit Scene(const wchar_t* s) : status(StatusCode::Loading), name(s) {}
	virtual ~Scene() {}

	StatusCode GetState() const { return status; }

private:
	virtual bool Load() { return true; }
	virtual bool Unload() { return true; }
	virtual int Update(double) = 0;
	virtual void UpdateForPause(double) {}
	virtual void Draw(Graphics::Graphics&) const = 0;
	virtual void Pause() {}
	virtual void Resume() {}

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
	bool Start(int);
	void Update(double delta);
	void Draw(Graphics::Graphics&) const;

private:
	const Creator* FindCreator(int) const;
	void LoadScene(const Creator*);
	void UnloadScene();

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