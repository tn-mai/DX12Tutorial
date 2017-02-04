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
* 画面あるいは状態を表すクラス.
*/
class Scene
{
	friend class TransitionController;

public:
	/// シーンの状態.
	enum class StatusCode {
		Loading, ///< シーンの準備処理中.
		Runnable, ///< 準備が完了して実行可能な状態.
		Unloading, ///< シーンの終了処理中.
		Stopped, ///< シーンは終了している.
	};

	/**
	* 終了コード.
	*
	* 派生クラスで独自の終了コードを定義する場合、ExitCode_User以上の値を割り当てること.
	*/
	enum ExitCode {
		ExitCode_Continue = -2, ///< 現在のシーンを継続.
		ExitCode_Exit = -1, ///< 現在のシーンを終了.
		ExitCode_User = 0, ///< ここから派生クラス用の終了コード.
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
* シーン作成情報.
*/
struct Creator
{
	typedef ScenePtr(*Func)();

	int id; ///< シーンID.
	Func func; ///< 作成関数へのポインタ.
};

/**
* シーンの遷移方法.
*/
enum class TransitionType
{
	Jump, ///< 現在のシーンを終了して次のシーンに遷移.
	Push, ///< 現在のシーンをスタックに積んでから次のシーンに遷移.
	Pop, ///< 現在のシーンをスタックから降ろして前のシーンに遷移.
};

/**
* シーンの遷移情報.
*/
struct Transition
{
	int currentScene; ///< 現在のシーンID.
	struct {
		int exitCode; ///< シーンの終了コード.
		TransitionType type; ///< 遷移方法.
		int nextScene; ///< 遷移先のシーンID.
	} trans; ///< 遷移先情報.
};

/**
* シーン遷移を制御するクラス.
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