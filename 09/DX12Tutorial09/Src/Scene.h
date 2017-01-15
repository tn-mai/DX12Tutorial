/**
* @file Scene.h
*/
#ifndef DX12TUTORIAL_SRC_SCENE_H_
#define DX12TUTORIAL_SRC_SCENE_H_
#include <memory>
#include <vector>
#include <map>
#include <string>

namespace Graphics { class Graphics; }

namespace Scene {

class Context;

/**
*
*/
class Scene
{
	friend class Context;

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

typedef std::shared_ptr<Scene> ScenePtr;

struct Creator
{
	typedef ScenePtr(*Func)();

	int id;
	Func func;
};

enum class TransitionType
{
	Jump,
	Push,
	Pop,
};

struct Transition
{
	int currentScene;
	struct {
		int exitCode;
		TransitionType type;
		int nextScene;
	} trans;
};

/**
*
*/
class Context
{
public:
	bool Initialize(const Transition* transitionList, size_t transitionCount, const Creator* creatorList, size_t creatorCount);
	bool Start(int);
	void Update(double delta);
	void Draw(Graphics::Graphics&) const;

private:
	void LoadScene(int, Creator::Func);
	void UnloadScene();

private:
	typedef std::vector<Transition> TransitionMap;
	typedef std::map<int, Creator::Func> MapType;

	MapType creatorMap;
	TransitionMap transitionMap;

	struct SceneInfo {
		int id;
		ScenePtr p;
	};
	std::vector<SceneInfo> sceneStack;
};

} // namespace Scene

#endif // DX12TUTORIAL_SRC_SCENE_H_